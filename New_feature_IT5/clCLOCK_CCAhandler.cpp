/************************************************************************
* FILE:         clCLOCK_CCAhandler.cpp
* PROJECT:      Ford
* SW-COMPONENT:
*------------------------------------------------------------------------
*
* DESCRIPTION :
*
*------------------------------------------------------------------------
* COPYRIGHT:    (c) 2004 Robert Bosch GmbH, Hildesheim
* HISTORY:      
* Date      | Author                        | Modification
* 24.05.06  | RBIN-EDI1 K Pavan Srinath     | initial version
* 13.09.06  | RBIN-EDI1 Naveen Kumar N      | Additional interface added
* 29.08.07  | RBIN-EDI1 Rama Rayalu | removed old functions and 
                                      added new functions according 
                                      to the new interfaces
*************************************************************************/

/* includes */

/* Include the CLOCK interface file */

/* HSI CLOCK Handler Interface */

// precompiled header, should always be the first include
#include "precompiled.hh"

#include "fiClock/clCLOCK_CCAhandler.h"

/* HSI CLOCK FI Interface */
#include "fiClock/clFI_CLOCK.h"

class hmicca_tclApp;

/* 
MESSAGE MAP:
Such a map and an own handler is needed for every service you are accessing!
Enter the function IDs (FID) and the corresponding functions here.
The function will be called when a message with the corresponding FID arrives
*/

BEGIN_MSG_MAP(clCLOCK_CCAhandler, ahl_tclBaseWork)

/************************ Properties *************************************/
          /**  Property Date Time **/
          ON_MESSAGE( MIDW_EXT_CLOCKFI_C_U16_LOCALTIMEDATE,
                      vHandleFID_DATEANDTIME_From_VDClock
                    )
          /**  Property Time Format **/
          ON_MESSAGE( MIDW_EXT_CLOCKFI_C_U16_TIMEFORMAT,
                      vHandleFID_TIMEFORMAT_From_VDClock
                    )
          /**  Property Time Format **/
          ON_MESSAGE( MIDW_EXT_CLOCKFI_C_U16_DATEFORMAT,
                      vHandleFID_DATEFORMAT_From_VDClock
                    )
          /**  Property Time Format **/
          ON_MESSAGE( MIDW_EXT_CLOCKFI_C_U16_CLOCKSOURCESTATUS,
                      vHandleFID_CLOCKSOURCE_STATUS_From_VDClock
                    )
          /**  Property Time Format **/
          ON_MESSAGE( MIDW_EXT_CLOCKFI_C_U16_FORMATSOURCESTATUS,
                      vHandleFID_FORMATSOURCE_STATUS_From_VDClock
                    )

         /**  Property TIMEZONE FID = 0x8410**/
         ON_MESSAGE( MIDW_EXT_CLOCKFI_C_U16_TIMEZONE,
                    vHandleFID_TIMEZONE_From_VDClock
                    )

         /**  Property DAYLIGHTSAVINGTIME FID = 0x8420**/
         ON_MESSAGE( MIDW_EXT_CLOCKFI_C_U16_DAYLIGHTSAVINGTIME,
                      vHandleFID_DAYLIGHTSAVINGTIME_From_VDClock
                    )

         /**  Property GPS_AUTOSYNC FID = 0x8430**/
         ON_MESSAGE( MIDW_EXT_CLOCKFI_C_U16_GPS_AUTOSYNC,
                    vHandleFID_GPSTIMESTATUS_From_VDClock
                    )
/******************* COMMON Methods result Handler ***********************/

          /**  Method Scalar Value conversion into structure  **/
          ON_MESSAGE( 
                      MIDW_EXT_CLOCKFI_C_U16_GETTIMEDATEFROMVALUE,
                      vHandleALL_FID_MethodResult_From_VDClock
                    )
         /**  Method Result for Getting Available Time Zone  **/
          ON_MESSAGE( 
                      MIDW_EXT_CLOCKFI_C_U16_GETAVAILABLETIMEZONES,
                      vHandleALL_FID_MethodResult_From_VDClock
                    )
        ON_MESSAGE(
                    MIDW_EXT_CLOCKFI_C_U16_GPS_WEEKEPOCH,
                    vHandleFID_GPS_WeekEpoch_From_VDClock
                    )
        /**Property update from LocalTimeOffset  **/
        ON_MESSAGE(
                    MIDW_EXT_CLOCKFI_C_U16_LOCALTIMEOFFSET,
                    vHandleFID_LOCALTIMEOFFSET_From_VDClock
                    )
END_MSG_MAP()

/*************************************************************************
* FUNCTION   : ~clCLOCK_CCAhandler
*
* DESCRIPTION: Destructor
*
* PARAMETER  : NONE
*
* RETURNVALUE: NONE
*
*************************************************************************/
clCLOCK_CCAhandler::~clCLOCK_CCAhandler()
{   
  /* FI pointer to NULL */
  poFI = NULL;

}

/*************************************************************************
* FUNCTION   : clCLOCK_CCAhandler    
*
* DESCRIPTION: Constructor
*
* PARAMETER  : NONE
*
* RETURNVALUE: None
*
*************************************************************************/
clCLOCK_CCAhandler::clCLOCK_CCAhandler(clFi_CLOCK * poFiPtr ) 
{  
   m_u16TZListLength = 0;
   poFI = poFiPtr;

   oFiRegHelper.bAddFunctionID( MIDW_EXT_CLOCKFI_C_U16_LOCALTIMEDATE );
   oFiRegHelper.bAddFunctionID( MIDW_EXT_CLOCKFI_C_U16_TIMEFORMAT );
   oFiRegHelper.bAddFunctionID( MIDW_EXT_CLOCKFI_C_U16_DATEFORMAT );
   oFiRegHelper.bAddFunctionID( MIDW_EXT_CLOCKFI_C_U16_CLOCKSOURCESTATUS );
   oFiRegHelper.bAddFunctionID( MIDW_EXT_CLOCKFI_C_U16_FORMATSOURCESTATUS );
   oFiRegHelper.bAddFunctionID( MIDW_EXT_CLOCKFI_C_U16_TIMEZONE );
   oFiRegHelper.bAddFunctionID( MIDW_EXT_CLOCKFI_C_U16_DAYLIGHTSAVINGTIME );
   oFiRegHelper.bAddFunctionID( MIDW_EXT_CLOCKFI_C_U16_GPS_AUTOSYNC );
   oFiRegHelper.bAddFunctionID( MIDW_EXT_CLOCKFI_C_U16_GPS_WEEKEPOCH );
   oFiRegHelper.bAddFunctionID( MIDW_EXT_CLOCKFI_C_U16_LOCALTIMEOFFSET);
}

/*************************************************************************
*
* FUNCTION: vGenericTraceDup 
* 
* DESCRIPTION: This function will provide Generic Interface 
*              to Pass the Info to Trace
*
* PARAMETER:  u8TraceLevel - Type of Trace 
*             u16SubType - Signifies the SubType of the Trace 
*             u16DataOne - The data to be passed
*             u16DataTwo - The data to be passed
*
* RETURNVALUE: void
*
*************************************************************************/
/* function same as that of vGenericTrace in FI 
   added to prevent FI pointer checking during every trace print */
tVoid clCLOCK_CCAhandler::vGenericTraceDup ( tU8  u8TraceLevel , 
                                             tU16 u16SubType , 
                                             tU16 u16DataOne , 
                                             tU16 u16DataTwo )const
{
   tU16 u16Temp[2];
   u16Temp[0] = u16DataOne;
   u16Temp[1] = u16DataTwo;

   /* check for Trace Pointer */
   if ( m_poTrace != NULL )
   {
      m_poTrace->vTrace( (hmi_tenTraceLevel)u8TraceLevel , 
         TR_CLASS_HSI_CLOCK_MSG , u16SubType , sizeof( u16Temp ) , u16Temp );
   }

}


/*************************************************************************
* FUNCTION   : vHandleALL_FID_MethodResult_From_VDClock 
*
* DESCRIPTION: common handler for Method Results   
*
* PARAMETER  : amt_tclServiceData*
*
* RETURNVALUE: NONE
*
* History    : InitialVersion
* 
* 
*************************************************************************/
tVoid clCLOCK_CCAhandler::vHandleALL_FID_MethodResult_From_VDClock
                                          ( amt_tclServiceData* poMessage ) 
{

   if ( FALSE == bIncomingMsgCheck( poMessage ) )
   {
      /* ERROR trace */
      vGenericTraceDup ( (tU8)TR_LEVEL_HMI_ERROR , CLOCK_TRACE_CCA , 
         HSI_CLOCK_MSG_REC_ERROR , HSI_CLOCK_ZERO );
      
      return;
   }
  
   /*  check the validity of the FI pointer  */
   if ( NULL == poFI )
   {
      /* ERROR trace */
      vGenericTraceDup ( (tU8)TR_LEVEL_HMI_ERROR , CLOCK_TRACE_ASSERT , 
         HSI_CLOCK_NULLPTR_FI , HSI_CLOCK_METHOD_RESULT );
      
      return; /* Function is not executed further */
   }
  
  /* required for TRACE obtain the FID of the message */
  tU16 u16FID = poMessage->u16GetFunctionID();
  
  /* check the validity of the message */
  if( TRUE == poMessage->bIsValid() )
  {
    /* Switch based on the opcode of the result message error check */
    switch ( poMessage->u8GetOpCode() )
    {

    case AMT_C_U8_CCAMSG_OPCODE_METHODRESULT: 
       {
          
          /* value used for trace */
          tU16 u16Result = 0 ;
          
          /* switch based on the function ID of the Method Result */
          switch ( u16FID )
          {
             
             /* FKT ID = 0x8050 */
          case MIDW_EXT_CLOCKFI_C_U16_GETTIMEDATEFROMVALUE :
              {
              vStartInitSequence(MIDW_EXT_CLOCKFI_C_U16_GETTIMEDATEFROMVALUE);

              midw_ext_clockfi_tclMsgGetTimeDateFromValueMethodResult myDateTime;
              vGetDataFromAmt(poMessage,myDateTime);

              tS16 s16YearIn            =    myDateTime.s16Year;
              tU8 u8MonthIn            =        myDateTime.u8Month;
              tU8 u8DayIn                =    myDateTime.u8Day;
              tU8 u8HourIn                =    myDateTime.u8Hours;
              tU8 u8MinuteIn            =    myDateTime.u8Minutes;
              tU8 u8SecondsIn            =    myDateTime.u8Seconds;
                  
                  
                  if (m_poTrace != NULL )
                  {
                      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
                          TR_CLASS_HSI_CLOCK_MSG,"Value of Hours is : %d",u8HourIn );
                  }
                  if (m_poTrace != NULL )
                  {
                      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
                          TR_CLASS_HSI_CLOCK_MSG,"Value of Minutes is : %d",u8MinuteIn );
                  }
                  if (m_poTrace != NULL )
                  {
                      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
                          TR_CLASS_HSI_CLOCK_MSG,"Value of Seconds is : %d",u8SecondsIn );
                  }
                  if (m_poTrace != NULL )
                  {
                      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
                          TR_CLASS_HSI_CLOCK_MSG,"Value of Year is : %d",s16YearIn );
                  }
                  if (m_poTrace != NULL )
                  {
                      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
                          TR_CLASS_HSI_CLOCK_MSG,"Value of Month is : %d",u8MonthIn );
                  }
                  if (m_poTrace != NULL )
                  {
                      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
                          TR_CLASS_HSI_CLOCK_MSG,"Value of Day is : %d",u8DayIn );
                  }
                
                  // command counter is used to identify the type of time to be send
                  tU16 u16KindOfTime = poMessage->u16GetCmdCounter(); 
                  if (m_poTrace != NULL )
                  {
                      switch ( u16KindOfTime )
                      {

                      case ETA_CONVERSION:
                          m_poTrace->vTrace( TR_LEVEL_HMI_INFO, TR_CLASS_HSI_CLOCK_MSG," Method result for ETA_CONVERSION " ); break;
                      case NAVI_TMCTIME_CONVERSION:
                          m_poTrace->vTrace( TR_LEVEL_HMI_INFO, TR_CLASS_HSI_CLOCK_MSG," Method result for NAVI_TMCTIME_CONVERSION " ); break;
                      case TESTMODE_TMC_LASTMSG_DATETIME_CONVERSION:
                          m_poTrace->vTrace( TR_LEVEL_HMI_INFO, TR_CLASS_HSI_CLOCK_MSG," Method result for TESTMODE_TMC_LASTMSG_DATETIME_CONVERSION " ); break;
                      case DAY_BEFORE_CURR_TIME_FROMSCALAR:
                          m_poTrace->vTrace( TR_LEVEL_HMI_INFO, TR_CLASS_HSI_CLOCK_MSG," Method result for DAY_BEFORE_CURR_TIME_FROMSCALAR " ); break;
                      case SXMSUSPENDDATE_UTC_TO_LOCAL:
                          m_poTrace->vTrace( TR_LEVEL_HMI_INFO, TR_CLASS_HSI_CLOCK_MSG," Method result for SXMSUSPENDDATE_UTC_TO_LOCAL " ); break;
                      default:break;
                      }
                  }

                  if( NULL != poFI->m_poDataPool )
                  {
                      /* write all the extracted values to DataPool if the flag is true */
                      if( ETA_CONVERSION == u16KindOfTime ) //ETA
                      {
                          if (m_poTrace != NULL )
                          {
                              m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
                                  TR_CLASS_HSI_CLOCK_MSG,"Writing Data to Arrival Time Datapools" );
                          }

                          poFI->m_poDataPool -> u32dp_set( DPCLOCK__ARRIVALDATETIME_HOURS, 
                              &u8HourIn, 
                              sizeof(u8HourIn)    );
                          poFI->m_poDataPool -> u32dp_set( DPCLOCK__ARRIVALDATETIME_MINS, 
                              &u8MinuteIn, 
                              sizeof(u8MinuteIn)    );
                          poFI->m_poDataPool -> u32dp_set( DPCLOCK__ARRIVALDATETIME_SECONDS, 
                              &u8SecondsIn, 
                              sizeof(u8SecondsIn)    );
                          poFI->m_poDataPool -> u32dp_set( DPCLOCK__ARRIVALDATETIME_MONTH, 
                              &u8MonthIn, 
                              sizeof(u8MonthIn)    );
                          poFI->m_poDataPool -> u32dp_set( DPCLOCK__ARRIVALDATETIME_DAY, 
                              &u8DayIn, 
                              sizeof(u8DayIn)    );
                          poFI->m_poDataPool -> u32dp_set( DPCLOCK__ARRIVALDATETIME_YEAR, 
                              &s16YearIn, 
                              sizeof(s16YearIn)    );
                      }
                      else if(NAVI_TMCTIME_CONVERSION == u16KindOfTime)
                      {
                          if (m_poTrace != NULL )
                          {
                              m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
                                  TR_CLASS_HSI_CLOCK_MSG,"Writing Data to TMC Datapools" );
                          }
                          poFI->m_poDataPool -> u32dp_set( DPCLOCK__CONVRTTIME_FORTMC_HOURS, 
                              &u8HourIn, 
                              sizeof(u8HourIn)    );
                          poFI->m_poDataPool -> u32dp_set( DPCLOCK__CONVRTTIME_FORTMC_MINS, 
                              &u8MinuteIn, 
                              sizeof(u8MinuteIn)    );
                          poFI->m_poDataPool -> u32dp_set( DPCLOCK__CONVRTTIME_FORTMC_SECONDS, 
                              &u8SecondsIn, 
                              sizeof(u8SecondsIn)    );
                          poFI->m_poDataPool -> u32dp_set( DPCLOCK__CONVRTTIME_FORTMC_MONTH, 
                              &u8MonthIn, 
                              sizeof(u8MonthIn)    );
                          poFI->m_poDataPool -> u32dp_set( DPCLOCK__CONVRTTIME_FORTMC_DAY, 
                              &u8DayIn, 
                              sizeof(u8DayIn)    );
                          poFI->m_poDataPool -> u32dp_set( DPCLOCK__CONVRTTIME_FORTMC_YEAR, 
                              &s16YearIn, 
                              sizeof(s16YearIn)    );
                      }
                      else if (TESTMODE_TMC_LASTMSG_DATETIME_CONVERSION == u16KindOfTime)
                      {
                          if (m_poTrace != NULL )
                          {
                              m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
                                  TR_CLASS_HSI_CLOCK_MSG,"Writing Data to TestMode TMC Datapools" );
                          }
                          poFI->m_poDataPool -> u32dp_set( DPTUNER__TMC_LAST_MSG_TIME_HOUR, 
                              &u8HourIn, 
                              sizeof(u8HourIn)    );
                          poFI->m_poDataPool -> u32dp_set( DPTUNER__TMC_LAST_MSG_TIME_MIN, 
                              &u8MinuteIn, 
                              sizeof(u8MinuteIn)    );
                          poFI->m_poDataPool -> u32dp_set( DPTUNER__TMC_LAST_MSG_DATE_MONTH, 
                              &u8MonthIn, 
                              sizeof(u8MonthIn)    );
                          poFI->m_poDataPool -> u32dp_set( DPTUNER__TMC_LAST_MSG_DATE_DAY, 
                              &u8DayIn, 
                              sizeof(u8DayIn)    );
                          poFI->m_poDataPool -> u32dp_set( DPTUNER__TMC_LAST_MSG_DATE_YEAR, 
                              &s16YearIn, 
                              sizeof(s16YearIn)    );
                       	if (NULL != clHSI_CMMngr::pclGetInstance())
						{
							clHSI_CMNAVI* pclHSI_CMNAVI = dynamic_cast<clHSI_CMNAVI*>
							((clHSI_CMMngr::pclGetInstance())->pHSI_BaseGet(EN_HSI_NAVI));
							if (NULL != pclHSI_CMNAVI)
							{
								sMsg msg(NULL, CMMSG__HSI__NAV__EPOC_TIME_CONVERTED, 0, 0 );
								pclHSI_CMNAVI->bExecuteMessage(msg);
							}
						}

                      }
                      else if (DAY_BEFORE_CURR_TIME_FROMSCALAR == u16KindOfTime)
                      {
                          if (m_poTrace != NULL )
                          {
                              m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
                                  TR_CLASS_HSI_CLOCK_MSG,"Writing time data to Day Before Time Datapools" );
                          }
                          poFI->m_poDataPool -> u32dp_set( DPCLOCK__YESTERDAY_DATETIME_HOUR, 
                              &u8HourIn, 
                              sizeof(u8HourIn)    );
                          poFI->m_poDataPool -> u32dp_set( DPCLOCK__YESTERDAY_DATETIME_MINUTES, 
                              &u8MinuteIn, 
                              sizeof(u8MinuteIn)    );
                          poFI->m_poDataPool -> u32dp_set( DPCLOCK__YESTERDAY_DATETIME_MONTH, 
                              &u8MonthIn, 
                              sizeof(u8MonthIn)    );
                          poFI->m_poDataPool -> u32dp_set( DPCLOCK__YESTERDAY_DATETIME_DAY, 
                              &u8DayIn, 
                              sizeof(u8DayIn)    );
                          poFI->m_poDataPool -> u32dp_set( DPCLOCK__YESTERDAY_DATETIME_YEAR, 
                              &s16YearIn, 
                              sizeof(s16YearIn)    );
                      }
                      else if (SXMSUSPENDDATE_UTC_TO_LOCAL == u16KindOfTime)
                      {
                          if (m_poTrace != NULL )
                          {
                              m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
                                  TR_CLASS_HSI_CLOCK_MSG,"Writing time to Datapools" );
                          }
                          poFI->m_poDataPool -> u32dp_set( DPDP_AUDIO__DIAGINFO_SUSPENDEDDATE_HOURS, 
                              &u8HourIn, 
                              sizeof(u8HourIn)    );
                          poFI->m_poDataPool -> u32dp_set( DPDP_AUDIO__DIAGINFO_SUSPENDEDDATE_MINS, 
                              &u8MinuteIn, 
                              sizeof(u8MinuteIn)    );
                          poFI->m_poDataPool -> u32dp_set( DPDP_AUDIO__DIAGINFO_SUSPENDEDDATE_SECONDS, 
                              &u8SecondsIn, 
                              sizeof(u8SecondsIn)    );
                          poFI->m_poDataPool -> u32dp_set( DPDP_AUDIO__DIAGINFO_SUSPENDEDDATE_MONTH, 
                              &u8MonthIn, 
                              sizeof(u8MonthIn)    );
                          poFI->m_poDataPool -> u32dp_set( DPDP_AUDIO__DIAGINFO_SUSPENDEDDATE_DAY, 
                              &u8DayIn, 
                              sizeof(u8DayIn)    );
                          poFI->m_poDataPool -> u32dp_set( DPDP_AUDIO__DIAGINFO_SUSPENDEDDATE_YEAR, 
                              &s16YearIn, 
                              sizeof(s16YearIn)    );
                      }
                  }
                  break;
             }
             case MIDW_EXT_CLOCKFI_C_U16_GETAVAILABLETIMEZONES:
                {
                   vStartInitSequence(MIDW_EXT_CLOCKFI_C_U16_GETAVAILABLETIMEZONES);

                   midw_ext_clockfi_tclMsgGetAvailableTimeZonesMethodResult myDateTime;
                   vGetDataFromAmt(poMessage,myDateTime);

                   //Storing the TimeZone list to a member variable.                    
                   m_u16TZListLength = myDateTime.u16Indexes.size(); 


                   poFI->m_poDataPool -> u32dp_set( DPCLOCK__TZ_LIST_COUNT, 
                      &m_u16TZListLength, 
                      sizeof(m_u16TZListLength)    );

                   //Getting the handle of the List Datapool
                   cldpt_list* pclDPList = dynamic_cast<cldpt_list*>(poFI->m_poDataPool->getdp_Elementhandle( DPCLOCK__LIST_TIMEZONES ));

                   if(pclDPList != NULL)
                   {
                	   tU16 u16MaxListLen = (tU16)TZ_MAX_STRING_LEN;
                	   if(m_u16TZListLength  <(tU16) TZ_MAX_STRING_LEN)
                	   {
                		   u16MaxListLen =m_u16TZListLength;
                	   }

                      for (tU16 u16Index = 0; u16Index < u16MaxListLen; u16Index++)
                      {
                         tU16 u16IndexValue = myDateTime.u16Indexes[u16Index];
                         tChar *chTZString = (tChar *)myDateTime.aszDescriptions[u16Index].szValue;
                         tU16 u16SizeString	=  (tU16)OSAL_u32StringLength(chTZString);
                         tU32 u32Size =  pclDPList->getElementSize(ROW_0, COLUMN_0);
                         if(u16SizeString > u32Size)
                         {
                             chTZString[u32Size - 1]= NULL;
                             pclDPList->writeElement(u16Index, COLUMN_0, chTZString, u32Size);
                         }
                         else
                         {
                        	 pclDPList->writeElement(u16Index, COLUMN_0, chTZString, u16SizeString);
                         }
                         pclDPList->writeElement(u16Index, COLUMN_1, &u16IndexValue, sizeof(u16IndexValue));

                         if (m_poTrace != NULL )
                         {
                            m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
                               TR_CLASS_HSI_CLOCK_MSG,"Timezone Index : %d Timezone Value : %s",u16IndexValue, chTZString );
                         }
                      }
                   }
                   break;
                }
          default :
             {
                break;
             }
             
       }    // end of switch (FID)
       
       /* developer enabled traces */
#ifdef CLOCK_DEVELOPER_TRACES
         vGenericTraceDup ( (tU8)TR_LEVEL_HMI_INFO , CLOCK_TRACE_MSG_IN , 
            u16FID , u16Result );
#endif
       
       break;
      }
      
      /* case denoting the error in recieving method result */
    case AMT_C_U8_CCAMSG_OPCODE_ERROR:
      {
         /* the method result error is dealt separately in common error handler */
         vHandleCCAErrorMessage( poMessage );
         
         break;
      }

    default:
      {
         /* ERROR traces */
         vGenericTraceDup ( (tU8)TR_LEVEL_HMI_ERROR , CLOCK_TRACE_CCA , 
            HSI_CLOCK_UNKNOWN_OPCODE , u16FID );
         
         /* send error message back to server for unkown opcode */
         vSendErrorMessage( u16FID , AMT_C_U16_ERROR_INVALID_OPCODE );
         
         break;
         
      }        /* end of default */
    }          /* end of switch */

  }            /* end of if */

}

/* property update */
/*************************************************************************
* FUNCTION   : vHandleFID_DATEANDTIME_From_VDClock
*
* DESCRIPTION: handle message to get status.
*
* PARAMETER  : poMessage 
*
* RETURNVALUE: NONE
*
* History    : InitialVersion
* 
*************************************************************************/
tVoid clCLOCK_CCAhandler:: vHandleFID_DATEANDTIME_From_VDClock
                                          ( amt_tclServiceData* poMessage )
{
   
   if ( FALSE == bIncomingMsgCheck( poMessage ) )
   {
      /* ERROR trace */
      vGenericTraceDup ( (tU8)TR_LEVEL_HMI_ERROR , CLOCK_TRACE_CCA , 
         HSI_CLOCK_MSG_REC_ERROR , HSI_CLOCK_ZERO );
      
      return;
   }
   
   /*  check the validity of the FI pointer  */
   if ( NULL == poFI )
   {
      /* ERROR trace */
      vGenericTraceDup ( (tU8)TR_LEVEL_HMI_ERROR , CLOCK_TRACE_ASSERT , 
         HSI_CLOCK_NULLPTR_FI , HSI_CLOCK_PROP_TIME );
      
      return; /* Function is not executed further */
   }
   
   /* decode the FID for the help of TRACE */
   tU16 u16FID = poMessage->u16GetFunctionID();
   
   /* switch based on the opcode */
   switch (poMessage->u8GetOpCode())
   {
      
      /* extraction the data from CCA msg from server */
   case AMT_C_U8_CCAMSG_OPCODE_STATUS:
      {
         
         /* developer enabled traces */
#ifdef CLOCK_DEVELOPER_TRACES
         vGenericTraceDup ( (tU8)TR_LEVEL_HMI_INFO , CLOCK_TRACE_MSG_IN , 
            u16FID , HSI_CLOCK_ZERO );
#endif
         

         vStartInitSequence(MIDW_EXT_CLOCKFI_C_U16_LOCALTIMEDATE);
         midw_ext_clockfi_tclMsgLocalTimeDateStatus myDateTime;
         vGetDataFromAmt(poMessage,myDateTime);

       tS16 s16YearIn            =    myDateTime.s16Year;
       tU8 u8MonthIn            =    myDateTime.u8Month;
       tU8 u8DayIn                =    myDateTime.u8Day;
       tU8 u8HourIn            =    myDateTime.u8Hours;
       tU8 u8MinuteIn            =    myDateTime.u8Minutes;
       tU8 u8SecondsIn        =    myDateTime.u8Seconds;
       tU32 u32ScalarValue    =  myDateTime.u32TimeDateValue;
       tU8 u8ChangeMask        =    myDateTime.u8TimeDateStatus;
      tU8 u8WeekDay            =    myDateTime.u8Weekday;

if (m_poTrace != NULL )
   {
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
          TR_CLASS_HSI_CLOCK_MSG,"WeekDay : %d",u8WeekDay );
   }


#ifdef DPCLOCK__CURRENT_WEEKDAY 
         if( NULL != poFI->m_poDataPool )
         {
             poFI->m_poDataPool -> u32dp_set( DPCLOCK__CURRENT_WEEKDAY, &u8WeekDay, sizeof(u8WeekDay)    );
         }
#endif

       /** Later the following two can be used */
       // tU8 u8TimeDateStatusIn    =    myDateTime.u8TimeDateStatus;
       // tU32 u32TimeDateValueIn    =    myDateTime.u32TimeDateValue
       
       /** Write the date and time validity into the datapool Elements */
       tBool bDateValid = FALSE;
       tBool bTimeValid = FALSE;
       tU16 u16Year  = (tU16)s16YearIn;


        
       if(  u8ChangeMask &  IS_TIME_VALID )
       {
           bTimeValid = TRUE;
       }
       if( u8ChangeMask &  IS_DATE_VALID  )
       {
           bDateValid = TRUE;
       }

if (m_poTrace != NULL )
   {
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
          TR_CLASS_HSI_CLOCK_MSG,"Date Validity : %d",bDateValid );
   }

if (m_poTrace != NULL )
   {
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
          TR_CLASS_HSI_CLOCK_MSG,"Time Validity : %d",bTimeValid );
   }


#ifdef DPCLOCK__VALID_DATE 
         if( NULL != poFI->m_poDataPool )
         {
             poFI->m_poDataPool -> u32dp_set( DPCLOCK__VALID_DATE, 
                                              &bDateValid, 
                                              sizeof(bDateValid)    );
         }
#endif
#ifdef DPCLOCK__VALID_TIME 
         if( NULL != poFI->m_poDataPool )
         {
             poFI->m_poDataPool -> u32dp_set( DPCLOCK__VALID_TIME, 
                                              &bTimeValid, 
                                              sizeof(bTimeValid)    );
         }
#endif

    tU8 u8TimeFormat = HSI_CLOCK_ZERO;
    tU8 u8CurrentMode = HSI_CLOCK_ZERO;
    
    if( NULL != poFI->m_poDataPool )
    {
        
        poFI->m_poDataPool -> u32dp_get( DPCLOCK__CURR_TIME_FORMAT, 
                                         &u8TimeFormat, 
                                         sizeof(u8TimeFormat)    );
    }
    
    /* HSI itself has to decide whether the mode is 'AM' or 'PM'.
        Therefore, we are deciding the mode value depending on the Time Format.

        Note :  1. Time value coming from the Server will always be in 24h format (0 - 23h).
                2. Server also updates the HSI with the Time Format data value.
                3. Depending on the Time Format value, HSI has to modify the Time value accordingly.
                    (i.e, if its a 12h format and the Time value is b/w (12h - 23h), substract the time value by 12.)
                4. If it is a 12h format and the Time value received from the server is 0h, we are adding 12 to the time value.
                    (because there is no '0h' value is 12h format) 

    */
    /** Code is commented as all the formatting is left in Common place in CM System for all Time displays */
    if(u8TimeFormat == FORMAT_IS_12H) 
    {
        if( u8HourIn >= MAX_HOUR_IN_12H_FORMAT )
        {
            u8CurrentMode = TIME_MODE_PM;
            // u8HourIn -= MAX_HOUR_IN_12H_FORMAT;
        }
        else
        {
            u8CurrentMode = TIME_MODE_AM;
        }
        if( u8HourIn == HSI_CLOCK_ZERO )
        {
            // u8HourIn = MAX_HOUR_IN_12H_FORMAT;
        }
    }
    else
    {
        // u8HourIn = u8HourIn;
    }

    
    if( NULL != poFI->m_poDataPool )
    {
        // After getting the know the value of Time mode, setting the corresponding data pool element     
        poFI->m_poDataPool -> u32dp_set( DPCLOCK__MODE_AM_PM, 
            &u8CurrentMode, 
            sizeof(u8CurrentMode)    );

        // Trace for Time mode value
        if (m_poTrace != NULL )
        {
            m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
                TR_CLASS_HSI_CLOCK_MSG," Time Mode Value: %d",u8CurrentMode );
        }
    }

    /** To Make Synchronisation between Server Mode Value and HMI Setting of Mode */
    if( NULL != poFI->m_poDataPool )
    {
			tbool IsClockManuallyScreen = FALSE;
			m_poDataPool->u32dp_get(DPCLOCK__TIME_FORMAT_SCREEN_ACTIVE,
					&IsClockManuallyScreen, sizeof(IsClockManuallyScreen));

	        if (m_poTrace != NULL )
	        {
	        	m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
					TR_CLASS_HSI_CLOCK_MSG,"IsClockManuallyScreen = %d ", IsClockManuallyScreen);
	        }

			if (IsClockManuallyScreen == FALSE) {

				poFI->m_poDataPool->u32dp_set(DPSYSTEM__REQ_MODE_AM_PM,
						&u8CurrentMode, sizeof(u8CurrentMode));
			}

        if (m_poTrace != NULL )
        {
            m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
                TR_CLASS_HSI_CLOCK_MSG," REQ Time Mode Value : %d",u8CurrentMode );
        }

    }

         
         /* write all the extracted values to DataPool */
                                                          
#ifdef DPCLOCK__CURR_HOURS
         poFI->bInterfaceToDataPool ( DPCLOCK__CURR_HOURS , &u8HourIn , 
            sizeof(u8HourIn) , HSI_CLOCK_DATA_WRITE );
#endif
#ifdef DPCLOCK__CURR_MINS
         poFI->bInterfaceToDataPool ( DPCLOCK__CURR_MINS , &u8MinuteIn , 
            sizeof(u8MinuteIn) , HSI_CLOCK_DATA_WRITE );
#endif
#ifdef DPCLOCK__CURR_SECONDS
         poFI->bInterfaceToDataPool ( DPCLOCK__CURR_SECONDS , &u8SecondsIn , 
            sizeof(u8MinuteIn) , HSI_CLOCK_DATA_WRITE );
#endif

#ifdef DPCLOCK__CURR_YEAR 
         if( NULL != poFI->m_poDataPool )
         {
             poFI->m_poDataPool -> u32dp_set( DPCLOCK__CURR_YEAR, 
                                              &s16YearIn, 
                                              sizeof(s16YearIn)    );
         }
#endif
   if (m_poTrace != NULL )
   {
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
                         TR_CLASS_HSI_CLOCK_MSG,
                         (tU16) CLOCK_TRACE_YEAR_WRITTEN,
                         1,
                         (tU16 *)&s16YearIn );
   }         
  

#ifdef DPCLOCK__CURR_MONTH
         poFI->bInterfaceToDataPool ( DPCLOCK__CURR_MONTH , &u8MonthIn , 
            sizeof(u8MonthIn) , HSI_CLOCK_DATA_WRITE );
#endif
#ifdef DPCLOCK__CURR_DAY
         poFI->bInterfaceToDataPool ( DPCLOCK__CURR_DAY , &u8DayIn , 
            sizeof(u8DayIn) , HSI_CLOCK_DATA_WRITE );
#endif

#ifdef DPCLOCK__BASE_SCALAR_DATETIMEVALUE 
         if( NULL != poFI->m_poDataPool )
         {
             poFI->m_poDataPool -> u32dp_set( DPCLOCK__BASE_SCALAR_DATETIMEVALUE, 
                                              &u32ScalarValue, 
                                              sizeof(u32ScalarValue)    );
         }
#endif


    //For data reminder popup
    vDataupdateReminder(u32ScalarValue);


   if (m_poTrace != NULL )
   {
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
          TR_CLASS_HSI_CLOCK_MSG,"SCALAR BASE VALUE IS : %d",u32ScalarValue );
   }
   
   if (m_poTrace != NULL )
   {
       
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
          TR_CLASS_HSI_CLOCK_MSG,"Values received from Server =====> Hour : %d, Minute : %d, Seconds : %d, Year : %d, Month : %d , TimeFormat : %d, TimeMode : %d",
                                    u8HourIn, u8MinuteIn, u8SecondsIn, u16Year ,u8MonthIn, u8TimeFormat, u8CurrentMode);

      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
          TR_CLASS_HSI_CLOCK_MSG,"Change Mask Value is : %d",u8ChangeMask );
   }
         
         break;
      }
      
   case AMT_C_U8_CCAMSG_OPCODE_ERROR:
      {
         /* property update error dealt separately in common error handler */
         vHandleCCAErrorMessage( poMessage );
         break;
      }
      
   default:
      {
         /* ERROR trace */
         vGenericTraceDup ((tU8)TR_LEVEL_HMI_ERROR , CLOCK_TRACE_CCA , 
            HSI_CLOCK_UNKNOWN_OPCODE , u16FID );
         
         /* send error message back to server for unkown opcode */
         vSendErrorMessage( u16FID , AMT_C_U16_ERROR_INVALID_OPCODE );
         
         break;
         
      }        /* end of default */
   }          /* end of switch */
   
}
/*************************************************************************
* FUNCTION   : vHandleFID_LOCALTIMEOFFSET_From_VDClock
*
* DESCRIPTION: common handler for Method Results
*
* PARAMETER  : amt_tclServiceData*
*
* RETURNVALUE: NONE
*
* History    : InitialVersion
*
*
*************************************************************************/
tVoid clCLOCK_CCAhandler::vHandleFID_LOCALTIMEOFFSET_From_VDClock(amt_tclServiceData* poMessage)
{   //Error Condition
    if ( FALSE == bIncomingMsgCheck( poMessage ) )
    {
        /* ERROR trace */
        vGenericTraceDup ( (tU8)TR_LEVEL_HMI_ERROR , CLOCK_TRACE_CCA ,
        HSI_CLOCK_MSG_REC_ERROR , HSI_CLOCK_ZERO );
        return;
    }

    /*  check the validity of the FI pointer  */
    if ( NULL == poFI )
    {
        /* ERROR trace */
        vGenericTraceDup ( (tU8)TR_LEVEL_HMI_ERROR , CLOCK_TRACE_ASSERT ,
        HSI_CLOCK_NULLPTR_FI , HSI_CLOCK_PROP_TIME );

        return; /* Function is not executed further */
    }

    /* decode the FID for the help of TRACE */
    tU16 u16FID = poMessage->u16GetFunctionID();

    /* switch based on the opcode */
    switch (poMessage->u8GetOpCode())
    {

        /* extraction the data from CCA msg from server */
        case AMT_C_U8_CCAMSG_OPCODE_STATUS:
        {
            /* developer enabled traces */
            #ifdef CLOCK_DEVELOPER_TRACES
            vGenericTraceDup ( (tU8)TR_LEVEL_HMI_INFO , CLOCK_TRACE_MSG_IN ,
            u16FID , HSI_CLOCK_ZERO );
            #endif

            vStartInitSequence(MIDW_EXT_CLOCKFI_C_U16_LOCALTIMEOFFSET);
            midw_ext_clockfi_tclMsgLocalTimeOffsetStatus oMsgOffsetStatus;
            vGetDataFromAmt(poMessage,oMsgOffsetStatus);

            //Extract the data from the AMT message
            tS64 s64Offset_ScalarValueIn = oMsgOffsetStatus.s64Offset_ScalarValue;
            tbool bIsOffsetPostiveIn = oMsgOffsetStatus.bIsOffsetPostive;
            tS16 s16Offset_YearIn = oMsgOffsetStatus.s16Offset_Year;
            tU8 u8Offset_MonthIn = oMsgOffsetStatus.u8Offset_Month;
            tU8 u8Offset_DayIn =oMsgOffsetStatus.u8Offset_Day;
            tU8 u8Offset_HourIn =oMsgOffsetStatus.u8Offset_Hour;
            tU8 u8Offset_MinuteIn = oMsgOffsetStatus.u8Offset_Minute;
            tU8 u8Offset_SecondIn = oMsgOffsetStatus.u8Offset_Second;
            tU32 u32UTCTimeDate_ScalarValue = oMsgOffsetStatus.u32UTCTimeDate_ScalarValue;
            tS16 s16UTCTimeDate_YearIn = oMsgOffsetStatus.s16UTCTimeDate_Year;
            tU8 u8UTCTimeDate_MonthIn = oMsgOffsetStatus.u8UTCTimeDate_Month;
            tU8 u8UTCTimeDate_DayIn = oMsgOffsetStatus.u8UTCTimeDate_Day;
            tU8 u8UTCTimeDate_HourIn = oMsgOffsetStatus.u8UTCTimeDate_Hour;
            tU8 u8UTCTimeDate_MinuteIn = oMsgOffsetStatus.u8UTCTimeDate_Minute;
            tU8 u8UTCTimeDate_Second = oMsgOffsetStatus.u8UTCTimeDate_Second;
            tU8 u8UTCTimeDate_WeekdayIn = oMsgOffsetStatus.u8UTCTimeDate_Weekday;
            tU8 u8GpsRtcTimeState =  (tU8)oMsgOffsetStatus.enGpsRtcTimeState.enType;

            //The traces for the received data
            if (m_poTrace != NULL )
             {
				m_poTrace->vTrace( TR_LEVEL_HMI_INFO,TR_CLASS_HSI_CLOCK_MSG,"0x8470 s64Offset_ScalarValueIn %d",s64Offset_ScalarValueIn );
                m_poTrace->vTrace( TR_LEVEL_HMI_INFO,TR_CLASS_HSI_CLOCK_MSG,"0x8470 bIsOffsetPostiveIn %d",bIsOffsetPostiveIn );
                m_poTrace->vTrace( TR_LEVEL_HMI_INFO,TR_CLASS_HSI_CLOCK_MSG,"0x8470 s16Offset_YearIn %d",s16Offset_YearIn );
                m_poTrace->vTrace( TR_LEVEL_HMI_INFO,TR_CLASS_HSI_CLOCK_MSG,"0x8470 u8Offset_MonthIn %d",u8Offset_MonthIn );
                m_poTrace->vTrace( TR_LEVEL_HMI_INFO,TR_CLASS_HSI_CLOCK_MSG,"0x8470 u8Offset_Day %d",u8Offset_DayIn );
                m_poTrace->vTrace( TR_LEVEL_HMI_INFO,TR_CLASS_HSI_CLOCK_MSG,"0x8470 u8Offset_HourIn %d",u8Offset_HourIn );
                m_poTrace->vTrace( TR_LEVEL_HMI_INFO,TR_CLASS_HSI_CLOCK_MSG,"0x8470 u8Offset_MinuteIn %d",u8Offset_MinuteIn );
                m_poTrace->vTrace( TR_LEVEL_HMI_INFO,TR_CLASS_HSI_CLOCK_MSG,"0x8470 u8Offset_SecondIn %d",u8Offset_SecondIn );
                m_poTrace->vTrace( TR_LEVEL_HMI_INFO,TR_CLASS_HSI_CLOCK_MSG,"0x8470 u32UTCTimeDate_ScalarValue %d",u32UTCTimeDate_ScalarValue );
                m_poTrace->vTrace( TR_LEVEL_HMI_INFO,TR_CLASS_HSI_CLOCK_MSG,"0x8470 s16UTCTimeDate_YearIn %d",s16UTCTimeDate_YearIn );
                m_poTrace->vTrace( TR_LEVEL_HMI_INFO,TR_CLASS_HSI_CLOCK_MSG,"0x8470 u8UTCTimeDate_MonthIn %d",u8UTCTimeDate_MonthIn );
                m_poTrace->vTrace( TR_LEVEL_HMI_INFO,TR_CLASS_HSI_CLOCK_MSG,"0x8470 u8UTCTimeDate_DayIn %d",u8UTCTimeDate_DayIn );
                m_poTrace->vTrace( TR_LEVEL_HMI_INFO,TR_CLASS_HSI_CLOCK_MSG,"0x8470 u8UTCTimeDate_HourIn %d",u8UTCTimeDate_HourIn );
                m_poTrace->vTrace( TR_LEVEL_HMI_INFO,TR_CLASS_HSI_CLOCK_MSG,"0x8470 u8UTCTimeDate_MinuteIn %d",u8UTCTimeDate_MinuteIn );
                m_poTrace->vTrace( TR_LEVEL_HMI_INFO,TR_CLASS_HSI_CLOCK_MSG,"0x8470 u8UTCTimeDate_Second %d",u8UTCTimeDate_Second );
                m_poTrace->vTrace( TR_LEVEL_HMI_INFO,TR_CLASS_HSI_CLOCK_MSG,"0x8470 u8UTCTimeDate_WeekdayIn %d",u8UTCTimeDate_WeekdayIn );
                m_poTrace->vTrace( TR_LEVEL_HMI_INFO,TR_CLASS_HSI_CLOCK_MSG,"0x8470 u8GpsRtcTimeState %d",u8GpsRtcTimeState );
             }
            //Update all the datapool
            if( NULL != poFI->m_poDataPool)
            {
                poFI->m_poDataPool -> u32dp_set( DPCLOCK__OFFSET_ISOFFSET_POSITIVE,&bIsOffsetPostiveIn,sizeof(bIsOffsetPostiveIn));
                poFI->m_poDataPool -> u32dp_set( DPCLOCK__OFFSET_YEAR,&s16Offset_YearIn,sizeof(s16Offset_YearIn));
                poFI->m_poDataPool -> u32dp_set( DPCLOCK__OFFSET_MONTH,&u8Offset_MonthIn,sizeof(u8Offset_MonthIn));
                poFI->m_poDataPool -> u32dp_set( DPCLOCK__OFFSET_DAY,&u8Offset_DayIn,sizeof(u8Offset_DayIn));
                poFI->m_poDataPool -> u32dp_set( DPCLOCK__OFFSET_HOUR,&u8Offset_HourIn,sizeof(u8Offset_HourIn));
                poFI->m_poDataPool -> u32dp_set( DPCLOCK__OFFSET_MINUTE,&u8Offset_MinuteIn,sizeof(u8Offset_MinuteIn));
                poFI->m_poDataPool -> u32dp_set( DPCLOCK__OFFSET_SECOND,&u8Offset_SecondIn,sizeof(u8Offset_SecondIn));
                poFI->m_poDataPool -> u32dp_set( DPCLOCK__OFFSET_UTCTIMEDATE_SCALARVALUE,&u32UTCTimeDate_ScalarValue,sizeof(u32UTCTimeDate_ScalarValue));
                poFI->m_poDataPool -> u32dp_set( DPCLOCK__OFFSET_UTCTIMEDATE_YEAR,&s16UTCTimeDate_YearIn,sizeof(s16UTCTimeDate_YearIn));
                poFI->m_poDataPool -> u32dp_set( DPCLOCK__OFFSET_UTCTIMEDATE_MONTH,&u8UTCTimeDate_MonthIn,sizeof(u8UTCTimeDate_MonthIn));
                poFI->m_poDataPool -> u32dp_set( DPCLOCK__OFFSET_UTCTIMEDATE_DAY,&u8UTCTimeDate_DayIn,sizeof(u8UTCTimeDate_DayIn));
                poFI->m_poDataPool -> u32dp_set( DPCLOCK__OFFSET_UTCTIMEDATE_HOUR,&u8UTCTimeDate_HourIn,sizeof(u8UTCTimeDate_HourIn));
                poFI->m_poDataPool -> u32dp_set( DPCLOCK__OFFSET_UTCTIMEDATE_MINUTE,&u8UTCTimeDate_MinuteIn,sizeof(u8UTCTimeDate_MinuteIn));
                poFI->m_poDataPool -> u32dp_set( DPCLOCK__OFFSET_UTCTIMEDATE_SECOND,&u8UTCTimeDate_Second,sizeof(u8UTCTimeDate_Second));
                poFI->m_poDataPool -> u32dp_set( DPCLOCK__OFFSET_UTCTIMEDATE_WEEKDAY,&u8UTCTimeDate_WeekdayIn,sizeof(u8UTCTimeDate_WeekdayIn));
                poFI->m_poDataPool -> u32dp_set( DPCLOCK__OFFSET_RTC_TIME_STATE,&u8GpsRtcTimeState,sizeof(u8GpsRtcTimeState));
            }
            //Trigger navi for the updated datapools
            if (NULL != clHSI_CMMngr::pclGetInstance())
            {
              clHSI_CMNAVI* pclHSI_CMNAVI = dynamic_cast<clHSI_CMNAVI*>((clHSI_CMMngr::pclGetInstance())->pHSI_BaseGet(EN_HSI_NAVI));
                if (NULL != pclHSI_CMNAVI)
                 {
                  pclHSI_CMNAVI->m_s64Offset_ScalarValue = s64Offset_ScalarValueIn;
                  sMsg msg(NULL, CMMSG__HSI__NAV__TIME_OFFSET_CHANGED, 0, 0 );
                  pclHSI_CMNAVI->bExecuteMessage(msg);
                 }
            }
        }// End of the Status case
        break;
        case AMT_C_U8_CCAMSG_OPCODE_ERROR:
             {
                /* property update error dealt separately in common error handler */
                vHandleCCAErrorMessage( poMessage );
                break;
             }
        default:
             {
                /* ERROR trace */
                vGenericTraceDup ((tU8)TR_LEVEL_HMI_ERROR , CLOCK_TRACE_CCA ,
                HSI_CLOCK_UNKNOWN_OPCODE , u16FID );

                /* send error message back to server for unkown opcode */
                vSendErrorMessage( u16FID , AMT_C_U16_ERROR_INVALID_OPCODE );

                break;
            }        /* end of default */
    }
}
/*************************************************************************
* FUNCTION   : clCLOCK_CCAhandler::bSendMessage( tU16 u16SvcID, 
*                                                 tU16 u16Fid, 
*                                                 tU8 u8OpCode, 
*                                                 fi_tclTypeBase& oOutData )
*
* DESCRIPTION: This function converts Amt message to visitor class and 
*              gets the data from visitor to the Fi class. Refer FI Manual.Doc.
*
*        Overloaded Function. It contain one extra parameter(u16KindOfTime). 
*        To ensure that Multiple request to server does not overlap, Following two functions of amt_basemessge are used.
*        1. vSetSourceSubID(), 2. u16GetTargetSubID(). 
*        While sending the request to server, the value of source id is set using vSetSourceSubID(). When MethodResult 
*        is received from server, the function vGetTargetSubID() is read and the corresponding datapools are updated
*        Therefore this extra parameter is used to set and get the Sub ID of the Application.
*
* PARAMETER  : u16DestAppID : 16 bit destination application ID
*              u16Fid       : 16 bit Function ID value
*              u16OpCode    : 16 bit CCA message opcode value
*               u16KindOfTime  : 16 bit Application Sub ID
*
* RETURNVALUE: BOOL  : TRUE  -> indicates cca msg posted successfuly
*                      FALSE -> indicates cca msg not posted successfuly 
*
*************************************************************************/
tBool clCLOCK_CCAhandler::bSendMessage( tU16 u16SvcID, tU16 u16Fid, 
                                         tU8 u8OpCode, 
                                         fi_tclTypeBase& oOutData, tU16 u16KindOfTime )
{
   tBool bRetVal = FALSE; 
   
   /** Create the Visitor Message **/
   
     if (m_poTrace != NULL )
     {
         switch ( u16KindOfTime )
            {
         case ETA_CONVERSION:
                m_poTrace->vTrace( TR_LEVEL_HMI_INFO, TR_CLASS_HSI_CLOCK_MSG," CCA Message sent by ETA_CONVERSION " ); break;
         case NAVI_TMCTIME_CONVERSION:
                m_poTrace->vTrace( TR_LEVEL_HMI_INFO, TR_CLASS_HSI_CLOCK_MSG," CCA Message sent by NAVI_TMCTIME_CONVERSION " ); break;
         case TESTMODE_TMC_LASTMSG_DATETIME_CONVERSION:
                m_poTrace->vTrace( TR_LEVEL_HMI_INFO, TR_CLASS_HSI_CLOCK_MSG," CCA Message sent by TESTMODE_TMC_LASTMSG_DATETIME_CONVERSION " ); break;
         case DAY_BEFORE_CURR_TIME_FROMSCALAR:
                m_poTrace->vTrace( TR_LEVEL_HMI_INFO, TR_CLASS_HSI_CLOCK_MSG," CCA Message sent by DAY_BEFORE_CURR_TIME_FROMSCALAR " ); break;
         default: break;
            }
        }

   /*Check code here*//* check the 2nd parameter */
   fi_tclVisitorMessage oOutVisitorMsg( oOutData , 1 );

 
   /* Set the CCA message information */
   oOutVisitorMsg.vInitServiceData(
      CCA_C_U16_APP_UI,                   /* Source app-ID */
      CCA_C_U16_APP_VD_CLOCK,             /* Dest. app-ID */
      AMT_C_U8_CCAMSG_STREAMTYPE_NODATA,  /* stream type*/ /*Check the code*/
      0,                                  /* stream counter*/ /*Check the code*/
      u16GetRegisterId(),                 /* Registry ID */
      u16KindOfTime,                      /* replaces command counter to identify type of time */
      u16SvcID,                           /* Service-ID */
      u16Fid,                             /* Function-ID */
      u8OpCode,                           /* OpCode */
      0,                                  /* ACT *//*Check the code*/
      0,                                  /* Source sub-ID */ /*Check the code*/
      0 );                                /* Dest. sub-ID */ /*Check the code*/
      
   /* Send the message  */
   if( AIL_EN_N_NO_ERROR == enPostMyMessage(&oOutVisitorMsg) )
   {
      /* message sent successfully make return value TRUE */
      bRetVal = TRUE;
   }

   else
   {
      /* message not posted */
      /* error handling and trace in enPostMyMessage() function */
   }
   
   return bRetVal;
}

/*************************************************************************
*  FUNCTION:    tVoid vHandleFID_DATEANDTIME_UPREG_To_VDClock
*
* DESCRIPTION:  Get Property sent to the Server
*
* PARAMETER:   
*
* RETURNVALUE: tVoid
*
* History:
* InitialVersion
* 
*************************************************************************/
tVoid clCLOCK_CCAhandler::vHandleFID_DATEANDTIME_UPREG_To_VDClock( tU8 u8UpdateTimePeriod )
{
    midw_ext_clockfi_tclMsgLocalTimeDateUpReg oMsgData;

    oMsgData.u8UpdatePeriod    =    u8UpdateTimePeriod;

    /* post the message to Server thro Set Property */
      bSendCCAMessage( MIDW_EXT_CLOCKFI_C_U16_LOCALTIMEDATE,
                       AMT_C_U8_CCAMSG_OPCODE_UPREG,
                       oMsgData );
   
}

/*************************************************************************
*  FUNCTION:    tVoid vHandleFID_SCALAR_DATETIME_To_VDClock
*
* DESCRIPTION:  Method to ocnvert the Scalar Value to Readable Structure format.
*
* PARAMETER:   
*
* RETURNVALUE: tVoid
*
* History:
* InitialVersion
* 
*************************************************************************/
tVoid clCLOCK_CCAhandler::vHandleFID_SCALAR_DATETIME_To_VDClock( tU32 u32TimeDateValueIn, tU16 u16KindOfTime)
{
    /** Make the Indicator Flag to TRUE */


    midw_ext_clockfi_tclMsgGetTimeDateFromValueMethodStart oMsgData;

    oMsgData.u32TimeDateValue    =    u32TimeDateValueIn;
    if ( 
        (u16KindOfTime == ETA_CONVERSION) 
        ||
        (u16KindOfTime == DAY_BEFORE_CURR_TIME_FROMSCALAR)
       )
    {
        oMsgData.u8SourceDomain    =    LOCAL_TIME;
    }
    else if (
               (u16KindOfTime == NAVI_TMCTIME_CONVERSION)
               ||
               (u16KindOfTime == TESTMODE_TMC_LASTMSG_DATETIME_CONVERSION)
               ||
               (u16KindOfTime == SXMSUSPENDDATE_UTC_TO_LOCAL)
            )
    {
        oMsgData.u8SourceDomain    =    UTC_SYSTEM_RTC;
    }

    oMsgData.u8TargetDomain    =    LOCAL_TIME;

    /* post the message to Server thro Set Property */
      bSendMessage( CCA_C_U16_SRV_CLOCK ,
MIDW_EXT_CLOCKFI_C_U16_GETTIMEDATEFROMVALUE,
                       AMT_C_U8_CCAMSG_OPCODE_METHODSTART,
                       oMsgData,
                       u16KindOfTime);
   
}

/*************************************************************************
* FUNCTION   : vHandleFID_TIMEFORMAT_From_VDClock
*
* DESCRIPTION: handle message to get status.
*
* PARAMETER  : poMessage 
*
* RETURNVALUE: NONE
*
* History    : InitialVersion
* 
*************************************************************************/
tVoid clCLOCK_CCAhandler:: vHandleFID_TIMEFORMAT_From_VDClock
                                          ( amt_tclServiceData* poMessage )
{
   
   if ( FALSE == bIncomingMsgCheck( poMessage ) )
   {
      /* ERROR trace */
      vGenericTraceDup ( (tU8)TR_LEVEL_HMI_ERROR , CLOCK_TRACE_CCA , 
         HSI_CLOCK_MSG_REC_ERROR , HSI_CLOCK_ZERO );
      
      return;
   }
   
   /*  check the validity of the FI pointer  */
   if ( NULL == poFI )
   {
      /* ERROR trace */
      vGenericTraceDup ( (tU8)TR_LEVEL_HMI_ERROR , CLOCK_TRACE_ASSERT , 
         HSI_CLOCK_NULLPTR_FI , HSI_CLOCK_PROP_TIME );
      
      return; /* Function is not executed further */
   }
   
   /* decode the FID for the help of TRACE */
   tU16 u16FID = poMessage->u16GetFunctionID();
   
   /* switch based on the opcode */
   switch (poMessage->u8GetOpCode())
   {
      
      /* extraction the data from CCA msg from server */
   case AMT_C_U8_CCAMSG_OPCODE_STATUS:
      {
         
         /* developer enabled traces */
#ifdef CLOCK_DEVELOPER_TRACES
         vGenericTraceDup ( (tU8)TR_LEVEL_HMI_INFO , CLOCK_TRACE_MSG_IN , 
            u16FID , HSI_CLOCK_ZERO );
#endif
         

         vStartInitSequence(MIDW_EXT_CLOCKFI_C_U16_TIMEFORMAT);
         midw_ext_clockfi_tclMsgTimeFormatStatus myTimeFormat;
         vGetDataFromAmt(poMessage,myTimeFormat);

         midw_ext_fi_tclDCLK_TEN_TimeFormat::tenType enTimeFormat = myTimeFormat.enTimeFormat.enType;

   if (m_poTrace != NULL )
   {
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
          TR_CLASS_HSI_CLOCK_MSG,"Time Format from Server is : %d (AT SERVER: 1 12h  ::: 2 24h )",(tU8) enTimeFormat );
   }


   tU8 u8TimeFormat = (tU8 )enTimeFormat;

   if( NULL != poFI->m_poDataPool )
   {
       poFI->m_poDataPool -> u32dp_set( DPCLOCK__CURR_TIME_FORMAT, 
                                        &u8TimeFormat, 
                                        sizeof(u8TimeFormat)    );
   }

   /** For Other Components Like Cluster and Navi This datapool is still Kept Alive */
   tBool bTimeFormat = FALSE;

   if( 0 != u8TimeFormat )
   {
       bTimeFormat = (tBool)(u8TimeFormat-1);
   }

   if( NULL != poFI->m_poDataPool )
   {
       poFI->m_poDataPool -> u32dp_set( DPCLOCK__MODE, 
                                        &bTimeFormat, 
                                        sizeof(bTimeFormat)    );
   }

   if (m_poTrace != NULL )
   {
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
          TR_CLASS_HSI_CLOCK_MSG," Time Format Written for NAvi/Cluster : %d ",bTimeFormat );
   }

   if (m_poTrace != NULL )
   {
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
          TR_CLASS_HSI_CLOCK_MSG," Time Format is set into the datapool : %d ",u8TimeFormat );
   }
   
   break;
      }
      
   case AMT_C_U8_CCAMSG_OPCODE_ERROR:
      {
         /* property update error dealt separately in common error handler */
         vHandleCCAErrorMessage( poMessage );
         break;
      }
      
   default:
      {
         /* ERROR trace */
         vGenericTraceDup ( (tU8)TR_LEVEL_HMI_ERROR , CLOCK_TRACE_CCA , 
            HSI_CLOCK_UNKNOWN_OPCODE , u16FID );
         
         /* send error message back to server for unkown opcode */
         vSendErrorMessage( u16FID , AMT_C_U16_ERROR_INVALID_OPCODE );
         
         break;
         
      }        /* end of default */
   }          /* end of switch */
   
}

/*************************************************************************
* FUNCTION   : vHandleFID_DATEFORMAT_From_VDClock
*
* DESCRIPTION: handle message to get status.
*
* PARAMETER  : poMessage 
*
* RETURNVALUE: NONE
*
* History    : InitialVersion
* 
*************************************************************************/
tVoid clCLOCK_CCAhandler:: vHandleFID_DATEFORMAT_From_VDClock
                                          ( amt_tclServiceData* poMessage )
{
   
   if ( FALSE == bIncomingMsgCheck( poMessage ) )
   {
      /* ERROR trace */
      vGenericTraceDup ( (tU8)TR_LEVEL_HMI_ERROR , CLOCK_TRACE_CCA , 
         HSI_CLOCK_MSG_REC_ERROR , HSI_CLOCK_ZERO );
      
      return;
   }
   
   /*  check the validity of the FI pointer  */
   if ( NULL == poFI )
   {
      /* ERROR trace */
      vGenericTraceDup ( (tU8)TR_LEVEL_HMI_ERROR , CLOCK_TRACE_ASSERT , 
         HSI_CLOCK_NULLPTR_FI , HSI_CLOCK_PROP_TIME );
      
      return; /* Function is not executed further */
   }
   
   /* decode the FID for the help of TRACE */
   tU16 u16FID = poMessage->u16GetFunctionID();
   
   /* switch based on the opcode */
   switch (poMessage->u8GetOpCode())
   {
      
      /* extraction the data from CCA msg from server */
   case AMT_C_U8_CCAMSG_OPCODE_STATUS:
      {
         
         /* developer enabled traces */
#ifdef CLOCK_DEVELOPER_TRACES
         vGenericTraceDup ( (tU8)TR_LEVEL_HMI_INFO , CLOCK_TRACE_MSG_IN , 
            u16FID , HSI_CLOCK_ZERO );
#endif
 

        vStartInitSequence(MIDW_EXT_CLOCKFI_C_U16_DATEFORMAT);
        midw_ext_clockfi_tclMsgDateFormatStatus myDateFormat;
         vGetDataFromAmt(poMessage,myDateFormat);

       midw_ext_fi_tclDCLK_TEN_DateFormat::tenType enDateFormat = myDateFormat.enDateFormat.enType;

   if (m_poTrace != NULL )
   {
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
          TR_CLASS_HSI_CLOCK_MSG,"Date Format is : %d (1  dd-mm-yyyy ::: 2 mm-dd-yyyy ::: 3 yyyy-dd-mm )",(tU8) enDateFormat );
   }


   tU8 u8DateFormat = (tU8 )enDateFormat;   
   if( NULL != poFI->m_poDataPool )
   {
       poFI->m_poDataPool -> u32dp_set( DPCLOCK__DATE_FORMAT, 
                                        &u8DateFormat, 
                                        sizeof(u8DateFormat)    );
   }

   if (m_poTrace != NULL )
   {
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
          TR_CLASS_HSI_CLOCK_MSG," Date Format is : %d ",u8DateFormat );
   }

         
         break;
      }
      
   case AMT_C_U8_CCAMSG_OPCODE_ERROR:
      {
         /* property update error dealt separately in common error handler */
         vHandleCCAErrorMessage( poMessage );
         break;
      }
      
   default:
      {
         /* ERROR trace */
         vGenericTraceDup ( (tU8)TR_LEVEL_HMI_ERROR , CLOCK_TRACE_CCA , 
            HSI_CLOCK_UNKNOWN_OPCODE , u16FID );
         
         /* send error message back to server for unkown opcode */
         vSendErrorMessage( u16FID , AMT_C_U16_ERROR_INVALID_OPCODE );
         
         break;
         
      }        /* end of default */
   }          /* end of switch */
   
}

/*************************************************************************
* FUNCTION   : vHandleFID_CLOCKSOURCE_STATUS_From_VDClock
*
* DESCRIPTION: handle message to get status.
*
* PARAMETER  : poMessage 
*
* RETURNVALUE: NONE
*
* History    : InitialVersion
* 
*************************************************************************/
tVoid clCLOCK_CCAhandler:: vHandleFID_CLOCKSOURCE_STATUS_From_VDClock
                                          ( amt_tclServiceData* poMessage )
{
   
   if ( FALSE == bIncomingMsgCheck( poMessage ) )
   {
      /* ERROR trace */
      vGenericTraceDup ( (tU8)TR_LEVEL_HMI_ERROR , CLOCK_TRACE_CCA , 
         HSI_CLOCK_MSG_REC_ERROR , HSI_CLOCK_ZERO );
      
      return;
   }
   
   /*  check the validity of the FI pointer  */
   if ( NULL == poFI )
   {
      /* ERROR trace */
      vGenericTraceDup ( (tU8)TR_LEVEL_HMI_ERROR , CLOCK_TRACE_ASSERT , 
         HSI_CLOCK_NULLPTR_FI , HSI_CLOCK_PROP_TIME );
      
      return; /* Function is not executed further */
   }
   
   /* decode the FID for the help of TRACE */
   tU16 u16FID = poMessage->u16GetFunctionID();
   
   /* switch based on the opcode */
   switch (poMessage->u8GetOpCode())
   {
      
      /* extraction the data from CCA msg from server */
   case AMT_C_U8_CCAMSG_OPCODE_STATUS:
      {
         
         /* developer enabled traces */
#ifdef CLOCK_DEVELOPER_TRACES
         vGenericTraceDup ( (tU8)TR_LEVEL_HMI_INFO , CLOCK_TRACE_MSG_IN , 
            u16FID , HSI_CLOCK_ZERO );
#endif
         
        vStartInitSequence(MIDW_EXT_CLOCKFI_C_U16_CLOCKSOURCESTATUS);
       midw_ext_clockfi_tclMsgClockSourceStatusStatus myClockSourceStatus;
       vGetDataFromAmt(poMessage,myClockSourceStatus);

       midw_ext_fi_tclDCLK_TEN_ClockSourceStatus::tenType enClockSourceStatus = myClockSourceStatus.enSourcesValid.enType;

   if (m_poTrace != NULL )
   {
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
          TR_CLASS_HSI_CLOCK_MSG,"Clock Source Status is : %d (1 No Master ::: 2 Master ::: 3 Master with time only )",(tU8) enClockSourceStatus );
   }


   tU8 u8ClockSourceStatus = (tU8 )enClockSourceStatus;
   
   tBool bIsTimeMasterPresent = FALSE;
   tBool bIsDateMasterPresent = FALSE;

   if( u8ClockSourceStatus == 1 )
   {
       bIsTimeMasterPresent = FALSE;
       bIsDateMasterPresent = FALSE;
   }
   else if( u8ClockSourceStatus == 2 )
   {
       bIsTimeMasterPresent = TRUE;
       bIsDateMasterPresent = TRUE;
   }
   else if( u8ClockSourceStatus == 3 )
   {
       bIsTimeMasterPresent = TRUE;
       bIsDateMasterPresent = FALSE;
   }
   else
   {
       // Nothing to do Here.
   }
   
   if( NULL != poFI->m_poDataPool )
   {
       poFI->m_poDataPool -> u32dp_set( DPCLOCK__IS_TIMEMASTER_PRESENT, 
                                        &bIsTimeMasterPresent, 
                                        sizeof(bIsTimeMasterPresent)    );
   }
   
   if( NULL != poFI->m_poDataPool )
   {
       poFI->m_poDataPool -> u32dp_set( DPCLOCK__IS_DATEMASTER_PRESENT, 
                                        &bIsDateMasterPresent, 
                                        sizeof(bIsDateMasterPresent)    );
   }

   if (m_poTrace != NULL )
   {
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
          TR_CLASS_HSI_CLOCK_MSG," Time Master Presence : %d ",bIsTimeMasterPresent );
   }

   if (m_poTrace != NULL )
   {
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
          TR_CLASS_HSI_CLOCK_MSG," Date Master Presence : %d ",bIsDateMasterPresent );
   }
         
         break;
      }
      
   case AMT_C_U8_CCAMSG_OPCODE_ERROR:
      {
         /* property update error dealt separately in common error handler */
         vHandleCCAErrorMessage( poMessage );
         break;
      }
      
   default:
      {
         /* ERROR trace */
         vGenericTraceDup ( (tU8)TR_LEVEL_HMI_ERROR , CLOCK_TRACE_CCA , 
            HSI_CLOCK_UNKNOWN_OPCODE , u16FID );
         
         /* send error message back to server for unkown opcode */
         vSendErrorMessage( u16FID , AMT_C_U16_ERROR_INVALID_OPCODE );
         
         break;
         
      }        /* end of default */
   }          /* end of switch */
   
}

/*************************************************************************
* FUNCTION   : vHandleFID_FORMATSOURCE_STATUS_From_VDClock
*
* DESCRIPTION: handle message to get status.
*
* PARAMETER  : poMessage 
*
* RETURNVALUE: NONE
*
* History    : InitialVersion
* 
*************************************************************************/
tVoid clCLOCK_CCAhandler:: vHandleFID_FORMATSOURCE_STATUS_From_VDClock
                                          ( amt_tclServiceData* poMessage )
{
   
   if ( FALSE == bIncomingMsgCheck( poMessage ) )
   {
      /* ERROR trace */
      vGenericTraceDup ( (tU8)TR_LEVEL_HMI_ERROR , CLOCK_TRACE_CCA , 
         HSI_CLOCK_MSG_REC_ERROR , HSI_CLOCK_ZERO );
      
      return;
   }
   
   /*  check the validity of the FI pointer  */
   if ( NULL == poFI )
   {
      /* ERROR trace */
      vGenericTraceDup ( (tU8)TR_LEVEL_HMI_ERROR , CLOCK_TRACE_ASSERT , 
         HSI_CLOCK_NULLPTR_FI , HSI_CLOCK_PROP_TIME );
      
      return; /* Function is not executed further */
   }
   
   /* decode the FID for the help of TRACE */
   tU16 u16FID = poMessage->u16GetFunctionID();
   
   /* switch based on the opcode */
   switch (poMessage->u8GetOpCode())
   {
      
      /* extraction the data from CCA msg from server */
   case AMT_C_U8_CCAMSG_OPCODE_STATUS:
      {
         
         /* developer enabled traces */
#ifdef CLOCK_DEVELOPER_TRACES
         vGenericTraceDup ( (tU8)TR_LEVEL_HMI_INFO , CLOCK_TRACE_MSG_IN , 
            u16FID , HSI_CLOCK_ZERO );
#endif
         
     

         vStartInitSequence(MIDW_EXT_CLOCKFI_C_U16_FORMATSOURCESTATUS);
         midw_ext_clockfi_tclMsgFormatSourceStatusStatus myFormatSourceStatus;
         vGetDataFromAmt(poMessage,myFormatSourceStatus);



       midw_ext_fi_tclDCLK_TEN_FormatSourceStatus::tenType enFormatSourceStatus = myFormatSourceStatus.enFormatSourceStatus.enType;

   if (m_poTrace != NULL )
   {
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
          TR_CLASS_HSI_CLOCK_MSG,"Format Source Status is : %d (1 No Master ::: 2 Master ::: 3 Master with time only )",(tU8) enFormatSourceStatus );
   }


   tU8 u8FormatSourceStatus = (tU8 )enFormatSourceStatus;
   
   tBool bIsTimeFormatMasterPresent = FALSE;
   tBool bIsDateFormatMasterPresent = FALSE;

   if( u8FormatSourceStatus == 1 )
   {
       bIsTimeFormatMasterPresent = FALSE;
       bIsDateFormatMasterPresent = FALSE;
   }
   else if( u8FormatSourceStatus == 2 )
   {
       bIsTimeFormatMasterPresent = TRUE;
       bIsDateFormatMasterPresent = TRUE;
   }
   else if( u8FormatSourceStatus == 3 )
   {
       bIsTimeFormatMasterPresent = TRUE;
       bIsDateFormatMasterPresent = FALSE;
   }
   else
   {
       // Nothing to do Here.
   }
   
   if( NULL != poFI->m_poDataPool )
   {
       poFI->m_poDataPool -> u32dp_set( DPCLOCK__IS_TIMEFORMATMASTER_PRESENT, 
                                        &bIsTimeFormatMasterPresent, 
                                        sizeof(bIsTimeFormatMasterPresent)    );
   }
   
   if( NULL != poFI->m_poDataPool )
   {
       poFI->m_poDataPool -> u32dp_set( DPCLOCK__IS_DATEFORMATMASTER_PRESENT, 
                                        &bIsDateFormatMasterPresent, 
                                        sizeof(bIsDateFormatMasterPresent)    );
   }

   if (m_poTrace != NULL )
   {
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
          TR_CLASS_HSI_CLOCK_MSG," Time Format Master Presence : %d ",bIsTimeFormatMasterPresent );
   }

   if (m_poTrace != NULL )
   {
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
          TR_CLASS_HSI_CLOCK_MSG," Date Format Master Presence : %d ",bIsDateFormatMasterPresent );
   }
         break;
      }
      
   case AMT_C_U8_CCAMSG_OPCODE_ERROR:
      {
         /* property update error dealt separately in common error handler */
         vHandleCCAErrorMessage( poMessage );
         break;
      }
      
   default:
      {
         /* ERROR trace */
         vGenericTraceDup ( (tU8)TR_LEVEL_HMI_ERROR , CLOCK_TRACE_CCA , 
            HSI_CLOCK_UNKNOWN_OPCODE , u16FID );
         
         /* send error message back to server for unkown opcode */
         vSendErrorMessage( u16FID , AMT_C_U16_ERROR_INVALID_OPCODE );
         
         break;
         
      }        /* end of default */
   }          /* end of switch */
   
}
/*************************************************************************
*  FUNCTION:    tVoid vHandleFID_TIMEDATE_SOURCE_FORMAT_SET_TO_VDClock
*
* DESCRIPTION:  Set Property sent to the Server
*
* PARAMETER:   
*
* RETURNVALUE: tVoid
*
* History:
* InitialVersion
* 
*************************************************************************/
tVoid clCLOCK_CCAhandler::vHandleFID_TIMEDATE_SOURCE_FORMAT_SET_TO_VDClock( tU16 u16Fid, tU8 u8Value )
{

    switch(u16Fid)
    {
        case MIDW_EXT_CLOCKFI_C_U16_TIMEFORMAT:
        {
            midw_ext_clockfi_tclMsgTimeFormatSet oMsgData;
            oMsgData.enTimeFormat.enType = (midw_ext_fi_tclDCLK_TEN_TimeFormat::tenType)u8Value;

    /* post the message to Server thro Set Property */
      bSendCCAMessage(u16Fid,
                       AMT_C_U8_CCAMSG_OPCODE_SET,
                       oMsgData );
            break;
        }
        case MIDW_EXT_CLOCKFI_C_U16_DATEFORMAT:
        {
            midw_ext_clockfi_tclMsgDateFormatSet oMsgData;
            oMsgData.enDateFormat.enType = (midw_ext_fi_tclDCLK_TEN_DateFormat::tenType)u8Value;

    /* post the message to Server thro Set Property */
      bSendCCAMessage( u16Fid,
                       AMT_C_U8_CCAMSG_OPCODE_SET,
                       oMsgData );
            break;
        }
        default:
        {
            // No6thing to do here.
            break;
        }
    }
   
}
/*************************************************************************
*  FUNCTION:    tVoid vHandleFID_TIME_SET_TO_VDClock
*
* DESCRIPTION:  Time Set Property sent to the Server
*
* PARAMETER:   
*
* RETURNVALUE: tVoid
*
* History:
* InitialVersion
* 
*************************************************************************/
tVoid clCLOCK_CCAhandler::vHandleFID_TIME_SET_TO_VDClock( tVoid )
{
    midw_ext_clockfi_tclMsgLocalTimeDateSet oMsgData;

    /** Read Hours Value and fill the message.*/
    tU8 u8ReqHours = 0;

    tU8 u8TimeFormat = 0;
    tU8 u8RequiredMode = 0;

    if (m_poTrace != NULL )
   {
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
          TR_CLASS_HSI_CLOCK_MSG," vHandleFID_TIME_SET_TO_VDClock function is called");
   }
    
    if( NULL != poFI->m_poDataPool )
    {
        
        poFI->m_poDataPool -> u32dp_get( DPCLOCK__CURR_TIME_FORMAT, 
                                         &u8TimeFormat, 
                                         sizeof(u8TimeFormat)    );
    
        
        poFI->m_poDataPool -> u32dp_get( DPSYSTEM__REQ_MODE_AM_PM, 
                                         &u8RequiredMode, 
                                         sizeof(u8RequiredMode)    );

        poFI->m_poDataPool -> u32dp_get( DPCLOCK__REQ_HOURS, 
                                         &u8ReqHours, 
                                         sizeof(u8ReqHours)    );
    }

    if( (u8TimeFormat == 1) && (u8RequiredMode == 1) )
    {
        if( u8ReqHours < 12 )
        {
            u8ReqHours += 12;
        }
    }
    else
    {
        u8ReqHours = u8ReqHours;
    }

    oMsgData.u8Hours = u8ReqHours;

    if (m_poTrace != NULL )
   {
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
          TR_CLASS_HSI_CLOCK_MSG," Value of hours Send for setting is: %d ",u8ReqHours );
   }

    /** Read Minutes and Write into the Message */
    tU8 u8ReqMins = 0;
    
    if( NULL != poFI->m_poDataPool )
    {
        poFI->m_poDataPool -> u32dp_get( DPCLOCK__REQ_MINS, 
                                         &u8ReqMins, 
                                         sizeof(u8ReqMins)    );
    }
    oMsgData.u8Minutes = u8ReqMins;

    if (m_poTrace != NULL )
   {
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
          TR_CLASS_HSI_CLOCK_MSG," Value of Mins Send for setting is: %d ",u8ReqMins );
   }

    

    /** Read Minutes and Write into the Message */
    tU8 u8ReqSeoconds = 0;
    
    if( NULL != poFI->m_poDataPool )
    {
        poFI->m_poDataPool -> u32dp_get( DPCLOCK__CURR_SECONDS, 
                                         &u8ReqSeoconds, 
                                         sizeof(u8ReqSeoconds)    );
    }
    oMsgData.u8Seconds = u8ReqSeoconds;

    if (m_poTrace != NULL )
   {
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
          TR_CLASS_HSI_CLOCK_MSG," Value of Seconds Send for setting is: %d ",u8ReqSeoconds );
   }

    

    /** Read Minutes and Write into the Message */
    tS16 s16ReqYear = 0;
    
    if( NULL != poFI->m_poDataPool )
    {
        poFI->m_poDataPool -> u32dp_get( DPCLOCK__CURR_YEAR, 
                                         &s16ReqYear, 
                                         sizeof(s16ReqYear)    );
    }
    oMsgData.s16Year = s16ReqYear;

    

    if (m_poTrace != NULL )
   {
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
          TR_CLASS_HSI_CLOCK_MSG," Value of Year Send for setting is: %d ",s16ReqYear );
   }

    

    /** Read Minutes and Write into the Message */
    tU8 u8ReqMonth = 0;
    
    if( NULL != poFI->m_poDataPool )
    {
        poFI->m_poDataPool -> u32dp_get( DPCLOCK__CURR_MONTH, 
                                         &u8ReqMonth, 
                                         sizeof(u8ReqMonth)    );
    }
    oMsgData.u8Month = u8ReqMonth;

    

    if (m_poTrace != NULL )
   {
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
          TR_CLASS_HSI_CLOCK_MSG," Value of Month Send for setting is: %d ",u8ReqMonth );
   }

    

    /** Read Minutes and Write into the Message */
    tU8 u8ReqDay = 0;
    
    if( NULL != poFI->m_poDataPool )
    {
        poFI->m_poDataPool -> u32dp_get( DPCLOCK__CURR_DAY, 
                                         &u8ReqDay, 
                                         sizeof(u8ReqDay)    );
    }
    oMsgData.u8Day = u8ReqDay;

    

    if (m_poTrace != NULL )
   {
      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
          TR_CLASS_HSI_CLOCK_MSG," Value of Day Send for setting is: %d ",u8ReqDay );
   }


    oMsgData.u8ChangeMask = 0x77;

    /* post the message to Server thro Set Property */
      bSendCCAMessage(MIDW_EXT_CLOCKFI_C_U16_LOCALTIMEDATE,
                       AMT_C_U8_CCAMSG_OPCODE_SET,
                       oMsgData );
   
}

/*************************************************************************
*  FUNCTION:    tVoid vHandleFID_DATE_SET_TO_VDClock
*
* DESCRIPTION:  Time Set Property sent to the Server
*
* PARAMETER:   
*
* RETURNVALUE: tVoid
*
* History:
* InitialVersion
* 
*************************************************************************/
tVoid clCLOCK_CCAhandler::vHandleFID_DATE_SET_TO_VDClock( tVoid )
{
    midw_ext_clockfi_tclMsgLocalTimeDateSet oMsgData;

    /** Read Hours Value and fill the message.*/
    tU8 u8ReqHours = 0;
    
    if( NULL != poFI->m_poDataPool )
    {
        poFI->m_poDataPool -> u32dp_get( DPCLOCK__CURR_HOURS, 
                                         &u8ReqHours, 
                                         sizeof(u8ReqHours)    );
    }
    oMsgData.u8Hours = u8ReqHours;

    /** Read Minutes and Write into the Message */
    tU8 u8ReqMins = 0;
    
    if( NULL != poFI->m_poDataPool )
    {
        poFI->m_poDataPool -> u32dp_get( DPCLOCK__CURR_MINS, 
                                         &u8ReqMins, 
                                         sizeof(u8ReqMins)    );
    }
    oMsgData.u8Minutes = u8ReqMins;

    

    /** Read Minutes and Write into the Message */
    tU8 u8ReqSeoconds = 0;
    
    if( NULL != poFI->m_poDataPool )
    {
        poFI->m_poDataPool -> u32dp_get( DPCLOCK__CURR_SECONDS, 
                                         &u8ReqSeoconds, 
                                         sizeof(u8ReqSeoconds)    );
    }
    oMsgData.u8Seconds = u8ReqSeoconds;

    

    /** Read Minutes and Write into the Message */
    tS16 s16ReqYear = 0;
    
    if( NULL != poFI->m_poDataPool )
    {
        poFI->m_poDataPool -> u32dp_get( DPCLOCK__REQ_YEAR, 
                                         &s16ReqYear, 
                                         sizeof(s16ReqYear)    );
    }
    oMsgData.s16Year = s16ReqYear;

    

    /** Read Minutes and Write into the Message */
    tU8 u8ReqMonth = 0;
    
    if( NULL != poFI->m_poDataPool )
    {
        poFI->m_poDataPool -> u32dp_get( DPCLOCK__REQ_MONTH, 
                                         &u8ReqMonth, 
                                         sizeof(u8ReqMonth)    );
    }
    oMsgData.u8Month = u8ReqMonth;

    

    /** Read Minutes and Write into the Message */
    tU8 u8ReqDay = 0;
    
    if( NULL != poFI->m_poDataPool )
    {
        poFI->m_poDataPool -> u32dp_get( DPCLOCK__REQ_DAY, 
                                         &u8ReqDay, 
                                         sizeof(u8ReqDay)    );
    }
    oMsgData.u8Day = u8ReqDay;


    oMsgData.u8ChangeMask = 0x77;

    /* post the message to Server thro Set Property */
      bSendCCAMessage( MIDW_EXT_CLOCKFI_C_U16_LOCALTIMEDATE,
                       AMT_C_U8_CCAMSG_OPCODE_SET,
                       oMsgData );
   
}
/*************************************************************************
*  FUNCTION:    tVoid vHandleFID_SCALAR_GETDATETIME_To_VDClock
*
* DESCRIPTION:  To get the timedate Update from Server upon the request of Navigation
*
* PARAMETER:   
*
* RETURNVALUE: tVoid
*
* History:
* InitialVersion
* 
*************************************************************************/
tVoid clCLOCK_CCAhandler::vHandleFID_SCALAR_GETDATETIME_To_VDClock( tVoid )
{
    midw_ext_clockfi_tclMsgLocalTimeDateStatus oMsgData;

    /* post the message to Server thro Set Property */
      bSendCCAMessage( MIDW_EXT_CLOCKFI_C_U16_LOCALTIMEDATE,
                       AMT_C_U8_CCAMSG_OPCODE_GET,
                       oMsgData );
   
}

/* property update */
/*************************************************************************
* FUNCTION   : vHandleFID_GPSTIMESTATUS_From_VDClock
*
* DESCRIPTION: handle message to Get GPSTIME status.
*
* PARAMETER  : poMessage 
*
* RETURNVALUE: NONE
*
* History    : InitialVersion
* 
*************************************************************************/
tVoid clCLOCK_CCAhandler::vHandleFID_GPSTIMESTATUS_From_VDClock
( amt_tclServiceData* poMessage )
{
   if ( FALSE == bIncomingMsgCheck( poMessage ) )
   {
      /* ERROR trace */
      vGenericTraceDup ( (tU8)TR_LEVEL_HMI_ERROR , CLOCK_TRACE_CCA , 
         HSI_CLOCK_MSG_REC_ERROR , HSI_CLOCK_ZERO );

      return;
   }

   /*  check the validity of the FI pointer  */
   if ( NULL == poFI )
   {
      /* ERROR trace */
      vGenericTraceDup ( (tU8)TR_LEVEL_HMI_ERROR , CLOCK_TRACE_ASSERT , 
         HSI_CLOCK_NULLPTR_FI , HSI_CLOCK_PROP_TIME );

      return; /* Function is not executed further */
   }

   /* decode the FID for the help of TRACE */
   tU16 u16FID = poMessage->u16GetFunctionID();

   /* switch based on the opcode */
   switch (poMessage->u8GetOpCode())
   {
   case AMT_C_U8_CCAMSG_OPCODE_STATUS:
      {

         /* developer enabled traces */
#ifdef CLOCK_DEVELOPER_TRACES
         vGenericTraceDup ((tU8) TR_LEVEL_HMI_INFO , CLOCK_TRACE_MSG_IN , 
            u16FID , HSI_CLOCK_ZERO );
#endif

         vStartInitSequence(MIDW_EXT_CLOCKFI_C_U16_GPS_AUTOSYNC);
         midw_ext_clockfi_tclMsgGPS_AutoSyncStatus oClkmsg_tclGPSTimeStatus;
         vGetDataFromAmt(poMessage,oClkmsg_tclGPSTimeStatus);

         //Added by sos1kor to Extract the  GPS Time Status Parameter from Clock server
         {
            tU8 u8ClockSettings = 0;
            tBool bGPS_Setting = oClkmsg_tclGPSTimeStatus.bEnabled;
            tBool bPositionBased = oClkmsg_tclGPSTimeStatus.bPositionBased;

            if(NULL != m_poTrace)
            {
               m_poTrace->vTrace( TR_LEVEL_HMI_INFO , 
                  TR_CLASS_HSI_CLOCK_MSG ,
                  "HMI Recv: GPS_Setting = %d, PositionBased = %d", bGPS_Setting , bPositionBased);
            }

            if( (FALSE == bGPS_Setting) && (FALSE == bPositionBased) )
            {
               u8ClockSettings = MANUAL;
            }
            else if( (TRUE == bGPS_Setting) && (FALSE == bPositionBased) )
            {
               u8ClockSettings = TIME_ZONE;
            }
            else if( (TRUE == bGPS_Setting) && (TRUE == bPositionBased) )
            {
               u8ClockSettings = AUTO;
            }
            else
            {
               //nothing to do here.
            }

            if( NULL != poFI->m_poDataPool )
            {
               poFI->m_poDataPool -> u32dp_set( DPCLOCK__GPS_TIME_STATUS, 
                  &u8ClockSettings, 
                  sizeof(u8ClockSettings)    );
            }

         }
      }
      break;

   case AMT_C_U8_CCAMSG_OPCODE_ERROR:
      {
         /* property update error dealt separately in common error handler */
         vHandleCCAErrorMessage( poMessage );
         break;
      }

   default:
      {
         /* ERROR trace */
         vGenericTraceDup ((tU8) TR_LEVEL_HMI_ERROR , CLOCK_TRACE_CCA , 
            HSI_CLOCK_UNKNOWN_OPCODE , u16FID );

         /* send error message back to server for unkown opcode */
         vSendErrorMessage( u16FID , AMT_C_U16_ERROR_INVALID_OPCODE );

         break;
      }      
   }
}


/* property update */
/*************************************************************************
* FUNCTION   : vHandleFID_DAYLIGHTSAVINGTIME_From_VDClock
*
* DESCRIPTION: handle message to Get DAYLIGHTSAVINGTIME status.
*
* PARAMETER  : poMessage 
*
* RETURNVALUE: NONE
*
* History    : InitialVersion
* 
*************************************************************************/
tVoid clCLOCK_CCAhandler::vHandleFID_DAYLIGHTSAVINGTIME_From_VDClock
( amt_tclServiceData* poMessage )
{
   if ( FALSE == bIncomingMsgCheck( poMessage ) )
   {
      /* ERROR trace */
      vGenericTraceDup ( (tU8)TR_LEVEL_HMI_ERROR , CLOCK_TRACE_CCA , 
         HSI_CLOCK_MSG_REC_ERROR , HSI_CLOCK_ZERO );

      return;
   }

   /*  check the validity of the FI pointer  */
   if ( NULL == poFI )
   {
      /* ERROR trace */
      vGenericTraceDup ( (tU8)TR_LEVEL_HMI_ERROR , CLOCK_TRACE_ASSERT , 
         HSI_CLOCK_NULLPTR_FI , HSI_CLOCK_PROP_TIME );

      return; /* Function is not executed further */
   }

   /* decode the FID for the help of TRACE */
   tU16 u16FID = poMessage->u16GetFunctionID();

   /* switch based on the opcode */
   switch (poMessage->u8GetOpCode())
   {
   case AMT_C_U8_CCAMSG_OPCODE_STATUS:
      {

         /* developer enabled traces */
#ifdef CLOCK_DEVELOPER_TRACES
         vGenericTraceDup ((tU8) TR_LEVEL_HMI_INFO , CLOCK_TRACE_MSG_IN , 
            u16FID , HSI_CLOCK_ZERO );
#endif

         vStartInitSequence(MIDW_EXT_CLOCKFI_C_U16_DAYLIGHTSAVINGTIME);
         midw_ext_clockfi_tclMsgDaylightSavingTimeStatus oClkmsg_tclDayLightSavTime;
         vGetDataFromAmt(poMessage,oClkmsg_tclDayLightSavTime);

         //Added by sos1kor to Extract the DayLight Save Time Parameter from Clock server
         {
            tBool bDST_Setting = oClkmsg_tclDayLightSavTime.bDST_Status;

            if( NULL != poFI->m_poDataPool )
            {
               poFI->m_poDataPool -> u32dp_set( DPCLOCK__DST_SETTING, 
                  &bDST_Setting, 
                  sizeof(bDST_Setting)    );
            }

         }
      }
      break;

   case AMT_C_U8_CCAMSG_OPCODE_ERROR:
      {
         /* property update error dealt separately in common error handler */
         vHandleCCAErrorMessage( poMessage );
         break;
      }

   default:
      {
         /* ERROR trace */
         vGenericTraceDup ( (tU8)TR_LEVEL_HMI_ERROR , CLOCK_TRACE_CCA , 
            HSI_CLOCK_UNKNOWN_OPCODE , u16FID );

         /* send error message back to server for unkown opcode */
         vSendErrorMessage( u16FID , AMT_C_U16_ERROR_INVALID_OPCODE );

         break;
      }      
   }
}
/* property update */
/*************************************************************************
* FUNCTION   : vHandleFID_GPS_WeekEpoch_From_VDClock
*
* DESCRIPTION: handle message to Get GPS Epoch status.
*
* PARAMETER  : poMessage
*
* RETURNVALUE: NONE
*
* History    : InitialVersion
*
*************************************************************************/
tVoid clCLOCK_CCAhandler::vHandleFID_GPS_WeekEpoch_From_VDClock(amt_tclServiceData *poMessage)
{
	if ( FALSE == bIncomingMsgCheck( poMessage ) )
	   {
	      vGenericTraceDup ( (tU8)TR_LEVEL_HMI_ERROR , CLOCK_TRACE_CCA ,
	         HSI_CLOCK_MSG_REC_ERROR , HSI_CLOCK_ZERO );
	      return;
	   }
	   if ( NULL == poFI )
	   {
	      vGenericTraceDup ( (tU8)TR_LEVEL_HMI_ERROR , CLOCK_TRACE_ASSERT ,
	         HSI_CLOCK_NULLPTR_FI , HSI_CLOCK_PROP_TIME );
	      return; /* Function is not executed further */
	   }
	   tU16 u16FID = poMessage->u16GetFunctionID();
	   switch (poMessage->u8GetOpCode())
	   {
			case AMT_C_U8_CCAMSG_OPCODE_STATUS:
			{
				vStartInitSequence(MIDW_EXT_CLOCKFI_C_U16_GPS_WEEKEPOCH);
				midw_ext_clockfi_tclMsgGPS_WeekEpochStatus oClkmsg_tclGPSEpoch;
				vGetDataFromAmt(poMessage,oClkmsg_tclGPSEpoch);
				tU8 u8TimeInterval = (tU8)oClkmsg_tclGPSEpoch.GPSepoch.enType;
				if( NULL != poFI->m_poDataPool )
				{
					poFI->m_poDataPool -> u32dp_set( DPCLOCK__TIME_INTERVAL,
					&u8TimeInterval,
					sizeof(u8TimeInterval)	);
				}
				break;
			}
			case AMT_C_U8_CCAMSG_OPCODE_ERROR:
			{
				vHandleCCAErrorMessage( poMessage );
				break;
			}
			default:
			{
				vGenericTraceDup ( (tU8)TR_LEVEL_HMI_ERROR , CLOCK_TRACE_CCA ,
				HSI_CLOCK_UNKNOWN_OPCODE , u16FID );
				vSendErrorMessage( u16FID , AMT_C_U16_ERROR_INVALID_OPCODE );
				break;
			}
		}
}
/*************************************************************************
*  FUNCTION:    tVoid vHandleFID_TIME_SET_TO_VDClock
*
* DESCRIPTION:  DayLight Save Time Set Property sent to the Server
*
* PARAMETER:   tBool bDST_Setting
*
* RETURNVALUE: tVoid
*
* History:
* InitialVersion
* 
*************************************************************************/
tVoid clCLOCK_CCAhandler::vHandleFID_DST_SET_TO_VDClock(tBool bDST_Setting)
{
    midw_ext_fi_tclDCLK_TEN_DST_Setting oMsgData;    

   oMsgData.enType= (midw_ext_fi_tclDCLK_TEN_DST_Setting::tenType)bDST_Setting;

    /* post the message to Server thro Set Property */
      bSendCCAMessage( MIDW_EXT_CLOCKFI_C_U16_DAYLIGHTSAVINGTIME,
                       AMT_C_U8_CCAMSG_OPCODE_SET,
                       oMsgData );      
}


/*************************************************************************
*  FUNCTION:    tVoid vHandleFID_GPS_TIME_STATUS_SET_TO_VDClock
*
* DESCRIPTION:  DayLight Save Time Set Property sent to the Server
*
* PARAMETER:   tBool bGPS_Setting
*
* RETURNVALUE: tVoid
*
* History:
* InitialVersion
* 
*************************************************************************/
tVoid clCLOCK_CCAhandler::vHandleFID_GPS_TIME_STATUS_SET_TO_VDClock(tBool bGPS_Setting, tBool bPositionbased)
{
   midw_ext_clockfi_tclMsgGPS_AutoSyncSet oMsgData;    

   oMsgData.bEnabled =  bGPS_Setting;
   oMsgData.bPositionBased = bPositionbased;

   /* post the message to Server thro Set Property */
   bSendCCAMessage( MIDW_EXT_CLOCKFI_C_U16_GPS_AUTOSYNC,
      AMT_C_U8_CCAMSG_OPCODE_SET,
      oMsgData );      
}

/*************************************************************************
*  FUNCTION:    tVoid vHandleFID_TIME_SET_TO_VDClock
*
* DESCRIPTION:  Time Zone Set Property sent to the Server
*
* PARAMETER:   tU8 u16TimeZoneIndex
*
* RETURNVALUE: tVoid
*
* History:
* InitialVersion
* 
*************************************************************************/
tVoid clCLOCK_CCAhandler::vHandleFID_TIMEZONE_SET_TO_VDClock(tU16 u16TimeZoneIndex)
{
    midw_ext_clockfi_tclMsgTimeZoneSet oMsgData;    

   oMsgData.u16Index = u16TimeZoneIndex;

    /* post the message to Server thro Set Property */
      bSendCCAMessage( MIDW_EXT_CLOCKFI_C_U16_TIMEZONE,
                       AMT_C_U8_CCAMSG_OPCODE_SET,
                       oMsgData );      
}

/* property update */
/*************************************************************************
* FUNCTION   : vHandleFID_TIMEZONE_From_VDClock
*
* DESCRIPTION: handle message to Get TIMEZONE status.
*
* PARAMETER  : poMessage 
*
* RETURNVALUE: NONE
*
* History    : InitialVersion
* 
*************************************************************************/
tVoid clCLOCK_CCAhandler::vHandleFID_TIMEZONE_From_VDClock
( amt_tclServiceData* poMessage )
{
   if ( FALSE == bIncomingMsgCheck( poMessage ) )
   {
      /* ERROR trace */
      vGenericTraceDup ((tU8) TR_LEVEL_HMI_ERROR , CLOCK_TRACE_CCA , 
         HSI_CLOCK_MSG_REC_ERROR , HSI_CLOCK_ZERO );

      return;
   }

   /*  check the validity of the FI pointer  */
   if ( NULL == poFI )
   {
      /* ERROR trace */
      vGenericTraceDup ( (tU8)TR_LEVEL_HMI_ERROR , CLOCK_TRACE_ASSERT , 
         HSI_CLOCK_NULLPTR_FI , HSI_CLOCK_PROP_TIME );

      return; /* Function is not executed further */
   }

   /* decode the FID for the help of TRACE */
   tU16 u16FID = poMessage->u16GetFunctionID();

   /* switch based on the opcode */
   switch (poMessage->u8GetOpCode())
   {
   case AMT_C_U8_CCAMSG_OPCODE_STATUS:
      {

         /* developer enabled traces */
#ifdef CLOCK_DEVELOPER_TRACES
         vGenericTraceDup ( (tU8)TR_LEVEL_HMI_INFO , CLOCK_TRACE_MSG_IN , 
            u16FID , HSI_CLOCK_ZERO );
#endif      

         vStartInitSequence(MIDW_EXT_CLOCKFI_C_U16_TIMEZONE);
         midw_ext_clockfi_tclMsgTimeZoneStatus oClkmsg_tclTimeZone;
         vGetDataFromAmt(poMessage,oClkmsg_tclTimeZone);


         //Added by sos1kor to Extract the Time Zone Parameter from Clock server
         {
            tChar* chTZString = OSAL_NEW tChar[TZ_MAX_STRING_LEN];
            if(chTZString != NULL)
            OSAL_pvMemorySet((tVoid*)chTZString,NULL,TZ_MAX_STRING_LEN);

            OSALUTIL_szSaveStringNCopy(chTZString, oClkmsg_tclTimeZone.szDescription.szValue, TZ_MAX_STRING_LEN);             
            tU16 u16TimeZoneIndex =(tU8) oClkmsg_tclTimeZone.u16Index;

            /**Incase there is an automatic update of the time zone, then trigger the display of popup for user notification**/        
             
            if( NULL != poFI->m_poDataPool )
            {
                tU16 u16PrevTZ;
                poFI->m_poDataPool -> u32dp_get( DPCLOCK__TZ_INDEX, 
                  &u16PrevTZ, 
                  sizeof(u16PrevTZ)    );

                tS16 s16CurrYear;
                poFI->m_poDataPool -> u32dp_get( DPCLOCK__CURR_YEAR, 
                  &s16CurrYear, 
                  sizeof(s16CurrYear)    );

                tU8 u8ClockMode;
                poFI->m_poDataPool -> u32dp_get( DPCLOCK__GPS_TIME_STATUS, 
                  &u8ClockMode, 
                  sizeof(u8ClockMode)    );

                if(((tU16)s16CurrYear != DEFAULT_YEAR) && (u8ClockMode == AUTO))
                {
                
                    //Check if TZ has changed
                    if(u16PrevTZ != u16TimeZoneIndex)
                    {//Automatic update of TZ, trigger system event to display popup
                        poclGUI_EventAdapter()->bGUI_SendSystemEvent(HSA_EVENT__CLOCK_TIME_ZONE_CHANGED);
                    }
                }

                //Update DP with new TZ
                poFI->m_poDataPool -> u32dp_set( DPCLOCK__TZ_INDEX, 
                                                 &u16TimeZoneIndex, 
                                                 sizeof(u16TimeZoneIndex)    );

               tU32 u32Size   = poFI->m_poDataPool->u32dp_getElementSize(DPCLOCK__TZ_DESCRIPTION);
               poFI->m_poDataPool->u32dp_set( DPCLOCK__TZ_DESCRIPTION, chTZString, u32Size );

               if (m_poTrace != NULL )
               {
                  m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
                     TR_CLASS_HSI_CLOCK_MSG,"Value of Index : %d and Description is: %s",u16TimeZoneIndex, chTZString );
               }
            }
            
            OSAL_DELETE[] chTZString;

         }
      }
      break;

   case AMT_C_U8_CCAMSG_OPCODE_ERROR:
      {
         /* property update error dealt separately in common error handler */
         vHandleCCAErrorMessage( poMessage );
         break;
      }

   default:
      {
         /* ERROR trace */
         vGenericTraceDup ( (tU8)TR_LEVEL_HMI_ERROR , CLOCK_TRACE_CCA , 
            HSI_CLOCK_UNKNOWN_OPCODE , u16FID );

         /* send error message back to server for unkown opcode */
         vSendErrorMessage( u16FID , AMT_C_U16_ERROR_INVALID_OPCODE );

         break;
      }      
   }
}

/* property update */
/*************************************************************************
* FUNCTION   : vHandleFID_GET_AVAILABLE_TIMEZONE_TO_VDClock
*
* DESCRIPTION: handle message to Get TIMEZONE values.
*
* PARAMETER  : poMessage 
*
* RETURNVALUE: NONE
*
* History    : InitialVersion
* 
*************************************************************************/
tVoid clCLOCK_CCAhandler::vHandleFID_GET_AVAILABLE_TIMEZONE_TO_VDClock()
{

   midw_ext_clockfi_tclMsgGetAvailableTimeZonesMethodResult oMsgData;    

   /* post the message to Server thro Set Property */
   bSendCCAMessage( MIDW_EXT_CLOCKFI_C_U16_GETAVAILABLETIMEZONES,
      AMT_C_U8_CCAMSG_OPCODE_METHODSTART,
      oMsgData );   
}
/* Set message */
/*************************************************************************
* FUNCTION   : vHandleFID_GPS_WeekEpoch_TO_VDClock
*
* DESCRIPTION: handle message to Set GPS Epoch.
*
* PARAMETER  : poMessage
*
* RETURNVALUE: NONE
*
* History    : InitialVersion
*
*************************************************************************/
tVoid clCLOCK_CCAhandler::vHandleFID_GPS_WeekEpoch_TO_VDClock()
{
	tU8 u8TimeInterval = 0;
	poFI->m_poDataPool -> u32dp_get( DPCLOCK__TIME_INTERVAL,
	                  &u8TimeInterval,
	                  sizeof(u8TimeInterval)	);
		midw_ext_clockfi_tclMsgGPS_WeekEpochSet oMsgData;
		oMsgData.GPSepoch.enType = (midw_ext_fi_tclDCLK_TEN_GPS_EPOCH::tenType)u8TimeInterval;
		bSendCCAMessage( MIDW_EXT_CLOCKFI_C_U16_GPS_WEEKEPOCH,
		AMT_C_U8_CCAMSG_OPCODE_SET,
		oMsgData );
}
/*************************************************************************
* FUNCTION   : vDataupdateReminder
*
* DESCRIPTION: Shows the data update reminder popup
*
* PARAMETER  : system time in seconds
*
* RETURNVALUE: NONE
*
* History    : InitialVersion
* 
*************************************************************************/

tVoid clCLOCK_CCAhandler::vDataupdateReminder(tU32 u32BaseScalarValue)const
{
    tU8 u8state = DUR_STATE0_COUNT_IGN_ON;
    tU32 u32DURPStartDate = 0;        
    
      if( NULL != poFI->m_poDataPool )
        {

        poFI->m_poDataPool->u32dp_get(DPSYSTEM__DUR_STATE, &u8state, sizeof(tU8));

        //get the data update reminder start date.
        poFI->m_poDataPool->u32dp_get( DPSYSTEM__DUR_START_DATE, 
                      &u32DURPStartDate, 
                      sizeof(u32DURPStartDate));

      // fix for MMS 235619. popup will not disable before 100 ignition are over
      // and before start date is set.
      if((u32BaseScalarValue > u32DURPStartDate) && 
         (u32DURPStartDate != 0) &&
         (u8state > DUR_STATE1_SET_START_DATE))
        {
            if ((u32BaseScalarValue - u32DURPStartDate) >= SECONDS_IN_DURP_MAX_PERIOD)
            {
                u8state = DUR_STATE6_POPUP_DISABLED;
                m_poDataPool->u32dp_set(DPSYSTEM__DUR_STATE, &u8state, sizeof(tU8));            
                return;
            }                      
        }
        if (m_poTrace != NULL )
        {
            m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
            TR_CLASS_HSI_CLOCK_MSG,"DURP: State==> %d", u8state);
        } 
      switch(u8state)
      {
      case DUR_STATE0_COUNT_IGN_ON:
          {
        	  if (m_poTrace != NULL )
        	  {
        		  	  m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
                         TR_CLASS_HSI_CLOCK_MSG," nn: %x:%x:%x:%x:%x:%x:",PRODUCT_C_STRING_NAV_BUILDDATE[0],PRODUCT_C_STRING_NAV_BUILDDATE[1],PRODUCT_C_STRING_NAV_BUILDDATE[2],PRODUCT_C_STRING_NAV_BUILDDATE[3],PRODUCT_C_STRING_NAV_BUILDDATE[4],PRODUCT_C_STRING_NAV_BUILDDATE[5]);
        	  }
            //  do nothing here
          }
          break;
      case DUR_STATE1_SET_START_DATE:
          {

        	  if (m_poTrace != NULL )
        	  {
        		  m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
                         TR_CLASS_HSI_CLOCK_MSG,"Inside State DUR_STATE1_SET_START_DATE ");
            // hem2hi: new condition to check scalar value in valid range

        		  m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
                          TR_CLASS_HSI_CLOCK_MSG,"ww: %x:%x:%x:%x:%x:%x:",PRODUCT_C_STRING_NAV_BUILDDATE[0],PRODUCT_C_STRING_NAV_BUILDDATE[1],PRODUCT_C_STRING_NAV_BUILDDATE[2],PRODUCT_C_STRING_NAV_BUILDDATE[3],PRODUCT_C_STRING_NAV_BUILDDATE[4],PRODUCT_C_STRING_NAV_BUILDDATE[5] );
				  m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
                         TR_CLASS_HSI_CLOCK_MSG,"u32BaseScalarValue is %d",u32BaseScalarValue);
				  m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
                         TR_CLASS_HSI_CLOCK_MSG,"date is %u",DATE_IN_SEC_ROUNDED(PRODUCT_C_STRING_NAV_BUILDDATE));
				  m_poTrace->vTrace( TR_LEVEL_HMI_INFO,TR_CLASS_HSI_CLOCK_MSG,"date is %x",DATE_IN_SEC_ROUNDED(PRODUCT_C_STRING_NAV_BUILDDATE));

        	  }

			tU32 u32BuildDate =  DATE_IN_SEC_ROUNDED(PRODUCT_C_STRING_NAV_BUILDDATE);
			if(u32BaseScalarValue > u32BuildDate) {
			//Set the start date; after one year from this date DUR popup will be shown
			if (m_poTrace != NULL )
			{
				m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
                         TR_CLASS_HSI_CLOCK_MSG,"Inside State u32BaseScalarValue > SECONDS_IN_TEN_YEAR ");
			}

			poFI->m_poDataPool->u32dp_set( DPSYSTEM__DUR_BASE_SCALAR_DATETIMEVALUE, &u32BaseScalarValue, sizeof(u32BaseScalarValue));
			if (u32DURPStartDate == 0) 
			{
			
				if (m_poTrace != NULL )
				{
					 m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
                         TR_CLASS_HSI_CLOCK_MSG,"Inside State u32DURPStartDate == 0 ");
				}
			// Set the start date, after 7 years from this date the popup will be disabled.
					if (m_poTrace != NULL )
                    {
                      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
                         TR_CLASS_HSI_CLOCK_MSG,"DPSYSTEM__DUR_START_DATE set with u32BaseScalarValue");
                    }
			m_poDataPool->u32dp_set( DPSYSTEM__DUR_START_DATE, &u32BaseScalarValue, sizeof(u32BaseScalarValue));        
            }

            //Advance to next state: wait for one year        
            u8state = DUR_STATE2_WAIT_YEAR;              
            m_poDataPool->u32dp_set(DPSYSTEM__DUR_STATE, &u8state, sizeof(tU8));
          }
		  else
		  {
			  if (m_poTrace != NULL )
			  {
				  m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
                         TR_CLASS_HSI_CLOCK_MSG,"Outside compare");
			  }
		  }
		  }
          break;
      case DUR_STATE2_WAIT_YEAR:
          {
            tU32 u32OldBaseScalarValue = 0;
            tS32 u32DiffTime = 0;
        
            // get data update reminder start date 
                   poFI->m_poDataPool->u32dp_get( DPSYSTEM__DUR_BASE_SCALAR_DATETIMEVALUE, 
                                                  &u32OldBaseScalarValue, 
                                                  sizeof(u32OldBaseScalarValue));
            
              //check whether year condition satisfies to trigger the DUR popup
              u32DiffTime = u32BaseScalarValue -u32OldBaseScalarValue;
              
              if (u32DiffTime >= SECONDS_IN_ONE_YEAR)
                 {             
                u8state = DUR_STATE4_POPUP_ACTIVE;    
                poFI->m_poDataPool->u32dp_set(DPSYSTEM__DUR_STATE, &u8state, sizeof(tU8));

                //Trigger the Data update reminder popup.
                poclGUI_EventAdapter()->bGUI_SendSystemEvent(HSA_EVENT__SYSTEM_DUR_OPEN); 
                
                }
          }
          break;
      case DUR_STATE3_WAIT_SNOOZE:
          {
            tU32 u32OldBaseScalarValue = 0;
            tS32 u32DiffTime = 0;
            
            // get data update reminder start date 
                   poFI->m_poDataPool->u32dp_get( DPSYSTEM__DUR_BASE_SCALAR_DATETIMEVALUE, 
                                                  &u32OldBaseScalarValue, 
                                                  sizeof(u32OldBaseScalarValue));
            
              //check whether year condition satisfies to trigger the DUR popup
              u32DiffTime = u32BaseScalarValue -u32OldBaseScalarValue;
              
              if (u32DiffTime >=SECONDS_IN_ONE_DAY)
                 {             
                u8state = DUR_STATE4_POPUP_ACTIVE;
                poFI->m_poDataPool->u32dp_set(DPSYSTEM__DUR_STATE, &u8state, sizeof(tU8));
                
                //Trigger the Data update reminder popup.
                poclGUI_EventAdapter()->bGUI_SendSystemEvent(HSA_EVENT__SYSTEM_DUR_OPEN); 
                
                }          
          }      
          break;
      case DUR_STATE4_POPUP_ACTIVE:
          {          
            //Do nothing here
          }          
          break;
      case DUR_STATE5_SHOW_POPUP:
          {          
            u8state = DUR_STATE4_POPUP_ACTIVE;    
            poFI->m_poDataPool->u32dp_set(DPSYSTEM__DUR_STATE, &u8state, sizeof(tU8));
            //Trigger the Data update reminder popup on next startup if popup is ignored by user .
            poclGUI_EventAdapter()->bGUI_SendSystemEvent(HSA_EVENT__SYSTEM_DUR_OPEN);             
          }          
          break;                  
      case DUR_STATE6_POPUP_DISABLED:
          {          
            //Do nothing here
          }          
          break;                    
      default:
          {
                 if (m_poTrace != NULL )
                   {
                      m_poTrace->vTrace( TR_LEVEL_HMI_INFO,
                         TR_CLASS_HSI_CLOCK_MSG,"DUR popup: illegal state");
                   }
          }
          break;
      }
    }
   }

clI_HSA_Event_IF *clCLOCK_CCAhandler::poclGUI_EventAdapter ()const
{
   // encapsulated singleton pattern
   // Pointer to send commands to GUI
   static clI_HSA_Event_IF* s_poclGUI_EventAdapter = NULL;
   if (NULL == s_poclGUI_EventAdapter)
   {
      s_poclGUI_EventAdapter = clHSAEventAdapter::getInstance();
   }
   return s_poclGUI_EventAdapter;
}

