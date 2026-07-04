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

#ifndef _ASM_TILE_COMPAT_H
#define _ASM_TILE_COMPAT_H

/*
 * Architecture specific compatibility types
 */
#include <linux/types.h>
#include <asm-generic/compat.h>

#ifdef CONFIG_COMPAT

#include <linux/sched.h>

/* We use the same register dump format in 32-bit images. */
typedef unsigned long compat_elf_greg_t;
#define COMPAT_ELF_NGREG (sizeof(struct pt_regs) / sizeof(compat_elf_greg_t))
typedef compat_elf_greg_t compat_elf_gregset_t[COMPAT_ELF_NGREG];

static inline int is_compat_task(void)
{
	return current_thread_info()->status & TS_COMPAT;
}

/* Sign-extend when storing a kernel pointer to a user's ptregs. */
static inline unsigned long ptr_to_compat_reg(void __user *uptr)
{
	return (long)(int)(long __force)uptr;
}

extern int compat_setup_rt_frame(struct ksignal *ksig, sigset_t *set,
				 struct pt_regs *regs);

/* Compat syscalls. */
struct compat_siginfo;
struct compat_sigaltstack;
long compat_sys_rt_sigreturn(void);
long compat_sys_truncate64(char __user *filename, u32 dummy, u32 low, u32 high);
long compat_sys_ftruncate64(unsigned int fd, u32 dummy, u32 low, u32 high);
long compat_sys_pread64(unsigned int fd, char __user *ubuf, size_t count,
			u32 dummy, u32 low, u32 high);
long compat_sys_pwrite64(unsigned int fd, char __user *ubuf, size_t count,
			 u32 dummy, u32 low, u32 high);
long compat_sys_sync_file_range2(int fd, unsigned int flags,
				 u32 offset_lo, u32 offset_hi,
				 u32 nbytes_lo, u32 nbytes_hi);
long compat_sys_fallocate(int fd, int mode,
			  u32 offset_lo, u32 offset_hi,
			  u32 len_lo, u32 len_hi);
long compat_sys_llseek(unsigned int fd, unsigned int offset_high,
		       unsigned int offset_low, loff_t __user * result,
		       unsigned int origin);

/* Assembly trampoline to avoid clobbering r0. */
long _compat_sys_rt_sigreturn(void);

#endif /* CONFIG_COMPAT */

#endif /* _ASM_TILE_COMPAT_H */
