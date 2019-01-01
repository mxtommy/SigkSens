#ifndef _discovery_H_
#define _discovery_H_
#include <ESPAsyncWebServer.h>

void setupDiscovery();
void setupSSDPHttpCallback(AsyncWebServer&);

#endif
