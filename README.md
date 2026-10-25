# KPP Modul 3 Magang Abinara-1 2026

<img width="587" height="527" alt="image" src="https://github.com/user-attachments/assets/a3862aa9-8cf9-4811-a2db-7c43c2c9c190" />

Simple Robotic Arm 4DOF yang telah dibuat pada modul sebelumnya, kini end effectornya dapat membentuk segitiga secara berulang kali dengan algoritma Inverse Kinematic, terintegrasi dengan ROS 1

Seluruh sistem dijalankan dengan satu perintah:

```bash
roslaunch kpp_ros_mujoco sim.launch
```
## Struktur repository

```
kpp_ros_mujoco/
├── CMakeLists.txt
├── package.xml
├── launch/
│   └── sim.launch              # untuk menjalankan ketiga node yang ada
├── model/
│   └── main.xml                # XML berisi MJCF robot arm
├── src/
│   ├── mujoco_sim_node.cpp     # simulasi utama MuJoCo nya dan bikin window GLFW
│   ├── ik_controller.cpp       # inverse kinematics dan Jacobian diperhitungkan disini
│   └── triangle_publisher.cpp  # untuk memberitahu lintasan end effectornya
```

## Arsitektur

<img width="1952" height="152" alt="rosgraph" src="https://github.com/user-attachments/assets/7657d37d-1f84-4e35-a53d-c79722407442" />

Semua node berjalan di namespace `/robot1`.

### Node

| Node | Tugas |
|---|---|
| `triangle_publisher` | Generasi titik-titik lintasan yang harus dilewati end-effector. |
| `ik_controller` | Mengkalkulasi tiap rotasi tiap sendi agar end effector berhasil melewati titik lintasan. |
| `mujoco_sim` | Menjalankan fisika simulasi dan render MuJoCo. |

