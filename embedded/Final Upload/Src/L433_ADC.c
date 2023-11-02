#include <string.h>
#include <stdio.h>
#include "L433_ADC.h"
#include "stm32l4xx_hal.h"
#include "display.h"
#include "ee.h"

/** Global Variables **/
int calFactor = 405;//The Calibration factor. Gets overwritten when read from EEPROM
uint32_t zeroOffset = 0;//The Zero reading from the ADC (With no weight)

/**
 * Initializes the 24bit ADC and EEPROM (non-volatile memory)
 */
void ADC_Init(I2C_HandleTypeDef *hi2c1, ScreenData* data) {
	
	HAL_Init();
	ee_init();//Initialise EEPROM
	
	uint8_t dataReadCal[2];//List to store the calibration factor
	ee_read(0, 2, dataReadCal);//Read from the eeprom bytes 0 and 1
	calFactor = (dataReadCal[0] << 8) | (dataReadCal[1] << 0);//Shift byte 0 and byte 1 into calFactor to get 16bit value

	uint8_t dataReadHigh[2];//List to store high threshold
	ee_read(2, 2, dataReadHigh);//Read from the eeprom bytes 2 and 3
	data->thrH = (dataReadHigh[0] << 8) | (dataReadHigh[1] << 0);

	uint8_t dataReadLow[2];//List to store low threshold
	ee_read(8, 2, dataReadLow);//Read from the eeprom bytes 4 and 5
	data->thrL = (dataReadLow[0] << 8) | (dataReadLow[1] << 0);

	//Set the RR bit to 1, to guarantee a reset of all register values
	uint8_t dataBufferRR[10] = {0x01};
	HAL_I2C_Mem_Write(hi2c1, (NAU7802_I2C_ADDR << 1), NAU7802_PU_CTRL, I2C_MEMADD_SIZE_8BIT, dataBufferRR, 1, HAL_MAX_DELAY);
	HAL_Delay(10);//Wait 10 ms for everything to reset
	//Set the RR bit to 0, PUD bit 1 and PUA bit 1, in R0x00, to enter normal operation
	uint8_t dataBufferPUD[10] = {0x06};
	HAL_I2C_Mem_Write(hi2c1, (NAU7802_I2C_ADDR << 1), NAU7802_PU_CTRL, I2C_MEMADD_SIZE_8BIT, dataBufferPUD, 1, HAL_MAX_DELAY);
	//Wait 10 milliseconds
	HAL_Delay(10);

	//Set gain and set LDO
	//LDO = 3V3 = 0b100
	//Gain = 128 = 0b111
	uint8_t dataBufferGain[10] = {0x27};//Gain 128
	HAL_I2C_Mem_Write(hi2c1, (NAU7802_I2C_ADDR << 1), NAU7802_CTRL1, I2C_MEMADD_SIZE_8BIT, dataBufferGain, 1, HAL_MAX_DELAY);

	//Turning up the sample rate to 80sps
	uint8_t dataBufferSamples[10] = {0x30};
	HAL_I2C_Mem_Write(hi2c1, (NAU7802_I2C_ADDR << 1), NAU7802_CTRL2, I2C_MEMADD_SIZE_8BIT, dataBufferSamples, 1, HAL_MAX_DELAY);

	//Turn off CLK_CHP
	uint8_t dataBufferCLK[10] = {0x30};
	HAL_I2C_Mem_Write(hi2c1, (NAU7802_I2C_ADDR << 1), NAU7802_ADC, I2C_MEMADD_SIZE_8BIT, dataBufferCLK, 1, HAL_MAX_DELAY);

	//Enables PGA output bypass capacitor connected
	uint8_t dataBufferCap[10] = {0x80};
	HAL_I2C_Mem_Write(hi2c1, (NAU7802_I2C_ADDR << 1), NAU7802_POWER, I2C_MEMADD_SIZE_8BIT, dataBufferCap, 1, HAL_MAX_DELAY);

	//Set CS, PUA, and PUD bits to 1
	uint8_t dataBufferCS[10] = {0x96};
	HAL_I2C_Mem_Write(hi2c1, (NAU7802_I2C_ADDR << 1), NAU7802_PU_CTRL, I2C_MEMADD_SIZE_8BIT, dataBufferCS, 1, HAL_MAX_DELAY);
}

/**
 * Read from the 3 registers of the ADC and then put that into a 32 bit value
 */
uint32_t ADC_Sample(I2C_HandleTypeDef *hi2c1) {

	HAL_StatusTypeDef ret;
	uint8_t buf[12];

	uint32_t value;
	uint32_t valueRaw;
	uint32_t valueShift;
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
			//Do Nothing
		} else {
			MSB = (uint8_t)buf[0];
		}
	}

	//Read the middle byte of the adc
	buf[0] = NAU7802_ADCO_B1;
	ret = HAL_I2C_Master_Transmit(hi2c1, (NAU7802_I2C_ADDR << 1), buf, 1, HAL_MAX_DELAY);
	if (ret != HAL_OK) {
		//Do Nothing
	} else {
		//Read 1 byte from NAU7802_ADCO_B2
		ret = HAL_I2C_Master_Receive(hi2c1, (NAU7802_I2C_ADDR << 1), buf, 1, HAL_MAX_DELAY);
		if (ret != HAL_OK) {
			//Do Nothing
		} else {
			MID = (uint8_t)buf[0];
		}
	}

	//Read the LSB of the adc
	buf[0] = NAU7802_ADCO_B0;
	ret = HAL_I2C_Master_Transmit(hi2c1, (NAU7802_I2C_ADDR << 1), buf, 1, HAL_MAX_DELAY);
	if (ret != HAL_OK) {
		//Do Nothing
	} else {
		//Read 1 byte from NAU7802_ADCO_B2
		ret = HAL_I2C_Master_Receive(hi2c1, (NAU7802_I2C_ADDR << 1), buf, 1, HAL_MAX_DELAY);
		if (ret != HAL_OK) {
			//Do Nothing
		} else {
			LSB = (uint8_t)buf[0];
		}
	}

	valueRaw = (MSB << 16) | (MID << 8) | (LSB << 0);
    valueShift = (int32_t)(valueRaw << 8);
    // shift the number back right to recover its intended magnitude
    value = (valueShift >> 8);

	if (value > 16000000) {//Overflow has occurred (since unsigned can't be negative it will overflow to 16777000)
		value = 0;
	}
	
	return value;
}

/**
 * ADC_Read creates a more stable reading as it will take multiple 
 * readings from the ADC and return the average of the readings.
 */
uint32_t ADC_Read(I2C_HandleTypeDef *hi2c1){
	
	uint32_t value = 0;
	int n = 20;//Number of times to read from the ADC

	//Sample n times
	for (int i = 0; i < n; i++) {
		value = value + ADC_Sample(hi2c1);
	}

	value = value/n;//Calculate the average

	return value;
}

/**
 * From the ADC value calculate the mass in grams
 */
float calc_Weight(uint32_t ADCValue) {
	
	float weight;
	
	weight = ((int)ADCValue - (int)zeroOffset)/calFactor;//y=mx+c -> x = y-c/m
	
	return weight;
}

/**
 * Calibration process takes the calibration weights and assigns them to a struct.
 * It also calculates a new calibration factor and stores it in non-volatile memory.
 */
void calibration_process(ScreenData* data) {

	int calWeight2 = data->calWeight2;//Assign calibration raw adc reading to struct
	int calWeight3 = data->calWeight3;//Assign calibration raw adc reading to struct
	
	int cal2 = data->cal2;//Assign calibration known weight to struct
	int cal3 = data->cal3;//Assign calibration known weight to struct
	
	//y=mx+c -> m = y2-y1/x2-x1
	calFactor = (int)((calWeight2 - calWeight3)/(cal2-cal3));//Calculate the new calibration factor
	
	uint8_t first = (calFactor >> 8) & 0xFF;//Bit shift the 16 bit value into two 8 bit values
	uint8_t second = (calFactor >> 0) & 0xFF;
	uint8_t dataWriteCal[2] = {first, second};//Store in a list
	ee_writeToRam(0, 2, dataWriteCal);//Write to EEPROM
	ee_commit();//Commit
}

/**
 * Store pass/fail mode high threshold in non-volatile memory.
 */
void save_high_threshold(ScreenData* data) {

	int high = data->thrH;//Assign high threshold weight to struct
	uint8_t first = (high >> 8) & 0xFF;//Bit shift the 16 bit value into two 8 bit values
	uint8_t second = (high >> 0) & 0xFF;
	uint8_t dataWriteHigh[2] = {first, second};//Store in a list
	ee_writeToRam(2, 2, dataWriteHigh);//Write to EEPROM
	ee_commit();//Commit
}

/**
 * Store pass/fail mode low threshold in non-volatile memory.
 */
void save_low_threshold(ScreenData* data) {

	int low = data->thrL;//Assign low threshold weight to struct
	uint8_t first = (low >> 8) & 0xFF;//Bit shift the 16 bit value into two 8 bit values
	uint8_t second = (low >> 0) & 0xFF;
	uint8_t dataWriteLow[2] = {first, second};//Store in a list
	ee_writeToRam(8, 2, dataWriteLow);//Write to EEPROM
	ee_commit();//Commit
}

/**
 * When called upon, this function overwrites the zero offset global variable
 */
void store_offset(uint32_t offset) {

	zeroOffset = offset;
}

/**
 * When called upon, this function returns the calibration factor global variable
 */
int return_cal_factor(void) {

	return calFactor;
}

/**
 * When called upon, this function returns the zero offset global variable
 */
uint32_t return_zero_offset(void) {

	return zeroOffset;
}
