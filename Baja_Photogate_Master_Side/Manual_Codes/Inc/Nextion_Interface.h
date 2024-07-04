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
void Wait_Nextion_Resp_us(uint32_t tim_to_wait_us);
void Nextion_Update_Battery(uint8_t Bat_Status_perc, uint16_t Batt_Voltage_mV);
void Clear_UART_RxTx();
void Nextion_Debug_Stopwatch_Send();
void Nextion_SD_Write_Confirmation_Page();
uint8_t Nextion_Get_Start_Download_Request();

uint8_t Get_Nextion_Request_Page();

BEACONMODE_TypeDef Get_Device_Mode();
uint8_t Nextion_Get_Car_Num();

typedef enum{

	STANDBY_PAGE = 0x02,
	CAR_SEL_PAGE = 0x04,
	INFO_PAGE = 0x03,
	RACE_PAGE = 0x01,
	LOCAL_REPORT_PAGE = 0X06,
	OTA_REPORT_PAGE = 0X08,
	MEM_ERASE = 0X09,

}Nextion_Pages_TypeDef;

typedef enum{

	ERR_NOT_REQUESTED = 0X00,
	ERR_REQUESTED = 0X01,

}Mem_Erase_Status;





#endif /* NEXTION_INTERFACE_H */
