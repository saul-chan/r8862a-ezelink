#include "platform.h"
#include "extension.h"
#include "feature/feature.h"
#include "datamodel.h"
#include "platform_os.h"
#include "al_send.h"
#include "al_utils.h"
#include "al_msg.h"

static void _topologyReNewTimer(void *data)
{
    struct al_device *d;
    uint32_t current_ts = PLATFORM_GET_TIMESTAMP(0);

    dlist_for_each(d, local_network, l) {
        if (d!=local_device) {
            if (PLATFORM_GET_AGE(d->ts_topo_resp)>30000) {
                sendTopologyQuery(idx2InterfaceName(d->recv_intf_idx), getNextMid(), d->al_mac);
                sendHighLayerQuery(idx2InterfaceName(d->recv_intf_idx), getNextMid(), d->al_mac);
                sendLinkMetricsQuery(idx2InterfaceName(d->recv_intf_idx), getNextMid(), d->al_mac, NULL,
                                        TX_LINK_METRICS_ONLY);
            }
            if (!d->ap_basic_capaed) {
                sendAPCapabilityQuery(idx2InterfaceName(d->recv_intf_idx), getNextMid(), d->al_mac);
            } else {
                if (isRegistrar() && (d->configured == STATUS_UNCONFIGURED)) {
                    if ((d->set_unconfigured_ts > 0) &&
                        (current_ts - d->set_unconfigured_ts) > 15*1000) {
                        sendAPAutoconfigurationRenew(getNextMid(), IEEE80211_FREQUENCY_BAND_2_4_GHZ, d);
                        d->set_unconfigured_ts = current_ts;
                    }
                }
            }
        }
    }
}


static int _topologyDeviceNew(void *data, uint8_t *p, uint16_t len)
{
    struct msg_attr attr[1];
    int type;
    while ((type = msgaParseOne(attr, &p, &len)) != attr_none) {
        switch (type) {
            case attr_mac:
            {
                struct al_device *d = alDeviceFind(msgaGetBin(attr));

                if (d) {
                    sendTopologyDiscovery(idx2InterfaceName(d->recv_intf_idx), getNextMid());
                    sendAPCapabilityQuery(idx2InterfaceName(d->recv_intf_idx), getNextMid(), d->al_mac);
                    sendTopologyQuery(idx2InterfaceName(d->recv_intf_idx), getNextMid(), d->al_mac);
                }
                break;
            }
            default:
                break;
        }
    }

    return 0;
}

static int _topologyStart(void *p, char *cmdline)
{
    featSuscribeEvent(feat_evt_dm_device_update, _topologyDeviceNew, NULL);
    platformAddTimer(5000, TIMER_FLAG_PERIODIC, _topologyReNewTimer, NULL);
    return 0;
}

static struct extension_ops _topology_ops = {
    .init = NULL,
    .start = _topologyStart,
    .stop = NULL,
    .deinit = NULL,
};


void topologyFeatLoad()
{
    registerExtension(&_topology_ops);
}
