# SPI NAND Flash Memory Drivers

Drivers for Micron NAND Flash 
- Supported Models
  - MT29F2G01ABAGD
- Dependencies
  - STM32 L0 Series Hardware Abstraction Library (HAL) 

Note: The STM32L0 HAL is used for SPI x1 data transfers to and from the NAND Flash IC. This driver set is meant to be used with STM32 Microcontrollers.
  
## Version History

Ver. |      Date      |     Comments     |
---  |      ---       |       ---        |
0.1	 |	  Jan 2022    |  In Development  | 					


## Contents

In order of high level functions => hardware: 
- nand_m79a:
  - Functions for reading and writing to M79a NAND Flash ICs
- nand_m79a_lld:
  - Low level drivers implementing individual commands and dealing with physical locations within the NAND
- nand_spi:
  - SPI wrapper functions used by NAND driver
  - Calls STM32L0 HAL Library to interface with hardware

## Usage 

- Clone this folder to Drivers/ in the STM32 IDE generated folder structure. 
- Add to project by going to Project -> Properties -> C/C++ General -> Paths and Symbols -> Includes
- Add `#include "nand_m79a.h"` to main.c
- Make sure SPI and GPIO are set up (see `NAND_SPI_Init` and `NAND_GPIO_Init` in nand_spi.c for expected settings)

## References 

### Documents
- Micron Product Catalog: [MT29F2G01ABAGD](https://www.micron.com/products/nand-flash/serial-nand/part-catalog/mt29f2g01abagdwb-it)
  - See link for Datasheet and Technical Note TN-29-17: NAND Flash Design and Use Considerations
- Technical Note [TN-29-19: NAND Flash 101](https://media-www.micron.com/-/media/client/global/documents/products/technical-note/nand-flash/tn2919_nand_101.pdf?rev=3774f4d24cec419382e02941b460e286)

### Similar Drivers
- SPI Drivers for M68A NAND Flash from Micron [Download: v1, 2011](https://media-www.micron.com/-/media/client/global/documents/products/nand-flash-software/mt29f_1gb-32gb_nand_driver.zip?rev=d418de6415a44bc98a55d30068b30494)
- SPI Drivers for MT29F x8, x16 NAND Flash from Micron [Download: v1.1, 2013](https://micron.com)
  - Technical Note TN-29-72: Software Drivers for MT29F Micron NAND Flash accompanies these drivers

## To-Do List

### Hardware 
- Validate driver functions

### Low level driver features (nand_m79a_lld)
- Finish implementing all of the commands
  - Read all registers, subfeatures
  - Read and write to spare areas
  - Move, lock operations
  - Read bad block bytes
  - Parameter page [Low priority]
  - OTP areas [Low priority]

- Optimize functions
  - NAND_Wait(1) wastes time. 
    - No command takes that long to complete.
    - Compounds when calling the function multiple times.

### Higher level features (nand_m79a)
- Handling storage levels and freeing up space as required [High priority]
- Bad-block management and mapping from logical addresses to physical locations [High priority]
- Wear leveling 
- Error correction code (ECC)
