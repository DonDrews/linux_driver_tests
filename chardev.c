/*
 * chardev.c: Creates a read-only char device that says how many times you've
 * read from the dev file.
 *
 * You can have some fun with this by removing the module_get/put calls,
 * allowing the module to be removed while the file is still open.
 *
 * Compile with `make`. Load with `sudo insmod chardev.ko`. Check `dmesg | tail`
 * output to see the assigned device number and command to create a device file.
 *
 * From TLDP.org's LKMPG book.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h> /* for put_user */
#include <linux/device.h>

#define CHARDEV_MAGIC 0x55
#define CHARDEV_IOCTL_CLR _IO(CHARDEV_MAGIC, 0x1)

//GPIO registers
#define GPFSEL1 (*(unsigned int*) 0x20200004)
#define GPSET0 (*(unsigned int*) 0x2020001C)
#define GPCLR0 (*(unsigned int*) 0x20200028)

/*
 * Prototypes - this would normally go in a .h file
 */

int init_module(void);
void cleanup_module(void);
static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);
static long mychardev_ioctl(struct file *file, unsigned int cmd, unsigned long arg);

#define SUCCESS 0
#define DEVICE_NAME "chardev"
#define BUF_LEN 80
#define CLASS_NAME "chardev" ///< The device class -- this is a character device driver

/*
 * Global variables are declared as static, so are global within the file.
 */

static int Major;
static int Device_Open = 0;
static char msg[BUF_LEN];
static char *msg_Ptr;

static struct class *chardevClass = NULL;	///< The device-driver class struct pointer
static struct device *chardevDevice = NULL; ///< The device-driver device struct pointer

static struct file_operations fops = {
	.read = device_read,
	.write = device_write,
	.open = device_open,
	.release = device_release,
	.unlocked_ioctl = mychardev_ioctl};

/*
 * This function is called when the module is loaded
 */
static int initialize(void)
{
	Major = register_chrdev(0, DEVICE_NAME, &fops);

	if (Major < 0)
	{
		printk(KERN_ALERT "Registering char device failed with %d\n", Major);
		return Major;
	}

	printk(KERN_INFO "I was assigned major number %d. To talk to\n", Major);
	printk(KERN_INFO "the driver, create a dev file with\n");
	printk(KERN_INFO "'mknod /dev/%s c %d 0'.\n", DEVICE_NAME, Major);
	printk(KERN_INFO "Try various minor numbers. Try to cat and echo to\n");
	printk(KERN_INFO "the device file.\n");
	printk(KERN_INFO "Remove the device file and module when done.\n");

	// Register the device class
	chardevClass = class_create(THIS_MODULE, CLASS_NAME);

	// Register the device driver
	chardevDevice = device_create(chardevClass, NULL, MKDEV(Major, 0), NULL, DEVICE_NAME);

	//setup GPIO
	GPFSEL1 &= (7 << 18);
	GPFSEL1 |= (1 << 18);

	return SUCCESS;
}

/*
 * This function is called when the module is unloaded
 */
void cleanup_module(void)
{
	/*
   * Unregister the device
   */
	device_destroy(chardevClass, MKDEV(Major, 0)); // remove the device
	class_unregister(chardevClass);				   // unregister the device class
	class_destroy(chardevClass);				   // remove the device class
	unregister_chrdev(Major, DEVICE_NAME);		   // unregister the major number
	printk(KERN_INFO "EBBChar: Goodbye from the LKM!\n");
}

/*
 * Methods
 */

/*
 * Called when a process tries to open the device file, like
 * "cat /dev/mycharfile"
 */
static int device_open(struct inode *inode, struct file *filp)
{
	static int counter = 0;

	if (Device_Open)
		return -EBUSY;

	Device_Open++;
	sprintf(msg, "I already told you %d times Hello world!\n", counter++);
	msg_Ptr = msg;
	/*
   * TODO: comment out the line below to have some fun!
   */
	try_module_get(THIS_MODULE);

	return SUCCESS;
}

/*
 * Called when a process closes the device file.
 */
static int device_release(struct inode *inode, struct file *filp)
{
	Device_Open--;

	/*
   * Decrement the usage count, or else once you opened the file, you'll never
   * get rid of the module.
   *
   * TODO: comment out the line below to have some fun!
   */
	module_put(THIS_MODULE);

	return SUCCESS;
}

/*
 * Called when a process, which already opened the dev file, attempts to read
 * from it.
 */
static ssize_t device_read(struct file *filp, /* see include/linux/fs.h   */
						   char *buffer,	  /* buffer to fill with data */
						   size_t length,	  /* length of the buffer     */
						   loff_t *offset)
{

	static int gpio_state = 0;
	/*
   * Number of bytes actually written to the buffer
   */
	int bytes_read = 0;

	/*
   * If we're at the end of the message, return 0 signifying end of file.
   */
	if (*msg_Ptr == 0)
		return 0;

	/*
   * Actually put the data into the buffer
   */
	while (length && *msg_Ptr)
	{
		/*
     * The buffer is in the user data segment, not the kernel segment so "*"
     * assignment won't work. We have to use put_user which copies data from the
     * kernel data segment to the user data segment.
     */
		put_user(*(msg_Ptr++), buffer++);
		length--;
		bytes_read++;
	}

	//toggle gpio
	if(gpio_state)
		GPCLR0 |= (1 << 16);
	else
		GPSET0 |= (1 << 16);

	gpio_state = !gpio_state;
	/*
   * Most read functions return the number of bytes put into the buffer
   */
	return bytes_read;
}

/*
 * Called when a process writes to dev file: echo "hi" > /dev/hello
 */
static ssize_t
device_write(struct file *filp, const char *buf, size_t len, loff_t *off)
{
	printk(KERN_ALERT "Sorry, this operation isn't supported.\n");
	return -EINVAL;
}

static long mychardev_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	printk("MYCHARDEV: Device ioctl with cmd:%d\n", cmd);
	printk("%d\n", CHARDEV_IOCTL_CLR);
	switch(cmd)
		{
		case CHARDEV_IOCTL_CLR:
			printk("Clearing count!\n");
			break;
		default:
			return -EINVAL;
		}
	return 0;
}

MODULE_LICENSE("GPL");
module_init(initialize);