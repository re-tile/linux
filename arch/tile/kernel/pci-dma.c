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

#include <linux/mm.h>
#include <linux/dma-direct.h>
#include <linux/dma-map-ops.h>
#include <linux/pci.h>
#include <asm/homecache.h>

/*
 * Allocate what Linux calls "coherent" memory.  On TILE-Gx it is
 * hash-for-home memory, which I/O devices see coherently.
 */
void *arch_dma_alloc(struct device *dev, size_t size, dma_addr_t *dma_handle,
		     gfp_t gfp, unsigned long attrs)
{
	u64 mask = dev->coherent_dma_mask ?: DMA_BIT_MASK(32);
	u64 limit = min_not_zero(mask, dev->bus_dma_limit);
	int node = dev_to_node(dev);
	int order = get_order(size);
	struct page *pg;
	dma_addr_t addr;

	gfp |= __GFP_ZERO;

	/*
	 * If the mask specifies that the memory be in the first 4 GB, then
	 * we force the allocation to come from the DMA zone.  We also
	 * force the node to 0 since that's the only node where the DMA
	 * zone isn't empty.  If the mask size is smaller than 32 bits, we
	 * may still not be able to guarantee a suitable memory address, in
	 * which case we will return NULL.  But such devices are uncommon.
	 */
	if (limit <= DMA_BIT_MASK(32)) {
		gfp |= GFP_DMA32;
		node = 0;
	}

	pg = homecache_alloc_pages_node(node, gfp, order, PAGE_HOME_HASH);
	if (pg == NULL)
		return NULL;

	addr = phys_to_dma(dev, page_to_phys(pg));
	if (addr + size - 1 > limit) {
		__homecache_free_pages(pg, order);
		return NULL;
	}

	*dma_handle = addr;

	return page_address(pg);
}

void arch_dma_free(struct device *dev, size_t size, void *cpu_addr,
		   dma_addr_t dma_addr, unsigned long attrs)
{
	homecache_free_pages((unsigned long)cpu_addr, get_order(size));
}

/*
 * Flush the page from cache if necessary.  On TILE-Gx, data is
 * delivered to hash-for-home L3.
 */
static void __dma_prep_page(struct page *page, enum dma_data_direction dir)
{
	int home = page_home(page);

	switch (home) {
	case PAGE_HOME_HASH:
		return;
	case PAGE_HOME_IMMUTABLE:
		/* Should be going to the device only. */
		BUG_ON(dir == DMA_FROM_DEVICE || dir == DMA_BIDIRECTIONAL);
		return;
	case PAGE_HOME_INCOHERENT:
		/* Incoherent anyway, so no need to work hard here. */
		return;
	default:
		BUG_ON(home < 0 || home >= NR_CPUS);
		break;
	}
	homecache_finv_page(page);
}

static void __dma_complete_page(struct page *page)
{
	switch (page_home(page)) {
	case PAGE_HOME_HASH:
		/* I/O device delivered data the way the cpu wanted it. */
		break;
	case PAGE_HOME_INCOHERENT:
		/* Incoherent anyway, so no need to work hard here. */
		break;
	case PAGE_HOME_IMMUTABLE:
		/* Extra read-only copies are not a problem. */
		break;
	default:
		/* Flush the bogus hash-for-home I/O entries to memory. */
		homecache_finv_map_page(page, PAGE_HOME_HASH);
		break;
	}
}

void arch_sync_dma_for_device(phys_addr_t paddr, size_t size,
			      enum dma_data_direction dir)
{
	unsigned long pfn;

	for (pfn = PFN_DOWN(paddr); pfn <= PFN_DOWN(paddr + size - 1); pfn++)
		__dma_prep_page(pfn_to_page(pfn), dir);
}

void arch_sync_dma_for_cpu(phys_addr_t paddr, size_t size,
			   enum dma_data_direction dir)
{
	unsigned long pfn;

	for (pfn = PFN_DOWN(paddr); pfn <= PFN_DOWN(paddr + size - 1); pfn++)
		__dma_complete_page(pfn_to_page(pfn));
}

#ifdef CONFIG_PCI
/*
 * TRIO maps all of PA space at TILE_PCI_MEM_MAP_BASE_OFFSET for devices
 * that can address above it.  Narrower devices see only the direct
 * window below the BAR window, so they get no offset and a bus limit,
 * and swiotlb bounces whatever falls above it.
 */
void arch_dma_set_mask(struct device *dev, u64 mask)
{
	if (!dev_is_pci(dev))
		return;

	if (mask >= (TILE_PCI_MEM_MAP_BASE_OFFSET << 1) - 1) {
		dev->archdata.dma_offset = TILE_PCI_MEM_MAP_BASE_OFFSET;
		dev->bus_dma_limit = 0;
	} else {
		dev->archdata.dma_offset = 0;
		dev->bus_dma_limit = TILE_PCI_MAX_DIRECT_DMA_ADDRESS;
	}
}
#endif
