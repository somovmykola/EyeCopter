### Usage:

* `ADNS_Sensor`: object that stores file descriptor to optical sensor as well as
details about the sensor such as field of view and number of pixels per sample.
Also keeps track of past readings in order to estimate change in X/Y.

* `ADNS_init()`: must be called before any other operation. Initializes sensor's
file descriptor and sets SPI read/write mode states. Returns 0 on success, and
a negative value on failure. Assumes that the device is connected on the first
SPI channel, ie /dev/spidev0.0

* `ADNS_update()`: Reads the raw dx and dy values from the optical sensor as well
as the sensor's estimation of surface quality. 

* `ADNS_update_position()`: Mostly a hold over from library used as reference for
setting up the ADNS3080 sensor. Updates the `ADNS_Sensor`'s internal longitude
and latitude with an estimation based on the raw dx and dy values read from the
sensor. It also attempts to correct for the orientation of the sensor, given as
roll, pitch, and yaw values as well as the sensor's altitude.


#### Surface Quality:
Surface quality is a value stored on the sensor and represents an estimate of
the accuracy of the dx/dy measures. Surface quality (squal) is a measure of the
number of valid features visible by the sensor in the current frame. During 
normal operation, a squal value greater than 15 is usually sufficient for
accurate readings.

#### Reference:
See [ADNS3080.pdf](../../Datasheets/ADNS3080.pdf) in the datasheets folder.