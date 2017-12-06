

os_timer_t  wsClientReconnectTimer; // once request cycle starts, this timer set so we can send when ready
bool  readyToReconnectWs = false;


void setupWebSocket() {

  os_timer_setfn(&wsClientReconnectTimer, interuptWsReconnect, NULL);

  
  webSocketServer.begin();
  webSocketServer.onEvent(webSocketServerEvent);
  webSocketClient.onEvent(webSocketClientEvent);

  connectWebSocketClient();
}




void handleWebSocket() {
  webSocketServer.loop();
  if (readyToReconnectWs) {
    readyToReconnectWs = false;
    connectWebSocketClient();
  }
  
  if (websocketConnected) {
    webSocketClient.loop();    
  }
  
}

void restartWebSocketClient() {
  if (websocketConnected) {
    webSocketClient.disconnect(); // disconnect will set timer to reconnect. if not connected it was already running
  }
}


void connectWebSocketClient() {
  if ( (signalKHost.length() > 0) && (signalKPort > 0) && (signalKPort < 65535) && (signalKPath.length() > 0)) {
    Serial.println("Websocket client starting!");
  } else {
      os_timer_arm(&wsClientReconnectTimer, 10000, false);
      return;
  }

  webSocketClient.begin(signalKHost, signalKPort, signalKPath + "?subscribe=none");
  websocketConnected = true;
  
}

void interuptWsReconnect(void *pArg) {
  readyToReconnectWs = true;
}



void webSocketClientEvent(WStype_t type, uint8_t * payload, size_t length) {

  switch(type) {
    case WStype_DISCONNECTED:
      websocketConnected = false;
      os_timer_arm(&wsClientReconnectTimer, 10000, false);
      Serial.printf("[WSc] Disconnected!\n");
      break;
    case WStype_CONNECTED: {
      websocketConnected = true;
      Serial.printf("[WSc] Connected to url: %s\n", payload);
      // send message to server when Connected
      // webSocket.sendTXT("Connected");
    }
      break;
    case WStype_TEXT:
      Serial.printf("[WSc] get text: %s\n", payload);

      // send message to server
      // webSocket.sendTXT("message here");
      break;
    case WStype_BIN:
      Serial.printf("[WSc] get binary length: %u\n", length);
      //hexdump(payload, length);

      // send data to server
      // webSocket.sendBIN(payload, length);
      break;
  }

}


void webSocketServerEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {

    switch(type) {
        case WStype_DISCONNECTED:
            Serial.printf("[%u] Disconnected!\n", num);
            break;
        case WStype_CONNECTED:
            {
                IPAddress ip = webSocketServer.remoteIP(num);
                Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
        
                // send message to client
                webSocketServer.sendTXT(num, "Connected");
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
    }

}

