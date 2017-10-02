## DHT22 NVidia Jetson Python
A python library to read temperature from your DHT22 sensor connected on GPIO pins of your Jetson Development Kit

Based on https://github.com/jetsonhacks/jetsonTX1GPIO
 * Copyright (c) 2015 JetsonHacks
 * All rights reserved.

### Prerequisites
Install python-dev
```
 sudo apt-get install python-dev
```

### Setup
Edit C_DHT.c to edit the pin number you wish to use to communicate with your DHT22.
Default set to:
 ```
 #define PIN0 gpio 388  //2-pin from bottom left
 #define PIN1 gpio 398  //6-th pin from bottom left
 ```
If you don't know the pin numbers google "Jetson gpio pinout tk1/tx1/tx2"

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
 import C_DHT
 C_DHT.readSensor(0)
 ```


### Other
The library is written in C so it should run faster than python implementation would. DHT22 sends signals which need to be caught every 50 microseconds (not mili, micro).

The function C_DHT.readSensor(pinNumber) returns (temperature, humidity, 0 or 1) triplet as result.
If the last value is 0 the communication failed so try again in 2 seconds. If 1 simply read the temperature and humidity.


The code sets the communication w/ the sensor on its own thread so it minimizes the chances of interrupt.
Don't expect to get a reading every time you try to call the function. Give it a few times.

You can also modify it to read from DHT11 or DHT21 as well

Datasheet on: https://cdn-shop.adafruit.com/datasheets/Digital+humidity+and+temperature+sensor+AM2302.pdf