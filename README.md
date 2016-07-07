# RealFakeGarageDoors

Garage Door Opener for ESP8266.

This is the hardware portion of my Real Fake Garage Doors project.

I took apart a [Linear Multi-Code 412001 Remote](https://www.amazon.com/dp/B000F5KEP6/)  and attached an ESP8266 that triggers it to open the garage door. Since the ESP8266 doesn't have an easy way to encrypt traffic as a server I'm using SHA256-HMAC to send authenticated commands to the ESP8266.

The client is the [homebridge-real-fake-garage-doors](https://github.com/plasticrake/homebridge-real-fake-garage-doors) plugin for [Homebridge](https://github.com/nfarina/homebridge).


## Configuration

`HASH_KEY`: SHA256-HMAC key that should match the key from homebridge-real-fake-garage-doors.

`MAX_OPEN_COUNT_PER_MINUTE` / `MAX_OPEN_COUNT_PER_HOUR` / `MAX_OPEN_COUNT_PER_DAY`: If any of these thresholds are exceeded then the device will no longer fulfill any open commands until the device is reset. This is a safeguard against any errant clients continuously sending open commands.

## Schematics

![Schematics](https://github.com/plasticrake/RealFakeGarageDoors/raw/master/hardware/schematics.png "Schematics")

### Parts
* ESP8266
* [Linear Multi-Code 412001 Remote](https://www.amazon.com/dp/B000F5KEP6/)
* NPN Transistor [PN2222](https://www.adafruit.com/product/756) x2
* 10k Resistor x2

## Thanks
SHA256-HMAC implementation from https://github.com/spaniakos/Cryptosuite which is a fork of https://github.com/Cathedrow/Cryptosuite
