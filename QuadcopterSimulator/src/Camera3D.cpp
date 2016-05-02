#include "Camera3D.h"
#include <glm/gtx/rotate_vector.hpp>
#include <algorithm>

using glm::vec3;


Camera3D::Camera3D() {
	pos = dir = vec3(0.0, 0.0, 0.0);
}

Camera3D::Camera3D(vec3 position, vec3 direction) {
	pos = position;
	dir = normalize(direction);
}

vec3 Camera3D::GetPos() const {
	return pos;
}

void Camera3D::SetPos(vec3 position) {
	pos = position;
}

vec3 Camera3D::GetDir() const {
	return dir;
}

void Camera3D::SetDir(vec3 direction) {
	dir = normalize(direction);
}

void Camera3D::MoveForward(float dist) {
	pos += dist * dir;
}

void Camera3D::MoveRight(float dist) {
	const double cosPhi = sqrt(1 - dir.y*dir.y);

	pos += dist * RightDir();
}

// Changing the pitch of the camera, or looking up or down. Automatically clamps
// the vertical angle to about 80 degrees to avoid gimbal lock.
void Camera3D::RotatePitch(float deg) {
	if (deg == 0.0f)
		return;
	
	const float ang = glm::radians(deg);
	const float phi = asin(dir.y);	// current vertical angle
	const float maxPhi = 1.4;		// max vertical angle, about 80 degrees

	float clampedAng;
	if (phi + ang > maxPhi)
		clampedAng = maxPhi - phi;
	else if (phi + ang < -maxPhi)
		clampedAng = maxPhi + phi;
	else
		clampedAng = ang;

	dir = glm::rotate(dir, clampedAng, RightDir());
}

void Camera3D::RotateYaw(float deg) {
	dir = glm::rotate(dir, glm::radians(deg), vec3(0.f, 1.f, 0.f));
}

vec3 Camera3D::RightDir() const {
	const double cosPhi = sqrt(1 - dir.y*dir.y);

	return vec3(-dir.z/cosPhi, 0, dir.x/cosPhi);
}

glm::mat4 Camera3D::GetMat() const {
	return glm::lookAt(pos, pos+dir, vec3(0,1,0));;
}