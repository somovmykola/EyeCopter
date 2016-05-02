//sets up control from transmitter to the flight controller through the pi.
//edges are controlled via interrupts on ever active pin.

#include <wiringPi.h>
#include <time.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>

#define PIN_P_IN    6
#define PIN_Y_IN    24
#define PIN_R_IN    25
#define PIN_T_IN    23

#define PIN_P_OUT   18
#define PIN_Y_OUT   17
#define PIN_R_OUT   27
#define PIN_T_OUT   22

int setup(void) {
    if (gpioInitialise() < 0)
        return 1;
  
    gpioSetMode(PIN_P_IN, PI_INPUT);
    gpioSetMode(PIN_Y_IN, PI_INPUT);
    gpioSetMode(PIN_R_IN, PI_INPUT);
    gpioSetMode(PIN_T_IN, PI_INPUT);

    gpioSetMode(PIN_P_OUT, PI_OUTPUT);
    gpioSetMode(PIN_Y_OUT, PI_OUTPUT);
    gpioSetMode(PIN_R_OUT, PI_OUTPUT);
    gpioSetMode(PIN_T_OUT, PI_OUTPUT);

    gpioSetPullUpDown(PIN_P_IN, PI_PUD_OFF);
    gpioSetPullUpDown(PIN_Y_IN, PI_PUD_OFF);
    gpioSetPullUpDown(PIN_R_IN, PI_PUD_OFF);
    gpioSetPullUpDown(PIN_T_IN, PI_PUD_OFF);
    gpioSetPullUpDown(PIN_P_OUT, PI_PUD_OFF);
    gpioSetPullUpDown(PIN_Y_OUT, PI_PUD_OFF);
    gpioSetPullUpDown(PIN_R_OUT, PI_PUD_OFF);
    gpioSetPullUpDown(PIN_T_OUT, PI_PUD_OFF);
    
    return 0;
}

void edgeCallback(int pinNum, int level, uint32_t tick) {
    switch(pinNum) {
    case PIN_P_IN:
        gpioWrite(PIN_P_OUT, level);
        break;
    case PIN_Y_IN:
        gpioWrite(PIN_Y_OUT, level);
        break;
    case PIN_R_IN:
        gpioWrite(PIN_R_OUT, level);
        break;
    case PIN_T_IN:
        gpioWrite(PIN_T_OUT, level);
        break;
    }
}

int main(void) {
    setup();
  
    gpioSetAlertFunc(PIN_P_IN, edgeCallback);
    gpioSetAlertFunc(PIN_Y_IN, edgeCallback);
    gpioSetAlertFunc(PIN_R_IN, edgeCallback);
    gpioSetAlertFunc(PIN_T_IN, edgeCallback);
  
    for (;;) {
        sleep(10);
    }
    
    gpioTerminate();
    
    return 0;
}
