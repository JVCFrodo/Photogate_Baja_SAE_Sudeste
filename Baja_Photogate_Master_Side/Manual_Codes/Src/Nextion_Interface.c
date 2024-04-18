
#include "Nextion_Interface.h"
#include "main.h"
#include "shared_definitions.h"
#include <stdio.h>



extern UART_HandleTypeDef huart1;
extern volatile BEACONMODE_TypeDef Device_Current_Mode;
extern volatile uint8_t Rx_Buffer[20];
uint8_t Mem_Sensor_Status = 0x00;

BEACONMODE_TypeDef Return_Mode = 0xFF;

Nextion_Pages_TypeDef Get_Nextion_Pages();


void Wait_Nextion_Resp_us(uint16_t tim_to_wait_us)
{
	uint32_t i = 0x00;
	uint32_t ticks = 0x00;

		ticks  = 64 * tim_to_wait_us;
		for(i = 0x00; i <= ticks; i++){}	//empirically waits for DMA transfer to finish. Attempts by IRQ were unsuccessfull

}


void Nextion_Init(){

	char Send_msg[20];
	uint8_t Recv_msg[20];
	uint8_t Msg_len = 0x00;


	Msg_len = sprintf(Send_msg, "get dp%c%c%c", Nextion_EndChar, Nextion_EndChar, Nextion_EndChar);

	HAL_UART_Transmit(&huart1, Send_msg, Msg_len , 10);


	if((Rx_Buffer[0] != 71) || (Rx_Buffer[1] != 2)){

		Msg_len = sprintf(Send_msg, "dp=2%c%c%c", Nextion_EndChar, Nextion_EndChar, Nextion_EndChar);
		HAL_UART_Transmit(&huart1, Send_msg, Msg_len, 10);

	}

	//for(uint8_t i = 0x00; i < 4; i++) Update_Sensor_Status_Stdby(i, 0, 0, 0);


}

void Update_Sensor_Status_Stdby(uint8_t sensor, uint8_t Sync, uint8_t Beam, uint8_t Battery){

	char Send_msg[40];
	uint8_t Msg_len = 0x00;

	switch(sensor){

	case BEACON1:

		if(Sync == 1) Msg_len = sprintf(Send_msg, "p1.pic=4%c%c%c", Nextion_EndChar, Nextion_EndChar, Nextion_EndChar);
		else if (Sync == 0) Msg_len = sprintf(Send_msg, "p1.pic=5%c%c%c", Nextion_EndChar, Nextion_EndChar, Nextion_EndChar);
		HAL_UART_Transmit(&huart1, Send_msg, Msg_len,10);



		if(Beam == NON_INTERRUPTED ) Msg_len = sprintf(Send_msg, "p2.pic=4%c%c%c", Nextion_EndChar, Nextion_EndChar, Nextion_EndChar);
		else if (Beam == INTERRUPTED) Msg_len = sprintf(Send_msg, "p2.pic=5%c%c%c", Nextion_EndChar, Nextion_EndChar, Nextion_EndChar);
		HAL_UART_Transmit(&huart1, Send_msg, Msg_len, 10);



		Msg_len = sprintf(Send_msg, "t0.txt=%c%d%c%c%c%c",'"', Battery,'"', Nextion_EndChar, Nextion_EndChar, Nextion_EndChar);
		HAL_UART_Transmit(&huart1, Send_msg, Msg_len, 10);



	break;

	case BEACON2:

		if(Sync == 1) Msg_len = sprintf(Send_msg, "p3.pic=4%c%c%c", Nextion_EndChar, Nextion_EndChar, Nextion_EndChar);
		else if (Sync == 0) Msg_len = sprintf(Send_msg, "p3.pic=5%c%c%c", Nextion_EndChar, Nextion_EndChar, Nextion_EndChar);
		HAL_UART_Transmit(&huart1, Send_msg, Msg_len, 10);

		if(Beam == NON_INTERRUPTED) Msg_len = sprintf(Send_msg, "p4.pic=4%c%c%c", Nextion_EndChar, Nextion_EndChar, Nextion_EndChar);
		else if (Beam == INTERRUPTED) Msg_len = sprintf(Send_msg, "p4.pic=5%c%c%c", Nextion_EndChar, Nextion_EndChar, Nextion_EndChar);
		HAL_UART_Transmit(&huart1, Send_msg, Msg_len, 10);

		Msg_len = sprintf(Send_msg, "t1.txt=%s %s", Battery, '%');
		HAL_UART_Transmit(&huart1, Send_msg, Msg_len, 10);


		break;

	case BEACON3:

		if(Sync == 1) Msg_len = sprintf(Send_msg, "p5.pic=4%c%c%c", Nextion_EndChar, Nextion_EndChar, Nextion_EndChar);
		else if (Sync == 0) Msg_len = sprintf(Send_msg, "p5.pic=5%c%c%c", Nextion_EndChar, Nextion_EndChar, Nextion_EndChar);
		HAL_UART_Transmit(&huart1, Send_msg, Msg_len, 10);

		if(Beam == NON_INTERRUPTED) Msg_len = sprintf(Send_msg, "p6.pic=4%c%c%c", Nextion_EndChar, Nextion_EndChar, Nextion_EndChar);
		else if (Beam == INTERRUPTED) Msg_len = sprintf(Send_msg, "p6.pic=5%c%c%c", Nextion_EndChar, Nextion_EndChar, Nextion_EndChar);
		HAL_UART_Transmit(&huart1, Send_msg, Msg_len, 10);

		Msg_len = sprintf(Send_msg, "t2.txt=%s %s", Battery, '%');
		HAL_UART_Transmit(&huart1, Send_msg, Msg_len, 10);


		break;

	case BEACON4:


		if(Sync == 1) Msg_len = sprintf(Send_msg, "p7.pic=4%c%c%c", Nextion_EndChar, Nextion_EndChar, Nextion_EndChar);
		else if (Sync == 0) Msg_len = sprintf(Send_msg, "p7.pic=5%c%c%c", Nextion_EndChar, Nextion_EndChar, Nextion_EndChar);
		HAL_UART_Transmit(&huart1, Send_msg, Msg_len, 10);

		if(Beam == NON_INTERRUPTED) Msg_len = sprintf(Send_msg, "p8.pic=4%c%c%c", Nextion_EndChar, Nextion_EndChar, Nextion_EndChar);
		else if (Beam == INTERRUPTED) Msg_len = sprintf(Send_msg, "p8.pic=5%c%c%c", Nextion_EndChar, Nextion_EndChar, Nextion_EndChar);
		HAL_UART_Transmit(&huart1, Send_msg, Msg_len, 10);

		Msg_len = sprintf(Send_msg, "t3.txt=%s %s", Battery, '%');
		HAL_UART_Transmit(&huart1, Send_msg, Msg_len, 10);

		break;

	default:
	break;

	}


}


void Update_Sensor_Status_Run(uint8_t Sensor, uint8_t Sync){

	char Send_msg[20];
	uint8_t Msg_len = 0x00;

	switch(Sensor){

	case BEACON1:

		if(Sync == 1) {
			Msg_len = sprintf(Send_msg, "p1.pic=4%c%c%c", Nextion_EndChar, Nextion_EndChar, Nextion_EndChar);
			Mem_Sensor_Status |= 0x01;
		}
		else if (Sync == 0) {
			Msg_len = sprintf(Send_msg, "p1.pic=5%c%c%c", Nextion_EndChar, Nextion_EndChar, Nextion_EndChar);
			Mem_Sensor_Status &= 0xFE;
		}
		HAL_UART_Transmit(&huart1, Send_msg, Msg_len, 10);


	break;

	case BEACON2:

		if(Sync == 1) {
			Msg_len = sprintf(Send_msg, "p2.pic=4%c%c%c", Nextion_EndChar, Nextion_EndChar, Nextion_EndChar);
			Mem_Sensor_Status |= 0x02;
		}
		else if (Sync == 0) {
			Msg_len = sprintf(Send_msg, "p2.pic=5%c%c%c", Nextion_EndChar, Nextion_EndChar, Nextion_EndChar);
			Mem_Sensor_Status &= 0xFD;
		}
		HAL_UART_Transmit(&huart1, Send_msg, Msg_len, 10);

		break;

	case BEACON3:

		if(Sync == 1) {
			Msg_len = sprintf(Send_msg, "p4.pic=4%c%c%c", Nextion_EndChar, Nextion_EndChar, Nextion_EndChar);
			Mem_Sensor_Status |= 0x04;
		}
		else if (Sync == 0) {
			Msg_len = sprintf(Send_msg, "p4.pic=5%c%c%c", Nextion_EndChar, Nextion_EndChar, Nextion_EndChar);
			Mem_Sensor_Status &= 0xFB;
		}
		HAL_UART_Transmit(&huart1, Send_msg, Msg_len, 10);

		break;

	case BEACON4:


		if(Sync == 1) {
			Msg_len = sprintf(Send_msg, "p3.pic=4%c%c%c", Nextion_EndChar, Nextion_EndChar, Nextion_EndChar);
			Mem_Sensor_Status |= 0x08;
		}
		else if (Sync == 0) {
			Msg_len = sprintf(Send_msg, "p3.pic=5%c%c%c", Nextion_EndChar, Nextion_EndChar, Nextion_EndChar);
			Mem_Sensor_Status &= 0xF7;
		}
		HAL_UART_Transmit(&huart1, Send_msg, Msg_len, 10);

		break;

	default:
	break;
	}

	if(Get_Nextion_Pages() == RACE_PAGE){
	if((Mem_Sensor_Status & 0x0F) == 0x0F) Msg_len = sprintf(Send_msg, "p5.pic=6%c%c%c", Nextion_EndChar, Nextion_EndChar, Nextion_EndChar);
	else Msg_len = sprintf(Send_msg, "p5.pic=7%c%c%c", Nextion_EndChar, Nextion_EndChar, Nextion_EndChar);

	HAL_UART_Transmit(&huart1, Send_msg, Msg_len, 10);
	}





}


uint8_t Get_Car_Num(){

	char Send_msg[35];
	uint8_t Recv_msg[35];
	uint8_t Msg_len = 0x00;

	uint8_t CarNum_H = 0xFF;
	uint8_t CarNum_L = 0xFF;
	uint8_t CarNum_T = 0xFF;

	Msg_len = sprintf(Send_msg, "get globals.CarNum_H.val%c%c%c", Nextion_EndChar, Nextion_EndChar, Nextion_EndChar);
	HAL_UART_Transmit(&huart1, Send_msg, Msg_len, 10);


	if(Rx_Buffer[0] == 71) CarNum_H = Rx_Buffer[1];

	Msg_len = sprintf(Send_msg, "get globals.CarNum_L.val%c%c%c", Nextion_EndChar, Nextion_EndChar, Nextion_EndChar);
	HAL_UART_Transmit(&huart1, Send_msg, Msg_len, 10);

	if(Rx_Buffer[0] == 71) CarNum_L = Rx_Buffer[1];

	CarNum_T = (CarNum_H * 10) + CarNum_L;
	CarNum_T = CarNum_T & 0xFF;

	return CarNum_T;




}


void Display_30m_time(uint16_t milis, uint8_t secs){

	char Send_msg[20];
	uint8_t Time_String[20];
	uint8_t Msg_len = 0x00;


	Msg_len = sprintf(Time_String, "0.%d'%d s%c%c%c", secs, milis, Nextion_EndChar, Nextion_EndChar, Nextion_EndChar);

	Msg_len = sprintf(Send_msg, "t0.txt= %.8s",Time_String);
	HAL_UART_Transmit(&huart1, Send_msg, Msg_len, 10);


}

void Display_Speed(uint16_t speed){

	char Send_msg[30];
	uint8_t Msg_len = 0x00;
	float Speed_f = 0.0;

	Speed_f = speed/1000;

	Msg_len = sprintf(Send_msg, "t1.txt= %.2f km/h%c%c%c", Speed_f, Nextion_EndChar, Nextion_EndChar, Nextion_EndChar);
	HAL_UART_Transmit(&huart1, Send_msg, Msg_len, 10);


}

BEACONMODE_TypeDef Get_Device_Mode(){

	char Send_msg[35];
	uint8_t Msg_len = 0x00;
	uint8_t Device_Mode = 0xff;
	uint8_t Return_Mode = 0xff;


	HAL_UART_DMAStop(&huart1);
	HAL_UART_Receive_DMA(&huart1, Rx_Buffer, 5);
	Msg_len = sprintf(Send_msg, "get globals.Mode.val%c%c%c", Nextion_EndChar, Nextion_EndChar, Nextion_EndChar);
	HAL_UART_Transmit(&huart1, Send_msg, Msg_len, 10);
	Wait_Nextion_Resp_us(50);

	if(Rx_Buffer[0] == 0x71) {

	Device_Mode = Rx_Buffer[1];
	if(Device_Mode == 0) Return_Mode = STANDBY_MODE;
	else if(Device_Mode == 0x01) Return_Mode = RACE_MODE;
	}

	else Return_Mode = Device_Current_Mode;


return Return_Mode;


}

Nextion_Pages_TypeDef Get_Nextion_Pages(){

	char Send_msg[35];
	uint8_t Msg_len = 0x00;
	uint8_t Device_Page = 0xff;
	uint8_t i = 0x00;

	HAL_UART_DMAStop(&huart1);
	HAL_UART_Receive_DMA(&huart1, Rx_Buffer, 5);

	Msg_len = sprintf(Send_msg, "sendme%c%c%c", Nextion_EndChar, Nextion_EndChar, Nextion_EndChar);
	HAL_UART_Transmit(&huart1, Send_msg, Msg_len, 10);
	Wait_Nextion_Resp_us(50);

	for(i = 0x00; i < sizeof(Rx_Buffer); i++){
		if(Rx_Buffer[i] == 0x66) Device_Page = Rx_Buffer[i+1];
	}

	return Device_Page;

}




