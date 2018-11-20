#include <ESP8266mDNS.h>        // Include the mDNS library


#include "../../config.h"
#include "../../sigksens.h"
#include "../services/signalK.h"



#include "webSocket.h"


#ifdef ENABLE_WEBSOCKET_SERVER
WebSocketsServer webSocketServer = WebSocketsServer(81);
#endif

SignalKClientInfo signalKClientInfo = { 
  .configuredHost = "", 
  .configuredPort = 80, 
  .activeHost = "",
  .activePort = 80,
  .path = "/signalk/v1/stream",
  .authToken = "",
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

  connectWebSocketClient();

  //app.onTick(&handleWebSocket);
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

String getWebsocketClientActiveHost() {
  SignalKClientInfo *skci = &signalKClientInfo;  // save some typing
  return skci->activeHost;
}

uint16_t getWebsocketClientActivePort() {
  SignalKClientInfo *skci = &signalKClientInfo;  // save some typing
  return skci->activePort;
}

bool getMDNSService(String &host, uint16_t &port) {
  // get IP address using an mDNS query
  int n = MDNS.queryService("signalk-ws", "tcp");
  if (n==0) {
    // no service found
    return false;
  } else {
    host = MDNS.IP(0).toString();
    port = MDNS.port(0);
    Serial.print(F("Found server with IP/Port: "));
    Serial.print(host); Serial.print(":"); Serial.println(port);
    return true;
  }
}

void connectWebSocketClient() {
  SignalKClientInfo *skci = &signalKClientInfo;  // save some typing
  String urlArgs = "?subscribe=none";

  if (skci->configuredHost.length() == 0) {
    getMDNSService(skci->activeHost, skci->activePort);
  } else {
    skci->activeHost = skci->configuredHost;
    skci->activePort = skci->configuredPort;
  }

  if ( (skci->activeHost.length() > 0) && 
       (skci->activePort > 0) && 
       (skci->path.length() > 0) ) {
    Serial.println(F("Websocket client starting!"));
  } else {
      app.onDelay(10000, connectWebSocketClient);
      return;
  }

  if (skci->authToken != "") {
    urlArgs = urlArgs + "&token=" + skci->authToken;
  }

  skci->client.begin(skci->activeHost, skci->activePort, skci->path + urlArgs);
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
      // send message to server when Connected
      // webSocket.sendTXT("Connected");
      break;
    case WStype_TEXT:
      //Serial.printf("[WSc] get text: %s\n", payload);
      receiveDelta(payload);
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
