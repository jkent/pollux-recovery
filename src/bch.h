/*
 * 4-bit 0x25AF BCH syndrome decoder
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 51
 * Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Based on:
 *    Generic binary BCH encoding/decoding library
 *    Copyright © 2011 Parrot S.A.
 *    Author: Ivan Djelic <ivan.djelic@parrot.com>
 */

#ifndef _BCH_H
#define _BCH_H

void bch_init(void);

int bch_decode(unsigned int len, unsigned int *syn, unsigned int *errloc);

#endif /* _BCH_H */

