/*
 * Copyright (c) 2020 Clément Dommerc <clement.dommerc@gmail.com>
 * MIT License
 *
 */

#include "qlstatus.h"

void            free_wireless(void *data) {
    t_module    *module = data;
    t_wlan      *wlan = module->data;

    nl_socket_free(wlan->socket);
    free(wlan->lb_unk);
}

// Based on NetworkManager/src/platform/wifi/wifi-utils-nl80211.c
static uint32_t     nl80211_xbm_to_percent(int32_t xbm, int32_t divisor) {
    xbm /= divisor;
    if (xbm < NOISE_FLOOR_DBM) {
        xbm = NOISE_FLOOR_DBM;
    }
    if (xbm > SIGNAL_MAX_DBM) {
        xbm = SIGNAL_MAX_DBM;
    }
    return 100 - 70 * (((float)SIGNAL_MAX_DBM - (float)xbm) /
            ((float)SIGNAL_MAX_DBM - (float)NOISE_FLOOR_DBM));
}

// Based on NetworkManager/src/platform/wifi/wifi-utils-nl80211.c
static void     find_ssid(uint8_t *ies, uint32_t ies_len, uint8_t **ssid,
                          uint32_t *ssid_len) {
    *ssid = NULL;
    *ssid_len = 0;
    while (ies_len > 2 && ies[0] != WLAN_EID_SSID) {
        ies_len -= ies[1] + 2;
        ies += ies[1] + 2;
    }
    if (ies_len < 2 || ies_len < (uint32_t)(2 + ies[1])) {
        return;
    }
    *ssid_len = ies[1];
    *ssid = ies + 2;
}

static int              nl_station_cb(struct nl_msg *msg, void *data) {
    t_wlan              *wlan = data;
    struct nlattr       *tb[NL80211_ATTR_MAX + 1];
    struct genlmsghdr   *gnlh = nlmsg_data(nlmsg_hdr(msg));
    struct nlattr       *attr = genlmsg_attrdata(gnlh, 0);
    int                 attrlen = genlmsg_attrlen(gnlh, 0);
    struct nlattr       *s_info[NL80211_STA_INFO_MAX + 1];
    static struct       nla_policy stats_policy[NL80211_STA_INFO_MAX + 1];
    int                 signal = 0;

    if (nla_parse(tb, NL80211_ATTR_MAX, attr, attrlen, NULL) < 0) {
        return NL_SKIP;
    }
    if (tb[NL80211_ATTR_STA_INFO] == NULL) {
        return NL_SKIP;
    }
    if (nla_parse_nested(s_info, NL80211_STA_INFO_MAX,
        tb[NL80211_ATTR_STA_INFO], stats_policy) < 0) {
        return NL_SKIP;
    }
    if (s_info[NL80211_STA_INFO_SIGNAL] != NULL) {
        wlan->flags |= WLAN_FLAG_HAS_SIGNAL;
        signal = (int8_t)nla_get_u8(s_info[NL80211_STA_INFO_SIGNAL]);
        wlan->signal = nl80211_xbm_to_percent(signal, 1);
    }
    return NL_SKIP;
}

void            resolve_essid(t_wlan *wlan, struct nlattr *attr) {
    uint8_t     *ssid = 0;
    uint32_t    ssid_len = 0;
    uint8_t     *bss_ies = nla_data(attr);
    uint32_t    bss_ies_len = nla_len(attr);

    find_ssid(bss_ies, bss_ies_len, &ssid, &ssid_len);
    if (ssid && ssid_len) {
        wlan->flags |= WLAN_FLAG_HAS_ESSID;
        if (ssid_len >= WLAN_ESSID_MAX_SIZE) {
            v_strncpy(wlan->essid, (char *)ssid, WLAN_ESSID_MAX_SIZE);
            wlan->essid[WLAN_ESSID_MAX_SIZE - 1] = ':';
            wlan->essid[WLAN_ESSID_MAX_SIZE - 2] = '.';
        } else {
            v_strncpy(wlan->essid, (char *)ssid, ssid_len);
            wlan->essid[ssid_len] = ':';
        }
    }
}

static int              nl_scan_cb(struct nl_msg *msg, void *data) {
    t_wlan              *wlan = data;
    uint32_t            status = 0;
    struct genlmsghdr   *gnlh = nlmsg_data(nlmsg_hdr(msg));
    struct nlattr       *attr = genlmsg_attrdata(gnlh, 0);
    int                 attrlen = genlmsg_attrlen(gnlh, 0);
    struct nlattr       *tb[NL80211_ATTR_MAX + 1];
    struct nlattr       *bss[NL80211_BSS_MAX + 1];
    struct nla_policy   bss_policy[NL80211_BSS_MAX + 1] = {
        [NL80211_BSS_BSSID] = {.type = NLA_UNSPEC},
        [NL80211_BSS_INFORMATION_ELEMENTS] = {.type = NLA_UNSPEC},
        [NL80211_BSS_STATUS] = {.type = NLA_U32},
    };

    if (nla_parse(tb, NL80211_ATTR_MAX, attr, attrlen, NULL) < 0) {
        return NL_SKIP;
    }
    if (tb[NL80211_ATTR_BSS] == NULL) {
        return NL_SKIP;
    }
    if (nla_parse_nested(bss, NL80211_BSS_MAX, tb[NL80211_ATTR_BSS],
        bss_policy) < 0) {
        return NL_SKIP;
    }
    if (bss[NL80211_BSS_STATUS] == NULL) {
        return NL_SKIP;
    }
    status = nla_get_u32(bss[NL80211_BSS_STATUS]);
    if (status != NL80211_BSS_STATUS_ASSOCIATED && status !=
        NL80211_BSS_STATUS_IBSS_JOINED) {
        return NL_SKIP;
    }
    if (bss[NL80211_BSS_BSSID] == NULL) {
        return NL_SKIP;
    }

    memcpy(wlan->bssid, nla_data(bss[NL80211_BSS_BSSID]), ETH_ALEN);
    if (bss[NL80211_BSS_INFORMATION_ELEMENTS]) {
        resolve_essid(wlan, bss[NL80211_BSS_INFORMATION_ELEMENTS]);
    }
    return NL_SKIP;
}

static int          send_for_station(t_wlan *wlan) {
    struct nl_msg   *msg = NULL;
    int             err = 0;

    if ((err = nl_socket_modify_cb(wlan->socket, NL_CB_VALID, NL_CB_CUSTOM,
        nl_station_cb, wlan)) < 0) {
        printf("Call to nl_socket_modify_cb() failed: %s\n", nl_geterror(err));
        return -1;
    }
    if ((msg = nlmsg_alloc()) == NULL) {
        printf("Call to nlmsg_alloc() failed\n");
        return -1;
    }
    if (genlmsg_put(msg, NL_AUTO_PORT, NL_AUTO_SEQ, wlan->nl80211_id, 0,
        NLM_F_DUMP, NL80211_CMD_GET_STATION, 0) == NULL) {
        printf("Call to genlmsg_put() failed\n");
        nlmsg_free(msg);
        return -1;
    }
    if ((err = nla_put_u32(msg, NL80211_ATTR_IFINDEX, wlan->ifindex)) < 0) {
        printf("Call to nla_put_u32() failed: %s\n", nl_geterror(err));
        nlmsg_free(msg);
        return -1;
    }
    if ((err = nla_put(msg, NL80211_ATTR_MAC, 6, wlan->bssid)) < 0) {
        printf("Call to nla_put() failed: %s\n", nl_geterror(err));
        nlmsg_free(msg);
        return -1;
    }
    if ((err = nl_send_sync(wlan->socket, msg)) < 0) {
        printf("Call to nl_send_sync() failed: %s\n", nl_geterror(err));
        return -1;
    }
    return 0;
}

static int          send_for_scan(t_wlan *wlan) {
    struct nl_msg   *msg = NULL;
    int             err = 0;

    if ((err = nl_socket_modify_cb(wlan->socket, NL_CB_VALID, NL_CB_CUSTOM,
        nl_scan_cb, wlan)) < 0) {
        printf("Call to nl_socket_modify_cb() failed: %s\n", nl_geterror(err));
        return -1;
    }
    if ((msg = nlmsg_alloc()) == NULL) {
        printf("Call to nlmsg_alloc() failed\n");
        return -1;
    }
    if (genlmsg_put(msg, NL_AUTO_PORT, NL_AUTO_SEQ, wlan->nl80211_id, 0,
        NLM_F_DUMP, NL80211_CMD_GET_SCAN, 0) == NULL) {
        printf("Call to genlmsg_put() failed\n");
        nlmsg_free(msg);
        return -1;
    }
    if ((err = nla_put_u32(msg, NL80211_ATTR_IFINDEX, wlan->ifindex)) < 0) {
        printf("Call to nla_put_u32() failed: %s\n", nl_geterror(err));
        nlmsg_free(msg);
        return -1;
    }
    if ((err = nl_send_sync(wlan->socket, msg)) < 0) {
        printf("Call to nl_send_sync() failed: %s\n", nl_geterror(err));
        return -1;
    }
    return 0;
}

static void     reset_data(t_wlan *wlan) {
    wlan->flags = 0;
    v_memset(wlan->bssid, 0, ETH_ALEN);
    v_memset(wlan->essid, 0, WLAN_ESSID_MAX_SIZE);
    wlan->signal = 0;
}

void            *run_wireless(void *data) {
    t_module    *module = data;
    t_wlan      *wlan = module->data;

    reset_data(wlan);
    if (send_for_scan(wlan) < 0 || send_for_station(wlan) < 0) {
        nl_socket_free(wlan->socket);
        exit(EXIT_FAILURE);
    }
    if (wlan->flags & WLAN_FLAG_HAS_SIGNAL) {
        module->value = wlan->signal;
    } else {
        module->value = 0;
    }
    if (wlan->flags & WLAN_FLAG_HAS_ESSID) {
        module->label = wlan->essid;
    } else {
        module->label = wlan->lb_unk;
    }
    return NULL;
}

void            init_wireless(void *data) {
    t_module    *module = data;
    t_wlan      *wlan = module->data;
    size_t      size = 0;
    int         i = -1;
    int         err = 0;

    while (++i < WLAN_NOPTS) {
        if (strcmp(module->opts[i].key, OPT_WLAN_LB_UNK) == 0) {
            size = v_strlen(module->opts[i].value);
            wlan->lb_unk = alloc_buffer(size + 2);
            v_strncpy(wlan->lb_unk, module->opts[i].value, size);
            wlan->lb_unk[size] = ':';
        } else if (strcmp(module->opts[i].key, OPT_WLAN_IFACE) == 0) {
            wlan->ifname = module->opts[i].value;
        }
    }
    module->label = wlan->lb_unk;
    wlan->socket = nl_socket_alloc();
    if (wlan->socket == NULL) {
        printf("Unable to allocate memory for netlink socket\n");
        exit(EXIT_FAILURE);
    }
    if ((err = genl_connect(wlan->socket)) < 0) {
        printf("Call to genl_connect() failed: %s\n", nl_geterror(err));
        nl_socket_free(wlan->socket);
        exit(EXIT_FAILURE);
    }
    if ((wlan->nl80211_id = genl_ctrl_resolve(wlan->socket, NL80211)) < 0) {
        printf("Call to genl_ctrl_resolve() failed: %s\n",
               nl_geterror(wlan->nl80211_id));
        nl_socket_free(wlan->socket);
        exit(EXIT_FAILURE);
    }
    if ((wlan->ifindex = if_nametoindex(wlan->ifname)) == 0) {
        printf("Unable to resolve wireless interface %s: %s\n", wlan->ifname,
               strerror(errno));
        nl_socket_free(wlan->socket);
        exit(EXIT_FAILURE);
    }
}
