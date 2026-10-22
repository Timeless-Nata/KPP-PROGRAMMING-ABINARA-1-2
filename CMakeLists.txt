cmake_minimum_required(VERSION 3.0.2)
project(kpp_ros_mujoco)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(catkin REQUIRED COMPONENTS
  roscpp
  std_msgs
  sensor_msgs
)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GLFW3 REQUIRED glfw3)

set(MUJOCO_DIR "$ENV{HOME}/mujoco")
find_library(MUJOCO_LIB mujoco PATHS ${MUJOCO_DIR}/build/lib NO_DEFAULT_PATH)
if(NOT MUJOCO_LIB)
  message(FATAL_ERROR "libmujoco.so tidak ditemukan di ${MUJOCO_DIR}/build/lib")
endif()

catkin_package()

include_directories(${catkin_INCLUDE_DIRS} ${MUJOCO_DIR}/include ${GLFW3_INCLUDE_DIRS})

add_executable(hello_node src/hello_node.cpp)
target_link_libraries(hello_node ${catkin_LIBRARIES})

add_executable(mujoco_sim src/mujoco_sim_node.cpp)
target_link_libraries(mujoco_sim ${catkin_LIBRARIES} ${MUJOCO_LIB} ${GLFW3_LIBRARIES})
