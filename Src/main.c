#include "stm32f4xx.h"
#include "ov7670fifo.h"

volatile uint8_t buffer[] = "buffer0";
volatile uint8_t buffer1[] = "buffer1";
volatile uint8_t  readEnable = 0;

void usartSetup(void);
void nvicSetup(void);
void dmaSetup(void);

int main(void) {
    volatile const uint64_t RUN = 1;
    usartSetup();
    nvicSetup();
    ov7670_init();
    //GPIO_WriteBit(GPIOC, GPIO_Pin_13, Bit_RESET);
    while (RUN) {
        if (readEnable) {
            fifo_captureFrame();
            fifo_readReset();
            for (int i = 0; i < 320 * 240 * 2; ++i) {
                while (!USART_GetFlagStatus(USART1, USART_FLAG_TC));
                USART_SendData(USART1, fifo_readByte());
            }
            readEnable = 0;
        }
    }
}

void usartSetup(void) {
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

    GPIO_InitTypeDef  gpioInitStruct = {
            .GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10,
            .GPIO_Speed = GPIO_Speed_50MHz,
            .GPIO_PuPd = GPIO_PuPd_NOPULL,
            .GPIO_Mode = GPIO_Mode_AF
    };
    GPIO_Init(GPIOA, &gpioInitStruct);

    GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_USART1);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_USART1);

    USART_InitTypeDef usartInitStruct = {
            .USART_BaudRate = 115200,
            .USART_WordLength = USART_WordLength_8b,
            .USART_StopBits = USART_StopBits_1,
            .USART_Parity = USART_Parity_No,
            .USART_Mode = USART_Mode_Tx | USART_Mode_Rx,
            .USART_HardwareFlowControl = USART_HardwareFlowControl_None,
    };

    USART_Init(USART1, &usartInitStruct);
    USART_Cmd(USART1, ENABLE);
    //USART_DMACmd(USART1, USART_DMAReq_Tx, ENABLE);
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
}

void nvicSetup(void) {
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);

    NVIC_InitTypeDef nvicInitStructure = {
            .NVIC_IRQChannel = USART1_IRQn,
            .NVIC_IRQChannelPreemptionPriority = 1,
            .NVIC_IRQChannelSubPriority = 3,
            .NVIC_IRQChannelCmd = ENABLE
    };

    NVIC_Init(&nvicInitStructure);
}

void dmaSetup(void) {
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);

    DMA_Cmd(DMA2_Stream7, DISABLE);
    while (DMA_GetCmdStatus(DMA2_Stream7) == ENABLE);

    DMA_InitTypeDef dmaInitStruct = {
            .DMA_Channel = DMA_Channel_4,
            .DMA_PeripheralBaseAddr = (uint32_t) &(USART1->DR),
            .DMA_Memory0BaseAddr=  (uint32_t) buffer,
            .DMA_DIR = DMA_DIR_MemoryToPeripheral,
            .DMA_BufferSize = sizeof(buffer) - 1,
            .DMA_PeripheralInc = DMA_PeripheralInc_Disable,
            .DMA_MemoryInc = DMA_MemoryInc_Enable,
            .DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte,
            .DMA_MemoryDataSize = DMA_MemoryDataSize_Byte,
            .DMA_Mode = DMA_Mode_Circular,
            .DMA_Priority = DMA_Priority_High,
            .DMA_FIFOMode = DMA_FIFOMode_Enable,
            .DMA_FIFOThreshold = DMA_FIFOThreshold_1QuarterFull,
            .DMA_MemoryBurst = DMA_MemoryBurst_Single,
            .DMA_PeripheralBurst = DMA_PeripheralBurst_Single
    };

    DMA_Init(DMA2_Stream7, &dmaInitStruct);
    DMA_DoubleBufferModeConfig(DMA2_Stream7, (uint32_t) buffer1, DMA_Memory_0);
    DMA_DoubleBufferModeCmd(DMA2_Stream7, ENABLE);
    DMA_Cmd(DMA2_Stream7, ENABLE);
}

void USART1_IRQHandler(void) {
    if ( USART_GetITStatus(USART1, USART_IT_RXNE) ) {
        USART_ClearITPendingBit(USART1, USART_IT_RXNE);
        if (USART_ReceiveData(USART1) == 85) {
            readEnable = 1;
        }
    }
}

void _exit(int status) {
    while (1);
}
