#ifndef _DUBHE1000_XPCS_H_
#define _DUBHE1000_XPCS_H_
//000: 10G-SXGMII
//001: 5G-SXGMII
//010: 2.5G-SXGMII
//011: 10G-DXGMII
//100: 5G-DXGMII
//101: 10G-QXGMII
//110/111: Reserved
enum {
	USXG_MODE_10G_SXGMII	= 0,
	USXG_MODE_5G_SXGMII	= 1,
	USXG_MODE_2_5G_SXGMII	= 2,
	USXG_MODE_10G_DXGMII	= 3,
	USXG_MODE_5G_DXGMII	= 4,
	USXG_MODE_10G_QXGMII	= 5,
	USXG_MODE_MAX_NUM	= 6
};

int dubhe1000_set_pcs_mode(struct dubhe1000_pcs *xpcs_priv);
void dubhe1000_xpcs_get_link_state(struct dubhe1000_pcs *xpcs_priv, int port, struct phylink_link_state *state);
void dubhe1000_xpcs_an_restart(struct dubhe1000_pcs *xpcs_priv, int port);
void dubhe1000_serdes_load_and_modify(struct dubhe1000_pcs *xpcs_priv);
void dubhe1000_xpcs_speed(struct dubhe1000_pcs *xpcs_priv, int port, unsigned int speed);
void dubhe1000_usxgmii_reset_rate_adapter(struct dubhe1000_pcs *xpcs_priv, int port);
void dubhe1000_xpcs_clear_an_interrupt(struct dubhe1000_pcs *xpcs_priv, int port);
int dubhe1000_xpcs_enable(struct dubhe1000_pcs *xpcs_priv, int port, int enable);
uint32_t dubhe1000_xpcs_get_sts2(struct dubhe1000_pcs *xpcs_priv);
#endif /* _DUBHE1000_XPCS_H_ */
