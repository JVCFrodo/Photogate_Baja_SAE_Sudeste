/*
 * Addition_Functions.c
 *
 *  Created on: Mar 19, 2024
 *      Author: CAJ1SBC
 */
#include "main.h"
#include "aditional_functions_master.h"

extern ADC_HandleTypeDef hadc1;
extern TIM_HandleTypeDef htim11;
extern TIM_HandleTypeDef htim10;
extern UART_HandleTypeDef huart1;
extern uint8_t Rx_Buffer[20];

extern char Payload_Log[32];
extern uint8_t RX_Payload_Flag, Dummy_Payload_Len;
extern uint16_t Analog_read, RWbytecounter, TotalSize1, FreeSpace1, FreClusters1;
extern FATFS fs, Dummy1;

extern FIL fil;
extern  BEACONMODE_TypeDef Device_Current_Mode;

uint16_t Filter_Array[100];

void On_Valid_NRF24_Msg();
void Race_Mode_StateMachine_Update(BEACONMESSAGE_TypeDef *Message);

uint32_t Start_Time_Full_ms = 0x00, End_Time_Full_ms = 0x00, Measured_Time_Full_ms = 0x00;
uint16_t Meastime_ms = 0x00, Start_Time_Speed_Calc_ms = 0x00, End_Time_Speed_Calc_ms = 0x00, Measured_Time_Speed_Calc_ms = 0x00,
		          Delta_msgs = 0xff, Message_Milis_Calc = 0x00;
uint8_t Meastime_secs = 0x00, Finished_30m_Flag = 0x00, Finished_Speed_Flag = 0x00;

float Vehicle_Speed = 0.0;








uint16_t Calc_Batt_Perc(uint8_t mode) {

	static float Conv_Factor_mv = 0.8083028, Res_Div_Ratio = 2;
	uint8_t Filtering_Window_Size = 20;
	uint32_t Avg_Aux = 0x00;
	uint16_t Analog_Read_Current = 0x00, Voltage_mv = 0x00, V_Bat_Calc = 0x00,
			Analog_Read_Avg = 0x00, Diff = 0x00, Min_Diff = 0xFFFF;
	uint8_t i = 0x00, Table_Size = 50, Index = 0x00;

	Analog_Read_Current = Analog_read;

	for (i = 0x00; i <= (Filtering_Window_Size - 2); i++) {
		Filter_Array[i] = Filter_Array[i + 1];
	}
	Filter_Array[Filtering_Window_Size - 1] = Analog_Read_Current;
	for (i = 0x00; i <= Filtering_Window_Size; i++)
		Avg_Aux += Filter_Array[i];

	Analog_Read_Avg = Avg_Aux / Filtering_Window_Size;
	Voltage_mv = (Analog_Read_Avg * Conv_Factor_mv);
	V_Bat_Calc = (Voltage_mv * Res_Div_Ratio);

	for (uint8_t i = 0; i < Table_Size; i++) {
		Diff = abs(V_Bat_Calc - V_Bat_mv_table[i]);
		if (Diff < Min_Diff) {
			Min_Diff = Diff;
			Index = i;
		}
	}
	if (mode == 0x00)
		return (Index * 2); //Each position of the table represents 2% of battery
	else if (mode == 0x01)
		return (V_Bat_Calc); //Each position of the table represents 2% of battery

	return 0;

}

void Initialize_Batt_Avg_Calc() {

	uint8_t i = 0x00;

	for (i = 0x00; i <= 10; i++)
		Filter_Array[i] = 0x00;

}

void Erase_SD_Card() {

	FRESULT SD_Operation_Result = FR_DISK_ERR;

	HAL_TIM_Base_Stop_IT(&htim11);
	HAL_TIM_Base_Stop_IT(&htim10);

	HAL_UART_DMAStop(&huart1);

	HAL_ADC_Stop_DMA(&hadc1);
	HAL_NVIC_DisableIRQ(EXTI2_IRQn);

	SD_Operation_Result = f_open(&fil, "ResultadosAVF.txt",
			FA_WRITE | FA_READ | FA_CREATE_ALWAYS);
	f_close(&fil);

	HAL_UART_Receive_DMA(&huart1, Rx_Buffer, 5);
	HAL_ADC_Start_DMA(&hadc1, &Analog_read, 1);

	HAL_TIM_Base_Start_IT(&htim10);
	HAL_NVIC_EnableIRQ(EXTI2_IRQn);
}

Data_FS Read_SD_Data() {

	Data_FS Filestruct;
	FRESULT SD_Operation_Result1 = FR_DISK_ERR;
	char Rbuffer[1000];
	uint16_t Read_Bytes_Count = 0x00;
	char Temp_Format_Str[10] = " ";
	uint8_t Comma_Count = 0x00, Curr_Line = 0x00, Param_Size = 0x00,
			Param_Max_Size = 0x00;
	;
	uint16_t i = 0x00, j = 0x00, Curr_Comma_Pos = 0x00, Prev_Comma_Pos = 0x00;

	SD_Operation_Result1 = f_open(&fil, "ResultadosAVF.txt",
			FA_READ | FA_OPEN_EXISTING);
	if (SD_Operation_Result1 != FR_OK) {
		return;
	}

	SD_Operation_Result1 = f_read(&fil, Rbuffer, f_size(&fil),
			&Read_Bytes_Count);
	if (SD_Operation_Result1 == FR_OK && Read_Bytes_Count > 0) {
		for (i = 0x00; i <= Read_Bytes_Count; i++) {
			if (Rbuffer[i] == ',' || Rbuffer[i] == '\r') {
				Comma_Count++;
				Curr_Comma_Pos = i;

				switch (Comma_Count) {
				case 1:
					if (Prev_Comma_Pos == 0x00) {
						Param_Size = Curr_Comma_Pos - Prev_Comma_Pos;
						if (Param_Size == 0x01)
							sprintf(Temp_Format_Str, "  %c", (char) Rbuffer[0]);
					} else {
						Param_Size = Curr_Comma_Pos - (Prev_Comma_Pos + 1);
						Param_Max_Size = sizeof(Filestruct.Lines->Passada) - 1;
						for (j = 0x00; j < Param_Size; j++) {
							Temp_Format_Str[Param_Max_Size - j] =
									Rbuffer[Curr_Comma_Pos - (j + 1)];
						}
					}

					memcpy(Filestruct.Lines[Curr_Line].Passada, Temp_Format_Str,
							3);
					Prev_Comma_Pos = Curr_Comma_Pos;
					memset(Temp_Format_Str, ' ', 10);
					break;

				case 2:
					Param_Size = Curr_Comma_Pos - (Prev_Comma_Pos + 1);
					Param_Max_Size = sizeof(Filestruct.Lines->Carro) - 1;

					for (j = 0x00; j < Param_Size; j++) {
						Temp_Format_Str[Param_Max_Size - j] =
								Rbuffer[Curr_Comma_Pos - (j + 1)];
					}
					memcpy(Filestruct.Lines[Curr_Line].Carro, Temp_Format_Str,
							3);
					Prev_Comma_Pos = Curr_Comma_Pos;
					memset(Temp_Format_Str, ' ', 10);
					break;

				case 3:
					Param_Size = Curr_Comma_Pos - (Prev_Comma_Pos + 1);
					Param_Max_Size = sizeof(Filestruct.Lines->Aceleracao) - 1;

					for (j = 0x00; j < Param_Size; j++) {
						Temp_Format_Str[Param_Max_Size - j] =
								Rbuffer[Curr_Comma_Pos - (j + 1)];
					}
					memcpy(Filestruct.Lines[Curr_Line].Aceleracao,
							Temp_Format_Str, 6);
					Prev_Comma_Pos = Curr_Comma_Pos;
					memset(Temp_Format_Str, ' ', 10);
					break;

				case 4:
					Param_Size = Curr_Comma_Pos - (Prev_Comma_Pos + 1);
					Param_Max_Size = sizeof(Filestruct.Lines->Velocidade) - 1;

					for (j = 0x00; j < Param_Size; j++) {
						Temp_Format_Str[Param_Max_Size - j] =
								Rbuffer[Curr_Comma_Pos - (j + 1)];
					}
					memcpy(Filestruct.Lines[Curr_Line].Velocidade,
							Temp_Format_Str, 6);
					Prev_Comma_Pos = Curr_Comma_Pos;
					memset(Temp_Format_Str, ' ', 10);
					break;

				default:
					break;
				}
			}

			else if (Rbuffer[i] == '\n') {
				Curr_Line++;
				Prev_Comma_Pos = i;
				Comma_Count = 0x00;
				Curr_Comma_Pos = 0x00;
			}
		}
		Filestruct.Size = Curr_Line;
	} else
		Filestruct.Size = 0;

	f_close(&fil);

	return Filestruct;

}

void OTA_Log_Routine(Data_FS Data) { //For some reason, strategy using while and calling this function only once would work at all. Must be checked

	uint8_t Num_Transmits = 0x00, Progress_Show_Step = 0x00,
			Cursor_Pos = 0x00;
	char Dummy_Str[31];
	uint16_t i = 0x00;
	uint8_t Cancel_req = 0x00, Start_Req = 0x00;

	Num_Transmits = Data.Size;
	Progress_Show_Step = (ceil(100 / Num_Transmits) + 1);

	while (Cancel_req == 0x00 && RX_Payload_Flag == 0x00) {
		Cancel_req = Nextion_Get_Cancel_OTA_Req();
		Wait_Nextion_Resp_us(500);
	}
	if (Cancel_req == 0x01)
		return;

	else if (RX_Payload_Flag == 0x01 && strstr(Payload_Log, "Handshake Request") != NULL) {
		RX_Payload_Flag = 0x00;

		Dummy_Str[0] = 12;
		memcpy(Dummy_Str + 1, "Handshake Ok", 12);
		nRF24_SetOperationalMode(nRF24_MODE_TX);
		nRF24_FlushRX();
		nRF24_FlushTX();
		nRF24_ClearIRQFlags();
		RX_Payload_Flag = 0x00;
		nRF24_TransmitPacket(Dummy_Str, 32);
		nRF24_SetOperationalMode(nRF24_MODE_RX);
		Nextion_Update_OTA_Sync(1);

		while (Start_Req == 0x00 && Cancel_req == 0x00) {
			Start_Req = Nextion_Get_Start_OTA_Req();
			Cancel_req = Nextion_Get_Cancel_OTA_Req();
			Wait_Nextion_Resp_us(500);
		}
		if (Cancel_req == 0x01)
			return;
		else if (Start_Req == 0x01) {
			RX_Payload_Flag = 0x00;
			Dummy_Str[0] = 12;
			memcpy(Dummy_Str + 1, "Data Size ", 10);
			sprintf(Dummy_Str + 11, "%02d", Data.Size);

			nRF24_SetOperationalMode(nRF24_MODE_TX);
			nRF24_FlushRX();
			nRF24_FlushTX();
			nRF24_ClearIRQFlags();
			RX_Payload_Flag = 0x00;
			nRF24_TransmitPacket(Dummy_Str, 32);
			nRF24_SetOperationalMode(nRF24_MODE_RX);

			while ((RX_Payload_Flag == 0x00) && (Cancel_req == 0x00)) {
				Cancel_req = Nextion_Get_Cancel_OTA_Req();
				Wait_Nextion_Resp_us(500);
			}

			Cursor_Pos = 0x00;
			for (i = 0x00; i < Data.Size; i++) {

				memcpy(Dummy_Str + Cursor_Pos, &Data.Lines[i].Passada, 3);
				Cursor_Pos += 3;
				memcpy(Dummy_Str + Cursor_Pos, ",", 1);
				Cursor_Pos += 1;
				memcpy(Dummy_Str + Cursor_Pos, &Data.Lines[i].Carro, 3);
				Cursor_Pos += 3;
				memcpy(Dummy_Str + Cursor_Pos, ",", 1);
				Cursor_Pos += 1;
				memcpy(Dummy_Str + Cursor_Pos, &Data.Lines[i].Aceleracao, 6);
				Cursor_Pos += 6;
				memcpy(Dummy_Str + Cursor_Pos, ",", 1);
				Cursor_Pos += 1;
				memcpy(Dummy_Str + Cursor_Pos, &Data.Lines[i].Velocidade, 6);
				Cursor_Pos += 6;

				Payload_Log[0] = Cursor_Pos;
				memcpy(Payload_Log + 1, Dummy_Str, Cursor_Pos);

				nRF24_SetOperationalMode(nRF24_MODE_TX);
				RX_Payload_Flag = 0x00;
				nRF24_TransmitPacket(Payload_Log, 32);
				nRF24_SetOperationalMode(nRF24_MODE_RX);

				Cursor_Pos = 0x00;

				while ((RX_Payload_Flag == 0x00) && (Cancel_req == 0x00)) {
					Cancel_req = Nextion_Get_Cancel_OTA_Req();
					nRF24_SetOperationalMode(nRF24_MODE_RX);
					Wait_Nextion_Resp_us(1000);
				}
				if (strstr(Payload_Log, "Line Ok") != NULL) {
					RX_Payload_Flag = 0x00;
					Nextion_Update_OTA_Progress((i + 1) * Progress_Show_Step);
					memcpy(Payload_Log, "", 32);
				} else
					i--;
				Cancel_req = Nextion_Get_Cancel_OTA_Req();
				if (Cancel_req == 0x01)
					return;
			}
			Wait_Nextion_Resp_us(200000);
		}
	}
}

void Store_Data_SD_30m(uint16_t Time_ms) {

	Data_FS Actual_Data;
	uint8_t Numlines = 0x00, len = 0x00, Time_s = 0x00,
			DummWC = 0x00, Car_Num = 0x00;
	uint16_t Time_ms_format = 0x00;
	char Write_Payload[20];
	FRESULT SD_Operation_Result1 = FR_DISK_ERR;

	Time_s = ceil(Time_ms / 1000);
	Time_ms_format = Time_ms % 1000;

	Actual_Data = Read_SD_Data();
	Numlines = Actual_Data.Size + 1;

	Car_Num = Nextion_Get_Car_Num();

	len = sprintf(Write_Payload, "%d,%d,%d.%d,--\r\n", Numlines, Car_Num,
			Time_s, Time_ms_format);

	SD_Operation_Result1 = f_open(&fil, "ResultadosAVF.txt",
			FA_OPEN_EXISTING | FA_WRITE);
	SD_Operation_Result1 = f_lseek(&fil, f_size(&fil)); // Move The File Pointer To The EOF (End-Of-File)

	SD_Operation_Result1 = f_write(&fil, Write_Payload, len, &DummWC);
	SD_Operation_Result1 = f_close(&fil);

	Nextion_SD_Write_Confirmation_Page();

	Finished_30m_Flag = 0x00;

}

void Store_Data_SD_All(uint16_t Time_ms, float Speed) {

	Data_FS Actual_Data;
	uint8_t Numlines = 0x00, len = 0x00, Time_s = 0x00, DummWC = 0x00, Car_Num =
			0x00;
	uint16_t Time_ms_format = 0x00;
	char Write_Payload[20];
	FRESULT SD_Operation_Result1 = FR_DISK_ERR;


	Time_s = ceil(Time_ms / 1000);
	Time_ms_format = Time_ms % 1000;

	Actual_Data = Read_SD_Data();
	Numlines = Actual_Data.Size + 1;

	Car_Num = Nextion_Get_Car_Num();

	len = sprintf(Write_Payload, "%d,%d,%d.%d,%.3f\r\n", Numlines, Car_Num,
			Time_s, Time_ms_format, Speed);

	SD_Operation_Result1 = f_open(&fil, "ResultadosAVF.txt",
			FA_OPEN_EXISTING | FA_WRITE);
	SD_Operation_Result1 = f_lseek(&fil, f_size(&fil)); // Move The File Pointer To The EOF (End-Of-File)

	SD_Operation_Result1 = f_write(&fil, Write_Payload, len, &DummWC);
	SD_Operation_Result1 = f_close(&fil);

	Nextion_SD_Write_Confirmation_Page();

	Finished_Speed_Flag = 0x00;

}

void Race_Mode_StateMachine_Update(BEACONMESSAGE_TypeDef *PMessage){

	extern uint16_t Timestamp_B1_Lastmsg_s, Timestamp_B1_Lastmsg_ms, Timestamp_B2_Lastmsg_s, Timestamp_B2_Lastmsg_ms,
					Timestamp_B3_Lastmsg_s, Timestamp_B3_Lastmsg_ms, Timestamp_B4_Lastmsg_s, Timestamp_B4_Lastmsg_ms;

    extern volatile uint16_t Msg_Count_Prev_B1, Msg_Count_Prev_B2, Msg_Count_Prev_B3, Msg_Count_Prev_B4;
	extern uint8_t Beacon_Blocker;

	const float dist = BEACON_DISTANCE;

	BEACONMESSAGE_TypeDef Message;

	Message = *PMessage;


	switch (Message.Beacon_Id){

		case BEACON1:
			if (Beacon_Blocker == 0x00){
				Delta_msgs = ((Message.Msg_Counter_H<<8)+ Message.Msg_Counter_L) - Msg_Count_Prev_B1;
				Message_Milis_Calc = ((Message.Time_MilisH << 8) + Message.Time_MilisL);

				Start_Time_Full_ms = (Timestamp_B1_Lastmsg_s*1000) + Timestamp_B1_Lastmsg_ms;
				Start_Time_Full_ms += (Delta_msgs*1000) + Message_Milis_Calc;
				Display_30m_Started();

				Beacon_Blocker = 0x01;

				}
			break;


		case BEACON2:
			if (Beacon_Blocker == 0x01){
				Delta_msgs = ((Message.Msg_Counter_H<<8)+ Message.Msg_Counter_L) - Msg_Count_Prev_B2;
				Message_Milis_Calc = (Message.Time_MilisH << 8) + Message.Time_MilisL;

				End_Time_Full_ms = (Timestamp_B2_Lastmsg_s*1000) + Timestamp_B2_Lastmsg_ms;
				End_Time_Full_ms += (Delta_msgs*1000) + Message_Milis_Calc;

				Measured_Time_Full_ms = End_Time_Full_ms - Start_Time_Full_ms;

				Meastime_secs = ceil(Measured_Time_Full_ms/1000);
				Meastime_ms = Measured_Time_Full_ms%1000;
				Display_30m_time(Meastime_ms, Meastime_secs);


				Beacon_Blocker = 0x02;
				Finished_30m_Flag = 0x01;
				}
		break;

		case BEACON3:

			if (Beacon_Blocker == 0x02){
				Delta_msgs = ((Message.Msg_Counter_H<<8)+ Message.Msg_Counter_L) - Msg_Count_Prev_B3;
				Message_Milis_Calc = ((Message.Time_MilisH << 8) + Message.Time_MilisL);

				Start_Time_Speed_Calc_ms = (Timestamp_B3_Lastmsg_s*1000) + Timestamp_B3_Lastmsg_ms;
				Start_Time_Speed_Calc_ms += (Delta_msgs*1000) + Message_Milis_Calc;

				Beacon_Blocker = 0x03;
				}
		break;


		case BEACON4:
			char Send_msg_tmp[50];
			uint8_t msg_len_tmp = 0x00;

			if (Beacon_Blocker == 0x03){


				Delta_msgs = ((Message.Msg_Counter_H<<8)+ Message.Msg_Counter_L) - Msg_Count_Prev_B4;
				Message_Milis_Calc = (Message.Time_MilisH << 8) + Message.Time_MilisL;

				End_Time_Speed_Calc_ms = (Timestamp_B4_Lastmsg_s*1000) + Timestamp_B4_Lastmsg_ms;
				End_Time_Speed_Calc_ms += (Delta_msgs*1000) + Message_Milis_Calc;

				Measured_Time_Speed_Calc_ms = End_Time_Speed_Calc_ms - Start_Time_Speed_Calc_ms;

				Vehicle_Speed = Delta_msgs + (float)(Measured_Time_Speed_Calc_ms/1000.0);
				Vehicle_Speed = dist/Vehicle_Speed;
				Vehicle_Speed = (Vehicle_Speed*3.6);

				msg_len_tmp = sprintf(Send_msg_tmp, "t0.txt=%c%d ms%c%c%c%c",'"', Measured_Time_Speed_Calc_ms,'"',Nextion_EndChar,Nextion_EndChar,Nextion_EndChar);
				HAL_UART_Transmit(&huart1, Send_msg_tmp, msg_len_tmp, 10);

				Display_Speed(Vehicle_Speed);

				Finished_Speed_Flag = 0x01;

				Beacon_Blocker = 0x04;
				}
		break;

		default:
			break;
	}
}





void On_Valid_NRF24_Msg(){

	extern BEACONMODE_TypeDef Device_Current_Mode;

	BEACONMESSAGE_TypeDef Message = {0};
	BEACONMESSAGE_TypeDef *PMessage = {0};
	uint8_t Msg_Len = 0x00;
	Nextion_Pages_TypeDef Nextion_Page = 0xff;

	if(Device_Current_Mode == STANDBY_MODE || Device_Current_Mode == RACE_MODE){
		nRF24_ReadPayload(&Message, &Msg_Len);
		Nextion_Page = Get_Nextion_Pages();

		if(Nextion_Page == STANDBY_PAGE || Nextion_Page == RACE_PAGE){

			switch(Device_Current_Mode){

				case STANDBY_MODE:

					Beacon_Sync_Calcs(&Message);

					if(Message.Beacon_Mode != STANDBY_MODE) Update_Sensor_Status_Stdby(Message.Beacon_Id, 0, Message.Sensor_status, Message.Battery_Percentage);
					else Update_Sensor_Status_Stdby(Message.Beacon_Id, 1, Message.Sensor_status, Message.Battery_Percentage);

					break;

				case RACE_MODE:

					if(Message.Beacon_Mode == RACE_MODE) {
						if(Message.Sensor_status == INTERRUPTED){
							PMessage = &Message;
							Race_Mode_StateMachine_Update(PMessage); //If Everything is OK, updates the state machine status.
						}

						else if(Message.Sensor_status == NON_INTERRUPTED){
								Beacon_Sync_Calcs(&Message);
								Update_Sensor_Status_Run(Message.Beacon_Id, 1); //At Run Page, indicates that the sensor is on the same status as the master (RACE)
							}
					}

					else if(Message.Beacon_Mode != RACE_MODE) {
							Update_Sensor_Status_Run(Message.Beacon_Id, 0); //At Run Page, indicates that the sensor is not on the same status as the master (STANDBY instead of RACE)
							RF_Transmit_Config_MSG(RACE_MODE); // Sends a configuration message in order to sync the modules
						}
					break;

				default:
					break;
			}
		}

		if(Message.Beacon_Mode != Device_Current_Mode) RF_Transmit_Config_MSG(Device_Current_Mode);

}
	//OTA Report Mode. Configures NRF24 to communicate with the python script executes it.
	else if (Device_Current_Mode == OTA_REPORT_MODE){
		Nextion_Page = Get_Nextion_Pages();
		if(Nextion_Page == OTA_REPORT_PAGE){
			nRF24_ReadPayload(Payload_Log, &Dummy_Payload_Len);
			RX_Payload_Flag = 0x01;
		}
	}
}

void Regular_Nextion_Updates(){


	Nextion_Pages_TypeDef Nextion_Page = 0xff;
	uint8_t Battery_Status_Perc = 0x00, Request_To_Save = 0x00;
	uint16_t Batt_Voltage_mv = 0x00;
    Data_FS SD_Data;
    extern FRESULT SD_Operation_Result;


	Nextion_Page = Get_Nextion_Pages();

	switch(Nextion_Page){

	case STANDBY_PAGE:
		Check_Apply_OPmode_Change();
		WDT_Calcs_Updates(Nextion_Page);

		Nextion_Update_SD_Status(SD_Operation_Result);

		Battery_Status_Perc = Calc_Batt_Perc(0);
		//Batt_Voltage_mv = Calc_Batt_Perc(1);
		Nextion_Update_Battery(Battery_Status_Perc, 0);


		break;

	case RACE_PAGE:
		Check_Apply_OPmode_Change();
		WDT_Calcs_Updates(Nextion_Page);
		Request_To_Save = Nextion_Get_Save_File_Req();
		if((Finished_30m_Flag == 0x01) && (Request_To_Save == 1) && (Finished_Speed_Flag == 0)) Store_Data_SD_30m(Measured_Time_Full_ms);
		if((Finished_30m_Flag == 0x01) && (Request_To_Save == 1) && (Finished_Speed_Flag == 1)) Store_Data_SD_All(Measured_Time_Full_ms, Vehicle_Speed);

		break;



	case LOCAL_REPORT_PAGE:

		memset(&SD_Data, 0x00, sizeof(SD_Data));
		SD_Data = Read_SD_Data();
		if(SD_Data.Size > 0)Nextion_Display_Mem_Data(SD_Data);

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




