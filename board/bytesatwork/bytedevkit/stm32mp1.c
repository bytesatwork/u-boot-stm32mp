// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright (c) 2019 bytes at work AG. All rights reserved.
 *
 * based on stm32mp1/stm32mp1.c:
 * Copyright (C) 2018, STMicroelectronics - All Rights Reserved
 */

#include <config.h>
#include <common.h>
#include <adc.h>
#include <dm.h>
#include <clk.h>
#include <console.h>
#include <fdt_support.h>
#include <generic-phy.h>
#include <i2c.h>
#include <led.h>
#include <misc.h>
#include <mtd_node.h>
#include <phy.h>
#include <remoteproc.h>
#include <reset.h>
#include <syscon.h>
#include <usb.h>
#include <asm/io.h>
#include <asm/gpio.h>
#include <asm/arch/stm32.h>
#include <asm/arch/stm32mp1_smc.h>
#include <jffs2/load_kernel.h>
#include <power/regulator.h>
#include <usb/dwc2_udc.h>

/* SYSCFG registers */
#define SYSCFG_BOOTR		0x00
#define SYSCFG_PMCSETR		0x04
#define SYSCFG_IOCTRLSETR	0x18
#define SYSCFG_ICNR		0x1C
#define SYSCFG_CMPCR		0x20
#define SYSCFG_CMPENSETR	0x24
#define SYSCFG_PMCCLRR		0x44

#define SYSCFG_IOCTRLSETR_HSLVEN_TRACE		BIT(0)
#define SYSCFG_IOCTRLSETR_HSLVEN_QUADSPI	BIT(1)
#define SYSCFG_IOCTRLSETR_HSLVEN_ETH		BIT(2)
#define SYSCFG_IOCTRLSETR_HSLVEN_SDMMC		BIT(3)
#define SYSCFG_IOCTRLSETR_HSLVEN_SPI		BIT(4)

#define SYSCFG_CMPCR_SW_CTRL		BIT(1)
#define SYSCFG_CMPCR_READY		BIT(8)

#define SYSCFG_CMPENSETR_MPU_EN		BIT(0)

#define SYSCFG_PMCSETR_ETH_CLK_SEL	BIT(16)
#define SYSCFG_PMCSETR_ETH_REF_CLK_SEL	BIT(17)

#define SYSCFG_PMCSETR_ETH_SELMII	BIT(20)

#define SYSCFG_PMCSETR_ETH_SEL_MASK	GENMASK(23, 21)
#define SYSCFG_PMCSETR_ETH_SEL_GMII_MII	(0 << 21)
#define SYSCFG_PMCSETR_ETH_SEL_RGMII	(1 << 21)
#define SYSCFG_PMCSETR_ETH_SEL_RMII	(4 << 21)

/*
 * Get a global data pointer
 */
DECLARE_GLOBAL_DATA_PTR;

int checkboard(void)
{
	int ret;
	char *mode;
	/* TODO: Do we need this? */
	u32 otp;
	struct udevice *dev;
	const char *fdt_compat;
	int fdt_compat_len;

	if (IS_ENABLED(CONFIG_STM32MP1_TRUSTED))
		mode = "trusted";
	else
		mode = "basic";

	printf("Board: stm32mp1 in %s mode", mode);
	fdt_compat = fdt_getprop(gd->fdt_blob, 0, "compatible",
				 &fdt_compat_len);
	if (fdt_compat && fdt_compat_len)
		printf(" (%s)", fdt_compat);
	puts("\n");

	ret = uclass_get_device_by_driver(UCLASS_MISC,
					  DM_GET_DRIVER(stm32mp_bsec),
					  &dev);

	if (!ret)
		ret = misc_read(dev, STM32_BSEC_SHADOW(BSEC_OTP_BOARD),
				&otp, sizeof(otp));
	if (!ret && otp) {
		printf("Board: MB%04x Var%d Rev.%c-%02d\n",
		       otp >> 16,
		       (otp >> 12) & 0xF,
		       ((otp >> 8) & 0xF) - 1 + 'A',
		       otp & 0xF);
	}

	return 0;
}
int board_late_init(void)
{
#ifdef CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG
	const void *fdt_compat;
	int fdt_compat_len;

	fdt_compat = fdt_getprop(gd->fdt_blob, 0, "compatible",
				 &fdt_compat_len);
	if (fdt_compat && fdt_compat_len) {
		if (strncmp(fdt_compat, "st,", 3) != 0)
			env_set("board_name", fdt_compat);
		else
			env_set("board_name", fdt_compat + 3);
	}
#endif

	return 0;
}

#ifdef CONFIG_STM32_SDMMC2
/* this is a weak define that we are overriding */
int board_mmc_init(void)
{
	return 0;
}
#endif

#ifdef CONFIG_STM32_QSPI
void board_qspi_init(void)
{
}
#endif /* CONFIG_STM32_QSPI */

static void sysconf_init(void)
{
	u8 *syscfg;
#ifdef CONFIG_DM_REGULATOR
	struct udevice *pwr_dev;
	struct udevice *pwr_reg;
	struct udevice *dev;
	int ret;
	u32 otp = 0;
#endif
	u32 bootr;

	syscfg = (u8 *)syscon_get_first_range(STM32MP_SYSCON_SYSCFG);
	debug("SYSCFG: init @0x%p\n", syscfg);

	/* interconnect update : select master using the port 1 */
	/* LTDC = AXI_M9 */
	/* GPU  = AXI_M8 */
	/* today information is hardcoded in U-Boot */
	writel(BIT(9), syscfg + SYSCFG_ICNR);
	debug("[0x%x] SYSCFG.icnr = 0x%08x (LTDC and GPU)\n",
	      (u32)syscfg + SYSCFG_ICNR, readl(syscfg + SYSCFG_ICNR));

	/* disable Pull-Down for boot pin connected to VDD */
	bootr = readl(syscfg + SYSCFG_BOOTR);
	bootr |= (bootr & 0x7 << 4);
	writel(bootr, syscfg + SYSCFG_BOOTR);
	debug("[0x%x] SYSCFG.bootr = 0x%08x\n",
	      (u32)syscfg + SYSCFG_BOOTR, readl(syscfg + SYSCFG_BOOTR));

#ifdef CONFIG_DM_REGULATOR
	/* High Speed Low Voltage Pad mode Enable for SPI, SDMMC, ETH, QSPI
	 * and TRACE. Needed above ~50MHz and conditioned by AFMUX selection.
	 * The customer will have to disable this for low frequencies
	 * or if AFMUX is selected but the function not used, typically for
	 * TRACE. Otherwise, impact on power consumption.
	 *
	 * WARNING:
	 *   enabling High Speed mode while VDD>2.7V
	 *   with the OTP product_below_2v5 (OTP 18, BIT 13)
	 *   erroneously set to 1 can damage the IC!
	 *   => U-Boot set the register only if VDD < 2.7V (in DT)
	 *      but this value need to be consistent with board design
	 */
	ret = syscon_get_by_driver_data(STM32MP_SYSCON_PWR, &pwr_dev);
	if (!ret) {

		ret = uclass_get_device_by_driver(UCLASS_MISC,
						  DM_GET_DRIVER(stm32mp_bsec),
						  &dev);
		if (ret) {
			pr_err("Can't find stm32mp_bsec driver\n");
			return;
		}

		ret = misc_read(dev, STM32_BSEC_SHADOW(18), &otp, 4);
		if (!ret)
			otp = otp & BIT(13);

		/* get VDD = pwr-supply */
		ret = device_get_supply_regulator(pwr_dev, "pwr-supply",
						  &pwr_reg);

		/* check if VDD is Low Voltage */
		if (!ret) {
			if (regulator_get_value(pwr_reg) < 2700000) {
				writel(SYSCFG_IOCTRLSETR_HSLVEN_TRACE |
				       SYSCFG_IOCTRLSETR_HSLVEN_QUADSPI |
				       SYSCFG_IOCTRLSETR_HSLVEN_ETH |
				       SYSCFG_IOCTRLSETR_HSLVEN_SDMMC |
				       SYSCFG_IOCTRLSETR_HSLVEN_SPI,
				       syscfg + SYSCFG_IOCTRLSETR);

				if (!otp)
					pr_err("product_below_2v5=0: HSLVEN protected by HW\n");
			} else {
				if (otp)
					pr_err("product_below_2v5=1: HSLVEN update is destructive, no update as VDD>2.7V\n");
			}
		} else {
			debug("VDD unknown");
		}
	}
#endif
	debug("[0x%x] SYSCFG.IOCTRLSETR = 0x%08x\n",
	      (u32)syscfg + SYSCFG_IOCTRLSETR,
	      readl(syscfg + SYSCFG_IOCTRLSETR));

	/* activate automatic I/O compensation
	 * warning: need to ensure CSI enabled and ready in clock driver
	 */
	writel(SYSCFG_CMPENSETR_MPU_EN, syscfg + SYSCFG_CMPENSETR);

	while (!(readl(syscfg + SYSCFG_CMPCR) & SYSCFG_CMPCR_READY))
		;
	clrbits_le32(syscfg + SYSCFG_CMPCR, SYSCFG_CMPCR_SW_CTRL);

	debug("[0x%x] SYSCFG.cmpcr = 0x%08x\n",
	      (u32)syscfg + SYSCFG_CMPCR, readl(syscfg + SYSCFG_CMPCR));
}

/* board dependent setup after realloc */
int board_init(void)
{
	struct udevice *dev;

	/* address of boot parameters */
	gd->bd->bi_boot_params = STM32_DDR_BASE + 0x100;

	/* probe all PINCTRL for hog */
	for (uclass_first_device(UCLASS_PINCTRL, &dev);
	     dev;
	     uclass_next_device(&dev)) {
		pr_debug("probe pincontrol = %s\n", dev->name);
	}

	sysconf_init();

#ifdef CONFIG_STM32_SDMMC2
	board_mmc_init();
#endif /* CONFIG_STM32_SDMMC2 */

#ifdef CONFIG_STM32_QSPI
	board_qspi_init();
#endif /* CONFIG_STM32_QSPI */

	return 0;
}

#ifdef CONFIG_SYS_MTDPARTS_RUNTIME
void board_mtdparts_default(const char **mtdids, const char **mtdparts)
{
	struct udevice *dev;
	char *s_nand0 = NULL, *s_nor0 = NULL;
	static char parts[256];
	static char ids[22];

	if (!uclass_get_device(UCLASS_MTD, 0, &dev))
		s_nand0 = env_get("mtdparts_nand0");

	if (!uclass_get_device(UCLASS_SPI_FLASH, 0, &dev))
		s_nor0 = env_get("mtdparts_nor0");

	strcpy(ids, "");
	strcpy(parts, "");
	if (s_nand0 && s_nor0) {
		snprintf(ids, sizeof(ids), "nor0=nor0,nand0=nand0");
		snprintf(parts, sizeof(parts),
			 "mtdparts=nor0:%s;nand0:%s", s_nor0, s_nand0);
	} else if (s_nand0) {
		snprintf(ids, sizeof(ids), "nand0=nand0");
		snprintf(parts, sizeof(parts), "mtdparts=nand0:%s", s_nand0);
	} else if (s_nor0) {
		snprintf(ids, sizeof(ids), "nor0=nor0");
		snprintf(parts, sizeof(parts), "mtdparts=nor0:%s", s_nor0);
	}
	*mtdids = ids;
	*mtdparts = parts;
	debug("%s:mtdids=%s & mtdparts=%s\n", __func__, ids, parts);
}
#endif

#if defined(CONFIG_OF_BOARD_SETUP)
int ft_board_setup(void *blob, bd_t *bd)
{
	ulong copro_rsc_addr, copro_rsc_size;
	int off;
	char *s_copro = NULL;
#ifdef CONFIG_FDT_FIXUP_PARTITIONS
	struct node_info nodes[] = {
		{ "st,stm32f469-qspi",		MTD_DEV_TYPE_NOR,  },
		{ "st,stm32mp15-fmc2",		MTD_DEV_TYPE_NAND, },
	};
	fdt_fixup_mtdparts(blob, nodes, ARRAY_SIZE(nodes));
#endif

	/* Update DT if coprocessor started */
	off = fdt_path_offset(blob, "/m4");
	if (off > 0) {
		s_copro = env_get("copro_state");
		copro_rsc_addr  = env_get_hex("copro_rsc_addr", 0);
		copro_rsc_size  = env_get_hex("copro_rsc_size", 0);

		if (s_copro) {
			fdt_setprop_empty(blob, off, "early-booted");
			if (copro_rsc_addr)
				fdt_setprop_u32(blob, off, "rsc-address",
						copro_rsc_addr);
			if (copro_rsc_size)
				fdt_setprop_u32(blob, off, "rsc-size",
						copro_rsc_size);
		} else {
			fdt_delprop(blob, off, "early-booted");
		}
	}

	return 0;
}
#endif

void board_stm32copro_image_process(ulong fw_image, size_t fw_size)
{
	int ret, id = 0; /* Copro id fixed to 0 as only one coproc on mp1 */
	unsigned int rsc_size;
	ulong rsc_addr;

	if (!rproc_is_initialized())
		if (rproc_init()) {
			printf("Remote Processor %d initialization failed\n",
			       id);
			return;
		}

	ret = rproc_load_rsc_table(id, fw_image, fw_size, &rsc_addr, &rsc_size);
	if (!ret) {
		env_set_hex("copro_rsc_addr", rsc_addr);
		env_set_hex("copro_rsc_size", rsc_size);
	}

	ret = rproc_load(id, fw_image, fw_size);
	printf("Load Remote Processor %d with data@addr=0x%08lx %u bytes:%s\n",
	       id, fw_image, fw_size, ret ? " Failed!" : " Success!");

	if (!ret) {
		rproc_start(id);
		env_set("copro_state", "booted");
	}
}

U_BOOT_FIT_LOADABLE_HANDLER(IH_TYPE_STM32COPRO, board_stm32copro_image_process);
