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
#include <Arduino.h>
#include <pb_encode.h>
#include "goosci_utility.h"
#include "sensor.pb.h"
#include "debug_print.h"

void wait_for_serial(void) {
#if defined(__AVR_ATmega32U4__)
  // Wait for 5 seconds to see if the USB Serial connects
  // We don't need to call Serial.begin because the 32u4 auto detects
  // a serial connection and baud rate for us.
  delay(5000);
#else
  Serial.begin(115200);
#endif
}

goosci_SensorData sd  = goosci_SensorData_init_zero;
const int BUFFER_LEN=256;
uint8_t buffer[BUFFER_LEN];
pb_ostream_t stream;


int packets = 0;

void send_data(GoosciBleGatt &goosciBle, unsigned long timestamp_key, int value) {
  stream = pb_ostream_from_buffer(buffer, BUFFER_LEN);

  sd.timestamp_key = timestamp_key; // timestamp
  sd.which_result = (pb_size_t)goosci_SensorData_data_tag;
  sd.result.data.pin = goosci_Pin();
  sd.result.data.pin.which_pin = goosci_Pin_analog_pin_tag;
  sd.result.data.pin.pin.analog_pin.pin = 0;
  sd.result.data.which_value = goosci_Data_analog_value_tag;
  sd.result.data.value.analog_value.value = value;

  if (!pb_encode(&stream, goosci_SensorData_fields, &sd)) {
    DEBUG_PRINT(F("Encoding failed: "));
    DEBUG_PRINTLN(PB_GET_ERROR(&stream));
  } else {
    // DEBUG_PRINT(F("send_data timestamp: "));
    // DEBUG_PRINTLN(sd.timestamp_key);
    // DEBUG_PRINT(F("send_data value: "));
    // DEBUG_PRINTLN(value);
    // DEBUG_PRINT(F("size: "));
    // DEBUG_PRINT(stream.bytes_written);
    // DEBUG_PRINTLN(F(" bytes."));

    // char b[4];
    // for (unsigned int i = 0; i < stream.bytes_written; ++i) {
    //   snprintf(b, 3, "%02x", buffer[i]);
    //   DEBUG_PRINT(b);
    //   DEBUG_PRINT(F(" "));
    // }
    // DEBUG_PRINTLN();
    goosciBle.sendData((const char *)buffer, stream.bytes_written);
  }
}

bool encode_pin(pb_ostream_t *stream, const pb_field_t *field, void * const *arg) {
  goosci_Pin sp = goosci_Pin_init_zero;
  sp.pin.analog_pin.pin = 0;
  if (!pb_encode_tag_for_field(stream, field))
    return false;
  if (!pb_encode_submessage(stream, goosci_Pin_fields, &sp))
    return false;
  return true;
}
