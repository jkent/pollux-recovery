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
#include "asm/types.h"

void init_tlb(u32 *tlb)
{
	int i;

	/* RAM */
#if defined(CONFIG_SHADOW)
	for (i = 0x000; i < 0x100; i++) {
#else
	for (i = 0x800; i < 0x900; i++) {
#endif
		tlb[i]  = (2 <<  0); /* section entry */
		tlb[i] |= (3 <<  2); /* cache and buffer */
		tlb[i] |= (0 <<  5); /* domain 0 */
		tlb[i] |= (3 << 10); /* unrestricted access */
		tlb[i] |= (i << 20); /* section address */
	}

	/* static bus */
#if defined(CONFIG_SHADOW)
	for (i = 0x800; i < 0xA80; i++) {
#else
	for (i = 0x000; i < 0x280; i++) {
#endif
		tlb[i]  = (2 <<  0); /* section entry */
		tlb[i] |= (3 <<  2); /* cache and buffer */
		tlb[i] |= (0 <<  5); /* domain 0 */
		tlb[i] |= (3 << 10); /* unrestricted access */
		tlb[i] |= (i << 20); /* section address */
	}

	/* NAND registers */
#if defined(CONFIG_SHADOW)
	for (i = 0xAC0; i < 0xB00; i++) {
#else
	for (i = 0x2C0; i < 0x300; i++) {
#endif
		tlb[i]  = (2 <<  0); /* section entry */
		tlb[i] |= (0 <<  2); /* no cache or buffer */
		tlb[i] |= (0 <<  5); /* domain 0 */
		tlb[i] |= (3 << 10); /* unrestricted access */
		tlb[i] |= (i << 20); /* section address */
	}

	/* I/O */
	for (i = 0xC00; i < 0xE00; i++) {
		tlb[i]  = (2 <<  0); /* section entry */
		tlb[i] |= (0 <<  2); /* no cache or buffer */
		tlb[i] |= (0 <<  5); /* domain 0 */
		tlb[i] |= (3 << 10); /* unrestricted access */
		tlb[i] |= (i << 20); /* section address */
	}
}
