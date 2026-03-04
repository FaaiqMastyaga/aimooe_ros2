cmake_minimum_required(VERSION 3.8)
project(aimooe_ros2)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(aimooe_core REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(tf2 REQUIRED)
find_package(tf2_ros REQUIRED)
find_package(std_srvs REQUIRED)
find_package(aimooe_msgs REQUIRED)
find_package(ament_index_cpp REQUIRED)

add_executable(aimooe_tracker_node 
  src/aimooe_tracker_node.cpp
)

target_include_directories(aimooe_tracker_node PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

ament_target_dependencies(aimooe_tracker_node
  rclcpp
  aimooe_core
  geometry_msgs
  tf2
  tf2_ros
  std_srvs
  aimooe_msgs
  ament_index_cpp
)

# Install the executable
install(TARGETS aimooe_tracker_node
  DESTINATION lib/${PROJECT_NAME}
)

# Install the tools and meshes directories
install(DIRECTORY tools meshes
  DESTINATION share/${PROJECT_NAME}
)

ament_package()