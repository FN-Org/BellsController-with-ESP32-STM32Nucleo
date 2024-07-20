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
#include "FLASH_SECTOR_F4.h"
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
#define NOTE_C4  262
#define NOTE_D4  294
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_G4  392


//Melody save addresses
#define MemoryStartAddress  0x08020000
#define MelodySize 2048
#define MelodyLineSize 28 //28 byte (28 indirizzi), 7 parole
#define NumberMelodiesMemoryAddress 0x08010000

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
RTC_HandleTypeDef hrtc;

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
char buf[BUFFER_SIZE];
char uart1_rx_buffer[257] = {'0'};
char uart1_rx_char;
int rx_index = 0;

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
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/*const char *jsonEx =
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
  MX_USART1_UART_Init();
  MX_RTC_Init();
  /* USER CODE BEGIN 2 */
  // Simulate receiving JSON message
   //process_json_events(jsonEx);
/*
   for (int i = 0; i < eventCount; i++)
   {
	   send_uart_message(events[i].melodyName);
	   send_uart_message("\n");
	   sprintf(buf, "%i\r\n", events[i].melodyNumber);
	   send_uart_message(buf);
	   send_uart_message(events[i].time);
	   send_uart_message("\n");
   }
   char* debugString;

   char *data = "hello FLASH from ControllerTech\
   			  This is a test to see how many words can we work with";

   uint32_t data2[] = {0x1,0x2,0x3,0x4,0x5,0x6,0x7,0x8,0x9};

   uint32_t Rx_Data[30];

   char string[100];
*/
   //int number = 0;

   //float val = 123.456;

   //float val2 = 0;

   //float val1;

   //Flash_Write_Data(0x08010002 , (uint32_t *)data2, 9);
   //Flash_Read_Data(0x08010002 , Rx_Data, 10);

/*
     int numofwords = (strlen(data)/4)+((strlen(data)%4)!=0);
     Flash_Write_Data(0x08010042 , (uint32_t *)data, numofwords);
     Flash_Read_Data(0x08010042 , Rx_Data, numofwords);
     Convert_To_Str(Rx_Data, string);

*/

     //Flash_Write_NUM(MemoryStartAddress, number);
     //val2 = Flash_Read_NUM(MemoryStartAddress);

     //Flash_Write_NUM(0x08010100, val);
     //val1 = Flash_Read_NUM(0x08010100);
/*
	char data[3] = "0\0";

	Flash_Write_Data(MemoryStartAddress,(uint32_t *) data,1);
*/
  //EraseFlashSector(MemoryStartAddress);
  //EraseFlashSector(NumberMelodiesMemoryAddress);

  readAndRing(1);
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
	  	        		parseMelodies();
	  	        		send_uart_message("Returned to main");
	  	        	}
	  	        	else if (strcmp((char *)uart1_rx_buffer, "-T-\r") == 0){
	  	        		memset(uart1_rx_buffer, 0, sizeof(uart1_rx_buffer));
	  	        		rx_index = 0;
	  	        		send_uart_message("Starting the time parsing");
	  	        		//parseTime();
	  	        	}
	  	        	else if (strcmp((char *)uart1_rx_buffer, "-S-\r") == 0){
	  	        		memset(uart1_rx_buffer, 0, sizeof(uart1_rx_buffer));
	  	        		rx_index = 0;
	  	        		send_uart_message("Starting the system info parsing");
	  	        		//parseSystem();
	  	        	}
	  	        	else {
	  	        		//send_uart_message("What else?");
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
  sTime.Hours = 0x0;
  sTime.Minutes = 0x0;
  sTime.Seconds = 0x0;
  sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  sTime.StoreOperation = RTC_STOREOPERATION_RESET;
  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }
  sDate.WeekDay = RTC_WEEKDAY_MONDAY;
  sDate.Month = RTC_MONTH_JANUARY;
  sDate.Date = 0x1;
  sDate.Year = 0x0;

  if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RTC_Init 2 */

  /* USER CODE END RTC_Init 2 */

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
			if (cJSON_IsString(melodyNumberValue) && melodyNumberValue->valuestring != NULL)
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


void parseEvents(){
	char *jsonEvents = NULL;
	int rx_index = 0;
	bool received = false;
	while(!received){
		if (HAL_UART_Receive(&huart1, &uart1_rx_char, 1, 100) == HAL_OK){
			uart1_rx_buffer[rx_index++] = uart1_rx_char;
			if (uart1_rx_char == '\n' || rx_index >= sizeof(uart1_rx_buffer)){
				uart1_rx_buffer[rx_index - 1] = '\0';
				if (strcmp((char *)uart1_rx_buffer, "---\r") == 0) {
					received = true;
					memset(uart1_rx_buffer, 0, sizeof(uart1_rx_buffer));
					rx_index = 0;
					break;
				}
				else {
					jsonEvents = strncat(jsonEvents,uart1_rx_buffer,sizeof(uart1_rx_buffer)-1);
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
	uint32_t melodyStartingAddress = MemoryStartAddress + (melodyNum-1)* MelodySize;
	uint32_t ReadBuffer[MelodyLineSize/4];
	int line = 0;
	int note = 0;
	double duration = 0.0;
	uint32_t melodyLineAddress = melodyStartingAddress + line * MelodyLineSize;

	char ReadedString[MelodyLineSize];

	Flash_Read_Data(melodyStartingAddress,ReadBuffer,(MelodyLineSize-1)/4);

	Convert_To_Str(ReadBuffer,ReadedString);

	send_uart_message("Starting playing:");
	send_uart_message(ReadedString);

	//show on display the title

	if (ReadedString[0] != 'ÿ'){
		line++;
		while((unsigned char)ReadedString[0] != 0xFF){
			melodyLineAddress = melodyStartingAddress + line * MelodyLineSize;
			Flash_Read_Data(melodyLineAddress,ReadBuffer,MelodyLineSize/4);
			Convert_To_Str(ReadBuffer,ReadedString);

			note = atoi(ReadedString);
			duration = atof(ReadedString + 2);

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

	// Introduce un ritardo basato sulla durata in secondi convertita in millisecondi
	HAL_Delay((uint32_t)(duration * 1000));
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
