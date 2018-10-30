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
#include <string.h>
#include <SPI.h>
#include <lib_aci.h>
#include <aci_setup.h>

#include "GoosciBleGatt.h"
#include "goosci_utility.h"
#include "services.h"
#include "sensor.pb.h"
#include "debug_print.h"
#include <pb_decode.h>

extern void handle(uint8_t* data, int8_t length);

// Get the service pipe data created in nRFGo Studio
#ifdef SERVICES_PIPE_TYPE_MAPPING_CONTENT
static services_pipe_type_mapping_t
services_pipe_type_mapping[NUMBER_OF_PIPES] =
    SERVICES_PIPE_TYPE_MAPPING_CONTENT;
#else
#define NUMBER_OF_PIPES 0
static services_pipe_type_mapping_t *services_pipe_type_mapping = NULL;
#endif

#define BTLE_DEVICE_NAME_SIZE 8
#define BTLE_BUFFER_SIZE 20

// Store the setup for the nRF8001 in the flash of the AVR to save on RAM
static const hal_aci_data_t setup_msgs[NB_SETUP_MESSAGES] PROGMEM =
    SETUP_MESSAGES_CONTENT;

static struct aci_state_t aci_state;  // ACI state data
static hal_aci_evt_t aci_data;        // Command buffer

static bool addrReceived;
static bool addrSet = false;
static bool timing_change_done = false;
static int adTimeout;
static int adInterval;
static char deviceName[BTLE_DEVICE_NAME_SIZE];
static char longName[BTLE_BUFFER_SIZE];
static char deviceDesc[BTLE_BUFFER_SIZE];
static unsigned char
deviceAddress[BTLE_DEVICE_ADDRESS_SIZE];  // LSB to MSB byte order

// Define how assert should function in the BLE library
void __ble_assert(const char *file, uint16_t line) {
  DEBUG_PRINT(F("ERROR "));
  DEBUG_PRINT(file);
  DEBUG_PRINT(F(": "));
  DEBUG_PRINT(line);
  DEBUG_PRINT(F("\n"));
  while (1)
    ;
}

static int _REQ, _RDY, _RST;

// Methods
bool GoosciBleGatt::isInitialized() {
  return (aci_state.device_state == ACI_DEVICE_STANDBY);
}

bool GoosciBleGatt::isReadyToSend() {
  return lib_aci_is_pipe_available(&aci_state, PIPE_GOOSCI_SENSOR_VALUE_TX) && aci_state.data_credit_available > 0;
}

uint8_t packet[BTLE_BUFFER_SIZE];

bool GoosciBleGatt::sendData(const char *buffer, int32_t size) {
  const uint8_t max_packet_size = BTLE_BUFFER_SIZE - 2;
  /* Force size/max_packet_size to round up */
  uint8_t num_packets = (size + max_packet_size - 1) / max_packet_size;
  bool send_complete = true;
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
    if (!(send_complete = lib_aci_send_data(PIPE_GOOSCI_SENSOR_VALUE_TX, packet, current_packet_size + 2)))
      break;
      
    aci_state.data_credit_available--;
    while (!is_last_packet && !isReadyToSend())
      pollACI();
  }
  
  return send_complete;
}

void initLocalData(void) {
  if (strlen(deviceName) > 0) {
    lib_aci_set_local_data(&aci_state, PIPE_GAP_DEVICE_NAME_SET,
                           (unsigned char *)&deviceName, strlen(deviceName));
  }

  if (strlen(deviceDesc) > 0) {
    lib_aci_set_local_data(&aci_state, PIPE_GOOSCI_SENSOR_DESCRIPTION_SET,
                           (unsigned char *)deviceDesc, strlen(deviceDesc));
  }

  if (strlen(longName) > 0) {
    lib_aci_set_local_data(&aci_state, PIPE_DEVICE_INFORMATION_MODEL_NUMBER_STRING_BROADCAST,
			   (unsigned char *)longName, 8);
  }
}

/**************************************************************************
 *
 *   Constructor for the Goosci service
 *
 **************************************************************************/
GoosciBleGatt::GoosciBleGatt(int req, int rdy, int rst) {
  _REQ = req;
  _RDY = rdy;
  _RST = rst;

  memset(deviceName, 0x00, BTLE_DEVICE_NAME_SIZE);
  memset(deviceDesc, 0x00, BTLE_BUFFER_SIZE);
}

/**************************************************************************
 *
 *  Update the device name (7 characters or less!)
 *
 **************************************************************************/
void GoosciBleGatt::setDeviceName(const char *newName) {
  if (strlen(newName) > BTLE_DEVICE_NAME_SIZE - 1) {
    DEBUG_PRINTLN(F("Device name too long; new name was not set. "));
    return;
  } else {
    strcpy(deviceName, newName);
  }
}

/**************************************************************************
 *
 *  Copy the device name to the input buffer
 *
 **************************************************************************/
void GoosciBleGatt::getDeviceName(char *name) {
  if (strlen(deviceName) > 0) {
    strcpy(name, deviceName);
  }
}

/**************************************************************************
 *
 *  Update the long name (20 characters or less!)
 *
 **************************************************************************/
void GoosciBleGatt::setLongName(const char *newName) {
  if (strlen(newName) > BTLE_BUFFER_SIZE - 1) {
    DEBUG_PRINTLN(F("Name too long; new name was not set. "));
    return;
  } else {
    strcpy(longName, newName);
  }
}

/**************************************************************************
 *
 *  Set the device description (20 characters or less!).
 *  This is the place to store the long name.
 *
 **************************************************************************/
void GoosciBleGatt::setDeviceDescription(const char *desc) {
  // Set the value of the description characteristic.
  if (strlen(desc) > BTLE_BUFFER_SIZE - 1) {
    DEBUG_PRINTLN(F("Description too long; new description was not set."));
    return;
  } else {
    strcpy(deviceDesc, desc);
  }
}

/**************************************************************************
 *
 *   Configures the nRF8001 and starts advertising the service
 *
 *   @param[in]  advTimeout
 *               The advertising timeout in seconds
 *               (0 = infinite advertising).
 *   @param[in]  advInterval
 *               The delay between advertising packets in 0.625ms units
 *               (1600 = 1 second).
 *
 **************************************************************************/
bool GoosciBleGatt::begin(int advTimeout, int advInterval) {
  // Store the advertising timeout and interval
  // TODO(nmai): check for valid ranges.
  adTimeout = advTimeout;
  adInterval = advInterval;

  // Setup the service data from nRFGo Studio (services.h)
  if (NULL != services_pipe_type_mapping) {
    aci_state.aci_setup_info.services_pipe_type_mapping =
        &services_pipe_type_mapping[0];
  } else {
    aci_state.aci_setup_info.services_pipe_type_mapping = NULL;
  }

  aci_state.aci_setup_info.number_of_pipes = NUMBER_OF_PIPES;
  aci_state.aci_setup_info.setup_msgs = (hal_aci_data_t *)setup_msgs;
  aci_state.aci_setup_info.num_setup_msgs = NB_SETUP_MESSAGES;

  // Setup the nRF8001 pins.
  aci_state.aci_pins.board_name = BOARD_DEFAULT;
  aci_state.aci_pins.reqn_pin = _REQ;
  aci_state.aci_pins.rdyn_pin = _RDY;
  aci_state.aci_pins.mosi_pin = MOSI;
  aci_state.aci_pins.miso_pin = MISO;
  aci_state.aci_pins.sck_pin = SCK;

  // SPI_CLOCK_DIV8  = 2MHz SPI speed.
  aci_state.aci_pins.spi_clock_divider = SPI_CLOCK_DIV8;

  // The Active pin is optional and can be marked UNUSED.
  aci_state.aci_pins.reset_pin = _RST;
  aci_state.aci_pins.active_pin = UNUSED;
  aci_state.aci_pins.optional_chip_sel_pin = UNUSED;

  // Interrupts still not available in Chipkit.
  aci_state.aci_pins.interface_is_interrupt = false;
  aci_state.aci_pins.interrupt_number = 1;

  // The second parameter is for turning debug printing on
  // for the ACI Commands and Events so they be printed on the Serial
  lib_aci_init(&aci_state, false);

  // Get the device address
  lib_aci_get_address();
  // Wait for the get address response
  addrReceived = false;
  while (!addrReceived) {
    pollACI();
  }

  return true;
}

void GoosciBleGatt::get_address(void) {
  // Generate a unique name using the last 4 digits of device address.
  if (!addrSet) {
    char d[20];
    char l[8];
    snprintf(l, 8, "Sci%02x%02x\0", deviceAddress[1], deviceAddress[0]);
    setLongName(l);
    strcpy(d, l);
    setDeviceName(d);
    initLocalData();
    addrSet = true;
  }
}

void GoosciBleGatt::print_address(void) {
  get_address();
#ifdef GIT_COMMIT_HASH
  DEBUG_PRINT(F("Firmware version: "));
  DEBUG_PRINTLN(F(GIT_COMMIT_HASH));
#endif
#ifdef JENKINS_BUILD_ID
  DEBUG_PRINT(F("Jenkins build id: "));
  DEBUG_PRINTLN(F(JENKINS_BUILD_ID));
#endif
  DEBUG_PRINT(F("DeviceName: "));
  DEBUG_PRINTLN(deviceName);
  DEBUG_PRINT(F("DeviceDesc: "));
  DEBUG_PRINTLN(deviceDesc);
  DEBUG_PRINT(F("LongName: "));
  DEBUG_PRINTLN(longName);
}

/**************************************************************************
 *
 *   Handles low level ACI events, and passes them up to an application
 *   level callback when appropriate
 *
 **************************************************************************/
void GoosciBleGatt::pollACI() {
  static bool setup_required = false;

  // We enter the if statement only when there is a ACI event
  // available to be processed
  if (lib_aci_event_get(&aci_state, &aci_data)) {
    aci_evt_t *aci_evt;
    aci_evt = &aci_data.evt;
    // DEBUG_PRINTLN(F("pollACI"));
    // DEBUG_PRINT("evt opcode: ");
    // DEBUG_PRINTLN2(aci_evt->evt_opcode, HEX);
    // DEBUG_PRINT(F("State Total credit: "));
    // DEBUG_PRINTLN(aci_state.data_credit_total);
    // DEBUG_PRINT(F("State Available credit: "));
    // DEBUG_PRINTLN(aci_state.data_credit_available);
    // DEBUG_PRINT("Event Available credit: ");
    // DEBUG_PRINTLN(aci_evt->params.device_started.credit_available);
    switch (aci_evt->evt_opcode) {
      case ACI_EVT_DEVICE_STARTED: {
	// DEBUG_PRINTLN("STARTED");
        aci_state.data_credit_total =
            aci_evt->params.device_started.credit_available;
        switch (aci_evt->params.device_started.device_mode) {
          case ACI_DEVICE_SETUP: {
	    // DEBUG_PRINTLN("SETUP");
            aci_state.device_state = ACI_DEVICE_SETUP;
            setup_required = true;
            break;
          }

          case ACI_DEVICE_STANDBY: {
	    // DEBUG_PRINTLN("STANDBY");
            aci_state.device_state = ACI_DEVICE_STANDBY;

            // sleep_to_wakeup_timeout = 30;

            if (aci_evt->params.device_started.hw_error) {
              // Magic number used to make sure the HW error
              // event is handled correctly.
              delay(20);
            } else {
	      get_address();
	      // DEBUG_PRINTLN("lib_aci_connect");
              lib_aci_connect(adTimeout, adInterval);
            }
            break;
          }
          case ACI_DEVICE_INVALID: {
            DEBUG_PRINTLN(F("Evt Device Invalid"));
            break;
          }

          case ACI_DEVICE_TEST: {
            break;
          }

          case ACI_DEVICE_SLEEP: {
            break;
          }
        }
        break;  // case ACI_EVT_DEVICE_STARTED:
      }

      case ACI_EVT_CMD_RSP: {
        DEBUG_PRINTLN("ACI_EVT_CMD_RSP");
        // If an ACI command response event comes with an error -> stop
        if (ACI_STATUS_SUCCESS != aci_evt->params.cmd_rsp.cmd_status) {
          // ACI ReadDynamicData and ACI WriteDynamicData
          // will have status codes of
          // TRANSACTION_CONTINUE and TRANSACTION_COMPLETE
          // all other ACI commands will have status code of
          // ACI_STATUS_SCUCCESS for a successful command
          DEBUG_PRINT(F("ACI Command 0x"));
          DEBUG_PRINTLN2(aci_evt->params.cmd_rsp.cmd_opcode, HEX);
          DEBUG_PRINT(F("Evt Cmd response: Error. "));
          DEBUG_PRINTLN2(aci_evt->params.cmd_rsp.cmd_status, HEX);
        }
        if (ACI_CMD_GET_DEVICE_ADDRESS == aci_evt->params.cmd_rsp.cmd_opcode) {
          // If this is a response to get device address, save the address
          addrReceived = true;
          // DEBUG_PRINT(F("Get device address response: "));
          for (int i = BTLE_DEVICE_ADDRESS_SIZE - 1; i >= 0; --i) {
            deviceAddress[i] = aci_evt->params.cmd_rsp.params.get_device_address
                .bd_addr_own[i];
            //   DEBUG_PRINT2(deviceAddress[i], HEX);
            //   DEBUG_PRINT(F(":"));
          }
          // DEBUG_PRINTLN(F(""));
        } else if (ACI_CMD_OPEN_ADV_PIPE ==
                   aci_evt->params.cmd_rsp.cmd_opcode) {
          DEBUG_PRINTLN(
              F("Open advertising pipe response, setting service data."));
          lib_aci_set_local_data(
              &aci_state, PIPE_DEVICE_INFORMATION_MODEL_NUMBER_STRING_BROADCAST,
              (unsigned char *)longName, 8);
  
          DEBUG_PRINT(F("Advertising starting for "));
          DEBUG_PRINTLN(deviceName);
          lib_aci_connect(adTimeout, adInterval);
        } else if (ACI_CMD_GET_DEVICE_VERSION == aci_evt->params.cmd_rsp.cmd_opcode) {
          DEBUG_PRINTLN("ACI_CMD_GET_DEVICE_VERSION");
        } else if (ACI_CMD_SET_LOCAL_DATA == aci_evt->params.cmd_rsp.cmd_opcode) {
          DEBUG_PRINTLN("ACI_CMD_SET_LOCAL_DATA");
        } else if (ACI_CMD_CONNECT == aci_evt->params.cmd_rsp.cmd_opcode) {
          DEBUG_PRINTLN("ACI_CMD_CONNECT");
        } else if (ACI_CMD_DISCONNECT == aci_evt->params.cmd_rsp.cmd_opcode) {
          DEBUG_PRINTLN("ACI_CMD_DISCONNECT");
        } else if (ACI_CMD_CHANGE_TIMING == aci_evt->params.cmd_rsp.cmd_opcode) {
          DEBUG_PRINTLN("ACI_CMD_CHANGE_TIMING");
        }  else {
          // print command
          DEBUG_PRINT(F("Unexpected ACI Command 0x"));
          DEBUG_PRINTLN2(aci_evt->params.cmd_rsp.cmd_opcode, HEX);
        }
        break;
      }

      case ACI_EVT_CONNECTED: {
        // The nRF8001 is now connected to the peer device.
        DEBUG_PRINTLN(F("Evt Connected"));
        aci_state.data_credit_available = aci_state.data_credit_total;
        timing_change_done = false;
        break;
      }

      case ACI_EVT_PIPE_STATUS: {
        DEBUG_PRINTLN(F("Evt Pipe Status: "));
        // DEBUG_PRINT2((int) aci_evt->params.pipe_status.pipes_open_bitmap, HEX);
        // DEBUG_PRINT(" ");
        // DEBUG_PRINTLN2((int) aci_evt->params.pipe_status.pipes_closed_bitmap, HEX);
        if (lib_aci_is_pipe_available(&aci_state, PIPE_GOOSCI_SENSOR_VALUE_TX) && !timing_change_done) {
          lib_aci_change_timing(6, 6, 0, 600); // Low-latency parameters
        timing_change_done = true;
      }

        break;
      }

      case ACI_EVT_TIMING: {
        // DEBUG_PRINT("ACI_EVT_TIMING: ");
        // DEBUG_PRINT(aci_evt->params.timing.conn_rf_interval);
        // DEBUG_PRINT(" ");
        // DEBUG_PRINT(aci_evt->params.timing.conn_slave_rf_latency);
        // DEBUG_PRINT(" ");
        // DEBUG_PRINT(aci_evt->params.timing.conn_rf_timeout);
		// DEBUG_PRINT(" ");
        // DEBUG_PRINTLN(aci_evt->params.timing.conn_rf_interval);
        break;
      }

      case ACI_EVT_DISCONNECTED: {
        // Advertise again if the advertising timed out.
        DEBUG_PRINTLN(F("Evt Disconnected."));
        // TODO(dek): figure out why the transition to using credits
        // broke disconnection (packets are still transmitted).
        // Setting the credits to 0 was an experiment but it didn't work.
        // aci_state.data_credit_available = 0; 
        lib_aci_connect(adTimeout, adInterval);
        timing_change_done = false;
        break;
      }

      case ACI_EVT_DATA_RECEIVED: {
	DEBUG_PRINTLN("ACI_EVT_DATA_RECEIVED");
        if (aci_evt->params.data_received.rx_data.pipe_number == PIPE_GOOSCI_SENSOR_CONFIGURATION_RX_ACK_AUTO) {
          int8_t packet_length = aci_evt->len;
	  handle(aci_evt->params.data_received.rx_data.aci_data, aci_evt->len);
	} else {
	  DEBUG_PRINT(F(" Data(Hex) : "));
	  for (int i = 0; i < aci_evt->len - 2; i++) {
	    DEBUG_PRINT2(aci_evt->params.data_received.rx_data.aci_data[i], HEX);
	    DEBUG_PRINT(F(" "));
	  }
	  DEBUG_PRINTLN(F(""));
	}
	break;
      }


      case ACI_EVT_DATA_CREDIT: {
        // DEBUG_PRINTLN(F("Evt Credit: Peer has received our send"));
        aci_state.data_credit_available = aci_state.data_credit_available + aci_evt->params.data_credit.credit;
        break;
      }

      case ACI_EVT_PIPE_ERROR: {
        // See the appendix in the nRF8001
        // Product Specication for details on the error codes
        DEBUG_PRINT(F("ACI Evt Pipe Error: Pipe #:"));
        DEBUG_PRINT2(aci_evt->params.pipe_error.pipe_number, DEC);
        DEBUG_PRINT(F("  Pipe Error Code: 0x"));
        DEBUG_PRINTLN2(aci_evt->params.pipe_error.error_code, HEX);

        // Increment the credit available as the data packet was not sent.
        // The pipe error also represents the Attribute protocol
        // Error Response sent from the peer and that should not be counted
        // for the credit.
        if (ACI_STATUS_ERROR_PEER_ATT_ERROR !=
            aci_evt->params.pipe_error.error_code) {
          aci_state.data_credit_available++;
        }
        break;
      }

      case ACI_EVT_DATA_ACK: {
        // DEBUG_PRINTLN(F("ACK"));
        break;
      }
      case ACI_EVT_HW_ERROR: {
        DEBUG_PRINTLN(F("HW error: "));
        DEBUG_PRINTLN2(aci_evt->params.hw_error.line_num, DEC);

        for (int counter = 0; counter <= (aci_evt->len - 3); counter++) {
          DEBUG_PRINT(aci_evt->params.hw_error.file_name[counter]);
        }
        DEBUG_PRINTLN();
        initLocalData();
        lib_aci_connect(adTimeout, adInterval);
        break;
      }

      default: {
        DEBUG_PRINT(F("Evt Opcode 0x"));
        DEBUG_PRINT2(aci_evt->evt_opcode, HEX);
        DEBUG_PRINTLN(F(" unhandled"));
        break;
      }
    }
  }

  // setup_required is set to true when the device starts
  // up and enters setup mode.
  // It indicates that do_aci_setup() should be called.
  // The flag should be cleared if do_aci_setup() returns
  // ACI_STATUS_TRANSACTION_COMPLETE.
  if (setup_required) {
    int result = do_aci_setup(&aci_state);
    if (result != SETUP_SUCCESS ) {
      switch(result) {
        case SETUP_FAIL_COMMAND_QUEUE_NOT_EMPTY:
          DEBUG_PRINTLN("SETUP_FAIL_COMMAND_QUEUE_NOT_EMPTY");
          break;
        case SETUP_FAIL_EVENT_QUEUE_NOT_EMPTY:
          DEBUG_PRINTLN("SETUP_EVENT_COMMAND_QUEUE_NOT_EMPTY");
          break;
        case SETUP_FAIL_NOT_SETUP_EVENT:
          DEBUG_PRINTLN("SETUP_FAIL_NOT_SETUP_EVENT");
          break;
        case SETUP_FAIL_NOT_COMMAND_RESPONSE:
          DEBUG_PRINTLN("SETUP_FAIL_NOT_COMMAND_RESPONSE");
          break;
      }
    } else {
      setup_required = false;
    }
  }
}
