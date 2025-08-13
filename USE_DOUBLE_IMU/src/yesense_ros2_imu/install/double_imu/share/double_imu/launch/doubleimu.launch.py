#!/usr/bin/env python3
"""
Launch two yesense_node_publisher instances for
/dev/wheeltec_IMU_1 and /dev/wheeltec_IMU_2
"""

from launch import LaunchDescription
from launch_ros.actions import Node


def generate_launch_description():
    # ========== 公共默认值 ==========
    common = {
        "baud_rate": 460800,
        "driver_type": "ros_serial",
    }

    # ========== IMU1 参数 ==========
    param_imu1 = {
        **common,
        "serial_port": "/dev/wheeltec_IMU_1",
        "frame_id":   "gyro_link_imu1",
        "imu_topic_ros": "/imu1/data_raw",
        "imu_topic":     "/imu1/imu_data",
    }

    # ========== IMU2 参数 ==========
    param_imu2 = {
        **common,
        "serial_port": "/dev/wheeltec_IMU_2",
        "frame_id":   "gyro_link_imu2",
        "imu_topic_ros": "/imu2/data_raw",
        "imu_topic":     "/imu2/imu_data",
    }

    return LaunchDescription([
        Node(
            package='double_imu',
            executable='yesense_node_publisher',
            name='yesense_imu1',
            parameters=[param_imu1],
            output='screen'
        ),
        Node(
            package='double_imu',
            executable='yesense_node_publisher',
            name='yesense_imu2',
            parameters=[param_imu2],
            output='screen'
        ),
    ])