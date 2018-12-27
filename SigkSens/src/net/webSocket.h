#ifndef _webSocket_H_
#define _webSocket_H_

#ifdef ENABLE_WEBSOCKET_SERVER
  #include <WebSocketsServer.h>
#endif
#include <WebSocketsClient.h>

class SignalKClientInfo {
  public:
    String configuredHost;
    uint16_t configuredPort;
    String activeHost;
    uint16_t activePort;
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
String getWebsocketClientActiveHost();
uint16_t getWebsocketClientActivePort();
void restartWebSocketClient();

#endif
