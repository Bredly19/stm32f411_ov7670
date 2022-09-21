#ifndef OV7670_FIFO_H
#define OV7670_FIFO_H
#include "stm32f4xx.h"

#define OV7670_VSYNC GPIO_Pin_14
#define FIFO_D0_D7  GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | \
                    GPIO_Pin_8 | GPIO_Pin_9
#define FIFO_RCK    GPIO_Pin_10
#define OV7670_RST  GPIO_Pin_12
#define FIFO_WR     GPIO_Pin_13
#define FIFO_RRST   GPIO_Pin_14
#define FIFO_WRST   GPIO_Pin_15


typedef struct {
    uint8_t reg_num;
    uint8_t value;
} ov7670_Regval;

void ov7670_init(void);
void fifo_captureFrame(void);
void fifo_readReset(void);
uint8_t fifo_readByte(void);
uint16_t fifo_countHREF(void);
#endif //OV7670_FIFO_H
