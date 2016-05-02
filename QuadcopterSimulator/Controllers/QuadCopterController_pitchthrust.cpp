#include "QuadCopterController.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include "Quadcopter.h"
#include "Settings.h"
#include "Model3D.h"
#include <fstream>

using namespace std;

extern Quadcopter* qc;
extern Settings settings;
extern Model3D* model;

double currTime, lastTime;
double hoverThrottle;
fstream fout;

double err;
double lastErr;
double errAcc;
double errF;
double lastErrF;
double errAccF;


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
}

// DT is in milliseconds!
void UpdateQCControls(double dt) {
	currTime += dt;

	double currY = qc->GetPosition().y;
	double currZ = qc->GetPosition().z;
	double currX = qc->GetPosition().x;
	double goalY = 2.0;
	double goalZ = 2.0;

	const double Kp = 1.0;
	const double Ki = 0.0;
	const double Kd = 0.5;

	const double p_kp = 1.0;
	const double p_ki = 0.0;
	const double p_kd = 1.0;

	double err = goalY - currY;
	errF = goalZ-currZ;

	errAcc += err * dt;
	errAccF += errF * dt;

	double deriv = (err - lastErr) / dt;
	double derivF= (errF - lastErrF)/dt;

	double newT = Kp*err + Ki*errAcc + Kd*deriv;
	double newF = p_kp*errF + p_ki*errAccF + p_kd*derivF;

	double T = sqrt(newT*newT + 2*newT*1.1*9.8+1.1*1.1*9.8*9.8+ newF*newF);
	double pitchAngle = atan(newF/(newT+1.1*9.8));

	qc->SetPitch(pitchAngle);
	qc->SetThrottle(T);

	fout << currTime << " " << qc->GetPosition().y << endl;
	lastTime = currTime;
	lastErr = err;
	lastErrF = errF;
	printf("Current height: %.2f Current z: %.2f \n", currY, currZ);
}