/************************************************************************
* FILE:         clTuner_CCAhandler.h
* PROJECT:      ARION Prototyp
* SW-COMPONENT: 
*----------------------------------------------------------------------
*
* DESCRIPTION: Tuner FI- CCA Handler
*              
*----------------------------------------------------------------------
* COPYRIGHT:    (c) 2004 Robert Bosch GmbH, Hildesheim
* HISTORY:      
* Date      | Author                 | Modification
* 23.05.05  | CM-DI/ESA (RBIN) Ajith | initial version
*
* 14.03.06 | Guru Prasad CM-DI/ESA(RBIN) | Added few member functions
* 20.06.06  | Pavan R     RBIN/EDI1    Tuner support for VR
*************************************************************************/

#ifndef _TUNER_CCAHANDLER_H_
#define _TUNER_CCAHANDLER_H_

//-----------------------------------------------------------------------------
// includes
//-----------------------------------------------------------------------------

// see precompiled.hh		#define GENERICMSGS_S_IMPORT_INTERFACE_GENERIC
// see precompiled.hh		#include "generic_msgs_if.h"              // to use Generic Message intefaces

// see precompiled.hh		#define OSAL_S_IMPORT_INTERFACE_GENERIC
// see precompiled.hh		#include "osal_if.h"

// see precompiled.hh		#define AIL_S_IMPORT_INTERFACE_GENERIC
// see precompiled.hh		#include "ail_if.h"                       // use AIL template 

// see precompiled.hh		#define AHL_S_IMPORT_INTERFACE_GENERIC
// see precompiled.hh		#include "ahl_if.h"                       // use Application Help Library interfaces 

// see precompiled.hh		#define CCA_MAIN_CALLBACK_INTERFACE
// see precompiled.hh		#include "hmicca_pif.h"
#define MIDW_FI_S_IMPORT_INTERFACE_FI_TYPES
#define MIDW_FI_S_IMPORT_INTERFACE_MIDW_TUNERFI_FUNCTIONIDS
//#define FI_DEFAULT_VERSION =0
#include "midw_fi_if.h"

#include "fituner/HSI_TUN_DEFINES.h"

//To include Trace Defines
#include "fituner/HSI_TUNER_TRACE.h"

#include "fituner/clFI_TUNER.h"

#include "fituner/clTun_ListManager.h"

/*To include datapool values*/
// see precompiled.hh		#define HMI_BASE_DATA_POOL_CONFIG_IF
// see precompiled.hh		#include "data_pool_if.h"     // use interface functions of DataPool


/** Include to use the same structure define from clHSI_TestMode.h for TMC List Message */
typedef struct 
{
	tU32 u32Frequency;
	tU32 u32PI;
	tU8  u8AvalableFlags;
	tU8	 u8ECC;
	tU8  u8SID;
	tU8  u8LTN;
	tU8  u8Quality;
}sTMCStation_info;

/* forward declaration of class used to compile this file */
class fi_tclTypeBase;

#include "fituner/clTun_AFListSink.h"


/* interface for hsi-handler (helper-class for 
   registration for porperties) */
// see precompiled.hh		#include "HSI_IF.h"

class clFi_TUNER;         //forward declarations 

class clTun_ListManager;
class clTun_AFListSink;

class clHSI_Radio;
class clHSI_TestMode;
class clHSI_TunerMaster;
//-----------------------------------------------------------------------------
// class definition
//-----------------------------------------------------------------------------

class clTuner_CCAhandler : public clCCAhandlerGeneral 
{
private:
   tU8                  m_u8CurrentPresetButton;
   midw_fi_tcl_b8_Tun_AudioSink  m_b8CurrentAudioSink;

   clHSI_Radio          *m_pHSI_Radio;
   clHSI_TestMode       *m_pHSI_TestModeRadio;
   clHSI_TunerMaster    *m_pHSI_TunerMaster;

   clFi_TUNER           *poFI;            // pointer to FI object 

   /* private member functions for internal usage */


   clTun_AFListSink     *poAFListSink;

   clTun_ListManager    *poAFListManager;




   /** helper methods to generate TTFIS trace */
   tVoid       vTrace(tU16 u16TraceId, hmi_tenTraceLevel enTraceLevel, tU32 u32TraceData = 0) const;
   tVoid       vTrace(hmi_tenTraceLevel eTraceLevel, const tChar *pcBuffer, ...) const;
   tVoid       vTrace(tU16 u16TraceId, hmi_tenTraceLevel enTraceLevel, tU8 u8Len, tU8* pu8TraceData) const;
   tVoid       vTrace(tU16 u16TraceId, hmi_tenTraceLevel enTraceLevel, tU8 u8Len, tS32* ps32TraceData) const;
   tVoid       vTrace(tU16 u16TraceId, hmi_tenTraceLevel enTraceLevel, tU8 InfoByte1, tU8 InfoByte2, tU8 InfoByte3) const;
   tVoid       vTrace(tU16 u16TraceId, hmi_tenTraceLevel enTraceLevel, tU8 InfoByte1, tU8 InfoByte2, tU8 u8Len, tU8* pu8TraceData) const;




   tU8         u8BG_AUD_ACTIVITY ; // to remember the state of BG tun for Audio

   /* member functions to handle data message from server - specific to each FIDs */

   tVoid vHandleFID_G_GET_STATIC_LIST_ELEMENT_From_FCTuner(amt_tclServiceData* poMessage);
      
   tVoid vHandleFID_G_STATION_DATA_VAG_From_FCTuner(amt_tclServiceData* poMessage);

   tVoid vHandleFID_G_ATSTATIONINFO_VAG_From_FCTuner(amt_tclServiceData* poMessage);

   tVoid vHandleFID_G_BAND_PLUS_STATION_INFO_From_FCTuner(amt_tclServiceData* poMessage);
  
   tVoid vHandleFID_G_TUNER_DATA_From_FCTuner(amt_tclServiceData* poMessage);

   tVoid vHandleFID_G_SET_FEATURE_From_FCTuner(amt_tclServiceData* poMessage);
   
   tVoid vHandleFID_G_SET_HICUT_From_FCTuner(amt_tclServiceData* poMessage);
   
   tVoid vHandleFID_G_GET_DATA_TESTMODE_DDADDS_From_FCTuner(amt_tclServiceData* poMessage);
      
   tVoid vHandleFID_G_AFLIST_V2_From_FCTuner(amt_tclServiceData* poMessage);

   tVoid vHandleFID_G_AFLIST_V2_ELEMENT_From_FCTuner(amt_tclServiceData* poMessage);
   
   tVoid vHandleFID_G_OPERATION_From_FCTuner(amt_tclServiceData* poMessage);

   tVoid vHandleFID_G_MAKE_CONFIG_LIST_From_FCTuner(amt_tclServiceData* poMessage );

   tVoid vHandleFID_G_CONFIG_LIST_ELEM_From_FCTuner(amt_tclServiceData* poMessage );

   tVoid vHandleFID_G_MIXEDPRESET_LIST_ELEMENT_From_FCTuner(amt_tclServiceData* poMessage );
   
   tVoid vHandleFID_G_TAINFO_From_FCTuner( amt_tclServiceData* poMessage );
   




   tVoid vHandleFID_G_SET_SHARXLEVEL_From_FCTuner( amt_tclServiceData* poMessage );

   tVoid vHandleFID_G_SET_DDA_From_FCTuner( amt_tclServiceData* poMessage );

   tVoid vHandleFID_G_SET_AF_From_FCTuner( amt_tclServiceData* poMessage );

   tVoid vHandleFID_G_SET_MEASUREMODE_From_FCTuner( amt_tclServiceData* poMessage );

   tVoid vHandleFID_G_SET_RDSreg_From_FCTuner( amt_tclServiceData* poMessage );



   tVoid vHandleFID_G_SET_DDADDS_From_FCTuner( amt_tclServiceData* poMessage );

   tVoid vHandleFID_G_RADIO_TEXT_From_FCTuner( amt_tclServiceData* poMessage );

#ifdef VARIANT_S_FTR_ENABLE_HMI_TUNER_ITG5
   tVoid vHandleFID_G_DemodulationSwitch_From_FCTuner( amt_tclServiceData* poMessage );
   
   tVoid vHandleFID_TUN_G_UPDATELANDSCAPE_PROGRESS_From_FCTuner( amt_tclServiceData* poMessage );
   
    tVoid vHandleFID_G_Get_StaticList_From_FCTuner( amt_tclServiceData* poMessage );
    
	tVoid vHandleStaticListData(amt_tclServiceData* poMessage);

	tVoid vHandleFID_G_Get_PSD_Data_From_FCTuner(amt_tclServiceData* poMessage);

	tVoid vHandle_FID_TUN_S_CONFIGLIST_UPDATE_From_FCTuner(amt_tclServiceData* poMessage);

#endif
   
	tVoid vHandleFID_TUN_G_HD_TEST_MODE_DATA_From_FCTuner( amt_tclServiceData* poMessage);
   //common Methods result\error  Handler
   
   tVoid vHandleALL_FID_MethodResult_Handler_From_FCTuner(amt_tclServiceData* poMessage); 

   //internal supporting handlers 

   tVoid vHandleTestModeDataStatus( amt_tclServiceData* poMessage );

   tVoid vHandleTestModeDataDDADDSStatus( amt_tclServiceData* poMessage );

   tVoid vHandleFID_G_GET_CALIBRATION_To_FCTuner( amt_tclServiceData* poMessage );
   
   tVoid vHandleFID_G_AtSeek_From_FCTuner( amt_tclServiceData* poMessage );

   tVoid vHandleAtStationInfo( amt_tclServiceData* poMessage );

   tVoid vHandleBand_Station_info( amt_tclServiceData* poMessage );

   tVoid vHandleTAInfo( amt_tclServiceData* poMessage );
   


   tVoid vHandleStaticListElement( amt_tclServiceData* poMessage );

   tVoid vHandleFID_G_TMC_INFO_From_FCTuner( amt_tclServiceData* poMessage );

   //separate handler for simulation message post

   tBool bFIDRegUnregForSimulation( tU16 u16FID , tU8 u8Opcode) const;
   


   clHSI_Radio          *pHSI_GetRadio();

   /** Added by rrv2kor for AF List and TMC List */
   clHSI_TestMode		*pclHSI_GetTestMode();

   /** Added by rrv2kor for TMC List Static Update */
   tBool		m_TMCListRequested;
   tU8			m_U8NoOfTMCStations;

   /** Added by rrv2kor for FM Station List Static Update */
   tBool		m_FMStationListRequested;
   tU8			m_U8NoOfFMStations;

   tBool        m_bAutoCmpcmdAtStartUp;
   tU32  m_u32Frequency;

    
   
   tU8   m_u8StatListElemID;
   tU8 m_u8BandFromHMI;

   tBool bcheckpresetlist;

#ifdef VARIANT_S_FTR_ENABLE_HMI_TUNER_ITG5
   //Member variable for storing DemodulationSwitch setting
      tU8 m_u8DemodSwitch;
	  tBool m_bAMStationListRequested;
      tU8 m_u8NoOfAMStations;
#endif

public:

	
   clTuner_CCAhandler (clFi_TUNER * );//clTuner_CCAhandler (); //constructor
   
   virtual ~clTuner_CCAhandler ( ); // destructor

   tU8   u8TunMode;  // To indicate Foreground/background processing
   
   tU8 u8NoTuners;
   
   tU32  m_u32AFFrequency;
   
   tChar          m_pcPSName[PSNAME_MAX_LEN_IN_UTF+1];
   
   tBool bSeekState;
    
   tBool bDirectFreq;
 

   clHSI_TunerMaster    *pHSI_GetTunerMaster();
   /* Member functions to Handle the framing & posting Messages for specific FIDs */  

    tVoid vHandleFID_S_ATFREQUENCY_To_FCTuner  (  tU8      u8Direction,
                                                 tU8      u8Steps,
                                                 tU8      TunerHeader,
                                                 tBool    blAF);

   /** Added by rrv2kor for Testmode */
   tVoid vHandleFID_S_ATFREQUENCY_WITH_TUNERHEADER_To_FCTuner(tU8 u8Direction, tU8 u8TunerHeader);

   /** Added by rrv2kor for Testmode */
   tVoid vHandleFID_S_ATSEEK_WITH_TUNERHEADER_To_FCTuner(tU8, tU8, tU32 );
    
   tVoid vHandleFID_TUN_S_STATIONLIST_EXIT_To_FCTuner();
   
   tVoid vHandleFID_S_SET_STATIC_LIST_To_FCTuner(tU8) ;

   tVoid vHandleFID_S_SCAN_BAND_To_FCTuner(tU8 u8Mode);

   tVoid vHandleFID_S_SET_FEATURE_To_FCTuner(tU32 , tU32);

   tVoid vHandleFID_TUN_GET_HD_TEST_MODE_DATA();

   tVoid vHandleFID_S_SET_HICUT_To_FCTuner(tU8);

   tVoid vHandleFID_S_AFLIST_V2_To_FCTuner(tU8 u8Command ,tU8 u8Data=0 );
 


   tVoid vHandleFID_S_STOP_ANNOUNCEMENT_To_FCTuner(tU8);

   tVoid vHandleFID_S_DIRECT_FREQUENCY_To_FCTuner(tU32);

   tVoid vHandleFID_S_SAVE_STATIC_LIST_ELEM_To_FCTuner(tU8);

   tVoid vHandleFID_S_AUTOSTORE_STATIC_LIST_To_FCTuner(tU8);
   
#ifdef VARIANT_S_FTR_ENABLE_HMI_TUNER_ITG5   
    tVoid vHandleFID_S_SEL_LIST_ELEM_To_FCTuner(tU8 , tU8, tU8);
#else
   tVoid vHandleFID_S_SEL_LIST_ELEM_To_FCTuner(tU8 , tU8);
#endif   
   
   tVoid vHandleFID_S_MAKE_CONFIG_LIST_To_FCTuner(tU8 u8Command ,tU8 u8ListType, tU8 u8Data=0);
   tVoid vHandleFID_S_GET_CONFIG_LIST_To_FCTuner(tU8 u8FirstElement ,tU8 u8NumOfElementsInList=1);



   tVoid vHandleFID_S_SET_PI_To_FCTuner(midw_fi_tcl_e8_Tun_Abs_Or_Rel::tenType,
										midw_fi_tcl_e8_Tun_Rel_Steps::tenType);

   tVoid vHandleFID_S_SET_SHARXLEVEL_To_FCTuner( tU8 );

   tVoid vHandleFID_S_SET_DDADDS_To_FCTuner( tU8 );

   tVoid vHandleFID_S_GET_CALIBRATION_To_FCTuner();

   tVoid vHandleFID_TUN_BAND_CHANGE_To_FCTuner( tU8 );
    
   tVoid vHandleFID_TUN_S_SEL_MIXEDPRESET_LIST_ELEM_To_FCTuner(tU8);

   tVoid vHandleFID_S_AUTOSTORE_MIXEDPRESET_To_FCTuner( midw_fi_tcl_e8_Tun_TunerMixedpresetList::tenType enListtype );

   enum tEN_E8_TUN_DELETEELEMENTS {//copied from alltypes.h (class midw_fi_tcl_e8_Tun_DeleteElements)
      FI_EN_ONE_ELEMENT = 0UL,   // midw_fi_tcl_e8_Tun_DeleteElements::FI_EN_ONE_ELEMENT
      FI_EN_ALL_ELEMENTS = 1UL   // midw_fi_tcl_e8_Tun_DeleteElements::FI_EN_ALL_ELEMENTS
   };

   tVoid          vHandleFID_TUN_S_DELETE_MIXEDPRESET_LIST_ELEM_To_FCTuner (  tEN_E8_TUN_DELETEELEMENTS  enDeleteMode,
                                                                              tU8                        u8ElemID);
   tVoid          vHandleFID_TUN_S_GET_MIXEDPRESET_LIST_To_FCTuner         (  );

   tVoid          vHandleFID_TUN_S_SAVE_MIXEDPRESET_LIST_ELEM_To_FCTuner   (  tU8                        u8ListElementIndex);

   tVoid          vHandleFID_TUN_S_PS_FREEZE_To_FCTuner                    (  tU32                       u32PI,
                                                                              tString                    strPSName);

   tVoid          vStartTimer                                              (  tU32                       u32Timer);
   tVoid          vStopTimer                                               (  tU32                       u32Timer);
   
   tVoid vHandleFID_S_SET_MIXEDPRESET_LIST(tU8 u8Bank);

#ifdef VARIANT_S_FTR_ENABLE_HMI_TUNER_ITG5
   tVoid vHandleFID_TUN_S_SET_DEMODULATION_SWITCH_To_FCTuner( tU8 u8Band, tBool bHDSetting);
   
   tVoid vHandleFID_TUN_S_CONFIGLIST_UPDATE_To_FCTuner( tU8 u8ListID, tBool bAction );

   tVoid vHandleFID_TUN_S_SELECT_HD_AUDIOPRGM_To_FCTuner(tU8 u8SelectMode, tU8 u8HDAudPrgmNum, tBool bDirection);
#endif


   tVoid bSetBandFromHMI( tU8 u8Band);
   tU8 u8GetBandFromHMI();
   

   DECLARE_MSG_MAP(clTuner_CCAhandler)  // AHL Macro , mapping the class 
};

#endif 


