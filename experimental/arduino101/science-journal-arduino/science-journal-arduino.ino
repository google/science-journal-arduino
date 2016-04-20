#include <CurieBLE.h>
#include "goosci_utility.h"
#include "arduino_nrf_pins.h"
#include "heartbeat.h"
#define SENSOR_PIN A0    // Input Pin to read from

BLEPeripheral blePeripheral; // create peripheral instance
BLEService whistlepunkService("555a0001-0aaa-467a-9538-01f0652c74e8"); // create service
const char *value = "test";

BLECharacteristic valueCharacteristic("555a0003-0aaa-467a-9538-01f0652c74e8", BLERead | BLENotify, value);

#include "internal/ble_client.h"
#include "services/ble/ble_service_gap_api.h"
    ble_addr_t _local_bda;
    char       _device_name[BLE_MAX_DEVICE_NAME+1];

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
DEBUG_PRINTLN("Bluetooth device active, waiting for address...");
    ble_client_get_factory_config(&_local_bda, _device_name);
  DEBUG_PRINT("Type: ");
  DEBUG_PRINTLN(_local_bda.type);
  DEBUG_PRINT("Addr: ");
  for (int i = 0; i <= BLE_ADDR_LEN; ++i) {
    Serial.print(_local_bda.addr[i], HEX);
  }
    DEBUG_PRINTLN("");

  String name = "Sci";
  name += String(_local_bda.addr[4], HEX);
  name += String(_local_bda.addr[5], HEX);
  

  blePeripheral.setLocalName(name.c_str());

  // advertise the service
  blePeripheral.begin();  
  


  DEBUG_PRINTLN("Bluetooth device active, waiting for connections...");
}


void loop() {
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

