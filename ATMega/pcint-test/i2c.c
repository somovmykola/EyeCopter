#include <avr/interrupt.h>
#include <util/delay.h>
#include "i2c.h"

static uint8_t TWI_buf[TWI_BUFFER_SIZE];
static uint8_t TWI_msgSize = 0;

static volatile uint8_t TWI_busy;

/**
 * Set up the TWI slave to its initial standby state.
 * -------------------------------------------------------------------------- */
void TWI_init(void) {
    // Set slave address. Address is 7 bits and stored in the 7 most siginificant
    // bits of the Twin Wire Address Register
    TWAR = TWI_ADDRESS << 1;
    
    TWCR = (1 << TWEN)|                             // Enable TWI interface
           (0 << TWIE)|(0 << TWINT)|                // Disable TWI interrupt
           (0 << TWEA)|(0 << TWSTA)|(0 << TWSTO)|   // Do not ACK any requests, yet
           (0 << TWWC);                             //
           
    TWI_busy = 0;
}

/**
 * Test if the TWI_ISR is busy transmitting or receiving
 * -------------------------------------------------------------------------- */
uint8_t TWI_transceiverBusy(void) {
    return TWI_busy;

    // Returns Control Register bit 0 - TWI Interrupt Enable
    //return TWCR & (1 << TWIE);
}

/**
 * Put transceiver in passive mode
 * -------------------------------------------------------------------------- */
void TWI_passiveTransciever_NB(void) {
    TWCR = (1<<TWEN)|                          // Enable TWI-interface and release TWI pins
           (0<<TWIE)|(0<<TWINT)|               // Disable Interupt
           (0<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|    // Do not acknowledge on any new requests.
           (0<<TWWC);
}

/**
 * Start transceiver without blocking/waiting for next transmission
 * -------------------------------------------------------------------------- */ 
void TWI_startTransceiver_NB(void) {
    TWCR = (1 << TWEN)|                             // Enable TWI interface
           (1 << TWIE)|(1 << TWINT)|                // Enable TWI interrupt and clear the flag
           (1 << TWEA)|(0 << TWSTA)|(0 << TWSTO)|   // Prepare to ACK next time this device is addressed
           (0 << TWWC);                             //
}

/**
 * Start the I2C transceiver. Useful for restarting a transmission or just
 * starting the transceiver for reception. The driver will reuse the data
 * previously stored in the transceiver buffers. The function will hold execution
 * until the TWI_ISR has completed with the previous operation, then init the
 * next operation and return.
 * -------------------------------------------------------------------------- */
void TWI_startTransceiver(void) {
    // Wait until TWI is ready for next transmission
    while (TWI_transceiverBusy());
    
    TWI_statusReg.all = 0;
    
    TWI_startTransceiver_NB();
    
    TWI_busy = 0;
}

/**
 * Same as above but also queues up datat for output. Not sure if this is needed
 * for a slave device
 * -------------------------------------------------------------------------- */
void TWI_startTransceiverWithData(uint8_t *msg, uint8_t size) {    
    // Wait until TWI is ready for next transmission
    while (TWI_transceiverBusy());
        
    // Copy data that may be transmitted if master requests data.
    TWI_msgSize = size;
    for (uint8_t i = 0; i < size; i++)
        TWI_buf[i] = msg[i];
        
    TWI_statusReg.all = 0;
    
    TWI_startTransceiver_NB();
    
    TWI_busy = 1;
}


/**
 * Read out the received data from the TWI transceiver buffer. The function will
 * hold execution until the TWI_ISR has completed with the previous operation
 * before reading out the data and returning. If there was an error in the 
 * previous transmission the function will return the TWI State code.
 *
 * This function seems to be kind of useless and more of an artifact of someone
 * trying to over-develop a TWI/I2C library. The buffer that this gets read into
 * (ie rxMessage) can be combined with the TWI_buf. The control state variables
 * lastTransOK and rxDataInBuf may be useful though.
 * -------------------------------------------------------------------------- */
uint8_t TWI_getData(uint8_t *msg, uint8_t size) {
    // Wait until TWI is ready
    while (TWI_transceiverBusy());
    
    if (TWI_statusReg.lastTransOK) {
        for (uint8_t i = 0; i < size; i++)
            msg[i] = TWI_buf[i];
            
        TWI_statusReg.rxDataInBuf = 0;
    }
    
    return TWI_statusReg.lastTransOK;
}

ISR(TWI_vect, ISR_BLOCK) {
    static uint8_t TWI_bufPtr = 0;
    
    switch(TW_STATUS) {
    // Own SLA+R has been received; ACK has been returned
    case TW_ST_SLA_ACK:
        // Reset buffer pointer
        TWI_bufPtr   = 0;
    
    // Data byte in TWDR has been transmitted; ACK has been received    
    case TW_ST_DATA_ACK:
        //PORTB ^= _BV(PORTB0);
        TWDR = TWI_buf[TWI_bufPtr++];
    
        TWI_startTransceiver_NB();
        TWI_busy = 1;
        break;
    
    // Data byte in TWDR has been transmitted; NACK has been received.
    // this could be the end of the transmission.
    case TW_ST_DATA_NACK:
        // Check if we've transceived all the expected data
        if (TWI_bufPtr == TWI_msgSize) {
            // Record successful transmission
            TWI_statusReg.lastTransOK = 1;
        }
        
        // Put TWI Transceiver in passive mode.
        TWI_startTransceiver_NB();
        TWI_busy = 0;
        break;
    
    // General call address has been received; ACK has been returned
    case TW_SR_GCALL_ACK:
        TWI_statusReg.genAddressCall = 1;
    // Own SLA+W has been received ACK has been returned
    case TW_SR_SLA_ACK:
        TWI_statusReg.rxDataInBuf = 1;
        TWI_bufPtr = 0;
        
        // Reset the TWI Interupt to wait for a new event.
        TWI_startTransceiver_NB();
        TWI_busy = 1;
        break;
        
    // Previously addressed with general call; data has been received; ACK has been returned
    case TW_SR_GCALL_DATA_ACK:
    // Previously addressed with own SLA+W; data has been received; ACK has been returned
    case TW_SR_DATA_ACK:
        TWI_buf[TWI_bufPtr++] = TWDR;
        TWI_statusReg.lastTransOK = 1;
        
        // Reset the TWI Interupt to wait for a new event.
        TWI_startTransceiver_NB();
        TWI_busy = 1;
        break;
        
    // A STOP condition or repeated START condition has been received while still addressed as Slave
    case TW_SR_STOP:
        // Put TWI Transceiver in passive mode.
        TWI_startTransceiver_NB();    
        TWI_busy = 0;
        break;
        
    // Previously addressed with own SLA+W; data has been received; NOT ACK has been returned
    case TW_SR_DATA_NACK:
    // Previously addressed with general call; data has been received; NOT ACK has been returned
    case TW_SR_GCALL_DATA_NACK:
    // Last data byte in TWDR has been transmitted; ACK has been received
    case TW_ST_LAST_DATA:
    // Bus error due to an illegal START or STOP condition
    case TW_BUS_ERROR:
        TWCR = (0<<TWEN)|                          // Disable TWI-interface and release TWI pins
               (0<<TWIE)|(1<<TWINT)|               // Disable Interupt
               (0<<TWEA)|(0<<TWSTA)|(1<<TWSTO)|    // Do not acknowledge on any new requests. Set STOP byte.
               (0<<TWWC);
        break;
    default:
        TWCR = (1<<TWEN)|                          // Enable TWI-interface and release TWI pins
               (1<<TWIE)|(1<<TWINT)|               // Keep interrupt enabled and clear the flag
               (1<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|    // Acknowledge on any new requests.
               (0<<TWWC);
        break;
    }
}
