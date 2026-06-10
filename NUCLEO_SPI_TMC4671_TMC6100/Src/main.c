/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "string.h"
#include <stdio.h>

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#define TMC_CS_GPIO_Port GPIOC
#define TMC_CS_Pin GPIO_PIN_9

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

SPI_HandleTypeDef hspi3;

/* USER CODE BEGIN PV */


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_SPI3_Init(void);
/* USER CODE BEGIN PFP */






void TMC6100_Write(uint8_t reg, uint32_t value)
{
    uint8_t tx[5];
    uint8_t rx[5] = {0};

    tx[0] = 0x80 | reg;                    // Write command (MSB=1)
    tx[1] = (value >> 24) & 0xFF;
    tx[2] = (value >> 16) & 0xFF;
    tx[3] = (value >> 8)  & 0xFF;
    tx[4] = value & 0xFF;

    HAL_GPIO_WritePin(TMC_CS_GPIO_Port, TMC_CS_Pin, GPIO_PIN_RESET);
    HAL_SPI_TransmitReceive(&hspi3, tx, rx, 5, 100);
    HAL_GPIO_WritePin(TMC_CS_GPIO_Port, TMC_CS_Pin, GPIO_PIN_SET);

    printf("Wrote 0x%08lX to register 0x%02X\r\n", value, reg);
}






uint32_t TMC6100_Read(uint8_t reg)
{
    uint8_t tx[5] = {0};
    uint8_t rx[5] = {0};

    tx[0] = reg;        // Read command (MSB=0)

    HAL_GPIO_WritePin(TMC_CS_GPIO_Port, TMC_CS_Pin, GPIO_PIN_RESET);
    HAL_SPI_TransmitReceive(&hspi3, tx, rx, 5, 100);
    HAL_GPIO_WritePin(TMC_CS_GPIO_Port, TMC_CS_Pin, GPIO_PIN_SET);

    uint32_t value = ((uint32_t)rx[1] << 24) | ((uint32_t)rx[2] << 16) |
                     ((uint32_t)rx[3] << 8)  | rx[4];

    printf("Read  reg 0x%02X = 0x%08lX\r\n", reg, value);
    return value;
}





void TMC6100_Init(void)
{
    printf("\r\n=== TMC6100 Safe Configuration ===\r\n");

    // Clear any pending status flags
    TMC6100_Write(0x01, 0xFFFFFFFF);   // GSTAT






    // ====================== GCONF TEST ======================
    // setting bit to 1 so we can see if it realy changes the register values, otherwise other values looks like are default settings
    uint32_t gconf = TMC6100_Read(0x00);
    printf("GCONF before = 0x%08lX   | singleline = %lu\r\n",
           gconf, (gconf >> 1) & 1);

    // === TEST: Set singleline = 1 ===
    gconf |= (1UL << 1);                    // Set bit 1 to 1
    TMC6100_Write(0x00, gconf);

    // Read back to verify
    gconf = TMC6100_Read(0x00);
    printf("GCONF after  = 0x%08lX   | singleline = %lu  (should be 1)\r\n",
           gconf, (gconf >> 1) & 1);




    // ====================== GCONF ======================
    // === 1. GCONF: Set only singleline bit to 0 ===

    //uint32_t gconf = TMC6100_Read(0x00);           // Read current value
    gconf = TMC6100_Read(0x00);           // Read current value
    printf("GCONF before modify = 0x%08lX   | singleline bit = %lu\r\n",
           gconf, (gconf >> 1) & 1);



    gconf &= ~(1UL << 1);                          // Clear bit 1 (singleline = 0)
    // 1UL = the number 1 as an unsigned long (32-bit). << 1 = shift left by 1 bit.
    // 1UL << 1 = 0b00000010 (binary) = 2 in decimal. This creates a mask with only bit 1 set.
    // ~ = bitwise NOT → inverts all bits.
    // So ~(1UL << 1) = 0b...11111101 (all bits 1 except bit 1 which is 0).
    // Result: This clears (sets to 0) only bit 1 of the register (gconf changed 1 bit), while leaving all other bits unchanged.
    // 1UL << 1		Mask for bit 1
    // ~(1UL << 1)	Clear only bit 1


    // gconf |= (1UL << x);                        // You can set other bits here if needed

    TMC6100_Write(0x00, gconf);

    // Verify after write
    gconf = TMC6100_Read(0x00);
    printf("GCONF after  modify = 0x%08lX   | singleline bit = %lu  (should be 0)\r\n",
           gconf, (gconf >> 1) & 1);




    // ====================== DRV_CONF ======================
    // === 2. DRV_CONF: Set only DRVSTRENGTH to 00 (weak) ===
    uint32_t drvconf = TMC6100_Read(0x0A);         // Read current value

    printf("DRV_CONF before = 0x%08lX   | DRVSTRENGTH = %lu (bits 19:18)\r\n",
               drvconf, (drvconf >> 18) & 3);

    // Force DRVSTRENGTH = 00 (weak)
    drvconf &= ~(3UL << 18);                       // Clear bits 19:18
    // You want to control 2 bits: bits 19 and 18 (DRVSTRENGTH).
    // 3UL in binary is 0b11 (two bits set to 1).
    // 3UL << 18 shifts these two 1s to the correct position by 18 positions:
    // 3UL << 18 = 0b0000 0000 0 -> OO <- 0 1100 0000 0000 0000 0000
    // So 3UL << 18 creates a mask for 2 bits.
    // ~ inverts it → all bits 1 except bits 19 and 18 which become 0.
    // &= applies this mask → clears both bit 19 and bit 18.
    // This is the standard way to clear a multi-bit field.
    // 3UL << 18	Mask for bits 19+18
    // ~(3UL << 18)	Clear bits 19 and 18

    // drvconf |= (0UL << 18);                     // 00 = weak (already cleared)
    TMC6100_Write(0x0A, drvconf);

    // Verify after write
    drvconf = TMC6100_Read(0x0A);
    printf("DRV_CONF after  = 0x%08lX   | DRVSTRENGTH = %lu (should be 0)\r\n",
           drvconf, (drvconf >> 18) & 3);

    printf("=== TMC6100 Configuration Finished ===\r\n\r\n");
}



/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

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
  MX_SPI3_Init();
  /* USER CODE BEGIN 2 */


  printf("TMC6100 Test Start\r\n");
  TMC6100_Init();


  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

      // Optional: periodic status check
      //HAL_Delay(1000);
      //TMC6100_Read(0x01);  // Check GSTAT

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 72;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief SPI3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI3_Init(void)
{

  /* USER CODE BEGIN SPI3_Init 0 */

  /* USER CODE END SPI3_Init 0 */

  /* USER CODE BEGIN SPI3_Init 1 */

  /* USER CODE END SPI3_Init 1 */
  /* SPI3 parameter configuration*/
  hspi3.Instance = SPI3;
  hspi3.Init.Mode = SPI_MODE_MASTER;
  hspi3.Init.Direction = SPI_DIRECTION_2LINES;
  hspi3.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi3.Init.CLKPolarity = SPI_POLARITY_HIGH;
  hspi3.Init.CLKPhase = SPI_PHASE_2EDGE;
  hspi3.Init.NSS = SPI_NSS_SOFT;
  hspi3.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_32;
  hspi3.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi3.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi3.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi3.Init.CRCPolynomial = 7;
  hspi3.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  hspi3.Init.NSSPMode = SPI_NSS_PULSE_DISABLE;
  if (HAL_SPI_Init(&hspi3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI3_Init 2 */

  /* USER CODE END SPI3_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */
  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, LD1_Pin|LD3_Pin|LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8|GPIO_PIN_9, GPIO_PIN_SET);

  /*Configure GPIO pins : LD1_Pin LD3_Pin LD2_Pin */
  GPIO_InitStruct.Pin = LD1_Pin|LD3_Pin|LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : PC8 PC9 */
  GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */
  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
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
