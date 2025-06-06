// 1.包含头文件
#include "rclcpp/rclcpp.hpp"

// 3.自定义节点类；
class ReadDatanode : public rclcpp::Node
{
public:
    ReadDatanode() : Node("ReadDatanode_node_cpp")
    {
    }
};

int main(int argc, char const *argv[])
{
    // 2.初始化ROS2客户端；
    rclcpp::init(argc, argv);
    // 4.调用spain函数，并传入节点对象指针；
    rclcpp::spin(std::make_shared<ReadDatanode>());
    // 5.资源释放
    rclcpp::shutdown();
    return 0;
}