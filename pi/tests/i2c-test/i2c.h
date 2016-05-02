#ifndef MY_I2C_H
#define MY_I2C_H

#include <stdint.h>

#define ATMEGA_I2C_ADDRESS 0x69

int i2c_init(void);
void i2c_close(int fd);

int i2c_writeBytes(int fd, uint8_t *data, uint8_t nBytes);
int i2c_readBytes(int fd, uint8_t *data, uint8_t nBytes);
int i2c_requestBytes(int fd, uint8_t *outData, uint8_t nOut, uint8_t *inData, uint8_t nIn);

#endif
