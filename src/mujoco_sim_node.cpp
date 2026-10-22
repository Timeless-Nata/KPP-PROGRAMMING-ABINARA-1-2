#include <GLFW/glfw3.h>
#include <mujoco/mujoco.h>
#include <ros/ros.h>
#include <sensor_msgs/JointState.h>

#include <string>

// Mengubah keadaan joint di MuJoCo menjadi pesan ROS.
// Asumsi: semua joint adalah hinge (1 DOF), sesuai main.xml.
sensor_msgs::JointState makeJointState(const mjModel* m, const mjData* d) {
  sensor_msgs::JointState js;
  js.header.stamp = ros::Time::now();
  for (int i = 0; i < m->njnt; ++i) {
    js.name.push_back(mj_id2name(m, mjOBJ_JOINT, i));
    js.position.push_back(d->qpos[m->jnt_qposadr[i]]);
    js.velocity.push_back(d->qvel[m->jnt_dofadr[i]]);
  }
  return js;
}

int main(int argc, char** argv) {
  ros::init(argc, argv, "mujoco_sim");
  ros::NodeHandle nh;
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

  ros::Publisher joint_pub = nh.advertise<sensor_msgs::JointState>("joint_states", 10);

  const double wall_start = glfwGetTime();

  while (ros::ok() && !glfwWindowShouldClose(window)) {
    ros::spinOnce();

    const double target_time = glfwGetTime() - wall_start;
    while (d->time < target_time) mj_step(m, d);

    joint_pub.publish(makeJointState(m, d));

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
