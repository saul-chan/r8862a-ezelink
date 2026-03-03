#include "power_tbl.h"

void usage(void)
{
	printf("Usage:\n");
	printf("power_tbl <radio> [project]\n");
}

uint8_t valid_2G_channels[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14};
int num_valid_2G_channels = sizeof(valid_2G_channels) / sizeof(valid_2G_channels[0]);

uint8_t valid_5G_channels[] = {36, 40, 44, 48, 52, 56, 60, 64,
			100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140, 144,
			149, 153, 157, 161, 165};
int num_valid_5G_channels = sizeof(valid_5G_channels) / sizeof(valid_5G_channels[0]);

bool isValidChannel(int radio, int channel)
{
	int i;

	if (radio == RADIO_2G) {
		for (i = 0; i < num_valid_2G_channels; i++) {
			if (valid_2G_channels[i] == channel) {
				return true;
			}
		}
	} else if (radio == RADIO_5G) {
		for (i = 0; i < num_valid_5G_channels; i++) {
			if (valid_5G_channels[i] == channel) {
				return true;
			}
		}
	}

	return false;
}

void dump_txpower_entry(struct txpower_db_entry entry)
{
	int i;

	printf("[%s] radio %s phymode %s chan %d bw %d:\n", __func__, entry.radio == RADIO_2G ? "2G" : "5G",
			entry.phymode == PHYMODE_11AC ? "11AC" : "11AX", entry.chan,  entry.bw);

	for (i = 0; i < MAX_MCS_NUM; i++) {
		printf("\t[mcs%d] %d;\n", i, entry.power_list[i]);
	}
}

void reset_drv_txpower(RadioType radio)
{
	FILE *fp = NULL;
	char *debugfs_path = NULL;
	char cmd[8] = "reset";

	if (radio == RADIO_2G)
		debugfs_path = TXPOWER_DB_CONFIG_DEBUGFS_2G;
	else
		debugfs_path = TXPOWER_DB_CONFIG_DEBUGFS_5G;

	fp = fopen(debugfs_path, "w");
	if (!fp) {
		printf("[%s] cannot open debugfs: %s\n", __func__, debugfs_path);
		return;
	}

	if (fwrite(cmd, sizeof(char), sizeof(cmd), fp) != sizeof(cmd))
		printf("[%s] Failed to write reset to debugfs: %s\n", __func__, debugfs_path);

	fclose(fp);
}

void sync_txpower_to_drv(struct txpower_db_entry entry)
{
	FILE *fp = NULL;
	char pwr_tbl[128] = {0}, cmd[160] = {0};
	char *debugfs_path = NULL;
	int len;

	#if 0
	dump_txpower_entry(entry);
	#endif

	if (entry.radio == RADIO_2G)
		debugfs_path = TXPOWER_DB_CONFIG_DEBUGFS_2G;
	else
		debugfs_path = TXPOWER_DB_CONFIG_DEBUGFS_5G;

	fp = fopen(debugfs_path, "w");
	if (!fp) {
		printf("[%s] cannot open debugfs: %s\n", __func__, debugfs_path);
		return;
	}

	len = sizeof(entry.radio) + sizeof(entry.phymode) + sizeof(entry.chan) + sizeof(entry.bw);
	len += MAX_MCS_NUM * sizeof(entry.power_list[0]);

	if (fwrite(&entry, sizeof(char), len, fp) != len)
		printf("[%s] Failed to write power to debugfs: %s\n", __func__, debugfs_path);

	fclose(fp);
}

bool isValid_pre_info(struct txpower_db_entry entry)
{
	if (entry.radio > RADIO_MAX || entry.radio < RADIO_MIN ) {
		printf("[%s] invalid radio %d\n", __func__, entry.radio);
		return false;
	}

	if (entry.phymode > PHYMODE_MAX || entry.phymode < PHYMODE_MIN) {
		printf("[%s] invalid phymod %d\n", __func__, entry.phymode);
		return false;
	}

	if ((entry.radio == RADIO_2G && entry.bw > BW_2G_MAX)
			|| (entry.radio == RADIO_5G && entry.bw > BW_5G_MAX)
			|| entry.bw < BW_MIN ) {
		printf("[%s] invalid radio %d and bw %d\n", __func__, entry.radio, entry.bw);
		return false;
	}

	if (!isValidChannel(entry.radio, entry.chan)) {
		printf("[%s] invalid radio %d and chan %d\n", __func__, entry.radio, entry.chan);
		return false;
	}

	return true;
}

int parse_post_info(const char *str, int8_t *numbers, int len)
{
	int numCount = 0;
	int num = 0;
	int sign = 1; //Used to handle possible negative signs
	int isNum = 0; //Mark whether the number is currently being read

	//Traverse every character in the str
	for (int i = 0; str[i] != '\0' && numCount < len; i++) {
		if (isspace(str[i])) { //ignore 'space'
			if (isNum) {
				// store the previous num, and start next num
				numbers[numCount++] = num * sign;// Possible negative sign
				num = 0;  
				sign = 1;  
				isNum = 0;  
			}

			continue;  
		}  
  
		// Possible negative sign
		if (str[i] == '-') {
			// The negative sign needs to be before the number,
			if (isNum) //Error: Incorrect negative sign position
				return 0;

			sign = -1;  
			isNum = 1;

			continue;  
		}

		if (isdigit(str[i])) {  
			// Accumulate num
			num = num * 10 + (str[i] - '0');
			isNum = 1;

			continue;
		}

		// if str[i] is non-numeric or non-space characters, return an error
		return 0;
	}

	// Check if all numbers have been correctly stored
	if (isNum) {
		/* The last number is not separated by a space and needs to be manually stored */
		numbers[numCount++] = num;
	}

	// check if numCount is enough
	return numCount == len ? 1 : 0;
}

void parse_txpower_db_and_sync2drv(FILE *fp, RadioType radio)
{
	char power_info[128] = {0};
	char *data;
	int ret = 0;
	struct txpower_db_entry power_entry;

	power_entry.radio = radio;

	while (fgets(&power_info[0], sizeof(power_info), fp) != NULL) {
		if (power_info[0] != '#') {
			ret = sscanf(&power_info[0], TXPOWER_ENTRY_PRE_FORMAT, &power_entry.chan, &power_entry.phymode, &power_entry.bw);

			if (ret == 3 && isValid_pre_info(power_entry)) {
				data = &power_info[0];
				//ignore pre_info
				while (*data != ' ') {
					data++;
				}

				memset(power_entry.power_list, -1, sizeof(power_entry.power_list));

				ret = parse_post_info(data, power_entry.power_list, MAX_MCS_NUM);
				if (ret)
					sync_txpower_to_drv(power_entry);
				else {
					printf("[%s] invalid post_info: <%s>\n", __func__, power_info);
				}
			} else {
				printf("[%s] invalid pre_info: <%s>\n", __func__, power_info);
			}
		}

		memset(power_info, 0, sizeof(power_info));
	}
}

enum cls_wifi_hw_rev parse_platform_hw_rev(uint8_t radio)
{
	FILE *fp = NULL;
	char hw_rev_str[32];
	enum cls_wifi_hw_rev hw_rev;

	if (radio == RADIO_2G)
		fp = fopen(TXPOWER_DB_CHIP_ID_DEBUGFS_2G, "r");
	else
		fp = fopen(TXPOWER_DB_CHIP_ID_DEBUGFS_5G, "r");

	if (!fp) {
		printf("failed to open %s: errno = %d\n",
			(radio == RADIO_2G) ? TXPOWER_DB_CHIP_ID_DEBUGFS_2G : TXPOWER_DB_CHIP_ID_DEBUGFS_5G, errno);
		return CLS_WIFI_HW_MAX_INVALID;
	}
	if (fgets(hw_rev_str, 32, fp) != NULL) {
		if (strncmp(hw_rev_str, "m2k", 3) == 0)
			hw_rev =  CLS_WIFI_HW_MERAK2000;
	} else
		hw_rev =  CLS_WIFI_HW_DUBHE2000;

	fclose(fp);
	return hw_rev;
}

int main(int argc, char *argv[])
{
	FILE *fp = NULL;
	char filename_project[64] = {0};
	char filename_default[64] = {0};
	RadioType radio;
	enum cls_wifi_hw_rev hw_rev;

	if (argc > 3) {
		usage();
		return -1;
	}

	if (!strcasecmp(argv[1], "5g")) {
		radio = RADIO_5G;
		hw_rev = parse_platform_hw_rev(radio);

		snprintf(filename_default, sizeof(filename_default), "%s%stxpower_%s_5g.txt",
				TXPOWER_DB_BASE_FILENAME, (hw_rev == CLS_WIFI_HW_MERAK2000) ? "m2k/" : "",
				TXPOWER_DB_DEFAULT_PROJECT);

		if (argc > 2)
			snprintf(filename_project, sizeof(filename_project), "%s%stxpower_%s_5g.txt",
				TXPOWER_DB_BASE_FILENAME, (hw_rev == CLS_WIFI_HW_MERAK2000) ? "m2k/" : "", argv[2]);
	} else if (!strcasecmp(argv[1], "2g")) {
		radio = RADIO_2G;
		hw_rev = parse_platform_hw_rev(radio);

		snprintf(filename_default, sizeof(filename_default), "%s%stxpower_%s_2g.txt",
				TXPOWER_DB_BASE_FILENAME, (hw_rev == CLS_WIFI_HW_MERAK2000) ? "m2k/" : "",
				TXPOWER_DB_DEFAULT_PROJECT);

		if (argc > 2)
			snprintf(filename_project, sizeof(filename_project), "%s%stxpower_%s_2g.txt",
				TXPOWER_DB_BASE_FILENAME, (hw_rev == CLS_WIFI_HW_MERAK2000) ? "m2k/" : "", argv[2]);
	} else {
		usage();
		return -1;
	}

	if (argc > 2) {
		printf("try to parse project txt: %s\n", filename_project);
		fp = fopen(filename_project, "r");
	}

	if (!fp) {
		printf("try to parse default txt: <%s>\n", filename_default);
		fp = fopen(filename_default, "r");

		if (!fp) {
			printf("failed to parse txpower txt\n");
			return -1;
		}
	}

	reset_drv_txpower(radio);
	parse_txpower_db_and_sync2drv(fp, radio);

	if (fp)
		fclose(fp);

	return 0;
}
