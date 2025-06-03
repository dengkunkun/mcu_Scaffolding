#include "stm32f4xx_hal.h"
#include "main.h"
#include "stdio.h"
#include "string.h"
#include "stdint.h"
#include "stdbool.h"
extern void Error_Handler(void);

void PrintSystemClockInfo(void)
{
  uint32_t sysclk_freq, hclk_freq, pclk1_freq, pclk2_freq;
  uint32_t pll_source, sysclk_source;
  uint32_t voltage_scale;
  char clock_source_str[32];
  char pll_source_str[16];
  char voltage_str[32];
  
  // 获取各种时钟频率
  sysclk_freq = HAL_RCC_GetSysClockFreq();
  hclk_freq = HAL_RCC_GetHCLKFreq();
  pclk1_freq = HAL_RCC_GetPCLK1Freq();
  pclk2_freq = HAL_RCC_GetPCLK2Freq();
  
  // 获取系统时钟源
  sysclk_source = __HAL_RCC_GET_SYSCLK_SOURCE();
  switch(sysclk_source)
  {
    case RCC_SYSCLKSOURCE_STATUS_HSI:
      strcpy(clock_source_str, "HSI");
      break;
    case RCC_SYSCLKSOURCE_STATUS_HSE:
      strcpy(clock_source_str, "HSE");
      break;
    case RCC_SYSCLKSOURCE_STATUS_PLLCLK:
      strcpy(clock_source_str, "PLL");
      // 获取PLL源
      pll_source = __HAL_RCC_GET_PLL_OSCSOURCE();
      if(pll_source == RCC_PLLSOURCE_HSI)
      {
        strcpy(pll_source_str, "(HSI)");
      }
      else if(pll_source == RCC_PLLSOURCE_HSE)
      {
        strcpy(pll_source_str, "(HSE)");
      }
      else
      {
        strcpy(pll_source_str, "(Unknown)");
      }
      strcat(clock_source_str, pll_source_str);
      break;
    default:
      strcpy(clock_source_str, "Unknown");
      break;
  }
  
  // 获取电压缩放等级
  voltage_scale = HAL_PWREx_GetVoltageRange();
  switch(voltage_scale)
  {
    case PWR_REGULATOR_VOLTAGE_SCALE1:
      strcpy(voltage_str, "Scale 1 (High Performance)");
      break;
    case PWR_REGULATOR_VOLTAGE_SCALE2:
      strcpy(voltage_str, "Scale 2 (Medium Performance)");
      break;
    case PWR_REGULATOR_VOLTAGE_SCALE3:
      strcpy(voltage_str, "Scale 3 (Low Power)");
      break;
    default:
      strcpy(voltage_str, "Unknown Scale");
      break;
  }
  
  // 打印所有信息
  printf("\r\n========== System Clock Information ==========\r\n");
  printf("Clock Source    : %s\r\n", clock_source_str);
  printf("SYSCLK Frequency: %lu Hz (%.2f MHz)\r\n", sysclk_freq, (float)sysclk_freq/1000000.0f);
  printf("HCLK Frequency  : %lu Hz (%.2f MHz)\r\n", hclk_freq, (float)hclk_freq/1000000.0f);
  printf("PCLK1 Frequency : %lu Hz (%.2f MHz)\r\n", pclk1_freq, (float)pclk1_freq/1000000.0f);
  printf("PCLK2 Frequency : %lu Hz (%.2f MHz)\r\n", pclk2_freq, (float)pclk2_freq/1000000.0f);
  printf("Voltage Scale   : %s\r\n", voltage_str);
  
  // 检查电压调节器状态
  if(__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY))
  {
    printf("Voltage Status  : Ready\r\n");
  }
  else
  {
    printf("Voltage Status  : Not Ready\r\n");
  }
  
  // 如果使用PLL，显示PLL配置
  if(sysclk_source == RCC_SYSCLKSOURCE_STATUS_PLLCLK)
  {
    printf("PLL Configuration:\r\n");
    printf("  PLLM: %lu\r\n", (RCC->PLLCFGR & RCC_PLLCFGR_PLLM) >> RCC_PLLCFGR_PLLM_Pos);
    printf("  PLLN: %lu\r\n", (RCC->PLLCFGR & RCC_PLLCFGR_PLLN) >> RCC_PLLCFGR_PLLN_Pos);
    printf("  PLLP: %lu\r\n", ((RCC->PLLCFGR & RCC_PLLCFGR_PLLP) >> RCC_PLLCFGR_PLLP_Pos) + 1);
    printf("  PLLQ: %lu\r\n", (RCC->PLLCFGR & RCC_PLLCFGR_PLLQ) >> RCC_PLLCFGR_PLLQ_Pos);
  }
  
  printf("==============================================\r\n\r\n");
}

static void SystemClockHSI_Config(int m, int n, int p, int q)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  /* -1- Select HSE bypass as system clock source to allow modification of the PLL configuration */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_SYSCLK;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSE;
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    /* Initialization Error */
    Error_Handler();
  }

  /* -2- Enable HSI Oscillator, select it as PLL source and finally activate the PLL */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = m;
  RCC_OscInitStruct.PLL.PLLN = n;
  RCC_OscInitStruct.PLL.PLLP = p;
  RCC_OscInitStruct.PLL.PLLQ = q;
  // RCC_OscInitStruct.PLL.PLLR             = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    /* Initialization Error */
    Error_Handler();
  }
  /* Activate the OverDrive to reach the 180 MHz Frequency */
  // if(HAL_PWREx_EnableOverDrive() != HAL_OK)
  // {
  //   Error_Handler();
  // }
  if(HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    /* Initialization Error */
    Error_Handler();
  }
  /* -3- Select the PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 clocks dividers */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    /* Initialization Error */
    Error_Handler();
  }

  /* -4- Optional: Disable HSE bypass Oscillator (if the HSE bypass is no more needed by the application) */
  // RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  // RCC_OscInitStruct.HSEState = RCC_HSE_OFF;
  // RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  // if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  // {
  //   /* Initialization Error */
  //   Error_Handler();
  // }
  if(PWR_REGULATOR_VOLTAGE_SCALE1!=HAL_PWREx_GetVoltageRange())
  {
    /* Initialization Error */
    Error_Handler();
  }
}
static void SystemClockHSEbypass_Config(int m, int n, int p, int q)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  /* -1- Select HSI as system clock source to allow modification of the PLL configuration */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_SYSCLK;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    /* Initialization Error */
    Error_Handler();
  }

  /* -2- Enable HSE bypass Oscillator, select it as PLL source and finally activate the PLL */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLM = m;
  RCC_OscInitStruct.PLL.PLLN = n;
  RCC_OscInitStruct.PLL.PLLP = p;
  RCC_OscInitStruct.PLL.PLLQ = q;
  // RCC_OscInitStruct.PLL.PLLR              = 2;

  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    /* Initialization Error */
    Error_Handler();
  }
  if(HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE3) != HAL_OK)
  {
    /* Initialization Error */
    Error_Handler();
  }
  /* -3- Select the PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 clocks dividers */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    /* Initialization Error */
    Error_Handler();
  }
  while ((__HAL_RCC_GET_FLAG(RCC_FLAG_HSERDY) == RESET) || __HAL_RCC_GET_FLAG(RCC_FLAG_PLLRDY) == RESET)
  {
  }
  /* -4- Optional: Disable HSI Oscillator (if the HSI is no more needed by the application)*/
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_OFF;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    /* Initialization Error */
    Error_Handler();
  }
  if(PWR_REGULATOR_VOLTAGE_SCALE3!=HAL_PWREx_GetVoltageRange())
  {
    /* Initialization Error */
    Error_Handler();
  }
}
__IO FlagStatus SwitchClock = RESET;
void SwitchSystemClock(void)
{
  if (__HAL_RCC_GET_PLL_OSCSOURCE() == RCC_PLLSOURCE_HSI)
  {
    /* PLL source is HSE oscillator */
    /* Set SYSCLK frequency to 180000000 Hz, coming from the PLL which is clocked by HSE bypass */
    SystemClockHSEbypass_Config(12, 96, 2, 4);
  }
  else if (__HAL_RCC_GET_PLL_OSCSOURCE() == RCC_PLLSOURCE_HSE)
  {
    /* PLL source is HSE bypass oscillator */
    /* Set SYSCLK frequency to 80000000 Hz, coming from the PLL which is clocked by HSI */
    SystemClockHSI_Config(8, 100, 4, 4);
  }
  
  /* reset global variable */
  SwitchClock = RESET;
}