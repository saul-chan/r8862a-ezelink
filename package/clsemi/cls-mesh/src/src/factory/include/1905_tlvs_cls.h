#ifndef _1905_TLVS_CLS_H_
#define _1905_TLVS_CLS_H_


#define CLS_OUI        "\xd0\x44\x33"

enum cls_tlv_type {
    CLS_TLV_TYPE_VIP_CONF = 0x00,
    CLS_TLV_TYPE_EGRESS_CONF = 0x01,
    CLS_TLV_TYPE_DSCP_MAPPING_CONF = 0x02,
    CLS_TLV_TYPE_CLS_CAPABILITIES = 0x03,
    CLS_TLV_TYPE_TC_MAPPING_CONF = 0x04,
    CLS_TLV_TYPE_CONTROLLER_WEIGHT = 0x05,

    CLS_TLV_TYPE_MAX
};

struct TLVDesc *getCLSTLVDesc(uint16_t type);
struct TLV *CLSTLVNew(dlist_head *parent, uint16_t type, uint32_t size);
struct TLV *CLSVendorTLVNew(struct TLV *cls_tlv);
#endif
