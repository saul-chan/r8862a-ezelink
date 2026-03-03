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
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include "smac_code.h"
#include "WTPipcHostapd.h"

#ifdef DMALLOC
#include "../dmalloc-5.5.0/dmalloc.h"
#endif

#define EXIT_FRAME_THREAD(sock)	CWLog("ERROR Handling Frames: application will be closed!"); close(sock); exit(1);


#define LOCALUDP
//#define NETUDP
//#define NETSEQ

//#define USEIPV6


int address_size;

#if defined(LOCALUDP)
	struct sockaddr_un client;
#else
	#if defined(USEIPV6)
		struct sockaddr_in6 client;
	#else
		struct sockaddr_in client;
	#endif

#endif



char connected = 0;
int sock;
int event_sk;
extern int wtpInRunState;

extern pthread_mutex_t gRADIO_MAC_mutex;
pthread_mutex_t mutext_info;

unsigned char WTP_Radio_Information = 0;
unsigned char WTP_Rates[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
unsigned char WTP_MDC[6] = { 0, 0, 0, 0, 0, 0 };

extern char gRADIO_MAC[6];

int flush_pcap(u_char *buf,int len,char *filename){
	
	FILE *file;
	file = fopen(filename,"a+");
	u_char index=0x00;
	int cnt=0;
	int i;
	int giro=0;
	for(i=0;cnt<len ;i++){
		fprintf(file,"0%02X0   ",index);
		for(;cnt<len;){
			fprintf(file,"%02X ",buf[cnt]);
			cnt++;
			if(giro==15){
				giro=0;
				break;
			}
			giro++;
		}
		fprintf(file,"\n");
		index++;
	}

	fprintf(file,"\n");
	fclose(file); 
	return 0;
}


void CWWTP_get_WTP_Rates(unsigned char *buf){
	
	CWThreadMutexLock(&mutext_info);
		memcpy( buf, WTP_Rates, 8 );
	CWThreadMutexUnlock(&mutext_info);
	
}


void CWWTP_get_WTP_MDC(unsigned char *buf){
	
	CWThreadMutexLock(&mutext_info);
		memcpy( buf, WTP_MDC, 6 );
	CWThreadMutexUnlock(&mutext_info);	
	
}

unsigned char CWTP_get_WTP_Radio_Information(){
	
	unsigned char tmp_info;
	CWThreadMutexLock(&mutext_info);
		tmp_info = WTP_Radio_Information;
	CWThreadMutexUnlock(&mutext_info);	
	return tmp_info;
	
}

void CWWTPsend_data_to_hostapd(unsigned char *buf, int len){  
	
	if(!connected)return;
	
	unsigned char tmp_buf[CW_BUFFER_SIZE];
	tmp_buf[0] = DATE_TO_WTP;
	memcpy(tmp_buf + 1, buf, len);
	
	if( sendto(sock,tmp_buf,len+1,0,(struct sockaddr *)&client,address_size)<0 ){
		CWDebugLog("Error to send data frame on Unix socket");
		return;
	}
	
}

void CWWTPsend_command_to_hostapd_SET_TXQ(unsigned char *buf, int len){
	
	if(!connected)return;
	buf[0] = SET_TXQ;
	
	if( sendto(sock, buf, len, 0, (struct sockaddr *)&client, address_size)<0 ){
		CWDebugLog("Error to send command frame on Unix socket");
		return;
	}
	
}

void CWWTPsend_command_to_hostapd_SET_ADDR(unsigned char *buf, int len){ 

	if(!connected)return;
	buf[0] = SET_ADDR;
	
	if( sendto(sock, buf, len, 0, (struct sockaddr *)&client, address_size)<0 ){
		CWDebugLog("Error to send command frame on socket");
		return;
	}
	
}

void CWWTPsend_command_to_hostapd_ADD_WLAN(unsigned char *buf, int len){ 

WAITHOSTAPDADD:

	if(!connected){ 
		sleep(0.2);
		goto WAITHOSTAPDADD; 
	}
	buf[0] = ADD_WLAN;
	int i;

	if( sendto(sock, buf, len, 0, (struct sockaddr *)&client, address_size)<0 ){
		CWDebugLog("Error to send command ADD WLAN on socket");
		return;
	}
	
}

void CWWTPsend_command_to_hostapd_DEL_WLAN(unsigned char *buf, int len){ 
	
WAITHOSTAPDDEL:

	if(!connected){ 
		sleep(0.2);
		goto WAITHOSTAPDDEL; 
	}
	buf[0] = DEL_WLAN;
	int i;
	
	if( sendto(sock, buf, len, 0, (struct sockaddr *)&client, address_size)<0 ){
		CWLog("Error to send command DEL WLAN on socket");
		return;
	}
	
}

void CWWTPsend_command_to_hostapd_DEL_ADDR(unsigned char *buf, int len){ 

	if(!connected)return;
	buf[0] = DEL_ADDR;
	
	if( sendto(sock, buf, len, 0, (struct sockaddr *)&client, address_size)<0 ){
		CWLog("Error to send command frame on socket");
		return;
	}
	
}

void CWWTPsend_command_to_hostapd_CLOSE(unsigned char *buf, int len){ 

	buf[0] = CLOSE;
	
	if( sendto(sock, buf, len, 0, (struct sockaddr *)&client, address_size)<0 ){
		CWLog("Error to send command frame on socket");
		return;
	}
	
}

#if 0
CW_THREAD_RETURN_TYPE CWWTPThread_read_data_from_hostapd(void *arg){
	
	/*
	CWThreadMutexLock(&gRADIO_MAC_mutex);
		gRADIO_MAC[0]=0xAA;
		gRADIO_MAC[1]=0xBB;
		gRADIO_MAC[2]=0xCC;
		gRADIO_MAC[3]=0xDD;
		gRADIO_MAC[4]=0xEE;
		gRADIO_MAC[5]=0xFF;
	CWThreadMutexUnlock(&gRADIO_MAC_mutex);
	*/
	
	int len;
        
    #if defined(LOCALUDP)
		struct sockaddr_un server;
    #else
        #if defined(USEIPV6)
			struct sockaddr_in6 server;
		#else
			struct sockaddr_in server;
		#endif
    #endif
    
	unsigned char buffer[CW_BUFFER_SIZE];
	int connect_ret;
	int flags;
	int n;
	char cmd[10];
	
	CWProtocolMessage* frame = NULL;
	CWBindingDataListElement* listElement = NULL;
	
	CWThreadSetSignals(SIG_BLOCK, 1, SIGALRM);

	#if defined(LOCALUDP)
		sock = socket(AF_UNIX, SOCK_DGRAM, 0);

	#elif defined(NETUDP)
		#if defined(USEIPV6)
			bzero(&server,sizeof(server));
			sock = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
		#else
			memset(&server, 0, sizeof(server));
			sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		#endif

	#else
		#if defined(USEIPV6)
			bzero(&server,sizeof(server));
			sock = socket(AF_INET6, SOCK_SEQPACKET, IPPROTO_SCTP);
		#else
			memset(&server, 0, sizeof(server));
			sock = socket(AF_INET, SOCK_SEQPACKET, IPPROTO_SCTP);
		#endif
		
	#endif

    if (sock < 0) {
		CWDebugLog("WTP ipc HOSTAPD: Error creating socket");
		EXIT_FRAME_THREAD(sock);
    }

    CWDebugLog("WTP ipc HOSTAPD: Trying to connect to hostapd (wtp)...");

	#if defined(LOCALUDP)
		server.sun_family = AF_UNIX;
		strcpy(server.sun_path, gHostapd_unix_path);
		unlink(server.sun_path);
		
		connect_ret = bind(sock, (struct sockaddr *)&server, strlen(server.sun_path) + sizeof(server.sun_family));
		
		client.sun_family=AF_UNIX;

    #else
		#if defined(USEIPV6)
			server.sin6_family = AF_INET6;
			server.sin6_port = gHostapd_port;
			server.sin6_addr = in6addr_any;
		#else
			server.sin_family = AF_INET;
			server.sin_port = gHostapd_port;
			server.sin_addr.s_addr = INADDR_ANY;
		#endif
		connect_ret = bind(sock,(struct sockaddr *)&server,sizeof(server));
		
    #endif
 
    
	if (connect_ret == -1) {
		CWDebugLog("WTP ipc HOSTAPD: Error connect/bind to socket");
		EXIT_FRAME_THREAD(sock);
	}

	#if defined(LOCALUDP)

	#elif defined(NETUDP)
	
	#else
		/* 1: Only one daemon Hostapd_WTP at time */
		if (listen(sock, 1) < 0){
			CWDebugLog("WTP ipc HOSTAPD: Error listen ");
			EXIT_FRAME_THREAD(sock);
		}
	#endif
	
	
	#if defined(LOCALUDP)
		CWDebugLog("Waiting packet from Hostapd_WTP at Pipe:%s",gHostapd_unix_path);
	#else
		CWDebugLog("Waiting packet from Hostapd_WTP at Port:%d",gHostapd_port);
	#endif

	
	address_size = sizeof(client);
	
	int sig_byte = 1;
	
	#if defined(LOCALUDP)
		sig_byte += 5;
	#endif
	
 	CW_REPEAT_FOREVER {
		
		len = recvfrom(sock,buffer,sizeof(buffer),0,(struct sockaddr *)&client,&address_size); 
		
		#if defined(LOCALUDP)
			sprintf(client.sun_path, "%s%c%c%c%c%c", server.sun_path, buffer[1], buffer[2], buffer[3], buffer[4], buffer[5]);
		#endif	
		

		if(len <= 0 ) {	EXIT_FRAME_THREAD(sock)	}
		
		if(connected==0 && buffer[0]!=CONNECT ){	
			CWLog("WTP is not in RUN state"); 	
			CWWTPsend_command_to_hostapd_CLOSE(cmd,10);
			continue;	
		}
		
		if( buffer[0] == DATE_TO_AC ){

			if(!wtpInRunState) continue;
			
			if (!extract802_11_Frame(&frame, buffer+sig_byte, len-sig_byte)){
				CWLog("THR FRAME: Error extracting a frame");
				EXIT_FRAME_THREAD(sock);
			}
				
			CWDebugLog("Send 802.11 management(len:%d) to AC",len-1);

			CW_CREATE_OBJECT_ERR(listElement, CWBindingDataListElement, EXIT_FRAME_THREAD(sock););
			listElement->frame = frame;
			listElement->bindingValues = NULL;
				
			listElement->frame->data_msgType = CW_IEEE_802_11_FRAME_TYPE;

			CWLockSafeList(gFrameList);
			CWAddElementToSafeListTail(gFrameList, listElement, sizeof(CWBindingDataListElement));
			CWUnlockSafeList(gFrameList);

			
		}else if( buffer[0]==CONNECT ){
			
			connected = 1;
			cmd[0] = CONNECT_R;
			sendto(sock, cmd, 1, 0, (struct sockaddr *)&client, address_size);
			
			#if defined(LOCALUDP)
				CWDebugLog("Hostapd_wtp Unix Domain Connect: %s",client.sun_path);
			#else
				#if defined(USEIPV6)
					CWDebugLog("Hostapd_wtp (v6) Connect: %d",client.sin6_port);
				#else
					CWDebugLog("Hostapd_wtp (v4) Connect: %s:%d",inet_ntoa(client.sin_addr), client.sin_port);
				#endif
			#endif
			
			cmd[0] = WTPRINFO; //Next info to get
			sendto(sock, cmd, 1, 0, (struct sockaddr *)&client, address_size);
		
		
		}else if( buffer[0]==WTPRINFO_R ){
			
			CWThreadMutexLock(&mutext_info);
				memcpy( &WTP_Radio_Information, buffer + sig_byte, 1);
			CWThreadMutexUnlock(&mutext_info);
			
			CWDebugLog("WTPRINFO_R:  %02X",WTP_Radio_Information);
			
			cmd[0] = GET_RATES; //Next info to get
			sendto(sock, cmd, 1, 0, (struct sockaddr *)&client, address_size);
		
		
		}else if( buffer[0]==GET_RATES_R){
			
			CWThreadMutexLock(&mutext_info);
				memcpy( WTP_Rates, buffer + sig_byte, 8);
			CWThreadMutexUnlock(&mutext_info);
			
			CWDebugLog("GET_RATES_R:   %02X  %02X  %02X  %02X  %02X  %02X  %02X  %02X",WTP_Rates[0],WTP_Rates[1],WTP_Rates[2],WTP_Rates[3],WTP_Rates[4],WTP_Rates[5],WTP_Rates[6],WTP_Rates[7]);
			
			cmd[0] = GET_MDC; //Next info to get
			sendto(sock, cmd, 1, 0, (struct sockaddr *)&client, address_size);
				
		
		}else if( buffer[0]==GET_MDC_R){
			
			
			CWThreadMutexLock(&mutext_info);
				memcpy( WTP_MDC, buffer + sig_byte, 6);
			CWThreadMutexUnlock(&mutext_info);
			
			CWDebugLog("GET_MDC_R: %02X  %02X  %02X  %02X  %02X  %02X",
										WTP_MDC[0],
										WTP_MDC[1],
										WTP_MDC[2],
										WTP_MDC[3],
										WTP_MDC[4],
										WTP_MDC[5]);
			
			
			cmd[0] = GET_MAC;
			sendto(sock, cmd, 1, 0, (struct sockaddr *)&client, address_size);
		
		}else if( buffer[0]==GET_MAC_R){
				
			
			CWThreadMutexLock(&gRADIO_MAC_mutex);
				memcpy( gRADIO_MAC, buffer + sig_byte, 6);
			CWThreadMutexUnlock(&gRADIO_MAC_mutex);
			
			CWDebugLog("GET_MAC_R:   %02X  %02X  %02X  %02X  %02X  %02X",
									(unsigned char)gRADIO_MAC[0],
									(unsigned char)gRADIO_MAC[1],
									(unsigned char)gRADIO_MAC[2],
									(unsigned char)gRADIO_MAC[3],
									(unsigned char)gRADIO_MAC[4],
									(unsigned char)gRADIO_MAC[5]);
		
			cmd[0] = GOWAITWLAN;
			sendto(sock, cmd, 1, 0, (struct sockaddr *)&client, address_size);
						
			
		}else if( buffer[0]==CLOSE ){
			
			connected = 0;
			#if defined(LOCALUDP)
				CWDebugLog("Hostapd_wtp Unix Domain DisConnect: %s",client.sun_path);
			#else
				#if defined(USEIPV6)
					CWDebugLog("Hostapd_wtp (v6) DisConnect: %d",client.sin6_port);
				#else
					CWDebugLog("Hostapd_wtp (v4) Disconnect: %s:%d",inet_ntoa(client.sin_addr), client.sin_port);
				#endif
			#endif
		
		}else if( buffer[0]==SET_TXQ_R ){
			
			CWDebugLog("Hostapd WTP \"SET_TXQ_R\" Command\n");
			
		}else if( buffer[0]==GOWAITWLAN_R ){

			CWDebugLog("Hostapd WTP in WAIT \"ADD WLAN\" Command\n");

		}else{

			CWDebugLog("Received Unknow Command from Hostapd WTP(%d)",buffer[0]);
		}

 	}
 	
	close(sock);
	return(NULL);
}
#endif
CW_THREAD_RETURN_TYPE CWWTPThread_read_data_from_hostapd(struct WTPBSSInfo *BSSInfo) {
	int len;
#if defined(LOCALUDP)
	struct sockaddr_un server;
	char file[CW_NAME_LEN] = {0};
#else
	#if defined(USEIPV6)
		struct sockaddr_in6 server;
	#else
		struct sockaddr_in server;
	#endif
#endif
	unsigned char buffer[CW_BUFFER_SIZE];
	int connect_ret;
	CWBool ret;

	CWThreadSetSignals(SIG_BLOCK, 1, SIGALRM);

	if (!BSSInfo->interfaceInfo->ifName) {
		CWDebugLog("WTP ipc HOSTAPD: Interface name is NULL");
		EXIT_FRAME_THREAD(BSSInfo->ipcDataSock);
	}

#if defined(LOCALUDP)
	BSSInfo->ipcDataSock = socket(AF_UNIX, SOCK_DGRAM, 0);
#elif defined(NETUDP)
	#if defined(USEIPV6)
		bzero(&server,sizeof(server));
		BSSInfo->ipcDataSock = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
	#else
		memset(&server, 0, sizeof(server));
		BSSInfo->ipcDataSock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	#endif
#else
	#if defined(USEIPV6)
		bzero(&server,sizeof(server));
		BSSInfo->ipcDataSock = socket(AF_INET6, SOCK_SEQPACKET, IPPROTO_SCTP);
	#else
		memset(&server, 0, sizeof(server));
		BSSInfo->ipcDataSock = socket(AF_INET, SOCK_SEQPACKET, IPPROTO_SCTP);
	#endif
#endif

	if (BSSInfo->ipcDataSock < 0) {
		CWDebugLog("WTP ipc HOSTAPD: Error creating socket");
		EXIT_FRAME_THREAD(BSSInfo->ipcDataSock);
	}

	CWDebugLog("WTP ipc HOSTAPD: Trying to connect to hostapd (wtp)...");

#if defined(LOCALUDP)
	sprintf(file, "%s_%s", gHostapd_unix_path, BSSInfo->interfaceInfo->ifName);
	if (access(file, F_OK) != -1) {
		CWDebugLog("file %s existed unexcepted, delete it first\n", file);
		unlink(file);
	}
	server.sun_family = AF_UNIX;
	strcpy(server.sun_path, file);
	unlink(server.sun_path);

	connect_ret = bind(BSSInfo->ipcDataSock, (struct sockaddr *)&server, strlen(server.sun_path) + sizeof(server.sun_family));

	client.sun_family=AF_UNIX;
#else
	#if defined(USEIPV6)
		server.sin6_family = AF_INET6;
		server.sin6_port = gHostapd_port;
		server.sin6_addr = in6addr_any;
	#else
		server.sin_family = AF_INET;
		server.sin_port = gHostapd_port;
		server.sin_addr.s_addr = INADDR_ANY;
	#endif
	connect_ret = bind(BSSInfo->ipcDataSock,(struct sockaddr *)&server,sizeof(server));
#endif

#if defined(LOCALUDP)
	if (connect_ret == -1) {
		CWDebugLog("WTP ipc HOSTAPD: Error connect/bind to socket");
		if (connect(BSSInfo->ipcDataSock, (struct sockaddr *) &server, sizeof(server)) < 0) {
			CWDebugLog("data iface exists, but does not allow connections - assuming it was left"
				"over from forced program termination");
			if (unlink(file) < 0) {
				CWDebugLog("Could not unlink existing data iface socket '%s': %s", file, strerror(errno));
				EXIT_FRAME_THREAD(BSSInfo->ipcDataSock);
			}
			if (bind(BSSInfo->ipcDataSock, (struct sockaddr *) &server, sizeof(server)) < 0) {
				CWDebugLog("hostapd data iface: bind(PF_UNIX): %s", strerror(errno));
				EXIT_FRAME_THREAD(BSSInfo->ipcDataSock);
			}
			CWDebugLog("Successfully replaced leftover data iface socket '%s'", file);
		} else {
			CWDebugLog("data iface exists and seems to be in use - cannot override it");
			CWDebugLog("Delete '%s' manually if it is not used anymore", file);
			EXIT_FRAME_THREAD(BSSInfo->ipcDataSock);
		}
	}
#endif
#if defined(LOCALUDP)

#elif defined(NETUDP)

#else
	/* 1: Only one daemon Hostapd_WTP at time */
	if (listen(BSSInfo->ipcDataSock, 1) < 0){
		CWDebugLog("WTP ipc HOSTAPD: Error listen ");
		EXIT_FRAME_THREAD(BSSInfo->ipcDataSock);
	}
#endif

#if defined(LOCALUDP)
	CWDebugLog("Waiting packet from Hostapd_WTP at Pipe:%s", file);
#else
	CWDebugLog("Waiting packet from Hostapd_WTP at Port:%d", gHostapd_port);
#endif

	address_size = sizeof(client);
	CW_REPEAT_FOREVER {
		len = recvfrom(BSSInfo->ipcDataSock, buffer, sizeof(buffer), 0, (struct sockaddr *)&client, &address_size);
		if(len <= 0 ) {
			EXIT_FRAME_THREAD(BSSInfo->ipcDataSock);
		}
		CWLog("Recevie frame from hostapd: len=%d\r\n", len);
		CWDump(buffer, len);

		if(!wtpInRunState){
			CWLog("WTP is not in RUN state");
			ret = CW_FALSE;
			goto send_resp;
		}
		ret = CWHandleFrameMgmt(BSSInfo, buffer, len);
send_resp:
		if (ret == CW_TRUE) {
			CWDebugLog("Handle frame successfully, send response");
			connect_ret = sendto(BSSInfo->ipcDataSock, RESPONSE_SUCCESS, strlen(RESPONSE_SUCCESS), 0, (struct sockaddr *) &client, address_size);
		} else {
			CWDebugLog("Handle frame failed, send response");
			connect_ret = sendto(BSSInfo->ipcDataSock, RESPONSE_FAIL, strlen(RESPONSE_FAIL), 0, (struct sockaddr *) &client, address_size);
		}
		if (connect_ret < 0)
			CWDebugLog("send response failed: %s", strerror(errno));
		else
			CWDebugLog("Send response successfully\r\n");
	}

	close(BSSInfo->ipcDataSock);
	return(NULL);
}

CWBool CWWTPipcHostapdThread(int radioIndex, int wlanIndex, WTPInterfaceInfo *interfaceInfo)
{
	int BSSId = 0;

	if(interfaceInfo == NULL)
		return CW_TRUE;

	if(interfaceInfo->typeInterface != CW_AP_MODE) {
		CWDebugLog("Interface %s is %d, needn't create thread.", interfaceInfo->ifName, interfaceInfo->typeInterface);
		return CW_TRUE;
	}

	//BSSID == AP Address
	if (interfaceInfo->BSSID == NULL)
		CW_CREATE_ARRAY_CALLOC_ERR(interfaceInfo->BSSID, ETH_ALEN+1, char, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
	CW_COPY_MEMORY(interfaceInfo->BSSID, interfaceInfo->MACaddr, ETH_ALEN);

	BSSId = getBSSIndex(radioIndex, wlanIndex);
	WTPGlobalBSSList[BSSId]->active = CW_TRUE;

	if ((WTPGlobalBSSList[BSSId]->threadBSS > 0) && (WTPGlobalBSSList[BSSId]->ipcDataSock > 0)) {
		CWDebugLog("WTP ipc HOSTAPD: already created: threadid=%d, ipcDataSock=%d", WTPGlobalBSSList[BSSId]->threadBSS,
			WTPGlobalBSSList[BSSId]->ipcDataSock);
		return CW_TRUE;
	} else {
		if (WTPGlobalBSSList[BSSId]->ipcDataSock > 0) {
			CWDebugLog("WTP ipc HOSTAPD: close previous socket");
			close(WTPGlobalBSSList[BSSId]->ipcDataSock);
			WTPGlobalBSSList[BSSId]->ipcDataSock = 0;
		}
		if (WTPGlobalBSSList[BSSId]->threadBSS > 0) {
			CWCancelThread(WTPGlobalBSSList[BSSId]->threadBSS);
			WTPGlobalBSSList[BSSId]->threadBSS = 0;
		}
	}
	CWDebugLog("Create thread for AP interface to handle management frame: interface=%s", interfaceInfo->ifName);
	if(!CWErr(CWCreateThread(&(WTPGlobalBSSList[BSSId]->threadBSS), CWWTPThread_read_data_from_hostapd, WTPGlobalBSSList[BSSId]))) {
		CWDebugLog("Error starting Thread that receive binding frame");
		exit(1);
	}

	return CW_TRUE;
}

CW_THREAD_RETURN_TYPE wtp_event_server() {
	int client_socket;
	struct sockaddr_un server;
	socklen_t addrlen = sizeof(server);
	struct sockaddr_un from;
	socklen_t fromlen;
	struct hostapd_wtp_event_msg event_msg;
	int len;

	event_sk = socket(PF_UNIX, SOCK_DGRAM, 0);
	if (event_sk < 0) {
		CWDebugLog("WTP ipc HOSTAPD: Error creating socket");
		goto fail;
	}

	memset(&server, 0, sizeof(server));
	server.sun_family = AF_UNIX;
	strcpy(server.sun_path, WTP_EVENT_SERVER_PATH);

	if (bind(event_sk, (struct sockaddr *) &server, sizeof(server)) < 0) {
		CWDebugLog("event socket bind(PF_UNIX) failed: %s", strerror(errno));
		if (connect(event_sk, (struct sockaddr *) &server, sizeof(server)) < 0) {
			CWDebugLog("event socket exists, but does not allow connections - assuming it was left"
				   "over from forced program termination");
			if (unlink(WTP_EVENT_SERVER_PATH) < 0) {
				CWDebugLog("Could not unlink existing event socket '%s': %s", WTP_EVENT_SERVER_PATH, strerror(errno));
				goto fail;
			}
			if (bind(event_sk, (struct sockaddr *) &server, sizeof(server)) <
			    0) {
				CWDebugLog("hostapd event socket: bind(PF_UNIX): %s", strerror(errno));
				goto fail;
			}
			CWDebugLog("Successfully replaced leftover event socket '%s'", WTP_EVENT_SERVER_PATH);
		} else {
			CWDebugLog("event socket exists and seems to be in use - cannot override it");
			CWDebugLog("Delete '%s' manually if it is not used anymore", WTP_EVENT_SERVER_PATH);
			goto fail;
		}
	}

	if (chmod(WTP_EVENT_SERVER_PATH, S_IRWXU | S_IRWXG) < 0) {
		CWDebugLog("chmod %s: %s", WTP_EVENT_SERVER_PATH, strerror(errno));
		goto fail;
	}

	CW_REPEAT_FOREVER {
		fromlen = sizeof(from);
		memset(&event_msg, 0, sizeof(event_msg));
		len = recvfrom(event_sk, &event_msg, sizeof(event_msg), 0, (struct sockaddr *)&from, &fromlen);
		if (len < 0) {
			CWDebugLog("recvfrom failed: %s", strerror(errno));
			continue;
		}
		CWDebugLog("recevie event from hostapd: client.family=%d, client.path=%s", from.sun_family, from.sun_path);
		if (event_msg.event == HOSTAPD_WTP_INTERFACE_INIT) {
			CWDebugLog("recevie HOSTAPD_WTP_INTERFACE_INIT event");
			if (strlen(event_msg.data) == 0) {
				if (!gRadiosInit) {
					if(CWWTPGetRadioGlobalInfo() == CW_FALSE)
						return CW_FALSE;
				}
				CWWTPInitRadio();
			} else
				CWWTPUpdateInterface(event_msg.data);
		} else if (event_msg.event == HOSTAPD_WTP_STA_DISCONNECTED) {
			CWWTPHandleSTADisconnectEvent(event_msg.data);
		} else {
			CWDebugLog("unsupported event: %d", event_msg.event);
			continue;
		}
		if (sendto(event_sk, RESPONSE_SUCCESS, strlen(RESPONSE_SUCCESS), 0, (struct sockaddr *) &from, fromlen) < 0)
			CWDebugLog("send event response failed: %s", strerror(errno));
		else
			CWDebugLog("send event response successfully");
	}

fail:
	if (event_sk >= 0)
		close(event_sk);
	unlink(WTP_EVENT_SERVER_PATH);
	return NULL;
}

int wtp_eapol_connect(struct hostapd_eapol_conn *eapol_conn, char *addr) {
	char cfile[MAX_FILE_PATH] = {0};
	char sfile[MAX_FILE_PATH] = {0};
	int flags = 0;
	int conn_cnt = 0;

	eapol_conn->sock = socket(AF_UNIX, SOCK_DGRAM, 0);
	if (eapol_conn->sock == -1)
		return -1;

	memset(&(eapol_conn->local), 0, sizeof(struct sockaddr_un));
	eapol_conn->local.sun_family = AF_UNIX;

	sprintf(cfile, "%s_%02x%02x%02x%02x%02x%02x", WTP_STA_EAPOL_PATH, addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
	if (access(cfile, F_OK) != -1) {
		CWDebugLog("file %s existed unexcepted, delete it first\n", cfile);
		unlink(cfile);
	}
	strncpy(eapol_conn->local.sun_path, cfile, sizeof(eapol_conn->local.sun_path)-1);

	/* Set client socket file permissions so that bind() creates the client
	 * socket with these permissions and there is no need to try to change
	 * them with chmod() after bind() which would have potential issues with
	 * race conditions. These permissions are needed to make sure the server
	 * side (wpa_supplicant or hostapd) can reply to the control interface
	 * messages.
	 *
	 * The lchown() calls below after bind() are also part of the needed
	 * operations to allow the response to go through. Those are using the
	 * no-deference-symlinks version to avoid races.
	 */
	fchmod(eapol_conn->sock, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
	if (bind(eapol_conn->sock, (struct sockaddr *)&eapol_conn->local, sizeof(eapol_conn->local)) < 0)
		goto fail;
	if (lchown(eapol_conn->local.sun_path, -1, 101) == -1)
		CWDebugLog("set group failed by lchown\n");
	if (lchown(eapol_conn->local.sun_path, 101, 101) == -1)
		CWDebugLog("set owner and group failed by lchown\n");

	sprintf(sfile, "%s_%02x%02x%02x%02x%02x%02x", STA_EAPOL_PATH, addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
	memset(&(eapol_conn->dest), 0, sizeof(struct sockaddr_un));
	eapol_conn->dest.sun_family = AF_UNIX;
	strncpy(eapol_conn->dest.sun_path, sfile, sizeof(eapol_conn->dest.sun_path)-1);

try_again:
	if (connect(eapol_conn->sock, (struct sockaddr *)&(eapol_conn->dest), sizeof(eapol_conn->dest)) == -1) {
		conn_cnt++;
		if (conn_cnt < EAPOL_CONNECT_MAX_RETRY) {
			usleep(50000);
			goto try_again;
		} else {
			CWDebugLog("connect to hostapd eapol failed %s\n", strerror(errno));
			unlink(eapol_conn->local.sun_path);
			goto fail;
		}
	}
	/*
	 * Make socket non-blocking so that we don't hang forever if
	 * target dies unexpectedly.
	 */
	flags = fcntl(eapol_conn->sock, F_GETFL);
	if (flags >= 0) {
		flags |= O_NONBLOCK;
		if (fcntl(eapol_conn->sock, F_SETFL, flags) < 0)
			CWDebugLog("set socket to O_NONBLOCK\n");
	}
	CWDebugLog("connect to hostapd eapol successfully\n");
	return 0;
fail:
	CWDebugLog("connect to hostapd eapol failed\n");
	if (eapol_conn->sock != -1)
		close(eapol_conn->sock);
	return -1;
}

int wtp_send_recv_eapol(int sock, const u8 *data, size_t len, char *res_data, int *res_len) {
	int ret = -1;
	int retry_cnt = 0;
	char resp[LEN_EAPOL_MAX] = {0};

	CWDump(data, len);
	if (sock <= 0)
		return -1;
retry_send:
	ret = send(sock, data, len, 0);
	retry_cnt++;
	if (ret < 0) {
		if (errno == EAGAIN || errno == EBUSY || errno == EWOULDBLOCK) {
			/*No buffer, wait for 50ms and try again, max three times*/
			if (retry_cnt < EAPOL_CONNECT_MAX_RETRY) {
				usleep(50000);
				goto retry_send;
			}
		}
		CWDebugLog("send frame to hostapd failed: errno=%d\n", errno);
		return ret;
	}
	CWDebugLog("send data successfully\n");
	retry_cnt = 0;

retry_recv:
	ret = recv(sock, &resp, sizeof(resp)-1, 0);
	retry_cnt++;
	if (ret < 0) {
		if (errno == EAGAIN || errno == EWOULDBLOCK) {
			if (retry_cnt < 2*EAPOL_CONNECT_MAX_RETRY) {
				usleep(100000);
				goto retry_recv;
			}
		}
		CWDebugLog("receive response from hostapd failed: %s", strerror(errno));
		return ret;
	}
	CWDebugLog("receive response from hostapd success: len=%d\n", ret);
	*res_len = ret;
	if (ret > 0) {
		CWDump(resp, ret);
		CW_COPY_MEMORY(res_data, resp, ret);
	}
	return 0;
}

