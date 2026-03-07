/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    fdcan.h
  * @brief   This file contains all the function prototypes for
  *          the fdcan.c file
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __FDCAN_H__
#define __FDCAN_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

extern FDCAN_HandleTypeDef hfdcan1;

/* USER CODE BEGIN Private defines */
//typedef struct
//{
    //uint8_t data[64];
  //  FDCAN_RxHeaderTypeDef header;
//} CanRxMsg;

#ifndef CANTX_H
#define CANTX_H
typedef struct {

   uint8_t datas[25];
   FDCAN_TxHeaderTypeDef header;
   }CanTxMsg;

extern CanTxMsg canSeg1, canSeg2, canSeg3, canSeg4, canSeg5, canAccuStat, canStat, canChargeStat;

#endif

void canFraming(void);
void CAN_Data_Init (void);
void CAN_DataTX_1s (void);
/* USER CODE END Private defines */

void MX_FDCAN1_Init(void);

/* USER CODE BEGIN Prototypes */

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __FDCAN_H__ */

