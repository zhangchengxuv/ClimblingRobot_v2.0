#ifndef MYCONFIG_h
#define MYCONFIG_h

#include "geometry_msgs/msg/twist.hpp"
#include "std_msgs/msg/int32.hpp"
#include "signal.h"
#include <sstream>
#include <iomanip>
#include <pthread.h>

#define BYTE unsigned char

// 电机编号
int motor_id_01 = 0x0001; // 左前
int motor_id_02 = 0x0002; // 右前
int motor_id_03 = 0x0003; // 左后
int motor_id_04 = 0x0004; // 右后
int motor_id_05 = 0x0005; // 推杆电机

// 电机原始速度(单位：HZ)
int joyspeed = 0;
int joyturuspeed = 0;
int cruising_speed = 0;
// 下放速度
int control_speed = 0;
int control_turnspeed = 0;

// CAN0 socket描述符
int can0_socket = -1;

// 电机状态
BYTE speed_zero[5] = {0x1d, 0x00, 0x00, 0x00, 0x00}; // 速度归零
BYTE motor_stop[1] = {0x02};                         // 电机关闭

// 获取速度
BYTE get_speed[1] = {0x06};
// 获取位置
BYTE get_position[1] = {0x08};
// 位置
float degree_position;     // 当前位置，单位：度
float new_degree_position; // 新位置
float last_degree_position;// 上一次位置
bool is_position = false;
// 同一发送
BYTE motor_01[5] = {0x1d, 0x00, 0x00, 0x00, 0x00};
BYTE motor_02[5] = {0x1d, 0x00, 0x00, 0x00, 0x00};
BYTE motor_03[5] = {0x1d, 0x00, 0x00, 0x00, 0x00};
BYTE motor_04[5] = {0x1d, 0x00, 0x00, 0x00, 0x00};
BYTE motor_05[5] = {0x1e, 0x00, 0x00, 0x00, 0x00};
// 位置消息
BYTE degree_0[5] = {0x1e, 0x00, 0x00, 0x00, 0x00};
//
BYTE degree_counterclockwise_30[5] = {0x1e, 0xaa, 0xaa, 0x06, 0x00};
BYTE degree_counterclockwise__60[5] = {0x1e, 0x55, 0x55, 0x0d, 0x00};
BYTE degree_counterclockwise__90[5] = {0x1e, 0x00, 0x00, 0x14, 0x00};
BYTE degree_counterclockwise__120[5] = {0x1e, 0xaa, 0xaa, 0x1A, 0x00};
BYTE degree_counterclockwise__160[5] = {0x1e, 0x38, 0x8e, 0x23, 0x00};
//
BYTE degree_clockwise_30[5] = {0x1e, 0x56, 0x55, 0xF9, 0xFF};
BYTE degree_clockwise_60[5] = {0x1e, 0xAB, 0xAA, 0xF2, 0xFF};
BYTE degree_clockwise_90[5] = {0x1e, 0x00, 0x00, 0xEC, 0xFF};
BYTE degree_clockwise_120[5] = {0x1e, 0x55, 0x55, 0xE5, 0xFF};
BYTE degree_clockwise_160[5] = {0x1e, 0xC8, 0x71, 0xDC, 0xFF};

auto rt1_data = std_msgs::msg::Int32();
auto rt2_data = std_msgs::msg::Int32();
auto rt3_data = std_msgs::msg::Int32();
auto rt4_data = std_msgs::msg::Int32();

#endif