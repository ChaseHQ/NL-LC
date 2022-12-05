#include "BTManager.h"

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

class MyCallbacks : public BLEServerCallbacks {
    void onConnect(BLEServer * pServer) {
        Serial.println("BTManager :: Someone Connected");
    }

    void onDisconnect(BLEServer * pServer) {
        Serial.println("BTManager :: Someone Disconnected");
    }
};

BTManager::BTManager(const std::string &name,BTManagerCallback* delegate, uint8_t * data, size_t dataSize) : _delegate(delegate), _data(data), _dataSize(dataSize) {
    Serial.println(String("BTManager :: Starting Device '") + name.c_str() + "'");
    BLEDevice::init(name);
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(this);
    BLEService *pService = pServer->createService(SERVICE_UUID);
    
    BLECharacteristic *pCharacteristic = pService->createCharacteristic(
        WRDATA_UUID,
        BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_NOTIFY
    );
    pCharacteristic->setCallbacks(this);
    pCharacteristic->setValue(_data,_dataSize);

    BLECharacteristic *pSaveCharacteristic = pService->createCharacteristic(
        SAVEMEM_UUID,
        BLECharacteristic::PROPERTY_WRITE
    );
    pSaveCharacteristic->setCallbacks(this);

    BLEDevice::setMTU(510);
    BLEDevice::setPower(ESP_PWR_LVL_P9, ESP_BLE_PWR_TYPE_DEFAULT);
    BLEDevice::setPower(ESP_PWR_LVL_P9, ESP_BLE_PWR_TYPE_ADV);
    BLEDevice::setPower(ESP_PWR_LVL_P9, ESP_BLE_PWR_TYPE_CONN_HDL0);
    pService->start();

    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06);
    pAdvertising->setMinPreferred(0x12);
    BLEDevice::startAdvertising();
}

void BTManager::onConnect(BLEServer *pServer) {
    Serial.println("BTManager :: Connection Recieved " + pServer->getConnId());
    if (_delegate != nullptr)
        _delegate->connected();
}

void BTManager::onDisconnect(BLEServer *pServer) {
    Serial.println("BTManager :: Peer Disconnected - Restarting Advertisement");
    BLEDevice::startAdvertising();
    if (_delegate != nullptr)
        _delegate->disconnected();
}

void BTManager::onRead(BLECharacteristic* pCharacteristic) {
    //Serial.println("BTManager :: Read Event");
    _delegate->readEvent();
}

void BTManager::onWrite(BLECharacteristic* pCharacteristic) {
    //Serial.println("BTManager :: Write Event");
    if (pCharacteristic->getUUID().equals(BLEUUID(WRDATA_UUID))) {
        _delegate->writeEvent(pCharacteristic->getData(), pCharacteristic->getLength());
    } else if (pCharacteristic->getUUID().equals(BLEUUID(SAVEMEM_UUID))) {
        _delegate->saveRequest();
    }
}

void BTManager::setWRData(uint8_t * data, size_t dataSize) {
    pServer->getServiceByUUID(BLEUUID(SERVICE_UUID))->getCharacteristic(BLEUUID(WRDATA_UUID))->setValue(data,dataSize);
    pServer->getServiceByUUID(BLEUUID(SERVICE_UUID))->getCharacteristic(BLEUUID(WRDATA_UUID))->notify();
}