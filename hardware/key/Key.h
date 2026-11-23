#ifndef __KEY_H
#define __KEY_H
#include <stdint.h>
typedef enum
{
	NONE = 0,
	ENTER = 1,
	LEFT = 2,
	RIGHT = 3
}KeyFun;
void Key_Init(void);
uint8_t Key_GetNum(void);
uint8_t KeyGetNumState(void);
#endif
