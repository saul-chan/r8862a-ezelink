/*
 *  Copyright (c) 2021-2024, Clourney Semiconductor. All rights reserved.
 *
 *  This software and/or documentation is licensed by Clourney Semiconductor under limited terms and conditions.
 *  Reproduction and redistribution in binary or source form, with or without modification,
 *  for use solely in conjunction with a Clourney Semiconductor chipset, is permitted in condition which
 *  must retain the above copyright notice.
 *
 *  By using this software and/or documentation, you agree to the limited terms and conditions.
 *
 *  System Information boarddata
 *
 */

#include <common.h>
#include <env.h>
#include <env_internal.h>
#include <log.h>
#include <asm/global_data.h>
#include <linux/bitops.h>
#include <linux/bug.h>

DECLARE_GLOBAL_DATA_PTR;

static struct env_driver *_sysinfo_env_driver_lookup(enum env_location loc)
{
	struct env_driver *drv;
	const int n_ents = ll_entry_count(struct env_driver, sysinfo_driver);
	struct env_driver *entry;

	drv = ll_entry_start(struct env_driver, sysinfo_driver);
	for (entry = drv; entry != drv + n_ents; entry++) {
		if (loc == entry->location)
			return entry;
	}

	/* Not found */
	return NULL;
}

static enum env_location sysinfo_locations[] = {
#ifdef CONFIG_SYSINFO_IS_IN_MTD
	ENVL_MTD,
#endif
#ifdef CONFIG_SYSINFO_IS_IN_UBI
	ENVL_UBI,
#endif
#ifdef CONFIG_ENV_IS_IN_MMC
	ENVL_MMC,
#endif
#ifdef CONFIG_ENV_IS_IN_SPI_FLASH
	ENVL_SPI_FLASH,
#endif
};

static bool sysinfo_env_has_inited(enum env_location location)
{
	return gd->sysinfo_has_init & BIT(location);
}

static void sysinfo_env_set_inited(enum env_location location)
{
	/*
	 * We're using a 32-bits bitmask stored in gd (sysinfo_has_init)
	 * using the above enum value as the bit index. We need to
	 * make sure that we're not overflowing it.
	 */
	BUILD_BUG_ON(ENVL_COUNT > BITS_PER_LONG);

	gd->sysinfo_has_init |= BIT(location);
}

/**
 * sysinfo_env_get_location() - Returns the best env location for a board
 * @op: operations performed on the environment
 * @prio: priority between the multiple environments, 0 being the
 *        highest priority
 *
 * This will return the preferred environment for the given priority.
 * This is overridable by boards if they need to.
 *
 * All implementations are free to use the operation, the priority and
 * any other data relevant to their choice, but must take into account
 * the fact that the lowest prority (0) is the most important location
 * in the system. The following locations should be returned by order
 * of descending priorities, from the highest to the lowest priority.
 *
 * Returns:
 * an enum env_location value on success, a negative error code otherwise
 */
__weak enum env_location sysinfo_env_get_location(enum env_operation op, int prio)
{
	if (prio >= ARRAY_SIZE(sysinfo_locations))
		return ENVL_UNKNOWN;

	return sysinfo_locations[prio];
}


/**
 * sysinfo_env_driver_lookup() - Finds the most suited environment location
 * @op: operations performed on the environment
 * @prio: priority between the multiple environments, 0 being the
 *        highest priority
 *
 * This will try to find the available environment with the highest
 * priority in the system.
 *
 * Returns:
 * NULL on error, a pointer to a struct env_driver otherwise
 */
static struct env_driver *sysinfo_env_driver_lookup(enum env_operation op, int prio)
{
	enum env_location loc = sysinfo_env_get_location(op, prio);
	struct env_driver *drv;

	if (loc == ENVL_UNKNOWN)
		return NULL;

	drv = _sysinfo_env_driver_lookup(loc);
	if (!drv) {
		debug("%s: No sysinfo data driver for location %d\n", __func__,
		      loc);
		return NULL;
	}

	return drv;
}

int sysinfo_env_load(void)
{
	struct env_driver *drv;
	int best_prio = -1;
	int prio;

	for (prio = 0; (drv = sysinfo_env_driver_lookup(ENVOP_LOAD, prio)); prio++) {
		int ret;

		if (!sysinfo_env_has_inited(drv->location))
			continue;

		printf("Loading System Information from %s... \n", drv->name);
		/*
		 * In error case, the error message must be printed during
		 * drv->load() in some underlying API, and it must be exactly
		 * one message.
		 */
		ret = drv->load();
		if (!ret) {
			printf("OK\n");
			gd->sysinfo_load_prio = prio;
			return 0;
		} else if (ret == -ENOMSG) {
			/* Handle "bad CRC" case */
			if (best_prio == -1)
				best_prio = prio;
		} else {
			debug("Failed (%d)\n", ret);
		}
	}

	/*
	 * In case of invalid environment, we set the 'default' env location
	 * to the best choice, i.e.:
	 *   1. Environment location with bad CRC, if such location was found
	 *   2. Otherwise use the location with highest priority
	 *
	 * This way, next calls to env_save() will restore the environment
	 * at the right place.
	 */
	if (best_prio >= 0)
		debug("Selecting environment with bad CRC\n");
	else
		best_prio = 0;

	gd->sysinfo_load_prio = best_prio;

	return -ENODEV;
}

int sysinfo_env_reload(void)
{
	struct env_driver *drv;

	drv = sysinfo_env_driver_lookup(ENVOP_LOAD, gd->sysinfo_load_prio);
	if (drv) {
		int ret;

		printf("Loading System Information from %s... \n", drv->name);

		if (!sysinfo_env_has_inited(drv->location)) {
			printf("not initialized\n");
			return -ENODEV;
		}

		ret = drv->load();
		if (ret)
			printf("Failed (%d)\n", ret);
		else
			printf("OK\n");

		if (!ret)
			return 0;
	}

	return -ENODEV;
}

int sysinfo_env_save(void)
{
	struct env_driver *drv;

	drv = sysinfo_env_driver_lookup(ENVOP_SAVE, gd->sysinfo_load_prio);
	if (drv) {
		int ret;

		printf("Saving System Information to %s... ", drv->name);
		if (!drv->save) {
			printf("not possible\n");
			return -ENODEV;
		}

		if (!sysinfo_env_has_inited(drv->location)) {
			printf("not initialized\n");
			return -ENODEV;
		}

		ret = drv->save();
		if (ret)
			printf("Failed (%d)\n", ret);
		else
			printf("OK\n");

		if (!ret)
			return 0;
	}

	return -ENODEV;
}

int sysinfo_env_erase(void)
{
	struct env_driver *drv;

	drv = sysinfo_env_driver_lookup(ENVOP_ERASE, gd->sysinfo_load_prio);
	if (drv) {
		int ret;

		if (!drv->erase)
			return -ENODEV;

		if (!sysinfo_env_has_inited(drv->location))
			return -ENODEV;

		printf("Erasing System Information on %s... ", drv->name);
		ret = drv->erase();
		if (ret)
			printf("Failed (%d)\n", ret);
		else
			printf("OK\n");

		if (!ret)
			return 0;
	}

	return -ENODEV;
}

int sysinfo_env_init(void)
{
	struct env_driver *drv;
	int ret = -ENOENT;
	int prio;

	for (prio = 0; (drv = sysinfo_env_driver_lookup(ENVOP_INIT, prio)); prio++) {
		if (!drv->init || !(ret = drv->init()))
			sysinfo_env_set_inited(drv->location);
		if (ret == -ENOENT)
			sysinfo_env_set_inited(drv->location);

		debug("%s: System Information %s init done (ret=%d)\n", __func__,
		      drv->name, ret);

		if (gd->sysinfo_valid == ENV_INVALID)
			ret = -ENOENT;
	}

	if (!prio)
		return -ENODEV;

	if (ret == -ENOENT) {
		gd->sysinfo_addr = (ulong)&default_sysinfo[0];
		gd->sysinfo_valid = ENV_VALID;

		return 0;
	}

	return ret;
}
