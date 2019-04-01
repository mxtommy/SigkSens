#ifndef _quaternionFilters_H_
#define _quaternionFilters_H_

extern float q[4];    // vector to hold quaternion
extern float eInt[3];       // vector to hold integral error for Mahony method

extern float deltat;

void MahonyQuaternionUpdate(float ax, float ay, float az, float gx, float gy, float gz, float mx, float my, float mz);

#endif
