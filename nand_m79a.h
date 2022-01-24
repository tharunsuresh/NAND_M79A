/************************** Flash Memory Driver ***********************************

    Filename:    nand_m79a.h
    Description: Top NAND controller layer that manages storage and maps logical addresses
                 to physical locations using low level drivers.

    Version:     0.1
    Author:      Tharun Suresh

********************************************************************************

    Version History.

    Ver.        Date            Comments

    0.1        Jan 2022         In Development

********************************************************************************

    The following functions are available in this library:


********************************************************************************/

#include "nand_m79a_lld.h"

// TODO:
// Write higher level functions such as:
//    NAND_Write(start_addr, *buffer, length)
//    NAND_Read(start_addr, end_addr, *buffer) 
//    NAND_Erase(start_addr, end_addr)

// Enable easy memory mapping of filled and available locations
// Manage writing appropriate amount of data to each page

// Map logical addresses from 0x0 to 0xMAX_STORAGE to page numbers, blocks inside the nand IC

// Manage bad blocks, ECC and locking. 
// Possibly more difficult features such as wear leveling

/******************************************************************************
 *                              Internal Functions
 *****************************************************************************/

NAND_ReturnType __map_logical_addr(NAND_Addr *address, PhysicalAddrs *addr_struct);




/******************************************************************************
 *                              List of APIs
 *****************************************************************************/

NAND_ReturnType NAND_Init(SPI_HandleTypeDef *hspi);








