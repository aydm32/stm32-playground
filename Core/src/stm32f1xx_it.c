#include "stm32f1xx_hal.h"

void button_tick_1ms(void);

void SysTick_Handler(void) {
  HAL_IncTick();
  button_tick_1ms();
}
