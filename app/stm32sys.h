#ifndef STM32_SYS_H_
#define STM32_SYS_H_

// defined(STM32L475_NO_OS)
#if 0
#include "stm32l4xx.h"

void delay_us(uint32_t);

// LED
#define LED_R(n)			(n?HAL_GPIO_WritePin(GPIOE,GPIO_PIN_7,GPIO_PIN_SET):HAL_GPIO_WritePin(GPIOE,GPIO_PIN_7,GPIO_PIN_RESET))
#define LED_R_TogglePin		HAL_GPIO_TogglePin(GPIOE,GPIO_PIN_7)

#define LED_G(n)			(n?HAL_GPIO_WritePin(GPIOE,GPIO_PIN_8,GPIO_PIN_SET):HAL_GPIO_WritePin(GPIOE,GPIO_PIN_8,GPIO_PIN_RESET))
#define LED_G_TogglePin     HAL_GPIO_TogglePin(GPIOE,GPIO_PIN_8)

#define LED_B(n)			(n?HAL_GPIO_WritePin(GPIOE,GPIO_PIN_9,GPIO_PIN_SET):HAL_GPIO_WritePin(GPIOE,GPIO_PIN_9,GPIO_PIN_RESET))
#define LED_B_TogglePin     HAL_GPIO_TogglePin(GPIOE,GPIO_PIN_9)

void LED_Init(void);

// 

#else



#endif // end STM32L475_NO_OS
#endif // end STM32_SYS_H_