/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _ASM_TILE_VERMAGIC_H
#define _ASM_TILE_VERMAGIC_H

#include <arch/chip.h>

/* We can't use modules built with different page sizes. */
#if defined(CONFIG_PAGE_SIZE_16KB)
# define MODULE_PGSZ " 16KB"
#elif defined(CONFIG_PAGE_SIZE_64KB)
# define MODULE_PGSZ " 64KB"
#else
# define MODULE_PGSZ ""
#endif

/* We don't really support no-SMP so tag if someone tries. */
#ifdef CONFIG_SMP
#define MODULE_NOSMP ""
#else
#define MODULE_NOSMP " nosmp"
#endif

#define MODULE_ARCH_VERMAGIC CHIP_ARCH_NAME MODULE_PGSZ MODULE_NOSMP

#endif /* _ASM_TILE_VERMAGIC_H */
