/************************** Flash Memory Driver ***********************************

   Filename:    nand_m79a.c
   Description: Low-level functions for reading and writing to M79a NAND Flash via SPI.

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


/******************************************************************************
 *						Initialization
 *****************************************************************************/

/**
    @brief Initializes the NAND. Steps: Reset device and check for correct device IDs.
 	@note This function must be called first when powered on.

    @return NANDReturnType
    @retval Ret_ResetFailed
    @retval Ret_WrongID
    @retval Ret_Success
 */
NANDReturnType NAND_Init(SPI_HandleTypeDef *hspi) {

	NAND_ID dev_ID;

	/* Reset NAND flash during initialization
	 * May not be necessary though (page 50) */

	NAND_Wait(T_POR);  // wait for T_POR = 1.25ms after power on
	
	// reset and wait until status register indicates no operations in progress
	if (NAND_Reset(hspi) != Ret_Success) {
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

	SPI_Status = NAND_SPI_SendData(hspi, &command, 1);
	NAND_Wait(T_POR);	// wait for T_POR = 1.25 ms after reset

	if (SPI_Status != HAL_OK) {
		return Ret_ResetFailed;
	} else {
		// wait until OIP bit resets again (Flash is ready for further instructions)
		return __wait_until_ready(hspi);
	}
}


/******************************************************************************
 *						Identification Operations
 *****************************************************************************/

/**
    @brief Sends command to read manufacturer and device ID of NAND flash chip
	@note Transaction length: 4 bytes (2 each way)

    @return NANDReturnType
    @retval Ret_ResetFailed
    @retval Ret_Success

 */

NANDReturnType NAND_Read_ID(SPI_HandleTypeDef *hspi, NAND_ID *nand_ID) {

	uint8_t length_tx = 1;
	uint8_t command = SPI_NAND_READ_ID; // read ID command to nand

	uint8_t length_rx = 2; // num of bytes received with ID data
	uint8_t data_rx[length_rx]; // data buffer for received data

	NAND_SPI_SendReceive(hspi, &command, length_tx, data_rx, length_rx);

	nand_ID -> manufacturer_ID = data_rx[0]; // second last byte from transmission
	nand_ID -> device_ID = data_rx[1]; // last byte

    return Ret_Success;
}

/******************************************************************************
 *						Feature Operations
 *****************************************************************************/

/**
 	@brief Sends command to read the entire status register
	@note Transaction length: 3 bytes (2 to transmit, 1 to receive)

    @return uint8_t
    @retval Status register contents
*/
uint8_t NAND_Read_Status_Reg(SPI_HandleTypeDef *hspi) {

	uint8_t length_tx = 2;
	uint8_t command[length_tx];

	uint8_t length_rx = 1;
	uint8_t data_rx;

	command[0] = SPI_NAND_GET_FEATURES;
	command[1] = SPI_NAND_STATUS_REG_ADDR;

	NAND_SPI_SendReceive(hspi, command, length_tx, &data_rx, length_rx);

	return data_rx;
}


/**
 	@brief Sends command to read status register bit 1 (OIP).
	@note Transaction length: 3 bytes (2 to transmit, 1 to receive)

    @return NANDReturnType
    @retval Ret_Success
	@retval Ret_NANDBusy
*/
NANDReturnType NAND_Check_Busy(SPI_HandleTypeDef *hspi) {
	if ((NAND_Read_Status_Reg(hspi) & SPI_NAND_OIP) == SPI_NAND_OIP) {
		return Ret_Success;
	} else {
		return Ret_NANDBusy;
	}
}





/******************************************************************************
 *						Read Operations
 *****************************************************************************/


/**
 	@brief Sends command to read data stored in a page
	@note Command sequence: 
			1) Send page read command to read data from page to cache
			2) Wait until OIP bit resets in status register
			3) Read data from cache

    @return Data buffer
    @retval 
*/
NANDReturnType NAND_Page_Read( ) {
}


/******************************************************************************
 *						Erase Operations
 *****************************************************************************/


/*
 * The first spare area location in each bad block contains the bad-block mark (0x00).
 * System software should initially check the first spare area location (byte 2048) for non-FFh data on
 * the first page of each block prior to performing any program or erase operations on the
 * NAND Flash device.
 *
 */




/******************************************************************************
 *						Write Operations
 *****************************************************************************/

/*
 * The first spare area location in each bad block contains the bad-block mark (0x00).
 * System software should initially check the first spare area location (byte 2048) for non-FFh data on
 * the first page of each block prior to performing any program or erase operations on the
 * NAND Flash device.
 *
 */




/******************************************************************************
 *						Move Operations
 *****************************************************************************/







/******************************************************************************
 *						Lock Operations
 *****************************************************************************/








/******************************************************************************
 *						Internal Functions
 *****************************************************************************/

/**
    @brief
    @note Waits until OIP bit in the Status Register resets again, indicating
	that the NAND Flash is ready for further instructions. If OIP = 1, 
	operation is ongoing, i.e. device is busy.

	Assumption: the device keeps outputting the status register contents
	until another command is issued. This is shown in page 17.
	TODO: confirm with an oscilloscope.

    @return NANDReturnType
    @retval Ret_Success
 */
NANDReturnType __wait_until_ready(SPI_HandleTypeDef *hspi) {
	// uint8_t data_rx;
	// 
	// if (NAND_Check_Busy(hspi) == Ret_NANDBusy) {
	// 	while ((data_rx & SPI_NAND_OIP) == SPI_NAND_OIP) {
	// 		NAND_SPI_ReceiveData(hspi, &data_rx, 1);
	// 		NAND_Wait(1);
	// 	}
	// }
	return Ret_Success;
}

HAL_StatusTypeDef __write_enable(SPI_HandleTypeDef *hspi) {
	uint8_t command = SPI_NAND_WRITE_ENABLE;
	return NAND_SPI_SendData(hspi, &command, 1);
}

HAL_StatusTypeDef __write_disable(SPI_HandleTypeDef *hspi) {
	uint8_t command = SPI_NAND_WRITE_DISABLE;
	return NAND_SPI_SendData(hspi, &command, 1);
}






