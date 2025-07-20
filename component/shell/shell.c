#include <string.h>
#include "lwshell/lwshell.h"
#include "log.h"
#if defined(STM32F4) || defined(STM32F411xE)
#pragma message("use STM32F4 family HAL")
#include "stm32f4xx_hal.h"
#elif defined(STM32H7) || defined(STM32H7xx) || defined(STM32H743xx) || defined(STM32H753xx)
#pragma message("use STM32H7 family HAL")
#include "stm32h7xx_hal.h"
#elif defined(STM32F1) || defined(STM32F103xE)
#pragma message("use STM32F1 family HAL")
#include "stm32f1xx_hal.h"
#else
#error "Unknown STM32 family, HAL header not included"
#endif

#define LWSHELL_INPUT_BUFFER_SIZE 128
void shell_update(void)
{
    extern UART_HandleTypeDef huart1;
    static uint8_t input_buffer[LWSHELL_INPUT_BUFFER_SIZE];
    if (HAL_UART_Receive_IT(&huart1, input_buffer, LWSHELL_INPUT_BUFFER_SIZE) != HAL_OK)
    {
        /* Error handling */
        loge("Error receiving data\r\n");
        return;
    }
    lwshell_input(input_buffer, huart1.RxXferSize);
    huart1.RxXferSize = 0; // Reset the Rx transfer size after processing
}
int32_t mycmd_fn(int32_t argc, char **argv)
{
    loge("mycmd_fn called. Number of argv: %d\r\n", (int)argc);
    for (int32_t i = 0; i < argc; ++i)
    {
        loge("ARG[%d]: %s\r\n", (int)i, argv[i]);
    }

    /* Successful execution */
    return 0;
}

/* Example code */
void shell_init(void)
{
    /* Init library */
    lwshell_init();

    /* Define shell commands */
    lwshell_register_cmd("mycmd", mycmd_fn, "Adds 2 integer numbers and prints them");

    /* User input to process every character */

    /* Now insert input */
}