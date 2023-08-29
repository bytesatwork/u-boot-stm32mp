/* SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause */
/*
 * Copyright (c) 2019 bytes at work AG. All rights reserved.
 *
 * based on stm32mp1.h:
 * Copyright (C) 2018, STMicroelectronics - All Rights Reserved
 *
 * Configuration settings for the STM32MP15x CPU
 */

#ifndef __CONFIG_H
#define __CONFIG_H
#include <linux/sizes.h>
#include <asm/arch/stm32.h>

/*
 * Number of clock ticks in 1 sec
 */
#define CONFIG_SYS_HZ				1000

#ifndef CONFIG_STM32MP1_TRUSTED
/* PSCI support */
#define CONFIG_ARMV7_PSCI_1_0
#define CONFIG_ARMV7_SECURE_BASE		STM32_SYSRAM_BASE
#define CONFIG_ARMV7_SECURE_MAX_SIZE		STM32_SYSRAM_SIZE
#endif

/*
 * Configuration of the external SRAM memory used by U-Boot
 */
#define CONFIG_SYS_SDRAM_BASE			STM32_DDR_BASE
#define CONFIG_SYS_INIT_SP_ADDR			CONFIG_SYS_TEXT_BASE

#define CONFIG_DISABLE_CONSOLE

/*
 * Console I/O buffer size
 */
#define CONFIG_SYS_CBSIZE			SZ_1K

/*
 * Needed by "loadb"
 */
#define CONFIG_SYS_LOAD_ADDR			STM32_DDR_BASE

/* ATAGs */
#define CONFIG_CMDLINE_TAG
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG

/* Extend size of kernel image for uncompression */
#define CONFIG_SYS_BOOTM_LEN			SZ_32M

/* SPL support */
#ifdef CONFIG_SPL
/* BOOTROM load address */
#define CONFIG_SPL_TEXT_BASE		0x2FFC2500
/* SPL use DDR */
#define CONFIG_SPL_BSS_START_ADDR	0xC0200000
#define CONFIG_SPL_BSS_MAX_SIZE		0x00100000
#define CONFIG_SYS_SPL_MALLOC_START	0xC0300000
#define CONFIG_SYS_SPL_MALLOC_SIZE	0x00100000

/* limit SYSRAM usage to first 128 KB */
#define CONFIG_SPL_MAX_SIZE		0x00020000
#define CONFIG_SPL_STACK		(STM32_SYSRAM_BASE + \
					 STM32_SYSRAM_SIZE)
#endif /* #ifdef CONFIG_SPL */

#define CONFIG_SYS_MEMTEST_START	STM32_DDR_BASE
#define CONFIG_SYS_MEMTEST_END		(CONFIG_SYS_MEMTEST_START + SZ_64M)
#define CONFIG_SYS_MEMTEST_SCRATCH	(CONFIG_SYS_MEMTEST_END + 4)

/* MMC SD */
#define CONFIG_SYS_MMC_MAX_DEVICE	3
#define CONFIG_SUPPORT_EMMC_BOOT

#define CONFIG_SYS_MAX_FLASH_BANKS	1

#if !defined(CONFIG_SPL_BUILD)

#define CONFIG_BOOTCOMMAND \
	"run select_dtb; " \
	"mmc rescan; " \
	"run mmc_boot; " \

#define MMC_BOOT_CMD \
	"mmc_boot=load mmc ${mmc_dev} ${fdt_addr_r} ${dtbfile} || exit; " \
	"load mmc ${mmc_dev} ${kernel_addr_r} ${kernelfile} || exit; " \
	"run mmc_args; " \
	"bootm ${kernel_addr_r} - ${fdt_addr_r};\0"

/*
 * memory layout for 32M uncompressed/compressed kernel,
 * 1M fdt, 1M script, 1M pxe and 1M for splashimage
 * and the ramdisk at the end.
 */
#define CONFIG_EXTRA_ENV_SETTINGS \
	"mmc_dev=0:4\0" \
	"mmc_root=/dev/mmcblk0p4\0" \
	"kernelfile=/boot/uImage\0" \
	"loadaddr=0xc1000000\0" \
	"stdin=serial\0" \
	"stdout=serial\0" \
	"stderr=serial\0" \
	"bootdelay=1\0" \
	"spl_file=u-boot-spl.stm32\0" \
	"uboot_file=u-boot.img\0" \
	"uboot_offset=0x80000\0" \
	"loadaddr=0xc1000000\0" \
	"spl_uboot_size=0x280000\0" \
	"kernel_addr_r=0xc2000000\0" \
	"fdt_addr_r=0xc4000000\0" \
	"scriptaddr=0xc4100000\0" \
	"pxefile_addr_r=0xc4200000\0" \
	"splashimage=0xc4300000\0"  \
	"ramdisk_addr_r=0xc4400000\0" \
	"fdt_high=0xffffffff\0" \
	"initrd_high=0xffffffff\0" \
	"bootlimit=0\0" \
	"console=ttySTM0,115200\0" \
	"default_args=rootwait rw vt.global_cursor_default=0 consoleblank=0\0" \
	"mmc_args=setenv bootargs ${default_args} console=${console} " \
		"root=${mmc_root} ${bootargs_append}; \0" \
	"select_dtb=if hwid check; " \
		"then " \
			"setenv dtbfile \"" \
			"/boot/stm32mp157c-bytedevkit-" \
			"v${board_major}-${board_minor}.dtb\"; " \
		"else " \
			"setenv dtbfile " \
			"\"/boot/stm32mp157c-bytedevkit-v1-1.dtb\";" \
		"fi;\0" \
	"update_spiflash=echo Updating SPI Flash ...; " \
		"sf probe 0; " \
		"sf erase 0 +${spl_uboot_size}; " \
		"mmc rescan; " \
		"ext4load mmc 0:4 ${loadaddr} ${spl_file} || exit; "\
		"sf write ${loadaddr} 0 ${filesize}; " \
		"ext4load mmc 0:4 ${loadaddr} ${uboot_file} || exit; " \
		"sf write ${loadaddr} ${uboot_offset} ${filesize};\0" \
	MMC_BOOT_CMD

#ifdef CONFIG_ENV_IS_IN_MMC
#define CONFIG_SYS_MMC_ENV_DEV		1
#define CONFIG_SYS_MMC_ENV_PART		0x0
#endif

#endif /* ifndef CONFIG_SPL_BUILD */

#define BAW_CONFIG_BUILTIN_PCB    M5_PCB_REV_1_2
#define BAW_CONFIG_BUILTIN_RAM    M5_RAM_K4B4G1646DBIK0
#define BAW_CONFIG_BUILTIN_FLASH  EMMC_8GB

#endif /* __CONFIG_H */
