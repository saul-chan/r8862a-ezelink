/*******************************************************************************************
 * Copyright (c) 2006-7 Laboratorio di Sistemi di Elaborazione e Bioingegneria Informatica *
 *                      Universita' Campus BioMedico - Italy                               *
 *                                                                                         *
 * This program is free software; you can redistribute it and/or modify it under the terms *
 * of the GNU General Public License as published by the Free Software Foundation; either  *
 * version 2 of the License, or (at your option) any later version.                        *
 *                                                                                         *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY         *
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A 	       *
 * PARTICULAR PURPOSE. See the GNU General Public License for more details.                *
 *                                                                                         *
 * You should have received a copy of the GNU General Public License along with this       *
 * program; if not, write to the:                                                          *
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,                    *
 * MA  02111-1307, USA.                                                                    *
 *                                                                                         *
 * In addition, as a special exception, the copyright holders give permission to link the  *
 * code of portions of this program with the OpenSSL library under certain conditions as   *
 * described in each individual source file, and distribute linked combinations including  * 
 * the two. You must obey the GNU General Public License in all respects for all of the    *
 * code used other than OpenSSL.  If you modify file(s) with this exception, you may       *
 * extend this exception to your version of the file(s), but you are not obligated to do   *
 * so.  If you do not wish to do so, delete this exception statement from your version.    *
 * If you delete this exception statement from all source files in the program, then also  *
 * delete it here.                                                                         *
 * 
 * --------------------------------------------------------------------------------------- *
 * Project:  Capwap                                                                        *
 *                                                                                         *
 * Author :  Sotiraq Sima (Sotiraq.Sima@gmail.com)                                         *  
 *                                                                                         *
 *******************************************************************************************/

#ifndef __CAPWAP_WTPipcHostapd_HEADER__
#define __CAPWAP_WTPipcHostapd_HEADER__

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "CWWTP.h"
#include "IEEEBinding.h"

#define WTP_EVENT_SERVER_PATH "/var/run/wtp/wtp_hostapd_event"
#define WTP_DATA_SERVER_PATH "/var/run/wtp/wtp_hostapd_data"
#define WTP_STA_EAPOL_PATH "/var/run/wtp/wtp_eapol"
#define STA_EAPOL_PATH "/var/run/wtp/hostapd_eapol"

#define EAPOL_CONNECT_MAX_RETRY 5
#define MAX_FILE_PATH 64

#define RESPONSE_SUCCESS "success"
#define RESPONSE_FAIL "fail"

struct hostapd_wtp_event_msg {
    int event;
    char data[256];
};

enum wtp_ipc_event{
	HOSTAPD_WTP_INTERFACE_INIT = 1,
	HOSTAPD_WTP_STA_DISCONNECTED
};

CW_THREAD_RETURN_TYPE CWWTPThread_read_data_from_hostapd(struct WTPBSSInfo *BSSInfo);
CWBool CWWTPipcHostapdThread(int radioIndex, int wlanIndex, WTPInterfaceInfo *interfaceInfo);
CW_THREAD_RETURN_TYPE wtp_event_server();
int wtp_eapol_connect(struct hostapd_eapol_conn *eapol_conn, char *addr);
int wtp_send_recv_eapol(int sock, const u8 *data, size_t len, char *res_data, int *res_len);

#endif

