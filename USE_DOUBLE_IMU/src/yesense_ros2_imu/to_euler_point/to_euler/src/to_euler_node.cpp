#include <memory>
#include <inttypes.h>
#include <unistd.h>
#include <rclcpp/rclcpp.hpp>
#include "std_msgs/msg/float64.hpp"
#include <sensor_msgs/msg/imu.hpp>
#include <tf2_ros/transform_broadcaster.h>
// #include <tf2_geometry_msgs/tf2_geometry_msgs.hpp>
#include <tf2/LinearMath/Transform.h>
#include <tf2/LinearMath/Quaternion.h>
#include <string>
#include <geometry_msgs/msg/transform_stamped.hpp>
#include <rclcpp/time.hpp>
#include "std_msgs/msg/float64_multi_array.hpp"
using std::placeholders::_1;
using namespace std;

float imu_1_r, imu_1_p, imu_1_y;
float imu_2_r, imu_2_p, imu_2_y;

// 3.自定义节点类；
class To_euler_node : public rclcpp::Node
{
public:
  To_euler_node() : Node("to_euler_node_cpp")
  {
    // 各自回调组
    sub_callback_group_1 = create_callback_group(rclcpp::CallbackGroupType::MutuallyExclusive);
    rclcpp::SubscriptionOptions options_sub_1;
    options_sub_1.callback_group = sub_callback_group_1;
    // 回调组
    sub_callback_group_2 = create_callback_group(rclcpp::CallbackGroupType::MutuallyExclusive);
    rclcpp::SubscriptionOptions options_sub_2;
    options_sub_2.callback_group = sub_callback_group_2;
    //
    sendtimer_callback_group_ = create_callback_group(rclcpp::CallbackGroupType::MutuallyExclusive);
    // 订阅IMU数据
    sub_imu1_ = this->create_subscription<sensor_msgs::msg::Imu>(
        "/imu1/data_raw", 10, std::bind(&To_euler_node::imu1_callback, this, _1), options_sub_1);
    sub_imu2_ = this->create_subscription<sensor_msgs::msg::Imu>(
        "imu2/data_raw", 10, std::bind(&To_euler_node::imu2_callback, this, _1), options_sub_2);
    // 发布欧拉角数据
    pub_euler1_ = this->create_publisher<std_msgs::msg::Float64MultiArray>(
        "/imu1euler", 10);
    pub_euler2_ = this->create_publisher<std_msgs::msg::Float64MultiArray>(
        "/imu2euler", 10);
    // 发布差值
    diff_r_pub  = this->create_publisher<std_msgs::msg::Float64>(
        "/imu_diff_r", 10);

    // 定时器发布欧拉角差值
    send_timer_ = this->create_wall_timer(10ms, std::bind(&To_euler_node::senddata_cb, this), sendtimer_callback_group_);
  }

private:
  rclcpp::CallbackGroup::SharedPtr sub_callback_group_1;
  rclcpp::CallbackGroup::SharedPtr sub_callback_group_2;
  rclcpp::CallbackGroup::SharedPtr sendtimer_callback_group_;
  rclcpp::Subscription<sensor_msgs::msg::Imu>::SharedPtr sub_imu1_;
  rclcpp::Subscription<sensor_msgs::msg::Imu>::SharedPtr sub_imu2_;
  rclcpp::Publisher<std_msgs::msg::Float64MultiArray>::SharedPtr pub_euler1_;
  rclcpp::Publisher<std_msgs::msg::Float64MultiArray>::SharedPtr pub_euler2_;
  rclcpp::Publisher<std_msgs::msg::Float64>::SharedPtr diff_r_pub;
  rclcpp::TimerBase::SharedPtr send_timer_;

  void imu1_callback(const sensor_msgs::msg::Imu::SharedPtr msg)
  {
    // 处理IMU1数据
    std_msgs::msg::Float64MultiArray euler_msg;
    euler_msg.data.resize(3);
    tf2::Quaternion q(msg->orientation.x, msg->orientation.y, msg->orientation.z, msg->orientation.w);
    tf2::Matrix3x3(q).getEulerYPR(euler_msg.data[0], euler_msg.data[1], euler_msg.data[2]);
    imu_1_r = euler_msg.data[2] * 180.0 / M_PI; // 转换为度
    imu_1_p = euler_msg.data[1] * 180.0 / M_PI;
    imu_1_y = euler_msg.data[0] * 180.0 / M_PI;
    // RCLCPP_INFO(this->get_logger(), "IMU1 Euler Angles: Roll: %.2f, Pitch: %.2f, Yaw: %.2f",
    //             imu_1_r, imu_1_p, imu_1_y);
    // 发布IMU1
    pub_euler1_->publish(euler_msg);
  }
  void imu2_callback(const sensor_msgs::msg::Imu::SharedPtr msg)
  {
    // 处理IMU2数据
    std_msgs::msg::Float64MultiArray euler_msg;
    euler_msg.data.resize(3);
    tf2::Quaternion q(msg->orientation.x, msg->orientation.y, msg->orientation.z, msg->orientation.w);
    tf2::Matrix3x3(q).getEulerYPR(euler_msg.data[0], euler_msg.data[1], euler_msg.data[2]);
    imu_2_r = euler_msg.data[2] * 180.0 / M_PI;
    imu_2_p = euler_msg.data[1] * 180.0 / M_PI;
    imu_2_y = euler_msg.data[0] * 180.0 / M_PI;
    // RCLCPP_INFO(this->get_logger(), "IMU2 Euler Angles: Roll: %.2f, Pitch: %.2f, Yaw: %.2f",
    //             imu_2_r, imu_2_p, imu_2_y);
    // 发布IMU2
    pub_euler2_->publish(euler_msg);
  }
  void senddata_cb()
  {
    auto euler_diff_msg = std_msgs::msg::Float64();
    // 发布欧拉角差值 Float 绕x轴
    float diff_r = imu_1_r - imu_2_r;
    euler_diff_msg.data = diff_r;
    diff_r_pub->publish(euler_diff_msg);
  }
};

int main(int argc, char const *argv[])
{
  // 2.初始化ROS2客户端；
  rclcpp::init(argc, argv);
  // 4.调用spain函数，并传入节点对象指针；
  rclcpp::spin(std::make_shared<To_euler_node>());
  // 5.资源释放
  rclcpp::shutdown();
  return 0;
}