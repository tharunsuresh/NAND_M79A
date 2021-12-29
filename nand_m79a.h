/************************** Flash Memory Driver ***********************************

   Filename:    nand_m79a.h
   Description: Functions for reading and writing to M79a NAND Flash. Uses HAL SPI calls for STM32L0 series.

   Version:     0.1
   Author:      Tharun Suresh 

********************************************************************************

   Version History.

   Ver.		Date			Comments

   0.1		Jan 2022 		In Development

********************************************************************************
  
  The following functions are available in this library:



********************************************************************************/

#include "stm32l0xx_hal.h"

// TODO: update this
// #define FlashAddressMask 0x083Ful
// #define FlashPageSize 0x0840ul

// TODO: check this 
// #define SE_TIMEOUT 3 	/* timeout in seconds suggested for Sector Erase Operation */

#define DUMMY_BYTE	0
#define NAND_SPI_TIMEOUT 5000

/* Functions Return Codes */
typedef enum {
	Ret_Success,
	Ret_ResetFailed,
	Ret_WrongID,
	Ret_NANDBusy,
	// Ret_AddressInvalid,
	// Ret_RegAddressInvalid,
	// Ret_MemoryOverflow,
	// Ret_BlockEraseFailed,
	// Ret_PageNrInvalid,
	// Ret_SubSectorNrInvalid,
	// Ret_SectorNrInvalid,
	// Ret_FunctionNotSupported,
	// Ret_NoInformationAvailable,
	// Ret_OperationOngoing,
	// Ret_OperationTimeOut,
	// Ret_ProgramFailed,
	// Ret_SectorProtected,
	// Ret_SectorUnprotected,
	// Ret_SectorProtectFailed,
	// Ret_SectorUnprotectFailed,
	// Ret_SectorLocked,
	// Ret_SectorUnlocked,
	// Ret_SectorLockDownFailed,
	Ret_WrongType
} NANDReturnType;

/* List of supported devices */
#define MT29F2G01ABAGD

#ifdef MT29F2G01ABAGD

	/* device ID */
	typedef struct {
		uint8_t manufacturer_ID;
		uint8_t device_ID;
	} NAND_ID;
	#define NAND_ID_MANUFACTURER 0x2C
	#define NAND_ID_DEVICE 0x24

	/* device details, see Memory Mapping in the Datasheet */
	#define FLASH_WIDTH				8				/* Flash data width */
	#define FLASH_SIZE_BYTES		0x11000000		/* Flash size in bytes */
  	#define NUM_BLOCKS				2048			/* Total number of blocks in the device*/
	#define NUM_PAGE_BLOCK			64				/* Number of pages per block*/
	#define PAGE_SIZE				2176			/* Page size in bytes */
	#define PAGE_DATA_SIZE			2048			/* Page data size in bytes */
	#define PAGE_SPARE_SIZE			128				/* Page spare size in bytes*/

    // 1 page   = (2048 data + 128 bytes spare)     =   2176 bytes / page 
    // 1 block  = 2176 bytes/page * 64 pages/block  = 139264 bytes / block = 136 KB/block
    // device   = 139264 bytes/block * 2048 blocks  = 285,212,672 bytes or 2176 Mbits

	/* utility macros */
    // TODO: check this. are these still accurate?
	// #define ADDRESS_2_BLOCK(Address)	((MT_uint16) (Address >> 18))
	// #define ADDRESS_2_PAGE(Address)		((MT_uint8)  ((Address >> 12) & 0x3F))
	// #define ADDRESS_2_COL(Address)		((MT_uint16) (Address & 0x0FFF))

	/* SPI NAND Command Set Definitions (see Datasheet page 13) */
	typedef enum {
		SPI_NAND_RESET 						= 0xFF,
		SPI_NAND_GET_FEATURES	 			= 0x0F, 
		SPI_NAND_SET_FEATURES 				= 0x1F,
		SPI_NAND_READ_ID 					= 0x9F,
		SPI_NAND_PAGE_READ	 				= 0x13,
		SPI_NAND_READ_CACHE		 			= 0x03,
		SPI_NAND_READ_CACHE_X2	 			= 0x3B, // dual wire I/O
		SPI_NAND_READ_CACHE_X4	 			= 0x6B, // quad wire I/O
		SPI_NAND_WRITE_ENABLE 				= 0x06, // sets WEL bit in Status Register to 1
		SPI_NAND_WRITE_DISABLE 				= 0x04, // clears WEL bit
		SPI_NAND_BLOCK_ERASE	 			= 0xD8, 
		SPI_NAND_PROGRAM_EXEC	 			= 0x10,
		SPI_NAND_PROGRAM_LOAD	 			= 0x02,
		SPI_NAND_PROGRAM_LOAD_RANDOM	 	= 0x84
	} CommandCodes;

	/* Using Get Feature command, we can access one of four registers */
	/* Register type (see Datasheet page 37) */
	typedef enum {
		SPI_NAND_BLKLOCK_REG_ADDR 	= 0xA0,
		SPI_NAND_CFG_REG_ADDR 		= 0xB0,  
		SPI_NAND_STATUS_REG_ADDR 	= 0xC0,
		SPI_NAND_DIE_SEL_REG_ADDR   = 0xD0,
	} RegisterAddr;

	/* Status Register Definitions (see Datasheet page 43) */

	/* Status register description:
	*
	* SR7 - CRBSY Cache Read Busy
	* SR6 - ECC status (*)
	* SR5 - ECC status (*)
	* SR4 - ECC status (*)
	* SR3 - P_Fail Program fail
	* SR2 - E_Fail Erase fail
	* SR1 - WEL Write enable
	* SR0 - OIP Operation in progress
	*
	* (*) ECC status bits (SR6,SR5,SR4) should be interpreted as follows:
	*      000 = no errors;
	*      001 = 1-3 bit errors corrected;
	*      010 = bit errors > 8 bits and NOT corrected;
	*      011 = 4-6 bit errors detected and corrected;
	*      101 = 7-8 bit errors detected and corrected;
	*      others = reserved.
	* 
	* Status register:
	*      - can't be changed by SET FEATURE command. 
	*      - can be read by GET FEATURE command.
	*      - Only WEL is writable, by using WRITE DISABLE and WRITE ENABLE commands.
	*/

	/* Values for status register (see Datasheet page 43) */
	enum {
		SPI_NAND_PF   = 0x08, /* program fail */
		SPI_NAND_EF   = 0x04, /* erase fail */
		SPI_NAND_WE   = 0x02, /* write enable */
		SPI_NAND_OIP  = 0x01, /* operation in progress */
	};

	/* Block Lock bits (see Datasheet page 37)
	*
	* BL7 - BRWD
	* BL6 - BP3
	* BL5 - BP2
	* BL4 - BP1
	* BL3 - BP0
	* BL2 - TB
	* BL1 - WP#/HOLD# Disable
	* BL0 - reserved
	*
	* (*) If BRWD (block register write disable) is enabled and WP# is LOW and WP#/HOLD# is LOW (default)
	*     then the block lock registers [7:2] cannot be changed
	* 
	* See datasheet page 38 for what the BPx bits mean
	*/
	enum {
		SPI_NAND_BRWD 	= 0x80, // block register write disable
		SPI_NAND_BP3    = 0x40,
		SPI_NAND_BP2 	= 0x20,
		SPI_NAND_BP1 	= 0x10,
		SPI_NAND_BP0 	= 0x08,
	};

	/* Page read mode (see Datasheet page 16) */
	typedef enum _PageReadMode {
		ReadFromCache,
		ReadFromCacheX2,
		ReadFromCacheX4,
		ReadFromCacheDualIO,
		ReadFromCacheQuadIO,
	} PageReadMode;

	/* Time constants, in ms (see datasheet pages )
	 * These are rounded up to the nearest ms for use with HAL_Delay() */
	#define T_POR	 			2    /* Power-On/Reset Time : Minimum time after power on or reset: 1.25 ms */
	#define TIME_MAX_ERS 		0
	#define TIME_MAX_PGM		0


#endif


/******************************************************************************
 *									List of APIs
 *****************************************************************************/

NANDReturnType NAND_Init(SPI_HandleTypeDef *hspi);
NANDReturnType NAND_Reset(SPI_HandleTypeDef *hspi);
NANDReturnType NAND_Check_Busy(SPI_HandleTypeDef *hspi);

/* identification operations */
NANDReturnType NAND_Read_ID(SPI_HandleTypeDef *hspi, NAND_ID *nand_ID);
// NANDReturnType NAND_Read_Param_Page(SPI_HandleTypeDef *hspi, param_page_t *ppage);

/* feature operations */
// NANDReturnType NAND_Get_Feature(SPI_HandleTypeDef *hspi, uint8_t feature_address, uint8_t subfeature);
// NANDReturnType NAND_Set_Feature(SPI_HandleTypeDef *hspi, uint8_t feature_address, uint8_t subfeature);
uint8_t NAND_Read_Status_Reg(SPI_HandleTypeDef *hspi);

 /* read operations */
// NANDReturnType NAND_Page_Read(nand_addr_t addr, width *buffer, uint32 length);
// NANDReturnType NAND_Spare_Read(nand_addr_t addr, width *buffer, uint32 length);

 /* erase operations */
// NANDReturnType NAND_Block_Erase(nand_addr_t addr);

 /* program operations */
// NANDReturnType NAND_Page_Program(nand_addr_t addr, width *buffer, uint32 length);
// NANDReturnType NAND_Spare_Program(nand_addr_t addr, width *buffer, uint32 length);

 /* internal data move operations */
// NANDReturnType NAND_Copy_Back(nand_addr_t src_addr, nand_addr_t dest_addr);

 /* block lock operations */
// NANDReturnType NAND_Lock(void);
// NANDReturnType NAND_Unlock(nand_addr_t start_block, nand_addr_t end_block);
// NANDReturnType NAND_Read_Lock_Status(nand_addr_t block_addr);
