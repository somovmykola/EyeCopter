//this file runs both the control system and the 
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <pigpio.h>

#include <stdbool.h>

#include <math.h>
#include <time.h>

#include "i2c.h"
#include "MakeI2CMessage.h"
#include "ADNS_Sensor.h"
#include "pins.h"
#include "AutonomousFlightFinal.h"
#define M_PI 3.14159265358979323846

int val = 0;

void InitQCControls() {
    currTime = 0.0;
    lastTime = 0.0;
    // !!!: This variable is never initialized
    neutralThrottle = 0.15; //fix this

    errAccY = 0.0;
    lastErrY = 0.0;

    lastErrF = 0.0;
    errAccF = 0.0;

    lastErrVX = 0.0;
    errAccVX = 0.0;

    lastErrVZ = 0.0;
    errAccVZ = 0.0;

    goalYaw = 0; //FIX THIS
    errYaw = 0.0;
    lastErrYaw = 0.0;
    errAccYaw = 0.0;

    lastState = Hover;
} 

// DT is in milliseconds!

void *control(){
    InitQCControls();
    struct timeval tv;
    gettimeofday(&tv, NULL);
    lastTime = 1000000 * tv.tv_sec + tv.tv_usec;

    while (1) {
        gettimeofday(&tv, NULL);
        currTime = 1000000 * tv.tv_sec + tv.tv_usec;
        double dt = currTime-lastTime;
        
        printf("dt = %.5f\n", dt);

        //double currVel.z = qc->GetVelocity().z;
        //double currVel.x = qc->GetVelocity().x;
        pthread_mutex_lock(&lockOpt);
        double realVelX = currVel.x*cos(currAng.r);
        double realVelZ = currVel.z*cos(currAng.p);
        pthread_mutex_unlock(&lockOpt);

        double goalY;
        double goalZ;
        double goalVelX;
        double goalVelZ;

        //get state of quadcopter operations
	// char state;
	// if (currTime > 320) {
	// 	state = Off;
	// }
	// else if (currTime > 300) {
	// 	state = Land;
	// }
	// else if (currTime <= 10) {
	// 	state = Hover;
	// }
	// else if (currZ >= 5.0){
	// 	state = Turn;
	// }
	// else if ((lastState == 'D') && (currZ>=lastDist)) {
	// 	state = Turn;
	// }
	// else if (currYaw !=lastYaw) {
	// 	state = Hover;
	// }
	// else {
	// 	state = MoveRight;
	// }
        state = Hover;  // ???: Why are we setting the state in the main loop?

        //set reference points based on state for PID controllers
        switch (state) {
            case Hover:
                goalY = 0.7;
                goalZ = 2.0;
                goalVelX = 0.0;
                goalVelZ = 0.0;
                break;
            case MoveRight:
                goalY = 0.7;
                goalZ = 2.0;
                goalVelX = 0.1;
                goalVelZ = 0.0;
                break;
            case Land:
                goalY = 0.0;
                goalZ = 0.0;
                goalVelX = 0.0;
                goalVelZ = 0.0;
                break;
            case Turn:
                goalY = 0.7;
                goalZ = 0.0;
                goalVelX = 0.0;
                goalVelZ = 0.0;
                break;
            case Off:
                goalY = 0.0;
                goalZ = 0.0;
                goalVelX = 0.0;
                goalVelZ = 0.0;
        }

        // Error calculations could be moved to their own function with static
        // variables to keep track of last error and errorAccumulator
        
        //error calculations
        pthread_mutex_lock(&lockUSD);
        const double errY = goalY - currPos.y;
        pthread_mutex_unlock(&lockUSD);
        
        pthread_mutex_lock(&lockUSF);
        const double errF = goalZ-currPos.z;
        pthread_mutex_unlock(&lockUSF);
        
        const double errVX = goalVelX - realVelX;
        const double errVZ = goalVelZ - realVelZ;

        //error accumulation for I component
        errAccY += errY * dt;
        errAccF += errF * dt;
        errAccVX += errVX * dt;
        errAccVZ += errVZ * dt;

        //change in error for D component
        double derivY = (errY - lastErrY) / dt;
        double derivF= (errF - lastErrF) / dt;
        double derivVX = (errVX - lastErrVX) / dt;
        double derivVZ = (errVZ - lastErrVZ) / dt;

        //calculate desired forces
        double newT = K.p*errY + K.i*errAccY + K.d*derivY;        
        double newF;
        if (state==MoveRight) {
            newF = PK.p*errF + PK.i*errAccF + PK.d*derivF;
        }
        else {
            newF = VZK.p*errVZ + VZK.i*errAccVZ + VZK.d*derivVZ;
        }
        double newX = VXK.p*errVX + VXK.i*errAccVX + VXK.d*derivVX;
        double newYaw;
        if (state == Turn) {
            newYaw = 0.5;
        }
        else {
            newYaw = 0.0;
        }

        //apply constraints
        
        if (realVelX > 0.1) {
		    newX = 0;
		}

	    if (realVelZ > 0.1) {
	    	newF = 0;
	    }

        //use spherical coordinates to calculate necessary thrust, pitch angle, and roll angle to produce desired forces
        double T = sqrt(newT*newT + 2*newT*QC_MASS*G + QC_MASS*QC_MASS*G*G + newF*newF + newX*newX);
        double pitchAngle = atan2(newF, (newT + QC_MASS*G));
        double rollAngle = atan2(newX, (newT + QC_MASS*G));

        if (state == Off) {
            T = 0;
            pitchAngle = 0;
            rollAngle = 0;
        }
        
        if (pitchAngle > M_PI / 9) {
		    pitchAngle = M_PI / 9;
	    }

	    if (pitchAngle < -M_PI / 9) {
		    pitchAngle = -M_PI / 9;
	    }

	    if (rollAngle > M_PI / 9) {
	    	rollAngle = M_PI / 9;
	    }

	    if (rollAngle < -M_PI / 9) {
	    	rollAngle = -M_PI / 9;
	    }

        //write out desired values to quadcopter flight controller
        
        pthread_mutex_lock(&lockP);
        setAng.p = pitchAngle;
        pthread_mutex_unlock(&lockP);
        
        pthread_mutex_lock(&lockR);
        setAng.r = rollAngle;
        pthread_mutex_unlock(&lockR);
                
        pthread_mutex_lock(&lockT);
        setThrottle = T*neutralThrottle / (QC_MASS*G);
        pthread_mutex_unlock(&lockT);

        pthread_mutex_lock(&lockY);
        setAng.y = newYaw;
        pthread_mutex_unlock(&lockY);

        
        // Save previous values
        lastTime = currTime;
        lastErrY = errY;
        lastErrF = errF;
        lastErrVX = errVX;
        lastErrVZ = errVZ;
        lastState = state;
        
        if (state != Turn) {
            pthread_mutex_lock(&lockUSF);
            lastDist = currPos.z;
            pthread_mutex_unlock(&lockUSF);

        }
        lastAng = currAng;
    }
}

//---------------------------------------------------------------------------
//-------------CONTROLLER ENDS HERE------------------------------------------
//---------------------------------------------------------------------------



void edge(int pinNum, int level, uint32_t tick) {
    static uint32_t lastT;    
    unsigned long dist;

    if (level == 1) {
        lastT = tick;
    } else {
        //printf("distance = %lu\n", ((tick - lastT)+191300)/57300);
    	pthread_mutex_lock(&lockTrig);
    	dist = 0.017449*((float)(tick - lastT) + 191.335);
    	pthread_mutex_unlock(&lockTrig);
    	// printf("distance = %lu\n", dist);
    }
    
    if (pinNum == BCM_ECHO1){
        pthread_mutex_lock(&lockUSF);
        currPos.z = dist;
        pthread_mutex_unlock(&lockUSF);
    }
    else if (pinNum == BCM_ECHO2){
        pthread_mutex_lock(&lockUSD);
        currPos.y = dist;
        pthread_mutex_unlock(&lockUSD);
    }
    //dist = (time + 191.3)/57.3;
}



// --------------------------TEST FUNCTIONS------------------------
/*void *USsensor(){

	for(;;){
		printf("T1 went \n");
		pthread_mutex_lock(&lock);
		printf("thread1 works\n");
		val++;
		printf("%d\n", val);
		pthread_mutex_unlock(&lock);
		sleep(10);
	}		
}*/

void *optical(){
    while (1) {
        ADNS_update(&sensor);
        //ADNS_update_position(&sensor, 0, 0, 0, 100);

        // check for errors
        if (sensor.has_overflow)
            printf("overflow!!\n");
        
        pthread_mutex_lock(&lockOpt);
        currVel.x = sensor.dx;
        currVel.z = sensor.dy;
        pthread_mutex_unlock(&lockOpt);
        
        gpioSleep(PI_TIME_RELATIVE, 0, 500000);

    }

/*	for(;;){
		printf("T2 went \n");
		pthread_mutex_lock(&lock);
		printf("thread2 works\n");
		val+=2;
		printf("%d\n", val);
		pthread_mutex_unlock(&lock);
		sleep(5);
	}*/
}


// ----------TRIGGER FUNCTION SENDS SIGNALS TO SENSORS--------------
void *trigger(){
	for(;;) {
        // Write a trigger pulse
        gpioWrite(BCM_TRIG, 1);
        // printf("sent trig\n");
        gpioDelay(20);
        gpioWrite(BCM_TRIG, 0);

        gpioSleep(PI_TIME_RELATIVE, 0, 100000);    // Wait for echo pulse
    }
}

void *interface(){
    double tempT;
    double tempY;
    double tempR;
    double tempP;

    int i2cFD = i2c_init();
    uint16_t wPitch;
    uint16_t wRoll;
    uint16_t wThrottle;
    uint16_t wYaw;

    uint8_t wMsgBytes[2];

    for(;;){
        printf("INTERFACE\n");
        pthread_mutex_lock(&lockT);
        tempT = setThrottle;
        pthread_mutex_unlock(&lockT);

        pthread_mutex_lock(&lockP);
        tempP = setAng.p;
        pthread_mutex_unlock(&lockP);

        pthread_mutex_lock(&lockY);
        tempY = setAng.y;
        pthread_mutex_unlock(&lockY);

        pthread_mutex_lock(&lockR);
        tempR = setAng.r;
        pthread_mutex_unlock(&lockR);

        // TODO: Bounds checking on these
        tempT = 970.0*tempT +910.0;
        tempP = 400*tempP/M_PI + 1410.0;
        tempR = 405*tempR/M_PI + 1525.0;
        // ---------tempY = <SOMETHING> I dont know the conversion from yaw to a pwm length

        wThrottle = makeWriteMsg(throttle, (int) tempT);
        //wYaw = makeWriteMsg(yaw, (int) tempY);
        wPitch = makeWriteMsg(pitch, (int) tempP);
        wRoll = makeWriteMsg(roll, (int) tempR);

        wMsgBytes[0] = (uint8_t)(wThrottle >> 8);
        wMsgBytes[1] = (uint8_t)(wThrottle & 0xFF);

        i2c_writeBytes(i2cFD, wMsgBytes, 2);

        wMsgBytes[0] = (uint8_t)(wPitch >> 8);
        wMsgBytes[1] = (uint8_t)(wPitch & 0xFF);

        i2c_writeBytes(i2cFD, wMsgBytes, 2);

        wMsgBytes[0] = (uint8_t)(wRoll >> 8);
        wMsgBytes[1] = (uint8_t)(wRoll & 0xFF);

        i2c_writeBytes(i2cFD, wMsgBytes, 2);

/*        wMsgBytes[0] = (uint8_t)(wYaw >> 8);
        wMsgBytes[1] = (uint8_t)(wYaw & 0xFF);

        i2c_writeBytes(i2cFD, wMsgBytes, 2);*/
        gpioSleep(PI_TIME_RELATIVE, 1, 0);
    }
}

//------------------------MAIN STARTS HERE---------------
int main(){
    gpioCfgClock(SAMPLE_RATE, 1, 1);

	int x = 0, y = 1;
	//printf("x: %d, y: %d\n", x, y);
	pthread_t thread_control, thread_interface, thread_US, thread_optical, thread_trig;

/*	if (pthread_mutex_init(&lock, NULL) != 0)
    {
        printf("\n mutex init failed on test lock\n");
        return 1;
    }*/
	
	if (pthread_mutex_init(&lockTrig, NULL) != 0)
    {
        printf("\n mutex init failed on trigger lock\n");
        return 1;
    }

    if (pthread_mutex_init(&lockT, NULL) != 0)
    {
        printf("\n mutex init failed on man override lock\n");
        return 1;
    }

    if (pthread_mutex_init(&lockP, NULL) != 0)
    {
        printf("\n mutex init failed on pitch\n");
        return 1;
    }
    
    if (pthread_mutex_init(&lockR, NULL) != 0)
    {
        printf("\n mutex init failed on roll\n");
        return 1;
    }
    
    if (pthread_mutex_init(&lockY, NULL) != 0)
    {
        printf("\n mutex init failed on yaw\n");
        return 1;
    }    

    if (pthread_mutex_init(&lockUSF, NULL) != 0)
    {
        printf("\n mutex init failed on front USS\n");
        return 1;
    }
    
    if (pthread_mutex_init(&lockUSD, NULL) != 0)
    {
        printf("\n mutex init failed on down USS\n");
        return 1;
    }
    
    if (pthread_mutex_init(&lockOpt, NULL) != 0)
    {
        printf("\n mutex init failed on optical\n");
        return 1;
    }

    if (gpioInitialise() < 0){
        return 1;
        printf("gpio init failed\n");
    }

    //-------------test threads--------------------------
	if(pthread_create(&thread_control, NULL, control, NULL)) {
		fprintf(stderr, "Error creating control thread\n");
		return 1;
	}

	if(pthread_create(&thread_interface, NULL, interface, NULL)) {
		fprintf(stderr, "Error creating interface thread\n");
		return 1;
	}
	//--------------------------------------------------

/*    if(pthread_create(&thread_US, NULL, USsensor, NULL)){
        fprintf(stderr, "Error creating US thread\n");
        return 1;
    }*/
    if(pthread_create(&thread_optical, NULL, optical, NULL)){
        fprintf(stderr, "Error creating optical thread\n");
        return 1;
    }

    gpioSetAlertFunc(BCM_ECHO1, edge);
    gpioSetAlertFunc(BCM_ECHO2, edge);

    gpioSetMode(BCM_ECHO1, PI_INPUT);
    gpioSetMode(BCM_ECHO2, PI_INPUT);
    gpioSetMode(BCM_TRIG, PI_OUTPUT);

    // reset trigger pin
    gpioWrite(BCM_TRIG, 0);




//  char c = 0;
	if(pthread_create(&thread_trig, NULL, trigger, NULL)) {
		fprintf(stderr, "Error creating trigger thread\n");
		return 1;
	}

	//thread_2();
	for(;;) sleep(10);
	//getchar();
    gpioTerminate();

	return 0;
}