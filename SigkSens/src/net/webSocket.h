#ifndef _webSocket_H_
#define _webSocket_H_

#include "config.h"

#ifdef ENABLE_WEBSOCKET_SERVER
  #include <WebSocketsServer.h>
#endif
#include <WebSocketsClient.h>

class SignalKClientInfo {
  public:
    bool connected;
    WebSocketsClient client;
};

extern SignalKClientInfo signalKClientInfo;

#ifdef ENABLE_WEBSOCKET_SERVER
extern WebSocketsServer webSocketServer;
#endif

void setupWebSocket();
void handleWebSocket();
bool getWebsocketClientStatus();
String getWebsocketClientActiveHost();
uint16_t getWebsocketClientActivePort();
bool getMDNSService();
void restartWebSocketClient();

#endif
