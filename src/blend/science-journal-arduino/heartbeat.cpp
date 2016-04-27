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

int heartbeat_interval = 1000;  // blink every second
long heartbeat_timer = 0;
bool heartbeat_led_status = false;
int heartbeat_led_pin = 13;


// just call at the end of loop() and this will blink the LED every
// heart_beat_interval milli_seconds, useful for knowing if your sketch
// is still running or has hung some place
bool heartbeat(void) {
  if (heartbeat_timer == 0) {
    heartbeat_timer = millis();
  } else {
    if (millis() - heartbeat_timer > heartbeat_interval) {
      heartbeat_timer = millis();
      heartbeat_led_status = !heartbeat_led_status;
      digitalWrite(heartbeat_led_pin, heartbeat_led_status);
    }
  }
  return heartbeat_led_status;
}
