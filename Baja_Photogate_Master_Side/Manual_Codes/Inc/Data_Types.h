#ifndef DATA_TYPES_H_
#define DATA_TYPES_H_




typedef struct{

	uint8_t Passada[3];
	uint8_t Carro[3];
	uint8_t Aceleracao[6];
	uint8_t Velocidade[2];
} Data_Line_Struct;

typedef struct{

	uint8_t Size;
	Data_Line_Struct Lines[255];
} Data_FS;

Data_FS Read_SD_Data();


#endif /* DATA_TYPES_H */
