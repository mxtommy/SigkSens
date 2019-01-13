
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
  _configFileName = String(name);
  if (!SPIFFS.exists(name)) {
    _started = true;
    return true; // file will be created on first save
  } else {
    //file exists, reading and loading
    Serial.print(F("opening config file: ")); Serial.println(name);
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

      for (auto kvp : root) {
        JsonObject& kv = root[kvp.key];
        if (kv["dataType"] == "boolean") {
          _mapBool[kvp.key] = kv["value"];
        } else if (kv["dataType"] == "string") {
          String temp = kv["value"];
          _mapString[kvp.key] = temp;
        } else if (kv["dataType"] == "float") {
          _mapFloat[kvp.key] = kv["value"];
        } else if (kv["dataType"] == "double") {
          _mapDouble[kvp.key] = kv["value"];
        } else if (kv["dataType"] == "int8") {
          _mapInt8[kvp.key] = kv["value"];
        } else if (kv["dataType"] == "uint8") {
          _mapUInt8[kvp.key] = kv["value"];
        } else if (kv["dataType"] == "int16") {
          _mapInt16[kvp.key] = kv["value"];
        } else if (kv["dataType"] == "uint16") {
          _mapUInt16[kvp.key] = kv["value"];
        } else if (kv["dataType"] == "int32") {
          _mapInt32[kvp.key] = kv["value"];
        } else if (kv["dataType"] == "uint32") {
          _mapUInt32[kvp.key] = kv["value"];
        }
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
  for (const auto& kv : _mapBool) {
    JsonObject& tmp = root.createNestedObject(kv.first);
    tmp["value"] = kv.second;
    tmp["dataType"] = "boolean";
  }
  //String
  for (const auto& kv : _mapString) {
    JsonObject& tmp = root.createNestedObject(kv.first);
    tmp["value"] = kv.second;
    tmp["dataType"] = "string";
  }
  //Float
  for (const auto& kv : _mapFloat) {
    JsonObject& tmp = root.createNestedObject(kv.first);
    tmp["value"] = kv.second;
    tmp["dataType"] = "float";
  }
  //Double
  for (const auto& kv : _mapDouble) {
    JsonObject& tmp = root.createNestedObject(kv.first);
    tmp["value"] = kv.second;
    tmp["dataType"] = "double";
  }
  //Int8
  for (const auto& kv : _mapInt8) {
    JsonObject& tmp = root.createNestedObject(kv.first);
    tmp["value"] = kv.second;
    tmp["dataType"] = "int8";
  }
 //UInt8
  for (const auto& kv : _mapUInt8) {
    JsonObject& tmp = root.createNestedObject(kv.first);
    tmp["value"] = kv.second;
    tmp["dataType"] = "uint8";
  }  
  //Int16
  for (const auto& kv : _mapInt16) {
    JsonObject& tmp = root.createNestedObject(kv.first);
    tmp["value"] = kv.second;
    tmp["dataType"] = "int16";
  }
 //UInt16
  for (const auto& kv : _mapUInt16) {
    JsonObject& tmp = root.createNestedObject(kv.first);
    tmp["value"] = kv.second;
    tmp["dataType"] = "uint16";
  }
  //Int32
  for (const auto& kv : _mapInt16) {
    JsonObject& tmp = root.createNestedObject(kv.first);
    tmp["value"] = kv.second;
    tmp["dataType"] = "int32";
  }
 //UInt32
  for (const auto& kv : _mapUInt32) {
    JsonObject& tmp = root.createNestedObject(kv.first);
    tmp["value"] = kv.second;
    tmp["dataType"] = "uint32";
  }

  File configFile = SPIFFS.open(_configFileName, "w");
  if (!configFile) {
    Serial.println(F("failed to open config file for writing"));
  }
  root.printTo(configFile);
  configFile.close();
  root.prettyPrintTo(Serial);
  Serial.print(F("Configuration saved to SPIFFS file: "));
  Serial.println(_configFileName);
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
