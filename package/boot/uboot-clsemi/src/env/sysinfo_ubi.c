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
#include <asm/global_data.h>

#include <command.h>
#include <env.h>
#include <env_internal.h>
#include <errno.h>
#include <malloc.h>
#include <memalign.h>
#include <search.h>
#include <ubi_uboot.h>
#undef crc32

#define _QUOTE(x) #x
#define QUOTE(x) _QUOTE(x)

#if (CONFIG_SYSINFO_UBI_VID_OFFSET == 0)
 #define SYSINFO_UBI_VID_OFFSET NULL
#else
 #define SYSINFO_UBI_VID_OFFSET QUOTE(CONFIG_SYSINFO_UBI_VID_OFFSET)
#endif

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_CMD_SAVESYSINFO
#ifdef CONFIG_SYS_REDUNDAND_ENVIRONMENT
static int sysinfo_ubi_save(void)
{
	ALLOC_CACHE_ALIGN_BUFFER(env_t, env_new, 1);
	int ret;

	ret = sysinfo_env_export(env_new);
	if (ret)
		return ret;

	if (ubi_part(CONFIG_SYSINFO_UBI_PART, SYSINFO_UBI_VID_OFFSET)) {
		printf("\n** Cannot find mtd partition \"%s\"\n",
		       CONFIG_SYSINFO_UBI_PART);
		return 1;
	}

	if (gd->sysinfo_valid == ENV_VALID) {
		puts("Writing to redundant UBI... ");
		if (ubi_volume_write(CONFIG_SYSINFO_UBI_VOLUME_REDUND,
				     (void *)env_new, CONFIG_SYSINFO_SIZE)) {
			printf("\n** Unable to write env to %s:%s **\n",
			       CONFIG_SYSINFO_UBI_PART,
			       CONFIG_SYSINFO_UBI_VOLUME_REDUND);
			return 1;
		}
	} else {
		puts("Writing to UBI... ");
		if (ubi_volume_write(CONFIG_SYSINFO_UBI_VOLUME,
				     (void *)env_new, CONFIG_SYSINFO_SIZE)) {
			printf("\n** Unable to write env to %s:%s **\n",
			       CONFIG_SYSINFO_UBI_PART,
			       CONFIG_SYSINFO_UBI_VOLUME);
			return 1;
		}
	}

	puts("done\n");

	gd->sysinfo_valid = gd->sysinfo_valid == ENV_REDUND ? ENV_VALID : ENV_REDUND;

	return 0;
}
#else /* ! CONFIG_SYS_REDUNDAND_ENVIRONMENT */
static int sysinfo_ubi_save(void)
{
	ALLOC_CACHE_ALIGN_BUFFER(env_t, env_new, 1);
	int ret;

	ret = sysinfo_env_export(env_new);
	if (ret)
		return ret;

	if (ubi_part(CONFIG_SYSINFO_UBI_PART, SYSINFO_UBI_VID_OFFSET)) {
		printf("\n** Cannot find mtd partition \"%s\"\n",
		       CONFIG_SYSINFO_UBI_PART);
		return 1;
	}

	if (ubi_volume_write(CONFIG_SYSINFO_UBI_VOLUME, (void *)env_new,
			     CONFIG_SYSINFO_SIZE)) {
		printf("\n** Unable to write env to %s:%s **\n",
		       CONFIG_SYSINFO_UBI_PART, CONFIG_SYSINFO_UBI_VOLUME);
		return 1;
	}

	puts("done\n");
	return 0;
}
#endif /* CONFIG_SYS_REDUNDAND_ENVIRONMENT */
#endif /* CONFIG_CMD_SAVESYSINFO */

int __weak sysinfo_ubi_volume_create(const char *volume)
{
	struct ubi_volume *vol;

	vol = ubi_find_volume((char *)volume);
	if (vol)
		return 0;

	return ubi_create_vol((char *)volume, CONFIG_SYSINFO_SIZE, true,
			      UBI_VOL_NUM_AUTO, false);
}

#ifdef CONFIG_SYS_REDUNDAND_ENVIRONMENT
static int sysinfo_ubi_load(void)
{
	ALLOC_CACHE_ALIGN_BUFFER(char, env1_buf, CONFIG_SYSINFO_SIZE);
	ALLOC_CACHE_ALIGN_BUFFER(char, env2_buf, CONFIG_SYSINFO_SIZE);
	int read1_fail, read2_fail;
	env_t *tmp_env1, *tmp_env2;

	/*
	 * In case we have restarted u-boot there is a chance that buffer
	 * contains old environment (from the previous boot).
	 * If UBI volume is zero size, ubi_volume_read() doesn't modify the
	 * buffer.
	 * We need to clear buffer manually here, so the invalid CRC will
	 * cause setting default environment as expected.
	 */
	memset(env1_buf, 0x0, CONFIG_SYSINFO_SIZE);
	memset(env2_buf, 0x0, CONFIG_SYSINFO_SIZE);

	tmp_env1 = (env_t *)env1_buf;
	tmp_env2 = (env_t *)env2_buf;

	if (ubi_part(CONFIG_SYSINFO_UBI_PART, SYSINFO_UBI_VID_OFFSET)) {
		printf("\n** Cannot find mtd partition \"%s\"\n",
		       CONFIG_SYSINFO_UBI_PART);
		sysinfo_env_set_default(NULL, 0);
		return -EIO;
	}

	if (IS_ENABLED(CONFIG_SYSINFO_UBI_VOLUME_CREATE)) {
		sysinfo_ubi_volume_create(CONFIG_SYSINFO_UBI_VOLUME);
		sysinfo_ubi_volume_create(CONFIG_SYSINFO_UBI_VOLUME_REDUND);
	}

	read1_fail = ubi_volume_read(CONFIG_SYSINFO_UBI_VOLUME, (void *)tmp_env1,
				     CONFIG_SYSINFO_SIZE);
	if (read1_fail)
		printf("\n** Unable to read env from %s:%s **\n",
		       CONFIG_SYSINFO_UBI_PART, CONFIG_SYSINFO_UBI_VOLUME);

	read2_fail = ubi_volume_read(CONFIG_SYSINFO_UBI_VOLUME_REDUND,
				     (void *)tmp_env2, CONFIG_SYSINFO_SIZE);
	if (read2_fail)
		printf("\n** Unable to read redundant env from %s:%s **\n",
		       CONFIG_SYSINFO_UBI_PART, CONFIG_SYSINFO_UBI_VOLUME_REDUND);

	return sysinfo_env_import_redund((char *)tmp_env1, read1_fail, (char *)tmp_env2,
				 read2_fail, H_EXTERNAL);
}
#else /* ! CONFIG_SYS_REDUNDAND_ENVIRONMENT */
static int sysinfo_ubi_load(void)
{
	ALLOC_CACHE_ALIGN_BUFFER(char, buf, CONFIG_SYSINFO_SIZE);

	/*
	 * In case we have restarted u-boot there is a chance that buffer
	 * contains old environment (from the previous boot).
	 * If UBI volume is zero size, ubi_volume_read() doesn't modify the
	 * buffer.
	 * We need to clear buffer manually here, so the invalid CRC will
	 * cause setting default environment as expected.
	 */
	memset(buf, 0x0, CONFIG_SYSINFO_SIZE);

	if (ubi_part(CONFIG_SYSINFO_UBI_PART, SYSINFO_UBI_VID_OFFSET)) {
		printf("\n** Cannot find mtd partition \"%s\"\n",
		       CONFIG_SYSINFO_UBI_PART);
		sysinfo_set_default(NULL, 0);
		return -EIO;
	}

	if (IS_ENABLED(CONFIG_SYSINFO_UBI_VOLUME_CREATE))
		sysinfo_ubi_volume_create(CONFIG_SYSINFO_UBI_VOLUME);

	if (ubi_volume_read(CONFIG_SYSINFO_UBI_VOLUME, buf, CONFIG_SYSINFO_SIZE)) {
		printf("\n** Unable to read env from %s:%s **\n",
		       CONFIG_SYSINFO_UBI_PART, CONFIG_SYSINFO_UBI_VOLUME);
		sysinfo_set_default(NULL, 0);
		return -EIO;
	}

	return sysinfo_env_import(buf, 1, H_EXTERNAL);
}
#endif /* CONFIG_SYS_REDUNDAND_ENVIRONMENT */

U_BOOT_SYSINFO_LOCATION(ubi) = {
	.location	= ENVL_UBI,
	SYSINFO_NAME("UBI")
	.load		= sysinfo_ubi_load,
	.save		= sysinfo_save_ptr(sysinfo_ubi_save),
};
