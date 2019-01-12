#ifndef _ComponentSensor_H_
#define _ComponentSensor_H_

#include <ESPAsyncWebServer.h>
#include "../services/configStore.h"


class ComponentSensor {
  public:
    ConfigStore config;
    String componentName;

    ComponentSensor(const char * name);
    void begin();

    virtual void setupWebServerHooks(AsyncWebServer&);

    virtual void setupComponent() = 0;
    virtual void handleComponent() = 0;

};


ComponentSensor* getComponent(String componentName);

extern std::vector<ComponentSensor*> * componentsPointers;
template<typename F>
void forEachComponent(F&& lambdaRef);
template<typename F>
void forEachComponent(F&& lambdaRef) {
  if (!componentsPointers) componentsPointers = new std::vector<ComponentSensor*>();
  for (auto it = componentsPointers->begin(); it != componentsPointers->end(); it++) {
    lambdaRef((*it));
  }
}
#endif