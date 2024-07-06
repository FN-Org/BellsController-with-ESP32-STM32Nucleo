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
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define BUFFER_SIZE 1024
#define MAX_EVENTS 100
#define MAX_STRING_SIZE 100
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
char rxBuffer[BUFFER_SIZE];
volatile int rxIndex = 0;

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
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
const char *jsonEx =
    "{\n"
    "  \"documents\": [\n"
    "    {\n"
    "      \"name\": \"projects/bellapp-b7a2b/databases/(default)/documents/systems/J28ld8tCvTi0QNdYyZKF/events/CcZx2lvtFXvvBCsLmit5\",\n"
    "      \"fields\": {\n"
    "        \"color\": {\n"
    "          \"integerValue\": \"2\"\n"
    "        },\n"
    "        \"id\": {\n"
    "          \"stringValue\": \"CcZx2lvtFXvvBCsLmit5\"\n"
    "        },\n"
    "        \"time\": {\n"
    "          \"timestampValue\": \"2024-08-01T13:03:00Z\"\n"
    "        },\n"
    "        \"melodyNumber\": {\n"
    "          \"integerValue\": \"1\"\n"
    "        },\n"
    "        \"melodyName\": {\n"
    "          \"stringValue\": \"Compleanno\"\n"
    "        }\n"
    "      },\n"
    "      \"createTime\": \"2024-07-06T16:03:15.493477Z\",\n"
    "      \"updateTime\": \"2024-07-06T16:03:15.493477Z\"\n"
    "    },\n"
    "    {\n"
    "      \"name\": \"projects/bellapp-b7a2b/databases/(default)/documents/systems/J28ld8tCvTi0QNdYyZKF/events/F9WztPqVo0CQqFF1Vlrk\",\n"
    "      \"fields\": {\n"
    "        \"melodyNumber\": {\n"
    "          \"integerValue\": \"3\"\n"
    "        },\n"
    "        \"melodyName\": {\n"
    "          \"stringValue\": \"Ciabatta\"\n"
    "        },\n"
    "        \"color\": {\n"
    "          \"integerValue\": \"1\"\n"
    "        },\n"
    "        \"id\": {\n"
    "          \"stringValue\": \"F9WztPqVo0CQqFF1Vlrk\"\n"
    "        },\n"
    "        \"time\": {\n"
    "          \"timestampValue\": \"2024-08-03T10:03:00Z\"\n"
    "        }\n"
    "      },\n"
    "      \"createTime\": \"2024-07-06T16:03:52.335451Z\",\n"
    "      \"updateTime\": \"2024-07-06T16:03:52.335451Z\"\n"
    "    },\n"
    "    {\n"
    "      \"name\": \"projects/bellapp-b7a2b/databases/(default)/documents/systems/J28ld8tCvTi0QNdYyZKF/events/rzesraYruWmiMLMu0JAu\",\n"
    "      \"fields\": {\n"
    "        \"time\": {\n"
    "          \"timestampValue\": \"2024-08-03T10:03:00Z\"\n"
    "        },\n"
    "        \"id\": {\n"
    "          \"stringValue\": \"rzesraYruWmiMLMu0JAu\"\n"
    "        },\n"
    "        \"melodyName\": {\n"
    "          \"stringValue\": \"Ciabatta\"\n"
    "        },\n"
    "        \"melodyNumber\": {\n"
    "          \"integerValue\": \"3\"\n"
    "        },\n"
    "        \"color\": {\n"
    "          \"integerValue\": \"2\"\n"
    "        }\n"
    "      },\n"
    "      \"createTime\": \"2024-07-06T16:04:06.580543Z\",\n"
    "      \"updateTime\": \"2024-07-06T16:04:06.580543Z\"\n"
    "    },\n"
    "    {\n"
    "      \"name\": \"projects/bellapp-b7a2b/databases/(default)/documents/systems/J28ld8tCvTi0QNdYyZKF/events/s7uMQqpkZi2fCahZesnR\",\n"
    "      \"fields\": {\n"
    "        \"time\": {\n"
    "          \"timestampValue\": \"2024-08-02T04:00:00Z\"\n"
    "        },\n"
    "        \"melodyName\": {\n"
    "          \"stringValue\": \"Pasquale\"\n"
    "        },\n"
    "        \"id\": {\n"
    "          \"stringValue\": \"s7uMQqpkZi2fCahZesnR\"\n"
    "        },\n"
    "        \"color\": {\n"
    "          \"integerValue\": \"3\"\n"
    "        },\n"
    "        \"melodyNumber\": {\n"
    "          \"integerValue\": \"2\"\n"
    "        }\n"
    "      },\n"
    "      \"createTime\": \"2024-07-06T16:03:38.019557Z\",\n"
    "      \"updateTime\": \"2024-07-06T16:03:38.019557Z\"\n"
    "    }\n"
    "  ]\n"
    "}";

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
  /* USER CODE BEGIN 2 */

  // Simulate receiving JSON message
   process_json_message(jsonEx);

      // Print out the events parsed
   	   send_uart_message("Events parsed:\n");
          for (int i = 0; i < eventCount; i++) {
        	  send_uart_message("Event %d:\n", i + 1);
        	  send_uart_message("  Melody Name: %s\n", events[i].melodyName);
        	  send_uart_message("  Melody Number: %d\n", events[i].melodyNumber);
        	  send_uart_message("  Time: %s\n", events[i].time);
          }

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  // Send "Hello world" message
	  // send_uart_message("Hello world\n");

	  // Delay for 1000 milliseconds (1 second)
	  // HAL_Delay(1000);
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

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

void send_uart_message(char *message) {
    HAL_UART_Transmit(&huart2, (uint8_t *)message, strlen(message), HAL_MAX_DELAY);
}

/**
 * @brief Parse and save the event in the array
 * @param message, pointer to the string parsed as a json
 */
void process_json_message(const char *message) {
    cJSON *json = cJSON_Parse(message);
    if (json == NULL) {
        // JSON parsing failed
        return;
    }

    // Get the "documents" array
    cJSON *documents = cJSON_GetObjectItem(json, "documents");
    if (documents == NULL || !cJSON_IsArray(documents)) {
        // No valid "documents" array found
        cJSON_Delete(json);
        return;
    }

    int arraySize = cJSON_GetArraySize(documents);
    for (int i = 0; i < arraySize; i++) {
        cJSON *eventObj = cJSON_GetArrayItem(documents, i);
        if (eventObj == NULL || !cJSON_IsObject(eventObj)) {
            continue; // Skip if not a valid object
        }

        // Extract fields from the event object
        cJSON *fields = cJSON_GetObjectItem(eventObj, "fields");
        if (fields == NULL || !cJSON_IsObject(fields)) {
            continue; // Skip if no valid "fields" object
        }

        cJSON *melodyNameObj = cJSON_GetObjectItem(fields, "melodyName");
        cJSON *melodyNumberObj = cJSON_GetObjectItem(fields, "melodyNumber");
        cJSON *timeObj = cJSON_GetObjectItem(fields, "time");

        if (cJSON_IsString(melodyNameObj) && cJSON_IsNumber(melodyNumberObj) && cJSON_IsString(timeObj)) {
            // Create a new event
            Event newEvent;
            strncpy(newEvent.melodyName, melodyNameObj->valuestring, MAX_STRING_SIZE);
            newEvent.melodyNumber = melodyNumberObj->valueint;
            strncpy(newEvent.time, timeObj->valuestring, MAX_STRING_SIZE);

            // Add the event to the array if there's space
            if (eventCount < MAX_EVENTS) {
                events[eventCount++] = newEvent;
            }
        }
    }

    // Free JSON object
    cJSON_Delete(json);
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
