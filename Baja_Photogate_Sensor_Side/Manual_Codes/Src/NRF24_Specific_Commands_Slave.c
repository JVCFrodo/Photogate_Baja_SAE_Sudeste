/*
 * NRF24_Specific_Commands.c
 *
 *  Created on: Mar 18, 2024
 *      Author: CAJ1SBC
 */

#include "hw_dependencies.h"
#include "shared_definitions.h"
#include "nrf24.h"
#include "definitions.h"
#include "main.h"

#define HEX_CHARS      "0123456789ABCDEF"

nRF24_RXResult pipe;

void NRF_24_Slave_Init();

void RF_Transmit_Alive_MSG();
void RF_Transmit_Trigger_MSG();
void RF_Read_Settings_Msg(BEACONMODE_TypeDef Received_Mode);

extern uint8_t Battery_Actual_Val;
extern volatile uint16_t Message_Counter, Frozen_Timer_ms;
extern volatile uint16_t StopWatch_Counter_1ms, StopWatch_Counter_100ms;
extern volatile SENSORSTATS_TypeDef Sensor_Status_Act;
extern BEACONMODE_TypeDef Device_Current_Mode;


// Helpers for transmit mode demo

// Timeout counter (depends on the CPU speed)
// Used for not stuck waiting for IRQ
#define nRF24_WAIT_TIMEOUT         (uint32_t)0x000FFFF

// Result of packet transmission
/*
 typedef enum {
 nRF24_TX_ERROR  = (uint8_t)0x00, // Unknown error
 nRF24_TX_SUCCESS,                // Packet has been transmitted successfully
 nRF24_TX_TIMEOUT,                // It was timeout during packet transmit
 nRF24_TX_MAXRT                   // Transmit failed with maximum auto retransmit count
 } nRF24_TXResult;
 */

nRF24_TXResult tx_res;

// Function to transmit data packet
// input:
//   pBuf - pointer to the buffer with data to transmit
//   length - length of the data buffer in bytes
// return: one of nRF24_TX_xx values
nRF24_TXResult nRF24_TransmitPacket(uint8_t *pBuf, uint8_t length) {
	volatile uint32_t wait = nRF24_WAIT_TIMEOUT;
	uint8_t status;

	// Deassert the CE pin (in case if it still high)
	nRF24_CE_L();

	// Transfer a data from the specified buffer to the TX FIFO
	nRF24_WritePayload(pBuf, length);

	// Start a transmission by asserting CE pin (must be held at least 10us)
	nRF24_CE_H();

	// Poll the transceiver status register until one of the following flags will be set:
	//   TX_DS  - means the packet has been transmitted
	//   MAX_RT - means the maximum number of TX retransmits happened
	// note: this solution is far from perfect, better to use IRQ instead of polling the status


	do {
		status = nRF24_GetStatus();
		if (status & (nRF24_FLAG_TX_DS | nRF24_FLAG_MAX_RT )) {
			break;
		}
	} while (wait--);


	// Deassert the CE pin (Standby-II --> Standby-I)
	//nRF24_CE_L();

	if (!wait) {
		// Timeout
		return nRF24_TX_TIMEOUT;
	}

	// Clear pending IRQ flags
	nRF24_ClearIRQFlags();

	if (status & nRF24_FLAG_MAX_RT) {
		// Auto retransmit counter exceeds the programmed maximum limit (FIFO is not removed)
		return nRF24_TX_MAXRT;
	}

	if (status & nRF24_FLAG_TX_DS) {
		// Successful transmission
		nRF24_FlushTX();
		return nRF24_TX_SUCCESS;
	}

	// Some banana happens, a payload remains in the TX FIFO, flush it
	nRF24_FlushTX();

	return nRF24_TX_ERROR;

}

void NRF_24_Slave_Init() {

	//   - RF channel: 115 (2515MHz)
	//   - data rate: 250kbps (minimum possible, to increase reception reliability)
	//   - CRC scheme: 2 byte
	uint8_t nRF24_ADDR_Act[3] = { 0 };
	static const uint8_t nRF24_ADDR0[] = Sensor_1_Pipe;
	static const uint8_t nRF24_ADDR1[] = Sensor_2_Pipe;
	static const uint8_t nRF24_ADDR2[] = Sensor_3_Pipe;
	static const uint8_t nRF24_ADDR3[] = Sensor_4_Pipe;

	// Set RF channel

	//nRF24_SetRFChannel(100);
	nRF24_SetRFChannel(106);


	// Set data rate
	nRF24_SetDataRate(nRF24_DR_250kbps);

	// Set CRC scheme
	nRF24_SetCRCScheme(nRF24_CRC_2byte);

	// Set address width in bytes, its common for all pipes (RX and TX)
	nRF24_SetAddrWidth(3);

	// Set Tx_Pipe and Pipe0
	switch (BeaconID) {

	case BEACON1:
		memcpy(&nRF24_ADDR_Act, nRF24_ADDR0, 5);
		break;

	case BEACON2:
		memcpy(&nRF24_ADDR_Act, nRF24_ADDR1, 5);
		break;

	case BEACON3:
		memcpy(&nRF24_ADDR_Act, nRF24_ADDR2, 5);
		break;

	case BEACON4:
		memcpy(&nRF24_ADDR_Act, nRF24_ADDR3, 5);
		break;

	default:
		break;

	}

	nRF24_SetAddr(nRF24_PIPETX, nRF24_ADDR_Act);
	nRF24_SetAddr(nRF24_PIPE0, nRF24_ADDR_Act);

	// Configure a specified RX pipe
	//   pipe - number of the RX pipe, value from 0 to 5
	//   aa_state - state of auto acknowledgment, one of nRF24_AA_xx values
	//   payload_len - payload length in bytes
	nRF24_SetRXPipe(nRF24_PIPE0, nRF24_AA_ON, Payload_Len);

	// Set TX power (maximum)
	nRF24_SetTXPower(nRF24_TXPWR_0dBm);
	//nRF24_SetTXPower(nRF24_TXPWR_0dBm);


	// Set automatic retransmission parameters
	nRF24_SetAutoRetr(nRF24_ARD_2500us, 10);

	// Set operational mode (PTX == transmitter)
	nRF24_SetOperationalMode(nRF24_MODE_RX);

	// Clear any pending IRQ flags
	nRF24_ClearIRQFlags();

	// Wake the transceiver
	nRF24_SetPowerMode(nRF24_PWR_UP);

	// Put the transceiver to the RX mode
	nRF24_CE_H();

}

void RF_Transmit_Alive_MSG() {

	BEACONMESSAGE_TypeDef Payload = { 0 };
	SENSORSTATS_TypeDef Sensor_Status_Send = NON_INTERRUPTED;
	extern uint16_t Sensor_Ind_Counter;

	nRF24_FlushRX();
	nRF24_FlushTX();

	Message_Counter++;

	//Only tells master that the signal is NON_INTERRUPTED if it stays more than 2 seconds without beeing interrupted.
	//This avoids flase positive situations where the signal is flickering
	if (Sensor_Ind_Counter < 200)
		Sensor_Status_Send = INTERRUPTED;
	else
		Sensor_Status_Send = NON_INTERRUPTED;

	Payload.Beacon_Id = BeaconID;
	Payload.Beacon_Mode = Device_Current_Mode;
	Payload.Sensor_status = Sensor_Status_Send;
	Payload.Time_MilisH = 0;
	Payload.Time_MilisL = 0;
	Payload.Msg_Counter_H = (Message_Counter >> 8) & 0xFF;
	Payload.Msg_Counter_L = Message_Counter & 0xFF;
	Payload.Battery_Percentage = Battery_Actual_Val;

	nRF24_CE_L();
	nRF24_SetPowerMode(nRF24_PWR_UP);
	nRF24_SetOperationalMode(nRF24_MODE_TX);
	tx_res = nRF24_TransmitPacket(&Payload, Payload_Len);
	nRF24_SetOperationalMode(nRF24_MODE_RX);
	nRF24_CE_H();

}

void RF_Transmit_Trigger_MSG() {

	BEACONMESSAGE_TypeDef Payload = { 0 };

	nRF24_FlushRX();
	nRF24_FlushTX();
	//nRF24_ClearIRQFlags();

	Payload.Beacon_Id = BeaconID;
	Payload.Beacon_Mode = RACE_MODE;
	Payload.Sensor_status = INTERRUPTED;
	Payload.Time_MilisH = (Frozen_Timer_ms >> 8) & 0xFF;
	Payload.Time_MilisL = Frozen_Timer_ms & 0xFF;
	Payload.Msg_Counter_H = (Message_Counter >> 8) & 0xFF;
	Payload.Msg_Counter_L = Message_Counter & 0xFF;
	Payload.Battery_Percentage = Battery_Actual_Val;

	nRF24_SetOperationalMode(nRF24_MODE_TX);
	tx_res = nRF24_TransmitPacket(&Payload, Payload_Len);
	nRF24_SetOperationalMode(nRF24_MODE_RX);
	nRF24_CE_H();

}

void RF_Read_Settings_Msg(BEACONMODE_TypeDef Received_Mode) {

	GPIO_InitTypeDef GPIO_InitStruct_Change = {0};
	extern uint8_t Car_Detected_Flag;

	if (Received_Mode != Device_Current_Mode) {
		switch (Received_Mode) {

		case STANDBY_MODE:
			Device_Current_Mode = STANDBY_MODE;
			HAL_NVIC_EnableIRQ(EXTI0_IRQn);
			Message_Counter = 0x00;
			Car_Detected_Flag = 0x00;
			break;

		case RACE_MODE:
			Device_Current_Mode = RACE_MODE;
			break;
		}

	}

}
