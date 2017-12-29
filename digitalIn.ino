

//Timers
os_timer_t  digitalInTimer; // timer to scan for new devices
bool readyToUpdateDigitalIn = false;


int d1Value = 0;
int d2Value = 0;

void setupDigitalIn() {

  pinMode(D1_PIN, INPUT); 
  pinMode(D2_PIN, INPUT); 

  os_timer_setfn(&digitalInTimer, interuptUpdateDigitalIn, NULL);
  os_timer_arm(&digitalInTimer, updateDigitalInDelay, true);

  
}


void handleDigitalIn() {



  
}

void interuptUpdateDigitalIn(void *pArg) {
  readyToUpdateDigitalIn = true;
}

void setDigitalInUpdateDelay(uint32_t newDelay) {
  os_timer_disarm(&digitalInTimer);
  Serial.print("Restarting DigitalIn polling timer at: ");
  Serial.print(newDelay);  
  Serial.println("ms");
  updateDigitalInDelay = newDelay;
  os_timer_arm(&digitalInTimer, updateDigitalInDelay, true);
}




