from launch import LaunchDescription
from launch_ros.actions import Node
from ament_index_python.packages import get_package_share_directory
import os

def generate_launch_description():
    config = os.path.join(
        get_package_share_directory('double_imu'),
        'config',
        'yesense_config.yaml',
    ),

    return LaunchDescription([
         Node(
            package='double_imu',
            executable='yesense_node_publisher',
            name='yesense_pub',
            parameters=[config],
            output='screen',
            ),
        ])
