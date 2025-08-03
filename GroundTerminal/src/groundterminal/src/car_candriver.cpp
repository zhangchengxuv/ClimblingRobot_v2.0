// 1.包含头文件
#include "rclcpp/rclcpp.hpp"
#include <std_msgs/msg/float64_multi_array.hpp>
#include "geometry_msgs/msg/twist.hpp"
#include "std_msgs/msg/int32.hpp"
#include "std_msgs/msg/float32.hpp"
#include "signal.h"
#include <iostream>
#include <unistd.h>
#include <array>
#include <fcntl.h>
#include "groundterminal/myconfig.h"
#include "groundterminal/cansend.h"

using namespace std::chrono_literals;
using namespace std;
std::mutex can_mutex;           // 全局互斥锁
std::atomic<bool> m_run0(true); // 使用原子变量
bool change_flag = false;
// 模式
int mode = 0;
int arm_state = 0;
float pitch;
float roll;

// 数据获取线程
int ret;
pthread_t dataread_thread;
// 将电机速度转换为十六进制数据
std::array<unsigned char, 5> createOutspeed(int dec_speed);
std::array<unsigned char, 5> createOutdegree(int dec_degree);

void signal_handler(int signal);

// 自定义节点类；
class Can_Driver : public rclcpp::Node
{
public:
  Can_Driver() : Node("can_driver_node_cpp")
  {
    // 数据获取线程
    // 设置socket为非阻塞模式
    int flags = fcntl(can0_socket, F_GETFL, 0);
    fcntl(can0_socket, F_SETFL, flags | O_NONBLOCK);
    ret = pthread_create(&dataread_thread, NULL, &Can_Driver::receive_func, this);
    // 创建可重入回调组
    // 创建订阅选项，并设置回调组
    // 订阅
    sub_callback_group_ = create_callback_group(rclcpp::CallbackGroupType::Reentrant);
    rclcpp::SubscriptionOptions options_sub;
    options_sub.callback_group = sub_callback_group_;
    // 方向订阅回调组
    direction_sub_callback_group_ = create_callback_group(rclcpp::CallbackGroupType::MutuallyExclusive);
    rclcpp::SubscriptionOptions options_direction_sub;
    options_direction_sub.callback_group = direction_sub_callback_group_;
    // 度数订阅回调组
    degree_sub_callback_group_ = create_callback_group(rclcpp::CallbackGroupType::MutuallyExclusive);
    rclcpp::SubscriptionOptions options_degree_sub;
    options_degree_sub.callback_group = degree_sub_callback_group_;
    // 遥杆速度订阅回调组
    joystick_speed_callback_group_ = create_callback_group(rclcpp::CallbackGroupType::MutuallyExclusive);
    rclcpp::SubscriptionOptions joystick_speed_sub;
    joystick_speed_sub.callback_group = joystick_speed_callback_group_;
    // 遥杆转向速度订阅回调组
    turn_speed_callback_group_ = create_callback_group(rclcpp::CallbackGroupType::MutuallyExclusive);
    rclcpp::SubscriptionOptions turn_speed_sub;
    turn_speed_sub.callback_group = turn_speed_callback_group_;
    // 巡航速度订阅回调组
    cruising_speed_callback_group_ = create_callback_group(rclcpp::CallbackGroupType::MutuallyExclusive);
    rclcpp::SubscriptionOptions cruising_speed_sub;
    cruising_speed_sub.callback_group = cruising_speed_callback_group_;
    // 遥杆和巡航车体状态订阅回调组
    joystick_or_cruising_callback_group_ = create_callback_group(rclcpp::CallbackGroupType::MutuallyExclusive);
    rclcpp::SubscriptionOptions joystick_or_cruising_sub;
    joystick_or_cruising_sub.callback_group = joystick_or_cruising_callback_group_;
    // IMU数据订阅回调组
    imudata_sub_callback_group_ = create_callback_group(rclcpp::CallbackGroupType::MutuallyExclusive);
    rclcpp::SubscriptionOptions options_imudata_sub;
    options_imudata_sub.callback_group = imudata_sub_callback_group_;
    // 定时器
    timer_callback_group_ = create_callback_group(rclcpp::CallbackGroupType::Reentrant);
    sendtimer_callback_group_ = create_callback_group(rclcpp::CallbackGroupType::Reentrant);

    // 方向订阅
    this->direction_sub_ = this->create_subscription<std_msgs::msg::Int32>("direction", 10,
                                                                           std::bind(&Can_Driver::directionCallback, this, std::placeholders::_1), options_direction_sub);
    // 度数订阅
    this->degree_sub_ = this->create_subscription<std_msgs::msg::Float32>("degree", 10,
                                                                          std::bind(&Can_Driver::degree_control_cb, this, std::placeholders::_1), options_degree_sub);
    // 遥杆速度订阅
    this->speed_sub_ = this->create_subscription<std_msgs::msg::Int32>("speed_joystick", 10,
                                                                       std::bind(&Can_Driver::joyspeedCallback, this, std::placeholders::_1), joystick_speed_sub);
    // 转向速度订阅
    this->turn_speed_sub_ = this->create_subscription<std_msgs::msg::Int32>("trunspeed_joystick", 10,
                                                                            std::bind(&Can_Driver::joyturnspeedCallback, this, std::placeholders::_1), turn_speed_sub);
    // 遥杆和巡航车体状态订阅
    this->joystick_or_cruising_sub_ = this->create_subscription<std_msgs::msg::Int32>("joystick_or_cruising", 10,
                                                                                      std::bind(&Can_Driver::joystickOrCruisingCallback, this, std::placeholders::_1), joystick_or_cruising_sub);
    // 巡航速度订阅
    this->cruising_speed_sub_ = this->create_subscription<std_msgs::msg::Int32>("cruising_speed", 10,
                                                                                std::bind(&Can_Driver::cruisingSpeedCallback, this, std::placeholders::_1), cruising_speed_sub);
    // 操作模式切换
    this->mode_sub_ = this->create_subscription<std_msgs::msg::Int32>("mode_flag", 10,
                                                                      std::bind(&Can_Driver::modeCallback, this, std::placeholders::_1), options_sub);
    // imu数据订阅
    this->imu_data_subr = this->create_subscription<std_msgs::msg::Float64MultiArray>("imu_data_pub", 10,
                                                                                      std::bind(&Can_Driver::imu_calculate_Callback, this, std::placeholders::_1), options_imudata_sub);
    // 创建实时速度发布方
    rtspeed1_pub_ = this->create_publisher<std_msgs::msg::Int32>("rt1_speed", 10);
    rtspeed2_pub_ = this->create_publisher<std_msgs::msg::Int32>("rt2_speed", 10);
    rtspeed3_pub_ = this->create_publisher<std_msgs::msg::Int32>("rt3_speed", 10);
    rtspeed4_pub_ = this->create_publisher<std_msgs::msg::Int32>("rt4_speed", 10);
    // 实时位置发布方
    rtspos_pub_ = this->create_publisher<std_msgs::msg::Float32>("rt_position", 10);

    // 定时器
    sp_timer_ = this->create_wall_timer(25ms, std::bind(&Can_Driver::on_rtsp_timer, this), timer_callback_group_);
    send_timer_ = this->create_wall_timer(25ms, std::bind(&Can_Driver::senddata_cb, this), sendtimer_callback_group_);
  }

private:
  std::atomic<bool> twist_called_{false}; // 新增原子标志
  std::mutex motor_mutex_;                // 新增互斥锁保护数据
  // 成员变量
  rclcpp::CallbackGroup::SharedPtr timer_callback_group_;
  rclcpp::CallbackGroup::SharedPtr sendtimer_callback_group_;

  rclcpp::CallbackGroup::SharedPtr sub_callback_group_;
  rclcpp::CallbackGroup::SharedPtr joystick_speed_callback_group_;
  rclcpp::CallbackGroup::SharedPtr turn_speed_callback_group_;
  rclcpp::CallbackGroup::SharedPtr imudata_sub_callback_group_;
  rclcpp::CallbackGroup::SharedPtr direction_sub_callback_group_;
  rclcpp::CallbackGroup::SharedPtr degree_sub_callback_group_;
  rclcpp::CallbackGroup::SharedPtr cruising_speed_callback_group_;
  rclcpp::CallbackGroup::SharedPtr joystick_or_cruising_callback_group_;

  // 键盘控制节点订阅
  rclcpp::Subscription<std_msgs::msg::Int32>::SharedPtr direction_sub_;
  // 度数订阅
  rclcpp::Subscription<std_msgs::msg::Float32>::SharedPtr degree_sub_;
  // 订阅IMU数据
  rclcpp::Subscription<std_msgs::msg::Float64MultiArray>::SharedPtr imu_data_subr;
  // 遥杆速度
  rclcpp::Subscription<std_msgs::msg::Int32>::SharedPtr speed_sub_;
  // 转向速度
  rclcpp::Subscription<std_msgs::msg::Int32>::SharedPtr turn_speed_sub_;
  // 巡航速度
  rclcpp::Subscription<std_msgs::msg::Int32>::SharedPtr cruising_speed_sub_;
  // 遥杆和巡航车体状态
  rclcpp::Subscription<std_msgs::msg::Int32>::SharedPtr joystick_or_cruising_sub_;
  // 模式订阅
  rclcpp::Subscription<std_msgs::msg::Int32>::SharedPtr mode_sub_;
  // 四个车轮的实时速度反馈
  rclcpp::Publisher<std_msgs::msg::Int32>::SharedPtr rtspeed1_pub_;
  rclcpp::Publisher<std_msgs::msg::Int32>::SharedPtr rtspeed2_pub_;
  rclcpp::Publisher<std_msgs::msg::Int32>::SharedPtr rtspeed3_pub_;
  rclcpp::Publisher<std_msgs::msg::Int32>::SharedPtr rtspeed4_pub_;
  // 实时位置反馈
  rclcpp::Publisher<std_msgs::msg::Float32>::SharedPtr rtspos_pub_;

  // 定义计时器
  rclcpp::TimerBase::SharedPtr sp_timer_;
  rclcpp::TimerBase::SharedPtr send_timer_;

  // 遥杆速度回调函数
  void joyspeedCallback(const std_msgs::msg::Int32::SharedPtr speed_msg)
  {

    joyspeed = speed_msg->data;
    // RCLCPP_INFO(this->get_logger(), "速度：%d", joyspeed);
  }
  // 转向速度回调函数
  void joyturnspeedCallback(const std_msgs::msg::Int32::SharedPtr speed_msg)
  {

    joyturuspeed = speed_msg->data;
    // RCLCPP_INFO(this->get_logger(), "转向：%d", joyturuspeed);
  }
  // 巡航速度回调函数
  void cruisingSpeedCallback(const std_msgs::msg::Int32::SharedPtr speed_msg)
  {
    cruising_speed = speed_msg->data;
    // RCLCPP_INFO(this->get_logger(), "巡航速度：%d", cruising_speed);
  }
  void joystickOrCruisingCallback(const std_msgs::msg::Int32::SharedPtr mode_msg)
  {
    // 根据遥杆或巡航模式设置速度
    if (mode_msg->data == 0) // 遥杆模式
    {
      // 先做裁剪
      joyspeed = std::min(joyspeed, 2000);
      joyturuspeed = std::min(joyturuspeed, 2000);
      // 赋值给控制量
      control_speed = joyspeed;
      control_turnspeed = joyturuspeed;
    }
    else if (mode_msg->data == 1) // 巡航模式
    {
      // 先做裁剪
      cruising_speed = std::min(cruising_speed, 2000);
      // 赋值给控制量
      control_speed = cruising_speed;
      control_turnspeed = cruising_speed;
    }
  }

  // 操作模式回调函数
  void modeCallback(const std_msgs::msg::Int32::SharedPtr mode_msg)
  {
    mode = mode_msg->data;
  }
  // imu数据回调函数
  void imu_calculate_Callback(const std_msgs::msg::Float64MultiArray::SharedPtr imu_data)
  {
    pitch = imu_data->data[1] + 90;
    roll = imu_data->data[0];
  }
  void degree_control_cb(const std_msgs::msg::Float32 degree_msg)
  {
    switch (arm_state)
    {
    case 0:
    {
      sleep(1); // 等待1秒
      arm_state = 1;
      break;
    }
    case 1:
    {
      if (std::abs(std::abs(degree_msg.data) - std::abs(degree_position)) < 2)
      {
        arm_state = 2;
      }
      break;
    }
    case 2:
    {
      // 发送控制位置
      int motor_degree = (degree_msg.data / 360.0) * 65536.0 * 101; // 转换为电机位置
      auto out_degree = createOutdegree(motor_degree);
      for (int i = 0; i < 5; ++i)
      {
        motor_05[i] = out_degree[i]; // 更新推杆电机数据
      }
      SendData(can0_socket, motor_id_05, false, motor_05, 1);
      break;
    }
    }
  }
  // 综合发布函数
  void senddata_cb()
  {
    bool called = twist_called_.exchange(false);    // 读取并重置标志
    std::lock_guard<std::mutex> lock(motor_mutex_); // 加锁

    // 发送速度获取数据
    SendData(can0_socket, motor_id_01, false, get_speed, 1);
    SendData(can0_socket, motor_id_02, false, get_speed, 1);
    SendData(can0_socket, motor_id_03, false, get_speed, 1);
    SendData(can0_socket, motor_id_04, false, get_speed, 1);
    // 发送位置获取命令
    SendData(can0_socket, motor_id_05, false, get_position, 1);

    if (called)
    {
      // directionCallback 被调用时的逻辑
      SendData(can0_socket, motor_id_01, false, motor_01, 5);
      SendData(can0_socket, motor_id_02, false, motor_02, 5);
      SendData(can0_socket, motor_id_03, false, motor_03, 5);
      SendData(can0_socket, motor_id_04, false, motor_04, 5);
    }
    else
    {
      // directionCallback 未被调用时的逻辑
      SendData(can0_socket, motor_id_01, false, speed_zero, 5);
      SendData(can0_socket, motor_id_02, false, speed_zero, 5);
      SendData(can0_socket, motor_id_03, false, speed_zero, 5);
      SendData(can0_socket, motor_id_04, false, speed_zero, 5);
    }
  }
  // 操作方向回调函数
  void directionCallback(const std_msgs::msg::Int32 msg)
  {
    twist_called_.store(true); // 标记回调被触发

    switch (mode)
    {
    // 正常模式
    case 0:
    {
      // --------------前进--------------
      if (msg.data == 1)
      {
        // 前进时左侧1和3轮速度一致
        auto leftwheel = createOutspeed(round(control_speed));
        for (int i = 0; i < 5; ++i)
        {
          motor_01[i] = leftwheel[i];
          motor_03[i] = leftwheel[i];
        }
        // 前进时右侧2和4轮速度一致
        auto rightwheel = createOutspeed(round(-control_speed));
        for (int i = 0; i < 5; ++i)
        {
          motor_02[i] = rightwheel[i];
          motor_04[i] = rightwheel[i];
        }
      }
      // --------------后退--------------
      else if (msg.data == 2)
      {
        // 后退时左侧1和3轮速度一致
        auto leftwheel = createOutspeed(round(-control_speed));
        for (int i = 0; i < 5; ++i)
        {
          motor_01[i] = leftwheel[i];
          motor_03[i] = leftwheel[i];
        }
        // 后退时右侧2和4轮速度一致
        auto rightwheel = createOutspeed(round(control_speed));
        for (int i = 0; i < 5; ++i)
        {
          motor_02[i] = rightwheel[i];
          motor_04[i] = rightwheel[i];
        }
      }
      // --------------左转--------------
      else if (msg.data == 3)
      {
        // 转向时的直线速度
        int joyspeed_t = 0;
        // 转向时的旋转速度
        int joyspeed_f = control_turnspeed;
        // 1&3
        auto leftwheel = createOutspeed(-round(joyspeed_t + joyspeed_f));
        for (int i = 0; i < 5; ++i)
        {
          motor_01[i] = leftwheel[i];
          motor_03[i] = leftwheel[i];
        }
        // 2&4
        auto rightwheel = createOutspeed(-round(joyspeed_t + joyspeed_f));
        for (int i = 0; i < 5; ++i)
        {
          motor_02[i] = rightwheel[i];
          motor_04[i] = rightwheel[i];
        }
      }
      // --------------右转--------------
      else if (msg.data == 4)
      {
        // 转向时的直线速度
        int joyspeed_t = 0;
        // 转向时的旋转速度
        int joyspeed_f = control_turnspeed;
        // 1&3
        auto leftwheel = createOutspeed(round(joyspeed_t + joyspeed_f));
        for (int i = 0; i < 5; ++i)
        {
          motor_01[i] = leftwheel[i];
          motor_03[i] = leftwheel[i];
        }
        // 2&4
        auto rightwheel = createOutspeed(round(joyspeed_t + joyspeed_f));
        for (int i = 0; i < 5; ++i)
        {
          motor_02[i] = rightwheel[i];
          motor_04[i] = rightwheel[i];
        }
      }
      // --------------停止--------------
      else
      {
        // cout << "----- 停止" << endl;
        for (int i = 0; i < 5; ++i)
        {
          motor_01[i] = speed_zero[i];
          motor_03[i] = speed_zero[i];
          motor_02[i] = speed_zero[i];
          motor_04[i] = speed_zero[i];
        }
      }
      break;
    }
    // Imu辅助模式
    case 1:
    {
      pitch = pitch / 180 * 3.1415;
      double sin_val = std::sin(pitch);
      double cos_val = std::cos(pitch);
      float V_y = control_speed * cos_val / sin_val;

      // --------------前进--------------
      if (msg.data == 1)
      {
        // 前进时前方1和2轮速度一致
        auto frontwheel_1 = createOutspeed(round(V_y));
        auto frontwheel_2 = createOutspeed(round(-V_y));
        for (int i = 0; i < 5; ++i)
        {
          motor_01[i] = frontwheel_1[i];
        }
        for (int i = 0; i < 5; ++i)
        {
          motor_02[i] = frontwheel_2[i];
        }
        // 前进时后方3和4轮速度一致
        auto behindwheel_3 = createOutspeed(round(control_speed));
        auto behindwheel_4 = createOutspeed(round(-control_speed));
        for (int i = 0; i < 5; ++i)
        {
          motor_03[i] = behindwheel_3[i];
        }
        for (int i = 0; i < 5; ++i)
        {
          motor_04[i] = behindwheel_4[i];
        }
      }
      // --------------后退--------------
      else if (msg.data == 2)
      {
        // 前方1和2轮速度一致
        auto frontwheel_1 = createOutspeed(round(-control_speed));
        auto frontwheel_2 = createOutspeed(round(control_speed));
        for (int i = 0; i < 5; ++i)
        {
          motor_01[i] = frontwheel_1[i];
        }
        for (int i = 0; i < 5; ++i)
        {
          motor_02[i] = frontwheel_2[i];
        }
        // 后方3和4轮速度一致
        auto behindwheel_3 = createOutspeed(round(-control_speed));
        auto behindwheel_4 = createOutspeed(round(control_speed));
        for (int i = 0; i < 5; ++i)
        {
          motor_03[i] = behindwheel_3[i];
        }
        for (int i = 0; i < 5; ++i)
        {
          motor_04[i] = behindwheel_4[i];
        }
      }
      // --------------停止--------------
      else
      {
        for (int i = 0; i < 5; ++i)
        {
          motor_01[i] = speed_zero[i];
          motor_03[i] = speed_zero[i];
          motor_02[i] = speed_zero[i];
          motor_04[i] = speed_zero[i];
        }
      }

      break;
    }
    }
  }
  // 接收数据函数
  static void *receive_func(void *param)
  {
    Can_Driver *speed_data = static_cast<Can_Driver *>(param);
    speed_data->ReceiveData();
    return nullptr;
  }
  void ReceiveData()
  {
    // 接收数据
    can_frame frame;
    sockaddr_can addr;
    socklen_t len = sizeof(addr);
    while (rclcpp::ok() && m_run0)
    {
      int nbytes = recvfrom(can0_socket, &frame, sizeof(frame), 0,
                            (struct sockaddr *)&addr, &len);

      if (nbytes < 0)
      {
        if (errno == EAGAIN)
        { // 非阻塞模式无数据
          std::this_thread::sleep_for(10ms);
          continue;
        }
        break;
      }

      switch (frame.can_id)
      {
      case 0x001:
      {
        for (int i = 0; i < 5; i++)
        {
          // printf("%02x", (int)frame.data[i]);
          receiveData[i] = (int)frame.data[i];
        }
        realData[0] = receiveData[2];
        realData[1] = receiveData[1];
        int16_t hexval = (static_cast<uint16_t>(realData[0] << 8) | realData[1]);
        int decval1 = static_cast<int>(hexval);
        // 发布信息
        rt1_data.data = decval1;
      }
      break;

      case 0x002:
      {
        for (int i = 0; i < 5; i++)
        {
          receiveData[i] = (int)frame.data[i];
        }
        realData[0] = receiveData[2];
        realData[1] = receiveData[1];
        int16_t hexval = (static_cast<uint16_t>(realData[0] << 8) | realData[1]);
        int decval2 = static_cast<int>(hexval);
        // 发布信息
        rt2_data.data = decval2;
      }
      break;

      case 0x003:
      {
        for (int i = 0; i < 5; i++)
        {
          receiveData[i] = (int)frame.data[i];
        }
        realData[0] = receiveData[2];
        realData[1] = receiveData[1];
        int16_t hexval = (static_cast<uint16_t>(realData[0] << 8) | realData[1]);
        int decval3 = static_cast<int>(hexval);
        // 发布信息
        rt3_data.data = decval3;
      }
      break;

      case 0x004:
      {
        for (int i = 0; i < 5; i++)
        {
          receiveData[i] = (int)frame.data[i];
        }
        realData[0] = receiveData[2];
        realData[1] = receiveData[1];
        int16_t hexval = (static_cast<uint16_t>(realData[0] << 8) | realData[1]);
        int decval4 = static_cast<int>(hexval);
        // 发布信息
        rt4_data.data = decval4;
      }
      break;

      case 0x005:
      {
        for (int i = 0; i < 5; i++)
        {
          receiveData[i] = (int)frame.data[i];
        }
        // 获取原始位置数据
        positionData[0] = receiveData[4];
        positionData[1] = receiveData[3];
        positionData[2] = receiveData[2];
        positionData[3] = receiveData[1];
        int32_t value = (positionData[0] << 24) | (positionData[1] << 16) | (positionData[2] << 8) | positionData[3];
        int decval5 = static_cast<int>(value);
        degree_position = (decval5 / 65536.0 / 101) * 360; // 转换为度数
        rp5_data.data = degree_position;                   // 发布位置数据
      }
      break;
      }
    }
  }
  // 速度反馈函数 此处应该调用回调组
  void on_rtsp_timer()
  {
    rtspeed1_pub_->publish(rt1_data);
    rtspeed2_pub_->publish(rt2_data);
    rtspeed3_pub_->publish(rt3_data);
    rtspeed4_pub_->publish(rt4_data);
    rtspos_pub_->publish(rp5_data); // 发布实时位置数据
  }
};

int main(int argc, char const *argv[])
{
  // 注册信号处理函数
  std::signal(SIGINT, signal_handler);
  // 初始化CAN设备
  can0_socket = init_can("can0");
  rclcpp::init(argc, argv);
  auto node = std::make_shared<Can_Driver>();
  rclcpp::executors::MultiThreadedExecutor executor;
  executor.add_node(node);
  executor.spin();
  return 0;
}

// 转换为速度发送，小字端
std::array<unsigned char, 5> createOutspeed(int dec_speed)
{
  return {
      0x1D,
      static_cast<unsigned char>(dec_speed & 0xFF),
      static_cast<unsigned char>((dec_speed >> 8) & 0xFF),
      static_cast<unsigned char>((dec_speed >> 16) & 0xFF),
      static_cast<unsigned char>((dec_speed >> 24) & 0xFF)};
}
// 转换为位置发送，小字端
std::array<unsigned char, 5> createOutdegree(int dec_degree)
{
  return {
      0x1E,
      static_cast<unsigned char>(dec_degree & 0xFF),
      static_cast<unsigned char>((dec_degree >> 8) & 0xFF),
      static_cast<unsigned char>((dec_degree >> 16) & 0xFF),
      static_cast<unsigned char>((dec_degree >> 24) & 0xFF)};
}

// 信号处理函数
void signal_handler(int signal)
{
  (void)signal;
  std::cout << "程序收到中断信号, 正在安全退出..." << std::endl;
  // 停止电机
  SendData(can0_socket, motor_id_01, false, motor_stop, 1);
  SendData(can0_socket, motor_id_02, false, motor_stop, 1);
  SendData(can0_socket, motor_id_03, false, motor_stop, 1);
  SendData(can0_socket, motor_id_04, false, motor_stop, 1);
  rclcpp::shutdown();
  m_run0 = false;
  // 关闭CAN设备
  close(can0_socket);
  cout << "CAN设备关闭" << std::endl;
}