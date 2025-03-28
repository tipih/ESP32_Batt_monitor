#include <Arduino.h>


#include "ble.h"
#include <BLEServer.h>




BLEService* bleService;
BLEServer* bleServer;
BLECharacteristic* bleCharacteristic;
BLECharacteristic* bleCharacteristic1;
MyCharacteristicCallbacks* myCharacteristicCallbacks;
unsigned long sleep_timer = 300000; //Init of global var
bool isConnected=false;





void MyCharacteristicCallbacks::onWrite(BLECharacteristic* characteristic){
    std::string receivedData = characteristic->getValue();
    BLEUUID myID = characteristic->getUUID();

    Serial.println("Received data");
    Serial.println(receivedData.c_str());
    Serial.println(myID.toString().c_str());
    sleep_timer = atoi(receivedData.c_str());
}

void MyServerCallbacks::onConnect(BLEServer* pServer){
    Serial.println("Device connected");
    isConnected=true;
    pServer->startAdvertising();
}

void MyServerCallbacks::onDisconnect(BLEServer* pServer){
    Serial.println("Device disconnected");
    isConnected=false;
    pServer->startAdvertising();
}

void ble_init(){
    Serial.print("Init of BLE");
    BLEDevice::init("TEST BLE RAHR");
    bleServer = BLEDevice::createServer();
    bleServer->setCallbacks(new MyServerCallbacks());
    bleService = bleServer->createService(BLEUUID((uint16_t)0x180D)); //TODO find a good id

    myCharacteristicCallbacks = new MyCharacteristicCallbacks();


    bleCharacteristic1 = bleService->createCharacteristic(BLEUUID((uint16_t)0x1A01), 
    BLECharacteristic::PROPERTY_READ | 
    BLECharacteristic::PROPERTY_NOTIFY |
    BLECharacteristic::PROPERTY_WRITE);

    bleCharacteristic = bleService->createCharacteristic(    BLEUUID((uint16_t)0x1A00), 
    BLECharacteristic::PROPERTY_READ | 
    BLECharacteristic::PROPERTY_NOTIFY |
    BLECharacteristic::PROPERTY_WRITE);

    bleCharacteristic1->setCallbacks(myCharacteristicCallbacks);
    bleCharacteristic->setCallbacks(myCharacteristicCallbacks);

    BLEDescriptor VariableDescriptor(BLEUUID((uint16_t)0x2901));  /*```````````````````````````````````````````````````````````````*/
    VariableDescriptor.setValue("Show Battery Voltage");          /* Use this format to add a hint for the user. This is optional. */
    bleCharacteristic->addDescriptor(&VariableDescriptor);        /*```````````````````````````````````````````````````````````````*/
    
    


    BLEDescriptor VariableDescriptor1(BLEUUID((uint16_t)0x2901));  /*```````````````````````````````````````````````````````````````*/
    VariableDescriptor1.setValue("Show timeout");                  /* Use this format to add a hint for the user. This is optional. */
    bleCharacteristic1->addDescriptor(&VariableDescriptor1);       /*```````````````````````````````````````````````````````````````*/
    
    bleCharacteristic1->addDescriptor(new BLE2902());   //Add this line only if the characteristic has the Notify property
    bleCharacteristic->addDescriptor(new BLE2902());    //Add this line only if the characteristic has the Notify property

    bleService->start();
    bleServer->getAdvertising()->start();
    Serial.print("BLE started");
}


//Test function insted of using class override
void onCharWrite(BLECharacteristic* characteristic){
    std::string rxValue = characteristic->getValue();
    Serial.print("value received = ");
    Serial.println(rxValue.c_str());
}

void ble_update_timeout(const std::string& data){
    Serial.print("Sending data  ");
    Serial.println(data.c_str());
    bleCharacteristic1->setValue(data);
    bleCharacteristic1->notify();
}

void ble_update_voltage(const std::string& data){
    Serial.print("Sending data  ");
    Serial.println(data.c_str());
    bleCharacteristic->setValue(data);
    bleCharacteristic->notify();

}