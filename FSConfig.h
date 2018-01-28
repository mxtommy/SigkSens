#ifndef _FSConfig_H_
#define _FSConfig_H_

#include "config.h"

extern bool shouldSaveConfig;

void setupFS();

void loadConfig();
void saveConfig();
void saveConfigCallback();

#endif
