#include <linux/miscdevice.h>
#include <linux/delay.h>
#include <asm/irq.h>
#include <mach/hardware.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/mm.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/delay.h>
#include <linux/moduleparam.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/ioctl.h>
#include <linux/cdev.h>
#include <linux/string.h>
#include <linux/list.h>
#include <linux/pci.h>
#include <asm/uaccess.h>
#include <asm/atomic.h>
#include <asm/unistd.h>
#include <mach/map.h>
#include <mach/regs-clock.h>
#include <mach/regs-gpio.h>
#include <plat/gpio-cfg.h>
#include <mach/gpio-bank-e.h>
#include <mach/gpio-bank-m.h>
#include <mach/gpio-bank-k.h>

#define DEVICE_NAME "rs485io"

static long s3c6410_485io_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	switch(cmd) {
		unsigned int tmp;
	case 0:
	case 1:
    	if (arg > 1) 
  		{
			return -EINVAL;
		}

		tmp = __raw_readl(S3C64XX_GPKDAT);
//		printk("S3C64XX_GPKDAT = %d\n",tmp);
            
		if(cmd==0) 
       	{ 
			tmp &= (~(1<<5));
      	}
		else  
      	{ 
			tmp |= (1<<5);
     	}

		printk("write tmp = %d\n",tmp);
      	writel(tmp,S3C64XX_GPMDAT);
		tmp = __raw_readl(S3C64XX_GPKDAT);
//		printk("S3C64XX_GPKDAT = %d,cmd=%d\n",tmp,cmd);

		return 0;
	default:
		return -EINVAL;
	}
}

static struct file_operations dev_fops = {
	.owner			= THIS_MODULE,
	.unlocked_ioctl	= s3c6410_485io_ioctl,
};

static struct miscdevice misc = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = DEVICE_NAME,
	.fops = &dev_fops,
};

static int __init rs485io_init(void)
{
	int ret;

	unsigned long tmp;

    //gpk5 pull up
	tmp = __raw_readl(S3C64XX_GPKPUD);
	tmp &= ~(1<<(2*5+1));
	tmp |= 1<<(2*5);
	__raw_writel(tmp,S3C64XX_GPKPUD);

	//gpk5 output mode
	tmp =__raw_readl(S3C64XX_GPKCON);
	tmp &= ~(0xf)<<(5*4);
	tmp |= 1<<(5*4);
	__raw_writel(tmp,S3C64XX_GPKCON);

	//gpk5 output 0
	tmp = __raw_readl(S3C64XX_GPKDAT);
	tmp &= (~(1<<5));
	printk("\n@@@@@@@@@@@\n");
	printk("\ntmp = %lu\n",tmp);
	printk("\n@@@@@@@@@@@\n");
	__raw_writel(tmp,S3C64XX_GPKDAT);  

	ret = misc_register(&misc);

	return ret;
}

static void __exit rs485io_exit(void)
{
	misc_deregister(&misc);
}

module_init(rs485io_init);
module_exit(rs485io_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("FORLINX Inc.");
