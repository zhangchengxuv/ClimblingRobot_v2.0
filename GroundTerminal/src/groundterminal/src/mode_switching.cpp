// 1.包含头文件

#include <mutex>
#include <cmath>
#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/int32.hpp"
#include "std_msgs/msg/float64_multi_array.hpp"
#include "groundterminal/mode_switching.h"

// 模式开关
int state = 0;

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
float mode_2_speed;                // 模式2的速度
float mode_2_1_startroll = 1.0;    // 模式2开始的roll角度
float mode_2_kp = 0.33;             // 模式2的比例系数
float mode_2_expect = -15.0;       // 模式2的推杆期望下压力
float mode_2_max_voltage_1 = 15.0; // 模式2的最大电压
float mode_2_max_voltage_2 = 15.0; // 模式2的最大电压
float mode_2_ads = 15.0;           // 期望电压与实际电压的绝对值
float mode_2_1_voltage;            // 模式2 阶段一 的电压
int temp_state = 0;                // 临时状态机阶段

// 模式3参数
int mode3_state = 0;   // 模式3状态机阶段
float mode_3_speed;    // 模式3的速度
float mode_3_distance; // 模式3的距离
float mode_3_1_minroll;
float mode_3_1_maxroll;
float mode_3_1_voltage = 0.0;
float mode_3_kp;          // 模式2的比例系数
float mode_3_expect;      // 模式2的推杆期望下压力
float mode_3_max_voltage; // 模式2的最大电压
float mode_3_ads;         // 期望电压与实际电压的绝对值
float mode_3_2_voltage;   // 模式2 阶段一 的电压

using std::placeholders::_1;
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
        if (state == 1)
        {
            // 模式开启
            mode_02(pitch, roll, distance, force);
        }
        else
        {
            // std::cout << "模式切换未启用" << std::endl;
        }
    }
    void mode_01(float pitch, float roll, float distance, float force)
    {
        (void)pitch; // 忽略pitch
        (void)force; // 忽略force
        // 处理模式1的逻辑
        auto combined_value = std_msgs::msg::Float64MultiArray();
        combined_value.data.resize(3);
        // 根据激光传感器判断车头是否探出
        if (distance > mode_1_distance)
        {
            // 第一阶段 推杆慢速
            if (roll < mode_1_1_maxroll)
            {
                // 定速值
                combined_value.data[0] = mode_1_speed;
                // 电压值
                combined_value.data[1] = mode_1_1_voltage;
                // 方向 0：升 1：降
                combined_value.data[2] = 0;
            }
            // 第二阶段 推杆快速
            else if (roll > mode_1_2_minroll)
            {
                // 定速值
                combined_value.data[0] = mode_1_speed;
                // 电压值
                combined_value.data[1] = mode_1_2_voltage;
                // 方向 0：升 1：降
                combined_value.data[2] = 0;
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
        (void)pitch;    // 忽略pitch
        (void)distance; // 忽略distance
        // 处理模式2的逻辑
        auto combined_value = std_msgs::msg::Float64MultiArray();
        combined_value.data.resize(3);
        // 定速值 确保车辆一直有速度
        combined_value.data[0] = mode_2_speed;
        // 俯角大于某个值后开启
        // if (roll < mode_2_1_startroll && roll > -89.5)
        if(true)
        {
            // 实际力小于期望力 且 力差值大于阈值 需要上拉
            if (force < mode_2_expect && std::abs(mode_2_expect - force) > mode_2_ads)
            {
                // 比例 * 力差值
                float mode_temp_voltage = mode_2_kp * std::abs(mode_2_expect - force);
                // 限制最大电压
                float mode_voltage = std::min(mode_temp_voltage, mode_2_max_voltage_1);
                //
                combined_value.data[1] = mode_voltage; // 阶段2 电压
                combined_value.data[2] = 0;            // 方向 0：升|1：降
                // std::cout << "upupupup" <<"force :"<<force<<"std::abs(mode_2_expect - force) "<<std::abs(mode_2_expect - force) << std::endl;
            }
            // 实际力大于期望力 且 力差值大于阈值 需要下压
            else if (force > mode_2_expect && std::abs(mode_2_expect - force) > mode_2_ads)
            {
                // 比例 * 力差值
                float mode_temp_voltage = mode_2_kp * std::abs(mode_2_expect - force);
                // 限制最大电压
                float mode_voltage = std::min(mode_temp_voltage, mode_2_max_voltage_2);
                //
                combined_value.data[1] = mode_voltage; // 阶段2 电压
                combined_value.data[2] = 1;            // 方向 0：升|1：降
                // std::cout << "downwown" <<"force :"<<force<<"std::abs(mode_2_expect - force) "<<std::abs(mode_2_expect - force) << std::endl;

            }
            // 实际力接近期望力，且力差值小于阈值，停止推杆动作，继续往前走
            else
            {
                // 力差值小于阈值，保持原有状态
                combined_value.data[1] = 0.0; // 停止
                combined_value.data[2] = 2;   // 停止
                // std::cout << "stop" << std::endl;

            }
        }
        // 关闭条件，俯角达到88度以上
        // else if (roll <= -89.5)
        // {
        //     if (temp_state == 0)
        //     {
        //         combined_value.data[0] = mode_2_speed; // 稍微加快速度
        //         combined_value.data[1] = 0.0;          // 停止
        //         combined_value.data[2] = 2;            // 停止
        //         temp_state = 1;
        //     }
        //     else if (temp_state == 1)
        //     {
        //         // 延时 2s
        //         rclcpp::sleep_for(std::chrono::seconds(2));
        //         temp_state = 2;
        //     }
        //     else if (temp_state == 2)
        //     {
        //         combined_value.data[0] = mode_2_speed; // 停止
        //         combined_value.data[1] = 0.0;          // 停止
        //         combined_value.data[2] = 2;            // 停止
        //     }
        // }

        // 发送数据
        value_pub_->publish(combined_value);
    }
    void mode_03(float pitch, float roll, float distance, float force)
    {
        (void)pitch; // 忽略pitch
        auto combined_value = std_msgs::msg::Float64MultiArray();
        combined_value.data.resize(3);
        switch (mode3_state)
        {
        case 0:
        {
            combined_value.data[0] = mode_3_speed; // 定速前进
            if (distance > mode_3_distance)
            {
                // 车头探出
                mode3_state = 1; // 进入下一阶段
            }
            break;
        }
        case 1:
        {
            // // 慢速上拉
            // if (roll > mode_3_1_minroll && roll < mode_3_1_maxroll)
            // {
            //     // 定速值
            //     combined_value.data[0] = mode_3_speed;
            //     // 电压值
            //     combined_value.data[1] = mode_3_1_voltage;
            //     // 方向 0：升 1：降
            //     combined_value.data[2] = 0;
            // }
            // else if (roll > mode_3_1_maxroll)
            // {
            //     // 进入下一阶段
            //     mode3_state = 2;
            // }
            // 定速值
            combined_value.data[0] = mode_3_speed;
            // 电压值
            combined_value.data[1] = mode_3_1_voltage;
            // 方向 0：升 1：降
            combined_value.data[2] = 0;
            if (roll > mode_3_1_maxroll)
            {
                // 进入下一阶段
                mode3_state = 2;
            }
            break;
        }
        case 2:
        {
            // 维持小推力？推杆自身重力影响
            if (force < mode_3_expect && std::abs(mode_3_expect - force) > mode_3_ads)
            {
                // 比例 * 力差值
                float mode_temp_voltage = mode_3_kp * std::abs(mode_3_expect - force);
                // 限制最大电压
                float mode_voltage = std::min(mode_temp_voltage, mode_3_max_voltage);
                combined_value.data[1] = mode_voltage; // 阶段2 电压
                combined_value.data[2] = 0;            // 方向 0：升|1：降
            }
            else if (force > mode_3_expect && std::abs(mode_3_expect - force) > mode_3_ads)
            {
                // 比例 * 力差值
                float mode_temp_voltage = mode_3_kp * std::abs(mode_3_expect - force);
                // 限制最大电压
                float mode_voltage = std::min(mode_temp_voltage, mode_3_max_voltage);
                combined_value.data[1] = mode_voltage; // 阶段2 电压
                combined_value.data[2] = 1;            // 方向 0：升|1：降
            }
            else
            {
                // 力差值小于阈值，保持原有状态
                combined_value.data[1] = 0.0; // 停止
                combined_value.data[2] = 2;   // 停止
            }
            // 关闭条件，俯角达到178度以上
            if (roll >= 178.0)
            {
                combined_value.data[0] = 0.4; // 稍微加快速度
                combined_value.data[1] = 0.0; // 停止
                combined_value.data[2] = 2;   // 停止
                mode3_state = 3;              // 进入下一阶段
            }
            break;
        }
        case 3:
        {
            // 延时 2s
            rclcpp::sleep_for(std::chrono::seconds(2));
            mode3_state = 4; // 进入下一阶段
            break;
        }
        case 4:
        {
            combined_value.data[0] = 0.0; // 停止
            combined_value.data[1] = 0.0; // 停止
            combined_value.data[2] = 2;   // 停止
            break;
        }
            // 发送数据
            value_pub_->publish(combined_value);
        }
    }
    // void mode_04(float pitch, float roll, float distance, float force)
    // {
    // }
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
