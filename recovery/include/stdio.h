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

#ifndef __STDIO_H
#define __STDIO_H

#include <stdio.h>
#include "config.h"

#ifdef CONFIG_UART_STDIO
# include "uart.h"
#endif

#undef FILE
#undef stdin
#undef stdout
#undef stderr
#define FILE void
#define stdin (void *)0
#define stdout (void *)1
#define stderr (void *)2

#undef printf
int printf (const char *, ...);

#ifdef CONFIG_UART_STDIO
# undef fputc
# undef fputs
# define fputc(c, stream) uart_putchar((char)c)
# define fputs(s, stream) uart_write(s)
#endif

#endif /* __STDIO_H */
