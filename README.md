The Arduino sketch used by the Science Journal Android application.
This sketch is used to make an Arduino that has Bluetooth Low Energy
(BLE) capabilities (such as the RedBear BLEND board and the Arduino 101)
send data from its pins to Science Journal.  The arduinoUno firmware
is provided for reference, but does not work because of limited RAM on
the Uno.

This project uses [Platform IO](http://platformio.org/get-started).
After installing PlatformIO, upload to a specific board using:
```platformio -f -c vim run --target upload -e mega2560```

