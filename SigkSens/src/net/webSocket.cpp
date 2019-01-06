#ifdef ESP8266
  #include <ESP8266mDNS.h>        // Include the mDNS library
#elif defined(ESP32)
  #include <ESPmDNS.h>
#endif

#include "../../config.h"
#include "../../sigksens.h"
#include "../services/configStore.h"
#include "../services/ledBlinker.h"
#include "../services/signalK.h"

#include "webSocket.h"


#ifdef ENABLE_WEBSOCKET_SERVER
WebSocketsServer webSocketServer = WebSocketsServer(81);
#endif

SignalKClientInfo signalKClientInfo = { 
  .connected = false
};

// forward declarations

void webSocketServerEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length);
void interruptWsReconnect(void *pArg);
void connectWebSocketClient();
void webSocketClientEvent(WStype_t type, uint8_t * payload, size_t length);
bool getWebsocketClientStatus();

void setupWebSocket() {
  #ifdef ENABLE_WEBSOCKET_SERVER
  webSocketServer.begin();
  webSocketServer.onEvent(webSocketServerEvent);
  #endif
  signalKClientInfo.client.onEvent(webSocketClientEvent);

  //sets defaults if not set already :)
  configStore.getString("signalKServerHost", "");
  configStore.getUInt16("signalKServerPort", 80);
  configStore.getString("signalKURLPath", "/signalk/v1/stream");
  configStore.getString("accessToken","");

  connectWebSocketClient();
  app.onRepeat(20, handleWebSocket); // calling websocket loop every 20ms instead of every tick doubles systemHz :)

}


void handleWebSocket() {
  #ifdef ENABLE_WEBSOCKET_SERVER
  webSocketServer.loop();
  #endif
  
  if (signalKClientInfo.connected) {
    signalKClientInfo.client.loop();    
  }
}


void restartWebSocketClient() {
  if (signalKClientInfo.connected) {
    signalKClientInfo.client.disconnect(); // disconnect will set timer to reconnect. if not connected it was already running
  }
}

bool getWebsocketClientStatus() {
  SignalKClientInfo *skci = &signalKClientInfo;  // save some typing
  return skci->connected;
}

bool getMDNSService() {
  // get IP address using an mDNS query
  int n = MDNS.queryService("signalk-ws", "tcp");
  if (n==0) {
    // no service found
    return false;
  } else {
    configStore.putString("signalKServerHost" ,MDNS.IP(0).toString());
    configStore.putUInt16("signalKServerPort", MDNS.port(0));
    Serial.print(F("Found server with IP/Port: "));
    Serial.print(MDNS.IP(0).toString()); Serial.print(":"); Serial.println(MDNS.port(0));
    return true;
  }
}

void connectWebSocketClient() {
  SignalKClientInfo *skci = &signalKClientInfo;  // save some typing
  String urlArgs = "?subscribe=none";

  if (configStore.getString("signalKServerHost").length() == 0) {
    getMDNSService();
  } 
  if ( (configStore.getString("signalKServerHost").length() > 0) && 
       (configStore.getUInt16("signalKServerPort") > 0) && 
       (configStore.getString("signalKURLPath").length() > 0) ) {
    Serial.println(F("Websocket client starting!"));
  } else {
      app.onDelay(10000, connectWebSocketClient); //retry in 10 sec
      return;
  }

  if (configStore.getString("accessToken").length() > 0) {
    urlArgs = urlArgs + "&token=" + configStore.getString("accessToken");
  }

  skci->client.begin(configStore.getString("signalKServerHost"), configStore.getUInt16("signalKServerPort"), configStore.getString("signalKURLPath") + urlArgs);
  skci->connected = true;
}


void webSocketClientEvent(WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
    case WStype_DISCONNECTED:
      signalKClientInfo.connected = false;
      app.onDelay(10000, connectWebSocketClient);
      Serial.printf("[WSc] Disconnected!\n");
      ledBlinker.setServerDisconnected();
      break;
    case WStype_ERROR:
      signalKClientInfo.connected = false;
      app.onDelay(10000, connectWebSocketClient);
      Serial.printf("[WSc] Error!\n");
      ledBlinker.setServerDisconnected();
      break;      
    case WStype_CONNECTED:
      signalKClientInfo.connected = true;
      Serial.printf("[WSc] Connected to url: %s\n", payload);
      ledBlinker.setServerConnected();
      if (configStore.getString("accessToken") == "") {
        signalK.requestAuth();
      }
      // send message to server when Connected
      // webSocket.sendTXT("Connected");
      break;
    case WStype_TEXT:
      //Serial.printf("[WSc] get text: %s\n", payload);
      signalK.receiveDelta(payload);
      // send message to server
      // webSocket.sendTXT("message here");
      break;
    case WStype_BIN:
      Serial.printf("[WSc] get binary length: %u\n", length);
      //hexdump(payload, length);

      // send data to server
      // webSocket.sendBIN(payload, length);
      break;
    case WStype_FRAGMENT_TEXT_START:
      break;
    case WStype_FRAGMENT_BIN_START:
      break;
    case WStype_FRAGMENT:
      break;
    case WStype_FRAGMENT_FIN:
      break;
    


  }

}

#ifdef ENABLE_WEBSOCKET_SERVER
void webSocketServerEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
    switch(type) {
        case WStype_DISCONNECTED:
            Serial.printf("[%u] Disconnected!\n", num);
            break;
        case WStype_ERROR:
          Serial.printf("[%u] Unknown Error!\n", num);
          break;      
        case WStype_CONNECTED:
            {
                IPAddress ip = webSocketServer.remoteIP(num);
                Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
        
                // send message to client
                //webSocketServer.sendTXT(num, "Connected");
            }
            break;
        case WStype_TEXT:
            //Serial.printf("[%u] get Text: %s\n", num, payload);

            // send message to client
            // webSocket.sendTXT(num, "message here");

            // send data to all connected clients
            // webSocket.broadcastTXT("message here");
            break;
        case WStype_BIN:
            //Serial.printf("[%u] get binary length: %u\n", num, length);
            //hexdump(payload, length);

            // send message to client
            // webSocket.sendBIN(num, payload, length);
            break;

        case WStype_FRAGMENT_TEXT_START:
          break;
        case WStype_FRAGMENT_BIN_START:
          break;
        case WStype_FRAGMENT:
          break;
        case WStype_FRAGMENT_FIN:
          break;
    }

}
#endif
