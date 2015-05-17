#include <stm32f10x.h>
#include <stm32f10x_rcc.h>
#include <stm32f10x_gpio.h>
#include <stm32f10x_usart.h>
#include <stm32f10x_dma.h>
#include "uartdma.h"

uint8_t USART_TxBuffer[USART_BUFFER_SIZE];


int uart_init(USART_TypeDef* USARTx, uint32_t baud, uint32_t flags) {
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);  // USART use DMA1 -> RM0008 Table 78

	if (USARTx == USART1) {

		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

		GPIO_StructInit(&GPIO_InitStructure);
		if (flags & USART_Mode_Tx) {
			// UART1_Tx Pin PA9
			GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
			GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
			GPIO_Init(GPIOA, &GPIO_InitStructure);
		}
		if (flags & USART_Mode_Rx) {
			// UART1_Rx Pin PA10
			GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
			GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
			GPIO_Init(GPIOA, &GPIO_InitStructure);
		}
	} else 	if (USARTx == USART2) {
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

		GPIO_StructInit(&GPIO_InitStructure);
		if (flags & USART_Mode_Tx) {
			// UART1_Tx Pin PA2
			GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
			GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
			GPIO_Init(GPIOA, &GPIO_InitStructure);
		}
		if (flags & USART_Mode_Rx) {
			// UART1_Rx Pin PA3
			GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
			GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
			GPIO_Init(GPIOA, &GPIO_InitStructure);
		}
	} else 	if (USARTx == USART3) {
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);

		GPIO_StructInit(&GPIO_InitStructure);
		if (flags & USART_Mode_Tx) {
			// UART1_Tx Pin PB10
			GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
			GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
			GPIO_Init(GPIOB, &GPIO_InitStructure);
		}
		if (flags & USART_Mode_Rx) {
			// UART1_Rx Pin PB11
			GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
			GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
			GPIO_Init(GPIOB, &GPIO_InitStructure);
		}
	}

	USART_InitTypeDef USART_InitStructure;
	USART_ClockInitTypeDef USART_ClockInitStructure;

	USART_StructInit(&USART_InitStructure);
	USART_ClockStructInit(&USART_ClockInitStructure);
	USART_ClockInit(USARTx, &USART_ClockInitStructure);
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_BaudRate = baud;
	USART_InitStructure.USART_Mode = flags;
	USART_Init(USARTx, &USART_InitStructure);

	USART_Cmd(USARTx, ENABLE);
}

int uart_close(USART_TypeDef* USARTx) {
	USART_Cmd(USARTx, DISABLE);

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, DISABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, DISABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, DISABLE);  // USART use DMA1 -> RM0008 Table 78

	if (USARTx == USART1) {
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, DISABLE);
	} else if (USARTx == USART2) {
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, DISABLE);
	} else if (USARTx == USART3) {
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, DISABLE);
	}
}

int uart_write(USART_TypeDef* USARTx, uint8_t *data, int nbytes) {
	DMA_Channel_TypeDef* DMAy_Channelx_RX=0;
	DMA_Channel_TypeDef* DMAy_Channelx_TX=0;
	uint32_t dmaflag;

	uint8_t dummy = ' ';
	
	if (USARTx == USART1) {
		DMAy_Channelx_RX = DMA1_Channel3;  // see RM0008: Table 78. Summary of DMA1 requests for each channel
		DMAy_Channelx_TX = DMA1_Channel4;  // see RM0008: Table 78. Summary of DMA1 requests for each channel
		dmaflag = DMA1_FLAG_TC4;
	} else 	if (USARTx == USART2) {
		DMAy_Channelx_RX = DMA1_Channel6;  // see RM0008: Table 78. Summary of DMA1 requests for each channel
		DMAy_Channelx_TX = DMA1_Channel7;  // see RM0008: Table 78. Summary of DMA1 requests for each channel
		dmaflag = DMA1_FLAG_TC7;
	} else 	if (USARTx == USART3) {
		DMAy_Channelx_RX = DMA1_Channel3;  // see RM0008: Table 78. Summary of DMA1 requests for each channel
		DMAy_Channelx_TX = DMA1_Channel2;  // see RM0008: Table 78. Summary of DMA1 requests for each channel
		dmaflag = DMA1_FLAG_TC2;
	}

	/* DMA clock enable */

	DMA_InitTypeDef DMA_InitStructure;

	/* USARTy_Tx_DMA_Channel (triggered by USARTy Tx event) Config */
	DMA_DeInit(DMAy_Channelx_RX);
	DMA_DeInit(DMAy_Channelx_TX);

	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)(&(USARTx->DR));
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_BufferSize = nbytes;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;

	/* RX */
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)&dummy;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Disable;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
	DMA_Init(DMAy_Channelx_RX, &DMA_InitStructure);

	/* TX */
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)data;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
	DMA_Init(DMAy_Channelx_TX, &DMA_InitStructure);

	/* enable USARTx DMA TX,RX request */
	USART_DMACmd(USARTx, USART_DMAReq_Tx|USART_DMAReq_Rx, ENABLE);

	/* send Buffer */
	DMA_Cmd(DMAy_Channelx_TX, ENABLE);
	DMA_Cmd(DMAy_Channelx_RX, ENABLE);
	
	while (DMA_GetFlagStatus(dmaflag)==RESET);

	/* disable USARTy DMA */
	DMA_Cmd(DMAy_Channelx_TX, DISABLE);
	DMA_Cmd(DMAy_Channelx_RX, DISABLE);

	USART_DMACmd(USARTx, USART_DMAReq_Tx|USART_DMAReq_Rx, DISABLE);

	return 0;
}


int uart_putc(int c, USART_TypeDef* USARTx) {
	USARTx->DR = (c & 0xff);
	while (USART_GetFlagStatus(USARTx, USART_FLAG_TXE) == RESET);
	return 0;
}

int uart_getc(USART_TypeDef* USARTx) {
	while (USART_GetFlagStatus(USARTx, USART_FLAG_RXNE) == RESET);
	return USARTx->DR & 0xff;
}
