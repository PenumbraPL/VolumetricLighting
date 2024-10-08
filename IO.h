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
	void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
	void cursor_position_callback(GLFWwindow* window, double new_xpos, double new_ypos);
	void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
	void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
	void focus_callback(GLFWwindow* window, int focused);
}
