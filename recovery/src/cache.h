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

#ifndef _CACHE_H
#define _CACHE_H

#if defined(__thumb__)
	extern void _thumb_enable_cache(void);
	extern void _thumb_disable_cache(void);
	extern void _thumb_assign_tlb(void *);
	#define enable_cache() _thumb_enable_cache()
	#define disable_cache() _thumb_disable_cache()
	#define assign_tlb(p) _thumb_assign_tlb(p)
#else
	extern void _enable_cache(void);
	extern void _disable_cache(void);
	extern void _assign_tlb(void *);
	#define enable_cache() _enable_cache()
	#define disable_cache() _disable_cache()
	#define assign_tlb(p) _assign_tlb(p)
#endif

extern void init_tlb(void *);

#endif /* _CACHE_H */
