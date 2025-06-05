// 1.包含头文件
#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/compressed_image.hpp"
#include "opencv2/opencv.hpp"
#include "cv_bridge/cv_bridge.h"

int ms = 16;
int img_q = 50;
// 3.自定义节点类；
class Camera_Node : public rclcpp::Node
{
public:
  Camera_Node() : Node("camera_node_node_cpp")
  {
    // 实例化回调组 
    // 使用互斥回调组，确保每个摄像头的定时器回调不会同时执行,如果使用Reentrant回调组，则会导致多个摄像头的回调同时执行,造成图像卡顿
    pub_callback_group_1 = this->create_callback_group(rclcpp::CallbackGroupType::MutuallyExclusive);
    pub_callback_group_2 = this->create_callback_group(rclcpp::CallbackGroupType::MutuallyExclusive);
    pub_callback_group_3 = this->create_callback_group(rclcpp::CallbackGroupType::MutuallyExclusive);
    // 创建发布者
    camera1_pub_ = this->create_publisher<sensor_msgs::msg::CompressedImage>("camera_1", 10);
    camera2_pub_ = this->create_publisher<sensor_msgs::msg::CompressedImage>("camera_2", 10);
    camera3_pub_ = this->create_publisher<sensor_msgs::msg::CompressedImage>("camera_3", 10);
    // 打开摄像头设备
    // 强制使用V4L2后端打开摄像头
    camera1_cap_.open("/dev/usb_camera_1",cv::CAP_V4L2);
    if (!camera1_cap_.isOpened())
    {
      RCLCPP_ERROR(this->get_logger(), "Failed to open camera at /dev/usb_camera_1!");
      return;
    }
    camera2_cap_.open("/dev/usb_camera_2",cv::CAP_V4L2);
    if (!camera2_cap_.isOpened())
    {
      RCLCPP_ERROR(this->get_logger(), "Failed to open camera at /dev/usb_camera_2!");
      return;
    }
    camera3_cap_.open("/dev/usb_camera_3",cv::CAP_V4L2);
    if (!camera3_cap_.isOpened())
    {
      RCLCPP_ERROR(this->get_logger(), "Failed to open camera at /dev/usb_camera_3!");
      return;
    }
    // 设置定时器，每30ms发布一次图像
    camera1_timer_ = this->create_wall_timer(std::chrono::milliseconds(ms), std::bind(&Camera_Node::camera1_timer_callback, this), pub_callback_group_1);
    camera2_timer_ = this->create_wall_timer(std::chrono::milliseconds(ms), std::bind(&Camera_Node::camera2_timer_callback, this), pub_callback_group_2);
    camera3_timer_ = this->create_wall_timer(std::chrono::milliseconds(ms), std::bind(&Camera_Node::camera3_timer_callback, this), pub_callback_group_3);
  }

private:
  // 摄像头对象
  cv::VideoCapture camera1_cap_;
  cv::VideoCapture camera2_cap_;
  cv::VideoCapture camera3_cap_;
  // 成员变量
  rclcpp::CallbackGroup::SharedPtr pub_callback_group_1;
  rclcpp::CallbackGroup::SharedPtr pub_callback_group_2;
  rclcpp::CallbackGroup::SharedPtr pub_callback_group_3;

  // 定时器
  rclcpp::TimerBase::SharedPtr camera1_timer_;
  rclcpp::TimerBase::SharedPtr camera2_timer_;
  rclcpp::TimerBase::SharedPtr camera3_timer_;
  // 发布者
  rclcpp::Publisher<sensor_msgs::msg::CompressedImage>::SharedPtr camera1_pub_;
  rclcpp::Publisher<sensor_msgs::msg::CompressedImage>::SharedPtr camera2_pub_;
  rclcpp::Publisher<sensor_msgs::msg::CompressedImage>::SharedPtr camera3_pub_;

  //
  void camera1_timer_callback()
  {
    cv::Mat frame;
    if (camera1_cap_.read(frame))
    {
      // 创建消息头并设置时间戳
      auto header = std_msgs::msg::Header();
      header.stamp = this->now();
      header.frame_id = "camera_frame";
      // 压缩图像
      std::vector<uchar> compressed_image_buffer;
      cv::imencode(".jpg", frame, compressed_image_buffer, {cv::IMWRITE_JPEG_QUALITY, img_q});
      // 创建并发布CompressedImage消息
      sensor_msgs::msg::CompressedImage compressed_msg;
      compressed_msg.header = header;
      compressed_msg.format = "jpeg";
      compressed_msg.data = std::vector<uint8_t>(compressed_image_buffer.begin(), compressed_image_buffer.end());
      camera1_pub_->publish(compressed_msg);
    }
    else
    {
      RCLCPP_ERROR(this->get_logger(), "Failed to read frame from camera_1!");
    }
  }
  void camera2_timer_callback()
  {
    cv::Mat frame;
    if (camera2_cap_.read(frame))
    {
      // 创建消息头并设置时间戳
      auto header = std_msgs::msg::Header();
      header.stamp = this->now();
      header.frame_id = "camera_frame";
      // 压缩图像
      std::vector<uchar> compressed_image_buffer;
      cv::imencode(".jpg", frame, compressed_image_buffer, {cv::IMWRITE_JPEG_QUALITY, img_q});
      // 创建并发布CompressedImage消息
      sensor_msgs::msg::CompressedImage compressed_msg;
      compressed_msg.header = header;
      compressed_msg.format = "jpeg";
      compressed_msg.data = std::vector<uint8_t>(compressed_image_buffer.begin(), compressed_image_buffer.end());
      camera2_pub_->publish(compressed_msg);
    }
    else
    {
      RCLCPP_ERROR(this->get_logger(), "Failed to read frame from camera_2!");
    }
  }
  void camera3_timer_callback()
  {
    cv::Mat frame;
    if (camera3_cap_.read(frame))
    {
      // 创建消息头并设置时间戳
      auto header = std_msgs::msg::Header();
      header.stamp = this->now();
      header.frame_id = "camera_frame";
      // 压缩图像
      std::vector<uchar> compressed_image_buffer;
      cv::imencode(".jpg", frame, compressed_image_buffer, {cv::IMWRITE_JPEG_QUALITY, img_q});
      // 创建并发布CompressedImage消息
      sensor_msgs::msg::CompressedImage compressed_msg;
      compressed_msg.header = header;
      compressed_msg.format = "jpeg";
      compressed_msg.data = std::vector<uint8_t>(compressed_image_buffer.begin(), compressed_image_buffer.end());
      camera3_pub_->publish(compressed_msg);
    }
    else
    {
      RCLCPP_ERROR(this->get_logger(), "Failed to read frame from camera_3!");
    }
  }
};

int main(int argc, char const *argv[])
{
  // 2.初始化ROS2客户端；
  rclcpp::init(argc, argv);
  // 4.调用spain函数，并传入节点对象指针；
  auto node = std::make_shared<Camera_Node>();
  rclcpp::executors::MultiThreadedExecutor executor;
  executor.add_node(node);
  executor.spin();
  rclcpp::shutdown();
  return 0;
}