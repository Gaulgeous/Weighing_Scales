#include "display.h"
#include "L433_ADC.h"
#include "wifi.h"

I2C_HandleTypeDef hi2c1;

uint8_t wifiData[5] = {0};

char mainWifiBuf[MAIN_BUF];
uint8_t errorFlag = 0;
uint8_t readyFlag = 0;
uint8_t connected = 0;
int uart2Pos = 0;
int mass[16] = {0};
int adc[16] = {0};
int mode = 0;
int currentTime = 0;
int previousTime = 0;
extern uint8_t guiFlag;
int uart2Counter = 0;
uint8_t wifiSentStat[6] = {0};
uint8_t wifiStat[6] = {0};


/*
 * Function for resetting the ESP32 to its basic state
 */
void atRst(UART_HandleTypeDef* huart1, ScreenData* screenData) {

	if (!wifiSentStat[RST]) {
		readyFlag = 0;
		char resetSig[15] = "AT+RST\r\n";
		HAL_UART_Transmit(huart1, (uint8_t*)resetSig, strlen(resetSig), HAL_MAX_DELAY);
		wifiSentStat[RST] = 1;
		previousTime = HAL_GetTick();

	} else {

		currentTime = HAL_GetTick();
		if (currentTime - previousTime > CHECK_TIME3) {
			wifiStat[RST] = 1;
			readyFlag = 1;
		}
	}
}

/*
 * Used to initialise connection to wifi
 * Sends message for ESP to configure to station mode, then to connect to UQ network
 */
void initialiseWifi(UART_HandleTypeDef* huart1, ScreenData* screenData, TIM_HandleTypeDef* htim2)
{
	if (!wifiSentStat[CWM]) {

		readyFlag = 0;
		char station[15] = "AT+CWMODE=1\r\n";
		HAL_UART_Transmit(huart1, (uint8_t*)station, strlen(station), HAL_MAX_DELAY);
		wifiSentStat[CWM] = 1;

	} else {

		if (readyFlag) {
			wifiStat[CWM] = 1;

		} else {

		// Wait for the OK signal to return
			currentTime = HAL_GetTick();
			if (currentTime - previousTime > CHECK_TIME) {
				returnState(*huart1, uart2Counter, screenData, htim2);
				previousTime = currentTime;
			}
		}
	}
}

/*
 * Function for connecting to the UQ internet for the breadboard esp
 * Each esp requires different login information
 */
void wifi_bread(UART_HandleTypeDef* huart1, ScreenData* screenData, TIM_HandleTypeDef* htim2) {

	if (!wifiSentStat[CWJ]) {
		readyFlag = 0;
		char wifiConnect[44] = "AT+CWJAP=\"infrastructure\",\"DT8Y2wg-9uvX\"\r\n";
		HAL_UART_Transmit(huart1, (uint8_t*)wifiConnect, strlen(wifiConnect), HAL_MAX_DELAY);
		wifiSentStat[CWJ] = 1;

	} else {

		if (readyFlag) {
			wifiStat[CWJ] = 1;

		} else {
			// Wait for the OK signal to return
			currentTime = HAL_GetTick();
			if (currentTime - previousTime > CHECK_TIME) {
				returnState(*huart1, uart2Counter, screenData, htim2);
				previousTime = currentTime;
			}
		}
	}
}

/*
 * Function for extracting the mac address from a new esp
 * This is used to initalise the esp and connect it to wifi
 */
void get_cip(UART_HandleTypeDef* huart1) {

	char userCfg[90] = "AT+CIPSTAMAC?\r\n";
	HAL_UART_Transmit(huart1, (uint8_t*)userCfg, strlen(userCfg), HAL_MAX_DELAY);
}

/*
 * Connects the PCB on the first esp to the UQ internet
 * Each esp requires separate login information, as it operates on a separate chip
 */
void wifi_pcb2(UART_HandleTypeDef* huart1, ScreenData* screenData, TIM_HandleTypeDef* htim2) {

	if (!wifiSentStat[CWJ]) {
		readyFlag = 0;
		char wifiConnect[44] = "AT+CWJAP=\"infrastructure\",\"64FeUCeERNRf\"\r\n";
		HAL_UART_Transmit(huart1, (uint8_t*)wifiConnect, strlen(wifiConnect), HAL_MAX_DELAY);
		wifiSentStat[CWJ] = 1;

	} else {

		if (readyFlag) {
			wifiStat[CWJ] = 1;

		} else {
		// Wait for the OK signal to return
			currentTime = HAL_GetTick();
			if (currentTime - previousTime > CHECK_TIME) {
				returnState(*huart1, uart2Counter, screenData, htim2);
				previousTime = currentTime;
			}
		}
	}
}

/*
 * Connects the PCB on the first esp to the UQ internet
 * Each esp requires separate login information, as it operates on a separate chip
 */
void wifi_pcb(UART_HandleTypeDef* huart1, ScreenData* screenData, TIM_HandleTypeDef* htim2) {

	if (!wifiSentStat[CWJ]) {
		readyFlag = 0;
		char wifiConnect[44] = "AT+CWJAP=\"infrastructure\",\"hLfbVTuZCd_U\"\r\n";
		HAL_UART_Transmit(huart1, (uint8_t*)wifiConnect, strlen(wifiConnect), HAL_MAX_DELAY);
		wifiSentStat[CWJ] = 1;

	} else {

		if (readyFlag) {
			wifiStat[CWJ] = 1;

		} else {
		// Wait for the OK signal to return
			currentTime = HAL_GetTick();
			if (currentTime - previousTime > CHECK_TIME) {
				returnState(*huart1, uart2Counter, screenData, htim2);
				previousTime = currentTime;
			}
		}
	}
}


/*
 * Sets the user configuration details for when the esp connects to the MQTT client
 * waits one second to check if the connection was already established
 */
void MQTTUse (UART_HandleTypeDef* huart1, ScreenData* screenData, TIM_HandleTypeDef* htim2) {
	// Wait for a second to see if MQTT session was pre-established
	HAL_Delay(1000);

	if (!connected) {
		if (!wifiSentStat[USE]) {

			readyFlag = 0;
			char userCfg[90] = "AT+MQTTUSERCFG=0,7,\"mightwork\",\"team20\",\"8c6d8944a624703e502824ed\",0,0,\"/ws\"\r\n";
			HAL_UART_Transmit(huart1, (uint8_t*)userCfg, strlen(userCfg), HAL_MAX_DELAY);
			wifiSentStat[USE] = 1;

		} else {

			if (readyFlag) {
				wifiStat[USE] = 1;

			} else {
			// Wait for the OK signal to return
				currentTime = HAL_GetTick();
				if (currentTime - previousTime > CHECK_TIME) {
					returnState(*huart1, uart2Counter, screenData, htim2);
					previousTime = currentTime;
				}
			}
		}
	}
}

/*
 * Subscribes to the team topic associated with messages from the GUI
 */
void MQTTSub(UART_HandleTypeDef* huart1, ScreenData* screenData, TIM_HandleTypeDef* htim2) {
	// Wait until system is connected before continuing

	if (connected) {

		if (!wifiSentStat[SUB]) {
			readyFlag = 0;
			char subscribe[30] = "AT+MQTTSUB=0,\"team20/GUI\",0\r\n";
			HAL_UART_Transmit(huart1, (uint8_t*)subscribe, strlen(subscribe), HAL_MAX_DELAY);
			wifiSentStat[SUB] = 1;

		} else {

			if (readyFlag) {
				wifiStat[SUB] = 1;

			} else {
				// Wait for the OK signal to return
				currentTime = HAL_GetTick();
				if (currentTime - previousTime > CHECK_TIME) {
					returnState(*huart1, uart2Counter, screenData, htim2);
					previousTime = currentTime;
				}
			}
		}
	}
}

/*
 * Connects the esp to the MQTT client
 */
void MQTTConnect(UART_HandleTypeDef* huart1, ScreenData* screenData)
{
	if (!wifiSentStat[CON]) {

		readyFlag = 0;
		char connect[45] = "AT+MQTTCONN=0,\"tp-mqtt.uqcloud.net\",443,1\r\n";
		HAL_UART_Transmit(huart1, (uint8_t*)connect, strlen(connect), HAL_MAX_DELAY);
		wifiSentStat[CON] = 1;
		previousTime = HAL_GetTick();

	} else {

		currentTime = HAL_GetTick();
		if (currentTime - previousTime > CHECK_TIME3) {
			wifiStat[CON] = 1;
			readyFlag = 1;
		}
	}
}

/*
 * Helper function for parsing the information from transmitted signals, before  sending a
 * response. This is in charge of evaluating if a raw adc reading is required
 */
int guiFlagHelper(int input, ScreenData* data, int* pos, TIM_HandleTypeDef* htim2) {

	int retValue = 0;

	switch(guiFlag) {
		case LIGHT:
			*pos = LIGHT;
			retValue = data->dutyCycle / 5;
			break;

		case ZERO:
			*pos = ZERO;
			retValue = data->calWeight1;
			break;

		case TARE:
			*pos = TARE;
			retValue = data->tareValue;
			break;

		case REF_WEIGHT:
			*pos = REF_WEIGHT;
			retValue = data->single * data->reference;
			break;

		case PASS1:
			*pos = PASS1;
			retValue = data->thrL;
			break;

		case PASS2:
			*pos = PASS2;
			retValue = data->thrH;
			break;

		case REF_COUNT:
			*pos = REF_COUNT;
			retValue = data->reference;
			break;

		case MODE:
			*pos = MODE;
			retValue = data->connection;
			break;

		case CAL1:
			*pos = CAL1;
			retValue = data->calWeight2;
			break;

		case CAL2:
			*pos = CAL2;
			retValue = data->calWeight3;
			break;

		case CAL_M1:
			*pos = CAL_M1;
			retValue = data->cal2;
			break;

		case CAL_M2:
			*pos = CAL_M2;
			retValue = data->cal3;
			break;

		case USER_MODE:
			*pos = USER_MODE;
			if (data->frame == CALIBRATION) {
				retValue = 3;
			} else if (data->frame == COUNTING) {
				retValue = 1;
			} else if (data->frame == PASS) {
				retValue = 2;
			} else {
				retValue = 0;
			}
			break;

		case CALIBRATE:
			*pos = DONE_CAL;
			retValue = 1;
			break;

		default:
			retValue = data->rawADC;
			break;
	}

	guiFlag = 0;
	return retValue;
}

/*
 * First helper function for parsing the information from transmitted signals, before  sending a
 * response
 */
int parse_helper1(int input, ScreenData* data, int* pos, TIM_HandleTypeDef* htim2) {

	int retValue = 0;

	switch(*pos) {

		case RAW:
			retValue = guiFlagHelper(input, data, pos, htim2);
			break;

		case ZERO:
			retValue = data->rawADC;
			break;

		case TARE:
			if (input == 0) {
				retValue = data->tareValue;
			} else {
				retValue = data->rawADC;
				data->tareValue = (int)calc_Weight(retValue);
				data->tareState = 1;
			}

			break;

		case REF_WEIGHT:

			retValue = input;
			if (input != -1) {
				data->single = round(data->weight / data->reference);
				data->count = round(data->weight/data->single);
			} else {
				retValue = data->reference;
			}
			break;

		case PASS1:

			retValue = input;
			if (input != -1) {
				data->thrL = input & 0xFFFF;
			} else {
				retValue = data->thrL;
			}
			break;

		case PASS2:

			retValue = input;
			if (input != -1) {
				data->thrH = input & 0xFFFF;
			} else {
				retValue = data->thrH;
			}
			break;

		case CAL1:

			retValue = data->rawADC;
			data->calWeight2 = retValue;
			//data->cal2Confirm = 1;
			if (input != -1) {
				data->cal2 = input & 0xFFFF;
			}
			break;

		case CAL2:

			retValue = data->rawADC;
			data->calWeight3 = retValue;
			//data->cal3Confirm = 1;
			if (input != -1) {
				data->cal3 = input & 0xFFFF;
			}
			break;
	}

	return retValue;
}

/*
 * Second helper function for parsing the information from transmitted signals, before  sending a
 * response
 */
int parse_helper2(int input, ScreenData* data, int* pos, TIM_HandleTypeDef* htim2) {

	int retValue = 0;

	switch(*pos) {

		case REF_COUNT:

			retValue = input;
			if (input != -1) {
				data->reference = input & 0xFFFF;
			} else {
				retValue = data->count;
			}
			break;

		case CALIBRATE:

			calibration_process(data);
			retValue = 1;
			break;

		case M:

			retValue = return_cal_factor();
			break;

		case C:

			retValue = return_zero_offset();
			break;

		case CAL_M1:

			retValue = data->cal2;
			break;

		case CAL_M2:

			retValue = data->cal3;
			break;

		case MODE:
			retValue = input;
			data->connection = input;
			break;


	}

	return retValue;
}

/*
 * Third helper function for parsing the information from transmitted signals, before  sending a
 * response
 */
int parse_helper3(int input, ScreenData* data, int* pos, TIM_HandleTypeDef* htim2) {

	int retValue = 0;

	switch(*pos) {

		case MASS:

			if (data->weight >= 2110) {
				retValue = 99999;
			} else if (data->tareState) {
				retValue = data->tareWeight;
			} else {
				retValue = data->weight;
			}
			break;

		case SET_ZERO:

			retValue = data->rawADC;
			data->calWeight1 = retValue;
			//data->cal1Confirm = 1;
			break;

		case DONE_CAL:
			retValue = data->cal_indicator;
			break;

		case CANCEL_TARE:
			data->tareState = 0;
			break;

		case LIGHT:
			if (input == 0) {
				changeBrightness(htim2, data, 1);
			} else if (input == 1) {
				changeBrightness(htim2, data, 0);
			}
			retValue = data->dutyCycle / 5;

			break;

		case USER_MODE:
			if (input == 3) {
				data->frame = CALIBRATION;
				retValue = 3;

			} else if (input == 1) {
				data->frame = COUNTING;
				retValue = 1;

			}else if (input == 2) {
				data->frame = PASS;
				retValue = 2;

			} else if (input == 0) {
				data->frame = WEIGHTING;

			} else if (input == -1){
				if (data->frame == CALIBRATION) {
					retValue = 3;
				} else if (data->frame == COUNTING) {
					retValue = 1;
				} else if (data->frame == PASS) {
					retValue = 2;
				} else if (data->frame == WEIGHTING) {
					retValue = 0;
				}
			}
			break;

		case CURRENT_COUNT:
			retValue = data->count;
			break;

	}

	return retValue;
}

/*
 * Extract the values from each input, before returning the value that's needed by GUI
 */
int parse_values(int input, ScreenData* data, int* pos, TIM_HandleTypeDef* htim2) {

	int retValue = 0;

	if (*pos <= CAL2) {
		retValue = parse_helper1(input, data, pos, htim2);
	} else if (*pos > CAL2 && *pos <= MODE) {
		retValue = parse_helper2(input, data, pos, htim2);
	} else {
		retValue = parse_helper3(input, data, pos, htim2);
	}

	return retValue;
}


/*
 * Evaluates the incoming response from ESP module
 * Readies STM for new transmission upon OK
 * Triggers error flag upon ERROR
 * Enables wifi sending upon CONNECT
 * Disables wifi sending upon DISCONNECT
 * analyses incoming data, and returns message upon MQTTSUBREC
 */
int returnState(UART_HandleTypeDef huart1, uint16_t size, ScreenData* screenData, TIM_HandleTypeDef* htim2) {
	int completed = 0;

	for (int x = 0; x < size; x++) {
		// OK message
		if (mainWifiBuf[x] == 'O' && mainWifiBuf[x + 1] == 'K') {
			readyFlag = 1;
			completed = 1;

		// ERROR message
		} else if (mainWifiBuf[x] == 'E' && mainWifiBuf[x + 1] == 'R' && mainWifiBuf[x + 2] == 'R' && mainWifiBuf[x + 3] == 'O' && mainWifiBuf[x + 4] == 'R') {
			errorFlag = 1;
			// Will have to change this transmission eventually
			readyFlag = 1;
			completed = 1;

		// DISCONNECT message
		} else if (mainWifiBuf[x] == 'M' && mainWifiBuf[x + 1] == 'Q' && mainWifiBuf[x + 2] == 'T' && mainWifiBuf[x + 3] == 'T' && mainWifiBuf[x + 4] == 'D' && mainWifiBuf[x + 5] == 'I' && mainWifiBuf[x + 6] == 'S' && mainWifiBuf[x + 7] == 'C' && mainWifiBuf[x + 8] == 'O' && mainWifiBuf[x + 9] == 'N' && mainWifiBuf[x + 10] == 'N' && mainWifiBuf[x + 11] == 'E' && mainWifiBuf[x + 12] == 'C' && mainWifiBuf[x + 13] == 'T') {
			connected = 0;
			completed = 1;

		// CONNECT message
		} else if (mainWifiBuf[x] == 'M' && mainWifiBuf[x + 1] == 'Q' && mainWifiBuf[x + 2] == 'T' && mainWifiBuf[x + 3] == 'T' && mainWifiBuf[x + 4] == 'C' && mainWifiBuf[x + 5] == 'O' && mainWifiBuf[x + 6] == 'N' && mainWifiBuf[x + 7] == 'N' && mainWifiBuf[x + 8] == 'E' && mainWifiBuf[x + 9] == 'C' && mainWifiBuf[x + 10] == 'T') {
			connected = 1;
			completed = 1;
			readyFlag = 1;

		// RECV message
		} else if (mainWifiBuf[x] == 'M' && mainWifiBuf[x + 1] == 'Q' && mainWifiBuf[x + 2] == 'T' && mainWifiBuf[x + 3] == 'T' && mainWifiBuf[x + 4] == 'S' && mainWifiBuf[x + 5] == 'U' && mainWifiBuf[x + 6] == 'B' && mainWifiBuf[x + 7] == 'R' && mainWifiBuf[x + 8] == 'E' && mainWifiBuf[x + 9] == 'C' && mainWifiBuf[x + 10] == 'V') {

			int counter = 0;
			int read_pos = 0;
			int space_pos = 0;
			while (!(mainWifiBuf[counter] == '\r' && mainWifiBuf[counter + 1] == '\n')) {
				if (mainWifiBuf[counter] == ',') {
					read_pos = counter;
				} else if (mainWifiBuf[counter] == ' ') {
					space_pos = counter;
				}
				counter++;
			}

			read_pos++;
			int value = 0;
			int pos = 0;

			for (int i = read_pos; i < space_pos; i++) {

				if (mainWifiBuf[i] >= '0' && mainWifiBuf[i] <= '9') {
					pos = pos * 10 + mainWifiBuf[i] - '0';
				}
			}

			for (int i = space_pos; i < counter; i++) {

				if (mainWifiBuf[i] >= '0' && mainWifiBuf[i] <= '9') {
					value = value * 10 + mainWifiBuf[i] - '0';
				} else if (mainWifiBuf[i] == '-') {
					value = value * -1;
				}
			}

			int retVal = parse_values(value, screenData, &pos, htim2);

			char mqttPublish[50];
			sprintf(mqttPublish, "AT+MQTTPUB=0,\"team20/ESP\",\"%d %d\",0,0\r\n", pos, retVal);
			HAL_UART_Transmit(&huart1, (uint8_t*)mqttPublish, strlen(mqttPublish), HAL_MAX_DELAY);
			completed = 1;
		}
	}

	return completed;
}

/*
 * Callback used for completion of tx transmissions
 * Used in debugging only
 */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
	__NOP();
}

