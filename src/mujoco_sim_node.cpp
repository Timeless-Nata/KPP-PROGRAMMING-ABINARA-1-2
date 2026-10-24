#include <GLFW/glfw3.h>
#include <mujoco/mujoco.h>
#include <ros/ros.h>
#include <sensor_msgs/JointState.h>
#include <std_msgs/Float64MultiArray.h>

#include <string>

// Global karena fungsi callback ROS tidak bisa menerima argumen tambahan.
mjModel* m = nullptr;
mjData* d = nullptr;

// Dipanggil ros::spinOnce() setiap ada pesan di joint_cmd.
// Isi pesan = sudut target (rad) per actuator, urutannya sama dengan <actuator> di main.xml.
void jointCmdCallback(const std_msgs::Float64MultiArray::ConstPtr& msg) {
  if (static_cast<int>(msg->data.size()) != m->nu) {
    ROS_WARN_THROTTLE(1.0, "joint_cmd harus berisi %d nilai, diterima %zu", m->nu,
                      msg->data.size());
    return;
  }
  for (int i = 0; i < m->nu; ++i) d->ctrl[i] = msg->data[i];
}

// Asumsi: semua joint adalah hinge (1 DOF), sesuai main.xml.
sensor_msgs::JointState makeJointState() {
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
  m = mj_loadXML(model_path.c_str(), nullptr, error, sizeof(error));
  if (!m) {
    ROS_FATAL("Gagal load model: %s", error);
    return 1;
  }
  d = mj_makeData(m);
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
  ros::Subscriber cmd_sub = nh.subscribe("joint_cmd", 1, jointCmdCallback);

  const double wall_start = glfwGetTime();

  while (ros::ok() && !glfwWindowShouldClose(window)) {
    ros::spinOnce();  // di sini jointCmdCallback dipanggil kalau ada pesan masuk

    const double target_time = glfwGetTime() - wall_start;
    while (d->time < target_time) mj_step(m, d);

    joint_pub.publish(makeJointState());

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
