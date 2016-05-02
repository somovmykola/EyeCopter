#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ncurses.h>

#include "ADNS_Sensor.h"
#include "pins.h"

#include <pigpio.h>
#include <pthread.h>

#define SAMPLE_RATE 5   // microseconds

ADNS_Sensor sensor;


void edge(int pinNum, int level, uint32_t tick) {
    static uint32_t lastT;

    if (level == 1) {
        lastT = tick;
    } else  {
	float dist = 0.017449*((float)(tick - lastT) + 191.335);
        mvprintw(0, 0, "Front sensor reading: %5.0fcm\n", dist);

        refresh();
    }
    //dist = (time + 191.3)/57.3;
}

void edge2(int pinNum, int level, uint32_t tick) {
    static uint32_t lastT;

    if (level == 1) {
        lastT = tick;
    } else {
	float dist = 0.017449*((float)(tick - lastT) + 191.335);
        mvprintw(1, 0, "Down sensor reading:  %5.0fcm\n", dist);
        refresh();
    }
    //dist = (time + 191.3)/57.3;
}

void trigger(){

    while(true) {
       // c = getchar();

        //if (c != 'x')   continue;

        // Write a trigger pulse
        gpioWrite(BCM_TRIG, 1);
        gpioDelay(20);
        gpioWrite(BCM_TRIG, 0);

        gpioDelay(1000000);    // Wait for echo pulse
    }
}

void cleanup() {
    endwin(); //end ncurses

    gpioTerminate();

}

int main(int argc, char *argv[]) {
    gpioCfgClock(SAMPLE_RATE, 1, 1);

    if (ADNS_init(&sensor) < 0)
        fprintf(stderr, "Failed to initialize ADNS3080\n");

    delay(200);

    pthread_t thread1;

    if (gpioInitialise() < 0)
        return 1;



    gpioSetAlertFunc(BCM_ECHO1, edge);
    gpioSetAlertFunc(BCM_ECHO2, edge2);

    gpioSetMode(BCM_ECHO1, PI_INPUT);
    gpioSetMode(BCM_ECHO2, PI_INPUT);
    gpioSetMode(BCM_TRIG, PI_OUTPUT);

    initscr(); //start ncruses

    // reset trigger pin
    gpioWrite(BCM_TRIG, 0);

    //char c = 0;
    if(pthread_create(&thread1, NULL, &trigger, NULL)!=0){
         mvprintw(1,0,"FAILURE TO CREATE TRIGGER THREAD");
         endwin();
         return 1;
    }

    atexit(cleanup);

    while (1) {
        ADNS_update(&sensor);
        //ADNS_update_position(&sensor, 0, 0, 0, 100);

        // check for errors
        if (sensor.has_overflow)
            printf("overflow!!\n");

        // TODO: make these numbers fixed length so it looks pretty
        // when outputting. Maybe remove x and only keep DX
        //printf("\r");

         //optical row
        mvprintw(3, 0, "Optical sensor readings: dx: %4d    dy: %4d    surface quality: %4d\n", sensor.dx, sensor.dy, sensor.surface_quality);            

        //mvprintw(1,0,"Sensor 2 detects this distance: %lu\n",((tick - lastT)+191300)/57300);
        //fflush(stdout);
        refresh();

        delay(500); // ms
    }
	
	cleanup();
	
    return 0;
}
