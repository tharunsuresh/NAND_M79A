/************************** Flash Memory Driver ***********************************

   Filename:    nand_m79a.c
   Description: Functions for reading and writing to M79a NAND Flash. Uses HAL SPI calls for STM32L0 series.

   Version:     0.1
   Author:      Tharun Suresh 

********************************************************************************

   Version History.

   Ver.		Date			Comments

   0.1		Jan 2022 		In Development

********************************************************************************
  
  	The following functions are available in this library:


********************************************************************************/

#include "nand_m79a.h"


/********************************************************************************/

// The following functions are declared for internal use only:

NANDReturnType __wait_until_ready(SPI_HandleTypeDef *hspi);
HAL_StatusTypeDef __write_enable(SPI_HandleTypeDef *hspi);
HAL_StatusTypeDef __write_disable(SPI_HandleTypeDef *hspi);

/********************************************************************************/


/******************************************************************************
 *						NAND Low Level Driver Functions
 *****************************************************************************/

/**
    @brief This function initializes the NAND. Steps: sending reset command and checking for correct device ID.
 	@note This function must be called first when powered on.

    @return NANDReturnType
    @retval Ret_ResetFailed
    @retval Ret_WrongID
    @retval Ret_Success
 */
NANDReturnType NAND_Init(SPI_HandleTypeDef *hspi) {

	NAND_ID dev_ID;
	NANDReturnType resetStatus;

	/* Reset NAND flash during initialization
	 * May not be necessary though (page 50) */

	HAL_Delay(T_POR);  // wait for T_POR = 1.25ms after power on
	__wait_until_ready(hspi); // wait until status register indicates no operations in progress
	resetStatus = NAND_Reset(hspi);
	__wait_until_ready(hspi);

	if (resetStatus != Ret_Success) {
		return Ret_ResetFailed;
	} else {
		/* check if device ID is same as expected */
		NAND_Read_ID(hspi, &dev_ID);
		if (dev_ID.manufacturer_ID != NAND_ID_MANUFACTURER || dev_ID.device_ID != NAND_ID_DEVICE) {
			return Ret_WrongID;
		} else {
			return Ret_Success;
		}
	}
}

/**
 	@brief Sends command to reset the NAND Flash chip.
	@note Transaction length: 1 byte; Returns success when Flash is ready for further instructions.

    @return NANDReturnType
    @retval Ret_ResetFailed
    @retval Ret_Success
*/
NANDReturnType NAND_Reset(SPI_HandleTypeDef *hspi) {

	HAL_StatusTypeDef SPI_Status;
	uint8_t command = SPI_NAND_RESET;

	SPI_Status = HAL_SPI_Transmit(hspi, &command, sizeof(command), NAND_SPI_TIMEOUT);
	HAL_Delay(T_POR);	// wait for T_POR = 1.25 ms after reset

	if (SPI_Status != HAL_OK) {
		return Ret_ResetFailed;
	} else {
		// wait until OIP bit sets again (Flash is ready for further instructions)
		return __wait_until_ready(hspi);
	}
}


/**
    @brief Sends command to read manufacturer and device ID of NAND flash chip
	@note Transaction length: 4 bytes (2 each way)

    @return NANDReturnType
    @retval Ret_ResetFailed
    @retval Ret_Success

 */

NANDReturnType NAND_Read_ID(SPI_HandleTypeDef *hspi, NAND_ID *nand_ID) {

	uint8_t data_tx[4]; // initialized to zero
	uint8_t data_rx[4];

	data_tx[0] = SPI_NAND_READ_ID; // first byte is the command, rest are zeroes

	HAL_SPI_TransmitReceive(hspi, data_tx, data_rx, sizeof(data_rx), NAND_SPI_TIMEOUT);

	nand_ID -> manufacturer_ID = data_tx[2]; // second last byte from transmission
	nand_ID -> device_ID = data_rx[3]; // last byte

    return Ret_Success;
}


/**
 	@brief Sends command to read the entire status register
	@note Transaction length: 3 bytes (2 to transmit, 1 to receive)

    @return uint8_t
    @retval Status register contents
*/
uint8_t NAND_Read_Status_Reg(SPI_HandleTypeDef *hspi) {

	uint8_t command[3];
	uint8_t data_rx[3];

	command[0] = SPI_NAND_GET_FEATURES;
	command[1] = SPI_NAND_STATUS_REG_ADDR;

	HAL_SPI_TransmitReceive(hspi, command, data_rx, sizeof(data_rx), NAND_SPI_TIMEOUT);

	return data_rx[3];
}


/**
 	@brief Sends command to read status register bit 1 (OIP).
	@note Transaction length: 3 bytes (2 to transmit, 1 to receive)

    @return NANDReturnType
    @retval
*/
NANDReturnType NAND_Check_Busy(SPI_HandleTypeDef *hspi) {
	if ((NAND_Read_Status_Reg(hspi) & SPI_NAND_OIP) == SPI_NAND_OIP) {
		return Ret_Success;
	} else {
		return Ret_NANDBusy;
	}
}


/******************************************************************************
 *						Internal Functions
 *****************************************************************************/

/**
    @brief
    @note Waits until OIP bit in the Status Register sets again, indicating
	that the NAND Flash is ready for further instructions. 

	This is written assuming that the device keeps reading & outputting the status register
	until another command is issued. This is true for other models but this datasheet doesn't
	specify this explicitly.

	TODO: check this on hardware with an oscilloscope.

    @return NANDReturnType
    @retval Ret_Success
 */
NANDReturnType __wait_until_ready(SPI_HandleTypeDef *hspi) {
	uint8_t data_rx;

	if (NAND_Check_Busy(hspi) == Ret_NANDBusy) {
		while ((data_rx & SPI_NAND_OIP) != SPI_NAND_OIP) {
			HAL_SPI_Receive(hspi, &data_rx, sizeof(data_rx), NAND_SPI_TIMEOUT);
			HAL_Delay(1);
		}
	}
	return Ret_Success;
}

HAL_StatusTypeDef __write_enable(SPI_HandleTypeDef *hspi) {
	uint8_t command = SPI_NAND_WRITE_ENABLE;
	return HAL_SPI_Transmit(hspi, &command, sizeof(command), NAND_SPI_TIMEOUT);
}

HAL_StatusTypeDef __write_disable(SPI_HandleTypeDef *hspi) {
	uint8_t command = SPI_NAND_WRITE_DISABLE;
	return HAL_SPI_Transmit(hspi, &command, sizeof(command), NAND_SPI_TIMEOUT);
}


/*
 * TODO:
 * The first spare area location in each bad block contains the bad-block mark (0x00).
 * System software should initially check the first spare area location (byte 2048) for non-FFh data on
 * the first page of each block prior to performing any program or erase operations on the
 * NAND Flash device.
 *
 */




