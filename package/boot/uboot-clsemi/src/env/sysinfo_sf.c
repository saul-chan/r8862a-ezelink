/*
 *  Copyright (c) 2021-2024, Clourney Semiconductor. All rights reserved.
 *
 *  This software and/or documentation is licensed by Clourney Semiconductor under limited terms and conditions.
 *  Reproduction and redistribution in binary or source form, with or without modification,
 *  for use solely in conjunction with a Clourney Semiconductor chipset, is permitted in condition which
 *  must retain the above copyright notice.
 *
 *  By using this software and/or documentation, you agree to the limited terms and conditions.
 */

#include <common.h>
#include <dm.h>
#include <env.h>
#include <env_internal.h>
#include <flash.h>
#include <malloc.h>
#include <spi.h>
#include <spi_flash.h>
#include <search.h>
#include <errno.h>
#include <uuid.h>
#include <asm/cache.h>
#include <asm/global_data.h>
#include <dm/device-internal.h>
#include <u-boot/crc.h>

#ifndef CONFIG_SPL_BUILD
#define INITSYSINFO
#endif

#define	OFFSET_INVALID		(~(u32)0)

#ifdef CONFIG_SYSINFO_OFFSET_REDUND
#define SYSINFO_OFFSET_REDUND	CONFIG_SYSINFO_OFFSET_REDUND

static ulong sysinfo_offset		= CONFIG_SYSINFO_OFFSET;
static ulong sysinfo_new_offset	= CONFIG_SYSINFO_OFFSET_REDUND;

#else

#define SYSINFO_OFFSET_REDUND	OFFSET_INVALID

#endif /* CONFIG_SYSINFO_OFFSET_REDUND */

DECLARE_GLOBAL_DATA_PTR;

static int setup_flash_device(struct spi_flash **sysinfo_flash)
{
#if CONFIG_IS_ENABLED(DM_SPI_FLASH)
	struct udevice *new;
	int	ret;

	/* speed and mode will be read from DT */
	ret = spi_flash_probe_bus_cs(CONFIG_ENV_SPI_BUS, CONFIG_ENV_SPI_CS,
				     CONFIG_ENV_SPI_MAX_HZ, CONFIG_ENV_SPI_MODE,
				     &new);
	if (ret) {
		sysinfo_env_set_default("spi_flash_probe_bus_cs() failed", 0);
		return ret;
	}

	*sysinfo_flash = dev_get_uclass_priv(new);
#else
	*sysinfo_flash = spi_flash_probe(CONFIG_ENV_SPI_BUS, CONFIG_ENV_SPI_CS,
				     CONFIG_ENV_SPI_MAX_HZ, CONFIG_ENV_SPI_MODE);
	if (!*sysinfo_flash) {
		sysinfo_env_set_default("spi_flash_probe() failed", 0);
		return -EIO;
	}
#endif
	return 0;
}

#if defined(CONFIG_SYSINFO_OFFSET_REDUND)
static int sysinfo_sf_save(void)
{
	env_t   env_new;
	char	*saved_buffer = NULL, flag = ENV_REDUND_OBSOLETE;
	u32	saved_size = 0, saved_offset = 0, sector;
	u32	sect_size = CONFIG_SYSINFO_SECT_SIZE;
	int	ret;
	struct spi_flash *sysinfo_flash;

	ret = setup_flash_device(&sysinfo_flash);
	if (ret)
		return ret;

	if (IS_ENABLED(CONFIG_SYSINFO_SECT_SIZE_AUTO))
		sect_size = sysinfo_flash->mtd.erasesize;

	ret = sysinfo_env_export(&env_new);
	if (ret)
		return -EIO;
	env_new.flags	= ENV_REDUND_ACTIVE;

	if (gd->sysinfo_valid == ENV_VALID) {
		sysinfo_new_offset = CONFIG_SYSINFO_OFFSET_REDUND;
		sysinfo_offset = CONFIG_SYSINFO_OFFSET;
	} else {
		sysinfo_new_offset = CONFIG_SYSINFO_OFFSET;
		sysinfo_offset = CONFIG_SYSINFO_OFFSET_REDUND;
	}

	/* Is the sector larger than the env (i.e. embedded) */
	if (sect_size > CONFIG_SYSINFO_SIZE) {
		saved_size = sect_size - CONFIG_SYSINFO_SIZE;
		saved_offset = sysinfo_new_offset + CONFIG_SYSINFO_SIZE;
		saved_buffer = memalign(ARCH_DMA_MINALIGN, saved_size);
		if (!saved_buffer) {
			ret = -ENOMEM;
			goto done;
		}
		ret = spi_flash_read(sysinfo_flash, saved_offset,
					saved_size, saved_buffer);
		if (ret)
			goto done;
	}

	sector = DIV_ROUND_UP(CONFIG_SYSINFO_SIZE, sect_size);

	puts("Erasing SPI flash...");
	ret = spi_flash_erase(sysinfo_flash, sysinfo_new_offset,
				sector * sect_size);
	if (ret)
		goto done;

	puts("Writing to SPI flash...");

	ret = spi_flash_write(sysinfo_flash, sysinfo_new_offset,
		CONFIG_SYSINFO_SIZE, &env_new);
	if (ret)
		goto done;

	if (sect_size > CONFIG_SYSINFO_SIZE) {
		ret = spi_flash_write(sysinfo_flash, saved_offset,
					saved_size, saved_buffer);
		if (ret)
			goto done;
	}

	ret = spi_flash_write(sysinfo_flash, sysinfo_offset + offsetof(env_t, flags),
				sizeof(env_new.flags), &flag);
	if (ret)
		goto done;

	puts("done\n");

	gd->sysinfo_valid = gd->sysinfo_valid == ENV_REDUND ? ENV_VALID : ENV_REDUND;

	printf("Valid environment: %d\n", (int)gd->sysinfo_valid);

done:
	spi_flash_free(sysinfo_flash);

	if (saved_buffer)
		free(saved_buffer);

	return ret;
}

static int sysinfo_sf_load(void)
{
	int ret;
	int read1_fail, read2_fail;
	env_t *tmp_env1, *tmp_env2;
	struct spi_flash *sysinfo_flash;

	tmp_env1 = (env_t *)memalign(ARCH_DMA_MINALIGN,
			CONFIG_SYSINFO_SIZE);
	tmp_env2 = (env_t *)memalign(ARCH_DMA_MINALIGN,
			CONFIG_SYSINFO_SIZE);
	if (!tmp_env1 || !tmp_env2) {
		sysinfo_env_set_default("malloc() failed", 0);
		ret = -EIO;
		goto out;
	}

	ret = setup_flash_device(&sysinfo_flash);
	if (ret)
		goto out;

	read1_fail = spi_flash_read(sysinfo_flash, CONFIG_SYSINFO_OFFSET,
				    CONFIG_SYSINFO_SIZE, tmp_env1);
	read2_fail = spi_flash_read(sysinfo_flash, CONFIG_SYSINFO_OFFSET_REDUND,
				    CONFIG_SYSINFO_SIZE, tmp_env2);

	ret = sysinfo_env_import_redund((char *)tmp_env1, read1_fail, (char *)tmp_env2,
				read2_fail, H_EXTERNAL);

	spi_flash_free(sysinfo_flash);
out:
	free(tmp_env1);
	free(tmp_env2);

	return ret;
}
#else
static int sysinfo_sf_save(void)
{
	u32	saved_size = 0, saved_offset = 0, sector;
	u32	sect_size = CONFIG_SYSINFO_SECT_SIZE;
	char	*saved_buffer = NULL;
	int	ret = 1;
	env_t	env_new;
	struct spi_flash *sysinfo_flash;

	ret = setup_flash_device(&sysinfo_flash);
	if (ret)
		return ret;

	if (IS_ENABLED(CONFIG_SYSINFO_SECT_SIZE_AUTO))
		sect_size = sysinfo_flash->mtd.erasesize;

	/* Is the sector larger than the env (i.e. embedded) */
	if (sect_size > CONFIG_SYSINFO_SIZE) {
		saved_size = sect_size - CONFIG_SYSINFO_SIZE;
		saved_offset = CONFIG_SYSINFO_OFFSET + CONFIG_SYSINFO_SIZE;
		saved_buffer = malloc(saved_size);
		if (!saved_buffer)
			goto done;

		ret = spi_flash_read(sysinfo_flash, saved_offset,
			saved_size, saved_buffer);
		if (ret)
			goto done;
	}

	ret = sysinfo_env_export(&env_new);
	if (ret)
		goto done;

	sector = DIV_ROUND_UP(CONFIG_SYSINFO_SIZE, sect_size);

	puts("Erasing SPI flash...");
	ret = spi_flash_erase(sysinfo_flash, CONFIG_SYSINFO_OFFSET,
		sector * sect_size);
	if (ret)
		goto done;

	puts("Writing to SPI flash...");
	ret = spi_flash_write(sysinfo_flash, CONFIG_SYSINFO_OFFSET,
		CONFIG_SYSINFO_SIZE, &env_new);
	if (ret)
		goto done;

	if (sect_size > CONFIG_SYSINFO_SIZE) {
		ret = spi_flash_write(sysinfo_flash, saved_offset,
			saved_size, saved_buffer);
		if (ret)
			goto done;
	}

	ret = 0;
	puts("done\n");

done:
	spi_flash_free(sysinfo_flash);

	if (saved_buffer)
		free(saved_buffer);

	return ret;
}

static int sysinfo_sf_load(void)
{
	int ret;
	char *buf = NULL;
	struct spi_flash *sysinfo_flash;

	buf = (char *)memalign(ARCH_DMA_MINALIGN, CONFIG_SYSINFO_SIZE);
	if (!buf) {
		sysinfo_env_set_default("malloc() failed", 0);
		return -EIO;
	}

	ret = setup_flash_device(&sysinfo_flash);
	if (ret)
		goto out;

	ret = spi_flash_read(sysinfo_flash,
		CONFIG_SYSINFO_OFFSET, CONFIG_SYSINFO_SIZE, buf);
	if (ret) {
		sysinfo_env_set_default("spi_flash_read() failed", 0);
		goto err_read;
	}

	ret = sysinfo_env_import(buf, 1, H_EXTERNAL);
	if (!ret)
		gd->sysinfo_valid = ENV_VALID;

err_read:
	spi_flash_free(sysinfo_flash);
out:
	free(buf);

	return ret;
}
#endif

static int sysinfo_sf_erase(void)
{
	int ret;
	env_t env;
	struct spi_flash *sysinfo_flash;

	ret = setup_flash_device(&sysinfo_flash);
	if (ret)
		return ret;

	memset(&env, 0, sizeof(env_t));
	ret = spi_flash_write(sysinfo_flash, CONFIG_SYSINFO_OFFSET, CONFIG_SYSINFO_SIZE, &env);
	if (ret)
		goto done;

	if (SYSINFO_OFFSET_REDUND != OFFSET_INVALID)
		ret = spi_flash_write(sysinfo_flash, SYSINFO_OFFSET_REDUND, CONFIG_SYSINFO_SIZE, &env);

done:
	spi_flash_free(sysinfo_flash);

	return ret;
}

#if CONFIG_SYSINFO_ADDR != 0x0
__weak void *sysinfo_sf_get_env_addr(void)
{
	return (void *)CONFIG_SYSINFO_ADDR;
}
#endif

#if defined(INITSYSINFO) && (CONFIG_SYSINFO_ADDR != 0x0)
/*
 * check if System Information on CONFIG_SYSINFO_ADDR is valid.
 */
static int sysinfo_sf_init_addr(void)
{
	env_t *env_ptr = (env_t *)sysinfo_sf_get_env_addr();

	if (crc32(0, env_ptr->data, SYSINFO_SIZE) == env_ptr->crc) {
		gd->env_addr = (ulong)&(env_ptr->data);
		gd->sysinfo_valid = ENV_VALID;
	} else {
		gd->sysinfo_valid = ENV_INVALID;
	}

	return 0;
}
#endif

static int sysinfo_sf_init(void)
{
#if defined(INITSYSINFO) && (CONFIG_SYSINFO_ADDR != 0x0)
	return sysinfo_sf_init_addr();
#endif
	/*
	 * return here -ENOENT, so env_init()
	 * can set the init bit and later if no
	 * other Environment storage is defined
	 * can set the default environment
	 */
	return -ENOENT;
}

U_BOOT_SYSINFO_LOCATION(sf) = {
	.location	= ENVL_SPI_FLASH,
	SYSINFO_NAME("SPIFlash")
	.load		= sysinfo_sf_load,
	.save		= SYSINFO_SAVE_PTR(sysinfo_sf_save),
	.init		= sysinfo_sf_init,
};
