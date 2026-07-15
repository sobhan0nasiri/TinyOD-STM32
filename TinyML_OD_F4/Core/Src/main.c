/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#include "crc.h"
#include "spi.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "infer_engine.h"
#include "risk_calc.h"
#include "fsm_controller.h"
#include "sensor_sim.h"

#include <stdio.h>
#include <string.h>

#include "ili9341.h"
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

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void Update_GLCD_With_Frame(const uint8_t *frame_buffer);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
#define TX_BUFFER_SIZE 300
char tx_buffer[TX_BUFFER_SIZE];
volatile uint8_t tx_flag = 1;
uint32_t tx_flag_timeout = 0;

VehicleState_t my_vehicle_state = {0};
uint8_t probability = 0;
uint8_t live_camera_frame[9216];
extern char result_buffer[200];
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
  MX_USART2_UART_Init();
  MX_CRC_Init();
  MX_SPI1_Init();
  /* USER CODE BEGIN 2 */
	
  FSM_Init();
	
	ILI9341_Init();
  ILI9341_FillScreen(ILI9341_BLACK);
	
  if (InferEngine_Init()) 
  {
      char err_msg[] = "AI Engine Init Failed!\r\n";
      HAL_UART_Transmit(&huart2, (uint8_t*)err_msg, strlen(err_msg), HAL_MAX_DELAY);
      Error_Handler();
  }
  
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

  while (1)
  {
		
		uint8_t prob = SensorSim_CaptureDynamicFrame(live_camera_frame);
		probability = prob;

		FSM_Update(probability, &my_vehicle_state);

		Update_GLCD_With_Frame(live_camera_frame);
		
		SystemState_t currentState = FSM_GetState();
    if (currentState == STATE_NORMAL) 
    {
       ILI9341_WriteString(20, 20, "State= Safe      ", ILI9341_GREEN, ILI9341_BLACK);
    } 
    else if (currentState == STATE_WARNING) 
    {
       ILI9341_WriteString(20, 20, "State= Warning   ", ILI9341_YELLOW, ILI9341_BLACK);
    } 
    else 
    {
       ILI9341_WriteString(20, 20, "State= BRAKING!  ", ILI9341_RED, ILI9341_BLACK);
    }
		
		char lcd_buf[32];
      
      snprintf(lcd_buf, sizeof(lcd_buf), "Prob : %d%%     ", probability);
      ILI9341_WriteString(20, 35, lcd_buf, ILI9341_WHITE, ILI9341_BLACK);
      
      if (my_vehicle_state.ttc_ms > 0 && my_vehicle_state.ttc_ms < 9999) {
          snprintf(lcd_buf, sizeof(lcd_buf), "TTC  : %d ms   ", my_vehicle_state.ttc_ms);
          ILI9341_WriteString(20, 50, lcd_buf, ILI9341_CYAN, ILI9341_BLACK);
      } else {
          ILI9341_WriteString(20, 50, "TTC  : Safe/None  ", ILI9341_DARKGREY, ILI9341_BLACK);
      }
		
		if (tx_flag || (HAL_GetTick() - tx_flag_timeout > 1500)) 
    {
        tx_flag = 0;
				tx_flag_timeout = HAL_GetTick();
				int offset = 0;
        
				if (result_buffer[0] != '\0')
        {
            offset += snprintf(tx_buffer + offset,
                                (size_t)(TX_BUFFER_SIZE - offset),
                                 "%s", result_buffer);
            if (offset < 0 || offset >= TX_BUFFER_SIZE)
            {
                offset = TX_BUFFER_SIZE - 1;
            }

            result_buffer[0] = '\0';
         }
			
         offset += snprintf(tx_buffer + offset,
                             (size_t)(TX_BUFFER_SIZE - offset),
                             "State: %d\r\n",
                             FSM_GetState());
				 
        HAL_UART_Transmit_IT(&huart2, (uint8_t*)tx_buffer, strlen(tx_buffer));
    }
		
		HAL_Delay(250);
		
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
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

#define COLOR_BLACK  0x0000
#define COLOR_RED    0xF800

void Update_GLCD_With_Frame(const uint8_t *frame_buffer) 
{
    uint16_t offset_lcd_x = (240 - 96) / 2;
    uint16_t offset_lcd_y = (320 - 96) / 2;
    
    for(uint16_t y = 0; y < 96; y++) 
    {
        for(uint16_t x = 0; x < 96; x++) 
        {
            uint16_t array_index = (y * 96) + x;
            uint8_t pixel_val = frame_buffer[array_index];

            uint16_t r = (pixel_val >> 3) & 0x1F;
            uint16_t g = (pixel_val >> 2) & 0x3F;
            uint16_t b = (pixel_val >> 3) & 0x1F;
            
            uint16_t final_color = (r << 11) | (g << 5) | b;
            
            ILI9341_DrawPixel(x + offset_lcd_x, y + offset_lcd_y, final_color);
        }
    }
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == B1_Pin_Pin) 
    {
        SensorSim_NextScenario();
    }
}

void UART_Print(const char *msg) {
    HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2)
    {
			  tx_flag = 1;
    }
}
int fputc(int ch, FILE *f) {
    HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
    return ch;
}
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
