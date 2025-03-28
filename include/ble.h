// Michael Rahr 03-28-2025
//  This code is under Public Domain License.
// Overall function to handle all ble communication and setup

#ifndef BLE_H
#define BLE_H

#include <BLE2902.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>

extern unsigned long sleep_timer;
extern bool isConnected;

void bleInit();
void bleUpdateTimeout(const std::string &data);
void bleUpdateVoltage(const std::string &data);

void onCharWrite(BLECharacteristic *characteristic);

class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer *pServer) override;
  void onDisconnect(BLEServer *pServer) override;
};

class MyCharacteristicCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *characteristic) override;
};

#endif // BLE_H