#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/device.h>

//local includes
#include "st7036.h"

/**
 * Prototypes for file operations on our device
 */
static int lcd_open(struct inode *, struct file *);
static int lcd_release(struct inode *, struct file *);
static ssize_t lcd_read(struct file *, char *buffer, size_t length, loff_t *off);
static ssize_t lcd_write(struct file *, const char *buffer, size_t length, loff_t *off);

static struct file_operations lcd_fops = {
	.read = lcd_read,
	.write = lcd_write,
	.open = lcd_open,
	.release = lcd_release,
};

#define SUCCESS 0
#define DEVICE_NAME "LCDchardev"
#define BUF_LEN 80
#define CLASS_NAME "chardev"

//global vars for our driver
static int Major;
static struct class* chardevClass = NULL;
static struct device* chardevDevice = NULL;

const char* filler_text = "You're reading!\n";

static int filler_index = 0;

//called when the module is loaded
//here we will run the long initialization sequence for the lcd controller
static int initialize(void)
{
	Major = register_chrdev(0, DEVICE_NAME, &lcd_fops);

	if (Major < 0)
	{
		printk(KERN_ALERT "Registering char device failed with %d\n", Major);
		return Major;
	}

	printk(KERN_INFO "LCDchardev assigned major num %d.\n", Major);

	// Register the device class for sysfs
	chardevClass = class_create(THIS_MODULE, CLASS_NAME);

	// Register the device driver for sysfs, this will make udev create an entry in /dev
	chardevDevice = device_create(chardevClass, NULL, MKDEV(Major, 0), NULL, DEVICE_NAME);

    //initialize the actual hardware
    st7036_Init();

	return SUCCESS;
}

//called when module is unloaded
static void cleanup(void)
{
	//undo everything in the reverse order
	st7036_Cleanup();
	device_destroy(chardevClass, MKDEV(Major, 0)); // remove the device
	class_unregister(chardevClass);				   // unregister the device class
	class_destroy(chardevClass);				   // remove the device class
	unregister_chrdev(Major, DEVICE_NAME);		   // unregister the major number
	printk(KERN_INFO "Goodbye!\n");
}

static int lcd_open(struct inode *ino, struct file *filp)
{
	try_module_get(THIS_MODULE);
	printk(KERN_INFO "LCD CHARDEV: opened\n");
	filler_index = 0;
	return SUCCESS;
}

static int lcd_release(struct inode *ino, struct file *filp)
{
	printk(KERN_INFO "LCD CHARDEV: closing\n");
	module_put(THIS_MODULE);
	return SUCCESS;
}

static ssize_t lcd_read(struct file *filp, char *buffer, size_t length, loff_t *off)
{
	const char* canned_msg = filler_text;
	int num_bytes = 0;
	printk(KERN_INFO "LCD CHARDEV: read\n");
	//send as much of the message as will fit
	while(length && canned_msg[filler_index])
	{
		put_user(canned_msg[filler_index], buffer);
		length--;
		buffer++;
		filler_index++;
		num_bytes++;
	}

	return num_bytes;
}

static ssize_t lcd_write(struct file *filp, const char *buffer, size_t length, loff_t *off)
{
	char from_user;
	int i;
	printk(KERN_INFO "LCD CHARDEV: write\n");
	//pull up to 32 characters from the file
	//ignore last character in buffer (null termination of string)
	for(i = 0; i < 32 && i < length - 1; i++)
	{
		if(i == 16)
		{
			st7036_SecondLine();
		}

		get_user(from_user, &buffer[i]);
		st7036_DataWrite((unsigned int)from_user);
	}

	//pretend like we read the whole buffer
	return length;
}

MODULE_LICENSE("GPL");
module_init(initialize);
module_exit(cleanup);

