
/************************** NAND SPI Functions ***********************************

   Filename:    nand_spi.h
   Description: Implements SPI wrapper functions for use by low-level drivers.
                  Uses HAL library for STM32L0 series.

   Version:     0.1
   Author:      Tharun Suresh

********************************************************************************

    Version History.

    Ver.    Date            Comments

    0.1     Jan 2022        In Development

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

#define DUMMY_BYTE         0x00
#define NAND_SPI_TIMEOUT   100

/* using custom return type to keep higher layers as platform-agnostic as possible */
typedef enum {
    SPI_OK,
    SPI_Fail
} NAND_SPI_ReturnType;

/* SPI Transaction Parameters */
typedef struct {
    uint8_t *buffer;
    uint16_t length;
} SPI_Params;

/******************************************************************************
 *                              Internal Functions
 *****************************************************************************/

    void __nand_spi_cs_low(void); 
    void __nand_spi_cs_high(void); 

/******************************************************************************
 *                                  List of APIs
 *****************************************************************************/
   
    /* General functions */
    void NAND_Wait(uint8_t milliseconds);

    /* Wrapper functions for sending and receiving data */
    NAND_SPI_ReturnType NAND_SPI_Send(SPI_HandleTypeDef *hspi, SPI_Params *data_send);
    NAND_SPI_ReturnType NAND_SPI_SendReceive(SPI_HandleTypeDef *hspi, SPI_Params *data_send, SPI_Params *data_recv);
    NAND_SPI_ReturnType NAND_SPI_Receive(SPI_HandleTypeDef *hspi, SPI_Params *data_recv);

    NAND_SPI_ReturnType NAND_SPI_Send_Command_Data(SPI_HandleTypeDef *hspi, SPI_Params *cmd_send, SPI_Params *data_send);

/******************************************************************************/
