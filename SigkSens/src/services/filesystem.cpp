
#include <FS.h> //this needs to be first, or it all crashes and burns...
#ifdef ESP32
#include "SPIFFS.h"
#endif

#include "filesystem.h"


void setupFilesystem() {
  bool mountOk = false;
  #ifdef ESP8266
  mountOk = SPIFFS.begin();
  #elif defined ESP32
  mountOk = SPIFFS.begin(true); //true = format spiffs if it's not already done :)
  #endif
  if (mountOk) {
    Serial.println(F("mounted file system"));
    #ifdef ESP8266
    Serial.println(F("FS Contents:"));
    String str = "";
    Dir dir = SPIFFS.openDir("/");
    while (dir.next()) {
      str += dir.fileName();
      str += " / ";
      str += dir.fileSize();
      str += "\r\n";
    }
    Serial.print(str); 
    #endif
  } else {
    Serial.println(F("failed to mount filesystem, It is possible that the SPIFF setting in your uploader is set to 'No SPIFFS'."));
    delay(64000);

    #ifdef ESP8266
      ESP.reset();
    #elif defined(ESP32)
      ESP.restart();
    #endif
        
  }

}