import os
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():
    config_dir = os.path.join(
        get_package_share_directory('aimooe_ros2'),
        'config',
        'tracker_params.yaml'
    )

    tracker_node = Node(
        package='aimooe_ros2',
        executable='aimooe_tracker_node',
        name='aimooe_tracker_node',
        parameters=[config_dir],
        output='screen',
        respawn=True,
        respawn_delay=2.0
    )

    return LaunchDescription([
        tracker_node
    ])