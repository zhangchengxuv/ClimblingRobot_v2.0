cmake_minimum_required(VERSION 3.8)
project(groundterminal)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(serial REQUIRED)

add_executable(car_candriver src/car_candriver.cpp)
add_executable(remote_control src/remote_control.cpp)


target_include_directories(car_candriver PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
include_directories(${EIGEN3_INCLUDE_DIR})

target_compile_features(car_candriver PUBLIC c_std_99 cxx_std_17) 

ament_target_dependencies(
  car_candriver
  "rclcpp"
  "std_msgs"
  "sensor_msgs"
  "geometry_msgs"
)

ament_target_dependencies(
  remote_control
  "rclcpp"
  "std_msgs"
  "sensor_msgs"
  "geometry_msgs"
  "serial"
)


install(TARGETS 
        car_candriver
        remote_control
        DESTINATION lib/${PROJECT_NAME})
install(DIRECTORY launch DESTINATION share/${PROJECT_NAME})


if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  set(ament_cmake_copyright_FOUND TRUE)
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
