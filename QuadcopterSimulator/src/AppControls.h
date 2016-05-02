#ifndef CONTROLS_H
#define CONTROLS_H

#include <GLFW/glfw3.h>

enum eAppState {
	freeCam,
	fixedCam,
	exportIMG
};

int InitApp();
int CleanupApp();

void UpdateApp();

void errorCallback(int errNum, const char* desc);
void windowCloseCallback(GLFWwindow* window);
void keyboardCallback(GLFWwindow* win, int key, int scancode, int action, int mods);
void mouseMotionCallback(GLFWwindow* win, double xpos, double ypos);



#endif