cmake_minimum_required(VERSION 3.8)
project(aimooe_core)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# --- Find Dependencies ---
find_package(ament_cmake REQUIRED)
find_package(OpenCV REQUIRED)

# --- Build wrapper into a shared library ---
add_library(${PROJECT_NAME} SHARED
  src/aimooe_tracker.cpp
)

# --- Include Directories ---
target_include_directories(${PROJECT_NAME} PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
  ${OpenCV_INCLUDE_DIRS}
)

# --- Link the Proprietary Aimooe Library & OpenCV ---
target_link_libraries(${PROJECT_NAME} PUBLIC 
  ${CMAKE_CURRENT_SOURCE_DIR}/lib/libAimPosition.so
  ${OpenCV_LIBS}
)

# Install the proprietary library so it is available at runtime
install(DIRECTORY lib/
  DESTINATION lib
  FILES_MATCHING PATTERN "*.so*"
)

install(DIRECTORY include/
  DESTINATION include
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  set(ament_cmake_copyright_FOUND TRUE)
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

# Export the include directories and libraries for the ROS 2 package to use
ament_export_include_directories(include)
ament_export_libraries(${PROJECT_NAME})

ament_package()