/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;

I2C_HandleTypeDef hi2c1;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_I2C1_Init(void);
static void MX_ADC1_Init(void);
static void MX_NVIC_Init(void);
/* USER CODE BEGIN PFP */
void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *hi2c);
void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *hi2c);
void add_device_parameters(uint16_t networkID, uint8_t flag, uint32_t posX, uint32_t posY, uint32_t posZ, deviceCoords_t *device);
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin);
void reassign_anchors(I2C_HandleTypeDef *hi2c, deviceCoords_t a1, deviceCoords_t a2, deviceCoords_t a3, deviceCoords_t a4,
		deviceCoords_t a5, deviceCoords_t a6, uint16_t networkID);

#define ADD_ANCHOR(networkID, posX, posY, posZ, device) add_device_parameters(networkID, ANCHOR_FLAG, posX, posY, posZ, device)
#define ADD_TAG(networkID, posX, posY, posZ, device) add_device_parameters(networkID, TAG_FLAG, posX, posY, posZ, device)

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
volatile int interruptFlag = 0;

uint8_t uartbuf[1] = {0};
uint8_t buffer[50] = {0};
int count = 0;
int flag = 0;
int index1 = 0;
int size = 0;
int dataSize = 0;
int dataFlag = 0;
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART1_UART_Init();
  MX_I2C1_Init();
  MX_ADC1_Init();

  /* Initialize interrupts */
  MX_NVIC_Init();
  /* USER CODE BEGIN 2 */

    HAL_Delay(10000);	//wait 4 seconds

	uint32_t prevTime = 0;	//last time since position request
	uint32_t prevTime2 = 0;	//time of last successful position calculation

	uint8_t readSinceLastPos = 1;	//number of reads since last successful positioning

	uint32_t adcResult = 0;		//holds ADC strain of load gauge

	uint8_t readsSinceMovement = 0;	//indicates how many reads since crane last moved

	uint8_t errorFlag = 0;	//1 if an error occurred in the last read, 0 if else

	//1byte buffers for sending/receiving data
	uint8_t rxBuffer[50];
	uint8_t txBuffer[50];
	memset(rxBuffer, '\0', sizeof (rxBuffer));
	memset(txBuffer, '\0', sizeof (txBuffer));

	//Initialise anchor positions to zero
	deviceCoords_t anchor1, anchor2, anchor3, anchor4, anchor5, anchor6, anchor7, anchor8, tag1;
	coordinates_t realTimePositions, prevPositions, outOfBoundsPos, errorPos;

	coordinates_t positionArr[10];
	uint8_t positionArrayIndex = 0;


	//Initialise anchor network id and positions locally
	ADD_ANCHOR(0x1172, 100, 100, 5000, &anchor1);
	ADD_ANCHOR(0x1114, 21860, 0, 5000, &anchor2);
	ADD_ANCHOR(0x1103, 0, 15200, 5000, &anchor3);
	ADD_ANCHOR(0x1131, 21860, 15200, 5000, &anchor4);
	ADD_ANCHOR(0x6830, 0, 30400, 5000, &anchor5);
	ADD_ANCHOR(0x1152, 21860, 30400, 5000, &anchor6);
	ADD_ANCHOR(0x6846, 0, 45600, 5000, &anchor7);
	ADD_ANCHOR(0x6842, 21860, 45600, 5000, &anchor8);
	ADD_TAG(0x6875, 0, 0, 0, &tag1);

	// TEST RESPONSE //
	uint8_t remoteInitOk2[] = {'B', 'E', 'G', 'I', 'N' , ' ', 'I', 'N', 'I', 'T', '\n'};
	zigbee_send_other_data(&huart1, remoteInitOk2, sizeof (remoteInitOk2));
	// TEST RESPONSE //

    //Initialize mater tag
    master_tag_init(SLAVE_ADDR, &hi2c1);

	HAL_Delay(150);	//wait 150ms

	//Add anchors into master tag memory
	add_anchors(SLAVE_ADDR, &hi2c1, anchor1);
	add_anchors(SLAVE_ADDR, &hi2c1, anchor2);
	add_anchors(SLAVE_ADDR, &hi2c1, anchor3);
	add_anchors(SLAVE_ADDR, &hi2c1, anchor4);
	add_anchors(SLAVE_ADDR, &hi2c1, anchor5);
	add_anchors(SLAVE_ADDR, &hi2c1, anchor6);
	add_anchors(SLAVE_ADDR, &hi2c1, anchor7);
	add_anchors(SLAVE_ADDR, &hi2c1, anchor8);
	add_anchors(SLAVE_ADDR, &hi2c1, tag1);

	HAL_Delay(150);		//wait 150ms

	//Set number of anchors on master tag
	memset(txBuffer, '\0', sizeof (txBuffer));
	txBuffer[0] = (NUM_ANCHORS | (1 << 7));
	I2C_Write_Reg(&hi2c1, POZYX_POS_NUM_ANCHORS, txBuffer, BYTE_SIZE_1);

	// TEST RESPONSE //
	uint8_t remoteInitOk3[] = {'I', 'N', 'I', 'T', ' ', 'O', 'K', '\r', '\n'};
	zigbee_send_other_data(&huart1, remoteInitOk3, sizeof (remoteInitOk3));
	// TEST RESPONSE //

	HAL_Delay(1000);	//wait 1000ms

	// TEST RESPONSE //
	uint8_t remoteInitOk1[] = {'B', 'E', 'G', 'I', 'N' , ' ', 'I', 'N', 'I', 'T', '\n'};
	zigbee_send_other_data(&huart1, remoteInitOk1, sizeof (remoteInitOk1));
	// TEST RESPONSE //

	//Initialise remote tag
	while (remote_tag_init(&hi2c1, tag1.networkID) != GOOD_INIT);

	memset(txBuffer, '\0', sizeof (txBuffer));

	// TEST RESPONSE //
	uint8_t remoteInitOk[] = {'I', 'N', 'I', 'T', ' ', 'O', 'K', '\r', '\n'};
	zigbee_send_other_data(&huart1, remoteInitOk, sizeof (remoteInitOk));
	// TEST RESPONSE //

	//Add anchors into remote tag memory
	remote_add_anchors(&hi2c1, anchor1, tag1.networkID);
	remote_add_anchors(&hi2c1, anchor2, tag1.networkID);
	remote_add_anchors(&hi2c1, anchor3, tag1.networkID);
	remote_add_anchors(&hi2c1, anchor4, tag1.networkID);
	remote_add_anchors(&hi2c1, anchor5, tag1.networkID);
	remote_add_anchors(&hi2c1, anchor6, tag1.networkID);
	remote_add_anchors(&hi2c1, anchor7, tag1.networkID);
	remote_add_anchors(&hi2c1, anchor8, tag1.networkID);

	remote_save_device_list(&hi2c1, tag1.networkID);	//flash device list into remote tag memory

	// TEST RESPONSE
	uint8_t anchorsOk[] = {'A', 'N', 'C', 'H', 'O', 'R', 'S', ' ', 'O', 'K', '\r', '\n'};
	zigbee_send_other_data(&huart1, anchorsOk,  sizeof (anchorsOk));
	// TEST RESPONSE

	//Set number of anchors on remote tag
	txBuffer[0] = (POZYX_ANCHOR_SEL_AUTO << 7) + NUM_ANCHORS;
	Remote_Write_Reg_Read(&hi2c1, tag1.networkID, POZYX_POS_NUM_ANCHORS, txBuffer,
			BYTE_SIZE_1, rxBuffer, BYTE_SIZE_2);
	memset(txBuffer, '\0', sizeof (txBuffer));

	//flash number of anchors into remote tag memory
	remote_flash_register(&hi2c1, tag1.networkID, POZYX_POS_NUM_ANCHORS);

	//Get start up position & send data
	remote_positioning(&hi2c1, tag1.networkID, &realTimePositions);
	zigbee_send_data(&huart1, realTimePositions, 1000, CRANE_ID);

	prevPositions.posX = realTimePositions.posX;
	prevPositions.posY = realTimePositions.posY;

	positionArr[0] = realTimePositions;
	positionArrayIndex++;

	uint8_t testFlag = 0x01;

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

	  //Check for an interrupt
	  if (interruptFlag) {
			I2C_Read_Reg(&hi2c1, POZYX_INT_STATUS, rxBuffer, BYTE_SIZE_1);
			if ((rxBuffer[0] & POZYX_INT_STATUS_ERR) == POZYX_INT_STATUS_ERR) {		//an error has occurred

			}
			interruptFlag = 0;
	  }

	  //Update positions and mass at 3.5Hz
	  if ((HAL_GetTick() - prevTime) >= 200) {

		  //Poll ADC
		  HAL_ADC_Start(&hadc1);
		  HAL_ADC_PollForConversion(&hadc1, 100);
		  adcResult = HAL_ADC_GetValue(&hadc1);

		  //perform remote positioning of tag 1. takes approx 140ms
		  if (remote_positioning(&hi2c1, tag1.networkID, &realTimePositions) != POSITIONS_RETRIEVED) {
			  //error in positioning
			  prevTime = HAL_GetTick();
			  continue;
		  }

		  uint32_t distanceChangeX, distanceChangeY;

		  //Check if the tag has moved at least 0.35m in any direction and check for any errors
		  if (((distanceChangeX = labs(realTimePositions.posX - prevPositions.posX)) >= 350) |
				  ((distanceChangeY = labs(realTimePositions.posY - prevPositions.posY)) >= 350)) {

			  if ((distanceChangeX < 400) & (distanceChangeY < 400)) {
				  if (readsSinceMovement < 200) {
					  readsSinceMovement++;
				  }
			  } else {
				  readsSinceMovement = 0;
			  }

			  //Do the positions suggest crane is travelling faster than possible?
			  if ((distanceChangeX > (MAX_DISTANCE_TRAVELLED * readSinceLastPos)) | (distanceChangeY > (MAX_DISTANCE_TRAVELLED * readSinceLastPos))) {
				  readSinceLastPos++;
				  prevTime = HAL_GetTick();
				  continue;
			  }

			  //Are the calculated positions beyond the boundaries?
			  if ((realTimePositions.posX > (BAY_WIDTH_MAX + OFFSET)) || (realTimePositions.posX < (BAY_WIDTH_MIN - OFFSET)) ||
					  (realTimePositions.posY > (BAY_LENGTH_MAX + OFFSET)) || (realTimePositions.posY < (BAY_LENGTH_MIN - OFFSET))) {

				  readSinceLastPos++;

				  //Update prevPositions
				  prevPositions.posX = realTimePositions.posX;
				  prevPositions.posY = realTimePositions.posY;

				  prevTime = HAL_GetTick();
				  continue;
 			  }
		  } else {
			  if (readsSinceMovement < 200) {
				  readsSinceMovement++;	//Crane hasn't moved minimum distance
			  }
		  }

		  //Reassign anchors if tag has moved past threshold
		  if ((realTimePositions.posY >= 30400) & !testFlag) {
				reassign_anchors(&hi2c1, anchor3, anchor4, anchor5, anchor6, anchor7, anchor8, tag1.networkID);
				testFlag = 1;
		  } else if ((realTimePositions.posY < 30400) & testFlag) {
				reassign_anchors(&hi2c1, anchor1, anchor2, anchor3, anchor4, anchor5, anchor6, tag1.networkID);
				testFlag = 0;
		  }

		  //Check if the crane has moved in the last minute by at least 0.35m
		  if (readsSinceMovement >= 200) {
			  zigbee_send_okay(&huart1, CRANE_ID);
		  } else {
			  zigbee_send_data(&huart1, realTimePositions, adcResult, CRANE_ID);
		  }

		  //Update prevPositions
		  prevPositions.posX = realTimePositions.posX;
		  prevPositions.posY = realTimePositions.posY;

		  //Update buffer with positions
		  if (positionArrayIndex < 10) {
			  positionArr[positionArrayIndex] = realTimePositions;
			  positionArrayIndex++;
		  } else {
			  for (int i = 0; i < positionArrayIndex; i++) {
				  positionArr[9 - i] = positionArr[8 - i];
			  }
			  positionArr[0] = realTimePositions;
		  }

		  readSinceLastPos = 1;

		  prevTime = HAL_GetTick();
	  }
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief NVIC Configuration.
  * @retval None
  */
static void MX_NVIC_Init(void)
{
  /* USART1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(USART1_IRQn, 10, 0);
  HAL_NVIC_EnableIRQ(USART1_IRQn);
  /* I2C1_ER_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(I2C1_ER_IRQn, 6, 0);
  HAL_NVIC_EnableIRQ(I2C1_ER_IRQn);
  /* I2C1_EV_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(I2C1_EV_IRQn, 10, 0);
  HAL_NVIC_EnableIRQ(I2C1_EV_IRQn);
  /* EXTI3_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(EXTI3_IRQn, 8, 0);
  HAL_NVIC_EnableIRQ(EXTI3_IRQn);
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Common config
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV1;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc1.Init.LowPowerAutoWait = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.Overrun = ADC_OVR_DATA_PRESERVED;
  hadc1.Init.OversamplingMode = DISABLE;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_9;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_2CYCLES_5;
  sConfig.SingleDiff = ADC_SINGLE_ENDED;
  sConfig.OffsetNumber = ADC_OFFSET_NONE;
  sConfig.Offset = 0;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x00000E14;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 38400;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin : PA3 */
  GPIO_InitStruct.Pin = GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

void reassign_anchors(I2C_HandleTypeDef *hi2c, deviceCoords_t a1, deviceCoords_t a2, deviceCoords_t a3, deviceCoords_t a4,
		deviceCoords_t a5, deviceCoords_t a6, uint16_t networkID) {

	uint8_t rxBuffer[10];
	memset(rxBuffer, 0, sizeof (rxBuffer));

	//Clear devices list
	if (Remote_Function_Call_Read(hi2c, networkID, POZYX_DEVICES_CLEAR, NULL,
			0, rxBuffer, BYTE_SIZE_2) != TRANSMITTED_MESSAGE) {
		return BAD_FUNCTION_CALL;
	}
	if (rxBuffer[1] != 0x01) {
		return BAD_FUNCTION_CALL;
	}

	//Add anchors into remote tag memory
	remote_add_anchors(hi2c, a1, networkID);
	remote_add_anchors(hi2c, a2, networkID);
	remote_add_anchors(hi2c, a3, networkID);
	remote_add_anchors(hi2c, a4, networkID);
	remote_add_anchors(hi2c, a5, networkID);
	remote_add_anchors(hi2c, a6, networkID);
}

/**
  * @brief  Master Tx Transfer completed callback.
  * @param  hi2c Pointer to a I2C_HandleTypeDef structure that contains
  *                the configuration information for the specified I2C.
  * @retval None
  */
void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hi2c);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_I2C_MasterTxCpltCallback could be implemented in the user file
   */
}

/**
  * @brief  Master Rx Transfer completed callback.
  * @param  hi2c Pointer to a I2C_HandleTypeDef structure that contains
  *                the configuration information for the specified I2C.
  * @retval None
  */
void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hi2c);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_I2C_MasterRxCpltCallback could be implemented in the user file
   */
}

/** Set the appropriate parameters for a pozyx device
 *  @param networkID the networkID of the device
 *  @param flag the pozyx flag for the device indicating if it is an anchor or tag
 *  @paramm posX the x position of the flag
 *  @param posY the y position of the flag
 *  @param posZ the z position of the flag
 */
void add_device_parameters(uint16_t networkID, uint8_t flag, uint32_t posX, uint32_t posY, uint32_t posZ,
		deviceCoords_t *device) {
	device->flag = flag;
	device->networkID = networkID;
	device->posX = posX;
	device->posY = posY;
	device->posZ = posZ;
}

/** External GPIO interrupt callback function
 *  @param GPIO_Pin pin which has triggered an interrupt
 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {

	//Check if interrupt triggered is on appropriate pin
	if (GPIO_Pin == INT_PIN) {
		interruptFlag = 1;
	}
}


//void master_tag_add_anchors(deviceCoords_t* anchors, uint16_t anchorsSize) {
//	for (int i = 0; i < anchorsSize; i++) {
//		add_anchors(SLAVE_ADDR, &hi2c1, *(anchors + i));
//	}
//}



/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
