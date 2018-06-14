#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/module.h>
#include <linux/io.h>
#include <linux/uaccess.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>

#define GPIO_MAJOR 200
#define GPIO_MINOR 0
#define GPIO_DEVICE "gpioirq_led1"

#define GPIO_LED1 17
#define GPIO_LED2 27
#define GPIO_SW	  24
#define buf_size 100
#define BUFFER_SIZE 100

#define GPIO_BASE (BCM_IO_BASE + 0x200000)
#define GPIO_SIZE 0xB4

static char msg[BUFFER_SIZE] = {0};

MODULE_LICENSE("GPL");
MODULE_AUTHOR("YW KIM");
MODULE_DESCRIPTION("Raspberry Pi First Device Driver");

static int gpio_open(struct inode *inod, struct file *fil);
static int gpio_close(struct inode *inod, struct file *fil);
static ssize_t gpio_write(struct file *inod, const char *buff, size_t len, loff_t *off);
static ssize_t gpio_read(struct file *inode, char *buff, size_t len, loff_t *off);

struct cdev gpio_cdev;
static int switch_irq;

static struct file_operations gpio_fops = {
	.owner = THIS_MODULE,
    .read = gpio_read,
    .write = gpio_write,
	.open = gpio_open,
	.release = gpio_close
}; 

static irqreturn_t isr_func(int irq, void *data)
{
	static int count;

	if(irq == switch_irq && !gpio_get_value(GPIO_LED1)){
		gpio_set_value(GPIO_LED1, 1);
	} else {
		gpio_set_value(GPIO_LED1, 0);
	}
	printk(KERN_INFO "isr_function: called isr_function (callno : %d)\n", count);

	count++;
	return IRQ_HANDLED;
}

volatile unsigned int *gpio;

static int __init initModule(void)
{
    dev_t devno;
    unsigned int count;
    int err;

    // checking function called

    // 1. register charater device
    devno = MKDEV(GPIO_MAJOR, GPIO_MINOR);
    printk(KERN_INFO "DEVICE NO = 0x%x\n", devno);
    register_chrdev_region(devno, 1, GPIO_DEVICE);

    // printk(KERN_INFO "Init gpio_module\n");

    // 2. initialze charater device structure
    cdev_init(&gpio_cdev, &gpio_fops);
    count = 1;
    
    // 3. add character device
    err = cdev_add(&gpio_cdev, devno, count);
    if(err < 0){
    	printk(KERN_INFO "Error: cdev_add()\n");
	return -1;
    }


    printk(KERN_INFO "'mknod /dev/%s c %d 0'\n", GPIO_DEVICE, GPIO_MAJOR);
    printk(KERN_INFO "'chmod 666 /dev/%s'\n", GPIO_DEVICE);

	// gpio_request fuction is defined in gpio.h
	err = gpio_request(GPIO_LED1,"LED");
	if(err == -EBUSY){
		printk(KERN_INFO "Error gpio_request\n");
	}

	err = gpio_request(GPIO_LED2,"LED");
	if(err == -EBUSY){
		printk(KERN_INFO "Error gpio_request\n");
	}

	// get IRQ ID
	switch_irq = gpio_to_irq(GPIO_SW);
	request_irq(switch_irq, isr_func, IRQF_TRIGGER_RISING,"switch", NULL);

	gpio_direction_output(GPIO_LED1, 0);
	gpio_direction_output(GPIO_LED2, 0);
	

   return 0;
}

static int gpio_open(struct inode *inod, struct file *fil)
{
    try_module_get(THIS_MODULE);
    printk(KERN_INFO "GPIO Device Opend()\n");

    return 0;
}

static ssize_t gpio_write(struct file  *inod, const char *buff, size_t len, loff_t *off)
{
    int g;
    short count;

    memset(msg, 0, sizeof(msg));
    count = copy_from_user(msg, buff, len);

    g = simple_strtol(buff, 0, 10);
	switch(g){
		case 0:
			gpio_set_value(GPIO_LED1, 0);
			gpio_set_value(GPIO_LED2, 0);
			break;
		case 1:
			gpio_set_value(GPIO_LED1, 1);
			gpio_set_value(GPIO_LED2, 0);
			break;
		case 2:
			gpio_set_value(GPIO_LED1, 1);
			gpio_set_value(GPIO_LED2, 1);
			break;
		default:
			break;

	}

    printk(KERN_INFO "GPIO Device Write : %d\n", g);

    return count;
}

static ssize_t gpio_read(struct file *inode, char *buff, size_t len, loff_t *off)
{
    int g;
    int results1, results2, count;

    memset(msg, 0, sizeof(msg));
    g = simple_strtol(buff, NULL, 10);
    results1 = gpio_get_value(GPIO_LED1);
	results2 = gpio_get_value(GPIO_LED2);
	
	strcat(msg, " from kernel");
	count = copy_to_user(buff, msg, strlen(msg)+1);

    printk(KERN_INFO "GPIO Device read: Message = %s, GPIO Output Mode: %d, GPIO LED1 Value: %d, GPIO_LED2 Value: %d\n", msg, g, results1, results2);

    return count;
}

static int gpio_close(struct inode *inod, struct file *fil)
{

    module_put(THIS_MODULE);
    printk(KERN_INFO "GPIO Device closed()\n");
    
    return 0;
}

static void __exit cleanupModule(void)
{
    dev_t devno = MKDEV(GPIO_MAJOR, GPIO_MINOR);
    // 1. unregister character device (device no, device name)
    unregister_chrdev_region(devno, 1);
    printk(KERN_INFO "Exit gpio_module\n");

    // 2. remove charcter device structure
    free_irq(switch_irq, NULL);

	cdev_del(&gpio_cdev);
	gpio_direction_output(GPIO_LED1, 0);
	gpio_free(GPIO_LED1);

	gpio_direction_output(GPIO_LED2, 0);
	gpio_free(GPIO_LED2);
	

    // 3. remove virtual address of charater device
    if(gpio)
        iounmap(gpio);

    printk(KERN_INFO "Exit GPIO module\n");
}

// define initialization function name
module_init(initModule);

// define clean up function name
module_exit(cleanupModule);
