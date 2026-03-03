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
#include "packet_tools.h"
#include "platform_crypto.h"
#include "platform_interfaces.h"
#include "al_wsc.h"
#include "1905_tlvs.h"


////////////////////////////////////////////////////////////////////////////////
// Private functions and data
////////////////////////////////////////////////////////////////////////////////

struct wscKey
{
    uint8_t  *key;
    uint32_t  key_len;
    uint8_t   mac[6];
};



// This is the key derivation function used in the WPS standard to obtain a
// final hash that is later used for encryption.
//
// The output is stored in the memory buffer pointed by 'res', which must be
// "SHA256_MAC_LEN" bytes long (ie. 're_len' must always be "SHA256_MAC_LEN",
// even if it is an input argument)
//
void _wps_key_derivation_function(uint8_t *key, uint8_t *label_prefix, uint32_t label_prefix_len, char *label, uint8_t *res, uint32_t res_len)
{
    uint8_t i_buf[4];
    uint8_t key_bits[4];

    uint8_t   *addr[4];
    uint32_t   len[4];

    uint32_t i, iter;

    uint8_t  hash[SHA256_MAC_LEN];
    uint8_t *opos;

    uint32_t left;

    uint8_t  *p;
    uint32_t  aux;

    aux = res_len * 8;
    p   = key_bits;

    _I4B(&aux, &p);

    addr[0] = i_buf;
    addr[1] = label_prefix;
    addr[2] = (uint8_t *) label;
    addr[3] = key_bits;
    len[0]  = sizeof(i_buf);
    len[1]  = label_prefix_len;
    len[2]  = PLATFORM_STRLEN(label);
    len[3]  = sizeof(key_bits);

    iter = (res_len + SHA256_MAC_LEN - 1) / SHA256_MAC_LEN;
    opos = res;
    left = res_len;

    for (i = 1; i <= iter; i++)
    {
        p = i_buf;
        _I4B(&i, &p);

        PLATFORM_HMAC_SHA256(key, SHA256_MAC_LEN, 4, addr, len, hash);

        if (i < iter)
        {
            PLATFORM_MEMCPY(opos, hash, SHA256_MAC_LEN);
            opos += SHA256_MAC_LEN;
            left -= SHA256_MAC_LEN;
        }
        else
        {
            if (left<=SHA256_MAC_LEN)
                PLATFORM_MEMCPY(opos, hash, left);
        }
    }
}


#define WSC_PUT_U8(_attr, _value) \
    do { \
        aux16 = (_attr);    _I2B(&aux16, &p); \
        aux16 = 1;          _I2B(&aux16, &p); \
        aux8 = (_value);    _I1B(&aux8,  &p); \
    } while (0)

#define WSC_PUT_U16(_attr, _value) \
    do { \
        aux16 = (_attr);    _I2B(&aux16, &p); \
        aux16 = 2;          _I2B(&aux16, &p); \
        aux16 = (_value);   _I2B(&aux16,  &p); \
    } while (0)

#define WSC_PUT_N(_attr, _len, _value) \
    do { \
        if ((_value)) { \
            aux16 = (_attr);    _I2B(&aux16, &p); \
            aux16 = (_len);     _I2B(&aux16, &p); \
            _InB((_value), &p, (_len)); \
        } \
    } while (0)

#define WSC_PUT_N_NOCHECK(_attr, _len, _value) \
    do { \
        aux16 = (_attr);    _I2B(&aux16, &p); \
        aux16 = (_len);     _I2B(&aux16, &p); \
        _InB((_value), &p, (_len)); \
    } while (0)

#define MAX_WSC_SIZE 1000
struct TLV *wscBuildM1(struct radio *r)
{
    uint8_t *p;
    struct device_vendor_info *info = &local_config.device_info;
    uint8_t  aux8;
    uint16_t aux16;
    uint32_t aux32;

    struct wscTLV *m1 = (struct wscTLV *)TLVNew(NULL, TLV_TYPE_WSC, sizeof(struct wscTLV)+MAX_WSC_SIZE);
    m1->wsc.datap = (uint8_t *)(m1+1);

    p = m1->wsc.datap;

    if (r->wsc) {
        if (r->wsc->priv_key)
            free(r->wsc->priv_key);
        if (r->wsc->m1)
            TLVFree(r->wsc->m1);
        memset(r->wsc, 0, sizeof(struct wsc_context));
    } else
        r->wsc = (struct wsc_context *)calloc(1, sizeof(struct wsc_context));
    //store m1
    r->wsc->m1 = TLVRef((struct TLV *)m1);
    r->wsc->mac = local_device->al_mac;
    // VERSION
    WSC_PUT_U8(ATTR_VERSION, 0x10);
    // MESSAGE TYPE
    WSC_PUT_U8(ATTR_MSG_TYPE, WPS_M1);
    // UUID
    WSC_PUT_N(ATTR_UUID_E, UUID_LEN, info->uuid);
    // MAC ADDRESS
    WSC_PUT_N(ATTR_MAC_ADDR, MACLEN, local_device->al_mac);
    // ENROLLEE NONCE
    {
        uint8_t nonce[NONCE_LEN];
        PLATFORM_GET_RANDOM_BYTES(nonce, NONCE_LEN);
        WSC_PUT_N_NOCHECK(ATTR_ENROLLEE_NONCE, NONCE_LEN, nonce);
        r->wsc->nonce = p-NONCE_LEN;
    }
    // PUBLIC KEY
    {
        uint8_t *pub;
        uint16_t pub_len;

        if (!PLATFORM_GENERATE_DH_KEY_PAIR(&r->wsc->priv_key, &r->wsc->priv_key_len, &pub, &pub_len)) {
            DEBUG_ERROR("Fail to generate DH key pair\n");
            goto fail;
        }
        WSC_PUT_N(ATTR_PUBLIC_KEY, pub_len, pub);
        free(pub);
    }
    // AUTHENTICATION TYPES
    {
        uint16_t auth_types = (auth_open | auth_wpa2psk);

        if (!local_config.wfa_mode)
            auth_types |= (auth_wpapsk | auth_shared);

        if (r->sae_capa_valid)
            auth_types |= auth_sae;
        r->wsc->auth = auth_types;
        WSC_PUT_U16(ATTR_AUTH_TYPE_FLAGS, auth_types);
    }
    // ENCRYPTION TYPES
    {
        uint16_t  encryption_types = encrypt_none | encrypt_aes;
        r->wsc->encrypt = encryption_types;
        WSC_PUT_U16(ATTR_ENCR_TYPE_FLAGS, encryption_types);
    }
    // CONNECTION TYPES
    WSC_PUT_U8(ATTR_CONN_TYPE_FLAGS, WPS_CONN_ESS);
    // CONFIGURATION METHODS
    WSC_PUT_U16(ATTR_CONFIG_METHODS, WPS_CONFIG_PHY_PUSHBUTTON | WPS_CONFIG_VIRT_PUSHBUTTON);
    // WPS STATE
    WSC_PUT_U8(ATTR_WPS_STATE, WPS_STATE_NOT_CONFIGURED);
    // MANUFACTURER
    WSC_PUT_N(ATTR_MANUFACTURER, MIN(strlen(info->manufacturer), 64), info->manufacturer);
    // MODEL NAME
    WSC_PUT_N(ATTR_MODEL_NAME, MIN(strlen(info->model_name), 32), info->model_name);
    // MODEL NUMBER
    WSC_PUT_N(ATTR_MODEL_NUMBER, MIN(strlen(info->model_num), 32), info->model_num);
    // SERIAL NUMBER
    WSC_PUT_N(ATTR_SERIAL_NUMBER, MIN(strlen(info->serial_no), 32), info->serial_no);
    // PRIMARY DEVICE TYPE
    {
        uint8_t oui[4]      = {0x00, 0x50, 0xf2, 0x00};
        aux16 = ATTR_PRIMARY_DEV_TYPE;                                    _I2B(&aux16,         &p);
        aux16 = 8;                                                        _I2B(&aux16,         &p);
        aux16 = WPS_DEV_NETWORK_INFRA;                                    _I2B(&aux16,         &p);
                                                                          _InB( oui,           &p, 4);
        aux16 = WPS_DEV_NETWORK_INFRA_ROUTER;                             _I2B(&aux16,         &p);
    }
    // DEVICE NAME
    WSC_PUT_N(ATTR_DEV_NAME, MIN(strlen(info->device_name), 32), info->device_name);
    // RF BANDS
    {
        uint8_t  rf_band = 0;

        if (r->bands & (band_5g | band_6g))
            rf_band = WPS_RF_5GHZ;
        else if (r->bands & band_2g)
            rf_band = WPS_RF_2GHZ;
        WSC_PUT_U8(ATTR_RF_BANDS, rf_band);
    }
    // ASSOCIATION STATE
    WSC_PUT_U16(ATTR_ASSOC_STATE, WPS_ASSOC_NOT_ASSOC);
    // DEVICE PASSWORD ID
    WSC_PUT_U16(ATTR_DEV_PASSWORD_ID, DEV_PW_PUSHBUTTON);
    // CONFIG ERROR
    WSC_PUT_U16(ATTR_CONFIG_ERROR, WPS_CFG_NO_ERROR);
    // OS VERSION
    {
        uint32_t os_version = 0x00000001;

        aux16 = ATTR_OS_VERSION;                                          _I2B(&aux16,         &p);
        aux16 = 4;                                                        _I2B(&aux16,         &p);
        aux32 = 0x80000000 | os_version;                                  _I4B(&aux32,         &p);
    }
    // VENDOR EXTENSIONS
    {
        aux16 = ATTR_VENDOR_EXTENSION;                                    _I2B(&aux16,         &p);
        aux16 = 6;                                                        _I2B(&aux16,         &p);
        aux8  = WPS_VENDOR_ID_WFA_1;                                      _I1B(&aux8,          &p);
        aux8  = WPS_VENDOR_ID_WFA_2;                                      _I1B(&aux8,          &p);
        aux8  = WPS_VENDOR_ID_WFA_3;                                      _I1B(&aux8,          &p);
        aux8  = WFA_ELEM_VERSION2;                                        _I1B(&aux8,          &p);
        aux8  = 1;                                                        _I1B(&aux8,          &p);
        aux8  = WPS_VERSION;                                              _I1B(&aux8,          &p);
    }

    m1->wsc.len = p-m1->wsc.datap;

    return (struct TLV *)m1;
fail:
    TLVFree((struct TLV *)m1);
    return NULL;
}

uint8_t wscProcessM1(struct TLV *m1, struct wsc_m1_info *info)
{
    struct wscTLV *wsc_tlv = (struct wscTLV *)m1;
    uint8_t *p = wsc_tlv->wsc.datap;

    info->m1 = p;
    info->m1_len = wsc_tlv->wsc.len;

    while (p-info->m1<info->m1_len) {
        uint16_t attr, len;

         _E2B(&p, &attr);
         _E2B(&p, &len);

         switch(attr) {
             case ATTR_MAC_ADDR:
                if (len==MACLEN)
                    info->al_mac = p;
                break;
             case ATTR_ENROLLEE_NONCE:
                if (len==NONCE_LEN)
                    info->nonce = p;
                break;
             case ATTR_PUBLIC_KEY:
                info->pub_len = len;
                info->pub = p;
                break;
             case ATTR_RF_BANDS:
                if (len==1)
                    info->bands = *p;
                break;
             case ATTR_AUTH_TYPE_FLAGS:
                if (len==2) {
                    _E2B(&p, &info->auth);
                    continue;
                }
                break;
             case ATTR_ENCR_TYPE_FLAGS:
                 if (len==2) {
                    _E2B(&p, &info->encrypt);
                    continue;
                }
                break;
             default:
                break;
         };
         p += len;
    };
    return 0;
}


struct TLV *wscBuildM2(struct wifi_config *wcfg, struct wsc_m1_info *m1_info)
{
    uint8_t *p, *p1;
    struct device_vendor_info *info = &local_config.device_info;
    uint8_t  aux8;
    uint16_t aux16;
    uint32_t aux32;

    struct wscTLV *m2 = (struct wscTLV *)TLVNew(NULL, TLV_TYPE_WSC, sizeof(struct wscTLV)+MAX_WSC_SIZE);
    m2->wsc.datap = (uint8_t *)(m2+1);

    uint8_t  *local_privkey;
    uint16_t  local_privkey_len;

    uint8_t authkey   [WPS_AUTHKEY_LEN];
    uint8_t keywrapkey[WPS_KEYWRAPKEY_LEN];
    uint8_t emsk      [WPS_EMSK_LEN];

    uint8_t  registrar_nonce[NONCE_LEN];

    p = m2->wsc.datap;

    // VERSION
    WSC_PUT_U8(ATTR_VERSION, 0x10);
    // MESSAGE TYPE
    WSC_PUT_U8(ATTR_MSG_TYPE, WPS_M2);
    // ENROLLEE NONCE
    WSC_PUT_N(ATTR_ENROLLEE_NONCE, NONCE_LEN, m1_info->nonce);
    // REGISTRAR NONCE
    {
        PLATFORM_GET_RANDOM_BYTES(registrar_nonce, NONCE_LEN);
        WSC_PUT_N_NOCHECK(ATTR_REGISTRAR_NONCE, NONCE_LEN, registrar_nonce);
    }
    // UUID
    WSC_PUT_N(ATTR_UUID_R, UUID_LEN, info->uuid);
    // PUBLIC KEY
    {
        uint8_t  *priv, *pub;
        uint16_t  priv_len, pub_len;

        PLATFORM_GENERATE_DH_KEY_PAIR(&priv, &priv_len, &pub, &pub_len);

        WSC_PUT_N(ATTR_PUBLIC_KEY, pub_len, pub);
        free(pub);
        local_privkey     = priv;
        local_privkey_len = priv_len;
    }

    // Key derivation (no bytes are written to the output buffer in the next
    // block of code, we just obtain three cryptographic keys that are needed
    // later
    {
        uint8_t  *shared_secret;
        uint16_t  shared_secret_len;

        uint8_t  *addr[3];
        uint32_t  len[3];

        uint8_t   dhkey[SHA256_MAC_LEN];
        uint8_t   kdk  [SHA256_MAC_LEN];

        uint8_t keys      [WPS_AUTHKEY_LEN + WPS_KEYWRAPKEY_LEN + WPS_EMSK_LEN];

        // With the enrolle public key (which we received in M1) and our private
        // key (which we generated above), obtain the Diffie Hellman shared
        // secret (when receiving M2, the enrollee will be able to obtain this
        // same shared secret using its private key and our public key
        // -contained in M2-)
        //
        PLATFORM_COMPUTE_DH_SHARED_SECRET(&shared_secret, &shared_secret_len, m1_info->pub, m1_info->pub_len, local_privkey, local_privkey_len);
        // TODO: ZERO PAD the shared_secret (doesn't seem to be really needed
        // though)

        // Next, obtain the SHA-256 digest of this shared secret. We will call
        // this "dhkey"
        //
        addr[0] = shared_secret;
        len[0]  = shared_secret_len;

        PLATFORM_SHA256(1, addr, len, dhkey);
        PLATFORM_FREE(shared_secret);

        // Next, concatenate three things (the enrollee nonce contained in M1,
        // the enrolle MAC address -also contained in M1-, and the nonce we just
        // generated before and calculate its HMAC (hash message authentication
        // code) using "dhkey" as the secret key.
        //
        addr[0] = m1_info->nonce;
        addr[1] = m1_info->al_mac;
        addr[2] = registrar_nonce;
        len[0]  = NONCE_LEN;
        len[1]  = MACLEN;
        len[2]  = NONCE_LEN;

        PLATFORM_HMAC_SHA256(dhkey, SHA256_MAC_LEN, 3, addr, len, kdk);

        // Finally, take "kdk" and using a function provided in the "Wi-Fi
        // simple configuration" standard, obtain THREE KEYS that we will use
        // later ("authkey", "keywrapkey" and "emsk")
        //
        _wps_key_derivation_function(kdk, NULL, 0, "Wi-Fi Easy and Secure Key Derivation", keys, sizeof(keys));

        PLATFORM_MEMCPY(authkey,    keys,                                        WPS_AUTHKEY_LEN);
        PLATFORM_MEMCPY(keywrapkey, keys + WPS_AUTHKEY_LEN,                      WPS_KEYWRAPKEY_LEN);
        PLATFORM_MEMCPY(emsk,       keys + WPS_AUTHKEY_LEN + WPS_KEYWRAPKEY_LEN, WPS_EMSK_LEN);
    }

    // AUTHENTICATION TYPES
    WSC_PUT_U16(ATTR_AUTH_TYPE_FLAGS, m1_info->auth);
    // ENCRYPTION TYPES
    WSC_PUT_U16(ATTR_ENCR_TYPE_FLAGS, m1_info->encrypt);
    // CONNECTION TYPES
    WSC_PUT_U8(ATTR_CONN_TYPE_FLAGS, WPS_CONN_ESS);
    // CONFIGURATION METHODS
    WSC_PUT_U16(ATTR_CONFIG_METHODS, WPS_CONFIG_PHY_PUSHBUTTON | WPS_CONFIG_VIRT_PUSHBUTTON);
    // MANUFACTURER
    WSC_PUT_N(ATTR_MANUFACTURER, MIN(strlen(info->manufacturer), 64), info->manufacturer);
    // MODEL NAME
    WSC_PUT_N(ATTR_MODEL_NAME, MIN(strlen(info->model_name), 32), info->model_name);
    // MODEL NUMBER
    WSC_PUT_N(ATTR_MODEL_NUMBER, MIN(strlen(info->model_num), 32), info->model_num);
    // SERIAL NUMBER
    WSC_PUT_N(ATTR_SERIAL_NUMBER, MIN(strlen(info->serial_no), 32), info->serial_no);
    // PRIMARY DEVICE TYPE
    {
        uint8_t oui[4]      = {0x00, 0x50, 0xf2, 0x00}; // Fixed value from the
                                                      // WSC spec

        aux16 = ATTR_PRIMARY_DEV_TYPE;                                    _I2B(&aux16,         &p);
        aux16 = 8;                                                        _I2B(&aux16,         &p);
        aux16 = WPS_DEV_NETWORK_INFRA;                                    _I2B(&aux16,         &p);
                                                                          _InB( oui,           &p, 4);
        aux16 = WPS_DEV_NETWORK_INFRA_ROUTER;                             _I2B(&aux16,         &p);
    }
    // DEVICE NAME
    WSC_PUT_N(ATTR_DEV_NAME, MIN(strlen(info->device_name), 32), info->device_name);
    // RF BANDS
    WSC_PUT_U8(ATTR_RF_BANDS, m1_info->bands);
    // ASSOCIATION STATE
    WSC_PUT_U16(ATTR_ASSOC_STATE, WPS_ASSOC_CONN_SUCCESS);
    // DEVICE PASSWORD ID
    WSC_PUT_U16(ATTR_DEV_PASSWORD_ID, DEV_PW_PUSHBUTTON);
    // CONFIG ERROR
    WSC_PUT_U16(ATTR_CONFIG_ERROR, WPS_CFG_NO_ERROR);
    // OS VERSION
    {
        uint32_t os_version = 0x00000001;

        aux16 = ATTR_OS_VERSION;                                          _I2B(&aux16,         &p);
        aux16 = 4;                                                        _I2B(&aux16,         &p);
        aux32 = 0x80000000 | os_version;                                  _I4B(&aux32,         &p);
    }
    // VENDOR EXTENSIONS
    {
        aux16 = ATTR_VENDOR_EXTENSION;                                    _I2B(&aux16,         &p);
        aux16 = 6;                                                        _I2B(&aux16,         &p);
        aux8  = WPS_VENDOR_ID_WFA_1;                                      _I1B(&aux8,          &p);
        aux8  = WPS_VENDOR_ID_WFA_2;                                      _I1B(&aux8,          &p);
        aux8  = WPS_VENDOR_ID_WFA_3;                                      _I1B(&aux8,          &p);
        aux8  = WFA_ELEM_VERSION2;                                        _I1B(&aux8,          &p);
        aux8  = 1;                                                        _I1B(&aux8,          &p);
        aux8  = WPS_VERSION;                                              _I1B(&aux8,          &p);
    }
    // ENCRYPTED SETTINGS
    {
        uint8_t   plain[200];
        uint32_t  plain_len;
        uint8_t   hash[SHA256_MAC_LEN];
        uint8_t  *iv_start;
        uint8_t  *data_start;

        uint8_t  pad_elements_nr;

        uint8_t  *addr[1];
        uint32_t  len[1];
        struct bss_info *bss;

        //save p
        p1 = p;
        p = plain;
        if (wcfg) {
            bss = &wcfg->bss;

            // SSID
            WSC_PUT_N(ATTR_SSID, bss->ssid.len, bss->ssid.ssid);
            // AUTH TYPE
            WSC_PUT_U16(ATTR_AUTH_TYPE, bss->auth);
            // ENCRYPTION TYPE
            WSC_PUT_U16(ATTR_ENCR_TYPE, bss->encrypt);
            // NETWORK KEY
            WSC_PUT_N(ATTR_NETWORK_KEY, bss->key.len, bss->key.key);
            // BSSID
            WSC_PUT_N(ATTR_MAC_ADDR, MACLEN, bss->bssid);
        }
        // WFA extension
        {
            uint8_t map_ext[9] = {WPS_VENDOR_ID_WFA_1, WPS_VENDOR_ID_WFA_2, WPS_VENDOR_ID_WFA_3,
                WFA_ELEM_VERSION2, 1, WPS_VERSION, WFA_ELEM_MAP_EXT, 1, 0};
            aux8 = 0;
            if (wcfg) {
                if (bss->backhaul)
                    aux8 |= MAP_BACKHAUL_BSS;
                if (bss->fronthaul)
                    aux8 |= MAP_FRONTHAUL_BSS;
            } else
                aux8 |= MAP_TEAR_DOWN;
            map_ext[8] = aux8;
            WSC_PUT_N_NOCHECK(ATTR_VENDOR_EXTENSION, 9, map_ext);
        }

        // Obtain the HMAC of the whole plain buffer using "authkey" as the
        // secret key.
        //
        addr[0] = plain;
        len[0]  = p-plain;;
        PLATFORM_HMAC_SHA256(authkey, WPS_AUTHKEY_LEN, 1, addr, len, hash);

        // ...and add it to the same plain buffer (well, only the first 8 bytes
        // of the hash)
        //
        WSC_PUT_N_NOCHECK(ATTR_KEY_WRAP_AUTH, 8, hash);

        // Finally, encrypt everything with AES and add the resulting blob to
        // the M2 buffer, as an "ATTR_ENCR_SETTINGS" attribute
        //
        //// Pad the length of the message to encrypt to a multiple of
        //// AES_BLOCK_SIZE. The new padded bytes must have their value equal to
        //// the amount of bytes padded (PKCS#5 v2.0 pad)
        ////
        pad_elements_nr = AES_BLOCK_SIZE - ((p-plain) % AES_BLOCK_SIZE);
        for (aux8 = 0; aux8<pad_elements_nr; aux8++)
        {
            _I1B(&pad_elements_nr, &p);
        }
        //// Add the attribute header ("type" and "lenght") to the M2 buffer,
        //// followed by the IV and the data to encrypt.
        ////
        plain_len = p - plain;
        // restore p
        p = p1;
        aux16 = ATTR_ENCR_SETTINGS;                                       _I2B(&aux16,         &p);
        aux16 = AES_BLOCK_SIZE + plain_len;                               _I2B(&aux16,         &p);
        iv_start   = p; PLATFORM_GET_RANDOM_BYTES(p, AES_BLOCK_SIZE); p+=AES_BLOCK_SIZE;
        data_start = p; _InB(plain, &p, plain_len);
        //// Encrypt the data IN-PLACE. Note that the "ATTR_ENCR_SETTINGS"
        //// attribute containes both the IV and the encrypted data.
        ////
        PLATFORM_AES_ENCRYPT(keywrapkey, iv_start, data_start, plain_len);
    }

    // AUTHENTICATOR
    {
        uint8_t  hash[SHA256_MAC_LEN];

        uint8_t  *addr[2];
        uint32_t  len[2];

        addr[0] = m1_info->m1;
        len[0] = m1_info->m1_len;
        addr[1] = m2->wsc.datap;
        len[1]  = p-m2->wsc.datap;

        PLATFORM_HMAC_SHA256(authkey, WPS_AUTHKEY_LEN, 2, addr, len, hash);
        WSC_PUT_N_NOCHECK(ATTR_AUTHENTICATOR, 8, hash);
    }

    m2->wsc.len = p-m2->wsc.datap;
    return (struct TLV *)m2;
}


struct wifi_config *wscProcessM2(struct TLV *m2, struct radio *r, uint8_t *teardown)
{
    uint8_t         *p;
    struct wsc_context *wsc = r->wsc;
    struct wscTLV *m1 = (struct wscTLV *)wsc->m1;
    struct wscTLV *m2_wsc = (struct wscTLV *)m2;

    struct wifi_config *wcfg = calloc(1, sizeof(struct wifi_config));
    struct bss_info *bss = &wcfg->bss;

    // Keys we need to compute to authenticate and decrypt M2
    //
    uint8_t authkey   [WPS_AUTHKEY_LEN];
    uint8_t keywrapkey[WPS_KEYWRAPKEY_LEN];
    uint8_t emsk      [WPS_EMSK_LEN];

    // "Intermediary data" we also need to extract from M2 to obtain the keys
    // that will let us decrypt the "useful data" from M2
    //
    uint8_t  *m2_nonce = NULL;
    uint8_t  *m2_pubkey = NULL;
    uint16_t  m2_pubkey_len;
    uint8_t  *m2_encrypted_settings = NULL;
    uint16_t  m2_encrypted_settings_len;
    uint8_t  *m2_authenticator = NULL;

    wcfg->bss.role = role_ap;
    p = m2_wsc->wsc.datap;
    while (p - m2_wsc->wsc.datap < m2_wsc->wsc.len)
    {
        uint16_t attr_type;
        uint16_t attr_len;

        _E2B(&p, &attr_type);
        _E2B(&p, &attr_len);

        switch (attr_type) {
            case ATTR_REGISTRAR_NONCE:
                if (NONCE_LEN != attr_len) {
                    DEBUG_WARNING("Incorrect length (%d) for ATTR_REGISTRAR_NONCE\n", attr_len);
                    goto fail;
                }
                m2_nonce = p;
                break;
            case ATTR_PUBLIC_KEY:
                m2_pubkey_len = attr_len;
                m2_pubkey = p;
                break;
            case ATTR_ENCR_SETTINGS:
                m2_encrypted_settings_len = attr_len;
                m2_encrypted_settings = p;
                break;
            case ATTR_AUTHENTICATOR:
                if (8 != attr_len) {
                    DEBUG_WARNING("Incorrect length (%d) for ATTR_AUTHENTICATOR\n", attr_len);
                    goto fail;
                }
                m2_authenticator = p;
                break;
            default:
                break;
        }

        p += attr_len;
    }
    if ((!m2_nonce) || (!m2_pubkey) || (!m2_encrypted_settings) || (!m2_authenticator)) {
        DEBUG_WARNING("Missing attributes in the received M2 message\n");
        goto fail;;
    }
    // With all the information we have just extracted from M1 and M2, obtain
    // the authentication/encryption keys.
    {
        uint8_t  *shared_secret;
        uint16_t  shared_secret_len;

        uint8_t  *addr[3];
        uint32_t  len[3];

        uint8_t   dhkey[SHA256_MAC_LEN];
        uint8_t   kdk  [SHA256_MAC_LEN];

        uint8_t keys[WPS_AUTHKEY_LEN + WPS_KEYWRAPKEY_LEN + WPS_EMSK_LEN];

        // With the enrolle public key (which we received in M1) and our private
        // key (which we generated above), obtain the Diffie Hellman shared
        // secret (when receiving M2, the enrollee will be able to obtain this
        // same shared secret using its private key and ou public key -contained
        // in M2-)
        //
        PLATFORM_COMPUTE_DH_SHARED_SECRET(&shared_secret, &shared_secret_len, m2_pubkey, m2_pubkey_len, wsc->priv_key, wsc->priv_key_len);

        // Next, obtain the SHA-256 digest of this shared secret. We will call
        // this "dhkey"
        //
        addr[0] = shared_secret;
        len[0]  = shared_secret_len;

        PLATFORM_SHA256(1, addr, len, dhkey);
        PLATFORM_FREE(shared_secret);

        // Next, concatenate three things (the enrolle nonce contained in M1,
        // the enrolle MAC address, and the nonce we just generated before, and
        // calculate its HMAC (hash message authentication code) using "dhkey"
        // as the secret key.
        //
        addr[0] = wsc->nonce;
        addr[1] = wsc->mac;
        addr[2] = m2_nonce;
        len[0]  = NONCE_LEN;
        len[1]  = MACLEN;
        len[2]  = NONCE_LEN;

        PLATFORM_HMAC_SHA256(dhkey, SHA256_MAC_LEN, 3, addr, len, kdk);

        // Finally, take "kdk" and using a function provided in the "Wi-Fi
        // simple configuration" standard, obtain THREE KEYS that we will use
        // later ("authkey", "keywrapkey" and "emsk")
        //
        _wps_key_derivation_function(kdk, NULL, 0, "Wi-Fi Easy and Secure Key Derivation", keys, sizeof(keys));

        PLATFORM_MEMCPY(authkey,    keys,                                        WPS_AUTHKEY_LEN);
        PLATFORM_MEMCPY(keywrapkey, keys + WPS_AUTHKEY_LEN,                      WPS_KEYWRAPKEY_LEN);
        PLATFORM_MEMCPY(emsk,       keys + WPS_AUTHKEY_LEN + WPS_KEYWRAPKEY_LEN, WPS_EMSK_LEN);
    }

    // With the just computed key, check the message authentication
    //
    {
        // Concatenate M1 and M2 (excluding the last 12 bytes, where the
        // authenticator attribute is contained) and calculate the HMAC, then
        // check it against the actual authenticator attribute value.
        //
        uint8_t   hash[SHA256_MAC_LEN];

        uint8_t  *addr[2];
        uint32_t  len[2];

        addr[0] = m1->wsc.datap;
        addr[1] = m2_wsc->wsc.datap;
        len[0]  = m1->wsc.len;
        len[1]  = m2_wsc->wsc.len-12;

        PLATFORM_HMAC_SHA256(authkey, WPS_AUTHKEY_LEN, 2, addr, len, hash);

        if (PLATFORM_MEMCMP(m2_authenticator, hash, 8) != 0)
        {
            DEBUG_WARNING("Message M2 authentication failed\n");
            goto fail;
        }
    }


    // With the just computed keys, decrypt the message and check the keywrap
    {
        uint8_t   *plain;
        uint16_t  plain_len;

        uint8_t m2_keywrap_present;

        plain     = m2_encrypted_settings + AES_BLOCK_SIZE;
        plain_len = m2_encrypted_settings_len - AES_BLOCK_SIZE;

        PLATFORM_AES_DECRYPT(keywrapkey, m2_encrypted_settings, plain, plain_len);

        // Remove padding
        //
        plain_len -= plain[plain_len-1];

        // Parse contents of decrypted settings
        //
        m2_keywrap_present        = 0;
        p                         = plain;
        while (p - plain < plain_len)
        {
            uint16_t attr_type;
            uint16_t attr_len;
            uint16_t value16;

            _E2B(&p, &attr_type);
            _E2B(&p, &attr_len);

            switch (attr_type) {
                case ATTR_SSID:
                    if ((attr_len>0) && (attr_len<=MAX_SSID_LEN)) {
                        bss->ssid.len = attr_len;
                        _EnB(&p, bss->ssid.ssid, attr_len);
                    } else
                        goto fail;
                    break;
                case ATTR_AUTH_TYPE:
                    if (attr_len==2) {
                        _E2B(&p, &value16);
                        bss->auth = value16;
                        bss->auth &= r->wsc->auth;
                    } else
                        goto fail;
                    break;
                case ATTR_ENCR_TYPE:
                    if (attr_len==2) {
                        _E2B(&p, &value16);
                        bss->encrypt = value16;
                        bss->encrypt &= r->wsc->encrypt;
                    } else
                        goto fail;
                    break;
                case ATTR_NETWORK_KEY:
                    if (attr_len<=MAX_KEY_LEN) {
                        bss->key.len = attr_len;
                        _EnB(&p, bss->key.key, attr_len);
                    } else
                        goto fail;
                    break;
                case ATTR_MAC_ADDR:
                    if (attr_len==MACLEN)
                        _EnB(&p, bss->bssid, MACLEN);
                    else
                        goto fail;
                    break;
                case ATTR_VENDOR_EXTENSION:
                    if ((attr_len>=9) && (p[0] == WPS_VENDOR_ID_WFA_1)
                        && (p[1] == WPS_VENDOR_ID_WFA_2) && (p[2] == WPS_VENDOR_ID_WFA_3)) {
                        uint8_t *p1 = p+3;
                        while (p1<(p+attr_len-2)) {
                            uint8_t ie_type, ie_len;
                            ie_type = *p1++;
                            ie_len = *p1++;
                            if ((ie_type==WFA_ELEM_MAP_EXT) && (ie_len==1)) {
                                if (p1[0] & MAP_TEAR_DOWN) {
                                    *teardown = 1;
                                    goto fail;
                                }
                                bss->backhaul = !!(p1[0] & MAP_BACKHAUL_BSS);
                                bss->fronthaul = !!(p1[0] & MAP_FRONTHAUL_BSS);
                            }
                            p1+=ie_len;
                        }
                    }
                    p+=attr_len;
                    break;
                case ATTR_KEY_WRAP_AUTH:
                    {
                        uint8_t *end_of_hmac;
                        uint8_t  hash[SHA256_MAC_LEN];

                        uint8_t  *addr[1];
                        uint32_t  len[1];

                        end_of_hmac = p - 4;

                        addr[0] = plain;
                        len[0]  = end_of_hmac-plain;

                        PLATFORM_HMAC_SHA256(authkey, WPS_AUTHKEY_LEN, 1, addr, len, hash);

                        if (PLATFORM_MEMCMP(p, hash, 8) != 0) {
                            DEBUG_WARNING("Message M2 keywrap failed\n");
                            goto fail;
                        }

                        p += attr_len;
                        m2_keywrap_present = 1;
                    }
                    break;
                default:
                    p += attr_len;
                    break;
            }
        }
        if ((!bss->ssid.len) || (!bss->auth) || (!bss->encrypt) || (!m2_keywrap_present)) {
            DEBUG_WARNING("Missing attributes in the configuration settings received in the M2 message\n");
            goto fail;
        }
    }

    return wcfg;
fail:
    free(wcfg);
    return NULL;
}

uint8_t wscGetType(uint8_t *m, uint16_t m_size)
{
    uint8_t type = 0;
    uint8_t *p;

    p = m;
    while (p - m < m_size) {
        uint16_t attr_type;
        uint16_t attr_len;

        _E2B(&p, &attr_type);
        _E2B(&p, &attr_len);
        if (ATTR_MSG_TYPE == attr_type)
        {
            if (1 != attr_len) {
                DEBUG_WARNING("Incorrect length (%d) for ATTR_MSG_TYPE\n", attr_len);
                return type;
            }
            _E1B(&p, &type);
            return type;;
        } else {
            p += attr_len;
        }
    }

    return type;
}

