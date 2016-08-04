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
#ifndef _BLE_PERIPHERAL_GET_ADDRESS_H_
#define _BLE_PERIPHERAL_GET_ADDRESS_H_
#include "BLEPeripheral.h"
#include "BLEDevice.h"

class BLEPeripheralGetAddress : public BLEPeripheral {
 public:
  BLEPeripheralGetAddress(unsigned char req = BLE_DEFAULT_REQ, unsigned char rdy = BLE_DEFAULT_RDY, unsigned char rst = BLE_DEFAULT_RST);
  virtual void BLEDeviceAddressReceived(BLEDevice& device, const unsigned char* address);
  unsigned char* getAddress();
  bool haveAddress();
 private:
  bool _have_address;
  unsigned char _device_address[6];
};

#endif //  _BLE_PERIPHERAL_GET_ADDRESS_H_
