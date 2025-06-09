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

// 电机原始速度(单位：HZ)
int V_x;

// CAN0 socket描述符
int can0_socket = -1; 


// 电机状态
BYTE speed_zero[5] = {0x1d, 0x00, 0x00, 0x00, 0x00}; // 速度归零
BYTE motor_stop[1] = {0x02};                         // 电机关闭

// 获取速度
BYTE get_speed[1] = {0x06};
// 同一发送
BYTE motor_01[5] = {0x1d, 0x00, 0x00, 0x00, 0x00}; 
BYTE motor_02[5] = {0x1d, 0x00, 0x00, 0x00, 0x00}; 
BYTE motor_03[5] = {0x1d, 0x00, 0x00, 0x00, 0x00}; 
BYTE motor_04[5] = {0x1d, 0x00, 0x00, 0x00, 0x00}; 
BYTE motor_05[5] = {0x1d, 0x00, 0x00, 0x00, 0x00}; 


auto rt1_data = std_msgs::msg::Int32();
auto rt2_data = std_msgs::msg::Int32();
auto rt3_data = std_msgs::msg::Int32();
auto rt4_data = std_msgs::msg::Int32();

#endif