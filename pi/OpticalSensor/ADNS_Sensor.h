#ifndef ADNS_SENSOR_H
#define ADNS_SENSOR_H

#include <stdint.h>
#include "pins.h"

// field of view of ADNS3080 sensor lenses
#define ADNS3080_08_FOV 0.202458  // 11.6 degrees

// scaler - value returned when sensor is moved equivalent of 1 pixel
#define ADNS3080_SCALER  1.1

// ADNS3080 hardware config
#define ADNS3080_PIXELS_X       30
#define ADNS3080_PIXELS_Y       30
#define ADNS3080_CLOCK_SPEED	2000000

// SPI information
#define ADNS3080_SPI_MODE		0
#define ADNS3080_SPI_BITS		8


// Register Map for the ADNS3080 Optical OpticalFlow Sensor
#define ADNS3080_PRODUCT_ID						0x00
#define ADNS3080_REVISION_ID					0x01
#define ADNS3080_MOTION							0x02
#define ADNS3080_DELTA_X						0x03
#define ADNS3080_DELTA_Y						0x04
#define ADNS3080_SQUAL							0x05
#define ADNS3080_PIXEL_SUM						0x06
#define ADNS3080_MAXIMUM_PIXEL					0x07
#define ADNS3080_CONFIGURATION_BITS				0x0A
#define ADNS3080_EXTENDED_CONFIG				0x0B
#define ADNS3080_DATA_OUT_LOWER					0x0C
#define ADNS3080_DATA_OUT_UPPER					0x0D
#define ADNS3080_SHUTTER_LOWER					0x0E
#define ADNS3080_SHUTTER_UPPER					0x0F
#define ADNS3080_FRAME_PERIOD_LOWER				0x10
#define ADNS3080_FRAME_PERIOD_UPPER				0x11
#define ADNS3080_MOTION_CLEAR					0x12
#define ADNS3080_FRAME_CAPTURE					0x13
#define ADNS3080_SROM_ENABLE					0x14
#define ADNS3080_FRAME_PERIOD_MAX_BOUND_LOWER	0x19
#define ADNS3080_FRAME_PERIOD_MAX_BOUND_UPPER	0x1A
#define ADNS3080_FRAME_PERIOD_MIN_BOUND_LOWER	0x1B
#define ADNS3080_FRAME_PERIOD_MIN_BOUND_UPPER	0x1C
#define ADNS3080_SHUTTER_MAX_BOUND_LOWER		0x1D
#define ADNS3080_SHUTTER_MAX_BOUND_UPPER		0x1E
#define ADNS3080_SROM_ID						0x1F
#define ADNS3080_OBSERVATION					0x3D
#define ADNS3080_INVERSE_PRODUCT_ID				0x3F
#define ADNS3080_PIXEL_BURST					0x40
#define ADNS3080_MOTION_BURST					0x50
#define ADNS3080_SROM_LOAD						0x60


// Configuration Bits
#define ADNS3080_CONFIG_BIT_LED_MODE		0x40
#define ADNS3080_CONFIG_BIT_SYS_TEST		0x20
#define ADNS3080_CONFIG_BIT_RES				0x10

#define ADNS3080_LED_MODE_ALWAYS_ON			0
#define ADNS3080_LED_MODE_WHEN_REQUIRED		1

#define ADNS3080_SYS_TEST_NO_TESTS			0
#define ADNS3080_SYS_TEST_ALL				1

#define ADNS3080_RESOLUTION_400				0
#define ADNS3080_RESOLUTION_1600			1


// Extended Configuration bits
#define ADNS3080_EXT_CFG_BIT_BUSY			0x80
#define ADNS3080_EXT_CFG_BIT_SERIALNPU		0x04
#define ADNS3080_EXT_CFG_BIT_NAGC			0x02
#define ADNS3080_EXT_CFG_BIT_FIXED_FR		0x01

#define ADNS3080_REGISTER_NOT_BUSY			0
#define ADNS3080_REGISTER_BUSY				1

#define ADNS3080_SERIAL_NO_PULLUP			0
#define ADNS3080_SERIAL_HAS_PULLUP			1

#define ADNS3080_AGC_ACTIVE					0
#define ADNS3080_AGC_DISABLED				1

#define ADNS3080_FRAME_RATE_AUTO			0
#define ADNS3080_FRAME_RATE_FIXED			1

// misc.
#define ADNS3080_SERIALNPU_OFF				0x02

#define ADNS3080_FRAME_RATE_MAX				6469
#define ADNS3080_FRAME_RATE_MIN				2000




typedef struct ADNS_Sensor {
	/*
	bool get_shutter_speed_auto();                      // get_shutter_speed_auto - returns true if shutter speed is adjusted automatically, false if manual
	void set_shutter_speed_auto(bool auto_shutter_speed);   // set_shutter_speed_auto - set shutter speed to auto (true), or manual (false)

	unsigned int get_shutter_speed();
	void set_shutter_speed(unsigned int shutter_speed);

	void print_pixel_data(); // dumps a 30x30 image to the Serial port*/

	int fd;					// file descriptor


    // multiply this number by altitude and pixel change to get horizontal move (in same units as altitude)
    float conv_factor;
    float radians_to_pixels;

	int raw_dx, raw_dy;		// raw sensor change in x and y position (i.e. unrotated)
	int surface_quality;	// image quality (below 15 you really can't trust the x,y values returned)
	int x, y;				// total x,y position
	int dx, dy;				// change in x and y position relative to sensor's orientation
	float vlon, vlat;		// absolute x/y motion
	float fov;				// field of view in Radians
	float scaler;			// number returned from sensor when moved one pixel
	int num_pixels;			// number of pixels of resolution in the sensor
	float vx_cm, vy_cm;

    uint8_t has_overflow;	// true if the x or y data buffers overflowed
	
	// bytes to store SPI settings
	uint8_t orig_spi_settings_spcr;
	uint8_t orig_spi_settings_spsr;
} ADNS_Sensor;

int ADNS_init(ADNS_Sensor *s);

void ADNS_reset(ADNS_Sensor *s);  

void ADNS_update(ADNS_Sensor *s);
void ADNS_update_position(ADNS_Sensor *s, float roll, float pitch, float yaw, float altitude);

void ADNS_clear_motion(ADNS_Sensor *s);	// ???


uint8_t ADNS_get_led_always_on(ADNS_Sensor *s);
void ADNS_set_led_always_on(ADNS_Sensor *s, uint8_t alwaysOn);

uint8_t ADNS_get_resolution(ADNS_Sensor *s);
void ADNS_set_resolution(ADNS_Sensor *s, uint8_t resolution);

uint8_t ADNS_get_frame_rate_auto(ADNS_Sensor *s);
void ADNS_set_frame_rate_auto(ADNS_Sensor *s, uint8_t auto_frame_rate);

uint8_t ADNS_get_serial_pullup(ADNS_Sensor *s);
void ADNS_set_serial_pullup(ADNS_Sensor *s, uint8_t pullup);

uint16_t ADNS_get_frame_period(ADNS_Sensor *s);

uint16_t ADNS_get_max_frame_period(ADNS_Sensor *s);
void ADNS_set_max_frame_period(ADNS_Sensor *s, uint16_t period);

uint16_t ADNS_get_min_frame_period(ADNS_Sensor *s);
void ADNS_set_min_frame_period(ADNS_Sensor *s, uint16_t period);

uint32_t ADNS_get_frame_rate(ADNS_Sensor *s);
void ADNS_set_frame_rate(ADNS_Sensor *s, uint32_t rate);

#endif

