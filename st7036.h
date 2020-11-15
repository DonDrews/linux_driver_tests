#ifndef ST7036_H
#define ST7036_H

#include <asm/io.h>
#include <linux/delay.h>
#include <linux/module.h>

//broadcom pin number for RPi
typedef uint32_t pin_t;

//map IO registers to virtual memory and
//send initialization commands to the LCD screen
int st7036_Init(void);

//free vmem
int st7036_Cleanup(void);

//send clear command to the lcd (called before each write)
void st7036_Clear(void);

//reset the cursor position to the start of screen
void st7036_Home(void);

//write a character to the screen
//cursor is auto-incremented
int st7036_DataWrite(unsigned int data);

//set the cursor to the second line of the screen
void st7036_SecondLine(void);

#endif
