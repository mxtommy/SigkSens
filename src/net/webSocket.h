#ifndef _webSocket_H_
#define _webSocket_H_

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
bool getWebsocketClientStatus();
void restartWebSocketClient();

#endif
