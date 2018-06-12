#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/module.h>
#include <linux/io.h>
#include <linux/uaccess.h>

#define GPIO_MAJOR 200
#define GPIO_MINOR 0
#define GPIO_DEVICE "gpioled"

// Raspberry PI 3 Physical I/O PERI뮨ㄸADDR
#define BCM_IO_BASE 0x3F000000
#define GPIO_BASE (BCM_IO_BASE + 0x20000000)
#define GPIO_SIZE 0xB4

#define GPIO_IN(g)  (*(gpio+((g)/10)) &= (1<<(((g)%10)*3)))
#define GPIO_OUT(g) (*(gpio+((g)/10)) |= (1<<(((g)%10)*3)))

#define GPIO_SET(g) (*(gpio+ 7) = (1<<g))
#define GPIO_CLR(g) (*(gpio+10) = (1<<g))
#define GPIO_GET(g) (*(gpio+13) & (1<<g))
#define GPIO_LED1 17
#define GPIO_LED2 27

#define BUFFER_SIZE 100

static char msg[BUFFER_SIZE] = {0};

MODULE_LICENSE("GPL");
MODULE_AUTHOR("YW KIM");
MODULE_DESCRIPTION("Raspberry Pi First Device Driver");

static int gpio_open(struct inode *inod, struct file *fil);
static int gpio_close(struct inode *inod, struct file *fil);
static ssize_t gpio_write(struct file *inod, const char *buff, size_t len, loff_t *off);
static ssize_t gpio_read(struct file *inode, char *buff, size_t len, loff_t *off);

struct cdev gpio_cdev;
static struct file_operations gpio_fops = {
	.owner = THIS_MODULE,
    .read = gpio_read,
    .write = gpio_write,
	.open = gpio_open,
	.release = gpio_close
}; 

volatile unsigned int *gpio;

static int __init initModule(void)
{
    dev_t devno;
    unsigned int count;
    static void *map;
    int err;

    // checking function called

    // 1. register charater device
    devno = MKDEV(GPIO_MAJOR, GPIO_MINOR);
    printk(KERN_INFO "DEVICE NO = 0x%x\n", devno);
    register_chrdev_region(devno, 1,GPIO_DEVICE);

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

    printk(KERN_INFO "'mknode /dev/%s c %d 0'\n", GPIO_DEVICE, GPIO_MAJOR);
    printk(KERN_INFO "'chmod 666 /dev/%s'\n", GPIO_DEVICE);

    // 4. return physical memory address
    map = ioremap(GPIO_BASE, GPIO_SIZE);
    if(!map){
	    printk(KERN_INFO "Error : mapping GPIO memory\n");
	    iounmap(map);
	    return -EBUSY;
    }

   gpio = (volatile unsigned int *)map;
   GPIO_OUT(GPIO_LED1);
   GPIO_OUT(GPIO_LED2);
    
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
    // GPIO_SET(g);
    GPIO_SET(17);

    printk(KERN_INFO "GPIO Device Write : %s %d\n", msg, g);

    return count;
}

static ssize_t gpio_read(struct file *inode, char *buff, size_t len, loff_t *off)
{
    int count;
    // long g;
    int results;

    // memset(msg, 0, sizeof(msg));
    // g = simple_strtol(buff, NULL, 10);
    strcat(msg, " from kernel");

    // count = copy_to_user(buff, msg, strlen(msg)+1);

    // results = GPIO_GET(g);
    printk(KERN_INFO "GPIO Device read: msg = %s\n", msg);

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
    cdev_del(&gpio_cdev);

    // 3. remove virtual address of charater device
    if(gpio)
        iounmap(gpio);

    printk(KERN_INFO "Exit GPIO module\n");
}

// define initialization function name
module_init(initModule);

// define clean up function name
module_exit(cleanupModule);
