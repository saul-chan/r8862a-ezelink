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

#include "1905_cmdus.h"
#include "1905_tlvs.h"
#include "packet_tools.h"
#include "al_recv.h"

////////////////////////////////////////////////////////////////////////////////
// Auxiliary, static tables
////////////////////////////////////////////////////////////////////////////////

//   WARNING:
//     If the CMDU message type changes (ie. the definition of  CMD_TYPE_*)
//     the following tables will have to be adapted (as the array index depends
//     on that).
//     Fortunately this should never happen.

// These tables marks, for each CMDU type of message, which TLVs are:
//
//   1. Require to be present zero or more times
//
//   2. required to be present exactly once
//
// The values in these tables were obtained from "IEEE Std 1905.1-2013, Section
// 6.3"
//
//
// TODO: 
//     Right now this mechanism only considers either "zero or more" or "exactly
//     one" possibilities... however, in the "1a" update of the standard, there
//     are new types of TLVs that can appear "zero or one" and "one or more"
//     times.
//     For now I'm treating:
//       A) ...the "zero or one" type as "zero or more" (this
//          happens with the "push button generic phy event notification TLV",
//          the "control URL TLV" and the "IPv4/v6 TLVs") and...
//       B) ...the "one or more" type as "exactly one" (the "interface power
//          change information type TLV" and the "interface power change status
//          TLV").
//     Case (B) is not really a problem (in fact, I think "one or more" is an
//     error in the standard for these TLVs... as it should be "exactly one"...
//     maybe this will be corrected in a future update).
//     However, because of case (A), we could end up considering valid CMDUs
//     with, for example, more than one "IPv4 TLVs" (which is clearly an error).
//
//
//
#if 0
static uint32_t _zeroormore_tlvs_for_cmdu[] = 
{
    /* CMDU_TYPE_TOPOLOGY_DISCOVERY             */  0x00000000,
    /* CMDU_TYPE_TOPOLOGY_NOTIFICATION          */  0x00000000,
    /* CMDU_TYPE_TOPOLOGY_QUERY                 */  0x00000000,
    /* CMDU_TYPE_TOPOLOGY_RESPONSE              */  1 << TLV_TYPE_DEVICE_BRIDGING_CAPABILITIES   | 1 << TLV_TYPE_NON_1905_NEIGHBOR_DEVICE_LIST | 1<< TLV_TYPE_NEIGHBOR_DEVICE_LIST | 1 << TLV_TYPE_POWER_OFF_INTERFACE | 1 << TLV_TYPE_L2_NEIGHBOR_DEVICE,
    /* CMDU_TYPE_VENDOR_SPECIFIC                */  0xffffffff,
    /* CMDU_TYPE_LINK_METRIC_QUERY              */  0x00000000,
    /* CMDU_TYPE_LINK_METRIC_RESPONSE           */  1 << TLV_TYPE_TRANSMITTER_LINK_METRIC | 1 << TLV_TYPE_RECEIVER_LINK_METRIC,
    /* CMDU_TYPE_AP_AUTOCONFIGURATION_SEARCH    */  0x00000000,
    /* CMDU_TYPE_AP_AUTOCONFIGURATION_RESPONSE  */  0x00000000,
    /* CMDU_TYPE_AP_AUTOCONFIGURATION_WSC       */  0x00000000,
    /* CMDU_TYPE_AP_AUTOCONFIGURATION_RENEW     */  0x00000000,
    /* CMDU_TYPE_PUSH_BUTTON_EVENT_NOTIFICATION */  1 << TLV_TYPE_GENERIC_PHY_EVENT_NOTIFICATION,
    /* CMDU_TYPE_PUSH_BUTTON_JOIN_NOTIFICATION  */  0x00000000,
    /* CMDU_TYPE_HIGHER_LAYER_QUERY             */  0x00000000,
    /* CMDU_TYPE_HIGHER_LAYER_RESPONSE          */  1 << TLV_TYPE_CONTROL_URL | 1 << TLV_TYPE_IPV4 | 1 << TLV_TYPE_IPV6,
    /* CMDU_TYPE_INTERFACE_POWER_CHANGE_REQUEST */  0x00000000,
    /* CMDU_TYPE_INTERFACE_POWER_CHANGE_RESPONSE*/  0x00000000,
    /* CMDU_TYPE_GENERIC_PHY_QUERY              */  0x00000000,
    /* CMDU_TYPE_GENERIC_PHY_RESPONSE           */  0x00000000,
};

static uint32_t _exactlyone_tlvs_for_cmdu[] = \
{
    /* CMDU_TYPE_TOPOLOGY_DISCOVERY             */  1 << TLV_TYPE_AL_MAC_ADDRESS_TYPE     | 1 << TLV_TYPE_MAC_ADDRESS_TYPE,
    /* CMDU_TYPE_TOPOLOGY_NOTIFICATION          */  1 << TLV_TYPE_AL_MAC_ADDRESS_TYPE,
    /* CMDU_TYPE_TOPOLOGY_QUERY                 */  0x00000000,
    /* CMDU_TYPE_TOPOLOGY_RESPONSE              */  1 << TLV_TYPE_DEVICE_INFORMATION_TYPE,
    /* CMDU_TYPE_VENDOR_SPECIFIC                */  0x00000000,
    /* CMDU_TYPE_LINK_METRIC_QUERY              */  1 << TLV_TYPE_LINK_METRIC_QUERY,
    /* CMDU_TYPE_LINK_METRIC_RESPONSE           */  0x00000000,
    /* CMDU_TYPE_AP_AUTOCONFIGURATION_SEARCH    */  1 << TLV_TYPE_AL_MAC_ADDRESS_TYPE     | 1 << TLV_TYPE_SEARCHED_ROLE                  | 1 << TLV_TYPE_AUTOCONFIG_FREQ_BAND,
    /* CMDU_TYPE_AP_AUTOCONFIGURATION_RESPONSE  */  1 << TLV_TYPE_SUPPORTED_ROLE          | 1 << TLV_TYPE_SUPPORTED_FREQ_BAND,
    /* CMDU_TYPE_AP_AUTOCONFIGURATION_WSC       */  1 << TLV_TYPE_WSC,
    /* CMDU_TYPE_AP_AUTOCONFIGURATION_RENEW     */  1 << TLV_TYPE_AL_MAC_ADDRESS_TYPE     | 1 << TLV_TYPE_SUPPORTED_ROLE                 | 1 << TLV_TYPE_SUPPORTED_FREQ_BAND,
    /* CMDU_TYPE_PUSH_BUTTON_EVENT_NOTIFICATION */  1 << TLV_TYPE_AL_MAC_ADDRESS_TYPE     | 1 << TLV_TYPE_PUSH_BUTTON_EVENT_NOTIFICATION,
    /* CMDU_TYPE_PUSH_BUTTON_JOIN_NOTIFICATION  */  1 << TLV_TYPE_AL_MAC_ADDRESS_TYPE     | 1 << TLV_TYPE_PUSH_BUTTON_JOIN_NOTIFICATION,
    /* CMDU_TYPE_HIGHER_LAYER_QUERY             */  0x00000000,
    /* CMDU_TYPE_HIGHER_LAYER_RESPONSE          */  1 << TLV_TYPE_AL_MAC_ADDRESS_TYPE     | 1 << TLV_TYPE_1905_PROFILE_VERSION           | 1 << TLV_TYPE_DEVICE_IDENTIFICATION,
    /* CMDU_TYPE_INTERFACE_POWER_CHANGE_REQUEST */  1 << TLV_TYPE_INTERFACE_POWER_CHANGE_INFORMATION,
    /* CMDU_TYPE_INTERFACE_POWER_CHANGE_RESPONSE*/  1 << TLV_TYPE_INTERFACE_POWER_CHANGE_STATUS,
    /* CMDU_TYPE_GENERIC_PHY_QUERY              */  0x00000000,
    /* CMDU_TYPE_GENERIC_PHY_RESPONSE           */  1 << TLV_TYPE_GENERIC_PHY_DEVICE_INFORMATION,
};

// The following table tells us the value of the 'relay_indicator' flag for
// each type of CMDU message.
//
// The values were obtained from "IEEE Std 1905.1-2013, Table 6-4"
//
// Note that '0xff' is a special value that means: "this CMDU message type can
// have the flag set to either '0' or '1' and its actual value for this
// particular message must be specified in some other way"
//
static uint8_t _relayed_CMDU[] = \
{
    /* CMDU_TYPE_TOPOLOGY_DISCOVERY             */  0,
    /* CMDU_TYPE_TOPOLOGY_NOTIFICATION          */  1,
    /* CMDU_TYPE_TOPOLOGY_QUERY                 */  0,
    /* CMDU_TYPE_TOPOLOGY_QUERY                 */  0,
    /* CMDU_TYPE_VENDOR_SPECIFIC                */  0xff,
    /* CMDU_TYPE_LINK_METRIC_QUERY              */  0,
    /* CMDU_TYPE_LINK_METRIC_RESPONSE           */  0,
    /* CMDU_TYPE_AP_AUTOCONFIGURATION_SEARCH    */  1,
    /* CMDU_TYPE_AP_AUTOCONFIGURATION_RESPONSE  */  0,
    /* CMDU_TYPE_AP_AUTOCONFIGURATION_WSC       */  0,
    /* CMDU_TYPE_AP_AUTOCONFIGURATION_RENEW     */  1,
    /* CMDU_TYPE_PUSH_BUTTON_EVENT_NOTIFICATION */  1,
    /* CMDU_TYPE_PUSH_BUTTON_JOIN_NOTIFICATION  */  1,
    /* CMDU_TYPE_HIGHER_LAYER_QUERY             */  0,
    /* CMDU_TYPE_HIGHER_LAYER_RESPONSE          */  0,
    /* CMDU_TYPE_INTERFACE_POWER_CHANGE_REQUEST */  0,
    /* CMDU_TYPE_INTERFACE_POWER_CHANGE_RESPONSE*/  0,
    /* CMDU_TYPE_GENERIC_PHY_QUERY              */  0,
    /* CMDU_TYPE_GENERIC_PHY_RESPONSE           */  0, 
};
#endif

////////////////////////////////////////////////////////////////////////////////
// Auxiliary static functions
////////////////////////////////////////////////////////////////////////////////

// Each CMDU must follow some rules regarding which TLVs they can contain 
// depending on their type.
//
// This is extracted from "IEEE Std 1905.1-2013, Section 6.2":
//
//   1. When generating a CMDU:
//      a) It shall include all of the TLVs that are listed for the message
//      b) It shall not include any other TLV that is not listed for the message
//      c) It may additionally include zero or more vendor specific TLVs
//
//   2. When receiving a CMDU:
//      a) It may process or ignore any vendor specific TLVs
//      b) It shall ignore all TLVs that are not specified for the message
//      c) It shall ignore the entire message if the message does not include
//         all of the TLVs that are listed for this message
//
// This function receives a pointer to a CMDU structure, 'p' and a 'rules_type'
// value:
//
//   * If 'rules_type' == CHECK_CMDU_TX_RULES, the function will check the
//     structure against the "generating a CMDU" rules (ie. rules 1.a, 1.b and
//     1.c).
//     If any of them is broken this function returns "0" (and 'p' is *not*
//     freed, as this is the caller's responsability)
//
//   * If 'rules_type' == CHECK_CMDU_RX_RULES, the function will check the
//     structure against the "receiving a CMDU" rules (ie. rules 2.a, 2.b and
//     2.c)
//     Regarding rule 2.a, we have chosen to preserve vendor specific TLVs in
//     the structure.
//     Rule 2.b is special in that non-vendor specific TLVs that are not
//     specified for the message type are removed (ie. the 'p' structure is
//     modified!)
//     Rule 2.c is special in that if it is broken, 'p' is freed
//
//  Note a small asymmetry: with 'rules_type' == CHECK_CMDU_TX_RULES,
//  unexpected options cause the function to fail while with 'rules_type' ==
//  CHECK_CMDU_RX_RULES they are simply removed (and freed) from the structure.
//  If you think about it, this is the correct behaviour: in transmission,
//  do not let invalid packets to be generated, while in reception, if invalid
//  packets are receive, ignore the unexpected pieces but process the rest.
//
//  In both cases, this function returns:
//    '0' --> If 'p' did not respect the rules and could not be "fixed"
//    '1' --> If 'p' was modified (ie. it is now valid). This can only happen
//            when 'rules_type' == CHECK_CMDU_RX_RULES
//    '2' --> If 'p' was not modifed (ie. it was valid from the beginning)
//
#define CHECK_CMDU_TX_RULES (1)
#define CHECK_CMDU_RX_RULES (2)
#if 0
static uint8_t _check_CMDU_rules(struct CMDU *p, uint8_t rules_type)
{
    uint8_t  i;
    uint8_t  structure_has_been_modified;
    uint8_t  counter[TLV_TYPE_LAST];
    uint8_t  tlvs_to_remove[TLV_TYPE_LAST];

    if ((NULL == p) || (NULL == p->list_of_TLVs))
    {
        // Invalid arguments
        //
        DEBUG_ERROR("Invalid CMDU structure\n");
        return 0;
    }

    // First of all, count how many times each type of TLV message appears in
    // the structure. We will use this information later
    //
    for (i=0; i<=TLV_TYPE_LAST; i++)
    {
        counter[i]        = 0;
        tlvs_to_remove[i] = 0;
    }

    i = 0;
    while (NULL != p->list_of_TLVs[i])
    {
        counter[*(p->list_of_TLVs[i])]++;
        printf("add type %d\n", *(p->list_of_TLVs[i]));
        i++;
    }
    
    // Rules 1.a and 2.c check the same thing : make sure the structure
    // contains, *at least*, the required TLVs
    //
    // If not, return '0'
    //
    // The required TLVs are those contained in the "_exactlyone_tlvs_for_cmdu"
    // table.
    //
    for (i=0; i<=TLV_TYPE_LAST; i++)
    {
        if (
             (1 != counter[i])                                       &&
             (_exactlyone_tlvs_for_cmdu[p->message_type] & (1 << i))
             )
        {
            DEBUG_WARNING("TLV %s/%d should appear once, but it appears %d times\n",
                                            convert_1905_TLV_type_to_string(i), i, counter[i]);
            return 0;
        }
    }

    // Rules 1.b and 2.b also both check for the same thing (unexpected TLVs),
    // but they act in different ways:
    //
    //   * In case 'rules_type' == CHECK_CMDU_TX_RULES, return '0'
    //   * In case 'rules_type' == CHECK_CMDU_RX_RULES, remove the unexpected
    //     TLVs (and later, when all other checks have been performed, return
    //     '1' to indicate that the structure has been modified)
    //  
    // Unexpected TLVs are those that do not appear in neither the
    // "_exactlyone_tlvs_for_cmdu" nor the "_zeroormore_tlvs_for_cmdu" tables
    // 
    for (i=0; i<=TLV_TYPE_LAST; i++)
    {
        if (
             (0 != counter[i])                                        &&
             (i != TLV_TYPE_VENDOR_SPECIFIC)                          &&
             !(_zeroormore_tlvs_for_cmdu[p->message_type] & (1 << i)) &&
             !(_exactlyone_tlvs_for_cmdu[p->message_type] & (1 << i))
             )
        {
            if (CHECK_CMDU_TX_RULES == rules_type)
            {
                DEBUG_WARNING("TLV %s should not appear on this CMDU, but it appears %d times\n", convert_1905_TLV_type_to_string(i), counter[i]);
                return 0;
            }
            else
            {
                tlvs_to_remove[i] = 1;
            }
        }
    }
    i = 0;
    structure_has_been_modified = 0;
    while (NULL != p->list_of_TLVs[i])
    {
        // Here we will just traverse the list of TLVs and remove the ones
        // that shouldn't be there.
        // When this happens, mark the structure as 'modified' so that we can
        // later return the appropriate return code.
        //
        //   NOTE:
        //     When removing TLVs they are first freed and the list of
        //     pointers ('list_of_TLVs') is simply overwritten.
        //     The original piece of memory that holds all pointers is not
        //     redimensioned, though, as it would make things unnecessary more
        //     complex.
        //     In other words:
        //
        //       Before removal:
        //         list_of_TLVs --> [p1, p2, p3, NULL]
        //
        //       After removing p2:
        //         list_of_TLVs --> [p1, p3, NULL, NULL]
        //
        //       ...and not:
        //         list_of_TLVs --> [p1, p3, NULL]
        //
        if (1 == tlvs_to_remove[*(p->list_of_TLVs[i])])
        {
            uint8_t j;

            free_1905_TLV_structure(p->list_of_TLVs[i]);

            structure_has_been_modified = 1;
            j = i + 1;
            while (p->list_of_TLVs[j])
            {
                p->list_of_TLVs[j-1] = p->list_of_TLVs[j];
                j++;
            }
            p->list_of_TLVs[j-1] = p->list_of_TLVs[j];
        }
        else
        {
           i++;
        }
    }

    // Regarding rules 1.c and 2.a, we don't really have to do anything special,
    // thus we can return now
    //
    if (1 == structure_has_been_modified)
    {
        return 1;
    }
    else
    {
        return 2;
    }
}

#endif

////////////////////////////////////////////////////////////////////////////////
// Actual API functions
////////////////////////////////////////////////////////////////////////////////

struct CMDU2 *parse_1905_CMDU_from_packets(uint8_t **packet_streams, uint16_t* lens)
{
    struct CMDU2 *c = NULL;

    uint8_t  fragments_nr;
    uint8_t  current_fragment;

    uint8_t  error;

    if (NULL == packet_streams)
    {
        // Invalid arguments
        //
        DEBUG_ERROR("NULL packet_streams\n");
        return NULL;
    }

    // Find out how many streams/fragments we have received
    //
    fragments_nr = 0;
    while (*(packet_streams+fragments_nr))
    {
        fragments_nr++;
    }
    if (0 == fragments_nr)
    {
        // No streams supplied!
        //
        DEBUG_ERROR("No fragments supplied\n");
        return NULL;
    }

    error = 0;
    for (current_fragment = 0; current_fragment<fragments_nr; current_fragment++)
    {
        uint8_t *p;
        uint16_t len;
        uint8_t i;

        uint8_t   message_version;
        uint8_t   reserved_field;
        uint16_t  message_type;
        uint16_t  message_id;
        uint8_t   fragment_id;

        uint8_t   indicators;

        uint8_t   relay_indicator;
        uint8_t   last_fragment_indicator;

        struct TLV *tlv;

        // We want to traverse fragments in order, thus lets search for the
        // fragment whose 'fragment_id' matches 'current_fragment' (which will
        // monotonically increase starting at '0')
        //
        for (i=0; i<fragments_nr; i++)
        {
            p = *(packet_streams+i);
            len = *(lens+i);
            // The 'fragment_id' field is the 7th byte (offset 6)
            //
            if (current_fragment == *(p+6))
            {
                break;
            }
        }
        if (i == fragments_nr)
        {
            // One of the fragments is missing!
            //
            error = 1;
            break;
        }

        // At this point 'p' points to the stream whose 'fragment_id' is
        // 'current_fragment'

        // Let's parse the header fields
        //
        _E1B(&p, &message_version);
        _E1B(&p, &reserved_field);
        _E2B(&p, &message_type);
        _E2B(&p, &message_id);
        _E1B(&p, &fragment_id);
        _E1B(&p, &indicators);

        last_fragment_indicator = (indicators & 0x80) >> 7; // MSB and 2nd MSB
        relay_indicator         = (indicators & 0x40) >> 6; // of the
                                                            // 'indicators'
                                                            // field

        len -= 8;
        if (0 == current_fragment)
        {
            // This is the first fragment, thus fill the 'common' values.
            // We will later (in later fragments) check that their values always
            // remain the same
            //

            c = cmdu2New(message_type, message_id);
            if (!c)
                break;

            c->relay = relay_indicator;
        }
        else
        {
            // Check for consistency in all 'common' values
            //
           if (
                (c->version != message_version) ||
                (c->type    != message_type)    ||
                (c->id      != message_id)      ||
                (c->relay != relay_indicator)
              )
           {
               // Fragments with different common fields were detected!
               //
               error = 2;
               break;
           }
        }

        // Regarding the 'relay_indicator', depending on the message type, it
        // can only have a valid specific value
        //
#if 0
        if (0xff == _relayed_CMDU[message_type])
        {
            // Special, case. All values are allowed
        }
        else
        {
            // Check if the value for this type of message is valid
            //
            if (_relayed_CMDU[message_type] != relay_indicator)
            {
                // Malformed packet
                //
                error = 3;
                break;
            }
        }
#endif
        // Regarding the 'last_fragment_indicator' flag, the following condition
        // must be met: the last fragement (and only it!) must have it set to
        // '1'
        //
        if ((1 == last_fragment_indicator) && (current_fragment < fragments_nr-1))
        {
            // 'last_fragment_indicator' appeared *before* the last fragment
            error = 4;
            break;
        }
        if ((0 == last_fragment_indicator) && (current_fragment == fragments_nr-1))
        {
            // 'last_fragment_indicator' did not appear in the last fragment
            error = 5;
            break;
        }

        while (len>=3)
        {
            uint8_t *p1 = p;
            uint8_t tlv_type;
            uint16_t tlv_len;

            _E1B(&p1, &tlv_type);
            _E2B(&p1, &tlv_len);

            tlv_len += 3;

            if (tlv_len>len) {
                DEBUG_WARNING("Parsing error, not fit for one TLV\n");
                break;
            }

            tlv = decodeTLV(p);
            if (tlv) {
                if (TLV_TYPE_END_OF_MESSAGE == tlv->tlv_type) {
                    tlist_delete_item(&tlv->s.t);
                    break;
                }
                cmdu2AddTlv(c, tlv);
            }

            p += tlv_len;
            len -= tlv_len;
        }
    }

    if ((c) && (0 == error)) {
        // Ok then... we now have our output structure properly filled.
        // However, there is one last battery of checks we must perform:
        //
        //   - CMDU_TYPE_VENDOR_SPECIFIC: The first TLV *must* be of type
        //     TLV_TYPE_VENDOR_SPECIFIC
        //
        //   - All the other message types: Some TLVs (different for each of
        //     them) can only appear once, others can appear zero or more times
        //     and others must be ignored.
        //     The '_check_CMDU_rules()' takes care of this for us.
        //
        DEBUG_DETAIL("CMDU type: %s\n", convert_1905_CMDU_type_to_string(c->type));
#if 0
        {

            switch (_check_CMDU_rules(ret, CHECK_CMDU_RX_RULES))
            {
                case 0:
                {
                    // The structure was missing some required TLVs. This is
                    // a malformed packet which must be ignored.
                    //
                    DEBUG_WARNING("Structure is missing some required TLVs\n");
                    DEBUG_WARNING("List of present TLVs:\n");

                    if (NULL != ret->list_of_TLVs)
                    {
                        uint8_t i;

                        i = 0;
                        while (ret->list_of_TLVs[i])
                        {
                            DEBUG_WARNING("  - %s\n", convert_1905_TLV_type_to_string(*(ret->list_of_TLVs[i])));
                            i++;
                        }
                        DEBUG_WARNING("  - <END>\n");
                    }
                    else
                    {
                        DEBUG_WARNING("  - <NONE>\n");
                    }

                    free_1905_CMDU_structure(ret);
                    return NULL;
                }
                case 1:
                {
                    // The structure contained unxecpected TLVs. They have been
                    // removed for us.
                    //
                    break;
                }
                case 2:
                {
                    // The structure was perfect and '_check_CMDU_rules()' did
                    // not need to modify anything.
                    //
                    break;
                }
                default:
                {
                    // This point should never be reached
                    //
                    error = 8;
                    break;
                }
            }
        }
#endif
    }

    // Finally! If we get this far without errors we are already done, otherwise
    // free everything and return NULL
    //
    if (0 != error)
    {
        cmdu2Free(c);
        return NULL;
    }

    return c;
}





#define FILL_CMDU_HEAD(cmdu, buffer, fragment_id, indicator) \
    do { \
        uint8_t value8 = 0;\
        uint8_t *s = cmduBufPush((buffer), 8);\
        _I1B(&(cmdu)->version, &s);\
        _I1B(&value8, &s);\
        _I2B(&(cmdu)->type, &s);\
        _I2B(&(cmdu)->id, &s);\
        _I1B(&(fragment_id), &s);\
        _I1B(&(indicator), &s);\
    }while(0)

#define CHECK_BUF_ROOM(_cmdu, _buf, _len, _fragment, _indicator) \
    do { \
        if (cmduBufTailRoom((_buf))<(_len)) { \
            FILL_CMDU_HEAD((_cmdu), (_buf), (_fragment), (_indicator)); \
            (_fragment)++; \
            dlist_add_tail(&(_cmdu)->streams, &(_buf)->l); \
            (_buf) = cmduBufNew(MAX_NETWORK_SEGMENT_SIZE, 22); \
        } \
    }while(0)

#define ADD_EOM(buffer) \
    do {\
        memset((buffer)->end, 0, 3);\
        cmduBufPut((buffer), 3);\
    }while(0)

int updateCMDU(struct CMDU2 *c, uint16_t mid)
{
    c->id = mid;
    if (!dlist_empty(&c->streams)) {
        struct cmdu_buf *buf;
        uint8_t *p;
        dlist_for_each(buf, c->streams, l) {
            p = buf->data+4;
            _I2B(&c->id, &p);
        }
    }
    return 0;
}

int encodeCMDU(struct CMDU2 *c)
{
    int ret = -1;
    struct cmdu_buf *buf;
    struct TLV *tlv;
    uint8_t *stream = NULL;
    uint16_t stream_len;
    uint8_t fragment = 0, indicator = (c->relay<<6);

    //already encoded, when retry happened
    if (!dlist_empty(&c->streams)) {
        return 0;
    }

    buf = cmduBufNew(MAX_NETWORK_SEGMENT_SIZE, 22);

    if (!dlist_empty(&c->tlvs)) {
        tlv = container_of(dlist_get_first(&c->tlvs), struct TLV, s.t.l);

        do {
            encodeTLV(tlv, &stream, &stream_len);
            if (!stream)
                return 0;

            CHECK_BUF_ROOM(c, buf, stream_len, fragment, indicator);
            CMDUBUF_PUT_DATA(buf, stream, stream_len);
            free(stream);
            stream = NULL;
            if (!(tlv = container_of(dlist_get_next(&c->tlvs, &tlv->s.t.l), struct TLV, s.t.l)))
                break;
        } while (1);
    }
    CHECK_BUF_ROOM(c, buf, 3, fragment, indicator);
    ADD_EOM(buf);
    indicator |= (1<<7);//last fragment
    FILL_CMDU_HEAD(c, buf, fragment, indicator);
    dlist_add_tail(&c->streams, &buf->l);
    ret = 0;
    return ret;
}


uint8_t parse_1905_CMDU_header_from_packet(uint8_t *stream, uint16_t *mid, uint8_t *fragment_id, uint8_t *last_fragment_indicator)
{
    uint8_t   message_version;
    uint8_t   reserved_field;
    uint16_t  message_type;
    uint8_t   indicators;

    if ((NULL == stream) || (NULL == mid) || (NULL == fragment_id) || (NULL == last_fragment_indicator))
    {
        // Invalid params
        //
        return 0;
    }

    // Let's parse the header fields
    //
    _E1B(&stream, &message_version);
    _E1B(&stream, &reserved_field);
    _E2B(&stream, &message_type);
    _E2B(&stream, mid);
    _E1B(&stream, fragment_id);
    _E1B(&stream, &indicators);

    *last_fragment_indicator = (indicators & 0x80) >> 7; // MSB and 2nd MSB

    return 1;
}


void free_1905_CMDU_packets(uint8_t **packet_streams)
{
    uint8_t i;

    if (NULL == packet_streams)
    {
        return;
    }

    i = 0;
    while (packet_streams[i])
    {
        PLATFORM_FREE(packet_streams[i]);
        i++;
    }
    PLATFORM_FREE(packet_streams);

    return;
}


#define DEFINE_CMDU_TYPE_DESC(_type, _resptype, _func, _flag) \
    [CMDU_TYPE_##_type] = { .name = #_type, .resp_type = CMDU_TYPE_##_resptype, .process = _func, .flag = _flag }
static struct cmdu_msg_desc _cmdu_msg_type_descs[] =
{
    DEFINE_CMDU_TYPE_DESC(TOPOLOGY_DISCOVERY, INVALID, processTopologyDiscovery, 0),
    DEFINE_CMDU_TYPE_DESC(TOPOLOGY_NOTIFICATION, INVALID, processTopologyNotification, 0),
    // do not use retry as we have timeout for topology query
    DEFINE_CMDU_TYPE_DESC(TOPOLOGY_QUERY, INVALID, processTopologyQuery, 0),
    DEFINE_CMDU_TYPE_DESC(TOPOLOGY_RESPONSE, INVALID, processTopologyResponse, 0),
    DEFINE_CMDU_TYPE_DESC(VENDOR_SPECIFIC, INVALID, NULL, 0),
    DEFINE_CMDU_TYPE_DESC(LINK_METRIC_QUERY, INVALID, processLinkMetricQuery, 0),
    DEFINE_CMDU_TYPE_DESC(LINK_METRIC_RESPONSE, INVALID, processLinkMetricResponse, 0),
    DEFINE_CMDU_TYPE_DESC(AP_AUTOCONFIGURATION_SEARCH, INVALID, processAPAutoconfigurationSearch, 0),
    DEFINE_CMDU_TYPE_DESC(AP_AUTOCONFIGURATION_RESPONSE, INVALID, processAPAutoconfigurationResponse, 0),
    DEFINE_CMDU_TYPE_DESC(AP_AUTOCONFIGURATION_WSC, INVALID, processAPAutoconfiguratioWSC, 0),
    DEFINE_CMDU_TYPE_DESC(AP_AUTOCONFIGURATION_RENEW, INVALID, processAPAutoconfigurationRenew, 0),
    DEFINE_CMDU_TYPE_DESC(PUSH_BUTTON_EVENT_NOTIFICATION, INVALID, NULL, 0),
    DEFINE_CMDU_TYPE_DESC(PUSH_BUTTON_JOIN_NOTIFICATION, INVALID, NULL, 0),
    DEFINE_CMDU_TYPE_DESC(HIGHER_LAYER_QUERY, HIGHER_LAYER_RESPONSE, processHigherLayerQuery, 0),
    DEFINE_CMDU_TYPE_DESC(HIGHER_LAYER_RESPONSE, INVALID, processHigherLayerResponse, FLAG_RESPONSE_TYPE),
    DEFINE_CMDU_TYPE_DESC(INTERFACE_POWER_CHANGE_REQUEST, INVALID, NULL, 0),
    DEFINE_CMDU_TYPE_DESC(INTERFACE_POWER_CHANGE_RESPONSE, INVALID, NULL, 0),
    DEFINE_CMDU_TYPE_DESC(GENERIC_PHY_QUERY, INVALID, NULL, 0),
    DEFINE_CMDU_TYPE_DESC(GENERIC_PHY_RESPONSE, INVALID, NULL, 0),
};


#define DEFINE_CMDU_TYPE_MAP_DESC(_type, _resptype, _func, _flag) \
    [CMDU_TYPE_##_type - CMDU_TYPE_MAP_FIRST] = { .name = #_type, .resp_type = CMDU_TYPE_##_resptype, .process = _func, .flag = _flag }
static struct cmdu_msg_desc _cmdu_msg_type_map_descs[CMDU_TYPE_MAP_LAST  \
                 - CMDU_TYPE_MAP_FIRST + 1] =
{
    /* R1 */
    DEFINE_CMDU_TYPE_MAP_DESC(ACK, INVALID, NULL, FLAG_RESPONSE_TYPE),
    DEFINE_CMDU_TYPE_MAP_DESC(AP_CAPABILITY_QUERY, AP_CAPABILITY_REPORT, processAPCapabilityQuery, 0),
    DEFINE_CMDU_TYPE_MAP_DESC(AP_CAPABILITY_REPORT, INVALID, processAPCapabilityReport, FLAG_RESPONSE_TYPE),
    DEFINE_CMDU_TYPE_MAP_DESC(MAP_POLICY_CONFIG_REQUEST, ACK, processMapPolicyConfigRequest, 0),
    DEFINE_CMDU_TYPE_MAP_DESC(CHANNEL_PREFERENCE_QUERY, CHANNEL_PREFERENCE_REPORT, processChannelPreferenceQuery, 0),
    DEFINE_CMDU_TYPE_MAP_DESC(CHANNEL_PREFERENCE_REPORT, INVALID, processChannelPreferenceReport,
                                (FLAG_RESPONSE_TYPE|FLAG_UNSOLICITED)),
    DEFINE_CMDU_TYPE_MAP_DESC(CHANNEL_SELECTION_REQUEST, CHANNEL_SELECTION_RESPONSE, processChannelSelectionRequest, 0),
    DEFINE_CMDU_TYPE_MAP_DESC(CHANNEL_SELECTION_RESPONSE, INVALID, processChannelSelectionResponse, FLAG_RESPONSE_TYPE),
    DEFINE_CMDU_TYPE_MAP_DESC(OPERATING_CHANNEL_REPORT, ACK, processOperatingChannelReport, 0),
    DEFINE_CMDU_TYPE_MAP_DESC(CLIENT_CAPABILITY_QUERY, CLIENT_CAPABILITY_REPORT, processClientCapabilityQuery, 0),
    DEFINE_CMDU_TYPE_MAP_DESC(CLIENT_CAPABILITY_REPORT, INVALID, processClientCapabilityReport, FLAG_RESPONSE_TYPE),
    DEFINE_CMDU_TYPE_MAP_DESC(AP_METRICS_QUERY, AP_METRICS_RESPONSE, processAPMetricsQuery, 0),
    DEFINE_CMDU_TYPE_MAP_DESC(AP_METRICS_RESPONSE, INVALID, processAPMetricsResponse, FLAG_RESPONSE_TYPE),
    DEFINE_CMDU_TYPE_MAP_DESC(ASSOCIATED_STA_LINK_METRICS_QUERY, ASSOCIATED_STA_LINK_METRICS_RESPONSE,
                                processAssociatedStaLinkMetricsQuery, 0),
    DEFINE_CMDU_TYPE_MAP_DESC(ASSOCIATED_STA_LINK_METRICS_RESPONSE, INVALID, processAssociatedStaLinkMetricsResponse,
                                FLAG_RESPONSE_TYPE),
    DEFINE_CMDU_TYPE_MAP_DESC(UNASSOCIATED_STA_LINK_METRICS_QUERY, ACK, processUnassociatedStaLinkMetricsQuery, 0),
    DEFINE_CMDU_TYPE_MAP_DESC(UNASSOCIATED_STA_LINK_METRICS_RESPONSE, ACK, processUnassociatedStaLinkMetricsResponse, 0),
    DEFINE_CMDU_TYPE_MAP_DESC(BEACON_METRICS_QUERY, ACK, processBeaconMetricsQuery, 0),
    DEFINE_CMDU_TYPE_MAP_DESC(BEACON_METRICS_RESPONSE, ACK, processBeaconMetricsResponse, 0),
    DEFINE_CMDU_TYPE_MAP_DESC(COMBINED_INFRASTRUCTURE_METRICS, ACK, processCombinedInfrastructureMetrics, 0),
    DEFINE_CMDU_TYPE_MAP_DESC(CLIENT_STEERING_REQUEST, ACK, processClientSteeringRequest, 0),
    DEFINE_CMDU_TYPE_MAP_DESC(CLIENT_STEERING_BTM_REPORT, ACK, processSteeringBTMReport, 0),
    DEFINE_CMDU_TYPE_MAP_DESC(CLIENT_ASSOCIATION_CONTROL, ACK, processClientAssocControlRequest, 0),
    DEFINE_CMDU_TYPE_MAP_DESC(STEERING_COMPLETED, ACK, processSteeringComplete, 0),
    DEFINE_CMDU_TYPE_MAP_DESC(HIGHER_LAYER_DATA, ACK, processHigherLayerData, 0),
    DEFINE_CMDU_TYPE_MAP_DESC(BACKHAUL_STEERING_REQUEST, ACK, processBackhaulSteeringRequest, 0),
    DEFINE_CMDU_TYPE_MAP_DESC(BACKHAUL_STEERING_RESPONSE, ACK, processBackhaulSteeringResponse, 0),

    /* R2 */
    DEFINE_CMDU_TYPE_MAP_DESC(CHANNEL_SCAN_REQUEST, ACK, processChannelScanRequest, 0),
    DEFINE_CMDU_TYPE_MAP_DESC(CHANNEL_SCAN_REPORT, ACK, processChannelScanReport, 0),
    DEFINE_CMDU_TYPE_MAP_DESC(CAC_REQUEST, ACK, processCacRequest, 0),
    DEFINE_CMDU_TYPE_MAP_DESC(CAC_TERMINATION, ACK, processCacTermination, 0),
    DEFINE_CMDU_TYPE_MAP_DESC(CLIENT_DISASSOCIATION_STATS, INVALID, processClientDisassocStat, 0),
    DEFINE_CMDU_TYPE_MAP_DESC(ERROR_RESPONSE, INVALID, processErrorResponse, 0),
    DEFINE_CMDU_TYPE_MAP_DESC(ASSOCIATION_STATUS_NOTIFICATION, INVALID, processAssocStatusNotification, 0),
    DEFINE_CMDU_TYPE_MAP_DESC(TUNNELED, ACK, NULL, 0),
    DEFINE_CMDU_TYPE_MAP_DESC(BACKHAUL_STA_CAPABILITY_QUERY, BACKHAUL_STA_CAPABILITY_REPORT,
                                processBackhaulStaCapabilityQuery, 0),
    DEFINE_CMDU_TYPE_MAP_DESC(BACKHAUL_STA_CAPABILITY_REPORT, INVALID, processBackhaulStaCapabilityReport,
                                FLAG_RESPONSE_TYPE),
    DEFINE_CMDU_TYPE_MAP_DESC(FAILED_CONNECTION, INVALID, processFailedConnection, 0),

    /* R3 */
    DEFINE_CMDU_TYPE_MAP_DESC(DPP_CCE_INDICATION, INVALID, NULL, 0),
    DEFINE_CMDU_TYPE_MAP_DESC(1905_REKEY_REQUEST, ACK, NULL, 0),
    DEFINE_CMDU_TYPE_MAP_DESC(1905_DECRYPTION_FAILURE, INVALID, NULL, 0),
    DEFINE_CMDU_TYPE_MAP_DESC(SERVICE_PRIORITIZATION_REQUEST, ACK, processServicePrioritizationRequest, 0),
    DEFINE_CMDU_TYPE_MAP_DESC(PROXIED_ENCAP_DPP, INVALID, NULL, 0),
    DEFINE_CMDU_TYPE_MAP_DESC(DIRECT_ENCAP_DPP, INVALID, NULL, 0),
    DEFINE_CMDU_TYPE_MAP_DESC(RECONFIGURATION_TRIGGER, INVALID, NULL, 0),
    DEFINE_CMDU_TYPE_MAP_DESC(BSS_CONFIGURATION_REQUEST, BSS_CONFIGURATION_RESPONSE, NULL, 0),
    DEFINE_CMDU_TYPE_MAP_DESC(BSS_CONFIGURATION_RESPONSE, INVALID, NULL, FLAG_RESPONSE_TYPE),
    DEFINE_CMDU_TYPE_MAP_DESC(BSS_CONFIGURATION_RESULT, INVALID, NULL, 0),
    DEFINE_CMDU_TYPE_MAP_DESC(CHIRP_NOTIFICATION, INVALID, NULL, 0),
    DEFINE_CMDU_TYPE_MAP_DESC(1905_ENCAP_EAPOL, INVALID, NULL, 0),
    DEFINE_CMDU_TYPE_MAP_DESC(DPP_BOOTSTRAPPING_URI_NOTIFICATION, INVALID, NULL, 0),
    DEFINE_CMDU_TYPE_MAP_DESC(DPP_BOOTSTRAPPING_URI_QUERY, INVALID, NULL, 0),
    DEFINE_CMDU_TYPE_MAP_DESC(AGENT_LIST, INVALID, NULL, 0),

    /* R5 */
    DEFINE_CMDU_TYPE_MAP_DESC(VBSS_CAPABILITIES_REQUEST, VBSS_CAPABILITIES_RESPONSE, processVBSSCapabilitiesRequest, 0),
    DEFINE_CMDU_TYPE_MAP_DESC(VBSS_CAPABILITIES_RESPONSE, INVALID, processVBSSCapabilitiesResponse, FLAG_RESPONSE_TYPE),
    // don't retry for vbss request now.
    //DEFINE_CMDU_TYPE_MAP_DESC(VBSS_REQUEST, VBSS_RESPONSE, processVBSSRequest, 0),
    DEFINE_CMDU_TYPE_MAP_DESC(VBSS_REQUEST, INVALID, processVBSSRequest, 0),
    DEFINE_CMDU_TYPE_MAP_DESC(VBSS_RESPONSE, INVALID, processVBSSResponse, FLAG_RESPONSE_TYPE),
    DEFINE_CMDU_TYPE_MAP_DESC(CLIENT_SECURITY_CONTEXT_REQUEST, CLIENT_SECURITY_CONTEXT_RESPONSE,
                                processClientSecurityContextRequest, 0),
    DEFINE_CMDU_TYPE_MAP_DESC(CLIENT_SECURITY_CONTEXT_RESPONSE, INVALID, processClientSecurityContextResponse, FLAG_RESPONSE_TYPE),
    DEFINE_CMDU_TYPE_MAP_DESC(TRIGGER_CSA_REQUEST, TRIGGER_CSA_RESPONSE, processTriggerCSARequest, 0),
    DEFINE_CMDU_TYPE_MAP_DESC(TRIGGER_CSA_RESPONSE, INVALID, processTriggerCSAResponse, FLAG_RESPONSE_TYPE),
    DEFINE_CMDU_TYPE_MAP_DESC(VBSS_MOVE_PREPARATION_REQUEST, VBSS_MOVE_PREPARATION_RESPONSE, processVbssMovePreparationRequest, 0),
    DEFINE_CMDU_TYPE_MAP_DESC(VBSS_MOVE_PREPARATION_RESPONSE, INVALID, processVbssMovePreparationResponse, FLAG_RESPONSE_TYPE),
    DEFINE_CMDU_TYPE_MAP_DESC(VBSS_MOVE_CANCEL_REQUEST, VBSS_MOVE_CANCEL_RESPONSE, processVbssMoveCancelRequest, 0),
    DEFINE_CMDU_TYPE_MAP_DESC(VBSS_MOVE_CANCEL_RESPONSE, INVALID, processVbssMoveCancelResponse, FLAG_RESPONSE_TYPE),

};


struct cmdu_msg_desc *CMDU_type_to_desc(uint16_t cmdu_type)
{
    struct cmdu_msg_desc *desc = NULL;

    if (cmdu_type <= CMDU_TYPE_1905_LAST)
        desc = &_cmdu_msg_type_descs[cmdu_type];
    if (cmdu_type >= CMDU_TYPE_MAP_FIRST
        && cmdu_type <= CMDU_TYPE_MAP_LAST)
        desc = &_cmdu_msg_type_map_descs[cmdu_type - CMDU_TYPE_MAP_FIRST];

    return desc;
}

const char *convert_1905_CMDU_type_to_string(uint16_t cmdu_type)
{
    struct cmdu_msg_desc *desc = CMDU_type_to_desc(cmdu_type);

    if ((desc) && (desc->name))
        return desc->name;
    else
        return "Unknown";
}

struct CMDU2 *cmdu2New(uint16_t type, uint16_t mid)
{
    struct CMDU2 *c = (struct CMDU2 *)malloc(sizeof(struct CMDU2));

    if (c) {
        c->version = CMDU_MESSAGE_VERSION_1905_1_2013;
        c->type = type;
        c->id = mid;
        c->relay = 0;
        c->ref = 0;
        dlist_head_init(&c->tlvs);
        dlist_head_init(&c->streams);
    } else
        DEBUG_ERROR("can not allocate cmdu2 for [%s]\n",
                                    convert_1905_CMDU_type_to_string(type));

    return c;
}

//coverity[+free : arg-0]
void cmdu2Free(struct CMDU2 *c)
{
    if (c) {
        dlist_item *item;

        if (c->ref--) return;
        while ((item = dlist_get_first(&c->tlvs))) {
            struct TLV *tlv =  container_of(item, struct TLV, s.t.l);
            TLVFree(tlv);
        }
        while ((item = dlist_get_first(&c->streams))) {
            struct cmdu_buf *cbuf = container_of(item, struct cmdu_buf, l);
            dlist_remove(item);
            cmduBufFree(cbuf);
        }
        free(c);
    }
}

struct CMDU2 *cmdu2Ref(struct CMDU2 *c)
{
    if (c)
        c->ref++;
    return c;
}

struct TLV *getTypedTLV(struct CMDU2 *c, uint8_t type, int idx)
{
    struct TLV *tlv;

    if (c) {
        dlist_for_each(tlv, c->tlvs, s.t.l) {
            if (tlv->tlv_type == type) {
                if (idx)
                    idx--;
                else
                    return tlv;
            }
        }
    }
    return NULL;
}

struct TLV *getSubTypedTLV(struct CMDU2 *c, uint8_t type, uint16_t subtype, int idx)
{
    struct superTLV *tlv;
    struct TLV *sub_tlv;

    if (c) {
        dlist_for_each(tlv, c->tlvs, tlv.s.t.l) {
            if ((tlv->tlv.tlv_type == type) && ((sub_tlv=tlv->sub_tlv)) &&
                (sub_tlv->tlv_subtype==subtype)) {
                if (idx)
                    idx--;
                else
                    return sub_tlv;
            }
        }
    }
    return NULL;
}

struct TLV *getCLSTypedTLV(struct CMDU2 *c, uint8_t type, int idx)
{
    struct vendorTLV *tlv;
    struct TLV *cls_tlv;

    if (c) {
        dlist_for_each(tlv, c->tlvs, super.tlv.s.t.l) {
            if ((tlv->super.tlv.tlv_type == TLV_TYPE_VENDOR_SPECIFIC) &&
                (!OUICMP(tlv->oui, CLS_OUI)) && ((cls_tlv = tlv->super.sub_tlv)) &&
                (cls_tlv->tlv_type==type)) {
                if (idx)
                    idx--;
                else
                    return cls_tlv;

            }
        }
    }
    return NULL;
}

struct TLVStruct *getChildTLVStruct(struct TLVStruct *parent, uint8_t order, int idx)
{
    struct TLVStruct *tlv_s;
    if (parent) {
        dlist_for_each(tlv_s, parent->t.childs[order], t.l) {
            if (idx)
                idx--;
            else
                return tlv_s;
        }
    }
    return NULL;
}

void cmdu2AddTlv(struct CMDU2 *c, struct TLV *tlv)
{
    if (tlv) dlist_add_tail(&c->tlvs, &tlv->s.t.l);
}



