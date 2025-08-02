#if defined(STM32F4) || defined(STM32F411xE)
// #pragma message("use STM32F4 family HAL")
#include "stm32f4xx_hal.h"
#elif defined(STM32H7) || defined(STM32H7xx) || defined(STM32H743xx) || defined(STM32H753xx)
// #pragma message("use STM32H7 family HAL")
#include "stm32h7xx_hal.h"
#elif defined(STM32F1) || defined(STM32F103xE)
// #pragma message("use STM32F1 family HAL")
#include "stm32f1xx_hal.h"
#else
#error "Unknown STM32 family, HAL header not included"
#endif
