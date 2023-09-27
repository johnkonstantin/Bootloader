#ifndef UBERSATSDBOOTLOADER_SD_H
#define UBERSATSDBOOTLOADER_SD_H

#include <stm32f1xx_hal.h>
#include "main.h"
#include <stdio.h>
#include <string.h>

extern Error_Code errorCode;
extern UART_HandleTypeDef huart1;

#define CT_MMC 0x01            // MMC ver 3
#define CT_SD1 0x02            // SD ver 1
#define CT_SD2 0x04            // SD ver 2
#define CT_SDC (CT_SD1|CT_SD2) // SD
#define CT_BLOCK 0x08          // Block addressing

// Definitions for MMC/SDC command
#define CMD0 (0x40+0)    // GO_IDLE_STATE
#define CMD1 (0x40+1)    // SEND_OP_COND (MMC)
#define ACMD41 (0xC0+41) // SEND_OP_COND (SDC)
#define CMD8 (0x40+8)    // SEND_IF_COND
#define CMD9 (0x40+9)    // SEND_CSD
#define CMD16 (0x40+16)  // SET_BLOCKLEN
#define CMD17 (0x40+17)  // READ_SINGLE_BLOCK
#define CMD24 (0x40+24)  // WRITE_BLOCK
#define CMD55 (0x40+55)  // APP_CMD
#define CMD58 (0x40+58)  // READ_OCR

typedef struct SD_Info_t SD_Info;
struct SD_Info_t {
	volatile uint8_t type;
};

uint8_t SD_Init(const SPI_HandleTypeDef* hspi, SD_Info* sdInfo);
uint8_t SPIx_Wait_Ready(const SPI_HandleTypeDef* hspi);
uint8_t SD_Read_Block(const SPI_HandleTypeDef* hspi, uint8_t *buff, uint32_t lba);
uint8_t SD_Write_Block(const SPI_HandleTypeDef* hspi, uint8_t *buff, uint32_t lba);
void SS_SD_Select(void);
static void SS_SD_Deselect(void);
static uint8_t SPIx_WriteRead(const SPI_HandleTypeDef* hspi, uint8_t byte);
static void SPIx_SendByte(const SPI_HandleTypeDef* hspi, uint8_t byte);
static uint8_t SPIx_ReceiveByte(const SPI_HandleTypeDef* hspi);
void SPIx_Release(const SPI_HandleTypeDef* hspi);
static uint8_t SD_cmd(const SPI_HandleTypeDef* hspi, uint8_t cmd, uint32_t arg);


#endif //UBERSATSDBOOTLOADER_SD_H
