#ifndef AIMOOE_CORE__AIMOOE_TRACKER_HPP_
#define AIMOOE_CORE__AIMOOE_TRACKER_HPP_

#include <string>
#include <vector>
#include <optional>
#include <array>
#include <tuple>
#include <thread>
#include <chrono>

// Aimooe SDK Headers
#include "aimooe_core/AimPositionAPI.h"
#include "aimooe_core/AimPositionDef.h"

typedef void* AimHandle;

namespace aimooe_core
{

enum class ConnectionInterface {
	ETHERNET = 0,
	USB = 1,
	WIFI = 2
};

enum class ReturnCode {
	ERROR = -1,
	OK = 0,
    CONNECT_ERROR = 1,
    NOT_CONNECTED = 2,
    READ_FAULT = 3,
    WRITE_FAULT = 4,
    STALE_DATA = 5,
    INIT_FAILED = 6,
    INVALID_HANDLE = 7
};

enum class ToolType {
    TOOL = 0,
    CALIBRATION_BOARD = 1
};

enum class AcquiredDataType {
	NONE = 0,
	INFO = 1,
	MARKER_INFO_WITH_WIFI = 2,
	STATUS_INFO = 3,
	IMGDUAL = 4,
	IMGCOLOR = 5,
	INFO_IMGDUAL = 6,
	INFO_IMGCOLOR = 7,
	INFO_IMGDUAL_IMGCOLOR = 8
};

enum class CollisionStatus {
	NONE = 0,
	OCCURED = 1,
	DISABLED = 2
};

enum class HardwareStatus {
    OK = 0,
    LCD_VOLTAGE_TOO_LOW = 1,
    LCD_VOLTAGE_TOO_HIGH = 2,
    IR_LEFT_VOLTAGE_TOO_LOW = 3,
    IR_LEFT_VOLTAGE_TOO_HIGH = 4,
    IR_RIGHT_VOLTAGE_TOO_LOW = 5,
    IR_RIGHT_VOLTAGE_TOO_HIGH = 6,
    GET_INIT_DATA_ERROR = 7
};

enum class MarkerBGLightStatus {
	OK = 0,
	ABNORMAL = 1
};

enum EdgeWarnings {
	NONE = 0,
	COMMON = 1,
	CRITICAL = 2
};

struct ToolInfo {
	ToolType tool_type;
	bool is_valid;
	std::string tool_name;
	float mean_abs_error;
	float rms_error;
	std::array<float, 3> rotation_vector;
	std::array<float, 4> quaternion; // Qx, Qy, Qz, Qo
	std::array<std::array<float, 3>, 3> rotation_matrix;
	std::array<float, 3> translation_vector;
	std::array<float, 3> origin_coordinates;
	std::vector<double> marker_points;
};

struct CameraStatusInfo {
	float temperature_cpu;
	float temperature_board;
	int fps_left;
	int fps_right;
	int fps_color;
	int fps_lcd;
	int exposure_time_left;
	int exposure_time_right;
	CollisionStatus collision_status;
	HardwareStatus hardware_status;
};

struct MarkersInfo {
	int id;
	int marker_count;
	std::array<double, 600> marker_coordinates;
	std::array<int, 200> phantom_marker_warning;
	int phantom_marker_group_count;
	MarkerBGLightStatus bg_light_status;
	std::array<EdgeWarnings, 200> marker_edge_warnings;
	EdgeWarnings left_out_warning;
	EdgeWarnings right_out_warning;
};

struct ToolCreationProgress {
	bool invalid_marker_flag{false};
	float progress_rate{0.0f};
	bool finished{false};
	float error{0.0f};
};

struct ToolSelfCalibrationProgress {
	int total_marker_count{0};
	int valid_calibration_count{0};
	bool finished{false};
	float match_error{0.0f};
};

struct ToolTipCalibrationProgress {
	bool board_found{false};
	bool tool_found{false};
	bool valid_calibration{false};
	bool finished{false};
	float calibration_error{0.0f};
	float progress_rate{0.0f};
	float rms_error{0.0f};
};

struct ToolTipPivotProgress {
	bool tool_found{false};
	bool finished{false};
	float progress_rate{0.0f};
	float mean_error{0.0f};
};

class AimooeTracker {
	public:
		AimooeTracker();
		~AimooeTracker();

		// Lifecycle
		ReturnCode connect(std::optional<ConnectionInterface> conn_interface);
		ReturnCode disconnect();

		// Configuration
		void set_connection_interface(ConnectionInterface conn_interface);
		ReturnCode set_acquired_data(AcquiredDataType acquired_data);
		bool set_tools_path(const std::string& path);
		bool is_connected() const;
		
		// Data Fetching
		std::string get_tools_path() const;
		std::tuple<ReturnCode, CameraStatusInfo> get_status_info();
		std::tuple<ReturnCode, MarkersInfo> get_markers_info();

		// Tool Creation Methods
		ReturnCode tool_create_init(int marker_count, const std::string& tool_name);
		std::tuple<ReturnCode, ToolCreationProgress> tool_create_process();
		ReturnCode tool_create_finish(bool save);

		// Tool Self-Calibration Methods
		std::tuple<ReturnCode, int> tool_self_calibration_init(const std::string& tool_name);
		std::tuple<ReturnCode, ToolSelfCalibrationProgress> tool_self_calibration_process();
		ReturnCode tool_self_calibration_finish(bool save);

		// Tool Tip Calibration Methods
		ReturnCode tool_tip_calibration_init(const std::string& calibration_board_tool_name, const std::string& tool_name);
		std::tuple<ReturnCode, ToolTipCalibrationProgress> tool_tip_calibration_process();
		ReturnCode tool_tip_calibration_finish(bool save);

		// Tool Tip Pivot Methods
		ReturnCode tool_tip_pivot_init(const std::string& tool_name, bool clear_tip_mid = false);
		std::tuple<ReturnCode, ToolTipPivotProgress> tool_tip_pivot_process();
		ReturnCode tool_tip_pivot_finish(bool save);

		// Tracking Methods
		std::tuple<ReturnCode, std::optional<ToolInfo>> find_tool(const std::string& tool_name, int min_match_points);
		std::tuple<ReturnCode, std::vector<ToolInfo>> find_tools(std::vector<std::string>& tool_names, int min_match_points);
		std::tuple<ReturnCode, std::vector<ToolInfo>> find_valid_tools(std::vector<std::string>& tool_names, int min_match_points);
		std::tuple<ReturnCode, bool> tool_detected(const std::string& tool_name, int min_match_points);

	private:
		AimHandle handle_;

		ConnectionInterface conn_interface_;
		AcquiredDataType acquired_data_;
		std::string tools_path_str_;
		bool tool_path_set_;
		bool connected_;

        t_ToolMadeProInfo tool_create_info_;
        bool tool_create_initialized_;
        
        t_ToolFixProInfo tool_self_cal_info_;
        bool tool_self_cal_initialized_;

        t_ToolTipCalProInfo tool_tip_cal_info_;
        bool tool_tip_cal_initialized_;

        T_ToolTipPivotInfo tool_tip_pivot_info_;
        bool tool_tip_pivot_initialized_;

		// Internal mappers
		E_Interface _to_aim_connection_interface(ConnectionInterface interface);
		E_DataType _to_aim_acquired_data_type(AcquiredDataType data_type);
		ReturnCode _from_aim_return_code(int aim_code);
		CollisionStatus _from_aim_collision_status(int aim_status);
		HardwareStatus _from_aim_hardware_status(int aim_status);
		MarkerBGLightStatus _from_aim_bg_light_status(int aim_status);
		EdgeWarnings _from_aim_edge_warning(int aim_warning);
};

} // namespace aimooe_core

#endif // AIMOOE_CORE__AIMOOE_TRACKER_HPP_