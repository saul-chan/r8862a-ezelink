/************************************************************************
 *                                                                      *
 * Netcwmp/Opencwmp Project                                             *
 * A software client for enabling TR-069 in embedded devices (CPE).     *
 *                                                                      *
 * Copyright (C) 2013-2014  netcwmp.netcwmp group                            *
 *                                                                      *
 * This program is free software; you can redistribute it and/or        *
 * modify it under the terms of the GNU General Public License          *
 * as published by the Free Software Foundation; either version 2       *
 * of the License, or (at your option) any later version.               *
 *                                                                      *
 * This program is distributed in the hope that it will be useful,      *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of       *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        *
 * GNU General Public License for more details.                         *
 *                                                                      *
 * You should have received a copy of the GNU Lesser General Public     *
 * License along with this library; if not, write to the                *
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330,          *
 * Boston, MA  02111-1307 USA                                           *
 *                                                                      *
 * Copyright 2013-2014  Mr.x(Mr.x) <netcwmp@gmail.com>          *
 *                                                                      *
 ***********************************************************************/
#include <sys/time.h>
#include "cwmp_module.h"
#include "cwmp_agent.h"
#include <cwmp/session.h>
#include <libcwmp/src/inc/cwmp_private.h>
#include "modules/data_model.h"
#include "modules/data_model_util.h"

#define CWMP_TRUE   1

#define MAX_SESSION_RETRY 3

extern struct timeval inform_time;


enum
{
    CWMP_ST_START = 0, CWMP_ST_INFORM, CWMP_ST_SEND, CWMP_ST_RESEMD, CWMP_ST_RECV, CWMP_ST_ANSLYSE, CWMP_ST_RETRY, CWMP_ST_END, CWMP_ST_EXIT
};





int cwmp_agent_retry_session(cwmp_session_t * session)
{

    int sec = 0;

    srand(time(NULL));
    switch (session->retry_count)
    {
    case 0:
    {
        sec = 5 + rand()%5; //5~10
        break;
    }
    case 1:
    {
        sec = 5 + rand()%10; //5~15
        break;
    }
    case 2:
    {
        sec = 5 + rand()%20; //5~25
        break;
    }
    default:
    {
        sec = 60 + rand()%30; //60~90
        break;
    }
    }

    while (sec>0)
    {
        sleep(1);
        sec--;
    }

    session->retry_count ++;
    return CWMP_OK;
}


int cwmp_agent_create_datetimes(datatime_t *nowtime)
{
    struct tm t;
    time_t tn;
    

    //FUNCTION_TRACE();
    tn = time(NULL);
#ifdef WIN32
    cwmp_log_debug("inform datatime");
    //localtime_s(&t, &tn);
    memset(&t, 0, sizeof(struct tm));
#else
    t = *localtime(&tn);
#endif

    nowtime->year = t.tm_year + 1900;
    nowtime->month = t.tm_mon + 1;
    nowtime->day = t.tm_mday;
    nowtime->hour = t.tm_hour;
    nowtime->min = t.tm_min;
    nowtime->sec = t.tm_sec;

    return CWMP_OK;
}

int cwmp_agent_get_active_event(cwmp_t *cwmp, cwmp_session_t * session,  event_list_t **pevent_list)
{
    event_list_t * el;
    event_code_t * ev;
    int i=0;
    FUNCTION_TRACE();

    el = cwmp_create_event_list(session->env, INFORM_MAX);



    event_code_t ** pec = cwmp->el->events;
   
    int elsize = cwmp->el->count;
    for(i=0; i<elsize; i++)
    {      
        
	if(pec[i]  && pec[i]->ref > 0)
        {
         
            event_code_t * ec = pec[i];
            ev = cwmp_create_event_code(session->env);
            memcpy(ev, ec, sizeof(event_code_t));
#if 0
            ev->event = ec->event;
            ev->code = ec->code;
	
            if (pec[i]->event == INFORM_MREBOOT || pec[i]->event == INFORM_MDOWNLOAD)
            {
                strcpy(ev->command_key , ec->command_key);
            }
#endif
	    
  
            el->events[el->count++] = ev;
            ev = NULL;

        }
    }
#if 0 /* INFORM_BOOT should be explicit set, remove this */
    if (el->count == 0)
    {
        ev = cwmp_create_event_code(session->env);
        ev->event = INFORM_BOOT;
        ev->code = CWMP_INFORM_EVENT_CODE_1;
        el->events[el->count++] = ev;
    }
#endif

    *pevent_list = el;

    return CWMP_OK;


	
}




int cwmp_agent_send_request(cwmp_session_t * session)
{
    FUNCTION_TRACE();
    return cwmp_session_send_request(session);
}

int cwmp_agent_recv_response(cwmp_session_t * session)
{
    return cwmp_session_recv_response(session);
}

void cwmp_agent_start_session(cwmp_t * cwmp)
{
    int rv;
    cwmp_session_t * session;
    int session_close = CWMP_NO;
    xmldoc_t * newdoc;
    FUNCTION_TRACE();
    event_list_t  *evtlist;
    while (TRUE)
    {
        if (cwmp->new_request == CWMP_NO)
        {
            //cwmp_log_debug("No new request from ACS\n");
            sleep(2);
            //cwmp->new_request = CWMP_YES;
            continue;

        }
        cwmp_log_debug("New request from ACS\n");
        cwmp->new_request = CWMP_NO;
        session = cwmp_session_create(cwmp);
        session_close  = CWMP_NO;
        session->timeout = cwmp_conf_get_int("cwmpd:http_timeout");
        //cwmp_session_set_timeout(cwmp_conf_get_int("cwmpd:http_timeout"));
        cwmp_log_debug("session timeout is %d", session->timeout);
		
        cwmp_session_open(session);

        while (!session_close)
        {
            //cwmp_log_debug("session status: %d", session->status);

            switch (session->status)
            {
            case CWMP_ST_START:
                //create a new connection to acs
                cwmp_log_debug("session stutus: New START\n");

                if (cwmp_session_connect(session, cwmp->acs_url) != CWMP_OK)
                {
                    cwmp_log_error("connect to acs: %s failed.\n", cwmp->acs_url);
                    session->status = CWMP_ST_RETRY;
                    cwmp->conn_state = 0;
                }
                else
                {
                    session->status = CWMP_ST_INFORM;
                    cwmp->conn_state = 1;
                }
                break;
            case CWMP_ST_INFORM:
				evtlist = NULL;
				cwmp_log_debug("session stutus: INFORM\n");
				cwmp_agent_get_active_event(cwmp, session,  & evtlist);
				if(evtlist != NULL)
				{
					cwmp_event_clear_active(cwmp);
				}
				cwmp_log_debug("session stutus: INFORM2\n");
                if (cwmp->acs_auth && session->dest->auth.state == AUTH_STATE_INITIAL)
                {
                    cwmp_session_set_auth(session,   cwmp->acs_user  , cwmp->acs_pwd );
                }				

				cwmp_log_debug("session stutus: INFORM3\n");
                newdoc = cwmp_session_create_inform_message(session, evtlist, session->envpool);

                cwmp_write_doc_to_chunk(newdoc, session->writers,  session->envpool);
                session->last_method = CWMP_INFORM_METHOD;
                session->status = CWMP_ST_SEND;


                break;

            case CWMP_ST_SEND:

                cwmp_log_debug("session stutus: SEND");
                cwmp_log_debug("session data request length: %d", cwmp_chunk_length(session->writers));
                session->newdata = CWMP_NO;

                rv = cwmp_agent_send_request(session);
                

                if (rv == CWMP_OK)
                {
					cwmp_log_debug("session data sended OK, rv=%d", rv);
					session->status = CWMP_ST_RECV;
                }
				else
				{
					cwmp_log_debug("session data sended faild! rv=%d", rv);
					session->status = CWMP_ST_EXIT;
					/*
					if (rv == CWMP_COULDNOT_CONNECT)
					{
						session->status = CWMP_ST_RETRY;
					}
					else
                    {
                        session->status = CWMP_ST_EXIT;
                    }
					*/
                }
     

                break;
            case CWMP_ST_RECV:
                cwmp_log_debug("session stutus: RECV");
                cwmp_chunk_clear(session->readers);

                rv = cwmp_agent_recv_response(session);

                if (rv == CWMP_OK)
                {
                    if (cwmp->acs_auth && session->dest->auth.active == CWMP_FALSE &&
                        session->dest->auth.state == AUTH_STATE_RX_CHALLENGE_REQUEST)
                        session->status = CWMP_ST_SEND;
                    else
                        session->status = CWMP_ST_ANSLYSE;
                }
                else
                {
                    session->status = CWMP_ST_END;
                }
                break;

            case CWMP_ST_ANSLYSE:
                cwmp_log_debug("session stutus: ANSLYSE");
                rv = cwmp_agent_analyse_session(session);
                if (rv == CWMP_OK)
                {
                    session->status = CWMP_ST_SEND;
                }
                else
                {
                    session->status = CWMP_ST_END;
                }
                break;
            case CWMP_ST_RETRY:
				cwmp_log_debug("session stutus: RETRY");
                if (cwmp_agent_retry_session(session) == CWMP_TIMEOUT)
                {
					cwmp_log_debug("session retry timeover, go out");
                    session->status = CWMP_ST_EXIT;
                }
                else
                {
                    session->status = CWMP_ST_START;
                }
                break;
            case CWMP_ST_END:
                //close connection of ACS
                cwmp_log_debug("session stutus: END");
				//run task from queue
		
                if (session->newdata == CWMP_YES)
                {
                    session->status = CWMP_ST_SEND;
                }
                else
                {
                    session->status = CWMP_ST_EXIT;
                }
                break;

            case CWMP_ST_EXIT:
                cwmp_log_debug("session stutus: EXIT");
                cwmp_session_close(session);
                if (session->reconnect == CWMP_YES)
                {
                    session->reconnect = CWMP_NO;
                    session->status = CWMP_ST_START;
                    break;
                }
                session_close = CWMP_YES;
                break;


            default:
				cwmp_log_debug("Unknown session stutus");
                break;
            }//end switch



        }//end while(!session_close)

        cwmp_log_debug("session stutus: EXIT");
        cwmp_session_free(session);
        session = NULL;
        if(cwmp->pinform_enable == 1) {
            gettimeofday(&inform_time, NULL);
            cwmp_log_debug("send queue condition signal to update the wait time");
            queue_cond_sig(cwmp->queue);
        }
#if 0
        int newtaskres = cwmp_agent_run_tasks(cwmp);
        if(newtaskres == CWMP_YES)
        {
            cwmp->new_request = CWMP_YES;
        }
#endif

    }//end while(TRUE)

}


int cwmp_agent_analyse_session(cwmp_session_t * session)
{
    pool_t * doctmppool  = NULL;
    char * xmlbuf;
    cwmp_uint32_t len;
    xmldoc_t *  doc;
    char * method;
    xmldoc_t *   newdoc = NULL;
    int rc;

    static char * xml_fault = "<SOAP-ENV:Envelope xmlns:SOAP-ENV=\"http://schemas.xmlsoap.org/soap/envelope/\" xmlns:SOAP-ENC=\"http://schemas.xmlsoap.org/soap/encoding/\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" xmlns:cwmp=\"urn:dslforum-org:cwmp-1-0\" xmlns=\"urn:dslforum-org:cwmp-1-0\"><SOAP-ENV:Body SOAP-ENV:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\"  id=\"_0\"><SOAP-ENV:Fault>Error Message</SOAP-ENV:Fault></SOAP-ENV:Body></SOAP-ENV:Envelope>";

    cwmp_uint32_t msglength = cwmp_chunk_length(session->readers);

    
    if (msglength<= 0)
    {
        session->newdata = CWMP_NO;
        cwmp_log_debug("analyse receive length is 0");
        goto eventcheck;
//      return CWMP_ERROR;
    }
    
    doctmppool = pool_create(POOL_DEFAULT_SIZE);

    xmlbuf = pool_palloc(doctmppool, msglength+32);

    len = sprintf(xmlbuf,"<cwmp>");
    cwmp_chunk_copy(xmlbuf + len, session->readers, msglength);
    strcpy(xmlbuf+len+msglength, "</cwmp>");

    cwmp_log_debug("agent analyse xml: \n%s", xmlbuf);

    doc = XmlParseBuffer(doctmppool, xmlbuf);
    if (!doc)
    {
        cwmp_log_debug("analyse create doc null\n");
        cwmp_chunk_write_string(session->writers, xml_fault, TRstrlen(xml_fault), session->envpool);
        goto finished;

    }

    method = cwmp_get_rpc_method_name(doc);
    cwmp_log_debug("analyse method is: %s\n", method);



 

    cwmp_chunk_clear(session->writers);
    pool_clear(session->envpool);


    if (TRstrcmp(method, CWMP_RPC_GETRPCMETHODS) == 0)
    {
        newdoc = cwmp_session_create_getrpcmethods_response_message(session, doc, doctmppool);
    }
    else if (TRstrcmp(method, CWMP_RPC_INFORMRESPONSE) == 0)
    {
        cwmp_session_handle_informresponse_message(session, doc, doctmppool);
        newdoc = NULL;
    }
    else if (TRstrcmp(method, CWMP_RPC_GETPARAMETERNAMES) == 0)
    {
        newdoc = cwmp_session_create_getparameternames_response_message(session, doc, doctmppool);
    }
    else if (TRstrcmp(method, CWMP_RPC_GETPARAMETERVALUES) == 0)
    {
        newdoc = cwmp_session_create_getparametervalues_response_message(session, doc, doctmppool);
    }
    else if (TRstrcmp(method, CWMP_RPC_SETPARAMETERVALUES) == 0)
    {
        newdoc = cwmp_session_create_setparametervalues_response_message(session, doc, doctmppool);
    }
    else if (TRstrcmp(method, CWMP_RPC_DOWNLOAD) == 0)
    {
        newdoc = cwmp_session_create_download_response_message(session, doc, doctmppool);
    }
    else if (TRstrcmp(method, CWMP_RPC_UPLOAD) == 0)
    {
        newdoc = cwmp_session_create_upload_response_message(session, doc, doctmppool);
    }
    else if (TRstrcmp(method, CWMP_RPC_TRANSFERCOMPLETERESPONSE) == 0)
    {
        cwmp_session_handle_transfercompleteresponse_message(session, doc, doctmppool);
        newdoc = NULL;
    }
    else if (TRstrcmp(method, CWMP_RPC_REBOOT) == 0)
    {
        newdoc = cwmp_session_create_reboot_response_message(session, doc, doctmppool);
    }
    else if (TRstrcmp(method, CWMP_RPC_ADDOBJECT) == 0)
    {
        newdoc = cwmp_session_create_addobject_response_message(session, doc, doctmppool);
    }
    else if (TRstrcmp(method, CWMP_RPC_DELETEOBJECT) == 0)
    {
        newdoc = cwmp_session_create_deleteobject_response_message(session, doc, doctmppool);
    }
    
    else if (TRstrcmp(method, CWMP_RPC_FACTORYRESET) == 0)
    {
        newdoc = cwmp_session_create_factoryreset_response_message(session, doc, doctmppool);
    }
	
    else
    {
    	//check event queue
    	//newdoc = cwmp_session_create_event_response_message(session, doc, doctmppool);

    }


    if(newdoc == NULL)
    {
        cwmp_log_debug("agent analyse newdoc is null. ");
		
eventcheck:
        cwmp_t * cwmp = session->cwmp;
        cwmp_log_debug("agent analyse begin check global event, %d", cwmp->event_global.event_flag);
		
        //check global event for transfercomplete
        if(cwmp->event_global.event_flag & EVENT_REBOOT_TRANSFERCOMPLETE_FLAG)
        {
            /* keep EVENT_REBOOT_TRANSFERCOMPLETE_FLAG to enable retry until it's confirmed,
            according spec A.3.3.2 TransferComplete */
            //cwmp->event_global.event_flag &=  ~EVENT_REBOOT_TRANSFERCOMPLETE_FLAG;
            if(!doctmppool)
            {
                doctmppool = pool_create(POOL_DEFAULT_SIZE);
            }
            event_code_t ec;
            ec.event = INFORM_TRANSFERCOMPLETE;
            TRstrncpy(ec.command_key, cwmp->event_global.event_key, COMMAND_KEY_LEN);
            ec.fault_code = cwmp->event_global.fault_code;
            ec.start = cwmp->event_global.start;
            ec.end = cwmp->event_global.end;
            newdoc = cwmp_session_create_transfercomplete_message(session, &ec, doctmppool);
        }
    }


    cwmp_log_debug("newdoc %p, msglength: %d", newdoc, msglength );
    if((newdoc != NULL) || (newdoc == NULL && msglength != 0)) // || (newdoc == NULL && msglength == 0 && session->retry_count < 2))
    {
        session->newdata = CWMP_YES;
        cwmp_write_doc_to_chunk(newdoc, session->writers,  session->envpool);
        rc = CWMP_OK;
    }
    else
    {  	
        rc = CWMP_ERROR;
    }
	
finished:
	if(doctmppool  != NULL)
	{
	    pool_destroy(doctmppool);
	}
    return rc;
}



static void print_param(parameter_node_t * param, int level)
{
  if(!param) return; 
 
  parameter_node_t * child;
  char fmt[64];
  //cwmp_log_debug("name: %s, type: %s, level: %d\n", param->name, cwmp_get_type_string(param->type), level);
  
  

  sprintf(fmt, "|%%-%ds%%s,  get:%%p set:%%p refresh:%%p", level*4);
    
  cwmp_log_debug(fmt, "----", param->name, param->get, param->set, param->refresh);


  child = param->child;

  if(!child)
	return;
  print_param(child, level+1);

  parameter_node_t * next = child->next_sibling;

  while(next)
 {
    print_param(next, level+1);
    next = next->next_sibling;
 }

	
}


void cwmp_agent_session(cwmp_t * cwmp)
{
    char name[1024] = {0};
    char value[1024]= {0};
    char local_ip[32];

    char envstr[INI_BUFFERSIZE] = {0};
    char encstr[INI_BUFFERSIZE] = {0};
	char *rootName = cwmp->root->name;

//    envstr = "SOAP-ENV"; //cwmp_conf_get("cwmp:soap_env");
//    encstr = "SOAP-ENC"; // cwmp_conf_get("cwmp:soap_enc");
    cwmp_conf_get("cwmp:soap_env", envstr);
    cwmp_conf_get("cwmp:soap_enc", encstr);

    cwmp_set_envelope_ns(envstr, encstr);

    if (cwmp_session_get_localip(local_ip) == -1)
    {
        cwmp_log_error("get local ip error. exited.\n");
        exit(-1);
    }

    print_param(cwmp->root, 0);

    /* Replace InternetGatewayDevice Module with cwmp->root->name to support both TR-098 and TR-181, data model is configured in device.xml */
    CWMP_SPRINTF_PARAMETER_NAME(name, 3, rootName, ManagementServerModule, URLModule);
    cwmp_data_set_parameter_value(cwmp, cwmp->root, name, cwmp->acs_url, TRstrlen(cwmp->acs_url), cwmp->pool);

    CWMP_SPRINTF_PARAMETER_NAME(name, 3, rootName, ManagementServerModule, ConnectionRequestURLModule);
    TRsnprintf(value, 1024, "http://%s:%d", local_ip, cwmp->httpd_port);
    cwmp_data_set_parameter_value(cwmp, cwmp->root, name, value, TRstrlen(value), cwmp->pool);

    CWMP_SPRINTF_PARAMETER_NAME(name, 3, rootName, ManagementServerModule, AliasBasedAddressing);
    cwmp_data_set_parameter_value(cwmp, cwmp->root, name, AliasBasedAddressingValue, TRstrlen(AliasBasedAddressingValue), cwmp->pool);

    CWMP_SPRINTF_PARAMETER_NAME(name, 3, rootName, ManagementServerModule, PeriodicInformEnableModule);
    TRsnprintf(value, 1024, "%d", cwmp->pinform_enable);
    cwmp_data_set_parameter_value(cwmp, cwmp->root, name, value, 1, cwmp->pool);

    CWMP_SPRINTF_PARAMETER_NAME(name, 3, rootName, ManagementServerModule, PeriodicInformIntervalModule);
    TRsnprintf(value, 1024, "%d", cwmp->pinform_interval);
    cwmp_data_set_parameter_value(cwmp, cwmp->root, name, value, 1, cwmp->pool);

    CWMP_SPRINTF_PARAMETER_NAME(name, 3, rootName, DeviceInfoModule, ManufacturerModule);
    cwmp_data_set_parameter_value(cwmp, cwmp->root, name, cwmp->cpe_mf, TRstrlen(cwmp->cpe_mf), cwmp->pool);

    CWMP_SPRINTF_PARAMETER_NAME(name, 3, rootName, DeviceInfoModule, ManufacturerOUIModule);
    cwmp_data_set_parameter_value(cwmp, cwmp->root, name, cwmp->cpe_oui, TRstrlen(cwmp->cpe_oui), cwmp->pool);

    CWMP_SPRINTF_PARAMETER_NAME(name, 3, rootName, DeviceInfoModule, ProductClassModule);
    cwmp_data_set_parameter_value(cwmp, cwmp->root, name, cwmp->cpe_pc, TRstrlen(cwmp->cpe_pc), cwmp->pool);

    CWMP_SPRINTF_PARAMETER_NAME(name, 3, rootName, DeviceInfoModule, SerialNumberModule);
    cwmp_data_set_parameter_value(cwmp, cwmp->root, name, cwmp->cpe_sn, TRstrlen(cwmp->cpe_sn), cwmp->pool);

    if (strncmp(rootName, ROOT_DATA_MODEL_TR181, strlen(ROOT_DATA_MODEL_TR181)) == 0) {
        CWMP_SPRINTF_PARAMETER_NAME(name, 2, rootName, DMTR181Param_DeviceRootDMVer);
        cwmp_data_set_parameter_value(cwmp, cwmp->root, name, DMTR181_ROOTDMVERSION, TRstrlen(DMTR181_ROOTDMVERSION), cwmp->pool);
    }
    softversion_update(cwmp);
    cwmp_agent_start_session(cwmp);
}


/*
int cwmp_agent_download_file(download_arg_t * dlarg)
{
    int faultcode = 0;
    char url[255];
    if (TRstrncasecmp(dlarg->url,  "ftp://", 6) == 0)
    {
        if (dlarg->username != NULL && strlen(dlarg->username) != 0)
        {
            TRsnprintf(url, 255, "ftp://%s:%s@%s", dlarg->username, dlarg->password, dlarg->url+6);
        }
        else
        {
            TRstrncpy(url, dlarg->url, 255);
        }
    }
    else
    {
        TRstrncpy(url, dlarg->url, 255);
    }

    fetchIO * downfile =   fetchGetURL(url, "");
    if (!downfile)
    {
        cwmp_log_debug("download file fail:  %s", url);
        faultcode = 9001;
    }
    else
    {
        char readbuf[1024];
        int readlen;
        char targetfile[64];
        if (dlarg->targetname != NULL && strlen(dlarg->targetname) != 0)
        {
            TRsnprintf(targetfile, 64, "/tmp/%s", dlarg->targetname);
        }
        else
        {
            TRsnprintf(targetfile, 64, "/tmp/%d.file", time(NULL));
        }
        FILE * df = fopen(targetfile, "wb+");
        while (df != NULL)
        {
            readlen = fetchIO_read(downfile, readbuf, 1023);
            if (readlen <= 0)
            {
                cwmp_log_debug("fetch io read zero. %s", readlen);
                break;
            }
            readbuf[readlen] = 0;
            fwrite(readbuf, readlen, 1, df);
        }

        if (df)
        {
            fclose(df);
        }
        else
        {
            faultcode = 9001;
        }

        cwmp_log_debug("download file ok:  %s, %s", url, targetfile);


        fetchIO_close(downfile);
    }


    return faultcode;

}



int cwmp_agent_upload_file(upload_arg_t * ularg)
{
    int faultcode = 0;
    char url[255];

    if (TRstrncasecmp(ularg->url,  "ftp://", 6) == 0)
    {
        if (ularg->username != NULL && strlen(ularg->username) != 0)
        {
            TRsnprintf(url, 255, "ftp://%s:%s@%s", ularg->username, ularg->password, ularg->url+6);
        }
        else
        {
            TRstrncpy(url, ularg->url, 255);
        }
    }
    else
    {
        TRstrncpy(url, ularg->url, 255);
    }

    fetchIO * uploadfile =   fetchPutURL(url, "");
    if (!uploadfile)
    {
        cwmp_log_debug("upload file fail:  %s", url);
        faultcode = 9001;
    }
    else
    {
        char readbuf[1024];
        int readlen;
	char targetfile[64];
	FILE * uf;
	int rc;
	if(strcmp(ularg->filetype, "1 Vendor Configuration File") == 0)
	{
		uf = fopen("/tmp/mysystem.cfg", "rb");		
	}
	else if(strcmp(ularg->filetype, "2 Vendor Log File") == 0)
	{
		uf = fopen("/tmp/mysystem.log", "rb");	
	}
	else
	{
		uf = fopen("/tmp/mysystem.log", "rb");	
	}

		
        while (uf != NULL)
        {
            readlen = fread(readbuf, 1024, 1,  uf);
            if (readlen <= 0)
            {
                cwmp_log_debug("fetch io read zero. %s", readlen);
			
                break;
            }
            readbuf[readlen] = 0;

	   rc = fetchIO_write(uploadfile, readbuf, readlen);
	   if(rc <= 0)
	   {
		faultcode = 9001;
		break;
	   }
		
        }

	if(uf)
	{
		fclose(uf);
	}
	else
	{
		faultcode = 9001;
	}
	


        cwmp_log_debug("upload file finished:  %s, file:%s", url, targetfile);


        fetchIO_close(uploadfile);
    }


    return faultcode;

}
*/


#define HTTP_CODE_OK	200
int cwmp_agent_download_file(download_arg_t * dlarg)
{
    int faultcode = 0;
    char * fromurl = dlarg->url;
    char * tofile = "/tmp/download.img";

    FUNCTION_TRACE();

    if(dlarg->url && TRstrncasecmp("ftp://", dlarg->url, 6) == 0)
    {
	    return 9001;
    }

    faultcode = http_receive_file(fromurl, tofile);

    if(faultcode != HTTP_CODE_OK)
    {
	     faultcode = 9001;
    }

    return faultcode;
}



int cwmp_agent_upload_file(upload_arg_t * ularg)
{
    int faultcode = 0;
    FUNCTION_TRACE();
    char * fromfile;

	if(strcpy(ularg->filetype, "1 Vendor Configuration File") == 0)
	{
		fromfile = "/tmp/mysystem.cfg";
	}
	else if(strcpy(ularg->filetype, "2 Vendor Log File") == 0)
	{
		fromfile = "/tmp/mysystem.log";
	}
	else
	{
		fromfile = "/tmp/mysystem.cfg";
	}
	
    faultcode = http_send_file(fromfile, ularg->url);

    if(faultcode != CWMP_OK)
    {
	faultcode = 9001;
    }
   


    return faultcode;
}



int cwmp_agent_run_tasks(cwmp_t * cwmp)
{
	void * data;
	int tasktype = 0;;
	int ok = CWMP_NO;
	struct timeval now;

	FUNCTION_TRACE();
	
	while(1)
	{
		if(cwmp->pinform_enable == 0 || inform_time.tv_sec == 0 || cwmp->pinform_interval == 0){
		    queue_wait(cwmp->queue, 0);
		}else{
		    queue_wait(cwmp->queue, cwmp->pinform_interval);
		}
		gettimeofday(&now, NULL);
		if((cwmp->pinform_enable == 1) && (inform_time.tv_sec != 0) && (now.tv_sec >= (inform_time.tv_sec + cwmp->pinform_interval))){
			cwmp_log_debug("Push periodic inform to queue");
			queue_push(cwmp->queue, NULL, TASK_PERIODIC_INFORM);
		}
		tasktype = queue_pop(cwmp->queue, &data);

		while(tasktype != -1)
		{
			//ok = CWMP_YES;
			switch(tasktype)
			{
			case TASK_DOWNLOAD_TAG:
			{
				download_arg_t * dlarg = (download_arg_t*)data;
				//begin download file
				time_t starttime = time(NULL);
				time_t endtime = 0;
				int faultcode = 0, need_reboot = 0;

				faultcode = cwmp_agent_download_file(dlarg);
				if (faultcode == HTTP_CODE_OK && dlarg && dlarg->filetype) {
					switch (atoi(dlarg->filetype)) {
					case 1: /* 1 Firmware Upgrade Image, "Download arguments" of TR-069 Protocol */
						cwmp_log_debug("Validating Firmware ...");
						faultcode = system("sysupgrade -T /tmp/download.img");
						if (faultcode != 0) {
							cwmp_log_debug("%s() FW upgrade FAIL! Error code=%d.", __func__, faultcode);
							faultcode = FAULT_CODE_9001;
						}
						else {
							cwmp_log_debug("%s() FW upgrade successful! Reboot to apply", __func__);
							faultcode = CWMP_OK;

							/* need reboot to apply new FW, then send 'TransferComplete message' */
							need_reboot = 1;
						}
						break;

					default:
						cwmp_log_error("Not supported file type: %s", dlarg->filetype);
						faultcode = FAULT_CODE_9000;
					}
				}
				else
					faultcode = FAULT_CODE_9001;

				if (faultcode == FAULT_CODE_OK)
					endtime = time(NULL);
				else
					ok = CWMP_YES;
				cwmp_event_set_value(cwmp, INFORM_TRANSFERCOMPLETE, EVENT_REF_UNTIL_DELIVERED, dlarg->cmdkey, faultcode, starttime, endtime);
				cwmp_event_save_global(cwmp);
				FREE(dlarg);

				if (need_reboot) {
					cwmp_event_clear_active(cwmp);
					system("sysupgrade -c /tmp/download.img");
				}
			}
			break;

			case TASK_UPLOAD_TAG:
			{
				upload_arg_t * ularg = (upload_arg_t*)data;
				//begin download file
				time_t starttime = time(NULL);
				int faultcode = 0;
		
				faultcode = cwmp_agent_upload_file(ularg);
					
				time_t endtime = time(NULL);
				cwmp_event_set_value(cwmp, INFORM_TRANSFERCOMPLETE, EVENT_REF_UNTIL_DELIVERED,ularg->cmdkey, faultcode, starttime, endtime);
				FREE(ularg);
			}
			break;

			case TASK_REBOOT_TAG:
			{
				char *key = (char *)data;
				//begin reboot system
				cwmp_log_debug("reboot ...");
				cwmp_event_set_value(cwmp, INFORM_MREBOOT, EVENT_REF_UNTIL_REBOOTED, key, 0, 0, 0);
				FREE(key);
				cwmp_event_save_global(cwmp);
				cwmp_event_clear_active(cwmp);
				system("reboot");
			}
				break;

			case TASK_FACTORYRESET_TAG:
			{
				//begin factory reset system
				cwmp_log_debug("factory reset ...");

				cwmp_event_clear_active(cwmp);
				cwmp_event_set_value(cwmp, INFORM_BOOTSTRAP, EVENT_REF_UNTIL_DELIVERED, NULL, 0, 0, 0);
				cwmp_event_save_global(cwmp);
				system("firstboot -y;reboot");
			}
				break;

			case TASK_PERIODIC_INFORM:
			{
				cwmp_log_debug("do periodic inform task");
				cwmp_event_set_value(cwmp, INFORM_PERIODIC, EVENT_REF_UNTIL_DELIVERED, NULL, 0, 0, 0);
				cwmp->new_request = CWMP_YES;
				gettimeofday(&inform_time, NULL);
			}
				break;
			default:
				break;

			}
			tasktype = queue_pop(cwmp->queue, &data);
		}
	}

	return ok;
}


