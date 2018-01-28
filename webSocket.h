#ifndef _webSocket_H_
#define _webSocket_H_

#include <WebSocketsServer.h>
#include <WebSocketsClient.h>

#include "config.h"

class SignalKClientInfo {
  public:
    String host;
    uint16_t port;
    String path;
    bool connected;
    WebSocketsClient client;
};

extern SignalKClientInfo signalKClientInfo;

extern WebSocketsServer webSocketServer;

void setupWebSocket();
void handleWebSocket();

void restartWebSocketClient();

#endif
