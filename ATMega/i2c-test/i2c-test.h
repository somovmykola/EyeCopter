#ifndef I2C_TEST_H
#define I2C_TEST_H

#include <util/twi.h> 

#define TWI_ADDRESS     0x69

#define INPUT_LED_ON    0x12
#define INPUT_LED_OFF   0x34

#define OUTPUT_LED_ON   0x01
#define OUTPUT_LED_OFF  0x00

#define RX_BUFFER_SIZE  1
#define TX_BUFFER_SIZE  1

#define MAX(x, y)   (((x)>(y))?(x):(y))

#define TWI_BUFFER_SIZE MAX(RX_BUFFER_SIZE, TX_BUFFER_SIZE)

#endif