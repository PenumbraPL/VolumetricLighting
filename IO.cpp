#include "IO.h"
#include "GUI.h"
#include "pch.h"

extern ConfigContext panel_config;
extern float mouse_speed;
extern double xpos;
extern double ypos;

// multiple definitions
struct WindowInfo {
    int width;
    int height;
    const char* title;
    int cursor_mode;
    int imgui;
    int mbutton;
};

extern WindowInfo windowConfig;

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    panel_config.dist += yoffset * panel_config.dist / -6.;
}

//static
void cursor_position_callback(GLFWwindow* window, double new_xpos, double new_ypos)
{
    if (!panel_config.focused1 && !panel_config.focused2) {
        int state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
        if (state != GLFW_RELEASE)
        {
            double nx = (mouse_speed / windowConfig.width) * (new_xpos - xpos);
            double ny = (mouse_speed / windowConfig.height) * (new_ypos - ypos);
            panel_config.phi += nx;
            panel_config.theta += ny;

            xpos = new_xpos;
            ypos = new_ypos;
        }
    }
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        glfwGetCursorPos(window, &xpos, &ypos);
    }
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_LEFT && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
        panel_config.phi += 0.01;
    }
    if (key == GLFW_KEY_RIGHT && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
        panel_config.phi -= 0.01;
    }
    if (key == GLFW_KEY_UP && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
        panel_config.theta += 0.01;
    }
    if (key == GLFW_KEY_DOWN && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
        panel_config.theta -= 0.01;
    }
    if (key == GLFW_KEY_A && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
        panel_config.tr_z += 1;
    }
    if (key == GLFW_KEY_D && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
        panel_config.tr_z -= 1;
    }
    if (key == GLFW_KEY_W && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
        panel_config.tr_x -= 1;
    }
    if (key == GLFW_KEY_S && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
        panel_config.tr_x += 1;
    }
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
    if (key == GLFW_KEY_H && action == GLFW_PRESS) {
        int mode[2] = { GLFW_CURSOR_DISABLED, GLFW_CURSOR_NORMAL };
        windowConfig.cursor_mode += 1;
        windowConfig.cursor_mode %= 2;
        glfwSetInputMode(window, GLFW_CURSOR, mode[windowConfig.cursor_mode % (sizeof(mode) / sizeof(int))]);
    }
}

void focus_callback(GLFWwindow* window, int focused)
{
    panel_config.focused1 = focused ? true : false;
    panel_config.focused2 = focused ? true : false;
}