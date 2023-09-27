#include "SD.h"

char str1[60] = {0};

uint8_t SD_Init(const SPI_HandleTypeDef* hspi, SD_Info* sdInfo) {
	HAL_Delay(20);
	uint8_t ocr[4];
	uint8_t cmd;
	int16_t tmr;
	sdInfo->type = 0;
	SPI_HandleTypeDef t = *hspi;
	t.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_256;
	HAL_SPI_Init(&t);
	SS_SD_Deselect();
	for (uint32_t i = 0; i < 10; ++i) {
		SPIx_Release(&t);
	}
	HAL_SPI_Init((SPI_HandleTypeDef*)hspi);
	SS_SD_Select();
	if (SD_cmd(hspi, CMD0, 0) == 1) {
		SPIx_Release(hspi);
		if (SD_cmd(hspi, CMD8, 0x1AA) == 1) {
			for (uint8_t i = 0; i < 4; i++) ocr[i] = SPIx_ReceiveByte(hspi);
			snprintf(str1, 60, "OCR: 0x%02X 0x%02X 0x%02X 0x%02X\r\n", ocr[0], ocr[1], ocr[2], ocr[3]);
			HAL_UART_Transmit(&huart1, (uint8_t*)str1, strlen(str1), 0x1000);
			if (ocr[2] == 0x01 && ocr[3] == 0xAA) {
				for (tmr = 12000; tmr && SD_cmd(hspi, ACMD41, 1UL << 30); tmr--);
				if (tmr && SD_cmd(hspi, CMD58, 0) == 0) {
					for (uint8_t i = 0; i < 4; i++) ocr[i] = SPIx_ReceiveByte(hspi);
					sprintf(str1, "OCR: 0x%02X 0x%02X 0x%02X 0x%02X\r\n", ocr[0], ocr[1], ocr[2], ocr[3]);
					HAL_UART_Transmit(&huart1, (uint8_t*)str1, strlen(str1), 0x1000);
					sdInfo->type = (ocr[0] & 0x40) ? CT_SD2 | CT_BLOCK : CT_SD2;
				}
			}
		}
		else {
			if (SD_cmd(hspi, ACMD41, 0) <= 1) {
				sdInfo->type = CT_SD1;
				cmd = ACMD41;
			}
			else {
				sdInfo->type = CT_MMC;
				cmd = CMD1;
			}
			for (tmr = 25000; tmr && SD_cmd(hspi, cmd, 0); tmr--);
			if (!tmr || SD_cmd(hspi, CMD16, 512) != 0) {
				sdInfo->type = 0;
			}
		}
	}
	else {
		return 1;
	}
	sprintf(str1, "Type SD: 0x%02X\r\n", sdInfo->type);
	HAL_UART_Transmit(&huart1, (uint8_t*)str1, strlen(str1), 0x1000);
	return 0;
}

uint8_t SPIx_WriteRead(const SPI_HandleTypeDef* hspi, uint8_t byte) {
	uint8_t receivedbyte = 0;
	if (HAL_SPI_TransmitReceive((SPI_HandleTypeDef*)hspi, (uint8_t*)&byte, (uint8_t*)&receivedbyte, 1, 1000) !=
	    HAL_OK) {
		errorCode = SPI_ERROR;
		Error_Handler();
	}
	return receivedbyte;
}

void SS_SD_Select() {
	HAL_GPIO_WritePin(SD_CS_GPIO_Port, SD_CS_Pin, GPIO_PIN_RESET);
}

void SS_SD_Deselect() {
	HAL_GPIO_WritePin(SD_CS_GPIO_Port, SD_CS_Pin, GPIO_PIN_SET);
}

void SPIx_SendByte(const SPI_HandleTypeDef* hspi, uint8_t byte) {
	SPIx_WriteRead(hspi, byte);
}

uint8_t SPIx_ReceiveByte(const SPI_HandleTypeDef* hspi) {
	return SPIx_WriteRead(hspi, 0xFF);
}

void SPIx_Release(const SPI_HandleTypeDef* hspi) {
	SPIx_WriteRead(hspi, 0xFF);
}

uint8_t SD_cmd(const SPI_HandleTypeDef* hspi, uint8_t cmd, uint32_t arg) {
	uint8_t n, res;
	if (cmd & 0x80) {
		cmd &= 0x7F;
		res = SD_cmd(hspi, CMD55, 0);
		if (res > 1) return res;
	}
	SS_SD_Deselect();
	SPIx_Release(hspi);
	SS_SD_Select();
	SPIx_Release(hspi);
	SPIx_SendByte(hspi, cmd);
	SPIx_SendByte(hspi, (uint8_t)(arg >> 24));
	SPIx_SendByte(hspi, (uint8_t)(arg >> 16));
	SPIx_SendByte(hspi, (uint8_t)(arg >> 8));
	SPIx_SendByte(hspi, (uint8_t)arg);
	n = 0x01;
	if (cmd == CMD0) {
		n = 0x95;
	}
	else if (cmd == CMD8) {
		n = 0x87;
	}
	SPIx_SendByte(hspi, n);
	n = 10;
	do {
		res = SPIx_ReceiveByte(hspi);
	} while ((res & 0x80) && --n);
	return res;
}

uint8_t SD_Read_Block(const SPI_HandleTypeDef* hspi, uint8_t* buff, uint32_t lba) {
	uint8_t result;
	uint16_t cnt;

	result = SD_cmd(hspi, CMD17, lba);
	if (result != 0x00) {
		return 5;
	}
	SPIx_Release(hspi);
	cnt = 0;
	do {
		result = SPIx_ReceiveByte(hspi);
		cnt++;
	} while ((result != 0xFE) && (cnt < 0xFFFF));
	if (cnt >= 0xFFFF) {
		return 5;
	}
	for (cnt = 0; cnt < 512; cnt++) {
		buff[cnt] = SPIx_ReceiveByte(hspi);
	}
	SPIx_Release(hspi);
	SPIx_Release(hspi);
	return 0;
}

uint8_t SD_Write_Block(const SPI_HandleTypeDef* hspi, uint8_t* buff, uint32_t lba) {
	uint8_t result;
	uint16_t cnt;

	result = SD_cmd(hspi, CMD24, lba);
	if (result != 0x00) {
		return 6;
	}
	SPIx_Release(hspi);
	SPIx_SendByte(hspi, 0xFE);
	for (cnt = 0; cnt < 512; cnt++) {
		SPIx_SendByte(hspi, buff[cnt]);
	}
	SPIx_Release(hspi);
	SPIx_Release(hspi);
	result = SPIx_ReceiveByte(hspi);
	if ((result & 0x05) != 0x05) {
		return 6;
	}
	cnt = 0;
	do {
		result = SPIx_ReceiveByte(hspi);
		cnt++;
	} while ((result != 0xFF) && (cnt < 0xFFFF));
	if (cnt >= 0xFFFF) {
		return 6;
	}
	return 0;
}

uint8_t SPIx_Wait_Ready(const SPI_HandleTypeDef* hspi) {
	uint8_t res;
	uint16_t cnt;
	cnt = 0;
	do {
		res = SPIx_ReceiveByte(hspi);
		cnt++;
	} while ((res != 0xFF) && (cnt < 0xFFFF));
	if (cnt >= 0xFFFF) {
		return 1;
	}
	return res;
}
