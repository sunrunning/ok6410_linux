#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/miscdevice.h>
#include <linux/delay.h>
#include <asm/irq.h>
#include <mach/hardware.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/gpio.h>
#include <plat/gpio-cfg.h>
#include <mach/regs-gpio.h>
#define DEVICE_NAME	"irda"
#define irda_MAJOR  239

void open_init (void)
{      
	s3c_gpio_cfgpin(S3C64XX_GPE(1), 0x0);
} 

static ssize_t  s3c6410_irda_read(struct file *filp, char *buf, size_t len, loff_t *off)
{
	*buf = gpio_get_value(S3C64XX_GPE(1));
	return 1;
}

static int s3c6410_irda_open(struct inode *inode, struct file *filp)
{
	open_init();
	return 0;
}

static struct file_operations s3c6410_irda_fops = 
{
	.owner	=	THIS_MODULE,
	.read	=	s3c6410_irda_read,
	.open	=	s3c6410_irda_open,
};

static struct cdev cdev_irda;

static int __init s3c6410_irda_init(void)
{
	int result;
	dev_t devno = MKDEV(irda_MAJOR,0);
	struct class *irda_class;

	result = register_chrdev_region(devno,1,DEVICE_NAME);

	if(result)
	{
		printk(KERN_NOTICE "Error %d register irda",result);
		return result;
	}

    
 	cdev_init(&cdev_irda,&s3c6410_irda_fops);

	result = cdev_add(&cdev_irda,devno,1);
	if(result)
	{
		printk(KERN_NOTICE "Error %d adding irda",result);
		return result;
	}

	irda_class = class_create(THIS_MODULE, "irda_class");
	device_create(irda_class, NULL, devno, "irda","irda%d", 0);

	printk ("\n@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
	printk (DEVICE_NAME"\tinitialized\n");
	printk ("\n@@@@@@@@@@@@@@@@@@@@@@@@@@\n");

	return 0;	
}

static void __exit s3c6410_irda_exit(void)
{
	cdev_del(&cdev_irda);
	unregister_chrdev_region(MKDEV(irda_MAJOR,0),1);
}


module_init(s3c6410_irda_init);
module_exit(s3c6410_irda_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("FORLINX Inc.");
