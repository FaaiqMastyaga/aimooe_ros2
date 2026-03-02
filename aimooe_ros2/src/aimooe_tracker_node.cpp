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
    this->declare_parameter<double>("polling_rate_hz", 60.0);

    // --- Get Parameters ---
    tracking_frame_ = this->get_parameter("tracking_frame").as_string();
    tools_to_track_ = this->get_parameter("tools_to_track").as_string_array();
    min_match_points_ = this->get_parameter("min_match_points").as_int();
    double polling_rate_hz = this->get_parameter("polling_rate_hz").as_double();

    // --- Initialize Broadcaster ---
    tf_broadcaster_ = std::make_unique<tf2_ros::TransformBroadcaster>(*this);

    // --- Initialize Tracker ---
    tracker_ = std::make_unique<aimooe_core::AimooeTracker>();
    if (!initialize_tracker()) {
        RCLCPP_ERROR(this->get_logger(), "Failed to initialize Aimooe Tracker. Shutting down node.");
        rclcpp::shutdown();
        return;
    }

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

    // Tell the API we only want tracking INFO, not heavy image data for this timer loop
    tracker_->set_acquired_data(aimooe_core::AcquiredDataType::INFO);

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
}

} // namespace aimooe_ros2

// Standard main function
int main(int argc, char ** argv)
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<aimooe_ros2::AimooeTrackerNode>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}