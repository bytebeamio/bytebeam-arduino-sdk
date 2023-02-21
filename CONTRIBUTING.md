# Contributing to BytebeamArduino

If you have loved using BytebeamArduino and want to give back, we would love to have you open GitHub issues and PRs for features, bugs and documentation improvements.

Before you start, please make yourself familiar with the architecture of BytebeamArduino and read the [design docs][design] before making your first contribution to increase it's chances of being adopted. Please follow the [Code of Conduct][coc] when communicating with other members of the community and keep discussions civil, we are excited to have you make your first of many contributiions to this repository, welcome!

## Steps to Contribute

### Getting the library

Go to <https://github.com/bytebeamio/BytebeamArduino> and fork the library repository.

```bash
# Open the terminal in the arduino libraries directory
$ cd path_to_arduino_libraries

# Remove the BytebeamArduino library if any
$ rmdir /s /q BytebeamArduino

# Clone your fork
$ git clone git@github.com:<YOU>/BytebeamArduino.git

# Step into library directory
$ cd BytebeamArduino

# Create a branch for your changes (say my_topical_branch)
$ git checkout -b my_topical_branch

# Open the library in your favorite text editor (say vs code)
$ code .
```

### Library Setup

Before stepping into library setup, launch the Arduino IDE. If you have not installed the Arduino IDE yet then the best place to get started will be the [Arduino Getting Started Guide][arduino-get-started]

Look at the example sketch (say ToggleLED) for reference. See File > Examples > BytebeamArduino > ToggleLED within the Arduino IDE. You can use Arduino IDE GUI to verify and upload the example sketch

### Making Changes

Please make sure your changes conform to Arduino Style Guide.

- For core library refer [Arduino Library Style Guide][arduino-library-style]
- For tutorials and example sketches refer [Arduino Writing Style Guide][arduino-writing-style]

### Testing & CI

Tests Suite is not yet added to the repo, will add it once we are done with the development cycle of the first release. By the way you can contribute this too :)

## Add yourself to Contributors

Thank you for contributing to BytebeamArduino, Please feel free to add yourself to [Contributors][contributors]

## License

BytebeamArduino is licensed under the permissive [Apache License Version 2.0][license] and we accept contributions under the implied notion that they are made in complete renunciation of the contributors any rights or claims to the same after the code has been merged into the codebase.

[license]: LICENSE
[design]: docs/design.md
[coc]: CODE_OF_CONDUCT.md
[arduino-get-started]: https://docs.arduino.cc/learn/starting-guide/getting-started-arduino
[arduino-library-style]: https://docs.arduino.cc/learn/contributions/arduino-library-style-guide
[arduino-writing-style]: https://docs.arduino.cc/learn/contributions/arduino-writing-style-guide
[contributors]: AUTHORS.md#contributors