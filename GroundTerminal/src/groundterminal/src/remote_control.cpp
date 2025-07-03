// 1.包含头文件
#include "rclcpp/rclcpp.hpp"
#include "serial/serial.h"

serial::Serial ser;
using namespace std::chrono_literals;

uint8_t readdata[8] = {0x03, 0x04, 0x00, 0x00, 0x00, 0x0a, 0x71, 0xef};

// 3.自定义节点类；
class Remote_Control : public rclcpp::Node
{
public:
    Remote_Control() : Node("remote_control_node_cpp")
    {
        read_timer_ = this->create_wall_timer(100ms, std::bind(&Remote_Control::read_data, this));
    }

private:
    // 1.创建一个定时器，定时发送数据；
    rclcpp::TimerBase::SharedPtr read_timer_;
    void read_data()
    {
        // 发送数据
        ser.write(readdata, 8);
        // 创建接受数据空数组
        uint8_t controldate[25] = {0};
        ser.read(controldate, 25);
        // 打印接受到的数据
        std::cout << "Received data: "; 
        for (int i = 0; i < 25; ++i)
        {
            std::cout << std::hex << static_cast<int>(controldate[i]) << " ";
        }
        std::cout << std::dec << std::endl;
    }
};

int main(int argc, char const *argv[])
{
    // 2.初始化ROS2客户端；
    rclcpp::init(argc, argv);
    ser.setPort("/dev/ttyUSB0");
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
    rclcpp::spin(std::make_shared<Remote_Control>());
    // 5.资源释放
    rclcpp::shutdown();
    return 0;
}