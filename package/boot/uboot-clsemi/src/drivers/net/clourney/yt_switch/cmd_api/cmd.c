#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "chipdef_tiger.h"
#include "api_include.h"
#include "cmd_tool.h"
#include "cmds_type_id.h"
#include "cmds_type.h"
#include "cmds_type_list.h"
#include "cmds_func.h"
#include "cmds_func_list.h"
#define enable  	1
#define Port0       0
#define Port1       1
#define Port2 		2
#define Port3 	    3
#define Port4 		4
#define Port5 		5
int g_debug_switch;
int get_Id_by_str(char * str, em_str_to_id_t * map, int map_size)
{
	int i;
	for (i=0; i< map_size - 1; i++) {
		if (0 == strcasecmp(map[i].name, str))
			return map[i].id;
	}

	return map[i].id;
}

void print_id(char * name , int id, em_str_to_id_t * map, int map_size)
{
	int i;
	for (i=0; i< map_size - 1; i++)
		if (map[i].id == id)
			break;

	printf("%40s:%40s\n", name, map[i].name);
}

int paramter_set(int type, char * address, char * value_str)
{

	switch (type)
	{
#if 0
	case T_VLAN_PROTOVLAN_FRAMETYPE:
		{
			uint32_t value = get_Id_by_str(value_str,
					rtk_vlan_protoVlan_frameType_map,
					sizeof(rtk_vlan_protoVlan_frameType_map) / sizeof(rtk_vlan_protoVlan_frameType_map[0]));

			memcpy(address, &value,  sizeof(rtk_vlan_protoVlan_frameType_t));
		}
		break;
#endif
	case T_UINT8_T:
	case T_CHAR:
		{
			uint32_t value = simple_strtoul(value_str, NULL, 0);
			memcpy(address, &value,  sizeof(uint8_t));
		}
		break;
	case T_UINT32_T:
		{
			uint32_t value = simple_strtoul(value_str, NULL, 0);
			memcpy(address, &value,  sizeof(uint32_t));
		}
		break;
	case T_UINT16_T:
		{
			uint32_t value = simple_strtoul(value_str, NULL, 0);
			memcpy(address, &value,  sizeof(uint16_t));
		}
		break;
	case T_UINT64_T:
		{
			uint64_t value = simple_strtoull(value_str, NULL, 0);
			memcpy(address, &value,  sizeof(uint64_t));
		}
		break;
	case T_MAC:
		{
			yt_mac_addr_t  * mac = (yt_mac_addr_t *) address;
		//	if (sscanf(value_str, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", &mac->addr[5], &mac->addr[4], &mac->addr[3], &mac->addr[2], &mac->addr[1], &mac->addr[0]) != 6) {
			if (sscanf(value_str, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", &mac->addr[0], &mac->addr[1], &mac->addr[2], &mac->addr[3], &mac->addr[4], &mac->addr[5]) != 6) {
				printf("Can't convert MAC String(%s) to ether_addr!!!\n", value_str);
				return 1;
			}
		}
		break;
#if 0
	case T_IPADDR:
		{
			unsigned char * ip  = (unsigned char *) address;
			if (sscanf(value_str, "%hhu.%hhu.%hhu.%hhu", ip+3, ip+2, ip+1, ip) != 4) {
				printf("Can't convert ipaddr to string: %s\n", value_str);
				return;
			}
		}
		break;
#endif
	default:
		//printf("Unkown Parameter type[%d] V:[%s]!\n", type, value_str);
		return 0;
	}

	return 1;
}

int convert_parameters_from_str(void * obj, int type, int * pargc, char ** argv)
{
	int index;
	int count = 0;
	object_type_st * obj_desc_ptr = NULL;

	count = paramter_set(type, obj, argv[0]);
	if (count)
		goto END;

	if (type >= T_TYPE_MAX_NUM) {
		printf("%s Unkown type %d\n", __func__, type);
		return 0;
	}

	obj_desc_ptr = &all_type_table[type];

	if (obj_desc_ptr->flags == F_ENUM) {

		if(*pargc <= 0) {
			printf("NO many parameters Exist(argc:%d)!\n", *pargc);
			return -1;
		}

		*((uint8_t *) obj) = get_Id_by_str(argv[0], obj_desc_ptr->object, obj_desc_ptr->len);

		if (g_debug_switch & 0x10)
			printf("ENUM T:%d, N:%s V%d\n", type, argv[0], *((uint8_t *) obj));

		count++;

	} else if (obj_desc_ptr->flags == F_STRUCT) {

		int list_size = obj_desc_ptr->len;
		parameters_convert_st * paramter_list = obj_desc_ptr->object;

		for (index = 0; index < *pargc - 1; index +=2) {
			int param_index = 0;
			if (*pargc - count < 2) {
				if (g_debug_switch & 0x10)
					printf("argc is end\n");
				goto END;
			}

			if (!strcasecmp(argv[index], ";")) {
				count ++;
				goto END;
			}

			for (param_index = 0; param_index < list_size; param_index ++) {
				if (0 == strcasecmp(argv[index], paramter_list[param_index].name)) {
					int argc = *pargc - index - 1;
					if (g_debug_switch & 0x10) {
						printf("Set para [%s] value[%s] \n",argv[index], argv[index + 1]);

						printf("Struct [%d]->T:%d,OF:%d N:%s\n", param_index,
								paramter_list[param_index].type,
								paramter_list[param_index].offset,
								paramter_list[param_index].name);
					}

					convert_parameters_from_str(((char *) (obj)) + paramter_list[param_index].offset,
							paramter_list[param_index].type, &argc, &argv[index + 1]);

					count = *pargc - argc;
					break;
				}

				if (param_index == list_size) {
					printf("Unkown Parameter [%s]\n", argv[index]);
					break;
				}
			}
		}
	} else {
		printf("%s Unkown flags %d\n", __func__, obj_desc_ptr->flags);
	}

END:
    *pargc -= count;

	return count;
}

int paramter_print(void * buf, int type, char * address, char * name)
{

	switch (type)
	{
#if 0
	case T_VLAN_PROTOVLAN_FRAMETYPE:
		print_id(name, address,
				rtk_vlan_protoVlan_frameType_map,
				sizeof(rtk_vlan_protoVlan_frameType_map) / sizeof(rtk_vlan_protoVlan_frameType_map[0]));
		break;
#endif
	case T_UINT8_T:
	case T_CHAR:
		{
			printf("%35s: %u\n", name, *((uint8_t *) address));
		}
		break;
	case T_UINT32_T:
		{
			//return sprintf(buf, "%35s: %u\n", name, *((uint32_t *) address));
			printf("%35s: %u\n", name, *((uint32_t *) address));
		}
		break;
	case T_UINT16_T:
		{
			//return sprintf(buf, "%35s: %u\n", name, *((uint32_t *) address));
			printf("%35s: %u\n", name, *((uint16_t *) address));
		}
		break;
	case T_UINT64_T:
		{
			//return sprintf(buf, "%35s: %llu\n", name, *((uint64_t *) address));
			printf("%35s: %llu\n", name, *((uint64_t *) address));
		}
		break;
	case T_MAC:
		{
			char mac_str[20] = {0};
			yt_mac_addr_t  * mac = (yt_mac_addr_t *) address;

			//snprintf(mac_str, sizeof(mac_str), "%02x:%02x:%02x:%02x:%02x:%02x", mac->addr[5], mac->addr[4], mac->addr[3], mac->addr[2], mac->addr[1], mac->addr[0]);
			snprintf(mac_str, sizeof(mac_str), "%02x:%02x:%02x:%02x:%02x:%02x", mac->addr[0], mac->addr[1], mac->addr[2], mac->addr[3], mac->addr[4], mac->addr[5]);
			//return sprintf(buf, "%20s:%20s\n", name, mac_str);
			printf("%20s:%20s\n", name, mac_str);
			return  0;
		}
		break;
#if 0
	case T_IPADDR:
		{

			char ip_str[20] = {0};
	        unsigned char * ip = (unsigned char *) address;

			snprintf(ip_str, sizeof(ip_str), "%d.%d.%d.%d", ip[3], ip[2], ip[1], ip[0]);

			//return sprintf(buf, "%20s:%20s\n", name, ip_str);
			return printf("%20s:%20s\n", name, ip_str);
		}
		break;
#endif
	default:
		//printf("Unkown Parameter type[%d] Name:[%s]!\n", type, name);
		return -1;
	}
	return 0;
}

void convert_parameters_to_str(void * obj, int type, char * name)
{
	//static char buf[MAX_PRINT_STR_LEN] = {0};
	static char buf[1] = {0};
	int  param_index;
	object_type_st * obj_desc_ptr = NULL;
	int ret;

	memset(buf, 0, sizeof(buf));

	ret = paramter_print(buf, type, obj, name);
	if (!ret)
		return;

	if (type >= T_TYPE_MAX_NUM) {
		printf("%s Unkown type %d\n", __func__, type);
		return;
	}

	obj_desc_ptr = &all_type_table[type];

	if (obj_desc_ptr->flags == F_ENUM) {
		print_id(name, (*((int *) obj)), obj_desc_ptr->object, obj_desc_ptr->len);
	} else {
		int list_size = obj_desc_ptr->len;
		parameters_convert_st * paramter_list = obj_desc_ptr->object;
		printf("%35s:", name);
		for (param_index  = 0; param_index < list_size; param_index ++) {
			if (g_debug_switch & 0x10)
				printf("Struct [%d]->T:%d,OF:%d N:%s\n", param_index, paramter_list[param_index].type,paramter_list[param_index].offset,paramter_list[param_index].name);
			convert_parameters_to_str(((char *) (obj)) + paramter_list[param_index].offset,
					paramter_list[param_index].type,
					paramter_list[param_index].name);
		}
		//printf("%s\n", buf);
	}
	return;
}

int yt_cmd_main(int argc, char *argv[])
{
	int func_id = 0;
	cmd_func_t * func_desc_ptr = NULL;
	char * cmd = NULL;

	if (argc < 1) {
		printf("Parameter num too long\n");
		return 0;
	}

	cmd = argv[0];
	argc -= 1;
	if (0 == strcasecmp("yt_modules_init", cmd)) {

		if (yt_modules_init()) {
			printk("yt_modules_init Error!\n");
			return 0;
		}

		printk("ret: 0");

	} else if (0 == strcasecmp("init_app", cmd)) {
		yt_port_mask_t member_portmask;
		yt_extif_mode_t mode = 0;
		yt_port_force_ctrl_t port_ctrl;
		yt_port_mask_t untag_portmask;
		uint32_t unit = 0;

		/*set Port5 HSGMII Mode，Force LinkRate 2.5G Full */
		/*use logic Port5 as physical GMac8, use logic Port6 as physical GMac9,*/
		yt_port_extif_mode_set(unit, 5, YT_EXTIF_MODE_BX2500);
		port_ctrl.speed_dup = PORT_SPEED_DUP_2500FULL;
		port_ctrl.rx_fc_en = 1;
		port_ctrl.tx_fc_en = 1;
		yt_port_mac_force_set(unit, 5, port_ctrl);

		/*set Port 5 as extern CPU port and CPU Tag disable*/
		yt_nic_cpuport_mode_set(unit, CPUPORT_MODE_EXTERNAL);
		yt_nic_ext_cpuport_en_set(unit, enable);
		yt_nic_ext_cpuport_port_set(unit, Port5);
		yt_nic_ext_cputag_en_set(unit, 0);

		yt_port_extif_mode_get(unit, 5, &mode);

		/*Vlan 1 group member, P0~P3 untagged port ,default PVID 1*/
		member_portmask.portbits[0] = 0xF;
		untag_portmask.portbits[0] = 0xF;
		yt_vlan_fid_set(unit, 1, 1);
		yt_vlan_port_set(unit, 1, member_portmask, untag_portmask);
		yt_vlan_port_igrPvid_set(unit, 0, Port0, 1);
		yt_vlan_port_igrPvid_set(unit, 0, Port1, 1);
		yt_vlan_port_igrPvid_set(unit, 0, Port2, 1);
		yt_vlan_port_igrPvid_set(unit, 0, Port3, 1);

		/*Vlan 2 group member, P4 untagged port ,default PVID 2*/
		member_portmask.portbits[0] = 0x10;
		yt_vlan_fid_set(unit, 2, 2);
		yt_vlan_port_set(unit, 2, member_portmask, untag_portmask);
		yt_vlan_port_igrPvid_set(unit, 0, Port4, 2);

		/*set Port0~Port4 Egress Entry Mode*/
		yt_vlan_port_egrTagMode_set(unit, VLAN_TYPE_CVLAN, Port0, VLAN_TAG_MODE_ENTRY_BASED);
		yt_vlan_port_egrTagMode_set(unit, VLAN_TYPE_CVLAN, Port1, VLAN_TAG_MODE_ENTRY_BASED);
		yt_vlan_port_egrTagMode_set(unit, VLAN_TYPE_CVLAN, Port2, VLAN_TAG_MODE_ENTRY_BASED);
		yt_vlan_port_egrTagMode_set(unit, VLAN_TYPE_CVLAN, Port3, VLAN_TAG_MODE_ENTRY_BASED);
		yt_vlan_port_egrTagMode_set(unit, VLAN_TYPE_CVLAN, Port4, VLAN_TAG_MODE_ENTRY_BASED);

		/*set Port0~Port4 Ingress Filter Enable*/
		yt_vlan_port_igrFilter_enable_set(unit, Port0, enable);
		yt_vlan_port_igrFilter_enable_set(unit, Port1, enable);

		yt_vlan_port_igrFilter_enable_set(unit, Port2, enable);
		yt_vlan_port_igrFilter_enable_set(unit, Port3, enable);
		yt_vlan_port_igrFilter_enable_set(unit, Port4, enable);

		/*set Port0~Port4 Egress Filter Enable*/
		yt_vlan_port_egrFilter_enable_set(unit, Port0, enable);
		yt_vlan_port_egrFilter_enable_set(unit, Port1, enable);
		yt_vlan_port_egrFilter_enable_set(unit, Port2, enable);
		yt_vlan_port_egrFilter_enable_set(unit, Port3, enable);
		yt_vlan_port_egrFilter_enable_set(unit, Port4, enable);

	} else if (0 == strcasecmp("debug", cmd)) {

		if (argc >= 2)
			g_debug_switch = simple_strtoul(argv[1], NULL, 0);
		else
			g_debug_switch = !g_debug_switch;

		printk("Debug is %s\n", g_debug_switch ? "OPEN" : "CLOSE");

	} else {
		for (func_id = 0; func_id < sizeof(func_str_list)/sizeof(func_str_list[0]); func_id++) {
			func_desc_ptr = &func_str_list[func_id];
			if (0 == strcasecmp(func_desc_ptr->name, cmd)) {
				func_desc_ptr->func(argc, &argv[1]);
				return 0;
			}
		}
		printf("Can't find the cmd[%s]\n", cmd);
	}

	return 0;
}
