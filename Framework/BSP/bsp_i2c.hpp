#ifndef BSP_I2C_HPP
#define BSP_I2C_HPP

#include <cstdint>
#include <variant>  // C++17 多类型返回核心

namespace bsp {

// I2C 外设标识
enum class I2cPeriph { I2C_1, I2C_2 };

// 传输方向
enum class I2cDir { Read, Write };

// 错误类型：不同错误对应不同枚举，上层可分别处理
enum class I2cError {
    NotOpened,     // 外设未初始化
    InvalidParam,  // 参数非法
    Busy,          // 总线被占用
    Nack,          // 从机无应答/硬件错误
    Timeout        // 传输超时（总线死锁典型表现）
};

// 统一返回类型：成功=传输字节数，失败=具体错误
using I2cResult = std::variant<uint16_t, I2cError>;

// I2C 配置
struct I2cConfig {
    uint32_t clock_hz;  // 100000=标准模式，400000=快速模式
    uint16_t own_addr = 0;
};

class I2cBus {
public:
    explicit I2cBus(I2cPeriph periph);

    // 打开/关闭总线
    bool open(const I2cConfig& cfg);
    void close();

    // ===== 唯一对外通讯接口 =====
    // dev_addr: 从机7位地址
    // reg: 从机寄存器地址
    // buf: 数据缓冲区
    // len: 传输长度
    // dir: 读/写
    // timeout_ms: 超时时间
    I2cResult mem_transfer(uint8_t dev_addr, uint8_t reg,
                           uint8_t* buf, uint16_t len,
                           I2cDir dir, uint32_t timeout_ms = 100);

    // 总线复位（解决死锁）
    void bus_reset();

private:
    I2cPeriph periph_;
    void* hi2c_handle_ = nullptr;  // 用void*彻底隐藏HAL类型
    bool is_opened_ = false;
};

} // namespace bsp

#endif