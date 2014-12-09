/************************************************************************
* FILE:         clTuner_CCAhandler.cpp
* PROJECT:      FORD HSRNS
* SW-COMPONENT: 
*----------------------------------------------------------------------
*
* DESCRIPTION: Tuner CCA Client Handler 
*              
*----------------------------------------------------------------------
* COPYRIGHT:    (c) 2004 Robert Bosch GmbH, Hildesheim
* HISTORY:      
* Date      | Author                 | Modification
* 23.05.05  | RBIN-CM-DI/ESA Ajith   | initial version
* 13.12.05    Guru Prasad RBIN/EDI1    MB3 features implementation
* 20.12.05    Guru Prasad RBIN/EDI1    Converted Traces from Strings to defines
* 17.01.06    Guru Prasad RBIN/EDI1    MB3 Testmode & MB4 implmentation   
* 07.02.06    Guru Prasad RBIN/EDI1    FM/AM Direct Frequency input implmentation 
* 21.02.06    Guru Prasad RBIN/EDI1    support through DP update for TA & TP Status display 
* 20.06.06  | Pavan R     RBIN/EDI1    Tuner support for VR
*************************************************************************/


//Macro for Byte Stuffing --- Remove Lint
//

// precompiled header, should always be the first include
#include "precompiled.hh"

//#define MAKEU16(u8Low, u8High) ( (tU16) (( (tU8)(u8Low)) | ( ( (tU16)( (tU8) (u8High)) ) << 8) ) )

/*To include statemachine values*/
//#include "statemachine_if.h"

//#include "clFI_TUNER.h"             // for using the Interfaces of clFI_TUNER
#include "fituner/clTuner_CCAhandler.h"

class hmicca_tclApp;  //required for socket interfaces during simulation
// see precompiled.hh		#include "../CompMan/clhsi_cmmngr.h"
#include "CompMan/clHSI_Radio.h"
#include "fc_tmctuner_if.h"
#include "fituner/HSI_TUN_DEFINES.h"
// see precompiled.hh		#include "service_if.h"	


//#define  RTS__FLAG_OFF        0

//#define TUN_AREA_NAR	0x02

//#define TUN_TM_FG		0X01
//#define TUN_TM_BG		0X02
//#define TUN_TM_FG_BG	0X03
#define SINGLE_TUNER	0X00
//#define DOUBLE_TUNER	0X01
//#define DOUBLE_DIVERSITY_TUNER	0X02


#define TM_BAND_FM          0
//#define TM_BAND_FM_NON_RDS  1
#define TM_BAND_AM          2
#define TM_BAND_DAB         1


#ifdef VARIANT_S_FTR_ENABLE_TRC_GEN
#define ETG_DEFAULT_TRACE_CLASS TR_CLASS_HSI_TUNER_MSG
#include "trcGenProj/Header/clTuner_CCAhandler.cpp.trc.h"
#endif

#ifdef VARIANT_S_FTR_ENABLE_HMI_TUNER_ITG5
#include "datapool/clDataPool.h"
static clDataPool DataPool;
#endif

/* +++
MESSAGE MAP:
Such a map and an own handler is needed for every service you are accessing!
Enter the function IDs (FID) and the corresponding functions here.
The function will be called when a message with the corresponding FID arrives
+++ */

BEGIN_MSG_MAP(clTuner_CCAhandler, ahl_tclBaseWork)   //  ajith - added from the message catelogue

   // This is taken from Middle ware Tuner Service Fi(.pdf file) V 1.0.0

   //Proprties
   ON_MESSAGE(MIDW_TUNERFI_C_U16_FID_TUN_G_STATIC_LIST_ELEMENT ,                             // FID= 0x099
   vHandleFID_G_GET_STATIC_LIST_ELEMENT_From_FCTuner)     
   ON_MESSAGE(MIDW_TUNERFI_C_U16_FID_TUN_G_TUNER_DATA,                                       // FID= 0x041
   vHandleFID_G_TUNER_DATA_From_FCTuner)                        
   ON_MESSAGE(MIDW_TUNERFI_C_U16_FID_TUN_G_TMC_INFO,                                         // FID= 0x0AA
   vHandleFID_G_TMC_INFO_From_FCTuner)                        

   ON_MESSAGE(MIDW_TUNERFI_C_U16_FID_TUN_G_SET_FEATURE,                                      // FID= 0x077
   vHandleFID_G_SET_FEATURE_From_FCTuner)                    
   ON_MESSAGE(MIDW_TUNERFI_C_U16_FID_TUN_G_SET_HICUT,                                        //FID = 0x07b
   vHandleFID_G_SET_HICUT_From_FCTuner ) 
   ON_MESSAGE(MIDW_TUNERFI_C_U16_FID_TUN_G_AFLIST_V2,                                        //FID = 0x05D
   vHandleFID_G_AFLIST_V2_From_FCTuner )          
   ON_MESSAGE(MIDW_TUNERFI_C_U16_FID_TUN_G_AFLIST_V2_ELEMENT,                                //FID = 0x05F
   vHandleFID_G_AFLIST_V2_ELEMENT_From_FCTuner )    

   ON_MESSAGE(MIDW_TUNERFI_C_U16_FID_TUN_G_MAKE_CONFIG_LIST,
   vHandleFID_G_MAKE_CONFIG_LIST_From_FCTuner)
   ON_MESSAGE(MIDW_TUNERFI_C_U16_FID_TUN_G_CONFIG_LIST_ELEMENT,
   vHandleFID_G_CONFIG_LIST_ELEM_From_FCTuner)
   ON_MESSAGE(MIDW_TUNERFI_C_U16_FID_TUN_G_STATION_DATA_VAG,                                       // FID= 0x041
   vHandleFID_G_STATION_DATA_VAG_From_FCTuner)
   ON_MESSAGE(MIDW_TUNERFI_C_U16_FID_TUN_G_ATSTATIONINFO,
   vHandleFID_G_ATSTATIONINFO_VAG_From_FCTuner)
	
	ON_MESSAGE(MIDW_TUNERFI_C_U16_FID_TUN_G_TAINFO, 
	vHandleFID_G_TAINFO_From_FCTuner)
	


   ON_MESSAGE(MIDW_TUNERFI_C_U16_FID_TUN_G_MIXEDPRESET_LIST_ELEMENT,
   vHandleFID_G_MIXEDPRESET_LIST_ELEMENT_From_FCTuner)

   ON_MESSAGE(MIDW_TUNERFI_C_U16_FID_TUN_G_BAND_PLUS_STATION_INFO,
   vHandleFID_G_BAND_PLUS_STATION_INFO_From_FCTuner)


   ON_MESSAGE(MIDW_TUNERFI_C_U16_FID_TUN_G_RADIO_TEXT,                                      // FID= 0x057
   vHandleFID_G_RADIO_TEXT_From_FCTuner)
   //Added by Pavan R
   ON_MESSAGE(MIDW_TUNERFI_C_U16_FID_TUN_G_SET_SHARXLEVEL,
   vHandleFID_G_SET_SHARXLEVEL_From_FCTuner)                   //FID = 0xA7
   ON_MESSAGE(MIDW_TUNERFI_C_U16_FID_TUN_G_SETUP_DDADDS,
   vHandleFID_G_SET_DDADDS_From_FCTuner)					   //FID = 0x0B3
   ON_MESSAGE(MIDW_TUNERFI_C_U16_FID_TUN_G_GET_DATA_TESTMODE_DDADDS,                                //FID = 0x130
   vHandleFID_G_GET_DATA_TESTMODE_DDADDS_From_FCTuner )
   ON_MESSAGE(MIDW_TUNERFI_C_U16_FID_TUN_G_SET_DDA,
   vHandleFID_G_SET_DDA_From_FCTuner)
   ON_MESSAGE(MIDW_TUNERFI_C_U16_FID_TUN_G_SET_AF,
   vHandleFID_G_SET_AF_From_FCTuner)
   ON_MESSAGE(MIDW_TUNERFI_C_U16_FID_TUN_G_SET_MEASUREMODE,
   vHandleFID_G_SET_MEASUREMODE_From_FCTuner)
   ON_MESSAGE(MIDW_TUNERFI_C_U16_FID_TUN_G_SET_RDSREG,
   vHandleFID_G_SET_RDSreg_From_FCTuner)

   ON_MESSAGE(MIDW_TUNERFI_C_U16_FID_TUN_G_CALIBRATION_DATA,
   vHandleFID_G_GET_CALIBRATION_To_FCTuner)

   ON_MESSAGE(MIDW_TUNERFI_C_U16_FID_TUN_G_ATSEEK,
   vHandleFID_G_AtSeek_From_FCTuner)

#ifdef VARIANT_S_FTR_ENABLE_HMI_TUNER_ITG5
   ON_MESSAGE(MIDW_TUNERFI_C_U16_FID_TUN_G_DEMODULATION_SWITCH_STATUS,
   vHandleFID_G_DemodulationSwitch_From_FCTuner)
   
   ON_MESSAGE(MIDW_TUNERFI_C_U16_FID_TUN_G_UPDATELANDSCAPE_PROGRESS,
   vHandleFID_TUN_G_UPDATELANDSCAPE_PROGRESS_From_FCTuner)

   ON_MESSAGE(MIDW_TUNERFI_C_U16_FID_TUN_G_GET_STATIC_LIST,
   vHandleFID_G_Get_StaticList_From_FCTuner)

   ON_MESSAGE(MIDW_TUNERFI_C_U16_FID_TUN_G_HD_PSD_DATA,
   vHandleFID_G_Get_PSD_Data_From_FCTuner)
   
   ON_MESSAGE(MIDW_TUNERFI_C_U16_FID_TUN_G_HD_TEST_MODE_DATA,
   vHandleFID_TUN_G_HD_TEST_MODE_DATA_From_FCTuner)
#endif


   // common Method result Handler 

   ON_MESSAGE(MIDW_TUNERFI_C_U16_FID_TUN_S_SET_STATIC_LIST,
   vHandleALL_FID_MethodResult_Handler_From_FCTuner)        // FID= 0x80
   ON_MESSAGE(MIDW_TUNERFI_C_U16_FID_TUN_S_SET_FEATURE,
   vHandleALL_FID_MethodResult_Handler_From_FCTuner)        // FID= 0x76  
   ON_MESSAGE(MIDW_TUNERFI_C_U16_FID_TUN_S_SET_HICUT,
   vHandleALL_FID_MethodResult_Handler_From_FCTuner)         //FID= 0x7a
   ON_MESSAGE(MIDW_TUNERFI_C_U16_FID_TUN_S_OPERATION ,
   vHandleALL_FID_MethodResult_Handler_From_FCTuner)         //FID= 0x78
   ON_MESSAGE(MIDW_TUNERFI_C_U16_FID_TUN_S_STOP_ANNOUNCEMENT ,
   vHandleALL_FID_MethodResult_Handler_From_FCTuner)         //FID= 0x4A
   ON_MESSAGE(MIDW_TUNERFI_C_U16_FID_TUN_S_DIRECT_FREQUENCY ,
   vHandleALL_FID_MethodResult_Handler_From_FCTuner)         //FID= 0x32        
   ON_MESSAGE(MIDW_TUNERFI_C_U16_FID_TUN_S_SEL_LIST_ELEM ,
   vHandleALL_FID_MethodResult_Handler_From_FCTuner)
   ON_MESSAGE(MIDW_TUNERFI_C_U16_FID_TUN_S_AUTOSTORE_STATIC_LIST ,
   vHandleALL_FID_MethodResult_Handler_From_FCTuner)
   ON_MESSAGE(MIDW_TUNERFI_C_U16_FID_TUN_S_AUDIO_FG_BG_NEW,
   vHandleALL_FID_MethodResult_Handler_From_FCTuner)
   ON_MESSAGE(MIDW_TUNERFI_C_U16_FID_TUN_S_SET_PI,
   vHandleALL_FID_MethodResult_Handler_From_FCTuner)         //FID= 0x42
   ON_MESSAGE(MIDW_TUNERFI_C_U16_FID_TUN_S_AFLIST_V2,
   vHandleALL_FID_MethodResult_Handler_From_FCTuner)
   ON_MESSAGE(MIDW_TUNERFI_C_U16_FID_TUN_S_SETUP_DDADDS,
   vHandleALL_FID_MethodResult_Handler_From_FCTuner)		//FID = 0x0B3
   ON_MESSAGE(MIDW_TUNERFI_C_U16_FID_TUN_S_CALIBRATION_DATA,
   vHandleALL_FID_MethodResult_Handler_From_FCTuner)
   ON_MESSAGE(MIDW_TUNERFI_C_U16_FID_TUN_S_SET_MIXEDPRESET_LIST,
   vHandleALL_FID_MethodResult_Handler_From_FCTuner)
   ON_MESSAGE(MIDW_TUNERFI_C_U16_FID_TUN_S_STATIONLIST_EXIT,
   vHandleALL_FID_MethodResult_Handler_From_FCTuner)

#ifdef VARIANT_S_FTR_ENABLE_HMI_TUNER_ITG5
   ON_MESSAGE(MIDW_TUNERFI_C_U16_FID_TUN_S_SET_DEMODULATION_SWITCH,
   vHandleALL_FID_MethodResult_Handler_From_FCTuner)
   
   ON_MESSAGE(MIDW_TUNERFI_C_U16_FID_TUN_S_CONFIGLIST_UPDATE,
   vHandle_FID_TUN_S_CONFIGLIST_UPDATE_From_FCTuner)

   ON_MESSAGE(MIDW_TUNERFI_C_U16_FID_TUN_S_SELECT_HD_AUDIOPRGM,
      vHandleALL_FID_MethodResult_Handler_From_FCTuner)
#endif

END_MSG_MAP()


/*************************************************************************
*  FUNCTION:    ~clTuner_CCAhandler()
*
*  DESCRIPTION: Destructor ( Cleanup )
*
*  PARAMETER:   NONE
*
*  RETURNVALUE: NONE
*
*************************************************************************/
clTuner_CCAhandler::~clTuner_CCAhandler()
{
   m_poTrace = NULL;
   poFI = NULL;





   if(poAFListManager != NULL)
   {
      OSAL_DELETE poAFListManager;
      poAFListManager =NULL;
   }

   if( poAFListSink !=  NULL)
   {
      OSAL_DELETE poAFListSink;
      poAFListSink = NULL;
   }





   if( m_pHSI_Radio != NULL )
   {
	   OSAL_DELETE m_pHSI_Radio;
	   m_pHSI_Radio = NULL;
   }

   if( m_pHSI_TestModeRadio != NULL )
   {
	   OSAL_DELETE m_pHSI_TestModeRadio;
	   m_pHSI_TestModeRadio = NULL;
   }

   if( m_pHSI_TunerMaster != NULL )
   {
	   OSAL_DELETE m_pHSI_TunerMaster;
	   m_pHSI_TunerMaster = NULL;
   }


   m_TMCListRequested  = FALSE;
   m_U8NoOfTMCStations = 0;

   m_FMStationListRequested = FALSE;
   m_U8NoOfFMStations = 0;


   m_u32Frequency = 0;
   m_u32AFFrequency = 0;

   bSeekState = false;
   bDirectFreq = false;
   m_pcPSName[0] = 0;
   m_u8StatListElemID = 0;
   m_u8BandFromHMI = 0;
   bcheckpresetlist = true;

#ifdef VARIANT_S_FTR_ENABLE_HMI_TUNER_ITG5
   m_u8DemodSwitch = 0;
   m_bAMStationListRequested = FALSE;
   m_u8NoOfAMStations = 0;
#endif
}

/*************************************************************************
*  FUNCTION:    clTuner_CCAhandler::clTuner_CCAhandler() 
*
*  DESCRIPTION: Constructor ( Initialize the Class members) 
*
*  PARAMETER:   NONE
*
*  RETURNVALUE: NONE 
*
*************************************************************************/


clTuner_CCAhandler::clTuner_CCAhandler(clFi_TUNER * poFiPtr)
{
   m_u8CurrentPresetButton       = 0xFF;    //  set to invalid value
   m_b8CurrentAudioSink.u8Value  = 0;
   m_pHSI_Radio                  = NULL;
   m_pHSI_TestModeRadio          = NULL;
   m_pHSI_TunerMaster            = NULL;

   m_TMCListRequested            = FALSE;
   m_U8NoOfTMCStations           = 0;

   m_FMStationListRequested      = FALSE;
   m_U8NoOfFMStations = 0;

 
   m_u32Frequency = 0;
   m_u32AFFrequency = 0;

   bSeekState = false;
   bDirectFreq = false;
   m_pcPSName[0] = 0;
   m_u8StatListElemID = 0;
   m_u8BandFromHMI = 0;
   bcheckpresetlist = true;

   // Configure base class with hsi TUNER trace class 
   this->vSetM_MY_TR_CLASS(TR_CLASS_HSI_TUNER_MSG);

   poFI  = poFiPtr; //poFI = NULL;





   poAFListSink      = OSAL_NEW clTun_AFListSink();

   poAFListManager   = OSAL_NEW clTun_ListManager(  poAFListSink ,
      FORD_HSRNS_AFLIST_MAXSIZE ,
      FORD_HSRNS_MAX_AFLIST_DISPLAY
      );




   u8TunMode         = 0;
   u8NoTuners  = 0;

   u8BG_AUD_ACTIVITY = 0;
   m_bAutoCmpcmdAtStartUp = TRUE;

#ifdef VARIANT_S_FTR_ENABLE_HMI_TUNER_ITG5
   m_u8DemodSwitch = 0;
   m_bAMStationListRequested     = FALSE;
   m_u8NoOfAMStations            = 0;
#endif

   oFiRegHelper.bAddFunctionID(MIDW_TUNERFI_C_U16_FID_TUN_G_STATIC_LIST_ELEMENT  );
   oFiRegHelper.bAddFunctionID(MIDW_TUNERFI_C_U16_FID_TUN_G_SET_FEATURE );
   oFiRegHelper.bAddFunctionID(MIDW_TUNERFI_C_U16_FID_TUN_G_SET_HICUT );
   oFiRegHelper.bAddFunctionID(MIDW_TUNERFI_C_U16_FID_TUN_G_AFLIST_V2);
   oFiRegHelper.bAddFunctionID(MIDW_TUNERFI_C_U16_FID_TUN_G_AFLIST_V2_ELEMENT);

   oFiRegHelper.bAddFunctionID(MIDW_TUNERFI_C_U16_FID_TUN_G_MAKE_CONFIG_LIST);
   oFiRegHelper.bAddFunctionID(MIDW_TUNERFI_C_U16_FID_TUN_G_CONFIG_LIST_ELEMENT);
   oFiRegHelper.bAddFunctionID(MIDW_TUNERFI_C_U16_FID_TUN_G_MIXEDPRESET_LIST_ELEMENT);
   oFiRegHelper.bAddFunctionID(MIDW_TUNERFI_C_U16_FID_TUN_G_AUDIO_FG_BG_NEW);
   oFiRegHelper.bAddFunctionID(MIDW_TUNERFI_C_U16_FID_TUN_G_SET_SHARXLEVEL);
   oFiRegHelper.bAddFunctionID(MIDW_TUNERFI_C_U16_FID_TUN_G_RADIO_TEXT);
   oFiRegHelper.bAddFunctionID(MIDW_TUNERFI_C_U16_FID_TUN_G_SETUP_DDADDS);
   oFiRegHelper.bAddFunctionID(MIDW_TUNERFI_C_U16_FID_TUN_G_TAINFO);

   oFiRegHelper.bAddFunctionID(MIDW_TUNERFI_C_U16_FID_TUN_G_GET_DATA_TESTMODE_DDADDS);
   oFiRegHelper.bAddFunctionID(MIDW_TUNERFI_C_U16_FID_TUN_G_ATSTATIONINFO,true);
   oFiRegHelper.bAddFunctionID(MIDW_TUNERFI_C_U16_FID_TUN_G_SET_AF);
   oFiRegHelper.bAddFunctionID(MIDW_TUNERFI_C_U16_FID_TUN_G_ATSEEK);

   oFiRegHelper.bAddFunctionID(MIDW_TUNERFI_C_U16_FID_TUN_G_SET_RDSREG);
   oFiRegHelper.bAddFunctionID(MIDW_TUNERFI_C_U16_FID_TUN_G_SET_MEASUREMODE);
   oFiRegHelper.bAddFunctionID(MIDW_TUNERFI_C_U16_FID_TUN_G_SET_DDA);
   oFiRegHelper.bAddFunctionID(MIDW_TUNERFI_C_U16_FID_TUN_G_TMC_INFO);
   oFiRegHelper.bAddFunctionID(MIDW_TUNERFI_C_U16_FID_TUN_G_CALIBRATION_DATA);
   oFiRegHelper.bAddFunctionID(MIDW_TUNERFI_C_U16_FID_TUN_G_BAND_PLUS_STATION_INFO);
   oFiRegHelper.bAddFunctionID(MIDW_TUNERFI_C_U16_FID_TUN_G_GET_DATA_TMC);

#ifdef VARIANT_S_FTR_ENABLE_HMI_TUNER_ITG5
   oFiRegHelper.bAddFunctionID(MIDW_TUNERFI_C_U16_FID_TUN_G_DEMODULATION_SWITCH_STATUS);
   oFiRegHelper.bAddFunctionID(MIDW_TUNERFI_C_U16_FID_TUN_G_UPDATELANDSCAPE_PROGRESS);
   oFiRegHelper.bAddFunctionID(MIDW_TUNERFI_C_U16_FID_TUN_G_GET_STATIC_LIST);
   oFiRegHelper.bAddFunctionID(MIDW_TUNERFI_C_U16_FID_TUN_G_HD_PSD_DATA);
   oFiRegHelper.bAddFunctionID(MIDW_TUNERFI_C_U16_FID_TUN_G_HD_TEST_MODE_DATA);
#endif


}




/*************** Message Handler functions******************/


/*************************************************************************
*  FUNCTION:    clTuner_CCAhandler::vHandleFID_G_TUNER_DATA_From_FCTuner()
*
* DESCRIPTION: handler to extract data from the status message from server for
*              FID= FID_TUN_G_TUNER_DATA
*
* PARAMETER:   amt_tclServiceData* poMessage ( data message to process )
*
* RETURNVALUE: NONE
*
* History:
* InitialVersion
*
*************************************************************************/
tVoid clTuner_CCAhandler::vHandleFID_G_TUNER_DATA_From_FCTuner(
   amt_tclServiceData* poMessage)
{

#ifdef VR_SRV_ENABLE
   tU8 u8CmdVR = 0;
#endif

   if ( m_poTrace != NULL )
   {
      m_poTrace->vTrace(  TR_LEVEL_HMI_INFO,
         TR_CLASS_HSI_TUNER_MSG,
         (tU16)TUN_FID_TUNER_DATA_FROM_SERV);
   }

   /* check the parameter for not NULL and then for validity of a messsage */
   if ( (poMessage != NULL) && (poMessage->bIsValid()) )
   {
      switch (poMessage->u8GetOpCode())  // get the opcode from the message
      {
      case AMT_C_U8_CCAMSG_OPCODE_STATUS:
         {
            vStartInitSequence(MIDW_TUNERFI_C_U16_FID_TUN_G_TUNER_DATA);

            /* separate handler for data extraction as data size is too big*/


            /** read the datapool Element value and send the right Auto Compare parameters command to Radio Lib */
            tU8 u8CurrentBank = INVALID_PRESET_NUMBER;

            if(NULL != poFI->m_poDataPool)
            {
               poFI->m_poDataPool->u32dp_get( DPTUNER__CURRENT_BANK,
                  &u8CurrentBank,
                  sizeof(u8CurrentBank));
            }

            if(TRUE == m_bAutoCmpcmdAtStartUp)
            {
               m_bAutoCmpcmdAtStartUp = FALSE;

               if((FM1_BANK == u8CurrentBank) || (FM2_BANK == u8CurrentBank) || (AM_BANK == u8CurrentBank))
               {
                  vTrace(TR_LEVEL_HMI_INFO,"AUTOCOMPARE PARAMETER COMMAND to Radio LIB From CCA %d",u8CurrentBank);
                  if(NULL != pHSI_GetRadio() )
                  {
                     pHSI_GetRadio()->vSwitchBankAutoCompareInCM(u8CurrentBank);
                  }
               }
               else
               {
                  // Nothing to do
               }
            }

#ifdef VR_SRV_ENABLE
            if( poFI != NULL)
            {
               if(clHSIDeviceGeneral::m_poDataPool !=   NULL)
               {
                  clHSIDeviceGeneral::m_poDataPool -> u32dp_get( DPVC__INPUT_CMD ,
                     &u8CmdVR , sizeof(u8CmdVR) );

                  if(VR_TUN_SELECT_STATION == u8CmdVR ||
                     VR_TUN_SELECT_BAND_MEM_BANK == u8CmdVR ||
                     VR_TUN_AUTOSTORE == u8CmdVR  ||
                     VR_TUN_STORE_STATION == u8CmdVR)
                  {
                     poFI->WriteDataToDp(TUNER_HSI_NO_ERROR,
                        HSI_NO_ERROR);
                  }
               }

            }
#endif

            break;
         }
      case AMT_C_U8_CCAMSG_OPCODE_ERROR:
         {
#ifdef VR_SRV_ENABLE
            tU8 u8VrStatus = 0;

            if(poFI != NULL)
            {
               if(clHSIDeviceGeneral::m_poDataPool != NULL)
               {
                  clHSIDeviceGeneral::m_poDataPool -> u32dp_get( DPVC__INPUT_CMD ,
                     &u8CmdVR , sizeof(u8CmdVR) );
               }
            }

            if(VR_TUN_SELECT_STATION == u8CmdVR ||
               VR_TUN_SELECT_BAND_MEM_BANK == u8CmdVR ||
               VR_TUN_AUTOSTORE == u8CmdVR  ||
               VR_TUN_STORE_STATION == u8CmdVR)
            {
               u8VrStatus = VR_INPUT_CMD_TRUE;
            }
            else
            {
               u8VrStatus = VR_INPUT_CMD_FALSE;
            }
#endif
            /* handle error message in a separate function */
            vHandleCCAErrorMessage( poMessage );

            break;
         }

      default : // unknown opcode
         {
#ifdef VR_SRV_ENABLE
            if(poFI != NULL)
            {
               if(clHSIDeviceGeneral::m_poDataPool != NULL)
               {
                  clHSIDeviceGeneral::m_poDataPool -> u32dp_get( DPVC__INPUT_CMD ,
                     &u8CmdVR , sizeof(u8CmdVR) );

                  if(VR_TUN_SELECT_STATION == u8CmdVR ||
                     VR_TUN_SELECT_BAND_MEM_BANK == u8CmdVR ||
                     VR_TUN_AUTOSTORE == u8CmdVR  ||
                     VR_TUN_STORE_STATION == u8CmdVR)
                  {
                     poFI->WriteDataToDp(TUNER_HSI_ERROR_UNKNOWN,
                        HSI_ERROR_UNKNOWN);
                  }
               }
            }
#endif

            /* send error message back to server for unkown opcode */
            vSendErrorMessage(   poMessage->u16GetFunctionID(),
               AMT_C_U16_ERROR_INVALID_OPCODE);
            break;
         }
      } // end of switch
   }
}

/*************************************************************************
* FUNCTION:     clTuner_CCAhandler::vHandleFID_G_GET_STATIC_LIST_ELEMENT_From_FCTuner()
*
* DESCRIPTION: handler to extract data from the status message from server for FID= 
FID_TUN_G_GET_STATIC_LIST_ELEMENT 
*
* PARAMETER:    amt_tclServiceData* poMessage ( data message to process )
*
*
* RETURNVALUE: NONE
*
* History:
* InitialVersion
* 
*************************************************************************/
tVoid clTuner_CCAhandler::vHandleFID_G_GET_STATIC_LIST_ELEMENT_From_FCTuner(
   amt_tclServiceData* poMessage )
{
#ifdef VR_SRV_ENABLE
   tU8 u8CmdVR = 0;
#endif

   if ( m_poTrace != NULL )
   {
      m_poTrace->vTrace(  TR_LEVEL_HMI_INFO,
         TR_CLASS_HSI_TUNER_MSG,
         (tU16)TUN_FID_STATIC_LIST_ELE_FROM_SERV);
   }

   /* check the parameter for not NULL and then for validity of a messsage */
   if ( (poMessage != NULL) && (poMessage->bIsValid()) )
   {
      switch (poMessage->u8GetOpCode())    // get the opcode from the message
      {
      case AMT_C_U8_CCAMSG_OPCODE_STATUS:    
         {            
            vStartInitSequence(MIDW_TUNERFI_C_U16_FID_TUN_G_STATIC_LIST_ELEMENT);

            /* separate handler for data extraction as data size is too big*/
            vHandleStaticListElement( poMessage );

#ifdef VR_SRV_ENABLE            
            if( poFI != NULL )
            {
               if(clHSIDeviceGeneral::m_poDataPool != NULL)
               {
                  clHSIDeviceGeneral::m_poDataPool -> u32dp_get( DPVC__INPUT_CMD , 
                     &u8CmdVR , sizeof(u8CmdVR) );


                  if(VR_TUN_SET_TUNER_SETTING == u8CmdVR ||
                     VR_TUN_SELECT_BAND_MEM_BANK == u8CmdVR ||
                     VR_TUN_AUTOSTORE == u8CmdVR ||
                     VR_TUN_STORE_STATION == u8CmdVR)
                  {
                     poFI->WriteDataToDp(TUNER_HSI_NO_ERROR,  HSI_NO_ERROR); 
                  }
               }
            }
#endif            
            break;
         }

      case AMT_C_U8_CCAMSG_OPCODE_ERROR:
         {
#ifdef VR_SRV_ENABLE
            tU8 u8VrStatus = 0;

            if(poFI != NULL)
            {
               if(clHSIDeviceGeneral::m_poDataPool != NULL)
               {
                  clHSIDeviceGeneral::m_poDataPool -> u32dp_get( DPVC__INPUT_CMD , 
                     &u8CmdVR , sizeof(u8CmdVR) );
               }
            }

            if(VR_TUN_SET_TUNER_SETTING == u8CmdVR ||
               VR_TUN_SELECT_BAND_MEM_BANK == u8CmdVR ||
               VR_TUN_AUTOSTORE == u8CmdVR ||
               VR_TUN_STORE_STATION == u8CmdVR)
            {
               u8VrStatus = VR_INPUT_CMD_TRUE;
            }
            else
            {
               u8VrStatus = VR_INPUT_CMD_FALSE;

            }
#endif
            /* handle error message in a separate function */
            vHandleCCAErrorMessage( poMessage );

            break;
         } 

      default : // unknown opcode
         {
#ifdef VR_SRV_ENABLE
            if(poFI != NULL)
            {
               if(clHSIDeviceGeneral::m_poDataPool != NULL)
               {
                  clHSIDeviceGeneral::m_poDataPool -> u32dp_get( DPVC__INPUT_CMD , 
                     &u8CmdVR , sizeof(u8CmdVR) );

                  if(VR_TUN_SET_TUNER_SETTING == u8CmdVR ||
                     VR_TUN_SELECT_BAND_MEM_BANK == u8CmdVR ||
                     VR_TUN_AUTOSTORE == u8CmdVR ||
                     VR_TUN_STORE_STATION == u8CmdVR)
                  {
                     poFI->WriteDataToDp(TUNER_HSI_ERROR_UNKNOWN,
                        HSI_ERROR_UNKNOWN);
                  }
               }
            }
#endif

            /* send error message back to server for unkown opcode */
            vSendErrorMessage(   poMessage->u16GetFunctionID(),            
               AMT_C_U16_ERROR_INVALID_OPCODE);
            break;
         }
      } // end of switch
   }
}



/*************************************************************************
*  FUNCTION:    clTuner_CCAhandler::vHandleFID_G_SET_FEATURE_From_FCTuner(  )
*
* DESCRIPTION: handler to extract data from the status message from server 
*              for FID= FID_TUN_G_SET_FEATURE   
*
*
* PARAMETER:   amt_tclServiceData* poMessage ( data message to process )
*
* RETURNVALUE: NONE
*
* History:
* InitialVersion
* 
*************************************************************************/
tVoid clTuner_CCAhandler::vHandleFID_G_SET_FEATURE_From_FCTuner( 
   amt_tclServiceData* poMessage )
{
#ifdef VR_SRV_ENABLE
   tU8 u8CmdVR = 0;
#endif

   if ( NULL !=  m_poTrace)
   {
      m_poTrace->vTrace(  TR_LEVEL_HMI_INFO, 
         TR_CLASS_HSI_TUNER_MSG,
         (tU16)TUN_FID_SET_FEATURE_FROM_SERV );

   }

   /* check the parameter for not NULL and then for validity of a messsage */
   if ( (poMessage != NULL) && (poMessage->bIsValid()) )
   {
      switch (poMessage->u8GetOpCode())  // get the opcode from the message
      {
      case AMT_C_U8_CCAMSG_OPCODE_STATUS:
         {
            vStartInitSequence(MIDW_TUNERFI_C_U16_FID_TUN_G_SET_FEATURE);

            /* extract the data from AMT Class to FI Class */
            midw_tunerfi_tclMsgFID_TUN_G_SET_FEATUREStatus oMsgData;			
            vGetDataFromAmt( poMessage , oMsgData );

            // extract the features info
            tU32 u32Features = oMsgData.b32_Features.u32Value;  

            if ( NULL !=  m_poTrace)
            {
               m_poTrace->vTrace(  TR_LEVEL_HMI_INFO,
                  TR_CLASS_HSI_TUNER_MSG,
                  (tU16)TUN_FEATURES_FROM_SERV, 
                  ONE_ELEMENT, 
                  &u32Features);
            }

            if(poFI != NULL)
            {
               poFI->WriteFeaturesFlagToDP( u32Features);
            }

#ifdef VR_SRV_ENABLE
            if( poFI != NULL )
            {
               if(clHSIDeviceGeneral::m_poDataPool != NULL)
               {
                  clHSIDeviceGeneral::m_poDataPool -> u32dp_get( DPVC__INPUT_CMD , 
                     &u8CmdVR , sizeof(u8CmdVR) );

                  if(VR_TUN_SET_TUNER_SETTING == u8CmdVR )
                  {
                     poFI->WriteDataToDp(TUNER_HSI_NO_ERROR,
                        HSI_NO_ERROR); 
                  }
               }
            }
#endif      
            break;
         }

      case AMT_C_U8_CCAMSG_OPCODE_ERROR:
         {
#ifdef VR_SRV_ENABLE
            tU8 u8VrStatus = 0;

            if(poFI != NULL)
            {
               if(clHSIDeviceGeneral::m_poDataPool != NULL)
               {
                  clHSIDeviceGeneral::m_poDataPool -> u32dp_get( DPVC__INPUT_CMD , 
                     &u8CmdVR , sizeof(u8CmdVR) );
               }
            }
            if(VR_TUN_SET_TUNER_SETTING == u8CmdVR )
            {
               u8VrStatus = VR_INPUT_CMD_TRUE;
            }
            else
            {
               u8VrStatus = VR_INPUT_CMD_FALSE;

            }

#endif
            /* handle error message in a separate function */
            vHandleCCAErrorMessage( poMessage );


            break;
         } 

      default : // unknown opcode
         {
#ifdef VR_SRV_ENABLE
            if(poFI != NULL)
            {
               if(clHSIDeviceGeneral::m_poDataPool != NULL)
               {
                  clHSIDeviceGeneral::m_poDataPool -> u32dp_get( DPVC__INPUT_CMD , 
                     &u8CmdVR , sizeof(u8CmdVR) );

                  if(VR_TUN_SET_TUNER_SETTING == u8CmdVR )
                  {
                     poFI->WriteDataToDp(TUNER_HSI_ERROR_UNKNOWN,
                        HSI_ERROR_UNKNOWN); 
                  }
               }
            }
#endif

            /* send error message back to server for unkown opcode */
            vSendErrorMessage(   poMessage->u16GetFunctionID(),            
               AMT_C_U16_ERROR_INVALID_OPCODE);
            break;
         }
      } // end of switch
   }
}



/*************************************************************************
*  FUNCTION:    clTuner_CCAhandler::vHandleFID_G_SET_HICUT_From_FCTuner(  )
*
* DESCRIPTION: handler to extract data from the status message from server for 
*               FID= FID_TUN_G_SET_HICUT
*
* PARAMETER:   amt_tclServiceData* poMessage (data message to process)
*
* RETURNVALUE: NONE
*
* History:
* InitialVersion
* 
*************************************************************************/
tVoid clTuner_CCAhandler::vHandleFID_G_SET_HICUT_From_FCTuner( 
   amt_tclServiceData* poMessage )
{

   /*if ( m_poTrace != NULL )
   {
      m_poTrace->vTrace(  TR_LEVEL_HMI_INFO, 
         TR_CLASS_HSI_TUNER_MSG, 
         (tU16)TUN_FID_SET_HICUT_FROM_SERV );
   }*/

	ETG_TRACE_USR4(("vHandleFID_G_SET_HICUT_From_FCTuner:"));

   /* check the parameter for not NULL and then for validity of a messsage */
   if ( (poMessage != NULL) && (poMessage->bIsValid()) )
   {

      switch (poMessage->u8GetOpCode())  // get the opcode from the message
      {
      case AMT_C_U8_CCAMSG_OPCODE_STATUS:
         {
            vStartInitSequence(MIDW_TUNERFI_C_U16_FID_TUN_G_SET_HICUT);

            /* extract the data from AMT Class to FI Class */
            midw_tunerfi_tclMsgFID_TUN_G_SET_HICUTStatus oMsgData;			
            vGetDataFromAmt( poMessage , oMsgData );

            //extract Hicut value
            tU8 u8HicutValue = oMsgData.u8AbsVal;  

            if(poFI != NULL)
            {
               poFI->WriteDataToDp(MIDW_TUNERFI_C_U16_FID_TUN_G_SET_HICUT, u8HicutValue);
            }
            break;
         }
      case AMT_C_U8_CCAMSG_OPCODE_ERROR:
         {           
            /* handle error message in a separate function */
            vHandleCCAErrorMessage( poMessage );
            break;
         } 

      default : // unknown opcode
         {
            /* send error message back to server for unkown opcode */
            vSendErrorMessage(   poMessage->u16GetFunctionID(),
               AMT_C_U16_ERROR_INVALID_OPCODE);
            break;
         }
      } // end of switch
   }
}

/*************************************************************************
*  FUNCTION:    vHandleFID_G_GET_DATA_TESTMODE_DDADDS_From_FCTuner( amt_tclServiceData* poMessage )
*
* DESCRIPTION: handler to extract data from the status message from server for
*               FID= FID_TUN_G_DATA_TESTMODE_DDADDS
*
* PARAMETER:  amt_tclServiceData* poMessage (data message to process)
*
* RETURNVALUE: NONE
*
* History:
* InitialVersion
* 
*************************************************************************/
tVoid clTuner_CCAhandler:: vHandleFID_G_GET_DATA_TESTMODE_DDADDS_From_FCTuner ( 
   amt_tclServiceData* poMessage )
{

   /*if ( m_poTrace != NULL )
   {
      m_poTrace->vTrace(  TR_LEVEL_HMI_INFO,
         TR_CLASS_HSI_TUNER_MSG, 
         (tU16)TUN_FID_DATA_TESTMODE_FROM_SERV );
   }*/

	ETG_TRACE_USR4(("vHandleFID_G_GET_DATA_TESTMODE_DDADDS_From_FCTuner:"));


   /* check the parameter for not NULL and then for validity of a messsage */
   if ( (poMessage != NULL) && (poMessage->bIsValid()) )
   {

      switch (poMessage->u8GetOpCode())       // get the opcode from the message
      {

      case AMT_C_U8_CCAMSG_OPCODE_STATUS:
         {
            vStartInitSequence(MIDW_TUNERFI_C_U16_FID_TUN_G_GET_DATA_TESTMODE_DDADDS);
            /* separate handler for data extraction as data size is too big*/

            vHandleTestModeDataDDADDSStatus ( poMessage );        

            break;

         }
      case AMT_C_U8_CCAMSG_OPCODE_ERROR:
         {          
            /* handle error message in a separate function */
            vHandleCCAErrorMessage( poMessage );

            break;
         } 

      default : // unknown opcode
         {
            /* send error message back to server for unkown opcode */
            vSendErrorMessage(
               poMessage->u16GetFunctionID(),            
               AMT_C_U16_ERROR_INVALID_OPCODE
               );
            break;
         }

      } // end of switch
   }
}

/*************************************************************************
*  FUNCTION:    clTuner_CCAhandler::vHandleFID_G_AFLIST_V2_From_FCTuner(  )
*
* DESCRIPTION: handler to extract data from the status message from server for FID= FID_TUN_G_AFLIST_V2
*
* PARAMETER:   amt_tclServiceData* poMessage (data message to process)
*
* RETURNVALUE: NONE
*
* History:
* InitialVersion
* 
*************************************************************************/
tVoid clTuner_CCAhandler::vHandleFID_G_AFLIST_V2_From_FCTuner(
   amt_tclServiceData* poMessage)
{

   if ( m_poTrace != NULL )
   {
      m_poTrace->vTrace(  TR_LEVEL_HMI_INFO,
         TR_CLASS_HSI_TUNER_MSG,
         (tU16)TUN_FID_AFLIST_V2_FROM_SERV );
   }

   /* check the parameter for not NULL and then for validity of a messsage */
   if ( (poMessage != NULL) && (poMessage->bIsValid()) )
   {
      switch (poMessage->u8GetOpCode())    // get the opcode from the message
      {

      case AMT_C_U8_CCAMSG_OPCODE_STATUS:
         {
            vStartInitSequence(MIDW_TUNERFI_C_U16_FID_TUN_G_AFLIST_V2);

            /* extract the data from AMT Class to FI Class */
            midw_tunerfi_tclMsgFID_TUN_G_AFLIST_V2Status oMsgData;			
            vGetDataFromAmt( poMessage , oMsgData );

            // extract AFListLength
            tU8 u8AFListLength = oMsgData.u8LengthOfAFList;  

            /** Added by rrv2kor to Send the AF List Length Message to CM TesTMode */
            if ( pclHSI_GetTestMode() != NULL)
            {
               sMsg refMsg(NULL, (tU32)CMMSG_TESTMODE_AFLIST_LENGTH, (tU32)u8AFListLength, NULL);
               pclHSI_GetTestMode()->bExecuteMessage( refMsg );
            }

            if( poAFListManager != NULL) //configure the AF list manager
            {
               poAFListManager->bConfigure_Array(u8AFListLength , sizeof(rAFListElement) , FALSE);
            }

            break;
         }
      case AMT_C_U8_CCAMSG_OPCODE_ERROR:
         {         
            /* handle error message in a separate function */
            vHandleCCAErrorMessage( poMessage );

            break;
         } 

      default : // unknown opcode
         {
            /* send error message back to server for unkown opcode */
            vSendErrorMessage(
               poMessage->u16GetFunctionID(),            
               AMT_C_U16_ERROR_INVALID_OPCODE
               );
            break;
         }

      } // end of switch
   }
}

/*************************************************************************
*  FUNCTION: vHandleFID_G_AFLIST_V2_ELEMENT_From_FCTuner( )
*
* DESCRIPTION: handler to extract data from the status message from server for FID= FID_TUN_G_AFLIST_V2_ELEMENT
*
* PARAMETER:   amt_tclServiceData* poMessage (data message to process)
*
* RETURNVALUE: NONE
*
* History:
* InitialVersion
* 
*************************************************************************/
tVoid clTuner_CCAhandler::vHandleFID_G_AFLIST_V2_ELEMENT_From_FCTuner(
   amt_tclServiceData* poMessage)
{

   if ( m_poTrace != NULL )
   {
      m_poTrace->vTrace(  TR_LEVEL_HMI_INFO,
         TR_CLASS_HSI_TUNER_MSG,
         (tU16)TUN_FID_AFLIST_ELEMENT_FROM_SERV );
   }

   /* check the parameter for not NULL and then for validity of a messsage */
   if ( (poMessage != NULL) && (poMessage->bIsValid()) )
   {
      switch (poMessage->u8GetOpCode())  // get the opcode from the message
      {      
      case AMT_C_U8_CCAMSG_OPCODE_STATUS:
         {
            vStartInitSequence(MIDW_TUNERFI_C_U16_FID_TUN_G_AFLIST_V2_ELEMENT);			

            /* extract the data from AMT Class to FI Class */
            midw_tunerfi_tclMsgFID_TUN_G_AFLIST_V2_ELEMENTStatus oMsgData;			
            vGetDataFromAmt( poMessage , oMsgData );


            if( NULL != poAFListManager )   
            {  
               rAFListElement oAFListElement;

               //extract AFListID

               tU8 u8AFListID = oMsgData.TunerAFList.u8AFListElementID;

               //extract AFFrequency

               oAFListElement.u32Freq = oMsgData.TunerAFList.u32AFFrequency;

               //extract AFInfo
               oAFListElement.u8AFInfo = oMsgData.TunerAFList.u8AFInformation;

               /** Added by rrv2kor to send the Message (List Element)to Component manager TestMode 
               As Logic of getting the list to Display is changed */
               if ( NULL != pclHSI_GetTestMode())
               {
                  pclHSI_GetTestMode()->bSetAFListElement( oMsgData.TunerAFList.u8AFListElementID,
                     oMsgData.TunerAFList.u32AFFrequency,
                     oMsgData.TunerAFList.u8AFInformation );
               }

               /** Even though the above code change will get the AF List to HMI still the code below is kept
               without commenting so as to the old interface for Selecting the AF List Element */          

               poAFListManager->vSetElement(++u8AFListID ,(tU8*)&oAFListElement ,sizeof(rAFListElement));  

               if ( m_poTrace != NULL )
               {
                  m_poTrace->vTrace(  TR_LEVEL_HMI_INFO,
                     TR_CLASS_HSI_TUNER_MSG,
                     "AFListElem %d, freq %d, info %x",u8AFListID,oAFListElement.u32Freq,oAFListElement.u8AFInfo );
               }

            }                        

            break;
         }

      case AMT_C_U8_CCAMSG_OPCODE_ERROR:
         {       
            /* handle error message in a separate function */
            vHandleCCAErrorMessage( poMessage );

            break;
         } 

      default : // unknown opcode
         {
            /* send error message back to server for unkown opcode */
            vSendErrorMessage(
               poMessage->u16GetFunctionID(),            
               AMT_C_U16_ERROR_INVALID_OPCODE
               );
            break;
         }

      } // end of switch
   }
}




/*************************************************************************
*  FUNCTION: clTuner_CCAhandler::vHandleFID_G_MAKE_CONFIG_LIST_From_FCTuner( )
*
* DESCRIPTION: handler to extract data from the status message from server for 
*               FID= FID_G_MAKE_CONFIG_LIST  
*
* PARAMETER:   amt_tclServiceData* poMessage (data message to process)
*
* RETURNVALUE: NONE
*
* History:
* InitialVersion
* 
*************************************************************************/
tVoid clTuner_CCAhandler::vHandleFID_G_MAKE_CONFIG_LIST_From_FCTuner( amt_tclServiceData* poMessage )
{ 
   /* check the parameter for not NULL and then for validity of a messsage */
   if ( (poMessage != NULL) && (poMessage->bIsValid()) )
   {
      switch (poMessage->u8GetOpCode())   // get the opcode from the message
      {

      case AMT_C_U8_CCAMSG_OPCODE_STATUS:
         {
            vStartInitSequence(MIDW_TUNERFI_C_U16_FID_TUN_G_MAKE_CONFIG_LIST);		

            /* extract the data from AMT Class to FI Class */
            midw_tunerfi_tclMsgFID_TUN_G_MAKE_CONFIG_LISTStatus oMsgData;			
            vGetDataFromAmt( poMessage , oMsgData );

            tU8 u8ListId               = oMsgData.TunMakeConfigListElem.u8ListID;
            tU8 u8NumOfElementsInList  = oMsgData.TunMakeConfigListElem.u8NumOfElementsInList;

            vTrace(CCA_RADIO_MAKE_CONFIG_LIST, TR_LEVEL_HMI_INFO,u8ListId,u8NumOfElementsInList,0);

            switch   (oMsgData.TunMakeConfigListElem.u8ListID)
            {
            case  TUN_CONFIG_LIST_FM:
               {
                  // FM station-list
                  if (pHSI_GetRadio() != NULL)
                  {
                     pHSI_GetRadio()->vSetFMStationListCount(u8NumOfElementsInList);
                  }
                  if(TRUE == m_FMStationListRequested)
                  {
                     m_U8NoOfFMStations = u8NumOfElementsInList;
                  }
                  if(u8NumOfElementsInList == 0)
                  {
                	 m_FMStationListRequested = FALSE;
                	 tU8 u8StationListLoadingState = (tU8)TRUE;
                     if(NULL != poFI->m_poDataPool)
                     {
                       poFI->m_poDataPool->u32dp_set( DPTUNER__STATION_LIST_LOADING_STATE,
                          &u8StationListLoadingState ,
                          sizeof(u8StationListLoadingState) );
                     }
                  }
               }
               break;
#ifdef VARIANT_S_FTR_ENABLE_HMI_TUNER_ITG5
            case  TUN_CONFIG_LIST_AM:
               { //AM station-list
            	   if(TRUE == m_bAMStationListRequested)
            	   {
                       if (AM_STATIONLIST_ELEM_MAX   < u8NumOfElementsInList)
                       {
                	       u8NumOfElementsInList = AM_STATIONLIST_ELEM_MAX;
                       }
                       DataPool.vSetU8Value(DPTUNER__AM_STATIONLIST_COUNT,u8NumOfElementsInList);

                       m_u8NoOfAMStations = u8NumOfElementsInList;

                       if (u8NumOfElementsInList == 0)
                       {
                	       m_bAMStationListRequested = FALSE;
                           tU8 u8StationListLoadingState = (tU8)TRUE;

                    	   DataPool.vSetU8Value(DPTUNER__STATION_LIST_LOADING_STATE,u8StationListLoadingState);
                       }
            	   }
               }
               break;
#endif

            case  TUN_CONFIG_LIST_TMC:
               {
                  if( TRUE == m_TMCListRequested )
                  {
                     /* Store No.Of Elements in the data member variable */
                     m_U8NoOfTMCStations = u8NumOfElementsInList;
                     /** Added by rrv2kor to Send the TMC List Length Message to CM TesTMode */
                     if ( NULL != pclHSI_GetTestMode())
                     {
                        sMsg refMsg(NULL, (tU32)CMMSG_TESTMODE_TMCLIST_LENGTH, (tU32)u8NumOfElementsInList, NULL);
                        pclHSI_GetTestMode()->bExecuteMessage( refMsg );

                        if (m_poTrace != NULL)
                        {
                           m_poTrace->vTrace(TR_LEVEL_HMI_INFO, TR_CLASS_HSI_TUNER_MSG,"Sending Message TMC List Length : %d", u8NumOfElementsInList );
                        }
                     }
                  }
               }
               break;

            default:
               {
                  vTrace(CCA_RADIO_MAKE_CONFIG_LIST_ERROR, TR_LEVEL_HMI_INFO,u8ListId,u8NumOfElementsInList,0);
               }
               break;
            }
         }
         break;



case AMT_C_U8_CCAMSG_OPCODE_ERROR:
   {      
      /* handle error message in a separate function */
      vHandleCCAErrorMessage( poMessage );

      break;
   } 

default : // unknown opcode
   {
      /* send error message back to server for unkown opcode */
      vSendErrorMessage(
         poMessage->u16GetFunctionID(),            
         AMT_C_U16_ERROR_INVALID_OPCODE
         );

      break;
   }
   } // end of switch
}
}



/*************************************************************************
*  FUNCTION: clTuner_CCAhandler::vHandleFID_G_CONFIG_LIST_ELEM_From_FCTuner( )
*
* DESCRIPTION: handler to extract data from the status message from server for 
*               FID= FID_TUN_G_CONFIG_LIST_ELEMENT  
*
* PARAMETER:   amt_tclServiceData* poMessage (data message to process)
*
* RETURNVALUE: NONE
*
* History:
* InitialVersion
* 
*************************************************************************/
tVoid clTuner_CCAhandler::vHandleFID_G_CONFIG_LIST_ELEM_From_FCTuner( amt_tclServiceData* poMessage )
{ 
   /* check the parameter for not NULL and then for validity of a messsage */
   if ( (poMessage != NULL) && (poMessage->bIsValid()) )
   {
      switch (poMessage->u8GetOpCode())   // get the opcode from the message
      {
      case AMT_C_U8_CCAMSG_OPCODE_STATUS:
         {
            vStartInitSequence(MIDW_TUNERFI_C_U16_FID_TUN_G_CONFIG_LIST_ELEMENT);

            /* object for handling the status data , server defined */
            midw_tunerfi_tclMsgFID_TUN_G_CONFIG_LIST_ELEMENTStatus oStatusMessage;

            vGetDataFromAmt( poMessage , oStatusMessage );

#ifdef VARIANT_S_FTR_ENABLE_HMI_TUNER_ITG5
            tU8 u8ListID = (tU8)oStatusMessage.TunConfigListElements.e8ConfigListID.enType;
#else
            tU8 u8ListID = oStatusMessage.TunConfigListElements.u8ListID;
#endif
            
            tU8 u8ElementSelected = oStatusMessage.TunConfigListElements.u8ElementSelected;

            // Extract & store ElemID
            // tU8 u8PositionInPresetList = oStatusMessage.TunConfigListElements.u8PositionInPresetList;
            tU8 u8ElemID = oStatusMessage.TunConfigListElements.u8ElementID;

            switch   (u8ListID)
            {
            case  TUN_CONFIG_LIST_FM:
               {
                  if (pHSI_GetRadio()  != NULL)
                  {
                     if(u8ElementSelected == 0x01)
                     {
                	    pHSI_GetRadio()->vSetCurStationListIndex(u8ElemID);
                     }
#ifdef VARIANT_S_FTR_ENABLE_HMI_TUNER_ITG5
                	 tChar *pcPSName   =  oStatusMessage.TunConfigListElements.sPSNameOrHDCallSign.szGet(midw_fi_tclString::FI_EN_UTF8);

                     if (!pHSI_GetRadio()->bSetFMStationList_Elem(
                        /*ElemID    */    u8ElemID,
                        /*PSName    */    pcPSName,
                        /*Freq      */    oStatusMessage.TunConfigListElements.u32Frequency,
                        /*PackedInfo*/    oStatusMessage.TunConfigListElements.b8PackedInfo.u8Value,
                        /*AvFlags   */    oStatusMessage.TunConfigListElements.u8AvailFlags.u8Value,
                        /*PI        */    oStatusMessage.TunConfigListElements.u32PIorHDStationId,
                        /*PresetNr  */    oStatusMessage.TunConfigListElements.u8PositionInPresetList,
                        /*HDAudioProgNo */oStatusMessage.TunConfigListElements.b8HDNumOfAudioPrograms.u8Value)
                        )
                     {
                        vTrace(CCA_RADIO_CONFIG_LIST_ELEM_VAG_ERROR, TR_LEVEL_HMI_INFO,(tU32)u8ElemID);
                     }
#else
                     tChar *pcPSName   =  oStatusMessage.TunConfigListElements.sPSName.szGet(midw_fi_tclString::FI_EN_UTF8);
                     if (!pHSI_GetRadio()->bSetFMStationList_Elem(
                        /*ElemID    */    u8ElemID,
                        /*PSName    */    pcPSName,
                        /*Freq      */    oStatusMessage.TunConfigListElements.u32Frequency,
                        /*PackedInfo*/    oStatusMessage.TunConfigListElements.u8PackedInfo.u8Value,
                        /*AvFlags   */    oStatusMessage.TunConfigListElements.u8AvailFlags.u8Value,
                        /*PI        */    oStatusMessage.TunConfigListElements.u32PI,
                        /*PresetNr  */    oStatusMessage.TunConfigListElements.u8PositionInPresetList)
                        )
                     {
                        vTrace(CCA_RADIO_CONFIG_LIST_ELEM_VAG_ERROR, TR_LEVEL_HMI_INFO,(tU32)u8ElemID);
                     }
#endif
                     if (pcPSName != NULL)
                     {
                        OSAL_DELETE [] pcPSName;
                        pcPSName = NULL;
                     }
                  }
                  if((TRUE == m_FMStationListRequested) && (u8ElemID == (m_U8NoOfFMStations-1)))
                  {
                     m_FMStationListRequested = FALSE;
                     m_U8NoOfFMStations = 0;
                     /** Set The Datapool Element to TRUE to release the wait Sync of Station List */
                     tU8 u8StationListLoadingState = (tU8)TRUE;
                     if(NULL != poFI->m_poDataPool)
                     {
                        poFI->m_poDataPool->u32dp_set( DPTUNER__STATION_LIST_LOADING_STATE,
                           &u8StationListLoadingState , 
                           sizeof(u8StationListLoadingState) );
                     }

                  }
               }
               break;
#ifdef VARIANT_S_FTR_ENABLE_HMI_TUNER_ITG5
            case TUN_CONFIG_LIST_AM:
               {
                   if ((pHSI_GetRadio()  != NULL) && (u8ElementSelected == ELEM_SELECTED_IN_LIST))
                   {
                       pHSI_GetRadio()->vSetCurStationListIndex(u8ElemID);
                   }
                    tU8 u8HDAudPrgNumBitValue = oStatusMessage.TunConfigListElements.b8HDNumOfAudioPrograms.u8Value;
                    tU8 u8HDAudPrgNum = 0;
                   /*Evaluate the HD Audio Program ID */
                   if(u8HDAudPrgNumBitValue != 0)
                   {
                       tU8 u8Bitmask = 0x01;
                       for(tU8 u8PrgmID = HD_MPS_AUD_PRGM_NUM; u8PrgmID <= HD_SPS_LAST_AUD_PRGM_NUM; u8PrgmID++)
                       {
                           if((u8HDAudPrgNumBitValue & u8Bitmask) == u8Bitmask)
                       	   {
                        	   u8HDAudPrgNum = u8PrgmID;
                       		   break;
                       	   }
                           u8HDAudPrgNumBitValue = u8HDAudPrgNum >>1;
                       }
                    }

                   if(poFI != NULL)
                   {
                       //Write the Station name
                   	   poFI->vTun_WriteDataToDataPool(
                   			DPTUNER__AM_STATIONLIST,
                   			TUNER__AM_STATIONLIST_STATION_NAME,
                   			u8ElemID,
                            &oStatusMessage.TunConfigListElements.sPSNameOrHDCallSign);

                   	   //Write the frequency
                   	   poFI->vTun_WriteDataToDataPool(
                   			DPTUNER__AM_STATIONLIST,
                   			TUNER__AM_STATIONLIST_FREQUENCY,
                   			u8ElemID,
                   	       (tVoid*)&oStatusMessage.TunConfigListElements.u32Frequency,
                   	       sizeof(oStatusMessage.TunConfigListElements.u32Frequency));

                   	   //Write the HD audio program number (will update 0 if not a HD station)
                   	   poFI->vTun_WriteDataToDataPool(
                   	        DPTUNER__AM_STATIONLIST,
                   	        TUNER__AM_STATIONLIST_HD_PRGM_NO,
                   	        u8ElemID,
                   	        (tVoid*)&u8HDAudPrgNum,
                   	        sizeof(u8HDAudPrgNum));

                       //Write the matching Preset ID (If no matching Preset is available, value will be 0)
                       poFI->vTun_WriteDataToDataPool(
                		   DPTUNER__AM_STATIONLIST,
                		   TUNER__AM_STATIONLIST_PRESET_NO,
                		   u8ElemID,
                   		  (tVoid*)&oStatusMessage.TunConfigListElements.u8PositionInPresetList,
                   		  sizeof(oStatusMessage.TunConfigListElements.u8PositionInPresetList));
                   }

                   if((TRUE == m_bAMStationListRequested) && (u8ElemID == (m_u8NoOfAMStations-1)))
                   {
                	   tU8 u8ClusterListNotify = DataPool.u8GetValue(DPTUNER__CLUSTER_NEW_LIST);
                       u8ClusterListNotify = u8ClusterListNotify != 0 ? 0 : 1;
                       DataPool.vSetU8Value(DPTUNER__CLUSTER_NEW_LIST,u8ClusterListNotify);

                	   m_bAMStationListRequested = FALSE;
                	   m_u8NoOfAMStations = 0;
                       /** Set The Datapool Element to TRUE to release the wait Sync of Station List */
                       tU8 u8StationListLoadingState = (tU8)TRUE;
                       DataPool.vSetU8Value(DPTUNER__STATION_LIST_LOADING_STATE,u8StationListLoadingState);
                   }
               }
               break;
#endif

            case  TUN_CONFIG_LIST_TMC:
               {
                  if(TRUE == m_TMCListRequested)
                  {
                     /** Added by rrv2kor to Send the TMC List Element Message to CM TesTMode */
                     if ( NULL != pclHSI_GetTestMode())
                     {
                        if ( m_poTrace != NULL)
                        {
                           m_poTrace->vTrace(TR_LEVEL_HMI_INFO, TR_CLASS_HSI_TUNER_MSG,"Got the update from Server" );
                           m_poTrace->vTrace(TR_LEVEL_HMI_INFO, TR_CLASS_HSI_TUNER_MSG,"List Element Update from :vHandleFID_G_CONFIG_LIST_ELEM_From_FCTuner " );
                        }

                        /** Extract the TMC Station data into Structure */
                        sTMCStation_info sTMCStationInfo;

                        sTMCStationInfo.u32Frequency		   =	oStatusMessage.TunConfigListElements.u32Frequency;

                        sTMCStationInfo.u32PI			   =	oStatusMessage.TunConfigListElements.u32PI;

                        sTMCStationInfo.u8ECC			   =    oStatusMessage.TunConfigListElements.u8ECC;


                        sTMCStationInfo.u8Quality		   =    oStatusMessage.TunConfigListElements.u8Quality;

                        sTMCStationInfo.u8SID              =    oStatusMessage.TunConfigListElements.u8SID;

                        sTMCStationInfo.u8LTN              =    oStatusMessage.TunConfigListElements.u8LTN;

                        if ( m_poTrace != NULL )
                        {

                           m_poTrace->vTrace(TR_LEVEL_HMI_INFO, TR_CLASS_HSI_TUNER_MSG,"PI Value from Server             : %x", sTMCStationInfo.u32PI );					   
                           m_poTrace->vTrace(TR_LEVEL_HMI_INFO, TR_CLASS_HSI_TUNER_MSG,"u32Frequency  data from Server   : %d", sTMCStationInfo.u32Frequency );
                           m_poTrace->vTrace(TR_LEVEL_HMI_INFO, TR_CLASS_HSI_TUNER_MSG,"Quality  data from Server        : %d", sTMCStationInfo.u8Quality );
                           m_poTrace->vTrace(TR_LEVEL_HMI_INFO, TR_CLASS_HSI_TUNER_MSG,"Country Code  data from Server   : %d", sTMCStationInfo.u8ECC );
                           m_poTrace->vTrace(TR_LEVEL_HMI_INFO, TR_CLASS_HSI_TUNER_MSG,"SID data from Server             : %d", sTMCStationInfo.u8SID );
                           m_poTrace->vTrace(TR_LEVEL_HMI_INFO, TR_CLASS_HSI_TUNER_MSG,"LTN data from Server             : %d", sTMCStationInfo.u8LTN );
                        }

                        if( NULL != pclHSI_GetTestMode() )
                        {
                           tChar *pcPSName   =  oStatusMessage.TunConfigListElements.sPSName.szGet(midw_fi_tclString::FI_EN_UTF8);
                           if (pcPSName != NULL)
                           {
                              pclHSI_GetTestMode()->bSetTMCListElement( 
                                 /*ElemID      */  u8ElemID,
                                 /*PSName      */  (tUTF8*)pcPSName,
                                 /*TMC Station */  sTMCStationInfo	);
                              OSAL_DELETE [] pcPSName;
                              pcPSName = NULL;
                           }
                        }						   
                        if ( m_poTrace  != NULL )
                        {
                           m_poTrace->vTrace(TR_LEVEL_HMI_INFO, TR_CLASS_HSI_TUNER_MSG,"TMC Element Send to CM TestMode" );
                        }
                        if( u8ElemID == (m_U8NoOfTMCStations-1) )
                        {
                           m_TMCListRequested  = FALSE;						   							  
                        }
                     }
                  }
               }
               break;

            default:
               {
                  vTrace(CCA_RADIO_CONFIG_LIST_ELEM_VAG_ERROR2, TR_LEVEL_HMI_INFO,(tU32)u8ListID);
               }
               break;
            }  // end switch

            tU8 uzBuffer[2] = { u8ListID,u8ElemID};
            if (m_poTrace != NULL)
            {
               m_poTrace->vTrace(   TR_LEVEL_HMI_INFO, 
                  TR_CLASS_HSI_TUNER_MSG, 
                  (tU16)CONFIG_LIST_ELEM_FROM_FCTUNER,
                  TWO_ELEMENT,
                  uzBuffer 
                  );
            }
         }
         break;

      case AMT_C_U8_CCAMSG_OPCODE_ERROR:
         {      
            /* handle error message in a separate function */
            vHandleCCAErrorMessage( poMessage );
         } 
         break;

      default : // unknown opcode
         {
            /* send error message back to server for unkown opcode */
            vSendErrorMessage(
               poMessage->u16GetFunctionID(),            
               AMT_C_U16_ERROR_INVALID_OPCODE
               );

         }
         break;
      } // end of switch
   }
}



/*************************************************************************
*  FUNCTION: clTuner_CCAhandler::vHandleFID_G_CONFIG_LIST_ELEM_From_FCTuner( )
*
* DESCRIPTION: handler to extract data from the status message from server for 
*               FID= FID_TUN_G_OPERATION  
*
* PARAMETER:   amt_tclServiceData* poMessage (data message to process)
*
* RETURNVALUE: NONE
*
* History:
* InitialVersion
* 
*************************************************************************/
tVoid clTuner_CCAhandler::vHandleFID_G_MIXEDPRESET_LIST_ELEMENT_From_FCTuner( amt_tclServiceData* poMessage )
{ 
   /* check the parameter for not NULL and then for validity of a messsage */
   vTrace(CCA_RADIO_MIXEDPRESET_LIST_ELEMENT_RECEIVED, TR_LEVEL_HMI_INFO);
   if ( (poMessage != NULL) && (poMessage->bIsValid()) )
   {
      switch (poMessage->u8GetOpCode())   // get the opcode from the message
      {
      case AMT_C_U8_CCAMSG_OPCODE_STATUS:
         {
            vStartInitSequence(MIDW_TUNERFI_C_U16_FID_TUN_G_MIXEDPRESET_LIST_ELEMENT);

            /* object for handling the status data , server defined */
            midw_tunerfi_tclMsgFID_TUN_G_MIXEDPRESET_LIST_ELEMENTStatus oStatusMessage;

            vGetDataFromAmt( poMessage , oStatusMessage );

            tU8                                    u8ElementID;
            u8ElementID = oStatusMessage.TunerStationData.u8ElementID;

            clHSI_Radio *pHSI =  pHSI_GetRadio();
            if (pHSI != NULL)
            {
               // is it an Up-Reg Telegram ?
               if (pHSI->m_blUpReg_G_MIXEDPRESET_LIST_ELEMENT   == FALSE)
               {
                  pHSI->m_blUpReg_G_MIXEDPRESET_LIST_ELEMENT   =  TRUE;
                  // now that we are registered we can request all the presets
                  /// a better design might be to just notify clHSI_Radio about the fact and move
                  /// loading the preset there.  this little fix does not warrent such an change yet
                  vTrace(TR_LEVEL_HMI_INFO, "LOAD PRESET LIST: G_MIXEDPRESET_LIST_ELEMENT");
                  sMsg refMsg(NULL, (tU32)CMMSG_RADIO_LOAD_LIST, clHSI_Radio::LT_PRESET, 0);
                  pHSI->bExecuteMessage(refMsg);
               }

               // only if it is valid
               if (midw_fi_tcl_e8_Tun_ListElementInfo::FI_EN_LIST_ELEMENT_INFO_VALID == oStatusMessage.TunerStationData.u8ListElementInfo.enType)
               {  


                  clHSI_Radio__MixedPresetList_Elem      stMixedPresetElement;


                  stMixedPresetElement.bUsed          = TRUE;
                  stMixedPresetElement.u16PackedInfo  = oStatusMessage.TunerStationData.u16PackedInfo;
                  stMixedPresetElement.u32Frequency   = oStatusMessage.TunerStationData.u32Frequency;
                  stMixedPresetElement.eBand	= (clHSI_Radio__MixedPresetList_Elem::BAND)oStatusMessage.TunerStationData.u8Band;
                  stMixedPresetElement.u32PI          = oStatusMessage.TunerStationData.u32PI;
                  stMixedPresetElement.u32AvailFlags  = oStatusMessage.TunerStationData.u32AvailFlags;


                  if (stMixedPresetElement.u32AvailFlags.bits.TUN_AVAILABLE_FLAGS_PS_AVAILABLE)
                  {
                     OSALUTIL_szSaveStringNCopy(   stMixedPresetElement.pcPSName,
                        oStatusMessage.TunerStationData.sPSName.szValue,
                        PSNAME_MAX_LEN_IN_UTF); // PS-Name
                     stMixedPresetElement.pcPSName[PSNAME_MAX_LEN_IN_UTF] = 0;   // for security
                  }
                  else  if (stMixedPresetElement.u32AvailFlags.bits.TUN_AVAILABLE_FLAGS_FRQ_AVAILABLE)
                  {
                     pHSI->bFormatFrequenz(stMixedPresetElement.pcPSName,PSNAME_MAX_LEN_IN_UTF,stMixedPresetElement.u32Frequency);
                  }
                  else
                  {
                     OSALUTIL_szSaveStringNCopy(stMixedPresetElement.pcPSName,"",PSNAME_MAX_LEN_IN_UTF);
                  }
                  stMixedPresetElement.au8PSName[0]   = oStatusMessage.TunerStationData.u8PSName1;
                  stMixedPresetElement.au8PSName[1]   = oStatusMessage.TunerStationData.u8PSName2;
                  stMixedPresetElement.au8PSName[2]   = oStatusMessage.TunerStationData.u8PSName3;
                  stMixedPresetElement.au8PSName[3]   = oStatusMessage.TunerStationData.u8PSName4;
                  stMixedPresetElement.au8PSName[4]   = oStatusMessage.TunerStationData.u8PSName5;
                  stMixedPresetElement.au8PSName[5]   = oStatusMessage.TunerStationData.u8PSName6;
                  stMixedPresetElement.au8PSName[6]   = oStatusMessage.TunerStationData.u8PSName7;
                  stMixedPresetElement.au8PSName[7]   = oStatusMessage.TunerStationData.u8PSName8;

                  pHSI->bSetMixedPresetList_Elem(u8ElementID,&stMixedPresetElement);

                  tU32   au32Data[] =
                  {
                     (tU32)u8ElementID,
                     stMixedPresetElement.u32Frequency,
                     stMixedPresetElement.u32PI
                  };
                  vTrace(CCA_RADIO_MIXEDPRESET_LIST_ELEMENT_RECEIVED_VALID_PS,TR_LEVEL_HMI_INFO,(tU8)strlen(stMixedPresetElement.pcPSName)+1,(tU8*)stMixedPresetElement.pcPSName);
                  vTrace(CCA_RADIO_MIXEDPRESET_LIST_ELEMENT_RECEIVED_VALID,TR_LEVEL_HMI_INFO,sizeof(au32Data),(tU8*)au32Data);
               }
               else
               {
                  //emptying the current entry
                  pHSI->vClearPresetListEntry(u8ElementID);
                  vTrace(CCA_RADIO_MIXEDPRESET_LIST_ELEMENT_RECEIVED_INVALID,TR_LEVEL_HMI_INFO,(tU32)u8ElementID);
               }
            }
            tU8 uzBuffer[1] = { u8ElementID};
            if (m_poTrace != NULL)
            {
               m_poTrace->vTrace(   TR_LEVEL_HMI_INFO, 
                  TR_CLASS_HSI_TUNER_MSG, 
                  (tU16)TUN_FID_G_MIXEDPRESET_LIST_ELEMENT_SERV,
                  ONE_ELEMENT,
                  uzBuffer 
                  );
            }
         }
         break;

      case AMT_C_U8_CCAMSG_OPCODE_ERROR:
         {      
            /* handle error message in a separate function */
            vHandleCCAErrorMessage( poMessage );

         } 
         break;

      default : // unknown opcode
         {
            /* send error message back to server for unkown opcode */
            vSendErrorMessage(
               poMessage->u16GetFunctionID(),            
               AMT_C_U16_ERROR_INVALID_OPCODE
               );

         }
         break;
      } // end of switch
   }
}




/*************************************************************************
*  FUNCTION: clTuner_CCAhandler::vHandleFID_G_SET_SHARXLEVEL_From_FCTuner( )
*
* DESCRIPTION: handler to extract data from the status message from server for 
*               FID= FID_TUN_G_SET_SHARXLEVEL  
*
* PARAMETER:   amt_tclServiceData* poMessage (data message to process)
*
* RETURNVALUE: NONE
*
* History:
* InitialVersion
* 
*************************************************************************/
tVoid clTuner_CCAhandler::vHandleFID_G_SET_SHARXLEVEL_From_FCTuner( amt_tclServiceData* poMessage )
{ 

	ETG_TRACE_USR4(("vHandleFID_G_SET_SHARXLEVEL_From_FCTuner()"));

	/* check the parameter for not NULL and then for validity of a messsage */
   if ( (poMessage != NULL) && (poMessage->bIsValid()) )
   {

      switch (poMessage->u8GetOpCode())   // get the opcode from the message
      {

      case AMT_C_U8_CCAMSG_OPCODE_STATUS:
         {
            vStartInitSequence(MIDW_TUNERFI_C_U16_FID_TUN_G_SET_SHARXLEVEL);		

            /* extract the data from AMT Class to FI Class */
            midw_tunerfi_tclMsgFID_TUN_G_SET_SHARXLEVELStatus oMsgData;			
            vGetDataFromAmt( poMessage , oMsgData );

            // Sharx level from server
            tU8 u8SharxLevel = oMsgData.u8SharxStep;

            if(poFI != NULL)
            {
               poFI->WriteDataToDp(MIDW_TUNERFI_C_U16_FID_TUN_G_SET_SHARXLEVEL , u8SharxLevel);
            }

            break;
         }

      case AMT_C_U8_CCAMSG_OPCODE_ERROR:
         {      
            /* handle error message in a separate function */
            vHandleCCAErrorMessage( poMessage );

            break;
         } 

      default : // unknown opcode
         {
            /* send error message back to server for unkown opcode */
            vSendErrorMessage(
               poMessage->u16GetFunctionID(),            
               AMT_C_U16_ERROR_INVALID_OPCODE
               );

            break;
         }

      } // end of switch
   }
}


/*************************************************************************
*  FUNCTION: clTuner_CCAhandler::vHandleFID_G_SET_DDA_From_FCTuner( )
*
* DESCRIPTION: handler to extract data from the status message from server for 
*               FID= FID_TUN_G_SET_DDA
*
* PARAMETER:   amt_tclServiceData* poMessage (data message to process)
*
* RETURNVALUE: NONE
*
* History:
* InitialVersion
* 
*************************************************************************/
tVoid clTuner_CCAhandler::vHandleFID_G_SET_DDA_From_FCTuner( amt_tclServiceData* poMessage )
{ 

	ETG_TRACE_USR4(("vHandleFID_G_SET_DDA_From_FCTuner()"));
	/* check the parameter for not NULL and then for validity of a messsage */
   if ( (poMessage != NULL) && (poMessage->bIsValid()) )
   {

      switch (poMessage->u8GetOpCode())   // get the opcode from the message
      {

      case AMT_C_U8_CCAMSG_OPCODE_STATUS:
         {
            vStartInitSequence(MIDW_TUNERFI_C_U16_FID_TUN_G_SET_DDA);

            /* extract the data from AMT Class to FI Class */
            midw_tunerfi_tclMsgFID_TUN_G_SET_DDAStatus oMsgData;
            vGetDataFromAmt( poMessage , oMsgData );

            // DDA level from server
            tU8 u8DDA = oMsgData.u8DDA;

            if(poFI != NULL)
            {
               poFI->WriteDataToDp(MIDW_TUNERFI_C_U16_FID_TUN_G_SET_DDA , u8DDA);
            }

            break;
         }

      case AMT_C_U8_CCAMSG_OPCODE_ERROR:
         {      
            /* handle error message in a separate function */
            vHandleCCAErrorMessage( poMessage );

            break;
         } 

      default : // unknown opcode
         {
            /* send error message back to server for unkown opcode */
            vSendErrorMessage(
               poMessage->u16GetFunctionID(),            
               AMT_C_U16_ERROR_INVALID_OPCODE
               );

            break;
         }

      } // end of switch
   }
}


/*************************************************************************
*  FUNCTION: clTuner_CCAhandler::vHandleFID_G_SET_AF_From_FCTuner( )
*
* DESCRIPTION: handler to extract data from the status message from server for
*               FID= FID_TUN_G_SET_AF
*
* PARAMETER:   amt_tclServiceData* poMessage (data message to process)
*
* RETURNVALUE: NONE
*
* History:
* InitialVersion
* 
*************************************************************************/
tVoid clTuner_CCAhandler::vHandleFID_G_SET_AF_From_FCTuner( amt_tclServiceData* poMessage )
{

	ETG_TRACE_USR4(("vHandleFID_G_SET_AF_From_FCTuner()"));
	/* check the parameter for not NULL and then for validity of a messsage */
   if ( (poMessage != NULL) && (poMessage->bIsValid()) )
   {

      switch (poMessage->u8GetOpCode())   // get the opcode from the message
      {

      case AMT_C_U8_CCAMSG_OPCODE_STATUS:
         {
            vStartInitSequence(MIDW_TUNERFI_C_U16_FID_TUN_G_SET_AF);

            /* extract the data from AMT Class to FI Class */
            midw_tunerfi_tclMsgFID_TUN_G_SET_AFStatus oMsgData;
            vGetDataFromAmt( poMessage , oMsgData );

            // AF level from server
            tU8 u8AF = oMsgData.u8AF;

            if(poFI != NULL)
            {
               poFI->WriteDataToDp(MIDW_TUNERFI_C_U16_FID_TUN_G_SET_AF , u8AF);
			   ETG_TRACE_USR4(("AF value written to datapool = %d", u8AF));
            }

            break;
         }

      case AMT_C_U8_CCAMSG_OPCODE_ERROR:
         {
            /* handle error message in a separate function */
            vHandleCCAErrorMessage( poMessage );

            break;
         }

      default : // unknown opcode
         {
            /* send error message back to server for unkown opcode */
            vSendErrorMessage(
               poMessage->u16GetFunctionID(),
               AMT_C_U16_ERROR_INVALID_OPCODE
               );

            break;
         }

      } // end of switch
   }
}



/*************************************************************************
*  FUNCTION: clTuner_CCAhandler::vHandleFID_G_SET_MEASUREMODE_From_FCTuner( )
*
* DESCRIPTION: handler to extract data from the status message from server for
*               FID= FID_TUN_G_SET_MEASUREMODE
*
* PARAMETER:   amt_tclServiceData* poMessage (data message to process)
*
* RETURNVALUE: NONE
*
* History:
* InitialVersion
*
*************************************************************************/
tVoid clTuner_CCAhandler::vHandleFID_G_SET_MEASUREMODE_From_FCTuner( amt_tclServiceData* poMessage )
{

	ETG_TRACE_USR4(("vHandleFID_G_SET_MEASUREMODE_From_FCTuner()"));
	/* check the parameter for not NULL and then for validity of a messsage */
   if ( (poMessage != NULL) && (poMessage->bIsValid()) )
   {

      switch (poMessage->u8GetOpCode())   // get the opcode from the message
      {

      case AMT_C_U8_CCAMSG_OPCODE_STATUS:
         {
            vStartInitSequence(MIDW_TUNERFI_C_U16_FID_TUN_G_SET_MEASUREMODE);

            /* extract the data from AMT Class to FI Class */
            midw_tunerfi_tclMsgFID_TUN_G_SET_MEASUREMODEStatus oMsgData;
            vGetDataFromAmt( poMessage , oMsgData );

            // AF level from server
            tU8 u8Measuremode = oMsgData.u8Measuremode;

            if(poFI != NULL)
            {
               poFI->WriteDataToDp(MIDW_TUNERFI_C_U16_FID_TUN_G_SET_MEASUREMODE , u8Measuremode);
            }

            break;
         }

      case AMT_C_U8_CCAMSG_OPCODE_ERROR:
         {
            /* handle error message in a separate function */
            vHandleCCAErrorMessage( poMessage );

            break;
         }

      default : // unknown opcode
         {
            /* send error message back to server for unkown opcode */
            vSendErrorMessage(
               poMessage->u16GetFunctionID(),
               AMT_C_U16_ERROR_INVALID_OPCODE
               );

            break;
         }

      } // end of switch
   }
}



/*************************************************************************
*  FUNCTION: clTuner_CCAhandler::vHandleFID_G_SET_RDSreg_From_FCTuner( )
*
* DESCRIPTION: handler to extract data from the status message from server for
*               FID= FID_TUN_G_SET_RDSreg
*
* PARAMETER:   amt_tclServiceData* poMessage (data message to process)
*
* RETURNVALUE: NONE
*
* History:
* InitialVersion
*
*************************************************************************/
tVoid clTuner_CCAhandler::vHandleFID_G_SET_RDSreg_From_FCTuner( amt_tclServiceData* poMessage )
{

	ETG_TRACE_USR4(("vHandleFID_G_SET_RDSreg_From_FCTuner()"));
	/* check the parameter for not NULL and then for validity of a messsage */
   if ( (poMessage != NULL) && (poMessage->bIsValid()) )
   {

      switch (poMessage->u8GetOpCode())   // get the opcode from the message
      {

      case AMT_C_U8_CCAMSG_OPCODE_STATUS:
         {
            vStartInitSequence(MIDW_TUNERFI_C_U16_FID_TUN_G_SET_RDSREG);

            /* extract the data from AMT Class to FI Class */
            midw_tunerfi_tclMsgFID_TUN_G_SET_RDSREGStatus oMsgData;
            vGetDataFromAmt( poMessage , oMsgData );

            // AF level from server
            tU8 u8RDSreg = oMsgData.u8RDSreg;

            if(poFI != NULL)
            {
               poFI->WriteDataToDp(MIDW_TUNERFI_C_U16_FID_TUN_G_SET_RDSREG , u8RDSreg);
			   ETG_TRACE_USR4(("RDS REG value written to dp = %d", u8RDSreg ));
            }

            break;
         }

      case AMT_C_U8_CCAMSG_OPCODE_ERROR:
         {
            /* handle error message in a separate function */
            vHandleCCAErrorMessage( poMessage );

            break;
         }

      default : // unknown opcode
         {
            /* send error message back to server for unkown opcode */
            vSendErrorMessage(
               poMessage->u16GetFunctionID(),
               AMT_C_U16_ERROR_INVALID_OPCODE
               );

            break;
         }

      } // end of switch
   }
}



/*************************************************************************
*  FUNCTION: clTuner_CCAhandler::vHandleFID_G_GET_CALIBRATION_To_FCTuner( )
*
* DESCRIPTION: handler to extract data from the status message from server for
*               FID= FID_TUN_G_CALIBRATION_DATA
*
* PARAMETER:   amt_tclServiceData* poMessage (data message to process)
*
* RETURNVALUE: NONE
*
* History:
* InitialVersion
*
*************************************************************************/
tVoid clTuner_CCAhandler::vHandleFID_G_GET_CALIBRATION_To_FCTuner( amt_tclServiceData* poMessage )
{

	ETG_TRACE_USR4(("vHandleFID_G_GET_CALIBRATION_To_FCTuner()"));
	/* check the parameter for not NULL and then for validity of a messsage */
   if ( (poMessage != NULL) && (poMessage->bIsValid()) )
   {

      switch (poMessage->u8GetOpCode())   // get the opcode from the message
      {

      case AMT_C_U8_CCAMSG_OPCODE_STATUS:
         {

            tU16 u16Calibrationdata;
        	vStartInitSequence(MIDW_TUNERFI_C_U16_FID_TUN_G_CALIBRATION_DATA);

            /* extract the data from AMT Class to FI Class */
            midw_tunerfi_tclMsgFID_TUN_G_CALIBRATION_DATAStatus oMsgData;
            vGetDataFromAmt( poMessage , oMsgData );

			if(NULL != poFI->m_poDataPool)
            {
				u16Calibrationdata =  oMsgData.CalibrationData.CalibrationValues[0];



			    ETG_TRACE_USR4(("Calibration values : FM_Tuner1:%d",u16Calibrationdata));

				poFI->m_poDataPool->u32dp_set( DPTUNER__FEATURE_CALIBRATIONDATA_FM_TUNER_0,
                      &u16Calibrationdata,
                      sizeof(u16Calibrationdata));

				u16Calibrationdata =  oMsgData.CalibrationData.CalibrationValues[1];



			    ETG_TRACE_USR4(("Calibration values : FM_Tuner2:%d",u16Calibrationdata));

				poFI->m_poDataPool->u32dp_set( DPTUNER__FEATURE_CALIBRATIONDATA_FM_TUNER_1,
                      &u16Calibrationdata,
                      sizeof(u16Calibrationdata));

				u16Calibrationdata =  oMsgData.CalibrationData.CalibrationValues[2];



			    ETG_TRACE_USR4(("Calibration values : FM_Tuner3:%d",u16Calibrationdata));

				poFI->m_poDataPool->u32dp_set( DPTUNER__FEATURE_CALIBRATIONDATA_FM_TUNER_2,
                      &u16Calibrationdata,
                      sizeof(u16Calibrationdata));

				u16Calibrationdata =  oMsgData.CalibrationData.CalibrationValues[3];



			    ETG_TRACE_USR4(("Calibration values : FM_Tuner4:%d",u16Calibrationdata));

				poFI->m_poDataPool->u32dp_set( DPTUNER__FEATURE_CALIBRATIONDATA_FM_TUNER_3,
                      &u16Calibrationdata,
                      sizeof(u16Calibrationdata));
				
				u16Calibrationdata =  oMsgData.CalibrationData.CalibrationValues[4];
				


			    ETG_TRACE_USR4(("Calibration values : AM_Tuner:%d",u16Calibrationdata));

				poFI->m_poDataPool->u32dp_set( DPTUNER__FEATURE_CALIBRATIONDATA_AM,
                      &u16Calibrationdata,
                      sizeof(u16Calibrationdata));
            }


            break;
         }

      case AMT_C_U8_CCAMSG_OPCODE_ERROR:
         {
            /* handle error message in a separate function */
            vHandleCCAErrorMessage( poMessage );

            break; 
         }

      default : // unknown opcode
         {
            /* send error message back to server for unkown opcode */
            vSendErrorMessage(
               poMessage->u16GetFunctionID(),
               AMT_C_U16_ERROR_INVALID_OPCODE
               );

            break;
         }

      } // end of switch
   }
}



/*************************************************************************
*  FUNCTION: clTuner_CCAhandler::vHandleFID_G_AtSeek_From_FCTuner( )
*
* DESCRIPTION: handler to extract data from the status message from server for
*               FID= FID_TUN_G_AtSeek
*
* PARAMETER:   amt_tclServiceData* poMessage (data message to process)
*
* RETURNVALUE: NONE

*************************************************************************/
tVoid clTuner_CCAhandler::vHandleFID_G_AtSeek_From_FCTuner( amt_tclServiceData* poMessage )
{

	ETG_TRACE_USR4(("vHandleFID_G_ATSEEK_From_FCTuner()"));
	/* check the parameter for not NULL and then for validity of a messsage */
   if ( (poMessage != NULL) && (poMessage->bIsValid()) )
   {

      switch (poMessage->u8GetOpCode())   // get the opcode from the message
      {

      case AMT_C_U8_CCAMSG_OPCODE_STATUS:
         {
        	vStartInitSequence(MIDW_TUNERFI_C_U16_FID_TUN_G_ATSEEK);
			
            /* extract the data from AMT Class to FI Class */
            midw_tunerfi_tclMsgFID_TUN_G_ATSEEKStatus oMsgData;
			vGetDataFromAmt( poMessage , oMsgData );
			 bSeekState =  (tBool)oMsgData.e8SeekState.enType;
			 bDirectFreq = false;
        	 m_u32Frequency = 0;
			  
			  ETG_TRACE_USR4(("vHandleFID_G_ATSEEK_From_FCTuner(), seek state = %d", bSeekState ));
			  pHSI_GetRadio()->vSetSeekMode(bSeekState);
            break;
         }

      case AMT_C_U8_CCAMSG_OPCODE_ERROR:
         {
            /* handle error message in a separate function */
            vHandleCCAErrorMessage( poMessage );

            break; 
         }

      default : // unknown opcode
         {
            /* send error message back to server for unkown opcode */
            vSendErrorMessage(
               poMessage->u16GetFunctionID(),
               AMT_C_U16_ERROR_INVALID_OPCODE
               );

            break;
         }

      } // end of switch
   }
}

#ifdef VARIANT_S_FTR_ENABLE_HMI_TUNER_ITG5
/*************************************************************************
* FUNCTION: clTuner_CCAhandler::vHandleFID_G_DemodulationSwitch_From_FCTuner( )
*
* DESCRIPTION: handler to extract data from the status message from server for
*               FID= FID_TUN_G_DEMODULATION_SWITCH_STATUS
*
* PARAMETER:   amt_tclServiceData* poMessage (data message to process)
*
* RETURNVALUE: NONE

*************************************************************************/
tVoid clTuner_CCAhandler::vHandleFID_G_DemodulationSwitch_From_FCTuner( amt_tclServiceData* poMessage )
{
	ETG_TRACE_USR4(("vHandleFID_G_DemodulationSwitch_From_FCTuner()"));
	/* check the parameter for not NULL and then for validity of a messsage */
   if ( (poMessage != NULL) && (poMessage->bIsValid()) )
   {
      switch (poMessage->u8GetOpCode())   // get the opcode from the message
      {

      case AMT_C_U8_CCAMSG_OPCODE_STATUS:
         {
        	vStartInitSequence(MIDW_TUNERFI_C_U16_FID_TUN_G_DEMODULATION_SWITCH_STATUS);

            /* extract the data from AMT Class to FI Class */
            midw_tunerfi_tclMsgFID_TUN_G_DEMODULATION_SWITCH_STATUSStatus oMsgData;
			vGetDataFromAmt( poMessage , oMsgData );

			m_u8DemodSwitch = oMsgData.DemodulationSwitch.u8Value;

			//Check if FM HD is enabled
			if((m_u8DemodSwitch & (midw_fi_tcl_b8_Tun_DemodSwitch::FI_C_U8_BIT_FM_HD)) == midw_fi_tcl_b8_Tun_DemodSwitch::FI_C_U8_BIT_FM_HD)
			{
				DataPool.vSetBoolValue(DPTUNER__HD_STATUS_FM,TRUE);
			}
			else
			{
				DataPool.vSetBoolValue(DPTUNER__HD_STATUS_FM,FALSE);
			}

			//Check if AM HD is enabled
			if((m_u8DemodSwitch & (midw_fi_tcl_b8_Tun_DemodSwitch::FI_C_U8_BIT_AM_HD)) == midw_fi_tcl_b8_Tun_DemodSwitch::FI_C_U8_BIT_AM_HD)
			{
				DataPool.vSetBoolValue(DPTUNER__HD_STATUS_AM,TRUE);
			}
			else
			{
				DataPool.vSetBoolValue(DPTUNER__HD_STATUS_AM,FALSE);
			}

			ETG_TRACE_USR4(("vHandleFID_G_DemodulationSwitch_From_FCTuner(), DemodulationSwitch = %d", m_u8DemodSwitch ));
			break;
         }

      case AMT_C_U8_CCAMSG_OPCODE_ERROR:
         {
            /* handle error message in a separate function */
            vHandleCCAErrorMessage( poMessage );
            break;
         }

      default : // unknown opcode
         {
            /* send error message back to server for unkown opcode */
            vSendErrorMessage(
               poMessage->u16GetFunctionID(),
               AMT_C_U16_ERROR_INVALID_OPCODE
               );

            break;
         }

      } // end of switch
   }
}

/*************************************************************************
* FUNCTION: clTuner_CCAhandler::vHandle_FID_TUN_S_CONFIGLIST_UPDATE_From_FCTuner( )
*
* DESCRIPTION: handler to extract data from the Method result message from server for
*               FID= FID_TUN_S_CONFIGLIST_UPDATE
*
* PARAMETER:   amt_tclServiceData* poMessage (data message to process)
*
* RETURNVALUE: NONE

*************************************************************************/
tVoid clTuner_CCAhandler::vHandle_FID_TUN_S_CONFIGLIST_UPDATE_From_FCTuner( amt_tclServiceData* poMessage )
{
	ETG_TRACE_USR4(("vHandle_FID_TUN_S_CONFIGLIST_UPDATE_From_FCTuner()"));
	/* check the parameter for not NULL and then for validity of a messsage */
   if ( (poMessage != NULL) && (poMessage->bIsValid()) )
   {
      switch (poMessage->u8GetOpCode())   // get the opcode from the message
      {

      case AMT_C_U8_CCAMSG_OPCODE_METHODRESULT:
         {
        	/* extract the data from AMT Class to FI Class */
        	 midw_tunerfi_tclMsgFID_TUN_S_CONFIGLIST_UPDATEMethodResult oMsgData;
			 vGetDataFromAmt( poMessage , oMsgData );

			 tU8 u8UpdateProgress = (tU8)oMsgData.TunerServer_StatusResponse.u8TunerStatus.enType;

			 ETG_TRACE_USR4(("vHandle_FID_TUN_S_CONFIGLIST_UPDATE_From_FCTuner(), u8Update Progress = %d", u8UpdateProgress ));

			 tBool bListUpdateStatus = DataPool.bGetValue(DPTUNER__LIST_UPDATE_STATUS);

			 if((u8UpdateProgress == (tU8)midw_fi_tcl_e8_Tun_ResponseStatus::FI_EN_TUN_RES_REQ_SUCCEED) && (bListUpdateStatus == TRUE))
			 {
			 	DataPool.vSetBoolValue(DPTUNER__LIST_UPDATE_STATUS,FALSE);
			 }
			break;
         }

      case AMT_C_U8_CCAMSG_OPCODE_ERROR:
         {
            /* handle error message in a separate function */
            vHandleCCAErrorMessage( poMessage );
            break;
         }

      default : // unknown opcode
         {
            /* send error message back to server for unkown opcode */
            vSendErrorMessage(
               poMessage->u16GetFunctionID(),
               AMT_C_U16_ERROR_INVALID_OPCODE
               );

            break;
         }

      } // end of switch
   }
}

/*************************************************************************
* FUNCTION: clTuner_CCAhandler::vHandleFID_G_Get_PSD_Data_From_FCTuner( )
*
* DESCRIPTION: handler to extract data from the status message from server for
*               FID= FID_TUN_G_HD_PSD_DATA
*
* PARAMETER:   amt_tclServiceData* poMessage (data message to process)
*
* RETURNVALUE: NONE

*************************************************************************/
tVoid clTuner_CCAhandler::vHandleFID_G_Get_PSD_Data_From_FCTuner( amt_tclServiceData* poMessage )
{
	ETG_TRACE_USR4(("vHandleFID_G_Get_PSD_Data_From_FCTuner()"));
	/* check the parameter for not NULL and then for validity of a messsage */
   if ( (poMessage != NULL) && (poMessage->bIsValid()) )
   {
      switch (poMessage->u8GetOpCode())   // get the opcode from the message
      {

      case AMT_C_U8_CCAMSG_OPCODE_STATUS:
         {
        	vStartInitSequence(MIDW_TUNERFI_C_U16_FID_TUN_G_HD_PSD_DATA);

            /* extract the data from AMT Class to FI Class */
            midw_tunerfi_tclMsgFID_TUN_G_HD_PSD_DATAStatus oMsgData;
			vGetDataFromAmt( poMessage , oMsgData );
			bpstl::string sTitle = oMsgData.sTitle.szValue;
			bpstl::string sAlbum = oMsgData.sAlbum.szValue;
			bpstl::string sArtist = oMsgData.sArtist.szValue;

			DataPool.vSetString(DPTUNER__HD_PSD_TITLE, sTitle);
			DataPool.vSetString(DPTUNER__HD_PSD_ALBUM, sAlbum);
			DataPool.vSetString(DPTUNER__HD_PSD_ARTIST, sArtist);

			ETG_TRACE_USR4((" Title = %s", sTitle.c_str()));
			ETG_TRACE_USR4((" Album = %s", sAlbum.c_str()));
			ETG_TRACE_USR4((" Artist = %s", sArtist.c_str()));
			break;
         }

      case AMT_C_U8_CCAMSG_OPCODE_ERROR:
         {
            /* handle error message in a separate function */
            vHandleCCAErrorMessage( poMessage );
            break;
         }

      default : // unknown opcode
         {
            /* send error message back to server for unkown opcode */
            vSendErrorMessage(
               poMessage->u16GetFunctionID(),
               AMT_C_U16_ERROR_INVALID_OPCODE
               );

            break;
         }

      } // end of switch
   }
}

/*************************************************************************
* FUNCTION: clTuner_CCAhandler::vHandleFID_TUN_GET_HD_TEST_MODE_DATA( )
*
* DESCRIPTION: Call handler to extract data from the status message from server for
*
* PARAMETER:   None
*
* RETURNVALUE: NONE

*************************************************************************/
tVoid clTuner_CCAhandler::vHandleFID_TUN_GET_HD_TEST_MODE_DATA()
{
	if ( m_poTrace != NULL )
	   {
		  m_poTrace->vTrace(  TR_LEVEL_HMI_INFO,
			 TR_CLASS_HSI_TUNER_MSG,
			 "Get HD Test Mode Data");
	   }
	bSendCCAMessage( MIDW_TUNERFI_C_U16_FID_TUN_G_HD_TEST_MODE_DATA,
			AMT_C_U8_CCAMSG_OPCODE_STATUS);
   return;
}


/*************************************************************************
* FUNCTION: clTuner_CCAhandler::vHandleFID_TUN_G_HD_TEST_MODE_DATA_From_FCTuner( )
*
* DESCRIPTION: handler to extract data from the status message from server for
*               FID= FID_TUN_G_HD_TEST_MODE_DATA
*
* PARAMETER:   amt_tclServiceData* poMessage (data message to process)
*
* RETURNVALUE: NONE

*************************************************************************/
tVoid clTuner_CCAhandler::vHandleFID_TUN_G_HD_TEST_MODE_DATA_From_FCTuner( amt_tclServiceData* poMessage )
{
	ETG_TRACE_USR4(("vHandleFID_TUN_G_HD_TEST_MODE_DATA_From_FCTuner()"));
	/* check the parameter for not NULL and then for validity of a messsage */
   if ( (poMessage != NULL) && (poMessage->bIsValid()) )
   {
      switch (poMessage->u8GetOpCode())   // get the opcode from the message
      {

      case AMT_C_U8_CCAMSG_OPCODE_STATUS:
         {
        	vStartInitSequence(MIDW_TUNERFI_C_U16_FID_TUN_G_HD_TEST_MODE_DATA);

            /* extract the data from AMT Class to FI Class */
            midw_tunerfi_tclMsgFID_TUN_G_HD_TEST_MODE_DATAStatus oMsgData;
			vGetDataFromAmt( poMessage , oMsgData );
		
			//Extract call sign
			
			bpstl::string sCallSign = oMsgData.TunerHDTestModeData.sHDCallSign;

			DataPool.vSetString(DPTUNER__TESTMODE_HD_CALL_SIGN, sCallSign);
	
			ETG_TRACE_USR4(("vHandleFID_TUN_G_HD_TEST_MODE_DATA_From_FCTuner(),sHDCallSign : %8s ",sCallSign.c_str()));
		
			//Extract stationID(PID)
		
			tU32 u32ProgID = oMsgData.TunerHDTestModeData.u32HDStationId;
		
			DataPool.vSetU32Value(DPTUNER__TESTMODE_HD_PRG_ID,u32ProgID);
									 
			ETG_TRACE_USR4(("vHandleFID_TUN_G_HD_TEST_MODE_DATA_From_FCTuner(), u32HDStationId : %d ",u32ProgID));
		
			//Extract CD No.
			tU8 u8CDno = oMsgData.TunerHDTestModeData.u8CD_NO;
			
			DataPool.vSetU32Value(DPTUNER__TESTMODE_HD_CD_NO,u8CDno);
			
			ETG_TRACE_USR4(("vHandleFID_TUN_G_HD_TEST_MODE_DATA_From_FCTuner(), u8CD_NO : %d ",u8CDno));
		
			break;
         }

      case AMT_C_U8_CCAMSG_OPCODE_ERROR:
         {
            /* handle error message in a separate function */
            vHandleCCAErrorMessage( poMessage );
            break;
         }

      default : // unknown opcode
         {
            /* send error message back to server for unkown opcode */
            vSendErrorMessage(
               poMessage->u16GetFunctionID(),
               AMT_C_U16_ERROR_INVALID_OPCODE
               );

            break;
         }

      } // end of switch
   }
}





/*************************************************************************
*  FUNCTION:  tVoid clTuner_CCAhandler::vHandleFID_TUN_S_SET_DEMODULATION_SWITCH_To_FCTuner()
*
* DESCRIPTION:  handler to form a CCA Message with embedding required data
*                 for FID= FID_TUN_S_SET_DEMODULATION_SWITCH and send to Server
*
* PARAMETER:  tU8 u8Band and tBool bHDSetting  ( to embed in the sending CCA message)
*
* RETURNVALUE: NONE
*
* History:
* InitialVersion
*
*************************************************************************/
tVoid clTuner_CCAhandler::vHandleFID_TUN_S_SET_DEMODULATION_SWITCH_To_FCTuner(  tU8 u8Band, tBool bHDSetting)
{
	midw_tunerfi_tclMsgFID_TUN_S_SET_DEMODULATION_SWITCHMethodStart oMsgData;

	/*if(u8Band == TUN_BAND_FM)
	{//Band is FM*/
		if(bHDSetting == TRUE)
			//Set FM_HD ON
		    oMsgData.DemodulationSwitch.u8Value = m_u8DemodSwitch | (midw_fi_tcl_b8_Tun_DemodSwitch::FI_C_U8_BIT_FM_HD);
		else
			//Set FM_HD OFF
			oMsgData.DemodulationSwitch.u8Value = m_u8DemodSwitch & TUN_FM_HD_RESET;
	/*}
	else
	{//Band is AM
		if(bHDSetting == TRUE)
			//Set AM_HD ON
			oMsgData.DemodulationSwitch.u8Value = m_u8DemodSwitch | (midw_fi_tcl_b8_Tun_DemodSwitch::FI_C_U8_BIT_AM_HD);
		else
			//Set AM_HD OFF
			oMsgData.DemodulationSwitch.u8Value = m_u8DemodSwitch & TUN_AM_HD_RESET;
	}*/

	ETG_TRACE_USR4(("vHandleFID_TUN_S_SET_DEMODULATION_SWITCH_To_FCTuner(), Band = %d, Req setting = %d, Current switch = %d, Requested switch = %d", u8Band, bHDSetting, m_u8DemodSwitch, oMsgData.DemodulationSwitch.u8Value ));

   bSendCCAMessage( MIDW_TUNERFI_C_U16_FID_TUN_S_SET_DEMODULATION_SWITCH,
      AMT_C_U8_CCAMSG_OPCODE_METHODSTART,
      oMsgData
      );

   return;
}

/*************************************************************************
* FUNCTION: clTuner_CCAhandler::vHandleFID_TUN_G_UPDATELANDSCAPE_PROGRESS_From_FCTuner( )
*
* DESCRIPTION: handler to extract data from the status message from server for
*               FID= FID_TUN_G_UPDATELANDSCAPE_PROGRESS
*
* PARAMETER:   amt_tclServiceData* poMessage (data message to process)
*
* RETURNVALUE: NONE

*************************************************************************/
tVoid clTuner_CCAhandler::vHandleFID_TUN_G_UPDATELANDSCAPE_PROGRESS_From_FCTuner( amt_tclServiceData* poMessage )
{
	ETG_TRACE_USR4(("vHandleFID_TUN_G_UPDATELANDSCAPE_PROGRESS_From_FCTuner()"));
	/* check the parameter for not NULL and then for validity of a messsage */
   if ( (poMessage != NULL) && (poMessage->bIsValid()) )
   {
      switch (poMessage->u8GetOpCode())   // get the opcode from the message
      {

      case AMT_C_U8_CCAMSG_OPCODE_STATUS:
         {
        	vStartInitSequence(MIDW_TUNERFI_C_U16_FID_TUN_G_UPDATELANDSCAPE_PROGRESS);

            /* extract the data from AMT Class to FI Class */
        	midw_tunerfi_tclMsgFID_TUN_G_UPDATELANDSCAPE_PROGRESSStatus oMsgData;
			vGetDataFromAmt( poMessage , oMsgData );

			tU8 u8UpdateProgress = oMsgData.u8FreqPerc;

			ETG_TRACE_USR4(("vHandleFID_TUN_G_UPDATELANDSCAPE_PROGRESS_From_FCTuner(), u8Update Progress = %d", u8UpdateProgress ));

			tBool bListUpdateStatus = DataPool.bGetValue(DPTUNER__LIST_UPDATE_STATUS);

			/*if((u8UpdateProgress == UPDATE_PROGRESS_COMPLETE) && (bListUpdateStatus == TRUE))
			{
				DataPool.vSetBoolValue(DPTUNER__LIST_UPDATE_STATUS,FALSE);
			}*/

			break;
         }

      case AMT_C_U8_CCAMSG_OPCODE_ERROR:
         {
            /* handle error message in a separate function */
            vHandleCCAErrorMessage( poMessage );
            break;
         }

      default : // unknown opcode
         {
            /* send error message back to server for unkown opcode */
            vSendErrorMessage(
               poMessage->u16GetFunctionID(),
               AMT_C_U16_ERROR_INVALID_OPCODE
               );

            break;
         }

      } // end of switch
   }
}

/*************************************************************************
*  FUNCTION:  tVoid clTuner_CCAhandler::vHandleFID_TUN_S_CONFIGLIST_UPDATE_To_FCTuner()
*
* DESCRIPTION:  handler to form a CCA Message with embedding required data
*                 for FID= FID_TUN_S_CONFIGLIST_UPDATE and send to Server
*
* PARAMETER:  tU8 u8ListID, tU8 u8Action  ( to embed in the sending CCA message)
*
* RETURNVALUE: NONE
*
* History:
* InitialVersion
*
*************************************************************************/
tVoid clTuner_CCAhandler::vHandleFID_TUN_S_CONFIGLIST_UPDATE_To_FCTuner( tU8 u8ListID, tBool bAction )
{
	ETG_TRACE_USR4(("vHandleFID_TUN_S_CONFIGLIST_UPDATE_To_FCTuner(), ListID = %d, Action = %d", u8ListID, bAction));
	midw_tunerfi_tclMsgFID_TUN_S_CONFIGLIST_UPDATEMethodStart oMsgData;

	if(u8ListID == TUN_LIST_TYPE_FM)
	{
		oMsgData.e8ConfigListID.enType = midw_fi_tcl_e8_Tun_ConfigListID::FI_EN_TUN_TUN_CONFIG_LIST_FM;
	}
	else if(u8ListID == TUN_LIST_TYPE_AM)
	{
		oMsgData.e8ConfigListID.enType = midw_fi_tcl_e8_Tun_ConfigListID::FI_EN_TUN_TUN_CONFIG_LIST_MW;
	}

	oMsgData.bAction = bAction;

	bSendCCAMessage( MIDW_TUNERFI_C_U16_FID_TUN_S_CONFIGLIST_UPDATE,
      AMT_C_U8_CCAMSG_OPCODE_METHODSTART,
      oMsgData
      );

   return;
}

/*************************************************************************
*  FUNCTION:  tVoid clTuner_CCAhandler::vHandleFID_TUN_S_SELECT_HD_AUDIOPRGM_To_FCTuner()
*
* DESCRIPTION:  handler to form a CCA Message with embedding required data
*                 for FID= FID_TUN_S_SELECT_HD_AUDIOPRGM and send to Server
*
* PARAMETER:  tU8 u8SelectMode, tU8 u8HDAudPrgmNum, tBool bDirection  ( to embed in the sending CCA message)
*
* RETURNVALUE: NONE
*
* History:
* InitialVersion
*
*************************************************************************/
tVoid clTuner_CCAhandler::vHandleFID_TUN_S_SELECT_HD_AUDIOPRGM_To_FCTuner( tU8 u8SelectMode, tU8 u8HDAudPrgmNum, tBool bDirection )
{
	ETG_TRACE_USR4(("vHandleFID_TUN_S_SELECT_HD_AUDIOPRGM_To_FCTuner(), u8SelectMode = %d, u8HDAudPrgmNum = %d, bDirection = %d", u8SelectMode, u8HDAudPrgmNum, bDirection));
	midw_tunerfi_tclMsgFID_TUN_S_SELECT_HD_AUDIOPRGMMethodStart oMsgData;

	oMsgData.SelectMode.enType = (midw_fi_tcl_e8_Tun_SelectMode::tenType)u8SelectMode;

	oMsgData.e8HDAudioPrg.enType = (midw_fi_tcl_e8_Tun_HDAudioProgram::tenType)u8HDAudPrgmNum;

	oMsgData.bDirection = bDirection;

	bSendCCAMessage( MIDW_TUNERFI_C_U16_FID_TUN_S_SELECT_HD_AUDIOPRGM,
      AMT_C_U8_CCAMSG_OPCODE_METHODSTART,
      oMsgData
      );

   return;
}

/*************************************************************************
* FUNCTION:  tVoid clTuner_CCAhandler::vHandleFID_G_Get_StaticList_From_FCTuner()
*
* DESCRIPTION: handler to extract data from the status message from server for
*               FID= FID_TUN_G_GET_STATIC_LIST
*
* PARAMETER:   amt_tclServiceData* poMessage (data message to process)
*
* RETURNVALUE: NONE
*
* History: InitialVersion
*
* AUTHOR: vnd4kor ECV2
*************************************************************************/
tVoid clTuner_CCAhandler::vHandleFID_G_Get_StaticList_From_FCTuner\
										( amt_tclServiceData* poMessage )
{
    ETG_TRACE_USR4(("vHandleFID_G_Get_StaticList_From_FCTuner()"));
	/* check the parameter for not NULL and then for validity of a messsage */
   if ( (poMessage != NULL) && (poMessage->bIsValid()) )
   {
	   switch (poMessage->u8GetOpCode())   // get the opcode from the message
       {

      case AMT_C_U8_CCAMSG_OPCODE_STATUS:
         {
            vStartInitSequence(MIDW_TUNERFI_C_U16_FID_TUN_G_GET_STATIC_LIST);

			/** Handler function to extract data and write to dp*/
 			vHandleStaticListData( poMessage );
            break;
         }

      case AMT_C_U8_CCAMSG_OPCODE_ERROR:
         {
            /* handle error message in a separate function */
            vHandleCCAErrorMessage( poMessage );
            break;
         }

      default : // unknown opcode
         {
            /* send error message back to server for unkown opcode */
            vSendErrorMessage(
               poMessage->u16GetFunctionID(),
               AMT_C_U16_ERROR_INVALID_OPCODE
               );

            break;
         }

      } // end of switch
   }
}

/*************************************************************************
* FUNCTION:  tVoid clTuner_CCAhandler::vHandleStaticListData()
*
* DESCRIPTION: handler to extract data from FID_TUN_G_GET_STATIC_LIST status message 
*  and write data into datapool
*
* PARAMETER:   amt_tclServiceData* poMessage (data message to process)
*
* RETURNVALUE: NONE
*
* History: InitialVersion
*
* AUTHOR: vnd4kor ECV2
*************************************************************************/
tVoid clTuner_CCAhandler::vHandleStaticListData(amt_tclServiceData* poMessage)
{
	/* object for handling the status data ( server defined) */
	midw_tunerfi_tclMsgFID_TUN_G_GET_STATIC_LISTStatus  otunmsg_tclStaticListDataS;
	vGetDataFromAmt( poMessage , otunmsg_tclStaticListDataS );

	/** Extract data from the message*/
	midw_fi_tcl_e8_Tun_ListID::tenType e8_ListID = otunmsg_tclStaticListDataS.e8ListID.enType;
	tU8 u8NoOfElementInList = otunmsg_tclStaticListDataS.u8NumofElementsinList;

	tU16 u16PresetHDStatus = 0x0000;
	tU16 u16BitMask = 0x0001;

	if((e8_ListID == midw_fi_tcl_e8_Tun_ListID::FI_EN_TUN_BANK_FM1)||(e8_ListID == midw_fi_tcl_e8_Tun_ListID::FI_EN_TUN_BANK_FM2))
	{
		/** Get dp value for FM Presets with HD. Done in order to prevent FM1 Preset's HD status
		from being over written while extracting for FM2 and vice-versa**/
		u16PresetHDStatus = DataPool.u8GetValue(DPTUNER__FM_PRESET_LIST_HD_STATUS);

		if(e8_ListID == midw_fi_tcl_e8_Tun_ListID::FI_EN_TUN_BANK_FM2)
		{
			/** Left shift u16BitMask by offset 6. In datapool first 6 bits belong to FM1
			and the next 6 belong to FM2 */
			u16BitMask = u16BitMask << MAX_PRESETS_PER_BANK;
		}
	}

	for(tU8 u8Index = 0; u8Index < u8NoOfElementInList; u8Index++)
	{
		tU32 u32HDStationId = otunmsg_tclStaticListDataS.Tun_StaticListElements.TunerStaticListElement[u8Index].u32HDStationId;

		u16PresetHDStatus = (u32HDStationId > 0) ?
			(u16PresetHDStatus | u16BitMask): (u16PresetHDStatus & ~u16BitMask);

			u16BitMask <<= 1;
	}

	/** Write it to datapool */
	if(e8_ListID == midw_fi_tcl_e8_Tun_ListID::FI_EN_TUN_BANK_AM_MW1)
	{
		DataPool.vSetU8Value(DPTUNER__AM_PRESET_LIST_HD_STATUS,(tU8)u16PresetHDStatus);
	}

	else if((e8_ListID == midw_fi_tcl_e8_Tun_ListID::FI_EN_TUN_BANK_FM1)||(e8_ListID == midw_fi_tcl_e8_Tun_ListID::FI_EN_TUN_BANK_FM2))
	{
		DataPool.vSetU16Value(DPTUNER__FM_PRESET_LIST_HD_STATUS,u16PresetHDStatus);
	}

}
#endif


/*************************************************************************
*  FUNCTION: clTuner_CCAhandler::vHandleFID_G_SET_DDADDS_From_FCTuner( )
*
* DESCRIPTION: handler to extract data from the status message from server for
*               FID= FID_TUN_G_SETUP_DDADDS
*
* PARAMETER:   amt_tclServiceData* poMessage (data message to process)
*
* RETURNVALUE: NONE
*
* History:
* InitialVersion
*
*************************************************************************/
tVoid clTuner_CCAhandler::vHandleFID_G_SET_DDADDS_From_FCTuner( amt_tclServiceData* poMessage )
{

	ETG_TRACE_USR4(("vHandleFID_G_SET_DDADDS_From_FCTuner()"));
	/* check the parameter for not NULL and then for validity of a messsage */
   if ( (poMessage != NULL) && (poMessage->bIsValid()) )
   {

      switch (poMessage->u8GetOpCode())   // get the opcode from the message
      {

      case AMT_C_U8_CCAMSG_OPCODE_STATUS:
         {
            vStartInitSequence(MIDW_TUNERFI_C_U16_FID_TUN_G_SETUP_DDADDS);

            /* object for handling the status data of type tunmsg_tclTestModeDataS ( server defined) */
            midw_tunerfi_tclMsgFID_TUN_G_SETUP_DDADDSStatus  otunmsg_tclTestModeDataS;
            vGetDataFromAmt( poMessage , otunmsg_tclTestModeDataS );
            if ( poFI != NULL )
            {
               midw_fi_tcl_e8_DDADDS_Mode::tenType enModeType  =  otunmsg_tclTestModeDataS.DDADDS_Mode.enType;
               poFI->WriteTestModeDataToDP(TM_DATA_TESTMODE_SET_DDADDS, (tU8)enModeType);
            }

            break;
         }

      case AMT_C_U8_CCAMSG_OPCODE_ERROR:
         {
            /* handle error message in a separate function */
            vHandleCCAErrorMessage( poMessage );

            break;
         }

      default : // unknown opcode
         {
            /* send error message back to server for unkown opcode */
            vSendErrorMessage(
               poMessage->u16GetFunctionID(),
               AMT_C_U16_ERROR_INVALID_OPCODE
               );

            break;
         }

      } // end of switch
   }
}

/*************************************************************************
*  FUNCTION:  tVoid clTuner_CCAhandler::vHandleFID_S_ATFREQUENCY_To_FCTuner()
*
* DESCRIPTION:  handler to form a CCA Message with embedding required data
*                 for FID= FID_TUN_S_ATFREQUENCY_ and send to Server
*
* PARAMETER:  tU8 u8FreqStep  ( to embed in the sending CCA message)
*
* RETURNVALUE: NONE
*
* History:
* InitialVersion
*
*************************************************************************/

tVoid clTuner_CCAhandler::vHandleFID_S_ATFREQUENCY_To_FCTuner(  tU8   u8Direction,
                                                              tU8   u8Steps,
                                                              tU8   TunerHeader,
                                                              tBool blAF)
{
   midw_tunerfi_tclMsgFID_TUN_S_ATFREQUENCYMethodStart oMsgData;
   oMsgData.TunerManualSingleStep.ManualFreqAdjustment.enType = (midw_fi_tcl_e8_Tun_ManualFreqAdjustment::tenType)u8Direction;
   oMsgData.TunerManualSingleStep.u8NumberOfTicks = u8Steps;
   oMsgData.TunerHeader.enType = (midw_fi_tcl_e8_Tun_TunerHeader::tenType)TunerHeader;
   oMsgData.u8AcceptAF.enType = (midw_fi_tcl_e8_Tun_AcceptAF::tenType)blAF;

   bSendCCAMessage( MIDW_TUNERFI_C_U16_FID_TUN_S_ATFREQUENCY,
      AMT_C_U8_CCAMSG_OPCODE_METHODSTART,
      oMsgData
      );

   return;
}



/*************************************************************************
*  FUNCTION:  tVoid clTuner_CCAhandler::vHandleFID_S_SET_MIXEDPRESET_LIST()
*
* DESCRIPTION:  handler to form a CCA Message with embedding required data
*                 for FID= FID_TUN_S_SET_MIXEDPRESET_LIST and send to Server
*
* PARAMETER:  tU8 u8Bank  
*
* RETURNVALUE: NONE
*************************************************************************/
tVoid clTuner_CCAhandler::vHandleFID_S_SET_MIXEDPRESET_LIST(tU8 u8Bank)
{
	midw_tunerfi_tclMsgFID_TUN_S_SET_MIXEDPRESET_LISTMethodStart oMsgData;
	oMsgData.e8ListID.enType = (midw_fi_tcl_e8_Tun_TunerMixedpresetList::tenType)u8Bank;
	
	ETG_TRACE_USR4(("clTuner_CCAhandler::vHandleFID_S_SET_MIXEDPRESET_LIST = %d %d" , oMsgData.e8ListID.enType
	, (midw_fi_tcl_e8_Tun_TunerMixedpresetList::tenType)u8Bank));
	
	bSendCCAMessage( MIDW_TUNERFI_C_U16_FID_TUN_S_SET_MIXEDPRESET_LIST,
      AMT_C_U8_CCAMSG_OPCODE_METHODSTART,
      oMsgData
      );

   return;
}


/*************************************************************************
*  FUNCTION:    clTuner_CCAhandler::vHandleFID_S_SET_STATIC_LIST_To_FCTuner()
*
* DESCRIPTION: handler to form a CCA Message with embedding required data
*               for FID= FID_TUN_S_SET_STATIC_LIST  and send to Server
*
* PARAMETER:  tU8 u8StaticListId  ( to embed in the sending CCA message)
*
* RETURNVALUE: NONE
*
* History:
* InitialVersion
* 
*************************************************************************/
tVoid clTuner_CCAhandler::vHandleFID_S_SET_STATIC_LIST_To_FCTuner(tU8 u8StaticListId)
{

   if ( m_poTrace != NULL )
   {
      m_poTrace->vTrace(  TR_LEVEL_HMI_INFO,
         TR_CLASS_HSI_TUNER_MSG,
         (tU16)TUN_FID_STATIC_LIST_TO_SERV);
   }

   midw_tunerfi_tclMsgFID_TUN_S_SET_STATIC_LISTMethodStart oMsgData;
   oMsgData.e8TunerStaticList.enType = (midw_fi_tcl_e8_Tun_TunerStaticList::tenType)u8StaticListId;

   bSendCCAMessage( MIDW_TUNERFI_C_U16_FID_TUN_S_SET_STATIC_LIST,
      AMT_C_U8_CCAMSG_OPCODE_METHODSTART, 
      oMsgData
      );

   return ;



}

/*************************************************************************
*  FUNCTION:   clTuner_CCAhandler::vHandleFID_S_SCAN_BAND_To_FCTuner()
*
* DESCRIPTION: handler to form a CCA Message with embedding required data
*              for FID= FID_TUN_S_SCAN_BAND and send to Server
*
* PARAMETER:   tU8 u8Mode  -  with which mode should the scan be started
*
* RETURNVALUE: NONE
*
* History:
* InitialVersion
* 
*************************************************************************/
tVoid clTuner_CCAhandler::vHandleFID_S_SCAN_BAND_To_FCTuner(tU8 u8Mode)
{
   if ( m_poTrace != NULL )
   {
      m_poTrace->vTrace(  TR_LEVEL_HMI_INFO,
         TR_CLASS_HSI_TUNER_MSG,
         (tU16)TUN_FID_SCAN_BAND_VAG_TO_SERV);
   }

   midw_tunerfi_tclMsgFID_TUN_S_SCAN_BANDMethodStart oMsgData;
   oMsgData.Tuner_ScanMode.enType = (midw_fi_tcl_Tun_Scan_Mode::tenType)u8Mode;

   bSendCCAMessage( MIDW_TUNERFI_C_U16_FID_TUN_S_SCAN_BAND,
      AMT_C_U8_CCAMSG_OPCODE_METHODSTART, 
      oMsgData
      );
   return;
}



/*************************************************************************
*  FUNCTION:    clTuner_CCAhandler::vHandleFID_S_SET_FEATURE_To_FCTuner()
*
* DESCRIPTION: handler to form a CCA Message with embedding required data
*               for FID= FID_TUN_S_SET_FEATURE  and send to Server
*
* PARAMETER: tU32 u32Feature ,tU32 u32Mask(to embed in the sending CCA message)
*
* RETURNVALUE: NONE
*
* History:
* InitialVersion
* 
*************************************************************************/
tVoid clTuner_CCAhandler::vHandleFID_S_SET_FEATURE_To_FCTuner(tU32 u32Feature,
                                                              tU32 u32Mask)
{
   if ( NULL !=  m_poTrace)
   {
      m_poTrace->vTrace(  TR_LEVEL_HMI_INFO,
         TR_CLASS_HSI_TUNER_MSG,
         (tU16)TUN_FID_SET_FEATURE_TO_SERV);
   }

   midw_tunerfi_tclMsgFID_TUN_S_SET_FEATUREMethodStart oMsgData;
   oMsgData.TunerFeature.b32Features.u32Value     = u32Mask;
   oMsgData.TunerFeature.b32MaskFeatures.u32Value = u32Feature;

   bSendCCAMessage( MIDW_TUNERFI_C_U16_FID_TUN_S_SET_FEATURE,
      AMT_C_U8_CCAMSG_OPCODE_METHODSTART,
      oMsgData
      );

   return ;
}


/*************************************************************************
*  FUNCTION:    clTuner_CCAhandler::vHandleFID_S_SET_HICUT_To_FCTuner()
*
* DESCRIPTION: handler to form a CCA Message with embedding required data
*               for FID= FID_TUN_S_SET_HICUT  and send to Server
*
* PARAMETER:   tU8 u8HicutValue ( to embed in the sending CCA message)
*
* RETURNVALUE: NONE
*
* History:
* InitialVersion
* 
*************************************************************************/
tVoid clTuner_CCAhandler::vHandleFID_S_SET_HICUT_To_FCTuner( tU8 u8HicutValue)
{
   if ( NULL !=  m_poTrace)
   {
      m_poTrace->vTrace(  TR_LEVEL_HMI_INFO,
         TR_CLASS_HSI_TUNER_MSG,
         (tU16)TUN_FID_SET_HICUT_TO_SERV);
   }

   midw_tunerfi_tclMsgFID_TUN_S_SET_HICUTMethodStart oMsgData;
   oMsgData.u8AbsVal = u8HicutValue;

   bSendCCAMessage( MIDW_TUNERFI_C_U16_FID_TUN_S_SET_HICUT,
      AMT_C_U8_CCAMSG_OPCODE_METHODSTART, 
      oMsgData
      );

   return;

}


/*************************************************************************
*  FUNCTION:    clTuner_CCAhandler::vHandleFID_S_GET_CALIBRATION_To_FCTuner()
*
* DESCRIPTION: handler to form a CCA Message with embedding required data
*               for FID= FID_TUN_S_CALIBRATION_DATA  and send to Server
*
* PARAMETER:
*
* RETURNVALUE: NONE
*
* History:
* InitialVersion
*
*************************************************************************/
tVoid clTuner_CCAhandler::vHandleFID_S_GET_CALIBRATION_To_FCTuner()
{
	ETG_TRACE_USR4(("clTuner_CCAhandler::vHandleFID_S_GET_CALIBRATION_To_FCTuner"));

	midw_tunerfi_tclMsgFID_TUN_S_CALIBRATION_DATAMethodStart oMsgData;

	bSendCCAMessage( MIDW_TUNERFI_C_U16_FID_TUN_S_CALIBRATION_DATA,
      AMT_C_U8_CCAMSG_OPCODE_METHODSTART,
      oMsgData
      );

    return;

}





/*************************************************************************
*  FUNCTION:    clTuner_CCAhandler::vHandleFID_S_SET_DDADDS_To_FCTuner()
*
* DESCRIPTION: handler to form a CCA Message with embedding required data
*               for FID= FID_TUN_S_SET_DDADDS  and send to Server
*
* PARAMETER:   tU8 u8TunerMode ( to embed in the sending CCA message)
*
* RETURNVALUE: NONE
*
* History:
* InitialVersion
* 
*************************************************************************/
tVoid clTuner_CCAhandler::vHandleFID_S_SET_DDADDS_To_FCTuner( tU8 u8TunerMode)
{
   if ( NULL !=  m_poTrace)
   {
      m_poTrace->vTrace(  TR_LEVEL_HMI_INFO,
         TR_CLASS_HSI_TUNER_MSG,
         (tU16)TUN_FID_SET_DDADDS_TO_SERV);
   }

   /* create a CCA Msg object of type gm_tclU8Message with the Basic Data */
   midw_tunerfi_tclMsgFID_TUN_S_SETUP_DDADDSMethodStart oMsgData;
   oMsgData.DDADDS_Mode.enType = (midw_fi_tcl_e8_DDADDS_Mode::tenType)u8TunerMode;

   bSendCCAMessage( MIDW_TUNERFI_C_U16_FID_TUN_S_SETUP_DDADDS,
      AMT_C_U8_CCAMSG_OPCODE_METHODSTART, 
      oMsgData
      );

   return;

}


/*************************************************************************
*  FUNCTION:    clTuner_CCAhandler::vHandleFID_S_AFLIST_V2_From_FCTuner()
*
* DESCRIPTION: handler to form a CCA Message with embedding required data
*               for FID= FID_TUN_S_AFLIST_V2  and send to Server
*
* PARAMETER: tU8 u8Value ( to embed in the sending CCA message)
*
* RETURNVALUE: NONE
*
* History:
* InitialVersion
* 
*************************************************************************/
tVoid clTuner_CCAhandler::vHandleFID_S_AFLIST_V2_To_FCTuner(tU8 u8Command , tU8 u8Value)
{
   if( NULL == poAFListManager )
   {
      return; //as no need to proceed further...
   }

   if ( NULL !=  m_poTrace)
   {
      m_poTrace->vTrace(  TR_LEVEL_HMI_INFO,
         TR_CLASS_HSI_TUNER_MSG, 
         (tU16)TUN_FID_AFLIST_V2_TO_SERV);
   }

   switch( u8Command )
   {

   case LOAD_LIST:
   case CLOSE_LIST:
      {
         /* create a CCA Msg object of type gm_tclU8Message with the Basic Data */
         midw_tunerfi_tclMsgFID_TUN_S_AFLIST_V2MethodStart oMsgData;
         oMsgData.u8Notification.enType = (midw_fi_tcl_e8_Tun_Notification::tenType)u8Value;

         bSendCCAMessage( MIDW_TUNERFI_C_U16_FID_TUN_S_AFLIST_V2,
            AMT_C_U8_CCAMSG_OPCODE_METHODSTART,
            oMsgData
            );

         if( CLOSE_LIST == u8Command )
         {
            poAFListManager->vCloseList();
         }

         break;
      }

   case NEXT_LIST:
      {
         poAFListManager->vGetNextSetOfElements();

         break;
      }

   case PREV_LIST:
      {
         poAFListManager->vGetPrevSetOfElements();

         break;
      }

   case SET_LIST_INDEX:
      {
         //get the Base index .. add it to the offset...
         poAFListManager->vGetElement(poAFListManager->GetCurrentBaseIndex()+ u8Value );

         break;
      }


   default:
      return ;
   }

}




/*************************************************************************
* FUNCTION:    clTuner_CCAhandler::vHandleFID_S_STOP_ANNOUNCEMENT_To_FCTuner()
*
* DESCRIPTION: handler to form a CCA Message with embedding required data
*               for FID= FID_TUN_S_OPERATION  and send to Server
*
* PARAMETER:   tU8 u8Announcement ( to embed in the sending CCA message)
*
* RETURNVALUE: NONE
*
* History:
* InitialVersion
* 
*************************************************************************/
tVoid clTuner_CCAhandler::vHandleFID_S_STOP_ANNOUNCEMENT_To_FCTuner(
   tU8 u8Announcement)
{
   if ( NULL !=  m_poTrace)
   {
      m_poTrace->vTrace(  TR_LEVEL_HMI_INFO, 
         TR_CLASS_HSI_TUNER_MSG, 
         (tU16)TUN_FID_STOP_ANNOUNCEMENT_TO_SERV);
   }

   /* create a CCA Msg object oMsgData */
   midw_tunerfi_tclMsgFID_TUN_S_STOP_ANNOUNCEMENTMethodStart oMsgData;
   oMsgData.StopAnnouncements.enType = (midw_fi_tcl_e8_StopAnnouncements::tenType)u8Announcement;

   bSendCCAMessage( MIDW_TUNERFI_C_U16_FID_TUN_S_STOP_ANNOUNCEMENT,
      AMT_C_U8_CCAMSG_OPCODE_METHODSTART, 
      oMsgData
      );

   return;
}

/*************************************************************************
*  FUNCTION:   clTuner_CCAhandler::vHandleFID_S_DIRECT_FREQUENCY_To_FCTuner()
*
* DESCRIPTION: handler to form a CCA Message with embedding required data
*               for FID= FID_TUN_S_DIRECT_FREQUENCY  and send to Server
*
* PARAMETER:   tU32 u32Frequency ( to embed in the sending CCA message)
*
* RETURNVALUE: NONE
*
* History:
* InitialVersion
* 
*************************************************************************/ 
tVoid clTuner_CCAhandler::vHandleFID_S_DIRECT_FREQUENCY_To_FCTuner(
   tU32 u32Frequency )
{
   if ( NULL !=  m_poTrace)
   {
      m_poTrace->vTrace(  TR_LEVEL_HMI_INFO,
         TR_CLASS_HSI_TUNER_MSG, 
         (tU16)TUN_FID_DIRECT_FREQ_TO_SERV);
   }

   /* create a CCA Msg object oMsgData */
   midw_tunerfi_tclMsgFID_TUN_S_DIRECT_FREQUENCYMethodStart oMsgData;
   oMsgData.u32Frequency = u32Frequency;
   
   m_u32Frequency = u32Frequency;
   if(bSeekState != 0)
   {
   bDirectFreq = true;
   }
   ETG_TRACE_USR4(("vHandleFID_S_DIRECT_FREQUENCY_To_FCTuner: bDirectFreq : %d"
                       		       			, 	bDirectFreq));
   bSendCCAMessage( MIDW_TUNERFI_C_U16_FID_TUN_S_DIRECT_FREQUENCY,
      AMT_C_U8_CCAMSG_OPCODE_METHODSTART, 
      oMsgData
      );

   return ;

}

/*************************************************************************
*  FUNCTION: clTuner_CCAhandler::vHandleFID_S_SAVE_STATIC_LIST_ELEM_To_FCTuner()
*
* DESCRIPTION: handler to form a CCA Message with embedding required data
*              for FID= FID_TUN_S_SAVE_STATIC_LIST_ELEM and send to Server
*
* PARAMETER:  tU8 u8ListElement ( to embed in the sending CCA message)
*
* RETURNVALUE: NONE
*
* History:
* InitialVersion
* 
*************************************************************************/ 
tVoid  clTuner_CCAhandler::vHandleFID_S_SAVE_STATIC_LIST_ELEM_To_FCTuner(
   tU8 u8ListElement)
{

   if ( NULL !=  m_poTrace)
   {
      m_poTrace->vTrace(  TR_LEVEL_HMI_INFO,
         TR_CLASS_HSI_TUNER_MSG, 
         (tU16)TUN_FID_SAVE_STATIC_LIST_ELEM_TO_SERV);
   }

   /* create a CCA Msg object oMsgData */
   midw_tunerfi_tclMsgFID_TUN_S_SAVE_STATIC_LIST_ELEMMethodStart oMsgData;
   oMsgData.u8ListElementID = u8ListElement;

   bSendCCAMessage( MIDW_TUNERFI_C_U16_FID_TUN_S_SAVE_STATIC_LIST_ELEM,
      AMT_C_U8_CCAMSG_OPCODE_METHODSTART, 
      oMsgData
      );

   return ;

}


/*************************************************************************
*  FUNCTION: clTuner_CCAhandler::vHandleFID_S_AUTOSTORE_STATIC_LIST_To_FCTuner()
*
* DESCRIPTION: handler to form a CCA Message with embedding required data
*              for FID= FID_TUN_S_AUTOSTORE_STATIC_LIST and send to Server
*
* PARAMETER:  tU8 u8ListID ( to embed in the sending CCA message)
*
* RETURNVALUE: NONE
*
* History:
* InitialVersion
* 
*************************************************************************/ 
tVoid  clTuner_CCAhandler::vHandleFID_S_AUTOSTORE_STATIC_LIST_To_FCTuner(tU8 u8ListID)
{
   if ( NULL !=  m_poTrace)
   {
      m_poTrace->vTrace(  TR_LEVEL_HMI_INFO,
         TR_CLASS_HSI_TUNER_MSG, 
         (tU16)TUN_FID_AUTOSTORE_STATIC_LIST_TO_SERV);
   }

   /* create a CCA Msg object oMsgData */
   midw_tunerfi_tclMsgFID_TUN_S_AUTOSTORE_STATIC_LISTMethodStart oMsgData;
   oMsgData.u8ListID = u8ListID;

   bSendCCAMessage( MIDW_TUNERFI_C_U16_FID_TUN_S_AUTOSTORE_STATIC_LIST,
      AMT_C_U8_CCAMSG_OPCODE_METHODSTART,
      oMsgData
      );

   return ;
}

#ifdef VARIANT_S_FTR_ENABLE_HMI_TUNER_ITG5 
/*************************************************************************
*  FUNCTION: clTuner_CCAhandler::vHandleFID_S_SEL_LIST_ELEM_To_FCTuner()
*
* DESCRIPTION: handler to form a CCA Message with embedding required data
*              for FID= FID_TUN_S_SAVE_STATIC_LIST_ELEM and send to Server
*
* PARAMETER:  tU8 u8ListType , u8ListElement ( to embed in the sending CCA message)
*
* RETURNVALUE: NONE
*
* History:
* InitialVersion
*
*************************************************************************/
tVoid  clTuner_CCAhandler::vHandleFID_S_SEL_LIST_ELEM_To_FCTuner(tU8 u8ListType,
                                                                 tU8 u8ListElement, tU8 u8ListID)
{

   if ( NULL !=  m_poTrace)
   {
      m_poTrace->vTrace(  TR_LEVEL_HMI_INFO,
         TR_CLASS_HSI_TUNER_MSG,
         (tU16)TUN_FID_SEL_LIST_ELEM_TO_SERV,
         ONE_ELEMENT,
         (tU8*)&u8ListElement);
   }

   /* create a CCA Msg object oMsgData */
   midw_tunerfi_tclMsgFID_TUN_S_SEL_LIST_ELEMMethodStart oMsgData;
   oMsgData.Sel_List_Elem.u8Listtype.enType = (midw_fi_tcl_e8_Tun_ListType::tenType)u8ListType;
   oMsgData.Sel_List_Elem.u8ListElementID = u8ListElement;
   oMsgData.Sel_List_Elem.e8ConfigListID.enType = (midw_fi_tcl_e8_Tun_ConfigListID::tenType)u8ListID;

   bSendCCAMessage( MIDW_TUNERFI_C_U16_FID_TUN_S_SEL_LIST_ELEM,
      AMT_C_U8_CCAMSG_OPCODE_METHODSTART,
      oMsgData
      );

   return ;

}

#else

/*************************************************************************
*  FUNCTION: clTuner_CCAhandler::vHandleFID_S_SEL_LIST_ELEM_To_FCTuner()
*
* DESCRIPTION: handler to form a CCA Message with embedding required data
*              for FID= FID_TUN_S_SAVE_STATIC_LIST_ELEM and send to Server
*
* PARAMETER:  tU8 u8ListType , u8ListElement ( to embed in the sending CCA message)
*
* RETURNVALUE: NONE
*
* History:
* InitialVersion
*
*************************************************************************/
tVoid  clTuner_CCAhandler::vHandleFID_S_SEL_LIST_ELEM_To_FCTuner(tU8 u8ListType,
                                                                 tU8 u8ListElement)
{

   if ( NULL !=  m_poTrace)
   {
      m_poTrace->vTrace(  TR_LEVEL_HMI_INFO,
         TR_CLASS_HSI_TUNER_MSG,
         (tU16)TUN_FID_SEL_LIST_ELEM_TO_SERV,
         ONE_ELEMENT,
         (tU8*)&u8ListElement);
   }

   /* create a CCA Msg object oMsgData */
   midw_tunerfi_tclMsgFID_TUN_S_SEL_LIST_ELEMMethodStart oMsgData;
   oMsgData.Sel_List_Elem.u8Listtype.enType = (midw_fi_tcl_e8_Tun_ListType::tenType)u8ListType;
   oMsgData.Sel_List_Elem.u8ListElementID = u8ListElement;

   bSendCCAMessage( MIDW_TUNERFI_C_U16_FID_TUN_S_SEL_LIST_ELEM,
      AMT_C_U8_CCAMSG_OPCODE_METHODSTART,
      oMsgData
      );

   return ;

}
#endif

/*************************************************************************
*  FUNCTION: clTuner_CCAhandler::vHandleFID_S_GET_CONFIG_LIST_To_FCTuner()
*
* DESCRIPTION: handler to form a CCA Message with embedding required data
*              for FID= FID_TUN_S_GET_CONFIG_LIST_ELEM and send to Server
*
* PARAMETER:   tU8 u8FirstElement            -  the first element that will be retrieved
*              tU8 u8NumOfElementsInList     -  the count of elements to be retrieved
*
* RETURNVALUE: NONE
*
* History:
* InitialVersion
*
*************************************************************************/
tVoid  clTuner_CCAhandler::vHandleFID_S_GET_CONFIG_LIST_To_FCTuner(tU8 u8FirstElement ,tU8 u8NumOfElementsInList)
{
   if ( NULL !=  m_poTrace)
   {
      tU8 uzBuffer[2] = { u8FirstElement,u8NumOfElementsInList};
      m_poTrace->vTrace(   TR_LEVEL_HMI_INFO,
         TR_CLASS_HSI_TUNER_MSG,
         (tU16)TUN_GET_CONFIG_LIST_TO_FC_TUNER,
         TWO_ELEMENT,
         uzBuffer
         );
   }

   /* create a CCA Msg object oMsgData */
   midw_tunerfi_tclMsgFID_TUN_S_GET_CONFIG_LISTMethodStart oMsgData;
   oMsgData.TunGetConfigList.u8FirstElement = u8FirstElement;
   oMsgData.TunGetConfigList.u8NumOfElementsInList = u8NumOfElementsInList;

   bSendCCAMessage( MIDW_TUNERFI_C_U16_FID_TUN_S_GET_CONFIG_LIST,
      AMT_C_U8_CCAMSG_OPCODE_METHODSTART,
      oMsgData
      );
}

#ifdef VARIANT_S_FTR_ENABLE_HMI_TUNER_ITG5
/*************************************************************************
*  FUNCTION:   clTuner_CCAhandler::vHandleFID_S_MAKE_CONFIG_LIST_To_FCTuner()
*
* DESCRIPTION: handler to form a CCA Message with embedding required data
*              for FID= FID_TUN_S_MAKE_CONFIG_LIST_ELEM and send to Server
*
* PARAMETER:   tU8 u8Command     -
*              tU8 u8ListType    -
*              tU8 u8Data        -
*
* RETURNVALUE: NONE
*
* History:
* InitialVersion
* 
*************************************************************************/ 
tVoid  clTuner_CCAhandler::vHandleFID_S_MAKE_CONFIG_LIST_To_FCTuner(tU8 u8Command ,tU8 u8ListType, tU8 u8Data)
{
   u8ListType  =  u8ListType; // HDE: only for Lint !!!
   if ( NULL !=  m_poTrace)
   {
      m_poTrace->vTrace(   TR_LEVEL_HMI_INFO,
         TR_CLASS_HSI_TUNER_MSG, 
         (tU16)CONFIG_LIST_TO_FC_TUNER,
         ONE_ELEMENT,
         &u8Command
         );
   }

   if ( TUN_LIST_TYPE_TMC == u8ListType )
   {
      /** Make the Datamember to true here */
      m_TMCListRequested = TRUE;

      /* variable to used to set the TMC list length to '0' */
      tU8 u8NoOfElements = 0;
      /** Send  to CM TesTMode to release the Memory and set the datapool element DPTUNER__TMC_ELEM_NUM to 0*/
      if ( NULL != pclHSI_GetTestMode())
      {
         sMsg refMsg(NULL, (tU32)CMMSG_TESTMODE_TMCLIST_LENGTH, (tU32)u8NoOfElements, NULL);
         pclHSI_GetTestMode()->bExecuteMessage( refMsg );
      }

   }

   switch( u8Command )
   {
   case LOAD_LIST:
      {
    	  tBool bHDSetting = FALSE;
    	  /* create a CCA Msg object oMsgData */
    	  midw_tunerfi_tclMsgFID_TUN_S_MAKE_CONFIG_LISTMethodStart oMsgData;
    	  oMsgData.TunMakeConfigList.u8ConfigFilters.enType = (midw_fi_tcl_e8_Tun_Config_Filters::tenType)u8Data;

    	  if(TUN_LIST_TYPE_FM == u8ListType )
    	  {
    	      m_FMStationListRequested = TRUE;
    	      m_U8NoOfFMStations = 0;

              bHDSetting = DataPool.bGetValue( DPTUNER__HD_STATUS_FM);
              oMsgData.TunMakeConfigList.e8ConfigListID.enType = midw_fi_tcl_e8_Tun_ConfigListID::FI_EN_TUN_TUN_CONFIG_LIST_FM;
    	  }
    	  else if(TUN_LIST_TYPE_AM == u8ListType)
    	  {
    	      m_bAMStationListRequested = TRUE;
    	      m_u8NoOfAMStations = 0;

    		  bHDSetting = DataPool.bGetValue( DPTUNER__HD_STATUS_AM);
    		  oMsgData.TunMakeConfigList.e8ConfigListID.enType = midw_fi_tcl_e8_Tun_ConfigListID::FI_EN_TUN_TUN_CONFIG_LIST_MW;
    	  }

          if(bHDSetting == TRUE)
          {
        	 oMsgData.TunMakeConfigList.e8DigitalFilters.enType = midw_fi_tcl_e8_Tun_Digital_Filters::FI_EN_TUN_CONFIG_DIGITAL_HD_RADIO_WITH_SPS;
          }

          bSendCCAMessage( MIDW_TUNERFI_C_U16_FID_TUN_S_MAKE_CONFIG_LIST,
            AMT_C_U8_CCAMSG_OPCODE_METHODSTART,
            oMsgData
            );

      }
      break;

   default:
      {
         vTrace(CCA_RADIO_S_MAKE_CONFIG_LIST_ERROR,TR_LEVEL_HMI_WARNING,(tU32)u8Command);
      }
      break;
   }


}
#else
/*************************************************************************
*  FUNCTION:   clTuner_CCAhandler::vHandleFID_S_MAKE_CONFIG_LIST_To_FCTuner()
*
* DESCRIPTION: handler to form a CCA Message with embedding required data
*              for FID= FID_TUN_S_MAKE_CONFIG_LIST_ELEM and send to Server
*
* PARAMETER:   tU8 u8Command     -
*              tU8 u8ListType    -
*              tU8 u8Data        -
*
* RETURNVALUE: NONE
*
* History:
* InitialVersion
* 
*************************************************************************/ 
tVoid  clTuner_CCAhandler::vHandleFID_S_MAKE_CONFIG_LIST_To_FCTuner(tU8 u8Command ,tU8 u8ListType, tU8 u8Data)
{
   u8ListType  =  u8ListType; // HDE: only for Lint !!!
   if ( NULL !=  m_poTrace)
   {
      m_poTrace->vTrace(   TR_LEVEL_HMI_INFO,
         TR_CLASS_HSI_TUNER_MSG, 
         (tU16)CONFIG_LIST_TO_FC_TUNER,
         ONE_ELEMENT,
         &u8Command
         );
   }

   if ( TUN_LIST_TYPE_TMC == u8ListType )
   {
      /** Make the Datamember to true here */
      m_TMCListRequested = TRUE;

      /* variable to used to set the TMC list length to '0' */
      tU8 u8NoOfElements = 0;
      /** Send  to CM TesTMode to release the Memory and set the datapool element DPTUNER__TMC_ELEM_NUM to 0*/
      if ( NULL != pclHSI_GetTestMode())
      {
         sMsg refMsg(NULL, (tU32)CMMSG_TESTMODE_TMCLIST_LENGTH, (tU32)u8NoOfElements, NULL);
         pclHSI_GetTestMode()->bExecuteMessage( refMsg );
      }

   }

   if(TUN_LIST_TYPE_FM == u8ListType )
   {
      m_FMStationListRequested = TRUE;
      m_U8NoOfFMStations = 0;
   }

   switch( u8Command )
   {
   case LOAD_LIST:
      {
         /* create a CCA Msg object oMsgData */
         midw_tunerfi_tclMsgFID_TUN_S_MAKE_CONFIG_LISTMethodStart oMsgData;
         oMsgData.TunMakeConfigList.u8ConfigFilters.enType = (midw_fi_tcl_e8_Tun_Config_Filters::tenType)u8Data;

         bSendCCAMessage( MIDW_TUNERFI_C_U16_FID_TUN_S_MAKE_CONFIG_LIST,
            AMT_C_U8_CCAMSG_OPCODE_METHODSTART,
            oMsgData
            );
      }
      break;

   default:
      {
         vTrace(CCA_RADIO_S_MAKE_CONFIG_LIST_ERROR,TR_LEVEL_HMI_WARNING,(tU32)u8Command);
      }
      break;
   }


}
#endif



/*************************************************************************
*  FUNCTION: clTuner_CCAhandler::vHandleFID_TUN_S_SET_PI_To_FCTuner()
*
* DESCRIPTION: handler to form a CCA Message with embedding required data
*              for FID= FID_TUN_S_SET_PI and send to Server
*
* PARAMETER:  tU8 u8AbsOrRel, u8RelSteps ,u8Direction( to embed in the sending CCA message)
*
* RETURNVALUE: NONE
*
* History:
* InitialVersion
* 
*************************************************************************/ 
tVoid clTuner_CCAhandler::vHandleFID_S_SET_PI_To_FCTuner(midw_fi_tcl_e8_Tun_Abs_Or_Rel::tenType u8AbsOrRel ,
                                                         midw_fi_tcl_e8_Tun_Rel_Steps::tenType u8RelSteps)
{

   /* create a CCA Msg object oMsgData */
   midw_tunerfi_tclMsgFID_TUN_S_SET_PIMethodStart oMsgData;
   oMsgData.TunSetPIPara.u8AbsOrRel.enType = u8AbsOrRel;
   oMsgData.TunSetPIPara.u8RelSteps.enType = u8RelSteps;


   bSendCCAMessage( MIDW_TUNERFI_C_U16_FID_TUN_S_SET_PI, 
      AMT_C_U8_CCAMSG_OPCODE_METHODSTART, 
      oMsgData
      );

   return;
}

/*************************************************************************
*  FUNCTION: clTuner_CCAhandler::vHandleFID_S_SET_SHARXLEVEL_To_FCTuner()
*
* DESCRIPTION: handler to form a CCA Message with embedding required data
*              for FID= FID_TUN_S_SET_SHARXLEVEL and send to Server
*
* PARAMETER:  tU8 u8SharxStep( to embed in the sending CCA message)
*
* RETURNVALUE: NONE
*
* History:
* InitialVersion
* 
*************************************************************************/ 
tVoid clTuner_CCAhandler::vHandleFID_S_SET_SHARXLEVEL_To_FCTuner(tU8 u8SharxStep)
{

   /* create a CCA Msg object oMsgData */
   midw_tunerfi_tclMsgFID_TUN_S_SET_SHARXLEVELMethodStart oMsgData;
   oMsgData.u8SharxStep = u8SharxStep;

   bSendCCAMessage( MIDW_TUNERFI_C_U16_FID_TUN_S_SET_SHARXLEVEL, 
      AMT_C_U8_CCAMSG_OPCODE_METHODSTART, 
      oMsgData
      );   

   return;
}



/*************************************************************************
*  FUNCTION:    clTuner_CCAhandler::vHandleALL_FID_MethodResult_Handler_From_FCTuner() 
*
* DESCRIPTION:  common handler for handling the Method Result Messages from server  
*
* PARAMETER: amt_tclServiceData* poMessage    
*
* RETURNVALUE: NONE
*
* History:
* InitialVersion
* 
*************************************************************************/
tVoid clTuner_CCAhandler::vHandleALL_FID_MethodResult_Handler_From_FCTuner(
   amt_tclServiceData* poMessage) 
{ 

   /* check the parameter for not NULL and then for validity of a messsage */
   if ( (poMessage != NULL) && (poMessage->bIsValid()) )
   {

      switch (poMessage->u8GetOpCode())   //extract the opcode from Message
      {

      case AMT_C_U8_CCAMSG_OPCODE_METHODRESULT: 
         {
            tU16 u16Fid = poMessage->u16GetFunctionID(); //extract the FID from Message

            if ( m_poTrace != NULL )
            {
               m_poTrace->vTrace(  TR_LEVEL_HMI_INFO,
                  TR_CLASS_HSI_TUNER_MSG, 
                  (tU16)TUN_COMMON_METHOD_RESULT_HANDLER ,
                  ONE_ELEMENT,
                  &u16Fid );
            }

            switch( u16Fid ) /** Common action for all cases , may need to change later **/
            {
            case MIDW_TUNERFI_C_U16_FID_TUN_S_SET_STATIC_LIST : 

            case MIDW_TUNERFI_C_U16_FID_TUN_S_SET_FEATURE:

            case MIDW_TUNERFI_C_U16_FID_TUN_G_SET_HICUT:

            case MIDW_TUNERFI_C_U16_FID_TUN_S_AFLIST_V2:

            case MIDW_TUNERFI_C_U16_FID_TUN_S_STOP_ANNOUNCEMENT:

            case MIDW_TUNERFI_C_U16_FID_TUN_S_OPERATION:

            case MIDW_TUNERFI_C_U16_FID_TUN_S_DIRECT_FREQUENCY:

            case MIDW_TUNERFI_C_U16_FID_TUN_S_SEL_LIST_ELEM:

            case MIDW_TUNERFI_C_U16_FID_TUN_S_AUTOSTORE_STATIC_LIST:

            case MIDW_TUNERFI_C_U16_FID_TUN_S_SETUP_DDADDS:

            case MIDW_TUNERFI_C_U16_FID_TUN_S_SET_PI:

#ifdef VARIANT_S_FTR_ENABLE_HMI_TUNER_ITG5
            case MIDW_TUNERFI_C_U16_FID_TUN_S_SET_DEMODULATION_SWITCH:
			
			case MIDW_TUNERFI_C_U16_FID_TUN_S_SELECT_HD_AUDIOPRGM:
#endif

               {
                  tU8 u8CmdVR = 0;

                  gm_tclU16Message oStatusMessage(poMessage); // handle common functionality here

                  tU16 u16Methodresult = oStatusMessage.u16GetWord(); 

                  /** LSB Tuner status is extracted first */
                  tU8 u8TunerStatus = (tU8)(u16Methodresult & 0x00FF);

                  /** MSB Tuner State is rightshifted and extracted as a byte*/                    
                  tU8 u8TunerState  = (tU8)(u16Methodresult >> 8);

                  /** To remove compiler warning */
                  u8TunerState = u8TunerState;

                  if  ( (tU8)midw_fi_tcl_e8_Tun_ResponseStatus::FI_EN_TUN_RES_REQ_FAIL == u8TunerStatus)
                  {
                     if(poFI != NULL)
                     {
                        if(clHSIDeviceGeneral::m_poDataPool != NULL)
                        {
                           clHSIDeviceGeneral::m_poDataPool -> u32dp_get
                              ( DPVC__INPUT_CMD , 
                              &u8CmdVR , sizeof(u8CmdVR) );

                           if(VR_TUN_SELECT_STATION == u8CmdVR ||
                              VR_TUN_SELECT_BAND_MEM_BANK == u8CmdVR ||
                              VR_TUN_AUTOSTORE == u8CmdVR ||
                              VR_TUN_SET_TUNER_SETTING == u8CmdVR ||
                              VR_TUN_STORE_STATION == u8CmdVR)
                           {
                              poFI->WriteDataToDp
                                 (TUNER_HSI_ERROR_ACTION_FAILED, 
                                 HSI_ERROR_ACTION_FAILED );

                           }
                        }
                     }
                  }


                  break;
               }
            default :
               break;
            } //end of switch  (u16Fid)

            break;

         }

      case AMT_C_U8_CCAMSG_OPCODE_ERROR:
         {            
            /* handle error message in a separate function */
            vHandleCCAErrorMessage( poMessage );

            break;
         } 

      default : // unknown opcode
         {
            /* send error message back to server for unkown opcode */
            vSendErrorMessage(
               poMessage->u16GetFunctionID(),            
               AMT_C_U16_ERROR_INVALID_OPCODE
               );
         }

      } // end of switch
   }
}



/*************************************************************************
*  FUNCTION:    clTuner_CCAhandler::bRegisterFIDForSimulation 
*
* DESCRIPTION: Handles the framing of CCA Upreg/Relupreg Msg for Simulation 
*               & enPost's the Msg using ( using Socket interface)   
*
* PARAMETER:   tU16 u16FID
*
* RETURNVALUE: TRUE incase of Success Else FALSE
*
* History:
* InitialVersion
* 
*************************************************************************/

tBool clTuner_CCAhandler::bFIDRegUnregForSimulation( tU16 u16FID , tU8 u8Opcode) const
{
   /** Removing lint */

   u16FID = u16FID;

   u8Opcode = u8Opcode;

#ifdef HMI_SIM_WITH_TARGET_Test 

   /* for simulation purpose ,which uses socket interface */

   hmicca_tclApp* poCCAOutput = NULL;

   if(poFI != NULL )
   {
      poCCAOutput = (hmicca_tclApp*)poFI->pvGetCCAMain(); /* Get the pointer of the hmicca-application */

      if( NULL == poCCAOutput)
      {
         return FALSE;         /* As the poCCAOutput is NULL ,no need to Proceed */
      }

   }
   else 
   {
      return FALSE;             /* As the poFI is NULL , poCCAOutput is not valid ,no need to Proceed */
   }

   /* form a CCA message with all details required for upreg with Tuner server , 
   place the First parameter occrdingly to SIMulation  */ 

   gm_tclEmptyMessage oUpRegMessage
      (
      poCCAOutput->poSocketClient->u16ClientAppId,  /*  AppID of this application-HMI socket inetrface  */
      CCA_C_U16_APP_TUNER,             /*  AppID of Tuner */
      _u16RegID,                       /*  RegId for the service */
      0,                               /*  always 0 (adopted from demo program)  */
      CCA_C_U16_SRV_TUNERCONTROL ,     /*  the SID of the service  */
      u16FID,                          /*  the FID to register for   */
      u8Opcode                         /*  OPCODE for Upreg /Relupreg -message ---*/
      );


   tU32 u32MsgQueueTuner_local = poCCAOutput ->u32MsgQueueTuner;    /* get the Tuner MSG Queue NO */

   poCCAOutput -> poSocketClient->poOSALIf->s32MsgQueuePost(        /* Posting the MSG through Osal  & socket interface */    
      u32MsgQueueTuner_local,
      sizeof(oUpRegMessage),
      (tU8*)oUpRegMessage.pu8GetSharedMemBase (),
      2 );

   if (m_poTrace != NULL)
   {
      m_poTrace->vTrace(  TR_LEVEL_HMI_INFO, 
         TR_CLASS_HSI_TUNER_MSG,
         (tU16)TUN_SIM_ENPOST_MSG );
   }

#endif

   return TRUE;
}


/*************************************************************************
* FUNCTION:    clTuner_CCAhandler::vHandleFID_G_TAINFO_From_FCTuner
*
* DESCRIPTION: separate handler to extract data from the status message , 
*              this is a internal function used by vHandleFID_G_ATSTATIONINFO_VAG_From_FCTuner
*
* PARAMETER:   amt_tclServiceData* poMessage ( Service data message to extract )
*
* RETURNVALUE: NONE
*
* History:
* InitialVersion
* 
*************************************************************************/
tVoid clTuner_CCAhandler::vHandleFID_G_TAINFO_From_FCTuner( amt_tclServiceData* poMessage )
{
	/* check the parameter for not NULL and then for validity of a messsage */
   if ( (poMessage != NULL) && (poMessage->bIsValid()) )
   {

      switch (poMessage->u8GetOpCode())  // get the opcode from the message
      {
      case AMT_C_U8_CCAMSG_OPCODE_STATUS:
         {
            vStartInitSequence(MIDW_TUNERFI_C_U16_FID_TUN_G_TAINFO);
			ETG_TRACE_USR4(("clTuner_CCAhandler::vHandleFID_G_TAINFO_From_FCTuner status received"));
								  
            vHandleTAInfo( poMessage );
            break;
         }
      case AMT_C_U8_CCAMSG_OPCODE_ERROR:
               {
                  /* handle error message in a separate function */
                  vHandleCCAErrorMessage( poMessage );

                  break;
               }

            default : // unknown opcode
               {
                  /* send error message back to server for unkown opcode */
                  vSendErrorMessage(
                     poMessage->u16GetFunctionID(),
                     AMT_C_U16_ERROR_INVALID_OPCODE
                     );
                  break;
               }
        } // end of switch
    }
}



/*************************************************************************
* FUNCTION:    clTuner_CCAhandler::vHandleTAInfo
*
* DESCRIPTION: separate handler to extract data from the status message , 
*              this is a internal function used by vHandleFID_G_TAINFO_From_FcTuner
*
* PARAMETER:   amt_tclServiceData* poMessage ( Service data message to extract )
*
* RETURNVALUE: NONE
*
* History:
* InitialVersion
* 
*************************************************************************/
tVoid clTuner_CCAhandler::vHandleTAInfo( amt_tclServiceData* poMessage )
{
	midw_tunerfi_tclMsgFID_TUN_G_TAINFOStatus otunmsg_tclTAinfoStatus;
	vGetDataFromAmt( poMessage , otunmsg_tclTAinfoStatus );
	
	tChar          pcTaStationName[PSNAME_MAX_LEN_IN_UTF+1];
	tU32 u32TAFrequency = 0;
	
	/*Extract and Write TA Frequency to datapool*/
	u32TAFrequency = otunmsg_tclTAinfoStatus.TAInfostatus.u32Frequency;
	

     ETG_TRACE_USR4(("vHandleTAInfo: Frequency : %d"
     		       				  , 	u32TAFrequency));
								  
	/*Extract TA StationName and write to datapool*/
	OSALUTIL_szSaveStringNCopy(   pcTaStationName,
            otunmsg_tclTAinfoStatus.TAInfostatus.sTaStationname.szValue,
            PSNAME_MAX_LEN_IN_UTF);                // PS-Name
	pHSI_GetTunerMaster()->vSetTAStationName_Var(pcTaStationName);
	tU8 u8TAsource = 0;
	if(m_poDataPool != NULL)
	{
		m_poDataPool->u32dp_get( DPTUNERMASTER__TA_SOURCE,
			&u8TAsource,
			sizeof(u8TAsource));

	     ETG_TRACE_USR4(("vHandleTAInfo: TA source : %d"
	     		       				  , 	u8TAsource));

	  if(u8TAsource == 0x04)// FM TA
	  {
		if(u32TAFrequency == 0) // Empty data obtained
		{
			// do nothing. 
		}
		else if( strlen(pcTaStationName) == 0)
        {
          pHSI_GetRadio()->bFormatFrequenz(pcTaStationName,PSNAME_MAX_LEN_IN_UTF,u32TAFrequency);
	      pHSI_GetTunerMaster()->vSetCurrentTAStationName(pcTaStationName);
        }
        else
        {
          pHSI_GetTunerMaster()->vSetCurrentTAStationName(pcTaStationName);
        }
	  }
   }
			
}


/*************************************************************************
* FUNCTION:    clTuner_CCAhandler::vHandleAtStationInfo()
*
* DESCRIPTION: separate handler to extract data from the status message , 
*              this is a internal function used by vHandleFID_G_ATSTATIONINFO_VAG_From_FCTuner
*
* PARAMETER:   amt_tclServiceData* poMessage ( Service data message to extract )
*
* RETURNVALUE: NONE
*
* History:
* InitialVersion
* 
*************************************************************************/

tVoid clTuner_CCAhandler::vHandleAtStationInfo( amt_tclServiceData* poMessage )
{

	clHSI_Radio    *pHSI  =  pHSI_GetRadio();
	midw_tunerfi_tclMsgFID_TUN_G_ATSTATIONINFOStatus otunmsg_tclStationS;
	vGetDataFromAmt( poMessage , otunmsg_tclStationS );
	if (  ( poFI != NULL )  &&
      ( pHSI != NULL)  )
   {

      tU32           u32Frequency   = 0;
      tChar          pcPSName[PSNAME_MAX_LEN_IN_UTF+1];
      tChar          curPSName[PSNAME_MAX_LEN_IN_UTF+1];
      tU8            u8TunerHeader;
      tU8            u8Band;
      tU8            u8TPinfo;
      tU8          u8AFFrequency;
      tU16            u16PresetListID;
      tU8            u8ActiveStationList;
      tU16           u16StaticListElementID;
      tU16           u16ConfigListElementID;
      
      pcPSName[0] = 0;  // empty string initialisation
      curPSName[0] = 0;
      
      m_u32AFFrequency = 0;

	  tBool bManual = FALSE;



        tU8 u8Bank = 0;
     	if (m_poDataPool != NULL)
     	{
     		m_poDataPool -> u32dp_get(DPTUNER__CURRENT_BANK ,
     			&u8Bank,
     			sizeof(u8Bank) );
     	}


      /* extract Frequency qualification */


      u8AFFrequency  =  otunmsg_tclStationS.Atstationinfo.u8FrequencyQualification;

      u8AFFrequency = u8AFFrequency & TUN_AF_CHECK;

      u8AFFrequency = u8AFFrequency >> 3;

      ETG_TRACE_USR4(("vHandleAtStationInfo: AFFrequency : %d"
                                        		       				  , 	u8AFFrequency));

   	if (m_poDataPool != NULL)
   	{
   		m_poDataPool -> u32dp_get(DPTUNER__MANUALMODE ,
   			&bManual,
   			sizeof(bManual) );
   	}



      if((!u8AFFrequency && bManual) || !bManual)     //check for AF in tune screen or main screen
      {
    	  
    	  
        /* extract Band  */
        {

        u8Band = (tU8)otunmsg_tclStationS.Atstationinfo.e8Band.enType;

        ETG_TRACE_USR4(("vHandleAtStationInfo: Band : %d"
                    		       			, 	u8Band));

        }

         /* extract Frequency */
         {

         u32Frequency = otunmsg_tclStationS.Atstationinfo.u32Frequency;
         


         ETG_TRACE_USR4(("vHandleAtStationInfo: Frequency : %d"
         		       				  , 	u32Frequency));
									  
		 if (m_poDataPool != NULL)
         {
			 if(u8Band == TUN_BAND_FM)
			 {
				 m_poDataPool -> u32dp_set(DPTUNER__STATION_FM_FREQVAL,
									 &u32Frequency,
									 sizeof(u32Frequency) );
			 }
			 else if(u8Band == TUN_BAND_AM)
			 {
				 m_poDataPool -> u32dp_set(DPTUNER__STATION_AM_FREQVAL,
										&u32Frequency,
										sizeof(u32Frequency) );
			 }
			 else
			 {
			  // do nothing
			 }



          }

         

         }
         

         /* extract Config list element ID info */
         {

         u16ConfigListElementID = otunmsg_tclStationS.Atstationinfo.u16ConfigListElementID;

         ETG_TRACE_USR4(("vHandleAtStationInfo: ConfigListElementID : %d"
                                      		       				  , 	u16ConfigListElementID));
         pHSI_GetRadio()->vSetCurStationListIndex((tU8)(u16ConfigListElementID-1));

         }



         /* extract Preset list ID info */
         {

         u16PresetListID = otunmsg_tclStationS.Atstationinfo.u16PresetListID;

         ETG_TRACE_USR4(("vHandleAtStationInfo: PresetListID : %d"
                        		       				  , 	u16PresetListID));

         }


         /* extract PS name */

   	     tU8 u8NoFMPreset = 0;
         if(m_poDataPool != NULL)
         {

  		    m_poDataPool -> u32dp_get(DPSYSTEM__VARIANT_CONFIG_RADIO_FM_NUM_BANKS ,
  	   			   &u8NoFMPreset,
  	   			   sizeof(u8NoFMPreset) );

         }
#ifdef VARIANT_S_FTR_ENABLE_HMI_TUNER_ITG5
         tU8 u8HDInfo = otunmsg_tclStationS.Atstationinfo.u8HDStationInfoBits.u8Value;
         if((u8HDInfo & (midw_fi_tcl_b8_Tun_HDInfoBits::FI_C_U8_BIT_HD_DATA_DECODING)) == (midw_fi_tcl_b8_Tun_HDInfoBits::FI_C_U8_BIT_HD_DATA_DECODING))
         {
        	 OSALUTIL_szSaveStringNCopy(   pcPSName,
        	   		otunmsg_tclStationS.Atstationinfo.sHDCallSign.szValue,
        	   		PSNAME_MAX_LEN_IN_UTF);
        	 pHSI->vSetCurrentPSName(pcPSName);
         }
         else
         {
        	 if(u8NoFMPreset == 0x02)  //  NAR variant , NIKAI-5917
        	   		 {

        	   			 if(((u16PresetListID == 0x0f20) && (u8Bank == 0x00)) || ((u16PresetListID == 0x0f21) && (u8Bank == 0x01)))
        	   			 {
        	   			      OSALUTIL_szSaveStringNCopy(   pcPSName,
        	   			            otunmsg_tclStationS.Atstationinfo.sPSName.szValue,
        	   			            PSNAME_MAX_LEN_IN_UTF);

        	   		          pHSI->vSetCurrentPSName(pcPSName);

        	              }
        	   			 else
        	   			 {
        	   				 // do nothing
        	   			 }
        	   		 }

        	   		 else                                           //   EU variant and Mexico variant , NIKAI2-2346
        	   		 {

        	            OSALUTIL_szSaveStringNCopy(   pcPSName,
        	 				otunmsg_tclStationS.Atstationinfo.sPSName.szValue,
        	 				PSNAME_MAX_LEN_IN_UTF);                // PS-Name

        	 		  if(u8Band == TUN_BAND_FM)
        	 		  {
        	 			  if(m_poDataPool != NULL)
        	 			  {

        	 				  tU32 u32Size   = m_poDataPool->u32dp_getElementSize(DPTUNER__FOR_XML_RADIO_ACTIVE_STATION_PS);
        	 				  m_poDataPool->u32dp_get( DPTUNER__FOR_XML_RADIO_ACTIVE_STATION_PS,
        	 					   curPSName,
        	 						  u32Size);

        	 			   if((strcmp(pcPSName , curPSName )) != 0)
        	 			   {
        	 				   pHSI->bSetFMStationList_NewPS(pcPSName);

        	 				   pHSI->vCheckUpdateStationList();
        	 			   }
        	 			  }

        	 			  pHSI->vSetCurrentPSName(pcPSName);

        	 		  }

        	 		  else
        	 		  {
        	 			  // do nothing
        	 		  }

        	   		 }
         }

#else
  		 if(u8NoFMPreset == 0x02)  //  NAR variant , NIKAI-5917
  		 {

  			 if(((u16PresetListID == 0x0f20) && (u8Bank == 0x00)) || ((u16PresetListID == 0x0f21) && (u8Bank == 0x01)))
  			 {
  			      OSALUTIL_szSaveStringNCopy(   pcPSName,
  			            otunmsg_tclStationS.Atstationinfo.sPSName.szValue,
  			            PSNAME_MAX_LEN_IN_UTF);

  		          pHSI->vSetCurrentPSName(pcPSName);

             }
  			 else
  			 {
  				 // do nothing
  			 }
  		 }

  		 else                                           //   EU variant and Mexico variant , NIKAI2-2346
  		 {

           OSALUTIL_szSaveStringNCopy(   pcPSName,
				otunmsg_tclStationS.Atstationinfo.sPSName.szValue,
				PSNAME_MAX_LEN_IN_UTF);                // PS-Name

		  if(u8Band == TUN_BAND_FM)
		  {
			  if(m_poDataPool != NULL)
			  {

				  tU32 u32Size   = m_poDataPool->u32dp_getElementSize(DPTUNER__FOR_XML_RADIO_ACTIVE_STATION_PS);
				  m_poDataPool->u32dp_get( DPTUNER__FOR_XML_RADIO_ACTIVE_STATION_PS,
					   curPSName,
						  u32Size);

			   if((strcmp(pcPSName , curPSName )) != 0)
			   {
				   pHSI->bSetFMStationList_NewPS(pcPSName);

				   pHSI->vCheckUpdateStationList();
			   }
			  }

			  pHSI->vSetCurrentPSName(pcPSName);

		  }

		  else
		  {
			  // do nothing
		  }

  		 }
#endif


  		/* Map screen update */

  		if(u8Band == TUN_BAND_FM)
  		{

	          if(strlen(pcPSName) != 0)
	          {
	             poFI->WriteStringDataToDp(STATION_DATA_STATIONNAME, (tChar*)pcPSName);

	          }
	          else
	          {
	              tUTF8 utf8FRQString[FRQString_Max_length];
	              pHSI->bFormatFrequenz(utf8FRQString,FRQString_Max_length,u32Frequency);   // return not needed
	              OSALUTIL_szSaveStringNCopy(   pcPSName,
	                 utf8FRQString,
	                 PSNAME_MAX_LEN_IN_UTF); // Show frequencz
	              poFI->WriteStringDataToDp(STATION_DATA_STATIONNAME, (tChar*)pcPSName);
	          }

  		}
  		else
  		{
  			// do nothing
  		}

      


      /*  extract Header  */
      {

      u8TunerHeader = (tU8)otunmsg_tclStationS.Atstationinfo.u8TunerHeader.enType;

      }





     /* extract TP info */
     {

     u8TPinfo = (tU8)otunmsg_tclStationS.Atstationinfo.e8AtStationTPinfo.enType;
	 
	//tBool bFlag = (tBool)u8TPinfo;
                /*m_poDataPool -> u32dp_set(DPTUNER__FEATURE_TA,
                                          &bFlag,
                                          sizeof(bFlag) );*/

     ETG_TRACE_USR4(("vHandleAtStationInfo: TP : %d"
               		       				  , 	u8TPinfo));
	
	

     }



     /* extract Active Station List info */
     {

     u8ActiveStationList = otunmsg_tclStationS.Atstationinfo.u8ActiveStationList;

     ETG_TRACE_USR4(("vHandleAtStationInfo: ActiveStationList : %d"
                        		       				  , 	u8ActiveStationList));

     }

     /* extract Static list element ID info */
     {

         tU8 u8NoFMPresetBank = 0;

         u16StaticListElementID = otunmsg_tclStationS.Atstationinfo.u16StaticListElementID;

         ETG_TRACE_USR4(("vHandleAtStationInfo: StaticListElementID : %d"
                                 		       				  , 	u16StaticListElementID));
         if(m_poDataPool != NULL)
         {

  		    m_poDataPool -> u32dp_get(DPSYSTEM__VARIANT_CONFIG_RADIO_FM_NUM_BANKS ,
   			   &u8NoFMPresetBank,
   			   sizeof(u8NoFMPresetBank) );
         }

         if(u8NoFMPresetBank == 0x01)
         {
    														  
    		if((u16PresetListID == TUN_FM_EU_PRESET) && (u8Band == 0x00)) // FM
    		{
    			 pHSI_GetTunerMaster()->vSetActiveStationPresetNr((tS8)u16StaticListElementID,u8Band);
    		}
    		else if((u16PresetListID == TUN_AM_EU_PRESET) && (u8Band == 0x02))// AM
    		{
      			if(u16StaticListElementID ==0)
      			{
      				pHSI_GetTunerMaster()->vSetActiveStationPresetNr((tS8)u16StaticListElementID,u8Band);
      			}
      			else
      			{
            	 pHSI_GetTunerMaster()->vSetActiveStationPresetNr((tS8)(u16StaticListElementID+12),u8Band);
      			}
    		}
    		else
    		{
    			// do nothing
    		}
         }
        	
         else if(u8NoFMPresetBank == 0x02)
         {
        	


        	    tU16 u16newpreset = 0; // send presetlistsetactive request if presetlist ID dosn't match
        	    tU8  u8new_Band=0;
                if(u8Bank == 0x00)
                {
                	u16newpreset = 0x0f20;
                	u8new_Band = 0x00;
                }
                else if(u8Bank == 0x01)
                {
                	u16newpreset = 0x0f21;
                	u8new_Band = 0x00;
                }
                else
                {
                	u16newpreset = 0x0f22;
                	u8new_Band = 0x02;
                }


                if(u8new_Band != u8Band)
                {
                	// don't send
                }
                else
				{
					ETG_TRACE_USR4(("vHandleAtStationInfo: u16newpreset : %x , u16PresetListID : %x",u16newpreset,u16PresetListID));

					if((u16newpreset != u16PresetListID) && (bcheckpresetlist == true))
					{
						vHandleFID_S_SET_MIXEDPRESET_LIST(u8Bank);
						bcheckpresetlist = false;
					}
					else
					{

						if(u16newpreset == u16PresetListID)
						{
							bcheckpresetlist = true;
							if((u16PresetListID == TUN_FM1_PRESET) && (u8Band == 0x00)) // FM1
							{
								pHSI_GetTunerMaster()->vSetActiveStationPresetNr((tS8)u16StaticListElementID,u8Band);
							}
							else if((u16PresetListID == TUN_FM2_PRESET) && (u8Band == 0x00))//FM2
							{
								if(u16StaticListElementID ==0)
								{
									pHSI_GetTunerMaster()->vSetActiveStationPresetNr((tS8)u16StaticListElementID,u8Band);
								}
								else
								{
								pHSI_GetTunerMaster()->vSetActiveStationPresetNr((tS8)(u16StaticListElementID+6),u8Band);
								}
							}
							else if((u16PresetListID == TUN_AM_NAR_PRESET)  && (u8Band == 0x02))// AM
							{
								if(u16StaticListElementID ==0)
								{
									pHSI_GetTunerMaster()->vSetActiveStationPresetNr((tS8)u16StaticListElementID,u8Band);
								}
								else
								{
								 pHSI_GetTunerMaster()->vSetActiveStationPresetNr((tS8)(u16StaticListElementID+12),u8Band);
								}
							}
							else
							{
								// do nothing
							}
						}

					}
                }

        	 
          }

         else
         {
        	 // do nothing
         }


     }

   }
      else   // if frequency is AF and in tune screen
      {
    	  
    	  m_u32AFFrequency = otunmsg_tclStationS.Atstationinfo.u32Frequency; 
    	  
    	  ETG_TRACE_USR4(("vHandleAtStationInfo: AF in manual mode : %d",m_u32AFFrequency));
    	  
          OSALUTIL_szSaveStringNCopy(   m_pcPSName,
                otunmsg_tclStationS.Atstationinfo.sPSName.szValue,
                PSNAME_MAX_LEN_IN_UTF);                // PS-Name
          tChar pcAFPSname[PSNAME_MAX_LEN_IN_UTF+1];
          
          strcpy(pcAFPSname,m_pcPSName);
          
          if(strlen(pcAFPSname) != 0)  // Map screen display
          {
             poFI->WriteStringDataToDp(STATION_DATA_STATIONNAME, (tChar*)pcAFPSname);

          }
          else
          {
              tUTF8 utf8FRQString[FRQString_Max_length];
              pHSI->bFormatFrequenz(utf8FRQString,FRQString_Max_length,m_u32AFFrequency);   // return not needed
              OSALUTIL_szSaveStringNCopy(   pcAFPSname,
                 utf8FRQString,
                 PSNAME_MAX_LEN_IN_UTF); // Show frequencz
              poFI->WriteStringDataToDp(STATION_DATA_STATIONNAME, (tChar*)pcAFPSname);
          }
          
          
      }
#ifdef VARIANT_S_FTR_ENABLE_HMI_TUNER_ITG5
      /****Extract HD information here****/
      //HD Station info bits
      tU8 u8HDStationInfoBits = otunmsg_tclStationS.Atstationinfo.u8HDStationInfoBits.u8Value;
      DataPool.vSetU8Value(DPTUNER__STATION_HD_INFO_BITS, u8HDStationInfoBits);

      //Active HD program number
      tU8 u8ActiveHDAudPrgmNo = otunmsg_tclStationS.Atstationinfo.u8ActiveHDAudPrgm;
      DataPool.vSetU8Value(DPTUNER__STATION_ACTIVE_HD_AUD_PRGM, u8ActiveHDAudPrgmNo);

      //Available HD Audio programs
      tU8 u8AvailHDAudPrgmBits = otunmsg_tclStationS.Atstationinfo.u8HDAudioPrgmAvail.u8Value;
      DataPool.vSetU8Value(DPTUNER__STATION_AVAIL_HD_AUDPRGMS, u8AvailHDAudPrgmBits);
#endif
   
 }
}


/*************************************************************************
* FUNCTION:    clTuner_CCAhandler::vHandleBand_Station_info()
*
* DESCRIPTION: separate handler to extract data from the status message ,
*              this is a internal function used by vHandleFID_G_BAND_PLUS_STATION_INFO_From_FCTuner
*
* PARAMETER:   amt_tclServiceData* poMessage ( Service data message to extract )
*
* RETURNVALUE: NONE
*
* History:
* InitialVersion
*
*************************************************************************/

tVoid clTuner_CCAhandler::vHandleBand_Station_info( amt_tclServiceData* poMessage )
{

	clHSI_Radio    *pHSI  =  pHSI_GetRadio();
	midw_tunerfi_tclMsgFID_TUN_G_BAND_PLUS_STATION_INFOStatus otunmsg_tclStationS;
	vGetDataFromAmt( poMessage , otunmsg_tclStationS );
	if (  ( poFI != NULL )  &&
      ( pHSI != NULL)  )
   {

      tU32           u32Frequency   = 0;
      tChar          pcPSName[PSNAME_MAX_LEN_IN_UTF+1];
      tU8            u8Band;

      tU16           u16StaticListElementID;

      pcPSName[0] = 0;  // empty string initialisation



      /* extract Band  */
           {

           u8Band = (tU8)otunmsg_tclStationS.TunerBandAndStationInfo.e8Band.enType;

           ETG_TRACE_USR4(("vHandleAtStationInfo: Band : %d"
                		       				  , 	u8Band));



           }

     /* extract Frequency */
     {

     u32Frequency = otunmsg_tclStationS.TunerBandAndStationInfo.u32Frequency;

     ETG_TRACE_USR4(("vHandleAtStationInfo: Frequency : %d"
     		       				  , 	u32Frequency));

       if(m_poDataPool != NULL)
       {
    	 if(u8Band == 0x00)
    	 {
    		 m_poDataPool->u32dp_set( DPTUNER__STATION_FM_FREQVAL,
    		                            &u32Frequency , 
    		                            sizeof(u32Frequency) );
    	 }
    	 else if(u8Band == 0x02)
    	 {
       		 m_poDataPool->u32dp_set( DPTUNER__STATION_AM_FREQVAL,
        		                            &u32Frequency , 
        		                            sizeof(u32Frequency) );
    	 }
    	 else
    	 {
    		 // do nothing
    	 }
       } 
     }


      /* extract PS name */

      OSALUTIL_szSaveStringNCopy(   pcPSName,
            otunmsg_tclStationS.TunerBandAndStationInfo.sPSName.szValue,
            PSNAME_MAX_LEN_IN_UTF);                // PS-Name

      if(u8Band == TUN_BAND_FM)
      {

          pHSI->vSetCurrentPSName(pcPSName);

      }
      else
      {
    	  // do nothing
      }



    /* extract Static list element ID info */
     {

         tU8 u8Bank = 0;
         tU8 u8Area = 0;

         u16StaticListElementID = otunmsg_tclStationS.TunerBandAndStationInfo.u16StaticListElementID;

         ETG_TRACE_USR4(("vHandleAtStationInfo: StaticListElementID : %d"
                                 		       				  , 	u16StaticListElementID));

     	if (m_poDataPool != NULL)
     	{
     		m_poDataPool -> u32dp_get(DPTUNER__CURRENT_BANK ,
     			&u8Bank,
     			sizeof(u8Bank) );
     	}

         if(u8Bank == TUN_BANK_FM1)
         {

    		pHSI_GetTunerMaster()->vSetActiveStationPresetNr((tS8)u16StaticListElementID,u8Band);

         }
         else if(u8Bank == TUN_BANK_FM2)
         {

        	 if(m_poDataPool != NULL)
        	 {
         		m_poDataPool -> u32dp_get(DPSYSTEM__VARIANT_CONFIG_RADIO_AREA ,
          			&u8Area,
          			sizeof(u8Area) );

         		if(u8Area == TUN_AREA_EU)
         		{
         			pHSI_GetTunerMaster()->vSetActiveStationPresetNr((tS8)u16StaticListElementID,u8Band);
         		}
         		else if(u8Area == TUN_AREA_NAR)
         		{
         			pHSI_GetTunerMaster()->vSetActiveStationPresetNr((tS8)(u16StaticListElementID+6),u8Band);
         		}
         		else
         		{
         			// do nothing
         		}
        	 }


         }
         else if(u8Bank == TUN_BANK_AM)
         {
        	 pHSI_GetTunerMaster()->vSetActiveStationPresetNr((tS8)(u16StaticListElementID+12),u8Band);
         }
         else
         {
        	 // do nothing
         }

     }






   }
}


/*************************************************************************
* FUNCTION:    clTuner_CCAhandler::vHandleTestModeDataStatus()
*
* DESCRIPTION: separate handler to extract data from the status message ,
*              this is a internal function used by vHandleFID_G_TESTMODE_DATA_From_FCTuner
*
* PARAMETER:   amt_tclServiceData* poMessage ( Service data message to extract )
*
* RETURNVALUE: NONE
*
* History:
* InitialVersion
*
*************************************************************************/

tVoid clTuner_CCAhandler::vHandleTestModeDataStatus( amt_tclServiceData * poMessage )
{

   /* object for handling the status data of type tunmsg_tclTestModeDataS ( server defined) */

   midw_tunerfi_tclMsgFID_TUN_G_GET_DATA_TESTMODEStatus  otunmsg_tclTestModeDataS;

   vGetDataFromAmt( poMessage , otunmsg_tclTestModeDataS );

   if ( poFI != NULL )
   {
      // extract Header

      tU8 u8Header = (tU8)otunmsg_tclTestModeDataS.TunerTestModeData.u8TunerHeader.enType;
      u8TunMode = u8Header;


      //extract Frequency
      {
         tU32 u32Frequency = otunmsg_tclTestModeDataS.TunerTestModeData.u32AFFrequency;

         poFI->WriteTestModeDataToDP(TM_DATA_FREQUENCY, u32Frequency);

      }

      // Added By rrv2kor for extracting HUB info
      {
         tU8 u8HubValue = otunmsg_tclTestModeDataS.TunerTestModeData.u8Hub;

         poFI->WriteTestModeDataToDP(TM_DATA_HUBVALUE, u8HubValue);

      }



      //extract signal strength
      {
         tU8 u8Fieldstrength = otunmsg_tclTestModeDataS.TunerTestModeData.u8Fieldstrength;

         poFI->WriteTestModeDataToDP(TM_DATA_FIELDSTRENGTH, u8Fieldstrength);

      }

      //extract Multipath value
      {
         tU8 u8Multipath = otunmsg_tclTestModeDataS.TunerTestModeData.u8Multipath;

         poFI->WriteTestModeDataToDP(TM_DATA_MULTIPATH, u8Multipath);

      }

      //extract Neighbour value
      {
         tU8 u8Neighbour = otunmsg_tclTestModeDataS.TunerTestModeData.u8Neighbour;

         poFI->WriteTestModeDataToDP(TM_DATA_NEIGHBOUR, u8Neighbour);

      }

      //extract RDSQuality value
      {
         tU8 u8RDSQuality = otunmsg_tclTestModeDataS.TunerTestModeData.u8Quality;

         poFI->WriteTestModeDataToDP(TM_DATA_QUALITY, u8RDSQuality);

      }

      //extract RDSQuality value
      {
         tU8 u8RDSErrorRate = otunmsg_tclTestModeDataS.TunerTestModeData.u8RdsErrorRate;

         poFI->WriteTestModeDataToDP(TM_DATA_RDS_ERROR_RATE, u8RDSErrorRate);

      }

      //extract HiCut value
      {
               //tU8 u8HiCut = otunmsg_tclTestModeDataS.TunerTestModeData.u8HiCut;

               //poFI->WriteTestModeDataToDP(TM_DATA_HICUT, u8HiCut);

      }
      	   //extract Sharx value
            {
               //tU8 u8Sharx = otunmsg_tclTestModeDataS.TunerTestModeData.u8Sharx;

               //poFI->WriteTestModeDataToDP(TM_DATA_SHARX, u8Sharx);

            }

      //extract AFListLength value
      {
         tU8 u8AFListLength = otunmsg_tclTestModeDataS.TunerTestModeData.u8AFListLength;

         poFI->WriteTestModeDataToDP(TM_AFLIST_LENGTH, u8AFListLength);

      }

      //extract PI value
      {
         tU32 u32PI = otunmsg_tclTestModeDataS.TunerTestModeData.u32PI;

         poFI->WriteTestModeDataToDP(TM_DATA_PI, u32PI);

      }

      //extract PSName
      {
         tChar* pcPSName = otunmsg_tclTestModeDataS.TunerTestModeData.sPSName.szGet( midw_fi_tclString::FI_EN_UTF8 );
         if (pcPSName != NULL)   // HDE: for security of null pointer
         {
            poFI->WriteStringDataToDp(TM_DATA_PSNAME,(tChar*) pcPSName);  
            OSAL_DELETE [] pcPSName;
            pcPSName = NULL;
         }
      }

      //extract TMC count
      {
         tU8 u8TMCStationCount = otunmsg_tclTestModeDataS.TunerTestModeData.u8TMCStationCount;

         poFI->WriteTestModeDataToDP(TM_DATA_INFOBITS, u8TMCStationCount);

      }

      //extract TestModeBits 
      {
         tU8 u8TestModeBits = otunmsg_tclTestModeDataS.TunerTestModeData.u8TestmodeBits.u8Value;

         poFI->WriteTestModeDataToDP(TM_DATA_INFOBITS, u8TestModeBits);
      }

   } //end of if( poFI != NULL )       
}

/*************************************************************************
* FUNCTION:    clTuner_CCAhandler::vHandleTestModeDataDDADDSStatus()
*
* DESCRIPTION: separate handler to extract data from the status message , 
*              this is a internal function used by vHandleFID_G_TESTMODE_DATA_From_FCTuner               
*
* PARAMETER:   amt_tclServiceData* poMessage ( Service data message to extract )
*
* RETURNVALUE: NONE
*
* History:
* InitialVersion
* 
*************************************************************************/

tVoid clTuner_CCAhandler::vHandleTestModeDataDDADDSStatus( amt_tclServiceData * poMessage )  
{
	/* object for handling the status data of type tunmsg_tclTestModeDataS ( server defined) */

   midw_tunerfi_tclMsgFID_TUN_G_GET_DATA_TESTMODE_DDADDSStatus  otunmsg_tclTestModeDataS;

   vGetDataFromAmt( poMessage , otunmsg_tclTestModeDataS );

   if ( poFI != NULL )
   {

	// extract Header

      tU8 u8PhysicalTuner = (tU8)otunmsg_tclTestModeDataS.TunerTestModeData.u8PhysicalTuner.enType;  // check
      u8TunMode = u8PhysicalTuner;

	 	
	

	  // extract Modulation
	  {

	     tU8 u8Band = (tU8)otunmsg_tclTestModeDataS.TunerTestModeData.u8Band.enType;

		 if( u8Band == 0x01 || u8Band == 0x02)
		 {
		    u8Band = TM_BAND_FM;
		 }
		 else if(u8Band == 0x03)
		 {
			u8Band = TM_BAND_AM;
		 }
		 else if(u8Band == 0x06)
		 {
			 u8Band = TM_BAND_DAB;
		 }
		 else
		 {
			 // do nothing
		 }

		 poFI->WriteTestModeDataToDP(TM_DATA_BAND, u8Band);

		 ETG_TRACE_USR4(("vHandleTestModeDataDDADDSStatus: Band : %d, Tuner : %d"
		       				  , 	u8Band,  u8PhysicalTuner));

	  }


      //extract Frequency
      {
         tU32 u32Frequency = otunmsg_tclTestModeDataS.TunerTestModeData.u32AFFrequency;

         poFI->WriteTestModeDataToDP(TM_DATA_FREQUENCY, u32Frequency);

         ETG_TRACE_USR4(("vHandleTestModeDataDDADDSStatus: Frequency : %d, Tuner : %d"
               				  , 	u32Frequency,  u8PhysicalTuner));

      }

      // Added By rrv2kor for extracting HUB info
      {
         tU8 u8HubValue = otunmsg_tclTestModeDataS.TunerTestModeData.u8Hub;

         poFI->WriteTestModeDataToDP(TM_DATA_HUBVALUE, u8HubValue);

         ETG_TRACE_USR4(("vHandleTestModeDataDDADDSStatus: Hub : %d, Tuner : %d"
                        				  , 	u8HubValue,  u8PhysicalTuner));

      }



      //extract signal strength
      {
         tU8 u8Fieldstrength = otunmsg_tclTestModeDataS.TunerTestModeData.u8Fieldstrength;

         poFI->WriteTestModeDataToDP(TM_DATA_FIELDSTRENGTH, u8Fieldstrength);

         ETG_TRACE_USR4(("vHandleTestModeDataDDADDSStatus: Fieldstrength : %d, Tuner : %d"
                                 				  , 	u8Fieldstrength,  u8PhysicalTuner));

      }

      //extract Multipath value
      {
         tU8 u8Multipath = otunmsg_tclTestModeDataS.TunerTestModeData.u8Multipath;

         poFI->WriteTestModeDataToDP(TM_DATA_MULTIPATH, u8Multipath);

         ETG_TRACE_USR4(("vHandleTestModeDataDDADDSStatus: Multipath : %d, Tuner : %d"
                                          				  , 	u8Multipath,  u8PhysicalTuner));

      }

      //extract Neighbour value
      {
         tU8 u8Neighbour = otunmsg_tclTestModeDataS.TunerTestModeData.u8Neighbour;

         poFI->WriteTestModeDataToDP(TM_DATA_NEIGHBOUR, u8Neighbour);

         ETG_TRACE_USR4(("vHandleTestModeDataDDADDSStatus: Neighbour : %d, Tuner : %d"
                                                   				  , 	u8Neighbour,  u8PhysicalTuner));

      }

      //extract RDSQuality value
      {
         tU8 u8Quality = otunmsg_tclTestModeDataS.TunerTestModeData.u8Quality;

         poFI->WriteTestModeDataToDP(TM_DATA_QUALITY, u8Quality);

         ETG_TRACE_USR4(("vHandleTestModeDataDDADDSStatus: Quality : %d, Tuner : %d"
                                                            	  , 	u8Quality,  u8PhysicalTuner));

      }

      //extract RDSErrorRate value
      {
         tU8 u8RDSErrorRate = otunmsg_tclTestModeDataS.TunerTestModeData.u8RdsErrorRate;

         poFI->WriteTestModeDataToDP(TM_DATA_RDS_ERROR_RATE, u8RDSErrorRate);

         ETG_TRACE_USR4(("vHandleTestModeDataDDADDSStatus: RDSErrorRate : %d, Tuner : %d"
                                                                   , 	u8RDSErrorRate,  u8PhysicalTuner));

      }

      //extract HiCut value
      {
         tU8 u8HiCut = otunmsg_tclTestModeDataS.TunerTestModeData.u8HiCut;

         poFI->WriteTestModeDataToDP(TM_DATA_HICUT, u8HiCut);

         ETG_TRACE_USR4(("vHandleTestModeDataDDADDSStatus: HiCut : %d, Tuner : %d"
                                                                    , 	u8HiCut,  u8PhysicalTuner));

      }
	   //extract Sharx value
      {
         tU8 u8Sharx = otunmsg_tclTestModeDataS.TunerTestModeData.u8Sharx;

         poFI->WriteTestModeDataToDP(TM_DATA_SHARX, u8Sharx);

         ETG_TRACE_USR4(("vHandleTestModeDataDDADDSStatus: Sharx : %d, Tuner : %d"
                                                                      ,  u8Sharx,  u8PhysicalTuner));

      }

      //extract AFListLength value
      {
         tU8 u8AFListLength = otunmsg_tclTestModeDataS.TunerTestModeData.u8AFListLength;

         poFI->WriteTestModeDataToDP(TM_AFLIST_LENGTH, u8AFListLength);

         ETG_TRACE_USR4(("vHandleTestModeDataDDADDSStatus: AFListLength : %d, Tuner : %d"
                                                                        ,  u8AFListLength,  u8PhysicalTuner));

      }

	  //extract ChannelSeperation value
	  {
		  tU8 u8ChannelSeparation = otunmsg_tclTestModeDataS.TunerTestModeData.u8ChannelSeparation;

		  poFI->WriteTestModeDataToDP(TM_DATA_CHANNELSEPERATION, u8ChannelSeparation);

		  ETG_TRACE_USR4(("vHandleTestModeDataDDADDSStatus: ChannelSeparation : %d, Tuner : %d"
		                                                                ,  u8ChannelSeparation,  u8PhysicalTuner));

	  }



      //extract PI value
      {
         tU16 u16PI = (tU16)otunmsg_tclTestModeDataS.TunerTestModeData.u16PI;

         poFI->WriteTestModeDataToDP(TM_DATA_PI, u16PI);

         ETG_TRACE_USR4(("vHandleTestModeDataDDADDSStatus: PI : %d, Tuner : %d"
         		                                                             ,  u16PI,  u8PhysicalTuner));

      }

      //extract PSName
      {
         tChar* pcPSName = otunmsg_tclTestModeDataS.TunerTestModeData.sPSName.szGet( midw_fi_tclString::FI_EN_UTF8 );

         if( NULL != pcPSName )  // for security of null pointer
         {
            poFI->WriteStringDataToDp(TM_DATA_PSNAME,(tChar*) pcPSName);  
            OSAL_DELETE [] pcPSName;

            pcPSName = NULL;
         }
      }


      //extract Info bits
      {
         tU8 u8Infobits = otunmsg_tclTestModeDataS.TunerTestModeData.u8TestmodeBits.u8Value;

         poFI->WriteTestModeDataToDP(TM_DATA_INFOBITS, u8Infobits);

         ETG_TRACE_USR4(("vHandleTestModeDataDDADDSStatus: InfoBits"));
      }




      //Extract Mode Information Added by rrv2kor
      /*{
         midw_fi_tcl_e8_Tuner_Mode::tenType enModeType = otunmsg_tclTestModeDataS.TunerTestModeData.u8Mode.enType;
         poFI->WriteTestModeDataToDP(TM_DATA_TESTMODE_GET_MODE, (tU8)enModeType);        
      }*/
		
	  tU8 u8TunerConfig =0;
      if(NULL != poFI->m_poDataPool)
      {
         poFI->m_poDataPool->u32dp_get( DPSYSTEM__VARIANT_CONFIG_RADIO_TUNER_MODE,
            &u8TunerConfig, 
            sizeof(u8TunerConfig));
      }
	  
		ETG_TRACE_USR4(("vHandleTestModeDataDDADDSStatus: u8TunerConfig = %d", u8TunerConfig ));
		u8NoTuners ++;
		ETG_TRACE_USR4(("vHandleTestModeDataDDADDSStatus: u8NoTuners = %d", u8NoTuners ));

      if ( NULL != pclHSI_GetTestMode())
       {
			if(u8TunerConfig == SINGLE_TUNER)
			{
				if(u8NoTuners == 0x01)
				{
					ETG_TRACE_USR4(("vHandleTestModeDataDDADDSStatus: Single tuner" ));
					 sMsg refMsg(NULL, (tU32)CMMSG_TESTMODE_REDRAW_TRIGGER, NULL, NULL);
					pclHSI_GetTestMode()->bExecuteMessage( refMsg );
					u8NoTuners = 0;

					ETG_TRACE_USR4(("vHandleTestModeDataDDADDSStatus: CMMSG_TESTMODE_REDRAW_TRIGGER is called" ));
				}
			}
			else
			{
				if(u8NoTuners == 0x02)
				{
					ETG_TRACE_USR4(("vHandleTestModeDataDDADDSStatus: Double tuner" ));
				   sMsg refMsg(NULL, (tU32)CMMSG_TESTMODE_REDRAW_TRIGGER, NULL, NULL);
				   pclHSI_GetTestMode()->bExecuteMessage( refMsg );
					u8NoTuners = 0;	
					
				   ETG_TRACE_USR4(("vHandleTestModeDataDDADDSStatus: CMMSG_TESTMODE_REDRAW_TRIGGER is called" ));	
								   
				}
			}
        }
     }
 }



/*************************************************************************
* FUNCTION:    clTuner_CCAhandler::vHandleStaticListElement()
*
* DESCRIPTION: separate handler to extract data from the status message , 
*              this is a internal function used by 
*              vHandleFID_G_GET_STATIC_LIST_ELEMENT_From_FCTuner()
*
* PARAMETER:   amt_tclServiceData* poMessage(Service data message to extract)
*
* RETURNVALUE: NONE
*
* History:
* InitialVersion
*
*************************************************************************/
tVoid clTuner_CCAhandler::vHandleStaticListElement( amt_tclServiceData* poMessage )
{
   /* object for handling the status data of type
   midw_tunerfi_tclMsgFID_TUN_G_STATIC_LIST_ELEMENTStatus ( server defined) */

   midw_tunerfi_tclMsgFID_TUN_G_STATIC_LIST_ELEMENTStatus otunmsg_tclGetStaticListElem;

   vGetDataFromAmt( poMessage , otunmsg_tclGetStaticListElem );

   if ( poFI != NULL )
   {
      //extract List Element ID
      tU8 u8ListElement = otunmsg_tclGetStaticListElem.TunerStationData.u8ElementID ;

      //extract AvailFlags
      tU32 u32Flags = otunmsg_tclGetStaticListElem.TunerStationData.u32AvailFlags.u32Value;


      //extract Frequency
      tU32 u32Frequency = otunmsg_tclGetStaticListElem.TunerStationData.u32Frequency;

      //extract PSName
      tChar* pcPSName = otunmsg_tclGetStaticListElem.TunerStationData.sPSName.szGet( midw_fi_tclString::FI_EN_UTF8);

      // check if PSName is available
      tU8 u8Flag = (u32Flags & STATIC_LIST_PSNAME ) ? (tU8)STATIC_LIST_PSNAME : (tU8)STATIC_LIST_FREQ ;

      if( NULL != pcPSName )  // for security of null pointer
      {
#ifdef VARIANT_S_FTR_ENABLE_EXT_PRESETLIST
#ifndef WIN32
         //extract Packed Info for getting TP status
         tU16 u16PackedInfo = otunmsg_tclGetStaticListElem.TunerStationData.u16PackedInfo;

         //extract the TP Status bit which is Bit 0
         tBool bTpStatus = u16PackedInfo & PACKEDINFO_TP_ON_BIT;

         poFI->WriteStaticListElementsToDP(u8ListElement , u8Flag ,(tChar*) pcPSName ,u32Frequency ,bTpStatus );
#endif
#else
         poFI->WriteStaticListElementsToDP(u8ListElement , u8Flag ,(tChar*) pcPSName ,u32Frequency  );
#endif
         OSAL_DELETE [] pcPSName;

         pcPSName = NULL;
      }
   }
}




/*************************************************************************
*  FUNCTION:    clTuner_CCAhandler::vHandleFID_TUN_BAND_CHANGE_To_FCTuner()
*
* DESCRIPTION: handler to form a CCA Message with embedding required data
*               for FID= FID_TUN_BAND_CHANGE  and send to Server
*
* PARAMETER:  tU8 u8BandId  ( to embed in the sending CCA message)
*
* RETURNVALUE: NONE
*
* History:
* InitialVersion
* 
*************************************************************************/
tVoid clTuner_CCAhandler::vHandleFID_TUN_BAND_CHANGE_To_FCTuner(tU8 u8BandId)
{

   if ( m_poTrace != NULL )
   {
      m_poTrace->vTrace(  TR_LEVEL_HMI_INFO,
         TR_CLASS_HSI_TUNER_MSG, 
         (tU16)TUN_FID_TUN_BAND_CHANGE_TO_SERV);
   }

   /* create a CCA Msg object oMsgData */
   midw_tunerfi_tclMsgFID_TUN_CHANGE_BANDMethodStart oMsgData;
   oMsgData.u8Band.enType = (midw_fi_tcl_e8_Tun_TunerBand::tenType)u8BandId;

   bSendCCAMessage( MIDW_TUNERFI_C_U16_FID_TUN_CHANGE_BAND,
      AMT_C_U8_CCAMSG_OPCODE_METHODSTART, 
      oMsgData
      );
   return;
}

/*************************************************************************
*
* FUNCTION: pHSI_GetRadio
*
* DESCRIPTION: Method to get a Pointer to ComponentManager Radio
*
* PARAMETER:
*
* RETURNVALUE: clHSI_Radio*
*
*************************************************************************/

clHSI_Radio* clTuner_CCAhandler::pHSI_GetRadio()
{
   if (m_pHSI_Radio == NULL)
   {
      clHSI_CMMngr*  pclHSI_CMManager = clHSI_CMMngr::pclGetInstance();
      
      if(pclHSI_CMManager != NULL)
      {
      m_pHSI_Radio = dynamic_cast <clHSI_Radio*>(pclHSI_CMManager->pHSI_BaseGet((tU8)EN_HSI_RADIO));
      }
   }
   return m_pHSI_Radio;
   
}

clHSI_TunerMaster* clTuner_CCAhandler::pHSI_GetTunerMaster()
{
   if (m_pHSI_TunerMaster == NULL)
   {
      clHSI_CMMngr*  pclHSI_CMManager = clHSI_CMMngr::pclGetInstance();

      if(pclHSI_CMManager != NULL)
      {
      m_pHSI_TunerMaster = dynamic_cast <clHSI_TunerMaster*>(pclHSI_CMManager->pHSI_BaseGet((tU8)EN_HSI_TUNERMASTER));
      }
   }
   return m_pHSI_TunerMaster;
}
/*************************************************************************
*
* FUNCTION: pclHSI_GetTestMode
*
* DESCRIPTION: Method to get a Pointer to ComponentManager TestMode
*
* PARAMETER:
*
* RETURNVALUE: clHSI_TestMode*
*
*************************************************************************/

clHSI_TestMode* clTuner_CCAhandler::pclHSI_GetTestMode()
{
   if (m_pHSI_TestModeRadio == NULL)
   {
      clHSI_CMMngr*  pclHSI_CMManager = clHSI_CMMngr::pclGetInstance();
      
      if(pclHSI_CMManager != NULL)
      {
      m_pHSI_TestModeRadio = dynamic_cast <clHSI_TestMode*>(pclHSI_CMManager->pHSI_BaseGet((tU8)EN_HSI_SYSTEM_TESTMODE));
      }
   }
   return m_pHSI_TestModeRadio;
}
/*************************************************************************
*  FUNCTION: clTuner_CCAhandler::vHandleFID_G_RADIO_TEXT_From_FCTuner( )
*
* DESCRIPTION: handler to extract data from the status message from server for
*               FID= FID_TUN_G_OPERATION
*
* PARAMETER:   amt_tclServiceData* poMessage (data message to process)
*
* RETURNVALUE: NONE
*
* History:
* InitialVersion
* 
*************************************************************************/
tVoid clTuner_CCAhandler::vHandleFID_G_RADIO_TEXT_From_FCTuner(
   amt_tclServiceData* poMessage
   )
{
   /* check the parameter for not NULL and then for validity of a messsage */
   if ( ( NULL != poMessage ) && ( poMessage->bIsValid() ) )
   {
      switch ( poMessage->u8GetOpCode() ) // get the opcode from the message
      {

      case AMT_C_U8_CCAMSG_OPCODE_STATUS:
         {
            vStartInitSequence( MIDW_TUNERFI_C_U16_FID_TUN_G_RADIO_TEXT );

            /* extract the data from AMT Class to FI Class */
            midw_tunerfi_tclMsgFID_TUN_G_RADIO_TEXTStatus oMsgData;
            vGetDataFromAmt( poMessage , oMsgData );

            midw_fi_tclString radioText = oMsgData.TunerRadioText.sRadioTextData;
            vTrace(CCA_RADIO_RADIO_TEXT,TR_LEVEL_HMI_INFO,(tU8)strlen(radioText.szValue)+1,(tU8*)radioText.szValue);
        	
            clHSI_Radio    *pHSI  =  pHSI_GetRadio();
            if (pHSI != NULL)
            {
            	pHSI->vSetFMRadioText( radioText.szValue );
            }

            /*
            if(poFI != NULL)
            {
            poFI->WriteDataToDp(MIDW_TUNERFI_C_U16_FID_TUN_G_RADIO_TEXT, u8Data);
            }
            */
            break;
         }

      case AMT_C_U8_CCAMSG_OPCODE_ERROR:
         {
            /* handle error message in a separate function */
            vHandleCCAErrorMessage( poMessage  );
            break;
         }

      default: // unknown opcode
         {
            /* send error message back to server for unkown opcode */
            vSendErrorMessage(
               poMessage->u16GetFunctionID(),
               AMT_C_U16_ERROR_INVALID_OPCODE
               );
            break;
         }
      } // end of switch
   }
}

/*************************************************************************
*  FUNCTION:    tVoid clTuner_CCAhandler::vHandleFID_TUN_S_SEL_MIXEDPRESET_LIST_ELEM_To_FCTuner()
*
* DESCRIPTION: handler to form a CCA Message with embedding required data for
*               FID = FID_TUN_S_SEL_MIXEDPRESET_LIST_ELEM and send to Server
*
* PARAMETER:   tU8 u8Command
*
* RETURNVALUE: NONE
*
* History:
* InitialVersion
* 
*************************************************************************/
tVoid clTuner_CCAhandler::vHandleFID_TUN_S_SEL_MIXEDPRESET_LIST_ELEM_To_FCTuner(tU8 u8Preset)
{
   if (m_poTrace != NULL)
   {
      m_poTrace->vTrace(TR_LEVEL_HMI_INFO,
         TR_CLASS_HSI_TUNER_MSG, 
         (tU16)FID_TUN_S_SEL_MIXEDPRESET_LIST_ELEM_To_FCTuner);  //change?
   }

   /* create a CCA Msg object oMsgData */
   midw_tunerfi_tclMsgFID_TUN_S_SEL_MIXEDPRESET_LIST_ELEMMethodStart oMsgData;
   oMsgData.u8ListElementID = u8Preset;

   bSendCCAMessage( MIDW_TUNERFI_C_U16_FID_TUN_S_SEL_MIXEDPRESET_LIST_ELEM,
      AMT_C_U8_CCAMSG_OPCODE_METHODSTART, 
      oMsgData
      );
}

/*************************************************************************
* FUNCTION:    clTuner_CCAhandler::vHandleFID_S_AUTOSTORE_MIXEDPRESET_To_FCTuner()
*
* DESCRIPTION: handler to form a CCA Message with embedding required data
*              for FID= FID_TUN_S_SET_STATIC_LIST  and send to Server
*
* PARAMETER:   NONE
*
* RETURNVALUE: NONE
*
* History:
* InitialVersion
* 
*************************************************************************/
tVoid clTuner_CCAhandler::vHandleFID_S_AUTOSTORE_MIXEDPRESET_To_FCTuner(
   midw_fi_tcl_e8_Tun_TunerMixedpresetList::tenType enListtype)
{
   if (m_poTrace != NULL)
   {
      m_poTrace->vTrace(
         TR_LEVEL_HMI_INFO,
         TR_CLASS_HSI_TUNER_MSG, 
         (tU16)TUN_FID_S_AUTOSTORE_MIXEDPRESET_To_SERV);
   }

   /* create a CCA Msg object oMsgData */
   midw_tunerfi_tclMsgFID_TUN_S_AUTOSTORE_MIXEDPRESET_LISTMethodStart oMsgData;
   oMsgData.u8ListID.enType = enListtype;

   bSendCCAMessage( MIDW_TUNERFI_C_U16_FID_TUN_S_AUTOSTORE_MIXEDPRESET_LIST,
      AMT_C_U8_CCAMSG_OPCODE_METHODSTART, 
      oMsgData
      );

   return;
}

/*************************************************************************
* FUNCTION:    clTuner_CCAhandler::vHandleFID_TUN_S_DELETE_MIXEDPRESET_LIST_ELEM_To_FCTuner()
*
* DESCRIPTION: deletes one or all entries of the Mixedpreset list
*
* PARAMETER:   enDeleteMode:  FI_EN_ONE_ELEMENT - deletes one element ID is given by second parameter
*                             FI_EN_ALL_ELEMENTS- deletes all elements in the MixedPreset-list
*              u8ElemID:      index of the to be deleted Entry
*
* RETURNVALUE: NONE
*
* History:
* InitialVersion
* 
*************************************************************************/
tVoid clTuner_CCAhandler::vHandleFID_TUN_S_DELETE_MIXEDPRESET_LIST_ELEM_To_FCTuner(tEN_E8_TUN_DELETEELEMENTS  enDeleteMode,
                                                                                   tU8 u8ElemID)
{
   if (m_poTrace != NULL)
   {
      m_poTrace->vTrace(
         TR_LEVEL_HMI_INFO,
         TR_CLASS_HSI_TUNER_MSG,
         (tU16)TUN_FID_S_DELETE_MIXEDPRESET_LIST_ELEM_SERV);
   }

   /* create a CCA Msg object oMsgData */
   midw_tunerfi_tclMsgFID_TUN_S_DELETE_MIXEDPRESET_LIST_ELEMMethodStart oMsgData;
   oMsgData.u8ListElementID.e8Action.enType = (midw_fi_tcl_e8_Tun_DeleteElements::tenType)enDeleteMode;
   oMsgData.u8ListElementID.u8ElementID = u8ElemID;

   bSendCCAMessage( MIDW_TUNERFI_C_U16_FID_TUN_S_DELETE_MIXEDPRESET_LIST_ELEM,
      AMT_C_U8_CCAMSG_OPCODE_METHODSTART,
      oMsgData
      );
   return;
}


/*************************************************************************
* FUNCTION:    clTuner_CCAhandler::vHandleFID_TUN_S_GET_MIXEDPRESET_LIST_To_FCTuner()
*
* DESCRIPTION: loads from the middleware the mixedpresetlist
*
* PARAMETER:   none
*
* RETURNVALUE: NONE
*
* History:
* InitialVersion
* 
*************************************************************************/
tVoid clTuner_CCAhandler::vHandleFID_TUN_S_GET_MIXEDPRESET_LIST_To_FCTuner()
{
   if (m_poTrace != NULL)
   {
      m_poTrace->vTrace(
         TR_LEVEL_HMI_INFO,
         TR_CLASS_HSI_TUNER_MSG, 
         (tU16)TUN_FID_S_GET_MIXEDPRESET_LIST_ELEM_SERV);
   }

   /* create a CCA Msg object oMsgData */
   midw_tunerfi_tclMsgFID_TUN_S_GET_MIXEDPRESET_LISTMethodStart oMsgData;
   oMsgData.GetMixedpresetList.u8FirstElementID = 0;
   oMsgData.GetMixedpresetList.u8NumberOfElements = clHSI_MIXEDPRESETLIST_MAXCOUNT;

   bSendCCAMessage( MIDW_TUNERFI_C_U16_FID_TUN_S_GET_MIXEDPRESET_LIST,
      AMT_C_U8_CCAMSG_OPCODE_METHODSTART, 
      oMsgData
      );
}


/*************************************************************************
* FUNCTION:    clTuner_CCAhandler::vHandleFID_TUN_S_SAVE_MIXEDPRESET_LIST_ELEM_To_FCTuner()
*
* DESCRIPTION: save the current station at the given index
*
* PARAMETER:   u8ListElementIndex      - the index, where to save the current station
*
* RETURNVALUE: NONE
*
* History:
* InitialVersion
* 
*************************************************************************/
tVoid clTuner_CCAhandler::vHandleFID_TUN_S_SAVE_MIXEDPRESET_LIST_ELEM_To_FCTuner(tU8  u8ListElementIndex)

{
   if (m_poTrace != NULL)
   {
      m_poTrace->vTrace(
         TR_LEVEL_HMI_INFO,
         TR_CLASS_HSI_TUNER_MSG,
         (tU16)TUN_FID_S_SAVE_MIXEDPRESET_LIST_ELEM_SERV,
         ONE_ELEMENT,
         &u8ListElementIndex);
   }

   /* create a CCA Msg object oMsgData */
   midw_tunerfi_tclMsgFID_TUN_S_SAVE_MIXEDPRESET_LIST_ELEMMethodStart oMsgData;
   oMsgData.u8ListElementID = u8ListElementIndex;

   bSendCCAMessage( MIDW_TUNERFI_C_U16_FID_TUN_S_SAVE_MIXEDPRESET_LIST_ELEM,
      AMT_C_U8_CCAMSG_OPCODE_METHODSTART,
      oMsgData
      );
}


/*************************************************************************
*  FUNCTION:    tVoid vHandleFID_S_ATSEEK_WITH_TUNERHEADER_To_FCTuner
*
* DESCRIPTION:  Method Start to seek with tuner Header information.
*
* PARAMETER:
*
* RETURNVALUE: tVoid
*
* History:
* InitialVersion
*
*************************************************************************/
tVoid clTuner_CCAhandler::vHandleFID_S_ATSEEK_WITH_TUNERHEADER_To_FCTuner(tU8 u8TunerSeek, tU8 u8TunerHeader, tU32 u32Frequency)
{
   midw_tunerfi_tclMsgFID_TUN_S_ATSEEK_WITH_TUNERHEADERMethodStart oMsgData;

   oMsgData.u8TunerSeek.enType   = (midw_fi_tcl_e8_Tun_TunerSeek::tenType)u8TunerSeek;
   oMsgData.u8TunerHeader.enType = (midw_fi_tcl_e8_Tun_TunerHeader::tenType)u8TunerHeader;
   oMsgData.u32Frequency         = u32Frequency;
   
   bSeekState = u8TunerSeek;

   vTrace(CCA_RADIO_S_ATSEEK_WITH_TUNERHEADER, TR_LEVEL_HMI_INFO,u8TunerSeek,u8TunerHeader,0);
   vTrace(CCA_RADIO_S_ATSEEK_WITH_TUNERHEADER_FRQ, TR_LEVEL_HMI_INFO,u32Frequency);

   /* post the message to Server thro Set Property */
   bSendCCAMessage( MIDW_TUNERFI_C_U16_FID_TUN_S_ATSEEK_WITH_TUNERHEADER,
      AMT_C_U8_CCAMSG_OPCODE_METHODSTART,
      oMsgData );
}


/*************************************************************************
*  FUNCTION:    tVoid vHandleFID_TUN_S_STATIONLIST_EXIT_To_FCTuner
*
* DESCRIPTION:  Method Start to close station list.
*
* PARAMETER:
*
* RETURNVALUE: tVoid
*
* History:
* InitialVersion
*
*************************************************************************/
tVoid clTuner_CCAhandler::vHandleFID_TUN_S_STATIONLIST_EXIT_To_FCTuner()
{
   midw_tunerfi_tclMsgFID_TUN_S_STATIONLIST_EXITMethodStart oMsgData;
   
   oMsgData.u16ConfigListID = TUN_LIST_FM;

   ETG_TRACE_USR4(("vHandleFID_TUN_S_STATIONLIST_EXIT_To_FCTuner"));

   bSendCCAMessage( MIDW_TUNERFI_C_U16_FID_TUN_S_STATIONLIST_EXIT,
      AMT_C_U8_CCAMSG_OPCODE_METHODSTART,
      oMsgData );
}

/*************************************************************************
*  FUNCTION:    tVoid vHandleFID_S_ATFREQUENCY_WITH_TUNERHEADER_To_FCTuner(tU8 u8Direction, tU8 u8TunerHeader)
*
* DESCRIPTION:  Method Start to do manual seek with tuner Header information.
*
* PARAMETER:
*
* RETURNVALUE: tVoid
*
* History:
* InitialVersion
*
*************************************************************************/
tVoid clTuner_CCAhandler::vHandleFID_S_ATFREQUENCY_WITH_TUNERHEADER_To_FCTuner(tU8 u8Direction, tU8 u8TunerHeader)
{
   midw_tunerfi_tclMsgFID_TUN_S_ATFREQUENCY_WITH_TUNERHEADERMethodStart oMsgData;

   oMsgData.ManualFreqAdjustment.enType = (midw_fi_tcl_e8_Tun_ManualFreqAdjustment::tenType)u8Direction;
   oMsgData.TunerHeader.enType =  (midw_fi_tcl_e8_Tun_TunerHeader::tenType)u8TunerHeader;

   vTrace(CCA_RADIO_S_ATFREQUENCY_WITH_TUNERHEADER, TR_LEVEL_HMI_INFO,u8Direction,u8TunerHeader,0);

   /* post the message to Server thro Set Property */
   bSendCCAMessage( MIDW_TUNERFI_C_U16_FID_TUN_S_ATFREQUENCY_WITH_TUNERHEADER,
      AMT_C_U8_CCAMSG_OPCODE_METHODSTART,
      oMsgData );
}

/*************************************************************************
* FUNCTION:    tVoid vHandleFID_TUN_S_PS_FREEZE_To_FCTuner(tU32 u32PI,tString strPSName)
*
* DESCRIPTION: Method Start to do a PS-Freeze
*
* PARAMETER:   u32PI    -  PI-Code of the station that have to be freezed
*              strPSName-  the freezed name of the station
*
* RETURNVALUE: tVoid
*
* History:
* InitialVersion
*
*************************************************************************/
tVoid clTuner_CCAhandler::vHandleFID_TUN_S_PS_FREEZE_To_FCTuner(  tU32     u32PI,
                                                                tString  strPSName)
{
   if (m_poTrace != NULL)
   {
      m_poTrace->vTrace(
         TR_LEVEL_HMI_INFO,
         TR_CLASS_HSI_TUNER_MSG,
         (tU16)TUN_FID_S_PS_FREEZE_SERV);
   }

   /* create a CCA Msg object oMsgData */
   midw_tunerfi_tclMsgFID_TUN_S_PS_FREEZEMethodStart oMsgData;
   oMsgData.PS_PI_COMBINATION.u32PI = u32PI;
   oMsgData.PS_PI_COMBINATION.sPSName.bSet((tCString)strPSName,midw_fi_tclString::FI_EN_UTF8);

   bSendCCAMessage( MIDW_TUNERFI_C_U16_FID_TUN_S_PS_FREEZE,
      AMT_C_U8_CCAMSG_OPCODE_METHODSTART,
      oMsgData
      );
}

/*************************************************************************
*  FUNCTION:    clTuner_CCAhandler::vHandleFID_G_STATION_DATA_VAG_From_FCTuner()
*
* DESCRIPTION: handler to extract data from the status message from server for
*              FID= FID_TUN_G_STATION_DATA_VAG
*
* PARAMETER:   amt_tclServiceData* poMessage ( data message to process )
*
* RETURNVALUE: NONE
*
* History:
* InitialVersion
*
*************************************************************************/
tVoid clTuner_CCAhandler::vHandleFID_G_STATION_DATA_VAG_From_FCTuner(amt_tclServiceData* poMessage)
{
#ifdef VR_SRV_ENABLE
   tU8 u8CmdVR = 0;
#endif

   /* check the parameter for not NULL and then for validity of a messsage */
   if ( (poMessage != NULL) && (poMessage->bIsValid()) )
   {

      switch (poMessage->u8GetOpCode())  // get the opcode from the message
      {
      case AMT_C_U8_CCAMSG_OPCODE_STATUS:
         {
            vStartInitSequence(MIDW_TUNERFI_C_U16_FID_TUN_G_STATION_DATA_VAG);

            if ( m_poTrace != NULL )
            {
               m_poTrace->vTrace(  TR_LEVEL_HMI_INFO,
                  TR_CLASS_HSI_TUNER_MSG,
                  (tU16)TUN_FID_STATION_DATA_FROM_SERV);
            }

            /* separate handler for data extraction as data size is too big*/
            tU8 u8taplaying=0;
            if(NULL != poFI->m_poDataPool)
            {
               poFI->m_poDataPool->u32dp_get( DPTUNER__TA_STATUS,
                  &u8taplaying,
                  sizeof(u8taplaying));
            }


            vTrace(TR_LEVEL_HMI_INFO, "*************u8taplaying==%d**********************",u8taplaying);


#ifdef VR_SRV_ENABLE
            if( poFI != NULL )
            {
               if(clHSIDeviceGeneral::m_poDataPool != NULL)
               {
                  clHSIDeviceGeneral::m_poDataPool -> u32dp_get( DPVC__INPUT_CMD ,
                     &u8CmdVR , sizeof(u8CmdVR) );

                  if(VR_TUN_SELECT_STATION == u8CmdVR ||
                     VR_TUN_SELECT_BAND_MEM_BANK == u8CmdVR ||
                     VR_TUN_AUTOSTORE == u8CmdVR  ||
                     VR_TUN_STORE_STATION == u8CmdVR)
                  {
                     poFI->WriteDataToDp(TUNER_HSI_NO_ERROR,
                        HSI_NO_ERROR);
                  }
               }

            }
#endif

            break;
         }
      case AMT_C_U8_CCAMSG_OPCODE_ERROR:
         {

#ifdef VR_SRV_ENABLE
            tU8 u8VrStatus = 0;

            if(poFI != NULL)
            {
               if(clHSIDeviceGeneral::m_poDataPool != NULL)
               {
                  clHSIDeviceGeneral::m_poDataPool -> u32dp_get( DPVC__INPUT_CMD ,
                     &u8CmdVR , sizeof(u8CmdVR) );
               }
            }

            if(VR_TUN_SELECT_STATION == u8CmdVR ||
               VR_TUN_SELECT_BAND_MEM_BANK == u8CmdVR ||
               VR_TUN_AUTOSTORE == u8CmdVR  ||
               VR_TUN_STORE_STATION == u8CmdVR)
            {
               u8VrStatus = VR_INPUT_CMD_TRUE;
            }
            else
            {
               u8VrStatus = VR_INPUT_CMD_FALSE;
            }
#endif
            /* handle error message in a separate function */
            vHandleCCAErrorMessage( poMessage );

            break;
         }

      default : // unknown opcode
         {
#ifdef VR_SRV_ENABLE
            if(poFI != NULL)
            {
               if(clHSIDeviceGeneral::m_poDataPool != NULL)
               {
                  clHSIDeviceGeneral::m_poDataPool -> u32dp_get( DPVC__INPUT_CMD ,
                     &u8CmdVR , sizeof(u8CmdVR) );

                  if(VR_TUN_SELECT_STATION == u8CmdVR ||
                     VR_TUN_SELECT_BAND_MEM_BANK == u8CmdVR ||
                     VR_TUN_AUTOSTORE == u8CmdVR  ||
                     VR_TUN_STORE_STATION == u8CmdVR)
                  {
                     poFI->WriteDataToDp(TUNER_HSI_ERROR_UNKNOWN,
                        HSI_ERROR_UNKNOWN);
                  }
               }
            }
#endif

            /* send error message back to server for unkown opcode */
            vSendErrorMessage(   poMessage->u16GetFunctionID(),
               AMT_C_U16_ERROR_INVALID_OPCODE);
            break;
         }
      } // end of switch
   }
}



/*************************************************************************
*  FUNCTION:    clTuner_CCAhandler::vHandleFID_G_STATIONINFO_VAG_From_FCTuner()
*
* DESCRIPTION: handler to extract data from the status message from server for
*              FID= FID_TUN_G_ATSTATIONINFO
*
* PARAMETER:   amt_tclServiceData* poMessage ( data message to process )
*
* RETURNVALUE: NONE
*
* History:
* InitialVersion
*
*************************************************************************/
tVoid clTuner_CCAhandler::vHandleFID_G_ATSTATIONINFO_VAG_From_FCTuner(amt_tclServiceData* poMessage)
{
#ifdef VR_SRV_ENABLE
   tU8 u8CmdVR = 0;
#endif

   /* check the parameter for not NULL and then for validity of a messsage */
   if ( (poMessage != NULL) && (poMessage->bIsValid()) )
   {

      switch (poMessage->u8GetOpCode())  // get the opcode from the message
      {
      case AMT_C_U8_CCAMSG_OPCODE_STATUS:
         {
            
        	 ETG_TRACE_USR4(("vHandleFID_G_ATSTATIONINFO_VAG_From_FCTuner:")); 
        	 vStartInitSequence(MIDW_TUNERFI_C_U16_FID_TUN_G_ATSTATIONINFO);
        	 
        	 midw_tunerfi_tclMsgFID_TUN_G_ATSTATIONINFOStatus otunmsg_tclStationS;
        	 vGetDataFromAmt( poMessage , otunmsg_tclStationS );
        	 
        	 ETG_TRACE_USR4(("vHandleFID_G_ATSTATIONINFO_VAG_From_FCTuner:bDirectFreq = %d",bDirectFreq));  
            
            if(bDirectFreq == true)
           	{
           	  // do nothing
           	}
            else
            {

            vHandleAtStationInfo( poMessage );
            
            
            }



            break;
         }
      case AMT_C_U8_CCAMSG_OPCODE_ERROR:
               {
                  /* handle error message in a separate function */
                  vHandleCCAErrorMessage( poMessage );

                  break;
               }

            default : // unknown opcode
               {
                  /* send error message back to server for unkown opcode */
                  vSendErrorMessage(
                     poMessage->u16GetFunctionID(),
                     AMT_C_U16_ERROR_INVALID_OPCODE
                     );
                  break;
               }

        } // end of switch
    }
}


/*************************************************************************
*  FUNCTION:    clTuner_CCAhandler::vHandleFID_G_BAND_PLUS_STATION_INFO_From_FCTuner()
*
* DESCRIPTION: handler to extract data from the status message from server for
*              FID= FID_TUN_G_BAND_PLUS_STATION_INFO
*
* PARAMETER:   amt_tclServiceData* poMessage ( data message to process )
*
* RETURNVALUE: NONE
*
* History:
* InitialVersion
*
*************************************************************************/
tVoid clTuner_CCAhandler::vHandleFID_G_BAND_PLUS_STATION_INFO_From_FCTuner(amt_tclServiceData* poMessage)
{
#ifdef VR_SRV_ENABLE
   tU8 u8CmdVR = 0;
#endif

   /* check the parameter for not NULL and then for validity of a messsage */
   if ( (poMessage != NULL) && (poMessage->bIsValid()) )
   {

      switch (poMessage->u8GetOpCode())  // get the opcode from the message
      {
      case AMT_C_U8_CCAMSG_OPCODE_STATUS:
         {
            vStartInitSequence(MIDW_TUNERFI_C_U16_FID_TUN_G_BAND_PLUS_STATION_INFO);

            vHandleBand_Station_info( poMessage );

            break;
         }
      case AMT_C_U8_CCAMSG_OPCODE_ERROR:
               {
                  /* handle error message in a separate function */
                  vHandleCCAErrorMessage( poMessage );

                  break;
               }

            default : // unknown opcode
               {
                  /* send error message back to server for unkown opcode */
                  vSendErrorMessage(
                     poMessage->u16GetFunctionID(),
                     AMT_C_U16_ERROR_INVALID_OPCODE
                     );
                  break;
               }

        } // end of switch
    }
}

/*************************************************************************
*  FUNCTION: clTuner_CCAhandler::vHandleFID_G_TMC_INFO_From_FCTuner( )
*
* DESCRIPTION: handler to extract tmc info from the status message from server for 
*               FID= FID_TUN_G_OPERATION  
*
* PARAMETER:   amt_tclServiceData* poMessage (data message to process)
*
* RETURNVALUE: NONE
*
* History:
* InitialVersion
* 
*************************************************************************/
tVoid clTuner_CCAhandler::vHandleFID_G_TMC_INFO_From_FCTuner(
   amt_tclServiceData* poMessage
   )
{ 
   /* check the parameter for not NULL and then for validity of a messsage */
   if ( ( NULL != poMessage ) && ( poMessage->bIsValid() ) )
   {
      switch ( poMessage->u8GetOpCode() ) // get the opcode from the message
      {
      case AMT_C_U8_CCAMSG_OPCODE_STATUS:
         {

        	 vStartInitSequence( MIDW_TUNERFI_C_U16_FID_TUN_G_TMC_INFO );

            /* extract the data from AMT Class to FI Class */
            midw_tunerfi_tclMsgFID_TUN_G_TMC_INFOStatus oMsgData;			
            vGetDataFromAmt( poMessage , oMsgData );

            if(poFI != NULL)
            {
               poFI->WriteDataToDp(TUNER_HSI_TMC_INFO, (tU32)oMsgData.bTMCAvailableFlag);
            }
            break;
         }

      case AMT_C_U8_CCAMSG_OPCODE_ERROR:
         {      
            /* handle error message in a separate function */
            vHandleCCAErrorMessage( poMessage );
            break;
         } 

      default: // unknown opcode
         {
            /* send error message back to server for unkown opcode */
            vSendErrorMessage(
               poMessage->u16GetFunctionID(),            
               AMT_C_U16_ERROR_INVALID_OPCODE
               );
            break;
         }
      } // end of switch
   }
}

/*!***********************************************************************
* METHOD:         vTrace
* CLASS:         clTuner_CCAhandler
* DESCRIPTION:   private helper method to generate info and error traces
* PARAMETER: 
*               u16TraceId(I):
*                      unique TTFIS trace id
*               enTraceLevel(I):
*                      trace level (TR_LEVEL_HMI_ERROR / _INFO / ...)
*               u32TraceData(I):
*                      additional trace data (default == 0)
* RETURNVALUE:
*               none
*************************************************************************/
tVoid clTuner_CCAhandler::vTrace(tU16 u16TraceId, hmi_tenTraceLevel enTraceLevel, tU32 u32TraceData) const
{
   if(m_poTrace != NULL)
   {
      tU32 u32TrData[1];
      u32TrData[0] = u32TraceData;

      m_poTrace->vTrace(enTraceLevel, TR_CLASS_HSI_TUNER_MSG,
         u16TraceId, 1, u32TrData);
   }
}

tVoid clTuner_CCAhandler::vTrace( hmi_tenTraceLevel eTraceLevel, const tChar *pcBuffer, ...) const
{
   if(m_poTrace != NULL)
   {
      tChar acBuffer[255];

      OSAL_tVarArgList arg;
      OSAL_VarArgStart( arg, pcBuffer);//lint !e1055 !e530 !e830 Weird macro. But the parameters are fine. Works this way.

      OSALUTIL_s32SaveVarNPrintFormat(acBuffer, 255, pcBuffer , arg);//lint !e530 : OSAL_VarArgStart IS the initialiser.
      OSAL_VarArgEnd(arg);

      m_poTrace->vTrace( eTraceLevel, TR_CLASS_HSI_TUNER_MSG, "%s", acBuffer);
   }
}
tVoid clTuner_CCAhandler::vTrace(tU16 u16TraceId, hmi_tenTraceLevel enTraceLevel, tU8 u8Len, tU8* pu8TraceData) const
{

   if(m_poTrace != NULL)
   {
      m_poTrace->vTrace(enTraceLevel, TR_CLASS_HSI_TUNER_MSG,
         u16TraceId, u8Len, pu8TraceData);
   }
}
tVoid clTuner_CCAhandler::vTrace( tU16 u16TraceId,hmi_tenTraceLevel enTraceLevel,tU8 u8Len, tS32* ps32TraceData) const
{
   if(m_poTrace != NULL)
   {
      m_poTrace->vTrace(
         enTraceLevel, 
         TR_CLASS_HSI_TUNER_MSG,
         u16TraceId, 
         u8Len, ps32TraceData);
   }
}

/****************************************************************************
*METHOD:      vTrace
*CLASS:       clTuner_CCAhandler
*DESCRIPTION: Private helper method to generate info and error traces.
*PARAMETER:
*             u16TraceId(I):
*                Unique TTFIS trace id
*             enTraceLevel(I):
*                Trace level (TR_LEVEL_HMI_ERROR / _INFO / ...)
*             u8InfoByte1(I):
*                additional trace data
*             u8InfoByte2(I):
*                additional trace data
*RETURNVALUE: tVoid
*HISTORY:
****************************************************************************/
tVoid clTuner_CCAhandler::vTrace
(
 tU16 u16TraceId,
 hmi_tenTraceLevel enTraceLevel,
 tU8 InfoByte1,
 tU8 InfoByte2,
 tU8 InfoByte3
 ) const
{
   tU8 au8TraceData[3];

   au8TraceData[0] = InfoByte1;
   au8TraceData[1] = InfoByte2;
   au8TraceData[2] = InfoByte3;

   vTrace( u16TraceId, enTraceLevel, 3, au8TraceData );
}

/****************************************************************************
*METHOD:      vTrace
*CLASS:       clTuner_CCAhandler
*DESCRIPTION: Private helper method to generate info and error traces.
*PARAMETER:
*             u16TraceId(I):
*                Unique TTFIS trace id
*             enTraceLevel(I):
*                Trace level (TR_LEVEL_HMI_ERROR / _INFO / ...)
*             u8InfoByte1(I):
*                additional trace data
*             u8InfoByte2(I):
*                additional trace data
*             u8Len(I):
*                Lenght (# bytes) of pu8TraceData
*             pu8TraceData(I):
*                additional trace data (default == 0)
*RETURNVALUE: tVoid
*HISTORY:
****************************************************************************/
tVoid clTuner_CCAhandler::vTrace
(
 tU16 u16TraceId,
 hmi_tenTraceLevel enTraceLevel,
 tU8 InfoByte1,
 tU8 InfoByte2,
 tU8 u8Len,
 tU8* pu8TraceData
 ) const
{
   tU8  u8TraceArrayLen = u8Len+(2*sizeof(tU8));
   tU8* pu8TraceArray = OSAL_NEW tU8[u8TraceArrayLen];

   if ( pu8TraceArray != NULL )
   {
      tU8 u8LaufIdx = 0, u8CopyIdx = 0;

      pu8TraceArray[u8LaufIdx++] = InfoByte1;
      pu8TraceArray[u8LaufIdx++] = InfoByte2;

      if ( pu8TraceData != NULL )
      {
         for ( ; u8LaufIdx < u8TraceArrayLen && u8CopyIdx < u8Len; )
         {
            pu8TraceArray[u8LaufIdx++] = pu8TraceData[u8CopyIdx++];
         } 
      }
      else
         /* no user data to copy */
      {
         /* in case of no user data, the output is reduced to 2 bytes */
         u8TraceArrayLen = u8LaufIdx;
      }

      if ( m_poTrace != NULL )
      {
         m_poTrace->vTrace(enTraceLevel, TR_CLASS_HSI_TUNER_MSG,
            u16TraceId, u8TraceArrayLen, pu8TraceArray);
      }

      OSAL_DELETE[] pu8TraceArray;
   }
}


//
// Timer functions
//
tVoid clTuner_CCAhandler::vStartTimer(tU32 u32Timer)
{
   switch(u32Timer)
   {
   case HSI__TUN__ALLOW_TPSEEK_TIMER:
      {
         vTrace(CCA_RADIO_START_ALLOW_TPSEEK_TIMER, TR_LEVEL_HMI_INFO);
         if (poFI!=NULL)
         {
            poFI->SendEventsToEvtEn(SRVENG__IN_RESTART_ALLOW_TPSEEK_TIMER, NULL);
         }
         break;
      }
   default:
      {
         vTrace(CCA_RADIO_START_TIMER_ERROR, TR_LEVEL_HMI_INFO,u32Timer);
      }
      break;
   }
}

tVoid clTuner_CCAhandler::vStopTimer(tU32 u32Timer)
{
   switch(u32Timer)
   {
   case HSI__TUN__ALLOW_TPSEEK_TIMER:
      {
         vTrace(CCA_RADIO_STOP_ALLOW_TPSEEK_TIMER, TR_LEVEL_HMI_INFO);
         if (poFI!=NULL)
         {
            poFI->SendEventsToEvtEn(SRVENG__IN_STOP_ALLOW_TPSEEK_TIMER, NULL);
         }
         break;
      }

   default:
      {
         vTrace(CCA_RADIO_STOP_TIMER_ERROR, TR_LEVEL_HMI_INFO,u32Timer);
      }
      break;
   }
}




tVoid clTuner_CCAhandler::bSetBandFromHMI( tU8 u8Band)
{
   m_u8BandFromHMI = u8Band;
}

tU8 clTuner_CCAhandler::u8GetBandFromHMI()
{
   m_poTrace->vTrace(  TR_LEVEL_HMI_INFO, TR_CLASS_HSI_TUNER_MSG,
      " Band value returned = %d ", m_u8BandFromHMI);
   return m_u8BandFromHMI;
}
