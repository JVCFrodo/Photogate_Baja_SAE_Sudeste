/*
 * Addition_Functions.c
 *
 *  Created on: Mar 19, 2024
 *      Author: CAJ1SBC
 */


#include "main.h"
#include "definitions.h"
#include "shared_definitions.h"
//#include "usbd_cdc_if.h"

void Report_Sensor_Counter_Value_USB(uint32_t val);

extern uint8_t msg[50];
extern BEACONMODE_TypeDef Device_Current_Mode;
extern volatile SENSORSTATS_TypeDef Sensor_Status_Act, Sensor_Status_Prev;
/*

void Report_Over_USB()
{

	uint8_t msglen = 0x00;
	if(Device_Current_Mode == STANDBY_MODE) msglen = sprintf(msg,"Current Mode: STANDBY | Sensor Stats: %d  \r\n", Sensor_Status_Act);
	else if(Device_Current_Mode == RACE_MODE) msglen = sprintf(msg,"Current Mode: RACE | Sensor Stats: %d  \r\n", Sensor_Status_Act);

	CDC_Transmit_FS(msg, msglen);

}
*/
