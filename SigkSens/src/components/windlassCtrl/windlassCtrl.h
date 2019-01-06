#ifndef _windlassCtrl_H_
#define _windlassCtrl_H_

enum WindlassStates {
  idle,
  channel1Active,
  channel2Active,
  idleWait
};

void setupWindlassCtrl();
void handleWindlassCtrl();


#endif