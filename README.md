### FIT demo firmwares

This repository contains simple firmwares for reading or interacting with
sensor nodes using the CoAP procotol. The available firmwares are:
* [node_bmp180](./firmwares/node_bmp180): read environmental values from a
  [BMP180](https://www.bosch-sensortec.com/bst/products/all_products/bmp180) sensor.
  The sensor has to be plugged on a SAMR21 Xplained Pro board;
* [node_leds](./firmwares/node_leds): interact with the on-board LED using CoAP.
  By default, the firmware is built for a SAMR21 Xplained Pro board;
* [node_leds](./firmwares/node_leds_xbee): same as `node_leds` but by default the
  firmware is built to run on an Arduino Zero board with an XBee shield;
* [node_imu](./firmwares/node_imu): read the inertial measurement unit of an
  IoTLAB-M3 board;
* [node_iotlab_a8_m3](./firmwares/node_iotlab_a8_m3): interact with M3 LED of an
  A8 node in the IoTLAB testbed;
* [node_ioxplained](./firmwares/ioxplained): read the temperature sensor of an
  IO1 Xplained extension board. The firmware is built for a SAMR21 Xplained Pro
  board;
* [node_tsl2561](./firmware/node_tsl2561): read the illuminance value (lx) from
  a
  [TSL2561](http://ams.com/eng/Products/Light-Sensors/Ambient-Light-Sensors/TSL2561/TSL2560-TSL2561-Datasheet)
  sensor. The firmware is built for a SAMR21 Xplained Pro board.

The firmwares source code are based on [RIOT](https://github.com/RIOT-OS/RIOT).

### Initializing the repository:

RIOT is included as a submodule of this repository. We provide a `make` helper
target to initialize it. From the root of this repository, issue the following
command:
```
$ make init_submodules
```

### Build the firmwares:

From the root directory of this repository, simply issue the following command:
```
$ make
```

### Cleanup the generated firmwares

From the root directory of this repository, issue the following command:
```
* make clean
```
