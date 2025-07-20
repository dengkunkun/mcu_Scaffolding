#include "log.h"
#include "shell.h"

#include "main.h"
#include "cmsis_os2.h"

#define LED_RED_TOGGLE() HAL_GPIO_TogglePin(led_red_GPIO_Port, led_red_Pin)
#define LED_BLUE_TOGGLE() HAL_GPIO_TogglePin(led_blue_GPIO_Port, led_blue_Pin)
#define LED_GREEN_TOGGLE() HAL_GPIO_TogglePin(led_green_GPIO_Port, led_green_Pin)
int app_main(void)
{
    log_init();   // Initialize the logging system
    shell_init(); // Initialize the shell system
    while (1)
    {
        osDelay(1000);
        logi("hello world");
        LED_RED_TOGGLE();
        LED_GREEN_TOGGLE();
        LED_BLUE_TOGGLE();
    }
}
