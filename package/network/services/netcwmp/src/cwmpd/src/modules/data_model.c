#include "cwmp/model.h"
#include "data_model.h"
#include "cwmp_module.h"
#include "InternetGatewayDevice/InternetGatewayDevice.c"
#include "Device/Device.c"

model_func_t ModelFunction[] =
{
    {"cpe_get_igd_di_manufacturer", cpe_get_igd_di_manufacturer},
    {"cpe_get_igd_di_manufactureroui", cpe_get_igd_di_manufactureroui},
    {"cpe_get_igd_di_productclass", cpe_get_igd_di_productclass},
    {"cpe_get_igd_di_serialnumber", cpe_get_igd_di_serialnumber},
    {"cpe_get_igd_di_hardwareversion", cpe_get_igd_di_hardwareversion},
    {"cpe_get_igd_di_softwareversion", cpe_get_igd_di_softwareversion},
    {"cpe_get_igd_di_provisioningcode", cpe_get_igd_di_provisioningcode},
    {"cpe_set_igd_di_provisioningcode", cpe_set_igd_di_provisioningcode},

    {"cpe_get_igd_ms_enable", cpe_get_igd_ms_enable},
    {"cpe_set_igd_ms_enable", cpe_set_igd_ms_enable},
    {"cpe_get_igd_ms_url", cpe_get_igd_ms_url},
    {"cpe_get_igd_ms_url", cpe_get_igd_ms_url},
    {"cpe_get_igd_ms_username", cpe_get_igd_ms_username},
    {"cpe_get_igd_ms_password", cpe_get_igd_ms_password},
    {"cpe_set_igd_ms_username", cpe_set_igd_ms_username},
    {"cpe_set_igd_ms_password", cpe_set_igd_ms_password},
    {"cpe_get_igd_ms_connectionrequesturl", cpe_get_igd_ms_connectionrequesturl},
    {"cpe_get_igd_ms_connectionrequestusername", cpe_get_igd_ms_connectionrequestusername},
    {"cpe_get_igd_ms_connectionrequestpassword", cpe_get_igd_ms_connectionrequestpassword},
    {"cpe_set_igd_ms_connectionrequestusername", cpe_set_igd_ms_connectionrequestusername},
    {"cpe_set_igd_ms_connectionrequestpassword", cpe_set_igd_ms_connectionrequestpassword},
    {"cpe_get_igd_ms_perio_info_enable",	cpe_get_igd_ms_perio_info_enable},
    {"cpe_set_igd_ms_perio_info_enable",	cpe_set_igd_ms_perio_info_enable},
    {"cpe_get_igd_ms_perio_info_intval",	cpe_get_igd_ms_perio_info_intval},
    {"cpe_set_igd_ms_perio_info_intval",	cpe_set_igd_ms_perio_info_intval},
    {"cpe_get_igd_ms_udp_conn_req_addr",			cpe_get_igd_ms_udp_conn_req_addr},
    {"cpe_get_igd_ms_nat_detected",					cpe_get_igd_ms_nat_detected},
    {"cpe_get_igd_ms_stun_enable",					cpe_get_igd_ms_stun_enable},
    {"cpe_set_igd_ms_stun_enable",					cpe_set_igd_ms_stun_enable},
    {"cpe_get_igd_ms_stun_srv_addr",				cpe_get_igd_ms_stun_srv_addr},
    {"cpe_set_igd_ms_stun_srv_addr",				cpe_set_igd_ms_stun_srv_addr},
    {"cpe_get_igd_ms_stun_srv_port",				cpe_get_igd_ms_stun_srv_port},
    {"cpe_set_igd_ms_stun_srv_port",				cpe_set_igd_ms_stun_srv_port},
    {"cpe_get_igd_ms_stun_username",				cpe_get_igd_ms_stun_username},
    {"cpe_set_igd_ms_stun_username",				cpe_set_igd_ms_stun_username},
    {"cpe_get_igd_ms_stun_password",				cpe_get_igd_ms_stun_password},
    {"cpe_set_igd_ms_stun_password",				cpe_set_igd_ms_stun_password},
    {"cpe_get_igd_ms_stun_max_keepalive_perio",		cpe_get_igd_ms_stun_max_keepalive_perio},
    {"cpe_set_igd_ms_stun_max_keepalive_perio",		cpe_set_igd_ms_stun_max_keepalive_perio},
    {"cpe_get_igd_ms_stun_min_keepalive_perio",		cpe_get_igd_ms_stun_min_keepalive_perio},
    {"cpe_set_igd_ms_stun_min_keepalive_perio",		cpe_set_igd_ms_stun_min_keepalive_perio},
    {"cpe_get_dev_wifi_radio_Stats_Noise",		cpe_get_dev_wifi_radio_Stats_Noise},

    {"cpe_refresh_igd_wandevice", cpe_refresh_igd_wandevice},
    {"cpe_refresh_igd_wanconnectiondevice", cpe_refresh_igd_wanconnectiondevice},
    {"cpe_refresh_igd_wanipconnection", cpe_refresh_igd_wanipconnection},

    /*Device. and Device.WiFi.*/
    {"cpe_get_dev_root_datamodel_version", cpe_get_dev_root_datamodel_version},
    {"cpe_get_dev_interface_stack_number_of_entries", cpe_get_dev_interface_stack_number_of_entries},

    {"cpe_get_dev_wifi_radio_number_of_entries", cpe_get_dev_wifi_radio_number_of_entries},
    {"cpe_get_dev_wifi_ssid_number_of_entries", cpe_get_dev_wifi_ssid_number_of_entries},
    {"cpe_get_dev_wifi_ap_number_of_entries", cpe_get_dev_wifi_ap_number_of_entries},
    {"cpe_get_dev_wifi_ep_number_of_entries", cpe_get_dev_wifi_ep_number_of_entries},

    {"cpe_refresh_dev_wifi_radio", cpe_refresh_dev_wifi_radio},
    {"cpe_get_dev_wifi_radio_Enable", cpe_get_dev_wifi_radio_Enable},
    {"cpe_set_dev_wifi_radio_Enable", cpe_set_dev_wifi_radio_Enable},
    {"cpe_get_dev_wifi_radio_SupportedFrequencyBands", cpe_get_dev_wifi_radio_SupportedFrequencyBands},
    {"cpe_get_dev_wifi_radio_OperatingFrequencyBand", cpe_get_dev_wifi_radio_OperatingFrequencyBand},
    {"cpe_get_dev_wifi_radio_SupportedStandards", cpe_get_dev_wifi_radio_SupportedStandards},
    {"cpe_get_dev_wifi_radio_OperatingStandards", cpe_get_dev_wifi_radio_OperatingStandards},
    {"cpe_set_dev_wifi_radio_OperatingStandards", cpe_set_dev_wifi_radio_OperatingStandards},
    {"cpe_get_dev_wifi_radio_PossibleChannels", cpe_get_dev_wifi_radio_PossibleChannels},
    {"cpe_get_dev_wifi_radio_Channel", cpe_get_dev_wifi_radio_Channel},
    {"cpe_set_dev_wifi_radio_Channel", cpe_set_dev_wifi_radio_Channel},
    {"cpe_get_dev_wifi_radio_AutoChannelSupported", cpe_get_dev_wifi_radio_AutoChannelSupported},
    {"cpe_get_dev_wifi_radio_AutoChannelEnable", cpe_get_dev_wifi_radio_AutoChannelEnable},
    {"cpe_set_dev_wifi_radio_AutoChannelEnable", cpe_set_dev_wifi_radio_AutoChannelEnable},
    {"cpe_get_dev_wifi_radio_MaxSupportedSSIDs", cpe_get_dev_wifi_radio_MaxSupportedSSIDs},
    {"cpe_get_dev_wifi_radio_MaxSupportedAssociations", cpe_get_dev_wifi_radio_MaxSupportedAssociations},
    {"cpe_get_dev_wifi_radio_SupportedOperatingChannelBandwidths", cpe_get_dev_wifi_radio_SupportedOperatingChannelBandwidths},
    {"cpe_get_dev_wifi_radio_OperatingChannelBandwidth", cpe_get_dev_wifi_radio_OperatingChannelBandwidth},
    {"cpe_set_dev_wifi_radio_OperatingChannelBandwidth", cpe_set_dev_wifi_radio_OperatingChannelBandwidth},
    {"cpe_get_dev_wifi_radio_CurrentOperatingChannelBandwidth", cpe_get_dev_wifi_radio_CurrentOperatingChannelBandwidth},
    {"cpe_get_dev_wifi_radio_TransmitPower", cpe_get_dev_wifi_radio_TransmitPower},
    {"cpe_set_dev_wifi_radio_TransmitPower", cpe_set_dev_wifi_radio_TransmitPower},
    {"cpe_get_dev_wifi_radio_RTSThreshold", cpe_get_dev_wifi_radio_RTSThreshold},
    {"cpe_set_dev_wifi_radio_RTSThreshold", cpe_set_dev_wifi_radio_RTSThreshold},
    {"cpe_get_dev_wifi_radio_BeaconPeriod", cpe_get_dev_wifi_radio_BeaconPeriod},
    {"cpe_set_dev_wifi_radio_BeaconPeriod", cpe_set_dev_wifi_radio_BeaconPeriod},

    {"cpe_refresh_dev_wifi_ssid", cpe_refresh_dev_wifi_ssid},
    {"cpe_add_dev_wifi_ssid", cpe_add_dev_wifi_ssid},
    {"cpe_del_dev_wifi_ssid", cpe_del_dev_wifi_ssid},
    {"cpe_get_dev_wifi_ssid_Enable", cpe_get_dev_wifi_ssid_Enable},
    {"cpe_set_dev_wifi_ssid_Enable", cpe_set_dev_wifi_ssid_Enable},
    {"cpe_get_dev_wifi_ssid_BSSID", cpe_get_dev_wifi_ssid_BSSID},
    {"cpe_get_dev_wifi_ssid_MACAddress", cpe_get_dev_wifi_ssid_MACAddress},
    {"cpe_get_dev_wifi_ssid_SSID", cpe_get_dev_wifi_ssid_SSID},
    {"cpe_set_dev_wifi_ssid_SSID", cpe_set_dev_wifi_ssid_SSID},

    {"cpe_refresh_dev_wifi_accesspoint", cpe_refresh_dev_wifi_accesspoint},
    {"cpe_add_dev_wifi_accesspoint", cpe_add_dev_wifi_accesspoint},
    {"cpe_del_dev_wifi_accesspoint", cpe_del_dev_wifi_accesspoint},
    {"cpe_get_dev_wifi_accesspoint_Enable", cpe_get_dev_wifi_accesspoint_Enable},
    {"cpe_set_dev_wifi_accesspoint_Enable", cpe_set_dev_wifi_accesspoint_Enable},
    {"cpe_get_dev_wifi_accesspoint_SSIDReference", cpe_get_dev_wifi_accesspoint_SSIDReference},
    {"cpe_get_dev_wifi_accesspoint_SSIDAdvertisementEnabled", cpe_get_dev_wifi_accesspoint_SSIDAdvertisementEnabled},
    {"cpe_set_dev_wifi_accesspoint_SSIDAdvertisementEnabled", cpe_set_dev_wifi_accesspoint_SSIDAdvertisementEnabled},
    {"cpe_get_dev_wifi_accesspoint_WMMCapability", cpe_get_dev_wifi_accesspoint_WMMCapability},
    {"cpe_get_dev_wifi_accesspoint_WMMEnable", cpe_get_dev_wifi_accesspoint_WMMEnable},
    {"cpe_set_dev_wifi_accesspoint_WMMEnable", cpe_set_dev_wifi_accesspoint_WMMEnable},
    {"cpe_get_dev_wifi_accesspoint_AssociatedDeviceNumberOfEntries", cpe_get_dev_wifi_accesspoint_AssociatedDeviceNumberOfEntries},
    {"cpe_get_dev_wifi_accesspoint_MaxAssociatedDevices", cpe_get_dev_wifi_accesspoint_MaxAssociatedDevices},
    {"cpe_set_dev_wifi_accesspoint_MaxAssociatedDevices", cpe_set_dev_wifi_accesspoint_MaxAssociatedDevices},
    {"cpe_get_dev_wifi_accesspoint_MaxAllowedAssociations", cpe_get_dev_wifi_accesspoint_MaxAllowedAssociations},
    {"cpe_set_dev_wifi_accesspoint_MaxAllowedAssociations", cpe_set_dev_wifi_accesspoint_MaxAllowedAssociations},
    {"cpe_get_dev_wifi_accesspoint_MACAddressControlEnabled", cpe_get_dev_wifi_accesspoint_MACAddressControlEnabled},
    {"cpe_set_dev_wifi_accesspoint_MACAddressControlEnabled", cpe_set_dev_wifi_accesspoint_MACAddressControlEnabled},
    {"cpe_get_dev_wifi_accesspoint_AllowedMACAddress", cpe_get_dev_wifi_accesspoint_AllowedMACAddress},
    {"cpe_set_dev_wifi_accesspoint_AllowedMACAddress", cpe_set_dev_wifi_accesspoint_AllowedMACAddress},
    {"cpe_get_dev_wifi_accesspoint_security_ModesSupported", cpe_get_dev_wifi_accesspoint_security_ModesSupported},
    {"cpe_get_dev_wifi_accesspoint_security_ModeEnabled", cpe_get_dev_wifi_accesspoint_security_ModeEnabled},
    {"cpe_set_dev_wifi_accesspoint_security_ModeEnabled", cpe_set_dev_wifi_accesspoint_security_ModeEnabled},
    {"cpe_get_dev_wifi_accesspoint_security_WEPKey", cpe_get_dev_wifi_accesspoint_security_WEPKey},
    {"cpe_set_dev_wifi_accesspoint_security_WEPKey", cpe_set_dev_wifi_accesspoint_security_WEPKey},
    {"cpe_get_dev_wifi_accesspoint_security_KeyPassphrase", cpe_get_dev_wifi_accesspoint_security_KeyPassphrase},
    {"cpe_set_dev_wifi_accesspoint_security_KeyPassphrase", cpe_set_dev_wifi_accesspoint_security_KeyPassphrase},
    {"cpe_get_dev_wifi_accesspoint_security_RadiusServerIPAddr", cpe_get_dev_wifi_accesspoint_security_RadiusServerIPAddr},
    {"cpe_set_dev_wifi_accesspoint_security_RadiusServerIPAddr", cpe_set_dev_wifi_accesspoint_security_RadiusServerIPAddr},
    {"cpe_get_dev_wifi_accesspoint_security_RadiusServerPort", cpe_get_dev_wifi_accesspoint_security_RadiusServerPort},
    {"cpe_set_dev_wifi_accesspoint_security_RadiusServerPort", cpe_set_dev_wifi_accesspoint_security_RadiusServerPort},
    {"cpe_get_dev_wifi_accesspoint_security_RadiusSecret", cpe_get_dev_wifi_accesspoint_security_RadiusSecret},
    {"cpe_set_dev_wifi_accesspoint_security_RadiusSecret", cpe_set_dev_wifi_accesspoint_security_RadiusSecret},
    {"cpe_get_dev_wifi_accesspoint_assocdev_MACAddress", cpe_get_dev_wifi_accesspoint_assocdev_MACAddress},
    {"cpe_get_dev_wifi_accesspoint_assocdev_AuthenticationState", cpe_get_dev_wifi_accesspoint_assocdev_AuthenticationState},
    {"cpe_get_dev_wifi_accesspoint_assocdev_LastDataDownlinkRate", cpe_get_dev_wifi_accesspoint_assocdev_LastDataDownlinkRate},
    {"cpe_get_dev_wifi_accesspoint_assocdev_LastDataUplinkRate", cpe_get_dev_wifi_accesspoint_assocdev_LastDataUplinkRate},
    {"cpe_get_dev_wifi_accesspoint_assocdev_AssociationTime", cpe_get_dev_wifi_accesspoint_assocdev_AssociationTime},
    {"cpe_get_dev_wifi_accesspoint_assocdev_SignalStrength", cpe_get_dev_wifi_accesspoint_assocdev_SignalStrength},
    {"cpe_get_dev_wifi_accesspoint_assocdev_Retransmissions", cpe_get_dev_wifi_accesspoint_assocdev_Retransmissions},
    {"cpe_get_dev_wifi_accesspoint_assocdev_Active", cpe_get_dev_wifi_accesspoint_assocdev_Active},
    {"cpe_get_dev_wifi_accesspoint_assocdev_stats_BytesSent", cpe_get_dev_wifi_accesspoint_assocdev_stats_BytesSent},
    {"cpe_get_dev_wifi_accesspoint_assocdev_stats_BytesReceived", cpe_get_dev_wifi_accesspoint_assocdev_stats_BytesReceived},
    {"cpe_get_dev_wifi_accesspoint_assocdev_stats_PacketsSent", cpe_get_dev_wifi_accesspoint_assocdev_stats_PacketsSent},
    {"cpe_get_dev_wifi_accesspoint_assocdev_stats_PacketsReceived", cpe_get_dev_wifi_accesspoint_assocdev_stats_PacketsReceived},
    {"cpe_get_dev_wifi_accesspoint_assocdev_stats_ErrorsSent", cpe_get_dev_wifi_accesspoint_assocdev_stats_ErrorsSent},
    {"cpe_get_dev_wifi_accesspoint_assocdev_stats_RetransCount", cpe_get_dev_wifi_accesspoint_assocdev_stats_RetransCount},
    {"cpe_get_dev_wifi_accesspoint_assocdev_stats_FailedRetransCount", cpe_get_dev_wifi_accesspoint_assocdev_stats_FailedRetransCount},
    {"cpe_get_dev_wifi_accesspoint_assocdev_stats_RetryCount", cpe_get_dev_wifi_accesspoint_assocdev_stats_RetryCount},

    {"cpe_refresh_dev_wifi_endpoint", cpe_refresh_dev_wifi_endpoint},
    {"cpe_get_dev_wifi_endpoint_enable", cpe_get_dev_wifi_endpoint_enable},
};

int get_index_after_paramname(parameter_node_t * param, const char * tag_name)
{
    parameter_node_t * parent;
    parameter_node_t * tmp;
    for(parent=param->parent, tmp = param; parent; tmp = parent, parent = parent->parent)
    {
        if(TRstrcmp(parent->name, tag_name) == 0)
        {
            if(is_digit(tmp->name) == 0)
            {
                return TRatoi(tmp->name);
            }
        }
    }
    return -1;
}


void cwmp_model_load(cwmp_t * cwmp, const char * xmlfile)
{
    cwmp_model_load_xml(cwmp, xmlfile, ModelFunction, sizeof(ModelFunction)/sizeof(model_func_t));
}


