#include <GLFW/glfw3.h>
#include <geometry_msgs/PointStamped.h>
#include <mujoco/mujoco.h>
#include <ros/ros.h>
#include <sensor_msgs/JointState.h>
#include <std_msgs/Float64MultiArray.h>

using namespace ros;
using namespace std;

int main(int argc, char** argv) {
    init(argc, argv, "ik_controller");
    NodeHandle nh;
    NodeHandle pnh("~");

    // cuma nyimpen path doang
    string model_path;
    if (!pnh.getParam("model_path", model_path)) {
        ROS_FATAL("Model_path gagal dimuat");
        return 1;
    }

    const double damping = 0.01;
    const double max_step = 0.01;
    const double rate_hz = 100.0;

    // list argumen: mj_loadXML(filename, vfs, error, error_sz);
    char error[1000] = "";
    mjModel* m = mj_loadXML(model_path.c_str(), nullptr, error, sizeof(error));
    if (!m) {
        ROS_FATAL("Gagal load model: %s", error);
        return 1;
    }
    mjData* d = mj_makeData(m);

    // ngambil id site
    const int site = mj_name2id(m, mjOBJ_SITE, "ee");
    if (site < 0) {
      ROS_FATAL("Site 'ee' tidak ditemukan");
      return 1;
    }

    // deklarasi pose awal
    double q[4] = {0.0, 0.0, 0.0, 0.0};

    Publisher pub = nh.advertise<std_msgs::Float64MultiArray>("joint_cmd", 1);
    Rate rate(rate_hz);
    vector<double> jacp(3 * m->nv);

    while (ok()) {
      spinOnce();

      // 1. Pasang sudut q ke model, hitung posisi ujung lengan (kinematika).
      for (int k = 0; k < 4; ++k) d->qpos[m->jnt_qposadr[jid[k]]] = q[k];
      mj_kinematics(m, d);
      mj_comPos(m, d);


}
