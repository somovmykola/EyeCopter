#ifndef RAY_INTERSECTION_H
#define RAY_INTERSECTION_H

#include <array>
#include <glm/glm.hpp>
#include "Model3D.h"

int RayTriangleIntersection(std::array<glm::vec3, 3> tri, glm::vec3 rayOrigin, glm::vec3 rayDir, float *dist);
int RayModelIntersection(Model3D *model, glm::vec3 rayOrigin, glm::vec3 rayDir, float *dist);

#endif