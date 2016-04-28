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
#ifndef _ARDUINO_NRF_PINS_H_
#define _ARDUINO_NRF_PINS_H_

// Digital pins on the Arduino.
#if defined(__AVR_ATmega32U4__)
#define REQ 9
#define RDY 8
#define RESET 255    // 255 = UNUSED in hal_aci_tl.h
#else
#define REQ 10
#define RDY 2
#define RESET 9
#endif

#endif  // ARDUINO_NRF_PINS_H_
