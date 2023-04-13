/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2015 bytes at work AG
 */

#ifndef BAW_CONFIG_H
#define BAW_CONFIG_H

#include <config.h>
#include <linux/types.h>

typedef enum {
	M2_PCB_REV_00 = 0,
	M2_PCB_REV_01 = 1,
	M2_PCB_REV_02 = 2,
	M2_PCB_REV_03 = 3,
	M2_PCB_REV_04 = 4,
	M5_PCB_REV_1_0 = 5,
	M5_PCB_REV_1_2 = 6,
	M6_PCB_REV_1_0 = 7,
	M6_PCB_REV_1_1 = 8,
	M8_PCB_REV_0_1 = 9,
	M8_PCB_REV_1_0 = 10,
} baw_config_pcb_t;

typedef enum {
	M2_RAM_MT47H128M16RT25E = 1,
	M2_RAM_K4B2G1646EBIH9   = 2,
	M2_RAM_K4B2G1646QBCK0   = 3,
	M2_RAM_K4B4G1646DBIK0   = 4,
	M2_RAM_MT41K512M16HA107 = 5,
	M6_RAM_MT53E128M32D2DS_053 = 6,
	M6_RAM_MT53E256M32D2DS_053 = 7,
	M6_RAM_MT53E384M32D2DS_053 = 8,
	M6_RAM_MT53D512M32D2DS_053 = 9,
	M6_RAM_MT53E768M32D4DT_053 = 10,
	M6_RAM_MT53D1024M32D4DT_053 = 11,
	/* According to the imx8mm datasheet only Quad-A53 supports more than
	 * 3 GB RAM. Therefore define dedicated enum to use 4 GB as 3 GB RAM.
	 */
	M6_RAM_MT53D1024M32D4DT_053_3GB = 12,
	M8_RAM_MT53E256M16D1DS_046 = 13,
	M8_RAM_MT53D512M16D1DS_046 = 14,
	M8_RAM_MT53E1G16D1FW_046 = 15,
	M8_RAM_MT53E4G16D4NQ_046 = 16,
	M5_RAM_K4B4G1646DBIK0 = 17,
	M5_RAM_MT41K512M16HA107 = 18,
} baw_config_ram_t;

typedef enum {
	M2_NAND_2GBIT  = 0,
	M2_NAND_4GBIT  = 1,
	EMMC_4GB  = 2,
	EMMC_8GB  = 3,
	EMMC_16GB = 4,
	EMMC_32GB = 5,
} baw_config_flash_t;

struct baw_config {
	baw_config_pcb_t pcb;
	baw_config_ram_t ram;
	baw_config_flash_t flash;
	bool ext_avail;
	u32 artno;
	u32 lot;
	u8 lotseq;
	char proddate[12];
	char flashdate[6];
	char flashuser[6];
	bool track_avail;
	char macaddr[18];
	char uid[16];
};

struct baw_config_name_pair {
	u32 config;
	const char *name;
};

extern const struct baw_config_name_pair baw_config_pcb_name[];
extern const struct baw_config_name_pair baw_config_ram_name[];
extern const struct baw_config_name_pair baw_config_flash_name[];

const char *baw_config_get_name(u32 config, const struct baw_config_name_pair *map);

static inline const char *baw_config_get_pcb_name(baw_config_pcb_t config)
{
	return baw_config_get_name(config, baw_config_pcb_name);
}

static inline const char *baw_config_get_ram_name(baw_config_ram_t config)
{
	return baw_config_get_name(config, baw_config_ram_name);
}

static inline const char *baw_config_get_flash_name(baw_config_flash_t config)
{
	return baw_config_get_name(config, baw_config_flash_name);
}

#endif
