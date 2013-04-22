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

#define CR_M  (1 <<  0)
#define CR_C  (1 <<  2)
#define CR_I  (1 << 12)

.section .text

.align 2
.thumb

.global _thumb_enable_cache
_thumb_enable_cache:
	ldr	r1, =_enable_cache
	bx	r1

.global _thumb_disable_cache
_thumb_disable_cache:
	ldr	r1, =_disable_cache
	bx	r1

.global _thumb_assign_tlb
_thumb_assign_tlb:
	ldr	r1, =_assign_tlb
	bx	r1

.align 4
.arm

.global _enable_cache
_enable_cache:
	/* invalidate ICache */
	mcr	p15, 0, r0, c7, c5, 0
	/* test, clean, and invalidate DCache */
1:	mrc	p15, 0, r15, c7, c14, 3
	bne	1b

	ldr	r1, =(CR_M | CR_C | CR_I)
	mrc	p15, 0, r0, c1, c0, 0
	orr	r0, r0, r1
	mcr	p15, 0, r0, c1, c0, 0
	bx	lr

.global _disable_cache
_disable_cache:
	/* test, clean, and invalidate DCache */
1:	mrc	p15, 0, r15, c7, c14, 3
	bne	1b

	ldr	r1, =(CR_M | CR_C | CR_I)
	mrc	p15, 0, r0, c1, c0, 0
	bic	r0, r0, r1
	mcr	p15, 0, r0, c1, c0, 0
	bx	lr

.global _assign_tlb
_assign_tlb:
	mov	r1, #3
	/* set domain 0 for unchecked access (manager) */
	mcr	p15, 0, r1, c3, c0, 0

	/* point ttbr at tlb (pointer in r0) */
	mcr	p15, 0, r0, c2, c0, 0
	bx	lr	

.end
