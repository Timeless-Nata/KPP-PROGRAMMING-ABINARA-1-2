#include <mujoco/mujoco.h>
#include <ros/ros.h>
#include <GLFW/glfw3.h>
#include <cstdio>
#include <vector>
#include <algorithm>

int main(int argc, char** argv) {
  ros::init(argc, argv, "keyboard_input");
  ros::NodeHandle nh;
  ros::NodeHandle pnh("~");

  std::string model_path;
  if (!pnh.getParam("model_path", model_path)) {
    ROS_FATAL("Parameter ~model_path belum di-set");
    return 1;
  }

  char error[1000] = "";
  m = mj_loadXML(model_path.c_str(), nullptr, error, sizeof(error));
  if (!m) {
    ROS_FATAL("Gagal load model: %s", error);
    return 1;
  }
  d = mj_makeData(m);
  ROS_INFO("Model dimuat: %d joint, %d actuator", m->njnt, m->nu);

  ee_site = mj_name2id(m, mjOBJ_SITE, "ee");
  if (ee_site < 0) {
    ROS_FATAL("Site 'ee' tidak ditemukan di model");
    return 1;
  }

  if (!glfwInit()) {
    ROS_FATAL("glfwInit() gagal");
    return 1;
  }
  GLFWwindow* window = glfwCreateWindow(1200, 900, "Arm 4DOF Sim", nullptr, nullptr);
  if (!window) {
    ROS_FATAL("glfwCreateWindow() gagal");
    glfwTerminate();
    return 1;
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
}
