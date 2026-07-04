/*
 * Copyright 2011 Tilera Corporation. All Rights Reserved.
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
 * Do not include directly; use <linux/atomic.h>.
 */

#ifndef _ASM_TILE_ATOMIC_64_H
#define _ASM_TILE_ATOMIC_64_H

#ifndef __ASSEMBLY__

#include <asm/barrier.h>
#include <arch/spr_def.h>

/* First, the 32-bit atomic ops that are "real" on our 64-bit platform. */

#define arch_atomic_set(v, i) WRITE_ONCE((v)->counter, (i))

/*
 * The smp_mb() operations throughout are to support the fact that
 * Linux requires memory barriers before and after the operation,
 * on any routine which updates memory and returns a value.
 */

/*
 * Note a subtlety of the locking here.  We are required to provide a
 * full memory barrier before and after the operation.  However, we
 * only provide an explicit mb before the operation.  After the
 * operation, we use barrier() to get a full mb for free, because:
 *
 * (1) The barrier directive to the compiler prohibits any instructions
 * being statically hoisted before the barrier;
 * (2) the microarchitecture will not issue any further instructions
 * until the fetchadd result is available for the "+ i" add instruction;
 * (3) the smb_mb before the fetchadd ensures that no other memory
 * operations are in flight at this point.
 */
static inline int arch_atomic_add_return(int i, atomic_t *v)
{
	int val;
	smp_mb();  /* barrier for proper semantics */
	val = __insn_fetchadd4((void *)&v->counter, i) + i;
	barrier();  /* equivalent to smp_mb(); see block comment above */
	return val;
}

#define ATOMIC_OPS(op)							\
static inline int arch_atomic_fetch_##op(int i, atomic_t *v)		\
{									\
	int val;							\
	smp_mb();							\
	val = __insn_fetch##op##4((void *)&v->counter, i);		\
	smp_mb();							\
	return val;							\
}									\
static inline void arch_atomic_##op(int i, atomic_t *v)		\
{									\
	__insn_fetch##op##4((void *)&v->counter, i);			\
}

ATOMIC_OPS(add)
ATOMIC_OPS(and)
ATOMIC_OPS(or)

#undef ATOMIC_OPS

/* The fallback layer probes these with the preprocessor. */
#define arch_atomic_add_return	arch_atomic_add_return
#define arch_atomic_add		arch_atomic_add
#define arch_atomic_fetch_add	arch_atomic_fetch_add
#define arch_atomic_and		arch_atomic_and
#define arch_atomic_fetch_and	arch_atomic_fetch_and
#define arch_atomic_or		arch_atomic_or
#define arch_atomic_fetch_or	arch_atomic_fetch_or
#define arch_atomic_xor		arch_atomic_xor
#define arch_atomic_fetch_xor	arch_atomic_fetch_xor

static inline int arch_atomic_fetch_xor(int i, atomic_t *v)
{
	int guess, oldval = v->counter;
	smp_mb();
	do {
		guess = oldval;
		__insn_mtspr(SPR_CMPEXCH_VALUE, guess);
		oldval = __insn_cmpexch4(&v->counter, guess ^ i);
	} while (guess != oldval);
	smp_mb();
	return oldval;
}

static inline void arch_atomic_xor(int i, atomic_t *v)
{
	int guess, oldval = v->counter;
	do {
		guess = oldval;
		__insn_mtspr(SPR_CMPEXCH_VALUE, guess);
		oldval = __insn_cmpexch4(&v->counter, guess ^ i);
	} while (guess != oldval);
}

/* Now the true 64-bit operations. */

#define arch_atomic64_read(v)		READ_ONCE((v)->counter)
#define arch_atomic64_set(v, i)		WRITE_ONCE((v)->counter, (i))

static inline s64 arch_atomic64_add_return(s64 i, atomic64_t *v)
{
	s64 val;
	smp_mb();  /* barrier for proper semantics */
	val = __insn_fetchadd((void *)&v->counter, i) + i;
	barrier();  /* equivalent to smp_mb; see arch_atomic_add_return() */
	return val;
}

#define ATOMIC64_OPS(op)						\
static inline s64 arch_atomic64_fetch_##op(s64 i, atomic64_t *v)	\
{									\
	s64 val;							\
	smp_mb();							\
	val = __insn_fetch##op((void *)&v->counter, i);			\
	smp_mb();							\
	return val;							\
}									\
static inline void arch_atomic64_##op(s64 i, atomic64_t *v)		\
{									\
	__insn_fetch##op((void *)&v->counter, i);			\
}

ATOMIC64_OPS(add)
ATOMIC64_OPS(and)
ATOMIC64_OPS(or)

#undef ATOMIC64_OPS

#define arch_atomic64_add_return	arch_atomic64_add_return
#define arch_atomic64_add		arch_atomic64_add
#define arch_atomic64_fetch_add		arch_atomic64_fetch_add
#define arch_atomic64_and		arch_atomic64_and
#define arch_atomic64_fetch_and		arch_atomic64_fetch_and
#define arch_atomic64_or		arch_atomic64_or
#define arch_atomic64_fetch_or		arch_atomic64_fetch_or
#define arch_atomic64_xor		arch_atomic64_xor
#define arch_atomic64_fetch_xor		arch_atomic64_fetch_xor

static inline s64 arch_atomic64_fetch_xor(s64 i, atomic64_t *v)
{
	s64 guess, oldval = v->counter;
	smp_mb();
	do {
		guess = oldval;
		__insn_mtspr(SPR_CMPEXCH_VALUE, guess);
		oldval = __insn_cmpexch(&v->counter, guess ^ i);
	} while (guess != oldval);
	smp_mb();
	return oldval;
}

static inline void arch_atomic64_xor(s64 i, atomic64_t *v)
{
	s64 guess, oldval = v->counter;
	do {
		guess = oldval;
		__insn_mtspr(SPR_CMPEXCH_VALUE, guess);
		oldval = __insn_cmpexch(&v->counter, guess ^ i);
	} while (guess != oldval);
}

#define arch_atomic64_sub_return(i, v)	arch_atomic64_add_return(-(s64)(i), (v))
#define arch_atomic64_fetch_sub(i, v)	arch_atomic64_fetch_add(-(s64)(i), (v))
#define arch_atomic64_sub(i, v)		arch_atomic64_add(-(s64)(i), (v))

#endif /* !__ASSEMBLY__ */

#endif /* _ASM_TILE_ATOMIC_64_H */
