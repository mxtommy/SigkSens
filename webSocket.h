#ifndef _webSocket_H_
#define _webSocket_H_

#include "config.h"

#ifdef ENABLE_WEBSOCKET_SERVER
#include <WebSocketsServer.h>
#endif
#include <WebSocketsClient.h>

#include "config.h"

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
