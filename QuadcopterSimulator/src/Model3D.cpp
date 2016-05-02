#include <algorithm>
#include <fstream>
#include <GL/glew.h>
#include "Model3D.h"
#include "Settings.h"

using namespace std;
using glm::vec3;
using glm::mat4;

extern Settings settings;

Model3D::Model3D() {
	vertexBuffer = 0;
	mat = mat4(1.f);
}

Model3D::~Model3D() {
	DeleteVBO();
}

// Read a binary STL file
void Model3D::LoadSTL(string filename) {
	// Todo: Check if file is ASCII and reject it.
    ifstream fin (filename.c_str(), ios::in | ios::binary);

	if (!fin.good()) {
		fprintf(stderr, "Could not open file \"%s\"\n", filename.c_str());
		return;
	}
 
    char stlHeader[80] = "";
    char nTri[4];
    unsigned long numTris;
 
    //read 80 byte header
    fin.read(stlHeader, 80);

	// If the header begins with "solid", its an ASCII file.
	if (strcmp(stlHeader, "solid") > 0) {
		printf("%s is an ASCII file\n", filename.c_str());
		return;
	}
 
    //read 4-byte ulong
    if (fin.good()) {
        fin.read(nTri, 4);
        numTris = *((unsigned long*)nTri) ;
    } else {
        printf("Unable to read number of triangles\n");
		return;
    }
 
    // Read triangles, stored in 50 bytes. 
	// First 12 bytes are 3 floats that describe the surface normal.
	// Each point of the triangle is given by 12 bytes or 3 floats, x,y,z
	// Last two bytes are for additional info and are ignored
	char facet[50];
	float fx, fy, fz;
	const float unitCorrection = 1.0;
    for(int i = 0; i < numTris && fin.good(); i++){ 
        fin.read(facet, 50);

		// start from 1 to that we skip over the normal vector
		for (int i = 1; i < 4; i++) {
			fx = *((float*) (facet+(i*12)+0));
			fy = *((float*) (facet+(i*12)+4));
			fz = *((float*) (facet+(i*12)+8));
			fx *= unitCorrection;
			fy *= unitCorrection;
			fz *= unitCorrection;
			points.push_back(vec3(fx, fy, fz));
		}
    }

	CreateVBO();
	UpdateBounds();
}

void Model3D::LoadOBJ(string filename) {
	printf("OBJ loading not implemented yet.\n");
}

void Model3D::SetPoints(vector<vec3> newPoints) {
	DeleteVBO();

	points.clear();
	points = newPoints;

	CreateVBO();
	UpdateBounds();
}

void Model3D::Scale(float scale) {
	DeleteVBO();

	for (int i = 0; i < points.size(); i++)
		points[i] *= scale;

	CreateVBO();
	UpdateBounds();
}

void Model3D::DeleteVBO() {
	// Only allow VBOs in visual mode
	if (settings.GetString("simulator_visual") == "off")
		return;

	// Free GL vertex buffer objects
	if (vertexBuffer != 0)
		glDeleteBuffers(1, &vertexBuffer);

	vertexBuffer = 0;
}

void Model3D::CreateVBO() {
	// Only allow VBOs in visual mode
	if (settings.GetString("simulator_visual") == "off")
		return;

	glGenBuffers(1, &vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(float) * points.size(), &points[0], GL_STATIC_DRAW);
}

void Model3D::Draw() const {
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glDrawArrays(GL_TRIANGLES, 0, points.size());
	glDisableVertexAttribArray(0);
}

void Model3D::UpdateBounds() {
	const float big = FLT_MAX;

	minBound = vec3(big, big, big);
	maxBound = -minBound;

	for (int i = 0; i < points.size(); i++) {
		minBound.x = min(minBound.x, points[i].x);
		minBound.y = min(minBound.y, points[i].y);
		minBound.z = min(minBound.z, points[i].z);

		maxBound.x = max(maxBound.x, points[i].x);
		maxBound.y = max(maxBound.y, points[i].y);
		maxBound.z = max(maxBound.z, points[i].z);
	}
}

vec3 Model3D::GetMinBounds() const {
	return minBound;
}

vec3 Model3D::GetMaxBounds() const {
	return maxBound;
}

unsigned int Model3D::GetNumPoints() const {
	return points.size();
}

unsigned int Model3D::GetNumTris() const {
	return points.size() / 3;
}

array<vec3, 3> Model3D::GetTri(unsigned int n) const {
	return {points.at(3*n+0), points.at(3*n+1), points.at(3*n+2)};
}
