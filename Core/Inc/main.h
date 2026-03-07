/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32l5xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "math.h"
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define USER_BUTTON_Pin GPIO_PIN_13
#define USER_BUTTON_GPIO_Port GPIOC
#define D8_Pin GPIO_PIN_12
#define D8_GPIO_Port GPIOF
#define D7_Pin GPIO_PIN_13
#define D7_GPIO_Port GPIOF
#define D4_Pin GPIO_PIN_14
#define D4_GPIO_Port GPIOF
#define D2_Pin GPIO_PIN_15
#define D2_GPIO_Port GPIOF
#define D6_Pin GPIO_PIN_9
#define D6_GPIO_Port GPIOE
#define D5_Pin GPIO_PIN_11
#define D5_GPIO_Port GPIOE
#define D3_Pin GPIO_PIN_13
#define D3_GPIO_Port GPIOE
#define SPI_CS_Pin GPIO_PIN_14
#define SPI_CS_GPIO_Port GPIOD
#define D9_Pin GPIO_PIN_15
#define D9_GPIO_Port GPIOD
#define LPUART1_TX_Pin GPIO_PIN_7
#define LPUART1_TX_GPIO_Port GPIOG
#define LPUART1_RX_Pin GPIO_PIN_8
#define LPUART1_RX_GPIO_Port GPIOG
#define LED_GREEN_Pin GPIO_PIN_7
#define LED_GREEN_GPIO_Port GPIOC
#define LED_RED_Pin GPIO_PIN_9
#define LED_RED_GPIO_Port GPIOA
#define LED_BLUE_Pin GPIO_PIN_7
#define LED_BLUE_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */
#ifndef SEGM_H
#define SEGM_H
typedef struct {
	float IC_T;
	float CELL_V[12];
	float CELL_T[12];
} SEG_PARAMS;

extern SEG_PARAMS SEG1, SEG2, SEG3, SEG4, SEG5;

#endif

#ifndef SEGB_H
#define SEGB_H

typedef struct {
	uint8_t M1to8;
	uint8_t M9to12;
} SEG_BSTAT;

extern SEG_BSTAT SEG1_B, SEG2_B, SEG3_B, SEG4_B, SEG5_B;

extern uint16_t TS_Current;
extern uint8_t AMS_SOC;
extern uint8_t AMS_SAFE;

extern uint16_t AMS_Charg_I;
extern uint16_t AMS_Charg_V;

extern uint8_t MCU_junction_temp;
extern float tV;
extern int all;
#endif


/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
