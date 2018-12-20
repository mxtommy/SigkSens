#ifndef _windlassCtrl_H_
#define _windlassCtrl_H_

#include "../../../sigksens.h"
#include "../sensorStorage.h"

enum WindlassStates {
  idle,
  channel1Active,
  channel2Active,
  idleWait
};

void setupWindlassCtrl(bool&);



#endif