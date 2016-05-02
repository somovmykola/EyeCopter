#include "RayIntersection.h"

using namespace std;
using glm::vec3;

// Compute the distance to the point of intersection of a ray and a triangle.
//
// Ray is specified using a starting vector and a direction vector. The
//		direction vector doesn't have to be normalized.
//
// Triangle is specified by an stl array of 3 glm::vec3
//
// dist is the distance along rayDir from the point of intersection to the ray
//		origin. The point of intersection is described by the following equation
//
//		point_of_intersection = rayOrigin + dist * rayDir;
//
// Returns 1 if intersection exists, 0 else

int RayTriangleIntersection(array<vec3, 3> tri, vec3 rayOrigin, vec3 rayDir, float *dist) {
	// https://en.wikipedia.org/wiki/M%C3%B6ller%E2%80%93Trumbore_intersection_algorithm
	const float EPS = 0.000001f;

	vec3 edge1 = tri[1] - tri[0];
	vec3 edge2 = tri[2] - tri[0];

	vec3 P = glm::cross(rayDir, edge2);
	

	// If determinant is close to zero, ray is in the plane of the triangle
	// ie there are infinite points of intersection.
	float det = glm::dot(edge1, P);
	if (fabs(det) < EPS)
		return 0;

	float detInv = 1.f / det;

	// Calculate and test u parameter
	vec3 T = rayOrigin - tri[0];
	float u = detInv * glm::dot(T, P);

	// Intersection lies outside of the triangle
	if (u < 0.f || u > 1.f)
		return 0;

	// Calculate and test u parameter
	vec3 Q = glm::cross(T, edge1);
	float v = detInv * glm::dot(rayDir, Q);

	// Intersection lies outside of the triangle
	if (v < 0.f || u + v > 1.f)
		return 0;

	float t = detInv * glm::dot(edge2, Q);

	if (t > EPS) {
		*dist = t;
		return 1;
	}

	return 0;

}

// Compute the point of intersection of a Model3D and a ray.
// 
// Ray is specified using a starting vector and a direction vector. The
//		direction vector doesn't have to be normalized.
// 
// dist is the distance along rayDir from the point of intersection to the ray
//		origin. The point of intersection is described by the following equation
//
//		point_of_intersection = rayOrigin + dist * rayDir;
// 
// If the ray intersects the model at multiple points, dist represents the 
//		distance along rayDir to the _closest_ point of intersection.
//
// Returns 1 if intersection exists, 0 else

int RayModelIntersection(Model3D *model, vec3 rayOrigin, vec3 rayDir, float *dist) {
	*dist = FLT_MAX;

	for (int i = 0; i < model->GetNumTris(); i ++) {			
		float tempDist;
		if (RayTriangleIntersection(model->GetTri(i), rayOrigin, rayDir, &tempDist))
			if (tempDist > 0.0 && tempDist < *dist)
				*dist = tempDist;
	}

	return *dist != FLT_MAX;
}