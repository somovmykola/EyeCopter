#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <pigpio.h>

#define SAMPLE_RATE 5   // microseconds

#define PIN_TRIG    3
#define PIN_ECHO2   8
#define PIN_ECHO    9


void edge(int pinNum, int level, uint32_t tick) {
    static uint32_t lastT;

    if (level == 1) {
        lastT = tick;
    } else {
        printf("distance = %lu\n", ((tick - lastT)+191300)/57300);
    }
    //dist = (time + 191.3)/57.3;
}

int main(int argc, char *argv[]) {
    gpioCfgClock(SAMPLE_RATE, 1, 1);

    if (gpioInitialise() < 0)
        return 1;

    gpioSetAlertFunc(PIN_ECHO, edge);
    gpioSetAlertFunc(PIN_ECHO2, edge);

    gpioSetMode(PIN_ECHO, PI_INPUT);
    gpioSetMode(PIN_TRIG, PI_OUTPUT);

    // reset trigger pin
    gpioWrite(PIN_TRIG, 0);

    char c = 0;
    while(c != 'q') {
        c = getchar();

        if (c != 'x')   continue;

        // Write a trigger pulse
        gpioWrite(PIN_TRIG, 1);
        gpioDelay(20);
        gpioWrite(PIN_TRIG, 0);

        gpioDelay(1000000);    // Wait for echo pulse
    }

    gpioTerminate();

    return 0;
}
