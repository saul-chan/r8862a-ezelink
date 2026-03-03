#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "calicmd.h"

int get_cmd_type(char *name)
{
    int i = 0;
    enum cali_cmd_type type = TYPE_MAX;
    for (i; i < ARRAY_SIZE(cmd_type); i++) {
        if (!strcasecmp(name, cmd_type[i]))
            break;
    }
    if (i < ARRAY_SIZE(cmd_type))
        type = i;
    return type;
}

int check_int_value(enum cali_cmd_type type, char *name, char *value)
{
    int ret = 0;
    int i = 0;
    int j = 0;
    int tmp = 0;
    for (; i < ARRAY_SIZE(name_int_value_config); i++) {
        if (type != name_int_value_config[i].type)
            continue;
        if (!strcasecmp(name, name_int_value_config[i].name)) {
            ret |= 0x1;
            tmp = strtol(value, NULL, 0);
            if (tmp >= name_int_value_config[i].minval &&
                tmp <= name_int_value_config[i].maxval) {
                ret |= 0x2;
                break;
            }
        }
    }
    return ret;
}

int check_string_value(enum cali_cmd_type type, char *name, char *value)
{
    int ret = 0;
    int i = 0;
    int j = 0;

    for (; i < ARRAY_SIZE(name_string_value_config); i++) {
        if (type != name_string_value_config[i].type)
            continue;

        if (!strcasecmp(name, name_string_value_config[i].name)) {
            ret |= 0x1;
            j = 0;
            while (name_string_value_config[i].value_list[j] != NULL)
            {
                if (!strcasecmp(value, name_string_value_config[i].value_list[j])) {
                    ret |= 0x2;
                    return ret;
                }
                j++;
            }
       }
    }
    return ret;
}

int check_hex_value(char *name, char *value)
{
    return 0;
}

int check_variable_name(enum cali_cmd_type type, char *name)
{
    int ret = 0;
    int i = 0;
    int j = 0;
    for (; i < ARRAY_SIZE(type_name_config); i++) {
        if (type != type_name_config[i].type)
            continue;
        j = 0;
        while (type_name_config[i].name_list[j] != NULL)
        {
            if (!strcasecmp(name, type_name_config[i].name_list[j])) {
                ret = 1;
                return ret;
            }
            j++;
        }
    }
    return ret;
}

int main(int argc, char **argv)
{
    char cmd_str[1024];
    char tmp_buf[128];
    char tmp[64];
    int idx = 0;
    int mu_id = 0;
    int start_index = 0;
    float err = 0;
    enum cali_cmd_type type = TYPE_MAX;

    if (argc < 2) {
        printf("%s\n", help);
        goto ERROR_RE;
    }

    if (argc == 2) {
        if (strcasecmp(argv[1], "update") && strcasecmp(argv[1], "tx-su")
		&& strcasecmp(argv[1], "tx-mu") && strcasecmp(argv[1], "reset")
		&& strcasecmp(argv[1], "sounding") && strcasecmp(argv[1], "hard-reset")
		&& strcasecmp(argv[1], "tx-mld") && strcasecmp(argv[1], "update-params-only")) {
            printf("please check the argument, the list is update, tx-su, tx-mu, reset and sounding\n");
            goto ERROR_RE;
        }
    }

    if (!strcasecmp(argv[1], "set")) {
        type = get_cmd_type(argv[2]);
        if (type == TYPE_MAX) {
            printf("please add valid type for set command\n");
            goto ERROR_RE;
        }
        if (!strcasecmp(argv[2], "radio") || !strcasecmp(argv[2], "rx-stats")) {
            if (argc < 4) {
                printf("the least argument number for setting radio/rx-stats is 4\n");
                goto ERROR_RE;
            }
            start_index = 2;
        } else if (!strcasecmp(argv[2], "ptk") || !strcasecmp(argv[2], "gtk")
                     || !strcasecmp(argv[2], "su") || !strcasecmp(argv[2], "bss")
                     || !strcasecmp(argv[2], "mac-phy") || !strcasecmp(argv[2], "mu-info")
                     || !strcasecmp(argv[2], "sounding")) {
            if (argc < 5) {
                printf("the least argument number is 5 for bss, ptk, gtk and su type\n");
                goto ERROR_RE;
            }
            if (argc % 2 == 0) {
                printf("The last name-value pair does not have value\n");
                goto ERROR_RE;
            }
            if (!strcasecmp(argv[2], "ptk") || !strcasecmp(argv[2], "gtk")
                     || !strcasecmp(argv[2], "su")) {
                if (strcasecmp(argv[3], "mac-addr")) {
                    printf("the first name-value pair must be mac-addr for setting ptk, gtk and su\n");
                    goto ERROR_RE;
                }
                start_index = 5;
            } else {
                start_index = 3;
            }
        } else if (!strcasecmp(argv[2], "mu")) {
            mu_id = strtol(argv[3], NULL, 10);
            if (mu_id < 0 || mu_id > 32) {
                printf("the mu id is between 1 and 32\n");
                goto ERROR_RE;
            }
            if (argc < 6) {
                printf("The least argumment number for set mu is 6\n");
                goto ERROR_RE;
            }
            if (argc % 2 == 1) {
                printf("The last name-value pair does not have value\n");
                goto ERROR_RE;
            }
            if (strcasecmp(argv[4], "mac-addr")) {
                printf("the first name-value pair must be mac-addr for mu type\n");
                goto ERROR_RE;
            }
            start_index = 6;
        } else if (!strcasecmp(argv[2], "wmm")) {
            mu_id = strtol(argv[3], NULL, 10);
            if (mu_id < 0 || mu_id > 4) {
                printf("the wmm id is between 1 and 4\n");
                goto ERROR_RE;
            }
            if (argc < 6) {
                printf("The least argumment number for setting wmm is 6\n");
                goto ERROR_RE;
            }
            if (argc % 2 == 1) {
                printf("The last name-value pair does not have value\n");
                goto ERROR_RE;
            }
            start_index = 4;
        } else if (!strcasecmp(argv[2], "pppc")) {
            if (argc < 5) {
                printf ("The least argument number for setting pppc is 5, including enable 0/1\n");
                goto ERROR_RE;
            }
	} else if (!strcasecmp(argv[2], "mld")) {
		// TODO. Check MLD parameters later.
		start_index = 0;
	} else if (!strcasecmp(argv[2], "ap-mld")) {
		// TODO. Check MLD parameters later.
		start_index = 0;
	} else if (!strcasecmp(argv[2], "sta-mld")) {
		// TODO. Check MLD parameters later.
		start_index = 0;
	}
    }else if (!strcmp(argv[1], "show")) {
        type = get_cmd_type(argv[2]);
        if (type == TYPE_MAX) {
            if (strcasecmp(argv[2], "rx-stats") && strcasecmp(argv[2], "rx-last")
                && strcasecmp(argv[2], "tx-stats") && strcasecmp(argv[2], "rssi-stats")
                && strcasecmp(argv[2], "rssi-stats-avg") && strcasecmp(argv[2], "leg-rssi-stats")
                && strcasecmp(argv[2], "leg-rssi-stats-avg") && strcasecmp(argv[2], "temperature")
                && strcasecmp(argv[2], "csi-last")) {
                printf("please check the argument after show\n");
                goto ERROR_RE;
            }
        }
    } else if (!strcmp(argv[1], "read-mem") || !strcmp(argv[1], "write-mem")) {
        if (argc < 4) {
            printf("The least number for read-mem and write-mem is 4\n");
            goto ERROR_RE;
        }
    } else if (!strcmp(argv[1], "radar-detect")) {
        if (argc < 4) {
            printf("The least number for radar-detect is 4\n");
            goto ERROR_RE;
        }
        type = TYPE_RADAR;
        start_index = 2;
    } else if (!strcmp(argv[1], "interference-detect")) {
        if (argc < 4) {
            printf("The least number for interference-detect is 4\n");
            goto ERROR_RE;
        }
        type = TYPE_INTERFERENCE;
        start_index = 2;
    } else if (!strcmp(argv[1], "rssi")) {
        if (argc < 8) {
            printf("The least number for interference-detect is 8\n");
            goto ERROR_RE;
        }
        type = TYPE_RSSI;
        start_index = 2;
    } else if (strcasecmp(argv[1], "update") && strcasecmp(argv[1], "tx-su")
            && strcasecmp(argv[1], "tx-mu") && strcasecmp(argv[1], "reset")
            && strcasecmp(argv[1], "sounding") && strcasecmp(argv[1], "dif-sample")
            && strcasecmp(argv[1], "log") && strcasecmp(argv[1], "hard-reset")
		&& strcasecmp(argv[1], "irf") && strcasecmp(argv[1], "csi")
		&& strcasecmp(argv[1], "tx-mld") && strcasecmp(argv[1], "time-sync")
		&& strcasecmp(argv[1], "time-get") && strcasecmp(argv[1], "tx-hw-agg")
		&& strcasecmp(argv[1], "update-params-only")) {
            printf("The first argument should be one of set, show, update, tx-su, tx-mu, reset and sounding\n");
            goto ERROR_RE;
    }

    if (start_index > 0) {
        for (; start_index < argc - 1;) {
            int  ret = 0;
            ret = check_variable_name(type, argv[start_index]);
            if (!ret) {
                printf("check the variable name[%s], it is not acceptable.\n", argv[start_index]);
                goto ERROR_RE;
            }

            ret = check_string_value(type, argv[start_index], argv[start_index + 1]);
            printf("type = %d, ret = 0x%x\n", type, ret);
            if (ret & 0x1) {
                if (ret & (1 << 1)) {
                    start_index += 2;
                    continue;
                }
                else {
                    printf("check the value of %s[%s], it is not a acceptable string\n",
                        argv[start_index], argv[start_index + 1]);
                    goto ERROR_RE;

                }
            }

            ret = check_int_value(type, argv[start_index], argv[start_index + 1]);
            if (ret & 0x1) {
                if (ret & (1 << 1)) {
                    start_index += 2;
                    continue;
                }
                else {
                    printf("check the value of %s[%s], it is out of the range\n",
                            argv[start_index], argv[start_index + 1]);
                    goto ERROR_RE;
                }
            }
            start_index += 2;
        }
    }

    if (!strcasecmp(argv[1], "irf") && !strcasecmp(argv[2], "xtal_cal") &&
        strcasecmp(argv[3], "start")) {
        sscanf(argv[4], "%f", &err);
        if (err >= 0) {
            sprintf(tmp, "%d", (int)(100 * err + 0.5));
        } else {
            sprintf(tmp, "%d", (int)(100 * err - 0.5));
        }
        argv[4] = tmp;
    }

    strcpy(cmd_str, "echo ");
    if (argc == 2) {
        sprintf(tmp_buf, "\"%s\" > ", argv[1]);
        strcat(cmd_str, tmp_buf);
    } else {
        for (idx = 1; idx < argc; idx++) {
            if (idx == 1)
                sprintf(tmp_buf, "\"%s ", argv[idx]);
            else if (idx == argc - 1)
                sprintf(tmp_buf, "%s\" > ", argv[idx]);
            else
                sprintf(tmp_buf, "%s ", argv[idx]);

            strcat(cmd_str, tmp_buf);
        }
    }
    strcat(cmd_str, filepath_set);
    system(cmd_str);

    if (!strcasecmp(argv[1], "read-mem") || !strcasecmp(argv[1], "show")) {
        memset(cmd_str, 0, sizeof(cmd_str));
        sprintf(cmd_str, "cat %s", filepath_show);
        system(cmd_str);
    }

ERROR_RE:
    return 0;

}
