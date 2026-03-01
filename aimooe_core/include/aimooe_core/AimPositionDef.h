
#pragma once

#ifdef __cplusplus
    #if defined(_WIN32)
        #define DLLExport  extern "C" __declspec(dllexport)
    #else
        #define DLLExport  extern "C" __attribute__((visibility("default")))
    #endif
#else
    #define DLLExport  extern "C"
#endif

#include <list>
#include<vector>
#include<string>
#include<map>

#ifdef __cplusplus
extern "C" {
#endif

	typedef unsigned int  UINT;
	typedef unsigned char UCHAR;
	typedef unsigned short USHORT;
	typedef void* AimHandle;
	#define PtMaxNUM 200			/**< Maximum number of marker points cannot be modified */
	#define TOOLIDMAX 50
	enum class AIMPOS_TYPE
	{
		eAP_Basic = 0,
		eAP_Standard =1,
		eAP_Industry = 2,
		eAP_Lite=3,
		eAP_Ultimate = 4,

		eOP_M31 = 5,			

		eOP_M321 = 6,				
		eOP_M322 = 7,				

		eOP_M62 = 8, 			

		eOP_M631 = 9,				
		eOP_M632 = 10,				

		eOP_I61 = 11,				
		eOP_I621 = 12,				
		eOP_I622 = 13,				

		eOP_I81 = 14,				
		eOP_I821 = 15,				
		eOP_I822 = 16,				

		eAP_NONE=100
	};

	enum E_AimToolType
	{
		ePosTool = 0,
		ePosCalBoard = 1

	};
	
	/**
	*	Communication interface type
	*/
	enum E_Interface
	{
		I_USB=0,			/**< USB interface */
		I_ETHERNET,	/**< Ethernet communication interface */
		I_WIFI,		/**< WIFI communication interface */
	};

	/**
	*	Position system commands
	*/
	enum E_SystemCommand
	{
		SC_COLLISION_DISABLE,	/**< Collision detection is turned off (on by default) and existing warnings of collision occurrences are automatically cleared. */
		SC_COLLISION_ENABLE,	/**< Turn on collision detection (on by default), when a collision is detected, the system information can be read as a warning that
								*	a collision has occurred, at which point collision detection is suspended,
								*	clear before continuing into collision detection. In addition, the
								*	collision detection can be set with the Aim_SetCollisinoDetectLevel() function. */
		SC_COLLISION_INFO_CLEAR,/**< Clear the warning that a collision has occurred */
		SC_IRLED_ON,			/**< Turn on IR illumination (default on), IR is required for tracking markers.
								*	lighting */
		SC_IRLED_OFF,			/**< Turn off infrared lighting (on by default), and when infrared lighting is not needed, 
								*	Switching off the illuminated ring reduces power consumption and prolongs lifetime */
		SC_LASER_ON,			/**< Positioning laser switched on (default off) */
		SC_LASER_OFF,			/**< Positioning laser switched off (default off) */
		SC_LCD_PAGE_SUBPIXEL,	/**< Touch display switches to marker point coordinate tracking interface (default interface on power-up) */
		SC_LCD_PAGE_COLOR,		/**< Touch display switches to colour image display interface */
		SC_LCD_ON,				/**< Turn on the touch display backlight (on by default) */
		SC_LCD_OFF,				/**< Turn off the touch display backlight (on by default) when the screen display is not needed.
								*	Switching off the screen backlight reduces power consumption and prolongs lifespan */
		SC_AF_CONTINUOUSLY_ON,	/**< Middle colour camera continuous autofocus on (default off), when turn on
								*	Colour camera autofocuses when it loses focus */
		SC_AF_CONTINUOUSLY_OFF,	/**< Continuous autofocus off for intermediate colour cameras (default off)
								*	Turning off the colour camera will no longer perform continuous autofocusing*/
		SC_AF_SINGLE,			/**< Intermediate colour camera focuses automatically in a single pass��
								*	When the command is given, the intermediate colour camera will perform a single autofocus
								*	(Tips: In the colour image display interface of the touchscreen display,
								*	clicking on a colour image also triggers the colour camera to perform a single autofocus. */
		SC_AF_FIX_INFINITY,		/**< Middle colour camera fixed focus to infinity (default focus at infinity on power up) */
		SC_AF_EXP_AUTO_ON,		/**< Turn on automatic exposure for intermediate colour cameras (on by default):
								*	When enabled, the system adjusts the exposure value and brightness gain according to the current colour image brightness */
		SC_AF_EXP_AUTO_OFF,		/**< Turn off automatic exposure for intermediate colour cameras (on by default):
								*	After switching off, the exposure value and brightness gain at the moment of switching off are maintained */
		SC_AF_RESTART,/**Restarting the colour camera*/
		SC_DUALCAM_AUTO_EXP_ON,/**Activate Binocular Camera Auto Exposure*/
		SC_DUALCAM_AUTO_EXP_OFF,/**Turn off binocular camera auto exposure*/
		SC_DUALCAM_POWER_ON, /**Start Binocular Camera, start by default on power up*/
		SC_DUALCAM_POWER_OFF, /**Disable Binocular Camera, boot up by default*/
		SC_DUALCAM_SYNC_TRIG,/**Binocular camera acquisition synchronisation*/

		SC_NONE
	};

	/**
	*	When indicating the current collision, collision detection can be set with the Aim_SetCollisinoDetectLevel() function.
	*/
	enum E_CollisionStatus
	{
		COLLISION_NOT_OCCURRED,	/**< Collision did not occur */
		COLLISION_OCCURRED,		/**<  A collision occurred, at which point the Aim_SetSystemCommand() function is used to
								*	Issue the SC_COLLISION_INFO_CLEAR command to clear the collision warning.
								*	The collision display will pause when a collision occurs, and the only way is to clear the current collision warning 
								*	and then detection will start again. */
		COLLISION_NOT_START,	/**< Collision detection is not switched on */
	};

	/**
	*	Indicate whether the background brightness of the current binocular camera is normal or not, the coordinates of the marker points acquired in case of abnormal brightness may be inaccurate.
	*/
	enum E_BackgroundLightStatus
	{
		BG_LIGHT_OK,		/**< Normal background light */
		BG_LIGHT_ABNORMAL,	/**< Background light anomaly (computing) */
	};

	/**
	*	Indicate the current hardware status, when an abnormality occurs, please try to restart the instrument first, if it can not be solved, please contact the manufacturer for maintenance as soon as possible.
	*/
	enum E_HardwareStatus
	{
		HW_OK,							/**< Hardware status is normal */
		HW_LCD_VOLTAGE_TOO_LOW,			/**< Low voltage on touch display */
		HW_LCD_VOLTAGE_TOO_HIGH,		/**< High voltage on touch display */
		HW_IR_LEFT_VOLTAGE_TOO_LOW,		/**< Low voltage in the left camera lighting system */
		HW_IR_LEFT_VOLTAGE_TOO_HIGH,	/**< High voltage in the left camera lighting system */
		HW_IR_RIGHT_VOLTAGE_TOO_LOW,	/**< Right camera lighting system voltage too low */
		HW_IR_RIGHT_VOLTAGE_TOO_HIGH,	/**< Right camera lighting system voltage too high */
		HW_GET_INITIAL_DATA_ERROR,		/**< Error getting initialisation data */
	};

	/**
	*	API readings from the locator.
	*/
	enum E_DataType
	{
		DT_NONE,			/**< No data reading (power-on default) */			
		DT_INFO,			/**< Reads only information data (including system status and 3D coordinates)*/
		DT_MARKER_INFO_WITH_WIFI,		/**< Only reads 3D coordinate data, valid only when using wifi communication. */
		DT_STATUS_INFO,		/**< Only reads system status data, only valid when using wifi communication. */
		DT_IMGDUAL,			/**< Reads binocular image data only, not valid when using WiFi communication. */
		DT_IMGCOLOR,		/**< Reads colour image data only, not available when using WiFi communication. */
		DT_INFO_IMGDUAL,	/**< Simultaneous reading of information data (including system status and 3D coordinates) and binocular image data */
		DT_INFO_IMGCOLOR,	/**< Simultaneous reading of information data (including system status and 3D coordinates) and colour image data */
		DT_INFO_IMGDUAL_IMGCOLOR,	
						/**< Simultaneous reading of information data (including system status and 3D coordinates), binocular image data and colour image data*/
	};

	/**
	*	Return values of API functions
	*/
	enum E_ReturnValue
	{
		AIMOOE_ERROR = -1,		/**< Unknown error, please make sure the function is called correctly. */
		AIMOOE_OK=0,				/**< Function Execution Succeeded */
		AIMOOE_CONNECT_ERROR,	/**< Device open error, please make sure the device is connected correctly */
		AIMOOE_NOT_CONNECT,		/**< Device not connected, please connect the device first */
		AIMOOE_READ_FAULT,		/**< Read device error, please make sure the device is connected properly */
		AIMOOE_WRITE_FAULT,		/**< Write device error, please make sure the device is connected properly  */
		AIMOOE_NOT_REFLASH,		/**< The data information has not been updated, if the data to be acquired has been set correctly, please try again. */
		AIMOOE_INITIAL_FAIL,		/**<Initialisation failure*/
		AIMOOE_HANDLE_IS_NULL,	/**<Getting a handle without calling the initialisation function*/
	};

	enum E_MarkWarnType
	{
		eWarn_None = 0,//Not close to the edge of the field of view
		eWarn_Common = 1,//Close to the edge of the field of view
		eWarn_Critical = 2,//Very close to the edge of the field of view
	};

	enum E_AcquireMode
	{
		ContinuousMode = 0,//Continuous collection
		SingleMasterMode = 1,//Single Active Acquisition
		SingleSlaveMode = 2,//Single Passive Acquisition
	};

	enum E_ToolFixRlt
	{
		eToolFixCancle = 0,
		eToolFixRedo = 1,
		eToolFixSave = 2

	};

	enum E_RTDirection
	{
		FromToolToOpticalTrackingSystem= 0,
		FromOpticalTrackingSystemToTool = 1
	};

	struct T_Img_Info
	{
		unsigned int width;//Picture width
		unsigned int height;//Picture height
		UCHAR channel;//1 channel 8bit for grayscale images and 2 channels 16bit for colour images (RGB565)

	};

	struct T_AIMPOS_DATAPARA
	{

		AIMPOS_TYPE devtype;
		T_Img_Info dualimg;
		T_Img_Info colorimg;
		char hardwareinfo[20] = { 0 };//Model Information

	};

	/**
	*	Contain factory information about the locator
	*/
	struct T_ManufactureInfo
	{
		USHORT Year;			/**< Date of shipment: Year */
		UCHAR Month;			/**< Date of shipment: Month */
		UCHAR Day;			/**< Date of shipment: Date*/
		char Version[20];	/**< Version Number */
		UCHAR VersionLength;	/**< Character length of the version number */
	};

	/**
	*	Contain system status information
	*/
	struct T_AimPosStatusInfo
	{
		float Tcpu;			/**< CPU temperature (-128��~127��), to ensure that the CPU temperature is less than 75��.*/
		float Tpcb;			/**< Main board temperature (-128��C~127��C), when the main board temperature is greater than 50��C,
							*	the instrument will sound two alarms.*/
		UCHAR LeftCamFps;	/**< Left camera live frame rate (fps)*/
		UCHAR RightCamFps;	/**< Right camera live frame rate (fps)*/
		UCHAR ColorCamFps;	/**< Intermediate colour camera live frame rate (fps)*/
		UCHAR LCDFps;		/**< Touch display real-time frame rate (fps)*/
		USHORT ExposureTimeLeftCam;
							/**< Left camera live exposure value (0x30-0x4000)*/
		USHORT ExposureTimeRightCam;
							/**< Right camera live exposure value (0x30-0x4000)*/
		E_CollisionStatus CollisionStatus;	/**< Current collision status */
		E_HardwareStatus HardwareStatus;	/**< Current Hardware Status */
	};
	/**Indicate the level at which the position of the marker point is close to the edge of the camera's field of view*/


	/**
	*	Contain information about the coordinates of marker points
	*/
	struct T_MarkerInfo
	{
		/** The coordinates of the marking point are numbered, and the number is automatically increased by 1 when the system updates the coordinates once, 
			and the number is automatically increased by 1 when the system updates the coordinates.
		*1/60th of a second between two adjacent frames in continuous acquisition mode.*/
		UINT ID;	

		/** The number of detected marker points, with 0 indicating that no marker points were detected.
		   *	Maximum number of marker points 200*/
		int MarkerNumber;	

		/** The system's coordinate point set: holds the 3D coordinates of the marker points, array type double.
		*	The array length degree is PtMaxNUM * 3 (where PtMaxNUM = 200)
		*	Each marker point occupies 3 lengths and the order of coordinates is X->Y->Z;*/					
		double MarkerCoordinate[PtMaxNUM * 3];

		/**Phantom point warning array: make value=PhantomMarkerWarning[i]��
		* When value > 0, it means that the ith point is likely to be a phantom point
		* The value value indicates the phantom point subgroup group number to which the point belongs.*/
		int PhantomMarkerWarning[PtMaxNUM];

		/**The total number of phantom point groupings, which is 0 if no phantom points exist */
		int PhantomMarkerGroupNumber;

		/** Indicate whether the background brightness of the current binocular camera is normal or not,
		*   the coordinates of the markers may be inaccurate if the brightness is not normal. */
		E_BackgroundLightStatus MarkerBGLightStatus;

		/**Indicate the level of proximity of each marker point to the edge of the camera's field of view*/
		E_MarkWarnType MarkWarn[PtMaxNUM];
		/**Rank of the point closest to the edge of the field of view among all marked points recognised by the left camera*/
		E_MarkWarnType bLeftOutWarnning;
		/**Rank of the point closest to the edge of the field of view among all marked points recognised by the right camera*/
		E_MarkWarnType bRightOutWarning;
	};

	struct T_ToolTipPivotInfo
	{
		bool isToolFind;			/**Whether the tool is found*/
		bool isPivotFinished;	/**Whether the rotation process around the point is finished*/
		float pivotRate;		/**Rotation progress around the point*/
		float pivotMeanError;		/**Mean error of tip acquisition*/
	};

	struct t_ToolTipCalProInfo
	{
		bool isBoardFind;			/**Whether the calibration board tool is found*/
		bool isToolFind;			/**Locate if the tool has been found*/
		bool isValidCalibrate;		/**Whether the current calibration is valid*/
		bool isCalibrateFinished;	/**Whether the calibration process is finished or not*/
		float CalibrateError;		/**Mean error of calibration*/
		float CalibrateRate;		/**Calibration progress*/
		float CalRMSError;			/**Calibrated RMS error*/
	};

	struct t_ToolFixProInfo//Tool identification information calibration
	{
		bool isToolFind;	//Whether the tool was found for the current calibration
		bool validfixflag;//Whether the current calibration is valid
		int  isValidFixCnt;//Valid calibration times, ends when 100 are reached
		bool isCalibrateFinished;//Whether the calibration is finished or not
		float MatchError;//Matching error after calibration
		bool *mpMarkMatchStatus;//This variable is no longer used and indicates whether the points on the tool match for the current calibration.
		int totalmarkcnt;//Marker points of the selected tool
	};
	struct t_ToolMadeProInfo//Tool identification information document production
	{
		bool unValidMarkerFlag;//Invalid points in the current acquisition
		float madeRate;//degree of progress (on project)
		bool isMadeProFinished;
		float MadeError;//production error	
	};
	struct t_ToolBaseInfo
	{
		char  *name;
		char  isBoard;
		char  markcnt;
	};

	struct T_AimToolDataResult
	{
		E_AimToolType type;			//Tool types, currently positioning tools and calibration plate tools
		bool validflag;				//Effectiveness of currently identified tools
		char  toolname[PtMaxNUM];	//Tool name (corresponds to the first line of the tool file)
		float MeanError;				//Absolute mean error of the deviation of the point distance on the tool
		float Rms;					//Standard deviation of tool point distance error

		float rotationvector[3];	//Rotation vectors from the tool coordinate system to the system coordinate system
		float Qoxyz[4];				//Quaternions from the tool coordinate system to the system coordinate system in the order Qx->Qy->Qz->Qo
		float Rto[3][3];			//Rotation matrix from the tool coordinate system to the system coordinate system
		float Tto[3];				//Translation matrix from the tool coordinate system to the system coordinate system

		float OriginCoor[3];		//Coordinates of the origin of the tool in the system coordinate system
		float tooltip[3];			//Coordinates of the tip of the tool in the system coordinate system
		float toolmid[3];			//Coordinates of another point in the tip direction of the tool in the system coordinate system
		float toolCstip[3];			//Tool coordinate system tip coordinates (i.e., tip coordinates on the tool's tool file)
		float toolCsmid[3];			//Coordinates of another point in the tip direction of the tool coordinate system
		std::vector<int>toolptidx;	//The index of the point on the tool under the system's coordinate point set, which is -1 for mismatched points
		T_AimToolDataResult*next;
	};

	struct T_AimToolDataResultSingle
	{
		E_AimToolType type;			//Tool types, currently positioning tools and calibration plate tools
		bool validflag;				//Effectiveness of currently identified tools
		char  toolname[PtMaxNUM];	//Tool name (corresponds to the first line of the tool file)
		float MeanError;				//Absolute mean error of the deviation of the point distance on the tool
		float Rms;					//Standard deviation of tool point distance error

		float rotationvector[3];	//Rotation vectors from the tool coordinate system to the system coordinate system
		float Qoxyz[4];				//Quaternions from the tool coordinate system to the system coordinate system in the order Qx->Qy->Qz->Qo
		float Rto[3][3];			//Rotation matrix from the tool coordinate system to the system coordinate system
		float Tto[3];				//Translation matrix from the tool coordinate system to the system coordinate system

		float OriginCoor[3];		//Coordinates of the origin of the tool in the system coordinate system
		float tooltip[3];			//Coordinates of the tip of the tool in the system coordinate system
		float toolmid[3];			//Coordinates of another point in the tip direction of the tool in the system coordinate system
		float toolCstip[3];			//Tool coordinate system tip coordinates (i.e., tip coordinates on the tool's tool file)
		float toolCsmid[3];			//Coordinates of another point in the tip direction of the tool coordinate system
		int toolPtNum;		//Total points on tools
		int toolPtId[200];	//The index of the point on the tool under the system's coordinate point set, which is -1 for mismatched points.

	};


	//Measurements of the left and right tools on the precision tool
	struct T_AccuracyToolResult
	{	
		bool validflag; //Whether the current test is valid
		float Dis;//Distance between left and right tool origin
		float Angle[3];//Angle of the left and right tool normal vectors in radians
	};
	struct T_ToolFileData
	{
		char  toolname[PtMaxNUM];//Tool Name 
		char tooType;
		int markerNumbers;
		float MarkerCoordinate[200 * 3];//Support up to 200 marker points
		int  constTipNum;
		float tipHeadCoordinate[3];
		float tipBodyCoordinate[3];
	};


#ifdef __cplusplus
}
#endif





