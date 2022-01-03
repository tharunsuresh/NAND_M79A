# SPI NAND Flash Memory Drivers

   Version:     0.1
   Author:      Tharun Suresh 

## Version History

Ver. |      Date      |     Comments     |
---  |      ---       |       ---        |
0.1	 |	  Jan 2022    |  In Development  | 					


## Contents

In order of high level functions => hardware level: 
- nand_m79a:
  - Low level drivers for M79a NAND Flash ICs. 
- spi:
  - SPI wrapper functions used by NAND driver
  - Calls STM32L0 HAL Library to interface with hardware

## Usage 

- Clone this folder to Drivers/ in the STM32 IDE generated folder structure. 
- Add to project by going to Project -> Properties -> C/C++ General -> Paths and Symbols -> Includes
- Add `#include "nand_m79a.h"` to main.c
- Make sure SPI and GPIO are set up (see `NAND_SPI_Init` and `NAND_GPIO_Init` in nand_spi.c for expected settings)

## References 
- SPI Drivers for M68A NAND Flash from Micron [v1, 2011](https://media-www.micron.com/-/media/client/global/documents/products/nand-flash-software/mt29f_1gb-32gb_nand_driver.zip?rev=d418de6415a44bc98a55d30068b30494)
- SPI Drivers for MT29F x8, x16 NAND Flash from Micron [v1.1, 2013](https://micron.com)


