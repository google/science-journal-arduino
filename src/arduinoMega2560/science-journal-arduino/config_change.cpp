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
#include "config_change.h"
#include "sensor.pb.h"
#include "goosci_utility.h"
#include "debug_print.h"

#define SENSOR_CONFIG_SIZE 20

PinType pin_type = ANALOG;
int pin = A0;
static uint8_t sensorConfig[SENSOR_CONFIG_SIZE];

bool decode_pin(pb_istream_t *stream, const pb_field_t *field, void * *arg) {
  goosci_Pin sp = goosci_Pin_init_zero;
  if (!pb_decode(stream, goosci_Pin_fields, &sp))
    return false;

  if (sp.which_pin == goosci_Pin_analog_pin_tag) {
    pin_type = ANALOG;
    pin = sp.pin.analog_pin.pin;
  } else if (sp.which_pin == goosci_Pin_digital_pin_tag) {
    pin_type = DIGITAL;
    pin = sp.pin.digital_pin.pin;
    pinMode(pin, INPUT);
  } else {
    pin_type = VIRTUAL;
  }
  return true;
}

void handle(uint8_t* data, int8_t length) {
  int8_t size = data[0];
  bool last = data[1] == 1;
  memcpy(sensorConfig, data + 2, size);
  if (last) {
    pb_istream_t stream = pb_istream_from_buffer(sensorConfig,size);
    goosci_SensorDataRequest sdr = goosci_SensorDataRequest_init_zero;
    sdr.pin.funcs.decode = &decode_pin;
    if (!pb_decode(&stream, goosci_SensorDataRequest_fields, &sdr)) {
      DEBUG_PRINTLN(F("Failed parse of string."));
    }
  }

}
