#include <Arduino.h>
#include <Preferences.h>
#include <map>

#include "BTManager.h"
#include <ZoneDataFactory.h>
#include <NeoPixelBus.h>

#define PKEY_ZONEDATA "ZDKey"

BTManager * btm = nullptr;
Preferences preferences;
auto _zoneMap = std::map<uint8_t,NeoPixelBus<NeoGrbFeature, NeoEsp32RmtNWs2812xMethod> *>();

class PIN_MAP {
  public:
  static uint8_t GetPinForZone(uint8_t zoneID) {
    switch (zoneID) {
      case 0:
        return 15;
      break;
      case 1:
        return 2;
      break;
      case 2:
        return 0;
      break;
      case 3:
        return 4;
      break;
      case 4:
        return 16;
      break;
      case 5:
        return 17;
      break;
      case 6:
        return 22;
      break;
      case 7:
      default:
        return 23;
      break;
    }
  }
};

NeoPixelBus<NeoGrbFeature, NeoEsp32RmtNWs2812xMethod> * getSetupZone(ZoneDataProperties * props) {
  if (_zoneMap[props->getZoneID()] == nullptr) {
    _zoneMap[props->getZoneID()] = new NeoPixelBus<NeoGrbFeature, NeoEsp32RmtNWs2812xMethod>(props->ledCount,PIN_MAP::GetPinForZone(props->getZoneID()),NeoBusChannel(props->getZoneID()));
    _zoneMap[props->getZoneID()]->Begin();
    return _zoneMap[props->getZoneID()];
  } else {
    if (_zoneMap[props->getZoneID()]->PixelCount() != props->ledCount) {
      // Reset the zone
      _zoneMap[props->getZoneID()]->ClearTo(RgbColor(0));
      _zoneMap[props->getZoneID()]->Show();
      delete _zoneMap[props->getZoneID()];
      _zoneMap[props->getZoneID()] = nullptr;
      return getSetupZone(props);
    }
  }
  return _zoneMap[props->getZoneID()];
}

void updateLights() {
  //Serial.println("UpdateLights :: Updating Lights");
  NeoGamma<NeoGammaEquationMethod> color;
  for (auto zp : ZoneDataFactory::Instance()->getZoneData()->getZonePropertyList()){
    auto zone = getSetupZone(&zp);
    auto newColor = color.Correct(RgbColor(zp.RGB.R,zp.RGB.G,zp.RGB.B)).Dim(zp.Brightness);
    if (zp.isOn) {
      zone->ClearTo(newColor);
    } else {
      zone->ClearTo(RgbColor(0,0,0));
    }
    zone->Show();
  }
}

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
  //Serial.printf("PUBLISH :: Set ZoneData on BT Characteristic : Size: %zu\n", bufferSize);
  free(buffer);
  updateLights();
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
      //Serial.println("CALLBACK :: Read Event Occured");
    }
    void writeEvent(uint8_t * data, size_t size) {
      //Serial.printf("CALLBACK :: Write Event with Data Occured :: Bytes Recieved: %zu\n",size);
      if (ZoneDataFactory::Instance()->deserialize(data)) {
        //Serial.println("ZoneDataFactory :: Successfully De-Serialized Write Event");
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
    if (!ZoneDataFactory::Instance()->deserialize(buffer)) {
      Serial.printf("EEPROM :: Failure Restoring State of Zone Data - Defaulting");
      setDefaultZones();
      saveZoneData();
    }
    free(buffer);
  }

  updateLights();

  uint8_t * buffer;
  buffer = (uint8_t *) malloc(ZoneDataFactory::Instance()->serializationBufferSize());
  ZoneDataFactory::Instance()->serialize(buffer);
  btm = new BTManager("NorthernLights-LC",new CBHandle(),buffer, ZoneDataFactory::Instance()->serializationBufferSize());
  free(buffer);
}

void loop() {
  sleep(1);
}

