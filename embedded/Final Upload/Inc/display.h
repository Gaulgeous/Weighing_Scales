#ifndef __DISPLAY_H__
#define __DISPLAY_H__

#include "ili9163.h"
#include "math.h"

#define WEIGHTING 	11
#define COUNTING 	12
#define PASS 		13
#define CALIBRATION 14
#define MENU 		21
#define KEYBOARD 	22

typedef struct ScreenData {

	// X LCD coordinate
	uint8_t xValue;
	// Y LCD coordinate
	uint8_t yValue;
	// Setup the duty cycle for PWM
	uint8_t dutyCycle;
	// display frame
	uint8_t frame;
	// reference number
	uint16_t reference;
	// thresholds high
	uint16_t thrH;
	// threshold low
	uint16_t thrL;
	// pervious mode
	uint8_t preMode;
	// calibration 1
	uint16_t cal1;
	// calibration 2
	uint16_t cal2;
	// calibration 3
	uint16_t cal3;
	// calibration number
	uint8_t calNum;
	// tare state
	uint8_t tareState;
	// thr value
	uint8_t thr;
	// weight data
	int weight;
	// cal weight
	uint32_t calWeight1;
	// cal weight
	uint32_t calWeight2;
	// cal weight
	uint32_t calWeight3;
	// cal process number
	uint8_t calSetNum;
	// cal1 confirm state
	uint8_t cal1Confirm;
	// cal1 confirm state
	uint8_t cal2Confirm;
	// cal1 confirm state
	uint8_t cal3Confirm;
	// count value
	int count;
	// single value
	float single;
	// Tare value
	uint16_t tareValue;
	// raw adc value
	uint32_t rawADC;
	// confirm for calculate calibration
	uint8_t calculateCal;
	// tare weight
	int tareWeight;
	// check for connection
	uint8_t connection;
	// cal indicator
	uint8_t cal_indicator;
	// cal START indicator
	uint8_t calStart;
}ScreenData;

void init_screen(ScreenData* data);

void mode_select(void);

void weighting_mode(ScreenData* data);

void counting_mode(ScreenData *data);

void pass_mode(ScreenData *data);

void keyboard(ScreenData *data);

void calibration(ScreenData *Data);

void draw_custom_icon(uint8_t x, uint8_t y, int type);

void brightness_button(void);

uint16_t keyboard_process(ScreenData* data, uint16_t value);

void touch_process(TIM_HandleTypeDef *htim1, ScreenData *data);

void calibration_process(ScreenData* data);

void change_brightness(TIM_HandleTypeDef *htim2, ScreenData *data, int type);

#endif // __DISPLAY_H__
