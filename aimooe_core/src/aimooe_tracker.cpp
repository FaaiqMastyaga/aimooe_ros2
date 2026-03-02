#include "aimooe_core/aimooe_tracker.hpp"

namespace aimooe_core
{

// ==============================================================================
// Constructor & Destructor
// ==============================================================================

AimooeTracker::AimooeTracker() :
    handle_(nullptr),
    conn_interface_(ConnectionInterface::ETHERNET),
    acquired_data_(AcquiredDataType::NONE),
    tools_path_str_(""),
    tool_path_set_(false),
    connected_(false),
    tool_create_info_({}), 
    tool_create_initialized_(false),
    tool_self_cal_info_({}), 
    tool_self_cal_initialized_(false),
    tool_tip_cal_info_({}), 
    tool_tip_cal_initialized_(false),
    tool_tip_pivot_info_({}), 
    tool_tip_pivot_initialized_(false)
{
    // Initialize the Aim API and get a handle
    Aim_API_Initial(handle_);
}
 
AimooeTracker::~AimooeTracker()
{
    // Clean up any resources and close the Aim API
    if (connected_) {
        disconnect();
    }
    else if (handle_ != nullptr) {
        Aim_API_Close(handle_);
        handle_ = nullptr;
    }
}

// ==============================================================================
// Helper Lambda for Hardware Polling
// ==============================================================================

// This local lambda avoids duplicating the 5-attempt polling loop everywhere
auto fetch_hardware_data = [](AimHandle handle, ConnectionInterface interface) -> std::tuple<int, T_MarkerInfo, T_AimPosStatusInfo> {
    T_MarkerInfo marker_info = {};
    T_AimPosStatusInfo status_info = {};
    
    E_ReturnValue ret = Aim_GetMarkerAndStatusFromHardware(handle, (E_Interface)interface, marker_info, status_info);
    int attempts = 0;
    
    while (ret == AIMOOE_NOT_REFLASH && attempts < 5) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        ret = Aim_GetMarkerAndStatusFromHardware(handle, (E_Interface)interface, marker_info, status_info);
        attempts++;
    }
    return {ret, marker_info, status_info};
};

// ==============================================================================
// Lifecycle & Configuration
// ==============================================================================

ReturnCode AimooeTracker::connect(std::optional<ConnectionInterface> conn_interface)
{
    /*
    Connect to the Aim device and initialize handles.

    Applies any configured tools path after connecting.

    Args:
        connection_interface: Optional override for the connection interface.
            Currently ignored; call set_connection_interface before connect.

    Returns:
        ReturnCode: Result of the connection attempt.
    */
    if (conn_interface.has_value()) {
        set_connection_interface(conn_interface.value());
    }

    if (handle_ == nullptr) {
        Aim_API_Initial(handle_);
        if (handle_ == nullptr) {
            return ReturnCode::INIT_FAILED;
        }
    }

    E_Interface connection_interface = _to_aim_connection_interface(conn_interface_);
    T_AIMPOS_DATAPARA pos_data = {};
    E_ReturnValue aim_result = Aim_ConnectDevice(
        handle_, 
        connection_interface, 
        pos_data
    );
    ReturnCode return_code = _from_aim_return_code(aim_result);
    connected_ = (return_code == ReturnCode::OK);
    if (connected_ && tools_path_str_ != "") {
        set_tools_path(tools_path_str_);
    }
    else {
        tool_path_set_ = false;
    }
    return return_code;
}

ReturnCode AimooeTracker::disconnect()
{
    /*
    Disconnect from the Aim device and clean up resources.

    Returns:
        ReturnCode: Result from Aim_API_Close, or INVALID_HANDLE if uninitialized.
    */
    if (handle_ == nullptr) {
        return ReturnCode::INVALID_HANDLE;
    }
    if (!connected_) {
        return ReturnCode::NOT_CONNECTED;
    }
    // Clean up any tool creation or calibration states
    tool_create_info_ = {};
    tool_create_initialized_ = false;
    tool_self_cal_info_ = {};
    tool_self_cal_initialized_ = false;
    tool_tip_cal_info_ = {};
    tool_tip_cal_initialized_ = false;
    tool_tip_pivot_info_ = {};
    tool_tip_pivot_initialized_ = false;

    // Call the Aim API to disconnect
    E_ReturnValue aim_result = Aim_API_Close(handle_);
    ReturnCode return_code = _from_aim_return_code(aim_result);
    if (return_code == ReturnCode::OK) {
        connected_ = false;
        handle_ = nullptr;
        tool_path_set_ = false;
    }
    return return_code;
}

void AimooeTracker::set_connection_interface(ConnectionInterface conn_interface) 
{
    /*
    Set the connection interface used for future connections.

    Args:
        interface: Connection interface such as Ethernet, USB, or WiFi.
    */
   conn_interface_ = conn_interface;
}

ReturnCode AimooeTracker::set_acquired_data(AcquiredDataType acquired_data) 
{
    /*
    Configure which data types the Aim backend should acquire.

    Args:
        acquired_data: Data type selection for the Aim device.

    Returns:
        ReturnCode: Result from the Aim_SetAcquireData call.
    */
    acquired_data_ = acquired_data;
    E_Interface interface = _to_aim_connection_interface(conn_interface_);
    E_DataType aim_data_type = _to_aim_acquired_data_type(acquired_data_);
    E_ReturnValue aim_result = Aim_SetAcquireData(
        handle_, 
        interface, 
        aim_data_type
    );
    return _from_aim_return_code(aim_result);
}

bool AimooeTracker::set_tools_path(const std::string& path) 
{
    /*
    Configure the tools file path for Aim lookups.

    Args:
        string: Path to the directory containing tool files.
    */    
    tools_path_str_ = path;
    if (connected_) {
        E_ReturnValue aim_result = Aim_SetToolInfoFilePath(
            handle_, 
            tools_path_str_.c_str(), 
            true
        );
        tool_path_set_ = _from_aim_return_code(aim_result) == ReturnCode::OK;
    } 
    else {
        tool_path_set_ = false;
    }
    return tool_path_set_;
}

bool AimooeTracker::is_connected() const
{
    /*
    Check if the tracker is currently connected to the Aim device.

    Returns:
        bool: True if connected, false otherwise.
    */
    return connected_;
}

// ==============================================================================
// Data Fetching
// ==============================================================================

std::string AimooeTracker::get_tools_path() const
{
    /*
    Get the currently set tools path.

    Returns:
        std::string: The current tools path.
    */
    return tools_path_str_;
}

std::tuple<ReturnCode, CameraStatusInfo> AimooeTracker::get_status_info()
{
    /*
    Fetch the latest camera and system status information from the Aim device.

    Returns:
        tuple[ReturnCode, CameraStatusInfo]: Result code and status info struct.
    */


    if (!connected_) {
        return {ReturnCode::NOT_CONNECTED, CameraStatusInfo()};
    }
    if (handle_ == nullptr) {
        return {ReturnCode::INVALID_HANDLE, CameraStatusInfo()};
    }

    auto [aim_code, marker_info, status_info] = fetch_hardware_data(handle_, conn_interface_);
    ReturnCode return_code = _from_aim_return_code(aim_code);
    
    CameraStatusInfo cam_status;
    cam_status.temperature_cpu = status_info.Tcpu;
    cam_status.temperature_board = status_info.Tpcb;
    cam_status.fps_left = status_info.LeftCamFps;
    cam_status.fps_right = status_info.RightCamFps;
    cam_status.fps_color = status_info.ColorCamFps;
    cam_status.fps_lcd = status_info.LCDFps;
    cam_status.exposure_time_left = status_info.ExposureTimeLeftCam;
    cam_status.exposure_time_right = status_info.ExposureTimeRightCam;
    cam_status.collision_status = _from_aim_collision_status(status_info.CollisionStatus);
    cam_status.hardware_status = _from_aim_hardware_status(status_info.HardwareStatus);
    
    return {return_code, cam_status};
}

std::tuple<ReturnCode, MarkersInfo> AimooeTracker::get_markers_info()
{
    /*
    Fetch the latest marker information from the Aim device.

    Returns:
        tuple[ReturnCode, MarkersInfo]: Result code and markers info struct.
    */
    if (!connected_) {
        return {ReturnCode::NOT_CONNECTED, MarkersInfo()};
    }
    if (handle_ == nullptr) {
        return {ReturnCode::INVALID_HANDLE, MarkersInfo()};
    }

    auto [aim_code, marker_info, status_info] = fetch_hardware_data(handle_, conn_interface_);
    ReturnCode return_code = _from_aim_return_code(aim_code);

    MarkersInfo markers_info;
    markers_info.id = marker_info.ID;
    markers_info.marker_count = marker_info.MarkerNumber;
    std::copy(std::begin(marker_info.MarkerCoordinate), std::end(marker_info.MarkerCoordinate), markers_info.marker_coordinates.begin());
    std::copy(std::begin(marker_info.PhantomMarkerWarning), std::end(marker_info.PhantomMarkerWarning), markers_info.phantom_marker_warning.begin());
    markers_info.phantom_marker_group_count = marker_info.PhantomMarkerGroupNumber;
    markers_info.bg_light_status = _from_aim_bg_light_status(marker_info.MarkerBGLightStatus);
    for (size_t i = 0; i < 200; ++i) markers_info.marker_edge_warnings[i] = _from_aim_edge_warning(marker_info.MarkWarn[i]);
    markers_info.left_out_warning = _from_aim_edge_warning(marker_info.bLeftOutWarnning);
    markers_info.right_out_warning = _from_aim_edge_warning(marker_info.bRightOutWarning);

    return {return_code, markers_info};
}

// ==============================================================================
// Tool Creation Workflow
// ==============================================================================

ReturnCode AimooeTracker::tool_create_init(int marker_count, const std::string& tool_name)
{
    /*
    Initialize a tool creation session for the Aim device.

    Use marker_count=4 for four-point tool creation.

    Args:
        marker_count: Number of markers expected for the tool.
        tool_name: Tool name used when saving the file.

    Returns:
        ReturnCode: Result from Aim_InitToolMadeInfo.
    */
    if (!connected_) {
        tool_create_info_ = {};
        tool_create_initialized_ = false;
        return ReturnCode::NOT_CONNECTED;
    }
    if (handle_ == nullptr) {
        tool_create_info_ = {};
        tool_create_initialized_ = false;
        return ReturnCode::INVALID_HANDLE;
    }
    if (marker_count <= 0 || tool_name.empty()) {
        tool_create_info_ = {};
        tool_create_initialized_ = false;
        return ReturnCode::ERROR;
    }

    E_ReturnValue aim_result = Aim_InitToolMadeInfo(
        handle_,
        marker_count,
        tool_name.c_str()
    );
    ReturnCode return_code = _from_aim_return_code(aim_result);
    if (return_code == ReturnCode::OK) {
        tool_create_info_ = t_ToolMadeProInfo();
        tool_create_initialized_ = true;
    }
    else {
        tool_create_info_ = {};
        tool_create_initialized_ = false;
    }
    return return_code;
}

std::tuple<ReturnCode, ToolCreationProgress> AimooeTracker::tool_create_process()
{
    /*
    Capture markers and advance tool creation progress.

    Call after tool_create_init until finished is True. Requires acquired data
    set to NONE or INFO.

    Returns:
        tuple[ReturnCode, ToolCreationProgress]: ReturnCode and progress info.
    */
    ToolCreationProgress empty_progress = ToolCreationProgress();
    if (!connected_) {
        return {ReturnCode::NOT_CONNECTED, empty_progress};
    }
    if (handle_ == nullptr) {
        return {ReturnCode::INVALID_HANDLE, empty_progress};
    }
    if (!tool_create_initialized_) {
        return {ReturnCode::ERROR, empty_progress};
    }
    if (acquired_data_ != AcquiredDataType::NONE && acquired_data_ != AcquiredDataType::INFO) {
        return {ReturnCode::STALE_DATA, empty_progress};
    }

    auto [aim_code, marker_info, status_info] = fetch_hardware_data(handle_, conn_interface_);
    ReturnCode return_code = _from_aim_return_code(aim_code);

    if (return_code != ReturnCode::OK) {
        return {return_code, empty_progress};
    }

    E_ReturnValue progress_result = Aim_ProceedToolMade(
        handle_, 
        marker_info, 
        tool_create_info_
    );
    return_code = _from_aim_return_code(progress_result);
    if (return_code != ReturnCode::OK) {
        return {return_code, empty_progress};
    }

    ToolCreationProgress progress;
    progress.invalid_marker_flag = tool_create_info_.unValidMarkerFlag;
    progress.progress_rate = tool_create_info_.madeRate;
    progress.finished = tool_create_info_.isMadeProFinished;
    progress.error = tool_create_info_.MadeError;
    return {return_code, progress};
}

ReturnCode AimooeTracker::tool_create_finish(bool save)
{
    /*
    Finish tool creation and optionally save the tool file.

    Args:
        save: If true, saves the created tool to the tools path.

    Returns:
        ReturnCode: Result from Aim_SaveToolMadeRlt.
    */
   if (!connected_) {
        tool_create_info_ = {};
        tool_create_initialized_ = false;
        return ReturnCode::NOT_CONNECTED;
    }
    if (handle_ == nullptr) {
        tool_create_info_ = {};
        tool_create_initialized_ = false;
        return ReturnCode::INVALID_HANDLE;
    }
    if (!tool_create_initialized_) {
        tool_create_info_ = {};
        tool_create_initialized_ = false;
        return ReturnCode::ERROR;
    }

    E_ReturnValue aim_result = Aim_SaveToolMadeRlt(
        handle_, 
        save
    );
    ReturnCode return_code = _from_aim_return_code(aim_result);
    // Reset state after finishing
    tool_create_info_ = {};
    tool_create_initialized_ = false; 
    return return_code;
}

// ==============================================================================
// Tool Self-Calibration Workflow
// ==============================================================================

std::tuple<ReturnCode, int> AimooeTracker::tool_self_calibration_init(const std::string& tool_name)
{
    /*
    Initialize a tool self-calibration session for the Aim device.

    Args:
        tool_name: Name of the tool to self-calibrate.

    Returns:
        tuple[ReturnCode, int]: Result code and total marker count for calibration.
    */
    if (!connected_) {
        tool_self_cal_info_ = {};
        tool_self_cal_initialized_ = false;
        return {ReturnCode::NOT_CONNECTED, -1};
    }
    if (handle_ == nullptr) {
        tool_self_cal_info_ = {};
        tool_self_cal_initialized_ = false;
        return {ReturnCode::INVALID_HANDLE, -1};
    }
    if (tool_name.empty()) {
        tool_self_cal_info_ = {};
        tool_self_cal_initialized_ = false;
        return {ReturnCode::ERROR, -1};
    }

    int total_marker_count = 0;
    E_ReturnValue aim_result = Aim_InitToolSelfCalibrationWithToolId(
        handle_,
        tool_name.c_str(),
        total_marker_count
    );

    ReturnCode return_code = _from_aim_return_code(aim_result);
    if (return_code != ReturnCode::OK) {
        tool_self_cal_initialized_ = false;
        return {return_code, -1};
    }

    tool_self_cal_info_ = t_ToolFixProInfo();
    tool_self_cal_info_.totalmarkcnt = total_marker_count;
    tool_self_cal_initialized_ = true;
    return {ReturnCode::OK, total_marker_count};
}

std::tuple<ReturnCode, ToolSelfCalibrationProgress> AimooeTracker::tool_self_calibration_process()
{
    /*
    Capture markers and advance tool self-calibration progress.

    Call after tool_self_calibration_init until finished is True. Requires acquired data
    set to NONE or INFO.

    Returns:
        tuple[ReturnCode, ToolSelfCalibrationProgress]: ReturnCode and progress info.
    */
    ToolSelfCalibrationProgress empty_progress = ToolSelfCalibrationProgress();
    if (!connected_) {
        return {ReturnCode::NOT_CONNECTED, empty_progress};
    }
    if (handle_ == nullptr) {
        return {ReturnCode::INVALID_HANDLE, empty_progress};
    }
    if (!tool_self_cal_initialized_) {
        return {ReturnCode::ERROR, empty_progress};
    }
    if (acquired_data_ != AcquiredDataType::NONE && acquired_data_ != AcquiredDataType::INFO) {
        return {ReturnCode::STALE_DATA, empty_progress};
    }

    auto [aim_code, marker_info, status_info] = fetch_hardware_data(handle_, conn_interface_);
    ReturnCode return_code = _from_aim_return_code(aim_code);

    if (return_code != ReturnCode::OK) {
        return {return_code, empty_progress};
    }

    E_ReturnValue progress_result = Aim_ProceedToolSelfCalibration(
        handle_, 
        marker_info, 
        tool_self_cal_info_
    );
    return_code = _from_aim_return_code(progress_result);
    if (return_code != ReturnCode::OK) {
        return {return_code, empty_progress};
    }

    ToolSelfCalibrationProgress progress;
    progress.total_marker_count = tool_self_cal_info_.totalmarkcnt;
    progress.valid_calibration_count = tool_self_cal_info_.isValidFixCnt;
    progress.finished = tool_self_cal_info_.isCalibrateFinished;
    progress.match_error = tool_self_cal_info_.MatchError;
    return {return_code, progress};
}

ReturnCode AimooeTracker::tool_self_calibration_finish(bool save)
{
    /*
    Finish tool self-calibration and optionally save the results.

    Call after tool_self_calibration_process reports finished.

    Args:
        save: If true, saves the calibrated tool to the tools path.

    Returns:
        ReturnCode: Result from Aim_SaveToolSelfCalibration.
    */
   if (!connected_) {
        tool_self_cal_info_ = {};
        tool_self_cal_initialized_ = false;
        return ReturnCode::NOT_CONNECTED;
    }
    if (handle_ == nullptr) {
        tool_self_cal_info_ = {};
        tool_self_cal_initialized_ = false;
        return ReturnCode::INVALID_HANDLE;
    }
    if (!tool_self_cal_initialized_) {
        tool_self_cal_info_ = {};
        tool_self_cal_initialized_ = false;
        return ReturnCode::ERROR;
    }

    E_ToolFixRlt fix_result;
    if (save) {
        fix_result = eToolFixSave;
    }
    else {
        fix_result = eToolFixCancle;
    }

    E_ReturnValue aim_result = Aim_SaveToolSelfCalibration(
        handle_, 
        fix_result
    );
    ReturnCode return_code = _from_aim_return_code(aim_result);
    // Reset state after finishing
    tool_self_cal_info_ = {};
    tool_self_cal_initialized_ = false; 
    return return_code;
}

// ==============================================================================
// Tool Tip Calibration Workflow (Using Registration Board)
// ==============================================================================

ReturnCode AimooeTracker::tool_tip_calibration_init(const std::string& calibration_board_tool_name, const std::string& tool_name)
{
    /*
    Initialize a tool tip calibration session for the Aim device.

    Args:
        calibration_board_tool_name: Name of the calibration board tool.
        tool_name: Name of the tool to calibrate.

    Returns:
        ReturnCode: Result from Aim_InitToolTipCalibrationWithToolId.
    */
    if (!connected_) {
        tool_tip_cal_info_ = {};
        tool_tip_cal_initialized_ = false;
        return ReturnCode::NOT_CONNECTED;
    }
    if (handle_ == nullptr) {
        tool_tip_cal_info_ = {};
        tool_tip_cal_initialized_ = false;
        return ReturnCode::INVALID_HANDLE;
    }
    if (!tool_path_set_) {
        tool_tip_cal_info_ = {};
        tool_tip_cal_initialized_ = false;
        return ReturnCode::ERROR;
    }
    if (calibration_board_tool_name.empty() || tool_name.empty()) {
        tool_tip_cal_info_ = {};
        tool_tip_cal_initialized_ = false;
        return ReturnCode::ERROR;
    }

    E_ReturnValue aim_result = Aim_InitToolTipCalibrationWithToolId(
        handle_,
        calibration_board_tool_name.c_str(),
        tool_name.c_str()
    );
    ReturnCode return_code = _from_aim_return_code(aim_result);
    if (return_code == ReturnCode::OK) {
        tool_tip_cal_info_ = t_ToolTipCalProInfo();
        tool_tip_cal_initialized_ = true;
    }
    else {
        tool_tip_cal_info_ = {};
        tool_tip_cal_initialized_ = false;
    }
    return return_code;
}

std::tuple<ReturnCode, ToolTipCalibrationProgress> AimooeTracker::tool_tip_calibration_process()
{
    /*
    Capture markers and advance tool tip calibration progress.

    Call after tool_tip_calibration_init until finished is True. Requires acquired data
    set to NONE or INFO.

    Returns:
        tuple[ReturnCode, ToolTipCalibrationProgress]: ReturnCode and progress info.
    */
    ToolTipCalibrationProgress empty_progress = ToolTipCalibrationProgress();
    if (!connected_) {
        return {ReturnCode::NOT_CONNECTED, empty_progress};
    }
    if (handle_ == nullptr) {
        return {ReturnCode::INVALID_HANDLE, empty_progress};
    }
    if (!tool_tip_cal_initialized_) {
        return {ReturnCode::ERROR, empty_progress};
    }
    if (acquired_data_ != AcquiredDataType::NONE && acquired_data_ != AcquiredDataType::INFO) {
        return {ReturnCode::STALE_DATA, empty_progress};
    }

    auto [aim_code, marker_info, status_info] = fetch_hardware_data(handle_, conn_interface_);
    ReturnCode return_code = _from_aim_return_code(aim_code);

    if (return_code != ReturnCode::OK) {
        return {return_code, empty_progress};
    }

    E_ReturnValue progress_result = Aim_ProceedToolTipCalibration(
        handle_, 
        marker_info, 
        tool_tip_cal_info_
    );
    return_code = _from_aim_return_code(progress_result);
    if (return_code != ReturnCode::OK) {
        return {return_code, empty_progress};
    }

    ToolTipCalibrationProgress progress;
    progress.board_found = tool_tip_cal_info_.isBoardFind;
    progress.tool_found = tool_tip_cal_info_.isToolFind;
    progress.valid_calibration = tool_tip_cal_info_.isValidCalibrate;
    progress.finished = tool_tip_cal_info_.isCalibrateFinished;
    progress.calibration_error = tool_tip_cal_info_.CalibrateError;
    progress.progress_rate = tool_tip_cal_info_.CalibrateRate;
    progress.rms_error = tool_tip_cal_info_.CalRMSError;
    return {return_code, progress};
}

ReturnCode AimooeTracker::tool_tip_calibration_finish(bool save)
{
    /*
    Finish tool tip calibration and optionally save the results.

    Call after tool_tip_calibration_process reports finished.

    Args:
        save: If true, saves the calibrated tool to the tools path.

    Returns:
        ReturnCode: Result from Aim_SaveToolTipCalibration.
    */
   if (!connected_) {
        tool_tip_cal_info_ = {};
        tool_tip_cal_initialized_ = false;
        return ReturnCode::NOT_CONNECTED;
    }
    if (handle_ == nullptr) {
        tool_tip_cal_info_ = {};
        tool_tip_cal_initialized_ = false;
        return ReturnCode::INVALID_HANDLE;
    }
    if (!tool_tip_cal_initialized_) {
        tool_tip_cal_info_ = {};
        tool_tip_cal_initialized_ = false;
        return ReturnCode::ERROR;
    }

    ReturnCode return_code = ReturnCode::OK;
    if (save) {
        E_ReturnValue save_result = Aim_SaveToolTipCalibration(handle_);
        return_code = _from_aim_return_code(save_result);
    }
    tool_tip_cal_info_ = {};
    tool_tip_cal_initialized_ = false;
    return return_code;
}

// ==============================================================================
// Tool Tip Pivot Workflow (Dynamic Tip Registration)
// ==============================================================================

ReturnCode AimooeTracker::tool_tip_pivot_init(const std::string& tool_name, bool clear_tip_mid)
{
    /*
    Initialize a tool tip pivot session for the Aim device.

    Args:
        tool_name: Name of the tool to pivot calibrate.
        clear_tip_mid: If true, clears existing tip mid-point before calibration.

    Returns:
        ReturnCode: Result from Aim_InitToolTipPivotWithToolId.
    */
    if (!connected_) {
        tool_tip_pivot_info_ = {};
        tool_tip_pivot_initialized_ = false;
        return ReturnCode::NOT_CONNECTED;
    }
    if (handle_ == nullptr) {
        tool_tip_pivot_info_ = {};
        tool_tip_pivot_initialized_ = false;
        return ReturnCode::INVALID_HANDLE;
    }
    if (!tool_path_set_) {
        tool_tip_pivot_info_ = {};
        tool_tip_pivot_initialized_ = false;
        return ReturnCode::ERROR;
    }
    if (tool_name.empty()) {
        tool_tip_pivot_info_ = {};
        tool_tip_pivot_initialized_ = false;
        return ReturnCode::ERROR;
    }

    E_ReturnValue aim_result = Aim_InitToolTipPivotWithToolId(
        handle_,
        tool_name.c_str(),
        clear_tip_mid
    );
    ReturnCode return_code = _from_aim_return_code(aim_result);
    if (return_code == ReturnCode::OK) {
        tool_tip_pivot_info_ = T_ToolTipPivotInfo();
        tool_tip_pivot_initialized_ = true;
    }
    else {
        tool_tip_pivot_info_ = {};
        tool_tip_pivot_initialized_ = false;
    }
    return return_code;
}

std::tuple<ReturnCode, ToolTipPivotProgress> AimooeTracker::tool_tip_pivot_process()
{
    /*
    Capture markers and advance tool tip pivot progress.

    Call after tool_tip_pivot_init until finished is True. Requires acquired data
    set to NONE or INFO.

    Returns:
        tuple[ReturnCode, ToolTipPivotProgress]: ReturnCode and progress info.
    */
    ToolTipPivotProgress empty_progress = ToolTipPivotProgress();
    if (!connected_) {
        return {ReturnCode::NOT_CONNECTED, empty_progress};
    }
    if (handle_ == nullptr) {
        return {ReturnCode::INVALID_HANDLE, empty_progress};
    }
    if (!tool_tip_pivot_initialized_) {
        return {ReturnCode::ERROR, empty_progress};
    }
    if (acquired_data_ != AcquiredDataType::NONE && acquired_data_ != AcquiredDataType::INFO) {
        return {ReturnCode::STALE_DATA, empty_progress};
    }

    auto [aim_code, marker_info, status_info] = fetch_hardware_data(handle_, conn_interface_);
    ReturnCode return_code = _from_aim_return_code(aim_code);

    if (return_code != ReturnCode::OK) {
        return {return_code, empty_progress};
    }

    E_ReturnValue progress_result = Aim_ProceedToolTipPivot(
        handle_, 
        marker_info, 
        tool_tip_pivot_info_
    );
    return_code = _from_aim_return_code(progress_result);
    if (return_code != ReturnCode::OK) {
        return {return_code, empty_progress};
    }

    ToolTipPivotProgress progress;
    progress.tool_found = tool_tip_pivot_info_.isToolFind;
    progress.finished = tool_tip_pivot_info_.isPivotFinished;
    progress.progress_rate = tool_tip_pivot_info_.pivotRate;
    progress.mean_error = tool_tip_pivot_info_.pivotMeanError;
    return {return_code, progress};
}

ReturnCode AimooeTracker::tool_tip_pivot_finish(bool save)
{
    /*
    Finish tool tip pivot and optionally save the results.

    Call after tool_tip_pivot_process reports finished.

    Args:
        save: If true, saves the pivot-calibrated tool to the tools path.

    Returns:
        ReturnCode: Result from Aim_SaveToolTipPivot.
    */
   if (!connected_) {
        tool_tip_pivot_info_ = {};
        tool_tip_pivot_initialized_ = false;
        return ReturnCode::NOT_CONNECTED;
    }
    if (handle_ == nullptr) {
        tool_tip_pivot_info_ = {};
        tool_tip_pivot_initialized_ = false;
        return ReturnCode::INVALID_HANDLE;
    }
    if (!tool_tip_pivot_initialized_) {
        tool_tip_pivot_info_ = {};
        tool_tip_pivot_initialized_ = false;
        return ReturnCode::ERROR;
    }

    ReturnCode return_code = ReturnCode::OK;
    if (save) {
        E_ReturnValue save_result = Aim_SaveToolTipPivot(handle_);
        return_code = _from_aim_return_code(save_result);
    }
    tool_tip_pivot_info_ = {};
    tool_tip_pivot_initialized_ = false;
    return return_code;
}

// ==============================================================================
// Tool Lookup
// ==============================================================================

std::tuple<ReturnCode, std::optional<ToolInfo>> AimooeTracker::find_tool(const std::string& tool_name, int min_match_points)
{
    /*
    Find a single tool by name and return its information.

    Args:
        tool_name: Name of the tool to find.
        min_match_points: Minimum number of marker points that must be matched.

    Returns:
        tuple[ReturnCode, optional[ToolInfo]]: Result code and ToolInfo if found.
    */
    std::vector<std::string> tool_names = {tool_name};
    auto [return_code, tools] = find_tools(tool_names, min_match_points);

    if (return_code != ReturnCode::OK) {
        return {return_code, std::nullopt};
    }
    if (tools.empty()) {
        return {ReturnCode::OK, std::nullopt}; 
    }
    ToolInfo tool_info = tools[0];
    if (!tool_info.is_valid) {
        return {ReturnCode::OK, std::nullopt};
    }
    return {ReturnCode::OK, tool_info};
}

std::tuple<ReturnCode, std::vector<ToolInfo>> AimooeTracker::find_tools(std::vector<std::string>& tool_names, int min_match_points)
{
    /*
    Find mmultiple tools that match the current markers and return their information.

    Args:
        tool_names: List of tool names to search for. Can be empty to search all tools.
        min_match_points: Minimum number of marker points that must be matched for a tool to be included.

    Returns:
        tuple[ReturnCode, vector[ToolInfo]]: Result code and list of matching ToolInfo.
    */
    if (!connected_) {
        return {ReturnCode::NOT_CONNECTED, {}};
    }
    if (handle_ == nullptr) {
        return {ReturnCode::INVALID_HANDLE, {}};
    }
    if (!tool_path_set_) {
        return {ReturnCode::ERROR, {}};
    }
    if (tool_names.empty()) {
        return {ReturnCode::OK, {}};
    }
    if (acquired_data_ != AcquiredDataType::NONE && acquired_data_ != AcquiredDataType::INFO) {
        return {ReturnCode::STALE_DATA, {}};
    }

    auto [aim_code, marker_info, status_info] = fetch_hardware_data(handle_, conn_interface_);
    ReturnCode return_code = _from_aim_return_code(aim_code);
    if (return_code != ReturnCode::OK) {
        return {return_code, {}};
    }

    std::vector<std::string> tool_name_vector;
    for (const auto& tool_name : tool_names) {
        tool_name_vector.push_back(tool_name);
    }
    T_AimToolDataResult* tool_results = new T_AimToolDataResult();
    E_ReturnValue aim_result = Aim_FindSpecificToolInfo(
        handle_,
        marker_info,
        tool_name_vector,
        tool_results,
        min_match_points
    );
    return_code = _from_aim_return_code(aim_result);
    if (return_code != ReturnCode::OK) {
        return {return_code, {}};
    }

    std::vector<ToolInfo> tools = {};
    int marker_count = marker_info.MarkerNumber;
    T_AimToolDataResult* result_node = tool_results;
    while (result_node != nullptr) {
        ToolType tool_type = ToolType::TOOL;
        if (result_node->type == E_AimToolType::ePosCalBoard) {
            tool_type = ToolType::CALIBRATION_BOARD;
        }

        std::array<float, 3> rotation_vector;
        for (size_t i = 0; i < 3; i++) {
            rotation_vector[i] = result_node->rotationvector[i];
        }

        std::array<float, 4> quaternion;
        for (size_t i = 0; i < 4; i++) {
            quaternion[i] = result_node->Qoxyz[i];
        }

        std::array<std::array<float, 3>, 3> rotation_matrix;
        for (size_t i = 0; i < 3; i++) {
            for (size_t j = 0; j < 3; j++) {
                rotation_matrix[i][j] = result_node->Rto[i][j];
            }
        }

        std::array<float, 3> translation_vector;
        for (size_t i = 0; i < 3; i++) {
            translation_vector[i] = result_node->Tto[i];
        }

        std::array<float, 3> origin_coordinates;
        for (size_t i = 0; i < 3; i++) {
            origin_coordinates[i] = result_node->OriginCoor[i];
        }

        std::vector<double> marker_points = {};
        for (const auto& tool_index : result_node->toolptidx) {
            int tool_index_value = static_cast<int>(tool_index);
            if (tool_index_value < 0 || tool_index_value >= marker_count) {
                marker_points.insert(marker_points.end(), {0.0, 0.0, 0.0});
            }
            else {
                int base = tool_index_value * 3;
                marker_points.insert(marker_points.end(), {
                    static_cast<double>(marker_info.MarkerCoordinate[base]),
                    static_cast<double>(marker_info.MarkerCoordinate[base+1]),    
                    static_cast<double>(marker_info.MarkerCoordinate[base+2]),    
                });
            }
        }

        ToolInfo tool_info;
        tool_info.tool_type = tool_type;
        tool_info.is_valid = result_node->validflag;
        tool_info.tool_name = result_node->toolname;
        tool_info.mean_abs_error = result_node->MeanError;
        tool_info.rms_error = result_node->Rms;
        tool_info.rotation_vector = rotation_vector;
        tool_info.quaternion = quaternion;
        tool_info.rotation_matrix = rotation_matrix;
        tool_info.translation_vector = translation_vector;
        tool_info.origin_coordinates = origin_coordinates;
        tool_info.marker_points = marker_points;
        tools.push_back(tool_info);

        T_AimToolDataResult* next_node = result_node->next;
        delete result_node;
        result_node = next_node;
    }
    tool_results = nullptr;
    return {ReturnCode::OK, tools};
}

std::tuple<ReturnCode, std::vector<ToolInfo>> AimooeTracker::find_valid_tools(std::vector<std::string>& tool_names, int min_match_points)
{
    /*
    Find multiple valid tools by name.

    Requires a connected device, a configured tools path, and acquired data
    set to INFO or NONE.

    Args:
        tool_names: List of tool names to locate.
        min_match_points: Minimum marker points required for a match.

    Returns:
        tuple[ReturnCode, list[ToolInfo]]: ReturnCode and detected valid tool list.
    */
    auto [return_code, tools_info] = find_tools(tool_names, min_match_points);
    if (return_code != ReturnCode::OK) {
        return {return_code, {}};
    }
    std::vector<ToolInfo> valid_tools;
    for (const auto& tool_info : tools_info) {
        if (tool_info.is_valid) {
            valid_tools.push_back(tool_info);
        }
    }
    return {ReturnCode::OK, valid_tools};
}

std::tuple<ReturnCode, bool> AimooeTracker::tool_detected(const std::string& tool_name, int min_match_points)
{
    /*
    Check whether a tool is detected by the Aim backend.

    Args:
        tool_name: Name of the tool to query.
        min_match_points: Minimum marker points required for a match.

    Returns:
        tuple[ReturnCode, bool]: ReturnCode and True if a tool is detected.
    */
    auto [return_code, tool_info] = find_tool(tool_name, min_match_points);
    bool detected = tool_info.has_value();
    return {return_code, detected};
}

// ==============================================================================
// Internal Mappers
// ==============================================================================

E_Interface AimooeTracker::_to_aim_connection_interface(ConnectionInterface conn_interface)
{
    switch (conn_interface) {
        case ConnectionInterface::ETHERNET:
            return I_ETHERNET;
        case ConnectionInterface::USB:
            return I_USB;
        case ConnectionInterface::WIFI:
            return I_WIFI;
        default:
            return I_ETHERNET; // Default to Ethernet if unknown
    }
}

E_DataType AimooeTracker::_to_aim_acquired_data_type(AcquiredDataType data_type)
{
    switch (data_type) {
        case AcquiredDataType::INFO:
            return E_DataType::DT_INFO;
        case AcquiredDataType::MARKER_INFO_WITH_WIFI:
            return E_DataType::DT_MARKER_INFO_WITH_WIFI;
        case AcquiredDataType::STATUS_INFO:
            return E_DataType::DT_STATUS_INFO;
        case AcquiredDataType::IMGDUAL:
            return E_DataType::DT_IMGDUAL;
        case AcquiredDataType::IMGCOLOR:
            return E_DataType::DT_IMGCOLOR;
        case AcquiredDataType::INFO_IMGDUAL:
            return E_DataType::DT_INFO_IMGDUAL;
        case AcquiredDataType::INFO_IMGCOLOR:
            return E_DataType::DT_INFO_IMGCOLOR;
        case AcquiredDataType::INFO_IMGDUAL_IMGCOLOR:
            return E_DataType::DT_INFO_IMGDUAL_IMGCOLOR;
        case AcquiredDataType::NONE:
        default:
            return E_DataType::DT_NONE;
    }
}

ReturnCode AimooeTracker::_from_aim_return_code(int aim_code)
{
    switch (aim_code) {
        case AIMOOE_OK:
            return ReturnCode::OK;
        case AIMOOE_ERROR:
            return ReturnCode::ERROR;
        case AIMOOE_CONNECT_ERROR:
            return ReturnCode::CONNECT_ERROR;
        case AIMOOE_NOT_CONNECT:
            return ReturnCode::NOT_CONNECTED;
        case AIMOOE_READ_FAULT:
            return ReturnCode::READ_FAULT;
        case AIMOOE_WRITE_FAULT:
            return ReturnCode::WRITE_FAULT;
        case AIMOOE_NOT_REFLASH:
            return ReturnCode::STALE_DATA;
        case AIMOOE_INITIAL_FAIL:
            return ReturnCode::INIT_FAILED;
        case AIMOOE_HANDLE_IS_NULL:
            return ReturnCode::INVALID_HANDLE;
        default:
            return ReturnCode::ERROR; // Default to ERROR for unknown codes
    }
}

CollisionStatus AimooeTracker::_from_aim_collision_status(int aim_status)
{
    switch (aim_status) {
        case E_CollisionStatus::COLLISION_OCCURRED:
            return CollisionStatus::OCCURED;
        case E_CollisionStatus::COLLISION_NOT_START:
            return CollisionStatus::DISABLED;
        case E_CollisionStatus::COLLISION_NOT_OCCURRED:
        default:
            return CollisionStatus::NONE;
    }
}

HardwareStatus AimooeTracker::_from_aim_hardware_status(int aim_status)
{
    switch (aim_status) {
        case E_HardwareStatus::HW_LCD_VOLTAGE_TOO_LOW:
            return HardwareStatus::LCD_VOLTAGE_TOO_LOW;
        case E_HardwareStatus::HW_LCD_VOLTAGE_TOO_HIGH:
            return HardwareStatus::LCD_VOLTAGE_TOO_HIGH;
        case E_HardwareStatus::HW_IR_LEFT_VOLTAGE_TOO_LOW:
            return HardwareStatus::IR_LEFT_VOLTAGE_TOO_LOW;
        case E_HardwareStatus::HW_IR_LEFT_VOLTAGE_TOO_HIGH:
            return HardwareStatus::IR_LEFT_VOLTAGE_TOO_HIGH;
        case E_HardwareStatus::HW_IR_RIGHT_VOLTAGE_TOO_LOW:
            return HardwareStatus::IR_RIGHT_VOLTAGE_TOO_LOW;
        case E_HardwareStatus::HW_IR_RIGHT_VOLTAGE_TOO_HIGH:
            return HardwareStatus::IR_RIGHT_VOLTAGE_TOO_HIGH;
        case E_HardwareStatus::HW_GET_INITIAL_DATA_ERROR:
            return HardwareStatus::GET_INIT_DATA_ERROR;
        case E_HardwareStatus::HW_OK:
        default:
            return HardwareStatus::OK;
    }
}

MarkerBGLightStatus AimooeTracker::_from_aim_bg_light_status(int aim_status)
{
    switch (aim_status) {
        case E_BackgroundLightStatus::BG_LIGHT_ABNORMAL:
            return MarkerBGLightStatus::ABNORMAL;
        case E_BackgroundLightStatus::BG_LIGHT_OK:
        default:
            return MarkerBGLightStatus::OK;
    }
}

EdgeWarnings AimooeTracker::_from_aim_edge_warning(int aim_warning)
{
    switch (aim_warning) {
        case E_MarkWarnType::eWarn_Common:
            return EdgeWarnings::COMMON;
        case E_MarkWarnType::eWarn_Critical:
            return EdgeWarnings::CRITICAL;
        case E_MarkWarnType::eWarn_None:
        default:
            return EdgeWarnings::NONE;
    }
}

}