# IoT_PowerSocketOutlet
A simple IoT device based on NodeMCU with ESP-8266.

The aim of this project was to test out several concepts and technologies, like IoT with MQTT communication and embedded computing via Node MCU (ESP-8266).

The wiring diagrams were done using KiCad.

## Functionalities

The device has four power socket outlets. Every socket has an LED indicator whether it is turned off or on and also a switch to turn it on or off. The software connects to an MQTT server, where it can be controlled (turn specific power sockets on or off) and request the status of the ports.

## Hardware

The main computing is performed by the Node MCU (ESP-8266) microcomputer. It is paired with a 4-way relay module and a power supply module (220V AC to 5V DC).

The only custom wiring is a low pass filter for the push buttons and was done on a prototyping board. I prefer a low pass filter for the debounce instead of a software implementation, altough, it is not a perfect solution.

The indicator LEDs are connected to the relay module's selector input. This way there is no need to maintain separate ports for the LEDs and the selectors.

The external power supply is connected both to the Node MCU (via the VIN and G ports) and the relay board. The relays need 5V to be able to operate, and the Node MCU is only able to provide 3.3V which is sufficient for the switching impulse, but not for powering the relays.

## Software

The software was done via the Arduino IDE. It uses the [PubSubClient](https://www.arduino.cc/reference/en/libraries/pubsubclient) and the [ESP8266WiFi](https://github.com/esp8266/Arduino/tree/master/libraries/ESP8266WiFi) libraries. The code builds upon the PubSubClient example. It holds the state of the ports (on/off) thus eliminating unneccessary `digitalWrite()` calls.

Node MCU needs to maintain the HTTP stack in the background. This is made sure via the `yield()` function, which is implicitely called each time before entering the `loop()` function. A problem occures, when the computation takes too long (for example because of `delay()` calls). In that case, the Node MCU reaches a timeout and performs a reset. To omit this, the code is 'polluted' with explicit `yield()` calls.

## Protocol

A simple protocol based on MQTT was implemented. There are two separate topics persent:

* /cord/update
* /cord/status

### `/cord/update`

This topic is used to send commands to the software. The possible commands consist of the following convention: `[port] [ON|OFF]` or the `STATUS` command. After each update the status of the device is published.

### `/cord/status`

Used by the device to publish its current status in a JSON format. 

## Test environemnt

The device was tested by using a [Mosquitto](https://mosquitto.org/) server running on an [OpenWRT](https://openwrt.org/) router on a separate IoT WiFi network.

## Todo

The communication protocol isn't a standardized format. It produces a JSON for the status message, but it doesn't accept one.

Should two separate messages be fetched in a short period of time, it sometimes hangs. Thus a delay of a few hundred milliseconds between command messages is a must.
