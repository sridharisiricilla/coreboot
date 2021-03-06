/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2007-2009 coresystems GmbH
 * Copyright (C) 2015 Google Inc.
 * Copyright (C) 2015 Intel Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <arch/acpi.h>
#include <baseboard/variant.h>
#include <console/console.h>
#include <device/device.h>
#include <gpio.h>
#include <stdlib.h>
#include <soc/nhlt.h>
#include <variant/gpio.h>
#include <vendorcode/google/chromeos/chromeos.h>
#include "ec.h"

static const char *oem_id_maxim = "INTEL";
static const char *oem_table_id_maxim = "SCRDMAX";

static void mainboard_init(struct device *dev)
{
	mainboard_ec_init();
}

static uint8_t adi_codec_enable(void)
{
#ifdef AUDIO_DB_ID
	return gpio_get(AUDIO_DB_ID);
#else
	return 1;
#endif
}

static uint8_t max_codec_enable(void)
{
#ifdef AUDIO_DB_ID
	return gpio_get(AUDIO_DB_ID) ? 0 : 1;
#else
	return 1;
#endif
}

static unsigned long mainboard_write_acpi_tables(
	struct device *device, unsigned long current, acpi_rsdp_t *rsdp)
{
	uintptr_t start_addr;
	uintptr_t end_addr;
	struct nhlt *nhlt;
	const char *oem_id = NULL;
	const char *oem_table_id = NULL;

	start_addr = current;

	nhlt = nhlt_init();

	if (nhlt == NULL)
		return start_addr;

	/* 2 Channel DMIC array. */
	if (nhlt_soc_add_dmic_array(nhlt, 2))
		printk(BIOS_ERR, "Couldn't add 2CH DMIC array.\n");

	/* 4 Channel DMIC array. */
	if (CONFIG(NHLT_DMIC_4CH))
		if (nhlt_soc_add_dmic_array(nhlt, 4))
			printk(BIOS_ERR, "Couldn't add 4CH DMIC arrays.\n");

	/* ADI Smart Amps for left and right. */
	if (CONFIG(NHLT_SSM4567) && adi_codec_enable())
		if (nhlt_soc_add_ssm4567(nhlt, AUDIO_LINK_SSP0))
			printk(BIOS_ERR, "Couldn't add ssm4567.\n");

	/* MAXIM Smart Amps for left and right. */
	if (CONFIG(NHLT_MAX98357) && max_codec_enable()) {
		if (nhlt_soc_add_max98357(nhlt, AUDIO_LINK_SSP0))
			printk(BIOS_ERR, "Couldn't add max98357.\n");

		oem_id = oem_id_maxim;
		oem_table_id = oem_table_id_maxim;
	}

	/* NAU88l25 Headset codec. */
	if (nhlt_soc_add_nau88l25(nhlt, AUDIO_LINK_SSP1))
		printk(BIOS_ERR, "Couldn't add headset codec.\n");

	end_addr = nhlt_soc_serialize_oem_overrides(nhlt, start_addr,
					 oem_id, oem_table_id, 0);

	if (end_addr != start_addr)
		acpi_add_table(rsdp, (void *)start_addr);

	return end_addr;
}

/*
 * mainboard_enable is executed as first thing after
 * enumerate_buses().
 */
static void mainboard_enable(struct device *dev)
{
	dev->ops->init = mainboard_init;
	dev->ops->write_acpi_tables = mainboard_write_acpi_tables;
	dev->ops->acpi_inject_dsdt_generator = chromeos_dsdt_generator;
}

static void mainboard_chip_init(void *chip_info)
{
	/* Configure pads prior to SiliconInit() in case there's any
	 * dependencies during hardware initialization. */
	gpio_configure_pads(gpio_table, ARRAY_SIZE(gpio_table));
}

struct chip_operations mainboard_ops = {
	.init = mainboard_chip_init,
	.enable_dev = mainboard_enable,
};
