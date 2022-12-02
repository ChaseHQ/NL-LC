#include "ZoneDataFactory.h"

ZoneDataFactory * ZoneDataFactory::_singleton = nullptr;
std::mutex ZoneDataFactory::_mutex;

ZoneDataFactory::ZoneDataFactory() {

}

ZoneDataFactory * ZoneDataFactory::Instance() {
    std::lock_guard<std::mutex> lock(_mutex); // Thread Safe Singleton
    if (_singleton == nullptr) {
        _singleton = new ZoneDataFactory();
    }
    return _singleton;
}

ZoneData * ZoneDataFactory::getZoneData() {
    std::lock_guard<std::mutex> lock(_mutex);
    if (_zoneData == nullptr) {
        _zoneData = new ZoneData;
    }
    return _zoneData;
}

ZoneDataFactory::~ZoneDataFactory() {
    deleteZoneData();
}

void ZoneDataFactory::deleteZoneData() {
    if (_zoneData != nullptr) {
        delete _zoneData;
        _zoneData = nullptr;
    }
}

size_t ZoneDataFactory::serializationBufferSize() {
    ZoneData * zd = getZoneData();
    return sizeof(ZoneDataSerialHeader) + (sizeof(ZoneDataProperties) * zd->getNumberOfZones());
}

void ZoneDataFactory::serialize(uint8_t * buffer) {
    ZoneData * zd = getZoneData();
    size_t buffPtr = 0;

    buffer[buffPtr++] = ZoneData::version;
    buffer[buffPtr++] = zd->getNumberOfZones();

    for (auto zone : zd->getZonePropertyList()) {
        buffer[buffPtr++] = zone.getZoneID();
        for (size_t nameSize = 0; nameSize < ZONEDATA_ZONENAME_SIZE; ++nameSize) {
            buffer[buffPtr++] = zone.getZoneName()[nameSize];
        }
        buffer[buffPtr++] = zone.Brightness;
        buffer[buffPtr++] = zone.isOn;
        buffer[buffPtr++] = zone.RGB.R;
        buffer[buffPtr++] = zone.RGB.G;
        buffer[buffPtr++] = zone.RGB.B;
        buffer[buffPtr++] = zone.pin;
        buffer[buffPtr++] = zone.ledCount;
    }
}

bool ZoneDataFactory::deserialize(uint8_t * buffer) {
    ZoneDataSerialHeader head;
    size_t buffPtr = 0;

    if (buffer[buffPtr++] != ZoneData::version) {
        return false;
    }
    head.numberOfZones = buffer[buffPtr++];
    ZoneDataFactory::Instance()->deleteZoneData();

    for (size_t numOfZones = 0; numOfZones < head.numberOfZones; ++numOfZones) {
        ZoneDataProperties zone;
        unsigned char reqZoneId = buffer[buffPtr++];
        
        zone.setZoneName((char *)&buffer[buffPtr]);
        buffPtr += ZONEDATA_ZONENAME_SIZE;

        zone.Brightness = buffer[buffPtr++];
        zone.isOn = buffer[buffPtr++];
        zone.RGB.R = buffer[buffPtr++];
        zone.RGB.G = buffer[buffPtr++];
        zone.RGB.B = buffer[buffPtr++];
        zone.pin = buffer[buffPtr++];
        zone.ledCount = buffer[buffPtr++];

        if (!ZoneDataFactory::Instance()->getZoneData()->addZone(zone,reqZoneId)) {
            return false;
        }
    }
    return true;
}