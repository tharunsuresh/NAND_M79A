/************************** Flash Memory Driver ***********************************

    Filename:    nand_m79a.c
    Description: Top NAND controller layer that manages storage and maps logical addresses
                to physical locations using low level drivers.

    Version:     0.1
    Author:      Tharun Suresh

********************************************************************************

    Version History.

    Ver.    Date            Comments

    0.1     Jan 2022        In Development

********************************************************************************

    The following functions are available in this library:


********************************************************************************/

#include "nand_m79a.h"




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


/******************************************************************************
 *                              Reads and Writes
 *****************************************************************************/

/**
    @brief 
    @note 

    @return NAND_ReturnType
    @retval 
 */
// NAND_ReturnType func(SPI_HandleTypeDef *hspi) {
// }



/******************************************************************************
 *                              Internal Functions
 *****************************************************************************/

/**
    @brief 
    @note 

    @return NAND_ReturnType
    @retval 
 */
NAND_ReturnType __map_logical_addr(NAND_Addr *address, PhysicalAddrs *addr_struct) {
    addr_struct -> plane    = ADDRESS_2_PLANE(*address);
    addr_struct -> block    = ADDRESS_2_BLOCK(*address);
    addr_struct -> page     = ADDRESS_2_PAGE(*address);
    addr_struct -> rowAddr  = 0 || ((ADDRESS_2_BLOCK(*address) << ROW_ADDRESS_PAGE_BITS) | ADDRESS_2_PAGE(*address));
    addr_struct -> colAddr  = 0 || ((ADDRESS_2_PLANE(*address) << COL_ADDRESS_BITS) | ADDRESS_2_COL(*address));

    return Ret_Success;
}
