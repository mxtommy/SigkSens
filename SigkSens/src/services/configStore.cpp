
#include <FS.h> //this needs to be first, or it all crashes and burns...

#ifdef ESP32
#include "SPIFFS.h"
#endif

#include <ArduinoJson.h>     //https://github.com/bblanchon/ArduinoJson
#include <string>

#include "configStore.h"
#include <map>

ConfigStore configStore;


ConfigStore::ConfigStore():_started(false),_needSave(false) {}

ConfigStore::~ConfigStore(){
    end();
}

bool ConfigStore::begin(const char * name) {
  if(_started){
        return false;
  }
  _configFileName = name;
  if (!SPIFFS.exists(name)) {
    
  } else {
    //file exists, reading and loading
    Serial.println(F("opening config file"));
    File configFile = SPIFFS.open(name, "r");
    if (configFile) {
      size_t size = configFile.size();
      // Allocate a buffer to store contents of the file.
      std::unique_ptr<char[]> buf(new char[size]);
      configFile.readBytes(buf.get(), size);
      DynamicJsonBuffer jsonBuffer;
      JsonObject& root = jsonBuffer.parseObject(buf.get());
      Serial.println(F("Current Configuration:"));
      root.prettyPrintTo(Serial);
      configFile.close();

      //bool
      JsonObject& nodeBool = root["bool"];
      for (auto kvp : nodeBool) {
        _mapBool[kvp.key] = kvp.value;
      }
      //String
      JsonObject& nodeString = root["string"];
      for (auto kvp : nodeString) {
        String temp = kvp.value;
        _mapString[kvp.key] = temp;
      }
      //float
      JsonObject& nodeFloat = root["float"];
      for (auto kvp : nodeFloat) {
        _mapFloat[kvp.key] = kvp.value;
      }
      //double
      JsonObject& nodeDouble = root["double"];
      for (auto kvp : nodeDouble) {
        _mapDouble[kvp.key] = kvp.value;
      }
      //int8
      JsonObject& nodeInt8 = root["int8"];
      for (auto kvp : nodeInt8) {
        _mapInt8[kvp.key] = kvp.value;
      }
      //uint8
      JsonObject& nodeUInt8 = root["uint8"];
      for (auto kvp : nodeUInt8) {
        _mapUInt8[kvp.key] = kvp.value;
      }
      //int16
      JsonObject& nodeInt16 = root["int16"];
      for (auto kvp : nodeInt16) {
        _mapInt16[kvp.key] = kvp.value;
      }
      //uint16
      JsonObject& nodeUInt16 = root["uint16"];
      for (auto kvp : nodeUInt16) {
        _mapUInt16[kvp.key] = kvp.value;
      }
      //int32
      JsonObject& nodeInt32 = root["int32"];
      for (auto kvp : nodeInt32) {
        _mapInt32[kvp.key] = kvp.value;
      }
      //uint32
      JsonObject& nodeUInt32 = root["uint32"];
      for (auto kvp : nodeUInt32) {
        _mapUInt32[kvp.key] = kvp.value;
      }

      _started = true;
      jsonBuffer.clear();
      return true;
    }
  } 
  return false;
}


void ConfigStore::handle() {
  if (_needSave) { 
    _needSave = false;
    saveConfig();
    }
}

void ConfigStore::saveConfig() {
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  //bool
  JsonObject& nodeBool = root.createNestedObject("bool");
  for (const auto& kv : _mapBool) {
    nodeBool[kv.first] = kv.second ;
  }
  //String
  JsonObject& nodeString = root.createNestedObject("string");
  for (const auto& kv : _mapString) {
    nodeString[kv.first] = kv.second ;
  }
  //Float
  JsonObject& nodeFloat = root.createNestedObject("float");
  for (const auto& kv : _mapFloat) {
    nodeFloat[kv.first] = kv.second ;
  }
  //Double
  JsonObject& nodeDouble = root.createNestedObject("double");
  for (const auto& kv : _mapDouble) {
    nodeDouble[kv.first] = kv.second ;
  }
  //Int8
  JsonObject& nodeInt8 = root.createNestedObject("int8");
  for (const auto& kv : _mapInt8) {
    nodeInt8[kv.first] = kv.second ;
  }
 //UInt8
  JsonObject& nodeUInt8 = root.createNestedObject("uint8");
  for (const auto& kv : _mapUInt8) {
    nodeUInt8[kv.first] = kv.second ;
  }  
  //Int16
  JsonObject& nodeInt16 = root.createNestedObject("int16");
  for (const auto& kv : _mapInt16) {
    nodeInt16[kv.first] = kv.second ;
  }
 //UInt16
  JsonObject& nodeUInt16 = root.createNestedObject("uint16");
  for (const auto& kv : _mapUInt16) {
    nodeUInt16[kv.first] = kv.second ;
  }
  //Int32
  JsonObject& nodeInt32 = root.createNestedObject("int32");
  for (const auto& kv : _mapInt16) {
    nodeInt32[kv.first] = kv.second ;
  }
 //UInt32
  JsonObject& nodeUInt32 = root.createNestedObject("uint32");
  for (const auto& kv : _mapUInt32) {
    nodeUInt32[kv.first] = kv.second ;
  }

  File configFile = SPIFFS.open(_configFileName, "w");
  if (!configFile) {
    Serial.println(F("failed to open config file for writing"));
  }
  root.printTo(configFile);
  configFile.close();
  root.prettyPrintTo(Serial);
  Serial.println("Configuration saved to SPIFFS");
}


void ConfigStore::end(){
  _started = false;
}

bool ConfigStore::clear(){
  if (SPIFFS.exists(_configFileName)) {
    SPIFFS.remove(_configFileName);
  }
  _mapString.clear();
  _mapFloat.clear();
  _mapDouble.clear();
  _mapBool.clear();
  _mapInt8.clear();
  _mapUInt8.clear();
  _mapInt16.clear();
  _mapUInt16.clear();
  _mapInt32.clear();
  _mapUInt32.clear();
  end();
  return true;
}

/*----------------------------------------------------------------------------
------------------------------------------------------------------------------
Setters
------------------------------------------------------------------------------
----------------------------------------------------------------------------*/


size_t ConfigStore::putBool(const char* key, bool value) { 
  _mapBool[key] = value;
  _needSave = true;
  return sizeof(value);
}

size_t ConfigStore::putInt8(const char* key, int8_t value) { 
  _mapInt8[key] = value;
  _needSave = true;
  return sizeof(value);
}
 
size_t ConfigStore::putUInt8(const char* key, uint8_t value) { 
  _mapUInt8[key] = value;
  _needSave = true;
  return sizeof(value);
}

size_t ConfigStore::putInt16(const char* key, int16_t value) { 
  _mapInt16[key] = value;
  _needSave = true;
  return sizeof(value);
}
 
size_t ConfigStore::putUInt16(const char* key, uint16_t value) { 
  _mapUInt16[key] = value;
  _needSave = true;
  return sizeof(value);
}

size_t ConfigStore::putInt32(const char* key, int32_t value) { 
  _mapInt32[key] = value;
  _needSave = true;
  return sizeof(value);
}

size_t ConfigStore::putUInt32(const char* key, uint32_t value) { 
  _mapUInt32[key] = value;
  _needSave = true;
  return sizeof(value);
}

size_t ConfigStore::putFloat(const char* key, float value) { 
  _mapFloat[key] = value;
  _needSave = true;
  return sizeof(value);
}

size_t ConfigStore::putDouble(const char* key, double value) { 
  _mapDouble[key] = value;
  _needSave = true;
  return sizeof(value);
}

size_t ConfigStore::putString(const char* key, String value) { 
  _mapString[key] = value;
  _needSave = true;
  return sizeof(value);
}



/*----------------------------------------------------------------------------
------------------------------------------------------------------------------
Getters
------------------------------------------------------------------------------
----------------------------------------------------------------------------*/
bool ConfigStore::getBool(const char* key, bool defaultValue) {
  if (_mapBool.count( key) ) {
    return _mapBool[key];
  } else {
    putBool(key, defaultValue);
    return defaultValue;
  }
}

int16_t ConfigStore::getInt16(const char* key, int16_t defaultValue) {
  if (_mapInt16.count( key) ) {
    return _mapInt16[key];
  } else {
    putInt16(key, defaultValue);
    return defaultValue;
  }
}

uint16_t ConfigStore::getUInt16(const char* key, uint16_t defaultValue) {
  if (_mapUInt16.count( key) ) {
    return _mapUInt16[key];
  } else {
    putUInt16(key, defaultValue);
    return defaultValue;
  }
}

int32_t ConfigStore::getInt32(const char* key, int32_t defaultValue) {
  if (_mapInt32.count( key) ) {
    return _mapInt32[key];
  } else {
    putInt32(key, defaultValue);
    return defaultValue;
  }
}

uint32_t ConfigStore::getUInt32(const char* key, uint32_t defaultValue) {
  if (_mapUInt32.count( key) ) {
    return _mapUInt32[key];
  } else {
    putUInt32(key, defaultValue);
    return defaultValue;
  }
}

float ConfigStore::getFloat(const char* key, float defaultValue) {
  if (_mapFloat.count( key) ) {
    return _mapFloat[key];
  } else {
    putFloat(key, defaultValue);
    return defaultValue;
  }
}

double ConfigStore::getDouble(const char* key, double defaultValue) {
  if (_mapDouble.count( key) ) {
    return _mapDouble[key];
  } else {
    putDouble(key, defaultValue);
    return defaultValue;
  }
}

String ConfigStore::getString(const char* key, String defaultValue) {
  if (_mapString.count(key) ) {
    return _mapString[key];
  } else {
    putString(key, defaultValue);
    return defaultValue;
  }
}
