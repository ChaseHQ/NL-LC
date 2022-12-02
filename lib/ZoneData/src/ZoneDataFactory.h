#include "ZoneDataStruct.h"
#include <mutex>
#include <string>

#ifndef __ZONEDATAFACTORY__
#define __ZONEDATAFACTORY__

class ZoneDataFactory { 
private:
    ZoneData * _zoneData = nullptr;
protected:
    static ZoneDataFactory * _singleton;
    static std::mutex _mutex;
    ZoneDataFactory();
    ~ZoneDataFactory();
public:
    static ZoneDataFactory * Instance();
    ZoneData * getZoneData();
    void deleteZoneData();
    size_t serializationBufferSize();
    void serialize(uint8_t * buffer);
    bool deserialize(uint8_t * buffer);
};

#endif