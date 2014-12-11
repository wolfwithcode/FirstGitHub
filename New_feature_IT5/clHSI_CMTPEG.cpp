/******************************************************************************
* FILE : clHSI_CMTPEG.cpp
* PROJECT: Nissan LCN2
* SW-COMPONENT: HSI
*------------------------------------------------------------------------------
* DESCRIPTION: File for HSI Component Manager for DAB
*------------------------------------------------------------------------------
* COPYRIGHT: (c) 2004 Robert Bosch GmbH, Hildesheim
* HISTORY:
*------------------------------------------------------------------------------
* Date      | Author                       | Modification
*------------------------------------------------------------------------------
* 13.04.11  | Rashmi Ashok (ECV4/RBEI) | Initial version.
******************************************************************************/

//-----------------------------------------------------------------------------
//Includes
//-----------------------------------------------------------------------------

// precompiled header, should always be the first include
#include "precompiled.hh"

#include "CompMan/clHSI_CMTPEG.h"
#include "CompMan/HSI_CMDAB_TRACE.h"

#include "fiTPEG/HSI_TPEG_SM_EVENTS.h"
#include "fiTPEG/HSI_TPEG_DEFINES.h"

#include "CompMan/clhsi_cmmngr.h"

#include "GUI_Widget/Trace/GUI_Trace.h"

#include "datapool/cldp_hsi_map.h"

#include "hmi_master/clHMIMngr.h"
#define COMPMAN_HSI_TPEG_INTERFACE

/******************************************************************************
* FUNCTION:    clHSI_CMTPEG::clHSI_CMTPEG
*------------------------------------------------------------------------------
* DESCRIPTION: Default constructor.
*------------------------------------------------------------------------------
* PARAMETER:   None
* RETURNVALUE: None
*******************************************************************************/
clHSI_CMTPEG::clHSI_CMTPEG()
{
	m_pclTPEG_CCAhandler = NULL;
	// configure my base class for traces
	m_u16TR_CLASS       = TR_CLASS_HMI_HSA_CMDAB;//defined in hmi_trace.h
}

/******************************************************************************
* FUNCTION:    clHSI_CMTPEG::clHSI_CMTPEG
*------------------------------------------------------------------------------
* DESCRIPTION: Constructor.
*------------------------------------------------------------------------------
* PARAMETER:   T_EN_HSI_ID en_HSI_ID
* RETURNVALUE: None
*******************************************************************************/
clHSI_CMTPEG::clHSI_CMTPEG(T_EN_HSI_ID en_HSI_ID)
{
	m_enHSI_ID           = en_HSI_ID;
	m_poTrace            = NULL;
	m_pclTPEG_CCAhandler  = NULL;
	// configure my base class for traces
	m_u16TR_CLASS        = TR_CLASS_HMI_HSA_CMDAB;//defined in hmi_trace.h
}

/******************************************************************************
* FUNCTION:    clHSI_CMTPEG::~clHSI_CMTPEG
*------------------------------------------------------------------------------
* DESCRIPTION: Destructor.
*------------------------------------------------------------------------------
* PARAMETER:   None
* RETURNVALUE: None
*******************************************************************************/
clHSI_CMTPEG::~clHSI_CMTPEG()
{
	if (m_poTrace != NULL)
    {
		m_poTrace->vTrace(TR_LEVEL_HMI_INFO, TR_CLASS_HMI_HSA_CMDAB,
			       (tU16)CMDAB_TRACE_DESTRUCTOR );
    }
	m_poTrace           = NULL;
	m_pclTPEG_CCAhandler = NULL;
	m_u16TR_CLASS       = NULL;
}

/******************************************************************************
* FUNCTION:    clHSI_CMTPEG::bConfigure ()
*------------------------------------------------------------------------------
* DESCRIPTION: Method to be called by HSI_Manager in early startup
*------------------------------------------------------------------------------
* PARAMETER:   clITrace* poTrace
* RETURNVALUE: TRUE   : the configuration is valid
*              FALSE  : a configuration error has occured
******************************************************************************/
tBool clHSI_CMTPEG::bConfigure( clITrace* poTrace )
{
	tBool bResult = TRUE;
	clHSI_CMBase::bConfigure(poTrace);

	/* To get the pointer to CCA Handler */
	m_pclTPEG_CCAhandler =  poclGetTPEGCCAHandler();
	if(m_pclTPEG_CCAhandler != NULL)
	{
		if (m_poTrace != NULL)
        {
		    m_poTrace->vTrace(TR_LEVEL_HMI_INFO, TR_CLASS_HMI_HSA_CMDAB,
			           (tU16)CMDAB_TRACE_DABCCAHANDLER );
        }
	}

	return bResult;
}

/******************************************************************************
* FUNCTION:    clHSI_CMTPEG::bAfterConfigure()
*------------------------------------------------------------------------------
* DESCRIPTION: Method to be called by HSI_Manager in early startup to do some
*              initializations
*------------------------------------------------------------------------------
* PARAMETER:   None
* RETURNVALUE: TRUE   : Initialization is successful
*              FALSE  : Initialization not successful
******************************************************************************/
tBool clHSI_CMTPEG::bAfterConfigure()
{
	if (m_poTrace != NULL)
    {
		m_poTrace->vTrace(TR_LEVEL_HMI_INFO, TR_CLASS_HMI_HSA_CMDAB,
			       (tU16)CMDAB_TRACE_AFTER_CONFIGURE );
    }
	tBool bResult = TRUE;

	//Register to Tunermaster TA source DP
	//bRegisterToDatapool(DPTUNERMASTER__TA_SOURCE);
	return bResult;
}

/******************************************************************************
* FUNCTION:    clHSI_CMTPEG::bExecuteMessage()
*------------------------------------------------------------------------------
* DESCRIPTION: Message interface for handling Data Pool update notification
*              messages. By default it forwards to the new bExecuteMessage
*------------------------------------------------------------------------------
* PARAMETER:   tU32 u32Msg
*              tU32 u32Param1
*              tU32 u32Param2
* RETURNVALUE: true  : if the operation is successful
*              false : if the operation fails
******************************************************************************/
tBool clHSI_CMTPEG::bExecuteMessage( tU32 u32Msg,
								    tU32 u32Param1,
								    tU32 u32Param2 )
{
	if (m_poTrace != NULL)
    {
		m_poTrace->vTrace(TR_LEVEL_HMI_INFO, TR_CLASS_HMI_HSA_CMDAB,
			       (tU16)CMDAB_TRACE_EXECUTE_MESSAGE );
    }
	sMsg refMsg;

	refMsg.u32Msg = u32Msg;
	refMsg.u32Param1 = u32Param1;
	refMsg.u32Param2 = u32Param2;

	return (this->bExecuteMessage(refMsg));
}

/******************************************************************************
* FUNCTION :   clHSI_CMTPEG::bExecuteMessage()
*------------------------------------------------------------------------------
* DESCRIPTION: Message interface.
*------------------------------------------------------------------------------
* PARAMETER:   sMsg &refMsg
* RETURNVALUE: true  : if the operation succeeds
*              false : if the operation fails
******************************************************************************/
tBool clHSI_CMTPEG::bExecuteMessage( sMsg &refMsg )
{
	if(m_poTrace != NULL)
    {
		m_poTrace->vTrace(TR_LEVEL_HMI_INFO, TR_CLASS_HMI_HSA_CMDAB,
			       (tU16)CMDAB_TRACE_NEW_EXECUTE_MSG );
    }

   if( refMsg.u32Param1 != NULL)
   {
   }

	if(m_pclTPEG_CCAhandler != NULL)
    {
		/*switch(refMsg.u32Msg)
		{
		case CMMSG_DAB_LOAD_LIST:
			{
				if((tU8)refMsg.u32Param1 == DAB_LIST)
				{//Load Service List
					m_pclTPEG_CCAhandler->vHandle_FID_DAB_S_SET_SRV_LIST_To_FCDAB((tU8)refMsg.u32Param2);
				}
				else if((tU8)refMsg.u32Param1 == SEC_DAB_LIST)
				{//Load Secondary Service List
					m_pclTPEG_CCAhandler->vHandle_FID_DAB_S_SET_COMP_LIST_To_FCDAB((tU8)refMsg.u32Param2);
				}
				break;
			}
		case CMMSG_DAB_ACTIVATE_SERVICE:
			{
				if((tU8)refMsg.u32Param1 == DAB_LIST)
				{
					m_pclTPEG_CCAhandler->vHandle_FID_DAB_S_SRV_SELECT_To_FCDAB(SELECT_MODE_LISTID,
						refMsg.u32Param2, 0 //No Direction for selection through list ID
						);
				}
				else if((tU8)refMsg.u32Param1 == SEC_DAB_LIST)
				{
					m_pclTPEG_CCAhandler->vHandle_FID_DAB_S_COMP_SELECT_To_FCDAB(SELECT_MODE_LISTID,
						refMsg.u32Param2, 0 //No Direction for selection through list ID
						);

				}
				break;
			}
		case CMMSG_DAB_TESTMODE_ENSEMBLE_FRQ:
		{
			m_pclTPEG_CCAhandler->vHandle_FID_DAB_S_SEEK_ENSEMBLE_ToFCDAB((tBool)refMsg.u32Param1);
			break;
		}
		case CMMSG_DAB_TESTMODE_SERVICE_CHANGE:
			{
				m_pclTPEG_CCAhandler->vHandle_FID_DAB_S_SEEK_SERVICE_ToFCDAB((tBool)refMsg.u32Param1);
				break;
			}
		case CMMSG_DAB_TESTMODE_SCREEN_QUERY:
		{
			m_pclTPEG_CCAhandler->vHandle_FID_DAB_S_DB_QUERY_ToFCDAB((tU8)refMsg.u32Param1);
			break;
		}
		case CMMSG_DAB_TESTMODE_EXPERT_API:
		{
			m_pclTPEG_CCAhandler->vHandle_FID_DAB_S_GET_EXP_API_INFO_ToFCDAB((tU16)refMsg.u32Param1);
			break;
		}
		case CMMSG_DAB_SAVE_PRESET:
			{
				m_pclTPEG_CCAhandler->vHandle_FID_DAB_S_SAVE_PRESET_LIST_ELEM_To_FCDAB(refMsg.u32Param1);
				break;
			}
		case CMMSG_DAB_SETUP:
			{//Do a Methodstart to select the SF mode/Concealment Level
				m_pclTPEG_CCAhandler->vHandle_FID_DAB_S_SETUP_To_FCDAB((tU8)refMsg.u32Param1, (tU8)refMsg.u32Param2);
				break;
			}
		case CMMSG_DAB_TESTMODE_ACTIVE:
			{//Do a Methodstart to start/stop testmode
				m_pclTPEG_CCAhandler->vHandle_FID_DAB_S_SET_TESTMODE_To_FCDAB((tBool)refMsg.u32Param1);
				break;
			}
		case CMMSG_DAB_SRV_LIST_UPDATE:
			{//Do a method start for initiating learn or stopping it
				m_pclTPEG_CCAhandler->vHandle_FID_DAB_S_LEARN_To_FCDAB((tBool)refMsg.u32Param1);
				break;
			}
		case CMMSG_DAB_SWITCH_OFF_SECONDARY:
			{
				m_pclTPEG_CCAhandler->vHandle_FID_DAB_S_SRV_SELECT_To_FCDAB(SELECT_MODE_RELATIVE,
						0, //To retreive the last active primary service
						FALSE
						);
				break;
			}
		case CMMSG_DAB_PREV_NEXT_SERVICE:
			{
				m_pclTPEG_CCAhandler->vHandle_FID_DAB_S_SRV_SELECT_To_FCDAB(SELECT_MODE_RELATIVE,
						1, //No. of steps would be 1 since it is next/previous service
						(tBool)refMsg.u32Param1
						);
				break;
			}
		case CMMSG_DAB_PREV_NEXT_SEC_COMP:
			{
				m_pclTPEG_CCAhandler->vHandle_FID_DAB_S_COMP_SELECT_To_FCDAB(SELECT_MODE_RELATIVE,
						1, //No. of steps would be 1 since it is next/previous service
						(tBool)refMsg.u32Param1
						);
				break;
			}
		case DP_VALUE_CHG:
			{
               switch(refMsg.u32Param1)
               {
			   case DPTUNERMASTER__TA_SOURCE:
				   {
					   tU8 u8TASource = 0;
					   if(m_poDataPool != NULL)
					   {
						   m_poDataPool->u32dp_get(DPTUNERMASTER__TA_SOURCE, &u8TASource, sizeof(u8TASource));

						   if(u8TASource == midw_fi_tcl_e8_Source::FI_EN_TUN_MSTR_SOURCE_DAB_TA)
						   {//If the active TA source is DAB, update service label to be displayed in TA popup

							   tChar pcServiceLabel[LABEL_LEN+1] = {'\0'};

			                   tU32 u32Size = m_poDataPool->u32dp_getElementSize(DPDAB__CHANNEL_INFO_SERVICE_LABEL);
			                   m_poDataPool->u32dp_get( DPDAB__CHANNEL_INFO_SERVICE_LABEL,
                                                        pcServiceLabel,
                                                        u32Size );
							   //Set the TA PS name Datapool with the service label
							   u32Size = m_poDataPool->u32dp_getElementSize(DPTUNER__FOR_XML_RADIO_CURRENT_TA_STATION_NAME);
                               m_poDataPool->u32dp_set( DPTUNER__FOR_XML_RADIO_CURRENT_TA_STATION_NAME,
                                                        pcServiceLabel,
                                                        u32Size);
						   }
					   }
					   break;
				   }
			   default:
				   {
					   break;
				   }
			   }
			   break;
			}
		default:
			{
				break;
			}
		}*/
	}
	tBool bResult = TRUE;
	return bResult;
}

/******************************************************************************
* FUNCTION:    clHSI_CMTPEG::bExecuteExtMessage()
*------------------------------------------------------------------------------
* DESCRIPTION: Extended message interface.
*------------------------------------------------------------------------------
* PARAMETER:   sExtMsg &refMsg
* RETURNVALUE: true  : if the operation succeeds
*              false : if the operation fails
******************************************************************************/
/*
tBool clHSI_CMTPEG::bExecuteExtMessage( sExtMsg &refMsg )
{
	if (m_poTrace != NULL)
    {
		m_poTrace->vTrace(TR_LEVEL_HMI_INFO, TR_CLASS_HMI_HSA_CMDAB,
			       (tU16)CMDAB_TRACE_EXE_EXTENDED_MSG );
    }
	tBool bRetVal = TRUE;
	return bRetVal;
}
*/

/******************************************************************************
* FUNCTION:    clHSI_CMTPEG::poclGetTPEGCCAHandler()
*------------------------------------------------------------------------------
* DESCRIPTION: Private helper to access clDAB_CCAhandler
*------------------------------------------------------------------------------
* PARAMETER:   None
* RETURNVALUE: Pointer to clDAB_CCAhandler, NULL in error case
******************************************************************************/
clTPEG_CCAhandler* clHSI_CMTPEG::poclGetTPEGCCAHandler () const
{
	if (m_poTrace != NULL)
    {
		m_poTrace->vTrace(TR_LEVEL_HMI_INFO, TR_CLASS_HMI_HSA_CMDAB,
			       (tU16)CMDAB_TRACE_GET_HANDLER_POINTER );
    }
	// encapsulated singleton pattern
	static clTPEG_CCAhandler* s_poclTPEGCCAhandler = NULL;
	if (NULL == s_poclTPEGCCAhandler)
	{
		clFiFactory* poclFiFactory = NULL;

		clHSIMngr* poclHSIMngr = clHSIMngr::pclGetInstance();
		if (NULL == poclHSIMngr)
		{
			return NULL;
		}

		poclHSIMngr->bGetIfcPtr2FiFactory(&poclFiFactory );
		if (poclFiFactory==NULL)
		{
			return NULL;
		}

		tU8 nEvFiAdress = (tU8)(poclHSIMngr->u32GetEventSubAdress(HSI|TPEG));

		clHSIDeviceGeneral* poFI = poclFiFactory->GetFi(nEvFiAdress);
		if (NULL==poFI)
		{
			return NULL;
		}
		s_poclTPEGCCAhandler = (dynamic_cast<clFi_TPEG*>(poFI))->poclGetTPEGCCAhandler();
	}
	return s_poclTPEGCCAhandler;
}

/******************************************************************************
* METHOD:         vSendSystemEvent
* DESCRIPTION:    send an System Event to the GUI
* PARAMETER:      HSA_EventEnum_SYSTEM_EVENT enGUI_SystemEvent
* RETURNVALUE:
*                  true     : if the systemevent is delivered
*                  false    : if the systemevent is not delivered
******************************************************************************/
/*tBool clHSI_CMTPEG::vSendSystemEvent(HSA_EventEnum_SYSTEM_EVENT enGUI_SystemEvent)
{
	// TRACE SYSTEM EVENT!!

	vTrace(HSI_CM_PHONE_SENDSYSTEM_EVENT, TR_LEVEL_HMI_INFO, (tU32) enGUI_SystemEvent);
	return clHSI_CMBase::vSendSystemEvent(enGUI_SystemEvent);

}*/




