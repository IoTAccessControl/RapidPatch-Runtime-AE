#include "stm32sys.h"

// defined(STM32L475_NO_OS)
#if 0

void delay_us(uint32_t nus) {
    uint32_t ticks;
    uint32_t told, tnow, tcnt = 0;
    uint32_t reload = SysTick->LOAD;				//LOAD的值
    ticks = nus * 80; 						//需要的节拍数
    told = SysTick->VAL;        				//刚进入时的计数器值

    while(1)
    {
        tnow = SysTick->VAL;

        if(tnow != told)
        {
            if(tnow < told)tcnt += told - tnow;	//这里注意一下SYSTICK是一个递减的计数器就可以了.
            else tcnt += reload - tnow + told;
			
            told = tnow;
            if(tcnt >= ticks)break;			//时间超过/等于要延迟的时间,则退出.
        }
    }
}

void LED_Init(void)
{
	/*
		LED-B	PE9
		LED-G	PE8
		LED-R	PE7	
	*/
    GPIO_InitTypeDef GPIO_InitStruct;

    __HAL_RCC_GPIOE_CLK_ENABLE();

    GPIO_InitStruct.Pin = GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

    HAL_GPIO_WritePin(GPIOE, GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9, GPIO_PIN_SET);
}

#endif