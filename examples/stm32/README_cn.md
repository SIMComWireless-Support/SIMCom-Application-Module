- [English](README.md)  
- [简体中文](README_cn.md)

# STM32 SAM 测试程序

本程序为 SIMCom 模组 AT 驱动（SAM_ATCDRV）在 STM32 平台下的测试示例，主要用于通过串口与 SIMCom 模组进行通信，并驱动各类业务功能（如 MQTT、Socket、TTS、FOTA 等）。

## 功能简介

- 初始化 STM32 外设（GPIO、DMA、UART、USB 等），配置系统时钟。
- 通过 UART 实现与 SIMCom 模组的数据收发，支持 AT 通道和调试通道。
- 提供系统毫秒计时（`GetSysTickCnt`）接口。
- 通过 `TesterInit()` 完成业务模块初始化，`TesterProc()` 轮询处理业务逻辑。
- 支持通过 `bsp_usart_write` 和 `bsp_usart_read` 进行底层串口收发。
- 重定向 `printf` 到 UART，方便调试输出。

## 使用方法

1. 使用 STM32CubeMX 或相关工具生成工程，并将本文件及相关驱动文件添加到工程中。
2. 编译并下载程序到 STM32 开发板。
3. 连接 SIMCom 模组到指定 UART 接口，复位并启动开发板。
4. 程序启动后会自动初始化外设并进入主循环，不断调用 `TesterProc()` 处理业务。

## 主要代码结构

- `main()`：程序入口，负责外设初始化、主循环等。
- `SendtoCom()` / `ReadfoCom()`：串口数据收发接口，支持 AT 通道和调试通道。
- `GetSysTickCnt()`：获取系统毫秒计时。
- `PUTCHAR_PROTOTYPE`：重定向 `printf` 到 UART。
- `SystemClock_Config()`、`MX_GPIO_Init()`、`MX_DMA_Init()`、`MX_LPUART1_UART_Init()`、`MX_USART3_UART_Init()` 等：外设初始化函数。

## 注意事项

- 需根据实际硬件连接修改 UART 端口和引脚配置。
- 依赖于 STM32 HAL 库和相关底层驱动（如 `bsp_usart_write`、`bsp_usart_read`）。
- 业务处理逻辑由 `TesterInit()` 和 `TesterProc()` 驱动，具体功能可参考 SAM_ATCDRV 文档及源码。

如需进一步了解各业务模块的实现，请参考 SAM_ATCDRV 目录及相关文档。