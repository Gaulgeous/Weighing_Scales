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
#include <string.h>
#include <stdio.h>
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
I2C_HandleTypeDef hi2c2;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */

//Default strain gauge value is 0.08mV

static const uint16_t  NAU7802_I2C_ADDR = 0x2A; //Fixed address for NAU7802
static const uint8_t NAU7802_PU_CTRL = 0x00;
static const uint8_t NAU7802_CTRL1 = 0x01;
static const uint8_t NAU7802_CTRL2 = 0x02;
//static const uint8_t NAU7802_I2C_CTRL = 0x11;
static const uint8_t NAU7802_ADCO_B2 = 0x12;
static const uint8_t NAU7802_ADCO_B1 = 0x13;
static const uint8_t NAU7802_ADCO_B0 = 0x14;
//static const uint8_t NAU7802_ADC = 0x15;
//static const uint8_t NAU7802_PGA = 0x1B;
static const uint8_t NAU7802_POWER = 0x1C;
//static const uint8_t NAU7802_REVISION_ID = 0x1F;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C2_Init(void);
static void MX_USART2_UART_Init(void);
/* USER CODE BEGIN PFP */
void ADC_Init(void);
void ADC_Calibrate(void);
uint32_t ADC_Read(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

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
  MX_I2C2_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */
  ADC_Init();
  //ADC_Calibrate();
  char message[12];
  uint32_t value;
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	value = ADC_Read();
	sprintf(message, "VALUE: %d \r\n\n", value);
	HAL_UART_Transmit(&huart2, message, strlen(message), HAL_MAX_DELAY);
	HAL_Delay(100);
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
  * @brief I2C2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C2_Init(void)
{

  /* USER CODE BEGIN I2C2_Init 0 */

  /* USER CODE END I2C2_Init 0 */

  /* USER CODE BEGIN I2C2_Init 1 */

  /* USER CODE END I2C2_Init 1 */
  hi2c2.Instance = I2C2;
  hi2c2.Init.Timing = 0x00000E14;
  hi2c2.Init.OwnAddress1 = 0;
  hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c2.Init.OwnAddress2 = 0;
  hi2c2.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c2) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c2, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c2, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C2_Init 2 */

  /* USER CODE END I2C2_Init 2 */

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
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

}

/* USER CODE BEGIN 4 */
/**
 * Initializes the 24bit ADC
 */
void ADC_Init(void)
{

	//Set the RR bit to 1, to guarantee a reset of all register values
	uint8_t dataBuffer_rr[10] = {0x01};
	HAL_I2C_Mem_Write(&hi2c2, (NAU7802_I2C_ADDR << 1), NAU7802_PU_CTRL, I2C_MEMADD_SIZE_8BIT, dataBuffer_rr, 1, HAL_MAX_DELAY);

	//Set the RR bit to 0, and PUD bit 1, in R0x00, to enter normal operation
	uint8_t dataBuffer_pud[10] = {0x02};
	HAL_I2C_Mem_Write(&hi2c2, (NAU7802_I2C_ADDR << 1), NAU7802_PU_CTRL, I2C_MEMADD_SIZE_8BIT, dataBuffer_pud, 1, HAL_MAX_DELAY);

	//Wait 200 milliseconds
	HAL_Delay(200);

	//Turning up the gain
	uint8_t dataBuffer_gain[10] = {0x22};
	HAL_I2C_Mem_Write(&hi2c2, (NAU7802_I2C_ADDR << 1), NAU7802_CTRL1, I2C_MEMADD_SIZE_8BIT, dataBuffer_gain, 1, HAL_MAX_DELAY);

	//Turning up the sample rate
	uint8_t dataBuffer_samples[10] = {0x30};
	HAL_I2C_Mem_Write(&hi2c2, (NAU7802_I2C_ADDR << 1), NAU7802_CTRL2, I2C_MEMADD_SIZE_8BIT, dataBuffer_samples, 1, HAL_MAX_DELAY);

	//Enables PGA output bypass capacitor connected
	uint8_t dataBuffer_cap[10] = {0x80};
	HAL_I2C_Mem_Write(&hi2c2, (NAU7802_I2C_ADDR << 1), NAU7802_POWER, I2C_MEMADD_SIZE_8BIT, dataBuffer_cap, 1, HAL_MAX_DELAY);

	//Set CS, PUA, and PUD bits to 1
	uint8_t dataBuffer_cs[10] = {0x96};
	HAL_I2C_Mem_Write(&hi2c2, (NAU7802_I2C_ADDR << 1), NAU7802_PU_CTRL, I2C_MEMADD_SIZE_8BIT, dataBuffer_cs, 1, HAL_MAX_DELAY);
}

void ADC_Calibrate(void){

	//Calibrate Gain
	uint8_t dataBuffer_cal_gain[10] = {0x37};
	HAL_I2C_Mem_Write(&hi2c2, (NAU7802_I2C_ADDR << 1), NAU7802_CTRL2, I2C_MEMADD_SIZE_8BIT, dataBuffer_cal_gain, 1, HAL_MAX_DELAY);

	HAL_Delay(200);

	//Calibrate DC Offset
	uint8_t dataBuffer_cal_Offset[10] = {0x36};
	HAL_I2C_Mem_Write(&hi2c2, (NAU7802_I2C_ADDR << 1), NAU7802_CTRL2, I2C_MEMADD_SIZE_8BIT, dataBuffer_cal_Offset, 1, HAL_MAX_DELAY);
}

uint32_t ADC_Read(){

	HAL_StatusTypeDef ret;
	uint8_t buf[12];

	uint32_t value;
	uint8_t MSB, MID, LSB;

	//Read from the MSB of the ADC
	buf[0] = NAU7802_ADCO_B2;
	ret = HAL_I2C_Master_Transmit(&hi2c2, (NAU7802_I2C_ADDR << 1), buf, 1, HAL_MAX_DELAY);
	if (ret != HAL_OK) {
		strcpy((char*)buf, "Error Tx\r\n");
	} else {
		//Read 1 byte from NAU7802_ADCO_B2
		ret = HAL_I2C_Master_Receive(&hi2c2, (NAU7802_I2C_ADDR << 1), buf, 1, HAL_MAX_DELAY);
		if (ret != HAL_OK) {
			//strcpy((char*)buf, "Error Tx\r\n");
		} else {
			MSB = (uint8_t)buf[0];
		}
	}

	//Read the middle byte of the adc
	buf[0] = NAU7802_ADCO_B1;
	ret = HAL_I2C_Master_Transmit(&hi2c2, (NAU7802_I2C_ADDR << 1), buf, 1, HAL_MAX_DELAY);
	if (ret != HAL_OK) {
		//strcpy((char*)buf, "Error Tx\r\n");
	} else {
		//Read 1 byte from NAU7802_ADCO_B2
		ret = HAL_I2C_Master_Receive(&hi2c2, (NAU7802_I2C_ADDR << 1), buf, 1, HAL_MAX_DELAY);
		if (ret != HAL_OK) {
			//strcpy((char*)buf, "Error Tx\r\n");
		} else {
			MID = (uint8_t)buf[0];
		}
	}

	//Read the LSB of the adc
	buf[0] = NAU7802_ADCO_B0;
	ret = HAL_I2C_Master_Transmit(&hi2c2, (NAU7802_I2C_ADDR << 1), buf, 1, HAL_MAX_DELAY);
	if (ret != HAL_OK) {
		//strcpy((char*)buf, "Error Tx\r\n");
	} else {
		//Read 1 byte from NAU7802_ADCO_B2
		ret = HAL_I2C_Master_Receive(&hi2c2, (NAU7802_I2C_ADDR << 1), buf, 1, HAL_MAX_DELAY);
		if (ret != HAL_OK) {
			//strcpy((char*)buf, "Error Tx\r\n");
		} else {
			LSB = (uint8_t)buf[0];
		}
	}

	value = (MSB << 16) | (MID << 8) | (LSB << 0);
	if (value > 16000000) {//Overflow has occurred (since unsigned can't be negative it will overflow to 16777000)
		value = 0;
	}
	return value;
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
