#include "ZoneDataStruct.h"
#include <string.h>

unsigned char ZoneDataProperties::getZoneID() {
    return _zoneID;
}

const char * ZoneDataProperties::getZoneName() {
    return _zoneName;
}

void ZoneDataProperties::setZoneName(const std::string &name) {
    memset(_zoneName, 0, ZONEDATA_ZONENAME_SIZE);
    name.copy(_zoneName, ZONEDATA_ZONENAME_SIZE - 1);
}

bool ZoneDataProperties::operator== (const ZoneDataProperties &rhs) {
    if (this->_zoneID == rhs._zoneID) {
        return true;
    }
    return false;
}

bool ZoneData::addZone(ZoneDataProperties &zone, unsigned char zoneID) {
    if (getNumberOfZones() == ZONEDATA_MAX_ZONES) return false;
    for (auto const &savedZone : _props) {
        if (savedZone._zoneID == zoneID) {
            return false;
        }
    }
    zone._zoneID = zoneID;
    _props.push_back(zone);
    return true;
}

const std::list<ZoneDataProperties> & ZoneData::getZonePropertyList() {
    return _props;
}

unsigned char ZoneData::getNumberOfZones() {
    return _props.size();
}

bool ZoneData::removeZone(unsigned char zoneID) {
    for (auto savedZone : _props) {
        if (savedZone.getZoneID() == zoneID) {
            _props.remove(savedZone);
            return true;
        }
    }
    return false;
}

ZoneDataProperties * ZoneData::getZoneReferenceAtID(unsigned char zoneID) {
    for (auto it = _props.begin(); it != _props.end(); ++it) {
        if (it->_zoneID == zoneID) {
            return &*it;
        }
    }
    return nullptr;
}

void ZoneData::deleteAllZones() {
    _props.clear();
}