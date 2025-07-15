// 1.包含头文件
#include "rclcpp/rclcpp.hpp"
#include <mutex>
#include "std_msgs/msg/int32.hpp"
#include "std_msgs/msg/float64_multi_array.hpp"
#include "groundterminal/mode_switching.h"

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
        float weight = msg->data[3];
        if (state == 1)
        {
            mode_01(pitch, roll, distance, weight);
        }
        else
        {
            std::cout << "模式切换未启用" << std::endl;
        }
    }
    void mode_01(float pitch, float roll, float distance, float weight)
    {
        auto combined_value = std_msgs::msg::Float64MultiArray();
        combined_value.data.resize(3);
        combined_value.data[0] = mode_speed_value;
        combined_value.data[1] = mode_voltage_value;
        combined_value.data[2] = mode_wheel_direction;
        value_pub_->publish(combined_value);
        std::cout << "pitch :" << pitch << " roll: " << roll << " distance: " << distance << " weight: " << weight << std::endl;
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
