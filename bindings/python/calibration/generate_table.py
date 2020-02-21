import argparse
import libm2k
import signal
import csv
import sys
import os


def create_table(path, calibration_values):
    if os.path.isfile(path) is False:
        f = open(path, 'w+')
        f.close()

    with open(path, mode='r') as file:
        reader = csv.reader(file)
        for row in reader:
            param = (float(row[0]), [float(row[i]) for i in range(1, len(row))])
            calibration_values[param[0]] = param[1]


def write_in_file(path, calibration_values):
    with open(path, mode='w') as file:
        writer = csv.writer(file, delimiter=',', quotechar='"', quoting=csv.QUOTE_MINIMAL)
        for key, value in sorted(calibration_values.items()):
            writer.writerow([key] + value)


def get_key_value(calibration_params):
    parameters = []
    for i in range(len(calibration_params[1])):
        parameters.append(calibration_params[1][i])
    return calibration_params[0], parameters


def generate_lookup_table(ctx, calibration_values, path, max_temperature):
    print("'CTRL + C' to stop the calibration data extraction process")
    create_table(path, calibration_values)
    i = 0
    while True:
        ctx.calibrateADC()
        ctx.calibrateDAC()

        key, value = get_key_value(ctx.getCalibrationParameters())
        if key >= max_temperature:
            break

        calibration_values[key] = value
        print('Temperature_' + str(i) + ': ' + str(key) + '\u2103')
        i += 1
    write_in_file(path, calibration_values)


def validate_table_values(ctx, path):
    print('NOT IMPLEMENTED')


def main():
    parser = argparse.ArgumentParser(prog='m2kcli',
                                     description='Calibration control of ADALM2000 from the command line',
                                     usage='%(prog)s [-h | --help] [-v | --version]')

    parser.add_argument('uri',
                        action='store',
                        metavar='uri',
                        type=str,
                        help='describe the context location ')

    parser.add_argument('-t', '--table',
                        action='store',
                        metavar='path',
                        nargs='*',
                        type=str,
                        help='generate the lookup table containing the calibration parameters')

    parser.add_argument('-v', '--validate',
                        action='store',
                        metavar='path',
                        type=str,
                        help='validate the accuracy of the calibration data from the given table')

    parser.set_defaults(func=parse_arguments)

    args = parser.parse_args()
    args.func(args)


def parse_arguments(args):
    try:
        if args.uri == 'auto':
            ctx = libm2k.m2kOpen()
        else:
            ctx = libm2k.m2kOpen(args.uri)
        if ctx is None:
            raise Exception('Invalid uri')
        print('Connection established')

        if args.table is not None:
            calibration_values = dict()

            if len(args.table) > 0:
                max_temperature = float(args.table[0])
            else:
                max_temperature = 75.0

            if len(args.table) > 1:
                path = args.table[1]
            else:
                path = 'calibration_' + str(ctx.getSerialNumber())

            def signal_handler(sig, frame):
                nonlocal calibration_values, path
                write_in_file(path, calibration_values)
                libm2k.contextClose(ctx)
                sys.exit(0)
            signal.signal(signal.SIGINT, signal_handler)

            generate_lookup_table(ctx, calibration_values, path, max_temperature)
        elif args.validate is not None:
            validate_table_values(ctx, args.validate)

        libm2k.contextClose(ctx)
        return 0
    except Exception as error:
        print(error)
        return -1


if __name__ == "__main__":
    main()
