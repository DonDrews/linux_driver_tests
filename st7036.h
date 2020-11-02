#ifndef ST7036_H
#define ST7036_H

#include <asm/io.h>
#include <linux/delay.h>
#include <linux/module.h>

typedef uint32_t pin_t;

int st7036_Init(void);
int st7036_Cleanup(void);
void st7036_Clear(void);
void st7036_Home(void);
int st7036_DataWrite(unsigned int data);
void st7036_SecondLine(void);

#endif
