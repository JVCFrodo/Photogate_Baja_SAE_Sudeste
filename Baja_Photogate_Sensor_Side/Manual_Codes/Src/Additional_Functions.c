/**
 * Addition_Functions.c
 *
 * Created on: Mar 19, 2024
 * Author: CAJ1SBC
 */

#include "main.h"
#include "definitions.h"
#include "shared_definitions.h"

SENSORSTATS_TypeDef Check_Sensor_Status();


uint8_t Calc_Batt_Perc();
void Initialize_Batt_Avg_Calc();


extern uint16_t Analog_read;
uint16_t Filter_Array[100];
uint8_t Battery_Actual_Val = 0x00;


/**
 * Calculates the battery percentage based on the analog read value.
 *
 * @return The battery percentage.
 */
uint8_t Calc_Batt_Perc() {
  // Define a table of voltage values corresponding to battery percentages.
  static uint16_t V_Bat_mv[] = {
    2210, 2610, 2820, 2940, 3040, 3120, 3190, 3270, 3320, 3370, 3410, 3430,
    3440, 3460, 3480, 3500, 3520, 3530, 3550, 3570, 3590, 3600, 3610, 3630,
    3650, 3670, 3690, 3700, 3730, 3750, 3770, 3780, 3800, 3820, 3840, 3850,
    3870, 3900, 3920, 3940, 3960, 3990, 4010, 4010, 4019, 4030, 4040, 4050,
    4070, 4100
  };

  // Define conversion factors for ADC to mV.
  static float Conv_Factor_mv = 0.8119658;
  static float Res_Div_Ratio = 2;

  // Define the filtering window size.
  uint8_t Filtering_Window_Size = 20;

  // Initialize variables to hold the average value, voltage, and index.
  uint32_t Avg_Aux = 0x00;
  uint16_t Analog_Read_Current = 0x00;
  uint16_t Voltage_mv = 0x00;
  uint16_t Analog_Read_Avg = 0x00;
  uint16_t Diff = 0x00;
  uint16_t Min_Diff = 0xFFFFFF;
  uint8_t i = 0x00;
  uint8_t Table_Size = 50;
  uint8_t Index = 0x00;

  // Get the current analog read value.
  Analog_Read_Current = Analog_read;

  // Shift the filter array to make room for the new value.
  for (i = 0x00; i <= (Filtering_Window_Size - 2); i++) {
    Filter_Array[i] = Filter_Array[i + 1];
  }
  Filter_Array[(Filtering_Window_Size - 1)] = Analog_Read_Current;

  // Calculate the average value.
  for (i = 0x00; i < Filtering_Window_Size; i++) {
    Avg_Aux += Filter_Array[i];
  }
  Analog_Read_Avg = Avg_Aux / Filtering_Window_Size;

  // Calculate the voltage.
  Voltage_mv = (Analog_Read_Avg * Conv_Factor_mv * Res_Div_Ratio);

  // Find the index of the closest voltage value in the table.
  for (i = 0; i <= Table_Size; i++) {
    Diff = abs(Voltage_mv - V_Bat_mv[i]);
    if (Diff < Min_Diff) {
      Min_Diff = Diff;
      Index = i;
    }
  }

  // Return the battery percentage.
  return (Index * 2);
}

/**
 * Initializes the battery average calculation array.
 */
void Initialize_Batt_Avg_Calc() {
  // Initialize the filter array.
  uint8_t i = 0x00;
  for (i = 0x00; i <= 10; i++) {
    Filter_Array[i] = 0x00;
  }
}

/**
 * Sets the input pin to pooling mode.
 */
void Set_InputPin_Pooling() {
  // Define the GPIO initialization structure.
  GPIO_InitTypeDef GPIO_InitStruct_Pooling = {0};

  HAL_NVIC_DisableIRQ(EXTI0_IRQn);
  HAL_GPIO_DeInit(GPIOA, GPIO_PIN_0);
  

  // Set the pin to input mode.
  GPIO_InitStruct_Pooling.Pin = GPIO_PIN_0;
  GPIO_InitStruct_Pooling.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct_Pooling.Pull = GPIO_NOPULL;

  // Initialize the GPIO pin.
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct_Pooling);
}


 SENSORSTATS_TypeDef Check_Sensor_Status(){

	extern uint16_t Sensor_Ind_Counter;
	SENSORSTATS_TypeDef Sensor_Status = INTERRUPTED;
	uint8_t Pinstate = 0x00;

	Pinstate = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0);

	if (Pinstate == TRUE) {
		Sensor_Status = INTERRUPTED;
		Sensor_Ind_Counter = 0;
	} else if (Pinstate == FALSE) {
		Sensor_Status = NON_INTERRUPTED;
		Sensor_Ind_Counter += 1;
		if (Sensor_Ind_Counter >= 200)
			Sensor_Ind_Counter = 200;
	}

	return Sensor_Status;

}

/**
 * Sets the input pin to interrupt mode.
 */
void Set_InputPin_Interrupt() {
  // Define the GPIO initialization structure.
  GPIO_InitTypeDef GPIO_InitStruct_Interrupt = {0};

  HAL_GPIO_DeInit(GPIOA, GPIO_PIN_0);
  
  // Set the pin to interrupt mode - Rising Edge.
  GPIO_InitStruct_Interrupt.Pin = GPIO_PIN_0;
  GPIO_InitStruct_Interrupt.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct_Interrupt.Pull = GPIO_NOPULL;

  // Initialize the GPIO pin.
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct_Interrupt);
  HAL_NVIC_SetPriority(EXTI0_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);
}
