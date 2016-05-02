#ifndef MAKE_I2C_MESSAGE_H
#define MAKE_I2C_MESSAGE_H

#include <stdint.h>

uint8_t parityFromByte(uint8_t byte);
uint8_t parityFromWord(uint16_t word);

uint8_t makeReadMsg(uint8_t src, uint8_t port);
uint16_t makeWriteMsg(uint8_t port, uint16_t pulseLen);

#endif