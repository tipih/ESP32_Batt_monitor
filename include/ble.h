#ifndef BLE_H
#define BLE_H

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

extern unsigned long sleep_timer;
extern bool isConnected;

void ble_init();
void ble_send_data(const std::string& data);
void ble_on_receive(BLECharacteristic* characteristic);

class MyServerCallbacks : public BLEServerCallbacks
{
    void onConnect(BLEServer* pServer);
    void onDisconnect(BLEServer* pServer);
};
#endif //BLE_H


