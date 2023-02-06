// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015 bytes at work AG
 */

#include "baw_config.h"

const struct baw_config_name_pair baw_config_pcb_name[] = {
	{ M2_PCB_REV_00, "M2 Rev.00" },
	{ M2_PCB_REV_01, "M2 Rev.01" },
	{ M2_PCB_REV_02, "M2 Rev.02" },
	{ M2_PCB_REV_03, "M2 Rev.03" },
	{ M2_PCB_REV_04, "M2 Rev.04" },
	{ M5_PCB_REV_1_0, "M5 Rev 1.0" },
	{ M5_PCB_REV_1_2, "M5 Rev 1.2" },
	{ M6_PCB_REV_1_0, "M6 Rev 1.0" },
	{ M6_PCB_REV_1_1, "M6 Rev 1.1" },
	{ -1, NULL },
};

const struct baw_config_name_pair baw_config_ram_name[] = {
	{ M2_RAM_MT47H128M16RT25E, "MT47H128M16RT25E 256 MB DDR2 @ 266MHz" },
	{ M2_RAM_K4B2G1646EBIH9,   "K4B2G1646EBIH9 legacy DDR3 @ 303MHz" },
	{ M2_RAM_K4B2G1646QBCK0,   "K4B2G1646QBCK0 256 MB DDR3 @ 400Mhz" },
	{ M2_RAM_K4B4G1646DBIK0,   "K4B4G1646DBIK0 512 MB DDR3 @ 400MHz" },
	{ M2_RAM_MT41K512M16HA107, "MT41K512M16HA107 1024MB DDR3 @ 303MHz" },
	{ M6_RAM_MT53E128M32D2DS_053, "MT53E128M32D2DS-053 512 MB LPDDR4 @ 1500 MHz" },
	{ M6_RAM_MT53E256M32D2DS_053, "MT53E256M32D2DS-053 1 GB LPDDR4 @ 1500 MHz" },
	{ M6_RAM_MT53E384M32D2DS_053, "MT53E384M32D2DS-053 1.5 GB LPDDR4 @ 1500 MHz" },
	{ M6_RAM_MT53D512M32D2DS_053, "MT53D512M32D2DS-053 2 GB LPDDR4 @ 1500 MHz" },
	{ M6_RAM_MT53E768M32D4DT_053, "MT53E768M32D4DT-053 3 GB LPDDR4 @ 1500 MHz" },
	{ M6_RAM_MT53D1024M32D4DT_053, "MT53D1024M32D4DT-053 4 GB LPDDR4 @ 1500 MHz" },
	{ M6_RAM_MT53D1024M32D4DT_053_3GB,
		"MT53D1024M32D4DT-053 4 GB using 3 GB @ 1500 MHz" },
	{ -1, NULL },
};

const struct baw_config_name_pair baw_config_flash_name[] = {
	{ M2_NAND_2GBIT, "Nand 2 GBit" },
	{ M2_NAND_4GBIT, "Nand 4 GBit" },
	{ EMMC_4GB, "EMMC 4 GB" },
	{ EMMC_8GB, "EMMC 8 GB" },
	{ EMMC_16GB, "EMMC 16 GB" },
	{ EMMC_32GB, "EMMC 32 GB" },
	{ -1, NULL },
};

const char *baw_config_get_name(u32 config, const struct baw_config_name_pair *map)
{
	const struct baw_config_name_pair *itr;

	for (itr = map; itr->name; itr++)
		if (itr->config == config)
			return itr->name;

	return "unknown";
}
