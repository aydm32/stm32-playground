#ifndef MAIN_H
#define MAIN_H
#include <stdint.h>

/* LED configuration */
#define LED_GPIO_PORT GPIOC
#define LED_GPIO_PIN GPIO_PIN_13

/* Button configuration */
#define BUTTON_GPIO_PORT GPIOA
#define BUTTON_GPIO_PIN GPIO_PIN_0

typedef struct {
  uint16_t history;
  uint8_t state;
} button_t;

void Error_Handler(void);

#endif // !MAIN_H
