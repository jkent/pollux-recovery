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

#include "config.h"

.section .text
.arm
.align 0

.extern _early_end

.global _entry
.org _entry

_entry:

_start:
	/* enable ICache */
	mrc     p15, 0, r0, c1, c0, 0
        orr     r0, r0, #(1 << 12)
        mcr     p15, 0, r0, c1, c0, 0

	/* setup stack */
	ldr	sp, =_stack_top

	/* load code */
#if defined(CONFIG_START_FROM_NAND)
	ldr	r0, =_reloc_start
	blx	nand_startup
#elif defined(CONFIG_START_FROM_UART)
	ldr	r0, =_reloc_start
	blx	uart_startup
#else /* relocate */
	adr	r0, _entry
	ldr	r1, =_early_end
	add	r0, r0, r1
	ldr	r1, =_reloc_start
	ldr	r2, =_reloc_end
1:
	ldmia	r0!, {r4-r11}
	stmia	r1!, {r4-r11}
	cmp	r1, r2
	bne	1b
#endif
	/* invalidate ICache */
	mov	r0, #0
	mcr	p15, 0, r0, c7, c5, 0

	/* clear .bss */
	ldr     r0, =_bss_start
	ldr     r1, =_bss_end
	mov     r2, #0
1:      cmp     r0, r1
	strlo   r2, [r0], #4
	blo     1b

	/* jump to main */
	ldr	r0, =main
	blx	r0
	ldr	r0, =halt
	blx	r0

.end
