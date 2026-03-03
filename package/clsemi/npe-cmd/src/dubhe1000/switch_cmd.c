// SPDX-License-Identifier: GPL-2.0
// Copyright(c) 2023 Clourneysemi Corporation.

#include "switch_vlan.h"
#include "switch_generic.h"
#include "switch_l2.h"
#include "switch_l3.h"
#include "switch_acl.h"
#include "switch_nat.h"
#include "npe_switch.h"
#include "npe.h"

struct cmd_module switch_cmd[] = {
	{"vlan", switch_vlan_config, switch_vlan_usage},
	{"l2", switch_l2_config, switch_l2_usage},
	{"l3", switch_l3_config, switch_l3_usage},
	{"generic", switch_generic_config, switch_generic_usage},
	{"acl", switch_acl_config, switch_acl_usage},
	{"nat", switch_nat_config, switch_nat_usage},
};

void npe_switch_usage(void)
{
	npecmd_err("Usage: npecmd switch FUNCTION SUB-FUNCTION {COMMAND}\n");
	npecmd_err("where  FUNCTION :=");
	for (int i = 0; i < ARRAY_SIZE(switch_cmd); i++)
		npecmd_err(" %s", switch_cmd[i].name);

	npecmd_err("\n");
}

int npe_switch_config(int argc, char **argv)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(switch_cmd); i++) {
		if (!strcasecmp(argv[0], switch_cmd[i].name)) {
			if (argc <= 1) {
				if (switch_cmd[i].usage)
					switch_cmd[i].usage();

				return 0;
			} else if (switch_cmd[i].func) {
				return switch_cmd[i].func(argc - 1, argv + 1);
			}
		}
	}

	npe_switch_usage();

	return 0;
}
