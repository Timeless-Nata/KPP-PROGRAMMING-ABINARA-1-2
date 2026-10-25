// Node IK: menerima titik target (ee_target), menghitung sudut sendi dengan
// Jacobian MuJoCo (damped least squares), lalu mengirimnya ke joint_cmd.
// Node ini hanya memakai kinematika MuJoCo (tanpa fisika, tanpa jendela).

#include <geometry_msgs/PointStamped.h>
#include <mujoco/mujoco.h>
#include <ros/ros.h>
#include <std_msgs/Float64MultiArray.h>

#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

namespace {

geometry_msgs::Point target;
bool has_target = false;

void targetCallback(const geometry_msgs::PointStamped::ConstPtr& msg) {
  target = msg->point;
  has_target = true;
}

// Menyelesaikan A * y = b untuk matriks 3x3 (eliminasi Gauss, pivot sebagian).
bool solve3(const double A[3][3], const double b[3], double y[3]) {
  double M[3][4];
  for (int i = 0; i < 3; ++i) {
    for (int j = 0; j < 3; ++j) M[i][j] = A[i][j];
    M[i][3] = b[i];
  }
  for (int c = 0; c < 3; ++c) {
    int p = c;
    for (int r = c + 1; r < 3; ++r)
      if (std::fabs(M[r][c]) > std::fabs(M[p][c])) p = r;
    if (std::fabs(M[p][c]) < 1e-12) return false;
    for (int j = 0; j < 4; ++j) std::swap(M[c][j], M[p][j]);
    for (int r = c + 1; r < 3; ++r) {
      const double f = M[r][c] / M[c][c];
      for (int j = c; j < 4; ++j) M[r][j] -= f * M[c][j];
    }
  }
  for (int i = 2; i >= 0; --i) {
    double s = M[i][3];
    for (int j = i + 1; j < 3; ++j) s -= M[i][j] * y[j];
    y[i] = s / M[i][i];
  }
  return true;
}

}  // namespace

int main(int argc, char** argv) {
  ros::init(argc, argv, "ik_controller");
  ros::NodeHandle nh;
  ros::NodeHandle pnh("~");

  std::string model_path;
  if (!pnh.getParam("model_path", model_path)) {
    ROS_FATAL("Parameter ~model_path belum di-set");
    return 1;
  }
  const double damping = pnh.param("damping", 0.01);    // peredam dekat titik singular
  const double max_step = pnh.param("max_step", 0.01);  // maks. langkah per iterasi (m)
  const double rate_hz = pnh.param("rate", 100.0);

  char error[1000] = "";
  mjModel* m = mj_loadXML(model_path.c_str(), nullptr, error, sizeof(error));
  if (!m) {
    ROS_FATAL("Gagal load model: %s", error);
    return 1;
  }
  mjData* d = mj_makeData(m);

  // Empat sendi lengan, urutannya sama dengan 4 actuator pertama di main.xml.
  const char* names[4] = {"hip", "shoulder", "arm", "hand"};
  int jid[4];
  for (int k = 0; k < 4; ++k) {
    jid[k] = mj_name2id(m, mjOBJ_JOINT, names[k]);
    if (jid[k] < 0) {
      ROS_FATAL("Joint '%s' tidak ditemukan", names[k]);
      return 1;
    }
  }
  const int site = mj_name2id(m, mjOBJ_SITE, "ee");
  if (site < 0) {
    ROS_FATAL("Site 'ee' tidak ditemukan");
    return 1;
  }

  // Pose awal: menekuk, jauh dari posisi lurus penuh (titik singular).
  double q[4] = {0.0, -0.93, 1.57, 0.0};

  ros::Subscriber sub = nh.subscribe("ee_target", 1, targetCallback);
  ros::Publisher pub = nh.advertise<std_msgs::Float64MultiArray>("joint_cmd", 1);
  ros::Rate rate(rate_hz);
  std::vector<double> jacp(3 * m->nv);

  while (ros::ok()) {
    ros::spinOnce();

    // 1. Pasang sudut q ke model, hitung posisi ujung lengan (kinematika).
    for (int k = 0; k < 4; ++k) d->qpos[m->jnt_qposadr[jid[k]]] = q[k];
    mj_kinematics(m, d);
    mj_comPos(m, d);

    if (has_target) {
      // 2. Selisih antara target dan posisi ujung lengan sekarang.
      const double* ee = d->site_xpos + 3 * site;
      double e[3] = {target.x - ee[0], target.y - ee[1], target.z - ee[2]};
      const double n = std::sqrt(e[0] * e[0] + e[1] * e[1] + e[2] * e[2]);
      if (n > max_step) {
        for (double& v : e) v *= max_step / n;  // batasi langkah supaya halus
      }

      // 3. Jacobian 3x4: ujung lengan bergeser ke mana jika tiap sendi diputar.
      mj_jacSite(m, d, jacp.data(), nullptr, site);
      double J[3][4];
      for (int r = 0; r < 3; ++r)
        for (int k = 0; k < 4; ++k) J[r][k] = jacp[r * m->nv + m->jnt_dofadr[jid[k]]];

      // 4. dq = J^T (J J^T + damping^2 I)^-1 e
      double A[3][3];
      for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j) {
          A[i][j] = (i == j) ? damping * damping : 0.0;
          for (int k = 0; k < 4; ++k) A[i][j] += J[i][k] * J[j][k];
        }
      double y[3];
      if (solve3(A, e, y)) {
        for (int k = 0; k < 4; ++k) {
          double dq = 0.0;
          for (int i = 0; i < 3; ++i) dq += J[i][k] * y[i];
          q[k] += dq;
          if (m->jnt_limited[jid[k]])  // jaga di dalam batas sendi
            q[k] = std::clamp(q[k], m->jnt_range[2 * jid[k]], m->jnt_range[2 * jid[k] + 1]);
        }
      }
    }

    // 5. Kirim sudut target ke simulasi (4 sendi lengan, gripper 0).
    std_msgs::Float64MultiArray cmd;
    cmd.data.assign(m->nu, 0.0);
    for (int k = 0; k < 4; ++k) cmd.data[k] = q[k];
    pub.publish(cmd);

    rate.sleep();
  }

  mj_deleteData(d);
  mj_deleteModel(m);
  return 0;
}
