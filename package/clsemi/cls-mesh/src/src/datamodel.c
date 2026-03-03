#include <stdlib.h>

#include "platform.h"
#include "1905_tlvs.h"
#include "datamodel.h"
#include "feature/feature.h"
#include "feature/ubus/ubus_helper.h"
#include "al_driver.h"
#include "al_action.h"
#include "wifi.h"

#include "platform_os.h"



struct al_device *local_device = NULL;
struct al_device *registrar = NULL;
struct map_config local_config = {0};
struct map_policy local_policy = {0};

DEFINE_DLIST_HEAD(local_network);

struct configurator local_configurator = {0};

void printBSSConfig(struct bss_info *bss)
{
    DEBUG_INFO("\tssid=%s, auth=0x%02x, encrypt=0x%02x\n", bss->ssid.ssid, bss->auth, bss->encrypt);
    DEBUG_INFO("\tbackhaul/fronthaul=%d/%d\n", bss->backhaul, bss->fronthaul);
    DEBUG_INFO("\tkey=%s\n", bss->key.key);
    if (bss->backhaul) {
        int i;
        while (bss->vlan_map[i]) {
            DEBUG_INFO("\tvlan=%d, ", bss->vlan_map[i]);
            i++;
        }
        DEBUG_INFO("\n");
    } else if (bss->vlan_map[0])
        DEBUG_INFO("\tvlan=%d, ", bss->vlan_map[0]);
    DEBUG_INFO("\n");
}

void _policyInit(struct map_policy *p)
{
    dlist_head_init(&p->steer);
    dlist_head_init(&p->stalist_local_steer_disallow);
    dlist_head_init(&p->stalist_btm_steer_disallow);
    dlist_head_init(&p->backhaul_bss_configs);
    dlist_head_init(&p->vlans);
    dlist_head_init(&p->vips);
    dlist_head_init(&p->queue_conf);
    dlist_head_init(&p->tc_conf.mapping_list);
    dlist_head_init(&p->dscp_conf.dscp_list);

    p->roaming_policy.rcpi_steer = 1;
    p->roaming_policy.band_steer = 1;
    p->roaming_policy.rssi_high = -60;
    p->roaming_policy.rssi_low = -75;
    p->roaming_policy.rcpi_thresh = 80;
    p->roaming_policy.rssi_gain_of_5g = 0;
    p->roaming_policy.rssi_gain_thresh = 5;
    p->roaming_policy.cooldown = 120;

    /* report policy default */
    for (int i = 0; i < band_max_idx; i++) {
        p->metrics_rpt[i].band_idx = i;
        p->metrics_rpt[i].rpt_policy.sta_rcpi_thresh = 80;
        p->metrics_rpt[i].rpt_policy.sta_rcpi_margin = 3;
        p->metrics_rpt[i].rpt_policy.ap_chutil_thresh = 80;
        p->metrics_rpt[i].rpt_policy.assoc_sta_inclusion_mode = REPORT_STA_LINK_METRICS;
    }

    p->vbss_conf.enable = 0;

    p->tc_conf.dft_qid = 255;
    p->tc_conf.dft_tid = 255;
    p->dscp_conf.dft_qid = 255;
    p->dscp_conf.dft_tid = 255;
}

static void _configInit(struct map_config *c)
{
    c->agent = 1;
    c->profile = profile_2;

    c->device_info.manufacturer = "ClourneySemi";
    c->device_info.model_name = "Dubhe1000";
    c->device_info.model_num = "0000001";
    c->device_info.device_name = "Ultra Mesh";
    c->device_info.serial_no = "000";
    c->autoconf_timeout = 5;
    c->topology_timeout = 5;
    c->age_timeout = 70;
    c->relay = 1;
    c->listen_specific_protocol = 1;
    c->wfa_mode = 0;
    c->sync_sta = 0;
    c->lldp = 0;

    dlist_head_init(&c->wifi_config.bsses);
}

void policyReset(struct map_policy *p)
{
    dlist_free_items(&p->stalist_local_steer_disallow, struct mac_item, l);
    dlist_free_items(&p->stalist_btm_steer_disallow, struct mac_item, l);
    dlist_free_items(&p->steer, struct steer_policy_item, l);
    dlist_free_items(&p->backhaul_bss_configs, struct backhaul_bss_conf_item, l);
    dlist_free_items(&p->vips, struct mac_item, l);
    dlist_free_items(&p->queue_conf, struct queue_conf_item, l);
    dlist_free_items(&p->tc_conf.mapping_list, struct tc_mapping_item, l);
    dlist_free_items(&p->vlans, struct vlan_config_item, l2.l);
    p->tc_conf.dft_qid = 255;
    p->tc_conf.dft_tid = 255;
}


void datamodelInit(void)
{
    _configInit(&local_config);
    _policyInit(&local_policy);
}

void DMalMacSet(uint8_t *al_mac)
{
    //create first al device
    local_device = alDeviceNew(al_mac);

    local_device->profile = local_config.profile;
    if (local_config.agent)
        local_device->is_agent = 1;
    if (local_config.controller)
        local_device->is_controller = 1;

    return;
}

uint8_t *DMalMacGet()
{
    return (local_device ? local_device->al_mac : (uint8_t *)ZEROMAC);
}

void updateLocalWifiConfig(struct radio *r, dlist_head *bsses)
{
    struct wifi_config *item, *tmp;

    if (!r || !bsses)
        return;

    dlist_for_each_safe(item, tmp, local_config.wifi_config.bsses, l) {
        if (r->bands == item->bands) {
            dlist_remove(&item->l);
            free(item);
            item = NULL;
        }
    }

    dlist_for_each(item, *bsses, l) {
        struct wifi_config *wcfg = calloc(1, sizeof(struct wifi_config));
        if (wcfg) {
            memcpy(&wcfg->bss, &item->bss, sizeof(wcfg->bss));
            wcfg->bands = item->bands;
            dlist_add_tail(&local_config.wifi_config.bsses, &wcfg->l);
        }
    }

    return;
}

uint8_t dmUpdateNeighbor(struct interface *intf, uint8_t *al_mac, uint8_t *intf_mac, uint8_t is_1905)
{
    uint32_t ts;
    uint8_t  first = 0;
    struct neighbor *n;

    if (!(alDeviceFind(al_mac))) {
        first = 1;
    }
    n = neighborAdd(intf, al_mac, intf_mac);
    if (is_1905)
        n->is_1905 = 1;
    ts = PLATFORM_GET_TIMESTAMP(0);

    if (is_1905)
        n->last_1905_discovery = ts;
    else
        n->last_lldp_discovery = ts;

    return first;
}

uint8_t dmDeviceNeedUpdate(struct al_device *d)
{
    return ((PLATFORM_GET_TIMESTAMP(0) - d->last_topo_resp_ts)
            > (local_config.topology_timeout * 1000));
}

void dmUpdateIPInfo()
{
    struct interface *intf;
    struct interface_ip_item *intf_ip_item, *tmp_ip;
    struct ipv4_item *ip_item, *tmp;
    struct ipv6_item *ipv6_item, *tmp6;
    DEFINE_DLIST_HEAD(ips);

    if ((!local_device) || (configuratorGetIpInfo(&ips)))
        return;

    dlist_for_each(intf, local_device->interfaces, l) {
       dlist_free_items(&intf->ipv4s, struct ipv4_item, l);
       dlist_free_items(&intf->ipv6s, struct ipv6_item, l);
    }

    dlist_for_each_safe(intf_ip_item, tmp_ip, ips, l) {
        if ((intf = interfaceFind(local_device, intf_ip_item->intf_mac, interface_type_unknown))) {
            intf->local_ipv6 = intf_ip_item->local_ipv6;
            dlist_for_each_safe(ip_item, tmp, intf_ip_item->ipv4s, l) {
                dlist_remove(&ip_item->l);
                dlist_add_tail(&intf->ipv4s, &ip_item->l);
            }
            dlist_for_each_safe(ipv6_item, tmp6, intf_ip_item->ipv6s, l) {
                dlist_remove(&ipv6_item->l);
                dlist_add_tail(&intf->ipv6s, &ipv6_item->l);
            }
        } else {
            while ((ip_item =
                container_of(dlist_get_first(&intf_ip_item->ipv4s), struct ipv4_item, l))) {
                dlist_remove(&ip_item->l);
                free(ip_item);
            }
            while ((ipv6_item =
                container_of(dlist_get_first(&intf_ip_item->ipv6s), struct ipv6_item, l))) {
                dlist_remove(&ipv6_item->l);
                free(ipv6_item);
            }
        }
        dlist_remove(&intf_ip_item->l);
        free(intf_ip_item);
    }
}

struct al_device *alDeviceNew(uint8_t *mac)
{
    struct al_device *d = (struct al_device *)calloc(1, sizeof(struct al_device));

    if (d) {
        DEBUG_INFO("new device " MACFMT "\n", MACARG(mac));

        MACCPY(d->al_mac, mac);

        d->device_info.start_time = 0-PLATFORM_GET_TIMESTAMP(0)/1000;
        dlist_head_init(&d->interfaces);
        dlist_head_init(&d->radios);
        dlist_head_init(&d->btm_disallow_list);
        dlist_head_init(&d->local_disallow_list);

        dlist_add_tail(&local_network, &d->l);
    }
    return d;
}

void alDeviceSetConfigured(struct al_device *d, uint8_t configured)
{
    struct radio *r;

    if (d) {
        dlist_for_each(r, d->radios, l) {
            r->configured = configured;
        }
        d->configured = configured;
        if (!configured)
            d->ap_basic_capaed = 0;
    }
}

void alDeviceDelete(struct al_device *d)
{
    struct radio *r, *rtmp;
    struct interface *intf, *intftmp;

    if (!d)
        return;

    dlist_remove(&d->l);

    if (d==registrar) {
        setRegistrar(NULL);
    }

    if (d->metrics_rpt_timer)
        platformCancelTimer(d->metrics_rpt_timer);

    dlist_for_each_safe(r, rtmp, d->radios, l) {
        radioDelete(r);
    }

    dlist_for_each_safe(intf, intftmp, d->interfaces, l) {
        if (intf->type==interface_type_wifi)
            ((struct wifi_interface *)intf)->radio = NULL;
        interfaceDelete(intf);
    }

    CHECK_FREE(d->device_info.friendly_name);
    CHECK_FREE(d->device_info.manufacturer);
    CHECK_FREE(d->device_info.model_name);

    free(d);
}

bool staInLocalDisallowedList(uint8_t *mac)
{
    struct mac_item *pos = NULL;

    if (!mac)
        return false;

    dlist_for_each(pos, local_device->local_disallow_list, l) {
        if (!MACCMP(pos->mac, mac))
            return true;
    }

    return false;
}

bool staInBTMDisallowedList(uint8_t *mac)
{
    struct mac_item *pos = NULL;

    if (!mac)
        return false;

    dlist_for_each(pos, local_device->btm_disallow_list, l) {
        if (!MACCMP(pos->mac, mac))
            return true;
    }

    return false;
}


int chanScanItemStatusTransfer(struct chscan_req_item *item) {
    if (!item || !item->req || !item->req->r)
        return SCAN_STATUS_SUCCESS;

    switch (item->status) {
    case CHANNEL_SCAN_STATUS_NOT_START:
    case CHANNEL_SCAN_STATUS_TRIGGERRED:
    case CHANNEL_SCAN_STATUS_GETTING_RESULTS:
        return SCAN_STATUS_NOT_COMPLETE;
    case CHANNEL_SCAN_STATUS_RESULTS_FINISHED:
        return SCAN_STATUS_SUCCESS;
    case CHANNEL_SCAN_STATUS_ABORTED:
        return SCAN_STATUS_SCAN_ABORT;
    }

    return SCAN_STATUS_SUCCESS;
}

struct chscan_req_item *chscanReqItemFind(struct radio *r, uint8_t opclass)
{
    struct chscan_req_item *item = NULL;

    if (!r || !r->chscan_req)
        return NULL;

    dlist_for_each(item, r->chscan_req->h, l) {
        if (item->opclass == opclass) {
            return item;
        }
    }

    return NULL;
}

struct chscan_req_item *chscanReqItemNew(struct chscan_req *req, uint8_t opclass, uint8_t chnum, uint8_t *ch)
{
    struct chscan_req_item *item = calloc(1, sizeof(struct chscan_req_item));

    if (!item)
        return NULL;

    item->req = req;
    item->opclass = opclass;
    item->ch_num = chnum;
    memcpy(item->chans, ch, chnum);
    item->status = CHANNEL_SCAN_STATUS_NOT_START;
    dlist_add_tail(&req->h, &item->l);
    return item;
}

struct chscan_req *chscanReqNew(struct radio *r, uint32_t ifindex, uint8_t *from)
{
    struct chscan_req *req = calloc(1, sizeof(struct chscan_req));
    if (!req)
        return NULL;

    dlist_head_init(&req->h);
    req->r = r;
    req->ts = PLATFORM_GET_TIMESTAMP(0);
    req->ifindex = ifindex;
    memcpy(req->src, from, sizeof(mac_address));
    return req;
}

void chscanReqDelete(struct chscan_req *req)
{
    if (!req)
        return;

    if (req->timer)
        platformCancelTimer(req->timer);

    if (!isRegistrar()) { /* controller will free the space in UBUS-API one shot */
        dlist_free_items(&req->h, struct chscan_req_item, l);
        free(req);
    }
}

struct radio *radioFind(struct al_device *d, uint8_t *mac)
{
    struct radio *r;
    if (d) {
        dlist_for_each(r, d->radios, l) {
            if (!MACCMP(r->uid, mac))
                return r;
        }
    } else {
        dlist_for_each(d, local_network, l) {
            if ((r = radioFind(d, mac)))
                return r;
        }
    }
    return NULL;
}

struct wifi_interface *radioFindInterface(struct radio *r, uint8_t *mac)
{
    int i;

    for (i=0;i<r->configured_bsses.len;i++) {
        struct wifi_interface *wif = r->configured_bsses.p[i];
        if (!MACCMP(mac, wif->i.mac))
            return wif;
    }
    return NULL;
}


struct radio *radioFindById(struct al_device *d, uint32_t id)
{
    struct radio *r;

    if (d) {
        dlist_for_each(r, d->radios, l) {
            if (id == r->index)
                return r;
        }
    } else {
        dlist_for_each(d, local_network, l) {
            if ((r = radioFindById(d, id)))
                return r;
        }
    }
    return NULL;
}

struct radio *radioFindByName(struct al_device *d, const char *name)
{
    struct radio *r;

    if (d) {
        dlist_for_each(r, d->radios, l) {
            if (!strcasecmp(r->name, name))
                return r;
        }
    } else {
        dlist_for_each(d, local_network, l) {
            if ((r = radioFindByName(d, name)))
                return r;
        }
    }
    return NULL;
}

struct radio *radioNew(struct al_device *d, uint8_t *mac)
{
    struct radio *r;

    if ((!d) || (!mac))
        return NULL;

    if ((r = (struct radio *)calloc(1, sizeof(struct radio)))) {
        DEBUG_INFO("Radio[" MACFMT "] added to dev[" MACFMT "]\n",
                MACARG(mac), MACARG(d->al_mac));
        MACCPY(r->uid, mac);

        r->index = -1;
        r->d = d;
        dlist_head_init(&r->unassoc_stas);
        dlist_head_init(&r->ctrl_most_pref_chs);
        dlist_add_tail(&d->radios, &r->l);
    } else {
        DEBUG_ERROR ("r or d == NULL in radioNew\n");
    }
    return r;
}

struct radio *radioAdd(struct al_device *d, uint8_t *mac)
{
    struct radio *r = NULL;

    if (d) {
        if ((r = radioFind(d, mac)))
            return r;
        else
            return radioNew(d, mac);
    }
    return r;
}

void radioDelete(struct radio *r)
{
    unsigned i;

    if (!r)
        return;

    dlist_remove(&r->l);
    for (i = 0; i < r->configured_bsses.len; i++) {
        /* The interfaceWifi is deleted automatically when we delete the interface itself. */
        r->configured_bsses.p[i]->radio = NULL;
        wifiInterfaceDelete(r->configured_bsses.p[i]);
    }
    chscanReqDelete(r->chscan_req);
    chscanReqDelete(r->chscan_req_pend);
    NARRAY_CLEAR(r->configured_bsses);
    free(r);
}

void alDeviceAddInterface(struct al_device *d, struct interface *intf)
{
    dlist_add_tail(&d->interfaces, &intf->l);
    intf->owner = d;
}

static struct interface *interfaceInit(struct interface *intf, uint8_t *mac, struct al_device *d)
{
    MACCPY(intf->mac, mac);
    intf->type = interface_type_unknown;
    if (d) {
        DEBUG_INFO("interface[" MACFMT "] added to device["MACFMT"]\n",
                MACARG(mac), MACARG(d->al_mac));
        alDeviceAddInterface(d, intf);
    }
    return intf;
}

struct interface *interfaceNew(struct al_device *d, uint8_t *mac, size_t size)
{
    struct interface *intf = (struct interface *)calloc(1, size);

    if (intf) {
        dlist_head_init(&intf->neighbors);
        dlist_head_init(&intf->ipv4s);
        dlist_head_init(&intf->ipv6s);
        return interfaceInit(intf, mac, d);
    }
    return intf;
}

void interfaceDelete(struct interface *intf)
{
    if (!intf)
        return;

    dlist_remove(&intf->l);

    if (intf->owner) {
        struct neighbor *n, *tmp;
        dlist_for_each_safe(n, tmp, intf->neighbors, l) {
            neighborDeleteRelated(n->al_mac, intf->owner->al_mac);
            neighborDelete(n);
        }
    }

    dlist_free_items(&intf->ipv6s, struct ipv6_item, l);
    dlist_free_items(&intf->ipv4s, struct ipv4_item, l);

    if (intf->type == interface_type_wifi) {
        struct wifi_interface *ifw = (struct wifi_interface *)intf;
        struct client *client, *client_tmp;

        if (ifw->radio)
            NARRAY_DELETE(ifw->radio->configured_bsses, ifw);

        dlist_for_each_safe(client, client_tmp, ifw->clients, l) {
            clientDelete(client);
        }

        if (ifw->vbss_client_context) {
            if (ifw->vbss_client_context->password.datap)
                free(ifw->vbss_client_context->password.datap);
            if (ifw->vbss_client_context->ptk.datap)
                free(ifw->vbss_client_context->ptk.datap);
            if (ifw->vbss_client_context->gtk.datap)
                free(ifw->vbss_client_context->gtk.datap);
            free(ifw->vbss_client_context);
        }
        DEBUG_INFO("BSS["MACFMT"] removed.\n", MACARG(intf->mac));
    } else {
        DEBUG_INFO("Interface["MACFMT"] removed.\n", MACARG(intf->mac));
    }

    if (intf->name)
        free(intf->name);
    free(intf);
}

struct wifi_interface *wifiInterfaceNew(struct al_device *d, uint8_t *mac)
{
    struct wifi_interface *ifw =
        (struct wifi_interface *)interfaceNew(d, mac, sizeof(struct wifi_interface));

    if (ifw) {
        dlist_head_init(&ifw->clients);
        ifw->i.type = interface_type_wifi;
        DEBUG_INFO("BSS["MACFMT"] created.\n", MACARG(mac));
    }
    return ifw;
}

struct vbss_client_context_info *wifiInterfaceAddVbssClientContext(struct wifi_interface *wif,
        uint8_t *sta_mac, uint64_t tx_packet_number, uint64_t group_tx_packet_number,
        uint8_t *ptk, uint16_t ptk_len, uint8_t *gtk, uint16_t gtk_len)
{
    if (!wif || !sta_mac)
        return NULL;

    if (!wif->vbss_client_context)
        wif->vbss_client_context = calloc(1, sizeof(struct vbss_client_context_info));
    if (!wif->vbss_client_context)
        return NULL;

    MACCPY(wif->vbss_client_context->client_mac, sta_mac);
    wif->vbss_client_context->tx_packet_number = tx_packet_number;
    wif->vbss_client_context->group_tx_packet_number = group_tx_packet_number;
    if (ptk && ptk_len > 0) {
        wif->vbss_client_context->ptk.len = ptk_len;
        if (wif->vbss_client_context->ptk.datap)
            free(wif->vbss_client_context->ptk.datap);
        wif->vbss_client_context->ptk.datap = (uint8_t *)calloc(1, ptk_len);
        memcpy(wif->vbss_client_context->ptk.datap, ptk, ptk_len);
    }
    if (gtk && gtk  > 0) {
        wif->vbss_client_context->gtk.len = gtk_len;
        if (wif->vbss_client_context->gtk.datap)
            free(wif->vbss_client_context->gtk.datap);
        wif->vbss_client_context->gtk.datap = (uint8_t *)calloc(1, gtk_len);
        memcpy(wif->vbss_client_context->gtk.datap, gtk, gtk_len);
    }

    return wif->vbss_client_context;
}

struct wifi_interface *wifiInterfaceAdd(struct al_device *d, struct radio *r,  uint8_t *mac)
{
    struct wifi_interface *wif = NULL;

    if (d) {
        if (!(wif = (struct wifi_interface *)interfaceFind(d, mac, interface_type_wifi)))
            wif = wifiInterfaceNew(d, mac);
        if ((wif) && (r)) {
            wif->radio = r;
            NARRAY_ADD(r->configured_bsses, wif);
        }
    }

    return wif;
}

struct wifi_interface *peerInterfaceFind(uint8_t *mac)
{
    struct al_device *d = NULL;
    struct interface *i = NULL;
    struct wifi_interface *wif = NULL;
    struct client *c = NULL;
    dlist_for_each(d, local_network, l) {
        dlist_for_each(i, d->interfaces, l) {
            wif = (struct wifi_interface *)i;
            c = clientFind(d, wif, mac);
            if (c) {
                return wif;
            }
        }
    }
    return NULL;
}

struct interface *interfaceFind(struct al_device *d, uint8_t *mac,
            enum e_interface_type type)
{
    struct interface *i;
    if (d) {
        dlist_for_each(i, d->interfaces, l) {
            if ((!MACCMP(i->mac, mac)) &&
                ((i->type==type) || (type==interface_type_unknown)))
                return i;
        }
    } else {
        dlist_for_each(d, local_network, l) {
            if ((i=interfaceFind(d, mac, type)))
                return i;
        }
    }
    return NULL;
}

struct interface *interfaceFindName(struct al_device *d, char *name)
{
    struct interface *i;
    if (d) {
        dlist_for_each(i, d->interfaces, l) {
            if ((i->name) && (!strcmp(name, i->name)))
                return i;
        }
    }
    return NULL;
}

static struct client *_clientFindInterface(struct wifi_interface *ifw, uint8_t *mac)
{
    struct client *c;

    dlist_for_each(c, ifw->clients, l) {
        if (!MACCMP(c->mac, mac))
            return c;
    }
    return NULL;
}

static struct client *_clientFindDevice(struct al_device *d, uint8_t *mac)
{
    struct client *c;
    struct interface *intf;
    struct wifi_interface *wintf;

    dlist_for_each(intf, d->interfaces, l) {
        if (intf->type != interface_type_wifi)
            continue;
        wintf = (struct wifi_interface *)intf;
        dlist_for_each(c, wintf->clients, l) {
            if (!MACCMP(c->mac, mac))
                return c;
        }
    }

    return NULL;
}

struct client *clientFind(struct al_device *d, struct wifi_interface *ifw, uint8_t *mac)
{
    if (ifw) {
        return _clientFindInterface(ifw, mac);
    } else if (d) {
        return _clientFindDevice(d, mac);
    } else {
        struct client *sta;
        dlist_for_each(d, local_network, l) {
            if ((sta = _clientFindDevice(d, mac)))
                return sta;
        }
    }
    return NULL;
}

struct client *clientNew(struct wifi_interface *ifw, uint8_t *mac)
{
    struct client *c = calloc(1, sizeof(struct client));

    if (c) {
        MACCPY(c->mac, mac);
        c->wif = ifw;
        c->steer_state = CLIENT_STEER_STATUS_INIT;
        dlist_head_init(&c->seens);
        dlist_add_tail(&ifw->clients, &c->l);
    }
    return c;
}

int clientMove(struct client *c, struct wifi_interface *dst)
{
    if (!c || !dst)
        return -1;

    DEBUG_INFO("client["MACFMT"] move from BSS["MACFMT"] to BSS["MACFMT"].\n",
        MACARG(c->mac), MACARG(c->wif->bssInfo.bssid), MACARG(dst->bssInfo.bssid));

    dlist_remove(&c->l);
    c->wif = dst;
    dlist_add_tail(&dst->clients, &c->l);

    return 0;
}

struct client *clientAdd(struct wifi_interface *ifw, uint8_t *mac)
{
    struct client *c;
    if (!(c = clientFind(NULL, NULL, mac))) {
        c = clientNew(ifw, mac);
        if (!c)
            return NULL;
    }
    if (c->wif != ifw)
        clientMove(c, ifw);

    return c;
}

void clientDelete(struct client *c)
{
    if (!c)
        return;

    dlist_remove(&c->l);
    if (c->last_assoc.datap)
        free(c->last_assoc.datap);

    dlist_free_items(&c->seens, struct sta_seen, l);
    PLATFORM_CANCEL_TIMER(c->wait_seen_timer);
    PLATFORM_CANCEL_TIMER(c->btm_ctx.check_timer);
    PLATFORM_CANCEL_TIMER(c->btm_ctx.wait_rsp_timer);
    free(c);
}

struct sta_seen *seenFind(struct client *c, uint8_t *rid)
{
    struct sta_seen *seen;

    dlist_for_each(seen, c->seens, l) {
        if (!MACCMP(seen->rid, rid))
            return seen;
    }
    return NULL;
}

struct sta_seen *seenNew(struct client *c, uint8_t *rid)
{
    struct sta_seen *seen = calloc(1, sizeof(struct sta_seen));

    if (seen) {
        if (c)
            dlist_add_tail(&c->seens, &seen->l);
        MACCPY(seen->rid, rid);
    }
    return seen;
}

struct sta_seen *seenAdd(struct client *c, uint8_t *rid)
{
    struct sta_seen *seen;

    if (!(seen = seenFind(c, rid)))
        seen = seenNew(c, rid);

    return seen;
}

struct unassoc_sta_metrics_query_per_chan_item *unassocStaChanItemAdd(struct dlist_head *head,
        uint8_t channel)
{
    struct unassoc_sta_metrics_query_per_chan_item *item = NULL;

    dlist_for_each(item, *head, l) {
        if (item->chan == channel)
            return item;
    }

    item = calloc(1, sizeof(struct unassoc_sta_metrics_query_per_chan_item));

    if (!item)
        return NULL;

    item->chan = channel;
    dlist_head_init(&item->sta_list);
    dlist_add_tail(head, &item->l);

    return item;
}

void unassocStaChanItemDelete(struct unassoc_sta_metrics_query_per_chan_item *item)
{
    if (!item)
        return;

    dlist_free_items(&item->sta_list, struct mac_item, l);
    free(item);

    return;
}

struct al_device *alDeviceFindBySta(struct client *sta)
{
    if (!sta || !sta->wif || !sta->wif->radio)
        return NULL;

    return sta->wif->radio->d;
}

struct al_device *alDeviceFind(uint8_t *mac)
{
    struct al_device *ret;
    dlist_for_each(ret, local_network, l) {
        if (!MACCMP(ret->al_mac, mac))
            return ret;
    }
    return NULL;
}

struct al_device *alDeviceFindAny(uint8_t *mac)
{
    struct al_device *ret;
    dlist_for_each(ret, local_network, l) {
        struct interface *intf;
        if (!MACCMP(ret->al_mac, mac))
            return ret;
        dlist_for_each(intf, ret->interfaces, l) {
            if (!MACCMP(intf->mac, mac))
                return ret;
        }

    }
    return NULL;
}

struct al_device *alDeviceAdd(uint8_t *mac)
{
    struct al_device *d;

    if ((d = alDeviceFind(mac)))
        return d;
    else
        return alDeviceNew(mac);
}

uint8_t *_findNeighborInterfaceMac(uint8_t *remote_al, uint8_t *local_al, uint8_t *local_intf)
{
    struct al_device *remote = alDeviceFind(remote_al);

    if (remote) {
        struct interface *remote_intf;

        dlist_for_each(remote_intf, remote->interfaces, l) {
            struct neighbor *nei;
            dlist_for_each(nei, remote_intf->neighbors, l) {
                if (!MACCMP(local_al, nei->al_mac)) {
                    MACCPY(nei->intf_mac, local_intf);
                    return remote_intf->mac;
                }
            }
        }
    }

    return NULL;
}

struct neighbor *neighborAdd(struct interface *i, uint8_t *al_mac, uint8_t *mac)
{
    struct neighbor *n = NULL;

    if ((!mac) && (i->owner))
        mac = _findNeighborInterfaceMac(al_mac, i->owner->al_mac, i->mac);

    dlist_for_each(n, i->neighbors, l) {
        if (!MACCMP(n->al_mac, al_mac))
            break;
    }

    if (!n) {
        n = calloc(1, sizeof(struct neighbor));
        MACCPY(n->al_mac, al_mac);
        dlist_add_tail(&i->neighbors, &n->l);
    }

    if (mac)
        MACCPY(n->intf_mac, mac);

    return n;
}

struct neighbor *neighborFind(struct interface *i, uint8_t *al_mac, uint8_t *mac)
{
    struct neighbor *n = NULL;

    if ((!mac) && (i->owner))
        mac = _findNeighborInterfaceMac(al_mac, i->owner->al_mac, i->mac);

    if (!mac)
        return n;

    dlist_for_each(n, i->neighbors, l) {
        if (!MACCMP(n->al_mac, al_mac) && !MACCMP(n->intf_mac, mac))
            break;
    }
    return n;
}

void neighborDelete(struct neighbor *n)
{
    if (!n)
        return;

    dlist_remove(&n->l);
    free(n);
}

int neighborDeleteRelated(uint8_t *remote, uint8_t *neighbor)
{
    struct al_device *d;
    int ret = 0;

    if (!remote || !neighbor)
        return 0;

    if ((d = alDeviceFind(remote))) {
        struct interface *i;

        dlist_for_each(i, d->interfaces, l) {
            struct neighbor *n, *n_tmp;
            dlist_for_each_safe(n, n_tmp, i->neighbors, l) {
                if (!MACCMP(n->al_mac, neighbor)) {
                    neighborDelete(n);
                    ret++;
                }
            }
        }
    }
    return ret;
}

struct neighbor_bss *neighborBssFind(struct chan_info *chan, uint8_t *bssid)
{
    struct neighbor_bss *nbss = NULL;

    if (!chan || !bssid)
        return NULL;

    dlist_for_each(nbss, chan->neighbor_list, l) {
        if (!MACCMP(nbss->bssid, bssid))
            return nbss;
    }

    return NULL;
}

struct neighbor_bss *neighborBssNew(struct chan_info *chan, uint8_t *bssid)
{
    struct neighbor_bss *nbss = NULL;

    if (!chan || !bssid)
        return NULL;

    nbss = neighborBssFind(chan, bssid);
    if (!nbss) {
        nbss = calloc(1, sizeof(struct neighbor_bss));
        MACCPY(nbss->bssid, bssid);
        dlist_add_tail(&chan->neighbor_list, &nbss->l);
    }

    return nbss;
}

void neighborBssFlush(struct chan_info *chan) {
    if (!chan)
        return;

    if (!dlist_empty(&chan->neighbor_list))
        dlist_free_items(&chan->neighbor_list, struct neighbor_bss, l);
    dlist_head_init(&chan->neighbor_list);
}

struct interface *interfaceFindIdx(int idx)
{
    struct interface *ret;
    dlist_for_each(ret, local_device->interfaces, l) {
        if (ret->index == idx) {
            return ret;
        }
    }
    return NULL;
}

int configuratorAdd(struct configurator_ops *ops, void *data)
{
    int ret = -1;
    if (!local_configurator.ops) {
        local_configurator.ops = ops;
        local_configurator.data = data;
        ret = 0;
    }
    return ret;
}

int configuratorGetWifiConfig(dlist_head *bsses)
{
    if ((local_configurator.ops) && (local_configurator.ops->getWifiConfig))
        return (local_configurator.ops->getWifiConfig)(local_configurator.data, bsses);
    return -1;
}

int configuratorGetPolicy(struct map_policy *policy)
{
    if ((local_configurator.ops) && (local_configurator.ops->getPolicy))
        return (local_configurator.ops->getPolicy)(local_configurator.data, &local_policy);
    return -1;
}

int configuratorGetConfig(struct map_config *config)
{
    if ((local_configurator.ops) && (local_configurator.ops->getConfig))
        return (local_configurator.ops->getConfig)(local_configurator.data, &local_config);
    return -1;
}

int configuratorGetVlan(struct map_policy *policy)
{
    if ((local_configurator.ops) && (local_configurator.ops->getVlan))
        return (local_configurator.ops->getVlan)(local_configurator.data, &local_policy);
    return -1;
}

int configuratorGetDeviceInfo(struct device_info *info)
{

    if (!info)
        return -1;

    if ((local_configurator.ops) && (local_configurator.ops->getDeviceInfo)
        && (!(local_configurator.ops->getDeviceInfo(local_configurator.data, info)))) {
        return 0;
    } else {
        return -1;
    }
}

int configuratorGetIpInfo(dlist_head *ips)
{
    if ((local_configurator.ops) && (local_configurator.ops->getIpInfo))
        return (local_configurator.ops->getIpInfo)(local_configurator.data, ips);
    return -1;
}

int isRegistrar()
{
    return (registrar == local_device);
}

int setRegistrar(struct al_device *d)
{
    if (registrar == d)
        return 0;
    if (d) {
        DEBUG_INFO("set registrar "MACFMT"\n", MACARG(d->al_mac));
        d->is_controller = 1;
        PLATFORM_CANCEL_TIMER(local_device->autorole_timer);
    } else {
        DEBUG_INFO("set registrar to NULL\n");
        if (local_config.auto_role)
            doAutoRole();
    }

    //delete cilent metrics report timer
    local_device->metrics_rpt_interval = 0;
    PLATFORM_CANCEL_TIMER(local_device->metrics_rpt_timer);

    registrar = d;

    if (d!=local_device) {
        alDeviceSetConfigured(local_device, 0);
        if (local_config.auto_role) {
            /* send ubus event */
            struct mesh_ubus_event_request req;
            req.u.role_change.old_role = local_device->is_controller ? 1 : 0; // auto
            req.u.role_change.new_role = 2; // agent
            sendUbusEvent(EVENT_ROLE_CHANGE, &req);
            local_device->is_agent = 1;
            local_device->is_controller = 0;
        }
    }

    return 0;
}

void radioApplyBsses(struct radio *r, dlist_head *pbsses, struct bss_info *backhaul_bss, uint8_t sync_sta)
{
    struct wifi_config *wcfg;
    int ret;

    DEBUG_INFO("== APPLY BSS on radio("MACFMT")\n", MACARG(r->uid));

    ret = radioTeardown(r);
    dlist_for_each(wcfg, *pbsses, l) {
        struct bss_info *bss = &wcfg->bss;

        printBSSConfig(bss);

        ret = radioAddAP(r, bss, backhaul_bss);
        if (ret)
            DEBUG_INFO("apply failed\n");

    }

    if ((backhaul_bss) && (sync_sta))
        ret = radioAddSTA(r, backhaul_bss);
    ret = radioCommit(r);
}

char *idx2InterfaceName(uint32_t idx)
{
    struct interface *intf;

    if (idx) {
        if ((intf = interfaceFindIdx(idx)))
            return (char *)intf->name;
        else
            return NULL;
    } else
        return "dummy";
}

uint8_t *interfaceName2MAC(char *name)
{
    struct interface *intf;
    if ((local_device) && (name)) {
        dlist_for_each(intf, local_device->interfaces, l) {
            if (!strcmp(name, intf->name))
                return intf->mac;
        }
    }
    return NULL;
}


struct band_width g_bwStr[] = {
    { bw_idx_20MHz, "20" },
    { bw_idx_40MHz, "40" },
    { bw_idx_80MHz, "80" },
    { bw_idx_160MHz, "160" },
    { bw_idx_80P80MHz, "80+80"},
};

enum e_wifi_band_index bandwidthStrToIndex(char *bwstr)
{
    int i = 0;
    uint8_t len = 0;

    for (i = 0; i < ARRAY_SIZE(g_bwStr); i++) {
        if ((len == strlen(g_bwStr[i].str)) && strncmp(g_bwStr[i].str, bwstr, len))
            break;
    }
    if (i < ARRAY_SIZE(g_bwStr))
        return g_bwStr[i].bw;
    else
        return bw_idx_20MHz;
}

char *bandwidth2String(uint8_t bw)
{
    if (bw >= ARRAY_SIZE(g_bwStr))
        bw = 0;
    return g_bwStr[bw].str;
}

int bandwidthToIndex(uint8_t band)
{
    int i, idx = 0;

    if (band == 25)
        return idx;

    for (i = 1; i <= 4 && band ; i++)
    {
        if (band == ((1 << i) * 10))
        {
            idx = i - 1;
            return idx;
        }
    }
    return 0;
}

struct operating_class *opclassFind(struct radio *r, uint8_t opclass)
{
    int i;

    for (i=0;i<r->num_opc_support;i++) {
        if (r->opclasses[i].op_class == opclass)
            return &r->opclasses[i];
    }
    return NULL;
}

struct operating_class *opclassAdd(struct radio *r, uint8_t opclass)
{
    struct operating_class *ret;

    if ((!(ret = opclassFind(r, opclass))) && (r->num_opc_support<MAX_OPCLASS)) {
        ret = &r->opclasses[r->num_opc_support++];
        ret->op_class = opclass;
    }

    return ret;
}

struct chan_info *channelFind(struct operating_class *opclass, uint8_t channel)
{
    int i;

    for (i = 0; i < opclass->num_support_chan; i++) {
        if (opclass->channels[i].id == channel)
            return &opclass->channels[i];
    }
    return NULL;
}

int initOperatingClass(struct operating_class *opclass, uint8_t disable)
{
    struct opclass_channel_table *t;
    int num = 0;

    if ((!opclass) || (!(t=findOperatingChannelTable(opclass->op_class))))
        return -1;

    if (!opclass->num_support_chan) {
        while (t->chan_set[num]) {
            INIT_CHANNEL_INFO(&opclass->channels[num], t->chan_set[num], disable);
            num++;
        }
        opclass->num_support_chan = num;
    } else {
        for (num = 0;num<opclass->num_support_chan;num++) {
            opclass->channels[num].disable = disable;
            opclass->channels[num].scan_status = SCAN_STATUS_INIT;
        }
    }
    return 0;
}

int resetOperatingClass(struct operating_class *opclass, uint8_t pref, uint8_t reason)
{
    int i;

    for (i = 0;i<opclass->num_support_chan;i++) {
        struct chan_info *info = &opclass->channels[i];

        info->pref = pref;
        info->reason = reason;
    }
    return 0;
}

int validateOperatingChannel(struct radio *r, uint8_t opclass, uint8_t channel)
{
    struct operating_class *class;
    struct chan_info *info;

    if ((class = opclassFind(r, opclass)) && (info = channelFind(class, channel)) &&
                    (!info->disable))
        return 0;
    return -1;
}

int dmSweep(void)
{
    int ret = 0;
    struct al_device *d, *tmp;
    uint32_t now = PLATFORM_GET_TIMESTAMP(0);

    dlist_for_each_safe(d, tmp, local_network, l) {
        if (d == local_device)
            continue;
        if (now - d->ts_alive>(local_config.age_timeout*1000)) {
            DEBUG_INFO("Age out "MACFMT"\n", MACARG(d->al_mac));
            if (d==registrar)
                setRegistrar(NULL);
            alDeviceDelete(d);
            ret++;
        }
    }
    return ret;
}

struct client * check_beacon_request_params(uint8_t *bssid, uint8_t *sta)
{
    struct wifi_interface *vap;
    struct client *sta_info;

    if (IS_ZERO_MAC(bssid)) {
        DEBUG_WARNING("bssid in beacon metrics query is zero\n");
        return NULL;
    }
    if (IS_WILDCARD_MAC(bssid)) { /* wildcard mac means scan all bss in target channel*/
        sta_info = clientFind(local_device, NULL,sta);
        if (sta_info)
            return sta_info;
    }
    vap = (struct wifi_interface *)interfaceFind(local_device, bssid, interface_type_wifi);
    if (!vap) {
        DEBUG_WARNING("can NOT find vap["MACFMT"] locally\n", MACARG(bssid));
        return NULL;
    }
    sta_info = clientFind(local_device, vap, sta);
    if (!sta_info) {
        DEBUG_WARNING("can NOT find sta["MACFMT"] in vap["MACFMT"]\n", MACARG(sta), MACARG(bssid));
        return NULL;
    }
    if (checkBeaconReportSupported(sta_info->ies.rm_enabled) == 0) {
        DEBUG_WARNING("sta["MACFMT"]'s beacon measurement cap has wrong value \n", MACARG(sta_info->mac));
        return NULL;
    }
    return sta_info;
}

#if 0
uint16_t check_and_fill_meas_elem(uint8_t *elem, struct client *sta)
{
    struct measure_elem_item *item;
    uint8_t *tmp = NULL;

    dlist_for_each(item, sta->measure_elem_list, l) {
        if (item->id == elem[0]) {  /* only the requested element will be stored */
            if (item->len < elem[1]) {
                tmp = realloc(item->elem, elem[1]);
                if (!tmp)
                    break;
                item->elem = tmp;
                memcpy(item->elem, &elem[2], elem[1]);
            }
            else {
                memset(item->elem, 0, item->len);
                memcpy(item->elem, &elem[2], elem[1]);
            }
            item->len = elem[1];
            break;
        }
    }
    return (elem[1] + 2);
}
#endif

int parseChanScanResults(struct radio *r, struct chscan_req_item *req_item,
            dlist_head *head, bool first_flag)
{
    int i = 0;
    struct chan_scan_result_item *item = NULL;
    struct operating_class *opclass = NULL;
    struct ieee80211_ies ies;
    uint8_t ssid[MAX_SSID_LEN+2] = {0};
    uint16_t ssid_len = 0;

    if (!r || !head)
        return -1;

    if (!r->chscan_req) {
        DEBUG_WARNING("radio has no ongoing channel scan request.\n");
        return -1;
    }

    if (!req_item) {
        DEBUG_WARNING("req_item is NULL.\n");
        return -1;
    }

    opclass = opclassFind(r, req_item->opclass);
    if (!opclass) {
        DEBUG_WARNING("radio(%s) find opclass(%u) failed.\n", r->name, req_item->opclass);
        return -1;
    }

    for (i = 0; i < req_item->ch_num; i++) {
        struct chan_info *chan = channelFind(opclass, req_item->chans[i]);
        if (!chan)
            continue;

        /* first update need clear history results */
        if (first_flag) {
            neighborBssFlush(chan);
        }

        dlist_for_each(item, *head, l) {
            if (req_item->chans[i] != item->channel)
                continue;

            memset(&ies, 0, sizeof(ies));
            memset(ssid, 0, sizeof(ssid));
            ssid_len = 0;
            struct neighbor_bss *nbss = neighborBssNew(chan, item->bssid);
            if (!nbss) {
                DEBUG_WARNING("neighborBssNew return NULL.\n");
                return -1;
            }
            nbss->bw = bw_idx_20MHz;
            nbss->bss_load_element_present = 0;
            nbss->signal_stength = item->signal;
            nbss->last_seen = item->last_seen_ms;
            if (parse80211IEs(item->ies, item->ies_len, 0, &ies)) {
                nbss->bw = parseBWFromIEs(item->channel > 14, &ies);
                if (parseBssLoadIE(ies.bss_load, &nbss->sta_cnt, &nbss->chan_utilize))
                    nbss->bss_load_element_present |= BSS_LOAD_ELE_PRESENT;
                if (!parseSsidIE(ies.ssid, ssid, &ssid_len))
                    parseMeshIdIE(ies.meshid, ssid, &ssid_len);
                if (ssid_len > 0) {
                    memcpy(nbss->ssid.ssid, ssid, ssid_len);
                    nbss->ssid.ssid[ssid_len] = 0;
                    nbss->ssid.len = ssid_len;
                }
                nbss->bss_color = parseHeopInfoBssColor(ies.heop);
            }
        }
    }

    return 0;
}

void buildCapaValue(struct client *client)
{
    if((client) && (client->ies.ht_cap)) {
        parseHTCapaIE(&client->bands_capa, client->ies.ht_cap);
    }
    if((client) && (client->ies.vht_cap)) {
        parseVHTCapaIE(&client->bands_capa, client->ies.vht_cap);
    }
    if((client) && (client->ies.he_cap)) {
        parseHECapaIE(&client->bands_capa, client->ies.he_cap);
    }
    return;
}

static int detectAssocOffset(struct client *client, struct wifi_interface *ifw)
{
    uint8_t *frm = client->last_assoc.datap;
    int frm_len = client->last_assoc.len;
    uint16_t ssid_len = 0;

    ssid_len = strlen((char *)ifw->bssInfo.ssid.ssid);

    if (frm_len<12)
        return 4;
    else if (frm[4] != 0)
        return 10;
    else if (frm[5]>32)
        return 10;
    else if (!PLATFORM_MEMCMP(frm+4, ifw->i.mac, 6))
        return 10;
    else if ((ifw) && (ssid_len) && (frm_len>=6+ssid_len)
        && (ssid_len==frm[5])
        && (!PLATFORM_MEMCMP(frm+6, ifw->bssInfo.ssid.ssid, ssid_len)))
        return 4;
    else if ((ifw) && (ssid_len) && (frm_len>=12+ssid_len)
        && (ifw->bssInfo.ssid.len==frm[115])
        && (!PLATFORM_MEMCMP(frm+12, ifw->bssInfo.ssid.ssid, ssid_len)))
        return 10;
    return 0;
}

uint8_t parseAssocFrame(struct client *client, uint8_t offset)
{
    uint8_t *frm,  *efrm;
    uint8_t *rm_enabled = NULL;
    uint8_t *extcap = NULL;
    uint8_t *ht_cap = NULL;
    uint8_t *vht_cap = NULL;
    uint8_t *he_cap = NULL;
    uint8_t *supported_rates = NULL;

    memset(&client->ies, 0, sizeof(client->ies));
    client->bsta = 0;

    if ((!client->last_assoc.datap) || (client->last_assoc.len == 0))
        return -1;

    if ((offset == 0) && ((offset = detectAssocOffset(client, client->wif))==0)) {
        DEBUG_ERROR("Can not parse re/assocation frame!\n");
        return -1;
    }

    frm = client->last_assoc.datap;
    efrm = frm + client->last_assoc.len;
    frm += offset;

    while (frm + 2 <= efrm)
    {
        switch (frm[0])
        {
            case IEEE80211_ELEMID_SUPPORTED_RATES:
                supported_rates = frm;
                break;
            case IEEE80211_ELEMID_RM_ENABLED:
                if (checkRMCapabilitiesIE(frm))
                    rm_enabled = frm;
                break;
            case IEEE80211_ELEMID_EXTCAP:
                if (checkExtCapabilitiesIE(frm))
                    extcap = frm;
                break;
            case IEEE80211_ELEMID_HTCAP:
                if (checkHTCapabilitiesIE(frm))
                    ht_cap = frm;
                break;
            case IEEE80211_ELEMID_VHTCAP:
                if (checkVHTCapabilitiesIE(frm))
                    vht_cap = frm;
                break;
            case IEEE80211_ELEMID_EXT:
                if(checkHECapabilitiesIE(frm))
                    he_cap = frm;
                break;
            case IEEE80211_ELEMID_VENDOR:
                if (!PLATFORM_MEMCMP(frm + 2, WFA_OUI, 3) && frm[5] == WFA_OUITYPE_MAP)
                    client->bsta = !!(frm[8] & WIFI_MAP_BACKHAUL_STA);
                break;
        }
        frm += frm[1] + 2;
    }
    if (frm != efrm)
        return -1;

    client->ies.rm_enabled = rm_enabled;
    client->ies.extcap = extcap;
    client->ies.ht_cap = ht_cap;
    client->ies.vht_cap = vht_cap;
    client->ies.he_cap = he_cap;
    client->ies.supported_rates = supported_rates;
    buildCapaValue(client);
    return 0;
}

uint8_t checkBeaconReportSupported(uint8_t *rm_enabled)
{
    struct ieee80211_ie_rmcap *rm_enabled_cap = NULL;

    rm_enabled_cap = (struct ieee80211_ie_rmcap *)(rm_enabled);
    if (!rm_enabled_cap)
        return 0;

    return (!!IEEE80211_RMCAP_BEACON_PASSIVE_MEASURE(rm_enabled_cap->cap) || !!IEEE80211_RMCAP_BEACON_ACTIVE_MEASURE(rm_enabled_cap->cap)
            || !!IEEE80211_RMCAP_BEACON_TABLE_MEASURE(rm_enabled_cap->cap) || !!IEEE80211_RMCAP_BEACON_MEASURE(rm_enabled_cap->cap));
}

uint8_t checkBtmSupported(uint8_t *extcap)
{
    struct ieee80211_ie_extcap *ext_cap = NULL;

    ext_cap = (struct ieee80211_ie_extcap *)(extcap);
    if (!ext_cap)
        return 0;

    return !!IEEE80211_EXCAP_BSS_TRANSITION(ext_cap->ext_cap);
}

uint8_t checkChanSelRequestValid(struct operating_class *opc, uint8_t *chans)
{
    /* check whether we should accept the channel selection request */
    /* 0 means accept, other value means refused */
    /* for now, it will always return 0, except the step-10 in R4#testcase-4.5.1. */
    /* so the testcase can be continued. */
    struct chan_info *ch;
    int i;

    /* step-10 in R4#testcase-4.5.1 */
    for (i = 0; i < chans[0]; i++) {
        ch = channelFind(opc, chans[1 + i]);
        if (!ch)
            return CHAN_SEL_RESPCODE_VIOLATE_RECENT; /* just meet the response code in test plan */
    }
    return CHAN_SEL_RESPCODE_ACCEPT;
}

int selectBestChannel(struct radio *r, uint8_t *popclass, uint8_t *pchannel)
{
    int i, k;
    struct operating_class *cur_opc, *target_opc;
    struct chan_info *cur_ch, *target_ch;

    if ((!r) || (!popclass) || (!pchannel))
        return -1;

    if ((!(cur_opc = opclassFind(r, r->opclass))) ||
        (!(cur_ch = channelFind(cur_opc, r->channel)))) {
        DEBUG_WARNING("can NOT find current opclass/channel\n");
        return -1;
    }

    target_ch = cur_ch;
    for (i = 0; i < r->num_opc_support; i++) {
        for (k = 0; k < r->opclasses[i].num_support_chan; k++) {
            if (r->opclasses[i].channels[k].pref > target_ch->pref) {
                target_opc = &r->opclasses[i];
                target_ch = &r->opclasses[i].channels[k];
                if ((!target_ch->disable) && (target_ch->pref == MOST_PREF_SCORE))
                    goto out;
            }
        }
    }

out:
    if (target_ch != cur_ch) {
        *popclass = target_opc->op_class;
        *pchannel = target_ch->id;

        return 1;
    } else
        return 0;
}

int updateRadioMetrics(struct radio *r, uint8_t cur_util)
{
    int ret = 0;

    if (!r)
        return ret;

    if ((r->ch_util) && (r->metrics_rpt_policy.ap_chutil_thresh)){
        ret = CROSSED_THRESHOLD(r->metrics_rpt_policy.ap_chutil_thresh, r->ch_util, cur_util);
    }

    r->ch_util = cur_util;
    return ret;
}

struct wifi_interface *findLocalOtherRadioBss(struct client *c)
{
    struct interface *intf = NULL;
    struct wifi_interface *bss = NULL;
    int min_sta_count = -1;

    if (!c || !c->wif || !c->wif->radio)
        return NULL;

    dlist_for_each(intf, local_device->interfaces, l) {
        if (intf->type != interface_type_wifi || ((struct wifi_interface *)intf)->role != role_ap)
            continue;
        if (!BSS_SSID_IS_SAME(c->wif, (struct wifi_interface *)intf))
            continue;
        if (c->wif->radio == ((struct wifi_interface *)intf)->radio)
            continue;
        if (!bss || dlist_count(&bss->clients) < min_sta_count) {
            bss = (struct wifi_interface *)intf;
            min_sta_count = dlist_count(&bss->clients);
        }
    }

    return bss;
}

struct wifi_interface *findSpecificBandBss(struct client *c, enum e_wifi_band_idx band_idx)
{
    struct interface *intf = NULL;
    struct wifi_interface *bss = NULL;
    int min_sta_count = -1;

    if (!c || !c->wif || !c->wif->radio)
        return NULL;

    dlist_for_each(intf, local_device->interfaces, l) {
        if (intf->type != interface_type_wifi || ((struct wifi_interface *)intf)->role != role_ap)
            continue;
        if (((struct wifi_interface *)intf)->radio->current_band_idx != band_idx)
            continue;
        if (!BSS_SSID_IS_SAME(c->wif, (struct wifi_interface *)intf))
            continue;
        if (!bss || dlist_count(&bss->clients) < min_sta_count) {
            bss = (struct wifi_interface *)intf;
            min_sta_count = dlist_count(&bss->clients);
        }
    }

    return bss;
}

int getPeriodForChannel(uint8_t channel, uint8_t opclass, struct radio *r, uint32_t *period_cac, uint32_t *unoccup)
{
    struct operating_class *opc = NULL;
    struct chan_info *ch = NULL;

    opc = opclassFind(r, opclass);
    if (!opc) {
        DEBUG_WARNING("can NOT find opclass(%d) for radio["MACFMT"]\n", opclass, MACARG(r->uid));
        return 0;
    }
    ch = channelFind(opc, channel);
    if(!ch) {
        DEBUG_WARNING("can NOT find ch[%d] in opc[%d] for radio["MACFMT"]\n", channel, opclass, MACARG(r->uid));
        return 0;
    }
    *period_cac = r->cac_capa[r->cur_cac.cur_scan.method].duration;
    return 1;
}

int updateOperatingChannelReport(struct radio *r)
{
    uint32_t freq = channel2Freq(r->channel, r->opclass);
    int list[] = {NL80211_CHAN_WIDTH_160, NL80211_CHAN_WIDTH_80, NL80211_CHAN_WIDTH_40,
                    NL80211_CHAN_WIDTH_20, -1};
    int *pbw = list;
    uint32_t bw = opclass2nlBandwidth(r->opclass, NULL);
    uint8_t opclass, channel;
    int i = 1;

    if (!freq)
        return -1;

    RESET_CURRENT_CHANNEL(r->current_opclass, r->current_channel);

    r->current_opclass[0] = r->opclass;
    r->current_channel[0] = freq2SpecChannel(freq, r->opclass);

    if (bw <= NL80211_CHAN_WIDTH_20)
        return 0;

    while ((*pbw)>=bw)
        pbw++;

    while ((*pbw)!=-1) {
        int sec_chan = 0;
        if (*pbw == NL80211_CHAN_WIDTH_40) {
            //only for 80 or above on 5ghz
            uint32_t cfreq = getCf1(NL80211_CHAN_WIDTH_40, freq, 0);
            sec_chan = (freq<=cfreq) ? 1:-1;
        }
        if ((!ieee80211Freq2ChannelExt(freq, sec_chan,  *pbw, &opclass, &channel))
            && (opclass2nlBandwidth(opclass, NULL)==*pbw))  {
            r->current_opclass[i] = opclass;
            r->current_channel[i++] = freq2SpecChannel(freq, opclass);
        }
        pbw++;
    }
    return 0;
}

int generateNewBssid(struct al_device *d, struct radio *r, uint8_t *new_bssid)
{
    int index = 0;

    if (!d || !r || !new_bssid)
        return -1;

    /* TODO: need match mask */
    MACCPY(new_bssid, r->uid);
    index = dlist_count(&d->interfaces) + 1;
    for(int i = 5; i >= 0; i--) {
        index += new_bssid[i];
        new_bssid[i] = index & 0xFF;
        index >>= 8;
    }

    if (IS_ZERO_MAC(new_bssid)) {
        DEBUG_ERROR("generate new bssid failed. radio("MACFMT")\n", MACARG(r->uid));
        return -1;
    }

    DEBUG_INFO("generate new bssid("MACFMT"). radio("MACFMT")\n", MACARG(new_bssid), MACARG(r->uid));

    return 0;
}

void neighborAgeHandle(struct al_device *d)
{
    struct interface *i;
    struct neighbor *n, *n_tmp;
    uint32_t current_ts = PLATFORM_GET_TIMESTAMP(0);

    if (!d)
        return;

    dlist_for_each(i, d->interfaces, l) {
        dlist_for_each_safe(n, n_tmp, i->neighbors, l) {
            if ((current_ts - n->last_lldp_discovery > 90000)
                && (current_ts - n->last_1905_discovery > 90000))
                neighborDelete(n);
        }
    }
}

void denyStaTimerHandle(void *data)
{
    struct deny_sta_info *param = (struct deny_sta_info *)data;
    struct wifi_interface *wif = NULL;

    if (!param)
        return;

    wif = (struct wifi_interface *)interfaceFind(NULL, param->bssid, interface_type_wifi);
    if (!wif) {
        DEBUG_ERROR("del sta["MACFMT"] from deny acl list failed for wifi interface not exist!\n", MACARG(param->mac));
        return;
    }

    staDeny(wif, param->mac, 0);
    DEBUG_INFO("del sta["MACFMT"] from deny acl list\n", MACARG(param->mac));

    PLATFORM_FREE(param);

    return;
}

struct policy_param_metrics_rpt *findReportPolicy(uint8_t band_idx)
{
    for (int i = 0; i < band_max_idx; i++) {
        if (local_policy.metrics_rpt[i].band_idx == band_idx)
            return &local_policy.metrics_rpt[i].rpt_policy;
    }

    return NULL;
}

void controllerWeightReset(uint8_t weight)
{
    uint8_t old_weight = local_device->controller_weight;
    DEBUG_INFO("API set controller_weight: %u, old_weight: %u\n", weight, old_weight);

    if (local_device->controller_weight == weight)
        return;

    local_device->controller_weight = weight;
    if (!local_config.auto_role)
        return;

    /* if auto role Controller weight become smaller. doAutoRole again */
    if (isRegistrar() && old_weight > weight) {
        DEBUG_INFO("local is Controller and old_weight(%u) > controller_weight(%u) doAutoRole again\n", old_weight, weight);
        setRegistrar(NULL);
    }
    /* if weight bigger than Controller doAutoRole again */
    else if (registrar && registrar->controller_weight < weight) {
        DEBUG_INFO("local controller_weight(%u) > registrar controller_weight(%u) set Registrar to NULL and doAutoRole again\n",
            weight, registrar->controller_weight);
        setRegistrar(NULL);
    }
}

