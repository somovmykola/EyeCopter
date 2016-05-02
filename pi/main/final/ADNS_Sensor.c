#include "ADNS_Sensor.h"
#include <stdio.h>
#include <math.h>
#include <sys/ioctl.h>
#include <sys/fcntl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include <wiringPi.h>

#define PI_4 0.78539816339

// Utility structure to help convert between two bytes and a short int when
// reading values from the sensor
typedef union IntUnion {
	int16_t   intValue;
	uint16_t uintValue;
	uint8_t  byteValue[2];
} IntUnion;

struct spi_ioc_transfer tr_info;

void update_conversion_factors(ADNS_Sensor *s) {
	const float scaledNumPixels = (float)s->num_pixels * s->scaler;

	s->conv_factor = 2.0 * tan(0.5 * s->fov) / scaledNumPixels;
	s->radians_to_pixels = scaledNumPixels / s->fov;
}

// Updates x/y, and dx/dy from raw_dx/raw_dy values, according to the current rotation matrix
// Currently just assume identity matrix and let dx = raw_dx
void apply_orientation_matrix(ADNS_Sensor *s) {
	s->dx = s->raw_dx;
	s->dy = s->raw_dy;

	// add rotated values to totals (perhaps this is pointless as we need to take into account yaw, roll, pitch)
	s->x += s->dx;
	s->y += s->dy;
}

uint8_t ADNS_read_register(ADNS_Sensor *s, uint8_t address) {
	uint8_t tx[2] = {address & 0x7F, 0};
	uint8_t rx[2] = {0, 0};

	tr_info.tx_buf = (uint32_t)tx;
	tr_info.rx_buf = (uint32_t)rx;
	tr_info.len = 2;

	// request data
	ioctl(s->fd, SPI_IOC_MESSAGE(1), &tr_info);

	// read data
	tr_info.tx_buf = (uint32_t)NULL;
	ioctl(s->fd, SPI_IOC_MESSAGE(1), &tr_info);

	return rx[1];
}

void ADNS_write_register(ADNS_Sensor *s, uint8_t address, uint8_t value) {
	uint8_t tx[2] = {address | 0x80, 0};
	uint8_t rx[2] = {0, 0};

	tr_info.tx_buf = (uint32_t)tx;
	tr_info.rx_buf = (uint32_t)rx;
	tr_info.len = 2;

	// send register address
	ioctl(s->fd, SPI_IOC_MESSAGE(1), &tr_info);

	tx[0] = value;

	// read data
	ioctl(s->fd, SPI_IOC_MESSAGE(1), &tr_info);
}


int SPI_init(char *device) {
	int mode = ADNS3080_SPI_MODE;
	int bits = ADNS3080_SPI_BITS;
	int speed = ADNS3080_CLOCK_SPEED;

	int fd = open(device, O_RDWR);

	if (fd < 0) {
		fprintf(stderr, "Could not open \"%s\"\n", device);
		return -1;
	}

	// Set SPI mode
	if (ioctl(fd, SPI_IOC_WR_MODE, &mode)) {
		fprintf(stderr, "Unable to set spi mode\n");
		return -1;
	}

	// Set SPI bits per word
	if (ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits)) {
		fprintf(stderr, "Unable to set bits per word\n");
		return -1;
	}

	// Set SPI mode
	if (ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed)) {
		fprintf(stderr, "Unable to set spi speed\n");
		return -1;
	}


	tr_info.delay_usecs = 50;		// 50 uS is minimum delay according to datasheet
	tr_info.speed_hz = ADNS3080_CLOCK_SPEED;
	tr_info.bits_per_word = 8;

	return fd;
}

// Initializes sensor's file descriptor and sets SPI read/write mode states.
// Returns 0 on success, and a negative value on failure.
int ADNS_init(ADNS_Sensor *s) {
	if ((s->fd = SPI_init("/dev/spidev0.0")) < 0)
		return -1;

	if (wiringPiSetup() == -1) {
		printf("Unable to init WiringPi\n");
		return -1;
	}

	pinMode(WPI_RST, OUTPUT);

	s->num_pixels = ADNS3080_PIXELS_X;
	s->fov = ADNS3080_08_FOV;
	s->scaler = ADNS3080_SCALER;

	//_orientation = ROTATION_NONE;
	update_conversion_factors(s);

	// reset the device
	ADNS_reset(s);

	// check the sensor is functioning
	tr_info.delay_usecs = 10000;	// give plenty of extra delay for first calibration

	int retry = 0;
	while (retry < 3) {
		int r = ADNS_read_register(s, ADNS3080_PRODUCT_ID);

		//printf("r = %d\n", r);

		if (r == 0x17)
			return 0;
		retry++;
	}

	fprintf(stderr, "Unable to connect to optical sensor after %d retries\n", retry);

	return -1;
}

// reset sensor by holding reset pin high for 10us.
void ADNS_reset(ADNS_Sensor *s) {
  digitalWrite(WPI_RST, HIGH);
  delayMicroseconds(10);
  digitalWrite(WPI_RST, LOW);
}

// read latest values from sensor and fill in x,y and totals
void ADNS_update(ADNS_Sensor *s) {
	tr_info.delay_usecs = 50;
	s->surface_quality = ADNS_read_register(s, ADNS3080_SQUAL);
	delayMicroseconds(50);

	// check for movement, update x,y values
	tr_info.delay_usecs = 80;
	uint8_t motion_reg = ADNS_read_register(s, ADNS3080_MOTION);
	s->has_overflow = (motion_reg & 0x10) != 0;

	if (motion_reg & 0x80) {
		tr_info.delay_usecs = 50;
		s->raw_dx = (int8_t)ADNS_read_register(s, ADNS3080_DELTA_X);
		delayMicroseconds(50);  // small delay
		s->raw_dy = (int8_t)ADNS_read_register(s, ADNS3080_DELTA_Y);
	} else {
		s->raw_dx = 0;
		s->raw_dy = 0;
	}

	apply_orientation_matrix(s);
}

// Updates internal longitude and latitude with estimation based on optical flow
void ADNS_update_position(ADNS_Sensor *s, float roll, float pitch, float yaw, float altitude) {
	static float prev_roll = 0;
	static float prev_pitch = 0;
	static float prev_altitude = 0;

	float diff_roll  = roll  - prev_roll;
	float diff_pitch = pitch - prev_pitch;

	float sinYaw = sin(yaw);
	float cosYaw = cos(yaw);

	float expected_dx, expected_dy;
	float change_dx, change_dy;

	// only update position if surface quality is good and angle is not over 45 degrees
	if (s->surface_quality >= 10 && fabs(roll) <= PI_4 && fabs(pitch) <= PI_4) {
		altitude = (altitude>0)?altitude:0;

		// calculate expected x,y diff due to roll and pitch change
		expected_dx =  diff_roll  * s->radians_to_pixels;
		expected_dy = -diff_pitch * s->radians_to_pixels;

		// real estimated raw change from mouse
		change_dx = s->dx - expected_dx;
		change_dy = s->dy - expected_dy;

		float avg_altitude = 0.5 * (altitude + prev_altitude);

		// convert raw change to horizontal movement in cm
		s->vx_cm = -change_dx * avg_altitude * s->conv_factor;    // perhaps this altitude should actually be the distance to the ground?  i.e. if we are very rolled over it should be longer?
		s->vy_cm = -change_dy * avg_altitude * s->conv_factor;    // for example if you are leaned over at 45 deg the ground will appear farther away and motion from opt flow sensor will be less

		// convert x/y movements into lon/lat movement
		s->vlon = s->vx_cm * sinYaw + s->vy_cm * cosYaw;
		s->vlat = s->vy_cm * sinYaw - s->vx_cm * cosYaw;
	}

	// Save current roll, pitch and altitude
	prev_roll = roll;
	prev_pitch = pitch;
	prev_altitude = altitude;
}

void ADNS_clear_motion(ADNS_Sensor *s) {
	// writing anything to this register will clear the sensor's motion registers
	tr_info.delay_usecs = 50;
	ADNS_write_register(s, ADNS3080_MOTION_CLEAR, 0xFF);

	s->x = 0;
	s->y = 0;
	s->dx = 0;
	s->dy = 0;
}

// Return values:
// - ADNS3080_LED_MODE_ALWAYS_ON
// - ADNS3080_LED_MODE_WHEN_REQUIRED
uint8_t ADNS_get_led_always_on(ADNS_Sensor *s) {
	uint8_t reg = ADNS_read_register(s, ADNS3080_CONFIGURATION_BITS);
	return (reg & ADNS3080_CONFIG_BIT_LED_MODE) != 0;
}
void ADNS_set_led_always_on(ADNS_Sensor *s, uint8_t alwaysOn) {
	uint8_t reg = ADNS_read_register(s, ADNS3080_CONFIGURATION_BITS);

	reg &= ~ADNS3080_CONFIG_BIT_LED_MODE;					// Mask out other bits
	if (alwaysOn)	reg |= ADNS3080_CONFIG_BIT_LED_MODE;	// Set LED mode bit

	delayMicroseconds(50);

	ADNS_write_register(s, ADNS3080_CONFIGURATION_BITS, reg);
}

// Return values:
// - ADNS3080_RESOLUTION_400
// - ADNS3080_RESOLUTION_1600
uint8_t ADNS_get_resolution(ADNS_Sensor *s) {
	uint8_t reg = ADNS_read_register(s, ADNS3080_CONFIGURATION_BITS);
	return (reg & ADNS3080_CONFIG_BIT_RES) != 0;
}
void ADNS_set_resolution(ADNS_Sensor *s, uint8_t resolution) {
	uint8_t reg = ADNS_read_register(s, ADNS3080_CONFIGURATION_BITS);

	reg &= ~ADNS3080_CONFIG_BIT_RES;					// Mask out other bits
	if (resolution)	reg |= ADNS3080_CONFIG_BIT_RES;		// Set resolution bit

	delayMicroseconds(50);

	ADNS_write_register(s, ADNS3080_CONFIGURATION_BITS, reg);
}

// Return values:
// - ADNS3080_FRAME_RATE_AUTO
// - ADNS3080_FRAME_RATE_FIXED
uint8_t ADNS_get_frame_rate_auto(ADNS_Sensor *s) {
	uint8_t reg = ADNS_read_register(s, ADNS3080_EXTENDED_CONFIG);
	return (reg & ADNS3080_EXT_CFG_BIT_FIXED_FR) != 0;
}
void ADNS_set_frame_rate_auto(ADNS_Sensor *s, uint8_t auto_frame_rate) {
	uint8_t reg = ADNS_read_register(s, ADNS3080_EXTENDED_CONFIG);

	reg &= ~ADNS3080_EXT_CFG_BIT_FIXED_FR;							// Mask out other bits
	if (auto_frame_rate)	reg |= ADNS3080_EXT_CFG_BIT_FIXED_FR;	// Set frame rate bit

	delayMicroseconds(50);

	ADNS_write_register(s, ADNS3080_EXTENDED_CONFIG, reg);
}

// Return values:
// - ADNS3080_SERIAL_NO_PULLUP
// - ADNS3080_SERIAL_HAS_PULLUP
uint8_t ADNS_get_serial_pullup(ADNS_Sensor *s) {
	uint8_t reg = ADNS_read_register(s, ADNS3080_EXTENDED_CONFIG);
	return (reg & ADNS3080_EXT_CFG_BIT_SERIALNPU) != 0;
}
void ADNS_set_serial_pullup(ADNS_Sensor *s, uint8_t pullup) {
	uint8_t reg = ADNS_read_register(s, ADNS3080_EXTENDED_CONFIG);

	reg &= ~ADNS3080_EXT_CFG_BIT_SERIALNPU;				// Mask out other bits
	if (pullup)	reg |= ADNS3080_EXT_CFG_BIT_SERIALNPU;	// Set frame rate bit

	delayMicroseconds(50);

	ADNS_write_register(s, ADNS3080_EXTENDED_CONFIG, reg);
}


// USAGE: Read these registers to determine the current frame period and to
// calculate the frame rate. Units are clock cycles. The formula is
//		Frame Rate = Clock Frequency/Register value
// See: ADNS3080 datasheet pg 32
uint16_t ADNS_get_frame_period(ADNS_Sensor *s) {
	IntUnion period;
	period.byteValue[1] = ADNS_read_register(s, ADNS3080_FRAME_PERIOD_UPPER);
	delayMicroseconds(50);  // small delay
	period.byteValue[0] = ADNS_read_register(s, ADNS3080_FRAME_PERIOD_LOWER);
	return period.uintValue;
}

// USAGE: This value sets the maximum frame period (the MINIMUM frame rate)
// which may be selected by the automatic frame rate control, or sets the
// actual frame period when operating in manual mode. Units are clock cycles.
// The formula is:
//		Frame Rate = Clock Frequency / Register value
// See: ADNS3080 datasheet pg 34
uint16_t ADNS_get_max_frame_period(ADNS_Sensor *s) {
	IntUnion period;
	period.byteValue[1] = ADNS_read_register(s, ADNS3080_FRAME_PERIOD_MAX_BOUND_UPPER);
	delayMicroseconds(50);  // small delay
	period.byteValue[0] = ADNS_read_register(s, ADNS3080_FRAME_PERIOD_MAX_BOUND_LOWER);
	return period.uintValue;
}
void ADNS_set_max_frame_period(ADNS_Sensor *s, uint16_t period) {
	IntUnion periodU;
	periodU.uintValue = period;

	// set specific frame period
	ADNS_write_register(s, ADNS3080_FRAME_PERIOD_MAX_BOUND_LOWER, periodU.byteValue[0]);
	delayMicroseconds(50);  // small delay
	ADNS_write_register(s, ADNS3080_FRAME_PERIOD_MAX_BOUND_UPPER, periodU.byteValue[1]);
}

// USAGE: This value sets the minimum frame period (the MAXIMUM frame rate)
// which may be selected by the automatic frame rate control. Units are clock cycles.
// The formula is:
//		Frame Rate = Clock Frequency / Register value
// See: ADNS3080 datasheet pg 35
uint16_t ADNS_get_min_frame_period(ADNS_Sensor *s) {
	IntUnion period;
	period.byteValue[1] = ADNS_read_register(s, ADNS3080_FRAME_PERIOD_MIN_BOUND_UPPER);
	delayMicroseconds(50);  // small delay
	period.byteValue[0] = ADNS_read_register(s, ADNS3080_FRAME_PERIOD_MIN_BOUND_LOWER);
	return period.uintValue;
}
void ADNS_set_min_frame_period(ADNS_Sensor *s, uint16_t period) {
	IntUnion periodU;
	periodU.uintValue = period;

	// set specific frame period
	ADNS_write_register(s, ADNS3080_FRAME_PERIOD_MIN_BOUND_LOWER, periodU.byteValue[0]);
	delayMicroseconds(50);  // small delay
	ADNS_write_register(s, ADNS3080_FRAME_PERIOD_MIN_BOUND_UPPER, periodU.byteValue[1]);
}

uint32_t ADNS_get_frame_rate(ADNS_Sensor *s) {
	return ADNS3080_CLOCK_SPEED / ADNS_get_frame_period(s);
}
void ADNS_set_frame_rate(ADNS_Sensor *s, uint32_t rate) {
	uint16_t period = ADNS3080_CLOCK_SPEED / rate;
	ADNS_set_max_frame_period(s, period);
}

