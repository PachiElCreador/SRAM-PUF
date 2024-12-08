/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
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
#include <stdint.h>  // Used for fixed-width integer types
#include <stdio.h>   // Provides sprintf for formatting strings
#include <string.h>  // Provides strlen for calculating string length
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
UART_HandleTypeDef huart2;  // Handle for UART2 communication

/* USER CODE BEGIN PV */
/**
 * Variable declared in the `.noinit` section to avoid being initialized during system resets.
 * This ensures that the residual SRAM content is preserved and can be read as a PUF source.
 */
__attribute__((section(".noinit"))) uint8_t puf_data[16];
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);

/* USER CODE BEGIN PFP */
/**
 * Function to read the SRAM residual data and transmit it over UART.
 */
void read_sram_and_generate_key(void);
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
  // Initialization phase before HAL is fully initialized
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();  // Initialize the HAL library

  /* USER CODE BEGIN Init */
  // Additional initialization steps (none required here)
  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();  // Set up the clock to ensure proper operation

  /* USER CODE BEGIN SysInit */
  // Additional system-level initialization
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();  // Configure GPIO pins
  MX_USART2_UART_Init();  // Initialize UART2 for serial communication

  /* USER CODE BEGIN 2 */
  /**
   * Call the function to read SRAM and send the extracted data over UART.
   * This is the core functionality to demonstrate the PUF.
   */
  read_sram_and_generate_key();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    // Continuous execution loop (not required for this demo)
  }
  /* USER CODE END 3 */
}

/**
  * @brief Reads the data stored in the `.noinit` section and transmits it via UART.
  */
void read_sram_and_generate_key(void)
{
    char buffer[64];  // Buffer to store formatted output strings

    // Iterate over the `puf_data` array and transmit each byte
    for (int i = 0; i < 16; i++) {
        // Format the SRAM data into a readable string
        sprintf(buffer, "PUF[%02d]: %02X\r\n", i, puf_data[i]);
        // Transmit the formatted string over UART
        HAL_UART_Transmit(&huart2, (uint8_t *)buffer, strlen(buffer), HAL_MAX_DELAY);
    }
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  // Configure the High Speed Internal (HSI) oscillator and the PLL
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL16;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();  // Handle errors during clock configuration
  }

  // Configure the CPU, AHB, and APB buses' clocks
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
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{
  // Configure the UART2 instance for communication
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 9600;  // Set baud rate to 9600
  huart2.Init.WordLength = UART_WORDLENGTH_8B;  // Data bits: 8
  huart2.Init.StopBits = UART_STOPBITS_1;  // Stop bit: 1
  huart2.Init.Parity = UART_PARITY_NONE;  // No parity
  huart2.Init.Mode = UART_MODE_TX_RX;  // Enable TX and RX modes
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;  // No hardware flow control
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;  // 16x oversampling for higher accuracy
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();  // Handle UART initialization errors
  }
}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  // Enable GPIO ports' clocks
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  // Configure the built-in LED (LD2)
  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);
  GPIO_InitStruct.Pin = LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);

  // Configure the User Button (B1)
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  // Enable external interrupt for the User Button
  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
}

/**
  * @brief  Error handler in case of failure
  * @retval None
  */
void Error_Handler(void)
{
  __disable_irq();  // Disable interrupts to isolate the error state
  while (1)  // Infinite loop to signal an error state
  {
  }
}
