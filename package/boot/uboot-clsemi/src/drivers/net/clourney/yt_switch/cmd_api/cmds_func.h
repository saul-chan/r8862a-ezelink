static inline int cmd_yt_vlan_init(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;

    if(g_debug_switch)
        printf("is yt_vlan_init command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
    }

    /* execution func */
    ret = yt_vlan_init(unit);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_vlan_port_set(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_vlan_t vid;
    yt_port_mask_t member_portmask;
    yt_port_mask_t untag_portmask;

    if(g_debug_switch)
        printf("is yt_vlan_port_set command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&vid, 0, sizeof(yt_vlan_t));
    memset(&member_portmask, 0, sizeof(yt_port_mask_t));
    memset(&untag_portmask, 0, sizeof(yt_port_mask_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_VLAN_T, vid);
    CONVERT_PARAMFROMSTR(T_YT_PORT_MASK_T, member_portmask);
    CONVERT_PARAMFROMSTR(T_YT_PORT_MASK_T, untag_portmask);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_VLAN_T, vid);
        CONVERT_PARAMTOSTR(T_YT_PORT_MASK_T, member_portmask);
        CONVERT_PARAMTOSTR(T_YT_PORT_MASK_T, untag_portmask);
    }

    /* execution func */
    ret = yt_vlan_port_set(unit, vid, member_portmask, untag_portmask);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_vlan_port_get(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_vlan_t vid;
    yt_port_mask_t pMember_portmask;
    yt_port_mask_t pUntag_portmask;

    if(g_debug_switch)
        printf("is yt_vlan_port_get command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&vid, 0, sizeof(yt_vlan_t));
    memset(&pMember_portmask, 0, sizeof(yt_port_mask_t));
    memset(&pUntag_portmask, 0, sizeof(yt_port_mask_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_VLAN_T, vid);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_VLAN_T, vid);
    }

    /* execution func */
    ret = yt_vlan_port_get(unit, vid, &pMember_portmask, &pUntag_portmask);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);/* print out parameters */
    if (!ret) {
        CONVERT_PARAMTOSTR(T_YT_PORT_MASK_T, pMember_portmask);
        CONVERT_PARAMTOSTR(T_YT_PORT_MASK_T, pUntag_portmask);
    }

    return 0;
}
static inline int cmd_yt_vlan_svlMode_enable_set(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_vlan_t vid;
    yt_enable_t enable;

    if(g_debug_switch)
        printf("is yt_vlan_svlMode_enable_set command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&vid, 0, sizeof(yt_vlan_t));
    memset(&enable, 0, sizeof(yt_enable_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_VLAN_T, vid);
    CONVERT_PARAMFROMSTR(T_YT_ENABLE_T, enable);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_VLAN_T, vid);
        CONVERT_PARAMTOSTR(T_YT_ENABLE_T, enable);
    }

    /* execution func */
    ret = yt_vlan_svlMode_enable_set(unit, vid, enable);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_vlan_svlMode_enable_get(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_vlan_t vid;
    yt_enable_t pEnable;

    if(g_debug_switch)
        printf("is yt_vlan_svlMode_enable_get command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&vid, 0, sizeof(yt_vlan_t));
    memset(&pEnable, 0, sizeof(yt_enable_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_VLAN_T, vid);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_VLAN_T, vid);
    }

    /* execution func */
    ret = yt_vlan_svlMode_enable_get(unit, vid, &pEnable);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);/* print out parameters */
    if (!ret) {
        CONVERT_PARAMTOSTR(T_YT_ENABLE_T, pEnable);
    }

    return 0;
}
static inline int cmd_yt_vlan_fid_set(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_vlan_t vid;
    yt_fid_t fid;

    if(g_debug_switch)
        printf("is yt_vlan_fid_set command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&vid, 0, sizeof(yt_vlan_t));
    memset(&fid, 0, sizeof(yt_fid_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_VLAN_T, vid);
    CONVERT_PARAMFROMSTR(T_YT_FID_T, fid);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_VLAN_T, vid);
        CONVERT_PARAMTOSTR(T_YT_FID_T, fid);
    }

    /* execution func */
    ret = yt_vlan_fid_set(unit, vid, fid);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_vlan_fid_get(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_vlan_t vid;
    yt_fid_t pFid;

    if(g_debug_switch)
        printf("is yt_vlan_fid_get command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&vid, 0, sizeof(yt_vlan_t));
    memset(&pFid, 0, sizeof(yt_fid_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_VLAN_T, vid);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_VLAN_T, vid);
    }

    /* execution func */
    ret = yt_vlan_fid_get(unit, vid, &pFid);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);/* print out parameters */
    if (!ret) {
        CONVERT_PARAMTOSTR(T_YT_FID_T, pFid);
    }

    return 0;
}
static inline int cmd_yt_vlan_igrTpid_set(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_tpid_profiles_t tpid;

    if(g_debug_switch)
        printf("is yt_vlan_igrTpid_set command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&tpid, 0, sizeof(yt_tpid_profiles_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_TPID_PROFILES_T, tpid);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_TPID_PROFILES_T, tpid);
    }

    /* execution func */
    ret = yt_vlan_igrTpid_set(unit, tpid);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_vlan_igrTpid_get(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_tpid_profiles_t pTpid;

    if(g_debug_switch)
        printf("is yt_vlan_igrTpid_get command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&pTpid, 0, sizeof(yt_tpid_profiles_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
    }

    /* execution func */
    ret = yt_vlan_igrTpid_get(unit, &pTpid);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);/* print out parameters */
    if (!ret) {
        CONVERT_PARAMTOSTR(T_YT_TPID_PROFILES_T, pTpid);
    }

    return 0;
}
static inline int cmd_yt_vlan_port_igrTpidSel_set(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_vlan_type_t type;
    yt_port_t port;
    yt_tpidprofile_id_mask_t tpidIdxMask;

    if(g_debug_switch)
        printf("is yt_vlan_port_igrTpidSel_set command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&type, 0, sizeof(yt_vlan_type_t));
    memset(&port, 0, sizeof(yt_port_t));
    memset(&tpidIdxMask, 0, sizeof(yt_tpidprofile_id_mask_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_VLAN_TYPE_T, type);
    CONVERT_PARAMFROMSTR(T_YT_PORT_T, port);
    CONVERT_PARAMFROMSTR(T_YT_TPIDPROFILE_ID_MASK_T, tpidIdxMask);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_VLAN_TYPE_T, type);
        CONVERT_PARAMTOSTR(T_YT_PORT_T, port);
        CONVERT_PARAMTOSTR(T_YT_TPIDPROFILE_ID_MASK_T, tpidIdxMask);
    }

    /* execution func */
    ret = yt_vlan_port_igrTpidSel_set(unit, type, port, tpidIdxMask);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_vlan_port_igrTpidSel_get(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_vlan_type_t type;
    yt_port_t port;
    yt_tpidprofile_id_mask_t pTpidIdxMask;

    if(g_debug_switch)
        printf("is yt_vlan_port_igrTpidSel_get command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&type, 0, sizeof(yt_vlan_type_t));
    memset(&port, 0, sizeof(yt_port_t));
    memset(&pTpidIdxMask, 0, sizeof(yt_tpidprofile_id_mask_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_VLAN_TYPE_T, type);
    CONVERT_PARAMFROMSTR(T_YT_PORT_T, port);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_VLAN_TYPE_T, type);
        CONVERT_PARAMTOSTR(T_YT_PORT_T, port);
    }

    /* execution func */
    ret = yt_vlan_port_igrTpidSel_get(unit, type, port, &pTpidIdxMask);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);/* print out parameters */
    if (!ret) {
        CONVERT_PARAMTOSTR(T_YT_TPIDPROFILE_ID_MASK_T, pTpidIdxMask);
    }

    return 0;
}
static inline int cmd_yt_vlan_port_igrPvid_set(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_vlan_type_t type;
    yt_port_t port;
    yt_vlan_t vid;

    if(g_debug_switch)
        printf("is yt_vlan_port_igrPvid_set command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&type, 0, sizeof(yt_vlan_type_t));
    memset(&port, 0, sizeof(yt_port_t));
    memset(&vid, 0, sizeof(yt_vlan_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_VLAN_TYPE_T, type);
    CONVERT_PARAMFROMSTR(T_YT_PORT_T, port);
    CONVERT_PARAMFROMSTR(T_YT_VLAN_T, vid);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_VLAN_TYPE_T, type);
        CONVERT_PARAMTOSTR(T_YT_PORT_T, port);
        CONVERT_PARAMTOSTR(T_YT_VLAN_T, vid);
    }

    /* execution func */
    ret = yt_vlan_port_igrPvid_set(unit, type, port, vid);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_vlan_port_igrPvid_get(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_vlan_type_t type;
    yt_port_t port;
    yt_vlan_t pVid;

    if(g_debug_switch)
        printf("is yt_vlan_port_igrPvid_get command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&type, 0, sizeof(yt_vlan_type_t));
    memset(&port, 0, sizeof(yt_port_t));
    memset(&pVid, 0, sizeof(yt_vlan_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_VLAN_TYPE_T, type);
    CONVERT_PARAMFROMSTR(T_YT_PORT_T, port);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_VLAN_TYPE_T, type);
        CONVERT_PARAMTOSTR(T_YT_PORT_T, port);
    }

    /* execution func */
    ret = yt_vlan_port_igrPvid_get(unit, type, port, &pVid);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);/* print out parameters */
    if (!ret) {
        CONVERT_PARAMTOSTR(T_YT_VLAN_T, pVid);
    }

    return 0;
}
static inline int cmd_yt_vlan_port_igrFilter_enable_set(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_port_t port;
    yt_enable_t enable;

    if(g_debug_switch)
        printf("is yt_vlan_port_igrFilter_enable_set command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&port, 0, sizeof(yt_port_t));
    memset(&enable, 0, sizeof(yt_enable_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_PORT_T, port);
    CONVERT_PARAMFROMSTR(T_YT_ENABLE_T, enable);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_PORT_T, port);
        CONVERT_PARAMTOSTR(T_YT_ENABLE_T, enable);
    }

    /* execution func */
    ret = yt_vlan_port_igrFilter_enable_set(unit, port, enable);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_vlan_port_igrFilter_enable_get(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_port_t port;
    yt_enable_t pEnable;

    if(g_debug_switch)
        printf("is yt_vlan_port_igrFilter_enable_get command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&port, 0, sizeof(yt_port_t));
    memset(&pEnable, 0, sizeof(yt_enable_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_PORT_T, port);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_PORT_T, port);
    }

    /* execution func */
    ret = yt_vlan_port_igrFilter_enable_get(unit, port, &pEnable);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);/* print out parameters */
    if (!ret) {
        CONVERT_PARAMTOSTR(T_YT_ENABLE_T, pEnable);
    }

    return 0;
}
static inline int cmd_yt_vlan_igrTransparent_set(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_port_t port;
    yt_port_mask_t port_mask;

    if(g_debug_switch)
        printf("is yt_vlan_igrTransparent_set command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&port, 0, sizeof(yt_port_t));
    memset(&port_mask, 0, sizeof(yt_port_mask_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_PORT_T, port);
    CONVERT_PARAMFROMSTR(T_YT_PORT_MASK_T, port_mask);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_PORT_T, port);
        CONVERT_PARAMTOSTR(T_YT_PORT_MASK_T, port_mask);
    }

    /* execution func */
    ret = yt_vlan_igrTransparent_set(unit, port, port_mask);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_vlan_igrTransparent_get(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_port_t port;
    yt_port_mask_t pPort_mask;

    if(g_debug_switch)
        printf("is yt_vlan_igrTransparent_get command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&port, 0, sizeof(yt_port_t));
    memset(&pPort_mask, 0, sizeof(yt_port_mask_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_PORT_T, port);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_PORT_T, port);
    }

    /* execution func */
    ret = yt_vlan_igrTransparent_get(unit, port, &pPort_mask);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);/* print out parameters */
    if (!ret) {
        CONVERT_PARAMTOSTR(T_YT_PORT_MASK_T, pPort_mask);
    }

    return 0;
}
static inline int cmd_yt_vlan_port_aft_set(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_vlan_type_t type;
    yt_port_t port;
    yt_vlan_aft_t aft;

    if(g_debug_switch)
        printf("is yt_vlan_port_aft_set command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&type, 0, sizeof(yt_vlan_type_t));
    memset(&port, 0, sizeof(yt_port_t));
    memset(&aft, 0, sizeof(yt_vlan_aft_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_VLAN_TYPE_T, type);
    CONVERT_PARAMFROMSTR(T_YT_PORT_T, port);
    CONVERT_PARAMFROMSTR(T_YT_VLAN_AFT_T, aft);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_VLAN_TYPE_T, type);
        CONVERT_PARAMTOSTR(T_YT_PORT_T, port);
        CONVERT_PARAMTOSTR(T_YT_VLAN_AFT_T, aft);
    }

    /* execution func */
    ret = yt_vlan_port_aft_set(unit, type, port, aft);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_vlan_port_aft_get(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_vlan_type_t type;
    yt_port_t port;
    yt_vlan_aft_t pAft;

    if(g_debug_switch)
        printf("is yt_vlan_port_aft_get command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&type, 0, sizeof(yt_vlan_type_t));
    memset(&port, 0, sizeof(yt_port_t));
    memset(&pAft, 0, sizeof(yt_vlan_aft_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_VLAN_TYPE_T, type);
    CONVERT_PARAMFROMSTR(T_YT_PORT_T, port);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_VLAN_TYPE_T, type);
        CONVERT_PARAMTOSTR(T_YT_PORT_T, port);
    }

    /* execution func */
    ret = yt_vlan_port_aft_get(unit, type, port, &pAft);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);/* print out parameters */
    if (!ret) {
        CONVERT_PARAMTOSTR(T_YT_VLAN_AFT_T, pAft);
    }

    return 0;
}
static inline int cmd_yt_vlan_port_egrTagMode_set(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_vlan_type_t type;
    yt_port_t port;
    yt_egr_tag_mode_t tagMode;

    if(g_debug_switch)
        printf("is yt_vlan_port_egrTagMode_set command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&type, 0, sizeof(yt_vlan_type_t));
    memset(&port, 0, sizeof(yt_port_t));
    memset(&tagMode, 0, sizeof(yt_egr_tag_mode_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_VLAN_TYPE_T, type);
    CONVERT_PARAMFROMSTR(T_YT_PORT_T, port);
    CONVERT_PARAMFROMSTR(T_YT_EGR_TAG_MODE_T, tagMode);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_VLAN_TYPE_T, type);
        CONVERT_PARAMTOSTR(T_YT_PORT_T, port);
        CONVERT_PARAMTOSTR(T_YT_EGR_TAG_MODE_T, tagMode);
    }

    /* execution func */
    ret = yt_vlan_port_egrTagMode_set(unit, type, port, tagMode);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_vlan_port_egrTagMode_get(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_vlan_type_t type;
    yt_port_t port;
    yt_egr_tag_mode_t pTagMode;

    if(g_debug_switch)
        printf("is yt_vlan_port_egrTagMode_get command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&type, 0, sizeof(yt_vlan_type_t));
    memset(&port, 0, sizeof(yt_port_t));
    memset(&pTagMode, 0, sizeof(yt_egr_tag_mode_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_VLAN_TYPE_T, type);
    CONVERT_PARAMFROMSTR(T_YT_PORT_T, port);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_VLAN_TYPE_T, type);
        CONVERT_PARAMTOSTR(T_YT_PORT_T, port);
    }

    /* execution func */
    ret = yt_vlan_port_egrTagMode_get(unit, type, port, &pTagMode);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);/* print out parameters */
    if (!ret) {
        CONVERT_PARAMTOSTR(T_YT_EGR_TAG_MODE_T, pTagMode);
    }

    return 0;
}
static inline int cmd_yt_vlan_port_egrDefVid_set(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_vlan_type_t type;
    yt_port_t port;
    yt_vlan_t default_vid;

    if(g_debug_switch)
        printf("is yt_vlan_port_egrDefVid_set command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&type, 0, sizeof(yt_vlan_type_t));
    memset(&port, 0, sizeof(yt_port_t));
    memset(&default_vid, 0, sizeof(yt_vlan_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_VLAN_TYPE_T, type);
    CONVERT_PARAMFROMSTR(T_YT_PORT_T, port);
    CONVERT_PARAMFROMSTR(T_YT_VLAN_T, default_vid);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_VLAN_TYPE_T, type);
        CONVERT_PARAMTOSTR(T_YT_PORT_T, port);
        CONVERT_PARAMTOSTR(T_YT_VLAN_T, default_vid);
    }

    /* execution func */
    ret = yt_vlan_port_egrDefVid_set(unit, type, port, default_vid);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_vlan_port_egrDefVid_get(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_vlan_type_t type;
    yt_port_t port;
    yt_vlan_t pDefault_vid;

    if(g_debug_switch)
        printf("is yt_vlan_port_egrDefVid_get command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&type, 0, sizeof(yt_vlan_type_t));
    memset(&port, 0, sizeof(yt_port_t));
    memset(&pDefault_vid, 0, sizeof(yt_vlan_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_VLAN_TYPE_T, type);
    CONVERT_PARAMFROMSTR(T_YT_PORT_T, port);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_VLAN_TYPE_T, type);
        CONVERT_PARAMTOSTR(T_YT_PORT_T, port);
    }

    /* execution func */
    ret = yt_vlan_port_egrDefVid_get(unit, type, port, &pDefault_vid);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);/* print out parameters */
    if (!ret) {
        CONVERT_PARAMTOSTR(T_YT_VLAN_T, pDefault_vid);
    }

    return 0;
}
static inline int cmd_yt_vlan_egrTpid_set(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_tpid_profiles_t tpids;

    if(g_debug_switch)
        printf("is yt_vlan_egrTpid_set command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&tpids, 0, sizeof(yt_tpid_profiles_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_TPID_PROFILES_T, tpids);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_TPID_PROFILES_T, tpids);
    }

    /* execution func */
    ret = yt_vlan_egrTpid_set(unit, tpids);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_vlan_egrTpid_get(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_tpid_profiles_t pTpids;

    if(g_debug_switch)
        printf("is yt_vlan_egrTpid_get command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&pTpids, 0, sizeof(yt_tpid_profiles_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
    }

    /* execution func */
    ret = yt_vlan_egrTpid_get(unit, &pTpids);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);/* print out parameters */
    if (!ret) {
        CONVERT_PARAMTOSTR(T_YT_TPID_PROFILES_T, pTpids);
    }

    return 0;
}
static inline int cmd_yt_vlan_port_egrTpidSel_set(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_vlan_type_t type;
    yt_port_t port;
    uint8_t tpidIdx;

    if(g_debug_switch)
        printf("is yt_vlan_port_egrTpidSel_set command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&type, 0, sizeof(yt_vlan_type_t));
    memset(&port, 0, sizeof(yt_port_t));
    memset(&tpidIdx, 0, sizeof(uint8_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_VLAN_TYPE_T, type);
    CONVERT_PARAMFROMSTR(T_YT_PORT_T, port);
    CONVERT_PARAMFROMSTR(T_UINT8_T, tpidIdx);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_VLAN_TYPE_T, type);
        CONVERT_PARAMTOSTR(T_YT_PORT_T, port);
        CONVERT_PARAMTOSTR(T_UINT8_T, tpidIdx);
    }

    /* execution func */
    ret = yt_vlan_port_egrTpidSel_set(unit, type, port, tpidIdx);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_vlan_port_egrTpidSel_get(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_vlan_type_t type;
    yt_port_t port;
    uint8_t pTpidIdx;

    if(g_debug_switch)
        printf("is yt_vlan_port_egrTpidSel_get command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&type, 0, sizeof(yt_vlan_type_t));
    memset(&port, 0, sizeof(yt_port_t));
    memset(&pTpidIdx, 0, sizeof(uint8_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_VLAN_TYPE_T, type);
    CONVERT_PARAMFROMSTR(T_YT_PORT_T, port);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_VLAN_TYPE_T, type);
        CONVERT_PARAMTOSTR(T_YT_PORT_T, port);
    }

    /* execution func */
    ret = yt_vlan_port_egrTpidSel_get(unit, type, port, &pTpidIdx);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);/* print out parameters */
    if (!ret) {
        CONVERT_PARAMTOSTR(T_UINT8_T, pTpidIdx);
    }

    return 0;
}
static inline int cmd_yt_vlan_port_egrTransparent_set(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_vlan_type_t type;
    yt_port_t port;
    yt_enable_t enable;
    yt_port_mask_t port_mask;

    if(g_debug_switch)
        printf("is yt_vlan_port_egrTransparent_set command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&type, 0, sizeof(yt_vlan_type_t));
    memset(&port, 0, sizeof(yt_port_t));
    memset(&enable, 0, sizeof(yt_enable_t));
    memset(&port_mask, 0, sizeof(yt_port_mask_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_VLAN_TYPE_T, type);
    CONVERT_PARAMFROMSTR(T_YT_PORT_T, port);
    CONVERT_PARAMFROMSTR(T_YT_ENABLE_T, enable);
    CONVERT_PARAMFROMSTR(T_YT_PORT_MASK_T, port_mask);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_VLAN_TYPE_T, type);
        CONVERT_PARAMTOSTR(T_YT_PORT_T, port);
        CONVERT_PARAMTOSTR(T_YT_ENABLE_T, enable);
        CONVERT_PARAMTOSTR(T_YT_PORT_MASK_T, port_mask);
    }

    /* execution func */
    ret = yt_vlan_port_egrTransparent_set(unit, type, port, enable, port_mask);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_vlan_port_egrTransparent_get(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_vlan_type_t type;
    yt_port_t port;
    yt_enable_t pEnable;
    yt_port_mask_t pPort_mask;

    if(g_debug_switch)
        printf("is yt_vlan_port_egrTransparent_get command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&type, 0, sizeof(yt_vlan_type_t));
    memset(&port, 0, sizeof(yt_port_t));
    memset(&pEnable, 0, sizeof(yt_enable_t));
    memset(&pPort_mask, 0, sizeof(yt_port_mask_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_VLAN_TYPE_T, type);
    CONVERT_PARAMFROMSTR(T_YT_PORT_T, port);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_VLAN_TYPE_T, type);
        CONVERT_PARAMTOSTR(T_YT_PORT_T, port);
    }

    /* execution func */
    ret = yt_vlan_port_egrTransparent_get(unit, type, port, &pEnable, &pPort_mask);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);/* print out parameters */
    if (!ret) {
        CONVERT_PARAMTOSTR(T_YT_ENABLE_T, pEnable);
        CONVERT_PARAMTOSTR(T_YT_PORT_MASK_T, pPort_mask);
    }

    return 0;
}
static inline int cmd_yt_vlan_port_egrFilter_enable_set(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_port_t port;
    yt_enable_t enable;

    if(g_debug_switch)
        printf("is yt_vlan_port_egrFilter_enable_set command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&port, 0, sizeof(yt_port_t));
    memset(&enable, 0, sizeof(yt_enable_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_PORT_T, port);
    CONVERT_PARAMFROMSTR(T_YT_ENABLE_T, enable);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_PORT_T, port);
        CONVERT_PARAMTOSTR(T_YT_ENABLE_T, enable);
    }

    /* execution func */
    ret = yt_vlan_port_egrFilter_enable_set(unit, port, enable);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_vlan_port_egrFilter_enable_get(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_port_t port;
    yt_enable_t pEnable;

    if(g_debug_switch)
        printf("is yt_vlan_port_egrFilter_enable_get command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&port, 0, sizeof(yt_port_t));
    memset(&pEnable, 0, sizeof(yt_enable_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_PORT_T, port);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_PORT_T, port);
    }

    /* execution func */
    ret = yt_vlan_port_egrFilter_enable_get(unit, port, &pEnable);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);/* print out parameters */
    if (!ret) {
        CONVERT_PARAMTOSTR(T_YT_ENABLE_T, pEnable);
    }

    return 0;
}
static inline int cmd_yt_vlan_port_vidTypeSel_set(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_port_t port;
    yt_vlan_type_t type;

    if(g_debug_switch)
        printf("is yt_vlan_port_vidTypeSel_set command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&port, 0, sizeof(yt_port_t));
    memset(&type, 0, sizeof(yt_vlan_type_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_PORT_T, port);
    CONVERT_PARAMFROMSTR(T_YT_VLAN_TYPE_T, type);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_PORT_T, port);
        CONVERT_PARAMTOSTR(T_YT_VLAN_TYPE_T, type);
    }

    /* execution func */
    ret = yt_vlan_port_vidTypeSel_set(unit, port, type);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_vlan_port_vidTypeSel_get(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_port_t port;
    yt_vlan_type_t pType;

    if(g_debug_switch)
        printf("is yt_vlan_port_vidTypeSel_get command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&port, 0, sizeof(yt_port_t));
    memset(&pType, 0, sizeof(yt_vlan_type_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_PORT_T, port);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_PORT_T, port);
    }

    /* execution func */
    ret = yt_vlan_port_vidTypeSel_get(unit, port, &pType);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);/* print out parameters */
    if (!ret) {
        CONVERT_PARAMTOSTR(T_YT_VLAN_TYPE_T, pType);
    }

    return 0;
}
static inline int cmd_yt_vlan_protocol_based_vlan_group_set(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    uint8_t group_id;
    yt_vlan_protocol_key_t key;

    if(g_debug_switch)
        printf("is yt_vlan_protocol_based_vlan_group_set command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&group_id, 0, sizeof(uint8_t));
    memset(&key, 0, sizeof(yt_vlan_protocol_key_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_UINT8_T, group_id);
    CONVERT_PARAMFROMSTR(T_YT_VLAN_PROTOCOL_KEY_T, key);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_UINT8_T, group_id);
        CONVERT_PARAMTOSTR(T_YT_VLAN_PROTOCOL_KEY_T, key);
    }

    /* execution func */
    ret = yt_vlan_protocol_based_vlan_group_set(unit, group_id, key);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_vlan_protocol_based_vlan_group_get(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    uint8_t group_id;
    yt_vlan_protocol_key_t pKey;

    if(g_debug_switch)
        printf("is yt_vlan_protocol_based_vlan_group_get command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&group_id, 0, sizeof(uint8_t));
    memset(&pKey, 0, sizeof(yt_vlan_protocol_key_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_UINT8_T, group_id);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_UINT8_T, group_id);
    }

    /* execution func */
    ret = yt_vlan_protocol_based_vlan_group_get(unit, group_id, &pKey);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);/* print out parameters */
    if (!ret) {
        CONVERT_PARAMTOSTR(T_YT_VLAN_PROTOCOL_KEY_T, pKey);
    }

    return 0;
}
static inline int cmd_yt_vlan_protocol_based_vlan_table_set(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_port_t port;
    uint8_t group_id;
    yt_vlan_protocol_action_t action;

    if(g_debug_switch)
        printf("is yt_vlan_protocol_based_vlan_table_set command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&port, 0, sizeof(yt_port_t));
    memset(&group_id, 0, sizeof(uint8_t));
    memset(&action, 0, sizeof(yt_vlan_protocol_action_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_PORT_T, port);
    CONVERT_PARAMFROMSTR(T_UINT8_T, group_id);
    CONVERT_PARAMFROMSTR(T_YT_VLAN_PROTOCOL_ACTION_T, action);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_PORT_T, port);
        CONVERT_PARAMTOSTR(T_UINT8_T, group_id);
        CONVERT_PARAMTOSTR(T_YT_VLAN_PROTOCOL_ACTION_T, action);
    }

    /* execution func */
    ret = yt_vlan_protocol_based_vlan_table_set(unit, port, group_id, action);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_vlan_protocol_based_vlan_table_get(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_port_t port;
    uint8_t group_id;
    yt_vlan_protocol_action_t pAction;

    if(g_debug_switch)
        printf("is yt_vlan_protocol_based_vlan_table_get command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&port, 0, sizeof(yt_port_t));
    memset(&group_id, 0, sizeof(uint8_t));
    memset(&pAction, 0, sizeof(yt_vlan_protocol_action_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_PORT_T, port);
    CONVERT_PARAMFROMSTR(T_UINT8_T, group_id);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_PORT_T, port);
        CONVERT_PARAMTOSTR(T_UINT8_T, group_id);
    }

    /* execution func */
    ret = yt_vlan_protocol_based_vlan_table_get(unit, port, group_id, &pAction);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);/* print out parameters */
    if (!ret) {
        CONVERT_PARAMTOSTR(T_YT_VLAN_PROTOCOL_ACTION_T, pAction);
    }

    return 0;
}
static inline int cmd_yt_vlan_protocol_based_vlan_table_del(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_port_t port;
    uint8_t group_id;

    if(g_debug_switch)
        printf("is yt_vlan_protocol_based_vlan_table_del command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&port, 0, sizeof(yt_port_t));
    memset(&group_id, 0, sizeof(uint8_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_PORT_T, port);
    CONVERT_PARAMFROMSTR(T_UINT8_T, group_id);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_PORT_T, port);
        CONVERT_PARAMTOSTR(T_UINT8_T, group_id);
    }

    /* execution func */
    ret = yt_vlan_protocol_based_vlan_table_del(unit, port, group_id);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_vlan_trans_untagPvidIgnore_set(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_vlan_type_t type;
    yt_port_t port;
    yt_enable_t enable;

    if(g_debug_switch)
        printf("is yt_vlan_trans_untagPvidIgnore_set command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&type, 0, sizeof(yt_vlan_type_t));
    memset(&port, 0, sizeof(yt_port_t));
    memset(&enable, 0, sizeof(yt_enable_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_VLAN_TYPE_T, type);
    CONVERT_PARAMFROMSTR(T_YT_PORT_T, port);
    CONVERT_PARAMFROMSTR(T_YT_ENABLE_T, enable);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_VLAN_TYPE_T, type);
        CONVERT_PARAMTOSTR(T_YT_PORT_T, port);
        CONVERT_PARAMTOSTR(T_YT_ENABLE_T, enable);
    }

    /* execution func */
    ret = yt_vlan_trans_untagPvidIgnore_set(unit, type, port, enable);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_vlan_trans_untagPvidIgnore_get(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_vlan_type_t type;
    yt_port_t port;
    yt_enable_t pEnable;

    if(g_debug_switch)
        printf("is yt_vlan_trans_untagPvidIgnore_get command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&type, 0, sizeof(yt_vlan_type_t));
    memset(&port, 0, sizeof(yt_port_t));
    memset(&pEnable, 0, sizeof(yt_enable_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_VLAN_TYPE_T, type);
    CONVERT_PARAMFROMSTR(T_YT_PORT_T, port);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_VLAN_TYPE_T, type);
        CONVERT_PARAMTOSTR(T_YT_PORT_T, port);
    }

    /* execution func */
    ret = yt_vlan_trans_untagPvidIgnore_get(unit, type, port, &pEnable);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);/* print out parameters */
    if (!ret) {
        CONVERT_PARAMTOSTR(T_YT_ENABLE_T, pEnable);
    }

    return 0;
}
static inline int cmd_yt_vlan_trans_rangeProfile_add(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_vlan_range_group_t vlan_range;
    yt_profile_id_t pProfile_id;

    if(g_debug_switch)
        printf("is yt_vlan_trans_rangeProfile_add command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&vlan_range, 0, sizeof(yt_vlan_range_group_t));
    memset(&pProfile_id, 0, sizeof(yt_profile_id_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_VLAN_RANGE_GROUP_T, vlan_range);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_VLAN_RANGE_GROUP_T, vlan_range);
    }

    /* execution func */
    ret = yt_vlan_trans_rangeProfile_add(unit, vlan_range, &pProfile_id);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);/* print out parameters */
    if (!ret) {
        CONVERT_PARAMTOSTR(T_YT_PROFILE_ID_T, pProfile_id);
    }

    return 0;
}
static inline int cmd_yt_vlan_trans_rangeProfile_get(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_profile_id_t profile_id;
    yt_vlan_range_group_t pVlan_range;

    if(g_debug_switch)
        printf("is yt_vlan_trans_rangeProfile_get command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&profile_id, 0, sizeof(yt_profile_id_t));
    memset(&pVlan_range, 0, sizeof(yt_vlan_range_group_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_PROFILE_ID_T, profile_id);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_PROFILE_ID_T, profile_id);
    }

    /* execution func */
    ret = yt_vlan_trans_rangeProfile_get(unit, profile_id, &pVlan_range);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);/* print out parameters */
    if (!ret) {
        CONVERT_PARAMTOSTR(T_YT_VLAN_RANGE_GROUP_T, pVlan_range);
    }

    return 0;
}
static inline int cmd_yt_vlan_trans_rangeProfile_del(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_profile_id_t profile_id;

    if(g_debug_switch)
        printf("is yt_vlan_trans_rangeProfile_del command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&profile_id, 0, sizeof(yt_profile_id_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_PROFILE_ID_T, profile_id);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_PROFILE_ID_T, profile_id);
    }

    /* execution func */
    ret = yt_vlan_trans_rangeProfile_del(unit, profile_id);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_vlan_trans_port_rangeProfileSel_set(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_port_t port;
    yt_profile_id_t profile_id;

    if(g_debug_switch)
        printf("is yt_vlan_trans_port_rangeProfileSel_set command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&port, 0, sizeof(yt_port_t));
    memset(&profile_id, 0, sizeof(yt_profile_id_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_PORT_T, port);
    CONVERT_PARAMFROMSTR(T_YT_PROFILE_ID_T, profile_id);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_PORT_T, port);
        CONVERT_PARAMTOSTR(T_YT_PROFILE_ID_T, profile_id);
    }

    /* execution func */
    ret = yt_vlan_trans_port_rangeProfileSel_set(unit, port, profile_id);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_vlan_trans_port_rangeProfileSel_get(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_port_t port;
    yt_profile_id_t pProfile_id;

    if(g_debug_switch)
        printf("is yt_vlan_trans_port_rangeProfileSel_get command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&port, 0, sizeof(yt_port_t));
    memset(&pProfile_id, 0, sizeof(yt_profile_id_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_PORT_T, port);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_PORT_T, port);
    }

    /* execution func */
    ret = yt_vlan_trans_port_rangeProfileSel_get(unit, port, &pProfile_id);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);/* print out parameters */
    if (!ret) {
        CONVERT_PARAMTOSTR(T_YT_PROFILE_ID_T, pProfile_id);
    }

    return 0;
}
static inline int cmd_yt_vlan_trans_mode_set(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_port_t port;
    yt_vlan_range_trans_mode_t rangeMode;

    if(g_debug_switch)
        printf("is yt_vlan_trans_mode_set command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&port, 0, sizeof(yt_port_t));
    memset(&rangeMode, 0, sizeof(yt_vlan_range_trans_mode_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_PORT_T, port);
    CONVERT_PARAMFROMSTR(T_YT_VLAN_RANGE_TRANS_MODE_T, rangeMode);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_PORT_T, port);
        CONVERT_PARAMTOSTR(T_YT_VLAN_RANGE_TRANS_MODE_T, rangeMode);
    }

    /* execution func */
    ret = yt_vlan_trans_mode_set(unit, port, rangeMode);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_vlan_trans_mode_get(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_port_t port;
    yt_vlan_range_trans_mode_t pRangeMode;

    if(g_debug_switch)
        printf("is yt_vlan_trans_mode_get command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&port, 0, sizeof(yt_port_t));
    memset(&pRangeMode, 0, sizeof(yt_vlan_range_trans_mode_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_PORT_T, port);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_PORT_T, port);
    }

    /* execution func */
    ret = yt_vlan_trans_mode_get(unit, port, &pRangeMode);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);/* print out parameters */
    if (!ret) {
        CONVERT_PARAMTOSTR(T_YT_VLAN_RANGE_TRANS_MODE_T, pRangeMode);
    }

    return 0;
}
static inline int cmd_yt_vlan_igr_trans_table_add(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_vlan_trans_tbl_t pRuleTbl;
    yt_vlan_trans_action_tbl_t pAction;
    yt_trans_tbl_id_t pEntry_id;

    if(g_debug_switch)
        printf("is yt_vlan_igr_trans_table_add command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&pRuleTbl, 0, sizeof(yt_vlan_trans_tbl_t));
    memset(&pAction, 0, sizeof(yt_vlan_trans_action_tbl_t));
    memset(&pEntry_id, 0, sizeof(yt_trans_tbl_id_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_VLAN_TRANS_TBL_T, pRuleTbl);
    CONVERT_PARAMFROMSTR(T_YT_VLAN_TRANS_ACTION_TBL_T, pAction);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_VLAN_TRANS_TBL_T, pRuleTbl);
        CONVERT_PARAMTOSTR(T_YT_VLAN_TRANS_ACTION_TBL_T, pAction);
    }

    /* execution func */
    ret = yt_vlan_igr_trans_table_add(unit, &pRuleTbl, &pAction, &pEntry_id);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);/* print out parameters */
    if (!ret) {
        CONVERT_PARAMTOSTR(T_YT_TRANS_TBL_ID_T, pEntry_id);
    }

    return 0;
}
static inline int cmd_yt_vlan_igr_trans_table_get(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_trans_tbl_id_t entry_id;
    yt_vlan_trans_tbl_t pRuleTbl;
    yt_vlan_trans_action_tbl_t pAction;

    if(g_debug_switch)
        printf("is yt_vlan_igr_trans_table_get command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&entry_id, 0, sizeof(yt_trans_tbl_id_t));
    memset(&pRuleTbl, 0, sizeof(yt_vlan_trans_tbl_t));
    memset(&pAction, 0, sizeof(yt_vlan_trans_action_tbl_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_TRANS_TBL_ID_T, entry_id);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_TRANS_TBL_ID_T, entry_id);
    }

    /* execution func */
    ret = yt_vlan_igr_trans_table_get(unit, entry_id, &pRuleTbl, &pAction);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);/* print out parameters */
    if (!ret) {
        CONVERT_PARAMTOSTR(T_YT_VLAN_TRANS_TBL_T, pRuleTbl);
        CONVERT_PARAMTOSTR(T_YT_VLAN_TRANS_ACTION_TBL_T, pAction);
    }

    return 0;
}
static inline int cmd_yt_vlan_igr_trans_table_del(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_trans_tbl_id_t entry_id;

    if(g_debug_switch)
        printf("is yt_vlan_igr_trans_table_del command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&entry_id, 0, sizeof(yt_trans_tbl_id_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_TRANS_TBL_ID_T, entry_id);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_TRANS_TBL_ID_T, entry_id);
    }

    /* execution func */
    ret = yt_vlan_igr_trans_table_del(unit, entry_id);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_vlan_egr_trans_table_add(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_egr_vlan_trans_tbl_t pRuleTbl;
    yt_egr_vlan_trans_action_tbl_t pAction;
    yt_trans_tbl_id_t pEntry_id;

    if(g_debug_switch)
        printf("is yt_vlan_egr_trans_table_add command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&pRuleTbl, 0, sizeof(yt_egr_vlan_trans_tbl_t));
    memset(&pAction, 0, sizeof(yt_egr_vlan_trans_action_tbl_t));
    memset(&pEntry_id, 0, sizeof(yt_trans_tbl_id_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_EGR_VLAN_TRANS_TBL_T, pRuleTbl);
    CONVERT_PARAMFROMSTR(T_YT_EGR_VLAN_TRANS_ACTION_TBL_T, pAction);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_EGR_VLAN_TRANS_TBL_T, pRuleTbl);
        CONVERT_PARAMTOSTR(T_YT_EGR_VLAN_TRANS_ACTION_TBL_T, pAction);
    }

    /* execution func */
    ret = yt_vlan_egr_trans_table_add(unit, &pRuleTbl, &pAction, &pEntry_id);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);/* print out parameters */
    if (!ret) {
        CONVERT_PARAMTOSTR(T_YT_TRANS_TBL_ID_T, pEntry_id);
    }

    return 0;
}
static inline int cmd_yt_vlan_egr_trans_table_get(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_trans_tbl_id_t entry_idx;
    yt_egr_vlan_trans_tbl_t pRuleTbl;
    yt_egr_vlan_trans_action_tbl_t pAction;

    if(g_debug_switch)
        printf("is yt_vlan_egr_trans_table_get command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&entry_idx, 0, sizeof(yt_trans_tbl_id_t));
    memset(&pRuleTbl, 0, sizeof(yt_egr_vlan_trans_tbl_t));
    memset(&pAction, 0, sizeof(yt_egr_vlan_trans_action_tbl_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_TRANS_TBL_ID_T, entry_idx);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_TRANS_TBL_ID_T, entry_idx);
    }

    /* execution func */
    ret = yt_vlan_egr_trans_table_get(unit, entry_idx, &pRuleTbl, &pAction);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);/* print out parameters */
    if (!ret) {
        CONVERT_PARAMTOSTR(T_YT_EGR_VLAN_TRANS_TBL_T, pRuleTbl);
        CONVERT_PARAMTOSTR(T_YT_EGR_VLAN_TRANS_ACTION_TBL_T, pAction);
    }

    return 0;
}
static inline int cmd_yt_vlan_egr_trans_table_del(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_trans_tbl_id_t entry_idx;

    if(g_debug_switch)
        printf("is yt_vlan_egr_trans_table_del command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&entry_idx, 0, sizeof(yt_trans_tbl_id_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_TRANS_TBL_ID_T, entry_idx);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_TRANS_TBL_ID_T, entry_idx);
    }

    /* execution func */
    ret = yt_vlan_egr_trans_table_del(unit, entry_idx);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_acl_init(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;

    if(g_debug_switch)
        printf("is yt_acl_init command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
    }

    /* execution func */
    ret = yt_acl_init(unit);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_acl_port_en_set(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_port_t port;
    yt_enable_t enable;

    if(g_debug_switch)
        printf("is yt_acl_port_en_set command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&port, 0, sizeof(yt_port_t));
    memset(&enable, 0, sizeof(yt_enable_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_PORT_T, port);
    CONVERT_PARAMFROMSTR(T_YT_ENABLE_T, enable);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_PORT_T, port);
        CONVERT_PARAMTOSTR(T_YT_ENABLE_T, enable);
    }

    /* execution func */
    ret = yt_acl_port_en_set(unit, port, enable);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_acl_port_en_get(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_port_t port;
    yt_enable_t pEnable;

    if(g_debug_switch)
        printf("is yt_acl_port_en_get command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&port, 0, sizeof(yt_port_t));
    memset(&pEnable, 0, sizeof(yt_enable_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_PORT_T, port);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_PORT_T, port);
    }

    /* execution func */
    ret = yt_acl_port_en_get(unit, port, &pEnable);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);/* print out parameters */
    if (!ret) {
        CONVERT_PARAMTOSTR(T_YT_ENABLE_T, pEnable);
    }

    return 0;
}
static inline int cmd_yt_acl_unmatch_permit_en_set(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_port_t port;
    yt_enable_t enable;

    if(g_debug_switch)
        printf("is yt_acl_unmatch_permit_en_set command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&port, 0, sizeof(yt_port_t));
    memset(&enable, 0, sizeof(yt_enable_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_PORT_T, port);
    CONVERT_PARAMFROMSTR(T_YT_ENABLE_T, enable);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_PORT_T, port);
        CONVERT_PARAMTOSTR(T_YT_ENABLE_T, enable);
    }

    /* execution func */
    ret = yt_acl_unmatch_permit_en_set(unit, port, enable);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_acl_unmatch_permit_en_get(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_port_t port;
    yt_enable_t pEnable;

    if(g_debug_switch)
        printf("is yt_acl_unmatch_permit_en_get command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&port, 0, sizeof(yt_port_t));
    memset(&pEnable, 0, sizeof(yt_enable_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_PORT_T, port);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_PORT_T, port);
    }

    /* execution func */
    ret = yt_acl_unmatch_permit_en_get(unit, port, &pEnable);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);/* print out parameters */
    if (!ret) {
        CONVERT_PARAMTOSTR(T_YT_ENABLE_T, pEnable);
    }

    return 0;
}
static inline int cmd_yt_acl_udf_rule_set(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    uint8_t index;
    yt_acl_udf_type_t type;
    uint8_t offset;

    if(g_debug_switch)
        printf("is yt_acl_udf_rule_set command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&index, 0, sizeof(uint8_t));
    memset(&type, 0, sizeof(yt_acl_udf_type_t));
    memset(&offset, 0, sizeof(uint8_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_UINT8_T, index);
    CONVERT_PARAMFROMSTR(T_YT_ACL_UDF_TYPE_T, type);
    CONVERT_PARAMFROMSTR(T_UINT8_T, offset);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_UINT8_T, index);
        CONVERT_PARAMTOSTR(T_YT_ACL_UDF_TYPE_T, type);
        CONVERT_PARAMTOSTR(T_UINT8_T, offset);
    }

    /* execution func */
    ret = yt_acl_udf_rule_set(unit, index, type, offset);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_acl_rule_init(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;

    if(g_debug_switch)
        printf("is yt_acl_rule_init command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
    }

    /* execution func */
    ret = yt_acl_rule_init(unit);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_acl_rule_reset(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;

    if(g_debug_switch)
        printf("is yt_acl_rule_reset command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
    }

    /* execution func */
    ret = yt_acl_rule_reset(unit);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_acl_rule_key_add(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_igrAcl_key_type_t type;
    yt_comm_key_t pKey_data;

    if(g_debug_switch)
        printf("is yt_acl_rule_key_add command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&type, 0, sizeof(yt_igrAcl_key_type_t));
    memset(&pKey_data, 0, sizeof(yt_comm_key_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_IGRACL_KEY_TYPE_T, type);
    CONVERT_PARAMFROMSTR(T_YT_COMM_KEY_T, pKey_data);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_IGRACL_KEY_TYPE_T, type);
        CONVERT_PARAMTOSTR(T_YT_COMM_KEY_T, pKey_data);
    }

    /* execution func */
    ret = yt_acl_rule_key_add(unit, type, &pKey_data);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_acl_rule_action_add(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_acl_action_type_t type;
    yt_comm_act_t pAction;

    if(g_debug_switch)
        printf("is yt_acl_rule_action_add command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&type, 0, sizeof(yt_acl_action_type_t));
    memset(&pAction, 0, sizeof(yt_comm_act_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_ACL_ACTION_TYPE_T, type);
    CONVERT_PARAMFROMSTR(T_YT_COMM_ACT_T, pAction);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_ACL_ACTION_TYPE_T, type);
        CONVERT_PARAMTOSTR(T_YT_COMM_ACT_T, pAction);
    }

    /* execution func */
    ret = yt_acl_rule_action_add(unit, type, &pAction);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_acl_rule_create(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    uint16_t rulePri;
    yt_bool_t ruleReverse;
    uint32_t pId;

    if(g_debug_switch)
        printf("is yt_acl_rule_create command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&rulePri, 0, sizeof(uint16_t));
    memset(&ruleReverse, 0, sizeof(yt_bool_t));
    memset(&pId, 0, sizeof(uint32_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_UINT16_T, rulePri);
    CONVERT_PARAMFROMSTR(T_YT_BOOL_T, ruleReverse);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_UINT16_T, rulePri);
        CONVERT_PARAMTOSTR(T_YT_BOOL_T, ruleReverse);
    }

    /* execution func */
    ret = yt_acl_rule_create(unit, rulePri, ruleReverse, &pId);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);/* print out parameters */
    if (!ret) {
        CONVERT_PARAMTOSTR(T_UINT32_T, pId);
    }

    return 0;
}
static inline int cmd_yt_acl_rule_active(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    uint32_t id;

    if(g_debug_switch)
        printf("is yt_acl_rule_active command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&id, 0, sizeof(uint32_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_UINT32_T, id);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_UINT32_T, id);
    }

    /* execution func */
    ret = yt_acl_rule_active(unit, id);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_acl_rule_del(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    uint32_t id;

    if(g_debug_switch)
        printf("is yt_acl_rule_del command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&id, 0, sizeof(uint32_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_UINT32_T, id);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_UINT32_T, id);
    }

    /* execution func */
    ret = yt_acl_rule_del(unit, id);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_ctrlpkt_unknown_ucast_act_set(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_port_t port;
    yt_ctrlpkt_l2_action_t act_ctrl;

    if(g_debug_switch)
        printf("is yt_ctrlpkt_unknown_ucast_act_set command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&port, 0, sizeof(yt_port_t));
    memset(&act_ctrl, 0, sizeof(yt_ctrlpkt_l2_action_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_PORT_T, port);
    CONVERT_PARAMFROMSTR(T_YT_CTRLPKT_L2_ACTION_T, act_ctrl);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_PORT_T, port);
        CONVERT_PARAMTOSTR(T_YT_CTRLPKT_L2_ACTION_T, act_ctrl);
    }

    /* execution func */
    ret = yt_ctrlpkt_unknown_ucast_act_set(unit, port, act_ctrl);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_ctrlpkt_unknown_ucast_act_get(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_port_t port;
    yt_ctrlpkt_l2_action_t pAct_ctrl;

    if(g_debug_switch)
        printf("is yt_ctrlpkt_unknown_ucast_act_get command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&port, 0, sizeof(yt_port_t));
    memset(&pAct_ctrl, 0, sizeof(yt_ctrlpkt_l2_action_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_PORT_T, port);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_PORT_T, port);
    }

    /* execution func */
    ret = yt_ctrlpkt_unknown_ucast_act_get(unit, port, &pAct_ctrl);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);/* print out parameters */
    if (!ret) {
        CONVERT_PARAMTOSTR(T_YT_CTRLPKT_L2_ACTION_T, pAct_ctrl);
    }

    return 0;
}
static inline int cmd_yt_ctrlpkt_unknown_mcast_act_set(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_port_t port;
    yt_ctrlpkt_l2_action_t act_ctrl;

    if(g_debug_switch)
        printf("is yt_ctrlpkt_unknown_mcast_act_set command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&port, 0, sizeof(yt_port_t));
    memset(&act_ctrl, 0, sizeof(yt_ctrlpkt_l2_action_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_PORT_T, port);
    CONVERT_PARAMFROMSTR(T_YT_CTRLPKT_L2_ACTION_T, act_ctrl);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_PORT_T, port);
        CONVERT_PARAMTOSTR(T_YT_CTRLPKT_L2_ACTION_T, act_ctrl);
    }

    /* execution func */
    ret = yt_ctrlpkt_unknown_mcast_act_set(unit, port, act_ctrl);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_ctrlpkt_unknown_mcast_act_get(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_port_t port;
    yt_ctrlpkt_l2_action_t pAct_ctrl;

    if(g_debug_switch)
        printf("is yt_ctrlpkt_unknown_mcast_act_get command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&port, 0, sizeof(yt_port_t));
    memset(&pAct_ctrl, 0, sizeof(yt_ctrlpkt_l2_action_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_PORT_T, port);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_PORT_T, port);
    }

    /* execution func */
    ret = yt_ctrlpkt_unknown_mcast_act_get(unit, port, &pAct_ctrl);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);/* print out parameters */
    if (!ret) {
        CONVERT_PARAMTOSTR(T_YT_CTRLPKT_L2_ACTION_T, pAct_ctrl);
    }

    return 0;
}
static inline int cmd_yt_ctrlpkt_arp_act_set(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_port_t port;
    yt_ctrlpkt_l2_action_t act_ctrl;

    if(g_debug_switch)
        printf("is yt_ctrlpkt_arp_act_set command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&port, 0, sizeof(yt_port_t));
    memset(&act_ctrl, 0, sizeof(yt_ctrlpkt_l2_action_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_PORT_T, port);
    CONVERT_PARAMFROMSTR(T_YT_CTRLPKT_L2_ACTION_T, act_ctrl);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_PORT_T, port);
        CONVERT_PARAMTOSTR(T_YT_CTRLPKT_L2_ACTION_T, act_ctrl);
    }

    /* execution func */
    ret = yt_ctrlpkt_arp_act_set(unit, port, act_ctrl);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_ctrlpkt_arp_act_get(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_port_t port;
    yt_ctrlpkt_l2_action_t pAct_ctrl;

    if(g_debug_switch)
        printf("is yt_ctrlpkt_arp_act_get command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&port, 0, sizeof(yt_port_t));
    memset(&pAct_ctrl, 0, sizeof(yt_ctrlpkt_l2_action_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_PORT_T, port);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_PORT_T, port);
    }

    /* execution func */
    ret = yt_ctrlpkt_arp_act_get(unit, port, &pAct_ctrl);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);/* print out parameters */
    if (!ret) {
        CONVERT_PARAMTOSTR(T_YT_CTRLPKT_L2_ACTION_T, pAct_ctrl);
    }

    return 0;
}
static inline int cmd_yt_ctrlpkt_nd_act_set(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_port_t port;
    yt_ctrlpkt_l2_action_t act_ctrl;

    if(g_debug_switch)
        printf("is yt_ctrlpkt_nd_act_set command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&port, 0, sizeof(yt_port_t));
    memset(&act_ctrl, 0, sizeof(yt_ctrlpkt_l2_action_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_PORT_T, port);
    CONVERT_PARAMFROMSTR(T_YT_CTRLPKT_L2_ACTION_T, act_ctrl);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_PORT_T, port);
        CONVERT_PARAMTOSTR(T_YT_CTRLPKT_L2_ACTION_T, act_ctrl);
    }

    /* execution func */
    ret = yt_ctrlpkt_nd_act_set(unit, port, act_ctrl);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_ctrlpkt_nd_act_get(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_port_t port;
    yt_ctrlpkt_l2_action_t pAct_ctrl;

    if(g_debug_switch)
        printf("is yt_ctrlpkt_nd_act_get command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&port, 0, sizeof(yt_port_t));
    memset(&pAct_ctrl, 0, sizeof(yt_ctrlpkt_l2_action_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_PORT_T, port);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_PORT_T, port);
    }

    /* execution func */
    ret = yt_ctrlpkt_nd_act_get(unit, port, &pAct_ctrl);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);/* print out parameters */
    if (!ret) {
        CONVERT_PARAMTOSTR(T_YT_CTRLPKT_L2_ACTION_T, pAct_ctrl);
    }

    return 0;
}
static inline int cmd_yt_ctrlpkt_lldp_eee_act_set(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_port_t port;
    yt_ctrlpkt_l2_action_t act_ctrl;

    if(g_debug_switch)
        printf("is yt_ctrlpkt_lldp_eee_act_set command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&port, 0, sizeof(yt_port_t));
    memset(&act_ctrl, 0, sizeof(yt_ctrlpkt_l2_action_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_PORT_T, port);
    CONVERT_PARAMFROMSTR(T_YT_CTRLPKT_L2_ACTION_T, act_ctrl);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_PORT_T, port);
        CONVERT_PARAMTOSTR(T_YT_CTRLPKT_L2_ACTION_T, act_ctrl);
    }

    /* execution func */
    ret = yt_ctrlpkt_lldp_eee_act_set(unit, port, act_ctrl);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_ctrlpkt_lldp_eee_act_get(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_port_t port;
    yt_ctrlpkt_l2_action_t pAct_ctrl;

    if(g_debug_switch)
        printf("is yt_ctrlpkt_lldp_eee_act_get command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&port, 0, sizeof(yt_port_t));
    memset(&pAct_ctrl, 0, sizeof(yt_ctrlpkt_l2_action_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_PORT_T, port);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_PORT_T, port);
    }

    /* execution func */
    ret = yt_ctrlpkt_lldp_eee_act_get(unit, port, &pAct_ctrl);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);/* print out parameters */
    if (!ret) {
        CONVERT_PARAMTOSTR(T_YT_CTRLPKT_L2_ACTION_T, pAct_ctrl);
    }

    return 0;
}
static inline int cmd_yt_ctrlpkt_lldp_act_set(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_port_t port;
    yt_ctrlpkt_l2_action_t act_ctrl;

    if(g_debug_switch)
        printf("is yt_ctrlpkt_lldp_act_set command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&port, 0, sizeof(yt_port_t));
    memset(&act_ctrl, 0, sizeof(yt_ctrlpkt_l2_action_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_PORT_T, port);
    CONVERT_PARAMFROMSTR(T_YT_CTRLPKT_L2_ACTION_T, act_ctrl);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_PORT_T, port);
        CONVERT_PARAMTOSTR(T_YT_CTRLPKT_L2_ACTION_T, act_ctrl);
    }

    /* execution func */
    ret = yt_ctrlpkt_lldp_act_set(unit, port, act_ctrl);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_ctrlpkt_lldp_act_get(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_port_t port;
    yt_ctrlpkt_l2_action_t pAct_ctrl;

    if(g_debug_switch)
        printf("is yt_ctrlpkt_lldp_act_get command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&port, 0, sizeof(yt_port_t));
    memset(&pAct_ctrl, 0, sizeof(yt_ctrlpkt_l2_action_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_PORT_T, port);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_PORT_T, port);
    }

    /* execution func */
    ret = yt_ctrlpkt_lldp_act_get(unit, port, &pAct_ctrl);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);/* print out parameters */
    if (!ret) {
        CONVERT_PARAMTOSTR(T_YT_CTRLPKT_L2_ACTION_T, pAct_ctrl);
    }

    return 0;
}
static inline int cmd_yt_port_init(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;

    if(g_debug_switch)
        printf("is yt_port_init command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
    }

    /* execution func */
    ret = yt_port_init(unit);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_port_enable_set(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_port_t port;
    yt_enable_t enable;

    if(g_debug_switch)
        printf("is yt_port_enable_set command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&port, 0, sizeof(yt_port_t));
    memset(&enable, 0, sizeof(yt_enable_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_PORT_T, port);
    CONVERT_PARAMFROMSTR(T_YT_ENABLE_T, enable);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_PORT_T, port);
        CONVERT_PARAMTOSTR(T_YT_ENABLE_T, enable);
    }

    /* execution func */
    ret = yt_port_enable_set(unit, port, enable);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_port_enable_get(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_port_t port;
    yt_enable_t pEnable;

    if(g_debug_switch)
        printf("is yt_port_enable_get command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&port, 0, sizeof(yt_port_t));
    memset(&pEnable, 0, sizeof(yt_enable_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_PORT_T, port);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_PORT_T, port);
    }

    /* execution func */
    ret = yt_port_enable_get(unit, port, &pEnable);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);/* print out parameters */
    if (!ret) {
        CONVERT_PARAMTOSTR(T_YT_ENABLE_T, pEnable);
    }

    return 0;
}
static inline int cmd_yt_port_link_status_get(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_port_t port;
    yt_port_link_status_t pLinkStatus;

    if(g_debug_switch)
        printf("is yt_port_link_status_get command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&port, 0, sizeof(yt_port_t));
    memset(&pLinkStatus, 0, sizeof(yt_port_link_status_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_PORT_T, port);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_PORT_T, port);
    }

    /* execution func */
    ret = yt_port_link_status_get(unit, port, &pLinkStatus);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);/* print out parameters */
    if (!ret) {
        CONVERT_PARAMTOSTR(T_YT_PORT_LINK_STATUS_T, pLinkStatus);
    }

    return 0;
}
static inline int cmd_yt_port_link_status_all_get(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_port_t port;
    yt_port_linkStatus_all_t pAllLinkStatus;

    if(g_debug_switch)
        printf("is yt_port_link_status_all_get command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&port, 0, sizeof(yt_port_t));
    memset(&pAllLinkStatus, 0, sizeof(yt_port_linkStatus_all_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_PORT_T, port);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_PORT_T, port);
    }

    /* execution func */
    ret = yt_port_link_status_all_get(unit, port, &pAllLinkStatus);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);/* print out parameters */
    if (!ret) {
        CONVERT_PARAMTOSTR(T_YT_PORT_LINKSTATUS_ALL_T, pAllLinkStatus);
    }

    return 0;
}
static inline int cmd_yt_port_backpress_enable_set(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_port_t port;
    yt_enable_t enable;

    if(g_debug_switch)
        printf("is yt_port_backpress_enable_set command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&port, 0, sizeof(yt_port_t));
    memset(&enable, 0, sizeof(yt_enable_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_PORT_T, port);
    CONVERT_PARAMFROMSTR(T_YT_ENABLE_T, enable);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_PORT_T, port);
        CONVERT_PARAMTOSTR(T_YT_ENABLE_T, enable);
    }

    /* execution func */
    ret = yt_port_backpress_enable_set(unit, port, enable);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_port_backpress_enable_get(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_port_t port;
    yt_enable_t pEnable;

    if(g_debug_switch)
        printf("is yt_port_backpress_enable_get command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&port, 0, sizeof(yt_port_t));
    memset(&pEnable, 0, sizeof(yt_enable_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_PORT_T, port);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_PORT_T, port);
    }

    /* execution func */
    ret = yt_port_backpress_enable_get(unit, port, &pEnable);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);/* print out parameters */
    if (!ret) {
        CONVERT_PARAMTOSTR(T_YT_ENABLE_T, pEnable);
    }

    return 0;
}
static inline int cmd_yt_port_cascade_set(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_cascade_info_t cascade_info;

    if(g_debug_switch)
        printf("is yt_port_cascade_set command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&cascade_info, 0, sizeof(yt_cascade_info_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_CASCADE_INFO_T, cascade_info);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_CASCADE_INFO_T, cascade_info);
    }

    /* execution func */
    ret = yt_port_cascade_set(unit, cascade_info);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_port_cascade_get(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_cascade_info_t pCascade_info;

    if(g_debug_switch)
        printf("is yt_port_cascade_get command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&pCascade_info, 0, sizeof(yt_cascade_info_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
    }

    /* execution func */
    ret = yt_port_cascade_get(unit, &pCascade_info);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);/* print out parameters */
    if (!ret) {
        CONVERT_PARAMTOSTR(T_YT_CASCADE_INFO_T, pCascade_info);
    }

    return 0;
}
static inline int cmd_yt_port_pkt_gap_set(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_port_t port;
    uint8_t gap;

    if(g_debug_switch)
        printf("is yt_port_pkt_gap_set command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&port, 0, sizeof(yt_port_t));
    memset(&gap, 0, sizeof(uint8_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_PORT_T, port);
    CONVERT_PARAMFROMSTR(T_UINT8_T, gap);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_PORT_T, port);
        CONVERT_PARAMTOSTR(T_UINT8_T, gap);
    }

    /* execution func */
    ret = yt_port_pkt_gap_set(unit, port, gap);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_port_pkt_gap_get(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_port_t port;
    uint8_t pGap;

    if(g_debug_switch)
        printf("is yt_port_pkt_gap_get command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&port, 0, sizeof(yt_port_t));
    memset(&pGap, 0, sizeof(uint8_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_PORT_T, port);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_PORT_T, port);
    }

    /* execution func */
    ret = yt_port_pkt_gap_get(unit, port, &pGap);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);/* print out parameters */
    if (!ret) {
        CONVERT_PARAMTOSTR(T_UINT8_T, pGap);
    }

    return 0;
}
static inline int cmd_yt_port_macAutoNeg_enable_set(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_port_t port;
    yt_enable_t enable;

    if(g_debug_switch)
        printf("is yt_port_macAutoNeg_enable_set command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&port, 0, sizeof(yt_port_t));
    memset(&enable, 0, sizeof(yt_enable_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_PORT_T, port);
    CONVERT_PARAMFROMSTR(T_YT_ENABLE_T, enable);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_PORT_T, port);
        CONVERT_PARAMTOSTR(T_YT_ENABLE_T, enable);
    }

    /* execution func */
    ret = yt_port_macAutoNeg_enable_set(unit, port, enable);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_port_macAutoNeg_enable_get(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_port_t port;
    yt_enable_t pEnable;

    if(g_debug_switch)
        printf("is yt_port_macAutoNeg_enable_get command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&port, 0, sizeof(yt_port_t));
    memset(&pEnable, 0, sizeof(yt_enable_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_PORT_T, port);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_PORT_T, port);
    }

    /* execution func */
    ret = yt_port_macAutoNeg_enable_get(unit, port, &pEnable);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);/* print out parameters */
    if (!ret) {
        CONVERT_PARAMTOSTR(T_YT_ENABLE_T, pEnable);
    }

    return 0;
}
static inline int cmd_yt_port_mac_force_set(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_port_t port;
    yt_port_force_ctrl_t port_ctrl;

    if(g_debug_switch)
        printf("is yt_port_mac_force_set command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&port, 0, sizeof(yt_port_t));
    memset(&port_ctrl, 0, sizeof(yt_port_force_ctrl_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_PORT_T, port);
    CONVERT_PARAMFROMSTR(T_YT_PORT_FORCE_CTRL_T, port_ctrl);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_PORT_T, port);
        CONVERT_PARAMTOSTR(T_YT_PORT_FORCE_CTRL_T, port_ctrl);
    }

    /* execution func */
    ret = yt_port_mac_force_set(unit, port, port_ctrl);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_port_mac_force_get(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_port_t port;
    yt_port_force_ctrl_t pPort_ctrl;

    if(g_debug_switch)
        printf("is yt_port_mac_force_get command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&port, 0, sizeof(yt_port_t));
    memset(&pPort_ctrl, 0, sizeof(yt_port_force_ctrl_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_PORT_T, port);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_PORT_T, port);
    }

    /* execution func */
    ret = yt_port_mac_force_get(unit, port, &pPort_ctrl);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);/* print out parameters */
    if (!ret) {
        CONVERT_PARAMTOSTR(T_YT_PORT_FORCE_CTRL_T, pPort_ctrl);
    }

    return 0;
}
static inline int cmd_yt_port_mac_fc_set(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_port_t port;
    yt_enable_t enable;

    if(g_debug_switch)
        printf("is yt_port_mac_fc_set command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&port, 0, sizeof(yt_port_t));
    memset(&enable, 0, sizeof(yt_enable_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_PORT_T, port);
    CONVERT_PARAMFROMSTR(T_YT_ENABLE_T, enable);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_PORT_T, port);
        CONVERT_PARAMTOSTR(T_YT_ENABLE_T, enable);
    }

    /* execution func */
    ret = yt_port_mac_fc_set(unit, port, enable);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_port_mac_fc_get(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_port_t port;
    yt_enable_t pEnable;

    if(g_debug_switch)
        printf("is yt_port_mac_fc_get command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&port, 0, sizeof(yt_port_t));
    memset(&pEnable, 0, sizeof(yt_enable_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_PORT_T, port);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_PORT_T, port);
    }

    /* execution func */
    ret = yt_port_mac_fc_get(unit, port, &pEnable);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);/* print out parameters */
    if (!ret) {
        CONVERT_PARAMTOSTR(T_YT_ENABLE_T, pEnable);
    }

    return 0;
}
static inline int cmd_yt_port_extif_mode_set(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_port_t port;
    yt_extif_mode_t mode;

    if(g_debug_switch)
        printf("is yt_port_extif_mode_set command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&port, 0, sizeof(yt_port_t));
    memset(&mode, 0, sizeof(yt_extif_mode_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_PORT_T, port);
    CONVERT_PARAMFROMSTR(T_YT_EXTIF_MODE_T, mode);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_PORT_T, port);
        CONVERT_PARAMTOSTR(T_YT_EXTIF_MODE_T, mode);
    }

    /* execution func */
    ret = yt_port_extif_mode_set(unit, port, mode);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_port_extif_mode_get(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_port_t port;
    yt_extif_mode_t pMode;

    if(g_debug_switch)
        printf("is yt_port_extif_mode_get command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&port, 0, sizeof(yt_port_t));
    memset(&pMode, 0, sizeof(yt_extif_mode_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_PORT_T, port);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_PORT_T, port);
    }

    /* execution func */
    ret = yt_port_extif_mode_get(unit, port, &pMode);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);/* print out parameters */
    if (!ret) {
        CONVERT_PARAMTOSTR(T_YT_EXTIF_MODE_T, pMode);
    }

    return 0;
}
static inline int cmd_yt_port_extif_rgmii_delay_set(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_port_t port;
    uint8_t rxc_delay;
    uint8_t txc_delay;
    yt_enable_t txc_2ns_en;

    if(g_debug_switch)
        printf("is yt_port_extif_rgmii_delay_set command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&port, 0, sizeof(yt_port_t));
    memset(&rxc_delay, 0, sizeof(uint8_t));
    memset(&txc_delay, 0, sizeof(uint8_t));
    memset(&txc_2ns_en, 0, sizeof(yt_enable_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_PORT_T, port);
    CONVERT_PARAMFROMSTR(T_UINT8_T, rxc_delay);
    CONVERT_PARAMFROMSTR(T_UINT8_T, txc_delay);
    CONVERT_PARAMFROMSTR(T_YT_ENABLE_T, txc_2ns_en);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_PORT_T, port);
        CONVERT_PARAMTOSTR(T_UINT8_T, rxc_delay);
        CONVERT_PARAMTOSTR(T_UINT8_T, txc_delay);
        CONVERT_PARAMTOSTR(T_YT_ENABLE_T, txc_2ns_en);
    }

    /* execution func */
    ret = yt_port_extif_rgmii_delay_set(unit, port, rxc_delay, txc_delay, txc_2ns_en);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_port_extif_rgmii_delay_get(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_port_t port;
    uint8_t pRxc_delay;
    uint8_t pTxc_delay;
    yt_enable_t pTxc_2ns_en;

    if(g_debug_switch)
        printf("is yt_port_extif_rgmii_delay_get command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&port, 0, sizeof(yt_port_t));
    memset(&pRxc_delay, 0, sizeof(uint8_t));
    memset(&pTxc_delay, 0, sizeof(uint8_t));
    memset(&pTxc_2ns_en, 0, sizeof(yt_enable_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_PORT_T, port);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_PORT_T, port);
    }

    /* execution func */
    ret = yt_port_extif_rgmii_delay_get(unit, port, &pRxc_delay, &pTxc_delay, &pTxc_2ns_en);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);/* print out parameters */
    if (!ret) {
        CONVERT_PARAMTOSTR(T_UINT8_T, pRxc_delay);
        CONVERT_PARAMTOSTR(T_UINT8_T, pTxc_delay);
        CONVERT_PARAMTOSTR(T_YT_ENABLE_T, pTxc_2ns_en);
    }

    return 0;
}
static inline int cmd_yt_port_phyAutoNeg_enable_set(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_port_t port;
    yt_enable_t enable;

    if(g_debug_switch)
        printf("is yt_port_phyAutoNeg_enable_set command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&port, 0, sizeof(yt_port_t));
    memset(&enable, 0, sizeof(yt_enable_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_PORT_T, port);
    CONVERT_PARAMFROMSTR(T_YT_ENABLE_T, enable);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_PORT_T, port);
        CONVERT_PARAMTOSTR(T_YT_ENABLE_T, enable);
    }

    /* execution func */
    ret = yt_port_phyAutoNeg_enable_set(unit, port, enable);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_port_phyAutoNeg_enable_get(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_port_t port;
    yt_enable_t pEnable;

    if(g_debug_switch)
        printf("is yt_port_phyAutoNeg_enable_get command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&port, 0, sizeof(yt_port_t));
    memset(&pEnable, 0, sizeof(yt_enable_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_PORT_T, port);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_PORT_T, port);
    }

    /* execution func */
    ret = yt_port_phyAutoNeg_enable_get(unit, port, &pEnable);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);/* print out parameters */
    if (!ret) {
        CONVERT_PARAMTOSTR(T_YT_ENABLE_T, pEnable);
    }

    return 0;
}
static inline int cmd_yt_port_phyAutoNeg_ability_set(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_port_t port;
    yt_port_an_ability_t ability;

    if(g_debug_switch)
        printf("is yt_port_phyAutoNeg_ability_set command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&port, 0, sizeof(yt_port_t));
    memset(&ability, 0, sizeof(yt_port_an_ability_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_PORT_T, port);
    CONVERT_PARAMFROMSTR(T_YT_PORT_AN_ABILITY_T, ability);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_PORT_T, port);
        CONVERT_PARAMTOSTR(T_YT_PORT_AN_ABILITY_T, ability);
    }

    /* execution func */
    ret = yt_port_phyAutoNeg_ability_set(unit, port, ability);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_port_phyAutoNeg_ability_get(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_port_t port;
    yt_port_an_ability_t pAbility;

    if(g_debug_switch)
        printf("is yt_port_phyAutoNeg_ability_get command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&port, 0, sizeof(yt_port_t));
    memset(&pAbility, 0, sizeof(yt_port_an_ability_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_PORT_T, port);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_PORT_T, port);
    }

    /* execution func */
    ret = yt_port_phyAutoNeg_ability_get(unit, port, &pAbility);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);/* print out parameters */
    if (!ret) {
        CONVERT_PARAMTOSTR(T_YT_PORT_AN_ABILITY_T, pAbility);
    }

    return 0;
}
static inline int cmd_yt_port_phy_force_set(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_port_t port;
    yt_port_speed_duplex_t speed_dup;

    if(g_debug_switch)
        printf("is yt_port_phy_force_set command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&port, 0, sizeof(yt_port_t));
    memset(&speed_dup, 0, sizeof(yt_port_speed_duplex_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_PORT_T, port);
    CONVERT_PARAMFROMSTR(T_YT_PORT_SPEED_DUPLEX_T, speed_dup);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_PORT_T, port);
        CONVERT_PARAMTOSTR(T_YT_PORT_SPEED_DUPLEX_T, speed_dup);
    }

    /* execution func */
    ret = yt_port_phy_force_set(unit, port, speed_dup);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_port_phy_force_get(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_port_t port;
    yt_port_speed_duplex_t pSpeedDup;

    if(g_debug_switch)
        printf("is yt_port_phy_force_get command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&port, 0, sizeof(yt_port_t));
    memset(&pSpeedDup, 0, sizeof(yt_port_speed_duplex_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_PORT_T, port);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_PORT_T, port);
    }

    /* execution func */
    ret = yt_port_phy_force_get(unit, port, &pSpeedDup);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);/* print out parameters */
    if (!ret) {
        CONVERT_PARAMTOSTR(T_YT_PORT_SPEED_DUPLEX_T, pSpeedDup);
    }

    return 0;
}
static inline int cmd_yt_port_phy_linkstatus_get(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_port_t port;
    yt_port_linkStatus_all_t pLinkStatus;

    if(g_debug_switch)
        printf("is yt_port_phy_linkstatus_get command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&port, 0, sizeof(yt_port_t));
    memset(&pLinkStatus, 0, sizeof(yt_port_linkStatus_all_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_PORT_T, port);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_PORT_T, port);
    }

    /* execution func */
    ret = yt_port_phy_linkstatus_get(unit, port, &pLinkStatus);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);/* print out parameters */
    if (!ret) {
        CONVERT_PARAMTOSTR(T_YT_PORT_LINKSTATUS_ALL_T, pLinkStatus);
    }

    return 0;
}
static inline int cmd_yt_port_phy_reg_set(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_port_t port;
    uint32_t regAddr;
    uint16_t data;
    yt_phy_type_t type;

    if(g_debug_switch)
        printf("is yt_port_phy_reg_set command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&port, 0, sizeof(yt_port_t));
    memset(&regAddr, 0, sizeof(uint32_t));
    memset(&data, 0, sizeof(uint16_t));
    memset(&type, 0, sizeof(yt_phy_type_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_PORT_T, port);
    CONVERT_PARAMFROMSTR(T_UINT32_T, regAddr);
    CONVERT_PARAMFROMSTR(T_UINT16_T, data);
    CONVERT_PARAMFROMSTR(T_YT_PHY_TYPE_T, type);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_PORT_T, port);
        CONVERT_PARAMTOSTR(T_UINT32_T, regAddr);
        CONVERT_PARAMTOSTR(T_UINT16_T, data);
        CONVERT_PARAMTOSTR(T_YT_PHY_TYPE_T, type);
    }

    /* execution func */
    ret = yt_port_phy_reg_set(unit, port, regAddr, data, type);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_port_phy_reg_get(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_port_t port;
    uint32_t regAddr;
    uint16_t pData;
    yt_phy_type_t type;

    if(g_debug_switch)
        printf("is yt_port_phy_reg_get command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&port, 0, sizeof(yt_port_t));
    memset(&regAddr, 0, sizeof(uint32_t));
    memset(&pData, 0, sizeof(uint16_t));
    memset(&type, 0, sizeof(yt_phy_type_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_PORT_T, port);
    CONVERT_PARAMFROMSTR(T_UINT32_T, regAddr);
    CONVERT_PARAMFROMSTR(T_YT_PHY_TYPE_T, type);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_PORT_T, port);
        CONVERT_PARAMTOSTR(T_UINT32_T, regAddr);
        CONVERT_PARAMTOSTR(T_YT_PHY_TYPE_T, type);
    }

    /* execution func */
    ret = yt_port_phy_reg_get(unit, port, regAddr, &pData, type);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);/* print out parameters */
    if (!ret) {
        CONVERT_PARAMTOSTR(T_UINT16_T, pData);
    }

    return 0;
}
static inline int cmd_yt_port_eee_enable_set(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_port_t port;
    yt_enable_t enable;

    if(g_debug_switch)
        printf("is yt_port_eee_enable_set command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&port, 0, sizeof(yt_port_t));
    memset(&enable, 0, sizeof(yt_enable_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_PORT_T, port);
    CONVERT_PARAMFROMSTR(T_YT_ENABLE_T, enable);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_PORT_T, port);
        CONVERT_PARAMTOSTR(T_YT_ENABLE_T, enable);
    }

    /* execution func */
    ret = yt_port_eee_enable_set(unit, port, enable);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_port_eee_enable_get(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_port_t port;
    yt_enable_t pEnable;

    if(g_debug_switch)
        printf("is yt_port_eee_enable_get command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&port, 0, sizeof(yt_port_t));
    memset(&pEnable, 0, sizeof(yt_enable_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_PORT_T, port);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_PORT_T, port);
    }

    /* execution func */
    ret = yt_port_eee_enable_get(unit, port, &pEnable);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);/* print out parameters */
    if (!ret) {
        CONVERT_PARAMTOSTR(T_YT_ENABLE_T, pEnable);
    }

    return 0;
}
static inline int cmd_yt_port_jumbo_size_set(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_port_t port;
    uint32_t size;

    if(g_debug_switch)
        printf("is yt_port_jumbo_size_set command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&port, 0, sizeof(yt_port_t));
    memset(&size, 0, sizeof(uint32_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_PORT_T, port);
    CONVERT_PARAMFROMSTR(T_UINT32_T, size);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_PORT_T, port);
        CONVERT_PARAMTOSTR(T_UINT32_T, size);
    }

    /* execution func */
    ret = yt_port_jumbo_size_set(unit, port, size);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_port_jumbo_size_get(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_port_t port;
    uint32_t pSize;

    if(g_debug_switch)
        printf("is yt_port_jumbo_size_get command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&port, 0, sizeof(yt_port_t));
    memset(&pSize, 0, sizeof(uint32_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_PORT_T, port);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_PORT_T, port);
    }

    /* execution func */
    ret = yt_port_jumbo_size_get(unit, port, &pSize);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);/* print out parameters */
    if (!ret) {
        CONVERT_PARAMTOSTR(T_UINT32_T, pSize);
    }

    return 0;
}
static inline int cmd_yt_port_jumbo_enable_set(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_port_t port;
    yt_enable_t enable;

    if(g_debug_switch)
        printf("is yt_port_jumbo_enable_set command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&port, 0, sizeof(yt_port_t));
    memset(&enable, 0, sizeof(yt_enable_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_PORT_T, port);
    CONVERT_PARAMFROMSTR(T_YT_ENABLE_T, enable);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_PORT_T, port);
        CONVERT_PARAMTOSTR(T_YT_ENABLE_T, enable);
    }

    /* execution func */
    ret = yt_port_jumbo_enable_set(unit, port, enable);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_port_jumbo_enable_get(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_port_t port;
    yt_enable_t pEnable;

    if(g_debug_switch)
        printf("is yt_port_jumbo_enable_get command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&port, 0, sizeof(yt_port_t));
    memset(&pEnable, 0, sizeof(yt_enable_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_PORT_T, port);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_PORT_T, port);
    }

    /* execution func */
    ret = yt_port_jumbo_enable_get(unit, port, &pEnable);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);/* print out parameters */
    if (!ret) {
        CONVERT_PARAMTOSTR(T_YT_ENABLE_T, pEnable);
    }

    return 0;
}
static inline int cmd_yt_port_cable_diag(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_port_t port;
    yt_port_cableDiag_t pCableStatus;

    if(g_debug_switch)
        printf("is yt_port_cable_diag command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&port, 0, sizeof(yt_port_t));
    memset(&pCableStatus, 0, sizeof(yt_port_cableDiag_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_PORT_T, port);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_PORT_T, port);
    }

    /* execution func */
    ret = yt_port_cable_diag(unit, port, &pCableStatus);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);/* print out parameters */
    if (!ret) {
        CONVERT_PARAMTOSTR(T_YT_PORT_CABLEDIAG_T, pCableStatus);
    }

    return 0;
}
static inline int cmd_yt_port_phyCrossover_mode_set(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_port_t port;
    yt_utp_crossover_mode_t mode;

    if(g_debug_switch)
        printf("is yt_port_phyCrossover_mode_set command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&port, 0, sizeof(yt_port_t));
    memset(&mode, 0, sizeof(yt_utp_crossover_mode_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_PORT_T, port);
    CONVERT_PARAMFROMSTR(T_YT_UTP_CROSSOVER_MODE_T, mode);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_PORT_T, port);
        CONVERT_PARAMTOSTR(T_YT_UTP_CROSSOVER_MODE_T, mode);
    }

    /* execution func */
    ret = yt_port_phyCrossover_mode_set(unit, port, mode);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_port_phyCrossover_mode_get(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_port_t port;
    yt_utp_crossover_mode_t pMode;

    if(g_debug_switch)
        printf("is yt_port_phyCrossover_mode_get command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&port, 0, sizeof(yt_port_t));
    memset(&pMode, 0, sizeof(yt_utp_crossover_mode_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_PORT_T, port);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_PORT_T, port);
    }

    /* execution func */
    ret = yt_port_phyCrossover_mode_get(unit, port, &pMode);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);/* print out parameters */
    if (!ret) {
        CONVERT_PARAMTOSTR(T_YT_UTP_CROSSOVER_MODE_T, pMode);
    }

    return 0;
}
static inline int cmd_yt_port_phyCrossover_status_get(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_port_t port;
    yt_utp_crossover_status_t pStatus;

    if(g_debug_switch)
        printf("is yt_port_phyCrossover_status_get command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&port, 0, sizeof(yt_port_t));
    memset(&pStatus, 0, sizeof(yt_utp_crossover_status_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_PORT_T, port);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_PORT_T, port);
    }

    /* execution func */
    ret = yt_port_phyCrossover_status_get(unit, port, &pStatus);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);/* print out parameters */
    if (!ret) {
        CONVERT_PARAMTOSTR(T_YT_UTP_CROSSOVER_STATUS_T, pStatus);
    }

    return 0;
}
static inline int cmd_yt_debug_phyTemplate_test_set(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_port_t port;
    yt_utp_template_testmode_t mode;

    if(g_debug_switch)
        printf("is yt_debug_phyTemplate_test_set command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&port, 0, sizeof(yt_port_t));
    memset(&mode, 0, sizeof(yt_utp_template_testmode_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_PORT_T, port);
    CONVERT_PARAMFROMSTR(T_YT_UTP_TEMPLATE_TESTMODE_T, mode);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_PORT_T, port);
        CONVERT_PARAMTOSTR(T_YT_UTP_TEMPLATE_TESTMODE_T, mode);
    }

    /* execution func */
    ret = yt_debug_phyTemplate_test_set(unit, port, mode);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_debug_phyLoopback_test_set(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_port_t port;
    yt_phy_loopback_mode_t mode;

    if(g_debug_switch)
        printf("is yt_debug_phyLoopback_test_set command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&port, 0, sizeof(yt_port_t));
    memset(&mode, 0, sizeof(yt_phy_loopback_mode_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_PORT_T, port);
    CONVERT_PARAMFROMSTR(T_YT_PHY_LOOPBACK_MODE_T, mode);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_PORT_T, port);
        CONVERT_PARAMTOSTR(T_YT_PHY_LOOPBACK_MODE_T, mode);
    }

    /* execution func */
    ret = yt_debug_phyLoopback_test_set(unit, port, mode);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_l2_init(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;

    if(g_debug_switch)
        printf("is yt_l2_init command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
    }

    /* execution func */
    ret = yt_l2_init(unit);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_l2_mcast_addr_add(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_vlan_t vid;
    yt_mac_addr_t mac_addr;
    yt_port_mask_t port_mask;

    if(g_debug_switch)
        printf("is yt_l2_mcast_addr_add command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&vid, 0, sizeof(yt_vlan_t));
    memset(&mac_addr, 0, sizeof(yt_mac_addr_t));
    memset(&port_mask, 0, sizeof(yt_port_mask_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_VLAN_T, vid);
    CONVERT_PARAMFROMSTR(T_YT_MAC_ADDR_T, mac_addr);
    CONVERT_PARAMFROMSTR(T_YT_PORT_MASK_T, port_mask);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_VLAN_T, vid);
        CONVERT_PARAMTOSTR(T_YT_MAC_ADDR_T, mac_addr);
        CONVERT_PARAMTOSTR(T_YT_PORT_MASK_T, port_mask);
    }

    /* execution func */
    ret = yt_l2_mcast_addr_add(unit, vid, mac_addr, port_mask);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_l2_fdb_ucast_addr_add(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_vlan_t vid;
    yt_mac_addr_t mac_addr;
    yt_port_t port;
    yt_bool_t isLag;

    if(g_debug_switch)
        printf("is yt_l2_fdb_ucast_addr_add command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&vid, 0, sizeof(yt_vlan_t));
    memset(&mac_addr, 0, sizeof(yt_mac_addr_t));
    memset(&port, 0, sizeof(yt_port_t));
    memset(&isLag, 0, sizeof(yt_bool_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_VLAN_T, vid);
    CONVERT_PARAMFROMSTR(T_YT_MAC_ADDR_T, mac_addr);
    CONVERT_PARAMFROMSTR(T_YT_PORT_T, port);
    CONVERT_PARAMFROMSTR(T_YT_BOOL_T, isLag);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_VLAN_T, vid);
        CONVERT_PARAMTOSTR(T_YT_MAC_ADDR_T, mac_addr);
        CONVERT_PARAMTOSTR(T_YT_PORT_T, port);
        CONVERT_PARAMTOSTR(T_YT_BOOL_T, isLag);
    }

    /* execution func */
    ret = yt_l2_fdb_ucast_addr_add(unit, vid, mac_addr, port, isLag);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_l2_fdb_ucast_addr_del(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_vlan_t vid;
    yt_mac_addr_t mac_addr;

    if(g_debug_switch)
        printf("is yt_l2_fdb_ucast_addr_del command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&vid, 0, sizeof(yt_vlan_t));
    memset(&mac_addr, 0, sizeof(yt_mac_addr_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_VLAN_T, vid);
    CONVERT_PARAMFROMSTR(T_YT_MAC_ADDR_T, mac_addr);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_VLAN_T, vid);
        CONVERT_PARAMTOSTR(T_YT_MAC_ADDR_T, mac_addr);
    }

    /* execution func */
    ret = yt_l2_fdb_ucast_addr_del(unit, vid, mac_addr);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_l2_mcast_addr_del(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_vlan_t vid;
    yt_mac_addr_t mac_addr;

    if(g_debug_switch)
        printf("is yt_l2_mcast_addr_del command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&vid, 0, sizeof(yt_vlan_t));
    memset(&mac_addr, 0, sizeof(yt_mac_addr_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_VLAN_T, vid);
    CONVERT_PARAMFROMSTR(T_YT_MAC_ADDR_T, mac_addr);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_VLAN_T, vid);
        CONVERT_PARAMTOSTR(T_YT_MAC_ADDR_T, mac_addr);
    }

    /* execution func */
    ret = yt_l2_mcast_addr_del(unit, vid, mac_addr);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_l2_fdb_linkdownFlush_en_set(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_enable_t enable;

    if(g_debug_switch)
        printf("is yt_l2_fdb_linkdownFlush_en_set command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&enable, 0, sizeof(yt_enable_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_ENABLE_T, enable);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_ENABLE_T, enable);
    }

    /* execution func */
    ret = yt_l2_fdb_linkdownFlush_en_set(unit, enable);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_l2_fdb_linkdownFlush_en_get(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_enable_t pEnable;

    if(g_debug_switch)
        printf("is yt_l2_fdb_linkdownFlush_en_get command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&pEnable, 0, sizeof(yt_enable_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
    }

    /* execution func */
    ret = yt_l2_fdb_linkdownFlush_en_get(unit, &pEnable);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);/* print out parameters */
    if (!ret) {
        CONVERT_PARAMTOSTR(T_YT_ENABLE_T, pEnable);
    }

    return 0;
}
static inline int cmd_yt_l2_fdb_all_ucast_flush(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;

    if(g_debug_switch)
        printf("is yt_l2_fdb_all_ucast_flush command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
    }

    /* execution func */
    ret = yt_l2_fdb_all_ucast_flush(unit);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_l2_fdb_port_ucast_flush(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_port_t port;

    if(g_debug_switch)
        printf("is yt_l2_fdb_port_ucast_flush command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&port, 0, sizeof(yt_port_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_PORT_T, port);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_PORT_T, port);
    }

    /* execution func */
    ret = yt_l2_fdb_port_ucast_flush(unit, port);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_l2_fdb_vlan_ucast_flush(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_vlan_t vid;

    if(g_debug_switch)
        printf("is yt_l2_fdb_vlan_ucast_flush command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&vid, 0, sizeof(yt_vlan_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_VLAN_T, vid);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_VLAN_T, vid);
    }

    /* execution func */
    ret = yt_l2_fdb_vlan_ucast_flush(unit, vid);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_l2_all_mcast_flush(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;

    if(g_debug_switch)
        printf("is yt_l2_all_mcast_flush command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
    }

    /* execution func */
    ret = yt_l2_all_mcast_flush(unit);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_l2_port_mcast_flush(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_port_t port;

    if(g_debug_switch)
        printf("is yt_l2_port_mcast_flush command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&port, 0, sizeof(yt_port_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_PORT_T, port);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_PORT_T, port);
    }

    /* execution func */
    ret = yt_l2_port_mcast_flush(unit, port);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_l2_vlan_mcast_flush(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_vlan_t vid;

    if(g_debug_switch)
        printf("is yt_l2_vlan_mcast_flush command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&vid, 0, sizeof(yt_vlan_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_VLAN_T, vid);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_VLAN_T, vid);
    }

    /* execution func */
    ret = yt_l2_vlan_mcast_flush(unit, vid);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_l2_fdb_type_get(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_vlan_t vid;
    yt_mac_addr_t mac_addr;
    yt_l2_fdb_type_t ptype;

    if(g_debug_switch)
        printf("is yt_l2_fdb_type_get command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&vid, 0, sizeof(yt_vlan_t));
    memset(&mac_addr, 0, sizeof(yt_mac_addr_t));
    memset(&ptype, 0, sizeof(yt_l2_fdb_type_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_VLAN_T, vid);
    CONVERT_PARAMFROMSTR(T_YT_MAC_ADDR_T, mac_addr);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_VLAN_T, vid);
        CONVERT_PARAMTOSTR(T_YT_MAC_ADDR_T, mac_addr);
    }

    /* execution func */
    ret = yt_l2_fdb_type_get(unit, vid, mac_addr, &ptype);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);/* print out parameters */
    if (!ret) {
        CONVERT_PARAMTOSTR(T_YT_L2_FDB_TYPE_T, ptype);
    }

    return 0;
}
static inline int cmd_yt_l2_port_learnlimit_en_set(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_port_t port;
    yt_enable_t enable;

    if(g_debug_switch)
        printf("is yt_l2_port_learnlimit_en_set command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&port, 0, sizeof(yt_port_t));
    memset(&enable, 0, sizeof(yt_enable_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_PORT_T, port);
    CONVERT_PARAMFROMSTR(T_YT_ENABLE_T, enable);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_PORT_T, port);
        CONVERT_PARAMTOSTR(T_YT_ENABLE_T, enable);
    }

    /* execution func */
    ret = yt_l2_port_learnlimit_en_set(unit, port, enable);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_l2_port_learnlimit_en_get(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_port_t port;
    yt_enable_t pEnable;

    if(g_debug_switch)
        printf("is yt_l2_port_learnlimit_en_get command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&port, 0, sizeof(yt_port_t));
    memset(&pEnable, 0, sizeof(yt_enable_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_PORT_T, port);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_PORT_T, port);
    }

    /* execution func */
    ret = yt_l2_port_learnlimit_en_get(unit, port, &pEnable);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);/* print out parameters */
    if (!ret) {
        CONVERT_PARAMTOSTR(T_YT_ENABLE_T, pEnable);
    }

    return 0;
}
static inline int cmd_yt_l2_port_learnlimit_cnt_set(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_port_t port;
    uint32_t maxcnt;

    if(g_debug_switch)
        printf("is yt_l2_port_learnlimit_cnt_set command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&port, 0, sizeof(yt_port_t));
    memset(&maxcnt, 0, sizeof(uint32_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_PORT_T, port);
    CONVERT_PARAMFROMSTR(T_UINT32_T, maxcnt);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_PORT_T, port);
        CONVERT_PARAMTOSTR(T_UINT32_T, maxcnt);
    }

    /* execution func */
    ret = yt_l2_port_learnlimit_cnt_set(unit, port, maxcnt);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_l2_port_learnlimit_cnt_get(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_port_t port;
    uint32_t pmaxcnt;

    if(g_debug_switch)
        printf("is yt_l2_port_learnlimit_cnt_get command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&port, 0, sizeof(yt_port_t));
    memset(&pmaxcnt, 0, sizeof(uint32_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_PORT_T, port);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_PORT_T, port);
    }

    /* execution func */
    ret = yt_l2_port_learnlimit_cnt_get(unit, port, &pmaxcnt);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);/* print out parameters */
    if (!ret) {
        CONVERT_PARAMTOSTR(T_UINT32_T, pmaxcnt);
    }

    return 0;
}
static inline int cmd_yt_l2_port_learnlimit_exceed_drop_set(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_port_t port;
    yt_enable_t enable;

    if(g_debug_switch)
        printf("is yt_l2_port_learnlimit_exceed_drop_set command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&port, 0, sizeof(yt_port_t));
    memset(&enable, 0, sizeof(yt_enable_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_PORT_T, port);
    CONVERT_PARAMFROMSTR(T_YT_ENABLE_T, enable);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_PORT_T, port);
        CONVERT_PARAMTOSTR(T_YT_ENABLE_T, enable);
    }

    /* execution func */
    ret = yt_l2_port_learnlimit_exceed_drop_set(unit, port, enable);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_l2_port_learnlimit_exceed_drop_get(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_port_t port;
    yt_enable_t pEnable;

    if(g_debug_switch)
        printf("is yt_l2_port_learnlimit_exceed_drop_get command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&port, 0, sizeof(yt_port_t));
    memset(&pEnable, 0, sizeof(yt_enable_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_PORT_T, port);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_PORT_T, port);
    }

    /* execution func */
    ret = yt_l2_port_learnlimit_exceed_drop_get(unit, port, &pEnable);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);/* print out parameters */
    if (!ret) {
        CONVERT_PARAMTOSTR(T_YT_ENABLE_T, pEnable);
    }

    return 0;
}
static inline int cmd_yt_l2_system_learnlimit_en_set(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_enable_t enable;

    if(g_debug_switch)
        printf("is yt_l2_system_learnlimit_en_set command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&enable, 0, sizeof(yt_enable_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_ENABLE_T, enable);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_ENABLE_T, enable);
    }

    /* execution func */
    ret = yt_l2_system_learnlimit_en_set(unit, enable);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_l2_system_learnlimit_en_get(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_enable_t pEnable;

    if(g_debug_switch)
        printf("is yt_l2_system_learnlimit_en_get command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&pEnable, 0, sizeof(yt_enable_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
    }

    /* execution func */
    ret = yt_l2_system_learnlimit_en_get(unit, &pEnable);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);/* print out parameters */
    if (!ret) {
        CONVERT_PARAMTOSTR(T_YT_ENABLE_T, pEnable);
    }

    return 0;
}
static inline int cmd_yt_l2_system_learnlimit_cnt_set(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    uint32_t maxcnt;

    if(g_debug_switch)
        printf("is yt_l2_system_learnlimit_cnt_set command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&maxcnt, 0, sizeof(uint32_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_UINT32_T, maxcnt);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_UINT32_T, maxcnt);
    }

    /* execution func */
    ret = yt_l2_system_learnlimit_cnt_set(unit, maxcnt);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_l2_system_learnlimit_cnt_get(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    uint32_t pmaxcnt;

    if(g_debug_switch)
        printf("is yt_l2_system_learnlimit_cnt_get command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&pmaxcnt, 0, sizeof(uint32_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
    }

    /* execution func */
    ret = yt_l2_system_learnlimit_cnt_get(unit, &pmaxcnt);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);/* print out parameters */
    if (!ret) {
        CONVERT_PARAMTOSTR(T_UINT32_T, pmaxcnt);
    }

    return 0;
}
static inline int cmd_yt_l2_system_learnlimit_exceed_drop_set(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_enable_t enable;

    if(g_debug_switch)
        printf("is yt_l2_system_learnlimit_exceed_drop_set command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&enable, 0, sizeof(yt_enable_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_ENABLE_T, enable);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_ENABLE_T, enable);
    }

    /* execution func */
    ret = yt_l2_system_learnlimit_exceed_drop_set(unit, enable);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_l2_system_learnlimit_exceed_drop_get(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_enable_t pEnable;

    if(g_debug_switch)
        printf("is yt_l2_system_learnlimit_exceed_drop_get command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&pEnable, 0, sizeof(yt_enable_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
    }

    /* execution func */
    ret = yt_l2_system_learnlimit_exceed_drop_get(unit, &pEnable);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);/* print out parameters */
    if (!ret) {
        CONVERT_PARAMTOSTR(T_YT_ENABLE_T, pEnable);
    }

    return 0;
}
static inline int cmd_yt_l2_fdb_drop_sa_set(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_vlan_t vid;
    yt_mac_addr_t mac_addr;
    yt_enable_t enable;

    if(g_debug_switch)
        printf("is yt_l2_fdb_drop_sa_set command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&vid, 0, sizeof(yt_vlan_t));
    memset(&mac_addr, 0, sizeof(yt_mac_addr_t));
    memset(&enable, 0, sizeof(yt_enable_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_VLAN_T, vid);
    CONVERT_PARAMFROMSTR(T_YT_MAC_ADDR_T, mac_addr);
    CONVERT_PARAMFROMSTR(T_YT_ENABLE_T, enable);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_VLAN_T, vid);
        CONVERT_PARAMTOSTR(T_YT_MAC_ADDR_T, mac_addr);
        CONVERT_PARAMTOSTR(T_YT_ENABLE_T, enable);
    }

    /* execution func */
    ret = yt_l2_fdb_drop_sa_set(unit, vid, mac_addr, enable);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_l2_fdb_drop_sa_get(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_vlan_t vid;
    yt_mac_addr_t mac_addr;
    yt_enable_t pEnable;

    if(g_debug_switch)
        printf("is yt_l2_fdb_drop_sa_get command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&vid, 0, sizeof(yt_vlan_t));
    memset(&mac_addr, 0, sizeof(yt_mac_addr_t));
    memset(&pEnable, 0, sizeof(yt_enable_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_VLAN_T, vid);
    CONVERT_PARAMFROMSTR(T_YT_MAC_ADDR_T, mac_addr);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_VLAN_T, vid);
        CONVERT_PARAMTOSTR(T_YT_MAC_ADDR_T, mac_addr);
    }

    /* execution func */
    ret = yt_l2_fdb_drop_sa_get(unit, vid, mac_addr, &pEnable);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);/* print out parameters */
    if (!ret) {
        CONVERT_PARAMTOSTR(T_YT_ENABLE_T, pEnable);
    }

    return 0;
}
static inline int cmd_yt_l2_fdb_drop_da_set(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_vlan_t vid;
    yt_mac_addr_t mac_addr;
    yt_enable_t enable;

    if(g_debug_switch)
        printf("is yt_l2_fdb_drop_da_set command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&vid, 0, sizeof(yt_vlan_t));
    memset(&mac_addr, 0, sizeof(yt_mac_addr_t));
    memset(&enable, 0, sizeof(yt_enable_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_VLAN_T, vid);
    CONVERT_PARAMFROMSTR(T_YT_MAC_ADDR_T, mac_addr);
    CONVERT_PARAMFROMSTR(T_YT_ENABLE_T, enable);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_VLAN_T, vid);
        CONVERT_PARAMTOSTR(T_YT_MAC_ADDR_T, mac_addr);
        CONVERT_PARAMTOSTR(T_YT_ENABLE_T, enable);
    }

    /* execution func */
    ret = yt_l2_fdb_drop_da_set(unit, vid, mac_addr, enable);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_l2_fdb_drop_da_get(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_vlan_t vid;
    yt_mac_addr_t mac_addr;
    yt_enable_t pEnable;

    if(g_debug_switch)
        printf("is yt_l2_fdb_drop_da_get command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&vid, 0, sizeof(yt_vlan_t));
    memset(&mac_addr, 0, sizeof(yt_mac_addr_t));
    memset(&pEnable, 0, sizeof(yt_enable_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_VLAN_T, vid);
    CONVERT_PARAMFROMSTR(T_YT_MAC_ADDR_T, mac_addr);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_VLAN_T, vid);
        CONVERT_PARAMTOSTR(T_YT_MAC_ADDR_T, mac_addr);
    }

    /* execution func */
    ret = yt_l2_fdb_drop_da_get(unit, vid, mac_addr, &pEnable);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);/* print out parameters */
    if (!ret) {
        CONVERT_PARAMTOSTR(T_YT_ENABLE_T, pEnable);
    }

    return 0;
}
static inline int cmd_yt_l2_fdb_copy2cpu_set(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_vlan_t vid;
    yt_mac_addr_t mac_addr;
    yt_enable_t enable;

    if(g_debug_switch)
        printf("is yt_l2_fdb_copy2cpu_set command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&vid, 0, sizeof(yt_vlan_t));
    memset(&mac_addr, 0, sizeof(yt_mac_addr_t));
    memset(&enable, 0, sizeof(yt_enable_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_VLAN_T, vid);
    CONVERT_PARAMFROMSTR(T_YT_MAC_ADDR_T, mac_addr);
    CONVERT_PARAMFROMSTR(T_YT_ENABLE_T, enable);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_VLAN_T, vid);
        CONVERT_PARAMTOSTR(T_YT_MAC_ADDR_T, mac_addr);
        CONVERT_PARAMTOSTR(T_YT_ENABLE_T, enable);
    }

    /* execution func */
    ret = yt_l2_fdb_copy2cpu_set(unit, vid, mac_addr, enable);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_l2_fdb_copy2cpu_get(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_vlan_t vid;
    yt_mac_addr_t mac_addr;
    yt_enable_t pEnable;

    if(g_debug_switch)
        printf("is yt_l2_fdb_copy2cpu_get command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&vid, 0, sizeof(yt_vlan_t));
    memset(&mac_addr, 0, sizeof(yt_mac_addr_t));
    memset(&pEnable, 0, sizeof(yt_enable_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_VLAN_T, vid);
    CONVERT_PARAMFROMSTR(T_YT_MAC_ADDR_T, mac_addr);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_VLAN_T, vid);
        CONVERT_PARAMTOSTR(T_YT_MAC_ADDR_T, mac_addr);
    }

    /* execution func */
    ret = yt_l2_fdb_copy2cpu_get(unit, vid, mac_addr, &pEnable);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);/* print out parameters */
    if (!ret) {
        CONVERT_PARAMTOSTR(T_YT_ENABLE_T, pEnable);
    }

    return 0;
}
static inline int cmd_yt_l2_filter_mcast_set(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_port_mask_t port_mask;

    if(g_debug_switch)
        printf("is yt_l2_filter_mcast_set command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&port_mask, 0, sizeof(yt_port_mask_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_PORT_MASK_T, port_mask);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_PORT_MASK_T, port_mask);
    }

    /* execution func */
    ret = yt_l2_filter_mcast_set(unit, port_mask);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_l2_filter_mcast_get(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_port_mask_t pport_mask;

    if(g_debug_switch)
        printf("is yt_l2_filter_mcast_get command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&pport_mask, 0, sizeof(yt_port_mask_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
    }

    /* execution func */
    ret = yt_l2_filter_mcast_get(unit, &pport_mask);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);/* print out parameters */
    if (!ret) {
        CONVERT_PARAMTOSTR(T_YT_PORT_MASK_T, pport_mask);
    }

    return 0;
}
static inline int cmd_yt_l2_filter_bcast_set(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_port_mask_t port_mask;

    if(g_debug_switch)
        printf("is yt_l2_filter_bcast_set command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&port_mask, 0, sizeof(yt_port_mask_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_PORT_MASK_T, port_mask);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_PORT_MASK_T, port_mask);
    }

    /* execution func */
    ret = yt_l2_filter_bcast_set(unit, port_mask);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_l2_filter_bcast_get(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_port_mask_t pport_mask;

    if(g_debug_switch)
        printf("is yt_l2_filter_bcast_get command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&pport_mask, 0, sizeof(yt_port_mask_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
    }

    /* execution func */
    ret = yt_l2_filter_bcast_get(unit, &pport_mask);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);/* print out parameters */
    if (!ret) {
        CONVERT_PARAMTOSTR(T_YT_PORT_MASK_T, pport_mask);
    }

    return 0;
}
static inline int cmd_yt_l2_filter_unknown_ucast_set(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_port_mask_t port_mask;

    if(g_debug_switch)
        printf("is yt_l2_filter_unknown_ucast_set command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&port_mask, 0, sizeof(yt_port_mask_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_PORT_MASK_T, port_mask);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_PORT_MASK_T, port_mask);
    }

    /* execution func */
    ret = yt_l2_filter_unknown_ucast_set(unit, port_mask);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_l2_filter_unknown_ucast_get(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_port_mask_t pport_mask;

    if(g_debug_switch)
        printf("is yt_l2_filter_unknown_ucast_get command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&pport_mask, 0, sizeof(yt_port_mask_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
    }

    /* execution func */
    ret = yt_l2_filter_unknown_ucast_get(unit, &pport_mask);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);/* print out parameters */
    if (!ret) {
        CONVERT_PARAMTOSTR(T_YT_PORT_MASK_T, pport_mask);
    }

    return 0;
}
static inline int cmd_yt_l2_filter_unknown_mcast_set(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_port_mask_t port_mask;

    if(g_debug_switch)
        printf("is yt_l2_filter_unknown_mcast_set command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&port_mask, 0, sizeof(yt_port_mask_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_PORT_MASK_T, port_mask);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_PORT_MASK_T, port_mask);
    }

    /* execution func */
    ret = yt_l2_filter_unknown_mcast_set(unit, port_mask);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_l2_filter_unknown_mcast_get(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_port_mask_t pport_mask;

    if(g_debug_switch)
        printf("is yt_l2_filter_unknown_mcast_get command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&pport_mask, 0, sizeof(yt_port_mask_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
    }

    /* execution func */
    ret = yt_l2_filter_unknown_mcast_get(unit, &pport_mask);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);/* print out parameters */
    if (!ret) {
        CONVERT_PARAMTOSTR(T_YT_PORT_MASK_T, pport_mask);
    }

    return 0;
}
static inline int cmd_yt_l2_rma_bypass_unknown_mcast_filter_set(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_enable_t enable;

    if(g_debug_switch)
        printf("is yt_l2_rma_bypass_unknown_mcast_filter_set command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&enable, 0, sizeof(yt_enable_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_ENABLE_T, enable);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_ENABLE_T, enable);
    }

    /* execution func */
    ret = yt_l2_rma_bypass_unknown_mcast_filter_set(unit, enable);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_l2_rma_bypass_unknown_mcast_filter_get(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_enable_t pEnable;

    if(g_debug_switch)
        printf("is yt_l2_rma_bypass_unknown_mcast_filter_get command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&pEnable, 0, sizeof(yt_enable_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
    }

    /* execution func */
    ret = yt_l2_rma_bypass_unknown_mcast_filter_get(unit, &pEnable);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);/* print out parameters */
    if (!ret) {
        CONVERT_PARAMTOSTR(T_YT_ENABLE_T, pEnable);
    }

    return 0;
}
static inline int cmd_yt_l2_fdb_port_uc_cnt_get(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_port_t port;
    uint32 pcnt;

    if(g_debug_switch)
        printf("is yt_l2_fdb_port_uc_cnt_get command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&port, 0, sizeof(yt_port_t));
    memset(&pcnt, 0, sizeof(uint32));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_PORT_T, port);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_PORT_T, port);
    }

    /* execution func */
    ret = yt_l2_fdb_port_uc_cnt_get(unit, port, &pcnt);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);/* print out parameters */
    if (!ret) {
        CONVERT_PARAMTOSTR(T_UINT32, pcnt);
    }

    return 0;
}
static inline int cmd_yt_l2_fdb_lag_uc_cnt_get(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    uint8_t groupid;
    uint32_t pcnt;

    if(g_debug_switch)
        printf("is yt_l2_fdb_lag_uc_cnt_get command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&groupid, 0, sizeof(uint8_t));
    memset(&pcnt, 0, sizeof(uint32_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_UINT8_T, groupid);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_UINT8_T, groupid);
    }

    /* execution func */
    ret = yt_l2_fdb_lag_uc_cnt_get(unit, groupid, &pcnt);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);/* print out parameters */
    if (!ret) {
        CONVERT_PARAMTOSTR(T_UINT32_T, pcnt);
    }

    return 0;
}
static inline int cmd_yt_l2_lag_learnlimit_en_set(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    uint8_t groupid;
    yt_enable_t enable;

    if(g_debug_switch)
        printf("is yt_l2_lag_learnlimit_en_set command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&groupid, 0, sizeof(uint8_t));
    memset(&enable, 0, sizeof(yt_enable_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_UINT8_T, groupid);
    CONVERT_PARAMFROMSTR(T_YT_ENABLE_T, enable);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_UINT8_T, groupid);
        CONVERT_PARAMTOSTR(T_YT_ENABLE_T, enable);
    }

    /* execution func */
    ret = yt_l2_lag_learnlimit_en_set(unit, groupid, enable);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_l2_lag_learnlimit_en_get(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    uint8_t groupid;
    yt_enable_t pEnable;

    if(g_debug_switch)
        printf("is yt_l2_lag_learnlimit_en_get command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&groupid, 0, sizeof(uint8_t));
    memset(&pEnable, 0, sizeof(yt_enable_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_UINT8_T, groupid);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_UINT8_T, groupid);
    }

    /* execution func */
    ret = yt_l2_lag_learnlimit_en_get(unit, groupid, &pEnable);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);/* print out parameters */
    if (!ret) {
        CONVERT_PARAMTOSTR(T_YT_ENABLE_T, pEnable);
    }

    return 0;
}
static inline int cmd_yt_l2_lag_learnlimit_cnt_set(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    uint8_t groupid;
    uint32_t maxcnt;

    if(g_debug_switch)
        printf("is yt_l2_lag_learnlimit_cnt_set command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&groupid, 0, sizeof(uint8_t));
    memset(&maxcnt, 0, sizeof(uint32_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_UINT8_T, groupid);
    CONVERT_PARAMFROMSTR(T_UINT32_T, maxcnt);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_UINT8_T, groupid);
        CONVERT_PARAMTOSTR(T_UINT32_T, maxcnt);
    }

    /* execution func */
    ret = yt_l2_lag_learnlimit_cnt_set(unit, groupid, maxcnt);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_l2_lag_learnlimit_cnt_get(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    uint8_t groupid;
    uint32_t pmaxcnt;

    if(g_debug_switch)
        printf("is yt_l2_lag_learnlimit_cnt_get command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&groupid, 0, sizeof(uint8_t));
    memset(&pmaxcnt, 0, sizeof(uint32_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_UINT8_T, groupid);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_UINT8_T, groupid);
    }

    /* execution func */
    ret = yt_l2_lag_learnlimit_cnt_get(unit, groupid, &pmaxcnt);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);/* print out parameters */
    if (!ret) {
        CONVERT_PARAMTOSTR(T_UINT32_T, pmaxcnt);
    }

    return 0;
}
static inline int cmd_yt_l2_lag_learnlimit_exceed_drop_set(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    uint8_t groupid;
    yt_enable_t enable;

    if(g_debug_switch)
        printf("is yt_l2_lag_learnlimit_exceed_drop_set command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&groupid, 0, sizeof(uint8_t));
    memset(&enable, 0, sizeof(yt_enable_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_UINT8_T, groupid);
    CONVERT_PARAMFROMSTR(T_YT_ENABLE_T, enable);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_UINT8_T, groupid);
        CONVERT_PARAMTOSTR(T_YT_ENABLE_T, enable);
    }

    /* execution func */
    ret = yt_l2_lag_learnlimit_exceed_drop_set(unit, groupid, enable);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_l2_lag_learnlimit_exceed_drop_get(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    uint8_t groupid;
    yt_enable_t pEnable;

    if(g_debug_switch)
        printf("is yt_l2_lag_learnlimit_exceed_drop_get command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&groupid, 0, sizeof(uint8_t));
    memset(&pEnable, 0, sizeof(yt_enable_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_UINT8_T, groupid);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_UINT8_T, groupid);
    }

    /* execution func */
    ret = yt_l2_lag_learnlimit_exceed_drop_get(unit, groupid, &pEnable);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);/* print out parameters */
    if (!ret) {
        CONVERT_PARAMTOSTR(T_YT_ENABLE_T, pEnable);
    }

    return 0;
}
static inline int cmd_yt_l2_fdb_uc_cnt_get(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    uint32 pcnt;

    if(g_debug_switch)
        printf("is yt_l2_fdb_uc_cnt_get command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&pcnt, 0, sizeof(uint32));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
    }

    /* execution func */
    ret = yt_l2_fdb_uc_cnt_get(unit, &pcnt);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);/* print out parameters */
    if (!ret) {
        CONVERT_PARAMTOSTR(T_UINT32, pcnt);
    }

    return 0;
}
static inline int cmd_yt_l2_mc_cnt_get(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    uint32 pcnt;

    if(g_debug_switch)
        printf("is yt_l2_mc_cnt_get command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&pcnt, 0, sizeof(uint32));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
    }

    /* execution func */
    ret = yt_l2_mc_cnt_get(unit, &pcnt);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);/* print out parameters */
    if (!ret) {
        CONVERT_PARAMTOSTR(T_UINT32, pcnt);
    }

    return 0;
}
static inline int cmd_yt_l2_fdb_aging_port_en_set(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_port_t port;
    yt_enable_t enable;

    if(g_debug_switch)
        printf("is yt_l2_fdb_aging_port_en_set command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&port, 0, sizeof(yt_port_t));
    memset(&enable, 0, sizeof(yt_enable_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_PORT_T, port);
    CONVERT_PARAMFROMSTR(T_YT_ENABLE_T, enable);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_PORT_T, port);
        CONVERT_PARAMTOSTR(T_YT_ENABLE_T, enable);
    }

    /* execution func */
    ret = yt_l2_fdb_aging_port_en_set(unit, port, enable);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_l2_fdb_aging_port_en_get(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_port_t port;
    yt_enable_t pEnable;

    if(g_debug_switch)
        printf("is yt_l2_fdb_aging_port_en_get command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&port, 0, sizeof(yt_port_t));
    memset(&pEnable, 0, sizeof(yt_enable_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_PORT_T, port);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_PORT_T, port);
    }

    /* execution func */
    ret = yt_l2_fdb_aging_port_en_get(unit, port, &pEnable);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);/* print out parameters */
    if (!ret) {
        CONVERT_PARAMTOSTR(T_YT_ENABLE_T, pEnable);
    }

    return 0;
}
static inline int cmd_yt_l2_fdb_aging_time_set(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    uint32_t sec;

    if(g_debug_switch)
        printf("is yt_l2_fdb_aging_time_set command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&sec, 0, sizeof(uint32_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_UINT32_T, sec);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_UINT32_T, sec);
    }

    /* execution func */
    ret = yt_l2_fdb_aging_time_set(unit, sec);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_l2_fdb_aging_time_get(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    uint32_t psec;

    if(g_debug_switch)
        printf("is yt_l2_fdb_aging_time_get command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&psec, 0, sizeof(uint32_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
    }

    /* execution func */
    ret = yt_l2_fdb_aging_time_get(unit, &psec);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);/* print out parameters */
    if (!ret) {
        CONVERT_PARAMTOSTR(T_UINT32_T, psec);
    }

    return 0;
}
static inline int cmd_yt_l2_fdb_uc_withindex_get(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    uint16_t index;
    l2_ucastMacAddr_info_t pUcastMac;

    if(g_debug_switch)
        printf("is yt_l2_fdb_uc_withindex_get command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&index, 0, sizeof(uint16_t));
    memset(&pUcastMac, 0, sizeof(l2_ucastMacAddr_info_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_UINT16_T, index);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_UINT16_T, index);
    }

    /* execution func */
    ret = yt_l2_fdb_uc_withindex_get(unit, index, &pUcastMac);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);/* print out parameters */
    if (!ret) {
        CONVERT_PARAMTOSTR(T_L2_UCASTMACADDR_INFO_T, pUcastMac);
    }

    return 0;
}
static inline int cmd_yt_l2_fdb_uc_withMacAndVid_get(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_vlan_t vid;
    yt_mac_addr_t mac_addr;
    l2_ucastMacAddr_info_t pUcastMac;

    if(g_debug_switch)
        printf("is yt_l2_fdb_uc_withMacAndVid_get command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&vid, 0, sizeof(yt_vlan_t));
    memset(&mac_addr, 0, sizeof(yt_mac_addr_t));
    memset(&pUcastMac, 0, sizeof(l2_ucastMacAddr_info_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_VLAN_T, vid);
    CONVERT_PARAMFROMSTR(T_YT_MAC_ADDR_T, mac_addr);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_VLAN_T, vid);
        CONVERT_PARAMTOSTR(T_YT_MAC_ADDR_T, mac_addr);
    }

    /* execution func */
    ret = yt_l2_fdb_uc_withMacAndVid_get(unit, vid, mac_addr, &pUcastMac);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);/* print out parameters */
    if (!ret) {
        CONVERT_PARAMTOSTR(T_L2_UCASTMACADDR_INFO_T, pUcastMac);
    }

    return 0;
}
static inline int cmd_yt_l2_fdb_uc_withindex_getnext(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    uint16_t index;
    uint16_t pNext_index;
    l2_ucastMacAddr_info_t pUcastMac;

    if(g_debug_switch)
        printf("is yt_l2_fdb_uc_withindex_getnext command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&index, 0, sizeof(uint16_t));
    memset(&pNext_index, 0, sizeof(uint16_t));
    memset(&pUcastMac, 0, sizeof(l2_ucastMacAddr_info_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_UINT16_T, index);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_UINT16_T, index);
    }

    /* execution func */
    ret = yt_l2_fdb_uc_withindex_getnext(unit, index, &pNext_index, &pUcastMac);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);/* print out parameters */
    if (!ret) {
        CONVERT_PARAMTOSTR(T_UINT16_T, pNext_index);
        CONVERT_PARAMTOSTR(T_L2_UCASTMACADDR_INFO_T, pUcastMac);
    }

    return 0;
}
static inline int cmd_yt_l2_port_learn_en_set(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_port_t port;
    yt_enable_t enable;

    if(g_debug_switch)
        printf("is yt_l2_port_learn_en_set command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&port, 0, sizeof(yt_port_t));
    memset(&enable, 0, sizeof(yt_enable_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_PORT_T, port);
    CONVERT_PARAMFROMSTR(T_YT_ENABLE_T, enable);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_PORT_T, port);
        CONVERT_PARAMTOSTR(T_YT_ENABLE_T, enable);
    }

    /* execution func */
    ret = yt_l2_port_learn_en_set(unit, port, enable);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_l2_port_learn_en_get(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_port_t port;
    yt_enable_t pEnable;

    if(g_debug_switch)
        printf("is yt_l2_port_learn_en_get command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&port, 0, sizeof(yt_port_t));
    memset(&pEnable, 0, sizeof(yt_enable_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_PORT_T, port);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_PORT_T, port);
    }

    /* execution func */
    ret = yt_l2_port_learn_en_get(unit, port, &pEnable);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);/* print out parameters */
    if (!ret) {
        CONVERT_PARAMTOSTR(T_YT_ENABLE_T, pEnable);
    }

    return 0;
}
static inline int cmd_yt_loop_detect_enable_set(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_enable_t enable;

    if(g_debug_switch)
        printf("is yt_loop_detect_enable_set command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&enable, 0, sizeof(yt_enable_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_ENABLE_T, enable);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_ENABLE_T, enable);
    }

    /* execution func */
    ret = yt_loop_detect_enable_set(unit, enable);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_loop_detect_enable_get(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_enable_t pEnable;

    if(g_debug_switch)
        printf("is yt_loop_detect_enable_get command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&pEnable, 0, sizeof(yt_enable_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
    }

    /* execution func */
    ret = yt_loop_detect_enable_get(unit, &pEnable);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);/* print out parameters */
    if (!ret) {
        CONVERT_PARAMTOSTR(T_YT_ENABLE_T, pEnable);
    }

    return 0;
}
static inline int cmd_yt_loop_detect_tpid_set(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_tpid_t tpid;

    if(g_debug_switch)
        printf("is yt_loop_detect_tpid_set command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&tpid, 0, sizeof(yt_tpid_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_TPID_T, tpid);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_TPID_T, tpid);
    }

    /* execution func */
    ret = yt_loop_detect_tpid_set(unit, tpid);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_loop_detect_tpid_get(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_tpid_t pTpid;

    if(g_debug_switch)
        printf("is yt_loop_detect_tpid_get command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&pTpid, 0, sizeof(yt_tpid_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
    }

    /* execution func */
    ret = yt_loop_detect_tpid_get(unit, &pTpid);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);/* print out parameters */
    if (!ret) {
        CONVERT_PARAMTOSTR(T_YT_TPID_T, pTpid);
    }

    return 0;
}
static inline int cmd_yt_loop_detect_generate_way_set(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_generate_way_t way;

    if(g_debug_switch)
        printf("is yt_loop_detect_generate_way_set command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&way, 0, sizeof(yt_generate_way_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_GENERATE_WAY_T, way);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_GENERATE_WAY_T, way);
    }

    /* execution func */
    ret = yt_loop_detect_generate_way_set(unit, way);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_loop_detect_generate_way_get(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_generate_way_t pWay;

    if(g_debug_switch)
        printf("is yt_loop_detect_generate_way_get command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&pWay, 0, sizeof(yt_generate_way_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
    }

    /* execution func */
    ret = yt_loop_detect_generate_way_get(unit, &pWay);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);/* print out parameters */
    if (!ret) {
        CONVERT_PARAMTOSTR(T_YT_GENERATE_WAY_T, pWay);
    }

    return 0;
}
static inline int cmd_yt_loop_detect_unitID_set(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_local_id_t localID;
    yt_remote_id_t remoteID;

    if(g_debug_switch)
        printf("is yt_loop_detect_unitID_set command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&localID, 0, sizeof(yt_local_id_t));
    memset(&remoteID, 0, sizeof(yt_remote_id_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_LOCAL_ID_T, localID);
    CONVERT_PARAMFROMSTR(T_YT_REMOTE_ID_T, remoteID);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_LOCAL_ID_T, localID);
        CONVERT_PARAMTOSTR(T_YT_REMOTE_ID_T, remoteID);
    }

    /* execution func */
    ret = yt_loop_detect_unitID_set(unit, localID, remoteID);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_loop_detect_unitID_get(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_local_id_t pLocalID;
    yt_remote_id_t pRemoteID;

    if(g_debug_switch)
        printf("is yt_loop_detect_unitID_get command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&pLocalID, 0, sizeof(yt_local_id_t));
    memset(&pRemoteID, 0, sizeof(yt_remote_id_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
    }

    /* execution func */
    ret = yt_loop_detect_unitID_get(unit, &pLocalID, &pRemoteID);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);/* print out parameters */
    if (!ret) {
        CONVERT_PARAMTOSTR(T_YT_LOCAL_ID_T, pLocalID);
        CONVERT_PARAMTOSTR(T_YT_REMOTE_ID_T, pRemoteID);
    }

    return 0;
}
static inline int cmd_yt_mirror_port_set(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_port_t target_port;
    yt_port_mask_t rx_portmask;
    yt_port_mask_t tx_portmask;

    if(g_debug_switch)
        printf("is yt_mirror_port_set command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&target_port, 0, sizeof(yt_port_t));
    memset(&rx_portmask, 0, sizeof(yt_port_mask_t));
    memset(&tx_portmask, 0, sizeof(yt_port_mask_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_PORT_T, target_port);
    CONVERT_PARAMFROMSTR(T_YT_PORT_MASK_T, rx_portmask);
    CONVERT_PARAMFROMSTR(T_YT_PORT_MASK_T, tx_portmask);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_PORT_T, target_port);
        CONVERT_PARAMTOSTR(T_YT_PORT_MASK_T, rx_portmask);
        CONVERT_PARAMTOSTR(T_YT_PORT_MASK_T, tx_portmask);
    }

    /* execution func */
    ret = yt_mirror_port_set(unit, target_port, rx_portmask, tx_portmask);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_mirror_port_get(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_port_t p_target_port;
    yt_port_mask_t p_rx_portmask;
    yt_port_mask_t p_tx_portmask;

    if(g_debug_switch)
        printf("is yt_mirror_port_get command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&p_target_port, 0, sizeof(yt_port_t));
    memset(&p_rx_portmask, 0, sizeof(yt_port_mask_t));
    memset(&p_tx_portmask, 0, sizeof(yt_port_mask_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
    }

    /* execution func */
    ret = yt_mirror_port_get(unit, &p_target_port, &p_rx_portmask, &p_tx_portmask);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);/* print out parameters */
    if (!ret) {
        CONVERT_PARAMTOSTR(T_YT_PORT_T, p_target_port);
        CONVERT_PARAMTOSTR(T_YT_PORT_MASK_T, p_rx_portmask);
        CONVERT_PARAMTOSTR(T_YT_PORT_MASK_T, p_tx_portmask);
    }

    return 0;
}
static inline int cmd_yt_nic_rx_register(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_nic_rx_cb_f rx_cb;

    if(g_debug_switch)
        printf("is yt_nic_rx_register command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&rx_cb, 0, sizeof(yt_nic_rx_cb_f));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_NIC_RX_CB_F, rx_cb);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_NIC_RX_CB_F, rx_cb);
    }

    /* execution func */
    ret = yt_nic_rx_register(unit, rx_cb);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_nic_rx_handler(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;

    if(g_debug_switch)
        printf("is yt_nic_rx_handler command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
    }

    /* execution func */
    ret = yt_nic_rx_handler(unit);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_nic_tx(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_tx_pkt_t p_tx_pkt;
    uint8_t mem_free_flag;

    if(g_debug_switch)
        printf("is yt_nic_tx command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&p_tx_pkt, 0, sizeof(yt_tx_pkt_t));
    memset(&mem_free_flag, 0, sizeof(uint8_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_TX_PKT_T, p_tx_pkt);
    CONVERT_PARAMFROMSTR(T_UINT8_T, mem_free_flag);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_TX_PKT_T, p_tx_pkt);
        CONVERT_PARAMTOSTR(T_UINT8_T, mem_free_flag);
    }

    /* execution func */
    ret = yt_nic_tx(unit, &p_tx_pkt, mem_free_flag);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_nic_eth_buf_cnt_len_set(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    uint16_t buf_cnt;
    uint16_t tx_buf_cnt;
    uint16_t buf_len;

    if(g_debug_switch)
        printf("is yt_nic_eth_buf_cnt_len_set command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&buf_cnt, 0, sizeof(uint16_t));
    memset(&tx_buf_cnt, 0, sizeof(uint16_t));
    memset(&buf_len, 0, sizeof(uint16_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_UINT16_T, buf_cnt);
    CONVERT_PARAMFROMSTR(T_UINT16_T, tx_buf_cnt);
    CONVERT_PARAMFROMSTR(T_UINT16_T, buf_len);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_UINT16_T, buf_cnt);
        CONVERT_PARAMTOSTR(T_UINT16_T, tx_buf_cnt);
        CONVERT_PARAMTOSTR(T_UINT16_T, buf_len);
    }

    /* execution func */
    ret = yt_nic_eth_buf_cnt_len_set(unit, buf_cnt, tx_buf_cnt, buf_len);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_nic_eth_debug_flag_set(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    uint8_t debug_flag;

    if(g_debug_switch)
        printf("is yt_nic_eth_debug_flag_set command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&debug_flag, 0, sizeof(uint8_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_UINT8_T, debug_flag);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_UINT8_T, debug_flag);
    }

    /* execution func */
    ret = yt_nic_eth_debug_flag_set(unit, debug_flag);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_nic_eth_drv_init(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;

    if(g_debug_switch)
        printf("is yt_nic_eth_drv_init command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
    }

    /* execution func */
    ret = yt_nic_eth_drv_init(unit);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_nic_init(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;

    if(g_debug_switch)
        printf("is yt_nic_init command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
    }

    /* execution func */
    ret = yt_nic_init(unit);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_nic_cpuport_mode_set(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_cpuport_mode_t mode;

    if(g_debug_switch)
        printf("is yt_nic_cpuport_mode_set command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&mode, 0, sizeof(yt_cpuport_mode_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_CPUPORT_MODE_T, mode);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_CPUPORT_MODE_T, mode);
    }

    /* execution func */
    ret = yt_nic_cpuport_mode_set(unit, mode);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_nic_cpuport_mode_get(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_cpuport_mode_t pMode;

    if(g_debug_switch)
        printf("is yt_nic_cpuport_mode_get command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&pMode, 0, sizeof(yt_cpuport_mode_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
    }

    /* execution func */
    ret = yt_nic_cpuport_mode_get(unit, &pMode);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);/* print out parameters */
    if (!ret) {
        CONVERT_PARAMTOSTR(T_YT_CPUPORT_MODE_T, pMode);
    }

    return 0;
}
static inline int cmd_yt_nic_ext_cpuport_en_set(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_enable_t enable;

    if(g_debug_switch)
        printf("is yt_nic_ext_cpuport_en_set command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&enable, 0, sizeof(yt_enable_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_ENABLE_T, enable);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_ENABLE_T, enable);
    }

    /* execution func */
    ret = yt_nic_ext_cpuport_en_set(unit, enable);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_nic_ext_cpuport_en_get(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_enable_t pEnable;

    if(g_debug_switch)
        printf("is yt_nic_ext_cpuport_en_get command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&pEnable, 0, sizeof(yt_enable_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
    }

    /* execution func */
    ret = yt_nic_ext_cpuport_en_get(unit, &pEnable);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);/* print out parameters */
    if (!ret) {
        CONVERT_PARAMTOSTR(T_YT_ENABLE_T, pEnable);
    }

    return 0;
}
static inline int cmd_yt_nic_ext_cpuport_port_set(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_port_t port;

    if(g_debug_switch)
        printf("is yt_nic_ext_cpuport_port_set command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&port, 0, sizeof(yt_port_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_PORT_T, port);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_PORT_T, port);
    }

    /* execution func */
    ret = yt_nic_ext_cpuport_port_set(unit, port);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_nic_ext_cpuport_port_get(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_port_t pPort;

    if(g_debug_switch)
        printf("is yt_nic_ext_cpuport_port_get command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&pPort, 0, sizeof(yt_port_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
    }

    /* execution func */
    ret = yt_nic_ext_cpuport_port_get(unit, &pPort);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);/* print out parameters */
    if (!ret) {
        CONVERT_PARAMTOSTR(T_YT_PORT_T, pPort);
    }

    return 0;
}
static inline int cmd_yt_nic_cpuport_tagtpid_set(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    uint16_t tpid;

    if(g_debug_switch)
        printf("is yt_nic_cpuport_tagtpid_set command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&tpid, 0, sizeof(uint16_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_UINT16_T, tpid);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_UINT16_T, tpid);
    }

    /* execution func */
    ret = yt_nic_cpuport_tagtpid_set(unit, tpid);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_nic_cpuport_tagtpid_get(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    uint16_t pTpid;

    if(g_debug_switch)
        printf("is yt_nic_cpuport_tagtpid_get command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&pTpid, 0, sizeof(uint16_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
    }

    /* execution func */
    ret = yt_nic_cpuport_tagtpid_get(unit, &pTpid);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);/* print out parameters */
    if (!ret) {
        CONVERT_PARAMTOSTR(T_UINT16_T, pTpid);
    }

    return 0;
}
static inline int cmd_yt_nic_ext_cputag_en_set(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_enable_t enable;

    if(g_debug_switch)
        printf("is yt_nic_ext_cputag_en_set command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&enable, 0, sizeof(yt_enable_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_ENABLE_T, enable);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_ENABLE_T, enable);
    }

    /* execution func */
    ret = yt_nic_ext_cputag_en_set(unit, enable);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_nic_ext_cputag_en_get(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_enable_t pEnable;

    if(g_debug_switch)
        printf("is yt_nic_ext_cputag_en_get command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&pEnable, 0, sizeof(yt_enable_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
    }

    /* execution func */
    ret = yt_nic_ext_cputag_en_get(unit, &pEnable);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);/* print out parameters */
    if (!ret) {
        CONVERT_PARAMTOSTR(T_YT_ENABLE_T, pEnable);
    }

    return 0;
}
static inline int cmd_yt_port_isolation_set(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_port_t port;
    yt_port_mask_t iso_portmask;

    if(g_debug_switch)
        printf("is yt_port_isolation_set command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&port, 0, sizeof(yt_port_t));
    memset(&iso_portmask, 0, sizeof(yt_port_mask_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_PORT_T, port);
    CONVERT_PARAMFROMSTR(T_YT_PORT_MASK_T, iso_portmask);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_PORT_T, port);
        CONVERT_PARAMTOSTR(T_YT_PORT_MASK_T, iso_portmask);
    }

    /* execution func */
    ret = yt_port_isolation_set(unit, port, iso_portmask);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_port_isolation_get(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_port_t port;
    yt_port_mask_t pIso_portmask;

    if(g_debug_switch)
        printf("is yt_port_isolation_get command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&port, 0, sizeof(yt_port_t));
    memset(&pIso_portmask, 0, sizeof(yt_port_mask_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_PORT_T, port);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_PORT_T, port);
    }

    /* execution func */
    ret = yt_port_isolation_get(unit, port, &pIso_portmask);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);/* print out parameters */
    if (!ret) {
        CONVERT_PARAMTOSTR(T_YT_PORT_MASK_T, pIso_portmask);
    }

    return 0;
}
static inline int cmd_yt_rate_init(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;

    if(g_debug_switch)
        printf("is yt_rate_init command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
    }

    /* execution func */
    ret = yt_rate_init(unit);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_rate_igrBandwidthCtrlEnable_set(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_port_t port;
    yt_enable_t enable;

    if(g_debug_switch)
        printf("is yt_rate_igrBandwidthCtrlEnable_set command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&port, 0, sizeof(yt_port_t));
    memset(&enable, 0, sizeof(yt_enable_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_PORT_T, port);
    CONVERT_PARAMFROMSTR(T_YT_ENABLE_T, enable);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_PORT_T, port);
        CONVERT_PARAMTOSTR(T_YT_ENABLE_T, enable);
    }

    /* execution func */
    ret = yt_rate_igrBandwidthCtrlEnable_set(unit, port, enable);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_rate_igrBandwidthCtrlEnable_get(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_port_t port;
    yt_enable_t pEnable;

    if(g_debug_switch)
        printf("is yt_rate_igrBandwidthCtrlEnable_get command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&port, 0, sizeof(yt_port_t));
    memset(&pEnable, 0, sizeof(yt_enable_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_PORT_T, port);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_PORT_T, port);
    }

    /* execution func */
    ret = yt_rate_igrBandwidthCtrlEnable_get(unit, port, &pEnable);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);/* print out parameters */
    if (!ret) {
        CONVERT_PARAMTOSTR(T_YT_ENABLE_T, pEnable);
    }

    return 0;
}
static inline int cmd_yt_rate_igrBandwidthCtrlMode_set(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_port_t port;
    yt_port_rate_mode_t port_rate_mode;

    if(g_debug_switch)
        printf("is yt_rate_igrBandwidthCtrlMode_set command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&port, 0, sizeof(yt_port_t));
    memset(&port_rate_mode, 0, sizeof(yt_port_rate_mode_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_PORT_T, port);
    CONVERT_PARAMFROMSTR(T_YT_PORT_RATE_MODE_T, port_rate_mode);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_PORT_T, port);
        CONVERT_PARAMTOSTR(T_YT_PORT_RATE_MODE_T, port_rate_mode);
    }

    /* execution func */
    ret = yt_rate_igrBandwidthCtrlMode_set(unit, port, port_rate_mode);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_rate_igrBandwidthCtrlMode_get(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_port_t port;
    yt_port_rate_mode_t pPort_rate_mode;

    if(g_debug_switch)
        printf("is yt_rate_igrBandwidthCtrlMode_get command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&port, 0, sizeof(yt_port_t));
    memset(&pPort_rate_mode, 0, sizeof(yt_port_rate_mode_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_PORT_T, port);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_PORT_T, port);
    }

    /* execution func */
    ret = yt_rate_igrBandwidthCtrlMode_get(unit, port, &pPort_rate_mode);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);/* print out parameters */
    if (!ret) {
        CONVERT_PARAMTOSTR(T_YT_PORT_RATE_MODE_T, pPort_rate_mode);
    }

    return 0;
}
static inline int cmd_yt_rate_igrBandwidthCtrlRate_set(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_port_t port;
    uint32_t rate;

    if(g_debug_switch)
        printf("is yt_rate_igrBandwidthCtrlRate_set command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&port, 0, sizeof(yt_port_t));
    memset(&rate, 0, sizeof(uint32_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_PORT_T, port);
    CONVERT_PARAMFROMSTR(T_UINT32_T, rate);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_PORT_T, port);
        CONVERT_PARAMTOSTR(T_UINT32_T, rate);
    }

    /* execution func */
    ret = yt_rate_igrBandwidthCtrlRate_set(unit, port, rate);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_rate_igrBandwidthCtrlRate_get(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_port_t port;
    uint32_t pRate;

    if(g_debug_switch)
        printf("is yt_rate_igrBandwidthCtrlRate_get command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&port, 0, sizeof(yt_port_t));
    memset(&pRate, 0, sizeof(uint32_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_PORT_T, port);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_PORT_T, port);
    }

    /* execution func */
    ret = yt_rate_igrBandwidthCtrlRate_get(unit, port, &pRate);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);/* print out parameters */
    if (!ret) {
        CONVERT_PARAMTOSTR(T_UINT32_T, pRate);
    }

    return 0;
}
static inline int cmd_yt_rate_meter_vlan_enable_set(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_vlan_t vid;
    yt_meterid_t meter_id;
    yt_enable_t enable;

    if(g_debug_switch)
        printf("is yt_rate_meter_vlan_enable_set command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&vid, 0, sizeof(yt_vlan_t));
    memset(&meter_id, 0, sizeof(yt_meterid_t));
    memset(&enable, 0, sizeof(yt_enable_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_VLAN_T, vid);
    CONVERT_PARAMFROMSTR(T_YT_METERID_T, meter_id);
    CONVERT_PARAMFROMSTR(T_YT_ENABLE_T, enable);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_VLAN_T, vid);
        CONVERT_PARAMTOSTR(T_YT_METERID_T, meter_id);
        CONVERT_PARAMTOSTR(T_YT_ENABLE_T, enable);
    }

    /* execution func */
    ret = yt_rate_meter_vlan_enable_set(unit, vid, meter_id, enable);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_rate_meter_vlan_enable_get(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_vlan_t vid;
    yt_meterid_t pMeter_id;
    yt_enable_t pEnable;

    if(g_debug_switch)
        printf("is yt_rate_meter_vlan_enable_get command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&vid, 0, sizeof(yt_vlan_t));
    memset(&pMeter_id, 0, sizeof(yt_meterid_t));
    memset(&pEnable, 0, sizeof(yt_enable_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_VLAN_T, vid);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_VLAN_T, vid);
    }

    /* execution func */
    ret = yt_rate_meter_vlan_enable_get(unit, vid, &pMeter_id, &pEnable);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);/* print out parameters */
    if (!ret) {
        CONVERT_PARAMTOSTR(T_YT_METERID_T, pMeter_id);
        CONVERT_PARAMTOSTR(T_YT_ENABLE_T, pEnable);
    }

    return 0;
}
static inline int cmd_yt_rate_meter_enable_set(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_meterid_t meter_id;
    yt_enable_t enable;

    if(g_debug_switch)
        printf("is yt_rate_meter_enable_set command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&meter_id, 0, sizeof(yt_meterid_t));
    memset(&enable, 0, sizeof(yt_enable_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_METERID_T, meter_id);
    CONVERT_PARAMFROMSTR(T_YT_ENABLE_T, enable);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_METERID_T, meter_id);
        CONVERT_PARAMTOSTR(T_YT_ENABLE_T, enable);
    }

    /* execution func */
    ret = yt_rate_meter_enable_set(unit, meter_id, enable);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_rate_meter_enable_get(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_meterid_t meter_id;
    yt_enable_t pEnable;

    if(g_debug_switch)
        printf("is yt_rate_meter_enable_get command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&meter_id, 0, sizeof(yt_meterid_t));
    memset(&pEnable, 0, sizeof(yt_enable_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_METERID_T, meter_id);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_METERID_T, meter_id);
    }

    /* execution func */
    ret = yt_rate_meter_enable_get(unit, meter_id, &pEnable);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);/* print out parameters */
    if (!ret) {
        CONVERT_PARAMTOSTR(T_YT_ENABLE_T, pEnable);
    }

    return 0;
}
static inline int cmd_yt_rate_meter_mode_set(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_meterid_t meter_id;
    yt_rate_meter_mode_t mode;

    if(g_debug_switch)
        printf("is yt_rate_meter_mode_set command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&meter_id, 0, sizeof(yt_meterid_t));
    memset(&mode, 0, sizeof(yt_rate_meter_mode_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_METERID_T, meter_id);
    CONVERT_PARAMFROMSTR(T_YT_RATE_METER_MODE_T, mode);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_METERID_T, meter_id);
        CONVERT_PARAMTOSTR(T_YT_RATE_METER_MODE_T, mode);
    }

    /* execution func */
    ret = yt_rate_meter_mode_set(unit, meter_id, mode);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_rate_meter_mode_get(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_meterid_t meter_id;
    yt_rate_meter_mode_t pMode;

    if(g_debug_switch)
        printf("is yt_rate_meter_mode_get command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&meter_id, 0, sizeof(yt_meterid_t));
    memset(&pMode, 0, sizeof(yt_rate_meter_mode_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_METERID_T, meter_id);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_METERID_T, meter_id);
    }

    /* execution func */
    ret = yt_rate_meter_mode_get(unit, meter_id, &pMode);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);/* print out parameters */
    if (!ret) {
        CONVERT_PARAMTOSTR(T_YT_RATE_METER_MODE_T, pMode);
    }

    return 0;
}
static inline int cmd_yt_rate_meter_rate_set(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_meterid_t meter_id;
    yt_qos_two_rate_t rate;

    if(g_debug_switch)
        printf("is yt_rate_meter_rate_set command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&meter_id, 0, sizeof(yt_meterid_t));
    memset(&rate, 0, sizeof(yt_qos_two_rate_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_METERID_T, meter_id);
    CONVERT_PARAMFROMSTR(T_YT_QOS_TWO_RATE_T, rate);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_METERID_T, meter_id);
        CONVERT_PARAMTOSTR(T_YT_QOS_TWO_RATE_T, rate);
    }

    /* execution func */
    ret = yt_rate_meter_rate_set(unit, meter_id, rate);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_rate_meter_rate_get(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_meterid_t meter_id;
    yt_qos_two_rate_t pRate;

    if(g_debug_switch)
        printf("is yt_rate_meter_rate_get command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&meter_id, 0, sizeof(yt_meterid_t));
    memset(&pRate, 0, sizeof(yt_qos_two_rate_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_METERID_T, meter_id);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_METERID_T, meter_id);
    }

    /* execution func */
    ret = yt_rate_meter_rate_get(unit, meter_id, &pRate);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);/* print out parameters */
    if (!ret) {
        CONVERT_PARAMTOSTR(T_YT_QOS_TWO_RATE_T, pRate);
    }

    return 0;
}
static inline int cmd_yt_rate_shaping_port_enable_set(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_port_t port;
    yt_enable_t enable;

    if(g_debug_switch)
        printf("is yt_rate_shaping_port_enable_set command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&port, 0, sizeof(yt_port_t));
    memset(&enable, 0, sizeof(yt_enable_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_PORT_T, port);
    CONVERT_PARAMFROMSTR(T_YT_ENABLE_T, enable);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_PORT_T, port);
        CONVERT_PARAMTOSTR(T_YT_ENABLE_T, enable);
    }

    /* execution func */
    ret = yt_rate_shaping_port_enable_set(unit, port, enable);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_rate_shaping_port_enable_get(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_port_t port;
    yt_enable_t pEnable;

    if(g_debug_switch)
        printf("is yt_rate_shaping_port_enable_get command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&port, 0, sizeof(yt_port_t));
    memset(&pEnable, 0, sizeof(yt_enable_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_PORT_T, port);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_PORT_T, port);
    }

    /* execution func */
    ret = yt_rate_shaping_port_enable_get(unit, port, &pEnable);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);/* print out parameters */
    if (!ret) {
        CONVERT_PARAMTOSTR(T_YT_ENABLE_T, pEnable);
    }

    return 0;
}
static inline int cmd_yt_rate_shaping_port_mode_set(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_port_t port;
    yt_shaping_mode_t shaping_mode;

    if(g_debug_switch)
        printf("is yt_rate_shaping_port_mode_set command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&port, 0, sizeof(yt_port_t));
    memset(&shaping_mode, 0, sizeof(yt_shaping_mode_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_PORT_T, port);
    CONVERT_PARAMFROMSTR(T_YT_SHAPING_MODE_T, shaping_mode);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_PORT_T, port);
        CONVERT_PARAMTOSTR(T_YT_SHAPING_MODE_T, shaping_mode);
    }

    /* execution func */
    ret = yt_rate_shaping_port_mode_set(unit, port, shaping_mode);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_rate_shaping_port_mode_get(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_port_t port;
    yt_shaping_mode_t pShaping_mode;

    if(g_debug_switch)
        printf("is yt_rate_shaping_port_mode_get command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&port, 0, sizeof(yt_port_t));
    memset(&pShaping_mode, 0, sizeof(yt_shaping_mode_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_PORT_T, port);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_PORT_T, port);
    }

    /* execution func */
    ret = yt_rate_shaping_port_mode_get(unit, port, &pShaping_mode);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);/* print out parameters */
    if (!ret) {
        CONVERT_PARAMTOSTR(T_YT_SHAPING_MODE_T, pShaping_mode);
    }

    return 0;
}
static inline int cmd_yt_rate_shaping_port_rate_set(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_port_t port;
    uint32_t rate;

    if(g_debug_switch)
        printf("is yt_rate_shaping_port_rate_set command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&port, 0, sizeof(yt_port_t));
    memset(&rate, 0, sizeof(uint32_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_PORT_T, port);
    CONVERT_PARAMFROMSTR(T_UINT32_T, rate);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_PORT_T, port);
        CONVERT_PARAMTOSTR(T_UINT32_T, rate);
    }

    /* execution func */
    ret = yt_rate_shaping_port_rate_set(unit, port, rate);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_rate_shaping_port_rate_get(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_port_t port;
    uint32_t pRate;

    if(g_debug_switch)
        printf("is yt_rate_shaping_port_rate_get command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&port, 0, sizeof(yt_port_t));
    memset(&pRate, 0, sizeof(uint32_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_PORT_T, port);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_PORT_T, port);
    }

    /* execution func */
    ret = yt_rate_shaping_port_rate_get(unit, port, &pRate);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);/* print out parameters */
    if (!ret) {
        CONVERT_PARAMTOSTR(T_UINT32_T, pRate);
    }

    return 0;
}
static inline int cmd_yt_rate_shaping_queue_enable_set(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_qid_t qinfo;
    yt_enable_t cshap_en;
    yt_enable_t eshap_en;

    if(g_debug_switch)
        printf("is yt_rate_shaping_queue_enable_set command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&qinfo, 0, sizeof(yt_qid_t));
    memset(&cshap_en, 0, sizeof(yt_enable_t));
    memset(&eshap_en, 0, sizeof(yt_enable_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_QID_T, qinfo);
    CONVERT_PARAMFROMSTR(T_YT_ENABLE_T, cshap_en);
    CONVERT_PARAMFROMSTR(T_YT_ENABLE_T, eshap_en);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_QID_T, qinfo);
        CONVERT_PARAMTOSTR(T_YT_ENABLE_T, cshap_en);
        CONVERT_PARAMTOSTR(T_YT_ENABLE_T, eshap_en);
    }

    /* execution func */
    ret = yt_rate_shaping_queue_enable_set(unit, qinfo, cshap_en, eshap_en);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_rate_shaping_queue_enable_get(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_qid_t qinfo;
    yt_enable_t pCshap_en;
    yt_enable_t pEshap_en;

    if(g_debug_switch)
        printf("is yt_rate_shaping_queue_enable_get command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&qinfo, 0, sizeof(yt_qid_t));
    memset(&pCshap_en, 0, sizeof(yt_enable_t));
    memset(&pEshap_en, 0, sizeof(yt_enable_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_QID_T, qinfo);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_QID_T, qinfo);
    }

    /* execution func */
    ret = yt_rate_shaping_queue_enable_get(unit, qinfo, &pCshap_en, &pEshap_en);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);/* print out parameters */
    if (!ret) {
        CONVERT_PARAMTOSTR(T_YT_ENABLE_T, pCshap_en);
        CONVERT_PARAMTOSTR(T_YT_ENABLE_T, pEshap_en);
    }

    return 0;
}
static inline int cmd_yt_rate_shaping_queue_mode_set(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_qid_t qinfo;
    yt_shaping_mode_t shaping_mode;

    if(g_debug_switch)
        printf("is yt_rate_shaping_queue_mode_set command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&qinfo, 0, sizeof(yt_qid_t));
    memset(&shaping_mode, 0, sizeof(yt_shaping_mode_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_QID_T, qinfo);
    CONVERT_PARAMFROMSTR(T_YT_SHAPING_MODE_T, shaping_mode);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_QID_T, qinfo);
        CONVERT_PARAMTOSTR(T_YT_SHAPING_MODE_T, shaping_mode);
    }

    /* execution func */
    ret = yt_rate_shaping_queue_mode_set(unit, qinfo, shaping_mode);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_rate_shaping_queue_mode_get(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_qid_t qinfo;
    yt_shaping_mode_t pShaping_mode;

    if(g_debug_switch)
        printf("is yt_rate_shaping_queue_mode_get command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&qinfo, 0, sizeof(yt_qid_t));
    memset(&pShaping_mode, 0, sizeof(yt_shaping_mode_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_QID_T, qinfo);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_QID_T, qinfo);
    }

    /* execution func */
    ret = yt_rate_shaping_queue_mode_get(unit, qinfo, &pShaping_mode);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);/* print out parameters */
    if (!ret) {
        CONVERT_PARAMTOSTR(T_YT_SHAPING_MODE_T, pShaping_mode);
    }

    return 0;
}
static inline int cmd_yt_rate_shaping_queue_rate_set(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_qid_t qinfo;
    yt_qos_two_rate_t rate;

    if(g_debug_switch)
        printf("is yt_rate_shaping_queue_rate_set command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&qinfo, 0, sizeof(yt_qid_t));
    memset(&rate, 0, sizeof(yt_qos_two_rate_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_QID_T, qinfo);
    CONVERT_PARAMFROMSTR(T_YT_QOS_TWO_RATE_T, rate);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_QID_T, qinfo);
        CONVERT_PARAMTOSTR(T_YT_QOS_TWO_RATE_T, rate);
    }

    /* execution func */
    ret = yt_rate_shaping_queue_rate_set(unit, qinfo, rate);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_rate_shaping_queue_rate_get(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_qid_t qinfo;
    yt_qos_two_rate_t pRate;

    if(g_debug_switch)
        printf("is yt_rate_shaping_queue_rate_get command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&qinfo, 0, sizeof(yt_qid_t));
    memset(&pRate, 0, sizeof(yt_qos_two_rate_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_QID_T, qinfo);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_QID_T, qinfo);
    }

    /* execution func */
    ret = yt_rate_shaping_queue_rate_get(unit, qinfo, &pRate);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);/* print out parameters */
    if (!ret) {
        CONVERT_PARAMTOSTR(T_YT_QOS_TWO_RATE_T, pRate);
    }

    return 0;
}
static inline int cmd_yt_stat_mib_init(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;

    if(g_debug_switch)
        printf("is yt_stat_mib_init command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
    }

    /* execution func */
    ret = yt_stat_mib_init(unit);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_stat_mib_enable_set(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_enable_t enable;

    if(g_debug_switch)
        printf("is yt_stat_mib_enable_set command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&enable, 0, sizeof(yt_enable_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_ENABLE_T, enable);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_ENABLE_T, enable);
    }

    /* execution func */
    ret = yt_stat_mib_enable_set(unit, enable);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_stat_mib_enable_get(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_enable_t pEnable;

    if(g_debug_switch)
        printf("is yt_stat_mib_enable_get command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&pEnable, 0, sizeof(yt_enable_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
    }

    /* execution func */
    ret = yt_stat_mib_enable_get(unit, &pEnable);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);/* print out parameters */
    if (!ret) {
        CONVERT_PARAMTOSTR(T_YT_ENABLE_T, pEnable);
    }

    return 0;
}
static inline int cmd_yt_stat_mib_clear(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_port_t port;

    if(g_debug_switch)
        printf("is yt_stat_mib_clear command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&port, 0, sizeof(yt_port_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_PORT_T, port);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_PORT_T, port);
    }

    /* execution func */
    ret = yt_stat_mib_clear(unit, port);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_stat_mib_clear_all(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;

    if(g_debug_switch)
        printf("is yt_stat_mib_clear_all command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
    }

    /* execution func */
    ret = yt_stat_mib_clear_all(unit);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_stat_mib_port_get(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_port_t port;
    yt_stat_mib_port_cnt_t pCnt;

    if(g_debug_switch)
        printf("is yt_stat_mib_port_get command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&port, 0, sizeof(yt_port_t));
    memset(&pCnt, 0, sizeof(yt_stat_mib_port_cnt_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_YT_PORT_T, port);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_YT_PORT_T, port);
    }

    /* execution func */
    ret = yt_stat_mib_port_get(unit, port, &pCnt);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);/* print out parameters */
    if (!ret) {
        CONVERT_PARAMTOSTR(T_YT_STAT_MIB_PORT_CNT_T, pCnt);
    }

    return 0;
}
static inline int cmd_yt_stat_flow_enable_set(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    uint32_t flow_id;
    yt_enable_t enable;

    if(g_debug_switch)
        printf("is yt_stat_flow_enable_set command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&flow_id, 0, sizeof(uint32_t));
    memset(&enable, 0, sizeof(yt_enable_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_UINT32_T, flow_id);
    CONVERT_PARAMFROMSTR(T_YT_ENABLE_T, enable);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_UINT32_T, flow_id);
        CONVERT_PARAMTOSTR(T_YT_ENABLE_T, enable);
    }

    /* execution func */
    ret = yt_stat_flow_enable_set(unit, flow_id, enable);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_stat_flow_enable_get(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    uint32_t flow_id;
    yt_enable_t pEnable;

    if(g_debug_switch)
        printf("is yt_stat_flow_enable_get command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&flow_id, 0, sizeof(uint32_t));
    memset(&pEnable, 0, sizeof(yt_enable_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_UINT32_T, flow_id);
    CONVERT_PARAMFROMSTR(T_YT_ENABLE_T, pEnable);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_UINT32_T, flow_id);
        CONVERT_PARAMTOSTR(T_YT_ENABLE_T, pEnable);
    }

    /* execution func */
    ret = yt_stat_flow_enable_get(unit, flow_id, &pEnable);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_stat_flow_mode_set(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    uint32_t flow_id;
    yt_stat_type_t type;
    yt_stat_mode_t mode;

    if(g_debug_switch)
        printf("is yt_stat_flow_mode_set command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&flow_id, 0, sizeof(uint32_t));
    memset(&type, 0, sizeof(yt_stat_type_t));
    memset(&mode, 0, sizeof(yt_stat_mode_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_UINT32_T, flow_id);
    CONVERT_PARAMFROMSTR(T_YT_STAT_TYPE_T, type);
    CONVERT_PARAMFROMSTR(T_YT_STAT_MODE_T, mode);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_UINT32_T, flow_id);
        CONVERT_PARAMTOSTR(T_YT_STAT_TYPE_T, type);
        CONVERT_PARAMTOSTR(T_YT_STAT_MODE_T, mode);
    }

    /* execution func */
    ret = yt_stat_flow_mode_set(unit, flow_id, type, mode);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_stat_flow_mode_get(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    uint32_t flow_id;
    yt_stat_type_t pType;
    yt_stat_mode_t pMode;

    if(g_debug_switch)
        printf("is yt_stat_flow_mode_get command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&flow_id, 0, sizeof(uint32_t));
    memset(&pType, 0, sizeof(yt_stat_type_t));
    memset(&pMode, 0, sizeof(yt_stat_mode_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_UINT32_T, flow_id);
    CONVERT_PARAMFROMSTR(T_YT_STAT_TYPE_T, pType);
    CONVERT_PARAMFROMSTR(T_YT_STAT_MODE_T, pMode);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_UINT32_T, flow_id);
        CONVERT_PARAMTOSTR(T_YT_STAT_TYPE_T, pType);
        CONVERT_PARAMTOSTR(T_YT_STAT_MODE_T, pMode);
    }

    /* execution func */
    ret = yt_stat_flow_mode_get(unit, flow_id, &pType, &pMode);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_stat_flow_count_set(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    uint32_t flow_id;
    uint64 cnt;

    if(g_debug_switch)
        printf("is yt_stat_flow_count_set command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&flow_id, 0, sizeof(uint32_t));
    memset(&cnt, 0, sizeof(uint64));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_UINT32_T, flow_id);
    CONVERT_PARAMFROMSTR(T_UINT64, cnt);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_UINT32_T, flow_id);
        CONVERT_PARAMTOSTR(T_UINT64, cnt);
    }

    /* execution func */
    ret = yt_stat_flow_count_set(unit, flow_id, cnt);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_stat_flow_count_get(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    uint32_t flow_id;
    uint64 pCnt;

    if(g_debug_switch)
        printf("is yt_stat_flow_count_get command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&flow_id, 0, sizeof(uint32_t));
    memset(&pCnt, 0, sizeof(uint64));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_UINT32_T, flow_id);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_UINT32_T, flow_id);
    }

    /* execution func */
    ret = yt_stat_flow_count_get(unit, flow_id, &pCnt);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);/* print out parameters */
    if (!ret) {
        CONVERT_PARAMTOSTR(T_UINT64, pCnt);
    }

    return 0;
}
static inline int cmd_yt_sys_mac_get(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_mac_addr_t pSys_mac;

    if(g_debug_switch)
        printf("is yt_sys_mac_get command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&pSys_mac, 0, sizeof(yt_mac_addr_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
    }

    /* execution func */
    ret = yt_sys_mac_get(unit, &pSys_mac);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);/* print out parameters */
    if (!ret) {
        CONVERT_PARAMTOSTR(T_YT_MAC_ADDR_T, pSys_mac);
    }

    return 0;
}
static inline int cmd_yt_sys_chip_reset(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;

    if(g_debug_switch)
        printf("is yt_sys_chip_reset command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
    }

    /* execution func */
    ret = yt_sys_chip_reset(unit);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_sys_database_reset(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;

    if(g_debug_switch)
        printf("is yt_sys_database_reset command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
    }

    /* execution func */
    ret = yt_sys_database_reset(unit);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_sys_version_get(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    char pVerStr;

    if(g_debug_switch)
        printf("is yt_sys_version_get command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&pVerStr, 0, sizeof(char));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
    }

    /* execution func */
    ret = yt_sys_version_get(unit, &pVerStr);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);/* print out parameters */
    if (!ret) {
        CONVERT_PARAMTOSTR(T_CHAR, pVerStr);
    }

    return 0;
}
static inline int cmd_yt_sys_register_value_get(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    uint32_t regAddr;
    uint32_t pVal;

    if(g_debug_switch)
        printf("is yt_sys_register_value_get command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&regAddr, 0, sizeof(uint32_t));
    memset(&pVal, 0, sizeof(uint32_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_UINT32_T, regAddr);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_UINT32_T, regAddr);
    }

    /* execution func */
    ret = yt_sys_register_value_get(unit, regAddr, &pVal);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);/* print out parameters */
    if (!ret) {
        CONVERT_PARAMTOSTR(T_UINT32_T, pVal);
    }

    return 0;
}
static inline int cmd_yt_sys_register_value_set(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    uint32_t regAddr;
    uint32_t value;

    if(g_debug_switch)
        printf("is yt_sys_register_value_set command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&regAddr, 0, sizeof(uint32_t));
    memset(&value, 0, sizeof(uint32_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);
    CONVERT_PARAMFROMSTR(T_UINT32_T, regAddr);
    CONVERT_PARAMFROMSTR(T_UINT32_T, value);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
        CONVERT_PARAMTOSTR(T_UINT32_T, regAddr);
        CONVERT_PARAMTOSTR(T_UINT32_T, value);
    }

    /* execution func */
    ret = yt_sys_register_value_set(unit, regAddr, value);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);

    return 0;
}
static inline int cmd_yt_sys_chipInfo_get(int argc, char *argv[])
{
    int post = 0;
    /* define result */
    yt_ret_t ret;

    /* define parameters */
    yt_unit_t unit;
    yt_switch_chip_t pChip;

    if(g_debug_switch)
        printf("is yt_sys_chipInfo_get command!\n");

    /* init result */
    memset(&ret, 0, sizeof(yt_ret_t));

    /* init parameters */
    memset(&unit, 0, sizeof(yt_unit_t));
    memset(&pChip, 0, sizeof(yt_switch_chip_t));

    /* get parameters */
    CONVERT_PARAMFROMSTR(T_YT_UNIT_T, unit);

    if (g_debug_switch) {
        CONVERT_PARAMTOSTR(T_YT_UNIT_T, unit);
    }

    /* execution func */
    ret = yt_sys_chipInfo_get(unit, &pChip);

    /* print ret */
    CONVERT_PARAMTOSTR(T_YT_RET_T, ret);/* print out parameters */
    if (!ret) {
        CONVERT_PARAMTOSTR(T_YT_SWITCH_CHIP_T, pChip);
    }

    return 0;
}
