from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():
    camera_node = Node(package="carterminal",executable="camera_talker_node",name="camera_talker_node")
    sensor_node = Node(package="carterminal",executable="data_collect_node",name="data_collect_node")
    wheel_node = Node(package="carterminal",executable="wheel_talker_node",name="wheel_talker_node")
    sensor_collect_node = Node(package="carterminal",executable="sensor_collect_node",name="sensor_collect_node",
                               output="screen"
                               )
    # IMU数据
    ahrs_driver=Node(
                    package="fdilink_ahrs",
                    executable="ahrs_driver_node",
                    parameters=[{'if_debug_': False,
                        'serial_port_':'/dev/fdilink_ahrs',
                        'serial_baud_':921600,
                        'imu_topic':'/imu',
                        'imu_frame_id_':'gyro_link',
                        'mag_pose_2d_topic':'/mag_pose_2d',
                        'Magnetic_topic':'/magnetic',
                        'Euler_angles_topic':'/euler_angles',
                        'gps_topic':'/gps/fix',
                        'twist_topic':'/system_speed',
                        'NED_odom_topic':'/NED_odometry',
                        'device_type_':1}],
                    )
    imu_talker = Node(package="fdilink_ahrs",executable="imu_tf_node",name="imu_talker")
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


    # return LaunchDescription([wheel,arm,camera_1,camera_2,ahrs_driver,imu_talker,weigh_sensor,driver])
    return LaunchDescription([sensor_node,
                              wheel_node,
                              ahrs_driver,
                              imu_talker,
                              camera_node,
                              sensor_collect_node])
