#include "st7036.h"

//physical pins for interfacing with the LCD screen
const pin_t rs = 2;
const pin_t rw = 1;
const pin_t en = 0;
const pin_t data_bus[] = {1, 2, 3, 4};

//GPIO registers on RPi 2b
#define GPIO_BASE 0x3F200000
#define GPIO_REG_SIZE 0xB0
#define GPFSEL0_OFF 0x0000
#define GPSET0_OFF 0x001C
#define GPCLR0_OFF 0x0028

static void __iomem* gpio_base_vaddr;

//Private interface functions
static void commandWrite(int cmd);
static void pulse_enable(void);
static void gpio_set(pin_t io);
static void gpio_clear(pin_t io);
static void bus_update(const pin_t* bus, uint8_t nibble);
static void gpio_function_set(pin_t io);
static void startup_sequence(void);


/****************************
 * Public interface
 * **************************
*/

int st7036_Init(void)
{
	int i;
	gpio_base_vaddr = ioremap(GPIO_BASE, GPIO_REG_SIZE);

	//Set pins as output
	gpio_function_set(rs);
	gpio_function_set(rw);
	gpio_function_set(en);

	for(i = 0; i < 4; i++)
		gpio_function_set(data_bus[i]);

	startup_sequence();

	return 0;
}

int st7036_Cleanup(void)
{
	iounmap((void*)GPIO_BASE);
	return 0;
}

void st7036_Clear(void)
{
	commandWrite(0x00);
	commandWrite(0x01);
	usleep_range(1500, 4000);
}


void st7036_Home(void)
{
	commandWrite(0x00);
	commandWrite(0x02);
	usleep_range(1500, 4000);
}

void st7036_SecondLine(void)
{
	//change cursor address
	commandWrite(12);
	commandWrite(0);
	usleep_range(1000, 4000);
}


int st7036_DataWrite(unsigned int data)
{
	unsigned int hinibble, lownibble;

	gpio_set(rs);
	gpio_clear(rw);

	hinibble = data >> 4;
	lownibble = data & 0xf;

	bus_update(data_bus, hinibble);
	pulse_enable();
	bus_update(data_bus, lownibble);
	pulse_enable();

	return 0;
}

/*************************
 *  Private interface
 * ************************
 */

static void startup_sequence()
{
	/* Initialize display */
	gpio_clear(rw);

	//initialization command 3 times
	commandWrite(0x3);
	usleep_range(1000, 4000);
	commandWrite(0x3);
	usleep_range(1000, 4000);
	commandWrite(0x3);
	usleep_range(1000, 4000);

	//set 4 bit mode
	commandWrite(2);
	commandWrite(2);

	//function set
	commandWrite(8);

	//set cursor properties
	commandWrite(0);
	commandWrite(15);

	//clear the display
	commandWrite(0);
	commandWrite(1);

	//entry mode, shifting options
	commandWrite(0);
	commandWrite(6);
	usleep_range(1000, 4000);

	//change cursor address
	commandWrite(12);
	commandWrite(3);
	usleep_range(1000, 4000);
}

static void commandWrite(int cmd)
{
	gpio_clear(rs);
	gpio_clear(rw);

	bus_update(data_bus, cmd);

	pulse_enable();
}

static void pulse_enable(void)
{
	gpio_clear(en);
	usleep_range(50, 200);

	gpio_set(en);
	usleep_range(50, 200);

	gpio_clear(en);
	usleep_range(50, 200);
}

static void gpio_set(pin_t io)
{
	void __iomem* reg_addr = gpio_base_vaddr + GPSET0_OFF + ((io / 32) << 2);
	writel(0x1 << (io % 32), reg_addr);
}

static void gpio_clear(pin_t io)
{
	void __iomem* reg_addr = gpio_base_vaddr + GPCLR0_OFF + ((io / 32) << 2);
	writel(0x1 << (io % 32), reg_addr);
}

//put nibble onto the pins in the data bus
static void bus_update(const pin_t* bus, uint8_t nibble)
{
	int i;
	for(i = 0; i < 4; i++)
	{
		if(nibble & 0x1)
		{
			gpio_set(bus[i]);
		}
		else
		{
			gpio_clear(bus[i]);
		}

		nibble = nibble >> 1;
	}
}

//set the pin mode registers to make io an output
static void gpio_function_set(pin_t io)
{
	int reg_num = io / 10;
	int offset;
	unsigned int gpfsel_val;
	void __iomem* reg_addr = gpio_base_vaddr + GPFSEL0_OFF + (reg_num << 2);
	offset = (io % 10) * 3;

	gpfsel_val = readl(reg_addr);
	gpfsel_val &= ~(0x7 << offset);
	gpfsel_val |= (0x1 << offset);
	writel(gpfsel_val, reg_addr);
}

MODULE_LICENSE("GPL");
