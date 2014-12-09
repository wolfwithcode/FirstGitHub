/******************************************************************************
* Filename          : clHSA_TestMode.cpp
* Project           : Nissan LCN2
* Copyright         : Robert Bosch GmbH, Hildesheim
*------------------------------------------------------------------------------
* Description       : Testmode APIs
*------------------------------------------------------------------------------
* HISTORY
*------------------------------------------------------------------------------
* Date         Author             Modification
*------------------------------------------------------------------------------
* 2007       | CM-DI/PJ-VW34    | Initial Version
*------------------------------------------------------------------------------
* 26-07-2011 | Sushma Manjunath | Added DAB Testmode APIs
*------------------------------------------------------------------------------
* 23-05-2012 | Sushma Manjunath | Added Meter version in API GetVersion()
*------------------------------------------------------------------------------
* 11-03-2013 | Vinod Bhargav B S| NIKAI-3091: Updated method ulwGetUSBMediaStatus()
*                                 NIKAI-3516: Updated method vGetUsbDeviceSrvID()
*------------------------------------------------------------------------------
* 15-03-2013 | Dirk Klein       | Append ODOMeter direction output to Navi speed
*------------------------------------------------------------------------------
* 25-09-2014 | Tapeswar Puhan   | NIKAI2-6447(Unknown medium for MTP devices)
******************************************************************************/

// precompiled header, should always be the first include


#include "precompiled.hh"

#if (OSAL_OS != OSAL_WINNT)
#include "GUI_Widget/util/GUI_UTIL_ScreenShot.h"
#endif


#include "API_Impl/HSA_System/HSA_TestMode/clHSA_TestMode.h"
#include "datapool/clDataPool.h"
static clDataPool dp;
#include "CompMan/clHSI_CMSystem.h"
#include "CompMan/clHSI_TestMode.h"

#include "fiVehicle/clVehicle_CCAhandler.h"
#include "fiPhone/HSI_TEL_DEFINES.h"
#include "fiaudio/HSI_AUD_SM_EVENTS.h"
#include "fiTunerMaster/HSI_TUNMSTR_DEFINES.h"
#include "fiVdSensor/clVDS_CCAhandler.h"
#include "cmb_fi_if.h"

/* Include for traces */
#include "API_Impl/HSA_System/HSA_TestMode/HSA_System_TestMode_API_Trace.h"

#define REG_S_IMPORT_INTERFACE_GENERIC
#include "reg_if.h"


#define GUI_TESTMODE_BT_DEV_TEST_TYPE_TXDATA1 0
#define GUI_TESTMODE_BT_DEV_TEST_TYPE_TXDATA2 1
#define GUI_TESTMODE_BT_DEV_TEST_TYPE_RXSTART1_FIXED_CH 2
#define GUI_TESTMODE_BT_DEV_TEST_TYPE_TXSTART 4

#define TUN_FM_START_FRQ    87500
#define TUN_FM_END_FRQ      108000
#define TUN_AM_START_FRQ    522
#define TUN_AM_END_FRQ      1611
#define SPELLER_DELETE_CODE "\xef\xA0\x88"
#define SPELLER_CHAR_SIZE_DAB      4
/******************** do NOT change - Start ********************************/
tVoid clHSA_TestMode::createInstance()
{       
   if (sm_pInstance == NULL)
   {
      sm_pInstance = OSAL_NEW clHSA_TestMode(EN_HSA_SYSTEM_TESTMODE);			 
   }
   else
   {
      // maybe some warning here?
   }
}

clHSA_TestMode::clHSA_TestMode(T_EN_HSA_ID en_HSA_ID):clHSA_System_TestMode_Base(en_HSA_ID)
{
   u8TrackAFList = 0;
   u8TrackTMCList = 0;
   u8ActiveTuner = 0;	
   bReqTunerCalData = false;
   pclHSI_TestMode = NULL;
   pclHSI_Radio = NULL;
   pclHSI_System = NULL;
   pclHSI_CMPhone = NULL;
   pclHSI_CMMeter = NULL;
   pclHSI_CMDAB = NULL;
   pclHSI_TunerMaster = NULL;
   pclHSI_CMSXM = NULL;
   pclHSI_CMSmartPhone = NULL;
   pclHSI_CMTCU = NULL;
   m_tU8BTDevActiveTest = NULL;
   pclHSI_Sound= NULL;
}

tVoid clHSA_TestMode::destroyInstance()
{
   if (sm_pInstance != NULL)
   {
      OSAL_DELETE(sm_pInstance);		
   }
}

/*!***********************************************************************
* METHOD:			bAfterConfigure 
* CLASS:			   clHSA_TestMode
* DESCRIPTION:    method to be called by HSA_Manager in early startup
* PARAMETER:     
* RETURNVALUE:
*					TRUE	: the configuration is valid 
*					FALSE	: a configuration error has occured
*************************************************************************/
tBool clHSA_TestMode::bAfterConfigure()
{
   clHSI_CMMngr*     pclHSI_CMManager = NULL;
   pclHSI_CMManager = clHSI_CMMngr::pclGetInstance();

   if( NULL != pclHSI_CMManager)
   {
      pclHSI_TestMode = dynamic_cast <clHSI_TestMode*>(pclHSI_CMManager->pHSI_BaseGet(EN_HSI_SYSTEM_TESTMODE));
      pclHSI_Radio = dynamic_cast <clHSI_Radio*>(pclHSI_CMManager->pHSI_BaseGet(EN_HSI_RADIO));
      pclHSI_System = dynamic_cast <clHSI_CMSystem*>(pclHSI_CMManager->pHSI_BaseGet(EN_HSI_CMSYSTEM));
      pclHSI_CMPhone = dynamic_cast <clHSI_CMPhone*>(pclHSI_CMManager->pHSI_BaseGet(EN_HSI_PHONE));
      pclHSI_CMMeter = dynamic_cast <clHSI_CMMeter*>(pclHSI_CMManager->pHSI_BaseGet(EN_HSI_METER));
      pclHSI_CMDAB = dynamic_cast <clHSI_CMDAB*>(pclHSI_CMManager->pHSI_BaseGet(EN_HSI_CMDAB));
      pclHSI_TunerMaster = dynamic_cast <clHSI_TunerMaster*>(pclHSI_CMManager->pHSI_BaseGet(EN_HSI_TUNERMASTER));
      pclHSI_CMSmartPhone = dynamic_cast <clHSI_CMSmartPhone*>(pclHSI_CMManager->pHSI_BaseGet(EN_HSI_SMART_PHONE));
	  pclHSI_CMTCU = dynamic_cast <clHSI_CMTCU*>(pclHSI_CMManager->pHSI_BaseGet(EN_HSI_TCU));
      pclHSI_CMSXM =  dynamic_cast <clHSI_CMSXM*>(pclHSI_CMManager->pHSI_BaseGet(EN_HSI_SXM));
      pclHSI_Sound = dynamic_cast <clHSI_Sound*>(pclHSI_CMManager->pHSI_BaseGet(EN_HSI_SOUND));
   }

   return true;
}


/******************** do NOT change - End **********************************/

/****************************************************************************
* you may take the function definitions from the clHSA_TestMode_Base class file
* for example:
* tVoid clHSA_TestMode::vFunctionName(tU32 param)
* {
*	//@ToDo: Developers implementation goes here...
* }
*
*  now add your code below
****************************************************************************/
/****************************DAB APIs***************************************/
/******************************************************************************
* FUNCTION:    clHSA_TestMode::vSetDABSFMode()
*------------------------------------------------------------------------------
* DESCRIPTION: This API is called to set the SF mode selected by the user
*------------------------------------------------------------------------------
* PARAMETER:   ulword ServiceLinkingMode
* RETURNVALUE: Void
******************************************************************************/
void clHSA_TestMode::vSetDABSFMode(ulword ServiceLinkingMode)
{
   if( NULL != pclHSI_CMDAB )
   {
      sMsg refMsg(NULL, CMMSG_DAB_SETUP, (tU32)FEA_SERVICE_LINKING, ServiceLinkingMode);
      pclHSI_CMDAB->bExecuteMessage( refMsg );
   }
}

/******************************************************************************
* FUNCTION:    clHSA_TestMode::ulwGetDABSFModeSelectionStatus()
*------------------------------------------------------------------------------
* DESCRIPTION: This API is called to get the SF mode selected by the user
*------------------------------------------------------------------------------
* PARAMETER:   Void
* RETURNVALUE: ulword
******************************************************************************/
ulword clHSA_TestMode::ulwGetDABSFModeSelectionStatus()
{
   tU8 u8SrvLinkingMode = 0;
   if(NULL != m_poDataPool)
   {		
      m_poDataPool->u32dp_get(DPDAB__TM_SERVICE_LINKING_MODE, 
         &u8SrvLinkingMode, sizeof(u8SrvLinkingMode) );
   }
   return (ulword)u8SrvLinkingMode;	
}

/******************************************************************************
* FUNCTION:    clHSA_TestMode::vSetDABConcealmentLevel()
*------------------------------------------------------------------------------
* DESCRIPTION: This API is called to increment or decrement the Concealment
*              level selected by the user
*------------------------------------------------------------------------------
* PARAMETER:   tbool Direction
* RETURNVALUE: Void
******************************************************************************/
void clHSA_TestMode::vSetDABConcealmentLevel(tbool Direction)
{
   tU8 u8ConcealmentLevel_set = 0;

   if(NULL != m_poDataPool)
   {
      m_poDataPool->u32dp_get(DPDAB__TM_CONCEALMENT_LEVEL,
         &u8ConcealmentLevel_set, sizeof(u8ConcealmentLevel_set) );
   }
   if(Direction)
      u8ConcealmentLevel_set++;
   else
      u8ConcealmentLevel_set--;

   if( NULL != pclHSI_CMDAB )
   {
      sMsg refMsg(NULL, CMMSG_DAB_SETUP, (tU32)FEA_CONCEALMENT_LEVEL,(ulword) u8ConcealmentLevel_set);
      pclHSI_CMDAB->bExecuteMessage( refMsg );
   }
}

/******************************************************************************
* FUNCTION:    clHSA_TestMode::ulwGetDABConcealmentLevel()
*------------------------------------------------------------------------------
* DESCRIPTION: This API is called to get the Concealment level selected by the user
*------------------------------------------------------------------------------
* PARAMETER:   Void
* RETURNVALUE: ulword
******************************************************************************/
ulword clHSA_TestMode::ulwGetDABConcealmentLevel()
{
   tU8 u8ConcealmentLevel = 0;
   if(NULL != m_poDataPool)
   {		
      m_poDataPool->u32dp_get(DPDAB__TM_CONCEALMENT_LEVEL, 
         &u8ConcealmentLevel, sizeof(u8ConcealmentLevel) );
   }
   return (ulword)u8ConcealmentLevel;	
}

/******************************************************************************
* FUNCTION:    clHSA_TestMode::ulwGetDABSourceState()
*------------------------------------------------------------------------------
* DESCRIPTION: This API is called to get the Source state for indicator in TM screen
*------------------------------------------------------------------------------
* PARAMETER:   Void
* RETURNVALUE: ulword
******************************************************************************/
ulword clHSA_TestMode::ulwGetDABSourceState()
{
   tU8 u8SourceState = 0;
   if(NULL != m_poDataPool)
   {		
      m_poDataPool->u32dp_get(DPDAB__TM_SOURCE_STATE, 
         &u8SourceState, sizeof(u8SourceState) );
   }
   return (ulword)u8SourceState;	
}

/******************************************************************************
* FUNCTION:    clHSA_TestMode::vGetDABChannelNumber()
*------------------------------------------------------------------------------
* DESCRIPTION: This API is called to get the Channel Label
*------------------------------------------------------------------------------
* PARAMETER:   Void
* RETURNVALUE: GUI_String
******************************************************************************/
void clHSA_TestMode::vGetDABChannelNumber(GUI_String *out_result)
{
   tU32 u32Size=0;
   tChar* pacText = OSAL_NEW tChar[MAX_GPS_FORMAT_STRLEN];   

   if(( NULL != pacText) && (NULL != m_poDataPool))
   {
      u32Size = m_poDataPool->u32dp_getElementSize(DPDAB__TM_CHANNEL_NUM);
      OSAL_pvMemorySet((tVoid*)pacText,NULL,MAX_GPS_FORMAT_STRLEN);

      m_poDataPool->u32dp_get( DPDAB__TM_CHANNEL_NUM,
         (tVoid*)pacText, 
         u32Size );
   }
   //Return Channel Label	
   GUI_String_vSetCStr(out_result, (tU8*)pacText);
   OSAL_DELETE[] pacText;
}
/******************************************************************************
* FUNCTION:    clHSA_TestMode::vGetDABEnsembleID()
*------------------------------------------------------------------------------
* DESCRIPTION: This API is called to get the DAB ensemble id in Hex
*------------------------------------------------------------------------------
* PARAMETER:   ulword, GUI_String
* RETURNVALUE:
******************************************************************************/
void clHSA_TestMode::vGetDABEnsembleID(GUI_String *out_result, ulword DABTuner)
{
   tU32 u32EnsembleID=0;
   if (DABTuner)
   {
      if(NULL != m_poDataPool)
         m_poDataPool->u32dp_get(DPDAB__TM_ENSEMBLE_ID_BG,
         &u32EnsembleID, sizeof(u32EnsembleID) );
   }
   else
   {
      if(NULL != m_poDataPool)
      {
         m_poDataPool->u32dp_get(DPDAB__TM_ENSEMBLE_ID,
            &u32EnsembleID, sizeof(u32EnsembleID) );
      }
   }
   tChar pacEnsembleID[ MAX_BUFFER_STRLEN ]  = "\0";

   UTF8_s32SaveNPrintFormat( pacEnsembleID,MAX_BUFFER_STRLEN,
      "%x",u32EnsembleID);

   /** Return the String*/
   GUI_String_vSetCStr(out_result, (tU8*)pacEnsembleID);
}
/******************************************************************************
* FUNCTION:    clHSA_TestMode::ulwGetDABNumOfAudServices()
*------------------------------------------------------------------------------
* DESCRIPTION: This API is called to get the number of audio services
*------------------------------------------------------------------------------
* PARAMETER:   Void
* RETURNVALUE: ulword
******************************************************************************/
ulword clHSA_TestMode::ulwGetDABNumOfAudServices()
{
   tU8 u8AudServices = 0;
   if(NULL != m_poDataPool)
   {
      m_poDataPool->u32dp_get(DPDAB__TM_AUDSERVICES,
         &u8AudServices, sizeof(u8AudServices) );
   }
   return (ulword)u8AudServices;
}

/******************************************************************************
* FUNCTION:    clHSA_TestMode::ulwGetDABNumOfDataServices()
*------------------------------------------------------------------------------
* DESCRIPTION: This API is called to get the number of data services
*------------------------------------------------------------------------------
* PARAMETER:   Void
* RETURNVALUE: ulword
******************************************************************************/
ulword clHSA_TestMode::ulwGetDABNumOfDataServices()
{
   tU8 u8DataServices = 0;
   if(NULL != m_poDataPool)
   {
      m_poDataPool->u32dp_get(DPDAB__TM_DATA_SERVICES,
         &u8DataServices, sizeof(u8DataServices) );
   }
   return (ulword)u8DataServices;
}
/******************************************************************************
* FUNCTION:    clHSA_TestMode::ulwGetDABNumOfAudSerComp()
*------------------------------------------------------------------------------
* DESCRIPTION: This API is called to get the number of audio service components.
*------------------------------------------------------------------------------
* PARAMETER:   Void
* RETURNVALUE: ulword
******************************************************************************/
ulword clHSA_TestMode::ulwGetDABNumOfAudSerComp()
{
   tU8 u8Audio_Service_Comp= 0;
   if(NULL != m_poDataPool)
   {
      m_poDataPool->u32dp_get(DPDAB__TM_AUDIO_SER_COMP,
         &u8Audio_Service_Comp, sizeof(u8Audio_Service_Comp) );
   }
   return (ulword)u8Audio_Service_Comp;
}
/******************************************************************************
* FUNCTION:    clHSA_TestMode::ulwGetDABNumOfDataSerComp()
*------------------------------------------------------------------------------
* DESCRIPTION: This API is called to get the number of data service components.
*------------------------------------------------------------------------------
* PARAMETER:   Void
* RETURNVALUE: ulword
******************************************************************************/
ulword clHSA_TestMode::ulwGetDABNumOfDataSerComp()
{
   tU8 u8Data_Service_Comp= 0;
   if(NULL != m_poDataPool)
   {
      m_poDataPool->u32dp_get(DPDAB__TM_DATA_SER_COMP,
         &u8Data_Service_Comp, sizeof(u8Data_Service_Comp) );
   }
   return (ulword)u8Data_Service_Comp;
}
/******************************************************************************
* FUNCTION:    clHSA_TestMode::vGetDABServiceID()
*------------------------------------------------------------------------------
* DESCRIPTION: This API is called to get the DAB service ID in Hex
*------------------------------------------------------------------------------
* PARAMETER:   GUI_String
* RETURNVALUE: void
******************************************************************************/
void clHSA_TestMode::vGetDABServiceID(GUI_String *out_result)
{
   tU32 u32DABServiceID = 0;
   if(NULL != m_poDataPool)
   {
      m_poDataPool->u32dp_get(DPDAB__TM_SERVICE_ID,
         &u32DABServiceID, sizeof(u32DABServiceID) );
   }
   tChar pacServiceID[ MAX_BUFFER_STRLEN ]  = "\0";

   UTF8_s32SaveNPrintFormat( pacServiceID,MAX_BUFFER_STRLEN,
      "%x",u32DABServiceID);

   /** Return the String*/
   GUI_String_vSetCStr(out_result, (tU8*)pacServiceID);
}

/******************************************************************************
* FUNCTION:    clHSA_TestMode::ulwGetDABAudioBitRate()
*------------------------------------------------------------------------------
* DESCRIPTION: This API is called to get the audio bit rate.
*------------------------------------------------------------------------------
* PARAMETER:   Void
* RETURNVALUE: ulword
******************************************************************************/
ulword clHSA_TestMode::ulwGetDABAudioBitRate()
{
   tU16 u16AudioBitRate = 0;
   if(NULL != m_poDataPool)
   {
      m_poDataPool->u32dp_get(DPDAB__TM_AUDIO_BIT_RATE,
         &u16AudioBitRate, sizeof(u16AudioBitRate) );
   }
   return (ulword)u16AudioBitRate;
}
/******************************************************************************
* FUNCTION:    clHSA_TestMode::ulwGetDABSamplingRate()
*------------------------------------------------------------------------------
* DESCRIPTION: This API is called to get the Sampling rate
*------------------------------------------------------------------------------
* PARAMETER:   Void
* RETURNVALUE: ulword
******************************************************************************/
ulword clHSA_TestMode::ulwGetDABSamplingRate()
{
   tU8 u8SamplingRate = 0;
   if(NULL != m_poDataPool)
   {
      m_poDataPool->u32dp_get(DPDAB__TM_SAMPLING_RATE,
         &u8SamplingRate, sizeof(u8SamplingRate) );
   }
   return (ulword)u8SamplingRate;
}

/******************************************************************************
* FUNCTION:    clHSA_TestMode::vGetDABAAC(GUI_String *out_result)
*------------------------------------------------------------------------------
* DESCRIPTION: This API is called to get the advanced audio codec.
*------------------------------------------------------------------------------
* PARAMETER:   GUI_String
* RETURNVALUE: Void
******************************************************************************/
void clHSA_TestMode::vGetDABAAC(GUI_String *out_result)
{
   tU32 u32Size_DABAAC = 0;
   tChar* pacText_DABAAC = OSAL_NEW tChar[MAX_GPS_FORMAT_STRLEN];

   if(( NULL != pacText_DABAAC) && (NULL != m_poDataPool))
   {
      u32Size_DABAAC = m_poDataPool->u32dp_getElementSize(DPDAB__TM_ADVANCED_AUDIO_CODEC);
      OSAL_pvMemorySet((tVoid*)pacText_DABAAC,NULL,MAX_GPS_FORMAT_STRLEN);

      m_poDataPool->u32dp_get( DPDAB__TM_ADVANCED_AUDIO_CODEC,
         (tVoid*)pacText_DABAAC,
         u32Size_DABAAC );
   }
   //Return Advanced Audio Codec
   GUI_String_vSetCStr(out_result, (tU8*)pacText_DABAAC);
   OSAL_DELETE[] pacText_DABAAC;
}
/******************************************************************************
* FUNCTION:    clHSA_TestMode::vGetDABAudioCodec(GUI_String *out_result)
*------------------------------------------------------------------------------
* DESCRIPTION: This API is called to get the audio codec.
*------------------------------------------------------------------------------
* PARAMETER:   GUI_String
* RETURNVALUE: Void
******************************************************************************/
void clHSA_TestMode::vGetDABAudioCodec(GUI_String *out_result)
{
   tU32 u32Size_DABAC=0;
   tChar* pacText_DABAC = OSAL_NEW tChar[MAX_GPS_FORMAT_STRLEN];

   if(( NULL != pacText_DABAC) && (NULL != m_poDataPool))
   {
      u32Size_DABAC = m_poDataPool->u32dp_getElementSize(DPDAB__TM_AUDIO_CODEC);
      OSAL_pvMemorySet((tVoid*)pacText_DABAC,NULL,MAX_GPS_FORMAT_STRLEN);

      m_poDataPool->u32dp_get( DPDAB__TM_AUDIO_CODEC,
         (tVoid*)pacText_DABAC,
         u32Size_DABAC );
   }
   //Return Audio Codec
   GUI_String_vSetCStr(out_result, (tU8*)pacText_DABAC);
   OSAL_DELETE[] pacText_DABAC;
}
/******************************************************************************
* FUNCTION:    clHSA_TestMode::vGetDABServiceLabel(GUI_String *out_result)
*------------------------------------------------------------------------------
* DESCRIPTION: This API is called to get the service label
*------------------------------------------------------------------------------
* PARAMETER:   GUI_String
* RETURNVALUE: Void
******************************************************************************/
void clHSA_TestMode::vGetDABServiceLabel(GUI_String *out_result)
{
   tU32 u32SizeServicelabel = 0;
   tChar* pacText_Servicelabel = OSAL_NEW tChar[LABEL_LEN];

   if(( NULL != pacText_Servicelabel) && (NULL != m_poDataPool))
   {
      u32SizeServicelabel = m_poDataPool->u32dp_getElementSize(DPDAB__TM_SERVICE_LABEL);
      OSAL_pvMemorySet((tVoid*)pacText_Servicelabel,NULL,LABEL_LEN);

      m_poDataPool->u32dp_get( DPDAB__TM_SERVICE_LABEL,
    		  (tVoid*)pacText_Servicelabel,
         u32SizeServicelabel );
   }
   //Return Service Label
   GUI_String_vSetCStr(out_result, (tU8*)pacText_Servicelabel);
   OSAL_DELETE[] pacText_Servicelabel;
}

/*****************************************************************************
* FUNCTION:    clHSA_TestMode::vGetDABEnsembleLabel(GUI_String *out_result)
*------------------------------------------------------------------------------
* DESCRIPTION: This API is called to get the ensemble label
*------------------------------------------------------------------------------
* PARAMETER:   GUI_String
* RETURNVALUE: Void
******************************************************************************/
void clHSA_TestMode::vGetDABEnsembleLabel(GUI_String *out_result)
{
   tU32 u32EnsembleLabelSize = 0;
   tChar* pacText_Ensemblelabel = OSAL_NEW tChar[LABEL_LEN];

   if(( NULL != pacText_Ensemblelabel) && (NULL != m_poDataPool))
   {
      u32EnsembleLabelSize = m_poDataPool->u32dp_getElementSize(DPDAB__TM_ENSEMBLE_LABEL);
      OSAL_pvMemorySet((tVoid*)pacText_Ensemblelabel, NULL, LABEL_LEN);

      m_poDataPool->u32dp_get( DPDAB__TM_ENSEMBLE_LABEL,
         (tVoid*)pacText_Ensemblelabel,
         u32EnsembleLabelSize );
   }
   //Return Ensemble Label
   GUI_String_vSetCStr(out_result, (tU8*)pacText_Ensemblelabel);
   OSAL_DELETE[] pacText_Ensemblelabel;
}
/*****************************************************************************
* FUNCTION:    clHSA_TestMode::vGetDABStereoMode(GUI_String *out_result)
*------------------------------------------------------------------------------
* DESCRIPTION: This API is called to get the stereo/audio mode
*------------------------------------------------------------------------------
* PARAMETER:   GUI_String
* RETURNVALUE: Void
******************************************************************************/
void clHSA_TestMode::vGetDABStereoMode(GUI_String *out_result)
{
   tU32 u32StereoModeSize = 0;
   tChar* pacText_StereoMode = OSAL_NEW tChar[MAX_GPS_FORMAT_STRLEN];

   if(( NULL != pacText_StereoMode) && (NULL != m_poDataPool))
   {
      u32StereoModeSize = m_poDataPool->u32dp_getElementSize(DPDAB__TM_AUDIO_MODE);
      OSAL_pvMemorySet((tVoid*)pacText_StereoMode, NULL, MAX_GPS_FORMAT_STRLEN);

      m_poDataPool->u32dp_get( DPDAB__TM_AUDIO_MODE,
         (tVoid*)pacText_StereoMode,
         u32StereoModeSize );
   }
   //Return Stereo Mode
   GUI_String_vSetCStr(out_result, (tU8*)pacText_StereoMode);
   OSAL_DELETE[] pacText_StereoMode;
}

/*****************************************************************************
* FUNCTION:    clHSA_TestMode::vGetDABShortEnsembleLabel(GUI_String *out_result)
*------------------------------------------------------------------------------
* DESCRIPTION: This API is called to get the short ensemble label
*------------------------------------------------------------------------------
* PARAMETER:   GUI_String
* RETURNVALUE: Void
******************************************************************************/
void clHSA_TestMode::vGetDABShortEnsembleLabel(GUI_String *out_result)
{
   tU32 u32ShortEnsembleLabelSize = 0;
   tChar* pacText_ShortEnsemblelabel = OSAL_NEW tChar[MAX_GPS_FORMAT_STRLEN];

   if(( NULL != pacText_ShortEnsemblelabel) && (NULL != m_poDataPool))
   {
      u32ShortEnsembleLabelSize = m_poDataPool->u32dp_getElementSize(DPDAB__TM_SHORT_ENSEMBLE_LABEL);
      OSAL_pvMemorySet((tVoid*)pacText_ShortEnsemblelabel, NULL, MAX_GPS_FORMAT_STRLEN);

      m_poDataPool->u32dp_get( DPDAB__TM_SHORT_ENSEMBLE_LABEL,
         (tVoid*)pacText_ShortEnsemblelabel,
         u32ShortEnsembleLabelSize );
   }
   //Return Short Ensemble Label
   GUI_String_vSetCStr(out_result, (tU8*)pacText_ShortEnsemblelabel);
   OSAL_DELETE[] pacText_ShortEnsemblelabel;
}

/******************************************************************************
* FUNCTION:    clHSA_TestMode::vGetDABShortServiceLabel(GUI_String *out_result)
*------------------------------------------------------------------------------
* DESCRIPTION: This API is called to get the short service label
*------------------------------------------------------------------------------
* PARAMETER:   GUI_String
* RETURNVALUE: Void
******************************************************************************/ 
void clHSA_TestMode::vGetDABShortServiceLabel(GUI_String *out_result)
{
   tU32 u32ShortServicelabelSize = 0;
   tChar* pacText_ShortServicelabel = OSAL_NEW tChar[MAX_GPS_FORMAT_STRLEN];

   if(( NULL != pacText_ShortServicelabel) && (NULL != m_poDataPool))
   {
      u32ShortServicelabelSize = m_poDataPool->u32dp_getElementSize(DPDAB__TM_SHORT_SERVICE_LABEL);
      OSAL_pvMemorySet((tVoid*)pacText_ShortServicelabel, NULL, MAX_GPS_FORMAT_STRLEN);

      m_poDataPool->u32dp_get( DPDAB__TM_SHORT_SERVICE_LABEL,
         (tVoid*)pacText_ShortServicelabel,
         u32ShortServicelabelSize );
   }

   //Return short service label
   GUI_String_vSetCStr(out_result, (tU8*)pacText_ShortServicelabel);
   OSAL_DELETE[] pacText_ShortServicelabel;
}

/******************************************************************************
* FUNCTION:    clHSA_TestMode::blGetDABDRC()
*------------------------------------------------------------------------------
* DESCRIPTION: This API is called to get dynamic compression support info
*------------------------------------------------------------------------------
* PARAMETER:   Void
* RETURNVALUE: tbool
******************************************************************************/
tbool clHSA_TestMode::blGetDABDRC()
{
   tBool bDABDRC = FALSE;

   if(NULL != m_poDataPool)
   {
      m_poDataPool->u32dp_get(DPDAB__TM_DYNAMIC_COMPRESSION,
         &bDABDRC, sizeof(bDABDRC));

   }
   return bDABDRC;
}
/******************************************************************************
* FUNCTION:    clHSA_TestMode::blGetDAB_PorS_Info()
*------------------------------------------------------------------------------
* DESCRIPTION: This API is called to know wheather active service is primary 
*              or secondary
*------------------------------------------------------------------------------
* PARAMETER:   Void
* RETURNVALUE: tbool
******************************************************************************/
tbool clHSA_TestMode::blGetDAB_PorS_Info()
{
   tBool bIsPrimaryOrSec = FALSE;

   if(NULL != m_poDataPool)
   {
      m_poDataPool->u32dp_get(DPDAB__TM_SECONDARY_COMPONENT,
         &bIsPrimaryOrSec, sizeof(bIsPrimaryOrSec));

   }
   /*The values expected by GUI for primary/secondary is compliment of what is received from fc_Dab
   fc_dab gives 0 = Primary, 1 = Secondary
   GUI requires 0 = Seconday, 1 = Primary*/

   return (!bIsPrimaryOrSec);
}
/******************************************************************************
* FUNCTION:    clHSA_TestMode::blGetDABTMCSupportStatus()
*------------------------------------------------------------------------------
* DESCRIPTION: This API is called to know wheather the station supports TMC
*------------------------------------------------------------------------------
* PARAMETER:   Void
* RETURNVALUE: tbool
******************************************************************************/
tbool clHSA_TestMode::blGetDABTMCSupportStatus()
{
   tBool bTMC = FALSE;

   if(NULL != m_poDataPool)
   {
      m_poDataPool->u32dp_get(DPDAB__TM_TMC_COMPONENT,
         &bTMC, sizeof(bTMC));
   }
   return bTMC;
}

/******************************************************************************
* FUNCTION:    clHSA_TestMode::vGetDABSrvCompID()
*------------------------------------------------------------------------------
* DESCRIPTION: This API is called to get the Service Component ID in Hex
*------------------------------------------------------------------------------
* PARAMETER:   GUI_String
* RETURNVALUE: Void
******************************************************************************/
void clHSA_TestMode::vGetDABSrvCompID(GUI_String *out_result)
{
   tU16 u16SrvCompID = 0;
   if(NULL != m_poDataPool)
   {
      m_poDataPool->u32dp_get(DPDAB__TM_SERVICE_COMPONENT_ID,
         &u16SrvCompID, sizeof(u16SrvCompID) );
   }
   tChar pacSrvCompID[ MAX_BUFFER_STRLEN ]  = "\0";

   UTF8_s32SaveNPrintFormat( pacSrvCompID,MAX_BUFFER_STRLEN,
      "%x",u16SrvCompID);

   /** Return the String*/
   GUI_String_vSetCStr(out_result, (tU8*)pacSrvCompID);
}
/******************************************************************************
* FUNCTION:    clHSA_TestMode::vGetDABSrvCompLabel
*------------------------------------------------------------------------------
* DESCRIPTION: This API is called to get the service component label
*------------------------------------------------------------------------------
* PARAMETER:GUI_String
* RETURNVALUE: Void
******************************************************************************/
void clHSA_TestMode::vGetDABSrvCompLabel(GUI_String *out_result)
{
   tU32 u32SizeServiceCompLabel =0;
   tChar* pacText_ServiceCompLabel = OSAL_NEW tChar[LABEL_LEN];

   if(( NULL != pacText_ServiceCompLabel) && (NULL != m_poDataPool))
   {
      u32SizeServiceCompLabel = m_poDataPool->u32dp_getElementSize(DPDAB__TM_SERVICE_COMPONENT_LABEL);
      OSAL_pvMemorySet((tVoid*)pacText_ServiceCompLabel,NULL,LABEL_LEN);

      m_poDataPool->u32dp_get( DPDAB__TM_SERVICE_COMPONENT_LABEL,
         (tVoid*)pacText_ServiceCompLabel,
         u32SizeServiceCompLabel );
   }
   //Return service component label
   GUI_String_vSetCStr(out_result, (tU8*)pacText_ServiceCompLabel);
   OSAL_DELETE[] pacText_ServiceCompLabel;
}
/******************************************************************************
* FUNCTION:    clHSA_TestMode::vGetDABShortSrvCompLabel
*------------------------------------------------------------------------------
* DESCRIPTION: This API is called to get the short service component label
*------------------------------------------------------------------------------
* PARAMETER:GUI_String
* RETURNVALUE: Void
******************************************************************************/
void clHSA_TestMode::vGetDABShortSrvCompLabel(GUI_String *out_result)
{
   tU32 u32SizeShortServiceCompLabel =0;
   tChar* pacText_ShortServiceCompLabel = OSAL_NEW tChar[MAX_GPS_FORMAT_STRLEN];

   if(( NULL != pacText_ShortServiceCompLabel) && (NULL != m_poDataPool))
   {
      u32SizeShortServiceCompLabel = m_poDataPool->u32dp_getElementSize(DPDAB__TM_SHORT_SERVICE_COMPONENT_LABEL);
      OSAL_pvMemorySet((tVoid*)pacText_ShortServiceCompLabel,NULL,MAX_GPS_FORMAT_STRLEN);

      m_poDataPool->u32dp_get( DPDAB__TM_SHORT_SERVICE_COMPONENT_LABEL,
         (tVoid*)pacText_ShortServiceCompLabel,
         u32SizeShortServiceCompLabel );
   }
   //Return short service component label
   GUI_String_vSetCStr(out_result, (tU8*)pacText_ShortServiceCompLabel);
   OSAL_DELETE[] pacText_ShortServiceCompLabel;
}
/******************************************************************************
* FUNCTION:    clHSA_TestMode::vGetDABAudioDataSerComType()
*------------------------------------------------------------------------------
* DESCRIPTION: This API is called to get the DAB Audio Service Type in Hex
*------------------------------------------------------------------------------
* PARAMETER:   GUI_String 
* RETURNVALUE: Void
******************************************************************************/
void clHSA_TestMode::vGetDABAudioDataSerComType(GUI_String *out_result)
{
   tU8 u8AudioDataSerComType = 0;
   if(NULL != m_poDataPool)
   {
      m_poDataPool->u32dp_get(DPDAB__TM_AUDIO_DATA_SERVICE_FLAG,
         &u8AudioDataSerComType, sizeof(u8AudioDataSerComType) );
   }
   tChar pacAudioDataSerComType[ MAX_BUFFER_STRLEN ]  = "\0";

   UTF8_s32SaveNPrintFormat( pacAudioDataSerComType,MAX_BUFFER_STRLEN,
      "%x",u8AudioDataSerComType);

   /** Return the String*/
   GUI_String_vSetCStr(out_result, (tU8*)pacAudioDataSerComType);
}
/******************************************************************************
* FUNCTION:    clHSA_TestMode::ulwGetDABTransportMechanismID()
*------------------------------------------------------------------------------
* DESCRIPTION: This API is called to get the DAB Transport Mechanism ID
*------------------------------------------------------------------------------
* PARAMETER:   Void
* RETURNVALUE: ulword
******************************************************************************/
ulword clHSA_TestMode::ulwGetDABTransportMechanismID()
{
   tU8 u8TranspMechID = 0;
   if(NULL != m_poDataPool)
   {
      m_poDataPool->u32dp_get(DPDAB__TM_TRANSPORT_MECHANISM_ID,
         &u8TranspMechID, sizeof(u8TranspMechID) );
   }
   return (ulword)u8TranspMechID;
}
/******************************************************************************
* FUNCTION:    clHSA_TestMode::blGetDAB_PorD_Flag()
*------------------------------------------------------------------------------
* DESCRIPTION: This API is called to get the Program or Data flag
*------------------------------------------------------------------------------
* PARAMETER:   Void
* RETURNVALUE: tbool
******************************************************************************/
tbool clHSA_TestMode::blGetDAB_PorD_Flag()
{
   tbool ubDAB_PorD_Flag = FALSE;

   if(NULL != m_poDataPool)
   {
      m_poDataPool->u32dp_get(DPDAB__TM_PROGRAM_DATA_FLAG,
         &ubDAB_PorD_Flag, sizeof(ubDAB_PorD_Flag));

   }
   return ubDAB_PorD_Flag;
}
/******************************************************************************
* FUNCTION:    clHSA_TestMode::blGetDABTPSupport()
*------------------------------------------------------------------------------
* DESCRIPTION: This API is called to get the TP Support flag
*------------------------------------------------------------------------------
* PARAMETER:   Void
* RETURNVALUE: tbool
******************************************************************************/
tbool clHSA_TestMode::blGetDABTPSupport()
{
   tbool ubDAB_TP_Support = FALSE;

   if(NULL != m_poDataPool)
   {
      m_poDataPool->u32dp_get(DPDAB__TM_TP_SUPPORT,
         &ubDAB_TP_Support, sizeof(ubDAB_TP_Support));

   }
   return ubDAB_TP_Support;
}
/******************************************************************************
* FUNCTION:    clHSA_TestMode::vGetDABAnnoSupport()
*------------------------------------------------------------------------------
* DESCRIPTION: This API is called to get the DAB Announcement Support in Hex
*------------------------------------------------------------------------------
* PARAMETER:   GUI_String 
* RETURNVALUE: Void
******************************************************************************/
void clHSA_TestMode::vGetDABAnnoSupport(GUI_String *out_result)
{
   tU16 u16AnnoSupport = 0;
   if(NULL != m_poDataPool)
   {
      m_poDataPool->u32dp_get(DPDAB__TM_ANNO_SUPPORT,
         &u16AnnoSupport, sizeof(u16AnnoSupport) );
   }
   tChar pacAnnoSupport[ MAX_BUFFER_STRLEN ]  = "\0";

   UTF8_s32SaveNPrintFormat( pacAnnoSupport,MAX_BUFFER_STRLEN,
      "%x",u16AnnoSupport);

   //Return the String
   GUI_String_vSetCStr(out_result, (tU8*)pacAnnoSupport);

}
/******************************************************************************
* FUNCTION:    clHSA_TestMode::vGetDABAnnoSwitchMask()
*------------------------------------------------------------------------------
* DESCRIPTION: This API is called to get the DAB Announcement Switch Mask
*------------------------------------------------------------------------------
* PARAMETER:   GUI_String 
* RETURNVALUE: Void
******************************************************************************/
void clHSA_TestMode::vGetDABAnnoSwitchMask(GUI_String *out_result)
{
   tU16 u16AnnoSwitchMask = 0;
   if(NULL != m_poDataPool)
   {
      m_poDataPool->u32dp_get(DPDAB__TM_ANNO_SWITCH_MASK,
         &u16AnnoSwitchMask, sizeof(u16AnnoSwitchMask) );
   }
   tChar pacAnnoSwitchMask[ MAX_BUFFER_STRLEN ]  = "\0";

   UTF8_s32SaveNPrintFormat( pacAnnoSwitchMask,MAX_BUFFER_STRLEN,
      "%x",u16AnnoSwitchMask);

   /** Return the String*/
   GUI_String_vSetCStr(out_result, (tU8*)pacAnnoSwitchMask);
}

/******************************************************************************
* FUNCTION:    clHSA_TestMode::vGetDABFrequencyTable()
*------------------------------------------------------------------------------
* DESCRIPTION: This API is called to get the Frequency Table
*------------------------------------------------------------------------------
* PARAMETER:   Void
* RETURNVALUE: GUI_String
******************************************************************************/
void clHSA_TestMode::vGetDABFrequencyTable(GUI_String *out_result)
{
   tU32 u32Size=0;
   tChar* pacText = OSAL_NEW tChar[MAX_GPS_FORMAT_STRLEN];   

   if(( NULL != pacText) && (NULL != m_poDataPool))
   {
      u32Size = m_poDataPool->u32dp_getElementSize(DPDAB__TM_FREQUENCY_TABLE);
      OSAL_pvMemorySet((tVoid*)pacText,NULL,MAX_GPS_FORMAT_STRLEN);

      m_poDataPool->u32dp_get( DPDAB__TM_FREQUENCY_TABLE,
         (tVoid*)pacText, 
         u32Size );
   }
   //Return Frequency Table	
   GUI_String_vSetCStr(out_result, (tU8*)pacText);
   OSAL_DELETE[] pacText;
}

/******************************************************************************
* FUNCTION:    clHSA_TestMode::ulwGetDABEnsembleFrequency(ulword Tuner)
*------------------------------------------------------------------------------
* DESCRIPTION: This API is called to get the Ensemble Frequency
*------------------------------------------------------------------------------
* PARAMETER:   ulword
* RETURNVALUE: ulword
******************************************************************************/
ulword clHSA_TestMode::ulwGetDABEnsembleFrequency(ulword DABTuner)
{
   tU32 u32EnsembleFrq = 0;
   if (DABTuner)
   {
      if(NULL != m_poDataPool)
      {
         m_poDataPool->u32dp_get(DPDAB__TM_ENSEMBLE_FREG_BG,
            &u32EnsembleFrq, sizeof(u32EnsembleFrq) );
      }
   }
   else
   {
      if(NULL != m_poDataPool)
      {
         m_poDataPool->u32dp_get(DPDAB__TM_ENSEMBLE_FRQ, 
            &u32EnsembleFrq, sizeof(u32EnsembleFrq) );
      }
   }
   return u32EnsembleFrq;	
}

/******************************************************************************
* FUNCTION:    clHSA_TestMode::ulwGetCurrentDABActivity()
*------------------------------------------------------------------------------
* DESCRIPTION: This API is called to get the current activity of DAB tuner
*------------------------------------------------------------------------------
* PARAMETER:   Void
* RETURNVALUE: ulword
******************************************************************************/
ulword clHSA_TestMode::ulwGetCurrentDABActivity()
{
   tU8 u8CurrentActivity = 0;
   if(NULL != m_poDataPool)
   {		
      m_poDataPool->u32dp_get(DPDAB__TM_ACTIVITY, 
         &u8CurrentActivity, sizeof(u8CurrentActivity) );
   }
   return (ulword)u8CurrentActivity;	
}

/******************************************************************************
* FUNCTION:    clHSA_TestMode::ulwGetDABSignalQuality()
*------------------------------------------------------------------------------
* DESCRIPTION: This API is called to get the Signal Quality
*------------------------------------------------------------------------------
* PARAMETER:   Void
* RETURNVALUE: ulword
******************************************************************************/
ulword clHSA_TestMode::ulwGetDABSignalQuality()
{
   tU8 u8SignalQuality = 0;
   if(NULL != m_poDataPool)
   {		
      m_poDataPool->u32dp_get(DPDAB__TM_SIGNAL_QUALITY, 
         &u8SignalQuality, sizeof(u8SignalQuality) );
   }
   return (ulword)u8SignalQuality;	
}

/******************************************************************************
* FUNCTION:    clHSA_TestMode::ulwGetDABAudioQuality()
*------------------------------------------------------------------------------
* DESCRIPTION: This API is called to get the Audio Quality
*------------------------------------------------------------------------------
* PARAMETER:   Void
* RETURNVALUE: ulword
******************************************************************************/
ulword clHSA_TestMode::ulwGetDABAudioQuality()
{
   tU8 u8AudioQuality = 0;
   if(NULL != m_poDataPool)
   {		
      m_poDataPool->u32dp_get(DPDAB__TM_AUDIO_QUALITY, 
         &u8AudioQuality, sizeof(u8AudioQuality) );
   }
   return (ulword)u8AudioQuality;	
}

/******************************************************************************
* FUNCTION:    clHSA_TestMode::blGetDABSync()
*------------------------------------------------------------------------------
* DESCRIPTION: This API is called to get the Sync Flag
*------------------------------------------------------------------------------
* PARAMETER:   ulword
* RETURNVALUE: ulword
******************************************************************************/
tbool clHSA_TestMode::blGetDABSync(ulword DABTuner)
{
   tU16 u16Flags = 0;
   tBool bIsSync = FALSE;
   if (DABTuner)
   {
      if(NULL != m_poDataPool)
      {
         m_poDataPool->u32dp_get(DPDAB__TM_STATUS_FLAGS_BG,
            &u16Flags, sizeof(u16Flags) );
      }
   }
   else
   {
      if(NULL != m_poDataPool)
      {
         m_poDataPool->u32dp_get(DPDAB__TM_STATUS_FLAGS, 
            &u16Flags, sizeof(u16Flags) );
      }
   }
   if((u16Flags & MASK_BIT_ZERO) == MASK_BIT_ZERO)
   {
      bIsSync = TRUE;
   }
   return bIsSync;	
}

/******************************************************************************
* FUNCTION:    clHSA_TestMode::blGetDABMute()
*------------------------------------------------------------------------------
* DESCRIPTION: This API is called to get the Mute Flag
*------------------------------------------------------------------------------
* PARAMETER:   Void
* RETURNVALUE: Bool
******************************************************************************/
tbool clHSA_TestMode::blGetDABMute()
{
   tU16 u16Flags = 0;
   tBool bIsMute = FALSE;
   if(NULL != m_poDataPool)
   {		
      m_poDataPool->u32dp_get(DPDAB__TM_STATUS_FLAGS, 
         &u16Flags, sizeof(u16Flags) );
   }
   if((u16Flags & MASK_BIT_FOUR) == MASK_BIT_FOUR)
   {
      bIsMute = TRUE;
   }
   return bIsMute;		
}

/******************************************************************************
* FUNCTION:    clHSA_TestMode::ulwGetDABMSCBER()
*------------------------------------------------------------------------------
* DESCRIPTION: This API is called to get the Bit Error Rate of the current audio
*              subchannel
*------------------------------------------------------------------------------
* PARAMETER:   Void
* RETURNVALUE: ulword
******************************************************************************/
ulword clHSA_TestMode::ulwGetDABMSCBER()
{
   tU32 u32DAB_MSCBER = 0;
   if(NULL != m_poDataPool)
   {		
      m_poDataPool->u32dp_get(DPDAB__TM_MSCBER, 
         &u32DAB_MSCBER, sizeof(u32DAB_MSCBER) );
   }
   return u32DAB_MSCBER;	
}

/******************************************************************************
* FUNCTION:    clHSA_TestMode::ulwGetDABFICBER(ulword Tuner)
*------------------------------------------------------------------------------
* DESCRIPTION: This API is called to get the Bit Error Rate of the Fast 
*              Information Channel
*------------------------------------------------------------------------------
* PARAMETER:   ulword
* RETURNVALUE: ulword
******************************************************************************/
ulword clHSA_TestMode::ulwGetDABFICBER(ulword DABTuner)
{
   tU32 u32DAB_FICBER = 0;

   if(DABTuner)
   {
      if(NULL != m_poDataPool)
      {
         m_poDataPool->u32dp_get(DPDAB__TM_FICBER_BG,
            &u32DAB_FICBER, sizeof(u32DAB_FICBER) );
      }
   }
   else
   {
      if(NULL != m_poDataPool)
      {
         m_poDataPool->u32dp_get(DPDAB__TM_FICBER, 
            &u32DAB_FICBER, sizeof(u32DAB_FICBER) );
      }
   }
   return u32DAB_FICBER;	
}

/******************************************************************************
* FUNCTION:    clHSA_TestMode::ulwGetDAB_RS_FEC()
*------------------------------------------------------------------------------
* DESCRIPTION: This API is called to get the Forward Error Correction(FEC) for
*              Reed-Soloman coding
*------------------------------------------------------------------------------
* PARAMETER:   Void
* RETURNVALUE: ulword
******************************************************************************/
ulword clHSA_TestMode::ulwGetDAB_RS_FEC()
{
   tU8 u8DAB_RS_FEC = 0;
   if(NULL != m_poDataPool)
   {		
      m_poDataPool->u32dp_get(DPDAB__TM_RS_FEC, 
         &u8DAB_RS_FEC, sizeof(u8DAB_RS_FEC) );
   }
   return (ulword)u8DAB_RS_FEC;	
}

/******************************************************************************
* FUNCTION:    clHSA_TestMode::slwGetDABFieldStrength()
*------------------------------------------------------------------------------
* DESCRIPTION: This API is called to get the Field strength in dBm
*------------------------------------------------------------------------------
* PARAMETER:   Void
* RETURNVALUE: slword
******************************************************************************/
slword clHSA_TestMode::slwGetDABFieldStrength()
{
   tS8 s8Field_strength = 0;
   if(NULL != m_poDataPool)
   {		
      m_poDataPool->u32dp_get(DPDAB__TM_FIELDSTRENGTH, 
         &s8Field_strength, sizeof(s8Field_strength) );
   }
   return (slword)s8Field_strength;	
}

/******************************************************************************
* FUNCTION:    clHSA_TestMode::ulwGetDABProtectionLevel()
*------------------------------------------------------------------------------
* DESCRIPTION: An ensemble has a maximum bit rate that can be carried, but this
*              depends on which error protection level is used. This API returns
*              the protection level
*------------------------------------------------------------------------------
* PARAMETER:   Void
* RETURNVALUE: ulword
******************************************************************************/
ulword clHSA_TestMode::ulwGetDABProtectionLevel()
{
   tU8 u8Protection_level = 0;
   if(NULL != m_poDataPool)
   {		
      m_poDataPool->u32dp_get(DPDAB__TM_PROTECTION_LEVEL, 
         &u8Protection_level, sizeof(u8Protection_level) );
   }
   return (ulword)u8Protection_level;	
}

/******************************************************************************
* FUNCTION:    clHSA_TestMode::vGetDABFMFrq()
*------------------------------------------------------------------------------
* DESCRIPTION: This API is called to get the FM frequency for FMLinking in kHz
*------------------------------------------------------------------------------
* PARAMETER:   Void
* RETURNVALUE: ulword
******************************************************************************/
void clHSA_TestMode::vGetDABFMFrq(GUI_String *out_result)
{
   tU32 u32FM_Frq = 0;
   if(NULL != m_poDataPool)
   {		
      m_poDataPool->u32dp_get(DPDAB__TM_FM_FRQ, 
         &u32FM_Frq, sizeof(u32FM_Frq) );
   }
   //Allocate Memory dynamically
   tChar* pacText = OSAL_NEW tChar[MAX_GPS_FORMAT_STRLEN];
   if(pacText != NULL)
      OSAL_pvMemorySet((tVoid*)pacText,NULL,MAX_GPS_FORMAT_STRLEN);

   //Format the values into required format
   UTF8_s32SaveNPrintFormat( pacText,
      MAX_GPS_FORMAT_STRLEN,
      "%d.%dMHz",
      (u32FM_Frq/1000),
      ((u32FM_Frq%1000)/100));

   //Return Frequency String		
   GUI_String_vSetCStr(out_result, (tU8*)pacText);

   //Release the Memory allocated dynamically
   OSAL_DELETE[] pacText;		
}

/******************************************************************************
* FUNCTION:    clHSA_TestMode::vGetDABFMPI()
*------------------------------------------------------------------------------
* DESCRIPTION: This API is called to get the Programme identification of the
*              tuned FM programme
*------------------------------------------------------------------------------
* PARAMETER:   GUI_String 
* RETURNVALUE: void
******************************************************************************/
void clHSA_TestMode::vGetDABFMPI(GUI_String *out_result)
{
   tU16 u16FMPI = 0;
   if(NULL != m_poDataPool)
   {		
      m_poDataPool->u32dp_get(DPDAB__TM_FM_PI, 
         &u16FMPI, sizeof(u16FMPI) );
   }
   tChar pacDABFMPI[ MAX_BUFFER_STRLEN ]  = "\0";

   UTF8_s32SaveNPrintFormat( pacDABFMPI,MAX_BUFFER_STRLEN,
      "%x",u16FMPI);

   /** Return the String*/
   GUI_String_vSetCStr(out_result, (tU8*)pacDABFMPI);
}

/******************************************************************************
* FUNCTION:    clHSA_TestMode::ulwGetDABFMQuality()
*------------------------------------------------------------------------------
* DESCRIPTION: This API is called to get the Quality of the tuned FM programme
*------------------------------------------------------------------------------
* PARAMETER:   Void
* RETURNVALUE: ulword
******************************************************************************/
ulword clHSA_TestMode::ulwGetDABFMQuality()
{
   tU8 u8FMQuality = 0;
   if(NULL != m_poDataPool)
   {		
      m_poDataPool->u32dp_get(DPDAB__TM_FM_QUALITY, 
         &u8FMQuality, sizeof(u8FMQuality) );
   }
   return (ulword)u8FMQuality;	
}

/******************************************************************************
* FUNCTION:    clHSA_TestMode::ulwGetDABTransmissionMode()
*------------------------------------------------------------------------------
* DESCRIPTION: This API is called to get the DAB transmission mode which is a
*              parameter of channel-decoding range
*------------------------------------------------------------------------------
* PARAMETER:   Void
* RETURNVALUE: ulword
******************************************************************************/
ulword clHSA_TestMode::ulwGetDABTransmissionMode()
{
   tU8 u8Mode = 0;
   if(NULL != m_poDataPool)
   {		
      m_poDataPool->u32dp_get(DPDAB__TM_MODE, 
         &u8Mode, sizeof(u8Mode) );
   }
   return (ulword)u8Mode;	
}

/******************************************************************************
* FUNCTION:    clHSA_TestMode::vSetDABTestmode()
*------------------------------------------------------------------------------
* DESCRIPTION: This API is called while entering or exiting from DAB testmode
*              to start/stop the testmode messages from fc_dabtuner
*------------------------------------------------------------------------------
* PARAMETER:   tbool
* RETURNVALUE: Void
******************************************************************************/
void clHSA_TestMode::vSetDABTestmode(tbool Active)
{
   if( NULL != pclHSI_CMDAB )
   {
      sMsg refMsg(NULL, CMMSG_DAB_TESTMODE_ACTIVE, (tU32)Active, NULL);
      pclHSI_CMDAB->bExecuteMessage( refMsg );
   }    		
}

/******************************************************************************
* FUNCTION:    clHSA_TestMode::ulwGetDABNumberOfLinks()
*------------------------------------------------------------------------------
* DESCRIPTION: This API is called to get the length of service link list
*------------------------------------------------------------------------------
* PARAMETER:   NONE
* RETURNVALUE: ulword
******************************************************************************/
ulword clHSA_TestMode::ulwGetDABNumberOfLinks()
{
   tU16 u16LinkListLen = 0;
   if(NULL != m_poDataPool)
   {		
      m_poDataPool->u32dp_get(DPDAB__TM_SERVICE_LINK_LIST_LEN, 
         &u16LinkListLen, sizeof(u16LinkListLen) );
   }
   return (ulword)u16LinkListLen;     		
}

/******************************************************************************
* FUNCTION:    clHSA_TestMode::vGetDABLinkType()
*------------------------------------------------------------------------------
* DESCRIPTION: This API is used to get the Service List link type for the
*              particular index of the link list
*------------------------------------------------------------------------------
* PARAMETER:   GUI_String, Index
* RETURNVALUE: void
******************************************************************************/
void clHSA_TestMode::vGetDABLinkType( GUI_String *out_result,ulword Index )
{
   //Read the number of elements in service link list from DP
   if(NULL != m_poDataPool)
   {
      tU16 u16SrvLinkListLen = 0;
      m_poDataPool->u32dp_get( DPDAB__TM_SERVICE_LINK_LIST_LEN,
         &u16SrvLinkListLen, 
         sizeof(u16SrvLinkListLen) );

      if((tU16)Index < u16SrvLinkListLen)
      {//Valid Request
         tU8 u8LinkType = 0;
         //Read the corresponding entry from the list
         vReadDataFromDataPool( (tU32)DPDAB__SERVICE_LINK_LIST,             //u32DataPoolIndex
            (tU8)DAB__SERVICE_LINK_LIST_LINK_TYPE,//u8DataPoolListColumnIndex 
            (tU8)Index,               //u16DataPoolListRowIndex
            &u8LinkType,                                     
            sizeof(u8LinkType)); 

         tUTF8 utf8Data[THREE_CHAR_LEN] = "\0";

         if((u8LinkType == midw_ext_fi_tcl_e8_DAB_LinkType::FI_EN_LINKTYPE_DAB_HARDLINK)
            || 
            (u8LinkType == midw_ext_fi_tcl_e8_DAB_LinkType::FI_EN_LINKTYPE_FM_HARDLINK))
         {
            UTF8_s32SaveNPrintFormat( utf8Data, THREE_CHAR_LEN, "%s", "HL");
         }
         else if((u8LinkType == midw_ext_fi_tcl_e8_DAB_LinkType::FI_EN_LINKTYPE_DAB_SOFTLINK)
            || 
            (u8LinkType == midw_ext_fi_tcl_e8_DAB_LinkType::FI_EN_LINKTYPE_FM_SOFTLINK))
         {
            UTF8_s32SaveNPrintFormat( utf8Data, THREE_CHAR_LEN, "%s", "SL");
         }
         else if((u8LinkType == midw_ext_fi_tcl_e8_DAB_LinkType::FI_EN_LINKTYPE_DAB_NOLINK)
            || 
            (u8LinkType == midw_ext_fi_tcl_e8_DAB_LinkType::FI_EN_LINKTYPE_FM_NOLINK))
         {
            UTF8_s32SaveNPrintFormat( utf8Data, THREE_CHAR_LEN, "%s", "DB");
         }
         else
         {
            //Do nothing, print null, not a valid case
         }
         GUI_String_vSetCStr(out_result,(const tU8*)utf8Data);
      }
      else
      {//Invalid List entry request
         GUI_String_vSetCStr(out_result, (const tU8*) "");
      }  
   }
   else
   {//DP pointer is NULL
      GUI_String_vSetCStr(out_result, (const tU8*) "");
   }		
}

/******************************************************************************
* FUNCTION:    clHSA_TestMode::vGetDABFrqLabel()
*------------------------------------------------------------------------------
* DESCRIPTION: This API is called to get the frequency label for the particular
*              index of the link list
*------------------------------------------------------------------------------
* PARAMETER:   GUI_String, Index
* RETURNVALUE: void
******************************************************************************/
void clHSA_TestMode::vGetDABFrqLabel(GUI_String *out_result,ulword Index)
{
   //Read the number of elements in service link list from DP
   if(NULL != m_poDataPool)
   {
      tU16 u16SrvLinkListLen = 0;
      m_poDataPool->u32dp_get( DPDAB__TM_SERVICE_LINK_LIST_LEN,
         &u16SrvLinkListLen, 
         sizeof(u16SrvLinkListLen) );

      if((tU16)Index < u16SrvLinkListLen)
      {//Valid Request
         bpstl::string ostr;

         //Read the corresponding entry from the list
         vReadDataFromDataPool( (tU32)DPDAB__SERVICE_LINK_LIST,              //u32DataPoolIndex
            (tU8)DAB__SERVICE_LINK_LIST_FREQ_LABEL,//u8DataPoolListColumnIndex 
            (tU8)Index,                           //u16DataPoolListRowIndex
            &ostr,                                 //bpstl::string  
            0);  

         //Copy for return
         GUI_String_vSetCStr(out_result, (const tU8*) ostr.c_str());
      }
      else
      {//If Index is invalid
         GUI_String_vSetCStr(out_result, (const tU8*) "");
      }
   }
   else
   {//If m_poDataPool == NULL
      GUI_String_vSetCStr(out_result, (const tU8*) "");
   }     		
}

/******************************************************************************
* FUNCTION:    clHSA_TestMode::vGetDABSID_PI()
*------------------------------------------------------------------------------
* DESCRIPTION: This API is called to get the SID or PI for the particular index
*              of the link list
*------------------------------------------------------------------------------
* PARAMETER:   Index
* RETURNVALUE: void
******************************************************************************/
void clHSA_TestMode::vGetDABSID_PI(GUI_String *out_result, ulword Index)
{
   //Read the number of elements in service link list from DP
   if(NULL != m_poDataPool)
   {
      tU16 u16SrvLinkListLen = 0;
      m_poDataPool->u32dp_get( DPDAB__TM_SERVICE_LINK_LIST_LEN,
         &u16SrvLinkListLen, 
         sizeof(u16SrvLinkListLen) );

      if((tU16)Index < u16SrvLinkListLen)
      {//Valid Request
         tU32 u32SID = 0;
         //Read the corresponding entry from the list
         vReadDataFromDataPool( (tU32)DPDAB__SERVICE_LINK_LIST,             //u32DataPoolIndex
            (tU8)DAB__SERVICE_LINK_LIST_SID,//u8DataPoolListColumnIndex 
            (tU8)Index,               //u16DataPoolListRowIndex
            &u32SID,                                     
            sizeof(u32SID)); 

         tUTF8 utf8Data[FOUR_CHAR_LEN] = "\0";
         UTF8_s32SaveNPrintFormat( utf8Data, FOUR_CHAR_LEN, "%X", u32SID);	
         GUI_String_vSetCStr(out_result,(tU8*)utf8Data);
      }
      else
      {//Invalid List entry request
         GUI_String_vSetCStr(out_result, (const tU8*) "");
      }  
   }
   else
   {//DP pointer is NULL
      GUI_String_vSetCStr(out_result, (const tU8*) "");
   }
}

/******************************************************************************
* FUNCTION:    clHSA_TestMode::ulwGetDABQuality()
*------------------------------------------------------------------------------
* DESCRIPTION: This API is called to get the quality for the particular index
*              of the link list
*------------------------------------------------------------------------------
* PARAMETER:   Index
* RETURNVALUE: ulword
******************************************************************************/
ulword clHSA_TestMode::ulwGetDABQuality(ulword Index)
{
   tU8 u8Quality = 0;
   //Read the number of elements in service link list from DP
   if(NULL != m_poDataPool)
   {
      tU16 u16SrvLinkListLen = 0;
      m_poDataPool->u32dp_get( DPDAB__TM_SERVICE_LINK_LIST_LEN,
         &u16SrvLinkListLen, 
         sizeof(u16SrvLinkListLen) );
      if((tU16)Index < u16SrvLinkListLen)
      {//Valid Request			
         //Read the corresponding entry from the list
         vReadDataFromDataPool( (tU32)DPDAB__SERVICE_LINK_LIST,             //u32DataPoolIndex
            (tU8)DAB__SERVICE_LINK_LIST_QUALITY,  //u8DataPoolListColumnIndex 
            (tU8)Index,                          //u16DataPoolListRowIndex
            &u8Quality,                                     
            sizeof(u8Quality)); 			
      }
   }
   return (ulword)u8Quality;				
}

/******************************************************************************
* FUNCTION:    clHSA_TestMode::vGetDABEID()
*------------------------------------------------------------------------------
* DESCRIPTION: This API is called to get the ECC-EID for the particular index
*              of the link list
*------------------------------------------------------------------------------
* PARAMETER:   Index
* RETURNVALUE: void
******************************************************************************/
void clHSA_TestMode::vGetDABEID(GUI_String *out_result, ulword Index)
{
   //Read the number of elements in service link list from DP
   if(NULL != m_poDataPool)
   {
      tU16 u16SrvLinkListLen = 0;
      m_poDataPool->u32dp_get( DPDAB__TM_SERVICE_LINK_LIST_LEN,
         &u16SrvLinkListLen, 
         sizeof(u16SrvLinkListLen) );

      if((tU16)Index < u16SrvLinkListLen)
      {//Valid Request
         tU32 u32EID = 0;
         //Read the corresponding entry from the list
         vReadDataFromDataPool( (tU32)DPDAB__SERVICE_LINK_LIST,        //u32DataPoolIndex
            (tU8)DAB__SERVICE_LINK_LIST_EID, //u8DataPoolListColumnIndex 
            (tU8)Index,                     //u16DataPoolListRowIndex
            &u32EID,                                     
            sizeof(u32EID)); 

         tUTF8 utf8Data[SIX_CHAR_LEN] = "\0";
         UTF8_s32SaveNPrintFormat( utf8Data, SIX_CHAR_LEN, "%X", u32EID);	
         GUI_String_vSetCStr(out_result,(tU8*)utf8Data);
      }
      else
      {//Invalid List entry request
         GUI_String_vSetCStr(out_result, (const tU8*) "");
      }  
   }
   else
   {//DP pointer is NULL
      GUI_String_vSetCStr(out_result, (const tU8*) "");
   }     		
}

/******************************************************************************
* FUNCTION:    clHSA_TestMode::ulwGetDABActiveLinkIndex()
*------------------------------------------------------------------------------
* DESCRIPTION: This API is called to get the Active Link Index from the list
*------------------------------------------------------------------------------
* PARAMETER:   NONE
* RETURNVALUE: ulword
******************************************************************************/
ulword clHSA_TestMode::ulwGetDABActiveLinkIndex()
{
   tU16 u16LinkListIndex = 0;
   if(NULL != m_poDataPool)
   {		
      m_poDataPool->u32dp_get(DPDAB__TM_ACTIVE_LINK_INDEX, 
         &u16LinkListIndex, sizeof(u16LinkListIndex) );
   }
   return (ulword)u16LinkListIndex;     		
}

/******************************************************************************
* FUNCTION:    clHSA_TestMode::vGetTASource()
*------------------------------------------------------------------------------
* DESCRIPTION: This API is called to get the Active TA source
*------------------------------------------------------------------------------
* PARAMETER:   NONE
* RETURNVALUE: GUI_String
******************************************************************************/
void clHSA_TestMode::vGetTASource(GUI_String *out_result)
{
   tU8 u8ActiveTASource = 0;
   tUTF8 utf8Data[FOUR_CHAR_LEN] = "\0";
   if(NULL != m_poDataPool)
   {		
      m_poDataPool->u32dp_get(DPTUNERMASTER__TA_SOURCE, 
         &u8ActiveTASource, sizeof(u8ActiveTASource) );		

      if (u8ActiveTASource == midw_fi_tcl_e8_Source::FI_EN_TUN_MSTR_SOURCE_FM_TA)
      {
         UTF8_s32SaveNPrintFormat( utf8Data, THREE_CHAR_LEN, "%s", "FM");
      }
      else if (u8ActiveTASource == midw_fi_tcl_e8_Source::FI_EN_TUN_MSTR_SOURCE_DAB_TA)
      {
         UTF8_s32SaveNPrintFormat( utf8Data, FOUR_CHAR_LEN, "%s", "DAB");
      }
      else if (u8ActiveTASource == midw_fi_tcl_e8_Source::FI_EN_TUN_MSTR_SOURCE_NO_ANNO)
      {
         UTF8_s32SaveNPrintFormat( utf8Data, FOUR_CHAR_LEN, "%s", "---");
      }
      else
      {
         UTF8_s32SaveNPrintFormat( utf8Data, FIVE_CHAR_LEN, "%s", "n.a.");
      }
   }
   GUI_String_vSetCStr(out_result,(tU8*)utf8Data);
}

/******************************************************************************
* FUNCTION:    clHSA_TestMode::ulwGetTMDataAvailability()
*------------------------------------------------------------------------------
* DESCRIPTION: This API is called to know if TM data is available
*------------------------------------------------------------------------------
* PARAMETER:   NONE
* RETURNVALUE: ulword
******************************************************************************/
ulword clHSA_TestMode::ulwGetTMDataAvailability()
{
   tU8 u8DataAvailable = 0;
   if(NULL != m_poDataPool)
   {		
      m_poDataPool->u32dp_get(DPDAB__TM_TMDATA_AVAILABLE, 
         &u8DataAvailable, sizeof(u8DataAvailable) );
   }
   return (ulword)u8DataAvailable;
}

/******************************************************************************
* FUNCTION:    clHSA_TestMode::vRequestToURIList()
*------------------------------------------------------------------------------
* DESCRIPTION: This API provides the status of the current TPEG messages decoded
*------------------------------------------------------------------------------
* PARAMETER:   NONE
* RETURNVALUE: ulword
******************************************************************************/
void clHSA_TestMode::vRequestToURIList( )
{
	if( NULL != pclHSI_TestMode )
	   {
	      sMsg refMsg (NULL, (tU32)CMMSG_TESTMODE_DISPLAY_URI_LIST, NULL, NULL) ;
	      pclHSI_TestMode->bExecuteMessage(refMsg);
	   }
}
/******************************************************************************
* FUNCTION:    clHSA_TestMode::ulwGetURIListCount()
*------------------------------------------------------------------------------
* DESCRIPTION: This API provides the count of URI list elements
*------------------------------------------------------------------------------
* PARAMETER:   NONE
* RETURNVALUE: ulword
******************************************************************************/
ulword clHSA_TestMode::ulwGetURIListCount( )
{
	tU16 u16NumOfElem = 0;
	m_poDataPool->u32dp_get(DPDAB__TPEGURI_LIST_LENGTH,
			 &u16NumOfElem, sizeof(u16NumOfElem) );
	return (ulword)u16NumOfElem;
}
/******************************************************************************
* FUNCTION:    clHSA_TestMode::vGetURIData()
*------------------------------------------------------------------------------
* DESCRIPTION: This API provides the count of URI list elements and also the count of active URI
*------------------------------------------------------------------------------
* PARAMETER:   NONE
* RETURNVALUE: ulword
******************************************************************************/
void clHSA_TestMode::vGetURIData(GUI_String *out_result, ulword ulwInfo_Type)
{
	tUTF8 pacData[16] = {'\0'};
	tU16 u16TotalURI = 0;
	tU16 u16ActiveURI = 0;
	if(TotalURI == ulwInfo_Type)
	   {
		m_poDataPool->u32dp_get(DPDAB__TPEGURI_LIST_LENGTH,
		         &u16TotalURI, sizeof(u16TotalURI) );
		UTF8_s32SaveNPrintFormat(pacData, 15 , "%d",u16TotalURI);
		GUI_String_vSetCStr(out_result, (tPU8)pacData);
	   }
	else
		{
		m_poDataPool->u32dp_get(DPDAB__ACTIVE_URI_COUNT,
				         &u16ActiveURI, sizeof(u16ActiveURI) );
		UTF8_s32SaveNPrintFormat(pacData, 15 , "%d",u16ActiveURI);
		GUI_String_vSetCStr(out_result, (tPU8)pacData);
		}
}
/******************************************************************************
* FUNCTION:    clHSA_TestMode::ulwGetTotalACIDperFrame()
*------------------------------------------------------------------------------
* DESCRIPTION: This API provides the status of the current TPEG messages decoded
*------------------------------------------------------------------------------
* PARAMETER:   NONE
* RETURNVALUE: ulword
******************************************************************************/
ulword clHSA_TestMode::ulwGetTotalACIDperFrame(ulword ulwInfo_Type)
{
   tU32 u32Data = 0;
   if(EN_ACIDPerFrame == ulwInfo_Type)
   {
      m_poDataPool->u32dp_get(DPDAB__ACID_PER_FRAME,
         &u32Data, sizeof(u32Data) );
   }
   else if(EN_MsgsPerFrame == ulwInfo_Type)
   {
      m_poDataPool->u32dp_get(DPDAB__MSGS_PER_FRAME,
         &u32Data, sizeof(u32Data) );
   }
   return (ulword)u32Data;
}

/******************************************************************************
* FUNCTION:    clHSA_TestMode::vGet_TEC()
*------------------------------------------------------------------------------
* DESCRIPTION: This API provides the status of the current TEC messages decoded
*------------------------------------------------------------------------------
* PARAMETER:   NONE
* RETURNVALUE: void
******************************************************************************/
void clHSA_TestMode::vGet_TEC(GUI_String *out_result, ulword ulwInfo_Type)
{
	tUTF8 pacData[16] = {'\0'};
	tU16 u16Data = 0;
	bpstl::string oReadBuffer = "";
	if(ulwInfo_Type ==(tU32) TEC_NO_OF_MSGS)
	{
		if(m_poDataPool)
			m_poDataPool->u32dp_get( DPNAVI__TPEG_TEC_NO_OF_MSGS,
			&u16Data,
			sizeof(u16Data) );
		UTF8_s32SaveNPrintFormat(pacData, 15 , "%d",u16Data);
		GUI_String_vSetCStr(out_result, (tPU8)pacData);
	}
	if(ulwInfo_Type == (tU32)TEC_LASTMSG_DATETIME)
	{
		dp.vGetString( DPNAVI__TPEG_TEC_LASTMSG_DATETIME, oReadBuffer);
		if(m_poTrace) { m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
		                  TR_CLASS_HMI_HSA_MNGR,
		                  "TEC_LASTMSG_DATETIME : %s", oReadBuffer.c_str());	}
		GUI_String_vSetCStr(out_result, (tU8*) const_cast<tChar*>(oReadBuffer.c_str()));
	}
	if(ulwInfo_Type == (tU32)TEC_NO_OF_MSGS_SEL_AREA)
	{
		if(m_poDataPool)
			m_poDataPool->u32dp_get( DPNAVI__TPEG_TEC_NO_OF_MSGS_SEL_AREA,
			&u16Data,
			sizeof(u16Data) );
			UTF8_s32SaveNPrintFormat(pacData, 15 , "%d",u16Data);
			GUI_String_vSetCStr(out_result, (tPU8)pacData);
	}
	if(ulwInfo_Type == (tU32)TEC_SEL_LASTMSG_DATETIME)
	{
		dp.vGetString( DPNAVI__TPEG_TEC_SEL_LASTMSG_DATETIME, oReadBuffer);
		if(m_poTrace) { m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
				                  TR_CLASS_HMI_HSA_MNGR,
				                  "TEC_SEL_LASTMSG_DATETIME : %s", oReadBuffer.c_str());	}
		GUI_String_vSetCStr(out_result, (tU8*) const_cast<tChar*>(oReadBuffer.c_str()));
	}
}


/******************************************************************************
* FUNCTION:    clHSA_TestMode::vGet_TMC()
*------------------------------------------------------------------------------
* DESCRIPTION: This API provides the status of the current TEC messages decoded
*------------------------------------------------------------------------------
* PARAMETER:   NONE
* RETURNVALUE: void
******************************************************************************/
void clHSA_TestMode::vGet_TMC(GUI_String *out_result, ulword ulwInfo_Type)
{
	tUTF8 pacData[16] = {'\0'};
	tU16 u16Data = 0;
	bpstl::string oReadBuffer = "";
	if(ulwInfo_Type ==(tU32) TFP_NO_OF_MSGS)
	{
		if(m_poDataPool)
			m_poDataPool->u32dp_get( DPNAVI__TMC_NO_OF_MSGS,
			&u16Data,
			sizeof(u16Data) );
		UTF8_s32SaveNPrintFormat(pacData, 15 , "%d",u16Data);
		GUI_String_vSetCStr(out_result, (tPU8)pacData);
	}
	if(ulwInfo_Type == (tU32)TFP_LASTMSG_DATETIME)
	{
		dp.vGetString( DPNAVI__TPEG_TMC_LASTMSG_DATETIME, oReadBuffer);
		if(m_poTrace) { m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
				                  TR_CLASS_HMI_HSA_MNGR,
				                  "TFP_LASTMSG_DATETIME : %s", oReadBuffer.c_str());	}
		GUI_String_vSetCStr(out_result, (tU8*) const_cast<tChar*>(oReadBuffer.c_str()));
	}
	if(ulwInfo_Type == (tU32)TFP_NO_OF_MSGS_SEL_AREA)
	{
		if(m_poDataPool)
			m_poDataPool->u32dp_get( DPNAVI__TMC_NO_OF_MSGS_SEL_AREA,
			&u16Data,
			sizeof(u16Data) );
			UTF8_s32SaveNPrintFormat(pacData, 15 , "%d",u16Data);
			GUI_String_vSetCStr(out_result, (tPU8)pacData);
	}
	if(ulwInfo_Type == (tU32)TFP_SEL_LASTMSG_DATETIME)
	{
		dp.vGetString( DPNAVI__TPEG_TMC_SEL_LASTMSG_DATETIME, oReadBuffer);
		if(m_poTrace) { m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
						                  TR_CLASS_HMI_HSA_MNGR,
						                  "TFP_SEL_LASTMSG_DATETIME : %s", oReadBuffer.c_str());	}
		GUI_String_vSetCStr(out_result, (tU8*) const_cast<tChar*>(oReadBuffer.c_str()));
	}
}
/******************************************************************************
* FUNCTION:    clHSA_TestMode::ulwGetStreamListCount()
*------------------------------------------------------------------------------
* DESCRIPTION: This API provides ther count
*------------------------------------------------------------------------------
* PARAMETER:   NONE
* RETURNVALUE: ulword
******************************************************************************/
ulword clHSA_TestMode::ulwGetStreamListCount( )
{
	tU16 u16ListLen = 0;
	m_poDataPool->u32dp_get(DPDAB__TPEGSTREAMINFO_LIST_LENGTH,
	         &u16ListLen, sizeof(u16ListLen) );
	return (ulword)u16ListLen;

}




void clHSA_TestMode::vGetURIList(GUI_String *out_result, ulword ulwListEntryNr, ulword ulwInfo_Type)
{
	if(m_poTrace) { m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
							               TR_CLASS_HMI_HSA_MNGR,
							               "ulwListEntryNr : %d", (tU16)ulwListEntryNr);	}
	if(ulwInfo_Type ==(tU32) URI_INDEX)
			{
					tUTF8 pacListIndex[10] = {'\0'};
					tU8 u8ListIndex = 0;
					vReadDataFromDataPool( DPDAB__TPEGURI_LIST,             //u32DataPoolIndex
							                               (tU8)DAB__TPEGURI_LIST_ROW_ID,//u8DataPoolListColumnIndex
							                               (tU8)ulwListEntryNr,               //u16DataPoolListRowIndex
							                               &u8ListIndex,
							                               sizeof(u8ListIndex));
					UTF8_s32SaveNPrintFormat(pacListIndex, 9 , "%d",u8ListIndex+1);
					GUI_String_vSetCStr(out_result, (tPU8)pacListIndex);
			}
	else if(ulwInfo_Type ==(tU32) URI_NAME)
			{
						bpstl::string ostr;

						//Read the corresponding entry from the list
						vReadDataFromDataPool( DPDAB__TPEGURI_LIST,              //u32DataPoolIndex
				                               DAB__TPEGURI_LIST_URI_NAME,//u8DataPoolListColumnIndex
				                               (tU16)ulwListEntryNr,             //u16DataPoolListRowIndex
				                               &ostr,                         //bpstl::string
				                               0);
						 if(m_poTrace) { m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
						               TR_CLASS_HMI_HSA_MNGR,
						               "URI_NAME : %s", ostr.c_str());	}
						//Copy for return
		                GUI_String_vSetCStr(out_result, const_cast<char*>(ostr.c_str()));

			}
	else if(ulwInfo_Type ==(tU32) URI_SELECTED)
				{
					bpstl::string ostr;
					tBool bReception = 0;
					vReadDataFromDataPool( DPDAB__TPEGURI_LIST,             //u32DataPoolIndex
				                           (tU8)DAB__TPEGURI_LIST_SELECT_STATE,//u8DataPoolListColumnIndex
				                           (tU16)ulwListEntryNr,               //u16DataPoolListRowIndex
				                           &bReception,                     //bpstl::string
				                           sizeof(bReception) );
					if(bReception)
						{
							ostr = "TRUE";
						}
						else
						{
							ostr = "FALSE";
						}
					GUI_String_vSetCStr(out_result, (const tU8*) ostr.c_str());

				}
	else if(ulwInfo_Type ==(tU32) URI_ACTIVE)
					{
						bpstl::string ostr;
						tBool bReception = 0;
						vReadDataFromDataPool( DPDAB__TPEGURI_LIST,             //u32DataPoolIndex
					                           (tU8)DAB__TPEGURI_LIST_ACTIVE_STATE,//u8DataPoolListColumnIndex
					                           (tU16)ulwListEntryNr,               //u16DataPoolListRowIndex
					                           &bReception,                     //bpstl::string
					                           sizeof(bReception) );
						if(bReception)
							{
								ostr = "TRUE";
							}
							else
							{
								ostr = "FALSE";
							}
						GUI_String_vSetCStr(out_result, (const tU8*) ostr.c_str());

					}
}

/******************************************************************************
* FUNCTION:    clHSA_TestMode::vGetStreamList()
*------------------------------------------------------------------------------
* DESCRIPTION: This API provides the Stream List Info
*------------------------------------------------------------------------------
* PARAMETER:
* RETURNVALUE: void
******************************************************************************/
void clHSA_TestMode::vGetStreamList(GUI_String *out_result, ulword ulwListEntryNr, ulword ulwInfo_Type)
{
	if(ulwInfo_Type ==(tU32) INDEX)
		{
		    tUTF8 pacListIndex[10] = {'\0'};
			tU8 u8ListIndex = 0;
			vReadDataFromDataPool( DPDAB__TPEGSTREAMINFO_LIST,             //u32DataPoolIndex
					                               (tU8)DAB__TPEGSTREAMINFO_LIST_ROW_ID,//u8DataPoolListColumnIndex
					                               (tU8)ulwListEntryNr,               //u16DataPoolListRowIndex
					                               &u8ListIndex,
					                               sizeof(u8ListIndex));
			UTF8_s32SaveNPrintFormat(pacListIndex, 9 , "%d",u8ListIndex+1);
			GUI_String_vSetCStr(out_result, (tPU8)pacListIndex);
		}
	else if(ulwInfo_Type ==(tU32) SID)
		{
		    tU8 u8SIDA = 0;
		    tU8 u8SIDB = 0;
		    tU8 u8SIDC = 0;
		    bpstl::string strdot = ".";
			vReadDataFromDataPool( DPDAB__TPEGSTREAMINFO_LIST,             //u32DataPoolIndex
												   (tU8)DAB__TPEGSTREAMINFO_LIST_SIDA,//u8DataPoolListColumnIndex
												   (tU16)ulwListEntryNr,               //u16DataPoolListRowIndex
												   &u8SIDA,
												   sizeof(u8SIDA));
			vReadDataFromDataPool( DPDAB__TPEGSTREAMINFO_LIST,             //u32DataPoolIndex
															   (tU8)DAB__TPEGSTREAMINFO_LIST_SIDB,//u8DataPoolListColumnIndex
															   (tU16)ulwListEntryNr,               //u16DataPoolListRowIndex
															   &u8SIDB,
															   sizeof(u8SIDB));
			vReadDataFromDataPool( DPDAB__TPEGSTREAMINFO_LIST,             //u32DataPoolIndex
															   (tU8)DAB__TPEGSTREAMINFO_LIST_SIDC,//u8DataPoolListColumnIndex
															   (tU16)ulwListEntryNr,               //u16DataPoolListRowIndex
															   &u8SIDC,
															   sizeof(u8SIDC));
			 tChar tSID[25] = "\0";
			 OSAL_s32PrintFormat(tSID,"%d%s%d%s%d",u8SIDA, strdot.c_str(),u8SIDB,strdot.c_str(),u8SIDC);
			 GUI_String_vSetCStr(out_result, (tU8*) tSID);
		}
		else if(ulwInfo_Type == (tU32)APP_ID)
		{
			tUTF8 pacAppID[10] = {'\0'};
			tU8 u8AppID = 0;
						vReadDataFromDataPool( DPDAB__TPEGSTREAMINFO_LIST,             //u32DataPoolIndex
								                               (tU8)DAB__TPEGSTREAMINFO_LIST_App_ID,//u8DataPoolListColumnIndex
								                               (tU16)ulwListEntryNr,               //u16DataPoolListRowIndex
								                               &u8AppID,
								                               sizeof(u8AppID));
						UTF8_s32SaveNPrintFormat(pacAppID, 9 , "%d",u8AppID);
						GUI_String_vSetCStr(out_result, (tPU8)pacAppID);
		}
		else if(ulwInfo_Type == (tU32)CONTENT_ID)
		{
			tUTF8 pacContentID[10] = {'\0'};
			tU8 u8ContentID = 0;
						vReadDataFromDataPool( DPDAB__TPEGSTREAMINFO_LIST,             //u32DataPoolIndex
															   (tU8)DAB__TPEGSTREAMINFO_LIST_Content_ID,//u8DataPoolListColumnIndex
															   (tU16)ulwListEntryNr,               //u16DataPoolListRowIndex
															   &u8ContentID,
															   sizeof(u8ContentID));
						UTF8_s32SaveNPrintFormat(pacContentID, 9 , "%d",u8ContentID);
						GUI_String_vSetCStr(out_result, (tPU8)pacContentID);
		}
		else if(ulwInfo_Type == (tU32)VERSION)
		{
			tUTF8 pacVersion[10] = {'\0'};
			tU8 u8Version = 0;
						vReadDataFromDataPool( DPDAB__TPEGSTREAMINFO_LIST,             //u32DataPoolIndex
															   (tU8)DAB__TPEGSTREAMINFO_LIST_Version,//u8DataPoolListColumnIndex
															   (tU16)ulwListEntryNr,               //u16DataPoolListRowIndex
															   &u8Version,
															   sizeof(u8Version));
						UTF8_s32SaveNPrintFormat(pacVersion, 9 , "%d",u8Version);
						GUI_String_vSetCStr(out_result, (tPU8)pacVersion);
		}
		else if(ulwInfo_Type == (tU32)LOC)
				{
					tUTF8 pacLOC[10] = {'\0'};
					tU8 u8LOC = 0;
								vReadDataFromDataPool( DPDAB__TPEGSTREAMINFO_LIST,             //u32DataPoolIndex
																	   (tU8)DAB__TPEGSTREAMINFO_LIST_LOC,//u8DataPoolListColumnIndex
																	   (tU16)ulwListEntryNr,               //u16DataPoolListRowIndex
																	   &u8LOC,
																	   sizeof(u8LOC));
								UTF8_s32SaveNPrintFormat(pacLOC, 9 , "%d",u8LOC);
								GUI_String_vSetCStr(out_result, (tPU8)pacLOC);
				}

		else if(ulwInfo_Type == (tU32)MSG_PER_FRAME)
						{
							tUTF8 pacMsgsperFrame[20] = {'\0'};
							tU32 u32MsgsperFrame = 0;
										vReadDataFromDataPool( DPDAB__TPEGSTREAMINFO_LIST,             //u32DataPoolIndex
																			   (tU8)DAB__TPEGSTREAMINFO_LIST_Msgs_Per_Frame,//u8DataPoolListColumnIndex
																			   (tU16)ulwListEntryNr,               //u16DataPoolListRowIndex
																			   &u32MsgsperFrame,
																			   sizeof(u32MsgsperFrame));
										UTF8_s32SaveNPrintFormat(pacMsgsperFrame, 19 , "%d",u32MsgsperFrame);
										GUI_String_vSetCStr(out_result, (tPU32*) pacMsgsperFrame);
						}
		else if(ulwInfo_Type == MAX_MSGS)
								{
									tUTF8 pacMaxMsgsperFrame[20] = {'\0'};
									tU32 u32MaxMsgsperFrame = 0;
												vReadDataFromDataPool( DPDAB__TPEGSTREAMINFO_LIST,             //u32DataPoolIndex
																					   (tU8)DAB__TPEGSTREAMINFO_LIST_MaxMsgs_Per_Frame,//u8DataPoolListColumnIndex
																					   (tU16)ulwListEntryNr,               //u16DataPoolListRowIndex
																					   &u32MaxMsgsperFrame,
																					   sizeof(u32MaxMsgsperFrame));
												UTF8_s32SaveNPrintFormat(pacMaxMsgsperFrame, 19 , "%d",u32MaxMsgsperFrame);
												GUI_String_vSetCStr(out_result, (tPU32*) pacMaxMsgsperFrame);
								}
		else if(ulwInfo_Type == (tU32)AVG_MSGS)
								{
									tUTF8 pacAvgMsgsperFrame[20] = {'\0'};
									tU32 u32AvgMsgsperFrame = 0;
												vReadDataFromDataPool( DPDAB__TPEGSTREAMINFO_LIST,             //u32DataPoolIndex
																					   (tU8)DAB__TPEGSTREAMINFO_LIST_AvgMsgs_Per_Frame,//u8DataPoolListColumnIndex
																					   (tU16)ulwListEntryNr,               //u16DataPoolListRowIndex
																					   &u32AvgMsgsperFrame,
																					   sizeof(u32AvgMsgsperFrame));
												UTF8_s32SaveNPrintFormat(pacAvgMsgsperFrame, 19 , "%d",u32AvgMsgsperFrame);
												GUI_String_vSetCStr(out_result, (tPU32*) pacAvgMsgsperFrame);
								}


}
/*******************************DAB APIs End**********************************/

/******API for Display Test******/
/******************************************************************************
* FUNCTION:    clHSA_TestMode::vDisplayTestStatus()
*------------------------------------------------------------------------------
* DESCRIPTION: This API is called by GUI to indicate whether the command to 
*              start/stop Display test was handled or not. This is required to 
*              send the response back to fc_diagnosis
*------------------------------------------------------------------------------
* PARAMETER:   Bool
* RETURNVALUE: void
******************************************************************************/
void clHSA_TestMode::vDisplayTestStatus(tbool DisplayTestSuccess)
{
   if( NULL != pclHSI_TestMode )
   {
      sMsg refMsg (NULL, (tU32)CMMSG_TESTMODE_DISPLAY_TEST_STATUS, (tU32)DisplayTestSuccess, NULL) ;
      pclHSI_TestMode->bExecuteMessage(refMsg);         
   }
}
/******API for Display test end******/

/*************************************************************************
* HSA-API METHOD:          bDisplayStringOnTxtLab
*
* DESCRIPTION:  To Display string on the controller
*
* PARAMETER:                            
*           const tChar * ptrStrToDisplay 
*                         ( Poniter to string to display )
*           tU16 u16Crtl_ID 
*               ( crtl ID of the controller configured in HMI Designer )    
* RETURNVALUE:
*               true : if message handled
* HISTROY:
*   
*  08.11.06 RBIN/EDI1 Rama Rayalu Vattikuti  Initial Revision
************************************************************************/

ulword clHSA_TestMode::ulwGetTunerTP(ulword Tuner)
{

   tU8 u8TpValue = 0;

   if(m_poDataPool)
   {
      switch( Tuner)
      {
      case TUNER_0:
         {
            m_poDataPool->u32dp_get(DPTUNER__TESTMODE_TP_TUNER_0, &u8TpValue, sizeof(tU8) );
            break;
         }
      case TUNER_1:
         {
            m_poDataPool->u32dp_get(DPTUNER__TESTMODE_TP_TUNER_1, &u8TpValue, sizeof(tU8) );
            break;
         }
      case TUNER_2:
         {
            m_poDataPool->u32dp_get(DPTUNER__TESTMODE_TP_TUNER_2, &u8TpValue, sizeof(tU8) );
            break;
         }
      case TUNER_3:
         {
            m_poDataPool->u32dp_get(DPTUNER__TESTMODE_TP_TUNER_3, &u8TpValue, sizeof(tU8) );
            break;
         }
      default:
         {
            // No Action Here
            break;
         }
      }

   } /** End Of switch */


   /*

   Checking for the TP value. 
   If the TA value is greater than the MAX_TP_VALUE(1), assigning the Max TP value that is allowed.

   */

   if ( u8TpValue > MAX_TP_VALUE )
   {
      u8TpValue = MAX_TP_VALUE;
   }

   /** Trace is here */	
   if(m_poTrace)
   {	
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
         TR_CLASS_HMI_HSA_SYSTEM_TESTMODE,
         (tU16)(HSA_API_RAYALU__GET_TUNER_TP ),
         1,
         &u8TpValue );
   }
   return u8TpValue;
}

/**
* Method: vGetGPSValues
* Returns values about the GPS
* B1
*/
void clHSA_TestMode::vGetGPSValues(GUI_String *out_result,ulword ListEntryNr)
{
   /** Character Buffer to Hold the Formatted String */ 
   tChar* tCharpacText = NULL;
   tCharpacText = OSAL_NEW tChar[MAX_GPS_FORMAT_STRLEN];	
   if (tCharpacText != NULL)
      OSAL_pvMemorySet((tVoid*)tCharpacText,NULL,MAX_GPS_FORMAT_STRLEN);

   switch(ListEntryNr)
   {

      /** Case Meridians */
   case TESTMODE_MERIDIANS:
      {
         tU32 u32Ephemeridis = 0;
         if (tCharpacText != NULL)
            OSAL_pvMemorySet((tVoid*)tCharpacText,NULL,MAX_GPS_FORMAT_STRLEN);

         if(m_poDataPool)
         {
            m_poDataPool->u32dp_get( DPVDS__GPS_DATA_INFO_EPHEMERIS, 
               &u32Ephemeridis, 
               sizeof(u32Ephemeridis) );
         }
         tBool barrdata[14] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0};
         for(tU8 u8ExtractIndex = 0; u8ExtractIndex<14; u8ExtractIndex++)
         {
            tU32 u32TempEphemeridis = u32Ephemeridis;
            barrdata[u8ExtractIndex] = (tBool)( (u32TempEphemeridis>>u8ExtractIndex) & 0x00000001 );
         }

         UTF8_s32SaveNPrintFormat( tCharpacText,
            MAX_GPS_FORMAT_STRLEN, 
            "%d  %d  %d  %d  %d  %d  %d  %d  %d  %d   %d   %d   %d  %d",        ///   spaces added 
            barrdata[13],barrdata[12],barrdata[11],
            barrdata[10],barrdata[9],barrdata[8],
            barrdata[7],barrdata[6],barrdata[5],
            barrdata[4],barrdata[3],barrdata[2],
            barrdata[1],barrdata[0]	
         );

         /** Return Antenna Connected Status*/	
         GUI_String_vSetCStr(out_result, (tU8*)tCharpacText);


         break;
      }


      /** Case Almanch Status */
   case TESTMODE_ANTENNASTATUS:
      {
         tU8 u8AntennaStatus = 0;
         if (tCharpacText != NULL)
            OSAL_pvMemorySet((tVoid*)tCharpacText,NULL,MAX_GPS_FORMAT_STRLEN);

         if(m_poDataPool)
         {
            m_poDataPool->u32dp_get( DPVDS__GPS_DATA_INFO_ANTENNA, 
               &u8AntennaStatus, 
               sizeof(tU8) );
         }

         /** Map the Antenna status in the following way */
         /** For mapping refer Vdsensor Fi Document */
         if( 0 == u8AntennaStatus )
         {
            UTF8_s32SaveNPrintFormat( tCharpacText,
               MAX_GPS_FORMAT_STRLEN, 
               "%s\0",
               "Connected" );
         }
         else if( 1 == u8AntennaStatus )
         {
            UTF8_s32SaveNPrintFormat( tCharpacText,
               MAX_GPS_FORMAT_STRLEN, 
               "%s\0",
               "Open" );
         }
         else if( (2 == u8AntennaStatus) || 
            (3 == u8AntennaStatus) ||
            (4 == u8AntennaStatus)	)
         {
            UTF8_s32SaveNPrintFormat( tCharpacText,
               MAX_GPS_FORMAT_STRLEN, 
               "%s\0",
               "Short" );
         }
         else 
         {
            UTF8_s32SaveNPrintFormat( tCharpacText,
               MAX_GPS_FORMAT_STRLEN, 
               "%s\0",
               "Unknown" );
         }
         /** Return Antenna Connected Status*/	
         GUI_String_vSetCStr(out_result, (tU8*)tCharpacText);


         break;
      }


      /** Case Trackning Status */
   case TESTMODE_TRACKINGSTATUS:
      {
         tU8 u8FixStatus = 0;
         if(tCharpacText != NULL)
            OSAL_pvMemorySet((tVoid*)tCharpacText,NULL,MAX_GPS_FORMAT_STRLEN);


         if(m_poDataPool)
         {
            m_poDataPool->u32dp_get( DPVDS__GPS_DATA_INFO_FIX_STATUS, 
               &u8FixStatus, 
               sizeof(tU8) );
         }
         if( u8FixStatus & 0x04)
         {
            UTF8_s32SaveNPrintFormat( tCharpacText,
               MAX_GPS_FORMAT_STRLEN, 
               "%s",
               "FIX_3D" );
         }
         else if( u8FixStatus & 0x02)
         {
            UTF8_s32SaveNPrintFormat( tCharpacText,
               MAX_GPS_FORMAT_STRLEN, 
               "%s",
               "FIX_2D" );
         }
         else if( u8FixStatus & 0x01 )
         {
            UTF8_s32SaveNPrintFormat( tCharpacText,
               MAX_GPS_FORMAT_STRLEN, 
               "%s",
               "FIX_TIME" );
         }
         else if( u8FixStatus & 0x80)
         {
            UTF8_s32SaveNPrintFormat( tCharpacText,
               MAX_GPS_FORMAT_STRLEN, 
               "%s",
               "FIX_COLDSTART" );
         }
         else
         {
            UTF8_s32SaveNPrintFormat( tCharpacText,
               MAX_GPS_FORMAT_STRLEN, 
               "%s",
               "NO_FIX" );
         }

         /** Return GPS Tracking Status*/	
         GUI_String_vSetCStr(out_result, (tU8*)tCharpacText);

         break;
      }


      /** Case Visible Satellites */
   case TESTMODE_SATVISIBLE:
      {
         tU8 u8SatsVisible = 0;
         if(tCharpacText != NULL)
            OSAL_pvMemorySet((tVoid*)tCharpacText,NULL,MAX_GPS_FORMAT_STRLEN);


         if(m_poDataPool)
         {
            m_poDataPool->u32dp_get( DPVDS__GPS_DATA_INFO_VISIBLE_SATS, 
               &u8SatsVisible, 
               sizeof(u8SatsVisible) );
         }
         UTF8_s32SaveNPrintFormat( tCharpacText,
            MAX_GPS_FORMAT_STRLEN, 
            "%d",
            u8SatsVisible );

         /** Return formatted Visible Satellites */	
         GUI_String_vSetCStr(out_result, (tU8*)tCharpacText);

         break;
      }


      /** Case Tracked Satellites */
   case TESTMODE_SATTRACKED:
      {
         tU8 u8SatsReceived = 0;
         if(tCharpacText != NULL)
            OSAL_pvMemorySet((tVoid*)tCharpacText,NULL,MAX_GPS_FORMAT_STRLEN);


         if(m_poDataPool)
         {
            m_poDataPool->u32dp_get( DPVDS__GPS_DATA_INFO_RECEIVED_SATS, 
               &u8SatsReceived, 
               sizeof(tU8) );
         }
         UTF8_s32SaveNPrintFormat( tCharpacText,
            MAX_GPS_FORMAT_STRLEN, 
            "%d",
            u8SatsReceived );

         /** Return Tracked Satellites*/	
         GUI_String_vSetCStr(out_result, (tU8*)tCharpacText);

         break;
      }


      /** Case Longitude */
   case TESTMODE_LONGITUDE:
      {
         tU8 u8FixStatus = 0;
         if(tCharpacText != NULL)
            OSAL_pvMemorySet((tVoid*)tCharpacText,NULL,MAX_GPS_FORMAT_STRLEN);

         if(m_poDataPool)
         {
            m_poDataPool->u32dp_get( DPVDS__GPS_DATA_INFO_FIX_STATUS, 
               &u8FixStatus, 
               sizeof(tU8) );
         }
         if( (u8FixStatus & 0x02) || (u8FixStatus & 0x04) )
         {
            tS32 s32temp = 0;

            /** Buffers to store the data temperarily */
            tUTF8 utf8FormattedData[MAX_GPS_FORMAT_STRLEN] = "\0";
            tUTF8 utf8IsoToUtf8Data[MAX_GPS_FORMAT_STRLEN] = "\0";
            tUTF8 utf8FormattedDegData[MAX_GPS_FORMAT_STRLEN] = "\0";
            tUTF8 utf8FormattedMinData[MAX_GPS_FORMAT_STRLEN] = "\0";
            tUTF8 utf8FormattedSecData[MAX_GPS_FORMAT_STRLEN] = "\0";
            tUTF8 utf8FormattedTenthSecData[MAX_GPS_FORMAT_STRLEN] = "\0";

            if(m_poDataPool)
            {

               m_poDataPool->u32dp_get( DPVDS__GPS_DATA_INFO_LONGITUDE_LOW_32BITS,
                  &s32temp,
                  sizeof(tS32) );
               tujoin  ujoin_Longitude;
               ujoin_Longitude.s32join[0]=s32temp;
               m_poDataPool->u32dp_get( DPVDS__GPS_DATA_INFO_LONGITUDE_HIGH_32BITS,
                  &s32temp,
                  sizeof(tS32) );
               ujoin_Longitude.s32join[1]=s32temp;

               tF64 *f64Longitudejoin=(tF64*)&ujoin_Longitude.s32join;
               sGps_Info prLongInfo=sGpsConversion(*f64Longitudejoin);
               if( prLongInfo.u16Degree < TESTMODE_DLR_GPS_ONEDIGIT)
               {
                  UTF8_s32SaveNPrintFormat( utf8FormattedDegData,
                     TESTMODE_DLR_GPS_INTEGER,
                     "\x30\x30""%d",
                     prLongInfo.u16Degree );
               }
               else if( (prLongInfo.u16Degree >= TESTMODE_DLR_GPS_ONEDIGIT) && 
                  (prLongInfo.u16Degree < TESTMODE_DLR_GPS_TWODIGIT) )
               {
                  UTF8_s32SaveNPrintFormat( utf8FormattedDegData,
                     TESTMODE_DLR_GPS_INTEGER,
                     "\x30""%d",
                     prLongInfo.u16Degree );
               }
               else
               {
                  UTF8_s32SaveNPrintFormat( utf8FormattedDegData,
                     TESTMODE_DLR_GPS_INTEGER, 
                     "%d",
                     prLongInfo.u16Degree );
               }					

               if( prLongInfo.u16Minutes < 10)
               {
                  UTF8_s32SaveNPrintFormat( utf8FormattedMinData,
                     MAX_GPS_FORMAT_STRLEN,
                     "\x30""%d",
                     prLongInfo.u16Minutes );
               }
               else
               {
                  UTF8_s32SaveNPrintFormat( utf8FormattedMinData,
                     MAX_GPS_FORMAT_STRLEN,
                     "%d",
                     prLongInfo.u16Minutes );
               }
               if( prLongInfo.u16Seconds < 10)
               {
                  UTF8_s32SaveNPrintFormat( utf8FormattedSecData,
                     MAX_GPS_FORMAT_STRLEN,
                     "\x30""%d",
                     prLongInfo.u16Seconds );
               }
               else
               {
                  UTF8_s32SaveNPrintFormat( utf8FormattedSecData,
                     MAX_GPS_FORMAT_STRLEN,
                     "%d",
                     prLongInfo.u16Seconds );
               }

               UTF8_s32SaveNPrintFormat( utf8FormattedTenthSecData,
                  MAX_GPS_FORMAT_STRLEN,
                  "%d",
                  prLongInfo.u16TenthSeconds );

               if( prLongInfo.bDirection == TRUE )
               {
                  /** Convert Raw Gps Longitude Info into Buffer for formatting */
                  UTF8_s32SaveNPrintFormat( utf8FormattedData,
                     MAX_GPS_FORMAT_STRLEN,
                     "E%s""\xB0""%s.%s""\x22", //Similar to mms 312838 fix
                     utf8FormattedDegData,
                     utf8FormattedMinData,
                     utf8FormattedSecData );
               }
               else
               {
                  /** Convert Raw Gps Longitude Info into Buffer for formatting */
                  UTF8_s32SaveNPrintFormat( utf8FormattedData,
                     MAX_GPS_FORMAT_STRLEN,
                     "W%s""\xB0""%s.%s""\x22", //Similar to mms 312838 fix
                     utf8FormattedDegData,
                     utf8FormattedMinData,
                     utf8FormattedSecData );
               }

               /** Convert the buffer data into required format 
               as the buffer data contains the UTF8 Coded Character
               and get the number of UTF8 Bytes the String Requires */

               utfutil_u32ConvertISOMod2UTF8( utf8IsoToUtf8Data,
                  MAX_GPS_FORMAT_STRLEN,
                  utf8FormattedData,
                  MAX_GPS_FORMAT_STRLEN );

               /** Get the UTF8 formatted String into the Buffer to Display */
               UTF8_s32SaveNPrintFormat( tCharpacText,
                  MAX_GPS_FORMAT_STRLEN,
                  "%s", 
                  utf8IsoToUtf8Data  );
            }

         }
         else
         {
            /** Buffer to store the data temperarily */
            tUTF8 utf8IsoToUtf8Data[MAX_GPS_FORMAT_STRLEN] = "\0";

            utfutil_u32ConvertISOMod2UTF8( utf8IsoToUtf8Data,
               MAX_GPS_FORMAT_STRLEN,
               "\x2d\x2d\xB0\x2d\x2d\x27\x2d\x2d\x2E\x2d\x22\0",
               MAX_GPS_FORMAT_STRLEN	);
            /** Get the UTF8 formatted String into the Buffer to Display */
            UTF8_s32SaveNPrintFormat( tCharpacText,
               MAX_GPS_FORMAT_STRLEN,
               "%s", 
               utf8IsoToUtf8Data  );


         }

         /** Return Longitude */	
         GUI_String_vSetCStr(out_result, (tU8*)tCharpacText);

         break;
      }


      /** Case Latitude */
   case TESTMODE_LATITUDE:
      {
         tU8 u8FixStatus = 0;
         if(tCharpacText != NULL)
            OSAL_pvMemorySet((tVoid*)tCharpacText,NULL,MAX_GPS_FORMAT_STRLEN);

         if(m_poDataPool)
         {
            m_poDataPool->u32dp_get( DPVDS__GPS_DATA_INFO_FIX_STATUS, 
               &u8FixStatus, 
               sizeof(tU8) );
         }
         if(  (u8FixStatus & 0x02) || (u8FixStatus & 0x04) )
         {

            tS32 s32temp = 0;
            /** Buffer to store the data temperarily */
            tUTF8 utf8FormattedData[MAX_GPS_FORMAT_STRLEN] = "\0";
            tUTF8 utf8IsoToUtf8Data[MAX_GPS_FORMAT_STRLEN] = "\0";
            tUTF8 utf8FormattedDegData[MAX_GPS_FORMAT_STRLEN] = "\0";
            tUTF8 utf8FormattedMinData[MAX_GPS_FORMAT_STRLEN] = "\0";
            tUTF8 utf8FormattedSecData[MAX_GPS_FORMAT_STRLEN] = "\0";
            tUTF8 utf8FormattedTenthSecData[MAX_GPS_FORMAT_STRLEN] = "\0";

            if(m_poDataPool)
            {
               /** Get the Latitude High*/
               m_poDataPool->u32dp_get( DPVDS__GPS_DATA_INFO_LATITUDE_LOW_32BITS,
                  &s32temp,
                  sizeof(tS32) );
               tujoin ujoin_Latitude;
               ujoin_Latitude.s32join[0]=s32temp;
               m_poDataPool->u32dp_get( DPVDS__GPS_DATA_INFO_LATITUDE_HIGH_32BITS,
                  &s32temp,
                  sizeof(tS32) );
               ujoin_Latitude.s32join[1]=s32temp; 

               tF64 *f64Latitudejoin=(tF64*)&ujoin_Latitude.s32join;
               sGps_Info prlatInfo=sGpsConversion(*f64Latitudejoin);

               if( prlatInfo.u16Degree < 10)
               {
                  UTF8_s32SaveNPrintFormat( utf8FormattedDegData,
                     MAX_GPS_FORMAT_STRLEN,
                     "\x30""%d",
                     prlatInfo.u16Degree );
               } 
               else
               {
                  UTF8_s32SaveNPrintFormat( utf8FormattedDegData,
                     MAX_GPS_FORMAT_STRLEN, 
                     "%d",
                     prlatInfo.u16Degree );
               }

               if( prlatInfo.u16Minutes < 10)
               {
                  UTF8_s32SaveNPrintFormat( utf8FormattedMinData,
                     MAX_GPS_FORMAT_STRLEN,
                     "\x30""%d",
                     prlatInfo.u16Minutes );
               }
               else
               {
                  UTF8_s32SaveNPrintFormat( utf8FormattedMinData,
                     MAX_GPS_FORMAT_STRLEN,
                     "%d",
                     prlatInfo.u16Minutes );
               }
               if( prlatInfo.u16Seconds < 10)
               {
                  UTF8_s32SaveNPrintFormat( utf8FormattedSecData,
                     MAX_GPS_FORMAT_STRLEN,
                     "\x30""%d",
                     prlatInfo.u16Seconds );
               }
               else
               {
                  UTF8_s32SaveNPrintFormat( utf8FormattedSecData,
                     MAX_GPS_FORMAT_STRLEN,
                     "%d",
                     prlatInfo.u16Seconds );
               }

               UTF8_s32SaveNPrintFormat( utf8FormattedTenthSecData,
                  MAX_GPS_FORMAT_STRLEN,
                  "%d",
                  prlatInfo.u16TenthSeconds );

               if( prlatInfo.bDirection == TRUE )
               {
                  /** Convert Raw Gps Longitude Info into Buffer for formatting */
                  UTF8_s32SaveNPrintFormat( utf8FormattedData,
                     MAX_GPS_FORMAT_STRLEN,
                     "N%s""\xB0""%s.%s""\x22", //Similar to mms 312838 fix
                     utf8FormattedDegData,
                     utf8FormattedMinData,
                     utf8FormattedSecData );
               }
               else
               {
                  /** Convert Raw Gps Longitude Info into Buffer for formatting */
                  UTF8_s32SaveNPrintFormat( utf8FormattedData,
                     MAX_GPS_FORMAT_STRLEN,
                     "S%s""\xB0""%s.%s""\x22", //Similar to mms 312838 fix
                     utf8FormattedDegData,
                     utf8FormattedMinData,
                     utf8FormattedSecData	);
               }

               /** Convert the buffer data into required format 
               as the buffer data contains the UTF8 Coded Character
               and get the number of UTF8 Bytes the String Requires */

               utfutil_u32ConvertISOMod2UTF8( utf8IsoToUtf8Data,
                  MAX_GPS_FORMAT_STRLEN,
                  utf8FormattedData,
                  MAX_GPS_FORMAT_STRLEN );

               /** Get the UTF8 formatted String into the Buffer to Display */
               UTF8_s32SaveNPrintFormat( tCharpacText,
                  MAX_GPS_FORMAT_STRLEN,
                  "%s", 
                  utf8IsoToUtf8Data  );
            }

         }
         else
         {
            /** Buffer to store the data temperarily */
            tUTF8 utf8IsoToUtf8Data[MAX_GPS_FORMAT_STRLEN] = "\0";

            utfutil_u32ConvertISOMod2UTF8( utf8IsoToUtf8Data,
               MAX_GPS_FORMAT_STRLEN,
               "\x2d\x2d\xB0\x2d\x2d\x27\x2d\x2d\x2E\x2d\x22\0",
               MAX_GPS_FORMAT_STRLEN	);
            /** Get the UTF8 formatted String into the Buffer to Display */
            UTF8_s32SaveNPrintFormat( tCharpacText,
               MAX_GPS_FORMAT_STRLEN,
               "%s", 
               utf8IsoToUtf8Data  );


         }

         /** Return Latitude*/	
         GUI_String_vSetCStr(out_result, (tU8*)tCharpacText);

         break;
      }

      /** Case Altitude */
   case TESTMODE_ALTITUDE:
      {
         tS16 s16Altitude = 0;
         if(tCharpacText != NULL)
            OSAL_pvMemorySet((tVoid*)tCharpacText,NULL,MAX_GPS_FORMAT_STRLEN);			

         if(m_poDataPool)
         {
            m_poDataPool->u32dp_get( DPVDS__GPS_DATA_INFO_ALTITUDE, 
               &s16Altitude, 
               sizeof(s16Altitude) );
         }
         /** Get the UTF8 formatted String into the Buffer to Display */
         UTF8_s32SaveNPrintFormat( tCharpacText,
            MAX_GPS_FORMAT_STRLEN,
            "%d m", 
            s16Altitude  );
         /** Return Altitude*/	
         GUI_String_vSetCStr(out_result, (tU8*)tCharpacText);

         break;
      }

      /** Case HDOP */
   case TESTMODE_HDOP:
      {
         tS32 s32HDOP = 0;
         tF32 f32HDOP = 0;
         if(tCharpacText != NULL)
            OSAL_pvMemorySet((tVoid*)tCharpacText,NULL,MAX_GPS_FORMAT_STRLEN);



         if(m_poDataPool)
         {
            m_poDataPool->u32dp_get( DPVDS__GPS_DATA_INFO_HDOP, 
               &s32HDOP, 
               sizeof(s32HDOP) );
         }
         f32HDOP = (tF32)s32HDOP/10.0;

         /** Get the UTF8 formatted String into the Buffer to Display */
         UTF8_s32SaveNPrintFormat( tCharpacText,
            MAX_GPS_FORMAT_STRLEN,
            "%.1f", 
            f32HDOP  );
         /** Return HDOP*/	
         GUI_String_vSetCStr(out_result, (tU8*)tCharpacText);

         break;
      }


      /** Case VDOP */
   case TESTMODE_VDOP:
      {
         tS32 s32PDOP = 0;
         tF32 f32PDOP = 0;
         if(tCharpacText != NULL)
            OSAL_pvMemorySet((tVoid*)tCharpacText,NULL,MAX_GPS_FORMAT_STRLEN);



         if(m_poDataPool)
         {
            m_poDataPool->u32dp_get( DPVDS__GPS_DATA_INFO_PDOP, 
               &s32PDOP, 
               sizeof(s32PDOP) );
         }
         f32PDOP = (tF32)s32PDOP/10.0;

         /** Get the UTF8 formatted String into the Buffer to Display */
         UTF8_s32SaveNPrintFormat( tCharpacText,
            MAX_GPS_FORMAT_STRLEN,
            "%.1f", 
            f32PDOP  );
         /** Return VDOP*/	
         GUI_String_vSetCStr(out_result, (tU8*)tCharpacText);

         break;
      }


      /** Case Heading */
   case TESTMODE_HEADING:
      {
         tS32 s32HeadinginDegrees = 0;
         if(tCharpacText != NULL)
            OSAL_pvMemorySet((tVoid*)tCharpacText,NULL,MAX_GPS_FORMAT_STRLEN);



         if(m_poDataPool)
         {
            m_poDataPool->u32dp_get( DPVDS__GPS_DATA_INFO_VEHICLE_HEADING, 
               &s32HeadinginDegrees, 
               sizeof(s32HeadinginDegrees) );
         }

         /** Format the display */
         tUTF8 utf8IsoToUtf8Data[MAX_GPS_FORMAT_STRLEN] = "\0";
         tUTF8 utf8FormattedData[MAX_GPS_FORMAT_STRLEN] = "\0";

         UTF8_s32SaveNPrintFormat( utf8FormattedData, 
            MAX_GPS_FORMAT_STRLEN, 
            "%d""\xB0", 
            s32HeadinginDegrees	);


         /** Convert the buffer data into required format 
         as the buffer data contains the UTF8 Coded Character
         and get the number of UTF8 Bytes the String Requires */
         utfutil_u32ConvertISOMod2UTF8( utf8IsoToUtf8Data,
            MAX_GPS_FORMAT_STRLEN,
            utf8FormattedData,
            MAX_GPS_FORMAT_STRLEN );


         /** Get the UTF8 formatted String into the Buffer to Display */
         UTF8_s32SaveNPrintFormat( tCharpacText,
            MAX_GPS_FORMAT_STRLEN,
            "%s", 
            utf8IsoToUtf8Data  );
         /** Return HEADING*/	
         GUI_String_vSetCStr(out_result, (tU8*)tCharpacText);

         break;
      }


      /** Case GPS Date */
   case TESTMODE_DATE:
      {
         tU8 u8Date = 0;
         tU8 u8Month = 0;
         tU16 u16Year = 0;
         if(tCharpacText != NULL)
            OSAL_pvMemorySet((tVoid*)tCharpacText,NULL,MAX_GPS_FORMAT_STRLEN);			

         if(m_poDataPool)
         {
#ifdef DPVDS__GPS_DATA_INFO_CURR_DAY
            m_poDataPool->u32dp_get( DPVDS__GPS_DATA_INFO_CURR_DAY, 
               &u8Date, 
               sizeof(u8Date) );
#endif

#ifdef DPVDS__GPS_DATA_INFO_CURR_MONTH
            m_poDataPool->u32dp_get( DPVDS__GPS_DATA_INFO_CURR_MONTH, 
               &u8Month, 
               sizeof(u8Month) );
#endif

#ifdef DPVDS__GPS_DATA_INFO_CURR_YEAR				
            m_poDataPool->u32dp_get( DPVDS__GPS_DATA_INFO_CURR_YEAR, 
               &u16Year, 
               sizeof(u16Year) );
#endif
         }

         //Extracting the last 2 digit of the Year value.
         u16Year = u16Year%MODULUSVALUE;

         UTF8_s32SaveNPrintFormat( tCharpacText,MAX_GPS_FORMAT_STRLEN,"%02d.%02d.%02d",u8Date,u8Month,u16Year);

         /** Return Date*/	
         GUI_String_vSetCStr(out_result, (tU8*)tCharpacText);

         break;
      }


      /** Case GPS Time */
   case TESTMODE_TIME:
      {
         tU8 u8hours = 0;
         tU8 u8Minutess = 0;
         tU8 u8Seconds = 0;
         if(tCharpacText != NULL)
            OSAL_pvMemorySet((tVoid*)tCharpacText,NULL,MAX_GPS_FORMAT_STRLEN);			



         if(m_poDataPool)
         {
#ifdef DPVDS__GPS_DATA_INFO_CURR_HOURS
            m_poDataPool->u32dp_get( DPVDS__GPS_DATA_INFO_CURR_HOURS, 
               &u8hours, 
               sizeof(u8hours) );
#endif

#ifdef DPVDS__GPS_DATA_INFO_CURR_MINS
            m_poDataPool->u32dp_get( DPVDS__GPS_DATA_INFO_CURR_MINS, 
               &u8Minutess, 
               sizeof(u8Minutess) );
#endif

#ifdef DPVDS__GPS_DATA_INFO_CURR_SECONDS				
            m_poDataPool->u32dp_get( DPVDS__GPS_DATA_INFO_CURR_SECONDS, 
               &u8Seconds, 
               sizeof(u8Seconds) );
#endif
         }

         UTF8_s32SaveNPrintFormat( tCharpacText,MAX_GPS_FORMAT_STRLEN,"%02d:%02d:%02d",u8hours,u8Minutess,u8Seconds);

         /** Return Time String*/	
         GUI_String_vSetCStr(out_result, (tU8*)tCharpacText);

         break;
      }


      /** Case Almanc Status */
   case TESTMODE_ALMANCSTATUS:
      {
         tU32 u32AlmancStatus = 0;
         tU8 u8NoOfOnesInAlmancStatus = 0;
         tU8 u8HealthySats = 0;
         if(tCharpacText != NULL)
            OSAL_pvMemorySet((tVoid*)tCharpacText,NULL,MAX_GPS_FORMAT_STRLEN);

         if(m_poDataPool)
         {
            m_poDataPool->u32dp_get( DPVDS__GPS_DATA_INFO_ALMANAC_STATUS, 
               &u32AlmancStatus, 
               sizeof(u32AlmancStatus) );


            m_poDataPool->u32dp_get( DPVDS__GPS_DATA_INFO_HEALTHY_SATS, 
               &u8HealthySats, 
               sizeof(u8HealthySats) );

         }
         for( tU8 u8index=0; u8index<32; u8index++)
         {

            if(m_poTrace) { m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
               TR_CLASS_HMI_HSA_MNGR,
               "IterationNumer : %d", u8index);	}

            if(m_poTrace) { m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
               TR_CLASS_HMI_HSA_MNGR,
               "Almanc Status Value before Shift : %d", u32AlmancStatus);	}


            if( (u32AlmancStatus) & (0x00000001) )
            {				
               u8NoOfOnesInAlmancStatus += 1;

               if(m_poTrace) { m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
                  TR_CLASS_HMI_HSA_MNGR,
                  "In IF loop Updating the No.Of Ones : %d", u8NoOfOnesInAlmancStatus);	}

            }
            else
            {

               if(m_poTrace) { m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
                  TR_CLASS_HMI_HSA_MNGR,
                  "In else loop notUpdating the No.Of Ones : %d", u8NoOfOnesInAlmancStatus);	}
            }
            u32AlmancStatus >>= 1;
         }
         if(m_poTrace) { m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
            TR_CLASS_HMI_HSA_MNGR,
            "No of Ones in Almanc Status is : %d", u8NoOfOnesInAlmancStatus);	}


         if( 0 == u8NoOfOnesInAlmancStatus )
         {
            UTF8_s32SaveNPrintFormat( tCharpacText,MAX_GPS_FORMAT_STRLEN,"%s","NOK");
         }
         else if( u8NoOfOnesInAlmancStatus < u8HealthySats )
         {
            UTF8_s32SaveNPrintFormat( tCharpacText,MAX_GPS_FORMAT_STRLEN,"%s","Loading");
         }
         else if ( u8NoOfOnesInAlmancStatus >= u8HealthySats )
         {
            UTF8_s32SaveNPrintFormat( tCharpacText,MAX_GPS_FORMAT_STRLEN,"%s","OK");
         }
         else
         {
            //Nothing to do here.
         }



         /** Return Altitude*/	
         GUI_String_vSetCStr(out_result, (tU8*)tCharpacText);
         break;
      }

   case TESTMODE_GNSS_TYPE:
      {
         tU8 u8GNSS_Type = 0;
         if(tCharpacText != NULL)
            OSAL_pvMemorySet((tVoid*)tCharpacText,NULL,MAX_GPS_FORMAT_STRLEN);
         if(m_poDataPool)
         {
            m_poDataPool->u32dp_get( DPVDS__TESTMODE_GNSS_TYPE,
               &u8GNSS_Type,
               sizeof(u8GNSS_Type) );
         }
         if( 1 == u8GNSS_Type )
         {
            UTF8_s32SaveNPrintFormat( tCharpacText,MAX_GPS_FORMAT_STRLEN,"%s","GPS");
         }
         else if( 2 == u8GNSS_Type )
         {
            UTF8_s32SaveNPrintFormat( tCharpacText,MAX_GPS_FORMAT_STRLEN,"%s","GLONASS");
         }
         else
         {
            //do nothing..
         }
         GUI_String_vSetCStr(out_result, (tU8*)tCharpacText);
         break;
      }


      /** Default Case */
   default :
      {
         //Nothing to Do Here
         break;
      }

   } /** End Of switch */

   /** Trace is here */	
   if(m_poTrace)
   {	
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
         TR_CLASS_HMI_HSA_SYSTEM_TESTMODE,
         (tU16)(HSA_API_RAYALU__GET_GPS_VALUES ),
         MAX_GPS_FORMAT_STRLEN,
         (tU8*)tCharpacText );
   }

   /** Release the memory Allocated dynamically */
   OSAL_DELETE[] tCharpacText;
}

/*************************************************************************
* METHODE:          sGpsConversion
*
* CLASS:            clHSA_TestMode
*
* DESCRIPTION:      To Convert 32 Degree info(can be latitude or longitude)
into xxx�yy'zz.z" Format 
*
* PARAMETER:        Raw GPS Info                      
*              
* RETURNVALUE:      pointer to the Structure
*              
* HISTROY:
*  
*  Initial Revision
************************************************************************/

sGps_Info clHSA_TestMode::sGpsConversion(tF64 f64Value)const
{
   /** Structture variable to hold the parameters in required from */
   sGps_Info gpsConv;
   gpsConv.u16Degree = 0;
   gpsConv.u16Minutes = 0;
   gpsConv.u16Seconds = 0;
   gpsConv.u16TenthSeconds = 0;

   /** F64 variable to hold the value from the server */ 

   // Temperary Variable to Hold the f64 Value...
   tF64 temp = 0; 

   if( f64Value >= 0.0)
   {
      gpsConv.bDirection = TRUE;
   }
   else
   {
      gpsConv.bDirection = FALSE;
   }

   /** Convert the float 64 bit value into User readable format */
   temp =static_cast<tF64>((fabs(f64Value)*180.0*36000.0/3.141592653589793238462643)+TESTMODE_CORRECTION_FACTOR);

   tU32 u32Seconds = 0;

   if( temp <= static_cast <tF64>(0xffffffff) )
   {
      u32Seconds = static_cast<tU32>(temp);
   }
   else
   {
      u32Seconds = 0;
   }

   gpsConv.u16Degree = static_cast<tU16>(u32Seconds/36000);

   gpsConv.u16Minutes = static_cast<tU16>((u32Seconds-(gpsConv.u16Degree*36000))/600);

   gpsConv.u16Seconds = static_cast<tU16>((u32Seconds-(gpsConv.u16Degree*36000)-(gpsConv.u16Minutes*600))/10);
   gpsConv.u16TenthSeconds = static_cast<tU16>(u32Seconds-(gpsConv.u16Degree*36000)-(gpsConv.u16Minutes*600)-(gpsConv.u16Seconds*10));


   return gpsConv;
}

/**
* Method: vGetVersion
* Returns the requested version as string
* B1
*/
void clHSA_TestMode::vGetVersion(GUI_String *out_result,ulword Version)
{

   tU32 u32Size=0;
   tChar* pacText = OSAL_NEW tChar[MAX_LEN_FOR_VERSION];

   /* Event is sent form Component manager when user enters the Version screen of DTM and STM */

   if(pacText && m_poDataPool)
   {
      switch( Version )
      {
      case EN_CUSTOMER_VERSION:
         {
            u32Size   = m_poDataPool->u32dp_getElementSize(DPOSAL__VERSION_TESTMODE_CUSTOMER_VERSION);
            OSAL_pvMemorySet((tVoid*)pacText,NULL,MAX_LEN_FOR_VERSION);

            m_poDataPool->u32dp_get( DPOSAL__VERSION_TESTMODE_CUSTOMER_VERSION,
               (tVoid*)pacText,
               u32Size	);

            //return the Customer version
            GUI_String_vSetCStr(out_result, (tU8*)pacText);

            break;
         }

      case EN_BUILDVERSION_CUSTVERSTRING:
         {
            u32Size   = m_poDataPool->u32dp_getElementSize(DPOSAL__VERSION_TESTMODE_BUILDVERSION_CUSTVERSTRING);
            OSAL_pvMemorySet((tVoid*)pacText,NULL,MAX_LEN_FOR_VERSION);

            m_poDataPool->u32dp_get( DPOSAL__VERSION_TESTMODE_BUILDVERSION_CUSTVERSTRING,
               (tVoid*)pacText,
               u32Size	);


            //return Software Version number
            GUI_String_vSetCStr(out_result, (tU8*)pacText);
            break;
         }
      case EN_BUILDVERSION_LABEL:
         {
            u32Size   = m_poDataPool->u32dp_getElementSize(DPOSAL__VERSION_TESTMODE_BUILDVERSION_LABEL);
            OSAL_pvMemorySet((tVoid*)pacText,NULL,MAX_LEN_FOR_VERSION);

            m_poDataPool->u32dp_get( DPOSAL__VERSION_TESTMODE_BUILDVERSION_LABEL,
               (tVoid*)pacText,
               u32Size	);

            //return Software Version label
            GUI_String_vSetCStr(out_result, (tU8*)pacText);
            break;
         }

      case EN_NAVI_REGISTRY_VERSION:
         {
            u32Size   = m_poDataPool->u32dp_getElementSize(DPOSAL__VERSION_TESTMODE_NAVI_SW_VERSION);
            OSAL_pvMemorySet((tVoid*)pacText,NULL,MAX_LEN_FOR_VERSION);

            m_poDataPool->u32dp_get( DPOSAL__VERSION_TESTMODE_NAVI_SW_VERSION,
               (tVoid*)pacText,
               u32Size	);

            //Return Navigation Software
            GUI_String_vSetCStr(out_result, (tU8*)pacText);

            break;
         }
      case EN_ADR_SW_VERSION:
         {
            u32Size   = m_poDataPool->u32dp_getElementSize(DPOSAL__VERSION_TESTMODE_ADR_VERSION);
            OSAL_pvMemorySet((tVoid*)pacText,NULL,MAX_LEN_FOR_VERSION);

            m_poDataPool->u32dp_get( DPOSAL__VERSION_TESTMODE_ADR_VERSION,
               (tVoid*)pacText,
               u32Size	);

            //Return ADR3 SW version
            GUI_String_vSetCStr(out_result, (tU8*)pacText);
            break;
         }
      case EN_CPLD_VERSION:
         {

            u32Size   = m_poDataPool->u32dp_getElementSize(DPOSAL__VERSION_TESTMODE_CPLD_VERSION);
            OSAL_pvMemorySet((tVoid*)pacText,NULL,MAX_LEN_FOR_VERSION);

            m_poDataPool->u32dp_get( DPOSAL__VERSION_TESTMODE_CPLD_VERSION,
               (tVoid*)pacText,
               u32Size	);

            //Return CPLD Version
            GUI_String_vSetCStr(out_result, (tU8*)pacText);

            break;
         }
      case EN_CONFIG_PART_NUMBER:
         {	
            u32Size   = m_poDataPool->u32dp_getElementSize(DPOSAL__VERSION_TESTMODE_CONFIG_PART_NUMBER);
            OSAL_pvMemorySet((tVoid*)pacText,NULL,MAX_LEN_FOR_VERSION);

            m_poDataPool->u32dp_get( DPOSAL__VERSION_TESTMODE_CONFIG_PART_NUMBER,
               (tVoid*)pacText,
               u32Size	);

            //Config Part Number & hash value
            GUI_String_vSetCStr(out_result, (tU8*)pacText);

            break;
         }
      case EN_BOSCH_CONFIGURATION_ID:
         {	
            u32Size   = m_poDataPool->u32dp_getElementSize(DPOSAL__VERSION_TESTMODE_BOSCH_CONFIGURATION_ID);
            OSAL_pvMemorySet((tVoid*)pacText,NULL,MAX_LEN_FOR_VERSION);

            m_poDataPool->u32dp_get( DPOSAL__VERSION_TESTMODE_BOSCH_CONFIGURATION_ID,
               (tVoid*)pacText,
               u32Size	);

            //Configuration ID
            GUI_String_vSetCStr(out_result, (tU8*)pacText);

            break;
         }
      case EN_CD_MODULE_VERSION:
         {
            tChar* pacTempTextCDModulVersion = OSAL_NEW tChar[41];

            if (pacTempTextCDModulVersion != NULL)
               OSAL_pvMemorySet((tVoid*)pacTempTextCDModulVersion,NULL,41);

            tU32 u32TextSize = 40;

            tChar* pacTextCDModulVersion = OSAL_NEW tChar[41];
            if (pacTextCDModulVersion != NULL)
               OSAL_pvMemorySet((tVoid*)pacTextCDModulVersion,NULL,41);

            m_poDataPool->u32dp_get( DPOSAL__CDDRIVE_VERSION,
               pacTempTextCDModulVersion, 
               u32TextSize	);


            /** Convert the buffer data into required format 
            as the buffer data contains the UTF8 Coded Character
            and get the number of UTF8 Bytes the String Requires */

            if( pacTempTextCDModulVersion && pacTextCDModulVersion )
            {

               utfutil_u32ConvertISOMod2UTF8( pacTextCDModulVersion,
                  u32TextSize,
                  pacTempTextCDModulVersion,
                  u32TextSize );
            }

            if(pacTextCDModulVersion != NULL)
            {
               /** Get the UTF8 formatted String into the Buffer to Display */
               //UTF8_s32SaveNPrintFormat( pacTextCDModulVersion,MAX_GPS_FORMAT_STRLEN,"%s",pacTextCDModulVersion);

               GUI_String_vSetCStr(out_result, (tU8*)pacTextCDModulVersion);
               OSAL_DELETE []pacTextCDModulVersion;
            }
            if(pacTempTextCDModulVersion != NULL)
               OSAL_DELETE[] pacTempTextCDModulVersion;

            break;
         }
      case EN_HARDWARE_VERSION:
         {

            u32Size   = m_poDataPool->u32dp_getElementSize(DPDVPTSM__VW_HWVERSION);
            OSAL_pvMemorySet((tVoid*)pacText,NULL,MAX_GPS_FORMAT_STRLEN);
            m_poDataPool->u32dp_get( DPDVPTSM__VW_HWVERSION,
               (tVoid*)pacText,
               u32Size	);


            //Hardware Version
            GUI_String_vSetCStr(out_result, (tU8*)pacText);
            break;
         }
      case EN_BT_MODULE_VERSION:
         {
            bpstl::string ostrHW_Version;
            bpstl::string ostrFW_Version;

            // Read the UGZZC firmware & hardware versions from DP

            m_poDataPool->u32dp_get(
               DPTELEPHONE__UGZZC_HW_VERSION_INFO, 
               &ostrHW_Version, 
               0);

            m_poDataPool->u32dp_get(
               DPTELEPHONE__UGZZC_FW_VERSION_INFO, 
               &ostrFW_Version, 
               0);


            if( (ostrHW_Version.empty() != TRUE) && (ostrFW_Version.empty() != TRUE) )
            {
               /* Append both firmware & hardware versions for display with a space in between*/
#ifdef PROJECTBUILD_NISSAN_LCN2
               ostrHW_Version.append(" / ");
#else
               ostrHW_Version.append(" ");
#endif
               ostrHW_Version.append(ostrFW_Version);

            }
            else
            {
               /* If either of the string is empty, then " N.A" to be displayed */
               ostrHW_Version.assign("---");
            }


            GUI_String_vSetCStr(out_result, (const tU8*)ostrHW_Version.c_str());

            break;
         }
      case EN_BT_FIRMWARE_VERSION:
         {
            bpstl::string ostrFW_Version;

            // Read the UGZZC firmware version from DP

            m_poDataPool->u32dp_get(
               DPTELEPHONE__UGZZC_FW_VERSION_INFO, 
               &ostrFW_Version, 
               0);


            // If string is empty, then assign " N.A" to be displayed
            if( ostrFW_Version.empty() == TRUE )
            {
               ostrFW_Version.assign("---");
            }

            GUI_String_vSetCStr(out_result, (const tU8*)ostrFW_Version.c_str());

            break;

         }

      case EN_ORDER_NUMBER:
         {  

            u32Size   = m_poDataPool->u32dp_getElementSize(DPDVPTSM__NISSAN_PART_NUMBER);
            OSAL_pvMemorySet((tVoid*)pacText,NULL,MAX_GPS_FORMAT_STRLEN);

            m_poDataPool->u32dp_get( DPDVPTSM__NISSAN_PART_NUMBER,
               (tVoid*)pacText,
               u32Size	);




            UTF8_s32SaveNPrintFormat( pacText, MAX_GPS_FORMAT_STRLEN, "%c%c%c%c%c%c%c%c%c%c",
               (tU8)(pacText[0]), (tU8)(pacText[1]),
               (tU8)(pacText[2]), (tU8)(pacText[3]),
               (tU8)(pacText[4]), (tU8)(pacText[5]),
               (tU8)(pacText[6]), (tU8)(pacText[7]),
               (tU8)(pacText[8]), (tU8)(pacText[9]));


            /** Return Altitude*/	
            GUI_String_vSetCStr(out_result, (tU8*)pacText);

            break;			
         }
      case EN_SERIAL_NUMBER:
         {

            u32Size   = m_poDataPool->u32dp_getElementSize(DPDVPTSM__NISSAN_SERIAL_NUMBER);
            OSAL_pvMemorySet((tVoid*)pacText,NULL,MAX_GPS_FORMAT_STRLEN);
            m_poDataPool->u32dp_get( DPDVPTSM__NISSAN_SERIAL_NUMBER,
               (tVoid*)pacText,
               u32Size	);
            UTF8_s32SaveNPrintFormat( pacText, MAX_GPS_FORMAT_STRLEN, "%c%c%c%c%c%c%c",
               (tU8)(pacText[0]), (tU8)(pacText[1]),
               (tU8)(pacText[2]), (tU8)(pacText[3]),
               (tU8)(pacText[4]), (tU8)(pacText[5]),
               (tU8)(pacText[6]), (tU8)(pacText[7]));

            GUI_String_vSetCStr(out_result, (tU8*)pacText);
            break;
         }
      case EN_SD_CARD_NUMBER:
         {
            tU32 u32DevSerialNumber = 0;
            /*

            Checking whether SD Card is inserted or not.
            If not inserted, Empty string is displayed.
            Otherwise, Display the Serial No. of SD Card.

            */

            tU32 u32SizeSDState = m_poDataPool -> u32dp_getElementSize(DPDEVMGR__DRIVEINFO_SD_CARD_STATE);
            tBool bSDState = 0;
            m_poDataPool -> u32dp_get(DPDEVMGR__DRIVEINFO_SD_CARD_STATE, &bSDState, u32SizeSDState);

            if (bSDState == FALSE)
            {
               UTF8_s32SaveNPrintFormat( pacText,MAX_GPS_FORMAT_STRLEN,"No Card");
            }
            else
            {
               m_poDataPool->u32dp_get(DPDEVMGR__DRIVEINFO_SD_SERIAL_NUMBER,
                  &u32DevSerialNumber,
                  sizeof(u32DevSerialNumber));

               m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
                  TR_CLASS_HMI_HSA_SYSTEM_TESTMODE, "u32DevSerialNumber = %d", u32DevSerialNumber );

               UTF8_s32SaveNPrintFormat( pacText,MAX_GPS_FORMAT_STRLEN,"%u",u32DevSerialNumber);
            }



            GUI_String_vSetCStr(out_result, (tU8*)pacText);

            break;
         }
      case EN_METER_VERSION_AUDIO:
         {
            tU8 u8SW_Version_Lower = 0;
            tU8 u8SW_Version_Mid = 0;
            tU8 u8SW_Version_Upper = 0;
            tU8 u8HW_Version_Lower = 0;
            tU8 u8HW_Version_Mid = 0;
            tU8 u8HW_Version_Upper = 0;


            m_poDataPool->u32dp_get(DPSYSTEM__METER_SW_AUDIO_UPPERVERSION,
               &u8SW_Version_Upper, 
               sizeof(u8SW_Version_Upper));
            m_poDataPool->u32dp_get(DPSYSTEM__METER_SW_AUDIO_MIDVERSION,
               &u8SW_Version_Mid, 
               sizeof(u8SW_Version_Mid));
            m_poDataPool->u32dp_get(DPSYSTEM__METER_SW_AUDIO_LOWERVERSION,
               &u8SW_Version_Lower, 
               sizeof(u8SW_Version_Lower));
            m_poDataPool->u32dp_get(DPSYSTEM__METER_HW_AUDIO_UPPERVERSION,
               &u8HW_Version_Upper, 
               sizeof(u8HW_Version_Upper));
            m_poDataPool->u32dp_get(DPSYSTEM__METER_HW_AUDIO_MIDVERSION,
               &u8HW_Version_Mid, 
               sizeof(u8HW_Version_Mid));
            m_poDataPool->u32dp_get(DPSYSTEM__METER_HW_AUDIO_LOWERVERSION,
               &u8HW_Version_Lower, 
               sizeof(u8HW_Version_Lower));

            UTF8_s32SaveNPrintFormat( pacText,MAX_GPS_FORMAT_STRLEN,"%u.%u.%u/%u.%u.%u",
               u8HW_Version_Upper, u8HW_Version_Mid, u8HW_Version_Lower,
               u8SW_Version_Upper, u8SW_Version_Mid, u8SW_Version_Lower);

            GUI_String_vSetCStr(out_result, (tU8*)pacText);
            break;
         }
      case EN_METER_VERSION_NAVI:
         {
            tU8 u8SW_Version_Lower = 0;
            tU8 u8SW_Version_Mid = 0;
            tU8 u8SW_Version_Upper = 0;
            tU8 u8HW_Version_Lower = 0;
            tU8 u8HW_Version_Mid = 0;
            tU8 u8HW_Version_Upper = 0;


            m_poDataPool->u32dp_get(DPSYSTEM__METER_SW_NAVI_UPPERVERSION,
               &u8SW_Version_Upper, 
               sizeof(u8SW_Version_Upper));
            m_poDataPool->u32dp_get(DPSYSTEM__METER_SW_NAVI_MIDVERSION,
               &u8SW_Version_Mid, 
               sizeof(u8SW_Version_Mid));
            m_poDataPool->u32dp_get(DPSYSTEM__METER_SW_NAVI_LOWERVERSION,
               &u8SW_Version_Lower, 
               sizeof(u8SW_Version_Lower));
            m_poDataPool->u32dp_get(DPSYSTEM__METER_HW_NAVI_UPPERVERSION,
               &u8HW_Version_Upper, 
               sizeof(u8HW_Version_Upper));
            m_poDataPool->u32dp_get(DPSYSTEM__METER_HW_NAVI_MIDVERSION,
               &u8HW_Version_Mid, 
               sizeof(u8HW_Version_Mid));
            m_poDataPool->u32dp_get(DPSYSTEM__METER_HW_NAVI_LOWERVERSION,
               &u8HW_Version_Lower, 
               sizeof(u8HW_Version_Lower));

            UTF8_s32SaveNPrintFormat( pacText,MAX_GPS_FORMAT_STRLEN,"%u.%u.%u/%u.%u.%u",
               u8HW_Version_Upper, u8HW_Version_Mid, u8HW_Version_Lower,
               u8SW_Version_Upper, u8SW_Version_Mid, u8SW_Version_Lower);

            GUI_String_vSetCStr(out_result, (tU8*)pacText);
            break;
         }
      case EN_METER_VERSION_SWC:
         {
            tU8 u8SW_Version_Lower = 0;
            tU8 u8SW_Version_Mid = 0;
            tU8 u8SW_Version_Upper = 0;
            tU8 u8HW_Version_Lower = 0;
            tU8 u8HW_Version_Mid = 0;
            tU8 u8HW_Version_Upper = 0;


            m_poDataPool->u32dp_get(DPSYSTEM__METER_SW_SWC_UPPERVERSION,
               &u8SW_Version_Upper, 
               sizeof(u8SW_Version_Upper));
            m_poDataPool->u32dp_get(DPSYSTEM__METER_SW_SWC_MIDVERSION,
               &u8SW_Version_Mid, 
               sizeof(u8SW_Version_Mid));
            m_poDataPool->u32dp_get(DPSYSTEM__METER_SW_SWC_LOWERVERSION,
               &u8SW_Version_Lower, 
               sizeof(u8SW_Version_Lower));
            m_poDataPool->u32dp_get(DPSYSTEM__METER_HW_SWC_UPPERVERSION,
               &u8HW_Version_Upper, 
               sizeof(u8HW_Version_Upper));
            m_poDataPool->u32dp_get(DPSYSTEM__METER_HW_SWC_MIDVERSION,
               &u8HW_Version_Mid, 
               sizeof(u8HW_Version_Mid));
            m_poDataPool->u32dp_get(DPSYSTEM__METER_HW_SWC_LOWERVERSION,
               &u8HW_Version_Lower, 
               sizeof(u8HW_Version_Lower));

            UTF8_s32SaveNPrintFormat( pacText,MAX_GPS_FORMAT_STRLEN,"%u.%u.%u/%u.%u.%u",
               u8HW_Version_Upper, u8HW_Version_Mid, u8HW_Version_Lower,
               u8SW_Version_Upper, u8SW_Version_Mid, u8SW_Version_Lower);

            GUI_String_vSetCStr(out_result, (tU8*)pacText);
            break;
         }
      case EN_APPS_NUMBER:
         {

            bpstl::string ostrHAPVersion =" \0";
            bpstl::string ostrHUPVersion =" \0";
            bpstl::string ostrServerVersion =" \0";
            bpstl::string ostrExtnAppVersion = " \0";


            m_poDataPool->u32dp_get(
               DPSMARTPHONE__HAP_VERSION,
               &ostrHAPVersion,
               0);

            m_poDataPool->u32dp_get(
               DPSMARTPHONE__HUP_VERSION,
               &ostrHUPVersion,
               0);
            m_poDataPool->u32dp_get(
               DPSMARTPHONE__SERVER_VERSION,
               &ostrServerVersion,
               0);
            m_poDataPool->u32dp_get(
               DPSMARTPHONE__EXTNAPP_VERSION,
               &ostrExtnAppVersion,
               0);

            ostrHAPVersion.append("/");
            ostrHAPVersion.append(" ");
            ostrHAPVersion.append(ostrHUPVersion);
            ostrHAPVersion.append("/");
            ostrHAPVersion.append(" ");
            ostrHAPVersion.append(ostrExtnAppVersion);
            ostrHAPVersion.append("/");
            ostrHAPVersion.append(" ");
            ostrHAPVersion.append(ostrServerVersion);
            GUI_String_vSetCStr(out_result, (const tU8*)ostrHAPVersion.c_str());
            break;
         }
      case EN_MCAN_BOSE_AMF:
         {
            //Not implemented in HSA layer as the info required from down layer not yet implemented.
            OSAL_pvMemorySet((tVoid*)pacText,NULL,MAX_LEN_FOR_VERSION);
            UTF8_s32SaveNPrintFormat( pacText,MAX_GPS_FORMAT_STRLEN,"--.--.--");
            GUI_String_vSetCStr(out_result, (tU8*)pacText);
            break;
         }
      case EN_MCAN_TCU:
		  {
			  tU8 u8SW_Version_Lower = 0;
			  tU8 u8SW_Version_Mid = 0;
			  tU8 u8SW_Version_Upper = 0;
			  tU8 u8HW_Version_Lower = 0;
			  tU8 u8HW_Version_Mid = 0;
			  tU8 u8HW_Version_Upper = 0;


			  m_poDataPool->u32dp_get(DPTCU__SOFTWARE_VERSION_UPPER,
				  &u8SW_Version_Upper, 
				  sizeof(u8SW_Version_Upper));
			  m_poDataPool->u32dp_get(DPTCU__SOFTWARE_VERSION_MID,
				  &u8SW_Version_Mid, 
				  sizeof(u8SW_Version_Mid));
			  m_poDataPool->u32dp_get(DPTCU__SOFTWARE_VERSION_LOWER,
				  &u8SW_Version_Lower, 
				  sizeof(u8SW_Version_Lower));
			  m_poDataPool->u32dp_get(DPTCU__HARDWARE_VERSION_UPPER,
				  &u8HW_Version_Upper, 
				  sizeof(u8HW_Version_Upper));
			  m_poDataPool->u32dp_get(DPTCU__HARDWARE_VERSION_MID,
				  &u8HW_Version_Mid, 
				  sizeof(u8HW_Version_Mid));
			  m_poDataPool->u32dp_get(DPTCU__HARDWARE_VERSION_LOWER,
				  &u8HW_Version_Lower, 
				  sizeof(u8HW_Version_Lower));

			  UTF8_s32SaveNPrintFormat( pacText,MAX_GPS_FORMAT_STRLEN,"%.2x.%.2x.%.2x/%.2x.%.2x.%.2x",
				  u8HW_Version_Upper, u8HW_Version_Mid, u8HW_Version_Lower,
				  u8SW_Version_Upper, u8SW_Version_Mid, u8SW_Version_Lower);

			  GUI_String_vSetCStr(out_result, (tU8*)pacText);
			  break;
		  }
	  case EN_SYSTEM_VERSION:
		  {
			  bpstl::string oSystemVersion("0");
			  reg_tclRegKey oReg;
			  tChar pcRegSVVersionNumber[MAX_VERSION_LENGTH] = "";
			  if (oReg.bOpen("/dev/registry/LOCAL_MACHINE/SOFTWARE/BLAUPUNKT/VERSIONS") == TRUE)
			  {
				  if (oReg.bQueryString( "DC_FLASH_VERSION", pcRegSVVersionNumber, sizeof(pcRegSVVersionNumber) ) == TRUE) 
				  {
					  oSystemVersion = pcRegSVVersionNumber;
				  }
			  }
			  GUI_String_vSetCStr(out_result, (const tU8*)oSystemVersion.c_str());
			  break;
		  }
	  case EN_SDCARD_VERSION:
		  {
			bpstl::string oSDCardVersion("0");
			reg_tclRegKey oReg;
			tChar pcRegSDCardVersionNumber[MAX_VERSION_LENGTH] = "";
			if (oReg.bOpen("/dev/registry/LOCAL_MACHINE/SOFTWARE/BLAUPUNKT/VERSIONS") == TRUE)
			{
				if (oReg.bQueryString( "DC_CARD_VERSION", pcRegSDCardVersionNumber, sizeof(pcRegSDCardVersionNumber) ) == TRUE) 
				{
					oSDCardVersion = pcRegSDCardVersionNumber;
				}
			}
			GUI_String_vSetCStr(out_result, (const tU8*)oSDCardVersion.c_str());
			break;
		  }

         /*case HAP_VERSION:
         {	
         m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
         TR_CLASS_HMI_HSA_SYSTEM_TESTMODE, "HAP Version");

         ////TODO : uncomment the following code once the HAP version data is available
         //if(m_poDataPool)
         //{
         //   u32Size   = m_poDataPool->u32dp_getElementSize(DPSMARTPHONE__HAP_VERSION);
         //   m_poDataPool->u32dp_get( DPSMARTPHONE__HAP_VERSION, 
         //      (tVoid*)pacText, 
         //      u32Size	);
         //}

         //// Return HUP_VERSION
         //GUI_String_vSetCStr(out_result, (tU8*)pacText);

         //break;
         // }*/
      default:
         {
            /** At present i cannot see this Version in the OSAL registry
            Need to verify With Mr.Mehring Thomas 
            (Responsible person for Hardware and Software Versions) */
            OSAL_pvMemorySet((tVoid*)pacText,NULL,MAX_GPS_FORMAT_STRLEN);
            break;
         }
      }/* End of Switch */

      /** Trace is here */	
      if(m_poTrace)
      {	
         m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
            TR_CLASS_HMI_HSA_SYSTEM_TESTMODE,
            (tU16)(HSA_API_RAYALU__GET_VERSION ),
            MAX_GPS_FORMAT_STRLEN,
            (tU8*)pacText );
      }

      OSAL_DELETE[] pacText;
   }
}

/**
* Method: vGetTemperatureGPS
* Returns the GPS Temperature
* B1
*/
void clHSA_TestMode::vGetTemperatureGPS(GUI_String *out_result)
{
   tS32 s32GpsTemp = 0;
   tChar* pacText = OSAL_NEW tChar[MAX_GPS_FORMAT_STRLEN];
   if(pacText != NULL)
      OSAL_pvMemorySet((tVoid*)pacText,NULL,MAX_GPS_FORMAT_STRLEN);

   tChar* tempPacText = OSAL_NEW tChar[MAX_GPS_FORMAT_STRLEN];
   if(tempPacText != NULL)
      OSAL_pvMemorySet((tVoid*)tempPacText,NULL,MAX_GPS_FORMAT_STRLEN);


   m_poDataPool->u32dp_get( DPDIAGNOSIS__DEVTEMP_GPSTEMP, 
      &s32GpsTemp, 
      sizeof(tS32)	);

   UTF8_s32SaveNPrintFormat( pacText,
      MAX_GPS_FORMAT_STRLEN,
      "%2d""\xB0""\x43""\0",
      (s32GpsTemp/10)  );

   /** Convert the buffer data into required format
   as the buffer data contains the UTF8 Coded Character
   and get the number of UTF8 Bytes the String Requires */

   utfutil_u32ConvertISOMod2UTF8( tempPacText,
      MAX_GPS_FORMAT_STRLEN,
      pacText,
      MAX_GPS_FORMAT_STRLEN );

   /** Get the UTF8 formatted String into the Buffer to Display */
   UTF8_s32SaveNPrintFormat( pacText,
      MAX_GPS_FORMAT_STRLEN,
      "%s", 
      tempPacText  );                       

   /** Return GPS Temperature*/	

   GUI_String_vSetCStr(out_result, (tU8*)pacText);

   /** Trace is here */	
   if(m_poTrace)
   {	
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
         TR_CLASS_HMI_HSA_SYSTEM_TESTMODE,
         (tU16)(HSA_API_RAYALU__GET_TEMPERATURE_GPS ),
         MAX_GPS_FORMAT_STRLEN,
         (tU8*)pacText );
   }

   OSAL_DELETE[] pacText;
   OSAL_DELETE[] tempPacText;
}


/**
* Method: vGetTemperatureDisplay
* Returns the Display Temperature
* B1
*/
void clHSA_TestMode::vGetTemperatureDisplay(GUI_String *out_result)
{
   tS32 s32DispTemp = 0;
   tChar* pacText = OSAL_NEW tChar[MAX_GPS_FORMAT_STRLEN];
   if (pacText != NULL)
      OSAL_pvMemorySet((tVoid*)pacText,NULL,MAX_GPS_FORMAT_STRLEN);

   tChar* tempPacText = OSAL_NEW tChar[MAX_GPS_FORMAT_STRLEN];
   if (tempPacText != NULL)
      OSAL_pvMemorySet((tVoid*)tempPacText,NULL,MAX_GPS_FORMAT_STRLEN);

   if(NULL != m_poDataPool)
   {
      m_poDataPool->u32dp_get( DPDIAGNOSIS__DEVTEMP_BEZELTEMP, 
         &s32DispTemp, 
         sizeof(tS32)	);
   }

   UTF8_s32SaveNPrintFormat( pacText,
      MAX_GPS_FORMAT_STRLEN,
      "%2d""\xB0""\x43""\0",
      (s32DispTemp/10)  );

   /** Convert the buffer data into required format
   as the buffer data contains the UTF8 Coded Character
   and get the number of UTF8 Bytes the String Requires */

   utfutil_u32ConvertISOMod2UTF8( tempPacText,
      MAX_GPS_FORMAT_STRLEN,
      pacText,
      MAX_GPS_FORMAT_STRLEN );

   /** Get the UTF8 formatted String into the Buffer to Display */
   UTF8_s32SaveNPrintFormat( pacText,
      MAX_GPS_FORMAT_STRLEN,
      "%s", 
      tempPacText  );                       


   /** Return Display Temperature*/	

   GUI_String_vSetCStr(out_result, (tU8*)pacText);

   /** Trace is here */	
   if(m_poTrace)
   {	
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
         TR_CLASS_HMI_HSA_SYSTEM_TESTMODE,
         (tU16)(HSA_API_RAYALU__GET_TEMPERATURE_DISPLAY ),
         MAX_GPS_FORMAT_STRLEN,
         (tU8*)pacText );
   }


   OSAL_DELETE[] pacText;
   OSAL_DELETE[] tempPacText;
}


/**
* Method: vGetTemperatureAmplifier
* Returns the Amplifier Temperature
* B1
*/
void clHSA_TestMode::vGetTemperatureAmplifier(GUI_String *out_result)
{
   tS32 s32AmplifierTemp = 0;
   tChar* pacText = OSAL_NEW tChar[MAX_GPS_FORMAT_STRLEN];
   if(pacText != NULL)
      OSAL_pvMemorySet((tVoid*)pacText,NULL,MAX_GPS_FORMAT_STRLEN);

   if( NULL != m_poDataPool )
   {
      m_poDataPool->u32dp_get( DPDIAGNOSIS__DEVTEMP_BEZELTEMP, 
         &s32AmplifierTemp, 
         sizeof(tS32)	);
   }

   UTF8_s32SaveNPrintFormat( pacText,
      MAX_GPS_FORMAT_STRLEN,
      "%2d.%d\0",
      (s32AmplifierTemp/10), 
      (s32AmplifierTemp%10)    );

   /** Return Altitude*/	

   GUI_String_vSetCStr(out_result, (tU8*)pacText);

   /** Trace is here */	
   if(m_poTrace)
   {	
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
         TR_CLASS_HMI_HSA_SYSTEM_TESTMODE,
         (tU16)(HSA_API_RAYALU__GET_TEMPERATURE_AMPLIFIER ),
         MAX_GPS_FORMAT_STRLEN,
         (tU8*)pacText );
   }

   OSAL_DELETE[] pacText;
}

/**
* Method: ulwGetTunerAFListCount
* Returns the number elements in the AF list. GetTunerAF was splitted into two calls.
* B1
*/
ulword clHSA_TestMode::ulwGetTunerAFListCount(ulword Tuner)
{
   tU8 u8AltFreqNum = 0;

   /** To avoid lint warning */
   Tuner = Tuner;

   /** Read the Datapool and Return the AF List Length Value */
   if(m_poDataPool)
   {
      m_poDataPool->u32dp_get( DPTUNER__ALTFREQ_NUM, 
         &u8AltFreqNum, 
         sizeof(tU8)	);
   }

   /** Trace is here */	
   if(m_poTrace)
   {	
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
         TR_CLASS_HMI_HSA_SYSTEM_TESTMODE,
         (tU16)(HSA_API_RAYALU__GET_TUNER_AF_LIST_COUNT ),
         1,
         &u8AltFreqNum );
   }
   return u8AltFreqNum;
}

/**
* Method: vGetTunerAFListIndex
* Returns the item with the given index. GetTunerAF was splitted into two calls.
* B1
*/
void clHSA_TestMode::vGetTunerAFListIndex(GUI_String *out_result,ulword Index)
{

   /** To read the Frequency and Quality of the AF Frequency */
   tU32 u32Frequency = 0;
   tU8 u8Quality = 0;

   /** Function call to Component Manager to read the Frequency and Quality values */
   if( NULL != pclHSI_TestMode)
   {
      pclHSI_TestMode->bGetAFListElement((tU8)Index,u32Frequency,u8Quality );
   }

   /** Mask the higher three bits of u8Quality information as it not the quality information */
   u8Quality = (u8Quality)&(AFLIST_QUALITY_MASK);

   /** String trace for testing --- To be removed */
   m_poTrace->vTrace(TR_LEVEL_HMI_INFO, TR_CLASS_HSI_VEHICLE_MSG,"Values from CM TestMode are : %d, %d", u32Frequency, u8Quality);

   /** Allocate Memory dynamically */
   tChar* pacText = OSAL_NEW tChar[MAX_GPS_FORMAT_STRLEN];
   if(pacText != NULL)
      OSAL_pvMemorySet((tVoid*)pacText,NULL,MAX_GPS_FORMAT_STRLEN);

   /** Format the Values we got from Above function call into Required format */
   UTF8_s32SaveNPrintFormat( pacText,
      MAX_GPS_FORMAT_STRLEN,
      "AF:%d.%dMHz Q:%X",
      (u32Frequency/1000),
      ((u32Frequency%1000)/100),
      u8Quality );

   /** Return AF List Index String */		
   GUI_String_vSetCStr(out_result, (tU8*)pacText);

   /** Trace is here */	
   if(m_poTrace)
   {	
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
         TR_CLASS_HMI_HSA_SYSTEM_TESTMODE,
         (tU16)(HSA_API_RAYALU__GET_TUNER_AF_LIST_INDEX ),
         MAX_GPS_FORMAT_STRLEN,
         (tU8*)pacText );
   }

   /** Release the Memory allocated dynamically */
   OSAL_DELETE[] pacText;
}

/**
* Method: vSetTuneToAF
* tunes to selected AF
* B1
*/
void clHSA_TestMode::vSetTuneToAF(ulword ListEntryNr)
{
   tU8 u8Index = (tU8)ListEntryNr;

   tU8 u8ListLength = 0;
   if(m_poDataPool)
   {		
      m_poDataPool->u32dp_get( DPTUNER__ALTFREQ_NUM, 
         &u8ListLength, 
         sizeof(u8ListLength)	);
      if( u8Index < u8ListLength)
      {

         m_poDataPool -> u32dp_set( DPTUNER__ALTFREQ_INDEX,
            &u8Index,
            sizeof(u8Index) );

         vSendCommandtoHSI(HSI__TUN__SEL_AF_LISTELEMENT);

      }						
   }

   /** Trace is here */	
   if(m_poTrace)
   {	
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
         TR_CLASS_HMI_HSA_SYSTEM_TESTMODE,
         (tU16)(HSA_API_RAYALU__SET_TUNE_TO_AF ),
         1,
         &u8Index );
   }
}
/******************************************************************************
* FUNCTION:     clHSA_TestMode::vGetTunerFrequency()
*
* DESCRIPTION:  This function is used to return the current frequency for the
*               requested tuner
*
* PARAMETERS:   Tuner (FG Tuner or BG Tuner)
*
* RETURN VALUE: None 
******************************************************************************/
void clHSA_TestMode::vGetTunerFrequency(GUI_String *out_result,ulword Tuner)
{	
   vSendCommandtoHSI( HSI__TUN__FEATURE_TM_DATA_MODE_ON_FG, 
      DPDVPTSM__EVENTFLAG_MONITOR_FLAG );

   //vSendCommandtoHSI ( (tU32)CMMSG_SERVICE_MODE_TESTMODE_ON );

   //Allocate memory dynamically for the buffer to hold the formatted Strings
   tChar* tCharpacText = NULL;	
   tCharpacText = OSAL_NEW tChar[TESTMODE_DLR_GPS_STRLEN];
   if(tCharpacText != NULL)
      OSAL_pvMemorySet((tVoid*)tCharpacText,NULL,TESTMODE_DLR_GPS_STRLEN);

   tU32 u32FreqValue = 0;
   tU8  u8FMStepSize =0;

   if(NULL != m_poDataPool)
   {
      // work around to stop load of AF List.
      tBool bAFListFlag = FALSE;
      m_poDataPool->u32dp_get( DPDVPTSM__EVENTFLAG_AFLIST, 
         &bAFListFlag, 
         sizeof(bAFListFlag));
      m_poDataPool->u32dp_get(DPSYSTEM__VARIANT_CONFIG_RADIO_FM_STEP_SIZE, 
         &u8FMStepSize, sizeof(u8FMStepSize));

      if( TRUE == bAFListFlag )
      {
         m_poDataPool->u32dp_get( DPTUNER__AFLIST_TRACK,&u8TrackAFList,sizeof(tU8));


         for( tU8 u8localIndex =0; u8localIndex < u8TrackAFList; u8localIndex++ )
         {
            vSendCommandtoHSI(HSI__TUN__PREV_AF_LIST);
         }
         // Send Command to close AF List.
         vSendCommandtoHSI(HSI__TUN__CLOSE_AF_LIST);

         bAFListFlag = FALSE;
         m_poDataPool->u32dp_set( DPDVPTSM__EVENTFLAG_AFLIST, 
            &bAFListFlag, 
            sizeof(bAFListFlag));
      }



      switch(Tuner)
      {
      case TUNER_0: 
         {
            m_poDataPool->u32dp_get( DPTUNER__TESTMODE_FREQVAL_TUNER_0, 
               &u32FreqValue, 
               sizeof(tU32));
            break;
         }
      case TUNER_1:
         {
            m_poDataPool->u32dp_get( DPTUNER__TESTMODE_FREQVAL_TUNER_1, 
               &u32FreqValue, 
               sizeof(tU32));
            break;
         }
      case TUNER_2:
         {
            m_poDataPool->u32dp_get( DPTUNER__TESTMODE_FREQVAL_TUNER_2, 
               &u32FreqValue, 
               sizeof(tU32));
            break;
         }
      case TUNER_3:
         {
            m_poDataPool->u32dp_get( DPTUNER__TESTMODE_FREQVAL_TUNER_3, 
               &u32FreqValue, 
               sizeof(tU32));
            break;
         }
      default:
         {
            // Nothing to Here 
            break;
         }
      }//End of switch

      if( u32FreqValue < TUN_FM_START_FRQ )
      {//Display AM frequency
         UTF8_s32SaveNPrintFormat( tCharpacText,
            TESTMODE_DLR_GPS_STRLEN,
            "%d",
            u32FreqValue );
      }
      else
      {//Display FM frequency

         if( u8FMStepSize == (tU8) GOM_FM_STEP_SIZE)
         {
            UTF8_s32SaveNPrintFormat( tCharpacText,
               TESTMODE_DLR_GPS_STRLEN,
               "%d.%d%d",
               (u32FreqValue/1000),
               ((u32FreqValue%1000)/100),
               (((u32FreqValue%1000)%100)/10));
         }
         else
         {
            UTF8_s32SaveNPrintFormat( tCharpacText,
               TESTMODE_DLR_GPS_STRLEN,
               "%d.%d",
               (u32FreqValue/1000),
               ((u32FreqValue%1000)/100));
         }
      }
   }

   GUI_String_vSetCStr(out_result,(tU8*)tCharpacText);

   if(m_poTrace)
   {
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO, TR_CLASS_HMI_HSA_SYSTEM_TESTMODE,
         (tU16)(HSA_API_RAYALU__GET_TUNER_FREQUENCY ),
         MAX_GPS_FORMAT_STRLEN,
         (tU8*)tCharpacText );
   }

   // Release the memory Allocated dynamically 
   OSAL_DELETE[] tCharpacText;
}
/******************************************************************************
* FUNCTION:    clHSA_TestMode::vGetProgramID(Tuner)
*------------------------------------------------------------------------------
* DESCRIPTION: This API is called by GUI to display Program ID
*------------------------------------------------------------------------------
* PARAMETER:   ulword
* RETURNVALUE: ulword
******************************************************************************/
void clHSA_TestMode::vGetProgramID(GUI_String *out_result,ulword Tuner)
{
	//extract HD test mode data from server
   vSendCommandtoHSI(HSI__TUN__FEATURE_HD_TEST_MODE_DATA);


   tU32 u32ProgramIDValue = 0;
   tUTF8 utf8Data[MAX_NO_OF_CHAR_FM] = "\0";
   if( NULL != m_poDataPool)
   {
      switch(Tuner)
      {
      case TUNER_0:
      case TUNER_1:
         {
            m_poDataPool->u32dp_get( DPTUNER__TESTMODE_HD_PRG_ID, &u32ProgramIDValue, sizeof(tU32));
            break;
         }

      default:
         {
            // Nothing to do Here
            break;
         }
      }
   }

   UTF8_s32SaveNPrintFormat( utf8Data, MAX_NO_OF_CHAR_FM, "%X", u32ProgramIDValue);
   GUI_String_vSetCStr(out_result,(tU8*)utf8Data);

   /** Trace is here */
   if(m_poTrace != NULL)
   {
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
         TR_CLASS_HMI_HSA_SYSTEM_TESTMODE,
         "Program ID send to GUI in Test Mode is %d",
         u32ProgramIDValue);
   }
}

/******************************************************************************
* FUNCTION:    clHSA_TestMode::vGetCdNo(Tuner)
*------------------------------------------------------------------------------
* DESCRIPTION: This API is called by GUI to display CD number
*------------------------------------------------------------------------------
* PARAMETER:   ulword
* RETURNVALUE: ulword
******************************************************************************/
void clHSA_TestMode::vGetCdNo(GUI_String *out_result,ulword Tuner)
{
	//extract HD test mode data from server
   vSendCommandtoHSI( HSI__TUN__FEATURE_HD_TEST_MODE_DATA);

   tU8 u8CdNoValue = 0;
   tUTF8 utf8Data[MAX_NO_OF_CHAR_FM] = "\0";
   if( NULL != m_poDataPool)
   {
      switch(Tuner)
      {
      case TUNER_0:
      case TUNER_1:
         {
            m_poDataPool->u32dp_get( DPTUNER__TESTMODE_HD_CD_NO, &u8CdNoValue, sizeof(tU8));
            break;
         }

      default:
         {
            // Nothing to do Here
            break;
         }
      }
   }

   UTF8_s32SaveNPrintFormat( utf8Data, MAX_NO_OF_CHAR_FM, "%X", u8CdNoValue);
   GUI_String_vSetCStr(out_result,(tU8*)utf8Data);

   /** Trace is here */
   if(m_poTrace != NULL)
   {
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
         TR_CLASS_HMI_HSA_SYSTEM_TESTMODE,
         "CD No send to GUI in Test Mode is %d",
         u8CdNoValue);
   }
}


ulword clHSA_TestMode::ulwGetTunerFS(ulword Tuner)
{	
   /** This is work around to send the event to HSI
   When Framework provides this trigger points
   we have to remove this function call as well as definition
   and we can remove the datapool ID's used in this function 
   from DVPTSM.xml */
   vSendCommandtoHSI( HSI__TUN__FEATURE_TM_DATA_MODE_ON_FG, 
      DPDVPTSM__EVENTFLAG_MONITOR_FLAG );


   tU8 u8FSValue = 0;
   switch(Tuner)
   {
   case TUNER_0: 
      {
         if(m_poDataPool)
         {
            m_poDataPool->u32dp_get( DPTUNER__TESTMODE_FIELDSTRENGTH_TUNER_0, &u8FSValue, sizeof(u8FSValue));

         }
         break;
      }
   case TUNER_1:
      {
         if(m_poDataPool)
         {
            m_poDataPool->u32dp_get( DPTUNER__TESTMODE_FIELDSTRENGTH_TUNER_1, &u8FSValue, sizeof(u8FSValue));
         }
         break;
      }
   case TUNER_2:
      {
         if(m_poDataPool)
         {
            m_poDataPool->u32dp_get( DPTUNER__TESTMODE_FIELDSTRENGTH_TUNER_2, &u8FSValue, sizeof(u8FSValue));
         }
         break;
      }
   case TUNER_3:
      {
         if(m_poDataPool)
         {
            m_poDataPool->u32dp_get( DPTUNER__TESTMODE_FIELDSTRENGTH_TUNER_3, &u8FSValue, sizeof(u8FSValue));
         }
         break;
      } 
   default:
      {
         //Nothing to Do here
         break;
      }
   }/* end of switch */

   /*
   Checking for the Max FS value. 
   If the FS value is greater than the MAX_FS_VALUE(99), assigning the Max FS value that is allowed.

   */

   if ( u8FSValue > MAX_FS_VALUE )
   {
      u8FSValue = MAX_FS_VALUE;
   }

   /** Trace is here */	
   if(m_poTrace)
   {	
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
         TR_CLASS_HMI_HSA_SYSTEM_TESTMODE,
         (tU16)(HSA_API_RAYALU__GET_TUNER_FS ),
         1,
         &u8FSValue );
   }
   return u8FSValue;
}

ulword clHSA_TestMode::ulwGetTunerTA(ulword Tuner)
{	
   /** This is work around to send the event to HSI
   When Framework provides this trigger points
   we have to remove this function call as well as definition
   and we can remove the datapool ID's used in this function 
   from DVPTSM.xml */
   vSendCommandtoHSI( HSI__TUN__FEATURE_TM_DATA_MODE_ON_FG, 
      DPDVPTSM__EVENTFLAG_MONITOR_FLAG );


   tU8 u8TAValue = 0;
   if( NULL != m_poDataPool)
   {
      switch(Tuner)
      {
      case TUNER_0: 
         {
            m_poDataPool->u32dp_get( DPTUNER__TESTMODE_TA_TUNER_0, &u8TAValue, sizeof(u8TAValue));
            break;
         }
      case TUNER_1:
         {
            m_poDataPool->u32dp_get( DPTUNER__TESTMODE_TA_TUNER_1, &u8TAValue, sizeof(u8TAValue));
            break;
         }
      case TUNER_2:
         {
            m_poDataPool->u32dp_get( DPTUNER__TESTMODE_TA_TUNER_2, &u8TAValue, sizeof(u8TAValue));
            break;
         }
      case TUNER_3:
         {
            m_poDataPool->u32dp_get( DPTUNER__TESTMODE_TA_TUNER_3, &u8TAValue, sizeof(u8TAValue));
            break;
         }
      default:
         {
            //Nothing to do here
            break;
         }
      }/* end of switch */
   }


   /*

   Checking for the TA value. 
   If the TA value is greater than the MAX_TA_VALUE(1), assigning the Max TA value that is allowed.

   */

   if ( u8TAValue > MAX_TA_VALUE )
   {
      u8TAValue = MAX_TA_VALUE;
   }

   /** Trace is here */	
   if(m_poTrace)
   {	
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
         TR_CLASS_HMI_HSA_SYSTEM_TESTMODE,
         (tU16)(HSA_API_RAYALU__GET_TUNER_TA ),
         1,
         &u8TAValue );
   }
   return (ulword)u8TAValue;
}


ulword clHSA_TestMode::ulwGetTunerSharx(ulword Tuner)
{	
   /** This is work around to send the event to HSI
   When Framework provides this trigger points
   we have to remove this function call as well as definition
   and we can remove the datapool ID's used in this function 
   from DVPTSM.xml */
   vSendCommandtoHSI( HSI__TUN__FEATURE_TM_DATA_MODE_ON_FG, 
      DPDVPTSM__EVENTFLAG_MONITOR_FLAG );


   tU8 u8SharxValue = 0;
   if( NULL != m_poDataPool )
   {
      switch(Tuner)
      {
      case TUNER_0: 
         {
            m_poDataPool->u32dp_get( DPTUNER__TESTMODE_SHARX_TUNER_0, &u8SharxValue, sizeof(u8SharxValue));
            break;
         }
      case TUNER_1:
         {
            m_poDataPool->u32dp_get( DPTUNER__TESTMODE_SHARX_TUNER_1, &u8SharxValue, sizeof(u8SharxValue));
            break;
         }
      case TUNER_2:
         {
            m_poDataPool->u32dp_get( DPTUNER__TESTMODE_SHARX_TUNER_2, &u8SharxValue, sizeof(u8SharxValue));
            break;
         }
      case TUNER_3:
         {
            m_poDataPool->u32dp_get( DPTUNER__TESTMODE_SHARX_TUNER_3, &u8SharxValue, sizeof(u8SharxValue));
            break;
         }
      default: 
         {
            //Nothing to do here
            break;
         }
      }/* end of switch */
   }

   /*

   Checking for the SHARX value. 
   If the SHARX value is greater than the MAX_SHARX_VALUE(0X3F), assigning the Max SHARX value that is allowed.

   */

   if ( u8SharxValue > MAX_SHARX_VALUE )
   {
      u8SharxValue = MAX_SHARX_VALUE;
   }

   /** Trace is here */	
   if(m_poTrace)
   {	
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
         TR_CLASS_HMI_HSA_SYSTEM_TESTMODE,
         (tU16)(HSA_API_RAYALU__GET_TUNER_SHARX ),
         1,
         &u8SharxValue );
   }
   return (ulword)u8SharxValue;
}


ulword clHSA_TestMode::ulwGetTunerAF(ulword Tuner)
{	
   /** This is work around to send the event to HSI
   When Framework provides this trigger points
   we have to remove this function call as well as definition
   and we can remove the datapool ID's used in this function 
   from DVPTSM.xml */
   vSendCommandtoHSI( HSI__TUN__FEATURE_TM_DATA_MODE_ON_FG, 
      DPDVPTSM__EVENTFLAG_MONITOR_FLAG );


   tU8 u8AFValue = 0;
   if( NULL != m_poDataPool)
   {
      switch(Tuner)
      {
      case TUNER_0: 
         {
            m_poDataPool->u32dp_get( DPTUNER__TESTMODE_AFLISTLENGTH_TUNER_0, &u8AFValue, sizeof(u8AFValue));
            break;
         }
      case TUNER_1:
         {
            m_poDataPool->u32dp_get( DPTUNER__TESTMODE_AFLISTLENGTH_TUNER_1, &u8AFValue, sizeof(u8AFValue));
            break;
         }
      case TUNER_2:
         {
            m_poDataPool->u32dp_get( DPTUNER__TESTMODE_AFLISTLENGTH_TUNER_2, &u8AFValue, sizeof(u8AFValue));
            break;
         }
      case TUNER_3:
         {
            m_poDataPool->u32dp_get( DPTUNER__TESTMODE_AFLISTLENGTH_TUNER_3, &u8AFValue, sizeof(u8AFValue));
            break;
         }
      default: 
         {
            //Nothing to do here
            break;
         }
      }/* end of switch */
   }
   /** Trace is here */	

   /*

   Checking for the AFLISTLENGTH value. 
   If the AFLISTLENGTH value is greater than the MAX_AFLIST_LENGTH(99), assigning the Max AFLISTLENGTH value that is allowed.

   */

   if ( u8AFValue > MAX_AFLIST_LENGTH )
   {
      u8AFValue = MAX_AFLIST_LENGTH;
   }


   if(m_poTrace)
   {	
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
         TR_CLASS_HMI_HSA_SYSTEM_TESTMODE,
         (tU16)(HSA_API_RAYALU__GET_TUNER_AF ),
         1,
         &u8AFValue );
   }
   return (ulword)u8AFValue;
}


ulword clHSA_TestMode::ulwGetTunerHC(ulword Tuner)
{	
   /** This is work around to send the event to HSI
   When Framework provides this trigger points
   we have to remove this function call as well as definition
   and we can remove the datapool ID's used in this function 
   from DVPTSM.xml */
   vSendCommandtoHSI( HSI__TUN__FEATURE_TM_DATA_MODE_ON_FG, 
      DPDVPTSM__EVENTFLAG_MONITOR_FLAG );


   tU8 u8HCValue = 0;
   if( NULL != m_poDataPool)
   {
      switch(Tuner)
      {
      case TUNER_0: 
         {
            m_poDataPool->u32dp_get( DPTUNER__TESTMODE_HICUT_TUNER_0, &u8HCValue, sizeof(u8HCValue));
            break;
         }
      case TUNER_1:
         {
            m_poDataPool->u32dp_get( DPTUNER__TESTMODE_HICUT_TUNER_1, &u8HCValue, sizeof(u8HCValue));
            break;
         }
      case TUNER_2:
         {
            m_poDataPool->u32dp_get( DPTUNER__TESTMODE_HICUT_TUNER_2, &u8HCValue, sizeof(u8HCValue));
            break;
         }
      case TUNER_3:
         {
            m_poDataPool->u32dp_get( DPTUNER__TESTMODE_HICUT_TUNER_3, &u8HCValue, sizeof(u8HCValue));
            break;
         }
      default:
         {
            // Nothing to do Here
            break;
         }
      }/* end of switch */
   }

   /*

   Checking for the HIGHCUT value. 
   If the HIGHCUT value is greater than the MAX_HIGHCUT_VALUE(7), assigning the Max HIGHCUT value that is allowed.

   */

   if ( u8HCValue > MAX_HIGHCUT_VALUE )
   {
      u8HCValue = MAX_HIGHCUT_VALUE;
   }

   /** Trace is here */	
   if(m_poTrace != NULL)
   {	
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
         TR_CLASS_HMI_HSA_SYSTEM_TESTMODE,
         (tU16)(HSA_API_RAYALU__GET_TUNER_HC ),
         1,
         &u8HCValue );
   }
   return (ulword)u8HCValue;
}

//API called on the exit from the AF List
void clHSA_TestMode::vExitAFList()
{
   //Send HSI event to tuner to close AF list
   vSendCommandtoHSI(HSI__TUN__CLOSE_AF_LIST);
}

//API for the tuner calibration data
void clHSA_TestMode::vGetTunerCalData(GUI_String *out_result,ulword Calibration)
{
   tU16 u16TunerCal=0;
   tChar* tCharpacText = NULL;
   tCharpacText = OSAL_NEW tChar[TESTMODE_DLR_GPS_STRLEN];
   if(tCharpacText != NULL)
      OSAL_pvMemorySet((tVoid*)tCharpacText,NULL,TESTMODE_DLR_GPS_STRLEN);
   if (bReqTunerCalData == false)
   {
      vSendCommandtoHSI(HSI__TUN__FEATURE_CALIBRATION_DATA);
   }
   else
   {
      //Nothing to be done
   }

   switch(Calibration)
   {
   case CAL_FM_0:
      {
         m_poDataPool->u32dp_get( DPTUNER__FEATURE_CALIBRATIONDATA_FM_TUNER_0, &u16TunerCal, sizeof(u16TunerCal));
         break;
      }
   case CAL_FM_1:
      {
         m_poDataPool->u32dp_get( DPTUNER__FEATURE_CALIBRATIONDATA_FM_TUNER_1, &u16TunerCal, sizeof(u16TunerCal));
         break;
      }
   case CAL_FM_2:
      {
         m_poDataPool->u32dp_get( DPTUNER__FEATURE_CALIBRATIONDATA_FM_TUNER_2, &u16TunerCal, sizeof(u16TunerCal));
         break;
      }
   case CAL_FM_3:
      {
         m_poDataPool->u32dp_get( DPTUNER__FEATURE_CALIBRATIONDATA_FM_TUNER_3, &u16TunerCal, sizeof(u16TunerCal));
         break;
      }
   case CAL_AM:
      {
         m_poDataPool->u32dp_get( DPTUNER__FEATURE_CALIBRATIONDATA_AM, &u16TunerCal, sizeof(u16TunerCal));
         break;
      }
   default:
      {
         // Nothing to do Here
         break;
      }
   }/* end of switch */
   bReqTunerCalData = true;
   UTF8_s32SaveNPrintFormat( tCharpacText,
      TESTMODE_DLR_GPS_STRLEN,
      "%X",
      u16TunerCal);

   GUI_String_vSetCStr(out_result,(tU8*)tCharpacText);

   OSAL_DELETE[] tCharpacText;
   //return (ulword)u16TunerCal;

}




void clHSA_TestMode::vGetTunerFrequencyUnit(GUI_String *out_result,ulword Tuner)
{
   vSendCommandtoHSI( HSI__TUN__FEATURE_TM_DATA_MODE_ON_FG, 
      DPDVPTSM__EVENTFLAG_MONITOR_FLAG );

   tU32 u32FreqValue = 0;    // To hold the current frq value
   tUTF8 utf8Data[MAX_NO_OF_CHAR_FM] = "\0";    // To hold the unit
   if( NULL != m_poDataPool )
   {

      switch(Tuner)
      {
      case TUNER_0: 
         {
            m_poDataPool->u32dp_get( DPTUNER__TESTMODE_FREQVAL_TUNER_0, &u32FreqValue, sizeof(tU32));
            break;
         }
      case TUNER_1:
         {
            m_poDataPool->u32dp_get( DPTUNER__TESTMODE_FREQVAL_TUNER_1, &u32FreqValue, sizeof(tU32));
            break;
         }
      case TUNER_2:
         {
            m_poDataPool->u32dp_get( DPTUNER__TESTMODE_FREQVAL_TUNER_2, &u32FreqValue, sizeof(tU32));
            break;
         }
      case TUNER_3:
         {
            m_poDataPool->u32dp_get( DPTUNER__TESTMODE_FREQVAL_TUNER_3, &u32FreqValue, sizeof(tU32));
            break;
         }
      default:
         {
            break;
         }
      }// end of switch

      if((u32FreqValue >= TUN_FM_START_FRQ) && (u32FreqValue <= TUN_FM_END_FRQ))
      {
         UTF8_s32SaveNPrintFormat( utf8Data, MAX_NO_OF_CHAR_FM, "%s", "MHz"); 
      } 
      else if((u32FreqValue >= TUN_AM_START_FRQ) && (u32FreqValue <= TUN_AM_END_FRQ))
      {				 
         UTF8_s32SaveNPrintFormat( utf8Data,MAX_NO_OF_CHAR_FM, "%s", "KHz");
      }
      else
      {
      }

      GUI_String_vSetCStr(out_result,(tU8*)utf8Data);
   }

   /** Trace is here */	
   if(m_poTrace != NULL)
   {
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
         TR_CLASS_HMI_HSA_SYSTEM_TESTMODE,
         (tU16)(HSA_API_RAYALU__GET_TUNER_FREQUENCY_UNIT ),
         MAX_NO_OF_CHAR_FM,
         (tU8*)utf8Data );
   }		
}

void clHSA_TestMode::vGetTunerNeighbourChannel(GUI_String *out_result,ulword Tuner)
{		
   vSendCommandtoHSI( HSI__TUN__FEATURE_TM_DATA_MODE_ON_FG, 
      DPDVPTSM__EVENTFLAG_MONITOR_FLAG );


   tU8 u8NbChannel = 0;
   tChar* tCharpacText = NULL;
   tCharpacText = OSAL_NEW tChar[TESTMODE_DLR_GPS_STRLEN];
   if(tCharpacText != NULL)
      OSAL_pvMemorySet((tVoid*)tCharpacText,NULL,TESTMODE_DLR_GPS_STRLEN);
   if( NULL != m_poDataPool)
   {
      switch(Tuner)
      {
      case TUNER_0: 
         {
            m_poDataPool->u32dp_get( DPTUNER__TESTMODE_NEIGHBOUR_TUNER_0,
               &u8NbChannel, 
               sizeof(u8NbChannel) );


            break;
         }
      case TUNER_1:
         {
            m_poDataPool->u32dp_get( DPTUNER__TESTMODE_NEIGHBOUR_TUNER_1,
               &u8NbChannel, 
               sizeof(u8NbChannel) );


            break;
         }
      case TUNER_2:
         {
            m_poDataPool->u32dp_get( DPTUNER__TESTMODE_NEIGHBOUR_TUNER_2,
               &u8NbChannel,
               sizeof(u8NbChannel) );


            break;
         }
      case TUNER_3:
         {
            m_poDataPool->u32dp_get( DPTUNER__TESTMODE_NEIGHBOUR_TUNER_3,
               &u8NbChannel, 
               sizeof(u8NbChannel) );


            break;
         }
      default:
         {
            //Nothing to do Here
            break;
         }
      }//End of switch
   }

   /*

   Checking for the TunerNeighbourChannel value. 
   If the TunerNeighbourChannel value is greater than the MAX_NEIGHBOUR_CHANNEL_VALUE(0X3F), assigning the Max Neighbour Channel  value that is allowed.

   */

   if ( u8NbChannel > MAX_NEIGHBOUR_CHANNEL_VALUE )
   {
      u8NbChannel = MAX_NEIGHBOUR_CHANNEL_VALUE;
   }

   UTF8_s32SaveNPrintFormat( tCharpacText,
      TESTMODE_DLR_GPS_STRLEN,
      "%X",
      u8NbChannel);

   GUI_String_vSetCStr(out_result,(tU8*)tCharpacText);

   if(m_poTrace)
   {	
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
         TR_CLASS_HMI_HSA_SYSTEM_TESTMODE,
         (tU16)(HSA_API_RAYALU__GET_TUNER_NEIGHBOUR_CHANNEL ),
         MAX_GPS_FORMAT_STRLEN,
         (tU8*)tCharpacText );
   }


   /** Release the memory Allocated dynamically */
   OSAL_DELETE[] tCharpacText;
}

ulword clHSA_TestMode::ulwGetTunerRDSErrorRate(ulword Tuner)
{	
   /** This is work around to send the event to HSI
   When Framework provides this trigger points
   we have to remove this function call as well as definition
   and we can remove the datapool ID's used in this function 
   from DVPTSM.xml */
   /*vSendCommandtoHSI( HSI__TUN__FEATURE_TM_DATA_MODE_ON_FG, 
   DPDVPTSM__EVENTFLAG_MONITOR_FLAG );*/


   tU8 u8RDSErrorRate = 0;
   if( NULL != m_poDataPool)
   {
      switch(Tuner)
      {
      case TUNER_0: 
         {
            m_poDataPool->u32dp_get( DPTUNER__TESTMODE_RDSERRORS_TUNER_0, &u8RDSErrorRate, sizeof(u8RDSErrorRate));
            break;
         }
      case TUNER_1:
         {
            m_poDataPool->u32dp_get( DPTUNER__TESTMODE_RDSERRORS_TUNER_1, &u8RDSErrorRate, sizeof(u8RDSErrorRate));
            break;
         }
      case TUNER_2:
         {
            m_poDataPool->u32dp_get( DPTUNER__TESTMODE_RDSERRORS_TUNER_2, &u8RDSErrorRate, sizeof(u8RDSErrorRate));
            break;
         }
      case TUNER_3:
         {
            m_poDataPool->u32dp_get( DPTUNER__TESTMODE_RDSERRORS_TUNER_3, &u8RDSErrorRate, sizeof(u8RDSErrorRate));
            break;
         }
      default:
         {
            // Nothing to do here
            break;
         }
      }/* end of switch */
   }

   /*
   Checking for the Max RDS Error Rate value. 
   If the RDS Error rate value is greater than the MAX_RDS_ERROR_RATE_VALUE(99), assigning the Max RDS Error rate value that is allowed.

   */

   if ( u8RDSErrorRate > MAX_RDS_ERROR_RATE_VALUE )
   {
      u8RDSErrorRate = MAX_RDS_ERROR_RATE_VALUE;
   }

   /** Trace is here */	
   if(m_poTrace)
   {	
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
         TR_CLASS_HMI_HSA_SYSTEM_TESTMODE,
         (tU16)(HSA_API_RAYALU__GET_TUNER_RDS_ERROR_RATE ),
         1,
         &u8RDSErrorRate );
   }	
   return (ulword)u8RDSErrorRate;
}


void clHSA_TestMode::vGetTunerMultiPath(GUI_String *out_result,ulword Tuner)
{	
   /** This is work around to send the event to HSI
   When Framework provides this trigger points
   we have to remove this function call as well as definition
   and we can remove the datapool ID's used in this function 
   from DVPTSM.xml */
   vSendCommandtoHSI( HSI__TUN__FEATURE_TM_DATA_MODE_ON_FG, 
      DPDVPTSM__EVENTFLAG_MONITOR_FLAG );

   tChar* tCharpacText = NULL;
   tCharpacText = OSAL_NEW tChar[TESTMODE_DLR_GPS_STRLEN];
   if(tCharpacText != NULL)
      OSAL_pvMemorySet((tVoid*)tCharpacText,NULL,TESTMODE_DLR_GPS_STRLEN);
   tU8 u8MultiPath = 0;
   if( NULL != m_poDataPool)
   {
      switch(Tuner)
      {
      case TUNER_0: 
         {
            m_poDataPool->u32dp_get( DPTUNER__TESTMODE_MULTIPATH_TUNER_0,
               &u8MultiPath, 
               sizeof(u8MultiPath));
            break;
         }

      case TUNER_1:
         {			
            m_poDataPool->u32dp_get( DPTUNER__TESTMODE_MULTIPATH_TUNER_1,
               &u8MultiPath, 
               sizeof(u8MultiPath));
            break;
         }
      case TUNER_2:
         {
            m_poDataPool->u32dp_get( DPTUNER__TESTMODE_MULTIPATH_TUNER_2,
               &u8MultiPath,
               sizeof(u8MultiPath));
            break;
         }
      case TUNER_3:
         {
            m_poDataPool->u32dp_get( DPTUNER__TESTMODE_MULTIPATH_TUNER_3,
               &u8MultiPath,
               sizeof(u8MultiPath));
            break;
         }

      default:
         {
            // Nothing to do Here
            break;
         }
      }//End of switch
      UTF8_s32SaveNPrintFormat( tCharpacText,
         TESTMODE_DLR_GPS_STRLEN,
         "%X",
         u8MultiPath );

      GUI_String_vSetCStr(out_result,(tU8*)tCharpacText);
   }

   if(m_poTrace)
   {	
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
         TR_CLASS_HMI_HSA_SYSTEM_TESTMODE,
         (tU16)(HSA_API_RAYALU__GET_TUNER_MULTI_PATH ),
         MAX_GPS_FORMAT_STRLEN,
         (tU8*)tCharpacText );
   }


   /** Release the memory Allocated dynamically */
   OSAL_DELETE[] tCharpacText;
}


void clHSA_TestMode::vGetTunerPI(GUI_String *out_result,ulword Tuner)
{	
   /** This is work around to send the event to HSI
   When Framework provides this trigger points
   we have to remove this function call as well as definition
   and we can remove the datapool ID's used in this function 
   from DVPTSM.xml */
   vSendCommandtoHSI( HSI__TUN__FEATURE_TM_DATA_MODE_ON_FG, 
      DPDVPTSM__EVENTFLAG_MONITOR_FLAG );


   tU32 u32PIValue = 0;
   tUTF8 utf8Data[MAX_NO_OF_CHAR_FM] = "\0";
   if( NULL != m_poDataPool)
   {
      switch(Tuner)
      {
      case TUNER_0: 
         {
            m_poDataPool->u32dp_get( DPTUNER__TESTMODE_PI_TUNER_0, &u32PIValue, sizeof(tU32));
            UTF8_s32SaveNPrintFormat( utf8Data, MAX_NO_OF_CHAR_FM, "%X", u32PIValue);
            GUI_String_vSetCStr(out_result,(tU8*)utf8Data);
            break;
         }

      case TUNER_1:
         {
            m_poDataPool->u32dp_get( DPTUNER__TESTMODE_PI_TUNER_1, &u32PIValue, sizeof(tU32));
            UTF8_s32SaveNPrintFormat( utf8Data, MAX_NO_OF_CHAR_FM, "%X", u32PIValue);
            GUI_String_vSetCStr(out_result,(tU8*)utf8Data);
            break;
         }
      case TUNER_2:
         {
            m_poDataPool->u32dp_get( DPTUNER__TESTMODE_PI_TUNER_2, &u32PIValue, sizeof(tU32));
            UTF8_s32SaveNPrintFormat( utf8Data, MAX_NO_OF_CHAR_FM, "%X", u32PIValue);
            GUI_String_vSetCStr(out_result,(tU8*)utf8Data);
            break;
         }
      case TUNER_3:
         {
            m_poDataPool->u32dp_get( DPTUNER__TESTMODE_PI_TUNER_3, &u32PIValue, sizeof(tU32));
            UTF8_s32SaveNPrintFormat( utf8Data, MAX_NO_OF_CHAR_FM, "%X", u32PIValue);
            GUI_String_vSetCStr(out_result,(tU8*)utf8Data);
            break;
         }

      default:
         {
            // Nothing to do Here
            break;
         }
      }
   } 



   /** Trace is here */	
   if(m_poTrace != NULL)
   {	
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
         TR_CLASS_HMI_HSA_SYSTEM_TESTMODE,
         (tU16)(HSA_API_RAYALU__GET_TUNER_PI ),
         MAX_NO_OF_CHAR_FM,
         (tU8*)utf8Data );
   }			
}


void clHSA_TestMode::vGetMatchedPositionValues(GUI_String *out_result,ulword ListEntryNr)
{
   /** Allocate memory dynamically for the 
   Buffer to hold the formatted Strings */
   tChar* tCharpacText = NULL;
   tCharpacText = OSAL_NEW tChar[TESTMODE_DLR_GPS_STRLEN];

   tS32 s32temp = 0;

   switch(ListEntryNr)
   {
   case TESTMODE_MAPMATCH_LATITUDE:
      {
         if(tCharpacText != NULL)
            OSAL_pvMemorySet((tVoid*)tCharpacText,NULL,TESTMODE_DLR_GPS_STRLEN);
         /** Buffer to store the data temperarily */
         tUTF8 utf8FormattedData[TESTMODE_DLR_GPS_STRLEN] = "\0";
         tUTF8 utf8IsoToUtf8Data[TESTMODE_DLR_GPS_STRLEN] = "\0";
         tUTF8 utf8FormattedDegData[TESTMODE_DLR_GPS_INTEGER] = "\0";
         tUTF8 utf8FormattedMinData[TESTMODE_DLR_GPS_INTEGER] = "\0";
         tUTF8 utf8FormattedSecData[TESTMODE_DLR_GPS_INTEGER] = "\0";
         tUTF8 utf8FormattedTenthSecData[TESTMODE_DLR_GPS_INTEGER] = "\0";

         if(m_poDataPool)  
            m_poDataPool->u32dp_get( DPNAVI__CURRENT_VEHICLE_POS_LATITUDE, 
            &s32temp, 
            sizeof(tS32) );

         /** Get the Longitude Data into a Structure by conversion*/
         sMapMatch_Info prLongInfo = sMatchedPosValueConversion(s32temp);

         if( prLongInfo.u8Degree < TESTMODE_DLR_GPS_ONEDIGIT)
         {
            UTF8_s32SaveNPrintFormat( utf8FormattedDegData,
               TESTMODE_DLR_GPS_INTEGER,
               "\x30""%d",
               prLongInfo.u8Degree );
         }
         else
         {
            UTF8_s32SaveNPrintFormat( utf8FormattedDegData,
               TESTMODE_DLR_GPS_INTEGER, 
               "%d",
               prLongInfo.u8Degree );
         }

         if( prLongInfo.u8Minutes < TESTMODE_DLR_GPS_ONEDIGIT)
         {
            UTF8_s32SaveNPrintFormat( utf8FormattedMinData,
               TESTMODE_DLR_GPS_INTEGER,
               "\x30""%d",
               prLongInfo.u8Minutes );
         }
         else
         {
            UTF8_s32SaveNPrintFormat( utf8FormattedMinData,
               TESTMODE_DLR_GPS_INTEGER,
               "%d",
               prLongInfo.u8Minutes );
         }
         if( prLongInfo.u8Seconds < TESTMODE_DLR_GPS_ONEDIGIT)
         {
            UTF8_s32SaveNPrintFormat( utf8FormattedSecData,
               TESTMODE_DLR_GPS_INTEGER,
               "\x30""%d",
               prLongInfo.u8Seconds );
         }
         else
         {
            UTF8_s32SaveNPrintFormat( utf8FormattedSecData,
               TESTMODE_DLR_GPS_INTEGER,
               "%d",
               prLongInfo.u8Seconds );
         }
         UTF8_s32SaveNPrintFormat( utf8FormattedTenthSecData,
            TESTMODE_DLR_GPS_INTEGER,
            "%d",
            prLongInfo.u8TenthSeconds );


         if( prLongInfo.bDirection == TRUE )
         {
            /** Convert Raw Gps Longitude Info into Buffer for formatting */
            UTF8_s32SaveNPrintFormat( utf8FormattedData,
               TESTMODE_DLR_GPS_STRLEN,
               "N%s""\xB0""%s""\x2E""%s""\x22", //Fix to mms 312838
               utf8FormattedDegData,
               utf8FormattedMinData,
               utf8FormattedSecData	);
         }
         else
         {
            /** Convert Raw Gps Longitude Info into Buffer for formatting */
            UTF8_s32SaveNPrintFormat( utf8FormattedData,
               TESTMODE_DLR_GPS_STRLEN,
               "S%s""\xB0""%s""\x2E""%s""\x22", //Fix to mms 312838
               utf8FormattedDegData,
               utf8FormattedMinData,
               utf8FormattedSecData );
         }

         /** Convert the buffer data into required format 
         as the buffer data contains the UTF8 Coded Character
         and get the number of UTF8 Bytes the String Requires */

         utfutil_u32ConvertISOMod2UTF8( utf8IsoToUtf8Data,
            TESTMODE_DLR_GPS_STRLEN,
            utf8FormattedData,
            TESTMODE_DLR_GPS_STRLEN );

         /** Get the UTF8 formatted String into the Buffer to Display */
         UTF8_s32SaveNPrintFormat( tCharpacText,
            TESTMODE_DLR_GPS_STRLEN,
            "%s", 
            utf8IsoToUtf8Data  );


         /** Return Altitude*/	
         GUI_String_vSetCStr(out_result, (tU8*)tCharpacText);

         break;
      }
   case TESTMODE_MAPMATCH_LONGITUDE:
      {
         if (tCharpacText != NULL)
            OSAL_pvMemorySet((tVoid*)tCharpacText,NULL,TESTMODE_DLR_GPS_STRLEN);	
         /** Buffer to store the data temperarily */
         tUTF8 utf8FormattedData[TESTMODE_DLR_GPS_STRLEN] = "\0";
         tUTF8 utf8IsoToUtf8Data[TESTMODE_DLR_GPS_STRLEN] = "\0";
         tUTF8 utf8FormattedDegData[TESTMODE_DLR_GPS_INTEGER] = "\0";
         tUTF8 utf8FormattedMinData[TESTMODE_DLR_GPS_INTEGER] = "\0";
         tUTF8 utf8FormattedSecData[TESTMODE_DLR_GPS_INTEGER] = "\0";
         tUTF8 utf8FormattedTenthSecData[TESTMODE_DLR_GPS_INTEGER] = "\0";

         if(m_poDataPool)  
            m_poDataPool->u32dp_get( DPNAVI__CURRENT_VEHICLE_POS_LONGITUDE, 
            &s32temp, 
            sizeof(tS32) );

         /** Get the Longitude Data into a Structure by conversion*/
         sMapMatch_Info prLongInfo = sMatchedPosValueConversion(s32temp);

         if( prLongInfo.u8Degree < TESTMODE_DLR_GPS_ONEDIGIT)
         {
            UTF8_s32SaveNPrintFormat( utf8FormattedDegData,
               TESTMODE_DLR_GPS_INTEGER,
               "\x30\x30""%d",
               prLongInfo.u8Degree );
         }
         else if( (prLongInfo.u8Degree >= TESTMODE_DLR_GPS_ONEDIGIT) && 
            (prLongInfo.u8Degree < TESTMODE_DLR_GPS_TWODIGIT) )
         {
            UTF8_s32SaveNPrintFormat( utf8FormattedDegData,
               TESTMODE_DLR_GPS_INTEGER,
               "\x30""%d",
               prLongInfo.u8Degree );
         }
         else
         {
            UTF8_s32SaveNPrintFormat( utf8FormattedDegData,
               TESTMODE_DLR_GPS_INTEGER, 
               "%d",
               prLongInfo.u8Degree );
         }

         if( prLongInfo.u8Minutes < TESTMODE_DLR_GPS_ONEDIGIT)
         {
            UTF8_s32SaveNPrintFormat( utf8FormattedMinData,
               TESTMODE_DLR_GPS_INTEGER,
               "\x30""%d",
               prLongInfo.u8Minutes );
         }
         else
         {
            UTF8_s32SaveNPrintFormat( utf8FormattedMinData,
               TESTMODE_DLR_GPS_INTEGER,
               "%d",
               prLongInfo.u8Minutes );
         }
         if( prLongInfo.u8Seconds < TESTMODE_DLR_GPS_ONEDIGIT)
         {
            UTF8_s32SaveNPrintFormat( utf8FormattedSecData,
               TESTMODE_DLR_GPS_INTEGER,
               "\x30""%d",
               prLongInfo.u8Seconds );
         }
         else
         {
            UTF8_s32SaveNPrintFormat( utf8FormattedSecData,
               TESTMODE_DLR_GPS_INTEGER,
               "%d",
               prLongInfo.u8Seconds );
         }
         UTF8_s32SaveNPrintFormat( utf8FormattedTenthSecData,
            TESTMODE_DLR_GPS_INTEGER,
            "%d",
            prLongInfo.u8TenthSeconds );


         if( prLongInfo.bDirection == TRUE )
         {
            /** Convert Raw Gps Longitude Info into Buffer for formatting */
            UTF8_s32SaveNPrintFormat( utf8FormattedData,
               TESTMODE_DLR_GPS_STRLEN,
               "E%s""\xB0""%s""\x2E""%s""\x22", //Fix to mms 312838
               utf8FormattedDegData,
               utf8FormattedMinData,
               utf8FormattedSecData	);
         }
         else
         {
            /** Convert Raw Gps Longitude Info into Buffer for formatting */
            UTF8_s32SaveNPrintFormat( utf8FormattedData,
               TESTMODE_DLR_GPS_STRLEN,
               "W%s""\xB0""%s""\x2E""%s""\x22", //Fix to mms 312838
               utf8FormattedDegData,
               utf8FormattedMinData,
               utf8FormattedSecData	);
         }

         /** Convert the buffer data into required format 
         as the buffer data contains the UTF8 Coded Character
         and get the number of UTF8 Bytes the String Requires */

         utfutil_u32ConvertISOMod2UTF8( utf8IsoToUtf8Data,
            TESTMODE_DLR_GPS_STRLEN,
            utf8FormattedData,
            TESTMODE_DLR_GPS_STRLEN );

         /** Get the UTF8 formatted String into the Buffer to Display */
         UTF8_s32SaveNPrintFormat( tCharpacText,
            TESTMODE_DLR_GPS_STRLEN,
            "%s", 
            utf8IsoToUtf8Data  );


         /** Return Altitude*/	
         GUI_String_vSetCStr(out_result, (tU8*)tCharpacText);


         break;
      }
   case TESTMODE_MAPMATCH_HEADING:
      {
         tU8 u8Heading = 0;
         if (tCharpacText != NULL)
            OSAL_pvMemorySet((tVoid*)tCharpacText,NULL,TESTMODE_DLR_GPS_STRLEN);
         if(NULL != m_poDataPool)
         {
            m_poDataPool->u32dp_get( DPNAVI__CURRENT_VEHICLE_VEHICLE_HEADING, 
               &u8Heading, 
               sizeof(u8Heading)	);
         }

         /** Converting u8Heading value into Degrees format.
         Instead of this conversion we can use the fHeadingInternalToDegree of enavi_EnaviServices.h 
         Conversion done at this place is same as fHeadingInternalToDegree()*/

         tFloat fHeadingDegrees = (MAXIMUM_VALUE_U8HEADING_FORCONVERSION-u8Heading)*
            (CONVERSION_U8HEADING_TO_U16DEGREES);

         /** Add the factor 0.5 before converting to tU16  */
         tU16 u16HeadingDegrees = (tU16)(fHeadingDegrees+FLOATHEADING_CORRECTION_FACTOR);

         /** To make 360 Degrees Equivalent to 0 Degrees */
         if( MAXIMUM_DEGREES_POSSIBLE == u16HeadingDegrees )
         {
            u16HeadingDegrees = 0;
         }

         /** Format the display */
         tUTF8 utf8IsoToUtf8Data[TESTMODE_DLR_GPS_STRLEN] = "\0";
         tUTF8 utf8FormattedData[TESTMODE_DLR_GPS_STRLEN] = "\0";

         UTF8_s32SaveNPrintFormat( utf8FormattedData, 
            TESTMODE_DLR_GPS_STRLEN, 
            "%d""\xB0", 
            u16HeadingDegrees	);


         /** Convert the buffer data into required format 
         as the buffer data contains the UTF8 Coded Character
         and get the number of UTF8 Bytes the String Requires */
         utfutil_u32ConvertISOMod2UTF8( utf8IsoToUtf8Data,
            TESTMODE_DLR_GPS_STRLEN,
            utf8FormattedData,
            TESTMODE_DLR_GPS_STRLEN );


         /** Get the UTF8 formatted String into the Buffer to Display */
         UTF8_s32SaveNPrintFormat( tCharpacText,
            TESTMODE_DLR_GPS_STRLEN,
            "%s", 
            utf8IsoToUtf8Data  );

         /** Send the Converted UTF8 String to GUI */
         GUI_String_vSetCStr(out_result,(tU8*)tCharpacText);

         break;
      }
   case TESTMODE_MAPMATCH_ALTITUDE:
      {
         tS32 s32Height = 0;
         if (tCharpacText != NULL)
            OSAL_pvMemorySet((tVoid*)tCharpacText,NULL,TESTMODE_DLR_GPS_STRLEN);
#ifdef DPNAVI__CURRENT_VEHICLE_POS_ALTITUDE
         if(NULL != m_poDataPool)
         {
            m_poDataPool->u32dp_get( DPNAVI__CURRENT_VEHICLE_POS_ALTITUDE, 
               &s32Height, 
               sizeof(s32Height)	);
         }
#endif

         UTF8_s32SaveNPrintFormat( tCharpacText, 
            TESTMODE_DLR_GPS_STRLEN, 
            "%d m", 
            s32Height	);

         GUI_String_vSetCStr(out_result,(tU8*)tCharpacText);

         break;
      }
   case TESTMODE_MAPMATCH_GRADIENT:
      {
         tS16 s16Gradient = 0;
         if (tCharpacText != NULL)
            OSAL_pvMemorySet((tVoid*)tCharpacText,NULL,TESTMODE_DLR_GPS_STRLEN);
#ifdef DPNAVI__CURRENT_VEHICLE_POS_INCLINATION
         if(NULL != m_poDataPool)
         {
            m_poDataPool->u32dp_get( DPNAVI__CURRENT_VEHICLE_POS_INCLINATION, 
               &s16Gradient, 
               sizeof(s16Gradient)	);
         }
#endif

         UTF8_s32SaveNPrintFormat( tCharpacText, 
            TESTMODE_DLR_GPS_STRLEN, 
            "%d", 
            s16Gradient	);

         GUI_String_vSetCStr(out_result,(tU8*)tCharpacText);

         break;
      }
   default:
      {
         //Nothing to do Here
         break;
      }
   }/** End of Switch */

   /** Trace is here */	
   if(m_poTrace)
   {	
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
         TR_CLASS_HMI_HSA_SYSTEM_TESTMODE,
         (tU16)(HSA_API_RAYALU__GET_MATCHED_POSITION_VALUES ),
         MAX_GPS_FORMAT_STRLEN,
         (tU8*)tCharpacText );
   }

   OSAL_DELETE[] tCharpacText;	
}



sMapMatch_Info clHSA_TestMode::sMatchedPosValueConversion(const tS32 GpsValue) const
{ 
   sMapMatch_Info GpsConv; 
   //temporary variables used in conversion 
   tU32   u32AbsValue = 0; 
   tU16  u16DigitValue = 0; 
   const tU16 SHIFT = 7; 

   //to find the Direction 
   if(GpsValue >= static_cast<tS32> (0) ) 
   { 
      GpsConv.bDirection = TRUE; 
      // lowest SHIFT bits are not used because of limited resolution 
      u32AbsValue = GpsValue >> SHIFT;         
   } 
   else 
   { 
      GpsConv.bDirection = FALSE; 
      // lowest SHIFT bits are not used because of limited resolution 
      u32AbsValue = (GpsValue * -1) >> SHIFT;   
   } 

   // calculate the degrees and subtract the result 
   u32AbsValue *= static_cast<tU32> (180) ; 
   u16DigitValue = static_cast<tU16> ( ( u32AbsValue >> (31-SHIFT) ) ); 
   GpsConv.u8Degree = static_cast<tU8> (u16DigitValue); 
   u32AbsValue -= (static_cast<tU32> (u16DigitValue) ) << (31-SHIFT); 

   // calculate the minutes and subtract the result 
   u32AbsValue *= static_cast<tU32> (60) ; 
   u16DigitValue = static_cast<tU16> (( u32AbsValue >> (31-SHIFT) )) ; 
   GpsConv.u8Minutes = static_cast<tU8> (u16DigitValue) ; 
   u32AbsValue -= ((static_cast<tU32> (u16DigitValue) ) << (31-SHIFT)); 

   // calculate the seconds and subtract the result 
   u32AbsValue *= static_cast<tU32> (60) ; 
   u16DigitValue = static_cast<tU16> (( u32AbsValue >> (31-SHIFT) )) ; 
   GpsConv.u8Seconds = static_cast<tU8> (u16DigitValue ); 
   u32AbsValue -= ((static_cast<tU32> (u16DigitValue) ) << (31-SHIFT)); 

   // calculate the second fraction and subtract the result 
   u32AbsValue *= static_cast<tU32> (DED_SECOND_FRACTION) ; 
   u16DigitValue = static_cast<tU16>(( static_cast<tU32>(u32AbsValue )>> (31-SHIFT) )) ; 
   //GpsConv.u8Seconds = static_cast<tU8>(u16DigitValue); 
   u32AbsValue -= ((static_cast<tU32>(u16DigitValue) ) << (31-SHIFT)); 



   if ( u32AbsValue > ((1 << (31-SHIFT-1))) ) 
   { 
      (GpsConv.u8Seconds)++; 

      if ( GpsConv.u8Seconds >= (tU8)(60 * DED_SECOND_FRACTION)) 
      { 
         GpsConv.u8Seconds -= (tU8)(60 * DED_SECOND_FRACTION); 
         (GpsConv.u8Minutes)++; 
         if ( GpsConv.u8Minutes >= 60 ) 
         { 
            GpsConv.u8Minutes -= 60; 
            (GpsConv.u8Degree)++; 
         } 
      } 
   } 
   return GpsConv; 
} 


void clHSA_TestMode::vSendCommandtoHSI(tU32 u32CommandID, tU32 u32DataID) const
{
   /** Variable to hold the value which 
   make a decision whther command is 
   required to send or not */
   tBool bCommandDecideFactor = FALSE;

   if(m_poDataPool)
   {
      m_poDataPool->u32dp_get( u32DataID, 
         &bCommandDecideFactor, 
         sizeof(bCommandDecideFactor)	);
   }

   if(!bCommandDecideFactor)
   {
      bCommandDecideFactor = TRUE;

      if(m_poDataPool)
      {
         m_poDataPool->u32dp_set( u32DataID, 
            &bCommandDecideFactor, 
            sizeof(bCommandDecideFactor)	);
      }
      if( NULL != pclHSI_TestMode)
      {
         sMsg refMsg (NULL, u32CommandID, NULL, NULL) ;
         pclHSI_TestMode->bExecuteMessage(refMsg);
      }
   }
   else
   {
      /** No need to send the event to HSI */
   }
}

void clHSA_TestMode::vSendCommandtoHSI(tU32 u32CommandID)
{

   if( NULL != pclHSI_TestMode )
   {
      sMsg refMsg (NULL, u32CommandID, NULL, NULL) ;
      pclHSI_TestMode->bExecuteMessage(refMsg);
   }
   else
   {
      /** No need to send the event to HSI */
   }
}

/********************************************************************************
************** Functions Implemented On Monday **********************************
********************************************************************************/

/**
* Method: blIsDevelopermodeEnabled
* Returns whether Developermode was activated or not.
* B1
*/
tbool clHSA_TestMode::blIsDevelopermodeEnabled()
{
   tBool bResult = FALSE;
   if(m_poDataPool)
   {
      m_poDataPool->u32dp_get(DPDVPTSM__ENABLED, &bResult, sizeof(tBool) );
      if (FALSE == bResult){
         // also check tempenable... Fix for MMS193968
         m_poDataPool->u32dp_get(DPDVPTSM__TEMPENABLED, &bResult, sizeof(tBool) );
      }
   }
   if(m_poTrace)
   {
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
         TR_CLASS_HMI_HSA_SYSTEM_TESTMODE,
         (tU16)(HSA_API_RAYALU__IS_DEVELOPERMODE_ENABLED),
         1,
         &bResult );
   }
   if (bResult){ // hd83hi: only notify the CMRadio when it's enabled

      if( NULL != pclHSI_Radio )
      {
         sMsg refMsg(NULL, CMMSG_SERVICE_MODE_TESTMODE_ON, NULL, NULL);
         pclHSI_Radio->bExecuteMessage( refMsg );
      }
   }	
   return bResult;
}
void clHSA_TestMode::vServiceModeEntry()
{
   /* No HMi for Meter version, this is internal shortcut to get version*/
	if( NULL != pclHSI_CMMeter )
	{
		sMsg refMsg(NULL, CMMSG_METER_GET_VERSION_INFO, NULL, NULL);
		pclHSI_CMMeter->bExecuteMessage( refMsg );
	}
	if( NULL != pclHSI_CMSmartPhone )
	{
		sMsg refMsg(NULL, (tU32)CMMSG_SPI_GET_VERSION_INFO, NULL, NULL);
		pclHSI_CMSmartPhone->bExecuteMessage( refMsg );
	}              
	if( NULL != pclHSI_CMTCU )
	{
		sMsg refMsg(NULL, (tU32)CMMSG_TCU_GET_VERSION_INFO, NULL, NULL);
		pclHSI_CMTCU->bExecuteMessage( refMsg );
	}
}

void clHSA_TestMode::vServiceModeExit()
{
   /* Command is sent to Radio comp manager to indicate that service testmode is ON. */
   if( NULL != pclHSI_Radio )
   {
      sMsg refMsg(NULL, CMMSG_TESTMODE_SERV_ON_TEST_OFF, NULL, NULL);
      pclHSI_Radio->bExecuteMessage( refMsg );
   }
}



/**
* Method: ulwGetSetupSharx
* Returns current sharx value for setup
* B1
*/
ulword clHSA_TestMode::ulwGetSetupSharx()
{
   tU8 u8SetUpSharx = 0;
   if( m_poDataPool )
   {
      m_poDataPool->u32dp_get( DPTUNER__FEATURE_SHARX_LEVEL, 
         &u8SetUpSharx, 
         sizeof(u8SetUpSharx)	);
   }
   if(m_poTrace)
   {
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
         TR_CLASS_HMI_HSA_SYSTEM_TESTMODE,
         (tU16)(HSA_API_RAYALU__GET_SETUP_SHARX),
         1,
         &u8SetUpSharx );
   }

   return u8SetUpSharx;
}


/**
* Method: vSetSetupSharx
* Sets the sharx value 
* B1
*/
void clHSA_TestMode::vSetSetupSharx(tbool Direction)
{
   tU8 u8SetUpSharx = 0;
   if( m_poDataPool )
   {
      m_poDataPool->u32dp_get( DPTUNER__FEATURE_SHARX_LEVEL, 
         &u8SetUpSharx, 
         sizeof(u8SetUpSharx)	);
   }

   if( TRUE == Direction)
   {
      u8SetUpSharx = u8SetUpSharx + (tU8)Add_1;
   }
   else
   {
      u8SetUpSharx = u8SetUpSharx - (tU8)Add_1;
   }

   if(u8SetUpSharx <= MAX_SHARX_VALUE)
   {

      if( m_poDataPool )
      {
         m_poDataPool->u32dp_set( DPTUNER__FEATURE_SHARX_LEVEL,
            &u8SetUpSharx,
            sizeof(u8SetUpSharx)	);
      }

      vSendCommandtoHSI(HSI__TUN__CHGFEATURE_SHARX_LEVEL);

   }

   else
   {
      //do nothing
   }

   if(m_poTrace)
   {
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
         TR_CLASS_HMI_HSA_SYSTEM_TESTMODE,
         (tU16)(HSA_API_RAYALU__SET_SETUP_SHARX ),
         1,
         &u8SetUpSharx );
   }
}

/**
* Method: ulwGetSetupHighcut
* Returns currently selected highcut in radio setup 
* B1
*/
ulword clHSA_TestMode::ulwGetSetupHighcut()
{
   tU8 u8SetUpHiCut = 0;
   if( m_poDataPool )
   {
      m_poDataPool->u32dp_get( DPTUNER__FEATURE_HIGHCUT, 
         &u8SetUpHiCut, 
         sizeof(u8SetUpHiCut)	);
   }


   if(m_poTrace)
   {
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
         TR_CLASS_HMI_HSA_SYSTEM_TESTMODE,
         (tU16)(HSA_API_RAYALU__GET_SETUP_HIGHCUT ),
         1,
         &u8SetUpHiCut );
   }

   return u8SetUpHiCut;
}


/**
* Method: vSetHighcut
* Sets current HC
* B1
*/
void clHSA_TestMode::vSetHighcut(tbool Direction)
{
   tU8 u8SetUpHiCut = 0;

   if( m_poDataPool )
   {
      m_poDataPool->u32dp_get( DPTUNER__FEATURE_HIGHCUT, 
         &u8SetUpHiCut, 
         sizeof(u8SetUpHiCut)	);
   }
   if (Direction)
   {
      u8SetUpHiCut = u8SetUpHiCut + (tU8)Add_1;
   }
   else
   {
      u8SetUpHiCut = u8SetUpHiCut - (tU8)Add_1;
   }

   if(u8SetUpHiCut <= MAX_HIGHCUT_VALUE)
   {

      if( m_poDataPool )
      {
         m_poDataPool->u32dp_set( DPTUNER__FEATURE_HIGHCUT,
            &u8SetUpHiCut,
            sizeof(u8SetUpHiCut)	);
      }


      vSendCommandtoHSI(HSI__TUN__CHGFEATURE_HIGHCUT);

   }

   else
   {
      // do nothing
   }

   if(m_poTrace)
   {
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
         TR_CLASS_HMI_HSA_SYSTEM_TESTMODE,
         (tU16)(HSA_API_RAYALU__SET_HIGHCUT ),
         1,
         &u8SetUpHiCut );
   }


}


/**
* Method: vToggleAFValue
* Toggles the AF value between true and false.
* B1
*/
void clHSA_TestMode::vToggleAFValue()
{
   tBool bAFValue = FALSE;
   tU32 u32EventToSend = 0;
   if( m_poDataPool )
   {
      m_poDataPool->u32dp_get( DPTUNER__FEATURE_AF, 
         &bAFValue, 
         sizeof(bAFValue)	);
   }
   if( TRUE == bAFValue )
   {
      u32EventToSend = HSI__TUN__FEATURE_AF_OFF;
   }
   else
   {
      u32EventToSend = HSI__TUN__FEATURE_AF_ON;
   }

   vSendCommandtoHSI(u32EventToSend);		
   if(m_poTrace)
   {
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
         TR_CLASS_HMI_HSA_SYSTEM_TESTMODE,
         (tU16)(HSA_API_RAYALU__TOGGLE_AF_VALUE ),
         4,
         &u32EventToSend );
   }
}


/**
* Method: blGetAF
* Returns if AF is activated or not.
* B1
*/
tbool clHSA_TestMode::blGetAF()
{
   tBool bAFValue = FALSE;
   if( m_poDataPool )
   {
      m_poDataPool->u32dp_get( DPTUNER__FEATURE_AF, 
         &bAFValue, 
         sizeof(bAFValue)	);
   }
   if(m_poTrace)
   {
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
         TR_CLASS_HMI_HSA_SYSTEM_TESTMODE,
         (tU16)(HSA_API_RAYALU__GET_AF ),
         1,
         &bAFValue );
   }
   return bAFValue;
}

/**
* Method: ToggleDDAState
* Toggles the DDA between on and off.
* B1
*/
void clHSA_TestMode::vToggleDDAState()
{
   tBool bDDAValue = FALSE;
   tU32 u32EventToSend = 0;
   if( m_poDataPool )
   {
      m_poDataPool->u32dp_get( DPTUNER__FEATURE_DDA, 
         &bDDAValue, 
         sizeof(bDDAValue)	);
   }
   if( TRUE == bDDAValue )
   {
      u32EventToSend = HSI__TUN__FEATURE_DDA_OFF;
   }
   else
   {
      u32EventToSend = HSI__TUN__FEATURE_DDA_ON;
   }

   vSendCommandtoHSI(u32EventToSend);		
   if(m_poTrace)
   {
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
         TR_CLASS_HMI_HSA_SYSTEM_TESTMODE,
         (tU16)(HSA_API_TOGGLE_DDA_VALUE ),
         4,
         &u32EventToSend );
   }
}
/**
* Method: SetRadioTMEnter
* Toggles the radio setup to enabled or disabled.
* B1
*/
void clHSA_TestMode::vSetRadioTMEnter()
{
   tbool bSetRadioActive = TRUE;

   if(pclHSI_TestMode)
   {
      sMsg refMsg (NULL, (tU32)CMMSG_TESTMODE_RADIO_AMFMSETUP_ON_API, (tU32)bSetRadioActive, NULL);
      pclHSI_TestMode->bExecuteMessage(refMsg);
   }
}

/**
* Method: GetDDAStatus
* Returns if AF is activated or not.
* B1
*/
tbool clHSA_TestMode::blGetDDAStatus()
{
   tBool bDDAValue = FALSE;
   if( m_poDataPool )
   {
      m_poDataPool->u32dp_get( DPTUNER__FEATURE_DDA, 
         &bDDAValue, 
         sizeof(bDDAValue)	);
   }
   if(m_poTrace)
   {
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
         TR_CLASS_HMI_HSA_SYSTEM_TESTMODE,
         (tU16)(HSA_API_GET_DDA_VALUE ),
         1,
         &bDDAValue );
   }
   return bDDAValue;
}

/**
* Method: vToggleFreezeBackgroundTuner
* Toggles the Freeze BackgroundTuner value between true and false.
* B1
*/

void clHSA_TestMode::vToggleFreezeBackgroundTuner()
{
   tBool bFreezeBackGndTuner = FALSE;
   if( m_poDataPool )
   {
      m_poDataPool->u32dp_get( DPTUNER__FEATURE_BGTUNER, 
         &bFreezeBackGndTuner, 
         sizeof(bFreezeBackGndTuner)	);
   }
   if( TRUE == bFreezeBackGndTuner)
   {
      /** Send HSI Command to enable the background tuner */
      vSendCommandtoHSI(HSI__TUN__DEACTIVATE_BG_TUNER);
   }
   else
   {
      /** Send HSI Command to freeze the background tuner */
      vSendCommandtoHSI(HSI__TUN__ACTIVATE_BG_TUNER);

   }
   if(m_poTrace)
   {
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
         TR_CLASS_HMI_HSA_SYSTEM_TESTMODE,
         (tU16)(HSA_API_RAYALU__TOGGLE_FREEZE_BACKGROUND_TUNER ) );
   }
}


/**
* Method: blGetFreezeBackgroundTuner
* Returns if Freeze Backgroung Tuner is activated
* B1
*/
tbool clHSA_TestMode::blGetFreezeBackgroundTuner()
{
   tBool bFreezeBackGndTuner = FALSE;
   if( m_poDataPool )
   {
      m_poDataPool->u32dp_get( DPTUNER__FEATURE_BGTUNER, 
         &bFreezeBackGndTuner, 
         sizeof(bFreezeBackGndTuner)	);
   }
   if(m_poTrace)
   {
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
         TR_CLASS_HMI_HSA_SYSTEM_TESTMODE,
         (tU16)(HSA_API_RAYALU__GET_FREEZE_BACKGROUND_TUNER ),
         1,
         &bFreezeBackGndTuner );
   }

   return bFreezeBackGndTuner;
}


/**
* Method: blGetLinearAudio
* Returns wether or not Linear Audio is activated or not. Requested by BP
* B1
*/
tbool clHSA_TestMode::blGetLinearAudio()
{
   tBool bLinearAudio = FALSE;
   if( m_poDataPool )
   {
      m_poDataPool->u32dp_get( DPDVPTSM__SOUND_LINEAR, 
         &bLinearAudio, 
         sizeof(bLinearAudio)	);
   }
   if(m_poTrace)
   {
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
         TR_CLASS_HMI_HSA_SYSTEM_TESTMODE,
         (tU16)(HSA_API_RAYALU__GET_LINEAR_AUDIO ),
         1,
         &bLinearAudio );
   }

   return bLinearAudio;
}


/**
* Method: vToggelLinearAudio
* Toggles the Linea Audio value between true and false
* B1
*/
void clHSA_TestMode::vToggelLinearAudio()
{
   tBool bLinearAudio = FALSE;
   tU32 u32EventToSend = 0;
   if( m_poDataPool )
   {
      m_poDataPool->u32dp_get( DPDVPTSM__SOUND_LINEAR, 
         &bLinearAudio, 
         sizeof(bLinearAudio)	);
   }
   if( TRUE == bLinearAudio )
   {
      u32EventToSend = HSI__AUD__DIAG__LINEAR_OFF;
   }
   else
   {
      u32EventToSend = HSI__AUD__DIAG__LINEAR_ON;
   }

   vSendCommandtoHSI(u32EventToSend);		
   if(m_poTrace)
   {
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
         TR_CLASS_HMI_HSA_SYSTEM_TESTMODE,
         (tU16)(HSA_API_RAYALU__TOGGEL_LINEAR_AUDIO),
         4,
         &u32EventToSend );
   }
}


/**
*
* Method: vClearResetCounterValues
* Deletes all values about the reset counter
* B1
*/
void clHSA_TestMode::vClearResetCounterValues()
{
   tBool bclearflagdata = FALSE;

   /** Send HSI Command to reset the reset counter values */
   vSendCommandtoHSI(HSI__SPM__RESETLIST_RESET);

   if(m_poTrace)
   {
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
         TR_CLASS_HMI_HSA_SYSTEM_TESTMODE,
         (tU16)(HSA_API_RAYALU__CLEAR_RESET_COUNTER_VALUES ) );
   }

   if( m_poDataPool )
   {
      m_poDataPool->u32dp_set( DPDVPTSM__EVENTFLAG_RESETLIST, 
         &bclearflagdata, 
         sizeof(bclearflagdata)	);
   }
}


/**
* Method: vGetResetCounterValue
* Returns the value for the given reset counter
* B1
*/
void clHSA_TestMode::vGetResetCounterValue(GUI_String *out_result,ulword Counter)
{


   /** This is work around to send the event to HSI.
   When Framework provides this trigger points
   we have to remove this function call as well as definition
   and we can remove the datapool ID's used in this function 
   from DVPTSM.xml */
   vSendCommandtoHSI( HSI__SPM__RESETLIST, 
      DPDVPTSM__EVENTFLAG_RESETLIST );

   /** Allocate memory dynamically for the 
   Buffer to hold the formatted Strings */
   tChar* tCharpacText = NULL;
   tCharpacText = OSAL_NEW tChar[TESTMODE_DLR_GPS_STRLEN];	
   if(tCharpacText != NULL)
      OSAL_pvMemorySet((tVoid*)tCharpacText,NULL,TESTMODE_DLR_GPS_STRLEN);

   tU8 u8ResetCounterValue = 0;

   switch(Counter)
   {
   case RESET_WATCHDOG:
      {			
         if( m_poDataPool )
         {
            m_poDataPool->u32dp_get( DPDIAGNOSIS__RSTCTR_HMIRST_WATCHDOG, 
               &u8ResetCounterValue, 
               sizeof(u8ResetCounterValue)	);
         }
         break;
      }

   case RESET_APPASSERTION_FAILED:
      {			
         if( m_poDataPool )
         {
            m_poDataPool->u32dp_get( DPDIAGNOSIS__RSTCTR_HMIRST_APP, 
               &u8ResetCounterValue, 
               sizeof(u8ResetCounterValue)	);
         }
         break;
      }

   case RESET_PROCESSOR_EXCEPTION:
      {			
         if( m_poDataPool )
         {
            m_poDataPool->u32dp_get( DPDIAGNOSIS__RSTCTR_HMIRST_SWEXCEP, 
               &u8ResetCounterValue, 
               sizeof(u8ResetCounterValue)	);
         }
         break;
      }

   case RESET_SPMAPP_CTRLERROR:
      {			
         if( m_poDataPool )
         {
            m_poDataPool->u32dp_get( DPDIAGNOSIS__RSTCTR_HMIRST_WATCHDOG, 
               &u8ResetCounterValue, 
               sizeof(u8ResetCounterValue)	);
         }
         break;
      }

   case RESET_FGS:
      {			
         if( m_poDataPool )
         {
            m_poDataPool->u32dp_get( DPDIAGNOSIS__RSTCTR_FGSRST_HMI, 
               &u8ResetCounterValue, 
               sizeof(u8ResetCounterValue)	);
         }
         break;
      }

   case RESET_DOWNLOAD:
      {			
         if( m_poDataPool )
         {
            m_poDataPool->u32dp_get( DPDIAGNOSIS__RSTCTR_HMIRST_DOWNLOAD, 
               &u8ResetCounterValue, 
               sizeof(u8ResetCounterValue)	);
         }
         break;
      }

   case RESET_DIAG:
      {			
         if( m_poDataPool )
         {
            m_poDataPool->u32dp_get( DPDIAGNOSIS__RSTCTR_HMIRST_DIAG, 
               &u8ResetCounterValue, 
               sizeof(u8ResetCounterValue)	);
         }
         break;
      }

   case RESET_OTHER:
      {			
         if( m_poDataPool )
         {
            m_poDataPool->u32dp_get( DPDIAGNOSIS__RSTCTR_HMIRST_HMI, 
               &u8ResetCounterValue, 
               sizeof(u8ResetCounterValue)	);
         }
         break;
      }

   default:
      {
         break;
      }
   }


   UTF8_s32SaveNPrintFormat( tCharpacText,
      TESTMODE_DLR_GPS_STRLEN,
      "%d\0",
      u8ResetCounterValue );

   GUI_String_vSetCStr(out_result, (tU8*)tCharpacText);


   if(m_poTrace)
   {	
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
         TR_CLASS_HMI_HSA_SYSTEM_TESTMODE,
         (tU16)(HSA_API_RAYALU__GET_RESET_COUNTER_VALUE ),
         TESTMODE_DLR_GPS_STRLEN,
         (tU8*)tCharpacText );
   }

   /** Release the memory Allocated dynamically */
   OSAL_DELETE[] tCharpacText;
}



/** Has to be removed before the release */

/**
* Method: vGetTMCStationPS
* returns the PS Name of the current TMC station
* B2
*/
void clHSA_TestMode::vGetTMCStationPS(GUI_String *out_result)
{
   /** Allocate memory dynamically for the 
   Buffer to hold the formatted Strings */
   tChar* tCharpacText = NULL;
   tCharpacText = OSAL_NEW tChar[TESTMODE_DLR_GPS_STRLEN];	
   if(tCharpacText != NULL)
      OSAL_pvMemorySet((tVoid*)tCharpacText,NULL,TESTMODE_DLR_GPS_STRLEN);

   tU32 u32Size = 0;
   u32Size   = m_poDataPool->u32dp_getElementSize(DPTUNER__TMC_PS);
   tChar* pacText = OSAL_NEW tChar[u32Size+1];
   if(pacText != NULL)
      OSAL_pvMemorySet((tVoid*)pacText,NULL,u32Size+1);
   if( NULL != m_poDataPool)
   {
      m_poDataPool->u32dp_get( DPTUNER__TMC_PS, 
         pacText, 
         u32Size	);
   }
   UTF8_s32SaveNPrintFormat( tCharpacText, TESTMODE_DLR_GPS_STRLEN, "%s", pacText);
   OSAL_DELETE[] pacText;
   GUI_String_vSetCStr(out_result,(tU8*)tCharpacText);

   if(m_poTrace)
   {	
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
         TR_CLASS_HMI_HSA_SYSTEM_TESTMODE,
         (tU16)(HSA_API_RAYALU__GET_TMC_STATION_PS ),
         TESTMODE_DLR_GPS_STRLEN,
         (tU8*)tCharpacText );
   }

   /** Release the memory Allocated dynamically */
   OSAL_DELETE[] tCharpacText;
}


/**
* Method: vGetTMCStationPI
* returns the Program Identifier Name of the active TMC station
* B2
*/
void clHSA_TestMode::vGetTMCStationPI(GUI_String *out_result)
{
   /** Allocate memory dynamically for the 
   Buffer to hold the formatted Strings */
   tChar* tCharpacText = NULL;
   tCharpacText = OSAL_NEW tChar[TESTMODE_DLR_GPS_STRLEN];	
   if(tCharpacText != NULL)
      OSAL_pvMemorySet((tVoid*)tCharpacText,NULL,TESTMODE_DLR_GPS_STRLEN);

   tU32 u32TMCPIValue = 0;
   if( NULL != m_poDataPool)
   {
      m_poDataPool->u32dp_get( DPTUNER__TMC_PI, 
         &u32TMCPIValue, 
         sizeof(u32TMCPIValue)	);
   }
   UTF8_s32SaveNPrintFormat( tCharpacText, TESTMODE_DLR_GPS_STRLEN, "%X", u32TMCPIValue);
   GUI_String_vSetCStr(out_result,(tU8*)tCharpacText);

   if(m_poTrace)
   {	
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
         TR_CLASS_HMI_HSA_SYSTEM_TESTMODE,
         (tU16)(HSA_API_RAYALU__GET_TMC_STATION_PI ),
         TESTMODE_DLR_GPS_STRLEN,
         (tU8*)tCharpacText );
   }

   /** Release the memory Allocated dynamically */
   OSAL_DELETE[] tCharpacText;

}


/**
* Method: vGetTMCStationCountryCode
* returns the Country Code of the active TMC station
* B2
*/
void clHSA_TestMode::vGetTMCStationCountryCode(GUI_String *out_result)
{
   /** Allocate memory dynamically for the 
   Buffer to hold the formatted Strings */
   tChar* tCharpacText = NULL;
   tCharpacText = OSAL_NEW tChar[TESTMODE_DLR_GPS_STRLEN];	
   if(tCharpacText != NULL)
      OSAL_pvMemorySet((tVoid*)tCharpacText,NULL,TESTMODE_DLR_GPS_STRLEN);

   tU8 u8TMCCountryCode = 0;
   if( NULL != m_poDataPool)
   {
      m_poDataPool->u32dp_get( DPTUNER__TMC_COUNTRY_CODE, 
         &u8TMCCountryCode, 
         sizeof(u8TMCCountryCode)	);
   }
   UTF8_s32SaveNPrintFormat( tCharpacText, TESTMODE_DLR_GPS_STRLEN, "%X", u8TMCCountryCode);
   GUI_String_vSetCStr(out_result,(tU8*)tCharpacText);


   if(m_poTrace)
   {	
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
         TR_CLASS_HMI_HSA_SYSTEM_TESTMODE,
         (tU16)(HSA_API_RAYALU__GET_TMC_STATION_COUNTRY_CODE ),
         10,
         (tU8*)tCharpacText );
   }

   /** Release the memory Allocated dynamically */
   OSAL_DELETE[] tCharpacText;
}


/**
* Method: ulwGetTMCStationSID
* returns the Service ID for the active TMC station
* B2
*/
ulword clHSA_TestMode::ulwGetTMCStationSID()
{
   tU8 u8SID = 0;
   if( NULL != m_poDataPool)
   {
      m_poDataPool->u32dp_get( DPTUNER__TMC_SER_ID, 
         &u8SID, 
         sizeof(u8SID)	);
   }

   if(m_poTrace)
   {	
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
         TR_CLASS_HMI_HSA_SYSTEM_TESTMODE,
         (tU16)(HSA_API_RAYALU__GET_TMC_STATION_SID ),
         1,
         &u8SID );
   }
   return (ulword)u8SID;
}


/**
* Method: ulwGetTMCStationLTN
* returns the Localtion Table Number for the active TMC station
* B2
*/
ulword clHSA_TestMode::ulwGetTMCStationLTN()
{
   tU8 u8TMCLTNumber = 0;
   if( NULL != m_poDataPool)
   {
      m_poDataPool->u32dp_get( DPTUNER__TMC_LOC_TAB_NUM, 
         &u8TMCLTNumber, 
         sizeof(u8TMCLTNumber)	);
   }	

   if(m_poTrace)
   {	
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
         TR_CLASS_HMI_HSA_SYSTEM_TESTMODE,
         (tU16)(HSA_API_RAYALU__GET_TMC_STATION_LTN ),
         1,
         &u8TMCLTNumber );
   }
   return (ulword)u8TMCLTNumber;
}


/**
* Method: vGetTMCStationQuality
* returns the Quality of the active TMC station
* B2
*/
void clHSA_TestMode::vGetTMCStationQuality(GUI_String *out_result)
{
   /** Allocate memory dynamically for the 
   Buffer to hold the formatted Strings */
   tChar* tCharpacText = NULL;
   tCharpacText = OSAL_NEW tChar[TESTMODE_DLR_GPS_STRLEN];	
   if(tCharpacText != NULL)
      OSAL_pvMemorySet((tVoid*)tCharpacText,NULL,TESTMODE_DLR_GPS_STRLEN);

   tU8 u8TMCQuality = 0;
   if( NULL != m_poDataPool)
   {
      m_poDataPool->u32dp_get( DPTUNER__TMC_RECP_QUAL, 
         &u8TMCQuality, 
         sizeof(u8TMCQuality)	);
   }
   if( u8TMCQuality <= MAX_TMCSTATION_POOR_QUALITY )
   {
      UTF8_s32SaveNPrintFormat( tCharpacText, TESTMODE_DLR_GPS_STRLEN, "%s", "poor");
   }
   else if((u8TMCQuality > MAX_TMCSTATION_POOR_QUALITY) && (u8TMCQuality <= MAX_TMCSTATION_WEAK_QUALITY))
   {
      UTF8_s32SaveNPrintFormat( tCharpacText, TESTMODE_DLR_GPS_STRLEN, "%s", "weak");
   }
   else
   {
      UTF8_s32SaveNPrintFormat( tCharpacText, TESTMODE_DLR_GPS_STRLEN, "%s", "stereo");
   }
   GUI_String_vSetCStr(out_result,(tU8*)tCharpacText);


   if(m_poTrace)
   {	
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
         TR_CLASS_HMI_HSA_SYSTEM_TESTMODE,
         (tU16)(HSA_API_RAYALU__GET_TMC_STATION_QUALITY ),
         10,
         (tU8*)tCharpacText );
   }

   /** Release the memory Allocated dynamically */
   OSAL_DELETE[] tCharpacText;
}

/**
* Method: vGetTMCStationQuality
* returns the Quality of the active TMC station
* B2
*/
void clHSA_TestMode::vGetTMCStation(GUI_String *out_result,ulword ListEntryNr)
{


   /** Allocate Memory dynamically */
   tChar* pacText = OSAL_NEW tChar[MAX_GPS_FORMAT_STRLEN];
   if(pacText != NULL)
      OSAL_pvMemorySet((tVoid*)pacText,NULL,MAX_GPS_FORMAT_STRLEN);

   /** Function call to Component Manager to read the Frequency and Quality values */
   if( NULL != pclHSI_TestMode )
   {
      pclHSI_TestMode->bGetTMCListElement((tU8)ListEntryNr, pacText );
   }

   /** Return TMC List Index String */		
   GUI_String_vSetCStr(out_result, (tU8*)pacText);

   /** Release the Memory allocated dynamically */
   OSAL_DELETE[] pacText;

}
/**
* Method: vGetTMCStationQuality
* returns the Quality of the active TMC station
* B2
*/
ulword clHSA_TestMode::ulwGetTMCStation_Count()
{
   tU8 u8TMCListCount = 0;

   /** This is work around to send the event to HSI
   When Framework provides this trigger points
   we have to remove this function call as well as definition
   and we can remove the datapool ID's used in this function 
   from DVPTSM.xml */
   vSendCommandtoHSI( HSI__TUN__LOAD_TMC_LIST, 
      DPDVPTSM__EVENTFLAG_TMC_FLAG );
   if(m_poDataPool)
   {
      m_poDataPool->u32dp_get( DPTUNER__TMC_ELEM_NUM, 
         &u8TMCListCount, 
         sizeof(u8TMCListCount)	);
   }

   /** Trace is here */	
   if(m_poTrace)
   {	
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
         TR_CLASS_HMI_HSA_SYSTEM_TESTMODE,
         (tU16)(HSA_API_RAYALU__GET_TUNER_AF_LIST_COUNT ),
         1,
         &u8TMCListCount );
   }
   return u8TMCListCount;
}


/**
* Method: vSelectTMCStation
* Selects a station from the TMC Stationlist for showing the details of this station
* B
*/
void clHSA_TestMode::vSelectTMCStation(ulword ListEntryNr)
{

   if( NULL != pclHSI_TestMode )
   {
      pclHSI_TestMode -> bGetTMCListElementDetails((tU8)ListEntryNr);
   }

}


/********************************************************************
***************** functions implemented in India ********************
********************************************************************/

/**
* Method: vGetDeadReckoningValues
* Returns values about the dead reckoning
* B1
*/
void clHSA_TestMode::vGetDeadReckoningValues(GUI_String *out_result,ulword ListEntryNr)
{
   /** Character Buffer to Hold the Formatted String */ 
   tChar* tCharpacText = NULL;
   tCharpacText = OSAL_NEW tChar[MAX_GPS_FORMAT_STRLEN];	
   if(tCharpacText != NULL)
      OSAL_pvMemorySet((tVoid*)tCharpacText,NULL,MAX_GPS_FORMAT_STRLEN);

   switch(ListEntryNr)
   {
   case DEADRECK_STATUS:
      {
         tU8 u8PosStatus = 0;
         if(tCharpacText != NULL)
            OSAL_pvMemorySet((tVoid*)tCharpacText,NULL,MAX_GPS_FORMAT_STRLEN);

         if(m_poDataPool)
         {
            m_poDataPool->u32dp_get( DPNAVI__CURRENT_VEHICLE_POS_STATUS, 
               &u8PosStatus, 
               sizeof(u8PosStatus) );
         }

         if( ( POSITION_STATUS_UNKNOWN == u8PosStatus ) || ( POSITION_STATUS_LASTPOSITION == u8PosStatus ) )
         {
            UTF8_s32SaveNPrintFormat( tCharpacText,MAX_GPS_FORMAT_STRLEN,"%s","No Pos");		  
         }
         else if( POSITION_STATUS_OFFMAP == u8PosStatus )
         {
            UTF8_s32SaveNPrintFormat( tCharpacText,MAX_GPS_FORMAT_STRLEN,"%s","Off-Map");		  
         }
         else if( ( POSITION_STATUS_OFFROAD == u8PosStatus ) || ( POSITION_STATUS_OFFCORRIDOR == u8PosStatus ) )
         {
            UTF8_s32SaveNPrintFormat( tCharpacText,MAX_GPS_FORMAT_STRLEN,"%s","Off-Road");		  
         }
         else if( ( POSITION_STATUS_FREEZE == u8PosStatus ) || ( POSITION_STATUS_ONROAD == u8PosStatus ) )
         {
            UTF8_s32SaveNPrintFormat( tCharpacText,MAX_GPS_FORMAT_STRLEN,"%s","On-Road");		  
         }		             
         else
         {
            if(u8PosStatus == 255) 
            {
               u8PosStatus = 0;
            }
            UTF8_s32SaveNPrintFormat( tCharpacText,MAX_GPS_FORMAT_STRLEN,"%d",u8PosStatus);		  
         }

         /** Return Antenna Connected Status*/	
         GUI_String_vSetCStr(out_result, (tU8*)tCharpacText);

         break;
      }

   case DEADRECK_SPEED:
      {
         tS16 s16Speed = 0;
         tU8 u8Direction = 0;
         if(tCharpacText != NULL)
            OSAL_pvMemorySet((tVoid*)tCharpacText,NULL,MAX_GPS_FORMAT_STRLEN);

         if(m_poDataPool)
         {
            m_poDataPool->u32dp_get( DPNAVI__CURRENT_VEHICLE_VEHICLE_SPEED, 
               &s16Speed, 
               sizeof(s16Speed) );
            m_poDataPool->u32dp_get( DPVDS__ODOMETER_DIRECTION,
               &u8Direction,
               sizeof(u8Direction) );
         }

         UTF8_s32SaveNPrintFormat( tCharpacText,
            MAX_GPS_FORMAT_STRLEN, 
            "%d km/h, ",
            s16Speed
            );

         if (u8Direction == ODOMETER_DIRECTION_FORWARD)
         {
            UTF8_szSaveStringConcat(tCharpacText,"Forward",MAX_GPS_FORMAT_STRLEN);
         }
         else if (u8Direction == ODOMETER_DIRECTION_REVERSE)
         {
            UTF8_szSaveStringConcat(tCharpacText,"Reverse",MAX_GPS_FORMAT_STRLEN);
         }
         else
         {
            UTF8_szSaveStringConcat(tCharpacText,"Unknown",MAX_GPS_FORMAT_STRLEN);
         }


         /** Return Antenna Connected Status*/	
         GUI_String_vSetCStr(out_result, (tU8*)tCharpacText);

         break;
      }

   case DEADRECK_CALIBRATION_ACCURACY:
      {
         tU8 u8CalibProgress = 0;
         if(tCharpacText != NULL)
            OSAL_pvMemorySet((tVoid*)tCharpacText,NULL,MAX_GPS_FORMAT_STRLEN);

         if(m_poDataPool)
         {
            m_poDataPool->u32dp_get( DPNAVI__CALIB_PROGRESS_VALUE, 
               &u8CalibProgress, 
               sizeof(u8CalibProgress) );
         }
         if(u8CalibProgress == 255)
         {
            u8CalibProgress = 0;
         }
         UTF8_s32SaveNPrintFormat( tCharpacText,
            MAX_GPS_FORMAT_STRLEN, 
            "%d%%",
            u8CalibProgress
            );

         /** Return Antenna Connected Status*/	
         GUI_String_vSetCStr(out_result, (tU8*)tCharpacText);

         break;
      }

   case DEADRECK_CALIBRATION_STATUS:
      {
         tU8 u8CalibStatus = 0;
         if(tCharpacText != NULL)
            OSAL_pvMemorySet((tVoid*)tCharpacText,NULL,MAX_GPS_FORMAT_STRLEN);

         if(m_poDataPool)
         {
            m_poDataPool->u32dp_get( DPDVPTSM__NAVI_CALIB_STATUS, 
               &u8CalibStatus, 
               sizeof(u8CalibStatus) );
         }

         UTF8_s32SaveNPrintFormat( tCharpacText,
            MAX_GPS_FORMAT_STRLEN, 
            "%d",
            u8CalibStatus
            );	

         /** Commeneted by rrv2kor as Logic of deciding the Calibration status is changed */

         //if( CALIB_STATUS_RESERVED == u8CalibStatus )
         //{
         //	UTF8_s32SaveNPrintFormat( tCharpacText,MAX_GPS_FORMAT_STRLEN,"%d",u8CalibStatus);		  
         //}
         //else if( CALIB_STATUS_CHECKING == u8CalibStatus )
         //{
         //	UTF8_s32SaveNPrintFormat( tCharpacText,MAX_GPS_FORMAT_STRLEN,"%d",u8CalibStatus);		  
         //}
         //else if( CALIB_STATUS_AVAILABLE == u8CalibStatus )
         //{
         //	UTF8_s32SaveNPrintFormat( tCharpacText,MAX_GPS_FORMAT_STRLEN,"%s","Calibrated");		  
         //}
         //else if( CALIB_STATUS_NEWINSTALL == u8CalibStatus )
         //{
         //	UTF8_s32SaveNPrintFormat( tCharpacText,MAX_GPS_FORMAT_STRLEN,"%s","Calibration in progress");		  
         //}
         //else if( CALIB_STATUS_WHEELCHANGE == u8CalibStatus )
         //{
         //	UTF8_s32SaveNPrintFormat( tCharpacText,MAX_GPS_FORMAT_STRLEN,"%s","Calibration in progress");		  
         //}
         //else if( CALIB_STATUS_NOTAVAILABLE == u8CalibStatus )
         //{
         //	UTF8_s32SaveNPrintFormat( tCharpacText,MAX_GPS_FORMAT_STRLEN,"%s","Not Calibrated");		  
         //}
         //else
         //{
         //	UTF8_s32SaveNPrintFormat( tCharpacText,MAX_GPS_FORMAT_STRLEN,"%d",u8CalibStatus);		  
         //}

         /** New Logic to decide the calibration status need to look */

         tU8 u8CalibProgress = 0;

         if(m_poDataPool)
         {
            m_poDataPool->u32dp_get( DPNAVI__CALIB_PROGRESS_VALUE, 
               &u8CalibProgress, 
               sizeof(u8CalibProgress) );
         }

         if( (CALIB_STATUS_RESERVED == u8CalibStatus)   || 
            (CALIB_STATUS_NEWINSTALL == u8CalibStatus) ||
            (CALIB_STATUS_WHEELCHANGE == u8CalibStatus)||
            (CALIB_STATUS_NOTAVAILABLE == u8CalibStatus) )
         {
            UTF8_s32SaveNPrintFormat( tCharpacText,MAX_GPS_FORMAT_STRLEN,"%s","Not Calibrated");		  
         }
         else if( CALIB_STATUS_CHECKING == u8CalibStatus )
         {
            if( MAX_CALIBPERCENTAGE_PROGRESS == u8CalibProgress )
            {
               UTF8_s32SaveNPrintFormat( tCharpacText,MAX_GPS_FORMAT_STRLEN,"%s","Calibrated");		  
            }
            else
            {
               UTF8_s32SaveNPrintFormat( tCharpacText,MAX_GPS_FORMAT_STRLEN,"%s","Calibration in progress");		  
            }
         }
         else if( CALIB_STATUS_AVAILABLE == u8CalibStatus )
         {
            UTF8_s32SaveNPrintFormat( tCharpacText,MAX_GPS_FORMAT_STRLEN,"%s","Calibrated");	
         }
         else
         {
            //nothing to do here
         }

         /** Return Antenna Connected Status*/	
         GUI_String_vSetCStr(out_result, (tU8*)tCharpacText);

         break;
      }
   default:
      {
         //Nothing to Do here.
         break;
      }
   } //End of switch


   if(m_poTrace)
   {	
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
         TR_CLASS_HMI_HSA_SYSTEM_TESTMODE,
         (tU16)(HSA_API_RAYALU__GET_DEAD_RECKONING_VALUES ),
         MAX_GPS_FORMAT_STRLEN,
         (tU8*)tCharpacText );
   }


   /** Release the memory Allocated dynamically */
   OSAL_DELETE[] tCharpacText;
}

/**
* Method: uwGetOdometerDataUpdate
* Returns odometer status and odometer pulse counter value
*/
ulword clHSA_TestMode::ulwGetOdometerDataUpdate(tBool OdoData)
{
   tU8 u8OdoStatus = 0;
   tU16 u16OdoCount = 0;
   tU16 u16Data = 0;

   switch(OdoData)
   {
   case ODO_STATUS:
      {
         if(m_poDataPool)
         {
            m_poDataPool->u32dp_get( DPVDS__ODOMETER_STATUS, &u8OdoStatus, sizeof(u8OdoStatus) );
         }
         if (u8OdoStatus == cmb_fi_tcl_e8_OdometerStatus::FI_EN_ODOMSTATE_CONNECTED_NORMAL)
         {
            //Do nothing
         }
         else
         {
            u16Data = u8OdoStatus - ODO_FI_TO_GUI_CONV; //workaround to send status values to GUI starting from 0
         }
         break;
      }
   case ODO_PULSE_COUNT:
      {
#ifdef DPVDS__ODOMETER_VALUE
         if(m_poDataPool)
         {
            m_poDataPool->u32dp_get( DPVDS__ODOMETER_VALUE, &u16OdoCount, sizeof(u16OdoCount) );
         }
#endif
         u16Data = u16OdoCount;
         break;
      }
   default:
      {
      }
   }
   return u16Data;
}

/**
* Method: vGetTunerPS
* Return current PS
* B1
*/
void clHSA_TestMode::vGetTunerPS(GUI_String *out_result,ulword Tuner)
{	
   /** This is work around to send the event to HSI
   When Framework provides this trigger points
   we have to remove this function call as well as definition
   and we can remove the datapool ID's used in this function 
   from DVPTSM.xml */
   vSendCommandtoHSI( HSI__TUN__FEATURE_TM_DATA_MODE_ON_FG, 
      DPDVPTSM__EVENTFLAG_MONITOR_FLAG );

   /** Allocate memory dynamically for the 
   Buffer to hold the formatted Strings */
   tChar* tCharpacText = NULL;
   tU32 u32Size = 0;
   tCharpacText = OSAL_NEW tChar[TESTMODE_DLR_GPS_STRLEN];
   if(tCharpacText != NULL)
      OSAL_pvMemorySet((tVoid*)tCharpacText,NULL,TESTMODE_DLR_GPS_STRLEN);

   switch(Tuner)
   {
   case TUNER_0: 
      {
         u32Size   = m_poDataPool->u32dp_getElementSize(DPTUNER__TESTMODE_PS_TUNER_0);
         tChar* pacText = OSAL_NEW tChar[u32Size+1];
         if(pacText != NULL)
         {
            OSAL_pvMemorySet((tVoid*)pacText,NULL,u32Size+1);

            if( NULL != m_poDataPool)
            {
               m_poDataPool->u32dp_get( DPTUNER__TESTMODE_PS_TUNER_0,
                  pacText, 
                  u32Size	);
            }
            UTF8_s32SaveNPrintFormat( tCharpacText, TESTMODE_DLR_GPS_STRLEN, "%s", pacText);
            OSAL_DELETE[] pacText;
         }

         break;
      }
   case TUNER_1:
      {
         u32Size   = m_poDataPool->u32dp_getElementSize(DPTUNER__TESTMODE_PS_TUNER_1);
         tChar* pacText = OSAL_NEW tChar[u32Size+1];
         if(pacText != NULL)
         {
            OSAL_pvMemorySet((tVoid*)pacText,NULL,u32Size+1);

            if( NULL != m_poDataPool)
            {
               m_poDataPool->u32dp_get( DPTUNER__TESTMODE_PS_TUNER_1,
                  pacText, 
                  u32Size	);
            }
            UTF8_s32SaveNPrintFormat( tCharpacText, TESTMODE_DLR_GPS_STRLEN, "%s", pacText);
            OSAL_DELETE[] pacText;
         }
         break;
      }
   case TUNER_2:
      {
         u32Size   = m_poDataPool->u32dp_getElementSize(DPTUNER__TESTMODE_PS_TUNER_2);
         tChar* pacText = OSAL_NEW tChar[u32Size+1];
         if(pacText != NULL)
         {
            OSAL_pvMemorySet((tVoid*)pacText,NULL,u32Size+1);

            if( NULL != m_poDataPool)
            {
               m_poDataPool->u32dp_get( DPTUNER__TESTMODE_PS_TUNER_2,
                  pacText, 
                  u32Size	);
            }
            UTF8_s32SaveNPrintFormat( tCharpacText, TESTMODE_DLR_GPS_STRLEN, "%s", pacText);
            OSAL_DELETE[] pacText;
         }
         break;
      }
   case TUNER_3:
      {
         u32Size   = m_poDataPool->u32dp_getElementSize(DPTUNER__TESTMODE_PS_TUNER_3);
         tChar* pacText = OSAL_NEW tChar[u32Size+1];
         if(pacText != NULL)
         {
            OSAL_pvMemorySet((tVoid*)pacText,NULL,u32Size+1);

            if( NULL != m_poDataPool)
            {
               m_poDataPool->u32dp_get( DPTUNER__TESTMODE_PS_TUNER_3,
                  pacText, 
                  u32Size	);
            }
            UTF8_s32SaveNPrintFormat( tCharpacText, TESTMODE_DLR_GPS_STRLEN, "%s", pacText);
            OSAL_DELETE[] pacText;
         }
         break;
      }
   default:
      {
         // Nothing to do Here
         break;
      }
   }//End of Switch Case



   GUI_String_vSetCStr(out_result,(tU8*)tCharpacText);




   if(m_poTrace)
   {	
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
         TR_CLASS_HMI_HSA_SYSTEM_TESTMODE,
         (tU16)(HSA_API_RAYALU__GET_TUNER_PS ),
         MAX_GPS_FORMAT_STRLEN,
         (tU8*)tCharpacText );
   }


   /** Release the memory Allocated dynamically */
   OSAL_DELETE[] tCharpacText;	

}
/**
* Method: vGetTunerHUB
* Return current HUB
* B1
*/
void clHSA_TestMode::vGetTunerHUB(GUI_String *out_result,ulword Tuner)
{

   vSendCommandtoHSI( HSI__TUN__FEATURE_TM_DATA_MODE_ON_FG, 
      DPDVPTSM__EVENTFLAG_MONITOR_FLAG );

   /** Allocate memory dynamically for the 
   Buffer to hold the formatted Strings */
   tChar* tCharpacText = NULL;
   tCharpacText = OSAL_NEW tChar[TESTMODE_DLR_GPS_STRLEN];
   if(tCharpacText != NULL)
      OSAL_pvMemorySet((tVoid*)tCharpacText,NULL,TESTMODE_DLR_GPS_STRLEN);

   tU8 u8HubValue = 0;

   switch(Tuner)
   {
   case TUNER_0: 
      {
         if( NULL != m_poDataPool)
         {

            m_poDataPool->u32dp_get( DPTUNER__TESTMODE_HUB_TUNER_0,
               &u8HubValue, 
               sizeof(u8HubValue)	);

         }			

         break;
      }
   case TUNER_1:
      {
         if( NULL != m_poDataPool)
         {

            m_poDataPool->u32dp_get( DPTUNER__TESTMODE_HUB_TUNER_1,
               &u8HubValue, 
               sizeof(u8HubValue)	);

         }		

         break;
      }
   case TUNER_2:
      {
         if( NULL != m_poDataPool)
         {

            m_poDataPool->u32dp_get( DPTUNER__TESTMODE_HUB_TUNER_2,
               &u8HubValue,
               sizeof(u8HubValue)	);

         }

         break;
      }
   case TUNER_3:
      {
         if( NULL != m_poDataPool)
         {

            m_poDataPool->u32dp_get( DPTUNER__TESTMODE_HUB_TUNER_3,
               &u8HubValue, 
               sizeof(u8HubValue)	);

         }		

         break;
      }
   default:
      {
         // Nothing to do Here
         break;
      }
   } //End of switch


   /*

   Checking for the HUB value. 
   If the HUB value is greater than the MAX_HUB_VALUE(0X7F), assigning the Max HUB value that is allowed.

   */

   if ( u8HubValue > MAX_HUB_VALUE )
   {
      u8HubValue = MAX_HUB_VALUE;
   }

   UTF8_s32SaveNPrintFormat( tCharpacText, 
      TESTMODE_DLR_GPS_STRLEN, 
      "%X", 
      u8HubValue	);

   GUI_String_vSetCStr(out_result,(tU8*)tCharpacText);

   if(m_poTrace)
   {	
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
         TR_CLASS_HMI_HSA_SYSTEM_TESTMODE,
         (tU16)(HSA_API_RAYALU__GET_TUNER_HUB ),
         TESTMODE_DLR_GPS_STRLEN,
         (tU8*)tCharpacText );
   }
   /** Release the memory Allocated dynamically */
   OSAL_DELETE[] tCharpacText;
}
/**
* Method: vGetTunerPD
* Returns Phase diversity of the requested tuner
*
*/

void clHSA_TestMode::vGetTunerPD(GUI_String *out_result,ulword Tuner)
{
   tChar* tCharpacText = NULL;
   tCharpacText = OSAL_NEW tChar[TESTMODE_PD_LEN];
   if(tCharpacText != NULL)
      OSAL_pvMemorySet((tVoid*)tCharpacText,NULL,TESTMODE_PD_LEN);

   tU8 u8DDA = 0;
   if( NULL != m_poDataPool)
   {
      switch(Tuner)
      {
      case TUNER_0:
         {

            m_poDataPool->u32dp_get( DPTUNER__TESTMODE_DDA_TUNER_0,
               &u8DDA, 
               sizeof(u8DDA)	);
            break;

         }
      case TUNER_1:
         {
            m_poDataPool->u32dp_get( DPTUNER__TESTMODE_DDA_TUNER_1,
               &u8DDA, 
               sizeof(u8DDA)	);
            break;

         }
      case TUNER_2:
         {
            m_poDataPool->u32dp_get( DPTUNER__TESTMODE_DDA_TUNER_2,
               &u8DDA, 
               sizeof(u8DDA)	);
            break;

         }
      case TUNER_3:
         {
            m_poDataPool->u32dp_get( DPTUNER__TESTMODE_DDA_TUNER_3,
               &u8DDA, 
               sizeof(u8DDA)	);
            break;

         }
      default:
         {
            //Nothing to do
            break;
         }
      }


   }

   if(u8DDA)
   {
      UTF8_s32SaveNPrintFormat( tCharpacText,TESTMODE_PD_LEN,"%s","ON");
   }
   else
   {
      UTF8_s32SaveNPrintFormat( tCharpacText,TESTMODE_PD_LEN,"%s","OFF");
   }
   GUI_String_vSetCStr(out_result,(tU8*)tCharpacText);

   if(m_poTrace)
   {	
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
         TR_CLASS_HMI_HSA_SYSTEM_TESTMODE,
         (tU16)(HSA_API_RAYALU__GET_TUNER_MODE ),
         TESTMODE_PD_LEN,
         (tU8*)tCharpacText );
   }
   /** Release the memory Allocated dynamically */
   OSAL_DELETE[] tCharpacText;

}


/**
* Method: ulwGetTunerMode
* Returns current Tuner Mode
* B1
*/
ulword clHSA_TestMode::ulwGetTunerMode()
{
   /** This is work around to send the event to HSI
   When Framework provides this trigger points
   we have to remove this function call as well as definition
   and we can remove the datapool ID's used in this function 
   from DVPTSM.xml */
   /*vSendCommandtoHSI( HSI__TUN__FEATURE_TM_DATA_MODE_ON_FG, 
   DPDVPTSM__EVENTFLAG_MONITOR_FLAG );*/

   tU8 u8Mode = 0;

   if( NULL != m_poDataPool)
   {
      m_poDataPool->u32dp_get( DPTUNER__MODE_SETUP, 
         &u8Mode, 
         sizeof(u8Mode)	);	
   }
   if (u8Mode == midw_fi_tcl_e8_DDADDS_Mode::FI_EN_TUN_AUTO)
      u8Mode = FMMONITOR_MODE_CUSTOMER ;		//customer mode is sent as 0 to the above layer(GUI)

   return (ulword)u8Mode;
}

/**
* Method: vGetTunerSignalQuality
* Returns Tuner signal quality
* B1
*/
void clHSA_TestMode::vGetTunerSignalQuality(GUI_String *out_result,ulword Tuner)
{ 

   vSendCommandtoHSI( HSI__TUN__FEATURE_TM_DATA_MODE_ON_FG, 
      DPDVPTSM__EVENTFLAG_MONITOR_FLAG );

   /** Allocate memory dynamically for the 
   Buffer to hold the formatted Strings */
   tChar* tCharpacText = NULL;
   tCharpacText = OSAL_NEW tChar[TESTMODE_DLR_GPS_STRLEN];
   if(tCharpacText != NULL)
      OSAL_pvMemorySet((tVoid*)tCharpacText,NULL,TESTMODE_DLR_GPS_STRLEN);

   tU8 u8SignalQuality = 0;

   switch(Tuner)
   {
   case TUNER_0: 
      {
         if( NULL != m_poDataPool)
         {

            m_poDataPool->u32dp_get( DPTUNER__TESTMODE_QUALITY_TUNER_0,
               &u8SignalQuality, 
               sizeof(u8SignalQuality)	);

         }

         break;
      }
   case TUNER_1:
      {
         if( NULL != m_poDataPool)
         {

            m_poDataPool->u32dp_get( DPTUNER__TESTMODE_QUALITY_TUNER_1,
               &u8SignalQuality, 
               sizeof(u8SignalQuality)	);
         }
         break;
      }
   case TUNER_2:
      {
         if( NULL != m_poDataPool)
         {

            m_poDataPool->u32dp_get( DPTUNER__TESTMODE_QUALITY_TUNER_2,
               &u8SignalQuality, 
               sizeof(u8SignalQuality)	);
         }
         break;
      }
   case TUNER_3:
      {
         if( NULL != m_poDataPool)
         {

            m_poDataPool->u32dp_get( DPTUNER__TESTMODE_QUALITY_TUNER_3,
               &u8SignalQuality, 
               sizeof(u8SignalQuality)	);		   
         }			

         break;
      }
   default:
      {
         // Nothing to do Here
         break;
      }
   } //End of switch

   /*

   Checking for the  Signal Quality value. 
   If the Signal Quality value is greater than the MAX_SIGNAL_QUALITY_VALUE(0X1F), assigning the Max  Signal Quality value that is allowed.

   */

   if ( u8SignalQuality > MAX_SIGNAL_QUALITY_VALUE )
   {
      u8SignalQuality = MAX_SIGNAL_QUALITY_VALUE;
   }

   UTF8_s32SaveNPrintFormat( tCharpacText, 
      TESTMODE_DLR_GPS_STRLEN, 
      "%X", 
      u8SignalQuality	);

   GUI_String_vSetCStr(out_result,(tU8*)tCharpacText);

   if(m_poTrace)
   {	
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
         TR_CLASS_HMI_HSA_SYSTEM_TESTMODE,
         (tU16)(HSA_API_RAYALU__GET_TUNER_SIGNAL_QUALITY ),
         TESTMODE_DLR_GPS_STRLEN,
         (tU8*)tCharpacText );
   }
   /** Release the memory Allocated dynamically */
   OSAL_DELETE[] tCharpacText;
}


/****************************************************************************
********************* CAN RELATED API CALLS *********************************
****************************************************************************/

/**
* Method: vGetCANKL15
* Returns value of KL15
* B1
*/
void clHSA_TestMode::vGetCANKL15(GUI_String *out_result)
{
   /** Allocate memory dynamically for the 
   Buffer to hold the formatted Strings */
   tChar* tCharpacText = NULL;
   tCharpacText = OSAL_NEW tChar[TESTMODE_DLR_GPS_STRLEN];	
   if(tCharpacText != NULL)
      OSAL_pvMemorySet((tVoid*)tCharpacText,NULL,TESTMODE_DLR_GPS_STRLEN);

   tU8 u8CAN_Not_Connected = 0;
   tU32 u32CanKL15 = 0;
   if( NULL != m_poDataPool)
   {

      m_poDataPool->u32dp_get( DPDVPTSM__CAN_KL15, 
         &u32CanKL15, 
         sizeof(u32CanKL15)	);

      m_poDataPool->u32dp_get( DPDVPTSM__CAN_IS_NOT_CONNECTED, 
         &u8CAN_Not_Connected, 
         sizeof(u8CAN_Not_Connected)	);
   }
   tBool bCANValue = FALSE;
   bCANValue = (tBool)( u8CAN_Not_Connected  & CAN_KL15_BIT );

   if( FALSE == bCANValue )
   {
      if( (CANSIGNAL_ZERO == u32CanKL15) || (CANSIGNAL_ONE == u32CanKL15) )
      {
         UTF8_s32SaveNPrintFormat( tCharpacText,TESTMODE_DLR_GPS_STRLEN,"%s","Off" );
      }
      else if( (CANSIGNAL_TWO == u32CanKL15 ) || (CANSIGNAL_THREE == u32CanKL15) )
      {
         UTF8_s32SaveNPrintFormat( tCharpacText,TESTMODE_DLR_GPS_STRLEN,"%s","On" );
      }
      else
      {
         UTF8_s32SaveNPrintFormat( tCharpacText,TESTMODE_DLR_GPS_STRLEN,"%s","Off" );
      }
   }
   else
   {
      UTF8_s32SaveNPrintFormat( tCharpacText,TESTMODE_DLR_GPS_STRLEN,"%s","NO COMM" );
   }
   GUI_String_vSetCStr(out_result, (tU8*)tCharpacText);


   if(m_poTrace)
   {	
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
         TR_CLASS_HMI_HSA_SYSTEM_TESTMODE,
         (tU16)(HSA_API_RAYALU__GET_CANKL15 ),
         TESTMODE_DLR_GPS_STRLEN,
         (tU8*)tCharpacText );
   }

   /** Release the memory Allocated dynamically */
   OSAL_DELETE[] tCharpacText;
}
/**
* Method: vGetCANSpeedSignal
* Returns value of KL15
* B1
*/
void clHSA_TestMode::vGetCANSpeedSignal(GUI_String *out_result)
{
   /** Allocate memory dynamically for the 
   Buffer to hold the formatted Strings */
   tChar* tCharpacText = NULL;
   tCharpacText = OSAL_NEW tChar[TESTMODE_DLR_GPS_STRLEN];	
   if(tCharpacText != NULL)
      OSAL_pvMemorySet((tVoid*)tCharpacText,NULL,TESTMODE_DLR_GPS_STRLEN);

   tU8 u8CAN_Not_Connected = 0;
   tU32 u32CanSpeed = 0;
   if( NULL != m_poDataPool)
   {

      m_poDataPool->u32dp_get( DPDVPTSM__CAN_SPEED, 
         &u32CanSpeed, 
         sizeof(u32CanSpeed)	);

      m_poDataPool->u32dp_get( DPDVPTSM__CAN_IS_NOT_CONNECTED, 
         &u8CAN_Not_Connected, 
         sizeof(u8CAN_Not_Connected)	);
   }
   if ( CANU32_MAXVALUE == u32CanSpeed )
   {
      u32CanSpeed = 0;
   }

   tBool bCANValue = FALSE;
   bCANValue = (tBool)( u8CAN_Not_Connected & CAN_SPEED_BIT );

   if( FALSE == bCANValue )
   {   
      UTF8_s32SaveNPrintFormat( tCharpacText,
         TESTMODE_DLR_GPS_STRLEN,
         "%u km/h ",
         u32CanSpeed );
   }
   else
   {
      UTF8_s32SaveNPrintFormat( tCharpacText,TESTMODE_DLR_GPS_STRLEN,"%s","NO COMM" );	
   }

   GUI_String_vSetCStr(out_result, (tU8*)tCharpacText);


   if(m_poTrace)
   {	
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
         TR_CLASS_HMI_HSA_SYSTEM_TESTMODE,
         (tU16)(HSA_API_RAYALU__GET_CAN_SPEED_SIGNAL ),
         TESTMODE_DLR_GPS_STRLEN,
         (tU8*)tCharpacText );
   }

   /** Release the memory Allocated dynamically */
   OSAL_DELETE[] tCharpacText;
}
/**
* Method: vGetCANSKontakt
* Returns value of KL15
* B1
*/
void clHSA_TestMode::vGetCANSKontakt(GUI_String *out_result)
{
   /** Allocate memory dynamically for the 
   Buffer to hold the formatted Strings */
   tChar* tCharpacText = NULL;
   tCharpacText = OSAL_NEW tChar[10];	
   if(tCharpacText != NULL)
      OSAL_pvMemorySet((tVoid*)tCharpacText,NULL,10);

   tU8 u8CAN_Not_Connected = 0;
   tU32 u32CanKL15 = 0;

   if( NULL != m_poDataPool)
   {

      m_poDataPool->u32dp_get( DPDVPTSM__CAN_KL15, 
         &u32CanKL15, 
         sizeof(u32CanKL15)	);

      m_poDataPool->u32dp_get( DPDVPTSM__CAN_IS_NOT_CONNECTED, 
         &u8CAN_Not_Connected, 
         sizeof(u8CAN_Not_Connected)	);
   }
   tBool bCANValue = FALSE;

   bCANValue = (tBool)( u8CAN_Not_Connected  & CAN_KL15_BIT );

   if( FALSE == bCANValue )
   {
      if(CANSIGNAL_ZERO == u32CanKL15)
      {
         UTF8_s32SaveNPrintFormat( tCharpacText,TESTMODE_DLR_GPS_STRLEN,"%s","Off" );
      }
      else if( (CANSIGNAL_ONE == u32CanKL15) || (CANSIGNAL_TWO == u32CanKL15 ) || (CANSIGNAL_THREE == u32CanKL15) )
      {
         UTF8_s32SaveNPrintFormat( tCharpacText,TESTMODE_DLR_GPS_STRLEN,"%s","On" );
      }
      else
      {
         UTF8_s32SaveNPrintFormat( tCharpacText,TESTMODE_DLR_GPS_STRLEN,"%s","Off" );
      }
   }
   else
   {      
      UTF8_s32SaveNPrintFormat( tCharpacText,TESTMODE_DLR_GPS_STRLEN,"%s","NO COMM" );
   }
   GUI_String_vSetCStr(out_result, (tU8*)tCharpacText);


   if(m_poTrace)
   {	
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
         TR_CLASS_HMI_HSA_SYSTEM_TESTMODE,
         (tU16)(HSA_API_RAYALU__GET_CANS_KONTAKT ),
         TESTMODE_DLR_GPS_STRLEN,
         (tU8*)tCharpacText );
   }

   /** Release the memory Allocated dynamically */
   OSAL_DELETE[] tCharpacText;
}
/**
* Method: vGetCANReverseGear
* Returns value of KL15
* B1
*/
void clHSA_TestMode::vGetCANReverseGear(GUI_String *out_result)
{
   /** Allocate memory dynamically for the 
   Buffer to hold the formatted Strings */
   tChar* tCharpacText = NULL;
   tCharpacText = OSAL_NEW tChar[TESTMODE_DLR_GPS_STRLEN];	
   if(tCharpacText != NULL)
      OSAL_pvMemorySet((tVoid*)tCharpacText,NULL,TESTMODE_DLR_GPS_STRLEN);

   tBool bCanReverseGear = FALSE;
   tU8 u8CAN_Not_Connected = 0;
   if( NULL != m_poDataPool)
   {

      m_poDataPool->u32dp_get( DPDVPTSM__CAN_REVERSEGEAR, 
         &bCanReverseGear, 
         sizeof(bCanReverseGear)	);

      m_poDataPool->u32dp_get( DPDVPTSM__CAN_IS_NOT_CONNECTED, 
         &u8CAN_Not_Connected, 
         sizeof(u8CAN_Not_Connected)	);
   }

   tBool bCANValue = FALSE;
   bCANValue = (tBool)( u8CAN_Not_Connected  & CAN_REVERSEGEAR_BIT );

   if( FALSE == bCANValue )
   {
      if( FALSE == bCanReverseGear )
      {
         UTF8_s32SaveNPrintFormat( tCharpacText,TESTMODE_DLR_GPS_STRLEN,"%s","Off");
      }
      else
      {
         UTF8_s32SaveNPrintFormat( tCharpacText,TESTMODE_DLR_GPS_STRLEN,"%s","On");
      }
   }
   else
   {     
      UTF8_s32SaveNPrintFormat( tCharpacText,TESTMODE_DLR_GPS_STRLEN,"%s","NO COMM" );
   }

   GUI_String_vSetCStr(out_result, (tU8*)tCharpacText);


   if(m_poTrace)
   {	
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
         TR_CLASS_HMI_HSA_SYSTEM_TESTMODE,
         (tU16)(HSA_API_RAYALU__GET_CAN_REVERSE_GEAR ),
         TESTMODE_DLR_GPS_STRLEN,
         (tU8*)tCharpacText );
   }

   /** Release the memory Allocated dynamically */
   OSAL_DELETE[] tCharpacText;
}
/**
* Method: vGetCANMuteValue
* Returns value of KL15
* B1
*/
void clHSA_TestMode::vGetCANMuteValue(GUI_String *out_result)
{
   /** Allocate memory dynamically for the 
   Buffer to hold the formatted Strings */
   tChar* tCharpacText = NULL;
   tCharpacText = OSAL_NEW tChar[TESTMODE_DLR_GPS_STRLEN];	
   if(tCharpacText != NULL)
      OSAL_pvMemorySet((tVoid*)tCharpacText,NULL,TESTMODE_DLR_GPS_STRLEN);

   tU32 u32CanMuteValue = 0;
   tU8 u8CAN_Not_Connected = 0;
   if( NULL != m_poDataPool)
   {

      m_poDataPool->u32dp_get( DPDVPTSM__CAN_MUTEVALUE, 
         &u32CanMuteValue, 
         sizeof(u32CanMuteValue)	);

      m_poDataPool->u32dp_get( DPDVPTSM__CAN_IS_NOT_CONNECTED, 
         &u8CAN_Not_Connected, 
         sizeof(u8CAN_Not_Connected)	);
   }

   tBool bCANValue = FALSE;	

   bCANValue =  (tBool)( u8CAN_Not_Connected  & CAN_MUTEVALUE_BIT );

   if(  FALSE == bCANValue )
   {
      UTF8_s32SaveNPrintFormat( tCharpacText,
         TESTMODE_DLR_GPS_STRLEN,
         "%d\0",
         u32CanMuteValue );
   }
   else
   {
      UTF8_s32SaveNPrintFormat( tCharpacText,TESTMODE_DLR_GPS_STRLEN,"%s","NO COMM" );
   }

   GUI_String_vSetCStr(out_result, (tU8*)tCharpacText);


   if(m_poTrace)
   {	
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
         TR_CLASS_HMI_HSA_SYSTEM_TESTMODE,
         (tU16)(HSA_API_RAYALU__GET_CAN_MUTE_VALUE ),
         TESTMODE_DLR_GPS_STRLEN,
         (tU8*)tCharpacText );
   }

   /** Release the memory Allocated dynamically */
   OSAL_DELETE[] tCharpacText;
}
/**
* Method: vGetCANMuteBit
* Returns value of KL15
* B1
*/
void clHSA_TestMode::vGetCANMuteBit(GUI_String *out_result)
{
   /** Allocate memory dynamically for the 
   Buffer to hold the formatted Strings */
   tChar* tCharpacText = NULL;
   tCharpacText = OSAL_NEW tChar[TESTMODE_DLR_GPS_STRLEN];	
   if(tCharpacText != NULL)
      OSAL_pvMemorySet((tVoid*)tCharpacText,NULL,TESTMODE_DLR_GPS_STRLEN);

   tBool bCanMuteBit = FALSE;
   tU8 u8CAN_Not_Connected = 0;
   if( NULL != m_poDataPool)
   {

      m_poDataPool->u32dp_get( DPDVPTSM__CAN_MUTE, 
         &bCanMuteBit, 
         sizeof(bCanMuteBit)	);

      m_poDataPool->u32dp_get( DPDVPTSM__CAN_IS_NOT_CONNECTED, 
         &u8CAN_Not_Connected, 
         sizeof(u8CAN_Not_Connected)	);
   }

   tBool bCANValue = FALSE;
   bCANValue = (tBool)( u8CAN_Not_Connected & CAN_MUTE_BIT );

   if( FALSE == bCANValue )
   {
      if( FALSE == bCanMuteBit )
      {		
         UTF8_s32SaveNPrintFormat( tCharpacText,TESTMODE_DLR_GPS_STRLEN,"%s","Off" );
      }
      else
      {
         UTF8_s32SaveNPrintFormat( tCharpacText,TESTMODE_DLR_GPS_STRLEN,"%s","On" );
      }
   }
   else
   {
      UTF8_s32SaveNPrintFormat( tCharpacText,TESTMODE_DLR_GPS_STRLEN,"%s","NO COMM" );
   }
   GUI_String_vSetCStr(out_result, (tU8*)tCharpacText);


   if(m_poTrace)
   {	
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
         TR_CLASS_HMI_HSA_SYSTEM_TESTMODE,
         (tU16)(HSA_API_RAYALU__GET_CAN_MUTE_BIT ),
         TESTMODE_DLR_GPS_STRLEN,
         (tU8*)tCharpacText );
   }

   /** Release the memory Allocated dynamically */
   OSAL_DELETE[] tCharpacText;
}
/**
* Method: vGetCANKL58D
* Returns value of KL15
* B1
*/
void clHSA_TestMode::vGetCANKL58D(GUI_String *out_result)
{
   /** Allocate memory dynamically for the 
   Buffer to hold the formatted Strings */
   tChar* tCharpacText = NULL;
   tCharpacText = OSAL_NEW tChar[TESTMODE_DLR_GPS_STRLEN];	
   if(tCharpacText != NULL)
      OSAL_pvMemorySet((tVoid*)tCharpacText,NULL,TESTMODE_DLR_GPS_STRLEN);

   tU32 u32CanKL58D = 15;
   tU8 u8CAN_Not_Connected = 0;
   if( NULL != m_poDataPool)
   {
      m_poDataPool->u32dp_get( DPDVPTSM__CAN_KL58D, 
         &u32CanKL58D, 
         sizeof(u32CanKL58D)	);
      m_poDataPool->u32dp_get( DPDVPTSM__CAN_IS_NOT_CONNECTED, 
         &u8CAN_Not_Connected, 
         sizeof(u8CAN_Not_Connected)	);
   }

   tBool bCANValue = FALSE;

   bCANValue = (tBool)( u8CAN_Not_Connected & CAN_KL58D_BIT );

   if( FALSE == bCANValue )
   {
      if( CANSIGNAL_ZERO == u32CanKL58D )
      {		
         UTF8_s32SaveNPrintFormat( tCharpacText,TESTMODE_DLR_GPS_STRLEN,"%s","Off" );
      }
      else
      {
         UTF8_s32SaveNPrintFormat( tCharpacText,TESTMODE_DLR_GPS_STRLEN,"%s","On" );
      }
   }
   else
   {      
      UTF8_s32SaveNPrintFormat( tCharpacText,TESTMODE_DLR_GPS_STRLEN,"%s","NO COMM" );
   }
   GUI_String_vSetCStr(out_result, (tU8*)tCharpacText);


   if(m_poTrace)
   {	
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
         TR_CLASS_HMI_HSA_SYSTEM_TESTMODE,
         (tU16)(HSA_API_RAYALU__GET_CANKL58D ),
         TESTMODE_DLR_GPS_STRLEN,
         (tU8*)tCharpacText );
   }

   /** Release the memory Allocated dynamically */
   OSAL_DELETE[] tCharpacText;
}
/**
* Method: vGetCANAnalogMute
* Returns value of KL15
* B1
*/
void clHSA_TestMode::vGetCANAnalogMute(GUI_String *out_result)
{
   /** Allocate memory dynamically for the 
   Buffer to hold the formatted Strings */
   tChar* tCharpacText = NULL;
   tCharpacText = OSAL_NEW tChar[TESTMODE_DLR_GPS_STRLEN];	
   if(tCharpacText != NULL)
      OSAL_pvMemorySet((tVoid*)tCharpacText,NULL,TESTMODE_DLR_GPS_STRLEN);

   tBool bCanAnalogMute = FALSE;
   if( NULL != m_poDataPool)
   {
      m_poDataPool->u32dp_get( DPDVPTSM__CAN_ANALOGMUTE, 
         &bCanAnalogMute, 
         sizeof(bCanAnalogMute)	);
   }

   if( FALSE == bCanAnalogMute )
   {
      UTF8_s32SaveNPrintFormat( tCharpacText,TESTMODE_DLR_GPS_STRLEN,"%s","Off" );
   }
   else
   {
      UTF8_s32SaveNPrintFormat( tCharpacText,TESTMODE_DLR_GPS_STRLEN,"%s","On" );
   }

   GUI_String_vSetCStr(out_result, (tU8*)tCharpacText);


   if(m_poTrace)
   {	
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
         TR_CLASS_HMI_HSA_SYSTEM_TESTMODE,
         (tU16)(HSA_API_RAYALU__GET_CAN_ANALOG_MUTE ),
         TESTMODE_DLR_GPS_STRLEN,
         (tU8*)tCharpacText );
   }

   /** Release the memory Allocated dynamically */
   OSAL_DELETE[] tCharpacText;
}

/**
* Method: ulwGetActiveTuner
* Returns the active Tuner
* B1
*/
ulword clHSA_TestMode::ulwGetActiveTuner()
{
   tU8 u8ActiveTuner_Get = 0;

   if(m_poDataPool)
   {
      m_poDataPool->u32dp_get( DPTUNER__TESTMODE_ACTIVE_TUNER,
         &u8ActiveTuner_Get,
         sizeof(u8ActiveTuner_Get)	);
   }

   return (ulword)u8ActiveTuner_Get;
}
/**
* Method: vSetActiveTuner
* Sets the active Tuner
* B1
*/
void clHSA_TestMode::vSetActiveTuner(ulword Tuner)
{
   tU8 u8ActiveTuner_Get = Tuner;

   //Sending Msg to HSI Testmode
   if(pclHSI_TunerMaster != NULL)
   {
      sMsg refMsg (NULL, (tU32)CMMSG_TUNERMASTER_SOURCE_CHANGE,(tU32)u8ActiveTuner_Get, NULL) ;
      pclHSI_TunerMaster->bExecuteMessage(refMsg);
   }

}
/**
* Method: vSetActiveBand
* Sets the active Tuner
* B1
*/
void clHSA_TestMode::vSetActiveBand(ulword Band)
{
   tU8 u8Band = Band;

   //Sending Msg to HSI Testmode
   if(pclHSI_TunerMaster != NULL)
   {
      sMsg refMsg (NULL, (tU32)CMMSG_TUNERMASTER_ACTIVATE_BAND,(tU32)u8Band, NULL) ;
      pclHSI_TunerMaster->bExecuteMessage(refMsg);
   }

}

/**
* Method: ulwGetActiveBand
* Returns current band of the requested tuner
* */

ulword clHSA_TestMode::ulwGetActiveBand(ulword Tuner)
{
   tU8 u8ActiveBand = 0;

   if( NULL != m_poDataPool)
   {
      switch (Tuner)
      {
      case TUNER_0:
         {
            m_poDataPool->u32dp_get( DPTUNER__TESTMODE_BAND_TUNER_0,
               &u8ActiveBand,
               sizeof(u8ActiveBand)	);
            break;
         }
      case TUNER_1:
         {
            m_poDataPool->u32dp_get( DPTUNER__TESTMODE_BAND_TUNER_1,
               &u8ActiveBand,
               sizeof(u8ActiveBand)	);
            break;
         }
      case TUNER_2:
         {
            m_poDataPool->u32dp_get( DPTUNER__TESTMODE_BAND_TUNER_2,
               &u8ActiveBand,
               sizeof(u8ActiveBand)	);
            break;
         }
      case TUNER_3:
         {
            m_poDataPool->u32dp_get( DPTUNER__TESTMODE_BAND_TUNER_3,
               &u8ActiveBand,
               sizeof(u8ActiveBand)	);
            break;
         }
      default:
         //Nothing to do
         break;

      }

   }
   return (tU32)u8ActiveBand;
}
/**
* Method: blGetTunerMeasureMode
* Returns Measure mode
* */
tbool clHSA_TestMode::blGetTunerMeasureMode()
{
   tbool bMeasureMode = FALSE;
   if( NULL != m_poDataPool)
   {
      m_poDataPool->u32dp_get( DPTUNER__FEATURE_MEASUREMODE,
         &bMeasureMode,
         sizeof(bMeasureMode)	);
   }
   return bMeasureMode;

}
/**
* Method: vToggleMeasureMode
* Toggles the Measure mode
* */
void clHSA_TestMode::vToggleMeasureMode()
{
   tbool bMM_Toggle = FALSE;
   if( NULL != m_poDataPool)
   {
      m_poDataPool->u32dp_get( DPTUNER__FEATURE_MEASUREMODE,
         &bMM_Toggle,
         sizeof(bMM_Toggle)	);
   }
   if ( TRUE == bMM_Toggle)
   {
      bMM_Toggle = FALSE;
   }
   else 	{
      bMM_Toggle = TRUE;
   }
   if(pclHSI_TestMode != NULL)
   {
      sMsg refMsg (NULL, (tU32)CMMSG_TESTMODE_RADIO_MEASURE_MODE,(tU32)bMM_Toggle, NULL) ;
      pclHSI_TestMode->bExecuteMessage(refMsg);
   }
}
/**
* Method: blGetTunerRDSReg
* Returns RDS Reg
* */
tbool clHSA_TestMode::blGetTunerRDSReg()
{
   tbool bRDSReg = FALSE;

   if( NULL != m_poDataPool)
   {
      m_poDataPool->u32dp_get( DPTUNER__FEATURE_REG,
         &bRDSReg,
         sizeof(bRDSReg)	);
   }
   return bRDSReg;

}
/**
* Method: vToggleRDSReg
* Toggles the RDS Reg
* */
void clHSA_TestMode::vToggleRDSReg()
{
   tbool bRDSReg_Toggle = FALSE;
   if( NULL != m_poDataPool)
   {
      m_poDataPool->u32dp_get( DPTUNER__FEATURE_REG,
         &bRDSReg_Toggle,
         sizeof(bRDSReg_Toggle)	);
   }
   if ( TRUE == bRDSReg_Toggle)
   {
      bRDSReg_Toggle = FALSE;
   }
   else 
   {
      bRDSReg_Toggle = TRUE;
   }
   if(pclHSI_TestMode != NULL)
   {
      sMsg refMsg (NULL, (tU32)CMMSG_TESTMODE_RADIO_RDS_REG,(tU32)bRDSReg_Toggle, NULL) ;
      pclHSI_TestMode->bExecuteMessage(refMsg);
   }
}
/**
* Method: blGetTASetup
* returns the TA setup for all the tuners
* */
tbool clHSA_TestMode::blGetTASetup()
{
   tbool bTASetup = FALSE;
   if( NULL != m_poDataPool)
   {
      m_poDataPool->u32dp_get( DPTUNER__FEATURE_TA,
         &bTASetup,
         sizeof(bTASetup)	);
   }
   return bTASetup;
}
/**
* Method: vToggleTASetup
* toggles the TA setup for all the tuners
* */
void clHSA_TestMode::vToggleTASetup()
{
   tbool bTASetupSet = FALSE;
   if( NULL != m_poDataPool)
   {
      m_poDataPool->u32dp_get( DPTUNER__FEATURE_TA,
         &bTASetupSet,
         sizeof(bTASetupSet)	);
   }
   if (TRUE == bTASetupSet)
   {
      bTASetupSet = FALSE;
   }
   else 
   {
      bTASetupSet = TRUE;
   }

   if(pclHSI_TunerMaster != NULL)
   {
      sMsg refMsg (NULL, (tU32)CMMSG_TUNERMASTER_SET_TP_STATE,(tU32)bTASetupSet, NULL) ;
      pclHSI_TunerMaster->bExecuteMessage(refMsg);
   }
}
/**
* Method: ulwGetTunerCS
* Returns channel separation for the requested tuner
* */
ulword clHSA_TestMode::ulwGetTunerCS(ulword Tuner)
{
   tU8 u8TunerCS = 0;

   if( NULL != m_poDataPool)
   {
      switch(Tuner)
      {
      case TUNER_0:
         {
            m_poDataPool->u32dp_get( DPTUNER__TESTMODE_CHANNELSEPERATION_TUNER_0,
               &u8TunerCS,
               sizeof(u8TunerCS)	);
            break;
         }
      case TUNER_1:
         {
            m_poDataPool->u32dp_get( DPTUNER__TESTMODE_CHANNELSEPERATION_TUNER_1,
               &u8TunerCS,
               sizeof(u8TunerCS)	);
            break;
         }
      case TUNER_2:
         {
            m_poDataPool->u32dp_get( DPTUNER__TESTMODE_CHANNELSEPERATION_TUNER_2,
               &u8TunerCS,
               sizeof(u8TunerCS)	);
            break;
         }
      case TUNER_3:
         {
            m_poDataPool->u32dp_get( DPTUNER__TESTMODE_CHANNELSEPERATION_TUNER_3,
               &u8TunerCS,
               sizeof(u8TunerCS)	);
            break;
         }
      default:
         //Nothing to do
         break;
      }

   }
   return (tU32)u8TunerCS;

}


/**
* Method: ulwGetTunerModeSetup
* Returns current Tuner Mode for Radio Setup
* B1
*/
ulword clHSA_TestMode::ulwGetTunerModeSetup()
{
   tU8 u8TunerMode = 0;
   if( NULL != m_poDataPool)
   {
#ifdef DPTUNER__MODE_SETUP
      m_poDataPool->u32dp_get( DPTUNER__MODE_SETUP, 
         &u8TunerMode, 
         sizeof(u8TunerMode)	);
#endif
   }

   if( u8TunerMode == 0)
   {
      u8TunerMode =4;
   }
   else if( u8TunerMode == 1 )
   {
      u8TunerMode =2;
   }
   else if(  u8TunerMode == 2 )
   {
      u8TunerMode =1;
   }
   else if(  u8TunerMode == 3 )
   {
      u8TunerMode =3;
   }
   else if(  u8TunerMode == 4 )
   {
      u8TunerMode =0;
   }
   else
   {
   }



   if ( NULL  != m_poTrace)
   {
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO, TR_CLASS_HSI_AUDIO_MSG, "Value send to GUI to display mode is  %d", u8TunerMode);
   }

   if(m_poTrace)
   {	
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
         TR_CLASS_HMI_HSA_SYSTEM_TESTMODE,
         (tU16)(HSA_API_RAYALU__GET_TUNER_MODE_SETUP ),
         1,
         &u8TunerMode );
   }
   return u8TunerMode;

}
/**
* Method: vSetTunerModeSetup
* Sets the Tuner mode in Radio Setup
* B1
*/
void clHSA_TestMode::vSetTunerModeSetup(ulword Mode)
{
   tU8 u8TunerMode = (tU8)Mode;

   if( Mode == FMMONITOR_MODE_CUSTOMER)
      u8TunerMode = midw_fi_tcl_e8_DDADDS_Mode::FI_EN_TUN_AUTO;			//customer mode is sent as 4 to the below layer(middleware)

   if( NULL != m_poDataPool)
   {
#ifdef DPTUNER__MODE_SETUP
      m_poDataPool->u32dp_set( DPTUNER__MODE_SETUP, 
         &u8TunerMode, 
         sizeof(u8TunerMode)	);
#endif
   }
   vSendCommandtoHSI(HSI__TUN__SET_TUNER_MODE_DDSDDA);


   if(m_poTrace)
   {	
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
         TR_CLASS_HMI_HSA_SYSTEM_TESTMODE,
         (tU16)(HSA_API_RAYALU__SET_TUNER_MODE_SETUP ),
         1,
         &u8TunerMode );
   }
}
/**
* Method: vRestoreDefaultSettings
* This call will set all user defined settings from developermode to default settings.
* B1
*/
void clHSA_TestMode::vRestoreDefaultSettings()
{
   // At Present this used to make the flag datapool Elements to Zero
   // And send the events to reduce the system load ( Mainly to switch Off TestMode Update from tuner ).

   tBool bAlwaysZero = FALSE;


   if( NULL != m_poDataPool)
   {
      tBool bMonitorFlag = FALSE;
      tBool bAFListFlag = FALSE;
      m_poDataPool->u32dp_get( DPDVPTSM__EVENTFLAG_MONITOR_FLAG, 
         &bMonitorFlag, 
         sizeof(bMonitorFlag)	);


      //if( TRUE == bMonitorFlag )
      {
         //Send Command to Tuner HSI to stop updating Data.
         vSendCommandtoHSI(HSI__TUN__FEATURE_TM_EXIT_MODE);
      }

      m_poDataPool->u32dp_get( DPDVPTSM__EVENTFLAG_AFLIST, 
         &bAFListFlag, 
         sizeof(bAFListFlag)	);

      /** Send Message to CM Radio to inform to carry out the usual seek operation with SeekNextStation API call */

      if( NULL != pclHSI_Radio )
      {
         sMsg refMsgtoCMRadio (NULL, (tU32)CMMSG_TESTMODE_SERV_ON_TEST_OFF, NULL, NULL) ;
         pclHSI_Radio->bExecuteMessage(refMsgtoCMRadio);
      }

      /*if( bActiveTuner == TRUE )
      {
      clAudioSMEngine* pclRetVal = NULL;
      clHMIMngr *pclHMIMaster = clHMIMngr::pclGetInstance();
      pclHMIMaster->bGetIfcPtr2Engine(&pclRetVal);


      sMsg refMsg(NULL, AUDIO_SM_SRC_OFF, AudioSources::RADIO_FM2, NULL);
      pclRetVal->bExecuteMessage(refMsg);
      bActiveTuner = 0;

      if(NULL != pclHSI_TestMode)
      {
      sMsg refMsgtoCMTest (NULL, (tU32)CMMSG_TESTMODE_ACTIVE_TUNER, (tU32)bActiveTuner, NULL) ;
      pclHSI_TestMode->bExecuteMessage(refMsgtoCMTest);
      }
      else
      {
      // No need to send the event to HSI
      }

      #ifdef DPTUNER__TESTMODE_ACTIVE_TUNER
      m_poDataPool->u32dp_set( DPTUNER__TESTMODE_ACTIVE_TUNER,       
      &u8ActiveTuner,
      sizeof(u8ActiveTuner)	);
      #endif
      }*/

      if( TRUE == bAFListFlag )
      {
         if(m_poDataPool)
         {
            m_poDataPool->u32dp_get( DPTUNER__AFLIST_TRACK,&u8TrackAFList,sizeof(tU8));
         }

         for ( tU8 u8localIndex =0; u8localIndex < u8TrackAFList; u8localIndex++ )
         {
            vSendCommandtoHSI(HSI__TUN__PREV_AF_LIST);
         }
         // Send Command to close AF List.
         vSendCommandtoHSI(HSI__TUN__CLOSE_AF_LIST);
      }


      m_poDataPool->u32dp_set( DPDVPTSM__EVENTFLAG_SWHWVERSIONS, 
         &bAlwaysZero, 
         sizeof(bAlwaysZero)	);

      m_poDataPool->u32dp_set( DPDVPTSM__EVENTFLAG_AFLIST, 
         &bAlwaysZero, 
         sizeof(bAlwaysZero)	);

      m_poDataPool->u32dp_set( DPDVPTSM__EVENTFLAG_RESETLIST, 
         &bAlwaysZero, 
         sizeof(bAlwaysZero)	);

      m_poDataPool->u32dp_set( DPDVPTSM__EVENTFLAG_MONITOR_FLAG, 
         &bAlwaysZero, 
         sizeof(bAlwaysZero)	);

      m_poDataPool->u32dp_set( DPDVPTSM__EVENTFLAG_TMC_FLAG, 
         &bAlwaysZero, 
         sizeof(bAlwaysZero)	);
   }
}


/** Service Mode Functions implemented for B2 Sample */
/**
* Method: ulwGetServiceModeFieldStrength
* Returns the field strength for the servicemode
* B2
*/
ulword clHSA_TestMode::ulwGetServiceModeFieldStrength(ulword Type)
{
   tU8 u8fieldStrength = 0;
   switch(Type)
   {		
   case SERVICEMODE_FIELDSTRENGTH_AM:
      {
         m_poDataPool->u32dp_get( DPTUNER__TESTMODE_FIELDSTRENGTH_TUNER_0,
            &u8fieldStrength, 
            sizeof(u8fieldStrength)	);
         break;
      }
   case SERVICEMODE_FIELDSTRENGTH_FM:
      {
         m_poDataPool->u32dp_get( DPTUNER__TESTMODE_FIELDSTRENGTH_TUNER_0,
            &u8fieldStrength, 
            sizeof(u8fieldStrength)	);
         break;
      }
   case SERVICEMODE_FIELDSTRENGTH_DAB:
      {
         // Nothing to Do here for B2 Sample.
         break;
      }
   default :
      {
         break;
      }
   } // End Of Switch
   if( u8fieldStrength > MAXIMUM_FIELDSTRENGTH )
   {
      u8fieldStrength = MAXIMUM_FIELDSTRENGTH;
   }

   return u8fieldStrength;
}

/**
* Method: ulwGetServiceModeQualityIndicatorValue
* Returns the current progressbar value of the quality indicator for servicemode iboc
* B2
*/
ulword clHSA_TestMode::ulwGetServiceModeQualityIndicatorValue()
{
   tU8 u8fieldStrength = 0;

   m_poDataPool->u32dp_get( DPTUNER__TESTMODE_FIELDSTRENGTH_TUNER_0,
      &u8fieldStrength, 
      sizeof(u8fieldStrength)	);

   if( MINIMUM_FIELDSTRENGTH > u8fieldStrength )
   {
      u8fieldStrength = QUALITY_INDICATOR_ZERO;
   }
   else if(( MINIMUM_FIELDSTRENGTH <= u8fieldStrength) || 
      ( MAXIMUM_FIELDSTRENGTH >= u8fieldStrength)		)
   {
      u8fieldStrength = ( ( ( u8fieldStrength - MINIMUM_FIELDSTRENGTH )/
         ( MAXIMUM_FIELDSTRENGTH - MINIMUM_FIELDSTRENGTH ) )*
         ( QUALITY_INDICATOR_FULL ) 
         );
   }
   else
   {
      u8fieldStrength = QUALITY_INDICATOR_FULL;
   }
   return (ulword)u8fieldStrength;
}


/**
* Method: vGetServiceModeQualityString
* Returns the service quality as string
* B2
*/
void clHSA_TestMode::vGetServiceModeQualityString(GUI_String *out_result,ulword Type)
{	
   /** Allocate memory dynamically for the 
   Buffer to hold the formatted Strings */
   tChar* tCharpacText = NULL;
   tCharpacText = OSAL_NEW tChar[TESTMODE_DLR_GPS_STRLEN];
   if(tCharpacText != NULL)
      OSAL_pvMemorySet((tVoid*)tCharpacText,NULL,TESTMODE_DLR_GPS_STRLEN);

   tU8 u8SignalQuality = 0;

   switch(Type)
   {
   case TUNER_0: 
   case TUNER_1:
      {
         if( NULL != m_poDataPool)
         {
            m_poDataPool->u32dp_get( DPTUNER__TESTMODE_QUALITY_TUNER_0,
               &u8SignalQuality, 
               sizeof(u8SignalQuality)	);
         }


         if( u8SignalQuality <= SERVICE_MODE_POOR_QUALITY )
         {
            UTF8_s32SaveNPrintFormat( tCharpacText, TESTMODE_DLR_GPS_STRLEN, "%s", "Poor");
         }
         else if((u8SignalQuality > SERVICE_MODE_POOR_QUALITY) && (u8SignalQuality <= SERVICE_MODE_WEAK_QUALITY))
         {
            UTF8_s32SaveNPrintFormat( tCharpacText, TESTMODE_DLR_GPS_STRLEN, "%s", "Mono");
         }
         else
         {
            UTF8_s32SaveNPrintFormat( tCharpacText, TESTMODE_DLR_GPS_STRLEN, "%s", "Stereo");
         }

         GUI_String_vSetCStr(out_result,(tU8*)tCharpacText);

         break;
      }
   default:
      {
         // Nothing to do Here
         break;
      }
   } //End of switch

   /** Release the memory Allocated dynamically */
   OSAL_DELETE[] tCharpacText;

}

/**
* Method: ulwGetTunerRDSErrorRateService
* Returns RDS error rate
* B2
*/
ulword clHSA_TestMode::ulwGetTunerRDSErrorRateService(ulword Tuner)
{
   tU8 u8RDSErrorRate = 0;
   if( NULL != m_poDataPool)
   {
      switch(Tuner)
      {
      case TUNER_0: 
         {

            m_poDataPool->u32dp_get( DPTUNER__TESTMODE_RDSERRORS_TUNER_0, &u8RDSErrorRate, sizeof(u8RDSErrorRate));


            break;
         }
      case TUNER_1:
         {

            m_poDataPool->u32dp_get( DPTUNER__TESTMODE_RDSERRORS_TUNER_1, &u8RDSErrorRate, sizeof(u8RDSErrorRate));

            break;
         }
      case TUNER_2:
         {

            m_poDataPool->u32dp_get( DPTUNER__TESTMODE_RDSERRORS_TUNER_2, &u8RDSErrorRate, sizeof(u8RDSErrorRate));

            break;
         }
      case TUNER_3:
         {

            m_poDataPool->u32dp_get( DPTUNER__TESTMODE_RDSERRORS_TUNER_3, &u8RDSErrorRate, sizeof(u8RDSErrorRate));

            break;
         }
      default:
         {
            // Nothing to do here
            break;
         }
      }/* end of switch */
   }

   return (ulword)u8RDSErrorRate;
}

/**
* Method: blIsAntenna2Available
* Returns true if two antenna are installed in the car. In Skoda Variante only one antenna exists.
* B1
*/
tbool clHSA_TestMode::blIsAntenna2Available()
{
   tU8 u8Tunermode = 0;
   tBool bAntenna2Available = FALSE;

   if( NULL != m_poDataPool )
   {		
      m_poDataPool->u32dp_get( DPTUNER__TESTMODE_MODE, 
         &u8Tunermode, 
         sizeof(u8Tunermode)		);
   }

   if( FMMONITOR_MODE_SINGLE == u8Tunermode )
   {
      bAntenna2Available = FALSE;
   }
   else
   {
      bAntenna2Available = TRUE;
   }

   return bAntenna2Available;
}

/**
* Method: vGetTunerAntenna1
* Returns tuner antenna 1
* B1
*/
void clHSA_TestMode::vGetTunerAntenna1(GUI_String *out_result,ulword Tuner)
{
   tBool bTunerAntenna1 = FALSE;

   Tuner = Tuner; // To avoid Lint warning. 

   /** Allocate memory dynamically for the 
   Buffer to hold the formatted Strings */
   tChar* tCharpacText = NULL;
   tCharpacText = OSAL_NEW tChar[TESTMODE_DLR_GPS_STRLEN];
   if(tCharpacText != NULL)
      OSAL_pvMemorySet((tVoid*)tCharpacText,NULL,TESTMODE_DLR_GPS_STRLEN);

   if( NULL != m_poDataPool )
   {		
      m_poDataPool->u32dp_get( DPDVPTSM__STATUS_ANTENNA1, 
         &bTunerAntenna1, 
         sizeof(bTunerAntenna1)		);
   }

   if ( TRUE == bTunerAntenna1 )
   {
      UTF8_s32SaveNPrintFormat( tCharpacText, 
         TESTMODE_DLR_GPS_STRLEN, 
         "%s",
         "ON"	);
   }
   else
   {
      UTF8_s32SaveNPrintFormat( tCharpacText, 
         TESTMODE_DLR_GPS_STRLEN, 
         "%s",
         "OFF"	);
   }
   GUI_String_vSetCStr(out_result,(tU8*)tCharpacText);

   /** Release the memory Allocated dynamically */
   OSAL_DELETE[] tCharpacText;

}

ulword clHSA_TestMode::ulwGetTunerAntennaOne()
{
   tBool bTunerAntennaOne = FALSE;

   if( NULL != m_poDataPool )
   {		
      m_poDataPool->u32dp_get( DPDVPTSM__STATUS_ANTENNA1, 
         &bTunerAntennaOne, 
         sizeof(bTunerAntennaOne)		);
   }

   return bTunerAntennaOne;

}

/**
* Method: vGetTunerAntenna2
* Returns tuner antenna 2
* B1
*/
void clHSA_TestMode::vGetTunerAntenna2(GUI_String *out_result,ulword Tuner)
{
   tBool bTunerAntenna2 = FALSE;

   Tuner = Tuner; // To avoid Lint warning. 

   /** Allocate memory dynamically for the 
   Buffer to hold the formatted Strings */
   tChar* tCharpacText = NULL;
   tCharpacText = OSAL_NEW tChar[TESTMODE_DLR_GPS_STRLEN];
   if(tCharpacText != NULL)
      OSAL_pvMemorySet((tVoid*)tCharpacText,NULL,TESTMODE_DLR_GPS_STRLEN);

   if( NULL != m_poDataPool )
   {		
      m_poDataPool->u32dp_get( DPDVPTSM__STATUS_ANTENNA2, 
         &bTunerAntenna2, 
         sizeof(bTunerAntenna2)		);
   }

   if ( TRUE == bTunerAntenna2 )
   {
      UTF8_s32SaveNPrintFormat( tCharpacText, 
         TESTMODE_DLR_GPS_STRLEN, 
         "%s",
         "ON"	);
   }
   else
   {
      UTF8_s32SaveNPrintFormat( tCharpacText, 
         TESTMODE_DLR_GPS_STRLEN, 
         "%s",
         "OFF"	);
   }
   GUI_String_vSetCStr(out_result,(tU8*)tCharpacText);

   /** Release the memory Allocated dynamically */
   OSAL_DELETE[] tCharpacText;

}

/**
* Method: vGetAngle
* returns the steering wheel angle
* B2
*/
void clHSA_TestMode::vGetAngle(GUI_String *out_result)
{
   tU32 u32GyroADCVal = 0;

   tChar* tCharpacText = NULL;
   tCharpacText = OSAL_NEW tChar[TESTMODE_DLR_GPS_STRLEN];
   if(tCharpacText != NULL)
   {
      OSAL_pvMemorySet((tVoid*)tCharpacText,NULL,TESTMODE_DLR_GPS_STRLEN);

      if( NULL != m_poDataPool )
      {
         m_poDataPool->u32dp_get( DPVDS__GYRO_3DGYRO_RVALUE,
            &u32GyroADCVal,
            sizeof(u32GyroADCVal));
      }

      /** Convert Wheel Angle Info into Buffer for formatting */
      UTF8_s32SaveNPrintFormat(tCharpacText,
         TESTMODE_DLR_GPS_STRLEN,
         "%d",
         (u32GyroADCVal));

      GUI_String_vSetCStr(out_result, (tU8*)tCharpacText);
   }
   OSAL_DELETE[] tCharpacText;
}

/**
* Method: vGetWheel1RPM
* returns RPM of wheel 1
* B2
*/
void clHSA_TestMode::vGetWheel1RPM(GUI_String *out_result)
{
   tU16 u16WheelRPM = 0;

   /** Allocate memory dynamically for the 
   Buffer to hold the formatted Strings */
   tChar* tCharpacText = NULL;
   tCharpacText = OSAL_NEW tChar[TESTMODE_DLR_GPS_STRLEN];
   if(tCharpacText != NULL)
      OSAL_pvMemorySet((tVoid*)tCharpacText,NULL,TESTMODE_DLR_GPS_STRLEN);

#ifdef DPVDS__ABSDATA_WHEEL1_SPEED
   if( NULL != m_poDataPool )
   {
      m_poDataPool->u32dp_get( DPVDS__ABSDATA_WHEEL1_SPEED, 
         &u16WheelRPM, 
         sizeof(u16WheelRPM)		);
   }
#endif

   /** Get the UTF8 formatted String into the Buffer to Display */
   UTF8_s32SaveNPrintFormat( tCharpacText,
      TESTMODE_DLR_GPS_STRLEN,
      "%d RPM", 
      u16WheelRPM  );

   GUI_String_vSetCStr(out_result,(tU8*)tCharpacText);

   /** Release the memory Allocated dynamically */
   OSAL_DELETE[] tCharpacText;

}
/**
* Method: vGetWheel2RPM
* returns RPM of wheel 2
* B2
*/
void clHSA_TestMode::vGetWheel2RPM(GUI_String *out_result)
{
   tU16 u16WheelRPM = 0;

   /** Allocate memory dynamically for the 
   Buffer to hold the formatted Strings */
   tChar* tCharpacText = NULL;
   tCharpacText = OSAL_NEW tChar[TESTMODE_DLR_GPS_STRLEN];
   if(tCharpacText != NULL)
      OSAL_pvMemorySet((tVoid*)tCharpacText,NULL,TESTMODE_DLR_GPS_STRLEN);

#ifdef DPVDS__ABSDATA_WHEEL2_SPEED
   if( NULL != m_poDataPool )
   {
      m_poDataPool->u32dp_get( DPVDS__ABSDATA_WHEEL2_SPEED, 
         &u16WheelRPM, 
         sizeof(u16WheelRPM)		);
   }
#endif

   /** Get the UTF8 formatted String into the Buffer to Display */
   UTF8_s32SaveNPrintFormat( tCharpacText,
      TESTMODE_DLR_GPS_STRLEN,
      "%d RPM", 
      u16WheelRPM  );

   GUI_String_vSetCStr(out_result,(tU8*)tCharpacText);

   /** Release the memory Allocated dynamically */
   OSAL_DELETE[] tCharpacText;

}
/**
* Method: vGetWheel3RPM
* returns RPM of wheel 3
* B2
*/
void clHSA_TestMode::vGetWheel3RPM(GUI_String *out_result)
{
   tU16 u16WheelRPM = 0;

   /** Allocate memory dynamically for the 
   Buffer to hold the formatted Strings */
   tChar* tCharpacText = NULL;
   tCharpacText = OSAL_NEW tChar[TESTMODE_DLR_GPS_STRLEN];
   if(tCharpacText != NULL)
      OSAL_pvMemorySet((tVoid*)tCharpacText,NULL,TESTMODE_DLR_GPS_STRLEN);

#ifdef DPVDS__ABSDATA_WHEEL3_SPEED
   if( NULL != m_poDataPool )
   {
      m_poDataPool->u32dp_get( DPVDS__ABSDATA_WHEEL3_SPEED, 
         &u16WheelRPM, 
         sizeof(u16WheelRPM)		);
   }
#endif

   /** Get the UTF8 formatted String into the Buffer to Display */
   UTF8_s32SaveNPrintFormat( tCharpacText,
      TESTMODE_DLR_GPS_STRLEN,
      "%d RPM", 
      u16WheelRPM  );

   GUI_String_vSetCStr(out_result,(tU8*)tCharpacText);

   /** Release the memory Allocated dynamically */
   OSAL_DELETE[] tCharpacText;

}
/**
* Method: vGetWheel4RPM
* returns RPM of wheel 4
* B2
*/
void clHSA_TestMode::vGetWheel4RPM(GUI_String *out_result)
{
   tU16 u16WheelRPM = 0;

   /** Allocate memory dynamically for the 
   Buffer to hold the formatted Strings */
   tChar* tCharpacText = NULL;
   tCharpacText = OSAL_NEW tChar[TESTMODE_DLR_GPS_STRLEN];
   if(tCharpacText != NULL)
      OSAL_pvMemorySet((tVoid*)tCharpacText,NULL,TESTMODE_DLR_GPS_STRLEN);

#ifdef DPVDS__ABSDATA_WHEEL4_SPEED
   if( NULL != m_poDataPool )
   {
      m_poDataPool->u32dp_get( DPVDS__ABSDATA_WHEEL4_SPEED, 
         &u16WheelRPM, 
         sizeof(u16WheelRPM)		);
   }
#endif

   /** Get the UTF8 formatted String into the Buffer to Display */
   UTF8_s32SaveNPrintFormat( tCharpacText,
      TESTMODE_DLR_GPS_STRLEN,
      "%d RPM", 
      u16WheelRPM  );

   GUI_String_vSetCStr(out_result,(tU8*)tCharpacText);

   /** Release the memory Allocated dynamically */
   OSAL_DELETE[] tCharpacText;

}

/**
* Method: ulwGetBestSatellite
* Returns an integer for the specified GPS satellite
* B1
*/
ulword clHSA_TestMode::ulwGetBestSatellite(ulword Satellite)
{
   tU32 u32BestSatelliteValue = 0;
   tU32 u32DataPoolID = NULL;
   switch(Satellite)
   {
   case BEST_SATELLITE_ONE:
      {
         u32DataPoolID = DPVDS__GPS_DATA_INFO_BESTSATELLITES_ONE;
         break;
      }

   case BEST_SATELLITE_TWO:
      {
         u32DataPoolID = DPVDS__GPS_DATA_INFO_BESTSATELLITES_TWO;
         break;
      }

   case BEST_SATELLITE_THREE:
      {
         u32DataPoolID = DPVDS__GPS_DATA_INFO_BESTSATELLITES_THREE;
         break;
      }

   case BEST_SATELLITE_FOUR:
      {
         u32DataPoolID = DPVDS__GPS_DATA_INFO_BESTSATELLITES_FOUR;
         break;
      }
   default:
      {
         u32DataPoolID = NULL;
         break;
      }
   }

   if( (NULL != m_poDataPool) && ( 0 != u32DataPoolID) )
   {
      m_poDataPool->u32dp_get( u32DataPoolID, 
         &u32BestSatelliteValue, 
         sizeof(u32BestSatelliteValue)		);
   }

   return (ulword)u32BestSatelliteValue;

}

/**
* Method: blGetSDTrace
* Returns if SD Trace is activated or not.
* B
*/
tbool clHSA_TestMode::blGetSDTrace()
{
   tBool bSDTraceStatus = FALSE;  //Variable to read the datapool element.

   if(m_poDataPool)
   {
      /* false=SD Trace inactice, true=SD Trace active */
      m_poDataPool->u32dp_get( DPOSAL__SDCARD_TTFIS_SINK_CONFIGURATION, 
         &bSDTraceStatus, 
         sizeof(bSDTraceStatus)
         );						
   }

   /** Trace is here */	
   if(m_poTrace)
   {	
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
         TR_CLASS_HMI_HSA_SYSTEM_TESTMODE,
         (tU16)(HSA_API_RAYALU__GET_SD_TRACE_STATUS),
         1,
         &bSDTraceStatus );
   }

   return bSDTraceStatus;
}

/**
* Method: vToggleSDTrace
* Toggles the SD Trace value between true and false.
* B
*/
void clHSA_TestMode::vToggleSDTrace()
{
   tBool bToggleSDTrace = FALSE; // Variable holds the value of SDTrace.
   /** Event sent to HSI to set SD Card trace value */
   tU32 u32EventToSend = HSI__OSAL__CONFIGURE_SDCARD_TTFIS_SINK; 

   /** Reading SDTrace value from Datapool element and toggle it */
   if( m_poDataPool )
   {
      m_poDataPool->u32dp_get( DPOSAL__SDCARD_TTFIS_SINK_CONFIGURATION, 
         &bToggleSDTrace, 
         sizeof(bToggleSDTrace)	);
   }
   if( TRUE == bToggleSDTrace )
   {
      bToggleSDTrace = FALSE; /** Toggling the SDTrace value */
   }
   else
   {
      bToggleSDTrace = TRUE;  /** Toggling the SDTrace value */
   }

   /** Setting the toggled value to Datapool */
   if( m_poDataPool )
   {
      m_poDataPool->u32dp_set( DPOSAL__SDCARD_TTFIS_SINK_CONFIGURATION, 
         &bToggleSDTrace, 
         sizeof(bToggleSDTrace)	);
   }

   /** After toggling the value of SDTrace, send the event to HSI */
   vSendCommandtoHSI(u32EventToSend);	

   if(m_poTrace)
   {
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
         TR_CLASS_HMI_HSA_SYSTEM_TESTMODE,
         (tU16)(HSA_API_RAYALU__TOGGLE_SD_TRACE),
         4,
         &u32EventToSend );
   }
}

/**
* Method: vGetTMCLastMsgTimeDate
* Returns the time and date of the last TMC message
* B
*/
void clHSA_TestMode::vGetTMCLastMsgTimeDate(GUI_String *out_result)
{
	//bpstl::string oReadBuffer = "";
	//dp.vGetString( DPNAVI__TPEG_TEC_LASTMSG_DATETIME, oReadBuffer);
	//GUI_String_vSetCStr(out_result, (tU8*) const_cast<tChar*>(oReadBuffer.c_str()));
   /** Character Buffer to Hold the Formatted String */ 
	tChar* tCharpacTimeDate = NULL;

   tU8 u8Hour=0;
   tU8 u8Mins=0;
   tU8 u8Day=0;
   tU8 u8Month=0;
   tS16 s16Year=0; 

   /** Allocating the memory to Buffer */
   tCharpacTimeDate = OSAL_NEW tChar[MAX_BUFFER_STRLEN];

   /** Initializing the char buffer to NULL */
   if(tCharpacTimeDate != NULL)
      OSAL_pvMemorySet((tVoid*)tCharpacTimeDate,NULL,MAX_BUFFER_STRLEN);	

  
   		
	
   /** To send Command to HSI we need to use the Component Manager
   Get the pointer for TestMode Component Manager Object */

   if( NULL != pclHSI_System)
   {
      if(m_poDataPool) 
      {
         m_poDataPool->u32dp_get( DPTUNER__TMC_LAST_MSG_TIME_HOUR, //Reading the datapool element
            &u8Hour, 
            sizeof(u8Hour) );

         m_poDataPool->u32dp_get( DPTUNER__TMC_LAST_MSG_TIME_MIN, //Reading the datapool element
            &u8Mins, 
            sizeof(u8Mins) );

         m_poDataPool->u32dp_get( DPTUNER__TMC_LAST_MSG_DATE_DAY, //Reading the datapool element
            &u8Day, 
            sizeof(u8Day) );

         m_poDataPool->u32dp_get( DPTUNER__TMC_LAST_MSG_DATE_MONTH, //Reading the datapool element
            &u8Month, 
            sizeof(u8Month) );

         m_poDataPool->u32dp_get( DPTUNER__TMC_LAST_MSG_DATE_YEAR, //Reading the datapool element
            &s16Year, 
            sizeof(s16Year) );
      }
      sTime_Info_For_Format sTime;
      sTime.u8Hour = u8Hour;
      sTime.u8Mins = u8Mins;

      sDate_Info_For_Format sDate;
      sDate.u8Day          =         u8Day;
      sDate.u8Month        =         u8Month;
      sDate.s16Year        =         s16Year;

      if( DATE_FORMAT_SUCCESS != pclHSI_System->u8FormatDateTimetoDisplay(sDate,sTime,(tUTF8 *)tCharpacTimeDate,MAX_BUFFER_STRLEN) )
      {
         // Set all bytes to NULL again.. And put a trace with error condition...
         if(tCharpacTimeDate != NULL)
            OSAL_pvMemorySet((tVoid*)tCharpacTimeDate,NULL,MAX_BUFFER_STRLEN);
      }
      else
      {
         // Nothing to do here...
      }
   }  
    
   /** Return Time String*/
   GUI_String_vSetCStr(out_result, (tU8*)tCharpacTimeDate);

   /** Trace */
   if(m_poTrace)
   {
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
         TR_CLASS_HMI_HSA_SYSTEM_TESTMODE,
         (tU16)(HSA_API_RAYALU__GET_TMC_LAST_MSG_TIME_DATE),
         MAX_BUFFER_STRLEN,
         (tU8*)tCharpacTimeDate );
   }

   OSAL_DELETE[] tCharpacTimeDate;
}


/**
* Method: vGetTMCNoOfMsgs
* Returns the number of TMC messages
* B
*/
void clHSA_TestMode::vGetTMCNoOfMsgs(GUI_String *out_result)
{
   tU16 u16TotalNoOfMsgs = 0;	//Variable to hold the No of messages value.

   /** Character Buffer to Hold the Formatted String */ 
   tChar* tCharpacText = NULL;
   tCharpacText = OSAL_NEW tChar[MAX_BUFFER_STRLEN];
   if(tCharpacText != NULL)
      OSAL_pvMemorySet((tVoid*)tCharpacText,NULL,MAX_BUFFER_STRLEN);	

   if(m_poDataPool) 
   {
      m_poDataPool->u32dp_get( DPNAVI__TMC_NO_OF_MSGS, //Reading the datapool element
         &u16TotalNoOfMsgs, 
         sizeof(u16TotalNoOfMsgs) );
   }

   UTF8_s32SaveNPrintFormat( tCharpacText,MAX_BUFFER_STRLEN,"%d",u16TotalNoOfMsgs);

   /** Return Time String*/	
   GUI_String_vSetCStr(out_result, (tU8*)tCharpacText);

   if(m_poTrace)
   {	
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
         TR_CLASS_HMI_HSA_SYSTEM_TESTMODE,
         (tU16)(HSA_API_RAYALU__GET_TMC_NO_OF_MSGS),
         MAX_BUFFER_STRLEN,
         (tU8*)tCharpacText );
   }

   /** Releasing memory */
   OSAL_DELETE[] tCharpacText;
}


/**
* Method: vGetTMCNoOfMsgsSelectionArea
* Returns the number of TMC messages in selection area
* B
*/
void clHSA_TestMode::vGetTMCNoOfMsgsSelectionArea(GUI_String *out_result)
{
   tU16 u16TMCNoOfMsgsSelArea = 0;	//Variable to hold the No of messages in Selection Area value.

   /** Character Buffer to Hold the Formatted String */ 
   tChar* tCharpacText = NULL;
   tCharpacText = OSAL_NEW tChar[MAX_BUFFER_STRLEN];
   if(tCharpacText != NULL)
      OSAL_pvMemorySet((tVoid*)tCharpacText,NULL,MAX_BUFFER_STRLEN);	

   if(m_poDataPool) 
   {
      m_poDataPool->u32dp_get( DPNAVI__TMC_NO_OF_MSGS_SEL_AREA, //Reading the datapool element
         &u16TMCNoOfMsgsSelArea, 
         sizeof(u16TMCNoOfMsgsSelArea) );
   }

   UTF8_s32SaveNPrintFormat( tCharpacText,MAX_BUFFER_STRLEN,"%d",u16TMCNoOfMsgsSelArea);

   /** Return Time String*/	
   GUI_String_vSetCStr(out_result, (tU8*)tCharpacText);

   if(m_poTrace)
   {	
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
         TR_CLASS_HMI_HSA_SYSTEM_TESTMODE,
         (tU16)(HSA_API_RAYALU__GET_TMC_NO_OF_MSGS_SEL_AREA),
         MAX_BUFFER_STRLEN,
         (tU8*)tCharpacText );
   }

   /** Releasing memory */
   OSAL_DELETE[] tCharpacText;
}

/**
* Method: vGetCogCount
* Returns the cog count
* B
*/
void clHSA_TestMode::vGetCogCount(GUI_String *out_result)
{
   tU32 u32CogCount = 0;	//Variable to hold the datapool element.

   /** Character Buffer to Hold the Formatted String */ 
   tChar* tCharpacText = NULL;
   tCharpacText = OSAL_NEW tChar[MAX_BUFFER_STRLEN];
   if(tCharpacText != NULL)
      OSAL_pvMemorySet((tVoid*)tCharpacText,NULL,MAX_BUFFER_STRLEN);	

   if(m_poDataPool) 
   {
      m_poDataPool->u32dp_get( DPVDS__ODOMETER_PULSE_PER_TYRE_CIRCUMFERENCE, //Reading the datapool element
         &u32CogCount, 
         sizeof(u32CogCount) );
   }

   //Dividing the value by '1000' for display.
   u32CogCount = (u32CogCount/CONVERT_MM_TO_METER);
   UTF8_s32SaveNPrintFormat( tCharpacText,MAX_BUFFER_STRLEN,"%d",u32CogCount);

   /** Return Time String*/	
   GUI_String_vSetCStr(out_result, (tU8*)tCharpacText);

   if(m_poTrace)
   {	
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
         TR_CLASS_HMI_HSA_SYSTEM_TESTMODE,
         (tU16)(HSA_API_RAYALU__GET_COG_COUNT),
         MAX_BUFFER_STRLEN,
         (tU8*)tCharpacText );
   }

   /** Releasing memory */
   OSAL_DELETE[] tCharpacText;
}


/**
* Method: vGetWheelCircumference
* Returns the  wheel circumference (odometer via line in pulse/meter)
* B
*/
void clHSA_TestMode::vGetWheelCircumference(GUI_String *out_result)
{
   tU32 u32WheelCircumference = 0;	//Variable to hold the datapool element.

   /** Character Buffer to Hold the Formatted String */ 
   tChar* tCharpacText = NULL;
   tCharpacText = OSAL_NEW tChar[MAX_BUFFER_STRLEN];
   if(tCharpacText != NULL)
      OSAL_pvMemorySet((tVoid*)tCharpacText,NULL,MAX_BUFFER_STRLEN);	

   if(m_poDataPool) 
   {
      m_poDataPool->u32dp_get( DPVDS__ODOMETER_DISTANCE_SCALE_FACTOR, //Reading the datapool element
         &u32WheelCircumference, 
         sizeof(u32WheelCircumference) );
   }

   UTF8_s32SaveNPrintFormat( tCharpacText,MAX_BUFFER_STRLEN,"%d",u32WheelCircumference);

   /** Return Time String*/	
   GUI_String_vSetCStr(out_result, (tU8*)tCharpacText);

   if(m_poTrace)
   {	
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
         TR_CLASS_HMI_HSA_SYSTEM_TESTMODE,
         (tU16)(HSA_API_RAYALU__GET_WHEEL_CIRCUMFERENCE),
         MAX_BUFFER_STRLEN,
         (tU8*)tCharpacText );
   }

   /** Releasing memory */
   OSAL_DELETE[] tCharpacText;
}

/**
* Method: ulwGetMediaStatus
* Returns the status for the CD or SD drive
* B
*/
ulword clHSA_TestMode::ulwGetMediaStatus(ulword Drive)
{
   Drive = Drive;  // for Lint

   tU8 u8MediaStatus = 0;
   switch(Drive)
   {
   case MEDIA_CD_STATUS:
      {
         if(NULL != m_poDataPool) 
         {
            m_poDataPool->u32dp_get( DPDVPTSM__CD_MEDIA_TYPE, //Reading the datapool element
               &u8MediaStatus, 
               sizeof(u8MediaStatus) );
         }
         // check whether valid software CD
         if (u8MediaStatus == MEDIA_CD_SOFTWARE_DIFF_VERSION)
         {
            u8MediaStatus = MEDIA_CD_VALID_SOFTWARE;
         }

         break;
      }
   case MEDIA_SD_STATUS:
      {
         if(NULL != m_poDataPool)
         {
            m_poDataPool->u32dp_get( DPDEVMGR__DRIVEINFO_SD_CARD_MEDIA, //Reading the datapool element
               &u8MediaStatus, 
               sizeof(u8MediaStatus) );
         }

         //Mapping HSI Values to GUI values
         if(u8MediaStatus == SRV_SD_CARD_MEDIA_EJECTED)
            u8MediaStatus = STATUS_EJECT;
         else if (u8MediaStatus == SRV_SD_CARD_INCORRECT_MEDIA)
            u8MediaStatus = STATUS_INCORRECT;
         else if (u8MediaStatus == SRV_SD_CARD_DATA_MEDIA)
            u8MediaStatus = STATUS_DATA;
         else
            u8MediaStatus = STATUS_UNKNOWN_SD_CARD;
         break;
      }
   default:
      {
         // Nothing to do here...
         break;
      }
   }
   return u8MediaStatus;
}

/**
* Method: vGetMapInfoValues
* Returns values for the map info of CD or SD
* B1
*/
void clHSA_TestMode::vGetMapInfoValues(GUI_String *out_result,ulword Medium)
{
   /** Character Buffer to Hold the Formatted String of SW Version*/ 
   tChar* tCharpacText = NULL;
   tCharpacText = OSAL_NEW tChar[MAX_BUFFER_STRLEN];
   if (tCharpacText != NULL)
      OSAL_pvMemorySet((tVoid*)tCharpacText,NULL,MAX_BUFFER_STRLEN);

   /** Temperaray Variable to hold the datapool element Adress */
   tU32 u32DatapoolID = 0;

   /** To get the size of Datapool Element */
   tU32 u32Size = 0;

   if(Medium == SD_INFO)
      u32DatapoolID = DPNAVI__SEAMLESS_SD_INFO;

   /** Read the datapool elements */
   if( NULL != m_poDataPool )
   { 
      //Reading SD Version Info
      {
         u32Size   = m_poDataPool->u32dp_getElementSize(u32DatapoolID);

         if( NULL != tCharpacText )
         {
            m_poDataPool->u32dp_get( u32DatapoolID, //Reading the datapool element
               tCharpacText, 
               u32Size );
         }

         UTF8_s32SaveNPrintFormat(tCharpacText,MAX_BUFFER_STRLEN,"MapVersion: %s",tCharpacText);
      }

      /** trace is here */
      if(m_poTrace)
      {
         m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
            TR_CLASS_HMI_HSA_SYSTEM_TESTMODE,"Map Info value = %s", tCharpacText);
      }
   }
   /** Return MAp info String*/	
   GUI_String_vSetCStr(out_result, (tU8*)tCharpacText);

   /** Releasing memory */
   OSAL_DELETE[] tCharpacText;
}


/**
* Method: vSetRadioTestModeActive
* Toggels the radio test mode to enabled or disabled.
* B
*/
void clHSA_TestMode::vSetRadioTestModeActive(ulword enable)
{
   /** Here the interpretation of the Enable is reverse 
   0--Enable TestMode Switch.
   1--Disable the TestMode Switch */


   switch(enable)
   {
   case TESTMODESWITCH_ON:
      {

         /** Send a Message to CM TestMode to remember the Radio testMode Switch is on from API. */
         vSendCommandtoHSI(CMMSG_TESTMODE_RADIO_TESTMODE_ON_API);

         if(m_poTrace)
         {	
            m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
               TR_CLASS_HMI_HSA_SYSTEM_TESTMODE,
               (tU16)(HSA_API_RAYALU__SET_TESTMODE_RADIO_ON));
         }
         break;
      }
   case TESTMODESWITCH_OFF:
      {

         /** Send a Message to CM TestMode to remember the Radio testMode Switch is on from API. */
         vSendCommandtoHSI(CMMSG_TESTMODE_RADIO_TESTMODE_OFF_API);              

         if(m_poTrace)
         {	
            m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
               TR_CLASS_HMI_HSA_SYSTEM_TESTMODE,
               (tU16)(HSA_API_RAYALU__SET_TESTMODE_RADIO_OFF));
         }
         break;
      }
   default:
      {
         //Nothing to do Here... 
         break;
      }
   }
}

/*!***********************************************************************
* METHOD:			ulwGetBTLinkQuality
* CLASS:			clHSA_TestMode
* DESCRIPTION:	Returns the link quality of BT.
* PARAMETER:    
* RETURNVALUE:  	ulword
*************************************************************************/
ulword clHSA_TestMode::ulwGetBTLinkQuality()
{
   tU8 u8BTLinkQuality = 0;  //Variable to read the datapool element.

   if(m_poDataPool)
   {
#ifdef DPTELEPHONE__BT_LINK_QUALITY
      m_poDataPool->u32dp_get( DPTELEPHONE__BT_LINK_QUALITY, 
         &u8BTLinkQuality, 
         sizeof(u8BTLinkQuality)
         );		
#endif
   }

   /** Trace is here */	
   if(m_poTrace)
   {	
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
         TR_CLASS_HMI_HSA_SYSTEM_TESTMODE,
         (tU16)(HSA_API_RAYALU__GET_BT_LINK_QUALITY),
         1,
         &u8BTLinkQuality );
   }

   return u8BTLinkQuality;
}

/*!***********************************************************************
* METHOD:			ulwGetBTRSSI
* CLASS:			clHSA_TestMode
* DESCRIPTION:	Returns the RSSI value.
* PARAMETER:    
* RETURNVALUE:  	ulword
*************************************************************************/
ulword clHSA_TestMode::ulwGetBTRSSI()
{
   tS8 s8BTRSSI = 0;  //Variable to read the datapool element.

   if(m_poDataPool)
   {
#ifdef DPTELEPHONE__BT_RSSI
      m_poDataPool->u32dp_get( DPTELEPHONE__BT_RSSI, 
         &s8BTRSSI, 
         sizeof(s8BTRSSI)
         );		
#endif
   }

   /** Trace is here */	
   if(m_poTrace)
   {	
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
         TR_CLASS_HMI_HSA_SYSTEM_TESTMODE,
         (tU16)(HSA_API_RAYALU__GET_BTRSSI),
         1,
         (tU8*)&s8BTRSSI);
   }

   return s8BTRSSI;
}

/*!***********************************************************************
* METHOD:			ulwGetBTModuleMode
* CLASS:			clHSA_TestMode
* DESCRIPTION:	Returns the BTModuleMode.
* PARAMETER:    
* RETURNVALUE:  	ulword
*************************************************************************/
ulword clHSA_TestMode::ulwGetBTModuleMode()
{
   tU8 u8BTModuleMode = 0;  //Variable to read the datapool element.

   if(m_poDataPool)
   {
      m_poDataPool->u32dp_get( DPTELEPHONE__BT_TEST_MODE, 
         &u8BTModuleMode, 
         sizeof(u8BTModuleMode)
         );		
   }

   /** Trace is here */	
   if(m_poTrace)
   {	
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
         TR_CLASS_HMI_HSA_SYSTEM_TESTMODE,
         (tU16)(HSA_API__GET_BTMODULE_MODE),
         1,
         &u8BTModuleMode );
   }

   return u8BTModuleMode;
}
/*!***********************************************************************
* METHOD:			vSetBTModuleMode
* CLASS:			clHSA_TestMode
* DESCRIPTION:	Set the BTModuleMode.
* PARAMETER:    
* RETURNVALUE:  	ulword
*************************************************************************/
void clHSA_TestMode::vSetBTModuleMode(ulword BTModuleMode)
{
   tU8 u8BTModuleMode = (tU8)BTModuleMode;  //Variable to read the datapool element.

   if(m_poDataPool)
   {
      m_poDataPool->u32dp_set( DPTELEPHONE__BT_TEST_MODE, 
         &u8BTModuleMode, 
         sizeof(u8BTModuleMode));		
   }

   if( NULL != pclHSI_TestMode)
   {
      sMsg refMsg (NULL, CMMSG_TESTMODE_BLUETOOTH_MODE, NULL, NULL) ;
      pclHSI_TestMode->bExecuteMessage(refMsg);
   }

   /** Trace is here */	
   if(m_poTrace)
   {	
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
         TR_CLASS_HMI_HSA_SYSTEM_TESTMODE,
         (tU16)(HSA_API__SET_BTMODULE_MODE));
   }
}
/*!***********************************************************************
* METHOD:		 ulwBTModuleStatus
* CLASS:			 clHSA_TestMode
* DESCRIPTION:	 Returns the BTModule status
* PARAMETER:    
* RETURNVALUE:  	ulword
*    Description :-
*    0 - OK
*    1 - NOT CONNECTED
*    2 - SWITCHED OFF
*************************************************************************/
ulword clHSA_TestMode::ulwBTModuleStatus()
{
   tU8 u8BTModuleSts = GUI_PHONE_BT_MODULE_STS_SWITCHED_OFF;

   if(m_poDataPool)
   {
      m_poDataPool->u32dp_get( 
         DPTELEPHONE__BT_MODULE_STATUS, 
         &u8BTModuleSts, 
         sizeof(u8BTModuleSts) );
   }

   if(m_poTrace)
   {	
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
         TR_CLASS_HMI_HSA_SYSTEM_TESTMODE,
         (tU16)(HSA_API_BT_MODULE_STATUS),
         1,
         &u8BTModuleSts );
   }

   return u8BTModuleSts;
}

/******************************************************************************
* FUNCTION:    clHSA_TestMode::vCheckSystemVoltage()
*------------------------------------------------------------------------------
* DESCRIPTION: API for getting the voltage value from SPM. Also to trigger the
*              500ms timer
*------------------------------------------------------------------------------
* PARAMETER:   tbool  
* RETURNVALUE: Void
******************************************************************************/
void clHSA_TestMode::vCheckSystemVoltage(tbool Action)
{
   if( NULL != pclHSI_TestMode)
   {
      sMsg refMsg (NULL, CMMSG_TESTMODE_SPM_BATTERY_VOLTAGE, Action, NULL) ;
      pclHSI_TestMode->bExecuteMessage(refMsg);
   }
}

/*!***********************************************************************
* METHOD:			vGetSystemVoltage
* CLASS:			clHSA_TestMode
* DESCRIPTION:	Returns the SystemVoltage.
* PARAMETER:    
* RETURNVALUE:  	void
*************************************************************************/
void clHSA_TestMode::vGetSystemVoltage(GUI_String *out_result)
{
   tU16 u16SystemVoltage = 0;  //Variable to read the datapool element.

   tChar* tCharpacText = NULL;
   tCharpacText = OSAL_NEW tChar[MAX_BUFFER_STRLEN];
   if(tCharpacText != NULL)
      OSAL_pvMemorySet((tVoid*)tCharpacText,NULL,MAX_BUFFER_STRLEN);

   /*if( NULL != pclHSI_TestMode)
   {
   sMsg refMsg (NULL, CMMSG_TESTMODE_SPM_BATTERY_VOLTAGE, NULL, NULL) ;
   pclHSI_TestMode->bExecuteMessage(refMsg);
   }*/


   if(m_poDataPool)
   {
      m_poDataPool->u32dp_get( DPSPM__BATTERY_VOLTAGE, 
         &u16SystemVoltage, 
         sizeof(u16SystemVoltage)
         );		
   }

   UTF8_s32SaveNPrintFormat( tCharpacText,MAX_BUFFER_STRLEN,"%0.2fV",((tF32)u16SystemVoltage/100));

   /** Return Time String*/	
   GUI_String_vSetCStr(out_result, (tU16*)tCharpacText);

   if(m_poTrace)
   {	
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
         TR_CLASS_HMI_HSA_SYSTEM_TESTMODE,
         (tU16)(HSA_API__GET_SYSTEM_VOLTAGE),
         MAX_BUFFER_STRLEN,
         (tU8*)tCharpacText );
      OSAL_DELETE []tCharpacText;
   }  
}
/*!***********************************************************************
* METHOD:			ulwGetTroubleCodeListCount
* CLASS:			clHSA_TestMode
* DESCRIPTION:	Returns the TroubleCode List count.
* PARAMETER:    
* RETURNVALUE:  	ulword
*************************************************************************/
ulword clHSA_TestMode::ulwGetTroubleCodeListCount()
{
   tU8 u8TroubleCodeListCount = 0;

   if(NULL != pclHSI_TestMode)
   {
      u8TroubleCodeListCount = pclHSI_TestMode->u8GetTroubleCodeListCount();
   }
   if(m_poTrace)
   {	
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
         TR_CLASS_HMI_HSA_SYSTEM_TESTMODE,
         (tU16)(HSA_API__GET_TROUBLE_CODE_COUNT),
         1,
         &u8TroubleCodeListCount);
   }  

   return u8TroubleCodeListCount;

}
ulword clHSA_TestMode::ulwGetSrvTroubleCodeListElement(ulword Index)
{
   tU8 u8TroubleCodeIndex = (tU8)Index;
   tU32 u32TroubleCode = 0;

   if(NULL != pclHSI_TestMode)
   {
      if (u8TroubleCodeIndex < pclHSI_TestMode->u8GetTroubleCodeListCount())
      {
         u32TroubleCode = pclHSI_TestMode->u32ReadTroubleCodeListElement(u8TroubleCodeIndex);			

         if(m_poTrace)
         {	
            m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
               TR_CLASS_HMI_HSA_SYSTEM_TESTMODE,"Trouble Code sent =%d", u32TroubleCode );
         }  
      }
   }

   return u32TroubleCode;
}
/*!***********************************************************************
* METHOD:			vClearTroubleCodeList
* CLASS:			clHSA_TestMode
* DESCRIPTION:	Clears the DTC list.
* PARAMETER:     ulword
* RETURNVALUE:  	void
*************************************************************************/
void clHSA_TestMode::vClearTroubleCodeList(tVoid)
{  
   if( NULL != pclHSI_TestMode)
   {
      sMsg refMsg (NULL, CMMSG_TESTMODE_CLEAR_TROUBLE_CODE_LIST, NULL, NULL) ;
      pclHSI_TestMode->bExecuteMessage(refMsg);      
   }

   if(m_poTrace)
   {	
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
         TR_CLASS_HMI_HSA_SYSTEM_TESTMODE,
         (tU16)(HSA_API__CLEAR_TROUBLE_CODE_LIST));
   }  
   return;
}
ulword clHSA_TestMode::ulwGetSrvSystemStatus(ulword SystemStatus)
{
   tU8 u8SystemStatus = (tU8)SystemStatus;
   tU32 u32RetVal = 0;

   if(m_poDataPool)
   {
      switch(u8SystemStatus)
      {
      case SRV_SD_CARD_SLOT_ACCESS_STATUS:
         {
            tBool bState = FALSE;

            m_poDataPool->u32dp_get( DPDEVMGR__DRIVEINFO_SD_CARD_STATE,
               &bState,
               sizeof(bState)
               );		
            u32RetVal = (tU32)bState;
            break;
         }    
      case SRV_SPEED_SIGNAL:
         { 
            tU32 u32Speed = 0;

            m_poDataPool->u32dp_get( DPVDS__GALA_SPEED, 
               &u32Speed, 
               sizeof(u32Speed)
               );		

            u32RetVal = u32Speed;

            break;
         }
      case SRV_DIRECTION_SIGNAL:
         {
            tU8 u8OdometerCount_Direction=0;

            m_poDataPool->u32dp_get( DPVDS__ODOMETER_DIRECTION, 
               &u8OdometerCount_Direction, 
               sizeof(u8OdometerCount_Direction)
               );		

            if(u8OdometerCount_Direction == 1)
            {
               u32RetVal = 0;  
            }
            else if(u8OdometerCount_Direction == 2)
            {
               u32RetVal = 1;
            }
            else
            {
               u32RetVal = 2;
            }
            /*  0=FWD,1=REVERSE, 2=UNKNOWN, */
            break;
         }
      case SRV_ILLUMINATION_SIGNAL:
         {
            tU16 u16Dim_PWM=0; //Holds the value of Dimming_PWM

            /* Reading the value of Dimming_PWM */
               m_poDataPool->u32dp_get( DPDISPLAY__DIMMING_PWM, 
                  &u16Dim_PWM, 
                  sizeof(u16Dim_PWM)
                  );

               /*
               The Min value of u16Dim_PWM is 8% and the Max value is 92%
               Actual value received from middleware is multiplied by ten 
               So, dividing the value by ten 

               If the value received is not within the range, HMI returns the value as it is to GUI
               */
               if( (u16Dim_PWM >= DIM_PWM_MIN_VALUE ) && (u16Dim_PWM <= DIM_PWM_MAX_VALUE) )
               {
                  u32RetVal = u16Dim_PWM/DIVIDE_BY_TEN;
               }
               else
               {
                  u32RetVal = 0;
               }

            break; // MMS Ticket : 235944
         }
      case SRV_GPS_ANTENNA_STATUS:
         {
            tU8 u8AntennaStatus = 0;

            m_poDataPool->u32dp_get( DPVDS__GPS_DATA_INFO_ANTENNA_STATUS, 
               &u8AntennaStatus, 
               sizeof(tU8) );

            if( 0 == u8AntennaStatus )
            {
               u32RetVal = 1;
            }
            else if( 1 == u8AntennaStatus )
            {
               u32RetVal = 2;
            }
            else if( (2 == u8AntennaStatus) || (3 == u8AntennaStatus) || (4 == u8AntennaStatus)	)
            {
               u32RetVal = 3;
            }
            else 
            {
               u32RetVal = 0;
            }
            /* 0 = Unknown, 1 = Connected. 2 = Open, 3 = Short */
            break;
         }
      case SRV_PHONE_MICROPHONE_CURRENT:
         {
            tU16 u16CurrentValue = 0;
            //tS32 s32CurrentValue = 0;

            m_poDataPool->u32dp_get( DPTELEPHONE__DIAG_MICROPHONE_CURRENT_VALUE, 
            &u16CurrentValue,
            sizeof(u16CurrentValue) );

            //Commented to fix NIKAI-2750
            // Calculation for Microphone current
            //s32CurrentValue = (((u16CurrentValue * 30 )/3850) - ((30 * 150)/3850));

            /* Returns the phone current */
            //if(s32CurrentValue < MICROPHONE_CURRENT_MIN_VALUE)
            // {
            //   u32RetVal = MICROPHONE_CURRENT_MIN_VALUE;
            //}
            // else if(s32CurrentValue > MICROPHONE_CURRENT_MAX_VALUE)
            // {
            //   u32RetVal = MICROPHONE_CURRENT_MAX_VALUE;
            //}
            //else
            // {
            u32RetVal = (tU32)u16CurrentValue;
            // }
            break;
         }
      case SRV_RADIO_ANTENNA_STATUS:
         {
            tU8 u8RadioAntennaStatus = 0;

            sMsg refMsg (NULL, (tU32)CMMSG_SYSTEM__RADIO_ANTENNA_STATUS, NULL, NULL) ;

            if ( pclHSI_System != NULL )
            {
               pclHSI_System->bExecuteMessage(refMsg);
            }     

            m_poDataPool->u32dp_get( DPTUNER__TESTMODE_RADIO_ANTENNA_STATUS, 
               &u8RadioAntennaStatus, 
               sizeof(u8RadioAntennaStatus) );

            switch(u8RadioAntennaStatus)
            {
            case TUNMSTR_ANTDIAG_ANTENNA_STATE_HMI_OK:
               u32RetVal = EN_ANTENNA_OK_GUI;
               break;
            case TUNMSTR_ANTDIAG_ANTENNA_STATE_HMI_SHORT:
               u32RetVal = EN_ANTENNA_SHORT_GUI;
               break;
            case TUNMSTR_ANTDIAG_ANTENNA_STATE_HMI_OPEN:
               u32RetVal = EN_ANTENNA_OPEN_GUI;
               break;
            case TUNMSTR_ANTDIAG_ANTENNA_STATE_HMI_DEACTIVATED_LOW_VG:
            case TUNMSTR_ANTDIAG_ANTENNA_STATE_HMI_DEACTIVATED_HIGH_VG:
               u32RetVal = EN_ANTENNA_DEACTIVATED_GUI;
               break;
            case TUNMSTR_ANTDIAG_ANTENNA_STATE_HMI_PASSIVE:
               u32RetVal = EN_ANTENNA_PASSIVE_GUI;
               break;
            case TUNMSTR_ANTDIAG_ANTENNA_STATE_HMI_UNKNOWN:
               u32RetVal = EN_ANTENNA_UNKNOWN_GUI;
               break;
            default:
               break;
            }
            break;
         }
      case SRV_RADIO_DAB_ANTENNA_STATUS:
         {
            tU8 u8RadioDABAntennaStatus = 0;

            sMsg refMsg (NULL, (tU32)CMMSG_SYSTEM__RADIO_DAB_ANTENNA_STATUS, NULL, NULL) ;

            if ( pclHSI_System != NULL )
            {
               pclHSI_System->bExecuteMessage(refMsg);
            }     

            m_poDataPool->u32dp_get( DPTUNER__TESTMODE_RADIO_DAB_ANTENNA_STATUS, 
               &u8RadioDABAntennaStatus, 
               sizeof(u8RadioDABAntennaStatus) );

            switch(u8RadioDABAntennaStatus)
            {
            case TUNMSTR_ANTDIAG_ANTENNA_STATE_HMI_OK:
               u32RetVal = EN_ANTENNA_OK_GUI;
               break;
            case TUNMSTR_ANTDIAG_ANTENNA_STATE_HMI_SHORT:
               u32RetVal = EN_ANTENNA_SHORT_GUI;
               break;
            case TUNMSTR_ANTDIAG_ANTENNA_STATE_HMI_OPEN:
               u32RetVal = EN_ANTENNA_OPEN_GUI;
               break;
            case TUNMSTR_ANTDIAG_ANTENNA_STATE_HMI_DEACTIVATED_LOW_VG:
            case TUNMSTR_ANTDIAG_ANTENNA_STATE_HMI_DEACTIVATED_HIGH_VG:
               u32RetVal = EN_ANTENNA_DEACTIVATED_GUI;
               break;
            case TUNMSTR_ANTDIAG_ANTENNA_STATE_HMI_PASSIVE:
               u32RetVal = EN_ANTENNA_DISCONNECTED_GUI;
               break;
            default:
               break;
            }
            if(u32RetVal > 0)	//work-around to send values to GUI starting from 0
               u32RetVal--;
            break;
         }
      default:
         {
            //nothing to do here
            break;
         }

      }
   }

   if(m_poTrace)
   {	
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
         TR_CLASS_HMI_HSA_SYSTEM_TESTMODE,"Return value = %d",  u32RetVal);
   }  
   return u32RetVal;
}
/*!***********************************************************************
* METHOD:			ulwGetUsbDeviceSrvStatus
* CLASS:			clHSA_TestMode
* DESCRIPTION:		Returns the status of USB Device.
* PARAMETER:		void
* RETURNVALUE:  	ulword
*************************************************************************/
ulword clHSA_TestMode::ulwGetUsbDeviceSrvStatus()
{
   tU8 u8USBMediaStatus = 0;
   tU8 u8DeviceSrvStatus = DEV_ERROR;

   m_poDataPool->u32dp_get( DPDEVMGR__DRIVEINFO_USB_DEVICE_CONNECT_STATUS,
      &u8USBMediaStatus, 
      sizeof(u8USBMediaStatus)
      );	

   if((u8USBMediaStatus == (tU8)(midw_fi_tcl_e8_DeviceConnectStatus::FI_EN_USB_DEV_REMOVED_BY_USR))||(u8USBMediaStatus == (tU8)(midw_fi_tcl_e8_DeviceConnectStatus::FI_EN_USB_DEV_UNDEFINED)))
   {
      u8DeviceSrvStatus = REMOVED;
   }
   else if (u8USBMediaStatus == (tU8)(midw_fi_tcl_e8_DeviceConnectStatus::FI_EN_USB_DEV_CONNECTED))
   {
      u8DeviceSrvStatus = ANY_OTHER_NUMBER;
   }
   else
   {
      u8DeviceSrvStatus = DEV_ERROR; //Connected or working
   }

   if(m_poTrace)
   {	
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
         TR_CLASS_HMI_HSA_SYSTEM_TESTMODE,"u8DeviceSrvStatus = %d", u8DeviceSrvStatus);
   }

   return u8DeviceSrvStatus;
}
/*!***********************************************************************
* METHOD:			vGetUsbDeviceSrvID
* CLASS:			clHSA_TestMode
* DESCRIPTION:		Returns the SERVICE ID of USB Device.
* PARAMETER:		GUI_String
* RETURNVALUE:  	void
*************************************************************************/
void clHSA_TestMode::vGetUsbDeviceSrvID(GUI_String *out_result)
{
   tU16 u16VendorID = 0;
   tU16 u16ProductID = 0;
   tChar* tCharpacText = NULL;
   tCharpacText = OSAL_NEW tChar[MAX_BUFFER_STRLEN];
   if(tCharpacText != NULL)
      OSAL_pvMemorySet((tVoid*)tCharpacText,NULL,MAX_BUFFER_STRLEN);
   // Read the Vendor ID and the Manufacture ID

   if(m_poDataPool != NULL)
   {
      m_poDataPool->u32dp_get( DPDEVMGR__DRIVEINFO_USB_VENDOR_ID,
         &u16VendorID,
         sizeof(u16VendorID) );
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,TR_CLASS_HMI_HSA_SYSTEM,
         "Vendor ID %04x",u16VendorID);

      m_poDataPool->u32dp_get(
         DPDEVMGR__DRIVEINFO_USB_PRODUCT_ID,
         &u16ProductID,
         sizeof(u16ProductID));
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,TR_CLASS_HMI_HSA_SYSTEM,
         "Product ID %04x",u16ProductID);

   }

   UTF8_s32SaveNPrintFormat(tCharpacText,MAX_BUFFER_STRLEN,"ID-%04x-%04x",u16VendorID,u16ProductID);

   GUI_String_vSetCStr(out_result, (const tU8*)tCharpacText);

   if(tCharpacText != NULL)
      OSAL_DELETE [] tCharpacText;
}
/*!***********************************************************************
* METHOD:			vGetFGSHWVersion
* CLASS:			clHSA_TestMode
* DESCRIPTION:	Returns the FGS HARDWARE VERSION.
* PARAMETER:     NONE
* RETURNVALUE:  	STRING
*************************************************************************/
void clHSA_TestMode:: vGetFGSHWVersion(GUI_String *out_result)
{
   tChar* pacText = OSAL_NEW tChar[MAX_GPS_FORMAT_STRLEN];

   /*if(m_poDataPool)
   {
   u32Size   = m_poDataPool->u32dp_getElementSize(DPOSAL__PARAMOUNT_VERSION_FGSVERSION_HW);
   OSAL_pvMemorySet((tVoid*)pacText,NULL,MAX_GPS_FORMAT_STRLEN);

   m_poDataPool->u32dp_get( DPOSAL__PARAMOUNT_VERSION_FGSVERSION_HW, 
   (tVoid*)pacText, 
   u32Size	);
   }*/

   /* Return Altitude*/
   GUI_String_vSetCStr(out_result, (tU8*)pacText);

   /* Release the Allocated memory before Leaving */
   if(pacText != NULL)
      OSAL_DELETE [] pacText;

}
/*!***********************************************************************
* METHOD:			ulwGetUSBMediaStatus
* CLASS:			clHSA_TestMode
* DESCRIPTION:	Returns the USB STATUS.
* PARAMETER:     NONE
* RETURNVALUE:  	ULWORD
*************************************************************************/
ulword clHSA_TestMode::ulwGetUSBMediaStatus()
{
   tU8 u8USBMediaStatus = 0;
   tU8 u8USBConnectionStatus = 0;

   m_poDataPool->u32dp_get( DPDEVMGR__DRIVEINFO_USB_DEVICE_TYPE,
      &u8USBMediaStatus,
      sizeof(u8USBMediaStatus)
      );	

   switch(u8USBMediaStatus)
   {
   case midw_fi_tcl_e8_Device::FI_EN_DTY_SDCARD:
   case midw_fi_tcl_e8_Device::FI_EN_DTY_USB:
      u8USBMediaStatus = EN_MASS_STORAGE;
      break;
   case midw_fi_tcl_e8_Device::FI_EN_DTY_IPHONE:
   case midw_fi_tcl_e8_Device::FI_EN_DTY_IPAD:
   case midw_fi_tcl_e8_Device::FI_EN_DTY_IPOD:
      u8USBMediaStatus = EN_IPOD;
      break;
   case midw_fi_tcl_e8_Device::FI_EN_DTY_UNKNOWN:
   case midw_fi_tcl_e8_Device::FI_EN_DTY_MTP:
      u8USBMediaStatus = EN_UNKNOWN_MEDIUM;
      break;
   default:
      u8USBMediaStatus =  EN_NO_MEDIA;
      break;
   }

   m_poDataPool->u32dp_get( DPDEVMGR__DRIVEINFO_USB_DEVICE_CONNECT_STATUS,
      &u8USBConnectionStatus,
      sizeof(u8USBConnectionStatus)
      );	      

   EDevMgrDeviceConnStatus eDevMgrDeviceConnStatus = USB_DEV_LAST;
   eDevMgrDeviceConnStatus = (EDevMgrDeviceConnStatus)u8USBConnectionStatus;

   if((USB_DEV_REMOVED_BY_USR == eDevMgrDeviceConnStatus)||(USB_DEV_UNDEFINED == eDevMgrDeviceConnStatus))
   {
      u8USBMediaStatus =  EN_NO_MEDIA;
   }
   else
   {
      //Do nothing
   }

   if(NULL != m_poTrace)
   {	
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
         TR_CLASS_HMI_HSA_SYSTEM_TESTMODE,
         (tU16)(HSA_API__GET_USB_MEDIA_STATUS),
         1,
         &u8USBMediaStatus);
   } 

   return (ulword)u8USBMediaStatus;
}
/*!***********************************************************************
* METHOD:			ulwGetCDDriveStatus
* CLASS:			clHSA_TestMode
* DESCRIPTION:	Returns the CD Drive state.
* PARAMETER:     NONE
* RETURNVALUE:  	ULWORD
*************************************************************************/
ulword clHSA_TestMode::ulwGetCDDriveStatus()
{
   tU8 u8CDDriveStatus = 0;

   m_poDataPool->u32dp_get( DPDVPTSM__CD_DRIVE_STATUS, 
      &u8CDDriveStatus, 
      sizeof(u8CDDriveStatus)
      );	

   if(m_poTrace)
   {	
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
         TR_CLASS_HMI_HSA_SYSTEM_TESTMODE,
         (tU16)(HSA_API__GET_CD_DRIVE_STATUS),
         1,
         &u8CDDriveStatus);
   } 

   /* Middleware is not ready, Always showing the value as Ready */
   u8CDDriveStatus = ALWAYS_OK; // to be removed when middleware is ready.

   return u8CDDriveStatus;
}
void clHSA_TestMode::vGetUSBMemoryTotal(GUI_String *out_result)
{
   tU32 u32USBMemoryTotal_LSB = 0;
   tU32 u32USBMemoryTotal_MSB = 0;

   tChar* tCharpacText = NULL;
   tCharpacText = OSAL_NEW tChar[MAX_BUFFER_STRLEN];
   if(tCharpacText != NULL)
      OSAL_pvMemorySet((tVoid*)tCharpacText,NULL,MAX_BUFFER_STRLEN);

   if ( m_poDataPool != NULL )
   {

      m_poDataPool->u32dp_get( DPDEVMGR__DRIVEINFO_USB_TOTAL_SIZE_MSB,
         &u32USBMemoryTotal_MSB,
         sizeof(u32USBMemoryTotal_MSB)
         );
      m_poDataPool->u32dp_get( DPDEVMGR__DRIVEINFO_USB_TOTAL_SIZE_LSB,
         &u32USBMemoryTotal_LSB,
         sizeof(u32USBMemoryTotal_LSB)
         );
   }  

   if ( NULL != m_poTrace )
   {
      /*Trace success config */
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,TR_CLASS_HMI_HSA_SYSTEM,
         "u32USBMemoryTotal_LSB %d u32USBMemoryTotal_MSB %d",u32USBMemoryTotal_LSB,u32USBMemoryTotal_MSB);
   }
   if(((u32USBMemoryTotal_LSB == 0xFFFFFFFF) && (u32USBMemoryTotal_MSB == 0xFFFFFFFF)) == TRUE)
   {
      u32USBMemoryTotal_LSB =0;
      u32USBMemoryTotal_MSB =0;
   }

   tU64 u64TotalSize = u32USBMemoryTotal_MSB;
   u64TotalSize = (((u64TotalSize <<BIT_32)| u32USBMemoryTotal_LSB))/DIVIDE_1M;
   if ( NULL != m_poTrace )
   {
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,TR_CLASS_HMI_HSA_SYSTEM,
         " %llx",u64TotalSize);
   }

   UTF8_s32SaveNPrintFormat( tCharpacText,MAX_GPS_FORMAT_STRLEN,"%lld MB",u64TotalSize);

   GUI_String_vSetCStr(out_result, (tU8*)tCharpacText);

   /** Release the Allocated memory before Leaving */
   OSAL_DELETE [] tCharpacText;
}

/*!***********************************************************************
* METHOD:			vGetIpodFirmwareVersion
* CLASS:			clHSA_TestMode
* DESCRIPTION:	Returns the IPOD SW VERSION.
* PARAMETER:     NONE
* RETURNVALUE:  	String
*************************************************************************/
void clHSA_TestMode::vGetIpodFirmwareVersion(GUI_String *out_result)
{
   tU32 u32Size=0;
   tChar* pacText = OSAL_NEW tChar[MAX_IPOD_SW_STRLEN];
   if(m_poTrace)
   {	
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
         TR_CLASS_HMI_HSA_SYSTEM_TESTMODE,"vGetIpodFirmwareversion");
   }  

   vSendCommandtoHSI( HSI__OSAL__VERSION__PARAMOUNT_SW);

   if(m_poDataPool)
   {
      u32Size   = m_poDataPool->u32dp_getElementSize(DPOSAL__IPOD_SW_VERSION);
      if(pacText != NULL)
         OSAL_pvMemorySet((tVoid*)pacText,NULL,MAX_IPOD_SW_STRLEN);

      m_poDataPool->u32dp_get( DPOSAL__IPOD_SW_VERSION, 
         (tVoid*)pacText, 
         u32Size);
   }

   GUI_String_vSetCStr(out_result, (tU8*)pacText);

   if(m_poTrace)
   {	
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
         TR_CLASS_HMI_HSA_SYSTEM_TESTMODE,"IPOD SW VERSION = %s",
         (tU8*)pacText );
   }  

   OSAL_DELETE [] pacText;
}
/*!***********************************************************************
* METHOD	  :	ulwGetMFLKeyPressed
* CLASS      :	clHSA_TestMode
* DESCRIPTION:	Returns the enum Value of MFL Key pressed.
* PARAMETER  : NONE
* RETURNVALUE:	ulword
*************************************************************************/
ulword clHSA_TestMode::ulwGetMFLKeyPressed()
{
   tU8 u8MFLKeyPressed = 0;

   if(m_poDataPool)
   {
      m_poDataPool->u32dp_get( 
         DPDVPTSM__MFL_KEY_PRESSED,
         &u8MFLKeyPressed,
         sizeof(u8MFLKeyPressed));
   }

   if(m_poTrace)
   {	
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
         TR_CLASS_HMI_HSA_SYSTEM_TESTMODE,"u8MFLKeyPressed = %d",
         u8MFLKeyPressed);
   }  

   return u8MFLKeyPressed;

}
/**
* Method: ulwGetExtPhoneSignalState
* Returns the state of ext Phone Signal as an integer
* NISSAN NAR
*/
ulword clHSA_TestMode::ulwGetExtPhoneSignalState()
{
   tBool bRetVal = NULL;
   //TODO: API Can be deleted, no more used.
   if(m_poTrace)
   {	
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
         TR_CLASS_HMI_HSA_SYSTEM_TESTMODE,"bRetVal = %d", bRetVal);
   }  

   return (tU8)bRetVal;
}

/**
* Method: vStartCsmEngineering
* Start the CsmEngineering method of VD VehicleData(cyclic methodStart).
* NISSAN LCN NAR NET#2 Sample
*/
void clHSA_TestMode::vStartCsmEngineering()
{
   sMsg msg(NULL,CMMSG_TESTMODE_SYSTEM_CSM_ENG, NULL, NULL );
   if (NULL != pclHSI_TestMode)
   {
      pclHSI_TestMode->bExecuteMessage(msg);
   }   
}
/**
* Method: ulwGetCsmEngineering_Count
* CsmEngineering data count
* NISSAN LCN NAR NET#2 Sample
*/
ulword clHSA_TestMode::ulwGetCsmEngineering_Count()
{
   tU8 u8CsmEnggData = 0;
   if ( NULL != m_poDataPool )
   {
      m_poDataPool->u32dp_get( 
         DPDVPTSM__CSMENGINEERING_DATA_COUNT,
         &u8CsmEnggData, 
         sizeof(u8CsmEnggData));
   }
   return u8CsmEnggData;
}
/**
* Method: vGetCsmEngineeringData
* This function is uesed to get the CsmEngineering data of the VD VehicleData.
* NISSAN LCN NAR NET#2 Sample
*/
void clHSA_TestMode::vGetCsmEngineeringData(GUI_String *out_result,ulword ListEntryNumber)
{
   tString pacCsmMsgEnggData[510]; 
   OSAL_pvMemorySet(pacCsmMsgEnggData,NULL,510);

   vReadDataFromDataPool(
      (tU32)DPDVPTSM__CSMENGINEERING_DATA,
      (tU8)DVPTSM__CSMENGINEERING_DATA_TEXT,//u8DataPoolListColumnIndex 
      (tU8)ListEntryNumber,//u8DataPoolListRowIndex 
      pacCsmMsgEnggData,
      510);
   /* copy for usage of checking function and for return*/
   GUI_String_vSetCStr(out_result, (tU8*) pacCsmMsgEnggData);
}
/**
* Method: vBTDevDeviceAdress
* Returns the Bluetooth adress of the internal bluetooth module chip
* B
*/
void clHSA_TestMode::vBTDevDeviceAdress(GUI_String *out_result)
{
   bpstl::string ostr;	//Variable to hold the datapool element.

   if(NULL != m_poDataPool)  
   {
      m_poDataPool->u32dp_get( 
         DPTELEPHONE__UGZZC_BT_ADDRESS, //Reading the datapool element
         &ostr, 
         0);
   }

   /** Return the String*/	
   GUI_String_vSetCStr(out_result, ( const tU8*)ostr.c_str());

   if(m_poTrace)
   {	
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
         TR_CLASS_HMI_HSA_SYSTEM_TESTMODE,
         (tU16)(HSA_API_GET_BT_DEVICE_ADDRESS),
         MAX_BUFFER_STRLEN,
         (const tU8*)ostr.c_str() );
   }

}
/**
* Method: vBTDevPIMOperationStatus
* Returns the u16PimOperationStatus Value as Hex fomated string
* B
*/
void clHSA_TestMode::vBTDevPIMOperationStatus(GUI_String *out_result)
{
   tU16 u16PimOperationSts = 0 ;	//Variable to hold the datapool element.

   if(NULL != m_poDataPool) 
   {
      m_poDataPool->u32dp_get( 
         DPTELEPHONE__BT_DEV_STATUSES_PIM_OPERATION_STS, //Reading the datapool element
         &u16PimOperationSts, 
         sizeof(u16PimOperationSts));
   }

   tChar pacStr[ MAX_BUFFER_STRLEN ]  = "\0";

   UTF8_s32SaveNPrintFormat( pacStr,MAX_BUFFER_STRLEN,
      "%x",u16PimOperationSts);
   /** Return the String*/	
   GUI_String_vSetCStr(out_result, (tU8*)pacStr);

   if(m_poTrace)
   {	
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
         TR_CLASS_HMI_HSA_SYSTEM_TESTMODE,
         (tU16)(HSA_API_GET_PIM_OPERATION_STS),
         MAX_BUFFER_STRLEN,
         (tU8*)pacStr );
   }

}

/**
* Method: vBTDevPhonebookEntriesCount
* Bluetooth module developer value for the two possible phone book sizes
* Parameter: Value="0" Meaning="Mem Phonebook" 
*            Value="1" Meaning="SIM Phonebook"
*/
void clHSA_TestMode::vBTDevPhonebookEntriesCount(GUI_String *out_result,ulword PhonebookType)
{
   tU32 u32PhoneBookSize = 0;
   if(NULL != m_poDataPool)  
   {
      /* If Phonebook type is of PhoneMemory*/
      if ( PhonebookType == MEM_PHONEBOOK )
      {
         m_poDataPool->u32dp_get( 
            DPTELEPHONE__BT_DEV_STATUSES_PIM_MEM_PHONEBOOK_SIZE, //Reading the datapool element
            &u32PhoneBookSize, 
            sizeof(u32PhoneBookSize));

      }
      /* If Phonebook type is of SIM Memory*/
      else if ( PhonebookType == SIM_PHONEBOOK )
      {
         m_poDataPool->u32dp_get( 
            DPTELEPHONE__BT_DEV_STATUSES_PIM_SIM_PHONEBOOK_SIZE, //Reading the datapool element
            &u32PhoneBookSize, 
            sizeof(u32PhoneBookSize));

      }

   }
   tChar pacStr[ MAX_BUFFER_STRLEN ]  = "\0";

   UTF8_s32SaveNPrintFormat( pacStr,MAX_BUFFER_STRLEN,
      "%d",u32PhoneBookSize);

   /** Return the String*/	
   GUI_String_vSetCStr(out_result, (tU8*)pacStr);

   if(m_poTrace)
   {	
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
         TR_CLASS_HMI_HSA_SYSTEM_TESTMODE,
         (tU16)(HSA_API_GET_PHONEBOOK_ENTRIES_COUNT),
         MAX_BUFFER_STRLEN,
         (tU8*)pacStr );
   }
}

/**
* Method: vBTDevHfpAgSupportedFeatures
* Bluetooth module developer value HfpAgSupportedFeatures.
* Returns the HfpAgSupportedFeatures Value as Hex fomated string
*/
void clHSA_TestMode::vBTDevHfpAgSupportedFeatures(GUI_String *out_result)
{
   tU16 u16HfpAgSupportedFeatures = 0;

   if(NULL != m_poDataPool) 
   {
      m_poDataPool->u32dp_get( 
         DPTELEPHONE__BT_DEV_STATUSES_HFPAG_SUPPORTED_FEATURES, //Reading the datapool element
         &u16HfpAgSupportedFeatures, 
         sizeof(u16HfpAgSupportedFeatures));

   }
   tChar pacStr[ MAX_BUFFER_STRLEN ]  = "\0";

   UTF8_s32SaveNPrintFormat( pacStr,MAX_BUFFER_STRLEN,
      "%x",u16HfpAgSupportedFeatures);

   /** Return the String*/	
   GUI_String_vSetCStr(out_result, (tU8*)pacStr);

   if(m_poTrace)
   {	
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
         TR_CLASS_HMI_HSA_SYSTEM_TESTMODE,
         (tU16)(HSA_API_GET_HFPAG_SUPPORTED_FEATURES),
         MAX_BUFFER_STRLEN,
         (tU8*)pacStr );
   }

}

/**
* Method: ulwBTDevAvpPlayStatus
* Bluetooth module developer value u8AvpPlayStatus, indicate the working Status of the currently connected media player.
* Possible Return Values : 0 = Stopped, 1 = Playing, 2 = Paused, 
*                          3 = Fwd_seeking, 4 = Rev_seeking
*/
ulword clHSA_TestMode::ulwBTDevAvpPlayStatus()
{
   tU8 u8AvpPlayStatus = 0;

   if(NULL != m_poDataPool) 
   {
      m_poDataPool->u32dp_get( 
         DPTELEPHONE__BT_DEV_STATUSES_AVP_PLAY_STS, //Reading the datapool element
         &u8AvpPlayStatus, 
         sizeof(u8AvpPlayStatus));

   }

   if(m_poTrace)
   {	
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
         TR_CLASS_HMI_HSA_SYSTEM_TESTMODE,
         (tU16)(HSA_API_GET_AVP_PLAY_STS),
         1,
         &u8AvpPlayStatus);
   }

   return u8AvpPlayStatus;
}

/**
* Method: vBTDevAvpSupportedPlayerStatus
* Bluetooth module developer value u16AvpSupportedPlayerStatus.
* Returns the u16AvpSupportedPlayerStatus Value as Hex fomated string
*/
void clHSA_TestMode::vBTDevAvpSupportedPlayerStatus(GUI_String *out_result)
{
   tU16 u16AvpSupportedPlayerStatus = 0;

   if(NULL != m_poDataPool) 
   {
      m_poDataPool->u32dp_get( 
         DPTELEPHONE__BT_DEV_STATUSES_AVP_SUPPORTED_PLAYER_STS, //Reading the datapool element
         &u16AvpSupportedPlayerStatus, 
         sizeof(u16AvpSupportedPlayerStatus));

   }
   tChar pacStr[ MAX_BUFFER_STRLEN ]  = "\0";

   UTF8_s32SaveNPrintFormat( pacStr,MAX_BUFFER_STRLEN,
      "%x",u16AvpSupportedPlayerStatus);

   /** Return the String*/	
   GUI_String_vSetCStr(out_result, (tU8*)pacStr);

   if(m_poTrace)
   {	
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
         TR_CLASS_HMI_HSA_SYSTEM_TESTMODE,
         (tU16)(HSA_API_GET_AVP_SUPPORTED_PLAYER_STS),
         MAX_BUFFER_STRLEN,
         (tU8*)pacStr );
   }

}
/**
* Method: vBTDevSMSSupportedFeatures
* Bluetooth module developer value u16SmsSupportedFeatures.
* Returns the u16SmsSupportedFeatures Value as Hex fomated string
*/
void clHSA_TestMode::vBTDevSMSSupportedFeatures(GUI_String *out_result)
{
   tU16 u16SmsSupportedFeatures = 0;

   if(NULL != m_poDataPool) 
   {
      m_poDataPool->u32dp_get( 
         DPTELEPHONE__BT_DEV_STATUSES_SMS_SUPPORTED_FEATURES, //Reading the datapool element
         &u16SmsSupportedFeatures, 
         sizeof(u16SmsSupportedFeatures));

   }
   tChar pacStr[ MAX_BUFFER_STRLEN ]  = "\0";

   UTF8_s32SaveNPrintFormat( pacStr,MAX_BUFFER_STRLEN,
      "%x",u16SmsSupportedFeatures);

   /** Return the String*/	
   GUI_String_vSetCStr(out_result, (tU8*)pacStr);

   if(m_poTrace)
   {	
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
         TR_CLASS_HMI_HSA_SYSTEM_TESTMODE,
         (tU16)(HSA_API_GET_SMS_SUPPORTED_FEATURES),
         MAX_BUFFER_STRLEN,
         (tU8*)pacStr );
   }
}

/**
* Method: vBTDevSMSDeviceSystemStatus
* Bluetooth module developer value u8SmsDeviceSystemStatus.
* Returns the u8SmsDeviceSystemStatus Value as Hex fomated string
*/
void clHSA_TestMode::vBTDevSMSDeviceSystemStatus(GUI_String *out_result)
{
   tU8 u8SmsDeviceSystemStatus = 0;

   if(NULL != m_poDataPool) 
   {
      m_poDataPool->u32dp_get( 
         DPTELEPHONE__BT_DEV_STATUSES_SMS_DEVICE_SYS_STS, //Reading the datapool element
         &u8SmsDeviceSystemStatus, 
         sizeof(u8SmsDeviceSystemStatus));

   }
   tChar pacStr[ MAX_BUFFER_STRLEN ]  = "\0";

   UTF8_s32SaveNPrintFormat( pacStr,MAX_BUFFER_STRLEN,
      "%x",u8SmsDeviceSystemStatus);

   /** Return the String*/	
   GUI_String_vSetCStr(out_result, (tU8*)pacStr);

   if(m_poTrace)
   {	
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
         TR_CLASS_HMI_HSA_SYSTEM_TESTMODE,
         (tU16)(HSA_API_GET_SMS_DEVICE_SYS_STS),
         MAX_BUFFER_STRLEN,
         (tU8*)pacStr );
   }
}

/**
* Method: vBTDevSMSSupportedNotification
* Bluetooth module developer value u16SmsSupportedNotification.
* Returns the u16SmsSupportedNotification Value as Hex fomated string
*/
void clHSA_TestMode::vBTDevSMSSupportedNotification(GUI_String *out_result)
{
   tU8 u16SmsSupportedNotification = 0;

   if(NULL != m_poDataPool) 
   {
      m_poDataPool->u32dp_get( 
         DPTELEPHONE__BT_DEV_STATUSES_SMS_SUPPORTED_NOTIFICATION, //Reading the datapool element
         &u16SmsSupportedNotification, 
         sizeof(u16SmsSupportedNotification));

   }
   tChar pacStr[ MAX_BUFFER_STRLEN ]  = "\0";

   UTF8_s32SaveNPrintFormat( pacStr,MAX_BUFFER_STRLEN,
      "%x",u16SmsSupportedNotification);

   /** Return the String*/	
   GUI_String_vSetCStr(out_result, (tU8*)pacStr);

   if(m_poTrace)
   {	
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
         TR_CLASS_HMI_HSA_SYSTEM_TESTMODE,
         (tU16)(HSA_API_GET_SMS_SUPPORTED_NOTIFICATION),
         MAX_BUFFER_STRLEN,
         (tU8*)pacStr );
   }
}

/**
* Method: blBTDevEnabledServices
* Bluetooth module developer value to show the available Services
* Parameter : BTService - The kind of service or BT Profile
* Value: "0" Meaning="HFP" , "1" Meaning="AVP", "2" Meaning="SMS" , 
*        "3" Meaning="PIM", "4" Meaning="DUN"
* ReturnType: "DATA_TYPE_BOOL"
*              Description="false=Service not available, true=Service available"
*/
tbool clHSA_TestMode::blBTDevEnabledServices(ulword BTService)
{
   tBool bServiceEnabled = FALSE;
   tU32 u32DPId = 0;

   switch(BTService)
   {
   case (HFP_SERVICE) :
      u32DPId = DPTELEPHONE__IS_HFP_FEATURE_ENABLED;
      break;

   case (AVP_SERVICE) :
      u32DPId = DPTELEPHONE__IS_AVP_FEATURE_ENABLED;
      break;

   case (SMS_SERVICE) :
      u32DPId = DPTELEPHONE__IS_SMS_FEATURE_ENABLED;
      break;

   case (PIM_SERVICE) :
      u32DPId = DPTELEPHONE__IS_PIM_FEATURE_ENABLED;
      break;

   case (DUN_SERVICE) :
      u32DPId = DPTELEPHONE__IS_DUN_FEATURE_ENABLED;
      break;

   default:

      break;
   }      
   if(NULL != m_poDataPool)  
   {
      m_poDataPool->u32dp_get( 
         u32DPId, //Reading the datapool element
         &bServiceEnabled, 
         sizeof(bServiceEnabled));

   }

   if(m_poTrace)
   {	
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
         TR_CLASS_HMI_HSA_SYSTEM_TESTMODE,
         (tU16)(HSA_API_GET_BTDEV_ENABLED_SERVICES),
         1,
         &bServiceEnabled);
   }

   return bServiceEnabled;

}
/**
* Method: blBTDevConnectedServices
* Bluetooth module developer value to show the connected Services
* Parameter : BTService - The kind of service or BT Profile
* Value: "0" Meaning="HFP" , "1" Meaning="AVP", "2" Meaning="SMS" , 
*        "3" Meaning="PIM", "4" Meaning="DUN"
* ReturnType: "DATA_TYPE_BOOL"
*              Description="false=Service inconnected, true=Service connected"
*/
tbool clHSA_TestMode::blBTDevConnectedServices(ulword BTService)
{
   tBool bServiceConnected = FALSE;
   tU32 u32DPId = 0;

   switch(BTService)
   {
   case (HFP_SERVICE) :
      u32DPId = DPTELEPHONE__IS_HFP_CONNECTION_ESTABLISHED;
      break;

   case (AVP_SERVICE) :
      u32DPId = DPTELEPHONE__IS_AVP_CONNECTION_ESTABLISHED;
      break;

   case (SMS_SERVICE) :
      u32DPId = DPTELEPHONE__IS_SMS_CONNECTION_ESTABLISHED;
      break;

   case (PIM_SERVICE) :
      u32DPId = DPTELEPHONE__IS_PIM_CONNECTION_ESTABLISHED;
      break;

   case (DUN_SERVICE) :
      u32DPId = DPTELEPHONE__IS_DUN_CONNECTION_ESTABLISHED;
      break;

   default:

      break;
   }

   if(NULL != m_poDataPool)  
   {
      m_poDataPool->u32dp_get( 
         u32DPId, //Reading the datapool element
         &bServiceConnected, 
         sizeof(bServiceConnected));

   }

   if(m_poTrace)
   {	
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
         TR_CLASS_HMI_HSA_SYSTEM_TESTMODE,
         (tU16)(HSA_API_GET_BTDEV_CONNECTED_SERVICES),
         1,
         &bServiceConnected);
   }

   return bServiceConnected;

}

/**
* Method: ulwGetTelephoneMicrophoneConnectionStatus
* Indicates the current microphone connection status.
* ReturnType: "DATA_TYPE_UINT"
*              Description="Returns an integer value that indicate the current telephone microphone connection status. 
* Possible return values are: 0: PHN_VAG_DIAG_MIC_CONN_OPEN Open load is detected for Microphone connection.; 
*                             1: PHN_VAG_DIAG_MIC_CONN_SHORT Short to ground is detected for Microphone connection.; 
*                             2: PHN_VAG_DIAG_MIC_CONN_SHORT Microphone connection status is Short.; 
*                             3: PHN_VAG_DIAG_MIC_CONN_OK Microphone connection status is OK.; 
*                             255: PHN_VAG_DIAG_MIC_CONN_UNDEF Microphone connection status is undefined" 
*/
ulword clHSA_TestMode::ulwGetTelephoneMicrophoneConnectionStatus()
{
   tU8 u8MicrophoneConnSts = 0;

   if(NULL != m_poDataPool)  
   {
      m_poDataPool->u32dp_get( 
         DPTELEPHONE__DIAG_MICROPHONE_CONNECTION_STATUS, //Reading the datapool element
         &u8MicrophoneConnSts, 
         sizeof(u8MicrophoneConnSts));

   }

   if(m_poTrace)
   {	
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
         TR_CLASS_HMI_HSA_SYSTEM_TESTMODE,
         (tU16)(HSA_API_GET_MICROPHONE_CONNECTION_STS),
         1,
         &u8MicrophoneConnSts);
   }

   return u8MicrophoneConnSts;
}

/**
* Method: vGetTelephoneMicrophoneConnectionValue
* Indicates the current microphone connection value.
* 
*/
void clHSA_TestMode::vGetTelephoneMicrophoneConnectionValue(GUI_String *out_result)
{
   tU16 u16CurrentValue = 0;

   if(NULL != m_poDataPool) 
   {
      m_poDataPool->u32dp_get( 
         DPTELEPHONE__DIAG_MICROPHONE_CURRENT_VALUE, //Reading the datapool element
         &u16CurrentValue, 
         sizeof(u16CurrentValue));

   }
   tChar pacStr[ MAX_BUFFER_STRLEN ]  = "\0";

   UTF8_s32SaveNPrintFormat( pacStr,MAX_BUFFER_STRLEN,
      "%d",u16CurrentValue);

   /** Return the String*/	
   GUI_String_vSetCStr(out_result, (tU8*)pacStr);

   if(m_poTrace)
   {	
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
         TR_CLASS_HMI_HSA_SYSTEM_TESTMODE,
         (tU16)(HSA_API_GET_MICROPHONE_CONNECTION_VALUE),
         MAX_BUFFER_STRLEN,
         (tU8*)pacStr );
   }
}



/**
* Method: blBTDevTestGetHCIModeStatus
* Returns the status of UGZZC Access Mode
* ReturnType: "DATA_TYPE_BOOL"
* Possible return values are: 0: HCI mode NOT ACTIVE
*                             1: HCI mode is ACTIVE
*/
tbool clHSA_TestMode::blBTDevTestGetHCIModeStatus()
{
   tBool bRetval = FALSE;
   tU8 u8UgzzcAccessMode = 0;

   if(NULL != m_poDataPool) 
   {
      m_poDataPool->u32dp_get( 
         DPTELEPHONE__UGZZC_ACCESS_MODE,
         &u8UgzzcAccessMode, 
         sizeof(u8UgzzcAccessMode));
   }

   switch(u8UgzzcAccessMode)
   {
   case UGZZC_APL_MODE:
      {
         bRetval = FALSE;
         break;
      }
   case UGZZC_HCI_MODE:
      {
         bRetval = TRUE;
         break;
      }
   case UGZZC_UNKNOWN_MODE:
   default:
      {
         bRetval = FALSE;
         break;
      }
   } /** End Of switch */

   if(m_poTrace)
   {	
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
         TR_CLASS_HMI_HSA_SYSTEM_TESTMODE,
         (tU16)(HSA_API_GET_UGZZC_ACCESS_MODE), 
         1,
         &bRetval);
   }

   return bRetval;
}

/**
* Method: vBTDevTestSetHCIModeStatus
* Sets HCI Mode of the BT Module for running the accreditation tests.
* 
*/
void clHSA_TestMode::vBTDevTestSetHCIModeStatus(tbool Value)
{
   /* Inform Component Manager Phone with this Message */
   if( NULL != pclHSI_CMPhone )
   {
      sMsg refMsg(NULL, (tU32)CMMSG_PHONE_SET_UGZZC_ACCESS_MODE, (tU32)Value, NULL);
      pclHSI_CMPhone->bExecuteMessage( refMsg );
   }

   if(m_poTrace)
   {	
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
         TR_CLASS_HMI_HSA_SYSTEM_TESTMODE,
         (tU16)(HSA_API_SET_UGZZC_ACCESS_MODE),
         1,
         &Value);
   }
}

/**
* Method: blBTDevTestGetECNREngineStatus
* Gets status of the echo cancelation noise reduction engine.
* Possible return values are: 0:FALSE - EC/NR engine is switched off
*                             1:TRUE  - EC/NR engine is switched on
*/
tbool clHSA_TestMode::blBTDevTestGetECNREngineStatus()
{
   tBool bRetval = FALSE;

   if(NULL != m_poDataPool)
   {
      m_poDataPool->u32dp_get( 
         DPTELEPHONE__EC_NR_ENGINE_ON_OFF,
         &bRetval, 
         sizeof(bRetval));
   }

   if(m_poTrace)
   {	
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
         TR_CLASS_HMI_HSA_SYSTEM_TESTMODE,
         (tU16)(HSA_API_GET_EC_NR_ENGINE_STS),
         1,
         &bRetval);
   }

   return bRetval;
}

/**
* Method: vBTDevTestToggleECNREngineStatus
* Toggles the status of the echo cancelation noise reduction engine.
* 
*/
void clHSA_TestMode::vBTDevTestToggleECNREngineStatus()
{
   tBool bEcNrEngineIsOn = FALSE;
   tBool bSetECNREngineSts = FALSE;

   if(NULL != m_poDataPool)
   {
      m_poDataPool->u32dp_get( 
         DPTELEPHONE__EC_NR_ENGINE_ON_OFF,
         &bEcNrEngineIsOn, 
         sizeof(bEcNrEngineIsOn));
   }

   /* Toggle the ON/OFF state based on the status */
   if ( bEcNrEngineIsOn == FALSE )
   {
      bSetECNREngineSts = TRUE;
   }

   /* Inform Component Manager Phone with this Message */
   if( NULL != pclHSI_CMPhone )
   {
      sMsg refMsg(NULL, CMMSG_PHONE_SET_EC_NR_ENGINE_STATUS, bSetECNREngineSts, NULL);
      pclHSI_CMPhone->bExecuteMessage( refMsg );
   }

   if(m_poTrace)
   {	
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
         TR_CLASS_HMI_HSA_SYSTEM_TESTMODE,
         (tU16)(HSA_API_SET_EC_NR_ENGINE_STS),
         1,
         &bSetECNREngineSts);
   }
}

/**
* Method: vBTDevTestSetFrequency
* Sets the BT Dev Test Frequency for BT Accredetation
* Parameters are
* 0: 2402MHz
* 1: 2441MHz
* 2: 2480MHz
*/
void clHSA_TestMode::vBTDevTestSetFrequency(ulword Value)
{
   tU8 u8Frequency = (tU8) Value;

   if ( NULL != m_poDataPool )
   {
      m_poDataPool->u32dp_set( 
         DPTELEPHONE__BTDEV_TEST_FREQUENCY,
         &u8Frequency, 
         sizeof(u8Frequency));
   }

   if(m_poTrace)
   {	
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
         TR_CLASS_HMI_HSA_SYSTEM_TESTMODE,
         (tU16)(HSA_API_SET_BTDEV_TEST_FREQUENCY),
         1,
         &u8Frequency);
   }
}

/**
* Method: ulwBTDevTestGetFrequency
* Gives the BT Dev Test Frequency which is set by vSetBTDevTestFrequency().
*  Possible Return values are : '0'->2402MHz or '1'->2441MHz or '2'->2480MHz
*/
ulword clHSA_TestMode::ulwBTDevTestGetFrequency()
{
   tU8 u8Frequency = 0;

   if ( NULL != m_poDataPool )
   {
      m_poDataPool->u32dp_get( 
         DPTELEPHONE__BTDEV_TEST_FREQUENCY,
         &u8Frequency, 
         sizeof(u8Frequency));
   }

   if(m_poTrace)
   {	
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
         TR_CLASS_HMI_HSA_SYSTEM_TESTMODE,
         (tU16)(HSA_API_GET_BTDEV_TEST_FREQUENCY),
         1,
         &u8Frequency);
   }

   return u8Frequency;
}

/**
* Method: vBTDevTestSetPacketType
* Sets the BT Dev Test Frequency for BT Accredetation
* Parameters are
* 0: DH5(339)
* 1: 3DH5(1021)
*/
void clHSA_TestMode::vBTDevTestSetPacketType(ulword Value)
{
   tU8 u8BTPacketType = (tU8)Value;

   if ( NULL != m_poDataPool )
   {
      m_poDataPool->u32dp_set( 
         DPTELEPHONE__BTDEV_TEST_PACKET_TYPE,
         &u8BTPacketType, 
         sizeof(u8BTPacketType));
   }

   if(m_poTrace)
   {	
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
         TR_CLASS_HMI_HSA_SYSTEM_TESTMODE,
         (tU16)(HSA_API_SET_BTDEV_TEST_PACKET_TYPE),
         1,
         &u8BTPacketType);
   }
}

/**
* Method: ulwBTDevTestGetPacketType
* Gives the BT Dev Test Packet type which is set by ulwGetBTDevTestPacketType().
* Possible Return values are : 0 --> DH5(5) or 1 --> 3DH5(53)
*/
ulword clHSA_TestMode::ulwBTDevTestGetPacketType()
{
   tU8 u8PacketType = 0;

   if ( NULL != m_poDataPool )
   {
      m_poDataPool->u32dp_get( 
         DPTELEPHONE__BTDEV_TEST_PACKET_TYPE,
         &u8PacketType, 
         sizeof(u8PacketType));
   }

   if(m_poTrace)
   {	
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
         TR_CLASS_HMI_HSA_SYSTEM_TESTMODE,
         (tU16)(HSA_API_GET_BTDEV_TEST_PACKET_TYPE),
         1,
         &u8PacketType);
   }

   return u8PacketType;
}

/**
* Method: vBTDevTestSetModulationValue
* Sets the BT Dev Test Modulation value for BT Accredetation
* Parameters are
* 0: 0 KHZ
* 1: 500 KHz
* 2: 1000 KHz
*/
void clHSA_TestMode::vBTDevTestSetModulationValue(ulword Value)
{
   tU8 u8Modulation = (tU8) Value;

   if ( NULL != m_poDataPool )
   {
      m_poDataPool->u32dp_set( 
         DPTELEPHONE__BTDEV_TEST_MODULATION_VALUE,
         &u8Modulation, 
         sizeof(u8Modulation));
   }

   if(m_poTrace)
   {	
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
         TR_CLASS_HMI_HSA_SYSTEM_TESTMODE,
         (tU16)(HSA_API_SET_BT_DEV_TEST_MODULATION),
         1,
         &u8Modulation);
   }
}

/**
* Method: ulwBTDevTestGetModulationValue
* Gives the BT Dev Test Modulation which is set by vBTDevTestSetModulationValue().
*  Possible Return values are : '0'->0KHz or '1'->500KHz or '2'->1000KHz
*/
ulword clHSA_TestMode::ulwBTDevTestGetModulationValue()
{
   tU8 u8Modulation = 0;

   if ( NULL != m_poDataPool )
   {
      m_poDataPool->u32dp_get( 
         DPTELEPHONE__BTDEV_TEST_MODULATION_VALUE,
         &u8Modulation, 
         sizeof(u8Modulation));
   }

   if(m_poTrace)
   {	
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
         TR_CLASS_HMI_HSA_SYSTEM_TESTMODE,
         (tU16)(HSA_API_GET_BT_DEV_TEST_MODULATION),
         1,
         &u8Modulation);
   }

   return u8Modulation;
}

/**
* Method: vBTDevRunTest
* Starts the BT Dev test of three test cases.
* 0:  TXDATA1
* 1:  TXDATA2
* 2:  RXSTART1_FIXED_CH
* 3:  HCI_ENABLE_DUT_MODE
* 4:  TXSTART
*/
void clHSA_TestMode::vBTDevRunTest(ulword Test)
{
   m_tU8BTDevActiveTest = (tU8)Test;

   sMsg msg(NULL,CMMSG_PHONE_BT_RF_TEST_START, (tU32)Test, 0 );
   if (NULL != pclHSI_CMPhone){
      pclHSI_CMPhone->bExecuteMessage(msg);
   }
}

/**
* Method: vBTDevStopTest
* Method to stop the BT RF Accredatation
*/
void clHSA_TestMode::vBTDevStopTest()
{
   /* Send the trigger via CMMSG to CMPhone */
   sMsg msg(NULL,CMMSG_PHONE_BT_RF_TEST_STOP, 0, 0 );

   if (NULL != pclHSI_CMPhone){
      pclHSI_CMPhone->bExecuteMessage(msg);
   }

   if(m_poTrace)
   {	
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
         TR_CLASS_HMI_HSA_SYSTEM_TESTMODE,
         (tU16)(HSA_API_BTDEV_STOP_TEST));
   }

}

/**
* Method: vBTDevTestGetLinkKey
* API which gives the BT test values as a string
*/
void clHSA_TestMode::vBTDevTestGetLinkKey(GUI_String *out_result)
{
   tU8 u8LinkKey[PHN_UGZZC_BT_DEVICE_LINK_KEY_LEN];

   OSAL_pvMemorySet(u8LinkKey,NULL,PHN_UGZZC_BT_DEVICE_LINK_KEY_LEN);
   tU8 u8LinkKeyLength = NULL;

   if ( NULL != m_poDataPool )
   {
      m_poDataPool->u32dp_get(
         DPTELEPHONE__BT_DEV_STATUSES_LINKKEY_SIZE,
         &u8LinkKeyLength,
         sizeof(u8LinkKeyLength));

      m_poDataPool->u32dp_get(
         DPTELEPHONE__BT_DEV_STATUSES_LINKKEY,
         &u8LinkKey,
         u8LinkKeyLength);
   }

   tChar pacStr[ MAX_BUFFER_STRLEN ]  = "\0";
   tChar pacStr_temp[ MAX_BUFFER_STRLEN ]  = "\0";

   for(tU8 u8i = 0; u8i < u8LinkKeyLength; u8i++)
   {
      UTF8_s32SaveNPrintFormat( pacStr,MAX_BUFFER_STRLEN,
         "%02X ",u8LinkKey[u8i]);
      UTF8_szSaveStringCopy((pacStr_temp+(u8i*3)),pacStr,MAX_BUFFER_STRLEN);
   }

   GUI_String_vSetCStr(out_result, (tU8*)pacStr_temp);

   if(m_poTrace)
   {	
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
         TR_CLASS_HMI_HSA_SYSTEM_TESTMODE,
         (tU16)(HSA_API_BT_DEV_TEST_LINKKEY),
         MAX_BUFFER_STRLEN,
         (tU8*)pacStr_temp);

      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
         TR_CLASS_HMI_HSA_SYSTEM_TESTMODE,
         (tU16)(HSA_API_BT_DEV_TEST_LINKKEY),
         MAX_BUFFER_STRLEN,
         out_result->pubBuffer_);
   }
}

/**
* Method: vBTDevTestGetTestValues
* API which gives the BT test values as a string
*/
void clHSA_TestMode::vBTDevTestGetTestValues(GUI_String *out_result,ulword TestParameter)
{
   bpstl::string ostr;
   switch(TestParameter)
   {
   case GUI_PHONE_BT_DEV_TEST_TESTNAME:
      {
         if( m_tU8BTDevActiveTest == GUI_TESTMODE_BT_DEV_TEST_TYPE_TXDATA1)
         {
            ostr.assign("EIRP+FrequencyRange+Spurious Emission Transmitter");
         }
         else if(m_tU8BTDevActiveTest == GUI_TESTMODE_BT_DEV_TEST_TYPE_TXDATA2)
         {
            ostr.assign("Frequency Hopping Requirements");
         }
         else if(m_tU8BTDevActiveTest == GUI_TESTMODE_BT_DEV_TEST_TYPE_RXSTART1_FIXED_CH)
         {
            ostr.assign("Spurious Emission Reciever");
         }
         else if(m_tU8BTDevActiveTest == GUI_TESTMODE_BT_DEV_TEST_TYPE_TXSTART)
         {
            ostr.assign("Carrier Only TX");
         }
         break;
      }
   case GUI_PHONE_BT_DEV_TEST_COMMAND_NAME:
      {
         if(m_tU8BTDevActiveTest == GUI_TESTMODE_BT_DEV_TEST_TYPE_TXDATA1)
         {
            ostr.assign("TXDATA1");
         }
         else if(m_tU8BTDevActiveTest == GUI_TESTMODE_BT_DEV_TEST_TYPE_TXDATA2)
         {
            ostr.assign("TXDATA2");
         }
         else if(m_tU8BTDevActiveTest == GUI_TESTMODE_BT_DEV_TEST_TYPE_RXSTART1_FIXED_CH)
         {
            ostr.assign("RXSTART1_FIXED_CH");
         }
         else if(m_tU8BTDevActiveTest == GUI_TESTMODE_BT_DEV_TEST_TYPE_TXSTART)
         {
            ostr.assign("TXSTART");
         }
         break;
      }
   case GUI_PHONE_BT_DEV_TEST_PACKET_MODE:
      {
         if(m_tU8BTDevActiveTest == GUI_TESTMODE_BT_DEV_TEST_TYPE_TXDATA1)
         {
            ostr.assign("TxBurst PRBS9");
         }
         else if(m_tU8BTDevActiveTest == GUI_TESTMODE_BT_DEV_TEST_TYPE_TXDATA2)
         {
            ostr.assign("TxHopping PRBS9");
         }
         else if(m_tU8BTDevActiveTest == GUI_TESTMODE_BT_DEV_TEST_TYPE_RXSTART1_FIXED_CH)
         {
            ostr.assign("N.A");
         }
         else if(m_tU8BTDevActiveTest == GUI_TESTMODE_BT_DEV_TEST_TYPE_TXSTART)
         {
            ostr.assign("N.A");
         }
         break;
      }
   case GUI_PHONE_BT_DEV_TEST_MODULATION:
      {
         if(m_tU8BTDevActiveTest == GUI_TESTMODE_BT_DEV_TEST_TYPE_TXDATA1)
         {
            ostr.assign("N.A");
         }
         else if(m_tU8BTDevActiveTest == GUI_TESTMODE_BT_DEV_TEST_TYPE_TXDATA2)
         {
            ostr.assign("N.A");
         }
         else if(m_tU8BTDevActiveTest == GUI_TESTMODE_BT_DEV_TEST_TYPE_RXSTART1_FIXED_CH)
         {
            ostr.assign("N.A");
         }
         else if(m_tU8BTDevActiveTest == GUI_TESTMODE_BT_DEV_TEST_TYPE_TXSTART)
         {
            tU8 u8Modulation = ulwBTDevTestGetModulationValue();
            if(u8Modulation == PHONE_BT_DEV_TEST_MODUALTION_VALUE_0)
               ostr.assign("0 KHz");
            else if(u8Modulation == PHONE_BT_DEV_TEST_MODUALTION_VALUE_500)
               ostr.assign("500 KHz");
            else if(u8Modulation == PHONE_BT_DEV_TEST_MODUALTION_VALUE_1000)
               ostr.assign("1000 KHz");
         }
         break;
      }
   default:
      break;
   }

   GUI_String_vSetCStr(out_result, (const tU8*) ostr.c_str());

   if(m_poTrace)
   {	
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
         TR_CLASS_HMI_HSA_SYSTEM_TESTMODE,
         (tU16)(HSA_API_BT_DEV_TEST_VALUES),
         MAX_BUFFER_STRLEN_BT_TEST,
         (const tU8*)ostr.c_str() );
   }
}
/*************************************************************************
* FUNCTION   : tVoid Tel_WriteDataToDataPool(tU32   u32DataPoolIndex, 
*                                            tU8    u8DataPoolListColumnIndex,
*                                            tU8    u8DataPoolListRowIndex,
*                                            tVoid* pu8DataWrite , 
*                                            tU32   u32ByteCount) 
* DESCRIPTION: Function writes list data into given data pool index. 
* PARAMETER  : u32DataPoolIndex : Index of the data pool value into which data must be written.
*              u32DataPoolListColumnIndex : Column where to write the data to
*              pu8DataWrite     : u8 data pointer pointing to the data 
*              u32ByteCount     : TByte count of the data to be written 
*
* RETURNVALUE: tVoid 
*
*************************************************************************/
tVoid clHSA_TestMode::vReadDataFromDataPool(
   tU32   u32DataPoolIndex, 
   tU8    u8DataPoolListColumnIndex,
   tU8    u8DataPoolListRowIndex,
   tVoid* pu8Data, 
   tU32   u32ByteCount)
{
   cldpt_base* pDynEnt           = NULL;
   cldpt_list* pDynLst           = NULL;

   if(NULL != pu8Data)
   {   
      if( NULL != m_poDataPool )
      {
         pDynEnt = m_poDataPool->getdp_Elementhandle( u32DataPoolIndex );/* get dynamic element */
         if(pDynEnt != NULL) /* got the dynamic element? */
         { 
            pDynLst = dynamic_cast<cldpt_list*>(pDynEnt);/* cast dynamic element to list element */

            /* write ( RowIdx, ColIdx, Reference, ByteCount ) */
            if(pDynLst != NULL)
               pDynLst->readElement( 
               u8DataPoolListRowIndex, 
               u8DataPoolListColumnIndex,
               pu8Data, 
               u32ByteCount );

         }
      }
   }
}
/******************************************************************************
* FUNCTION:    clHSA_TestMode::vGetDAB_BG_Mode
*------------------------------------------------------------------------------
* DESCRIPTION: API gives the Mode of Background tuner
*------------------------------------------------------------------------------
* PARAMETER:   GUI_String
* RETURNVALUE: void
******************************************************************************/
tVoid clHSA_TestMode::vGetDAB_BG_Mode(GUI_String *out_result)
{
   tU32 u32Size=0;
   tChar* pacText = OSAL_NEW tChar[MAX_GPS_FORMAT_STRLEN];

   if(( NULL != pacText) && (NULL != m_poDataPool))
   {
      u32Size = m_poDataPool->u32dp_getElementSize(DPDAB__TM_DAB_MODE_BG);
      OSAL_pvMemorySet((tVoid*)pacText,NULL,MAX_GPS_FORMAT_STRLEN);

      m_poDataPool->u32dp_get( DPDAB__TM_DAB_MODE_BG,
         (tVoid*)pacText,
         u32Size );
   }
   //Return DAB_BG mode
   GUI_String_vSetCStr(out_result, (tU8*)pacText);
   OSAL_DELETE[] pacText;
}
/******************************************************************************
* FUNCTION:    clHSA_TestMode::vGetDABTMC
*------------------------------------------------------------------------------
* DESCRIPTION: API for the TMC
*------------------------------------------------------------------------------
* PARAMETER:   GUI_String
* RETURNVALUE: void
******************************************************************************/
ulword clHSA_TestMode::ulwGetDABTMC()
{
   tU32 u32DABTMC = 0;
   if ( NULL != m_poDataPool )
   {
      m_poDataPool->u32dp_get(
         DPDAB__TM_NO_TMC_MSGS,
         &u32DABTMC,
         sizeof(u32DABTMC));
   }
   return (ulword)u32DABTMC;
}
/******************************************************************************
* FUNCTION:    clHSA_TestMode::vGetDAB_TPEG
*------------------------------------------------------------------------------
* DESCRIPTION: API for the TPEG
*------------------------------------------------------------------------------
* PARAMETER:   GUI_String
* RETURNVALUE: void
******************************************************************************/
ulword clHSA_TestMode::ulwGetDAB_TPEG()
{
   tU32 u32DABTPEG = 0;
   if ( NULL != m_poDataPool )
   {
      m_poDataPool->u32dp_get(
         DPDAB__TM_NO_TPEG_MSGS,
         &u32DABTPEG,
         sizeof(u32DABTPEG));
   }
   return (ulword)u32DABTPEG;
}

/******************************************************************************
* FUNCTION:    clHSA_TestMode::vGetDAB_TPEGNew
*------------------------------------------------------------------------------
* DESCRIPTION: API for the TPEG
*------------------------------------------------------------------------------
* PARAMETER:   GUI_String
* RETURNVALUE: void
******************************************************************************/
void clHSA_TestMode::vGetDAB_TPEGNew(GUI_String *out_result, ulword ulwInfo_Type)
{
	if(EN_MsgsReceived == ulwInfo_Type)
	   {
		   tU32 u32DABTPEG = 0;
		   if ( NULL != m_poDataPool )
		   {
			  m_poDataPool->u32dp_get(
					  DPDAB__TM_NO_TPEG_MSGS,
				 &u32DABTPEG,
				 sizeof(u32DABTPEG));
		   }
		   tUTF8 pacMsgsRecv[10] = {'\0'};
    	   UTF8_s32SaveNPrintFormat(pacMsgsRecv, 9 ,"%d",u32DABTPEG);
		   GUI_String_vSetCStr(out_result, (tPU8)pacMsgsRecv);
	   }
	else if(EN_LastMsgsRecv == ulwInfo_Type)
	{
		tU32 DPID=0;
		DPID=DPDAB__TM_LAST_RECV_MSGS;
		 bpstl::string param = "";
		 dp.vGetString(DPID, param);

		 GUI_String_vSetCStr(out_result, const_cast<char*>(param.c_str()));
	}
}
/******************************************************************************
* FUNCTION:    clHSA_TestMode::blGetDAB_TSU_Status
*------------------------------------------------------------------------------
* DESCRIPTION: API for TSU status
*------------------------------------------------------------------------------
* PARAMETER:   -
* RETURNVALUE: tbool
******************************************************************************/
tBool clHSA_TestMode::blGetDAB_TSU_Status()
{
   tBool bDAB_TSU_Status = FALSE;
   if ( NULL != m_poDataPool )
   {
      m_poDataPool->u32dp_get(
         DPDAB__TM_TSU_STATUS,
         &bDAB_TSU_Status,
         sizeof(bDAB_TSU_Status));
   }
   return bDAB_TSU_Status;

}
/******************************************************************************
* FUNCTION:    clHSA_TestMode::GetDABSwitchingStatus
*------------------------------------------------------------------------------
* DESCRIPTION: API for getting the switching status
*------------------------------------------------------------------------------
* PARAMETER:   -
* RETURNVALUE: tbool
******************************************************************************/
tBool clHSA_TestMode::blGetDABSwitchingStatus()
{
   tBool bDABSwitchingStatus = FALSE;
   if ( NULL != m_poDataPool )
   {
      m_poDataPool->u32dp_get(
         DPDAB__TM_SWITCHING_STATUS,
         &bDABSwitchingStatus,
         sizeof(bDABSwitchingStatus));
   }
   return bDABSwitchingStatus;
}
/******************************************************************************
* FUNCTION:    clHSA_TestMode::GetDABNumEnsembles_DB
*------------------------------------------------------------------------------
* DESCRIPTION: API for Number of ensembles in the Database
*------------------------------------------------------------------------------
* PARAMETER:   -
* RETURNVALUE: ulword
******************************************************************************/
ulword clHSA_TestMode::ulwGetDABNumEnsembles_DB()
{
   tU32 u32DABNumEnsembles_DB = 0;
   if ( NULL != m_poDataPool )
   {
      m_poDataPool->u32dp_get(
         DPDAB__TM_NO_ENSEMBLES_DB,
         &u32DABNumEnsembles_DB,
         sizeof(u32DABNumEnsembles_DB));
   }
   return (ulword)u32DABNumEnsembles_DB;
}
/******************************************************************************
* FUNCTION:    clHSA_TestMode::GetDABNum_TMC_Services
*------------------------------------------------------------------------------
* DESCRIPTION: API for Number of TMC Services
*------------------------------------------------------------------------------
* PARAMETER:   -
* RETURNVALUE: ulword
******************************************************************************/
ulword clHSA_TestMode::ulwGetDABNum_TMC_Services()
{
   tU32 u32DABNum_TMC_Services = 0;
   if ( NULL != m_poDataPool )
   {
      m_poDataPool->u32dp_get(
         DPDAB__TM_NO_TMC_SERVICES,
         &u32DABNum_TMC_Services,
         sizeof(u32DABNum_TMC_Services));
   }
   return (ulword)u32DABNum_TMC_Services;
}
/******************************************************************************
* FUNCTION:    clHSA_TestMode::GetDABNum_TPEG_Services
*------------------------------------------------------------------------------
* DESCRIPTION: API for Number of TPEG Services
*------------------------------------------------------------------------------
* PARAMETER:   -
* RETURNVALUE: ulword
******************************************************************************/
ulword clHSA_TestMode::ulwGetDABNum_TPEG_Services()
{
   tU32 u32DABNum_TPEG_Services = 0;
   if ( NULL != m_poDataPool )
   {
      m_poDataPool->u32dp_get(
         DPDAB__TM_NO_TPEG_SERVICES,
         &u32DABNum_TPEG_Services,
         sizeof(u32DABNum_TPEG_Services));
   }
   return (ulword)u32DABNum_TPEG_Services;

}
/******************************************************************************
* FUNCTION:    clHSA_TestMode::GetDABExpertIDValue
*------------------------------------------------------------------------------
* DESCRIPTION: API for getting the DAB expert ID value
*------------------------------------------------------------------------------
* PARAMETER:   ulword
* RETURNVALUE: ulword
******************************************************************************/
void clHSA_TestMode::vGetDABExpertIDValue(GUI_String *out_result, ulword ID)
{
   tU16 	u16ExpertID =0;

   if ( NULL != m_poDataPool )
   {
      switch(ID)
      {
      case ID_NO_1:
         {
            m_poDataPool->u32dp_get(DPDAB__TM_EXPERT_API_ID1,&u16ExpertID,
               sizeof(u16ExpertID));
            break;
         }
      case ID_NO_2:
         {
            m_poDataPool->u32dp_get(DPDAB__TM_EXPERT_API_ID2,&u16ExpertID,
               sizeof(u16ExpertID));
            break;
         }
      case ID_NO_3:
         {
            m_poDataPool->u32dp_get(DPDAB__TM_EXPERT_API_ID3,&u16ExpertID,
               sizeof(u16ExpertID));
            break;
         }
      default:
         {
            //Nothing to do
            break;
         }
      }
   }
   tChar pacExpertID[ MAX_BUFFER_STRLEN ]  = "\0";

   UTF8_s32SaveNPrintFormat( pacExpertID,MAX_BUFFER_STRLEN,
      "%x",u16ExpertID);

   /** Return the String*/
   GUI_String_vSetCStr(out_result, (tU8*)pacExpertID);
}
/******************************************************************************
* FUNCTION:    clHSA_TestMode::GetDABExpertIDInfo
*------------------------------------------------------------------------------
* DESCRIPTION: API for getting the Expert label/value
*------------------------------------------------------------------------------
* PARAMETER:   -
* RETURNVALUE: tVoid
******************************************************************************/
tVoid clHSA_TestMode::vGetDABExpertIDInfo(GUI_String *out_result,ulword ID,ulword ID_Value ,tbool Type )
{
   bpstl::string ostrDAB_Expert_Value_Label;

   tU32 ID_ValueRefer = ID_Value;			// to avoid lint warnings
   ID_Value = ID_ValueRefer;

   if(NULL != m_poDataPool)
   {
      if (Type)
      {

         switch(ID)
         {

         case ID_NO_1:
            {
               m_poDataPool->u32dp_get(DPDAB__TM_EXPERT_VALUE1,&ostrDAB_Expert_Value_Label,0);
               break;
            }
         case ID_NO_2:
            {
               m_poDataPool->u32dp_get(DPDAB__TM_EXPERT_VALUE2,&ostrDAB_Expert_Value_Label,0);
               break;
            }
         case ID_NO_3:
            {
               m_poDataPool->u32dp_get(DPDAB__TM_EXPERT_VALUE3,&ostrDAB_Expert_Value_Label,0);
               break;
            }
         default:
            {
               //Do nothing
               break;
            }

         }

      }
      else
      {
         switch(ID)
         {
         case ID_NO_1:
            {
               m_poDataPool->u32dp_get(DPDAB__TM_EXPERT_LABEL1,&ostrDAB_Expert_Value_Label,0);
               break;
            }
         case ID_NO_2:
            {
               m_poDataPool->u32dp_get(DPDAB__TM_EXPERT_LABEL2,&ostrDAB_Expert_Value_Label,0);
               break;
            }
         case ID_NO_3:
            {
               m_poDataPool->u32dp_get(DPDAB__TM_EXPERT_LABEL3,&ostrDAB_Expert_Value_Label,0);
               break;
            }
         default:
            {
               //Do nothing
               break;
            }

         }
      }
   }
   GUI_String_vSetCStr(out_result, (const tU8*)ostrDAB_Expert_Value_Label.c_str());
}

/******************************************************************************
* FUNCTION:    clHSA_TestMode::vDABEnsembleFrequency
*------------------------------------------------------------------------------
* DESCRIPTION: API for changing the DAB ensemble frequency
*------------------------------------------------------------------------------
* PARAMETER:   tbool Direction
* RETURNVALUE: tVoid
******************************************************************************/
tVoid clHSA_TestMode::vDABEnsembleFrequency(tbool Direction)
{
   if( NULL != pclHSI_CMDAB )
   {
      sMsg refMsg(NULL, CMMSG_DAB_TESTMODE_ENSEMBLE_FRQ, (tU32)Direction, NULL);
      pclHSI_CMDAB->bExecuteMessage( refMsg );
   }

}
/******************************************************************************
* FUNCTION:    clHSA_TestMode::vDABChangeService
*------------------------------------------------------------------------------
* DESCRIPTION: API for changing the DAB service
*------------------------------------------------------------------------------
* PARAMETER:   tbool Direction
* RETURNVALUE: tVoid
******************************************************************************/
tVoid clHSA_TestMode::vDABChangeService(tbool Direction)
{
   if( NULL != pclHSI_CMDAB )
   {
      sMsg refMsg(NULL, CMMSG_DAB_TESTMODE_SERVICE_CHANGE, (tU32)Direction, NULL);
      pclHSI_CMDAB->bExecuteMessage( refMsg );
   }
}
/******************************************************************************
* FUNCTION:    clHSA_TestMode::vGetDAB_Database_String
*------------------------------------------------------------------------------
* DESCRIPTION: API for getting the DAB database string
*------------------------------------------------------------------------------
* PARAMETER:   GUI_String *out_result
* RETURNVALUE: tVoid
******************************************************************************/
tVoid clHSA_TestMode::vGetDAB_Database_String(GUI_String *out_result)
{
   bpstl::string ostrDAB_DB_String;

   // Read the DAB database string
   if( NULL != m_poDataPool)
   {
      m_poDataPool->u32dp_get(
         DPDAB__TM_DAB_DB_STRING,
         &ostrDAB_DB_String,
         0);
   }

   // If string is empty, then assign " N.A" to be displayed
   if( ostrDAB_DB_String.empty() == TRUE )
   {
      ostrDAB_DB_String.assign("N.A");
   }

   GUI_String_vSetCStr(out_result, (const tU8*)ostrDAB_DB_String.c_str());
}
/******************************************************************************
* FUNCTION:    clHSA_TestMode::vDAB_DB_CurrentScreenQuery
*------------------------------------------------------------------------------
* DESCRIPTION: API for getting the DAB database current screen query
*------------------------------------------------------------------------------
* PARAMETER:   -
* RETURNVALUE: tVoid
******************************************************************************/
tVoid clHSA_TestMode::vDAB_DB_CurrentScreenQuery()
{
   tU8 u8PageNumber = 0;

   if ( NULL != m_poDataPool )
   {
      m_poDataPool->u32dp_get(DPDAB__TM_DAB_DB_PAGE,&u8PageNumber,
         sizeof(u8PageNumber));
   }
   if( NULL != pclHSI_CMDAB )
   {
      sMsg refMsg(NULL, CMMSG_DAB_TESTMODE_SCREEN_QUERY, (tU32)u8PageNumber, NULL);
      pclHSI_CMDAB->bExecuteMessage( refMsg );
   }
}
/******************************************************************************
* FUNCTION:    clHSA_TestMode::vDAB_DB_Screen
*------------------------------------------------------------------------------
* DESCRIPTION: API for getting the DAB database first screen/next screen query
*------------------------------------------------------------------------------
* PARAMETER:   -
* RETURNVALUE: tVoid
******************************************************************************/
tVoid clHSA_TestMode::vDAB_DB_Screen( tbool Screen_ID)
{
   tU8 u8PageNumber = 0;

   if ( NULL != m_poDataPool )
   {
      m_poDataPool->u32dp_get(DPDAB__TM_DAB_DB_PAGE,&u8PageNumber,
         sizeof(u8PageNumber));
   }

   if (Screen_ID == TRUE) 					//On the exit from the screen
      u8PageNumber = Page_1 ;
   else							// On the entry to any other screens
      u8PageNumber = u8PageNumber + (tU8)Add_1;

   if(u8PageNumber > (tU8)Page_10)		//reset the page number to 1 after 10 pages
   {
      u8PageNumber = (tU8)Page_1;
   }
   else
   {
      //Nothing to do
   }

   if ( NULL != m_poDataPool )
   {
      m_poDataPool->u32dp_set(DPDAB__TM_DAB_DB_PAGE,&u8PageNumber,
         sizeof(u8PageNumber));
   }
   if( NULL != pclHSI_CMDAB )
   {
      sMsg refMsg(NULL, CMMSG_DAB_TESTMODE_SCREEN_QUERY, (tU32)u8PageNumber, NULL);
      pclHSI_CMDAB->bExecuteMessage( refMsg );
   }

}
/******************************************************************************
* FUNCTION:    clHSA_TestMode::ulwGetDAB_Database_Scrn_No
*------------------------------------------------------------------------------
* DESCRIPTION: API for getting the DAB database screen number
*------------------------------------------------------------------------------
* PARAMETER:   -
* RETURNVALUE: tVoid
******************************************************************************/
ulword clHSA_TestMode::ulwGetDAB_Database_Scrn_No()
{
   tU8 u8PageNumber = 0;

   if ( NULL != m_poDataPool )
   {
      m_poDataPool->u32dp_get(DPDAB__TM_DAB_DB_PAGE,&u8PageNumber,
         sizeof(u8PageNumber));
   }
   return (ulword)u8PageNumber;
}
/******************************************************************************
* FUNCTION:    clHSA_TestMode::vSpellerInitforID
*------------------------------------------------------------------------------
* DESCRIPTION: API on the press of ID to go to speller
*------------------------------------------------------------------------------
* PARAMETER:   -
* RETURNVALUE: tVoid
******************************************************************************/
tVoid clHSA_TestMode::vSpellerInitforID(ulword ID)
{

   tU8 u8RowID = (tU8)ID;
   m_poDataPool->u32dp_set( DPDAB__SPELLER_ACTIVE_ROW_ID,
      &u8RowID,sizeof(u8RowID));

   if(m_poTrace) { m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
      TR_CLASS_HMI_HSA_MNGR,
      "RowID %d", u8RowID);	}
}
/******************************************************************************
* FUNCTION:    clHSA_TestMode::vSpellerCharacterInput
*------------------------------------------------------------------------------
* DESCRIPTION: API on the press of ID to go to speller
*------------------------------------------------------------------------------
* PARAMETER:   -
* RETURNVALUE: tVoid
******************************************************************************/

void clHSA_TestMode::vSpellerCharacterInput(const GUI_String *InputString)
{
   tU16  U16Charcount = 0;

   //Get the count of the speller character
   m_poDataPool->u32dp_get( DPDAB__SPELLER_CHARACTER_COUNT,
      &U16Charcount,sizeof(U16Charcount));

   tUTF8 pacAppendedText[SPELLER_CHAR_SIZE_DAB] = "\0";

   //Print the input character
   if(m_poTrace != NULL)
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,TR_CLASS_HMI_HSA_MNGR,"Len=%d [2]=%x", InputString->ulwLen_,InputString->pubBuffer_[2]);

   if ( 0 == OSAL_s32StringCompare(SPELLER_DELETE_CODE,InputString->pubBuffer_))//BackSpace means ef a0 88
   {
      //decrement the count of speller character
      if(U16Charcount > 0)
      {
         U16Charcount --;
         m_poDataPool->u32dp_set( DPDAB__SPELLER_CHARACTER_COUNT,
            &U16Charcount,sizeof(U16Charcount));

         tChar* pcSpellerName = OSAL_NEW tChar[MAX_GPS_FORMAT_STRLEN];
         if (pcSpellerName !=NULL)
         {
            OSAL_pvMemorySet((tVoid*)pcSpellerName,NULL,MAX_GPS_FORMAT_STRLEN);
            tU32 u32Size = m_poDataPool->u32dp_getElementSize(DPDAB__SPELLER_SELECTED_CHAR);
            m_poDataPool->u32dp_get( DPDAB__SPELLER_SELECTED_CHAR,
               (tVoid*)pcSpellerName,u32Size );

            // Delete the last character
            tU16 nIndex = 0;
            while(pcSpellerName[nIndex++] != 0){}
            //Minimum One character should be present to delete
            if(nIndex > 1)
               nIndex = nIndex - 2;
            pcSpellerName[nIndex] = NULL;
            m_poDataPool->u32dp_set(DPDAB__SPELLER_SELECTED_CHAR,pcSpellerName,SPELLER_CHAR_SIZE_DAB);
         }
         else
         {
            //Do nothing
         }
         OSAL_DELETE[] pcSpellerName;

      }
      else
      {
         //Nothing to do
      }
   }
   else
   {
      if(U16Charcount<SPELLER_CHAR_SIZE_DAB)
      {

         tUTF8 pacAppendedText1[SPELLER_CHAR_SIZE_DAB] = "\0";
         UTF8_s32SaveNPrintFormat(pacAppendedText1, SPELLER_CHAR_SIZE_DAB, "%s", (tChar*)InputString->pubBuffer_);
         m_poDataPool->u32dp_get(DPDAB__SPELLER_SELECTED_CHAR,pacAppendedText,SPELLER_CHAR_SIZE_DAB);
         pacAppendedText[U16Charcount]=pacAppendedText1[0];
         m_poDataPool->u32dp_set(DPDAB__SPELLER_SELECTED_CHAR,pacAppendedText,SPELLER_CHAR_SIZE_DAB);
         U16Charcount ++;
         m_poDataPool->u32dp_set( DPDAB__SPELLER_CHARACTER_COUNT,
            &U16Charcount,sizeof(U16Charcount));
      }
      else
      {
         //nothing to do
      }
      if(m_poTrace != NULL)
         m_poTrace->vTrace( TR_LEVEL_HMI_INFO,TR_CLASS_HMI_HSA_MNGR,"SELECTED_CHAR = %s",pacAppendedText);

   }

}
/******************************************************************************
* FUNCTION:    clHSA_TestMode::vSpellerGetNameInput
*------------------------------------------------------------------------------
* DESCRIPTION: API to get the Speller input characters
*------------------------------------------------------------------------------
* PARAMETER:   -
* RETURNVALUE: tVoid
******************************************************************************/
void clHSA_TestMode::vSpellerGetNameInput(GUI_String *out_result)
{
   tChar* pcSpellerName = OSAL_NEW tChar[MAX_GPS_FORMAT_STRLEN];
   if( pcSpellerName != NULL )
   {

      OSAL_pvMemorySet((tVoid*)pcSpellerName,NULL,MAX_GPS_FORMAT_STRLEN);
      tU32 u32Size = m_poDataPool->u32dp_getElementSize(DPDAB__SPELLER_SELECTED_CHAR);
      m_poDataPool->u32dp_get( DPDAB__SPELLER_SELECTED_CHAR,
         (tVoid*)pcSpellerName,u32Size );
   }
   else
   {
      //Do nothing
   }
   GUI_String_vSetCStr(out_result, (tU8*)pcSpellerName);
   OSAL_DELETE[] pcSpellerName;
}
/******************************************************************************
* FUNCTION:    clHSA_TestMode::vSpellerSetNameInput
*------------------------------------------------------------------------------
* DESCRIPTION: API on the press of OK in the speller screen
*------------------------------------------------------------------------------
* PARAMETER:   -
* RETURNVALUE: tVoid
******************************************************************************/

tVoid clHSA_TestMode::vSpellerSetNameInput()
{

   tU16 u16ExpertID = ulwConversionHexToDec();

   if(m_poTrace != NULL)
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,TR_CLASS_HMI_HSA_MNGR,"u16ExpertID = %d",u16ExpertID);
   tU8 u8RowID = 0;
   m_poDataPool->u32dp_get( DPDAB__SPELLER_ACTIVE_ROW_ID,
      &u8RowID,sizeof(u8RowID));
   switch(u8RowID)
   {
   case ID_NO_1:
      {
         m_poDataPool->u32dp_set(DPDAB__TM_EXPERT_API_ID1,&u16ExpertID,
            sizeof(u16ExpertID));
         break;
      }
   case ID_NO_2:
      {
         m_poDataPool->u32dp_set(DPDAB__TM_EXPERT_API_ID2,&u16ExpertID,
            sizeof(u16ExpertID));
         break;
      }
   case ID_NO_3:
      {
         m_poDataPool->u32dp_set(DPDAB__TM_EXPERT_API_ID3,&u16ExpertID,
            sizeof(u16ExpertID));
         break;
      }
   default:
      {
         //Nothing to do
         break;
      }

   }
   if( NULL != pclHSI_CMDAB )
   {
      sMsg refMsg(NULL, CMMSG_DAB_TESTMODE_EXPERT_API, (tU32)u16ExpertID, (tU32)TRUE);
      pclHSI_CMDAB->bExecuteMessage( refMsg );
   }

   tU16  U16Charcount = 0;
   m_poDataPool->u32dp_get( DPDAB__SPELLER_CHARACTER_COUNT,
      &U16Charcount,sizeof(U16Charcount));

   tChar* pcSpellerName = OSAL_NEW tChar[MAX_GPS_FORMAT_STRLEN];


   if (pcSpellerName != NULL)
   {
      OSAL_pvMemorySet((tVoid*)pcSpellerName,NULL,MAX_GPS_FORMAT_STRLEN);
      while (U16Charcount!= 0)
      {
         U16Charcount--;
         pcSpellerName[U16Charcount]=NULL;

      }

      m_poDataPool->u32dp_set( DPDAB__SPELLER_CHARACTER_COUNT,
         &U16Charcount,sizeof(U16Charcount));
      m_poDataPool->u32dp_set(DPDAB__SPELLER_SELECTED_CHAR,pcSpellerName,SPELLER_CHAR_SIZE_DAB);
   }
   else
   {
      //Do nothing
   }
   OSAL_DELETE[] pcSpellerName;

}
/******************************************************************************
* FUNCTION:    clHSA_TestMode::ulwConversionHexToDec
*------------------------------------------------------------------------------
* DESCRIPTION: API for the conversion from hex to dec
*------------------------------------------------------------------------------
* PARAMETER:   -
* RETURNVALUE: tVoid
******************************************************************************/
tU16 clHSA_TestMode::ulwConversionHexToDec()
{
   tU16  U16Charcount = 0;
   m_poDataPool->u32dp_get( DPDAB__SPELLER_CHARACTER_COUNT,
      &U16Charcount,sizeof(U16Charcount));

   tChar* pcSpellerName = OSAL_NEW tChar[MAX_GPS_FORMAT_STRLEN];
   tU16 u16SpellerChar = 0;
   tU16 u16ExpApiId= 0;

   if (pcSpellerName != NULL)
   {
      OSAL_pvMemorySet((tVoid*)pcSpellerName,NULL,MAX_GPS_FORMAT_STRLEN);
      tU32 u32Size = m_poDataPool->u32dp_getElementSize(DPDAB__SPELLER_SELECTED_CHAR);
      m_poDataPool->u32dp_get( DPDAB__SPELLER_SELECTED_CHAR,
         (tVoid*)pcSpellerName,u32Size );


      for( tU8 i= 0;i<U16Charcount;i++)
      {
         //Extracting each character
         u16SpellerChar = (tU16)pcSpellerName[U16Charcount - (i+Add_1)];
         if(u16SpellerChar <(tU16) NUMBER_INPUT)
            u16SpellerChar = u16SpellerChar - (tU16)SUB_48;
         else
            u16SpellerChar = u16SpellerChar - (tU16)SUB_55;

         //Conversion from hex to decimal
         tU16 u16Index = (tU16)pow ((double)16,(double)i);
         u16ExpApiId = (u16SpellerChar *u16Index) + u16ExpApiId;

      }
   }
   else
   {
      //Do nothing
   }
   OSAL_DELETE[] pcSpellerName;
   return u16ExpApiId;
}
/******************************************************************************
* FUNCTION:    clHSA_TestMode::ulwSpellerGetCursorPos
*------------------------------------------------------------------------------
* DESCRIPTION: API to get teh current cursor position
*------------------------------------------------------------------------------
* PARAMETER:   -
* RETURNVALUE: ulword
******************************************************************************/
ulword clHSA_TestMode::ulwSpellerGetCursorPos()
{
   tU16  U16Charcount = 0;
   if(m_poTrace != NULL)
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,TR_CLASS_HMI_HSA_MNGR,"U16Charcount = %d",U16Charcount);
   m_poDataPool->u32dp_get( DPDAB__SPELLER_CHARACTER_COUNT,
      &U16Charcount,sizeof(U16Charcount));
   if(m_poTrace != NULL)
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,TR_CLASS_HMI_HSA_MNGR,"U16Charcount = %d",U16Charcount);
   return (ulword)U16Charcount;
}

#if (OSAL_OS != OSAL_WINNT)
/******************************************************************************
* FUNCTION:    clHSA_TestMode::vCreateScreenShot
*------------------------------------------------------------------------------
* DESCRIPTION: API for creating a screenShot and storing the result in DPGUI__BOOL_SCREEN_SHOT_STATE
*------------------------------------------------------------------------------
* PARAMETER:   -
* RETURNVALUE: tVoid
******************************************************************************/
tVoid clHSA_TestMode::vCreateScreenShot() {

   if (m_poDataPool)	{
      tbool s32ScreenIntermediateValue = 128;
      m_poDataPool->u32dp_set(DPGUI__BOOL_SCREEN_SHOT_STATE,&s32ScreenIntermediateValue, sizeof(s32ScreenIntermediateValue));
      GUI_UTIL_ScreenShot *m_screenShotHandler = OSAL_NEW GUI_UTIL_ScreenShot();
      if (m_screenShotHandler) {
         s32ScreenIntermediateValue = m_screenShotHandler->bCreateScreenShot();
         m_poDataPool->u32dp_set(DPGUI__BOOL_SCREEN_SHOT_STATE,&s32ScreenIntermediateValue, sizeof(s32ScreenIntermediateValue));
         OSAL_DELETE m_screenShotHandler;
      } else {
         s32ScreenIntermediateValue = 0;
         m_poDataPool->u32dp_set(DPGUI__BOOL_SCREEN_SHOT_STATE,&s32ScreenIntermediateValue, sizeof(s32ScreenIntermediateValue));
      }
   }
}

/******************************************************************************
* FUNCTION:    clHSA_TestMode::blGetScreenShotState
*------------------------------------------------------------------------------
* DESCRIPTION: API for getting the stored state of the last screen shot creation attempt DPGUI__BOOL_SCREEN_SHOT_STATE
*------------------------------------------------------------------------------
* PARAMETER:   -
* RETURNVALUE: tBool
******************************************************************************/
tbool clHSA_TestMode::blGetScreenShotState() {
   tbool bScreenShotState = 128;
   if (m_poDataPool)	{
      m_poDataPool->u32dp_get(DPGUI__BOOL_SCREEN_SHOT_STATE,&bScreenShotState, sizeof(bScreenShotState));
   }

   return bScreenShotState;

}

/******************************************************************************
* FUNCTION:    clHSA_TestMode::vScreenShotResetState
*------------------------------------------------------------------------------
* DESCRIPTION: API for reset DPGUI__BOOL_SCREEN_SHOT_STATE to 128 for model.
*------------------------------------------------------------------------------
* PARAMETER:   -
* RETURNVALUE: tVoid
******************************************************************************/
void clHSA_TestMode::vScreenShotResetState() {

   if (m_poDataPool)	{
      tbool bScreenShotState = FALSE;
      m_poDataPool->u32dp_get(DPGUI__BOOL_SCREEN_SHOT_STATE,&bScreenShotState, sizeof(bScreenShotState));
      if (bScreenShotState != 128) {
         tbool bScreenShotState_after_reading = 128;
         m_poDataPool->u32dp_set(DPGUI__BOOL_SCREEN_SHOT_STATE,&bScreenShotState_after_reading, sizeof(bScreenShotState_after_reading));
      }
   }

}

/******************************************************************************
* FUNCTION:    clHSA_TestMode::vToggleScreenShotSetting
*------------------------------------------------------------------------------
* DESCRIPTION: API for toggling the screen shot setting. Torn off/on the functionality
*------------------------------------------------------------------------------
* PARAMETER:   -
* RETURNVALUE: tVoid
******************************************************************************/
tVoid clHSA_TestMode::vToggleScreenShotSetting() {

   if (m_poDataPool) {
      GUI_UTIL_ScreenShot *m_screenShotHandler = OSAL_NEW GUI_UTIL_ScreenShot();
      tbool bNewScreenShotSetting = FALSE;

      if (m_screenShotHandler) {
         if (m_screenShotHandler->oSearchMountsFileforUSB() == OSAL_OK) {
            tbool bScreenShotSetting = FALSE;
            m_poDataPool->u32dp_get(DPGUI__BOOL_SCREEN_SHOT,&bScreenShotSetting, sizeof(bScreenShotSetting));

            if (bScreenShotSetting == FALSE) {
               bNewScreenShotSetting = TRUE;
            } else if (bScreenShotSetting == TRUE) {
               bNewScreenShotSetting = FALSE;
            }

         }
         OSAL_DELETE m_screenShotHandler;
      }

      m_poDataPool->u32dp_set(DPGUI__BOOL_SCREEN_SHOT,&bNewScreenShotSetting, sizeof(bNewScreenShotSetting));
   }
}

/******************************************************************************
* FUNCTION:    clHSA_TestMode::blGetScreenShotSetting
*------------------------------------------------------------------------------
* DESCRIPTION: API for getting the stored setting of screen shot functionality
*------------------------------------------------------------------------------
* PARAMETER:   -
* RETURNVALUE: tbool
******************************************************************************/
tbool clHSA_TestMode::blGetScreenShotSetting() {
   tbool bScreenShotSetting = FALSE;
   GUI_UTIL_ScreenShot *m_screenShotHandler = OSAL_NEW GUI_UTIL_ScreenShot();
   if (m_screenShotHandler) {
      if (m_screenShotHandler->oSearchMountsFileforUSB() != OSAL_OK) {
         OSAL_DELETE m_screenShotHandler;
         m_poDataPool->u32dp_set(DPGUI__BOOL_SCREEN_SHOT,&bScreenShotSetting, sizeof(bScreenShotSetting));
         return bScreenShotSetting;
      }
   }

   if (m_poDataPool)	{
      m_poDataPool->u32dp_get(DPGUI__BOOL_SCREEN_SHOT,&bScreenShotSetting, sizeof(bScreenShotSetting));
   }
   if(m_screenShotHandler != NULL)
      OSAL_DELETE m_screenShotHandler;
   return bScreenShotSetting;
}

#endif

/******************************************************************************
* FUNCTION:    clHSA_TestMode::vSetTimeInterval
* DESCRIPTION: API to set the time interval value
*------------------------------------------------------------------------------
* PARAMETER:   ulword
* RETURNVALUE: -
******************************************************************************/

void clHSA_TestMode::vSetTimeInterval(ulword Interval)
{
   tU8 u8TimeInterval = 0;
   if (Interval < (ulword)LESS_THREE)
   {
      u8TimeInterval = Interval + Add_1;		//The Middleware takes the value from 1-3
   }
   if ( NULL != m_poDataPool )
   {
      m_poDataPool->u32dp_set(DPCLOCK__TIME_INTERVAL,&u8TimeInterval,
         sizeof(u8TimeInterval));
   }
   if( NULL != pclHSI_TestMode )
   {
      sMsg refMsg (NULL, (tU32)CMMSG_TESTMODE_CLOCK_TIME_INTERVAL, (tU32)NULL, NULL) ;
      pclHSI_TestMode->bExecuteMessage(refMsg);
   }
   if(m_poTrace != NULL)
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,TR_CLASS_HMI_HSA_MNGR,"Time Interval  = %d",u8TimeInterval);
}
/******************************************************************************
* FUNCTION:    clHSA_TestMode::ulwGetTimeInterval
*------------------------------------------------------------------------------
* DESCRIPTION: API to get the time interval value
*------------------------------------------------------------------------------
* PARAMETER:   -
* RETURNVALUE: ulword
******************************************************************************/
ulword clHSA_TestMode::ulwGetTimeInterval()
{
   tU8 u8TimeInterval = 0;
   if ( NULL != m_poDataPool )
   {
      m_poDataPool->u32dp_get(DPCLOCK__TIME_INTERVAL,&u8TimeInterval,
         sizeof(u8TimeInterval));
   }
   if (u8TimeInterval>0)
   {
      --u8TimeInterval;		//the return value is 0-2 for Model
   }

   return (ulword)u8TimeInterval;
}

/******************************************************************************
* FUNCTION:    clHSA_TestMode::vSetNewScreenDataID
*------------------------------------------------------------------------------
* DESCRIPTION: API to get the time interval value
*------------------------------------------------------------------------------
* PARAMETER:   -
* RETURNVALUE: ulword
******************************************************************************/
void clHSA_TestMode::vSetNewScreenDataID(ulword Screen_ID)
{
   if( NULL != pclHSI_TestMode )
   {
      if(Screen_ID == RADIO_STM)														//Different CM message to be sent on entering radio screen in STM
      {
         sMsg refMsg(NULL, CMMSG_SERVICE_MODE_TESTMODE_ON, NULL, NULL);				//Update the sMsg with required parameter
         pclHSI_TestMode->bExecuteMessage( refMsg );									//Trigger to the HSI_Testmode
      }
      else
      {
         sMsg refMsg (NULL, (tU32)CMMSG_TESTMODE_SCREEN_ID, (tU32)Screen_ID, NULL) ;	//Update the sMsg with required parameter
         pclHSI_TestMode->bExecuteMessage(refMsg);									//Trigger to the HSI_Testmode
      }
   }
}

/*****************************SXM DTM APIs START******************************/


/******************************************************************************
* FUNCTION:    clHSA_TestMode::vGetSXMServiceDTMMonData
*------------------------------------------------------------------------------
* DESCRIPTION: API to get SXM Service monitor data
*------------------------------------------------------------------------------
* PARAMETER:   ulword LineNo
* RETURNVALUE: GUI_String *out_result
*******************************************************************************/
void clHSA_TestMode::vGetSXMServiceDTMMonData(GUI_String *out_result, ulword LineNo)
{
   tU32 DPID=0;
   switch(LineNo)
   {
   case SXM_SERVICE_DTM_MONITOR_LINE_0:
      {
         DPID=DPDP_AUDIO__DIAGINFO_SIGQUALITY;
         bpstl::string param = "";
         dp.vGetString(DPID, param);

         GUI_String_vSetCStr(out_result, const_cast<char*>(param.c_str()));
      }
      break;
   case SXM_SERVICE_DTM_MONITOR_LINE_1:
      {
         DPID=DPDP_AUDIO__DIAGINFO_TUNERSTATUS;
         bpstl::string param = "";
         dp.vGetString(DPID, param);

         GUI_String_vSetCStr(out_result, const_cast<char*>(param.c_str()));
      }
      break;
   case SXM_SERVICE_DTM_MONITOR_LINE_2:
      {
         DPID=DPDP_AUDIO__DIAGINFO_ENSALOCKSTATUS;
         bpstl::string param = "";
         dp.vGetString(DPID, param);

         GUI_String_vSetCStr(out_result, const_cast<char*>(param.c_str()));
      }
      break;
   case SXM_SERVICE_DTM_MONITOR_LINE_3:
      {
         DPID=DPDP_AUDIO__DIAGINFO_ENSBLOCKSTATUS;
         bpstl::string param = "";
         dp.vGetString(DPID, param);

         GUI_String_vSetCStr(out_result, const_cast<char*>(param.c_str()));
      }
      break;
   case SXM_SERVICE_DTM_MONITOR_LINE_4:
      {
         bpstl::string BER = "";
         bpstl::string param1 = "";
         bpstl::string param2 = "";
         bpstl::string param3 = "";


         dp.vGetString(DPDP_AUDIO__DIAGINFO_BERS1,param1);
         dp.vGetString(DPDP_AUDIO__DIAGINFO_BERS2,param2);
         dp.vGetString(DPDP_AUDIO__DIAGINFO_BERT,param3);

         BER.append("S1:");
         BER.append(param1);
         BER.append(" ");
         BER.append("S2:");
         BER.append(param2);
         BER.append(" ");
         BER.append("T:");
         BER.append(param3);

         GUI_String_vSetCStr(out_result,(const tU8*)BER.c_str());

      }
      break;
   case SXM_SERVICE_DTM_MONITOR_LINE_5:
      {
         bpstl::string CN1 = "";
         bpstl::string param1 = "";
         bpstl::string param2 = "";

         dp.vGetString(DPDP_AUDIO__DIAGINFO_CTONS1A,param1);
         dp.vGetString(DPDP_AUDIO__DIAGINFO_CTONS1B,param2);

         CN1.append("S1A:");
         CN1.append(param1);
         CN1.append(" ");
         CN1.append("S1B:");
         CN1.append(param2);

         GUI_String_vSetCStr(out_result,(const tU8 *)CN1.c_str());
      }
      break;
   case SXM_SERVICE_DTM_MONITOR_LINE_6:
      {

         bpstl::string CN2 = "";
         bpstl::string param1 = "";
         bpstl::string param2 = "";

         dp.vGetString(DPDP_AUDIO__DIAGINFO_CTONS2A,param1);
         dp.vGetString(DPDP_AUDIO__DIAGINFO_CTONS2B,param2);

         CN2.append("S2A:");
         CN2.append(param1);
         CN2.append(" ");
         CN2.append("S2B:");
         CN2.append(param2);

         GUI_String_vSetCStr(out_result,(const tU8 *)CN2.c_str());
      }
      break;
   case SXM_SERVICE_DTM_MONITOR_LINE_7:
      {
         bpstl::string RS = "";
         bpstl::string param1 = "";
         bpstl::string param2 = "";
         bpstl::string param3 = "";


         dp.vGetString(DPDP_AUDIO__DIAGINFO_RSERRSWORDS,param1);
         dp.vGetString(DPDP_AUDIO__DIAGINFO_RSERRSATSYMB,param2);
         dp.vGetString(DPDP_AUDIO__DIAGINFO_RSERRTERRSYMB,param3);

         RS.append("W:");
         RS.append(param1);
         RS.append(" ");
         RS.append("S:");
         RS.append(param2);
         RS.append(" ");
         RS.append("T:");
         RS.append(param3);

         GUI_String_vSetCStr(out_result,(const tU8 *)RS.c_str());
      }
      break;
   case SXM_SERVICE_DTM_MONITOR_LINE_8:
      {
         DPID=DPDP_AUDIO__DIAGINFO_TUNERCARRFREQOFSET;
         bpstl::string param = "";
         dp.vGetString(DPID, param);

         GUI_String_vSetCStr(out_result, const_cast<char*>(param.c_str()));
      }
      break;
   case SXM_SERVICE_DTM_MONITOR_LINE_9:
      {
         DPID=DPDP_AUDIO__DIAGINFO_RSSI;
         bpstl::string param = "";
         dp.vGetString(DPID, param);

         GUI_String_vSetCStr(out_result, const_cast<char*>(param.c_str()));
      }
      break;
   case SXM_SERVICE_DTM_MONITOR_LINE_10:
      {
         DPID=DPDP_AUDIO__DIAGINFO_RECEIVERSTATE;
         bpstl::string param = "";
         dp.vGetString(DPID, param);

         GUI_String_vSetCStr(out_result, const_cast<char*>(param.c_str()));
      }
      break;
   case SXM_SERVICE_DTM_MONITOR_LINE_11:
      {
         bpstl::string OBERA = "";
         bpstl::string param1 = "";
         bpstl::string param2 = "";
         bpstl::string param3 = "";


         dp.vGetString(DPDP_AUDIO__DIAGINFO_OBERS1A,param1);
         dp.vGetString(DPDP_AUDIO__DIAGINFO_OBERS2A,param2);
         dp.vGetString(DPDP_AUDIO__DIAGINFO_OBERTA,param3);

         OBERA.append("S1:");
         OBERA.append(param1);
         OBERA.append(" ");
         OBERA.append("S2:");
         OBERA.append(param2);
         OBERA.append(" ");
         OBERA.append("T:");
         OBERA.append(param3);

         GUI_String_vSetCStr(out_result,(const tU8 *)OBERA.c_str());
      }
      break;
   case SXM_SERVICE_DTM_MONITOR_LINE_12:
      {
         bpstl::string OBERB = "";
         bpstl::string param1 = "";
         bpstl::string param2 = "";
         bpstl::string param3 = "";


         dp.vGetString(DPDP_AUDIO__DIAGINFO_OBERS1B,param1);
         dp.vGetString(DPDP_AUDIO__DIAGINFO_OBERS2B,param2);
         dp.vGetString(DPDP_AUDIO__DIAGINFO_OBERTB,param3);

         OBERB.append("S1:");
         OBERB.append(param1);
         OBERB.append(" ");
         OBERB.append("S2:");
         OBERB.append(param2);
         OBERB.append(" ");
         OBERB.append("T:");
         OBERB.append(param3);
         GUI_String_vSetCStr(out_result,(const tU8 *)OBERB.c_str());
      }
      break;
   case SXM_SERVICE_DTM_MONITOR_LINE_13:
      {
         bpstl::string ORS = "";
         bpstl::string param1 = "";
         bpstl::string param2 = "";

         dp.vGetString(DPDP_AUDIO__DIAGINFO_OBERTRBWRDERRT0A,param1);
         dp.vGetString(DPDP_AUDIO__DIAGINFO_OBERTRBWRDERRT0B,param2);

         ORS.append("0A:");
         ORS.append(param1);
         ORS.append(" ");
         ORS.append("0B:");
         ORS.append(param2);


         GUI_String_vSetCStr(out_result,(const tU8 *)ORS.c_str());
      }
      break;
   case SXM_SERVICE_DTM_MONITOR_LINE_14:
      {
         DPID=DPDP_AUDIO__DIAGINFO_CAPABILITY;
         bpstl::string param = "";
         dp.vGetString(DPID, param);

         GUI_String_vSetCStr(out_result, const_cast<char*>(param.c_str()));
      }
      break;
   case SXM_SERVICE_DTM_MONITOR_LINE_15:
      {
         DPID=DPDP_AUDIO__DIAGINFO_MODULEID;
         bpstl::string param = "";
         dp.vGetString(DPID, param);

         GUI_String_vSetCStr(out_result, const_cast<char*>(param.c_str()));
      }
      break;
   case SXM_SERVICE_DTM_MONITOR_LINE_16:
      {
         DPID=DPDP_AUDIO__DIAGINFO_HWREV;
         bpstl::string param = "";
         dp.vGetString(DPID, param);

         GUI_String_vSetCStr(out_result, const_cast<char*>(param.c_str()));
      }
      break;
   case SXM_SERVICE_DTM_MONITOR_LINE_17:
      {
         DPID=DPDP_AUDIO__DIAGINFO_SWREV;
         bpstl::string param = "";
         dp.vGetString(DPID, param);

         GUI_String_vSetCStr(out_result, const_cast<char*>(param.c_str()));
      }
      break;
   case SXM_SERVICE_DTM_MONITOR_LINE_18:
      {
         DPID=DPDP_AUDIO__DIAGINFO_SXIREV;
         bpstl::string param = "";
         dp.vGetString(DPID, param);

         GUI_String_vSetCStr(out_result, const_cast<char*>(param.c_str()));
      }
      break;
   case SXM_SERVICE_DTM_MONITOR_LINE_19:
      {
         DPID=DPDP_AUDIO__DIAGINFO_BBREV;
         bpstl::string param = "";
         dp.vGetString(DPID, param);

         GUI_String_vSetCStr(out_result, const_cast<char*>(param.c_str()));
      }
      break;
   case SXM_SERVICE_DTM_MONITOR_LINE_20:
      {
         DPID=DPDP_AUDIO__DIAGINFO_HDECREV;
         bpstl::string param = "";
         dp.vGetString(DPID, param);

         GUI_String_vSetCStr(out_result, const_cast<char*>(param.c_str()));
      }
      break;
   case SXM_SERVICE_DTM_MONITOR_LINE_21:
      {
         DPID=DPDP_AUDIO__DIAGINFO_RFREV;
         bpstl::string param = "";
         dp.vGetString(DPID, param);

         GUI_String_vSetCStr(out_result, const_cast<char*>(param.c_str()));
      }
      break;
   case SXM_SERVICE_DTM_MONITOR_LINE_22:
      {
         DPID=DPOSAL__VERSION_TESTMODE_BUILDVERSION_CUSTVERSTRING;
         bpstl::string param = "";
         dp.vGetString(DPID, param);

         GUI_String_vSetCStr(out_result, const_cast<char*>(param.c_str()));
      }
      break;
   case SXM_SERVICE_DTM_MONITOR_LINE_23:
      {
         DPID=DPDP_AUDIO__DIAGINFO_CGSDB;
         bpstl::string param = "";
         dp.vGetString(DPID, param);

         GUI_String_vSetCStr(out_result, const_cast<char*>(param.c_str()));
      }
      break;
   case SXM_SERVICE_DTM_MONITOR_LINE_24:
      {
         DPID=DPDP_AUDIO__DIAGINFO_FUELDB;
         bpstl::string param = "";
         dp.vGetString(DPID, param);

         GUI_String_vSetCStr(out_result, const_cast<char*>(param.c_str()));
      }
      break;
   case SXM_SERVICE_DTM_MONITOR_LINE_25:
      {
         DPID=DPDP_AUDIO__DIAGINFO_MOVIEDB;
         bpstl::string param = "";
         dp.vGetString(DPID, param);

         GUI_String_vSetCStr(out_result, const_cast<char*>(param.c_str()));
      }
      break;
   case SXM_SERVICE_DTM_MONITOR_LINE_26:
      {
         DPID=DPDP_AUDIO__DIAGINFO_TABWEATHERDB;
         bpstl::string param = "";
         dp.vGetString(DPID, param);

         GUI_String_vSetCStr(out_result, const_cast<char*>(param.c_str()));
      }
      break;
   case SXM_SERVICE_DTM_MONITOR_LINE_27:
      {
         DPID=DPDP_AUDIO__DIAGINFO_STOCKDB;
         bpstl::string param = "";
         dp.vGetString(DPID, param);

         GUI_String_vSetCStr(out_result, const_cast<char*>(param.c_str()));
      }
      break;
   case SXM_SERVICE_DTM_MONITOR_LINE_28:
      {
         DPID=DPDP_AUDIO__DIAGINFO_SAFEVIEWDB;
         bpstl::string param = "";
         dp.vGetString(DPID, param);

         GUI_String_vSetCStr(out_result, const_cast<char*>(param.c_str()));
      }
      break;
   case SXM_SERVICE_DTM_MONITOR_LINE_29:
      {
         DPID=DPDP_AUDIO__DIAGINFO_SMSVERSION;
         bpstl::string param = "";
         dp.vGetString(DPID, param);

         GUI_String_vSetCStr(out_result, const_cast<char*>(param.c_str()));
      }
      break;
   case SXM_SERVICE_DTM_MONITOR_LINE_30:
      {
         DPID=DPDP_AUDIO__DIAGINFO_SUBSTATUS;
         bpstl::string param = "";
         dp.vGetString(DPID, param);

         GUI_String_vSetCStr(out_result, const_cast<char*>(param.c_str()));
      }
      break;
   case SXM_SERVICE_DTM_MONITOR_LINE_31:
      {
         DPID=DPDP_AUDIO__DIAGINFO_REASONCODE;
         bpstl::string param = "";
         dp.vGetString(DPID, param);

         GUI_String_vSetCStr(out_result, const_cast<char*>(param.c_str()));
      }
      break;
   case SXM_SERVICE_DTM_MONITOR_LINE_32:
      {
         bpstl::string SuspendedDate= "";
         vGetSXMSuspendDateinLocaltime(SuspendedDate);
         GUI_String_vSetCStr(out_result, const_cast<char*>(SuspendedDate.c_str()));
      }
      break;

   default:
      break;
   }
}
/******************************************************************************
* FUNCTION:    clHSA_TestMode::u8GetSXMSettingsMenuData
*------------------------------------------------------------------------------
* DESCRIPTION: API to get SXM settings menu data
*------------------------------------------------------------------------------
* PARAMETER: ulword LineNo
* RETURNVALUE: ulword
*******************************************************************************/
ulword clHSA_TestMode::ulwGetSXMSettingsMenuData(ulword LineNo)
{
   tU32 DPID=0;
   tU8 u8RetValue=0;
   tU8 u8TrafficSubscription=0;
   tU8 u8DataServiceState=0;
   switch(LineNo)
   {
   case SXM_SETTINGS_MENU_LINE_0:
      {
         DPID=DPDP_AUDIO__DIAGINFO_SIGNALQUALITYSTATUS;
      }
      break;
   case SXM_SETTINGS_MENU_LINE_1:
      {
         DPID=DPDP_AUDIO__DIAGINFO_AUDIOSUBSTATUS;
      }
      break;
   case SXM_SETTINGS_MENU_LINE_2:
      {
         u8TrafficSubscription = ulwGetSXMTrafficServiceSubInfo();
         return u8TrafficSubscription;
      }
      break;
   case SXM_SETTINGS_MENU_LINE_3:
      {
         u8DataServiceState = ulwGetSXMDataServiceSubInfo();
         return u8DataServiceState;
      }
      break;

   default:
      break;
   }

   u8RetValue = dp.u8GetValue(DPID);
   return (ulword)u8RetValue;	
}

/******************************************************************************
* FUNCTION:     ulwGetSXMDataServiceSubInfo
* CLASS:        clHSA_TestMode
*-----------------------------------------------------------------------------
* DESCRIPTION:  To get dataservice subscription state
*-----------------------------------------------------------------------------
* PARAMETER:    tVoid
*
* RETURNVALUE:  ulword.
*****************************************************************************/
ulword clHSA_TestMode::ulwGetSXMDataServiceSubInfo()
{
   tU8 u8DataServiceState_FUEL       = dp.u8GetValue(DPDP_AUDIO__DATASERVICE_STATUS_FUEL);
   tU8 u8DataServiceState_MOVIES     = dp.u8GetValue(DPDP_AUDIO__DATASERVICE_STATUS_MOVIES);
   tU8 u8DataServiceState_TABWEATHER = dp.u8GetValue(DPDP_AUDIO__DATASERVICE_STATUS_TABWEATHER);
   tU8 u8DataServiceState_STOCKS     = dp.u8GetValue(DPDP_AUDIO__DATASERVICE_STATUS_STOCKS);
   tU8 u8DataServiceState_AGW        = dp.u8GetValue(DPDP_AUDIO__DATASERVICE_STATUS_AGW);


   if((DATASERVICE_SUBSCRIBED == u8DataServiceState_FUEL)||
      (DATASERVICE_SUBSCRIBED == u8DataServiceState_MOVIES)||
      (DATASERVICE_SUBSCRIBED == u8DataServiceState_TABWEATHER)||
      (DATASERVICE_SUBSCRIBED == u8DataServiceState_STOCKS)||
      (DATASERVICE_SUBSCRIBED == u8DataServiceState_AGW)
      )
   {
      return GUI_DATASERVICE_SUBSCRIBED;
   }
   else if( NO_ANTENNA ==u8DataServiceState_FUEL || NO_SIGNAL == u8DataServiceState_FUEL || DATASERVICE_NOT_AVAILABLE == u8DataServiceState_FUEL)
      return GUI_DATASERVICE_NOT_AVAILABLE;

   else if( DATASERVICE_NOT_SUBSCRIBED == u8DataServiceState_FUEL)
      return GUI_DATASERVICE_NOT_SUBSCRIBED;

   else
      return GUI_DATASERVICE_LOADING;

}
/******************************************************************************
* FUNCTION:     vGetXMRadioID
* CLASS:        clHSA_TestMode
*-----------------------------------------------------------------------------
* DESCRIPTION:  To get the Setting menu  Radio ID
*-----------------------------------------------------------------------------
* PARAMETER:    tVoid
*
* RETURNVALUE:  GUI_String *out_result
*****************************************************************************/
void clHSA_TestMode::vGetSXMRadioID(GUI_String *out_result)
{
   bpstl::string SXMRadioID = "";
   dp.vGetString(DPDP_AUDIO__DIAGINFO_RADIOID, SXMRadioID);
   GUI_String_vSetCStr(out_result, const_cast<char*>(SXMRadioID.c_str()));
}

/******************************************************************************
* FUNCTION:     vStartMethodforUPCLID
* CLASS:        clHSA_TestMode
*-----------------------------------------------------------------------------
* DESCRIPTION:  To do method start for UPCLID values
*-----------------------------------------------------------------------------
* PARAMETER:    tVoid
*
* RETURNVALUE:  tVoid.
*****************************************************************************/
void clHSA_TestMode::vStartMethodforUPCLID()
{
   tBool bWaitSync = FALSE;
   dp.vSetBoolValue( DPDP_AUDIO__DIAGINFO_WAIT_SYNC_FOR_DIAG, bWaitSync );
   if( NULL != pclHSI_CMSXM )
   {
      sMsg refMsg(NULL, CMMSG_SXM_UPCLID_VALUE,NULL, NULL);
      pclHSI_CMSXM->bExecuteMessage( refMsg );
   }
}

/******************************************************************************						
* FUNCTION:     blWaitSyncforSXMDiag
* CLASS:        clHSA_TestMode
*-----------------------------------------------------------------------------
* DESCRIPTION:  Returns False if GUI has to wait for data, True if the data is available
*-----------------------------------------------------------------------------
* PARAMETER:    tVoid
*
* RETURNVALUE:  tBool TRUE/FALSE
*****************************************************************************/
tbool clHSA_TestMode::blWaitSyncforSXMDiag()   
{
   tBool bWaitSyncForDiag = FALSE;
   bWaitSyncForDiag = dp.bGetValue(DPDP_AUDIO__DIAGINFO_WAIT_SYNC_FOR_DIAG);
   return bWaitSyncForDiag;
}

/******************************************************************************						
* FUNCTION:     vActivateSXMDTM
* CLASS:        clHSA_SXM_FUEL
*-----------------------------------------------------------------------------
* DESCRIPTION:  to do method start for activating DTM screen
*-----------------------------------------------------------------------------
* PARAMETER:    ulword Action
*
* RETURNVALUE:  tVoid
*****************************************************************************/
tVoid clHSA_TestMode::vActivateSXMDTM(ulword Action)   
{
   tU8 u8EnableDTM=Action;
   if( NULL != pclHSI_CMSXM )
   {
      sMsg refMsg(NULL, CMMSG_SXM_DTM_ENABLE,(tU32)u8EnableDTM, NULL);
      pclHSI_CMSXM->bExecuteMessage( refMsg );
   }

}

/******************************************************************************
* FUNCTION:    clHSA_TestMode::vClearSXMDTMFunctions
*------------------------------------------------------------------------------
* DESCRIPTION: API to get SXM settings menu data
*------------------------------------------------------------------------------
* PARAMETER: ulword LineNo
* RETURNVALUE: void
*******************************************************************************/
tVoid clHSA_TestMode::vClearSXMDTMFunctions(ulword LineNo)
{
   tU8 u8DTMFunction=LineNo;
   if( NULL != pclHSI_CMSXM )
   {
      sMsg refMsg(NULL, CMMSG_SXM_DTM_FUNCTION,(tU32)u8DTMFunction, NULL);
      pclHSI_CMSXM->bExecuteMessage( refMsg );
   }

}

/******************************************************************************
* FUNCTION:     ulwGetSXMSTMDataParam1
* CLASS:        clHSA_TestMode
*-----------------------------------------------------------------------------
* DESCRIPTION:  To get SXM service monitor information
*-----------------------------------------------------------------------------
* PARAMETER:    ulword LineNo
*
* RETURNVALUE:  ulword
*-----------------------------------------------------------------------------
* Date       |   Modification                         | Author
* 18.12.2012 |   Initial revision                     | Pallavi S
*****************************************************************************/
ulword clHSA_TestMode::ulwGetSXMSTMDataParam1(ulword LineNo)
{
   tU8 u8AntennaStatus = 0;
   tU8 u8TrafficSubscription = 0;
   tU8 u8DataServiceState=0;
   tU32 DPID=0;
   tU8 u8RetValue=0;


   switch(LineNo)
   {
   case STM_LINE_1:
      {
         u8AntennaStatus = ulwGetSXMAntennaStatus();
         return u8AntennaStatus;
      }
      break;
   case STM_LINE_2:
      {
         DPID=DPDP_AUDIO__DIAGINFO_SIGNALQUALITYSTATUS;
      }
      break;
   case STM_LINE_5:
      {
         DPID=DPDP_AUDIO__DIAGINFO_AUDIOSUBSTATUS;
      }
      break;
   case STM_LINE_6:
      {
         u8TrafficSubscription = ulwGetSXMTrafficServiceSubInfo();
         return u8TrafficSubscription;
      }
      break;
   case STM_LINE_7:
      {
         u8DataServiceState = ulwGetSXMDataServiceSubInfo();
         return u8DataServiceState;
      }
      break;
   case STM_LINE_8:
      {
         DPID=DPDP_AUDIO__DIAGINFO_MODULESUBSCRIPTION;
      }
      break;
   case STM_LINE_10:
      {
         DPID=DPDP_AUDIO__DATASERVICE_STATUS_CHANNELART;
         u8DataServiceState = ulwGetSXMServiceStatus(DPID);
         return u8DataServiceState;
      }
      break;
   case STM_LINE_11:
      {
         DPID=DPDP_AUDIO__DATASERVICE_STATUS_MOVIES;
         u8DataServiceState = ulwGetSXMServiceStatus(DPID);
         return u8DataServiceState;
      }
      break;
   case STM_LINE_12:
      {
         DPID=DPDP_AUDIO__DATASERVICE_STATUS_AGW;
         u8DataServiceState = ulwGetSXMServiceStatus(DPID);
         return u8DataServiceState;
      }
      break;
   case STM_LINE_13:
      {
         DPID=DPDP_AUDIO__DATASERVICE_STATUS_TABWEATHER;
         u8DataServiceState = ulwGetSXMServiceStatus(DPID);
         return u8DataServiceState;
      }
      break;
   case STM_LINE_14:
      {
         DPID=DPDP_AUDIO__DATASERVICE_STATUS_FUEL;
         u8DataServiceState = ulwGetSXMServiceStatus(DPID);
         return u8DataServiceState;
      }
      break;
   case STM_LINE_15:
      {
         DPID=DPDP_AUDIO__DATASERVICE_STATUS_STOCKS;
         u8DataServiceState = ulwGetSXMServiceStatus(DPID);
         return u8DataServiceState;
      }
      break;
   default:
      break;
   }

   u8RetValue = dp.u8GetValue(DPID);
   return (ulword)u8RetValue;	

}
/******************************************************************************
* FUNCTION:     vGetSXMSTMDataParam2
* CLASS:        clHSA_TestMode
*-----------------------------------------------------------------------------
* DESCRIPTION:  To get SXM service monitor information
*-----------------------------------------------------------------------------
* PARAMETER:    ulword LineNo
*
* RETURNVALUE:  GUI_String *out_result
*-----------------------------------------------------------------------------
* Date       |   Modification                         | Author
* 18.12.2012 |   Initial revision                     | Pallavi S
*****************************************************************************/
void clHSA_TestMode::vGetSXMSTMDataParam2(GUI_String *out_result, ulword LineNo)
{
   bpstl::string param = "";
   bpstl::string SuspendedDate= "";

   switch(LineNo)
   {
   case STM_LINE_3:
      {
         dp.vGetString(DPDP_AUDIO__DIAGINFO_UPCLID0, param);
         GUI_String_vSetCStr(out_result, const_cast<char*>(param.c_str()));
      }
      break;
   case STM_LINE_4:
      {
         dp.vGetString(DPDP_AUDIO__DIAGINFO_UPCLID1, param);
         GUI_String_vSetCStr(out_result, const_cast<char*>(param.c_str()));
      }
      break;
   case STM_LINE_9:
      {

         vGetSXMSuspendDateinLocaltime(SuspendedDate);
         GUI_String_vSetCStr(out_result, const_cast<char*>(SuspendedDate.c_str()));
      }
      break;
   default:
      break;
   }
}

/******************************************************************************
* FUNCTION:     ulwGetSXMTrafficServiceSubInfo
* CLASS:        clHSA_TestMode
*-----------------------------------------------------------------------------
* DESCRIPTION:  To get traffic subscription state
*-----------------------------------------------------------------------------
* PARAMETER:    tVoid
*
* RETURNVALUE:  ulword.
*****************************************************************************/
ulword clHSA_TestMode::ulwGetSXMTrafficServiceSubInfo()
{
   tU8 u8TrafficSubscription = dp.u8GetValue(DPDP_AUDIO__DATASERVICE_STATUS_TRAFFIC);
   switch(u8TrafficSubscription)
   {
   case SUB_UNKNOWN:
      {
         return TRAFFIC_LOADING;
      }
      break;
   case DATASERVICE_SUBSCRIBED:
      {
         return TRAFFIC_SUBSCRIBED;
      }
      break;
   case DATASERVICE_NOT_SUBSCRIBED:
      {
         return TRAFFIC_NOT_SUBSCRIBED;
      }
      break;
   case DATASERVICE_NOT_AVAILABLE:
      {
         return TRAFFIC_NOT_AVAILABLE;
      }
      break;
   case NO_ANTENNA:
      {
         return TRAFFIC_NOT_AVAILABLE;
      }
      break;
   case NO_SIGNAL:
      {
         return TRAFFIC_NOT_AVAILABLE;
      }
      break;

   default:
      return TRAFFIC_LOADING;
      break;
   }
}

/******************************************************************************
* FUNCTION:     ulwGetSXMAntennaStatus
* CLASS:        clHSA_TestMode
*-----------------------------------------------------------------------------
* DESCRIPTION:  To get antenna status
*-----------------------------------------------------------------------------
* PARAMETER:    tVoid
*
* RETURNVALUE:  ulword.
*****************************************************************************/
ulword clHSA_TestMode::ulwGetSXMAntennaStatus()
{
   tU8 u8AntennaStatus = dp.u8GetValue(DPDP_AUDIO__DIAGINFO_ANTENNASTATUS);
   switch(u8AntennaStatus)
   {
   case FC_SXM_DIAG_ANT_UNSUPPORTED:
      {
         return FC_SXM_DIAG_ANT_UNSUPPORTED;
      }
      break;
   case FC_SXM_DIAG_ANT_DETECTED:
      {
         return FC_SXM_DIAG_ANT_DETECTED;
      }
      break;
   case FC_SXM_DIAG_ANT_NOT_DETECTED:
      {
         return FC_SXM_DIAG_ANT_NOT_DETECTED;
      }
      break;
   case FC_SXM_DIAG_ANT_SHORTED:
      {
         return FC_SXM_DIAG_ANT_SHORTED;
      }
      break;
   case FC_SXM_DIAG_ANT_UNKNOWN:
      {
         return FC_SXM_DIAG_ANT_UNSUPPORTED;
      }
      break;

   default:
      return FC_SXM_DIAG_ANT_UNSUPPORTED;
      break;
   }
}

/******************************************************************************
* FUNCTION:     vGetSXMSuspendDateinLocaltime
* CLASS:        clHSA_TestMode
*-----------------------------------------------------------------------------
* DESCRIPTION:  To get antenna status
*-----------------------------------------------------------------------------
* PARAMETER:    tVoid
*
* RETURNVALUE:  bpstl::string& SuspendedDate.
*****************************************************************************/
void clHSA_TestMode::vGetSXMSuspendDateinLocaltime(bpstl::string& SuspendedDate)
{
   tU16 u16Year=0;
   tU8 u8Month=0;
   tU8 u8Day=0;
   tChar pacSelectedSmsReceivedDate[25]  = "\0";

   tU8 u8ModuleSub = dp.u8GetValue(DPDP_AUDIO__DIAGINFO_MODULESUBSCRIPTION);
   if(AUDIO_SUBSCRIBED == u8ModuleSub || AUDIO_NOT_SUBSCRIBED == u8ModuleSub)
   {
      SuspendedDate.assign(" ");
   }
   else
   {

      tU8 u8CurrDateFormat = 0;
      u8CurrDateFormat = dp.u8GetValue(DPCLOCK__DATE_FORMAT);

      u16Year = dp.u16GetValue(DPDP_AUDIO__DIAGINFO_SUSPENDEDDATE_YEAR);
      u8Month = dp.u8GetValue(DPDP_AUDIO__DIAGINFO_SUSPENDEDDATE_MONTH);
      u8Day = dp.u8GetValue(DPDP_AUDIO__DIAGINFO_SUSPENDEDDATE_DAY);



      switch (u8CurrDateFormat)
      {
      case DD_MM_YYYY_SLASH:
         {
            UTF8_s32SaveNPrintFormat(pacSelectedSmsReceivedDate,25, "%02d/%02d/%4d",u8Day,u8Month, u16Year);
            break;
         }
      case MM_DD_YYYY_SLASH:
         {
            UTF8_s32SaveNPrintFormat(pacSelectedSmsReceivedDate,25, "%02d/%02d/%4d",u8Month,u8Day,u16Year);
            break;
         }
      case DD_MM_YYYY_DOT:
         {
            UTF8_s32SaveNPrintFormat(pacSelectedSmsReceivedDate,25, "%02d.%02d.%4d",u8Day,u8Month, u16Year);
            break;
         }
      case DD_MM_YYYY_HYPHEN:
         {
            UTF8_s32SaveNPrintFormat(pacSelectedSmsReceivedDate,25, "%02d-%02d-%4d",u8Day,u8Month, u16Year);
            break;
         }
      case YYYY_DD_MM_HYPHEN:
         {
            UTF8_s32SaveNPrintFormat(pacSelectedSmsReceivedDate,25, "%4d-%02d-%02d",u16Year,u8Day,u8Month);
            break;
         }
      default:
         break;
      }

      SuspendedDate.assign(pacSelectedSmsReceivedDate);
   }
}

/******************************************************************************
* FUNCTION:     vToggleExternalDiagMode
* CLASS:        clHSA_TestMode
*-----------------------------------------------------------------------------
* DESCRIPTION:  To toggle external Diag mode
*-----------------------------------------------------------------------------
* PARAMETER:    void
*
* RETURNVALUE:  void.
*****************************************************************************/
void clHSA_TestMode::vToggleExternalDiagMode()
{
    if( NULL != pclHSI_CMSXM )
    {
        sMsg refMsg (OSAL_NULL,
            (tU32)CMMSG_SXM_TOGGLE_EXTERNAL_DIAG_MODE,0,0) ;
        pclHSI_CMSXM->bExecuteMessage( refMsg );
    }
}
/******************************************************************************
* FUNCTION:     blExternalDiagModeState
* CLASS:        clHSA_TestMode
*-----------------------------------------------------------------------------
* DESCRIPTION:  To get the state external Diag  State
*-----------------------------------------------------------------------------
* PARAMETER:    void
*
* RETURNVALUE:  tBool.
*****************************************************************************/
tbool clHSA_TestMode::blExternalDiagModeState()
{
    if( NULL != pclHSI_CMSXM )
    {
        return pclHSI_CMSXM->bGetSXMExternalDiagModeState();
    }
    else
        return FALSE;
}
/*-----------------------------------------------------------------------------
* DESCRIPTION:  To get the state of popup in DTM function screen 
*-----------------------------------------------------------------------------*/
tbool clHSA_TestMode::blGetSxmDTMPopupStatus()
{
   return dp.bGetValue(DPDP_AUDIO__DIAGINFO_POPUP_FOR_DTM);
}
/******************************************************************************
* FUNCTION    : clHSA_TestMode::ulwGetCounrty()
*------------------------------------------------------------------------------
* DESCRIPTION : The API is used to get the country information.
*------------------------------------------------------------------------------
* PARAMETER   : -
*------------------------------------------------------------------------------
* RETURNVALUE : ulword
*------------------------------------------------------------------------------
*------------------------------------------------------------------------------
* HISTORY :
* DATE :     | AUTHOR           | COMMENTS
*------------------------------------------------------------------------------
* 22-04-2013 | Sapna Suresha | Initial Version
******************************************************************************/
ulword clHSA_TestMode::ulwGetCountry()
{
    tU16 u16CountryCode = 0;
    if ( NULL != m_poDataPool )
    {
        m_poDataPool->u32dp_get(DPNAVI__ISO_COUNTRY,&u16CountryCode,
        sizeof(u16CountryCode));
    }
    switch(u16CountryCode)
    {
    case(fi_tcl_e16_ISOCountryCode::FI_EN_ISO_ALPHA_3_UNDEFINED):
        return EN_CON_UNDEFINED;
    case(fi_tcl_e16_ISOCountryCode::FI_EN_ISO_ALPHA_3_DEU):
        return EN_CON_GERMANY;
    case(fi_tcl_e16_ISOCountryCode::FI_EN_ISO_ALPHA_3_GBR):
         return EN_CON_UK;
    case(fi_tcl_e16_ISOCountryCode::FI_EN_ISO_ALPHA_3_AUS):
         return EN_CON_AUSTRALIA;
    case(fi_tcl_e16_ISOCountryCode::FI_EN_ISO_ALPHA_3_ITA):
         return EN_CON_ITALY;
    case(fi_tcl_e16_ISOCountryCode::FI_EN_ISO_ALPHA_3_CZE):
         return EN_CON_CZECH_REPUBLIC;
    case(fi_tcl_e16_ISOCountryCode::FI_EN_ISO_ALPHA_3_NLD):
         return EN_CON_NETHERLANDS;
    case(fi_tcl_e16_ISOCountryCode::FI_EN_ISO_ALPHA_3_PRT):
         return EN_CON_PORTUGAL;
    case(fi_tcl_e16_ISOCountryCode::FI_EN_ISO_ALPHA_3_BRA):
        return EN_CON_BRAZIL;
    case(fi_tcl_e16_ISOCountryCode::FI_EN_ISO_ALPHA_3_RUS):
        return EN_CON_RUSSIA;
    case(fi_tcl_e16_ISOCountryCode::FI_EN_ISO_ALPHA_3_UKR):
        return EN_CON_UKRAINE;
    case(fi_tcl_e16_ISOCountryCode::FI_EN_ISO_ALPHA_3_TUR):
        return EN_CON_TURKEY;
    case(fi_tcl_e16_ISOCountryCode::FI_EN_ISO_ALPHA_3_USA):
        return EN_CON_USA;
    case(fi_tcl_e16_ISOCountryCode::FI_EN_ISO_ALPHA_3_CAN):
        return EN_CON_CANADA;
    case(fi_tcl_e16_ISOCountryCode::FI_EN_ISO_ALPHA_3_FRA):
        return EN_CON_FRANCE;
    case(fi_tcl_e16_ISOCountryCode::FI_EN_ISO_ALPHA_3_MEX):
        return EN_CON_MEXICO;
    case(fi_tcl_e16_ISOCountryCode::FI_EN_ISO_ALPHA_3_ESP):
        return EN_CON_SPAIN;
    case(fi_tcl_e16_ISOCountryCode::FI_EN_ISO_ALPHA_3_DNK):
        return EN_CON_DENMARK;
    case(fi_tcl_e16_ISOCountryCode::FI_EN_ISO_ALPHA_3_SWE):
        return EN_CON_SWEDEN;
    case(fi_tcl_e16_ISOCountryCode::FI_EN_ISO_ALPHA_3_FIN):
        return EN_CON_FINLAND;
    case(fi_tcl_e16_ISOCountryCode::FI_EN_ISO_ALPHA_3_NOR):
        return EN_CON_NORWAY;
    case(fi_tcl_e16_ISOCountryCode::FI_EN_ISO_ALPHA_3_POL):
        return EN_CON_POLAND;
    case(fi_tcl_e16_ISOCountryCode::FI_EN_ISO_ALPHA_3_SVK):
        return EN_CON_SLOVAKIA;
    case(fi_tcl_e16_ISOCountryCode::FI_EN_ISO_ALPHA_3_HUN):
        return EN_CON_HUNGARY;
    case(fi_tcl_e16_ISOCountryCode::FI_EN_ISO_ALPHA_3_GRC):
        return EN_CON_GREECE;
    case(fi_tcl_e16_ISOCountryCode::FI_EN_ISO_ALPHA_3_SAU):
        return EN_CON_SAUDI_ARABIA;
    case(fi_tcl_e16_ISOCountryCode::FI_EN_ISO_ALPHA_3_THA):
        return EN_CON_THAILAND;
    default:
    	return EN_CON_UNDEFINED;
     }
}
/******************************************************************************
* FUNCTION    : clHSA_TestMode::ulwGetGender()
*------------------------------------------------------------------------------
* DESCRIPTION : The API is used to get the gender information in the language
* screen of DTM.
*------------------------------------------------------------------------------
* PARAMETER   : -
*------------------------------------------------------------------------------
* RETURNVALUE : ulword
*------------------------------------------------------------------------------
*------------------------------------------------------------------------------
* HISTORY :
* DATE :     | AUTHOR           | COMMENTS
*------------------------------------------------------------------------------
* 22-04-2013 | Sapna Suresha | Initial Version
******************************************************************************/
ulword clHSA_TestMode::ulwGetGender()
{
	tU8 u8GenderInfo =0;
	if ( NULL != m_poDataPool )
	    {
	        m_poDataPool->u32dp_get(DPSDS__SDS_SPEAKER_GENDER,&u8GenderInfo,
	        sizeof(u8GenderInfo));
	    }
	return (ulword)u8GenderInfo;
}
/******************************************************************************
* FUNCTION    : clHSA_TestMode::vSpellerCharacterDelete()
*------------------------------------------------------------------------------
* DESCRIPTION :The API used to delete speller character
*------------------------------------------------------------------------------
* PARAMETER   : -
*------------------------------------------------------------------------------
* RETURNVALUE : void
*------------------------------------------------------------------------------
*------------------------------------------------------------------------------
* HISTORY :
* DATE :     | AUTHOR           | COMMENTS
*------------------------------------------------------------------------------
*02-05-2013 | Sapna Suresha | Initial Version
******************************************************************************/
void clHSA_TestMode::vSpellerCharacterDelete()
{
    tU16 U16Charcount= 0 ;

    if (m_poDataPool != NULL)
    {
        tU32 u32Size = m_poDataPool->u32dp_getElementSize(DPDAB__SPELLER_SELECTED_CHAR);
        tChar pcSpellerName_Delete[FOUR_CHAR_LEN] = {'\0'};
        m_poDataPool->u32dp_set( DPDAB__SPELLER_SELECTED_CHAR,
        pcSpellerName_Delete,u32Size );

        m_poDataPool->u32dp_set( DPDAB__SPELLER_CHARACTER_COUNT,
        &U16Charcount,sizeof(U16Charcount));
    }
}
/******************************************************************************
* FUNCTION    : clHSA_TestMode::TriggerNavSamplePrompt()
*------------------------------------------------------------------------------
* DESCRIPTION :The API trigger Navigation sample prompt
*------------------------------------------------------------------------------
* PARAMETER   : -
*------------------------------------------------------------------------------
* RETURNVALUE : void
*------------------------------------------------------------------------------
* HISTORY :
* DATE :     | AUTHOR           | COMMENTS
*------------------------------------------------------------------------------
*31-05-2013 | Sapna Suresha | Initial Version
******************************************************************************/
void clHSA_TestMode::vTriggerNavSamplePrompt()
{
   if(pclHSI_Sound!= NULL)
   {
      sMsg refMsg (NULL,(tU32)CMMSG_SOUND_START_NAVPHRASE,NULL,NULL) ;
      pclHSI_Sound->bExecuteMessage( refMsg );
   }
}
/******************************************************************************
* FUNCTION:     ulwGetSXMServiceStatus
* CLASS:        clHSA_TestMode
*-----------------------------------------------------------------------------
* DESCRIPTION:  To get dataservice subscription state
*-----------------------------------------------------------------------------
* PARAMETER:    DPID
*
* RETURNVALUE:  ulword.
*****************************************************************************/
ulword clHSA_TestMode::ulwGetSXMServiceStatus(ulword DPID)
{
   tU8 u8RetValue = dp.u8GetValue(DPID);

   if(DATASERVICE_SUBSCRIBED == u8RetValue)
   {
      return GUI_DATASERVICE_SUBSCRIBED;
   }
   else if( NO_ANTENNA ==u8RetValue || NO_SIGNAL == u8RetValue || DATASERVICE_NOT_AVAILABLE == u8RetValue)
      return GUI_DATASERVICE_NOT_AVAILABLE;

   else if( DATASERVICE_NOT_SUBSCRIBED == u8RetValue )
      return GUI_DATASERVICE_NOT_SUBSCRIBED;

   else
      return GUI_DATASERVICE_LOADING;
}


/**
Method : ulwBTDevAudioCodecUsed
Returns the Audio Codec Used 
Possible return values are : PHONE_CALL_NBS = 0, PHONE_CALL_WBS = 1, SIRI_NBS = 2, SIRI_WBS = 3, DEFAULT_VA_NBS = 4, DEFAULT_VA_WBS = 5, IDLE = 6
**/

ulword clHSA_TestMode::ulwBTDevAudioCodecUsed()
{
   tU8 u8OperatingAudioCodec = GUI_PHONE_OPERATING_AUDIO_CODEC_IDLE;

   if (m_poDataPool != NULL)
   {
      m_poDataPool->u32dp_get( 
         DPTELEPHONE__OPERATING_AUDIO_CODEC,
         &u8OperatingAudioCodec, 
         sizeof(u8OperatingAudioCodec)); 
   }

   if(AUDIO_CODEC_IDLE == u8OperatingAudioCodec )
   {
      u8OperatingAudioCodec = GUI_PHONE_OPERATING_AUDIO_CODEC_IDLE;
   }

   if(m_poTrace)
   {	
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
         TR_CLASS_HMI_HSA_SYSTEM_TESTMODE,
         (tU16)(HSA_API_SET_BT_DEV_OPERATING_AUDIO_CODEC),
         1,
         &u8OperatingAudioCodec);
   }

   return u8OperatingAudioCodec;
}
