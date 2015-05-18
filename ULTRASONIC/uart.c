#include <stm32f10x.h>
#include <stm32f10x_rcc.h>
#include <stm32f10x_gpio.h>
#include <stm32f10x_usart.h>
#include "uart.h"

int uart_init(USART_TypeDef* USARTx, uint32_t baud, uint32_t flags) {
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

	if (USARTx == USART1) {
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

		GPIO_StructInit(&GPIO_InitStructure);
		if (flags & USART_Mode_Tx) {
			// UART1_Tx
			GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
			GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
			GPIO_Init(GPIOA, &GPIO_InitStructure);
		}
		if (flags & USART_Mode_Rx) {
			// UART1_Rx
			GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
			GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
			GPIO_Init(GPIOA, &GPIO_InitStructure);
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

	if (USARTx == USART1) {
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, DISABLE);
	}
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
