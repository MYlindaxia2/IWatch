#include "../stm32f10x.h"

void LED_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	GPIO_InitTypeDef GPIO_Def = {0};
	GPIO_Def.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Def.GPIO_Pin = GPIO_Pin_15;
	GPIO_Def.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_SetBits(GPIOB, GPIO_Pin_15);
	GPIO_Init(GPIOB, &GPIO_Def);

}

void LED_ON(void)
{
	GPIO_ResetBits(GPIOB, GPIO_Pin_15);
}
void LED_OFF(void)
{
	GPIO_SetBits(GPIOB, GPIO_Pin_15);
}
void LED_FLIP(void)
{
	if(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_15))
		GPIO_ResetBits(GPIOB, GPIO_Pin_15);
	else
		GPIO_SetBits(GPIOB, GPIO_Pin_15);
}

