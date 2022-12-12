# BytebeamArduino
This library provies a client for connecting ESP devices to [Bytebeam](https://bytebeam.io/) IoT platform

## Features
- Efficiently send data to cloud.
- Receive commands from the cloud, execute them and update progress of execution.
- Download Firmware images from cloud in case of OTA updates being triggered from cloud.

## What's included in the Library :-

- **src** :-  This section contains source code for various functions that can be used by applications for interacting with Bytebeam platform. 
- **examples** :- This folder conatins few example sketch which demonstrates establishing secure connection with Bytebeam platform. Also, it demonstrates periodic data pushing and receiving actions.

## Dependencies :-
- [Arduinojson](https://github.com/bblanchon/ArduinoJson)
- [PubSubClient](https://github.com/knolleary/pubsubclient) 
- Currently we support Arduino ESP32 Core SDK Version [2.0.3](https://github.com/espressif/arduino-esp32/tree/2.0.3) 
- ESP32 Board

## Library Setup and Integration :-
This Library can be integrated with new as well as existing Arduino ESP32 sketch. Follow the [instruction guide](https://github.com/bytebeamio/BytebeamArduino/blob/main/README.md) for setting up and integrating library with your Arduino ESP32 sketch. 
