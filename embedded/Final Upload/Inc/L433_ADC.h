#ifndef SRC_L433_ADC_H_
#define SRC_L433_ADC_H_

#include <string.h>
#include <stdio.h>
#include "stm32l4xx_hal.h"
#include "display.h"

/** DEFINES **/
#define NAU7802_I2C_ADDR 0x2A //Fixed address for NAU7802
#define NAU7802_PU_CTRL 0x00
#define NAU7802_CTRL1 0x01
#define NAU7802_CTRL2 0x02
#define NAU7802_ADCO_B2 0x12
#define NAU7802_ADCO_B1 0x13
#define NAU7802_ADCO_B0 0x14
#define NAU7802_POWER 0x1C
//#define NAU7802_REVISION_ID 0x1F
//#define NAU7802_I2C_CTRL 0x11
#define NAU7802_ADC 0x15
//#define NAU7802_PGA 0x1B

#define MASS_KILOGRAMS 1000
#define MASS_GRAMS 1
#define MASS_MILLIGRAMS 0.001

/** Functions **/
void ADC_Init(I2C_HandleTypeDef *hi2c1, ScreenData* data);
uint32_t ADC_Sample(I2C_HandleTypeDef *hi2c1);
uint32_t ADC_Read(I2C_HandleTypeDef *hi2c1);
float calc_Weight(uint32_t ADCValue);
void calibration_process(ScreenData* data);
void store_offset(uint32_t offset);
int return_cal_factor(void);
uint32_t return_zero_offset(void);
void save_low_threshold(ScreenData* data);
void save_high_threshold(ScreenData* data);

#endif /* SRC_L433_ADC_H_ */
