
#include "main.h"
#include "definitions.h"
#include "shared_definitions.h"
#include "hw_dependencies.h"
#include "nrf24.h"
#include "math.h"

volatile uint8_t StopWatch_Counter_1ms = 0x00, StopWatch_Counter_100ms = 0x00;
volatile uint8_t Scheduler_Counter_200ms = 0x00;
volatile uint16_t Message_Counter = 0x00, Frozen_Timer = 0x00;

volatile uint32_t Pulse_Cnt_38Khz = 0x00;
volatile SENSORSTATS_TypeDef Sensor_Status_Act = NON_INTERRUPTED, Sensor_Status_Prev = NON_INTERRUPTED;

volatile uint8_t Led_Tim_Count = 0x00;
volatile uint8_t Interruption_Flag_WDT = 0x00, Interruption_Tim_Count = 0x00;

extern const uint8_t cc1100_OOK_5_kb[25][2];

extern volatile BEACONMODE_TypeDef Device_Current_Mode;
extern volatile uint16_t Analog_read[2];
extern TIM_HandleTypeDef htim2;

uint8_t msg[50];


extern void RF_Transmit_Alive_MSG();
extern void RF_Transmit_Trigger_MSG();
extern void RF_Read_Settings_Msg(BEACONMODE_TypeDef Received_Mode);
extern void Report_Sensor_Counter_Value_USB(uint32_t val);
void Check_Log_Analog_Values();



void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)						// Management of Timer interrupts
{

	if (htim->Instance == TIM9)															// period  = 500 us - Used as stopwatch timebase
	{

		StopWatch_Counter_1ms++;

		if(StopWatch_Counter_1ms >= 2)												// period = 1ms
			{
				StopWatch_Counter_100ms++;
				StopWatch_Counter_1ms = 0x00;
				Pulse_Cnt_38Khz = TIM2->CNT;
				TIM2->CNT = 0x00;

				if(Pulse_Cnt_38Khz < 35) {

					Sensor_Status_Act = INTERRUPTED;

					if((Sensor_Status_Act != Sensor_Status_Prev) && (Interruption_Flag_WDT == 0x00) && (Device_Current_Mode == RACE_MODE)){
						Interruption_Flag_WDT = 0x01;
						Frozen_Timer = (StopWatch_Counter_100ms * 10) + StopWatch_Counter_1ms;
						RF_Transmit_Trigger_MSG();

					}
				}
				else Sensor_Status_Act = NON_INTERRUPTED;

				Sensor_Status_Prev = Sensor_Status_Act;


				if(StopWatch_Counter_100ms >= 100) StopWatch_Counter_100ms = 0x00;
			}

	}
	else if(htim->Instance == TIM10){

	Scheduler_Counter_200ms++;

	nRF24_FlushRX();
	nRF24_FlushTX();
	nRF24_ClearIRQFlags();


	if(Device_Current_Mode == STANDBY_MODE) HAL_GPIO_WritePin(Led_Azul_GPIO_Port, Led_Azul_Pin, TRUE);
	else if (Device_Current_Mode == RACE_MODE) HAL_GPIO_WritePin(Led_Azul_GPIO_Port, Led_Azul_Pin, FALSE);



	if(Scheduler_Counter_200ms >= 5)		// Period = 200ms * 5 = 1 second
	{

			//Report_Over_USB();

			/*Routine for disabling duplicate reads from sensor. Set comparison value for the time in
		    seconds to be waited before re-activating the sensor.*/
		    if(Interruption_Flag_WDT) Interruption_Tim_Count++;
			if(Interruption_Tim_Count >= 4)
			{
				Interruption_Tim_Count = 0x00;
				Interruption_Flag_WDT = 0x00;
			}

			if(Interruption_Flag_WDT == 0x00) RF_Transmit_Alive_MSG();
			else RF_Transmit_Trigger_MSG();

			Scheduler_Counter_200ms = 0x00;

	}
	}

}


void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){								//This functions was imported from "rf_driver.c". Put here for organization purposes

	BEACONMESSAGE_TypeDef Message;
	uint8_t pipe = 0xff;
	uint8_t Irq_Flags = 0xff;
	uint8_t Msg_Len = Payload_Len;


	switch(GPIO_Pin){

	case NRF_IRQ_Pin:

		Irq_Flags = nRF24_GetIRQFlags();

		if((Irq_Flags & nRF24_FLAG_RX_DR) == nRF24_FLAG_RX_DR){

			pipe = nRF24_ReadPayload(&Message, &Msg_Len);

			if(pipe == 0x00 && Message.Beacon_Id == HOST) RF_Read_Settings_Msg(Message.Beacon_Mode);

		}
		nRF24_ClearIRQFlags();
		nRF24_FlushRX();
		break;

	default:
		break;

	}
}









