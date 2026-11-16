#include "main.h"
#include "stm32f1xx_hal.h"

static volatile uint8_t press_event;
static volatile uint8_t button_initialized = 0;
static button_t btn;
static void GPIO_Init() {
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();

  GPIO_InitStruct.Pin = LED_GPIO_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

  HAL_GPIO_Init(LED_GPIO_PORT, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = BUTTON_GPIO_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;

  HAL_GPIO_Init(BUTTON_GPIO_PORT, &GPIO_InitStruct);
  button_initialized = 1;
}

static void SystemClock_Config() {
  RCC_OscInitTypeDef osc = {0};
  RCC_ClkInitTypeDef clk = {0};
  // Blue Pill has 8 Mhz external clock
  // HSE -> on, PLL = HSE x 9 = 8 x 9 = 72 Mhz
  osc.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  osc.HSEState = RCC_HSE_ON;
  osc.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  osc.PLL.PLLState = RCC_PLL_ON;
  osc.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  osc.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&osc) != HAL_OK) {
    Error_Handler();
  }
  // Bus clocks and flash latency
  clk.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                  RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  clk.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  clk.AHBCLKDivider = RCC_SYSCLK_DIV1;
  clk.APB1CLKDivider = RCC_HCLK_DIV2;
  clk.APB2CLKDivider = RCC_HCLK_DIV1;
  if (HAL_RCC_ClockConfig(&clk, FLASH_LATENCY_2) != HAL_OK) {
    Error_Handler();
  }
}

static int8_t button_update(button_t *b, uint8_t raw) {
  b->history = (uint16_t)((b->history << 1) | raw);
  if (b->history == 0xFFFFU && !b->state) {
    b->state = 1;
    return +1;
  }
  if (b->history == 0x0000U && b->state) {
    b->state = 0;
    return -1;
  }
  return 0;
}

void button_tick_1ms(void) {
  if (!button_initialized)
    return;

  uint8_t raw =
      (HAL_GPIO_ReadPin(BUTTON_GPIO_PORT, BUTTON_GPIO_PIN) == GPIO_PIN_RESET);

  if (button_update(&btn, raw) > 0)
    press_event = 1;
}

void Error_Handler(void) {
  __disable_irq();

  while (1) {
  }
}

int main(void) {
  HAL_Init();
  SystemClock_Config();
  GPIO_Init();

  while (1) {
    uint8_t event;

    __disable_irq();
    event = press_event;
    press_event = 0;
    __enable_irq();

    if (event) {
      HAL_GPIO_TogglePin(LED_GPIO_PORT, LED_GPIO_PIN);
    }
    __WFI();
  }
}
