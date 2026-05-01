#include "aimooe_ros2/aimooe_tracker_node.hpp"
#include <ament_index_cpp/get_package_share_directory.hpp>

using namespace std::chrono_literals;

namespace aimooe_ros2
{

// Helper
bool is_camera_reachable(const std::string& ip_address) {
    // Ping the camera exactly 1 time (-c 1) with a strict 1-second timeout (-W 1)
    std::string command = "ping -c 1 -W 1 " + ip_address + " > /dev/null 2>&1";
    int result = std::system(command.c_str());
    return (result == 0);
}

AimooeTrackerNode::AimooeTrackerNode(const rclcpp::NodeOptions & options)
: Node("aimooe_tracker_node", options)
{
    // --- Declare Parameters ---
    this->declare_parameter<std::string>("tracking_frame", "aimooe_camera_link");
    this->declare_parameter<std::string>("connection_type", "");
    this->declare_parameter<std::string>("camera_ip", "");
    this->declare_parameter<std::string>("tools_dir", "");
    this->declare_parameter<std::vector<std::string>>("tools_to_track", std::vector<std::string>());
    this->declare_parameter<int>("min_match_points", 3);

    // --- Get Parameters ---
    tracking_frame_ = this->get_parameter("tracking_frame").as_string();
    connection_type_ = this->get_parameter("connection_type").as_string();
    camera_ip_ = this->get_parameter("camera_ip").as_string();
    tools_dir_ = this->get_parameter("tools_dir").as_string();
    tools_to_track_ = this->get_parameter("tools_to_track").as_string_array();
    min_match_points_ = this->get_parameter("min_match_points").as_int();

    // --- Initialize Broadcaster ---
    tf_broadcaster_ = std::make_unique<tf2_ros::TransformBroadcaster>(*this);

    // --- Initialize Publisher ---
    tool_info_pub_ = this->create_publisher<aimooe_msgs::msg::ToolArray>("/aimooe/tool_info", 10);

    // --- Initialize Services ---
    srv_connect_ = this->create_service<std_srvs::srv::Trigger>("aimooe/connect", std::bind(&AimooeTrackerNode::handle_connect, this, std::placeholders::_1, std::placeholders::_2));
    srv_disconnect_ = this->create_service<std_srvs::srv::Trigger>("aimooe/disconnect", std::bind(&AimooeTrackerNode::handle_disconnect, this, std::placeholders::_1, std::placeholders::_2));

    // --- Initialize Action Servers ---
    action_tool_create_ = rclcpp_action::create_server<aimooe_msgs::action::ToolCreation>(
        this, "aimooe/tool_create",
        std::bind(&AimooeTrackerNode::handle_tool_create_goal, this, std::placeholders::_1, std::placeholders::_2),
        std::bind(&AimooeTrackerNode::handle_tool_create_cancel, this, std::placeholders::_1),
        std::bind(&AimooeTrackerNode::handle_tool_create_accepted, this, std::placeholders::_1)
    );

    action_self_calib_ = rclcpp_action::create_server<aimooe_msgs::action::SelfCalibration>(
        this, "aimooe/self_calib",
        std::bind(&AimooeTrackerNode::handle_self_calib_goal, this, std::placeholders::_1, std::placeholders::_2),
        std::bind(&AimooeTrackerNode::handle_self_calib_cancel, this, std::placeholders::_1),
        std::bind(&AimooeTrackerNode::handle_self_calib_accepted, this, std::placeholders::_1)
    );

    action_tip_calib_ = rclcpp_action::create_server<aimooe_msgs::action::TipCalibration>(
        this, "aimooe/tip_calib",
        std::bind(&AimooeTrackerNode::handle_tip_calib_goal, this, std::placeholders::_1, std::placeholders::_2),
        std::bind(&AimooeTrackerNode::handle_tip_calib_cancel, this, std::placeholders::_1),
        std::bind(&AimooeTrackerNode::handle_tip_calib_accepted, this, std::placeholders::_1)
    );

    action_tip_pivot_ = rclcpp_action::create_server<aimooe_msgs::action::TipPivot>(
        this,
        "aimooe/tip_pivot",
        std::bind(&AimooeTrackerNode::handle_tip_pivot_goal, this, std::placeholders::_1, std::placeholders::_2),
        std::bind(&AimooeTrackerNode::handle_tip_pivot_cancel, this, std::placeholders::_1),
        std::bind(&AimooeTrackerNode::handle_tip_pivot_accepted, this, std::placeholders::_1)
    );

    // --- Initialize Tracker ---
    tracker_ = std::make_unique<aimooe_core::AimooeTracker>();
    current_state_.store(SystemState::CONNECTING);
    last_reconnect_time_ = this->now() - rclcpp::Duration(5, 0); // Force immediate connection attempt

    // --- Start Dedicated Hardware Thread ---
    running_.store(true);
    tracking_thread_ = std::thread(&AimooeTrackerNode::tracking_loop, this);

    RCLCPP_INFO(this->get_logger(), "Aimooe Tracker Node initialized. Hardware polling loop running.");
}

AimooeTrackerNode::~AimooeTrackerNode()
{
    // Safely shutdown the thread before destroying the tracker
    running_.store(false);
    if (tracking_thread_.joinable()) {
        tracking_thread_.join();
    }

    if (tracker_) {
        std::lock_guard<std::mutex> lock(api_mutex_);
        tracker_->disconnect();
        RCLCPP_INFO(this->get_logger(), "Camera safely disconnected. Goodbye!");
    }
}

bool AimooeTrackerNode::initialize_tracker()
{
    RCLCPP_INFO(this->get_logger(), "Connecting to Aimooe device via %s...", connection_type_.c_str());

    // Choose the connection interface based on the parameter
    aimooe_core::ConnectionInterface conn_interface;
    if (connection_type_ == "ETHERNET") {
        conn_interface = aimooe_core::ConnectionInterface::ETHERNET;
    }
    else if (connection_type_ == "USB") {
        conn_interface = aimooe_core::ConnectionInterface::USB;
    }
    else {
        RCLCPP_ERROR(this->get_logger(), "Invalid connection type specified: %s. Use 'ETHERNET' or 'USB'.", connection_type_.c_str());
        return false;
    }

    auto connect_code = tracker_->connect(conn_interface);
    
    if (connect_code != aimooe_core::ReturnCode::OK) {
        RCLCPP_ERROR(this->get_logger(), "Connection failed.");
        return false;
    }

    tracker_->set_acquired_data(aimooe_core::AcquiredDataType::NONE);

    // Check if the user provided a hardcoded path via parameters
    std::string tools_path = tools_dir_;

    // If not, fallback to the ROS 2 install space (Read-Only Warning!)
    if (tools_path.empty()) {
        try {
            std::string package_share_dir = ament_index_cpp::get_package_share_directory("aimooe_ros2");
            tools_path = package_share_dir + "/tools";
            RCLCPP_WARN(this->get_logger(), "Using ROS 2 install space for tools. Calibrations may be overwritten by colcon build!");
        } catch (const std::exception& e) {
            RCLCPP_ERROR(this->get_logger(), "Could not find package share directory: %s", e.what());
            return false;
        }
    }

    if (tracker_->set_tools_path(tools_path)) {
        RCLCPP_INFO(this->get_logger(), "Loaded tool definitions from: %s", tools_path.c_str());
    } else {
        RCLCPP_ERROR(this->get_logger(), "Failed to load tool definitions from: %s", tools_path.c_str());
        return false;
    }

    // Auto Detect Tools
    tools_to_track_.clear();

    try {
        for (const auto& entry : std::filesystem::directory_iterator(tools_path)) {
            if (entry.path().extension() == ".aimtool") {
                // Get just the name without extension
                tools_to_track_.push_back(entry.path().stem().string());
            }
        }
    } 
    catch (const std::filesystem::filesystem_error& e) {
        RCLCPP_WARN(this->get_logger(), "Could not scan tools directory for .tool files: %s", e.what());
    }

    RCLCPP_INFO(this->get_logger(), "Auto-detected %zu tools to track:", tools_to_track_.size());
    for (const auto& tool_name : tools_to_track_) {
        RCLCPP_INFO(this->get_logger(), "  - %s", tool_name.c_str());
    }

    return true;
}

// =========================================================================================
// High-Speed Hardware Thread
// =========================================================================================
void AimooeTrackerNode::tracking_loop() {
    // Pre-allocate the TF message to avoid heap allocation
    geometry_msgs::msg::TransformStamped t;
    
    while (rclcpp::ok() && running_.load()) {

        {
            // Lock the camera API. Services must wait until this unblocks.
            std::lock_guard<std::mutex> lock(api_mutex_);

            switch (current_state_.load()) {
                case SystemState::IDLE:
                    break;
                
                case SystemState::CONNECTING: {
                    if ((this->now() - last_reconnect_time_).seconds() > 2.0) {
                        last_reconnect_time_ = this->now();

                        // Only use the ping guard if we're trying to connect over Ethernet. USB connections don't have an IP to ping and will fail this check.
                        if (connection_type_ == "ETHERNET") {
                            if (!is_camera_reachable(camera_ip_)) {
                                RCLCPP_WARN(this->get_logger(), "Network unreachable. Waiting for cable...");
                                break;
                            }
                        }
                        
                        if (initialize_tracker()) {
                            RCLCPP_INFO(this->get_logger(), "Camera Reconnected!");
                            current_state_.store(SystemState::TRACKING);
                        }
                    }
                    break;
                }

                case SystemState::TRACKING: {
                    auto [code, valid_tools] = tracker_->find_valid_tools(tools_to_track_, min_match_points_);

                    if (code != aimooe_core::ReturnCode::OK) {
                        RCLCPP_WARN(this->get_logger(), "Tracker error. Attempting to reconnect...");
                        tracker_->disconnect();
                        current_state_.store(SystemState::CONNECTING);
                        last_reconnect_time_ = this->now();
                        break;
                    }

                    rclcpp::Time now = this->get_clock()->now();
                    
                    // Prepare custom message array
                    aimooe_msgs::msg::ToolArray tool_array_msg;
                    tool_array_msg.header.stamp = now;
                    tool_array_msg.header.frame_id = tracking_frame_;

                    for (const auto& tool : valid_tools) {
                        // Broadcast TF Frame
                        t.header.stamp = now;
                        t.header.frame_id = tracking_frame_;
                        t.child_frame_id = tool.tool_name;
                        t.transform.translation.x = tool.translation_vector[0] / 1000.0;
                        t.transform.translation.y = tool.translation_vector[1] / 1000.0;
                        t.transform.translation.z = tool.translation_vector[2] / 1000.0;
                        t.transform.rotation.x = tool.quaternion[0];
                        t.transform.rotation.y = tool.quaternion[1];
                        t.transform.rotation.z = tool.quaternion[2];
                        t.transform.rotation.w = tool.quaternion[3];
                        tf_broadcaster_->sendTransform(t);

                        // --- Pack ToolInfo data ---
                        aimooe_msgs::msg::Tool tool_msg;
                        tool_msg.tool_name = tool.tool_name;
                        tool_msg.tool_type = static_cast<int16_t>(tool.tool_type);
                        tool_msg.is_valid = tool.is_valid;
                        tool_msg.mean_abs_error = tool.mean_abs_error;
                        tool_msg.rms_error = tool.rms_error;

                        // Rotation Vector (Geometry Vector3)
                        tool_msg.rotation_vector.x = tool.rotation_vector[0];
                        tool_msg.rotation_vector.y = tool.rotation_vector[1];
                        tool_msg.rotation_vector.z = tool.rotation_vector[2];

                        // Quaternion (Geometry Quaternion)
                        tool_msg.quaternion.x = tool.quaternion[0];
                        tool_msg.quaternion.y = tool.quaternion[1];
                        tool_msg.quaternion.z = tool.quaternion[2];
                        tool_msg.quaternion.w = tool.quaternion[3];

                        // Translation Vector (Geometry Vector3)
                        tool_msg.translation_vector.x = tool.translation_vector[0] / 1000.0;
                        tool_msg.translation_vector.y = tool.translation_vector[1] / 1000.0;
                        tool_msg.translation_vector.z = tool.translation_vector[2] / 1000.0;

                        // Origin Coordinates (Geometry Point32)
                        tool_msg.origin_coordinates.x = tool.origin_coordinates[0];
                        tool_msg.origin_coordinates.y = tool.origin_coordinates[1];
                        tool_msg.origin_coordinates.z = tool.origin_coordinates[2];

                        // Rotation Matrix (Flattened 3x3 Matrix)
                        for (int i = 0; i < 3; i++) {
                            for (int j = 0; j < 3; j++) {
                                tool_msg.rotation_matrix[i*3 + j] = tool.rotation_matrix[i][j];
                            }
                        }

                        // Marker Points
                        for (size_t i = 0; i < tool.marker_points.size(); i += 3) {
                            if (i + 2 < tool.marker_points.size()) {
                                geometry_msgs::msg::Point32 pt;
                                pt.x = tool.marker_points[i];
                                pt.y = tool.marker_points[i+1];
                                pt.z = tool.marker_points[i+2];
                                tool_msg.marker_points.push_back(pt);
                            }
                        }

                        // Add to array
                        tool_array_msg.tools.push_back(tool_msg);
                    }
                    tool_info_pub_->publish(tool_array_msg);
                    break;
                }

                default:
                    break;
            }
        } // <--- MUTEX IS UNLOCKED HERE. Service callbacks can now execute!

        // Sleep for 1 ms to prevent 100% CPU usage and allow services to grab the lock
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

// =========================================================================================
// Service Callbacks
// =========================================================================================

void AimooeTrackerNode::handle_connect(const std::shared_ptr<std_srvs::srv::Trigger::Request> request, std::shared_ptr<std_srvs::srv::Trigger::Response> response)
{
    // Quick check without locking
    if (current_state_.load() == SystemState::TRACKING) {
        response->success = false;
        response->message = "Already connected and tracking.";
        return;
    }

    bool connection_successful = false;

    {
        // LOCK THE CAMERA API!
        std::lock_guard<std::mutex> lock(api_mutex_);
        
        // Try to connect immediately so we can give Slicer an honest answer
        connection_successful = initialize_tracker();
        
        if (connection_successful) {
            // Jump straight to tracking!
            current_state_.store(SystemState::TRACKING);
        } else {
            // If it fails, put the state machine into CONNECTING so the 
            // tracking_loop() takes over and automatically retries every 2 seconds.
            current_state_.store(SystemState::CONNECTING);
            last_reconnect_time_ = this->now(); 
        }
    } // UNLOCK!

    // Reply
    if (connection_successful) {
        RCLCPP_INFO(this->get_logger(), "Camera connected via service.");
        response->success = true;
        response->message = "Connected successfully!";
    } else {
        RCLCPP_WARN(this->get_logger(), "Manual connect failed. Background loop will keep trying.");
        response->success = false; // Note: We return false so the UI knows the camera isn't ready yet
        response->message = "Connection failed. Retrying in background...";
    }
}

void AimooeTrackerNode::handle_disconnect(const std::shared_ptr<std_srvs::srv::Trigger::Request> request, std::shared_ptr<std_srvs::srv::Trigger::Response> response)
{
    {
        std::lock_guard<std::mutex> lock(api_mutex_);   // LOCK THE CAMERA!
        tracker_->disconnect();
        current_state_.store(SystemState::IDLE);
    }
    RCLCPP_INFO(this->get_logger(), "Camera disconnected via service. Node is IDLE.");
    response->success = true;
    response->message = "Camera disconnected successfully.";
}

// =========================================================================================
// Tool Creation Action Callbacks
// =========================================================================================

rclcpp_action::GoalResponse AimooeTrackerNode::handle_tool_create_goal(const rclcpp_action::GoalUUID & uuid, std::shared_ptr<const aimooe_msgs::action::ToolCreation::Goal> goal) 
{
    if (current_state_.load() != SystemState::TRACKING) return rclcpp_action::GoalResponse::REJECT;
    return rclcpp_action::GoalResponse::ACCEPT_AND_EXECUTE;
}

rclcpp_action::CancelResponse AimooeTrackerNode::handle_tool_create_cancel(const std::shared_ptr<rclcpp_action::ServerGoalHandle<aimooe_msgs::action::ToolCreation>> goal_handle) 
{
    return rclcpp_action::CancelResponse::ACCEPT;
}

void AimooeTrackerNode::handle_tool_create_accepted(const std::shared_ptr<rclcpp_action::ServerGoalHandle<aimooe_msgs::action::ToolCreation>> goal_handle)
{
    std::thread{std::bind(&AimooeTrackerNode::execute_tool_create_action, this, std::placeholders::_1), goal_handle}.detach();
}

void AimooeTrackerNode::execute_tool_create_action(const std::shared_ptr<rclcpp_action::ServerGoalHandle<aimooe_msgs::action::ToolCreation>> goal_handle)
{
    const auto goal = goal_handle->get_goal();
    auto feedback = std::make_shared<aimooe_msgs::action::ToolCreation::Feedback>();
    auto result = std::make_shared<aimooe_msgs::action::ToolCreation::Result>();

    {
        std::lock_guard<std::mutex> lock(api_mutex_);
        current_state_.store(SystemState::TOOL_CREATING);
        if (tracker_->tool_create_init(goal->marker_count, goal->tool_name) != aimooe_core::ReturnCode::OK) {
            result->success = false; result->message = "Failed to init tool creation.";
            goal_handle->abort(result); current_state_.store(SystemState::TRACKING); return;
        }
    }

    rclcpp::Rate loop_rate(25); bool finished = false;
    while (rclcpp::ok() && !finished) {
        if (goal_handle->is_canceling()) {
            std::lock_guard<std::mutex> lock(api_mutex_);
            tracker_->tool_create_finish(false);
            result->success = false; result->message = "Canceled.";
            goal_handle->canceled(result); current_state_.store(SystemState::TRACKING); return;
        }
        {
            std::lock_guard<std::mutex> lock(api_mutex_);
            auto [code, progress] = tracker_->tool_create_process();
            if (code == aimooe_core::ReturnCode::OK) {
                feedback->progress_rate = progress.progress_rate;
                feedback->current_error = progress.error;
                feedback->invalid_marker_flag = progress.invalid_marker_flag;
                goal_handle->publish_feedback(feedback);
                if (progress.finished) { finished = true; result->final_error = progress.error; }
            }
        }
        loop_rate.sleep();
    }

    {
        std::lock_guard<std::mutex> lock(api_mutex_);
        if (tracker_->tool_create_finish(true) == aimooe_core::ReturnCode::OK) {
            result->success = true; result->message = "Saved!"; goal_handle->succeed(result);
        } else {
            result->success = false; result->message = "Failed to save."; goal_handle->abort(result);
        }
        current_state_.store(SystemState::TRACKING);
    }
}

// =========================================================================================
// Self Calibration Action Callbacks
// =========================================================================================

rclcpp_action::GoalResponse AimooeTrackerNode::handle_self_calib_goal(const rclcpp_action::GoalUUID & uuid, std::shared_ptr<const aimooe_msgs::action::SelfCalibration::Goal> goal)
{
    if (current_state_.load() != SystemState::TRACKING) return rclcpp_action::GoalResponse::REJECT;
    return rclcpp_action::GoalResponse::ACCEPT_AND_EXECUTE;
}

rclcpp_action::CancelResponse AimooeTrackerNode::handle_self_calib_cancel(const std::shared_ptr<rclcpp_action::ServerGoalHandle<aimooe_msgs::action::SelfCalibration>> goal_handle)
{
    return rclcpp_action::CancelResponse::ACCEPT;
}

void AimooeTrackerNode::handle_self_calib_accepted(const std::shared_ptr<rclcpp_action::ServerGoalHandle<aimooe_msgs::action::SelfCalibration>> goal_handle)
{
    std::thread{std::bind(&AimooeTrackerNode::execute_self_calib_action, this, std::placeholders::_1), goal_handle}.detach();
}

void AimooeTrackerNode::execute_self_calib_action(const std::shared_ptr<rclcpp_action::ServerGoalHandle<aimooe_msgs::action::SelfCalibration>> goal_handle)
{
    const auto goal = goal_handle->get_goal();
    auto feedback = std::make_shared<aimooe_msgs::action::SelfCalibration::Feedback>();
    auto result = std::make_shared<aimooe_msgs::action::SelfCalibration::Result>();

    {
        std::lock_guard<std::mutex> lock(api_mutex_);
        current_state_.store(SystemState::SELF_CALIBRATING);
        auto [code, count] = tracker_->tool_self_calibration_init(goal->tool_name);
        if (code != aimooe_core::ReturnCode::OK) {
            result->success = false; result->message = "Failed to init self calibration.";
            goal_handle->abort(result); current_state_.store(SystemState::TRACKING); return;
        }
    }

    rclcpp::Rate loop_rate(25); bool finished = false;
    while (rclcpp::ok() && !finished) {
        if (goal_handle->is_canceling()) {
            std::lock_guard<std::mutex> lock(api_mutex_);
            tracker_->tool_self_calibration_finish(false);
            result->success = false; result->message = "Canceled.";
            goal_handle->canceled(result); current_state_.store(SystemState::TRACKING); return;
        }
        {
            std::lock_guard<std::mutex> lock(api_mutex_);
            auto [code, progress] = tracker_->tool_self_calibration_process();
            if (code == aimooe_core::ReturnCode::OK) {
                feedback->total_marker_count = progress.total_marker_count;
                feedback->valid_calibration_count = progress.valid_calibration_count;
                feedback->current_match_error = progress.match_error;
                goal_handle->publish_feedback(feedback);
                if (progress.finished) { finished = true; result->final_match_error = progress.match_error; }
            }
        }
        loop_rate.sleep();
    }

    {
        std::lock_guard<std::mutex> lock(api_mutex_);
        if (tracker_->tool_self_calibration_finish(true) == aimooe_core::ReturnCode::OK) {
            result->success = true; result->message = "Saved!"; goal_handle->succeed(result);
        } else {
            result->success = false; result->message = "Failed to save."; goal_handle->abort(result);
        }
        current_state_.store(SystemState::TRACKING);
    }
}

// =========================================================================================
// Tip Calibration Action Callbacks
// =========================================================================================

rclcpp_action::GoalResponse AimooeTrackerNode::handle_tip_calib_goal(const rclcpp_action::GoalUUID & uuid, std::shared_ptr<const aimooe_msgs::action::TipCalibration::Goal> goal)
{
    if (current_state_.load() != SystemState::TRACKING) return rclcpp_action::GoalResponse::REJECT;
    return rclcpp_action::GoalResponse::ACCEPT_AND_EXECUTE;
}

rclcpp_action::CancelResponse AimooeTrackerNode::handle_tip_calib_cancel(const std::shared_ptr<rclcpp_action::ServerGoalHandle<aimooe_msgs::action::TipCalibration>> goal_handle)
{
    return rclcpp_action::CancelResponse::ACCEPT;
}

void AimooeTrackerNode::handle_tip_calib_accepted(const std::shared_ptr<rclcpp_action::ServerGoalHandle<aimooe_msgs::action::TipCalibration>> goal_handle)
{
    std::thread{std::bind(&AimooeTrackerNode::execute_tip_calib_action, this, std::placeholders::_1), goal_handle}.detach();
}

void AimooeTrackerNode::execute_tip_calib_action(const std::shared_ptr<rclcpp_action::ServerGoalHandle<aimooe_msgs::action::TipCalibration>> goal_handle)
{
    const auto goal = goal_handle->get_goal();
    auto feedback = std::make_shared<aimooe_msgs::action::TipCalibration::Feedback>();
    auto result = std::make_shared<aimooe_msgs::action::TipCalibration::Result>();

    {
        std::lock_guard<std::mutex> lock(api_mutex_);
        current_state_.store(SystemState::TIP_CALIBRATING);
        if (tracker_->tool_tip_calibration_init(goal->calibration_board_name, goal->tool_name) != aimooe_core::ReturnCode::OK) {
            result->success = false; result->message = "Failed to init tip calibration.";
            goal_handle->abort(result); current_state_.store(SystemState::TRACKING); return;
        }
    }

    rclcpp::Rate loop_rate(25); bool finished = false;
    while (rclcpp::ok() && !finished) {
        if (goal_handle->is_canceling()) {
            std::lock_guard<std::mutex> lock(api_mutex_);
            tracker_->tool_tip_calibration_finish(false);
            result->success = false; result->message = "Canceled.";
            goal_handle->canceled(result); current_state_.store(SystemState::TRACKING); return;
        }
        {
            std::lock_guard<std::mutex> lock(api_mutex_);
            auto [code, progress] = tracker_->tool_tip_calibration_process();
            if (code == aimooe_core::ReturnCode::OK) {
                feedback->progress_rate = progress.progress_rate;
                feedback->calibration_error = progress.calibration_error;
                feedback->rms_error = progress.rms_error;
                feedback->board_found = progress.board_found;
                feedback->tool_found = progress.tool_found;
                feedback->valid_calibration = progress.valid_calibration;
                goal_handle->publish_feedback(feedback);
                
                if (progress.finished) { 
                    finished = true; 
                    result->final_calibration_error = progress.calibration_error;
                    result->final_rms_error = progress.rms_error;
                }
            }
        }
        loop_rate.sleep();
    }

    {
        std::lock_guard<std::mutex> lock(api_mutex_);
        if (tracker_->tool_tip_calibration_finish(true) == aimooe_core::ReturnCode::OK) {
            result->success = true; result->message = "Saved!"; goal_handle->succeed(result);
        } else {
            result->success = false; result->message = "Failed to save."; goal_handle->abort(result);
        }
        current_state_.store(SystemState::TRACKING);
    }
}

// =========================================================================================
// Tip Pivot Action Callbacks
// =========================================================================================

rclcpp_action::GoalResponse AimooeTrackerNode::handle_tip_pivot_goal(const rclcpp_action::GoalUUID & uuid, std::shared_ptr<const aimooe_msgs::action::TipPivot::Goal> goal)
{
    RCLCPP_INFO(this->get_logger(), "Received goal request for Tip Pivot on tool: %s", goal->tool_name.c_str());

    // Safety: Only allow calibration to start if the camera is actively tracking
    if (current_state_.load() != SystemState::TRACKING) {
        RCLCPP_WARN(this->get_logger(), "Rejected Tip Pivot: Camera is not in TRACKING state.");
        return rclcpp_action::GoalResponse::REJECT;
    }
    return rclcpp_action::GoalResponse::ACCEPT_AND_EXECUTE;
}

rclcpp_action::CancelResponse AimooeTrackerNode::handle_tip_pivot_cancel(const std::shared_ptr<rclcpp_action::ServerGoalHandle<aimooe_msgs::action::TipPivot>> goal_handle)
{
    RCLCPP_INFO(this->get_logger(), "Received request to cancel Tip Pivot. Aborting safely...");
    return rclcpp_action::CancelResponse::ACCEPT;
}

void AimooeTrackerNode::handle_tip_pivot_accepted(const std::shared_ptr<rclcpp_action::ServerGoalHandle<aimooe_msgs::action::TipPivot>> goal_handle)
{
    // Spawn a dedicated thread for the calibration loop so we don't freeze the ROS executor
    std::thread{std::bind(&AimooeTrackerNode::execute_tip_pivot_action, this, std::placeholders::_1), goal_handle}.detach();
}

void AimooeTrackerNode::execute_tip_pivot_action(const std::shared_ptr<rclcpp_action::ServerGoalHandle<aimooe_msgs::action::TipPivot>> goal_handle)
{
    const auto goal = goal_handle->get_goal();
    auto feedback = std::make_shared<aimooe_msgs::action::TipPivot::Feedback>();
    auto result = std::make_shared<aimooe_msgs::action::TipPivot::Result>();

    // 1. Initialize the calibration in the SDK
    {
        std::lock_guard<std::mutex> lock(api_mutex_);
        current_state_.store(SystemState::PIVOT_CALIBRATING); // Pause normal TF tracking
        
        auto init_code = tracker_->tool_tip_pivot_init(goal->tool_name, goal->clear_tip_mid);
        if (init_code != aimooe_core::ReturnCode::OK) {
            result->success = false;
            result->message = "Failed to initialize pivot calibration in SDK.";
            goal_handle->abort(result);
            current_state_.store(SystemState::TRACKING); // Resume TF tracking
            return;
        }
    }

    // 2. The Feedback Loop (running at 25 Hz)
    rclcpp::Rate loop_rate(25); 
    bool finished = false;

    while (rclcpp::ok() && !finished) {
        
        // Did the Slicer UI click "Cancel"?
        if (goal_handle->is_canceling()) {
            std::lock_guard<std::mutex> lock(api_mutex_);
            tracker_->tool_tip_pivot_finish(false); // false = discard data
            result->success = false;
            result->message = "Calibration canceled by Slicer.";
            goal_handle->canceled(result);
            current_state_.store(SystemState::TRACKING); 
            return;
        }

        // Process one frame of calibration data
        {
            std::lock_guard<std::mutex> lock(api_mutex_);
            auto [code, progress] = tracker_->tool_tip_pivot_process();

            if (code == aimooe_core::ReturnCode::OK) {
                // Populate the Feedback message and blast it to Slicer
                feedback->progress_rate = progress.progress_rate;
                feedback->current_mean_error = progress.mean_error;
                feedback->tool_found = progress.tool_found;
                goal_handle->publish_feedback(feedback);

                if (progress.finished) {
                    finished = true;
                    result->final_mean_error = progress.mean_error;
                }
            } else if (code != aimooe_core::ReturnCode::STALE_DATA) {
                RCLCPP_WARN_THROTTLE(this->get_logger(), *this->get_clock(), 1000, "Hardware error during pivot process.");
            }
        }
        loop_rate.sleep();
    }

    // 3. Save the File and Finish
    {
        std::lock_guard<std::mutex> lock(api_mutex_);
        auto save_code = tracker_->tool_tip_pivot_finish(true); // true = save to .rom file
        
        if (save_code == aimooe_core::ReturnCode::OK) {
            result->success = true;
            result->message = "Pivot calibration completed and saved successfully!";
            goal_handle->succeed(result);
            RCLCPP_INFO(this->get_logger(), "Pivot success! Final Error: %f", result->final_mean_error);
        } else {
            result->success = false;
            result->message = "Calibration finished but failed to save to disk.";
            goal_handle->abort(result);
        }
        
        current_state_.store(SystemState::TRACKING); // Resume normal Slicer tracking!
    }
}

} // namespace aimooe_ros2

// Standard main function
int main(int argc, char ** argv)
{
    // Ignore broken pipe signals so cable unplugs don't crash the node
    signal(SIGPIPE, SIG_IGN);

    rclcpp::init(argc, argv);
    auto node = std::make_shared<aimooe_ros2::AimooeTrackerNode>();

    // Crucial for allowing Actions/Services to process concurrently
    rclcpp::executors::MultiThreadedExecutor executor;
    executor.add_node(node);

    try {
        executor.spin();
    } catch (const std::exception & e) {
        RCLCPP_FATAL(node->get_logger(), "Node terminated: %s", e.what());
    }
    
    node.reset();
    rclcpp::shutdown();
    return 0;
}