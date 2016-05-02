#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <string.h> 
#include "i2c.h"

// Pre-scaler macros
#define PRSC_8      _BV(CS11)               // used to generate ctrl signals
#define PRSC_64     _BV(CS11)|_BV(CS10)     // used to measure radio signals
#define PRSC_1024   _BV(CS12)|_BV(CS10)

// Bounds on radio input signals
#define MIN_TICKS   222     //  888 uS at 1/64
#define MAX_TICKS   477     // 1908 uS at 1/64



enum {
    pitch = 0,
    yaw,
    roll,
    throttle
} portcode;


volatile uint8_t oldPORTD;      // For RADIO_P, Y, and R
volatile uint16_t oldClk[4];
volatile uint8_t pulseLen[4];

void initGPIO(void) {
    // Set B0 to output
    DDRB |= _BV(DDB0);	
	// Clear output to zero
	PORTB &= ~_BV(PORTB0);
     
    
    // Set up RADIO_* pins as input
    DDRC &= ~_BV(DDC3);
    DDRD &= ~(_BV(DDD0) | _BV(DDD1) | _BV(DDD2));    
    // Turn on internal pull-ups
    PORTC |= _BV(PORTC3);
    PORTD |= _BV(PORTD0) | _BV(PORTD1) | _BV(PORTD2);
    
    oldPORTD = PORTD;
    
    PCICR |= _BV(PCIE1);    // Enable PCMSK1 scan
    PCMSK1 |= _BV(PCINT11);
        
    PCICR |= _BV(PCIE2);    // Enable PCMSK2 scan
    PCMSK2 |= _BV(PCINT16); // RADIO_P
    PCMSK2 |= _BV(PCINT17); // RADIO_Y
    PCMSK2 |= _BV(PCINT18); // RADIO_R
}

void initTimer(void) {
    // Timer starts in compare mode to generate output signals.
    
    // Enable timer compare interrupt
    TIMSK1 |= _BV(OCIE1A);

    TCCR1B = PRSC_8 |       // Set  prescaler to 1/8
             _BV(WGM12);    // Set mode to Clear Timer on Compare
    
    TCNT1 = 0x0000;
    
    // Init timer1 compare register
    OCR1A = 15625;
    
    // Enable timer compare interrupt
    TIMSK1 |= _BV(OCIE1A);
    
    // Enable overflow interrupt
    // TIMSK1 |= _BV(TOIE1);
}

ISR(TIMER1_COMPA_vect) {
    PORTB ^= _BV(PORTB0);
}

ISR(TIMER1_OVF_vect) {

}

ISR(PCINT1_vect) {
    // Pin C3 - Throttle
    if (PINC & _BV(PORTC3)) {
        // A rising edge on the throttle port means that we are now in manual
        // control. TIMER1 must be changed from capture compare mode to normal
        // mode.
        
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
    }
}

ISR(PCINT2_vect) {
    uint16_t dt;
    uint8_t change = PIND ^ oldPORTD;
    oldPORTD = PIND;
    
    // Pin D0 - Pitch
    if (change & _BV(PORTD0)) {
        if (PIND & _BV(PORTD0)) {
            // Rising Edge: save current clock tick for later comparison
            oldClk[pitch] = TCNT1;
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
        }
    }
    
    // Pin D1 - Yaw
    if (change & _BV(PORTD1)) {
        if (PIND & _BV(PORTD1)) {
            // Rising Edge: save current clock tick for later comparison
            oldClk[yaw] = TCNT1;
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
        }
    }
    
    // Pin D2 - Roll
    if (change & _BV(PORTD2)) {
        if (PIND & _BV(PORTD2)) {
            // Rising Edge: save current clock tick for later comparison
            oldClk[roll] = TCNT1;
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
        }
    }
}

int main(void) {
    initGPIO();
    initTimer();
    
    PORTB &= ~_BV(PORTB0);
    
    memset(pulseLen, 0, 4);
    
    TWI_init();
    TWI_startTransceiver();

    // set global interrupt enable
    sei();

    for(;;) {
        if (!TWI_transceiverBusy()/* && TWI_statusReg.lastTransOK*/) {
            TWI_startTransceiverWithData(&pulseLen[throttle], 1);
            _delay_ms(50);
        }
    }
}
