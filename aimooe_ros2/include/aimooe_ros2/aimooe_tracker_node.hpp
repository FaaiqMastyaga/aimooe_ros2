#ifndef AIMOOE_ROS2__AIMOOE_TRACKER_NODE_HPP_
#define AIMOOE_ROS2__AIMOOE_TRACKER_NODE_HPP_

#include <rclcpp/rclcpp.hpp>
#include <geometry_msgs/msg/transform_stamped.hpp>
#include <tf2_ros/transform_broadcaster.h>

#include <memory>
#include <string>
#include <vector>

#include "aimooe_core/aimooe_tracker.hpp"

namespace aimooe_ros2
{

class AimooeTrackerNode : public rclcpp::Node
{
public:
    AimooeTrackerNode(const rclcpp::NodeOptions & options = rclcpp::NodeOptions());
    ~AimooeTrackerNode() override;

private:
    void timer_callback();
    bool initialize_tracker();

    // Hardware Wrapper
    std::unique_ptr<aimooe_core::AimooeTracker> tracker_;
    
    // ROS 2 Timers & Broadcasters
    rclcpp::TimerBase::SharedPtr tracking_timer_;
    std::unique_ptr<tf2_ros::TransformBroadcaster> tf_broadcaster_;

    // Parameters
    std::string tracking_frame_;
    std::vector<std::string> tools_to_track_;
    int min_match_points_;
};

} // namespace aimooe_ros2

#endif // AIMOOE_ROS2__AIMOOE_TRACKER_NODE_HPP_