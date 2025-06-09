#include "rclcpp/rclcpp.hpp"
#include "serial/serial.h"
#include "std_msgs/msg/float64.hpp"
#include <iostream>
#include <cmath>

serial::Serial ser;
using namespace std::chrono_literals;
// 3.自定义节点类；
class Data_Collect : public rclcpp::Node

{
public:
    Data_Collect() : Node("Data_Collect_node_cpp")
    {
        // 4.创建定时器，周期为10ms
        data_collect_timer_ = this->create_wall_timer(10ms, std::bind(&Data_Collect::data_collect_callback, this));
    }
    ~Data_Collect()
    {
        data_collect_timer_->cancel(); // 停止定时器
    }

private:
    // 定义定时器
    rclcpp::TimerBase::SharedPtr data_collect_timer_;
    // 定义回调函数
    void data_collect_callback()
    {
        // 定义一个Buffer接受订阅消息
        uint8_t Readdate[15] = {0};
        uint8_t Validdata[13];
        size_t bytes_read = ser.read(Readdate, 15);
        // 从串口读取数据
        ser.read(Readdate, 15);
        if (bytes_read == 15)
        {
            // 检查帧头和帧尾
            if (Readdate[0] == 0xFF && Readdate[14] == 0xFE)
            {
                // 提取有效数据
                for (int i = 0; i < 13; i++)
                {
                    Validdata[i] = Readdate[i + 1];
                }
                // 获取有效数据
                uint32_t data1 = ((uint32_t)Validdata[3]) |
                                 ((uint32_t)Validdata[2] << 8) |
                                 ((uint32_t)Validdata[1] << 16) |
                                 ((uint32_t)Validdata[0] << 24);
                uint32_t data2 = ((uint32_t)Validdata[7]) |
                                 ((uint32_t)Validdata[6] << 8) |
                                 ((uint32_t)Validdata[5] << 16) |
                                 ((uint32_t)Validdata[4] << 24);
                uint32_t data3 = (uint32_t)Validdata[8];
                uint32_t data4 = ((uint32_t)Validdata[10]) |
                                 ((uint32_t)Validdata[9] << 8);
                uint32_t data5 = ((uint32_t)Validdata[12]) |
                                 ((uint32_t)Validdata[11] << 8);
                // 转换为有符号整数
                int32_t signed_data1 = (int32_t)data1;
                int32_t signed_data2 = (int32_t)data2;
                // 打印数据
                std::cout << "data1: " << signed_data1 << std::endl;
                std::cout << "data2: " << signed_data2 << std::endl;
                std::cout << "data3: " << data3 << std::endl;
                std::cout << "data4: " << data4 << std::endl;
                std::cout << "data5: " << data5 << std::endl;
            }
            else
            {
                RCLCPP_ERROR(this->get_logger(), "包头包尾验证失败，无效数据");
            }
        }
        else
        {
            RCLCPP_ERROR(this->get_logger(), "数组长度错误");
        }
    }
};

int main(int argc, char const *argv[])
{
    rclcpp::init(argc, argv);
    ser.setPort("/dev/ttyS1");
    ser.setBaudrate(19200);
    serial::Timeout to = serial::Timeout::simpleTimeout(1000);
    ser.setTimeout(to);
    try
    {
        ser.open();
    }
    catch (serial::IOException &e)
    {
        std::cout << "unable to open" << std::endl;
        return -1;
    }
    if (ser.isOpen())
    {
        std::cout << "-----open------" << std::endl;
    }
    else
    {
        return -1;
    }
    // 4.调用spain函数，并传入节点对象指针；
    rclcpp::spin(std::make_shared<Data_Collect>());
    // 5.资源释放
    rclcpp::shutdown();
    // 关闭串口
    if (ser.isOpen())
    {
        ser.close();
    }
    return 0;
}