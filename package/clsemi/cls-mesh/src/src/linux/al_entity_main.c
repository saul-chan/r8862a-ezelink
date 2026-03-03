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
#include "version.h"
#include "platform.h"
#include "platform_interfaces_priv.h"            // addInterface
#include "al.h"                                  // start1905AL
#include "datamodel.h"

#include <stdio.h>   // printf
#include <unistd.h>  // getopt
#include <getopt.h>
#include <stdlib.h>  // exit
#include <string.h>  // strtok

////////////////////////////////////////////////////////////////////////////////
// Static (auxiliary) private functions, structures and macros
////////////////////////////////////////////////////////////////////////////////

// Port number where the ALME server will be listening to by default
//
#define DEFAULT_ALME_SERVER_PORT 8888


// Convert a character to lower case
//
static char _asciiToLowCase (char c)
{
    if (c >= 'a' && c <= 'z')
    {
        return c;
    }
    else if (c >= 'A' && c <= 'Z')
    {
        return c + ('a' - 'A');
    }
    else
    {
        return c;
    }
}

// Convert a MAC string representation (example: "0a:fa:41:a3:ff:40") into a
// six bytes array (example: {0x0a, 0xfa, 0x41, 0xa3, 0xff, 0x40})
//
static void _asciiToMac (const char *str, uint8_t *addr)
{
    int i = 0;

    if (NULL == str)
    {
        addr[0] = 0x00;
        addr[1] = 0x00;
        addr[2] = 0x00;
        addr[3] = 0x00;
        addr[4] = 0x00;
        addr[5] = 0x00;

        return;
    }

    while (0x00 != *str && i < 6)
    {
        uint8_t byte = 0;

        while (0x00 != *str && ':' != *str)
        {
            char low;

            byte <<= 4;
            low    = _asciiToLowCase (*str);

            if (low >= 'a')
            {
                byte |= low - 'a' + 10;
            }
            else
            {
                byte |= low - '0';
            }
            str++;
        }

        addr[i] = byte;
        i++;

        if (*str == 0)
        {
            break;
        }

        str++;
      }
}

// This function receives a comma separated list of interface names (example:
// "eth0,eth1,wlan0") and, for each of them, calls "addInterface()" (example:
// addInterface("eth0") + addInterface("eth1") + addInterface("wlan0"))
//
static void _parseInterfacesList(const char *str)
{
    char *aux;
    char *interface_name;
    char *save_ptr;

    if (NULL == str)
    {
        return;
    }

    aux = strdup(str);

    interface_name = strtok_r(aux, ",", &save_ptr);
    if (NULL != interface_name)
    {
        addInterface(interface_name, 1);

        while (NULL != (interface_name = strtok_r(NULL, ",", &save_ptr)))
        {
            addInterface(interface_name, 1);
        }
    }

    free(aux);
    return;
}

static void _printUsage(char *program_name)
{
    printf("Usage: %s -m <al_mac> [-i <interfaces_list>] [-w] [-v[v...]]\n", program_name);
    printf("\n");
    printf(" where:\n");
    printf("    '<al_mac>' is the AL MAC address(ex: '00:4f:21:03:ab:0c'\n");
    printf("    '<interfaces_list>' is a comma sepparated list of local LAN interfaces\n");
    printf("    '-w', to probe whole network topology\n");
    printf("    '-v', verbose mode, add more v for more detail logs\n");
    printf("    '-p', specify EasyMesh profile level\n");
    printf("    '-c', specify external %s configuration file\n", program_name);
    printf("\n");

    return;
}


////////////////////////////////////////////////////////////////////////////////
// External public functions
////////////////////////////////////////////////////////////////////////////////
static struct option long_options[] = {
      {0}
};
static int option_index = 0;

int main(int argc, char *argv[])
{
    uint8_t al_mac_address[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t map_whole_network = 0;

    int   c;
    char *al_mac              = NULL;
    char *al_interfaces       = NULL;
    char *registrar_interface = NULL;

    int verbosity_counter = DEBUG_LVL_DEFAULT; // Only ERROR and WARNING messages

    datamodelInit();

    while ((c = getopt_long(argc, argv, "m:i:w:v::h:N:ASslnap:re",
                 long_options, &option_index)) != -1)    {
        switch (c)
        {
            case 'm':
            {
                // AL MAC address in "xx:xx:..:xx" format
                //
                al_mac = optarg;
                break;
            }
            case 'i':
            {
                // Comma sepparated list of interfaces: 'eth0,eth1,wlan0'
                //
                al_interfaces = optarg;
                break;
            }
            case 'w':
            {
                // If set to '1', the AL entity will not only query its direct
                // neighbors, but also its neighbors's neighbors and so on...
                // taking much more memory but obtaining a whole network map.
                //
                map_whole_network = 1;
                break;
            }
            case 'v':
            {
                // Each time this flag appears, the verbosity counter is
                // incremented.
                int i=0;
                verbosity_counter++;
                if (optarg) {
                    while (optarg[i++]=='v')
                        verbosity_counter++;
                }
                break;
            }
            case 'p':
            {
                local_config.profile = atoi(optarg);
                break;
            }
            case 'A':
                local_config.auto_role = 1;
                break;
            case 'S':
            {
                local_config.controller = 1;
                break;
            }
            case 's':
            {
                local_config.controller = 1;
                local_config.agent = 0;
                break;
            }
            case 'N':
            {
                local_config.ubus_prefix = optarg;
                break;
            }
            case 'n':
            {
                local_config.relay = 0;
                break;
            }
            case 'a':
            {
                local_config.listen_specific_protocol = 0;
                break;
            }
            case 'l':
            {
                local_config.lldp = 1;
                break;
            }
            case 'r':
            {
                local_config.wfa_mode = 1;
                break;
            }
            case 'e':
            {
                local_config.sync_sta = 1;
                break;
            }
            case 'h':
            {
                _printUsage(argv[0]);
                exit(0);
            }

        }
    }

    if (!al_mac)
    {
        _printUsage(argv[0]);
        exit(1);
    }
    _asciiToMac(al_mac, al_mac_address);

    printf("%s %s-%s starting(al="MACFMT")...\n", argv[0],
            CLMESH_VERSION, CLMESH_COMMIT, MACARG(al_mac_address));
#if 0
    if (0 == alme_port_number)
    {
        alme_port_number = DEFAULT_ALME_SERVER_PORT;
    }
#endif
    DEBUG_SET(debug_param_level, verbosity_counter);

    _parseInterfacesList(al_interfaces);
    //almeServerPortSet(alme_port_number);
    start1905AL(al_mac_address, map_whole_network, registrar_interface);

    DEBUG_CLOSE();
    return 0;
}
