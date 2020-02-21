import libm2k
import statistics
import numpy as np


def print_samples(analog_in):
    samples = analog_in.getSamples(100)
    print('Average value ch_1: ' + str(statistics.mean(samples[0])))
    print('Average value ch_2: ' + str(statistics.mean(samples[1])))
    print()


def main():
    ctx = libm2k.m2kOpen()

    analog_out = ctx.getAnalogOut()

    analog_out.setSampleRate(0, 750000)
    analog_out.setSampleRate(1, 750000)
    analog_out.enableChannel(0, True)
    analog_out.enableChannel(1, True)
    analog_out.setCyclic(True)

    x = np.linspace(-np.pi, np.pi, 1024)
    buffer_ch2 = np.sin(x)

    buffer_ch1 = []
    for i in range(512):
        buffer_ch1.append(2)
    for i in range(512):
        buffer_ch1.append(-2)

    buffer = [buffer_ch1, buffer_ch2]

    analog_in = ctx.getAnalogIn()

    analog_in.enableChannel(0, True)
    analog_in.enableChannel(1, True)
    analog_in.setSampleRate(100000)
    analog_in.setRange(0, -10, 10)

    print_samples(analog_in)
    analog_out.push(buffer)

    # wait for user input in order to calibrate the ADC and the DACs
    input()
    analog_out.stop()

    if ctx.calibrateAllFromFile() is not True:
        print("Could not find calibration data")
        return

    print_samples(analog_in)

    analog_out.push(buffer)

    # wait for user input in order to stop the generation
    input()
    analog_out.stop()
    libm2k.contextClose(ctx)


if __name__ == '__main__':
    main()
