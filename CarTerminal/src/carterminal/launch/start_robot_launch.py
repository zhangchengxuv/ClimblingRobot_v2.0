from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():
    camera_node = Node(package="carterminal",executable="camera_talker_node",name="camera_talker_node")
    sensor_node = Node(package="carterminal",executable="data_collect_node",name="data_collect_node")
    wheel_node = Node(package="carterminal",executable="wheel_talker_node",name="wheel_talker_node")
    
    
    ahrs_driver=Node(
                    package="fdilink_ahrs",
                    executable="ahrs_driver_node",
                    parameters=[{'if_debug_': False,
                        'serial_port_':'/dev/wheeltec_FDI_IMU_GNSS',
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
                    output="screen"
                    )
    imu_talker = Node(package="fdilink_ahrs",executable="imu_tf_node",name="imu_talker")

    # return LaunchDescription([wheel,arm,camera_1,camera_2,ahrs_driver,imu_talker,weigh_sensor,driver])
    return LaunchDescription([camera_node,sensor_node,wheel_node,ahrs_driver,imu_talker])
