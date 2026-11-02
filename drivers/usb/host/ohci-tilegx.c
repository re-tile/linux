// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2012 Tilera Corporation. All Rights Reserved.
 */

/*
 * Tilera TILE-Gx USB OHCI host controller driver.
 */

#include <linux/dma-mapping.h>
#include <linux/irq.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/usb/tilegx.h>
#include <linux/usb.h>
#include <linux/usb/hcd.h>

#include <asm/homecache.h>

#include <gxio/iorpc_usb_host.h>
#include <gxio/usb_host.h>

#include "ohci.h"

static struct hc_driver __read_mostly ohci_tilegx_hc_driver;

static const struct ohci_driver_overrides ohci_tilegx_overrides __initconst = {
	.product_desc	= "Tile-Gx OHCI",
};

static int ohci_hcd_tilegx_drv_probe(struct platform_device *pdev)
{
	struct usb_hcd *hcd;
	struct tilegx_usb_platform_data *pdata = dev_get_platdata(&pdev->dev);
	pte_t pte = { 0 };
	int my_cpu = smp_processor_id();
	int ret;

	if (usb_disabled())
		return -ENODEV;

	ret = dma_coerce_mask_and_coherent(&pdev->dev, DMA_BIT_MASK(32));
	if (ret)
		return ret;

	/*
	 * Try to initialize our GXIO context; if we can't, the device
	 * doesn't exist.
	 */
	if (gxio_usb_host_init(&pdata->usb_ctx, pdata->dev_index, 0) != 0)
		return -ENXIO;

	hcd = usb_create_hcd(&ohci_tilegx_hc_driver, &pdev->dev,
			     dev_name(&pdev->dev));
	if (!hcd) {
		ret = -ENOMEM;
		goto err_hcd;
	}

	/*
	 * We don't use rsrc_start to map in our registers, but seems like
	 * we ought to set it to something, so we use the register VA.
	 */
	hcd->rsrc_start =
		(ulong) gxio_usb_host_get_reg_start(&pdata->usb_ctx);
	hcd->rsrc_len = gxio_usb_host_get_reg_len(&pdata->usb_ctx);
	hcd->regs = gxio_usb_host_get_reg_start(&pdata->usb_ctx);

	/* Create our IRQs and register them. */
	pdata->irq = tile_irq_alloc();
	if (!pdata->irq) {
		ret = -ENXIO;
		goto err_no_irq;
	}

	tile_irq_activate(pdata->irq, TILE_IRQ_PERCPU);

	/* Configure interrupts. */
	ret = gxio_usb_host_cfg_interrupt(&pdata->usb_ctx,
					  cpu_x(my_cpu), cpu_y(my_cpu),
					  KERNEL_PL, pdata->irq);
	if (ret) {
		ret = -ENXIO;
		goto err_have_irq;
	}

	/* Register all of our memory. */
	pte = pte_set_home(pte, PAGE_HOME_HASH);
	ret = gxio_usb_host_register_client_memory(&pdata->usb_ctx, pte, 0);
	if (ret) {
		ret = -ENXIO;
		goto err_have_irq;
	}

	ret = usb_add_hcd(hcd, pdata->irq, IRQF_SHARED);
	if (ret == 0) {
		platform_set_drvdata(pdev, hcd);
		device_wakeup_enable(hcd->self.controller);
		return ret;
	}

err_have_irq:
	tile_irq_free(pdata->irq);
err_no_irq:
	usb_put_hcd(hcd);
err_hcd:
	gxio_usb_host_destroy(&pdata->usb_ctx);
	return ret;
}

static void ohci_hcd_tilegx_drv_remove(struct platform_device *pdev)
{
	struct usb_hcd *hcd = platform_get_drvdata(pdev);
	struct tilegx_usb_platform_data *pdata = dev_get_platdata(&pdev->dev);

	usb_remove_hcd(hcd);
	usb_put_hcd(hcd);
	gxio_usb_host_destroy(&pdata->usb_ctx);
	tile_irq_free(pdata->irq);
}

static struct platform_driver ohci_hcd_tilegx_driver = {
	.probe		= ohci_hcd_tilegx_drv_probe,
	.remove		= ohci_hcd_tilegx_drv_remove,
	.shutdown	= usb_hcd_platform_shutdown,
	.driver = {
		.name	= "tilegx-ohci",
	}
};

static int __init ohci_tilegx_init(void)
{
	if (usb_disabled())
		return -ENODEV;

	ohci_init_driver(&ohci_tilegx_hc_driver, &ohci_tilegx_overrides);
	return platform_driver_register(&ohci_hcd_tilegx_driver);
}
module_init(ohci_tilegx_init);

static void __exit ohci_tilegx_cleanup(void)
{
	platform_driver_unregister(&ohci_hcd_tilegx_driver);
}
module_exit(ohci_tilegx_cleanup);

MODULE_DESCRIPTION("Tilera TILE-Gx OHCI host controller driver");
MODULE_ALIAS("platform:tilegx-ohci");
MODULE_LICENSE("GPL");
