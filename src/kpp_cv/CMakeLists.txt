cmake_minimum_required(VERSION 3.0.2)
project(kpp_cv)
find_package(catkin REQUIRED COMPONENTS rospy sensor_msgs cv_bridge)
catkin_package()
catkin_install_python(PROGRAMS
  scripts/publisher_node.py
  scripts/viewer_node.py
  scripts/shapes_node.py
  scripts/colorful_node.py
  scripts/denoise_node.py
  scripts/locate_node.py
  scripts/match_node.py
  scripts/denoise_node.py
  scripts/locate_node.py
  scripts/match_node.py
  scripts/colorful_node.py
  scripts/denoise_node.py
  scripts/locate_node.py
  scripts/match_node.py
  scripts/denoise_node.py
  scripts/locate_node.py
  scripts/match_node.py
  DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION}
)
