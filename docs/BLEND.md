* To install the firmware on the RedBear BLEND board,
    - Navigate to the Sketch->Include Library->Manage Libraries menu item and click it
      ![Sketch Add Library highlight](docs/sketch_add_library.png "Sketch Add Library highlight")
    - Enter "BLE SDK" in the "Filter your search" box and select the "BLE SDK for Arduino by RedBearLab" then click "Install"
      ![BLEND add BLE SDK](docs/blend_add_ble_sdk.png "BLEND add BLE SDK")
    - Click "close" to return to the Arduino IDE
      
* Add the Red Bear site to your Arduino IDE's preferences and install the BLEND board drivers:
    - Navigate to the File->Preferences menu item
      ![File Preferences highlight](docs/file_preferences.png "File preferences highlight")
    - install the BLEND Device's definition to the Arduino IDE Preferences.  The [URL to add is[(https://redbearlab.github.io/arduino/package_redbearlab_index.json)
      ![Add RedBear Arduino package highlight](docs/add_redbear_arduino_package.png "Add RedBear Arduino package highlight")
    - Add the BLEND Device to the Arduino IDE Board Manager
      ![File Boards Manager Highlight](docs/file_boards_manager.png "File Boards Manager Highlight")
    - Enter RedBear as the search term and click on the "RedBear AVR" board
      ![Boards Manager RedBear Search Highlight](docs/boards_manager_redbear_search.png "Boards Manager RedBear Search Highlight")
    - Click on the "Install" button
      ![Boards Manager RedBear Install Highlight](docs/boards_manager_redbear_install.png "Boards Manager RedBear Install Highlight")

* Program the BLEND board
    - Tell the Arduino IDE you want to program a BLEND board
      ![Board BLEND Highlight](docs/board_blend.png "Board BLEND Highlight")
    - Tell the Arduino IDE what port the BLEND board is on.  It will report itself as an "Arduino Leonardo"
      ![Port Highlight](docs/port_blend.png "Port Highlight")
    - Click the checkmark to upload the firm
      ![Upload Firmware Highlight](docs/upload_firmware.png "Upload Firmware Highlight")
    - When it succeeds, it will say "Done uploading"
      ![Done uploading Highlight](docs/done_uploading.png "Done uploading Highlight")

