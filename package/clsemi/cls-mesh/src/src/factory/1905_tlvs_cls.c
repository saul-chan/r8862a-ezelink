#include "platform.h"

#include "1905_tlvs.h"
#include "tlv.h"

DECLARE_STRUCT_DESC_GLOBAL(DSCPMappingStruct) =
    TLV_DESC_3FIELDS(DSCPMappingStruct, 1, dscp_value, fmt_unsigned, swq_id, fmt_unsigned, wmm_tid, fmt_unsigned, NULL);
DECLARE_STRUCT_DESC_GLOBAL(swQueueStruct) =
    TLV_DESC_3FIELDS(swQueueStruct, 1, qid, fmt_unsigned, weight, fmt_unsigned, port, fmt_unsigned, NULL);
DECLARE_STRUCT_DESC_GLOBAL(TcMappingStruct) =
    TLV_DESC_3FIELDS(TcMappingStruct, 1, tc, fmt_unsigned, tid, fmt_unsigned, qid, fmt_unsigned, NULL);


static struct TLVDesc _cls_tlv_descs[CLS_TLV_TYPE_MAX] = {
    [CLS_TLV_TYPE_VIP_CONF] =
            TLV_DESC_0FIELD(TLV, 0, &SNAME(macStruct), NULL),
    [CLS_TLV_TYPE_CLS_CAPABILITIES] =
            TLV_DESC_1FIELD(clsCapTLV, 0, cap, fmt_l1vv, NULL),
    [CLS_TLV_TYPE_DSCP_MAPPING_CONF] =
            TLV_DESC_2FIELDS(DSCPMappingTLV, 0, dft_swq, fmt_unsigned, dft_tid, fmt_unsigned,
                                &SNAME(DSCPMappingStruct), NULL),
    [CLS_TLV_TYPE_EGRESS_CONF] =
            TLV_DESC_0FIELD(TLV, 0, &SNAME(swQueueStruct), NULL),
    [CLS_TLV_TYPE_TC_MAPPING_CONF] =
            TLV_DESC_2FIELDS(TcMappingTLV, 0, dft_tid, fmt_unsigned, dft_qid, fmt_unsigned,
                                &SNAME(TcMappingStruct), NULL),
    [CLS_TLV_TYPE_CONTROLLER_WEIGHT] =
            TLV_DESC_1FIELD(controllerWeightTLV, 0, weight, fmt_unsigned, NULL),
};


struct TLVDesc *getCLSTLVDesc(uint16_t type)
{
    return getDesc(_cls_tlv_descs, CLS_TLV_TYPE_MAX, type);
}

struct TLV *CLSTLVNew(dlist_head *parent, uint16_t type, uint32_t size)
{
    struct TLVDesc *desc = getCLSTLVDesc(type);
    struct TLV *tlv = (struct TLV *)TLVStructNew(desc, parent, size);
    if (tlv) {
        tlv->tlv_subtype = type;
    }
    return tlv;
}

struct TLV *CLSVendorTLVNew(struct TLV *cls_tlv)
{
    struct vendorTLV *tlv = NULL;

    if (cls_tlv) {
        tlv = (struct vendorTLV *)TLVNew(NULL, TLV_TYPE_VENDOR_SPECIFIC, 0);
        OUICPY(tlv->oui, CLS_OUI);
        tlv->super.sub_tlv = cls_tlv;
    }

    return (struct TLV *)tlv;
}

struct DSCPMappingStruct *mappingConfAddEntry(struct TLV *tlv, struct DSCP_mapping_item *conf)
{
    struct DSCPMappingStruct *mapping =
        (struct DSCPMappingStruct *)TLVStructDeclareGlobal(DSCPMappingStruct, &tlv->s.t.childs[0]);

    mapping->dscp_value = conf->dscp_value;
    mapping->swq_id = conf->queue_id;
    mapping->wmm_tid = conf->tid;
    return mapping;
}

struct swQueueStruct *egressQConfAddQueue(struct TLV *tlv, struct queue_conf_item *conf)
{
    struct swQueueStruct *queue =
        (struct swQueueStruct *)TLVStructDeclareGlobal(swQueueStruct, &tlv->s.t.childs[0]);

    queue->qid = conf->queue_id;
    queue->weight = conf->weight;
    queue->port = conf->port_id;
    return queue;
}

struct TcMappingStruct *TcConfAddEntry(struct TLV *tlv, struct tc_mapping_item *conf)
{
    struct TcMappingStruct *mapping =
        (struct TcMappingStruct *)TLVStructDeclareGlobal(TcMappingStruct, &tlv->s.t.childs[0]);

    mapping->tc = conf->tc_value;
    mapping->tid = conf->tid;
    mapping->qid = conf->queue_id;
    return mapping;
}

