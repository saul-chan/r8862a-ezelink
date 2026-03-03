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

/*
 * Support for persistent sysinfo data
 *
 * The "sysinfo" is stored on external storage as a list of '\0'
 * terminated "name=value" strings. The end of the list is marked by
 * a double '\0'. The sysinfo is preceded by a 32 bit CRC over
 * the data part and, in case of redundant sysinfo, a byte of
 * flags.
 *
 * This linearized representation will also be used before
 * relocation, i. e. as long as we don't have a full C runtime
 * sysinfo data. After that, we use a hash table.
 */

#include <common.h>
#include <cli.h>
#include <command.h>
#include <console.h>
#include <env.h>
#include <env_internal.h>
#include <log.h>
#include <search.h>
#include <errno.h>
#include <malloc.h>
#include <mapmem.h>
#include <asm/global_data.h>
#include <linux/bitops.h>
#include <u-boot/crc.h>
#include <linux/stddef.h>
#include <asm/byteorder.h>
#include <asm/io.h>

DECLARE_GLOBAL_DATA_PTR;

#if	defined(CONFIG_SYSINFO_IS_IN_UBI)		|| \
	defined(CONFIG_SYSINFO_IS_IN_MTD)		|| \
	defined(CONFIG_SYSINFO_IS_IN_SPI_FLASH)

#define SYSINFO_IS_IN_DEVICE

#endif

#if	!defined(SYSINFO_IS_IN_DEVICE)
# error Define one of CONFIG_SYSINFO_IS_IN_{UBI|MTD|SPI_FLASH}
#endif

/*
 * This variable is incremented on each do_sysinfo_set(), so it can
 * be used via env_get_id() as an indication, if the sysinfo
 * has changed or not. So it is possible to reread an sysinfo
 * variable only if the sysinfo data was changed ... done so for
 * example in NetInitLoop()
 */
static int sysinfo_id = 1;

/*
 * Command interface: print one or all sysinfo variables
 *
 * Returns 0 in case of error, or length of printed string
 */
static int sysinfo_print(char *name, int flag)
{
	char *res = NULL;
	ssize_t len;

	if (name) {		/* print a single name */
		struct env_entry e, *ep;

		e.key = name;
		e.data = NULL;
		hsearch_r(e, ENV_FIND, &ep, &sysinfo_htab, flag);
		if (ep == NULL)
			return 0;
		len = printf("%s=%s\n", ep->key, ep->data);
		return len;
	}

	/* print whole list */
	len = hexport_r(&sysinfo_htab, '\n', flag, &res, 0, 0, NULL);

	if (len > 0) {
		puts(res);
		free(res);
		return len;
	}

	/* should never happen */
	printf("## Error: cannot export sysinfo data\n");
	return 0;
}

static int do_sysinfo_print(struct cmd_tbl *cmdtp, int flag, int argc,
			char *const argv[])
{
	int i;
	int rcode = 0;
	int env_flag = H_HIDE_DOT;

	if (argc > 1 && argv[1][0] == '-' && argv[1][1] == 'a') {
		argc--;
		argv++;
		env_flag &= ~H_HIDE_DOT;
	}

	if (argc == 1) {
		/* print all env vars */
		rcode = sysinfo_print(NULL, env_flag);
		if (!rcode)
			return 1;
		printf("\nEnvironment size: %d/%ld bytes\n",
			rcode, (ulong)ENV_SIZE);
		return 0;
	}

	/* print selected env vars */
	env_flag &= ~H_HIDE_DOT;
	for (i = 1; i < argc; ++i) {
		int rc = sysinfo_print(argv[i], env_flag);
		if (!rc) {
			printf("## Error: \"%s\" not defined\n", argv[i]);
			++rcode;
		}
	}

	return rcode;
}

/*
 * Set a new sysinfo variable,
 * or replace or delete an existing one.
 */
static int _do_sysinfo_set(int flag, int argc, char *const argv[], int env_flag)
{
	int   i, len;
	char  *name, *value, *s;
	struct env_entry e, *ep;

	debug("Initial value for argc=%d\n", argc);

	while (argc > 1 && **(argv + 1) == '-') {
		char *arg = *++argv;

		--argc;
		while (*++arg) {
			switch (*arg) {
			case 'f':		/* force */
				env_flag |= H_FORCE;
				break;
			default:
				return CMD_RET_USAGE;
			}
		}
	}
	debug("Final value for argc=%d\n", argc);
	name = argv[1];

	if (strchr(name, '=')) {
		printf("## Error: illegal character '='"
		       "in variable name \"%s\"\n", name);
		return 1;
	}

	sysinfo_id++;

	/* Delete only ? */
	if (argc < 3 || argv[2] == NULL) {
		int rc = hdelete_r(name, &sysinfo_htab, env_flag);

		/* If the variable didn't exist, don't report an error */
		return rc && rc != -ENOENT ? 1 : 0;
	}

	/*
	 * Insert / replace new value
	 */
	for (i = 2, len = 0; i < argc; ++i)
		len += strlen(argv[i]) + 1;

	value = malloc(len);
	if (value == NULL) {
		printf("## Can't malloc %d bytes\n", len);
		return 1;
	}
	for (i = 2, s = value; i < argc; ++i) {
		char *v = argv[i];

		while ((*s++ = *v++) != '\0')
			;
		*(s - 1) = ' ';
	}
	if (s != value)
		*--s = '\0';

	e.key	= name;
	e.data	= value;
	hsearch_r(e, ENV_ENTER, &ep, &sysinfo_htab, env_flag);
	free(value);
	if (!ep) {
		printf("## Error inserting \"%s\" variable, errno=%d\n",
			name, errno);
		return 1;
	}

	return 0;
}

int sysinfo_set(const char *varname, const char *varvalue)
{
	const char * const argv[4] = { "setsysinfo", varname, varvalue, NULL };

	/* before import into hashtable */
	if (!(gd->flags & GD_FLG_SYSINFO_READY))
		return 1;

	if (varvalue == NULL || varvalue[0] == '\0')
		return _do_sysinfo_set(0, 2, (char * const *)argv, H_PROGRAMMATIC);
	else
		return _do_sysinfo_set(0, 3, (char * const *)argv, H_PROGRAMMATIC);
}

static int do_sysinfo_set(struct cmd_tbl *cmdtp, int flag, int argc,
		      char *const argv[])
{
	if (argc < 2)
		return CMD_RET_USAGE;

	return _do_sysinfo_set(flag, argc, argv, H_INTERACTIVE);
}

#if defined(CONFIG_CMD_EDITSYSINFO)
/*
 * Interactively edit an sysinfo variable
 */
static int do_sysinfo_edit(struct cmd_tbl *cmdtp, int flag, int argc,
		       char *const argv[])
{
	char buffer[CONFIG_SYS_CBSIZE];
	char *init_val;

	if (argc < 2)
		return CMD_RET_USAGE;

	/* before import into hashtable */
	if (!(gd->flags & GD_FLG_SYSINFO_READY))
		return 1;

	/* Set read buffer to initial value or empty sting */
	init_val = sysinfo_env_get(argv[1]);
	if (init_val)
		snprintf(buffer, CONFIG_SYS_CBSIZE, "%s", init_val);
	else
		buffer[0] = '\0';

	if (cli_readline_into_buffer("edit: ", buffer, 0) < 0)
		return 1;

	if (buffer[0] == '\0') {
		const char * const _argv[3] = { "setsysinfo", argv[1], NULL };

		return _do_sysinfo_set(0, 2, (char * const *)_argv, H_INTERACTIVE);
	} else {
		const char * const _argv[4] = { "setsysinfo", argv[1], buffer,
			NULL };

		return _do_sysinfo_set(0, 3, (char * const *)_argv, H_INTERACTIVE);
	}
}
#endif

#if defined(CONFIG_CMD_SAVESYSINFO) && defined(SYSINFO_IS_IN_DEVICE)
static int do_sysinfo_save(struct cmd_tbl *cmdtp, int flag, int argc,
		       char *const argv[])
{
	return sysinfo_env_save() ? 1 : 0;
}

U_BOOT_CMD(
	savesys, 1, 0,	do_sysinfo_save,
	"save system information variables to persistent storage",
	""
);

#if defined(CONFIG_CMD_ERASESYSINFO)
static int do_sysinfo_erase(struct cmd_tbl *cmdtp, int flag, int argc,
			char *const argv[])
{
	return sysinfo_erase() ? 1 : 0;
}

U_BOOT_CMD(
	erasesys, 1, 0,	do_sysinfo_erase,
	"erase system information variables from persistent storage",
	""
);
#endif
#endif

static int do_sysinfo_delete(struct cmd_tbl *cmdtp, int flag,
			 int argc, char *const argv[])
{
	int env_flag = H_INTERACTIVE;
	int ret = 0;

	debug("Initial value for argc=%d\n", argc);
	while (argc > 1 && **(argv + 1) == '-') {
		char *arg = *++argv;

		--argc;
		while (*++arg) {
			switch (*arg) {
			case 'f':		/* force */
				env_flag |= H_FORCE;
				break;
			default:
				return CMD_RET_USAGE;
			}
		}
	}
	debug("Final value for argc=%d\n", argc);

	sysinfo_id++;

	while (--argc > 0) {
		char *name = *++argv;

		if (hdelete_r(name, &sysinfo_htab, env_flag))
			ret = 1;
	}

	return ret;
}

#if defined(CONFIG_CMD_SYSINFO_EXISTS)
static int do_sysinfo_exists(struct cmd_tbl *cmdtp, int flag, int argc,
			 char *const argv[])
{
	struct env_entry e, *ep;

	if (argc < 2)
		return CMD_RET_USAGE;

	e.key = argv[1];
	e.data = NULL;
	hsearch_r(e, ENV_FIND, &ep, &sysinfo_htab, 0);

	return (ep == NULL) ? 1 : 0;
}
#endif

/*
 * New command line interface: "sysinfo" command with subcommands
 */
static struct cmd_tbl cmd_sysinfo_sub[] = {
	U_BOOT_CMD_MKENT(delete, CONFIG_SYS_MAXARGS, 0, do_sysinfo_delete, "", ""),
#if defined(CONFIG_CMD_EDITSYSINFO)
	U_BOOT_CMD_MKENT(edit, 2, 0, do_sysinfo_edit, "", ""),
#endif
	U_BOOT_CMD_MKENT(print, CONFIG_SYS_MAXARGS, 1, do_sysinfo_print, "", ""),
#if defined(CONFIG_CMD_SAVESYSINFO) && defined(SYSINFO_IS_IN_DEVICE)
	U_BOOT_CMD_MKENT(save, 1, 0, do_sysinfo_save, "", ""),
#if defined(CONFIG_CMD_ERASESYSINFO)
	U_BOOT_CMD_MKENT(erase, 1, 0, do_sysinfo_erase, "", ""),
#endif
#endif
	U_BOOT_CMD_MKENT(set, CONFIG_SYS_MAXARGS, 0, do_sysinfo_set, "", ""),
#if defined(CONFIG_CMD_SYSINFO_EXISTS)
	U_BOOT_CMD_MKENT(exists, 2, 0, do_sysinfo_exists, "", ""),
#endif
};

static int do_sysinfo(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	struct cmd_tbl *cp;

	if (argc < 2)
		return CMD_RET_USAGE;

	/* drop initial "sysinfo" arg */
	argc--;
	argv++;

	cp = find_cmd_tbl(argv[0], cmd_sysinfo_sub, ARRAY_SIZE(cmd_sysinfo_sub));

	if (cp)
		return cp->cmd(cmdtp, flag, argc, argv);

	return CMD_RET_USAGE;
}

#ifdef CONFIG_SYS_LONGHELP
static char sysinfo_help_text[] =
	"sys delete [-f] var [...] - [forcibly] delete variable(s)\n"
#if defined(CONFIG_CMD_EDITSYSINFO)
	"sys edit name - edit system information variable\n"
#endif
#if defined(CONFIG_CMD_SYSINFO_EXISTS)
	"sys exists name - tests for existence of variable\n"
#endif
	"sys print [-a | name ...] - print system information variable\n"
#if defined(CONFIG_CMD_SAVESYSINFO) && defined(SYSINFO_IS_IN_DEVICE)
	"sys save - save system information\n"
#if defined(CONFIG_CMD_ERASESYSINFO)
	"sys erase - erase system information\n"
#endif
#endif
	"sys set [-f] name [arg ...]\n";
#endif


U_BOOT_CMD(
	sysinfo, CONFIG_SYS_MAXARGS, 1, do_sysinfo,
	"system information handling commands", sysinfo_help_text
);

#if defined(CONFIG_CMD_EDITSYSINFO)
U_BOOT_CMD_COMPLETE(
	editsys, 2, 0,	do_sysinfo_edit,
	"edit system information variable",
	"name\n"
	"    - edit system information variable 'name'",
	var_complete
);
#endif

U_BOOT_CMD_COMPLETE(
	printsys, CONFIG_SYS_MAXARGS, 1,	do_sysinfo_print,
	"print system information data variables",
	"[-a]\n    - print [all] values of all system information variables\n"
	"printsys name ...\n"
	"    - print value of system information variable 'name'",
	var_complete
);

U_BOOT_CMD_COMPLETE(
	setsys, CONFIG_SYS_MAXARGS, 0,	do_sysinfo_set,
	"set system information data variables",
	"setsys [-f] name value ...\n"
	"    - [forcibly] set system information variable 'name' to 'value ...'\n"
	"setsys [-f] name\n"
	"    - [forcibly] delete system information variable 'name'",
	var_complete
);

