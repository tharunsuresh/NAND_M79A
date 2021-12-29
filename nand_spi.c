/*******************************************************************************

  Filename:		nand_spi.c
  Description:  Functions for SPI NAND interface


********************************************************************************

   Version History.

   Ver.				No		Date     	Comments


*******************************************************************************/

#include "nand_m79a.h";
#include "nand_spi.h";

/**
	Initialize HW NAND Controller.
	This function is called during driver initialization.
*/

void SPI_Init(){



};

/**
 	Open NAND device.
 	This function is called every time an I/O is performed.
*/
void SPI_Open(void);

/**
	NAND Command input.
	This function is used to send a command to NAND.
*/
void SPI_SendCmd(bus_t ubCommand);

/**
	NAND Address input.
	This function is used to send an address to NAND.
*/
void SPI_SendAddr(bus_t ubAddress);

/**
	NAND Data input.
	This function is used to send data to NAND.
*/
void SPI_SendData(bus_t data);

/**
	NAND Data output.
	This function is used to read data from NAND.
*/
bus_t SPI_ReadData(void) {


};


// probably better to move these following two to 
/**
	NAND Write protect (set WP = L).
	This function is used to set Write Protect (WP) pin to LOW
*/
void SPI_SetWriteProtect(void);

/**
	NAND Write protect (set WP = H).
	This function is used to set Write Protect (WP) pin to HIGH
*/
void SPI_UnsetWriteProtect(void);

/**
	Wait for microseconds.
	This function should call a platform or OS wait() function.
*/
void SPI_Wait(int microseconds);

/**
	Close HW NAND Controller.
	This function is used to close the NAND HW controller in the right way.
*/
void SPI_Close(void);