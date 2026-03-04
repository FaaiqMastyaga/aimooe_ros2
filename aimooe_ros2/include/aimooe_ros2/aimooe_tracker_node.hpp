#ifndef AIMOOE_ROS2__AIMOOE_TRACKER_NODE_HPP_
#define AIMOOE_ROS2__AIMOOE_TRACKER_NODE_HPP_

#include <rclcpp/rclcpp.hpp>
#include <geometry_msgs/msg/transform_stamped.hpp>
#include <tf2_ros/transform_broadcaster.h>
#include <std_srvs/srv/trigger.hpp>

#include "aimooe_msgs/srv/tool_creation.hpp"
#include "aimooe_msgs/srv/self_calibration.hpp"
#include "aimooe_msgs/srv/tip_calibration.hpp"
#include "aimooe_msgs/srv/tip_pivot.hpp"

#include <memory>
#include <string>
#include <vector>

#include "aimooe_core/aimooe_tracker.hpp"

namespace aimooe_ros2
{

enum class SystemState {
    IDLE,               // Camera is disconnected intentionally
    CONNECTING,         // Trying to establish.recover connection
    TRACKING,           // Broadcasting tool trasnform
    TOOL_CREATING,      // Running tool registration
    SELF_CALIBRATING,   // Running self calibration
    TIP_CALIBRATING,    // Running tip calibration
    PIVOT_CALIBRATING,  // Running pivot calibration
};

class AimooeTrackerNode : public rclcpp::Node
{
public:
    AimooeTrackerNode(const rclcpp::NodeOptions & options = rclcpp::NodeOptions());
    ~AimooeTrackerNode() override;

private:
    void timer_callback();
    bool initialize_tracker();

    // Service Callbacks
    void handle_connect(const std::shared_ptr<std_srvs::srv::Trigger::Request> request, std::shared_ptr<std_srvs::srv::Trigger::Response> response);
    void handle_disconnect(const std::shared_ptr<std_srvs::srv::Trigger::Request> request, std::shared_ptr<std_srvs::srv::Trigger::Response> response);

    void handle_start_tool_create(const std::shared_ptr<aimooe_msgs::srv::ToolCreation::Request> request, std::shared_ptr<aimooe_msgs::srv::ToolCreation::Response> response);
    void handle_cancel_tool_create(const std::shared_ptr<std_srvs::srv::Trigger::Request> request, std::shared_ptr<std_srvs::srv::Trigger::Response> response);

    void handle_start_self_calib(const std::shared_ptr<aimooe_msgs::srv::SelfCalibration::Request> request, std::shared_ptr<aimooe_msgs::srv::SelfCalibration::Response> response);
    void handle_cancel_self_calib(const std::shared_ptr<std_srvs::srv::Trigger::Request> request, std::shared_ptr<std_srvs::srv::Trigger::Response> response);

    void handle_start_tip_calib(const std::shared_ptr<aimooe_msgs::srv::TipCalibration::Request> request, std::shared_ptr<aimooe_msgs::srv::TipCalibration::Response> response);
    void handle_cancel_tip_calib(const std::shared_ptr<std_srvs::srv::Trigger::Request> request, std::shared_ptr<std_srvs::srv::Trigger::Response> response);

    void handle_start_pivot_calib(const std::shared_ptr<aimooe_msgs::srv::TipPivot::Request> request, std::shared_ptr<aimooe_msgs::srv::TipPivot::Response> response);
    void handle_cancel_pivot_calib(const std::shared_ptr<std_srvs::srv::Trigger::Request> request, std::shared_ptr<std_srvs::srv::Trigger::Response> response);

    // State Variables
    SystemState current_state_;
    rclcpp::Time last_reconnect_time_;

    // Hardware Wrapper
    std::unique_ptr<aimooe_core::AimooeTracker> tracker_;
    
    // ROS 2 Inrastructure
    rclcpp::TimerBase::SharedPtr tracking_timer_;
    std::unique_ptr<tf2_ros::TransformBroadcaster> tf_broadcaster_;

    rclcpp::Service<std_srvs::srv::Trigger>::SharedPtr srv_connect_;
    rclcpp::Service<std_srvs::srv::Trigger>::SharedPtr srv_disconnect_;

    rclcpp::Service<aimooe_msgs::srv::ToolCreation>::SharedPtr srv_start_tool_create_;
    rclcpp::Service<std_srvs::srv::Trigger>::SharedPtr srv_cancel_tool_create_;

    rclcpp::Service<aimooe_msgs::srv::SelfCalibration>::SharedPtr srv_start_self_calib_;
    rclcpp::Service<std_srvs::srv::Trigger>::SharedPtr srv_cancel_self_calib_;

    rclcpp::Service<aimooe_msgs::srv::TipCalibration>::SharedPtr srv_start_tip_calib_;
    rclcpp::Service<std_srvs::srv::Trigger>::SharedPtr srv_cancel_tip_calib_;

    rclcpp::Service<aimooe_msgs::srv::TipPivot>::SharedPtr srv_start_pivot_calib_;
    rclcpp::Service<std_srvs::srv::Trigger>::SharedPtr srv_cancel_pivot_calib_;

    // Parameters
    std::string tracking_frame_;
    std::vector<std::string> tools_to_track_;
    int min_match_points_;
};

} // namespace aimooe_ros2

#endif // AIMOOE_ROS2__AIMOOE_TRACKER_NODE_HPP_