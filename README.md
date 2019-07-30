## DHT22 NVidia Jetson Python
A python library to read temperature from your DHT22 sensor connected on GPIO pins of your Jetson Development Kit.

## DHT11 support
Also added a DHT11 sensor read - However this should be tested by someone.

## List of supported Jetson boards
* TX1
* TX2
* Xavier
* Nano

Check the `jetsonGPIO/jetsonGPIO.h`  for defines. It should be obvious which is which.

### Prerequisites
Install python-dev
```
 sudo apt-get install python-dev
```

This repository relies on a fork I made. To clone the repository use:
```
 git clone --recurse-submodules -j8 https://github.com/GrgoMariani/NVidia-Jetson-DHT22-Python
```

### Setup
Edit C_DHT.c to edit the pin number you wish to use to communicate with your DHT22.
Default set to:
 ```
 #define PIN0 jetsontx2_pin37  //2-pin from bottom left
 #define PIN1 jetsontx2_pin29  //6-th pin from bottom left
 ```
If you don't know the pin numbers google __Jetson gpio pinout tk1/tx1/tx2/xavier__

### Install
Download project and install the python library:
 ```
 cd /path/to/this/dir/
 sudo python setup.py build
 sudo python setup.py install
 ```

Now your python library is set up on your Jetson.

### Usage
To read sensor enter python as superuser

 ```
 sudo python
 ```
 Once in python
 ```python
 import C_DHT
 C_DHT.readSensor(0)        # if used with DHT22, read first sensor
 C_DHT.readSensor(1)        # if used with DHT22, read second sensor
 C_DHT.readSensorDHT11(0)   # if used with DHT11 sensors
 C_DHT.readSensorDHT11(1)   # if used with DHT11 sensors
 ```


### Other
The library is written in C so it should run faster than python implementation would. DHT22 sends signals which need to be caught every 50 microseconds (not mili, micro).

The function C_DHT.readSensor(pinNumber) returns (temperature, humidity, 0 or 1) triplet as result.
If the last value is 0 the communication failed so try again in 2 seconds. If 1 simply read the temperature and humidity.


The code sets the communication w/ the sensor on its own thread so it minimizes the chances of interrupt.
Don't expect to get a reading every time you try to call the function. Give it a few times.

~~You can also modify it to read from DHT11 or DHT21 as well.~~ (Done recently)

Datasheet on: https://cdn-shop.adafruit.com/datasheets/Digital+humidity+and+temperature+sensor+AM2302.pdf