#ifndef INCLUDES
#define INCLUDES
#include "stdio.h"
#include "string.h"
#include "stm32l4xx_hal.h"
#endif

#define RX_BUF 500
#define MAIN_BUF 500

#define RAW 0
#define ZERO 1
#define TARE 2
#define REF_WEIGHT 3
#define PASS1 4
#define PASS2 5
#define CAL1 6
#define CAL2 7
#define REF_COUNT 8
#define CALIBRATE 9
#define M 10
#define C 11
#define CAL_M1 12
#define CAL_M2 13
#define MODE 14
#define SET_ZERO 15
#define MASS 16
#define CANCEL_TARE 17
#define LIGHT 18
#define DONE_CAL 19
#define USER_MODE 20
#define CURRENT_COUNT 21

#define CHECK_TIME 2000
#define CHECK_TIME3 3500

#define WIFI 1
#define PORT 0

#define RST 0
#define CWM 1
#define CWJ 2
#define USE 3
#define CON 4
#define SUB 5

void initialiseWifi(UART_HandleTypeDef* huart1, ScreenData* screenData, TIM_HandleTypeDef* htim2);
void MQTTConnect(UART_HandleTypeDef* huart1, ScreenData* screenData);
int returnState(UART_HandleTypeDef huart1, uint16_t size, ScreenData* screenData, TIM_HandleTypeDef* htim2);
int parse_values(int input, ScreenData* data, int* pos, TIM_HandleTypeDef* htim2);
void get_cip(UART_HandleTypeDef* huart1);
void wifi_bread(UART_HandleTypeDef* huart1, ScreenData* screenData, TIM_HandleTypeDef* htim2);
void wifi_pcb(UART_HandleTypeDef* huart1, ScreenData* screenData, TIM_HandleTypeDef* htim2);
void wifi_pcb2(UART_HandleTypeDef* huart1, ScreenData* screenData, TIM_HandleTypeDef* htim2);
void MQTTSub(UART_HandleTypeDef* huart1, ScreenData* screenData, TIM_HandleTypeDef* htim2);
void MQTTUse (UART_HandleTypeDef* huart1, ScreenData* screenData, TIM_HandleTypeDef* htim2);
void atRst(UART_HandleTypeDef* huart1, ScreenData* screenData);
