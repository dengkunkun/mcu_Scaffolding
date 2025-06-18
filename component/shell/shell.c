#include <string.h>
#include "lwshell/lwshell.h"

#include "stm32f4xx_hal.h"
#include "log.h"
#define LWSHELL_INPUT_BUFFER_SIZE 128
void shell_update(void)
{
    extern UART_HandleTypeDef huart1;
    static uint8_t input_buffer[LWSHELL_INPUT_BUFFER_SIZE];
    if(HAL_UART_Receive_IT(&huart1, input_buffer,LWSHELL_INPUT_BUFFER_SIZE)!=HAL_OK)
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