#ifndef MODE_SWITCHING_H
#define MODE_SWITCHING_H

#pragma once
#include <atomic>
#include <memory>

// 速度、推杆电压、方向在日常和模式状态下的切换
int mode_speed_status = 0;
// 模式辅助轮状态
int mode_wheel_status = 0; // 0:日常 1:模式
// 模式定速值
float mode_speed_value = 0.5;
// 模式推杆电压值
float mode_voltage_value = 0;
// 模式推杆方向
float mode_wheel_direction = 0; // 0:上升 1:下降

struct SharedState
{
    float v1 = 0;
    float v2 = 0;
    int mode = 0;
};

extern std::atomic<std::shared_ptr<const SharedState>> g_state_ptr;

// 距离传感器数据
float distance; // 距离
// mode 01 模式
void mode_01();
// mode 02 模式
void mode_02();
// mode 03 模式
void mode_03();
// mode 04 模式
void mode_04();

#endif