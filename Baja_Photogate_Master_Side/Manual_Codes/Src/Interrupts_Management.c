
#include "main.h"


#include "usbd_cdc_if.h"
#include "aditional_functions_master.h"

extern volatile BEACONMODE_TypeDef Device_Current_Mode;

extern TIM_HandleTypeDef htim11;


extern volatile uint8_t WDTf_Beacon1, WDTf_Beacon2, WDTf_Beacon3, WDTf_Beacon4;

volatile uint16_t StopWatch_Counter_ms = 0x00;
volatile uint8_t StopWatch_Counter_secs = 0x00, Stopwatch_Counter_Mins = 0x00;
volatile uint8_t Generic_Tim_Count_200ms = 0x00;
volatile char Payload_Log[32];
volatile uint8_t RX_Payload_Flag = 0x00, Dummy_Payload_Len = 0x00;
volatile uint8_t Cancel_req = 0x00, Start_Req = 0x00, Step = 0x00, Sync = 0x00, Setup_Complete = 0x00;



volatile uint8_t Beacon_Blocker = 0x00; 	//This variable follows the expected beacon order, and only allows recordings to be done whe this given order is fullfilled.

uint8_t regread = 0x00;
volatile uint8_t msg_len = 0x00;
char msg[80];

extern void RF_Transmit_Config_MSG(BEACONMODE_TypeDef reqmode);
extern void Beacon_Sync_Calcs(BEACONMESSAGE_TypeDef* Message);
extern void WDT_Calcs_Updates();
extern void NRF_24_Log_Init();




void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)						// Management of Timer interrupts
{


	if(htim->Instance == TIM11)			//Master Stopwatch and Sync timer interrupts
		{

		StopWatch_Counter_secs++;
		HAL_GPIO_TogglePin(Led_Azul_GPIO_Port, Led_Azul_Pin);
		/*
		StopWatch_Counter_ms++;
		if(StopWatch_Counter_ms >= 1000)
		{
			StopWatch_Counter_ms = 0x00;
			StopWatch_Counter_secs++;

		}
		*/
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
		Calc_Batt_Perc(0);

		//Happens each 1s (200ms * 5)
		if(Generic_Tim_Count_200ms >= 5)
		{

			Generic_Tim_Count_200ms = 0x00;
			if(Device_Current_Mode == STANDBY_MODE || Device_Current_Mode == RACE_MODE) Regular_Nextion_Updates();

			}
		}
	}



void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){								//This functions was imported from "rf_driver.c". Put here for organization purposes

	BEACONMESSAGE_TypeDef Message;
	uint8_t Irq_Flags = 0xFF;
	Nextion_Pages_TypeDef Nextion_Page = 0xff;
	uint8_t Msg_Len = Payload_Len;

	switch(GPIO_Pin){

	case NRF_IRQ_Pin:

		Irq_Flags = nRF24_GetIRQFlags();
		if((Irq_Flags & nRF24_FLAG_RX_DR) == nRF24_FLAG_RX_DR){
			On_Valid_NRF24_Msg();
		}

		nRF24_ClearIRQFlags();
		nRF24_FlushRX();

}
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart){


	return 0;
}

