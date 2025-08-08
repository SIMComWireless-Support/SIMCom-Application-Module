- [English](README.md)  
- [简体中文](README_cn.md)

# Windows SAM 测试程序

本程序为 SIMCom 模组 AT 驱动（SAM_ATCDRV）在 Windows 平台下的测试示例，主要用于通过串口与模块进行通信，并驱动各类业务功能（如 MQTT、Socket、TTS、FOTA 等）。

## 功能简介

- 支持串口初始化、配置波特率、数据位、校验位、停止位等参数。
- 支持自动或手动选择串口（可通过宏 `WIN_COM_SELECT` 控制）。
- 实现了数据的发送（`SendtoCom`）和接收（`ReadfoCom`）接口。
- 提供毫秒级系统计时（`GetSysTickCnt`）和日志输出（`OutputLog`）功能。
- 通过 `TesterInit()` 完成业务模块初始化，`TesterProc()` 轮询处理业务逻辑。
- 支持通过输入 "STOP" 命令安全退出程序。

## 使用方法

1. 编译本程序（需确保已编译好 SAM_ATCDRV 静态库，并正确链接）。
2. 运行程序，若启用 `WIN_COM_SELECT`，可在启动时选择串口号；否则默认使用 `ATC_COM` 宏指定的串口号。
3. 程序启动后会自动初始化串口并进入主循环，不断调用 `TesterProc()` 处理业务。
4. 在控制台输入 "STOP" 可安全退出程序。

## 主要代码结构

- `main()`：程序入口，负责串口初始化、主循环、命令处理等。
- `SysInitUart()`：串口初始化与配置。
- `SendtoCom()` / `ReadfoCom()`：串口数据收发接口。
- `ComDrvPoll()`：串口收发数据轮询。
- `GetSysTickCnt()` / `GetSysRtcTime()`：系统时间获取。
- `OutputLog()`：日志输出接口。

## 注意事项

- 需根据实际串口设备名称和编号修改宏定义或选择串口。
- 依赖于 Windows 平台相关头文件和 SAM_ATCDRV 相关头文件及库。
- 业务处理逻辑由 `TesterInit()` 和 `TesterProc()` 驱动，具体功能可参考 SAM_ATCDRV 文档及源码。

如需进一步了解各业务模块的实现，请参考 SAM_ATCDRV 目录及相关文档。