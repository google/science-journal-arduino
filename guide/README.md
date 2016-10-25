# A walkthru of the basic workflow of PlatformIO building and uploading the  Science-Journal-Arduino firmware.

* Download the [Arduino Firmware](http://github.com/google/science-journal-arduino) via the green "**Clone or download**" button on the right hand side.
![](images/Github.png "Repository Index")

* Once downloaded, unzip using your favorite tool if needed.

* Download and install [PlatformIO IDE](http://www.platformio.org) for your OS.  After installation you should be greeted with the Home screen.
![PlatformIO Home](images/PlatformIO.png)

* From this screen select "Open Project" and select the path to your checkout.  After a few seconds you should be greeted with a screen like below.
![](images/FolderOpened.png "PIO Home")


* Click the checkmark on the left hand side to build the firmwares.
![](images/Build.png "Build")

	PlatformIO should automatically install all the required compiler toolchains and libraries.
![](images/Building.png "Building")

	If everything has worked according to plan you should see something like this for a little bit and then be returned to the project screen.
![](images/Build_Success.png "Build Success")


* Now launch the built-in **Terminal**
![](images/Terminal.png "Terminal")

* From the terminal upload the firmware to your device.  You need to specify which board you are using.  The current supported board environment identifiers are [arduino101](https://www.arduino.cc/en/Main/ArduinoBoard101), [blend](http://redbearlab.com/blend/), or [mega2560](https://www.arduino.cc/en/Main/ArduinoBoardMega2560) .  If PlatformIO has trouble autodetecting your serial port, please refer to [documentation](http://docs.platformio.org/en/stable/).

		pio run -e <ENV> --target upload

![](images/PIO_Upload_Start.png "Upload Start")

* If everything went according to plan, you should be greeted with a screen similar to below.  **[If you're on Windows and receiving an error "Cannot open DFU device 8087:0aba", click here](101.md)**

![](images/PIO_Upload_Success.png "Upload Success")

* Just to verify that the firmware took, let's connect to it via Serial Monitor.

	1. Select the Serial Monitor
![](images/PIO_Serial.png "Serial Monitor")

	1. Select the port and baud rate
![](images/PIO_Serial_Baud.png "Select Baud")

	1. Connect and you should see something similar to the screen below.  *DeviceName* is the bluetooth name that your board will advertise itself as.
![](images/PIO_Serial_Success.png "Serial Success")
