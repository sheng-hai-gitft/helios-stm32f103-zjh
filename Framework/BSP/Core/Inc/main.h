#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f1xx_hal.h"

void Error_Handler(void);

/* 板载 LED（蓝板 PC13，低电平点亮）——仅作为编译/烧录冒烟测试，可随时删除 */
#define LED_Pin        GPIO_PIN_13
#define LED_GPIO_Port  GPIOC

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
