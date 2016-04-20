#include <CurieBLE.h>
#include "goosci_utility.h"
#include "arduino_nrf_pins.h"
#include "heartbeat.h"
#define SENSOR_PIN A0    // Input Pin to read from

BLEPeripheral blePeripheral; // create peripheral instance
BLEService whistlepunkService("555a0001-0aaa-467a-9538-01f0652c74e8"); // create service
// Must be 20 char long to accomodate full-size messages.
const char *value = "                     ";

String BleLongName = "Sci";
bool serialConnected = false;

BLECharacteristic valueCharacteristic("555a0003-0aaa-467a-9538-01f0652c74e8", BLENotify, value);

#include "internal/ble_client.h"
#include "services/ble/ble_service_gap_api.h"
   

void blePeripheralConnectHandler(BLECentral& central) {
  // central connected event handler
  DEBUG_PRINT("Connected event, central: ");
  DEBUG_PRINTLN(central.address());
}

void blePeripheralDisconnectHandler(BLECentral& central) {
  // central disconnected event handler
  DEBUG_PRINT("Disconnected event, central: ");
  DEBUG_PRINTLN(central.address());
}


void bleNotificationSubscribeHandler(BLECentral& central, BLECharacteristic& characteristic) {
  // value characteristic event handler
  DEBUG_PRINT("Subscribe event, central: ");
  DEBUG_PRINTLN(central.address());
}

void bleNotificationUnsubscribeHandler(BLECentral& central, BLECharacteristic& characteristic) {
  // value characteristic unsubscribe event handler
  DEBUG_PRINT("Unsubscribe event, central: ");
  DEBUG_PRINTLN(central.address());
}

void setup() {
  wait_for_serial();
  
  // set the local name peripheral advertises
  blePeripheral.setLocalName("Initial");
  // set the UUID for the service this peripheral advertises:
  blePeripheral.setAdvertisedServiceUuid(whistlepunkService.uuid());

  // add service and characteristics
  blePeripheral.addAttribute(whistlepunkService);
  blePeripheral.addAttribute(valueCharacteristic);


  // assign event handlers for connected, disconnected to peripheral
  blePeripheral.setEventHandler(BLEConnected, blePeripheralConnectHandler);
  blePeripheral.setEventHandler(BLEDisconnected, blePeripheralDisconnectHandler);
  valueCharacteristic.setEventHandler(BLESubscribed, bleNotificationSubscribeHandler);
  valueCharacteristic.setEventHandler(BLEUnsubscribed, bleNotificationUnsubscribeHandler);
  
  ble_addr_t _local_bda;
  char       _device_name[BLE_MAX_DEVICE_NAME+1];  
  ble_client_get_factory_config(&_local_bda, _device_name);
  BleLongName += String(_local_bda.addr[4], HEX);
  BleLongName += String(_local_bda.addr[5], HEX);
  DEBUG_PRINT("Address is: ");
  DEBUG_PRINTLN(BleLongName);
  blePeripheral.setLocalName(BleLongName.c_str());

  // advertise the service
  blePeripheral.begin();  
}


void loop() {
  if (Serial) {
    if (!serialConnected) {
        serialConnected = true;
        DEBUG_PRINT(F("LongName: "));
        DEBUG_PRINTLN(BleLongName);
    }
  } else {
    if (serialConnected)
        serialConnected = false;
  }

  // poll peripheral
  blePeripheral.poll();

  if (valueCharacteristic.subscribed()) {
      int sensorValue = analogRead(SENSOR_PIN);
      send_data(valueCharacteristic, millis(), sensorValue);
  }
#ifdef GOOSCI_DEVELOPER_MODE
  heartbeat();
#endif

  heartbeat();
}

