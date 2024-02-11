Overview
========

This project is a work in progress of an IPv6 implementation for memory
constrained Arduino devices. The objective is mainly to be able to operate a
COAP / UDP / IPv6 protocol stack on ATmega 328p microcontrollers with 2KB SRAM,
for the use-cases of sensor data feedback and telemetry. There is no intention
to provide a general-purpose IPv6 stack for embedded devices nor to implement
complex protocols such as TCP or HTTP, as such protocols would require memory
caching, streaming, and timer management.

Due to the constrained memory, this IPv6 implementation tries to find a
trade-off between memory consumption and usability, which is probably not
reached yet. It intendedly not uses a classical BSD-sockets API, since such and
abstraction would require caching received packets before their delivery to the
caller and would thus entirely consume the available memory.

This project is implemented in C, tests are implemented in Python. It is based
on Arduino libraries, though it can be easily ported to another platform by
implementing the platform.cpp file.


Development status
==================

This project is in draft status. It is currently unitarily tested and checked
for protocolar compliance, though it has not been tested on real applications
yet.

Some features are known missing or not-functional yet. The project suffers some
design flaws that may require future changes in the API to meet the objectives
of the project.

Hardware support modules have been implemented to allow ethernet communication.

The following hardware modules have been implemented:
* hw_w5500: Sending/receiving Ethernet frames using the W5500 Ethernet Shield
* hw_serial: Sending/receiving Ethernet frames over the USB-serial interface
(for protocol testing)
 

Requirements
============

This project is developed primarily for Arduino devices and currently relies on
the Arduino HW and SW ecosystem. More platforms might be supported later.

Hardware requirements:
* Arduino Uno
* W5500 Ethernet Shield

Arduino dependencies:
* Arduino SPI
* Arduino Serial

Python dependencies (for testing):
* Python 2.7 (should be portable to Python 3 without trouble)
* pyserial 3.4
* scapy


Usage
=====

Due to the WIP status of this implementation, this documentation only provides
steps necessary to execute the tests.

Sample projects
---------------

Example projects are provided:
* test_net.ino: Protocol testing (to be used with tester.py)
* test_eth.ino: Allows to test the W5500 Ethernet driver
* test_real.ino: Allows to test a real communication over the network

Configuring
-----------

Before compiling, you must setup the implementation using the config.h file. In
particular, the NET_MAC_PROTO_LOWER() macro shall be set to the proper value,
and the includes must be adapted, depending on the HW used.

For ethernet over serial interface (protocol testing):

```
#define NET_MAC_PROTO_LOWER(SUFFIX)  hw_serial ## SUFFIX
(…)
#include "hw_serial.h"
```

For W5500 hardware:

```
#define NET_MAC_PROTO_LOWER(SUFFIX)  hw_w5500 ## SUFFIX
(…)
#include "hw_w5500.h"
```


Compiling
---------

The project is currently organised to be built using Arduino Studio:
1. Create a zip file of the project root directory
2. In Arduino studio, open one of the test samples (test_eth.ino, test_net.ino,
test_real.ino)
3. Select « Add a library Zip », and select the zip file of the project
4. Compile the project
5. Flash to your device

Note: This constrainst is due to the way Arduino manages its libraries.
Compiling and flashing from command-line should be possible and will be
supported in a future version.


Protocol testing
================

The protocol testing system is based on the project `test_net.ino` and the
script `tester.py`.


The protocol testing system is composed of the following elements:

```
+---------------+             +------------------+
|  +-----------+|             |+--------------+  |
|  | tester.py ||  <——USB——>  || test_net.ino |  |
|  +-----------+|             |+--------------+  |
|               |             |                  |
|      PC       |             |      Arduino     |
+--------------—+             +—-----------------+
```


When flashing the `test_net.ino` program on the Arduino, it listens for
commands over the USB-serial interface. The test suite is executed by the
`tester.py` script which connects to the USB-serial port and drives the
execution of tests. The tester.py script and the Arduino then use the serial
port to exchange Ethernet frames for the purpose of the test.


To execute the test suite, proceed as follows:
1. Flash the test program on the Arduino
2. Make sure the serial console is disconnected in Arduino Studio
3. Set the serial port configuration variables SERIAL_PORT and
SERIAL_BAUDRATE in tester.py
4. Run the tester.py script (make sure your user has the right to connect to
the serial port or use sudo)


Memory consumption
==================

This section is a Work-in-progress.

* Serial (USE_SERIAL): 180 bytes
* SPI (USE_SPI): 17 bytes
