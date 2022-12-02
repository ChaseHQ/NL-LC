#include <string>
#include <BLEServer.h>
#include "BTManagerCallback.h"

#define SERVICE_UUID "382db2d8-07f4-4d24-957a-46e77b0cb345"
#define WRDATA_UUID  "9a433dc3-30fa-48b6-bd04-627a17ec0704"
#define SAVEMEM_UUID "697fff61-1ffb-4acd-bd50-67b9ea085e17"

#ifndef __BTMANAGER__
#define __BTMANAGER__

class BTManager : public BLEServerCallbacks, BLECharacteristicCallbacks {
public:
    BTManager(const std::string &name,BTManagerCallback* delegate, uint8_t * data, size_t dataSize);
    void onConnect(BLEServer *pServer);
    void onDisconnect(BLEServer *pServer);
    void onRead(BLECharacteristic* pCharacteristic);
    void onWrite(BLECharacteristic* pCharacteristic);
    void setWRData(uint8_t * data, size_t dataSize);
private:
    BTManagerCallback * _delegate = nullptr;
    uint8_t * _data = nullptr;
    size_t _dataSize = 0;
    BLEServer * pServer = nullptr;
};

#endif