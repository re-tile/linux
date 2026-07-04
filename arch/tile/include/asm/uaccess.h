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
 */

#ifndef _ASM_TILE_UACCESS_H
#define _ASM_TILE_UACCESS_H

/*
 * User space memory access functions
 */
#include <asm/processor.h>
#include <asm/page.h>
#include <asm-generic/access_ok.h>

#include <asm/extable.h>

/*
 * This is a type: either unsigned long, if the argument fits into
 * that type, or otherwise unsigned long long.
 */
#define __inttype(x) \
	__typeof__(__builtin_choose_expr(sizeof(x) > sizeof(0UL), 0ULL, 0UL))

/*
 * Support macros for __get_user().
 * Note that __get_user() and __put_user() assume proper alignment.
 */

#define _ASM_PTR	".quad"
#define _ASM_ALIGN	".align 8"

#define __get_user_asm(OP, x, ptr, ret)					\
	asm volatile("1: {" #OP " %1, %2; movei %0, 0 }\n"		\
		     ".pushsection .fixup,\"ax\"\n"			\
		     "0: { movei %1, 0; movei %0, %3 }\n"		\
		     "j 9f\n"						\
		     ".section __ex_table,\"a\"\n"			\
		     _ASM_ALIGN "\n"					\
		     _ASM_PTR " 1b, 0b\n"				\
		     ".popsection\n"					\
		     "9:"						\
		     : "=r" (ret), "=r" (x)				\
		     : "r" (ptr), "i" (-EFAULT))

#define __get_user_1(x, ptr, ret) __get_user_asm(ld1u, x, ptr, ret)
#define __get_user_2(x, ptr, ret) __get_user_asm(ld2u, x, ptr, ret)
#define __get_user_4(x, ptr, ret) __get_user_asm(ld4s, x, ptr, ret)
#define __get_user_8(x, ptr, ret) __get_user_asm(ld, x, ptr, ret)

extern int __get_user_bad(void)
  __attribute__((warning("sizeof __get_user argument not 1, 2, 4 or 8")));

/**
 * __get_user: - Get a simple variable from user space, with less checking.
 * @x:   Variable to store result.
 * @ptr: Source address, in user space.
 *
 * Context: User context only. This function may sleep if pagefaults are
 *          enabled.
 *
 * This macro copies a single simple variable from user space to kernel
 * space.  It supports simple types like char and int, but not larger
 * data types like structures or arrays.
 *
 * @ptr must have pointer-to-simple-variable type, and the result of
 * dereferencing @ptr must be assignable to @x without a cast.
 *
 * Returns zero on success, or -EFAULT on error.
 * On error, the variable @x is set to zero.
 *
 * Caller must check the pointer with access_ok() before calling this
 * function.
 */
#define __get_user(x, ptr)						\
	({								\
		int __ret;						\
		typeof(x) _x;						\
		__chk_user_ptr(ptr);					\
		switch (sizeof(*(ptr))) {				\
		case 1: __get_user_1(_x, ptr, __ret); break;		\
		case 2: __get_user_2(_x, ptr, __ret); break;		\
		case 4: __get_user_4(_x, ptr, __ret); break;		\
		case 8: __get_user_8(_x, ptr, __ret); break;		\
		default: __ret = __get_user_bad(); break;		\
		}							\
		(x) = (typeof(*(ptr))) _x;				\
		__ret;							\
	})

/* Support macros for __put_user(). */

#define __put_user_asm(OP, x, ptr, ret)			\
	asm volatile("1: {" #OP " %1, %2; movei %0, 0 }\n"		\
		     ".pushsection .fixup,\"ax\"\n"			\
		     "0: { movei %0, %3; j 9f }\n"			\
		     ".section __ex_table,\"a\"\n"			\
		     _ASM_ALIGN "\n"					\
		     _ASM_PTR " 1b, 0b\n"				\
		     ".popsection\n"					\
		     "9:"						\
		     : "=r" (ret)					\
		     : "r" (ptr), "r" (x), "i" (-EFAULT))

#define __put_user_1(x, ptr, ret) __put_user_asm(st1, x, ptr, ret)
#define __put_user_2(x, ptr, ret) __put_user_asm(st2, x, ptr, ret)
#define __put_user_4(x, ptr, ret) __put_user_asm(st4, x, ptr, ret)
#define __put_user_8(x, ptr, ret) __put_user_asm(st, x, ptr, ret)

extern int __put_user_bad(void)
  __attribute__((warning("sizeof __put_user argument not 1, 2, 4 or 8")));

/**
 * __put_user: - Write a simple value into user space, with less checking.
 * @x:   Value to copy to user space.
 * @ptr: Destination address, in user space.
 *
 * Context: User context only. This function may sleep if pagefaults are
 *          enabled.
 *
 * This macro copies a single simple value from kernel space to user
 * space.  It supports simple types like char and int, but not larger
 * data types like structures or arrays.
 *
 * @ptr must have pointer-to-simple-variable type, and @x must be assignable
 * to the result of dereferencing @ptr.
 *
 * Caller must check the pointer with access_ok() before calling this
 * function.
 *
 * Returns zero on success, or -EFAULT on error.
 */
#define __put_user(x, ptr)						\
({									\
	int __ret;							\
	typeof(*(ptr)) _x = (x);					\
	__chk_user_ptr(ptr);						\
	switch (sizeof(*(ptr))) {					\
	case 1: __put_user_1(_x, ptr, __ret); break;			\
	case 2: __put_user_2(_x, ptr, __ret); break;			\
	case 4: __put_user_4(_x, ptr, __ret); break;			\
	case 8: __put_user_8(_x, ptr, __ret); break;			\
	default: __ret = __put_user_bad(); break;			\
	}								\
	__ret;								\
})

/*
 * The versions of get_user and put_user without initial underscores
 * check the address of their arguments to make sure they are not
 * in kernel space.
 */
#define put_user(x, ptr)						\
({									\
	__typeof__(*(ptr)) __user *__Pu_addr = (ptr);			\
	access_ok((__Pu_addr), sizeof(*(__Pu_addr))) ?			\
		__put_user((x), (__Pu_addr)) :				\
		-EFAULT;						\
})

#define get_user(x, ptr)						\
({									\
	__typeof__(*(ptr)) const __user *__Gu_addr = (ptr);		\
	access_ok((__Gu_addr), sizeof(*(__Gu_addr))) ?			\
		__get_user((x), (__Gu_addr)) :				\
		((x) = 0, -EFAULT);					\
})

extern unsigned long __must_check
raw_copy_to_user(void __user *to, const void *from, unsigned long n);
extern unsigned long __must_check
raw_copy_from_user(void *to, const void __user *from, unsigned long n);
#define INLINE_COPY_FROM_USER
#define INLINE_COPY_TO_USER

extern long strnlen_user(const char __user *str, long n);
extern long strncpy_from_user(char *dst, const char __user *src, long);

/**
 * clear_user: - Zero a block of memory in user space.
 * @mem:   Destination address, in user space.
 * @len:   Number of bytes to zero.
 *
 * Zero a block of memory in user space.
 *
 * Returns number of bytes that could not be cleared.
 * On success, this will be zero.
 */
extern unsigned long clear_user_asm(void __user *mem, unsigned long len);
static inline unsigned long __must_check __clear_user(
	void __user *mem, unsigned long len)
{
	might_fault();
	return clear_user_asm(mem, len);
}
static inline unsigned long __must_check clear_user(
	void __user *mem, unsigned long len)
{
	if (access_ok(mem, len))
		return __clear_user(mem, len);
	return len;
}

/**
 * flush_user: - Flush a block of memory in user space from cache.
 * @mem:   Destination address, in user space.
 * @len:   Number of bytes to flush.
 *
 * Returns number of bytes that could not be flushed.
 * On success, this will be zero.
 */
extern unsigned long flush_user_asm(void __user *mem, unsigned long len);
static inline unsigned long __must_check __flush_user(
	void __user *mem, unsigned long len)
{
	int retval;

	might_fault();
	retval = flush_user_asm(mem, len);
	mb_incoherent();
	return retval;
}

static inline unsigned long __must_check flush_user(
	void __user *mem, unsigned long len)
{
	if (access_ok(mem, len))
		return __flush_user(mem, len);
	return len;
}

/**
 * finv_user: - Flush-inval a block of memory in user space from cache.
 * @mem:   Destination address, in user space.
 * @len:   Number of bytes to invalidate.
 *
 * Returns number of bytes that could not be flush-invalidated.
 * On success, this will be zero.
 */
extern unsigned long finv_user_asm(void __user *mem, unsigned long len);
static inline unsigned long __must_check __finv_user(
	void __user *mem, unsigned long len)
{
	int retval;

	might_fault();
	retval = finv_user_asm(mem, len);
	mb_incoherent();
	return retval;
}
static inline unsigned long __must_check finv_user(
	void __user *mem, unsigned long len)
{
	if (access_ok(mem, len))
		return __finv_user(mem, len);
	return len;
}

#endif /* _ASM_TILE_UACCESS_H */
