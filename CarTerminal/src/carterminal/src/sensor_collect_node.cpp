// 1.包含头文件
#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/float64_multi_array.hpp"
#include "sensor_msgs/msg/range.hpp"

using std::placeholders::_1;
using namespace std::chrono_literals;
float r;
float p;
float distance;

// 3.自定义节点类；
class Sensor_Collect_Node : public rclcpp::Node
{
public:
    Sensor_Collect_Node() : Node("sensor_collect_node_node_cpp")
    {
        sub_callback_group_1 = create_callback_group(rclcpp::CallbackGroupType::MutuallyExclusive);
        rclcpp::SubscriptionOptions options_sub_1;
        options_sub_1.callback_group = sub_callback_group_1;

        sub_callback_group_2 = create_callback_group(rclcpp::CallbackGroupType::MutuallyExclusive);
        rclcpp::SubscriptionOptions options_sub_2;
        options_sub_2.callback_group = sub_callback_group_2;

        sendtimer_callback_group_ = create_callback_group(rclcpp::CallbackGroupType::MutuallyExclusive);


        imu_sub_ = this->create_subscription<std_msgs::msg::Float64MultiArray>("/imu_data_pub", 10, std::bind(&Sensor_Collect_Node::imu_callback, this, _1), options_sub_1);
        dis_sub_ = this->create_subscription<sensor_msgs::msg::Range>("/laser", 10, std::bind(&Sensor_Collect_Node::range_callback, this, _1), options_sub_2);
        send_timer_ = this->create_wall_timer(40ms, std::bind(&Sensor_Collect_Node::senddata_cb, this), sendtimer_callback_group_);
    }

private:
    rclcpp::Subscription<std_msgs::msg::Float64MultiArray>::SharedPtr imu_sub_;
    rclcpp::Subscription<sensor_msgs::msg::Range>::SharedPtr dis_sub_;

    rclcpp::CallbackGroup::SharedPtr sub_callback_group_1;
    rclcpp::CallbackGroup::SharedPtr sub_callback_group_2;
    rclcpp::CallbackGroup::SharedPtr sendtimer_callback_group_;

    rclcpp::TimerBase::SharedPtr send_timer_;

    void imu_callback(const std_msgs::msg::Float64MultiArray msg)
    {
        // 左右翻滚
        r = msg.data[0];
        p = msg.data[1];
    }
    void range_callback(const sensor_msgs::msg::Range::SharedPtr msg)
    {
        distance = msg->range;
    }

    void senddata_cb()
    {
        std::cout<<"r:"<<r<<" "<<"p: "<<p<<" "<<"distance:"<<distance<<std::endl;
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