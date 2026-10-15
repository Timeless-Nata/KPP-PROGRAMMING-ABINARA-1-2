ting#include <mujoco/mujoco.h>
#include <GLFW/glfw3.h>
#include <cstdio>
#include <vector>
#include <algorithm>

bool button_left = false;
bool button_middle = false;
bool button_right = false;
double lastx = 0.0, lasty = 0.0;

void mouse_button_callback(GLFWwindow* window, int button, int act, int mods) {
    button_left = (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS);
    button_middle = (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS);
    button_right = (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS);

    glfwGetCursorPos(window, &lastx, &lasty);
}

int main() {
    mjModel* m = nullptr;
    mjData* d = nullptr;
    mjvScene scn;
    mjvCamera cam;
    mjvOption opt;
    mjrContext con;

    char error[1000] = "Couldn't load model";
    m = mj_loadXML("main.xml", nullptr, error, sizeof(error));
    if (!m) {
        fprintf(stderr, "%s\n", error);
        return 1;
    }

    d = mj_makeData(m);

    glfwSetErrorCallback([](int error, const char* description) {
            fprintf(stderr, "GLFW Error (%d): %s\n", error, description);
        });

    GLFWwindow* window = glfwCreateWindow(1200, 900, "Arm 4DOF Sim", nullptr, nullptr);
    if(!window) {
        fprintf(stderr, "glfwCreateWindow() gagal\n");
        glfwTerminate();
        return 1;
    }

    glfwSetMouseButtonCallback(window, mouse_button_callback);

}
