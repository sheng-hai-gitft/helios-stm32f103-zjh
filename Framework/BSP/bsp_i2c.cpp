#include "bsp_i2c.hpp"
#include "stm32f1xx_hal.h"

// CubeMX 生成的全局句柄
extern I2C_HandleTypeDef hi2c1;
extern I2C_HandleTypeDef hi2c2;

namespace bsp {

I2cBus::I2cBus(I2cPeriph periph)
    : periph_(periph)
{
    if (periph_ == I2cPeriph::I2C_1) hi2c_handle_ = &hi2c1;
    else hi2c_handle_ = &hi2c2;
}

bool I2cBus::open(const I2cConfig& cfg)
{
    auto* hi2c = static_cast<I2C_HandleTypeDef*>(hi2c_handle_);
    if (!hi2c) return false;

    HAL_I2C_DeInit(hi2c);
    hi2c->Init.ClockSpeed = cfg.clock_hz;
    hi2c->Init.OwnAddress1 = cfg.own_addr;
    hi2c->Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c->Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c->Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c->Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;

    is_opened_ = (HAL_I2C_Init(hi2c) == HAL_OK);
    return is_opened_;
}

void I2cBus::close()
{
    auto* hi2c = static_cast<I2C_HandleTypeDef*>(hi2c_handle_);
    if (hi2c && is_opened_) {
        HAL_I2C_DeInit(hi2c);
        is_opened_ = false;
    }
}

// 核心：单一通讯接口 + 映射错误类型
I2cResult I2cBus::mem_transfer(uint8_t dev_addr, uint8_t reg,
                               uint8_t* buf, uint16_t len,
                               I2cDir dir, uint32_t timeout_ms)
{
    if (!is_opened_) return I2cError::NotOpened;
    if (!buf || len == 0) return I2cError::InvalidParam;

    auto* hi2c = static_cast<I2C_HandleTypeDef*>(hi2c_handle_);
    uint16_t dev_addr_8bit = static_cast<uint16_t>(dev_addr << 1);
    HAL_StatusTypeDef hal_ret;

    if (dir == I2cDir::Read) {
        hal_ret = HAL_I2C_Mem_Read(hi2c, dev_addr_8bit, reg,
                                   I2C_MEMADD_SIZE_8BIT, buf, len, timeout_ms);
    } else {
        hal_ret = HAL_I2C_Mem_Write(hi2c, dev_addr_8bit, reg,
                                    I2C_MEMADD_SIZE_8BIT, buf, len, timeout_ms);
    }

    // HAL 状态映射到自定义错误类型
    switch (hal_ret) {
        case HAL_OK:      return len;          // 成功：返回传输字节数
        case HAL_BUSY:    return I2cError::Busy;
        case HAL_ERROR:   return I2cError::Nack;
        case HAL_TIMEOUT: return I2cError::Timeout;
        default:          return I2cError::Nack;
    }
}

// 总线复位函数（GPIO模拟时序，引脚按你实际硬件修改）
void I2cBus::bus_reset()
{
    GPIO_InitTypeDef gpio_cfg = {0};
    GPIO_TypeDef *scl_port, *sda_port;
    uint16_t scl_pin, sda_pin;

    // 请根据你的实际引脚配置修改
    if (periph_ == I2cPeriph::I2C_1) {
        scl_port = GPIOB; scl_pin = GPIO_PIN_6;
        sda_port = GPIOB; sda_pin = GPIO_PIN_7;
    } else {
        scl_port = GPIOB; scl_pin = GPIO_PIN_10;
        sda_port = GPIOB; sda_pin = GPIO_PIN_11;
    }

    // 切换为GPIO输出，发9个时钟释放SDA
    gpio_cfg.Pin = scl_pin | sda_pin;
    gpio_cfg.Mode = GPIO_MODE_OUTPUT_OD;
    gpio_cfg.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(scl_port, &gpio_cfg);

    HAL_GPIO_WritePin(sda_port, sda_pin, GPIO_PIN_SET);
    for (int i = 0; i < 9; i++) {
        HAL_GPIO_WritePin(scl_port, scl_pin, GPIO_PIN_RESET);
        HAL_Delay(1);
        HAL_GPIO_WritePin(scl_port, scl_pin, GPIO_PIN_SET);
        HAL_Delay(1);
    }

    // 重新初始化硬件I2C
    auto* hi2c = static_cast<I2C_HandleTypeDef*>(hi2c_handle_);
    if (hi2c) HAL_I2C_Init(hi2c);
}

} // namespace bsp