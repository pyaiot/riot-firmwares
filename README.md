### RIOT firmwares for [Pyaiot](https://github.com/pyaiot/pyaiot)

[![Build Status](https://travis-ci.org/pyaiot/riot-firmwares.svg?branch=master)](https://travis-ci.org/pyaiot/riot-firmwares)

This repository contains simple firmwares for reading or interacting with
sensor nodes using CoAP and MQTT-SN procotols.

The firmwares are designed to be used with the
[Pyaiot](https://github.com/pyaiot/pyaiot) transport and display backend.

The available firmwares are:
* [BMP180 sensor (CoAP)](apps/node_bmp180): read environmental values from a
  [BMP180](https://www.bosch-sensortec.com/bst/products/all_products/bmp180)
  sensor.
  The sensor has to be plugged on a SAMR21 Xplained Pro board
* [Weather sensor (CoAP)](apps/node_bme180): read weather values (temperature,
  pressure, humidity) from a
  [BME280](https://www.bosch-sensortec.com/bst/products/all_products/bme280)
  sensor
* [LED control (CoAP)](apps/node_leds): interact with the on-board LED using CoAP.
  By default, the firmware is built for an Atmel SAMR21 Xplained Pro board
  (inverted LED)
* [IMU sensor (CoAP)](aps/node_imu): read the inertial measurement unit of an
  IoTLAB-M3 board
* [IoT-Lab A8-M3 node (CoAP)](apps/node_iotlab_a8_m3): interact with M3 LED of an
  A8 node in the IoTLAB testbed
* [Atmel IO1 Xplained sensor (CoAP)](apps/node_io1_xplained): read the temperature
  sensor of an IO1 Xplained extension board. The firmware is built for a SAMR21
  Xplained Pro board
* [Light sensor (CoAP)](apps/node_tsl2561): read the illuminance value (lx) from a
  [TSL2561](http://ams.com/eng/Products/Light-Sensors/Ambient-Light-Sensors/TSL2561/TSL2560-TSL2561-Datasheet)
  sensor.
* [BME280 sensor (MQTT-SN)](apps/node_mqtt_bme280): read environmental values
  from a
  [BME280](https://www.bosch-sensortec.com/bst/products/all_products/bme280)
  sensor. Values are raised using the MQTT-SN protocol.

All firmwares source codes are based on [RIOT](https://github.com/RIOT-OS/RIOT).

#### Initializing the repository:

RIOT is included as a submodule of this repository. We provide a `make` helper
target to initialize it.
From the root of this repository, issue the following command:
```
$ make init_submodules
```

#### Building the firmwares:

From the root directory of this repository, simply issue the following command:
```
$ make
```

#### Flashing the firmwares

For each firmwares use the RIOT way of flashing them. For example, in
`apps/node_bmp180`, use:
```
$ make -C apps/node_bmp180 flash
```
to flash the firmware on a SAMR21 XPlained Pro board.

#### Global cleanup of the generated firmwares

From the root directory of this repository, issue the following command:
```
$ make clean
```
