# RealFakeGarageDoors
[![Build Status](https://travis-ci.org/plasticrake/RealFakeGarageDoors.svg?branch=master)](https://travis-ci.org/plasticrake/RealFakeGarageDoors)

Garage Door Opener for ESP8266 / Arduino.

This is the hardware portion of my Real Fake Garage Doors project.
The client is the [homebridge-real-fake-garage-doors](https://github.com/plasticrake/homebridge-real-fake-garage-doors) plugin for [Homebridge](https://github.com/nfarina/homebridge).

I opened a [Linear Multi-Code 412001](https://www.amazon.com/dp/B000F5KEP6/) garage door remote and attached an ESP8266 to trigger it to open the garage door. Since the ESP8266 doesn't have an easy way to encrypt traffic as a server I'm using SHA256-HMAC to send authenticated commands to the ESP8266.

## Configuration

`HASH_KEY`: SHA256-HMAC key that should match the key from homebridge-real-fake-garage-doors.

`MAX_OPEN_COUNT_PER_MINUTE` / `MAX_OPEN_COUNT_PER_HOUR` / `MAX_OPEN_COUNT_PER_DAY`: If any of these thresholds are exceeded then the device will no longer fulfill any open commands until the device is reset. This is a safeguard against any errant clients continuously sending open commands.

## Hardware

![Schematic](https://github.com/plasticrake/RealFakeGarageDoors/raw/master/hardware/schematic.png "Schematic")
![Breadboard](https://github.com/plasticrake/RealFakeGarageDoors/raw/master/hardware/breadboard.jpg "Breadboard")

### Parts
* ESP8266
* [Linear Multi-Code 412001 Remote](https://www.amazon.com/dp/B000F5KEP6/)
* NPN Transistor [PN2222](https://www.adafruit.com/product/756) x2
* P-Channel MOSFET [BS250](http://www.jameco.com/z/BS250-Major-Brands-Transistor-BS250-TO-92-Transistor-P-Channel-MOSFET-45-Volt_256057.html) x2
* 10k Resistor x4
* 680 Resistor x2

## Thanks
SHA256-HMAC implementation from https://github.com/spaniakos/Cryptosuite which is a fork of https://github.com/Cathedrow/Cryptosuite
