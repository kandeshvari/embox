/**
 * @file cfg80211.h
 * @brief
 * @author Denis Deryugin <deryugin.denis@gmail.com>
 * @version 0.0.1
 * @date 2015-07-02
 */

#ifndef NET_CFG80211
#define NET_CFG80211

#define ETH_ALEN         6 /* Octets in one ethernet addr */

enum nl80211_iftype {
	NL80211_IFTYPE_UNSPECIFIED,
	NL80211_IFTYPE_ADHOC,
	NL80211_IFTYPE_STATION,
	NL80211_IFTYPE_AP,
	NL80211_IFTYPE_AP_VLAN,
	NL80211_IFTYPE_WDS,
	NL80211_IFTYPE_MONITOR,
	NL80211_IFTYPE_MESH_POINT,
	NL80211_IFTYPE_P2P_CLIENT,
	NL80211_IFTYPE_P2P_GO,
	NL80211_IFTYPE_P2P_DEVICE,
	NL80211_IFTYPE_OCB,

	/* keep last */
	NUM_NL80211_IFTYPES,
	NL80211_IFTYPE_MAX = NUM_NL80211_IFTYPES - 1
};

enum nl80211_bss_scan_width {
	NL80211_BSS_CHAN_WIDTH_20,
	NL80211_BSS_CHAN_WIDTH_10,
	NL80211_BSS_CHAN_WIDTH_5,
};

#ifndef BIT
#define BIT(x)		(1 << (x))
#endif

enum wiphy_flags {
	/* use hole at 0 */
	/* use hole at 1 */
	/* use hole at 2 */
	WIPHY_FLAG_NETNS_OK			= BIT(3),
	WIPHY_FLAG_PS_ON_BY_DEFAULT		= BIT(4),
	WIPHY_FLAG_4ADDR_AP			= BIT(5),
	WIPHY_FLAG_4ADDR_STATION		= BIT(6),
	WIPHY_FLAG_CONTROL_PORT_PROTOCOL	= BIT(7),
	WIPHY_FLAG_IBSS_RSN			= BIT(8),
	WIPHY_FLAG_MESH_AUTH			= BIT(10),
	WIPHY_FLAG_SUPPORTS_SCHED_SCAN		= BIT(11),
	/* use hole at 12 */
	WIPHY_FLAG_SUPPORTS_FW_ROAM		= BIT(13),
	WIPHY_FLAG_AP_UAPSD			= BIT(14),
	WIPHY_FLAG_SUPPORTS_TDLS		= BIT(15),
	WIPHY_FLAG_TDLS_EXTERNAL_SETUP		= BIT(16),
	WIPHY_FLAG_HAVE_AP_SME			= BIT(17),
	WIPHY_FLAG_REPORTS_OBSS			= BIT(18),
	WIPHY_FLAG_AP_PROBE_RESP_OFFLOAD	= BIT(19),
	WIPHY_FLAG_OFFCHAN_TX			= BIT(20),
	WIPHY_FLAG_HAS_REMAIN_ON_CHANNEL	= BIT(21),
	WIPHY_FLAG_SUPPORTS_5_10_MHZ		= BIT(22),
	WIPHY_FLAG_HAS_CHANNEL_SWITCH		= BIT(23),
};

enum nl80211_ext_feature_index {
	NL80211_EXT_FEATURE_VHT_IBSS,

	/* add new features before the definition below */
	NUM_NL80211_EXT_FEATURES,
	MAX_NL80211_EXT_FEATURES = NUM_NL80211_EXT_FEATURES - 1
};

enum cfg80211_signal_type {
	CFG80211_SIGNAL_TYPE_NONE,
	CFG80211_SIGNAL_TYPE_MBM,
	CFG80211_SIGNAL_TYPE_UNSPEC,
};

struct wiphy {
	/* assign these fields before you register the wiphy */

	/* permanent MAC address(es) */
	u8 perm_addr[ETH_ALEN];
	u8 addr_mask[ETH_ALEN];

	//struct mac_address *addresses;

	const struct ieee80211_txrx_stypes *mgmt_stypes;

	const struct ieee80211_iface_combination *iface_combinations;
	int n_iface_combinations;
	u16 software_iftypes;

	u16 n_addresses;

	/* Supported interface modes, OR together BIT(NL80211_IFTYPE_...) */
	u16 interface_modes;

	u16 max_acl_mac_addrs;

	u32 flags, regulatory_flags, features;
	//u8 ext_features[DIV_ROUND_UP(NUM_NL80211_EXT_FEATURES, 8)];

	u32 ap_sme_capa;

	enum cfg80211_signal_type signal_type;

	int bss_priv_size;
	u8 max_scan_ssids;
	u8 max_sched_scan_ssids;
	u8 max_match_sets;
	u16 max_scan_ie_len;
	u16 max_sched_scan_ie_len;

	int n_cipher_suites;
	const u32 *cipher_suites;

	u8 retry_short;
	u8 retry_long;
	u32 frag_threshold;
	u32 rts_threshold;
	u8 coverage_class;

	//char fw_version[ETHTOOL_FWVERS_LEN];
	u32 hw_version;

#ifdef CONFIG_PM
	const struct wiphy_wowlan_support *wowlan;
	struct cfg80211_wowlan *wowlan_config;
#endif

	u16 max_remain_on_channel_duration;

	u8 max_num_pmkids;

	u32 available_antennas_tx;
	u32 available_antennas_rx;

	/*
	 * Bitmap of supported protocols for probe response offloading
	 * see &enum nl80211_probe_resp_offload_support_attr. Only valid
	 * when the wiphy flag @WIPHY_FLAG_AP_PROBE_RESP_OFFLOAD is set.
	 */
	u32 probe_resp_offload;

	const u8 *extended_capabilities, *extended_capabilities_mask;
	u8 extended_capabilities_len;

	/* If multiple wiphys are registered and you're handed e.g.
	 * a regular netdev with assigned ieee80211_ptr, you won't
	 * know whether it points to a wiphy your driver has registered
	 * or not. Assign this to something global to your driver to
	 * help determine whether you own this wiphy or not. */
	const void *privid;

	struct ieee80211_supported_band *bands[IEEE80211_NUM_BANDS];

	/* Lets us get back the wiphy on the callback */
	//void (*reg_notifier)(struct wiphy *wiphy, struct regulatory_request *request);

	/* fields below are read-only, assigned by cfg80211 */

	//const struct ieee80211_regdomain __rcu *regd;

	/* the item in /sys/class/ieee80211/ points to this,
	 * you need use set_wiphy_dev() (see below) */
	//struct device dev;

	/* protects ->resume, ->suspend sysfs callbacks against unregister hw */
	bool registered;

	/* dir in debugfs: ieee80211/<wiphyname> */
	struct dentry *debugfsdir;

	const struct ieee80211_ht_cap *ht_capa_mod_mask;
	const struct ieee80211_vht_cap *vht_capa_mod_mask;

	/* the network namespace this phy lives in currently */
	//possible_net_t _net;

#ifdef CONFIG_CFG80211_WEXT
	const struct iw_handler_def *wext;
#endif

	const struct wiphy_coalesce_support *coalesce;

	const struct wiphy_vendor_command *vendor_commands;
	const struct nl80211_vendor_cmd_info *vendor_events;
	int n_vendor_commands, n_vendor_events;

	u16 max_ap_assoc_sta;

	u8 max_num_csa_counters;
	u8 max_adj_channel_rssi_comp;

	char priv[0];
};

enum ieee80211_max_ampdu_length_exp {
	IEEE80211_HT_MAX_AMPDU_8K = 0,
	IEEE80211_HT_MAX_AMPDU_16K = 1,
	IEEE80211_HT_MAX_AMPDU_32K = 2,
	IEEE80211_HT_MAX_AMPDU_64K = 3
};

enum ieee80211_min_mpdu_spacing {
	IEEE80211_HT_MPDU_DENSITY_NONE = 0,	/* No restriction */
	IEEE80211_HT_MPDU_DENSITY_0_25 = 1,	/* 1/4 usec */
	IEEE80211_HT_MPDU_DENSITY_0_5 = 2,	/* 1/2 usec */
	IEEE80211_HT_MPDU_DENSITY_1 = 3,	/* 1 usec */
	IEEE80211_HT_MPDU_DENSITY_2 = 4,	/* 2 usec */
	IEEE80211_HT_MPDU_DENSITY_4 = 5,	/* 4 usec */
	IEEE80211_HT_MPDU_DENSITY_8 = 6,	/* 8 usec */
	IEEE80211_HT_MPDU_DENSITY_16 = 7	/* 16 usec */
};

enum sme_state {
	CFG80211_SME_CONNECTING,
};

struct wireless_dev {
	struct wiphy *wiphy;
	enum nl80211_iftype iftype;

	enum sme_state sme_state;
	/* the remainder of this struct should be private to cfg80211 */
	//struct list_head list;
	struct net_device *netdev;

	u32 identifier;

	//struct list_head mgmt_registrations;
	spinlock_t mgmt_registrations_lock;

	struct mutex mtx;

	bool use_4addr, p2p_started;

	u8 address[ETH_ALEN];

	/* currently used for IBSS and SME - might be rearranged later */
	u8 ssid[IEEE80211_MAX_SSID_LEN];
	u8 ssid_len, mesh_id_len, mesh_id_up_len;
	struct cfg80211_conn *conn;
	struct cfg80211_cached_keys *connect_keys;

	//struct list_head event_list;
	spinlock_t event_lock;

	struct cfg80211_internal_bss *current_bss; /* associated / joined */
	//struct cfg80211_chan_def preset_chandef;
	//struct cfg80211_chan_def chandef;

	bool ibss_fixed;
	bool ibss_dfs_possible;

	bool ps;
	int ps_timeout;

	int beacon_interval;

	u32 ap_unexpected_nlportid;

	bool cac_started;
	unsigned long cac_start_time;
	unsigned int cac_time_ms;

	u32 owner_nlportid;
};

struct ieee80211_rate {
	u32 flags;
	u16 bitrate;
	u16 hw_value, hw_value_short;
};

struct cfg80211_ssid {
	u8 ssid[IEEE80211_MAX_SSID_LEN];
	u8 ssid_len;
};

struct cfg80211_scan_request {
	struct cfg80211_ssid *ssids;
	int n_ssids;
	u32 n_channels;
	enum nl80211_bss_scan_width scan_width;
	const u8 *ie;
	size_t ie_len;
	u32 flags;

	u32 rates[IEEE80211_NUM_BANDS];

	struct wireless_dev *wdev;

	u8 mac_addr[ETH_ALEN];
	u8 mac_addr_mask[ETH_ALEN];

	/* internal */
	struct wiphy *wiphy;
	unsigned long scan_start;
	bool aborted, notified;
	bool no_cck;

	/* keep last */
	struct ieee80211_channel *channels[0];
};

struct rate_info {
	u8 flags;
	u8 mcs;
	u16 legacy;
	u8 nss;
	u8 bw;
};

#define IEEE80211_MAX_CHAINS	4
#define STATION_INFO_SIGNAL 0x1
#define STATION_INFO_TX_BITRATE 0x2
#define STATION_INFO_RX_PACKETS 0x4
#define STATION_INFO_TX_PACKETS 0x8

struct station_info {
	u32 filled;
	u32 connected_time;
	u32 inactive_time;
	u64 rx_bytes;
	u64 tx_bytes;
	u16 llid;
	u16 plid;
	u8 plink_state;
	s8 signal;
	s8 signal_avg;

	u8 chains;
	s8 chain_signal[IEEE80211_MAX_CHAINS];
	s8 chain_signal_avg[IEEE80211_MAX_CHAINS];

	struct rate_info txrate;
	struct rate_info rxrate;
	u32 rx_packets;
	u32 tx_packets;
	u32 tx_retries;
	u32 tx_failed;
	u32 rx_dropped_misc;
	//struct sta_bss_parameters bss_param;
	//struct nl80211_sta_flag_update sta_flags;

	int generation;

	const u8 *assoc_req_ies;
	size_t assoc_req_ies_len;

	u32 beacon_loss_count;
	s64 t_offset;
	//enum nl80211_mesh_power_mode local_pm;
	//enum nl80211_mesh_power_mode peer_pm;
	//enum nl80211_mesh_power_mode nonpeer_pm;

	u32 expected_throughput;

	u64 rx_beacon;
	u8 rx_beacon_signal_avg;
	//struct cfg80211_tid_stats pertid[IEEE80211_NUM_TIDS + 1];
};

struct key_params {
	const u8 *key;
	const u8 *seq;
	int key_len;
	int seq_len;
	u32 cipher;
};

#define IW_AUTH_CIPHER_NONE	0x00000001

enum nl80211_auth_type {
	NL80211_AUTHTYPE_OPEN_SYSTEM,
	NL80211_AUTHTYPE_SHARED_KEY,
	NL80211_AUTHTYPE_FT,
	NL80211_AUTHTYPE_NETWORK_EAP,
	NL80211_AUTHTYPE_SAE,

	/* keep last */
	__NL80211_AUTHTYPE_NUM,
	NL80211_AUTHTYPE_MAX = __NL80211_AUTHTYPE_NUM - 1,
	NL80211_AUTHTYPE_AUTOMATIC
};

enum nl80211_mfp {
	NL80211_MFP_NO,
	NL80211_MFP_REQUIRED,
};

#define NL80211_MAX_NR_CIPHER_SUITES		5
#define NL80211_MAX_NR_AKM_SUITES		2

struct cfg80211_crypto_settings {
	u32 wpa_versions;
	u32 cipher_group;
	int n_ciphers_pairwise;
	u32 ciphers_pairwise[NL80211_MAX_NR_CIPHER_SUITES];
	int n_akm_suites;
	u32 akm_suites[NL80211_MAX_NR_AKM_SUITES];
	bool control_port;
	__be16 control_port_ethertype;
	bool control_port_no_encrypt;
};

struct ieee80211_ht_cap {
	unsigned short	cap_info;
	unsigned char	ampdu_params_info;
	unsigned char	supp_mcs_set[16];
	unsigned short	extended_ht_cap_info;
	unsigned int		tx_BF_cap_info;
	unsigned char	       antenna_selection_info;
};

enum nl80211_wpa_versions {
	NL80211_WPA_VERSION_1 = 1 << 0,
	NL80211_WPA_VERSION_2 = 1 << 1,
};

struct ieee80211_vht_cap {
	__le32 vht_cap_info;
	struct ieee80211_vht_mcs_info supp_mcs;
};

struct cfg80211_connect_params {
	struct ieee80211_channel *channel;
	struct ieee80211_channel *channel_hint;
	const u8 *bssid;
	const u8 *bssid_hint;
	const u8 *ssid;
	size_t ssid_len;
	enum nl80211_auth_type auth_type;
	const u8 *ie;
	size_t ie_len;
	bool privacy;
	enum nl80211_mfp mfp;
	struct cfg80211_crypto_settings crypto;
	const u8 *key;
	u8 key_len, key_idx;
	u32 flags;
	int bg_scan_period;
	struct ieee80211_ht_cap ht_capa;
	struct ieee80211_ht_cap ht_capa_mask;
	struct ieee80211_vht_cap vht_capa;
	struct ieee80211_vht_cap vht_capa_mask;
};

struct cfg80211_pmksa {
	const u8 *bssid;
	const u8 *pmkid;
};

struct ieee80211_radiotap_header {
u8 it_version;		/* Version 0. Only increases
			 * for drastic changes,
			 * introduction of compatible
			 * new fields does not count.
			 */
u8 it_pad;
__le16 it_len;		/* length of the whole
			 * header in bytes, including
			 * it_version, it_pad,
			 * it_len, and data fields.
			 */
__le32 it_present;	/* A bitmap telling which
			 * fields are present. Set bit 31
			 * (0x80000000) to extend the
			 * bitmap by another 32 bits.
			 * Additional extensions are made
			 * by setting bit 31.
			 */
};

struct ieee80211_hdr {
	__le16 frame_control;
	__le16 duration_id;
	u8 addr1[ETH_ALEN];
	u8 addr2[ETH_ALEN];
	u8 addr3[ETH_ALEN];
	__le16 seq_ctrl;
	u8 addr4[ETH_ALEN];
};

struct cfg80211_beacon_data {
	const u8 *head, *tail;
	const u8 *beacon_ies;
	const u8 *proberesp_ies;
	const u8 *assocresp_ies;
	const u8 *probe_resp;

	size_t head_len, tail_len;
	size_t beacon_ies_len;
	size_t proberesp_ies_len;
	size_t assocresp_ies_len;
	size_t probe_resp_len;
};

enum nl80211_chan_width {
	NL80211_CHAN_WIDTH_20_NOHT,
	NL80211_CHAN_WIDTH_20,
	NL80211_CHAN_WIDTH_40,
	NL80211_CHAN_WIDTH_80,
	NL80211_CHAN_WIDTH_80P80,
	NL80211_CHAN_WIDTH_160,
	NL80211_CHAN_WIDTH_5,
	NL80211_CHAN_WIDTH_10,
};

struct cfg80211_chan_def {
	struct ieee80211_channel *chan;
	enum nl80211_chan_width width;
	u32 center_freq1;
	u32 center_freq2;
};

enum nl80211_hidden_ssid {
	NL80211_HIDDEN_SSID_NOT_IN_USE,
	NL80211_HIDDEN_SSID_ZERO_LEN,
	NL80211_HIDDEN_SSID_ZERO_CONTENTS
};

enum nl80211_smps_mode {
	NL80211_SMPS_OFF,
	NL80211_SMPS_STATIC,
	NL80211_SMPS_DYNAMIC,

	__NL80211_SMPS_AFTER_LAST,
	NL80211_SMPS_MAX = __NL80211_SMPS_AFTER_LAST - 1
};

enum nl80211_acl_policy {
	NL80211_ACL_POLICY_ACCEPT_UNLESS_LISTED,
	NL80211_ACL_POLICY_DENY_UNLESS_LISTED,
};

struct cfg80211_acl_data {
	enum nl80211_acl_policy acl_policy;
	int n_acl_entries;

	/* Keep it last */
	//struct mac_address mac_addrs[];
};

struct cfg80211_ap_settings {
	struct cfg80211_chan_def chandef;

	struct cfg80211_beacon_data beacon;

	int beacon_interval, dtim_period;
	const u8 *ssid;
	size_t ssid_len;
	enum nl80211_hidden_ssid hidden_ssid;
	struct cfg80211_crypto_settings crypto;
	bool privacy;
	enum nl80211_auth_type auth_type;
	enum nl80211_smps_mode smps_mode;
	int inactivity_timeout;
	u8 p2p_ctwindow;
	bool p2p_opp_ps;
	const struct cfg80211_acl_data *acl;
};

#define WLAN_AKM_SUITE_8021X		0x000FAC01
#define WLAN_AKM_SUITE_PSK		0x000FAC02
#define WLAN_PMKID_LEN			16
#define ARPHRD_IEEE80211_RADIOTAP 803
#define IEEE80211_FTYPE_MGMT		0x0000
#define IEEE80211_HT_CAP_SUP_WIDTH_20_40	0x0002
#endif /* NET_CFG80211 */
