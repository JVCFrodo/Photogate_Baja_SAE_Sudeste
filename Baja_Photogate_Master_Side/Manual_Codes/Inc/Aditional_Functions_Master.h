#ifndef ADITIONAL_FUNTIONS_MASTER_H
#define ADITIONAL_FUNTIONS_MASTER_H


#include "main.h"
#include "definitions.h"
#include "shared_definitions.h"
#include "fatfs.h"
#include "data_types.h"
#include "nextion_interface.h"
#include "nrf24.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

uint16_t Calc_Batt_Perc(uint8_t mode);
void Initialize_Batt_Avg_Calc();
void Erase_SD_Card();
void OTA_Log_Routine(Data_FS Data);
void Store_Data_SD_30m(uint16_t Time_ms);
void Store_Data_SD_All(uint16_t Time_ms, float Speed);
void Reset_Counters_And_Values();

void Regular_Nextion_Updates();
void Check_Apply_OPmode_Change();
void NRF_24_Master_Init();

extern void Race_Mode_StateMachine_Update(BEACONMESSAGE_TypeDef *Message);
extern uint32_t Start_Time_Full_ms, End_Time_Full_ms, Measured_Time_Full_ms;
extern uint16_t Meastime_ms, Start_Time_Speed_Calc_ms, End_Time_Speed_Calc_ms, Measured_Time_Speed_Calc_ms,
		          Delta_msgs, Message_Milis_Calc;
extern uint8_t Meastime_secs, Finished_30m_Flag, Finished_Speed_Flag;
extern float Vehicle_Speed;

extern void On_Valid_NRF24_Msg();


static uint16_t V_Bat_mv_table[] = {2210, 2610, 2820, 2940, 3040, 3120, 3190, 3270, 3320, 3370, 3410, 3430, 3440, 3460, 3480, 3500, 3520, 3530, 3550, 3570, 3590, 3600, 3610, 3630, 3650, 3670, 3690, 3700, 3730, 3750, 3770, 3780, 3800, 3820, 3840, 3850, 3870, 3900, 3920, 3940, 3960, 3990, 4010, 4010, 4019, 4030, 4040, 4050, 4070, 4100};


#endif /* ADITIONAL_FUNTIONS_MASTER_H */

