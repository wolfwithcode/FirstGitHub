/************************************************************************
* FILE:         clFI_Tuner.cpp
* PROJECT:      FORD HSRNS
* SW-COMPONENT: 
*----------------------------------------------------------------------
*
* DESCRIPTION: Tuner Functional Interface Component 
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

/*To include datapool values*/

// precompiled header, should always be the first include
#include "precompiled.hh"

// see precompiled.hh		#define HMI_BASE_DATA_POOL_CONFIG_IF
// see precompiled.hh		#include "data_pool_if.h"     // use interface functions of DataPool

/*To include statemachine values*/
//#include "statemachine_if.h"

#include "fituner/clFI_TUNER.h"
// see precompiled.hh		#include "../CompMan/clhsi_cmmngr.h"
#include "CompMan/clHSI_Radio.h"

//To include HSI generated events to SM
#include "fituner/HSI_TUN_SM_EVENTS.h"

//To include HSI local defines
#include "fituner/HSI_TUN_DEFINES.h"

//To include Trace Defines
#include "fituner/HSI_TUNER_TRACE.h"


//static variables initialization
clFi_TUNER * clFi_TUNER::pclInstance = NULL;
tU16 clFi_TUNER::u16InstanceCnt = 0;


/** Total No of Char for FM(Freq, Station Name and "TP") */
#define MAX_NO_OF_CHAR_FM 38
#define NO_OF_STEPS		1

//Tuner NAR Area
#define TUN_AREA_NAR	0x02


#ifdef VARIANT_S_FTR_ENABLE_TRC_GEN
#define ETG_DEFAULT_TRACE_CLASS TR_CLASS_HSI_TUNER_MSG
#include "trcGenProj/Header/clFI_TUNER.cpp.trc.h"
#endif




/*************************************************************************
*  FUNCTION:   clFi_TUNER::~clFi_TUNER()
*
* DESCRIPTION:   Destructor
*
* PARAMETER:   NONE
*
* RETURNVALUE: NONE
*
*************************************************************************/

clFi_TUNER::~clFi_TUNER()
{
   bRegFlag = FALSE;

   if (poclTuner_CCAhandler !=   NULL)
   {
      OSAL_DELETE poclTuner_CCAhandler ;  // delete the created Handler object 
      
      poclTuner_CCAhandler = NULL;  // assign NULL to the pointer 
   }
   if (poclTuner_CCAhandler_Diag !=   NULL)
   {
      OSAL_DELETE poclTuner_CCAhandler_Diag ;  // delete the created Handler object 
      
      poclTuner_CCAhandler_Diag = NULL;  // assign NULL to the pointer 
   }
   if (m_pHSI_Radio !=  NULL)
   {
	   OSAL_DELETE m_pHSI_Radio ;  // delete the created Handler object

	   m_pHSI_Radio = NULL;  // assign NULL to the pointer
   }
   //it will be NULL else
}

/*************************************************************************
*  FUNCTION:    clFi_TUNER::clFi_TUNER() 
*
* DESCRIPTION:  Constructor
*
* PARAMETER:   NONE
*
* RETURNVALUE: None
*
*************************************************************************/


clFi_TUNER::clFi_TUNER() 
{
   poclTuner_CCAhandler = OSAL_NEW clTuner_CCAhandler(this); // create a object of FI CCA handler
   poclTuner_CCAhandler_Diag = OSAL_NEW clTuner_CCAhandler_Diag(this);
   pclInstance  = this;

   m_pHSI_Radio   =  NULL;

   //FM Seek Test -- variables to store DP values
   OSALUTIL_s32SaveNPrintFormat(pcStationName,PSNAME_SIZE_IN_BYTES,"");
   bTPSeekFlag             = FALSE; // introduced to avoid sending TP_FOUND event unnecessarily.
   u8StationBand           = 0;
   u32StationFreVal        = 0;
   u8StationBandLevel      = 0;
   bStationFlag            = FALSE;
   u8StaticListElementNum  = 0;
   bCheck                  = FALSE;
   bRegFlag                = FALSE;
   
}

/*************************************************************************
* FUNCTION: clFi_TUNER:: pclGetInstance()
*
* DESCRIPTION: this is the interface to get the pointer of the singleton
*              object , it creates the singlton object for 1st time. 
*
* PARAMETER:   NONE
*
* RETURNVALUE: clFi_TUNER *  ( pointer to the singleton object)
*
*************************************************************************/

clFi_TUNER * clFi_TUNER::pclGetInstance(tVoid)
{
   /*
   //if the singleton object is not created 
   if ( pclInstance == NULL ) 
   {
      pclInstance = OSAL_NEW clFi_TUNER();  // create the singleton object

      u16InstanceCnt = 1;  // make the counter one 
      
   }
   else  //if already created
   {
      if( u16InstanceCnt != MAX_U16_VALUE )  // to avoid roll back to zero
      {
         u16InstanceCnt++; // just increment the counter
      }
   }
   */
   
   return pclInstance;
}

/*************************************************************************
*  FUNCTION:    getTracePointer()
*
* DESCRIPTION:  returns the trace object pointer.
*
* PARAMETER:   NONE
*
* RETURNVALUE: clITrace *
*
*************************************************************************/
clITrace* clFi_TUNER::getTracePointer(tVoid ) const
{
   return m_poTrace;
}


/*************************************************************************
*  FUNCTION:    u16GetNumberOfHandler
*
* DESCRIPTION:  this function will be called by CCA gateway to get the  
*                number of services this FI uses.
*
* PARAMETER:   NONE
*
* RETURNVALUE: tU16  ( NO. of services needed by this FI)
*
*************************************************************************/

tU16 clFi_TUNER::u16GetNumberOfHandler(tVoid)
{   
   return TUN_FI_NUM_OF_SERVICES ;
}

/*************************************************************************
*  FUNCTION:    vGetHandlerData
*
* DESCRIPTION: this function will be called TUN_FI_NUM_OF_SERVICES's times by 
*              CCA gateway to get the details of Service Id , version NO.s etc 
*                
* PARAMETER:    tVoid*  pHandler
*               tU16*   u16ServiceID 
*               tU16*   u16MajorVersion 
*               tU16*   u16MinorVersion 
*               tChar** szFIName
*
* RETURNVALUE: NONE
*
*************************************************************************/
tVoid clFi_TUNER::vGetHandlerData( tU16 u16Handler,      tVoid** pHandler, 
                                  tU16* u16ServiceID,   tU16* u16ServerAppId,
                                  tU16* u16MajorVersion,tU16* u16MinorVersion,
                                  const tChar** szFIName, tU16* u16HandlerType )
{
   switch (u16Handler)
   {
   case TUN_CONTROL: 
      {
         /* fill all the details in the Parameterd pointers */
         
         *pHandler         = poclTuner_CCAhandler;
         *u16ServiceID     = CCA_C_U16_SRV_TUNERCONTROL; //Tuner service Id  
         *u16ServerAppId   = CCA_C_U16_APP_TUNER;        //Tuner Server APP Id
         *szFIName         = "FI_Tuner";
		 *u16MajorVersion  = MIDW_TUNERFI_C_U16_SERVICE_MAJORVERSION;
		 m_poTrace->vTrace( TR_LEVEL_HMI_INFO, TR_CLASS_HSI_AUDIO_MSG, "TUN_CONTROL: *u16MajorVersion==  %d", *u16MajorVersion);
         *u16MinorVersion  = MIDW_TUNERFI_C_U16_SERVICE_MINORVERSION;
		 m_poTrace->vTrace( TR_LEVEL_HMI_INFO, TR_CLASS_HSI_AUDIO_MSG, "TUN_CONTROL: *u16*u16MinorVersion==  %d", *u16MinorVersion);
		 *u16HandlerType   = (tU16)HANDLER_EN_CLIENT;          //this is a client

		 if( poclTuner_CCAhandler != NULL )
		 {
			 poclTuner_CCAhandler->bInitCCAHandlerGeneral( 
				 this,
				 m_poTrace,
				 *u16ServerAppId,
				 *u16ServiceID,
				 *u16MajorVersion,
				 *u16MinorVersion);
		 }
         break;
      } 
   case TUN_DIAG:  
      {
         /* fill all the details in the Parameterd pointers */
         
         *pHandler         = poclTuner_CCAhandler_Diag;
         *u16ServiceID     = CCA_C_U16_SRV_TUNERDIAG; //Tuner service Id  
         *u16ServerAppId   = CCA_C_U16_APP_TUNER;        //Tuner Server APP Id
         *szFIName         = "FI_Tuner Diag";
         *u16MajorVersion  = TUN_SERVICE_MAJOR_NO;
         *u16MinorVersion  = TUN_SERVICE_MINOR_NO;
         *u16HandlerType   = (tU16)HANDLER_EN_CLIENT;          //this is a client
		    if(poclTuner_CCAhandler_Diag!=NULL)
            {
               poclTuner_CCAhandler_Diag->bInitCCAHandlerGeneral( 
                  this,
                  m_poTrace,
                  *u16ServerAppId,
                  *u16ServiceID,
                  *u16MajorVersion,
                  *u16MinorVersion);
            }  
         break;
      } // more cases may come later
      
   default:
      break;
   }
}


/*************************************************************************
*  FUNCTION:    vServiceStateChanged
*
* DESCRIPTION: CCA gateway will call this function to pass the Service state 
*              of the server. FI will pass this info to its handler. 
*
* PARAMETER:   tUqq uServiceState
*
* RETURNVALUE: NONE
*
*************************************************************************/
tVoid clFi_TUNER::vServiceStateChanged(tU16 u16ServiceID, tU8 u8ServiceState)
{
   if (m_poTrace  != NULL)
   {
      tU16  u16Array[] =
      {
         u16ServiceID,
         (tU16)u8ServiceState
      };
      m_poTrace->vTrace(TR_LEVEL_HMI_INFO, TR_CLASS_HSI_TUNER_MSG,(tU16)FI_RADIO_SERVICE_STATE_CHANGED_1,2,u16Array);
   }

   if( CCA_C_U16_SRV_TUNERCONTROL == u16ServiceID )
   {
      if( NULL != poclTuner_CCAhandler)
      {
         /* store the service state of Server in Handler */
         poclTuner_CCAhandler->vSetServiceState(u8ServiceState);
      } 

   /* convey the status info of the Tuner Server Availability to 
      VR_HSI via DataPool -- added by Pavan R */
      
      if( AMT_C_U8_SVCSTATE_AVAILABLE == u8ServiceState )
      {
         // for security send a unfix to MW
         if (pHSI_GetRadio()  != NULL)
         {
            if (m_poTrace  != NULL)
            {
               m_poTrace->vTrace(TR_LEVEL_HMI_INFO, TR_CLASS_HSI_TUNER_MSG,(tU16)FI_RADIO_SERVICE_STATE_CHANGED_2);
            }
            sMsg Msg (NULL, (tU32)CMMSG_RADIO_UNFIX_ACTIVE_STATION_NAME, 0, 0);
            pHSI_GetRadio()->bExecuteMessage(Msg);
            // recalling the List only when upreg is already done
            if (pHSI_GetRadio()->bGetUpReg_G_MIXEDPRESET_LIST_ELEMENT())
            {
               if (m_poTrace  != NULL)
               {
                  m_poTrace->vTrace(TR_LEVEL_HMI_INFO, TR_CLASS_HSI_TUNER_MSG,"LOAD PRESET LIST: vServiceStateChanged");
               }
               pHSI_GetRadio()->vLoadList(clHSI_Radio::LT_PRESET);
            }


			if(NULL != m_poDataPool)
			{
				/** read the datapool Element value and send the right Auto Compare parameters command to Radio Lib */
				tU8 u8CurrentBank = INVALID_PRESET_NUMBER;
				m_poDataPool->u32dp_get( DPTUNER__CURRENT_BANK,
					&u8CurrentBank, 
					sizeof(u8CurrentBank));

				if((FM1_BANK == u8CurrentBank) || (FM2_BANK == u8CurrentBank) || (AM_BANK == u8CurrentBank))
				{
					if (m_poTrace  != NULL)
					{
						m_poTrace->vTrace(TR_LEVEL_HMI_INFO, TR_CLASS_HSI_TUNER_MSG,"AUTOCOMPARE PARAMETER COMMAND to Radio LIB from FI%d",u8CurrentBank);
					}
					pHSI_GetRadio()->vSwitchBankAutoCompareInCM(u8CurrentBank);
				}	

				else
				{
					// Nothing to do
				}
			}
         }

         /* send the error status to VR_HSI */
         WriteDataToDp(TUNER_DATA_SERVICE_STATE , HSI_TUN_IDLE);

#ifdef TUNSMT__AVAILABLE__TUN         
         if(NULL != m_poEventEngine)
         {
            m_poEventEngine->bEvent2Engine( TUNSMT__AVAILABLE__TUN );
         }
#endif
      }
      else if( AMT_C_U8_SVCSTATE_NOT_AVAILABLE == u8ServiceState )
      {
         /* send the error status to VR_HSI */
         WriteDataToDp(TUNER_DATA_SERVICE_STATE , HSI_TUN_NOT_AVAILABLE);

#ifdef TUNSMT__UNAVAILABLE__TUN
         if(NULL != m_poEventEngine)
         {
            m_poEventEngine->bEvent2Engine( TUNSMT__UNAVAILABLE__TUN );
         }
#endif
         
		  // set seek state to 0 during low voltage added by mba4cob

		 poclTuner_CCAhandler->bSeekState = 0;

		 poclTuner_CCAhandler->bDirectFreq = false;
		 
         // Interactive FM seek test -- write values on service unavailable         
         
         tBool bIndex = TRUE;
         m_poDataPool -> u32dp_get( DPTUNER__FEATURE_FMSEEK,
                                    &bIndex,sizeof(tBool) );         
         
         if(FALSE == bIndex)// Check whether FM seek test is ON or OFF
         {
            bCheck = TRUE;
            vFMSeekTestReadWrite(FMSEEK_TEST_WRITE_VALUES);            
         }
         
      }
   }
   else if( CCA_C_U16_SRV_TUNERDIAG == u16ServiceID )
   {
      if( NULL != poclTuner_CCAhandler_Diag)
      {
         /* store the service state of Server in Handler */
         poclTuner_CCAhandler_Diag->vSetServiceState(u8ServiceState);
         if( AMT_C_U8_SVCSTATE_AVAILABLE == u8ServiceState )
         {
            poclTuner_CCAhandler_Diag->bRegisterForAllFID();
         }
      }
   }

   if (u16ServiceID == CCA_C_U16_SRV_TUNERCONTROL) // if related to Tuner service
   { 
      if ( m_poTrace != NULL )
      {
         m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
                            TR_CLASS_HSI_TUNER_MSG, 
                            (tU16)TUN_SERVICE_STATE,
                            ONE_ELEMENT,
                            &u8ServiceState);
      }

      if (AMT_C_U8_SVCSTATE_AVAILABLE == u8ServiceState )
      {
         // Invoking the Functional Registration.
         if(poclTuner_CCAhandler != NULL)
         {
            poclTuner_CCAhandler->bRegisterForAllFID();
         }
      }
      
   }
}


/*************************************************************************
*  FUNCTION:    vSetRegisterID
*
* DESCRIPTION: CCA gateway will call this function to pass the Register ID
*              sent by the server. FI will pass this info to its handler.
*
* PARAMETER:   tU16 u16RegisterID 
*
* RETURNVALUE: NONE
*
*************************************************************************/
tVoid clFi_TUNER::vSetRegisterID(tU16 u16ServiceID, tU16 u16RegisterID)
{
   /* check weather service ID is the required one or not */
   
   if (u16ServiceID == CCA_C_U16_SRV_TUNERCONTROL)
   { 
      if( poclTuner_CCAhandler !=NULL )
      {
         poclTuner_CCAhandler->vSetRegisterId(u16RegisterID); // pass the Reg ID to handler
      }
      
      if ( m_poTrace != NULL )
      {
         m_poTrace->vTrace(  TR_LEVEL_HMI_INFO,
                             TR_CLASS_HSI_TUNER_MSG,
                             (tU16)TUN_REG_ID, 
                             ONE_ELEMENT,
                             &u16RegisterID );
      }
   }
   else if(u16ServiceID == CCA_C_U16_SRV_TUNERDIAG)
   {
      if( poclTuner_CCAhandler_Diag != NULL )
      {
         poclTuner_CCAhandler_Diag->vSetRegisterId(u16RegisterID); // pass the Reg ID to handler
      }
      
      if ( m_poTrace != NULL )
      {
         m_poTrace->vTrace(  TR_LEVEL_HMI_INFO,
                             TR_CLASS_HSI_TUNER_MSG," CCA_C_U16_SRV_TUNERDIAG u16RegisterID =%d",u16RegisterID );
      }
   }
   else
   {
      /* VR support from Tuner -- send the error status to VR_HSI */
      WriteDataToDp(TUNER_DATA_SERVICE_STATE , HSI_TUN_NOT_AVAILABLE);
   }
}

/*************************************************************************
*  FUNCTION:    pvGetCCAMain
*
* DESCRIPTION: returns the pointer of CCA gateway stored in the class member. 
*
* PARAMETER:   tVoid
*
* RETURNVALUE: tVoid*  ( pointer of CCA gateway type casted as void *)
*
*************************************************************************/
tVoid* clFi_TUNER::pvGetCCAMain(tVoid)
{
   return (tVoid *)m_poCCAMainCallback;
}


/*************************************************************************
*
* FUNCTION: clFi_Tuner::SendEventsToEventEngine() 
* 
* DESCRIPTION: this interface will be used by the handler
*               to Send events to Event engine (general )
*
* PARAMETER: u32FunctionId - FID of message, u32Data- to decide the event
*
* RETURNVALUE: NONE
*
*************************************************************************/
tVoid clFi_TUNER::SendEventsToEvtEn(tU32 u32FunctionId, tU32 u32Data) 
{
   u32Data = u32Data;

   m_poEventEngine->bEvent2Engine(u32FunctionId);
   return ;  /* this function is splitted into separate handlers 
               for the specifc use ,so no implementation here */
}


/*************************************************************************
*
* FUNCTION: clFi_TUNER::ReceiveEventsFromEvtEn(tU32 u32Event) 
* 
* DESCRIPTION: HSI Manager will call this function to pass the event from 
*              event engine by diverting events to the corresponding FI's
*
* PARAMETER: u32Event ( EVENT to this FI )
*
* RETURNVALUE: NONE
*
*************************************************************************/
tVoid clFi_TUNER::ReceiveEventsFromEvtEn(tU32 u32Event) 
{
   
   if ( m_poTrace != NULL )
   {
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO, 
                         TR_CLASS_HSI_TUNER_MSG,
                         (tU16)TUN_RECVD_EVT,
                         ONE_ELEMENT,
                         &u32Event );
   }
   
   /** check weather the Tuner_CCA handler pointer is valid or not before proceeding **/

   if( poclTuner_CCAhandler != NULL ) 
   {
      /** depending on the Event call the respective handler_funtions of the 
          Tuner_CCAhandler object with the required parameters               **/
           

      switch(u32Event)
      {      
      case  HSI__TUN__ALLOW_TPSEEK_TIMER: // ALLOW-TPSEEK-Timer
         {
            if (pHSI_GetRadio() !=  NULL)
            {
               sMsg  refMsg(NULL,(tU32)CMMSG_RADIO_ALLOWTPSEEK_TIMER,   0, 0);
               pHSI_GetRadio()->bExecuteMessage(refMsg);
            }

            if ( m_poTrace != NULL )
            {
               m_poTrace->vTrace( TR_LEVEL_HMI_INFO, 
                                  TR_CLASS_HSI_TUNER_MSG,
                                  (tU16)FI_RADIO_ALLOW_TPSEEK_TIMER);
            }
         }
         break;
         
      case HSI__TUN__INIT: //call the registration procedure
         {             
            //poclTuner_CCAhandler->bRegisterForAllFID(); 
            
            break;
         }
         
     /*case HSI__TUN__RADIO_ANTENNA_TEST:  // Put the tuner into foreground
         {
            // here Tuner sent to Foreground because of User  
            tU8 u8Antenna = TUN_PHANTOM_ANTENNA;
            if(poclTuner_CCAhandler_Diag!= NULL)
            {
              // poclTuner_CCAhandler_Diag->vHandleFID_S__RF_ANTENNA_TEST_To_FCTuner(u8Antenna);
            }
            
            break;
         }

	 case HSI__TUN__RADIO_DAB_ANTENNA_TEST:
		 {
			 tU8 u8Antenna = TUN_DAB_ANTENNA;
            if(poclTuner_CCAhandler_Diag!= NULL)
            {
               //poclTuner_CCAhandler_Diag->vHandleFID_S__RF_ANTENNA_TEST_To_FCTuner(u8Antenna);
            }
			 break;
		 }*/
              
      case HSI__TUN__SEEK_NEXT_SHORT: // increment freq by one step size
         {         
               poclTuner_CCAhandler->vHandleFID_S_ATFREQUENCY_To_FCTuner(
		         (tU8)midw_fi_tcl_e8_Tun_ManualFreqAdjustment::FI_EN_TUN_SEEK_DIRECTION_UP, 
				 NO_OF_STEPS, (tU8)midw_fi_tcl_e8_Tun_TunerHeader::FI_EN_TUN_FOREGROUND,
				 (tU8)midw_fi_tcl_e8_Tun_AcceptAF::FI_EN_TUN_NOT_ACCEPT_AF );
            
            break;
         }
         
      
      case HSI__TUN__SEEK_PRV_SHORT: // decrement freq by one step size
         {     
            poclTuner_CCAhandler->vHandleFID_S_ATFREQUENCY_To_FCTuner(
               (tU8)midw_fi_tcl_e8_Tun_ManualFreqAdjustment::FI_EN_TUN_SEEK_DIRECTION_DOWN, 
			   NO_OF_STEPS, (tU8)midw_fi_tcl_e8_Tun_TunerHeader::FI_EN_TUN_FOREGROUND,
			   (tU8)midw_fi_tcl_e8_Tun_AcceptAF::FI_EN_TUN_NOT_ACCEPT_AF );
            break;
         }
         
                 
      case HSI__TUN__SEEK_NEXT_LONG: //increment freq continuosly untill stop Event 
         {
            poclTuner_CCAhandler->vHandleFID_S_ATSEEK_WITH_TUNERHEADER_To_FCTuner(
				   (tU8)midw_fi_tcl_e8_Tun_TunerSeek::FI_EN_TUN_SEEKTYPE_UP_CONTINOUS,
				   (tU8)midw_fi_tcl_e8_Tun_TunerHeader::FI_EN_TUN_FOREGROUND,
				   0x00);
            
            break;
         }

      /* stop the current frequency seeking actions */
      case HSI__TUN__STOP_SCAN :         // stop the band scan
      case HSI__TUN__SEEK_PRV_LONG_REL:  // stop continous seek    
      case HSI__TUN__SEEK_NEXT_LONG_REL: // stop continous seek
         /* common functionality for all these cases */
         {
            if ( m_poTrace != NULL )
            {
               m_poTrace->vTrace(  TR_LEVEL_HMI_INFO,
                  TR_CLASS_HSI_TUNER_MSG,
                  "*** REMOVED SOME CODE ***" );
            }
/*
            poclTuner_CCAhandler->vHandleFID_S_ATSEEK_WITH_TUNERHEADER_To_FCTuner(
				midw_fi_tcl_e8_Tun_TunerSeek::FI_EN_TUN_SEEKTYPE_OFF,
				midw_fi_tcl_e8_Tun_PhysicalTuner::FI_EN_TUNER_0,
				0x00	);
*/
            break;
         }
         
      case HSI__TUN__SEEK_PRV_LONG: //decrement freq continuosly untill stop Event
         {
            poclTuner_CCAhandler->vHandleFID_S_ATSEEK_WITH_TUNERHEADER_To_FCTuner(
				   (tU8)midw_fi_tcl_e8_Tun_TunerSeek::FI_EN_TUN_SEEKTYPE_DOWN_CONTINOUS,
				   (tU8)midw_fi_tcl_e8_Tun_TunerHeader::FI_EN_TUN_FOREGROUND,
				   0x00	);
            
            break;
         }
         
                
      case HSI__TUN__BANDSEL__FM1:  // selct the FM1 staticlist  
         {
            poclTuner_CCAhandler->vHandleFID_S_SET_STATIC_LIST_To_FCTuner(
				   (tU8)midw_fi_tcl_e8_Tun_TunerStaticList::FI_EN_TUN_TUN_STATIC_LIST_FM1 );
            
            break;
         }
                  
      case HSI__TUN__BANDSEL__FM2: // selct the FM2 staticlist  
         {
            poclTuner_CCAhandler->vHandleFID_S_SET_STATIC_LIST_To_FCTuner(
				   (tU8)midw_fi_tcl_e8_Tun_TunerStaticList::FI_EN_TUN_TUN_STATIC_LIST_FM2 );
            
            break;
         }
         
      case HSI__TUN__BANDSEL__FM3: // selct the FM3 staticlist  
         {
            poclTuner_CCAhandler->vHandleFID_S_SET_STATIC_LIST_To_FCTuner(
				   (tU8)midw_fi_tcl_e8_Tun_TunerStaticList::FI_EN_TUN_TUN_STATIC_LIST_FM3 );
            
            break;
         }
         
      case HSI__TUN__BANDSEL__AST: // selct the AST staticlist
         {
            poclTuner_CCAhandler->vHandleFID_S_SET_STATIC_LIST_To_FCTuner(
				   (tU8)midw_fi_tcl_e8_Tun_TunerStaticList::FI_EN_TUN_TUN_STATIC_LIST_AST );
            
            break;
         }
         
      case HSI__TUN__BANDSEL__MW:  // selct the MW staticlist
         { 
            poclTuner_CCAhandler->vHandleFID_S_SET_STATIC_LIST_To_FCTuner(
				   (tU8)midw_fi_tcl_e8_Tun_TunerStaticList::FI_EN_TUN_TUN_STATIC_LIST_AM1 );
            
            break;
         }
         
      case HSI__TUN__BANDSEL__LW: // selct the LW staticlist
         {
            poclTuner_CCAhandler->vHandleFID_S_SET_STATIC_LIST_To_FCTuner(
				   (tU8)midw_fi_tcl_e8_Tun_TunerStaticList::FI_EN_TUN_TUN_STATIC_LIST_LW1 ); 
            
            break;
         }
         
         
      case HSI__TUN__SEEK_AUTO_NEXT: // auto seek to next station
         {
            // PI Stepping added by Pavan R
            tBool bPiStepping = FALSE;
            
            bPiStepping = bCheckPiStepping();
            
            if(FALSE == bPiStepping)
            {
               poclTuner_CCAhandler->vHandleFID_S_ATSEEK_WITH_TUNERHEADER_To_FCTuner(
                  (tU8)midw_fi_tcl_e8_Tun_TunerSeek::FI_EN_TUN_SEEKTYPE_UP_AUTO,
			         (tU8)midw_fi_tcl_e8_Tun_TunerHeader::FI_EN_TUN_FOREGROUND,
		   	      0x00	);
            }
            else 
            {
               poclTuner_CCAhandler->vHandleFID_S_SET_PI_To_FCTuner(
				   midw_fi_tcl_e8_Tun_Abs_Or_Rel::FI_EN_TUN_PI_REL,
				   midw_fi_tcl_e8_Tun_Rel_Steps::FI_EN_TUN_PI_INC);
            }
            
            break;
         }
         
      case HSI__TUN__SEEK_AUTO_PRV: // auto seek to prev station
         {
            // PI Stepping added by Pavan R
            tBool bPiStepping = FALSE;
            
            bPiStepping = bCheckPiStepping();
            
            if(FALSE == bPiStepping)
            {
               poclTuner_CCAhandler->vHandleFID_S_ATSEEK_WITH_TUNERHEADER_To_FCTuner(
                  (tU8)midw_fi_tcl_e8_Tun_TunerSeek::FI_EN_TUN_SEEKTYPE_DOWN_AUTO,
			         (tU8)midw_fi_tcl_e8_Tun_TunerHeader::FI_EN_TUN_FOREGROUND,
			         0x00	);
            }
            else 
            {
               poclTuner_CCAhandler->vHandleFID_S_SET_PI_To_FCTuner(
                              midw_fi_tcl_e8_Tun_Abs_Or_Rel::FI_EN_TUN_PI_REL,
                              midw_fi_tcl_e8_Tun_Rel_Steps::FI_EN_TUN_PI_DEC);
            }
            
            break;
         }
         
         /***************Enable/disable FEATUREs ********************/ 
         /* first parameter : Feature , second parameter : Mask (on/off) */
         
      case HSI__TUN__FEATURE_SHARX_ON:  // enable sharx
         {
            poclTuner_CCAhandler->vHandleFID_S_SET_FEATURE_To_FCTuner(
                                                      TUNER_FEATURE_SHARX ,
                                                      TUNER_FEATURE_SHARX  );
            
            break;
         }
         
      case HSI__TUN__FEATURE_SHARX_OFF:  // disable sharx
         {          
            poclTuner_CCAhandler->vHandleFID_S_SET_FEATURE_To_FCTuner(
                                                      TUNER_FEATURE_SHARX ,
                                                      TUNER_FEATURE_OFF_MASK );
            
            break;
         }
         
      case HSI__TUN__FEATURE_FMSTEP_ON: // enable FMStep (50 khz) 
         {         
            poclTuner_CCAhandler->vHandleFID_S_SET_FEATURE_To_FCTuner(
                                                       TUNER_FEATURE_FMSTEP ,
                                                       TUNER_FEATURE_FMSTEP );
            
            break;
         }
         
      case HSI__TUN__FEATURE_FMSTEP_OFF: // disable FMStep (100 khz)
         {
            poclTuner_CCAhandler->vHandleFID_S_SET_FEATURE_To_FCTuner(
                                                       TUNER_FEATURE_FMSTEP ,
                                                       TUNER_FEATURE_OFF_MASK );
            
            break;
         }
         
      case HSI__TUN__FEATURE_AF_ON:  // enable AF
         {
            poclTuner_CCAhandler->vHandleFID_S_SET_FEATURE_To_FCTuner( 
                                                         TUNER_FEATURE_AF ,
                                                         TUNER_FEATURE_AF );
			ETG_TRACE_USR4(("FI_TUNER :: HSI__TUN__FEATURE_AF_ON "));
            
            break;
         }
         
      case HSI__TUN__FEATURE_AF_OFF: //diable AF
         {
            poclTuner_CCAhandler->vHandleFID_S_SET_FEATURE_To_FCTuner(
                                                       TUNER_FEATURE_AF ,
                                                       TUNER_FEATURE_OFF_MASK );
			ETG_TRACE_USR4(("FI_TUNER :: HSI__TUN__FEATURE_AF_OFF "));
            
            break;
         }

	 case HSI__TUN__FEATURE_DDA_ON:  // enable DDA
         {
            poclTuner_CCAhandler->vHandleFID_S_SET_FEATURE_To_FCTuner( 
                                                         TUNER_FEATURE_DDA ,
                                                         TUNER_FEATURE_DDA );
			ETG_TRACE_USR4(("FI_TUNER :: HSI__TUN__FEATURE_DDA_ON "));
            
            break;
         }
         
      case HSI__TUN__FEATURE_DDA_OFF: //diable DDA
         {
            poclTuner_CCAhandler->vHandleFID_S_SET_FEATURE_To_FCTuner(
                                                       TUNER_FEATURE_DDA ,
                                                       TUNER_FEATURE_OFF_MASK );
			ETG_TRACE_USR4(("FI_TUNER :: HSI__TUN__FEATURE_DDA_OFF "));
            
            break;
         }
         
      case HSI__TUN__MEASURE_MODE_ON:
        {
        	poclTuner_CCAhandler->vHandleFID_S_SET_FEATURE_To_FCTuner(
        	                                             TUNER_FEATURE_MEASURE_MODE ,
        	                                             TUNER_FEATURE_MEASURE_MODE );
			ETG_TRACE_USR4(("FI_TUNER :: HSI__TUN__MEASURE_MODE_ON "));

			break;

        }

      case HSI__TUN__MEASURE_MODE_OFF:
        {
             poclTuner_CCAhandler->vHandleFID_S_SET_FEATURE_To_FCTuner(
              	                                         TUNER_FEATURE_MEASURE_MODE ,
              	                                         TUNER_FEATURE_OFF_MASK );
			ETG_TRACE_USR4(("FI_TUNER :: HSI__TUN__MEASURE_MODE_OFF "));

			break;

        }



         
      case HSI__TUN__FEATURE_PSNAME_ON: // enable varying PSname
         {        
            poclTuner_CCAhandler->vHandleFID_S_SET_FEATURE_To_FCTuner(
                                                   TUNER_FEATURE_PSNAME ,
                                                   TUNER_FEATURE_PSNAME );
            
            break;
         }
         
      case HSI__TUN__FEATURE_PSNAME_OFF:  // disable varying PSname
         {         
            poclTuner_CCAhandler->vHandleFID_S_SET_FEATURE_To_FCTuner( 
                                                   TUNER_FEATURE_PSNAME ,
                                                   TUNER_FEATURE_OFF_MASK );
            
            break;
         }
         
      case HSI__TUN__FEATURE_TM_DATA_MODE_ON:  // ask servr to send continous Testmode data
      case HSI__TUN__FEATURE_TM_DATA_MODE_ON_FG:
         {
         	tBool bTM = TRUE;

         	ETG_TRACE_USR4(("FI_TUNER :: HSI__TUN__FEATURE_TM_DATA_MODE_ON_FG "));

             if( NULL != m_poDataPool )
                 {

                     m_poDataPool -> u32dp_set(DPTUNER__FEATURE_TESTMODE,
                           	                                   &bTM ,
                           	                                   sizeof(bTM) );
                 }

        	 poclTuner_CCAhandler->vHandleFID_S_SET_FEATURE_To_FCTuner(
                                                   TUNER_FEATURE_TM_DATA ,
                                                   TUNER_FEATURE_TM_DATA );
            
            break;
         }

      case HSI__TUN__FEATURE_TM_DATA_MODE_ON_BG:
         {
			 /* Commented by Rayalu as it is not required for VW */
             // poclTuner_CCAhandler->u8TunMode = FI_EN_TUNER_1 ;

            
            poclTuner_CCAhandler->vHandleFID_S_SET_FEATURE_To_FCTuner( 
                                                   TUNER_FEATURE_TM_DATA ,
                                                   TUNER_FEATURE_TM_DATA );
            
            break;
         }
         
         
      case HSI__TUN__FEATURE_TM_DATA_MODE_OFF: //ask servr to stop sending Testmode data
         {

             tBool bTM = FALSE;

             ETG_TRACE_USR4(("FI_TUNER :: HSI__TUN__FEATURE_TM_DATA_MODE_OFF "));
             if( NULL != m_poDataPool )
                 {

                     m_poDataPool -> u32dp_set(DPTUNER__FEATURE_TESTMODE,
                           	                                   &bTM ,
                           	                                   sizeof(bTM) );
                 }


        	 poclTuner_CCAhandler->vHandleFID_S_SET_FEATURE_To_FCTuner(
                                                     TUNER_FEATURE_TM_DATA ,
                                                     TUNER_FEATURE_OFF_MASK );
            
            break;
         }
         
      case HSI__TUN__FEATURE_TM_EXIT_MODE: // ask servr to stop sending Testmode data and complete exit from the testmode
        {
        	tBool bTM = FALSE;

        	ETG_TRACE_USR4(("FI_TUNER :: HSI__TUN__FEATURE_TM_EXIT_MODE "));
            if( NULL != m_poDataPool )
                {

                    m_poDataPool -> u32dp_set(DPTUNER__FEATURE_TESTMODE,
                          	                                   &bTM ,
                          	                                   sizeof(bTM) );
                }


        	poclTuner_CCAhandler->vHandleFID_S_SET_FEATURE_To_FCTuner(
        	                                                     TUNER_FEATURE_TM_DATA ,
        	                                                     TUNER_FEATURE_OFF_MASK );
        	poclTuner_CCAhandler->vHandleFID_S_SET_FEATURE_To_FCTuner(
        			                                             TUNER_FEATURE_PERSISTENT_STORAGE ,
        			                                             TUNER_FEATURE_PERSISTENT_STORAGE );

            poclTuner_CCAhandler->vHandleFID_S_SET_FEATURE_To_FCTuner(
             	                                         TUNER_FEATURE_MEASURE_MODE ,
             	                                         TUNER_FEATURE_OFF_MASK );

        	tU8 u8Mode = midw_fi_tcl_e8_DDADDS_Mode::FI_EN_TUN_AUTO;  // customer mode

        	if( NULL != m_poDataPool)
        	{

        	      m_poDataPool->u32dp_set( DPTUNER__MODE_SETUP,
        	         &u8Mode,
        	         sizeof(u8Mode)	);

        	}

        	poclTuner_CCAhandler->vHandleFID_S_SET_DDADDS_To_FCTuner(u8Mode);

        	break;

        }


      case HSI__TUN__FEATURE_TM_RADIO_ON: // enable AMFM setup and disable persistent storage on the press of RADIO in testmode
        {
        	tBool bTM = TRUE;

        	ETG_TRACE_USR4(("FI_TUNER :: HSI__TUN__FEATURE_TM_RADIO_ON "));

            if( NULL != m_poDataPool )
                {

                    m_poDataPool -> u32dp_set(DPTUNER__FEATURE_TESTMODE,
                          	                                   &bTM ,
                          	                                   sizeof(bTM) );
                }



        	poclTuner_CCAhandler->vHandleFID_S_SET_FEATURE_To_FCTuner(
        			                                              TUNER_FEATURE_AMFMSETUP ,
        			                                              TUNER_FEATURE_AMFMSETUP );
            poclTuner_CCAhandler->vHandleFID_S_SET_FEATURE_To_FCTuner(
        	        			                                  TUNER_FEATURE_PERSISTENT_STORAGE ,
        	        			                                  TUNER_FEATURE_OFF_MASK );
            break;
        }




      case HSI__TUN__FEATURE_TA_ON:  //enable TA
         {          

        	 ETG_TRACE_USR4(("FI_TUNER :: HSI__TUN__FEATURE_TA_ON "));
        	 poclTuner_CCAhandler->vHandleFID_S_SET_FEATURE_To_FCTuner(
                                                     TUNER_FEATURE_TA ,
                                                     TUNER_FEATURE_TA );
            
            break;
         }
         
      case HSI__TUN__FEATURE_TA_OFF: //disable TA
         {           

        	 ETG_TRACE_USR4(("FI_TUNER :: HSI__TUN__FEATURE_TA_OFF "));

        	 poclTuner_CCAhandler->vHandleFID_S_SET_FEATURE_To_FCTuner(
                                                      TUNER_FEATURE_TA , 
                                                      TUNER_FEATURE_OFF_MASK );
            
            break;
         }
         
         
      case HSI__TUN__FEATURE_REG_ON: // enable regional
         {

        	 ETG_TRACE_USR4(("FI_TUNER :: HSI__TUN__FEATURE_REG_ON "));

        	 poclTuner_CCAhandler->vHandleFID_S_SET_FEATURE_To_FCTuner(
                                                      TUNER_FEATURE_REG ,
                                                      TUNER_FEATURE_REG );
            
            break;
         }
         
      case HSI__TUN__FEATURE_REG_OFF:  //disable regional
         {

        	 ETG_TRACE_USR4(("FI_TUNER :: HSI__TUN__FEATURE_REG_OFF "));

        	 poclTuner_CCAhandler->vHandleFID_S_SET_FEATURE_To_FCTuner(
                                                      TUNER_FEATURE_REG ,
                                                      TUNER_FEATURE_OFF_MASK );
            
            break;
         }
         
      case HSI__TUN__FEATURE_REG_AUTO_ON: //enable regional AUTO
         {

        	 ETG_TRACE_USR4(("FI_TUNER :: HSI__TUN__FEATURE_REG_AUTO_ON "));

        	 poclTuner_CCAhandler->vHandleFID_S_SET_FEATURE_To_FCTuner(
                                                      TUNER_FEATURE_REG_AUTO ,
                                                      TUNER_FEATURE_REG_AUTO );
            
            break;
         }
         
      case HSI__TUN__FEATURE_REG_AUTO_OFF: //disable regional AUTO
         {

        	 ETG_TRACE_USR4(("FI_TUNER :: HSI__TUN__FEATURE_REG_AUTO_OFF "));

        	 poclTuner_CCAhandler->vHandleFID_S_SET_FEATURE_To_FCTuner(
                                                      TUNER_FEATURE_REG_AUTO ,
                                                      TUNER_FEATURE_OFF_MASK );
            
            break;
         }

      case HSI__TUN__FEATURE_CALIBRATION_DATA:
      {
    	  ETG_TRACE_USR4(("FI_TUNER :: HSI__TUN_FEATURE_CALIBRATION "));

    	  poclTuner_CCAhandler->vHandleFID_S_GET_CALIBRATION_To_FCTuner() ;

    	  break;

      }

      /** Added by Pavan R**/
      case HSI__TUN__FEATURE_TMC_ON:  //enable TMC
         {
            poclTuner_CCAhandler->vHandleFID_S_SET_FEATURE_To_FCTuner( 
                                                      TUNER_FEATURE_TMC_MODE ,
                                                      TUNER_FEATURE_TMC_MODE );
            
            break;
         }
      case HSI__TUN__FEATURE_TMC_OFF: // disable TMC
         {
            poclTuner_CCAhandler->vHandleFID_S_SET_FEATURE_To_FCTuner( 
                                                      TUNER_FEATURE_TMC_MODE ,
                                                      TUNER_FEATURE_OFF_MASK );
            
            break;
         }
      case HSI__TUN__FEATURE_MONO_ON:  //enable mono
         {
            poclTuner_CCAhandler->vHandleFID_S_SET_FEATURE_To_FCTuner( 
                                                      TUNER_FEATURE_MONO_MODE ,
                                                      TUNER_FEATURE_MONO_MODE );
            
            break;
         }
      case HSI__TUN__FEATURE_MONO_OFF:  //disable mono
         {
            poclTuner_CCAhandler->vHandleFID_S_SET_FEATURE_To_FCTuner( 
                                                      TUNER_FEATURE_MONO_MODE ,
                                                      TUNER_FEATURE_OFF_MASK );
            
            break;
         }
      case HSI__TUN__FEATURE_RADIOTEXT_ON: //enable radio text
         {
            poclTuner_CCAhandler->vHandleFID_S_SET_FEATURE_To_FCTuner( 
                                                      TUNER_FEATURE_RT_MODE ,
                                                      TUNER_FEATURE_RT_MODE );
            
            break;
         }
      case HSI__TUN__FEATURE_RADIOTEXT_OFF:  //disable radiotext
         {
            poclTuner_CCAhandler->vHandleFID_S_SET_FEATURE_To_FCTuner( 
                                                      TUNER_FEATURE_RT_MODE ,
                                                      TUNER_FEATURE_OFF_MASK );
            
            break;
         }
      case HSI__TUN__FEATURE_SEEK_SENSITIVITY_ON: // enable seek sensitivity
         {
            poclTuner_CCAhandler->vHandleFID_S_SET_FEATURE_To_FCTuner( 
                                                      TUNER_FEATUER_SEEK_SENSE ,
                                                      TUNER_FEATUER_SEEK_SENSE );
            
            break;
         }
      case HSI__TUN__FEATURE_SEEK_SENSITIVITY_OFF:  // disable seeksensitivity
         {
            poclTuner_CCAhandler->vHandleFID_S_SET_FEATURE_To_FCTuner( 
                                                      TUNER_FEATUER_SEEK_SENSE ,
                                                      TUNER_FEATURE_OFF_MASK );
            
            break;
         }

#ifdef DPTUNER__FEATURE_SHARX_LEVEL

         /* pass the sharx value to server  */
      case HSI__TUN__CHGFEATURE_SHARX_LEVEL:
         {
            
        	 ETG_TRACE_USR4(("FI_TUNER :: HSI__TUN__CHGFEATURE_SHARX_LEVEL "));

        	 tU8 u8SharxValue = 0;// a local variable to read the sharx value from server

            if(m_poDataPool != NULL)  // read from the data pool
            {
               m_poDataPool -> u32dp_get( DPTUNER__FEATURE_SHARX_LEVEL,
                                          &u8SharxValue,
                                          sizeof(u8SharxValue)    );


               poclTuner_CCAhandler->vHandleFID_S_SET_SHARXLEVEL_To_FCTuner(u8SharxValue);
            }
            break;
         }
#endif

         /** till here Pavan R **/
                 
        /* pass the hicut value to server  */
         
      case HSI__TUN__CHGFEATURE_HIGHCUT:
         {

        	 ETG_TRACE_USR4(("FI_TUNER :: HSI__TUN__CHGFEATURE_HIGHCUT "));

        	 tU8 u8HicutValue =0 ; // a local variable to read the hicut value from server
           
            if(m_poDataPool != NULL)  // read from the data pool
            {
               m_poDataPool -> u32dp_get( DPTUNER__FEATURE_HIGHCUT,
                                          &u8HicutValue,
                                          sizeof(u8HicutValue) );

                poclTuner_CCAhandler->vHandleFID_S_SET_HICUT_To_FCTuner( u8HicutValue ) ;
            }
                     
            
            break;
            
         }
         
        
         

         

         
         
         
      case HSI__TUN__STOP_ANNOUNCEMENT_ALL:  // stop all the Announcements
         {
            poclTuner_CCAhandler->vHandleFID_S_STOP_ANNOUNCEMENT_To_FCTuner( 
                                                  TUNER_ANNOUNCEMENT_STOP_ALL );
            
            break;
         }
         
      case HSI__TUN__STOP_ANNOUNCEMENT_CUR:
         {         
            poclTuner_CCAhandler->vHandleFID_S_STOP_ANNOUNCEMENT_To_FCTuner(
                                                  TUNER_ANNOUNCEMENT_STOP_CUR );
            
            break;
         }

        

      case HSI__TUN__SET_FREQUENCY:    // set the frequency to the desired value
         {
               
#ifdef DPTUNER__DIRECT_FREQUENCY

            tU32 u32Frequency = 0;    // local variable to read the freq from DP
            
            if(m_poDataPool!=NULL)  // read from the Datapool
            {

               m_poDataPool -> u32dp_get(DPTUNER__DIRECT_FREQUENCY,
                                         &u32Frequency,
                                         sizeof(u32Frequency) );

               poclTuner_CCAhandler->vHandleFID_S_DIRECT_FREQUENCY_To_FCTuner(
                                                                 u32Frequency );
            }
#endif
            break;
         }


      case HSI__TUN__SAVE_STATIC_LISTELEMENT:
         {

#ifdef DPTUNER__STATICLIST_ELEMENT_NUM

            tU8 u8Element = 0;

            if(m_poDataPool!=NULL)  // read from the Datapool
            {
               m_poDataPool -> u32dp_get(DPTUNER__STATICLIST_ELEMENT_NUM,
                                         &u8Element,
                                         sizeof(u8Element) );
            
               poclTuner_CCAhandler->vHandleFID_S_SAVE_STATIC_LIST_ELEM_To_FCTuner(
                                                                      u8Element );
            }
#endif           
            break;

         }//end of case


      case HSI__TUN__AUTOSTRORE_STATIC_LIST:
         {
            poclTuner_CCAhandler->vHandleFID_S_AUTOSTORE_STATIC_LIST_To_FCTuner(
                                                          FORD_HSRNS_AST_LIST_ID );
            
            break;

         }//end of case


      case HSI__TUN__SEL_STATIC_LISTELEMENT:
         {

#ifdef DPTUNER__STATICLIST_ELEMENT_NUM

            tU8 u8Element = 0;

            if(m_poDataPool!=NULL)  // read from the Datapool
            {
               m_poDataPool -> u32dp_get(DPTUNER__STATICLIST_ELEMENT_NUM,
                                         &u8Element,
                                         sizeof(u8Element) );
#ifdef VARIANT_S_FTR_ENABLE_HMI_TUNER_ITG5
               poclTuner_CCAhandler->vHandleFID_S_SEL_LIST_ELEM_To_FCTuner(
               				      (tU8)midw_fi_tcl_e8_Tun_ListType::FI_EN_TUN_LIST_TYPE_STATIC,
                                 u8Element, TUN_INVALID_LISTID  );
#else
               poclTuner_CCAhandler->vHandleFID_S_SEL_LIST_ELEM_To_FCTuner(
				      (tU8)midw_fi_tcl_e8_Tun_ListType::FI_EN_TUN_LIST_TYPE_STATIC,
                  u8Element  );
#endif

            }
#endif            
            break;

         }//end of case

         //Case added by Pavan -- this event is for MFL
         case HSI__TUN__SELECT_PRESET:
         {
            tU8 u8Element = 0;
            if(m_poDataPool != NULL)  // read from the Datapool
            {
               m_poDataPool -> u32dp_get(DPTUNER__STATICLIST_ELEMENT_NUM,
                                         &u8Element,
                                         sizeof(u8Element) );
               if(u8Element < TUNER_DATA_NO_OF_PRESETS)
               {
                  u8Element++;
               }
               else
               {
                  u8Element = 1;
               }
#ifdef VARIANT_S_FTR_ENABLE_HMI_TUNER_ITG5
               poclTuner_CCAhandler->vHandleFID_S_SEL_LIST_ELEM_To_FCTuner(
               				      (tU8)midw_fi_tcl_e8_Tun_ListType::FI_EN_TUN_LIST_TYPE_STATIC,
                                 u8Element , TUN_INVALID_LISTID );
#else
               poclTuner_CCAhandler->vHandleFID_S_SEL_LIST_ELEM_To_FCTuner(
				      (tU8)midw_fi_tcl_e8_Tun_ListType::FI_EN_TUN_LIST_TYPE_STATIC,
                  u8Element  );
#endif
            }
            break;
         }


      case HSI__TUN__LOAD_FM_CONFIG_LIST:
         {         
            poclTuner_CCAhandler->vHandleFID_S_MAKE_CONFIG_LIST_To_FCTuner(LOAD_LIST ,
               (tU8)TUN_LIST_TYPE_FM,
					(tU8)midw_fi_tcl_e8_Tun_Config_Filters::FI_EN_TUN_CONFIG_NO_FILTER);

            break;
         }

      case HSI__TUN__NEXT_FM_CONFIG_LIST:
         {         
            poclTuner_CCAhandler->vHandleFID_S_MAKE_CONFIG_LIST_To_FCTuner(NEXT_LIST , TUN_LIST_TYPE_FM);
            
            break;
         }
         
      case HSI__TUN__PREV_FM_CONFIG_LIST:
         {         
            poclTuner_CCAhandler->vHandleFID_S_MAKE_CONFIG_LIST_To_FCTuner(PREV_LIST , TUN_LIST_TYPE_FM);
            
            break;
         }

 

      case HSI__TUN__LOAD_FMLIST_FROM_INDEX:
         {
            tU8 u8Index = 0;
            
            if(m_poDataPool != NULL)  // read from the Datapool
            {
               m_poDataPool -> u32dp_get( DPTUNER__LIST_LOADING_INDEX,
                                          &u8Index,
                                          sizeof(u8Index) );
        
                           
              poclTuner_CCAhandler->vHandleFID_S_MAKE_CONFIG_LIST_To_FCTuner( SET_LIST_INDEX,
                                                                              TUN_LIST_TYPE_FM,
                                                                              u8Index   );

              if ( m_poTrace != NULL )
              {
                 m_poTrace->vTrace(  TR_LEVEL_HMI_INFO,
                    TR_CLASS_HSI_TUNER_MSG,
                    "**Load index = %d",u8Index );
              }
                                                               
            }
            
            break;
            
         }

      case HSI__TUN__ACTIVATE_BG_TUNER:
         {
            /*poclTuner_CCAhandler->vHandleFID_S_OPERATION_To_FCTuner(
                                                         TUNER_OPERATION_DISABLE,
                                                         TUNER_OPERATION_BG_TUN 
                                                                   );*/

            break;
         }

      case HSI__TUN__DEACTIVATE_BG_TUNER:
         {
            /*poclTuner_CCAhandler->vHandleFID_S_OPERATION_To_FCTuner(
                                                       TUNER_OPERATION_ENABLE,
                                                       TUNER_OPERATION_BG_TUN 
                                                                  );*/
            
            break;
         }





      case HSI__TUN__FMSEEK_TEST_READ_DP:
         {
            vFMSeekTestReadWrite(FMSEEK_TEST_READ_VALUES);
            break;
         }
      case HSI__TUN__FMSEEK_TEST_WRITE_DP:
         {
            vFMSeekTestReadWrite(FMSEEK_TEST_WRITE_VALUES);
            break; 
         }

        //cases of AF List

      case HSI__TUN__LOAD_AF_LIST:
         {
			   poclTuner_CCAhandler->vHandleFID_S_AFLIST_V2_To_FCTuner( LOAD_LIST ,
				   (tU8)midw_fi_tcl_e8_Tun_Notification::FI_EN_TUN_NOTIFICATION_ON_TEST_MODE_ON);
            break;
         }

      case HSI__TUN__NEXT_AF_LIST:
         {
            poclTuner_CCAhandler->vHandleFID_S_AFLIST_V2_To_FCTuner( NEXT_LIST );
            break;
         }

      case HSI__TUN__PREV_AF_LIST:
         {
            poclTuner_CCAhandler->vHandleFID_S_AFLIST_V2_To_FCTuner( PREV_LIST );
            break;
         }

      case HSI__TUN__SEL_AF_LISTELEMENT:
         {
            if(m_poDataPool != NULL)  // read from the Datapool
            {
               tU8 u8Index = 0;

#ifdef DPTUNER__ALTFREQ_INDEX
               m_poDataPool -> u32dp_get( DPTUNER__ALTFREQ_INDEX,
                                          &u8Index,
                                         sizeof(u8Index) );
#endif
               poclTuner_CCAhandler->vHandleFID_S_AFLIST_V2_To_FCTuner( SET_LIST_INDEX, u8Index );

               //using the alreay existing interface for Direct freq input
               ReceiveEventsFromEvtEn(HSI__TUN__SET_FREQUENCY);
            }

            break;
         }   

      case HSI__TUN__CLOSE_AF_LIST:
         {

			   poclTuner_CCAhandler->vHandleFID_S_AFLIST_V2_To_FCTuner( CLOSE_LIST , (tU8)midw_fi_tcl_e8_Tun_Notification::FI_EN_TUN_NOTIFICATION_OFF);
            break;
            
         }

      case HSI__TUN__LOAD_TMC_LIST:
         {
            poclTuner_CCAhandler->vHandleFID_S_MAKE_CONFIG_LIST_To_FCTuner( LOAD_LIST ,
               (tU8)TUN_LIST_TYPE_TMC ,
					(tU8)midw_fi_tcl_e8_Tun_Config_Filters::FI_EN_TUN_CONFIG_FILTER_TMC);

            break;
         }
         
      case HSI__TUN__NEXT_TMC_LIST:
         {         
            poclTuner_CCAhandler->vHandleFID_S_MAKE_CONFIG_LIST_To_FCTuner(NEXT_LIST , TUN_LIST_TYPE_TMC);
            break;
         }
         
      case HSI__TUN__PREV_TMC_LIST:
         {         
            poclTuner_CCAhandler->vHandleFID_S_MAKE_CONFIG_LIST_To_FCTuner(PREV_LIST , TUN_LIST_TYPE_TMC);
            break;
         }

	  case HSI__TUN__SET_TUNER_MODE_DDSDDA:
		  {
				tU8 u8TunerMode = 0;// a local variable to read the Mode value from server
			    
				if(m_poDataPool != NULL)  // read from the data pool
				{
			      m_poDataPool -> u32dp_get( DPTUNER__MODE_SETUP,
					   						   &u8TunerMode,
											   sizeof(u8TunerMode) );
               
               poclTuner_CCAhandler->vHandleFID_S_SET_DDADDS_To_FCTuner(u8TunerMode);
				}
            break;
		   }
      case HSI__TUN__SEEK_NEXT_FROM_TESTMODE:
         {
            tBool bActiveTuner = FALSE;
#ifdef DPTUNER__TESTMODE_ACTIVE_TUNER
		      m_poDataPool->u32dp_get( DPTUNER__TESTMODE_ACTIVE_TUNER, 
			                     &bActiveTuner, 
								 sizeof(bActiveTuner)	);
#endif
			   poclTuner_CCAhandler->vHandleFID_S_ATSEEK_WITH_TUNERHEADER_To_FCTuner(
				   (tU8)midw_fi_tcl_e8_Tun_TunerSeek::FI_EN_TUN_SEEKTYPE_UP_AUTO,
					bActiveTuner,
					0x00);
			   break;
         }
       
	
      case HSI__TUN__SEEK_PREV_FROM_TESTMODE:
         {
            tBool bActiveTuner = FALSE;
#ifdef DPTUNER__TESTMODE_ACTIVE_TUNER
		      m_poDataPool->u32dp_get( DPTUNER__TESTMODE_ACTIVE_TUNER, 
			                     &bActiveTuner, 
								 sizeof(bActiveTuner)	);
#endif
			   poclTuner_CCAhandler->vHandleFID_S_ATSEEK_WITH_TUNERHEADER_To_FCTuner( 
				   (tU8)midw_fi_tcl_e8_Tun_TunerSeek::FI_EN_TUN_SEEKTYPE_DOWN_AUTO,
					bActiveTuner,
					0x00);
			   break;
         }
       
      case HSI__TUN__DETAILED_TMC_LISTELEMENT:
         {
 
            tU8 u8Element = 0;

            if(m_poDataPool != NULL)  // read from the Datapool
            {
#ifdef DPTUNER__TMC_INDEX
               m_poDataPool -> u32dp_get(DPTUNER__TMC_INDEX,
                                         &u8Element,
                                         sizeof(u8Element) );
#endif
            
               poclTuner_CCAhandler->vHandleFID_S_MAKE_CONFIG_LIST_To_FCTuner(
                                                               GET_LIST_ELEM,
                                                               TUN_LIST_TYPE_TMC,
                                                               u8Element  );

            }
            
            break;
         }
      case HSI__TUN__BAND_CHANGE_FM:
        {
			   poclTuner_CCAhandler->vHandleFID_TUN_BAND_CHANGE_To_FCTuner((tU8)midw_fi_tcl_e8_Tun_TunerBand::FI_EN_TUN_BAND_FM);
        }
        break;
      case HSI__TUN__BAND_CHANGE_AM:
        {
			   poclTuner_CCAhandler->vHandleFID_TUN_BAND_CHANGE_To_FCTuner((tU8)midw_fi_tcl_e8_Tun_TunerBand::FI_EN_TUN_BAND_MW);
        }
        break;
      /*case HSI__TUN__BAND_CHANGE_DAB:
        {
          poclTuner_CCAhandler->vHandleFID_TUN_BAND_CHANGE_To_FCTuner();
        }
        break;*/
        case HSI__TUN__FEATURE_HD_TEST_MODE_DATA:
		{
		  poclTuner_CCAhandler->vHandleFID_TUN_GET_HD_TEST_MODE_DATA();
		}
		break;

         
      default :  // unkown event
         break;
         
         
      } // end of switch (u32Event) 
   }
   

}




/*************************************************************************
*
* FUNCTION: clFi_Tuner::WriteDataToDataPool(tU32 u32FunctionId, tU32 u32Data) 
* 
* DESCRIPTION: General interface for Handler functions to write the data
*              into datapool ,used incase of dataload is less
*
* PARAMETER: tU32 u32FunctionId  tU32 u32Data
*
* RETURNVALUE: 
*
*************************************************************************/
tVoid clFi_TUNER::WriteDataToDp(tU32 u32FunctionId, tU32 u32Data) 
{
   
   /* check validity of datapool engine object pointer before procceding */
   if( NULL != m_poDataPool )
   {
      tU16 u16TraceID = 0;
      
      tU8 u8CommonData = 0;
      
      switch (u32FunctionId)  //switch on FID
      {         
         
      case MIDW_TUNERFI_C_U16_FID_TUN_G_SET_HICUT:
         {
            
            u8CommonData = (tU8)u32Data;  //HicutValue will be in 1byte LSB 
            
            m_poDataPool -> u32dp_set(DPTUNER__FEATURE_HIGHCUT,
               &u8CommonData , 
               sizeof(u8CommonData) );
            
            //u16TraceID = TUN_HICUT_VALUE_WRITTEN;

            ETG_TRACE_USR4(("HICUT : %d"
                  				  , 	u8CommonData ));
            
            
            break;
         }

#ifdef DPTUNER__FEATURE_SHARX_LEVEL

      /** Case added by Pavan R*/
      case MIDW_TUNERFI_C_U16_FID_TUN_G_SET_SHARXLEVEL:
         {
            u8CommonData = (tU8)u32Data;  //SharxValue will be in 1byte LSB 


            
            m_poDataPool -> u32dp_set(DPTUNER__FEATURE_SHARX_LEVEL,
               &u8CommonData , 
               sizeof(u8CommonData) );

            //u16TraceID = TUN_SHARX_LEVEL_VALUE_WRITTEN;

            ETG_TRACE_USR4(("SHARX : %d"
                               , 	u8CommonData ));

            break;
         }

#endif
         

      case MIDW_TUNERFI_C_U16_FID_TUN_G_SET_DDA:
      		  {
      			u8CommonData = (tU8)u32Data;
      			tBool bFlag = (tBool)u8CommonData;
      			m_poDataPool -> u32dp_set(DPTUNER__FEATURE_DDA,
                                         &bFlag,
                                         sizeof(bFlag) );

      			ETG_TRACE_USR4(("PHASE DIVERSITY : %d"
      			                     , 	u8CommonData ));

      			break;

      		  }


      case MIDW_TUNERFI_C_U16_FID_TUN_G_SET_AF:
      		  {
      			u8CommonData = (tU8)u32Data;
      			tBool bFlag = (tBool)u8CommonData;
      			m_poDataPool -> u32dp_set(DPTUNER__FEATURE_AF,
                                         &bFlag,
                                         sizeof(bFlag) );

      			ETG_TRACE_USR4(("AF FOLLOWING : %d"
      			      			     , 	u8CommonData ));

      			break;

      		  }


      case MIDW_TUNERFI_C_U16_FID_TUN_G_SET_MEASUREMODE:
             {
            	u8CommonData = (tU8)u32Data;
            	tBool bFlag = (tBool)u8CommonData;
            	m_poDataPool -> u32dp_set(DPTUNER__FEATURE_MEASUREMODE,
                                         &bFlag,
                                         sizeof(bFlag) );

            	ETG_TRACE_USR4(("MEASURE MODE : %d"
            	      			     , 	u8CommonData ));

            	break;

             }


      case MIDW_TUNERFI_C_U16_FID_TUN_G_SET_RDSREG:
            {

                u8CommonData = (tU8)u32Data;
                tBool bFlag = (tBool)u8CommonData;

                m_poDataPool -> u32dp_set(DPTUNER__FEATURE_REG,
                     &bFlag ,
                     sizeof(bFlag) );

                m_poDataPool -> u32dp_set(DPTUNER__FEATURE_REG_AUTO,
                     &bFlag ,
                     sizeof(bFlag) );

                ETG_TRACE_USR4(("RDSREG : %d"
                      			     , 	u8CommonData ));

                break;

            }


      case MIDW_TUNERFI_C_U16_FID_TUN_G_SET_TA:
           {

                u8CommonData = (tU8)u32Data;
                 tBool bFlag = (tBool)u8CommonData;
                m_poDataPool -> u32dp_set(DPTUNER__FEATURE_TA,
                                          &bFlag,
                                          sizeof(bFlag) );

                ETG_TRACE_USR4(("TA : %d"
                      			     , 	u8CommonData ));

                break;


           }



      case MIDW_TUNERFI_C_U16_FID_TUN_G_AFLIST_V2:
         { 
            u8CommonData = (tU8)u32Data; //AFListLength will be in 1byte LSB 

#ifdef DPTUNER__ALTFREQ_NUM
            m_poDataPool -> u32dp_set(DPTUNER__ALTFREQ_NUM ,
                                      &u8CommonData , 
                                      sizeof(u8CommonData) );
#endif  
            
            //u16TraceID = TUN_TM_AFLISTLENGTH_WRITTEN;

              ETG_TRACE_USR4(("AFListLength : %d"
                                , 	u8CommonData ));
            
          break;
         }
         
      case MIDW_TUNERFI_C_U16_FID_TUN_G_MAKE_CONFIG_LIST:
         {
            u8CommonData = (tU8)u32Data; // data will be in 1byte LSB
            
            m_poDataPool -> u32dp_set(DPTUNER__LIST_TOTALELEMENTS,
               &u8CommonData , 
               sizeof(u8CommonData) );
            
            //u16TraceID = TUN_FMLISTLENGTH_WRITTEN;

            
            ETG_TRACE_USR4(("CONFIG LIST : %d"
                                            , 	u8CommonData ));
            
            break;
         }
         
#ifdef VR_SRV_ENABLE
         // done by Pavan Rauri
         
      case TUNER_HSI_ERROR_SERVER_BUSY:
      case TUNER_HSI_ERROR_INVALID_PARAM:
      case TUNER_HSI_NO_ERROR:
      case TUNER_HSI_ERROR_UNKNOWN:
      case TUNER_HSI_ERROR_ACTION_FAILED:
         {
            
            m_poDataPool -> u32dp_set(DPVC__CMD_STATUS, 
               &u32Data,sizeof(u32Data) );
            
            if ( m_poTrace != NULL )
            {
               m_poTrace->vTrace(  TR_LEVEL_HMI_INFO,
                  TR_CLASS_HSI_TUNER_MSG,
                  (tU16)TUN_VR_UPDATE ,1,&u32Data);
            }
            break;
            
         } // end case done by Pavan rauri
         
      /** Case added by Pavan R*/
      case TUNER_DATA_SERVICE_STATE:
         {
            u8CommonData = (tU8)u32Data;

            tU8 u8ServiceState = 0;
            
            m_poDataPool -> u32dp_set(DPTUNER__VRTUNER_STATUS, 
               &u8CommonData,sizeof(u8CommonData) );
            
            if(NULL != poclTuner_CCAhandler)
            {
               u8ServiceState = poclTuner_CCAhandler->vGetServiceState();
            }
            
            /** Print trace based on service state */
            if(AMT_C_U8_SVCSTATE_NOT_AVAILABLE == u8ServiceState)
            {
               u16TraceID = TUN_SERVICE_STATE_NOT_AVAILABLE;
            }
            else if(AMT_C_U8_SVCSTATE_AVAILABLE == u8ServiceState)
            {
               u16TraceID = TUN_SERVICE_STATE_AVAILABLE;
            }
            
            break;
         }
#endif
         
      case MIDW_TUNERFI_C_U16_FID_TUN_G_OPERATION:
         {
            
            u8CommonData = (tU8)u32Data;
            
            u8CommonData = ( u8CommonData & TUNER_OPERATION_BG_TUN)? FALSE :TRUE ;
            
#ifdef DPTUNER__FEATURE_BGTUNER
            m_poDataPool -> u32dp_set(DPTUNER__FEATURE_BGTUNER,
               &u8CommonData , 
               sizeof(u8CommonData) );
            
#endif
            
            u16TraceID = TUN_BGTUNER_STATUS_WRITTEN;
            
            break;
         }
         

         
      case TUNER_HSI_TMC_INFO:
         {
            u8CommonData = (tU8)u32Data;
            tBool flag = (tBool)u32Data;
#ifdef DPTUNER__TMC_INFO
            m_poDataPool -> u32dp_set(DPTUNER__TMC_INFO,
               &flag,
               sizeof(flag) );
#endif
            //u16TraceID = TUN_TMC_INFO_WRITTEN;

            ETG_TRACE_USR4(("TMC : %d"
                                   , 	u8CommonData ));

            break;
         }

      default:  //unkown FID
         break;
         
      }// end of switch
      
      
      /*if ( (m_poTrace != NULL ) && (u16TraceID != 0) )
      {
         m_poTrace->vTrace(  TR_LEVEL_HMI_INFO,
            TR_CLASS_HSI_TUNER_MSG,
            (tU16)u16TraceID,
            ONE_ELEMENT,
            &u8CommonData);
      }*/



    }//end of IF
    
    
    return ;
    
}


/*************************************************************************
*
*  FUNCTION:   clFi_TUNER::WriteStringDataToDp( ) 
* 
*  DESCRIPTION:  separate function to handle writng string to datapool
*
*  PARAMETER:   tU32 u32DataElem
*               tChar* pcBuffer
*
*  RETURNVALUE: NONE
*
*************************************************************************/
tVoid clFi_TUNER::WriteStringDataToDp(tU32 u32DataElem, tChar* pcBuffer) const 
{
   if( NULL == pcBuffer )
   {
      if ( NULL != m_poTrace )
      {
         m_poTrace->vTrace(  TR_LEVEL_HMI_INFO,
                             TR_CLASS_HSI_TUNER_MSG,
                             (tU16)TUN_WRITE_STRING_NULL_POINTER );
      }
      return;
   }
   
     
   switch ((tU8)u32DataElem)
   {
   
   case STATION_DATA_STATIONNAME:      //common functionality for both cases
      {         
         /*** PS NAME **/
         if( NULL != m_poDataPool ) 
         {
            tU32 u32Size   = m_poDataPool->u32dp_getElementSize(DPTUNER__STATION_NAME);
            m_poDataPool->u32dp_set( DPTUNER__STATION_NAME,
                                     pcBuffer, 
                                     u32Size );
									 
            ETG_TRACE_USR4(("WriteStringDataToDp : %8s ",pcBuffer));
         }         
         
         break;
      }

   case TM_DATA_PSNAME:
      {
		  tU8 u8TunerMode = 0;
		  
		  /** Assign Tuner Mode to Local Variable */
		  if(poclTuner_CCAhandler)
		  {
			  u8TunerMode = poclTuner_CCAhandler->u8TunMode; 
		  }

		  if( u8TunerMode == (tU8)midw_fi_tcl_e8_Tun_PhysicalTuner::FI_EN_TUNER_0)
			{

				if(m_poDataPool != NULL)
				{
					m_poDataPool -> u32dp_set( DPTUNER__TESTMODE_PS_TUNER_0,
						                       pcBuffer ,               //write PS NAme
											   OSAL_u32StringLength( pcBuffer) + 1 ); // PSName size
				}

		  }

		  else if( u8TunerMode == (tU8)midw_fi_tcl_e8_Tun_PhysicalTuner::FI_EN_TUNER_1)
		  {

				if(m_poDataPool != NULL)
				{
					m_poDataPool -> u32dp_set( DPTUNER__TESTMODE_PS_TUNER_1,
						                       pcBuffer ,               //write PS Name
											   OSAL_u32StringLength( pcBuffer) + 1 ); // PSName size
				}


		  }
		  
		  else if( u8TunerMode == (tU8)midw_fi_tcl_e8_Tun_PhysicalTuner::FI_EN_TUNER_2)
		  {

				if(m_poDataPool != NULL)
				{
					m_poDataPool -> u32dp_set( DPTUNER__TESTMODE_PS_TUNER_2,
						                       pcBuffer ,               //write PS Name
											   OSAL_u32StringLength( pcBuffer) + 1 ); // PSName size
				}


		  }

		  else
		  {
				if(m_poDataPool != NULL)
				{
					m_poDataPool -> u32dp_set( DPTUNER__TESTMODE_PS_TUNER_3,
						                       pcBuffer ,               //write PS Name
											   OSAL_u32StringLength( pcBuffer) + 1 ); // PSName size
				}

		  }
         break;
      }

      /* some more cases will come later */
   default://invalid functionId
     return ;
      
   } //end of switch

   ETG_TRACE_USR4(("WriteStringDataToDp : %8s ",pcBuffer));

   return ;
} 



  
  

/*************************************************************************
*
* FUNCTION: tVoid clFi_TUNER::ReadDataFromDp() 
* 
* DESCRIPTION: To read read data from Data Pool
*
* PARAMETER: NONE
*
* RETURNVALUE: void
*
*************************************************************************/

tVoid clFi_TUNER::ReadDataFromDp() 
{
   
   
}




/*************************************************************************
*
* FUNCTION: clFi_Tuner::SendEventsOnTunerDataAnnouncements() 
* 
* DESCRIPTION: this interface will be used by the handler
*               to Send events to Event engine for TunerData Announcements
*
* PARAMETER: tU8 u8Announcement ( data to decide the event)
*
* RETURNVALUE: NONE
*
*************************************************************************/
tVoid clFi_TUNER::SendEventsOnTunerDataAnnouncements(tU8 u8Announcement) const
{
   tU32 u32Event =0;

   tU16 u16TraceID = 0;

   /* check validity of event engine object pointer before procceding */
   if(m_poEventEngine != NULL)
   {      
	   switch((tU16)u8Announcement) //switch on diff type of Announcements
      {
         case (tU16)midw_fi_tcl_e8_Tun_Announcement::FI_EN_TUN_NO_ANNOUNCEMENT:
            {          
//             u32Event = TUNSMT__TUN_NO_ANNOUNCEMENT ;
               u16TraceID = TUN_SENT_EVT_NO_ANNOUNCEMENT;
               break;
            }
         
         case (tU16)midw_fi_tcl_e8_Tun_Announcement::FI_EN_TUN_ANNOUNCEMENT_TA + (tU16)midw_fi_tcl_e8_Tun_Announcement::FI_EN_TUN_ANNOUNCEMENT_TA_ACTIVE:
            {            
//             u32Event = TUNSMT__TUN_ANNOUNCEMENT_TA ;
               u16TraceID = TUN_SENT_EVT_ANNOUNCEMENT_TA;
               break;
            }

         case (tU16)midw_fi_tcl_e8_Tun_Announcement::FI_EN_TUN_ANNOUNCEMENT_PTY_31 :         
         case (tU16)midw_fi_tcl_e8_Tun_Announcement::FI_EN_TUN_ANNOUNCEMENT_PTY_31 + (tU16)midw_fi_tcl_e8_Tun_Announcement::FI_EN_TUN_ANNOUNCEMENT_TA_ACTIVE:
            {                 
//             u32Event = TUNSMT__TUN_ANNOUNCEMENT_PTY31 ;
               u16TraceID = TUN_SENT_EVT_ANNOUNCEMENT_PTY31;
               break;
            }
         default: //unkown announcement
            return ; // no further processing .
      } // end of switch

      m_poEventEngine->bEvent2Engine( u32Event );
            
      if ( m_poTrace != NULL )
      {
         m_poTrace->vTrace(  TR_LEVEL_HMI_INFO,
                             TR_CLASS_HSI_TUNER_MSG,
                             (tU16)u16TraceID );
      }

      
   } //end of if(NULL != m_poEventEngine )

   return;
      
} 

/*************************************************************************
*
* FUNCTION: clFi_Tuner::SendEventsOnTunerDataListID() 
* 
* DESCRIPTION: this interface will be used by the handler
*               to Send events to Event engine for TunerData staticListID
*
* PARAMETER: tU8 u8StaticListID  - to decide the event
*
* RETURNVALUE: NONE
*
*************************************************************************/
tVoid clFi_TUNER::SendEventsOnTunerDataListID( tU8 u8StaticListID) const
{
    
   tU32 u32Event = 0;
   
   /* check validity of event engine object pointer before procceding */
   if(m_poEventEngine != NULL)
   {     
      
	   switch((midw_fi_tcl_e8_Tun_StaticListID::tenType)u8StaticListID) //switch on diff type of StaticListID
      {
	  case midw_fi_tcl_e8_Tun_StaticListID::FI_EN_TUN_TUN_STATIC_LIST_FM1 :
         {
//            u32Event = TUNSMT__BANDSEL_FM1__TUN__OK;
            
            break;
         }
      case midw_fi_tcl_e8_Tun_StaticListID::FI_EN_TUN_TUN_STATIC_LIST_FM2 :
         {
//            u32Event = TUNSMT__BANDSEL_FM2__TUN__OK;
            
            break;
         }
         
      case midw_fi_tcl_e8_Tun_StaticListID::FI_EN_TUN_TUN_STATIC_LIST_FM3 :
         {
//            u32Event = TUNSMT__BANDSEL_FM3__TUN__OK;
            
            break;
         }
         
      case midw_fi_tcl_e8_Tun_StaticListID::FI_EN_TUN_TUN_STATIC_LIST_AST :
         {
//            u32Event = TUNSMT__BANDSEL_AST__TUN__OK;
            
            break;
         }

      case midw_fi_tcl_e8_Tun_StaticListID::FI_EN_TUN_TUN_STATIC_LIST_AM1 :
         {
//            u32Event = TUNSMT__BANDSEL_MW__TUN__OK;
            
            break;
         }

      case midw_fi_tcl_e8_Tun_StaticListID::FI_EN_TUN_TUN_STATIC_LIST_LW1 :
         {
//            u32Event = TUNSMT__BANDSEL_LW__TUN__OK;
            
            break;
         }

      default: // unkown StaticListId
         {
           return ; // no need to process further

         }
      } //end of switch
            
      
      m_poEventEngine->bEvent2Engine( u32Event );

      if (( m_poTrace != NULL ) )
      {
         m_poTrace->vTrace(  TR_LEVEL_HMI_INFO,
                             TR_CLASS_HSI_TUNER_MSG, 
                             (tU16)TUN_EVT_SENT_FOR_SLISTID);
      }

   }// end of if

   return;
}


/*************************************************************************
*
* FUNCTION: clFi_Tuner::SendEventsOnTunerDataBand() 
* 
* DESCRIPTION: this interface will be used by the handler
*               to Send events to Event engine for TunerData Band info
*
* PARAMETER:  tU8 u8Band  - to decide the event
*
* RETURNVALUE: NONE
*
*************************************************************************/
tVoid clFi_TUNER::SendEventsOnTunerDataBand(tU8 u8Band) const
{
   tU32 u32Event = 0;
   
   /* check validity of event engine object pointer before procceding */
   if(m_poEventEngine != NULL)
   {     
      
      switch(u8Band) //switch on diff type of Band
      {
	  case midw_fi_tcl_e8_Tun_TunerBand::FI_EN_TUN_BAND_FM :
         {
#ifdef TUNSMT__TUNFM_BAND__TUN
            u32Event = TUNSMT__TUNFM_BAND__TUN;
#endif            
            break;
         }

	  //case midw_fi_tcl_e8_Tun_TunerBand::FI_EN_TUN_BAND_LW:
      case midw_fi_tcl_e8_Tun_TunerBand::FI_EN_TUN_BAND_MW:
         {
#ifdef TUNSMT__TUNAM_BAND__TUN
            u32Event = TUNSMT__TUNAM_BAND__TUN;
#endif
            break;
         }

      default:
         break;
      }
      
      m_poEventEngine->bEvent2Engine( u32Event );

   }

}

/*************************************************************************
*
* FUNCTION: clFi_Tuner::SendEventsOnTunerDataMode(tU32 u32FunctionId,tU32 u32Data) 
* 
* DESCRIPTION: this interface will be used by the handler
*               to Send events to Event engine for TunerData Mode
*
* PARAMETER: u8Mode - data to decide the event
*
* RETURNVALUE: NONE
*
*************************************************************************/
tVoid clFi_TUNER::SendEventsOnTunerDataMode(tU8 u8Mode) const
{
   tBool bTraceFlag = TRUE; // a local Flag to decide on sending trace
   
   /* check validity of event engine object pointer before procceding */
   if(m_poEventEngine != NULL)
   { 
      
	   switch((midw_fi_tcl_e8_Tun_TunerMode::tenType)u8Mode) //switch on diff type of Modes
      {
	   case midw_fi_tcl_e8_Tun_TunerMode::FI_EN_TUN_MODE_NORM:
         {                   
//            m_poEventEngine->bEvent2Engine(TUNSMT__TUN_MODE_NORMAL__TUN);
            
            break;
         }
         
	   case midw_fi_tcl_e8_Tun_TunerMode::FI_EN_TUN_MODE_BANDSCAN:
         {                     
//            m_poEventEngine->bEvent2Engine(TUNSMT__TUN_MODE_BANDSCAN__TUN);
            
            break;
         } 
         /*some more cases will come later */

      case  midw_fi_tcl_e8_Tun_TunerMode::FI_EN_TUN_MODE_STATIC_LISTSCAN:
      case  midw_fi_tcl_e8_Tun_TunerMode::FI_EN_TUN_MODE_CONFIG_LISTSCAN:
      default: // unkown MODE
         {
            bTraceFlag = FALSE;
            break;
         }
         
      }//end of switch
      
      
      
      if ( (bTraceFlag == TRUE ) && (m_poTrace != NULL ))
      {
         m_poTrace->vTrace(  TR_LEVEL_HMI_INFO, 
                             TR_CLASS_HSI_TUNER_MSG, 
                             (tU16)TUN_SENT_EVT_FOR_MODE);
      }

   }
}


/*************************************************************************
*
* FUNCTION: clFi_Tuner::SendEventsOnStationDataPackedInfo(u16PackedInfo) 
* 
* DESCRIPTION: this interface will be used by the handler
*               to Send events to Event engine for PackedInfo of StationData
*
* PARAMETER: u16PackedInfo (Data- to decide the event)
*
* RETURNVALUE: NONE
*
*************************************************************************/
tVoid clFi_TUNER::SendEventsOnStationDataPackedInfo( tU16 u16PackedInfo) const
{
   /* check validity of event engine object pointer before procceding */
   if(m_poEventEngine  != NULL)
   {      
      /* check weather TP_ON bit is 1 by AND logic */ 
      if((u16PackedInfo & PACKEDINFO_TP_ON_BIT) == PACKEDINFO_TP_ON_BIT )
      {  
         if((u16PackedInfo & PACKEDINFO_TP_DECODE_BIT ) == PACKEDINFO_TP_STATE_ON_BIT ) // and state is also ON
         {
            if(TRUE == bTPSeekFlag ) /* needed to avoid sending the event redundantly */ 
            {                  
//               m_poEventEngine->bEvent2Engine( TUNSMT__TP_STATION__TUN);
               
               if ( m_poTrace != NULL )
               {
                  m_poTrace->vTrace(  TR_LEVEL_HMI_INFO,
                                      TR_CLASS_HSI_TUNER_MSG, 
                                      (tU16)TUN_SENT_EVT_FOR_TP_FOUND);
               }
            }
         }
                  
      }
      /* check if any of the TP_LOST, TP_OFF and TP_TEMP_OFF bits is 1 */
      else if( (u16PackedInfo & PACKEDINFO_TPLOST_BIT ) == PACKEDINFO_TPLOST_BIT )   
      {                
//         m_poEventEngine->bEvent2Engine( TUNSMT__TP_LOST__TUN );
         
         if ( m_poTrace != NULL )
         {
            m_poTrace->vTrace(  TR_LEVEL_HMI_INFO, 
                                TR_CLASS_HSI_TUNER_MSG, 
                                (tU16)TUN_SENT_EVT_FOR_TP_LOST);
         }
         
      }
      else if( (u16PackedInfo & PACKEDINFO_TP_OFF_BIT )  ||
               (u16PackedInfo & PACKEDINFO_TP_OFF_TEMP_BIT ) ||
               ((u16PackedInfo & PACKEDINFO_TP_DECODE_BIT) == 0 ) )
      {
#ifdef TUNSMT__TP_LOST__OFF
          m_poEventEngine->bEvent2Engine( TUNSMT__TP_LOST__OFF );
#endif
         
         if ( m_poTrace != NULL )
         {
            m_poTrace->vTrace(  TR_LEVEL_HMI_INFO, 
                                TR_CLASS_HSI_TUNER_MSG, 
                                (tU16)TUN_SENT_EVT_FOR_TP_OFF);
         }
      }

   }

}
      


/*************************************************************************
*
* FUNCTION: clFi_Tuner::WriteTunerDataToDP() 
* 
* DESCRIPTION: this interface will be used by the handler
*               to write TunerData elements into Datapool
*
* PARAMETER: tU8 u8Element , tU8 u8Data  ( element to recognize DP-ID , and the data)
*
* RETURNVALUE: NONE
*
*************************************************************************/
 
tVoid clFi_TUNER::WriteTunerDataToDP( tU8 u8Element , tU8 u8Data ) const
{
   
   tU32 u32DatapoolID = 0;

   tU16 u16TraceID = 0;
   
   /* check validity of datapool engine object pointer before procceding */
   if( NULL != m_poDataPool )
   {
      switch(u8Element )  // switch on the element 
      {
         
      case TUNER_DATA_BAND: // Band information
         {             
            //u32DatapoolID = DPTUNER__STATION_BAND ;
            //
            //u16TraceID = TUN_BAND_WRITTEN;
            //            
            //break;
                      
         }// end of case
         
         
         
      case TUNER_DATA_MODE : // Mode Information
         {
            u32DatapoolID = DPTUNER__TUN_MODE ;
            
            u16TraceID = TUN_MODE_WRITTEN;
                        
            break;
         } // end of case
         
         
      case TUNER_DATA_STATICLISTID:  //Static ListID
         {
            u32DatapoolID = DPTUNER__STATION_BANDLEVEL ;
            
            u16TraceID = TUN_STATIC_LIST_ID_WRITTEN;
                                    
            break;
         }

      case TUNER_DATA_STATICLIST_ELEM_ID:
         {
            
            //u32DatapoolID = DPTUNER__STATICLIST_ELEMENT_NUM;

            //u16TraceID = TUN_STATIC_LIST_ELEMID_WRITTEN;

            break;
         }
         
      case TUNER_DATA_ANNOUNCEMENT:  // TA for Status line indication
         {
      //      u32DatapoolID = DPTUNER__TA_STATUS ;
      //      
      //      u16TraceID = TUN_TA_STATUS_WRITTEN;
      //      
      //      // data to be written is of type Bool 
			   //if (u8Data == ((tU8)midw_fi_tcl_e8_Tun_Announcement::FI_EN_TUN_ANNOUNCEMENT_TA + (tU8)midw_fi_tcl_e8_Tun_Announcement::FI_EN_TUN_ANNOUNCEMENT_TA_ACTIVE) )
      //      {
      //         u8Data = (tU8)TRUE;  //if yes , set to TRUE
      //      }
      //      else 
      //      {
      //         u8Data = (tU8)FALSE; //if NO , set to False
      //      }
                                 
            break;
         }

      case TUNER_DATA_CONFIGLIST_ELEM_ID:   // currently active station in FM List
         {
#ifdef DPTUNER__LIST_ELEMENTINDEX
            u32DatapoolID = DPTUNER__LIST_ELEMENTINDEX;
#endif
            u16TraceID = (tU16)TUN_FMLIST_ELEMENT_WRITTEN;
            break;
         }
         

      default :
         return;  // invalid case , no need to process further.

      }// end of switch

      
      //valid case , write its data to DP
      m_poDataPool -> u32dp_set( u32DatapoolID,
                                 &u8Data, 
                                 sizeof(u8Data) );
            
      if ( m_poTrace != NULL )
      {
         m_poTrace->vTrace(  TR_LEVEL_HMI_INFO,
                             TR_CLASS_HSI_TUNER_MSG,
                             (tU16)u16TraceID,
                             ONE_ELEMENT,
                             &u8Data );
      }
   
   } //end of IF

   return;

}
  


/*************************************************************************
*
* FUNCTION: clFi_Tuner::WriteFeaturesFlagToDP() 
* 
* DESCRIPTION: this interface will be used by the handler
*               to write FeaturesFlag values into Datapool
*
* PARAMETER:  tU32 u32Features ( data to be written)
*
* RETURNVALUE: NONE
*
*************************************************************************/
 
tVoid clFi_TUNER::WriteFeaturesFlagToDP(  tU32 u32Features ) const
{
   /* check validity of datapool engine object pointer before procceding */
   if( NULL != m_poDataPool )
   {        
      tBool bFlag = FALSE ; //a temp Flag to hold the status of the Features 

      //write the AF feature status
      {  
         bFlag = ( (u32Features & TUNER_FEATURE_AF) ? TRUE : FALSE ); //check AF bit
         
         m_poDataPool -> u32dp_set(DPTUNER__FEATURE_AF,
                                   &bFlag,
                                   sizeof(bFlag) );
         
      }
             
      //write the SHARX feature status
      {
         bFlag = ( (u32Features & TUNER_FEATURE_SHARX) ? TRUE : FALSE ); //check Sharx bit
         
         m_poDataPool -> u32dp_set(DPTUNER__FEATURE_SHARX, 
                                   &bFlag , 
                                   sizeof(bFlag) );
          
      }
              
      //write the FMSTEP feature status
      {
         bFlag = ( (u32Features & TUNER_FEATURE_FMSTEP) ? TRUE : FALSE ) ;//check FMstep bit
           
         m_poDataPool -> u32dp_set(DPTUNER__FEATURE_FMSTEP,
                                   &bFlag , 
                                   sizeof(bFlag) );
            
      }
      
      //write the PSNAME feature status
      {
         bFlag = ( (u32Features & TUNER_FEATURE_PSNAME) ? TRUE : FALSE ) ; //check PsName bit
       
         m_poDataPool -> u32dp_set(DPTUNER__FEATURE_PSNAME ,
                                   &bFlag ,
                                   sizeof(bFlag) );

      }


      //write the Testmode data feature status
      {
         bFlag = ( (u32Features & TUNER_FEATURE_TM_DATA) ? TRUE : FALSE ) ; //check TM data bit
		 if ( m_poTrace != NULL )
		{
         m_poTrace->vTrace(  TR_LEVEL_HMI_INFO,
                             TR_CLASS_HSI_TUNER_MSG," write the Testmode data feature status =%d",(tU8)bFlag );
		 
		 m_poDataPool -> u32dp_set(DPTUNER__FEATURE_TESTMODE,
                                   &bFlag ,
                                   sizeof(bFlag) );
		 m_poTrace->vTrace(  TR_LEVEL_HMI_INFO,
                             TR_CLASS_HSI_TUNER_MSG," write the Testmode data feature status11 =%d",(tU8)bFlag );
		}

#ifdef DPTUNER__FEATURE_TM_DATA
         
         m_poDataPool -> u32dp_set(DPTUNER__FEATURE_TM_DATA,
                                   &bFlag ,
                                   sizeof(bFlag) );
#endif

                 
      }

      
      //write the TA feature status
      {
         //bFlag = ( (u32Features & TUNER_FEATURE_TA) ? TRUE : FALSE ) ; //check TA bit
         //
         //m_poDataPool -> u32dp_set(DPTUNER__FEATURE_TA ,
         //                          &bFlag ,
         //                          sizeof(bFlag) );
  
        
      }

      //write the REG feature status
      {
         bFlag = ( (u32Features & TUNER_FEATURE_REG) ? TRUE : FALSE ) ; //check REG bit
                  
         m_poDataPool -> u32dp_set(DPTUNER__FEATURE_REG, 
                                   &bFlag ,
                                   sizeof(bFlag) );
         
                  
      }

      /*** Added by Pavan R ***/
#ifdef DPTUNER__FEATURE_REG_AUTO
      
      //write the REG(AUTO) feature status
      {
         bFlag = ( (u32Features & TUNER_FEATURE_REG_AUTO) ? TRUE : FALSE ) ; //check REG bit
                  
         m_poDataPool -> u32dp_set(DPTUNER__FEATURE_REG_AUTO, 
                                   &bFlag ,
                                   sizeof(bFlag) );
      }

#endif

      //write the TMC feature status
      {
         bFlag = ( (u32Features & TUNER_FEATURE_TMC_MODE) ? TRUE : FALSE ) ; //check REG bit
                  
         m_poDataPool -> u32dp_set(DPTUNER__FEATURE_TMC, 
                                   &bFlag ,
                                   sizeof(bFlag) );
         
                  
      }

      //write the SEEKSENSE feature status
      {
         bFlag = ( (u32Features & TUNER_FEATUER_SEEK_SENSE) ? TRUE : FALSE ) ; //check REG bit
                  
         m_poDataPool -> u32dp_set(DPTUNER__FEATURE_SEEKSENSE, 
                                   &bFlag ,
                                   sizeof(bFlag) );
         
                  
      }

      //write the MONO feature status
      {
         bFlag = ( (u32Features & TUNER_FEATURE_MONO_MODE) ? TRUE : FALSE ) ; //check REG bit
                  
         m_poDataPool -> u32dp_set(DPTUNER__FEATURE_MONO, 
                                   &bFlag ,
                                   sizeof(bFlag) );
         
                  
      }

      //write the RADIO TEXT feature status
      {
         bFlag = ( (u32Features & TUNER_FEATURE_RT_MODE) ? TRUE : FALSE ) ; //check REG bit
                  
         m_poDataPool -> u32dp_set(DPTUNER__FEATURE_RTEXT, 
                                   &bFlag ,
                                   sizeof(bFlag) );
         
                  
      }

	  //write the DDA feature status
      {  
         bFlag = ( (u32Features & TUNER_FEATURE_DDA) ? TRUE : FALSE ); //check AF bit
         
         m_poDataPool -> u32dp_set(DPTUNER__FEATURE_DDA,
                                   &bFlag,
                                   sizeof(bFlag) );
         
      }

      /*** till here  **/

      if ( m_poTrace != NULL )
      {
         m_poTrace->vTrace(  TR_LEVEL_HMI_INFO,
                             TR_CLASS_HSI_TUNER_MSG,
                             (tU16)TUN_FEATURES_WRITTEN );
      }
   }

}

            
/*************************************************************************
*
* FUNCTION: clFi_Tuner::WriteTestModeDataToDP() 
* 
* DESCRIPTION: this interface will be used by the handler
*               to write TestModeData into Datapool
*
* PARAMETER: tU8 u8Element , tU32 u32Data  ( element to recognize DP-ID , and the data)
*
* RETURNVALUE: NONE
*
*************************************************************************/
 
tVoid clFi_TUNER::WriteTestModeDataToDP( tU8 u8Element , tU32 u32Data ) const
{
   /* check validity of datapool engine object pointer before procceding */
	if( NULL != m_poDataPool )
	   {
		   tU8 u8TunerMode = 0;

		   /** Assign Tuner Mode to Local Variable */
		   if(poclTuner_CCAhandler)
		   {
			  u8TunerMode = poclTuner_CCAhandler->u8TunMode;
		   }


	      switch(u8Element )  // switch on the element
	      {

	      case TM_DATA_FIELDSTRENGTH:
	         {
	            tU8 u8Strength = (tU8)u32Data; //strength info will be in 1byte (LSB)

				   if ( u8TunerMode == (tU8)midw_fi_tcl_e8_Tun_PhysicalTuner::FI_EN_TUNER_0)
				   {
					   m_poDataPool -> u32dp_set( DPTUNER__TESTMODE_FIELDSTRENGTH_TUNER_0,
	                                              &u8Strength ,
	                                              sizeof(u8Strength) );
				   }
				   else if  ( u8TunerMode == (tU8)midw_fi_tcl_e8_Tun_PhysicalTuner::FI_EN_TUNER_1)
				   {
					   m_poDataPool -> u32dp_set( DPTUNER__TESTMODE_FIELDSTRENGTH_TUNER_1,
	                                              &u8Strength ,
	                                              sizeof(u8Strength) );
				   }
				   else if  ( u8TunerMode == (tU8)midw_fi_tcl_e8_Tun_PhysicalTuner::FI_EN_TUNER_2)
				   {
					   m_poDataPool -> u32dp_set( DPTUNER__TESTMODE_FIELDSTRENGTH_TUNER_2,
	                                              &u8Strength ,
	                                              sizeof(u8Strength) );
				   }

				   else
				   {
					   m_poDataPool -> u32dp_set( DPTUNER__TESTMODE_FIELDSTRENGTH_TUNER_3,
	                                              &u8Strength ,
	                                              sizeof(u8Strength) );
				   }

	           /* if ( m_poTrace != NULL )
	            {
	               m_poTrace->vTrace(  TR_LEVEL_HMI_INFO,
	                                   TR_CLASS_HSI_TUNER_MSG,
	                                   (tU16)TUN_TM_FSTRENGTH_WRITTEN,
	                                   ONE_ELEMENT,
	                                   &u8Strength);
	            }*/

				  ETG_TRACE_USR4(("TUNER %d FIELDSTRENGTH : %d" , u8TunerMode, u8Strength ));


	            break;
	         }


		  case TM_DATA_BAND:
			  {
				  tU8 u8Band = (tU8)u32Data;

				   if ( u8TunerMode == (tU8)midw_fi_tcl_e8_Tun_PhysicalTuner::FI_EN_TUNER_0)
				   {
					   m_poDataPool -> u32dp_set( DPTUNER__TESTMODE_BAND_TUNER_0,
	                                              &u8Band ,
	                                              sizeof(u8Band) );
				   }
				   else if  ( u8TunerMode == (tU8)midw_fi_tcl_e8_Tun_PhysicalTuner::FI_EN_TUNER_1)
				   {
					   m_poDataPool -> u32dp_set( DPTUNER__TESTMODE_BAND_TUNER_1,
	                                              &u8Band ,
	                                              sizeof(u8Band) );
				   }
				   else if  ( u8TunerMode == (tU8)midw_fi_tcl_e8_Tun_PhysicalTuner::FI_EN_TUNER_2)
				   {
					   m_poDataPool -> u32dp_set( DPTUNER__TESTMODE_BAND_TUNER_2,
	                                              &u8Band ,
	                                              sizeof(u8Band) );
				   }

				   else
				   {
					   m_poDataPool -> u32dp_set( DPTUNER__TESTMODE_BAND_TUNER_3,
	                                              &u8Band ,
	                                              sizeof(u8Band) );
				   }

				   ETG_TRACE_USR4(("TUNER %d BAND : %d" , u8TunerMode, u8Band ));

				   break;
			  }



	      case TM_DATA_MULTIPATH:
	         {
	            tU8 u8Multipath = (tU8)u32Data; //Multipath info will be in 1byte (LSB)


				   if ( u8TunerMode == (tU8)midw_fi_tcl_e8_Tun_PhysicalTuner::FI_EN_TUNER_0)
				   {
					   m_poDataPool -> u32dp_set( DPTUNER__TESTMODE_MULTIPATH_TUNER_0,
	                                              &u8Multipath ,
	                                              sizeof(u8Multipath) );
				   }
				   else if  ( u8TunerMode == (tU8)midw_fi_tcl_e8_Tun_PhysicalTuner::FI_EN_TUNER_1)
				   {
					   m_poDataPool -> u32dp_set( DPTUNER__TESTMODE_MULTIPATH_TUNER_1,
	                                              &u8Multipath ,
	                                              sizeof(u8Multipath) );
				   }
				   else if  ( u8TunerMode == (tU8)midw_fi_tcl_e8_Tun_PhysicalTuner::FI_EN_TUNER_2)
				   {
					   m_poDataPool -> u32dp_set( DPTUNER__TESTMODE_MULTIPATH_TUNER_2,
	                                              &u8Multipath ,
	                                              sizeof(u8Multipath) );
				   }

				   else
				   {
					   m_poDataPool -> u32dp_set( DPTUNER__TESTMODE_MULTIPATH_TUNER_3,
	                                              &u8Multipath ,
	                                              sizeof(u8Multipath) );
				   }

	            /*if ( m_poTrace != NULL )
	            {
	               m_poTrace->vTrace(  TR_LEVEL_HMI_INFO,
	                                   TR_CLASS_HSI_TUNER_MSG,
	                                   (tU16)TUN_TM_MULTIPATH_WRITTEN,
	                                   ONE_ELEMENT,
	                                   &u8Multipath );
	            }*/

				   ETG_TRACE_USR4(("TUNER %d MULTIPATH : %d" , u8TunerMode, u8Multipath ));

	            break;
	         }

	      case TM_DATA_NEIGHBOUR:
	         {
	            tU8 u8Neighbour = (tU8)u32Data; //Neighbour info will be in 1byte (LSB)

				   if ( u8TunerMode == (tU8)midw_fi_tcl_e8_Tun_PhysicalTuner::FI_EN_TUNER_0)
				   {
					   m_poDataPool -> u32dp_set( DPTUNER__TESTMODE_NEIGHBOUR_TUNER_0,
	                                              &u8Neighbour ,
	                                              sizeof(u8Neighbour) );
				   }
				   else if  ( u8TunerMode == (tU8)midw_fi_tcl_e8_Tun_PhysicalTuner::FI_EN_TUNER_1)
				   {
					   m_poDataPool -> u32dp_set( DPTUNER__TESTMODE_NEIGHBOUR_TUNER_1,
	                                              &u8Neighbour ,
	                                              sizeof(u8Neighbour) );
				   }
				   else if  ( u8TunerMode == (tU8)midw_fi_tcl_e8_Tun_PhysicalTuner::FI_EN_TUNER_2)
				   {
					   m_poDataPool -> u32dp_set( DPTUNER__TESTMODE_NEIGHBOUR_TUNER_2,
	                                              &u8Neighbour ,
	                                              sizeof(u8Neighbour) );
				   }

				   else
				   {
					   m_poDataPool -> u32dp_set( DPTUNER__TESTMODE_NEIGHBOUR_TUNER_3,
	                                              &u8Neighbour ,
	                                              sizeof(u8Neighbour) );
				   }

	            /*if ( m_poTrace != NULL )
	            {
	               m_poTrace->vTrace(  TR_LEVEL_HMI_INFO,
	                                   TR_CLASS_HSI_TUNER_MSG,
	                                   (tU16)TUN_TM_NEIGHBOUR_WRITTEN,
	                                   ONE_ELEMENT,
	                                   &u8Neighbour);
	            }*/

				   ETG_TRACE_USR4(("TUNER %d NEIGHBOUR : %d" , u8TunerMode, u8Neighbour ));
	            break;
	         }

	      case TM_DATA_QUALITY:
	         {
	            tU8 u8Quality = (tU8)u32Data; //RDSQuality info will be in 1byte (LSB)

				   if ( u8TunerMode == (tU8)midw_fi_tcl_e8_Tun_PhysicalTuner::FI_EN_TUNER_0)
				   {
					   m_poDataPool -> u32dp_set( DPTUNER__TESTMODE_QUALITY_TUNER_0,
	                                              &u8Quality ,
	                                              sizeof(u8Quality) );
				   }
				   else if  ( u8TunerMode == (tU8)midw_fi_tcl_e8_Tun_PhysicalTuner::FI_EN_TUNER_1)
				   {
					   m_poDataPool -> u32dp_set( DPTUNER__TESTMODE_QUALITY_TUNER_1,
	                                              &u8Quality ,
	                                              sizeof(u8Quality) );
				   }
				   else if  ( u8TunerMode == (tU8)midw_fi_tcl_e8_Tun_PhysicalTuner::FI_EN_TUNER_2)
				   {
					   m_poDataPool -> u32dp_set( DPTUNER__TESTMODE_QUALITY_TUNER_2,
	                                              &u8Quality ,
	                                              sizeof(u8Quality) );
				   }

				   else
				   {
					   m_poDataPool -> u32dp_set( DPTUNER__TESTMODE_QUALITY_TUNER_3,
	                                              &u8Quality ,
	                                              sizeof(u8Quality) );
				   }

	           /* if ( m_poTrace != NULL )
	            {
	               m_poTrace->vTrace(  TR_LEVEL_HMI_INFO,
	                                   TR_CLASS_HSI_TUNER_MSG,
	                                   (tU16)TUN_TM_RDSQUALITY_WRITTEN,
	                                   ONE_ELEMENT,
	                                   &u8Quality);
	            }*/

				   ETG_TRACE_USR4(("TUNER %d QUALITY : %d" , u8TunerMode, u8Quality ));

	            break;
	         }

	      /** case added by Pavan R */
	      case TM_DATA_RDS_ERROR_RATE:
	         {

	            tU8 u8RDSErrorRate = (tU8)u32Data; //RDSErrorrate info will be in 1byte (LSB)

				   if ( u8TunerMode == (tU8)midw_fi_tcl_e8_Tun_PhysicalTuner::FI_EN_TUNER_0)
				   {
					   m_poDataPool -> u32dp_set( DPTUNER__TESTMODE_RDSERRORS_TUNER_0,
	                                              &u8RDSErrorRate ,
	                                              sizeof(u8RDSErrorRate) );
				   }
				   else if  ( u8TunerMode == (tU8)midw_fi_tcl_e8_Tun_PhysicalTuner::FI_EN_TUNER_1)
				   {
					   m_poDataPool -> u32dp_set( DPTUNER__TESTMODE_RDSERRORS_TUNER_1,
	                                              &u8RDSErrorRate ,
	                                              sizeof(u8RDSErrorRate) );
				   }
				   else if  ( u8TunerMode == (tU8)midw_fi_tcl_e8_Tun_PhysicalTuner::FI_EN_TUNER_2)
				   {
					   m_poDataPool -> u32dp_set( DPTUNER__TESTMODE_RDSERRORS_TUNER_2,
	                                              &u8RDSErrorRate ,
	                                              sizeof(u8RDSErrorRate) );
				   }

				   else
				   {
					   m_poDataPool -> u32dp_set( DPTUNER__TESTMODE_RDSERRORS_TUNER_3,
	                                              &u8RDSErrorRate ,
	                                              sizeof(u8RDSErrorRate) );
				   }

	            /*if ( m_poTrace != NULL )
	            {
	               m_poTrace->vTrace(  TR_LEVEL_HMI_INFO,
	                                   TR_CLASS_HSI_TUNER_MSG,
	                                   (tU16)TUN_TM_RDSERRORRATE_WRITTEN,
	                                   ONE_ELEMENT,
	                                   &u8RDSErrorRate);
	            }*/

				ETG_TRACE_USR4(("TUNER %d RDS_ERROR_RATE : %d" , u8TunerMode, u8RDSErrorRate ));

				break;
	         }

	      case TM_AFLIST_LENGTH:
	         {
	            tU8 u8AFListLength = (tU8)u32Data; //AFListLength info will be in 1byte (LSB)

				   if ( u8TunerMode == (tU8)midw_fi_tcl_e8_Tun_PhysicalTuner::FI_EN_TUNER_0)
				   {
					   m_poDataPool -> u32dp_set( DPTUNER__TESTMODE_AFLISTLENGTH_TUNER_0,
	                                              &u8AFListLength ,
	                                              sizeof(u8AFListLength) );
				   }
				   else if  ( u8TunerMode == (tU8)midw_fi_tcl_e8_Tun_PhysicalTuner::FI_EN_TUNER_1)
				   {
					   m_poDataPool -> u32dp_set( DPTUNER__TESTMODE_AFLISTLENGTH_TUNER_1,
	                                              &u8AFListLength ,
	                                              sizeof(u8AFListLength) );
				   }
				   else if  ( u8TunerMode == (tU8)midw_fi_tcl_e8_Tun_PhysicalTuner::FI_EN_TUNER_2)
				   {
					   m_poDataPool -> u32dp_set( DPTUNER__TESTMODE_AFLISTLENGTH_TUNER_2,
	                                              &u8AFListLength ,
	                                              sizeof(u8AFListLength) );
				   }

				   else
				   {
					   m_poDataPool -> u32dp_set( DPTUNER__TESTMODE_AFLISTLENGTH_TUNER_3,
	                                              &u8AFListLength ,
	                                              sizeof(u8AFListLength) );
				   }
	            /*if ( m_poTrace != NULL )
	            {
	               m_poTrace->vTrace(  TR_LEVEL_HMI_INFO,
	                                   TR_CLASS_HSI_TUNER_MSG,
	                                   (tU16)TUN_TM_AFLISTLENGTH_WRITTEN,
	                                   ONE_ELEMENT,
	                                   &u8AFListLength);
	            }*/

				   ETG_TRACE_USR4(("TUNER %d AFListLength : %d" , u8TunerMode, u8AFListLength ));
	            break;
	         }

	      case TM_DATA_PI:
	         {
				   if ( u8TunerMode == (tU8)midw_fi_tcl_e8_Tun_PhysicalTuner::FI_EN_TUNER_0)
				   {
					   m_poDataPool -> u32dp_set( DPTUNER__TESTMODE_PI_TUNER_0,
	                                              &u32Data ,
	                                              sizeof(u32Data) );
				   }
				   else if  ( u8TunerMode == (tU8)midw_fi_tcl_e8_Tun_PhysicalTuner::FI_EN_TUNER_1)
				   {
					   m_poDataPool -> u32dp_set( DPTUNER__TESTMODE_PI_TUNER_1,
	                                              &u32Data ,
	                                              sizeof(u32Data) );
				   }
				   else if  ( u8TunerMode == (tU8)midw_fi_tcl_e8_Tun_PhysicalTuner::FI_EN_TUNER_2)
				   {
					   m_poDataPool -> u32dp_set( DPTUNER__TESTMODE_PI_TUNER_2,
	                                              &u32Data ,
	                                              sizeof(u32Data) );
				   }

				   else
				   {
					   m_poDataPool -> u32dp_set( DPTUNER__TESTMODE_PI_TUNER_3,
	                                              &u32Data ,
	                                              sizeof(u32Data) );
				   }
	            /*if ( m_poTrace != NULL )
	            {
	               m_poTrace->vTrace(  TR_LEVEL_HMI_INFO,
	                                   TR_CLASS_HSI_TUNER_MSG,
	                                   (tU16)TUN_TM_PI_WRITTEN,
	                                   ONE_ELEMENT,
	                                   &u32Data);
	            }*/

				   ETG_TRACE_USR4(("TUNER %d PI : %d" , u8TunerMode, u32Data ));
	            break;
	         }


          case TM_DATA_HICUT :
	         {
	            tU8 u8Value = (tU8) u32Data ; // only 1 byte LSB contains the data

	            /*tU8 u8HiCut =  u8Value & TESTMODE_HICUT_MASK ; // 00000111 ,consider only 3 bits
	            tU8 u8Sharx =  u8Value & TESTMODE_SHARX_MASK ; // 01110000, consider only 3 bits

	            u8Sharx = u8Sharx >> 4 ; */// shift right 4 bits to make 01110000 to 00000111

				   if ( u8TunerMode == (tU8)midw_fi_tcl_e8_Tun_PhysicalTuner::FI_EN_TUNER_0)
				   {
					   m_poDataPool -> u32dp_set( DPTUNER__TESTMODE_HICUT_TUNER_0,
	                                              &u8Value ,
	                                              sizeof(u8Value) );
				   }
				   else if  ( u8TunerMode == (tU8)midw_fi_tcl_e8_Tun_PhysicalTuner::FI_EN_TUNER_1)
				   {
					   m_poDataPool -> u32dp_set( DPTUNER__TESTMODE_HICUT_TUNER_1,
	                                              &u8Value ,
	                                              sizeof(u8Value) );
				   }
				   else if  ( u8TunerMode == (tU8)midw_fi_tcl_e8_Tun_PhysicalTuner::FI_EN_TUNER_2)
				   {
					   m_poDataPool -> u32dp_set( DPTUNER__TESTMODE_HICUT_TUNER_2,
	                                              &u8Value ,
	                                              sizeof(u8Value) );
				   }

				   else
				   {
					   m_poDataPool -> u32dp_set( DPTUNER__TESTMODE_HICUT_TUNER_3,
	                                              &u8Value ,
	                                              sizeof(u8Value) );
				   }




	          /*  if ( m_poTrace != NULL )
	            {

	               m_poTrace->vTrace(  TR_LEVEL_HMI_INFO,
	                                   TR_CLASS_HSI_TUNER_MSG,
	                                   (tU16)TUN_TM_SHARX_WRITTEN,
	                                   TWO_ELEMENT,
	                                   u8Value );

	            }*/

				   ETG_TRACE_USR4(("TUNER %d HICUT : %d" , u8TunerMode, u8Value ));

	            break;
	         }

		 case TM_DATA_SHARX :
	         {
	            tU8 u8Value = (tU8) u32Data ; // only 1 byte LSB contains the data

	            /*tU8 u8HiCut =  u8Value & TESTMODE_HICUT_MASK ; // 00000111 ,consider only 3 bits
	            tU8 u8Sharx =  u8Value & TESTMODE_SHARX_MASK ; // 01110000, consider only 3 bits

	            u8Sharx = u8Sharx >> 4 ; */// shift right 4 bits to make 01110000 to 00000111

				   if ( u8TunerMode == (tU8)midw_fi_tcl_e8_Tun_PhysicalTuner::FI_EN_TUNER_0)
				   {
					   m_poDataPool -> u32dp_set( DPTUNER__TESTMODE_SHARX_TUNER_0,
	                                              &u8Value ,
	                                              sizeof(u8Value) );
				   }
				   else if  ( u8TunerMode == (tU8)midw_fi_tcl_e8_Tun_PhysicalTuner::FI_EN_TUNER_1)
				   {
					   m_poDataPool -> u32dp_set( DPTUNER__TESTMODE_SHARX_TUNER_1,
	                                              &u8Value ,
	                                              sizeof(u8Value) );
				   }
				   else if  ( u8TunerMode == (tU8)midw_fi_tcl_e8_Tun_PhysicalTuner::FI_EN_TUNER_2)
				   {
					   m_poDataPool -> u32dp_set( DPTUNER__TESTMODE_SHARX_TUNER_2,
	                                              &u8Value ,
	                                              sizeof(u8Value) );
				   }

				   else
				   {
					   m_poDataPool -> u32dp_set( DPTUNER__TESTMODE_SHARX_TUNER_3,
	                                              &u8Value ,
	                                              sizeof(u8Value) );
				   }




	            /*if ( m_poTrace != NULL )
	            {

	               m_poTrace->vTrace(  TR_LEVEL_HMI_INFO,
	                                   TR_CLASS_HSI_TUNER_MSG,
	                                   (tU16)TUN_TM_SHARX_WRITTEN,
	                                   TWO_ELEMENT,
	                                   u8Value );

	            }*/

				   ETG_TRACE_USR4(("TUNER %d SHARX : %d" , u8TunerMode, u8Value ));

	            break;
	         }

		  case TM_DATA_CHANNELSEPERATION:
			 {
				 tU8 u8Value = (tU8) u32Data ; // only 1 byte LSB contains the data


				 if ( u8TunerMode == (tU8)midw_fi_tcl_e8_Tun_PhysicalTuner::FI_EN_TUNER_0)
				   {
					   m_poDataPool -> u32dp_set( DPTUNER__TESTMODE_CHANNELSEPERATION_TUNER_0,
	                                              &u8Value ,
	                                              sizeof(u8Value) );
				   }
				   else if  ( u8TunerMode == (tU8)midw_fi_tcl_e8_Tun_PhysicalTuner::FI_EN_TUNER_1)
				   {
					   m_poDataPool -> u32dp_set( DPTUNER__TESTMODE_CHANNELSEPERATION_TUNER_1,
	                                              &u8Value ,
	                                              sizeof(u8Value) );
				   }
				   else if  ( u8TunerMode == (tU8)midw_fi_tcl_e8_Tun_PhysicalTuner::FI_EN_TUNER_2)
				   {
					   m_poDataPool -> u32dp_set( DPTUNER__TESTMODE_CHANNELSEPERATION_TUNER_2,
	                                              &u8Value ,
	                                              sizeof(u8Value) );
				   }

				   else
				   {
					   m_poDataPool -> u32dp_set( DPTUNER__TESTMODE_CHANNELSEPERATION_TUNER_3,
	                                              &u8Value ,
	                                              sizeof(u8Value) );
				   }

				 ETG_TRACE_USR4(("TUNER %d CHANNELSEPERATION : %d" , u8TunerMode, u8Value ));

				   break;
			 }


	      case TM_DATA_INFOBITS:
	         {
	            tU8 u8Value = (tU8) u32Data ; // only 1 byte LSB contains the data
	            tU8 u8TABit = u8Value & TESTMODE_TA_MASK ; // 00000010 , consider only 1 bit
	            tU8 u8TPBit = u8Value & TESTMODE_TP_MASK ; // 00000001, consider only 1 bit
                u8TABit = u8TABit >> 1 ; // shift right 1 bit to make 00000010 to 00000001
                tU8 u8TMCbit = u8Value & TESTMODE_TMC_MASK;
                u8TMCbit = u8TMCbit >> 2 ;
                tU8 u8DDAbit = u8Value & TESTMODE_DDA_MASK;
                u8DDAbit = u8DDAbit >> 3 ;
                tU8 u8AudibleTunerbit = u8Value & TESTMODE_AUDIBLETUNER_MASK;
                u8AudibleTunerbit = u8AudibleTunerbit >> 4 ;
				tU8 u8AudibleTuner = 0;


                if ( u8TunerMode == (tU8)midw_fi_tcl_e8_Tun_PhysicalTuner::FI_EN_TUNER_0)
				   {
					   m_poDataPool -> u32dp_set( DPTUNER__TESTMODE_TA_TUNER_0,
	                                              &u8TABit ,
	                                              sizeof(u8TABit) );
					   m_poDataPool -> u32dp_set( DPTUNER__TESTMODE_TP_TUNER_0,
					  	                          &u8TPBit ,
					  	                          sizeof(u8TPBit) );
					   m_poDataPool -> u32dp_set( DPTUNER__TESTMODE_TMCCOUNT_TUNER_0,
					   	                          &u8TMCbit ,
					   	                          sizeof(u8TMCbit) );
					   m_poDataPool -> u32dp_set( DPTUNER__TESTMODE_DDA_TUNER_0,
					  	                          &u8DDAbit ,
					  	                          sizeof(u8DDAbit) );

						if(u8AudibleTunerbit)
						{
							u8AudibleTuner = 0;
						}
	               }
				   else if  ( u8TunerMode == (tU8)midw_fi_tcl_e8_Tun_PhysicalTuner::FI_EN_TUNER_1)
				   {
					   m_poDataPool -> u32dp_set( DPTUNER__TESTMODE_TA_TUNER_1,
	                                              &u8TABit ,
	                                              sizeof(u8TABit) );
					   m_poDataPool -> u32dp_set( DPTUNER__TESTMODE_TP_TUNER_1,
					  					  	      &u8TPBit ,
					  					  	      sizeof(u8TPBit) );
				       m_poDataPool -> u32dp_set( DPTUNER__TESTMODE_TMCCOUNT_TUNER_1,
					  					   	      &u8TMCbit ,
					  					   	      sizeof(u8TMCbit) );
				       m_poDataPool -> u32dp_set( DPTUNER__TESTMODE_DDA_TUNER_1,
					  					  	      &u8DDAbit ,
					  					  	      sizeof(u8DDAbit) );
						if(u8AudibleTunerbit)
						{
							u8AudibleTuner = 1;
						}
				   }
				   else if  ( u8TunerMode == (tU8)midw_fi_tcl_e8_Tun_PhysicalTuner::FI_EN_TUNER_2)
				   {
					   m_poDataPool -> u32dp_set( DPTUNER__TESTMODE_TA_TUNER_2,
	                                              &u8TABit ,
	                                              sizeof(u8TABit) );
					   m_poDataPool -> u32dp_set( DPTUNER__TESTMODE_TP_TUNER_2,
					  					  	      &u8TPBit ,
					  					  	      sizeof(u8TPBit) );
				       m_poDataPool -> u32dp_set( DPTUNER__TESTMODE_TMCCOUNT_TUNER_2,
					  					   	      &u8TMCbit ,
					  					   	      sizeof(u8TMCbit) );
				       m_poDataPool -> u32dp_set( DPTUNER__TESTMODE_DDA_TUNER_2,
					  					  	      &u8DDAbit ,
					  					  	      sizeof(u8DDAbit) );
					  if(u8AudibleTunerbit)
						{
							u8AudibleTuner = 2;
						}

				   }

				   else
				   {
					   m_poDataPool -> u32dp_set( DPTUNER__TESTMODE_TA_TUNER_3,
	                                              &u8TABit ,
	                                              sizeof(u8TABit) );
					   m_poDataPool -> u32dp_set( DPTUNER__TESTMODE_TP_TUNER_3,
					  					  	      &u8TPBit ,
					  					  	      sizeof(u8TPBit) );
				       m_poDataPool -> u32dp_set( DPTUNER__TESTMODE_TMCCOUNT_TUNER_3,
					  					   	      &u8TMCbit ,
					  					   	      sizeof(u8TMCbit) );
				       m_poDataPool -> u32dp_set( DPTUNER__TESTMODE_DDA_TUNER_3,
					  					  	      &u8DDAbit ,
					  					  	      sizeof(u8DDAbit) );
						if(u8AudibleTunerbit)
						{
							u8AudibleTuner = 3;
						}
				   }

				    if(u8AudibleTunerbit)
				    {
						m_poDataPool -> u32dp_set( DPTUNER__TESTMODE_ACTIVE_TUNER,
					   	                          &u8AudibleTuner ,
					   	                          sizeof(u8AudibleTuner) );
					}

                   ETG_TRACE_USR4(("TUNER %d TABit : %d" , u8TunerMode, u8TABit));
				   ETG_TRACE_USR4(("TUNER %d TPBit : %d" , u8TunerMode, u8TPBit));
				   ETG_TRACE_USR4(("TUNER %d TMCbit : %d" , u8TunerMode, u8TMCbit));
				   ETG_TRACE_USR4(("TUNER %d DDAbit : %d" , u8TunerMode, u8DDAbit));
				   ETG_TRACE_USR4(("TUNER %d AudibleTunerbit : %d AudibleTuner : %d" , u8TunerMode, u8AudibleTunerbit, u8AudibleTuner));
	               break;
	         }

	      case TM_DATA_FREQUENCY:
	         {
				   if ( u8TunerMode == (tU8)midw_fi_tcl_e8_Tun_PhysicalTuner::FI_EN_TUNER_0)
				   {
					   m_poDataPool -> u32dp_set( DPTUNER__TESTMODE_FREQVAL_TUNER_0,
	                                              &u32Data ,
	                                              sizeof(u32Data) );
				   }
				   else if  ( u8TunerMode == (tU8)midw_fi_tcl_e8_Tun_PhysicalTuner::FI_EN_TUNER_1)
				   {
					   m_poDataPool -> u32dp_set( DPTUNER__TESTMODE_FREQVAL_TUNER_1,
	                                              &u32Data ,
	                                              sizeof(u32Data) );
				   }
				   else if  ( u8TunerMode == (tU8)midw_fi_tcl_e8_Tun_PhysicalTuner::FI_EN_TUNER_2)
				   {
					   m_poDataPool -> u32dp_set( DPTUNER__TESTMODE_FREQVAL_TUNER_2,
	                                              &u32Data ,
	                                              sizeof(u32Data) );
				   }

				   else
				   {
					   m_poDataPool -> u32dp_set( DPTUNER__TESTMODE_FREQVAL_TUNER_3,
	                                              &u32Data ,
	                                              sizeof(u32Data) );
				   }

	            /*if ( m_poTrace != NULL )
	            {
	               m_poTrace->vTrace(  TR_LEVEL_HMI_INFO,
	                                   TR_CLASS_HSI_TUNER_MSG,
	                                   (tU16)TUN_TM_FREQ_WRITTEN,
	                                   ONE_ELEMENT,
	                                   &u32Data );
	            }*/

				   ETG_TRACE_USR4(("TUNER %d FREQUENCY : %d" , u8TunerMode, u32Data ));

	             break;
	         }

			 //added by rrv2kor for HUB Value

	      case TM_DATA_HUBVALUE:
	         {
	            tU8 u8Value = (tU8) u32Data ; // only 1 byte LSB contains the data
	            if ( u8TunerMode == (tU8)midw_fi_tcl_e8_Tun_PhysicalTuner::FI_EN_TUNER_0)
				   {
					   m_poDataPool -> u32dp_set( DPTUNER__TESTMODE_HUB_TUNER_0,
	                                              &u8Value ,
	                                              sizeof(u8Value) );
				   }
				   else if  ( u8TunerMode == (tU8)midw_fi_tcl_e8_Tun_PhysicalTuner::FI_EN_TUNER_1)
				   {
					   m_poDataPool -> u32dp_set( DPTUNER__TESTMODE_HUB_TUNER_1,
	                                              &u8Value ,
	                                              sizeof(u8Value) );
				   }
				   else if  ( u8TunerMode == (tU8)midw_fi_tcl_e8_Tun_PhysicalTuner::FI_EN_TUNER_2)
				   {
					   m_poDataPool -> u32dp_set( DPTUNER__TESTMODE_HUB_TUNER_2,
	                                              &u8Value ,
	                                              sizeof(u8Value) );
				   }

				   else
				   {
					   m_poDataPool -> u32dp_set( DPTUNER__TESTMODE_HUB_TUNER_3,
	                                              &u8Value ,
	                                              sizeof(u8Value) );
				   }
	           /* if ( m_poTrace != NULL )
				   {
	               m_poTrace->vTrace(  TR_LEVEL_HMI_INFO,
	                                   TR_CLASS_HSI_TUNER_MSG,
	                                   (tU16)TUN_TM_HUB_VALUE_WRITTEN,
	                                   ONE_ELEMENT,
	                                   &u8Value );
	            }*/

	            ETG_TRACE_USR4(("TUNER %d HUBVALUE : %d" , u8TunerMode, u8Value ));

	             break;
	         }



	       //added by rrv2kor for Mode using New interface

	      case TM_DATA_TESTMODE_SET_DDADDS:
			  {
				 tU8 u8ModeValue = (tU8) u32Data; // only 1 byte LSB contains the data

	#ifdef DPTUNER__TESTMODE_MODE
				 m_poDataPool -> u32dp_set( DPTUNER__MODE_SETUP,
					                        &u8ModeValue ,
											sizeof(u8ModeValue) );
	#endif

				 ETG_TRACE_USR4(("TUNER %d MODE : %d" , u8TunerMode, u8ModeValue ));



	             break;
	         }

			 /* case TM_DATA_TESTMODE_GET_MODE:
			  {
				  tU8 u8ModeValue = (tU8) u32Data; // only 1 byte LSB contains the data

				  #ifdef DPTUNER__TESTMODE_MODE
				  			 m_poDataPool -> u32dp_set( DPTUNER__TESTMODE_MODE,
				  				                        &u8ModeValue ,
				  										sizeof(u8ModeValue) );
				  #endif
				              if ( m_poTrace != NULL )
				              {
				                 m_poTrace->vTrace(  TR_LEVEL_HMI_INFO,
				                                     TR_CLASS_HSI_TUNER_MSG,
				                                     (tU16)TUN_TM_MODE_VALUE_WRITTEN,
				                                     ONE_ELEMENT,
				                                     &u8ModeValue );
				              }

	             break;
	        }*/


	      case TM_DATA_RADIO_ANTENNA_STATUS:
	      	           {
	      	              tU8 u8RFAntennaStatus = (tU8) u32Data; // Radio Antenna status


	      	              m_poDataPool -> u32dp_set( DPTUNER__TESTMODE_RADIO_ANTENNA_STATUS,
	      	                 &u8RFAntennaStatus ,
	      	                 sizeof(u8RFAntennaStatus) );

	      	              m_poDataPool -> u32dp_set( DPTUNER__TESTMODE_RADIO_ANTENNA_SELF_TEST,
	      	                 &u8RFAntennaStatus ,
	      	                 sizeof(u8RFAntennaStatus) );

	      	              /*if ( NULL  != m_poTrace)
	      	              {
	      	                 m_poTrace->vTrace( TR_LEVEL_HMI_INFO, TR_CLASS_HSI_AUDIO_MSG, "Value written from the tuner response is  %d", u8RFAntennaStatus);
	      	              }*/

	      	            ETG_TRACE_USR4(("Value written from the tuner response is  %d" , u8RFAntennaStatus ));

	      	              break;
	      	           }

			   case TM_DATA_RADIO_DAB_ANTENNA_STATUS:
				   {
					   tU8 u8DABAntennaStatus = (tU8) u32Data; // DAB Antenna status

					   //Used to display DAB antenna status in Running System Status
					  m_poDataPool -> u32dp_set( DPTUNER__TESTMODE_RADIO_DAB_ANTENNA_STATUS,
						 &u8DABAntennaStatus ,
						 sizeof(u8DABAntennaStatus) );

					  //Used to display DAB antenna status in Self Test
					  m_poDataPool -> u32dp_set( DPTUNER__TESTMODE_RADIO_DAB_ANTENNA_SELF_TEST,
	                 &u8DABAntennaStatus ,
	                 sizeof(u8DABAntennaStatus) );

					  /*if ( NULL  != m_poTrace)
						{
						 m_poTrace->vTrace( TR_LEVEL_HMI_INFO, TR_CLASS_HSI_AUDIO_MSG, "DAB Antenna status is  %d", u8DABAntennaStatus);
						}*/

					  ETG_TRACE_USR4(("DAB Antenna status : %d" , u8DABAntennaStatus ));

					   break;
				   }
	      default: // unkown case
	         break;

	      }// end of switch

	   }


	}
        
 
/*************************************************************************
*
* FUNCTION: clFi_Tuner::WriteStaticListElementsToDP() 
* 
* DESCRIPTION: this interface will be used by the FI handler
*               to write Static List Elements into Datapool,
               Specific to LSRNS has additional parameter bTpStatus
*
* PARAMETER: tU8 u8Element ,u8Flag  , tChar u32Data  
*           ( element to recognize DP-ID ,Flag to know the Type of Data and actual data)
*
* RETURNVALUE: NONE
*
*************************************************************************/
tVoid clFi_TUNER::WriteStaticListElementsToDP( tU8 u8Element, 
                                               tU8 u8Flag,   
                                               tChar* pcData,
                                               tU32 u32Freq,
                                               tBool bTpStatus) const
{

   /* check validity of datapool engine object pointer before procceding */
   if( NULL != m_poDataPool )
   { 
      tU32 u32Flag_DP_ID =0;

      tU32 u32PSName_DP_ID =0;

      tU32 u32Freq_DP_ID =0;

      tBool bPresetFlag = FALSE;

      tU32 u32TPStatus_DP_ID = 0;
    

      switch(u8Element )  // switch on the element 
      {
         
      case S_LIST_ELEM_FIRST:
         {
#ifdef DPTUNER__STATICLIST_ELEMENT1_PSNAME

            u32Flag_DP_ID = DPTUNER__STATICLIST_ELEMENT1_FLAG;
            u32PSName_DP_ID = DPTUNER__STATICLIST_ELEMENT1_PSNAME;
            u32Freq_DP_ID = DPTUNER__STATICLIST_ELEMENT1_FREQ;
#endif

#ifdef DPTUNER__STATICLIST_ELEMENT1_TPSTATUS
            u32TPStatus_DP_ID = DPTUNER__STATICLIST_ELEMENT1_TPSTATUS;
#endif
            break;
         }

      case S_LIST_ELEM_SECOND:
         {
#ifdef DPTUNER__STATICLIST_ELEMENT2_PSNAME

            u32Flag_DP_ID = DPTUNER__STATICLIST_ELEMENT2_FLAG;
            u32PSName_DP_ID = DPTUNER__STATICLIST_ELEMENT2_PSNAME;
            u32Freq_DP_ID = DPTUNER__STATICLIST_ELEMENT2_FREQ;
#endif

#ifdef DPTUNER__STATICLIST_ELEMENT2_TPSTATUS
            u32TPStatus_DP_ID = DPTUNER__STATICLIST_ELEMENT2_TPSTATUS;
#endif
            break;
         }

      case S_LIST_ELEM_THIRD:
         {
#ifdef DPTUNER__STATICLIST_ELEMENT3_PSNAME

            u32Flag_DP_ID = DPTUNER__STATICLIST_ELEMENT3_FLAG;
            u32PSName_DP_ID = DPTUNER__STATICLIST_ELEMENT3_PSNAME;
            u32Freq_DP_ID = DPTUNER__STATICLIST_ELEMENT3_FREQ;
#endif

#ifdef DPTUNER__STATICLIST_ELEMENT3_TPSTATUS
            u32TPStatus_DP_ID = DPTUNER__STATICLIST_ELEMENT3_TPSTATUS;
#endif
            break;
         }

      case S_LIST_ELEM_FOURTH:
         {
#ifdef DPTUNER__STATICLIST_ELEMENT4_PSNAME

            u32Flag_DP_ID = DPTUNER__STATICLIST_ELEMENT4_FLAG;
            u32PSName_DP_ID = DPTUNER__STATICLIST_ELEMENT4_PSNAME;
            u32Freq_DP_ID = DPTUNER__STATICLIST_ELEMENT4_FREQ;
#endif

#ifdef DPTUNER__STATICLIST_ELEMENT4_TPSTATUS
            u32TPStatus_DP_ID = DPTUNER__STATICLIST_ELEMENT4_TPSTATUS;
#endif
            break;
         }

      case S_LIST_ELEM_FIFTH:
         {
#ifdef DPTUNER__STATICLIST_ELEMENT5_PSNAME
            
            u32Flag_DP_ID = DPTUNER__STATICLIST_ELEMENT5_FLAG;
            u32PSName_DP_ID = DPTUNER__STATICLIST_ELEMENT5_PSNAME;
            u32Freq_DP_ID = DPTUNER__STATICLIST_ELEMENT5_FREQ;
#endif
      
#ifdef DPTUNER__STATICLIST_ELEMENT5_TPSTATUS
            u32TPStatus_DP_ID = DPTUNER__STATICLIST_ELEMENT5_TPSTATUS;
#endif            
            //Update a DP to tell all presets are updated
            bPresetFlag = TRUE; 
            
            break;
         }

      default:  // invalid case , no need to proceed further
         return;
      }

      m_poDataPool -> u32dp_set( u32Flag_DP_ID,
                                 &u8Flag ,         //write SList_Flag to DP 
                                 sizeof(tU8) );
          
      
      tU8 u8TpStatus = (tU8) bTpStatus; //For Trace
      
      m_poDataPool -> u32dp_set( u32TPStatus_DP_ID,
         &bTpStatus ,         
         sizeof(tBool) );
      
      if(m_poTrace  != NULL)
      {
         m_poTrace->vTrace(  TR_LEVEL_HMI_INFO,
            TR_CLASS_HSI_TUNER_MSG,
            (tU16)TUN_TP_STATUS_WRITTEN_IN_DP,
            ONE_ELEMENT,
            &u8TpStatus );        
         
         
      }
      
      
      if( u8Flag == (tU8)STATIC_LIST_PSNAME )
      {
         if( NULL != pcData )
         {
            m_poDataPool -> u32dp_set( u32PSName_DP_ID,
                                       pcData ,    //write PSName to DP 
                                       OSAL_u32StringLength( pcData) + 1
                                     ); // 8 byte char     
         }
         else
         {
             m_poDataPool -> u32dp_set( u32PSName_DP_ID,
                                        (const char *)"",    //write Blank to DP
                                        1
                                      ); // 8 byte char 

            if(m_poTrace  != NULL)
            {               
               m_poTrace->vTrace(  TR_LEVEL_HMI_INFO,
                                   TR_CLASS_HSI_TUNER_MSG,
                                  (tU16)TUN_SLIST_ELEM_PSNAME_NULL
                                );     
            }
               
         }

      }      
      
      
      m_poDataPool -> u32dp_set( u32Freq_DP_ID,
                                 &u32Freq ,         //write Freq to DP               
                                 sizeof(tU32)
                               );         
      
      
      if(m_poTrace  != NULL)
      {
         m_poTrace->vTrace(  TR_LEVEL_HMI_INFO,
                             TR_CLASS_HSI_TUNER_MSG,
                             (tU16)TUN_SLIST_ELEM_FREQ_WRITTEN,
                              ONE_ELEMENT,
                               &u32Freq
                           );
      }
      

      if( TRUE == bPresetFlag )
      {
         m_poDataPool -> u32dp_set( DPTUNER__STATICLIST_UPDATED,
                                    &bPresetFlag ,                       
                                    sizeof(tBool) );
         if(m_poTrace  != NULL)
         {
            m_poTrace->vTrace(  TR_LEVEL_HMI_INFO,
                                TR_CLASS_HSI_TUNER_MSG,
                                (tU16)TUN_SLIST_ELEM_FLAG_UPDATED );
         }

      }     


   }

   return;

}


/*************************************************************************
*
* FUNCTION: clFi_Tuner::WriteStaticListElementsToDP() 
* 
* DESCRIPTION: this interface will be used by the FI handler
*               to write Static List Elements into Datapool,
               Specific to HSRNS, don't have additional parameter bTpStatus
*
* PARAMETER: tU8 u8Element ,u8Flag  , tChar u32Data  
*           ( element to recognize DP-ID ,Flag to know the Type of Data and actual data)
*
* RETURNVALUE: NONE
*
*************************************************************************/
tVoid clFi_TUNER::WriteStaticListElementsToDP( tU8 u8Element, 
                                               tU8 u8Flag,   
                                               tChar* pcData,
                                               tU32 u32Freq) const
{

   /* check validity of datapool engine object pointer before procceding */
   if( NULL != m_poDataPool )
   { 
      tU32 u32Flag_DP_ID =0;

      tU32 u32PSName_DP_ID =0;

      tU32 u32Freq_DP_ID =0;

      tBool bPresetFlag = FALSE;
    

      switch(u8Element )  // switch on the element 
      {
         
      case S_LIST_ELEM_FIRST:
         {
#ifdef DPTUNER__STATICLIST_ELEMENT1_PSNAME

            u32Flag_DP_ID = DPTUNER__STATICLIST_ELEMENT1_FLAG;
            u32PSName_DP_ID = DPTUNER__STATICLIST_ELEMENT1_PSNAME;
            u32Freq_DP_ID = DPTUNER__STATICLIST_ELEMENT1_FREQ;
#endif
            break;
         }

      case S_LIST_ELEM_SECOND:
         {
#ifdef DPTUNER__STATICLIST_ELEMENT2_PSNAME

            u32Flag_DP_ID = DPTUNER__STATICLIST_ELEMENT2_FLAG;
            u32PSName_DP_ID = DPTUNER__STATICLIST_ELEMENT2_PSNAME;
            u32Freq_DP_ID = DPTUNER__STATICLIST_ELEMENT2_FREQ;
#endif
            break;
         }

      case S_LIST_ELEM_THIRD:
         {
#ifdef DPTUNER__STATICLIST_ELEMENT3_PSNAME

            u32Flag_DP_ID = DPTUNER__STATICLIST_ELEMENT3_FLAG;
            u32PSName_DP_ID = DPTUNER__STATICLIST_ELEMENT3_PSNAME;
            u32Freq_DP_ID = DPTUNER__STATICLIST_ELEMENT3_FREQ;
#endif
            break;
         }

      case S_LIST_ELEM_FOURTH:
         {
#ifdef DPTUNER__STATICLIST_ELEMENT4_PSNAME

            u32Flag_DP_ID = DPTUNER__STATICLIST_ELEMENT4_FLAG;
            u32PSName_DP_ID = DPTUNER__STATICLIST_ELEMENT4_PSNAME;
            u32Freq_DP_ID = DPTUNER__STATICLIST_ELEMENT4_FREQ;
#endif
            break;
         }

      case S_LIST_ELEM_FIFTH:
         {
#ifdef DPTUNER__STATICLIST_ELEMENT5_PSNAME
            
            u32Flag_DP_ID = DPTUNER__STATICLIST_ELEMENT5_FLAG;
            u32PSName_DP_ID = DPTUNER__STATICLIST_ELEMENT5_PSNAME;
            u32Freq_DP_ID = DPTUNER__STATICLIST_ELEMENT5_FREQ;
#endif
            
            //Update a DP to tell all presets are updated
            bPresetFlag = TRUE; 
            
            break;
         }

      default:  // invalid case , no need to proceed further
         return;
      }

      m_poDataPool -> u32dp_set( u32Flag_DP_ID,
                                 &u8Flag ,         //write SList_Flag to DP 
                                 sizeof(tU8) );
          
      
      
      if( u8Flag == (tU8)STATIC_LIST_PSNAME )
      {
         if( NULL != pcData )
         {
            m_poDataPool -> u32dp_set( u32PSName_DP_ID,
                                       pcData ,    //write PSName to DP 
                                       OSAL_u32StringLength( pcData) + 1
                                     ); // 8 byte char     
         }
         else
         {
             m_poDataPool -> u32dp_set( u32PSName_DP_ID,
                                        (const char *)"",    //write Blank to DP
                                        1
                                      ); // 8 byte char 

            if(m_poTrace  != NULL)
            {               
               m_poTrace->vTrace(  TR_LEVEL_HMI_INFO,
                                   TR_CLASS_HSI_TUNER_MSG,
                                  (tU16)TUN_SLIST_ELEM_PSNAME_NULL
                                );     
            }
               
         }

      }      
      
      
      m_poDataPool -> u32dp_set( u32Freq_DP_ID,
                                 &u32Freq ,         //write Freq to DP               
                                 sizeof(tU32)
                               );         
      
      
      if(m_poTrace  != NULL)
      {
         m_poTrace->vTrace(  TR_LEVEL_HMI_INFO,
                             TR_CLASS_HSI_TUNER_MSG,
                             (tU16)TUN_SLIST_ELEM_FREQ_WRITTEN,
                              ONE_ELEMENT,
                               &u32Freq
                           );
      }
      

      if( TRUE == bPresetFlag )
      {
         m_poDataPool -> u32dp_set( DPTUNER__STATICLIST_UPDATED,
                                    &bPresetFlag ,                       
                                    sizeof(tBool) );
         if(m_poTrace  != NULL)
         {
            m_poTrace->vTrace(  TR_LEVEL_HMI_INFO,
                                TR_CLASS_HSI_TUNER_MSG,
                                (tU16)TUN_SLIST_ELEM_FLAG_UPDATED );
         }

      }     


   }

   return;
}

/*************************************************************************
*
* FUNCTION: clFi_Tuner::vFMSeekTestReadWrite() 
* 
* DESCRIPTION: this interface will be used by the handler
*               to read/write data from/to Datapool needed for FM interactive seek test
*
* PARAMETER: tU8 u8ReadOrWrite
*
* RETURNVALUE: NONE
*
*************************************************************************/
 
tVoid clFi_TUNER::vFMSeekTestReadWrite( tU8 u8ReadOrWrite )
{
   if(m_poDataPool != NULL)
   {
      if( FMSEEK_TEST_READ_VALUES == u8ReadOrWrite)
      {
         bCheck = TRUE;
         
         {
            tU32 u32Size   = m_poDataPool->u32dp_getElementSize(DPTUNER__STATION_NAME);
            m_poDataPool -> u32dp_get( DPTUNER__STATION_NAME, 
               pcStationName , u32Size );
         }
         
         {
            m_poDataPool -> u32dp_get( DPTUNER__STATION_BAND,
               &u8StationBand , sizeof(u8StationBand) );
            
            u8Trace[0] = u8StationBand;
         }
         
         {
            m_poDataPool -> u32dp_get( DPTUNER__STATION_BANDLEVEL,
               &u8StationBandLevel , sizeof(u8StationBandLevel) );
            
            u8Trace[1] = u8StationBandLevel ;
         }
         
         {
            m_poDataPool -> u32dp_get( DPTUNER__STATICLIST_ELEMENT_NUM,
               &u8StaticListElementNum , sizeof(u8StaticListElementNum) );
            
            u8Trace[2] = u8StaticListElementNum ;
         }
         
         {
            m_poDataPool -> u32dp_get( DPTUNER__STATION_FM_FREQVAL,
               &u32StationFreVal , sizeof(u32StationFreVal) );
            
            u32Trace[0] = u32StationFreVal ;
         }
         
         {
            m_poDataPool -> u32dp_get( DPTUNER__STATION_FLAG,
               &bStationFlag , sizeof(bStationFlag) );
            
            u32Trace[1] = bStationFlag;
         }

#ifdef DPTUNER__FEATURE_FMSEEK
         {
            tBool bIndex = FALSE;
            m_poDataPool -> u32dp_set( DPTUNER__FEATURE_FMSEEK,
               &bIndex,
               sizeof(tBool) );
         }
#endif
         
      }
      else if(FMSEEK_TEST_WRITE_VALUES == u8ReadOrWrite && TRUE == bCheck)
      {

#ifdef DPTUNER__FEATURE_FMSEEK
         {
            tBool bIndex = TRUE;
            m_poDataPool -> u32dp_set( DPTUNER__FEATURE_FMSEEK,
               &bIndex,
               sizeof(tBool) );
         }
#endif
         
         {
            tU32 u32Size   = m_poDataPool->u32dp_getElementSize(DPTUNER__STATION_NAME);
            m_poDataPool -> u32dp_set( DPTUNER__STATION_NAME, 
               pcStationName , u32Size );
         }
         
         {
            //m_poDataPool -> u32dp_set( DPTUNER__STATION_BAND,
            //   &u8StationBand , sizeof(u8StationBand) );
         }
         
         {
            m_poDataPool -> u32dp_set( DPTUNER__STATION_FM_FREQVAL,
               &u32StationFreVal , sizeof(u32StationFreVal) );
         }
         
         {
            m_poDataPool -> u32dp_set( DPTUNER__STATION_BANDLEVEL,
               &u8StationBandLevel , sizeof(u8StationBandLevel) );
         }
         
         {
            m_poDataPool -> u32dp_set( DPTUNER__STATION_FLAG,
               &bStationFlag , sizeof(bStationFlag) );
         }
         
         {
            //m_poDataPool -> u32dp_set( DPTUNER__STATICLIST_ELEMENT_NUM,
            //   &u8StaticListElementNum , sizeof(u8StaticListElementNum) );
         }

         bCheck = FALSE;
      }
      
      ETG_TRACE_USR4(("vFMSeekTestReadWrite, Station Name : %8s ",pcStationName));

      if(m_poTrace  != NULL)
      {

         m_poTrace->vTrace(  TR_LEVEL_HMI_INFO,
            TR_CLASS_HSI_TUNER_MSG,
            (tU16)TUN_FM_SEEK_TEST_ONE,
            THREE_ELEMENT,
            u8Trace);

         m_poTrace->vTrace(  TR_LEVEL_HMI_INFO,
            TR_CLASS_HSI_TUNER_MSG,
            (tU16)TUN_FM_SEEK_TEST_TWO,
            TWO_ELEMENT,
            u32Trace);
      }
      
   }

}

/*************************************************************************
*
* FUNCTION: clFi_Tuner::bCheckPiStepping() 
* 
* DESCRIPTION: this interface will be used by the handler
*              to read datapool for PI Stepping and return corresponding value
*
* PARAMETER:  --
*
* RETURNVALUE: tBool
*
*************************************************************************/
tBool clFi_TUNER::bCheckPiStepping() const
{
   tBool bPiOnOff = FALSE;

   if(m_poDataPool != NULL)
   {
      m_poDataPool -> u32dp_get( DPTUNER__FEATURE_PISTEP,
                                 &bPiOnOff , sizeof(bPiOnOff) );
   }

   return( bPiOnOff );

}

/*************************************************************************
*
* FUNCTION: tVoid clFi_TUNER::vStartInitSequence() 
* 
* DESCRIPTION: to start the initialization-sequence from HSI-handler.
*
* PARAMETER: u16FunctionID
*
* RETURNVALUE: none
*
*************************************************************************/

tVoid clFi_TUNER::vStartInitSequence(tU16 u16ServiceID)
{
    switch (u16ServiceID)
    {
        case CCA_C_U16_SRV_TUNERCONTROL:
        {
            if (FALSE == bRegFlag)
            {
                // Tuner Init Event.
//                m_poEventEngine->bEvent2Engine(TUNSMT__INIT__TUN);
                
                // Tuner Init Ok Event.
//                m_poEventEngine->bEvent2Engine(TUNSMT__INIT__TUN__OK);
                bRegFlag = TRUE;
            }
        }
        break;
        case CCA_C_U16_SRV_TUNERDIAG:
        {
        }
        break;
        default:

        {
            ;
        }
    }
}


/*************************************************************************
*
* FUNCTION: tVoid clFi_TUNER::bFormatFrequenz() 
* 
* DESCRIPTION: format the frequency fro HMI
*
* PARAMETER: u32Freq
*
* RETURNVALUE: none
*
*************************************************************************/
tBool clFi_TUNER::bFormatFrequenz( tU32 u32Freq, tBool bUpdateSL /* = false */ )
{
   tBool bReturn = FALSE;
   /** Variables to hold the frequency values */
   tU8 u8Major = 0;
   tU8 u8Minor = 0;
   /** this variable will hold current active band */
   // CURRENTLY NOT USED sda2hi 20070425 ARM WARNING // tU8 u8CurrentBand = 0;
   /** These variables hold the formatted frequency or Station name */
   tUTF8 utf8Data[MAX_NO_OF_CHAR_FM] = {'\0'};


   tU8 u8TunArea = 0;

     tU32 u32FMMinFreqValue = 0;
   tU32 u32AMMaxFreqValue = 0;


   if (m_poDataPool != NULL)
   {
	    m_poDataPool -> u32dp_get(DPSYSTEM__VARIANT_CONFIG_RADIO_AREA,
		    &u8TunArea, 
		    sizeof(u8TunArea) );

		m_poDataPool -> u32dp_get(DPSYSTEM__VARIANT_CONFIG_RADIO_FM_START_FRQ ,&u32FMMinFreqValue,
	       sizeof(u32FMMinFreqValue) );

		m_poDataPool -> u32dp_get(DPSYSTEM__VARIANT_CONFIG_RADIO_AM_END_FRQ ,&u32AMMaxFreqValue,
	       sizeof(u32AMMaxFreqValue) );

   }
    // if the frequency is non zero value and in the FM band range, then only formate the frequency as string.
    // if the frequency is zero don't formate as string.
	if((u32Freq >= u32FMMinFreqValue) &&( u32Freq != 0) ) // FM
   {
      // As we get frequency in KHz we have to convert it into MHz for FM Band          
      u8Major = static_cast<tU8>(u32Freq / 1000);
      u8Minor = static_cast<tU8>((u32Freq % 1000)/100);
      
	  if(u8TunArea == TUN_AREA_NAR)//If NAR -unit is FM else MHz
	  {
		OSALUTIL_s32SaveNPrintFormat( utf8Data,MAX_NO_OF_CHAR_FM, "%i.%i FM", u8Major, u8Minor);
	  }
	  else
	  {
		OSALUTIL_s32SaveNPrintFormat( utf8Data,MAX_NO_OF_CHAR_FM, "%i.%i MHz", u8Major, u8Minor);
	  }
      
	  if (bUpdateSL)
	  {
         pHSI_GetRadio()->bSetFMStationList_NewPS(utf8Data);
	  }
      bReturn = TRUE;
   }
// if the frequency is non zero value and in the AM band range, then only formate the frequency as string.
//if the frequency is zero don't formate as string.
    else if((u32Freq <= u32AMMaxFreqValue) && (u32Freq  != 0))//AM
   {
      // Display the formatted frequency 
      // Storeing the value of FREQUENCY into the the String utf8Data
	  if(u8TunArea == TUN_AREA_NAR)//If NAR -unit is FM else MHz
	  {
		OSALUTIL_s32SaveNPrintFormat( utf8Data,MAX_NO_OF_CHAR_FM, "%d AM", u32Freq );
	  }
	  else
	  {
		OSALUTIL_s32SaveNPrintFormat( utf8Data,MAX_NO_OF_CHAR_FM, "%d kHz", u32Freq );
	  }
      
	  if (bUpdateSL)
	  {
         pHSI_GetRadio()->bSetFMStationList_NewPS(utf8Data);
	  }
      bReturn = TRUE;
   }

   if(bReturn == TRUE)
   {
     WriteStringDataToDp(STATION_DATA_STATIONNAME, (tChar*)utf8Data);
   }

   return bReturn;
}/** end of bFormatFrequenz function */

/*************************************************************************
*
* FUNCTION:    pHSI_GetRadio 
* 
* DESCRIPTION: Method to get a Pointer to ComponentManager Radio-Manager
*
* PARAMETER: 
*
* RETURNVALUE: clHSI_Radio*
*
*************************************************************************/

clHSI_Radio* clFi_TUNER::pHSI_GetRadio()
{
   if (m_pHSI_Radio  == NULL)
   {
      clHSI_CMMngr*  pclHSI_CMManager = clHSI_CMMngr::pclGetInstance();

      if(pclHSI_CMManager != NULL)
      {
      m_pHSI_Radio   =  dynamic_cast <clHSI_Radio*>(pclHSI_CMManager->pHSI_BaseGet(EN_HSI_RADIO));
      }
   }

   return m_pHSI_Radio;
}

/*************************************************************************
*
* FUNCTION:    vNewAppState(tenFwAppStates enAppStates) (HMI)
* 
* DESCRIPTION: This function gets the new app state and if system is running
*              down, it sets a flag
*              This is used to avoid resetting SD values when running down
*
* PARAMETER:   enAppStates - new system state
* RETURNVALUE: void
*
*************************************************************************/
tVoid clFi_TUNER::vNewAppState(tenFwAppStates enAppStates)
{
   if (m_poTrace  != NULL)
   {
      m_poTrace->vTrace(TR_LEVEL_HMI_INFO, TR_CLASS_HSI_TUNER_MSG,(tU16)FI_RADIO_NEW_APP_STATE,1,(tU16*)&enAppStates);
   }


   // Check if system is running down
   if (enAppStates != FW_APP_STATE_NORMAL)
   {
      //m_bIsSystemRunningDown = FALSE;
	  clHSI_Radio    *pHSI  =  pHSI_GetRadio();
      if (pHSI  != NULL)
      {
         if (m_poTrace  != NULL)
         {
            m_poTrace->vTrace(TR_LEVEL_HMI_INFO, TR_CLASS_HSI_TUNER_MSG,(tU16)FI_RADIO_NEW_APP_STATE_UNFIX,1,(tU16*)&enAppStates);
         }
         sMsg Msg (NULL, (tU32)CMMSG_RADIO_UNFIX_ACTIVE_STATION_NAME, 0, 0);
         pHSI->bExecuteMessage(Msg);
      }
   }
}
#ifdef VARIANT_S_FTR_ENABLE_HMI_TUNER_ITG5
/******************************************************************************
* FUNCTION:    clFi_TUNER::vTun_WriteDataToDataPool()
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
tVoid clFi_TUNER::vTun_WriteDataToDataPool(tU32 u32DataPoolIndex,
										 tU8 u8DataPoolListColumnIndex,
										 tU8 u8DataPoolListRowIndex,
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
					u8DataPoolListRowIndex,
					u8DataPoolListColumnIndex,
					pvDataWrite,
					u32ByteCount );
			}
		}
	}
}

/******************************************************************************
* FUNCTION:    clFi_TUNER::vTun_WriteDataToDataPool()
*------------------------------------------------------------------------------
* DESCRIPTION: Function writes list string data into given data pool index
*------------------------------------------------------------------------------
* PARAMETER:   u32DataPoolIndex : Index of the data pool value into which data must be written.
*              u32DataPoolListColumnIndex : Column index where the data has to written
*              u8DataPoolListRowIndex : Row index where the data has to written
*              pMWstr : midw_ext_fi_tclString string which has to be written
* RETURNVALUE: tVoid
*******************************************************************************/
tVoid clFi_TUNER::vTun_WriteDataToDataPool( tU32 u32DataPoolIndex,
										  tU8 u8DataPoolListColumnIndex,
										  tU8 u8DataPoolListRowIndex,
										  midw_fi_tclString* pMWstr)
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
			}
		}
	}
}
#endif
