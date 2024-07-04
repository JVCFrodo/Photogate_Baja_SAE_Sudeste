/*
 * Addition_Functions.c
 *
 *  Created on: Mar 19, 2024
 *      Author: CAJ1SBC
 */


#include "main.h"
#include "definitions.h"
#include "shared_definitions.h"


uint8_t Calc_Batt_Perc();
void Initialize_Batt_Avg_Calc();


extern uint16_t Analog_read;
uint16_t Filter_Array[100];



uint8_t Calc_Batt_Perc(){


static uint16_t V_Bat_mv[] = {2210, 2610, 2820, 2940, 3040, 3120, 3190, 3270, 3320, 3370, 3410, 3430, 3440, 3460, 3480, 3500, 3520, 3530, 3550, 3570, 3590, 3600, 3610, 3630, 3650, 3670, 3690, 3700, 3730, 3750, 3770, 3780, 3800, 3820, 3840, 3850, 3870, 3900, 3920, 3940, 3960, 3990, 4010, 4010, 4019, 4030, 4040, 4050, 4070, 4100};

static float Conv_Factor_mv = 0.8119658, Res_Div_Ratio = 2; //Conv factor regards the ADC conversion bits-->mV (3.33/4095)*1000. ResRatio regards the hardware.
uint8_t Filtering_Window_Size = 20;

uint32_t Avg_Aux = 0x00;
uint16_t Analog_Read_Current = 0x00, Voltage_mv = 0x00, Analog_Read_Avg = 0x00, Diff = 0x00, Min_Diff = 0xFFFFFF;
uint8_t i = 0x00, Table_Size = 50, Index = 0x00;

	Analog_Read_Current = Analog_read;

	for(i = 0x00; i <= (Filtering_Window_Size-2); i++)
		{
		Filter_Array[i] = Filter_Array[i+1];
		}
	Filter_Array[(Filtering_Window_Size-1)] = Analog_Read_Current;
	for(i = 0x00; i < Filtering_Window_Size; i++) Avg_Aux += Filter_Array[i];

	Analog_Read_Avg = Avg_Aux/Filtering_Window_Size;
	Voltage_mv = (Analog_Read_Avg*Conv_Factor_mv*Res_Div_Ratio);

	for(uint8_t i = 0; i <= Table_Size; i++) {
		Diff = abs(Voltage_mv - V_Bat_mv[i]);
        if (Diff < Min_Diff) {
            Min_Diff = Diff;
            Index = i;
        }
    }

return (Index*2); //Each position of the table represents 2% of battery

}

void Initialize_Batt_Avg_Calc(){

	uint8_t i = 0x00;

	for(i = 0x00; i <= 10; i++)Filter_Array[i] = 0x00;

}




