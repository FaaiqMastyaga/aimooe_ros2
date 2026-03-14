#include "aimooe_ros2/aimooe_tracker_node.hpp"
#include <ament_index_cpp/get_package_share_directory.hpp>

using namespace std::chrono_literals;

namespace aimooe_ros2
{

AimooeTrackerNode::AimooeTrackerNode(const rclcpp::NodeOptions & options)
: Node("aimooe_tracker_node", options)
{
    // --- Declare Parameters ---
    this->declare_parameter<std::string>("tracking_frame", "aimooe_camera_link");
    this->declare_parameter<std::vector<std::string>>("tools_to_track", {"tool", "cal", "drb"});
    this->declare_parameter<int>("min_match_points", 3);
    this->declare_parameter<double>("polling_rate_hz", 300.0);

    // --- Get Parameters ---
    tracking_frame_ = this->get_parameter("tracking_frame").as_string();
    tools_to_track_ = this->get_parameter("tools_to_track").as_string_array();
    min_match_points_ = this->get_parameter("min_match_points").as_int();
    double polling_rate_hz = this->get_parameter("polling_rate_hz").as_double();

    // --- Initialize Broadcaster ---
    tf_broadcaster_ = std::make_unique<tf2_ros::TransformBroadcaster>(*this);

    // --- Initialize Server ---
    srv_connect_ = this->create_service<std_srvs::srv::Trigger>("aimooe/connect", std::bind(&AimooeTrackerNode::handle_connect, this, std::placeholders::_1, std::placeholders::_2));
    srv_disconnect_ = this->create_service<std_srvs::srv::Trigger>("aimooe/disconnect", std::bind(&AimooeTrackerNode::handle_disconnect, this, std::placeholders::_1, std::placeholders::_2));
    srv_start_tool_create_ = this->create_service<aimooe_msgs::srv::ToolCreation>("aimooe/start_tool_create", std::bind(&AimooeTrackerNode::handle_start_tool_create, this, std::placeholders::_1, std::placeholders::_2));
    srv_cancel_tool_create_ = this->create_service<std_srvs::srv::Trigger>("aimooe/cancel_tool_create", std::bind(&AimooeTrackerNode::handle_cancel_tool_create, this, std::placeholders::_1, std::placeholders::_2));
    srv_start_self_calib_ = this->create_service<aimooe_msgs::srv::SelfCalibration>("aimooe/start_self_calib", std::bind(&AimooeTrackerNode::handle_start_self_calib, this, std::placeholders::_1, std::placeholders::_2));
    srv_cancel_self_calib_ = this->create_service<std_srvs::srv::Trigger>("aimooe/cancel_self_calib", std::bind(&AimooeTrackerNode::handle_cancel_self_calib, this, std::placeholders::_1, std::placeholders::_2));
    srv_start_tip_calib_ = this->create_service<aimooe_msgs::srv::TipCalibration>("aimooe/start_tip_calib", std::bind(&AimooeTrackerNode::handle_start_tip_calib, this, std::placeholders::_1, std::placeholders::_2));
    srv_cancel_tip_calib_ = this->create_service<std_srvs::srv::Trigger>("aimooe/cancel_tip_calib", std::bind(&AimooeTrackerNode::handle_cancel_tip_calib, this, std::placeholders::_1, std::placeholders::_2));
    srv_start_pivot_calib_ = this->create_service<aimooe_msgs::srv::TipPivot>("aimooe/start_pivot_calib", std::bind(&AimooeTrackerNode::handle_start_pivot_calib, this, std::placeholders::_1, std::placeholders::_2));
    srv_cancel_pivot_calib_ = this->create_service<std_srvs::srv::Trigger>("aimooe/cancel_pivot_calib", std::bind(&AimooeTrackerNode::handle_cancel_pivot_calib, this, std::placeholders::_1, std::placeholders::_2));

    // --- Initialize Tracker ---
    tracker_ = std::make_unique<aimooe_core::AimooeTracker>();
    if (!initialize_tracker()) {
        RCLCPP_ERROR(this->get_logger(), "Failed to initialize Aimooe Tracker. Shutting down node.");
        rclcpp::shutdown();
        return;
    }
    current_state_ = SystemState::TRACKING;

    // --- Start Polling Timer ---
    auto timer_period = std::chrono::duration<double>(1.0 / polling_rate_hz);
    tracking_timer_ = this->create_wall_timer(
        std::chrono::duration_cast<std::chrono::nanoseconds>(timer_period),
        std::bind(&AimooeTrackerNode::timer_callback, this)
    );

    RCLCPP_INFO(this->get_logger(), "Aimooe Tracker Node initialized. Publishing TF at %.1f Hz", polling_rate_hz);
}

AimooeTrackerNode::~AimooeTrackerNode()
{
    if (tracker_) {
        tracker_->disconnect();
    }
}

bool AimooeTrackerNode::initialize_tracker()
{
    RCLCPP_INFO(this->get_logger(), "Connecting to Aimooe device via Ethernet...");
    auto connect_code = tracker_->connect(aimooe_core::ConnectionInterface::ETHERNET);
    
    if (connect_code != aimooe_core::ReturnCode::OK) {
        RCLCPP_ERROR(this->get_logger(), "Connection failed.");
        return false;
    }

    tracker_->set_acquired_data(aimooe_core::AcquiredDataType::NONE);

    // Dynamically locate the tools folder inside the ROS 2 install space
    std::string package_share_dir;
    try {
        package_share_dir = ament_index_cpp::get_package_share_directory("aimooe_ros2");
    } catch (const std::exception& e) {
        RCLCPP_ERROR(this->get_logger(), "Could not find package share directory: %s", e.what());
        return false;
    }

    std::string tools_path = package_share_dir + "/tools";
    if (tracker_->set_tools_path(tools_path)) {
        RCLCPP_INFO(this->get_logger(), "Loaded tool definitions from: %s", tools_path.c_str());
    } else {
        RCLCPP_ERROR(this->get_logger(), "Failed to load tool definitions from: %s", tools_path.c_str());
        return false;
    }

    return true;
}

void AimooeTrackerNode::timer_callback()
{
    switch (current_state_) {
        case SystemState::IDLE:
            break;
        case SystemState::CONNECTING: {
            if ((this->now() - last_reconnect_time_).seconds() > 2.0) {
                last_reconnect_time_ = this->now();
                if (initialize_tracker()) {
                    RCLCPP_INFO(this->get_logger(), "Camera Connected! Broadcasting TF...");
                    current_state_ = SystemState::TRACKING;
                }
                else {
                    RCLCPP_WARN(this->get_logger(), "Connection failed. Retrying in 2 seconds...");
                }
            }
            break;
        }
        case SystemState::TRACKING: {
            // Fetch tracked data from the pure C++ core library
            auto [code, valid_tools] = tracker_->find_valid_tools(tools_to_track_, min_match_points_);
        
            if (code != aimooe_core::ReturnCode::OK) {
                // If data is just stale, skip this frame. If it's a real error, log it.
                if (code != aimooe_core::ReturnCode::STALE_DATA) {
                    RCLCPP_WARN_THROTTLE(this->get_logger(), *this->get_clock(), 1000, 
                        "Error fetching tracker data. Code: %d", static_cast<int>(code));
                }
                return;
            }
        
            // Broadcast each detected tool to the TF2 tree
            rclcpp::Time now = this->get_clock()->now();
            for (const auto& tool : valid_tools) {
                geometry_msgs::msg::TransformStamped t;
        
                t.header.stamp = now;
                t.header.frame_id = tracking_frame_;
                t.child_frame_id = tool.tool_name;
        
                // Translation (Note: Ensure API units match ROS 2 standard, which is usually meters. 
                // If Aimooe returns millimeters, divide by 1000.0)
                t.transform.translation.x = tool.translation_vector[0] / 1000.0; 
                t.transform.translation.y = tool.translation_vector[1] / 1000.0;
                t.transform.translation.z = tool.translation_vector[2] / 1000.0;
        
                // Rotation (Quaternions)
                t.transform.rotation.x = tool.quaternion[0];
                t.transform.rotation.y = tool.quaternion[1];
                t.transform.rotation.z = tool.quaternion[2];
                t.transform.rotation.w = tool.quaternion[3];
        
                tf_broadcaster_->sendTransform(t);
            }
            break;
        }
        case SystemState::TOOL_CREATING: {
            auto [code, progress] = tracker_->tool_create_process();
            if (code == aimooe_core::ReturnCode::NOT_CONNECTED || code == aimooe_core::ReturnCode::READ_FAULT) {
                tracker_->disconnect();
                current_state_ = SystemState::CONNECTING;
            }
            if (code == aimooe_core::ReturnCode::OK) {
                auto msg = std_msgs::msg::String();

                if (progress.finished) {
                    RCLCPP_INFO(this->get_logger(), "Tool Creation Finished. Error: %.4f", progress.error);
                    tracker_->tool_create_finish(true);

                    current_state_ = SystemState::TRACKING;
                }
            }
            break;
        }
        case SystemState::SELF_CALIBRATING: {
            auto [code, progress] = tracker_->tool_self_calibration_process();
            if (code == aimooe_core::ReturnCode::NOT_CONNECTED || code == aimooe_core::ReturnCode::READ_FAULT) {
                tracker_->disconnect();
                current_state_ = SystemState::CONNECTING;
            }
            if (code == aimooe_core::ReturnCode::OK) {
                auto msg = std_msgs::msg::String();

                if (progress.finished) {
                    RCLCPP_INFO(this->get_logger(), "Self Calibration Finished. Match Error: %.4f", progress.match_error);
                    tracker_->tool_self_calibration_finish(true);

                    current_state_ = SystemState::TRACKING;
                }
            }
            break;        }
        case SystemState::TIP_CALIBRATING: {
            auto [code, progress] = tracker_->tool_tip_calibration_process();
            if (code == aimooe_core::ReturnCode::NOT_CONNECTED || code == aimooe_core::ReturnCode::READ_FAULT) {
                tracker_->disconnect();
                current_state_ = SystemState::CONNECTING;
            }
            if (code == aimooe_core::ReturnCode::OK) {
                auto msg = std_msgs::msg::String();

                if (progress.finished) {
                    RCLCPP_INFO(this->get_logger(), "Tip Calibration Finished. RMS Error: %.4f", progress.rms_error);
                    tracker_->tool_tip_calibration_finish(true);

                    current_state_ = SystemState::TRACKING;
                }
            }
            break;
        }
        case SystemState::PIVOT_CALIBRATING: {
            auto [code, progress] = tracker_->tool_tip_pivot_process();
            if (code == aimooe_core::ReturnCode::NOT_CONNECTED || code == aimooe_core::ReturnCode::READ_FAULT) {
                tracker_->disconnect();
                current_state_ = SystemState::CONNECTING;
            }
            if (code == aimooe_core::ReturnCode::OK) {
                auto msg = std_msgs::msg::String();

                if (progress.finished) {
                    RCLCPP_INFO(this->get_logger(), "Tip Calibration Finished. Mean Error: %.4f", progress.mean_error);
                    tracker_->tool_tip_pivot_finish(true);

                    current_state_ = SystemState::TRACKING;
                }
            }
            break;
        }
    }
}

// =========================================================================================
// Service Callbacks
// =========================================================================================

void AimooeTrackerNode::handle_connect(const std::shared_ptr<std_srvs::srv::Trigger::Request> request, std::shared_ptr<std_srvs::srv::Trigger::Response> response)
{
    if (current_state_ == SystemState::TRACKING) {
        response->success = false;
        response->message = "Already connected and tracking.";
        return;
    }
    current_state_ = SystemState::CONNECTING;
    last_reconnect_time_ = this->now() - rclcpp::Duration(5, 0);
    response->success = true;
    response->message = "Connection sequence started.";
}

void AimooeTrackerNode::handle_disconnect(const std::shared_ptr<std_srvs::srv::Trigger::Request> request, std::shared_ptr<std_srvs::srv::Trigger::Response> response)
{
    current_state_ = SystemState::IDLE;
    tracker_->disconnect();
    RCLCPP_INFO(this->get_logger(), "Camera disconnected via service. Node is IDLE.");
    response->success = true;
    response->message = "Camera disconnected successfully.";
}

void AimooeTrackerNode::handle_start_tool_create(const std::shared_ptr<aimooe_msgs::srv::ToolCreation::Request> request, std::shared_ptr<aimooe_msgs::srv::ToolCreation::Response> response)
{
    current_state_ = SystemState::TOOL_CREATING;

    int marker_count = request->marker_count;
    std::string tool_name = request->tool_name;
    aimooe_core::ReturnCode code = tracker_->tool_create_init(marker_count, tool_name);
    
    if (code != aimooe_core::ReturnCode::OK) {
        current_state_ = SystemState::TRACKING;
        response->success = false;
        response->message = "Failed to initialize tool creation";
        RCLCPP_ERROR(this->get_logger(), "Failed to initialize tool creation: %d", (int)code);
        return;
    }
    response->success = true;
    response->message = "Tool creation started";
    RCLCPP_INFO(this->get_logger(), "Initialized tool creation: %s (%d markers)", tool_name.c_str(), (int)marker_count);
    RCLCPP_INFO(this->get_logger(), "Hold tool steady in view");
}

void AimooeTrackerNode::handle_cancel_tool_create(const std::shared_ptr<std_srvs::srv::Trigger::Request> request, std::shared_ptr<std_srvs::srv::Trigger::Response> response)
{
    tracker_->tool_create_finish(false);
    current_state_ = SystemState::TRACKING;
    response->success = true;
    response->message = "Tool creation canceled";
}

void AimooeTrackerNode::handle_start_self_calib(const std::shared_ptr<aimooe_msgs::srv::SelfCalibration::Request> request, std::shared_ptr<aimooe_msgs::srv::SelfCalibration::Response> response)
{
    current_state_ = SystemState::SELF_CALIBRATING;

    std::string tool_name = request->tool_name;
    auto [code, marker_count] = tracker_->tool_self_calibration_init(tool_name);
    
    if (code != aimooe_core::ReturnCode::OK) {
        current_state_ = SystemState::TRACKING;
        response->success = false;
        response->message = "Failed to initialize self calibration";
        RCLCPP_ERROR(this->get_logger(), "Failed to initialize tool self calibration: %d", (int)code);
        return;
    }
    response->success = true;
    response->message = "Self calibration started";
    RCLCPP_INFO(this->get_logger(), "Initialized self calibration: %s (%d markers)", tool_name.c_str(), (int)marker_count);
    RCLCPP_INFO(this->get_logger(), "Hold tool steady in view");
}

void AimooeTrackerNode::handle_cancel_self_calib(const std::shared_ptr<std_srvs::srv::Trigger::Request> request, std::shared_ptr<std_srvs::srv::Trigger::Response> response)
{
    tracker_->tool_self_calibration_finish(false);
    current_state_ = SystemState::TRACKING;
    response->success = true;
    response->message = "Self calibration canceled";}

void AimooeTrackerNode::handle_start_tip_calib(const std::shared_ptr<aimooe_msgs::srv::TipCalibration::Request> request, std::shared_ptr<aimooe_msgs::srv::TipCalibration::Response> response)
{
    current_state_ = SystemState::TIP_CALIBRATING;

    std::string calibration_board_name = request->calibration_board_name;
    std::string tool_name = request->tool_name;
    aimooe_core::ReturnCode code = tracker_->tool_tip_calibration_init(calibration_board_name, tool_name);
    
    if (code != aimooe_core::ReturnCode::OK) {
        current_state_ = SystemState::TRACKING;
        response->success = false;
        response->message = "Failed to initialize tip calibration";
        RCLCPP_ERROR(this->get_logger(), "Failed to initialize tool tip calibration: %d", (int)code);
        return;
    }
    response->success = true;
    response->message = "Tip calibration started";
    RCLCPP_INFO(this->get_logger(), "Initialized tool tip calibration: board=%s, tool=%s", calibration_board_name.c_str(), tool_name.c_str());
    RCLCPP_INFO(this->get_logger(), "Move tool and board in view");
}

void AimooeTrackerNode::handle_cancel_tip_calib(const std::shared_ptr<std_srvs::srv::Trigger::Request> request, std::shared_ptr<std_srvs::srv::Trigger::Response> response)
{
    tracker_->tool_tip_calibration_finish(false);
    current_state_ = SystemState::TRACKING;
    response->success = true;
    response->message = "Tip calibration canceled";
}

void AimooeTrackerNode::handle_start_pivot_calib(const std::shared_ptr<aimooe_msgs::srv::TipPivot::Request> request, std::shared_ptr<aimooe_msgs::srv::TipPivot::Response> response)
{
    current_state_ = SystemState::PIVOT_CALIBRATING;

    std::string tool_name = request->tool_name;
    bool clear_tip_mid = request->clear_tip_mid;
    aimooe_core::ReturnCode code = tracker_->tool_tip_pivot_init(tool_name, clear_tip_mid);
    
    if (code != aimooe_core::ReturnCode::OK) {
        current_state_ = SystemState::TRACKING;
        response->success = false;
        response->message = "Failed to initialize pivot calibration";
        RCLCPP_ERROR(this->get_logger(), "Failed to initialize pivot calibration: %d", (int)code);
        return;
    }
    response->success = true;
    response->message = "Pivot calibration started";
    RCLCPP_INFO(this->get_logger(), "Initialized pivot calibration: tool=%s", tool_name.c_str());
    RCLCPP_INFO(this->get_logger(), "Rotate tool tip around fixed point");
}

void AimooeTrackerNode::handle_cancel_pivot_calib(const std::shared_ptr<std_srvs::srv::Trigger::Request> request, std::shared_ptr<std_srvs::srv::Trigger::Response> response)
{
    tracker_->tool_tip_pivot_finish(false);
    current_state_ = SystemState::TRACKING;
    response->success = true;
    response->message = "Pivot calibration canceled";
}

} // namespace aimooe_ros2

// Standard main function
int main(int argc, char ** argv)
{
    rclcpp::init(argc, argv);
    try {
        auto node = std::make_shared<aimooe_ros2::AimooeTrackerNode>();
        rclcpp::spin(node);
    } catch (const std::exception & e) {
        RCLCPP_FATAL(rclcpp::get_logger("aimooe_tracker_node"), "Node terminated: %s", e.what());
    }
    rclcpp::shutdown();
    return 0;
}