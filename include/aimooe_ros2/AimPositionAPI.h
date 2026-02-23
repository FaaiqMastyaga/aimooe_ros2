/*
* 
V2.3.5 rc Update time£º2024-06-05
API
1.Added the reference coordinate system function£º
Aim_SetReferenceTool
Aim_GetToolInfoInRef
Aim_CancelRef
Aim_GetReferenceInfo1to1
2.Locator is now compatible with NDI's .rom file (automatically creates a .aimtool with the same name based on the .rom)

V2.3.4-Beta Update time £º2023-12-5
API
1.Change the Aim_SetToolInfoFilePath to multi-path addition and use std::vector<char *> to add paths
2.Added a Aim_FindToolInfo_Map to return a map when looking for a tool, instead of traversing the linked list multiple times

V2.3.4 update time£º2023-6-7
API
1.Optimize USB and network port communication¡£
2.Add a tool coordinate conversion function, which can transfer the coordinate system of the tool to the coordinate system of the registration board.£º
Aim_InitToolCoordinateRenewWithToolId()
Aim_ProceedToolCoordinateRenew()
Aim_SaveToolCoordinateRenew()

V2.3.3 update time£º2022-10-10
API
1.New API function for rotating around a point to find tip information.£º
	Aim_InitToolTipPivotWithToolId()
	Aim_ProceedToolTipPivot()
	Aim_SaveToolTipPivot()
2.Added compatibility with mini version of optical cameras.
3.Change the Aim_API_Initial() and Aim_API_Close() function return types, and add two new function return values:
	When AimHandle is not NULL, the initialization function is called, which returns AIMOOE_INITIAL_FAIL
	Calling other API functions when AimHandle is NULL returns AIMOOE_HANDLE_IS_NULL
4.Lock all API functions to prevent errors when multiple API functions are called at the same time.
5.Resolved bugs in the progress display of tool making, calibration, tip registration, etc.
6.Solve the bug caused by Aim_CheckToolFile() function in some situations.
7.Solve the bug that there may be no connection when connecting to multiple cameras.
8.New mac address get function Aim_GetAimPositonMacAddress().
9.Added 2D point set to 3D point set calculation function Aim_Calculate3DPoints(), convenient for users to carry out in-depth development.
10.Solve the bug when registering the registration board.

V2.3.2 Update time:2022-4-8
1.Change the Aim_SetMarkerRoundnessThreshold() function to the Aim_SetMarkerParameters() function, which can set the thresholds for each parameter of the marker point recognition, including roundness, area, brightness, and so on. 
2.Add Aim_SetToolFindRTDirection() and Aim_GetToolFindRTDirection() functions.
3.Add Aim_FindSingleToolInfo() function and T_AimToolDataResultSingle structure.
4.Add Aim_SetToolFindPointMatchOptimizeEnable() and Aim_GetToolFindPointMatchOptimizeEnable() functions.
5.Fixed a bug that crashed when calling Aim_API_Close() too many times.
6.Solve the bug that the return values of three connection methods are inconsistent.
7.Add Aim_CheckToolFile() function. 
8.Add Aim_SetFlashDelay() function, which is available in firmware version V1.2.3 and above. 
9.Solve the bug that the output of rotation vector is not normal in some cases.

V2.3.1 Update time:2022-1-22
1. Add acquisition mode settings (default continuous acquisition mode, two single acquisition modes are only valid in instruments with firmware version V1.2.0 and above).
2.Two sets of data collection were established:
	2.1The API internal thread is used to get data from the instrument to put into the internal cache, and the API gets data from the internal cache.
	2.2API functions get data directly from the instrument. 
3.Add API function to set roundness judgment threshold. 
4.Optimize the retry mechanism of wifi communication. 
5.Add phantom point indication.
6.Add function to get current positioning camera IP. 

V2.3.0 Update time:2021-11-4
1.Changed the names of some interface functions, see Function Upgrade Name Comparison Table for details.
2.Removed some interface functions (the removed functions can be replaced by other functions)
3.The 2 tool finding functions and the spatial alignment function are upgraded by adding the parameter of the minimum number of matching points, which allows tool identification and spatial alignment to be realized even when the points are occluded. 

V2.2.4 Update time:2021-9-6
1.Optimize the judgment of multi-point tool recognition
2.Optimized calibration function

V2.2.3 Update time:2021-8-17
1.Optimize the registration of pinpoints
2.Optimize judgment criteria for tool identification

V2.2.2 Update time:2021-7-30
1.Optimize connectivity for network communications
2.Optimizing the handling of multipoint copolars

V2.2.1 Update time:2021-7-6
1.Optimize some of the annotations. 
2.Adjusts the limits of the threshold setting for the tool recognition function. 
3.Optimize the network communication mechanism by adding a retry mechanism.
4.Support for more than 4 points of tool recognition and registration.

V2.2.0 Update time:2021-6-15
1.Supports v1.1.0 firmware.
2.Add function to set binocular camera exposure value according to the distance used. 
3.Supports multiple tools of the same type to be recognized. 
4.Optimize the wifi communication mechanism.

V2.1.6 Update time:2021-5-26
1. Fix the problem that the serial number of the returned point set is abnormal when the tool is searching.
2. Optimize the function of AAK accuracy tool verification.
3. Optimize the function of adding tools to identify files.
4. Add the calibration initialization function for tools identified by ID.
5. Optimize the mechanism for border warnings.
6. Optimize the space matching function.
7. Modify the order of functions in the header file to make it more readable.

V2.1.5 Update time:2021-3-19
1.The instrumental error is defined as the mean error of the point spacing and update the display.
2. Optimize tool recognition algorithm.
3.USB and wired network data calibration optimized, positioning camera firmware updated.
4. Increase the tool to recognize the full data interface.
5. Add firmware version information

V2.1.4 Update time£º2020-12-2
1. add tool information acquisition function 
2. optimize image data acquired by wired network 
3. optimize tool tracking algorithm to match robot control 
4. add calculation of target position in robot base coordinates 
5. add input interface for robot calibration results

V2.1.3 Update time: 2020-9-30 
1. Add space registration function

V2.1.2 Update Time: 2020-9-24 
1.Add the function of tool recognizing files 
2.Add the function of accuracy test

V2.1.1
Update Time: 2020-8-26 
1.Exposure setting support multi-model camera AP-100/AP-200 
2.AimToolBox: optimize image display, optimize resource consumption and memory management

V2.1.0
Update Time: 2020-8-21 
1.Support multi-model camera AP-100/AP-200.

V1.4.5
Update: 2020-8-4
1. Add a wired network setting interface
2.dll adds the version suffix

V1.4.004
Update: 2020-8-3
1. Add a hint that the marker point has moved to the edge, which is reflected in the result of getting the marker point.
2. The self-calibration process uses a four-point svd algorithm and the calibration tip uses a three-point planar algorithm.
3. Tip calibration error is replaced by calculating the RMS of the distance from the tip to a certain point.

V1.4.003
Update time:2020-7-27
1. Fix the assignment of the four elements and the interface display
2. Optimize the calculation of RT algorithm, support three-point plane algorithm
3. Optimize the process and accuracy of tool identification calculations.

V1.4.002
Update time:2020-6-1
1. Optimize the tool registration algorithm 
2. Optimize the progress bar display effect in the tool registration interface 
3. Optimize memory management

V1.4.001
Update time:2020-3-27
1. Add tool self-calibration function and tool tip registration function.
2. Add library version reading function.
3. Optimize the tool recognition algorithm, optimize the tool data structure.
4.Optimization tool basic information acquisition structure

V1.3.001
Update time:2019-12-5
1. Increase the tool identification function, return the results of the relevant parameters identified by the tool

V1.2.3:
Update time:2019-10-30
1. Increase the synchronization frame of image acquisition to ensure image stability for long time operation. 

V1.2.2£º
Update time:2019-10-20
1. Improve the modification of the previous version, add the judgment when connecting to Ethernet.
2. Solve the read/write error when reconnecting several times. 

V1.2.1£º
Update time:2019-10-10
1. Modify the mechanism when acquiring information data, binocular image data, and color image data at the same time to solve the possible situation of frame stringing. 

V1.2.0£º
Update time:2019-9-27
1. Modify the API function architecture to integrate the API functions of different communication modes, which greatly reduces the number of API functions, specifically reflected in the following: USB, Ethernet, wifi three types of API functions into one.
2.Modify the API call method, e.g. you need to configure the data to be acquired by Aim_SetAcquireData function before acquiring the data, etc.c
3. When acquiring image data and information data at the same time, information data can also be acquired at a speed of 60 fps.
4. Reduce the delay time of information data acquisition (except for WIFI communication), and ensure that the delay time of information data is less than 1 frame (1/60th of a second). 

V1.1.1£º
Update time:2019-9-9
1. Optimize USB communication, so that it can reconnect correctly when exiting abnormally.
2. Optimize the output mechanism when the device is not updated.
3. Add the function of background brightness indication. 

V1.1.0£º
Update time:2019-9-2
1. Optimize the copolar line judgment condition of stereo matching, and retain the output of non-copolar line points.

V1.0.1£º
Update time:2019-7-11
1. Three camera commands have been added:
£¨1£©SC_AF_FIX_INFINITY£¬Used to control the intermediate color camera fixed focus to infinity.
£¨2£©SC_AF_EXP_AUTO_ON£¬Used to control the intermediate color camera to turn on auto exposure.
£¨3£©SC_AF_EXP_AUTO_OFF£¬Used to control the intermediate color camera to turn off auto exposure.
2.Modified the name of the original function to set the exposure value of the binocular camera and added a function to set the exposure value of the intermediate color camera. 

*/
#pragma once
#include"AimPositionDef.h"


/*------------------------------API FUNCTION---------------------------*/

/***********************************************************************/
/**
*	Description:
*		Call this function to initialize the API before calling other functions. 
*	Parameters:
*		aimHandle: initialization, the initial value to be assigned NULL, each system uses a AimHandle.
*	Return Value:
*		Null
*************************************************************************/
DLLExport E_ReturnValue Aim_API_Initial(AimHandle &aimHandle);

/************************************************************************/
/**
*	Description:
*		At the end of the program, call this function to close the API.
*	Parameters:
*		aimHandle:Handle to the current API to be closed. 
*	Return Value:
*		Null
*************************************************************************/
DLLExport E_ReturnValue Aim_API_Close(AimHandle &aimHandle);

/************************************************************************/
/**
*	Description:
*		Connect the device using the selected interface, each interface can be used at the same time. 
*	Parameters:
*		interfaceType: Specifies the communication interface selected.
         o_pospara:  Positioner model categories, image parameters
*	Return Value£º
*		AIMOOE_OK: The function was executed successfully. 
*		AIMOOE_CONNECT_ERROR : Device connection failed, please make sure the device is working properly. 
*		AIMOOE_READ_FAULT : Read device error, please reconnect the device or restart the device and try again.
*		AIMOOE_WRITE_FAULT : Write device error, please reconnect the device or restart the device and try again.
*		AIMOOE_ERROR: Unknown error, please make sure the function is called correctly. 
*************************************************************************/
DLLExport E_ReturnValue Aim_ConnectDevice(AimHandle aimHandle, E_Interface interfaceType, T_AIMPOS_DATAPARA &o_pospara);

/***********************************************************************/
/**
*	Description:
*		Get the software version number. 
*	Parameters:
*		Null
*	Return Value:
*		Null
*************************************************************************/
DLLExport char* Aim_API_GetLibVersion();

/************************************************************************/
/**
*	Description:
*		Set the camera acquisition mode: continuous, single active, single slave. 
*		This function is only valid for instruments with firmware version V1.2.0 and above.
*		Before calling this function, make sure that the acquired data set by Aim_SetAcquireData() is DT_NONE, which defaults to DT_NONE after the system is connected.
*		This function is only used under USB and Ethernet communication. 
*	Parameters:
*		interfaceType: Specifies the communication interface selected. 
*		mode: Acquisition mode selection. 
*	Return Value:
*		AIMOOE_OK: The function was executed successfully. 
*		AIMOOE_NOT_CONNECT: The device is not connected, please connect the device first.
*		AIMOOE_READ_FAULT: Read device error, please reconnect the device or restart the device and try again. 
*		AIMOOE_WRITE_FAULT: Write device error, please reconnect the device or restart the device and try again. 
*		AIMOOE_ERROR: Unknown error, please make sure the function is called correctly.
*  Caution:
*   1. The power-on default is continuous acquisition mode. This function does not belong to the memory type parameter setting function, and its set value will not be saved in the camera. 
*		Therefore, the factory settings are automatically restored after restarting the instrument.
*   2.When selecting the single acquisition mode, you need to select the corresponding master-slave mode according to different machines. 
*	3. Before calling this function, make sure that the acquired data set by Aim_SetAcquireData() is DT_NONE, which defaults to DT_NONE after system connection.
*	4. This function cannot be used under wifi mode.
*************************************************************************/
DLLExport E_ReturnValue Aim_SetAcquireMode(AimHandle aimHandle, E_Interface interfaceType, E_AcquireMode mode);

/************************************************************************/
/**
*	Description:
*		Obtain marker point coordinates and system status information from the hardware system. 
*		This function ensures that the acquired data set by Aim_SetAcquireData() is DT_NONE before it is called, and defaults to DT_NONE after the system is connected.
*	Parameters:
*		interfaceType: Specifies the communication interface selected.
*		markerSt: Stores the returned 3D coordinate information data. 
*		statusSt: Stores the returned 3D coordinate information data. 
*	Return Value:
*		AIMOOE_OK: The function was executed successfully. 
*		AIMOOE_NOT_CONNECT: The device is not connected, please connect the device first.
*		AIMOOE_READ_FAULT: Read device error, please reconnect the device or restart the device and try again. 
*		AIMOOE_WRITE_FAULT: Write device error, please reconnect the device or restart the device and try again.
*		AIMOOE_ERROR: Unknown error, please make sure the function is called correctly. 
*  Caution:
*      1.This function ensures that the acquired data set by Aim_SetAcquireData() is DT_NONE before it is called, and defaults to DT_NONE after the system is connected.
*************************************************************************/
DLLExport E_ReturnValue Aim_GetMarkerAndStatusFromHardware(AimHandle aimHandle, E_Interface interfaceType, T_MarkerInfo & markerSt, T_AimPosStatusInfo& statusSt);

/************************************************************************/
/**
*	Description:
*		Obtain marker point coordinates, system status and binocular image information from the hardware system. 
*		This function ensures that the acquired data set by Aim_SetAcquireData() is DT_NONE before it is called, and defaults to DT_NONE after the system is connected.
*		This function is only used under USB and Ethernet communication.
*	Parameters:
*		interfaceType: Specifies the communication interface selected.
*		markerSt: Stores the returned 3D coordinate information data. 
*		statusSt: Stores returned system status message data. 
*		imageL: Refers to the address that stores the left image data, which needs to be defined by the user, and the image size
*		AP-100:1280*720*1£¬AP-200:1280*1240*1 1 byte per pixel, pixel refresh direction from left to right, top to bottom.
*		imageR: Refers to the address that stores the right image data, which needs to be defined by the user, and the image size

*		AP-100:1280*720*1£¬AP-200:1280*1240*1 1 byte per pixel, pixel refresh direction from left to right, top to bottom. 
*	Return Value:
*		AIMOOE_OK: The function was executed successfully. 
*		AIMOOE_NOT_CONNECT: The device is not connected, please connect the device first. 
*		AIMOOE_READ_FAULT: Read device error, please reconnect the device or restart the device and try again.
*		AIMOOE_WRITE_FAULT: Write device error, please reconnect the device or restart the device and try again. 
*		AIMOOE_ERROR: Unknown error, please make sure the function is called correctly. 
*  Caution:
*      1. Before calling this function, make sure that the acquired data set by Aim_SetAcquireData() is DT_NONE, which defaults to DT_NONE after system connection.
*		2. This function is only valid for USB and Ethernet communication. 
*************************************************************************/
DLLExport E_ReturnValue Aim_GetMarkerStatusAndGreyImageFromHardware(AimHandle aimHandle, E_Interface interfaceType, T_MarkerInfo & markerSt, T_AimPosStatusInfo& statusSt, char * imageL, char * imageR);

/************************************************************************/
/**
*	Description:
*		Acquire intermediate camera color images from hardware systems. 
*		This function ensures that the acquired data set by Aim_SetAcquireData() is DT_NONE before it is called, and defaults to DT_NONE after the system is connected.
*		This function is only used under USB and Ethernet communication. 
*	Parameters:
*		interfaceType: Specifies the communication interface selected. 
*		imageC: Refers to the address where the color image data is stored. The image size needs to be defined by the user.
*		1280*720*2£¬2 bytes per pixel (RGB565, low in front, high in back, high to low)
*		( R->G->B), the pixel refresh direction is from left to right, top to bottom.
*	Return Value:
*		AIMOOE_OK: The function was executed successfully. 
*		AIMOOE_NOT_CONNECT: The device is not connected, please connect the device first. 
*		AIMOOE_READ_FAULT: Error reading device. Please reconnect or restart the device and try again.
*		AIMOOE_WRITE_FAULT: Error reading device. Please reconnect or restart the device and try again.
*		AIMOOE_NOT_REFLASH: If the data  be acquired by camera has been correctly set by Aim_SetAcquireData function, the image data is not update
*		AIMOOE_ERROR: Unknown error, please confirm that the function call is correct.
*   Caution:
*      1.Before calling the function, make sure the data set by Aim_SetAcquireData() is DT_NONE. After connecting the default is DT_NONE
*		2.This function is only valid under USB and Ethernet communication.
*************************************************************************/
DLLExport E_ReturnValue Aim_GetColorImageFromHardware(AimHandle aimHandle, E_Interface interfaceType, char* imageC);

/************************************************************************/
/**
*	Description:
*		Read the latest intermediate camera color image from the API internal storage.
*	Parameters:
*		interfaceType: Specifies the communication interface to be used.
*		imageC: Points to the address where the color image data is stored. The image size needs to be defined by the user.
*		1280*720*2£¬Each pixel has 2 bytes (RGB565, low bit first, high bit last, from high to low
*		ÎªR->G->B£©£¬The pixel refresh direction is from left to right and from top down
*	Return Value:
*		AIMOOE_OK:The function executed successfully.
*		AIMOOE_NOT_CONNECT: The device is not connected. Please connect the device first.
*		AIMOOE_READ_FAULT: Error reading device. Please reconnect or restart the device and try again.
*		AIMOOE_WRITE_FAULT: Error writing to device, please reconnect the device or restart the device and try again.
*		AIMOOE_NOT_REFLASH: If the data  be acquired by camera has been correctly set by Aim_SetAcquireData function, the image data is not update
*		AIMOOE_ERROR: Unknown error, please confirm that the function call is correct.
*************************************************************************/
DLLExport E_ReturnValue Aim_GetColorImageMiddle(AimHandle aimHandle, E_Interface interfaceType, char* imageC);


/**
*	Description:
*		Acquire intermediate camera Grey images from hardware systems.
*		This function ensures that the acquired data set by Aim_SetAcquireData() is DT_NONE before it is called, and defaults to DT_NONE after the system is connected.
*		This function is only used under USB and Ethernet communication.
*	Parameters:
*		interfaceType: Specifies the communication interface selected.
*		imageL: Refers to the address where the Grey image data is stored. The image size needs to be defined by the user.
* 		imageR: Refers to the address where the Grey image data is stored. The image size needs to be defined by the user.

*	Return Value:
*		AIMOOE_OK: The function was executed successfully.
*		AIMOOE_NOT_CONNECT: The device is not connected, please connect the device first.
*		AIMOOE_READ_FAULT: Error reading device. Please reconnect or restart the device and try again.
*		AIMOOE_WRITE_FAULT: Error reading device. Please reconnect or restart the device and try again.
*		AIMOOE_NOT_REFLASH: If the data  be acquired by camera has been correctly set by Aim_SetAcquireData function, the image data is not update
*		AIMOOE_ERROR: Unknown error, please confirm that the function call is correct.
*   Caution:
*      1.Before calling the function, make sure the data set by Aim_SetAcquireData() is DT_NONE. After connecting the default is DT_NONE
*		2.This function is only valid under USB and Ethernet communication.
*************************************************************************/
DLLExport E_ReturnValue Aim_GetGreyImageFromHardware(AimHandle aimHandle, E_Interface interfaceType, char* imageL, char* imageR);

/************************************************************************/
/**
*	Description:
*		Read the latest left and right camera grayscale images from the API internal storage space.
*	Parameters:
*		interfaceType: Specifies the communication interface to be used.
*		imageL: Points to the address where the left image data is stored. The user needs to define the image size.
*		AP-200:1280*1240*1 Each pixel is 1 byte, and the pixel refresh direction is from left to right and from top down.
*		imageR: Points to the address where the right image data is stored. The user needs to define the image size.
*		AP-200:1280*1240*1 Each pixel is 1 byte, and the pixel refresh direction is from left to right and from top down.
*	Return Value:
*		AIMOOE_OK:The function executed successfully.
*		AIMOOE_NOT_CONNECT: The device is not connected. Please connect the device first.
*		AIMOOE_READ_FAULT: Error reading device. Please reconnect or restart the device and try again.
*		AIMOOE_WRITE_FAULT: Error writing to device, please reconnect the device or restart the device and try again.
*		AIMOOE_NOT_REFLASH: If the data  be acquired by camera has been correctly set by Aim_SetAcquireData function, the image data is not update
*		AIMOOE_ERROR: Unknown error, please confirm that the function call is correct.
*************************************************************************/
DLLExport E_ReturnValue Aim_GetGreyImageDual(AimHandle aimHandle, E_Interface interfaceType, char* imageL, char* imageR);

/************************************************************************/
/**
*	Description:
*		Set the data obtained from the camera by the API internal thread. This function must be called before calling the following get function:
*		Aim_GetMarkerInfo()
*		Aim_GetStatusInfo()
*		Aim_GetGreyImageDual()
*		Aim_GetColorImageMiddle()
*	Parameters:
*		interfaceType: Specifies the communication interface to be used.
*		dataType: Make sure the API reads data from the specified communication interface.
*	Return Value:
*		AIMOOE_OK:The function executed successfully.
*		AIMOOE_NOT_CONNECT: The device is not connected. Please connect the device first.
*		AIMOOE_ERROR: Unknown error, please confirm that the function call is correct.
*   Caution:
*      1.This function is valid for a long time after called. When reconnecting a read/write error occurs, it should be called again.
*      2.Make sure that the data acquired by the camera contains the data read, otherwise the Get function will return the result of AIMOOE_NOT_REFLASH.
*      3.This function does not belong to the memory parameter setting function. The value set by this function will not be saved in the camera. After restarting the instrument, the default value DT_NONE will be automatically restored.
*************************************************************************/
DLLExport E_ReturnValue Aim_SetAcquireData(AimHandle aimHandle, E_Interface interfaceType, E_DataType dataType);


/************************************************************************/
/**
*	Description:
*		Read the latest 3D coordinate information of the marker from the API internal storage space.
*	Parameters:
*		interfaceType: Specifies the communication interface to be used.
*		markerSt: Store the returned 3D coordinate information data.
*	Return Value:
*		AIMOOE_OK:The function executed successfully.
*		AIMOOE_NOT_CONNECT: The device is not connected. Please connect the device first.
*		AIMOOE_READ_FAULT: Error reading device. Please reconnect or restart the device and try again.
*		AIMOOE_WRITE_FAULT: Error writing to device, please reconnect the device or restart the device and try again.
*		AIMOOE_NOT_REFLASH: The coordinate information is not updated. At this time, the number of 3D points in markerSt is 0. Please confirm that the data to be acquired by the camera has been correctly set through the Aim_SetAcquireData() function.
*		AIMOOE_ERROR: Unknown error, please confirm that the function call is correct.
*   Caution:
*      After connecting to the system, please call Aim_SetAcquireData() function first, and then call this function.
*************************************************************************/
DLLExport E_ReturnValue Aim_GetMarkerInfo(AimHandle aimHandle, E_Interface interfaceType, T_MarkerInfo & markerSt);

/************************************************************************/
/**
*	Description:
*		Read system status information.
*	Parameters:
*		interfaceType: Specifies the communication interface to be used.
*		statusSt: Stores the returned system status information data.
*	Return Value:
*		AIMOOE_OK:The function executed successfully.
*		AIMOOE_NOT_CONNECT: The device is not connected. Please connect the device first.
*		AIMOOE_READ_FAULT: Error reading device. Please reconnect or restart the device and try again.
*		AIMOOE_WRITE_FAULT: Error writing to device, please reconnect the device or restart the device and try again.
*		AIMOOE_NOT_REFLASH: If the data to be acquired by the camera has been correctly set by the Aim_SetAcquireData function, the system status information is not updated.
*		AIMOOE_ERROR: Unknown error, please confirm that the function call is correct.
*************************************************************************/
DLLExport E_ReturnValue Aim_GetStatusInfo(AimHandle aimHandle, E_Interface interfaceType, T_AimPosStatusInfo & statusSt);

/************************************************************************/
/**
*	Description:
*		Read the camera factory information.
*	Parameters:
*		interfaceType: Specifies the communication interface to be used.
*		manufactureInfo: Store the returned factory information data.
*	Return Value:
*		AIMOOE_OK:The function executed successfully.
*		AIMOOE_NOT_CONNECT: The device is not connected. Please connect the device first.
*		AIMOOE_ERROR: Unknown error, please confirm that the function call is correct.
*************************************************************************/
DLLExport E_ReturnValue Aim_GetManufactureInfo(AimHandle aimHandle, E_Interface interfaceType, T_ManufactureInfo & manufactureInfo);

/************************************************************************/
/**
*	Description:
*		Issue camera system instructions.
*	Parameters:
*		interfaceType: Specifies the communication interface to be used.
*		com: The command value to be issued.
*	Return Value:
*		AIMOOE_OK:The function executed successfully.
*		AIMOOE_NOT_CONNECT: The device is not connected. Please connect the device first.
*		AIMOOE_WRITE_FAULT : Error writing to device, please reconnect the device or restart the device and try again.
*		AIMOOE_ERROR: Unknown error, please confirm that the function call is correct.
*************************************************************************/
DLLExport E_ReturnValue Aim_SetSystemCommand(AimHandle aimHandle, E_Interface interfaceType, E_SystemCommand com);

/************************************************************************/
/**
*	Description:
*		Set the exposure value of the left and right cameras (the last set value is used by default when the camera is turned on). Range:
*		16-16384 (ie: 0x10-0x4000), unit is us, the minimum resolution is 16us.
*		The setting of this exposure value will affect the acquisition accuracy, please do not modify it at will.
*	Parameters:
*		interfaceType: Specifies the communication interface to be used.
*		expTime: The exposure value of the binocular left and right cameras to be set, range: 16-16384 (ie: 0x10-0x4000),
*		The unit is us, and the minimum resolution is 16us.

*	Return Value:
*		AIMOOE_OK:The function executed successfully.
*		AIMOOE_NOT_CONNECT: The device is not connected. Please connect the device first.
*		AIMOOE_WRITE_FAULT: Error writing to device, please reconnect the device or restart the device and try again.
*		AIMOOE_ERROR: Unknown error, please confirm that the function call is correct.
*	 Caution:
*		1. This function belongs to the memory parameter setting function. The set value will be saved in the camera and will be restored the next time it is opened.
*		It will be used when locating the instrument.
*		2.Since the memory parameter setting function needs to wait for the camera to save the data, after this function is called,
*		A delay of more than 0.5s is required before calling the next memory parameter setting function.
*************************************************************************/
DLLExport E_ReturnValue Aim_SetDualExpTime(AimHandle aimHandle, E_Interface interfaceType, int expTime);

/************************************************************************/
/**
*	Description:
*		Set the exposure value of the binocular camera according to the use distance.
*	Parameters:
*		interfaceType: Specifies the communication interface to be used.
*		distanceInMM: Use distance, in millimeters
*	Return Value:
*		AIMOOE_OK:The function executed successfully.
*		AIMOOE_NOT_CONNECT: The device is not connected. Please connect the device first.
*		AIMOOE_WRITE_FAULT: Error writing to device, please reconnect the device or restart the device and try again.
*		AIMOOE_ERROR: Unknown error, please confirm that the function call is correct.
*	 Caution:
*		1.This function belongs to the memory parameter setting function. The set value will be saved in the camera and will be restored
*		The settings will be used next time you turn on the camera.
*		2.Since the memory parameter setting function needs to wait for the camera to save the data, after this function is called,
*		A delay of more than 0.5s is required before calling the next memory parameter setting function.
*************************************************************************/
DLLExport E_ReturnValue Aim_SetDualExpTimeByDistance(AimHandle aimHandle, E_Interface interfaceType, int distanceInMM);

/************************************************************************/
/**
*	Description:
*		Set the delay between the flash and the exposure time. Including start delay and shut down delay.
*		When using the API normally, there is no need to modify the delay parameters.
*		This function is only used in USB and Ethernet communication.
*	Parameters:
*		interfaceType: Specifies the communication interface to be used.
*		flashOnDelay: Delay time from flash start to camera exposure start = flashOnDelay*125us, value range of flashOnDelay 0-255.
*		flashOffDelay: Delay time from flash off to camera exposure off = flashOffDelay*125us, flashOffDelay value range 0-255.
*								Also, the delay time from flash off to camera exposure off must be less than the exposure time.
*	Return Value:
*		AIMOOE_OK:The function executed successfully.
*		AIMOOE_NOT_CONNECT: The device is not connected. Please connect the device first.
*		AIMOOE_WRITE_FAULT: Error writing to device, please reconnect the device or restart the device and try again.
*		AIMOOE_ERROR: Unknown error, please confirm that the function call is correct.
*	 Caution:
*		1.This function does not belong to the memory type parameter setting function, and its set value will not be saved in the positioner, and the default value will be restored automatically after restarting the instrument. 
*		2.This function cannot be used under wifi.
*************************************************************************/
DLLExport E_ReturnValue Aim_SetFlashDelay(AimHandle aimHandle, E_Interface interfaceType, int flashOnDelay, int flashOffDelay);

/************************************************************************/
/**
*	Description:
*		Sets the exposure value for the intermediate color camera, range: 48-8192 (i.e.: 0x30-0x2000).
*		This setting will not be effective if the color camera auto exposure is turned on (turned on by default at power on).
*	Parameters:
*		interfaceType: Specifies the communication interface to be used.
*		expTimeAF: Intermediate color camera exposure value to be set, range: 48-8192 (i.e., 0x30-
*		0x2000)£¬The unit is: transmission time for 1 line of image/16.
*	Return Value:
*		AIMOOE_OK:The function executed successfully.
*		AIMOOE_NOT_CONNECT: The device is not connected. Please connect the device first.
*		AIMOOE_WRITE_FAULT: Error writing to device, please reconnect the device or restart the device and try again.
*		AIMOOE_ERROR: Unknown error, please confirm that the function call is correct.
*	 Caution:
*		1.The exposure value set by this function will not be saved in the camera because the power-up default turns on the automatic exposure of the intermediate color camera. 
*************************************************************************/
DLLExport E_ReturnValue Aim_SetColorExpTime(AimHandle aimHandle, E_Interface interfaceType, int expTimeAF);

/************************************************************************/
/**
*	Description:
*		Set the collision detection sensitivity, range: 1-10, 1 sensitivity is the highest, 10 sensitivity is the lowest. 
*	Parameters:
*		interfaceType: Specifies the communication interface to be used.
*		level: Sensitivity parameter, the lower the value, the more sensitive. 
*	Return Value:
*		AIMOOE_OK:The function executed successfully.
*		AIMOOE_NOT_CONNECT: The device is not connected. Please connect the device first.
*		AIMOOE_WRITE_FAULT: Error writing to device, please reconnect the device or restart the device and try again.
*		AIMOOE_ERROR: Unknown error, please confirm that the function call is correct.
*	 Caution:
*		1.This function belongs to the memory parameter setting function. The set value will be saved in the camera and will be restored
*		The settings will be used next time you turn on the camera.
*		2.Since the memory parameter setting function needs to wait for the camera to save the data, after this function is called,
*		A delay of more than 0.5s is required before calling the next memory parameter setting function.
*************************************************************************/
DLLExport E_ReturnValue Aim_SetCollisinoDetectLevel(AimHandle aimHandle, E_Interface interfaceType, UCHAR level);


/************************************************************************/
/**
*	Description:
*		Set the IP of the camera (factory default is 192.168.31.10), restart the camera after setting is valid. 
*		Currently, only the IP setting of the camera is supported under USB and network port connection. 
*	Parameters:
*		interfaceType: Specifies the communication interface to be used.
*		IP_A¡¢IP_B¡¢IP_C¡¢IP_D: The four fields A.B.C.D of the IP address to be set, each field
*		Occupies one byte.
*	Return Value:
*		AIMOOE_OK:The function executed successfully.
*		AIMOOE_NOT_CONNECT: The device is not connected. Please connect the device first.
*		AIMOOE_WRITE_FAULT: Error writing to device, please reconnect the device or restart the device and try again.
*		AIMOOE_ERROR: Unknown error, please confirm that the function call is correct.
*	 Caution:
*		1.This function belongs to the memory parameter setting function. The set value will be saved in the camera and will be restored
*		The settings will be used next time you turn on the camera.
*		2.Since the memory parameter setting function needs to wait for the camera to save the data, after this function is called,
*		A delay of more than 0.5s is required before calling the next memory parameter setting function.
*		3.This function cannot be used under wifi.
*************************************************************************/
DLLExport E_ReturnValue Aim_SetAimPositionIP(AimHandle aimHandle, E_Interface interfaceType, UCHAR IP_A, UCHAR IP_B, UCHAR IP_C, UCHAR IP_D);
/************************************************************************/
/**
*	Description:
*		Get the IP currently used by the camera.
*		Available after successful connection in any way.
*	Parameters:
*		interfaceType: Specifies the communication interface to be used.
*		IP_A¡¢IP_B¡¢IP_C¡¢IP_D: The four fields A.B.C.D of the IP address to be connected, each field
*		Occupies one byte. 
*	Return Value:
*		AIMOOE_OK:The function executed successfully.
*		AIMOOE_NOT_CONNECT: The device is not connected. Please connect the device first.
*		AIMOOE_ERROR: Unknown error, please confirm that the function call is correct.
*************************************************************************/
DLLExport E_ReturnValue Aim_GetAimPositionIP(AimHandle aimHandle, E_Interface interfaceType, UCHAR &IP_A, UCHAR &IP_B, UCHAR &IP_C, UCHAR &IP_D);

/************************************************************************/
/**
*	Description:
*		For Ethernet connection, set the IP of the locator to be connected, which needs to be the same as the IP of the locator set.
*	Parameters:
*		IP_A¡¢IP_B¡¢IP_C¡¢IP_D: The four fields A.B.C.D of the IP address to be connected, each field
*		Occupies one byte. 
*	Return Value:
*		AIMOOE_OK:The function executed successfully.
*		AIMOOE_ERROR: Unknown error, please confirm that the function call is correct.
*	 Caution:
*		1.This function needs to be used before the Aim_ConnectDevice() function is called. 
*		2.This function does not need to be called if the camera IP is using the factory default settings (192.168.31.10). 
*************************************************************************/
DLLExport E_ReturnValue Aim_SetEthernetConnectIP(AimHandle aimHandle, UCHAR IP_A, UCHAR IP_B, UCHAR IP_C, UCHAR IP_D);
/************************************************************************/
/**
*	Description:
*		Gets the currently set IP of the locator to be connected, which is not equal to the IP used by the current locator.
*	Parameters:
*		IP_A¡¢IP_B¡¢IP_C¡¢IP_D: The four fields A.B.C.D of the IP address to be connected, each field
*		Occupies one byte. 
*	Return Value:
*		AIMOOE_OK:The function executed successfully.
*		AIMOOE_ERROR: Unknown error, please confirm that the function call is correct.
*	 Caution:
*		1.This IP is not equivalent to the IP currently used by the locator.
*************************************************************************/
DLLExport E_ReturnValue Aim_GetEthernetConnectIP(AimHandle aimHandle, UCHAR &IP_A, UCHAR &IP_B, UCHAR &IP_C, UCHAR &IP_D);

/************************************************************************/
/**
*	Description:
*		Sets whether the locator LCD screen displays unprocessed points. 
*		The factory default is not to display unprocessed points, i.e., only points from the left and right cameras that are a match are displayed. 
*		This function is only used in USB and Ethernet communication.
*	Parameters:
*		isRawPointShow: true displays unprocessed points; false displays only processed points.
*   Return Value:
*		AIMOOE_OK:The function executed successfully.
*		AIMOOE_NOT_CONNECT: The device is not connected. Please connect the device first.
*		AIMOOE_WRITE_FAULT: Error writing to device, please reconnect the device or restart the device and try again.
*		AIMOOE_ERROR: Unknown error, please confirm that the function call is correct.
*	 Caution:
*		1.This function belongs to the memory parameter setting function. The set value will be saved in the camera and will be restored
*		The settings will be used next time you turn on the camera.
*		2.Since the memory parameter setting function needs to wait for the camera to save the data, after this function is called,
*		A delay of more than 0.5s is required before calling the next memory parameter setting function.
*		3.This function cannot be used under wifi.
*************************************************************************/
DLLExport E_ReturnValue Aim_SetLCDShowRawPoint(AimHandle aimHandle, E_Interface interfaceType, bool  isRawPointShow = false);

/************************************************************************/
/**
*	Description:
*		Sets the parameter threshold for marker point recognition, which is generally not recommended to be modified.
*		This function is only used in USB and Ethernet communication.
*	Parameters:
*		minRoundness: Roundness threshold, range: 0-100;
*		maxArea: Area threshold, range: 5-4000;
*		minBrightness: Brightness threshold, range: 1-255;
*   Return Value:
*		AIMOOE_OK:The function executed successfully.
*		AIMOOE_NOT_CONNECT: The device is not connected. Please connect the device first.
*		AIMOOE_WRITE_FAULT: Error writing to device, please reconnect the device or restart the device and try again.
*		AIMOOE_ERROR: Unknown error, please confirm that the function call is correct.
*	 Caution:
*		1.This function is not a memory type parameter setting function and the value it sets will not be saved in the camera.
*		Therefore, the factory settings are automatically restored after restarting the instrument. 
*		2.This function cannot be used under wifi.
*************************************************************************/
DLLExport E_ReturnValue Aim_SetMarkerParameters(AimHandle aimHandle, E_Interface interfaceType,
	int minRoundness = 75, int maxRoundness = 100,int minArea = 5, int maxArea = 1000, int minBrightness = 80);

/************************************************************************/
/**
*	Description:
*		Set the maximum permissible deviation of point error for tool recognition in mm.
*	Parameters:
*		offset: Maximum permissible deviation of the point error when recognizing the tool, range: 0.3-3.0.
*   Return Value:
*		AIMOOE_OK:The function executed successfully.
*		AIMOOE_ERROR: Unknown error, please confirm that the function call is correct.
*	 Caution:
*      1.This function does not belong to the memory type parameter setting function, its set value will not be saved in the positioner, and the default value of 1.5 will be restored automatically after restarting the instrument.
*************************************************************************/
DLLExport E_ReturnValue Aim_SetToolFindOffset(AimHandle aimHandle, float offset = 1.5f);

/************************************************************************/
/**
*	Description:
*		Get the maximum permissible deviation of the point error of the current tool recognition, in mm.
*	Parameters:
*		offset: Returns the current maximum allowable deviation, range: 0.3-3.0.
*   Return Value:
*		AIMOOE_OK:The function executed successfully.
*		AIMOOE_ERROR: Unknown error, please confirm that the function call is correct.
*************************************************************************/
DLLExport E_ReturnValue Aim_GetToolFindOffset(AimHandle aimHandle, float &offset);

/************************************************************************/
/**
*	Description:
*		Set whether or not point matching error optimization is activated when the tool is recognized, it is activated by default.
*		At startup, an error judgment is made on the matched points, and when the error of a point is larger than that of other points, the point is automatically filtered out when the RT matrix is calculated. 
*	Parameters:
*		en: true to start, false to not start.
*   Return Value:
*		AIMOOE_OK:The function executed successfully.
*		AIMOOE_ERROR: Unknown error, please confirm that the function call is correct.
*	 Caution:
*      1.This function does not belong to the memory type parameter setting function, and its set value will not be saved in the positioner, and the default value will be restored automatically after restarting the instrument. 
*************************************************************************/
DLLExport E_ReturnValue Aim_SetToolFindPointMatchOptimizeEnable(AimHandle aimHandle,  bool en= true);

/************************************************************************/
/**
*	Description:
*		Whether or not point matching error optimization is initiated when obtaining the current tool identification.
*		At startup, an error judgment is made on the matched points, and when the error of a point is larger than that of other points,This point is automatically filtered out when calculating the RT matrix.
*	Parameters:
*		en: Return value, true for startup, false for no startup.
*   Return Value:
*		AIMOOE_OK:The function executed successfully.
*		AIMOOE_ERROR: Unknown error, please confirm that the function call is correct.
*************************************************************************/
DLLExport E_ReturnValue Aim_GetToolFindPointMatchOptimizeEnable(AimHandle aimHandle, bool &en);

/************************************************************************/
/**
*	Description:
*		Reads and checks if the tool file data is correct, the read data is stored in toolData. 
*	Parameters:
*		filePathFullName: Tool file path + name + suffix. 
*		toolData£ºReturns the tool file data read.
*	Return Value:
*		AIMOOE_OK:The function executed successfully.
*		AIMOOE_ERROR: The tool file is incorrect. 
*************************************************************************/
DLLExport E_ReturnValue Aim_CheckToolFile(AimHandle aimHandle,const char * filePathFullName, T_ToolFileData &toolData);


/************************************************************************/
/**
*	Description:
*		Set the path of the tool file, the system will traverse the information of all tool files under this path. 
*		When this function is called, the original tool file information is automatically cleared first. 
*		This function does not need to be called multiple times. It is recommended to call this function once after calling the Aim_ConnectDevice() connection function. 
*	Parameters:
*		path: Path parameters. 
*	Return Value:
*		AIMOOE_OK:The function executed successfully.
*		AIMOOE_ERROR: The path is empty or there is no tool file under the path. 
*   Caution:
*		When calling this function, do not call tool-related functions (e.g., tool lookup functions) in other threads, as this may cause the code to crash! 
*************************************************************************/
DLLExport E_ReturnValue Aim_SetToolInfoFilePath(AimHandle aimHandle, const char * path, bool clearBefore=true);

/************************************************************************/
/**
*	Description£º
*		Add multiple tool file information in the format of "Tool Path/Tool File"
*		When this function is called, the clearBefore of the Aim_SetToolInfoFilePath must be "false"
*	Parameters£º
*		path: Path parameters.
*	Return Value:
*		AIMOOE_OK: The function is executed successfully.
*		AIMOOE_ERROR:The path is empty or there is no tool file under the path.
*  Caution:
*		When calling this function, do not call tool-related functions (e.g., tool lookup functions) in other threads, as this may cause the code to crash!
*************************************************************************/
DLLExport E_ReturnValue Aim_AddToolInfoFile(AimHandle aimHandle, char* path);

/************************************************************************/
/**
*	Description:
*		Gets the path to the currently set tool file. 
*	Return Value:
*		const char * path parameter
*************************************************************************/
DLLExport const char * Aim_GetToolInfoFilePath(AimHandle aimHandle);
/**
*	Description:
*		Recognizes the Parameters:Marker points in the selected tool file and the coordinates in the tool file by their IDs.
*	Parameters:
*		ptoolid: Tool identification number. 
*		marksize: The number of marker points in the tool file. 
*		toolsysinfo:Marker point datasets in tool files
*	Return Value:
*		AIMOOE_OK:The function executed successfully.
*		AIMOOE_ERROR: Unknown error, please confirm that the function call is correct.
*************************************************************************/
DLLExport E_ReturnValue Aim_GetSpecificToolFileInfoList(AimHandle aimHandle, const char *ptoolid, int &marksize, std::list<float>*toolsysinfo);
/************************************************************************/
/**
*	Description:
*		Recognizes the Parameters:Marker points in the selected tool file and the coordinates in the tool file by their IDs.
*	Parameters:
*		ptoolid: Tool identification number. 
*		marksize: The number of marker points in the tool file. 
*		toolsysinfo:Marker point datasets in tool files
*	Return Value:
*		AIMOOE_OK:The function executed successfully.
*		AIMOOE_ERROR: Unknown error, please confirm that the function call is correct.
*************************************************************************/
DLLExport E_ReturnValue Aim_GetSpecificToolFileInfoArray(AimHandle aimHandle, const char* ptoolid, int& markersize, float* toolsysinfo);
/************************************************************************/
/**
*	Description:
*		Gets the number of tool files under the selected path. 
*	Parameters:
*		size: Quantity.
*	Return Value:
*		AIMOOE_OK:The function executed successfully.
*		AIMOOE_ERROR: Unknown error, please confirm that the function call is correct.
*************************************************************************/
DLLExport E_ReturnValue Aim_GetCountOfToolInfo(AimHandle aimHandle, int &size);
/************************************************************************/
/**
*	Description:
*		Get the basic information of all the tool files under the selected path, you can first call the Aim_GetCountOfToolInfo() function to get the number of tool files and initialize the parameter array.
*	Parameters:
*		ptools: Returns an array of structures with basic information about the tool, including name, type and number of markers.
*	Return Value:
*		AIMOOE_OK:The function executed successfully.
*		AIMOOE_ERROR: Unknown error, please confirm that the function call is correct.
*************************************************************************/
DLLExport E_ReturnValue Aim_GetAllToolFilesBaseInfo(AimHandle aimHandle, t_ToolBaseInfo *ptools);

/************************************************************************/
/**
*	Description:
*		All tools under the selected path are recognized by inputting 3D data. 
*		Prior to each call to this function, you can pass Aim_GetMarkerInfo() or the
*		Aim_GetMarkerAndStatusFromHardware()function to get the latest 3D data information. 
*		It is recommended to use Aim_FindSpecificToolInfo() instead of this function, which returns more comprehensive data
*	Parameters:
*		marker: Three-dimensional data. 
*		pResultList: Returns the data that holds the information about the found tool. 
*		minimumMatchPts:The minimum number of match points (¡Ý3) that can be allowed when the multipoint tool is performing recognition. The default value of 0 indicates a full match. 
*	Return Value:
*		AIMOOE_OK:The function executed successfully.
*		AIMOOE_ERROR: Failure to recognize tools.
*************************************************************************/
DLLExport E_ReturnValue Aim_FindToolInfo(AimHandle aimHandle, T_MarkerInfo & marker, T_AimToolDataResult*pResultList, int minimumMatchPts =0);
/************************************************************************/
/**
*	Description:
*		By inputting 3D data and tool ID information (can be more than one), the corresponding tool (can be more than one) is recognized. 
*		Before each call to this function, it can be passed through Aim_GetMarkerInfo() or Aim_GetMarkerAndStatusFromHardware()
*		function to get the latest 3D data information. 
*	Parameters:
*		marker: 3D data list
*		toolids:list of IDs recognized by the tool
*		pResultList: Returns the dataset that holds the found tool information. 
*		minimumMatchPts:The minimum number of match points (¡Ý3) that can be allowed when the multipoint tool is performing recognition.The default value of 0 indicates a full match.
*	Return Value:
*		AIMOOE_OK:The function executed successfully.
*		AIMOOE_ERROR: Failure to recognize tools.
*************************************************************************/
DLLExport E_ReturnValue Aim_FindSpecificToolInfo(AimHandle aimHandle, T_MarkerInfo & marker,
	std::vector<std::string>&toolids, T_AimToolDataResult*pResultList, int minimumMatchPts=0);

/************************************************************************/
/**
*	Description:
*		The tool is identified by entering 3D data and ID information for the individual tool.
*		Before each call to this function, it can be passed through Aim_GetMarkerInfo() or Aim_GetMarkerAndStatusFromHardware()
*		function to get the latest 3D data information.
*	Parameters:
*		marker: 3D data list
*		toolids:ID of the tool to be recognized
*		dataResult: Returns the dataset that holds the found tool information. 
*		minimumMatchPts:The minimum number of match points (¡Ý3) that can be allowed when the multipoint tool is performing recognition.The default value of 0 indicates a full match.
*	Return Value:
*		AIMOOE_OK:The function executed successfully.
*		AIMOOE_ERROR: Failure to recognize tools.
*************************************************************************/
DLLExport E_ReturnValue Aim_FindSingleToolInfo(AimHandle aimHandle, T_MarkerInfo & marker,
	const char * toolids, T_AimToolDataResultSingle &dataResult, int minimumMatchPts = 0);

/************************************************************************/
/**
*	Description:
*		Sets the direction of all coordinate transformation relations (RT matrices, rotation vectors, and quaternions) in the results obtained during tool lookup; the default direction is from the tool coordinate system to the system coordinate system. 
*		The API functions affected by this function setting are:
*		Aim_FindToolInfo()
*		Aim_FindSpecificToolInfo()
*		Aim_FindSingleToolInfo()
*		Aim_MappingPointSetsForMarkerSpaceReg()
*	Parameters:
*		direction: There are two directions, from tools to systems and from systems to tools. 
*	Return Value:
*		AIMOOE_OK:The function executed successfully.
*		AIMOOE_ERROR: Failure to recognize tools.
*************************************************************************/
DLLExport E_ReturnValue Aim_SetToolFindRTDirection(AimHandle aimHandle, E_RTDirection direction);

/************************************************************************/
/**
*	Description:
*		Gets the orientation of all coordinate transformation relations (RT matrices, rotation vectors, and quaternions) in the results obtained when the tool looks up. 
*	Parameters:
*		direction: There are two directions, from tools to systems and from systems to tools. 
*	Return Value:
*		AIMOOE_OK:The function executed successfully.
*		AIMOOE_ERROR: Failure to recognize tools.
*************************************************************************/
DLLExport E_ReturnValue Aim_GetToolFindRTDirection(AimHandle aimHandle, E_RTDirection &direction);

/************************************************************************/
/**
*	Description:
*		Initialize the information of the tool file to be created based on the tool ID and the number of marker points. 
*		The file is saved under the tool path set by the Aim_SetToolInfoFilePath() function.
*		Therefore, the Aim_SetToolInfoFilePath() function needs to be called before calling this function. 
*	Parameters:
*		markcnt£ºNumber of marking points on the tool
*		id£ºTool ID number
*	Return Value:
*		AIMOOE_OK:The function executed successfully.
*		AIMOOE_ERROR: Unknown error, please confirm that the function call is correct.
*************************************************************************/
DLLExport E_ReturnValue Aim_InitToolMadeInfo(AimHandle aimHandle, const int markcnt, const char * id);
/************************************************************************/
/**
*	Description:
*		Make a tool file and call Aim_InitToolMadeInfo() to initialize the tool information before use. 
*		The tool making process needs to call the and this function to get the coordinates of the marker points in a loop until Proinfo.isMadeProFinished is true.
*	Parameters:
*		marker£ºMarks the collection of points;
*		ProInfo:  Process information,when Proinfo.isMadeProFinished is true, the production is finished;
*	Return Value:
*		AIMOOE_OK:The function executed successfully.
*		AIMOOE_ERROR: Unknown error, please confirm that the function call is correct.
*************************************************************************/
DLLExport E_ReturnValue Aim_ProceedToolMade(AimHandle aimHandle, T_MarkerInfo &marker, t_ToolMadeProInfo&ProInfo);
/************************************************************************/
/**
*	Description:
*		Save or cancel the information of the file produced by the tool, and recall it after the production tool is finished.
*	Parameters:
*		saved: true Default saves the tool file information; false deletes the current file information after Aim_DoneToolMade is made;
*	Return Value:
*		AIMOOE_OK:The function executed successfully.
*		AIMOOE_ERROR: Unknown error, please confirm that the function call is correct.
*************************************************************************/
DLLExport E_ReturnValue Aim_SaveToolMadeRlt(AimHandle aimHandle, bool saved = true);

/************************************************************************/
/**
*	Description:
*		Select the tool to be registered (calibrated) with the tip and the registration board (calibration board) to be used. (It is not recommended to use this function)
*		It is recommended to use Aim_InitToolTipCalibrationWithToolId() instead of this function.
*	Parameters:
*		CalToolIndex Currently selected registration board (calibration board) (index of acquired tool files)
		PosToolIndex Currently selected tool (index of fetched tool files)
*	Return Value:
*		AIMOOE_OK:The function executed successfully.
*		AIMOOE_ERROR: Unknown error, please confirm that the function call is correct.
*************************************************************************/
DLLExport E_ReturnValue Aim_InitToolTipCalibration(AimHandle aimHandle, int CalToolIndex, int PosToolIndex);
/************************************************************************/
/**
*	Description:
*		Select the tool for which the tip needs to be registered (calibrated) and the registration plate (calibration plate) to be used by the tool ID. 
*	Parameters:
*		CalTool ID number of the currently used registration board (calibration board)
*		PosTool ID number of the currently selected tool
*	Return Value:
*		AIMOOE_OK:The function executed successfully.
*		AIMOOE_ERROR: Function execution failed. 
*************************************************************************/
DLLExport E_ReturnValue Aim_InitToolTipCalibrationWithToolId(AimHandle aimHandle, const char* CalTool, const char* PosTool);
/************************************************************************/
/**
*	Description:
*		Operate the selected tool and the registration board (calibration board) to get the tip information of the tool, Before calling, need
*	    Call the Aim_InitToolTipCalibrationWithToolId function to set the selected tool and registration plate (calibration plate). 
*		This function (and the function to get the coordinates of the marker) needs to be called multiple times until info.isCalibrateFinished is true
*	Parameters:
*		marker £ºCurrently captured marker point
*		ProInfo£ºReturns the parameters of the tip, the result is valid when the flag bit ProInfo.isCalibrateFinished is true
*	Return Value:
*		AIMOOE_OK:The function executed successfully.
*		AIMOOE_ERROR: Function execution failed.
*************************************************************************/
DLLExport E_ReturnValue Aim_ProceedToolTipCalibration(AimHandle aimHandle, T_MarkerInfo & marker, t_ToolTipCalProInfo &ProInfo);
/************************************************************************/
/**
*	Description:
*		Save the registered (calibrated) tool tip information to the original tool file. 
*	Parameters:
*		null
*	Return Value:
*		AIMOOE_OK:The function executed successfully.
*		AIMOOE_ERROR: fail to save.
*************************************************************************/
DLLExport E_ReturnValue Aim_SaveToolTipCalibration(AimHandle aimHandle);

/************************************************************************/
/**
*	Description:
*		Select the registration board (calibration board) to be used and the tool whose coordinates need to be converted by the tool ID. 
*	Parameters:
*		CalTool ID number of the currently used registration board (calibration board)
*		PosTool ID number of the currently selected tool
*	Return Value:
*		AIMOOE_OK:The function executed successfully.
*		AIMOOE_ERROR: Function execution failed.
*************************************************************************/
DLLExport E_ReturnValue Aim_InitToolCoordinateRenewWithToolId(AimHandle aimHandle, const char * CalTool, const char * PosTool);
/************************************************************************/
/**
*	Description:
*		Operate the selected tool and the registration plate (calibration plate) to transfer the coordinate system of the tool under the coordinate system of the registration plate. Before calling
*	    The Aim_InitToolCoordinateRenewWithToolId function needs to be called to set the selected registration board (calibration board) and tool.
*		This function (and the function to get the coordinates of the marker) needs to be called multiple times until info.isCalibrateFinished is true
*	Parameters:
*		marker £ºCurrently captured marker point
*		ProInfo£ºReturns the parameters of the chipping tool, the result is valid when the flag bit ProInfo.isCalibrateFinished is true.
*	Return Value:
*		AIMOOE_OK:The function executed successfully.
*		AIMOOE_ERROR: Function execution failed.
*************************************************************************/
DLLExport E_ReturnValue Aim_ProceedToolCoordinateRenew(AimHandle aimHandle, T_MarkerInfo & marker, t_ToolTipCalProInfo & info);
/************************************************************************/
/**
*	Description:
*		Save the coordinates updated tool information to the original tool file. 
*	Parameters:
*		Null
*	Return Value:
*		AIMOOE_OK:The function executed successfully.
*		AIMOOE_ERROR: fail to save.
*************************************************************************/
DLLExport E_ReturnValue Aim_SaveToolCoordinateRenew(AimHandle aimHandle);

/************************************************************************/
/**
*	Description:
*		Select the tool to be registered (calibrated) with the tip by the tool ID, and complete the tip registration by rotating around the point. 
*	Parameters:
*		toolID: ID number of the currently selected tool
*		clearTipMid: When true, the data at the point on the tip is cleared to 0; when false, the data at the point on the tip remains unchanged. 
*	Return Value:
*		AIMOOE_OK:The function executed successfully.
*		AIMOOE_ERROR: Function execution failed.
*************************************************************************/
DLLExport E_ReturnValue Aim_InitToolTipPivotWithToolId(AimHandle aimHandle, const char* toolID, bool clearTipMid=false);
/************************************************************************/
/**
*	Description:
*		Data collection was performed on the selected tool to get the tip information of the tool, before calling function, need
*	    Call the Aim_InitToolTipPivotWithToolId function to set the selected tool. 
*		This function (and the function to get the coordinates of the marker) needs to be called multiple times until info.isCalibrateFinished is true.
*		Please start rotating the tip of the needle to be registered around the point beforehand, and then use this function. 
*	Parameters:
*		marker £ºCurrently captured marker point
*		pivotInfo£ºReturns the parameters of the needle tip, the result is valid when the flag bit ProInfo.isCalibrateFinished is true.
*	Return Value:
*		AIMOOE_OK:The function executed successfully.
*		AIMOOE_ERROR: Function execution failed.
*************************************************************************/
DLLExport E_ReturnValue Aim_ProceedToolTipPivot(AimHandle aimHandle, T_MarkerInfo & marker, T_ToolTipPivotInfo &pivotInfo);
/************************************************************************/
/**
*	Description:
*		Save the registered (calibrated) tool tip information to the original tool file. 
*	Parameters:
*		Null
*	Return Value:
*		AIMOOE_OK:The function executed successfully.
*		AIMOOE_ERROR: fail to save.
*************************************************************************/
DLLExport E_ReturnValue Aim_SaveToolTipPivot(AimHandle aimHandle,int lineTndex=1);

/************************************************************************/
/**
*	Description:
*		Select the tool to be calibrated and call it before calling Aim_ProceedToolSelfCalibration
*		Calling this function is not recommended;
*		It is recommended to use Aim_InitToolSelfCalibrationWithToolId() instead of this function.
*	Parameters:
*		ToolIndex Currently selected tool (index of fetched tool files)
*		markcnt  Returns the number of marker points for the selected tool
*	Return Value:
*		AIMOOE_OK:The function executed successfully.
*		AIMOOE_ERROR: Function execution failed.
*************************************************************************/
DLLExport E_ReturnValue Aim_InitToolSelfCalibration(AimHandle aimHandle, int ToolIndex, int &markcnt);
/**
*	Description:
*		Select the tool to be calibrated before calling Aim_ProceedToolSelfCalibration
*	Parameters:
*		tool ID number of the currently selected tool
*		markcnt  Returns the number of marker points for the selected tool
*	Return Value:
*		AIMOOE_OK:The function executed successfully.
*		AIMOOE_ERROR: Function execution failed.
*************************************************************************/
DLLExport E_ReturnValue Aim_InitToolSelfCalibrationWithToolId(AimHandle aimHandle, const char * tool, int &markcnt);
/************************************************************************/
/**
*	Description:
*		To perform a self-calibration operation on the selected tool, you need to call Aim_InitToolSelfCalibrationWithToolId 
*	to set the selected tool before calling it.This function needs to be called several times until the isCalibrateFinished parameter in ProInfo is true.
*	Confirm that the accuracy meets the requirements and save the result according to other parameters in ProInfo. 
*	Parameters:
*		marker: Currently captured marker point
*		ProInfo: Returns the calibration result, where the ProInfo.isCalibrateFinished parameter is true, indicating that the calibration operation is complete. 
*	Return Value:
*		AIMOOE_OK:The function executed successfully.
*		AIMOOE_ERROR: The function execution fails and the current calibration is invalidated.
*************************************************************************/
DLLExport E_ReturnValue Aim_ProceedToolSelfCalibration(AimHandle aimHandle, T_MarkerInfo &marker, t_ToolFixProInfo&ProInfo);
/************************************************************************/
/**
*	Description:
*		Operational processing of calibration results for selected tools

*	Parameters:
*		fixrltcmd Cancel, Redo and Save
*	Return Value:
*		AIMOOE_OK:The function executed successfully.
*		AIMOOE_ERROR: Unknown error, please confirm that the function call is correct.
*************************************************************************/
DLLExport E_ReturnValue Aim_SaveToolSelfCalibration(AimHandle aimHandle, E_ToolFixRlt fixrltcmd);

/************************************************************************/
/**
*	Description:
*		Initialize the ID number of the accuracy test tool used, only the left and right four-point aimooe accuracy tools are supported
*	Parameters:
*		toolids:ID numbers for all points of the tool, toolidl left 4-point tool ID, toolid2 right 4-point tool ID
*	Return Value:
*		AIMOOE_OK: The function executes successfully, indicating that an accuracy test can be performed.
*		AIMOOE_ERROR: Unknown error, please confirm that the function call is correct.	
*************************************************************************/
DLLExport E_ReturnValue Aim_InitAccuracyCheckTool(AimHandle aimHandle, const char *toolids, const char*toolid1, const char*toolid2);
/************************************************************************/
/**
*	Description:
*		Call this function (and the function to get the coordinates of the marker points) in a loop as needed. 
*	Parameters:
*		markarr:markerset[markcnt][3]
*		markcnt:Number of marker points
*		T_AccuracyToolResult:Distance and rotation angle of the left and right tools for the current acquisition
*	Return Value:
*		AIMOOE_OK:The function executed successfully.
*		AIMOOE_ERROR: Unknown error, please confirm that the function call is correct.
*   Caution:
*	After this function is called, the data is stored internally for the Aim_AccuracyCheckToolCalculateError function to perform calculations
*************************************************************************/
DLLExport E_ReturnValue Aim_AccuracyCheckTool(AimHandle aimHandle, double** markarr,const int markcnt,T_AccuracyToolResult &Rlt);
/************************************************************************/
/**
*	Description:
*		Obtain positioning system accuracy error data, including mean, standard deviation, and angular error.
*	Parameters:
*		meanerro Mean error, stdev distance standard deviation, angle[3] angle mean error. 
*	Return Value:
*		AIMOOE_OK:The function executed successfully.
*		AIMOOE_ERROR: Unknown error, please confirm that the function call is correct.
*	 Caution:This function is called to zero out the internally stored data. 
*************************************************************************/
DLLExport E_ReturnValue Aim_AccuracyCheckToolCalculateError(AimHandle aimHandle, float &meanerro,float &stdev,float *angle);

/************************************************************************/
/**
*	Description:
*		Point set initialization for spatial alignment, passing spatial point set information as a tool to the optical positioning system
*	Parameters:
*		ImgPtArr Image space coordinate point set
*		ImgPtSize Number of point sets
*	Return Value:
*		AIMOOE_OK:The function executed successfully.
*		AIMOOE_ERROR: Unknown error, please confirm that the function call is correct.
*************************************************************************/
DLLExport E_ReturnValue Aim_InitMappingPointSetsForMarkerSpaceReg(AimHandle aimHandle, const float(*ImgPtArr)[3], const int ImgPtSize);
/************************************************************************/
/**
*	Description:
*		Spatial point set alignment, obtaining relevant information after alignment
*	Parameters:
*		marker£ºThree-dimensional point set acquired by an optical positioning system
*		pResultList£ºResults after registration
*		minimumMatchPts:The minimum number of match points (¡Ý3) that can be allowed when the multipoint tool is performing recognition.The default value of 0 indicates a full match.
*	Return Value:
*		AIMOOE_OK:The function executed successfully.
*		AIMOOE_ERROR: Unknown error, please confirm that the function call is correct.
*************************************************************************/
DLLExport E_ReturnValue Aim_MappingPointSetsForMarkerSpaceReg(AimHandle aimHandle, T_MarkerInfo & marker, T_AimToolDataResult * pResultList, int minimumMatchPts=0);

/************************************************************************/
/**
*	Description:
*		Setting up calibration results and tracked results for robot control
*	Parameters:
*		Sys2RobotBaseRTArray RT array from optical localization system to robot base (3*4 array, columns 0-2 R, column 3 dimension T)
* 	    Tool2RobotEndRTArray RT array from tool to robot end (3*4 array, columns 0-2 are R, column 3 is dimension T)
		toolid Tool ID number
		
*	Return Value:
*		AIMOOE_OK:The function executed successfully.
*		AIMOOE_ERROR: Unknown error, please confirm that the function call is correct.
*************************************************************************/
DLLExport E_ReturnValue Aim_SetRobotCalculateRlt(AimHandle aimHandle, const double(*Sys2RobotBaseRTArray)[4], const double(*Tool2RobotEndRTArray)[4],const char*toolid);
/************************************************************************/
/**
*	Description:
*		Calculate the target position of the robot path
*	Parameters:
*		TargetPathArr Paths under optical localization system: array of format 2*3, 0*3 for safe points, 1*3 for target points (XYZ)
* 	    targetPoseArr Target Position Array[X,Y,Z,Rx,Ry,Rz]

*	Return Value:
*		AIMOOE_OK:The function executed successfully.
*		AIMOOE_ERROR: Unknown error, please confirm that the function call is correct.
*************************************************************************/
DLLExport E_ReturnValue Aim_CalculateRobotTargetPose(AimHandle aimHandle, const double(*TargetPathArr)[3], float* targetPoseArr);

/************************************************************************/
/**
*	Description:
*		Get device MAC address (format: hexadecimal xx:xx:xx:xx:xx:xx)
*	Parameters:
*		addr£ºMac address of the current device
*	Return Value:
*		AIMOOE_OK:The function executed successfully.
*		AIMOOE_NOT_CONNECT: The device is not connected. Please connect the device first.
*		AIMOOE_ERROR: Unknown error, please confirm that the function call is correct.
*************************************************************************/
DLLExport E_ReturnValue Aim_GetAimPositonMacAddress(AimHandle aimHandle, char addr[18]);

/************************************************************************/
/**
*	Description:
*		Calculate 3D point coordinates from 2D point coordinates. 
*	Parameters:
*		leftPoint: Array of 2D point coordinates (x, y) for the left image, up to 200. 
*		leftNum£ºLeft image 2D point count, up to 200. 
*		rightPoint: Array of 2D point coordinates (x, y) for the right image, up to 200. 
*		rightNum£ºRight image 2D point count, up to 200. 
*		markerSt: Store the returned 3D coordinate information data.
*	Return Value:
*		AIMOOE_OK:The function executed successfully.
*		AIMOOE_NOT_CONNECT: The device is not connected. Please connect the device first.
*		AIMOOE_ERROR: Unknown error, please confirm that the function call is correct.
*************************************************************************/
DLLExport E_ReturnValue Aim_Calculate3DPoints(AimHandle aimHandle, float leftPoint[400], int leftNum, float rightPoint[400], int rightNum, T_MarkerInfo & markerSt);

/**
*	Description£º
*		2.3.5test£¬3D points are given directly
*	Parameters£º
*		markerSt: Store the 3D coordinate information data of the return post¡£
*	Return Value£º
*		AIMOOE_OK: The function executed successfully.
*		AIMOOE_NOT_CONNECT: The device is not connected. Please connect the device first.
*		AIMOOE_ERROR: Unknown error, please confirm that the function call is correct.
*************************************************************************/

DLLExport E_ReturnValue Aim_FindToolInfo_Map(AimHandle aimHandle, T_MarkerInfo& marker,
	std::vector<std::string>& toolids, std::map<std::string, T_AimToolDataResult*>& resultMap, int minimumMatchPts = 0);


//*2.3.5 
/************************************************************************/
/**
*	Description:
*		Set the tool existing in the tool folder as a reference tool, if the tool folder does not have the tool, it will return the AIMOOE_ERROR,
*		and after setting, the Aim_GetToolInfoInRef function will return the information in the coordinate system of the tool, and only one tool can be set.
*Parameter:
*		referTool: Refer to the name of the tool, no suffix is required
*	Return value:
*		AIMOOE_OK: The function is executed successfully
*		AIMOOE_ERROR: Execution failed
*************************************************************************/
DLLExport E_ReturnValue Aim_SetReferenceTool(AimHandle aimHandle, const char* referTool);//1-N Set one of the tools as a reference tool
/************************************************************************/
/**
*	Description:
*		To use it in pairs with Aim_SetReferenceTool, you need to call the Aim_SetReferenceTool function to set up the reference tool first, and call it directly will return the AIMOOE_ERROR
*		 The result will be stored in the resultMap, the key is the tool name, and the value is the tool information
*	Parameter:
*		Marker: The current collection of markers
*		toolids: a list of tool names that currently need to be converted
*		resultMap: Tool information
*		minimumMatchPts: The minimum number of match points
*	Return value:
*		AIMOOE_OK: The function is executed successfully.
*		AIMOOE_ERROR: Execution failed
*************************************************************************/
DLLExport E_ReturnValue Aim_GetToolInfoInRef(AimHandle aimHandle, T_MarkerInfo& marker,
	std::vector<std::string>& toolids, std::map<std::string, T_AimToolDataResult*>& resultMap, int minimumMatchPts = 0);//Set the corresponding tool names and the information of these tools under the ref
/************************************************************************/
/**
*	Description:
*		Cancels the reference coordinate system conversion state and clears the tools Aim_SetReferenceTool settings
*	Parameter:
*		Null
*	Return value:
*		AIMOOE_OK: The function is executed successfully.
*		AIMOOE_ERROR: Execution failed
*************************************************************************/
DLLExport E_ReturnValue Aim_CancelRef(AimHandle aimHandle);//Cancel the reference status

DLLExport E_ReturnValue Aim_GetReferenceInfo1to1(AimHandle aimHandle, T_MarkerInfo& marker,
	const char* referTool, const char* otherTool);//1-1
/************************************************************************/
/**
*	Description:
*		Compatible with NDI tools, convert files ending in.rom suffixes to.aimtool files
*	Parameter :
*		NDIFilename : the name of the NDI tool file
*	Return value :
*		AIMOOE_OK : The function is executed successfully.
*		AIMOOE_ERROR : Execution failed
*************************************************************************/
DLLExport E_ReturnValue Aim_ConvertNDIrom2Aimtool(AimHandle aimHandle, const char* NDIFilename);

DLLExport E_ReturnValue Aim_SetColineThreshold(AimHandle aimHandle, E_Interface interfaceType, int threshold);
/*---------------------------------API END------------------------------*/
