/*
 * cmpxchg.h -- forked from asm/atomic.h with this copyright:
 *
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
 */

#ifndef _ASM_TILE_CMPXCHG_H
#define _ASM_TILE_CMPXCHG_H

#ifndef __ASSEMBLY__

#include <linux/cmpxchg-emu.h>
#include <asm/barrier.h>

/* Nonexistent functions intended to cause compile errors. */
extern void __xchg_called_with_bad_pointer(void)
	__compiletime_error("Bad argument size for xchg");
extern void __cmpxchg_called_with_bad_pointer(void)
	__compiletime_error("Bad argument size for cmpxchg");

#define arch_xchg(ptr, n)						\
	({								\
		typeof(*(ptr)) __x;					\
		smp_mb();						\
		switch (sizeof(*(ptr))) {				\
		case 4:							\
			__x = (typeof(__x))(unsigned long)		\
				__insn_exch4((ptr),			\
					(u32)(unsigned long)(n));	\
			break;						\
		case 8:							\
			__x = (typeof(__x))				\
				__insn_exch((ptr), (unsigned long)(n));	\
			break;						\
		default:						\
			__xchg_called_with_bad_pointer();		\
			break;						\
		}							\
		smp_mb();						\
		__x;							\
	})

#define arch_cmpxchg(ptr, o, n)						\
	({								\
		typeof(*(ptr)) __x;					\
		smp_mb();						\
		switch (sizeof(*(ptr))) {				\
		case 1:							\
			__x = (typeof(__x))cmpxchg_emu_u8(		\
				(volatile u8 *)(ptr),			\
				(uintptr_t)(o), (uintptr_t)(n));	\
			break;						\
		case 4:							\
			__insn_mtspr(SPR_CMPEXCH_VALUE,			\
				     (unsigned long)(o));		\
			__x = (typeof(__x))(unsigned long)		\
				__insn_cmpexch4((ptr),			\
					(u32)(unsigned long)(n));	\
			break;						\
		case 8:							\
			__insn_mtspr(SPR_CMPEXCH_VALUE,			\
				     (unsigned long)(o));		\
			__x = (typeof(__x))__insn_cmpexch((ptr),	\
						(long long)(n));	\
			break;						\
		default:						\
			__cmpxchg_called_with_bad_pointer();		\
			break;						\
		}							\
		smp_mb();						\
		__x;							\
	})

#define arch_cmpxchg64		arch_cmpxchg

/*
 * "Local" versions have no cheaper CPU-local form on tilegx; the
 * full-barrier versions are simply stronger than required.
 */
#define arch_cmpxchg_local	arch_cmpxchg
#define arch_cmpxchg64_local	arch_cmpxchg

#endif /* __ASSEMBLY__ */

#endif /* _ASM_TILE_CMPXCHG_H */
