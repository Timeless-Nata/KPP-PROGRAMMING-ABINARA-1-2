#include <GLFW/glfw3.h>
#include <mujoco/mujoco.h>
#include <ros/ros.h>

#include <string>

int main(int argc, char** argv) {
  ros::init(argc, argv, "mujoco_sim");
  ros::NodeHandle pnh("~");

  std::string model_path;
  if (!pnh.getParam("model_path", model_path)) {
    ROS_FATAL("Parameter ~model_path belum di-set");
    return 1;
  }

  char error[1000] = "";
  mjModel* m = mj_loadXML(model_path.c_str(), nullptr, error, sizeof(error));
  if (!m) {
    ROS_FATAL("Gagal load model: %s", error);
    return 1;
  }
  mjData* d = mj_makeData(m);
  ROS_INFO("Model dimuat: %d joint, %d actuator", m->njnt, m->nu);

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
  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);

  mjvCamera cam;
  mjvOption opt;
  mjvScene scn;
  mjrContext con;
  mjv_defaultCamera(&cam);
  mjv_defaultOption(&opt);
  mjv_defaultScene(&scn);
  mjr_defaultContext(&con);
  mjv_makeScene(m, &scn, 2000);
  mjr_makeContext(m, &con, mjFONTSCALE_150);
  cam.distance = 1;

  const double wall_start = glfwGetTime();

  while (ros::ok() && !glfwWindowShouldClose(window)) {
    ros::spinOnce();

    // Jalankan simulasi sampai waktu simulasi menyusul waktu nyata.
    const double target_time = glfwGetTime() - wall_start;
    while (d->time < target_time) mj_step(m, d);

    mjrRect viewport = {0, 0, 0, 0};
    glfwGetFramebufferSize(window, &viewport.width, &viewport.height);
    mjv_updateScene(m, d, &opt, nullptr, &cam, mjCAT_ALL, &scn);
    mjr_render(viewport, &scn, &con);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  mjv_freeScene(&scn);
  mjr_freeContext(&con);
  mj_deleteData(d);
  mj_deleteModel(m);
  glfwTerminate();
  return 0;
}
