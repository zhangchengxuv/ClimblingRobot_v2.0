// 1.包含头文件

#include <mutex>
#include <cmath>
#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/int32.hpp"
#include "std_msgs/msg/float64_multi_array.hpp"
#include "groundterminal/mode_switching.h"

// 模式1参数
float mode_1_speed = 0.2;
float mode_1_distance = 0.2;
float mode_1_1_minroll;
float mode_1_1_maxroll;
float mode_1_1_voltage = 0.0;
float mode_1_2_minroll;
float mode_1_2_maxroll;
float mode_1_2_voltage = 0.0;

// 模式2参数
float mode_2_speed;       // 模式2的速度
float mode_2_1_startroll; // 模式2开始的roll角度
float mode_2_kp;          // 模式2的比例系数
float mode_2_expect;      // 模式2的推杆期望下压力
float mode_2_max_voltage; // 模式2的最大电压
float mode_2_ads;         // 期望电压与实际电压的绝对值
float mode_2_1_voltage;   // 模式2 阶段一 的电压
int state = 0;            // 状态机阶段
//
//

using std::placeholders::_1;
int state = 0;
// 3.自定义节点类；
class Mode_Switching : public rclcpp::Node
{
public:
    Mode_Switching() : Node("mode_switching_node_cpp")
    {
        //
        sub_callback_group_1 = create_callback_group(rclcpp::CallbackGroupType::MutuallyExclusive);
        rclcpp::SubscriptionOptions options_sub_1;
        options_sub_1.callback_group = sub_callback_group_1;
        //
        sub_callback_group_2 = create_callback_group(rclcpp::CallbackGroupType::MutuallyExclusive);
        rclcpp::SubscriptionOptions options_sub_2;
        options_sub_2.callback_group = sub_callback_group_2;
        // 订阅IMU和激光数据
        imu_and_laser_sub_ = this->create_subscription<std_msgs::msg::Float64MultiArray>(
            "imu_and_laser", 10, std::bind(&Mode_Switching::imu_and_laser_callback, this, _1), options_sub_1);
        mode_status_sub_ = this->create_subscription<std_msgs::msg::Int32>(
            "mode_status", 1, std::bind(&Mode_Switching::mode_status_callback, this, _1), options_sub_2);
        value_pub_ = this->create_publisher<std_msgs::msg::Float64MultiArray>("/mode_value", 10);
    }

private:
    rclcpp::Publisher<std_msgs::msg::Float64MultiArray>::SharedPtr value_pub_;

    rclcpp::Subscription<std_msgs::msg::Float64MultiArray>::SharedPtr imu_and_laser_sub_;
    rclcpp::Subscription<std_msgs::msg::Int32>::SharedPtr mode_status_sub_;

    rclcpp::CallbackGroup::SharedPtr sub_callback_group_1;
    rclcpp::CallbackGroup::SharedPtr sub_callback_group_2;

    void mode_status_callback(const std_msgs::msg::Int32::SharedPtr msg)
    {
        state = msg->data;
    }
    void imu_and_laser_callback(const std_msgs::msg::Float64MultiArray::SharedPtr msg)
    {
        // 处理IMU和激光数据
        float pitch = msg->data[0];
        float roll = msg->data[1];
        float distance = msg->data[2];
        float force = msg->data[3];
        if (state == 0)
        {
            mode_01(pitch, roll, distance, force);
        }
        else
        {
            std::cout << "模式切换未启用" << std::endl;
        }
    }
    void mode_01(float pitch, float roll, float distance, float force)
    {
        auto combined_value = std_msgs::msg::Float64MultiArray();
        combined_value.data.resize(3);
        // 根据激光传感器判断车头是否探出
        if (distance > mode_1_distance)
        {
            // 第一阶段 推杆慢速
            if (roll > mode_1_1_minroll && roll < mode_1_1_maxroll)
            {
                // 定速值
                combined_value.data[0] = mode_1_speed;
                // 电压值
                combined_value.data[1] = mode_1_1_voltage;
                // 方向 0：升 1：降
                combined_value.data[2] = 1;
            }
            // 第二阶段 推杆快速
            else if (roll > mode_1_2_minroll && roll < mode_1_2_maxroll)
            {
                // 定速值
                combined_value.data[0] = mode_1_speed;
                // 电压值
                combined_value.data[1] = mode_1_2_voltage;
                // 方向 0：升 1：降
                combined_value.data[2] = 1;
            }
            else
            {
                combined_value.data[0] = mode_1_speed;
                combined_value.data[1] = 0.0;
                combined_value.data[2] = 2;
            }
        }
        // 发送数据
        value_pub_->publish(combined_value);
        // std::cout << "pitch :" << pitch << "deg roll: " << roll << "deg distance: " << distance << "m force: " << force << std::endl;
    }
    void mode_02(float pitch, float roll, float distance, float force)
    {
        auto combined_value = std_msgs::msg::Float64MultiArray();
        combined_value.data.resize(3);

        // float mode_temp_voltage = std::abs(mode_2_kp * (mode_2_pressure - force));
        // // 限制电压范围
        // // abs !
        // float mode_voltage = std::min(mode_temp_voltage, mode_2_max_voltage);
        // 定速值
        combined_value.data[0] = mode_2_speed;
        switch (state)
        {
        case 0:
        {
            // 缓慢下放，直到测量值force小于 0
            if (force > 0)
            {
                // 电压值
                combined_value.data[1] = mode_2_1_voltage; // 阶段一的电压
                combined_value.data[2] = 1;                // 方向 0：升 1：降
            }
            else
            {
                state = 1; // 切换到 阶段 2
            }
            break;
        }
        case 1:
        {
            // 俯角大于某个值后开启
            if (roll > mode_2_1_startroll)
            {
                // 实际力小于期望力 且 力差值大于阈值 需要上拉
                if (force < mode_2_expect && std::abs(mode_2_expect - force) > mode_2_ads)
                {
                    // 比例 * 力差值
                    float mode_temp_voltage = mode_2_kp * std::abs(mode_2_expect - force);
                    // 限制最大电压
                    float mode_voltage = std::min(mode_temp_voltage, mode_2_max_voltage);
                    //
                    combined_value.data[1] = mode_voltage; // 阶段2 电压
                    combined_value.data[2] = 0;            // 方向 0：升|1：降
                }
                // 实际力大于期望力 且 力差值大于阈值 需要下压
                else if (force > mode_2_expect && std::abs(mode_2_expect - force) > mode_2_ads)
                {
                    // 比例 * 力差值
                    float mode_temp_voltage = mode_2_kp * std::abs(mode_2_expect - force);
                    // 限制最大电压
                    float mode_voltage = std::min(mode_temp_voltage, mode_2_max_voltage);
                    //
                    combined_value.data[1] = mode_voltage; // 阶段2 电压
                    combined_value.data[2] = 1;            // 方向 0：升|1：降
                }
                // 实际力接近期望力，且力差值小于阈值，停止推杆动作，继续往前走
                else
                {
                    // 力差值小于阈值，保持原有状态
                    combined_value.data[1] = 0.0; // 停止
                    combined_value.data[2] = 2;   // 停止
                }
            }
            // 关闭条件，俯角达到88度以上
            else if (roll > 88.0)
            {
                combined_value.data[1] = 0.0; // 停止
                combined_value.data[2] = 2;   // 停止
            }
        }

        default:
            break;
        }

        // // 开启条件：俯角达到一定值，且期望值与实际值的绝对差大于一定值
        // if (roll > mode_2_1_startroll && std::abs(mode_2_pressure - force) > mode_2_ads)
        // {
        //     // 推杆实际受拉力，需要下压
        //     if (force > 0)
        //     {
        //         // 定速值
        //         combined_value.data[0] = mode_2_speed;
        //         // 电压值
        //         combined_value.data[1] = std::abs(mode_voltage); // 取绝对值
        //         // 方向 0：升 1：降
        //         combined_value.data[2] = 1; // 方向 0：升 1：降
        //     }
        //     // 推杆实际受推力（或不受力），需要上升
        //     else
        //     {
        //         // 定速值
        //         combined_value.data[0] = mode_2_speed;
        //         // 电压值
        //         combined_value.data[1] = std::abs(mode_voltage); // 取绝对值
        //         // 方向 0：升 1：降
        //         combined_value.data[2] = 0; // 方向 0：升 1：降
        //     }
        // }
        // // 关闭条件，俯角达到88度以上
        // else if (roll > 88.0)
        // {
        //     // 停止推杆,定慢速前行
        //     combined_value.data[0] = mode_2_speed;
        //     combined_value.data[1] = 0.0;
        //     combined_value.data[2] = 2; // 停止
        // }
        // else
        // {
        //     // 不满足开启条件，停止推杆,定慢速前行
        //     combined_value.data[0] = mode_2_speed;
        //     combined_value.data[1] = 0.0;
        //     combined_value.data[2] = 2; // 停止
        // }
    }
    void mode_03(float pitch, float roll, float distance, float force)
    {
    }
    void mode_04(float pitch, float roll, float distance, float force)
    {
    }
};

int main(int argc, char const *argv[])
{
    // 2.初始化ROS2客户端；
    rclcpp::init(argc, argv);
    // 4.调用spain函数，并传入节点对象指针；
    rclcpp::spin(std::make_shared<Mode_Switching>());
    // 5.资源释放
    rclcpp::shutdown();
    return 0;
}
