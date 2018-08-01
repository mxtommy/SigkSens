#ifndef _webSocket_H_
#define _webSocket_H_

extern "C" {
#include "user_interface.h"
}
#include "../../config.h"
#include "../../signalK.h"
#include "../../sigksens.h"

#include <ESP8266mDNS.h>        // Include the mDNS library
#ifdef ENABLE_WEBSOCKET_SERVER
  #include <WebSocketsServer.h>
#endif
#include <WebSocketsClient.h>







class SignalKClientInfo {
  public:
    String host;
    uint16_t port;
    String path;
    String authToken;
    bool connected;
    WebSocketsClient client;
};

extern SignalKClientInfo signalKClientInfo;

#ifdef ENABLE_WEBSOCKET_SERVER
extern WebSocketsServer webSocketServer;
#endif

void setupWebSocket();
void handleWebSocket();

void restartWebSocketClient();

#endif
