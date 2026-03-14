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
    this->declare_parameter<std::vector<std::string>>("tools_to_track", {"tool", "cal", "drb", "ict"});
    this->declare_parameter<int>("min_match_points", 3);

    // --- Get Parameters ---
    tracking_frame_ = this->get_parameter("tracking_frame").as_string();
    tools_to_track_ = this->get_parameter("tools_to_track").as_string_array();
    min_match_points_ = this->get_parameter("min_match_points").as_int();

    // --- Initialize Broadcaster ---
    tf_broadcaster_ = std::make_unique<tf2_ros::TransformBroadcaster>(*this);

    // --- Initialize Publisher ---
    tool_info_pub_ = this->create_publisher<aimooe_msgs::msg::ToolArray>("/aimooe/tool_info", 10);

    // --- Initialize Services ---
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
    }
    else {
        current_state_.store(SystemState::TRACKING);
    }

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
        tracker_->disconnect();
    }
}

bool AimooeTrackerNode::initialize_tracker()
{
    RCLCPP_INFO(this->get_logger(), "Connecting to Aimooe device...");
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
                        if (initialize_tracker()) {
                            RCLCPP_INFO(this->get_logger(), "Camera Reconnected!");
                            current_state_.store(SystemState::TRACKING);
                        }
                        else {
                            RCLCPP_WARN(this->get_logger(), "Connection failed. Retrying in 2 seconds...");
                        }
                    }
                    break;
                }

                case SystemState::TRACKING: {
                    auto [code, valid_tools] = tracker_->find_valid_tools(tools_to_track_, min_match_points_);

                    if (code != aimooe_core::ReturnCode::OK) {
                        RCLCPP_WARN_THROTTLE(this->get_logger(), *this->get_clock(), 1000, "Error fetching tracker data. Code: %d", static_cast<int>(code));
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
                        tool_msg.translation_vector.x = tool.translation_vector[0];
                        tool_msg.translation_vector.y = tool.translation_vector[1];
                        tool_msg.translation_vector.z = tool.translation_vector[2];

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

                case SystemState::TOOL_CREATING: {
                    break;
                }
                
                case SystemState::SELF_CALIBRATING: {
                    break;
                }
                
                case SystemState::TIP_CALIBRATING: {
                    break;
                }
                
                case SystemState::PIVOT_CALIBRATING: {
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

void AimooeTrackerNode::handle_start_tool_create(const std::shared_ptr<aimooe_msgs::srv::ToolCreation::Request> request, std::shared_ptr<aimooe_msgs::srv::ToolCreation::Response> response)
{

}

void AimooeTrackerNode::handle_cancel_tool_create(const std::shared_ptr<std_srvs::srv::Trigger::Request> request, std::shared_ptr<std_srvs::srv::Trigger::Response> response)
{

}

void AimooeTrackerNode::handle_start_self_calib(const std::shared_ptr<aimooe_msgs::srv::SelfCalibration::Request> request, std::shared_ptr<aimooe_msgs::srv::SelfCalibration::Response> response)
{

}

void AimooeTrackerNode::handle_cancel_self_calib(const std::shared_ptr<std_srvs::srv::Trigger::Request> request, std::shared_ptr<std_srvs::srv::Trigger::Response> response)
{

}

void AimooeTrackerNode::handle_start_tip_calib(const std::shared_ptr<aimooe_msgs::srv::TipCalibration::Request> request, std::shared_ptr<aimooe_msgs::srv::TipCalibration::Response> response)
{

}

void AimooeTrackerNode::handle_cancel_tip_calib(const std::shared_ptr<std_srvs::srv::Trigger::Request> request, std::shared_ptr<std_srvs::srv::Trigger::Response> response)
{

}

void AimooeTrackerNode::handle_start_pivot_calib(const std::shared_ptr<aimooe_msgs::srv::TipPivot::Request> request, std::shared_ptr<aimooe_msgs::srv::TipPivot::Response> response)
{

}

void AimooeTrackerNode::handle_cancel_pivot_calib(const std::shared_ptr<std_srvs::srv::Trigger::Request> request, std::shared_ptr<std_srvs::srv::Trigger::Response> response)
{
    
}

} // namespace aimooe_ros2

// Standard main function
int main(int argc, char ** argv)
{
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
    
    rclcpp::shutdown();
    return 0;
}