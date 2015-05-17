#include <stdio.h>
#include <stm32f10x.h>
#include <stm32f10x_rcc.h>
#include <stm32f10x_gpio.h>
#include <stm32f10x_spi.h>
#include <stm32f10x_dma.h>
#include "spidma.h"

int spi_init(SPI_TypeDef* SPIx) {
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);  // USART use DMA1 -> RM0008 Table 78

	if (SPIx == SPI1) {

		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);

		GPIO_StructInit(&GPIO_InitStructure);
		// SPI1_NCC Pin PA4
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOA, &GPIO_InitStructure);

		// SPI1_SCK Pin PA5
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOA, &GPIO_InitStructure);

		// SPI1_MISO Pin PA6
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;  // Input pull-up
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOA, &GPIO_InitStructure);

		// SPI1_MOSI Pin PA7
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOA, &GPIO_InitStructure);
	} else 	if (SPIx == SPI2) {

		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);

		GPIO_StructInit(&GPIO_InitStructure);
		// SPI2_NCC Pin PB12
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOB, &GPIO_InitStructure);

		// SPI2_SCK Pin PB13
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOB, &GPIO_InitStructure);

		// SPI2_MISO Pin PB14
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;  // Input pull-up
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOB, &GPIO_InitStructure);

		// SPI2_MOSI Pin PB15
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOB, &GPIO_InitStructure);
	}

	SPI_InitTypeDef SPI_InitStructure;

	SPI_StructInit(&SPI_InitStructure);
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
  	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
        SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_64;
	SPI_InitStructure.SPI_CRCPolynomial = 7;
	SPI_Init(SPIx, &SPI_InitStructure);

	SPI_Cmd(SPIx, ENABLE);
}

int spi_close(SPI_TypeDef* SPIx) {
	SPI_Cmd(SPIx, DISABLE);

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, DISABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, DISABLE);  // SPI use DMA1 -> RM0008 Table 78

	if (SPIx == SPI1) {
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, DISABLE);
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, DISABLE);
	} else if (SPIx == SPI2) {
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, DISABLE);
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, DISABLE);
	}
}

int spi_read_write(SPI_TypeDef* SPIx,uint8_t *rbuf, const uint8_t *wbuf, int cnt, uint16_t speed) {
	DMA_Channel_TypeDef* DMAy_Channelx_RX=0;
	DMA_Channel_TypeDef* DMAy_Channelx_TX=0;
	uint32_t dmaflag;
	uint16_t dummy[] = { 0xFFFF }; /* dummy if rbuf or wbuf empty */
	int i;

	assert_param(IS_SPI_BAUDRATE_PRESCALER(speed));

	SPIx->CR1 = (SPIx->CR1 & ~SPI_BaudRatePrescaler_256) | speed;
	SPI_DataSizeConfig(SPIx, SPI_DataSize_8b);

	if (cnt>4) {
		if (SPIx == SPI1) {
			DMAy_Channelx_RX = DMA1_Channel2;  // see RM0008: Table 78. Summary of DMA1 requests for each channel
			DMAy_Channelx_TX = DMA1_Channel3;  // see RM0008: Table 78. Summary of DMA1 requests for each channel
			if (rbuf) {
				dmaflag = DMA1_FLAG_TC2;
			} else {
				dmaflag = DMA1_FLAG_TC3;
			}
		} else 	if (SPIx == SPI2) {
			DMAy_Channelx_RX = DMA1_Channel4;  // see RM0008: Table 78. Summary of DMA1 requests for each channel
			DMAy_Channelx_TX = DMA1_Channel5;  // see RM0008: Table 78. Summary of DMA1 requests for each channel
			if (rbuf) {
				dmaflag = DMA1_FLAG_TC4;
			} else {
				dmaflag = DMA1_FLAG_TC5;
			}
		}

		DMA_InitTypeDef DMA_InitStructure;

		/* SPIx_Tx_DMA_Channel (triggered by SPIx Rx event) Config */
		DMA_DeInit(DMAy_Channelx_RX);
		DMA_DeInit(DMAy_Channelx_TX);

		DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
		DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)(&(SPIx->DR));
		DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
		DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
		DMA_InitStructure.DMA_BufferSize = cnt;
		DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
		DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
		DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;

		/* RX */
		if (rbuf) {
			DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)rbuf;
			DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
		} else {
			DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)dummy;
			DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Disable;
		}
		DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
		DMA_Init(DMAy_Channelx_RX, &DMA_InitStructure);

		/* TX */
		if (wbuf) {
			DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)wbuf;
			DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
		} else {
			DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)dummy;
			DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Disable;
		}
		DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
		DMA_Init(DMAy_Channelx_TX, &DMA_InitStructure);

		/* enable SPIx DMA TX,RX request */
		SPI_I2S_DMACmd(SPIx, SPI_I2S_DMAReq_Tx|SPI_I2S_DMAReq_Rx, ENABLE);

		/* send Buffer */
		DMA_Cmd(DMAy_Channelx_TX, ENABLE);
		DMA_Cmd(DMAy_Channelx_RX, ENABLE);
	
		while (DMA_GetFlagStatus(dmaflag)==RESET);

		/* disable SPIx DMA */
		DMA_Cmd(DMAy_Channelx_TX, DISABLE);
		DMA_Cmd(DMAy_Channelx_RX, DISABLE);

		SPI_I2S_DMACmd(SPIx, SPI_I2S_DMAReq_Tx|SPI_I2S_DMAReq_Rx, DISABLE);
	} else {	
		for (i = 0; i < cnt; i++) {
			while (SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_TXE) == RESET);
			if (wbuf) {
				SPI_I2S_SendData(SPIx, *wbuf++);
			} else {
				SPI_I2S_SendData(SPIx, 0xFF);
			}
			while (SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_RXNE) == RESET); // Receive buffer not empty flag.
			if (rbuf) {
				*rbuf++ = SPI_I2S_ReceiveData(SPIx);
			} else {
				SPI_I2S_ReceiveData(SPIx); // dump data
			}
		}
	}
	
	return i;
}

int spi_read_write16(SPI_TypeDef* SPIx,uint16_t *rbuf, const uint16_t *wbuf, int cnt, uint16_t speed) {
	DMA_Channel_TypeDef* DMAy_Channelx_RX=0;
	DMA_Channel_TypeDef* DMAy_Channelx_TX=0;
	uint16_t dummy[] = { 0xFFFF }; /* dummy if rbuf or wbuf empty */
	uint32_t dmaflag;
	int i;

	assert_param(IS_SPI_BAUDRATE_PRESCALER(speed));

	SPIx->CR1 = (SPIx->CR1 & ~SPI_BaudRatePrescaler_256) | speed;
	SPI_DataSizeConfig(SPIx, SPI_DataSize_16b);
	

	if (cnt>4) {
		if (SPIx == SPI1) {
			DMAy_Channelx_RX = DMA1_Channel2;  // see RM0008: Table 78. Summary of DMA1 requests for each channel
			DMAy_Channelx_TX = DMA1_Channel3;  // see RM0008: Table 78. Summary of DMA1 requests for each channel
			if (rbuf) {
				dmaflag = DMA1_FLAG_TC2;
			} else {
				dmaflag = DMA1_FLAG_TC3;
			}
		} else 	if (SPIx == SPI2) {
			DMAy_Channelx_RX = DMA1_Channel4;  // see RM0008: Table 78. Summary of DMA1 requests for each channel
			DMAy_Channelx_TX = DMA1_Channel5;  // see RM0008: Table 78. Summary of DMA1 requests for each channel
			if (rbuf) {
				dmaflag = DMA1_FLAG_TC4;
			} else {
				dmaflag = DMA1_FLAG_TC5;
			}
		}

		DMA_InitTypeDef DMA_InitStructure;

		/* SPIx_Tx_DMA_Channel (triggered by SPIx Rx event) Config */
		DMA_DeInit(DMAy_Channelx_RX);
		DMA_DeInit(DMAy_Channelx_TX);

		DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
		DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)(&(SPIx->DR));
		DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
		DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
		DMA_InitStructure.DMA_BufferSize = cnt;
		DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
		DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
		DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;

		/* RX */
		if (rbuf) {
			DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)rbuf;
			DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
		} else {
			DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)dummy;
			DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Disable;
		}
		DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
		DMA_Init(DMAy_Channelx_RX, &DMA_InitStructure);

		/* TX */
		if (wbuf) {
			DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)wbuf;
			DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
		} else {
			DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)dummy;
			DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Disable;
		}
		DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
		DMA_Init(DMAy_Channelx_TX, &DMA_InitStructure);

		/* enable SPIx DMA TX,RX request */
		SPI_I2S_DMACmd(SPIx, SPI_I2S_DMAReq_Tx|SPI_I2S_DMAReq_Rx, ENABLE);

		/* send Buffer */
		DMA_Cmd(DMAy_Channelx_TX, ENABLE);
		DMA_Cmd(DMAy_Channelx_RX, ENABLE);
	
		while (DMA_GetFlagStatus(dmaflag)==RESET);

		/* disable SPIx DMA */
		DMA_Cmd(DMAy_Channelx_TX, DISABLE);
		DMA_Cmd(DMAy_Channelx_RX, DISABLE);

		SPI_I2S_DMACmd(SPIx, SPI_I2S_DMAReq_Tx|SPI_I2S_DMAReq_Rx, DISABLE);
	} else {	
		for (i = 0; i < cnt; i++) {
			while (SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_TXE) == RESET);
			if (wbuf) {
				SPI_I2S_SendData(SPIx, *wbuf++);
			} else {
				SPI_I2S_SendData(SPIx, 0xFF);
			}
			while (SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_RXNE) == RESET); // Receive buffer not empty flag.
			if (rbuf) {
				*rbuf++ = SPI_I2S_ReceiveData(SPIx);
			} else {
				SPI_I2S_ReceiveData(SPIx); // dump data
			}
		}
	}
	
	return i;
}
