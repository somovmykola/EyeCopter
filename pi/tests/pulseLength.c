#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <pigpio.h>

#include "pins.h"

#define SAMPLE_RATE 5   // microseconds

void edge(int pinNum, int level, uint32_t tick) {
    static uint32_t lastT;

    if (level == 1) {
        lastT = tick;
    } else {
	uint32_t len = tick - lastT;
	float dist = 0.017449*((float)len + 191.335);
        printf("\rPulse length = %6lu", len);
	printf(", distance = %3dcm", (int)dist);
	fflush(stdout);
    }
}

int main(int argc, char *argv[]) {
    gpioCfgClock(SAMPLE_RATE, 1, 1);

    if (gpioInitialise() < 0)
        return 1;

    gpioSetAlertFunc(BCM_ECHO2, edge);

    gpioSetMode(BCM_ECHO2, PI_INPUT);
    gpioSetMode(BCM_TRIG, PI_OUTPUT);

    gpioSetPullUpDown(BCM_ECHO1, PI_PUD_OFF);
    gpioSetPullUpDown(BCM_ECHO2, PI_PUD_OFF);
    gpioSetPullUpDown(BCM_TRIG, PI_PUD_OFF);

    // reset trigger pin
    gpioWrite(BCM_TRIG, 0);

    char c = 0;
    while(c != 'q') {
//        c = getchar();

//        if (c != 'x')   continue;

        // Write a trigger pulse
        gpioWrite(BCM_TRIG, 1);
        gpioDelay(20);
        gpioWrite(BCM_TRIG, 0);

        gpioDelay(500000);    // Wait for echo pulse
    }

    gpioTerminate();

    return 0;
}
