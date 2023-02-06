// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015 bytes at work AG
 */

#include <common.h>
#include <command.h>
#include <post.h>
#include "baw_config_builtin.h"
#include "baw_config_eeprom.h"

#if defined(CONFIG_BAW_CONFIG_BUILTIN) || defined(CONFIG_BAW_CONFIG_EEPROM)
static void print_config(const struct baw_config *config)
{
	printf("PCB:   %u (%s)\n", config->pcb,
	       baw_config_get_pcb_name(config->pcb));
	printf("RAM:   %u (%s)\n", config->ram,
	       baw_config_get_ram_name(config->ram));
	printf("Flash: %u (%s)\n", config->flash,
	       baw_config_get_flash_name(config->flash));

	if (config->ext_avail == 1) {
		printf("\n");
		printf("Article number:  %u\n", config->artno);
		printf("Lot:             %u.%u\n", config->lot, config->lotseq);
		printf("Production date: %s\n", config->proddate);
		printf("Flash date:      %s\n", config->flashdate);
		printf("Flash user:      %s\n", config->flashuser);
	}

	if (config->track_avail == 1) {
		printf("\n");
		printf("MAC address:     %s\n", config->macaddr);
		printf("UID:             %s\n", config->uid);
	}
}
#endif

#if defined(CONFIG_BAW_CONFIG_BUILTIN)
static enum command_ret_t cmd_builtin(int argc, char * const argv[])
{
	struct baw_config config = {0};

	if (argc != 0)
		return CMD_RET_USAGE;

	if (baw_config_builtin(&config) != 0) {
		printf("could not get built-in configuration\n");
		return CMD_RET_FAILURE;
	}

	print_config(&config);

	return CMD_RET_SUCCESS;
}
#endif

#if defined(CONFIG_BAW_CONFIG_EEPROM)
static enum command_ret_t cmd_read(int argc, char * const argv[])
{
	struct baw_config config = {0};
	int ret;

	if (argc != 0)
		return CMD_RET_USAGE;

	ret = baw_config_eeprom_read(&config);
	if (ret != 0)
		printf("no configuration in eeprom: %i\n", ret);
	else
		print_config(&config);

	return CMD_RET_SUCCESS;
}

static enum command_ret_t cmd_write(int argc, char * const argv[])
{
	struct baw_config config = {0};

	if (argc != 11)
		return CMD_RET_USAGE;

	config.pcb = simple_strtoul(argv[0], NULL, 10);
	config.ram = simple_strtoul(argv[1], NULL, 10);
	config.flash = simple_strtoul(argv[2], NULL, 10);

	config.artno = simple_strtoul(argv[3], NULL, 10);
	config.lot = simple_strtoul(argv[4], NULL, 10);
	config.lotseq = simple_strtoul(argv[5], NULL, 10);
	strlcpy(config.proddate, argv[6], sizeof(config.proddate));
	strlcpy(config.flashdate, argv[7], sizeof(config.flashdate));
	strlcpy(config.flashuser, argv[8], sizeof(config.flashuser));

	strlcpy(config.macaddr, argv[9], sizeof(config.macaddr));
	strlcpy(config.uid, argv[10], sizeof(config.uid));

	if (baw_config_eeprom_write(&config) != 0) {
		printf("could not write to EEPROM\n");
		return CMD_RET_FAILURE;
	}

	return CMD_RET_SUCCESS;
}

static enum command_ret_t cmd_erase(int argc, char * const argv[])
{
	if (argc != 0)
		return CMD_RET_USAGE;

	if (baw_config_eeprom_erase() != 0) {
		printf("could not erase configuration\n");
		return CMD_RET_FAILURE;
	}

	return CMD_RET_SUCCESS;
}

#if defined(CONFIG_BAW_CONFIG_BUILTIN)
static enum command_ret_t cmd_builtin2eeprom(int argc, char * const argv[])
{
	struct baw_config config = {0};

	if (argc != 0)
		return CMD_RET_USAGE;

	if (baw_config_builtin(&config) != 0) {
		printf("could not get built-in configuration\n");
		return CMD_RET_FAILURE;
	}

	if (baw_config_eeprom_write(&config) != 0) {
		printf("could not write to EEPROM\n");
		return CMD_RET_FAILURE;
	}

	return CMD_RET_SUCCESS;
}
#endif
#endif

struct baw_config_cmd_struct {
	char *name;
	enum command_ret_t (*func)(int argc, char * const argv[]);
};

static const struct baw_config_cmd_struct baw_config_cmd[] = {
#if defined(CONFIG_BAW_CONFIG_BUILTIN)
	{ "builtin", cmd_builtin },
#endif
#if defined(CONFIG_BAW_CONFIG_EEPROM)
#if defined(CONFIG_BAW_CONFIG_BUILTIN)
	{ "builtin2eeprom", cmd_builtin2eeprom },
#endif
	{ "read", cmd_read },
	{ "erase", cmd_erase },
	{ "write", cmd_write },
#endif
	{ NULL, NULL }
};

static const struct baw_config_cmd_struct *get_cmd(const char *name)
{
	const struct baw_config_cmd_struct *cmd;

	for (cmd = baw_config_cmd; cmd->name; cmd++)
		if (strcmp(name, cmd->name) == 0)
			return cmd;

	return NULL;
}

static void print_config_names(const struct baw_config_name_pair *map)
{
	const struct baw_config_name_pair *itr;

	for (itr = map; itr->name; itr++)
		printf("  %2u (%s)\n", itr->config, itr->name);
}

static enum command_ret_t print_configurations(void)
{
	printf("PCB:\n");
	print_config_names(baw_config_pcb_name);
	printf("RAM:\n");
	print_config_names(baw_config_ram_name);
	printf("Flash:\n");
	print_config_names(baw_config_flash_name);

	return CMD_RET_SUCCESS;
}

enum command_ret_t do_baw_config(cmd_tbl_t *cmdtp, int flag, int argc,
				 char * const argv[])
{
	if (argc >= 2) {
		const struct baw_config_cmd_struct *cmd = get_cmd(argv[1]);

		if (cmd)
			return cmd->func(argc - 2, &argv[2]);
		else
			return CMD_RET_USAGE;
	}

	return print_configurations();
}

U_BOOT_CMD(bawconfig, 13, 0, do_baw_config,
	   "perform baw module configuration",
	   "- list known configuration values\n"
#if defined(CONFIG_BAW_CONFIG_BUILTIN)
	   "builtin  - print built-in configuration from u-boot\n"
#endif
#if defined(CONFIG_BAW_CONFIG_EEPROM)
	   "read     - read configuration from EEPROM\n"
	   "erase    - erase configuration from EEPROM\n"
	   "write <PCB> <RAM> <Flash> <Article number> <Lot> <Lot sequence number>\n"
	   "      <Production date> <Flash date> <Flash user> <MAC address> <UID>\n"
	   "         - write configuration to EEPROM\n"
#if defined(CONFIG_BAW_CONFIG_BUILTIN)
	   "builtin2eeprom\n"
	   "         - write built-in configuration to EEPROM\n"
#endif
#endif
);
