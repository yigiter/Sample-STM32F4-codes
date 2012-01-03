#include "stm32f4xx.h"
#include "EvalBoard.h"


void LED_LowLevel_Init(void) {
  GPIO_InitTypeDef  GPIO_InitStructure;
  
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

  /* Configure the GPIO_LED pin */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13| GPIO_Pin_14 | GPIO_Pin_15;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOD, &GPIO_InitStructure);
}


void panic(uint32_t code, char *message) {
  uint32_t i=0;
  RCC_ClocksTypeDef RCC_ClockFreq;
  
  RCC_GetClocksFreq(&RCC_ClockFreq);
  while (1) {
    GPIO_SetBits(GPIOD, LEDUP | LEDDOWN);
    GPIO_ResetBits(GPIOD, LEDRIGHT | LEDLEFT);
    i=RCC_ClockFreq.SYSCLK_Frequency / 4;
    while(i--){}
    GPIO_ResetBits(GPIOD, LEDUP | LEDDOWN);
    GPIO_SetBits(GPIOD, LEDRIGHT | LEDLEFT);
    i=RCC_ClockFreq.SYSCLK_Frequency / 4;
    while(i--){}
  }
}
