// 1.包含头文件
#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/int32.hpp"
#include "std_msgs/msg/float64_multi_array.hpp"
#include "sensor_msgs/msg/range.hpp"

using std::placeholders::_1;
using namespace std::chrono_literals;
float r;
float p;
float distance;
float weight_sensor;

// 3.自定义节点类；
class Sensor_Collect_Node : public rclcpp::Node
{
public:
    Sensor_Collect_Node() : Node("sensor_collect_node_node_cpp")
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
        sub_callback_group_3 = create_callback_group(rclcpp::CallbackGroupType::MutuallyExclusive);
        rclcpp::SubscriptionOptions options_sub_3;
        options_sub_3.callback_group = sub_callback_group_3;
        // imu and laser 的发送 timer
        sendtimer_callback_group_ = create_callback_group(rclcpp::CallbackGroupType::MutuallyExclusive);
        // 话题及定时器定义
        imu_sub_ = this->create_subscription<std_msgs::msg::Float64MultiArray>("/imu_data_pub", 10, std::bind(&Sensor_Collect_Node::imu_callback, this, _1), options_sub_1);
        dis_sub_ = this->create_subscription<sensor_msgs::msg::Range>("/laser", 10, std::bind(&Sensor_Collect_Node::range_callback, this, _1), options_sub_2);
        weight_sub = this->create_subscription<std_msgs::msg::Int32>("/weigh_talker_3", 10, std::bind(&Sensor_Collect_Node::weight_callback, this, _1), options_sub_3);
        // 发布综合数据
        value_pub_ = this->create_publisher<std_msgs::msg::Float64MultiArray>("/imu_and_laser", 10);
        send_timer_ = this->create_wall_timer(40ms, std::bind(&Sensor_Collect_Node::senddata_cb, this), sendtimer_callback_group_);
    }

private:
    rclcpp::Subscription<std_msgs::msg::Float64MultiArray>::SharedPtr imu_sub_;
    rclcpp::Subscription<sensor_msgs::msg::Range>::SharedPtr dis_sub_;
    rclcpp::Subscription<std_msgs::msg::Int32>::SharedPtr weight_sub;

    rclcpp::Publisher<std_msgs::msg::Float64MultiArray>::SharedPtr value_pub_;

    rclcpp::CallbackGroup::SharedPtr sub_callback_group_1;
    rclcpp::CallbackGroup::SharedPtr sub_callback_group_2;
    rclcpp::CallbackGroup::SharedPtr sub_callback_group_3;
    rclcpp::CallbackGroup::SharedPtr sendtimer_callback_group_;

    rclcpp::TimerBase::SharedPtr send_timer_;

    void imu_callback(const std_msgs::msg::Float64MultiArray msg)
    {
        // 此处的值已经调换，防止欧拉角万向节
        // 翻滚
        r = msg.data[0];
        // 俯仰
        p = msg.data[1];
    }
    void range_callback(const sensor_msgs::msg::Range::SharedPtr msg)
    {
        // laser距离值，单位：m
        distance = msg->range;
        if (distance > 65)
        {
            distance = 0.000;
        }
    }
    void weight_callback(const std_msgs::msg::Int32 msg)
    {
        weight_sensor = - msg.data;
    }
    void senddata_cb()
    {
        // std::cout<<"r:"<<r<<" "<<"p: "<<p<<" "<<"distance:"<<distance<<std::endl;
        auto combined_value = std_msgs::msg::Float64MultiArray();
        combined_value.data.resize(4);
        combined_value.data[0] = r;
        combined_value.data[1] = p;
        combined_value.data[2] = distance;
        combined_value.data[3] = weight_sensor / 1000;
        value_pub_->publish(combined_value);
    }
};

int main(int argc, char const *argv[])
{
    // 2.初始化ROS2客户端；
    rclcpp::init(argc, argv);
    // 4.调用spain函数，并传入节点对象指针；
    auto node = std::make_shared<Sensor_Collect_Node>();
    rclcpp::executors::MultiThreadedExecutor executor;
    executor.add_node(node);
    executor.spin();
    // 5.资源释放
    rclcpp::shutdown();
    return 0;
}