/* run this program using the console pauser or add your own getch, system("pause") or input loop */
#include <windows.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <setupapi.h>
#include <initguid.h>
#include <devguid.h>

#include "include.h"

//#define WIN_COM_SELECT	//if enable this,you are able to select which serial port will be used for AT cmd,setupapi lib will be used from the compiler.

#ifdef	WIN_COM_SELECT		

#define MAX_PORTS 64
int PortNumUsed = 0;
typedef struct {
	char portName[256];
	char friendlyName[256];
} ComPortInfo;

#endif


//Uart Com
HANDLE hSerial;
UartTag UartATC;
UartTag UartDBG;
//UART
#define UART_ATC	0x01
#define UART_DBG	0x02


static char KeyBuffer[256];
static uint8 CmdBp;

/////////////////////////////////////////////////
// UART 
/////////////////////////////////////////////////
void InitCom(uint8 com)
{
	if(com == UART_ATC)
	{
		UartATC.rinp = 0;
		UartATC.routp = 0;
		UartATC.sendbuf = UartATC.sbuf;
		UartATC.scnt = 0;
		UartATC.sdp = 0;
		UartATC.ctrsta= 0;
	}
	else if(com == UART_DBG)
	{
		UartDBG.rinp = 0;
		UartDBG.routp = 0;
		UartDBG.sendbuf = UartDBG.sbuf;
		UartDBG.scnt = 0;
		UartDBG.sdp = 0;
		UartDBG.ctrsta= 0;
	}
}

uint16 SendtoCom(uint8 com, char *dp, uint16 dlen)
{
	if(dlen > UARTSBLENMK)
	{
		return(0);
	}
	if(com == UART_ATC)
	{
		if((UartATC.sdp + dlen) < UARTSBLENMK)
		{
			memcpy(&(UartATC.sbuf[UartATC.sdp]), dp, dlen);
			UartATC.scnt += dlen;
			UartATC.sdp += dlen;
			return(dlen);
		}
	}
	else if(com == UART_DBG)
	{
		/*
		if((UartDBG.sdp + dlen) < UARTSBLENMK)
		{
			memcpy(&(UartDBG.sbuf[UartDBG.sdp]), dp, dlen);
			UartDBG.scnt += dlen;
			UartDBG.sdp += dlen;
			return(dlen);
		}
		*/
		fwrite(dp, 1, dlen, stdout);
        fflush(stdout);
	}
	return(0);
}

uint16 ReadfoCom(uint8 com, char *dp,  uint16 dmax)
{
	uint16 i;
	i = 0;
	if(dmax == 0) return(0);
	if(com == UART_ATC)
	{
		while(UartATC.rinp != UartATC.routp)
		{
			dp[i++] = UartATC.recvbuf[UartATC.routp++];
			UartATC.routp &= UARTRBLENMK;
			if(i == dmax)
			{
				return(i);
			}
		}
	}
	else if(com == UART_DBG)
	{
		while(UartDBG.rinp != UartDBG.routp)
		{
			dp[i++] = UartDBG.recvbuf[UartDBG.routp++];
			UartDBG.routp &= UARTRBLENMK;
			if(i == dmax)
			{
				return(i);
			}
		}
	}
	
	return(i);
}

unsigned int GetSysTickCnt(void)
{
	unsigned int   cms;
	cms = GetTickCount();
	return(cms);
}

unsigned int SaLHalGetMsCnt(unsigned int stms)
{
    unsigned int   cms;
	unsigned int   dtim;
	cms = GetTickCount();
	if(cms >= stms)
	{
		dtim = cms - stms;
	}
	else
	{
		dtim = 0x0FFFFFFFF - stms;
		dtim += cms;
	}
	return(dtim);
}

#ifdef WIN_COM_SELECT
unsigned char Win32_COM_Select(void)
{
	HDEVINFO hDevInfo;
	SP_DEVINFO_DATA DeviceInfoData;
	DWORD i;
	ComPortInfo ports[MAX_PORTS];
	int portCount = 0;
	
	hDevInfo = SetupDiGetClassDevs(
								   &GUID_DEVCLASS_PORTS,
								   0,
								   0,
								   DIGCF_PRESENT);
	
	if (hDevInfo == INVALID_HANDLE_VALUE) {
		printf("SetupDiGetClassDevs failed.\n");
		return 1;
	}
	
	DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
	
	for (i = 0; SetupDiEnumDeviceInfo(hDevInfo, i, &DeviceInfoData); i++) {
		HKEY hDeviceRegistryKey = SetupDiOpenDevRegKey(
													   hDevInfo,
													   &DeviceInfoData,
													   DICS_FLAG_GLOBAL,
													   0,
													   DIREG_DEV,
													   KEY_READ);
		
		if (hDeviceRegistryKey == INVALID_HANDLE_VALUE) {
			continue;
		}
		
		char portName[256] = {0};
		DWORD portNameLen = sizeof(portName);
		DWORD regType = 0;
		
		if (RegQueryValueExA(
							 hDeviceRegistryKey,
							 "PortName",
							 NULL,
							 &regType,
							 (LPBYTE)portName,
							 &portNameLen) == ERROR_SUCCESS) {
			if (regType == REG_SZ && portCount < MAX_PORTS) {
				strncpy(ports[portCount].portName, portName, sizeof(ports[portCount].portName) - 1);
				
				// 获取友好名称
				char friendlyName[256] = {0};
				if (SetupDiGetDeviceRegistryPropertyA(
													  hDevInfo,
													  &DeviceInfoData,
													  SPDRP_FRIENDLYNAME,
													  NULL,
													  (PBYTE)friendlyName,
													  sizeof(friendlyName),
													  NULL)) {
					strncpy(ports[portCount].friendlyName, friendlyName, sizeof(ports[portCount].friendlyName) - 1);
				} else {
					strncpy(ports[portCount].friendlyName, portName, sizeof(ports[portCount].friendlyName) - 1);
				}
				portCount++;
			}
		}
		
		RegCloseKey(hDeviceRegistryKey);
	}
	
	SetupDiDestroyDeviceInfoList(hDevInfo);
	
	if (portCount == 0) {
		printf("No COM ports found.\n");
		return 0;
	}

COM_Selection:
	printf("Available COM ports:\n");
	for (int j = 0; j < portCount; ++j) {
		printf("%d: %s - %s\n", j + 1, ports[j].portName, ports[j].friendlyName);
	}
	
	int choice = 0;
	printf("Select a COM port by number (1-%d): ", portCount);
	if (scanf("%d", &choice) != 1 || choice < 1 || choice > portCount) {
		printf("Invalid selection.\n");
		goto COM_Selection;
	}
	
	printf("You selected: %s - %s\n", ports[choice - 1].portName, ports[choice - 1].friendlyName);
	
	if (sscanf(ports[choice - 1].portName, "COM%d", &PortNumUsed) == 1) {
			printf("Will open COM%d for AT communication\n", PortNumUsed);
		} else {
			printf("Unable to extract port number\n");
		}
   
    return 0;
}

#endif

char SysInitUart(uint16 com, uint32 bps)	//in some cases the com port number is more than 255
{
	char buf[128];
	sprintf(buf, "\\\\.\\COM%u", com);
	hSerial = CreateFile(buf, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hSerial == INVALID_HANDLE_VALUE) 
	{
	    printf("Error in opening serial port\n");
	    return(RETCHAR_FALSE);
	}
	else
	{
	    printf("Opening serial port OK %p:%s\n", hSerial, buf);
	}
	
	DCB dcbSerialParams = {0};
	dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
	if (!GetCommState(hSerial, &dcbSerialParams))
	{
	    printf("Error getting serial port state\n");
	}
	else
	{
	    dcbSerialParams.BaudRate = bps;
	    dcbSerialParams.ByteSize = 8;
	    dcbSerialParams.StopBits = ONESTOPBIT;
	    dcbSerialParams.Parity = NOPARITY;
	    if (!SetCommState(hSerial, &dcbSerialParams)) 
		{
	        // ??????
	        printf("Error setting serial port state\n");
	    } 
		else 
		{
	        printf("Serial port configured successfully\n");
	    }
	}
	
	COMMTIMEOUTS timeouts = {0};
	timeouts.ReadIntervalTimeout = 1;
	timeouts.ReadTotalTimeoutConstant = 1;
	timeouts.ReadTotalTimeoutMultiplier = 1;
	timeouts.WriteTotalTimeoutConstant = 3; 
	timeouts.WriteTotalTimeoutMultiplier = 1;
	if (!SetCommTimeouts(hSerial, &timeouts)) 
	{
	    // ??????
	    printf("Error setting timeouts\n");
	}
	else 
	{
	    printf("Timeouts set successfully\n");
	}
	return(RETCHAR_TRUE);
}

void ComDrvPoll(void)
{
	char buffer[256];
	DWORD bytes_written, bytes_read;
	uint32 i;
	
	bytes_read = 0;
	bytes_written = 0;
	if(UartATC.scnt > 255)
	{
		bytes_read = 255;
		bytes_written = UartATC.scnt;
		WriteFile(hSerial, UartATC.sendbuf, 255, &bytes_written, NULL);
		UartATC.scnt -= 255;
		UartATC.sendbuf += 255;
	}
	else if(UartATC.scnt != 0)
	{
		bytes_read = UartATC.scnt;
		bytes_written = 0;
		WriteFile(hSerial, UartATC.sendbuf, UartATC.scnt, &bytes_written, NULL);
		UartATC.scnt = 0;
		UartATC.sdp = 0;
		UartATC.sendbuf = UartATC.sbuf;
	}
	
	if(bytes_written != bytes_read)
	{
		printf("Error writing to serial port:%lu != %lu\n", bytes_read, bytes_written);
	}
	
	bytes_read = 0;
	ReadFile(hSerial, buffer, sizeof(buffer), &bytes_read, NULL);
	if(bytes_read > 0)
	{
		for(i=0; i<bytes_read; i++)			
		{				
			UartATC.recvbuf[UartATC.rinp++] = buffer[i];				
			UartATC.rinp &= UARTRBLENMK;			
		}
	}
	
}

int GetSysRtcTime(char *buffer, uint16_t size)
{   
#if 0 //def _WIN32 
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    int milliseconds = 0;
    
    if (t == NULL) {
        return -1;
    }
    
    /* Get milliseconds on Windows platform */
    SYSTEMTIME st;
    GetLocalTime(&st);
    milliseconds = st.wMilliseconds;    
    
    /* Format the time string with milliseconds */
    // int len = strftime(buffer, size, "%Y-%m-%d %H:%M:%S", t);
    int len = strftime(buffer, size, "%H:%M:%S", t);
    if (len <= 0 || len >= size - 4) {
        return -1;
    }
    
    /* Append milliseconds */
    return snprintf(buffer + len, size - len, ".%03d", milliseconds);
    
#else
    unsigned int t;
    t = (GetTickCount()&0x7FFFF);
    
    return snprintf(buffer, size, "%06u", t);
#endif
}

void OutputLog(char *msg, uint16_t len)
{
    SendtoCom(DBGCH_A, msg, len);
//    printf("log[%d]:%s", len, msg);
}

#define ATC_COM		5
#define THREADAYMAX	2
#define THDTIMER	0
#define THDUART1	1
int main(void)
{
    uint8 KeepRun;
    
 	printf("Begin Running VMCU Tester. %s %s\r\n",__DATE__, __TIME__);
 	
#ifdef WIN_COM_SELECT
	Win32_COM_Select();

 	if(SysInitUart(PortNumUsed, 115200) == RETCHAR_FALSE)
 	{
 		return(0);	
 	}
#else
	if(SysInitUart(ATC_COM, 115200) == RETCHAR_FALSE)
	{
		return(0);	
	}
#endif

	/* init sam log */
//	sam_dbg_init(NULL, NULL);
	sam_dbg_init(OutputLog, GetSysRtcTime);
	sam_dbg_set_level(SAM_DBG_LEVEL_TRACE);

	InitCom(UART_ATC);
	InitCom(UART_DBG);
	TesterInit( );
	CmdBp = 0;
	KeepRun = 0xFF;
	while(KeepRun)
	{
		ComDrvPoll( );
		
		TesterProc( );
		
		if(CmdBp > 3)
		{
			if(Strsearch(KeyBuffer, "STOP") == 1)
			{
				break;
			}
			else if(Strsearch(KeyBuffer, "DBG:") == 1)
			{
				
			}
		}
		Sleep(5);
	}
    
    CloseHandle(hSerial);
    return 0;
}
