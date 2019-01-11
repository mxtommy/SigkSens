#include "sensorComponent.h"

SensorComponent::SensorComponent(const char * name) {
  configFilename = "/" + String(name) + ".json";

}

void SensorComponent::begin() {
  config.begin(configFilename.c_str());
  setupComponent();
}

void SensorComponent::setupComponent() {} 
void SensorComponent::handleComponent() {}

void SensorComponent::setupWebServerHooks(AsyncWebServer& server){ }

