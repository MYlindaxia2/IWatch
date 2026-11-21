#include "stm32f10x.h"                  // Device header
#include "../hardware/delay/Delay.h"

uint8_t count = 0;
/**
  * 函    数：按键初始化
  * 参    数：无
  * 返 回 值：无
  */
void Key_Init(void)
{
	/*开启时钟*/
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);		//开启GPIOB的时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	/*GPIO初始化*/
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);						//将PB1和PB11引脚初始化为上拉输入
	GPIO_InitTypeDef GPIO_Def = {0};
	GPIO_Def.GPIO_Mode = GPIO_Mode_IPD;
	GPIO_Def.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_4;
	GPIO_Def.GPIO_Speed = GPIO_Speed_50MHz;

	GPIO_Init(GPIOA, &GPIO_Def);
}

/**
  * 函    数：按键获取键码
  * 参    数：无
  * 返 回 值：按下按键的键码值，范围：0~2，返回0代表没有按键按下
  * 注意事项：此函数是阻塞式操作，当按键按住不放时，函数会卡住，直到按键松手
  */
uint8_t Key_GetNum(void)
{
	uint8_t KeyNum = 0;		//定义变量，默认键码值为0
	
	if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_1) == 0)			//读PB1输入寄存器的状态，如果为0，则代表按键1按下
	{
		Delay_ms(20);											//延时消抖
		while (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_1) == 0);	//等待按键松手
		Delay_ms(20);											//延时消抖
		KeyNum = 1;												//置键码为1
	}
	
	if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_11) == 0)			//读PB11输入寄存器的状态，如果为0，则代表按键2按下
	{
		Delay_ms(20);											//延时消抖
		while (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_11) == 0);	//等待按键松手
		Delay_ms(20);											//延时消抖
		KeyNum = 2;												//置键码为2
	}
	
	return KeyNum;			//返回键码值，如果没有按键按下，所有if都不成立，则键码为默认值0
}


uint8_t KeyGetNumState(void)
{
	uint8_t raw = 0;
	static uint8_t lastRaw = 0;
	static uint8_t pressed = 0;

	if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_4))	raw = 1;
	if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_6))	raw = 2;
	if(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_1))	raw = 3;
	// 按键没稳定之前
	if(raw != lastRaw)
	{
		lastRaw = raw;
		count = 0;
		return 0;
	}
	if(count < 20)
	{
		return 0;
	}
	// 按键稳定结束,松开之前
	if(raw != 0)
	{
		pressed = raw;
		return 0;
	}else
	{
		uint8_t key = pressed;
		pressed = 0;
		return key;
	}
	return 0;
}

/* 定时器中断函数，可以复制到使用它的地方 */
void TIM2_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) == SET)
	{
		count++;
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
	}
}
