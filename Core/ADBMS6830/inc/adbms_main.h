/*******************************************************************************
Copyright (c) 2020 - Analog Devices Inc. All Rights Reserved.
This software is proprietary & confidential to Analog Devices, Inc.
and its licensor.
******************************************************************************
* @file:    adbms_main.h
* @brief:   adbms main Heade file
* @version: $Revision$
* @date:    $Date$
* Developed by: ADIBMS Software team, Bangalore, India
*****************************************************************************/
/** @addtogroup MAIN
*  @{
*
*/

/** @addtogroup ADBMS_MAIN MAIN
*  @{
*
*/
#ifndef _ADBMS6830_MAIN_H
#define _ADBMS6830_MAIN_H

#include "main.h"

#include "adBms6830Data.h"
#include "adBms6830GenericType.h"
#include "adBms6830ParseCreate.h"
#include "common.h"
#include "mcuWrapper.h"


void adbms_main(void);
void adbms_init_loop(void);
void adbms_readCell_loop(void);
void adbms_startCellConv_loop(void);
void adbms_readTempToggle (void);
void testOpenWire(void);
void adbmsReinit(uint8_t tIC, cell_asic *ic);
void adbmsReinitMain(void);
#endif
/** @}*/
/** @}*/
