#include <string.h>
#include <stdio.h>
#include "L433_ADC.h"
#include "stm32l4xx_hal.h"

/** NOTES **/
//Default strain gauge value is 0.08mV

/** Global Variables **/
//The Zero reading from the ADC (With no weight)
uint32_t zeroWeight = 0;
//Calibrating mass
uint16_t Cal1Mass = 0;
uint16_t Cal2Mass = 0;

/**
 * Initializes the 24bit ADC
 */
void ADC_Init(I2C_HandleTypeDef *hi2c1)
{
	HAL_Init();
	//Set the RR bit to 1, to guarantee a reset of all register values

	uint8_t dataBuffer_rr[10] = {0x01};
	HAL_I2C_Mem_Write(hi2c1, (NAU7802_I2C_ADDR << 1), NAU7802_PU_CTRL, I2C_MEMADD_SIZE_8BIT, dataBuffer_rr, 1, HAL_MAX_DELAY);
	HAL_Delay(10);//Wait 10 ms for everything to reset
	//Set the RR bit to 0, PUD bit 1 and PUA bit 1, in R0x00, to enter normal operation
	uint8_t dataBuffer_pud[10] = {0x06};
	HAL_I2C_Mem_Write(hi2c1, (NAU7802_I2C_ADDR << 1), NAU7802_PU_CTRL, I2C_MEMADD_SIZE_8BIT, dataBuffer_pud, 1, HAL_MAX_DELAY);
	//Wait 10 milliseconds
	HAL_Delay(10);

	//Set gain and set LDO
	//LDO = 3V3 = 0b100
	//Gain = 128 = 0b111
	uint8_t dataBuffer_gain[10] = {0x27};//Gain 128
	HAL_I2C_Mem_Write(hi2c1, (NAU7802_I2C_ADDR << 1), NAU7802_CTRL1, I2C_MEMADD_SIZE_8BIT, dataBuffer_gain, 1, HAL_MAX_DELAY);

	//Turning up the sample rate to 80sps
	uint8_t dataBuffer_samples[10] = {0x30};
	HAL_I2C_Mem_Write(hi2c1, (NAU7802_I2C_ADDR << 1), NAU7802_CTRL2, I2C_MEMADD_SIZE_8BIT, dataBuffer_samples, 1, HAL_MAX_DELAY);

	//Turn off CLK_CHP
	uint8_t dataBuffer_clk[10] = {0x30};
	HAL_I2C_Mem_Write(hi2c1, (NAU7802_I2C_ADDR << 1), NAU7802_ADC, I2C_MEMADD_SIZE_8BIT, dataBuffer_clk, 1, HAL_MAX_DELAY);

	//Enables PGA output bypass capacitor connected
	uint8_t dataBuffer_cap[10] = {0x80};
	HAL_I2C_Mem_Write(hi2c1, (NAU7802_I2C_ADDR << 1), NAU7802_POWER, I2C_MEMADD_SIZE_8BIT, dataBuffer_cap, 1, HAL_MAX_DELAY);

	//Set CS, PUA, and PUD bits to 1
	uint8_t dataBuffer_cs[10] = {0x96};
	HAL_I2C_Mem_Write(hi2c1, (NAU7802_I2C_ADDR << 1), NAU7802_PU_CTRL, I2C_MEMADD_SIZE_8BIT, dataBuffer_cs, 1, HAL_MAX_DELAY);
}

/**
 * Calibrate the internals of the ADC chip
 */
void ADC_Calibrate(I2C_HandleTypeDef *hi2c1){
//	int wait = 1;
//	HAL_StatusTypeDef ret;
//	uint8_t buf[12];
//	uint8_t byte;
//
//	//Calibrate Offset
//	uint8_t dataBuffer_cal[10] = {0x34};
//	HAL_I2C_Mem_Write(hi2c1, (NAU7802_I2C_ADDR << 1), NAU7802_CTRL2, I2C_MEMADD_SIZE_8BIT, dataBuffer_cal, 1, HAL_MAX_DELAY);
//
//	//Wait for calibration of offset to finish
//	wait = 1;
//	while(wait){
//
//		//Read from control register 2
//		buf[0] = NAU7802_CTRL2;
//		ret = HAL_I2C_Master_Transmit(&hi2c1, (NAU7802_I2C_ADDR << 1), buf, 1, HAL_MAX_DELAY);
//		if (ret != HAL_OK) {
//			strcpy((char*)buf, "Error Tx\r\n");
//		} else {
//			//Read 1 byte
//			ret = HAL_I2C_Master_Receive(&hi2c1, (NAU7802_I2C_ADDR << 1), buf, 1, HAL_MAX_DELAY);
//			if (ret != HAL_OK) {
//				//strcpy((char*)buf, "Error Tx\r\n");
//			} else {
//				byte = (uint8_t)buf[0];
//			}
//		}
//		if (byte == 0x30) {
//			break;
//		}
//	}
}


/**
 * Read from the 3 registers of the ADC and then put that into a 32 bit value
 */
uint32_t ADC_Sample(I2C_HandleTypeDef *hi2c1){

	HAL_StatusTypeDef ret;
	uint8_t buf[12];

	uint32_t value;
	uint32_t value_raw;
	uint32_t value_shift;
	uint8_t MSB, MID, LSB;

	//Read from the MSB of the ADC
	buf[0] = NAU7802_ADCO_B2;
	ret = HAL_I2C_Master_Transmit(hi2c1, (NAU7802_I2C_ADDR << 1), buf, 1, HAL_MAX_DELAY);
	if (ret != HAL_OK) {
		strcpy((char*)buf, "Error Tx\r\n");
	} else {
		//Read 1 byte from NAU7802_ADCO_B2
		ret = HAL_I2C_Master_Receive(hi2c1, (NAU7802_I2C_ADDR << 1), buf, 1, HAL_MAX_DELAY);
		if (ret != HAL_OK) {
			//strcpy((char*)buf, "Error Tx\r\n");
		} else {
			MSB = (uint8_t)buf[0];
		}
	}

	//Read the middle byte of the adc
	buf[0] = NAU7802_ADCO_B1;
	ret = HAL_I2C_Master_Transmit(hi2c1, (NAU7802_I2C_ADDR << 1), buf, 1, HAL_MAX_DELAY);
	if (ret != HAL_OK) {
		//strcpy((char*)buf, "Error Tx\r\n");
	} else {
		//Read 1 byte from NAU7802_ADCO_B2
		ret = HAL_I2C_Master_Receive(hi2c1, (NAU7802_I2C_ADDR << 1), buf, 1, HAL_MAX_DELAY);
		if (ret != HAL_OK) {
			//strcpy((char*)buf, "Error Tx\r\n");
		} else {
			MID = (uint8_t)buf[0];
		}
	}

	//Read the LSB of the adc
	buf[0] = NAU7802_ADCO_B0;
	ret = HAL_I2C_Master_Transmit(hi2c1, (NAU7802_I2C_ADDR << 1), buf, 1, HAL_MAX_DELAY);
	if (ret != HAL_OK) {
		//strcpy((char*)buf, "Error Tx\r\n");
	} else {
		//Read 1 byte from NAU7802_ADCO_B2
		ret = HAL_I2C_Master_Receive(hi2c1, (NAU7802_I2C_ADDR << 1), buf, 1, HAL_MAX_DELAY);
		if (ret != HAL_OK) {
			//strcpy((char*)buf, "Error Tx\r\n");
		} else {
			LSB = (uint8_t)buf[0];
		}
	}

	value_raw = (MSB << 16) | (MID << 8) | (LSB << 0);
    value_shift = (int32_t)(value_raw << 8);
    // shift the number back right to recover its intended magnitude
    value = (value_shift >> 8);

	if (value > 16000000) {//Overflow has occurred (since unsigned can't be negative it will overflow to 16777000)
		value = 0;
	}
	return value;
}

uint32_t ADC_Read(I2C_HandleTypeDef *hi2c1){
	uint32_t value = 0;
	int n = 20;

	//Sample n times
	for (int i = 0; i < n; i++) {
		value = value + ADC_Sample(hi2c1);
	}

	value = value/n;

	return value;
}
/**
 * Set setting the zero/tare of the scale, read ADC value and then set the global variable zero_weight
 */
uint32_t ADC_Zero(I2C_HandleTypeDef *hi2c1) {
	HAL_StatusTypeDef ret;
	uint8_t buf[12];

	uint32_t value;
	uint8_t MSB, MID, LSB;

	//Read from the MSB of the ADC
	buf[0] = NAU7802_ADCO_B2;
	ret = HAL_I2C_Master_Transmit(hi2c1, (NAU7802_I2C_ADDR << 1), buf, 1, HAL_MAX_DELAY);
	if (ret != HAL_OK) {
		strcpy((char*)buf, "Error Tx\r\n");
	} else {
		//Read 1 byte from NAU7802_ADCO_B2
		ret = HAL_I2C_Master_Receive(hi2c1, (NAU7802_I2C_ADDR << 1), buf, 1, HAL_MAX_DELAY);
		if (ret != HAL_OK) {
			//strcpy((char*)buf, "Error Tx\r\n");
		} else {
			MSB = (uint8_t)buf[0];
		}
	}
	//Read the middle byte of the adc
	buf[0] = NAU7802_ADCO_B1;
	ret = HAL_I2C_Master_Transmit(hi2c1, (NAU7802_I2C_ADDR << 1), buf, 1, HAL_MAX_DELAY);
	if (ret != HAL_OK) {
		//strcpy((char*)buf, "Error Tx\r\n");
	} else {
		//Read 1 byte from NAU7802_ADCO_B2
		ret = HAL_I2C_Master_Receive(hi2c1, (NAU7802_I2C_ADDR << 1), buf, 1, HAL_MAX_DELAY);
		if (ret != HAL_OK) {
			//strcpy((char*)buf, "Error Tx\r\n");
		} else {
			MID = (uint8_t)buf[0];
		}
	}
		//Read the LSB of the adc
	buf[0] = NAU7802_ADCO_B0;
	ret = HAL_I2C_Master_Transmit(hi2c1, (NAU7802_I2C_ADDR << 1), buf, 1, HAL_MAX_DELAY);
	if (ret != HAL_OK) {
		//strcpy((char*)buf, "Error Tx\r\n");
	} else {
		//Read 1 byte from NAU7802_ADCO_B2
		ret = HAL_I2C_Master_Receive(hi2c1, (NAU7802_I2C_ADDR << 1), buf, 1, HAL_MAX_DELAY);
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

	//Set the Global Variable
	zeroWeight = value;
	return value;
}

/**
 *
 */
void ADC_Cal1_Mass(void) {

}


void ADC_Cal2_Mass(void) {

}


float calc_Weight(uint32_t ADCValue, uint32_t offset) {
	float weight;
	int cal_factor = 405;//Checked reading experimentally
	weight = (ADCValue - offset)/cal_factor;
	return weight;
}

/**
 * Check that the drdy pin is high
 */
int check_drdy(I2C_HandleTypeDef *hi2c1){
	int ready = 0;
	HAL_StatusTypeDef ret;
	uint8_t buf[12];
	uint8_t byte;
	//Read from the PU control register
	buf[0] = NAU7802_PU_CTRL;
	ret = HAL_I2C_Master_Transmit(hi2c1, (NAU7802_I2C_ADDR << 1), buf, 1, HAL_MAX_DELAY);
	if (ret != HAL_OK) {
		strcpy((char*)buf, "Error Tx\r\n");
	} else {
		//Read 1 byte from NAU7802_ADCO_B2
		ret = HAL_I2C_Master_Receive(hi2c1, (NAU7802_I2C_ADDR << 1), buf, 1, HAL_MAX_DELAY);
		if (ret != HAL_OK) {
			//strcpy((char*)buf, "Error Tx\r\n");
		} else {
			byte = (uint8_t)buf[0];
		}
	}
	if (byte & (1 << 5)) {
		ready = 1;
	}
	return ready;
}

uint32_t get_zero_weight(void){
	return zeroWeight;
}

uint16_t get_Cal1Mass(void){
	return Cal1Mass;
}

uint16_t get_Cal2Mass(void){
	return Cal2Mass;
}
