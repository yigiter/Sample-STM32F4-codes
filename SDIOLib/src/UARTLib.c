#include "stm32f4xx.h"
#include "UARTLib.h"

#define USARTx      USART2

void UART_LowLevel_Init(void) {
  //Configuration for USART2
  //TX=GPIOA_Pin_2 (Rx of ftdi)
  //RX=GPIOA_Pin_3  (tx of ftdi)
  //RTS=GPIOA_Pin_1 (cts of ftdi)
  //CTS=GPIOD_Pin_3 (rts of ftdi)
  GPIO_InitTypeDef GPIO_InitStructure;
  USART_InitTypeDef USART_InitStructure;
  
  //Enable the GPIO pins for USART2
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);   //For tx,rx,rts 
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);   //for cts
  
  //Enable the UART2
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
  
  //Connect GPIO pins to the UART
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource1 ,GPIO_AF_USART2); //RTS
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource2 ,GPIO_AF_USART2); //TX
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource3 ,GPIO_AF_USART2); //RX
  GPIO_PinAFConfig(GPIOD, GPIO_PinSource3 ,GPIO_AF_USART2); //CTS
  
  //Configure the GPIO Pins
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
  GPIO_Init(GPIOD, &GPIO_InitStructure);
  
  //Initialize the USART
  //USART_InitStructure.USART_BaudRate = 2625000;   //MAximum rate for APB1 with 16 over-sampling
  USART_InitStructure.USART_BaudRate = 9600;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
  USART_Init(USARTx, &USART_InitStructure);
  
  //Enable (UE) the USART
  USART_Cmd(USARTx, ENABLE);

  return;
}


void UART_Send(uint8_t *buf, uint32_t cnt){
  while(cnt>1) {  
    //Write data to DR
    USARTx->DR = (*buf & (uint16_t)0x01FF);
    buf++;
    
    //Wait for DR ready
    while (!(USARTx->SR & USART_SR_TXE)) {};
    cnt--;
  }
  
  //Send the last byte
  USARTx->DR = (*buf & (uint16_t)0x01FF);
  
  //Wait for the end of actual transmission
  while (!(USARTx->SR & USART_SR_TC)) {};   //Not this will be cleared only after a read from SR and then write to DR. (Therefore after this operation, it will still be set)
}

void UART_SendLine(char *buf){
  uint32_t i=256; //Max line length
  while(i>0 & *buf!='\n') {  
    //Write data to DR
    USARTx->DR = (*buf & (uint16_t)0x01FF);
    buf++;
    
    //Wait for DR ready
    while (!(USARTx->SR & USART_SR_TXE)) {};
    i--;
  }
  
  //Send the last byte
  USARTx->DR = (*buf & (uint16_t)0x01FF);
  
  //Wait for the end of actual transmission
  while (!(USARTx->SR & USART_SR_TC)) {};   //Not this will be cleared only after a read from SR and then write to DR. (Therefore after this operation, it will still be set)
}


void UART_Receive(uint8_t *buf, uint32_t cnt) {
  while(cnt>0) {
    //Wait for new data
    while (!(USARTx->SR & USART_SR_RXNE)) {};
    
    //Read the data
    *buf++=USARTx->DR;
    
    cnt--;
  }
}
