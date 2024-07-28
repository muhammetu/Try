#pragma once
enum
{
	ADDRESS_MAX_LEN = 15
};

enum ItemDestroyTime { ITEM_DESTROY_TIME_AUTOGIVE, ITEM_DESTROY_TIME_DROPITEM, ITEM_DESTROY_TIME_MAX };

void config_init(const std::string& st_localeServiceName); // default "" is CONFIG

extern char sql_addr[256];

extern WORD mother_port;
extern WORD p2p_port;

extern char db_addr[ADDRESS_MAX_LEN + 1];
extern WORD db_port;

extern int passes_per_sec;
extern int save_event_second_cycle;
extern int ping_event_second_cycle;
extern int test_server;
extern bool	guild_mark_server;
extern BYTE guild_mark_min_level;

extern bool	g_bNoMoreClient;
extern bool	g_bNoRegen;

extern bool g_bGlobalShoutEnable;
extern bool g_bDisablePrismNeed;
extern bool g_bDisableEmotionMask;
#ifdef __ITEM_COUNT_LIMIT__
extern DWORD g_bItemCountLimit;
#else
extern BYTE g_bItemCountLimit;
#endif
extern bool	g_bGMHostCheck;
extern bool	g_bGuildInviteLimit;
extern bool	g_bGuildInfiniteMembers;
extern int g_iStatusPointGetLevelLimit;
extern int g_iStatusPointSetMaxValue;
extern int g_iShoutLimitLevel;
extern DWORD g_dwSkillBookNextReadMin;
extern DWORD g_dwSkillBookNextReadMax;
extern bool g_bNewChatView;
extern bool g_bPoisonUnaggr;
extern int g_iFishingTimeMin;
extern int g_iFishingTimeMax;
extern bool g_bUnlimitedAutoPotion;
extern bool g_bDisableSellItem;
extern bool g_bNoDropMetinStoneItem;
extern bool g_bEndPvpWhenDisconnect;
extern bool g_bNonCheckDSQualified;
extern bool g_bInfiniteHorseHealth;
extern bool g_bCantSkillForBoss;
extern bool g_bStoneExpConst;
extern bool g_bSafeboxWithoutDistance;
extern bool g_bDisableResetSubSkill;
extern bool g_bStatPlusNoLimit;
extern bool g_bDisableStopWhenRidingDie;
extern bool g_bFlushAtShutdown;
#ifdef __OFFLINE_SHOP__
extern bool g_bOfflineShopServer;
#endif
extern std::string g_stProxyIP;
extern int g_iDbLogLevel;
extern int g_aiItemDestroyTime[ITEM_DESTROY_TIME_MAX];

extern BYTE	g_bChannel;

extern bool	map_allow_find(int index);
extern void	map_allow_copy(long* pl, int size);

extern int		g_iUserLimit;
extern time_t	g_global_time;
const char* get_table_postfix();
extern std::string	g_stHostname;
extern std::string	g_stLocale;
extern std::string	g_stLocaleFilename;

extern char		g_szPublicIP[16];
extern char		g_szInternalIP[16];

extern int (*is_twobyte) (const char* str);
extern int (*check_name) (const char* str);

extern bool		g_bSkillDisable;

extern int		g_iFullUserCount;
extern int		g_iBusyUserCount;
extern void		LoadStateUserCount();

extern BYTE	g_bAuthServer;

extern BYTE	PK_PROTECT_LEVEL;

extern std::string	g_stAuthMasterIP;
extern WORD		g_wAuthMasterPort;

extern std::string	g_stQuestDir;
//extern std::string	g_stQuestObjectDir;
extern std::set<std::string> g_setQuestObjectDir;

extern int VIEW_RANGE;
extern int VIEW_BONUS_RANGE;

extern bool g_protectNormalPlayer;

extern bool g_NoDropMetinStone;
extern bool g_NoMountAtGuildWar;
extern bool g_NoPotionsOnPVP;

extern int gPlayerMaxLevel;
extern int gShutdownAge;
extern int gShutdownEnable;

// missing begin
extern std::string g_stBlockDate;
extern void map_allow_log();
// missing end

#ifdef __CLIENT_VERSION_UTILITY__
extern bool g_bOpenPlayer;
extern int g_iClientVersion;
extern void ReloadVersion();
#endif

#ifdef USE_OFFLINE_SHOP_CONFIG
extern bool shop_cities_find(int32_t index);
extern size_t shop_cities_size();
extern bool LoadOfflineShopConfig();
extern WORD get_shop_cities_index(WORD wMap);
extern bool g_bShopGlobal; // Active P2P Cores
#endif