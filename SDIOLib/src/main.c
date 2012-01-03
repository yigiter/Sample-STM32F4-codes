#include "stm32f4xx.h"
#include "EvalBoard.h"
#include "UARTLib.h"
#include "SDIO.h"

uint8_t outbuf[512];
uint8_t inbuf[512];

int main(void) {
  int i;
  
  LED_LowLevel_Init();
  UART_LowLevel_Init();
  SD_LowLevel_Init();   //Initialize PINS, vector table and SDIO interface
  UART_SendLine("Ready...Steady\n");
  
  //Initialize the SD
  SD_Init();  //After return from this function sd is in transfer mode.
  UART_SendLine("GO\n");
  
  //Write a single block to the SD
  for (i=0;i<512;i++) {
    outbuf[i]='a';
  }
  
  //Single Block Test
  SD_WriteSingleBlock(outbuf, 3000);
  SD_ReadSingleBlock(inbuf, 3000);
  UART_SendLine("First Block\n");
  UART_Send(inbuf,512);
  
  SD_WriteSingleBlock(outbuf, 3001);
  SD_ReadSingleBlock(inbuf, 3001);
  UART_SendLine("Second Block\n");
  UART_Send(inbuf,512);
  
  //Multiple block
  SD_StartMultipleBlockWrite(1000);
  SD_WriteData(outbuf,500);
  SD_WriteData(outbuf+50,50);
  SD_WriteData(outbuf,500);
  SD_StopMultipleBlockWrite();
  
  
  SD_ReadSingleBlock(inbuf, 1000);
  UART_SendLine("Mult. Block 1\n");
  UART_Send(inbuf,512);
  SD_WaitTransmissionEnd();
  SD_ReadSingleBlock(inbuf, 1001);
  UART_SendLine("Mult. Block 2\n");
  UART_Send(inbuf,512);
  
  while(1) {
  };
}
