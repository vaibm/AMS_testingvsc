/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    fdcan.c
  * @brief   This file provides code for the configuration
  *          of the FDCAN instances.
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
/* Includes ------------------------------------------------------------------*/
#include "fdcan.h"

/* USER CODE BEGIN 0 */
CanTxMsg canSeg1, canSeg2, canSeg3, canSeg4, canSeg5, canAccuStat, canStat, canChargeStat;

SEG_BSTAT *segstat_list[] = {
		&SEG1_B,
		&SEG2_B,
		&SEG3_B,
		&SEG4_B,
		&SEG5_B
};

/* USER CODE END 0 */

FDCAN_HandleTypeDef hfdcan1;

/* FDCAN1 init function */
void MX_FDCAN1_Init(void)
{

  /* USER CODE BEGIN FDCAN1_Init 0 */

  /* USER CODE END FDCAN1_Init 0 */

  /* USER CODE BEGIN FDCAN1_Init 1 */

  /* USER CODE END FDCAN1_Init 1 */
  hfdcan1.Instance = FDCAN1;
  hfdcan1.Init.ClockDivider = FDCAN_CLOCK_DIV1;
  hfdcan1.Init.FrameFormat = FDCAN_FRAME_FD_NO_BRS;
  hfdcan1.Init.Mode = FDCAN_MODE_NORMAL;
  hfdcan1.Init.AutoRetransmission = DISABLE;
  hfdcan1.Init.TransmitPause = DISABLE;
  hfdcan1.Init.ProtocolException = DISABLE;
  hfdcan1.Init.NominalPrescaler = 1;
  hfdcan1.Init.NominalSyncJumpWidth = 13;
  hfdcan1.Init.NominalTimeSeg1 = 86;
  hfdcan1.Init.NominalTimeSeg2 = 13;
  hfdcan1.Init.DataPrescaler = 25;
  hfdcan1.Init.DataSyncJumpWidth = 1;
  hfdcan1.Init.DataTimeSeg1 = 2;
  hfdcan1.Init.DataTimeSeg2 = 1;
  hfdcan1.Init.StdFiltersNbr = 0;
  hfdcan1.Init.ExtFiltersNbr = 0;
  hfdcan1.Init.TxFifoQueueMode = FDCAN_TX_FIFO_OPERATION;
  if (HAL_FDCAN_Init(&hfdcan1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN FDCAN1_Init 2 */

  /* USER CODE END FDCAN1_Init 2 */

}

void HAL_FDCAN_MspInit(FDCAN_HandleTypeDef* fdcanHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};
  if(fdcanHandle->Instance==FDCAN1)
  {
  /* USER CODE BEGIN FDCAN1_MspInit 0 */

  /* USER CODE END FDCAN1_MspInit 0 */

  /** Initializes the peripherals clock
  */
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_FDCAN;
    PeriphClkInit.FdcanClockSelection = RCC_FDCANCLKSOURCE_PLL;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
    {
      Error_Handler();
    }

    /* FDCAN1 clock enable */
    __HAL_RCC_FDCAN1_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**FDCAN1 GPIO Configuration
    PA11     ------> FDCAN1_RX
    PA12     ------> FDCAN1_TX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_11|GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF9_FDCAN1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* USER CODE BEGIN FDCAN1_MspInit 1 */

  /* USER CODE END FDCAN1_MspInit 1 */
  }
}

void HAL_FDCAN_MspDeInit(FDCAN_HandleTypeDef* fdcanHandle)
{

  if(fdcanHandle->Instance==FDCAN1)
  {
  /* USER CODE BEGIN FDCAN1_MspDeInit 0 */

  /* USER CODE END FDCAN1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_FDCAN1_CLK_DISABLE();

    /**FDCAN1 GPIO Configuration
    PA11     ------> FDCAN1_RX
    PA12     ------> FDCAN1_TX
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_11|GPIO_PIN_12);

  /* USER CODE BEGIN FDCAN1_MspDeInit 1 */

  /* USER CODE END FDCAN1_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

void canFraming(void)
{
	canStat.datas[0] = MCU_junction_temp; // need to be verified

	for (int i=0; i<=11; i++)
	{
		canSeg1.datas[i] = (uint8_t) round((SEG1.CELL_V[i])*50) ;
		canSeg1.datas[i+12] = (uint8_t) round((SEG1.CELL_T[i])*4) ;
	}
	canSeg1.datas[24] = (uint8_t) round((SEG1.IC_T)*4);

	for (int i=0; i<=11; i++)
	{
		canSeg2.datas[i] = (uint8_t) round((SEG2.CELL_V[i])*50) ;
		canSeg2.datas[i+12] = (uint8_t) round((SEG2.CELL_T[i])*4) ;
	}
	canSeg2.datas[24] = (uint8_t) round((SEG2.IC_T)*4);

	for (int i=0; i<=11; i++)
	{
		canSeg3.datas[i] = (uint8_t) round((SEG3.CELL_V[i])*50) ;
		canSeg3.datas[i+12] = (uint8_t) round((SEG3.CELL_T[i])*4) ;
	}
	canSeg3.datas[24] = (uint8_t) round((SEG3.IC_T)*4);

	for (int i=0; i<=11; i++)
	{
		canSeg4.datas[i] = (uint8_t) round((SEG4.CELL_V[i])*50) ;
		canSeg4.datas[i+12] = (uint8_t) round((SEG4.CELL_T[i])*4) ;
	}
	canSeg4.datas[24] = (uint8_t) round((SEG4.IC_T)*4);


	for (int i=0; i<=11; i++)
	{
		canSeg5.datas[i] = (uint8_t) round((SEG5.CELL_V[i])*50) ;
		canSeg5.datas[i+12] = (uint8_t) round((SEG5.CELL_T[i])*4) ;
	}
	canSeg5.datas[24] = (uint8_t) round((SEG5.IC_T)*4);
	//int j = 0;

	for (int j=0; j<9; j=j+2)
	{
		SEG_BSTAT *st = segstat_list[j/2];
		canAccuStat.datas[j] = st->M1to8 ;
		canAccuStat.datas[j+1] = st->M9to12 ;
	}

	canAccuStat.datas[10] = TS_Current & 0xFF;
	canAccuStat.datas[11] = TS_Current >> 8;
	canAccuStat.datas[12] = AMS_SOC; // SOC need to be calculated
	canAccuStat.datas[13] = AMS_SAFE;

	canChargeStat.datas[0] = AMS_Charg_V & 0xFF;
	canChargeStat.datas[1] = AMS_Charg_V >> 8;
	canChargeStat.datas[2] = AMS_Charg_I & 0xFF;
	canChargeStat.datas[3] = AMS_Charg_I >> 8;




}

void CAN_Data_Init (void)
{
	if(HAL_FDCAN_Start(&hfdcan1)!= HAL_OK) {
		Error_Handler();
	}


	canSeg1.header.Identifier = 0x0E1;
	canSeg1.header.IdType = FDCAN_STANDARD_ID;
	canSeg1.header.TxFrameType = FDCAN_DATA_FRAME;
	canSeg1.header.DataLength = FDCAN_DLC_BYTES_32;
	canSeg1.header.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
	canSeg1.header.BitRateSwitch = FDCAN_BRS_OFF;
	canSeg1.header.FDFormat = FDCAN_FD_CAN;
	canSeg1.header.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
	canSeg1.header.MessageMarker = 0;

}

void CAN_DataTX_1s (void)
{

	if (HAL_FDCAN_GetTxFifoFreeLevel(&hfdcan1) > 0)
	  {
			if (HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &canSeg1.header, canSeg1.datas)!= HAL_OK)
			 {
			  Error_Handler();
			 }
	  }



	};
/* USER CODE END 1 */
