#include "i2c.h"
#include <stdio.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>

int i2c_init(void) {
    int fd = open("/dev/i2c-1", O_RDWR);

    if(fd < 0)
        fprintf(stderr, "Could not open i2c device\n");

    if (ioctl(fd, I2C_SLAVE, ATMEGA_I2C_ADDRESS) < 0)
        fprintf(stderr, "Failed to acquire bus access and/or talk to slave.\n");

    return fd;
}

void i2c_close(int fd) {
    close(fd);
}

int i2c_writeBytes(int fd, uint8_t *data, uint8_t nBytes){
    struct i2c_msg msgs[1];
    msgs[0].addr = ATMEGA_I2C_ADDRESS;
    msgs[0].flags = 0;
    msgs[0].len = nBytes;
    msgs[0].buf = data;

    struct i2c_rdwr_ioctl_data packets;
    packets.msgs = msgs;
    packets.nmsgs = 1;

    int ret = ioctl(fd, I2C_RDWR, &packets);

    if (ret < 0)
        fprintf(stderr, "Write to I2C device failed!\n");

    return ret;
}

int i2c_readBytes(int fd, uint8_t *data, uint8_t nBytes){
    if (nBytes > 1)
        fprintf(stderr, "Can't read more than one byte yet. Kinda stupid, I know\n");

    struct i2c_msg msgs[1];
    msgs[0].addr = ATMEGA_I2C_ADDRESS;
    msgs[0].flags = I2C_M_RD;
    msgs[0].len = nBytes;
    msgs[0].buf = data;

    struct i2c_rdwr_ioctl_data packets;
    packets.msgs = msgs;
    packets.nmsgs = 1;

    int ret = ioctl(fd, I2C_RDWR, &packets);
    if (ret < 0)
        fprintf(stderr, "Read from I2C device failed\n");

    return ret;
}

int i2c_requestBytes(int fd, uint8_t *outData, uint8_t nOut, uint8_t *inData, uint8_t nIn) {
    if (nIn > 1)
        fprintf(stderr, "Can't read more than one byte yet. Kinda stupid, I know\n");

    struct i2c_msg msgs[2];
    msgs[0].addr = ATMEGA_I2C_ADDRESS;
    msgs[0].flags = 0;
    msgs[0].len = nOut;
    msgs[0].buf = outData;

    msgs[1].addr = ATMEGA_I2C_ADDRESS;
    msgs[1].flags = I2C_M_RD;
    msgs[1].len = nIn;
    msgs[1].buf = inData;

    struct i2c_rdwr_ioctl_data packets;
    packets.msgs = msgs;
    packets.nmsgs = 2;

    int ret = ioctl(fd, I2C_RDWR, &packets);
    if(ret < 0)
        fprintf(stderr, "Read from I2C device failed\n");

    return ret;
}
