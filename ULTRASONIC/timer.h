int timer_init(TIM_TypeDef* TIMx, uint32_t chanels,uint16_t prescaler, uint16_t period,uint16_t OCMode);
int timer_initIC(TIM_TypeDef* TIMx, uint32_t chanels,uint16_t prescaler, uint16_t period);