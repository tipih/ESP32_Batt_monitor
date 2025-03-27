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


class MyServerCallbacks : public BLEServerCallbacks
{
    void onConnect(BLEServer* pServer) override; 
    void onDisconnect(BLEServer* pServer) override;
};


class MyCharacteristicCallbacks : public BLECharacteristicCallbacks{
    void onWrite(BLECharacteristic* characteristic) override;
};

#endif //BLE_H