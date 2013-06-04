/*
 * Copyright (C) 2013 Jeff Kent <jeff@jkent.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <stdbool.h>
#include <stdio.h>
#include "asm/io.h"
#include "mach/udc.h"

#include "timer.h"
#include "udc.h"
#include "udc_driver.h"


bool timeout_aborted = 0;


static void try_usb(void)
{
	void __iomem *udc = (void __iomem *) UDC_BASE;

	/* clock and power for udc block */
	writel(UDC_CLKGEN_CLKSRC_EXT | UDC_CLKGEN_CLKDIV(0),
			udc + UDC_CLKGEN);
	writel(UDC_CLKENB_PCLK_ALWAYS | UDC_CLKENB_CLKGENENB
			| UDC_CLKENB_USBD_ALWAYS, udc + UDC_CLKENB);
	writew(UDC_USER1_VBUSENB, udc + UDC_USER1);

	/* check if VBUS is powered */
	if (readw(udc + UDC_TR) & UDC_TR_VBUS) {
		puts("Detected VBUS power, waiting...");
		timer_init();
		udc_init(&udc_driver);
		while (timeout_aborted || msecs < 2000) {
			udc_task();
			timer_task();
		}
		puts("Timeout");
	}

	/* turn off power and clock to udc block */
	writew(0, udc + UDC_USER1);
	writel(UDC_CLKENB_USBD_ALWAYS, udc + UDC_CLKENB);
	writel(0, udc + UDC_CLKGEN);
}

int main(void)
{
	try_usb();

	puts("Normal boot...");

	halt();
}
