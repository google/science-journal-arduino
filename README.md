The Arduino sketch used by the Science Journal Android application.
This sketch is used to make an Arduino that has Bluetooth Low Energy
(BLE) capabilities (such as the RedBear BLEND board) send data from
its pins to Science Journal.

To build and install this sketch, 
  * [install the Arduino IDE](https://www.arduino.cc/en/Main/Software) on your desktop/laptop computer. The [Getting Started](https://www.arduino.cc/en/Guide/HomePage) page explains how to install the IDE.
  * You also need to install the nanopb support library. 
    - In your browser, open https://github.com/nanopb/nanopb.  Click the "Download ZIP" button:
      ![Download ZIP button highlight](download_nanopb_zip.png "Download ZIP button highlight")
    - Inside the Arduino IDE go to Sketch -> Include Library -> Add .ZIP Library:
      ![Include library add zip highlight](include_library_add_zip.png "Include library add zip highlight")
    - Navigate to the Downloads directory for your computer's browser
      (this location differs by Operating System) and select the file
      "nanopb-master.zip".
      ![Select nanopb-master.zip highlight](select_nanopb.png "Select nanopb-master.zip highlight")
  * If you are using the BLEND board, you need to add the Red Bear site to your Arduino IDE's preferences:
    - Navigate to the File->Preferences menu item
      ![File Preferences highlight](file_preferences.png "File preferences highlight")
    - install the BLEND Device's definition to the Arduino IDE Preferences
      ![Add RedBear Arduino package highlight](add_redbear_arduino_package.png "Add RedBear Arduino package highlight")
    - Add the BLEND Device to the Arduino IDE Board Manager
      ![File Boards Manager Highlight](file_boards_manager.png "File Boards Manager Highlight")
    - Enter RedBear as the search term and click on the "RedBear AVR" board
      ![Boards Manager RedBear Search Highlight](boards_manager_redbear_search.png "Boards Manager RedBear Search Highlight")
    - Click on the "Install" button
      ![Boards Manager RedBear Install Highlight](boards_manager_redbear_install.png "Boards Manager RedBear Install Highlight")
    - Tell the Arduino IDE you want to program a BLEND board
      ![Board BLEND Highlight](board_blend.png "Board BLEND Highlight")
    - Tell the Arduino IDE what port the BLEND board is on.  It will report itself as an "Arduino Leonardo"
      ![Port Highlight](Port.png "Port Highlight")
    - Click the checkmark to upload the firm
      ![Upload Firmware Highlight](upload_firmware.png "Upload Firmware Highlight")
    - When it succeeds, it will say "Done uploading"
      ![Done uploading Highlight](done_uploading.png "Done uploading Highlight")

You must install the following libraries
* https://github.com/RedBearLab/ble-sdk-arduino
- Follow the guide here https://github.com/RedBearLab/Blend/blob/master/Docs/LibraryManager.pdf
