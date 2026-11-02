/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _ASM_TILE_DMA_DIRECT_H
#define _ASM_TILE_DMA_DIRECT_H

#include <linux/device.h>

static inline dma_addr_t phys_to_dma(struct device *dev, phys_addr_t paddr)
{
	return paddr + dev->archdata.dma_offset;
}

static inline phys_addr_t dma_to_phys(struct device *dev, dma_addr_t dma_addr)
{
	return dma_addr - dev->archdata.dma_offset;
}

#endif /* _ASM_TILE_DMA_DIRECT_H */
