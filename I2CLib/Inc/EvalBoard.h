#ifndef __EVALLEDS_H
#define __EVALLEDS_H

#include "stdint.h"
#define LEDUP   GPIO_Pin_13
#define LEDDOWN   GPIO_Pin_15
#define LEDRIGHT   GPIO_Pin_14
#define LEDLEFT   GPIO_Pin_12

#ifdef __cplusplus
 extern "C" {
#endif

void panic(uint32_t code, char *message);
void LED_LowLevel_Init(void);


#ifdef __cplusplus
}
#endif
#endif //__EVALLEDS_H
