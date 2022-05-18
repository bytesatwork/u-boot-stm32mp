// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2022 bytes at work AG. All rights reserved.
 *
 * U-Boot command to read HWID EEPROM module and parse the there-contained
 * information offers functionality to read hardware identifiers from EEPROM and
 * write such
 */

#include <command.h>
#include <common.h>
#include <eeprom.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <u-boot/crc.h>
#include <vsprintf.h>

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#define HWID_EEPROM_DEVICE (1)
#define EEPROM_ROW (16)
#define MAGIC "byteDEVKIT"
#define MAGIC_SIZE (16)
struct bytedevkit_header {
	/* Header Format for byteDEVKIT 1.3 */
	char magic[MAGIC_SIZE];	/* "byteDEVKIT", as defined in MAGIC */
	u32 crc;		/* checksum of the payload */
	u32 payload_size;	/* size of the header starting here (it's 57) */
	u8 header_version;
	u8 major;
	u8 minor;
	u8 patch;
	u32 art_nr;
	u32 lot;
	u8 lotseq;
	char proddate[12];
	char flashdate[6];
	char flashuser[6];
	char uid[16];
} __packed;
#define HEADER_PAYLOAD_START (MAGIC_SIZE + sizeof(u32))
#define HEADER_PAYLOAD_SIZE (sizeof(struct bytedevkit_header) \
			     - HEADER_PAYLOAD_START)
#define UNDEFINED (-1)
#define CURRENT_HEADER_VERSION (1)

static struct bytedevkit_header hwid_eeprom;

#if defined(DEBUG)
/* DEBUGGING FUNCTIONS */
static void inspect_args(int argc,  char * const argv[])
{
	/* Print the command being called */
	printf("calling: `hwid");
	for (int i = 0; i < argc; i++)
		printf(" %s", argv[i]);
	printf("`\n");
}
#endif

/* UTILITIES */
static int check_magic_number(void)
{
	/* Compare the EEPROM string with our pre-processor #define MAGIC */
	if (strncmp(hwid_eeprom.magic, MAGIC, 11)) {
		printf("magic number did NOT have the expected value: \"%s\""
		       "vs \"%s\"\n\n", hwid_eeprom.magic, MAGIC);
		return -EINVAL;
	}
	return 0;
}

static int read_whole_header(void)
{	/* read the whole EEPROM header and store the info in hwid_eeprom */
	int ret = eeprom_read(CONFIG_SYS_I2C_EEPROM_ADDR, 0,
			      (uchar *)&hwid_eeprom,
			      sizeof(struct bytedevkit_header));
	if (ret)
		printf("Error accessing HWID EEPROM\n");
	return ret;
}

static void set_env_vars(s8 header, s8 major, s8 minor, s8 patch)
{
	/* sets the environment variables for byteDEVKIT */
	char env_string[4] = ""; /* maximum: "255" */

	sprintf(env_string, "%d", header);
	env_set("board_header", env_string);
	sprintf(env_string, "%d", major);
	env_set("board_major", env_string);
	sprintf(env_string, "%d", minor);
	env_set("board_minor", env_string);
	sprintf(env_string, "%d", patch);
	env_set("board_patch", env_string);
}

#if defined(DEBUG)
static int prompt_agreement(void)
{
	/* returns zero if the input character is *not* y or Y */
	char prompt_input = getc();

	printf("%c\n", prompt_input);
	return (prompt_input == 'y' || prompt_input == 'Y');
}
#endif

static int get_header_and_integrity_checks(void)
{
	int ret;

	ret = read_whole_header();
	if (ret) {
		printf("ERROR while reading header\n");
		return ret;
	}
	ret = check_magic_number();
	if (ret) {
		if (ret == -EIO) {
			/* No HWID EEPROM could be accessed: we assume to be
			 * running on a byteDEVKIT version 1.2
			 */
			set_env_vars(UNDEFINED, 1, 2, UNDEFINED);
		} else {
			printf("failed reading magic number from HWID EEPROM\n");
		}
		return ret;
	}
	if (hwid_eeprom.payload_size != HEADER_PAYLOAD_SIZE) {
		printf("HWID EEPROM: unexpected payload size: %d != %d\n",
		       hwid_eeprom.payload_size, HEADER_PAYLOAD_SIZE);
		return -EINVAL;
	}
	if (crc32(0, (u8 *)&hwid_eeprom + HEADER_PAYLOAD_START,
		  hwid_eeprom.payload_size) != hwid_eeprom.crc) {
		printf("HWID EEPROM CRC validation failed!\n");
		return -EINVAL;
	}
	return 0;
}

static int write_eeprom(unsigned int addr, uchar *source, unsigned int count)
{
	const int block_size = 64; /* as specified by the reference manual */
	int offset;
	int ret = 0;

	for (int i = 0; i <= count / block_size; i++) {
		offset = i * block_size;
		ret = eeprom_write(addr, offset, source + offset, block_size);
		if (ret)
			break;
		mdelay(5);
	}
	return ret;
}

/* U-BOOT CLI COMMANDS */
int do_hwid_check(struct cmd_tbl_s *cmdtp, int flag, int argc,
		  char * const argv[])
{
	/* Checks for presence of an EEPROM on I2C bus. If no EEPROM is found
	 * we probably operate on a byteDEVKIT 1.1; otherwise we check the
	 * EEPROM contents for valid data.
	 * There's no explicit need to call this function; it gets called by
	 * `hwid get` before setting environment variables.
	 */
	int ret;

	ret = get_header_and_integrity_checks();
	if (ret) {
		printf("error checking header\n");
	} else {
		set_env_vars(hwid_eeprom.header_version, hwid_eeprom.major,
			     hwid_eeprom.minor, hwid_eeprom.patch);
	}
	return ret;
}

int do_prod_data(struct cmd_tbl_s *cmdtp, int flag, int argc,
		 char * const argv[])
{
	int ret;

	ret = get_header_and_integrity_checks();
	if (ret)
		printf("WARNING: error checking header\n");

	printf("\tPRODUCTION DATA\n");
	printf("art_nr: %d\n", hwid_eeprom.art_nr);
	printf("lot: %d\n", hwid_eeprom.lot);
	printf("lotseq: %d\n", hwid_eeprom.lotseq);
	printf("proddate: %s\n", hwid_eeprom.proddate);
	printf("flashdate: %s\n", hwid_eeprom.flashdate);
	printf("flashuser: %s\n", hwid_eeprom.flashuser);
	printf("uid: %s\n", hwid_eeprom.uid);

	return ret;
}

int do_hwid_set(struct cmd_tbl_s *cmdtp, int flag, int argc,
		char * const argv[])
{
	/* Write the magic-number, the CRC, the size of the header, as well as
	 * the values for header version, major version, minor version and patch
	 * to the EEPROM.
	 */
	struct bytedevkit_header new_header;
	int ret;

	if (argc != 11) {
		printf("wrong number of arguments!\n");
		return CMD_RET_USAGE;
	}

	strncpy(new_header.magic, MAGIC, MAGIC_SIZE);
	new_header.payload_size = (u32)HEADER_PAYLOAD_SIZE;
	new_header.header_version = CURRENT_HEADER_VERSION;
	new_header.major = (u8)simple_strtoul(argv[1], NULL, 10);
	new_header.minor = (u8)simple_strtoul(argv[2], NULL, 10);
	new_header.patch = (u8)simple_strtoul(argv[3], NULL, 10);
	new_header.art_nr = (u32)simple_strtoul(argv[4], NULL, 10);
	new_header.lot = (u32)simple_strtoul(argv[5], NULL, 10);
	new_header.lotseq = (u8)simple_strtoul(argv[6], NULL, 10);
	strncpy(new_header.proddate, argv[7], 12);
	strncpy(new_header.flashdate, argv[8], 6);
	strncpy(new_header.flashuser, argv[9], 6);
	strncpy(new_header.uid, argv[10], 16);
	new_header.crc = crc32(0, (u8 *)&new_header + HEADER_PAYLOAD_START,
			       new_header.payload_size);

	ret = write_eeprom(CONFIG_SYS_I2C_EEPROM_ADDR, (uchar *)&new_header,
			   (sizeof(struct bytedevkit_header)));
	if (!ret)
		printf("HWID EEPROM was flashed successfully\n");
	return ret;
}

#if defined(DEBUG)
int do_eeprom_reset(struct cmd_tbl_s *cmdtp, int flag, int argc,
		    char * const argv[])
{
	/* reset COUNT rows (16 bytes) of EEPROM starting at OFFSET */

	char reset_string[EEPROM_ROW];
	char byte;
	int offset;
	int count;
	int ret;

	if (argc != 4)
		return CMD_RET_USAGE;

	offset = simple_strtoul(argv[1], NULL, 16);
	byte = simple_strtoul(argv[2], NULL, 16);
	count = simple_strtoul(argv[3], NULL, 10);

	for (int i = 0; i < EEPROM_ROW; i++)
		reset_string[i] = byte;

	for (int i = 0; i < count; i++) {
		ret = eeprom_write(CONFIG_SYS_I2C_EEPROM_ADDR,
				   offset + EEPROM_ROW * i,
				   (uchar *)&reset_string,
				   sizeof(reset_string));
		if (ret)
			return ret;
		mdelay(5);
	}

	return 0;
}

int do_hwid_read(struct cmd_tbl_s *cmdtp, int flag, int argc,
		 char * const argv[])
{
	/* debug-interface to view COUNT bytes of EEPROM (like `i2c md`) */
	u8 byte;
	int count;
	int ret;

	if (argc != 2)
		return -EINVAL;

	count = simple_strtoul(argv[1], NULL, 10);

	for (int i = 0; i < count; i++) {
		if (i % EEPROM_ROW == 0)
			printf("\n%04x   ", i);

		ret = eeprom_read(CONFIG_SYS_I2C_EEPROM_ADDR, i, &byte, 1);

		if (ret != 0) {
			printf("Error reading from eeprom\n");
			return -EINVAL;
		}
		printf("%02x ", byte);
	}

	printf("\nfinished reading raw content from EEPROM\n");

	return 0;
};

int do_e2e_test(struct cmd_tbl_s *cmdtp, int flag, int argc,
		char * const argv[])
{
	/* Run an End-To-End Test over the whole module
	 *
	 * First we erase data in the EEPROM, we check back whether this
	 * succeeded.  We then write some header info into the EEPROM and check
	 * back.  Finally we read the data in the EEPROM, check if for validity
	 * and sets.  Environment Variables accordingly. We check that back as
	 * well.
	 */

	int ret;
	int e2e_argc;
	char *e2e_argv[12];

	printf("\n\t\tE2E TEST\n\n");

	printf("Reset EEPROM? (yN) ");
	if (!prompt_agreement()) {
		printf("aborted\n");
		return -UNDEFINED;
	}

	printf("\tRESETTING EEPROM\n");
	e2e_argc = 4;
	e2e_argv[0] = "eeprom_reset";
	e2e_argv[1] = "0";
	e2e_argv[2] = "ff";
	e2e_argv[3] = "4";
	inspect_args(e2e_argc, e2e_argv);
	ret = do_eeprom_reset(cmdtp, flag, e2e_argc, e2e_argv);
	if (ret) {
		printf("FAILURE (code: %d)\n", ret);
		return ret;
	}

	printf("\tREADING EEPROM\n");
	e2e_argc = 2;
	e2e_argv[0] = "read";
	e2e_argv[1] = "32";
	inspect_args(e2e_argc, e2e_argv);
	ret = do_hwid_read(cmdtp, flag, e2e_argc, e2e_argv);
	if (ret) {
		printf("FAILURE (code: %d)\n", ret);
		return ret;
	}

	printf("\tFLASH HEADER\n");
	e2e_argc = 12;
	e2e_argv[0] = "set";
	e2e_argv[1] = "0";			/* header */
	e2e_argv[2] = "1";			/* major */
	e2e_argv[3] = "3";			/* minor */
	e2e_argv[4] = "0";			/* patch */
	e2e_argv[5] = "888053501";		/* art_nr */
	e2e_argv[6] = "103";			/* lot */
	e2e_argv[7] = "1";			/* lotseq */
	e2e_argv[8] = "2022-01-01";		/* proddate */
	e2e_argv[9] = "19/20";			/* flashdate */
	e2e_argv[10] = "sdu";			/* flashuser */
	e2e_argv[11] = "1804270009737";		/* uid */

	inspect_args(e2e_argc, e2e_argv);
	ret = do_hwid_set(cmdtp, flag, e2e_argc, e2e_argv);
	if (ret) {
		printf("FAILURE (code: %d)\n", ret);
		return ret;
	}

	printf("\tCHECKING VALIDITY OF EEPROM CONTENT\n");
	e2e_argc = 1;
	e2e_argv[0] = "check";
	inspect_args(e2e_argc, e2e_argv);
	ret = do_hwid_check(cmdtp, flag, e2e_argc, e2e_argv);
	if (ret) {
		printf("FAILURE (code: %d)\n", ret);
		return ret;
	}

	printf("\n\n");

	return 0;
};

int do_info(struct cmd_tbl_s *cmdtp, int flag, int argc,
	    char * const argv[])
{
	printf("Information about this module:\n");
	printf("HWID EEPROM DEVICE: %d\n", HWID_EEPROM_DEVICE);
	printf("HWID EEPROM ADDR: 0x%X\n", CONFIG_SYS_I2C_EEPROM_ADDR);
	printf("MAGIC: \"%s\"\n", MAGIC);
	printf("HEADER PAYLOAD START: %d\n", HEADER_PAYLOAD_START);
	printf("HEADER PAYLOAD SIZE: %d\n", HEADER_PAYLOAD_SIZE);
	printf("UNDEFINED: %d\n", UNDEFINED);
	return 0;
}
#endif

static cmd_tbl_t cmd_hwid_sub[] = {
	U_BOOT_CMD_MKENT(check, 1, 1, do_hwid_check, "", ""),
	U_BOOT_CMD_MKENT(prod_data, 1, 1, do_prod_data, "", ""),
	U_BOOT_CMD_MKENT(set, 11, 1, do_hwid_set, "", ""),
#if defined(DEBUG)
	U_BOOT_CMD_MKENT(eeprom_reset, 1, 1, do_eeprom_reset, "", ""),
	U_BOOT_CMD_MKENT(read, 2, 1, do_hwid_read, "", ""),
	U_BOOT_CMD_MKENT(e2e, 1, 1, do_e2e_test, "", ""),
	U_BOOT_CMD_MKENT(info, 1, 1, do_info, "", "")
#endif
};

int do_hwid(struct cmd_tbl_s *cmdtp, int flag,  int argc,  char * const argv[])
{
	cmd_tbl_t *cmd;

	if (argc < 2) {
		/* abort and show help if no subcommand is entered */
		return CMD_RET_USAGE;
	}

	argc--; /* decrease argument counter */
	argv++; /* move argument pointer past first argument */

	cmd = find_cmd_tbl(argv[0], cmd_hwid_sub, ARRAY_SIZE(cmd_hwid_sub));
	if (cmd) {
		eeprom_init(HWID_EEPROM_DEVICE);
		return cmd->cmd(cmdtp, flag, argc, argv);
	} else {
		return CMD_RET_USAGE;
	}

	return 0;
}

U_BOOT_CMD(
	/* Here the entry-command for the U-Boot CLI is defined.  Just enter
	 * `hwid` and find yourself within a quasi-guided text-based user
	 * interface as you are probably already used to from U-Boot.
	 */
	hwid, 12, 0, do_hwid,
	"HWID EEPROM module. commands are: `check`, `prod_data`, `set`, "
#if defined(DEBUG)
	"`get`, `eeprom_reset`, `read` and `e2e`"
#endif
	,
	"check - checks the connection to the EEPROM module and the general "
	"validity\n\n"
	"hwid prod_data - prints production data\n\n"
	"hwid set MAJOR MINOR PATCH ART_NR LOT LOTSEQ PRODDATE FLASHDATE "
	"FLASHUSER UID - writes these values into HWID EEPROM\n\n"
#if defined(DEBUG)
	"hwid eeprom_reset OFFSET BYTE COUNT - resets COUNT rows (16 Bytes) "
	"OFFSET (in hex) in HWID EEPROM to BYTE (passed in hex).\n\n"
	"hwid read COUNT - reads COUNT (decimal) bytes from HWID EEPROM and "
	"writes to stdout\n\n"
	"hwid e2e - test this whole module (this will modify EEPROM and may "
	"delete precious data\n\n"
	"hwid info - information about constants in this module\n\n"
	"eeprom_reset, read, e2e and info subcommands are intended for "
	"debugging purposes\n\n"
#endif
	)
