#ifndef _sht30_H_
#define _sht30_H_

#include "../../../sigksens.h"
#include "../sensorStorage.h"



class SHT30SensorInfo : public SensorInfo {
  public:
    SHT30SensorInfo(String addr);
    SHT30SensorInfo( String addr, 
                    String path1, float offset1, float scale1,
                    String path2, float offset2, float scale2) ;

    static SHT30SensorInfo *fromJson(JsonObject &jsonSens);
    void toJson(JsonObject &jsonSens);
};


void setupSHT30();
void handleSHT30(bool&);


#endif
