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
