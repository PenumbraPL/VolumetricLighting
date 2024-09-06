#include "IO.h"
#include "pch.h"
#include "GUI.h"

extern GUI myGui;
extern WindowInfo windowConfig;


namespace control
{
    void scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
    {
        if (!myGui.workSpaceFocused) {
            myGui.viewDistance.data += (float)(yoffset * myGui.viewDistance / -6.);
            myGui.viewDistance.notifyAll();
        }
    }
    //static
    void cursorPositionCallback(GLFWwindow* window, double new_xpos, double new_ypos)
    {
        if (!myGui.workSpaceFocused) {
            int state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
            if (state != GLFW_RELEASE) {
                double nx = (windowConfig.mouseSpeed / windowConfig.width) * (new_xpos - windowConfig.xCursorPosition);
                double ny = (windowConfig.mouseSpeed / windowConfig.height) * (new_ypos - windowConfig.yCursorPosition);
                myGui.viewPhi.data += (float) nx;
                myGui.viewTheta.data += (float) ny;
                myGui.viewPhi.notifyAll();
                myGui.viewTheta.notifyAll();

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
        if (!myGui.workSpaceFocused) {
            if (key == GLFW_KEY_LEFT && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
                myGui.viewPhi.data += 0.01f;
                myGui.viewPhi.notifyAll();
            }
            if (key == GLFW_KEY_RIGHT && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
                myGui.viewPhi.data -= 0.01f;
                myGui.viewPhi.notifyAll();
            }
            if (key == GLFW_KEY_UP && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
                myGui.viewTheta.data += 0.01f;
                myGui.viewTheta.notifyAll();
            }
            if (key == GLFW_KEY_DOWN && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
                myGui.viewTheta.data -= 0.01f;
                myGui.viewTheta.notifyAll();
            }
            if (key == GLFW_KEY_A && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
                myGui.zTranslate.data += 1;
                myGui.zTranslate.notifyAll();
            }
            if (key == GLFW_KEY_D && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
                myGui.zTranslate.data -= 1;
                myGui.zTranslate.notifyAll();
            }
            if (key == GLFW_KEY_W && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
                myGui.xTranslate.data -= 1;
                myGui.xTranslate.notifyAll();
            }
            if (key == GLFW_KEY_S && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
                myGui.xTranslate.data += 1;
                myGui.xTranslate.notifyAll();
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
        myGui.workSpaceFocused = focused ? true : false;
    }
}
