#ifndef __I2CLIB_H
#define __I2CLIB_H

#include "stdint.h"
#include "EvalBoard.h"

#ifdef __cplusplus
 extern "C" {
#endif




void I2C_LowLevel_Init(void);
void I2C_LowLevel_DeInit(void);
uint32_t I2C_WrBuf(uint8_t DevAddr, uint8_t *buf, uint32_t cnt);
uint32_t I2C_RdBuf(uint8_t DevAddr, uint8_t *buf, uint32_t cnt);
uint32_t I2C_RdBufEasy(uint8_t DevAddr, uint8_t *buf, uint32_t cnt);

//Write a single byte to the given register address
uint32_t I2C_WrData(uint8_t DevAddr, uint8_t RegAddr, uint8_t data);

//Reads "cnt" number of data starting from RegAddr
uint32_t I2C_RdData(uint8_t DevAddr, uint8_t RegAddr, uint8_t *buf, uint32_t cnt);


#ifdef __cplusplus
}
#endif
#endif //__I2CLIB_H
