#ifndef AIMOOE_ROS2__AIMOOE_TRACKER_NODE_HPP_
#define AIMOOE_ROS2__AIMOOE_TRACKER_NODE_HPP_

#include <rclcpp/rclcpp.hpp>
#include <geometry_msgs/msg/transform_stamped.hpp>
#include <tf2_ros/transform_broadcaster.h>
#include <std_srvs/srv/trigger.hpp>

// Custom Messages
#include "aimooe_msgs/msg/tool.hpp"
#include "aimooe_msgs/msg/tool_array.hpp"

// Custom Services
#include "aimooe_msgs/srv/tool_creation.hpp"
#include "aimooe_msgs/srv/self_calibration.hpp"
#include "aimooe_msgs/srv/tip_calibration.hpp"
#include "aimooe_msgs/srv/tip_pivot.hpp"

#include "aimooe_core/aimooe_tracker.hpp"

#include <memory>
#include <string>
#include <vector>

#include <thread>
#include <mutex>
#include <atomic>

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
    bool initialize_tracker();

    // High Speed Loop
    void tracking_loop();

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

    // Core Components
    std::unique_ptr<aimooe_core::AimooeTracker> tracker_;
    std::unique_ptr<tf2_ros::TransformBroadcaster> tf_broadcaster_;
    
    // Parameters
    std::string tracking_frame_;
    std::vector<std::string> tools_to_track_;
    int min_match_points_;

    // State Variables
    rclcpp::Time last_reconnect_time_;

    // Multithreading Variables
    std::atomic<SystemState> current_state_{SystemState::IDLE};
    std::atomic<bool> running_{true};   // Used to safely stop the thread on shutdown
    std::mutex api_mutex_;
    std::thread tracking_thread_;

    rclcpp::Publisher<aimooe_msgs::msg::ToolArray>::SharedPtr tool_info_pub_;

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
};

} // namespace aimooe_ros2

#endif // AIMOOE_ROS2__AIMOOE_TRACKER_NODE_HPP_