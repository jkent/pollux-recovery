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

#include "asm/io.h"
#include "mach/timer.h"

static void __iomem *timer0 = (void __iomem *) TIMER0_BASE;
unsigned int msecs;

void timer_init(void)
{
	msecs = 0;

	writel(TIMER_CLKENB_TCLKMODE | TIMER_CLKENB_CLKGENENB,
			timer0 + TIMER_CLKENB);
	writel(TIMER_CLKGEN_CLKDIV(146) | TIMER_CLKGEN_CLKSRCSEL_PLL1,
			timer0 + TIMER_CLKGEN);
	writel(TIMER_CONTROL_RUN, timer0 + TIMER_CONTROL);
	writel(0, timer0 + TIMER_COUNT);
	writel(999, timer0 + TIMER_MATCH);
	writel(TIMER_CONTROL_SELTCLK_1 | TIMER_CONTROL_RUN,
			timer0 + TIMER_CONTROL);
}

void timer_task(void)
{
	u32 tmp = readl(timer0 + TIMER_CONTROL);
	if (tmp & TIMER_CONTROL_INTPEND) {
		writel(tmp, timer0 + TIMER_CONTROL);
		msecs++;
	}
}
