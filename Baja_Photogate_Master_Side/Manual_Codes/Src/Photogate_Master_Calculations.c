/*
 * Photogate_Master_Calculations.c
 *
 *  Created on: Mar 18, 2024
 *      Author: CAJ1SBC
 */

#include "main.h"
#include "definitions.h"
#include "shared_definitions.h"
#include "usbd_cdc_if.h"
#include "nextion_interface.h"

extern TIM_HandleTypeDef htim9;

extern volatile BEACONMODE_TypeDef Device_Current_Mode;
extern volatile BEACONMODE_TypeDef Device_Previous_Mode;

extern volatile uint16_t StopWatch_Counter_ms, Meastime_us;
extern volatile uint8_t StopWatch_Counter_secs, Stopwatch_Counter_Mins;
extern volatile uint8_t Meastime_secs, Meastime_Mins;
extern volatile uint8_t Generic_Tim_Count_200ms;

extern char msg[80];
extern volatile uint8_t msg_len;
extern BEACONID_TypeDef Return_Mode;



volatile uint8_t WDTf_Beacon1 = 0x00, WDTf_Beacon2 = 0x00, WDTf_Beacon3= 0x00, WDTf_Beacon4 = 0x00;
volatile uint16_t Msg_Count_Act_B1 = 0x00, Msg_Count_Act_B2 = 0x00, Msg_Count_Act_B3 = 0x00, Msg_Count_Act_B4 = 0x00;
volatile uint16_t Msg_Count_Delta_B1 = 0x00, Msg_Count_Delta_B2 = 0x00, Msg_Count_Delta_B3 = 0x00, Msg_Count_Delta_B4 = 0x00;
volatile uint16_t Timestamp_B1_Lastmsg = 0x00; Timestamp_B2_Lastmsg = 0x00, Timsetamp_B3_Lastmsg = 0x00, Timestamp_B4_Lastmsg = 0x00;


void Beacon_Sync_Calcs(BEACONMESSAGE_TypeDef* Message);
void WDT_Calcs_Updates(Nextion_Pages_TypeDef Page);

void Beacon_Sync_Calcs(BEACONMESSAGE_TypeDef* Message){

	uint16_t Calc_message_Dt_ms = 0x00, Meas_message_Dt_ms = 0x00;
	int16_t Unsync_Time_B1 = 0x00;
	uint8_t USB_Stats = 0x00;

	switch(Message->Beacon_Id){

	case BEACON1:


		WDTf_Beacon1 = 0x00;

		Msg_Count_Delta_B1 = (Message->Msg_Counter_H << 8) + Message->Msg_Counter_L;
		if((Msg_Count_Delta_B1 - Msg_Count_Act_B1) > 0) Msg_Count_Delta_B1 -= Msg_Count_Act_B1;
		else Msg_Count_Delta_B1 = (65536 - Msg_Count_Act_B1) + Msg_Count_Delta_B1;

		Calc_message_Dt_ms = (Msg_Count_Delta_B1 * 200) + (Message->Time_MilisH << 8 + Message->Time_MilisL);
		Meas_message_Dt_ms = ((StopWatch_Counter_secs * 1000) + StopWatch_Counter_ms) - Timestamp_B1_Lastmsg;

		Msg_Count_Act_B1 = (Message->Msg_Counter_H << 8) + Message->Msg_Counter_L;
		Timestamp_B1_Lastmsg = (StopWatch_Counter_secs * 1000) + StopWatch_Counter_ms;

		Unsync_Time_B1 = Meas_message_Dt_ms - Calc_message_Dt_ms;


		//msg_len = sprintf(msg, "Msg_B1: %d | Plus %d ms | TS_B1: %d ms | Unsync_B1: %d ms\r\n", Msg_Count_Delta_B1, (Message->Time_MilisH << 8 + Message->Time_MilisL), Timestamp_B1_Lastmsg, Unsync_Time_B1);
		//CDC_Transmit_FS(msg, msg_len);


		break;

	case BEACON2:

		WDTf_Beacon2 = 0x00;

		Msg_Count_Delta_B2 = (Message->Msg_Counter_H << 8) + Message->Msg_Counter_L;
		if((Msg_Count_Delta_B2 - Msg_Count_Act_B2) > 0) Msg_Count_Delta_B2 -= Msg_Count_Act_B2;
		else Msg_Count_Delta_B2 = (65536 - Msg_Count_Act_B2) + Msg_Count_Delta_B2;



		break;

	case BEACON3:

		WDTf_Beacon3 = 0x00;

		Msg_Count_Delta_B3 = (Message->Msg_Counter_H << 8) + Message->Msg_Counter_L;
		if((Msg_Count_Delta_B3 - Msg_Count_Act_B3) > 0) Msg_Count_Delta_B3 -= Msg_Count_Act_B3;
		else Msg_Count_Delta_B3 = (65536 - Msg_Count_Act_B3) + Msg_Count_Delta_B3;

		Msg_Count_Act_B3 = (Message->Msg_Counter_H << 8) + Message->Msg_Counter_L;

		break;

	case BEACON4:

		WDTf_Beacon4 = 0x00;

		Msg_Count_Delta_B4 = (Message->Msg_Counter_H << 8) + Message->Msg_Counter_L;
		if((Msg_Count_Delta_B4 - Msg_Count_Act_B4) > 0) Msg_Count_Delta_B4 -= Msg_Count_Act_B4;
		else Msg_Count_Delta_B4 = (65536 - Msg_Count_Act_B4) + Msg_Count_Delta_B4;

		Msg_Count_Act_B4 = (Message->Msg_Counter_H << 8) + Message->Msg_Counter_L;

		break;

	default:
		break;

}
}

void WDT_Calcs_Updates(Nextion_Pages_TypeDef Page){


	if(Device_Current_Mode == STANDBY_MODE && Page == STANDBY_PAGE){
		if(WDTf_Beacon1 >= 20) Update_Sensor_Status_Stdby(BEACON1, 0, INTERRUPTED, 255);
		//else if(WDTf_Beacon2 >= 10) Update_Sensor_Status_Stdby(BEACON2, 0, INTERRUPTED, 0);
		//else if(WDTf_Beacon3 >= 10) Update_Sensor_Status_Stdby(BEACON3, 0, INTERRUPTED, 0);
		//else if(WDTf_Beacon4 >= 10) Update_Sensor_Status_Stdby(BEACON4, 0, INTERRUPTED, 0);

		}

	else if(Device_Current_Mode == RACE_MODE && Page == RACE_PAGE)
	{
		if(WDTf_Beacon1 >= 20){

		Update_Sensor_Status_Run(BEACON1, 0);

		}

		if(WDTf_Beacon2 >= 10) Update_Sensor_Status_Run(BEACON2, 0);

		if(WDTf_Beacon3 >= 10) Update_Sensor_Status_Run(BEACON3, 0);

		if(WDTf_Beacon4 >= 10) Update_Sensor_Status_Run(BEACON4, 0);

	}




}

void Check_Apply_OPmode_Change()
{


	Device_Previous_Mode = Device_Current_Mode;
	Device_Current_Mode = Get_Device_Mode();

	if(Device_Previous_Mode != Device_Current_Mode){

		switch(Device_Current_Mode){

			case STANDBY_MODE:

				HAL_TIM_Base_Stop_IT(&htim9);
				break;

		case RACE_MODE:

			StopWatch_Counter_secs = 0x00;
			Stopwatch_Counter_Mins = 0x00;
			HAL_TIM_Base_Start_IT(&htim9);

			break;

		default:
			break;
		}
	}
}
