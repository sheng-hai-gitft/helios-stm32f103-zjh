/* MCU 支持包（MSP）初始化。串口 / I2C / DMA 的底层引脚与时钟在这里配置 */
#include "main.h"

void HAL_MspInit(void)
{
  __HAL_RCC_AFIO_CLK_ENABLE();
  __HAL_RCC_PWR_CLK_ENABLE();

  /* 关掉 JTAG，保留 SWD：PB3/PB4/PA15 可当普通 IO 用，同时仍可下载调试 */
  __HAL_AFIO_REMAP_SWJ_NOJTAG();
}
