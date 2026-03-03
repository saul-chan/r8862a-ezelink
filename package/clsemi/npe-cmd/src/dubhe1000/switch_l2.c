// SPDX-License-Identifier: GPL-2.0
// Copyright(c) 2023 Clourneysemi Corporation.

#include "npe_switch.h"
#include "switch_conf.h"
#include "switch_l2.h"
#include <arpa/inet.h>

/**********************L2 Hash Calculation*********************************/
//return (gid << 48 + *macaddr)
static u64 switch_l2_hashkey(u64 macaddr, u16 gid)
{
	u64 key;

	key = gid;
	key = (key << 48) + (macaddr & 0xFFFFFFFFFFFF);

	return key;
}

static u16 switch_l2_hash_calc(u64 macaddr, u16 gid)
{
	u16 hashval = 0;
	u64 key = 0;

	key = switch_l2_hashkey(macaddr, gid);

	hashval = key & 0x3FF;
	hashval = hashval ^ (key >> 10);
	hashval = hashval & 0x3FF;
	hashval = hashval ^ (key >> 20);
	hashval = hashval & 0x3FF;
	hashval = hashval ^ (key >> 30);
	hashval = hashval & 0x3FF;
	hashval = hashval ^ (key >> 40);
	hashval = hashval & 0x3FF;
	hashval = hashval ^ (key >> 50);
	hashval = hashval & 0x3FF;

	return hashval;
}
/**************************************************************************/

struct switch_field_def switch_l2_destination_table[] = {
	{L2_DESTINATION_TABLE_ARG_UC, "uc", 1, 0},
	{L2_DESTINATION_TABLE_ARG_DESTPORT_OR_MCADDR, "destPort_or_mcAddr", 6, 1},
	{L2_DESTINATION_TABLE_ARG_PKTDROP, "pktDrop", 1, 7},
	{L2_DESTINATION_TABLE_ARG_TUNNELENTRY, "tunnelEntry", 1, 8},
	{L2_DESTINATION_TABLE_ARG_TUNNELENTRYPTR, "tunnelEntryPtr", 6, 9},
	{L2_DESTINATION_TABLE_ARG_TUNNELEXIT, "tunnelExit", 1, 15},
	{L2_DESTINATION_TABLE_ARG_TUNNELEXITPTR, "tunnelExitPtr", 7, 16},
};

void switch_l2_dest_usage(void)
{
	printf("Usage: npecmd switch l2 dest <index> <--arg=val ...>\n");

	printf("index range: [0 - %d]\n", L2_DESTINATION_TABLE_MAX - 1);
	printf("arg :=");
	for (int i = 0; i < ARRAY_SIZE(switch_l2_destination_table); i++)
		printf(" %s", switch_l2_destination_table[i].name);

	printf("\n");
}

int switch_l2_dest_config(int argc, char **argv)
{
	u64 address;
	u32 value[L2_DESTINATION_TABLE_ADDR_PER_ENTRY], index;
	int i;
	u16 real_index = 0;

	if (argc <= 1) {
		printf("[%s] Failed! Invalid argc\n", __func__);
		goto ERR;
	}

	//check index
	if ((sscanf(argv[0], "%i", &index) != 1)
			|| index >= L2_DESTINATION_TABLE_MAX) {
		printf("[%s] Failed! Invalid index\n", __func__);
		goto ERR;
	}

	// get address
	address = L2_DESTINATION_TABLE;
	address += index * L2_DESTINATION_TABLE_ADDR_PER_ENTRY;

	//get old value
	for (i = 0; i < L2_DESTINATION_TABLE_ADDR_PER_ENTRY; i++)
		switch_read(address + i, &value[i]);

	//update
	if (!switch_field_setup(argc, argv, switch_l2_destination_table, ARRAY_SIZE(switch_l2_destination_table), value)) {
		printf("[%s] Failed! Invalid arg list\n", __func__);
		goto ERR;
	}

	//config
	for (i = 0; i < L2_DESTINATION_TABLE_ADDR_PER_ENTRY; i++)
		switch_write(address + i, value[i]);


	if (index < L2_DA_HASH_LOOKUP_TABLE_MAX)
		real_index = index;
	else
		real_index = index - L2_DA_HASH_LOOKUP_TABLE_MAX;


	printf("[%s] index 0x%x(%s 0x%x) addr 0x%lx value 0x%08x\n",
			__func__, index,
			(index < L2_DA_HASH_LOOKUP_TABLE_MAX) ? "hash" : "collision", real_index, address, value[0]);

	return 1;
ERR:
	switch_l2_dest_usage();

	return 0;

}

void switch_l2_lookup_hash_usage(void)
{
	printf("Usage: npecmd switch l2 lookup_hash add MACADDR <gid> <bucket>\n");
	printf("Usage: npecmd switch l2 lookup_hash del <table_id>\n");

	printf("gid range: [0 - %d]\n", VLAN_TABLE_MAX - 1);
	printf("bucket range: [0 - %d]\n", L2_DA_HASH_LOOKUP_TABLE_BUCKET_MAX - 1);
	printf("table_id range: [0 - %d]\n", L2_DA_HASH_LOOKUP_TABLE_MAX - 1);
}

int switch_l2_lookup_hash_config(int argc, char **argv)
{
	int is_add, is_da, tmp;
	u64 address;
	u64 hashkey;
	u32 value[L2_DA_HASH_LOOKUP_TABLE_ADDR_PER_ENTRY];
	u16 hashval = 0, index = 0, gid;
	u8 bucket;
	u64 macaddr;

	//check arg
	if (argc <= 1) {
		printf("[%s] Failed! Invalid argc\n", __func__);
		goto ERR;
	}

	if (!strcasecmp(argv[0], L2_LOOKUP_ACTION_TYPE_ADD)) {
		is_add = 1;
	} else if (!strcasecmp(argv[0], L2_LOOKUP_ACTION_TYPE_DEL)) {
		is_add = 0;
	} else {
		printf("[%s] Failed! Invalid action_type\n", __func__);
		goto ERR;
	}

	if (is_add) {
		if (argc != 4) {
			printf("[%s] Del Failed! Invalid argc\n", __func__);
			goto ERR;
		}

		if (!switch_mac_to_u64(argv[1], &macaddr)) {
			printf("[%s] Add Failed! Invalid MACADDR\n", __func__);
			goto ERR;
		}

		if ((sscanf(argv[2], "%i", &tmp) != 1) || (tmp >= VLAN_TABLE_MAX)) {
			printf("[%s] Add Failed! Invalid gid\n", __func__);
			goto ERR;
		}

		gid = tmp;

		if ((sscanf(argv[3], "%i", &tmp) != 1) || (tmp >= L2_DA_HASH_LOOKUP_TABLE_BUCKET_MAX)) {
			printf("[%s] Add Failed! Invalid bucket\n", __func__);
			goto ERR;
		}

		bucket = tmp;

		hashval = switch_l2_hash_calc(macaddr, gid);
		index =  hashval + ((bucket & 0x3) << 10);

		hashkey = switch_l2_hashkey(macaddr, gid);
		value[0] = hashkey & 0xFFFFFFFF;
		value[1] = hashkey >> 32;
	} else {
		if (argc != 2) {
			printf("[%s] Del Failed! Invalid argc\n", __func__);
			goto ERR;
		}

		if ((sscanf(argv[1], "%i", &tmp) != 1) || (tmp >= L2_DA_HASH_LOOKUP_TABLE_MAX)) {
			printf("[%s] Del Failed! Invalid bucket\n", __func__);
			goto ERR;
		}

		index = tmp;

		value[0] = 0;
		value[1] = 0;
	}

	address = L2_DA_HASH_LOOKUP_TABLE + index * L2_DA_HASH_LOOKUP_TABLE_ADDR_PER_ENTRY;

	for (int i = 0; i < L2_DA_HASH_LOOKUP_TABLE_ADDR_PER_ENTRY; i++)
		switch_write(address + i, value[i]);

	if (is_add)
		printf("[%s] add index 0x%x addr 0x%lx: hashval 0x%x hashkey 0x%lx value 0x[%08x %08x]\n",
				__func__, index, address, hashval, hashkey, value[1], value[0]);
	else
		printf("[%s] del index 0x%x addr 0x%lx: value 0x[%08x %08x]\n",
				__func__, index, address, value[1], value[0]);

	return 1;
ERR:
	switch_l2_lookup_hash_usage();

	return 0;
}

struct switch_field_def switch_l2_lookup_coll_table[] = {
	{L2_LOOKUP_COLLISION_ARG_MACADDR, "macAddr", 48, 0, SWITCH_FIELD_TYPE_MACADDR},
	{L2_LOOKUP_COLLISION_ARG_GID, "gid", 12, 48},
};

void switch_l2_lookup_coll_usage(void)
{
	printf("Usage: npecmd switch l2 lookup_coll table <table_id> <--arg=val ...>\n");
	printf("Usage: npecmd switch l2 lookup_coll mask <mask_id> <--arg=val ...>\n");
	printf("table_id range: [0 - %d]\n", L2_LOOKUP_COLLISION_TABLE_MAX - 1);
	printf("mask_id range: [0 - %d]\n", L2_LOOKUP_COLLISION_TABLE_MASKS_MAX - 1);
	printf("arg :=");
	for (int i = 0; i < ARRAY_SIZE(switch_l2_lookup_coll_table); i++)
		printf(" %s", switch_l2_lookup_coll_table[i].name);

	printf("\n");
}

int switch_l2_lookup_coll_config(int argc, char **argv)
{
	int is_table, tmp, i;
	u16 index;
	u64 address;
	u32 value[L2_LOOKUP_COLLISION_TABLE_MAX];

	if (argc <= 2) {
		printf("[%s] Failed! Invalid argc\n", __func__);
		goto ERR;
	}

	if (!strcasecmp(argv[0], L2_COLL_TYPE_TABLE)) {
		is_table = 1;
	} else if (!strcasecmp(argv[0], L2_COLL_TYPE_MASK)) {
		is_table = 0;
	} else {
		printf("[%s] Failed! Invalid table_type\n", __func__);
		goto ERR;
	}

	if ((sscanf(argv[1], "%i", &tmp) != 1)
			|| (is_table && tmp >= L2_LOOKUP_COLLISION_TABLE_MAX)
			|| (!is_table && tmp >= L2_LOOKUP_COLLISION_TABLE_MASKS_MAX)) {
		printf("[%s] Failed! Invalid index\n", __func__);
		goto ERR;
	}

	index = tmp;

	// get address
	if (!is_table)
		address = L2_LOOKUP_COLLISION_TABLE_MASKS + index * L2_LOOKUP_COLLISION_TABLE_MASKS_ADDR_PER_ENTRY;
	else
		address = L2_LOOKUP_COLLISION_TABLE + index * L2_LOOKUP_COLLISION_TABLE_ADDR_PER_ENTRY;


	//get old value
	for (i = 0; i < L2_LOOKUP_COLLISION_TABLE_ADDR_PER_ENTRY; i++)
		switch_read(address + i, &value[i]);

	//update
	if (!switch_field_setup(argc - 1, argv + 1, switch_l2_lookup_coll_table, ARRAY_SIZE(switch_l2_lookup_coll_table), value)) {
		printf("[%s] Failed! Invalid arg list\n", __func__);
		goto ERR;
	}

	//config
	for (i = 0; i < L2_LOOKUP_COLLISION_TABLE_ADDR_PER_ENTRY; i++)
		switch_write(address + i, value[i]);


	printf("[%s] %s index 0x%x addr 0x%lx value 0x[%08x %08x]\n", __func__, is_table ? "TABLE" : "MASK", index, address, value[1], value[0]);

	return 1;
ERR:
	switch_l2_lookup_coll_usage();
	return 0;
}

struct switch_field_def switch_l2_multicast_table[] = {
	{L2_MULTICAST_TABLE_ARG_MCPORTMASK, "mcPortMask", 6, 0},
	{L2_MULTICAST_TABLE_ARG_TUNNELENTRY, "tunnelEntry", 1, 6},
	{L2_MULTICAST_TABLE_ARG_TUNNELENTRYPTR, "tunnelEntryPtr", 6, 7},
	{L2_MULTICAST_TABLE_ARG_TUNNELEXIT, "tunnelExit", 1, 13},
	{L2_MULTICAST_TABLE_ARG_TUNNELEXITPTR, "tunnelExitPtr", 7, 14},
};

void switch_l2_multi_table_usage(void)
{
	printf("Usage: npecmd switch l2 multi_table <index> <--arg=val ...>\n");

	printf("index range: [0 - %d]\n", L2_MULTICAST_TABLE_MAX - 1);
	printf("arg :=");
	for (int i = 0; i < ARRAY_SIZE(switch_l2_multicast_table); i++)
		printf(" %s", switch_l2_multicast_table[i].name);

	printf("\n");
}

int switch_l2_multi_table_config(int argc, char **argv)
{
	u64 address;
	u32 value[L2_MULTICAST_TABLE_ADDR_PER_ENTRY], index;
	int i;

	//check
	if (argc <= 1) {
		printf("[%s] Failed! Invalid argc\n", __func__);
		goto ERR;
	}

	if ((sscanf(argv[0], "%i", &index) != 1) || (index >= L2_MULTICAST_TABLE_MAX)) {
		printf("[%s] Failed! Invalid index\n", __func__);
		goto ERR;
	}

       address = L2_MULTICAST_TABLE + index * L2_MULTICAST_TABLE_ADDR_PER_ENTRY;

	//get old value
	for (i = 0; i < L2_MULTICAST_TABLE_ADDR_PER_ENTRY; i++)
		switch_read(address + i, &value[i]);

	//update
	if (!switch_field_setup(argc, argv, switch_l2_multicast_table, ARRAY_SIZE(switch_l2_multicast_table), value)) {
		printf("[%s] Failed! Invalid arg list\n", __func__);
		goto ERR;
	}

	//config
	for (i = 0; i < L2_MULTICAST_TABLE_ADDR_PER_ENTRY; i++)
		switch_write(address + i, value[i]);

	printf("[%s] index %d addr 0x%lx value 0x%08x\n", __func__, index, address, value[0]);

	return 1;
ERR:
	switch_l2_multi_table_usage();

	return 0;
}

struct switch_field_def switch_l2_aging_table[] = {
       {L2_AGING_TABLE_ARG_VALID, "valid", 1, 0},
       {L2_AGING_TABLE_ARG_STAT, "stat", 1, 1},
       {L2_AGING_TABLE_ARG_HIT, "hit", 1, 2},

};

void switch_l2_aging_table_usage(void)
{
       printf("Usage: npecmd switch l2 aging_table <index> <--arg=val ...>\n");

       printf("index range: [0 - %d]\n", L2_AGING_TABLE_MAX - 1);
       printf("arg :=");
       for (int i = 0; i < ARRAY_SIZE(switch_l2_aging_table); i++)
               printf(" %s", switch_l2_aging_table[i].name);

       printf("\n");
}

int switch_l2_aging_table_config(int argc, char **argv)
{
       u64 address;
       u32 value[L2_AGING_TABLE_ADDR_PER_ENTRY], index;
       int i;

       //check
       if (argc <= 1) {
               printf("[%s] Failed! Invalid argc\n", __func__);
               goto ERR;
       }

       if ((sscanf(argv[0], "%i", &index) != 1) || (index >= L2_AGING_TABLE_MAX)) {
               printf("[%s] Failed! Invalid index\n", __func__);
               goto ERR;
       }

       address = L2_AGING_TABLE + index * L2_AGING_TABLE_ADDR_PER_ENTRY;

       //get old value
       for (i = 0; i < L2_AGING_TABLE_ADDR_PER_ENTRY; i++)
               switch_read(address + i, &value[i]);

       //update
       if (!switch_field_setup(argc, argv, switch_l2_aging_table, ARRAY_SIZE(switch_l2_aging_table), value)) {
               printf("[%s] Failed! Invalid arg list\n", __func__);
               goto ERR;
       }

       //config
       for (i = 0; i < L2_AGING_TABLE_ADDR_PER_ENTRY; i++)
               switch_write(address + i, value[i]);

       printf("[%s] index %d addr 0x%lx value 0x%08x\n", __func__, index, address, value[0]);

       return 1;
ERR:
       switch_l2_aging_table_usage();

       return 0;
}

void switch_l2_aging_coll_table_usage(void)
{
       printf("Usage: npecmd switch l2 aging_coll_table <index> <--arg=val ...>\n");

       printf("index range: [0 - %d]\n", L2_AGING_COLLISION_TABLE - 1);
       printf("arg :=");
       for (int i = 0; i < ARRAY_SIZE(switch_l2_aging_table); i++)
               printf(" %s", switch_l2_aging_table[i].name);

       printf("\n");
}

int switch_l2_aging_coll_table_config(int argc, char **argv)
{
       u64 address;
       u32 value[L2_AGING_COLLISION_TABLE_ADDR_PER_ENTRY], index;
       int i;

       //check
       if (argc <= 1) {
               printf("[%s] Failed! Invalid argc\n", __func__);
               goto ERR;
       }

       if ((sscanf(argv[0], "%i", &index) != 1) || (index >= L2_AGING_COLLISION_TABLE_MAX)) {
               printf("[%s] Failed! Invalid index\n", __func__);
               goto ERR;
       }

       address = L2_AGING_COLLISION_TABLE + index * L2_AGING_COLLISION_TABLE_ADDR_PER_ENTRY;

       //get old value
       for (i = 0; i < L2_AGING_COLLISION_TABLE_ADDR_PER_ENTRY; i++)
               switch_read(address + i, &value[i]);

       //update
       if (!switch_field_setup(argc, argv, switch_l2_aging_table, ARRAY_SIZE(switch_l2_aging_table), value)) {
               printf("[%s] Failed! Invalid arg list\n", __func__);
               goto ERR;
       }

       //config
       for (i = 0; i < L2_AGING_COLLISION_TABLE_ADDR_PER_ENTRY; i++)
               switch_write(address + i, value[i]);

       printf("[%s] index %d addr 0x%lx value 0x%08x\n", __func__, index, address, value[0]);

       return 1;
ERR:
       switch_l2_aging_coll_table_usage();

       return 0;
}



struct switch_field_def switch_l2_aging_shadow[] = {
       {L2_AGING_STATUS_SHADOW_TABLE_ARG_VALID, "valid", 1, 0},
};

void switch_l2_aging_shadow_usage(void)
{
       printf("Usage: npecmd switch l2 aging_shadow <index> <--arg=val ...>\n");

       printf("index range: [0 - %d]\n", L2_AGING_STATUS_SHADOW_TABLE_MAX - 1);
       printf("arg :=");
       for (int i = 0; i < ARRAY_SIZE(switch_l2_aging_shadow); i++)
               printf(" %s", switch_l2_aging_shadow[i].name);

       printf("\n");
}

int switch_l2_aging_shadow_config(int argc, char **argv)
{
       u64 address;
       u32 value[L2_AGING_STATUS_SHADOW_TABLE_ADDR_PER_ENTRY], index;
       int i;

       //check
       if (argc <= 1) {
               printf("[%s] Failed! Invalid argc\n", __func__);
               goto ERR;
       }

       if ((sscanf(argv[0], "%i", &index) != 1) || (index >= L2_AGING_STATUS_SHADOW_TABLE_MAX)) {
               printf("[%s] Failed! Invalid index\n", __func__);
               goto ERR;
       }

       address = L2_AGING_STATUS_SHADOW_TABLE + index * L2_AGING_STATUS_SHADOW_TABLE_ADDR_PER_ENTRY;

       //get old value
       for (i = 0; i < L2_AGING_STATUS_SHADOW_TABLE_ADDR_PER_ENTRY; i++)
               switch_read(address + i, &value[i]);

       //update
       if (!switch_field_setup(argc, argv, switch_l2_aging_shadow, ARRAY_SIZE(switch_l2_aging_shadow), value)) {
               printf("[%s] Failed! Invalid arg list\n", __func__);
               goto ERR;
       }

       //config
       for (i = 0; i < L2_AGING_STATUS_SHADOW_TABLE_ADDR_PER_ENTRY; i++)
               switch_write(address + i, value[i]);

       printf("[%s] index %d addr 0x%lx value 0x%08x\n", __func__, index, address, value[0]);

       return 1;
ERR:
       switch_l2_aging_shadow_usage();

       return 0;
}

void switch_l2_aging_coll_shadow_usage(void)
{
       printf("Usage: npecmd switch l2 aging_coll_shadow <index> <--arg=val ...>\n");

       printf("index range: [0 - %d]\n", L2_AGING_COLLISION_SHADOW_TABLE_MAX - 1);
       printf("arg :=");
       for (int i = 0; i < ARRAY_SIZE(switch_l2_aging_shadow); i++)
               printf(" %s", switch_l2_aging_shadow[i].name);

       printf("\n");
}

int switch_l2_aging_coll_shadow_config(int argc, char **argv)
{
       u64 address;
       u32 value[L2_AGING_COLLISION_SHADOW_TABLE_ADDR_PER_ENTRY], index;
       int i;

       //check
       if (argc <= 1) {
               printf("[%s] Failed! Invalid argc\n", __func__);
               goto ERR;
       }

       if ((sscanf(argv[0], "%i", &index) != 1) || (index >= L2_AGING_COLLISION_SHADOW_TABLE_MAX)) {
               printf("[%s] Failed! Invalid index\n", __func__);
               goto ERR;
       }

       address = L2_AGING_COLLISION_SHADOW_TABLE + index * L2_AGING_COLLISION_SHADOW_TABLE_ADDR_PER_ENTRY;

       //get old value
       for (i = 0; i < L2_AGING_COLLISION_SHADOW_TABLE_ADDR_PER_ENTRY; i++)
               switch_read(address + i, &value[i]);

       //update
       if (!switch_field_setup(argc, argv, switch_l2_aging_shadow, ARRAY_SIZE(switch_l2_aging_shadow), value)) {
               printf("[%s] Failed! Invalid arg list\n", __func__);
               goto ERR;
       }

       //config
       for (i = 0; i < L2_AGING_COLLISION_SHADOW_TABLE_ADDR_PER_ENTRY; i++)
               switch_write(address + i, value[i]);

       printf("[%s] index %d addr 0x%lx value 0x%08x\n", __func__, index, address, value[0]);

       return 1;
ERR:
       switch_l2_aging_coll_shadow_usage();

       return 0;
}

struct switch_field_def switch_l2_aging_replica[] = {
       {L2_AGING_STATUS_SHADOW_TABLE_REPLICA_ARG_VALID, "valid", 1, 0},
};

void switch_l2_aging_replica_usage(void)
{
       printf("Usage: npecmd switch l2 aging_replica <index> <--arg=val ...>\n");

       printf("index range: [0 - %d]\n", L2_AGING_STATUS_SHADOW_TABLE_REPLICA_MAX - 1);
       printf("arg :=");
       for (int i = 0; i < ARRAY_SIZE(switch_l2_aging_replica); i++)
               printf(" %s", switch_l2_aging_replica[i].name);

       printf("\n");
}

int switch_l2_aging_replica_config(int argc, char **argv)
{
       u64 address;
       u32 value[L2_AGING_STATUS_SHADOW_TABLE_REPLICA_ADDR_PER_ENTRY], index;
       int i;

       //check
       if (argc <= 1) {
               printf("[%s] Failed! Invalid argc\n", __func__);
               goto ERR;
       }

       if ((sscanf(argv[0], "%i", &index) != 1) || (index >= L2_AGING_STATUS_SHADOW_TABLE_REPLICA_MAX)) {
               printf("[%s] Failed! Invalid index\n", __func__);
               goto ERR;
       }

       address = L2_AGING_STATUS_SHADOW_TABLE_REPLICA + index * L2_AGING_STATUS_SHADOW_TABLE_REPLICA_ADDR_PER_ENTRY;

       //get old value
       for (i = 0; i < L2_AGING_STATUS_SHADOW_TABLE_REPLICA_ADDR_PER_ENTRY; i++)
               switch_read(address + i, &value[i]);

       //update
       if (!switch_field_setup(argc, argv, switch_l2_aging_replica, ARRAY_SIZE(switch_l2_aging_replica), value)) {
               printf("[%s] Failed! Invalid arg list\n", __func__);
               goto ERR;
       }

       //config
       for (i = 0; i < L2_AGING_STATUS_SHADOW_TABLE_REPLICA_ADDR_PER_ENTRY; i++)
               switch_write(address + i, value[i]);

       printf("[%s] index %d addr 0x%lx value 0x%08x\n", __func__, index, address, value[0]);

       return 1;
ERR:
       switch_l2_aging_replica_usage();

       return 0;
}

struct cmd_module switch_l2_cmd[] = {
	{"dest", switch_l2_dest_config, switch_l2_dest_usage},
	{"lookup_hash", switch_l2_lookup_hash_config, switch_l2_lookup_hash_usage},
	{"lookup_coll", switch_l2_lookup_coll_config, switch_l2_lookup_coll_usage},
	{"multi_table", switch_l2_multi_table_config, switch_l2_multi_table_usage},
	{"aging_table", switch_l2_aging_table_config, switch_l2_aging_table_usage},
	{"aging_coll_table", switch_l2_aging_coll_table_config, switch_l2_aging_coll_table_usage},
	{"aging_shadow", switch_l2_aging_shadow_config, switch_l2_aging_shadow_usage},
	{"aging_coll_shadow", switch_l2_aging_coll_shadow_config, switch_l2_aging_coll_shadow_usage},
	{"aging_replica", switch_l2_aging_replica_config, switch_l2_aging_replica_usage},
};

void switch_l2_usage(void)
{
	printf("Usage: npecmd switch l2 SUB-FUNCTION {COMMAND}\n");
	printf("where  SUB-FUNCTION :=");
	for (int i = 0; i < ARRAY_SIZE(switch_l2_cmd); i++)
		printf(" %s", switch_l2_cmd[i].name);

	printf("\n");
}

int switch_l2_config(int argc, char **argv)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(switch_l2_cmd); i++) {
		if (!strcasecmp(argv[0], switch_l2_cmd[i].name)) {
			if (argc <= 1) {
				if (switch_l2_cmd[i].usage)
					switch_l2_cmd[i].usage();

				return 0;
			}

			if (switch_l2_cmd[i].func)
				return switch_l2_cmd[i].func(argc - 1, argv + 1);
		}
	}

	switch_l2_usage();

	return 0;
}
