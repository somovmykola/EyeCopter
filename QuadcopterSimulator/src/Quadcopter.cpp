#include "Quadcopter.h"
#include <algorithm>
#include <vector>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtx/rotate_vector.hpp>
#define _USE_MATH_DEFINES
#include "math.h"
#include "Noise.h"
#include "RayIntersection.h"
#include "Settings.h"

#define M_2PI		2*M_PI

using namespace std;
using glm::vec3;
using glm::mat3;
using glm::mat4;

extern Settings settings;

// Noise
extern Noise* noiseGen;	// Why use on stack? singleton pattern maybe?

Quadcopter::Quadcopter() {
	size = settings.GetDouble("copter_size");
	mass = settings.GetDouble("copter_mass");

	hoverThrottle = settings.GetDouble("copter_throttle_hover");
	pitchLimit = glm::radians(settings.GetDouble("copter_pitch_limit"));
	rollLimit = glm::radians(settings.GetDouble("copter_roll_limit"));

	velocity = vec3(0,0,0);
	position = vec3(0,0,0);

	if (settings.GetString("simulator_visual") == "on") {
		vector<vec3> points;
		points.push_back(0.5f * vec3(size, 0, size));
		points.push_back(0.5f * vec3(-size, 0, size));
		points.push_back(0.5f * vec3(-size, 0, -size));

		points.push_back(0.5f * vec3(-size, 0, -size));
		points.push_back(0.5f * vec3(size, 0, size));
		points.push_back(0.5f * vec3(size, 0, -size));

		model = new Model3D();
		model -> SetPoints(points);
	} else {
		model = NULL;
	}

	pitch = yaw = roll = 0.0;
	yawRate = 0.0;
	throttle = hoverThrottle;

	sensorAng = settings.GetDouble("copter_sensor_angle");

	sensorMax = settings.GetDouble("sensor_range_max");
	sensorMin = settings.GetDouble("sensor_range_min");
	sensorMin = max(sensorMin, 0.f);
}

Quadcopter::~Quadcopter() {
	if (model != NULL)
		delete model;
}

template <typename T> 
T clamp(T v, T lo, T hi) {
    return (v < lo) ? lo : ((v > hi) ? hi : v);
}

vec3 Quadcopter::GetFSensPos() const {
	const float offset = 0.5f*size;
	return position + offset * GetFSensDir();
}
vec3 Quadcopter::GetLSensPos() const {
	const float offset = 0.5f*size;
	return position + offset * GetLSensDir();
}
vec3 Quadcopter::GetRSensPos() const {
	const float offset = 0.5f*size;
	return position + offset * GetRSensDir();
}
vec3 Quadcopter::GetDSensPos() const {
	const float offset = 0.5f*size;
	return position - offset * GetForwardDir();
}

vec3 Quadcopter::GetFSensDir() const {
	return GetForwardDir();
}
vec3 Quadcopter::GetLSensDir() const {
	return glm::rotate(GetForwardDir(), +sensorAng, GetUpDir());
}
vec3 Quadcopter::GetRSensDir() const {
	return glm::rotate(GetForwardDir(), -sensorAng, GetUpDir());
}
vec3 Quadcopter::GetDSensDir() const {
	return -GetUpDir();
}

float Quadcopter::readSensor(Model3D *target, vec3 pos, vec3 dir) const {
	float dist;
	if (RayModelIntersection(target, pos, dir, &dist) == 0)
		return sensorMax;

	// add random noise to sensor reading
	dist += noiseGen->GetVal(Noise::sensor);

	return clamp(dist, sensorMin, sensorMax);
}

float Quadcopter::ReadFSensor(Model3D *target) const {
	return readSensor(target, GetFSensPos(), GetFSensDir());
}
float Quadcopter::ReadLSensor(Model3D *target) const {
	return readSensor(target, GetLSensPos(), GetLSensDir());
}
float Quadcopter::ReadRSensor(Model3D *target) const {
	return readSensor(target, GetRSensPos(), GetRSensDir());
}
float Quadcopter::ReadDSensor(Model3D *target) const {
	// Emulate a downward sensor by finding the distance along a ray to a plane
	// where the ray starts at DSensPos and goes in the direction of DSensDir
	// and the plane is given by y=0, ie the floor plane
	float dist = - GetDSensPos().y / GetDSensDir().y;

	// return clamped reading with sensor noise added
	return clamp(dist, sensorMin, sensorMax) + noiseGen->GetVal(Noise::sensor);
}

double Quadcopter::GetPitch() const {
	return pitch;
}
double Quadcopter::GetYaw() const {
	return yaw;
}
double Quadcopter::GetRoll() const {
	return roll;
}
double Quadcopter::GetThrottle() const {
	return throttle;
}

void Quadcopter::SetPitch(double p) {
	pitch = clamp(p, -pitchLimit, pitchLimit);
}
// Warning, don't call this in a real time simulation environment
void Quadcopter::SetYaw(double y) {
	yaw = y;
}
// Rate in rad/s
void Quadcopter::SetYawRate(double rate) {
	yawRate = rate;
}
void Quadcopter::SetRoll(double r) {
	roll = clamp(r, -rollLimit, rollLimit);
}
void Quadcopter::SetThrottle(double t) {
	throttle = clamp(t, 0.0, 1.0);
}

glm::vec3 Quadcopter::GetPosition() const {
	return position;
}

// Use this function sparingly. The whole point of this simulator is to figure
// out how to control this thing. You can't directly set the position of the
// quadcopter mid-flight so you shouldn't be doing it mid-simulation.
void Quadcopter::SetPosition(vec3 pos) {
	position = pos;
}

vec3 Quadcopter::GetVelocity() const {
	return velocity;
}

glm::vec3 Quadcopter::GetForwardDir() const {
	return glm::column(GetOrientationMatrix(), 2);
}
glm::vec3 Quadcopter::GetLeftDir() const {
	return glm::column(GetOrientationMatrix(), 0);
}
glm::vec3 Quadcopter::GetUpDir() const {
	return glm::column(GetOrientationMatrix(), 1);
}

// Note: Does not effect roll
// Use this function sparingly. The whole point of this simulator is to figure
// out how to control this thing. You can't directly set the direction of the
// quadcopter mid-flight so you shouldn't be doing it mid-simulation.
void Quadcopter::SetDirection(glm::vec3 dir) {
	pitch = asin(dir.y / glm::length(dir));
	yaw = atan2(dir.x, dir.z);
}

void Quadcopter::Update(double timestep) {
	// Update yaw
	yaw += yawRate * timestep;

	// Compute gravity vector
	const double G = 9.81;
	vec3 mgVec(0.0, mass * -G, 0.0);

	// Compute throttle vector
	double throtMag = (throttle / hoverThrottle) * mass * G;
	vec3 throtVec = (float)throtMag * GetUpDir();

	// Compute net acceleration
	vec3 netF = throtVec + mgVec;
	vec3 netA = netF / mass;

	// Euler integration:
	position += (float)timestep * velocity;
	velocity += (float)timestep * netA;

	// Simulate ground by clamping y position to positive values
	position.y = max(position.y, 0.f);
}

Model3D* Quadcopter::GetModel() const {
	return model;
}

mat3 Quadcopter::GetOrientationMatrix() const {
	return mat3(glm::yawPitchRoll(yaw, -pitch, roll));
}

mat4 Quadcopter::GetModelMatrix() const {
	mat4 ypr = glm::yawPitchRoll(yaw, -pitch, roll);

	return glm::translate(mat4(1.f), position) * ypr;
}
