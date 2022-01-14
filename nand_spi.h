
/************************** NAND SPI Functions ***********************************

   Filename:    nand_spi.h
   Description: Implements SPI wrapper functions for use by low-level drivers.
                Uses HAL library for STM32L0 series.

   Version:     0.1
   Author:      Tharun Suresh

********************************************************************************

   Version History.

   Ver.		Date			   Comments

   0.1		Jan 2022 	   In Development

********************************************************************************
  
  The following functions are available in this library:



********************************************************************************/

#include "stm32l0xx_hal.h"

#define NAND_NCS_PIN    GPIO_PIN_12
#define NAND_SCK_PIN    GPIO_PIN_13
#define NAND_MISO_PIN   GPIO_PIN_14
#define NAND_MOSI_PIN   GPIO_PIN_15

#define NAND_MOSI_PORT  GPIOB
#define NAND_MISO_PORT  GPIOB
#define NAND_SCK_PORT   GPIOB
#define NAND_NCS_PORT   GPIOB

#define DUMMY_BYTE	      0
#define NAND_SPI_TIMEOUT   100


/******************************************************************************
 *									Internal Functions
 *****************************************************************************/

   void __nand_spi_cs_low(void); 
   void __nand_spi_cs_high(void); 

/******************************************************************************
 *									List of APIs
 *****************************************************************************/

   void NAND_Wait(uint8_t milliseconds);

   /* Wrapper functions for sending and receiving data */
   HAL_StatusTypeDef NAND_SPI_Send(SPI_HandleTypeDef *hspi, uint8_t *buffer_send, uint16_t length_send);
   HAL_StatusTypeDef NAND_SPI_SendReceive(SPI_HandleTypeDef *hspi,
		   	   	   	   	   	   	uint8_t *buffer_send, uint16_t length_send,
									               uint8_t *buffer_recv, uint16_t length_recv);
   HAL_StatusTypeDef NAND_SPI_Receive(SPI_HandleTypeDef *hspi, uint8_t *buffer_recv, uint16_t length_recv);



   HAL_StatusTypeDef NAND_SPI_Send_CommandData(SPI_HandleTypeDef *hspi, uint8_t *buffer_cmd, uint16_t length_cmd, 
																	                     uint8_t *buffer_data, uint16_t length_data);
/******************************************************************************/
