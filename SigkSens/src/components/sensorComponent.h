#ifndef _sensorComponent_H_
#define _sensorComponent_H_

#include <ESPAsyncWebServer.h>
#include "../services/configStore.h"

class SensorComponent {
  public:
    
    ConfigStore config;
    String configFilename;

    SensorComponent(const char * name);
    void begin();
    
    virtual void setupWebServerHooks(AsyncWebServer&);

    virtual void setupComponent();
    virtual void handleComponent();

};

#endif