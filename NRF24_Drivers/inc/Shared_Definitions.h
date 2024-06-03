/*
 * Shared_Definitions.h
 *
 *  Created on: Mar 18, 2024
 *      Author: CAJ1SBC
 */

#ifndef SHARED_DEFINITIONS_H_
#define SHARED_DEFINITIONS_H_

#define Payload_Len  0x08


#define TRUE 1
#define FALSE 0

#define Master_TX_Pipe {0xab, 0xcd, 0xef}
#define Sensor_1_Pipe {0x12, 0x34, 0x12}
#define Sensor_2_Pipe {0x12, 0x34, 0x23}
#define Sensor_3_Pipe {0x12, 0x34, 0x34}
#define Sensor_4_Pipe {0x12, 0x34, 0x45}
#define Datalog_Rx_Pipe {0xDA, 0x7A, 0x10, 0x66, 0xee}



typedef enum{

	STANDBY_MODE = 0b0101,
	RACE_MODE = 0b0110,
	OTA_REPORT_MODE = 0b1100,
} BEACONMODE_TypeDef;


typedef enum{

	BEACON1 = 0x12,
	BEACON2 = 0x23,
	BEACON3 = 0x34,
	BEACON4 = 0x45,
	HOST    = 0xEF,
} BEACONID_TypeDef;


typedef enum{

	INTERRUPTED = 0xDD,
	NON_INTERRUPTED = 0xBB,

}SENSORSTATS_TypeDef;

typedef struct{

	uint8_t Beacon_Id ;
	BEACONMODE_TypeDef Beacon_Mode;
	uint8_t Sensor_status;
	uint8_t Msg_Counter_H;
	uint8_t Msg_Counter_L;
	uint8_t Time_MilisH;
	uint8_t Time_MilisL;
	uint8_t Battery_Percentage;


} BEACONMESSAGE_TypeDef;


#endif /* SHARED_DEFINITIONS_H_ */
