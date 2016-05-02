#ifndef QCS_VIEW_MODEL_3D_H
#define QCS_VIEW_MODEL_3D_H

#include <array>
#include <string>
#include <vector>
#include <glm/glm.hpp>

class Model3D {
public:
	Model3D();
	~Model3D();
	
	void LoadSTL(std::string filename);
	void LoadOBJ(std::string filename);
	void SetPoints(std::vector<glm::vec3> newPoints);

	void Scale(float scale);

	void Draw() const;

	glm::vec3 GetMinBounds() const;
	glm::vec3 GetMaxBounds() const;

	unsigned int GetNumPoints() const;
	unsigned int GetNumTris() const;

	std::array<glm::vec3, 3> GetTri(unsigned int n) const;
	
	// ModelMatrix to keep track of position and orientation
	glm::mat4 mat;
private:
	void CreateVBO();
	void DeleteVBO();

	void UpdateBounds();
	
	unsigned int vertexBuffer;

	std::vector<glm::vec3> points;
	glm::vec3 minBound, maxBound;
};

#endif