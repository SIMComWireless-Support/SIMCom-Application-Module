- [English](README.md)  
- [Chinese](README_cn.md)

# STM32 SAM Test Program

This project is a test example for the SIMCom Application Module AT driver (SAM_ATCDRV) running on an STM32 microcontroller. It demonstrates how to communicate with a SIMCom module via UART and drive various business features (such as MQTT, Socket, TTS, FOTA, etc.) in an embedded environment.

## Features

- Initializes STM32 peripherals (GPIO, DMA, UART, USB, etc.) and configures the system clock.
- Implements UART communication with the SIMCom module, supporting both AT and debug channels.
- Provides system tick count via `GetSysTickCnt()` for millisecond timing.
- Implements `SendtoCom()` and `ReadfoCom()` for data transmission and reception with the module.
- Redirects `printf` output to UART for convenient debugging.
- Calls `TesterInit()` for business module initialization and repeatedly calls `TesterProc()` for business logic processing in the main loop.

## Usage

1. Use STM32CubeMX or a similar tool to generate the project and add this file and related drivers to your project.
2. Compile and flash the program to your STM32 development board.
3. Connect the SIMCom module to the designated UART interface on your board.
4. After reset and startup, the program will initialize all peripherals and enter the main loop, continuously calling `TesterProc()` to handle business logic.

## Main Code Structure

- `main()`: Entry point; handles peripheral initialization and the main loop.
- `SendtoCom()` / `ReadfoCom()`: UART data transmission and reception interfaces for AT and debug channels.
- `GetSysTickCnt()`: Returns the system tick count in milliseconds.
- `PUTCHAR_PROTOTYPE`: Redirects `printf` to UART for debugging.
- Peripheral initialization functions: `SystemClock_Config()`, `MX_GPIO_Init()`, `MX_DMA_Init()`, `MX_LPUART1_UART_Init()`, `MX_USART3_UART_Init()`, `MX_USB_OTG_FS_PCD_Init()`, etc.

## Notes

- Modify UART port and pin configurations according to your hardware setup.
- Depends on STM32 HAL library and related low-level drivers (such as `bsp_usart_write`, `bsp_usart_read`).
- Business logic is handled by `TesterInit()` and `TesterProc()`. For details, refer to the SAM_ATCDRV documentation and source code.

For more information about the business modules, please refer to the SAM_ATCDRV directory and related documentation.