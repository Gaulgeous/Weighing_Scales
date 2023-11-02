from Model import Model
from View import View
from constants import *
import serial
import serial.tools.list_ports
import paho.mqtt.client as mqtt

"""
Controller governs communication between the GUI and model
"""


class Controller:

    """
    Initialise all controller variables
    """
    def __init__(self):

        self.mqtt_address = "tp-mqtt.uqcloud.net"
        self.wifi_password = "8c6d8944a624703e502824ed"
        self.slider_read = 5
        self.serial_port = None
        self.connected_port = None
        self.wifi_client = None
        self.weights = [0, 0, 0, 0]
        self.count_weights = [0, 0, 0, 0]
        self.pass_weights = [0, 0, 0]
        self.cal_weights = [0, 0, 0, 0]
        self.coms_type = PORT
        self._data = None
        self.new_mode = GENERAL
        self.mode = GENERAL
        self.wifi_connected = 0
        self.do_cal = 0
        self.cal_icon_update = 0
        self.wifi_connect()
        self.model = Model()
        self.view = View(self)

    """
    Evaluate any changes that have been made in the system
    Update each of the display readings on the user interface
    """
    def evaluate(self):

        self.weights = self.model.general_weights()
        self.count_weights = self.model.count_weights()
        self.pass_weights = self.model.pass_weights()
        self.cal_weights = self.model.cal_weights()

    """
    Increment the brightness of the lcd display
    """
    def inc_light(self):
        self.model.update_queue.append((LIGHT, 0))

    """
    Decrement the brightness of the lcd display
    """
    def dec_light(self):
        self.model.update_queue.append((LIGHT, 1))

    """
    Send a refresh command to the STM to refresh all GUI data
    """
    def refresh(self):
        self.send_calibration()

    """
    Used to restart count mode (reset all the values)
    """
    def restart_count(self):
        self.model.adc[REF_COUNT] = 0
        self.model.mass[REF_COUNT] = 0
        self.model.mass[REF_WEIGHT] = 0
        self.model.update_queue.append((REF_COUNT, 0))
        self.model.update_queue.append((REF_COUNT, 0))

        self.model.update_queue.append((REF_WEIGHT, 0))
        self.model.update_queue.append((REF_WEIGHT, 0))

    """
    Sends the tare trigger to stm32
    """
    def call_tare(self):
        # Update tare. -1 acts as a sentinel value
        self.model.update_queue.append((TARE, -1))
        self.model.update_queue.append((TARE, -1))
        self.view.tare_icon.configure(image=self.view.tarecon)

    """
    Clears the tare trigger in the stm32
    """
    def clear_tare(self):
        # Update tare. -1 acts as a sentinel value
        self.model.update_queue.append((CANCEL_TARE, -1))
        self.model.update_queue.append((CANCEL_TARE, -1))
        self.view.tare_icon.configure(image='')

    """
    Sends the tare trigger to stm32
    """
    def call_zero(self):
        # Update tare. -1 acts as a sentinel value
        self.model.update_queue.append((SET_ZERO, -1))
        self.model.update_queue.append((SET_ZERO, -1))

    """
    Sends the cal trigger to stm32
    """
    def call_cal(self):
        # Update tare. -1 acts as a sentinel value
        self.model.update_queue.append((CALIBRATE, -1))
        self.model.update_queue.append((CALIBRATE, -1))
        self.view.cal_icon.config(image=self.view.calcon)

    """
    Connects the GUI to the given COM port
    """
    def cereal(self, com_port):

        try:

            self.serial_port = serial.Serial(port=com_port, baudrate=115200, timeout=0, write_timeout=0)
            self.serial_port.open()

            # print messages to explain connection status
            print("Connected to: " + com_port)

        except serial.SerialException:
            if self.serial_port:
                if self.serial_port.portstr == com_port:
                    print("Already connected")
                    self.model.update_queue.append((MODE, PORT))
                    self.model.update_queue.append((MODE, PORT))
                    self.cal_icon_update = 1
                    pass

                else:
                    self.serial_port = None
                    print("Connected, but crashed")

            else:
                print("No serial port exception")

    """
    Sends the most important message required over MQTT client
    Importance of message is determined by priority in model's update queue
    """
    def wifi_send(self):
        if self.wifi_client and self.wifi_connected:

            message = self.model.coms_swap(self.mode)

            # Bit shift message
            number = (message[1] << 24) | (message[2] << 16) | (message[3] << 8) | message[4]

            self.wifi_client.publish("team20/GUI", "{0} {1}".format(message[0], number))

            # Then update all the parameters
            self.evaluate()
        else:
            return

    """
    Establishes connection over UQ MQTT server
    """
    def wifi_connect(self):
        self.wifi_client = mqtt.Client(client_id="yeeeeehaw", clean_session=True, transport="websockets")
        self.wifi_client.ws_set_options(path="/ws")
        self.wifi_client.username_pw_set("team20", self.wifi_password)
        self.wifi_client.tls_set()
        self.wifi_client.on_connect = self.on_connect
        self.wifi_client.on_disconnect = self.on_disconnect
        self.wifi_client.on_message = self.on_message
        self.wifi_client.loop_start()
        self.wifi_client.connect_async(self.mqtt_address, port=443)

    """
    Handles receiving of messages over wifi
    Breaks down the messages and places incoming values within their respective variables
    """
    def on_message(self, _client, _userdata, msg):
        message = str(msg.payload.decode("ascii"))
        self.received_data(message)

    """
    Sets the weights displayed in either grams or kilograms
    """
    def set_units(self, _event):
        if self.view.units_clicked.get() == 'kg':
            # Kilograms
            self.model.units = 1000
        elif self.view.units_clicked.get() == 'g':
            # grams
            self.model.units = 1

    """
    Called when MQTT client connects to UQ server. Subscribes to relevant topics
    """
    def on_connect(self, _client, _userdata, _flags, rc):
        if rc == 0:
            print("successfully connected")
            self.wifi_client.subscribe("team20/ESP")
            self.wifi_connected = 1
        else:
            print("error code " + str(rc))

    """
    Toggles connection flag in case a disconnect happens during runtime
    """
    def on_disconnect(self, _client, _userdata, _rc):
        self.wifi_connected = 0

    """
    Function for sending data over UART to STM32
    """
    def uart_send(self):

        if self.serial_port:
            message = self.model.coms_swap(self.mode)
            self.serial_port.write(message)

            # data received from serialPort
            message = self.serial_port.readline().decode('utf-8')
            self.received_data(message)

            # Then update all the parameters
            self.evaluate()

        else:

            return

    """
    Send a query for each of the calibration parameters over to the STM
    """
    def send_calibration(self):
        # Update all parameters. -1 is a sentinel value
        self.model.update_queue.append((ZERO, -1))
        self.model.update_queue.append((CAL_1, -1))
        self.model.update_queue.append((CAL_2, -1))
        self.model.update_queue.append((CAL_M1, -1))
        self.model.update_queue.append((CAL_M2, -1))
        self.model.update_queue.append((LOW_PASS, -1))
        self.model.update_queue.append((HIGH_PASS, -1))
        self.model.update_queue.append((REF_COUNT, -1))
        self.model.update_queue.append((REF_WEIGHT, -1))
        self.model.update_queue.append((TARE, 0))
        self.model.update_queue.append((USER_MODE, -1))
        self.model.update_queue.append((DONE_CAL, -1))

        if self.coms_type == WIFI:
            self.model.update_queue.append((ZERO, -1))
            self.model.update_queue.append((CAL_1, -1))
            self.model.update_queue.append((CAL_2, -1))
            self.model.update_queue.append((CAL_M1, -1))
            self.model.update_queue.append((CAL_M2, -1))
            self.model.update_queue.append((LOW_PASS, -1))
            self.model.update_queue.append((HIGH_PASS, -1))
            self.model.update_queue.append((REF_COUNT, -1))
            self.model.update_queue.append((REF_WEIGHT, -1))
            self.model.update_queue.append((TARE, 0))
            self.model.update_queue.append((USER_MODE, -1))
            self.model.update_queue.append((DONE_CAL, -1))

    """
    Breaks down incoming data and stores it within the relevant variables
    """
    def received_data(self, message):

        if len(message) > 0:

            pos = 0
            value = 0
            frags = message.split()

            if len(frags) == 2:
                for i in frags[0]:
                    if i.isdigit():
                        pos = pos * 10 + int(i)
                value = int(frags[1])

            # Sort values into their allocated areas
            if pos == M:
                self.model.m = value
            elif pos == USER_MODE:
                self.new_mode = value
            elif pos == C:
                self.model.c = value
            elif pos == TARE:
                self.model.adc[pos] = value - self.model.adc[ZERO]
            elif pos == CAL_M1:
                self.model.mass[CAL_1] = value
            elif pos == CAL_M2:
                self.model.mass[CAL_2] = value
            elif pos == LOW_PASS:
                self.model.mass[LOW_PASS] = value
            elif pos == HIGH_PASS:
                self.model.mass[HIGH_PASS] = value
            elif pos == MODE:
                self.coms_type = value
            elif pos == MASS:
                self.model.weight = value
            elif pos == CALIBRATE:
                if self.mode == CALIBRATION:
                    self.do_cal = 1
            elif pos == SET_ZERO:
                self.model.adc[ZERO] = value
            elif pos == CANCEL_TARE:
                self.model.adc[TARE] = self.model.adc[ZERO]
            elif pos == DONE_CAL:
                self.cal_icon_update = 1
            elif pos == LIGHT:
                self.slider_read = value
            elif pos == CURRENT_COUNT:
                self.model.current_count = value
            else:
                self.model.adc[pos] = value

    """
    Adds any weight updates from the GUI into the priority queue for sending over to STM32
    Handles the first set of weight variables
    """
    def weight_update1(self):

        if self.view.data1.get().isdigit():

            if self.mode == CALIBRATION:
                self.model.mass[CAL_1] = int(self.view.data1.get())
                self.model.update_queue.append((CAL_1, int(self.view.data1.get())))
                self.model.update_queue.append((CAL_1, int(self.view.data1.get())))

            elif self.mode == COUNTING:
                self.model.mass[REF_COUNT] = int(self.view.data1.get())
                self.model.adc[REF_COUNT] = self.model.mass[REF_COUNT]
                self.model.update_queue.append((REF_COUNT, int(self.view.data1.get())))
                self.model.update_queue.append((REF_COUNT, int(self.view.data1.get())))

            else:
                self.model.mass[LOW_PASS] = int(self.view.data1.get())
                self.model.update_queue.append((LOW_PASS, int(self.view.data1.get())))
                self.model.update_queue.append((LOW_PASS, int(self.view.data1.get())))

    """
    Adds any weight updates from the GUI into the priority queue for sending over to STM32
    Handles the second set of weight variables
    """
    def weight_update2(self):

        if self.mode == COUNTING:
            self.model.update_queue.append((REF_WEIGHT, self.model.mass[REF_WEIGHT]))
            self.model.update_queue.append((REF_WEIGHT, self.model.mass[REF_WEIGHT]))
            self.model.mass[REF_WEIGHT] = self.model.weight

        elif self.view.data2.get().isdigit():
            if self.mode == CALIBRATION:
                self.model.mass[CAL_2] = int(self.view.data2.get())
                self.model.update_queue.append((CAL_2, int(self.view.data2.get())))
                self.model.update_queue.append((CAL_2, int(self.view.data2.get())))
            else:
                self.model.mass[HIGH_PASS] = int(self.view.data2.get())
                self.model.update_queue.append((HIGH_PASS, int(self.view.data2.get())))
                self.model.update_queue.append((HIGH_PASS, int(self.view.data2.get())))

    """
    Connects the com port to the serial
    """
    def change_com_port(self, selection):
        self.connected_port = selection.device
        self.cereal(selection.device)


"""
Engage in the main loop
"""
if __name__ == "__main__":
    interface = Controller()
    interface.view.mainloop()
