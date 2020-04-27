# ADALM2000 calibrated using predefined values

## Generating your own calibration fil
### Command
    generate_table <uri> -t [temperature] [file_path]

### Examples
Create a CSV file 'calibration_<serial_number>'. The process can be stopped by
pressing 'CTRL + C' or when the temperature of the board rises up to 75 °C

    python3 generate_table.py auto -t

Create a CSV file 'my_file.csv'. The process can be stopped by
pressing 'CTRL + C' or when the temperature of the board rises up to 54 °C

    python3 generate_table.py auto -t 54 my_file.cs


## Calibrating using a file
The purpose of this calibration type is to automate and to make the calibration
process more efficient. Please make sure to have a valid calibration file.
There are 3 calibration methods: calibrateADCFromFile, calibrateDACFromFile,
calibrateAllFromFile. For a better understanding please have a look at the given
example: calibrate_from_table.py
