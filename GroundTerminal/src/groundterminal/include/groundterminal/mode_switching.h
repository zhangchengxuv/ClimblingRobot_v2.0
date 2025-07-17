#ifndef MODE_SWITCHING_H
#define MODE_SWITCHING_H

#include <atomic>
#include <memory>

int mode = 0;
// 模式定速值
float mode_speed_value = 0.0;
// 模式推杆电压值
float mode_voltage_value = 0.0;
// 模式推杆方向
float mode_wheel_direction = 0.0; // 0:上升 1:下降

// mode 01 模式
void mode_01(float pitch, float roll, float distance, float force);
// mode 02 模式
void mode_02(float pitch, float roll, float distance, float force);
// mode 03 模式
void mode_03(float pitch, float roll, float distance, float force);
// mode 04 模式
void mode_04(float pitch, float roll, float distance, float force);

#endif