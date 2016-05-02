#ifndef I2C_TEST_H
#define I2C_TEST_H

#include <util/twi.h> 

#define TWI_ADDRESS     0x69

#define RX_BUFFER_SIZE  2
#define TX_BUFFER_SIZE  1

#define TWI_BUFFER_SIZE 2       // MAX(RX, TX)

union TWI_statusReg_t {                     // Status byte holding flags.
    unsigned char all;
    struct {
        unsigned char lastTransOK:1;      
        unsigned char rxDataInBuf:1;
        unsigned char genAddressCall:1;     // TRUE = General call, FALSE = TWI Address;
        unsigned char unusedBits:5;
    };
};
union TWI_statusReg_t TWI_statusReg;


void TWI_init(void);

uint8_t TWI_transceiverBusy(void);

void TWI_startTransceiver(void);
void TWI_startTransceiverWithData(uint8_t *msg, uint8_t size);

uint8_t TWI_getData(uint8_t *msg, uint8_t size);

#endif