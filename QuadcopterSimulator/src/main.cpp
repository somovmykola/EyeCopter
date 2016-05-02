#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <string>

#include "AppControls.h"
#include "Drawing.h"
#include "ImageRender.h"
#include "Settings.h"

extern GLFWwindow* window;

// Controls / State
extern bool shouldQuit;

Settings settings;

int main(int argc, char** argv) {
	settings.LoadFromFile("Settings.txt");

	bool visualMode;
	std::string visualStr = settings.GetString("simulator_visual");
	if (visualStr == "on")
		visualMode = true;
	else if (visualStr == "off")
		visualMode = false;

	if (visualMode) {
		int err = InitGL();

		if (err)
			return err;
	}

	InitApp();

	if (visualMode)
		InitDrawing();

	CalculateCameraPath();

	while(!shouldQuit) {
		UpdateApp();

		if (visualMode) {
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			DrawScene();		

			glfwSwapBuffers(window);
			glfwPollEvents();
		}
	}

	if (visualMode) {
		CleanupDrawing();
		CleanupGL();
	}

	CleanupApp();

	if (visualMode)
		glfwTerminate();

	//if (!visualMode)
	//	getchar();

	return 0;
}