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
ofstream fout;

double err;
double lastErr;
double errAcc;


void InitQCControls() {
	currTime = 0.0;
	lastTime = 0.0;
	hoverThrottle = settings.GetDouble("copter_throttle_hover");

	fout.open("data.txt");

	err = 0.0;
	errAcc = 0.0;
	lastErr = 0.0;
}

// DT is in milliseconds!
void UpdateQCControls(double dt) {
	currTime += dt;

	double currY = qc->ReadDSensor(model);
	double goalY = 2.0;
	
	const double Kp = 1.0;
	const double Ki = 0.0;
	const double Kd = 0.5;

	double err = goalY - currY;

	errAcc += err * dt;

	double deriv = (err - lastErr) / dt;

	double newT = Kp*err + Ki*errAcc + Kd*deriv + hoverThrottle;
	
	qc->SetThrottle(newT);

	fout << currTime << " " << qc->GetPosition().y << endl;
	lastTime = currTime;
	lastErr = err;
}