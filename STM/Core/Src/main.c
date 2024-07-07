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
char buf[BUFFER_SIZE];

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

   for (int i = 0; i < eventCount; i++)
   {
	   send_uart_message(events[i].melodyName);
	   send_uart_message("\n");
	   sprintf(buf, "%d\r\n", events[i].melodyNumber);
	   send_uart_message(buf);
	   send_uart_message(events[i].time);
	   send_uart_message("\n");
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
void process_json_message(const char *event) {
	const cJSON *documents = NULL;
	const cJSON *document = NULL;
	const cJSON *fields = NULL;
	const cJSON *field = NULL;
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
    // send_uart_message(string);

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

            send_uart_message("Melody name: ");
            send_uart_message(cJSON_Print(melodyNameValue));
            send_uart_message("\n");

			// Melody number
			cJSON *melodyNumber = cJSON_GetObjectItemCaseSensitive(fields, "melodyNumber");
			cJSON *melodyNumberValue = cJSON_GetObjectItemCaseSensitive(melodyNumber, "integerValue");
			if (cJSON_IsNumber(melodyNumberValue))
			{
				melodyNumberInt = atoi(melodyNumberValue->valuestring);
			}

            send_uart_message("Melody number: ");
            send_uart_message(cJSON_Print(melodyNumberValue));
            send_uart_message("\n");

			// Time
            cJSON *time = cJSON_GetObjectItemCaseSensitive(fields, "time");
            cJSON *timeValue = cJSON_GetObjectItemCaseSensitive(time, "timestampValue");

            if (cJSON_IsString(timeValue) && timeValue->string != NULL)
            {
            	strncpy(timeString, timeValue->valuestring, MAX_STRING_SIZE - 1);
            }

            send_uart_message("Time: ");
            send_uart_message(cJSON_Print(timeValue));
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
