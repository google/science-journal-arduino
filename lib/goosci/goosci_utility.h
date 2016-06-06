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
#ifndef _GOOSCI_UTILITY_H_
#define _GOOSCI_UTILITY_H_

#if defined(__ARDUINO_ARC__)
  #include <CurieBLE.h>
  void send_data(BLECharacteristic& characteristic, unsigned long timestamp, int value);
#endif

#if defined(__AVR_ATmega32U4__)
  #include "GoosciBleGatt.h"
  void send_data(GoosciBleGatt &goosciBle, unsigned long timestamp, int value);
#endif

// The serial port on leonardo will hang (UART buffer full) if the
// serial port is not physically opened on the USB host.  This
// function will block until the serial port is open.
void wait_for_serial(void);

#endif
