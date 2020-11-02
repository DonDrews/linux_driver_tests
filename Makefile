obj-m += total_lcd.o
total_lcd-objs := lcd_chardev.o st7036.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
