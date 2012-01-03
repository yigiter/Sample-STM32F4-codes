#ifndef __UARTLIB_H
#define __UARTLIB_H

#include "stdint.h"

#ifdef __cplusplus
 extern "C" {
#endif


void UART_LowLevel_Init(void);
void UART_Send(uint8_t *buf, uint32_t cnt);
void UART_Receive(uint8_t *buf, uint32_t cnt);
void UART_SendLine(char *buf);

#ifdef __cplusplus
}
#endif
#endif //__UARTLIB_H
