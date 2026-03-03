typedef struct {
	char * name;
	int id;
} em_str_to_id_t;

typedef struct {
	char * name;
	int  offset;
	int  array;
	int  type;
} parameters_convert_st;

typedef struct {
	void * object;
	int len;
	int flags;
} object_type_st;

typedef struct {
	char * name;
	int (*func)(int argc, char *argv[]);
} cmd_func_t;

#define T_CHAR 				(T_TYPE_MAX_NUM + 1)
#define T_UINT64_T 			(T_TYPE_MAX_NUM + 2)
#define T_UINT8_T 			(T_TYPE_MAX_NUM + 3)
#define T_UINT16_T 			(T_TYPE_MAX_NUM + 4)
#define T_UINT32_T 			(T_TYPE_MAX_NUM + 5)
#define T_MAC               (T_TYPE_MAX_NUM + 6)

#define F_STRUCT 			1
#define F_STURCT 			1
#define F_ENUM 				2

#define T_UINT64 						 T_UINT64_T
#define T_YT_BOOL_T       				 T_UINT8_T
#define T_YT_VLAN_T      	 			 T_UINT16_T
#define T_YT_TPID_T       				 T_UINT16_T
#define T_YT_FID_T       	 			 T_UINT16_T
#define T_YT_PORT_T      	 			 T_UINT32_T
#define T_YT_MACID_T       				 T_UINT32_T
#define T_YT_RET_T       				 T_UINT32_T
#define T_YT_UNIT_T       				 T_UINT8_T
#define T_YT_TPIDPROFILE_ID_MASK_T       T_UINT8_T
#define T_YT_STP_ID_T       			 T_UINT8_T
#define T_YT_LOCAL_ID_T       			 T_UINT8_T
#define T_YT_PROFILE_ID_T       		 T_UINT8_T
#define T_YT_METERID_T    				 T_UINT8_T
#define T_YT_PRI_T       				 T_UINT8_T
#define T_YT_DSCP_T       				 T_UINT8_T
#define T_YT_CPU_CODE_T       			 T_UINT32_T
#define T_YT_QUEUE_PRI_T       			 T_UINT32_T
#define T_YT_QUEUE_WEIGHT_T       		 T_UINT32_T
#define T_YT_TRANS_TBL_ID_T       		 T_UINT16_T
#define T_YT_INTR_STATUS_T       		 T_UINT32_T
#define T_UINT32       					 T_UINT32_T
#define T_UINT8       					 T_UINT8_T
#define T_YT_COMM_ACT_T                  T_UINT32_T
#define T_YT_COMM_KEY_T                  T_UINT32_T
#define yt_comm_key_t                    uint32_t
#define yt_comm_act_t                    uint32_t
#define T_YT_NIC_RX_CB_F 				 T_UINT64_T
//#define simple_strtoul                 strtoul
//#define simple_strtoull 				 strtoull
//#define printk    					 printf
#define printf						nprintf

#define MAX_PRINT_STR_LEN   (2048 + 512)

int convert_parameters_from_str(void * obj, int type, int * pargc, char ** argv);
void convert_parameters_to_str(void * obj, int type, char * name);

void nprintf(const char *fmt, ...);

#define CONVERT_PARAMFROMSTR(type,val) 		if(argc <= 0) { printf("NO many parameters Exist(post:%d,argc:%d)!\n", post, argc); return -1; }; \
											 post += convert_parameters_from_str(&val, type, &argc, &argv[post])
#define CONVERT_PARAMTOSTR(type,val) 		 convert_parameters_to_str(&val, type, #val)
#define PRINT_HEX_PARAM(val) 				 printf("%20s:%#20x\n", #val , val)

extern int g_debug_switch;
