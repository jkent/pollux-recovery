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

#include "asm/types.h"
#include "asm/io.h"
#include "mach/uart.h"
#include "mach/gpio.h"

#include "uart.h"

static void __iomem *uart = (void __iomem *) UART0_BASE;

/*
 * This is a minimal UART initialization.  We assume that PLL1 is set to the
 * default 147.461538 MHz and the UART registers are sane.
 */
void uart_init(void)
{
	void __iomem *gpioa = (void __iomem *) GPIOA_BASE;
	u32 tmp;

	/* Configure clock */
	writel(UART_CLKGEN_CLKSRCSEL_PLL1 | UART_CLKGEN_CLKDIV(39),
			uart + UART_CLKGEN);

	/* Set baudrate generator to 115200 */
	writew(1, uart + UART_BRD);

	/* Set GPIO as TX */
	tmp = readl(gpioa + GPIO_ALTFNL);
	tmp &= ~(0x3 << (8 * 2));
	tmp |= (GPIO_ALTFN_ALTFN1 << (8 * 2));
	writel(tmp, gpioa + GPIO_ALTFNL);

	/* Enable TX and RX */
	tmp = readw(uart + UART_UCON);
	tmp &= ~(UART_UCON_TRANSMODE_MASK | UART_UCON_RECVMODE_MASK);
	tmp |= UART_UCON_TRANSMODE_INTPOLL | UART_UCON_RECVMODE_INTPOLL;
	writew((u16) tmp, uart + UART_UCON);
}

int uart_getchar(void)
{
	while (!(readw(uart + UART_TRSTATUS) & UART_TRSTATUS_RXREADY));
	return readw(uart + UART_RHB) & 0xFF;
}

int uart_putchar(int c)
{
	while (!(readw(uart + UART_TRSTATUS) & UART_TRSTATUS_TXEMPTY));
	writew((u16) c & 0xFF, uart + UART_THB);
	return 0;
}

int uart_write(const char *s)
{
	while (*s)
		uart_putchar(*s++);
	return 0;
}

int uart_puts(const char *s)
{
	uart_write(s);
	uart_putchar('\r');
	uart_putchar('\n');
	return 0;
}
