/****************************************************************************
* Filename          : clHSA_TestMode.h
* Module            :
* Project           : VW-LL-RNS300
*----------------------------------------------------------------------------
* Copyright         : 2007 Robert Bosch GmbH, Hildesheim
* Author            : CM-DI/PJ-VW34
*----------------------------------------------------------------------------
* Description       : This class will overwrite the generated base classes.
Developers can work on these classes without running 
into merge conflicts after the generator run. 
The implementation classes may provide functionality 
by overwriting the virtual functions, 
defined in the base class clHSA_TestMode_base
*------------------------------------------------------------------------------
* HISTORY
*------------------------------------------------------------------------------
* Date         Author             Modification
*------------------------------------------------------------------------------
* 11-03-2013 | Vinod Bhargav B S| NIKAI-3091: Added new enum EDevMgrDeviceConnStatus
* 15-03-2013 | Dirk Klein       | Added defines for ODOMETER_DIRECTION_
******************************************************************************/

#ifndef HSA_TESTMODE_H
#define HSA_TESTMODE_H


#include "API/HSA_System/HSA_TestMode/clHSA_System_TestMode_Base.h"

#include "API_Impl/HSA_Navigation/clHSA_Navigation.h"


#define TRC_PARAM_TESTMODE_MASK    0x0000
#define TRC_RET_TESTMODE_MASK      0x8000

#define TRC_TYPE_TESTMODE_UINT     0x1000
#define TRC_TYPE_TESTMODE_SINT     0x2000
#define TRC_TYPE_TESTMODE_BOOL     0x3000
#define TRC_TYPE_TESTMODE_STRING   0x4000
#define TRC_TYPE_TESTMODE_COMMENT  0x0000

#define TESTMODE_MERIDIANS       0
#define TESTMODE_ANTENNASTATUS   1
#define TESTMODE_TRACKINGSTATUS  2
#define TESTMODE_SATVISIBLE      3
#define TESTMODE_SATTRACKED      4
#define TESTMODE_LONGITUDE       5
#define TESTMODE_LATITUDE        6
#define TESTMODE_ALTITUDE        7
#define TESTMODE_HDOP            8
#define TESTMODE_VDOP            9
#define TESTMODE_HEADING         10
#define TESTMODE_DATE            11
#define TESTMODE_TIME            12
#define TESTMODE_ALMANCSTATUS    13
#define TESTMODE_GNSS_TYPE       14

#define TESTMODE_CORRECTION_FACTOR  0.5

#define CONVERT_MM_TO_METER         1000


//Calibration
#define CAL_FM_0  0
#define CAL_FM_1  1
#define CAL_FM_2  2
#define CAL_FM_3  3
#define CAL_AM    4
#define LESS_THREE   3

#define MAX_VERSION_LENGTH 35


enum VERSION_ID
{
   EN_CUSTOMER_VERSION = 0,
   EN_BUILDVERSION_CUSTVERSTRING =1,
   EN_BUILDVERSION_LABEL= 2,
   EN_NAVI_REGISTRY_VERSION =3,
   EN_ADR_SW_VERSION=4,
   EN_CPLD_VERSION=5,
   EN_CONFIG_PART_NUMBER=6,
   EN_BOSCH_CONFIGURATION_ID=7,
   EN_CD_MODULE_VERSION=8,
   EN_HARDWARE_VERSION=9,
   EN_BT_MODULE_VERSION=10,
   EN_BT_FIRMWARE_VERSION=11,
   EN_SD_CARD_NUMBER=12,
   EN_ORDER_NUMBER=13,
   EN_SERIAL_NUMBER=14,
   EN_METER_VERSION_AUDIO=15,
   EN_METER_VERSION_NAVI=16,
   EN_METER_VERSION_SWC=17,
   EN_MCAN_BOSE_AMF=18,
   EN_APPS_NUMBER=19,
   EN_MCAN_TCU=20,
   EN_SYSTEM_VERSION=21,
   EN_SDCARD_VERSION=22
};

enum StreamInfo
{
	EN_ACIDPerFrame = 0,
	EN_MsgsPerFrame
};
enum UriData
{
	TotalURI = 0,
	ActiveURI
};
enum TPEGDabTuner
{
	EN_MsgsReceived = 0,
	EN_MsgsSelserv,
	EN_LastMsgsRecv
};
/** Defines for Tuner Parameter */
#define MAX_IPOD_SW_STRLEN          50
#define TSTMODE_SHIFT_EIGHT         8
#define ALWAYS_OK           1
/* Maximum string length for MApmatchnig info Formatting */
#define TESTMODE_DLR_GPS_STRLEN     50
#define TESTMODE_DLR_GPS_INTEGER    5
#define TESTMODE_DLR_GPS_ONEDIGIT   10
#define TESTMODE_DLR_GPS_TWODIGIT   100
#define TESTMODE_PD_LEN             20

#define TESTMODE_MAPMATCH_LATITUDE  0
#define TESTMODE_MAPMATCH_LONGITUDE 1
#define TESTMODE_MAPMATCH_HEADING   2
#define TESTMODE_MAPMATCH_ALTITUDE  3
#define TESTMODE_MAPMATCH_GRADIENT  4

#define DED_SECOND_FRACTION 10

#define MAX_BUFFER_STRLEN           50
#define MAX_TROUBLE_CODE_STRLEN     100
#define MODULUSVALUE                100
/* Maximum string length for GPS info Formatting */
#define MAX_GPS_FORMAT_STRLEN       50
#define MAX_LEN_FOR_VERSION         255

#define MAX_BUFFER_STRLEN_BT_TEST   60

/** TestMode Tuner Defines */
#define MAX_NO_OF_CHAR_FM           38

#define MAX_FS_VALUE                99
#define MAX_RDS_ERROR_RATE_VALUE    99
#define MAX_NEIGHBOUR_CHANNEL_VALUE 0X3F
#define MAX_SIGNAL_QUALITY_VALUE    0X1F
#define MAX_HUB_VALUE               0X7F
#define MIN_SHARX_VALUE             00
#define MAX_SHARX_VALUE             06
#define MIN_HIGHCUT_VALUE           00
#define MAX_HIGHCUT_VALUE           07
#define MAX_AFLIST_LENGTH           99
#define MAX_TA_VALUE                01
#define MAX_TP_VALUE                01

/* Trouble code defines */
#define BLUETOOTH_CONNECT_FAILURE   0x000001
#define SDCARD_CONNECT_FAILURE      0x000002
#define DOWNLOAD_ECU_DEFECTIVE      0x000003
#define TMC_TUNER_ANT1_FAILURE      0x000004
#define GPS_ANTENNA_FAILURE         0x000005
#define DEVICE_FAIL_CD              0x000006
#define READ_ERR_CD                 0x000007
#define UNDER_VOLTAGE               0x000008
#define HIGH_VOLTAGE                0x000009
#define CRITICAL_OVERTEMP           0x00000A
#define BEZEL_HIGH_TEMP             0x00000B
#define ODE_PLUG_MISSING            0x00000C
#define WRITE_TERMINATION_SDCARD    0x00000D
#define SAT_ANTENNA_FAILURE         0x00000E

/** Defines for reset counter */
#define RESET_WATCHDOG                 1
#define RESET_APPASSERTION_FAILED      2
#define RESET_PROCESSOR_EXCEPTION      3
#define RESET_SPMAPP_CTRLERROR         4
#define RESET_FGS                      5
#define RESET_DOWNLOAD                 6
#define RESET_DIAG                     7
#define RESET_OTHER                    8

/** Dead reckoning defines */
#define DEADRECK_STATUS                0
#define DEADRECK_SPEED                 1
#define DEADRECK_CALIBRATION_ACCURACY  2
#define DEADRECK_CALIBRATION_STATUS    3

/** Odometer Data update defines */
#define ODO_STATUS                     0
#define ODO_PULSE_COUNT                1

/**  Odometer status convertion to GUI */
#define ODO_FI_TO_GUI_CONV             247

#define ODOMETER_DIRECTION_FORWARD     1
#define ODOMETER_DIRECTION_REVERSE     2

/** FM Monitor Mode Defines --- Change the values as per the FI*/

#define FMMONITOR_MODE_DDA             1
#define FMMONITOR_MODE_DDS             2
#define FMMONITOR_MODE_SINGLE          3
#define FMMONITOR_MODE_CUSTOMER        0

/** Radio Set-Up Mode Defines */

#define SETUP_MODE_DDA                 0
#define SETUP_MODE_DDS                 1
#define SETUP_MODE_AUTO                2
#define SETUP_MODE_SINGLE              3
#define SETUP_MODE_CAL                 4

/** Navigation Position Status Defines */
#define POSITION_STATUS_UNKNOWN        0
#define POSITION_STATUS_OFFMAP         1
#define POSITION_STATUS_OFFROAD        2
#define POSITION_STATUS_FREEZE         3
#define POSITION_STATUS_ONROAD         4
#define POSITION_STATUS_OFFCORRIDOR    5
#define POSITION_STATUS_LASTPOSITION   6

/** Calibration Status Defines */
#define CALIB_STATUS_RESERVED          0
#define CALIB_STATUS_CHECKING          1
#define CALIB_STATUS_AVAILABLE         2
#define CALIB_STATUS_NEWINSTALL        3
#define CALIB_STATUS_WHEELCHANGE       4
#define CALIB_STATUS_NOTAVAILABLE      5
#define MAX_CALIBPERCENTAGE_PROGRESS   100

/** Can Signal Defines */
#define CANSIGNAL_ZERO              0
#define CANSIGNAL_ONE               1
#define CANSIGNAL_TWO               2
#define CANSIGNAL_THREE             3
#define CANSIGNAL_FOUR              4
#define CANU32_MAXVALUE             0xFFFFFFFF

/** List Defines */
#define AFLIST_MAX_AVAILABLE        6
#define AFLIST_INDEX_MAX_AVAILABLE  5
#define AFLIST_QUALITY_MASK         0x1F

/** Service mode Field Strength case Defines */
#define SERVICEMODE_FIELDSTRENGTH_AM   0
#define SERVICEMODE_FIELDSTRENGTH_FM   1
#define SERVICEMODE_FIELDSTRENGTH_DAB  2

/** Service Mode Quality Indicator defines */
#define MINIMUM_FIELDSTRENGTH          20
#define MAXIMUM_FIELDSTRENGTH          60
#define QUALITY_INDICATOR_ZERO         0
#define QUALITY_INDICATOR_FULL         15

/** BT connected phone Link Key **/
#define PHN_UGZZC_BT_DEVICE_LINK_KEY_LEN 16

/** TMC Station Quality Indicator Limits */
#define MAX_TMCSTATION_POOR_QUALITY    0x0c
#define MAX_TMCSTATION_WEAK_QUALITY    0x18

/** Service Mode Quality Limits indicator */
#define SERVICE_MODE_POOR_QUALITY      0x0c
#define SERVICE_MODE_WEAK_QUALITY      0x18

/** Defines for Best Satellites */
#define BEST_SATELLITE_ONE          1
#define BEST_SATELLITE_TWO          2
#define BEST_SATELLITE_THREE        3
#define BEST_SATELLITE_FOUR         4

/** Defines used for Media Status in SD Card and CD */
#define MEDIA_CD_STATUS                      0
#define MEDIA_SD_STATUS                      1
#define MEDIA_CD_SOFTWARE_TO_LOAD            3
#define MEDIA_CD_SOFTWARE_TO_IGNORE          1
#define MEDIA_CD_VALID_SOFTWARE              7
#define MEDIA_CD_SOFTWARE_DIFF_VERSION       8

/** Defines used for Heading(MapMatched position) Conversion */
#define MAXIMUM_VALUE_U8HEADING_FORCONVERSION   256
#define CONVERSION_U8HEADING_TO_U16DEGREES      1.40625    // 360/256
#define FLOATHEADING_CORRECTION_FACTOR          0.5
#define MAXIMUM_DEGREES_POSSIBLE                360

#define SD_INFO                        1
#define MAX_CD_SD_NAVINFO_STRLEN       3300
#define MAX_CD_SD_LISTELEMENT_STRLEN   150

/**  Defines for Tuner Testmode Switch */
#define TESTMODESWITCH_ON           0
#define TESTMODESWITCH_OFF          1

/**  Defines for Dimming Value */
#define DIM_PWM_MIN_VALUE           10
#define DIM_PWM_MAX_VALUE           1000
#define DIVIDE_BY_TEN               10

#define SRCSTATE_SD_NOT_INSERTED    0x0c

/**  Defines for PhoneBook Types */
#define MEM_PHONEBOOK         0
#define SIM_PHONEBOOK         1

/**  Defines for BTService Types */
#define HFP_SERVICE           0
#define AVP_SERVICE           1
#define SMS_SERVICE           2
#define PIM_SERVICE           3
#define DUN_SERVICE           4

/**  Defines for BT Access Mode */
#define UGZZC_UNKNOWN_MODE    0
#define UGZZC_APL_MODE        1
#define UGZZC_HCI_MODE        2

#define GUI_PHONE_BT_DEV_TEST_TESTNAME 0
#define GUI_PHONE_BT_DEV_TEST_COMMAND_NAME 1
#define GUI_PHONE_BT_DEV_TEST_PACKET_MODE 2
#define GUI_PHONE_BT_DEV_TEST_MODULATION  3

#define PHONE_BT_DEV_TEST_MODUALTION_VALUE_0       0
#define PHONE_BT_DEV_TEST_MODUALTION_VALUE_500     1
#define PHONE_BT_DEV_TEST_MODUALTION_VALUE_1000    2

#define GUI_PHONE_BT_MODULE_STS_SWITCHED_OFF 2

/**  Defines for  Microphone Current Value */
#define MICROPHONE_CURRENT_MIN_VALUE  0
#define MICROPHONE_CURRENT_MAX_VALUE  30

//Defines for DAB setup
#define FEA_SERVICE_LINKING 0
#define FEA_CONCEALMENT_LEVEL 1

//Defines for Bit Masks
#define MASK_BIT_ZERO 0x0001
#define MASK_BIT_FOUR 0x0010

//Defines for string lengths
#define TWO_CHAR_LEN 2
#define THREE_CHAR_LEN 3
#define FOUR_CHAR_LEN 4
#define FIVE_CHAR_LEN 5
#define SIX_CHAR_LEN 6

//FM step size for GCC device
#define GOM_FM_STEP_SIZE  0X32

#define ID_NO_1 0
#define ID_NO_2 1
#define ID_NO_3 2

#define Page_1    1
#define Page_10   10
#define Add_1     1
#define NUMBER_INPUT 58
#define SUB_48 48
#define SUB_55 55

// Opertaing Audio Codec Values
#define GUI_PHONE_OPERATING_AUDIO_CODEC_PHONE_CALL_NBS 0
#define GUI_PHONE_OPERATING_AUDIO_CODEC_PHONE_CALL_WBS 1
#define GUI_PHONE_OPERATING_AUDIO_CODEC_SIRI_NBS 2
#define GUI_PHONE_OPERATING_AUDIO_CODEC_SIRI_WBS 3
#define GUI_PHONE_OPERATING_AUDIO_CODEC_DEFAULT_VA_NBS 4
#define GUI_PHONE_OPERATING_AUDIO_CODEC_DEFAULT_VA_WBS 5
#define GUI_PHONE_OPERATING_AUDIO_CODEC_IDLE 6


//SXM DTM SCREEN
#define SXM_DTM_DATA_MAX_SIZE    64

enum e_Antenna_Status_GUI
{
   EN_ANTENNA_UNKNOWN_GUI,
   EN_ANTENNA_OK_GUI,
   EN_ANTENNA_SHORT_GUI,
   EN_ANTENNA_OPEN_GUI,
   EN_ANTENNA_DEACTIVATED_GUI,
   EN_ANTENNA_PASSIVE_GUI,
   EN_ANTENNA_DISCONNECTED_GUI=5,
};

enum e_USB_Status
{
   EN_MASS_STORAGE = 0,
   EN_IPOD ,
   EN_UNKNOWN_MEDIUM,
   EN_NO_MEDIA,
};
enum e_SDCardStatusFromHSI
{
   SRV_SD_CARD_MEDIA_EJECTED = 1,
   SRV_SD_CARD_INCORRECT_MEDIA = 2,
   SRV_SD_CARD_DATA_MEDIA = 4,
};
enum e_Media_Status
{
   STATUS_INCORRECT =3,
   STATUS_DATA =5,
   STATUS_UNKNOWN_SD_CARD =0,
   STATUS_EJECT =17,
};
enum e_SrvSystemStatus
{
   SRV_SD_CARD_SLOT_ACCESS_STATUS = 0,
   SRV_SPEED_SIGNAL,
   SRV_DIRECTION_SIGNAL,
   SRV_ILLUMINATION_SIGNAL,
   SRV_GPS_ANTENNA_STATUS,
   SRV_PHONE_MICROPHONE_CURRENT,
   SRV_RADIO_ANTENNA_STATUS,
   SRV_RADIO_DAB_ANTENNA_STATUS,
};

enum e_SrvUSBDeviceStatus
{
   DEV_ERROR = 0,
   REMOVED,
   ANY_OTHER_NUMBER,
};

typedef struct 
{
   tU8	u8Degree;
   tU8   u8Minutes;
   tU8   u8Seconds;
   tU8   u8TenthSeconds;
   tBool bDirection;
}sMapMatch_Info;

typedef enum
{
   SXM_SERVICE_DTM_MONITOR_LINE_0 = 0,
   SXM_SERVICE_DTM_MONITOR_LINE_1,
   SXM_SERVICE_DTM_MONITOR_LINE_2,
   SXM_SERVICE_DTM_MONITOR_LINE_3,
   SXM_SERVICE_DTM_MONITOR_LINE_4,
   SXM_SERVICE_DTM_MONITOR_LINE_5,
   SXM_SERVICE_DTM_MONITOR_LINE_6,
   SXM_SERVICE_DTM_MONITOR_LINE_7,
   SXM_SERVICE_DTM_MONITOR_LINE_8,
   SXM_SERVICE_DTM_MONITOR_LINE_9,
   SXM_SERVICE_DTM_MONITOR_LINE_10,
   SXM_SERVICE_DTM_MONITOR_LINE_11,
   SXM_SERVICE_DTM_MONITOR_LINE_12,
   SXM_SERVICE_DTM_MONITOR_LINE_13,
   SXM_SERVICE_DTM_MONITOR_LINE_14,
   SXM_SERVICE_DTM_MONITOR_LINE_15,
   SXM_SERVICE_DTM_MONITOR_LINE_16,
   SXM_SERVICE_DTM_MONITOR_LINE_17,
   SXM_SERVICE_DTM_MONITOR_LINE_18,
   SXM_SERVICE_DTM_MONITOR_LINE_19,
   SXM_SERVICE_DTM_MONITOR_LINE_20,
   SXM_SERVICE_DTM_MONITOR_LINE_21,
   SXM_SERVICE_DTM_MONITOR_LINE_22,
   SXM_SERVICE_DTM_MONITOR_LINE_23,
   SXM_SERVICE_DTM_MONITOR_LINE_24,
   SXM_SERVICE_DTM_MONITOR_LINE_25,
   SXM_SERVICE_DTM_MONITOR_LINE_26,
   SXM_SERVICE_DTM_MONITOR_LINE_27,
   SXM_SERVICE_DTM_MONITOR_LINE_28,
   SXM_SERVICE_DTM_MONITOR_LINE_29,
   SXM_SERVICE_DTM_MONITOR_LINE_30,
   SXM_SERVICE_DTM_MONITOR_LINE_31,
   SXM_SERVICE_DTM_MONITOR_LINE_32,
   SXM_SERVICE_DTM_MONITOR_MAX_LINE
}enHSASXM_SerDTMMon_screen;

typedef enum
{
   SXM_SETTINGS_MENU_LINE_0 = 0,
   SXM_SETTINGS_MENU_LINE_1,
   SXM_SETTINGS_MENU_LINE_2,
   SXM_SETTINGS_MENU_LINE_3,
   SXM_SETTINGS_MENU_MAX_LINE
}enHSASXM_SettingsMenu_screen;

typedef enum
{
   FC_SXM_DIAG_ANT_UNSUPPORTED = 0,
   FC_SXM_DIAG_ANT_DETECTED,
   FC_SXM_DIAG_ANT_NOT_DETECTED,
   FC_SXM_DIAG_ANT_SHORTED,
   FC_SXM_DIAG_ANT_UNKNOWN
}enHSASXM_AntennaStatus;

typedef enum
{
   FC_SXM_DIAG_SQ_NO_SIGNAL = 0,
   FC_SXM_DIAG_SQ_WEAK_SIGNAL,
   FC_SXM_DIAG_SQ_GOOD_SIGNAL,
   FC_SXM_DIAG_SQ_EXCELLENT_SIGNAL,
   FC_SXM_DIAG_SQ_INVALID_SIGNAL
}enHSASXM_SignalQuality;

typedef enum
{
   AUDIO_NOT_SUBSCRIBED = 0,
   AUDIO_SUBSCRIBED,
   SUSPENDED_ALERT,
   SUSPENDED,
   INVALID
}enHSASXM_AudioSubscription;

typedef enum
{
   SUB_UNKNOWN = 1,
   DATASERVICE_SUBSCRIBED,
   DATASERVICE_NOT_SUBSCRIBED,
   DATASERVICE_NOT_AVAILABLE,
   NO_ANTENNA,
   NO_SIGNAL
}enHSASXM_DataServiceSubscription;

typedef enum
{
   GUI_DATASERVICE_NOT_SUBSCRIBED=0,
   GUI_DATASERVICE_SUBSCRIBED,
   GUI_DATASERVICE_NOT_AVAILABLE,
   GUI_DATASERVICE_LOADING
}enHSASXM_DataServiceSubscriptionforGUI;

typedef enum
{

   TRAFFIC_NOT_SUBSCRIBED = 0,
   TRAFFIC_SUBSCRIBED,
   TRAFFIC_NOT_AVAILABLE,
   TRAFFIC_LOADING
}enHSASXM_TrafficSubscription;

typedef enum
{

   STM_LINE_1 = 1,
   STM_LINE_2,
   STM_LINE_3,
   STM_LINE_4,
   STM_LINE_5,
   STM_LINE_6,
   STM_LINE_7,
   STM_LINE_8,
   STM_LINE_9,
   STM_LINE_10,
   STM_LINE_11,
   STM_LINE_12,
   STM_LINE_13,
   STM_LINE_14,
   STM_LINE_15
}enHSASXM_STMData;

//USB Media Connection
enum EDevMgrDeviceConnStatus
{
   USB_DEV_UNDEFINED  = 0,
   USB_DEV_WARNING,
   USB_DEV_CONNECTED,
   USB_DEV_REMOVED_BY_USR,
   USB_DEV_UNAVAIL_BAT_LOWVOLT,
   USB_DEV_UNAVAIL_HW_MALFUNC,
   USB_DEV_LAST
};

enum ISO_CONTRY_GUI
{
           EN_CON_UNDEFINED,
           EN_CON_GERMANY,
   	       EN_CON_UK,
           EN_CON_AUSTRALIA,
           EN_CON_ITALY,
           EN_CON_CZECH_REPUBLIC,
           EN_CON_NETHERLANDS,
           EN_CON_PORTUGAL,
           EN_CON_BRAZIL,
           EN_CON_RUSSIA,
           EN_CON_UKRAINE,
           EN_CON_TURKEY,
           EN_CON_USA,
           EN_CON_CANADA,
           EN_CON_FRANCE,
           EN_CON_MEXICO,
           EN_CON_SPAIN,
           EN_CON_DENMARK,
           EN_CON_SWEDEN,
           EN_CON_FINLAND,
           EN_CON_NORWAY,
           EN_CON_POLAND,
           EN_CON_SLOVAKIA,
           EN_CON_HUNGARY,
           EN_CON_GREECE,
           EN_CON_SAUDI_ARABIA,
           EN_CON_THAILAND
};


enum  enTPEGs_StreamListInfo
   {
      INDEX = 0,
      SID  = 1,
      APP_ID  = 2,
      CONTENT_ID  = 3,
      VERSION,
      LOC,
      MSG_PER_FRAME,
      AVG_MSGS,
	  MAX_MSGS
   };

enum  enTPEGs_URIData
   {
	  URI_INDEX = 0,
      URI_NAME  = 1,
      URI_SELECTED  = 2,
      URI_ACTIVE  = 3
   };

enum  enTPEG_TEC
   {
	  TEC_NO_OF_MSGS,
	  TEC_NO_OF_MSGS_SEL_AREA,
	  TEC_LASTMSG_DATETIME,
	  TEC_SEL_LASTMSG_DATETIME
   };

enum enTPEG_TMC
	{
	  TFP_NO_OF_MSGS,
	  TFP_NO_OF_MSGS_SEL_AREA,
	  TFP_LASTMSG_DATETIME,
	  TFP_SEL_LASTMSG_DATETIME
   };

class clHSI_TestMode;
class clHSI_Radio;
class clHSI_CMSystem;
class clHSI_CMPhone;
class clHSI_CMMeter;
class clHSI_CMDAB;
class clHSI_CMSXM;
class clHSI_Sound;
class clHSI_CMTCU;

class clHSA_TestMode : public clHSA_System_TestMode_Base
{
public:
   // standard methods to fullfill framework requirements
   static tVoid createInstance();
   static tVoid destroyInstance();
   clHSI_TestMode *     pclHSI_TestMode;
   clHSI_Radio*         pclHSI_Radio;
   clHSI_CMSystem*      pclHSI_System;
   clHSI_CMPhone*       pclHSI_CMPhone;
   clHSI_CMMeter*       pclHSI_CMMeter;
   clHSI_CMDAB*         pclHSI_CMDAB;
   clHSI_TunerMaster*   pclHSI_TunerMaster;
   clHSI_CMSmartPhone*  pclHSI_CMSmartPhone;
   clHSI_CMTCU*			pclHSI_CMTCU;
   clHSI_CMSXM*         pclHSI_CMSXM;
   clHSI_Sound*			pclHSI_Sound;
   tU8 u8TrackAFList;
   tU8 u8TrackTMCList;
   tU8 u8ActiveTuner;
   tBool bReqTunerCalData;

protected:
   clHSA_TestMode(T_EN_HSA_ID en_HSA_ID);

public:

   tU8  m_tU8BTDevActiveTest;

   // public API functions follow here
   virtual void vCheckSystemVoltage(tbool Action);

   virtual tBool bAfterConfigure( );

   /*********************DAB APIs************************/
   virtual ulword ulwGetTMDataAvailability();
   virtual void vSetDABSFMode(ulword ServiceLinkingMode);

   virtual ulword ulwGetDABSFModeSelectionStatus();

   virtual void vSetDABConcealmentLevel(tbool Direction);

   virtual ulword ulwGetDABConcealmentLevel();

   virtual ulword ulwGetDABSourceState();

   virtual void vGetDABChannelNumber(GUI_String *out_result);

   virtual void vGetDABFrequencyTable(GUI_String *out_result);

   virtual ulword ulwGetDABEnsembleFrequency(ulword DABTuner);

   virtual ulword ulwGetCurrentDABActivity();

   virtual ulword ulwGetDABSignalQuality();

   virtual void vGetDABEnsembleID(GUI_String *out_result,ulword DABTuner);

   virtual ulword ulwGetDABNumOfAudServices();

   virtual ulword ulwGetDABNumOfDataServices();

   virtual ulword ulwGetDABNumOfAudSerComp();

   virtual ulword ulwGetDABNumOfDataSerComp();

   virtual void vGetDABServiceID(GUI_String *out_result);

   virtual ulword ulwGetDABAudioBitRate();

   virtual ulword ulwGetDABSamplingRate();

   virtual void vGetDABAAC(GUI_String *out_result);

   virtual void vGetDABAudioCodec(GUI_String *out_result);

   virtual void vGetDABServiceLabel(GUI_String *out_result);

   virtual void vGetDABEnsembleLabel(GUI_String *out_result);

   virtual void vGetDABStereoMode(GUI_String *out_result);

   virtual void vGetDABShortEnsembleLabel(GUI_String *out_result);

   virtual void vGetDABShortServiceLabel(GUI_String *out_result);

   virtual tbool blGetDABTMCSupportStatus();

   virtual tbool blGetDAB_PorS_Info();

   virtual tbool blGetDABDRC();

   virtual void vGetDABShortSrvCompLabel(GUI_String *out_result);

   virtual void vGetDABSrvCompLabel(GUI_String *out_result);

   virtual void vGetDABSrvCompID(GUI_String *out_result);

   virtual void vGetDABAudioDataSerComType(GUI_String *out_result);

   virtual ulword ulwGetDABTransportMechanismID();

   virtual  tbool blGetDAB_PorD_Flag();

   virtual tbool blGetDABTPSupport();

   virtual void vGetDABAnnoSupport(GUI_String *out_result);

   virtual void vGetDABAnnoSwitchMask(GUI_String *out_result);

   virtual ulword ulwGetDABAudioQuality();

   virtual tbool blGetDABSync(ulword DABTuner);

   virtual tbool blGetDABMute();

   virtual ulword ulwGetDABMSCBER();

   virtual ulword ulwGetDABFICBER(ulword DABTuner);

   virtual ulword ulwGetDAB_RS_FEC();

   virtual slword slwGetDABFieldStrength();

   virtual ulword ulwGetDABProtectionLevel();

   virtual void vGetDABFMFrq(GUI_String *out_result);

   virtual void vGetDABFMPI(GUI_String *out_result);

   virtual ulword ulwGetDABFMQuality();

   virtual ulword ulwGetDABTransmissionMode();

   virtual void vSetDABTestmode(tbool Active);

   virtual ulword ulwGetDABNumberOfLinks();

   virtual void vGetDABLinkType(GUI_String *out_result,ulword Index);

   virtual void vGetDABFrqLabel(GUI_String *out_result,ulword Index);

   virtual void vGetDABSID_PI(GUI_String *out_result,ulword Index);

   virtual ulword ulwGetDABQuality(ulword Index);

   virtual void vGetDABEID(GUI_String *out_result,ulword Index); 

   virtual ulword ulwGetDABActiveLinkIndex();
   virtual void vGetTASource(GUI_String *out_result);

   virtual void vGetDAB_BG_Mode(GUI_String *out_result);

   virtual ulword ulwGetDABTMC();

   virtual ulword ulwGetDAB_TPEG();

   virtual void vGetDAB_TPEGNew(GUI_String *out_result, ulword ulwInfo_Type);

   virtual void vGetDABExpertIDInfo(GUI_String *out_result,ulword ID,ulword ID_Value ,tbool Type );

   virtual tBool blGetDAB_TSU_Status();

   virtual tBool blGetDABSwitchingStatus();

   virtual ulword ulwGetDABNumEnsembles_DB();

   virtual ulword ulwGetDABNum_TMC_Services();

   virtual ulword ulwGetDABNum_TPEG_Services();

   virtual void vGetDABExpertIDValue(GUI_String *out_result,ulword ID);

   virtual void vDABEnsembleFrequency(tbool Direction);

   virtual void vDABChangeService(tbool Direction);

   virtual void vGetDAB_Database_String(GUI_String *out_result);

   virtual void vDAB_DB_CurrentScreenQuery();

   virtual void vDAB_DB_Screen(tbool Screen_ID);

   virtual ulword ulwGetDAB_Database_Scrn_No();

   virtual void vSpellerInitforID(ulword ID);

   virtual void vSpellerCharacterInput(const GUI_String *InputString);

   virtual void vSpellerGetNameInput(GUI_String *out_result);

   virtual void vSpellerSetNameInput();

   tU16 ulwConversionHexToDec();

   virtual ulword ulwSpellerGetCursorPos();

   virtual void vSetTimeInterval(ulword Interval);

   virtual ulword ulwGetTimeInterval();

   virtual void vSetNewScreenDataID(ulword Screen_ID);

   virtual void vSpellerCharacterDelete();
   /******************DAB APIs End***********************/

   virtual void vDisplayTestStatus(tbool DisplayTestSuccess);

   virtual ulword ulwGetTunerTP(ulword Tuner);

   virtual void vGetGPSValues(GUI_String *out_result,ulword ListEntryNr);

   virtual void vGetVersion(GUI_String *out_result,ulword Version);

   virtual void vGetTemperatureGPS(GUI_String *out_result);

   virtual void vGetTemperatureDisplay(GUI_String *out_result);

   virtual void vGetTemperatureAmplifier(GUI_String *out_result);

   virtual ulword ulwGetTunerAFListCount(ulword Tuner);

   virtual void vGetTunerAFListIndex(GUI_String *out_result,ulword Index);

   virtual void vGetTunerFrequency(GUI_String *out_result,ulword Tuner);

   virtual void vGetProgramID(GUI_String *out_result,ulword Tuner);

   virtual void vGetCdNo(GUI_String *out_result,ulword Tuner);

   virtual ulword ulwGetTunerFS(ulword Tuner);

   virtual ulword ulwGetTunerTA(ulword Tuner);

   virtual ulword ulwGetTunerSharx(ulword Tuner);

   virtual ulword ulwGetTunerAF(ulword Tuner);

   virtual ulword ulwGetTunerHC(ulword Tuner);

   virtual void vGetMatchedPositionValues(GUI_String *out_result,ulword ListEntryNr);

   virtual void vGetTunerFrequencyUnit(GUI_String *out_result,ulword Tuner);

   virtual void vGetTunerNeighbourChannel(GUI_String *out_result,ulword Tuner);

   virtual ulword ulwGetTunerRDSErrorRate(ulword Tuner);

   virtual void vGetTunerSignalQuality(GUI_String *out_result,ulword Tuner);

   virtual void vGetTunerMultiPath(GUI_String *out_result,ulword Tuner);

   virtual void vGetTunerPI(GUI_String *out_result,ulword Tuner);

   virtual void vTriggerNavSamplePrompt();


   virtual ulword ulwGetTotalACIDperFrame(ulword ulwInfo_Type);

   virtual void vGetStreamList(GUI_String *out_result, ulword ulwListEntryNr, ulword ulwInfo_Type);

   virtual ulword ulwGetStreamListCount( );

   virtual void vRequestToURIList( );

   virtual ulword ulwGetURIListCount( );

   virtual void vGetURIData(GUI_String *out_result, ulword ulwInfo_Type);
   virtual void vGetURIList(GUI_String *out_result, ulword ulwListEntryNr, ulword ulwInfo_Type);
   virtual void vGet_TEC(GUI_String *out_result, ulword ulwInfo_Type);

   virtual void vGet_TMC(GUI_String *out_result, ulword ulwInfo_Type);


   /********************************************************************
   **************** Functions implemented on Monday ********************
   ********************************************************************/

   virtual tbool blIsDevelopermodeEnabled();

   virtual ulword ulwGetSetupSharx();

   virtual void vSetSetupSharx(tbool Direction);

   virtual ulword ulwGetSetupHighcut();

   virtual void vSetHighcut(tbool Direction);

   virtual tbool blGetAF();

   virtual void vToggleAFValue();

   virtual tbool blGetDDAStatus();

   virtual void vToggleDDAState();

   virtual void vExitAFList();

   virtual void vGetTunerCalData(GUI_String *out_result,ulword Calibration);

   virtual void vSetRadioTMEnter();
   virtual void vToggleFreezeBackgroundTuner();

   virtual tbool blGetFreezeBackgroundTuner();

   virtual tbool blGetLinearAudio();

   virtual void vToggelLinearAudio();

   virtual void vClearResetCounterValues();

   virtual void vGetResetCounterValue(GUI_String *out_result,ulword Counter);

   sGps_Info sGpsConversion(tF64 combinedToF64) const;

   sMapMatch_Info sMatchedPosValueConversion(const tS32 GpsValue) const;

   void vSendCommandtoHSI(tU32 u32CommandID, tU32 u32DataID) const;

   void vSendCommandtoHSI(tU32 u32CommandID);



   /** Has to be removed before release */


   virtual void vGetTMCStationPS(GUI_String *out_result);

   virtual void vGetTMCStationPI(GUI_String *out_result);

   virtual void vGetTMCStationCountryCode(GUI_String *out_result);

   virtual ulword ulwGetTMCStationSID();

   virtual ulword ulwGetTMCStationLTN();

   virtual void vGetTMCStationQuality(GUI_String *out_result);

   virtual void vGetTMCStation(GUI_String *out_result,ulword ListEntryNr);

   virtual ulword ulwGetTMCStation_Count();

   virtual void vSelectTMCStation(ulword ListEntryNr);

   /** Functions implemented in India */
   virtual void vGetDeadReckoningValues(GUI_String *out_result,ulword ListEntryNr);

   virtual ulword ulwGetOdometerDataUpdate(tBool OdoData);

   virtual void vGetTunerPS(GUI_String *out_result,ulword Tuner);

   virtual void vGetTunerHUB(GUI_String *out_result,ulword Tuner);

   virtual ulword ulwGetTunerMode();

   virtual void vSetTuneToAF(ulword ListEntryNr);

   virtual tbool blGetTunerRDSReg();

   virtual void vGetTunerPD(GUI_String *out_result,ulword Tuner);

   virtual tbool blGetTunerMeasureMode();

   virtual void vToggleRDSReg();

   virtual tbool blGetTASetup();

   virtual void vToggleTASetup();

   virtual void vToggleMeasureMode();

   virtual ulword ulwGetTunerCS(ulword Tuner);

   virtual ulword ulwGetActiveBand(ulword Tuner);

   virtual void vSetActiveBand(ulword Band);


   /** CAN SIGNALS API CALLS */	

   virtual void vGetCANAnalogMute(GUI_String *out_result);

   virtual void vGetCANKL15(GUI_String *out_result);

   virtual void vGetCANKL58D(GUI_String *out_result);

   virtual void vGetCANMuteBit(GUI_String *out_result);

   virtual void vGetCANMuteValue(GUI_String *out_result);

   virtual void vGetCANReverseGear(GUI_String *out_result);

   virtual void vGetCANSKontakt(GUI_String *out_result);

   virtual void vGetCANSpeedSignal(GUI_String *out_result);


   virtual ulword ulwGetActiveTuner();

   virtual void vSetActiveTuner(ulword Tuner);

   virtual void vSetTunerModeSetup(ulword Mode);

   virtual ulword ulwGetTunerModeSetup();

   virtual void vRestoreDefaultSettings();


   /** Service Mode Functions Implementation for B2 Sample */

   virtual ulword ulwGetServiceModeFieldStrength(ulword Type);

   virtual ulword ulwGetServiceModeQualityIndicatorValue();

   virtual void vGetServiceModeQualityString(GUI_String *out_result,ulword Type);

   virtual ulword ulwGetTunerRDSErrorRateService(ulword Tuner);

   virtual tbool blIsAntenna2Available();

   virtual void vGetTunerAntenna1(GUI_String *out_result,ulword Tuner);

   virtual ulword ulwGetTunerAntennaOne();	

   virtual void vGetTunerAntenna2(GUI_String *out_result,ulword Tuner);

   virtual void vGetWheel1RPM(GUI_String *out_result);

   virtual void vGetWheel2RPM(GUI_String *out_result);

   virtual void vGetWheel3RPM(GUI_String *out_result);

   virtual void vGetWheel4RPM(GUI_String *out_result);

   virtual void vGetAngle(GUI_String *out_result);

   virtual ulword ulwGetBestSatellite(ulword Satellite);

   virtual ulword ulwGetMediaStatus(ulword Drive);

   virtual tbool blGetSDTrace();

   virtual void vToggleSDTrace();

   virtual void vGetTMCLastMsgTimeDate(GUI_String *out_result);

   virtual void vGetTMCNoOfMsgs(GUI_String *out_result);

   virtual void vGetTMCNoOfMsgsSelectionArea(GUI_String *out_result);

   virtual void vGetCogCount(GUI_String *out_result);

   virtual void vGetWheelCircumference(GUI_String *out_result);

   virtual void vGetMapInfoValues(GUI_String *out_result,ulword Medium);

   /////////////////////// End of Plus Variant ////////////////////
   virtual void vSetRadioTestModeActive(ulword enable);

   virtual ulword ulwGetBTLinkQuality();

   virtual ulword ulwGetBTRSSI();

   virtual ulword ulwGetBTModuleMode();

   virtual void vSetBTModuleMode(ulword BTModuleMode);

   virtual ulword ulwBTModuleStatus();

   virtual void vGetSystemVoltage(GUI_String *out_result);

   virtual ulword ulwGetTroubleCodeListCount(tVoid);

   virtual void vClearTroubleCodeList(tVoid);

   virtual void vGetFGSHWVersion(GUI_String *out_result);

   virtual ulword ulwGetUSBMediaStatus();

   virtual ulword ulwGetCDDriveStatus();

   virtual void vGetUSBMemoryTotal(GUI_String *out_result);

   virtual void vGetIpodFirmwareVersion(GUI_String *out_result);

   virtual ulword ulwGetSrvTroubleCodeListElement(ulword Index);

   virtual ulword ulwGetSrvSystemStatus(ulword SystemStatus);

   virtual ulword ulwGetMFLKeyPressed();

   virtual void vServiceModeEntry();

   virtual void vServiceModeExit();

   virtual ulword ulwGetUsbDeviceSrvStatus();

   virtual void vGetUsbDeviceSrvID(GUI_String *out_result);

   virtual ulword ulwGetExtPhoneSignalState();

   virtual void vGetCsmEngineeringData(GUI_String *out_result,ulword ListEntryNumber);
   virtual ulword ulwGetCsmEngineering_Count();
   virtual void vStartCsmEngineering();

   tVoid vReadDataFromDataPool( tU32   u32DataPoolIndex, 
      tU8    u8DataPoolListColumnIndex,
      tU8    u8DataPoolListRowIndex,
      tVoid* pu8Data, 
      tU32   u32ByteCount);

   virtual void vBTDevDeviceAdress(GUI_String *out_result);

   virtual void vBTDevPIMOperationStatus(GUI_String *out_result);

   virtual void vBTDevPhonebookEntriesCount(GUI_String *out_result,ulword PhonebookType);

   virtual void vBTDevHfpAgSupportedFeatures(GUI_String *out_result);

   virtual ulword ulwBTDevAvpPlayStatus();

   virtual void vBTDevAvpSupportedPlayerStatus(GUI_String *out_result);

   virtual void vBTDevSMSSupportedFeatures(GUI_String *out_result);

   virtual void vBTDevSMSDeviceSystemStatus(GUI_String *out_result);

   virtual void vBTDevSMSSupportedNotification(GUI_String *out_result);

   virtual tbool blBTDevEnabledServices(ulword BTService);

   virtual tbool blBTDevConnectedServices(ulword BTService);

   virtual ulword ulwGetTelephoneMicrophoneConnectionStatus();

   virtual void vGetTelephoneMicrophoneConnectionValue(GUI_String *out_result);

   virtual ulword ulwBTDevAudioCodecUsed();


   /*APIs related to BT Accredetation test*/

   virtual void vBTDevTestSetHCIModeStatus(tbool Value);

   virtual tbool blBTDevTestGetHCIModeStatus();

   virtual tbool blBTDevTestGetECNREngineStatus();

   virtual void vBTDevTestToggleECNREngineStatus();

   virtual void vBTDevTestSetFrequency(ulword Value);

   virtual ulword ulwBTDevTestGetFrequency();

   virtual void vBTDevTestSetPacketType(ulword Value);

   virtual ulword ulwBTDevTestGetPacketType();

   virtual void vBTDevTestSetModulationValue(ulword Value);

   virtual ulword ulwBTDevTestGetModulationValue();

   virtual void vBTDevRunTest(ulword Test);

   virtual void vBTDevStopTest();

   virtual void vBTDevTestGetTestValues(GUI_String *out_result,ulword TestParameter);

   virtual void vBTDevTestGetLinkKey(GUI_String *out_result);

   virtual void vCreateScreenShot();

   virtual tbool blGetScreenShotState();

   virtual void vScreenShotResetState();

   virtual void vToggleScreenShotSetting();

   virtual tbool blGetScreenShotSetting();

   /******************APIs for SXM******************/
   virtual void vGetSXMServiceDTMMonData(GUI_String *out_result, ulword LineNo);
   virtual ulword ulwGetSXMSettingsMenuData(ulword LineNo);
   virtual void vGetSXMRadioID(GUI_String *out_result);
   virtual void vStartMethodforUPCLID();
   virtual ulword ulwGetSXMDataServiceSubInfo();
   virtual tbool blWaitSyncforSXMDiag();
   virtual void vActivateSXMDTM(ulword Action);
   virtual void vClearSXMDTMFunctions(ulword LineNo);
   virtual ulword ulwGetSXMSTMDataParam1(ulword LineNo);
   virtual void vGetSXMSTMDataParam2(GUI_String *out_result,ulword LineNo);
   virtual ulword ulwGetSXMTrafficServiceSubInfo();
   virtual ulword ulwGetSXMAntennaStatus();
   virtual void vGetSXMSuspendDateinLocaltime(bpstl::string& SuspendedDate);
   virtual void vToggleExternalDiagMode();
   virtual tbool blExternalDiagModeState();
   virtual tbool blGetSxmDTMPopupStatus();
   virtual ulword ulwGetCountry();
   virtual ulword ulwGetGender();
   virtual ulword ulwGetSXMServiceStatus(ulword DPID);
};

#endif //HSA_TESTMODE_H






