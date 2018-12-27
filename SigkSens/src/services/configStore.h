#ifndef _configStore_H_
#define _configStore_H_
#include "Arduino.h"
#include <ArduinoJson.h>
#include <map>

void setupFS();

class ConfigStore {
    protected:
        bool _started;
        const char * _configFileName;
        bool _needSave;
        std::map<String, String> _mapString;
        std::map<String, float> _mapFloat;
        std::map<String, float> _mapDouble;
        std::map<String, bool> _mapBool;

        std::map<String, int8_t> _mapInt8;
        std::map<String, uint8_t> _mapUInt8;
        std::map<String, int16_t> _mapInt16;
        std::map<String, uint16_t> _mapUInt16;
        std::map<String, int32_t> _mapInt32;
        std::map<String, uint32_t> _mapUInt32;

    public:
        ConfigStore();
        ~ConfigStore();

        bool begin(const char * name);
        void handle();
        void saveConfig();
        void end();

        bool clear();
        bool remove(const char * key);

        size_t putInt8(const char* key, int8_t value);
        size_t putUInt8(const char* key, uint8_t value);
        size_t putInt16(const char* key, int16_t value);
        size_t putUInt16(const char* key, uint16_t value);
        size_t putInt32(const char* key, int32_t value);
        size_t putUInt32(const char* key, uint32_t value);
        size_t putFloat(const char* key, float value);
        size_t putDouble(const char* key, double value);
        size_t putBool(const char* key, bool value);
        size_t putString(const char* key, String value);

        int8_t getInt8(const char* key, int8_t defaultValue = 0);
        uint8_t getUInt8(const char* key, uint8_t defaultValue = 0);
        int16_t getInt16(const char* key, int16_t defaultValue = 0);
        uint16_t getUInt16(const char* key, uint16_t defaultValue = 0);
        int32_t getInt32(const char* key, int32_t defaultValue = 0);
        uint32_t getUInt32(const char* key, uint32_t defaultValue = 0);
        float getFloat(const char* key, float defaultValue = NAN);
        double getDouble(const char* key, double defaultValue = NAN);
        bool getBool(const char* key, bool defaultValue = false);
        //size_t getString(const char* key, char* value, size_t maxLen);
        String getString(const char* key, String defaultValue = String());
        //size_t getBytes(const char* key, void * buf, size_t maxLen);
        //size_t freeEntries();
};


extern ConfigStore configStore;



#endif
