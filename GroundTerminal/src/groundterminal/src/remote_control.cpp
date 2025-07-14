// 1.包含头文件
#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/int32.hpp"
#include "std_msgs/msg/float32.hpp"
#include "std_msgs/msg/float64_multi_array.hpp"
#include "serial/serial.h"
#include "mode_switching.h"
#include <cmath>
#include <thread>
#include <atomic>

serial::Serial ser;
using namespace std::chrono_literals;

uint8_t readdata[8] = {0x03, 0x04, 0x00, 0x00, 0x00, 0x0a, 0x71, 0xef};

// 3.自定义节点类；
class Remote_Control : public rclcpp::Node
{
public:
    Remote_Control() : Node("remote_control_node_cpp")
    {
        read_timer_ = this->create_wall_timer(10ms, std::bind(&Remote_Control::read_data, this));
        // 方向
        direction_pub_ = this->create_publisher<std_msgs::msg::Int32>("direction", 10);
        // 发送两种电压值（数组）
        voltageArray_pub_ = this->create_publisher<std_msgs::msg::Float64MultiArray>("voltage_array", 10);
        // 发送两种电压值（数组）
        voltageArray_pub_2 = this->create_publisher<std_msgs::msg::Float64MultiArray>("voltage_array_2", 10);
        // wheel状态
        wheel_status_pub_ = this->create_publisher<std_msgs::msg::Int32>("wheel_status", 10);
        // arm状态
        arm_status_pub_ = this->create_publisher<std_msgs::msg::Int32>("arm_status", 10);
        // 遥杆或者巡航车体状态
        joystick_or_cruising_pub_ = this->create_publisher<std_msgs::msg::Int32>("joystick_or_cruising", 10);
        // 遥杆车体速度
        speed_joystick_pub_ = this->create_publisher<std_msgs::msg::Int32>("speed_joystick", 10);
        // 巡航车体速度
        cruising_speed_pub = this->create_publisher<std_msgs::msg::Int32>("cruising_speed", 10);
        // 转向遥杆速度
        trunspeed_joystick_pub_ = this->create_publisher<std_msgs::msg::Int32>("trunspeed_joystick", 10);
    }

private:
    // 定时发送请求数据
    rclcpp::TimerBase::SharedPtr read_timer_;
    // 运动方向(Int32)
    rclcpp::Publisher<std_msgs::msg::Int32>::SharedPtr direction_pub_;
    // 遥杆或者巡航车体状态(Int32)
    rclcpp::Publisher<std_msgs::msg::Int32>::SharedPtr joystick_or_cruising_pub_;
    // 巡航车体速度(Int32)
    rclcpp::Publisher<std_msgs::msg::Int32>::SharedPtr cruising_speed_pub;
    // 遥杆车体速度(Int32)
    rclcpp::Publisher<std_msgs::msg::Int32>::SharedPtr speed_joystick_pub_;
    // 转向遥杆速度(Int32)
    rclcpp::Publisher<std_msgs::msg::Int32>::SharedPtr trunspeed_joystick_pub_;
    // wheel状态(Int32)
    rclcpp::Publisher<std_msgs::msg::Int32>::SharedPtr wheel_status_pub_;
    // arm状态(Int32)
    rclcpp::Publisher<std_msgs::msg::Int32>::SharedPtr arm_status_pub_;
    // 发送两种电压值（数组）
    rclcpp::Publisher<std_msgs::msg::Float64MultiArray>::SharedPtr voltageArray_pub_;
    // 发送两种电压值（数组）
    rclcpp::Publisher<std_msgs::msg::Float64MultiArray>::SharedPtr voltageArray_pub_2;
    // 模式切换线程

    void read_data()
    {
        //-----------------------------发送的变量----------------------------//
        // 方向变量 前进：1 |后退：2 |左转：3 |右转：4
        auto direction = std_msgs::msg::Int32();
        // 遥杆车体速度
        auto speed_joystick = std_msgs::msg::Int32();
        // 遥杆转向速度
        auto speed_turn = std_msgs::msg::Int32();
        // 巡航车体速度
        auto cruising_speed = std_msgs::msg::Int32();
        // 遥杆或者巡航车体状态
        auto joystick_or_cruising = std_msgs::msg::Int32();
        // wheel状态
        auto wheel_status = std_msgs::msg::Int32();
        // arm状态
        auto arm_status = std_msgs::msg::Int32();
        // // 辅助轮推杆变量
        // auto auxiliary_rod = std_msgs::msg::Float32();
        // // 前轮推杆变量
        // auto front_rod = std_msgs::msg::Float32();
        // 综合推杆电压变量
        auto combined_voltage = std_msgs::msg::Float64MultiArray();
        combined_voltage.data.resize(2);
        auto combined_voltage_2 = std_msgs::msg::Float64MultiArray();
        combined_voltage_2.data.resize(2);

        //-----------------------------数据的解析----------------------------//
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
        // 巡航车体速度
        if (mode_speed_status == 0)
        {
            uint8_t speed_wheel = controldate[8];
            float dec_speed_wheel = static_cast<int>(speed_wheel) / 255.0 * 10; // 速度范围是0-10 m/min (2335)
            std::cout << "巡航车体速度: " << dec_speed_wheel << " m/min" << std::endl;
            float motorspeed = dec_speed_wheel * 3.0 * 110 * 100 / 3.14 / 0.125 / 360; // 转换为电机速度
            cruising_speed.data = motorspeed;
        }
        else if (mode_speed_status == 1)
        {
            float motorspeed = mode_speed_value * 3.0 * 110 * 100 / 3.14 / 0.125 / 360; // 转换为电机速度
            cruising_speed.data = motorspeed;
        }
        // 遥杆车体速度
        uint8_t speed_wheel_1 = controldate[4];
        float dec_speed_wheel_1 = static_cast<int>(speed_wheel_1) / 255.0 * 10; // 速度范围是0-10 m/min
        std::cout << "遥杆车体速度: " << dec_speed_wheel_1 << " m/min" << std::endl;
        float motorspeed1 = dec_speed_wheel_1 * 3.0 * 110 * 100 / 3.14 / 0.125 / 360; // 转换为电机速度
        speed_joystick.data = motorspeed1;
        // 遥杆转向速度
        uint8_t speed_turu_1 = controldate[6];
        float dec_speed_turn = static_cast<int>(speed_turu_1) / 255.0 * 10; // 速度范围是0-10m/min
        std::cout << "遥杆转向速度: " << dec_speed_turn << " m/min" << std::endl;
        float motorspeed2 = dec_speed_turn * 3.0 * 110 * 100 / 3.14 / 0.125 / 360; // 转换为电机速度
        speed_turn.data = motorspeed2;
        // 摆臂电机速度
        uint8_t speed_arm = controldate[14];
        float dec_speed_arm = static_cast<int>(speed_arm) / 255.0 * 5; // 速度范围是0-10 m/min
        std::cout << "摆臂电机速度: " << dec_speed_arm << " m/min" << std::endl;
        // 辅助推杆电压
        uint8_t voltage_0 = controldate[10];
        float dec_voltage_0 = static_cast<int>(voltage_0) / 255.0 * 24.0; // 电压范围是0-24V
        combined_voltage_2.data[0] = dec_voltage_0;                       // 转换为推杆电压
        std::cout << "辅助推杆电压: " << dec_voltage_0 << " V" << std::endl;

        // 前臂推杆电压
        uint8_t voltage_1 = controldate[12];
        float dec_voltage_1 = static_cast<int>(voltage_1) / 255.0 * 24.0; // 电压范围是0-24V
        combined_voltage_2.data[1] = dec_voltage_1;                       // 转换为推杆电压
        std::cout << "前臂推杆电压: " << dec_voltage_1 << " V" << std::endl;
        // controldate[17] 二进制，|按钮1|喷漆关|喷漆开|F4|F3|F2|F1|除锈|
        uint8_t status_0 = controldate[17];
        if ((status_0 & 0x80) != 0)
        {
            std::cout << "按钮1: 按下" << std::endl;
        }
        else
        {
            std::cout << "按钮1: 未按下" << std::endl;
        }
        if ((status_0 & 0x40) != 0)
        {
            std::cout << "喷漆: 关闭" << std::endl;
        }
        else
        {
            std::cout << "喷漆: 开启" << std::endl;
        }
        // 日常模式与特殊模式切换
        if ((status_0 & 0x10) != 0)
        {
            // 速度状态
            mode_speed_status = 0;
            // 模式辅助轮状态
            mode_wheel_status = 0; // 0:日常 1:模式
            std::cout << "遥杆模式，推杆电压手动" << std::endl;
        }
        else
        {
            // 速度状态
            mode_speed_status = 1;
            // 模式辅助轮状态
            mode_wheel_status = 1; // 0:日常 1:模式
            std::cout << "特殊过渡模式" << std::endl;
        }
        // 遥杆速度与巡航速度切换
        if ((status_0 & 0x04) != 0)
        {
            // F2 按键
            std::cout << "遥杆模式" << std::endl;
            joystick_or_cruising.data = 0; // 遥杆模式
        }
        else
        {
            std::cout << "巡航模式" << std::endl;
            joystick_or_cruising.data = 1; // 巡航模式
        }

        // if ((status_0 & 0x02) != 0)
        // {
        //     std::cout << "F1: 按下" << std::endl;
        // }
        // else
        // {
        //     std::cout << "F1: 未按下" << std::endl;
        // }

        if ((status_0 & 0x01) != 0)
        {
            std::cout << "除锈: 开启" << std::endl;
        }
        else
        {
            std::cout << "除锈: 关闭" << std::endl;
        }
        // controldate[18] 二进制，|车头方向|水平校准|左转|右转|后退|前进|无线状态|急停|
        uint8_t status_1 = controldate[18];
        if ((status_1 & 0x80) != 0)
        {
            std::cout << "车头方向: 前进" << std::endl;
        }
        else
        {
            std::cout << "车头方向: 后退" << std::endl;
        }
        if ((status_1 & 0x40) != 0)
        {
            std::cout << "水平校准: 开启" << std::endl;
        }
        else
        {
            std::cout << "水平校准: 关闭" << std::endl;
        }
        if ((status_1 & 0x20) != 0)
        {
            std::cout << "左转" << std::endl;
            direction.data = 3; // 左转
        }
        else if ((status_1 & 0x10) != 0)
        {
            std::cout << "右转" << std::endl;
            direction.data = 4; // 右转
        }
        else if ((status_1 & 0x08) != 0)
        {
            std::cout << "后退" << std::endl;
            direction.data = 2; // 后退
        }
        else if ((status_1 & 0x04) != 0)
        {
            std::cout << "前进" << std::endl;
            direction.data = 1; // 前进
        }
        else
        {
            std::cout << "停止：无方向控制" << std::endl;
        }
        if ((status_1 & 0x02) != 0)
        {
            std::cout << "无线状态: 开启" << std::endl;
        }
        else
        {
            std::cout << "无线状态: 关闭" << std::endl;
        }
        if ((status_1 & 0x01) != 0)
        {
            std::cout << "急停: 开启" << std::endl;
        }
        else
        {
            std::cout << "急停: 关闭" << std::endl;
        }
        // controldate[19] 二进制，辅助轮下降|辅助轮上升|摆臂下降|各自停止
        uint8_t status_2 = controldate[19];
        // 日常
        if (mode_wheel_status == 0)
        {
            if ((status_2 & 0x04) != 0)
            {
                std::cout << "辅助轮下降" << std::endl;
                // auxiliary_rod.data = dec_voltage_0 * 10.0;
                combined_voltage.data[0] = dec_voltage_0 * 10.0;
                wheel_status.data = 1; // 辅助轮下降状态
            }
            else if ((status_2 & 0x02) != 0)
            {
                std::cout << "辅助轮上升" << std::endl;
                // auxiliary_rod.data = dec_voltage_0 * 10.0 + 240.0;
                float temp = dec_voltage_0 * 10.0 + 240.0;
                if (temp == 240)
                {
                    combined_voltage.data[0] = 0;
                }
                else
                {
                    combined_voltage.data[0] = dec_voltage_0 * 10.0 + 240.0;
                }
                wheel_status.data = 2; // 辅助轮上升状态
            }
            else
            {
                std::cout << "辅助轮停止" << std::endl;
            }
        }
        // 模式
        else if (mode_wheel_status == 1)
        {
            if (mode_wheel_direction == 0)
            {
                float temp = mode_voltage_value * 10.0 + 240.0;
                if (temp == 240)
                {
                    combined_voltage.data[0] = 0;
                }
                else
                {
                    combined_voltage.data[0] = mode_voltage_value * 10.0 + 240.0;
                }
                wheel_status.data = 2; // 辅助轮上升状态
            }
            else if (mode_wheel_direction == 1)
            {
                combined_voltage.data[0] = mode_voltage_value * 10.0;
                wheel_status.data = 1; // 辅助轮下降状态
            }
        }

        // controldate[20] 二进制，辅助轮下降|辅助轮上升|摆臂下降|各自停止
        uint8_t status_3 = controldate[20];
        if ((status_3 & 0x80) != 0)
        {
            std::cout << "摆臂上升" << std::endl;
            // front_rod.data = dec_voltage_1 * 10.0;
            combined_voltage.data[1] = dec_voltage_1 * 10.0;
            arm_status.data = 1; // 摆臂上升状态
        }
        else if ((status_2 & 0x01) != 0)
        {
            std::cout << "摆臂下降" << std::endl;
            // front_rod.data = dec_voltage_1 * 10.0 + 240.0;
            combined_voltage.data[1] = dec_voltage_1 * 10.0 + 240.0;
            arm_status.data = 2; // 摆臂下降状态
        }
        else
        {
            std::cout << "摆臂停止" << std::endl;
        }
        if ((status_3 & 0x40) != 0)
        {
            std::cout << "阳角过渡: 关闭" << std::endl;
        }
        else
        {
            std::cout << "阳角过渡: 开启" << std::endl;
        }
        if ((status_3 & 0x10) != 0)
        {
            std::cout << "阴角过渡: 关闭" << std::endl;
        }
        else
        {
            std::cout << "阴角过渡: 开启" << std::endl;
        }
        if ((status_3 & 0x04) != 0)
        {
            std::cout << "按钮4：按下" << std::endl;
        }
        else
        {
            std::cout << "按钮4：未按下" << std::endl;
        }
        if ((status_3 & 0x02) != 0)
        {
            std::cout << "按钮3：按下" << std::endl;
        }
        else
        {
            std::cout << "按钮3：未按下" << std::endl;
        }
        if ((status_3 & 0x01) != 0)
        {
            std::cout << "按钮2：按下" << std::endl;
        }
        else
        {
            std::cout << "按钮2：未按下" << std::endl;
        }
        // 12位开关设置
        uint8_t status_4 = controldate[22];
        // if ((status_4 & 0x40) != 0)
        // {
        //     std::cout << "遥控关机" << std::endl;
        // }
        // else
        // {
        //     std::cout << "遥控开机" << std::endl;
        // }
        if ((status_4 & 0x20) != 0)
        {
            std::cout << "旋转开关在11" << std::endl;
        }
        else if ((status_4 & 0x10) != 0)
        {
            std::cout << "旋转开关在10" << std::endl;
        }
        else if ((status_4 & 0x08) != 0)
        {
            std::cout << "旋转开关在9" << std::endl;
        }
        else if ((status_4 & 0x04) != 0)
        {
            std::cout << "旋转开关在8" << std::endl;
        }
        else if ((status_4 & 0x02) != 0)
        {
            std::cout << "旋转开关在7" << std::endl;
        }
        else if ((status_4 & 0x01) != 0)
        {
            std::cout << "旋转开关在6" << std::endl;
        }
        else if ((status_2 & 0x80) != 0)
        {
            std::cout << "旋转开关在5" << std::endl;
        }
        else if ((status_2 & 0x40) != 0)
        {
            std::cout << "旋转开关在4" << std::endl;
        }
        else if ((status_2 & 0x20) != 0)
        {
            std::cout << "旋转开关在3" << std::endl;
        }
        else if ((status_2 & 0x10) != 0)
        {
            std::cout << "旋转开关在2" << std::endl;
        }
        else if ((status_2 & 0x08) != 0)
        {
            std::cout << "旋转开关在1" << std::endl;
        }
        else
        {
            std::cout << "旋转开关在0" << std::endl;
        }
        //-----------------------------数据的发布----------------------------//

        // 方向发布
        direction_pub_->publish(direction);
        // 遥杆或者巡航车体状态发布
        joystick_or_cruising_pub_->publish(joystick_or_cruising);
        // 巡航车体速度发布
        cruising_speed_pub->publish(cruising_speed);
        // 遥杆车体速度发布
        speed_joystick_pub_->publish(speed_joystick);
        // 综合推杆电压发布
        voltageArray_pub_->publish(combined_voltage);
        voltageArray_pub_2->publish(combined_voltage_2);
        // wheel状态发布
        wheel_status_pub_->publish(wheel_status);
        // arm状态发布
        arm_status_pub_->publish(arm_status);
        // 转向遥杆速度发布
        trunspeed_joystick_pub_->publish(speed_turn);
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