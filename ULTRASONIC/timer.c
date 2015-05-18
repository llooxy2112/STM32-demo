#include <stdio.h>
#include <stm32f10x.h>
#include <stm32f10x_rcc.h>
#include <stm32f10x_gpio.h>
#include <stm32f10x_tim.h>
#include "timer.h"

int timer_init(TIM_TypeDef* TIMx, uint32_t chanels,uint16_t prescaler, uint16_t period,uint16_t OCMode) {
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

	if (TIMx == TIM1) {
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);

		if (OCMode) {
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
			GPIO_StructInit(&GPIO_InitStructure);
			if (chanels & 1) {
				// Pin PA8
				GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
				GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
				GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
				GPIO_Init(GPIOA, &GPIO_InitStructure);
			}
			if (chanels & 2) {
				// Pin PA9
				GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
				GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
				GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
				GPIO_Init(GPIOA, &GPIO_InitStructure);
			}
			if (chanels & 3) {
				// Pin PA10
				GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
				GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
				GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
				GPIO_Init(GPIOA, &GPIO_InitStructure);
			}
			if (chanels & 4) {
				// Pin PA11
				GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
				GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
				GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
				GPIO_Init(GPIOA, &GPIO_InitStructure);
			}
		}
	} else if (TIMx == TIM2) {
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

		if (OCMode) {
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
			GPIO_StructInit(&GPIO_InitStructure);
			if (chanels & 1) {
				// Pin PA0
				GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
				GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
				GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
				GPIO_Init(GPIOA, &GPIO_InitStructure);
			}
			if (chanels & 2) {
				// Pin PA1
				GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
				GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
				GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
				GPIO_Init(GPIOA, &GPIO_InitStructure);
			}
			if (chanels & 3) {
				// Pin PA2
				GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
				GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
				GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
				GPIO_Init(GPIOA, &GPIO_InitStructure);
			}
			if (chanels & 4) {
				// Pin PA3
				GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
				GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
				GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
				GPIO_Init(GPIOA, &GPIO_InitStructure);
			}
		}
	}


	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	TIM_OCInitTypeDef TIM_OCInitStructure;

	// configure timer
	TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
	TIM_TimeBaseStructure.TIM_Prescaler = prescaler; 
	TIM_TimeBaseStructure.TIM_Period = period;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIMx,&TIM_TimeBaseStructure);
	// PWM1 Mode configuration : Channelx
	// Edge - aligned ; not single pulse mode
	if (OCMode) {
		TIM_OCStructInit(&TIM_OCInitStructure);
		if (chanels & 1) {
			TIM_OCInitStructure.TIM_OCMode = OCMode;
			TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
			TIM_OC1Init(TIMx,&TIM_OCInitStructure);
		}
		if (chanels & 2) {
			TIM_OCInitStructure.TIM_OCMode = OCMode;
			TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
			TIM_OC2Init(TIMx,&TIM_OCInitStructure);
		}
		if (chanels & 3) {
			TIM_OCInitStructure.TIM_OCMode = OCMode;
			TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
			TIM_OC3Init(TIMx,&TIM_OCInitStructure);
		}
		if (chanels & 4) {
			TIM_OCInitStructure.TIM_OCMode = OCMode;
			TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
			TIM_OC4Init(TIMx,&TIM_OCInitStructure);
		}
	}
	// Enable Timer
	TIM_Cmd(TIMx,ENABLE);
}


int timer_initIC(TIM_TypeDef* TIMx, uint32_t chanels,uint16_t prescaler, uint16_t period) {
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

	if (TIMx == TIM1) {
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);

		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
		GPIO_StructInit(&GPIO_InitStructure);
		if (chanels & 1) {
			// Pin PA8
			GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
			GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
			GPIO_Init(GPIOA, &GPIO_InitStructure);
		}
		if (chanels & 2) {
			// Pin PA9
			GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
			GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
			GPIO_Init(GPIOA, &GPIO_InitStructure);
		}
		if (chanels & 3) {
			// Pin PA10
			GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
			GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
			GPIO_Init(GPIOA, &GPIO_InitStructure);
		}
		if (chanels & 4) {
			// Pin PA11
			GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
			GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
			GPIO_Init(GPIOA, &GPIO_InitStructure);
		}
	} else if (TIMx == TIM2) {
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
		GPIO_StructInit(&GPIO_InitStructure);
		if (chanels & 1) {
			// Pin PA0
			GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
			GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
			GPIO_Init(GPIOA, &GPIO_InitStructure);
		}
		if (chanels & 2) {
			// Pin PA1
			GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
			GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
			GPIO_Init(GPIOA, &GPIO_InitStructure);
		}
		if (chanels & 3) {
			// Pin PA2
			GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
			GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
			GPIO_Init(GPIOA, &GPIO_InitStructure);
		}
		if (chanels & 4) {
			// Pin PA3
			GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
			GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
			GPIO_Init(GPIOA, &GPIO_InitStructure);
		}
	}


	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	TIM_OCInitTypeDef TIM_OCInitStructure;

	// configure timer
	TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
	TIM_TimeBaseStructure.TIM_Prescaler = prescaler; 
	TIM_TimeBaseStructure.TIM_Period = period;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIMx,&TIM_TimeBaseStructure);
	// PWM1 Mode configuration : Channelx
	// Edge - aligned ; not single pulse mode

	TIM_ICInitTypeDef TIM_ICInitStructure;
	TIM_ICStructInit(&TIM_ICInitStructure);

	TIM_ICInitStructure.TIM_Channel = TIM_Channel_1;
	TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;
	TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
	TIM_ICInitStructure.TIM_ICPrescaler = 0;
	TIM_ICInitStructure.TIM_ICFilter = 0;
	TIM_ICInit(TIMx,&TIM_ICInitStructure);

	TIM_ICInitStructure.TIM_Channel = TIM_Channel_2;
	TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Falling;
	TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_IndirectTI;  // connect input Chan2 to Chan1 
	TIM_ICInit(TIMx,&TIM_ICInitStructure);

	TIM_SelectInputTrigger(TIMx,TIM_TS_TI1FP1);  // Chan 1 Filtered as Input Trigger
	TIM_SelectSlaveMode(TIM1,TIM_SlaveMode_Reset);
	TIM_SelectMasterSlaveMode(TIM1,TIM_MasterSlaveMode_Enable);

	// Enable Timer
	TIM_Cmd(TIMx,ENABLE);
}
