
#include "Nextion_Interface.h"
#include "main.h"
#include "shared_definitions.h"
#include <stdio.h>
#include "data_types.h"



extern UART_HandleTypeDef huart1;
extern volatile BEACONMODE_TypeDef Device_Current_Mode;
extern volatile int16_t Timestamp_B1_Lastmsg_s, Timestamp_B1_Lastmsg_ms;
extern volatile uint16_t StopWatch_Counter_ms, StopWatch_Counter_secs, Stopwatch_Counter_Mins;
extern volatile uint8_t Rx_Buffer[20];
uint8_t Mem_Sensor_Status = 0x00;

BEACONMODE_TypeDef Return_Mode = 0xFF;
Nextion_Pages_TypeDef Get_Nextion_Pages();

char Send_msg[50];
uint8_t Msg_len = 0x00;

uint8_t Current_Page = 0x00, Prev_Page = 0xff;


void Wait_Nextion_Resp_us(uint32_t tim_to_wait_us)
{
	uint32_t i = 0x00;
	uint32_t ticks = 0x00;

		ticks  = 64 * tim_to_wait_us;
		for(i = 0x00; i <= ticks; i++){

		}	//empirically waits for DMA transfer to finish. Attempts by IRQ were unsuccessfull.

}


void Nextion_Init(){


	Device_Current_Mode =  STANDBY_MODE;

	Msg_len = sprintf(Send_msg, "get dp%c%c%c", Nextion_EndChar, Nextion_EndChar, Nextion_EndChar);

	HAL_UART_Transmit(&huart1, Send_msg, Msg_len , 10);


	if((Rx_Buffer[0] != 71) || (Rx_Buffer[1] != 2)){

		Msg_len = sprintf(Send_msg, "dp=2%c%c%c", Nextion_EndChar, Nextion_EndChar, Nextion_EndChar);
		HAL_UART_Transmit(&huart1, Send_msg, Msg_len, 10);

	}

}

void Update_Sensor_Status_Stdby(uint8_t sensor, uint8_t Sync, uint8_t Beam, uint8_t Battery){

	Nextion_Pages_TypeDef pg = 0x00;

	pg = Get_Nextion_Pages();

	if(pg == STANDBY_PAGE){
		switch(sensor){

		case BEACON1:

			if(Sync == 1) Msg_len = sprintf(Send_msg, "p1.pic=4%c%c%c", Nextion_EndChar, Nextion_EndChar, Nextion_EndChar);
			else if (Sync == 0) Msg_len = sprintf(Send_msg, "p1.pic=5%c%c%c", Nextion_EndChar, Nextion_EndChar, Nextion_EndChar);
			HAL_UART_Transmit(&huart1, Send_msg, Msg_len,10);



			if(Beam == NON_INTERRUPTED ) Msg_len = sprintf(Send_msg, "p2.pic=4%c%c%c", Nextion_EndChar, Nextion_EndChar, Nextion_EndChar);
			else if (Beam == INTERRUPTED) Msg_len = sprintf(Send_msg, "p2.pic=5%c%c%c", Nextion_EndChar, Nextion_EndChar, Nextion_EndChar);
			HAL_UART_Transmit(&huart1, Send_msg, Msg_len, 10);



			Msg_len = sprintf(Send_msg, "t0.txt=%c%d%c%c%c%c%c",'"', Battery,'%','"', Nextion_EndChar, Nextion_EndChar, Nextion_EndChar);
			HAL_UART_Transmit(&huart1, Send_msg, Msg_len, 10);



		break;

		case BEACON2:

			if(Sync == 1) Msg_len = sprintf(Send_msg, "p3.pic=4%c%c%c", Nextion_EndChar, Nextion_EndChar, Nextion_EndChar);
			else if (Sync == 0) Msg_len = sprintf(Send_msg, "p3.pic=5%c%c%c", Nextion_EndChar, Nextion_EndChar, Nextion_EndChar);
			HAL_UART_Transmit(&huart1, Send_msg, Msg_len, 10);

			if(Beam == NON_INTERRUPTED) Msg_len = sprintf(Send_msg, "p4.pic=4%c%c%c", Nextion_EndChar, Nextion_EndChar, Nextion_EndChar);
			else if (Beam == INTERRUPTED) Msg_len = sprintf(Send_msg, "p4.pic=5%c%c%c", Nextion_EndChar, Nextion_EndChar, Nextion_EndChar);
			HAL_UART_Transmit(&huart1, Send_msg, Msg_len, 10);

			Msg_len = sprintf(Send_msg, "t1.txt=%c%d%c%c%c%c%c",'"', Battery,'%','"', Nextion_EndChar, Nextion_EndChar, Nextion_EndChar);
			HAL_UART_Transmit(&huart1, Send_msg, Msg_len, 10);


			break;

		case BEACON3:

			if(Sync == 1) Msg_len = sprintf(Send_msg, "p5.pic=4%c%c%c", Nextion_EndChar, Nextion_EndChar, Nextion_EndChar);
			else if (Sync == 0) Msg_len = sprintf(Send_msg, "p5.pic=5%c%c%c", Nextion_EndChar, Nextion_EndChar, Nextion_EndChar);
			HAL_UART_Transmit(&huart1, Send_msg, Msg_len, 10);

			if(Beam == NON_INTERRUPTED) Msg_len = sprintf(Send_msg, "p6.pic=4%c%c%c", Nextion_EndChar, Nextion_EndChar, Nextion_EndChar);
			else if (Beam == INTERRUPTED) Msg_len = sprintf(Send_msg, "p6.pic=5%c%c%c", Nextion_EndChar, Nextion_EndChar, Nextion_EndChar);
			HAL_UART_Transmit(&huart1, Send_msg, Msg_len, 10);

			Msg_len = sprintf(Send_msg, "t2.txt=%c%d%c%c%c%c%c",'"', Battery,'%','"', Nextion_EndChar, Nextion_EndChar, Nextion_EndChar);
			HAL_UART_Transmit(&huart1, Send_msg, Msg_len, 10);


			break;

		case BEACON4:


			if(Sync == 1) Msg_len = sprintf(Send_msg, "p7.pic=4%c%c%c", Nextion_EndChar, Nextion_EndChar, Nextion_EndChar);
			else if (Sync == 0) Msg_len = sprintf(Send_msg, "p7.pic=5%c%c%c", Nextion_EndChar, Nextion_EndChar, Nextion_EndChar);
			HAL_UART_Transmit(&huart1, Send_msg, Msg_len, 10);

			if(Beam == NON_INTERRUPTED) Msg_len = sprintf(Send_msg, "p8.pic=4%c%c%c", Nextion_EndChar, Nextion_EndChar, Nextion_EndChar);
			else if (Beam == INTERRUPTED) Msg_len = sprintf(Send_msg, "p8.pic=5%c%c%c", Nextion_EndChar, Nextion_EndChar, Nextion_EndChar);
			HAL_UART_Transmit(&huart1, Send_msg, Msg_len, 10);

			Msg_len = sprintf(Send_msg, "t3.txt=%c%d%c%c%c%c%c",'"', Battery,'%','"', Nextion_EndChar, Nextion_EndChar, Nextion_EndChar);
			HAL_UART_Transmit(&huart1, Send_msg, Msg_len, 10);

			break;

		default:
		break;

		}
	}


}


void Update_Sensor_Status_Run(uint8_t Sensor, uint8_t Sync){


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

	uint8_t Recv_msg[35];


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

	uint8_t Time_String[20];


	Msg_len = sprintf(Time_String, "%d.%03d s", secs, milis, Nextion_EndChar, Nextion_EndChar, Nextion_EndChar);
	Msg_len = sprintf(Send_msg, "t0.txt=%c %s%c%c%c%c",'"', Time_String,'"',Nextion_EndChar,Nextion_EndChar,Nextion_EndChar);
	HAL_UART_Transmit(&huart1, Send_msg, Msg_len, 10);


}

void Display_30m_Started(){

	HAL_StatusTypeDef Resp = HAL_BUSY;
	Msg_len = sprintf(Send_msg, "t0.txt=%c Contagem Iniciada%c%c%c%c",'"','"',Nextion_EndChar,Nextion_EndChar,Nextion_EndChar);
	Resp = HAL_UART_Transmit(&huart1, &Send_msg, Msg_len, 10);

}


void Display_Speed(float Speed){

	Msg_len = sprintf(Send_msg, "t1.txt=%c %.3f km/h%c%c%c%c",'"',Speed,'"',Nextion_EndChar,Nextion_EndChar,Nextion_EndChar);
	HAL_UART_Transmit(&huart1, Send_msg, Msg_len, 10);

}

BEACONMODE_TypeDef Get_Device_Mode(){

	uint8_t Device_Mode = 0xff;
	uint8_t Return_Mode = 0xff;


	HAL_UART_DMAStop(&huart1);
	HAL_UART_Receive_DMA(&huart1, Rx_Buffer, 20);
	Msg_len = sprintf(Send_msg, "get globals.Mode.val%c%c%c", Nextion_EndChar, Nextion_EndChar, Nextion_EndChar);
	HAL_UART_Transmit(&huart1, Send_msg, Msg_len, 10);
	Wait_Nextion_Resp_us(1000);

	if(Rx_Buffer[0] == 0x71) {

	Device_Mode = Rx_Buffer[1];
	if(Device_Mode == 0) Return_Mode = STANDBY_MODE;
	else if(Device_Mode == 0x01) Return_Mode = RACE_MODE;
	}

	else Return_Mode = Device_Current_Mode;


return Return_Mode;


}

Nextion_Pages_TypeDef Get_Nextion_Pages(){


	uint8_t Device_Page = 0xff;
	uint8_t i = 0x00;

	HAL_UART_DMAStop(&huart1);
	HAL_UART_Receive_DMA(&huart1, Rx_Buffer, 5);

	Msg_len = sprintf(Send_msg, "sendme%c%c%c", Nextion_EndChar, Nextion_EndChar, Nextion_EndChar);
	HAL_UART_Transmit(&huart1, Send_msg, Msg_len, 10);
	Wait_Nextion_Resp_us(1000);

	for(i = 0x00; i < sizeof(Rx_Buffer); i++){
		if(Rx_Buffer[i] == 0x66) Device_Page = Rx_Buffer[i+1];
	}

	return Device_Page;

}

void Nextion_Update_Battery(uint8_t Bat_Status_perc, uint16_t Batt_Voltage_mV){


	Nextion_Pages_TypeDef Current_Page = RACE_PAGE;

	Current_Page = Get_Nextion_Pages();
	if(Current_Page == STANDBY_PAGE){
		Msg_len = sprintf(Send_msg, "t4.txt=%c%d%c%c%c%c%c",'"', Bat_Status_perc,'%','"', Nextion_EndChar, Nextion_EndChar, Nextion_EndChar);
		HAL_UART_Transmit(&huart1, Send_msg, Msg_len, 10);
	}
}

Mem_Erase_Status Nextion_Get_Mem_Erase_Status(){

	uint8_t Rx_Val = 0xff;

	HAL_UART_DMAStop(&huart1);
	HAL_UART_Receive_DMA(&huart1, Rx_Buffer, 20);


	Msg_len = sprintf(Send_msg, "get Loc_Req.val%c%c%c", Nextion_EndChar, Nextion_EndChar, Nextion_EndChar);
	HAL_UART_Transmit(&huart1, Send_msg, Msg_len, 10);
	Wait_Nextion_Resp_us(50);
	if(Rx_Buffer[0] == 0x71)
		{
		Rx_Val =  Rx_Buffer[1];
		}
	else Rx_Val = 0;

	return Rx_Val;


}

void Nextion_Update_SD_Status(uint8_t Status){

	if(Status == 0x00) Msg_len = sprintf(Send_msg, "p9.pic=13%c%c%c",Nextion_EndChar, Nextion_EndChar, Nextion_EndChar);
	else Msg_len = sprintf(Send_msg, "p9.pic=12%c%c%c",Nextion_EndChar, Nextion_EndChar, Nextion_EndChar);
	HAL_UART_Transmit(&huart1, Send_msg, Msg_len, 10);
}


void Nextion_Retunr_From_Mem_Erase(){

	Wait_Nextion_Resp_us(150000);
	Msg_len = sprintf(Send_msg, "t0.pco=16296%c%c%c",Nextion_EndChar, Nextion_EndChar, Nextion_EndChar);
	HAL_UART_Transmit(&huart1, Send_msg, Msg_len, 10);
	Msg_len = sprintf(Send_msg, "t0.txt=%cMemoria apagada!!%c%c%c%c",'"','"', Nextion_EndChar, Nextion_EndChar, Nextion_EndChar);
	HAL_UART_Transmit(&huart1, Send_msg, Msg_len, 10);
	Msg_len = sprintf(Send_msg, "p1.pic=4%c%c%c",Nextion_EndChar, Nextion_EndChar, Nextion_EndChar);
	HAL_UART_Transmit(&huart1, Send_msg, Msg_len, 10);
	Wait_Nextion_Resp_us(200000);
	Msg_len = sprintf(Send_msg, "page 2%c%c%c", Nextion_EndChar, Nextion_EndChar, Nextion_EndChar);
	HAL_UART_Transmit(&huart1, Send_msg, Msg_len, 10);


}


uint8_t Nextion_Get_Current_Display_Data_page(){

	uint8_t Rx_Val = 0xff;

	HAL_UART_DMAStop(&huart1);
	HAL_UART_Receive_DMA(&huart1, Rx_Buffer, 20);


	Msg_len = sprintf(Send_msg, "get Pag_Num.val%c%c%c", Nextion_EndChar, Nextion_EndChar, Nextion_EndChar);
	HAL_UART_Transmit(&huart1, Send_msg, Msg_len, 10);
	Wait_Nextion_Resp_us(50);
	if(Rx_Buffer[0] == 0x71)
		{
		Rx_Val =  Rx_Buffer[1];
		}
	else Rx_Val = 0;

	return Rx_Val;


}


void Nextion_Display_Mem_Data(Data_FS Data){

	uint8_t Num_Pages = 0x00, Last_Page_Lines = 0x00,offset = 0x00, Rows_To_Print = 0x00;
	uint16_t i = 0x00 ;

	Current_Page = Nextion_Get_Current_Display_Data_page();

	if(Data.Size != 0xff){

	Num_Pages = (Data.Size/8);
	Last_Page_Lines = (Data.Size%8);
	if(Last_Page_Lines != 0x00) Num_Pages++;




		Msg_len = sprintf(Send_msg, "t32.txt=%c%d/%d%c%c%c%c",'"',Current_Page,Num_Pages,'"', Nextion_EndChar, Nextion_EndChar, Nextion_EndChar);
		HAL_UART_Transmit(&huart1, Send_msg, Msg_len, 10);
		Msg_len = sprintf(Send_msg, "Max_Pag_Num.val=%d%c%c%c",Num_Pages, Nextion_EndChar, Nextion_EndChar, Nextion_EndChar);
		HAL_UART_Transmit(&huart1, Send_msg, Msg_len, 10);

		offset = (Current_Page-1)*8;
		if(Current_Page != Num_Pages) Rows_To_Print = 0x08;
		else if(Current_Page == Num_Pages) Rows_To_Print = Last_Page_Lines;


		for(i = 0; i < Rows_To_Print; i++){


			Msg_len = sprintf(Send_msg, "t%d.txt=%c%.3s%c%c%c%c",(i*4),'"',Data.Lines[i+offset].Passada,'"', Nextion_EndChar, Nextion_EndChar, Nextion_EndChar);
			HAL_UART_Transmit(&huart1, Send_msg, Msg_len, 10);

			Msg_len = sprintf(Send_msg, "t%d.txt=%c%.3s%c%c%c%c",(i*4) + 1,'"',Data.Lines[i+offset].Carro,'"', Nextion_EndChar, Nextion_EndChar, Nextion_EndChar);
			HAL_UART_Transmit(&huart1, Send_msg, Msg_len, 10);

			Msg_len = sprintf(Send_msg, "t%d.txt=%c%.6s%c%c%c%c",(i*4) + 2,'"',Data.Lines[i+offset].Aceleracao,'"', Nextion_EndChar, Nextion_EndChar, Nextion_EndChar);
			HAL_UART_Transmit(&huart1, Send_msg, Msg_len, 10);

			Msg_len = sprintf(Send_msg, "t%d.txt=%c%.6s%c%c%c%c",(i*4) + 3,'"',Data.Lines[i+offset].Velocidade,'"', Nextion_EndChar, Nextion_EndChar, Nextion_EndChar);
			HAL_UART_Transmit(&huart1, Send_msg, Msg_len, 10);

		}
		if(Rows_To_Print < 8){

			for(i = Rows_To_Print; i < 8; i++){

				Msg_len = sprintf(Send_msg, "t%d.txt=%c%.3s%c%c%c%c",(i*4),'"'," -- ",'"', Nextion_EndChar, Nextion_EndChar, Nextion_EndChar);
				HAL_UART_Transmit(&huart1, Send_msg, Msg_len, 10);

				Msg_len = sprintf(Send_msg, "t%d.txt=%c%.3s%c%c%c%c",(i*4) + 1,'"', " -- " ,'"', Nextion_EndChar, Nextion_EndChar, Nextion_EndChar);
				HAL_UART_Transmit(&huart1, Send_msg, Msg_len, 10);

				Msg_len = sprintf(Send_msg, "t%d.txt=%c%.6s%c%c%c%c",(i*4) + 2,'"'," -- ",'"', Nextion_EndChar, Nextion_EndChar, Nextion_EndChar);
				HAL_UART_Transmit(&huart1, Send_msg, Msg_len, 10);

				Msg_len = sprintf(Send_msg, "t%d.txt=%c%.4s%c%c%c%c",(i*4) + 3,'"'," -- ",'"', Nextion_EndChar, Nextion_EndChar, Nextion_EndChar);
				HAL_UART_Transmit(&huart1, Send_msg, Msg_len, 10);

				}
		}

	}

	else{

			for(i = 0x00; i < 1; i++){

				Msg_len = sprintf(Send_msg, "t%d.txt=%c%.5s%c%c%c%c",(i*4),'"',"Falha",'"', Nextion_EndChar, Nextion_EndChar, Nextion_EndChar);
				HAL_UART_Transmit(&huart1, Send_msg, Msg_len, 10);

				Msg_len = sprintf(Send_msg, "t%d.txt=%c%.5s%c%c%c%c",(i*4) + 1,'"', " no  " ,'"', Nextion_EndChar, Nextion_EndChar, Nextion_EndChar);
				HAL_UART_Transmit(&huart1, Send_msg, Msg_len, 10);

				Msg_len = sprintf(Send_msg, "t%d.txt=%c%.6s%c%c%c%c",(i*4) + 2,'"',"cartao",'"', Nextion_EndChar, Nextion_EndChar, Nextion_EndChar);
				HAL_UART_Transmit(&huart1, Send_msg, Msg_len, 10);

				Msg_len = sprintf(Send_msg, "t%d.txt=%c%.4s%c%c%c%c",(i*4) + 3,'"'," SD ",'"', Nextion_EndChar, Nextion_EndChar, Nextion_EndChar);
				HAL_UART_Transmit(&huart1, Send_msg, Msg_len, 10);


	}

	}

}

uint8_t Nextion_Get_Cancel_OTA_Req(){

	uint8_t Rx_Val = 0xff;

	HAL_UART_DMAStop(&huart1);
	HAL_UART_Receive_DMA(&huart1, Rx_Buffer, 20);

	Msg_len = sprintf(Send_msg, "get Return_Req.val%c%c%c", Nextion_EndChar, Nextion_EndChar, Nextion_EndChar);
	HAL_UART_Transmit(&huart1, Send_msg, Msg_len, 10);
	Wait_Nextion_Resp_us(100);
	if(Rx_Buffer[0] == 0x71)
		{
		Rx_Val =  Rx_Buffer[1];
		}
	else Rx_Val = 0;

	return Rx_Val;
}

uint8_t Nextion_Get_Start_OTA_Req(){

	uint8_t Rx_Val = 0xff;

	HAL_UART_DMAStop(&huart1);
	HAL_UART_Receive_DMA(&huart1, Rx_Buffer, 20);

	Msg_len = sprintf(Send_msg, "get Start_Req.val%c%c%c", Nextion_EndChar, Nextion_EndChar, Nextion_EndChar);
	HAL_UART_Transmit(&huart1, Send_msg, Msg_len, 10);
	Wait_Nextion_Resp_us(100);
	if(Rx_Buffer[0] == 0x71)
		{
		Rx_Val =  Rx_Buffer[1];
		}
	else Rx_Val = 0;

	return Rx_Val;
}

void Nextion_Update_OTA_Sync(uint8_t Status){

	if(Status == 0x01) Msg_len = sprintf(Send_msg, "p3.pic=4%c%c%c",Nextion_EndChar, Nextion_EndChar, Nextion_EndChar);
	else Msg_len = sprintf(Send_msg, "p3.pic=5%c%c%c",Nextion_EndChar, Nextion_EndChar, Nextion_EndChar);
	HAL_UART_Transmit(&huart1, Send_msg, Msg_len, 10);
}

void Nextion_Update_OTA_Progress(uint8_t perc){

	if(perc >= 100) perc = 100;

	Msg_len = sprintf(Send_msg, "j0.val=%d%c%c%c",perc,Nextion_EndChar, Nextion_EndChar, Nextion_EndChar);
	HAL_UART_Transmit(&huart1, Send_msg, Msg_len, 10);

	if(perc == 100){
		Msg_len = sprintf(Send_msg, "p1.pic=4%c%c%c",Nextion_EndChar, Nextion_EndChar, Nextion_EndChar);
		HAL_UART_Transmit(&huart1, Send_msg, Msg_len, 10);
	}

}

void Clear_Nextion_NewPage(Nextion_Pages_TypeDef Page){

	if(Page == STANDBY_PAGE){
		Msg_len = sprintf(Send_msg, "dp=2%c%c%c",Nextion_EndChar, Nextion_EndChar, Nextion_EndChar);
		HAL_UART_Transmit(&huart1, Send_msg, Msg_len, 10);
	}
	else if(Page == RACE_PAGE){

		Msg_len = sprintf(Send_msg, "dp=4%c%c%c",Nextion_EndChar, Nextion_EndChar, Nextion_EndChar);
		HAL_UART_Transmit(&huart1, Send_msg, Msg_len, 10);
	}
}

uint8_t Get_Nextion_Request_Page(){
	uint8_t rsp = 0x00;

		HAL_UART_DMAStop(&huart1);
		HAL_UART_Receive_DMA(&huart1, Rx_Buffer, 20);
		Msg_len = sprintf(Send_msg, "get globals.Request_Page.val%c%c%c", Nextion_EndChar, Nextion_EndChar, Nextion_EndChar);
		HAL_UART_Transmit(&huart1, Send_msg, Msg_len, 10);
		Wait_Nextion_Resp_us(1000);

		if(Rx_Buffer[0] == 0x71) {
			rsp = Rx_Buffer[1];
		}
	return rsp;

}

void Nextion_Set_Page(Nextion_Pages_TypeDef Pg){

	HAL_UART_Abort(&huart1);

	Msg_len = sprintf(Send_msg, "dp=%d%c%c%c%c",Pg,Nextion_EndChar, Nextion_EndChar, Nextion_EndChar);
	HAL_UART_Transmit(&huart1, Send_msg, Msg_len, 10);
	Msg_len = sprintf(Send_msg, "globals.Request_Page.val=0%c%c%c", Nextion_EndChar, Nextion_EndChar, Nextion_EndChar);
	HAL_UART_Transmit(&huart1, Send_msg, Msg_len, 10);

}



void Nextion_Debug_Stopwatch_Send(){

	Msg_len = sprintf(Send_msg, "t2.txt=%c%d.%d%c%c%c%c%c",'"',Timestamp_B1_Lastmsg_s,Timestamp_B1_Lastmsg_ms,'"', Nextion_EndChar, Nextion_EndChar, Nextion_EndChar);
	HAL_UART_Transmit(&huart1, Send_msg, Msg_len, 10);

}

uint8_t Nextion_Get_Car_Num(){

	uint8_t Response = 0x00;

	HAL_UART_DMAStop(&huart1);
	HAL_UART_Receive_DMA(&huart1, Rx_Buffer, 20);
	Msg_len = sprintf(Send_msg, "get globals.CarNum_H.val%c%c%c", Nextion_EndChar, Nextion_EndChar, Nextion_EndChar);
	HAL_UART_Transmit(&huart1, Send_msg, Msg_len, 10);
	Wait_Nextion_Resp_us(1000);

	if(Rx_Buffer[0] == 0x71) Response += Rx_Buffer[1]*10;

	HAL_UART_DMAStop(&huart1);
	HAL_UART_Receive_DMA(&huart1, Rx_Buffer, 20);
	Msg_len = sprintf(Send_msg, "get globals.CarNum_L.val%c%c%c", Nextion_EndChar, Nextion_EndChar, Nextion_EndChar);
	HAL_UART_Transmit(&huart1, Send_msg, Msg_len, 10);
	Wait_Nextion_Resp_us(1000);

	if(Rx_Buffer[0] == 0x71) Response += Rx_Buffer[1];

return Response;
}

void Nextion_SD_Write_Confirmation_Page(){

	Msg_len = sprintf(Send_msg, "dp=10%c%c%c",Nextion_EndChar, Nextion_EndChar, Nextion_EndChar);
	HAL_UART_Transmit(&huart1, Send_msg, Msg_len, 10);

}

uint8_t Nextion_Get_Save_File_Req(){

	uint8_t Rx_Val = 0xff;

	HAL_UART_DMAStop(&huart1);
	HAL_UART_Receive_DMA(&huart1, Rx_Buffer, 20);

	Msg_len = sprintf(Send_msg, "get globals.Save_File.val%c%c%c", Nextion_EndChar, Nextion_EndChar, Nextion_EndChar);
	HAL_UART_Transmit(&huart1, Send_msg, Msg_len, 10);
	Wait_Nextion_Resp_us(100);
	if(Rx_Buffer[0] == 0x71)
		{
		Rx_Val =  Rx_Buffer[1];
		}
	else Rx_Val = 0;

	return Rx_Val;
}





