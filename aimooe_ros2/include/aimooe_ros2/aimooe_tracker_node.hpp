#ifndef AIMOOE_ROS2__AIMOOE_TRACKER_NODE_HPP_
#define AIMOOE_ROS2__AIMOOE_TRACKER_NODE_HPP_

#include <rclcpp/rclcpp.hpp>
#include <geometry_msgs/msg/transform_stamped.hpp>
#include <tf2_ros/transform_broadcaster.h>
#include <std_srvs/srv/trigger.hpp>
#include <rclcpp_action/rclcpp_action.hpp>

// Custom Messages
#include "aimooe_msgs/msg/tool.hpp"
#include "aimooe_msgs/msg/tool_array.hpp"

// Custom Services
#include "aimooe_msgs/action/tool_creation.hpp"
#include "aimooe_msgs/action/self_calibration.hpp"
#include "aimooe_msgs/action/tip_calibration.hpp"
#include "aimooe_msgs/action/tip_pivot.hpp"

#include "aimooe_core/aimooe_tracker.hpp"

#include <memory>
#include <string>
#include <vector>

#include <thread>
#include <mutex>
#include <atomic>
#include <csignal>
#include <cstdlib>

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

    // Action Callbacks
    // Tool Creation
    rclcpp_action::GoalResponse handle_tool_create_goal(const rclcpp_action::GoalUUID & uuid, std::shared_ptr<const aimooe_msgs::action::ToolCreation::Goal> goal);
    rclcpp_action::CancelResponse handle_tool_create_cancel(const std::shared_ptr<rclcpp_action::ServerGoalHandle<aimooe_msgs::action::ToolCreation>> goal_handle);
    void handle_tool_create_accepted(const std::shared_ptr<rclcpp_action::ServerGoalHandle<aimooe_msgs::action::ToolCreation>> goal_handle);
    void execute_tool_create_action(const std::shared_ptr<rclcpp_action::ServerGoalHandle<aimooe_msgs::action::ToolCreation>> goal_handle);

    // Self Calibration
    rclcpp_action::GoalResponse handle_self_calib_goal(const rclcpp_action::GoalUUID & uuid, std::shared_ptr<const aimooe_msgs::action::SelfCalibration::Goal> goal);
    rclcpp_action::CancelResponse handle_self_calib_cancel(const std::shared_ptr<rclcpp_action::ServerGoalHandle<aimooe_msgs::action::SelfCalibration>> goal_handle);
    void handle_self_calib_accepted(const std::shared_ptr<rclcpp_action::ServerGoalHandle<aimooe_msgs::action::SelfCalibration>> goal_handle);
    void execute_self_calib_action(const std::shared_ptr<rclcpp_action::ServerGoalHandle<aimooe_msgs::action::SelfCalibration>> goal_handle);

    // Tip Calibration
    rclcpp_action::GoalResponse handle_tip_calib_goal(const rclcpp_action::GoalUUID & uuid, std::shared_ptr<const aimooe_msgs::action::TipCalibration::Goal> goal);
    rclcpp_action::CancelResponse handle_tip_calib_cancel(const std::shared_ptr<rclcpp_action::ServerGoalHandle<aimooe_msgs::action::TipCalibration>> goal_handle);
    void handle_tip_calib_accepted(const std::shared_ptr<rclcpp_action::ServerGoalHandle<aimooe_msgs::action::TipCalibration>> goal_handle);
    void execute_tip_calib_action(const std::shared_ptr<rclcpp_action::ServerGoalHandle<aimooe_msgs::action::TipCalibration>> goal_handle);

    // Tip Pivot
    rclcpp_action::GoalResponse handle_tip_pivot_goal(const rclcpp_action::GoalUUID & uuid, std::shared_ptr<const aimooe_msgs::action::TipPivot::Goal> goal);
    rclcpp_action::CancelResponse handle_tip_pivot_cancel(const std::shared_ptr<rclcpp_action::ServerGoalHandle<aimooe_msgs::action::TipPivot>> goal_handle);
    void handle_tip_pivot_accepted(const std::shared_ptr<rclcpp_action::ServerGoalHandle<aimooe_msgs::action::TipPivot>> goal_handle);
    void execute_tip_pivot_action(const std::shared_ptr<rclcpp_action::ServerGoalHandle<aimooe_msgs::action::TipPivot>> goal_handle);

    // Core Components
    std::unique_ptr<aimooe_core::AimooeTracker> tracker_;
    std::unique_ptr<tf2_ros::TransformBroadcaster> tf_broadcaster_;
    
    // Parameters
    std::string tracking_frame_;
    std::string camera_ip_;
    std::string tools_dir_;
    std::vector<std::string> tools_to_track_;
    int min_match_points_;

    // State Variables
    rclcpp::Time last_reconnect_time_;

    // Multithreading Variables
    std::atomic<SystemState> current_state_{SystemState::IDLE};
    std::atomic<bool> running_{true};   // Used to safely stop the thread on shutdown
    std::mutex api_mutex_;
    std::thread tracking_thread_;

    // --- Publishers 
    rclcpp::Publisher<aimooe_msgs::msg::ToolArray>::SharedPtr tool_info_pub_;

    // --- Services ---
    rclcpp::Service<std_srvs::srv::Trigger>::SharedPtr srv_connect_;
    rclcpp::Service<std_srvs::srv::Trigger>::SharedPtr srv_disconnect_;

    // --- Action Servers ---
    rclcpp_action::Server<aimooe_msgs::action::ToolCreation>::SharedPtr action_tool_create_;
    rclcpp_action::Server<aimooe_msgs::action::SelfCalibration>::SharedPtr action_self_calib_;
    rclcpp_action::Server<aimooe_msgs::action::TipCalibration>::SharedPtr action_tip_calib_;
    rclcpp_action::Server<aimooe_msgs::action::TipPivot>::SharedPtr action_tip_pivot_;
};

} // namespace aimooe_ros2

#endif // AIMOOE_ROS2__AIMOOE_TRACKER_NODE_HPP_