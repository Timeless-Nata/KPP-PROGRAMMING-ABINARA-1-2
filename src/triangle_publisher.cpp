// Menerbitkan titik target end-effector yang menyusuri segitiga sama sisi.
// Segitiga ada di bidang tegak (x-z) dengan y tetap, berpusat di (center_x, center_y, center_z).

#include <geometry_msgs/PointStamped.h>
#include <ros/ros.h>

#include <cmath>

int main(int argc, char** argv) {
  ros::init(argc, argv, "triangle_publisher");
  ros::NodeHandle nh;
  ros::NodeHandle pnh("~");

  double cx, cy, cz;
  if (!(pnh.getParam("center_x", cx) && pnh.getParam("center_y", cy) &&
        pnh.getParam("center_z", cz))) {
    ROS_FATAL("Parameter ~center_x, ~center_y, ~center_z harus di-set");
    return 1;
  }
  double side = pnh.param("side", 0.06);      // panjang sisi (m)
  double period = pnh.param("period", 9.0);   // waktu satu putaran penuh (detik)
  double rate_hz = pnh.param("rate", 50.0);   // frekuensi publish (Hz)

  // Tiga titik sudut segitiga sama sisi di bidang x-z.
  const double R = side / std::sqrt(3.0);  // jarak pusat ke tiap sudut
  const double vx[3] = {cx, cx - side / 2.0, cx + side / 2.0};
  const double vz[3] = {cz + R, cz - R / 2.0, cz - R / 2.0};

  ros::Publisher pub = nh.advertise<geometry_msgs::PointStamped>("ee_target", 10);
  ros::Rate rate(rate_hz);
  const ros::Time start = ros::Time::now();

  while (ros::ok()) {
    // Posisi di sepanjang keliling: 0..3, bagian bulat = sisi ke-i, sisanya = seberapa jauh di sisi itu.
    const double s = std::fmod((ros::Time::now() - start).toSec() / period, 1.0) * 3.0;
    const int i = static_cast<int>(s);
    const double f = s - i;
    const int j = (i + 1) % 3;

    geometry_msgs::PointStamped p;
    p.header.stamp = ros::Time::now();
    p.header.frame_id = "world";
    p.point.x = vx[i] + f * (vx[j] - vx[i]);
    p.point.y = cy;
    p.point.z = vz[i] + f * (vz[j] - vz[i]);
    pub.publish(p);

    ros::spinOnce();
    rate.sleep();
  }
  return 0;
}
