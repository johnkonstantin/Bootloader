/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "fatfs.h"
#include "spi.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>
#include <stdio.h>
#include "sd.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
Error_Code errorCode = NO_ERROR;
const uint32_t errorLongDelay = 1000;
const uint32_t errorShortDelay = 250;
const uint32_t errorWaitDelay = 500;

extern char str1[60];
uint32_t byteswritten, bytesread;
uint8_t result;
char USER_Path[4];
SD_Info sdInfo;
FATFS SDFatFs;
FATFS* fs;
FIL MyFile;
uint8_t sect[512];
FRESULT res;
uint8_t wtext[] = "Hello from STM32!!!";
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);

/* USER CODE BEGIN PFP */
FRESULT ReadLongFile(void) {
	uint16_t i = 0, i1 = 0;
	uint32_t ind = 0;
	uint32_t f_size = MyFile.fsize;
	sprintf(str1, "fsize: %lurn", (unsigned long)f_size);
	HAL_UART_Transmit(&huart1, (uint8_t*)str1, strlen(str1), 0x1000);
	ind = 0;
	do {
		if (f_size < 512) {
			i1 = f_size;
		}
		else {
			i1 = 512;
		}
		f_size -= i1;
		f_lseek(&MyFile, ind);
		f_read(&MyFile, sect, i1, (UINT*)&bytesread);
		for (i = 0; i < bytesread; i++) {
			HAL_UART_Transmit(&huart1, sect + i, 1, 0x1000);
		}
		ind += i1;
	} while (f_size > 0);
	HAL_UART_Transmit(&huart1, (uint8_t*)"\r\n", 2, 0x1000);
	return FR_OK;
}
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void) {
	/* USER CODE BEGIN 1 */

	/* USER CODE END 1 */

	/* MCU Configuration--------------------------------------------------------*/

	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();

	/* USER CODE BEGIN Init */

	/* USER CODE END Init */

	/* Configure the system clock */
	SystemClock_Config();

	/* USER CODE BEGIN SysInit */

	/* USER CODE END SysInit */

	/* Initialize all configured peripherals */
	MX_GPIO_Init();
	MX_USART1_UART_Init();
	MX_SPI1_Init();
	MX_FATFS_Init();
	/* USER CODE BEGIN 2 */
	HAL_GPIO_WritePin(ERROR_LED_GPIO_Port, ERROR_LED_Pin, GPIO_PIN_SET);

	if (f_mount(&SDFatFs, (TCHAR const*)USER_Path, 0) != FR_OK) {
		errorCode = SD_MOUNT_ERROR;
		Error_Handler();
	}
	else {
		if (f_open(&MyFile, "test.txt", FA_READ) != FR_OK) {
			errorCode = SD_OPEN_FILE_ERROR;
			Error_Handler();
		}
		else {
			ReadLongFile();
			f_close(&MyFile);
		}
	}

	if (f_mount(&SDFatFs, (TCHAR const*)USER_Path, 0) != FR_OK) {
		errorCode = SD_MOUNT_ERROR;
		Error_Handler();
	}
	else {
		if (f_open(&MyFile, "mywrite.txt", FA_CREATE_ALWAYS | FA_WRITE) != FR_OK) {
			errorCode = SD_OPEN_FILE_ERROR;
			Error_Handler();
		}
		else {
			res = f_write(&MyFile, wtext, sizeof(wtext), (void*)&byteswritten);
			if ((byteswritten == 0) || (res != FR_OK)) {
				errorCode = SD_WRITE_FILE_ERROR;
				Error_Handler();
			}
			f_close(&MyFile);
		}
	}
	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1) {
		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */
	}
	/* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void) {
	RCC_OscInitTypeDef RCC_OscInitStruct = {0};
	RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

	/** Initializes the RCC Oscillators according to the specified parameters
	* in the RCC_OscInitTypeDef structure.
	*/
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL4;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		Error_Handler();
	}

	/** Initializes the CPU, AHB and APB buses clocks
	*/
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK) {
		Error_Handler();
	}

	/** Enables the Clock Security System
	*/
	HAL_RCC_EnableCSS();
}

/* USER CODE BEGIN 4 */
static void RCC_Delay(uint32_t mdelay) {
	__IO uint32_t Delay = mdelay * (SystemCoreClock / 8U / 1000U);
	do {
		__NOP();
	} while (Delay--);
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void) {
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	uint32_t longCount = 0;
	uint32_t shortCount = 0;
	HAL_GPIO_WritePin(ERROR_LED_GPIO_Port, ERROR_LED_Pin, GPIO_PIN_SET);
	RCC_Delay(errorWaitDelay);
	switch (errorCode) {
		case SPI_ERROR:
			longCount = 1;
			shortCount = 1;
			break;
		case SD_MOUNT_ERROR:
			longCount = 2;
			shortCount = 1;
			break;
		case SD_OPEN_FILE_ERROR:
			longCount = 1;
			shortCount = 2;
			break;
		case SD_WRITE_FILE_ERROR:
			longCount = 2;
			shortCount = 2;
			break;
		default:
			HAL_GPIO_WritePin(ERROR_LED_GPIO_Port, ERROR_LED_Pin, GPIO_PIN_RESET);
			while (1);
	}
	uint32_t i;
	while (1) {
		for (i = 0; i < longCount; ++i) {
			HAL_GPIO_WritePin(ERROR_LED_GPIO_Port, ERROR_LED_Pin, GPIO_PIN_RESET);
			RCC_Delay(errorLongDelay);
			HAL_GPIO_WritePin(ERROR_LED_GPIO_Port, ERROR_LED_Pin, GPIO_PIN_SET);
			RCC_Delay(errorWaitDelay);
		}
		for (i = 0; i < shortCount; ++i) {
			HAL_GPIO_WritePin(ERROR_LED_GPIO_Port, ERROR_LED_Pin, GPIO_PIN_RESET);
			RCC_Delay(errorShortDelay);
			HAL_GPIO_WritePin(ERROR_LED_GPIO_Port, ERROR_LED_Pin, GPIO_PIN_SET);
			RCC_Delay(errorWaitDelay);
		}
		RCC_Delay(errorWaitDelay * 3);
	}
	/* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
	 ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
