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

#include <stdio.h>
#include <Arduino.h>
#include "pb_encode.h"
#include "debug_print.h"
#include "goosci_utility.h"
#include "sensor.pb.h"

void wait_for_serial(void) {
#if defined(__AVR_ATmega32U4__) || defined(__ARDUINO_ARC__)
  // Wait for 5 seconds to see if the USB Serial connects
  // We don't need to call Serial.begin because the 32u4 auto detects
  // a serial connection and baud rate for us.
  delay(5000);
#else
  Serial.begin(115200);
#endif
#if defined(__ARDUINO_ARC__) 
  Serial.begin(115200);
#endif
}

goosci_SensorData sd  = goosci_SensorData_init_zero;
const int BUFFER_LEN=256;
uint8_t buffer[BUFFER_LEN];
pb_ostream_t stream;

#define BTLE_BUFFER_SIZE 20

int packets = 0;
uint8_t packet[BTLE_BUFFER_SIZE];

void send_data(BLECharacteristic& characteristic, unsigned long timestamp_key, int value) {
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
    /*
    DEBUG_PRINT(F("send_data timestamp: "));
    DEBUG_PRINTLN(sd.timestamp_key);
    DEBUG_PRINT(F("send_data value: "));
    DEBUG_PRINTLN(value);
    DEBUG_PRINT(F("size: "));
    DEBUG_PRINT(stream.bytes_written);
    DEBUG_PRINTLN(F(" bytes."));
    String s;
    for (unsigned int i = 0; i < stream.bytes_written; ++i) {
      s += String(buffer[i], HEX);
      if ((i-1) % 2 == 0) s += " ";
    }
    DEBUG_PRINTLN(s.c_str());
    */
    uint8_t size = stream.bytes_written;
    const uint8_t max_packet_size = BTLE_BUFFER_SIZE - 2;
    /* Force size/max_packet_size to round up */
    uint8_t num_packets = (size + max_packet_size - 1) / max_packet_size;
    

    for (uint8_t ii = 0; ii < num_packets; ii++) {
      bool is_last_packet = ((num_packets - 1) == ii);
      /* There are 3 possibilities for current_packet_size
         1) It is the last packet and the remaining data is smaller than our allocated buffer (size % max_packet_size)
         2) It is the last packet and the remaining data is equal to our allocated buffer (max_packet_size)
         3) It is not the last packet (max_packet_size)
      */
      uint8_t current_packet_size = (is_last_packet ? ((size % max_packet_size == 0) ? max_packet_size : (size % max_packet_size)) : max_packet_size);
      packet[0] = current_packet_size;
      packet[1] = is_last_packet;
      memcpy((void*)(packet + 2), buffer + ii * max_packet_size, current_packet_size);
      
      /* If send fails then we give up */
      if (!characteristic.setValue(packet, current_packet_size+2)) {
        DEBUG_PRINTLN("Send of packet failed.");
        break;
      }
    }        
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
