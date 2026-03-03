#include <linux/stringify.h>
const char default_sysinfo[] = {
#ifdef	CONFIG_USE_SYSINFO_DEFAULT
	CONFIG_SYSINFO_ARGS		"\0"
#endif
};

#include <env_internal.h>
static_assert(sizeof(default_sysinfo) <= SYSINFO_SIZE,
	      "Default sysinfo variables is too large");
