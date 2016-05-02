#include <math.h>
#include <time.h>

double currTime, lastTime;
double hoverThrottle;

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
double setPitch;
double setRoll;
double setThrottle;
double setYaw;

double currY;
double currZ;
double currYaw;

double velZ;
double velX;

double currPitch;
double currRoll;

double lastTime;
const double g = 9.8;
const double weight = 1.1;


currTime = 0.0;
lastTime = 0.0;
hoverThrottle = 0.15; //fix this

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
struct timeval tv;
gettimeofday(&tv, NULL);
lastTime= 1000000 * tv.tv_sec + tv.tv_usec;

// DT is in milliseconds!
while (true){
	struct timeval tv;
	gettimeofday(&tv, NULL);
	currTime = 1000000 * tv.tv_sec + tv.tv_usec;
	double dt = currTime-lastTime;

	//double velZ = qc->GetVelocity().z;
	//double velX = qc->GetVelocity().x;
	double realVelX = velX*cos(currRoll);
	double realVelZ = velZ*cos(currPitch);

	double goalY;
	double goalZ;
	double goalVelX;
	double goalVelZ;

	//get state of quadcopter operations
	// char state;
	// if (currTime > 320) {
	// 	state = 'E';
	// }
	// else if (currTime > 300) {
	// 	state = 'C';
	// }
	// else if (currTime <= 10) {
	// 	state = 'A';
	// }
	// else if (currZ >= 5.0){
	// 	state = 'D';
	// }
	// else if ((lastState == 'D') && (currZ>=lastDist)) {
	// 	state = 'D';
	// }
	// else if (currYaw !=lastYaw) {
	// 	state = 'A';
	// }
	// else {
	// 	state = 'B';
	// }
	char state;
	state='A';

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
			goalY = 0.0;
			goalZ = 0.0;
			goalVelX = 0.0;
			goalVelZ = 0.0;
	}

	//PID constant declarations
	const double Kp = 0.01;
	const double Ki = 0.0;
	const double Kd = 0.5;

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
	errVX = goalVelX - realVelX;
	errVZ = goalVelZ - realVelZ;

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
	double T = sqrt(newT*newT + 2*newT*weight*g+weight*weight*g*g+ newF*newF + newX*newX);
	double pitchAngle = atan(newF/(newT+weight*g));
	double rollAngle = atan(newX / (newT + weight*g));

	if (state == 'F') {
		T = 0;
		pitchAngle = 0;
		rollAngle = 0;
	}

   	 //write out desired values to quadcopter flight controller
	setPitch=pitchAngle;
	setRoll=rollAngle;
	double throttle = T*hoverThrottle / (weight*g);
	setThrottle=throttle;
	setYaw=newYaw;

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
	gettimeofday(&tv, NULL);
	lastTime = 1000000 * tv.tv_sec + tv.tv_usec;
}