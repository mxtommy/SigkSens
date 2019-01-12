#include "ComponentSensor.h"
#include <vector>

std::vector<ComponentSensor*> * componentsPointers;// = nullptr; //nullptr means it's initialized before other classes

void registerComponent(ComponentSensor* sc) {
  if (!componentsPointers) componentsPointers = new std::vector<ComponentSensor*>();
  componentsPointers->push_back(sc);
}

void listComponents() {
  if (!componentsPointers) componentsPointers = new std::vector<ComponentSensor*>();
  for (auto it = componentsPointers->begin(); it != componentsPointers->end(); it++) {
    Serial.println((*it)->componentName);
  }
}

ComponentSensor::ComponentSensor(const char * name) {
  componentName = String(name);
  registerComponent(this);
}

void ComponentSensor::begin() {
  Serial.print("Starting component: "); Serial.println(componentName);
  config.begin(String("/" + String(componentName) + ".json").c_str());
  setupComponent();
}

void ComponentSensor::setupWebServerHooks(AsyncWebServer& server){ }

