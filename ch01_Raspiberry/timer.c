#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/timer.h>
static struct timer_list my_timer;
void my_timer_callback(unsigned long data)
{
	printk("%s called (%ld).\n", __FUNCTION__, jiffies);
}
static int __init my_init(void)
{
	int retval;
	printk("Timer module loaded\n");

	// Step 1. Setting up the timer
	setup_timer(&my_timer, my_timer_callback, 0);
	printk("Setup timer to fire in 300ms (%ld)\n", jiffies);
	
	// Step 2. Setting the expiration time
	retval = mod_timer( &my_timer, jiffies + msecs_to_jiffies(300) );
	if (retval)
		printk("Timer firing failed\n");
	return 0;
}
static void my_exit(void)
{
	int retval;
	
	// Step 3. Releasing the timer
	retval = del_timer(&my_timer);
	/* Is timer still active (1) or no (0) */
	if (retval)
		printk("The timer is still in use...\n");
	pr_info("Timer module unloaded\n");
}
module_init(my_init);
module_exit(my_exit);
MODULE_AUTHOR("John Madieu <john.madieu@gmail.com>");
MODULE_DESCRIPTION("Standard timer example");
MODULE_LICENSE("GPL");
