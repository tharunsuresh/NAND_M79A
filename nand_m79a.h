/************************** Flash Memory Driver ***********************************

   Filename:    nand_m79a.h
   Description: Hardware-independent functions for reading and writing to M79a NAND Flash.

   Version:     0.1
   Author:      Tharun Suresh 

********************************************************************************

   Version History.

   Ver.		Date			Comments

   0.1		Dec 2021 		In Development 

********************************************************************************
  
  The following functions are available in this library:



********************************************************************************/

#include "nand_spi.h";

/* Basic Data-types */
typedef unsigned char        MT_uint8;
typedef signed char          MT_sint8;
typedef unsigned short       MT_uint16;
typedef signed short         MT_sint16;
typedef unsigned int         MT_uint32;
typedef signed int           MT_sint32;

typedef MT_uint32 uAddrType;

// TODO: check this 
// #define SE_TIMEOUT 3 	/* timeout in seconds suggested for Sector Erase Operation */
#define TRUE 1
#define FALSE 0

/* Driver status constants */
#define DRIVER_STATUS_INITIALIZED		0 	/* normal driver state */
#define DRIVER_STATUS_NOT_INITIALIZED	1 	/* driver is not initialized */

/* Functions Return Codes */
typedef enum {
	Op_Success,
	Op_ResetFailed,
	// Op_AddressInvalid,
	// Op_RegAddressInvalid,
	// Op_MemoryOverflow,
	// Op_BlockEraseFailed,
	// Op_PageNrInvalid,
	// Op_SubSectorNrInvalid,
	// Op_SectorNrInvalid,
	// Op_FunctionNotSupported,
	// Op_NoInformationAvailable,
	// Op_OperationOngoing,
	// Op_OperationTimeOut,
	// Op_ProgramFailed,
	// Op_SectorProtected,
	// Op_SectorUnprotected,
	// Op_SectorProtectFailed,
	// Op_SectorUnprotectFailed,
	// Op_SectorLocked,
	// Op_SectorUnlocked,
	// Op_SectorLockDownFailed,
	Op_WrongType
} ReturnType;

// TODO: update this
// #define FlashAddressMask 0x083Ful
// #define FlashPageSize 0x0840ul

/* List of supported devices */
#define MT29F2G01ABAGD

#ifdef MT29F2G01ABAGD
	/* device details, see Memory Mapping in the Datasheet */
	typedef MT_uint8 bus_t;							/* Flash data type */
	#define FLASH_WIDTH				8				/* Flash data width */
	#define FLASH_SIZE				0x11000000		/* Flash size in bytes */
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
	} CommandCode;

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

	/* Time constants, in µs (see datasheet pages ) */
	#define T_POR	 			1250    /* Power on Reset Time (device initialization) from VCC Min */
	#define TIME_MAX_ERS 		10000	
	#define TIME_MAX_PGM		900000

	#define DUMMY_BYTE			0

	typedef struct {
		MT_uint8 manufacturer_ID;
		MT_uint8 device_ID;
	} ID;

#endif


/******************************************************************************
 *									List of APIs
 *****************************************************************************/

MT_uint8 Init_Driver(void);

/* reset operations */
ReturnType NAND_Reset(void);

/* identification operations */
ID NAND_Read_ID(ID device_ID);
// MT_uint8 NAND_Read_ID_ONFI(Op_width *buf);
// MT_uint8 NAND_Read_Param_Page(param_page_t *ppage);

/* feature operations */
// MT_uint8 NAND_Get_Feature(Op_width feature_address, Op_width *subfeature);
// MT_uint8 NAND_Set_Feature(Op_width feature_address, Op_width subfeature);
// Op_width NAND_Read_Status();
// Op_width NAND_Read_Status_Enhanced(nand_addr_t addr);

// /* read operations */
// MT_uint8 NAND_Page_Read(nand_addr_t addr, Op_width *buffer, MT_uint32 lenght);
// MT_uint8 NAND_Spare_Read(nand_addr_t addr, Op_width *buffer, MT_uint32 lenght);

// /* erase operations */
// MT_uint8 NAND_Block_Erase(nand_addr_t addr);

// /* program operations */
// MT_uint8 NAND_Page_Program(nand_addr_t addr, Op_width *buffer, MT_uint32 lenght);
// MT_uint8 NAND_Spare_Program(nand_addr_t addr, Op_width *buffer, MT_uint32 lenght);

// /* internal data move operations */
// MT_uint8 NAND_Copy_Back(nand_addr_t src_addr, nand_addr_t dest_addr);

// /* block lock operations */
// MT_uint8 NAND_Lock(void);
// MT_uint8 NAND_Unlock(nand_addr_t start_block, nand_addr_t end_block);
// MT_uint8 NAND_Read_Lock_Status(nand_addr_t block_addr);
