/* linux/arch/arm/plat-s3c64xx/setup-fb-24bpp.c
 *
 * Copyright 2008 Openmoko, Inc.
 * Copyright 2008 Simtec Electronics
 *	Ben Dooks <ben@simtec.co.uk>
 *	http://armlinux.simtec.co.uk/
 *
 * Base S3C64XX setup information for 24bpp LCD framebuffer
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/fb.h>
#include <linux/gpio.h>

#include <plat/fb.h>
#include <plat/gpio-cfg.h>
#include <mach/gpio-samsung.h>

void s3c64xx_fb_gpio_setup_24bpp(void)
{
	int err;
  
	s3c_gpio_cfgrange_nopull(S3C64XX_GPI(0), 16, S3C_GPIO_SFN(2));
	s3c_gpio_cfgrange_nopull(S3C64XX_GPJ(0), 12, S3C_GPIO_SFN(2));
		
	if (gpio_is_valid(S3C64XX_GPE(0))) {
		err = gpio_request(S3C64XX_GPE(0), "GPE");

		if (err) {
			printk(KERN_ERR "failed to request GPE for "
				"lcd reset control\n");
			return;
		}

		gpio_direction_output(S3C64XX_GPE(0), 1);
	}
	gpio_set_value(S3C64XX_GPE(0), 1);
	gpio_free(S3C64XX_GPE(0));
}
