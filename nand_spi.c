/************************** NAND SPI Functions ***********************************

    Filename:    nand_spi.c
    Description: Implements SPI wrapper functions for use by low-level drivers.
                Uses HAL library for STM32L0 series.

    Version:     0.1
    Author:      Tharun Suresh

********************************************************************************

    Version History.

    Ver.    Date            Comments

    0.1     Jan 2022        In Development

********************************************************************************

    The following functions are available in this library:


********************************************************************************/

#include "nand_spi.h"

/******************************************************************************
 *                              Initialization
 *****************************************************************************/

// SPI_HandleTypeDef hspi_nand;

/**
	@brief Initialize SPI bus to NAND IC.
	@note For reference only. Not to be called.
*/
//static void NAND_SPI_Init(void){
//
//   /* NAND SPI parameter configuration*/
//   hspi_nand.Instance = SPI2;
//   hspi_nand.Init.Mode = SPI_MODE_MASTER;
//   hspi_nand.Init.Direction = SPI_DIRECTION_2LINES;
//   hspi_nand.Init.DataSize = SPI_DATASIZE_8BIT;
//   hspi_nand.Init.CLKPolarity = SPI_POLARITY_LOW;
//   hspi_nand.Init.CLKPhase = SPI_PHASE_1EDGE;
//   hspi_nand.Init.NSS = SPI_NSS_SOFT;
//   hspi_nand.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_256;
//   hspi_nand.Init.FirstBit = SPI_FIRSTBIT_MSB;
//   hspi_nand.Init.TIMode = SPI_TIMODE_DISABLE;
//   hspi_nand.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
//   hspi_nand.Init.CRCPolynomial = 7;
//
//   HAL_SPI_Init(&hspi_nand);
//};

/**
	@brief Initialize GPIO pins connected to NAND IC.
	@note For reference only. Not to be called.
*/
//static void NAND_GPIO_Init(void){
//
//   GPIO_InitTypeDef NAND_GPIO_InitStruct = {0};
//
//   HAL_GPIO_WritePin(NAND_NCS_PORT, NAND_NCS_PIN, GPIO_PIN_SET); // active low
//   HAL_GPIO_WritePin(NAND_SCK_PORT, NAND_SCK_PIN, GPIO_PIN_RESET);
//   HAL_GPIO_WritePin(NAND_MISO_PORT, NAND_MISO_PIN, GPIO_PIN_RESET);
//   HAL_GPIO_WritePin(NAND_MOSI_PORT, NAND_MOSI_PIN, GPIO_PIN_RESET);
//
//   NAND_GPIO_InitStruct.Pin = NAND_SCK_PIN | NAND_MISO_PIN | NAND_MOSI_PIN;
//   NAND_GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
//   NAND_GPIO_InitStruct.Pull = GPIO_PULLUP;
//   NAND_GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
//   NAND_GPIO_InitStruct.Alternate = GPIO_AF0_SPI2;
//   HAL_GPIO_Init(NAND_SCK_PORT, &NAND_GPIO_InitStruct);
//
//   NAND_GPIO_InitStruct.Pin = NAND_NCS_PIN;
//   HAL_GPIO_Init(NAND_NCS_PORT, &NAND_GPIO_InitStruct);
//
//};


/**
	@brief Calls HAL_Delay() for stated number of milliseconds
*/
void NAND_Wait(uint8_t milliseconds){
   HAL_Delay(milliseconds);
};


/******************************************************************************
 *                     Send & Receive Complete Transactions 
 *****************************************************************************/

/**
	@brief NAND Data input: this function is used to write data to NAND.
*/
NAND_SPI_ReturnType NAND_SPI_Send(SPI_HandleTypeDef *hspi, SPI_Params *data_send) {
	HAL_StatusTypeDef send_status;

	__nand_spi_cs_low();
	send_status = HAL_SPI_Transmit(hspi, data_send->buffer, data_send->length, NAND_SPI_TIMEOUT);
	__nand_spi_cs_high();

	if (send_status != HAL_OK) {
		return SPI_Fail; 
	} else {
		return SPI_OK;
	}

};


/**
	@brief NAND Data transmit: this function is used to send and receive read data from NAND.
*/
NAND_SPI_ReturnType NAND_SPI_SendReceive(SPI_HandleTypeDef *hspi, SPI_Params *data_send, SPI_Params *data_recv) {
	HAL_StatusTypeDef transmit_status;

	__nand_spi_cs_low();
	HAL_SPI_Transmit(hspi, data_send->buffer, data_send->length, NAND_SPI_TIMEOUT);
	transmit_status = HAL_SPI_Receive(hspi, data_recv->buffer, data_recv->length, NAND_SPI_TIMEOUT);
	__nand_spi_cs_high();

	if (transmit_status != HAL_OK) {
		return SPI_Fail; 
	} else {
		return SPI_OK;
	}
};


/**
	@brief NAND Data output: this function is used to read data from NAND.
*/
NAND_SPI_ReturnType NAND_SPI_Receive(SPI_HandleTypeDef *hspi, SPI_Params *data_recv) {
	HAL_StatusTypeDef receive_status;

	__nand_spi_cs_low();
	receive_status = HAL_SPI_Receive(hspi, data_recv->buffer, data_recv->length, NAND_SPI_TIMEOUT);
	__nand_spi_cs_high();

	if (receive_status != HAL_OK) {
		return SPI_Fail; 
	} else {
		return SPI_OK;
	}
};

/******************************************************************************
 *                  Send command and data in one transaction  
 *****************************************************************************/

/**
	@brief NAND Data input: this function is used to write data to NAND.
*/
NAND_SPI_ReturnType NAND_SPI_Send_Command_Data(SPI_HandleTypeDef *hspi, SPI_Params *cmd_send, SPI_Params *data_send) {
	HAL_StatusTypeDef send_status;

	__nand_spi_cs_low();
	HAL_SPI_Transmit(hspi, cmd_send->buffer, cmd_send->length, NAND_SPI_TIMEOUT);
	send_status = HAL_SPI_Transmit(hspi, data_send->buffer, data_send->length, NAND_SPI_TIMEOUT);
	__nand_spi_cs_high();

	if (send_status != HAL_OK) {
		return SPI_Fail; 
	} else {
		return SPI_OK;
	}
};

/******************************************************************************
 *                              Internal Functions
 *****************************************************************************/

/**
 	@brief Enable SPI communication to NAND by pulling chip select pin low.
    @note Must be called prior to every SPI transmission
*/
void __nand_spi_cs_low(void){
   HAL_GPIO_WritePin(NAND_NCS_PORT, NAND_NCS_PIN, GPIO_PIN_RESET);
};


/**
 	 @brief Close SPI communication to NAND by pulling chip select pin high.
   	 @note Must be called after every SPI transmission
*/
void __nand_spi_cs_high(void){
	HAL_GPIO_WritePin(NAND_NCS_PORT, NAND_NCS_PIN, GPIO_PIN_SET);
};

