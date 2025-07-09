// 1.包含头文件
#include <iostream>
#include <sstream>
#include <iomanip>
#include "rclcpp/rclcpp.hpp"
#include "serial/serial.h"
#include "std_msgs/msg/int32.hpp"
#include "std_msgs/msg/float32.hpp"
#include "std_msgs/msg/float64_multi_array.hpp"

serial::Serial ros_ser;

using std::placeholders::_1;

int dec_voltage;
int dec_voltage_2;
int hex_voltage;
int hex_voltage_2;
// 3.自定义节点类；
class ROS2_STM32_Lis : public rclcpp::Node
{
public:
    ROS2_STM32_Lis() : Node("ros2_stm32_lis_node_cpp", rclcpp::NodeOptions().allow_undeclared_parameters(true))
    {
        sub_ = this->create_subscription<std_msgs::msg::Float64MultiArray>("voltage_talker", 10, std::bind(&ROS2_STM32_Lis::topic_callback, this, _1));
    }
    void topic_callback(const std_msgs::msg::Float64MultiArray msg)
    {
        // 定义一个Buffer接受订阅消息
        unsigned char buffer[6] = {0};
        // 将订阅的消息赋值给 dec_voltage（十进制）
        dec_voltage = msg.data[0];
        // 转换成十六进制数字
        std::ostringstream stream;
        stream << std::hex << dec_voltage;
        std::string hexStr = stream.str();
        std::cout << "辅助轮十六进制数: " << hexStr << std::endl;
        // 将十六进制字符串转换为无符号整数
        unsigned int num = std::stoul(hexStr, nullptr, 16);

        // 将订阅的消息赋值给 dec_voltage（十进制）
        dec_voltage_2 = msg.data[1];
        // 转换成十六进制数字
        std::ostringstream stream_2;
        stream_2 << std::hex << dec_voltage_2;
        std::string hexStr_2 = stream_2.str();
        std::cout << "机械臂十六进制数: " << hexStr_2 << std::endl;
        // 将十六进制字符串转换为无符号整数
        unsigned int num_2 = std::stoul(hexStr_2, nullptr, 16);

        // 帧头
        buffer[0] = 0xFF;
        // 将十六进制数的高8位和低8位分别存储到buffer[1]和buffer[2]
        buffer[1] = (num >> 8) & 0xFF; // 高8位
        buffer[2] = num & 0xFF;        // 低8位
        // 将十六进制数的高8位和低8位分别存储到buffer[3]和buffer[4]
        buffer[3] = (num_2 >> 24) & 0xFF;
        buffer[4] = num_2 & 0xFF;
        // 帧尾
        buffer[5] = 0xFE;

        for (int i = 0; i < 5; i++)
        {
            std::cout << std::hex << (buffer[i] & 0xff) << " ";
        }
        std::cout << std::endl;
        // 将数据通过串口发送给下位机
        ros_ser.write(buffer, 6);
    }

private:
    rclcpp::Subscription<std_msgs::msg::Float64MultiArray>::SharedPtr sub_;
};

int main(int argc, char const *argv[])
{
    // 2.初始化ROS2客户端；
    rclcpp::init(argc, argv);
    // 连接串口
    ros_ser.setPort("/dev/ttyS0");
    // ros_ser.setPort("/dev/ttyUSB0");

    // 串口波特率
    ros_ser.setBaudrate(19200);
    serial::Timeout to = serial::Timeout::simpleTimeout(1000);
    ros_ser.setTimeout(to);
    // ros_ser.close();

    try
    {
        ros_ser.open();
    }
    catch (serial::IOException &e)
    {
        std::cout << "unable to open" << std::endl;
        return -1;
    }
    if (ros_ser.isOpen())
    {
        std::cout << "-----open------" << std::endl;
    }
    else
    {
        return -1;
    }
    // 4.调用spain函数，并传入节点对象指针；
    rclcpp::spin(std::make_shared<ROS2_STM32_Lis>());
    // 关闭串口
    ros_ser.close();
    // 5.资源释放
    rclcpp::shutdown();

    return 0;
}
// 查看是否存在串口，或检查串口是否被占用。
// ls -l /dev/ttyUSB0
// 存在空闲的USB口，请检查/dev/ttyUSB0的权限，放开所有权限。
// sudo chmod 777 /dev/ttyUSB0