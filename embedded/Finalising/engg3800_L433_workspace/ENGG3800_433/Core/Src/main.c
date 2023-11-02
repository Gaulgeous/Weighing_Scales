/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
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
#include "ili9163.h"
#include "ili9341_touch.h"
#include "display.h"
#include "L433_ADC.h"
#include <string.h>
#include <stdio.h>
#include <inttypes.h>
#include <stdint.h>
#include "wifi.h"

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
extern I2C_HandleTypeDef hi2c1;

SPI_HandleTypeDef hspi1;
DMA_HandleTypeDef hdma_spi1_tx;

TIM_HandleTypeDef htim2;

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
uint8_t SPI_DMA_FL = 0;
uint32_t SPI_DMA_CNT=1;
uint8_t touchProcessing = 0;
uint16_t lastTick = 0;

ScreenData screenData;
uint8_t uart2RxBuffer[1] = {0};
uint8_t uart1RxBuffer[5] = {0};
uint8_t serialMsg[5] = {0};
extern int uart2Counter;
extern uint8_t pos;
extern char mainWifiBuf[MAIN_BUF];
extern uint8_t wifiStat[6];
extern uint8_t wifiSentStat[6];

uint32_t lastWifi = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_SPI1_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_TIM2_Init(void);
static void MX_I2C1_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void hardware_init(ScreenData* screenData) {

	init_screen(screenData);
	HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_3);
	__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_3, screenData->dutyCycle);

}

void display_screen(ScreenData* screenData) {

	ILI9163_newFrame();

	if (screenData->frame == WEIGHTING) {

		  weighting_mode(screenData);
		  brightnessButton();
	  } else if (screenData->frame == COUNTING) {

		  counting_mode(screenData);
		  brightnessButton();
	  } else if(screenData->frame == PASS) {

		  pass_mode(screenData);
		  brightnessButton();
	  } else if (screenData->frame == MENU) {

		  mode_select();
		  brightnessButton();
	  } else if (screenData->frame == KEYBOARD) {

		  keyboard(screenData);
		  brightnessButton();
	  } else if(screenData->frame == CALIBRATION) {

		  calibration(screenData);
		  brightnessButton();
	  }
	  ILI9163_render();
}

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
  HAL_Delay(1000);
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_SPI1_Init();
  MX_USART2_UART_Init();
  MX_USART1_UART_Init();
  MX_TIM2_Init();
  MX_I2C1_Init();
  /* USER CODE BEGIN 2 */

  hardware_init(&screenData);
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_3);
  //HAL_Delay(1000);
  ILI9163_init(1); // Flipped landscape
  ADC_Init(&hi2c1, &screenData);

  char message[12];
  float weight;
  uint32_t weight_int;
  uint32_t value;
  uint32_t offset_value;
  int lastTick1 = 0;

  uint32_t zero;
  uint32_t offset;

  HAL_Delay(500);
  offset = ADC_Read(&hi2c1); //Read an initial value
  //  offset = zero - zero*0.001;// Zero value plus 0.1% of the zero value
  store_offset(offset);

  HAL_UART_Receive_IT(&huart1, uart2RxBuffer, 1);

  // Last thing is to put this inside the wifi initialisation process
  HAL_UART_Receive_IT(&huart2, uart1RxBuffer, 5);

  //get_cip(&huart1);

  //initialiseWifi(&huart1, &screenData);
  //MQTTConnect(&huart1, &screenData);


  uint32_t temp2 = 0;

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  if (!wifiStat[SUB]) {
		if (!wifiStat[RST]) {
			 atRst(&huart1, &screenData);
	  	} else if (!wifiStat[CWM]) {
			initialiseWifi(&huart1, &screenData, &htim2);
		} else if (!wifiStat[CWJ]) {
			wifi_bread(&huart1, &screenData, &htim2);
		} else if (!wifiStat[USE]) {
			MQTTUse(&huart1, &screenData, &htim2);
		} else if (!wifiStat[CON]) {
			MQTTConnect(&huart1, &screenData);
		} else {
			MQTTSub(&huart1, &screenData, &htim2);
		}
	  }

	  if (screenData.connection == 1 && HAL_GetTick() - lastWifi > CHECK_TIME3) {
		  HAL_UART_Receive_IT(&huart1, uart2RxBuffer, 1);
	  }

	 uint16_t x = 0;
	 uint16_t y = 0;
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

	if (screenData.calculateCal == 1) {

		calibration_process(&screenData);
		sprintf(message, "confirm");
		//HAL_UART_Transmit(&huart1, message, strlen(message), HAL_MAX_DELAY);

		screenData.calculateCal = 0;
	}
	temp2 = return_cal_factor();
	//sprintf(message, "C: %d\r\n\n", temp2);
	//HAL_UART_Transmit(&huart1, message, strlen(message), HAL_MAX_DELAY);

	value = ADC_Read(&hi2c1);
	screenData.rawADC = value;
	//200: 405, 500: 405
	//offset_value = value - offset;
	weight = calc_Weight(value);
	//weight_int = (int)weight;
	screenData.weight = (int)weight;
//	sprintf(message, "VALUE: %d\r\n\n", weight_int);
//	HAL_UART_Transmit(&huart1, message, strlen(message), HAL_MAX_DELAY);
	if(touchProcessing == 1)
	{
	  if ((HAL_GetTick() - lastTick1) >= 200) {

	  	  ILI9341_TouchGetCoordinates(&y, &x);
		  screenData.xValue = x;
		  screenData.yValue = 128 - y;
	  	  lastTick1 = HAL_GetTick();
	  }
	  //ILI9163_drawRect(1, 1, 75, 75, 3, RED);
	  //ILI9163_drawCircle(screenData.xValue, screenData.yValue, 10, RED);
	  touchProcessing = 0;
	  touchProcess(&htim2, &screenData);

	}

	display_screen(&screenData);
	//HAL_Delay(100);
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
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_10;
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

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
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
  hi2c1.Init.Timing = 0x00707CBB;
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
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 7;
  hspi1.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  hspi1.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 0;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 25;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */
  HAL_TIM_MspPostInit(&htim2);

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
  huart1.Init.BaudRate = 115200;
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
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel3_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel3_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel3_IRQn);

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

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, DISP_CS_Pin|ILI9341_TOUCH_CS_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, DISP_DC_Pin|DISP_RST_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : DISP_CS_Pin ILI9341_TOUCH_CS_Pin */
  GPIO_InitStruct.Pin = DISP_CS_Pin|ILI9341_TOUCH_CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : ILI9341_TOUCH_IRQ_Pin */
  GPIO_InitStruct.Pin = ILI9341_TOUCH_IRQ_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(ILI9341_TOUCH_IRQ_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : DISP_DC_Pin DISP_RST_Pin */
  GPIO_InitStruct.Pin = DISP_DC_Pin|DISP_RST_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

}

/* USER CODE BEGIN 4 */


/*
 * Callback for in case when the wifi doesn't pick up a character
 * Resets the buffer for receiving more information
 */
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart) {
	if (huart->Instance == USART1) {
		HAL_UART_Receive_IT(&huart1, uart1RxBuffer, 5);
	} else if (huart->Instance == USART2) {
		HAL_UART_Receive_IT(&huart2, uart2RxBuffer, 1);
	}
}

/*
 * Callback for the receive interrupt for both UART lines
 * WIFI uses uart line 2, serial uses line 1
 * Breaks down each incoming message into its components
 * Sends a return confirmation over given coms line
 *
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	// Serial input
	if (huart->Instance == USART2) {
		char message[12];

		for (int i = 0; i < 5; i++) {
			serialMsg[i] = uart1RxBuffer[i];
		}

		int input = serialMsg[4] | (serialMsg[3] << 8) | (serialMsg[2] << 16) | (serialMsg[1] << 24);
		int pos = serialMsg[0];

		int retValue = parse_values(input, &screenData, &pos, &htim2);

		sprintf(message, "%d %d\n", pos, retValue);
		HAL_UART_Transmit(&huart2, (uint8_t*)message, strlen(message), HAL_MAX_DELAY);
		HAL_UART_Receive_IT(&huart2, uart1RxBuffer, 5);

	// WIFI
	} else if (huart->Instance == USART1) {

		mainWifiBuf[uart2Counter] = uart2RxBuffer[0];
		if (uart2Counter == RX_BUF) {
			memset(mainWifiBuf, 0, MAIN_BUF);
			uart2Counter = 0;
		}

		if (mainWifiBuf[uart2Counter] == '\n' && mainWifiBuf[uart2Counter - 1] == '\r') {
			if (returnState(huart1, uart2Counter, &screenData, &htim2)) {
				memset(mainWifiBuf, 0, MAIN_BUF);
				uart2Counter = 0;
			}
		}

		uart2Counter++;
		lastWifi = HAL_GetTick();

		HAL_UART_Receive_IT(&huart1, uart2RxBuffer, 1);
	}
}

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi) // Your TxCpltCallback
{
	SPI_DMA_CNT--;
	if(SPI_DMA_CNT==0)
	{
		HAL_SPI_DMAStop(&hspi1);
		SPI_DMA_CNT=1;
		SPI_DMA_FL=1;
	}
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {

	if (GPIO_Pin == ILI9341_TOUCH_IRQ_Pin) {

		if(ILI9341_TouchPressed()) {

			if ((HAL_GetTick() - lastTick) >= 300) {

				touchProcessing = 1;
			}
			lastTick = HAL_GetTick();
		}
	}
}

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
