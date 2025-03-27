#include <Arduino.h>


#include "ble.h"
#include <BLEServer.h>




BLEService* bleService;
BLEServer* bleServer;
BLECharacteristic* bleCharacteristic;
BLECharacteristic* bleCharacteristic1;
unsigned long sleep_timer = 300000; //Init of global var
bool isConnected=false;





void MyCharacteristicCallbacks::onWrite(BLECharacteristic* characteristic){
    std::string receivedData = characteristic->getValue();
    Serial.println("Received data");
    Serial.println(receivedData.c_str());
    sleep_timer = atoi(receivedData.c_str());
}

void MyServerCallbacks::onConnect(BLEServer* pServer){
    Serial.println("Device connected");
    isConnected=true;
}

void MyServerCallbacks::onDisconnect(BLEServer* pServer){
    Serial.println("Device disconnected");
    isConnected=false;
}

void ble_init(){
    BLEDevice::init("TEST BLE RAHR");
    bleServer = BLEDevice::createServer();
    bleServer->setCallbacks(new MyServerCallbacks());
    bleService = bleServer->createService(BLEUUID((uint16_t)0x180D)); //TODO find a good id

    bleCharacteristic1 = bleService->createCharacteristic(BLEUUID((uint16_t)0x1A01), 
    BLECharacteristic::PROPERTY_READ | 
    BLECharacteristic::PROPERTY_NOTIFY |
    BLECharacteristic::PROPERTY_WRITE);

    bleCharacteristic = bleService->createCharacteristic(    BLEUUID((uint16_t)0x1A00), 
    BLECharacteristic::PROPERTY_READ | 
    BLECharacteristic::PROPERTY_NOTIFY);

    bleCharacteristic1->setCallbacks(new MyCharacteristicCallbacks());

}


