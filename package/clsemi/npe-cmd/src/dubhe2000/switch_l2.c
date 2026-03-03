// SPDX-License-Identifier: GPL-2.0
// Copyright(c) 2023 Clourneysemi Corporation.

#include "npe_switch.h"
#include "switch_conf.h"
#include "switch_l2.h"
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/if_ether.h>
#include <linux/if_packet.h>
#include <net/if.h>
#include <netinet/ether.h>
#include <errno.h>


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
	{L2_DESTINATION_TABLE_ARG_L2ACTIONTABLEDASTATUS, "l2ActionTableDaStatus", 1, 8},
	{L2_DESTINATION_TABLE_ARG_L2ACTIONTABLESASTATUS, "l2ActionTableSaStatus", 1, 9},
	{L2_DESTINATION_TABLE_ARG_METADATA, "metaData", 16, 10},
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
	{L2_MULTICAST_TABLE_ARG_METADATA, "metaData", 16, 6},
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

       printf("index range: [0 - %d]\n", L2_AGING_COLLISION_TABLE_MAX - 1);
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

struct switch_field_def switch_l2_action_table[] = {
	{L2_ACTION_TABLE_ARG_NOLEARNINGUC, "noLearningUc", 1, 0},
	{L2_ACTION_TABLE_ARG_NOLEARNINGMC, "noLearningMc", 1, 1},
	{L2_ACTION_TABLE_ARG_DROPALL, "dropAll", 1, 2},
	{L2_ACTION_TABLE_ARG_DROP, "drop", 1, 3},
	{L2_ACTION_TABLE_ARG_DROPPORTMOVE, "dropPortMove", 1, 4},
	{L2_ACTION_TABLE_ARG_SENDTOCPU, "sendToCpu", 1, 5},
	{L2_ACTION_TABLE_ARG_FORCESENDTOCPUORIGPKT, "forceSendToCpuOrigPkt", 1, 6},
	{L2_ACTION_TABLE_ARG_NOPORTMOVE, "noPortMove", 1, 7},
	{L2_ACTION_TABLE_ARG_USESPECIALALLOW, "useSpecialAllow", 1, 8},
	{L2_ACTION_TABLE_ARG_ALLOWPTR, "allowPtr", 2, 9},
	{L2_ACTION_TABLE_ARG_MMPVALID, "mmpValid", 1, 11},
	{L2_ACTION_TABLE_ARG_MMPPTR, "mmpPtr", 5, 12},
	{L2_ACTION_TABLE_ARG_MMPORDER, "mmpOrder", 2, 17},
};

void switch_l2_action_table_usage(void)
{
	printf("Usage: npecmd switch l2 action_table <index> <--arg=val ...>\n");

	printf("index range: [0 - %d]\n", L2_ACTION_TABLE_MAX - 1);
	printf("arg :=");
	for (int i = 0; i < ARRAY_SIZE(switch_l2_action_table); i++)
		printf(" %s", switch_l2_action_table[i].name);

	printf("\n");
}

int switch_l2_action_table_config(int argc, char **argv)
{
	u64 address;
	u32 value[L2_ACTION_TABLE_ADDR_PER_ENTRY], index;
	int i;

	if (argc <= 1) {
		printf("[%s] Failed! Invalid argc\n", __func__);
		goto ERR;
	}

	//check index
	if ((sscanf(argv[0], "%i", &index) != 1)
			|| index >= L2_ACTION_TABLE_MAX) {
		printf("[%s] Failed! Invalid index\n", __func__);
		goto ERR;
	}

	// get address
	address = L2_ACTION_TABLE;
	address += index * L2_ACTION_TABLE_ADDR_PER_ENTRY;

	//get old value
	for (i = 0; i < L2_ACTION_TABLE_ADDR_PER_ENTRY; i++)
		switch_read(address + i, &value[i]);

	//update
	if (!switch_field_setup(argc, argv, switch_l2_action_table, ARRAY_SIZE(switch_l2_action_table), value)) {
		printf("[%s] Failed! Invalid arg list\n", __func__);
		goto ERR;
	}

	//config
	for (i = 0; i < L2_ACTION_TABLE_ADDR_PER_ENTRY; i++)
		switch_write(address + i, value[i]);

	printf("[%s] index 0x%x val 0x%x\n", __func__, index, value[0]);

	return 1;
ERR:
	switch_l2_action_table_usage();

	return 0;
}

struct switch_field_def switch_l2_action_sport[] = {
	{L2_ACTION_TABLE_SPORT_ARG_NOLEARNINGUC, "noLearningUc", 1, 0},
	{L2_ACTION_TABLE_SPORT_ARG_NOLEARNINGMC, "noLearningMc", 1, 1},
	{L2_ACTION_TABLE_SPORT_ARG_DROPALL, "dropAll", 1, 2},
	{L2_ACTION_TABLE_SPORT_ARG_DROP, "drop", 1, 3},
	{L2_ACTION_TABLE_SPORT_ARG_DROPPORTMOVE, "dropPortMove", 1, 4},
	{L2_ACTION_TABLE_SPORT_ARG_SENDTOCPU, "sendToCpu", 1, 5},
	{L2_ACTION_TABLE_SPORT_ARG_FORCESENDTOCPUORIGPKT, "forceSendToCpuOrigPkt", 1, 6},
	{L2_ACTION_TABLE_SPORT_ARG_NOPORTMOVE, "noPortMove", 1, 7},
	{L2_ACTION_TABLE_SPORT_ARG_USESPECIALALLOW, "useSpecialAllow", 1, 8},
	{L2_ACTION_TABLE_SPORT_ARG_ALLOWPTR, "allowPtr", 2, 9},
	{L2_ACTION_TABLE_SPORT_ARG_MMPVALID, "mmpValid", 1, 11},
	{L2_ACTION_TABLE_SPORT_ARG_MMPPTR, "mmpPtr", 5, 12},
	{L2_ACTION_TABLE_SPORT_ARG_MMPORDER, "mmpOrder", 2, 17},
};

void switch_l2_action_sport_usage(void)
{
	printf("Usage: npecmd switch l2 action_table_sport <index> <--arg=val ...>\n");

	printf("index range: [0 - %d]\n", L2_ACTION_TABLE_SOURCE_PORT_MAX - 1);
	printf("arg :=");
	for (int i = 0; i < ARRAY_SIZE(switch_l2_action_sport); i++)
		printf(" %s", switch_l2_action_sport[i].name);

	printf("\n");
}

int switch_l2_action_sport_config(int argc, char **argv)
{
	u64 address;
	u32 value[L2_ACTION_TABLE_SOURCE_PORT_ADDR_PER_ENTRY], index;
	int i;

	if (argc <= 1) {
		printf("[%s] Failed! Invalid argc\n", __func__);
		goto ERR;
	}

	//check index
	if ((sscanf(argv[0], "%i", &index) != 1)
			|| index >= L2_ACTION_TABLE_SOURCE_PORT_MAX) {
		printf("[%s] Failed! Invalid index\n", __func__);
		goto ERR;
	}

	// get address
	address = L2_ACTION_TABLE_SOURCE_PORT;
	address += index * L2_ACTION_TABLE_SOURCE_PORT_ADDR_PER_ENTRY;

	//get old value
	for (i = 0; i < L2_ACTION_TABLE_SOURCE_PORT_ADDR_PER_ENTRY; i++)
		switch_read(address + i, &value[i]);

	//update
	if (!switch_field_setup(argc, argv, switch_l2_action_sport, ARRAY_SIZE(switch_l2_action_sport), value)) {
		printf("[%s] Failed! Invalid arg list\n", __func__);
		goto ERR;
	}

	//config
	for (i = 0; i < L2_ACTION_TABLE_SOURCE_PORT_ADDR_PER_ENTRY; i++)
		switch_write(address + i, value[i]);

	printf("[%s] index 0x%x val 0x%x\n", __func__, index, value[0]);

	return 1;
ERR:
	switch_l2_action_sport_usage();

	return 0;
}


void switch_l2_action_eport_usage(void)
{
	printf("Usage: npecmd switch l2 action_eport [value]\n");
	printf("Usage: npecmd switch l2 action_eport show\n");
	printf("value range: [0 - %d]\n", get_one_bits(0, EGRESS_PORT_CONFIGURATION_MAX - 1));
}

int switch_l2_action_eport_config(int argc, char **argv)
{
	u64 address;
	u32 value[1];
	int i;

	if (argc != 1) {
		printf("[%s] Failed! Invalid argc\n", __func__);
		goto ERR;
	}

	address = L2_ACTION_TABLE_EGRESS_PORT_STATE;

	if (argc == 1) {
		if (strcasecmp(argv[0], "show")) {

			//check value
			if ((sscanf(argv[0], "%i", &value[0]) != 1)
					|| value[0] > get_one_bits(0, EGRESS_PORT_CONFIGURATION_MAX - 1)) {
				printf("[%s] Failed! Invalid index\n", __func__);
				goto ERR;
			}

			//config
			switch_write(address, value[0]);
		}
	}
	
	//get new value
	switch_read(address, &value[0]);
	printf("[%s] val 0x%x\n", __func__, value[0]);

	return 1;
ERR:
	switch_l2_action_eport_usage();

	return 0;
}

struct switch_field_def switch_l2_learning_header[] = {
	{L2_LEARNING_HEADER_ARG_MAC, "MAC", 48, 0, SWITCH_FIELD_TYPE_MACADDR},
	{L2_LEARNING_HEADER_ARG_GID, "GID", 16, 48},
	{L2_LEARNING_HEADER_ARG_UNICAST_PORT_OR_MULTICAST_POINTER, "Unicast_Port_or_Multicast_Pointer", 32, 64},
	{L2_LEARNING_HEADER_ARG_UNICAST, "Unicast", 8, 96},
	{L2_LEARNING_HEADER_ARG_DROP, "Drop", 8, 104},
	{L2_LEARNING_HEADER_ARG_L2_DESTINATION_TABLE_ADDRESS, "L2_Destination_Table_Address", 32, 112},
	{L2_LEARNING_HEADER_ARG_VALID, "Valid", 8, 144},
	{L2_LEARNING_HEADER_ARG_STATIC, "Static", 8, 152},
	{L2_LEARNING_HEADER_ARG_HIT, "Hit", 8, 160},
	{L2_LEARNING_HEADER_ARG_L2_ACTION_TABLE_DA_STATUS, "L2_Action_Table_DA_Status", 8, 168},
	{L2_LEARNING_HEADER_ARG_L2_ACTION_TABLE_SA_STATUS, "L2_Action_Table_SA_Status", 8, 176},
	{L2_LEARNING_HEADER_ARG_META_DATA, "Meta_Data", 16, 184},
};

void switch_l2_inject_learning_usage(void)
{
	printf("Usage: npecmd switch l2 inject_learning <interface> <DA_MAC>  <--arg=val ...>\n");
	printf("interface: network interface\n");
	printf("DA_MAC: Destination MAC Address\n");
	printf("tag: Learning_tag\n");

	printf("arg :=");
	for (int i = 0; i < ARRAY_SIZE(switch_l2_learning_header); i++)
		printf(" %s", switch_l2_learning_header[i].name);

	printf("\n");
}

void switch_learning_tag_to_payload(u32 *tag, int tag_size, u8 *payload, int payload_len)
{
	int i, j, m;

	for (i = 0; i < tag_size; i++) {
		for (j = 0; j < 4; j++) {
			m = i * 4 + j;
			payload[m] = (tag[i] >> (8 * j)) & 0xFF;
			if (m == (payload_len -1))
				return;
		}
	}
}

int switch_l2_inject_learning_config(int argc, char **argv)
{
	int sockfd;
	struct ether_addr dest_mac;
	struct ether_header eth_hdr;
	struct sockaddr_ll sa;
	ssize_t bytes_sent;
	unsigned char src_mac[ETH_ALEN] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55};
	const char *payload_str;
	int ifindex;
	u32 tag[7] = {0};// 200bits
	int payload_len = 200 / 8;
	u8 payload[payload_len];
	u8 frame[sizeof(struct ether_header) + payload_len];

	if (argc <= 2) {
		printf("[%s] Failed! Invalid argc\n", __func__);
		goto ERR;
	}

	// get network interface index
	ifindex = if_nametoindex(argv[0]);
	if (ifindex == 0) {
		printf("[%s] Invalid interface! Failed to if_nametoindex %s\n", __func__, argv[0]);
		close(sockfd);
		goto ERR;
	}

	//maybe this macaddr can obtain by SIOCGIFHWADDR
	if (ether_aton_r(argv[1], &dest_mac) == NULL) {
		printf("[%s] Invalid Macaddr! Failed to ether_aton_r\n", __func__);
		return EXIT_FAILURE;
	}

	// construct ethernet header
	memcpy(eth_hdr.ether_dhost, &dest_mac, ETH_ALEN);
	memcpy(eth_hdr.ether_shost, src_mac, ETH_ALEN);
	eth_hdr.ether_type = htons(ETH_P_ALL);

	//update
	if (!switch_field_setup(argc - 1, argv + 1, switch_l2_learning_header, ARRAY_SIZE(switch_l2_learning_header), tag)) {
		printf("[%s] Failed! Invalid arg list\n", __func__);
		goto ERR;
	}

	// exchange tag to payload
	switch_learning_tag_to_payload(tag, sizeof(tag), payload, payload_len);

	// constuct completed ethernet frame
	memcpy(frame, &eth_hdr, sizeof(struct ether_header));
	memcpy(frame + sizeof(struct ether_header), payload, payload_len);

	// construct socket raw
	sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	if (sockfd == -1) {
		printf("[%s] Failed to socket\n", __func__);
		goto ERR;
	}

	// construct sockaddr_ll
	memset(&sa, 0, sizeof(sa));
	sa.sll_family = AF_PACKET;
	sa.sll_protocol = htons(ETH_P_ALL);
	sa.sll_ifindex = ifindex;

	// send packet
	bytes_sent = sendto(sockfd, frame, sizeof(struct ether_header) + payload_len, 0, (struct sockaddr *)&sa, sizeof(sa));
	if (bytes_sent == -1) {
		printf("[%s] Failed to sendto\n", __func__);
		close(sockfd);
		goto ERR;
	}

	printf("Sent %zd bytes\n", bytes_sent);

	close(sockfd);

	return 1;
ERR:
	switch_l2_inject_learning_usage();

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
	{"action_table", switch_l2_action_table_config, switch_l2_action_table_usage},
	{"action_table_sport", switch_l2_action_sport_config, switch_l2_action_sport_usage},
	{"action_eport", switch_l2_action_eport_config, switch_l2_action_eport_usage},
	{"inject_learning", switch_l2_inject_learning_config, switch_l2_inject_learning_usage},
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
