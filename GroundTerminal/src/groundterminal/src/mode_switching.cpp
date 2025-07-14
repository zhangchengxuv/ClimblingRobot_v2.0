// 1.包含头文件
#include "rclcpp/rclcpp.hpp"
#include "mode_switching.h"

// 3.自定义节点类；
class Mode_Switching : public rclcpp::Node
{
public:
    Mode_Switching() : Node("mode_switching_node_cpp")
    {
    }

private:
};

void mode_01()
{
    std::cout << "模式 01: 正常模式" << std::endl;
    // 执行模式 01 的逻辑
    auto new_state = std::make_shared<SharedState>(*g_state_ptr.load());
    new_state->v1 = 3.3f;
    new_state->v2 = 3.3f;
    new_state->mode = 2;
    g_state_ptr.store(new_state);
}

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
