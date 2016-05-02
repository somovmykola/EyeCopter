#ifndef QCS_CAMERA_3D_H
#define QCS_CAMERA_3D_H

#include <glm/glm.hpp>

class Camera3D {
public:
	Camera3D();
	Camera3D(glm::vec3 position, glm::vec3 direction);

	glm::vec3 GetPos() const;
	void SetPos(glm::vec3 position);

	glm::vec3 GetDir() const;
	void SetDir(glm::vec3 direction);

	void MoveForward(float dist);
	void MoveRight(float dist);
	// void MoveNormal(float dist);

	void RotatePitch(float deg);
	void RotateYaw(float deg);

	glm::mat4 GetMat() const;
private:
	glm::vec3 RightDir() const;

	glm::vec3 pos;
	glm::vec3 dir;
};

#endif