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
/* In compat mode, we use sys_llseek() for compat_sys_llseek(). */
#ifdef CONFIG_COMPAT
#define __ARCH_WANT_SYS_LLSEEK
#endif
/*
 * The tile generic-ABI userland (glibc 2.12 era) predates these syscalls
 * becoming opt-in for new architectures, so keep the classic set that was
 * unconditional in 4.16: newfstatat/fstat (the __ARCH_WANT_SYS_NEWFSTATAT
 * gate was renamed __ARCH_WANT_NEW_STAT in 4.20) and getrlimit/setrlimit.
 * Without NEW_STAT the dynamic loader dies stat'ing libc.so.6 with ENOSYS.
 */
#define __ARCH_WANT_NEW_STAT
#define __ARCH_WANT_SET_GET_RLIMIT
#define __ARCH_WANT_SYS_CLONE
#include <uapi/asm/unistd.h>
