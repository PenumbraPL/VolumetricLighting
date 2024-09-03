#pragma once
#include "GLFW/glfw3.h"

struct WindowInfo {
    int width;
    int height;
    const char* title;
    int cursorMode = 0;
	float mouseSpeed = 2.f;
	double xCursorPosition = 0.;
	double yCursorPosition = 0.;
};

namespace control 
{
	void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
	void cursorPositionCallback(GLFWwindow* window, double new_xpos, double new_ypos);
	void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
	void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
	void focusCallback(GLFWwindow* window, int focused);
}
