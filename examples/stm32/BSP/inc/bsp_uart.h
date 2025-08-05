#ifndef  __BSP_UART_H__
#define  __BSP_UART_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

void    bsp_uart_init(void);
void    bsp_usart_write(const char* str , uint32_t len);
uint32_t  bsp_usart_read(const char* str,uint32_t len);
#ifdef __cplusplus
}
#endif

#endif
