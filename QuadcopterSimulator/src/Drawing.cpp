#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <array>
#include <iostream>
#include <vector>
#include "Camera3D.h"
#include "AppControls.h"
#include "Drawing.h"
#include "Model3D.h"
#include "Settings.h"
#include "Shader.h"
#include "Quadcopter.h"

using namespace std;
using glm::vec3;
using glm::mat4;

GLFWwindow* window;

// Models
Model3D *model, *cameraModel;

// GLSL IDs
GLuint MultiShaderID, SingleShaderID;
GLuint MVPmatrixID[2];
GLuint colorBuffer;
GLuint vertexArrayID;

// Drawing matrices
mat4 modelMatrix;
mat4 viewMatrix;
mat4 projMatrix;
mat4 MVP;

// Program State
extern unsigned int appState;

// Drawing States
bool drawCameraPath;
bool drawCameraModels;
unsigned int floorType;
float floorHeight;

// Misc
extern vector<Camera3D> capturePositions;
extern Settings settings;

extern Quadcopter *qc;


int InitGL() {
	if (!glfwInit()) {
		fprintf(stderr, "Could not initialize glfw\n");
		return EXIT_FAILURE;
	}

	glfwSetErrorCallback(errorCallback);

	window = glfwCreateWindow(800, 600, "Simulator", NULL, NULL);
	glfwSetWindowPos(window, 400, 200);

	if (!window) {
		fprintf(stderr, "Could not create GLFWwindow\n");
		glfwTerminate();
		return EXIT_FAILURE;
	}

	glfwMakeContextCurrent(window);

	glewExperimental = true; // Needed in core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		glfwTerminate();
		return EXIT_FAILURE;
	}
	
    glfwSwapInterval(1);

	glfwSetKeyCallback(window, keyboardCallback);
	glfwSetCursorPosCallback(window, mouseMotionCallback);
	glfwSetWindowCloseCallback(window, windowCloseCallback);

	//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);
	
	// Create and compile our GLSL program from the shaders
	SingleShaderID = LoadShaders("SingleColorVertShader.glsl", "SimpleFragShader.glsl");
	if (SingleShaderID == 0)
		return EXIT_FAILURE;

	MultiShaderID = LoadShaders("MultiColorVertShader.glsl", "SimpleFragShader.glsl");
	if (MultiShaderID == 0)
		return EXIT_FAILURE;

	// Get a handle for our "MVP" uniform
	MVPmatrixID[0] = glGetUniformLocation(SingleShaderID, "MVP");
	MVPmatrixID[1] = glGetUniformLocation(MultiShaderID, "MVP");

	// Create vertex array object
	glGenVertexArrays(1, &vertexArrayID);
	glBindVertexArray(vertexArrayID);

	return 0;
}

// fov in radians, ratio in w/h
Model3D* MakeCameraModel(double fov, double ratio) {
	vector<vec3> points;

	const double theta = glm::radians(fov);
	const double y = 0.2;	// 50 cm;
	const double z = ratio * y;
	const double x = 0.5*z / tan(0.5*theta);

	// Front face:
	points.push_back(vec3(x, y,-z));
	points.push_back(vec3(x, y, z));
	points.push_back(vec3(x,-y,-z));
	points.push_back(vec3(x,-y,-z));
	points.push_back(vec3(x, y, z));
	points.push_back(vec3(x,-y, z));

	// Top face:
	points.push_back(vec3(x, y,-z));
	points.push_back(vec3(x, y, z));
	points.push_back(vec3(0.0, 0.0, 0.0));
	// Bottom face:
	points.push_back(vec3(x,-y,-z));
	points.push_back(vec3(x,-y, z));
	points.push_back(vec3(0.0, 0.0, 0.0));
	// Left face:
	points.push_back(vec3(x, y,-z));
	points.push_back(vec3(x,-y,-z));
	points.push_back(vec3(0.0, 0.0, 0.0));
	// Right face:
	points.push_back(vec3(x, y, z));
	points.push_back(vec3(x,-y, z));
	points.push_back(vec3(0.0, 0.0, 0.0));

	Model3D *ret = new Model3D();
	ret -> SetPoints(points);
	return ret;
}

int LoadModels() {
	

	// Create Camera Models
	const double cameraFov = settings.GetDouble("camera_fov");
	const double cameraRatio = settings.GetDouble("image_h_res") / settings.GetDouble("image_v_res");
	cameraModel = MakeCameraModel(cameraFov, cameraRatio);

	// Init model color data
	// move to model3d?
	const unsigned int numPoints = model -> GetNumPoints();
	float *colorBuffData = (float*)malloc(3*numPoints * sizeof(float));

	for (int i = 0; i < numPoints/3; i++) {
		for (int j = 0; j < 3; j++) {
			float c = rand() / (float)RAND_MAX;
			for (int k = 0; k < 3; k++)
				colorBuffData[9*i+3*k+j] = c;
		}
	}		
	
	glGenBuffers(1, &colorBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
	glBufferData(GL_ARRAY_BUFFER, 3*numPoints*sizeof(float), colorBuffData, GL_STATIC_DRAW);

	free(colorBuffData);
	return 0;
}

int InitScene() {
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

	// Init Draw states
	drawCameraPath = false;
	drawCameraModels = false;
	
	// Init floor
	floorType = none;
	floorHeight = model -> GetMinBounds().y - 5;

	return 0;
}

int InitDrawing() {
	int err;

	if (err = LoadModels())
		return err;

	if (err = InitScene())
		return err;

	return 0;
}

int CleanupGL() {
	glDeleteBuffers(1, &colorBuffer);
	glDeleteVertexArrays(1, &vertexArrayID);

	glDeleteProgram(SingleShaderID);
	glDeleteProgram(MultiShaderID);
	glfwDestroyWindow(window);

	return 0;
}

int CleanupDrawing() {
	delete cameraModel;

	return 0;
}

void UpdateMVP() {
	MVP = projMatrix*viewMatrix*modelMatrix;

	for (int i = 0; i < 2; i++)
		glUniformMatrix4fv(MVPmatrixID[i], 1, GL_FALSE, &MVP[0][0]);
}

void glVertex3(vec3 v) {
	glVertex3f(v.x, v.y, v.z);
}

void DrawAxes() {
	glUseProgram(SingleShaderID);
	modelMatrix = mat4(1.f);
	UpdateMVP();

	const double len = 1.0;

	// X
	glColor3f(1.0, 0.0, 0.0);
	glBegin(GL_LINES);
	glVertex3d(0.0, 0.0, 0.0);
	glVertex3d(len, 0.0, 0.0);
	glEnd();

	// Y
	glColor3f(0.0, 1.0, 0.0);
	glBegin(GL_LINES);
	glVertex3d(0.0, 0.0, 0.0);
	glVertex3d(0.0, len, 0.0);
	glEnd();

	// Z
	glColor3f(0.0, 0.0, 1.0);
	glBegin(GL_LINES);
	glVertex3d(0.0, 0.0, 0.0);
	glVertex3d(0.0, 0.0, len);
	glEnd();
}

void DrawFloor(unsigned int type) {
	if (type == solid) {
		glUseProgram(SingleShaderID);
		modelMatrix = mat4(1.f);
		UpdateMVP();

		glColor3f(0.3, 0.3, 0.3);
		glBegin(GL_QUADS);
		glVertex3d(-100.0, -1.0, -100.0);
		glVertex3d(+100.0, -1.0, -100.0);
		glVertex3d(+100.0, -1.0, +100.0);
		glVertex3d(-100.0, -1.0, +100.0);
		glEnd();
	} else if (type == checker) {

	}
}

void DrawCaptureCameras() {
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glUseProgram(MultiShaderID);
	modelMatrix = mat4(1.f);
	UpdateMVP();

	const vec3 initDir = vec3(1.f, 0.f, 0.f);
	const vec3 upDir = vec3(0.f, 1.f, 0.f);
	const vec3 normDir = vec3(0.f, 0.f, 1.f);
	for (int i = 0; i < capturePositions.size(); i++) {
		vec3 finalDir = capturePositions[i].GetDir();

		vec3 v1 = glm::normalize(vec3(finalDir.x, 0.0, finalDir.z));
		double ang1 = glm::orientedAngle(initDir, v1, upDir);

		double ang2 = glm::orientedAngle(v1, finalDir, glm::cross(v1, upDir));

		modelMatrix = glm::translate(mat4(1.f), capturePositions[i].GetPos());
		modelMatrix = glm::rotate(modelMatrix, (float)ang1, upDir);
		modelMatrix = glm::rotate(modelMatrix, (float)ang2, normDir);
			
		UpdateMVP();

		cameraModel -> Draw();
	}

	glDisableVertexAttribArray(1);
}

void DrawQuadcopter() {
	glUseProgram(SingleShaderID);
	modelMatrix = qc->GetModelMatrix();
	UpdateMVP();
		

	glColor3f(0.0, 0.4, 0.3);
	qc->GetModel()->Draw();

	// Draw quadcopter's axes
	if (0) {
		const double axL = 1.0;

		// X
		glColor3f(1.0, 0.0, 0.0);
		glBegin(GL_LINES);
		glVertex3d(0.0, 0.0, 0.0);
		glVertex3d(axL, 0.0, 0.0);
		glEnd();

		// Y
		glColor3f(0.0, 1.0, 0.0);
		glBegin(GL_LINES);
		glVertex3d(0.0, 0.0, 0.0);
		glVertex3d(0.0, axL, 0.0);
		glEnd();

		// Z
		glColor3f(0.0, 0.0, 1.0);
		glBegin(GL_LINES);
		glVertex3d(0.0, 0.0, 0.0);
		glVertex3d(0.0, 0.0, axL);
		glEnd();
	}
		
	// Draw quadcopter's sensor rays
	if (1) {
		float len = 1.0f;

		modelMatrix = mat4(1.f);
		UpdateMVP();

		glColor3f(1.0, 0.3, 0.0);
		glBegin(GL_LINES);
		glVertex3(qc->GetRSensPos());
		glVertex3(qc->GetRSensPos() + len*qc->GetRSensDir());
		glVertex3(qc->GetFSensPos());
		glVertex3(qc->GetFSensPos() + len*qc->GetFSensDir());
		glVertex3(qc->GetLSensPos());
		glVertex3(qc->GetLSensPos() + len*qc->GetLSensDir());
		glEnd();
	}
}

void DrawCameraPath() {
	glUseProgram(SingleShaderID);
	modelMatrix = mat4(1.f);
	UpdateMVP();
		
	glColor3f(0.0, 0.0, 1.0);
	glBegin(GL_LINE_LOOP);
	for (int i = 0; i < capturePositions.size(); i++)
		glVertex3(capturePositions[i].GetPos());
	glEnd();
}

void DrawModel() {
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glUseProgram(MultiShaderID);
	modelMatrix = mat4(1.f);
	UpdateMVP();

	model->Draw();

	glDisableVertexAttribArray(1);
}

void DrawScene() {
	//DrawAxes();

	DrawFloor(floorType);
	

	// Draw cameras if enabled
	if (appState == freeCam && drawCameraModels)
		DrawCaptureCameras();

	if (appState == freeCam)
		DrawQuadcopter();

	// Draw camera path if enabled
	if (appState == freeCam && drawCameraPath)
		DrawCameraPath();


	DrawModel();
}