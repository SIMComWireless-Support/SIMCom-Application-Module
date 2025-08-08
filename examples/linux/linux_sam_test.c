#include "serial_port.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>
#include <termios.h>
#include "../../SAM_ATCDRV/include.h"

// Global serial port structure
serial_port_t port;

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void msleep(unsigned int milliseconds) {
    struct timespec ts;
    ts.tv_sec = milliseconds / 1000;
    ts.tv_nsec = (milliseconds % 1000) * 1000000;
    nanosleep(&ts, NULL);
}

unsigned int GetSysTickCnt()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

unsigned short SendtoCom(unsigned char com, char *dp, unsigned short dlen)
{
	if(com == ATCCH_A)
	{
		serial_write(&port , (const uint8_t *)dp,(uint32_t)dlen);
	}
	else if(com == DBGCH_A)
	{
		printf("%s",dp);
	}

	return 0;
}

unsigned short ReadfoCom(unsigned char com, char *dp,  unsigned short dmax)
{
	unsigned short len=0;
    (void)com; // Unused parameter, can be used for different commands in the future
	len= serial_read(&port,(uint8_t *)dp , (uint32_t)dmax);
	return len;
}

/* USER CODE END 0 */

int main(int argc, char *argv[]) {
    
    serial_config_t config = {
        .baudrate = 115200,
        .parity = 'N',
        .data_bits = 8,
        .stop_bits = 1,
        .flow_control = false
    };

    char *device = NULL;
    int opt;

    while ((opt = getopt(argc, argv, "D:")) != -1) {
        switch (opt) {
            case 'D':
                device = optarg;
                break;
            default:
                fprintf(stderr, "Usage: %s -D /dev/ttyXXX\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }
    
    if (device == NULL) {
        fprintf(stderr, "No device specified. Use -D option.\n");
        exit(EXIT_FAILURE);
    }

    fprintf(stderr, "Device: %s\n", device);

    // Initialize serial port
    if (!serial_init(&port, device, &config)) {
        fprintf(stderr, "Failed to initialize serial port\n");
        return 1;
    }
    
	TesterInit();
    while (1) {
        TesterProc();
        msleep(1); // 1ms delay
    }
    
    // Clean up
    serial_close(&port);
    return 0;
}