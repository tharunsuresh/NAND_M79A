# SPI NAND Flash Memory Drivers

   Version:     0.1
   Author:      Tharun Suresh 

## Version History

Ver. |      Date      |     Comments     |
---  |      ---       |       ---        |
0.1	 |	  Jan 2021    |  In Development  | 					


## Contents

- nand_m79a:
  - Low level functions for reading and writing to M79a NAND Flash 
  - Uses STM32L0 HAL Library for SPI communication with the Flash IC
- write_images:
  - High level functions dealing with images 

## Usage 

- Clone this folder to Drivers/ in the STM32 IDE generated folder structure. 
- Add to project by going to Project -> Properties -> C/C++ General -> Paths and Symbols -> Includes

## References 
- SPI Drivers for M68A NAND Flash from Micron [v1, 2011](https://media-www.micron.com/-/media/client/global/documents/products/nand-flash-software/mt29f_1gb-32gb_nand_driver.zip?rev=d418de6415a44bc98a55d30068b30494)
- SPI Drivers for MT29F x8, x16 NAND Flash from Micron [v1.1, 2013](https://micron.com)


