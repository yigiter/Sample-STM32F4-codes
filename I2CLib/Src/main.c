#include "stm32f4xx.h"
#include "I2CLib.h"
#include "EvalBoard.h"

#define MUXADDR  (0x70)

static uint8_t buf[10];

int main(void) {
  //Initialize the LEDS
  LED_LowLevel_Init();
  
  //Initilaze I2C Ports and Peripheral
  I2C_LowLevel_Init();
  
  
  buf[0]=0x05;
  buf[1]=0x02;
  I2C_WrBuf(MUXADDR, buf, 2);
  I2C_RdBuf(MUXADDR, &buf[5], 1);
  
  
  buf[0]=0x05;
  buf[1]=0x03;
  I2C_WrBuf(MUXADDR, buf, 2);
  I2C_RdBuf(MUXADDR, &buf[5], 1);
  if (buf[0]==buf[5]) {
    GPIO_SetBits(GPIOD, LEDUP);
  }
  
  if (buf[1]==buf[5]) {
    GPIO_SetBits(GPIOD, LEDLEFT);
  }
    
  while (1) {
  }
}
