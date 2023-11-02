from constants import *
import numpy as np

"""
Function handle for if the data hasn't been initialised for calibration
"""
def zeros_func(value):
    return 0

"""
Model is the back end that controls all the analysis of the input commands
"""


class Model:

    """
    Initialise all relevant variables
    """
    def __init__(self):

        self.units = 1
        self.adc_fac = 1
        self.adc = [0] * 30
        self.mass = [0] * 30
        self.update_queue = []

        self.m = 0
        self.c = 0
        self.cal_factor = 405
        self.current_count = 0

        self.send_type = 0
        self.weight = 0
        self.serial_port = None

    """
    Return the general weights for the system
    """
    def general_weights(self):

        # if the reading is over 2.1kg
        if self.weight >= 2110:
            return [self.adc[RAW] // self.adc_fac, self.adc[ZERO] // self.adc_fac,
                    self.adc[TARE] // self.adc_fac, OVERLOAD]
        else:
            return [self.adc[RAW] // self.adc_fac, self.adc[ZERO] // self.adc_fac,
                    self.adc[TARE] // self.adc_fac, self.weight / self.units]

    """
    Return the pass parameters
    """
    def pass_weights(self):

        valid = 0
        if self.mass[HIGH_PASS] > self.weight > self.mass[LOW_PASS]:
            valid = 1
        if self.units == 1:
            return [self.mass[LOW_PASS], valid, self.mass[HIGH_PASS]]
        else:
            return [self.mass[LOW_PASS] / self.units, valid, self.mass[HIGH_PASS] / self.units]

    """
    Return the calibration parameters
    """
    def cal_weights(self):

        return [self.mass[CAL_1] / self.units, self.mass[CAL_2] / self.units, self.adc[CAL_1], self.adc[CAL_2]]

    """
    Return counting parameters
    """
    def count_weights(self):

        if self.adc[REF_COUNT] > 0 and self.mass[REF_WEIGHT] > 0:

            single = float(self.mass[REF_WEIGHT]) / self.adc[REF_COUNT]

            return [self.mass[REF_WEIGHT] / self.units, self.adc[REF_COUNT], single / self.units, self.current_count]

        else:
            return [self.mass[REF_WEIGHT] / self.units, self.adc[REF_COUNT], 0, 0]

    """
    Constructs message to send to STM32 in order of priority
    Oldest message requests are sent over first
    If no message requests, send generic request for raw adc reading
    """
    def coms_swap(self, input_mode):
        to_send = bytearray([0, 0, 0, 0, 0])

        if len(self.update_queue) > 0:
            output = self.update_queue.pop(0)

            output[1].to_bytes(4, byteorder="big", signed=True)
            send1 = output[1] & 0xFF
            send2 = (output[1] & (0xFF << 8)) >> 8
            send3 = (output[1] & (0xFF << 16)) >> 16
            send4 = (output[1] & (0xFF << 24)) >> 24

            to_send = bytearray([output[0], send4, send3, send2, send1])

        if to_send[0] == 0:

            if self.send_type == 0:
                to_send[0] = 0
                self.send_type = 1
            elif self.send_type == 1:
                to_send[0] = MASS
                self.send_type = 2
            else:
                if input_mode == COUNTING:
                    to_send[0] = CURRENT_COUNT
                    self.send_type = 0
                else:
                    self.send_type = 1
                    to_send[0] = 0

        return to_send

    """
    Returns data points needed for plotting the calibration mode input on graph
    """
    def lin_reg(self):
        return [self.adc[CAL_1], self.adc[CAL_2], self.adc[ZERO]], [self.mass[CAL_1], self.mass[CAL_2], 0]

    """
    Converts ADC reading to actual weight using the calibration parameters
    """
    def weight_conversion(self, adc, mass):

        if self.adc[CAL_1] and self.adc[CAL_2] and self.adc[ZERO] and self.mass[CAL_1] and self.adc[CAL_2]:
            model = np.polyfit(adc, mass, 1)
            lin_reg = np.poly1d(model)
            return lin_reg
        else:
            return zeros_func
