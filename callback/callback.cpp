#include <mujoco/mujoco.h>
#include <GLFW/glfw3.h>
#include <cstdio>
#include <vector>
#include <algorithm>
using namespace std;

mjModel* m = nullptr;
mjData* d = nullptr;

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

void print_joint_feedback() {
    const char* nama_sensor[] = {
        "muter_pos",
        "bahu_pos",
        "lengan_pos",
        "telapak_pos",
        "jari1_pos",
        "jari2_pos",
        "jaribawah1_pos",
        "jaribawah2_pos"
    };

    printf("\n=== Posisi Joint ===\n");

    for (const char* name : nama_sensor) {
        int id_sensor = mj_name2id(
            m, mjOBJ_SENSOR, name
        );

        if (id_sensor == -1) {
            printf("%s: sensor tidak ditemukan\n", name);
            continue;
        }

        int adr = m->sensor_adr[id_sensor];

        printf("%s: %.4f\n",
               name, d->sensordata[adr]);
    }
}

void keyboard_callback(GLFWwindow* window, int key, int scancode, int act, int mods) {
    if (act != GLFW_PRESS) return;
    double step = 0.1;
    switch (key) {
        case GLFW_KEY_Q:
            d->ctrl[0] += step;
            printf("q key ditekan\n");
            break;

        case GLFW_KEY_A:
            d->ctrl[0] -= step;
            printf("a key ditekan\n");
            break;

        case GLFW_KEY_W:
            d->ctrl[1] += step;
            printf("w key ditekan\n");
            break;

        case GLFW_KEY_S:
            d->ctrl[1] -= step;
            printf("s key ditekan\n");
            break;

        case GLFW_KEY_E:
            d->ctrl[2] += step;
            printf("e key ditekan\n");
            break;

        case GLFW_KEY_D:
            d->ctrl[2] -= step;
            printf("d key ditekan\n");
            break;

        case GLFW_KEY_R:
            d->ctrl[3] += step;
            printf("r key ditekan\n");
            break;

        case GLFW_KEY_F:
            d->ctrl[3] -= step;
            printf("f key ditekan\n");
            break;

        case GLFW_KEY_SPACE:
            mj_resetData(m, d);
            printf("Ulang simulasi\n");
            break;

        case GLFW_KEY_LEFT_SHIFT:
            print_joint_feedback();
            break;
    }
}

int main() {
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

    if (!glfwInit()) {
            fprintf(stderr, "glfwInit() gagal\n");
            return 1;
        }

    GLFWwindow* window = glfwCreateWindow(1200, 900, "Arm 4DOF Sim", nullptr, nullptr);
    if(!window) {
        fprintf(stderr, "glfwCreateWindow() gagal\n");
        glfwTerminate();
        return 1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    glfwSetMouseButtonCallback(window, mouse_button_callback);

    mjv_defaultCamera(&cam);
    mjv_defaultOption(&opt);
    mjv_defaultScene(&scn);
    mjr_defaultContext(&con);

    mjv_makeScene(m, &scn, 2000);
    mjr_makeContext(m, &con, mjFONTSCALE_150);

    cam.distance = 1;

    while (!glfwWindowShouldClose(window)) {
        mj_step(m, d);

        mjrRect viewport = {0, 0, 0, 0};
        glfwGetFramebufferSize(window, &viewport.width, &viewport.height);

        mjv_updateScene(m, d, &opt, nullptr, &cam, mjCAT_ALL, &scn);
        mjr_render(viewport, &scn, &con);

        glfwSwapBuffers(window);
        glfwPollEvents();
        glfwSetMouseButtonCallback(window, mouse_button_callback);
        glfwSetKeyCallback(window, keyboard_callback);
    }
}
