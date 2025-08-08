- [English](README.md)  
- [Chinese](README_cn.md)

# Windows SAM Test Program

This program is a Windows-based test example for the SIMCom Application Module AT driver (SAM_ATCDRV). It demonstrates how to communicate with a SIMCom module via serial port and drive various business features (such as MQTT, Socket, TTS, FOTA, etc.).

## Features

- Supports serial port initialization and configuration (baud rate, data bits, parity, stop bits, etc.).
- Allows automatic or manual selection of the serial port (controlled by the `WIN_COM_SELECT` macro).
- Implements data transmission (`SendtoCom`) and reception (`ReadfoCom`) interfaces.
- Provides millisecond-level system tick (`GetSysTickCnt`) and log output (`OutputLog`) functions.
- Calls `TesterInit()` for business module initialization and repeatedly calls `TesterProc()` for business logic processing.
- Supports safe program exit by entering the "STOP" command in the console.

## Usage

1. Compile the program (ensure the SAM_ATCDRV static library is built and linked properly).
2. Run the program. If `WIN_COM_SELECT` is enabled, you can select the COM port at startup; otherwise, the port number is defined by the `ATC_COM` macro.
3. The program will initialize the serial port and enter the main loop, continuously calling `TesterProc()` to process business logic.
4. Enter "STOP" in the console to safely exit the program.

## Main Code Structure

- `main()`: Entry point; handles serial port initialization, main loop, and command processing.
- `SysInitUart()`: Serial port initialization and configuration.
- `SendtoCom()` / `ReadfoCom()`: Serial data transmission and reception interfaces.
- `ComDrvPoll()`: Serial port polling for data transmission and reception.
- `GetSysTickCnt()` / `GetSysRtcTime()`: System time functions.
- `OutputLog()`: Log output interface.

## Notes

- Modify macro definitions or select the COM port according to your actual device.
- Depends on Windows-specific headers and SAM_ATCDRV headers and libraries.
- Business logic is handled by `TesterInit()` and `TesterProc()`. For details, refer to the SAM_ATCDRV documentation and source code.

For more information about the business modules, please refer to the SAM_ATCDRV directory and related documentation.