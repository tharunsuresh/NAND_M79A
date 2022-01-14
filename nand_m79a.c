/************************** Flash Memory Driver ***********************************

   Filename:    nand_m79a_lld.c
   Description: Low-level driver functions for reading and writing to M79a NAND Flash via SPI.

   Version:     0.1
   Author:      Tharun Suresh

********************************************************************************

   Version History.

   Ver.		Date			Comments

   0.1		Jan 2022 		In Development

********************************************************************************

  	The following functions are available in this library:


********************************************************************************/

#include "nand_m79a_lld.h"


/******************************************************************************
 *						Initialization
 *****************************************************************************/

/**
    @brief Initializes the NAND. Steps: Reset device and check for correct device IDs.
 	@note This function must be called first when powered on.

    @return NAND_ReturnType
    @retval Ret_ResetFailed
    @retval Ret_WrongID
    @retval Ret_Success
 */
NAND_ReturnType NAND_Init(SPI_HandleTypeDef *hspi) {

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

    @return NAND_ReturnType
    @retval Ret_ResetFailed
    @retval Ret_Success
*/
NAND_ReturnType NAND_Reset(SPI_HandleTypeDef *hspi) {

	NAND_SPI_ReturnType SPI_Status;
	uint8_t command = SPI_NAND_RESET;

	SPI_Status = NAND_SPI_Send(hspi, &command, 1);
	NAND_Wait(T_POR);	// wait for T_POR = 1.25 ms after reset

	if (SPI_Status != SPI_OK) {
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

    @return NAND_ReturnType
    @retval Ret_ResetFailed
    @retval Ret_Success

 */

NAND_ReturnType NAND_Read_ID(SPI_HandleTypeDef *hspi, NAND_ID *nand_ID) {

	uint8_t length_tx = 2;
	uint8_t data_tx[] = {SPI_NAND_READ_ID, 0}; // second byte is dummy byte

	uint8_t length_rx = 2; // num of bytes received with ID data
	uint8_t data_rx[length_rx]; // data buffer for received data

	NAND_SPI_SendReceive(hspi, data_tx, length_tx, data_rx, length_rx);

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
	uint8_t command[] = {SPI_NAND_GET_FEATURES, SPI_NAND_STATUS_REG_ADDR};

	uint8_t length_rx = 1;
	uint8_t data_rx;

	NAND_SPI_SendReceive(hspi, command, length_tx, &data_rx, length_rx);

	return data_rx;
}


/**
 	@brief Sends command to read status register bit 1 (OIP).
	@note Transaction length: 3 bytes (2 to transmit, 1 to receive)

    @return NAND_ReturnType
    @retval Ret_Success
	@retval Ret_NANDBusy
*/
NAND_ReturnType NAND_Check_OIP(SPI_HandleTypeDef *hspi) {
	if ((NAND_Read_Status_Reg(hspi) & SPI_NAND_OIP) == SPI_NAND_OIP) {
		return Ret_NANDBusy;
	} else {
		return Ret_Success;
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
NAND_ReturnType NAND_Page_Read(SPI_HandleTypeDef *hspi, NAND_Addr addr, uint8_t *buffer) {

}


/******************************************************************************
 *						Erase Operations
 *****************************************************************************/


/* TODO:
 * The first spare area location in each bad block contains the bad-block mark (0x00).
 * System software should initially check the first spare area location (byte 2048) for non-FFh data on
 * the first page of each block prior to performing any program or erase operations on the
 * NAND Flash device.
 *
 */

/**
 	@brief 
	@note Command sequence: 


    @return 
    @retval 
*/
NAND_ReturnType NAND_Block_Erase( ) {


}



/******************************************************************************
 *						Write Operations
 *****************************************************************************/

/* TODO: 
 * The first spare area location in each bad block contains the bad-block mark (0x00).
 * System software should initially check the first spare area location (byte 2048) for non-FFh data on
 * the first page of each block prior to performing any program or erase operations on the
 * NAND Flash device.
 */
/**
 	@brief Write 2176 bytes to a page. Address must be the start address of a page. TODO: how to make sure? 
	@note Command sequence: 
			1) WRITE ENABLE
			2) PROGRAM LOAD : load data into cache register
			3) PROGRAM EXECUTE : transfers data from cache to main array and waits until OIP bit is cleared
			4) WRITE DISABLE
    @return 
    @retval 
*/
NAND_ReturnType NAND_Page_Program(SPI_HandleTypeDef *hspi, NAND_Addr logical_addr, uint8_t *buffer) {

	/* variables for SPI transmissions */
	uint16_t addr;
	uint16_t length_cmd;
	uint16_t length_data;
	NAND_SPI_ReturnType status; 

	/* construct row and column addresses from NAND Address */
	PhysicalAddrs physical_addrs;
	__map_logical_addr_to_physical(&logical_addr, &physical_addrs);

	/* Command 1: WRITE ENABLE */
	__write_enable(hspi);

	/* Command 2: PROGRAM LOAD. See datasheet page 30 for details */
	addr 		 = 0 & (((physical_addrs.plane) << COL_ADDRESS_BITS) | physical_addrs.colAddr);
	length_cmd   = 3;
	length_data  = PAGE_SIZE;
	uint8_t command_load[3] = {SPI_NAND_PROGRAM_LOAD_X1, (addr >> 8), (addr & 0xFF)};
	
	status = NAND_SPI_Send_Command_Data(hspi, command_load, length_cmd, buffer, length_data);
	
	if (status != SPI_OK) {
		return Ret_ProgramFailed;
	}

	/* Command 3: PROGRAM EXECUTE. See datasheet page 31 for details */
	addr         = physical_addrs.rowAddr;
	length_cmd   = 4;
	uint8_t command_exec[4] = {SPI_NAND_PROGRAM_EXEC, (addr >> 16), (addr >> 8), (addr & 0xFF)};
	
	status = NAND_SPI_Send(hspi, command_exec, length_cmd);
	
	if (status != SPI_OK) {
		return Ret_ProgramFailed;
	}

	/* Command 4: WRITE DISABLE */
	__write_disable(hspi);
	
	// TODO: check program fail bit in status register

	return __wait_until_ready(hspi);
}


/******************************************************************************
 *						Move Operations
 *****************************************************************************/







/******************************************************************************
 *						Lock Operations
 *****************************************************************************/








/******************************************************************************
 *						Internal Functions
 *****************************************************************************/


NAND_SPI_ReturnType __write_enable(SPI_HandleTypeDef *hspi) {
	uint8_t command = SPI_NAND_WRITE_ENABLE;
	return NAND_SPI_Send(hspi, &command, 1);
}

NAND_SPI_ReturnType __write_disable(SPI_HandleTypeDef *hspi) {
	uint8_t command = SPI_NAND_WRITE_DISABLE;
	return NAND_SPI_Send(hspi, &command, 1);
}

/**
    @brief
    @note Waits until OIP bit in the Status Register resets again, indicating
	that the NAND Flash is ready for further instructions. If OIP = 1, 
	operation is ongoing, i.e. device is busy.

	Assumption: the device keeps outputting the status register contents
	until another command is issued. This is shown in pages 17 and 31.
	TODO: confirm with an oscilloscope.

    @return NAND_ReturnType
    @retval Ret_Success
 */
NAND_ReturnType __wait_until_ready(SPI_HandleTypeDef *hspi) {
	uint8_t data_rx;
	uint8_t counter = 0;
	uint8_t max_attempts = 10;
	
	/* check once if any operations in progress */
	NAND_ReturnType status = NAND_CHECK_OIP(hspi);

	/* if busy, keep polling for until reaching max_attempts. if still busy, return busy */
	if (status == Ret_NANDBusy) {
		while ((data_rx & SPI_NAND_OIP) == SPI_NAND_OIP) {
			if (counter < max_timeout_attempts) {
				NAND_SPI_ReceiveData(hspi, &data_rx, 1);
				NAND_Wait(1);
				counter += 1;
			} else {
				return Ret_NANDBusy;
			}
		}
	}
	return Ret_Success;
}


NAND_ReturnType __map_logical_addr_to_physical(NAND_Addr *address, PhysicalAddrs *addr_struct) {
	addr_struct -> plane 	= ADDRESS_2_PLANE(*address);
	addr_struct -> block 	= ADDRESS_2_BLOCK(*address);
	addr_struct -> page  	= ADDRESS_2_PAGE(*address);
	addr_struct -> rowAddr 	= (ADDRESS_2_BLOCK(*address) << ROW_ADDRESS_PAGE_BITS) | ADDRESS_2_PAGE(*address); 
	addr_struct -> colAddr  = ADDRESS_2_COL(*address);

	return Ret_Success;
}
