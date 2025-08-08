- [English](README.md)  
- [简体中文](README_cn.md)

# Linux SAM 测试程序

本程序为 SIMCom 模组 AT 驱动（SAM_ATCDRV）在 Linux 平台下的测试示例，主要用于通过串口与模块进行通信，并驱动各类业务功能（如 MQTT、Socket、TTS、FOTA 等）。

## 功能简介

- 初始化串口，配置波特率、数据位、校验位等参数。
- 通过 `SendtoCom` 和 `ReadfoCom` 实现与模块的数据收发。
- 提供毫秒级延时函数 `msleep` 和系统时间戳获取函数 `GetSysTickCnt`。
- 通过 `TesterInit()` 完成各业务模块初始化，通过 `TesterProc()` 轮询处理业务逻辑。
- 支持通过命令行参数 `-D` 指定串口设备（如 `/dev/ttyUSB0`）。

## 使用方法

1. 编译本程序（需确保已编译好 SAM_ATCDRV 静态库，并正确链接）。
2. 运行示例程序，指定串口设备：
   ```sh
   ./linux_sam_test -D /dev/ttyUSB0
3. 程序启动后会自动初始化串口并进入主循环，不断调用 TesterProc() 处理各业务。