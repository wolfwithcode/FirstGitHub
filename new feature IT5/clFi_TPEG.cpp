/******************************************************************************
* FILE: clFi_TPEG.cpp
* PROJECT: Nissan LCN2
* SW-COMPONENT: TPEG HMI
*------------------------------------------------------------------------------
* DESCRIPTION: HMI Functional Interface Component to TPEG
*------------------------------------------------------------------------------
* COPYRIGHT: (c) 2004 Robert Bosch GmbH, Hildesheim
* HISTORY:  
*------------------------------------------------------------------------------
* Date      | Author                       | Modification
*------------------------------------------------------------------------------
* 12.04.11  | Rashmi Ashok (RBEI/ECV4) | Initial version
******************************************************************************/

//-----------------------------------------------------------------------------
// includes
//-----------------------------------------------------------------------------
// precompiled header, should always be the first include
#include "precompiled.hh"
//Header files
#include "fiTPEG/clFi_TPEG.h"
// see precompiled.hh		#include "clhsi_cmmngr.h"
#include "CompMan/clHSI_CMTPEG.h"

//To include HSI generated events to SM
#include "fiTPEG/HSI_TPEG_SM_EVENTS.h"

//To include HSI local defines
#include "fiTPEG/HSI_TPEG_DEFINES.h"

//To include Trace Defines
#include "fiTPEG/HSI_TPEG_TRACE.h"

#ifdef VARIANT_S_FTR_ENABLE_HMI_EXTSYS_ITG5
#ifdef VARIANT_S_FTR_ENABLE_TRC_GEN
#define ETG_DEFAULT_TRACE_CLASS TR_CLASS_HMI_HSA_SYSTEM_CONFIG
#include "trcGenProj/Header/clHSA_Config.cpp.trc.h"
#endif
#endif //VARIANT_S_FTR_ENABLE_HMI_EXTSYS_ITG5

//static variables initialization
clFi_TPEG* clFi_TPEG::pclInstance = NULL;
tU16 clFi_TPEG::u16InstanceCnt = 0;

/******************************************************************************
* FUNCTION:    clFi_TPEG::~clFi_TPEG()
*------------------------------------------------------------------------------
* DESCRIPTION: Destructor ( Cleanup )
*------------------------------------------------------------------------------
* PARAMETER:   NONE
* RETURNVALUE: NONE
******************************************************************************/
clFi_TPEG::~clFi_TPEG()
{
	if (m_poTrace != NULL)
    {
		m_poTrace->vTrace(TR_LEVEL_HMI_INFO, TR_CLASS_HSI_DAB_MSG,
			       (tU16)FIDAB_TRACE_DESTRUCTOR );
    }
	bRegFlag = FALSE;

if (poclTPEG_CCAhandler !=   NULL)
    {
        //Delete the created Handler object
		OSAL_DELETE poclTPEG_CCAhandler ;
        //Assign NULL to the pointer 
        poclTPEG_CCAhandler = NULL;
    }
	if(m_pHSI_TPEG != NULL)
	{
		//Delete the created object
		OSAL_DELETE m_pHSI_TPEG ;
        //Assign NULL to the pointer 
        m_pHSI_TPEG = NULL;
	}
    else
    {
		//Pointer is already NULL- Nothing to do
	}
}

/******************************************************************************
* FUNCTION:    clFi_TPEG::clFi_TPEG()
*------------------------------------------------------------------------------
* DESCRIPTION: Constructor 
*------------------------------------------------------------------------------
* PARAMETER:   NONE
* RETURNVALUE: NONE
******************************************************************************/
clFi_TPEG::clFi_TPEG()
{
    //Create an object of FI CCA handler
	poclTPEG_CCAhandler = OSAL_NEW clTPEG_CCAhandler(this);
    pclInstance  = this;

    m_pHSI_TPEG   =  NULL;
    bRegFlag     = FALSE;
}

/******************************************************************************
* FUNCTION:    clFi_TPEG::pclGetInstance()
*------------------------------------------------------------------------------
* DESCRIPTION: This is the interface to get the pointer of the singleton
*              object, it creates the singleton object for 1st time. 
*------------------------------------------------------------------------------
* PARAMETER:   NONE
* RETURNVALUE: clFi_TPEG *  ( pointer to the singleton object)
******************************************************************************/
clFi_TPEG * clFi_TPEG::pclGetInstance(tVoid)
{
	/*if (m_poTrace != NULL)
    {
		m_poTrace->vTrace(TR_LEVEL_HMI_INFO, TR_CLASS_HSI_DAB_MSG,
			       (tU16)FIDAB_TRACE_GET_INSTANCE );
    }*/
    return pclInstance;
}

/******************************************************************************
* FUNCTION:    clFi_TPEG::getTracePointer()
*------------------------------------------------------------------------------
* DESCRIPTION: Returns the trace object pointer.
*------------------------------------------------------------------------------
* PARAMETER:   NONE
* RETURNVALUE: clITrace *
******************************************************************************/
clITrace* clFi_TPEG::getTracePointer(tVoid ) const
{
	if (m_poTrace != NULL)
    {
		m_poTrace->vTrace(TR_LEVEL_HMI_INFO, TR_CLASS_HSI_DAB_MSG,
			       (tU16)FIDAB_TRACE_GET_TRACE_POINTER );
    }
    return m_poTrace;
}

/******************************************************************************
* FUNCTION:    clFi_TPEG::u16GetNumberOfHandler()
*------------------------------------------------------------------------------
* DESCRIPTION: This function will be called by CCA gateway to get the  
*              number of services this FI uses.
*------------------------------------------------------------------------------
* PARAMETER:   NONE
* RETURNVALUE: tU16  ( NO. of services needed by this FI)
******************************************************************************/
tU16 clFi_TPEG::u16GetNumberOfHandler(tVoid)
{
	if (m_poTrace != NULL)
    {
		m_poTrace->vTrace(TR_LEVEL_HMI_INFO, TR_CLASS_HSI_DAB_MSG,
			       (tU16)FIDAB_TRACE_GET_NO_OF_HANDLER );
    }
    //The value EN_DAB_NUM_OF_SERVICE_HANDLERS is one more than the actual number of handlers
    return EN_TPEG_NUM_OF_SERVICE_HANDLERS -1;
}

/******************************************************************************
* FUNCTION:    clFi_TPEG::vGetHandlerData()
*------------------------------------------------------------------------------
* DESCRIPTION: This function will be called EN_DAB_NUM_OF_SERVICE_HANDLERS's-1
*              times by CCA gateway to get the details of Service Id , version No.s etc 
*------------------------------------------------------------------------------                
* PARAMETER:   tVoid*  pHandler
*              tU16*   u16ServiceID 
*              tU16*   u16MajorVersion 
*              tU16*   u16MinorVersion 
*              tChar** szFIName
* RETURNVALUE: NONE
******************************************************************************/
tVoid clFi_TPEG::vGetHandlerData( tU16 u16Handler,      tVoid** pHandler,
                                 tU16* u16ServiceID,   tU16* u16ServerAppId,
                                 tU16* u16MajorVersion,tU16* u16MinorVersion,
                                 const tChar** szFIName, tU16* u16HandlerType )
{
	if (m_poTrace != NULL)
    {
		m_poTrace->vTrace(TR_LEVEL_HMI_INFO, TR_CLASS_HSI_DAB_MSG,
			       (tU16)FIDAB_TRACE_GET_HANDLER_DATA, sizeof( u16Handler), &u16Handler );
    }
	switch (u16Handler)
    {
    case EN_TPEG_SERVICE_HANDLER:
       {
		   /* fill all the details in the Parameterd pointers */
    	   //Currently this service is not used. So we can disable this via #ifdef.
			#ifdef VARIANT_S_FTR_ENABLE_HMI_TUNER_ITG5
           *pHandler         = poclTPEG_CCAhandler;
           *u16ServiceID     = CCA_C_U16_SRV_FC_TPEG; //TPEG Service Id
           *u16ServerAppId   = CCA_C_U16_APP_FC_TPEG; //DAB App Id
           *szFIName         = "MIDW_TPEG_FI"; //FI name
           *u16MajorVersion  = MIDW_TPEGFI_C_U16_SERVICE_MAJORVERSION;
           *u16MinorVersion  = MIDW_TPEGFI_C_U16_SERVICE_MINORVERSION;
		   *u16HandlerType   = (tU16)HANDLER_EN_CLIENT; //This is a client

		   if( poclTPEG_CCAhandler != NULL )
		   {
			   poclTPEG_CCAhandler->bInitCCAHandlerGeneral(
				                       this,
				                       m_poTrace,
				                       *u16ServerAppId,
				                       *u16ServiceID,
				                       *u16MajorVersion,
				                       *u16MinorVersion);
		   }
			#endif
           break;
       } 
    default:
        break;
    }
}

/******************************************************************************
* FUNCTION:    clFi_TPEG::vServiceStateChanged()
*------------------------------------------------------------------------------
* DESCRIPTION: CCA gateway will call this function to pass the Service state 
*              of the server. FI will pass this info to its handler. 
*------------------------------------------------------------------------------
* PARAMETER:   tU16 u16ServiceID
*              tU8  u8ServiceState
* RETURNVALUE: NONE
******************************************************************************/
tVoid clFi_TPEG::vServiceStateChanged(tU16 u16ServiceID, tU8 u8ServiceState)
{
	if (m_poTrace != NULL)
    {
		m_poTrace->vTrace(TR_LEVEL_HMI_INFO, TR_CLASS_HSI_DAB_MSG,
			       (tU16)FIDAB_TRACE_SERVICE_STATE_CHANGE , 
                   sizeof( u8ServiceState) , &u8ServiceState );
    }
    if( CCA_C_U16_SRV_FC_TPEG == u16ServiceID )
    {
	    if( NULL != poclTPEG_CCAhandler)
        {
			/* store the service state of Server in Handler */
	    	poclTPEG_CCAhandler->vSetServiceState(u8ServiceState);
        
		    if( AMT_C_U8_SVCSTATE_AVAILABLE == u8ServiceState )
            {
				if (m_poTrace != NULL)
                {
		            m_poTrace->vTrace(TR_LEVEL_HMI_INFO, TR_CLASS_HSI_DAB_MSG,
			            (tU16)FIDAB_TRACE_REG_FIDS );
                }
			    // Invoking the Functional Registration.
				poclTPEG_CCAhandler->bRegisterForAllFID();
                //Load preset list and set autocompare parameters
            }
            else if( AMT_C_U8_SVCSTATE_NOT_AVAILABLE == u8ServiceState )
            {
            }
		}
    }
}

/******************************************************************************
* FUNCTION:    clFi_TPEG::vSetRegisterID()
*------------------------------------------------------------------------------
* DESCRIPTION: CCA gateway will call this function to pass the Register ID
*              sent by the server. FI will pass this info to its handler.
*------------------------------------------------------------------------------
* PARAMETER:   tU16 u16ServiceID
*              tU16 u16RegisterID 
* RETURNVALUE: NONE
******************************************************************************/
tVoid clFi_TPEG::vSetRegisterID(tU16 u16ServiceID, tU16 u16RegisterID)
{
    /* check weather service ID is the required one or not */   
    if (u16ServiceID == CCA_C_U16_SRV_FC_TPEG)
    { 
		if (m_poTrace != NULL)
        {
		    m_poTrace->vTrace(TR_LEVEL_HMI_INFO, TR_CLASS_HSI_DAB_MSG,
			           (tU16)FIDAB_TRACE_SET_REG_ID , 
                       sizeof( u16RegisterID) , &u16RegisterID );
        }
        if( poclTPEG_CCAhandler !=NULL )
        {
            // pass the Reg ID to handler
        	poclTPEG_CCAhandler->vSetRegisterId(u16RegisterID);
        }
    }
}

/******************************************************************************
* FUNCTION:    clFi_TPEG::pvGetCCAMain()
*------------------------------------------------------------------------------
* DESCRIPTION: Returns the pointer of CCA gateway stored in the class member. 
*------------------------------------------------------------------------------
* PARAMETER:   tVoid
* RETURNVALUE: tVoid*  ( pointer of CCA gateway type casted as void *)
******************************************************************************/
tVoid* clFi_TPEG::pvGetCCAMain(tVoid)
{
	if (m_poTrace != NULL)
    {
		m_poTrace->vTrace(TR_LEVEL_HMI_INFO, TR_CLASS_HSI_DAB_MSG,
			       (tU16)FIDAB_TRACE_GET_CCA_MAIN);
    }
    return (tVoid *)m_poCCAMainCallback;
}


/******************************************************************************
* FUNCTION:    clFi_TPEG::SendEventsToEventEngine()
*------------------------------------------------------------------------------ 
* DESCRIPTION: This interface will be used by the handler
*               to send events to Event engine (general )
*------------------------------------------------------------------------------
* PARAMETER: u32FunctionId - FID of message 
*            u32Data- to decide the event
* RETURNVALUE: NONE
******************************************************************************/
tVoid clFi_TPEG::SendEventsToEvtEn(tU32 u32FunctionId, tU32 u32Data)
{
	//This function is split into separate handlers for specifc use
	//Hence no implementation here 
    tU32 u32TempData = u32Data;
    u32Data = u32TempData;

    m_poEventEngine->bEvent2Engine(u32FunctionId);
    return ;  
}

/******************************************************************************
* FUNCTION:    clFi_TPEG::ReceiveEventsFromEvtEn()
*------------------------------------------------------------------------------ 
* DESCRIPTION: HSI Manager will call this function to pass the event from 
*              event engine by diverting events to the corresponding FI's
*------------------------------------------------------------------------------
* PARAMETER:   u32Event ( EVENT to this FI )
* RETURNVALUE: NONE
******************************************************************************/
tVoid clFi_TPEG::ReceiveEventsFromEvtEn(tU32 u32Event)
{
	//Check whether the DAB_CCA handler pointer is valid or not before proceeding
    if( poclTPEG_CCAhandler != NULL )
    {
		/* Depending on the Event, call the respective handler funtions of the 
          DAB_CCAhandler object with the required parameters */
		switch(u32Event)
        {
		case 0:
			break;
		default:
			break;
	    }
    }
}

/******************************************************************************
* FUNCTION:    clFi_TPEG::WriteDataToDataPool()
*------------------------------------------------------------------------------ 
* DESCRIPTION: General interface for Handler functions to write the data
*              into datapool ,used incase of lesser dataload
*------------------------------------------------------------------------------
* PARAMETER:   tU32 u32FunctionId  
*              tU32 u32Data
* RETURNVALUE: NONE
******************************************************************************/
tVoid clFi_TPEG::WriteDataToDp(tU32 u32FunctionId, tU32 u32Data)
{
	tU32 u32Temp = u32Data;//To get rid of lint
	u32Data = u32Temp;//To get rid of lint
	//Check validity of datapool engine object pointer before proceeding
    if( NULL != m_poDataPool )
    {
		switch (u32FunctionId) //switch on FID
        { 
		case 0:
			break;
		default:
			break;
		}
    }
}

/******************************************************************************
* FUNCTION:    clFi_TPEG::WriteStringDataToDp()
*------------------------------------------------------------------------------ 
* DESCRIPTION: Function to handle writing of string data to datapool
*------------------------------------------------------------------------------
* PARAMETER:   tU32 u32DataElem
*              tChar* pcBuffer
* RETURNVALUE: NONE
******************************************************************************/
tVoid clFi_TPEG::WriteStringDataToDp(tU32 u32DataElem, tChar* pcBuffer) const
{
    if( NULL == pcBuffer )
    {
        //Add trace for printing that string is NULL
        return;
    }
	switch ((tU8)u32DataElem)
    {
	case 0:
		break;
	default:
		break;
    }
    return;
}   

/******************************************************************************
* FUNCTION:    clFi_TPEG::ReadDataFromDp()
*------------------------------------------------------------------------------ 
* DESCRIPTION: To read read data from Data Pool
*------------------------------------------------------------------------------
* PARAMETER:   NONE
* RETURNVALUE: void
******************************************************************************/
tVoid clFi_TPEG::ReadDataFromDp()
{   
}

/******************************************************************************
* FUNCTION:    clFi_TPEG::vDAB_WriteDataToDataPool()
*------------------------------------------------------------------------------
* DESCRIPTION: Function writes list data into given data pool index. 
*------------------------------------------------------------------------------
* PARAMETER:   u32DataPoolIndex : Index of the datapool into which data must be written
*              u8DataPoolListColumnIndex : Column Index where the data has to be written
*              u16DataPoolListRowIndex: Row Index where the data has to be written
*              pvDataWrite: Data pointer pointing to the data 
*              u32ByteCount: Byte count of the data to be written 
* RETURNVALUE: tVoid 
******************************************************************************/
tVoid clFi_TPEG::vTPEG_WriteDataToDataPool(tU32 u32DataPoolIndex,
										 tU8 u8DataPoolListColumnIndex,
										 tU16 u16DataPoolListRowIndex,
										 tVoid* pvDataWrite, 
										 tU32 u32ByteCount)
{
	cldpt_base* pDynEnt = NULL;
	cldpt_list* pDynLst = NULL;

	if(NULL != pvDataWrite)
	{   
		if( NULL != m_poDataPool )
		{
		    pDynEnt = m_poDataPool->getdp_Elementhandle( u32DataPoolIndex ); //Get dynamic element
			if( pDynEnt != NULL) //Got the dynamic element?
			{ 
			    pDynLst = dynamic_cast<cldpt_list*>(pDynEnt); //Cast dynamic element to list element

				//Write ( RowIdx, ColIdx, Reference, ByteCount ) 
			    if(pDynLst != NULL)
				pDynLst->writeElement( 
					u16DataPoolListRowIndex, 
					u8DataPoolListColumnIndex,
					pvDataWrite, 
					u32ByteCount );

				vTraceWriteToDatapool(
					pvDataWrite,
					u32ByteCount,
					u32DataPoolIndex,
					u16DataPoolListRowIndex,
					u8DataPoolListColumnIndex);

			}
		}
	}
}

/******************************************************************************
* FUNCTION:    clFi_TPEG::vDAB_WriteDataToDataPool()
*------------------------------------------------------------------------------
* DESCRIPTION: Function writes list string data into given data pool index
*------------------------------------------------------------------------------
* PARAMETER:   u32DataPoolIndex : Index of the data pool value into which data must be written.
*              u32DataPoolListColumnIndex : Column index where the data has to written
*              u8DataPoolListRowIndex : Row index where the data has to written 
*              pMWstr : midw_ext_fi_tclString string which has to be written
* RETURNVALUE: tVoid 
*******************************************************************************/
tVoid clFi_TPEG::vTPEG_WriteDataToDataPool( tU32 u32DataPoolIndex,
										  tU8 u8DataPoolListColumnIndex,
										  tU8 u8DataPoolListRowIndex,
										  midw_ext_fi_tclString* pMWstr)
{
	cldpt_base* pDynEnt = NULL;
	cldpt_list* pDynLst = NULL;
	bpstl::string ostr(pMWstr->szValue);

	if(NULL != pMWstr)
	{   
		if( NULL != m_poDataPool )
		{
			//Get dynamic element
			pDynEnt = m_poDataPool->getdp_Elementhandle( u32DataPoolIndex );
			if( pDynEnt != NULL )//Got the dynamic element?
			{ 
				pDynLst = dynamic_cast<cldpt_list*>(pDynEnt);//Cast dynamic element to list element

				//Write ( RowIdx, ColIdx, String )
				if(pDynLst != NULL)
				pDynLst->writeElement( 
					u8DataPoolListRowIndex, 
					u8DataPoolListColumnIndex,
					&ostr, 
					0 );
				;

				vTraceWriteToDatapool(
						reinterpret_cast<tVoid *>(const_cast<char *>(ostr.c_str())),
					ostr.length(),
					u32DataPoolIndex,
					u8DataPoolListRowIndex,
					u8DataPoolListColumnIndex);
			}
		}
	}
}

/******************************************************************************
* FUNCTION:    clFi_TPEG::vStartInitSequence()
*------------------------------------------------------------------------------ 
* DESCRIPTION: To start the initialization-sequence from HSI-handler.
*------------------------------------------------------------------------------
* PARAMETER:   u16ServiceID
* RETURNVALUE: NONE
******************************************************************************/
tVoid clFi_TPEG::vStartInitSequence(tU16 u16ServiceID)
{
	if (m_poTrace != NULL)
    {
		m_poTrace->vTrace(TR_LEVEL_HMI_INFO, TR_CLASS_HSI_DAB_MSG,
			       (tU16)FIDAB_TRACE_START_INIT_SEQUENCE);
    }
    switch (u16ServiceID)
    {
	case CCA_C_U16_SRV_DAB_TUNER:
        {
            if (FALSE == bRegFlag)
            {
                bRegFlag = TRUE;
            }
			break;
        }
    default:
        {
			break;
        }
    }
}

/******************************************************************************
* FUNCTION:    clFi_TPEG::pHSI_GetDAB()
*------------------------------------------------------------------------------ 
* DESCRIPTION: Method to get a Pointer to ComponentManager DAB
*------------------------------------------------------------------------------
* PARAMETER:   NONE
* RETURNVALUE: clHSI_CMDAB*
******************************************************************************/
clHSI_CMTPEG* clFi_TPEG::pHSI_GetTPEG()
{
	if (m_poTrace != NULL)
    {
		m_poTrace->vTrace(TR_LEVEL_HMI_INFO, TR_CLASS_HSI_DAB_MSG,
			       (tU16)FIDAB_TRACE_GET_DABCM_POINTER);
    }
    if (m_pHSI_TPEG == NULL)
    {
        clHSI_CMMngr*  pclHSI_CMManager = clHSI_CMMngr::pclGetInstance();
        m_pHSI_TPEG   =  dynamic_cast <clHSI_CMTPEG*>(pclHSI_CMManager->pHSI_BaseGet(EN_HSI_TPEG));
    }
    return m_pHSI_TPEG;
}

/******************************************************************************
* FUNCTION:    clFi_TPEG::vNewAppState()
*------------------------------------------------------------------------------ 
* DESCRIPTION: This function gets the new app state and if system is running
*              down, it sets a flag
*              This is used to avoid resetting SD values when running down
*------------------------------------------------------------------------------
* PARAMETER:   enAppStates - new system state
* RETURNVALUE: NONE
******************************************************************************/
tVoid clFi_TPEG::vNewAppState(tenFwAppStates enAppStates)
{
    if (m_poTrace != NULL)
    {
		tU8 u8AppState = (tU8)enAppStates;
        m_poTrace->vTrace(TR_LEVEL_HMI_INFO, TR_CLASS_HSI_DAB_MSG,
			       (tU16)FIDAB_TRACE_NEW_APP_STATE, sizeof(u8AppState), &u8AppState);
    }
	//Check if system state
   if (enAppStates != FW_APP_STATE_NORMAL)
   {
	   //Handle app state when not normal if required
   }
}

/******************************************************************************
* FUNCTION:    clFi_TPEG::vTraceWriteToDatapool()
*------------------------------------------------------------------------------
* DESCRIPTION: Trace Function for List 
*------------------------------------------------------------------------------
* PARAMETERS:  tVoid* pvDataWrite
*              tU32 u32ByteCount 
*              tU32 u32DpIdx
*              tU16 u16DataPoolListRowIndex
*              tU8 u8DataPoolListColumnIndex
* RETURNVALUE: tVoid 
******************************************************************************/
tVoid clFi_TPEG::vTraceWriteToDatapool(const tVoid* pvDataWrite,
									  tU32 u32ByteCount, 
									  tU32 u32DpIdx,
									  tU16 u16DataPoolListRowIndex,
									  tU8 u8DataPoolListColumnIndex)const
{
	tU8* pu8Tracedata = NULL;
	tU32 u32TraceDataSize = NULL;

	u32TraceDataSize = (
		u32ByteCount 
		+ sizeof(u32DpIdx) 
		+ sizeof(u16DataPoolListRowIndex) 
		+ sizeof(u8DataPoolListColumnIndex));

	/*get mem*/
	pu8Tracedata = OSAL_NEW tU8[u32TraceDataSize];

	if (pu8Tracedata != NULL)
	{
		OSAL_pvMemoryCopy( pu8Tracedata, &u32DpIdx, sizeof(u32DpIdx) );
		OSAL_pvMemoryCopy( (tVoid*)&pu8Tracedata[4], &u16DataPoolListRowIndex, sizeof(u16DataPoolListRowIndex) );
		OSAL_pvMemoryCopy( (tVoid*)&pu8Tracedata[5], &u8DataPoolListColumnIndex, sizeof(u8DataPoolListColumnIndex) );
		OSAL_pvMemoryCopy( (tVoid*)&pu8Tracedata[6], pvDataWrite, u32ByteCount );
	}

	vTrace( TR_LEVEL_HMI_INFO, 
		    (tU16) DAB_WRITE_LIST_DATA_TO_DATAPOOL,
		    pu8Tracedata,
		    u32TraceDataSize );

	OSAL_DELETE []pu8Tracedata;
}

/******************************************************************************
* FUNCTION:    clFi_TPEG::vTrace()
*------------------------------------------------------------------------------
* DESCRIPTION: Trace Function
*------------------------------------------------------------------------------
* PARAMETER:   hmi_tenTraceLevel eTraceLevel   INFO, ERROR, ...
*              tU16              u16TraceCmd   defined in "HSI_DAB_TRACE.h"      
*              tU8*              u8Data        any kind of data
*              tU32              u32DataLength Length of data
* RETURNVALUE: tVoid 
******************************************************************************/
tVoid clFi_TPEG::vTrace( hmi_tenTraceLevel eTraceLevel,
						tU16 u16TraceCmd, 
						tU8* u8Data,
						tU32 u32DataLength) const
{
	if( NULL != m_poTrace ) 
	{
		if (u8Data == NULL)
		{
			//Trace simple command
			m_poTrace->vTrace( eTraceLevel,
				               TR_CLASS_HSI_DAB_MSG, 
				               (tU16)u16TraceCmd );
		}
		else
		{
			//Trace data message
			m_poTrace->vTrace( eTraceLevel,
				               TR_CLASS_HSI_DAB_MSG, 
				               (tU16)u16TraceCmd, 
				               (tU8)u32DataLength, 
				               u8Data );
		}
	}
	if(u8Data != NULL)
		u8Data = NULL;
}


