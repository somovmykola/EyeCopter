#include <avr/io.h>
#include <avr/interrupt.h>

// Since the clock speed is 16MHz, and the timer pre-scaler is 1/8,
// TWO clock ticks equal ONE microsecond.

#define MAX_PULSE_TICKS		4000
#define PERIOD_TICKS		44000

volatile uint16_t delays[4];
volatile uint16_t syncTime;

typedef enum {
	pitch 		= 0b00,
	yaw			= 0b01,
	roll		= 0b10,
	throttle	= 0b11
} signalcode_t;

typedef enum {
    pulseP, syncP,
    pulseY, syncY,
    pulseR, syncR,
    pulseT, syncT
} outstate_t;

outstate_t outState;

void initGPIO(void) {
    DDRB |= (1 << DDB0);	// B0 - CTRL_Y
    DDRD |= (1 << DDD7);	// D7 - CTRL_P
    DDRD |= (1 << DDD6);	// D6 - CTRL_R
    DDRD |= (1 << DDD5);	// D5 - CTRL_T
	
	// Clear outputs all to zero
	PORTB &= ~(1 << PORTB0);
	PORTD &= ~(1 << PORTD5);
	PORTD &= ~(1 << PORTD6);
	PORTD &= ~(1 << PORTD7);
}

void initTimer(void) {    
    // Enable timer compare interrupt
    TIMSK1 |= (1 << OCIE1A);

    TCCR1B = (1 << CS11) |  // Set  prescaler to 1/8
             (1 << WGM12);  // Set mode to Clear Timer on Compare

    TCNT1 = 0x00;

    // Init compare value to a small number so that 
    OCR1A = 10;
}

ISR(TIMER1_COMPA_vect) {
    switch(outState) {
    case pulseP:
		PORTD &= ~(1 << PORTD7);		// Turn off CTRL_P
        outState = syncP;
        OCR1A = syncTime;
        break;
    
    case syncP:
        PORTB |= (1 << PORTB0);			// Turn on CTRL_Y
        outState = pulseY;
        OCR1A = delays[yaw];
        syncTime = MAX_PULSE_TICKS - delays[yaw];
        break;
    
    case pulseY:
		PORTB &= ~(1 << PORTB0);		// Turn off CTRL_Y
        outState = syncY;
        OCR1A = syncTime;
        break;
    
    case syncY:
        PORTD |= (1 << PORTD6);			// Turn on CTRL_R
        outState = pulseR;
        OCR1A = delays[roll];
        syncTime = MAX_PULSE_TICKS - delays[roll];
        break;
    
    case pulseR:
		PORTD &= ~(1 << PORTD6);		// Turn off CTRL_R
        outState = syncR;
        OCR1A = syncTime;
        break;
    
    case syncR:
        PORTD |= (1 << PORTD5);			// Turn on CTRL_T
        outState = pulseT;
        OCR1A = delays[throttle];
        syncTime = PERIOD_TICKS - 3*MAX_PULSE_TICKS - delays[throttle];
		break;
        
    case pulseT:
		PORTD &= ~(1 << PORTD5);		// Turn off CTRL_T
        outState = syncT;
        OCR1A = syncTime;
        break;
    
    case syncT:
        PORTD |= (1 << PORTD7);			// Turn on CTRL_P
        outState = pulseP;
        OCR1A = delays[pitch];
        syncTime = MAX_PULSE_TICKS - delays[pitch];
        break;
    }
}

int main(void) {
    initGPIO();

    delays[pitch] = 2000;
    delays[yaw] = 2000;
    delays[roll] = 2000;
    delays[throttle] = 2000;
    
	// Set initial state to right before the first signal is outputted
    outState = syncT;
    
    initTimer();

    // set global interrupt enable
    sei();

    for(;;) {}
}
