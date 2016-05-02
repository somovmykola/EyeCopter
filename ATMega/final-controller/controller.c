#include "controller.h"

void initInputs(void) {
    // Set up RADIO_* pins as input
    DDRC &= ~_BV(DDC3);
    DDRD &= ~(_BV(DDD0) | _BV(DDD1) | _BV(DDD2));
    
    // Turn on internal pull-ups
    PORTC |= _BV(PORTC3);
    PORTD |= _BV(PORTD0) | _BV(PORTD1) | _BV(PORTD2);
    
    // save state of PORTD so that the pin change interrupt can decide which
    // pin was updated
    oldPORTD = PORTD;
    
    // Enable pin change interrupts
    PCICR |= _BV(PCIE1);    // Enable PCMSK1 scan
    PCMSK1 |= _BV(PCINT11); // RADIO_T
        
    PCICR |= _BV(PCIE2);    // Enable PCMSK2 scan
    PCMSK2 |= _BV(PCINT16); // RADIO_P
    PCMSK2 |= _BV(PCINT17); // RADIO_Y
    PCMSK2 |= _BV(PCINT18); // RADIO_R
}

void initOutputs(void) {
    // Set up CTRL_* as outputs
    DDRB |= _BV(DDB0);
    DDRD |= _BV(DDD5) | _BV(DDD6) | _BV(DDD7);
    
    // Clear outputs to zero
    PORTB &= ~_BV(PORTB0);
    PORTD &= ~(_BV(PORTD5) | _BV(PORTD6) | _BV(PORTD7));
}

void initTimer(void) {
    // Timer starts in compare mode to generate output signals.
    
    // Enable timer compare interrupt
    TIMSK1 |= _BV(OCIE1A);

    TCCR1B = PRSC_8 |       // Set  prescaler to 1/8
            _BV(WGM12);     // Set mode to Clear Timer on Compare
    
    TCNT1 = 0x0000;
    
    // Init timer1 compare register to a small value to quickly call interrupt
    OCR1A = 10;
    
    // Set initial state to right before the first signal is outputted
    ctrlState = syncT;
    
    // Enable timer compare interrupt
    TIMSK1 |= _BV(OCIE1A);
}

// Calculate parity of two bytes using modified version of the bitwise parity
// algorithm described here:
// http://www.graphics.stanford.edu/~seander/bithacks.html#ParityParallel
uint8_t getParity(uint8_t b1, uint8_t b2) {
    uint8_t v = b1 ^ b2;
    v ^= v >> 4;
    return (0x6996 >> (v & 0x0F)) & 0x01;
}

// Read through the incomming message and take appropriate action. 
// Returns 1 when an out message is supplied, 0 otherwise
uint8_t processMessage(uint8_t *inMsg, uint8_t *outMsg) {    
    uint8_t port = (inMsg[0] >> 4) & 0x03;
    
    if (inMsg[0] & 0x80) {
        // Message type is write
        
        // If incoming message has wrong parity, don't bother processing        
        if (getParity(inMsg[0], inMsg[1]) == 0)
            return 0;
        
        uint16_t len = 0x00;
        
        len |= ((uint16_t)inMsg[1] >> 1) & 0x007F;
        len |= ((uint16_t)inMsg[0] << 7) & 0x0780;
        
        ctrlLen[port] = len;
        
        return 0;
    } else {
        // Message type is read
        
        // If incoming message has wrong parity, don't bother processing. Note
        // that we only check first byte since second byte will contain data
        // from last write message
        if (getParity(inMsg[0], 0x00) == 0)
            return 0;
    
        uint8_t src = (inMsg[0] >> 6) & 0x01;
        
        if (src)    // Source is radio
            outMsg[0] = pulseLen[port];
        else        // Source is memory
            outMsg[0] = (ctrlLen[port] - 4*MIN_TICKS) >> 2;
        
        return 1;
    }
}

// Pin change interrupt corresponding solely to Pin C3 - RADIO_T
// A rising edge on the throttle port means that we must enable manual control
// mode. TIMER1 must be changed from capture compare mode to normal mode.
ISR(PCINT1_vect) {
    if (PINC & _BV(PORTC3)) {
        // If the timer is in Clear Timer on Compare (CTC) mode, reset it to
        // normal mode
        if (TCCR1B & _BV(WGM12)) {
            // Clear prescaler bits, also pauses clock
            TCCR1B &= ~(_BV(CS10) | _BV(CS11) | _BV(CS12));
            
            // Clear wave generation mode bits, sets TIMER0 to normal mode
            TCCR1B &= ~_BV(WGM12);
            
            // Disable compare interrupt
            TIMSK1 &= ~_BV(OCIE1A);
            
            // Enable overflow interrupt
            TIMSK1 |= _BV(TOIE1);
            
            // Set prescaler to 1/64 to measure radio input
            TCCR1B |= PRSC_64;
        }
        
        // save current clock tick for later comparison
        oldClk[throttle] = TCNT1;
        
        // Set the CTRL_T pin to mirror RADIO_T
        PORTD |= _BV(PORTD5);
    } else {
        // find difference
        uint16_t dt = TCNT1 - oldClk[throttle];
        
        // clamp difference
        if (dt > MAX_TICKS)
            dt = MAX_TICKS;
        if (dt < MIN_TICKS)
            dt = MIN_TICKS;
        
        // Save pulse length
        pulseLen[throttle] = dt - MIN_TICKS;
        
        // Set the CTRL_T pin to mirror RADIO_T
        PORTD &= ~_BV(PORTD5);
    }
}

// Pin change interrupt corresponding to Pins D0, D1, D2, corresponding to 
// RADIO_P, RADIO_Y, and RADIO_R respectively.
ISR(PCINT2_vect) {
    uint16_t dt;
    uint8_t change = PIND ^ oldPORTD;
    oldPORTD = PIND;
    
    // Pin D0 - Pitch
    if (change & _BV(PORTD0)) {
        if (PIND & _BV(PORTD0)) {
            // Rising Edge: save current clock tick for later comparison
            oldClk[pitch] = TCNT1;
            
            // Set the CTRL_P pin to mirror RADIO_P
            PORTD |= _BV(PORTD7);
        } else {
            // find difference
            dt = TCNT1 - oldClk[pitch];
            
            // clamp difference
            if (dt > MAX_TICKS)
                dt = MAX_TICKS;
            if (dt < MIN_TICKS)
                dt = MIN_TICKS;
            
            // Save pulse length
            pulseLen[pitch] = dt - MIN_TICKS;
            
            // Set the CTRL_P pin to mirror RADIO_P
            PORTD &= ~_BV(PORTD7);
        }
    }
    
    // Pin D1 - Yaw
    if (change & _BV(PORTD1)) {
        if (PIND & _BV(PORTD1)) {
            // Rising Edge: save current clock tick for later comparison
            oldClk[yaw] = TCNT1;
            
            // Set the CTRL_Y pin to mirror RADIO_Y
            PORTB |= _BV(PORTB0);
        } else {
            // find difference
            dt = TCNT1 - oldClk[yaw];
            
            // clamp difference
            if (dt > MAX_TICKS)
                dt = MAX_TICKS;
            if (dt < MIN_TICKS)
                dt = MIN_TICKS;
            
            // Save pulse length
            pulseLen[yaw] = dt - MIN_TICKS;
            
            // Set the CTRL_Y pin to mirror RADIO_Y
            PORTB &= ~_BV(PORTB0);
        }
    }
    
    // Pin D2 - Roll
    if (change & _BV(PORTD2)) {
        if (PIND & _BV(PORTD2)) {
            // Rising Edge: save current clock tick for later comparison
            oldClk[roll] = TCNT1;
            
            // Set the CTRL_R pin to mirror RADIO_R
            PORTD |= _BV(PORTD6);
        } else {
            // find difference
            dt = TCNT1 - oldClk[roll];
            
            // clamp difference
            if (dt > MAX_TICKS)
                dt = MAX_TICKS;
            if (dt < MIN_TICKS)
                dt = MIN_TICKS;
            
            // Save pulse length
            pulseLen[roll] = dt - MIN_TICKS;
            
            // Set the CTRL_R pin to mirror RADIO_R
            PORTD &= ~_BV(PORTD6);
        }
    }
}

// I think a overflow interrupt must be defined to ensure the clock counts, or 
// at least have it enabled.
// TODO: Test this shit 
ISR(TIMER1_OVF_vect) {

}

// Timer 1 compare interrupt. Used for generating output signals for CTRL_*
// Each channel is outputted one after another, with at least 2000 microseconds 
// between rising edges of different channels. In the event that a ctrlLen value
// is changed during it's pulse, the length will not update until its next
// rising edge. ie the remaining time between a falling edge and the next rising
// edge is computed and stored on the rising edge, not the falling edge.
ISR(TIMER1_COMPA_vect) {
    switch(ctrlState) {
    case pulseP:
        PORTD &= ~_BV(PORTD7);      // Turn off CTRL_P
        ctrlState = syncP;
        OCR1A = syncTime;
        break;
    
    case syncP:
        PORTB |= _BV(PORTB0);       // Turn on CTRL_Y
        ctrlState = pulseY;
        OCR1A = ctrlLen[yaw];
        syncTime = MAX_PULSE_TICKS - ctrlLen[yaw];
        break;
    
    case pulseY:
        PORTB &= ~_BV(PORTB0);      // Turn off CTRL_Y
        ctrlState = syncY;
        OCR1A = syncTime;
        break;
    
    case syncY:
        PORTD |= _BV(PORTD6);       // Turn on CTRL_R
        ctrlState = pulseR;
        OCR1A = ctrlLen[roll];
        syncTime = MAX_PULSE_TICKS - ctrlLen[roll];
        break;
    
    case pulseR:
        PORTD &= ~_BV(PORTD6);      // Turn off CTRL_R
        ctrlState = syncR;
        OCR1A = syncTime;
        break;
    
    case syncR:
        PORTD |= _BV(PORTD5);       // Turn on CTRL_T
        ctrlState = pulseT;
        OCR1A = ctrlLen[throttle];
        syncTime = PERIOD_TICKS - 3*MAX_PULSE_TICKS - ctrlLen[throttle];
        break;
        
    case pulseT:
        PORTD &= ~_BV(PORTD5);      // Turn off CTRL_T
        ctrlState = syncT;
        OCR1A = syncTime;
        break;
    
    case syncT:
        PORTD |= _BV(PORTD7);       // Turn on CTRL_P
        ctrlState = pulseP;
        OCR1A = ctrlLen[pitch];
        syncTime = MAX_PULSE_TICKS - ctrlLen[pitch];
        break;
    }
}

int main(void) {
    initInputs();
    initOutputs();
    initTimer();
    
    // Dummy init ctrl signals to make debugging easier
    ctrlLen[pitch]      = 1000;
    ctrlLen[yaw]        = 1326;
    ctrlLen[roll]       = 1568;
    ctrlLen[throttle]   = 1812;
        
    TWI_init();
    TWI_startTransceiver();
    
    // set global interrupt enable
    sei();
    
    // Create a buffer for input messages
    uint8_t rxMessage[RX_BUFFER_SIZE];
    uint8_t txMessage[TX_BUFFER_SIZE];
    
    for(;;) {
        if (!TWI_transceiverBusy() && TWI_statusReg.lastTransOK) {
            if (TWI_statusReg.rxDataInBuf) {
                TWI_getData(rxMessage, RX_BUFFER_SIZE);
                
                // Proccess message, queue up output if message needs it
                if (processMessage(rxMessage, txMessage))
                    TWI_startTransceiverWithData(txMessage, TX_BUFFER_SIZE);
            }
        }
    }
}