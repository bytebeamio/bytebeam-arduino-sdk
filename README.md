# BytebeamArduino

<div>
    <img alt="Bytebeam Arduino Logo" src="docs/logo.png" />
</div>

---

[![arduino-library-badge](https://www.ardu-badge.com/badge/BytebeamArduino.svg?)](https://www.ardu-badge.com/BytebeamArduino)
[![PlatformIO Registry](https://badges.registry.platformio.org/packages/bytebeamio/library/BytebeamArduino.svg)](https://registry.platformio.org/libraries/bytebeamio/BytebeamArduino)

This Library provides a simple client for connecting ESP devices to [Bytebeam](https://bytebeam.io/) IoT platform.

## Features :-
- Efficiently send data to cloud.
- Receive commands from the cloud, execute them and update progress of execution.
- Download Firmware images from cloud in case of OTA updates being triggered from cloud.


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
This Library can be integrated with new as well as existing Arduino ESP32 or Arduino ESP8266 sketches. Follow the [instruction guide](https://bytebeam.io/docs/arduino) for setting up and integrating Library with your Arduino sketch. 

## Community :-

- Follow us on [Twitter](https://twitter.com/bytebeamhq)
- Connect with us on [LinkedIn](https://www.linkedin.com/company/bytebeam/)
- Read our official [Blog](https://bytebeam.io/blog/)

## Contributing :-
Contributions are welcome! Not only you’ll encourage the development of the Library, but you’ll also learn how to best use the Library and probably some C++ too.

See [the contributing guide](CONTRIBUTING.md) for detailed instructions on how to get started with the Library. Please follow the [code of conduct](CODE_OF_CONDUCT.md) while contributing.

## License :-

This project is released under The Apache License, Version 2.0 (See [LICENSE](./LICENSE) for details)
