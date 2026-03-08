/*******************************************************************************
Copyright (c) 2020 - Analog Devices Inc. All Rights Reserved.
This software is proprietary & confidential to Analog Devices, Inc.
and its licensor.
******************************************************************************
* @file:    adbms_Application.c
* @brief:   adbms application test cases
* @version: $Revision$
* @date:    $Date$
* Developed by: ADIBMS Software team, Bangalore, India
*****************************************************************************/
/*! \addtogroup APPLICATION
*  @{
*/

/*! @addtogroup Application
*  @{
*/
#include "adBms_Application.h"

#include "common.h"

#include "adBms6830CmdList.h"
#include "mcuWrapper.h"
#include "serialPrintResult.h"
#include "adBms6830CmdList.h"
#include "adBms6830GenericType.h"
#include "stddef.h"
#include "math.h"
#ifdef MBED
extern Serial pc;
#endif
/**
*******************************************************************************
* @brief Setup Variables
* The following variables can be modified to configure the software.
*******************************************************************************
*/

static const uint8_t rev4_lut[16] = {
    0x0, /* 0000 -> 0000 */
    0x8, /* 0001 -> 1000 */
    0x4, /* 0010 -> 0100 */
    0xC, /* 0011 -> 1100 */
    0x2, /* 0100 -> 0010 */
    0xA, /* 0101 -> 1010 */
    0x6, /* 0110 -> 0110 */
    0xE, /* 0111 -> 1110 */
    0x1, /* 1000 -> 0001 */
    0x9, /* 1001 -> 1001 */
    0x5, /* 1010 -> 0101 */
    0xD, /* 1011 -> 1101 */
    0x3, /* 1100 -> 0011 */
    0xB, /* 1101 -> 1011 */
    0x7, /* 1110 -> 0111 */
    0xF  /* 1111 -> 1111 */
};

#define TOTAL_IC 1
cell_asic IC[TOTAL_IC];
float tV;
SEG_PARAMS *seg_paramst_list[] = {
		&SEG1,
		&SEG2,
		&SEG3,
		&SEG4,
		&SEG5
};

/* ADC Command Configurations */
RD      REDUNDANT_MEASUREMENT           = RD_OFF;
CH      AUX_CH_TO_CONVERT               = GPIO10;
CONT    CONTINUOUS_MEASUREMENT          = SINGLE; //changed from single
OW_C_S  CELL_OPEN_WIRE_DETECTION        = OW_OFF_ALL_CH; // changed from off
OW_AUX  AUX_OPEN_WIRE_DETECTION         = AUX_OW_OFF;
PUP     OPEN_WIRE_CURRENT_SOURCE        = PUP_UP;
DCP     DISCHARGE_PERMITTED             = DCP_OFF;
RSTF    RESET_FILTER                    = RSTF_OFF;
ERR     INJECT_ERR_SPI_READ             = WITHOUT_ERR;

/* Set Under Voltage and Over Voltage Thresholds */
const float OV_THRESHOLD = 4.2;                 /* Volt */
const float UV_THRESHOLD = 2.6;                 /* Volt */
const int OWC_Threshold = 2000;                 /* Cell Open wire threshold(mili volt) */
const int OWA_Threshold = 50000;                /* Aux Open wire threshold(mili volt) */
const uint32_t LOOP_MEASUREMENT_COUNT = 1;      /* Loop measurment count */
const uint16_t MEASUREMENT_LOOP_TIME  = 10;     /* milliseconds(mS)*/
uint32_t loop_count = 0;
uint32_t pladc_count;
uint8_t val;
static const float DCC_HYSTERESIS_V = 0.010f;
/*Loop Measurement Setup These Variables are ENABLED or DISABLED Remember ALL CAPS*/
LOOP_MEASURMENT MEASURE_CELL            = ENABLED;        /*   This is ENABLED or DISABLED       */
LOOP_MEASURMENT MEASURE_AVG_CELL        = ENABLED;        /*   This is ENABLED or DISABLED       */
LOOP_MEASURMENT MEASURE_F_CELL          = ENABLED;        /*   This is ENABLED or DISABLED       */
LOOP_MEASURMENT MEASURE_S_VOLTAGE       = ENABLED;        /*   This is ENABLED or DISABLED       */
LOOP_MEASURMENT MEASURE_AUX             = DISABLED;        /*   This is ENABLED or DISABLED       */
LOOP_MEASURMENT MEASURE_RAUX            = DISABLED;        /*   This is ENABLED or DISABLED       */
LOOP_MEASURMENT MEASURE_STAT            = DISABLED;        /*   This is ENABLED or DISABLED       */

void adbms_main()
{
  printMenu();
  adBms6830_init_config(TOTAL_IC, &IC[0]);
  while(1)
  {
    int user_command;
//#ifdef MBED
//    pc.scanf("%d", &user_command);
//    pc.printf("Enter cmd:%d\n", user_command);
//#else
    scanf("%d", &user_command);
    printf("Enter cmd:%d\n", user_command);
//#endif
    run_command(user_command);
  }
}

void run_command(int cmd)
{
  switch(cmd)
  {

  case 1:
    adBms6830_write_read_config(TOTAL_IC, &IC[0]);
    break;

  case 2:
    adBms6830_read_config(TOTAL_IC, &IC[0]);
    break;

  case 3:
    adBms6830_start_adc_cell_voltage_measurment(TOTAL_IC);
    break;

  case 4:
    adBms6830_read_cell_voltages(TOTAL_IC, &IC[0]);
    break;

  case 5:
    adBms6830_start_adc_s_voltage_measurment(TOTAL_IC);
    break;

  case 6:
    adBms6830_read_s_voltages(TOTAL_IC, &IC[0]);
    break;

  case 7:
    adBms6830_start_avgcell_voltage_measurment(TOTAL_IC);
    break;

  case 8:
    adBms6830_read_avgcell_voltages(TOTAL_IC, &IC[0]);
    break;

  case 9:
    adBms6830_start_fcell_voltage_measurment(TOTAL_IC);
    break;

  case 10:
    adBms6830_read_fcell_voltages(TOTAL_IC, &IC[0]);
    break;

  case 11:
    adBms6830_start_aux_voltage_measurment(TOTAL_IC, &IC[0]);
    break;

  case 12:
    adBms6830_read_aux_voltages(TOTAL_IC, &IC[0]);
    break;

  case 13:
    adBms6830_start_raux_voltage_measurment(TOTAL_IC, &IC[0]);
    break;

  case 14:
    adBms6830_read_raux_voltages(TOTAL_IC, &IC[0]);
    break;

  case 15:
    adBms6830_read_status_registers(TOTAL_IC, &IC[0]);
    break;

  case 16:
    loop_count = 0;
    adBmsWakeupIc(TOTAL_IC);
    adBmsWriteData(TOTAL_IC, &IC[0], WRCFGA, Config, A);
    adBmsWriteData(TOTAL_IC, &IC[0], WRCFGB, Config, B);
    adBmsWakeupIc(TOTAL_IC);
    adBms6830_Adcv(REDUNDANT_MEASUREMENT, CONTINUOUS, DISCHARGE_PERMITTED, RESET_FILTER, CELL_OPEN_WIRE_DETECTION);
    Delay_ms(1); // ADCs are updated at their conversion rate is 1ms
    adBms6830_Adcv(RD_ON, CONTINUOUS, DISCHARGE_PERMITTED, RESET_FILTER, CELL_OPEN_WIRE_DETECTION);
    Delay_ms(1); // ADCs are updated at their conversion rate is 1ms
    adBms6830_Adsv(CONTINUOUS, DISCHARGE_PERMITTED, CELL_OPEN_WIRE_DETECTION);
    Delay_ms(8); // ADCs are updated at their conversion rate is 8ms
    while(loop_count < LOOP_MEASUREMENT_COUNT)
    {
      measurement_loop();
      Delay_ms(MEASUREMENT_LOOP_TIME);
      loop_count = loop_count + 1;
    }
    printMenu();
    break;

  case 17:
    adBms6830_clear_cell_measurement(TOTAL_IC);
    break;

  case 18:
    adBms6830_clear_aux_measurement(TOTAL_IC);
    break;

  case 19:
    adBms6830_clear_spin_measurement(TOTAL_IC);
    break;

  case 20:
    adBms6830_clear_fcell_measurement(TOTAL_IC);
    break;

  case 21:
    adBms6830_write_config(TOTAL_IC, &IC[0]);
    break;

  case 22:
  	  // printf("Enter 4 bit val in integer:");
  	  // scanf("%u", &val);
  	  // adBms6830_setgpo_69(TOTAL_IC, &IC[0], val);

  case 0:
    printMenu();
    break;

  default:
#ifdef MBED
    pc.printf("Incorrect Option\n\n");
#else
    printf("Incorrect Option\n\n");
#endif
    break;
  }
}
//??????????????///

void adbms_init_loop(void)
{
	adBms6830_init_config(TOTAL_IC, &IC[0]);
}

void adbms_readCell_loop(void)
{
	adBms6830_read_cell_voltages(TOTAL_IC, &IC[0]);
}

void adbms_startCellConv_loop(void)
{
	adBms6830_Adcv(REDUNDANT_MEASUREMENT, CONTINUOUS, DISCHARGE_PERMITTED, RESET_FILTER, CELL_OPEN_WIRE_DETECTION);
}

void adbms_readTempToggle(void)
{
	adBms6830_setgpo_69(TOTAL_IC, &IC[0]);
}

void testOpenWire(void)
{
	printOpenWireTestResult(TOTAL_IC, &IC[0],Cell);
	HAL_Delay(1000);
	printOpenWireTestResult(TOTAL_IC, &IC[0],S_volt);
	HAL_Delay(1000);
	printOpenWireTestResult(TOTAL_IC, &IC[0],Aux);
	HAL_Delay(1000);
}

void adbmsReinit(uint8_t tIC, cell_asic *ic)
{
	for(uint8_t cic = 0; cic < tIC; cic++)
	{
		if ((ic[cic].cccrc.cmd_cntr == 0) ||
     (ic[cic].cccrc.cmd_cntr == 0x3F) ||
      (ic[cic].cccrc.stat_pec == 1) ||
        (ic[cic].cccrc.cfgr_pec  != 0U) ||
        (ic[cic].cccrc.cell_pec  != 0U) ||
        (ic[cic].cccrc.acell_pec != 0U) ||
        (ic[cic].cccrc.scell_pec != 0U) ||
        (ic[cic].cccrc.fcell_pec != 0U) ||
        (ic[cic].cccrc.aux_pec   != 0U) ||
        (ic[cic].cccrc.raux_pec  != 0U) ||
        (ic[cic].cccrc.stat_pec  != 0U) ||
        (ic[cic].cccrc.comm_pec  != 0U) ||
        (ic[cic].cccrc.pwm_pec   != 0U) ||
        (ic[cic].cccrc.sid_pec   != 0U))
		{
			//ic[cic].tx_cfga.refon = PWR_UP;
			//adBmsWakeupIc(tIC);
			//adBmsWriteData(tIC, &ic[0], WRCFGA, Config, A);
			adbms_init_loop();
//			adbms_readCell_loop();
		}
	}

}

void adbmsReinitMain(void)
{
	adbmsReinit(TOTAL_IC, &IC[0]);
}
//?????????????///
/**
*******************************************************************************
* @brief Set configuration register A. Refer to the data sheet
*        Set configuration register B. Refer to the data sheet
*******************************************************************************
*/
void adBms6830_init_config(uint8_t tIC, cell_asic *ic)
{
  for(uint8_t cic = 0; cic < tIC; cic++)
  {
    /* Init config A */
    ic[cic].tx_cfga.refon = PWR_UP;
//    ic[cic].cfga.cth = CVT_8_1mV;
//    ic[cic].cfga.flag_d = ConfigA_Flag(FLAG_D0, FLAG_SET) | ConfigA_Flag(FLAG_D1, FLAG_SET);
    //ic[cic].tx_cfga.gpo = ConfigA_Gpo(GPO2, GPO_SET) | ConfigA_Gpo(GPO10, GPO_SET);
    ic[cic].tx_cfga.gpo = 0X000; /* All GPIO pull down off */
    //ic[cic].tx_cfga.soakon = SOAKON_CLR;
    ic[cic].tx_cfga.fc = IIR_FPA256;

    /* Init config B */
//    ic[cic].cfgb.dtmen = DTMEN_ON;
    ic[cic].tx_cfgb.vov = SetOverVoltageThreshold(OV_THRESHOLD);
    ic[cic].tx_cfgb.vuv = SetUnderVoltageThreshold(UV_THRESHOLD);
    ic[cic].tx_cfgb.dtmen=0;
    ic[cic].tx_cfgb.dcc = 0x0FFF;
    //ic[cic].tx_cfgb.dtrng=0X0;
    ic[cic].tx_cfgb.dcto=1;
    ic[cic].tx_cfgb.dtrng = 0;
    

    SetConfigB_DischargeTimeOutValue(tIC, &ic[cic], RANG_0_TO_63_MIN, TIME_1MIN_OR_0_26HR);
  }
  adBmsWakeupIc(tIC);
  adBmsWriteData(tIC, &ic[0], WRCFGA, Config, A);
  adBmsWriteData(tIC, &ic[0], WRCFGB, Config, B);
  adBms6830_Adcv(REDUNDANT_MEASUREMENT, CONTINUOUS, DISCHARGE_PERMITTED, RESET_FILTER, CELL_OPEN_WIRE_DETECTION);
  HAL_Delay(1);
  //adBmsWakeupIcTim(12);
}

/**
*******************************************************************************
* @brief Write and Read Configuration Register A/B
*******************************************************************************
*/
void adBms6830_write_read_config(uint8_t tIC, cell_asic *ic)
{
  adBmsWakeupIc(tIC);
  adBmsWriteData(tIC, &ic[0], WRCFGA, Config, A);
  adBmsWriteData(tIC, &ic[0], WRCFGB, Config, B);
  adBmsReadData(tIC, &ic[0], RDCFGA, Config, A);
  adBmsReadData(tIC, &ic[0], RDCFGB, Config, B);
  printWriteConfig(tIC, &ic[0], Config, ALL_GRP);
  printReadConfig(tIC, &ic[0], Config, ALL_GRP);
}

/**
*******************************************************************************
* @brief Write Configuration Register A/B
*******************************************************************************
*/
void adBms6830_write_config(uint8_t tIC, cell_asic *ic)
{
  adBmsWakeupIc(tIC);
  adBmsWriteData(tIC, &ic[0], WRCFGA, Config, A);
  adBmsWriteData(tIC, &ic[0], WRCFGB, Config, B);
  printWriteConfig(tIC, &ic[0], Config, ALL_GRP);
}

/**
*******************************************************************************
* @brief Read Configuration Register A/B
*******************************************************************************
*/
void adBms6830_read_config(uint8_t tIC, cell_asic *ic)
{
  adBmsWakeupIc(tIC);
  adBmsReadData(tIC, &ic[0], RDCFGA, Config, A);
  adBmsReadData(tIC, &ic[0], RDCFGB, Config, B);
  printReadConfig(tIC, &ic[0], Config, ALL_GRP);
}

/**
*******************************************************************************
* @brief Start ADC Cell Voltage Measurement
*******************************************************************************
*/
void adBms6830_start_adc_cell_voltage_measurment(uint8_t tIC)
{
  adBmsWakeupIc(tIC);
  adBms6830_Adcv(REDUNDANT_MEASUREMENT, CONTINUOUS_MEASUREMENT, DISCHARGE_PERMITTED, RESET_FILTER, CELL_OPEN_WIRE_DETECTION);
  pladc_count = adBmsPollAdc(PLADC);
#ifdef MBED
  pc.printf("Cell conversion completed\n");
#else
  printf("Cell conversion completed\n");
#endif
  printPollAdcConvTime(pladc_count);
}

static void adBms6830_update_dcc_from_cell_average(uint8_t tIC, cell_asic *ic)
{
  for (uint8_t cic = 0U; cic < tIC; cic++)
  {
    float sumCellV = 0.0f;

    for (uint8_t cellIndex = 0U; cellIndex < 12U; cellIndex++)
    {
      sumCellV += getVoltage(ic[cic].cell.c_codes[cellIndex]);
    }

    float avgSegCellV = sumCellV / 12.0f;
    uint16_t dccMask = ic[cic].tx_cfgb.dcc;

    for (uint8_t cellIndex = 0U; cellIndex < 12U; cellIndex++)
    {
      float cellV = getVoltage(ic[cic].cell.c_codes[cellIndex]);
      dccMask = adBms6830_update_cell_over_avg_bit_hysteresis(cellV,
                                                               avgSegCellV,
                                                               cellIndex,
                                                               dccMask,
                                                               DCC_HYSTERESIS_V);
    }

    ic[cic].tx_cfgb.dcc = dccMask;
  }
}

/**
*******************************************************************************
* @brief Read Cell Voltages
*******************************************************************************
*/
void adBms6830_read_cell_voltages(uint8_t tIC, cell_asic *ic)
{
  adBmsWakeupIc(tIC);
  adBmsReadData(tIC, &ic[0], RDCVA, Cell, A);
  adBmsReadData(tIC, &ic[0], RDCVB, Cell, B);
  adBmsReadData(tIC, &ic[0], RDCVC, Cell, C);
  adBmsReadData(tIC, &ic[0], RDCVD, Cell, D);
  adBmsReadData(tIC, &ic[0], RDCVE, Cell, E);
  adBmsReadData(tIC, &ic[0], RDCVF, Cell, F);

  // adBms6830_update_dcc_from_cell_average(tIC, &ic[0]);
  adBmsWakeupIc(tIC);
  adBmsWriteData(tIC, &ic[0], WRCFGB, Config, B);

  printVoltages(tIC, &ic[0], Cell);
}

/**
*******************************************************************************
* @brief Start ADC S-Voltage Measurement
*******************************************************************************
*/
void adBms6830_start_adc_s_voltage_measurment(uint8_t tIC)
{
  adBmsWakeupIc(tIC);
  adBms6830_Adsv(CONTINUOUS_MEASUREMENT, DISCHARGE_PERMITTED, CELL_OPEN_WIRE_DETECTION);
  pladc_count = adBmsPollAdc(PLADC);
#ifdef MBED
  pc.printf("S-Voltage conversion completed\n");
#else
  printf("S-Voltage conversion completed\n");
#endif
  printPollAdcConvTime(pladc_count);
}

/**
*******************************************************************************
* @brief Read S-Voltages
*******************************************************************************
*/
void adBms6830_read_s_voltages(uint8_t tIC, cell_asic *ic)
{
  adBmsWakeupIc(tIC);
  adBmsReadData(tIC, &ic[0], RDSVA, S_volt, A);
  adBmsReadData(tIC, &ic[0], RDSVB, S_volt, B);
  adBmsReadData(tIC, &ic[0], RDSVC, S_volt, C);
  adBmsReadData(tIC, &ic[0], RDSVD, S_volt, D);
  adBmsReadData(tIC, &ic[0], RDSVE, S_volt, E);
  adBmsReadData(tIC, &ic[0], RDSVF, S_volt, F);
  printVoltages(tIC, &ic[0], S_volt);
}

/**
*******************************************************************************
* @brief Start Avarage Cell Voltage Measurement
*******************************************************************************
*/
void adBms6830_start_avgcell_voltage_measurment(uint8_t tIC)
{
  adBmsWakeupIc(tIC);
  adBms6830_Adcv(RD_ON, CONTINUOUS_MEASUREMENT, DISCHARGE_PERMITTED, RESET_FILTER, CELL_OPEN_WIRE_DETECTION);
  pladc_count = adBmsPollAdc(PLADC);
#ifdef MBED
  pc.printf("Avg Cell voltage conversion completed\n");
#else
  printf("Avg Cell voltage conversion completed\n");
#endif
  printPollAdcConvTime(pladc_count);
}

/**
*******************************************************************************
* @brief Read Avarage Cell Voltages
*******************************************************************************
*/
void adBms6830_read_avgcell_voltages(uint8_t tIC, cell_asic *ic)
{
  adBmsWakeupIc(tIC);
  adBmsReadData(tIC, &ic[0], RDACA, AvgCell, A);
  adBmsReadData(tIC, &ic[0], RDACB, AvgCell, B);
  adBmsReadData(tIC, &ic[0], RDACC, AvgCell, C);
  adBmsReadData(tIC, &ic[0], RDACD, AvgCell, D);
  adBmsReadData(tIC, &ic[0], RDACE, AvgCell, E);
  adBmsReadData(tIC, &ic[0], RDACF, AvgCell, F);
  printVoltages(tIC, &ic[0], AvgCell);
}

/**
*******************************************************************************
* @brief Start Filtered Cell Voltages Measurement
*******************************************************************************
*/
void adBms6830_start_fcell_voltage_measurment(uint8_t tIC)
{
  adBmsWakeupIc(tIC);
  adBms6830_Adcv(REDUNDANT_MEASUREMENT, CONTINUOUS_MEASUREMENT, DISCHARGE_PERMITTED, RESET_FILTER, CELL_OPEN_WIRE_DETECTION);
  pladc_count = adBmsPollAdc(PLADC);
#ifdef MBED
  pc.printf("F Cell voltage conversion completed\n");
#else
  printf("F Cell voltage conversion completed\n");
#endif
  printPollAdcConvTime(pladc_count);
}

/**
*******************************************************************************
* @brief Read Filtered Cell Voltages
*******************************************************************************
*/
void adBms6830_read_fcell_voltages(uint8_t tIC, cell_asic *ic)
{
  adBmsWakeupIc(tIC);
  adBmsReadData(tIC, &ic[0], RDFCA, F_volt, A);
  adBmsReadData(tIC, &ic[0], RDFCB, F_volt, B);
  adBmsReadData(tIC, &ic[0], RDFCC, F_volt, C);
  adBmsReadData(tIC, &ic[0], RDFCD, F_volt, D);
  adBmsReadData(tIC, &ic[0], RDFCE, F_volt, E);
  adBmsReadData(tIC, &ic[0], RDFCF, F_volt, F);
  printVoltages(tIC, &ic[0], F_volt);
}

/**
*******************************************************************************
* @brief Start AUX, VMV, V+ Voltages Measurement
*******************************************************************************
*/
void adBms6830_start_aux_voltage_measurment(uint8_t tIC, cell_asic *ic)
{
  for(uint8_t cic = 0; cic < tIC; cic++)
  {
    /* Init config A */
    ic[cic].tx_cfga.refon = PWR_UP;
    ic[cic].tx_cfga.gpo = 0X3FF; /* All GPIO pull down off */
  }
  adBmsWakeupIc(tIC);
  adBmsWriteData(tIC, &ic[0], WRCFGA, Config, A);
  adBms6830_Adax(AUX_OPEN_WIRE_DETECTION, OPEN_WIRE_CURRENT_SOURCE, AUX_CH_TO_CONVERT);
  pladc_count = adBmsPollAdc(PLADC);
#ifdef MBED
  pc.printf("Aux voltage conversion completed\n");
#else
  printf("Aux voltage conversion completed\n");
#endif
  printPollAdcConvTime(pladc_count);
}

/**
*******************************************************************************
* @brief Read AUX, VMV, V+ Voltages
*******************************************************************************
*/
void adBms6830_read_aux_voltages(uint8_t tIC, cell_asic *ic)
{
  adBmsWakeupIc(tIC);
  adBmsReadData(tIC, &ic[0], RDAUXA, Aux, A);
  adBmsReadData(tIC, &ic[0], RDAUXB, Aux, B);
  adBmsReadData(tIC, &ic[0], RDAUXC, Aux, C);
  adBmsReadData(tIC, &ic[0], RDAUXD, Aux, D);
  printVoltages(tIC, &ic[0], Aux);
}

/**
*******************************************************************************
* @brief Start Redundant GPIO Voltages Measurement
*******************************************************************************
*/
void adBms6830_start_raux_voltage_measurment(uint8_t tIC,  cell_asic *ic)
{
  for(uint8_t cic = 0; cic < tIC; cic++)
  {
    /* Init config A */
    ic[cic].tx_cfga.refon = PWR_UP;
    ic[cic].tx_cfga.gpo = 0X3FF; /* All GPIO pull down off */
  }
  adBmsWakeupIc(tIC);
  adBmsWriteData(tIC, &ic[0], WRCFGA, Config, A);
  adBms6830_Adax2(AUX_CH_TO_CONVERT);
  pladc_count = adBmsPollAdc(PLADC);
#ifdef MBED
  pc.printf("RAux voltage conversion completed\n");
#else
  printf("RAux voltage conversion completed\n");
#endif
  printPollAdcConvTime(pladc_count);
}

/**
*******************************************************************************
* @brief Read Redundant GPIO Voltages
*******************************************************************************
*/
void adBms6830_read_raux_voltages(uint8_t tIC, cell_asic *ic)
{
  adBmsWakeupIc(tIC);
  adBmsReadData(tIC, &ic[0], RDRAXA, RAux, A);
  adBmsReadData(tIC, &ic[0], RDRAXB, RAux, B);
  adBmsReadData(tIC, &ic[0], RDRAXC, RAux, C);
  adBmsReadData(tIC, &ic[0], RDRAXD, RAux, D);
  printVoltages(tIC, &ic[0], RAux);
}

/**
*******************************************************************************
* @brief Read Status Reg. A, B, C, D and E.
*******************************************************************************
*/
void adBms6830_read_status_registers(uint8_t tIC, cell_asic *ic)
{
  adBmsWakeupIc(tIC);
  adBmsWriteData(tIC, &ic[0], WRCFGA, Config, A);
  adBmsWriteData(tIC, &ic[0], WRCFGB, Config, B);
  adBms6830_Adax(AUX_OPEN_WIRE_DETECTION, OPEN_WIRE_CURRENT_SOURCE, AUX_CH_TO_CONVERT);
  pladc_count = adBmsPollAdc(PLADC);
  adBms6830_Adcv(REDUNDANT_MEASUREMENT, CONTINUOUS_MEASUREMENT, DISCHARGE_PERMITTED, RESET_FILTER, CELL_OPEN_WIRE_DETECTION);
  pladc_count = pladc_count + adBmsPollAdc(PLADC);

  adBmsReadData(tIC, &ic[0], RDSTATA, Status, A);
  adBmsReadData(tIC, &ic[0], RDSTATB, Status, B);
  adBmsReadData(tIC, &ic[0], RDSTATC, Status, C);
  adBmsReadData(tIC, &ic[0], RDSTATD, Status, D);
  adBmsReadData(tIC, &ic[0], RDSTATE, Status, E);
  printPollAdcConvTime(pladc_count);
  printStatus(tIC, &ic[0], Status, ALL_GRP);
}

/**
*******************************************************************************
* @brief Loop measurment.
*******************************************************************************
*/

/**
*******************************************************************************
* @brief Clear Cell measurement reg.
*******************************************************************************
*/
void adBms6830_clear_cell_measurement(uint8_t tIC)
{
  adBmsWakeupIc(tIC);
  spiSendCmd(CLRCELL);
#ifdef MBED
  pc.printf("Cell Registers Cleared\n\n");
#else
  printf("Cell Registers Cleared\n\n");
#endif
}

/**
*******************************************************************************
* @brief Clear Aux measurement reg.
*******************************************************************************
*/
void adBms6830_clear_aux_measurement(uint8_t tIC)
{
  adBmsWakeupIc(tIC);
  spiSendCmd(CLRAUX);
#ifdef MBED
  pc.printf("Aux Registers Cleared\n\n");
#else
  printf("Aux Registers Cleared\n\n");
#endif
}

/**
*******************************************************************************
* @brief Clear spin measurement reg.
*******************************************************************************
*/
void adBms6830_clear_spin_measurement(uint8_t tIC)
{
  adBmsWakeupIc(tIC);
  spiSendCmd(CLRSPIN);
#ifdef MBED
  pc.printf("Spin Registers Cleared\n\n");
#else
  printf("Spin Registers Cleared\n\n");
#endif
}

/**
*******************************************************************************
* @brief Clear fcell measurement reg.
*******************************************************************************
*/
void adBms6830_clear_fcell_measurement(uint8_t tIC)
{
  adBmsWakeupIc(tIC);
  spiSendCmd(CLRFC);
#ifdef MBED
  pc.printf("Fcell Registers Cleared\n\n");
#else
  printf("Fcell Registers Cleared\n\n");
#endif
}

/** @}*/
/** @}*/

void adBms6830_setgpo_69(uint8_t tIC, cell_asic *ic) {

	//adBmsWakeupIc(tIC);
	//int16_t tempVoltage;
	float icTV;
	int val;

	for (int i = 0; i < tIC; i++) {
		 val=0;
		 SEG_PARAMS *st = seg_paramst_list[i];
	    for (uint8_t value = 0x0; value <= 0xB; value++) {
					// ic[i].tx_cfga.refon = PWR_UP;
					ic[i].tx_cfga.gpo = (((reverse4(value) << 5) & 0x3FF) | 0x21F) ;
//					HAL_Delay(100);
					adBmsWakeupIc(tIC);
					adBmsWriteData(tIC, &ic[0], WRCFGA, Config, A);
//					HAL_Delay(200);
					adBmsWakeupIc(tIC);
					adBms6830_Adax(AUX_OPEN_WIRE_DETECTION, OPEN_WIRE_CURRENT_SOURCE, GPIO10);
					HAL_Delay(8);
//					HAL_Delay(500);
					adBmsWakeupIc(tIC);
					adBmsReadData(tIC, &ic[0], RDAUXD, Aux, D);
//					adBmsReadData(tIC, &ic[0], RDSTATA, Status, A);
					tV = getVoltage(ic[i].aux.a_codes[9]);

					st->CELL_T[val]= get_temp(tV);
					val++;
//					HAL_Delay(50);
					//IC temp
					adBmsWakeupIc(tIC);
//					adBmsWriteData(tIC, &ic[0], WRCFGA, Config, A);
//					adBmsWriteData(tIC, &ic[0], WRCFGB, Config, B);
					adBms6830_Adax(AUX_OPEN_WIRE_DETECTION, OPEN_WIRE_CURRENT_SOURCE, 0);
					adBmsReadData(tIC, &ic[0], RDSTATA, Status, A);
					icTV = getVoltage(ic[i].stata.itmp);
					st->IC_T = (icTV/0.0075)-273 ;

					}


	}

}

float v2Temp(float x)
{
	return -2.13555866e-11 * pow(x,5)
    -2.023286867e-9 * pow(x,4)
    +1.354951937e-6 * pow(x,3)
    -6.256450703e-5 * pow(x,2)
    -0.0113354782 * x
    +2.164523065;
}

static inline uint8_t reverse4(uint8_t x)
{
    return rev4_lut[x & 0x0F];
}


float get_temp_from_voltage(float voltage) {
    float coeffs[7];
    float offset;

    // Note: Voltage decreases as Temperature increases.
    // Range is approx 2.45V (-40C) down to 1.32V (120C)

    if (voltage > 2.3299f) {
        // Segment 0: -40C to -15C approx
        offset = 2.448116f;
        coeffs[0] = -40.0f;
        coeffs[1] = -277.02597f;
        coeffs[2] = -2158.7002f;
        coeffs[3] = -57401.977f;
        coeffs[4] = -336871.0f;
        coeffs[5] = 4091673.8f;
        coeffs[6] = 32178934.0f;
    } else if (voltage > 2.0638f) {
        // Segment 1: -15C to 10C approx
        offset = 2.329855f;
        coeffs[0] = -15.0f;
        coeffs[1] = -35.84161f;
        coeffs[2] = 2084.3179f;
        coeffs[3] = 16646.664f;
        coeffs[4] = 19478.951f;
        coeffs[5] = -196392.22f;
        coeffs[6] = -501810.7f;
    } else if (voltage > 1.6991f) {
        // Segment 2: 10C to 40C approx
        offset = 2.063768f;
        coeffs[0] = 10.0f;
        coeffs[1] = -125.60757f;
        coeffs[2] = -1229.5671f;
        coeffs[3] = -11787.77f;
        coeffs[4] = -52521.227f;
        coeffs[5] = -111740.266f;
        coeffs[6] = -91734.23f;
    } else if (voltage > 1.5020f) {
        // Segment 3: 40C to 65C approx
        offset = 1.699130f;
        coeffs[0] = 40.0f;
        coeffs[1] = -78.087166f;
        coeffs[2] = 548.28186f;
        coeffs[3] = 1052.2971f;
        coeffs[4] = -9148.8f;
        coeffs[5] = 7719.947f;
        coeffs[6] = 212690.75f;
    } else if (voltage > 1.3936f) {
        // Segment 4: 65C to 90C approx
        offset = 1.502029f;
        coeffs[0] = 65.0f;
        coeffs[1] = 31.022549f;
        coeffs[2] = 10350.445f;
        coeffs[3] = 122236.35f;
        coeffs[4] = -205808.05f;
        coeffs[5] = -9048350.0f;
        coeffs[6] = -27475272.0f;
    } else {
        // Segment 5: 90C to 120C approx
        offset = 1.393623f;
        coeffs[0] = 90.0f;
        coeffs[1] = 300.7878f;
        coeffs[2] = 52135.133f;
        coeffs[3] = 1725735.5f;
        coeffs[4] = 31025744.0f;
        coeffs[5] = 288140380.0f;
        coeffs[6] = 1082880500.0f;
    }

    // Centered Input (Input Voltage - Offset)
    float v_prime = voltage - offset;

    // Horner's Method
    float temp = coeffs[6];
    for (int i = 5; i >= 0; i--) {
        temp = temp * v_prime + coeffs[i];
    }

    return temp;
}

  uint16_t adBms6830_update_cell_over_avg_bit_hysteresis(float cellV,
                               float avgSegCellV,
                               uint8_t cellIndex,
                               uint16_t currentMask,
                               float hysteresis)
  {
    if ((cellIndex >= 12U) || (hysteresis < 0.0f)) {
      return currentMask;
    }

    uint16_t cellBit = (uint16_t)(1U << cellIndex);
    float setThreshold = avgSegCellV + hysteresis;
    float clearThreshold = avgSegCellV - hysteresis;

    if ((currentMask & cellBit) == 0U) {
      if (cellV > setThreshold) {
        currentMask |= cellBit;
      }
    } else {
      if (cellV < clearThreshold) {
        currentMask &= (uint16_t)(~cellBit);
      }
    }

    return currentMask;
  }

// Coeff 2
 float get_temp(float y) {
     float offset;
     float coeffs[3];

     // Clamp low range if necessary
     if (y < 1.3246f) return 120.0f;

     // Find the correct segment based on Y value
     if (y <= 1.344348f) {
         offset = 1.324638f;
         coeffs[0] = 120.00000000f; // a0 (Temperature at start of segment)
         coeffs[1] = -507.35294118f; // a1
         coeffs[2] = -1.00934293e-09f; // a2
     } else if (y <= 1.364058f) {
         offset = 1.344348f;
         coeffs[0] = 110.00000000f;
         coeffs[1] = -507.35294118f;
         coeffs[2] = 2.63844650e-08f;
     } else if (y <= 1.393623f) {
         offset = 1.364058f;
         coeffs[0] = 100.00000000f;
         coeffs[1] = -591.91176471f;
         coeffs[2] = 8.58023356e+03f;
     } else if (y <= 1.423188f) {
         offset = 1.393623f;
         coeffs[0] = 90.00000000f;
         coeffs[1] = -591.91176471f;
         coeffs[2] = 8.58023356e+03f;
     } else if (y <= 1.472464f) {
         offset = 1.423188f;
         coeffs[0] = 80.00000000f;
         coeffs[1] = -118.38235294f;
         coeffs[2] = -1.71604671e+03f;
     } else if (y <= 1.531594f) {
         offset = 1.472464f;
         coeffs[0] = 70.00000000f;
         coeffs[1] = -169.11764706f;
         coeffs[2] = -9.39198450e-10f;
     } else if (y <= 1.610435f) {
         offset = 1.531594f;
         coeffs[0] = 60.00000000f;
         coeffs[1] = -126.83823529f;
         coeffs[2] = 4.12100710e-10f;
     } else if (y <= 1.699130f) {
         offset = 1.610435f;
         coeffs[0] = 50.00000000f;
         coeffs[1] = -138.11274510f;
         coeffs[2] = 2.86007785e+02f;
     } else if (y <= 1.817391f) {
         offset = 1.699130f;
         coeffs[0] = 40.00000000f;
         coeffs[1] = -84.55882353f;
         coeffs[2] = 2.83721015e-12f;
     } else if (y <= 1.935652f) {
         offset = 1.817391f;
         coeffs[0] = 30.00000000f;
         coeffs[1] = -84.55882353f;
         coeffs[2] = 1.14084683e-10f;
     } else if (y <= 2.063768f) {
         offset = 1.935652f;
         coeffs[0] = 20.00000000f;
         coeffs[1] = -65.97446671f;
         coeffs[2] = -9.42882809e+01f;
     } else if (y <= 2.182029f) {
         offset = 2.063768f;
         coeffs[0] = 10.00000000f;
         coeffs[1] = -84.55882353f;
         coeffs[2] = 9.00682666e-13f;
     } else if (y <= 2.280580f) {
         offset = 2.182029f;
         coeffs[0] = 0.00000000f;
         coeffs[1] = -59.19117647f;
         coeffs[2] = -4.29011678e+02f;
     } else if (y <= 2.359420f) {
         offset = 2.280580f;
         coeffs[0] = -10.00000000f;
         coeffs[1] = -59.19117647f;
         coeffs[2] = -8.58023356e+02f;
     } else if (y <= 2.408696f) {
         offset = 2.359420f;
         coeffs[0] = -20.00000000f;
         coeffs[1] = -118.38235294f;
         coeffs[2] = -1.71604671e+03f;
     } else {
         // Last Segment (> 2.408696)
         offset = 2.408696f;
         coeffs[0] = -30.00000000f;
         coeffs[1] = -253.67647059f;
         coeffs[2] = 5.03436447e-12f;
     }

     // Compute Quadratic: Temp = a2*(y')^2 + a1*(y') + a0
     float y_p = y - offset;
     return coeffs[2] * y_p * y_p + coeffs[1] * y_p + coeffs[0];
 }
