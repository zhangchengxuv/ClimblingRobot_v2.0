from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():
    can_node = Node(package="groundterminal",executable="car_candriver",name="car_control_node")
    remote_node = Node(package="groundterminal",executable="remote_control",name="remote_control_node")
    mode_switching = Node(package="groundterminal",executable="mode_switching",name="mode_switching")
    
    
    return LaunchDescription([can_node,remote_node,mode_switching])
