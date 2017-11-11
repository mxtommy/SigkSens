
bool currentButtonState = false;
bool lastButtonState = false;
unsigned long buttonPressedTime = 0;


void setupConfigReset() {
  
}





void handleConfigReset() {
  if (digitalRead(RESET_CONFIG_PIN) == LOW) {
    currentButtonState = true; //pressed
  } else {
    currentButtonState = false; //not pressed
  }

  if (currentButtonState && !lastButtonState) { 
    //just pressed!
    lastButtonState = currentButtonState;
    buttonPressedTime = millis();
  }

  if (!currentButtonState && lastButtonState) {
    lastButtonState = currentButtonState;
    // just released

    if (((millis() - buttonPressedTime) > SHORT_BUTTON_PRESS_MS) && ((millis() - buttonPressedTime) <= LONG_BUTTON_PRESS_MS)) { // short press
      Serial.println("Short Button press!");      
    } else if ((millis() - buttonPressedTime) > LONG_BUTTON_PRESS_MS) {
      Serial.println("Long Button press! Resetting!");
      resetConfig();
    }
  }

}









void resetConfig() {
  WiFiManager wifiManager;
  Serial.println("Resetting Config!");
  wifiManager.resetSettings();
  SPIFFS.remove("/config.json");
  for (uint8_t x=0; x<50; x++) { // need delay as resetting right away 
    digitalWrite(LED_BUILTIN, LOW);  // to cause wifi settings to not erase
    delay(20);                 
    digitalWrite(LED_BUILTIN, HIGH);
    delay(20);
  }

  ESP.reset();
}
