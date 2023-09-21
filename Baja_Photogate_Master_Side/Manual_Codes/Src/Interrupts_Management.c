
#include "main.h"
#include "definitions.h"
#include "rf_driver.h"



extern TIM_HandleTypeDef htim10;
extern TIM_HandleTypeDef htim9;

volatile uint16_t StopWatch_Counter_us = 0x00;

volatile uint8_t StopWatch_Counter_secs = 0x00, Stopwatch_Counter_Mins = 0x00;

volatile uint16_t Meastime_us = 0x00;
volatile uint8_t Meastime_secs = 0x00, Meastime_Mins = 0x00;



uint8_t Generic_Tim_Count_100ms = 0x00;

extern volatile BEACONMODE_TypeDef Device_Current_Mode;

uint8_t regread = 0x00;
uint8_t msg_len = 0x00;
char msg[80];

void RF_Transmit_Config_MSG(BEACONMODE_TypeDef reqmode);



void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)						// Management of Timer interrupts
{




	if(htim->Instance == TIM9)
		{
		StopWatch_Counter_us++;

		if(StopWatch_Counter_us >= 2000)
		{
			StopWatch_Counter_us = 0x00;
			StopWatch_Counter_secs++;
			if(StopWatch_Counter_secs >= 60)
			{
				StopWatch_Counter_secs = 0x00;
				Stopwatch_Counter_Mins++;

			}

		}


		}



	else if (htim->Instance == TIM10){		//Happens each 1 second.  Separated from Stopwatch timer.


		Generic_Tim_Count_100ms++;

		if(Generic_Tim_Count_100ms >= 1){
			Generic_Tim_Count_100ms = 0x00;
			if(Device_Current_Mode == STANDBY_MODE) {RF_Transmit_Config_MSG(STANDBY_MODE);}	//Requires all modules to be at standby when Master is on standby - Guarantees Sync
			else if (Device_Current_Mode == RACE_MODE) {RF_Transmit_Config_MSG(RACE_MODE);}

		msg_len = sprintf( msg,"Master Actual mode: %d \r\n", Device_Current_Mode);
		CDC_Transmit_FS(msg, msg_len);

		HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);

		rf_receive();

		}

		}




}


void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){								//This functions was imported from "rf_driver.c". Put here for organization purposes

	SHORTMESSAGE_TypeDef Message;
	BEACONMODE_TypeDef Modo;
	uint8_t *lqi;
	uint8_t *rssi;

	float Vehicle_Speed = 0.0;
	float dist = BEACON_DISTANCE;

	uint8_t Beacon_Blocker = 0x00; 										//This variable follows the expected beacon order, and only allows recordings to be done whe this given order is fullfilled.


	switch(GPIO_Pin){


	case Extern_Button_Pin:

		if(Device_Current_Mode == STANDBY_MODE) Device_Current_Mode = RACE_MODE;
		else if(Device_Current_Mode == RACE_MODE) Device_Current_Mode = STANDBY_MODE;

		break;




	case CC1101_GD0_Intpin_Pin:

		if(receive_frame_fixdlen(&Message, 0x03, lqi, rssi) == FRAME_OK){


		switch(Device_Current_Mode){

			case STANDBY_MODE:

				if(Message.Beacon_Mode != STANDBY_MODE) {
					msg_len = sprintf( msg,"Beacon of ID 0x%x is out of sync!! \r\n", (Message.Beacon_Id));
					CDC_Transmit_FS(&msg, msg_len);
				}
				else {
					msg_len = sprintf( msg,"Beacon 0x%x is SYNC -> Stat: %d \r\n", Message.Beacon_Id, Message.Sensor_status);
					CDC_Transmit_FS(&msg, msg_len);
				}

				break;

			case RACE_MODE:

				if(Message.Beacon_Mode != RACE_MODE) {
					msg_len = sprintf( msg,"Beacon of ID 0x%x is out of sync!! \r\n", Message.Beacon_Id);
					CDC_Transmit_FS(msg, msg_len);

				}

				else if (Message.Beacon_Id == BEACON1  && Beacon_Blocker == 0x00){

					msg_len = sprintf( msg,"Car detected at BEACON 1 - StopWatch Started! \r\n");
					CDC_Transmit_FS(msg, msg_len);

					StopWatch_Counter_us = 0x00;
					StopWatch_Counter_secs = 0x00;
					Stopwatch_Counter_Mins = 0x00;
					HAL_TIM_Base_Start_IT(&htim9);
					Beacon_Blocker = 0x01;
				}

				else if (Message.Beacon_Id == BEACON2  && Beacon_Blocker == 0x01){

					HAL_TIM_Base_Stop_IT(&htim9);
					Meastime_us = StopWatch_Counter_us;
					Meastime_secs = StopWatch_Counter_secs;
					Meastime_Mins = Stopwatch_Counter_Mins;

					StopWatch_Counter_us = 0x00;
					StopWatch_Counter_secs = 0x00;
					Stopwatch_Counter_Mins = 0x00;

					msg_len = sprintf( msg,'Car detected at BEACON 2 - Measured time: %d:%.2d.%.3d%d \r\n', Meastime_Mins, Meastime_secs, (Meastime_us/2),((Meastime_us%2)));
					CDC_Transmit_FS(msg, msg_len);

					Beacon_Blocker = 0x02;
				}

				else if (Message.Beacon_Id == BEACON3 && Beacon_Blocker == 0x02)
				{
					HAL_TIM_Base_Start_IT(&htim9);

					msg_len = sprintf( msg,'Car detected at BEACON 3 - Timer reset \r\n');
					CDC_Transmit_FS(msg, msg_len);

					Beacon_Blocker = 0x03;

				}

				else if (Message.Beacon_Id == BEACON4 && Beacon_Blocker == 0x03)
				{
					HAL_TIM_Base_Stop_IT(&htim9);

					Vehicle_Speed = (StopWatch_Counter_us/2) + ((StopWatch_Counter_us%2)*0.1);
					Vehicle_Speed = dist/Vehicle_Speed;
					Vehicle_Speed = (Vehicle_Speed*3.6);

					msg_len = sprintf( msg,'Vehicle Speed was: %.3f km/h\r\n', Vehicle_Speed);
					CDC_Transmit_FS(msg, msg_len);

					RF_Transmit_Config_MSG(STANDBY_MODE);
					Beacon_Blocker = 0x00;

				}


				break;

			}


		break;
		}


	case CC1101_GD2_Intpin_Pin:

		break;


	}
}


void RF_Transmit_Config_MSG(BEACONMODE_TypeDef reqmode){

	SHORTMESSAGE_TypeDef Payload;


	Payload.Beacon_Id = BeaconID;
	Payload.Beacon_Mode = reqmode;
	Payload.Sensor_status = 0x01;

	send_frame_fixdlen(&Payload, 0x03);
	rf_receive();

}








