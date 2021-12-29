/*******************************************************************************

  Filename:		nand_spi.h
  Description:  Header file for SPI NAND interface



********************************************************************************

   Version History.

   Ver.				No		Date     	Comments


*******************************************************************************/




/******************************************************************************
 *									List of APIs
 *****************************************************************************/

void  SPI_Init();
void  SPI_Open(void);
void  SPI_SendCmd(bus_t ubCommand);
void  SPI_SendAddr(bus_t ubAddress);
void  SPI_SendData(bus_t data);
bus_t SPI_ReadData(void);
void  SPI_SetWriteProtect(void);
void  SPI_UnsetWriteProtect(void);
void  SPI_Wait(int microseconds);
void  SPI_Close(void);