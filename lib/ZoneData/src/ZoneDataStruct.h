#include <string>
#include <list>

#ifndef __ZONEDATASTRUCT__
#define __ZONEDATASTRUCT__

#define ZONEDATA_EEPROM_SIZE 400
#define ZONEDATA_STRUCT_VERSION 0x01
#define ZONEDATA_ZONENAME_SIZE  11
#define ZONEDATA_MAX_ZONES ( (ZONEDATA_EEPROM_SIZE - sizeof(ZoneDataSerialHeader)) / sizeof(ZoneDataProperties) )

struct ZoneColor {
    unsigned char R = 0;
    unsigned char G = 0;
    unsigned char B = 0;
};

class ZoneDataProperties {
    friend class ZoneData;
private:
    char _zoneName[ZONEDATA_ZONENAME_SIZE];
    unsigned char _zoneID;
public:
    bool operator== (const ZoneDataProperties &rhs);
    unsigned char getZoneID();
    const char * getZoneName();
    void setZoneName(const std::string &name);
    ZoneColor RGB;
    unsigned char Brightness = 0;
    unsigned char isOn = 0;
    unsigned char pin = 0;
    unsigned char ledCount = 0;
};

class ZoneData {
public:
    static const unsigned char version = ZONEDATA_STRUCT_VERSION;
    unsigned char getNumberOfZones();
    const std::list<ZoneDataProperties> & getZonePropertyList();
    ZoneDataProperties * getZoneReferenceAtID(unsigned char zoneID);
    bool addZone(ZoneDataProperties &zone, unsigned char zoneID);
    bool removeZone(unsigned char zoneID);
    void deleteAllZones();
private:
    std::list<ZoneDataProperties> _props;
};

struct ZoneDataSerialHeader {
    const unsigned char version = ZoneData::version;
    unsigned char numberOfZones;
};

#endif