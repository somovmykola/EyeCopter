__blink__ - Bare-bones test program to turn on and off an LED after a set number of milliseconds

__i2c-test__ - Basic implementation of I2C protocol, aka TWI on the ATMega. Doesn't use final write/read request protocol but instead a highly simplified protocol where a specific one byte message turns an LED on, and a different one byte message turns an LED off. When either of these bytes is received, the microcontroller returns a byte corresponding to the current state of the LED. In later projects, all the I2C backend code is contained in the files i2c.c and i2c.h

__timer-test__ - First use of 16bit timer. Generates CTRL_* signals using a state machine. TIMER1 is configured to Clear Timer on Compare (CTC) to fire an interrupt after a specified number of clock ticks. Generated signals are precise to 0.5 microseconds.

__pcint-test__ - Configures RADIO_* pins as inputs, uses Pin Change INTerrupts and the 16 bit timer to measure the length of incomming pulses. In this test program, it compares the length of pulses against a constant value to decide whether or not to turn on an LED.

__final-controller__ - More or less a combination of the previous three. Generates CTRL_* signals from internal memory. Listens on I2C for commands to change these internal memory values. If a pulse is detected on the RADIO_T input, the device is switched to manual control mode. Once in manual control mode, the device may not switch back until restarted. In manual control mode, CTRL_* signals are mirrored from their corresponding RADIO_* pins. The clock is also switched from CTC mode to Normal mode and the prescaler is switched from 1/8 to 1/64 in order to store the length of a RADIO_* pulse in 8 bits. The values of the RADIO_* pulses as well as the internal CTRL_* memory values may also be read via I2C.

For more information on the convention for passing I2C messages, see [PROTOCOL.md](final-controller/PROTOCOL.md)