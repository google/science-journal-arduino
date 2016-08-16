[Science Journal][play-store] allows you to gather data from the world around you. It uses sensors to measure your environment, like light and sound, so you can graph your data, record your experiments, and organize your questions and ideas. It's the lab notebook you always have with you.

This sketch is used to make an Arduino that has Bluetooth Low Energy
(BLE) capabilities (such as the RedBear BLEND board and the Arduino 101)
send data from its pins to Science Journal.  The arduinoUno firmware
is provided for reference, but does not work because of limited RAM on
the Uno.

## Building the firmware

This project uses [Platform IO](http://platformio.org/get-started).
After installing PlatformIO, upload to a specific board using:
```platformio -f -c vim run --target upload -e mega2560```

## Contributing

Please read our [guidelines for contributors][contributing].

## License

Open Science Journal is licensed under the [Apache 2 license][license].

## More

Science Journal is brought to you by [Making & Science][making-science], an initiative by Google. Open Science Journal is not an official Google product.
