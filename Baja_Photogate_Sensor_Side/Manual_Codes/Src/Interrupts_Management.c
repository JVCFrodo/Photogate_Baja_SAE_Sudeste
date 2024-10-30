#include "main.h"
#include "definitions.h"
#include "shared_definitions.h"
#include "hw_dependencies.h"
#include "nrf24.h"
#include "math.h"

volatile uint16_t StopWatch_Counter_1ms = 0x00, StopWatch_Counter_NoZero_1ms =
		0x00, StopWatch_Counter_1s = 0x00;
volatile uint16_t Scheduler_Counter_200ms = 0x00, Sensor_Ind_Counter = 0x00;
volatile uint16_t Message_Counter = 0x00, Frozen_Timer_s,
		Frozen_Timer_ms = 0x00;

volatile SENSORSTATS_TypeDef Sensor_Status_Act = NON_INTERRUPTED,
		Sensor_Status_Prev = NON_INTERRUPTED;

volatile uint8_t Led_Tim_Count = 0x00;
volatile uint8_t Interruption_Flag_WDT = 0x00, Interruption_Tim_Count = 0x00;



extern volatile BEACONMODE_TypeDef Device_Current_Mode;


uint8_t msg[50];

uint8_t Pinstate = 0x00;

extern void RF_Transmit_Alive_MSG();
extern void RF_Transmit_Trigger_MSG();
extern void RF_Read_Settings_Msg(BEACONMODE_TypeDef Received_Mode);
extern uint8_t Calc_Batt_Perc();


void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)	// Management of Timer interrupts
{

	if (htim->Instance == TIM11)// period  = 500 us - Used as stopwatch timebase
	{

		StopWatch_Counter_1ms++;//This timer is reset after sending RF Message
		StopWatch_Counter_NoZero_1ms++;
		Pinstate = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0);

		if (Pinstate == TRUE) {
			Sensor_Status_Act = INTERRUPTED;
			Sensor_Ind_Counter = 0;
			if ((Interruption_Flag_WDT == 0x00)
					&& (Device_Current_Mode == RACE_MODE)) {
				Frozen_Timer_ms = StopWatch_Counter_1ms;
				RF_Transmit_Trigger_MSG();
				Interruption_Flag_WDT = 0x01;
			}
			Sensor_Ind_Counter = 0x00;
		} else if (Pinstate == FALSE) {
			Sensor_Status_Act = NON_INTERRUPTED;
			Sensor_Ind_Counter += 1;
			if (Sensor_Ind_Counter >= 2000)
				Sensor_Ind_Counter = 2000;
		}

		if (StopWatch_Counter_NoZero_1ms % 10 == 0) {
			if (Sensor_Status_Act == INTERRUPTED)
				HAL_GPIO_WritePin(Led_Azul_GPIO_Port, Led_Azul_Pin, TRUE);
			else if (Sensor_Status_Act == NON_INTERRUPTED)
				HAL_GPIO_WritePin(Led_Azul_GPIO_Port, Led_Azul_Pin, FALSE);

			if (StopWatch_Counter_NoZero_1ms >= 1000) {

				StopWatch_Counter_1s++;
				StopWatch_Counter_NoZero_1ms = 0x00;
				if (StopWatch_Counter_1s >= 65535)
					StopWatch_Counter_1s = 0x00;
			}
		}

	} else if (htim->Instance == TIM10) {

		Scheduler_Counter_200ms++;

		nRF24_FlushRX();
		nRF24_FlushTX();
		nRF24_ClearIRQFlags();

		Calc_Batt_Perc();



		if (Scheduler_Counter_200ms >= 5)// Period = 200ms * 5 = 1000 milisseconds
				{
				Scheduler_Counter_200ms = 0x00;
				RF_Transmit_Alive_MSG();


		}
	}

}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {//This functions was imported from "rf_driver.c". Put here for organization purposes

	BEACONMESSAGE_TypeDef Message;
	uint8_t pipe = 0xff;
	uint8_t Irq_Flags = 0xff;
	uint8_t Msg_Len = Payload_Len;

	switch (GPIO_Pin) {

	case NRF_IRQ_Pin:

		Irq_Flags = nRF24_GetIRQFlags();

		if ((Irq_Flags & nRF24_FLAG_RX_DR ) == nRF24_FLAG_RX_DR) {

			pipe = nRF24_ReadPayload(&Message, &Msg_Len);
			if (pipe == 0x00 && Message.Beacon_Id == HOST)
				RF_Read_Settings_Msg(Message.Beacon_Mode);

		}
		nRF24_ClearIRQFlags();
		nRF24_FlushRX();
		break;
	default:
		break;

	}
}

