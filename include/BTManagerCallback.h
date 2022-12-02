
#ifndef __BTMANAGERCALLBACK__
#define __BTMANAGERCALLBACK__

class BTManagerCallback {
public:
    virtual void connected(void);
    virtual void disconnected(void);
    virtual void readEvent();
    virtual void writeEvent(uint8_t * data, size_t size);
    virtual void saveRequest();
};

#endif