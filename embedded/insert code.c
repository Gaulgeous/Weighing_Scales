//
// Created by gauld on 15/09/2022.
//

#include "wifi.h"

extern I2C_HandleTypeDef hi2c1;
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;

uint16_t lastTick = 0;
uint32_t zero;
uint32_t offset;
extern int uart2_counter;

extern uint8_t UART1RxBuffer[5];
extern uint8_t UART2RxBuffer[1];
extern uint8_t serialMsg[5];
extern uint8_t UART2RxBuf[RX_BUF];
extern uint8_t serialMsg[5];
extern char mainWifiBuf[MAIN_BUF];
extern int mass[13];
extern int adc[13];

initialiseWifi(&huart2);
MQTTConnect(&huart2);

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    //HAL_UART_Transmit_IT(&huart1, 'test', 4);
    if (huart->Instance == USART1) {
        char message[12];
        //float weight;
        //int weight_int;
        uint32_t value;
        //uint32_t offset_value;

        for (int i = 0; i < 5; i++) {
            serialMsg[i] = UART1RxBuffer[i];
            //UART1RxBuffer[i] = 0;
        }

        uint8_t pos = serialMsg[0];
        int input = serialMsg[4] | (serialMsg[3] << 8) | (serialMsg[2] << 16) | (serialMsg[1] << 24);
        if (input != -1) {
            mass[pos] = input;
        }

        if (pos == CALIBRATE) {
            // Calibration function needs to be set up
        } else if (pos == RAW || pos == TARE) {
            value = ADC_Read(&hi2c1);
            adc[pos]= value;
        } else if (pos == REF_COUNT) {
            value = input;
        } else {
            value = adc[pos];
        }


        sprintf(message, "%d %lu\n", pos, value);
        HAL_UART_Transmit(&huart1, (uint8_t*)message, strlen(message), HAL_MAX_DELAY);
        HAL_UART_Receive_IT(&huart1, UART1RxBuffer, 5);

    } else if (huart->Instance == USART2) {

        mainWifiBuf[uart2_counter] = UART2RxBuffer[0];
        //HAL_UART_Transmit(&huart2, &UART2RxBuf[uart2_counter], 1, HAL_MAX_DELAY);
        if (uart2_counter == RX_BUF) {
            memset(mainWifiBuf, 0, uart2_counter);
            uart2_counter = 0;
        }

        if (mainWifiBuf[uart2_counter] == '\n' && mainWifiBuf[uart2_counter - 1] == '\r') {
            //HAL_UART_Transmit(&huart2, "print", 5, HAL_MAX_DELAY);
            if (returnState(huart2, uart2_counter)) {
                memset(mainWifiBuf, 0, uart2_counter);
                uart2_counter = 0;
            }
        }

        uart2_counter++;

        HAL_UART_Receive_IT(&huart2, UART2RxBuffer, 1);
    }
}