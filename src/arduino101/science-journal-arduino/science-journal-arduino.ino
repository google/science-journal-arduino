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
#include <CurieBLE.h>
#include "goosci_utility.h"
#include "arduino_nrf_pins.h"
#include "debug_print.h"
#include "config_change.h"
#include "heartbeat.h"
#include "internal/ble_client.h"
#include "services/ble/ble_service_gap_api.h"
   
BLEPeripheral blePeripheral; // create peripheral instance
BLEService whistlepunkService("555a0001-0aaa-467a-9538-01f0652c74e8"); // create service
// Must be 20 char long to accomodate full-size messages.
const char *value = "                     ";
BLECharacteristic valueCharacteristic("555a0003-0aaa-467a-9538-01f0652c74e8", BLENotify, value);

String BleLongName = "Sci";
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

void setup() {
  wait_for_serial();

  // set the local name peripheral advertises
  blePeripheral.setLocalName("Initial");
  // set the UUID for the service this peripheral advertises:
  blePeripheral.setAdvertisedServiceUuid(whistlepunkService.uuid());

  // add service and characteristics
  blePeripheral.addAttribute(whistlepunkService);
  blePeripheral.addAttribute(valueCharacteristic);

  // assign event handlers for connected, disconnected to peripheral
  blePeripheral.setEventHandler(BLEConnected, blePeripheralConnectHandler);
  blePeripheral.setEventHandler(BLEDisconnected, blePeripheralDisconnectHandler);
  valueCharacteristic.setEventHandler(BLESubscribed, bleNotificationSubscribeHandler);
  valueCharacteristic.setEventHandler(BLEUnsubscribed, bleNotificationUnsubscribeHandler);

  ble_addr_t _local_bda;
  char       _device_name[BLE_MAX_DEVICE_NAME+1];
  ble_client_get_factory_config(&_local_bda, _device_name);
  BleLongName += String(_local_bda.addr[4], HEX);
  BleLongName += String(_local_bda.addr[5], HEX);
  DEBUG_PRINT("Address is: ");
  DEBUG_PRINTLN(BleLongName);
  blePeripheral.setLocalName(BleLongName.c_str());

  // advertise the service
  blePeripheral.begin();
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
