/*
 *  Copyright (c) 2021-2024, Clourney Semiconductor. All rights reserved.
 *
 *  This software and/or documentation is licensed by Clourney Semiconductor under limited terms and conditions.
 *  Reproduction and redistribution in binary or source form, with or without modification,
 *  for use solely in conjunction with a Clourney Semiconductor chipset, is permitted in condition which
 *  must retain the above copyright notice.
 *
 *  By using this software and/or documentation, you agree to the limited terms and conditions.
 */
#include "clsapi_wifi.h"

int  cpe_refresh_dev_wifi_endpoint(cwmp_t * cwmp, parameter_node_t * param_node, callback_register_func_t callback_reg)
{
    FUNCTION_TRACE();

    if(!param_node)
    {
        return FAULT_CODE_9002;
    }
    parameter_node_t * tmp_param, *tmp_node, *child_param;
    child_param = param_node->child;
    if(child_param)
    {
        for(tmp_param=child_param->next_sibling; tmp_param; )
        {
            cwmp_log_info("refresh endpoint node, delete param %s\n", tmp_param->name);
            tmp_node = tmp_param->next_sibling;
            cwmp_model_delete_parameter(tmp_param);
            tmp_param = tmp_node;
        }
        child_param->next_sibling = NULL;
        cwmp_model_refresh_object(cwmp, param_node, 0, callback_reg);
    }
    
    return FAULT_CODE_OK;
}

/* Device.WiFi.EndPoint.{i}.Enable */
int cpe_get_dev_wifi_endpoint_enable(cwmp_t * cwmp, const char * name, char ** value, pool_t * pool)
{
	bool enable;
	char buf[32] = {0};

	clsapi_wifi_get_bss_enabled("wlan0", &enable);
	snprintf(buf, sizeof(buf), "%d", enable);
	*value = PSTRDUP(buf);
	return FAULT_CODE_OK;
}

