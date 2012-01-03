#include "stm32f4xx.h"
#include "I2CLib.h"

//Internal functions
static uint32_t I2C_Start(void);
static uint32_t I2C_Addr(uint8_t DevAddr, uint8_t dir);
static uint32_t I2C_Write(uint8_t byte);
static uint32_t I2C_Read(uint8_t *pBuf);


static uint32_t WaitSR1FlagsSet (uint32_t Flags);
static uint32_t WaitLineIdle(void);


//GPIO and I2C Peripheral (I2C3 Configuration)
#define I2Cx                      I2C3  //Selected I2C peripheral
#define RCC_APB1Periph_I2Cx       RCC_APB1Periph_I2C3 //Bus where the peripheral is connected
#define RCC_AHB1Periph_GPIO_SCL   RCC_AHB1Periph_GPIOA  //Bus for GPIO Port of SCL
#define RCC_AHB1Periph_GPIO_SDA   RCC_AHB1Periph_GPIOC  //Bus for GPIO Port of SDA
#define GPIO_AF_I2Cx              GPIO_AF_I2C3    //Alternate function for GPIO pins
#define GPIO_SCL                  GPIOA
#define GPIO_SDA                  GPIOC
#define GPIO_Pin_SCL              GPIO_Pin_8
#define GPIO_Pin_SDA              GPIO_Pin_9
#define GPIO_PinSource_SCL        GPIO_PinSource8
#define GPIO_PinSource_SDA        GPIO_PinSource9


void I2C_LowLevel_Init(void) {
  GPIO_InitTypeDef  GPIO_InitStructure;
  I2C_InitTypeDef   I2C_InitStructure;
  
  //Enable the i2c
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2Cx, ENABLE);
  //Reset the Peripheral
  RCC_APB1PeriphResetCmd(RCC_APB1Periph_I2Cx, ENABLE);
  RCC_APB1PeriphResetCmd(RCC_APB1Periph_I2Cx, DISABLE);
  
  //Enable the GPIOs for the SCL/SDA Pins
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIO_SCL | RCC_AHB1Periph_GPIO_SDA, ENABLE);
  
  //Configure and initialize the GPIOs
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_SCL;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
  GPIO_Init(GPIO_SCL, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_SDA;
  GPIO_Init(GPIO_SDA, &GPIO_InitStructure);
  
  //Connect GPIO pins to peripheral
  GPIO_PinAFConfig(GPIO_SCL, GPIO_PinSource_SCL, GPIO_AF_I2Cx);
	GPIO_PinAFConfig(GPIO_SDA, GPIO_PinSource_SDA, GPIO_AF_I2Cx);
  
  //Configure and Initialize the I2C
  I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
  I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
  I2C_InitStructure.I2C_OwnAddress1 = 0x00; //We are the master. We don't need this
  I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
  I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
  I2C_InitStructure.I2C_ClockSpeed = 50000;  //400kHz (Fast Mode) (
  
  //Initialize the Peripheral
  I2C_Init(I2Cx, &I2C_InitStructure);
  // I2C Peripheral Enable
  I2C_Cmd(I2Cx, ENABLE);
  
  return; 
}


void I2C_LowLevel_DeInit(void) {
  GPIO_InitTypeDef  GPIO_InitStructure; 
  //I2C Peripheral Disable
  I2C_Cmd(I2Cx, DISABLE);
 
  //I2C DeInit (Disables clock)
  I2C_DeInit(I2Cx);
  //RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2Cx, DISABLE);
    
  //GPIO configuration
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_SCL;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIO_SCL, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_SDA;
  GPIO_Init(GPIO_SDA, &GPIO_InitStructure);
  
  return;
}

uint32_t I2C_WrData(uint8_t DevAddr, uint8_t RegAddr, uint8_t data){
  //Write a single byte data to the given register address
  
  //Generate a Start condition
  I2C_Start();
  
  //Send I2C device Address and clear ADDR
  I2C_Addr(DevAddr, I2C_Direction_Transmitter);
  (void) I2Cx->SR2;
  
  //Send Data
  I2Cx->DR = data;
  WaitSR1FlagsSet(I2C_SR1_BTF);  //wait till the data is actually written.
  
  //Generate Stop
  I2Cx->CR1 |= I2C_CR1_STOP;
  
  //Wait to be sure that line is iddle
  WaitLineIdle();
  
  return 0;
}


uint32_t I2C_RdData(uint8_t DevAddr, uint8_t RegAddr, uint8_t *buf, uint32_t cnt) {
  //Reads "cnt" number of data starting from RegAddr
  
  //Send the Register Addres
  I2C_Start();
  I2C_Addr(DevAddr, I2C_Direction_Transmitter);
  (void) I2Cx->SR2;
  I2Cx->DR = RegAddr;
  WaitSR1FlagsSet(I2C_SR1_BTF);
  
  //Start Reading
  I2C_RdBuf(DevAddr, buf, cnt);
  
  return 0;
}


uint32_t I2C_WrBuf(uint8_t DevAddr, uint8_t *buf, uint32_t cnt) {
  
  //Generate a Start condition
  I2C_Start();
  
  //Send I2C device Address
  I2C_Addr(DevAddr, I2C_Direction_Transmitter);
  //Unstretch the clock by just reading SR2 (Physically the clock is continued to be strectehed because we have not written anything to the DR yet.)
  (void) I2Cx->SR2; 
  
  //Start Writing Data
  while (cnt--) {
    I2C_Write(*buf++);
  }
  
  //Wait for the data on the shift register to be transmitted completely
  WaitSR1FlagsSet(I2C_SR1_BTF);
  //Here TXE=BTF=1. Therefore the clock stretches again.
  
  //Order a stop condition at the end of the current tranmission (or if the clock is being streched, generate stop immediatelly)
  I2Cx->CR1 |= I2C_CR1_STOP;
  //Stop condition resets the TXE and BTF automatically.
  
  //Wait to be sure that line is iddle
  WaitLineIdle();
  
  return 0;
}


uint32_t I2C_RdBufEasy (uint8_t DevAddr, uint8_t *buf, uint32_t cnt) {
  //The easy read.
  //We assume that we will reset ACK and order a stop condition while the last byte is being received by the shift register.
  //If this can't be done on time (during last byte reception), the slave will continue to send at least 1 more byte than cnt.
  //In most cases, such a condition does not hurt at all. Therefore people uses this method exclusively.
  
  //Note that it is impossible to guarantee the timig requirement only for single byte reception.
  //For N>=2, the timing is almost always satisfied. (if there is no interrupt, it will definetely be satisfied)
  
  //Generate Start
  I2C_Start();
  
  //Send I2C Device Address and clear ADDR
  I2C_Addr(DevAddr, I2C_Direction_Receiver);
  (void)I2Cx->SR2;
  
  while ((cnt--)>1) {
    I2C_Read(buf++);
  }
  
  //At this point we assume last byte is being received by the shift register. (reception has not been completed yet)
  //Reset ack
  I2Cx->CR1 &= (uint16_t)~((uint16_t)I2C_CR1_ACK);
  
  //Order a stop condition
  I2Cx->CR1 |= I2C_CR1_STOP;
  
  //Now read the final byte
  I2C_Read(buf);
  
  //Make Sure Stop bit is cleared and Line is now Iddle
  WaitLineIdle();
    
  //Enable the Acknowledgement
  I2Cx->CR1 |= ((uint16_t)I2C_CR1_ACK);
  
  return 0;
}


uint32_t I2C_RdBuf (uint8_t DevAddr, uint8_t *buf, uint32_t cnt) {
  //Generate Start
  I2C_Start();
  
  //Send I2C Device Address
  I2C_Addr(DevAddr, I2C_Direction_Receiver);
  
  if (cnt==1) {//We are going to read only 1 byte
    //Before Clearing Addr bit by reading SR2, we have to cancel ack.
    I2Cx->CR1 &= (uint16_t)~((uint16_t)I2C_CR1_ACK);
    
    //Now Read the SR2 to clear ADDR
    (void)I2Cx->SR2;
    
    //Order a STOP condition
    //Note: Spec_p583 says this should be done just after clearing ADDR
    //If it is done before ADDR is set, a STOP is generated immediately as the clock is being streched
    I2Cx->CR1 |= I2C_CR1_STOP;
    //Be carefull that till the stop condition is actually transmitted the clock will stay active even if a NACK is generated after the next received byte.
    
    //Read the next byte
    I2C_Read(buf);
    
    //Make Sure Stop bit is cleared and Line is now Iddle
    WaitLineIdle();
    
    //Enable the Acknowledgement again
    I2Cx->CR1 |= ((uint16_t)I2C_CR1_ACK);
  }
  
  else if (cnt==2) {  //We are going to read 2 bytes (See: Spec_p584)
    //Before Clearing Addr, reset ACK, set POS
    I2Cx->CR1 &= (uint16_t)~((uint16_t)I2C_CR1_ACK);
    I2Cx->CR1 |= I2C_CR1_POS;
    
    //Read the SR2 to clear ADDR
    (void)I2Cx->SR2;
    
    //Wait for the next 2 bytes to be received (1st in the DR, 2nd in the shift register)
    WaitSR1FlagsSet(I2C_SR1_BTF);
    //As we don't read anything from the DR, the clock is now being strecthed.
    
    //Order a stop condition (as the clock is being strecthed, the stop condition is generated immediately)
    I2Cx->CR1 |= I2C_CR1_STOP;
    
    //Read the next two bytes
    I2C_Read(buf++);
    I2C_Read(buf);
    
    //Make Sure Stop bit is cleared and Line is now Iddle
    WaitLineIdle();
    
    //Enable the ack and reset Pos
    I2Cx->CR1 |= ((uint16_t)I2C_CR1_ACK);
    I2Cx->CR1 &= (uint16_t)~((uint16_t)I2C_CR1_POS);
  }
  else { //We have more than 2 bytes. See spec_p585
    //Read the SR2 to clear ADDR
    (void)I2Cx->SR2;
     
    while((cnt--)>3) {//Read till the last 3 bytes
      I2C_Read(buf++);
    }
    
    //3 more bytes to read. Wait till the next to is actually received
    WaitSR1FlagsSet(I2C_SR1_BTF);
    //Here the clock is strecthed. One more to read.
    
    //Reset Ack
    I2Cx->CR1 &= (uint16_t)~((uint16_t)I2C_CR1_ACK);
    
    //Read N-2
    I2C_Read(buf++);
    //Once we read this, N is going to be read to the shift register and NACK is generated
    
    //Wait for the BTF
    WaitSR1FlagsSet(I2C_SR1_BTF); //N-1 is in DR, N is in shift register
    //Here the clock is stretched
    
    //Generate a stop condition
    I2Cx->CR1 |= I2C_CR1_STOP;
    
    //Read the last two bytes (N-1 and N)
    //Read the next two bytes
    I2C_Read(buf++);
    I2C_Read(buf);
    
    //Make Sure Stop bit is cleared and Line is now Iddle
    WaitLineIdle();
    
    //Enable the ack
    I2Cx->CR1 |= ((uint16_t)I2C_CR1_ACK);
  }
  
  return 0;
}

///////////////PRIVATE FUNCTIONS/////////////////////
static uint32_t I2C_Read(uint8_t *pBuf) {
    uint32_t err;
    
    //Wait till new data is ready to be read
    err=WaitSR1FlagsSet(I2C_SR1_RXNE);
        
    if (!err) {
      *pBuf = I2Cx->DR;   //This clears the RXNE bit. IF both RXNE and BTF is set, the clock stretches
      return 0;
    }
    else {return err;}
    
    
    /*
    //Wait for Ev7 (Ev7 cleared by reading DR. If not cleared strecthes the clock)
    err=WaitEvent(I2C_EVENT_MASTER_BYTE_RECEIVED);
    if (!err) {
      *pBuf = I2C_ReceiveData(I2Cx);
    }
    else {
      return err;
    }
    */
}


static uint32_t I2C_Write(uint8_t byte){
  
  //Write the byte to the DR
  I2Cx->DR = byte;
  
  //Wait till the content of DR is transferred to the shift Register.
  return WaitSR1FlagsSet(I2C_SR1_TXE);
  //At this point point DR is available for the next byte even if it is not actually transmitted from the shift register
  //TXE will be reset automatically when DR is written again (TXE does not strecth the clock unless BTF is also set)
  
   
  /*
  //Send Data
  I2C_SendData(I2Cx, byte);
  
  //Check Ev8 (Cleared with another DR write)
  return WaitEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTING);
  */
}

static uint32_t I2C_Addr(uint8_t DevAddr, uint8_t dir) {
  
  //Write address to the DR (to the bus)
  I2Cx->DR = (DevAddr << 1) | dir;
  
  //Wait till ADDR is set (ADDR is set when the slave sends ACK to the address).
  //Clock streches till ADDR is Reset. To reset the hardware i)Read the SR1 ii)Wait till ADDR is Set iii)Read SR2
  //Note1:Spec_p602 recommends the waiting operation
  //Note2:We don't read SR2 here. Therefore the clock is going to be streched even after return from this function
  return WaitSR1FlagsSet(I2C_SR1_ADDR); 
  
  /*
  //Send DevAddr
  I2C_Send7bitAddress(I2Cx, DevAddr, dir);  //This Clears EV5 (SR1 was read before).

  //Check EV6 and clear it (Reading SR1 and SR2 clears it) (Strectes the clock till cleared)
  if (dir==I2C_Direction_Transmitter) {
    return WaitEvent(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED);
  }
  else {  //I2C_Direction_Receiver
    return WaitEvent(I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED);
  }
  */
}


static uint32_t I2C_Start(void) {
  
  //Generate a start condition. (As soon as the line becomes idle, a Start condition will be generated)
  I2Cx->CR1 |= I2C_CR1_START;
  
  //When start condition is generated SB is set and clock is stretched.
  //To activate the clock again i)read SR1 ii)write something to DR (e.g. address)
  return WaitSR1FlagsSet(I2C_SR1_SB);  //Wait till SB is set

/*  
  //Generate Start Condition
	I2C_GenerateSTART(I2Cx, ENABLE);
  
  //Check EV5 (SB bit will be cleared after SR1 read and DR write.)
	return WaitEvent(I2C_EVENT_MASTER_MODE_SELECT);
  //Note: Ev5 Strectes the CLK till it is cleared.
*/  
}


static uint32_t WaitSR1FlagsSet (uint32_t Flags) {
  //Wait till the specified SR1 Bits are set
  //More than 1 Flag can be "or"ed. This routine reads only SR1.
  uint32_t TimeOut = HSI_VALUE;
  
  while(((I2Cx->SR1) & Flags) != Flags) {
    if (!(TimeOut--)) {
      panic(Flags, "I2C Error\n");
      return 1;
    }
  } 
  return 0;
}


static uint32_t WaitLineIdle(void) {
  //Wait till the Line becomes idle.
  
  uint32_t TimeOut = HSI_VALUE;
  //Check to see if the Line is busy
  //This bit is set automatically when a start condition is broadcasted on the line (even from another master)
  //and is reset when stop condition is detected.
  while((I2Cx->SR2) & (I2C_SR2_BUSY)) {
    if (!(TimeOut--)) {
      panic(0, "I2C Error\n");
      return 1;
    }
  }
  
  /*
  //Additonal check (not really necessary): Check to see if the mode is slave
  while((I2Cx->SR2) & (I2C_SR2_MSL)) {
    if (!(TimeOut--)) {
      panic(0, "I2C Error\n");
      return 1;
    }
  }
  */
  
  return 0;
}



/*
static uint32_t I2C_Stop(void) {

  I2C_GenerateSTOP(I2Cx, ENABLE);
  return I2C_LineIdle();  //I2C_SR2_BUSY is Reset after a stop condition is detected on the line (I assume, we are the only master on the line)
}

static uint32_t WaitEvent(uint32_t Event) {
  //Wait till all the specified SR Bits are set
  uint32_t TimeOut=HSI_VALUE;
  while(!I2C_CheckEvent(I2Cx, Event)) {
    	if((TimeOut--) == 0) {panic(Event, "I2C Error\n"); return 1;}
  }
  return 0;
}

static uint32_t WaitFlag(uint32_t Flag, FlagStatus Status) {
  //Wait till the specified SR Bit (only 1 bit at a time: No "or")
  //Note: I2C_GetFlagStatus only Checks one the Status Registers (i.e. will not clear EV6)
  uint32_t TimeOut = HSI_VALUE;
  while(I2C_GetFlagStatus(I2Cx, Flag)!=Status)
  {
    if((TimeOut--) == 0) {panic(Flag, "I2C Error\n"); return 1;}
  }
  
  return 0;
}
*/
