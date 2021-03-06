/* linux/arch/arm/mach-s3c64xx/mach-smdk6410.c
 *
 * Copyright 2008 Openmoko, Inc.
 * Copyright 2008 Simtec Electronics
 *	Ben Dooks <ben@simtec.co.uk>
 *	http://armlinux.simtec.co.uk/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
*/

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/list.h>
#include <linux/timer.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/serial_core.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/i2c.h>
#include <linux/leds.h>
#include <linux/fb.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/smsc911x.h>
#include <linux/regulator/fixed.h>
#include <linux/regulator/machine.h>
#include <linux/pwm_backlight.h>

#ifdef CONFIG_SMDK6410_WM1190_EV1
#include <linux/mfd/wm8350/core.h>
#include <linux/mfd/wm8350/pmic.h>
#endif

#ifdef CONFIG_SMDK6410_WM1192_EV1
#include <linux/mfd/wm831x/core.h>
#include <linux/mfd/wm831x/pdata.h>
#endif

#include <video/platform_lcd.h>

#include <asm/mach/arch.h>
#include <asm/mach/map.h>
#include <asm/mach/irq.h>

#include <mach/hardware.h>
#include <mach/regs-fb.h>
#include <mach/map.h>

#include <asm/irq.h>
#include <asm/mach-types.h>

#include <plat/regs-serial.h>
#include <mach/regs-modem.h>
#include <mach/regs-gpio.h>
#include <mach/regs-sys.h>
#include <mach/regs-srom.h>
#include <plat/ata.h>
#include <plat/iic.h>
#include <plat/fb.h>
#include <plat/gpio-cfg.h>

#include <mach/s3c6410.h>
#include <plat/clock.h>
#include <plat/devs.h>
#include <plat/cpu.h>
#include <plat/adc.h>
#include <plat/keypad.h>

#include <linux/gpio_keys.h>
#include <plat/audio.h>
#include <mach/ts.h>
#include <plat/sdhci.h>
#include <plat/regs-usb-hsotg-phy.h>
#include <plat/fimc.h>

#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <plat/nand.h>
#include <linux/dm9000.h>

//for spi  add by fatfish
#include <plat/s3c64xx-spi.h>
#include <linux/spi/spi.h>

//for mcp251x  add by fatfish
#include <linux/can/platform/mcp251x.h>
#include <mach/gpio-bank-c.h>

#define UCON S3C2410_UCON_DEFAULT | S3C2410_UCON_UCLK
#define ULCON S3C2410_LCON_CS8 | S3C2410_LCON_PNONE | S3C2410_LCON_STOPB
#define UFCON S3C2410_UFCON_RXTRIG8 | S3C2410_UFCON_FIFOMODE



extern void s3c64xx_reserve_bootmem(void);

static struct s3c2410_uartcfg smdk6410_uartcfgs[] __initdata = {
	[0] = {
		.hwport	     = 0,
		.flags	     = 0,
		.ucon	     = UCON,
		.ulcon	     = ULCON,
		.ufcon	     = UFCON,
	},
	[1] = {
		.hwport	     = 1,
		.flags	     = 0,
		.ucon	     = UCON,
		.ulcon	     = ULCON,
		.ufcon	     = UFCON,
	},
	[2] = {
		.hwport	     = 2,
		.flags	     = 0,
		.ucon	     = UCON,
		.ulcon	     = ULCON,
		.ufcon	     = UFCON,
	},
	[3] = {
		.hwport	     = 3,
		.flags	     = 0,
		.ucon	     = UCON,
		.ulcon	     = ULCON,
		.ufcon	     = UFCON,
	},
};

/*keypad add by fatfish*/
static uint32_t smdk6410_keymap[] __initdata = {
	/* KEY(row, col, keycode) */
	KEY(0, 3, KEY_1), KEY(0, 4, KEY_2), KEY(0, 5, KEY_3),
	KEY(0, 6, KEY_4), KEY(0, 7, KEY_5),
	KEY(1, 3, KEY_A), KEY(1, 4, KEY_B), KEY(1, 5, KEY_C),
	KEY(1, 6, KEY_D), KEY(1, 7, KEY_E)
};

static struct matrix_keymap_data smdk6410_keymap_data __initdata = {
	.keymap		= smdk6410_keymap,
	.keymap_size	= ARRAY_SIZE(smdk6410_keymap),
};

static struct samsung_keypad_platdata smdk6410_keypad_data __initdata = {
	.keymap_data	= &smdk6410_keymap_data,
	.rows		= 8,
	.cols		= 8,
};
/* Ethernet */
#ifdef CONFIG_DM9000
#define S3C64XX_PA_DM9000	(0x18000000)
#define S3C64XX_SZ_DM9000	SZ_1M
#define S3C64XX_VA_DM9000	S3C_ADDR(0x03b00300)

static struct resource dm9000_resources[] = {
	[0] = {
		.start		= S3C64XX_PA_DM9000,
		.end		= S3C64XX_PA_DM9000 + 3,
		.flags		= IORESOURCE_MEM,
	},
	[1] = {
		.start		= S3C64XX_PA_DM9000 + 4,
		.end		= S3C64XX_PA_DM9000 + S3C64XX_SZ_DM9000 - 1, 
		.flags		= IORESOURCE_MEM,
	},
	[2] = {
		.start		= IRQ_EINT(7),
		.end		= IRQ_EINT(7),
		.flags		= IORESOURCE_IRQ | IRQF_TRIGGER_HIGH,
	},
};

static struct dm9000_plat_data dm9000_setup = {
	.flags			= DM9000_PLATF_16BITONLY,
	.dev_addr		= { 0x08, 0x90, 0x00, 0xa0, 0x90, 0x90 },
};

static struct platform_device s3c_device_dm9000 = {
	.name			= "dm9000",
	.id				= 0,
	.num_resources	= ARRAY_SIZE(dm9000_resources),
	.resource		= dm9000_resources,
	.dev			= {
		.platform_data = &dm9000_setup,
	}
};
#endif //#ifdef CONFIG_DM9000


/*add by fatfish for mcp251x*/
static void  cs_set_level(unsigned line_id, int lvl) {
    gpio_direction_output(line_id, lvl);
};

static struct s3c64xx_spi_csinfo s3c64xx_spi1_csinfo = {
  	.fb_delay=0x3,
  	.line=S3C64XX_GPC(7),
  	.set_level=cs_set_level,
};

static int mcp251x_ioSetup(struct spi_device *spi)
{
	printk(KERN_INFO "mcp251x: setup gpio pins CS and External Int\n");
	s3c_gpio_setpull(S3C64XX_GPL(8), S3C_GPIO_PULL_UP);		// External interrupt from CAN controller
	s3c_gpio_cfgpin(S3C64XX_GPL(8), S3C_GPIO_SFN(3)); 		// External interrupt from CAN controller (hopefully external interrupt)
	//s3c_gpio_cfgpin(S3C64XX_GPL(8), S3C_GPIO_INPUT);		// External interrupt from CAN controller
	s3c_gpio_setpull(S3C64XX_GPC(7), S3C_GPIO_PULL_NONE);	// Manual chip select pin as used in 6410_set_cs
	s3c_gpio_cfgpin(S3C64XX_GPC(7), S3C_GPIO_OUTPUT);		// Manual chip select pin as used in 6410_set_cs
	return 0;
}

static struct mcp251x_platform_data mcp251x_info = {
	.oscillator_frequency = 8000000,
	.board_specific_setup = mcp251x_ioSetup,
	.transceiver_enable = NULL,
	.power_enable = NULL,
};

static struct spi_board_info __initdata forlinx6410_mc251x_info[]  = {
	{
		.modalias = "mcp2515",	
		.platform_data = &mcp251x_info,
		.irq = IRQ_EINT(16),
		.max_speed_hz = 10*1000*1000,	
		.bus_num = 1,
		.chip_select = 0,
		.mode = SPI_MODE_0,	
		.controller_data=&s3c64xx_spi1_csinfo,
	},
};




/* framebuffer and LCD setup. */

/* GPF15 = LCD backlight control
 * GPF13 => Panel power
 * GPN5 = LCD nRESET signal
 * PWM_TOUT1 => backlight brightness
 */

static void smdk6410_lcd_power_set(struct plat_lcd_data *pd,
				   unsigned int power)
{
	if (power) {
		gpio_direction_output(S3C64XX_GPF(13), 1);
		gpio_direction_output(S3C64XX_GPF(15), 1);

		/* fire nRESET on power up */
		gpio_direction_output(S3C64XX_GPN(5), 0);
		msleep(10);
		gpio_direction_output(S3C64XX_GPN(5), 1);
		msleep(1);
	} else {
		gpio_direction_output(S3C64XX_GPF(15), 0);
		gpio_direction_output(S3C64XX_GPF(13), 0);
	}
}

static struct plat_lcd_data smdk6410_lcd_power_data = {
	.set_power	= smdk6410_lcd_power_set,
};

static struct platform_device smdk6410_lcd_powerdev = {
	.name			= "platform-lcd",
	.dev.parent		= &s3c_device_fb.dev,
	.dev.platform_data	= &smdk6410_lcd_power_data,
};

static struct s3c_fb_pd_win smdk6410_fb_win0 = {
	/* this is to ensure we use win0 */
	.win_mode	= {
		.left_margin	= 8,
		.right_margin	= 13,
		.upper_margin	= 7,
		.lower_margin	= 5,
		.hsync_len	= 3,
		.vsync_len	= 1,
		.xres		= 800,
		.yres		= 480,
	},
	.max_bpp	= 32,
	.default_bpp	= 16,
	.virtual_y	= 480 * 2,
	.virtual_x	= 800,
};

/* 405566 clocks per frame => 60Hz refresh requires 24333960Hz clock */
static struct s3c_fb_platdata smdk6410_lcd_pdata __initdata = {
	.setup_gpio	= s3c64xx_fb_gpio_setup_24bpp,
	.win[0]		= &smdk6410_fb_win0,
	.vidcon0	= VIDCON0_VIDOUT_RGB | VIDCON0_PNRMODE_RGB,
	.vidcon1	= VIDCON1_INV_HSYNC | VIDCON1_INV_VSYNC,
};



/* MMC/SD config */
static struct s3c_sdhci_platdata ok6410_hsmmc0_pdata = {
    .max_width      = 4,
    .cd_type        = S3C_SDHCI_CD_INTERNAL,
};

static struct s3c_sdhci_platdata ok6410_hsmmc1_pdata = {
    .max_width      = 4,
    .cd_type        = S3C_SDHCI_CD_PERMANENT,
//    .cd_type        = S3C_SDHCI_CD_GPIO,
//    .ext_cd_gpio    = S3C64XX_GPN(6),
};


/*
 * Configuring Nandflash on SMDK6410
 */
struct mtd_partition ok6410_nand_part[] = {
	{
		.name		= "Bootloader",
		.offset		= 0,
		.size		= (1 * SZ_1M),
		.mask_flags	= MTD_CAP_NANDFLASH,
	},
	{
		.name		= "Kernel",
		.offset		= (1 * SZ_1M),
		.size		= (5*SZ_1M) ,
		.mask_flags	= MTD_CAP_NANDFLASH,
	},
/*
	{
		.name		= "User",
		.offset		= (6 * SZ_1M),
		.size		= (120*SZ_1M) ,
	},
*/
	{
		.name		= "File System",
		.offset		= MTDPART_OFS_APPEND,
		.size		= MTDPART_SIZ_FULL,
	}
};

static struct s3c2410_nand_set ok6410_nand_sets[] = {
	[0] = {
		.name       = "nand",
		.nr_chips   = 1,
		.nr_partitions  = ARRAY_SIZE(ok6410_nand_part),
		.partitions = ok6410_nand_part,
	},
};

static struct s3c2410_platform_nand ok6410_nand_info = {
	.tacls      = 25,
	.twrph0     = 55,
	.twrph1     = 40,
	.nr_sets    = ARRAY_SIZE(ok6410_nand_sets),
	.sets       = ok6410_nand_sets,
};




/*
 * Configuring Ethernet on SMDK6410
 *
 * Both CS8900A and LAN9115 chips share one chip select mediated by CFG6.
 * The constant address below corresponds to nCS1
 *
 *  1) Set CFGB2 p3 ON others off, no other CFGB selects "ethernet"
 *  2) CFG6 needs to be switched to "LAN9115" side
 */

static struct resource smdk6410_smsc911x_resources[] = {
	[0] = {
		.start = S3C64XX_PA_XM0CSN1,
		.end   = S3C64XX_PA_XM0CSN1 + SZ_64K - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = S3C_EINT(10),
		.end   = S3C_EINT(10),
		.flags = IORESOURCE_IRQ | IRQ_TYPE_LEVEL_LOW,
	},
};

static struct smsc911x_platform_config smdk6410_smsc911x_pdata = {
	.irq_polarity  = SMSC911X_IRQ_POLARITY_ACTIVE_LOW,
	.irq_type      = SMSC911X_IRQ_TYPE_OPEN_DRAIN,
	.flags         = SMSC911X_USE_32BIT | SMSC911X_FORCE_INTERNAL_PHY,
	.phy_interface = PHY_INTERFACE_MODE_MII,
};


static struct platform_device smdk6410_smsc911x = {
	.name          = "smsc911x",
	.id            = -1,
	.num_resources = ARRAY_SIZE(smdk6410_smsc911x_resources),
	.resource      = &smdk6410_smsc911x_resources[0],
	.dev = {
		.platform_data = &smdk6410_smsc911x_pdata,
	},
};

#ifdef CONFIG_REGULATOR
static struct regulator_consumer_supply smdk6410_b_pwr_5v_consumers[] = {
	{
		/* WM8580 */
		.supply = "PVDD",
		.dev_name = "0-001b",
	},
	{
		/* WM8580 */
		.supply = "AVDD",
		.dev_name = "0-001b",
	},
};

static struct regulator_init_data smdk6410_b_pwr_5v_data = {
	.constraints = {
		.always_on = 1,
	},
	.num_consumer_supplies = ARRAY_SIZE(smdk6410_b_pwr_5v_consumers),
	.consumer_supplies = smdk6410_b_pwr_5v_consumers,
};

static struct fixed_voltage_config smdk6410_b_pwr_5v_pdata = {
	.supply_name = "B_PWR_5V",
	.microvolts = 5000000,
	.init_data = &smdk6410_b_pwr_5v_data,
	.gpio = -EINVAL,
};

static struct platform_device smdk6410_b_pwr_5v = {
	.name          = "reg-fixed-voltage",
	.id            = -1,
	.dev = {
		.platform_data = &smdk6410_b_pwr_5v_pdata,
	},
};
#endif




static struct s3c_ide_platdata smdk6410_ide_pdata __initdata = {
	.setup_gpio	= s3c64xx_ide_setup_gpio,
};


static int smdk6410_backlight_init(struct device *dev)
{
	int ret;

	ret = gpio_request(S3C64XX_GPF(15), "Backlight");
	if (ret) {
		printk(KERN_ERR "failed to request GPF for PWM-OUT1\n");
		return ret;
	}

	/* Configure GPIO pin with S3C64XX_GPF15_PWM_TOUT1 */
	s3c_gpio_cfgpin(S3C64XX_GPF(15), S3C_GPIO_SFN(2));

	return 0;
}

static void smdk6410_backlight_exit(struct device *dev)
{
	s3c_gpio_cfgpin(S3C64XX_GPF(15), S3C_GPIO_OUTPUT);
	gpio_free(S3C64XX_GPF(15));
}

static struct platform_pwm_backlight_data smdk6410_backlight_data = {
	.pwm_id		= 1,
	.max_brightness	= 255,
	.dft_brightness	= 255,
	.pwm_period_ns	= 78770,
	.init		= smdk6410_backlight_init,
	.exit		= smdk6410_backlight_exit,
};

static struct platform_device smdk6410_backlight_device = {
	.name		= "pwm-backlight",
	.dev		= {
		.parent		= &s3c_device_timer[1].dev,
		.platform_data	= &smdk6410_backlight_data,
	},
};

static struct map_desc smdk6410_iodesc[] = {
	{
		/* LCD support */
		.virtual    = (unsigned long)S3C_VA_LCD,
		.pfn        = __phys_to_pfn(S3C_PA_FB),
		.length     = SZ_16K,
		.type       = MT_DEVICE,
	},
};

/* gpio buttons */
static struct gpio_keys_button gpio_buttons[] = {
	{
		.gpio		= S3C64XX_GPN(0),
		//.code		= 25,
		.code		= KEY_UP,
		.desc		= "BUTTON1",
		.active_low	= 1,
		.wakeup		= 0,
	},
	{
		.gpio		= S3C64XX_GPN(1),
		//.code		= 42,
		.code		= KEY_DOWN,
		.desc		= "BUTTON2",
		.active_low	= 1,
		.wakeup		= 0,
	},
	{
		.gpio		= S3C64XX_GPN(2),
		//.code		= 50,
		.code		= KEY_LEFT,
		.desc		= "BUTTON3",
		.active_low	= 1,
		.wakeup		= 0,
	},
	{
		.gpio		= S3C64XX_GPN(3),
		//.code		= 10,
		.code		= KEY_RIGHT,
		.desc		= "BUTTON4",
		.active_low	= 1,
		.wakeup		= 0,
	},
	{
		.gpio		= S3C64XX_GPN(4),
		//.code		= 24,
		.code		= KEY_ENTER,
		.desc		= "BUTTON5",
		.active_low	= 1,
		.wakeup		= 0,
	},
	{
		.gpio		= S3C64XX_GPN(5),
		//.code		= 38,
		.code		= KEY_ESC,
		.desc		= "BUTTON6",
		.active_low	= 1,
		.wakeup		= 0,
	}
};

static struct gpio_keys_platform_data gpio_button_data = {
	.buttons	= gpio_buttons,
	.nbuttons	= ARRAY_SIZE(gpio_buttons),
};

static struct platform_device gpio_button_device = {
	.name		= "gpio-keys",
	.id		= -1,
	.num_resources	= 0,
	.dev		= {
		.platform_data	= &gpio_button_data,
	}
};


static struct platform_device *smdk6410_devices[] __initdata = {



/*#ifdef CONFIG_SMDK6410_SD_CH0
	&s3c_device_hsmmc0,
#endif
#ifdef CONFIG_SMDK6410_SD_CH1
	&s3c_device_hsmmc1,
#endif  */

	&s3c_device_hsmmc0,
	&s3c_device_hsmmc1,

	&s3c_device_i2c0,
	//&s3c_device_i2c1,
	&s3c_device_fb,
	&s3c_device_ohci,
	&s3c_device_usb_hsotg,
	&samsung_asoc_dma,

	&samsung_device_keypad,


#ifdef CONFIG_REGULATOR
	//&smdk6410_b_pwr_5v,
#endif
	//&smdk6410_lcd_powerdev,

	//&smdk6410_smsc911x,
	//&s3c_device_adc,
	&gpio_button_device,

	&s3c_device_nand,   // gjl 

#ifdef CONFIG_DM9000
	&s3c_device_dm9000,
#endif

	&s3c64xx_device_ac97,

	//&s3c_device_cfcon,
	&s3c_device_rtc,

	&s3c_device_ts,

	
	&s3c_device_fimc0,
	&s3c_device_fimc1,

        &s3c_device_wdt, //dg add 2012-03-26

	//&s3c_device_timer[1],
	// gjl &smdk6410_backlight_device,

	// cym add
        &s3c_device_vpp,
	&s3c_device_mfc,
	&s3c_device_tvenc,
	&s3c_device_tvscaler,
	&s3c_device_rotator,
	&s3c_device_jpeg,
//	&s3c_device_fimc0,
//	&s3c_device_fimc1,
	&s3c_device_g2d,
	&s3c_device_g3d,


	/*add by fatfish*/
	&s3c64xx_device_spi0,
	&s3c64xx_device_spi1,
};

#ifdef CONFIG_REGULATOR
/* ARM core */
static struct regulator_consumer_supply smdk6410_vddarm_consumers[] = {
	{
		.supply = "vddarm",
	}
};

/* VDDARM, BUCK1 on J5 */
static struct regulator_init_data smdk6410_vddarm = {
	.constraints = {
		.name = "PVDD_ARM",
		.min_uV = 1000000,
		.max_uV = 1300000,
		.always_on = 1,
		.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE,
	},
	.num_consumer_supplies = ARRAY_SIZE(smdk6410_vddarm_consumers),
	.consumer_supplies = smdk6410_vddarm_consumers,
};

/* VDD_INT, BUCK2 on J5 */
static struct regulator_init_data smdk6410_vddint = {
	.constraints = {
		.name = "PVDD_INT",
		.min_uV = 1000000,
		.max_uV = 1200000,
		.always_on = 1,
		.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE,
	},
};

/* VDD_HI, LDO3 on J5 */
static struct regulator_init_data smdk6410_vddhi = {
	.constraints = {
		.name = "PVDD_HI",
		.always_on = 1,
	},
};

/* VDD_PLL, LDO2 on J5 */
static struct regulator_init_data smdk6410_vddpll = {
	.constraints = {
		.name = "PVDD_PLL",
		.always_on = 1,
	},
};

/* VDD_UH_MMC, LDO5 on J5 */
static struct regulator_init_data smdk6410_vdduh_mmc = {
	.constraints = {
		.name = "PVDD_UH+PVDD_MMC",
		.always_on = 1,
	},
};

/* VCCM3BT, LDO8 on J5 */
static struct regulator_init_data smdk6410_vccmc3bt = {
	.constraints = {
		.name = "PVCCM3BT",
		.always_on = 1,
	},
};

/* VCCM2MTV, LDO11 on J5 */
static struct regulator_init_data smdk6410_vccm2mtv = {
	.constraints = {
		.name = "PVCCM2MTV",
		.always_on = 1,
	},
};

/* VDD_LCD, LDO12 on J5 */
static struct regulator_init_data smdk6410_vddlcd = {
	.constraints = {
		.name = "PVDD_LCD",
		.always_on = 1,
	},
};

/* VDD_OTGI, LDO9 on J5 */
static struct regulator_init_data smdk6410_vddotgi = {
	.constraints = {
		.name = "PVDD_OTGI",
		.always_on = 1,
	},
};

/* VDD_OTG, LDO14 on J5 */
static struct regulator_init_data smdk6410_vddotg = {
	.constraints = {
		.name = "PVDD_OTG",
		.always_on = 1,
	},
};

/* VDD_ALIVE, LDO15 on J5 */
static struct regulator_init_data smdk6410_vddalive = {
	.constraints = {
		.name = "PVDD_ALIVE",
		.always_on = 1,
	},
};

/* VDD_AUDIO, VLDO_AUDIO on J5 */
static struct regulator_init_data smdk6410_vddaudio = {
	.constraints = {
		.name = "PVDD_AUDIO",
		.always_on = 1,
	},
};
#endif

#ifdef CONFIG_SMDK6410_WM1190_EV1
/* S3C64xx internal logic & PLL */
static struct regulator_init_data wm8350_dcdc1_data = {
	.constraints = {
		.name = "PVDD_INT+PVDD_PLL",
		.min_uV = 1200000,
		.max_uV = 1200000,
		.always_on = 1,
		.apply_uV = 1,
	},
};

/* Memory */
static struct regulator_init_data wm8350_dcdc3_data = {
	.constraints = {
		.name = "PVDD_MEM",
		.min_uV = 1800000,
		.max_uV = 1800000,
		.always_on = 1,
		.state_mem = {
			 .uV = 1800000,
			 .mode = REGULATOR_MODE_NORMAL,
			 .enabled = 1,
		},
		.initial_state = PM_SUSPEND_MEM,
	},
};

/* USB, EXT, PCM, ADC/DAC, USB, MMC */
static struct regulator_consumer_supply wm8350_dcdc4_consumers[] = {
	{
		/* WM8580 */
		.supply = "DVDD",
		.dev_name = "0-001b",
	},
};

static struct regulator_init_data wm8350_dcdc4_data = {
	.constraints = {
		.name = "PVDD_HI+PVDD_EXT+PVDD_SYS+PVCCM2MTV",
		.min_uV = 3000000,
		.max_uV = 3000000,
		.always_on = 1,
	},
	.num_consumer_supplies = ARRAY_SIZE(wm8350_dcdc4_consumers),
	.consumer_supplies = wm8350_dcdc4_consumers,
};

/* OTGi/1190-EV1 HPVDD & AVDD */
static struct regulator_init_data wm8350_ldo4_data = {
	.constraints = {
		.name = "PVDD_OTGI+HPVDD+AVDD",
		.min_uV = 1200000,
		.max_uV = 1200000,
		.apply_uV = 1,
		.always_on = 1,
	},
};

static struct {
	int regulator;
	struct regulator_init_data *initdata;
} wm1190_regulators[] = {
	{ WM8350_DCDC_1, &wm8350_dcdc1_data },
	{ WM8350_DCDC_3, &wm8350_dcdc3_data },
	{ WM8350_DCDC_4, &wm8350_dcdc4_data },
	{ WM8350_DCDC_6, &smdk6410_vddarm },
	{ WM8350_LDO_1, &smdk6410_vddalive },
	{ WM8350_LDO_2, &smdk6410_vddotg },
	{ WM8350_LDO_3, &smdk6410_vddlcd },
	{ WM8350_LDO_4, &wm8350_ldo4_data },
};

static int __init smdk6410_wm8350_init(struct wm8350 *wm8350)
{
	int i;

	/* Configure the IRQ line */
	s3c_gpio_setpull(S3C64XX_GPN(12), S3C_GPIO_PULL_UP);

	/* Instantiate the regulators */
	for (i = 0; i < ARRAY_SIZE(wm1190_regulators); i++)
		wm8350_register_regulator(wm8350,
					  wm1190_regulators[i].regulator,
					  wm1190_regulators[i].initdata);

	return 0;
}

static struct wm8350_platform_data __initdata smdk6410_wm8350_pdata = {
	.init = smdk6410_wm8350_init,
	.irq_high = 1,
	.irq_base = IRQ_BOARD_START,
};
#endif

#ifdef CONFIG_SMDK6410_WM1192_EV1
static struct gpio_led wm1192_pmic_leds[] = {
	{
		.name = "PMIC:red:power",
		.gpio = GPIO_BOARD_START + 3,
		.default_state = LEDS_GPIO_DEFSTATE_ON,
	},
};

static struct gpio_led_platform_data wm1192_pmic_led = {
	.num_leds = ARRAY_SIZE(wm1192_pmic_leds),
	.leds = wm1192_pmic_leds,
};

static struct platform_device wm1192_pmic_led_dev = {
	.name          = "leds-gpio",
	.id            = -1,
	.dev = {
		.platform_data = &wm1192_pmic_led,
	},
};

static int wm1192_pre_init(struct wm831x *wm831x)
{
	int ret;

	/* Configure the IRQ line */
	s3c_gpio_setpull(S3C64XX_GPN(12), S3C_GPIO_PULL_UP);

	ret = platform_device_register(&wm1192_pmic_led_dev);
	if (ret != 0)
		dev_err(wm831x->dev, "Failed to add PMIC LED: %d\n", ret);

	return 0;
}

static struct wm831x_backlight_pdata wm1192_backlight_pdata = {
	.isink = 1,
	.max_uA = 27554,
};

static struct regulator_init_data wm1192_dcdc3 = {
	.constraints = {
		.name = "PVDD_MEM+PVDD_GPS",
		.always_on = 1,
	},
};

static struct regulator_consumer_supply wm1192_ldo1_consumers[] = {
	{ .supply = "DVDD", .dev_name = "0-001b", },   /* WM8580 */
};

static struct regulator_init_data wm1192_ldo1 = {
	.constraints = {
		.name = "PVDD_LCD+PVDD_EXT",
		.always_on = 1,
	},
	.consumer_supplies = wm1192_ldo1_consumers,
	.num_consumer_supplies = ARRAY_SIZE(wm1192_ldo1_consumers),
};

static struct wm831x_status_pdata wm1192_led7_pdata = {
	.name = "LED7:green:",
};

static struct wm831x_status_pdata wm1192_led8_pdata = {
	.name = "LED8:green:",
};

static struct wm831x_pdata smdk6410_wm1192_pdata = {
	.pre_init = wm1192_pre_init,
	.irq_base = IRQ_BOARD_START,

	.backlight = &wm1192_backlight_pdata,
	.dcdc = {
		&smdk6410_vddarm,  /* DCDC1 */
		&smdk6410_vddint,  /* DCDC2 */
		&wm1192_dcdc3,
	},
	.gpio_base = GPIO_BOARD_START,
	.ldo = {
		 &wm1192_ldo1,        /* LDO1 */
		 &smdk6410_vdduh_mmc, /* LDO2 */
		 NULL,                /* LDO3 NC */
		 &smdk6410_vddotgi,   /* LDO4 */
		 &smdk6410_vddotg,    /* LDO5 */
		 &smdk6410_vddhi,     /* LDO6 */
		 &smdk6410_vddaudio,  /* LDO7 */
		 &smdk6410_vccm2mtv,  /* LDO8 */
		 &smdk6410_vddpll,    /* LDO9 */
		 &smdk6410_vccmc3bt,  /* LDO10 */
		 &smdk6410_vddalive,  /* LDO11 */
	},
	.status = {
		&wm1192_led7_pdata,
		&wm1192_led8_pdata,
	},
};
#endif

static struct i2c_board_info i2c_devs0[] __initdata = {
	{ I2C_BOARD_INFO("24c08", 0x50), },
	{ I2C_BOARD_INFO("wm8580", 0x1b), },

	{ I2C_BOARD_INFO("ov965x", 0x30), }, // gjl

#ifdef CONFIG_SMDK6410_WM1192_EV1
	{ I2C_BOARD_INFO("wm8312", 0x34),
	  .platform_data = &smdk6410_wm1192_pdata,
	  .irq = S3C_EINT(12),
	},
#endif

#ifdef CONFIG_SMDK6410_WM1190_EV1
	{ I2C_BOARD_INFO("wm8350", 0x1a),
	  .platform_data = &smdk6410_wm8350_pdata,
	  .irq = S3C_EINT(12),
	},
#endif
};

static struct i2c_board_info i2c_devs1[] __initdata = {
	{ I2C_BOARD_INFO("24c128", 0x57), },	/* Samsung S524AD0XD1 */
};

/*
static struct s3c2410_ts_mach_info s3c_ts_platform __initdata = {
	.delay			= 10000,
	.presc			= 49,
	.oversampling_shift	= 2,
};*/

static struct s3c_ts_mach_info s3c_ts_platform __initdata = {
	.delay 			= 10000,
	.presc 			= 49,
	.oversampling_shift	= 2,
	.resol_bit 		= 12,
	.s3c_adc_con		= ADC_TYPE_2,
};


#ifdef CONFIG_USB_SUPPORT
/* Initializes OTG Phy. to output 48M clock */
void s3c_otg_phy_config(int enable) {
	u32 val;

	if (enable) {
		__raw_writel(0x0, S3C_PHYPWR);	/* Power up */

		val = __raw_readl(S3C_PHYCLK);
		val &= ~S3C_PHYCLK_CLKSEL_MASK;
		__raw_writel(val, S3C_PHYCLK);

		__raw_writel(0x1, S3C_RSTCON);
		udelay(5);
		__raw_writel(0x0, S3C_RSTCON);	/* Finish the reset */
		udelay(5);
	} else {
		__raw_writel(0x19, S3C_PHYPWR);	/* Power down */
	}
}
EXPORT_SYMBOL(s3c_otg_phy_config);
#endif


static void __init smdk6410_map_io(void)
{
	u32 tmp;

	s3c64xx_init_io(smdk6410_iodesc, ARRAY_SIZE(smdk6410_iodesc));
	s3c24xx_init_clocks(12000000);
	s3c24xx_init_uarts(smdk6410_uartcfgs, ARRAY_SIZE(smdk6410_uartcfgs));

	/* set the LCD type */

	tmp = __raw_readl(S3C64XX_SPCON);
	tmp &= ~S3C64XX_SPCON_LCD_SEL_MASK;
	tmp |= S3C64XX_SPCON_LCD_SEL_RGB;
	__raw_writel(tmp, S3C64XX_SPCON);

	/* remove the lcd bypass */
	tmp = __raw_readl(S3C64XX_MODEM_MIFPCON);
	tmp &= ~MIFPCON_LCD_BYPASS;
	__raw_writel(tmp, S3C64XX_MODEM_MIFPCON);

#ifdef CONFIG_VIDEO_SAMSUNG
	s3c64xx_reserve_bootmem();
#endif
}

static void __init smdk6410_machine_init(void)
{
	u32 cs1;

	s3c_i2c0_set_platdata(NULL);
	//s3c_i2c1_set_platdata(NULL);
	s3c_fb_set_platdata(&smdk6410_lcd_pdata);


	s3c_nand_set_platdata(&ok6410_nand_info);  // gjl 

	s3c_sdhci0_set_platdata(&ok6410_hsmmc0_pdata);
	s3c_sdhci1_set_platdata(&ok6410_hsmmc1_pdata);


	samsung_keypad_set_platdata(&smdk6410_keypad_data);


//	s3c24xx_ts_set_platdata(&s3c_ts_platform);
	s3c_ts_set_platdata(&s3c_ts_platform);

	s3c64xx_ac97_setup_gpio(0);

	/* configure nCS1 width to 16 bits */

	cs1 = __raw_readl(S3C64XX_SROM_BW) &
		    ~(S3C64XX_SROM_BW__CS_MASK << S3C64XX_SROM_BW__NCS1__SHIFT);
	cs1 |= ((1 << S3C64XX_SROM_BW__DATAWIDTH__SHIFT) |
		(1 << S3C64XX_SROM_BW__WAITENABLE__SHIFT) |
		(1 << S3C64XX_SROM_BW__BYTEENABLE__SHIFT)) <<
						   S3C64XX_SROM_BW__NCS1__SHIFT;
	__raw_writel(cs1, S3C64XX_SROM_BW);

	/* set timing for nCS1 suitable for ethernet chip */

	__raw_writel((0 << S3C64XX_SROM_BCX__PMC__SHIFT) |
		     (6 << S3C64XX_SROM_BCX__TACP__SHIFT) |
		     (4 << S3C64XX_SROM_BCX__TCAH__SHIFT) |
		     (1 << S3C64XX_SROM_BCX__TCOH__SHIFT) |
		     (0xe << S3C64XX_SROM_BCX__TACC__SHIFT) |
		     (4 << S3C64XX_SROM_BCX__TCOS__SHIFT) |
		     (0 << S3C64XX_SROM_BCX__TACS__SHIFT), S3C64XX_SROM_BC1);

	//gpio_request(S3C64XX_GPN(5), "LCD power");
	//gpio_request(S3C64XX_GPF(13), "LCD power");

	i2c_register_board_info(0, i2c_devs0, ARRAY_SIZE(i2c_devs0));
	i2c_register_board_info(1, i2c_devs1, ARRAY_SIZE(i2c_devs1));

	//s3c_ide_set_platdata(&smdk6410_ide_pdata);



#ifdef CONFIG_S3C64XX_DEV_FIMC0
	s3c_fimc0_set_platdata(NULL);
#endif
#ifdef CONFIG_S3C64XX_DEV_FIMC1
	s3c_fimc1_set_platdata(NULL);
#endif
	
	s3c64xx_spi_set_info(0,0,1);
	s3c64xx_spi_set_info(1,0,1);
	spi_register_board_info(forlinx6410_mc251x_info,ARRAY_SIZE(forlinx6410_mc251x_info));
	samsung_keypad_set_platdata(&smdk6410_keypad_data);
	platform_add_devices(smdk6410_devices, ARRAY_SIZE(smdk6410_devices));
}

MACHINE_START(SMDK6410, "SMDK6410")
	/* Maintainer: Ben Dooks <ben-linux@fluff.org> */
	//.phys_io	= S3C_PA_UART & 0xfff00000,
	//.io_pg_offst	= (((u32)S3C_VA_UART) >> 18) & 0xfffc,
	.boot_params	= S3C64XX_PA_SDRAM + 0x100,

	.init_irq	= s3c6410_init_irq,
	.map_io		= smdk6410_map_io,
	.init_machine	= smdk6410_machine_init,
	.timer		= &s3c24xx_timer,
MACHINE_END

