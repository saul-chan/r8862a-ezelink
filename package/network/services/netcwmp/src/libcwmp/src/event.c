#include <cwmp/types.h>
#include <cwmp/cwmp.h>
#include <cwmp/event.h>
#include <cwmp/log.h>
#include <cwmp/cfg.h>

typedef struct  memorystruct {
    char *data;
    size_t size;
}memorystruct_t;


static inform_event_t inform_event_table[] =
{

    {INFORM_BOOTSTRAP,                 			0,	CWMP_INFORM_EVENT_CODE_0 , "inform:event_bootstrap",      				    "inform:command_bootstrap"},
    {INFORM_BOOT,                      			0,	CWMP_INFORM_EVENT_CODE_1 , "inform:event_boot",                    	"inform:command_boot"},
    {INFORM_PERIODIC,                  			0,	CWMP_INFORM_EVENT_CODE_2 , "inform:event_periodic",                	"inform:command_periodic"},
    {INFORM_SCHEDULED,                 			0,	CWMP_INFORM_EVENT_CODE_3 , "inform:event_scheduled",               	"inform:command_scheduled"},
    {INFORM_VALUECHANGE,               			0,	CWMP_INFORM_EVENT_CODE_4 , "inform:event_valuechange",             	"inform:command_valuechange"},
    {INFORM_KICKED,                    			0,	CWMP_INFORM_EVENT_CODE_5 , "inform:event_kicked",                  	"inform:command_kicked"},
    {INFORM_CONNECTIONREQUEST,         			0,	CWMP_INFORM_EVENT_CODE_6 , "inform:event_connreq",       				    "inform:command_connreq"},
    {INFORM_TRANSFERCOMPLETE,          			0,	CWMP_INFORM_EVENT_CODE_7 , "inform:event_transcomplt",        			  "inform:command_transcomplt"},
    {INFORM_DIAGNOSTICSCOMPLETE,       			0,	CWMP_INFORM_EVENT_CODE_8 , "inform:event_diagcomplt",            		"inform:command_diagcomplt"},
    {INFORM_REQUESTDOWNLOAD,           			0,	CWMP_INFORM_EVENT_CODE_9 , "inform:event_reqdl",         				    "inform:command_reqdl"},
    {INFORM_AUTONOMOUSTRANSFERCOMPLETE,			0,	CWMP_INFORM_EVENT_CODE_10, "inform:event_autotranscomplt",         	"inform:command_autotranscomplt"},
    {INFORM_MREBOOT,                   			0,	CWMP_INFORM_EVENT_CODE_11, "inform:event_mreboot",                 	"inform:command_mreboot"},
    {INFORM_MSCHEDULEINFORM,           			0,	CWMP_INFORM_EVENT_CODE_12, "inform:event_mscheinform",         		  "inform:command_mscheinform"},
    {INFORM_MDOWNLOAD,                 			0,	CWMP_INFORM_EVENT_CODE_13, "inform:event_mdownload",               	"inform:command_mdownload"},
    {INFORM_MUPLOAD,                   			0,	CWMP_INFORM_EVENT_CODE_14, "inform:event_mupload",                 	"inform:command_mupload"},
    {INFORM_ACCOUNTCHANGE,						0,	CWMP_INFORM_EVENT_CODE_15, "inform:event_accountchange",           	"inform:command_accountchange"},
    {INFORM_MVENDORSPECRPC,         			0,	CWMP_INFORM_EVENT_CODE_16, "inform:event_mvendorspecrpc",       		  "inform:command_mvendorspecrpc"},
    {INFORM_XOUIEVENT,                 			0,	CWMP_INFORM_EVENT_CODE_17, "inform:event_xouievent",                	"inform:command_xouievent"},
    {INFORM_MAX,             					0,	"", "",                	                    ""}



};



event_list_t * cwmp_event_list_create(pool_t * pool, int size)
{
    //create event list
    event_list_t * el;
    el = PMALLOC(sizeof(event_list_t));
    if (!el)
    {
        return NULL;
    }
    el->events = PMALLOC(sizeof(event_code_t *) * size);
    el->count = 0;
    el->size = size;
    return el;
}

event_code_t * cwmp_event_code_create(pool_t * pool )
{
    event_code_t * ev;
    ev = PMALLOC(sizeof(event_code_t));
    //ev->event = 0;
    //ev->code = NULL;
    //memset(ev->command_key, 0 , COMMAND_KEY_LEN+1);
    //ev->have_key = 0;
    //ev->policy = 0;
    //ev->ref = 0;
    //ev->fault_code = 0;
    //ev->start = 0;
    //ev->end = 0;
    return ev;
}

int cwmp_event_list_init(pool_t * pool, event_list_t * el)
{
    //create event list
    int i, size;
    size = el->size;
    for(i=0; i<size; i++)
    {
	event_code_t * ec = cwmp_event_code_create(pool);
	el->events[i] = ec;	
    }
   el->count = 0;
    return CWMP_OK;
}


int cwmp_event_global_init(cwmp_t * cwmp)
{
    FILE    *fp = NULL;
    
    if(!cwmp)
    {
        cwmp_log_error("param cwmp is NULL\n");
        return CWMP_ERROR;
    }
    memset(&cwmp->event_global, 0, sizeof(event_global_t));
    if (cwmp->event_filename == NULL)
        cwmp->event_filename = cwmp_conf_pool_get(cwmp->pool, "cwmp:event_filename");

    if(access(cwmp->event_filename, F_OK) == -1)
    {
        cwmp_log_error("%s() access(%s) error\n", __func__, cwmp->event_filename);
        return CWMP_ERROR;
    }

    fp = fopen(cwmp->event_filename, "rb");
    if(!fp)
    {
        cwmp_log_error("could not open event file %s\n", cwmp->event_filename);
        return CWMP_ERROR;
    }

    if(fread(&cwmp->event_global, sizeof(event_global_t), 1, fp) != 1)
    {
        cwmp_log_error("read event global file fail\n");
    }
	
    fclose(fp);
    
    return CWMP_OK;
}

//把事件信息写入文件中
int cwmp_event_file_save(cwmp_t * cwmp)
{
    FILE    *fp = NULL;
    int     ret = CWMP_OK;
    FUNCTION_TRACE(); 
    if(!cwmp)
    {
        return CWMP_ERROR;
    }
    
    if((fp = fopen(cwmp->event_filename, "wb+")) == NULL)
    {
        cwmp_log_error("can't open file: %s.\n", cwmp->event_filename);
        return CWMP_ERROR;
    }

    if(fwrite(&cwmp->event_global, sizeof(event_global_t), 1, fp) != 1)
    {
        cwmp_log_error("can't write event global to %s\n", cwmp->event_filename);
     
        ret = CWMP_ERROR;
    }

    fclose(fp);
    return ret;
}



//设备启动以后，一些事件需要上报
int cwmp_event_init(cwmp_t *cwmp)
{
    if(!cwmp)
    {
        cwmp_log_error("param cwmp is NULL\n");
        return CWMP_ERROR;
    }

    pthread_mutex_init(&cwmp->event_mutex, NULL);

    cwmp->el = cwmp_event_list_create(cwmp->pool, INFORM_MAX);

    cwmp_event_list_init(cwmp->pool, cwmp->el);

    cwmp_event_global_init(cwmp);

    if(cwmp->event_global.event_flag == EVENT_REBOOT_NONE_FLAG || cwmp->event_global.event_flag & EVENT_REBOOT_BOOTSTRAP_FLAG)
    {
        cwmp->event_global.event_flag = EVENT_REBOOT_BOOTSTRAP_FLAG;
        cwmp_event_set_value(cwmp, INFORM_BOOTSTRAP, EVENT_REF_UNTIL_DELIVERED, NULL, 0, 0, 0);
    }    
    else    //reboot
    {
        cwmp_log_info("reboot_flag=%d, key=%s\n", cwmp->event_global.event_flag,
                                                               cwmp->event_global.event_key);

        cwmp_event_set_value(cwmp, INFORM_BOOT, EVENT_REF_UNTIL_REBOOTED, NULL, 0, 0, 0);
        if(cwmp->event_global.event_flag & EVENT_REBOOT_ACS_FLAG)
        {
            cwmp_event_set_value(cwmp, INFORM_MREBOOT, EVENT_REF_UNTIL_DELIVERED, cwmp->event_global.event_key, 0, 0, 0);
        }
    }

    //upgrade firmware
 
    if(cwmp->event_global.event_flag & EVENT_REBOOT_TRANSFERCOMPLETE_FLAG)
    {
        cwmp_event_set_value(cwmp, INFORM_MDOWNLOAD, EVENT_REF_UNTIL_DELIVERED, cwmp->event_global.event_key, 0, 0, 0);
        cwmp_event_set_value(cwmp, INFORM_TRANSFERCOMPLETE, EVENT_REF_UNTIL_DELIVERED, cwmp->event_global.event_key, cwmp->event_global.fault_code, cwmp->event_global.start, cwmp->event_global.end);
    }

    cwmp->event_global.event_flag |= EVENT_REBOOT_UNKNOWN_FLAG;

    return CWMP_OK;
}

/*
//设置某个事件的commandkey
int cwmp_event_set_commandkey(cwmp_t *cwmp, const int event, const char *cmd_key, time_t start)
{
    cwmp_log_debug( "cwmp_event_set_commandkey begin, event=%d\n", event);
    if(event < 0 || event >= INFORM_MAX)
    {
        cwmp_log_error( "event=%d, max=%d\n", event, INFORM_MAX);
        return CWMP_ERROR;
    }

    pthread_mutex_lock(&cwmp->event_mutex);

    int max = cwmp->el->size;
    int i;
    for(i=0; i<max; i++)
    {
	event_code_t * ec = cwmp->el->events[i];
	if(ec->event == event)
	{
		strcpy(ec->command_key, cmd_key);
		ec->start = start;		
		break;
	}
    }


    pthread_mutex_unlock(&cwmp->event_mutex);
    
    cwmp_log_debug( "cwmp_event_set_commandkey end\n");
    return CWMP_OK;
}
*/

int cwmp_event_set_value(cwmp_t *cwmp,  int event,   int value, const char * cmd_key, int fault_code, time_t start, time_t end)
{
    cwmp_log_debug( "cwmp_event_set_value begin, event=%d, value=%d, %s\n", event, value, cmd_key);
    if(event < 0 || event >= INFORM_MAX)
    {
        cwmp_log_error( "event=%d, max=%d\n", event, INFORM_MAX);
        return CWMP_ERROR;
    }
    int count = cwmp->el->count;

     if(count  >= cwmp->el->size)
     {
		return CWMP_ERROR;
     }

    pthread_mutex_lock(&cwmp->event_mutex);

 
    int i;
    int k=count;
	
    for(i=0; i<count; i++)
    {
	event_code_t * ec = cwmp->el->events[i];
	if(ec->event == event)
	{
		k = i;		
		break;
	}
    }

   

	event_code_t * ec = cwmp->el->events[k];
	memset(ec, 0, sizeof(event_code_t));
	ec->event = event;
	ec->ref = value;
	ec->code = inform_event_table[event].code;	
	if(cmd_key != NULL)
	{
		TRstrncpy(ec->command_key, cmd_key,COMMAND_KEY_LEN);
	}

	ec->fault_code = fault_code;
	ec->start = start;
	ec->end = end;
	cwmp->el->count ++;

    	pthread_mutex_unlock(&cwmp->event_mutex);
    
    	return CWMP_OK;

}

/*
//设置某个事件的值
int cwmp_event_set_value1(cwmp_t *cwmp,  int event,   int value)
{
    cwmp_log_debug( "cwmp_event_set_value begin, event=%d, value=%d\n", event, value);
    if(event < 0 || event >= INFORM_MAX)
    {
        cwmp_log_error( "event=%d, max=%d\n", event, INFORM_MAX);
        return CWMP_ERROR;
    }

    pthread_mutex_lock(&cwmp->event_mutex);
    if(cwmp->el->count < cwmp->el->size-1)
    {
        int count = cwmp->el->count;
	event_code_t * ec = cwmp->el->events[count];
	ec->event = event;
        ec->ref = value;
	ec->code = inform_event_table[event].code;	
	cwmp->el->count ++;
    }
    
    pthread_mutex_unlock(&cwmp->event_mutex);
    
    cwmp_log_debug( "cwmp_event_set_value end\n");
    return CWMP_OK;
}

int cwmp_event_inc_set_value(cwmp_t *cwmp, int event,  int step)
{
    cwmp_log_debug( "cwmp_event_inc_set_value begin, event=%d, step=%d\n", event, step);
    if(event < 0 || event >= INFORM_MAX)
    {
        cwmp_log_error( "event=%d, max=%d\n", event, INFORM_MAX);
        return CWMP_ERROR;
    }

    pthread_mutex_lock(&cwmp->event_mutex);
   // cwmp->event_info[index].ref += step;
    int max = cwmp->el->size;
    int i;
    for(i=0; i<max-1; i++)
    {
	event_code_t * ec = cwmp->el->events[i];
	if(ec->event == event)
	{
		ec->ref += step;
		break;
	}
    }
    if(i == max)
    {
    	event_code_t * newec = cwmp->el->events[cwmp->el->count];
	newec->event = event;
	newec->ref += 1;
	newec->code = inform_event_table[event].code;	 
    }
		
	
    pthread_mutex_unlock(&cwmp->event_mutex);
    
    cwmp_log_debug( "cwmp_event_inc_set_value end\n");
    return CWMP_OK;
}

int cwmp_event_dec_set_value(cwmp_t *cwmp, const int event, const unsigned int step)
{
    cwmp_log_debug( "cwmp_event_dec_set_value begin, event=%d, step=%d\n", event, step);
    if(event < 0 || event >= INFORM_MAX)
    {
        cwmp_log_error( "event=%d, max=%d\n", event, INFORM_MAX);
        return CWMP_ERROR;
    }

    pthread_mutex_lock(&cwmp->event_mutex);
    int max = cwmp->el->size;
    int i;
    for(i=0; i<max-1; i++)
    {
	event_code_t * ec = cwmp->el->events[i];
	if(ec->event == event)
	{
		ec->ref -= step;
		if(ec->ref < 0)
		{
			ec->ref = 0;
		}
		break;
	}
    }
    pthread_mutex_unlock(&cwmp->event_mutex);
    
    cwmp_log_debug( "cwmp_event_dec_set_value end\n");
    return CWMP_OK;
}

*/

int cwmp_event_get_index(cwmp_t *cwmp,const char *name)
{
    int index = -1;
    int i;

    if(!name)
    {
        goto finish;
    }
	
    for(i=0; i<INFORM_MAX; i++)
    {
        if(TRstrcmp(cwmp->el->events[i]->code, name) == 0)
        {
            index = i;
            break;
        }
    }

finish:
    return index;
}

size_t cwmp_write_callback(void *ptr, size_t size, size_t nmemb, void *data)
{
    size_t          realsize = size * nmemb;
    memorystruct_t  *mem = (memorystruct_t*)data;

    if(!mem || !ptr)
    {
        cwmp_log_error( "ptr or data is NULL\n");
        return 0;
    }

    mem->data = (char *)realloc(mem->data, mem->size + realsize + 1);
    if (mem->data)
    {
        memcpy(&(mem->data[mem->size]), ptr, realsize);
        mem->size += realsize;
        mem->data[mem->size] = 0;
    }
    
    return realsize;

}


int cwmp_event_clear_active(cwmp_t *cwmp)
{
	int     i;

	assert(cwmp != NULL);
	pthread_mutex_lock(&cwmp->event_mutex);

	event_code_t ** pec = cwmp->el->events;
	int evsize = cwmp->el->count;
	for(i=0; i<evsize; i++)
	{
	    if(pec[i]->ref <= 0)
	    {
	        continue;
	    }

	    if (pec[i]->ref & (EVENT_REF_UNTIL_DELIVERED | EVENT_REF_UNTIL_REBOOTED))
	        continue;

	    pec[i]->ref = 0; //clear
	}

	cwmp->el->count = 0;
	pthread_mutex_unlock(&cwmp->event_mutex);

    //清除Value Change
    /*
    if(notify_flag == 1)
    {
        hash_index_t    *hi = NULL;
        val_change_t    *tmp = NULL;
        char            *key = NULL;
        
        for (hi = hash_first(cwmp->ht_val_change); hi; hi = hash_next(hi))
        {
            hash_this(hi, (const void**)(&key), NULL, (void**)&tmp);
            if(!key || !tmp)
            {
                continue;
            }

            //从value change hash去除
            hash_set(cwmp->ht_val_change, (void*)tmp->name, strlen(tmp->name), NULL);

            //释放内存
            if(tmp->value)
            {
                free_check(tmp->value);
            }
            free_check(tmp);
        }
    }
    */

	return CWMP_OK;
}


/*
 * save active event to event file
 */
int cwmp_event_save_global(cwmp_t *cwmp)
{
    int i;
    int notify_flag = 0;
    event_global_t *evt_global = &(cwmp->event_global);

    assert(cwmp != NULL);
    pthread_mutex_lock(&cwmp->event_mutex);

    event_code_t ** pec = cwmp->el->events;
    int evsize = cwmp->el->count;
    for(i=0; i<evsize; i++) {
        switch(pec[i]->event) {
        case INFORM_BOOTSTRAP:
            if (pec[i]->ref > 0) {
                memset(evt_global, 0, sizeof(cwmp->event_global));
                evt_global->event_flag = EVENT_REBOOT_NONE_FLAG;
                evt_global->event_key[0] = '\0';
            } else
                evt_global->event_flag &= ~EVENT_REBOOT_BOOTSTRAP_FLAG;

            break;

        case INFORM_MREBOOT:
            if (pec[i]->ref > 0) {
                evt_global->event_flag |= EVENT_REBOOT_ACS_FLAG;
                strncpy(evt_global->event_key, pec[i]->command_key, COMMAND_KEY_LEN);
                evt_global->event_key[COMMAND_KEY_LEN] = '\0';
            } else {
                evt_global->event_flag &= ~EVENT_REBOOT_ACS_FLAG;
                evt_global->event_key[0] = '\0';
            }

            break;

        case INFORM_VALUECHANGE:
            notify_flag = 1;

            break;

        case INFORM_TRANSFERCOMPLETE:
            if (pec[i]->ref > 0) {
                evt_global->event_flag |= EVENT_REBOOT_TRANSFERCOMPLETE_FLAG;
                strncpy(evt_global->event_key, pec[i]->command_key, COMMAND_KEY_LEN);
                evt_global->event_key[COMMAND_KEY_LEN] = '\0';
                evt_global->fault_code = pec[i]->fault_code;
                evt_global->start = pec[i]->start;
                evt_global->end = pec[i]->end;
            } else {
                evt_global->event_flag &= ~EVENT_REBOOT_TRANSFERCOMPLETE_FLAG;
                evt_global->event_key[0] = '\0';
                evt_global->fault_code = CWMP_OK;
                evt_global->start = evt_global->end = 0;
            }

            break;
        }
    }

    cwmp_event_file_save(cwmp);

    pthread_mutex_unlock(&cwmp->event_mutex);

    return CWMP_OK;
}


//取得active event以及count
/*
static int get_active_event_list(cwmp_t *cwmp, event_list_t **pevent_list, int *pevt_count)
{
    int             i;
    event_list_t    *evlist = NULL;
    int             count = 0;
    int             ret = CWMP_OK;
    
    if(!cwmp || !pevent_list || !pevt_count)
    {
        cwmp_log_error( "param is NULL\n");
        return CWMP_ERROR;
    }

    pthread_mutex_lock(&cwmp->event_mutex);
    int max = cwmp->event_list->count;
    for(i=0; max; i++)
    {
        if(cwmp->event_info[i].ref > 0)
        {            
            evlist = realloc(evlist, sizeof(event_t)*(count+1));
            if(!evlist)
            {
                ret = CWMP_ERROR;
                cwmp_log_error( "realloc fail\n");
                break;
            }
            memcpy(&evlist[count], &cwmp->event_info[i], sizeof(event_list_t));
            count++;
        }
    }
    pthread_mutex_unlock(&cwmp->event_mutex);

    if(ret == CWMP_OK)
    {
        *pevent_list = evlist;
        *pevt_count = count;
    }
    else
    {
        free(evlist);
    }

    return ret;
}

*/


