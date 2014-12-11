/******************************************************************************
* FILE:         HSI_DAPIHelper.cpp
* PROJECT:      Ford
* SW-COMPONENT: 
*----------------------------------------------------------------------
*
* DESCRIPTION: Navi Client 
*               
*----------------------------------------------------------------------
* COPYRIGHT:    (c) 2004 Robert Bosch GmbH, Hildesheim
* HISTORY:      
* Date      | Author          | Modification
* 27.10.06  | RBIN(EDI1)jeevan | initial version
*
******************************************************************************/


// precompiled header, should always be the first include
#include "precompiled.hh"

#ifndef HMI_NAVSERVER_HANDLER_H
#include "FiNavi/Hmi_NavServerHandler.h"
#endif //HMI_NAVSERVER_HANDLER_H

#include "CompMan/clHSI_CMNavi.h"
// see precompiled.hh		#include "../CompMan/clhsi_cmmngr.h"

/* To include Data update reminder popup state machine defines */
#include "fiClock/HSI_CLOCK_DEFINES.h"

/* To get the static object of clHSAEventAdapter class.for MMS 235619*/
#include "HSA_EventAdapter/clHSAEventAdapter.h"

/******************************************************************************
*
* FUNCTION   : vHandleOnUpdateMediaStateOfDevice() 
* 
* DESCRIPTION: Handle function for vHandleOnUpdateMediaStateOfDevice()
*
* PARAMETER  : none
*
* RETURNVALUE: none
*
******************************************************************************/

tVoid clHmiNavServerHandler::vHandleOnUpdateMediaStateOfDevice(tU8 DeviceName)
{
   vTrace(NAVI_ON_HANDLE_MEDIA_DEVICESTATE);

   if((pOFi_Navi != NULL) && (poIDAPI != NULL) && (pOFi_Navi->m_poDataPool !=NULL) &&
      (poIEnavi->bIsNavServerServiceAvailable(CCA_C_U16_SRV_RESOURCE_INFORMATION) ))
   {
      switch(oDAPISinkImplementation.enMediaDeviceStatus)
      {
      case ENAVI_PROCESS_RESULT_OK:
         {
            tU8 u8NavServerStatus =  0;

            vTrace(NAVI_ENAVI_PROCESS_RESULT_OK);

            pOFi_Navi->m_poDataPool->u32dp_get( DPVC__NAV_STATUS, &u8NavServerStatus,
               sizeof(tU8));

            poIEnavi->vLockDataReception();
            enServerState = poIEnavi->enGetEnaviServerState();
            tBool bIsSDMediaUnKnown = poIDAPI->bIsMediaStateDataUnknown(HSI_C_STRING_DEVICE_SDCARD);
            enSDValidity = poIDAPI-> enCheckDataMedium(HSI_C_STRING_DEVICE_SDCARD);
            tBool SDAvailable = poIDAPI->bIsAMediumFullyAvailable(HSI_C_STRING_DEVICE_SDCARD);
            poIEnavi->vUnlockDataReception();

            // Update HMI with available navi media

            if (DeviceName == EN_DEFAULT_DEVICE_CD)
               vUpdateMediumStatus(OSAL_C_STRING_DEVICE_CACHEDCDFS);
            else if (DeviceName == EN_DEFAULT_DEVICE_SD)
               vUpdateMediumStatus(HSI_C_STRING_DEVICE_SDCARD);

            if ((enServerState == ENAVISERVER_DATA_CARRIER_REQUESTED)||
               (enServerState == ENAVISERVER_STARTING_UP))

            {
               vCheckMediumValidity();

               tBool DeviceStatus = FALSE;

               if (!bIsSDMediaUnKnown)
               {
                  if(enSDValidity != DATAMEDIUM_VALID)
                  {
                     vNoMediumStopRouteGuidance();
                  }
                  else if ((enSDValidity == DATAMEDIUM_VALID) && 
                     (SDAvailable))
                  {
                     poIEnavi->vChangeStartupDeviceName(HSI_C_STRING_DEVICE_SDCARD);

                     DeviceStatus = EN_DEFAULT_DEVICE_SD;
                     pOFi_Navi->m_poDataPool->u32dp_set( DPNAVI__UPDATE_MEDIA_STATUS ,
                        &DeviceStatus,
                        sizeof(DeviceStatus)
                        );
                     if(pOTracePointer != NULL)
                        pOTracePointer->vTrace(TR_LEVEL_HMI_INFO, TR_CLASS_HSI_NAV_MSG, 
                        "SD card is valid , change startup device name to SD Card");
                     tBool bNoSource= FALSE;
                     pOFi_Navi->m_poDataPool->u32dp_set( DPNAVI__SEAMLESS_NO_SOURCE_AVAILABLE, 
                        &bNoSource ,  sizeof(bNoSource) );
                  }
               }
            }
            else if(enServerState == ENAVISERVER_NORMAL)

            {
               tU8 u8NavAppStatus = EN_HSI_NAV_SERVER_STATUS_INVALID;
               tBool bIsNaviStartedBefore= FALSE;


               pOFi_Navi->m_poDataPool->u32dp_get( DPNAVI__SEAMLESS_NAVI_STARTED ,
                  &bIsNaviStartedBefore,sizeof(bIsNaviStartedBefore));

               pOFi_Navi->m_poDataPool->u32dp_get( DPNAVI__NAV_APP_STATUS, 
                  &u8NavAppStatus,sizeof(u8NavAppStatus));

               if(pOTracePointer != NULL)
                  pOTracePointer->vTrace(TR_LEVEL_HMI_INFO, TR_CLASS_HSI_NAV_MSG, 
                  "SD Status: %d",enSDValidity);

               if (enSDValidity == DATAMEDIUM_VALID)
               {
                  if ((!bIsNaviStartedBefore)||(u8NavAppStatus != EN_HSI_NAV_SERVER_STATUS_OK))
                  {
                     vHMIInitialization ();
                  }
               }

               poIEnavi->vLockDataReception();
               fi_tcl_e8_DapDataAvailable::tenType  enDefaultDatasetAvailability = 
                  poIDAPI->enGetDefaultDatasetAvailability();
               poIEnavi->vUnlockDataReception();

               vCheckMediumValidity();

               if ((fi_tcl_e8_DapDataAvailable::FI_EN_DAP_ENDATAFULLAVAILABLE
                  == enDefaultDatasetAvailability)&&
                  (enSDValidity == DATAMEDIUM_VALID))
               {
                  tBool bNoSource= FALSE;
                  pOFi_Navi->m_poDataPool->u32dp_set( DPNAVI__SEAMLESS_NO_SOURCE_AVAILABLE, 
                     &bNoSource ,  sizeof(bNoSource) );

                  vCheckMediumVersions();

                  if(pOTracePointer != NULL)
                     pOTracePointer->vTrace(TR_LEVEL_HMI_INFO, TR_CLASS_HSI_NAV_MSG, 
                     "DAPI->UpdateofMediaState CD has valid nav,check region versions for copy");
               }
               else
               {
                  vCheckMediumVersions();
               }
            }
         }
         break;
      case ENAVI_PROCESS_RESULT_INVALID:
         {
            vTrace(NAVI_ENAVI_PROCESS_RESULT_INVALID);
         }
         break;

      case ENAVI_PROCESS_BUSY:
         {
            vTrace(NAVI_ENAVI_PROCESS_RESULT_BUSY);
         }
         break;

      default:
         break;
      }
   }
   else
      if(pOTracePointer != NULL)
         pOTracePointer->vTrace(TR_LEVEL_HMI_INFO, TR_CLASS_HSI_NAV_MSG, 
         "HMINAVSERVER SERVICES ARE NOT AVAILABLE");
}

/******************************************************************************
*
* FUNCTION   : vHandleOnUpdateDefaultDataset() 
* 
* DESCRIPTION: 
*           
* PARAMETER  : none
*
* RETURNVALUE: tVoid
*
******************************************************************************/
tVoid clHmiNavServerHandler::vHandleOnUpdateDefaultDataset() 
{

   // Checking for CD and SD
   if(oDAPISinkImplementation.enDatasetStatus == ENAVI_PROCESS_RESULT_OK)
   {
      //vRefreshDescriptors();
      if( ( poIEnavi != NULL ) && (poIDAPI != NULL))
      {
         poIEnavi->vLockDataReception();

         tU32 u32DefaultDatasetMediaID = poIDAPI->u32GetDefaultDatasetMediaID();
         tU32 u32DefaultDatasetID      = poIDAPI->u32GetDefaultDatasetID();
         fi_tcl_e8_DapDataAvailable::tenType  enDefaultDatasetAvailability = poIDAPI->enGetDefaultDatasetAvailability();

         poIEnavi->vUnlockDataReception();


         if( (ENAVI_INVALID_MEDIA_ID  != u32DefaultDatasetMediaID) && 
            (ENAVI_INVALID_DATASET_ID  != u32DefaultDatasetID) &&
            (fi_tcl_e8_DapDataAvailable::FI_EN_DAP_ENDATAFULLAVAILABLE  == enDefaultDatasetAvailability)
            )
         {  
            tU32 u32OldDatasetID = 0;
            tU8 u8DURPState = 0;  		//Data update reminder state

            if(pOFi_Navi->m_poDataPool != NULL)
            {
               pOFi_Navi->m_poDataPool->u32dp_get( DPSYSTEM__DUR_NAVI_DATASET_ID, 
                  &u32OldDatasetID ,  sizeof(u32OldDatasetID) );

               if(u32DefaultDatasetID != u32OldDatasetID)
               {
                  // set the dataset of the new medium (sd card) inserted
                  pOFi_Navi->m_poDataPool->u32dp_set( DPSYSTEM__DUR_NAVI_DATASET_ID, 
                     &u32DefaultDatasetID ,  sizeof(u32DefaultDatasetID) );

                  //get the data update reminder state
                  pOFi_Navi->m_poDataPool->u32dp_get( DPSYSTEM__DUR_STATE, 
                     &u8DURPState,  sizeof(u8DURPState) );

                  // if the ign_on state is over set one year wait time to show the data update popup
                  if(u8DURPState != (tU8)DUR_STATE0_COUNT_IGN_ON  && u8DURPState != (tU8)DUR_STATE6_POPUP_DISABLED )
                  {
                     u8DURPState = (tU8)DUR_STATE1_SET_START_DATE;

                     // reset the one year wait time from now onward.
                     pOFi_Navi->m_poDataPool->u32dp_set( DPSYSTEM__DUR_STATE, 
                        &u8DURPState ,  sizeof(u8DURPState) );
                  }
               }
            }

            if(EN_RESTART_RG_BACKUP == m_u8ResumeRGDBChanged)
            {
               /** If database is changed Route Calculation will be stopped
               by HMINAVSERVER so wait till stopRC is OK */

               m_u8ResumeRGDBChanged = EN_RESTART_RG_OK;

               if(pOTracePointer != NULL)
                  pOTracePointer->vTrace(TR_LEVEL_HMI_INFO, TR_CLASS_HSI_NAV_MSG, 
                  "Update Def Dataset %d",m_u8ResumeRGDBChanged);
            }
         }
      }
   }
}

/******************************************************************************
*
* FUNCTION   : vUpdateMediumStatus() 
* 
* DESCRIPTION:  Check SD Card or CD Validity
*
* PARAMETER  : none
*
* RETURNVALUE: none
*
******************************************************************************/

tVoid clHmiNavServerHandler :: vUpdateMediumStatus(const fi_tclString &oMediaName)
{
   if( (pOFi_Navi != NULL) && (poIDAPI != NULL) &&
      (pOFi_Navi-> m_poDataPool != NULL))
   {

      clHSI_CMNAVI*    pclHSI_CMNavi  = NULL;
      clHSI_CMMngr*    pclHSI_CMManager = NULL;
      pclHSI_CMManager = clHSI_CMMngr::pclGetInstance();
      pclHSI_CMNavi    = dynamic_cast <clHSI_CMNAVI*>
         (pclHSI_CMManager->pHSI_BaseGet((tU8)EN_HSI_NAVI));

      tU8 DeviceStatus = EN_DEFAULT_DEVICE_CD;

      tenDataMediumValidity enMediumValidity = DATAMEDIUM_INVALID;// initialization
      fi_tcl_e8_DapMediaType::tenType enMediaType = fi_tcl_e8_DapMediaType::FI_EN_DAP_ENMEDIANONE;

      if (poIEnavi != NULL)
      {
         poIEnavi->vLockDataReception();
         enMediumValidity = poIDAPI->enCheckDataMedium(oMediaName);
         enServerState = poIEnavi->enGetEnaviServerState();
         enMediaType = poIDAPI->enGetMediaType( oMediaName );
         poIEnavi->vUnlockDataReception();
      }

      pOFi_Navi->m_poDataPool->u32dp_get( DPNAVI__UPDATE_MEDIA_STATUS,
         &DeviceStatus, sizeof(DeviceStatus));

      tBool bData = FALSE;
      tU32 u32DatapoolId = 0;

      if(oMediaName == OSAL_C_STRING_DEVICE_CACHEDCDFS)
      {
         u32DatapoolId = DPNAVI__NAVSRC_CD_VALID;
      }
      else if(oMediaName == HSI_C_STRING_DEVICE_SDCARD)
      {
         u32DatapoolId = DPNAVI__NAVSRC_SD_VALID;
      }

      if(pOTracePointer != NULL)
      {
         pOTracePointer->vTrace(TR_LEVEL_HMI_INFO, TR_CLASS_HSI_NAV_MSG, 
            "DAPI->vUpdateMediumStatus Validity: %d", enMediumValidity);
         pOTracePointer->vTrace(TR_LEVEL_HMI_INFO, TR_CLASS_HSI_NAV_MSG, 
            "DAPI->vUpdateMediumStatus Type: %d", enMediaType);
      }

      if( oMediaName == OSAL_C_STRING_DEVICE_CACHEDCDFS )
      {
         switch(enMediumValidity)
         {
         case DATAMEDIUM_VALID:
         case DATAMEDIUM_NOT_INSERTED:
         case DATAMEDIUM_VERSION_INCOMPATIBLE_WITH_NAVSERVER:
         case DATAMEDIUM_INVALID:
         case DATAMEDIUM_TEMP_NOT_ACCESSIBLE:
         case DATAMEDIUM_INCORRECT:
         case DATAMEDIUM_UNKNOWN:
            {
               bData = FALSE;
               vTrace(NAVI_CD_NAV_SRC_INVALID);
               break;
            }
         default:
            break;
         }
      }
      else if(oMediaName == HSI_C_STRING_DEVICE_SDCARD)
      {
         switch(enMediumValidity)
         {
         case DATAMEDIUM_VALID:
            {
               /*Show Loading message only if all datacarriers were ejected during runtime */
               if (( bPreviousSDStatus == FALSE ) &&
                  ( bPreviousCDStatus == FALSE ) && 
                  ( enServerState == ENAVISERVER_NORMAL ))
               {
                  if (pclHSI_CMNavi != NULL)
                  {
                     sMsg msg(NULL,(tU32)CMMSG__NAV_SD_LOADING_REQUEST, 0, 0 );
                     pclHSI_CMNavi->bExecuteMessage(msg);
                  }
               }

               bData = TRUE;
               bPreviousSDStatus = TRUE;
               vTrace(NAVI_SD_NAV_SRC_VALID);
               if (pclHSI_CMNavi != NULL)
               {
                  sMsg msg2(NULL,(tU32)CMMSG__NAV_SD_INSERTED, 0, 0 );
                  pclHSI_CMNavi->bExecuteMessage(msg2);
               }
               break;
            }

         case DATAMEDIUM_NOT_INSERTED:
            {
               if ((bPreviousSDStatus == TRUE) &&
                  (DeviceStatus == EN_DEFAULT_DEVICE_SD))
               {
                  if (pclHSI_CMNavi != NULL)
                  {
                     sMsg MediumEjected(NULL,(tU32)CMMSG_NAV_MEDIUM_EJECTED, 0, 0 );
                     pclHSI_CMNavi->bExecuteMessage(MediumEjected);
                  }

                  if(pOTracePointer != NULL)
                     pOTracePointer->vTrace(TR_LEVEL_HMI_INFO, TR_CLASS_HSI_NAV_MSG, 
                     "DAPI->CheckMediumValidty -> NAV_MEDIUM_SD_EJECTED");
               }

               bPreviousSDStatus = FALSE;
               vTrace(NAVI_SD_NAV_SRC_INVALID);
               break;
            }

         case DATAMEDIUM_VERSION_INCOMPATIBLE_WITH_NAVSERVER:
         case DATAMEDIUM_INVALID:
         case DATAMEDIUM_INCORRECT:
         case DATAMEDIUM_TEMP_NOT_ACCESSIBLE:
         case DATAMEDIUM_UNKNOWN:
            {
               bData = FALSE;
               vTrace(NAVI_SD_NAV_SRC_INVALID);
               break;
            }

         default:
            break;
         }
      }

      if( u32DatapoolId != 0 )
      {
         pOFi_Navi->m_poDataPool->u32dp_set( u32DatapoolId,&bData,sizeof(bData) );
      }

   } // NULL != pOFi_Navi
}

/******************************************************************************
*
* FUNCTION   : vHandleRequestSetDefaultDevice() 
* 
* DESCRIPTION: 
*
* PARAMETER  : none
*
* RETURNVALUE: none
*
******************************************************************************/
tVoid clHmiNavServerHandler :: vHandleRequestSetDefaultDevice()const
{
   if((pOFi_Navi != NULL) && (pOFi_Navi->m_poEventEngine != NULL) && 
      (pOFi_Navi->m_poDataPool != NULL) && (poIDAPI != NULL))
   {
      switch(oDAPISinkImplementation.enRequestSetDefaultDevice)
      {
      case ENAVI_PROCESS_RESULT_OK:
         {
            tBool bNoSource= FALSE;
            if(pOFi_Navi->m_poDataPool != NULL)
               pOFi_Navi->m_poDataPool->u32dp_set( DPNAVI__SEAMLESS_NO_SOURCE_AVAILABLE, 
               &bNoSource ,  sizeof(bNoSource) );
            break;
         }
      case ENAVI_PROCESS_RESULT_INVALID:
         {
            vTrace(NAVI_ENAVI_PROCESS_RESULT_INVALID);
            break;
         }

      case ENAVI_PROCESS_BUSY:
         {
            vTrace(NAVI_ENAVI_PROCESS_RESULT_BUSY);
            break;
         }
      default:
         {
            break;
         }
      }
   }
}

/******************************************************************************
*
* FUNCTION   : vHandlevOnUpdateStateOfDevice() 
* 
* DESCRIPTION: 
*
* PARAMETER  : none
*
* RETURNVALUE: none
*
******************************************************************************/
tVoid clHmiNavServerHandler :: vHandlevOnUpdateStateOfDevice()const
{
   if((pOFi_Navi != NULL) && (pOFi_Navi->m_poEventEngine != NULL) && 
      (pOFi_Navi->m_poDataPool != NULL) && (poIDAPI != NULL))
   {
      switch(oDAPISinkImplementation.enDeviceStatus)
      {
      case ENAVI_PROCESS_RESULT_OK:
         {
            break;
         }
      case ENAVI_PROCESS_RESULT_INVALID:
         {
            vTrace(NAVI_ENAVI_PROCESS_RESULT_INVALID);
            break;
         }

      case ENAVI_PROCESS_BUSY:
         {
            vTrace(NAVI_ENAVI_PROCESS_RESULT_BUSY);
            break;
         }
      default:
         {
            break;
         }
      }
   }
}

/******************************************************************************
*
* FUNCTION   : vDatabaseChangedStopRouteGuidance() 
* 
* DESCRIPTION:Sends message to component manager to stop route guidance
*
* PARAMETER  : none
*
* RETURNVALUE: none
*
******************************************************************************/
tVoid clHmiNavServerHandler :: vDatabaseChangedStopRouteGuidance()
{
   tU8 u8RGState    = 0;

   clHSI_CMNAVI*    pclHSI_CMNavi  = NULL;
   clHSI_CMMngr*    pclHSI_CMManager = NULL;
   pclHSI_CMManager = clHSI_CMMngr::pclGetInstance();
   pclHSI_CMNavi    = dynamic_cast <clHSI_CMNAVI*>
      (pclHSI_CMManager->pHSI_BaseGet((tU8)EN_HSI_NAVI));

   //if route guidance is active, send  the HSI event to stop the routeguidance
   if(pOFi_Navi->m_poDataPool != NULL)
   { 
      pOFi_Navi->m_poDataPool->u32dp_get( DPNAVI__ROUTE_GUIDANCE_STATE, &u8RGState ,  sizeof(tU8) );
      tBool bNoSource= FALSE;
      pOFi_Navi->m_poDataPool->u32dp_get( DPNAVI__SEAMLESS_NO_SOURCE_AVAILABLE, 
         &bNoSource ,  sizeof(bNoSource) );
      if(u8RGState == EN_HSI_ROUTESTATUS_RG_ACTIVE) 
      {
         m_u8ResumeRGDBChanged = EN_RESTART_RG_BACKUP;
         if(pOTracePointer != NULL)
            pOTracePointer->vTrace(TR_LEVEL_HMI_INFO, TR_CLASS_HSI_NAV_MSG, 
            "Back up taken %d",m_u8ResumeRGDBChanged);
      }
      if (bNoSource == FALSE)
      {
         if(pOTracePointer != NULL)
            pOTracePointer->vTrace(TR_LEVEL_HMI_INFO, TR_CLASS_HSI_NAV_MSG, 
            "@@@@@RESET THE QUEUE");
         if (pclHSI_CMNavi != NULL)
         {
            sMsg msg(NULL,(tU32)CMMSG_NAVI__RESET_QUEUE, 0, 0 );
            pclHSI_CMNavi->bExecuteMessage(msg);
         }
         u8CauseForOnRequestRefineOfLocIp = NO_CAUSE;
      }
   }
}
/******************************************************************************
*
* FUNCTION   : vNoMediumStopRouteGuidance() 
* 
* DESCRIPTION:Sends message to component manager to stop route guidance
*
* PARAMETER  : none
*
* RETURNVALUE: none
*
******************************************************************************/
tVoid clHmiNavServerHandler :: vNoMediumStopRouteGuidance()
{
   clHSI_CMNAVI*    pclHSI_CMNavi  = NULL;
   clHSI_CMMngr*    pclHSI_CMManager = NULL;
   tU8 u8RGState    = 0;
   pclHSI_CMManager = clHSI_CMMngr::pclGetInstance();
   pclHSI_CMNavi = dynamic_cast <clHSI_CMNAVI*>
      (pclHSI_CMManager->pHSI_BaseGet((tU8)EN_HSI_NAVI));

   //if route guidance is active, send  the HSI event to stop the routeguidance
   if(pOFi_Navi->m_poDataPool != NULL)
   { 
      // set datapool there is no available source.
      tBool bNoSource= TRUE;
      pOFi_Navi->m_poDataPool->u32dp_set( DPNAVI__SEAMLESS_NO_SOURCE_AVAILABLE, 
         &bNoSource ,  sizeof(bNoSource) );

      pOFi_Navi->m_poDataPool->u32dp_get( DPNAVI__ROUTE_GUIDANCE_STATE, &u8RGState ,  sizeof(tU8) );

      if(u8RGState == EN_HSI_ROUTESTATUS_RG_ACTIVE) 
      {
         if (pclHSI_CMNavi != NULL)
         {
            sMsg msg(NULL,(tU32)CMMSG_NAVI_STOP_ROUTE_GUIDANCE, 0, 0 );
            pclHSI_CMNavi->bExecuteMessage(msg);
         }
      }

      if(pOTracePointer != NULL)
         pOTracePointer->vTrace(TR_LEVEL_HMI_INFO, TR_CLASS_HSI_NAV_MSG, 
         "@@@@@RESET THE QUEUE");
      if (pclHSI_CMNavi != NULL)
      {
         sMsg msg(NULL,(tU32)CMMSG_NAVI__RESET_QUEUE, 0, 0 );
         pclHSI_CMNavi->bExecuteMessage(msg);
      }
      u8CauseForOnRequestRefineOfLocIp = NO_CAUSE;
      bpstl::string oClearVersionName = "";
      clNaviUtil::getDataPool().vSetString( DPNAVI__SEAMLESS_SD_INFO, oClearVersionName );
   }
}

/******************************************************************************
*
* FUNCTION   : CheckMediumValidty() 
* 
* DESCRIPTION:Sends message to media component manager 
*
* PARAMETER  : none
*
* RETURNVALUE: none
*
******************************************************************************/
tVoid  clHmiNavServerHandler :: vCheckMediumValidity() const
{
   clHSI_CMNAVI*    pclHSI_CMNavi  = NULL;
   clHSI_CMMngr*    pclHSI_CMManager = NULL;
   pclHSI_CMManager = clHSI_CMMngr::pclGetInstance();
   pclHSI_CMNavi    = dynamic_cast <clHSI_CMNAVI*>
      (pclHSI_CMManager->pHSI_BaseGet((tU8)EN_HSI_NAVI));

   const fi_tclString* pSDInfo = NULL;

   if(pOTracePointer != NULL)
      pOTracePointer->vTrace(TR_LEVEL_HMI_INFO, TR_CLASS_HSI_NAV_MSG, 
      "vCheckMediumValidity()");

   if(enSDValidity == DATAMEDIUM_VALID)
   {
      vTrace(NAVI_SD_NAV_SRC_VALID);
      //Datacarrier SD has nav data message to send to media manager
      if (pclHSI_CMNavi != NULL)
      {
         sMsg msg(NULL,(tU32)CMMSG_NAVI__MMGR_SD_DATA_NAV, 0, 0 );
         pclHSI_CMNavi->bExecuteMessage(msg);
      }
   }
   else 
   {
      vTrace(NAVI_SD_NAV_SRC_INVALID);
      //Datacarrier has no nav data message to send to media manager
      if (pclHSI_CMNavi != NULL)
      {
         sMsg msg(NULL,(tU32)CMMSG_NAVI__MMGR_SD_NO_DATA_NAV, 0, 0 );
         pclHSI_CMNavi->bExecuteMessage(msg);
      }
   }

   if (poIDAPI != NULL)
   {
      //Media Description of SD
      tU32 u32SDGetNum = poIDAPI->u32GetNumberOfAvailableMedia(HSI_C_STRING_DEVICE_SDCARD);

      for(tU32 u32Index2 = 0; u32Index2 < u32SDGetNum; u32Index2++)
      {
         fi_tcl_e8_DapDataAvailable::tenType enGetMediaData = 
            poIDAPI->enGetMediaDataAvailability
            (HSI_C_STRING_DEVICE_SDCARD,u32Index2);
         if (enGetMediaData == fi_tcl_e8_DapDataAvailable::FI_EN_DAP_ENDATAFULLAVAILABLE)
            pSDInfo = poIDAPI->poGetMediaDescription(HSI_C_STRING_DEVICE_SDCARD,u32Index2);
      }
   }

   if((pSDInfo!= NULL) && (pOFi_Navi != NULL))
   {
      pOFi_Navi->vWriteToDpForStringElements( NAVI_HSI_SD_INFO,pSDInfo);
   }

   if(pOTracePointer != NULL)
   {
      if (pSDInfo != NULL)
         pOTracePointer->vTrace( TR_LEVEL_HMI_INFO, TR_CLASS_HSI_NAV_MSG, 
         "pSDInfo = %s", pSDInfo->szValue);
   }
}

/******************************************************************************
*
* FUNCTION   : CheckMediumVersions() 
* 
* DESCRIPTION:Sends message to component manager to stop route guidance
*
* PARAMETER  : none
*
* RETURNVALUE: none
*
******************************************************************************/
tVoid  clHmiNavServerHandler :: vCheckMediumVersions()
{

   tBool DeviceStatus = EN_DEFAULT_DEVICE_CD;

   if((pOFi_Navi->m_poDataPool != NULL)&& (poIDAPI != NULL))
   {
      pOFi_Navi->m_poDataPool->u32dp_get( DPNAVI__UPDATE_MEDIA_STATUS ,
         &DeviceStatus,sizeof(DeviceStatus));
      if(pOTracePointer != NULL)
         pOTracePointer->vTrace( TR_LEVEL_HMI_INFO, TR_CLASS_HSI_NAV_MSG, 
         "vCheckMediumVersions: ActiveDevice %d",DeviceStatus);

      if ((enSDValidity == DATAMEDIUM_VALID)&&
         (poIDAPI-> bIsAMediumFullyAvailable(HSI_C_STRING_DEVICE_SDCARD)))
      {
         poIDAPI->bRequestSetDefaultDevice(HSI_C_STRING_DEVICE_SDCARD);

         if(DeviceStatus != EN_DEFAULT_DEVICE_SD)
         {
            vDatabaseChangedStopRouteGuidance();
            DeviceStatus = EN_DEFAULT_DEVICE_SD;
         }

         if(pOTracePointer != NULL)
            pOTracePointer->vTrace(TR_LEVEL_HMI_INFO, TR_CLASS_HSI_NAV_MSG, 
            "vCheckMediumVersions: SD has valid nav, SetDefaultDevice to SD Card");
      }

      else
      {
         vNoMediumStopRouteGuidance();

         //Close the Data update reminder popup when No SD card is inserted. MMS 235619
         clHSAEventAdapter* poGUI_EventAdapter = clHSAEventAdapter::getInstance();
         if ( poGUI_EventAdapter )
         {
            if (!poGUI_EventAdapter->bGUI_SendSystemEvent(HSA_EVENT__SYSTEM_DUR_CLOSE))
            {
               if(pOTracePointer != NULL)
                  pOTracePointer->vTrace(TR_LEVEL_HMI_INFO, TR_CLASS_HSI_NAV_MSG, 
                  "Data update popup close event rejected");
            }
         }

         if(pOTracePointer != NULL)
            pOTracePointer->vTrace(TR_LEVEL_HMI_INFO, TR_CLASS_HSI_NAV_MSG, 
            "vCheckMediumVersions: SD has not valid nav");
      }

      pOFi_Navi->m_poDataPool->u32dp_set( DPNAVI__UPDATE_MEDIA_STATUS ,
         &DeviceStatus,sizeof(DeviceStatus));
   }
}


/******************************************************************************
*
* FUNCTION   : vHandleOnUpdateRequestReadBlock() 
* 
* DESCRIPTION: 
*
* PARAMETER  : none
*
* RETURNVALUE: none
*
******************************************************************************/
tVoid clHmiNavServerHandler::vHandleOnUpdateRequestReadBlock()
{
   if ( (pOFi_Navi != NULL) && (pOFi_Navi->m_poEventEngine != NULL) && 
      (pOFi_Navi->m_poDataPool != NULL) && (poIDAPI != NULL) && (pOTracePointer != NULL) )
   {
      tBool bDataValid = FALSE;

      switch(oDAPISinkImplementation.enReadBlockStatus)
      {
      case ENAVI_PROCESS_RESULT_OK:
         {
            vTrace(NAVI_ENAVI_PROCESS_RESULT_OK);

            bDataValid = TRUE;
            break;
         }
      case ENAVI_PROCESS_RESULT_INVALID:
         {
            vTrace(NAVI_ENAVI_PROCESS_RESULT_INVALID);

            vMultimediaElementDataReadFailed();
            break;
         }

      case ENAVI_PROCESS_BUSY:
         {
            vTrace(NAVI_ENAVI_PROCESS_RESULT_BUSY);
            break;
         }
      default:
         {
            break;
         }
      }

      if ( bDataValid == TRUE )
      {
         pOTracePointer->vTrace( TR_LEVEL_HMI_INFO, TR_CLASS_HSI_NAV_MSG, 
            "vHandleOnUpdateRequestReadBlock: MMI_MainState %d", m_e8_MMI_MainState);

         vReadMultimediaElementData();
      }
   }
}

