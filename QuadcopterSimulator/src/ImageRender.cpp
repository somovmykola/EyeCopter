#include <algorithm>    // for find
#include <cctype>		// for tolower
#include <fstream>
#include <string>
#include <sys/types.h>	// for directory checking
#include <sys/stat.h>
#include <vector>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#define _USE_MATH_DEFINES
#include "math.h"
#include <windows.h>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/vector_angle.hpp>

#include "Camera3D.h"
#include "AppControls.h"
#include "Drawing.h"
#include "Noise.h"
#include "Settings.h"

using namespace std;
using glm::mat4;
using glm::vec3;

// Controls / State
extern unsigned int appState;

// Drawing
extern mat4 viewMatrix, projMatrix;

// Noise
extern Noise* noiseGen;	// Why use on stack? singleton pattern maybe?

// Misc
extern Settings settings;
vector<Camera3D> capturePositions;

// FBO
GLuint frameBuffer, renderTexture, depthRenderBuffer;
GLenum drawBuffers[1];
unsigned int imgW, imgH;

mat4 GetNoiseTransformMat() {
	const mat4 iden = mat4(1.f);
	const vec3 pitchAxis = vec3(-1,0,0);
	const vec3 yawAxis = vec3(0,1,0);
	const vec3 rollAxis = vec3(0,0,1);

	const float pitchAng = glm::radians(noiseGen->GetVal(Noise::pitch));
	const float yawAng = glm::radians(noiseGen->GetVal(Noise::yaw));
	const float rollAng = glm::radians(noiseGen->GetVal(Noise::roll));

	const mat4 pitchMat = glm::rotate(iden, pitchAng, pitchAxis);
	const mat4 yawMat = glm::rotate(iden, yawAng, yawAxis);
	const mat4 rollMat = glm::rotate(iden, rollAng, rollAxis);

	return rollMat * yawMat * pitchMat;
}

// Maybe move this to its own file
void CalculateRenderCamera(int camNum) {
	const vec3 pos = capturePositions[camNum].GetPos();
	const vec3 dir = capturePositions[camNum].GetDir();

	viewMatrix = glm::lookAt(pos, pos+dir, vec3(0,1,0));
	viewMatrix = GetNoiseTransformMat() * viewMatrix;

	const float fov = glm::radians(settings.GetDouble("camera_fov"));
	const float ratio = settings.GetDouble("image_h_res") / settings.GetDouble("image_v_res");

	projMatrix = glm::perspective(fov, ratio, 0.1f, 100.0f);
}

void CalculateCameraPath() {
	const unsigned int numPoints = settings.GetInt("camera_capture_num");
	const double dAng = 2*M_PI / (double)numPoints;
	const double camY = settings.GetDouble("camera_path_y");
	const double camR = settings.GetDouble("camera_path_r");
	vec3 v (0.0, camY, 0.0);

	capturePositions.clear();
	double ang = 0.0;
	for (int i = 0; i < numPoints; i++, ang += dAng) {
		v.x = camR * cos(ang);
		v.z = camR * sin(ang);
		capturePositions.push_back(Camera3D(v, -v));
	}
}

string tolower(string s) {
	string lowS(" ", s.length());	// init new string as spaces of equal length to s
	for (int i = 0; i < s.length(); i++)
		lowS[i] = tolower(s[i]);

	return lowS;
}

bool DirExists(string dir) {
	struct stat info;

	if (stat(dir.c_str(), &info) != 0)
		return false;
	else if (info.st_mode & S_IFDIR)
		return true;
	
	return false;
}

// Sanitize image path to remove trailing slashes
string GetCorrectedImagePath() {
	string path = settings.GetString("image_path");

	return path.substr(0, path.find_last_not_of("/")+1);
}


void CleanupFBO() {
	glDeleteFramebuffers(1, &frameBuffer);
	glDeleteTextures(1, &renderTexture);
	glDeleteRenderbuffers(1, &depthRenderBuffer);
}

int SetupFBO() {
	// Set up framebuffer
	frameBuffer = 0;
	glGenFramebuffers(1, &frameBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);

	// Set up render target texture
	glGenTextures(1, &renderTexture);
	glBindTexture(GL_TEXTURE_2D, renderTexture);

	// Get image dimensions
	imgW = settings.GetInt("image_h_res");
	imgH = settings.GetInt("image_v_res");;

	// Allocate space for an empty image, last 0 is init data
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, imgW, imgH, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

	// Poor filtering. Needed !
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	// Need a depth buffer to render 3d images
	glGenRenderbuffers(1, &depthRenderBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, depthRenderBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, imgW, imgH);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRenderBuffer);

	// Configure framebuffer
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, renderTexture, 0);

	// Set list of draw buffers
	drawBuffers[0] = GL_COLOR_ATTACHMENT0;
	glDrawBuffers(1, drawBuffers);

	// Error checking
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		return 1;

	return 0;
}

int ExportImages() {
	const string imgPath = GetCorrectedImagePath();

	// Check if image path exists
	if (DirExists(imgPath) == false) {
		fprintf(stderr, "Directory \"%s\" does not exist!\n", imgPath.c_str());
		return 1;
	}

	const string imgPrefix = settings.GetString("image_prefix");
	const string imgExt = settings.GetString("image_filetype");
	const string imgExtLower = tolower(imgExt);

	// Check if supplied image extension is valid
	const vector<string> allowedExt = {"bmp", "tga", "png"};
	if (find(allowedExt.begin(), allowedExt.end(), imgExtLower) == allowedExt.end()) {
		fprintf(stderr, "Invalid image extension \"%s\"\n", imgExt.c_str());
		return 1;
	}
	
	// Save current viewport settings
	GLint oldViewport[4];
	glGetIntegerv(GL_VIEWPORT, oldViewport);
	
	if (SetupFBO()) {
		fprintf(stderr, "Framebuffer problemo!\n");
		CleanupFBO();
		return 1;
	}

	unsigned int oldState = appState;
	appState = exportIMG;
	unsigned char *data = (unsigned char*)malloc(imgW*imgH*4);
		
	for (int i = 0; i < capturePositions.size(); i++) {
		// Calculate view + projection matrix for the current camera
		CalculateRenderCamera(i);
		// Flip y axis so to account for opengl texture coordinates
		projMatrix = glm::scale(projMatrix, vec3(1.0, -1.0, 1.0));
		
		glViewport(0, 0, imgW, imgH);

		// Set openGL to render to texture
		glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Draw the scene. Will know the appropriate options since appState is
		// set to export images
		DrawScene();

		// Write to file
		glReadPixels(0, 0, imgW, imgH, GL_RGBA, GL_UNSIGNED_BYTE, data);
		
		// Create filename with path, image prefix, picture number, and extension
		char filename[100];
		sprintf(filename, "%s/%s%02d.%s", imgPath.c_str(), imgPrefix.c_str(), i, imgExt.c_str());
		

		//Encode the image
		int comp = 4;	// 3 = RGB, 4 = RGBA
		unsigned error;
		
		if (imgExtLower == "png")
			error = stbi_write_png(filename, imgW, imgH, comp, data, 0);
		else if (imgExtLower == "bmp")
			error = stbi_write_bmp(filename, imgW, imgH, comp, data);
		else if (imgExtLower == "tga")
			error = stbi_write_tga(filename, imgW, imgH, comp, data);

		if (!error)
			fprintf(stderr, "Error writing to file \"%s\".\n", filename);
	}
	free(data);

	appState = oldState;

	CleanupFBO();

	// restore old viewport
	glViewport(oldViewport[0], oldViewport[1], oldViewport[2], oldViewport[3]);

	return 0;
}