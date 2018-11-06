/*
 *  Copyright 2018 Google Inc. All Rights Reserved.
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

#include "config_change.h"
#include "debug_print.h"
#include "goosci_utility.h"
#include "sensor.pb.h"

#include <Goosci.h>

extern PinType pin_type;
extern int pin;

GoosciClass::GoosciClass() :
  _previousMillis(0),
  _whistlepunkService("555a0001-0aaa-467a-9538-01f0652c74e8"),
  _valueCharacteristic( "555a0003-0aaa-467a-9538-01f0652c74e8", BLENotify, 20),
  _configCharacteristic("555a0010-0aaa-467a-9538-01f0652c74e8", BLEWrite, 20),
  _versionCharacteristic("555a0011-0aaa-467a-9538-01f0652c74e8", BLERead)
{
  _whistlepunkService.addCharacteristic(_valueCharacteristic);
  _whistlepunkService.addCharacteristic(_configCharacteristic);
  _whistlepunkService.addCharacteristic(_versionCharacteristic);
}

GoosciClass::~GoosciClass()
{
}

int GoosciClass::begin()
{
  if (!BLE.begin()) {
    return 0;
  }

  // set the UUID for the service this peripheral advertises:
  BLE.setAdvertisedServiceUuid(_whistlepunkService.uuid());

  // add service
  BLE.addService(_whistlepunkService);

  _valueCharacteristic.writeValue("                        ");
  _configCharacteristic.writeValue("                        ");
  _versionCharacteristic.writeValue(goosci_Version_Version_LATEST);

  // assign event handlers for connected, disconnected to peripheral
  BLE.setEventHandler(BLEConnected, bleConnectHandler);
  BLE.setEventHandler(BLEDisconnected, bleDisconnectHandler);

  _valueCharacteristic.setEventHandler(BLESubscribed, bleValueSubscribeHandler);
  _valueCharacteristic.setEventHandler(BLEUnsubscribed, bleValueUnubscribeHandler);
  _configCharacteristic.setEventHandler(BLEWritten, bleConfigWriteHandler);

  String address = BLE.address();

  address.toUpperCase();

  _name = "Sci";
  _name += address[address.length() - 5];
  _name += address[address.length() - 4];
  _name += address[address.length() - 2];
  _name += address[address.length() - 1];

  DEBUG_PRINT("Address is: ");
  DEBUG_PRINTLN(_name);

  BLE.setLocalName(_name.c_str());
  BLE.setDeviceName(_name.c_str());

  // advertise
  BLE.advertise();

  return 1;
}

void GoosciClass::loop()
{
  unsigned long currentMillis = millis();

  // poll peripheral
  BLE.poll();

  if (currentMillis - _previousMillis >= 100) {
    _previousMillis = currentMillis;

    if (_valueCharacteristic.subscribed()) {
      int sensorValue = 0;
      if (pin_type == ANALOG) {
        sensorValue = analogRead(pin);
      } else if (pin_type == DIGITAL) {
        sensorValue = digitalRead(pin);
      } else {
        sensorValue = 666;
      }
      Serial.println(sensorValue);
      send_data(_valueCharacteristic, millis(), sensorValue);
    }
  }
}

void GoosciClass::end()
{
}

void GoosciClass::bleConnectHandler(BLEDevice central)
{
  // central connected event handler
  DEBUG_PRINT("Connected event, central: ");
  DEBUG_PRINTLN(central.address());
}

void GoosciClass::bleDisconnectHandler(BLEDevice central)
{
  // central disconnected event handler
  DEBUG_PRINT("Disconnected event, central: ");
  DEBUG_PRINTLN(central.address());
}

void GoosciClass::bleValueSubscribeHandler(BLEDevice central, BLECharacteristic /*characteristic*/)
{
  // value characteristic event handler
  DEBUG_PRINT("Subscribe event, central: ");
  DEBUG_PRINTLN(central.address());
}

void GoosciClass::bleValueUnubscribeHandler(BLEDevice central, BLECharacteristic /*characteristic*/)
{
  // value characteristic unsubscribe event handler
  DEBUG_PRINT("Unsubscribe event, central: ");
  DEBUG_PRINTLN(central.address());
}

void GoosciClass::bleConfigWriteHandler(BLEDevice central, BLECharacteristic characteristic)
{
  // config characteristic event handler
  DEBUG_PRINTLN("config change event");
  handle( (uint8_t*) characteristic.value(), characteristic.valueLength());
  DEBUG_PRINT("Pin: ");
  DEBUG_PRINTLN(pin);
}

GoosciClass Goosci;
