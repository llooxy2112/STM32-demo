#include <stdio.h>
#include <stm32f10x.h>
#include <stm32f10x_rcc.h>
#include <stm32f10x_gpio.h>
#include <stm32f10x_usart.h>
#include <stm32f10x_tim.h>
#include "uart.h"
#include "timer.h"

uint8_t TxBuffer[512];
#define TxBufferSize   (countof(TxBuffer))
#define countof(a)   (sizeof(a) / sizeof(*(a)))

static __IO uint32_t TimingDelay;

void Delay(uint32_t nTime);

void int2hex(char *buf,uint32_t i) {
	char v[] = { '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F' };
	int pos;
	uint32_t code = i;
        
	for (pos=0;pos<8;pos++) {
		buf[7-pos] = v[(code&0x0F)];
		code = code >> 4;
	}
	buf[8]=0;
}

int main(void) {
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

	GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	if (SysTick_Config(SystemCoreClock/1000)) 
		while(1);
	
	
	uart_init(USART1, 19200, USART_Mode_Rx | USART_Mode_Tx);
	// PWM frequency = 100kHz Pulse (10µs) at 100 hz with 72,000,000 hz system clock
	// 24.000.000/240 = 100,000 Hz
	// 100,000/10,000 = 10Hz
	timer_init(TIM2,2,240-1,10000-1,TIM_OCMode_PWM1);  // TIM2 Chan2 0..239,0..9999
	TIM_SetCompare2(TIM2,1);  // Chan2 one 100kHz Pulse (10µs)

	// 24.000.000/24 = 1,000,000 Hz (1µs resulution) 
	// no prescale

	timer_initIC(TIM1,1,24-1,50000);  
                // TIM1 Chan1 0..23,0 as Input Capture with edgedetect. chan2 (falling) tied to cha1 (rising) 
		// prescaler -> resulution 1µs (1MHz)
		// periode -> max time to mesure -> 1µs*50.000-> 50ms	(20Hz)
	int iCount1 = 0;
	int iCount2 = 0;
	int i;
	static int ledval = 0;
	int iinc = 1;
	while (1) {
		if (iCount1%500==0) {
			GPIO_WriteBit(GPIOC, GPIO_Pin_13, (ledval) ? Bit_SET : Bit_RESET);
			ledval = 1 - ledval;
			int2hex((char *)TxBuffer,iCount2++);
			for (i=0;TxBuffer[i];i++) {
				uart_putc(TxBuffer[i],USART1);
			}
			uart_putc(' ',USART1);

			int2hex((char *)TxBuffer,TIM_GetCapture1(TIM1));
			for (i=0;TxBuffer[i];i++) {
				uart_putc(TxBuffer[i],USART1);
			}
			uart_putc(' ',USART1);
			int2hex((char *)TxBuffer,TIM_GetCapture2(TIM1));
			for (i=0;TxBuffer[i];i++) {
				uart_putc(TxBuffer[i],USART1);
			}
			uart_putc('\n',USART1);
			uart_putc('\r',USART1);

		}
		iCount1++;
		Delay(1);
		i=i+iinc;
	}
}

void Delay(uint32_t nTime) {
	TimingDelay = nTime;
	while (TimingDelay != 0);
}

void SysTick_Handler(void) {
	if (TimingDelay != 0x00) {
		TimingDelay--;
	}
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t* file, uint32_t line) {
	while (1);
}
#endif


