#include <cwmp/cwmp.h>
#include "cwmp/stun.h"
#include "clsapi_wifi.h"
#include "clsapi_base.h"

int cpe_get_localip(const char * eth_name, char *hostip)
{
    register int fd,intrface,retn=0;
    struct ifreq buf[32];
    struct ifconf ifc;
    char domain_host[100] = {0};
    char local_ip_addr[20] = {0};

    //Get Domain Name --------------------------------------------------
    strcpy(local_ip_addr, "127.0.0.1");
    if (!hostip)
        return -1;
    if (getdomainname(&domain_host[0], 100) != 0)
    {
        return -1;
    }
    //------------------------------------------------------------------
    //Get IP Address & Mac Address ----------------------------------------
    if ((fd=socket(AF_INET,SOCK_DGRAM,0))>=0)
    {
        ifc.ifc_len=sizeof buf;
        ifc.ifc_buf=(caddr_t)buf;
        if (!ioctl(fd,SIOCGIFCONF,(char*)&ifc))
        {
            intrface=ifc.ifc_len/sizeof(struct ifreq);
            while (intrface-->0)
            {
                if (!(ioctl(fd,SIOCGIFFLAGS,(char*)&buf[intrface])))
                {
                    if (buf[intrface].ifr_flags&IFF_PROMISC)
                    {
                        retn++;
                    }
                }
                //Get IP Address
                if (!(ioctl(fd,SIOCGIFADDR,(char*)&buf[intrface])))
                {
		    if(strcmp(eth_name, buf[intrface].ifr_name) == 0)
		    {
                    sprintf(local_ip_addr, "%s", inet_ntoa(((struct sockaddr_in*)(&buf[intrface].ifr_addr))->sin_addr));
		
		    break;
		    }
                }
                //Get Hardware Address

            }//While
        }
    }
    if ( fd > 0 )
    {
        close(fd);
    }

    strcpy(hostip, local_ip_addr);

    return CWMP_OK;
}

//InternetGatewayDevice.ManagementServer.EnableCWMP
int cpe_get_igd_ms_enable(cwmp_t * cwmp, const char * name, char ** value, pool_t * pool)
{
    *value = cwmp_conf_pool_get(pool, "cwmp:enable");
    return FAULT_CODE_OK;
}

int cpe_set_igd_ms_enable(cwmp_t * cwmp, const char * name, const char * value, int length, callback_register_func_t callback_reg)
{
    if(clsapi_base_set_apply_conf_param(UCI_CFG_NETCWMP, UCI_SECTION_CWMP, UCI_PARA_ENABLE, value))
		return FAULT_CODE_9002;
    cwmp_conf_set("cwmp:enable", value);
    return FAULT_CODE_OK;
}

//InternetGatewayDevice.ManagementServer.Username
int cpe_get_igd_ms_username(cwmp_t * cwmp, const char * name, char ** value, pool_t * pool)
{    
    *value = cwmp_conf_pool_get(pool, "cwmp:acs_username");
    return FAULT_CODE_OK;
}

//InternetGatewayDevice.ManagementServer.Username
int cpe_set_igd_ms_username(cwmp_t * cwmp, const char * name, const char * value, int length, callback_register_func_t callback_reg)
{
    if(clsapi_base_set_apply_conf_param(UCI_CFG_NETCWMP, UCI_SECTION_CWMP, UCI_PARA_ACS_USERNAME, value))
		return FAULT_CODE_9002;
    cwmp_conf_set("cwmp:acs_username", value);
    return FAULT_CODE_OK;
}

//InternetGatewayDevice.ManagementServer.Password
int cpe_get_igd_ms_password(cwmp_t * cwmp, const char * name, char ** value, pool_t * pool)
{
    *value = PSTRDUP("");
    return FAULT_CODE_OK;
}

int cpe_set_igd_ms_password(cwmp_t * cwmp, const char * name, const char * value, int length, callback_register_func_t callback_reg)
{
    if(clsapi_base_set_apply_conf_param(UCI_CFG_NETCWMP, UCI_SECTION_CWMP, UCI_PARA_ACS_PASSWORD, value))
		return FAULT_CODE_9002;
    cwmp_conf_set("cwmp:acs_password", value);
    return FAULT_CODE_OK;
}

//InternetGatewayDevice.ManagementServer.URL
int cpe_get_igd_ms_url(cwmp_t * cwmp, const char * name, char ** value, pool_t * pool)
{    
    *value = cwmp_conf_pool_get(pool, "cwmp:acs_url");
    return FAULT_CODE_OK;
}

//InternetGatewayDevice.ManagementServer.URL
int cpe_set_igd_ms_url(cwmp_t * cwmp, const char * name, const char * value, int length, callback_register_func_t callback_reg)
{
    if(clsapi_base_set_apply_conf_param(UCI_CFG_NETCWMP, UCI_SECTION_CWMP, UCI_PARA_ACS_URL, value))
		return FAULT_CODE_9002;
    cwmp_conf_set("cwmp:acs_url", value);
    return FAULT_CODE_OK;
}

//InternetGatewayDevice.ManagementServer.ConnectionRequestURL
int cpe_get_igd_ms_connectionrequesturl(cwmp_t * cwmp, const char * name, char ** value, pool_t * pool)
{
    char buf[256]={0};
    char interface[CWMP_STR_LEN_1025] = {0};
    char local_ip[32]={0};

    if(clsapi_base_get_conf_param(UCI_CFG_NETWORK, UCI_SECTION_WAN, UCI_PARA_DEVICE, interface))
		return FAULT_CODE_9002;
    cpe_get_localip(interface, local_ip);
    int port = cwmp_conf_get_int("cwmpd:httpd_port");
    snprintf(buf, 256, "http://%s:%d", local_ip, port);
    *value = PSTRDUP(buf);
    return FAULT_CODE_OK;
}

//InternetGatewayDevice.ManagementServer.ConnectionRequestUsername
int cpe_get_igd_ms_connectionrequestusername(cwmp_t * cwmp, const char * name, char ** value, pool_t * pool)
{
    *value = cwmp_conf_pool_get(pool, "cwmp:cpe_username");
    return FAULT_CODE_OK;
}
int cpe_set_igd_ms_connectionrequestusername(cwmp_t * cwmp, const char * name, const char * value, int length, callback_register_func_t callback_reg)
{
    if(clsapi_base_set_apply_conf_param(UCI_CFG_NETCWMP, UCI_SECTION_CWMP, UCI_PARA_CPE_USERNAME, value))
		return FAULT_CODE_9002;
    cwmp_conf_set("cwmp:cpe_username", value);
    return FAULT_CODE_OK;
}

//InternetGatewayDevice.ManagementServer.ConnectionRequestPassword
int cpe_get_igd_ms_connectionrequestpassword(cwmp_t * cwmp, const char * name, char ** value, pool_t * pool)
{
    *value = PSTRDUP("");
    return FAULT_CODE_OK;
}
int cpe_set_igd_ms_connectionrequestpassword(cwmp_t * cwmp, const char * name, const char * value, int length, callback_register_func_t callback_reg)
{
    if(clsapi_base_set_apply_conf_param(UCI_CFG_NETCWMP, UCI_SECTION_CWMP, UCI_PARA_CPE_PASSWORD, value))
		return FAULT_CODE_9002;
    cwmp_conf_set("cwmp:cpe_password", value);
    return FAULT_CODE_OK;
}

//Device.ManagementServer.UDPConnectionRequestAddress
int cpe_get_igd_ms_udp_conn_req_addr(cwmp_t * cwmp, const char * name, char ** value, pool_t * pool)
{
	char buf[256] = {0};
	char host[32] = {0};
	int udp_port = -1;
	char interface[CWMP_STR_LEN_1025] = {0};
	FUNCTION_TRACE();

	if (cwmp->stun.stun_conf.enable) {
		if (cwmp->stun.nat_mapped_addr.sin_port) {
			strncpy(host, inet_ntoa(cwmp->stun.nat_mapped_addr.sin_addr), sizeof(host) - 1);
			host[sizeof(host) - 1] = '\0';
			udp_port = ntohs(cwmp->stun.nat_mapped_addr.sin_port);
		}
	}
	else {
		if(clsapi_base_get_conf_param(UCI_CFG_NETWORK, UCI_SECTION_WAN, UCI_PARA_DEVICE, interface))
			return FAULT_CODE_9002;
		if (cpe_get_localip(interface, host) < 0)
			return FAULT_CODE_9002;
		udp_port = cwmp_conf_get_int("cwmpd:udp_port");
	}

	if (strlen(host) > 0)
		snprintf(buf, 256, "%s:%d", host, udp_port);
	*value = PSTRDUP(buf);

	return FAULT_CODE_OK;
}

//Device.ManagementServer.NATDetected
int cpe_get_igd_ms_nat_detected(cwmp_t * cwmp, const char * name, char ** value, pool_t * pool)
{
	char buf[256] = {0};
	int nat_detected = -1;

	if (cwmp->stun.stun_conf.enable)
		nat_detected = cwmp->stun.nat_detected;
	else
		nat_detected = 0;

	snprintf(buf, sizeof(buf), "%d", nat_detected);
	*value = PSTRDUP(buf);

	return FAULT_CODE_OK;
}
//Device.ManagementServer.PeriodicInformEnable
int cpe_get_igd_ms_perio_info_enable(cwmp_t * cwmp, const char * name, char ** value, pool_t * pool)
{
    char buf[256] = {0};

    snprintf(buf, sizeof(buf), "%d", cwmp_conf_get_int("cwmp:pinform_enable"));
    *value = PSTRDUP(buf);
    return FAULT_CODE_OK;
}
int cpe_set_igd_ms_perio_info_enable(cwmp_t * cwmp, const char * name, const char * value, int length, callback_register_func_t callback_reg)
{
    cwmp_conf_set("cwmp:pinform_enable", value);
    cwmp->pinform_enable = cwmp_conf_get_int("cwmp:pinform_enable");

    if(clsapi_base_set_apply_conf_param(UCI_CFG_NETCWMP, UCI_SECTION_CWMP, UCI_PARA_PINFORM_ENABLE, value))
		return FAULT_CODE_9002;
    return FAULT_CODE_OK;
}

//Device.ManagementServer.PeriodicInformInterval
int cpe_get_igd_ms_perio_info_intval(cwmp_t * cwmp, const char * name, char ** value, pool_t * pool)
{
    char buf[256] = {0};

    snprintf(buf, sizeof(buf), "%d", cwmp_conf_get_int("cwmp:pinform_interval"));
    *value = PSTRDUP(buf);
    return FAULT_CODE_OK;
}

int cpe_set_igd_ms_perio_info_intval(cwmp_t * cwmp, const char * name, const char * value, int length, callback_register_func_t callback_reg)
{
    cwmp_conf_set("cwmp:pinform_interval", value);
    cwmp->pinform_interval = cwmp_conf_get_int("cwmp:pinform_interval");

    if(clsapi_base_set_apply_conf_param(UCI_CFG_NETCWMP, UCI_SECTION_CWMP, UCI_PARA_PINFORM_INTERVAL, value))
		return FAULT_CODE_9002;
    return FAULT_CODE_OK;
}

//Device.ManagementServer.STUNEnable
int cpe_get_igd_ms_stun_enable(cwmp_t * cwmp, const char * name, char ** value, pool_t * pool)
{
	char buf[256] = {0};

	snprintf(buf, sizeof(buf), "%d", cwmp_conf_get_int("stun:enable"));
	*value = PSTRDUP(buf);

	return FAULT_CODE_OK;
}
int cpe_set_igd_ms_stun_enable(cwmp_t * cwmp, const char * name, const char * value, int length, callback_register_func_t callback_reg)
{
	if( cwmp_conf_get_int("stun:enable") == atoi(value)){
		cwmp_log_info("Value is same as before for stun:enable\n");
		return FAULT_CODE_OK;
	}
	cwmp_conf_set("stun:enable", value);
	cwmp->stun.stun_conf.enable = cwmp_conf_get_int("stun:enable");
	cwmp->stun.state = CWMP_STUN_STATE_INIT;

	if(clsapi_base_set_apply_conf_param(UCI_CFG_NETCWMP, UCI_SECTION_STUN, UCI_PARA_ENABLE, value))
		return FAULT_CODE_9002;
	return FAULT_CODE_OK;
}

//Device.ManagementServer.STUNServerAddress
int cpe_get_igd_ms_stun_srv_addr(cwmp_t * cwmp, const char * name, char ** value, pool_t * pool)
{
	*value = cwmp_conf_pool_get(pool, "stun:server_addr");

	return FAULT_CODE_OK;
}
int cpe_set_igd_ms_stun_srv_addr(cwmp_t * cwmp, const char * name, const char * value, int length, callback_register_func_t callback_reg)
{
	pool_t * pool = cwmp->pool;

	if(strcmp(cwmp_conf_pool_get(pool, "stun:server_addr"), value) == 0){
		cwmp_log_info("Value is same as before for stun:server_addr\n");
		return FAULT_CODE_OK;
	}
	cwmp_conf_set("stun:server_addr", value);
	cwmp->stun.stun_conf.server_addr = cwmp_conf_pool_get(pool, "stun:server_addr");
	cwmp->stun.state = CWMP_STUN_STATE_INIT;

	if(clsapi_base_set_apply_conf_param(UCI_CFG_NETCWMP, UCI_SECTION_STUN, UCI_PARA_SERVER_ADDR, value))
		return FAULT_CODE_9002;
	return FAULT_CODE_OK;
}

//Device.ManagementServer.STUNServerPort
int cpe_get_igd_ms_stun_srv_port(cwmp_t * cwmp, const char * name, char ** value, pool_t * pool)
{
	char buf[256] = {0};

	snprintf(buf, sizeof(buf), "%d", cwmp_conf_get_int("stun:server_port"));
	*value = PSTRDUP(buf);

	return FAULT_CODE_OK;
}
int cpe_set_igd_ms_stun_srv_port(cwmp_t * cwmp, const char * name, const char * value, int length, callback_register_func_t callback_reg)
{
	if(cwmp_conf_get_int("stun:server_port") == atoi(value)){
		cwmp_log_info("Value is same as before for stun:server_port\n");
		return FAULT_CODE_OK;
	}
	cwmp_conf_set("stun:server_port", value);
	cwmp->stun.stun_conf.server_port = cwmp_conf_get_int("stun:server_port");
	cwmp->stun.state = CWMP_STUN_STATE_INIT;

	if(clsapi_base_set_apply_conf_param(UCI_CFG_NETCWMP, UCI_SECTION_STUN, UCI_PARA_SERVER_PORT, value))
		return FAULT_CODE_9002;
	return FAULT_CODE_OK;
}

//Device.ManagementServer.STUNUsername
int cpe_get_igd_ms_stun_username(cwmp_t * cwmp, const char * name, char ** value, pool_t * pool)
{
	*value = cwmp_conf_pool_get(pool, "stun:username");
	return FAULT_CODE_OK;
}
int cpe_set_igd_ms_stun_username(cwmp_t * cwmp, const char * name, const char * value, int length, callback_register_func_t callback_reg)
{
	pool_t * pool = cwmp->pool;

	if(strcmp(cwmp_conf_pool_get(pool, "stun:username"), value) == 0){
		cwmp_log_info("Value is same as before for stun:username\n");
		return FAULT_CODE_OK;
	}
	cwmp_conf_set("stun:username", value);
	cwmp->stun.stun_conf.password = cwmp_conf_pool_get(pool, "stun:username");
	cwmp->stun.state = CWMP_STUN_STATE_INIT;

	if(clsapi_base_set_apply_conf_param(UCI_CFG_NETCWMP, UCI_SECTION_STUN, UCI_PARA_USERNAME, value))
		return FAULT_CODE_9002;
	return FAULT_CODE_OK;
}

//Device.ManagementServer.STUNPassword
int cpe_get_igd_ms_stun_password(cwmp_t * cwmp, const char * name, char ** value, pool_t * pool)
{
	*value = PSTRDUP("");
	return FAULT_CODE_OK;
}
int cpe_set_igd_ms_stun_password(cwmp_t * cwmp, const char * name, const char * value, int length, callback_register_func_t callback_reg)
{
	pool_t * pool = cwmp->pool;

	if(strcmp(cwmp_conf_pool_get(pool, "stun:password"), value) == 0){
		cwmp_log_info("Value is same as before for stun:password\n");
		return FAULT_CODE_OK;
	}
	cwmp_conf_set("stun:password", value);
	cwmp->stun.stun_conf.password = cwmp_conf_pool_get(pool, "stun:password");
	cwmp->stun.state = CWMP_STUN_STATE_INIT;

	if(clsapi_base_set_apply_conf_param(UCI_CFG_NETCWMP, UCI_SECTION_STUN, UCI_PARA_PASSWORD, value))
		return FAULT_CODE_9002;
	return FAULT_CODE_OK;
}

//Device.ManagementServer.STUNMaximumKeepAlivePeriod
int cpe_get_igd_ms_stun_max_keepalive_perio(cwmp_t * cwmp, const char * name, char ** value, pool_t * pool)
{
	char buf[256] = {0};

	snprintf(buf, sizeof(buf), "%d", cwmp_conf_get_int("stun:max_keepalive_period"));
	*value = PSTRDUP(buf);

	return FAULT_CODE_OK;
}
int cpe_set_igd_ms_stun_max_keepalive_perio(cwmp_t * cwmp, const char * name, const char * value, int length, callback_register_func_t callback_reg)
{
	if(cwmp_conf_get_int("stun:max_keepalive_period") == atoi(value)){
		cwmp_log_info("Value is same as before for stun:max_keepalive_period\n");
		return FAULT_CODE_OK;
	}
	cwmp_conf_set("stun:max_keepalive_period", value);
	cwmp->stun.stun_conf.max_keepalive_period = cwmp_conf_get_int("stun:max_keepalive_period");
	cwmp->stun.state = CWMP_STUN_STATE_INIT;

	if(clsapi_base_set_apply_conf_param(UCI_CFG_NETCWMP, UCI_SECTION_STUN, UCI_PARA_MAX_KEEPALIVE_PERIOD, value))
		return FAULT_CODE_9002;
    return FAULT_CODE_OK;
}

//Device.ManagementServer.STUNMinimumKeepAlivePeriod
int cpe_get_igd_ms_stun_min_keepalive_perio(cwmp_t * cwmp, const char * name, char ** value, pool_t * pool)
{
	char buf[256] = {0};

	snprintf(buf, sizeof(buf), "%d", cwmp_conf_get_int("stun:min_keepalive_period"));
	*value = PSTRDUP(buf);

	return FAULT_CODE_OK;
}
int cpe_set_igd_ms_stun_min_keepalive_perio(cwmp_t * cwmp, const char * name, const char * value, int length, callback_register_func_t callback_reg)
{
	if(cwmp_conf_get_int("stun:min_keepalive_period") == atoi(value)){
		cwmp_log_info("Value is same as before for stun:min_keepalive_period\n");
		return FAULT_CODE_OK;
	}
	cwmp_conf_set("stun:min_keepalive_period", value);
	cwmp->stun.stun_conf.min_keepalive_period = cwmp_conf_get_int("stun:min_keepalive_period");
	cwmp->stun.state = CWMP_STUN_STATE_INIT;

	if(clsapi_base_set_apply_conf_param(UCI_CFG_NETCWMP, UCI_SECTION_STUN, UCI_PARA_MIN_KEEPALIVE_PERIOD, value))
		return FAULT_CODE_9002;
    return FAULT_CODE_OK;
}
