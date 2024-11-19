#include "main.h"
#include "definitions.h"
#include "shared_definitions.h"
#include "hw_dependencies.h"
#include "nrf24.h"
#include "math.h"

volatile uint16_t StopWatch_Counter_1ms = 0x00, StopWatch_Counter_NoZero_1ms =
		0x00, StopWatch_Counter_1s = 0x00;
volatile uint16_t Scheduler_Counter_10ms = 0x00, Sensor_Ind_Counter = 0x00;
volatile uint16_t Message_Counter = 0x00, Frozen_Timer_s,
		Frozen_Timer_ms = 0x00;

volatile SENSORSTATS_TypeDef Sensor_Status_Act = NON_INTERRUPTED;

volatile uint8_t Led_Tim_Count = 0x00;
volatile uint8_t Car_Detected_Flag = 0x00, Interruption_Tim_Count = 0x00;



extern volatile BEACONMODE_TypeDef Device_Current_Mode;
extern uint8_t Battery_Actual_Val;


uint8_t msg[50];

uint8_t Pinstate = 0x00;

extern void RF_Transmit_Alive_MSG();
extern void RF_Transmit_Trigger_MSG();
extern void RF_Read_Settings_Msg(BEACONMODE_TypeDef Received_Mode);
extern uint8_t Calc_Batt_Perc();

uint16_t Dummy_s = 0x00, Dummy_ms = 0x00;

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)	// Management of Timer interrupts
{

	if (htim->Instance == TIM11)// period  = 1 s - Used as stopwatch timebase (Read dircetly from register)
	{
		if(Car_Detected_Flag == 0x00)RF_Transmit_Alive_MSG();
		else RF_Transmit_Trigger_MSG();

		/*
		if (StopWatch_Counter_1s >= 65535)
			StopWatch_Counter_1s = 0x00;
		*/
	}

	else if (htim->Instance == TIM10) {

		Scheduler_Counter_10ms++;

		if(Device_Current_Mode == STANDBY_MODE){

			Sensor_Status_Act = Check_Sensor_Status();
			if (Sensor_Status_Act == INTERRUPTED)
				HAL_GPIO_WritePin(Led_Azul_GPIO_Port, Led_Azul_Pin, TRUE);
			else if (Sensor_Status_Act == NON_INTERRUPTED)
				HAL_GPIO_WritePin(Led_Azul_GPIO_Port, Led_Azul_Pin, FALSE);

			}

		// Period = 100ms * 10 = 1000 milisseconds
		if (Scheduler_Counter_10ms >= 100){
				Scheduler_Counter_10ms = 0x00;

		}
		else if((Scheduler_Counter_10ms % 20) == 0) Battery_Actual_Val = Calc_Batt_Perc();

	}

}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {

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


	case GPIO_PIN_0:
		if (Device_Current_Mode == RACE_MODE){
			Frozen_Timer_ms = TIM11->CNT;
			HAL_NVIC_DisableIRQ(EXTI0_IRQn);
			Car_Detected_Flag = 0x01;
			HAL_GPIO_WritePin(Led_Azul_GPIO_Port, Led_Azul_Pin, TRUE);
			RF_Transmit_Trigger_MSG();
		}
		break;
		




	default:
		break;

	}
}

