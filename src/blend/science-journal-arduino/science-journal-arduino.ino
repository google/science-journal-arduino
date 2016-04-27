/*
 *  Copyright 2016 Google Inc. All Rights Reserved.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *  Generic analog sensor for Whistlepunk.  Reads values from A0 and
 *  sends them to Whistlepunk via BLE.
 */

#include "GoosciBleGatt.h"
#include "goosci_utility.h"
#include "arduino_nrf_pins.h"
#include "heartbeat.h"
#include "sensor.pb.h"
#include "debug_print.h"
#include "config_change.h"

#include <pb_decode.h>

GoosciBleGatt goosciBle = GoosciBleGatt(REQ, RDY, RESET);

unsigned long timestamp;
bool serialConnected = false;

// Sketches
void setup(void) {
  wait_for_serial();
  // Set long name.
  goosciBle.setLongName("Generic Analog");   // 20 chars max!

  // Set device description.
  goosciBle.setDeviceDescription("Generic Analog");  // 20 chars max!

  goosciBle.begin();
}

extern PinType pin_type;
extern int pin;

void loop() {
  // always respond to something over serial it makes it easier to
  // identify our firmware from a pc
  if (Serial) {
    if (!serialConnected) {
        goosciBle.print_address();
        serialConnected = true;
      }
  } else {
    if (serialConnected)
      serialConnected = false;
  }

  goosciBle.pollACI();
  if (goosciBle.isReadyToSend()) {
    int sensorValue = 0;
    if (pin_type == ANALOG) {
      sensorValue = analogRead(pin);
    } else if (pin_type == DIGITAL) {
      sensorValue = digitalRead(pin);
    } else {
      sensorValue = 666;
    }
    send_data(goosciBle, millis(), sensorValue);
  }
#ifdef GOOSCI_DEVELOPER_MODE
  heartbeat();
#endif
}
