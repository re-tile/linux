/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _ASM_TILE_SPARSEMEM_H
#define _ASM_TILE_SPARSEMEM_H

#ifdef CONFIG_SPARSEMEM

#include <arch/chip.h>

/*
 * Memory controllers sit 2^(CHIP_PA_WIDTH - CHIP_LOG_NUM_MSHIMS) = 2^38
 * apart in the PA space, so 4GB sections always align with the
 * controller boundaries and an 8GB controller is two full sections.
 */
#define SECTION_SIZE_BITS	32
#define MAX_PHYSMEM_BITS	CHIP_PA_WIDTH()

#endif /* CONFIG_SPARSEMEM */

#endif /* _ASM_TILE_SPARSEMEM_H */
