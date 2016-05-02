#include "QuadCopterController.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include "Quadcopter.h"
#include "Settings.h"
#include "Model3D.h"
#include <fstream>

#include <glm/gtx/projection.hpp>

using namespace std;

extern Quadcopter* qc;
extern Settings settings;
extern Model3D* model;

double currTime, lastTime;
double hoverThrottle;
ofstream fout;

double err;
double lastErr;
double errAcc;
double errF;
double lastErrF;
double errAccF;
double errVX;
double lastErrVX;
double errAccVX;
double errVZ;
double lastErrVZ;
double errAccVZ;
double goalYaw;
double errYaw;
double lastErrYaw;
double errAccYaw;
double lastState;
double lastYaw;
double lastDist;


void InitQCControls() {
	currTime = 0.0;
	lastTime = 0.0;
	hoverThrottle = settings.GetDouble("copter_throttle_hover");

	fout.open("data.txt");

	err = 0.0;
	errAcc = 0.0;
	lastErr = 0.0;

	errF = 0.0;
	lastErrF = 0.0;
	errAccF = 0.0;

	errVX = 0.0;
	lastErrVX = 0.0;
	errAccVX = 0.0;

	errVZ = 0.0;
	lastErrVZ = 0.0;
	errAccVZ = 0.0;

	goalYaw = qc->GetYaw();
	errYaw = 0.0;
	lastErrYaw = 0.0;
	errAccYaw = 0.0;

	lastState = 'A';
}

// DT is in milliseconds!
void UpdateQCControls(double dt) {
	currTime += dt;

	double currY = qc->ReadDSensor(model);
	double currZ = qc->ReadFSensor(model);
	double currYaw = qc->GetYaw();

	double zComp = qc->GetVelocity().z;
	double yComp = qc->GetVelocity().y;
	double xComp = qc->GetVelocity().x;
	double fDirX = -qc->GetForwardDir().x;
	double fDirY = -qc->GetForwardDir().y;
	double fDirZ = -qc->GetForwardDir().z;
	double lDirX = -qc->GetLeftDir().x;
	double lDirY = -qc->GetLeftDir().y;
	double lDirZ = -qc->GetLeftDir().z;
	double velZ = ((xComp)*(fDirX) + (yComp)*(fDirY) + (zComp)*(fDirZ)) / (fDirX*fDirX + fDirY*fDirY + fDirZ*fDirZ);
	double velX = ((xComp)*(lDirX) + (yComp)*(lDirY) + (zComp)*(lDirZ)) / (lDirX*lDirX + lDirY*lDirY + lDirZ*lDirZ);
	//double velZ = qc->GetVelocity().z;
	//double velX = qc->GetVelocity().x;
	velX = velX*cos(qc->GetRoll());
	velZ = velZ*cos(qc->GetPitch());

	double goalY;
	double goalZ;
	double goalVelX;
	double goalVelZ;

	//get state of quadcopter operations
	char state;
	if (currTime > 300) {
		state = 'C';
	}
	else if (currTime <= 10) {
		state = 'A';
	}
	else if (currZ >= 5.0){
		state = 'D';
	}
	else if ((lastState == 'D') && (currZ>=lastDist)) {
		state = 'D';
	}
	else if (currYaw !=lastYaw) {
		state = 'A';
	}
	else {
		state = 'B';
	}

	//set reference points based on state for PID controllers
	switch (state) {
		case 'A':
			goalY = 0.7;
			goalZ = 2.0;
			goalVelX = 0.0;
			goalVelZ = 0.0;
			break;
		case 'B':
			goalY = 0.7;
			goalZ = 2.0;
			goalVelX = 0.1;
			goalVelZ = 0.0;
			break;
		case 'C':
			goalY = 0.0;
			goalZ = 0.0;
			goalVelX = 0.0;
			goalVelZ = 0.0;
			break;
		case 'D':
			goalY = 0.7;
			goalZ = 0.0;
			goalVelX = 0.0;
			goalVelZ = 0.0;
			break;
		case 'E':
			goalY = 0.7;
			goalZ = 2.0;
			goalVelX = 0.0;
			goalVelZ = 0.0;
	}

	//PID constant declarations
	const double Kp = 0.0085;
	const double Ki = 0.0;
	const double Kd = 0.001;

	const double p_kp = 0.001;
	const double p_ki = 0.0;
	const double p_kd = 0.0015;

	const double vx_kp = 0.2;
	const double vx_ki = 0.0;
	const double vx_kd = 0.35;

	const double vz_kp = 0.2;
	const double vz_ki = 0.0;
	const double vz_kd = 0.35;

	//error calculations
	err = goalY - currY;
	errF = goalZ-currZ;
	errVX = goalVelX - velX;
	errVZ = goalVelZ - velZ;

	//error accumulation for I component
	errAcc += err * dt;
	errAccF += errF * dt;
	errAccVX += errVX * dt;
	errAccVZ += errVZ * dt;

	//change in error for D component
	double deriv = (err - lastErr) / dt;
	double derivF= (errF - lastErrF)/dt;
	double derivVX = (errVX - lastErrVX) / dt;
	double derivVZ = (errVZ - lastErrVZ) / dt;

	//calculate desired forces
	double newT = Kp*err + Ki*errAcc + Kd*deriv;
	double newF;
	if (state=='B') {
		newF = p_kp*errF + p_ki*errAccF + p_kd*derivF;
	}
	else {
		newF = vz_kp*errVZ + vz_ki*errAccVZ + vz_kd*derivVZ;
	}
	double newX = vx_kp*errVX + vx_ki*errAccVX + vx_kd*derivVX;
	double newYaw;
	if (state == 'D') {
		newYaw = 0.5;
	}
	else {
		newYaw = 0.0;
	}
	
	//apply constraints
	
	//use spherical coordinates to calculate necessary thrust, pitch angle, and roll angle to produce desired forces
	double T = sqrt(newT*newT + 2*newT*1.1*9.8+1.1*1.1*9.8*9.8+ newF*newF + newX*newX);
	double pitchAngle = atan(newF/(newT+1.1*9.8));
	double rollAngle = atan(newX / (newT + 1.1*9.8));

    //write out desired values to quadcopter flight controller
	qc->SetPitch(pitchAngle);
	qc->SetRoll(rollAngle);
	double throttle = T*hoverThrottle / (1.1*9.8);
	qc->SetThrottle(throttle);
	qc->SetYawRate(newYaw);

	double a = qc->GetVelocity().x;
	double b = qc->GetVelocity().z;
	fout << currTime << " " << 0.0 << " " << newF << endl;
	lastTime = currTime;
	lastErr = err;
	lastErrF = errF;
	lastErrVX = errVX;
	lastErrVZ = errVZ;
	lastState = state;
	if (state != 'D') {
		lastDist = currZ;
	}
	lastYaw = qc->GetYaw();
	printf("Current time: %.2f Current vel: %.2f \n", currTime, velZ);
}

void CleanupQCControls() {
	fout.close();
}