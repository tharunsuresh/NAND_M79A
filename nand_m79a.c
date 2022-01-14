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
    @brief 
    @note 

    @return NAND_ReturnType
    @retval 
 */
NAND_ReturnType NAND_Init(SPI_HandleTypeDef *hspi) {
    

}
/******************************************************************************
 *                              Internal Functions
 *****************************************************************************/


NAND_ReturnType __map_logical_addr_to_physical(NAND_Addr *address, PhysicalAddrs *addr_struct) {
    addr_struct -> plane    = ADDRESS_2_PLANE(*address);
    addr_struct -> block    = ADDRESS_2_BLOCK(*address);
    addr_struct -> page     = ADDRESS_2_PAGE(*address);
    addr_struct -> rowAddr  = (ADDRESS_2_BLOCK(*address) << ROW_ADDRESS_PAGE_BITS) | ADDRESS_2_PAGE(*address); 
    addr_struct -> colAddr  = ADDRESS_2_COL(*address);

    return Ret_Success;
}
