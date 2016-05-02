#define F_CPU 8000000UL

#include <avr/io.h>
#include <util/delay.h>

#define ASDF PORTB0

int main(void) {
    DDRB |= (1 << DDB0);            //Set the 6th bit on PORTB (i.e. PB5) to 1 => output
    
    while(1) {
        PORTB |= (1 << ASDF);
        _delay_ms(1000);
        PORTB &= ~(1 << ASDF);
        _delay_ms(1000);
    }
}
