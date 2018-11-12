#ifndef __AIRPRESS_H
#define __AIRPRESS_H
void AirPressInit(void);
void GetInitPressure(void);
void GetPressure(void);

#define AIR_DOWN_VALU	12 //气压下降值判断有吸力
#endif
