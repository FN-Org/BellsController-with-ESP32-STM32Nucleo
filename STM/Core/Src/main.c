/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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
#include <stdio.h>
#include "string.h"
#include "cJSON.h"
#include <stdbool.h>
#include <stdlib.h>
#include "liquidcrystal_i2c.h"
#include "FLASH_SECTOR_F4.h"
#include <math.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define BUFFER_SIZE 1024
#define MAX_EVENTS 100
#define MAX_STRING_SIZE 100

// Defining note frequency //

// Concert in C (DO)
#define NOTE_C4  261.6
#define NOTE_D4  293.6
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_G4  392

// Melody save addresses
#define MemoryStartAddress  0x08020000
#define MelodySize 2048
#define MelodyLineSize 28 //28 byte (28 indirizzi), 7 parole
#define NumberMelodiesMemoryAddress 0x08010000

// Time
#define StartYear 2000
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

RTC_HandleTypeDef hrtc;

TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim8;
TIM_HandleTypeDef htim12;

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
char buf[BUFFER_SIZE];
char uart1_rx_buffer[257] = {'0'};
char uart1_rx_char;
int rx_index = 0;
RTC_TimeTypeDef CurrentTime = {0};
RTC_DateTypeDef CurrentDate = {0};

typedef struct {
    char melodyName[MAX_STRING_SIZE];
    int melodyNumber;
    char time[MAX_STRING_SIZE];
} Event;

Event events[MAX_EVENTS];
int eventCount = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_RTC_Init(void);
static void MX_I2C1_Init(void);
static void MX_TIM1_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM3_Init(void);
static void MX_TIM8_Init(void);
static void MX_TIM12_Init(void);
/* USER CODE BEGIN PFP */

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
  HAL_InitTick(0);
  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  MX_USART1_UART_Init();
  MX_RTC_Init();
  MX_I2C1_Init();
  MX_TIM1_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_TIM8_Init();
  MX_TIM12_Init();
  /* USER CODE BEGIN 2 */

  // EraseFlashSector(MemoryStartAddress);
  // EraseFlashSector(NumberMelodiesMemoryAddress);

  HD44780_Init(2);
  HD44780_Clear();
  HD44780_SetCursor(0,0);
  HD44780_PrintStr("Sync and start");
  // readAndRing(2);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  // Send "Hello world" message

	  // Delay for 1000 milliseconds (1 second)
	  // HAL_Delay(1000);

	  if (HAL_UART_Receive(&huart1, &uart1_rx_char, 1, 100) == HAL_OK){
	  	         uart1_rx_buffer[rx_index++] = uart1_rx_char;
	  	         if (uart1_rx_char == '\n' || rx_index >= sizeof(uart1_rx_buffer))
	  	         {
	  	        	uart1_rx_buffer[rx_index - 1] = '\0';
	  	        	send_uart_message(uart1_rx_buffer);
	  	        	if (strcmp(uart1_rx_buffer, "-E-\r") == 0) {
	  	        		memset(uart1_rx_buffer, 0, sizeof(uart1_rx_buffer));
	  	        		send_uart_message("Starting the event parsing");
	  	        		rx_index = 0;
	  	        	    parseEvents();
	  	        	}
	  	        	else if (strcmp(uart1_rx_buffer, "-M-\r") == 0){
	  	        		memset(uart1_rx_buffer, 0, sizeof(uart1_rx_buffer));
	  	        		rx_index = 0;
	  	        		send_uart_message("Starting the melodies parsing");
	  	        		__disable_irq();
	  	        		parseMelodies();
	  	        		send_uart_message("Returned to main");
	  	        		__enable_irq();
	  	        	}
	  	        	else if (strcmp((char *)uart1_rx_buffer, "-T-\r") == 0){
	  	        		memset(uart1_rx_buffer, 0, sizeof(uart1_rx_buffer));
	  	        		rx_index = 0;
	  	        		send_uart_message("Starting the time parsing");
	  	        		parseTime();
	  	        	}
	  	        	else if (strcmp((char *)uart1_rx_buffer, "-S-\r") == 0){
	  	        		memset(uart1_rx_buffer, 0, sizeof(uart1_rx_buffer));
	  	        		rx_index = 0;
	  	        		send_uart_message("Starting the system info parsing");
	  	        		// parseSystem();
	  	        	}
	  	        	else {
	  	        		send_uart_message("What else?");
	  	        		memset(uart1_rx_buffer, 0, sizeof(uart1_rx_buffer));
	  	        		rx_index = 0;
	  	        	}
	  	         }
	  	     }
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
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSI;
  RCC_OscInitStruct.LSEState = RCC_LSE_OFF;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 180;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Activate the Over-Drive mode
  */
  if (HAL_PWREx_EnableOverDrive() != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief RTC Initialization Function
  * @param None
  * @retval None
  */
static void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */
  /* USER CODE END RTC_Init 0 */

  RTC_TimeTypeDef sTime = {0};
  RTC_DateTypeDef sDate = {0};
  RTC_AlarmTypeDef sAlarm = {0};

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */

  /** Initialize RTC Only
  */
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 127;
  hrtc.Init.SynchPrediv = 255;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN Check_RTC_BKUP */

  /* USER CODE END Check_RTC_BKUP */

  /** Initialize RTC and set the Time and Date
  */
  sTime.Hours = 0;
  sTime.Minutes = 0;
  sTime.Seconds = 0;
  sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  sTime.StoreOperation = RTC_STOREOPERATION_RESET;
  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK)
  {
    Error_Handler();
  }
  sDate.WeekDay = RTC_WEEKDAY_MONDAY;
  sDate.Month = RTC_MONTH_JANUARY;
  sDate.Date = 1;
  sDate.Year = 1;

  if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN) != HAL_OK)
  {
    Error_Handler();
  }

  /** Enable the Alarm A
  */
  sAlarm.AlarmTime.Hours = 0;
  sAlarm.AlarmTime.Minutes = 0;
  sAlarm.AlarmTime.Seconds = 0;
  sAlarm.AlarmTime.SubSeconds = 0;
  sAlarm.AlarmTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  sAlarm.AlarmTime.StoreOperation = RTC_STOREOPERATION_RESET;
  sAlarm.AlarmMask = RTC_ALARMMASK_DATEWEEKDAY|RTC_ALARMMASK_HOURS
                              |RTC_ALARMMASK_MINUTES;
  sAlarm.AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK_ALL;
  sAlarm.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_DATE;
  sAlarm.AlarmDateWeekDay = 1;
  sAlarm.Alarm = RTC_ALARM_A;
  if (HAL_RTC_SetAlarm_IT(&hrtc, &sAlarm, RTC_FORMAT_BIN) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RTC_Init 2 */
  HAL_NVIC_SetPriority(RTC_Alarm_IRQn,1,0);
  /* USER CODE END RTC_Init 2 */

}

/**
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};
  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 180-1;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 3822-1;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 3822/2;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
  sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
  sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
  sBreakDeadTimeConfig.DeadTime = 0;
  sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
  sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
  sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
  if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */
  HAL_TIM_MspPostInit(&htim1);

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 90-1;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 3406-1;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 3406/2;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */
  HAL_TIM_MspPostInit(&htim2);

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 90-1;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 3030-1;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 3030/2;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */
  HAL_TIM_MspPostInit(&htim3);

}

/**
  * @brief TIM8 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM8_Init(void)
{

  /* USER CODE BEGIN TIM8_Init 0 */

  /* USER CODE END TIM8_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};
  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

  /* USER CODE BEGIN TIM8_Init 1 */

  /* USER CODE END TIM8_Init 1 */
  htim8.Instance = TIM8;
  htim8.Init.Prescaler = 180-1;
  htim8.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim8.Init.Period = 2865-1;
  htim8.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim8.Init.RepetitionCounter = 0;
  htim8.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim8) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim8, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim8) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim8, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 2865/2;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if (HAL_TIM_PWM_ConfigChannel(&htim8, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
  sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
  sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
  sBreakDeadTimeConfig.DeadTime = 0;
  sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
  sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
  sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
  if (HAL_TIMEx_ConfigBreakDeadTime(&htim8, &sBreakDeadTimeConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM8_Init 2 */

  /* USER CODE END TIM8_Init 2 */
  HAL_TIM_MspPostInit(&htim8);

}

/**
  * @brief TIM12 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM12_Init(void)
{

  /* USER CODE BEGIN TIM12_Init 0 */

  /* USER CODE END TIM12_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM12_Init 1 */

  /* USER CODE END TIM12_Init 1 */
  htim12.Instance = TIM12;
  htim12.Init.Prescaler = 90-1;
  htim12.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim12.Init.Period = 2551-1;
  htim12.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim12.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim12) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim12, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim12) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 2551/2;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim12, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM12_Init 2 */

  /* USER CODE END TIM12_Init 2 */
  HAL_TIM_MspPostInit(&htim12);

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 9600;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 9600;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

void send_uart_message(char *message) {
	sprintf((char *)buf,"%s\r\n",message);
    HAL_UART_Transmit(&huart2, (uint8_t *)buf, strlen(buf), HAL_MAX_DELAY);
}

/**
 * @brief Parse and save the event in the array
 * @param message, pointer to the string parsed as a json
 */
void process_json_events(const char *event) {
	const cJSON *documents = NULL;
	const cJSON *document = NULL;
	const cJSON *fields = NULL;
	const cJSON *field = NULL;
	eventCount = 0;

    cJSON *event_json = cJSON_Parse(event);
    if (event_json == NULL)
    {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL)
        {
            fprintf(stderr, "Error before: %s\n", error_ptr);
        }
        goto end;
    }

    char *string = cJSON_Print(event_json);
    //send_uart_message(string);

    documents = cJSON_GetObjectItemCaseSensitive(event_json, "documents");
        cJSON_ArrayForEach(document, documents)
        {
            fields = cJSON_GetObjectItemCaseSensitive(document, "fields");

            char melodyNameString[MAX_STRING_SIZE] = {0};
			int melodyNumberInt = 0;
			char timeString[MAX_STRING_SIZE] = {0};

            // Melody name
            cJSON *melodyName = cJSON_GetObjectItemCaseSensitive(fields, "melodyName");
			cJSON *melodyNameValue = cJSON_GetObjectItemCaseSensitive(melodyName, "stringValue");
			if (cJSON_IsString(melodyNameValue) && melodyNameValue->valuestring != NULL)
			{
				strncpy(melodyNameString, melodyNameValue->valuestring, MAX_STRING_SIZE - 1);
			}
			else {
				send_uart_message("Error when parsing melodyName");
				goto end;
			}

            send_uart_message("Melody name: ");
            send_uart_message(melodyNameString);
            send_uart_message("\n");

			// Melody number
			cJSON *melodyNumber = cJSON_GetObjectItemCaseSensitive(fields, "melodyNumber");
			cJSON *melodyNumberValue = cJSON_GetObjectItemCaseSensitive(melodyNumber, "integerValue");


			if (cJSON_IsString(melodyNumberValue) && melodyNumberValue->valuestring != NULL)
			{
			    melodyNumberInt = atoi( melodyNumberValue->valuestring);
			}else {
				send_uart_message("Error when parsing melodyNumber");
				goto end;
			}

            send_uart_message("Melody number: ");
            sprintf(buf,"%d",melodyNumberInt);
            send_uart_message(buf);
            send_uart_message("\n");

			// Time
            cJSON *time = cJSON_GetObjectItemCaseSensitive(fields, "time");
            cJSON *timeValue = cJSON_GetObjectItemCaseSensitive(time, "timestampValue");

            if (cJSON_IsString(timeValue) && timeValue->valuestring != NULL)
            {
            	strncpy(timeString, timeValue->valuestring, MAX_STRING_SIZE - 1);
            }else {
				send_uart_message("Error when parsing time");
				goto end;
			}


            send_uart_message("Time: ");
            send_uart_message(timeString);
            send_uart_message("\n");

            // Create an event and add it to the array
			if (eventCount < MAX_EVENTS)
			{
				strncpy(events[eventCount].melodyName, melodyNameString, MAX_STRING_SIZE - 1);
				events[eventCount].melodyNumber = melodyNumberInt;
				strncpy(events[eventCount].time, timeString, MAX_STRING_SIZE - 1);
				eventCount++;
			}
        }

    end:
        cJSON_Delete(event_json);
        return;
}


void parseEvents(){
	bool received;
    size_t jsonBufferSize = 1024;
    char *jsonEvents = (char *)malloc(jsonBufferSize);
    if (jsonEvents == NULL) {
        // Handle memory allocation failure
        return;
    }
    jsonEvents[0] = '\0'; // Initialize the buffer with an empty string

    while(!received) {
        if (HAL_UART_Receive(&huart1, &uart1_rx_char, 1, 100) == HAL_OK) {
            uart1_rx_buffer[rx_index++] = uart1_rx_char;
            if (uart1_rx_char == '\n' || rx_index >= sizeof(uart1_rx_buffer)) {
                uart1_rx_buffer[rx_index - 1] = '\0';
                if (strcmp((char *)uart1_rx_buffer, "---\r") == 0) {
                    received = true;
                    memset(uart1_rx_buffer, 0, sizeof(uart1_rx_buffer));
                    //strcat(jsonEvents,"}");
                    send_uart_message("events parsing ended!");
                    rx_index = 0;
                    break;
                } else {
                    //uart1_rx_buffer[rx_index - 2] = '\0'; // Removing \r
                    size_t newLength = strlen(jsonEvents) + strlen((char *)uart1_rx_buffer) + 1;
                    if (newLength > jsonBufferSize) {
                        jsonBufferSize = newLength * 2; // Increase buffer size
                        char *newJsonEvents = (char *)realloc(jsonEvents, jsonBufferSize);
                        if (newJsonEvents == NULL) {
                            // Handle memory allocation failure
                            free(jsonEvents);
                            return;
                        }
                        jsonEvents = newJsonEvents;
                    }
                    strcat(jsonEvents, uart1_rx_buffer);
                    memset(uart1_rx_buffer, 0, sizeof(uart1_rx_buffer));
                    rx_index = 0;
                }
            }
        }
    }
	process_json_events(jsonEvents);
}

void parseMelodies(){

	rx_index = 0;
	uint32_t error = 0;
	float melodiesNum = 0;

	melodiesNum = Flash_Read_NUM(NumberMelodiesMemoryAddress);


	if (isnan(melodiesNum) || melodiesNum > 63 || melodiesNum < 0) {
		melodiesNum = 0;
	}


	// Calcola l'indirizzo di salvataggio della melodia
	uint32_t melodySavingAddress = MemoryStartAddress + (melodiesNum * MelodySize);

	melodiesNum++;

	error = Flash_Write_NUM(NumberMelodiesMemoryAddress,melodiesNum,true);

	uint32_t write_data[MelodyLineSize/4] = {0};
	uint32_t line = 0;
	bool received = false;

	// Loop per ricezione dati via UART
	while (!received) {
		if (HAL_UART_Receive(&huart1, &uart1_rx_char, 1, 100) == HAL_OK) {
			uart1_rx_buffer[rx_index++] = uart1_rx_char;
			if (uart1_rx_char == '\n' || rx_index >= sizeof(uart1_rx_buffer)) {
				uart1_rx_buffer[rx_index - 1] = '\0';
				if (strcmp((char *)uart1_rx_buffer, "---\r") == 0) {
					received = true;
					memset(uart1_rx_buffer, 0, sizeof(uart1_rx_buffer));
					send_uart_message("melody parsing ended!");
					rx_index = 0;
					line = 0;
				} else {
					// Conversione di uart1_rx_buffer a write_data
					for (int i = 0; i < (rx_index + sizeof(uint32_t) - 1) / sizeof(uint32_t); i++) {
						write_data[i] = ((uint32_t*)uart1_rx_buffer)[i];
					}
					// Scrittura dei dati nella memoria Flash
					int numofwords = (strlen(write_data)/4)+((strlen(write_data)%4)!=0);
					Flash_Write_Data((melodySavingAddress + (line * MelodyLineSize)), write_data,numofwords,false);

					// Pulizia del buffer e reset dell'indice
					memset(uart1_rx_buffer, 0, sizeof(uart1_rx_buffer));
					rx_index = 0;
					line++;
				}
			}
		}
	}
}


void readAndRing(int melodyNum){
	bool tooLong = false;
	uint32_t melodyStartingAddress = MemoryStartAddress + (melodyNum-1)* MelodySize;
	uint32_t ReadBuffer[MelodyLineSize/4];
	int line = 0;
	int note = 0;
	double duration = 0.0;
	uint32_t melodyLineAddress = melodyStartingAddress + line * MelodyLineSize;

	char ReadedString[MelodyLineSize];

	Flash_Read_Data(melodyStartingAddress,ReadBuffer,(MelodyLineSize-1)/4);

	Convert_To_Str(ReadBuffer,ReadedString);

	size_t length = strlen(ReadedString);
	for (size_t i = 0; i < length; ++i) {
	        if (ReadedString[i] == '\r') {
	            ReadedString[i] = '\0';
	            break;
	        }
	    }

	if ((unsigned char)ReadedString[0] != 0xFF){
		ReadedString[strlen(ReadedString)] = '\0';
		if (strlen(ReadedString)>16){
			tooLong = true;
		}
		HD44780_Clear();
		sprintf(buf,"Reproducing:");
		HD44780_SetCursor(0,0);
		HD44780_PrintStr(buf);
		HD44780_SetCursor(0,1);
		HD44780_PrintStr(ReadedString);

		line++;
		while(true){
			melodyLineAddress = melodyStartingAddress + line * MelodyLineSize;
			Flash_Read_Data(melodyLineAddress,ReadBuffer,MelodyLineSize/4);
			Convert_To_Str(ReadBuffer,ReadedString);
			if ((unsigned char)ReadedString[0] == 0xFF) break;
			note = atoi(ReadedString);
			duration = atof(ReadedString + 2);
			if (tooLong){
				HD44780_ScrollDisplayLeft();
			}
			play(note,duration);

			line++;
		}

		send_uart_message("Melody Playing finished");
	}
	else send_uart_message("Melody not found!");
}


void play(int note,double duration){
	char buf[20];
	sprintf(buf, "Playing: %d", note);
	send_uart_message(buf);

	sprintf(buf, "Duration: %.2f", duration); // Limita a 2 cifre decimali per la durata
	send_uart_message(buf);

	switch (note) {
	case 1 :
		Do();
		break;
	case 2 :
		Re();
		break;
	case 3 :
		Mi();
		break;
	case 4 :
		Fa();
		break;
	case 5 :
		Sol();
		break;
	default: break;
	}


	// Introduce un ritardo basato sulla durata in secondi convertita in millisecondi
	HAL_Delay((uint32_t)(duration * 1000));
}

void parseTime(){

	char jsonTime[128] = {0};


	while (true) {
			if (HAL_UART_Receive(&huart1, &uart1_rx_char, 1, 100) == HAL_OK) {
				uart1_rx_buffer[rx_index++] = uart1_rx_char;
				if (uart1_rx_char == '\n' || rx_index >= sizeof(uart1_rx_buffer)) {
					uart1_rx_buffer[rx_index - 1] = '\0';
					if (strncmp((char *)uart1_rx_buffer, "---",3) == 0) {
						memset(uart1_rx_buffer, 0, sizeof(uart1_rx_buffer));
						send_uart_message("Time parsing ended!");
						rx_index = 0;
						break;
					}
					else {
						uart1_rx_buffer[rx_index - 2] = '\0';
						strcpy(jsonTime,uart1_rx_buffer);
						memset(uart1_rx_buffer, 0, sizeof(uart1_rx_buffer));
						rx_index = 0;
						process_json_time(jsonTime);
					}
				}

			}
	}
}

void process_json_time(const char* time){

	cJSON *time_json = cJSON_Parse(time);
	if (time_json == NULL)
	{
		const char *error_ptr = cJSON_GetErrorPtr();
		if (error_ptr != NULL)
		{
			fprintf(stderr, "Error before: %s\n", error_ptr);
		}
		goto end;
	}

	cJSON *seconds = cJSON_GetObjectItemCaseSensitive(time_json, "s");
	double seconds_num = 0;
	cJSON *minutes = cJSON_GetObjectItemCaseSensitive(time_json, "mi");
	double minutes_num = 0;
	cJSON *hour = cJSON_GetObjectItemCaseSensitive(time_json, "h");
	double hour_num = 0;
	cJSON *monthDay = cJSON_GetObjectItemCaseSensitive(time_json, "md");
	double monthDay_num = 0;
	cJSON *yearMonth = cJSON_GetObjectItemCaseSensitive(time_json, "mo");
	double yearMonth_num = 0;
	cJSON *year = cJSON_GetObjectItemCaseSensitive(time_json, "y");
	double year_num = 0;
	cJSON *weekDay = cJSON_GetObjectItemCaseSensitive(time_json, "wd");
	double weekDay_num = 0;
	cJSON *legalHour = cJSON_GetObjectItemCaseSensitive(time_json, "l");
	double legalHour_num = 0;

	seconds_num  = cJSON_GetNumberValue(seconds);
	minutes_num  = cJSON_GetNumberValue(minutes);
	hour_num  = cJSON_GetNumberValue(hour);
	monthDay_num  = cJSON_GetNumberValue(monthDay);
	yearMonth_num  = cJSON_GetNumberValue(yearMonth);
	year_num  = cJSON_GetNumberValue(year);
	weekDay_num = cJSON_GetNumberValue(weekDay);
	legalHour_num  = cJSON_GetNumberValue(legalHour);


	if (seconds_num == NAN || minutes_num == NAN || hour_num == NAN || monthDay_num == NAN
			||yearMonth_num == NAN || year_num == NAN || legalHour_num == NAN || weekDay_num == NAN)
	{
		goto end;
	}
	else{
		if (weekDay = 0) weekDay = 7;
		CurrentTime.Hours = (uint8_t) hour_num;
		CurrentTime.Minutes = (uint8_t) minutes_num;
		CurrentTime.Seconds = (uint8_t) seconds_num;
		CurrentTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
		CurrentTime.StoreOperation = RTC_STOREOPERATION_RESET;
		if (HAL_RTC_SetTime(&hrtc, &CurrentTime, RTC_FORMAT_BIN) != HAL_OK)
		{
			send_uart_message("Error parsing time 1");
			Error_Handler();
		}
		CurrentDate.WeekDay = (uint8_t)weekDay_num;
		CurrentDate.Month = (uint8_t)yearMonth_num+1;
		CurrentDate.Date = (uint8_t)monthDay_num;
		CurrentDate.Year = (uint8_t)year_num%100;

		if (HAL_RTC_SetDate(&hrtc, &CurrentDate, RTC_FORMAT_BIN) != HAL_OK)
		{
			send_uart_message("Error parsing time 2");
			Error_Handler();
		}
	}


	end:
	      cJSON_Delete(time_json);
	      return;
}



// Funzione per convertire data e ora RTC in stringa ISO 8601
void RTC_to_ISO8601(RTC_DateTypeDef *date, RTC_TimeTypeDef *time, char *buffer) {
    sprintf(buffer, "%04d-%02d-%02dT%02d:%02d:%02dZ\0",
             date->Year + StartYear, // Anno completo
             date->Month,
             date->Date,
             time->Hours,
             time->Minutes,
             time->Seconds);
}

void HAL_RTC_AlarmAEventCallback(RTC_HandleTypeDef *hrtcAlarm){
	HAL_RTC_GetTime(&hrtc, &CurrentTime, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc, &CurrentDate, RTC_FORMAT_BIN);

	RTC_to_ISO8601(&CurrentDate,&CurrentTime,buf);
	if (eventCount > 0){
		if (strcmp(buf,events[0].time)== 0){
			readAndRing(events[0].melodyNumber);
		}
	}

	HD44780_Clear();

	sprintf(buf,"Date: %02d.%02d.%02d",CurrentDate.Date,CurrentDate.Month,CurrentDate.Year);
	HD44780_SetCursor(0,0);
	HD44780_PrintStr(buf);

	sprintf(buf,"Time: %02d.%02d",CurrentTime.Hours+2,CurrentTime.Minutes,CurrentTime.Seconds);
	HD44780_SetCursor(0,1);
	HD44780_PrintStr(buf);

}

// Notes

void Do() {
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
    HAL_UART_Transmit(&huart2, (uint8_t *)"DO Started\n", strlen("DO Started\n"), HAL_MAX_DELAY);
    HAL_Delay(500);
    HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
    HAL_UART_Transmit(&huart2, (uint8_t *)"DO Stopped\n", strlen("DO Stopped\n"), HAL_MAX_DELAY);
}

void Re() {
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_3);
    HAL_UART_Transmit(&huart2, (uint8_t *)"RE Started\n", strlen("RE Started\n"), HAL_MAX_DELAY);
    HAL_Delay(500);
    HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_3);
    HAL_UART_Transmit(&huart2, (uint8_t *)"RE Stopped\n", strlen("RE Stopped\n"), HAL_MAX_DELAY);
}

void Mi() {
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
    HAL_UART_Transmit(&huart2, (uint8_t *)"MI Started\n", strlen("MI Started\n"), HAL_MAX_DELAY);
    HAL_Delay(500);
    HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_1);
    HAL_UART_Transmit(&huart2, (uint8_t *)"MI Stopped\n", strlen("M Stopped\n"), HAL_MAX_DELAY);
}

void Fa() {
    HAL_TIM_PWM_Start(&htim8, TIM_CHANNEL_1);
    HAL_UART_Transmit(&huart2, (uint8_t *)"RE Started\n", strlen("RE Started\n"), HAL_MAX_DELAY);
    HAL_Delay(500);
    HAL_TIM_PWM_Stop(&htim8, TIM_CHANNEL_1);
    HAL_UART_Transmit(&huart2, (uint8_t *)"RE Stopped\n", strlen("RE Stopped\n"), HAL_MAX_DELAY);
}

void Sol() {
    HAL_TIM_PWM_Start(&htim12, TIM_CHANNEL_1);
    HAL_UART_Transmit(&huart2, (uint8_t *)"MI Started\n", strlen("MI Started\n"), HAL_MAX_DELAY);
    HAL_Delay(500);
    HAL_TIM_PWM_Stop(&htim12, TIM_CHANNEL_1);
    HAL_UART_Transmit(&huart2, (uint8_t *)"MI Stopped\n", strlen("M Stopped\n"), HAL_MAX_DELAY);
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
