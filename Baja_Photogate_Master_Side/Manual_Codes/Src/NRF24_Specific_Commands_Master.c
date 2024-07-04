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

#define HEX_CHARS      "0123456789ABCDEF"

extern volatile uint8_t msg_len;
extern char msg[80];

uint8_t nRF24_payload[32];

nRF24_RXResult pipe;

void NRF_24_Master_Init();
void NRF_24_Log_Init();
void RF_Transmit_Config_MSG(BEACONMODE_TypeDef reqmode);



static const uint8_t nRF24_ADDR0[] = Sensor_1_Pipe;
static const uint8_t nRF24_ADDR1[] = Sensor_2_Pipe;
static const uint8_t nRF24_ADDR2[] = Sensor_3_Pipe;
static const uint8_t nRF24_ADDR3[] = Sensor_4_Pipe;



// Helpers for transmit mode demo

// Timeout counter (depends on the CPU speed)
// Used for not stuck waiting for IRQ
#define nRF24_WAIT_TIMEOUT         (uint32_t)0x00FFF

// Result of packet transmission




// Function to transmit data packet
// input:
//   pBuf - pointer to the buffer with data to transmit
//   length - length of the data buffer in bytes
// return: one of nRF24_TX_xx values
nRF24_TXResult nRF24_TransmitPacket(uint8_t *pBuf, uint8_t length) {
	volatile uint32_t wait = nRF24_WAIT_TIMEOUT;
	uint8_t status = 0xff;

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
		if (status & (nRF24_FLAG_TX_DS | nRF24_FLAG_MAX_RT)) {
			break;
		}
	} while (wait--);

	// Deassert the CE pin (Standby-II --> Standby-I)
	nRF24_CE_L();

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

void NRF_24_Master_Init(){


		//   - RF channel: 115 (2515MHz)
		//   - data rate: 250kbps (minimum possible, to increase reception reliability)
		//   - CRC scheme: 2 byte


	    // Set RF channel
	    nRF24_SetRFChannel(100);

	    // Set data rate
	    nRF24_SetDataRate(nRF24_DR_250kbps);

	    // Set CRC scheme
	    nRF24_SetCRCScheme(nRF24_CRC_2byte);

	    // Set address width in bytes, its common for all pipes (RX and TX)
	    nRF24_SetAddrWidth(3);

	    //nRF24_SetAddr(nRF24_PIPETX, nRF24_ADDR);
	    nRF24_SetAddr(nRF24_PIPE0, nRF24_ADDR0);
	    nRF24_SetAddr(nRF24_PIPE1, nRF24_ADDR1);
	    nRF24_SetAddr(nRF24_PIPE2, nRF24_ADDR2);
	    nRF24_SetAddr(nRF24_PIPE3, nRF24_ADDR3);

	    // Configure a specified RX pipe
	    //   pipe - number of the RX pipe, value from 0 to 5
	    //   aa_state - state of auto acknowledgment, one of nRF24_AA_xx values
	    //   payload_len - payload length in bytes
	    nRF24_SetRXPipe(nRF24_PIPE0, nRF24_AA_ON, Payload_Len);
	    nRF24_SetRXPipe(nRF24_PIPE1, nRF24_AA_ON, Payload_Len);
	    nRF24_SetRXPipe(nRF24_PIPE2, nRF24_AA_ON, Payload_Len);
	    nRF24_SetRXPipe(nRF24_PIPE3, nRF24_AA_ON, Payload_Len);


	    // Set automatic retransmission parameters
	    nRF24_SetAutoRetr(nRF24_ARD_2500us, 10);


	    // Set TX power (maximum)
	    nRF24_SetTXPower(nRF24_TXPWR_0dBm);

	    // Set operational mode (PTX == transmitter)
	    nRF24_SetOperationalMode(nRF24_MODE_RX);

	    // Clear any pending IRQ flags
	    nRF24_ClearIRQFlags();

	    // Wake the transceiver
	    nRF24_SetPowerMode(nRF24_PWR_UP);

	    // Put the transceiver to the RX mode
	    nRF24_CE_H();



}

void RF_Transmit_Config_MSG(BEACONMODE_TypeDef reqmode){

	BEACONMESSAGE_TypeDef Payload = {0};
	uint8_t i = 0x00;
	volatile uint8_t nRF24_ADDR0[] = Sensor_1_Pipe;
	nRF24_TXResult tx_res;

	Payload.Beacon_Id = BeaconID;
	Payload.Beacon_Mode = reqmode;
	Payload.Sensor_status = 0x01;
	Payload.Time_MilisH = 0xAA;
	Payload.Time_MilisL = 0xAA;
	Payload.Msg_Counter_H = 0xAA;
	Payload.Msg_Counter_L = 0xAA;
	Payload.Battery_Percentage = 0xAA;

	nRF24_SetOperationalMode(nRF24_MODE_TX);

	nRF24_SetAddr(nRF24_PIPETX, nRF24_ADDR0);
 	tx_res |= nRF24_TransmitPacket(&Payload, Payload_Len);
 	//msg_len = sprintf( msg,"Tx res for Sensor 1: %x \n", tx_res);
 	//CDC_Transmit_FS(msg, msg_len);

	nRF24_SetAddr(nRF24_PIPETX, nRF24_ADDR1);
 	tx_res |= nRF24_TransmitPacket(&Payload, Payload_Len);
 	//msg_len = sprintf( msg,"Tx res for Sensor 2: %x \n", tx_res);
 	//CDC_Transmit_FS(msg, msg_len);

	nRF24_SetAddr(nRF24_PIPETX, nRF24_ADDR2);
 	tx_res |= nRF24_TransmitPacket(&Payload, Payload_Len);
 	//msg_len = sprintf( msg,"Tx res for Sensor 3: %x \n", tx_res);
 	//CDC_Transmit_FS(msg, msg_len);

	nRF24_SetAddr(nRF24_PIPETX, nRF24_ADDR3);
 	tx_res |= nRF24_TransmitPacket(&Payload, Payload_Len);
 	//msg_len = sprintf( msg,"Tx res for Sensor 4: %x \n", tx_res);
 	//CDC_Transmit_FS(msg, msg_len);

 	nRF24_SetOperationalMode(nRF24_MODE_RX);
 	nRF24_FlushRX();
 	nRF24_FlushTX();
	nRF24_CE_H();
}

void NRF_24_Log_Init(){

	//   - RF channel: 115 (2515MHz)
			//   - data rate: 250kbps (minimum possible, to increase reception reliability)
			//   - CRC scheme: 2 byte

	    	//static const uint8_t nRF24_ADDR_Log[] = {0xDA, 0x7A, 0x10, 0x66, 0xee};
			static const uint8_t nRF24_ADDR_Log[] = {0xee, 0x66, 0x10, 0x7A, 0xDA};

			//static const uint8_t nRF_ADDR_Log_RX[] = {0xab, 0xbc, 0xab, 0xbc, 0xcd}; // O SOFTWWARE DO CHINES TA INVERTIDO
	    	static const uint8_t nRF_ADDR_Log_RX[] = {0xcd, 0xbc, 0xab, 0xbc, 0xab};

		    nRF24_SetPowerMode(nRF24_PWR_DOWN);

	    	// Set RF channel
		    nRF24_SetRFChannel(124);

		    // Set data rate
		    nRF24_SetDataRate(nRF24_DR_250kbps);

		    // Set CRC scheme
		    nRF24_SetCRCScheme(nRF24_CRC_2byte);

		    // Set address width in bytes, its common for all pipes (RX and TX)
		    nRF24_SetAddrWidth(5);


		    nRF24_SetAddr(nRF24_PIPETX, nRF24_ADDR_Log);
		    nRF24_SetAddr(nRF24_PIPE1, nRF_ADDR_Log_RX);


		    nRF24_SetRXPipe(nRF24_PIPE1, nRF24_AA_ON, 32);


		    nRF24_SetAutoRetr(nRF24_ARD_2500us, 10);

		    // Set TX power (maximum)
		    nRF24_SetTXPower(nRF24_TXPWR_0dBm);

		    nRF24_SetOperationalMode(nRF24_MODE_RX);

		    // Clear any pending IRQ flags
		    nRF24_ClearIRQFlags();

		    // Wake the transceiver
		    nRF24_SetPowerMode(nRF24_PWR_UP);
		    nRF24_CE_H();

}






