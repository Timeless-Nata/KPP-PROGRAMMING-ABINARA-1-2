// Menerbitkan titik target end-effector yang menyusuri segitiga sama sisi
// di bidang tegak (x = depan, z = atas), dengan y tetap.

#include <geometry_msgs/PointStamped.h>
#include <ros/ros.h>

#include <cmath>

int main(int argc, char** argv) {
  ros::init(argc, argv, "triangle_publisher");
  ros::NodeHandle nh;
  ros::NodeHandle pnh("~");

  // Pusat dan ukuran segitiga (meter). Nilai ini bawaan, bisa diubah lewat parameter.
  const double cx = pnh.param("center_x", 0.22);
  const double cy = pnh.param("center_y", -0.0011);
  const double cz = pnh.param("center_z", 0.155);
  const double side = pnh.param("side", 0.06);      // panjang sisi
  const double period = pnh.param("period", 9.0);   // detik per satu putaran

  // Tiga titik sudut: A di atas, B kiri-bawah, C kanan-bawah.
  const double R = side / std::sqrt(3.0);  // jarak pusat ke tiap sudut
  const double vx[3] = {cx, cx - side / 2.0, cx + side / 2.0};
  const double vz[3] = {cz + R, cz - R / 2.0, cz - R / 2.0};

  ros::Publisher pub = nh.advertise<geometry_msgs::PointStamped>("ee_target", 10);
  ros::Rate rate(50);  // 50 kali per detik
  const ros::Time start = ros::Time::now();

  while (ros::ok()) {
    // s berjalan 0 -> 3 selama satu putaran. Bagian bulat = sisi mana, desimal = sudah sejauh apa.
    const double s = std::fmod((ros::Time::now() - start).toSec() / period, 1.0) * 3.0;
    const int i = static_cast<int>(s);   // titik awal sisi ini
    const int j = (i + 1) % 3;           // titik akhir sisi ini
    const double f = s - i;              // 0..1 di sepanjang sisi

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
