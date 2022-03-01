// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015 bytes at work AG
 */

#include "baw_config_eeprom.h"
#include "baw_config.h"

#include <common.h>
#include <dm/uclass.h>
#include <i2c.h>
#include <hexdump.h>

#define EEPROM_ADDRESS		0x50
#define EEPROM_MAXWRITE		64	/* max. page size for writes */
#define MAGIC			0x6268
#define EEPROM_EXT_OFFSET	0x14
#define MAGIC_EXT		0x6235
#define EEPROM_TRACK_OFFSET	0x3d
#define MAGIC_TRACK		0x6e4a

struct udevice *baw_config_dev, *busp;

struct eeprom_header {
	u16 magic;
	u16 length;
} __packed;

struct eeprom_content {
	u32 pcb;
	u32 ram;
	u32 flash;
} __packed;

struct eeprom_content_ext {
	u32 artno;
	u32 lot;
	u8 lotseq;
	char proddate[12];
	char flashdate[6];
	char flashuser[6];
} __packed;

struct eeprom_content_track {
	char macaddr[18];
	char uid[16];	/* enable use of characters */
} __packed;

struct eeprom_data {
	struct eeprom_header header;
	struct eeprom_content content;
	u32 crc;
} __packed;

struct eeprom_data_ext {
	struct eeprom_header header;
	u32 crc;
	struct eeprom_content_ext content;
} __packed;

struct eeprom_data_track {
	struct eeprom_header header;
	u32 crc;
	struct eeprom_content_track content;
} __packed;

struct eeprom_write_frame {
	struct eeprom_data content;
	struct eeprom_data_ext content_ext;
	struct eeprom_data_track content_track;
} __packed;

struct eeprom_read_frame {
	struct eeprom_data content;
	struct eeprom_data_ext content_ext;
	struct eeprom_data_track content_track;
} __packed;

int baw_config_eeprom_init(void)
{
#ifdef CONFIG_DM_I2C
	int ret;

	ret = uclass_get_device_by_seq(UCLASS_I2C, 0, &busp);
	if (ret)
		return ret;

	ret = dm_i2c_probe(busp, EEPROM_ADDRESS, 0, &baw_config_dev);
	if (ret)
		return ret;

	ret = i2c_set_chip_offset_len(baw_config_dev, 2);
	if (ret)
		return ret;

#endif
	return 0;
}

static int i2c_write_rdy(void)
{
	/*
	 * Check if device is ready for write to eeprom. Write can last up to
	 * 5 ms. Poll with empty write.
	 */
	int i, ret = -5;
	u8 dummy = 0xff; /* i2c_write doesn't like to write NULL pointers */

	for (i = 0; ret < 0 && i < 10; ++i) {
#ifndef CONFIG_DM_I2C
		ret = i2c_write(EEPROM_ADDRESS, 0, 2, &dummy, 0);
#else
		ret = dm_i2c_write(baw_config_dev, 0, &dummy, 0);
#endif
		if (ret != 0)
			udelay(500);
	}

	if (ret != 0)
		printf("Error: eeprom busy, giving up.\n");

	return ret;
}

int i2c_long_write(struct eeprom_write_frame frame, int len)
{
	int wlen, ret, off = 0;

	while (len > 0) {
		if (len >= EEPROM_MAXWRITE)
			wlen = EEPROM_MAXWRITE;
		else
			wlen = len;

		if (i2c_write_rdy() != 0) {
			ret = -EIO;
			break;
		}
#ifndef CONFIG_DM_I2C
		ret = i2c_write(EEPROM_ADDRESS, off, 2, (u8 *)(&frame) + off,
				wlen);
#else
		ret = dm_i2c_write(baw_config_dev, off, (u8 *)(&frame) + off,
				   wlen);
#endif

		if (ret != 0) {
			ret = -EIO;
			break;
		}

		len -= wlen;
		off += wlen;
	}

	return ret;
}

int baw_config_eeprom_read(struct baw_config *config)
{
	struct eeprom_read_frame frame;

#ifndef CONFIG_DM_I2C
	if (i2c_read(EEPROM_ADDRESS, 0, 2, (u8 *)&frame, sizeof(frame)) != 0)
		return -3;
#else
	if (dm_i2c_read(baw_config_dev, 0, (u8 *)&frame, sizeof(frame)))
		return -12;
#endif

	if (frame.content.header.magic != MAGIC)
		return -4;

	if (frame.content.header.length != sizeof(frame.content.content))
		return -5;

	/* Cast crc32 value to u8 because of legacy code */
	if (frame.content.crc != (u8)crc32(0, (u8 *)&frame.content.content,
					   sizeof(frame.content.content)))
		return -6;

	config->pcb    = frame.content.content.pcb;
	config->ram    = frame.content.content.ram;
	config->flash  = frame.content.content.flash;

	config->ext_avail = 0;
	if (frame.content_ext.header.magic != MAGIC_EXT) {
		printf("ext magic wrong\n");
		return 0;
	}

	if (frame.content_ext.header.length != sizeof(frame.content_ext.content)) {
		printf("ext len wrong\n");
		return 0;
	}

	if (frame.content_ext.crc != crc32(0, (u8 *)&frame.content_ext.content,
					   sizeof(frame.content_ext.content))) {
		printf("ext crc wrong\n");
		return 0;
	}

	config->artno = frame.content_ext.content.artno;
	config->lot = frame.content_ext.content.lot;
	config->lotseq = frame.content_ext.content.lotseq;

	strlcpy(config->proddate, frame.content_ext.content.proddate,
		sizeof(config->proddate));
	strlcpy(config->flashdate, frame.content_ext.content.flashdate,
		sizeof(config->flashdate));
	strlcpy(config->flashuser, frame.content_ext.content.flashuser,
		sizeof(config->flashuser));

	config->ext_avail = 1;

	config->track_avail = 0;
	if (frame.content_track.header.magic != MAGIC_TRACK) {
		printf("track magic wrong\n");
		return 0;
	}

	if (frame.content_track.header.length !=
			sizeof(frame.content_track.content)) {
		printf("track len wrong\n");
		return 0;
	}

	if (frame.content_track.crc != crc32(0, (u8 *)&frame.content_track.content,
					     sizeof(frame.content_track.content))) {
		printf("track crc wrong\n");
		return 0;
	}

	strlcpy(config->macaddr, frame.content_track.content.macaddr,
		sizeof(config->macaddr));
	strlcpy(config->uid, frame.content_track.content.uid,
		sizeof(config->uid));

	config->track_avail = 1;

	return 0;
}

#if defined(CONFIG_SKIP_LOWLEVEL_INIT)

int baw_config_eeprom_write(struct baw_config *config)
{
	struct eeprom_write_frame   frame;

	frame.content.header.magic = MAGIC;
	frame.content.header.length = sizeof(frame.content.content);
	frame.content.content.pcb = config->pcb;
	frame.content.content.ram = config->ram;
	frame.content.content.flash = config->flash;
	/* Cast crc32 value to u8 because of legacy code */
	frame.content.crc = (u8)crc32(0, (const u8 *)&frame.content.content,
				      sizeof(frame.content.content));

	frame.content_ext.header.magic = MAGIC_EXT;
	frame.content_ext.header.length = sizeof(frame.content_ext.content);
	frame.content_ext.content.artno = config->artno;
	frame.content_ext.content.lot = config->lot;
	frame.content_ext.content.lotseq = config->lotseq;
	strlcpy(frame.content_ext.content.proddate, config->proddate,
		sizeof(frame.content_ext.content.proddate));
	strlcpy(frame.content_ext.content.flashdate, config->flashdate,
		sizeof(frame.content_ext.content.flashdate));
	strlcpy(frame.content_ext.content.flashuser, config->flashuser,
		sizeof(frame.content_ext.content.flashuser));
	frame.content_ext.crc = crc32(0, (u8 *)&frame.content_ext.content,
				      sizeof(frame.content_ext.content));

	frame.content_track.header.magic = MAGIC_TRACK;
	frame.content_track.header.length = sizeof(frame.content_track.content);
	strlcpy(frame.content_track.content.macaddr, config->macaddr,
		sizeof(frame.content_track.content.macaddr));
	strlcpy(frame.content_track.content.uid, config->uid,
		sizeof(frame.content_track.content.uid));
	frame.content_track.crc = crc32(0, (u8 *)&frame.content_track.content,
					sizeof(frame.content_track.content));

	if (i2c_long_write(frame, sizeof(frame)) != 0)
		return -7;

	return 0;
}

int baw_config_eeprom_erase(void)
{
	struct eeprom_write_frame   frame;

	memset(&frame, 0xff, sizeof(frame));

	if (i2c_long_write(frame, sizeof(frame)) != 0)
		return -8;

	return 0;
}

#endif
