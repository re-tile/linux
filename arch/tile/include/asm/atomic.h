/*
 * Copyright 2010 Tilera Corporation. All Rights Reserved.
 *
 *   This program is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU General Public License
 *   as published by the Free Software Foundation, version 2.
 *
 *   This program is distributed in the hope that it will be useful, but
 *   WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, GOOD TITLE or
 *   NON INFRINGEMENT.  See the GNU General Public License for
 *   more details.
 *
 * Atomic primitives.
 */

#ifndef _ASM_TILE_ATOMIC_H
#define _ASM_TILE_ATOMIC_H

#include <asm/cmpxchg.h>

#ifndef __ASSEMBLY__

#include <linux/compiler.h>
#include <linux/types.h>

static inline int arch_atomic_read(const atomic_t *v)
{
	return READ_ONCE(v->counter);
}

#define arch_atomic_sub_return(i, v)	arch_atomic_add_return((int)(-(i)), (v))

#define arch_atomic_fetch_sub(i, v)	arch_atomic_fetch_add(-(int)(i), (v))

#define arch_atomic_sub(i, v)		arch_atomic_add((int)(-(i)), (v))

#include <asm/atomic_64.h>

#endif /* __ASSEMBLY__ */

#endif /* _ASM_TILE_ATOMIC_H */
