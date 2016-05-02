#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <ctime>
#include <map>
#include <array>
#include <vector>
#include "AppControls.h"
#include "Camera3D.h"
#include "ImageRender.h"
#include "Model3D.h"
#include "Noise.h"
#include "RayIntersection.h"
#include "Settings.h"
#include "Quadcopter.h"
#include "QuadCopterController.h"

using namespace std;
using glm::mat4;
using glm::vec3;
using glm::vec4;

// Input
map<int, bool> keyState;
double lastMouseX, lastMouseY;
double mouseDX, mouseDY;

// Controls / State
Camera3D cam;
mat4 freeCamProjMat;
unsigned int appState;
bool shouldQuit;
unsigned int currCamNum;

bool interactiveMode;
bool visualMode;
unsigned int endTime;
double timestep;

bool ignoreInputsThisFrame;

// Timing
float lastTime;

// Drawing
extern mat4 viewMatrix, projMatrix, modelMatrix;
extern void UpdateMVP();

extern Model3D *model;

// Misc
extern vector<Camera3D> capturePositions;
extern GLFWwindow* window;
Noise* noiseGen;

Quadcopter *qc = NULL;

extern Settings settings;

int InitState() {
	shouldQuit = false;
	appState = freeCam;

	// Set to true since for some reason app starts with a mouse motion
	ignoreInputsThisFrame = true;

	string visualStr = settings.GetString("simulator_visual");
	if (visualStr == "on")
		visualMode = true;
	else if (visualStr == "off")
		visualMode = false;

	string interStr = settings.GetString("simulator_interactive");
	if (interStr == "on")
		interactiveMode = true;
	else if (interStr == "off")
		interactiveMode = false;

	endTime = settings.GetInt("simulator_end_time");

	timestep = settings.GetDouble("simulator_timestep");
	timestep *= 0.001;	// Convert timestep from milliseconds to seconds
	
	return 0;
}

int InitInputs() {
	lastMouseX = lastMouseY = 0.0;
	mouseDX = mouseDY = 0.0;

	// all keys are released at start
	for (int i = 0; i < GLFW_KEY_LAST; i++)
		keyState[i] = false;

	return 0;
}

int InitCamera() {
	glViewport(0, 0, 800, 600);

	// Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	freeCamProjMat = glm::perspective(glm::radians(90.0f), 4.0f/3.0f, 0.1f, 100.0f);
	projMatrix = freeCamProjMat;

	// Camera matrix
	cam.SetPos(vec3(-1, 3, 6));
	cam.SetDir(vec3(0, 0, -1));	// Camera starts looking in -Z
	viewMatrix = glm::lookAt(cam.GetPos(), vec3(0,0,0), vec3(0,1,0));

	// Model matrix
	modelMatrix = mat4(1.0f);
	modelMatrix = glm::translate(modelMatrix, vec3(5.0, 0.0, 0.0));

	// Update model-view-projection matrix
	UpdateMVP();

	currCamNum = 0;

	return 0;
}

int InitQuadcopter() {
	if (qc != NULL)
		delete qc;

	qc = new Quadcopter();
	vec3 pos;
	pos.x = settings.GetDouble("copter_init_pos_x");
	pos.y = settings.GetDouble("copter_init_pos_y");
	pos.z = settings.GetDouble("copter_init_pos_z");
	qc -> SetPosition(pos);

	vec3 dir;
	dir.x = settings.GetDouble("copter_init_dir_x");
	dir.y = settings.GetDouble("copter_init_dir_y");
	dir.z = settings.GetDouble("copter_init_dir_z");
	qc -> SetDirection(dir);

	return 0;
}

int InitOthers() {
	srand(time(0));

	noiseGen = new Noise(settings);

	// Load model
	model = new Model3D();
	string modelFilename = settings.GetString("model_file");
	model -> LoadSTL(modelFilename);

	double modelScale = settings.GetDouble("model_scale");
	model -> Scale(modelScale);

	// timing
	lastTime = 0.f;

	return 0;
}


int InitApp() {
	int err;

	if (err = InitState())
		return err;

	if (visualMode) {
		if (err = InitInputs())		return err;
		if (err = InitCamera())		return err;
	}

	if (err = InitQuadcopter())
		return err;

	if (err = InitOthers())
		return err;

	InitQCControls();

	return 0;
}

int CleanupApp() {
	delete qc;
	delete noiseGen;
	delete model;

	return 0;
}

void UpdateQuadcopter(float dt) {
	if (interactiveMode == false) {
		UpdateQCControls(dt);
		qc->Update(dt);

		return;
	} 
	
	// if (interactiveMode)
	if (ignoreInputsThisFrame) {
		qc->Update(dt);
		return;
	}

	const float moveSpeed = 5.f;

	vec3 qcPos = qc->GetPosition();
	vec3 qcFDir = qc->GetForwardDir();
	vec3 qcLDir = qc->GetLeftDir();
	vec3 qcUDir = qc->GetUpDir();

	if (keyState[GLFW_KEY_UP])
		qc->SetPosition(qcPos + qcFDir*moveSpeed*dt);
	if (keyState[GLFW_KEY_DOWN])
		qc->SetPosition(qcPos - qcFDir*moveSpeed*dt);
	if (keyState[GLFW_KEY_RIGHT])
		qc->SetPosition(qcPos - qcLDir*moveSpeed*dt);
	if (keyState[GLFW_KEY_LEFT])
		qc->SetPosition(qcPos + qcLDir*moveSpeed*dt);


	const float orientSpeed = 1.f;
	const double P = qc->GetPitch();
	const double R = qc->GetRoll();
	const double Y = qc->GetYaw();

	if (keyState[GLFW_KEY_I])
		qc->SetPitch(P + orientSpeed * dt);
	if (keyState[GLFW_KEY_K])
		qc->SetPitch(P - orientSpeed * dt);
	if (keyState[GLFW_KEY_L])
		qc->SetRoll(R + orientSpeed * dt);
	if (keyState[GLFW_KEY_J])
		qc->SetRoll(R - orientSpeed * dt);
	if (keyState[GLFW_KEY_LEFT_BRACKET])
		qc->SetYaw(Y + orientSpeed * dt);
	if (keyState[GLFW_KEY_RIGHT_BRACKET])
		qc->SetYaw(Y - orientSpeed * dt);

	qc -> Update(dt);
}

void UpdateFreeCamera(double dt) {
	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);
	const double dx = lastMouseX - xpos;
	const double dy = lastMouseY - ypos;
	lastMouseX = xpos;
	lastMouseY = ypos;
	
	if (ignoreInputsThisFrame)
		return;
	
	const float mouseLookSpeed = 5.f;
	cam.RotateYaw(dx * dt * mouseLookSpeed);
	cam.RotatePitch(dy * dt * mouseLookSpeed);

	const float moveSpeed = 5.f;
	if (keyState[GLFW_KEY_W])
		cam.MoveForward(moveSpeed * dt);
	if (keyState[GLFW_KEY_S])
		cam.MoveForward(-moveSpeed * dt);
	if (keyState[GLFW_KEY_D])
		cam.MoveRight(moveSpeed * dt);
	if (keyState[GLFW_KEY_A])
		cam.MoveRight(-moveSpeed * dt);	

	viewMatrix = cam.GetMat();
	projMatrix = freeCamProjMat;
}

void UpdateApp() {
	float currTime;

	if (visualMode) {
		// Compute time difference between current and last frame
		currTime = glfwGetTime();
		const float dt = currTime - lastTime;
		
		// Update Camera
		if (appState == freeCam)
			UpdateFreeCamera(dt);
		else if (appState == fixedCam)
			CalculateRenderCamera(currCamNum);
	
		// Only update quadcopter if we are with in time limits. If the current time
		// is after the end time, the app will continue to run but the quadcopter
		// must stop updating.
		if (endTime == 0 || currTime < endTime)
			UpdateQuadcopter(dt);

		if (ignoreInputsThisFrame)
			ignoreInputsThisFrame = false;
	
		UpdateMVP();
	} else {
		currTime = lastTime + timestep;

		if (endTime == 0 || currTime < endTime)
			UpdateQuadcopter(timestep);
		else {
			shouldQuit = true;
			CleanupQCControls();
		}
	}	

	lastTime = currTime;
}

void errorCallback(int errNum, const char* desc) {
	fprintf(stderr, "Error no. %d: %s\n", errNum, desc);
}

void windowCloseCallback(GLFWwindow* window) {
	shouldQuit = true;
}

void keyboardCallback(GLFWwindow* win, int key, int scancode, int action, int mods) {
	keyState[key] = action != GLFW_RELEASE;

	if (appState == exportIMG) {
		// Disable keyboard controls in image export mode
		return;
	}


	if (key == GLFW_KEY_ENTER && action == GLFW_PRESS) {
		if (appState == freeCam) {
			glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			appState = fixedCam;
		} else {			
			glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			glfwGetCursorPos(win, &lastMouseX, &lastMouseY);
			appState = freeCam;
		}
	}

	if (appState == fixedCam && action == GLFW_PRESS) {
		if (key == GLFW_KEY_RIGHT)
			currCamNum = (currCamNum + 1) % capturePositions.size();

		if (key == GLFW_KEY_LEFT)
			currCamNum = (currCamNum==0)?(capturePositions.size()-1):(currCamNum-1);
	}

	if (key == GLFW_KEY_G && action == GLFW_PRESS) {
		ExportImages();

		// Read mouse inputs to clear any movements that may have been made
		// while writing images to disk.
		ignoreInputsThisFrame = true;
	}

	// Reset application
	if (appState == freeCam && key == GLFW_KEY_R && action == GLFW_PRESS) {
		InitQuadcopter();
	}

	if (keyState[GLFW_KEY_ESCAPE])
		shouldQuit = true;
}

void mouseMotionCallback(GLFWwindow* win, double xpos, double ypos) {
	
}