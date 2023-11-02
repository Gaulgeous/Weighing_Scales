import tkinter as tk
from tkinter import *
from PIL import ImageTk, Image
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
from matplotlib.figure import Figure
import matplotlib
import threading
import serial
import serial.tools.list_ports
from constants import *
matplotlib.use('TkAgg')


"""
View controls the GUI that the user sees
"""


class View(tk.Tk):

    """
    Initialise the many variables that are required for user interface
    """
    def __init__(self, controller):
        super().__init__()
        self.controller = controller
        self.geometry('800x750')

        self.modes = ['General', 'Counting', 'Pass/Fail', 'Calibration']
        self.mode_clicked = StringVar()
        self.mode_clicked.set(self.modes[0])

        self.units = ['g', 'kg']
        self.units_clicked = StringVar()
        self.units_clicked.set(self.units[0])

        self.title('ENGG3800 GUI')
        self.main_frame = tk.Frame(self)
        self.main_frame.pack()

        # this is the comm-port frame at the top of the GUI
        self.port_frame = tk.Frame(self.main_frame, height=50, width=800, bg='black')
        self.port_frame.pack(fill=BOTH)

        self.usb_active_img = Image.open('GUI images/usb_active.PNG').resize((25, 25))
        self.usb_active = ImageTk.PhotoImage(self.usb_active_img)
        self.usb_inactive_img = Image.open('GUI images/usb_inactive.PNG').resize((25, 25))
        self.usb_inactive = ImageTk.PhotoImage(self.usb_inactive_img)

        self.wifi_active_img = Image.open('GUI images/wifi_active.PNG').resize((25, 25))
        self.wifi_active = ImageTk.PhotoImage(self.wifi_active_img)
        self.wifi_inactive_img = Image.open('GUI images/wifi_inactive.PNG').resize((25, 25))
        self.wifi_inactive = ImageTk.PhotoImage(self.wifi_inactive_img)

        self.ports_options = None
        self.all_ports = None

        self.tarecon_img = Image.open('GUI images/tare.PNG').resize((25, 25))
        self.tarecon = ImageTk.PhotoImage(self.tarecon_img)

        self.calcon_img = Image.open('GUI images/gears.PNG').resize((25, 25))
        self.calcon = ImageTk.PhotoImage(self.calcon_img)

        self.tick_img = Image.open('GUI images/tick.PNG').resize((80, 80))
        self.tick = ImageTk.PhotoImage(self.tick_img)
        self.cross_img = Image.open('GUI images/cross.PNG').resize((80, 80))
        self.cross = ImageTk.PhotoImage(self.cross_img)

        self.sm_tick_img = Image.open('GUI images/tick.PNG').resize((20, 20))
        self.sm_tick = ImageTk.PhotoImage(self.sm_tick_img)
        self.sm_cross_img = Image.open('GUI images/cross.PNG').resize((20, 20))
        self.sm_cross = ImageTk.PhotoImage(self.sm_cross_img)

        self.usb_button = None
        self.wifi_button = None
        self.pass_state = None
        self.graph_canvas = None
        self.tare_icon = None
        self.main_weight = None
        self.main_label = None
        self.sm_weight = None
        self.sm_weight_label = None
        self.sm_count = None
        self.sm_count_label = None
        self.com_port_selection()

        self.controller.send_calibration()

        self.top_frame = tk.Frame(self.main_frame, bg='black', height=190, width=800)
        self.top_frame.pack(fill=BOTH)

        self.bottom_frame = tk.Frame(self.main_frame, bg='black', height=210, width=800)
        self.bottom_frame.pack(fill=BOTH)
        self.button_frame = tk.Frame(self.main_frame, bg='orange', height=100, width=800)
        self.button_frame.pack(fill=BOTH)
        self.bottom_button_frame = tk.Frame(self.main_frame, bg='orange', height=100, width=800)
        self.bottom_button_frame.pack(fill=BOTH)

        self.data1 = Entry(self.bottom_button_frame, width=20)
        self.data2 = Entry(self.bottom_button_frame, width=20)

        self.__make__weights__normal()
        self._make_buttons()
        self.thread_update()

    """
    Creates the wizard like interface for calibration mode
    """
    def cal_window(self):

        cal_window = Toplevel(self)
        cal_window.title("CAL wizard")
        cal_window.geometry("600x100")
        cal_frame = tk.Frame(cal_window, height=100, width=600, bg='orange')
        cal_frame.pack(fill=BOTH)

        self.data1 = Entry(cal_window, width=20)
        self.data2 = Entry(cal_window, width=20)

        input_label1 = tk.Label(cal_window,
                                text='Input first weight (in grams): ', fg='black', bg='orange')
        input_label1.config(font=("Helvetica", 12))
        input_label1.place(x=40, y=10)

        self.data1.place(x=290, y=10)

        enter_button1 = tk.Button(cal_window, text='Enter', command=self.controller.weight_update1)
        enter_button1.place(x=450, y=10)

        input_label2 = tk.Label(cal_window,
                                text='Input second weight (in grams): ', fg='black', bg='orange')
        input_label2.config(font=("Helvetica", 12))
        input_label2.place(x=40, y=60)

        self.data2.place(x=290, y=60)

        enter_button2 = tk.Button(cal_window, text='Enter', command=self.controller.weight_update2)
        enter_button2.place(x=450, y=60)

        cal_button = tk.Button(cal_window, text='Calibrate', command=self.controller.call_cal)
        cal_button.place(x=510, y=10)
        zero_button = tk.Button(cal_window, text='Zero', command=self.controller.call_zero)
        zero_button.place(x=510, y=60)

    """
    Update the displayed weight readings based on what mode is currently used
    """
    def update_displays(self):

        if self.controller.mode != self.controller.new_mode:

            if self.controller.new_mode == GENERAL:
                self.mode_clicked.set("General")
            elif self.controller.new_mode == CALIBRATION:
                self.mode_clicked.set("Calibration")
            elif self.controller.new_mode == COUNTING:
                self.mode_clicked.set("Counting")
            else:
                self.mode_clicked.set("Pass/Fail")

            self.__frame__clear(self.mode_clicked.get())

        # Display the first 5 digits of every number
        self.raw_value['text'] = str(self.controller.weights[0])[0:5]
        self.zero_value['text'] = str(self.controller.weights[1])[0:5]
        self.tare_value['text'] = str(self.controller.weights[2])[0:5]
        if self.controller.weights[3] == OVERLOAD:
            self.meas_value['text'] = "OL"
        else:
            self.meas_value['text'] = str(self.controller.weights[3])[0:5]

        self.light_label['text'] = "Light: {0}".format(self.controller.slider_read)

        if self.controller.cal_icon_update:
            self.cal_icon.config(image=self.calcon)
        else:
            self.cal_icon.config(image='')

        if self.controller.do_cal == 1:
            self.make_plot()
            self.controller.do_cal = 0

        if self.controller.mode == CALIBRATION:
            # Display the first 5 digits of every number
            self.num_val1['text'] = str(self.controller.cal_weights[0])[0:5]
            self.num_val2['text'] = str(self.controller.cal_weights[1])[0:5]
            self.num_val3['text'] = str(self.controller.cal_weights[2])[0:5]
            self.num_val4['text'] = str(self.controller.cal_weights[3])[0:5]

        elif self.controller.mode == COUNTING:
            # Display the first 5 digits of every number
            self.weight_value['text'] = str(self.controller.count_weights[0])[0:5]
            self.count_value['text'] = str(self.controller.count_weights[1])[0:5]
            self.single_value['text'] = str(self.controller.count_weights[2])[0:5]

            if self.controller.weights[3] == OVERLOAD:
                self.current_value['text'] = "OL"
            else:
                self.current_value['text'] = str(self.controller.count_weights[3])[0:5]

            if self.controller.count_weights[0] != 0:
                self.sm_weight.config(image=self.sm_tick)
            else:
                self.sm_weight.config(image=self.sm_cross)

            if self.controller.count_weights[1] != 0:
                self.sm_count.config(image=self.sm_tick)
            else:
                self.sm_count.config(image=self.sm_cross)

        elif self.controller.mode == PASS:
            # Display the first 5 digits of every number
            self.lower_value['text'] = str(self.controller.pass_weights[0])[0:5]
            self.upper_value['text'] = str(self.controller.pass_weights[2])[0:5]

            if self.controller.pass_weights[1] == 1:
                self.status_label['text'] = 'Pass'
                self.pass_state.configure(image=self.tick)
            else:
                self.status_label['text'] = 'Fail'
                self.pass_state.configure(image=self.cross)

        elif self.controller.mode == GENERAL:
            if self.controller.weights[3] == OVERLOAD:
                self.main_weight['text'] = "OL"
            else:
                self.main_weight['text'] = str(self.controller.weights[3])[0:5]

        if self.controller.coms_type == PORT:
            self.usb_button.config(image=self.usb_active, bg='red')
            self.wifi_button.config(image=self.wifi_inactive, bg='white')
        else:
            self.usb_button.config(image=self.usb_inactive, bg='white')
            self.wifi_button.config(image=self.wifi_active, bg='red')

    """
    Creates the main weight that's present during GENERAL weighing mode
    """
    def normal_weight(self):
        self.main_weight = tk.Label(self.bottom_frame, text='0', fg='white', bg='black')
        self.main_weight.place(x=350, y=0)
        self.main_weight.config(font=('Helvetica', '60'))

        self.main_label = tk.Label(self.bottom_frame, text='Weight', fg='white', bg='black')
        self.main_label.place(x=350, y=110)
        self.main_label.config(font=('Helvetica', '30'))

    """
    Clear all the widgets from the display frames
    Set whichever mode is being used
    Update the control interface for whichever mode
    """
    def __frame__clear(self, _event):

        self.controller.mode = self.mode_clicked.get()

        for widget in self.bottom_frame.winfo_children():
            widget.destroy()

        for widget in self.bottom_button_frame.winfo_children():
            widget.destroy()

        if self.mode_clicked.get() == 'Calibration':
            self.controller.mode = CALIBRATION
            self.controller.new_mode = CALIBRATION
            self.controller.model.update_queue.append((USER_MODE, CALIBRATION))
            self.controller.model.update_queue.append((USER_MODE, CALIBRATION))
            self.__make__weights__calibration()
            self.make_plot()
            self.cal_window()

        elif self.mode_clicked.get() == 'Counting':
            self.controller.mode = COUNTING
            self.controller.new_mode = COUNTING
            self.controller.model.update_queue.append((USER_MODE, COUNTING))
            self.controller.model.update_queue.append((USER_MODE, COUNTING))
            self.__make__weights__counting()

        elif self.mode_clicked.get() == 'Pass/Fail':
            self.controller.new_mode = PASS
            self.controller.mode = PASS
            self.controller.model.update_queue.append((USER_MODE, PASS))
            self.controller.model.update_queue.append((USER_MODE, PASS))
            self.__make__weights__pass()

        else:
            self.controller.mode = GENERAL
            self.controller.new_mode = GENERAL
            self.controller.model.update_queue.append((USER_MODE, GENERAL))
            self.controller.model.update_queue.append((USER_MODE, GENERAL))
            self.normal_weight()

        self.controller.evaluate()

    """
    Creates weight readings for calibration mode
    """
    def __make__weights__calibration(self):

        self.num_val1 = tk.Label(self.bottom_frame, text='0', fg='white', bg='black')
        self.num_val1.config(font=('Helvetica', '30'))
        self.num_val1.place(x=20, y=0)
        self.num_val2 = tk.Label(self.bottom_frame, text='0', fg='white', bg='black')
        self.num_val2.place(x=20, y=90)
        self.num_val2.config(font=('Helvetica', '30'))
        self.num_val3 = tk.Label(self.bottom_frame, text='0', fg='white', bg='black')
        self.num_val3.place(x=200, y=0)
        self.num_val3.config(font=('Helvetica', '30'))
        self.num_val4 = tk.Label(self.bottom_frame, text='0', fg='white', bg='black')
        self.num_val4.place(x=200, y=90)
        self.num_val4.config(font=('Helvetica', '30'))

        self.lab_val1 = tk.Label(self.bottom_frame, text='M 1', fg='white', bg='black')
        self.lab_val1.place(x=20, y=50)
        self.lab_val1.config(font=('Helvetica', '18'))
        self.lab_val2 = tk.Label(self.bottom_frame, text='M 2', fg='white', bg='black')
        self.lab_val2.place(x=20, y=150)
        self.lab_val2.config(font=('Helvetica', '18'))
        self.lab_val3 = tk.Label(self.bottom_frame, text='ADC 1', fg='white', bg='black')
        self.lab_val3.place(x=200, y=50)
        self.lab_val3.config(font=('Helvetica', '18'))
        self.lab_val4 = tk.Label(self.bottom_frame, text='ADC 2', fg='white', bg='black')
        self.lab_val4.place(x=200, y=150)
        self.lab_val4.config(font=('Helvetica', '18'))

    """
    Draws the plot that is shown during calibration mode
    """
    def make_plot(self):

        adc, mass = self.controller.model.lin_reg()
        fig = Figure(figsize=(3, 2))
        fig.patch.set_facecolor('0')

        ax = fig.add_subplot(111)
        ax.set_facecolor('0')

        ax.set_ylabel('Weight')
        ax.xaxis.label.set_color('white')
        ax.yaxis.label.set_color('white')

        ax.tick_params(axis='x', colors='white')
        ax.tick_params(axis='y', colors='white')

        ax.spines['left'].set_color('white')
        ax.spines['top'].set_color('white')
        func = self.controller.model.weight_conversion(adc, mass)
        ax.plot(adc, mass, 'wo', adc, [func(x) for x in adc], 'w')

        canvas = FigureCanvasTkAgg(fig, self.bottom_frame)
        canvas.draw()
        canvas.get_tk_widget().place(x=400, y=0)

    """
    Creates weight readings for pass/fail mode
    """
    def __make__weights__pass(self):

        self.lower_value = tk.Label(self.bottom_frame, text='0', fg='white', bg='black')
        self.lower_value.config(font=('Helvetica', '48'))
        self.lower_value.place(x=110, y=0)
        self.pass_state = tk.Label(self.bottom_frame, image=self.tick, bg='black')
        self.pass_state.place(x=360, y=0)
        self.upper_value = tk.Label(self.bottom_frame, text='0', fg='white', bg='black')
        self.upper_value.place(x=610, y=0)
        self.upper_value.config(font=('Helvetica', '48'))

        self.lower_label = tk.Label(self.bottom_frame, text='Lower', fg='white', bg='black')
        self.lower_label.place(x=110, y=110)
        self.lower_label.config(font=('Helvetica', '24'))
        self.status_label = tk.Label(self.bottom_frame, text='Pass', fg='white', bg='black')
        self.status_label.place(x=360, y=110)
        self.status_label.config(font=('Helvetica', '24'))
        self.upper_label = tk.Label(self.bottom_frame, text='Upper', fg='white', bg='black')
        self.upper_label.place(x=610, y=110)
        self.upper_label.config(font=('Helvetica', '24'))

        self.calibration_mode()

    """
    Display weights for counting mode
    """
    def __make__weights__counting(self):

        self.weight_value = tk.Label(self.bottom_frame, text='0', fg='white', bg='black')
        self.weight_value.place(x=20, y=0)
        self.weight_value.config(font=('Helvetica', '48'))
        self.count_value = tk.Label(self.bottom_frame, text='0', fg='white', bg='black')
        self.count_value.place(x=220, y=0)
        self.count_value.config(font=('Helvetica', '48'))
        self.single_value = tk.Label(self.bottom_frame, text='0', fg='white', bg='black')
        self.single_value.place(x=420, y=0)
        self.single_value.config(font=('Helvetica', '48'))
        self.current_value = tk.Label(self.bottom_frame, text='0', fg='white', bg='black')
        self.current_value.place(x=620, y=0)
        self.current_value.config(font=('Helvetica', '48'))

        self.weight_label = tk.Label(self.bottom_frame, text='Weight', fg='white', bg='black')
        self.weight_label.place(x=20, y=110)
        self.weight_label.config(font=('Helvetica', '30'))
        self.count_label = tk.Label(self.bottom_frame, text='Count', fg='white', bg='black')
        self.count_label.place(x=220, y=110)
        self.count_label.config(font=('Helvetica', '30'))
        self.single_label = tk.Label(self.bottom_frame, text='Single', fg='white', bg='black')
        self.single_label.place(x=420, y=110)
        self.single_label.config(font=('Helvetica', '30'))
        self.current_label = tk.Label(self.bottom_frame, text='Current', fg='white', bg='black')
        self.current_label.place(x=620, y=110)
        self.current_label.config(font=('Helvetica', '30'))

        self.counting_input()

    """
    Creates weights display for all normal inputs
    """
    def __make__weights__normal(self):

        self.raw_value = tk.Label(self.top_frame, text='0', fg='white', bg='black')
        self.raw_value.place(x=20, y=50)
        self.raw_value.config(font=('Helvetica', '24'))
        self.raw_label = tk.Label(self.top_frame, text='Raw', fg='white', bg='black')
        self.raw_label.place(x=20, y=100)
        self.raw_label.config(font=('Helvetica', '20'))

        self.zero_value = tk.Label(self.top_frame, text='0', fg='white', bg='black')
        self.zero_value.place(x=220, y=50)
        self.zero_value.config(font=('Helvetica', '24'))
        self.zero_label = tk.Label(self.top_frame, text='Zero', fg='white', bg='black')
        self.zero_label.place(x=220, y=100)
        self.zero_label.config(font=('Helvetica', '20'))

        self.tare_value = tk.Label(self.top_frame, text='0', fg='white', bg='black')
        self.tare_value.place(x=420, y=50)
        self.tare_value.config(font=('Helvetica', '24'))
        self.tare_label = tk.Label(self.top_frame, text='Tare', fg='white', bg='black')
        self.tare_label.place(x=420, y=100)
        self.tare_label.config(font=('Helvetica', '20'))

        self.meas_value = tk.Label(self.top_frame, text='0', fg='white', bg='black')
        self.meas_value.place(x=620, y=50)
        self.meas_value.config(font=('Helvetica', '24'))
        self.meas_label = tk.Label(self.top_frame, text='Meas.', fg='white', bg='black')
        self.meas_label.place(x=620, y=100)
        self.meas_label.config(font=('Helvetica', '20'))

        if self.controller.mode == GENERAL:
            self.normal_weight()

    """
    Creates generic buttons that will be used regardless of mode
    """
    def _make_buttons(self):

        mode_label = tk.Label(self.button_frame, text='Mode: ', fg='black', bg='orange')
        mode_label.config(font=("Helvetica", 16))
        mode_label.place(x=50, y=30)
        mode_button = OptionMenu(self.button_frame, self.mode_clicked, *self.modes, command=self.__frame__clear)
        mode_button.place(x=150, y=30)

        unit_label = tk.Label(self.button_frame, text='Units: ', fg='black', bg='orange')
        unit_label.config(font=("Helvetica", 16))
        unit_label.place(x=550, y=30)
        unit_button = OptionMenu(self.button_frame, self.units_clicked, *self.units, command=self.controller.set_units)
        unit_button.place(x=650, y=30)

        tare_button = tk.Button(self.button_frame, text='   Tare    ', command=self.controller.call_tare)
        tare_button.place(x=650, y=70)

        clear_tare = tk.Button(self.button_frame, text='Clear Tare', command=self.controller.clear_tare)
        clear_tare.place(x=710, y=70)

        self.tare_icon = tk.Label(self.button_frame, bg='orange')
        self.tare_icon.place(x=710, y=30)

        self.cal_icon = tk.Label(self.button_frame, bg='orange')
        self.cal_icon.place(x=740, y=30)

        inc_light = tk.Button(self.button_frame, text='Inc. light', command=self.controller.inc_light)
        inc_light.place(x=370, y=50)

        dec_light = tk.Button(self.button_frame, text='Dec. light', command=self.controller.dec_light)
        dec_light.place(x=300, y=50)

        ref_but = tk.Button(self.button_frame, text='Refresh', command=self.controller.refresh)
        ref_but.place(x=150, y=70)

        self.light_label = tk.Label(self.button_frame, bg='orange', text="Light: ")
        self.light_label.config(font=("Helvetica", 12))
        self.light_label.place(x=440, y=50)

    """
    Creates the input buttons required for counting mode
    One button to capture the reference weight, and an input for selecting reference count
    """
    def counting_input(self):

        self.data1 = Entry(self.bottom_button_frame, width=20)
        input_label1 = tk.Label(self.bottom_button_frame, text='1) Input reference count: ', fg='black',
                                bg='orange')
        input_label1.config(font=("Helvetica", 12))
        input_label1.place(x=150, y=10)

        self.data1.place(x=350, y=10)

        input_label2 = tk.Label(self.bottom_button_frame, text='2) Capture ref. weight and calculate: ', fg='black',
                                bg='orange')
        input_label2.config(font=("Helvetica", 12))
        input_label2.place(x=150, y=50)

        enter_button1 = tk.Button(self.bottom_button_frame, text='Enter', command=self.controller.weight_update1)
        enter_button1.place(x=500, y=10)

        enter_button2 = tk.Button(self.bottom_button_frame, text='Capture', command=self.controller.weight_update2)
        enter_button2.place(x=420, y=50)

        enter_button3 = tk.Button(self.bottom_button_frame, text='Reset', command=self.controller.restart_count)
        enter_button3.place(x=500, y=50)

        self.sm_weight = tk.Label(self.bottom_button_frame, image='', bg='orange')
        self.sm_weight.place(x=570, y=50)

        self.sm_weight_label = tk.Label(self.bottom_button_frame, text='weight', bg='orange')
        self.sm_weight_label.place(x=600, y=50)
        self.sm_weight_label.config(font=('Helvetica', '12'))

        self.sm_count = tk.Label(self.bottom_button_frame, image='', bg='orange')
        self.sm_count.place(x=570, y=10)

        self.sm_count_label = tk.Label(self.bottom_button_frame, text='count', bg='orange')
        self.sm_count_label.place(x=600, y=10)
        self.sm_count_label.config(font=('Helvetica', '12'))

    """
    Creates buttons to be used for calibration and pass/fail mode
    Has slightly different label updates for either mode
    Functionality is identical between modes
    """
    def calibration_mode(self):

        self.data1 = Entry(self.bottom_button_frame, width=20)
        self.data2 = Entry(self.bottom_button_frame, width=20)

        input_label1 = tk.Label(self.bottom_button_frame,
                                text='Input first weight (in grams): ', fg='black', bg='orange')
        input_label1.config(font=("Helvetica", 12))
        input_label1.place(x=150, y=10)

        self.data1.place(x=350, y=10)

        enter_button1 = tk.Button(self.bottom_button_frame, text='Enter', command=self.controller.weight_update1)
        enter_button1.place(x=500, y=10)

        input_label2 = tk.Label(self.bottom_button_frame,
                                text='Input second weight (in grams): ', fg='black', bg='orange')
        input_label2.config(font=("Helvetica", 12))
        input_label2.place(x=150, y=60)

        self.data2.place(x=350, y=60)

        enter_button2 = tk.Button(self.bottom_button_frame, text='Enter', command=self.controller.weight_update2)
        enter_button2.place(x=500, y=60)

        input_label1['text'] = "Input lower weight"
        input_label2['text'] = "Input higher weight"

    """
    Creates thread to send information over selected coms line to STM32
    """
    def thread_update(self):

        if self.controller.coms_type == PORT:
            connected = 0
            for p in list(serial.tools.list_ports.comports()):
                if self.controller.connected_port == p.device:
                    connected = 1
                    thread = threading.Thread(target=self.controller.uart_send)
                    thread.start()
                    thread.join()
            if connected == 0:

                self.controller.serial_port = None
                self.controller.connected_port = None

            self.update_displays()
            # Wait for an amount of time before repeating
            self.after(200, self.thread_update)

        else:
            if self.controller.wifi_connected:
                thread = threading.Thread(target=self.controller.wifi_send)
                thread.start()
                thread.join()
            self.update_displays()
            # Wait for an amount of time before repeating
            self.after(300, self.thread_update)

    """
    Triggered when USB button is clicked
    Changes coms type to serial, and updates button logos
    """
    def swap_to_srl(self):
        self.controller.coms_type = PORT
        self.usb_button = tk.Button(self.port_frame, image=self.usb_active, command=self.swap_to_srl, bg='red')
        self.usb_button.place(x=550, y=20)
        self.wifi_button = tk.Button(self.port_frame, image=self.wifi_inactive, command=self.swap_to_wifi, bg='white')
        self.wifi_button.place(x=590, y=20)

        self.controller.model.update_queue.append((MODE, PORT))
        self.controller.model.update_queue.append((MODE, PORT))

    """
    Triggered when wifi button is clicked
    Changes coms type to wifi, and updates button logos
    """
    def swap_to_wifi(self):
        self.controller.coms_type = WIFI
        self.usb_button = tk.Button(self.port_frame, image=self.usb_inactive, command=self.swap_to_srl, bg='white')
        self.usb_button.place(x=550, y=20)
        self.wifi_button = tk.Button(self.port_frame, image=self.wifi_active, command=self.swap_to_wifi, bg='red')
        self.wifi_button.place(x=590, y=20)

        self.controller.model.update_queue.append((MODE, WIFI))
        self.controller.model.update_queue.append((MODE, WIFI))
        self.controller.cal_icon_update = 1

    """
    Initialisation of communication buttons
    """
    def com_type_selection(self):
        self.usb_button = tk.Button(self.port_frame, image=self.usb_active, command=self.swap_to_srl, bg='red')
        self.usb_button.place(x=550, y=20)
        self.wifi_button = tk.Button(self.port_frame, image=self.wifi_inactive, command=self.swap_to_wifi, bg='white')
        self.wifi_button.place(x=590, y=20)

    def coms_update(self):
        for widgets in self.port_frame.winfo_children():
            widgets.destroy()

        com_port_selected = tk.Label(self.port_frame, text="Coms Port: ", fg='white', bg='black')
        com_port_selected.place(x=120, y=20)
        com_port_selected.config(font=("Helvetica", 16))

        com_refresh = tk.Button(self.port_frame, text='ref. coms', command=self.coms_update)
        com_refresh.place(x=20, y=20)

        self.com_type_selection()

        default = StringVar(self.port_frame, "Select Port")
        self.ports_options = serial.tools.list_ports.comports()
        if self.ports_options != 0:
            self.all_ports = OptionMenu(self.port_frame, default, *self.ports_options,
                                        command=self.controller.change_com_port) \
                .place(x=250, y=20)
        else:
            tk.Label(self.port_frame, text="No comm-ports available",
                     relief='groove') \
                .place(x=250, y=20)

    """
    Creates the option menu to select which port to connect to
    """
    def com_port_selection(self):

        com_refresh = tk.Button(self.port_frame, text='ref. coms', command=self.coms_update)
        com_refresh.place(x=20, y=20)

        com_port_selected = tk.Label(self.port_frame, text="Coms Port: ", fg='white', bg='black')
        com_port_selected.place(x=120, y=20)
        com_port_selected.config(font=("Helvetica", 16))

        """
                        Note: port will not be visible until device is plugged in
                        """

        # creates the options menu drop down
        default = StringVar(self.port_frame, "Select Port")
        self.ports_options = serial.tools.list_ports.comports()
        if self.ports_options != 0:
            self.all_ports = OptionMenu(self.port_frame, default, *self.ports_options,
                       command=self.controller.change_com_port) \
                .place(x=250, y=20)
        else:
            tk.Label(self.port_frame, text="No comm-ports available",
                     relief='groove') \
                .place(x=250, y=20)

        self.com_type_selection()

