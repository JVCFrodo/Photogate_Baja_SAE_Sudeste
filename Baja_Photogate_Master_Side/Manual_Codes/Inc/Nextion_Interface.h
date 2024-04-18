#ifndef NEXTION_INTERFACE_H_
#define NEXTION_INTERFACE_H_

#include "main.h"
#include "shared_definitions.h"
#include <stdio.h>

#define Nextion_EndChar 255



void Update_Sensor_Status_Stdby(uint8_t sensor, uint8_t Sync, uint8_t Beam, uint8_t Signal);
void Nextion_Init();
void Update_Sensor_Status_Run(uint8_t Sensor, uint8_t Sync);
void Display_30m_time(uint16_t milis, uint8_t secs);
void Wait_Nextion_Resp_us(uint16_t tim_to_wait_us);

BEACONMODE_TypeDef Get_Device_Mode();
uint8_t Get_Car_Num();

typedef enum{

	STANDBY_PAGE = 0x02,
	CAR_SEL_PAGE = 0x04,
	INFO_PAGE = 0x03,
	RACE_PAGE = 0x01,

}Nextion_Pages_TypeDef;

#endif /* NEXTION_INTERFACE_H */
