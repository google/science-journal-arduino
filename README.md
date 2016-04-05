The Arduino sketch used by the Science Journal Android application. It provides
a basic analog sensor -> BLE gateway interface.

If you are using the default Blend AVR board you should follow the guide here.
 * https://github.com/RedBearLab/Blend/blob/master/Docs/BoardsManager.pdf

You must install the following libraries
 * https://github.com/RedBearLab/ble-sdk-arduino
   - Follow the guide here https://github.com/RedBearLab/Blend/blob/master/Docs/LibraryManager.pdf
 * https://github.com/nanopb/nanopb
   - You need to download the zip file from Github
   - Inside the Arduino IDE go to Sketch -> Include Library -> Add .ZIP Library and point it to the ZIP you downloaded
