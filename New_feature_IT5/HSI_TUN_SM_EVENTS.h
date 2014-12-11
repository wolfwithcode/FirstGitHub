/************************************************************************
* Generated : 8/30/2005 1:38:47 PM
*
* FILE        :   HSI_TUN_SM_EVENTS.H
*
* SW-COMPONENT:   TUNER HSI 
*
* DESCRIPTION:    Events from HMI Application (state machine) to HSI
*                    First 2 Nibble - HMI Base  
*                    Next  3 Nibble  - Tuner Base     
*                    Next  3 Nibble  - Tuner events
*                    Total 32 Bits
*************************************************************************
* AUTHOR:         CM-DI/ESA - Ajith (RBIN - EDI1) 
*                 CM-DI/ESA - Guru Prasad (RBIN - EDI1)
*
* COPYRIGHT:      (c) 2005 Blaupunkt Werke HISTROY:
*************************************************************************/


#ifndef HSI_TUN_SM_EVENTSH
#define HSI_TUN_SM_EVENTSH

/*************************************/
/*       Defines for Events          */
/*************************************/

/***********HSI Base Event define***************/

#define HSI                0x81000000

/***********Component Base Event define*********/

#define TUN                0x00002000

/***********HSI Event define *******************/

#define HSI__TUN__BANDSEL__FM1            HSI|TUN|0x00000001
#define HSI__TUN__BANDSEL__FM2            HSI|TUN|0x00000002
#define HSI__TUN__BANDSEL__FM3            HSI|TUN|0x00000003
#define HSI__TUN__BANDSEL__AST            HSI|TUN|0x00000004
#define HSI__TUN__BANDSEL__MW             HSI|TUN|0x00000005
#define HSI__TUN__BANDSEL__LW             HSI|TUN|0x00000006


#define HSI__TUN__INIT                    HSI|TUN|0x00000007


#define HSI__TUN__TUN_FORE_GRD            HSI|TUN|0x00000008	//is not used now due to clean-up of unused fi, can be used later


#define HSI__TUN__SEEK_NEXT_SHORT         HSI|TUN|0x00000009
#define HSI__TUN__SEEK_PRV_SHORT          HSI|TUN|0x00000010


#define HSI__TUN__SEEK_NEXT_LONG          HSI|TUN|0x00000011
#define HSI__TUN__SEEK_NEXT_LONG_REL      HSI|TUN|0x00000012
#define HSI__TUN__SEEK_PRV_LONG           HSI|TUN|0x00000013
#define HSI__TUN__SEEK_PRV_LONG_REL       HSI|TUN|0x00000014

#define HSI__TUN__TUN_BACK_GRD            HSI|TUN|0x00000015	//is not used now due to clean-up of unused fi, can be used later

#define HSI__TUN__SEEK_AUTO_NEXT          HSI|TUN|0x00000016
#define HSI__TUN__SEEK_AUTO_PRV           HSI|TUN|0x00000017

#define HSI__TUN__SCAN_BAND               HSI|TUN|0x00000018	//is not used now due to clean-up of unused fi, can be used later
#define HSI__TUN__STOP_SCAN               HSI|TUN|0x00000019

#define HSI__TUN__FEATURE_SHARX_ON        HSI|TUN|0x0000001A
#define HSI__TUN__FEATURE_AF_ON           HSI|TUN|0x0000001B
#define HSI__TUN__FEATURE_FMSTEP_ON       HSI|TUN|0x0000001C
#define HSI__TUN__FEATURE_PSNAME_ON       HSI|TUN|0x0000001D
#define HSI__TUN__FEATURE_TM_DATA_MODE_ON HSI|TUN|0x0000001E
#define HSI__TUN__FEATURE_TM_DATA_MODE_ON_FG  HSI|TUN|0x0000001F
#define HSI__TUN__FEATURE_TM_DATA_MODE_ON_BG  HSI|TUN|0x00000020  
#define HSI__TUN__FEATURE_TA_ON           HSI|TUN|0x00000021
#define HSI__TUN__FEATURE_REG_ON          HSI|TUN|0x00000022
#define HSI__TUN__FEATURE_REG_AUTO_ON     HSI|TUN|0x00000023
#define HSI__TUN__ACTIVATE_BG_TUNER       HSI|TUN|0x00000024
#define HSI__TUN__DEACTIVATE_BG_TUNER     HSI|TUN|0x00000025

/********Added by Pavan R ******/

#define HSI__TUN__FEATURE_TMC_ON          HSI|TUN|0x00000026
#define HSI__TUN__FEATURE_MONO_ON         HSI|TUN|0x00000027
#define HSI__TUN__FEATURE_RADIOTEXT_ON    HSI|TUN|0x00000028
#define HSI__TUN__FEATURE_SEEK_SENSITIVITY_ON HSI|TUN|0x00000029
#define HSI__TUN__FEATURE_DDA_ON          HSI|TUN|0x0000002a


/********************************/

#define HSI__TUN__FEATURE_SHARX_OFF          HSI|TUN|0x00000030
#define HSI__TUN__FEATURE_AF_OFF             HSI|TUN|0x00000031
#define HSI__TUN__FEATURE_FMSTEP_OFF         HSI|TUN|0x00000032
#define HSI__TUN__FEATURE_PSNAME_OFF         HSI|TUN|0x00000033
#define HSI__TUN__FEATURE_TM_DATA_MODE_OFF   HSI|TUN|0x00000034
#define HSI__TUN__FEATURE_TA_OFF             HSI|TUN|0x00000035
#define HSI__TUN__FEATURE_REG_OFF            HSI|TUN|0x00000036
#define HSI__TUN__FEATURE_REG_AUTO_OFF       HSI|TUN|0x00000037


/********Added by Pavan R ******/

#define HSI__TUN__FEATURE_TMC_OFF          HSI|TUN|0x00000038
#define HSI__TUN__FEATURE_MONO_OFF         HSI|TUN|0x00000039
#define HSI__TUN__FEATURE_RADIOTEXT_OFF    HSI|TUN|0x0000003A
#define HSI__TUN__FEATURE_SEEK_SENSITIVITY_OFF HSI|TUN|0x0000003B
#define HSI__TUN__FEATURE_DDA_OFF          HSI|TUN|0x0000003C


#define HSI__TUN__CHGFEATURE_SHARX_LEVEL   HSI|TUN|0x0000004F

/********************************/


#define HSI__TUN__CHGFEATURE_HIGHCUT      HSI|TUN|0x00000050
#define HSI__TUN__STOP_ANNOUNCEMENT_ALL   HSI|TUN|0x00000051
#define HSI__TUN__STOP_ANNOUNCEMENT_CUR   HSI|TUN|0x00000052
#define HSI__TUN__OPERATION_TPSEEK_ON     HSI|TUN|0x00000053
#define HSI__TUN__OPERATION_TPSEEK_OFF    HSI|TUN|0x00000054

#define HSI__TUN__SET_FREQUENCY           HSI|TUN|0x00000055

#define HSI__TUN__SAVE_STATIC_LISTELEMENT HSI|TUN|0x00000056
#define HSI__TUN__AUTOSTRORE_STATIC_LIST  HSI|TUN|0x00000057
#define HSI__TUN__SEL_STATIC_LISTELEMENT  HSI|TUN|0x00000058

         
#define HSI__TUN__LOAD_FM_CONFIG_LIST   HSI|TUN|0x00000059
#define HSI__TUN__NEXT_FM_CONFIG_LIST   HSI|TUN|0x0000005A
#define HSI__TUN__PREV_FM_CONFIG_LIST   HSI|TUN|0x0000005B

#define HSI__TUN__SEL_CONFIG_LISTELEMENT  HSI|TUN|0x0000005C
#define HSI__TUN__LOAD_FMLIST_FROM_INDEX   HSI|TUN|0x0000005D 

#define HSI__TUN__TA_FORE_GRD              HSI|TUN|0x0000005E		//is not used now due to clean-up of unused fi, can be used later
#define HSI__TUN__TA_BACK_GRD              HSI|TUN|0x0000005F		//is not used now due to clean-up of unused fi, can be used later
#define HSI__TUN__PTY31_FORE_GRD              HSI|TUN|0x00000060	//is not used now due to clean-up of unused fi, can be used later	
#define HSI__TUN__PTY31_BACK_GRD              HSI|TUN|0x00000061	//is not used now due to clean-up of unused fi, can be used later

#define HSI__TUN__SET_BGTUN_TO_AUDIO          HSI|TUN|0x00000062
#define HSI__TUN__CLEAR_BGTUN_FROM_AUDIO      HSI|TUN|0x00000063

/** Included by Pavan R **/
#define HSI__TUN__FMSEEK_TEST_READ_DP         HSI|TUN|0x00000064
#define HSI__TUN__FMSEEK_TEST_WRITE_DP        HSI|TUN|0x00000065
#define HSI__TUN__SELECT_PRESET               HSI|TUN|0x00000066
/*************************/



#define HSI__TUN__LOAD_AF_LIST                HSI|TUN|0x00000067
#define HSI__TUN__NEXT_AF_LIST                HSI|TUN|0x00000068
#define HSI__TUN__PREV_AF_LIST                HSI|TUN|0x00000069
#define HSI__TUN__SEL_AF_LISTELEMENT          HSI|TUN|0x00000070
#define HSI__TUN__CLOSE_AF_LIST               HSI|TUN|0x00000071

#define HSI__TUN__LOAD_TMC_LIST              HSI|TUN|0x00000072
#define HSI__TUN__NEXT_TMC_LIST              HSI|TUN|0x00000073
#define HSI__TUN__PREV_TMC_LIST              HSI|TUN|0x00000074
#define HSI__TUN__DETAILED_TMC_LISTELEMENT   HSI|TUN|0x00000075

#define HSI__TUN__BAND_CHANGE_FM             HSI|TUN|0x00000076
#define HSI__TUN__BAND_CHANGE_AM             HSI|TUN|0x00000077
#define HSI__TUN__BAND_CHANGE_DAB            HSI|TUN|0x00000078

#define HSI__TUN__SET_TUNER_MODE_DDSDDA      HSI|TUN|0x00000079

#define HSI__TUN__SEEK_NEXT_FROM_TESTMODE      HSI|TUN|0x0000007A
#define HSI__TUN__SEEK_PREV_FROM_TESTMODE      HSI|TUN|0x0000007B


#define HSI__TUN__ALLOW_TPSEEK_TIMER          (HSI|TUN|0x000000080)
#define HSI__TUN__RADIO_ANTENNA_TEST           (HSI|TUN|0x000000081)
#define HSI__TUN__RADIO_DAB_ANTENNA_TEST           (HSI|TUN|0x000000082)


#define HSI__TUN__MEASURE_MODE_ON				(HSI|TUN|0x000000083)
#define HSI__TUN__MEASURE_MODE_OFF				(HSI|TUN|0x000000084)
#define HSI__TUN__ACTIVATE_TUNER				(HSI|TUN|0x000000085)
#define HSI__TUN__ACTIVATE_BAND_FM				(HSI|TUN|0x000000086)
#define HSI__TUN__ACTIVATE_BAND_AM				(HSI|TUN|0x000000087)
#define HSI__TUN__FEATURE_TM_EXIT_MODE          (HSI|TUN|0x000000088)
#define HSI__TUN__FEATURE_TM_RADIO_ON           (HSI|TUN|0x000000089)
#define HSI__TUN__FEATURE_CALIBRATION_DATA      (HSI|TUN|0x00000008A)
#define HSI__TUN__FEATURE_HD_TEST_MODE_DATA     (HSI|TUN|0x00000008B)

#endif



