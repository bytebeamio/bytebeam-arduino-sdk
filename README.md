# BytebeamArduino
This library provies a client for connecting ESP devices to [Bytebeam](https://bytebeam.io/) IoT platform

## Features
- Efficiently send data to cloud.
- Receive commands from the cloud, execute them and update progress of execution.
- Download Firmware images from cloud in case of OTA updates being triggered from cloud.

## What's included in the Library :-

- **src** :-  This section contains source code for various functions that can be used by applications for interacting with Bytebeam platform. 
- **examples** :- This folder conatins few sketches which demonstrates establishing secure connection with Bytebeam platform. Also, it demonstrates periodic data pushing and receiving actions.

## Dependencies :-
- [Arduinojson](https://github.com/bblanchon/ArduinoJson)
- [PubSubClient](https://github.com/knolleary/pubsubclient) 
- [Arduino ESP32 Core SDK](https://github.com/espressif/arduino-esp32) 
- ESP32 Board

## Library Setup and Integration :-
This Library can be integrated with new as well as existing Arduino ESP32 sketches. Follow the [instruction guide](https://bytebeam.io/docs/esp-idf) for setting up and integrating library with your Arduino ESP32 sketches. 
