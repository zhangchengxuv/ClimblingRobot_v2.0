#!/usr/bin/env python3
from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():
  # LDROBOT LiDAR publisher node
  ldlidar_node = Node(
      package='ldlidar',
      executable='stp23lnet_ros2node',
      name='STP23L',
      # output='screen',
      parameters=[
        {'product_name': 'LDLiDAR_STP23L'},
        {'topic_name': 'laser'},
        {'frame_id': 'base_laser'},
        {'device_ip': '192.168.1.200'},
        {'device_port': 2368},
        {'difop_ip': '192.168.1.102'},
        {'difop_port': 2369},
      ]
  )

  # base_link to base_laser tf node
  base_link_to_laser_tf_node = Node(
    package='tf2_ros',
    executable='static_transform_publisher',
    name='base_link_to_base_laser_stp23l',
    arguments=['0','0','0.18','0','0','0','base_link','base_laser']
  )


  # Define LaunchDescription variable
  ld = LaunchDescription()

  ld.add_action(ldlidar_node)
  ld.add_action(base_link_to_laser_tf_node)

  return ld