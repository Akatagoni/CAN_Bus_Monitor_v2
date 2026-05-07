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
#include "cmsis_os.h"
#include "fatfs.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "string.h"
#include "stdio.h"
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
CAN_HandleTypeDef hcan1;
CAN_HandleTypeDef hcan2;

I2C_HandleTypeDef hi2c1;

SPI_HandleTypeDef hspi1;

UART_HandleTypeDef huart2;

/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for SensorTask */
osThreadId_t SensorTaskHandle;
const osThreadAttr_t SensorTask_attributes = {
  .name = "SensorTask",
  .stack_size = 1024 * 4,
  .priority = (osPriority_t) osPriorityHigh,
};
/* Definitions for CANTxTask */
osThreadId_t CANTxTaskHandle;
const osThreadAttr_t CANTxTask_attributes = {
  .name = "CANTxTask",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityHigh1,
};
/* Definitions for CANRxTask */
osThreadId_t CANRxTaskHandle;
const osThreadAttr_t CANRxTask_attributes = {
  .name = "CANRxTask",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityHigh2,
};
/* Definitions for ErrorTask */
osThreadId_t ErrorTaskHandle;
const osThreadAttr_t ErrorTask_attributes = {
  .name = "ErrorTask",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityBelowNormal,
};
/* Definitions for LogTask */
osThreadId_t LogTaskHandle;
const osThreadAttr_t LogTask_attributes = {
  .name = "LogTask",
  .stack_size = 2048 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for UARTTask */
osThreadId_t UARTTaskHandle;
const osThreadAttr_t UARTTask_attributes = {
  .name = "UARTTask",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for SensorQueue */
osMessageQueueId_t SensorQueueHandle;
const osMessageQueueAttr_t SensorQueue_attributes = {
  .name = "SensorQueue"
};
/* Definitions for CANRxQueue */
osMessageQueueId_t CANRxQueueHandle;
const osMessageQueueAttr_t CANRxQueue_attributes = {
  .name = "CANRxQueue"
};
/* Definitions for ErrorQueue */
osMessageQueueId_t ErrorQueueHandle;
const osMessageQueueAttr_t ErrorQueue_attributes = {
  .name = "ErrorQueue"
};
/* USER CODE BEGIN PV */
uint32_t canError = 0;
volatile float g_temp = 0;
volatile float g_pres = 0;
volatile float g_hum  = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_CAN1_Init(void);
static void MX_CAN2_Init(void);
static void MX_I2C1_Init(void);
static void MX_SPI1_Init(void);
static void MX_USART2_UART_Init(void);
void StartDefaultTask(void *argument);
void StartSensorTask(void *argument);
void StartCANTxTask(void *argument);
void StartCANRxTask(void *argument);
void StartErrorTask(void *argument);
void StartLogTask(void *argument);
void StartUARTTask(void *argument);

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

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_CAN1_Init();
  MX_CAN2_Init();
  MX_I2C1_Init();
  MX_SPI1_Init();
  MX_USART2_UART_Init();
  MX_FATFS_Init();
  /* USER CODE BEGIN 2 */
  char *boot = "System booting...\r\n";
  HAL_UART_Transmit(&huart2, (uint8_t*)boot, strlen(boot), 100);

  CAN_FilterTypeDef sFilterConfig;
  sFilterConfig.FilterBank = 14;
  sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
  sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
  sFilterConfig.FilterIdHigh = 0x0000;
  sFilterConfig.FilterIdLow = 0x0000;
  sFilterConfig.FilterMaskIdHigh = 0x0000;
  sFilterConfig.FilterMaskIdLow = 0x0000;
  sFilterConfig.FilterFIFOAssignment = CAN_RX_FIFO0;
  sFilterConfig.FilterActivation = ENABLE;
  sFilterConfig.SlaveStartFilterBank = 14;
  HAL_CAN_ConfigFilter(&hcan2, &sFilterConfig);

  HAL_StatusTypeDef sr = HAL_CAN_Start(&hcan2);
  HAL_CAN_ActivateNotification(&hcan2, CAN_IT_RX_FIFO0_MSG_PENDING);
  char m[60];
  snprintf(m, sizeof(m), "CAN2 Start:%d State:%d\r\n", sr, HAL_CAN_GetState(&hcan2));
  HAL_UART_Transmit(&huart2, (uint8_t*)m, strlen(m), 100);
  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize();

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* creation of SensorQueue */
  SensorQueueHandle = osMessageQueueNew (5, sizeof(uint32_t), &SensorQueue_attributes);

  /* creation of CANRxQueue */
  CANRxQueueHandle = osMessageQueueNew (10, sizeof(uint32_t), &CANRxQueue_attributes);

  /* creation of ErrorQueue */
  ErrorQueueHandle = osMessageQueueNew (5, sizeof(uint32_t), &ErrorQueue_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* creation of SensorTask */
  SensorTaskHandle = osThreadNew(StartSensorTask, NULL, &SensorTask_attributes);

  /* creation of CANTxTask */
  CANTxTaskHandle = osThreadNew(StartCANTxTask, NULL, &CANTxTask_attributes);

  /* creation of CANRxTask */
  CANRxTaskHandle = osThreadNew(StartCANRxTask, NULL, &CANRxTask_attributes);

  /* creation of ErrorTask */
  ErrorTaskHandle = osThreadNew(StartErrorTask, NULL, &ErrorTask_attributes);

  /* creation of LogTask */
  LogTaskHandle = osThreadNew(StartLogTask, NULL, &LogTask_attributes);

  /* creation of UARTTask */
  UARTTaskHandle = osThreadNew(StartUARTTask, NULL, &UARTTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
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
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
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
  * @brief CAN1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_CAN1_Init(void)
{

  /* USER CODE BEGIN CAN1_Init 0 */
	__HAL_RCC_CAN1_CLK_ENABLE();

  /* USER CODE END CAN1_Init 0 */

  /* USER CODE BEGIN CAN1_Init 1 */

  /* USER CODE END CAN1_Init 1 */
  hcan1.Instance = CAN1;
  hcan1.Init.Prescaler = 5;
  hcan1.Init.Mode = CAN_MODE_NORMAL;
  hcan1.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan1.Init.TimeSeg1 = CAN_BS1_15TQ;
  hcan1.Init.TimeSeg2 = CAN_BS2_2TQ;
  hcan1.Init.TimeTriggeredMode = DISABLE;
  hcan1.Init.AutoBusOff = DISABLE;
  hcan1.Init.AutoWakeUp = DISABLE;
  hcan1.Init.AutoRetransmission = DISABLE;
  hcan1.Init.ReceiveFifoLocked = DISABLE;
  hcan1.Init.TransmitFifoPriority = DISABLE;
  if (HAL_CAN_Init(&hcan1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CAN1_Init 2 */

  /* USER CODE END CAN1_Init 2 */

}

/**
  * @brief CAN2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_CAN2_Init(void)
{

  /* USER CODE BEGIN CAN2_Init 0 */
	__HAL_RCC_CAN1_CLK_ENABLE();
  /* USER CODE END CAN2_Init 0 */

  /* USER CODE BEGIN CAN2_Init 1 */

  /* USER CODE END CAN2_Init 1 */
  hcan2.Instance = CAN2;
  hcan2.Init.Prescaler = 6;
  hcan2.Init.Mode = CAN_MODE_LOOPBACK;
  hcan2.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan2.Init.TimeSeg1 = CAN_BS1_11TQ;
  hcan2.Init.TimeSeg2 = CAN_BS2_2TQ;
  hcan2.Init.TimeTriggeredMode = DISABLE;
  hcan2.Init.AutoBusOff = DISABLE;
  hcan2.Init.AutoWakeUp = DISABLE;
  hcan2.Init.AutoRetransmission = DISABLE;
  hcan2.Init.ReceiveFifoLocked = DISABLE;
  hcan2.Init.TransmitFifoPriority = DISABLE;
  if (HAL_CAN_Init(&hcan2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CAN2_Init 2 */
  CAN_FilterTypeDef sFilterConfig;
  sFilterConfig.FilterBank = 0;
  sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
  sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
  sFilterConfig.FilterIdHigh = 0x0000;
  sFilterConfig.FilterIdLow = 0x0000;
  sFilterConfig.FilterMaskIdHigh = 0x0000;
  sFilterConfig.FilterMaskIdLow = 0x0000;
  sFilterConfig.FilterFIFOAssignment = CAN_RX_FIFO0;
  sFilterConfig.FilterActivation = ENABLE;
  sFilterConfig.SlaveStartFilterBank = 14;
  HAL_CAN_ConfigFilter(&hcan1, &sFilterConfig);
  /* USER CODE END CAN2_Init 2 */

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
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI3;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

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
  huart2.Init.BaudRate = 115200;
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
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_4;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);


  /* USER CODE END MX_GPIO_Init_2 */

}

/* USER CODE BEGIN 4 */

uint16_t dig_T1;
int16_t  dig_T2, dig_T3;
uint16_t dig_P1;
int16_t  dig_P2, dig_P3, dig_P4, dig_P5;
int16_t  dig_P6, dig_P7, dig_P8, dig_P9;
uint8_t  dig_H1;
int16_t  dig_H2;
uint8_t  dig_H3;
int16_t  dig_H4, dig_H5;
int8_t   dig_H6;

void I2C_Scan(void)
{
    char msg[50];
    char *start = "Starting I2C Scan...\r\n";
    HAL_UART_Transmit(&huart2, (uint8_t*)start, strlen(start), 100);
    for(uint8_t addr = 1; addr < 128; addr++)
    {
        if(HAL_I2C_IsDeviceReady(&hi2c1, addr << 1, 1, 10) == HAL_OK)
        {
            snprintf(msg, sizeof(msg), "Device found at: 0x%02X\r\n", addr);
            HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), 100);
        }
    }
    char *done = "I2C Scan complete.\r\n";
    HAL_UART_Transmit(&huart2, (uint8_t*)done, strlen(done), 100);
}

void BME280_ReadCalibration(void)
{
    uint8_t calib[6];
    uint8_t reg = 0x88;
    HAL_StatusTypeDef status;
    status = HAL_I2C_Master_Transmit(&hi2c1, 0x76<<1, &reg, 1, 100);
    if(status != HAL_OK) {
        char *err = "Calibration Transmit FAILED!\r\n";
        HAL_UART_Transmit(&huart2, (uint8_t*)err, strlen(err), 100);
        return;
    }
    status = HAL_I2C_Master_Receive(&hi2c1, 0x76<<1, calib, 6, 100);
    if(status != HAL_OK) {
        char *err = "Calibration Receive FAILED!\r\n";
        HAL_UART_Transmit(&huart2, (uint8_t*)err, strlen(err), 100);
        return;
    }
    dig_T1 = (uint16_t)(calib[1]<<8|calib[0]);
    dig_T2 = (int16_t)(calib[3]<<8|calib[2]);
    dig_T3 = (int16_t)(calib[5]<<8|calib[4]);
    char msg[80];
    snprintf(msg, sizeof(msg), "T1:%u T2:%d T3:%d\r\n", dig_T1, dig_T2, dig_T3);
    HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), 100);
}

void BME280_ReadPressureCalibration(void)
{
    uint8_t calib[18];
    uint8_t reg = 0x8E;
    HAL_I2C_Master_Transmit(&hi2c1, 0x76<<1, &reg, 1, 100);
    HAL_I2C_Master_Receive(&hi2c1, 0x76<<1, calib, 18, 100);
    dig_P1 = (uint16_t)(calib[1]<<8|calib[0]);
    dig_P2 = (int16_t)(calib[3]<<8|calib[2]);
    dig_P3 = (int16_t)(calib[5]<<8|calib[4]);
    dig_P4 = (int16_t)(calib[7]<<8|calib[6]);
    dig_P5 = (int16_t)(calib[9]<<8|calib[8]);
    dig_P6 = (int16_t)(calib[11]<<8|calib[10]);
    dig_P7 = (int16_t)(calib[13]<<8|calib[12]);
    dig_P8 = (int16_t)(calib[15]<<8|calib[14]);
    dig_P9 = (int16_t)(calib[17]<<8|calib[16]);
}

void BME280_ReadHumidityCalibration(void)
{
    uint8_t calib1, calib2[7];
    uint8_t reg;
    reg = 0xA1;
    HAL_I2C_Master_Transmit(&hi2c1, 0x76<<1, &reg, 1, 100);
    HAL_I2C_Master_Receive(&hi2c1, 0x76<<1, &calib1, 1, 100);
    dig_H1 = calib1;
    reg = 0xE1;
    HAL_I2C_Master_Transmit(&hi2c1, 0x76<<1, &reg, 1, 100);
    HAL_I2C_Master_Receive(&hi2c1, 0x76<<1, calib2, 7, 100);
    dig_H2 = (int16_t)(calib2[1]<<8|calib2[0]);
    dig_H3 = calib2[2];
    dig_H4 = (int16_t)(calib2[3]<<4|(calib2[4]&0x0F));
    dig_H5 = (int16_t)(calib2[5]<<4|(calib2[4]>>4));
    dig_H6 = (int8_t)calib2[6];
}

void BME280_Init()
{
    uint8_t data[2];
    data[0] = 0xF2;
    data[1] = 0x01;
    HAL_I2C_Master_Transmit(&hi2c1, 0x76<<1, data, 2, 100);
    data[0] = 0xF4;
    data[1] = 0x27;
    HAL_I2C_Master_Transmit(&hi2c1, 0x76<<1, data, 2, 100);
}

int32_t BME280_ReadRawTemperature(void)
{
    uint8_t data[3];
    uint8_t reg = 0xFA;
    HAL_I2C_Master_Transmit(&hi2c1, 0x76<<1, &reg, 1, 100);
    HAL_I2C_Master_Receive(&hi2c1, 0x76<<1, data, 3, 100);
    return (int32_t)(data[0]<<12|data[1]<<4|data[2]>>4);
}

int32_t BME280_ReadRawPressure(void)
{
    uint8_t data[3];
    uint8_t reg = 0xF7;
    HAL_I2C_Master_Transmit(&hi2c1, 0x76<<1, &reg, 1, 100);
    HAL_I2C_Master_Receive(&hi2c1, 0x76<<1, data, 3, 100);
    return (int32_t)(data[0]<<12|data[1]<<4|data[2]>>4);
}

int32_t BME280_ReadRawHumidity(void)
{
    uint8_t data[2];
    uint8_t reg = 0xFD;
    HAL_I2C_Master_Transmit(&hi2c1, 0x76<<1, &reg, 1, 100);
    HAL_I2C_Master_Receive(&hi2c1, 0x76<<1, data, 2, 100);
    return (int32_t)(data[0]<<8|data[1]);
}

float BME280_GetTemperature(int32_t raw)
{
    int32_t var1, var2;
    var1 = ((((raw>>3)-((int32_t)dig_T1<<1)))*((int32_t)dig_T2))>>11;
    var2 = (((((raw>>4)-((int32_t)dig_T1))*((raw>>4)-((int32_t)dig_T1)))>>12)*((int32_t)dig_T3))>>14;
    int32_t t_fine = var1 + var2;
    return (float)((t_fine * 5 + 128) >> 8) / 100.0f;
}

float BME280_GetPressure(int32_t raw_temp, int32_t raw_pres)
{
    int32_t var1, var2;
    var1 = ((((raw_temp>>3)-((int32_t)dig_T1<<1)))*((int32_t)dig_T2))>>11;
    var2 = (((((raw_temp>>4)-((int32_t)dig_T1))*((raw_temp>>4)-((int32_t)dig_T1)))>>12)*((int32_t)dig_T3))>>14;
    int32_t t_fine = var1 + var2;
    int64_t p_var1, p_var2, p;
    p_var1 = ((int64_t)t_fine) - 128000;
    p_var2 = p_var1 * p_var1 * (int64_t)dig_P6;
    p_var2 = p_var2 + ((p_var1*(int64_t)dig_P5)<<17);
    p_var2 = p_var2 + (((int64_t)dig_P4)<<35);
    p_var1 = ((p_var1*p_var1*(int64_t)dig_P3)>>8)+((p_var1*(int64_t)dig_P2)<<12);
    p_var1 = (((((int64_t)1)<<47)+p_var1))*((int64_t)dig_P1)>>33;
    if(p_var1 == 0) return 0;
    p = 1048576 - raw_pres;
    p = (((p<<31)-p_var2)*3125)/p_var1;
    p_var1 = (((int64_t)dig_P9)*(p>>13)*(p>>13))>>25;
    p_var2 = (((int64_t)dig_P8)*p)>>19;
    p = ((p+p_var1+p_var2)>>8)+(((int64_t)dig_P7)<<4);
    return (float)p/256.0f/100.0f;
}

float BME280_GetHumidity(int32_t raw_temp, int32_t raw_hum)
{
    int32_t var1, var2;
    var1 = ((((raw_temp>>3)-((int32_t)dig_T1<<1)))*((int32_t)dig_T2))>>11;
    var2 = (((((raw_temp>>4)-((int32_t)dig_T1))*((raw_temp>>4)-((int32_t)dig_T1)))>>12)*((int32_t)dig_T3))>>14;
    int32_t t_fine = var1 + var2;
    int32_t h;
    h = t_fine - 76800;
    h = (((((raw_hum<<14)-(((int32_t)dig_H4)<<20)-(((int32_t)dig_H5)*h))+16384)>>15)*
        (((((((h*((int32_t)dig_H6))>>10)*(((h*((int32_t)dig_H3))>>11)+32768))>>10)+2097152)*
        ((int32_t)dig_H2)+8192)>>14));
    h = h-(((((h>>15)*(h>>15))>>7)*((int32_t)dig_H1))>>4);
    h = h<0?0:h;
    h = h>419430400?419430400:h;
    return (float)(h>>12)/1024.0f;
}

/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN 5 */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END 5 */
}

/* USER CODE BEGIN Header_StartSensorTask */
/**
* @brief Function implementing the SensorTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartSensorTask */
void StartSensorTask(void *argument)
{
  /* USER CODE BEGIN StartSensorTask */
  char *test = "SensorTask started!\r\n";
  HAL_UART_Transmit(&huart2, (uint8_t*)test, strlen(test), 100);
  osDelay(1000);

  //I2C_Scan();
  BME280_ReadCalibration();
  BME280_ReadPressureCalibration();
  BME280_ReadHumidityCalibration();
  BME280_Init();
  osDelay(300);

  for(;;)
  {

      int32_t raw_temp = BME280_ReadRawTemperature();
      int32_t raw_pres = BME280_ReadRawPressure();
      int32_t raw_hum  = BME280_ReadRawHumidity();


      float temp = BME280_GetTemperature(raw_temp);
      float pres = BME280_GetPressure(raw_temp, raw_pres);
      float hum  = BME280_GetHumidity(raw_temp, raw_hum);


      uint32_t temp_raw = *((uint32_t*)&temp);
      uint32_t pres_raw = *((uint32_t*)&pres);
      uint32_t hum_raw  = *((uint32_t*)&hum);

      g_temp = temp;
      g_pres = pres;
      g_hum  = hum;
      osMessageQueueReset(SensorQueueHandle);
      osMessageQueuePut(SensorQueueHandle, &temp_raw, 0, 0);
      osMessageQueuePut(SensorQueueHandle, &pres_raw, 0, 0);
      osMessageQueuePut(SensorQueueHandle, &hum_raw,  0, 0);

      char msg[80];
      snprintf(msg, sizeof(msg), "T:%.2fC P:%.2fhPa H:%.2f%%\r\n", temp, pres, hum);
      HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), 100);

      osDelay(1000);
  }
  /* USER CODE END StartSensorTask */
}

/* USER CODE BEGIN Header_StartCANTxTask */
/**
* @brief Function implementing the CANTxTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartCANTxTask */
void StartCANTxTask(void *argument)
{
  /* USER CODE BEGIN StartCANTxTask */
  char *test = "CANTxTask started!\r\n";
  HAL_UART_Transmit(&huart2, (uint8_t*)test, strlen(test), 100);
  osDelay(500);

  CAN_TxHeaderTypeDef TxHeader;
  uint8_t TxData[4];
  uint32_t TxMailbox;

  TxHeader.IDE = CAN_ID_STD;
  TxHeader.RTR = CAN_RTR_DATA;
  TxHeader.DLC = 4;
  TxHeader.TransmitGlobalTime = DISABLE;

  for(;;)
  {
      uint32_t temp_raw;
      if(osMessageQueueGet(SensorQueueHandle, &temp_raw, NULL, 100) == osOK)
      {
          TxHeader.StdId = 0x100;
          TxData[0] = (temp_raw >> 24) & 0xFF;
          TxData[1] = (temp_raw >> 16) & 0xFF;
          TxData[2] = (temp_raw >> 8)  & 0xFF;
          TxData[3] = (temp_raw)       & 0xFF;
          while(HAL_CAN_GetTxMailboxesFreeLevel(&hcan2) == 0) { osDelay(1); }
          HAL_CAN_AddTxMessage(&hcan2, &TxHeader, TxData, &TxMailbox);

          uint32_t pres_raw;
          if(osMessageQueueGet(SensorQueueHandle, &pres_raw, NULL, 100) == osOK)
          {
              TxHeader.StdId = 0x101;
              TxData[0] = (pres_raw >> 24) & 0xFF;
              TxData[1] = (pres_raw >> 16) & 0xFF;
              TxData[2] = (pres_raw >> 8)  & 0xFF;
              TxData[3] = (pres_raw)       & 0xFF;
              HAL_CAN_AddTxMessage(&hcan2, &TxHeader, TxData, &TxMailbox);
          }

          uint32_t hum_raw;
          if(osMessageQueueGet(SensorQueueHandle, &hum_raw, NULL, 100) == osOK)
          {
              TxHeader.StdId = 0x102;
              TxData[0] = (hum_raw >> 24) & 0xFF;
              TxData[1] = (hum_raw >> 16) & 0xFF;
              TxData[2] = (hum_raw >> 8)  & 0xFF;
              TxData[3] = (hum_raw)       & 0xFF;
              HAL_CAN_AddTxMessage(&hcan2, &TxHeader, TxData, &TxMailbox);
          }

          char *msg = "CAN TX OK!\r\n";
          HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), 100);
      }
      osDelay(200);
  }
  /* USER CODE END StartCANTxTask */
}

/* USER CODE BEGIN Header_StartCANRxTask */
/**
* @brief Function implementing the CANRxTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartCANRxTask */
void StartCANRxTask(void *argument)
{
  /* USER CODE BEGIN StartCANRxTask */
  char *test = "CANRxTask started!\r\n";
  HAL_UART_Transmit(&huart2, (uint8_t*)test, strlen(test), 100);

  for(;;)
  {
      while(HAL_CAN_GetRxFifoFillLevel(&hcan2, CAN_RX_FIFO0) > 0)
      {
          CAN_RxHeaderTypeDef RxHeader;
          uint8_t RxData[8];
          HAL_CAN_GetRxMessage(&hcan2, CAN_RX_FIFO0, &RxHeader, RxData);

          uint32_t raw = ((uint32_t)RxData[0]<<24)|
                         ((uint32_t)RxData[1]<<16)|
                         ((uint32_t)RxData[2]<<8) |
                         ((uint32_t)RxData[3]);
          float value = *((float*)&raw);

          char *rcvr = "Received_";
          HAL_UART_Transmit(&huart2, (uint8_t*)rcvr, strlen(rcvr), 100);

          char msg[60];
          if(RxHeader.StdId == 0x100)
              snprintf(msg, sizeof(msg), "Temp: %.2f C\r\n", value);
          else if(RxHeader.StdId == 0x101)
              snprintf(msg, sizeof(msg), "Pres: %.2f hPa\r\n", value);
          else if(RxHeader.StdId == 0x102)
              snprintf(msg, sizeof(msg), "Hum: %.2f %%\r\n", value);

          HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), 100);
      }
      osDelay(100);
  }
  /* USER CODE END StartCANRxTask */
}

/* USER CODE BEGIN Header_StartErrorTask */
/**
* @brief Function implementing the ErrorTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartErrorTask */
void StartErrorTask(void *argument)
{
  /* USER CODE BEGIN StartErrorTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartErrorTask */
}

/* USER CODE BEGIN Header_StartLogTask */
/**
* @brief Function implementing the LogTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartLogTask */
void StartLogTask(void *argument)
{
  /* USER CODE BEGIN StartLogTask */
  char *start = "LogTask started!\r\n";
  HAL_UART_Transmit(&huart2, (uint8_t*)start, strlen(start), 100);

  FATFS fs;
  FIL fil;
  FRESULT res;
  char line[100];

  osDelay(3000);

  res = f_mount(&fs, "", 0);
  char *mnt = res == FR_OK ? "SD Mounted!\r\n" : "SD Mount FAILED!\r\n";
  HAL_UART_Transmit(&huart2, (uint8_t*)mnt, strlen(mnt), 100);

  /* Write header on first boot */
  if(res == FR_OK)
  {
      FRESULT fres = f_open(&fil, "data.txt", FA_OPEN_ALWAYS | FA_WRITE);
      if(fres == FR_OK)
      {
          if(f_size(&fil) == 0)
          {
              f_puts("Temperature(C), Pressure(hPa), Humidity(%)\n", &fil);
          }
          f_close(&fil);
      }
  }

  for(;;)
  {
      if(res == FR_OK)
      {
          FRESULT fres = f_open(&fil, "data.txt", FA_OPEN_ALWAYS | FA_WRITE);
          if(fres == FR_OK)
          {
              f_lseek(&fil, f_size(&fil));
              snprintf(line, sizeof(line),
                  "Temperature:%.2fC, Pressure:%.2fhPa, Hum:%.2f%%\n",
                  g_temp, g_pres, g_hum);
              f_puts(line, &fil);
              f_close(&fil);

              char *log = "SD Write OK!\r\n";
              HAL_UART_Transmit(&huart2, (uint8_t*)log, strlen(log), 100);
          }
          else
          {
              char dbg[40];
              snprintf(dbg, sizeof(dbg), "Open failed: %d\r\n", fres);
              HAL_UART_Transmit(&huart2, (uint8_t*)dbg, strlen(dbg), 100);
          }
      }
      osDelay(5000);
  }
  /* USER CODE END StartLogTask */
}

/* USER CODE BEGIN Header_StartUARTTask */
/**
* @brief Function implementing the UARTTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartUARTTask */
void StartUARTTask(void *argument)
{
  /* USER CODE BEGIN StartUARTTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartUARTTask */
}

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
