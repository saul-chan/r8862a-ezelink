/*
 *  Broadband Forum IEEE 1905.1/1a stack
 *  
 *  Copyright (c) 2017, Broadband Forum
 *  
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are
 *  met:
 *  
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  
 *  Subject to the terms and conditions of this license, each copyright
 *  holder and contributor hereby grants to those receiving rights under
 *  this license a perpetual, worldwide, non-exclusive, no-charge,
 *  royalty-free, irrevocable (except for failure to satisfy the
 *  conditions of this license) patent license to make, have made, use,
 *  offer to sell, sell, import, and otherwise transfer this software,
 *  where such license applies only to those patent claims, already
 *  acquired or hereafter acquired, licensable by such copyright holder or
 *  contributor that are necessarily infringed by:
 *  
 *  (a) their Contribution(s) (the licensed copyrights of copyright holders
 *      and non-copyrightable additions of contributors, in source or binary
 *      form) alone; or
 *  
 *  (b) combination of their Contribution(s) with the work of authorship to
 *      which such Contribution(s) was added by such copyright holder or
 *      contributor, if, at the time the Contribution is added, such addition
 *      causes such combination to be necessarily infringed. The patent
 *      license shall not apply to any other combinations which include the
 *      Contribution.
 *  
 *  Except as expressly stated above, no rights or licenses from any
 *  copyright holder or contributor is granted under this license, whether
 *  expressly, by implication, estoppel or otherwise.
 *  
 *  DISCLAIMER
 *  
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
 *  IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 *  TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 *  PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 *  OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 *  TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 *  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 *  DAMAGE.
 */

#include "platform.h"
#include "datamodel.h"
#include "platform_interfaces.h"
#include "platform_interfaces_priv.h"
#include "platform_os.h"

#ifdef _FLAVOUR_ARM_WRT1900ACX_
#include "platform_interfaces_wrt1900acx_priv.h"
#endif

#include <stdio.h>            // printf(), popen()
#include <stdlib.h>           // malloc(), ssize_t
#include <stdarg.h>           // va_*
#include <string.h>           // strdup()
#include <errno.h>            // errno
#include <unistd.h>           // sleep()

#include <arpa/inet.h>        // socket, AF_INTER, htons(), ...
#include <linux/if_packet.h>  // sockaddr_ll
#include <sys/ioctl.h>        // ioctl(), SIOCGIFINDEX
#include <net/if.h>           // struct ifreq, IFNAZSIZE
#include <netinet/ether.h>    // ETH_P_ALL, ETH_A_LEN 
#include <unistd.h>           // close()
#include <pthread.h>          // pthread_create(), mutex functions

#include "datamodel.h"

////////////////////////////////////////////////////////////////////////////////
// Private data and functions
////////////////////////////////////////////////////////////////////////////////

// Global variables to store the list of names of the interfaces and their
// status.
//
static DEFINE_DLIST_HEAD(interfaces_list);

// The interfaces status variables can be accessed/modified from different
// threads:
//   - The "main" AL thread (where "start1905AL()" runs)
//   - The "push button" configuration process thread
//
// Thus, their access must be protected with a mutex
//
pthread_mutex_t interface_mutex = PTHREAD_MUTEX_INITIALIZER;

#define INTF_TYPE_ERROR          (0)
#define INTF_TYPE_ETHERNET       (1)
#define INTF_TYPE_WIFI           (2)
#define INTF_TYPE_UNKNOWN        (0xFF)

uint8_t _getInterfaceType(char *interface_name)
{
    // According to www.kernel.org/doc/Documentation/ABI/testing/sysfs-class-net
    //
    //                                    Regular ethernet          Wifi
    //                                    interface                 interface
    //                                    ================          =========
    //
    // /sys/class/net/<iface>/type        1                         1
    //
    // /sys/class/net/<iface>/wireless    <Does not exist>          <Exists>

    uint8_t  ret;

    FILE *fp;

    char sys_path[100];

    ret = INTF_TYPE_ERROR;

    snprintf(sys_path, sizeof(sys_path), "/sys/class/net/%s/type", interface_name);

    if(NULL != (fp = fopen(sys_path, "r")))
    {
        char aux[30];

        if (NULL != fgets(aux, sizeof(aux), fp))
        {
            int interface_type;

            interface_type = atoi(aux);

            switch (interface_type)
            {
                case 1:
                {
                    snprintf(sys_path, sizeof(sys_path), "/sys/class/net/%s/wireless", interface_name);

                    if( -1 != access( sys_path, F_OK ))
                    {
                        // Wireless
                        //
                        ret = INTF_TYPE_WIFI;
                    }
                    else
                    {
                        // Ethernet
                        //
                        ret = INTF_TYPE_ETHERNET;
                    }
                    break;
                }

                default:
                {
                    DEBUG_ERROR("[PLATFORM] Unknow interface type %d\n", interface_type);
                    ret = INTF_TYPE_UNKNOWN;

                    break;
                }
            }
        }

        fclose(fp);
    }

    return ret;
}

// These are the data structures and functions in charge of handling the press
// of physical/virtual buttons on the platform
//
struct _pushButtonThreadData
{
    uint8_t     queue_id;
    char     *interface_name;
    uint8_t     al_mac_address[6];
    uint16_t    mid;
};

// Returns an uint32_t obtained by reading the first line of file
// "/sys/class/net/<interface_name>/<parameter_name>"
//
static int32_t _readInterfaceParameter(char *interface_name, char *parameter_name)
{
    int32_t ret;

    FILE *fp;

    char sys_path[100];

    ret = 0;

    snprintf(sys_path, sizeof(sys_path), "/sys/class/net/%s/%s", interface_name, parameter_name);

    if(NULL != (fp = fopen(sys_path, "r")))
    {
        char aux[30];
      
        if (NULL != fgets(aux, sizeof(aux), fp))
        {
            ret = atoi(aux);
        }
        fclose(fp);
    }

    return ret;
}

// Returns an int32_t obtained by reading the output of
// "iw dev $INTERFACE station get $MAC | grep $PARAMETER_NAME"
//
static int32_t _readWifiNeighborParameter(char *interface_name, uint8_t *neighbor_interface_address, char *parameter_name)
{
    int32_t   ret;
    FILE    *pipe;
    char    *line;
    size_t   len;

    char     command[200];

    ret = 0;

    if ((NULL == interface_name) || (NULL == neighbor_interface_address) || (NULL == parameter_name))
    {
        return 0;
    }

    sprintf(command, "iw dev %s station get %02x:%02x:%02x:%02x:%02x:%02x | grep %s",
            interface_name,
            neighbor_interface_address[0], neighbor_interface_address[1], neighbor_interface_address[2],
            neighbor_interface_address[3], neighbor_interface_address[4], neighbor_interface_address[5],
            parameter_name);

    // Execute the IW query command
    //
    pipe = popen(command, "r");

    if (!pipe)
    {
        DEBUG_ERROR("[PLATFORM] popen() returned with errno=%d (%s)\n", errno, strerror(errno));
        return 0;
    }

    // Next read the parameter
    //
    line = NULL;
    if (-1 != getline(&line, &len, pipe))
    {
        char    *value;

        // Remove the last "\n"
        //
        line[strlen(line)-1] = 0x00;

        value = strstr(line, ":");
        if ((NULL != value) && (1 == sscanf(value+1, "%d", &ret)))
        {
             DEBUG_DETAIL("[PLATFORM] Neighbor %02x:%02x:%02x:%02x:%02x:%02x (%s) %s = %d\n",
                 neighbor_interface_address[0], neighbor_interface_address[1], neighbor_interface_address[2],
                 neighbor_interface_address[3], neighbor_interface_address[4], neighbor_interface_address[5],
                 interface_name, parameter_name, ret);
        }
        else
        {
             DEBUG_DETAIL("[PLATFORM] Parameter not found\n");
        }
    }

    free(line);
    pclose(pipe);

    return ret;
}

void addInterface(char *long_interface_name, uint8_t fix)
{
    struct interface_list_item *item;

    dlist_for_each(item, interfaces_list, l) {
        if (!strcmp(item->intf_name, long_interface_name)) {
            DEBUG_ERROR("duplicated interface %s\n", long_interface_name);
            return;
        }
    }

    if ((item=calloc(1, sizeof(struct interface_list_item)))) {
        item->intf_name = strdup(long_interface_name);
        if (fix)
            item->fix = 1;
        dlist_add_tail(&interfaces_list, &item->l);
    }
}

void removeInterface(char *long_interface_name)
{
    struct interface_list_item *item;

    dlist_for_each(item, interfaces_list, l) {
        if (!strcmp(item->intf_name, long_interface_name)) {
            dlist_remove(&item->l);
            free(item->intf_name);
            free(item);
            return;
        }
    }
    DEBUG_ERROR("can not remove nonexist interface %s\n", long_interface_name);
}

////////////////////////////////////////////////////////////////////////////////
// Platform API: Interface related functions to be used by platform-independent
// files (functions declarations are  found in "../interfaces/platform.h)
////////////////////////////////////////////////////////////////////////////////
dlist_head *PLATFORM_GET_LIST_OF_1905_INTERFACES()
{
    return &interfaces_list;
}

struct interfaceInfo *PLATFORM_GET_1905_INTERFACE_INFO(char *interface_name)
{
    struct interfaceInfo *m;

    struct ifreq s;

    uint8_t i;

    struct interface *intf;

    if (!(intf = interfaceFindName(local_device, interface_name))) {
        DEBUG_ERROR("Unknown interface %s!\n", interface_name);
        return NULL;
    }

    DEBUG_DETAIL("[PLATFORM] Retrieving info for interface %s\n", interface_name);

    if (!(m = (struct interfaceInfo *)malloc(sizeof(struct interfaceInfo)))) {
        DEBUG_ERROR("[PLATFORM] Not enough memory for the 'interface' structure\n");
        return NULL;
    }

    m->index = intf->index;
    // Fill the 'name' field
    //
    m->name = strdup(interface_name);

    // Obtain the 'mac_address'
    //

    // Give "sane" values in case any of the following parameters can not be
    // filled later
    //
    m->mac_address[0] = 0x00;
    m->mac_address[1] = 0x00;
    m->mac_address[2] = 0x00;
    m->mac_address[3] = 0x00;
    m->mac_address[4] = 0x00;
    m->mac_address[5] = 0x00;

    m->interface_type                                                = INTERFACE_TYPE_UNKNOWN;
    m->interface_type_data.other.oui[0]                              = 0x00;
    m->interface_type_data.other.oui[1]                              = 0x00;
    m->interface_type_data.other.oui[2]                              = 0x00;
    m->interface_type_data.other.generic_phy_description_xml_url     = NULL;
    m->interface_type_data.other.variant_index                       = 0;
    m->interface_type_data.other.variant_name                        = 0;
    m->interface_type_data.other.media_specific.unsupported.bytes_nr = 0;
    m->interface_type_data.other.media_specific.unsupported.bytes    = NULL;

    m->is_secured                     = 0;
    m->push_button_on_going           = 2;    // "2" means "unsupported"
    m->push_button_new_mac_address[0] = 0x00;
    m->push_button_new_mac_address[1] = 0x00;
    m->push_button_new_mac_address[2] = 0x00;
    m->push_button_new_mac_address[3] = 0x00;
    m->push_button_new_mac_address[4] = 0x00;
    m->push_button_new_mac_address[5] = 0x00;

    m->power_state                    = INTERFACE_POWER_STATE_OFF;
    m->neighbor_mac_addresses_nr      = INTERFACE_NEIGHBORS_UNKNOWN;
    m->neighbor_mac_addresses         = NULL;

    m->ipv4_nr                        = 0;
    m->ipv4                           = NULL;
    m->ipv6_nr                        = 0;
    m->ipv6                           = NULL;

    m->vendor_specific_elements_nr    = 0;
    m->vendor_specific_elements       = NULL;

    {
        // *********************************************************************
        // ********************** REGULAR INTERFACE ****************************
        // *********************************************************************
        
        // This is a "regular" interface. Query the Linux kernel for data

        int fd;

        strcpy(s.ifr_name, m->name);
        fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);
        if ((fd<0) || (0 != ioctl(fd, SIOCGIFHWADDR, &s)))
        {
            DEBUG_ERROR("[PLATFORM] Could not obtain MAC address of interface %s\n", m->name);
            free(m->name);
            free(m);
            if (fd>=0)
                close(fd);
            return NULL;
        }
        close(fd);
        memcpy(m->mac_address, s.ifr_addr.sa_data, 6);

#ifdef _FLAVOUR_ARM_WRT1900ACX_
        // The "linksys wrt1900ac" platform flavour uses the following flavour
        // specific function to fill the "m" structure.
        //
        linksys_wrt1900acx_get_interface_info(interface_name, m);
#else
        // TODO: In a flavour-neutral device we don't really know how to fill
        //       many of these things, thus we will use "default" values when
        //       needed. This will obviously not always work!

        //DEBUG_WARNING("[PLATFORM] No platform flavour defined. Using default values when needed.\n");

        switch (_getInterfaceType(interface_name))
        {
            case INTF_TYPE_ETHERNET:
            {
                m->interface_type = INTERFACE_TYPE_IEEE_802_3AB_GIGABIT_ETHERNET;
                break;
            }
            case INTF_TYPE_WIFI:
            {
                m->interface_type = INTERFACE_TYPE_IEEE_802_11B_2_4_GHZ;
                break;
            }
            default:
            {
                DEBUG_ERROR("[PLATFORM] Unknown interface type. Assuming ethernet.\n");

                m->interface_type = INTERFACE_TYPE_IEEE_802_3AB_GIGABIT_ETHERNET;
                break;
            }
        }

        // Check 'is_secured'
        //
        if (
             INTERFACE_TYPE_IEEE_802_3U_FAST_ETHERNET     == m->interface_type ||
             INTERFACE_TYPE_IEEE_802_3AB_GIGABIT_ETHERNET == m->interface_type
           )
        {
            m->is_secured = 1;
        }
        else if ((
             INTERFACE_TYPE_IEEE_802_11B_2_4_GHZ == m->interface_type ||
             INTERFACE_TYPE_IEEE_802_11G_2_4_GHZ == m->interface_type ||
             INTERFACE_TYPE_IEEE_802_11A_5_GHZ   == m->interface_type ||
             INTERFACE_TYPE_IEEE_802_11N_2_4_GHZ == m->interface_type ||
             INTERFACE_TYPE_IEEE_802_11N_5_GHZ   == m->interface_type ||
             INTERFACE_TYPE_IEEE_802_11AC_5_GHZ  == m->interface_type ||
             INTERFACE_TYPE_IEEE_802_11AD_60_GHZ == m->interface_type ||
             INTERFACE_TYPE_IEEE_802_11AF_GHZ    == m->interface_type
              ) &&
             (m->interface_type_data.ieee80211.authentication_mode == IEEE80211_AUTH_MODE_WPA    ||
              m->interface_type_data.ieee80211.authentication_mode == IEEE80211_AUTH_MODE_WPAPSK ||
              m->interface_type_data.ieee80211.authentication_mode == IEEE80211_AUTH_MODE_WPA2   ||
              m->interface_type_data.ieee80211.authentication_mode == IEEE80211_AUTH_MODE_WPA2PSK)
              )
        {
            m->is_secured = 1;
        }
        else
        {
            //FIXME: allow all type wifi inteface temporary
            m->is_secured = 1;
        }

        // Check 'push button' configuration sequence status
        //
        m->push_button_on_going = 2; // "2" means "not supported"

        // Check the 'power_state'
        //
        m->power_state = INTERFACE_POWER_STATE_ON;

        // Add neighbor MAC addresses
        //
        m->neighbor_mac_addresses_nr = INTERFACE_NEIGHBORS_UNKNOWN;
        m->neighbor_mac_addresses    = NULL;

        // Add IPv4 info
        //
        m->ipv4_nr = 0;
        m->ipv4    = NULL;

        // Add IPv6 info
        //
        m->ipv6_nr = 0;
        m->ipv6    = NULL;

        // Add vendor specific data
        //
        m->vendor_specific_elements_nr = 0;
        m->vendor_specific_elements    = NULL;
#endif
    }

    DEBUG_DETAIL("[PLATFORM]   mac_address                 : %02x:%02x:%02x:%02x:%02x:%02x\n", m->mac_address[0], m->mac_address[1], m->mac_address[2], m->mac_address[3], m->mac_address[4], m->mac_address[5]);
    DEBUG_DETAIL("[PLATFORM]   interface_type              : %d\n", m->interface_type);
    if (
         INTERFACE_TYPE_IEEE_802_11B_2_4_GHZ == m->interface_type ||
         INTERFACE_TYPE_IEEE_802_11G_2_4_GHZ == m->interface_type ||
         INTERFACE_TYPE_IEEE_802_11A_5_GHZ   == m->interface_type ||
         INTERFACE_TYPE_IEEE_802_11N_2_4_GHZ == m->interface_type ||
         INTERFACE_TYPE_IEEE_802_11N_5_GHZ   == m->interface_type ||
         INTERFACE_TYPE_IEEE_802_11AC_5_GHZ  == m->interface_type ||
         INTERFACE_TYPE_IEEE_802_11AD_60_GHZ == m->interface_type ||
         INTERFACE_TYPE_IEEE_802_11AF_GHZ    == m->interface_type
       )
    {
        DEBUG_DETAIL("[PLATFORM]     ieee80211 data\n");
        DEBUG_DETAIL("[PLATFORM]       bssid                       : %02x:%02x:%02x:%02x:%02x:%02x\n", m->interface_type_data.ieee80211.bssid[0], m->interface_type_data.ieee80211.bssid[1], m->interface_type_data.ieee80211.bssid[2], m->interface_type_data.ieee80211.bssid[3], m->interface_type_data.ieee80211.bssid[4], m->interface_type_data.ieee80211.bssid[5]);
        DEBUG_DETAIL("[PLATFORM]       ssid                        : %s\n", m->interface_type_data.ieee80211.ssid);
        DEBUG_DETAIL("[PLATFORM]       role                        : %d\n", m->interface_type_data.ieee80211.role);
        DEBUG_DETAIL("[PLATFORM]       ap_channel_band             : 0x%02x\n", m->interface_type_data.ieee80211.ap_channel_band);
        DEBUG_DETAIL("[PLATFORM]       ap_channel_center_f1        : 0x%02x\n", m->interface_type_data.ieee80211.ap_channel_center_frequency_index_1);
        DEBUG_DETAIL("[PLATFORM]       ap_channel_center_f2        : 0x%02x\n", m->interface_type_data.ieee80211.ap_channel_center_frequency_index_2);
        DEBUG_DETAIL("[PLATFORM]       authentication_mode         : 0x%04x\n", m->interface_type_data.ieee80211.authentication_mode);
        DEBUG_DETAIL("[PLATFORM]       encryption_mode             : 0x%04x\n", m->interface_type_data.ieee80211.encryption_mode);
        DEBUG_DETAIL("[PLATFORM]       network_key                 : %s\n", m->interface_type_data.ieee80211.network_key);
    }
    else if (
         INTERFACE_TYPE_IEEE_1901_WAVELET    == m->interface_type ||
         INTERFACE_TYPE_IEEE_1901_FFT        == m->interface_type
       )
    {
        DEBUG_DETAIL("[PLATFORM]     ieee1901 data\n");
        DEBUG_DETAIL("[PLATFORM]       network_identifier          : %02x:%02x:%02x:%02x:%02x:%02x:%02x\n", m->interface_type_data.ieee1901.network_identifier[0], m->interface_type_data.ieee1901.network_identifier[1], m->interface_type_data.ieee1901.network_identifier[2], m->interface_type_data.ieee1901.network_identifier[3], m->interface_type_data.ieee1901.network_identifier[4], m->interface_type_data.ieee1901.network_identifier[5], m->interface_type_data.ieee1901.network_identifier[6]);
    }
    else if (
              INTERFACE_TYPE_UNKNOWN == m->interface_type
            )
    {
        uint16_t  len;
        uint8_t  *data;

        DEBUG_DETAIL("[PLATFORM]     generic interface data\n");
        DEBUG_DETAIL("[PLATFORM]       OUI                           : %02x:%02x:%02x\n", m->interface_type_data.other.oui[0], m->interface_type_data.other.oui[1], m->interface_type_data.other.oui[2]);
        DEBUG_DETAIL("[PLATFORM]       URL description               : %s\n", NULL == m->interface_type_data.other.generic_phy_description_xml_url ? "<none>" : m->interface_type_data.other.generic_phy_description_xml_url);
        DEBUG_DETAIL("[PLATFORM]       variant index                 : %d\n", m->interface_type_data.other.variant_index);
        DEBUG_DETAIL("[PLATFORM]       variant name                  : %s\n", NULL == m->interface_type_data.other.variant_name ? "<none>" : m->interface_type_data.other.variant_name);
        if (NULL != (data = forge_media_specific_blob(&m->interface_type_data.other, &len)))
        {
            if (len > 4)
            {
                DEBUG_DETAIL("[PLATFORM]       media specific data (%d bytes) : 0x%02x:0x%02x:0x%02x:0x%02x:0x%02x...\n", len, data[0], data[1], data[2], data[3], data[4]);
            }
            else
            {
                DEBUG_DETAIL("[PLATFORM]       media specific data (%d bytes)\n", len);
            }
            free_media_specific_blob(data);
        }
    }
    DEBUG_DETAIL("[PLATFORM]   is_secure                   : %d\n", m->is_secured);
    DEBUG_DETAIL("[PLATFORM]   push_button_on_going        : %d\n", m->push_button_on_going);
    DEBUG_DETAIL("[PLATFORM]   push_button_new_mac_address : %02x:%02x:%02x:%02x:%02x:%02x\n", m->push_button_new_mac_address[0], m->push_button_new_mac_address[1], m->push_button_new_mac_address[2], m->push_button_new_mac_address[3], m->push_button_new_mac_address[4], m->push_button_new_mac_address[5]);
    DEBUG_DETAIL("[PLATFORM]   power_state                 : %d\n", m->power_state);
    DEBUG_DETAIL("[PLATFORM]   neighbor_mac_addresses_nr   : %d\n", m->neighbor_mac_addresses_nr);
    if (m->neighbor_mac_addresses_nr != INTERFACE_NEIGHBORS_UNKNOWN)
    {
        for (i=0; i<m->neighbor_mac_addresses_nr; i++)
        {
            DEBUG_DETAIL("[PLATFORM]     - neighbor #%d                : %02x:%02x:%02x:%02x:%02x:%02x\n", i, m->neighbor_mac_addresses[i][0], m->neighbor_mac_addresses[i][1], m->neighbor_mac_addresses[i][2], m->neighbor_mac_addresses[i][3], m->neighbor_mac_addresses[i][4], m->neighbor_mac_addresses[i][5]);
        }
    }
    DEBUG_DETAIL("[PLATFORM]   IPs                         : %d\n", m->ipv4_nr+m->ipv6_nr);
    for (i=0; i<m->ipv4_nr; i++)
    {
        DEBUG_DETAIL("[PLATFORM]     - IPv4 #%d                    : %d.%d.%d.%d (type = %s, dhcpserver = %d.%d.%d.%d)\n", i, m->ipv4[i].address[0], m->ipv4[i].address[1], m->ipv4[i].address[2], m->ipv4[i].address[3], m->ipv4[i].type == IPV4_UNKNOWN ? "unknown" : m->ipv4[i].type == IPV4_DHCP ? "dhcp" : m->ipv4[i].type == IPV4_STATIC ? "static" : m->ipv4[i].type == IPV4_AUTOIP ? "auto" : "error", m->ipv4[i].dhcp_server[0], m->ipv4[i].dhcp_server[1], m->ipv4[i].dhcp_server[2], m->ipv4[i].dhcp_server[3]);
    }
    for (i=0; i<m->ipv6_nr; i++)
    {
        DEBUG_DETAIL("[PLATFORM]     - IPv6 #%d                    : %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x (type = %s, origin = %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x)\n",
                i,
                m->ipv6[i].address[0], m->ipv6[i].address[1], m->ipv6[i].address[2], m->ipv6[i].address[3], m->ipv6[i].address[4], m->ipv6[i].address[5], m->ipv6[i].address[6], m->ipv6[i].address[7], m->ipv6[i].address[8], m->ipv6[i].address[9], m->ipv6[i].address[10], m->ipv6[i].address[11], m->ipv6[i].address[12], m->ipv6[i].address[13], m->ipv6[i].address[14], m->ipv6[i].address[15],
                m->ipv6[i].type == IPV6_UNKNOWN ? "unknown" : m->ipv6[i].type == IPV6_DHCP ? "dhcp" : m->ipv6[i].type == IPV6_STATIC ? "static" : m->ipv6[i].type == IPV6_SLAAC ? "slaac" : "error",
                m->ipv6[i].origin[0], m->ipv6[i].origin[1], m->ipv6[i].origin[2], m->ipv6[i].origin[3], m->ipv6[i].origin[4], m->ipv6[i].origin[5], m->ipv6[i].origin[6], m->ipv6[i].origin[7], m->ipv6[i].origin[8], m->ipv6[i].origin[9], m->ipv6[i].origin[10], m->ipv6[i].origin[11], m->ipv6[i].origin[12], m->ipv6[i].origin[13], m->ipv6[i].origin[14], m->ipv6[i].origin[15]);
    }
    DEBUG_DETAIL("[PLATFORM]   vendor_specific_elements_nr : %d\n", m->vendor_specific_elements_nr);
    for (i=0; i<m->vendor_specific_elements_nr; i++)
    {
        DEBUG_DETAIL("[PLATFORM]         OUI                       : %02x:%02x:%02x\n", m->vendor_specific_elements[i].oui[0], m->vendor_specific_elements[i].oui[1], m->vendor_specific_elements[i].oui[2]);
        DEBUG_DETAIL("[PLATFORM]         vendor_data_len           : %d\n", m->vendor_specific_elements[i].vendor_data_len);
        DEBUG_DETAIL("[PLATFORM]         vendor_data               : <TODO>\n"); // TODO: Dump bytes as done, for example, in PLATFORM_SEND_ALME_REPLY()
    }

    return m;
}

void PLATFORM_FREE_1905_INTERFACE_INFO(struct interfaceInfo *x)
{
    uint8_t i;

    free(x->name);

    if (INTERFACE_TYPE_UNKNOWN == x->interface_type)
    {
        if (
             !(
               x->interface_type_data.other.oui[0] == 0x00  &&
               x->interface_type_data.other.oui[1] == 0x19  &&
               x->interface_type_data.other.oui[2] == 0xA7  &&
               0 == strcmp("http://handle.itu.int/11.1002/3000/1706", x->interface_type_data.other.generic_phy_description_xml_url) &&
               (x->interface_type_data.other.variant_index == 1 || x->interface_type_data.other.variant_index == 2 || x->interface_type_data.other.variant_index == 3 || x->interface_type_data.other.variant_index == 4)
              )
           )
        {
            if (0 != x->interface_type_data.other.media_specific.unsupported.bytes_nr && NULL != x->interface_type_data.other.media_specific.unsupported.bytes)
            {
                free(x->interface_type_data.other.media_specific.unsupported.bytes);
            }
        }

        if (NULL != x->interface_type_data.other.generic_phy_description_xml_url)
        {
            free(x->interface_type_data.other.generic_phy_description_xml_url);
        }
        if (NULL != x->interface_type_data.other.variant_name)
        {
            free(x->interface_type_data.other.variant_name);
        }
    }

    if (x->neighbor_mac_addresses_nr > 0  && INTERFACE_NEIGHBORS_UNKNOWN != x->neighbor_mac_addresses_nr && NULL != x->neighbor_mac_addresses)
    {
        free(x->neighbor_mac_addresses);
    }

    if (x->ipv4_nr > 0 && NULL != x->ipv4)
    {
        free(x->ipv4);
    }

    if (x->ipv6_nr > 0 && NULL != x->ipv6)
    {
        free(x->ipv6);
    }

    if (x->vendor_specific_elements_nr > 0)
    {
        for (i=0; i<x->vendor_specific_elements_nr; i++)
        {
            if (x->vendor_specific_elements[i].vendor_data_len > 0 && NULL != x->vendor_specific_elements[i].vendor_data)
            {
                free(x->vendor_specific_elements[i].vendor_data);
            }
        }
        free(x->vendor_specific_elements);
    }


    free(x);
}

struct linkMetrics *PLATFORM_GET_LINK_METRICS(char *local_interface_name, uint8_t *neighbor_interface_address)
{
    struct linkMetrics    *ret;
    int32_t tmp;

    ret = (struct linkMetrics *)malloc(sizeof(struct linkMetrics));
    if (NULL == ret)
    {
        return NULL;
    }

    {
        // *********************************************************************
        // ********************** REGULAR INTERFACE ****************************
        // *********************************************************************

        // This is a "regular" interface. Query the Linux kernel for data

        // Obtain how much time the process collecting stats has been running
        //
        //     TODO: This should be set to the amount of seconds ellapsed since
        //     the interface was brought up. However I could not find an easy
        //     way to obtain this information in Linux.
        //     For now, we will simply set this to the amount of seconds since
        //     the system was started, which is typically correct on most
        //     cases.
        //
        ret->measures_window = PLATFORM_GET_TIMESTAMP(0) / 1000;

        // Check interface name
        // TODO: Find a more robust way of identifying a wifi interface. Maybe
        //       checking "/sys"?
        //
        if (strstr(local_interface_name, "wlan") != NULL)
        {
            // Obtain the amount of (correct and incorrect) packets transmitted
            // to 'neighbor_interface_address' in the last
            // 'ret->measures_window' seconds.
            //
            // This is done by reading the response of the command
            //
            //   "iw dev $INTERFACE station get $NEIGHBOR_MAC | grep "tx packets"" and
            //   "iw dev $INTERFACE station get $NEIGHBOR_MAC | grep "tx failed""
            //
            ret->tx_link_metrics.tx_packet_ok     = (uint32_t)_readWifiNeighborParameter(local_interface_name, neighbor_interface_address, "\"tx packets\"");
            ret->tx_link_metrics.tx_packet_errors = (uint32_t)_readWifiNeighborParameter(local_interface_name, neighbor_interface_address, "\"tx failed\"");

            // Obtain the estimated max MAC xput and PHY rate when transmitting
            // data from "A" to "B".
            //
            // This is done by reading the response of the command
            //
            //   "iw dev $INTERFACE station get $NEIGHBOR_MAC | grep speed"
            //
            ret->tx_link_metrics.tx_max_xput = (uint16_t)_readWifiNeighborParameter(local_interface_name, neighbor_interface_address, "\"tx bitrate\"");
            ret->tx_link_metrics.tx_phy_rate = (uint16_t)_readWifiNeighborParameter(local_interface_name, neighbor_interface_address, "\"tx bitrate\"");

            // Obtain the estimated average percentage of time that the link is
            // available for transmission.
            //
            //   TODO: I'll just say "100% of the time" for now.
            //
            ret->tx_link_metrics.tx_link_availability = 100;

            // Obtain the amount of (correct and incorrect) packets received
            // from 'neighbor_interface_address' in the last
            // 'ret->measures_window' seconds.
            //
            // This is done by reading the response of the command
            //
            //   "iw dev $INTERFACE station get $NEIGHBOR_MAC | grep "rx packets""
            //
            //   TODO: rx errors can't be obtained from this console command.
            //   Right now it's assigned a zero value. Investigate how to
            //   obtain this value.
            //
            ret->rx_link_metrics.rx_packet_ok     = (uint32_t)_readWifiNeighborParameter(local_interface_name, neighbor_interface_address, "\"rx packets\"");
            ret->rx_link_metrics.rx_packet_errors = 0;


            // Obtain the estimated RX RSSI
            //
            // RSSI is a term used to measure the relative quality of a
            // received signal to a client device, but has no absolute value.
            // The IEEE 802.11 standard specifies that RSSI can be on a scale
            // of 0 to up to 255 and that each chipset manufacturer can define
            // their own “RSSI_Max” value. It’s all up to the manufacturer
            // (which is why RSSI is a relative index), but you can infer that
            // the higher the RSSI value is, the better the signal is.
            //
            // A basic (saturated linear) conversion formula has been
            // implemented:
            //   RSSI range 0-100 <--> signal -40 to -70 dBm
            //
            // Feel free to redefine this conversion formula. Maybe to a
            // logarithmical one.
            //
            tmp = _readWifiNeighborParameter(local_interface_name, neighbor_interface_address, "\"signal:\"");

            #define  SIGNAL_MAX  (-40)   // dBm
            #define  SIGNAL_MIN  (-70)
            if (tmp >= SIGNAL_MAX)
            {
                ret->rx_link_metrics.rx_rssi = 100;
            }
            else if(tmp <= SIGNAL_MIN)
            {
                ret->rx_link_metrics.rx_rssi = 0;
            }
            else
            {
                ret->rx_link_metrics.rx_rssi = (tmp - SIGNAL_MIN)*100/(SIGNAL_MAX - SIGNAL_MIN);
            }
        }
        // Other interface types, probably ethernet
        else
        {
            // Obtain the amount of (correct and incorrect) packets transmitted
            // to 'neighbor_interface_address' in the last
            // 'ret->measures_window' seconds.
            //
            //   TODO: In Linux there is no easy way to obtain this
            //   information. We will just report the amount of *total* packets
            //   transmitted from our local interface, no matter the
            //   destination.
            //   This information will only be correct when the local interface
            //   is connected to one single remote interface... however we
            //   better report this than nothing at all.
            //
            // This is done by reading the contents of files
            //
            //   "/sys/class/net/<interface_name>/statistics/tx_packets" and
            //   "/sys/class/net/<interface_name>/statistics/tx_errors"
            //
            ret->tx_link_metrics.tx_packet_ok     = (uint32_t)_readInterfaceParameter(local_interface_name, "statistics/tx_packets");
            ret->tx_link_metrics.tx_packet_errors = (uint32_t)_readInterfaceParameter(local_interface_name, "statistics/tx_errors");

            // Obtain the estimatid max MAC xput and PHY rate when transmitting
            // data from "A" to "B".
            //
            //   TODO: The same considerations as in the previous parameters
            //   apply here.
            //
            // This is done by reading the contents of file
            //
            //   "/sys/class/net/<interface_name>/speed
            //
            // NOTE: I'll set both parameters to the same value. Is there a
            // better way to do this?
            //
            ret->tx_link_metrics.tx_max_xput = (uint16_t)_readInterfaceParameter(local_interface_name, "speed");
            ret->tx_link_metrics.tx_phy_rate = (uint16_t)_readInterfaceParameter(local_interface_name, "speed");

            // Obtain the estimated average percentage of time that the link is
            // available for transmission.
            //
            //   TODO: I'll just say "100% of the time" for now.
            //
            ret->tx_link_metrics.tx_link_availability = 100;

            // Obtain the amount of (correct and incorrect) packets received from
            // 'neighbor_interface_address' in the last 'ret->measures_window'
            // seconds.
            //
            //   TODO: In Linux there is no easy way to obtain this information. We
            //   will just report the amount of *total* packets received on our
            //   local interface, no matter the origin.
            //   This information will only be correct when the local interface is
            //   connected to one single remote interface... however we better
            //   report this than nothing at all.
            //
            // This is done by reading the contents of files
            //
            //   "/sys/class/net/<interface_name>/statistics/rx_packets" and
            //   "/sys/class/net/<interface_name>/statistics/rx_errors"
            //
            ret->rx_link_metrics.rx_packet_ok     = (uint32_t)_readInterfaceParameter(local_interface_name, "statistics/rx_packets");
            ret->rx_link_metrics.rx_packet_errors = (uint32_t)_readInterfaceParameter(local_interface_name, "statistics/rx_errors");

            // Obtain the estimated RX RSSI
            //
            //   TODO: ???
            ret->rx_link_metrics.rx_rssi = 0;
        }
    }

    return ret;
}

void PLATFORM_FREE_LINK_METRICS(struct linkMetrics *l)
{
    // This is a simple structure which does not require any special treatment.
    //
    if (l)
    {
        free(l);
    }

    return;
}


struct bridge *PLATFORM_GET_LIST_OF_BRIDGES(uint8_t *nr)
{
    // TODO

    *nr = 0;
    return NULL;
}

void PLATFORM_FREE_LIST_OF_BRIDGES(struct bridge *x, uint8_t nr)
{
    // TODO
    //
    if (0 == nr || NULL == x)
    {
        return;
    }

    return;
}

#define MIN_RAW_PACKET_SIZE (60) //min ethernet packet size
uint8_t PLATFORM_SEND_RAW_PACKET2(char *interface_name, uint8_t *dst_mac, uint8_t *src_mac,
                                    uint16_t eth_type, struct cmdu_buf *buf)
{
    uint8_t ret = 0;
    int                 s;
    struct ifreq        ifr;
    int                 ifindex;
    struct sockaddr_ll  socket_address;

    struct ether_header *eh;

    if (!interface_name) {
        DEBUG_ERROR("Preparing to send RAW packet on a not-exist interface\n");
        goto fail;
    }

    DEBUG_DETAIL("Preparing to send RAW on %s, len=%ld\n", interface_name, cmduBufLen(buf));
    DEBUG_DETAIL("DST="MACFMT", SRC="MACFMT", EtherType=0x%04x\n",
                                    MACARG(dst_mac), MACARG(src_mac), eth_type);

    s = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (-1 == s) {
        DEBUG_ERROR("failed open socket errno=%d (%s)\n", errno, strerror(errno));
        goto fail;
    }

    strncpy(ifr.ifr_name, interface_name, IFNAMSIZ);
    if (ioctl(s, SIOCGIFINDEX, &ifr) == -1) {
        DEBUG_ERROR(" ioctl(SIOCGIFINDEX) failed, errno=%d (%s)t\n",  errno, strerror(errno));
        goto fail;
    }
    ifindex = ifr.ifr_ifindex;

    eh = (struct ether_header *)cmduBufPush(buf, sizeof(struct ether_header));
    MACCPY(&eh->ether_dhost, dst_mac);
    MACCPY(&eh->ether_shost, src_mac);
    eh->ether_type = htons(eth_type);

    memset(&socket_address, 0, sizeof(socket_address));
    socket_address.sll_ifindex = ifindex;
    socket_address.sll_halen   = ETH_ALEN;
    MACCPY(socket_address.sll_addr, dst_mac);
    socket_address.sll_addr[6] = 0x00;
    socket_address.sll_addr[7] = 0x00;

    if (-1 == sendto(s,
                     buf->data,
                     cmduBufLen(buf),
                     0,
                     (struct sockaddr*)&socket_address,
                     sizeof(socket_address))
       ) {
        cmduBufPop(buf, sizeof(struct ether_header));
        DEBUG_ERROR(" sendto failed, errorno=%d (%s)\n", errno, strerror(errno));
        goto fail;
    }
    cmduBufPop(buf, sizeof(struct ether_header));
    ret = 1;
fail:
    if (s>=0)
        close(s);
    return ret;
}

uint8_t PLATFORM_SEND_RAW_PACKET(char *interface_name, uint8_t *dst_mac, uint8_t *src_mac, uint16_t eth_type, uint8_t *payload, uint16_t payload_len)
{
    int i, first_time;
    char aux1[200];
    char aux2[10];

    int                 s;
    struct ifreq        ifr;
    int                 ifindex;
    struct sockaddr_ll  socket_address;

    uint8_t buffer[MAX_NETWORK_SEGMENT_SIZE];
    struct ether_header *eh;

    // Print packet (used for debug purposes)
    //
    DEBUG_DETAIL("[PLATFORM] Preparing to send RAW packet:\n");
    DEBUG_DETAIL("[PLATFORM]   - Interface name = %s\n", interface_name);
    DEBUG_DETAIL("[PLATFORM]   - DST  MAC       = 0x%02x:0x%02x:0x%02x:0x%02x:0x%02x:0x%02x\n", dst_mac[0], dst_mac[1], dst_mac[2], dst_mac[3], dst_mac[4], dst_mac[5]);
    DEBUG_DETAIL("[PLATFORM]   - SRC  MAC       = 0x%02x:0x%02x:0x%02x:0x%02x:0x%02x:0x%02x\n", src_mac[0], src_mac[1], src_mac[2], src_mac[3], src_mac[4], src_mac[5]);
    DEBUG_DETAIL("[PLATFORM]   - Ether type     = 0x%04x\n", eth_type);
    DEBUG_DETAIL("[PLATFORM]   - Payload length = %d\n", payload_len);

    aux1[0]    = 0x0;
    aux2[0]    = 0x0;
    first_time = 1;
    for (i=0; i<payload_len; i++)
    {
        snprintf(aux2, 6, "0x%02x ", payload[i]);
        strncat(aux1, aux2, 200-strlen(aux1)-1);

        if (0 != i && 0 == (i+1)%8)
        {
            if (1 == first_time)
            {
                DEBUG_DETAIL("[PLATFORM]   - Payload        = %s\n", aux1);
                first_time = 0;
            }
            else
            {
                DEBUG_DETAIL("[PLATFORM]                      %s\n", aux1);
            }
            aux1[0] = 0x0;
        }
    }
    if (1 == first_time)
    {
        DEBUG_DETAIL("[PLATFORM]   - Payload        = %s\n", aux1);
    }
    else
    {
        DEBUG_DETAIL("[PLATFORM]                      %s\n", aux1);
    }

    // Open RAW socket
    //
    DEBUG_DETAIL("[PLATFORM] Opening RAW socket\n");
    s = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (-1 == s)
    {
        DEBUG_ERROR("[PLATFORM] socket('%s') returned with errno=%d (%s) while opening a RAW socket\n", interface_name, errno, strerror(errno));
        return 0;
    }
  
    // Retrieve ethernet interface index
    // 
    DEBUG_DETAIL("[PLATFORM] Retrieving interface index\n");
    strncpy(ifr.ifr_name, interface_name, IFNAMSIZ);
    if (ioctl(s, SIOCGIFINDEX, &ifr) == -1)
    {
          DEBUG_ERROR("[PLATFORM] ioctl('%s',SIOCGIFINDEX) returned with errno=%d (%s) while opening a RAW socket\n", interface_name, errno, strerror(errno));
          close(s);
          return 0;
    }
    ifindex = ifr.ifr_ifindex;
    DEBUG_DETAIL("[PLATFORM] Successfully got interface index %d\n", ifindex);

    // Empy buffer
    //
    memset(buffer, 0, MAX_NETWORK_SEGMENT_SIZE);

    // Fill ethernet header
    //
    eh                 = (struct ether_header *)buffer;
    eh->ether_dhost[0] = dst_mac[0];
    eh->ether_dhost[1] = dst_mac[1];
    eh->ether_dhost[2] = dst_mac[2];
    eh->ether_dhost[3] = dst_mac[3];
    eh->ether_dhost[4] = dst_mac[4];
    eh->ether_dhost[5] = dst_mac[5];
    eh->ether_shost[0] = src_mac[0];
    eh->ether_shost[1] = src_mac[1];
    eh->ether_shost[2] = src_mac[2];
    eh->ether_shost[3] = src_mac[3];
    eh->ether_shost[4] = src_mac[4];
    eh->ether_shost[5] = src_mac[5];
    eh->ether_type     = htons(eth_type);
    
    // Fill buffer
    //
    memcpy(buffer + sizeof(*eh), payload, payload_len);

    // Prepare sockaddr_ll
    //
    memset(&socket_address, 0, sizeof(socket_address));
    socket_address.sll_ifindex  = ifindex;
    socket_address.sll_halen    = ETH_ALEN;
    socket_address.sll_addr[0]  = dst_mac[0];
    socket_address.sll_addr[1]  = dst_mac[1];
    socket_address.sll_addr[2]  = dst_mac[2];
    socket_address.sll_addr[3]  = dst_mac[3];
    socket_address.sll_addr[4]  = dst_mac[4];
    socket_address.sll_addr[5]  = dst_mac[5];
    socket_address.sll_addr[6]  = 0x00; 
    socket_address.sll_addr[7]  = 0x00;

    DEBUG_DETAIL("[PLATFORM] Sending data to RAW socket\n");
    if (-1 == sendto(s,
                     buffer,
                     sizeof(*eh) + payload_len >= 60 ? sizeof(*eh) + payload_len : 60, // 60 is the minimum ethernet frame length
                     0,
                     (struct sockaddr*)&socket_address,
                     sizeof(socket_address))
       )
    {
          DEBUG_ERROR("[PLATFORM] sendto('%s') returned with errno=%d (%s)\n", interface_name, errno, strerror(errno));
          close(s);
          return 0;
    }
    DEBUG_DETAIL("[PLATFORM] Data sent!\n");

    close(s);
    return 1;
}


#if 0
uint8_t PLATFORM_START_PUSH_BUTTON_CONFIGURATION(char *interface_name, uint8_t queue_id, uint8_t *al_mac_address, uint16_t mid)
{
    pthread_t                     thread;
    struct _pushButtonThreadData *p;
    struct interfaceInfo         *x;

    // Make sure the interface:
    //   - is not already in the middle of a "push button" configuration
    //     process and...
    //   - ...it has support for the "push button" configuration mechanism.
    //
    x = PLATFORM_GET_1905_INTERFACE_INFO(interface_name);
    if (NULL == x)
    {
        return 0;
    }
    if (0 != x->push_button_on_going)
    {
        PLATFORM_FREE_1905_INTERFACE_INFO(x);
        return 1;
    }
    PLATFORM_FREE_1905_INTERFACE_INFO(x);

    p = (struct _pushButtonThreadData *)malloc(sizeof(struct _pushButtonThreadData));
    if (NULL == p)
    {
        // Out of memory
        //
        return 0;
    }

    p->queue_id          = queue_id;
    p->interface_name    = strdup(interface_name);
    p->al_mac_address[0] = al_mac_address[0];
    p->al_mac_address[1] = al_mac_address[1];
    p->al_mac_address[2] = al_mac_address[2];
    p->al_mac_address[3] = al_mac_address[3];
    p->al_mac_address[4] = al_mac_address[4];
    p->al_mac_address[5] = al_mac_address[5];
    p->mid               = mid;

    pthread_create(&thread, NULL, _pushButtonConfigurationThread, (void *)p);

    return 1;
}
#endif

uint8_t PLATFORM_SET_INTERFACE_POWER_MODE(char *interface_name, uint8_t power_mode)
{
    // TODO
    switch(power_mode)
    {
        case INTERFACE_POWER_STATE_ON:
        {
            DEBUG_DETAIL("[PLATFORM] %s --> POWER ON\n", interface_name);
            break;
        }
        case INTERFACE_POWER_STATE_OFF:
        {
            DEBUG_DETAIL("[PLATFORM] %s --> POWER OFF\n", interface_name);
            break;
        }
        case INTERFACE_POWER_STATE_SAVE:
        {
            DEBUG_DETAIL("[PLATFORM] %s --> POWER SAVE\n", interface_name);
            break;
        }
        default:
        {
            DEBUG_WARNING("[PLATFORM] Unknown power mode for interface %s (%d)\n", interface_name, power_mode);
            return 0;
        }
    }

    return INTERFACE_POWER_RESULT_EXPECTED;
}

uint8_t PLATFORM_CONFIGURE_80211_AP(char *interface_name, uint8_t *ssid, uint8_t *bssid, uint16_t auth_type, uint16_t encryption_type, uint8_t *network_key)
{
    DEBUG_INFO("Applying WSC configuration (%s): \n", interface_name);
    DEBUG_INFO("  - SSID            : %s\n", ssid);
    DEBUG_INFO("  - BSSID           : %02x:%02x:%02x:%02x:%02x:%02x\n", bssid[0], bssid[1], bssid[2], bssid[3], bssid[4], bssid[5]);
    DEBUG_INFO("  - AUTH_TYPE       : 0x%04x\n", auth_type);
    DEBUG_INFO("  - ENCRYPTION_TYPE : 0x%04x\n", encryption_type);
    DEBUG_INFO("  - NETWORK_KEY     : %s\n", network_key);

#ifdef _FLAVOUR_ARM_WRT1900ACX_
    linksys_wrt1900acx_apply_80211_configuration(interface_name, ssid, network_key);
#else
    DEBUG_WARNING("[PLATFORM] Configuration has no effect on flavour-neutral platform\n");
#endif

    return 1;
}

int getInterfaceMac(char *name, uint8_t *mac)
{
    int fd;
    struct ifreq s;

    if ((!name) || (!mac))
        return -1;

    strncpy(s.ifr_name, name, IFNAMSIZ - 1);
    fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (fd < 0)
        return -1;

    if (0 != ioctl(fd, SIOCGIFHWADDR, &s))
    {
        DEBUG_ERROR("Couldn't get MAC address for %s\n", name);
        close(fd);
        return -1;
    }
    close(fd);
    MACCPY(mac, s.ifr_addr.sa_data);
    return 0;
}

void addLocalInterfaces(void)
{
    struct interface_list_item *item;

    dlist_for_each(item, interfaces_list, l) {
        uint8_t addr[MACLEN];
        struct interface *intf;

        if (getInterfaceMac(item->intf_name, addr))
            continue;

        intf = interfaceNew(local_device, addr, sizeof(struct interface));

        if (intf) {
            /* @todo get correct media type */
            intf->type = interface_type_ethernet;
            intf->name = strdup(item->intf_name);
            intf->power_state = power_state_on;
            intf->index = getInterfaceIndex(item->intf_name);
        }
    }
}

