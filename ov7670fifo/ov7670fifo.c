#include "ov7670fifo.h"
#include "ov7670regs.h"

static ov7670_Regval ov7670_regs[] = {
        {REG_COM3, 0x01},/////TEST COLOR BAR!!!!!!!!!!!!!!!!!!!!!!!!!
        {REG_CLKRC, 0x80},/*default*/
        {REG_COM11, 0x0A},
        {REG_TSLB, 0x04},
        {REG_TSLB, 0x04},
        {REG_COM7, 0x04}, /* output format: rgb */

        {REG_RGB444, 0x00}, /* disable RGB444 */
        {REG_COM15, 0xD0}, /* set RGB565 */

        /* not even sure what all these do, gonna check the oscilloscope and go
         * from there... */
        {REG_HSTART, 0x16},
        {REG_HSTOP, 0x04},
        {REG_HREF, 0x24},
        {REG_VSTART, 0x02},
        {REG_VSTOP, 0x7a},
        {REG_VREF, 0x0a},
        {REG_COM10, 0x02},
        {REG_COM3, 0x04},
        {REG_MVFP, 0x3f},

        /* 160x120, i think */
        //{REG_COM14, 0x1a}, // divide by 4
        //{0x72, 0x22}, // downsample by 4
        //{0x73, 0xf2}, // divide by 4

        /* 320x240: */
        {REG_COM14, 0x19},
        {0x72, 0x11},
        {0x73, 0xf1},

        // test pattern
        //{0x70, 0xf0},
        //{0x71, 0xf0},

        // COLOR SETTING
        {0x4f, 0x80},
        {0x50, 0x80},
        {0x51, 0x00},
        {0x52, 0x22},
        {0x53, 0x5e},
        {0x54, 0x80},
        {0x56, 0x40},
        {0x58, 0x9e},
        {0x59, 0x88},
        {0x5a, 0x88},
        {0x5b, 0x44},
        {0x5c, 0x67},
        {0x5d, 0x49},
        {0x5e, 0x0e},
        {0x69, 0x00},
        {0x6a, 0x40},
        {0x6b, 0x0a},
        {0x6c, 0x0a},
        {0x6d, 0x55},
        {0x6e, 0x11},
        {0x6f, 0x9f},

        {0xb0, 0x84}
};

static void gpioInit(void) {
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOC, ENABLE);

    GPIO_InitTypeDef gpioInitStruct;

    /*Inputs*/
    gpioInitStruct.GPIO_Pin = OV7670_VSYNC;
    gpioInitStruct.GPIO_Mode = GPIO_Mode_IN;
    gpioInitStruct.GPIO_OType = GPIO_OType_PP;
    gpioInitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
    gpioInitStruct.GPIO_Speed = GPIO_Low_Speed;
    GPIO_Init(GPIOC, &gpioInitStruct);
    gpioInitStruct.GPIO_Pin = FIFO_D0_D7;
    GPIO_Init(GPIOB, &gpioInitStruct);

    /*NVIC_InitTypeDef nvicInitStruct = {
            .NVIC_IRQChannel = EXTI15_10_IRQn,
            .NVIC_IRQChannelPreemptionPriority = 0,
            .NVIC_IRQChannelSubPriority = 0,
            .NVIC_IRQChannelCmd = ENABLE
    };
    NVIC_Init(&nvicInitStruct);


    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOC, EXTI_PinSource14);

    EXTI_InitTypeDef extiInitStruct = {
            .EXTI_Line = EXTI_Line14,
            .EXTI_LineCmd = ENABLE,
            .EXTI_Mode = EXTI_Mode_Interrupt,
            .EXTI_Trigger = EXTI_Trigger_Rising_Falling
    };
    EXTI_Init(&extiInitStruct);*/

    /*Outputs*/
    gpioInitStruct.GPIO_Pin = GPIO_Pin_13;
    gpioInitStruct.GPIO_Mode = GPIO_Mode_OUT;
    gpioInitStruct.GPIO_OType = GPIO_OType_PP;
    gpioInitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
    gpioInitStruct.GPIO_Speed = GPIO_High_Speed;
    GPIO_Init(GPIOC, &gpioInitStruct);
    gpioInitStruct.GPIO_Pin = OV7670_RST | FIFO_WR | FIFO_RRST | FIFO_RCK | FIFO_WRST;
    GPIO_Init(GPIOB, &gpioInitStruct);

    GPIO_WriteBit(GPIOC, GPIO_Pin_13, Bit_SET);
    GPIO_WriteBit(GPIOB, FIFO_WR, Bit_RESET);
    GPIO_WriteBit(GPIOB, OV7670_RST, Bit_RESET);
    GPIO_WriteBit(GPIOB,  FIFO_RRST, Bit_SET);
    GPIO_WriteBit(GPIOB,  FIFO_WRST, Bit_SET);
    GPIO_WriteBit(GPIOC, FIFO_RCK, Bit_RESET);

}

/*volatile uint8_t GetFrame = 0;
void EXTI15_10_IRQHandler(void) {
    if(EXTI_GetITStatus(EXTI_Line0)) {
        EXTI_ClearITPendingBit(EXTI_Line0);
        GPIO_ToggleBits(GPIOC, GPIO_Pin_13);
        if (GPIO_ReadInputDataBit(GPIOC, OV7670_VSYNC)) { //Rising edge
            if (GetFrame) {
                GetFrame = 0;
                GPIO_ResetBits(GPIOB, FIFO_WRST | FIFO_WR);
            }
        } else { //Falling edge
            if (!GetFrame){
                GPIO_SetBits(GPIOB, FIFO_WRST | FIFO_WR);
            }
        }
    }
}*/

static void i2cInit(void) {
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

    GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_I2C1);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_I2C1);
    GPIO_InitTypeDef  gpioInitStructure = {
            .GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7,
            .GPIO_Mode = GPIO_Mode_AF,
            .GPIO_Speed = GPIO_Speed_50MHz,
            .GPIO_OType = GPIO_OType_OD,
            .GPIO_PuPd = GPIO_PuPd_NOPULL
    };
    GPIO_Init(GPIOB, &gpioInitStructure);

    I2C_InitTypeDef  i2cInitStructure = {
            /*.I2C_ClockSpeed = 100000,
            .I2C_Mode = I2C_Mode_I2C,
            .I2C_DutyCycle = I2C_DutyCycle_2,
            .I2C_OwnAddress1 = 0x01,
            .I2C_Ack = I2C_Ack_Enable,
            .I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_10bit*/
    };
    I2C_StructInit(&i2cInitStructure);
    I2C_Init(I2C1, &i2cInitStructure);
    I2C_Cmd(I2C1, ENABLE);
}

static void i2cStart(void) {
    while (I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY));
    I2C_GenerateSTART(I2C1, ENABLE);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));
}

static void i2cSendAddress(uint8_t address, uint8_t direction) {
    I2C_Send7bitAddress(I2C1, address, direction);

    if (direction == I2C_Direction_Transmitter) {
        while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));
    } else if (direction == I2C_Direction_Receiver) {
        while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));
    }
}

static void i2cTransmitByte(uint8_t data) {
    I2C_SendData(I2C1, data);
    while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
}

static uint8_t i2cReceiveByte(void) {
    while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED));
    return I2C_ReceiveData(I2C1);
}

static void i2cStop(void) {
    I2C_GenerateSTOP(I2C1,ENABLE);
    while (I2C_GetFlagStatus(I2C1, I2C_FLAG_STOPF));
}


void ov7670_writeRegister(uint8_t reg, uint8_t value) {
    i2cStart();
    i2cSendAddress(0x42, I2C_Direction_Transmitter);
    i2cTransmitByte(reg);
    i2cTransmitByte(value);
    i2cStop();
}

uint8_t ov7670_readRegister(uint8_t reg) {
    uint8_t value = 0;
    i2cStart();
    i2cSendAddress(0x42, I2C_Direction_Transmitter);
    i2cTransmitByte(reg);
    i2cStop();

    i2cStart();
    i2cSendAddress(0x43, I2C_Direction_Receiver);
    value = i2cReceiveByte();
    i2cStop();

    return value;
}

static void sigalizeError(void) {
    volatile const uint64_t RUN = 1;
    while (RUN) {
        GPIO_ToggleBits(GPIOC, GPIO_Pin_13);
        for (int i = 0; i < 1000000; ++i);
    }
}

void ov7670_writeArrayRegval(const ov7670_Regval * regval, uint32_t size) {
    for (int i = 0; i < size; ++i) {
        ov7670_writeRegister(regval[i].reg_num, regval[i].value);
        uint8_t data = ov7670_readRegister(regval[i].reg_num);

        if ((data & regval[i].value) != regval[i].value) {
            sigalizeError();
        }
    }
}

static uint8_t ov7670_checkSensor(void) {
    uint8_t productIdMSB = ov7670_readRegister(0x0a);
    uint8_t productIdLSB = ov7670_readRegister(0x0b);
    uint8_t manufacturerIdMSB = ov7670_readRegister(0x1c);
    uint8_t manufacturerIdLSB = ov7670_readRegister(0x1d);

    if (productIdMSB == 0x76 && productIdLSB == 0x73 && manufacturerIdMSB == 0x7f && manufacturerIdLSB == 0xa2) {
        return 1;
    }
    return 0;
}

static void ov7670_reset(void) {
    GPIO_WriteBit(GPIOB, OV7670_RST, Bit_RESET);
    for (int i = 0; i < 1000; ++i);
    GPIO_WriteBit(GPIOB, OV7670_RST, Bit_SET);
    for (int i = 0; i < 1000; ++i);
}

void ov7670_init(void) {
    i2cInit();
    gpioInit();

    ov7670_reset();
    if (ov7670_checkSensor() == 0) {
        sigalizeError();
    }
    ov7670_writeRegister(REG_COM7, 0x80);
    for (int i = 0; i < 16 * 500; ++i);
    ov7670_writeArrayRegval(ov7670_regs, sizeof(ov7670_regs) / sizeof(ov7670_Regval));
}

void fifo_captureFrame(void) {
    while (GPIO_ReadInputDataBit(GPIOC, OV7670_VSYNC));
    while (!GPIO_ReadInputDataBit(GPIOC, OV7670_VSYNC));
    GPIO_WriteBit(GPIOB, FIFO_WR, Bit_SET);
    while (GPIO_ReadInputDataBit(GPIOC, OV7670_VSYNC));
    GPIO_WriteBit(GPIOB, FIFO_WR, Bit_RESET);
    for (int i = 0; i < 1000000; ++i);
}



void fifo_readReset(void) {
    GPIO_WriteBit(GPIOB, FIFO_RRST, Bit_RESET);
    GPIO_WriteBit(GPIOB, FIFO_RCK, Bit_SET);
    GPIO_WriteBit(GPIOB, FIFO_RCK, Bit_RESET);
    GPIO_WriteBit(GPIOB, FIFO_RCK, Bit_SET);
    for (int i = 0; i < 100; ++i);
    GPIO_WriteBit(GPIOB, FIFO_RRST, Bit_SET);
    for (int i = 0; i < 100; ++i);
    GPIO_WriteBit(GPIOB, FIFO_RCK, Bit_RESET);

    }

uint8_t fifo_readByte(void) {
    uint8_t  data;
    GPIO_WriteBit(GPIOB, FIFO_RCK, Bit_SET);
    data =  ((GPIOB->IDR & (GPIO_Pin_8 | GPIO_Pin_9)) >> 2) | (GPIOB->IDR & (GPIO_Pin_5 | GPIO_Pin_4 |
                            GPIO_Pin_3 | GPIO_Pin_2| GPIO_Pin_1 | GPIO_Pin_0));
    GPIO_WriteBit(GPIOB, FIFO_RCK, Bit_RESET);
    return data;
}

