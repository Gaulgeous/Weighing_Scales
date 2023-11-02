import matplotlib.pyplot as plt
import numpy as np

adc_readings = [1, 2, 3, 4]
weight_readings = [1, 2, 3, 4]

def make_plot():
    coef = np.polyfit(adc_readings, weight_readings, 1)
    poly = np.poly1d(coef)
    fig = plt.figure(figsize=(3, 2))
    fig.patch.set_facecolor('0')

    ax = fig.add_subplot(111)
    ax.set_facecolor('0')

    ax.set_xlabel('ADC')
    ax.set_ylabel('Weight')

    ax.xaxis.label.set_color('white')
    ax.yaxis.label.set_color('white')

    ax.tick_params(axis='x', colors='white')
    ax.tick_params(axis='y', colors='white')

    ax.spines['left'].set_color('white')
    ax.spines['top'].set_color('white')

    plt.plot(adc_readings, weight_readings, 'wo', adc_readings, poly(adc_readings), 'w')
    plt.show()

if __name__ == '__main__':
    make_plot()
