cmake_minimum_required(VERSION 3.8)
project(aimooe_ros2)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# --- 1. Find ROS2 Dependencies ---
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(tf2_ros REQUIRED)

# Find OpenCV
find_package(OpenCV REQUIRED)

# --- 2. Define the Executable Node ---
add_executable(aimooe_tracker_node src/aimooe_tracker_node.cpp)

# --- 3. Include Directories ---
target_include_directories(aimooe_tracker_node PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
  ${OpenCV_INCLUDE_DIRS}
)

# --- 4. Link the Proprietary Aimooe Library & OpenCV ---
target_link_libraries(aimooe_tracker_node 
  ${CMAKE_CURRENT_SOURCE_DIR}/lib/libAimPosition.so
  ${OpenCV_LIBS}
)

# --- 5. Link ROS2 Dependencies ---
ament_target_dependencies(aimooe_tracker_node
  rclcpp
  geometry_msgs
  tf2_ros
)

# --- 6. Install Rules ---
install(TARGETS aimooe_tracker_node
  DESTINATION lib/${PROJECT_NAME}
)

# Install the proprietary library so it is available at runtime
install(DIRECTORY lib/
  DESTINATION lib
  FILES_MATCHING PATTERN "*.so*"
)

install(DIRECTORY include/
  DESTINATION include
)

install(DIRECTORY launch/
  DESTINATION share/${PROJECT_NAME}/launch
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  set(ament_cmake_copyright_FOUND TRUE)
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()