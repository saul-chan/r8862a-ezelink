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
#include "platform_os.h"
#include "1905_l2.h"
#include "packet_tools.h"
#include "al_msg.h"

#include <stdio.h>
#include <stdlib.h>      // free(), malloc(), ...
#include <string.h>      // memcpy(), memcmp(), ...
#include <pthread.h>     // threads and mutex functions
#include <mqueue.h>      // mq_*() functions
#include <errno.h>       // errno
#include <poll.h>        // poll()
#include <sys/inotify.h> // inotify_*()
#include <unistd.h>      // read(), sleep()
#include <signal.h>
#include <sys/socket.h>  // recv(), setsockopt()
#include <linux/if_packet.h>
#include <net/ethernet.h> /* the L2 protocols */
#include <linux/filter.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/un.h>
////////////////////////////////////////////////////////////////////////////////
// Private functions, structures and macros

#define DEFAULT_QUEUE_BUFSIZE 8192

struct _linux_interface_info {
    int if_index;
    int fd_1905;
    int fd_lldp;
    pthread_t   thread;
    uint8_t al_mac[MACLEN];
    uint8_t interface_mac[MACLEN];
    char *interface_name;
};

struct _platform_queue {
    struct uloop_fd uloop_fd;
    const char *name;
    uint8_t *buf;
    uint32_t buf_len;
    uint16_t capacity;  // queue capacity default:127
    uint16_t size;      // current queue size
    pthread_mutex_t lock;
    int rd;
    int sd;
    void (*handler)(void *, uint8_t *, uint32_t);
    void *data;
};

struct _platform_timer {
    struct uloop_timeout to;
    void (*handler)(void *);
    void *data;
    uint32_t timeout;
    uint8_t flag;
};

struct _platform_task {
    pthread_t pthread;
};

#if 1
static struct sock_filter bpfcode_1905[8] = {
    //./tcpdump -dd 'ether proto 0x893a and !ether src a1:b2:c3:d4:e5:f6' -s 0
    { 0x28, 0, 0, 0x0000000c },
    { 0x15, 0, 5, 0x0000893a },
    { 0x20, 0, 0, 0x00000008 },
    { 0x15, 0, 2, 0xc3d4e5f6 },
    { 0x28, 0, 0, 0x00000006 },
    { 0x15, 1, 0, 0x0000a1b2 },
    { 0x6, 0, 0, 0x0000ffff },
    { 0x6, 0, 0, 0x00000000 },
};
static struct sock_fprog filter_1905 = {8, bpfcode_1905};
#endif
static void _processPacket(struct _linux_interface_info *info, uint8_t *packet, size_t packet_len)
{
    uint8_t   *interface_mac_address = info->interface_mac;
    uint32_t   interface_index = info->if_index;

    uint8_t *msg = packet-6; //family(1)+evt(1)+packethdr(4)
    uint8_t *p;
    uint16_t ether_type, value;

    if ((packet_len > MAX_NETWORK_SEGMENT_SIZE) || (packet_len < 2 * MACLEN+2)) {
        DEBUG_ERROR("Drop packet with invalid size(len=%d)!\n", (uint32_t)packet_len);
        return;
    }

    p = packet+12;
    _E2B(&p, &ether_type);

    if (MACCMP(packet, DMalMacGet())
            && MACCMP(packet, (uint8_t *)MCAST_1905)
            && MACCMP(packet, (uint8_t *)MCAST_LLDP)
            && MACCMP(packet, interface_mac_address))
    {
        DEBUG_INFO("Drop Packet not for us(intf %s, dst=" MACFMT " src=" MACFMT "!\n",
                                    info->interface_name, MACARG(packet), MACARG(packet+6));
        return;
    }

    if ((ether_type!=ETHERTYPE_1905) && (ether_type!=ETHERTYPE_LLDP)) {
        DEBUG_INFO("Drop packet with ethertype 0x%x\n", ether_type);
        return;
    }

    p = msg;

    MSG_PUT_HEADER(p, msg_family_driver_evt, drv_evt_packet);

    value = attr_packet; _I2B(&value, &p);
    value = packet_len;  _I2B(&value, &p);
    p+=packet_len;

    p = msgaPutU32(p, attr_if_idx, interface_index);
    p = msgaPutU16(p, attr_ethertype, ether_type);

    if (msgSend(msg, (uint16_t)(p-msg))) {
        DEBUG_ERROR("Error sending new 1905 message to queue\n");
    }
}

static int _openSocket(int if_index, uint16_t eth_type)
{
    int                 s;
    struct sockaddr_ll  s_addr;

    s = socket(AF_PACKET, SOCK_RAW, htons(eth_type));
    if (s==-1) {
        return -1;
    }
    memset(&s_addr, 0, sizeof(s_addr));
    s_addr.sll_family   = AF_PACKET;
    s_addr.sll_ifindex  = if_index;
    s_addr.sll_protocol = htons(eth_type);

    if (bind(s, (struct sockaddr*)&s_addr, sizeof(s_addr))==-1) {
        close(s);
        return -1;
    }
    return s;
}

static int _openUnixSocket(const char *name)
{
    int fd;
    struct sockaddr_un addr = {0};

    if ((fd = socket(PF_UNIX, SOCK_DGRAM, 0))<0) {
        DEBUG_ERROR("[PLATFORM]socket(%s) open failed errno=%d(%s)\n", name, errno, strerror(errno));
        return fd;
    }
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, name, sizeof(addr.sun_path) - 1);
    unlink(addr.sun_path);

    if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        DEBUG_ERROR("[PLATFORM]socket(%s) bind failed errno=%d(%s)\n", name, errno, strerror(errno));
        close(fd);
        fd = -1;
    }
    return fd;
}

static int _getLinuxInterfaceIdx(const char *interface_name)
{
    int                 s;
    struct ifreq        ifr;

    s = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (s==-1) {
        DEBUG_ERROR("Socket ('%s') SOCK_RAW failed with errno=%d(%s)",
                                    interface_name, errno, strerror(errno));
        return -1;
    }

    strncpy(ifr.ifr_name, interface_name, IFNAMSIZ - 1);
    if (ioctl(s, SIOCGIFINDEX, &ifr) == -1) {
        DEBUG_ERROR("Socket ('%s') SIOCGIFINDEX failed with errno=%d(%s)",
                                    interface_name, errno, strerror(errno));
        close(s);
        return -1;
    }

	close(s);
    return ifr.ifr_ifindex;
}

#define EVT_PACKETHDR_SIZE (8) //family(1)+evt(1)+packethdr(4)
#define EVT_PACKET_RESERVE_SIZE (EVT_PACKETHDR_SIZE+16) //recvidx(8)+ethtype(6)

static void *_receiveLoop(void *data)
{
    struct _linux_interface_info *info = (struct _linux_interface_info *)data;
    struct packet_mreq multicast_request;
    char interface_name[IFNAMSIZ];
    int rxbuf = 1024*1024;
    uint16_t protocol;
    struct cmdu_buf *buf = cmduBufNew(MAX_NETWORK_SEGMENT_SIZE+EVT_PACKET_RESERVE_SIZE, 0);

    if ((!buf) || (!info)) {
        DEBUG_ERROR("Invalid data point!\n");
        return NULL;
    }

    DEBUG_INFO("Start _receiveLoop on %s\n", info->interface_name);
	info->fd_1905 = info->fd_lldp = -1;

    strncpy(interface_name, info->interface_name, IFNAMSIZ-1);

    info->if_index = _getLinuxInterfaceIdx(interface_name);

    if (info->if_index == -1) {
        DEBUG_ERROR("if_index invalid(=-1), interface_name = %s\n", interface_name);
        if (info->interface_name)
            free(info->interface_name);
        free(info);
        return NULL;
    }

    protocol = (local_config.listen_specific_protocol ? ETHERTYPE_1905 : ETH_P_ALL);
    if ((info->fd_1905 = _openSocket(info->if_index, protocol)) == -1) {
        DEBUG_ERROR("Socket 1905('%s') open failed with errno=%d(%s)\n",
                                    interface_name, errno, strerror(errno));
        goto fail;
    }

    if (!local_config.listen_specific_protocol) {
        // set filter
        bpfcode_1905[3].k = info->al_mac[5] + (info->al_mac[4]<<8)
            + (info->al_mac[3]<<16) + (info->al_mac[2]<<24);
        bpfcode_1905[5].k = info->al_mac[1] + (info->al_mac[0]<<8);

        if (setsockopt(info->fd_1905, SOL_SOCKET, SO_ATTACH_FILTER,
                    &filter_1905, sizeof(filter_1905))==-1) {
            DEBUG_ERROR("Socket 1905('%s') ATTACH_FILTER failed with errno=%d(%s)",
                    interface_name, errno, strerror(errno));
        }
    }
    /* Add the AL address */
    memset(&multicast_request, 0, sizeof(multicast_request));

	multicast_request.mr_type = PACKET_MR_UNICAST;
    multicast_request.mr_ifindex = info->if_index;

    multicast_request.mr_alen = 6;
    MACCPY(multicast_request.mr_address, info->al_mac);
    if (setsockopt(info->fd_1905, SOL_PACKET, PACKET_ADD_MEMBERSHIP,
                     &multicast_request, sizeof(multicast_request))==-1) {
        DEBUG_ERROR("Socket 1905('%s') PACKET_ADD_MEMBERSHIP failed with errno=%d(%s)",
                                     interface_name, errno, strerror(errno));
    }

    /* Add the 1905 multicast address */
    multicast_request.mr_type = PACKET_MR_MULTICAST;
    MACCPY(multicast_request.mr_address, MCAST_1905);
    if (setsockopt(info->fd_1905, SOL_PACKET, PACKET_ADD_MEMBERSHIP,
                     &multicast_request, sizeof(multicast_request))==-1) {
        DEBUG_WARNING("Socket 1905('%s') PACKET_ADD_MEMBERSHIP2 failed with errno=%d(%s)",
                                    interface_name, errno, strerror(errno));
    }

    if (setsockopt(info->fd_1905, SOL_SOCKET, SO_RCVBUF, &rxbuf, sizeof(rxbuf))==-1) {
        DEBUG_WARNING("Socket 1905('%s') SO_RCVBUF failed with errno=%d(%s)",
                                    interface_name, errno, strerror(errno));
    }

    if ((info->fd_lldp = _openSocket(info->if_index, ETHERTYPE_LLDP))==-1) {
        DEBUG_ERROR("Socket lldp('%s') open failed with errno=%d(%s)\n",
                                    interface_name, errno, strerror(errno));
        goto fail;
    }

    /* Add the LLDP multicast address to this interface */
    multicast_request.mr_type = PACKET_MR_MULTICAST;
    memcpy(multicast_request.mr_address, MCAST_LLDP, 6);
    if (-1 == setsockopt(info->fd_lldp, SOL_PACKET, PACKET_ADD_MEMBERSHIP, &multicast_request, sizeof(multicast_request)))
    {
        DEBUG_ERROR("Socket lldp('%s') SOL_PACKET failed with errno=%d(%s)",
                                    interface_name, errno, strerror(errno));
    }

    DEBUG_DETAIL("Starting receive(%s)\n", interface_name);
    /** @todo move to libevent instead of threads + poll */
    while(1)
    {
        struct pollfd fdset[2];
        uint32_t i;

        DEBUG_DETAIL("Receiving one packet on %s\n", interface_name);
        memset((void*)fdset, 0, sizeof(fdset));

        fdset[0].fd = info->fd_1905;
        fdset[0].events = POLLIN;
        fdset[1].fd = info->fd_lldp;
        fdset[1].events = POLLIN;

        pthread_testcancel();
        if (0 > poll(fdset, 2, -1))
        {
            DEBUG_ERROR("Socket(%s) poll failed with errno=%d (%s)\n",
                                        interface_name, errno, strerror(errno));
            if (errno == EAGAIN || errno == EINTR)
                continue;
            else
                break;
        }
        pthread_testcancel();

        for (i = 0; i < ARRAY_SIZE(fdset); i++)
        {
            if (fdset[i].revents & (POLLIN|POLLERR)) {
                uint8_t *packet = cmduBufReserve(cmduReset(buf), EVT_PACKETHDR_SIZE);
                ssize_t packet_len;
                struct sockaddr_ll ll = {0};
                socklen_t fromlen;

                fromlen = sizeof(struct sockaddr_ll);
                packet_len = recvfrom(fdset[i].fd, packet, MAX_NETWORK_SEGMENT_SIZE, MSG_DONTWAIT, (struct sockaddr *)&ll, &fromlen);
                if (packet_len < 0) {
                    if (errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR && errno != ENETDOWN) {
                        DEBUG_ERROR("Socket(%s:%d) recvfrom failed with errno=%d (%s)\n",
                                                    interface_name, i, errno, strerror(errno));
                        /* Unrecoverable error */
                        goto fail;
                    }
                }
                else if (ll.sll_pkttype != PACKET_OUTGOING) {
                     _processPacket(info, packet, (size_t)packet_len);
                }
            }
        }
    }

fail:
    // Should not be here!
    DEBUG_ERROR("_receiveLoop(%s) Exited\n", interface_name);
    return NULL;
}

static uint32_t _readQueue(struct _platform_queue *queue)
{
    ssize_t len;

    if ((len = recv(queue->rd, queue->buf, queue->buf_len, MSG_DONTWAIT))<=0) {
        DEBUG_ERROR("[PLATFORM] recv() failed with errno=%d (%s)\n", errno, strerror(errno));
        return 0;
    }
    if (queue->size > 0)
        queue->size--;
    DEBUG_DETAIL("queue[%s], size: %u, capacity: %u\n", queue->name, queue->size, queue->capacity);

    return (int32_t)len;
}


static void _timerHandler(struct uloop_timeout *to)
{
    struct _platform_timer *timer = (struct _platform_timer *)
        container_of(to, struct _platform_timer, to);

    if (timer->handler)
        timer->handler(timer->data);

    if (timer->flag & TIMER_FLAG_PERIODIC) {
        if (uloop_timeout_set(&timer->to, timer->timeout) < 0) {
            DEBUG_ERROR("can not relaunch pedirioc timer\n");
            free(timer);
        }
    } else
        free(timer);
}

static void _queueHandler(struct uloop_fd *fd, unsigned int events)
{
    struct _platform_queue *queue =
        container_of(fd, struct _platform_queue, uloop_fd);
    uint32_t len;
    if ((len=_readQueue(queue)) && (queue->handler)) {
        queue->handler(queue->data, queue->buf, len);
    }
}


void *platformAddTimer(uint32_t ms, uint8_t flag, void (*handler)(void *), void *data)
{
    struct _platform_timer *timer = calloc(1, sizeof(struct _platform_timer));

    if (timer) {
        timer->handler = handler;
        timer->data = data;
        timer->timeout = ms;
        timer->flag = flag;
        timer->to.cb = _timerHandler;

        if (uloop_timeout_set(&timer->to, ms) < 0) {
            free(timer);
            timer = NULL;
        }
    }
    return timer;
}

int platformCancelTimer(void *t)
{
    struct _platform_timer *timer = (struct _platform_timer *)t;

    if (timer) {
        if (uloop_timeout_cancel(&timer->to) < 0) {
            DEBUG_ERROR("can not cancel timer\n");
            return -1;
        }
        free(timer);
    }
    return 0;
}

void platformStartQueue(void *q)
{
    struct _platform_queue *queue = (struct _platform_queue *)q;

    if (queue) {
        queue->uloop_fd.fd = queue->rd;
        queue->uloop_fd.cb = _queueHandler;
    }
    uloop_fd_add(&queue->uloop_fd, ULOOP_READ);
}

void *platformCreateQueue(const char *name, uint32_t buf_size, void (*handler)(void *, uint8_t *, uint32_t),void *data)
{
#define TMP_SOCKET_NAME_SIZE 20
    struct _platform_queue *ret;
    ret = calloc(1, sizeof(struct _platform_queue));
    if (ret) {
        char tmp[TMP_SOCKET_NAME_SIZE];
        int rxbuf = 20*1024;
        if ((ret->rd = _openUnixSocket(name))<0) {
            goto fail;
        }
        if (setsockopt(ret->rd, SOL_SOCKET, SO_RCVBUF, &rxbuf, sizeof(rxbuf))==-1)
            goto fail;

        snprintf(tmp, TMP_SOCKET_NAME_SIZE, "%s.l", name);
        if ((ret->sd = _openUnixSocket(tmp))<0) {
            goto fail;
        }

        if (!buf_size)
            buf_size = DEFAULT_QUEUE_BUFSIZE;
        if (!(ret->buf=malloc(buf_size)))
            goto fail;
        ret->buf_len = buf_size;
        ret->name = name;
        ret->capacity = 127;
        ret->size = 0;
        ret->handler = handler;
        ret->data = data;
        pthread_mutex_init(&ret->lock, NULL);
    }
    return ret;
fail:
    if (ret) {
        if (ret->sd>=0)
            close(ret->sd);
        if (ret->rd>=0)
            close(ret->rd);
        free(ret);
        ret = NULL;
    }
    return ret;
}

void _linuxInterfaceInfoFree(struct interface *intf)
{
    struct _linux_interface_info *info = NULL;

    info = intf->interface_info;
    intf->interface_info = NULL;
    if (!info)
        return;
    if (info->fd_1905 != -1) {
        close(info->fd_1905);
    }
    if (info->fd_lldp != -1) {
        close(info->fd_lldp);
    }
    if (info->interface_name) {
        free(info->interface_name);
    }
    free(info);
}

int platformRegisterReceiveInterface(struct interface *intf)
{
    struct _linux_interface_info *info;

    if ((!intf) || (!intf->owner) || (!intf->name)) {
        DEBUG_ERROR("[PLATFORM] platformRegisterReceiveInterface failed!!!\n");
        return -1;
    }

    if (intf->interface_info) {
        DEBUG_INFO("[PLATFORM] platformRegisterReceiveInterface interface already registered\n");
        return 0;
    }

    DEBUG_INFO("[PLATFORM] register new 1905/lldp packet receive for interface('%s')\n", intf->name);
    if ((info = (struct _linux_interface_info *)calloc(1, sizeof(struct _linux_interface_info)))) {
        info->if_index = -1;
        info->fd_1905 = -1;
        info->fd_lldp = -1;
        info->interface_name = strdup(intf->name);
        MACCPY(info->interface_mac, intf->mac);
        MACCPY(info->al_mac, intf->owner->al_mac);
        intf->interface_info = info;
        if (pthread_create(&info->thread, NULL, _receiveLoop, (void *)info)) {
            DEBUG_ERROR("[PLATFORM] create thread failed\n");
            _linuxInterfaceInfoFree(intf);
            return -1;
        }
    }
    return 0;
}

void platformUnregisterReceiveInterface(struct interface *intf)
{
    struct _linux_interface_info *info = NULL;

    if ((!intf) || (!intf->interface_info)) {
        DEBUG_ERROR("[PLATFORM] platformUnregisterReceiveInterface failed!!!\n");
        return;
    }
    DEBUG_INFO("[PLATFORM] un-register new 1905/lldp packet queue event('%s')\n", intf->name);
    info = intf->interface_info;
    pthread_cancel(info->thread);
    pthread_join(info->thread, NULL);
    _linuxInterfaceInfoFree(intf);
}

int platformQueueSend(void *q, uint8_t *message, uint16_t message_len)
{
    struct sockaddr_un addr = {0};
    socklen_t addr_len;
    int result;

    struct _platform_queue *queue = (struct _platform_queue *)q;
    if ((!queue) || (!message))
        return 0;
    if (queue->size >= queue->capacity) {
        DEBUG_WARNING("queue[%s] size(%u) >= capacity(%u), drop message!!!\n", queue->name,
            queue->size, queue->capacity);
        return 0;
    }

    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, queue->name, sizeof(addr.sun_path) - 1);
    addr_len = sizeof(addr);

    pthread_mutex_lock(&queue->lock);
    result = sendto(queue->sd, message, message_len, 0, (struct sockaddr *)&addr, addr_len);
    if (result<0) {
        DEBUG_ERROR("[PLATFORM] sendto failed with errno=%d (%s)\n", errno, strerror(errno));
        pthread_mutex_unlock(&queue->lock);
        return result;
    }
    queue->size++;
    pthread_mutex_unlock(&queue->lock);

    return 0;
}

void *platformAddTask(void *(*handler)(void *), void *data)
{
    struct _platform_task *task = calloc(1, sizeof(struct _platform_task));

    if (task) {
        pthread_create(&task->pthread, NULL, handler, data);
    }
    return task;
}

#if 0
uint8_t PLATFORM_REGISTER_QUEUE_EVENT(uint8_t queue_id, uint8_t event_type, void *data)
{
        case PLATFORM_QUEUE_EVENT_NEW_ALME_MESSAGE:
        {
            // The AL entity is telling us that it is capable of processing ALME
            // messages and that it wants to receive ALME messages on the
            // provided queue.
            //
            // In our platform-dependent implementation, we have decided that
            // ALME messages are going to be received on a dedicated thread
            // that runs a TCP server.
            //
            // What we are going to do now is:
            //
            //   1) Create that thread
            //
            //   2) Tell it that everytime a new packet containing ALME
            //      commands arrives on its socket it should forward the
            //      payload to this queue.
            //
            pthread_t                thread;
            struct almeServerThreadData  *p;

            p = (struct almeServerThreadData *)malloc(sizeof(struct almeServerThreadData));
            if (NULL == p)
            {
                // Out of memory
                //
                return 0;
            }
            p->queue_id = queue_id;

            pthread_create(&thread, NULL, almeServerThread, (void *)p);

            break;
        }

        case PLATFORM_QUEUE_EVENT_PUSH_BUTTON:
        {
            // The AL entity is telling us that it is capable of processing
            // "push button" configuration events.
            //
            // Create the thread in charge of generating these events.
            //
            pthread_t                      thread;
            struct _pushButtonThreadData  *p;

            p = (struct _pushButtonThreadData *)malloc(sizeof(struct _pushButtonThreadData));
            if (NULL == p)
            {
                // Out of memory
                //
                return 0;
            }

            p->queue_id = queue_id;
            pthread_create(&thread, NULL, _pushButtonThread, (void *)p);

            break;
        }

        case PLATFORM_QUEUE_EVENT_AUTHENTICATED_LINK:
        {
            // The AL entity is telling us that it is capable of processing
            // "authenticated link" events.
            //
            // We don't really need to do anything here. The interface specific
            // thread will be created when the AL entity calls the
            // "PLATFORM_START_PUSH_BUTTON_CONFIGURATION()" function.

            break;
        }

        case PLATFORM_QUEUE_EVENT_TOPOLOGY_CHANGE_NOTIFICATION:
        {
            // The AL entity is telling us that it is capable of processing
            // "topology change" events.
            //
            // We will create a new thread in charge of monitoring the local
            // topology to generate these events.
            //
            pthread_t                           thread;
            struct _topologyMonitorThreadData  *p;

            p = (struct _topologyMonitorThreadData *)malloc(sizeof(struct _topologyMonitorThreadData));
            if (NULL == p)
            {
                // Out of memory
                //
                return 0;
            }

            p->queue_id = queue_id;

            pthread_create(&thread, NULL, _topologyMonitorThread, (void *)p);

            break;
        }
    return 1;
}


// *********** Push button stuff ***********************************************

// Pressing the button can be simulated by "touching" (ie. updating the
// timestamp) the following tmp file
//
#define PUSH_BUTTON_VIRTUAL_FILENAME  "/tmp/virtual_push_button"

// For those platforms with a physical buttons attached to a GPIO, we need to
// know the actual GPIO number (as seen by the Linux kernel) to use.
//
//     NOTE: "PUSH_BUTTON_GPIO_NUMBER" is a string, not a number. It will later
//     be used in a string context, thus the "" are needed.
//     It can take the string representation of a number (ex: "26") or the
//     special value "disable", meaning we don't have GPIO support.
//
#define PUSH_BUTTON_GPIO_NUMBER              "disable" //"26"

#define PUSH_BUTTON_GPIO_EXPORT_FILENAME     "/sys/class/gpio/export"
#define PUSH_BUTTON_GPIO_DIRECTION_FILENAME  "/sys/class/gpio/gpio"PUSH_BUTTON_GPIO_NUMBER"/direction"
#define PUSH_BUTTON_GPIO_VALUE_FILENAME      "/sys/class/gpio/gpio"PUSH_BUTTON_GPIO_NUMBER"/direction"

// The only information that needs to be sent to the new thread is the "queue
// id" to later post messages to the queue.
//
struct _pushButtonThreadData
{
    uint8_t     queue_id;
};

static void *_pushButtonThread(void *p)
{
    // In this implementation we will send the "push button" configuration
    // event message to the queue when either:
    //
    //   a) The user presses a physical button associated to a GPIO whose number
    //      is "PUSH_BUTTON_GPIO_NUMBER" (ie. it is exported by the linux kernel
    //      in "/sys/class/gpio/gpioXXX", where "XXX" is
    //      "PUSH_BUTTON_GPIO_NUMBER")
    //
    //   b) The user updates the timestamp of a tmp file called
    //      "PUSH_BUTTON_VIRTUAL_FILENAME".
    //      This is useful for debugging and for supporting the "push button"
    //      mechanism in those platforms without a physical button.
    //
    // This thread will simply wait for activity on any of those two file
    // descriptors and then send the "push button" configuration event to the
    // AL queue.
    // How is this done?
    //
    //   1. Configure the GPIO as input.
    //   2. Create an "inotify" watch on the tmp file.
    //   3. Use "poll()" to wait for either changes in the value of the GPIO or
    //      timestamp updates in the tmp file.

    int    gpio_enabled;

    FILE  *fd_gpio;
    FILE  *fd_tmp;

    int  fdraw_gpio;
    int  fdraw_tmp;

    struct pollfd fdset[2];

    uint8_t queue_id;

    queue_id = ((struct _pushButtonThreadData *)p)->queue_id;;

    if (0 != strcmp(PUSH_BUTTON_GPIO_NUMBER, "disable"))
    {
        gpio_enabled = 1;
    }
    else
    {
        gpio_enabled = 0;
    }

    // First of all, prepare the GPIO kernel descriptor for "reading"...
    //
    if (gpio_enabled)
    {

        // 1. Write the number of the GPIO where the physical button is
        //    connected to file "/sys/class/gpio/export".
        //    This will instruct the Linux kernel to create a folder named
        //    "/sys/class/gpio/gpioXXX" that we can later use to read the GPIO
        //    level.
        //
        if (NULL == (fd_gpio = fopen(PUSH_BUTTON_GPIO_EXPORT_FILENAME, "w")))
        {
            DEBUG_ERROR("[PLATFORM] *Push button thread* Error opening GPIO fd %s\n", PUSH_BUTTON_GPIO_EXPORT_FILENAME);
            return NULL;
        }
        if (0 == fwrite(PUSH_BUTTON_GPIO_NUMBER, 1, strlen(PUSH_BUTTON_GPIO_NUMBER), fd_gpio))
        {
            DEBUG_ERROR("[PLATFORM] *Push button thread* Error writing '"PUSH_BUTTON_GPIO_NUMBER"' to %s\n", PUSH_BUTTON_GPIO_EXPORT_FILENAME);
            fclose(fd_gpio);
            return NULL;
        }
        fclose(fd_gpio);

        // 2. Write "in" to file "/sys/class/gpio/gpioXXX/direction" to tell the
        //    kernel that this is an "input" GPIO (ie. we are only going to
        //    read -and not write- its value).

        if (NULL == (fd_gpio = fopen(PUSH_BUTTON_GPIO_DIRECTION_FILENAME, "w")))
        {
            DEBUG_ERROR("[PLATFORM] *Push button thread* Error opening GPIO fd %s\n", PUSH_BUTTON_GPIO_DIRECTION_FILENAME);
            return NULL;
        }
        if (0 == fwrite("in", 1, strlen("in"), fd_gpio))
        {
            DEBUG_ERROR("[PLATFORM] *Push button thread* Error writing 'in' to %s\n", PUSH_BUTTON_GPIO_DIRECTION_FILENAME);
            fclose(fd_gpio);
            return NULL;
        }
        fclose(fd_gpio);
    }

    // ... and then re-open the GPIO file descriptors for reading in "raw"
    // (ie "open" instead of "fopen") mode.
    //
    if (gpio_enabled)
    {
        if (-1  == (fdraw_gpio = open(PUSH_BUTTON_GPIO_VALUE_FILENAME, O_RDONLY | O_NONBLOCK)))
        {
            DEBUG_ERROR("[PLATFORM] *Push button thread* Error opening GPIO fd %s\n", PUSH_BUTTON_GPIO_VALUE_FILENAME);
        }
    }

    // Next, regarding the "virtual" button, first create the "tmp" file in
    // case it does not already exist...
    //
    if (NULL == (fd_tmp = fopen(PUSH_BUTTON_VIRTUAL_FILENAME, "w+")))
    {
        DEBUG_ERROR("[PLATFORM] *Push button thread* Could not create tmp file %s\n", PUSH_BUTTON_VIRTUAL_FILENAME);
        return NULL;
    }
    fclose(fd_tmp);

    // ...and then add a "watch" that triggers when its timestamp changes (ie.
    // when someone does a "touch" of the file or writes to it, for example).
    //
    if (-1 == (fdraw_tmp = inotify_init()))
    {
        DEBUG_ERROR("[PLATFORM] *Push button thread* inotify_init() returned with errno=%d (%s)\n", errno, strerror(errno));
        return NULL;
    }
    if (-1 == inotify_add_watch(fdraw_tmp, PUSH_BUTTON_VIRTUAL_FILENAME, IN_ATTRIB))
    {
        DEBUG_ERROR("[PLATFORM] *Push button thread* inotify_add_watch() returned with errno=%d (%s)\n", errno, strerror(errno));
        return NULL;
    }

    // At this point we have two file descriptors ("fdraw_gpio" and "fdraw_tmp")
    // that we can monitor with a call to "poll()"
    //
    while(1)
    {
        int   nfds;
        uint8_t button_pressed;

        memset((void*)fdset, 0, sizeof(fdset));

        fdset[0].fd     = fdraw_tmp;
        fdset[0].events = POLLIN;
        nfds            = 1;

        if (gpio_enabled)
        {
            fdset[1].fd     = fdraw_gpio;
            fdset[1].events = POLLPRI;
            nfds            = 2;
        }

        // The thread will block here (forever, timeout = -1), until there is
        // a change in one of the two file descriptors ("changes" in the "tmp"
        // file fd are cause by "attribute" changes -such as the timestamp-,
        // while "changes" in the GPIO fd are caused by a value change in the
        // GPIO value).
        //
        if (0 > poll(fdset, nfds, -1))
        {
            DEBUG_ERROR("[PLATFORM] *Push button thread* poll() returned with errno=%d (%s)\n", errno, strerror(errno));
            break;
        }

        button_pressed = 0;

        if (fdset[0].revents & POLLIN)
        {
            struct inotify_event event;

            DEBUG_DETAIL("[PLATFORM] *Push button thread* Virtual button has been pressed!\n");
            button_pressed = 1;

            // We must "read()" from the "tmp" fd to "consume" the event, or
            // else the next call to "poll() won't block.
            //
            read(fdraw_tmp, &event, sizeof(event));
        }
        else if (gpio_enabled && (fdset[1].revents & POLLPRI))
        {
            char buf[3];

            if (-1 == read(fdset[1].fd, buf, 3))
            {
                DEBUG_ERROR("[PLATFORM] *Push button thread* read() returned with errno=%d (%s)\n", errno, strerror(errno));
                continue;
            }

            if (buf[0] == '1')
            {
                DEBUG_DETAIL("[PLATFORM] *Push button thread* Physical button has been pressed!\n");
                button_pressed = 1;
            }
        }

        if (1 == button_pressed)
        {
            uint8_t   message[3];

            message[0] = PLATFORM_QUEUE_EVENT_PUSH_BUTTON;
            message[1] = 0x0;
            message[2] = 0x0;

            DEBUG_DETAIL("[PLATFORM] *Push button thread* Sending 3 bytes to queue (0x%02x, 0x%02x, 0x%02x)\n", message[0], message[1], message[2]);

            if (0 == platformQueueSend(queue_id, message, 3))
            {
                DEBUG_ERROR("[PLATFORM] *Push button thread* Error sending message to queue from _pushButtonThread()\n");
            }
        }
    }

    // Close file descriptors and exit
    //
    if (gpio_enabled)
    {
        fclose(fd_gpio);
    }

    DEBUG_INFO("[PLATFORM] *Push button thread* Exiting...\n");

    free(p);
    return NULL;
}

// *********** Topology change notification stuff ******************************

// The platform notifies the 1905 that a topology change has just took place
// by "touching" the following tmp file
//
#define TOPOLOGY_CHANGE_NOTIFICATION_FILENAME  "/tmp/topology_change"

// The only information that needs to be sent to the new thread is the "queue
// id" to later post messages to the queue.
//
struct _topologyMonitorThreadData
{
    uint8_t     queue_id;
};

static void *_topologyMonitorThread(void *p)
{
    FILE  *fd_tmp;

    int  fdraw_tmp;

    struct pollfd fdset[2];

    uint8_t  queue_id;

    queue_id = ((struct _topologyMonitorThreadData *)p)->queue_id;

    // Regarding the "virtual" notification system, first create the "tmp" file
    // in case it does not already exist...
    //
    if (NULL == (fd_tmp = fopen(TOPOLOGY_CHANGE_NOTIFICATION_FILENAME, "w+")))
    {
        DEBUG_ERROR("[PLATFORM] *Topology change monitor thread* Could not create tmp file %s\n", TOPOLOGY_CHANGE_NOTIFICATION_FILENAME);
        return NULL;
    }
    fclose(fd_tmp);

    // ...and then add a "watch" that triggers when its timestamp changes (ie.
    // when someone does a "touch" of the file or writes to it, for example).
    //
    if (-1 == (fdraw_tmp = inotify_init()))
    {
        DEBUG_ERROR("[PLATFORM] *Push button thread* inotify_init() returned with errno=%d (%s)\n", errno, strerror(errno));
        return NULL;
    }
    if (-1 == inotify_add_watch(fdraw_tmp, TOPOLOGY_CHANGE_NOTIFICATION_FILENAME, IN_ATTRIB))
    {
        DEBUG_ERROR("[PLATFORM] *Push button thread* inotify_add_watch() returned with errno=%d (%s)\n", errno, strerror(errno));
        return NULL;
    }

    while (1)
    {
        int   nfds;
        uint8_t notification_activated;

        memset((void*)fdset, 0, sizeof(fdset));

        fdset[0].fd     = fdraw_tmp;
        fdset[0].events = POLLIN;
        nfds            = 1;

        // TODO: Other fd's to detect topoly changes would be initialized here.
        // One good idea would be to use a NETLINK socket that is notified by
        // the Linux kernel when network "stuff" (routes, IPs, ...) change.
        //
        //fdset[0].fd     = ...;
        //fdset[0].events = POLLIN;
        //nfds            = 2;

        // The thread will block here (forever, timeout = -1), until there is
        // a change in one of the previous file descriptors .
        //
        if (0 > poll(fdset, nfds, -1))
        {
            DEBUG_ERROR("[PLATFORM] *Topology change monitor thread* poll() returned with errno=%d (%s)\n", errno, strerror(errno));
            break;
        }

        notification_activated = 0;

        if (fdset[0].revents & POLLIN)
        {
            struct inotify_event event;

            DEBUG_DETAIL("[PLATFORM] *Topology change monitor thread* Virtual notification has been activated!\n");
            notification_activated = 1;

            // We must "read()" from the "tmp" fd to "consume" the event, or
            // else the next call to "poll() won't block.
            //
            read(fdraw_tmp, &event, sizeof(event));
        }

        if (1 == notification_activated)
        {
            uint8_t  message[3];

            message[0] = PLATFORM_QUEUE_EVENT_TOPOLOGY_CHANGE_NOTIFICATION;
            message[1] = 0x0;
            message[2] = 0x0;

            DEBUG_DETAIL("[PLATFORM] *Topology change monitor thread* Sending 3 bytes to queue (0x%02x, 0x%02x, 0x%02x)\n", message[0], message[1], message[2]);

            if (0 == platformQueueSend(queue_id, message, 3))
            {
                DEBUG_ERROR("[PLATFORM] *Topology change monitor thread* Error sending message to queue from _pushButtonThread()\n");
            }
        }
    }

    DEBUG_INFO("[PLATFORM] *Topology change monitor thread* Exiting...\n");

    free(p);
    return NULL;
}
#endif
