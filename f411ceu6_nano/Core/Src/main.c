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
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "rcc.h"
#include <stdio.h>
#include "log.h"
#include "shell.h"
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
I2C_HandleTypeDef hi2c1;

I2S_HandleTypeDef hi2s2;
DMA_HandleTypeDef hdma_spi2_tx;

UART_HandleTypeDef huart1;
DMA_HandleTypeDef hdma_usart1_rx;
DMA_HandleTypeDef hdma_usart1_tx;

/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
    .name = "defaultTask",
    .stack_size = 128 * 4,
    .priority = (osPriority_t)osPriorityNormal,
};
/* USER CODE BEGIN PV */
int _write(int file, char *ptr, int len)
{
  (void)file; // Suppress unused parameter warning
  HAL_UART_Transmit(&huart1, (uint8_t *)ptr, len, HAL_MAX_DELAY);
  return len;
}
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_I2C1_Init(void);
static void MX_I2S2_Init(void);
void StartDefaultTask(void *argument);

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
  MX_DMA_Init();
  MX_USART1_UART_Init();
  MX_I2C1_Init();
  MX_I2S2_Init();
  /* USER CODE BEGIN 2 */
  log_init();   // Initialize the logging system
  shell_init(); // Initialize the shell system
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

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

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
  RCC_OscInitStruct.PLL.PLLM = 12;
  RCC_OscInitStruct.PLL.PLLN = 96;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
   */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
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
 * @brief I2S2 Initialization Function
 * @param None
 * @retval None
 */
static void MX_I2S2_Init(void)
{

  /* USER CODE BEGIN I2S2_Init 0 */

  /* USER CODE END I2S2_Init 0 */

  /* USER CODE BEGIN I2S2_Init 1 */

  /* USER CODE END I2S2_Init 1 */
  hi2s2.Instance = SPI2;
  hi2s2.Init.Mode = I2S_MODE_MASTER_TX;
  hi2s2.Init.Standard = I2S_STANDARD_PHILIPS;
  hi2s2.Init.DataFormat = I2S_DATAFORMAT_16B;
  hi2s2.Init.MCLKOutput = I2S_MCLKOUTPUT_DISABLE;
  hi2s2.Init.AudioFreq = I2S_AUDIOFREQ_8K;
  hi2s2.Init.CPOL = I2S_CPOL_LOW;
  hi2s2.Init.ClockSource = I2S_CLOCK_PLL;
  hi2s2.Init.FullDuplexMode = I2S_FULLDUPLEXMODE_DISABLE;
  if (HAL_I2S_Init(&hi2s2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2S2_Init 2 */

  /* USER CODE END I2S2_Init 2 */
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
  huart1.Init.BaudRate = 115200;
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
 * Enable DMA controller clock
 */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA2_CLK_ENABLE();
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Stream4_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream4_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream4_IRQn);
  /* DMA2_Stream2_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream2_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream2_IRQn);
  /* DMA2_Stream7_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream7_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream7_IRQn);
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
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(led_GPIO_Port, led_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : led_Pin */
  GPIO_InitStruct.Pin = led_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(led_GPIO_Port, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

TIM_HandleTypeDef TIM3_Handler; // 定时器句柄

// 通用定时器3中断初始化
// arr：自动重装值。
// psc：时钟预分频数
// 定时器溢出时间计算方法:Tout=((arr+1)*(psc+1))/Ft us.
// Ft=定时器工作频率,单位:Mhz
// 这里使用的是定时器3!
//  void TIM3_Init(uint16_t arr,uint16_t psc)
//  {
//      TIM3_Handler.Instance=TIM3;                          //通用定时器3
//      TIM3_Handler.Init.Prescaler=psc;                     //分频系数
//      TIM3_Handler.Init.CounterMode=TIM_COUNTERMODE_UP;    //向上计数器
//      TIM3_Handler.Init.Period=arr;                        //自动装载值
//      TIM3_Handler.Init.ClockDivision=TIM_CLOCKDIVISION_DIV1;//时钟分频因子
//      HAL_TIM_Base_Init(&TIM3_Handler);

//     HAL_TIM_Base_Start_IT(&TIM3_Handler); //使能定时器3和定时器3更新中断：TIM_IT_UPDATE
// }

// 定时器底册驱动，开启时钟，设置中断优先级
// 此函数会被HAL_TIM_Base_Init()函数调用
void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *htim)
{
  if (htim->Instance == TIM3)
  {
    __HAL_RCC_TIM3_CLK_ENABLE();           // 使能TIM3时钟
    HAL_NVIC_SetPriority(TIM3_IRQn, 1, 3); // 设置中断优先级，抢占优先级1，子优先级3
    HAL_NVIC_EnableIRQ(TIM3_IRQn);         // 开启ITM3中断
  }
  if (htim->Instance == TIM1)
  {
    __HAL_RCC_TIM1_CLK_ENABLE();              // 使能TIM1时钟
    HAL_NVIC_SetPriority(TIM1_CC_IRQn, 1, 1); // 设置中断优先级，抢占优先级1，子优先级1
    HAL_NVIC_EnableIRQ(TIM1_CC_IRQn);         // 开启ITM1中断
  }
}

// 定时器3中断服务函数
//  void TIM1_IRQHandler(void)
//  {
//      HAL_TIM_IRQHandler(&TIM3_Handler);
//  }
#if 0
void uart_test1()
{

  char *msg = "Hello, World!                 \n";
  HAL_StatusTypeDef ret = HAL_UART_Transmit(&huart1, (uint8_t *)msg, strlen(msg), 1);
  HAL_StatusTypeDef temp_ret = ret;
  void *temp_pTxBuffPtr = huart1.pTxBuffPtr; // 0 函数返回后置空
  uint16_t TxXferSize = huart1.TxXferSize;   // 31 发送缓冲区长度
  uint16_t TxXferCount = huart1.TxXferCount; // 17 剩余未发送长度

  printf("UART transmit status: %d\n", temp_ret);
  printf("pTxBuffPtr: %p msg:%p\n", temp_pTxBuffPtr, msg); // Print the pointer to the Tx buffer
  printf("TxXferSize: %d\n", TxXferSize);                  // Print the TxXferSize
  printf("TxXferCount: %d\n", TxXferCount);                // Print the TxXferCount

  uint8_t buf[64];
  ret = HAL_UART_Receive(&huart1, (uint8_t *)buf, sizeof(buf) - 1, 1000);
  printf("UART receive status: %d\n", ret);
  printf("pRxBuffPtr: %p buf:%p\n", huart1.pRxBuffPtr, buf); // 0  函数返回后置空
  printf("RxXferSize: %d\n", huart1.RxXferSize);             // 63 接受缓冲区长度
  printf("RxXferCount: %d\n", huart1.RxXferCount);           // 62 剩余未接收长度
}

static uint8_t rx_buffer_it[256];
static uint8_t tx_buffer_it[256];
static volatile uint8_t tx_complete_flag = 0;
static volatile uint8_t rx_complete_flag = 0;
static volatile uint8_t tx_half_complete_flag = 0;
static volatile uint8_t rx_half_complete_flag = 0;
static volatile uint16_t received_length = 0;

/**
 * @brief UART发送完成回调函数
 */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
  if (huart->Instance == USART1)
  {
    tx_complete_flag = 1;
  }
}
void HAL_UART_TxHalfCpltCallback(UART_HandleTypeDef *huart)
{
  if (huart->Instance == USART1)
  {
    tx_half_complete_flag = 1;
  }
}

/**
 * @brief UART接收完成回调函数
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if (huart->Instance == USART1)
  {
    rx_complete_flag = 1;
    received_length = huart->RxXferSize;
  }
}

void HAL_UART_RxHalfCpltCallback(UART_HandleTypeDef *huart)
{
  if (huart->Instance == USART1)
  {
    rx_half_complete_flag = 1;
  }
}

/**
 * @brief UART错误回调函数
 */
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
  if (huart->Instance == USART1)
  {
    loge("UART Error occurred: 0x%08X", huart->ErrorCode);

    // 重新启动接收
    if (huart->ErrorCode & HAL_UART_ERROR_ORE)
    {
      loge("Overrun error detected, restarting RX");
      HAL_UART_Receive_IT(&huart1, rx_buffer_it, 10);
    }
  }
}

// 修正后的uart_test2函数
void uart_test2()
{
    static uint32_t test_count = 0;
    test_count++;
    
    logi("=== UART Test Round %lu ===", test_count);
    
    // 清除所有标志
    tx_complete_flag = 0;
    rx_complete_flag = 0;
    tx_half_complete_flag = 0;
    rx_half_complete_flag = 0;
    received_length = 0;
    
    // 确保UART处于就绪状态
    if(huart1.gState != HAL_UART_STATE_READY) {
        logw("UART TX not ready (state: %d), aborting...", huart1.gState);
        HAL_UART_AbortTransmit(&huart1);
        osDelay(10);
    }
    
    if(huart1.RxState != HAL_UART_STATE_READY) {
        logw("UART RX not ready (state: %d), aborting...", huart1.RxState);
        HAL_UART_AbortReceive(&huart1);
        osDelay(10);
    }
    
    // 清除错误标志
    __HAL_UART_CLEAR_OREFLAG(&huart1);
    huart1.ErrorCode = HAL_UART_ERROR_NONE;
    
    // === 发送测试 ===
    // 使用更长的消息来测试半完成中断
    const char *long_message = 
        "This is a very long UART interrupt transmit test message. "
        "We need enough data to trigger the half-complete callback. "
        "The message should be long enough to demonstrate the feature. "
        "Adding more text to ensure we have sufficient bytes for testing. "
        "Even more content to make sure the half-complete interrupt works!\r\n";
    
    logi("Starting TX test (message length: %d bytes)", strlen(long_message));
    
    HAL_StatusTypeDef status = HAL_UART_Transmit_IT(&huart1, (uint8_t *)long_message, strlen(long_message));
    
    if (status == HAL_OK) {
        logi("Interrupt transmit started successfully");
        
        // 等待发送完成，检查中间状态
        uint32_t start_time = HAL_GetTick();
        uint32_t timeout = start_time + 2000; // 2秒超时
        
        while (!tx_complete_flag && HAL_GetTick() < timeout) {
            osDelay(1);
            
            // 检查半完成标志
            if (tx_half_complete_flag && !tx_complete_flag) {
                logi("✓ TX Half-complete detected!");
            }
        }
        
        if (tx_complete_flag) {
            uint32_t elapsed = HAL_GetTick() - start_time;
            logi("✓ TX completed in %lu ms", elapsed);
            logi("TX Half-complete flag: %s", tx_half_complete_flag ? "YES" : "NO");
        } else {
            loge("✗ TX timeout!");
            HAL_UART_AbortTransmit(&huart1);
        }
    } else {
        loge("Failed to start interrupt transmit: %d", status);
        return;
    }
    
    // 等待发送完全结束
    osDelay(100);
    
    // === 接收测试 ===
    logi("Starting RX test (expecting 5 bytes)...");
    logi("Please send exactly 5 bytes within 10 seconds");
    
    // 启动接收较少的字节数便于测试
    uint16_t rx_size = 5;
    status = HAL_UART_Receive_IT(&huart1, rx_buffer_it, rx_size);
    
    if (status == HAL_OK) {
        logi("Interrupt receive started successfully");
        
        // 等待接收完成
        uint32_t start_time = HAL_GetTick();
        uint32_t timeout = start_time + 10000; // 10秒超时
        uint32_t last_check_time = start_time;
        
        while (!rx_complete_flag && HAL_GetTick() < timeout) {
            osDelay(100);
            
            // 每秒显示一次进度
            if (HAL_GetTick() - last_check_time >= 1000) {
                uint16_t received_so_far = rx_size - huart1.RxXferCount;
                logi("RX Progress: %d/%d bytes (%.1f sec elapsed)", 
                     received_so_far, rx_size, 
                     (HAL_GetTick() - start_time) / 1000.0f);
                last_check_time = HAL_GetTick();
            }
            
            // 检查错误
            if (huart1.ErrorCode != HAL_UART_ERROR_NONE) {
                loge("UART Error during RX: 0x%08lX", huart1.ErrorCode);
                break;
            }
        }
        
        if (rx_complete_flag) {
            uint32_t elapsed = HAL_GetTick() - start_time;
            logi("✓ RX completed in %lu ms", elapsed);
            logi("Received length: %d", received_length);
            logi("RX Half-complete flag: %s", rx_half_complete_flag ? "YES" : "NO");
            
            // 显示接收到的数据
            rx_buffer_it[received_length] = '\0';
            logi("Received data: \"%s\"", rx_buffer_it);
            
            // 十六进制显示
            logi("Hex data: ");
            for(int i = 0; i < received_length; i++) {
                printf("%02X ", rx_buffer_it[i]);
            }
            printf("\r\n");
            
        } else {
            uint16_t partial_received = rx_size - huart1.RxXferCount;
            logw("✗ RX timeout! Received %d/%d bytes", partial_received, rx_size);
            
            if (partial_received > 0) {
                logi("Partial data received:");
                for(int i = 0; i < partial_received; i++) {
                    printf("%02X ", rx_buffer_it[i]);
                }
                printf("\r\n");
            }
            
            HAL_UART_AbortReceive(&huart1);
        }
    } else {
        loge("Failed to start interrupt receive: %d", status);
        
        // 分析错误原因
        if (status == HAL_BUSY) {
            loge("UART is busy - RxState: %d", huart1.RxState);
        }
    }
    
    logi("=== Test Round %lu Complete ===\n", test_count);
}
#endif

// uart_dma_demo.c - 真正的半完成中断
#include "main.h"
#include "log.h"

extern UART_HandleTypeDef huart1;
static uint8_t tx_dma_buffer[512];
static uint8_t rx_dma_buffer[512];
static volatile uint8_t dma_tx_complete = 0;
static volatile uint8_t dma_tx_half_complete = 0;
static volatile uint8_t dma_rx_complete = 0;
static volatile uint8_t dma_rx_half_complete = 0;

/**
 * @brief DMA发送完成回调
 */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if(huart->Instance == USART1) {
        dma_tx_complete = 1;
        logi("DMA TX Complete!");
    }
}

/**
 * @brief DMA发送半完成回调 - 这个会真正触发！
 */
void HAL_UART_TxHalfCpltCallback(UART_HandleTypeDef *huart)
{
    if(huart->Instance == USART1) {
        dma_tx_half_complete = 1;
        logi("DMA TX Half Complete!");
    }
}

/**
 * @brief DMA接收完成回调
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if(huart->Instance == USART1) {
        dma_rx_complete = 1;
        logi("DMA RX Complete!");
    }
}

/**
 * @brief DMA接收半完成回调 - 这个会真正触发！
 */
void HAL_UART_RxHalfCpltCallback(UART_HandleTypeDef *huart)
{
    if(huart->Instance == USART1) {
        dma_rx_half_complete = 1;
        logi("DMA RX Half Complete!");
    }
}

/**
 * @brief 测试DMA半完成中断
 */
void test_uart_dma_half_complete(void)
{
    logi("=== DMA Half-Complete Test ===");
    
    // 准备发送数据（足够长以触发半完成中断）
    for(unsigned int i = 0; i < sizeof(tx_dma_buffer); i++) {
        tx_dma_buffer[i] = 'A' + (i % 26); // A-Z循环
    }
    
    // 添加结束符
    const char* end_msg = "\r\n=== END ===\r\n";
    strcpy((char*)&tx_dma_buffer[sizeof(tx_dma_buffer) - strlen(end_msg) - 1], end_msg);
    
    // 重置标志
    dma_tx_complete = 0;
    dma_tx_half_complete = 0;
    
    logi("Starting DMA transmit of %d bytes", sizeof(tx_dma_buffer));
    
    // 启动DMA发送
    HAL_StatusTypeDef status = HAL_UART_Transmit_DMA(&huart1, tx_dma_buffer, sizeof(tx_dma_buffer));
    
    if(status == HAL_OK) {
        logi("DMA TX started successfully");
        
        // 等待半完成和完成
        uint32_t start_time = HAL_GetTick();
        uint8_t half_reported = 0;
        
        while(!dma_tx_complete && (HAL_GetTick() - start_time) < 5000) {
            if(dma_tx_half_complete && !half_reported) {
                uint32_t half_time = HAL_GetTick() - start_time;
                logi("✓ DMA TX Half-Complete detected at %lu ms", half_time);
                half_reported = 1;
            }
            osDelay(10);
        }
        
        if(dma_tx_complete) {
            uint32_t total_time = HAL_GetTick() - start_time;
            logi("✓ DMA TX Complete at %lu ms", total_time);
            logi("Half-complete flag: %s", dma_tx_half_complete ? "YES" : "NO");
        } else {
            loge("✗ DMA TX timeout");
            HAL_UART_AbortTransmit(&huart1);
        }
    } else {
        loge("Failed to start DMA TX: %d", status);
    }
}
#include <ctype.h>
/**
 * @brief 测试DMA接收半完成中断
 */
void test_uart_dma_rx_half_complete(void)
{
    logi("=== DMA RX Half-Complete Test ===");
    logi("Please send %d bytes to test half-complete", sizeof(rx_dma_buffer));
    
    // 重置标志和缓冲区
    dma_rx_complete = 0;
    dma_rx_half_complete = 0;
    memset(rx_dma_buffer, 0, sizeof(rx_dma_buffer));
    
    // 启动DMA接收
    HAL_StatusTypeDef status = HAL_UART_Receive_DMA(&huart1, rx_dma_buffer, sizeof(rx_dma_buffer));
    
    if(status == HAL_OK) {
        logi("DMA RX started successfully");
        
        // 等待接收
        uint32_t start_time = HAL_GetTick();
        uint8_t half_reported = 0;
        
        while(!dma_rx_complete && (HAL_GetTick() - start_time) < 30000) { // 30秒超时
            if(dma_rx_half_complete && !half_reported) {
                uint32_t half_time = HAL_GetTick() - start_time;
                logi("✓ DMA RX Half-Complete detected at %lu ms", half_time);
                logi("Received so far: %d bytes", sizeof(rx_dma_buffer) / 2);
                half_reported = 1;
            }
            
            // 显示进度
            if((HAL_GetTick() - start_time) % 2000 == 0) {
                uint16_t remaining = __HAL_DMA_GET_COUNTER(huart1.hdmarx);
                uint16_t received = sizeof(rx_dma_buffer) - remaining;
                logi("Progress: %d/%d bytes received", received, (int)sizeof(rx_dma_buffer));
            }
            
            osDelay(100);
        }
        
        if(dma_rx_complete) {
            uint32_t total_time = HAL_GetTick() - start_time;
            logi("✓ DMA RX Complete at %lu ms", total_time);
            logi("Half-complete flag: %s", dma_rx_half_complete ? "YES" : "NO");
            
            // 显示接收到的数据（前100字节）
            logi("First 100 bytes received:");
            for(unsigned int i = 0; i < 100 && i < sizeof(rx_dma_buffer); i++) {
                printf("%c", isprint(rx_dma_buffer[i]) ? rx_dma_buffer[i] : '.');
                if((i + 1) % 50 == 0) printf("\r\n");
            }
            printf("\r\n");
        } else {
            uint16_t remaining = __HAL_DMA_GET_COUNTER(huart1.hdmarx);
            uint16_t received = sizeof(rx_dma_buffer) - remaining;
            logw("✗ DMA RX timeout! Received %d/%d bytes", received, (int)sizeof(rx_dma_buffer));
            logw("Half-complete flag: %s", dma_rx_half_complete ? "YES" : "NO");
            
            HAL_UART_AbortReceive(&huart1);
        }
    } else {
        loge("Failed to start DMA RX: %d", status);
    }
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
  (void)argument; // Prevent unused argument warning
  // TIM3_Init(1000,100);
  MX_USART1_UART_Init();
  /* Infinite loop */
  for (;;)
  {
    osDelay(1000);
    // HAL_GPIO_TogglePin(led_GPIO_Port, led_Pin); // Toggle the LED
    // SwitchSystemClock();                        // Switch the system clock
    // MX_USART1_UART_Init();                      // Reinitialize GPIO after clock switch
    // printf("System clock switched.\n"); // Print message to UART
    // uint32_t freq = HAL_RCC_GetSysClockFreq();
    // printf("System Clock Frequency: %lu Hz\n", freq); // Print the system clock frequency
    // PrintSystemClockInfo();
    // logi("elog test");
    // shell_update(); // Update the shell system

    test_uart_dma_half_complete();
    test_uart_dma_rx_half_complete();

  }
  /* USER CODE END 5 */
}

/**
 * @brief  Period elapsed callback in non blocking mode
 * @note   This function is called  when TIM10 interrupt took place, inside
 * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
 * a global variable "uwTick" used as application time base.
 * @param  htim : TIM handle
 * @retval None
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */
  if (htim->Instance == TIM1)
  {
    HAL_GPIO_TogglePin(led_GPIO_Port, led_Pin); // LED1反转
  }
  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM10)
  {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
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
    HAL_GPIO_WritePin(led_GPIO_Port, led_Pin, GPIO_PIN_RESET); // Turn on the LED to indicate error
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
