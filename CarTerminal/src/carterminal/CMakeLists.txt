cmake_minimum_required(VERSION 3.8)
project(carterminal)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(message_filters REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(serial REQUIRED)
find_package(image_transport REQUIRED)
find_package(cv_bridge REQUIRED)
find_package(OpenCV REQUIRED)



add_executable(camera_talker_node src/camera_talker_node.cpp)
add_executable(wheel_talker_node src/wheel_talker_node.cpp)

target_include_directories(camera_talker_node PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
target_compile_features(camera_talker_node PUBLIC c_std_99 cxx_std_17)  # Require C99 and C++17

ament_target_dependencies(
  camera_talker_node
  "rclcpp"
  "std_msgs"
  "sensor_msgs"
  "image_transport"
  "message_filters"
  "geometry_msgs"
  "serial"
  "OpenCV"
  "cv_bridge"
)

ament_target_dependencies(
  wheel_talker_node
  "rclcpp"
  "std_msgs"
  "sensor_msgs"
  "message_filters"
  "serial"
)


install(TARGETS 
  camera_talker_node
  wheel_talker_node
  DESTINATION lib/${PROJECT_NAME})

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  # the following line skips the linter which checks for copyrights
  # comment the line when a copyright and license is added to all source files
  set(ament_cmake_copyright_FOUND TRUE)
  # the following line skips cpplint (only works in a git repo)
  # comment the line when this package is in a git repo and when
  # a copyright and license is added to all source files
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
