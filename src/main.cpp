#include <Arduino.h>
#include <Preferences.h>

#include "BTManager.h"
#include <ZoneDataFactory.h>

#define PKEY_ZONEDATA "ZDKey"

BTManager * btm = nullptr;
Preferences preferences;

void setDefaultZones() {
  ZoneDataProperties zdp;
  zdp.setZoneName("Default");
  ZoneDataFactory::Instance()->getZoneData()->addZone(zdp,0);
}

void saveZoneData() {
  uint8_t * buffer;
  size_t bufferSize = ZoneDataFactory::Instance()->serializationBufferSize();
  buffer = (uint8_t *) malloc(bufferSize);
  ZoneDataFactory::Instance()->serialize(buffer);
  Serial.printf("SAVEZONEDATA :: Writing ZoneData to EEPROM : Size: %zu\n",bufferSize);
  preferences.putBytes(PKEY_ZONEDATA,buffer,bufferSize);
  free(buffer);
}

void publishZoneData() {
  uint8_t * buffer;
  size_t bufferSize = ZoneDataFactory::Instance()->serializationBufferSize();
  buffer = (uint8_t *) malloc(bufferSize);
  ZoneDataFactory::Instance()->serialize(buffer);
  btm->setWRData(buffer,bufferSize);
  Serial.printf("PUBLISH :: Set ZoneData on BT Characteristic : Size: %zu\n", bufferSize);
  free(buffer);
}

class CBHandle : public BTManagerCallback {
  public:
    void connected() {
      Serial.println("CALLBACK :: Someone Connected");
    }
    void disconnected() {
      Serial.println("CALLBACK :: Disconnected");
    }
    void readEvent() {
      Serial.println("CALLBACK :: Read Event Occured");
    }
    void writeEvent(uint8_t * data, size_t size) {
      Serial.printf("CALLBACK :: Write Event with Data Occured :: Bytes Recieved: %zu\n",size);
      if (ZoneDataFactory::Instance()->deserialize(data)) {
        Serial.println("ZoneDataFactory :: Successfully De-Serialized Write Event");
        publishZoneData();
      } else {
        Serial.println("ZoneDataFactory :: ERROR :: Could not deserialize the write event");
        publishZoneData();
      }
    }
    void saveRequest() {
      Serial.println("CALLBACK :: Save To EEPROM Request");
      saveZoneData();
    }
};

void setup() {
  Serial.begin(115200);
  preferences.begin("ZoneData",false);

  if (!preferences.getBytesLength(PKEY_ZONEDATA)){
    // No Zone Data - Set up some Default Stuff
    Serial.println("Warning :: No ZoneData found in EEPROM - Setting Defaults");
    setDefaultZones();
    saveZoneData();
  } else {
    uint8_t * buffer;
    size_t bufferSize = preferences.getBytesLength(PKEY_ZONEDATA);
    buffer = (uint8_t *) malloc(bufferSize);
    preferences.getBytes(PKEY_ZONEDATA, buffer, bufferSize);
    Serial.printf("EEPROM :: Restoring state of ZoneData : Size: %zu\n",bufferSize);
    ZoneDataFactory::Instance()->deserialize(buffer);
    free(buffer);
  }

  uint8_t * buffer;
  buffer = (uint8_t *) malloc(ZoneDataFactory::Instance()->serializationBufferSize());
  ZoneDataFactory::Instance()->serialize(buffer);
  btm = new BTManager("NorthernLights-LC",new CBHandle(),buffer, ZoneDataFactory::Instance()->serializationBufferSize());
  free(buffer);
}

void loop() {
  delay(2000);
}
