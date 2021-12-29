/************************** Flash Memory Driver ***********************************

   Filename:    nand_m79a.c
   Description: Hardware-independent for reading and writing to M79a NAND Flash.

   Version:     0.1
   Author:      Tharun Suresh 

********************************************************************************

   Version History.

   Ver.		Date			Comments

   0.1		Dec 2021 		In Development 

********************************************************************************
  
  	The following functions are available in this library:
		MT_uint8 Init_Driver(void);
		ReturnType NAND_Reset(void);


********************************************************************************/

#include "nand_m79a.h";

/* state of the driver */
MT_uint8 driver_status = DRIVER_STATUS_NOT_INITIALIZED;

/* global structure with device info */
struct parameter_page_t device_param_page;


/******************************************************************************
 *						NAND Low Level Driver Functions
 *****************************************************************************/

/**
    This function initialize the driver and it must be called before
    any other function

    @return Return code
    @retval NAND_DRIVER_STATUS_INITIALIZED
    @retval DRIVER_STATUS_NOT_INITIALIZED
 */
MT_uint8 Init_Driver(void) {
	// bus_t onfi_signature[ONFI_SIGNATURE_LENGHT];

	/* check if the driver is previously initialized */
	if(DRIVER_STATUS_INITIALIZED == driver_status)
		return DRIVER_STATUS_INITIALIZED;

	SPI_Init();

	/* Resetting the NAND Flash at startup is usually mandatory but the 
	default Micron boot up setting is the alternative SPI NAND Boot up sequence:
	The device automatically kicks off initialization and reset (see page 50)
 */
	NAND_Reset();

	// update driver status

	// /* read if this device is ONFI complaint */
	// NAND_Read_ID_ONFI(onfi_signature);

	// /* verify ONFI signature in the first field of parameter page */
	// if(strcmp((const char *)onfi_signature, "ONFI") &&
	// 	(NAND_BAD_PARAMETER_PAGE != NAND_Read_Param_Page(&device_info)))
	// driver_status = DRIVER_STATUS_INITIALIZED;

	driver_status = DRIVER_STATUS_INITIALIZED;

	return driver_status;
}

/**
 	Sends command to reset the NAND Flash chip. Returns success when Flash is ready for further instructions.

	Transaction length: 1 byte
	Wait time: Unlimited 

    @return Return code
    @retval Op_Success
*/
ReturnType NAND_Reset(void) { 

	SPI_Open();						// CS# low
   	SPI_SendCmd(SPI_NAND_RESET); 	// send command for reset 
   	SPI_Wait(T_POR);				// wait for 1.25 ms after reset 
	__wait_for_ready();				// wait until OIP bit sets again (Flash is ready for further instructions)
	SPI_Close();					// CS# high

	return Op_Success;
}


/**
    Sends command to read ID of Flash chip. Returns the manufacturer and device ID.
	
	Transaction length: 4 bytes (2 each way)

	@return Struct: ID 
    @retval device_ID

 */

ID NAND_Read_ID(ID device_ID) {

	SPI_Open();						// CS# low
   	SPI_SendCmd(SPI_NAND_READ_ID); 	// send command for ID 
	SPI_SendData(DUMMY_BYTE);	
	device_ID.manufacturer_ID = SPI_ReadData();
	device_ID.device_ID	= SPI_ReadData();
	SPI_Close();					// CS# high

    return device_ID;
}

















/******************************************************************************
 *						Internal Functions
 *****************************************************************************/

/**
    This function waits until OIP bit in the Status Register sets again, indicating 
	that the NAND Flash is ready for further instructions. 

    @return Return code
    @retval Flash_Success

 */

// TODO: Use NAND_Read_Status instead. if that is 
// TODO: if it takes too long, return failure 

ReturnType __wait_for_ready() {
	SPI_SendCmd(SPI_NAND_GET_FEATURES);     // send the command
	SPI_SendAddr(SPI_NAND_STATUS_REG_ADDR);	// send register address

	// see page 17. as long as you keep reading, you get the status register contents 
	// keep reading until OIP bit toggles then return success
	
	// currently it just checks once and returns. 
	if (SPI_NAND_OIP == (SPI_NAND_OIP & SPI_ReadData())) { 
		return Op_Success;
	}
	else {
		return Op_ResetFailed;
	}
	



