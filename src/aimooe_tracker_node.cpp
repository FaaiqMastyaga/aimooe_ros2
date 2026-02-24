#include <chrono>
#include <memory>
#include <string>
#include <vector>
#include <map>

#include "rclcpp/rclcpp.hpp"
#include "tf2_ros/transform_broadcaster.h"
#include "geometry_msgs/msg/transform_stamped.hpp"

// Aimooe API Headers
#include "aimooe_ros2/AimPositionAPI.h"
#include "aimooe_ros2/AimPositionDef.h"

using namespace std::chrono_literals;

class AimooeTrackerNode : public rclcpp::Node
{
public:
    AimooeTrackerNode() : Node("aimooe_tracker_node"), aim_handle_(nullptr)
    {
        RCLCPP_INFO(this->get_logger(), "Starting Aimooe Tracker Node...");

        // 1. Declare ROS2 Parameters
        // This allows you to change the IP and tools from the terminal or a launch file
        this->declare_parameter<std::string>("ip_address", "192.168.31.10");
        this->declare_parameter<std::string>("tool_path", "~/Documents/aimooe_ros2_ws/src/tools");
        this->declare_parameter<std::vector<std::string>>("tracked_tools", {"tool4", "CTS-B4B0-006-1"});
        
        std::string ip_address = this->get_parameter("ip_address").as_string();
        tool_path_ = this->get_parameter("tool_path").as_string();
        tracked_tools_ = this->get_parameter("tracked_tools").as_string_array();

        // 2. Initialize the Aimooe Hardware API
        Aim_API_Initial(aim_handle_);

        // Parse IP address string into integers
        int ip1, ip2, ip3, ip4;
        if (sscanf(ip_address.c_str(), "%d.%d.%d.%d", &ip1, &ip2, &ip3, &ip4) == 4) {
            Aim_SetEthernetConnectIP(aim_handle_, ip1, ip2, ip3, ip4);
        } else {
            RCLCPP_ERROR(this->get_logger(), "Invalid IP format. Use X.X.X.X");
            return;
        }

        // Connect to the device via Ethernet
        T_AIMPOS_DATAPARA pos_para;
        E_ReturnValue rlt = Aim_ConnectDevice(aim_handle_, I_ETHERNET, pos_para);
        
        if (rlt != AIMOOE_OK) {
            RCLCPP_ERROR(this->get_logger(), "Failed to connect to Aimooe tracker at %s", ip_address.c_str());
            return;
        }

        RCLCPP_INFO(this->get_logger(), "Connected successfully! Initializing data stream...");
        
        // Instruct hardware to only stream 3D coordinate and status data (fastest method)
        Aim_SetAcquireData(aim_handle_, I_ETHERNET, DT_INFO);

        // Tell the SDK where to find the .aimtool definition files
        Aim_SetToolInfoFilePath(aim_handle_, tool_path_.c_str());

        // 3. Initialize the TF2 Broadcaster
        tf_broadcaster_ = std::make_unique<tf2_ros::TransformBroadcaster>(*this);

        // 4. Create a high-frequency polling timer (100Hz = 10ms)
        timer_ = this->create_wall_timer(
            10ms, std::bind(&AimooeTrackerNode::tracking_loop, this));
    }

    ~AimooeTrackerNode()
    {
        if (aim_handle_) {
            RCLCPP_INFO(this->get_logger(), "Shutting down Aimooe SDK safely...");
            Aim_API_Close(aim_handle_);
        }
    }

private:
    void tracking_loop()
    {
        T_MarkerInfo marker_st;
        
        // Pull the raw 3D marker coordinates from the camera
        E_ReturnValue rlt = Aim_GetMarkerInfo(aim_handle_, I_ETHERNET, marker_st);
        
        if (rlt == AIMOOE_OK) {
            std::map<std::string, T_AimToolDataResult*> result_map;
            
            // Map the scattered raw points to our defined tools
            E_ReturnValue tool_rlt = Aim_FindToolInfo_Map(
                aim_handle_, marker_st, tracked_tools_, result_map, 3);

            if (tool_rlt == AIMOOE_OK) {
                
                // Iterate through the results
                for (const auto& pair : result_map) {
                    const std::string& tool_name = pair.first;
                    T_AimToolDataResult* result = pair.second;

                    // If the tool is currently visible and valid, broadcast it
                    if (result != nullptr && result->validflag) {
                        geometry_msgs::msg::TransformStamped t;

                        // Header configuration
                        t.header.stamp = this->get_clock()->now();
                        t.header.frame_id = "aimooe_camera_link"; // The parent fixed frame
                        t.child_frame_id = tool_name;             // The dynamic tool frame

                        // IMPORTANT: Aimooe SDK returns millimeters. 
                        // MoveIt and ROS2 standard require meters.
                        t.transform.translation.x = result->Tto[0] / 1000.0;
                        t.transform.translation.y = result->Tto[1] / 1000.0;
                        t.transform.translation.z = result->Tto[2] / 1000.0;

                        // Aimooe SDK returns quaternion as: Qx, Qy, Qz, Qo (w)
                        t.transform.rotation.x = result->Qoxyz[0];
                        t.transform.rotation.y = result->Qoxyz[1];
                        t.transform.rotation.z = result->Qoxyz[2];
                        t.transform.rotation.w = result->Qoxyz[3];

                        // Send the transform to the ROS network
                        tf_broadcaster_->sendTransform(t);
                    }
                }
            }
        }
    }

    AimHandle aim_handle_;
    std::string tool_path_;
    std::vector<std::string> tracked_tools_;
    
    rclcpp::TimerBase::SharedPtr timer_;
    std::unique_ptr<tf2_ros::TransformBroadcaster> tf_broadcaster_;
};

// ==========================================
// ROS2 Node Entry Point
// ==========================================
int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<AimooeTrackerNode>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}