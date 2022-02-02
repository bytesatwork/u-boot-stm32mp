// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015 bytes at work AG
 */

#include "m2config.h"

const struct m2config_name_pair m2config_pcb_name[] = {
	{ M2_PCB_REV_00, "Rev.00" },
	{ M2_PCB_REV_01, "Rev.01" },
	{ M2_PCB_REV_02, "Rev.02" },
	{ M2_PCB_REV_03, "Rev.03" },
	{ M2_PCB_REV_04, "Rev.04" },
	{ -1, NULL },
};

const struct m2config_name_pair m2config_ram_name[] = {
	{ M2_RAM_MT47H128M16RT25E, "MT47H128M16RT25E 256 MB DDR2 @ 266MHz" },
	{ M2_RAM_K4B2G1646EBIH9,   "K4B2G1646EBIH9 legacy DDR3 @ 303MHz" },
	{ M2_RAM_K4B2G1646QBCK0,   "K4B2G1646QBCK0 256 MB DDR3 @ 400Mhz" },
	{ M2_RAM_K4B4G1646DBIK0,   "K4B4G1646DBIK0 512 MB DDR3 @ 400MHz" },
	{ M2_RAM_MT41K512M16HA107, "MT41K512M16HA107 1024MB DDR3 @ 303MHz" },
	{ -1, NULL },
};

const struct m2config_name_pair m2config_flash_name[] = {
	{ M2_NAND_2GBIT, "Nand 2 GBit" },
	{ M2_NAND_4GBIT, "Nand 4 GBit" },
	{ M2_EMMC_4GB, "EMMC 4 GB" },
	{ M2_EMMC_8GB, "EMMC 8 GB" },
	{ M2_EMMC_16GB, "EMMC 16 GB" },
	{ M2_EMMC_32GB, "EMMC 32 GB" },
	{ -1, NULL },
};

const char *m2config_get_name(u32 config, const struct m2config_name_pair *map)
{
	const struct m2config_name_pair *itr;

	for (itr = map; itr->name; itr++)
		if (itr->config == config)
			return itr->name;

	return "unknown";
}
