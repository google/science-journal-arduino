#ifndef _GOOSCI_UTILITY_H_
#define _GOOSCI_UTILITY_H_
#include <CurieBLE.h>

// The serial port on leonardo will hang (UART buffer full) if the
// serial port is not physically opened on the USB host.  This
// function will block until the serial port is open.
void wait_for_serial(void);

void send_data(BLECharacteristic& characteristic, unsigned long timestamp, int value);

#ifndef _DEBUG_PRINT_H_

#define DEBUG_PRINT(x) \
  {                    \
    if (Serial) {      \
      Serial.print(x); \
    }                  \
  }
#define DEBUG_PRINTLN(x) \
  {                      \
    if (Serial) {        \
      Serial.println(x); \
    }                    \
  }

#define DEBUG_PRINT2(x, y) \
  {                        \
    if (Serial) {          \
      Serial.print(x, y);  \
    }                      \
  }
#define DEBUG_PRINTLN2(x, y) \
  {                          \
    if (Serial) {            \
      Serial.println(x, y);  \
    }                        \
  }

#endif

#endif
