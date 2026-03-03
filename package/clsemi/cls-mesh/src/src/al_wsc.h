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

#ifndef _AL_WSC_H_
#define _AL_WSC_H_

#include "platform.h"

// "defines" used to fill the M1 and M2 message fields
#define ATTR_VERSION            (0x104a)
#define ATTR_MSG_TYPE           (0x1022)
#define WPS_M1                  (0x04)
#define WPS_M2                  (0x05)
#define ATTR_UUID_E             (0x1047)
#define ATTR_UUID_R             (0x1048)
#define ATTR_MAC_ADDR           (0x1020)
#define ATTR_ENROLLEE_NONCE     (0x101a)
#define ATTR_REGISTRAR_NONCE    (0x1039)
#define ATTR_PUBLIC_KEY         (0x1032)
#define ATTR_AUTH_TYPE_FLAGS    (0x1004)
#define WPS_AUTH_OPEN           (0x0001)
#define WPS_AUTH_WPAPSK         (0x0002)
#define WPS_AUTH_SHARED         (0x0004) /* deprecated */
#define WPS_AUTH_WPA            (0x0008)
#define WPS_AUTH_WPA2           (0x0010)
#define WPS_AUTH_WPA2PSK        (0x0020)
#define ATTR_ENCR_TYPE_FLAGS    (0x1010)
#define WPS_ENCR_NONE           (0x0001)
#define WPS_ENCR_WEP            (0x0002) /* deprecated */
#define WPS_ENCR_TKIP           (0x0004)
#define WPS_ENCR_AES            (0x0008)
#define ATTR_CONN_TYPE_FLAGS    (0x100d)
#define WPS_CONN_ESS            (0x01)
#define WPS_CONN_IBSS           (0x02)
#define ATTR_CONFIG_METHODS     (0x1008)
#define WPS_CONFIG_VIRT_PUSHBUTTON (0x0280)
#define WPS_CONFIG_PHY_PUSHBUTTON  (0x0480)
#define ATTR_WPS_STATE          (0x1044)
#define WPS_STATE_NOT_CONFIGURED (1)
#define WPS_STATE_CONFIGURED     (2)
#define ATTR_MANUFACTURER       (0x1021)
#define ATTR_MODEL_NAME         (0x1023)
#define ATTR_MODEL_NUMBER       (0x1024)
#define ATTR_SERIAL_NUMBER      (0x1042)
#define ATTR_PRIMARY_DEV_TYPE   (0x1054)
#define WPS_DEV_COMPUTER                          (1)
#define WPS_DEV_COMPUTER_PC                       (1)                                                                          
#define WPS_DEV_COMPUTER_SERVER                   (2)                                                                              
#define WPS_DEV_COMPUTER_MEDIA_CENTER             (3)                                                                                    
#define WPS_DEV_COMPUTER_ULTRA_MOBILE             (4)                                                                                    
#define WPS_DEV_COMPUTER_NOTEBOOK                 (5)                                                                                
#define WPS_DEV_COMPUTER_DESKTOP                  (6)                                                                               
#define WPS_DEV_COMPUTER_MID                      (7)                                                                           
#define WPS_DEV_COMPUTER_NETBOOK                  (8)                                                                               
#define WPS_DEV_COMPUTER_TABLET                   (9)                                                                              
#define WPS_DEV_INPUT                             (2)                      
#define WPS_DEV_INPUT_KEYBOARD                    (1)                                                                             
#define WPS_DEV_INPUT_MOUSE                       (2)                                                                          
#define WPS_DEV_INPUT_JOYSTICK                    (3)                                                                             
#define WPS_DEV_INPUT_TRACKBALL                   (4)                                                                              
#define WPS_DEV_INPUT_GAMING                      (5)                                                                           
#define WPS_DEV_INPUT_REMOTE                      (6)                                                                           
#define WPS_DEV_INPUT_TOUCHSCREEN                 (7)                                                                                
#define WPS_DEV_INPUT_BIOMETRIC_READER            (8)                                                                                     
#define WPS_DEV_INPUT_BARCODE_READER              (9)                                                                                   
#define WPS_DEV_PRINTER                           (3)
#define WPS_DEV_PRINTER_PRINTER                   (1)                                                                              
#define WPS_DEV_PRINTER_SCANNER                   (2)                                                                              
#define WPS_DEV_PRINTER_FAX                       (3)                                                                          
#define WPS_DEV_PRINTER_COPIER                    (4)                                                                             
#define WPS_DEV_PRINTER_ALL_IN_ONE                (5)                                                                                 
#define WPS_DEV_CAMERA                            (4)
#define WPS_DEV_CAMERA_DIGITAL_STILL_CAMERA       (1)                                                                                          
#define WPS_DEV_CAMERA_VIDEO                      (2)                                                                           
#define WPS_DEV_CAMERA_WEB                        (3)                                                                         
#define WPS_DEV_CAMERA_SECURITY                   (4)                                                                              
#define WPS_DEV_STORAGE                           (5)
#define WPS_DEV_STORAGE_NAS                       (1)                                                                          
#define WPS_DEV_NETWORK_INFRA                     (6)
#define WPS_DEV_NETWORK_INFRA_AP                  (1)                                                                               
#define WPS_DEV_NETWORK_INFRA_ROUTER              (2)                                                                                   
#define WPS_DEV_NETWORK_INFRA_SWITCH              (3)                                                                                   
#define WPS_DEV_NETWORK_INFRA_GATEWAY             (4)                                                                                    
#define WPS_DEV_NETWORK_INFRA_BRIDGE              (5)                                                                                   
#define WPS_DEV_DISPLAY                           (7)
#define WPS_DEV_DISPLAY_TV                        (1)                                                                         
#define WPS_DEV_DISPLAY_PICTURE_FRAME             (2)                                                                                    
#define WPS_DEV_DISPLAY_PROJECTOR                 (3)                                                                                
#define WPS_DEV_DISPLAY_MONITOR                   (4)                                                                              
#define WPS_DEV_MULTIMEDIA                        (8)
#define WPS_DEV_MULTIMEDIA_DAR                    (1)                                                                             
#define WPS_DEV_MULTIMEDIA_PVR                    (2)                                                                             
#define WPS_DEV_MULTIMEDIA_MCX                    (3)                                                                             
#define WPS_DEV_MULTIMEDIA_SET_TOP_BOX            (4)                                                                                     
#define WPS_DEV_MULTIMEDIA_MEDIA_SERVER           (5)                                                                                      
#define WPS_DEV_MULTIMEDIA_PORTABLE_VIDEO_PLAYER  (6)                                                                                               
#define WPS_DEV_GAMING                             (9)
#define WPS_DEV_GAMING_XBOX                       (1)                                                                          
#define WPS_DEV_GAMING_XBOX360                    (2)                                                                             
#define WPS_DEV_GAMING_PLAYSTATION                (3)                                                                                 
#define WPS_DEV_GAMING_GAME_CONSOLE               (4)                                                                                  
#define WPS_DEV_GAMING_PORTABLE_DEVICE            (5)                                                                                     
#define WPS_DEV_PHONE                             (10)
#define WPS_DEV_PHONE_WINDOWS_MOBILE              (1)                                                                                   
#define WPS_DEV_PHONE_SINGLE_MODE                 (2)                                                                                
#define WPS_DEV_PHONE_DUAL_MODE                   (3)                                                                              
#define WPS_DEV_PHONE_SP_SINGLE_MODE              (4)                                                                                   
#define WPS_DEV_PHONE_SP_DUAL_MODE                (5)                                                                                 
#define WPS_DEV_AUDIO                             (11)
#define WPS_DEV_AUDIO_TUNER_RECV                  (1)                                                                               
#define WPS_DEV_AUDIO_SPEAKERS                    (2)                                                                             
#define WPS_DEV_AUDIO_PMP                         (3)                                                                        
#define WPS_DEV_AUDIO_HEADSET                     (4)                                                                            
#define WPS_DEV_AUDIO_HEADPHONES                  (5)                                                                               
#define WPS_DEV_AUDIO_MICROPHONE                  (6)                                                                               
#define WPS_DEV_AUDIO_HOME_THEATRE                (7)                                                                                 
#define ATTR_DEV_NAME          (0x1011)
#define ATTR_RF_BANDS          (0x103c)
#define WPS_RF_2GHZ            (0x01)
#define WPS_RF_5GHZ            (0x02)
#define WPS_RF_60GHZ           (0x04)
#define ATTR_ASSOC_STATE       (0x1002)
#define WPS_ASSOC_NOT_ASSOC    (0)
#define WPS_ASSOC_CONN_SUCCESS (1)
#define ATTR_DEV_PASSWORD_ID   (0x1012)
#define DEV_PW_PUSHBUTTON      (0x0004)
#define ATTR_CONFIG_ERROR      (0x1009)
#define WPS_CFG_NO_ERROR       (0)
#define ATTR_OS_VERSION        (0x102d)
#define ATTR_VENDOR_EXTENSION  (0x1049)
#define WPS_VENDOR_ID_WFA_1    (0x00)
#define WPS_VENDOR_ID_WFA_2    (0x37)
#define WPS_VENDOR_ID_WFA_3    (0x2A)
#define WFA_ELEM_VERSION2      (0x00)
#define WPS_VERSION            (0x20)
#define ATTR_SSID              (0x1045)
#define ATTR_AUTH_TYPE         (0x1003)
#define ATTR_ENCR_TYPE         (0x100f)
#define ATTR_NETWORK_KEY       (0x1027)
#define ATTR_KEY_WRAP_AUTH     (0x101e)
#define ATTR_ENCR_SETTINGS     (0x1018)
#define ATTR_AUTHENTICATOR     (0x1005)

#define WFA_ELEM_MAP_EXT       (0x06)
#define MAP_TEAR_DOWN          (0x10)
#define MAP_FRONTHAUL_BSS      (0x20)
#define MAP_BACKHAUL_BSS       (0x40)
#define MAP_BACKHAUL_STA       (0x80)


// Keys sizes
#define WPS_AUTHKEY_LEN    32
#define WPS_KEYWRAPKEY_LEN 16
#define WPS_EMSK_LEN       32

struct wsc_m1_info {
    uint8_t *m1;
    uint8_t *nonce;
    uint8_t *al_mac;
    uint8_t *pub;
    uint16_t m1_len;
    uint16_t pub_len;
    uint16_t auth;
    uint16_t encrypt;
    uint16_t bands;
};

struct TLV *wscBuildM1(struct radio *r);
uint8_t wscProcessM1(struct TLV *m1, struct wsc_m1_info *info);
struct wifi_config *wscProcessM2(struct TLV *m2, struct radio *r, uint8_t *teardown);
struct TLV *wscBuildM2(struct wifi_config *wcfg, struct wsc_m1_info *m1_info);
uint8_t wscGetType(uint8_t *m, uint16_t m_size);


#endif
