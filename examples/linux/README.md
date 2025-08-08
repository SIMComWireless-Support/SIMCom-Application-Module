- [English](README.md)  
- [Chinese](README_cn.md)

# Linux SAM Test Program

This program is a Linux-based test example for the SIMCom Application Module AT driver (SAM_ATCDRV). It demonstrates how to communicate with a SIMCom module via serial port and drive various business features (such as MQTT, Socket, TTS, FOTA, etc.).

## Features

- Initializes the serial port with configurable baud rate, data bits, parity, stop bits, and flow control.
- Implements data transmission and reception with the module using `SendtoCom` and `ReadfoCom`.
- Provides millisecond-level delay (`msleep`) and system tick count (`GetSysTickCnt`) functions.
- Calls `TesterInit()` for business module initialization and repeatedly calls `TesterProc()` for business logic processing.
- Supports specifying the serial device via the `-D` command-line option (e.g., `/dev/ttyUSB0`).

## Usage

1. Compile the program (ensure the SAM_ATCDRV static library is built and linked properly).
2. Run the program, specifying the serial device:
   ```sh
   ./linux_sam_test -D /dev/ttyUSB0
3. The program will initialize the serial port and enter the main loop, continuously calling TesterProc() to process business logic.