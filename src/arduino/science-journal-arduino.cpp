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
 */
#include "BLEPeripheralGetAddress.h"
#include "goosci_utility.h"
#include "arduino_nrf_pins.h"
#include "debug_print.h"
#include "config_change.h"
#include "heartbeat.h"
#include "sensor.pb.h"

#define BLE_REQ   9
#define BLE_RDY   8
#define BLE_RST   2

bool have_address = false;
unsigned char *address = NULL;
BLEPeripheralGetAddress blePeripheral(BLE_REQ, BLE_RDY, BLE_RST); // create peripheral instance

BLEService whistlepunkService("555a0001-0aaa-467a-9538-01f0652c74e8"); // create service
// Must be 20 char long to accommodate full-size messages.
const char *value = "                     ";
const char *config = "                     ";
BLECharacteristic valueCharacteristic( "555a0003-0aaa-467a-9538-01f0652c74e8", BLENotify, value);
BLECharacteristic configCharacteristic("555a0010-0aaa-467a-9538-01f0652c74e8", BLEWrite, config);
const unsigned short version = goosci_Version_Version_LATEST;

BLEUnsignedShortCharacteristic versionCharacteristic("555a0011-0aaa-467a-9538-01f0652c74e8", BLERead);

char BleLongName[8];
bool serialConnected = false;
extern PinType pin_type;
extern int pin;

void blePeripheralConnectHandler(BLECentral& central) {
  // central connected event handler
  DEBUG_PRINT("Connected event, central: ");
  DEBUG_PRINTLN(central.address());
}

void blePeripheralDisconnectHandler(BLECentral& central) {
  // central disconnected event handler
  DEBUG_PRINT("Disconnected event, central: ");
  DEBUG_PRINTLN(central.address());
}

void bleNotificationSubscribeHandler(BLECentral& central, BLECharacteristic& characteristic) {
  // value characteristic event handler
  DEBUG_PRINT("Subscribe event, central: ");
  DEBUG_PRINTLN(central.address());
}

void bleNotificationUnsubscribeHandler(BLECentral& central, BLECharacteristic& characteristic) {
  // value characteristic unsubscribe event handler
  DEBUG_PRINT("Unsubscribe event, central: ");
  DEBUG_PRINTLN(central.address());
}

void bleConfigChangeHandler(BLECentral& central, BLECharacteristic& characteristic) {
  // config characteristic event handler
  DEBUG_PRINTLN("config change event");
  handle( (uint8_t*) characteristic.value(), characteristic.valueLength());
  DEBUG_PRINT("Pin: ");
  DEBUG_PRINTLN(pin);
}

void setup() {
  wait_for_serial();

  // set the UUID for the service this peripheral advertises:
  blePeripheral.setAdvertisedServiceUuid(whistlepunkService.uuid());

  // add service and characteristics
  blePeripheral.addAttribute(whistlepunkService);
  blePeripheral.addAttribute(valueCharacteristic);
  blePeripheral.addAttribute(configCharacteristic);
  blePeripheral.addAttribute(versionCharacteristic);
  versionCharacteristic.setValue(version);

  // assign event handlers for connected, disconnected to peripheral
  blePeripheral.setEventHandler(BLEConnected, blePeripheralConnectHandler);
  blePeripheral.setEventHandler(BLEDisconnected, blePeripheralDisconnectHandler);
  valueCharacteristic.setEventHandler(BLESubscribed, bleNotificationSubscribeHandler);
  valueCharacteristic.setEventHandler(BLEUnsubscribed, bleNotificationUnsubscribeHandler);
  configCharacteristic.setEventHandler(BLEWritten, bleConfigChangeHandler);


  // advertise the service
  blePeripheral.begin();
  while (true) {
    blePeripheral.poll();
    if (blePeripheral.haveAddress()) {
      have_address = true;
      address = blePeripheral.getAddress();
      sprintf(BleLongName, "Sci%02x%02x", address[0], address[1]);
      blePeripheral.setLocalName(BleLongName);
      blePeripheral.begin();
      break;
    }
  }
}


void loop() {
  if (Serial) {
    if (!serialConnected) {
      serialConnected = true;
      DEBUG_PRINT(F("LongName: "));
      DEBUG_PRINTLN(BleLongName);
    }
  } else {
    if (serialConnected)
      serialConnected = false;
  }

  // poll peripheral
  blePeripheral.poll();

  if (valueCharacteristic.subscribed()) {
    int sensorValue = 0;
    if (pin_type == ANALOG) {
      sensorValue = analogRead(pin);
    } else if (pin_type == DIGITAL) {
      sensorValue = digitalRead(pin);
    } else {
      sensorValue = 666;
    }
    send_data(valueCharacteristic, millis(), sensorValue);
  }
#ifdef GOOSCI_DEVELOPER_MODE
  heartbeat();
#endif
}
