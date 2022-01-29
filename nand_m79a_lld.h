/************************** Flash Memory Driver ***********************************

    Filename:    nand_m79a_lld.h
    Description: Low-level driver functions for reading and writing to M79a NAND Flash via SPI.

    Version:     0.1
    Author:      Tharun Suresh 

********************************************************************************
    
	Version History.

    Ver.    Date            Comments

    0.1     Jan 2022        In Development

********************************************************************************

    The following functions are available in this library:



********************************************************************************/

#include "nand_spi.h"

/* Functions Return Codes */
typedef enum {
    Ret_Success,
    Ret_Failed,
    Ret_ResetFailed,
    Ret_WrongID,
    Ret_NANDBusy,
    // Ret_AddressInvalid,
    Ret_RegAddressInvalid,
    // Ret_MemoryOverflow,
    // Ret_BlockEraseFailed,
    // Ret_PageNrInvalid,
    // Ret_SubSectorNrInvalid,
    // Ret_SectorNrInvalid,
    // Ret_FunctionNotSupported,
    // Ret_NoInformationAvailable,
    // Ret_OperationOngoing,
    // Ret_OperationTimeOut,
    Ret_ReadFailed,
    Ret_ProgramFailed,
	Ret_EraseFailed,
    // Ret_SectorProtected,
    // Ret_SectorUnprotected,
    // Ret_SectorProtectFailed,
    // Ret_SectorUnprotectFailed,
    // Ret_SectorLocked,
    // Ret_SectorUnlocked,
    // Ret_SectorLockDownFailed,
    Ret_WrongType
} NAND_ReturnType;

/* List of supported devices */
#define MT29F2G01ABAGD

#ifdef MT29F2G01ABAGD

    /* device ID */
    typedef struct {
        uint8_t manufacturer_ID;
        uint8_t device_ID;
    } NAND_ID;
    #define NAND_ID_MANUFACTURER    0x2C
    #define NAND_ID_DEVICE          0x24

    /* device details, see Memory Mapping (Datasheet page 11) */
    #define FLASH_WIDTH             8               /* Flash data width */
    #define FLASH_SIZE_BYTES        0x10000000      /* Flash size in bytes */
    #define NUM_BLOCKS              2048            /* Total number of blocks in the device*/
    #define NUM_PAGES_PER_BLOCK     64              /* Number of pages per block*/
    #define PAGE_SIZE               2176            /* Page size in bytes */
    #define PAGE_DATA_SIZE          2048            /* Page data size in bytes */
    #define PAGE_SPARE_SIZE         128             /* Page spare size in bytes*/

    #define BAD_BLOCK_BYTE          PAGE_DATA_SIZE
    #define BAD_BLOCK_VALUE         0x00;

    /*
    Page data only:
        1 page  => 2048 bytes                        = 2048 bytes/page
        1 block => 2048 bytes/page * 64 pages/block  = 131072 bytes/block = 128 KB/block
        device  => 131072 bytes/block * 2048 blocks  = 268,435,456 bytes (256 MB, 0x10_000_000 addrs)

    With spares:
        1 page  => (2048 data + 128 bytes spare)     = 2176 bytes/page
        1 block => 2176 bytes/page * 64 pages/block  = 139264 bytes/block = 136 KB/block
        device  => 139264 bytes/block * 2048 blocks  = 285,212,672 bytes (2176 Mb, 272 MB, 0x11_000_000 addrs)
    */

    /* ADDRESSING DEFINITIONS (see Datasheet page 11) */
    typedef uint32_t NAND_Addr; // logical address type. Max FLASH_SIZE_BYTES

    #define ROW_ADDRESS_BLOCK_BITS   11
    #define ROW_ADDRESS_PAGE_BITS    6
    #define ROW_ADDRESS_BITS         24
    #define COL_ADDRESS_BITS         12
    typedef struct {
        uint16_t plane       : 1;                       // 1 bit to specify plane number
        uint16_t block       : ROW_ADDRESS_BLOCK_BITS;  // block number
        uint16_t page        : ROW_ADDRESS_PAGE_BITS;   // page number
        uint32_t rowAddr     : ROW_ADDRESS_BITS;        // block/page address
        uint32_t colAddr     : COL_ADDRESS_BITS;        // starting address within a page for writes
    } PhysicalAddrs;

    /* physical address macros; Input address must be of type NAND_Addr */
    #define ADDRESS_2_BLOCK(Address)    ((uint16_t) (Address >> 17))   // divide by 131072 (2^17 bytes per block)
    #define ADDRESS_2_PLANE(Address)    (ADDRESS_2_BLOCK(Address) & 1) // get the last bit of the block number
    #define ADDRESS_2_PAGE(Address)     ((uint16_t) ((Address >> 11) & 0x3F))
    #define ADDRESS_2_COL(Address)      ((uint32_t) (Address & 0x07FF)) // take last 11 bits of address

    /* bit macros */
    #define CHECK_OIP(status_reg)       (status_reg & SPI_NAND_OIP) // returns 1 if OIP bit is 1 and device is busy

    /* Command Code Definitions (see Datasheet page 13) */
    typedef enum {
        SPI_NAND_RESET                  = 0xFF,
        SPI_NAND_GET_FEATURES           = 0x0F, 
        SPI_NAND_SET_FEATURES           = 0x1F,
        SPI_NAND_READ_ID                = 0x9F,
        SPI_NAND_PAGE_READ              = 0x13,
        SPI_NAND_READ_PAGE_CACHE_RANDOM = 0x30,
        SPI_NAND_READ_PAGE_CACHE_LAST   = 0x3F,
        SPI_NAND_READ_CACHE_X1          = 0x03,
        SPI_NAND_READ_CACHE_X2          = 0x3B, // dual wire I/O
        SPI_NAND_READ_CACHE_X4          = 0x6B, // quad wire I/O
        SPI_NAND_READ_CACHE_DUAL_IO     = 0xBB,
        SPI_NAND_READ_CACHE_QUAD_IO     = 0xEB, 
        SPI_NAND_WRITE_ENABLE           = 0x06, // sets WEL bit in Status Register to 1
        SPI_NAND_WRITE_DISABLE          = 0x04, // clears WEL bit
        SPI_NAND_BLOCK_ERASE            = 0xD8, 
        SPI_NAND_PROGRAM_EXEC           = 0x10,
        SPI_NAND_PROGRAM_LOAD_X1        = 0x02,
        SPI_NAND_PROGRAM_LOAD_X4        = 0x32,
        SPI_NAND_PROGRAM_LOAD_RANDOM_X1 = 0x84,
        SPI_NAND_PROGRAM_LOAD_RANDOM_X4 = 0x34,
        SPI_NAND_PERMANENT_BLK_LOCK     = 0x2C     // permanently protect a group of blocks
    } CommandCodes;

    /* Using Get Feature command, we can access one of four registers */
    /* Register addrs (see Datasheet page 37) */
    typedef enum {
        SPI_NAND_BLKLOCK_REG_ADDR = 0xA0,
        SPI_NAND_CFG_REG_ADDR     = 0xB0,  
        SPI_NAND_STATUS_REG_ADDR  = 0xC0,
        SPI_NAND_DIE_SEL_REG_ADDR = 0xD0,
    } RegisterAddr;

    /* Block Lock Register Definitions (see Datasheet page 37)
    *
    *   BL7 - BRWD
    *   BL6 - BP3
    *   BL5 - BP2
    *   BL4 - BP1
    *   BL3 - BP0
    *   BL2 - TB
    *   BL1 - WP#/HOLD# Disable
    *   BL0 - reserved
    *
    * (*) If BRWD (block register write disable) is enabled and WP# is LOW and WP#/HOLD# is LOW (default)
    *     then the block lock registers [7:2] cannot be changed
    *  
    * See datasheet page 38 for explanation on BPx bits
    */
    typedef enum {
        SPI_NAND_BRWD   = (1 << 7), /* block register write disable */
        SPI_NAND_BP     = (1 << 6) | (1 << 5) | (1 << 4) | (1 << 3), 
        SPI_NAND_TB     = (1 << 2),
        SPI_NAND_WP_D   = (1 << 1), /* Write protect#/hold# disable */
    } BlockLockRegBits;

    /* Configuration Register Definitions (see Datasheet page 37)
    *
    *   CR7 - CFG2
    *   CR6 - CFG1
    *   CR5 - LOT_EN
    *   CR4 - ECC_EN
    *   CR3 - reserved
    *   CR2 - reserved
    *   CR1 - CFG0
    *   CR0 - reserved
    */
    typedef enum {
        SPI_NAND_CFG    = (1 << 7) | (1 << 6) | (1 << 1), 
        SPI_NAND_LOT_EN = (1 << 5),
        SPI_NAND_ECC_EN = (1 << 4),
    } ConfigRegBits;

    /* Status Register Definitions (see Datasheet page 43)
    *
    *   SR7 - CRBSY Cache Read Busy = '1' when READ PAGE CACHE RANDOM operation is ongoing
    *   SR6 - ECC status (*)
    *   SR5 - ECC status (*)
    *   SR4 - ECC status (*)
    *   SR3 - P_Fail Program fail
    *   SR2 - E_Fail Erase fail
    *   SR1 - WEL Write enable
    *   SR0 - OIP Operation in progress. When 0, device is ready.
    *
    * (*) ECC status bits (SR6,SR5,SR4) should be interpreted as follows:
    *   000 = no errors;
    *   001 = 1-3 bit errors corrected;
    *   010 = bit errors > 8 bits and NOT corrected;
    *   011 = 4-6 bit errors detected and corrected;
    *   101 = 7-8 bit errors detected and corrected;
    *   others = reserved.
    * 
    * Status register:
    *   - can't be changed by SET FEATURE command. 
    *   - can be read by GET FEATURE command.
    *   - Only WEL is writable, by using WRITE DISABLE and WRITE ENABLE commands.
    */

    /* Values for status register (see Datasheet page 43) */
    typedef enum {
        SPI_NAND_CRBSY = (1 << 7), /* Cache Read Busy */
        SPI_NAND_ECC   = (1 << 6) | (1 << 5) | (1 << 4),  /* ECC bits */
        SPI_NAND_PF    = (1 << 3), /* program fail */
        SPI_NAND_EF    = (1 << 2), /* erase fail */
        SPI_NAND_WEL   = (1 << 1), /* write enable latch */
        SPI_NAND_OIP   = (1 << 0), /* operation in progress */
    } StatusRegBits;

    /* Die Select Register Definitions (see Datasheet page 37)
    *   DR6     - DS0
    *   others  - reserved
    */
    typedef enum {
        SPI_NAND_DS0    = (1 << 6), 
    } DieSelRegBits;

    /* Page read mode (see Datasheet page 16) */
    typedef enum {
        ReadFromCache,
        ReadFromCacheX2,
        ReadFromCacheX4,
        ReadFromCacheDualIO,
        ReadFromCacheQuadIO,
    } PageReadMode;

    /* Time constants, in ms (see datasheet pages )
     * These are rounded up to the nearest ms for use with HAL_Delay() */
    #define T_POR           2    /* Power-On/Reset Time : Minimum time after power on or reset: 1.25 ms */
    #define TIME_MAX_ERS    0
    #define TIME_MAX_PGM    0

#endif


/******************************************************************************
 *                            Internal Functions
 *****************************************************************************/
NAND_SPI_ReturnType __write_enable(SPI_HandleTypeDef *hspi);
NAND_SPI_ReturnType __write_disable(SPI_HandleTypeDef *hspi);

/******************************************************************************
 *                            List of APIs
 *****************************************************************************/

/* status operations */
NAND_ReturnType NAND_Reset(SPI_HandleTypeDef *hspi);
NAND_ReturnType NAND_Wait_Until_Ready(SPI_HandleTypeDef *hspi);

/* identification operations */
NAND_ReturnType NAND_Read_ID(SPI_HandleTypeDef *hspi, NAND_ID *nand_ID);
// NAND_ReturnType NAND_Read_Param_Page(SPI_HandleTypeDef *hspi, param_page_t *ppage);

/* feature operations */
NAND_ReturnType NAND_Check_Busy(SPI_HandleTypeDef *hspi);
NAND_ReturnType NAND_Get_Features(SPI_HandleTypeDef *hspi, RegisterAddr reg_addr, uint8_t *reg);
NAND_ReturnType NAND_Set_Features(SPI_HandleTypeDef *hspi, RegisterAddr reg_addr, uint8_t reg);

/* read operations */
NAND_ReturnType NAND_Page_Read(SPI_HandleTypeDef *hspi, PhysicalAddrs *addr, uint8_t *buffer, uint16_t length);
// NAND_ReturnType NAND_Spare_Read(SPI_HandleTypeDef *hspi, PhysicalAddrs *addr, uint8_t *buffer);

/* write operations */
NAND_ReturnType NAND_Page_Program(SPI_HandleTypeDef *hspi, PhysicalAddrs *addr, uint8_t *buffer, uint16_t length);
// NAND_ReturnType NAND_Spare_Program(SPI_HandleTypeDef *hspi, PhysicalAddrs *addrs, uint8_t *buffer);

/* erase operation */
NAND_ReturnType NAND_Block_Erase(SPI_HandleTypeDef *hspi, PhysicalAddrs *addr);

/* internal data move operations */
// NAND_ReturnType NAND_Copy_Back(SPI_HandleTypeDef *hspi, NAND_Addr src_addr, NAND_Addr dest_addr);

/* block lock operations */
// NAND_ReturnType NAND_Lock(void);
// NAND_ReturnType NAND_Unlock(NAND_Addr start_block, NAND_Addr end_block);
// NAND_ReturnType NAND_Read_Lock_Status(NAND_Addr block_addr);
