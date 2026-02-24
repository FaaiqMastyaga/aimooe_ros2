from launch import LaunchDescription
from launch_ros.actions import Node
import os

def generate_launch_description():
    return LaunchDescription([
        Node(
            package='aimooe_ros2',
            executable='aimooe_tracker_node',
            name='aimooe_tracker_node',
            output='screen',
            parameters=[{
                'ip_address': '192.168.31.10',
                # Change this path to exactly where your .aimtool files are located
                'tool_path': '/home/dian/Documents/aimooe_ros2_ws/src/aimooe_ros2/tools/',
                # The names of the tools as defined in the first line of the .aimtool files
                'tracked_tools': ['cal', 'tool', 'drb'] 
            }]
        )
    ])