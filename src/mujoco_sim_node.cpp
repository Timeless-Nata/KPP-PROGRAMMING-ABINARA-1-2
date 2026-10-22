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

  ROS_INFO("Model dimuat: %d joint, %d actuator", m->njnt, m->nu);
  for (int i = 0; i < m->njnt; ++i) {
    ROS_INFO("joint %d: %s", i, mj_id2name(m, mjOBJ_JOINT, i));
  }

  mj_deleteModel(m);
  return 0;
}
