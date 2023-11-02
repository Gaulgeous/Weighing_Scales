#include "display.h"
#include "ili9163.h"
#include "L433_ADC.h"

void init_screen(ScreenData* data) {

	data->xValue = 0;
	data->yValue = 0;
	data->dutyCycle = 25;
	data->frame = WEIGHTING;
	data->reference = 0;
	data->thrH = 1000;
	data->thrL = 0;
	data->preMode = PASS;
	data->cal1 = 0;
	data->cal2 = 10;
	data->cal3 = 100;
	data->cal_num = 1;
	data->tareState = 0;
	data->thr = 0;
	data->weight = 2850;
	data->calWeight1 = 0;
	data->calWeight2 = 0;
	data->calWeight3 = 0;
	data->calSetNum = 1;
	data->cal1Confirm = 0;
	data->cal2Confirm = 0;
	data->cal3Confirm = 0;
	data->count = 0;
	data->single = 0;
	data->tareValue = 0;
	data->rawADC = 0;
	data->calculateCal = 0;

}

void draw_number(uint8_t x, uint8_t y, FontDef font, uint16_t color, int value) {

	char buffer[12];
	sprintf(buffer, "%d%s", value, "g");
	ILI9163_drawString(x, y, font, color, buffer);

}

void write_number(uint8_t x, uint8_t y, FontDef font, uint16_t color, int value) {

	char buffer[12];
	sprintf(buffer, "%d", value);
	ILI9163_drawString(x, y, font, color, buffer);

}
/////////////////////////////////////////////////////////////////////////
//void draw_float(uint8_t x, uint8_t y, FontDef font, uint16_t color, int value) {
//	char buffer[12];
//	sprintf(buffer, "%2f", value);
//	ILI9163_drawString(x, y, font, color, buffer);
//}
///////////////////////////////////////////////////////////////////////////

void mode_select(void) {

	ILI9163_drawRect(0, 0, 80, 54, 1, RED);
	ILI9163_drawRect(0, 54, 80, 110, 1, RED);
	ILI9163_drawRect(80, 0, 160, 54, 1, RED);
	ILI9163_drawRect(79, 54, 160, 110, 1, RED);
	ILI9163_drawString(2, 20, Font_7x10, BLACK, "WEIGHTING");
	ILI9163_drawString(82, 20, Font_7x10, BLACK, "COUNTING");
	ILI9163_drawString(2, 74, Font_7x10, BLACK, "PASS/FAIL");
	ILI9163_drawString(82, 74, Font_7x10, BLACK, "CALIBRATION");

}

void weighting_mode(ScreenData *data) {

	char lightBuffer[12];
	sprintf(lightBuffer, "%s %d", "BL:", data->dutyCycle/5);
	ILI9163_drawString(10, 15, Font_7x10, BLACK, lightBuffer);

	ILI9163_drawRect(0, 30, 120, 110, 2, RED);
	//draw_number(10, 70, Font_7x10, BLACK, data->weight);
	ILI9163_drawString(10, 5, Font_7x10, BLACK, "WEIGHTING MODE");
	ILI9163_drawRect(120, 90, 160, 110, 2, RED);
	ILI9163_drawString(122, 95, Font_7x10, RED, "Tare");
	ILI9163_drawRect(120, 30, 160, 50, 1, RED);
	ILI9163_drawString(125, 35, Font_7x10, RED, "N-T");
	if (data->tareState) {

		draw_custom_icon(105, 95, 5);
		int weight = data->weight;
		int tare = data->tareValue;
		int now = weight - tare;

		if (data->weight < 2110) {

			draw_number(10, 70, Font_7x10, BLACK, now);

		} else {

			ILI9163_drawString(10, 70, Font_7x10, BLACK, "OL");
		}
	} else {

		if(data->weight < 2110) {
			draw_number(10, 70, Font_7x10, BLACK, data->weight);
		} else {
			ILI9163_drawString(10, 70, Font_7x10, BLACK, "OL");
		}
	}
}

void counting_mode(ScreenData *data) {

	char buffer[12];
	sprintf(buffer, "%d %s", data->count, "count(s)");
	ILI9163_drawString(10, 85, Font_7x10, BLACK, buffer);

	char buf[12];
	////////////////////////////////////////////////////
	sprintf(buf, "%s %.2f", "Single:",data->single);
	////////////////////////////////////////////////////
	ILI9163_drawString(10, 95, Font_7x10, BLACK, buf);

	ILI9163_drawRect(0, 30, 120, 110, 2, RED);
	//draw_number(10, 70, Font_7x10, BLACK, data->weight);
	ILI9163_drawString(10, 5, Font_7x10, BLACK, "COUNTING MODE");
	ILI9163_drawRect(120, 90, 160, 110, 1, RED);
	ILI9163_drawString(122, 95, Font_7x10, RED, "Tare");
	ILI9163_drawRect(120, 70, 160, 90, 1, RED);
	write_number(122, 75, Font_7x10, RED, data->reference);
	ILI9163_drawRect(120, 50, 160, 70, 1, RED);
	ILI9163_drawString(130, 55, Font_7x10, RED, "Ref");
	ILI9163_drawRect(120, 30, 160, 50, 1, RED);
	ILI9163_drawString(125, 35, Font_7x10, RED, "N-T");

	ILI9163_drawRect(0, 30, 60, 50, 1, RED);
	ILI9163_drawString(8, 35, Font_7x10, RED, "SET");

	ILI9163_drawRect(60, 30, 120, 50, 1, RED);
	ILI9163_drawString(68, 35, Font_7x10, RED, "COUNT");
	if (data->tareState) {

		draw_custom_icon(105, 95, 5);
		int weight = data->weight;
		int tare = data->tareValue;
		int now = weight - tare;
		if (data->weight < 2110) {

			draw_number(10, 70, Font_7x10, BLACK, now);

		} else {

			ILI9163_drawString(10, 70, Font_7x10, BLACK, "OL");
		}

	} else {

		if(data->weight < 2110) {
			draw_number(10, 70, Font_7x10, BLACK, data->weight);

		} else {
			ILI9163_drawString(10, 70, Font_7x10, BLACK, "OL");

		}
	}
}

void pass_mode(ScreenData *data) {

	ILI9163_drawRect(0, 30, 120, 110, 2, RED);
	draw_number(10, 70, Font_7x10, BLACK, data->weight);
	ILI9163_drawString(10, 5, Font_7x10, BLACK, "PASS/FAIL MODE");
	ILI9163_drawRect(120, 90, 159, 110, 2, RED);
	ILI9163_drawString(122, 95, Font_7x10, RED, "Tare");
	ILI9163_drawRect(120, 70, 160, 90, 1, RED);
	ILI9163_drawString(110, 75, Font_7x10, RED, "L");
	draw_number(121, 75, Font_7x10, RED, data->thrL);
	ILI9163_drawRect(120, 50, 160, 70, 1, RED);
	ILI9163_drawString(110, 55, Font_7x10, RED, "H");
	draw_number(121, 55, Font_7x10, RED, data->thrH);
	ILI9163_drawRect(120, 30, 160, 50, 1, RED);
	ILI9163_drawString(125, 35, Font_7x10, RED, "N-T");
	if (data->tareState) {

		draw_custom_icon(105, 95, 5);
		int weight = data->weight;
		int tare = data->tareValue;
		int now = weight - tare;
		if (data->weight < 2110) {

			draw_number(10, 70, Font_7x10, BLACK, now);

		} else {

			ILI9163_drawString(10, 70, Font_7x10, BLACK, "OL");
		}
	} else {

		if(data->weight < 2110) {
			draw_number(10, 70, Font_7x10, BLACK, data->weight);

		} else {
			ILI9163_drawString(10, 70, Font_7x10, BLACK, "OL");

		}
	}
	if (data->weight > data->thrL && data->weight < data->thrH) {

		draw_custom_icon(80, 45, 2);
	} else {

		draw_custom_icon(80, 45, 1);
	}
}

void calibration(ScreenData *Data) {

	ILI9163_drawString(10, 5, Font_7x10, BLACK, "CAL MODE ");

	if (Data->calWeight1 == 0 && Data->calWeight2 == 0 && Data->calWeight3 == 0) {
		//show first
		ILI9163_drawRect(0, 30, 40, 56, 1, RED);
		draw_number(2, 40, Font_7x10, BLACK, Data->cal1);
		ILI9163_drawRect(40, 30, 120, 56, 1, RED);
		ILI9163_drawRect(120, 30, 160, 56, 1, RED);
		ILI9163_drawString(121, 40, Font_7x10, RED, "Y/N");
		write_number(41, 40, Font_7x10, BLACK, Data->calWeight1);

		ILI9163_drawString(5, 60, Font_7x10, RED, "PLEASE READ ZERO ADC");

	} else if (Data->calWeight1 != 0 && Data->calWeight2 == 0) {
		//show second
		ILI9163_drawRect(0, 56, 40, 82, 1, RED);
		draw_number(2, 66, Font_7x10, BLACK, Data->cal2);
		ILI9163_drawRect(40, 56, 120, 82, 1, RED);
		ILI9163_drawRect(120, 56, 160, 82, 1, RED);
		ILI9163_drawString(121, 66, Font_7x10, RED, "Y/N");
		write_number(41, 66, Font_7x10, BLACK, Data->calWeight2);

		ILI9163_drawString(5, 35, Font_7x10, RED, "PLEASE ENTER 1st WEIGHT");
		ILI9163_drawString(5, 86, Font_7x10, RED, "PLEASE READ 1st ADC");
	} else if (Data->calWeight2 != 0&& Data->calWeight3 == 0) {
		//show third
		ILI9163_drawRect(0, 82, 40, 110, 1, RED);
		draw_number(2, 92, Font_7x10, BLACK, Data->cal3);
		ILI9163_drawRect(40, 82, 120, 110, 1, RED);
		ILI9163_drawRect(120, 82, 160, 110, 1, RED);
		ILI9163_drawString(121, 92, Font_7x10, RED, "Y/N");
		write_number(41, 92, Font_7x10, BLACK, Data->calWeight3);

		ILI9163_drawString(5, 35, Font_7x10, RED, "PLEASE ENTER 2ed WEIGHT");
		ILI9163_drawString(5, 60, Font_7x10, RED, "PLEASE READ 2ed ADC");

	} else if (Data->calWeight3 != 0) {

		ILI9163_drawRect(0, 30, 40, 56, 1, RED);
		draw_number(2, 40, Font_7x10, BLACK, Data->cal1);
		ILI9163_drawRect(40, 30, 120, 56, 1, RED);
		ILI9163_drawRect(120, 30, 160, 56, 1, RED);
		ILI9163_drawString(121, 40, Font_7x10, RED, "Y/N");
		write_number(41, 40, Font_7x10, BLACK, Data->calWeight1);

		ILI9163_drawRect(0, 56, 40, 82, 1, RED);
		draw_number(2, 66, Font_7x10, BLACK, Data->cal2);
		ILI9163_drawRect(40, 56, 120, 82, 1, RED);
		ILI9163_drawRect(120, 56, 160, 82, 1, RED);
		ILI9163_drawString(121, 66, Font_7x10, RED, "Y/N");
		write_number(41, 66, Font_7x10, BLACK, Data->calWeight2);

		ILI9163_drawRect(0, 82, 40, 110, 1, RED);
		draw_number(2, 92, Font_7x10, BLACK, Data->cal3);
		ILI9163_drawRect(40, 82, 120, 110, 1, RED);
		ILI9163_drawRect(120, 82, 160, 110, 1, RED);
		ILI9163_drawString(121, 92, Font_7x10, RED, "Y/N");
		write_number(41, 92, Font_7x10, BLACK, Data->calWeight3);
	}

//	ILI9163_drawRect(0, 30, 40, 56, 1, RED);
//	draw_number(2, 40, Font_7x10, BLACK, Data->cal1);
//	ILI9163_drawRect(40, 30, 120, 56, 1, RED);
//	ILI9163_drawRect(120, 30, 160, 56, 1, RED);
//	ILI9163_drawString(121, 40, Font_7x10, RED, "Y/N");
//	write_number(41, 40, Font_7x10, BLACK, Data->calWeight1);

//	ILI9163_drawRect(0, 82, 40, 110, 1, RED);
//	draw_number(2, 92, Font_7x10, BLACK, Data->cal3);
//	ILI9163_drawRect(40, 82, 120, 110, 1, RED);
//	ILI9163_drawRect(120, 82, 160, 110, 1, RED);
//	ILI9163_drawString(121, 92, Font_7x10, RED, "Y/N");
//	write_number(41, 92, Font_7x10, BLACK, Data->calWeight3);

	ILI9163_drawRect(80, 0, 160, 30, 1, RED);
	ILI9163_drawString(101, 13, Font_7x10, RED, "Confirm");

}

void keyboard(ScreenData *data) {

	ILI9163_drawRect(0, 0, 35, 36, 1, RED);
	ILI9163_drawString(17, 16, Font_7x10, RED, "1");
	ILI9163_drawRect(35, 0, 70, 36, 1, RED);
	ILI9163_drawString(52, 16, Font_7x10, RED, "2");
	ILI9163_drawRect(70, 0, 105, 36, 1, RED);
	ILI9163_drawString(87, 16, Font_7x10, RED, "3");

	ILI9163_drawRect(0, 36, 35, 72, 1, RED);
	ILI9163_drawString(17, 52, Font_7x10, RED, "4");
	ILI9163_drawRect(35, 36, 70, 72, 1, RED);
	ILI9163_drawString(52, 52, Font_7x10, RED, "5");
	ILI9163_drawRect(70, 36, 105, 72, 1, RED);
	ILI9163_drawString(87, 52, Font_7x10, RED, "6");

	ILI9163_drawRect(0, 72, 35, 110, 1, RED);
	ILI9163_drawString(17, 88, Font_7x10, RED, "7");
	ILI9163_drawRect(35, 72, 70, 110, 1, RED);
	ILI9163_drawString(52, 88, Font_7x10, RED, "8");
	ILI9163_drawRect(70, 72, 105, 110, 1, RED);
	ILI9163_drawString(87, 88, Font_7x10, RED, "9");

	ILI9163_drawRect(105, 0, 159, 70, 1, RED);
	ILI9163_drawString(122, 16, Font_7x10, RED, "0");

	ILI9163_drawRect(105, 90, 159, 110, 1, RED);
	ILI9163_drawString(108, 95, Font_7x10, RED, "Confirm");

	ILI9163_drawRect(105, 70, 159, 90, 1, RED);
	ILI9163_drawString(113, 75, Font_7x10, RED, "Clear");

	ILI9163_drawRect(105, 50, 159, 70, 1, RED);

	if (data->preMode == COUNTING) {

		write_number(107, 55, Font_7x10, RED, data->reference);

	} else if (data->preMode == PASS) {

		if (data->thr == 0) {

			draw_number(107, 55, Font_7x10, RED, data->thrL);
		} else if (data->thr == 1) {

			draw_number(107, 55, Font_7x10, RED, data->thrH);
		}

	} else if (data->preMode == CALIBRATION) {

		if (data->cal_num == 1) {

			draw_number(107, 55, Font_7x10, RED, data->cal1);
		} else if (data->cal_num == 2) {

			draw_number(107, 55, Font_7x10, RED, data->cal2);
		} else if (data->cal_num == 3) {

			draw_number(107, 55, Font_7x10, RED, data->cal3);
		}
	}
}

void draw_custom_icon(uint8_t x, uint8_t y, int type) {

	// fail icon
	if (type == 1) {

		uint8_t radius = 2;
		uint16_t color = WHITE;

		ILI9163_fillCircle(x + 4, y + 4, 10, RED);

		ILI9163_fillCircle(x, y, radius, color);
		ILI9163_fillCircle(x + 2, y + 2, radius, color);
		ILI9163_fillCircle(x + 4, y + 4, radius, color);
		ILI9163_fillCircle(x + 6, y + 6, radius, color);
		ILI9163_fillCircle(x + 8, y + 8, radius, color);

		ILI9163_fillCircle(x, y + 8, radius, color);
		ILI9163_fillCircle(x + 2, y + 6, radius, color);
		ILI9163_fillCircle(x + 6, y + 2, radius, color);
		ILI9163_fillCircle(x + 8, y, radius, color);

		//pass icon
	} else if (type == 2) {

		uint8_t radius = 2;
		uint16_t color = WHITE;

		ILI9163_fillCircle(x + 6, y + 2, 10, GREEN);

		ILI9163_fillCircle(x + 2, y + 2, radius, color);
		ILI9163_fillCircle(x + 4, y + 4, radius, color);
		ILI9163_fillCircle(x + 6, y + 6, radius, color);

		ILI9163_fillCircle(x + 8, y + 4, radius, color);
		ILI9163_fillCircle(x + 10, y + 2, radius, color);
		ILI9163_fillCircle(x + 12, y, radius, color);

		//wifi icon
	} else if (type == 3) {

		uint16_t color = GREEN;
		ILI9163_drawCircle(x, y -2, 8, color);
		ILI9163_drawCircle(x, y -2, 6, color);
		ILI9163_drawCircle(x, y -2, 4, color);

		ILI9163_fillRect(x - 1, y - 2, x + 1, y + 10, color);

		// PC communication
	}else if (type == 4) {

		ILI9163_fillRect(x - 3, y, x + 3, y + 4, DARKGRAY);
		ILI9163_fillRect(x - 5, y + 4, x + 5, y + 14, BLACK);
		ILI9163_fillRect(x - 1, y + 14, x + 1, y + 16, BLACK);

		ILI9163_fillRect(x, y + 16, x + 8, y + 18, BLACK);
		ILI9163_fillRect(x + 7, y + 4, x + 9, y + 18, BLACK);

		// tare
	}else if (type == 5) {

		ILI9163_fillRect(x - 7, y - 7, x + 8, y + 8, BLACK);
		ILI9163_fillCircle(x, y, 5, WHITE);
		ILI9163_fillCircle(x, y, 2, BLACK);

		ILI9163_fillRect(x - 8, y -11, x + 9, y - 9, BLACK);
		ILI9163_fillRect(x - 9, y - 13, x - 7, y - 11, BLACK);
		ILI9163_fillRect(x + 8, y - 13, x + 10, y - 11, BLACK);


	}
}

void changeBrightness(TIM_HandleTypeDef *htim2, ScreenData *data, int type) {

	// Increment or decrement the brightness level by changing the dutyCycle

	if (type == 1) {

		if(data->dutyCycle >= 25) {

			data->dutyCycle = 25;

		} else {

			data->dutyCycle += 5;
		}

	} else {

		if(data->dutyCycle <= 0) {

			data->dutyCycle = 0;

		} else {

			data->dutyCycle -= 5;
		}
	}

	// Update the display with the new brightness and bar graph level
	__HAL_TIM_SET_COMPARE(htim2, TIM_CHANNEL_3, data->dutyCycle);
}

void brightnessButton(void){

	ILI9163_drawRect(0, 110, 60, 127, 1, RED);
	ILI9163_drawString(5, 113, Font_7x10, RED, "INC Lig");
	ILI9163_drawRect(60, 110, 120, 127, 1, RED);
	ILI9163_drawString(65, 113, Font_7x10, RED, "DEC Lig");
	ILI9163_drawRect(120, 110, 160, 127, 1, RED);
	ILI9163_drawString(125, 113, Font_7x10, RED, "Menu");
}

void touchProcess(TIM_HandleTypeDef *htim2, ScreenData *data) {

	//char* a;
	//char* ref;

	if (data->yValue > 110) {

		if (data->xValue < 59 && data->xValue > 0){

			changeBrightness(htim2, data, 1);

		} else if (data->xValue < 119 && data->xValue > 59){

			changeBrightness(htim2, data, 0);

		} else if (data->xValue > 120) {

			data->frame = MENU;

		}
	}
	if (data->frame == WEIGHTING) {

		if (data->xValue > 120) {

			if (data->yValue > 90 && data->yValue < 110) {

				data->tareState = 1;
				data->tareValue = data->weight;

			} else if (data->yValue > 30 && data->yValue < 50) {

				data->tareState = 0;
			}
		}
	} else if (data->frame == COUNTING) {

		if (data->xValue > 120) {

			if (data->yValue > 70 && data->yValue < 90) {

				data->frame = KEYBOARD;
				data->preMode = COUNTING;

			} else if (data->yValue > 90 && data->yValue < 110) {

				data->tareState = 1;
				data->tareValue = data->weight;

			} else if (data->yValue > 30 && data->yValue < 50) {

				data->tareState = 0;
			}
		} else if (data->xValue < 120 && data->xValue > 60) {

			if (data->yValue > 30 && data->yValue < 50) {

				if (data->single != 0) {

					data->count = round(data->weight/data->single);
				}
			}
		} else if (data->xValue < 60) {

			if (data->yValue > 30 && data->yValue < 50) {

				if (data->reference != 0) {
///////////////////////////////////////////////////////////////////
					float weight = data->weight;
					float ref = data->reference;

					data->single = weight/ref;
//////////////////////////////////////////////////////////////////
				}
			}
		}
	} else if (data->frame == PASS) {

		if (data->xValue > 120) {

			if (data->yValue > 70 && data->yValue < 90) {

				data->frame = KEYBOARD;
				data->preMode = PASS;
				data->thr = 0;

			} else if (data->yValue > 90 && data->yValue < 110) {

				data->tareState = 1;
				data->tareValue = data->weight;

			} else if (data->yValue > 30 && data->yValue < 50) {

				data->tareState = 0;

			} else if (data->yValue > 50 && data->yValue < 70) {

				data->frame = KEYBOARD;
				data->preMode = PASS;
				data->thr = 1;
			}
		}
	} else if (data->frame == CALIBRATION) {
		if (data->xValue < 40) {

			if (data->yValue < 82 && data->calWeight1 != 0 && data->calWeight2 == 0) {

				data->frame = KEYBOARD;
				data->preMode = CALIBRATION;
				data->cal_num = 2;

			} else if (data->yValue < 110 && data->yValue > 82 && data->calWeight2 != 0 && data->calWeight3 == 0) {

				data->frame = KEYBOARD;
				data->preMode = CALIBRATION;
				data->cal_num = 3;
			}
		} else if (data->xValue > 80){

			if (data->yValue < 30) {

				data->calculateCal = 1;

			}else if (data->yValue < 56 && data->yValue > 30 && data->xValue > 120 && data->calWeight1 == 0 && data->calWeight2 == 0 && data->calWeight3 == 0) {

				data->cal1Confirm = 1 - data->cal1Confirm;

				if (data->cal1Confirm == 1) {

					data->calWeight1 = data->rawADC;

					data->cal1Confirm = 0;

				} else {

					data->calWeight1 = 0;
				}

			} else if (data->yValue < 82 && data->yValue > 56 && data->xValue > 120 && data->calWeight1 != 0 && data->calWeight2 == 0) {

				data->cal2Confirm = 1 - data->cal2Confirm;

				if (data->cal2Confirm == 1) {

					data->calWeight2 = data->rawADC;

					data->cal2Confirm = 0;
				} else {

					data->calWeight2 = 0;
				}

			} else if (data->yValue < 110 && data->yValue > 82 && data->xValue > 120 && data->calWeight2 != 0 && data->calWeight3 == 0) {

				data->cal3Confirm = 1 - data->cal3Confirm;

				if (data->cal3Confirm == 1) {

					data->calWeight3 = data->rawADC;

					data->cal3Confirm = 0;
				} else {

					data->calWeight3 = 0;
				}

			}
		}
	} else if (data->frame == KEYBOARD) {

		// pass mode
		if (data->preMode == PASS) {

			if (data->thr == 0) {

				data->thrL = keyboard_process(data, data->thrL);
			} else if (data->thr == 1) {

				data->thrH = keyboard_process(data, data->thrH);
			}

			// Counting mode keyboard
		} else if (data->preMode == COUNTING) {

			data->reference = keyboard_process(data, data->reference);

		} else if (data->preMode == CALIBRATION) {

			if (data->cal_num == 1) {

				data->cal1 = keyboard_process(data, data->cal1);

			} else if (data->cal_num == 2) {

				data->cal2 = keyboard_process(data, data->cal2);

			}else if (data->cal_num == 3) {

				data->cal3 = keyboard_process(data, data->cal3);
			}
		}
	//sprintf(ref, "%d", data->reference);
	//ILI9163_drawString(107, 55, Font_7x10, BLACK, ref);
	} else if (data->frame == MENU) {
		if (data->yValue < 54) {

			if (data->xValue < 80){

				data->frame = WEIGHTING;

			} else if (data->xValue > 80) {

				data->frame = COUNTING;
			}
		} else if (data->yValue > 54 && data->yValue < 110) {

			if (data->xValue < 80){

				data->frame = PASS;

			} else if (data->xValue > 80) {

				data->frame = CALIBRATION;
				data->calWeight1 = 0;
				data->calWeight2 = 0;
				data->calWeight3 = 0;
			}
		}
	}
}

uint16_t keyboard_process(ScreenData* data, uint16_t value) {

	if (data->xValue < 35 && data->xValue > 0) {

		if (data->yValue < 36 && data->yValue > 0){

			value = value * 10 + 1;

		} else if (data->yValue < 72 && data->yValue > 36){

			value = value * 10 + 4;

		} else if (data->yValue < 110 && data->yValue > 72) {

			value = value * 10 + 7;

		}
	} else if (data->xValue < 70 && data->xValue > 35) {

		if (data->yValue < 36){

			value = value * 10 + 2;

		} else if (data->yValue < 72 && data->yValue > 36){

			value = value * 10 + 5;

		} else if (data->yValue < 110 && data->yValue > 72) {

			value = value * 10 + 8;

		}
	} else if(data->xValue < 105 && data->xValue > 70) {

		if (data->yValue < 36){

			value = value * 10 + 3;

		} else if (data->yValue < 72 && data->yValue > 36){

			value = value * 10 + 6;

		} else if (data->yValue < 110 && data->yValue > 72) {

			value = value * 10 + 9;

		}

	} else if (data->xValue > 105) {

		if (data->yValue < 110 && data->yValue > 90){

			data->frame = data->preMode;

		} else if (data->yValue < 90 && data->yValue > 70){

			value = 0;

		} else if (data->yValue < 70) {

			value = value * 10 + 0;
		}
	}

	return value;
}

//void calibration_process(ScreenData* data) {
//
//	;
//}



