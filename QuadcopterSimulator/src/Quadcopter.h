#ifndef QUADCOPTER_H
#define QUADCOPTER_H

#include <glm/glm.hpp>
#include "Model3D.h"

class Quadcopter {
public:
	Quadcopter();
	~Quadcopter();

	glm::vec3 GetFSensPos() const;
	glm::vec3 GetLSensPos() const;
	glm::vec3 GetRSensPos() const;
	glm::vec3 GetDSensPos() const;
	glm::vec3 GetFSensDir() const;
	glm::vec3 GetLSensDir() const;
	glm::vec3 GetRSensDir() const;
	glm::vec3 GetDSensDir() const;

	float ReadFSensor(Model3D *target) const;
	float ReadLSensor(Model3D *target) const;
	float ReadRSensor(Model3D *target) const;
	float ReadDSensor(Model3D *target) const;

	double GetPitch() const;
	double GetYaw() const;
	double GetRoll() const;
	double GetThrottle() const;

	void SetPitch(double rang);
	void SetYaw(double rang);
	void SetYawRate(double rate);
	void SetRoll(double rang);
	void SetThrottle(double t);

	glm::vec3 GetPosition() const;
	void SetPosition(glm::vec3 pos);

	glm::vec3 GetVelocity() const;
	
	glm::vec3 GetForwardDir() const;
	glm::vec3 GetLeftDir() const;
	glm::vec3 GetUpDir() const;
	void SetDirection(glm::vec3 dir);

	glm::mat3 GetOrientationMatrix() const;
	glm::mat4 GetModelMatrix() const;

	void Update(double timestep);

	Model3D* GetModel() const;	
private:
	float readSensor(Model3D *target, glm::vec3 pos, glm::vec3 dir) const;
	float clampedSensorReading(float d) const;

	double pitch, yaw, roll;	// radians
	double yawRate;				// rad / s
	double throttle;			// percentage

	float mass;
	float size;

	double hoverThrottle;
	double pitchLimit, rollLimit;

	float sensorAng;
	float sensorMin, sensorMax;

	glm::vec3 position, velocity;
	Model3D *model;
};

#endif