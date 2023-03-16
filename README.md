# BytebeamArduino

<div>
    <img alt="Bytebeam Arduino Logo" src="docs/logo.png" />
</div>

This library provides a simple client for connecting ESP devices to [Bytebeam](https://bytebeam.io/) IoT platform.

## What's included in the Library :-

- **src** :-  This section contains source code for various functions that can be used by applications for interacting with Bytebeam platform. 
- **examples** :- This folder conatins few example sketch which demonstrates establishing secure connection with Bytebeam platform. Also, it demonstrates periodic data pushing and receiving actions.

## Dependencies :-
- [PubSubClient](https://github.com/knolleary/pubsubclient) 
- [Arduinojson](https://github.com/bblanchon/ArduinoJson)
- [NTPClient](https://github.com/arduino-libraries/NTPClient)
- Core SDK ( [ESP32](https://github.com/espressif/arduino-esp32) or [ESP8266](https://github.com/esp8266/Arduino) )
- Dev Board ( ESP32 or ESP8266 )

We recommend to install the latest versions of the libraries and Core SDK.

## Getting Started :-
This Library can be integrated with new as well as existing Arduino ESP32 or Arduino ESP8266 sketches. Follow the [instruction guide](https://bytebeam.io/docs/arduino) for setting up and integrating library with your Arduino sketch. 

## Features
- Efficiently send data to cloud.
- Receive commands from the cloud, execute them and update progress of execution.
- Download Firmware images from cloud in case of OTA updates being triggered from cloud.

## Community

- Follow us on [Twitter](https://twitter.com/bytebeamhq)
- Connect with us on [LinkedIn](https://www.linkedin.com/company/bytebeam/)
- Read our official [Blog](https://bytebeam.io/blog/)

## Contributing
Please follow the [code of conduct](CODE_OF_CONDUCT.md) while opening issues to report bugs or before you contribute fixes, also do read our [contributor guide](CONTRIBUTING.md) to get a better idea of what we'd appreciate and what we won't.

## License

This project is released under The Apache License, Version 2.0 ([LICENSE](./LICENSE) or http://www.apache.org/licenses/LICENSE-2.0)
