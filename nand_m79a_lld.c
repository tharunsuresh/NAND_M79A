/************************** Flash Memory Driver ***********************************

    Filename:    nand_m79a_lld.c
    Description: Low-level driver functions for reading and writing to M79a NAND Flash via SPI.

    Version:     0.1
    Author:      Tharun Suresh

********************************************************************************

    Version History.

    Ver.    Date            Comments

    0.1     Jan 2022        In Development

********************************************************************************

    The following functions are available in this library:


********************************************************************************/

#include "nand_m79a_lld.h"


/******************************************************************************
 *                              Initialization
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

    uint8_t command = SPI_NAND_RESET;
    SPI_Params transmit = { .buffer = &command, .length = 1 };

    NAND_SPI_ReturnType SPI_Status = NAND_SPI_Send(hspi, &transmit);
    NAND_Wait(T_POR);	// wait for T_POR = 1.25 ms after reset

    if (SPI_Status != SPI_OK) {
        return Ret_ResetFailed;
    } else {
        // wait until OIP bit resets again (Flash is ready for further instructions)
        return __wait_until_ready(hspi);
    }
}


/******************************************************************************
 *                      Identification Operations
 *****************************************************************************/

/**
    @brief Sends command to read manufacturer and device ID of NAND flash chip
    @note Transaction length: 4 bytes (2 each way)

    @return NAND_ReturnType
    @retval Ret_ResetFailed
    @retval Ret_Success

    */

    NAND_ReturnType NAND_Read_ID(SPI_HandleTypeDef *hspi, NAND_ID *nand_ID) {

    uint8_t data_tx[] = {SPI_NAND_READ_ID, 0}; // second byte is dummy byte
    uint8_t data_rx[2]; // data buffer for received data

    SPI_Params tx = { .buffer = data_tx, .length = 2 };
    SPI_Params rx = { .buffer = data_rx, .length = 2 };

    NAND_SPI_SendReceive(hspi, &tx, &rx);

    nand_ID -> manufacturer_ID = data_rx[0]; // second last byte from transmission
    nand_ID -> device_ID       = data_rx[1]; // last byte

    return Ret_Success;
}

/******************************************************************************
 *                              Feature Operations
 *****************************************************************************/

/**
    @brief Sends command to read the entire status register
    @note Transaction length: 3 bytes (2 to transmit, 1 to receive)

    @return uint8_t
    @retval Status register contents
*/
uint8_t NAND_Read_Status_Reg(SPI_HandleTypeDef *hspi) {

    uint8_t command[] = {SPI_NAND_GET_FEATURES, SPI_NAND_STATUS_REG_ADDR};
    uint8_t data_rx;

    SPI_Params tx = { .buffer = command,  .length = 2 };
    SPI_Params rx = { .buffer = &data_rx, .length = 1 };

    NAND_SPI_SendReceive(hspi, &tx, &rx);

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
 *                              Read Operations
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
// NAND_ReturnType NAND_Page_Read(SPI_HandleTypeDef *hspi) {
// }


/******************************************************************************
 *                              Erase Operations
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
// NAND_ReturnType NAND_Block_Erase(SPI_HandleTypeDef *hspi) {
// }


/******************************************************************************
 *                              Write Operations
 *****************************************************************************/

/* TODO:
 * The first spare area location in each bad block contains the bad-block mark (0x00).
 * System software should initially check the first spare area location (byte 2048) for non-FFh data on
 * the first page of each block prior to performing any program or erase operations on the
 * NAND Flash device.
 */
/**
    @brief Write 2176 bytes to a page. 
    @note Command sequence:
            1) WRITE ENABLE
            2) PROGRAM LOAD : load data into cache register
            3) PROGRAM EXECUTE : transfers data from cache to main array and waits until OIP bit is cleared
            4) WRITE DISABLE
    @return
    @retval
*/
NAND_ReturnType NAND_Page_Program(SPI_HandleTypeDef *hspi, PhysicalAddrs *addrs, uint8_t *buffer) {

    NAND_SPI_ReturnType status;

    /* Command 1: WRITE ENABLE */
    __write_enable(hspi);

    /* Command 2: PROGRAM LOAD. See datasheet page 30 for details */
    uint32_t col = 0 & (((addrs->plane) << COL_ADDRESS_BITS) | addrs->colAddr);
    uint8_t command_load[3] = {SPI_NAND_PROGRAM_LOAD_X1, (col >> 8), (col & 0xFF)};

    SPI_Params tx_cmd = {.buffer = command_load, .length = 3};
    SPI_Params tx_data = {.buffer = buffer, .length = PAGE_SIZE};

    status = NAND_SPI_Send_Command_Data(hspi, &tx_cmd, &tx_data);

    if (status != SPI_OK) {
        return Ret_ProgramFailed;
    }

    /* Command 3: PROGRAM EXECUTE. See datasheet page 31 for details */
    uint32_t row = addrs->rowAddr;
    uint8_t command_exec[4] = {SPI_NAND_PROGRAM_EXEC, (row >> 16), (row >> 8), (row & 0xFF)};

    tx_cmd = {.buffer = command_exec, .length = 4};

    status = NAND_SPI_Send(hspi, &tx_cmd);

    if (status != SPI_OK) {
        return Ret_ProgramFailed;
    }

    /* Command 4: WRITE DISABLE */
    __write_disable(hspi);

    // TODO: check program fail bit in status register

    return __wait_until_ready(hspi);
}


/******************************************************************************
 *                              Move Operations
 *****************************************************************************/







/******************************************************************************
 *                              Lock Operations
 *****************************************************************************/








/******************************************************************************
 *                              Internal Functions
 *****************************************************************************/


NAND_SPI_ReturnType __write_enable(SPI_HandleTypeDef *hspi) {
    uint8_t command = SPI_NAND_WRITE_ENABLE;
    SPI_Params transmit = { .buffer = &command, .length = 1 };
    return NAND_SPI_Send(hspi, &transmit);
}

NAND_SPI_ReturnType __write_disable(SPI_HandleTypeDef *hspi) {
    uint8_t command = SPI_NAND_WRITE_DISABLE;
    SPI_Params transmit = { .buffer = &command, .length = 1 };
    return NAND_SPI_Send(hspi, &transmit);
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
    uint8_t counter = 0;
    uint8_t max_attempts = 10;

    /* SPI Transaction set up */
    uint8_t data_rx;
    SPI_Params receive = { .buffer = &data_rx, .length = 1 };

    /* check once if any operations in progress */
    NAND_ReturnType status = NAND_Check_OIP(hspi);

    /* if busy, keep polling for until reaching max_attempts. if still busy, return busy */
    if (status == Ret_NANDBusy) {
        while ((data_rx & SPI_NAND_OIP) == SPI_NAND_OIP) {
            if (counter < max_attempts) {
                NAND_SPI_Receive(hspi, &receive);
                NAND_Wait(1);
                counter += 1;
            } else {
                return Ret_NANDBusy;
            }
        }
    }
    return Ret_Success;
}
