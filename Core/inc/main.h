#ifndef MAIN_H
#define MAIN_H

#include "stm32f1xx_hal.h"

/* Blue Pill: user LED on PC13, active-low (on when pin is LOW) */
#define LED_GPIO_PORT GPIOC
#define LED_GPIO_PIN GPIO_PIN_13

void Error_Handler(void);

#endif
