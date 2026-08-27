# Hardware Setup

The sensor uses the following components:

- [Arduino UNO R4 WiFi](https://docs.arduino.cc/hardware/uno-r4-wifi/)
- [MKR ENV Shield](https://docs.arduino.cc/hardware/mkr-env-shield/)
- [Sensirion SEN55](https://sensirion.com/products/catalog/SEN55)
- [SparkFun Qwiic CO<sub>2</sub>](https://docs.sparkfun.com/SparkFun_Qwiic_CO2_Sensor_SCD4X/introduction/)
- [Soldered Ozone Sensor MQ131 Breakout](https://soldered.com/products/ozone-sensor-mq131-breakout)

The only component necessary for functionality of the device is the UNO R4 WiFi. All others can be removed without affecting the functionality of other components.

Connection information is available on the documentation pages for all the sensors except for the MKR ENV Shield. To connect the MKR ENV Shield to the UNO R4 WiFi, connect the following inputs to outputs (UNO R4 WiFi is on the left; MKR ENV Shield is on the right):

- `scl -> scl`
- `sda -> sda`
- `10 -> 10 (MISO)`
- `9 -> 9 (SCK)`
- `8 -> 8 (MOSI)`
- `7 -> 7`
- `6 -> 6`
- `4 -> 4`

- `RESET -> RESET`
- `3.3V -> VCC`
- `VIN -> VIN`
- `5V -> 5V`
- `A2 -> A2`

# Installation

Connect the USB-C port on the UNO R4 WiFI to a computer. Using either the Arduino IDE or Arudino CLI, connect to the board and upload the code to the board. Make sure you have the following libraries downloaded:

- `Arduino.h`
- `Arduino_MKRENV.h`
- `ArduinoJson.h`
- `MQ131.h`
- `NTPClient.h`
- `SD.h`
- `SensirionI2cSps30.h`
- `SparkFun_SCD4x_Arduino_Library.h`
- `SPI.h`
- `WiFiS3.h`
- `WiFiUdp.h`
- `Wire.h>`

# Setup

## WiFi Connection

Create a text file named `networkInfo.txt`. On line 1, write the name of the network to which you intend to connect, and on the second line, write the password of the network to which you intend to connect. Both lines should end with `\r\n`. If you're on Windows, make sure that after the first two lines, there is one empty line. On other systems, check each return to make sure it is of the correct type (and not `\r` or `\n`).

If the SD Card reader is not working, you can edit the network ID and password in the code by searching for `char ssid[]` and `char pass[]` for the ID and password, respectively. This will require recompiling the code, as seen in [Installation](#installation).

You can find the IP of the sensor bby monitoring the connections to your router.

## Power

Connect the USB-C port on the Arduino UNO R4 WiFi to any power source. For debugging, you can connect it to a power source with a serial display.