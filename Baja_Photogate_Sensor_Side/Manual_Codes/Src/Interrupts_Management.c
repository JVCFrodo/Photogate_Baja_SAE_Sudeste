
#include "main.h"
#include "definitions.h"
#include "rf_driver.h"
#include "usbd_cdc_if.h"
#include "math.h"

volatile uint8_t StopWatch_Counter_500u = 0x00, StopWatch_Counter_100ms = 0x00, Sw_Counter_100ms = 0x00;
volatile uint8_t Led_Tim_Count = 0x00;

volatile uint8_t Interruption_Flag_WDT = 0x00, Interruption_Tim_Count = 0x00;

uint8_t msg_len;
uint8_t msg[40] = {0};

extern const uint8_t cc1100_OOK_5_kb[25][2];



extern volatile BEACONMODE_TypeDef Device_Current_Mode;
extern volatile uint16_t Analog_read[2];

void RF_Transmit_Alive_MSG();
void RF_Transmit_Trigger_MSG();
void RF_Read_Settings_Msg(BEACONMODE_TypeDef Received_Mode);
void Check_Log_Analog_Values();


void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)						// Management of Timer interrupts
{

	if (htim->Instance == TIM9)															// period  = 500 us - Used as stopwatch timebase
	{
		StopWatch_Counter_500u++;

		if(StopWatch_Counter_500u >= 200)												// period = 100ms
			{

			StopWatch_Counter_500u = 0x00;
			StopWatch_Counter_100ms++;

			}
				if(StopWatch_Counter_100ms >= 20)												// period =	1s - Alive LED blink
					{
					StopWatch_Counter_100ms = 0x00;

					if(Interruption_Flag_WDT) Interruption_Tim_Count++;
					if(Interruption_Tim_Count >= 4){
						Interruption_Tim_Count = 0x00;
						Interruption_Flag_WDT = 0x00;
						Restar_External_Interrupt();


					}

					if(Device_Current_Mode == STANDBY_MODE)
					{
						//RF_Transmit_Alive_MSG();
						Check_Log_Analog_Values();
						//HAL_GPIO_WritePin(GPIOC, Led_Azul_Pin, FALSE);
						//rf_receive();

					}
					else if (Device_Current_Mode == RACE_MODE)
						{
						//HAL_GPIO_WritePin(GPIOC, Led_Azul_Pin, TRUE);
						RF_Read_Settings_Msg(STANDBY_MODE);
						//rf_receive();
						}



	}
	}

}


void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){								//This functions was imported from "rf_driver.c". Put here for organization purposes

	SHORTMESSAGE_TypeDef Message;
	BEACONMODE_TypeDef Modo;
	uint8_t lqi = 0x00;
	uint8_t rssi = 0x00;


	switch(GPIO_Pin){


	case IR_Receiver_Outpin_Pin:

		HAL_NVIC_DisableIRQ(EXTI1_IRQn);	//DESABILITAR INTERRUPÇÂO DO PINO PARA NÂO TRIGGAR MAIS INTERRUPÇÔES - REABILITARÁ DEPOIS DE 3 SEGUNDOS
		Interruption_Flag_WDT = 0x01;

		if(Device_Current_Mode == RACE_MODE) RF_Transmit_Trigger_MSG();

		break;

	case CC1101_GD0_Intpin_Pin:

		if(receive_frame_fixdlen(&Message, 0x03, &lqi, &rssi) == FRAME_OK){

		if(Message.Beacon_Id == HOST)
		{

			RF_Read_Settings_Msg(Message.Beacon_Mode);
			//RF_Transmit_Alive_MSG();

		}
		}

		break;

	case CC1101_GD2_Intpin_Pin:

		break;


	default:
		break;



	}
}

void RF_Transmit_Alive_MSG(){

	SHORTMESSAGE_TypeDef Payload = {0};


	Payload.Beacon_Id = BeaconID;
	Payload.Beacon_Mode = STANDBY_MODE;
	Payload.Sensor_status = !HAL_GPIO_ReadPin(IR_Receiver_Outpin_GPIO_Port, IR_Receiver_Outpin_Pin);

	send_frame_fixdlen(&Payload, 0x03);
	rf_receive();


}

void RF_Transmit_Trigger_MSG(){

	SHORTMESSAGE_TypeDef Payload = {0};


	Payload.Beacon_Id = BeaconID;
	Payload.Beacon_Mode = RACE_MODE;
	Payload.Sensor_status = 0x01;

	send_frame_fixdlen(&Payload, 0x03);

	rf_receive();

}

void RF_Read_Settings_Msg(BEACONMODE_TypeDef Received_Mode)
{

	GPIO_InitTypeDef GPIO_InitStruct = {0};

	if(Received_Mode != Device_Current_Mode){


	switch(Received_Mode){

	case STANDBY_MODE:

		Device_Current_Mode = STANDBY_MODE;

		HAL_NVIC_DisableIRQ(EXTI1_IRQn);
		HAL_GPIO_DeInit(IR_Receiver_Outpin_GPIO_Port, IR_Receiver_Outpin_Pin);

		  /*Configure GPIO pin : IR_Receiver_Outpin_Pin as simple Input pin for state monitoring*/
		  GPIO_InitStruct.Pin = IR_Receiver_Outpin_Pin;
		  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
		  //GPIO_InitStruct.Pull = GPIO_PULLUP;
		  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
		  HAL_GPIO_Init(IR_Receiver_Outpin_GPIO_Port, &GPIO_InitStruct);


		break;

	case RACE_MODE:

		Device_Current_Mode = RACE_MODE;

		HAL_GPIO_DeInit(IR_Receiver_Outpin_GPIO_Port, IR_Receiver_Outpin_Pin);

		/*Configure GPIO pin : IR_Receiver_Outpin_Pin as Interruption on Falling Edge */
		GPIO_InitStruct.Pin = IR_Receiver_Outpin_Pin;
		GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
		GPIO_InitStruct.Pull = GPIO_PULLUP;
		HAL_GPIO_Init(IR_Receiver_Outpin_GPIO_Port, &GPIO_InitStruct);
		Restar_External_Interrupt();


		break;

	}

	}


}

void Restar_External_Interrupt()			// Need to use this routine in order to avoid an activation of the External interrupt on the exact moment it is turned on again
{

	__HAL_GPIO_EXTI_CLEAR_IT(IR_Receiver_Outpin_Pin);
	HAL_NVIC_ClearPendingIRQ(EXTI1_IRQn);
	HAL_NVIC_EnableIRQ(EXTI1_IRQn);


}

void Check_Log_Analog_Values()
{


	uint16_t Trimpot_Voltage, IR_Ana_Voltage;
	float mV_Conv_Factor = 0.805860;
	uint8_t pinstate = 0x00;

	uint8_t msglen = 0x00;
	uint8_t msg[50];


	Trimpot_Voltage = ceil(Analog_read[0]*mV_Conv_Factor);
	IR_Ana_Voltage = ceil(Analog_read[1]*mV_Conv_Factor);
	pinstate = HAL_GPIO_ReadPin(IR_Receiver_Outpin_GPIO_Port, IR_Receiver_Outpin_Pin);

	msglen = sprintf(msg,"Trimpot Voltage read = %d mV \r\n", Trimpot_Voltage);
	CDC_Transmit_FS(msg, msglen);

	msglen = sprintf(msg, "IR Sensor Voltage read = %d mV \r\n", IR_Ana_Voltage);
	CDC_Transmit_FS(msg, msglen);

	msglen = sprintf(msg, "Pin State is = %d  \r\n", pinstate);
	CDC_Transmit_FS(msg, msglen);



	if(Trimpot_Voltage >= IR_Ana_Voltage) HAL_GPIO_WritePin(IR_Indicator_GPIO_Port, IR_Indicator_Pin, TRUE);
	else if(IR_Ana_Voltage >= Trimpot_Voltage) HAL_GPIO_WritePin(IR_Indicator_GPIO_Port, IR_Indicator_Pin, FALSE);

}



