
#include "main.h"
#include "definitions.h"
#include "shared_definitions.h"
#include "usbd_cdc_if.h"
#include "nrf24.h"
#include "nextion_interface.h"
#include "data_types.h"
#include "fatfs.h"



extern volatile BEACONMODE_TypeDef Device_Current_Mode;
extern TIM_HandleTypeDef htim10;
extern TIM_HandleTypeDef htim9;
extern UART_HandleTypeDef huart1;

extern volatile uint8_t WDTf_Beacon1, WDTf_Beacon2, WDTf_Beacon3, WDTf_Beacon4;

extern FRESULT SD_Operation_Result;



volatile uint16_t StopWatch_Counter_ms = 0x00, Meastime_us = 0x00;
volatile uint8_t StopWatch_Counter_secs = 0x00, Stopwatch_Counter_Mins = 0x00;
volatile uint8_t Meastime_secs = 0x00, Meastime_Mins = 0x00;
volatile uint8_t Generic_Tim_Count_200ms = 0x00;
volatile char Payload_Log[32];
volatile uint8_t RX_Payload_Flag = 0x00, Dummy_Payload_Len = 0x00;
volatile uint8_t Cancel_req = 0x00, Start_Req = 0x00, Step = 0x00, Sync = 0x00, Setup_Complete = 0x00;


volatile uint8_t Beacon_Blocker = 0x00; 	//This variable follows the expected beacon order, and only allows recordings to be done whe this given order is fullfilled.

uint8_t regread = 0x00;
volatile uint8_t msg_len = 0x00;
char msg[80];


extern Rx_Buffer[20];


extern void RF_Transmit_Config_MSG(BEACONMODE_TypeDef reqmode);
extern void Beacon_Sync_Calcs(BEACONMESSAGE_TypeDef* Message);
extern void WDT_Calcs_Updates();
extern void NRF_24_Log_Init();



void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)						// Management of Timer interrupts
{


	if(htim->Instance == TIM9)			//Master Stopwatch and Sync timer interrupts
		{
		StopWatch_Counter_ms++;

		if(StopWatch_Counter_ms >= 1000)
		{
			StopWatch_Counter_ms = 0x00;
			StopWatch_Counter_secs++;
			if(StopWatch_Counter_secs >= 60)
			{
				StopWatch_Counter_secs = 0x00;
				Stopwatch_Counter_Mins++;

			}

		}
		}


	else if (htim->Instance == TIM10){		//Happens each 200 msecond.  Separated from Stopwatch timer.


		Generic_Tim_Count_200ms++;


		WDTf_Beacon1++;
		WDTf_Beacon2++;
		WDTf_Beacon3++;
		WDTf_Beacon4++;

		nRF24_FlushRX();
		nRF24_FlushTX();
		nRF24_ClearIRQFlags();
		Calc_Batt_Perc();


		if(Generic_Tim_Count_200ms >= 5){

			Generic_Tim_Count_200ms = 0x00;
			if(Device_Current_Mode == STANDBY_MODE || Device_Current_Mode == RACE_MODE) Regular_Nextion_Updates();

			}


		}
	}




void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){								//This functions was imported from "rf_driver.c". Put here for organization purposes

	BEACONMESSAGE_TypeDef Message;
	BEACONMODE_TypeDef Modo;
	uint8_t Irq_Flags = 0xFF;
	uint8_t pipe = 0xFF;
	Nextion_Pages_TypeDef Nextion_Page = 0xff;

	uint8_t Msg_Len = Payload_Len;

	float Vehicle_Speed = 0.0;
	float dist = BEACON_DISTANCE;

	switch(GPIO_Pin){


	case NRF_IRQ_Pin:

		Irq_Flags = nRF24_GetIRQFlags();
		Nextion_Page = Get_Nextion_Pages();

		if((Irq_Flags & nRF24_FLAG_RX_DR) == nRF24_FLAG_RX_DR){
			if(Device_Current_Mode == STANDBY_MODE || Device_Current_Mode == RACE_MODE){
				pipe = nRF24_ReadPayload(&Message, &Msg_Len);

				if(Message.Beacon_Mode != Device_Current_Mode) RF_Transmit_Config_MSG(Device_Current_Mode);

				if(Nextion_Page == STANDBY_PAGE || Nextion_Page == RACE_PAGE){

					switch(Device_Current_Mode){

						case STANDBY_MODE:


							if(Message.Beacon_Mode != STANDBY_MODE) Update_Sensor_Status_Stdby(Message.Beacon_Id, 0, Message.Sensor_status, Message.Battery_Percentage);
							else Update_Sensor_Status_Stdby(Message.Beacon_Id, 1, Message.Sensor_status, Message.Battery_Percentage);

							Beacon_Sync_Calcs(&Message);

							break;

						case RACE_MODE:

							if(Message.Beacon_Mode != RACE_MODE) {
								Update_Sensor_Status_Run(Message.Beacon_Id, 0); //At Run Page, indicates that the sensor is not on the same status as the master (STANDBY instead of RACE)
								msg_len = sprintf( msg,"Beacon of ID 0x%x is out of sync!! \n", Message.Beacon_Id);
								CDC_Transmit_FS(msg, msg_len);
								RF_Transmit_Config_MSG(Device_Current_Mode); // Sends a configuration message in order to sync the modules

							}

							else if(Message.Beacon_Mode == RACE_MODE) {

								Beacon_Sync_Calcs(&Message);
								Update_Sensor_Status_Run(Message.Beacon_Id, 1); //At Run Page, indicates that the sensor is on the same status as the master (RACE)

									if (Message.Beacon_Id == BEACON1  && Beacon_Blocker == 0x00 && Message.Sensor_status == INTERRUPTED){

										msg_len = sprintf( msg,"Car detected at BEACON 1 - StopWatch Started! \n");
										CDC_Transmit_FS(msg, msg_len);
										Beacon_Blocker = 0x01;
											}


									else if (Message.Beacon_Id == BEACON2  && Beacon_Blocker == 0x01 && Message.Sensor_status == INTERRUPTED){

										HAL_TIM_Base_Stop_IT(&htim9);

										Meastime_secs = StopWatch_Counter_secs;
										Meastime_Mins = Stopwatch_Counter_Mins;


										StopWatch_Counter_secs = 0x00;
										Stopwatch_Counter_Mins = 0x00;

										msg_len = sprintf( msg,'Car detected at BEACON 2 - Measured time: %d:%.2d.%.3d%d \n', Meastime_Mins, Meastime_secs, (Meastime_us/2),((Meastime_us%2)));
										CDC_Transmit_FS(msg, msg_len);

										Beacon_Blocker = 0x02;
										}

									else if (Message.Beacon_Id == BEACON3 && Beacon_Blocker == 0x02){

										HAL_TIM_Base_Start_IT(&htim9);

										msg_len = sprintf( msg,'Car detected at BEACON 3 - Timer reset \n');
										CDC_Transmit_FS(msg, msg_len);

										Beacon_Blocker = 0x03;
										}

									else if (Message.Beacon_Id == BEACON4 && Beacon_Blocker == 0x03){

										HAL_TIM_Base_Stop_IT(&htim9);

										/*
										Vehicle_Speed = (StopWatch_Counter_us/2) + ((StopWatch_Counter_us%2)*0.1);
										Vehicle_Speed = dist/Vehicle_Speed;
										Vehicle_Speed = (Vehicle_Speed*3.6);
										 */
										msg_len = sprintf( msg,'Vehicle Speed was: %.3f km/h\n', Vehicle_Speed);
										CDC_Transmit_FS(msg, msg_len);

										RF_Transmit_Config_MSG(STANDBY_MODE);
										Beacon_Blocker = 0x00;
										}
										break;

					break;
					}


					default:
					break;
				}
				}
		}
			else if(Device_Current_Mode == OTA_REPORT_MODE){
				pipe = nRF24_ReadPayload(Payload_Log, &Dummy_Payload_Len);
				RX_Payload_Flag = 0x01;



			}
			}

		nRF24_ClearIRQFlags();
		nRF24_FlushRX();
}
}

void Regular_Nextion_Updates(){


	Nextion_Pages_TypeDef Nextion_Page = 0xff;
	uint8_t Battery_Status_Perc = 0x00;
	uint16_t Batt_Voltage_mv = 0x00;
    Data_FS SD_Data;


	Nextion_Page = Get_Nextion_Pages();


	switch(Nextion_Page){

	case STANDBY_PAGE:
		Check_Apply_OPmode_Change();
		WDT_Calcs_Updates(Nextion_Page);

		Nextion_Update_SD_Status(SD_Operation_Result);

		Battery_Status_Perc = Calc_Batt_Perc(0);
		Batt_Voltage_mv = Calc_Batt_Perc(1);
		Nextion_Update_Battery(Battery_Status_Perc, Batt_Voltage_mv);

		break;

	case RACE_PAGE:
		Check_Apply_OPmode_Change();
		WDT_Calcs_Updates(Nextion_Page);
		break;



	case LOCAL_REPORT_PAGE:

		SD_Data = Read_SD_Data();
		Nextion_Display_Mem_Data(SD_Data);

		break;

	case OTA_REPORT_PAGE:

		Device_Current_Mode = OTA_REPORT_MODE;
		NRF_24_Log_Init();
		SD_Data = Read_SD_Data();
		OTA_Log_Routine(SD_Data);
		NRF_24_Master_Init();
		Nextion_Init();


		break;

	case MEM_ERASE:

		if (Nextion_Get_Mem_Erase_Status() == ERR_REQUESTED) {
			Erase_SD_Card();
			Nextion_Retunr_From_Mem_Erase();
		}

		break;

	default:
		break;
	}


}











