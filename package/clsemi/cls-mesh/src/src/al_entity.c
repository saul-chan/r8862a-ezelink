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

// In the comments below, every time a reference is made (ex: "See Section 6.4"
// or "See Table 6-11") we are talking about the contents of the following
// document:
//
//   "IEEE Std 1905.1-2013"

#include "platform.h"
#include "utils.h"
#include "packet_tools.h"

#include "1905_tlvs.h"
#include "1905_cmdus.h"
#include "1905_l2.h"
#include "lldp_tlvs.h"
#include "lldp_payload.h"

#include "al.h"
#include "al_action.h"
#include "al_send.h"
#include "al_recv.h"
#include "al_utils.h"
#include "al_msg.h"
#include "al_driver.h"

#include "linux/platform_interfaces_priv.h"

#include "platform_interfaces.h"
#include "platform_os.h"
#include "al_wsc.h"
#include "extension.h"
#include "feature/feature.h"
#include "feature/feature_helper.h"
#include "feature/ubus/ubus_helper.h"
#include "wifi.h"

#define TIMER_TOKEN_DISCOVERY          (1)
#define TIMER_TOKEN_GARBAGE_COLLECTOR  (2)
#define TIMER_TOKEN_AUTOCONFIG         (3)

#define FRAME_MATCH_DESC(_tag, _type, _content, _matchlen, _process) \
    {.tag = (_tag), .type = (_type), .match = (_content), .match_len = (_matchlen), .process=(_process)}

#define REPORT_AP_METRICS_MASK BIT(0)
#define REPORT_OPERATING_CHANNEL_MASK BIT(1)

#define CHECK_AND_REPORT(_old, _new, _action, _report, _mask) \
    do {\
        if ((_old) != (_new)) {\
            (_old) = (_new);\
            (_report) |= (_mask);\
            _action;\
        }\
    }while(0)

static int _sendTunnledMsg(struct frame_match_desc *desc, uint8_t *p, uint16_t len);

static struct frame_match_desc _default_bss_rx_matches[] = {
    FRAME_MATCH_DESC(MSG_ASSOC_REQUEST, IEEE80211_FC0_SUBTYPE_ASSOC_REQ, NULL, 0, _sendTunnledMsg),
    FRAME_MATCH_DESC(MSG_REASSOC_REQUEST, IEEE80211_FC0_SUBTYPE_REASSOC_REQ, NULL, 0, _sendTunnledMsg),
    //btm query
    FRAME_MATCH_DESC(MSG_BTM_QUERY, IEEE80211_FC0_SUBTYPE_ACTION, (uint8_t *)"\x0a\x06", 2, _sendTunnledMsg),
    //wnm notification request
    FRAME_MATCH_DESC(MSG_WNM_REQUEST, IEEE80211_FC0_SUBTYPE_ACTION, (uint8_t *)"\x0a\x1a", 2, _sendTunnledMsg),
    //gas initial request/anqp request
    FRAME_MATCH_DESC(MSG_ANQP_REQUEST, IEEE80211_FC0_SUBTYPE_ACTION, (uint8_t *)"\x04\x0a", 2, _sendTunnledMsg),
    //protected Gas initial request/anqp request
    FRAME_MATCH_DESC(MSG_ANQP_REQUEST, IEEE80211_FC0_SUBTYPE_ACTION, (uint8_t *)"\x09\x0a", 2, _sendTunnledMsg),
    //btm response
    FRAME_MATCH_DESC(MSG_INTERESTING, IEEE80211_FC0_SUBTYPE_ACTION, (uint8_t *)"\x0a\x08", 2, cbBTMResponse),
    //beacon report
    FRAME_MATCH_DESC(MSG_INTERESTING, IEEE80211_FC0_SUBTYPE_ACTION, (uint8_t *)"\x05\x01", 2, cbBeaconReport),
    //disassoc request
    //FRAME_MATCH_DESC(MSG_INTERESTING, IEEE80211_FC0_SUBTYPE_DISASSOC, NULL, 0, cbDisassociateSta),
    FRAME_MATCH_DESC(MSG_NULL, 0, NULL, 0, NULL)
};

////////////////////////////////////////////////////////////////////////////////
// Private functions and data
////////////////////////////////////////////////////////////////////////////////

// CMDUs can be received in multiple fragments/packets when they are too big to
// fit in a single "network transmission unit" (which is never bigger than
// MAX_NETWORK_SEGMENT_SIZE).
//
// Fragments that belong to one same CMDU contain the same 'mid' and different
// 'fragment id' values. In addition, the last fragment is the only one to
// contain the 'last fragment indicator' field set.
//
//   NOTE: This is all also explained in "Sections 7.1.1 and 7.1.2"
//
// This function will "buffer" fragments until either all pieces arrive or a
// timer expires (in which case all previous fragments are discarded/ignored)
//
//   NOTE: Instead of a timer, we will use a buffer that holds up to
//         MAX_MIDS_IN_FLIGHT CMDUs.
//         If we are still waiting for MAX_MIDS_IN_FLIGHT CMDUs to be completed
//         (ie. we haven't received all their fragments yet), and a new fragment
//         for a new CMDU arrives, we will discard all fragments from the
//         oldest one.
// 
// Every time this function is called, two things can happen:
//
//   1. The just received fragment was the last one needed to complete a CMDU.
//      In this case, the CMDU structure result of all those fragments being
//      parsed is returned.
//
//   2. The just received fragment is not yet the last one needed to complete a
//      CMDU. In this case the fragment is internally buffered (ie. the caller
//      does not need to keep the passed buffer around in memory) and this
//      function returns NULL.
//
// This function received two arguments:
//
//   - 'packet_buffer' is a pointer to the received stream containing a
//     fragment (or a whole) CMDU
//
//   - 'len' is the length of this 'packet_buffer' in bytes
//
struct CMDU2 *_reAssembleFragmentedCMDUs(uint8_t *packet_buffer, uint16_t len)
{
    #define MAX_MIDS_IN_FLIGHT     16
    #define MAX_FRAGMENTS_PER_MID  16

    // This is a static structure used to store the fragments belonging to up to
    // 'MAX_MIDS_IN_FLIGHT' CMDU messages.
    // Initially all entries are marked as "empty" by setting the 'in_use' field
    // to "0"
    //
    static struct _midsInFlight
    {
        uint8_t in_use;  // Is this entry free?

        uint16_t mid;     // 'mid' associated to this CMDU

        uint8_t src_addr[6];
        uint8_t dst_addr[6];
                       // These two (together with the 'mid' field) will be used
                       // to identify fragments belonging to one same CMDU.

        uint8_t fragments[MAX_FRAGMENTS_PER_MID+1];
                       // Each entry represents a fragment number.
                       //   - "1" means that fragment has been received
                       //   - "0" means no fragment with that number has been
                       //     received.
                      
        uint8_t last_fragment;
                       // Number of the fragment carrying the
                       // 'last_fragment_indicator' flag.
                       // This is always a number between 0 and
                       // MAX_FRAGMENTS_PER_MID-1.
                       // Iniitally it is set to "MAX_FRAGMENTS_PER_MID",
                       // meaning that no fragment with the
                       // 'last_fragment_indicator' flag has been received yet.

        uint8_t *streams[MAX_FRAGMENTS_PER_MID+1];
        uint16_t stream_lens[MAX_FRAGMENTS_PER_MID+1];
                        // Each of the bit streams associated to each fragment
                       //
                       // The size is "MAX_FRAGMENTS_PER_MID+1" instead of
                       // "MAX_FRAGMENTS_PER_MID" to store a final NULL entry
                       // (this makes it easier to later call
                       // "parse_1905_CMDU_header_from_packet()"
                       
        uint32_t age;    // Used to keep track of which is the oldest CMDU for 
                       // which a fragment was received (so that we can free
                       // it when the CMDUs buffer is full)

    } mids_in_flight[MAX_MIDS_IN_FLIGHT] = \
    {[ 0 ... MAX_MIDS_IN_FLIGHT-1 ] = (struct _midsInFlight) { .in_use = 0 }};

    static uint32_t current_age = 0;

    uint8_t  dst_addr[6];
    uint8_t  src_addr[6];
    uint16_t ether_type;

    uint16_t mid;
    uint8_t  fragment_id;
    uint8_t  last_fragment_indicator;

    uint8_t  i, j;
    uint8_t *p;

    p = packet_buffer;

    _EnB(&p, dst_addr, 6);
    _EnB(&p, src_addr, 6);
    _E2B(&p, &ether_type);

    len -= (6+6+2);

    if (0 == parse_1905_CMDU_header_from_packet(p, &mid, &fragment_id, &last_fragment_indicator))
    {
        DEBUG_ERROR("Could not retrieve CMDU header from bit stream\n");
        return NULL;
    }
    DEBUG_DETAIL("mid = %d, fragment_id = %d, last_fragment_indicator = %d\n", mid, fragment_id, last_fragment_indicator);

    // Find the set of streams associated to this 'mid' and add the just
    // received stream to its set of streams
    //
    for (i = 0; i<MAX_MIDS_IN_FLIGHT; i++)
    {
        if (
                                  1        ==  mids_in_flight[i].in_use          &&
                                  mid      ==  mids_in_flight[i].mid             &&
             0 == PLATFORM_MEMCMP(dst_addr,    mids_in_flight[i].dst_addr, 6)    &&
             0 == PLATFORM_MEMCMP(src_addr,    mids_in_flight[i].src_addr, 6)
           )
        {
            // Fragments for this 'mid' have previously been received. Add this
            // new one to the set.
            
            // ...but first check for errors
            //
            if (fragment_id > MAX_FRAGMENTS_PER_MID)
            {
                DEBUG_ERROR("Too many fragments (%d) for one same CMDU (max supported is %d)\n",fragment_id, MAX_FRAGMENTS_PER_MID);
                DEBUG_ERROR("  mid      = %d\n", mid);
                DEBUG_ERROR("  src_addr = %02x:%02x:%02x:%02x:%02x:%02x\n", src_addr[0], src_addr[1], src_addr[2], src_addr[3], src_addr[4], src_addr[5]);
                DEBUG_ERROR("  dst_addr = %02x:%02x:%02x:%02x:%02x:%02x\n", dst_addr[0], dst_addr[1], dst_addr[2], dst_addr[3], dst_addr[4], dst_addr[5]);
                return NULL;
            }

            if (1 == mids_in_flight[i].fragments[fragment_id])
            {
                DEBUG_WARNING("Ignoring duplicated fragment #%d\n",fragment_id);
                DEBUG_WARNING("  mid      = %d\n", mid);
                DEBUG_WARNING("  src_addr = %02x:%02x:%02x:%02x:%02x:%02x\n", src_addr[0], src_addr[1], src_addr[2], src_addr[3], src_addr[4], src_addr[5]);
                DEBUG_WARNING("  dst_addr = %02x:%02x:%02x:%02x:%02x:%02x\n", dst_addr[0], dst_addr[1], dst_addr[2], dst_addr[3], dst_addr[4], dst_addr[5]);
                return NULL;
            }

            if (1 == last_fragment_indicator && MAX_FRAGMENTS_PER_MID != mids_in_flight[i].last_fragment)
            {
                DEBUG_WARNING("This fragment (#%d) and a previously received one (#%d) both contain the 'last_fragment_indicator' flag set. Ignoring...\n", fragment_id, mids_in_flight[i].last_fragment);
                DEBUG_WARNING("  mid      = %d\n", mid);
                DEBUG_WARNING("  src_addr = %02x:%02x:%02x:%02x:%02x:%02x\n", src_addr[0], src_addr[1], src_addr[2], src_addr[3], src_addr[4], src_addr[5]);
                DEBUG_WARNING("  dst_addr = %02x:%02x:%02x:%02x:%02x:%02x\n", dst_addr[0], dst_addr[1], dst_addr[2], dst_addr[3], dst_addr[4], dst_addr[5]);
                return NULL;
            }

            // ...and now actually save the stream for later
            //
            mids_in_flight[i].fragments[fragment_id] = 1;

            if (1 == last_fragment_indicator)
            {
                mids_in_flight[i].last_fragment = fragment_id;
            }

            mids_in_flight[i].streams[fragment_id] = (uint8_t *)PLATFORM_MALLOC((sizeof(uint8_t) * len));
            PLATFORM_MEMCPY(mids_in_flight[i].streams[fragment_id], p, len);
            mids_in_flight[i].stream_lens[fragment_id] = len;
            mids_in_flight[i].age = current_age++;

            break;
        }
    }

    // If we get inside the next "if()", that means no previous entry matches
    // this 'mid' + 'src_addr' + 'dst_addr' tuple.
    // What we have to do then is to search for an empty slot and add this as
    // the first stream associated to this new tuple.
    //
    if (MAX_MIDS_IN_FLIGHT == i)
    {
        for (i = 0; i<MAX_MIDS_IN_FLIGHT; i++)
        {
            if (0 == mids_in_flight[i].in_use)
            {
                break;
            }
        }

        if (MAX_MIDS_IN_FLIGHT == i)
        {
            // All slots are in use!!
            //
            // We need to discard the oldest one (ie. the one with the lowest
            // 'age')
            //
            uint32_t lowest_age;

            lowest_age = mids_in_flight[0].age;
            j          = 0;

            for (i=1; i<MAX_MIDS_IN_FLIGHT; i++)
            {
                if (mids_in_flight[i].age < lowest_age)
                {
                    lowest_age = mids_in_flight[i].age;
                    j          = i;
                }
            }

            DEBUG_WARNING("Discarding old CMDU fragments to make room for the just received one. CMDU being discarded:\n");
            DEBUG_WARNING("  mid      = %d\n", mids_in_flight[j].mid);
            DEBUG_WARNING("  mids_in_flight[j].src_addr = %02x:%02x:%02x:%02x:%02x:%02x\n", mids_in_flight[j].src_addr[0], mids_in_flight[j].src_addr[1], mids_in_flight[j].src_addr[2], mids_in_flight[j].src_addr[3], mids_in_flight[j].src_addr[4], mids_in_flight[j].src_addr[5]);
            DEBUG_WARNING("  mids_in_flight[j].dst_addr = %02x:%02x:%02x:%02x:%02x:%02x\n", mids_in_flight[j].dst_addr[0], mids_in_flight[j].dst_addr[1], mids_in_flight[j].dst_addr[2], mids_in_flight[j].dst_addr[3], mids_in_flight[j].dst_addr[4], mids_in_flight[j].dst_addr[5]);

            for (i=0; i<MAX_FRAGMENTS_PER_MID; i++)
            {
                if (1 == mids_in_flight[j].fragments[i] && NULL != mids_in_flight[j].streams[i])
                {
                    PLATFORM_FREE(mids_in_flight[j].streams[i]);
                }
            }

            mids_in_flight[j].in_use = 0;

            i = j;
        }

        // Now that we have our empty slot, initialize it and fill it with the
        // just received stream:
        //
        mids_in_flight[i].in_use = 1;
        mids_in_flight[i].mid    = mid;

        PLATFORM_MEMCPY(mids_in_flight[i].src_addr, src_addr, 6);
        PLATFORM_MEMCPY(mids_in_flight[i].dst_addr, dst_addr, 6);

        for (j=0; j<MAX_FRAGMENTS_PER_MID; j++)
        {
            mids_in_flight[i].fragments[j] = 0;
            mids_in_flight[i].streams[j]   = NULL;
        }
        mids_in_flight[i].streams[MAX_FRAGMENTS_PER_MID] = NULL;

        mids_in_flight[i].fragments[fragment_id]  = 1;
        mids_in_flight[i].streams[fragment_id]    = (uint8_t *)PLATFORM_MALLOC((sizeof(uint8_t) * len));
        PLATFORM_MEMCPY(mids_in_flight[i].streams[fragment_id], p, len);
        mids_in_flight[i].stream_lens[fragment_id] = len;
        if (1 == last_fragment_indicator)
        {
            mids_in_flight[i].last_fragment = fragment_id;
        }
        else
        {
            mids_in_flight[i].last_fragment = MAX_FRAGMENTS_PER_MID;
              // NOTE: This means "no 'last_fragment_indicator' flag has been
              //       received yet.
        }

        mids_in_flight[i].age = current_age++;
    }

    // At this point we have an entry in the 'mids_in_flight' array (entry 'i')
    // where a new stream/fragment has been added.
    //
    // We now have to check if we have received all fragments for this 'mid'
    // and, if so, process them and obtain a CMDU structure that will be
    // returned to the caller of the function.
    //
    // Otherwise, return NULL.
    //
    if (MAX_FRAGMENTS_PER_MID != mids_in_flight[i].last_fragment)
    {
        struct CMDU2 *c;

        for (j=0; j<=mids_in_flight[i].last_fragment; j++)
        {
            if (0 == mids_in_flight[i].fragments[j])
            {
                DEBUG_DETAIL("We still have to wait for more fragments to complete the CMDU message\n");
                return NULL;
            }
        }

        c = parse_1905_CMDU_from_packets(mids_in_flight[i].streams, mids_in_flight[i].stream_lens);

        if (NULL == c)
        {
            DEBUG_WARNING("parse_1905_CMDU_header_from_packet() failed\n");
        }
        else
        {
            DEBUG_DETAIL("All fragments belonging to this CMDU have already been received and the CMDU structure is ready\n");
        }

        for (j=0; j<=mids_in_flight[i].last_fragment; j++)
        {
            PLATFORM_FREE(mids_in_flight[i].streams[j]);
        }
        mids_in_flight[i].in_use = 0;

        return c;
    }
    
    DEBUG_DETAIL("The last fragment has not yet been received\n");
    return NULL;
}

// Returns '1' if the packet has already been processed in the past and thus,
// should be discarded (to avoid network storms). '0' otherwise.
//
// According to what is explained in "Sections 7.5, 7.6 and 7.7" if a
// defragmented packet whose "AL MAC address TLV" and "message id" match one
// that has already been received in the past, then it should be discarded.
//
// I *personally* think the standard is "slightly" wrong here because *not* all
// CMDUs contain an "AL MAC address TLV".
// We could use the ethernet source address instead, however this would only
// work for those messages that are *not* relayed (one same duplicated relayed
// message can arrive at our local node with two different ethernet source
// addresses).
// Fortunately for us, all relayed CMDUs *do* contain an "AL MAC address TLV",
// thus this is what we are going to do:
//
//   1. If the CMDU is a relayed one, check against the "AL MAC" contained in
//      the "AL MAC address TLV"
//
//   2. If the CMDU is *not* a relayed one, check against the ethernet source
//      address
//
// This function keeps track of the latest MAX_DUPLICATES_LOG_ENTRIES tuples
// of ("mac_address", "message_id") and:
//
//   1. If the provided tuple matches an already existing one, this function
//      returns '1'
//
//   2. Otherwise, the entry is added (discarding, if needed, the oldest entry)
//      and this function returns '0'
//
uint8_t _checkDuplicates(uint8_t *src_mac_address, struct CMDU2 *c)
{
    #define MAX_DUPLICATES_LOG_ENTRIES 10

    static uint8_t  mac_addresses[MAX_DUPLICATES_LOG_ENTRIES][6];
    static uint16_t message_ids  [MAX_DUPLICATES_LOG_ENTRIES];

    static uint8_t start = 0;
    static uint8_t total = 0;

    uint8_t mac_address[6];

    uint8_t i;

    if(
        CMDU_TYPE_TOPOLOGY_RESPONSE               == c->type ||
        CMDU_TYPE_LINK_METRIC_RESPONSE            == c->type ||
        CMDU_TYPE_AP_AUTOCONFIGURATION_RESPONSE   == c->type ||
        CMDU_TYPE_HIGHER_LAYER_RESPONSE           == c->type ||
        CMDU_TYPE_INTERFACE_POWER_CHANGE_RESPONSE == c->type ||
        CMDU_TYPE_GENERIC_PHY_RESPONSE            == c->type
      )
    {
        // This is a "hack" until a better way to handle MIDs is found.
        //
        // Let me explain.
        //
        // According to the standard, each AL entity generates its monotonically
        // increasing MIDs every time a new packet is sent.
        // The only exception to this rule is when generating a "response". In
        // these cases the same MID contained in the original query must be
        // used.
        //
        // Imagine we have two ALs that are started in different moments:
        //
        //        AL 1               AL 2
        //        ====               ====
        //   t=0  --- MID=1 -->      
        //   t=1  --- MID=2 -->      
        //   t=2  --- MID=3 -->      <-- MID=1 --
        //   t=3  --- MID=4 -->      <-- MID=2 --
        //   t=4  --- MID=5 -->      <-- MID=3 --
        //
        // In "t=2", "AL 2" learns that, in the future, messages from "AL 1" with
        // a "MID=3" should be discarded.
        //
        // Now, imagine in "t=4" the message "AL 2" sends (with "MID=3") is a
        // query that triggers a response from "AL 1" (which *must* have the
        // same MID, ie., "MID=3").
        //
        // HOWEVER, because of what "AL 2" learnt in "t=2", this response will
        // be discarded!
        //
        // In oder words... until the standard clarifies how MIDs should be
        // generated to avoid this problem, we will just accept (and process)
        // all response messages... even if they are duplicates.
        //
        return 0;
    }

    // For relayed CMDUs, use the AL MAC, otherwise use the ethernet src MAC.
    //
    PLATFORM_MEMCPY(mac_address, src_mac_address, 6);
    if (1 == c->relay)
    {
        struct macAddressTLV *tlv = (struct macAddressTLV *)getTypedTLV(c, TLV_TYPE_AL_MAC_ADDRESS_TYPE, 0);

        if (tlv)
            MACCPY(mac_address, tlv->mac);
    }

    // Also, discard relayed CMDUs whose AL MAC is our own (that means someone
    // is retrasnmitting us back a message we originally created)
    //
    if (1 == c->relay)
    {
        if (0 == PLATFORM_MEMCMP(mac_address, DMalMacGet(), 6))
        {
            return 1;
        }
    }

    // Find if the ("mac_address", "message_id") tuple is already present in the
    // database
    //
    for (i=0; i<total; i++)
    {
        uint8_t index;

        index = (start + i) % MAX_DUPLICATES_LOG_ENTRIES;

        if (
             0 == PLATFORM_MEMCMP(mac_addresses[index],    mac_address, 6) &&
                                  message_ids[index]    == c->id
           )
        {
            // The entry already exists!
            //
            return 1;
        }
    }

    // This is a new entry, insert it into the cache and return "0"
    //
    if (total < MAX_DUPLICATES_LOG_ENTRIES)
    {
        // There is space for new entries
        //
        uint8_t index;

        index = (start + total) % MAX_DUPLICATES_LOG_ENTRIES;

        PLATFORM_MEMCPY(mac_addresses[index], mac_address, 6);
        message_ids[index] = c->id;

        total++;
    }
    else
    {
        // We need to replace the oldest entry
        //
        PLATFORM_MEMCPY(mac_addresses[start], mac_address, 6);
        message_ids[start] = c->id;

        start++;

        start = start % MAX_DUPLICATES_LOG_ENTRIES;
    }

    return 0;
}

void _checkForwarding(struct CMDU2 *c, uint32_t recv_idx, uint8_t *src, uint8_t *dst)
{
    if (!c->relay)
        return;

    sendMulticast(c, recv_idx, dst, src);
    return;
}

static void _triggerAPAutoConfig(void)
{
    struct radio *r;
    struct TLV *m1_tlv;

    dlist_for_each(r, local_device->radios, l)
    {
        if (!r->configured)
        {
            DEBUG_DETAIL("Radio %s is not configured, send M2 with it..\n", r->name);

            m1_tlv = wscBuildM1(r);

            if ( 0 == sendAPAutoconfigurationWSCM1(idx2InterfaceName(registrar->recv_intf_idx), getNextMid(),
                        registrar->al_mac, r, m1_tlv))
            {
                DEBUG_WARNING("Could not send 'AP autoconfiguration WSC-M1' message\n");
                return;
            }
            return;
        }
    }

    /* All radios configured -> whole system is configured */
    local_device->configured = 1;

    struct mesh_ubus_event_request req;
    MACCPY(req.u.configuration_synchronized.controller, registrar->al_mac);
    sendUbusEvent(EVENT_CONFIGURATION_SYNCHRONIZED, &req);

    /* if configured. send current operating channel report */
    sendOperatingChannelReport(idx2InterfaceName(registrar->recv_intf_idx), getNextMid(),
        registrar->al_mac, local_device, 0);

    DEBUG_INFO("All the radios on local device are configured\n");
}

//This function sends an "AP-autoconfig search" message on all authenticated
//interfaces BUT ONLY if there is at least one unconfigured AP interface on
//this node.
//
//A function has been created for this because the same code is executed from
//three different places:
//
//- When a new interface becomes authenticated
//
//- When the *local* push button is pressed and there is at least one
//interface which does not support this configuration mechanism (ex:
//        ethernet)
//
//- After a local unconfigured AP interface becomes configured (this is
//        needed in the unlikely situation where there are more than one
//        unconfigured APs in the same node)
//
static void _triggerAPSearch(void)
{
    static uint8_t index = 0;
    if (isRegistrar())
        return;

    if ((index++) & 0x01)
        sendAPAutoconfigurationSearch(getNextMid(), IEEE80211_FREQUENCY_BAND_5_GHZ);
    else
        sendAPAutoconfigurationSearch(getNextMid(), IEEE80211_FREQUENCY_BAND_2_4_GHZ);
}

static void _timerDiscovery(void *data)
{
    uint16_t mid;
    struct interface_list_item *item;
    dlist_head *interfaces = PLATFORM_GET_LIST_OF_1905_INTERFACES();

    // According to "Section 8.2.1.1" and "Section 8.2.1.2"
    // we now have to send a "Topology discovery message"
    // followed by a "802.1 bridge discovery message" but,
    // according to the rules in "Section 7.2", only on each
    // and every of the *authenticated* 1905 interfaces
    // that are in the state of "PWR_ON" or "PWR_SAVE"
    mid       = getNextMid();

    dlist_for_each(item, *interfaces, l) {
        uint8_t authenticated;
        uint8_t power_state;

        struct interfaceInfo *x;

        if ((x = PLATFORM_GET_1905_INTERFACE_INFO(item->intf_name))) {
            authenticated = x->is_secured;
            power_state   = x->power_state;

            PLATFORM_FREE_1905_INTERFACE_INFO(x);
        } else {
            DEBUG_WARNING("Could not retrieve info of interface %s\n", item->intf_name);
            authenticated = 0;
            power_state   = INTERFACE_POWER_STATE_OFF;
        }

        if ((0 == authenticated) ||
            ((power_state != INTERFACE_POWER_STATE_ON) && (power_state!= INTERFACE_POWER_STATE_SAVE))) {
            // Do not send the discovery messages on this
            // interface
            //
            continue;
        }

        // Topology discovery message
        //
        if (0 == sendTopologyDiscovery(item->intf_name, mid)) {
            DEBUG_WARNING("Could not send 1905 topology discovery message\n");
        }

        // 802.1 bridge discovery message
        //
        if ((local_config.lldp) && (0 == sendLLDPDiscovery(item->intf_name))) {
            DEBUG_WARNING("Could not send LLDP bridge discovery message\n");
        }
    }
}

void _timerGarbageCollector(void *data)
{
    DEBUG_DETAIL("Running garbage collector...\n");

    if (dmSweep() > 0) {
        DEBUG_DETAIL("Some elements were removed");
    }

    neighborAgeHandle(local_device);
}

static void _timerGetStationStats(void *data)
{
    struct interface *intf;

    DEBUG_DETAIL("Running station stats get timer handler...\n");

    dlist_for_each(intf, local_device->interfaces, l) {
        if ((intf->type != interface_type_wifi) ||
            (((struct wifi_interface *)intf)->role != role_ap))
            continue;

        stationGetStats(intf->index, NULL);
    }
}

static void triggerControllerSearch(void)
{
    if (!registrar)
        _triggerAPSearch();
    else if ((!isRegistrar()) && (!local_device->configured))
        _triggerAPAutoConfig();
}

static void _timerAutoConfig(void *data)
{
    triggerControllerSearch();
}

void processALEvent1905Packet(uint8_t *p, uint16_t len)
{
    uint8_t *packet;
    uint16_t packet_len;
    uint8_t  *dst;
    uint8_t  *src;
    uint16_t ether_type;
    struct msg_attr attrs[attr_drv_max] = {0};

    uint32_t recv_inf_idx;
    //uint8_t  recv_inf_addr[MACLEN];
    char  *recv_inf_name;

    if (msgaParse(attrs, attr_drv_max, p, len)<0)
        return;

    if ((!hasMsga(attrs, attr_packet)) ||
        (!hasMsga(attrs, attr_ethertype)) ||
        (!hasMsga(attrs, attr_if_idx)))
        return;

    recv_inf_idx = msgaGetU32(&attrs[attr_if_idx]);
    ether_type = msgaGetU16(&attrs[attr_ethertype]);

    packet = msgaGetBin(&attrs[attr_packet]);
    packet_len = msgaGetLen(&attrs[attr_packet]);

    dst = packet;
    src = packet + MACLEN;

    recv_inf_name = idx2InterfaceName(recv_inf_idx);
    if (!recv_inf_name) {
        DEBUG_ERROR("receive on an unknown interface(idx=%d)\n", recv_inf_idx);
        return;
    }

    DEBUG_DETAIL("received packet on %s(idx=%d)\n", recv_inf_name, recv_inf_idx);
    DEBUG_DETAIL("    dst="MACFMT", src="MACFMT", type=0x%04x\n", MACARG(dst), MACARG(src), ether_type);

    switch(ether_type)
    {
        case ETHERTYPE_LLDP:
            {
                struct PAYLOAD *payload;

                DEBUG_DETAIL("LLDP message received.\n");
                payload = parse_lldp_PAYLOAD_from_packet(packet+14, packet_len-14);

                if (NULL == payload)
                {
                    DEBUG_WARNING("Invalid LLDP from "MACFMT"\n", MACARG(src));
                }
                else
                {
                    //DEBUG_DETAIL("LLDP message contents:\n");
                    //visit_lldp_PAYLOAD_structure(payload, print_callback, DEBUG_DETAIL, "");
                    processLlpdPayload(payload, recv_inf_idx);
                    free_lldp_PAYLOAD_structure(payload);
                }

                break;
            }

        case ETHERTYPE_1905:
            {
                struct CMDU2 *c;

                DEBUG_DETAIL("CMDU message received. Reassembling...\n");

                c = _reAssembleFragmentedCMDUs(packet, packet_len);

                if (NULL == c)
                {
                    // This was just a fragment part of a big CMDU.
                    // The data has been internally cached, waiting for
                    // the rest of pieces.
                }
                else
                {
                    if (_checkDuplicates(src, c) == 1) {
                        DEBUG_WARNING("Duplication on %s(idx=%d), mid=0x%04x. Discarding..\n",
                                recv_inf_name, recv_inf_idx, c->id);
                    } else {
                        uint8_t res;
#if 0
                        DEBUG_DETAIL("CMDU message contents:\n");
                        visit_1905_CMDU_structure(c, print_callback, DEBUG_DETAIL, "");
#endif
                        // Process the message on the local node
                        //
                        res = process1905Cmdu(c, recv_inf_idx, src, dst);
                        if (PROCESS_CMDU_OK_TRIGGER_CONTROLLER_SEARCH == res)
                        {
                            triggerControllerSearch();
                        }

                        // It might be necessary to retransmit this
                        // message on the rest of interfaces (depending
                        // on the "relayed multicast" flag
                        //
                        if (local_config.relay)
                            _checkForwarding(c, recv_inf_idx, src, dst);
                    }

                    cmdu2Free(c);
                }

                break;
            }

        default:
            {
                DEBUG_WARNING("Unknown ethertype 0x%04x!! Ignoring...\n", ether_type);
                break;
            }
    }
}

static void unsolicateReport(uint32_t report_mask)
{
    if ((isRegistrar()) || (!registrar)) {
        return;
    }

    if (report_mask & REPORT_AP_METRICS_MASK)
        sendApMetricsResponse(idx2InterfaceName(registrar->recv_intf_idx), getNextMid(),
                registrar->al_mac, NULL, 1);

    if (report_mask & REPORT_OPERATING_CHANNEL_MASK) {
        sendOperatingChannelReport(idx2InterfaceName(registrar->recv_intf_idx), getNextMid(),
                registrar->al_mac, local_device, 0);
    }
}

static void processRadioUpdate(uint8_t *p, uint16_t len)
{
    int num_opc_support = 0;
    struct radio *r = NULL;
    struct band_capability *band_capa = NULL;
    struct operating_class *opclass_info = NULL;
    struct chan_info *chan_info = NULL;
    struct msg_attr attr[1];
    int type;
    uint32_t report_mask = 0;

    while ((type = msgaParseOne(attr, &p, &len)) != attr_none) {
        switch (type) {
            case attr_radio_mac:
                r = radioAdd(local_device, msgaGetBin(attr));
                // assign default max_bss
                if (r) {
                    if (!r->max_bss)
                        r->max_bss = DEFAULT_MAX_BSS;
                    DEBUG_INFO("add/update radio: "MACFMT"\n", MACARG(r->uid));
                }
                break;
            case attr_radio_id:
                if (r)
                    r->index = msgaGetU8(attr);
                break;
            case attr_radio_name:
                if (r)
                    REPLACE_STR(r->name, strdup(msgaGetStr(attr)));
                break;
            case attr_band_idx:
                if (r) {
                    r->current_band_idx = msgaGetU8(attr);
                    if (isRegistrar()) {
                        struct policy_param_metrics_rpt *rpt = findReportPolicy(r->current_band_idx);
                        if (rpt) {
                            r->metrics_rpt_policy.sta_rcpi_thresh = rpt->sta_rcpi_thresh;
                            r->metrics_rpt_policy.sta_rcpi_margin = rpt->sta_rcpi_margin;
                            r->metrics_rpt_policy.ap_chutil_thresh = rpt->ap_chutil_thresh;
                            r->metrics_rpt_policy.assoc_sta_inclusion_mode = rpt->assoc_sta_inclusion_mode;
                        }
                    }
                }
                break;
            case attr_band:
                if (r) {
                    r->bands |= msgaGetU8(attr);
                    band_capa = &r->bands_capa[band2BandIdx(msgaGetU8(attr))];
                }
                break;
            case attr_opclass_id:
                if (r) {
                    uint8_t class = msgaGetU8(attr);
                    if ((opclass_info = opclassAdd(r, class))) {
                        initOperatingClass(opclass_info, 1);
                    }
                }
                break;
            case attr_opclass_max_txpower:
                if (opclass_info)
                    opclass_info->max_tx_power = msgaGetU8(attr);
                break;
            case attr_opclass_chan_id:
                chan_info = NULL;
                if (opclass_info) {
                    int i;
                    for (i=0;i<opclass_info->num_support_chan;i++) {
                        if (opclass_info->channels[i].id==msgaGetU8(attr)) {
                            chan_info = &opclass_info->channels[i];
                            chan_info->disable = 0;
                        }
                    }
                }
                break;
            case attr_opclass_chan_pref:
                if (chan_info)
                    chan_info->pref = msgaGetU8(attr);
                break;
            case attr_opclass_chan_reason:
                if (chan_info)
                    chan_info->reason = msgaGetU8(attr);
                break;
            case attr_opclass_chan_freq_separation:
                if (chan_info)
                    chan_info->freq_separation = msgaGetU8(attr);
                break;
            case attr_bandwidth:
                if ((r) && (r->bw!=msgaGetU8(attr))) {
                    r->bw = msgaGetU8(attr);
                }
                break;
            case attr_opclass:
                if (r) {
                    CHECK_AND_REPORT(r->opclass, msgaGetU8(attr), (r->change_mask|= OPERATING_CHANNEL_CHANGE_MASK),
                        report_mask, REPORT_OPERATING_CHANNEL_MASK);
                }
                break;
            case attr_channel:
                if (r) {
                    CHECK_AND_REPORT(r->channel, msgaGetU8(attr), (r->change_mask|= OPERATING_CHANNEL_CHANGE_MASK),
                        report_mask, REPORT_OPERATING_CHANNEL_MASK);
                }
                break;
            case attr_ht_capa:
                if (band_capa) {
                    band_capa->ht_capa_valid = 1;
                    band_capa->ht_capa.capa = msgaGetU8(attr);
                }
                break;
            case attr_vht_capa:
                if (band_capa) {
                    band_capa->vht_capa_valid = 1;
                    band_capa->vht_capa.capa = msgaGetU16(attr);
                }
                break;
            case attr_vht_rx_mcs:
                if (band_capa) {
                    band_capa->vht_capa.rx_mcs = msgaGetU16(attr);
                }
                break;
            case attr_vht_tx_mcs:
                if (band_capa) {
                    band_capa->vht_capa.tx_mcs = msgaGetU16(attr);
                }
                break;
            case attr_he_capa:
                if (band_capa) {
                    band_capa->he_capa_valid = 1;
                    band_capa->he_capa.capa = msgaGetU16(attr);
                }
                break;
            case attr_he_mcs:
                uint8_t mcs_len = msgaGetLen(attr);
                if ((band_capa) && (mcs_len <= MAX_HE_MCS_SIZE)) {
                    band_capa->he_capa.mcs[0] = mcs_len;
                    memcpy(&band_capa->he_capa.mcs[1], msgaGetBin(attr), mcs_len);
                }
                break;
            case attr_chan_scan_on_boot:
                if (r)
                    r->scan_capa.scan_bootonly = msgaGetU8(attr);
                break;
            case attr_chan_scan_impact:
                if (r)
                    r->scan_capa.impact_mode = msgaGetU8(attr);
                break;
            case attr_chan_scan_min_interval:
                if (r)
                    r->scan_capa.min_scan_interval = msgaGetU32(attr);
                break;
            case attr_chan_util:
                if (r) {
                    if (updateRadioMetrics(r, msgaGetU8(attr))) {
                        r->ch_util_crossed = 1;
                        report_mask |= REPORT_AP_METRICS_MASK;
                    }
                }
                break;
            case attr_radio_capa:
                if (r) {
                    if (msgaGetBin(attr)[0] & RADIO_CAPA_SAE)
                        r->sae_capa_valid = 1;
                }
                break;
            case attr_max_vbss:
                if (r)
                    r->vbss_capa.max_vbss = msgaGetU8(attr);
                break;
            case attr_vbss_subtract:
                if (r)
                    r->vbss_capa.vbss_subtract = msgaGetU8(attr);
                break;
            case attr_vbssid_restrictions:
                if (r)
                    r->vbss_capa.vbssid_restrictions = msgaGetU8(attr);
                break;
            case attr_matched_and_mask_restrictions:
                if (r)
                    r->vbss_capa.match_and_mask_restrictions = msgaGetU8(attr);
                break;
            case attr_fixed_bits_restrictions:
                if (r)
                    r->vbss_capa.fixed_bits_restrictions = msgaGetU8(attr);
                break;
            case attr_fixed_bits_mask:
                if (r)
                    MACCPY(r->vbss_capa.fixed_bits_mask, msgaGetBin(attr));
                break;
            case attr_fixed_bits_value:
                if (r)
                    MACCPY(r->vbss_capa.fixed_bits_value, msgaGetBin(attr));
                break;
            default:
                DEBUG_WARNING("unknown type %d\n", type);
                break;
        }
    }

    /* if radio not configured. */
    if (!r->configured)
        local_device->configured = 0;
    if (num_opc_support > 0 && r)
        r->num_opc_support = num_opc_support;
    if ((r) && (r->change_mask & OPERATING_CHANNEL_CHANGE_MASK))
        updateOperatingChannelReport(r);

    unsolicateReport(report_mask);
}


static int _registerBSSRxFrames(uint32_t idx, struct frame_match_desc *desc)
{
    int ret = 0;

    while ((desc) && (desc->tag!=MSG_NULL)) {
        ret = bssRegisterMgmtFrame(idx, desc->type, desc->match, desc->match_len);
        desc++;
    }
    return ret;
}

static int checkBackhaul(struct wifi_interface *wintf)
{
    struct wifi_config *config;

    /* listen sta interface which wds is 1 default now */
    if (wintf->bssInfo.role == role_sta && wintf->bssInfo.backhaul) {
        wintf->bssInfo.backhaul_sta = 1;
        return 1;
    }

    dlist_for_each(config, local_config.wifi_config.bsses, l) {
        if ((config->bss.backhaul_sta || config->bss.backhaul)
            && ((wintf->bssInfo.ssid.len) && (!SSIDCMP(wintf->bssInfo.ssid, config->bss.ssid)))
            && (wintf->bssInfo.role==config->bss.role)
            && ((wintf->radio) && (bandIdx2Band(wintf->radio->current_band_idx)& config->bands))) {
            if (wintf->bssInfo.role==role_ap)
                wintf->bssInfo.backhaul = 1;
            else
                wintf->bssInfo.backhaul_sta = 1;
            return 1;
        }
    }

    return 0;
}

static void processBSSUpdate(uint8_t *p, uint16_t len)
{
    struct radio *r = NULL;
    struct wifi_interface *wintf = NULL;
    struct msg_attr attr[1];
    int type;
    uint32_t report_mask = 0;

    while ((type = msgaParseOne(attr, &p, &len)) != attr_none) {
        switch (type) {
            case attr_radio_mac:
                if ((r) && (r->change_mask & OPERATING_CHANNEL_CHANGE_MASK))
                    updateOperatingChannelReport(r);
                r = radioFind(local_device, msgaGetBin(attr));
                break;
            case attr_if_mac:
                if (r)
                    wintf = wifiInterfaceAdd(local_device, r, msgaGetBin(attr));
                if (wintf) {
                    MACCPY(wintf->bssInfo.bssid, msgaGetBin(attr));
                    DEBUG_INFO("add/update bss: "MACFMT"\n", MACARG(wintf->i.mac));
                }
                break;
            case attr_if_name:
                if (wintf)
                    REPLACE_STR(wintf->i.name, strdup(msgaGetStr(attr)));
                break;
            case attr_bss_role:
                if (wintf) {
                    wintf->role = msgaGetU8(attr);
                    wintf->bssInfo.role = msgaGetU8(attr);
                }
                break;
            case attr_wds:
                if (wintf) {
                    wintf->bssInfo.backhaul = msgaGetU8(attr);
                }
                break;
            case attr_if_idx:
                if (wintf) {
                    wintf->i.index = msgaGetU32(attr);
                    if (wintf->role == role_ap)
                        _registerBSSRxFrames(wintf->i.index, _default_bss_rx_matches);
                }
                break;
            case attr_ssid:
                if (wintf) {
                    if (msgaGetLen(attr)<=MAX_SSID_LEN) {
                        wintf->bssInfo.ssid.len = msgaGetLen(attr);
                        memcpy(wintf->bssInfo.ssid.ssid, msgaGetBin(attr), wintf->bssInfo.ssid.len);
                        wintf->bssInfo.ssid.ssid[msgaGetLen(attr)] = 0;
                    }
                    /* each bss need to check if backhaul for this msg is a batch msg */
                    if ((wintf->i.name) && checkBackhaul(wintf)) {
                        platformRegisterReceiveInterface(&wintf->i);
                        addInterface(wintf->i.name, 0);
                    }
                }
                break;
            case attr_power_lvl:
                if (r)
                    r->tx_power = msgaGetU8(attr);
                break;
            case attr_opclass:
                if (r) {
                    CHECK_AND_REPORT(r->opclass, msgaGetU8(attr), (r->change_mask|= OPERATING_CHANNEL_CHANGE_MASK),
                            report_mask, REPORT_OPERATING_CHANNEL_MASK);
                }
                break;
            case attr_channel:
                if (r) {
                    CHECK_AND_REPORT(r->channel, msgaGetU8(attr), (r->change_mask|= OPERATING_CHANNEL_CHANGE_MASK),
                            report_mask, REPORT_OPERATING_CHANNEL_MASK);
                }
                break;
            case attr_bandwidth:
                if (r)
                    r->bw = msgaGetU8(attr);
                break;
            case attr_band_idx:
                if (r)
                    r->current_band_idx = msgaGetU8(attr);
            default:
                break;
        }
    }

    if ((r) && (r->change_mask & OPERATING_CHANNEL_CHANGE_MASK))
        updateOperatingChannelReport(r);

    unsolicateReport(report_mask);
}

static void processBSSDelete(uint8_t *p, uint16_t len)
{
    struct wifi_interface *wif = NULL;
    struct msg_attr attrs[attr_drv_max] = {0};

    msgaParse(attrs, attr_drv_max, p, len);

    if (hasMsga(attrs, attr_if_idx))
        wif = (struct wifi_interface *)interfaceFindIdx(msgaGetU32(&attrs[attr_if_idx]));

    if (!wif)
        return;

    if (BSS_IS_BACKHAUL(wif->bssInfo)) {
        platformUnregisterReceiveInterface(&wif->i);
        if (wif->i.name)
            removeInterface(wif->i.name);
    }

    wifiInterfaceDelete(wif);
    return;
}

static void processStationUpdate(uint8_t *p, uint16_t len)
{
    struct wifi_interface *wintf = NULL;
    struct client *client = NULL;
    uint8_t *frame = NULL;
    uint32_t frame_len = 0;
    struct ieee80211_header *wh = NULL;
    int offset = 0;
    uint32_t ts = 0;
    uint8_t last_rcpi = 0;

    struct msg_attr attr[1];
    int type;

    while ((type = msgaParseOne(attr, &p, &len)) != attr_none) {
        switch (type) {
            case attr_if_idx:
                wintf = (struct wifi_interface *)interfaceFindIdx(msgaGetU32(attr));
                if (!wintf)
                    return;
                break;
            case attr_mac:
                if (!wintf)
                    continue;
                if (!(client = clientFind(NULL, NULL, msgaGetBin(attr)))) {
                    client = clientNew(wintf, msgaGetBin(attr));
                    if (!client)
                        return;
                }
                if (client->wif != wintf)
                    clientMove(client, wintf);
                break;
            case attr_frame:
                if (client) {
                    frame = msgaGetBin(attr);
                    frame_len = msgaGetLen(attr);
                    wh = (struct ieee80211_header *)frame;
                    offset = 4;

                    if ((!frame) || (frame_len < (sizeof(*wh)+6)))
                        break;
                    if (IEEE80211_FC0_TYPE_MGT != (wh->fc[0] & IEEE80211_FC0_TYPE_MASK))
                        break;
                    if (IEEE80211_FC0_SUBTYPE_ASSOC_REQ != (wh->fc[0] & IEEE80211_FC0_SUBTYPE_MASK)
                        && IEEE80211_FC0_SUBTYPE_REASSOC_REQ != (wh->fc[0] & IEEE80211_FC0_SUBTYPE_MASK))
                        break;
                    if (IEEE80211_FC0_SUBTYPE_REASSOC_REQ == (wh->fc[0] & IEEE80211_FC0_SUBTYPE_MASK))
                        offset += 6;
                    frame += sizeof(struct ieee80211_header);
                    frame_len -= sizeof(struct ieee80211_header);
                    client->last_assoc_ts = 0;
                    if (client->last_assoc.datap) {
                        free(client->last_assoc.datap);
                        client->last_assoc.len = 0;
                    }
                    if (!(client->last_assoc.datap = calloc(1, frame_len)))
                        return;

                    client->last_assoc.len = frame_len;
                    client->last_assoc_ts = ts;
                    PLATFORM_MEMCPY(client->last_assoc.datap, frame, frame_len);
                    parseAssocFrame(client, offset);
                    sendTopologyNotification(getNextMid(), client, 1);
                    sendClientAssoEvent(client->mac, client->wif->i.mac);
                }
                break;
            case attr_ts:
                ts = msgaGetU32(attr);
                break;
            case attr_station_last_ts:
                break;
            case attr_station_rate_dl:
                if (client) {
                    client->link_metrics.mac_rate_dl = msgaGetU32(attr);
                    client->link_metrics_ts = ts;
                }
                break;
            case attr_station_rate_ul:
                if (client)
                    client->link_metrics.mac_rate_ul = msgaGetU32(attr);
                break;
            case attr_station_rcpi_ul:
                if (client) {
                    last_rcpi = client->link_metrics.rcpi_ul;
                    client->link_metrics.rcpi_ul = msgaGetU8(attr);
                    if (last_rcpi && (client->link_metrics.rcpi_ul < 220)) {
                        stationUpdateEvent(client->mac, client->wif->bssInfo.bssid,
                                        client->link_metrics.rcpi_ul, last_rcpi);
                        if (registrar && !isRegistrar() && wintf && wintf->radio) {
                            uint8_t hysteresis = wintf->radio->metrics_rpt_policy.sta_rcpi_margin > 0 ?
                                wintf->radio->metrics_rpt_policy.sta_rcpi_margin : DEFAULT_RPT_RCPI_HYSTERSIS;
                            if (CROSSED_DOWN_THRESHOLD_MARGIN(wintf->radio->metrics_rpt_policy.sta_rcpi_thresh,
                                hysteresis, last_rcpi, client->link_metrics.rcpi_ul)) {
                                sendAssociatedStaLinkMetricsResponse(idx2InterfaceName(registrar->recv_intf_idx),
                                    getNextMid(), registrar->al_mac, client->mac);
                            }
                        }
                    }
                }
                break;
            case attr_station_rx_bytes:
                if (client)
                    client->traffic_stats.bytes_rx = msgaGetU32(attr);
                break;
            case attr_station_tx_bytes:
                if (client)
                    client->traffic_stats.bytes_tx = msgaGetU32(attr);
                break;
            case attr_station_rx_packets:
                if (client)
                    client->traffic_stats.packets_rx = msgaGetU32(attr);
                break;
            case attr_station_tx_packets:
                if (client)
                    client->traffic_stats.packets_tx = msgaGetU32(attr);
                break;
            case attr_station_rx_errors:
                if (client)
                    client->traffic_stats.packets_err_rx = msgaGetU32(attr);
                break;
            case attr_station_tx_errors:
                if (client)
                    client->traffic_stats.packets_err_tx = msgaGetU32(attr);
                break;
            case attr_station_tx_tries:
                if (client)
                    client->traffic_stats.retransmission = msgaGetU32(attr);
                break;
            case attr_station_last_datarate_dl:
                if (client)
                    client->ext_link_metrics.last_data_rate_dl = msgaGetU32(attr);
                break;
            case attr_station_last_datarate_ul:
                if (client)
                    client->ext_link_metrics.last_data_rate_ul = msgaGetU32(attr);
                break;
            case attr_station_utilization_rx:
                if (client)
                    client->ext_link_metrics.util_rx = msgaGetU32(attr);
                break;
            case attr_station_utilization_tx:
                if (client)
                    client->ext_link_metrics.util_tx = msgaGetU32(attr);
                break;
            case attr_ret_code:
                if (client) {
                    if (msgaGetU8(attr)) {
                        if ((client->wif == wintf) && ((++client->fail_cnt) > CLIENT_MAX_FAIL)) {
                            DEBUG_WARNING("Get station ["MACFMT"] bssid ["MACFMT"] stats failed exceed %d times, remove\n",
                                MACARG(client->mac), MACARG(wintf->bssInfo.bssid), CLIENT_MAX_FAIL);
                            sendTopologyNotification(getNextMid(), client, 0);
                            clientDelete(client);
                            client = NULL;
                        }
                    } else {
                        client->fail_cnt = 0;
                    }
                }
                break;
            default:
                DEBUG_WARNING("unknown type %d\n", type);
                break;
        }
    }

    return;
}

static void processStationDelete(uint8_t *p, uint16_t len)
{
    struct wifi_interface *wintf = NULL;
    struct client *client = NULL;
    struct msg_attr attrs[attr_drv_max] = {0};
    uint16_t reason_code;

    msgaParse(attrs, attr_drv_max, p, len);

    if (!hasMsga(attrs, attr_mac) ||
        !hasMsga(attrs, attr_reason_code))
        return;

    if (hasMsga(attrs, attr_if_idx))
        wintf = (struct wifi_interface *)interfaceFindIdx(msgaGetU32(&attrs[attr_if_idx]));
    else if (hasMsga(attrs, attr_if_name))
        wintf = (struct wifi_interface *)interfaceFindName(local_device, (char *)msgaGetBin(&attrs[attr_if_name]));
    else
        return;

    if (!wintf)
        return;

    reason_code = msgaGetU16(&attrs[attr_reason_code]);
    client = clientFind(NULL, wintf, msgaGetBin(&attrs[attr_mac]));

    if (client) {
        DEBUG_INFO("processStationDelete: ("MACFMT", frame_len: %u, ts: %u)\n", MACARG(client->mac),
                        client->last_assoc.len, client->last_assoc_ts);
        sendTopologyNotification(getNextMid(), client, 0);
        if ((!isRegistrar()) && registrar)
            sendClientDisassocStatsMessage(idx2InterfaceName(registrar->recv_intf_idx), getNextMid(),
                                                registrar->al_mac, client, reason_code);
        clientDelete(client);
    }

    return;
}

uint8_t flag_get_survey[3] = {0, 0, 0}; /* channel survey for band_2g, band_5g, band_6g */
static void processChanScanFinished(uint8_t *p, uint16_t len)
{
    struct wifi_interface *wintf = NULL;
    struct msg_attr attrs[attr_drv_max] = {0};

    msgaParse(attrs, attr_drv_max, p, len);

    if (!hasMsga(attrs, attr_if_idx)) {
        DEBUG_ERROR("channel scan finished event msg does not include attr_if_idx.\n");
        return;
    }

    wintf = (struct wifi_interface *)interfaceFindIdx(msgaGetU32(&attrs[attr_if_idx]));
    if (!wintf) {
        DEBUG_INFO("wifi interface: %u doest exist.\n", msgaGetU32(&attrs[attr_if_idx]));
        return;
    }

    DEBUG_INFO("wifi interface: %s channel scan finished.\n", wintf->i.name);
    if ((wintf->radio->current_band_idx != band_unknown && wintf->radio->current_band_idx != band_max_idx)
            && (0 == flag_get_survey[wintf->radio->current_band_idx])) {
            bssGetChannelSurvey(wintf->i.index); /* AKA ch_util/noise/scan duration, do it after scan finished for fresh */
            flag_get_survey[wintf->radio->current_band_idx] = 1;
    }
    /* get scan results */
    bssGetLastChannelScanResult(wintf->i.index, 0);

    return;
}

/* TODO: need send channel scan report msg */
static void processChanScanResults(uint8_t *p, uint16_t len)
{
    struct wifi_interface *wintf = NULL;
    struct radio *r = NULL;
    struct chscan_req_item *req_item = NULL;
    struct chan_scan_result_item *item = NULL;
    dlist_head head;
    struct msg_attr attr[1];
    int type;
    uint32_t opclass = 0;
    uint32_t start_num = 0;
    uint32_t result_item_num = 0;
    uint8_t finished_flag = 0;

    dlist_head_init(&head);
    while ((type = msgaParseOne(attr, &p, &len)) != attr_none) {
        switch (type) {
            case attr_if_idx:
                wintf = (struct wifi_interface *)interfaceFindIdx(msgaGetU32(attr));
                if (!wintf || !wintf->radio) {
                    DEBUG_ERROR("wifi interface: %u or wintf->radio is NULL.\n", msgaGetU32(attr));
                    return;
                }
                r = wintf->radio;
                break;
            case attr_opclass:
                opclass = msgaGetU32(attr);
                req_item = chscanReqItemFind(r, opclass);
                break;
            case attr_bssid:
                item = (struct chan_scan_result_item *)calloc(1, sizeof(struct chan_scan_result_item));
                MACCPY(item->bssid, msgaGetBin(attr));
                dlist_add_tail(&head, &item->l);
                result_item_num++;
                break;
            case attr_channel:
                if (item)
                    item->channel = msgaGetU8(attr);
                break;
            case attr_signal:
                if (item)
                    item->signal = msgaGetU8(attr);
                break;
            case attr_ts:
                if (item)
                    item->last_seen_ms = msgaGetU32(attr);
                break;
            case attr_ies:
                if (item) {
                    item->ies = msgaGetBin(attr);
                    item->ies_len = msgaGetLen(attr);
                }
                break;
            case attr_beacon_ies:
                if (item) {
                    item->beacon_ies = msgaGetBin(attr);
                    item->beacon_ies_len = msgaGetLen(attr);
                }
                break;
            case attr_start_num:
                start_num = msgaGetU32(attr);
                break;
            case attr_finished_flag:
                finished_flag = msgaGetU8(attr);
                break;
            default:
                DEBUG_WARNING("processChanScanResults unknown type %d\n", type);
                break;
        }
    }

    DEBUG_INFO("wifi interface: %s received chan scan results of opclass(%u), "
               "start_num: %u, finished: %u, result_items: \n",
               wintf->i.name, opclass, start_num, finished_flag);

    int i = 0;
    dlist_for_each(item, head, l) {
        DEBUG_INFO("result(%u): (bssid: "MACFMT", channel: %u, signal: %u, ies_len: %u, "
                   "beacon_ies_len: %u, last_seen_ms: %u)\n",
                   ++i, MACARG(item->bssid), item->channel, item->signal,
                   item->ies_len, item->beacon_ies_len, item->last_seen_ms);
    }

    /* 1. parse scan results to datamodel. first update need clear history results */
    if (parseChanScanResults(r, req_item, &head, result_item_num == start_num) < 0)
        goto bail;

    if (!finished_flag) {
        req_item->status = CHANNEL_SCAN_STATUS_GETTING_RESULTS;
        bssGetLastChannelScanResult(wintf->i.index, start_num);
    }
    /* 2. if scan req item finished getting result then trigger next scan and restart req timer*/
    else {
        DEBUG_INFO("wifi interface: %s received chan scan results of opclass(%u) finished.\n",
               wintf->i.name, opclass);
        req_item->status = CHANNEL_SCAN_STATUS_RESULTS_FINISHED;
        req_item = container_of(dlist_get_next(&r->chscan_req->h, &req_item->l),
                                    struct chscan_req_item, l);
        if (req_item) {
            bssTriggerChannelScan(wintf->i.index, req_item->opclass, req_item->chans, req_item->ch_num);
            req_item->status = CHANNEL_SCAN_STATUS_TRIGGERRED;
            PLATFORM_CANCEL_TIMER(r->chscan_req->timer);
            r->chscan_req->timer = platformAddTimer(10*1000, 0, chanScanTimerHandle, req_item);
        }
    }

    /* 3. check if all chscan_req_items finished getting scan results if finished report it. */
    int finished = 1;
    dlist_for_each(req_item, r->chscan_req->h, l) {
        if (req_item->status != CHANNEL_SCAN_STATUS_RESULTS_FINISHED &&
            req_item->status != CHANNEL_SCAN_STATUS_ABORTED) {
            finished = 0;
            break;
        }
    }
    if (finished) {
        DEBUG_INFO("wifi interface: %s send channel scan report finished.\n", wintf->i.name);
        reportChanScanResult(r->chscan_req);
        r->chscan_req = NULL;
        if (r->chscan_req_pend) {
            r->chscan_req = r->chscan_req_pend;
            r->chscan_req_pend = NULL;
            startChannelScan(r->chscan_req);
        }
    }

bail:
    dlist_free_items(&head, struct chan_scan_result_item, l);

    return;
}

#if 0


static void _processActionFrame(uint8_t *bssid, uint8_t *sta,uint8_t *frame, uint16_t len)
{
    struct ieee80211_action_frame_hdr *action_hdr = (struct ieee80211_action_frame_hdr *)frame;

    switch(action_hdr->category) {
        case ACTION_FRM_CATEGORY_WNM:
            if (action_hdr->action == ACTION_FRM_ACTION_BTM_RSP) /* 2 = sizeof(struct ieee80211_action_frame_hdr) */
                _updateBtmReport(bssid, sta, frame + 2, len - 2);
        break;

        case ACTION_FRM_CATEGORY_RM:
            if (action_hdr->action == ACTION_FRM_ACTION_RSP) /* 2 = sizeof(struct ieee80211_action_frame_hdr) */
                updateBcnMeasReport(bssid, sta, frame + 2, len - 2);
        break;

        default:
            DEBUG_WARNING("can NOT handle action frame[cat=%d, action=%d]\n", action_hdr->category, action_hdr->action);
        break;
    }
    return;
}
#endif

static int _isMatchFrame(struct frame_match_desc *desc, uint8_t *p, uint16_t len)
{
    struct ieee80211_header *header = (struct ieee80211_header *)p;
    uint8_t *body = (uint8_t *)(header+1);

    if (header->fc[0]!=IEEE80211_FC0(IEEE80211_FC0_TYPE_MGT, desc->type))
        return 0;
    if ((desc->match) && (desc->match_len) && memcmp(desc->match, body, desc->match_len))
        return 0;
    return 1;
}

static struct frame_match_desc *_findMatchFrame(struct frame_match_desc *desc, uint8_t *p, uint16_t len)
{
    while ((desc) && (desc->tag!=MSG_NULL)) {
        if (_isMatchFrame(desc, p, len))
            return desc;
        desc++;
    }
    return NULL;
}

static int _sendTunnledMsg(struct frame_match_desc *desc, uint8_t *p, uint16_t len)
{
    struct ieee80211_header *header = (struct ieee80211_header *)p;

    //skip header
    p += sizeof(struct ieee80211_header);
    len -= sizeof(struct ieee80211_header);

    if ((!isRegistrar()) && (registrar))
        sendTunnledMessage(idx2InterfaceName(registrar->recv_intf_idx), getNextMid(), registrar->al_mac, header->sa,
                            desc->type, p, len);
    return 0;
}


static void processMgmtFrame(uint8_t *p, uint16_t len)
{
    struct msg_attr attrs[attr_drv_max] = {0};
    uint8_t *frame;
    uint16_t frame_len;
    static struct frame_match_desc *desc;

    msgaParse(attrs, attr_drv_max, p, len);

    if (!hasMsga(attrs, attr_if_idx) ||
        !hasMsga(attrs, attr_frame))
        return;

//  wintf = (struct wifi_interface *)interfaceFindIdx(msgaGetU32(&attrs[attr_if_idx]));
    frame = msgaGetBin(&attrs[attr_frame]);
    frame_len = msgaGetLen(&attrs[attr_frame]);

    if ((desc = _findMatchFrame(_default_bss_rx_matches, frame, frame_len))) {
        if (desc->process)
            desc->process(desc, frame, frame_len);
    }
#if 0
    switch (wifi_hdr->fc[0] & IEEE80211_FC0_SUBTYPE_MASK) {
        case IEEE80211_FC0_SUBTYPE_ACTION:
            _processActionFrame(wintf->bssInfo.bssid, wifi_hdr->sa, (frame  + sizeof(struct ieee80211_hdr)),
                                    (len_frame - sizeof(struct ieee80211_hdr)));
        break;

        default:
        break;
    }
#endif
}

static void processConnect(uint8_t *p, uint16_t len)
{
    struct msg_attr attrs[attr_drv_max] = {0};
    struct wifi_interface *wintf;

    msgaParse(attrs, attr_drv_max, p, len);

    if (!hasMsga(attrs, attr_if_idx) ||
        !hasMsga(attrs, attr_bssid))
        return;

    wintf = (struct wifi_interface *)interfaceFindIdx(msgaGetU32(&attrs[attr_if_idx]));
    if (!wintf)
        return;

    MACCPY(wintf->bssInfo.bssid, msgaGetBin(&attrs[attr_bssid]));
    DEBUG_INFO(MACFMT" connected to "MACFMT"\n", MACARG(wintf->i.mac), MACARG(wintf->bssInfo.bssid));
}

static void processDisconnect(uint8_t *p, uint16_t len)
{
    struct msg_attr attrs[attr_drv_max] = {0};
    struct wifi_interface *wintf;

    msgaParse(attrs, attr_drv_max, p, len);

    if (!hasMsga(attrs, attr_if_idx))
        return;

    wintf = (struct wifi_interface *)interfaceFindIdx(msgaGetU32(&attrs[attr_if_idx]));
    if (!wintf)
        return;

    DEBUG_INFO(MACFMT" disconnected from "MACFMT"\n", MACARG(wintf->i.mac), MACARG(wintf->bssInfo.bssid));
    memset(wintf->bssInfo.bssid, 0, MACLEN);
}

static void processConfigChange()
{
    struct al_device *dev;
    uint32_t current_ts = PLATFORM_GET_TIMESTAMP(0);

    dlist_free_items(&local_config.wifi_config.bsses, struct wifi_config, l);
    configuratorGetWifiConfig(&local_config.wifi_config.bsses);

    if (isRegistrar()) {
        dlist_for_each(dev, local_network, l) {
            dev->status = STATUS_UNCONFIGURED;
            dev->set_unconfigured_ts = current_ts;
        }
        sendAPAutoconfigurationRenew(getNextMid(), IEEE80211_FREQUENCY_BAND_2_4_GHZ, NULL);
    }
}

static void processDisallowNotification(uint8_t *p, uint16_t len)
{
    struct msg_attr attrs[attr_drv_max] = {0};
    uint8_t disallow_status = 0;
    struct radio *r;

    msgaParse(attrs, attr_drv_max, p, len);

    if (!hasMsga(attrs, attr_bssid) || !hasMsga(attrs, attr_disallow)) {
        DEBUG_ERROR("lacking key info in disallow notification event.\n");
        return;
    }
    r = radioFind(local_device, msgaGetBin(&attrs[attr_bssid]));
    if (!r) {
        DEBUG_WARNING("["MACFMT"] is not local radio\n", MACARG(msgaGetBin(&attrs[attr_bssid])));
        return;
    }
    disallow_status = msgaGetU8(&attrs[attr_disallow]);
    if (registrar)
        sendAssociationStatNotification(idx2InterfaceName(registrar->recv_intf_idx), getNextMid(), registrar->al_mac,
                                            r->uid, disallow_status);
}

static void processVipConfUpdate(void)
{
    if (isRegistrar()) {
        configuratorGetPolicy(&local_policy);
        updateVipSTAConf();
    }
}

static void processChanSurvey(uint8_t *p, uint16_t len)
{
    struct wifi_interface *wintf = NULL;
    struct radio *r = NULL;
    uint8_t channel = 0, opclass = 0;
    struct operating_class  *opc_info = NULL;
    struct chan_info *ch_info = NULL;
    uint8_t noise = 0;
    uint32_t ch_busy = 0, ch_dwell = 0;

    struct msg_attr attr[1];
    int type;

    while ((type = msgaParseOne(attr, &p, &len)) != attr_none) {
        switch (type) {
            case attr_if_idx:
                wintf = (struct wifi_interface *)interfaceFindIdx(msgaGetU32(attr));
                if (!wintf || !wintf->radio) {
                    DEBUG_ERROR("wifi interface: %u or wintf->radio is NULL.\n", msgaGetU32(attr));
                    return;
                }
                r = wintf->radio;
                break;
            case attr_opclass:
                opclass = msgaGetU8(attr);
                if (r)
                    opc_info = opclassFind(r, opclass);
                break;
            case attr_channel:
                channel = msgaGetU8(attr);
                break;
            case attr_chan_noise:
                noise = msgaGetU8(attr);
                break;
            case attr_chan_dwell:
                ch_dwell = msgaGetU32(attr);
                break;
            case attr_chan_busytime:
                ch_busy = msgaGetU32(attr);
                break;
            default:
                break;
        }
        if (opc_info && channel && noise && ch_dwell) {
            ch_info = channelFind(opc_info, channel);
            if (NULL == ch_info) {
                DEBUG_WARNING("can NOT find channel info for ch[%d]\n", channel);
                goto reset_value;
            }
            if (noise != 255) {
                ch_info->utilization = (uint8_t)((ch_busy * 255) / ch_dwell);
                ch_info->avg_noise = noise;
                ch_info->chscan_dur = ch_dwell;
                ch_info->chscan_type = 1;    /* send probe, so force the type to active scan */
                DEBUG_INFO("al recv channel[%d] + opc[%d] survey from driver: ch_util=%d, noise=%d, scan_duration=%d, scan_type=%d\n",
                            channel, opclass, ch_info->utilization, ch_info->avg_noise, ch_info->chscan_dur, ch_info->chscan_type);
            }
reset_value:
            opclass = 0;
            channel = 0;
            noise = 0;
            ch_dwell = 0;
            ch_busy = 0;
        }
    }
}

static void processDrvEvent(void *data, uint8_t *msg, uint32_t len)
{
    uint8_t   evt = msg[0];

    switch(evt) {
        case drv_evt_packet:
            processALEvent1905Packet(msg+1, len-1);
            break;
        case drv_evt_radio:
            processRadioUpdate(msg+1, len-1);
            break;
        case drv_evt_bss:
            processBSSUpdate(msg+1, len-1);
            break;
        case drv_evt_bss_delete:
            processBSSDelete(msg+1, len-1);
            break;
        case drv_evt_station:
            processStationUpdate(msg+1, len-1);
            break;
        case drv_evt_station_delete:
            processStationDelete(msg+1, len-1);
            break;
        case drv_evt_chan_scan_finished:
            processChanScanFinished(msg+1, len-1);
            break;
        case drv_evt_chan_scan_results:
            processChanScanResults(msg+1, len-1);
            break;
        case drv_evt_mgmt_frame:
            processMgmtFrame(msg+1, len-1);
            break;
        case drv_evt_connect:
            processConnect(msg+1, len-1);
            break;
        case drv_evt_disconnect:
            processDisconnect(msg+1, len-1);
            break;
        case drv_evt_config_change:
            processConfigChange();
            break;
        case drv_evt_disallow_notification:
            processDisallowNotification(msg+1, len-1);
            break;
        case drv_evt_vip_conf_changed:
            processVipConfUpdate();
            break;
        case drv_evt_chan_survey:
            processChanSurvey(msg+1, len-1);
            break;
        default:
            DEBUG_WARNING("Unknown driver event type (%d)\n", evt);
            break;
    }
}

struct vlan_config_item *findVlanConfig(uint16_t vlan, struct ssid *ssid)
{
    struct vlan_config_item *item;
    dlist_for_each(item, local_policy.vlans, l2.l) {
        if ((item->vlan == vlan) && ((!ssid) ||(item->ssid.len==0) || (!SSIDCMP(*ssid, item->ssid))))
            return item;
    }
    return NULL;
}

struct vlan_config_item *addVlanConfig(uint16_t vlan)
{
    struct vlan_config_item *item = calloc(1, sizeof(struct vlan_config_item));
    item->vlan = vlan;
    dlist_add_tail(&local_policy.vlans, &item->l2.l);
    return item;
}

int checkTrafficSeperation(dlist_head *bsses)
{
    int ret = 0;
    struct wifi_config *config;
    struct vlan_config_item *item;

    dlist_for_each(config, *bsses, l) {
        uint16_t *p_vlan = config->bss.vlan_map;
        while(*p_vlan) {
            if (!config->bss.backhaul){
                if (!(item = findVlanConfig(*p_vlan, &config->bss.ssid))) {
                    ret = -1;
                    item = addVlanConfig(*p_vlan);
                } else {
                }
                SSIDCPY(item->ssid, config->bss.ssid);
            } else {
                if (!(item = findVlanConfig(*p_vlan, NULL))) {
                    ret = -1;
                    item = addVlanConfig(*p_vlan);
                }
            }
            if (!config->bss.backhaul)
                break;
            else
                p_vlan++;
        }
    }

    return ret;
}

////////////////////////////////////////////////////////////////////////////////
// Public functions
////////////////////////////////////////////////////////////////////////////////

uint8_t start1905AL(uint8_t *al_mac_address, uint8_t map_whole_network_flag, char *registrar_interface)
{
    void *timer_discovery=NULL, *timer_garbage=NULL, *timer_autoconf=NULL, *timer_sta_stats=NULL;
    struct interface_list_item *item;
    dlist_head *interfaces_names;

    initMid();
    // Initialize platform-specific code
    //
    if (PLATFORM_INIT()) {
        DEBUG_ERROR("Failed to initialize platform\n");
        return AL_ERROR_OS;
    }

    if (NULL == al_mac_address)
    {
        // Invalid arguments
        //
        DEBUG_ERROR("NULL AL MAC address not allowed\n");
        return AL_ERROR_INVALID_ARGUMENTS;
    }

    // Insert the provided AL MAC address into the database
    //
    //DMinit();
    configuratorGetConfig(&local_config);
    DMalMacSet(al_mac_address);
    configuratorGetDeviceInfo(&local_device->device_info);
    addLocalInterfaces();
    configuratorGetPolicy(&local_policy);
    configuratorGetVlan(&local_policy);
    init_non_std_tlv_ops();

    msgInit();
    featInit();

    msgRegisterFamily(msg_family_driver_evt, processDrvEvent, NULL);
    startExtensions();

    while (configuratorGetWifiConfig(&local_config.wifi_config.bsses) < 0) {
        DEBUG_ERROR("configuratorGetWifiConfig failed. try again after 5s\n");
        usleep(5000*1000);
    }

    if (local_config.controller) {
        checkTrafficSeperation(&local_config.wifi_config.bsses);
        setRegistrar(local_device);
        local_device->configured = 1;
    }

    if (local_config.auto_role)
        doAutoRole();

    InitVipConf(&local_policy);

    //discovery timer for every 60s
    DEBUG_DETAIL("Registering DISCOVERY time out event (periodic)...\n");
    timer_discovery = platformAddTimer(60000, TIMER_FLAG_PERIODIC, _timerDiscovery, NULL);

    //garbage clean timer for every 70s
    DEBUG_DETAIL("Registering GARBAGE COLLECTOR time out event (periodic)...\n");
    timer_garbage = platformAddTimer(70000, TIMER_FLAG_PERIODIC, _timerGarbageCollector, NULL);

    //station link metrics and stats period get timer for every 5s
    DEBUG_DETAIL("Registering station metrics and stats get timer out event (periodic)...\n");
    timer_sta_stats = platformAddTimer(1000, TIMER_FLAG_PERIODIC, _timerGetStationStats, NULL);

    // autoconf search or get config timer
    DEBUG_DETAIL("Registering auto configuration time out event (periodic)...\n");
    if (!isRegistrar() && local_config.autoconf_timeout) {
        timer_autoconf = platformAddTimer(local_config.autoconf_timeout*1000, TIMER_FLAG_PERIODIC, _timerAutoConfig, NULL);
    }

    // start a one time immidiated discovery
    DEBUG_DETAIL("Registering a one time forced DISCOVERY event...\n");
    platformAddTimer(1000, 0, _timerDiscovery, NULL);

    // register receive on local interfaces
    DEBUG_INFO("Registering packet arrival event for each interface...\n");

    interfaces_names = PLATFORM_GET_LIST_OF_1905_INTERFACES();
    dlist_for_each(item, *interfaces_names, l) {
        struct interface *intf = interfaceFindName(local_device, item->intf_name);
        if (intf) {
            platformRegisterReceiveInterface(intf);
            //Make sure interface is up
            usleep(20000);
        }
        DEBUG_INFO("%s: done\n", item->intf_name);
    }

    DEBUG_DETAIL("Entering read-process loop...\n");
    uloop_run();

    //clean up for quit
    platformCancelTimer(timer_autoconf);
    platformCancelTimer(timer_garbage);
    platformCancelTimer(timer_discovery);
    platformCancelTimer(timer_sta_stats);
    uloop_done();
    PLATFORM_DEINIT();
    return 0;
}



