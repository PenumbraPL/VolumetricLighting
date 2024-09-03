#include "IO.h"
#include "pch.h"
#include "GUI.h"

extern ConfigContext panelConfig;
extern WindowInfo windowConfig;


namespace control
{
    void scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
    {
        if (!panelConfig.focused) {
            panelConfig.viewDistance += (float)(yoffset * panelConfig.viewDistance / -6.);
        }
    }
    //static
    void cursorPositionCallback(GLFWwindow* window, double new_xpos, double new_ypos)
    {
        if (!panelConfig.focused) {
            int state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
            if (state != GLFW_RELEASE) {
                double nx = (windowConfig.mouseSpeed / windowConfig.width) * (new_xpos - windowConfig.xCursorPosition);
                double ny = (windowConfig.mouseSpeed / windowConfig.height) * (new_ypos - windowConfig.yCursorPosition);
                panelConfig.viewPhi += (float) nx;
                panelConfig.viewTheta += (float) ny;

                windowConfig.xCursorPosition = new_xpos;
                windowConfig.yCursorPosition = new_ypos;
            }
        }
    }

    void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
    {
        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
            glfwGetCursorPos(window, &windowConfig.xCursorPosition, &windowConfig.yCursorPosition);
        }
    }

    void keyCallback(
        GLFWwindow* window,
        int key,
        int scancode,
        int action,
        int mods)
    {
        if (!panelConfig.focused) {
            if (key == GLFW_KEY_LEFT && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
                panelConfig.viewPhi += 0.01f;
            }
            if (key == GLFW_KEY_RIGHT && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
                panelConfig.viewPhi -= 0.01f;
            }
            if (key == GLFW_KEY_UP && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
                panelConfig.viewTheta += 0.01f;
            }
            if (key == GLFW_KEY_DOWN && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
                panelConfig.viewTheta -= 0.01f;
            }
            if (key == GLFW_KEY_A && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
                panelConfig.zTranslate += 1;
            }
            if (key == GLFW_KEY_D && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
                panelConfig.zTranslate -= 1;
            }
            if (key == GLFW_KEY_W && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
                panelConfig.xTranslate -= 1;
            }
            if (key == GLFW_KEY_S && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
                panelConfig.xTranslate += 1;
            }
            if (key == GLFW_KEY_H && action == GLFW_PRESS) {
                int mode[2] = { GLFW_CURSOR_DISABLED, GLFW_CURSOR_NORMAL };
                windowConfig.cursorMode += 1;
                windowConfig.cursorMode %= 2;
                glfwSetInputMode(window, GLFW_CURSOR, mode[windowConfig.cursorMode % (sizeof(mode) / sizeof(int))]);
            }
        }
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
    }

    void focusCallback(GLFWwindow* window, int focused)
    {
        panelConfig.focused = focused ? true : false;
    }
}
