/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.h
 * @brief          : Header for main.c file.
 *                   This file contains the common defines of the application.
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2023 STMicroelectronics.
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
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_hal.h"

  /* Private includes ----------------------------------------------------------*/
  /* USER CODE BEGIN Includes */
  typedef struct __attribute__((packed)) _deviceCoords
  {
    uint16_t networkID;
    uint8_t flag;
    int32_t posX;
    int32_t posY;
    int32_t posZ;
  } deviceCoords_t;

  typedef struct __attribute__((packed)) _coordinates
  {
    int32_t posX;
    int32_t posY;
    int32_t posZ;
  } coordinates_t;

  typedef struct __attribute__((packed)) _calibration
  {
    uint16_t anchorID1;
    uint16_t anchorID2;
    uint16_t anchorID3;
    uint16_t anchorID4;
  } calibration_t;

#include "string.h"
#include "stdio.h"
#include "stddef.h"
#include "i2c.h"
#include "wireless.h"
#include "pozyx.h"
#include "registers.h"
#include "zigbee.h"
#include "stdlib.h"
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
#define SLAVE_ADDR 0x4B
#define MASTER_TAG_I2C hi2c1
#define NUM_ANCHORS 8
#define ANCHOR_1 1
#define ANCHOR_ID_1 0x1114
#define ANCHOR_2 2
#define ANCHOR_ID_2 0x1131
#define ANCHOR_3 3
#define ANCHOR_ID_3 0x1103
#define ANCHOR_4 4
#define ANCHOR_ID_4 0x1172
#define ANCHOR_FLAG 0x1
#define TAG_FLAG 0x0

#define INT_PORT GPIOA
#define INT_PIN GPIO_PIN_3

#define BYTE_SIZE_1 sizeof(uint8_t)
#define BYTE_SIZE_2 sizeof(uint8_t) * 2
#define BYTE_SIZE_3 sizeof(uint8_t) * 3

#define DIMENSION POZYX_2_5D

#define BAY_WIDTH_MAX 21860
#define BAY_LENGTH_MAX 45600
#define BAY_WIDTH_MIN 0
#define BAY_LENGTH_MIN 0
#define MAX_CRANE_SPEED 1500
#define OFFSET 1500
#define MAX_DISTANCE_TRAVELLED 2000

#define CRANE_ID 3
  /* USER CODE END EM */

  /* Exported functions prototypes ---------------------------------------------*/
  void Error_Handler(void);

  /* USER CODE BEGIN EFP */

  /* USER CODE END EFP */

  /* Private defines -----------------------------------------------------------*/
  /* USER CODE BEGIN Private defines */

  /* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
