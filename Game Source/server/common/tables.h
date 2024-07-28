#pragma once
#include "length.h"
#include "item_length.h"

typedef	DWORD IDENT;

#define GD_PACKET_START	150
#define DG_PACKET_START	150

enum EGameToDBPackets
{

	HEADER_GD_LOGOUT = 1,
	HEADER_GD_PLAYER_LOAD,
	HEADER_GD_PLAYER_SAVE,
	HEADER_GD_PLAYER_CREATE,
	HEADER_GD_PLAYER_DELETE,
	HEADER_GD_BOOT,
	HEADER_GD_PLAYER_COUNT,
	HEADER_GD_QUEST_SAVE,
	HEADER_GD_SAFEBOX_LOAD,
	HEADER_GD_SAFEBOX_SAVE,
	HEADER_GD_SAFEBOX_CHANGE_SIZE,
	HEADER_GD_EMPIRE_SELECT,
	HEADER_GD_SAFEBOX_CHANGE_PASSWORD,
	HEADER_GD_GUILD_SKILL_UPDATE,
	HEADER_GD_GUILD_EXP_UPDATE,
	HEADER_GD_GUILD_ADD_MEMBER,
	HEADER_GD_GUILD_REMOVE_MEMBER,
	HEADER_GD_GUILD_CHANGE_GRADE,
	HEADER_GD_GUILD_CHANGE_MEMBER_DATA,
	HEADER_GD_GUILD_DISBAND,
	HEADER_GD_GUILD_WAR,
	HEADER_GD_GUILD_WAR_SCORE,
	HEADER_GD_GUILD_CREATE,
	HEADER_GD_ITEM_SAVE,
	HEADER_GD_ITEM_DESTROY,
	HEADER_GD_SAVE_AFFECT,
	HEADER_GD_ADD_AFFECT,
	HEADER_GD_CHANGE_AFFECT,
	HEADER_GD_REMOVE_AFFECT,
	HEADER_GD_ITEM_FLUSH,
	HEADER_GD_PARTY_CREATE,
	HEADER_GD_PARTY_DELETE,
	HEADER_GD_PARTY_ADD,
	HEADER_GD_PARTY_REMOVE,
	HEADER_GD_PARTY_STATE_CHANGE,
	HEADER_GD_FLUSH_CACHE,
	HEADER_GD_RELOAD_PROTO,
	HEADER_GD_CHANGE_NAME,
	HEADER_GD_GUILD_CHANGE_LADDER_POINT,
	HEADER_GD_GUILD_USE_SKILL,
	HEADER_GD_REQUEST_EMPIRE_PRIV,
	HEADER_GD_SET_EVENT_FLAG,
	HEADER_GD_PARTY_SET_MEMBER_LEVEL,
	HEADER_GD_AUTH_LOGIN,
	HEADER_GD_MALL_LOAD,
	HEADER_GD_RELOAD_ADMIN,
	HEADER_GD_REQ_CHANGE_GUILD_MASTER,
	HEADER_GD_REQ_SPARE_ITEM_ID_RANGE,
	HEADER_GD_UPDATE_HORSE_NAME,
	HEADER_GD_REQ_HORSE_NAME,
	HEADER_GD_VALID_LOGOUT,
	HEADER_GD_REQUEST_CHARGE_CASH,
	HEADER_GD_UPDATE_CHANNELSTATUS,
	HEADER_GD_REQUEST_CHANNELSTATUS,
	HEADER_GD_LOGIN_BY_KEY,

#ifdef __CHANNEL_CHANGER__
	HEADER_GD_FIND_CHANNEL							= GD_PACKET_START+0,
#endif
#ifdef __SKILL_COLOR__
	HEADER_GD_SKILL_COLOR_SAVE 						= GD_PACKET_START+1,
#endif
#ifdef __OFFLINE_SHOP__
	HEADER_GD_NEW_OFFLINESHOP						= GD_PACKET_START+2,
#endif
#ifdef ENABLE_RELOAD_SHOP_COMMAND
	HEADER_GD_RELOAD_SHOP		= GD_PACKET_START + 3,
#endif
#ifdef ENABLE_RELOAD_REFINE_COMMAND
	HEADER_GD_RELOAD_REFINE		= GD_PACKET_START + 4,
#endif
#ifdef __BATTLE_PASS_SYSTEM__
	HEADER_GD_SAVE_BATTLE_PASS						= GD_PACKET_START+5,
	HEADER_GD_REGISTER_BP_RANKING					= GD_PACKET_START+6,
	HEADER_GD_BATTLE_PASS_RANKING					= GD_PACKET_START+7,
	HEADER_GD_ADD_BATTLE_PASS						= GD_PACKET_START+8,
#endif

	HEADER_GD_SETUP									= 0xff,
};

enum EDBToGamePackets
{
	HEADER_DG_NOTICE								= 1,
	HEADER_DG_LOGIN_SUCCESS							= 2,
	HEADER_DG_LOGIN_NOT_EXIST						= 3,
	HEADER_DG_LOGIN_WRONG_PASSWD					= 4,
	HEADER_DG_LOGIN_ALREADY							= 5,
	HEADER_DG_PLAYER_LOAD_SUCCESS					= 6,
	HEADER_DG_PLAYER_LOAD_FAILED					= 7,
	HEADER_DG_PLAYER_CREATE_SUCCESS					= 8,
	HEADER_DG_PLAYER_CREATE_ALREADY					= 9,
	HEADER_DG_PLAYER_CREATE_FAILED					= 10,
	HEADER_DG_PLAYER_DELETE_SUCCESS					= 11,
	HEADER_DG_PLAYER_DELETE_FAILED					= 12,
	HEADER_DG_ITEM_LOAD								= 13,
	HEADER_DG_BOOT									= 14,
	HEADER_DG_QUEST_LOAD							= 15,
	HEADER_DG_SAFEBOX_LOAD							= 16,
	HEADER_DG_SAFEBOX_CHANGE_SIZE					= 17,
	HEADER_DG_SAFEBOX_WRONG_PASSWORD				= 18,
	HEADER_DG_SAFEBOX_CHANGE_PASSWORD_ANSWER		= 19,
	HEADER_DG_EMPIRE_SELECT							= 20,
	HEADER_DG_AFFECT_LOAD							= 21,
	HEADER_DG_MALL_LOAD								= 22,
	HEADER_DG_GUILD_SKILL_UPDATE					= 23,
	HEADER_DG_GUILD_SKILL_RECHARGE					= 24,
	HEADER_DG_GUILD_EXP_UPDATE						= 25,
	HEADER_DG_PARTY_CREATE							= 26,
	HEADER_DG_PARTY_DELETE							= 27,
	HEADER_DG_PARTY_ADD								= 28,
	HEADER_DG_PARTY_REMOVE							= 29,
	HEADER_DG_PARTY_STATE_CHANGE					= 30,
	HEADER_DG_PARTY_SET_MEMBER_LEVEL				= 31,
	HEADER_DG_TIME									= 32,
	HEADER_DG_GUILD_ADD_MEMBER						= 33,
	HEADER_DG_GUILD_REMOVE_MEMBER					= 34,
	HEADER_DG_GUILD_CHANGE_GRADE					= 35,
	HEADER_DG_GUILD_CHANGE_MEMBER_DATA				= 36,
	HEADER_DG_GUILD_DISBAND							= 37,
	HEADER_DG_GUILD_WAR								= 38,
	HEADER_DG_GUILD_WAR_SCORE						= 39,
	HEADER_DG_GUILD_LOAD							= 40,
	HEADER_DG_GUILD_LADDER							= 41,
	HEADER_DG_GUILD_SKILL_USABLE_CHANGE				= 42,
	HEADER_DG_SET_EVENT_FLAG						= 43,
	HEADER_DG_RELOAD_PROTO							= 44,
	HEADER_DG_CHANGE_NAME							= 45,
	HEADER_DG_AUTH_LOGIN							= 46,
	HEADER_DG_CHANGE_EMPIRE_PRIV					= 47,
	HEADER_DG_RELOAD_ADMIN							= 48,
	HEADER_DG_ACK_CHANGE_GUILD_MASTER				= 49,
	HEADER_DG_ACK_SPARE_ITEM_ID_RANGE				= 50,
	HEADER_DG_UPDATE_HORSE_NAME						= 51,
	HEADER_DG_ACK_HORSE_NAME						= 52,
	HEADER_DG_RESPOND_CHANNELSTATUS					= 54,
	HEADER_DG_NEED_LOGIN_LOG						= 55,
#ifdef __CHANNEL_CHANGER__
	HEADER_DG_CHANNEL_RESULT						= DG_PACKET_START+0,
#endif
#ifdef __OFFLINE_SHOP__
	HEADER_DG_NEW_OFFLINESHOP						= DG_PACKET_START+1,
#endif
#ifdef __SKILL_COLOR__
	HEADER_DG_SKILL_COLOR_LOAD						= DG_PACKET_START+2,
#endif
#ifdef ENABLE_RELOAD_SHOP_COMMAND
	HEADER_DG_RELOAD_SHOP = DG_PACKET_START + 3,
#endif
#ifdef ENABLE_RELOAD_REFINE_COMMAND
	HEADER_DG_RELOAD_REFINE = DG_PACKET_START + 4,
#endif
#ifdef __BATTLE_PASS_SYSTEM__
	HEADER_DG_BATTLE_PASS_LOAD						= DG_PACKET_START+5,
	HEADER_DG_BATTLE_PASS_LOAD_RANKING				= DG_PACKET_START+6,
#endif

	HEADER_DG_MAP_LOCATIONS							= 0xfe,
	HEADER_DG_P2P									= 0xff,
};

#pragma pack(1)

enum ERequestChargeType
{
	ERequestCharge_Cash = 0,
	ERequestCharge_Mileage,
};

typedef struct SRequestChargeCash
{
	DWORD		dwAID;		// id(primary key) - Account Table
	DWORD		dwAmount;
	ERequestChargeType	eChargeType;
} TRequestChargeCash;

typedef struct SSimplePlayer
{
	DWORD		dwID;
	char		szName[CHARACTER_NAME_MAX_LEN + 1];
	BYTE		byJob;
	BYTE		byLevel;
	DWORD		dwPlayMinutes;
#ifdef __CHARACTER_WINDOW_RENEWAL__
	DWORD		last_play;
#endif
	BYTE		byST, byHT, byDX, byIQ;
	DWORD		wMainPart;
	BYTE		bChangeName;
	DWORD		wHairPart;
#ifdef __ACCE_SYSTEM__
	DWORD		wAccePart;
#endif
	BYTE		bDummy[5];
	long		x, y;
	long		lAddr;
	WORD		wPort;
	BYTE		skill_group;
} TSimplePlayer;

typedef struct SAccountTable
{
	DWORD		id;
	char		login[LOGIN_MAX_LEN + 1];
	char		passwd[PASSWD_MAX_LEN + 1];
	char		social_id[SOCIAL_ID_MAX_LEN + 1];
	char		status[ACCOUNT_STATUS_MAX_LEN + 1];
	BYTE		bEmpire;
#ifdef __HWID_SECURITY_UTILITY__
	char		cpu_id[CPU_ID_MAX_LEN + 1];
	char		hdd_model[HDD_MODEL_MAX_LEN + 1];
	char		machine_guid[MACHINE_GUID_MAX_LEN + 1];
	char		mac_addr[MAC_ADDR_MAX_LEN + 1];
	char		hdd_serial[HDD_SERIAL_MAX_LEN + 1];
	char		bios_id[BIOS_ID_MAX_LEN + 1];
	char		pc_name[PC_NAME_MAX_NUM + 1];
	char		user_name[USER_NAME_MAX_NUM + 1];
	char		os_version[OS_VERSION_MAX_NUM + 1];
#endif // __HWID_SECURITY_UTILITY__
	TSimplePlayer	players[PLAYER_PER_ACCOUNT];
} TAccountTable;

typedef struct SPacketDGCreateSuccess
{
	BYTE		bAccountCharacterIndex;
	TSimplePlayer	player;
} TPacketDGCreateSuccess;

typedef struct TPlayerItemAttribute
{
	BYTE	bType;
	short	sValue;
} TPlayerItemAttribute;

typedef struct SPlayerItem
{
	DWORD	id;
	BYTE	window;
	WORD	pos;
	DWORD	count;

	DWORD	vnum;
	long	alSockets[ITEM_SOCKET_MAX_NUM];

	TPlayerItemAttribute    aAttr[ITEM_ATTRIBUTE_MAX_NUM];

	DWORD	owner;
#ifdef __BEGINNER_ITEM__
	bool is_basic;
#endif
#ifdef __ITEM_EVOLUTION__
	DWORD	evolution;
#endif
#ifdef __ITEM_CHANGELOOK__
	DWORD	transmutation;
#endif
} TPlayerItem;

typedef struct SQuickslot
{
	BYTE	type;
	BYTE	pos;
} TQuickslot;

typedef struct SPlayerSkill
{
	BYTE	bMasterType;
	BYTE	bLevel;
	time_t	tNextRead;
} TPlayerSkill;

struct	THorseInfo
{
	BYTE	bLevel;
	BYTE	bRiding;
	short	sStamina;
	short	sHealth;
	DWORD	dwHorseHealthDropTime;
};

typedef struct SPlayerTable
{
	DWORD	id;

	char	name[CHARACTER_NAME_MAX_LEN + 1];
	char	ip[IP_ADDRESS_LENGTH + 1];

	WORD	job;

	BYTE	level;
	BYTE	level_step;
	short	st, ht, dx, iq;

	DWORD	exp;
#ifdef __GOLD_LIMIT_REWORK__
	long long	gold;
#else
	INT		gold;
#endif
#ifdef __CHEQUE_SYSTEM__
	INT		cheque;
#endif
	BYTE	dir;
	INT		x, y, z;
	INT		lMapIndex;

	long	lExitX, lExitY;
	long	lExitMapIndex;

	// @duzenleme int yaptik zamaninda metin2dev'de paylasilmisti sebep ???
	HPTYPE		hp;
	int		sp;

	HPTYPE	sRandomHP;
	short	sRandomSP;

	int		playtime;
#ifdef __CHARACTER_WINDOW_RENEWAL__
	int		last_play;
#endif
	short	stat_point;
	short	skill_point;
	short	sub_skill_point;
	short	horse_skill_point;

#ifdef ENABLE_PLAYER_STATISTICS
	int		iKilledShinsoo;
	int		iKilledChunjo;
	int		iKilledJinno;
	int		iTotalKill;
	int		iDuelWon;
	int		iDuelLost;
	int		iKilledMonster;
	int		iKilledStone;
	int		iKilledBoss;
	int		iCompletedDungeon;
	int		iTakedFish;
	long long		iBestStoneDamage;
	long long		iBestBossDamage;
#endif

	TPlayerSkill skills[SKILL_MAX_NUM];

	TQuickslot  quickslot[QUICKSLOT_MAX_NUM];

	BYTE	part_base;
	DWORD	parts[PART_MAX_NUM];

	short	stamina;

	BYTE	skill_group;
	long	lAlignment;
#ifdef __RANK_SYSTEM__
	long	lRank;
#endif
#ifdef __LANDRANK_SYSTEM__
	long	lLandRank;
#endif
#ifdef __REBORN_SYSTEM__
	long	lReborn;
#endif
	short	stat_reset_count;

	THorseInfo	horse;

	DWORD	logoff_interval;

	int		aiPremiumTimes[PREMIUM_MAX_NUM];
#ifdef __BATTLE_PASS_SYSTEM__
	DWORD	dwBattlePassEndTime;
#endif
#ifdef ENABLE_CHAT_COLOR_SYSTEM
	BYTE color;
#endif // ENABLE_CHAT_COLOR_SYSTEM
#ifdef __TEAM_SYSTEM__
	long	lTeam;
#endif
#ifdef __MONIKER_SYSTEM__
	char moniker[MONIKER_MAX_LEN];
#endif
#ifdef ENABLE_LOVE_SYSTEM
	char love1[2];
	char love2[2];
#endif // ENABLE_LOVE_SYSTEM
#ifdef ENABLE_WORD_SYSTEM
	char word[24+1];
#endif // ENABLE_WORD_SYSTEM
} TPlayerTable;

typedef struct SMobSkillLevel
{
	DWORD	dwVnum;
	BYTE	bLevel;
} TMobSkillLevel;

typedef struct SEntityTable
{
	DWORD dwVnum;
} TEntityTable;

typedef struct SMobTable : public SEntityTable
{
	char	szName[CHARACTER_NAME_MAX_LEN + 1];
	char	szLocaleName[CHARACTER_NAME_MAX_LEN + 1];

	BYTE	bType;			// Monster, NPC
	BYTE	bRank;			// PAWN, KNIGHT, KING
	BYTE	bBattleType;		// MELEE, etc..
	BYTE	bLevel;			// Level
	BYTE	bSize;

#ifdef __GOLD_LIMIT_REWORK__
	long long	llGoldMin;
	long long	llGoldMax;
#else
	DWORD	dwGoldMin;
	DWORD	dwGoldMax;
#endif
	DWORD	dwExp;
	HPTYPE	dwMaxHP;
	BYTE	bRegenCycle;
	BYTE	bRegenPercent;
	WORD	wDef;

	DWORD	dwAIFlag;
	DWORD	dwRaceFlag;
	DWORD	dwImmuneFlag;

	BYTE	bStr, bDex, bCon, bInt;
	DWORD	dwDamageRange[2];

	short	sAttackSpeed;
	short	sMovingSpeed;
	BYTE	bAggresiveHPPct;
	WORD	wAggressiveSight;
	WORD	wAttackRange;

	char	cEnchants[MOB_ENCHANTS_MAX_NUM];
	char	cResists[MOB_RESISTS_MAX_NUM];

	DWORD	dwResurrectionVnum;
	DWORD	dwDropItemVnum;

	BYTE	bMountCapacity;
	BYTE	bOnClickType;

	BYTE	bEmpire;
	char	szFolder[64 + 1];

	float	fDamMultiply;

	DWORD	dwSummonVnum;
	DWORD	dwDrainSP;
	DWORD	dwMobColor;
	DWORD	dwPolymorphItemVnum;

	TMobSkillLevel Skills[MOB_SKILL_MAX_NUM];

	BYTE	bBerserkPoint;
	BYTE	bStoneSkinPoint;
	BYTE	bGodSpeedPoint;
	BYTE	bDeathBlowPoint;
	BYTE	bRevivePoint;
} TMobTable;

typedef struct SSkillTable
{
	DWORD	dwVnum;
	char	szName[32 + 1];
	BYTE	bType;
	BYTE	bMaxLevel;
	DWORD	dwSplashRange;

	char	szPointOn[64];
	char	szPointPoly[100 + 1];
	char	szSPCostPoly[100 + 1];
	char	szDurationPoly[100 + 1];
	char	szDurationSPCostPoly[100 + 1];
	char	szCooldownPoly[100 + 1];
	char	szMasterBonusPoly[100 + 1];
	//char	szAttackGradePoly[100 + 1];
	char	szGrandMasterAddSPCostPoly[100 + 1];
	DWORD	dwFlag;
	DWORD	dwAffectFlag;

	// Data for secondary skill
	char 	szPointOn2[64];
	char 	szPointPoly2[100 + 1];
	char 	szDurationPoly2[100 + 1];
	DWORD 	dwAffectFlag2;

	// Data for grand master point
	char 	szPointOn3[64];
	char 	szPointPoly3[100 + 1];
	char 	szDurationPoly3[100 + 1];

	BYTE	bLevelStep;
	BYTE	bLevelLimit;
	DWORD	preSkillVnum;
	BYTE	preSkillLevel;

	long	lMaxHit;
	char	szSplashAroundDamageAdjustPoly[100 + 1];

	BYTE	bSkillAttrType;

	DWORD	dwTargetRange;
} TSkillTable;

typedef struct SShopItemTable
{
	DWORD		vnum;
#ifdef __ITEM_COUNT_LIMIT__
	DWORD		count;
#else
	BYTE		count;
#endif
	TItemPos	pos;
#ifdef __GOLD_LIMIT_REWORK__
	long long	price;
#else
	DWORD		price;
#endif
#ifdef __CHEQUE_SYSTEM__
	DWORD		cheque;
#endif
	BYTE		display_pos;
#ifdef __SHOP_PRICE_TYPE_ITEM__
	DWORD		witemVnum;
#endif
} TShopItemTable;

typedef struct SShopTable
{
	DWORD		dwVnum;
	DWORD		dwNPCVnum;

#ifdef __ITEM_COUNT_LIMIT__
	DWORD		byItemCount;
#else
	BYTE		byItemCount;
#endif
	TShopItemTable	items[SHOP_HOST_ITEM_MAX_NUM];
} TShopTable;

#define QUEST_NAME_MAX_LEN	32
#define QUEST_STATE_MAX_LEN	64

typedef struct SQuestTable
{
	DWORD		dwPID;
	char		szName[QUEST_NAME_MAX_LEN + 1];
	char		szState[QUEST_STATE_MAX_LEN + 1];
	long		lValue;
} TQuestTable;

typedef struct SItemLimit
{
	BYTE	bType;
	long	lValue;
} TItemLimit;

typedef struct SItemApply
{
	BYTE	bType;
	long	lValue;
} TItemApply;

typedef struct SItemTable : public SEntityTable
{
	DWORD		dwVnumRange;
	char        szName[ITEM_NAME_MAX_LEN + 1];
	char	szLocaleName[ITEM_NAME_MAX_LEN + 1];
	BYTE	bType;
	BYTE	bSubType;

	BYTE        bWeight;
	BYTE	bSize;

	DWORD	dwAntiFlags;
	DWORD	dwFlags;
	DWORD	dwWearFlags;
	DWORD	dwImmuneFlag;

#ifdef __GOLD_LIMIT_REWORK__
	long long       llGold;
	long long       llShopBuyPrice;
#else
	DWORD       dwGold;
	DWORD       dwShopBuyPrice;
#endif

	TItemLimit	aLimits[ITEM_LIMIT_MAX_NUM];
	TItemApply	aApplies[ITEM_APPLY_MAX_NUM];
	long        alValues[ITEM_VALUES_MAX_NUM];
	long	alSockets[ITEM_SOCKET_MAX_NUM];
	DWORD	dwRefinedVnum;
	DWORD	dwRefineSet;
	BYTE	bAlterToMagicItemPct;
	BYTE	bSpecular;
	BYTE	bGainSocketPct;

	short int	sAddonType;



	char		cLimitRealTimeFirstUseIndex;
	char		cLimitTimerBasedOnWearIndex;

	int GetLevelLimit()
	{
		for (int i = 0; i < ITEM_LIMIT_MAX_NUM; ++i)
		{
			if (aLimits[i].bType == LIMIT_LEVEL)
				return aLimits[i].lValue;
		}

		return 0;
	}

} TItemTable;

struct TItemAttrTable
{
	TItemAttrTable() :
		dwApplyIndex(0),
		dwProb(0)
	{
		szApply[0] = 0;
		memset(&lValues, 0, sizeof(lValues));
		memset(&bMaxLevelBySet, 0, sizeof(bMaxLevelBySet));
	}

	char    szApply[APPLY_NAME_MAX_LEN + 1];
	DWORD   dwApplyIndex;
	DWORD   dwProb;
	long    lValues[ITEM_ATTRIBUTE_MAX_LEVEL];
	BYTE    bMaxLevelBySet[ATTRIBUTE_SET_MAX_NUM];
};

typedef struct SLoginPacket
{
	char	login[LOGIN_MAX_LEN + 1];
	char	passwd[PASSWD_MAX_LEN + 1];
} TLoginPacket;

typedef struct SPlayerLoadPacket
{
	DWORD	account_id;
	DWORD	player_id;
	BYTE	account_index;
} TPlayerLoadPacket;

typedef struct SPlayerCreatePacket
{
	char		login[LOGIN_MAX_LEN + 1];
	char		passwd[PASSWD_MAX_LEN + 1];
	DWORD		account_id;
	BYTE		account_index;
	TPlayerTable	player_table;
} TPlayerCreatePacket;

typedef struct SPlayerDeletePacket
{
	char	login[LOGIN_MAX_LEN + 1];
	DWORD	player_id;
	BYTE	account_index;
	//char	name[CHARACTER_NAME_MAX_LEN + 1];
	char	private_code[8];
} TPlayerDeletePacket;

typedef struct SLogoutPacket
{
	char	login[LOGIN_MAX_LEN + 1];
	char	passwd[PASSWD_MAX_LEN + 1];
} TLogoutPacket;

typedef struct SPlayerCountPacket
{
	DWORD	dwCount;
} TPlayerCountPacket;

typedef struct SSafeboxTable
{
	DWORD	dwID;
	BYTE	bSize;
#ifdef __GOLD_LIMIT_REWORK__
	long long	llGold;
#else
	DWORD	dwGold;
#endif
	WORD	wItemCount;
} TSafeboxTable;

typedef struct SSafeboxChangeSizePacket
{
	DWORD	dwID;
	BYTE	bSize;
} TSafeboxChangeSizePacket;

typedef struct SSafeboxLoadPacket
{
	DWORD	dwID;
	char	szLogin[LOGIN_MAX_LEN + 1];
	char	szPassword[SAFEBOX_PASSWORD_MAX_LEN + 1];
} TSafeboxLoadPacket;

typedef struct SSafeboxChangePasswordPacket
{
	DWORD	dwID;
	char	szOldPassword[SAFEBOX_PASSWORD_MAX_LEN + 1];
	char	szNewPassword[SAFEBOX_PASSWORD_MAX_LEN + 1];
} TSafeboxChangePasswordPacket;

typedef struct SSafeboxChangePasswordPacketAnswer
{
	BYTE	flag;
} TSafeboxChangePasswordPacketAnswer;

typedef struct SEmpireSelectPacket
{
	DWORD	dwAccountID;
	BYTE	bEmpire;
} TEmpireSelectPacket;

typedef struct SPacketGDSetup
{
	char	szPublicIP[16];	// Public IP which listen to users
	BYTE	bChannel;
	WORD	wListenPort;
	WORD	wP2PPort;
	long	alMaps[MAP_ALLOW_LIMIT];
	DWORD	dwLoginCount;
	BYTE	bAuthServer;
} TPacketGDSetup;

typedef struct SMapLocation
{
	long	alMaps[MAP_ALLOW_LIMIT];
	char	szHost[MAX_HOST_LENGTH + 1];
	WORD	wPort;
} TMapLocation;

typedef struct SPacketDGP2P
{
	char	szHost[MAX_HOST_LENGTH + 1];
	WORD	wPort;
	BYTE	bChannel;
} TPacketDGP2P;

typedef struct SPacketGuildSkillUpdate
{
	DWORD guild_id;
	int amount;
	BYTE skill_levels[12];
	BYTE skill_point;
	BYTE save;
} TPacketGuildSkillUpdate;

typedef struct SPacketGuildExpUpdate
{
	DWORD guild_id;
	int amount;
} TPacketGuildExpUpdate;

typedef struct SPacketGuildChangeMemberData
{
	DWORD guild_id;
	DWORD pid;
	DWORD offer;
	BYTE level;
	BYTE grade;
} TPacketGuildChangeMemberData;

typedef struct SPacketDGLoginAlready
{
	char	szLogin[LOGIN_MAX_LEN + 1];
} TPacketDGLoginAlready;

typedef struct TPacketAffectElement
{
	DWORD	dwType;
	BYTE	bApplyOn;
	long	lApplyValue;
	DWORD	dwFlag;
	long	lDuration;
	long	lSPCost;
} TPacketAffectElement;

typedef struct SPacketGDAddAffect
{
	DWORD			dwPID;
	TPacketAffectElement	elem;
} TPacketGDAddAffect;

typedef struct SPacketGDRemoveAffect
{
	DWORD	dwPID;
	DWORD	dwType;
	BYTE	bApplyOn;
} TPacketGDRemoveAffect;

typedef struct SPacketPartyCreate
{
	DWORD	dwLeaderPID;
} TPacketPartyCreate;

typedef struct SPacketPartyDelete
{
	DWORD	dwLeaderPID;
} TPacketPartyDelete;

typedef struct SPacketPartyAdd
{
	DWORD	dwLeaderPID;
	DWORD	dwPID;
	BYTE	bState;
} TPacketPartyAdd;

typedef struct SPacketPartyRemove
{
	DWORD	dwLeaderPID;
	DWORD	dwPID;
} TPacketPartyRemove;

typedef struct SPacketPartyStateChange
{
	DWORD	dwLeaderPID;
	DWORD	dwPID;
	BYTE	bRole;
	BYTE	bFlag;
} TPacketPartyStateChange;

typedef struct SPacketPartySetMemberLevel
{
	DWORD	dwLeaderPID;
	DWORD	dwPID;
	BYTE	bLevel;
} TPacketPartySetMemberLevel;

typedef struct SPacketGDBoot
{
	DWORD	dwItemIDRange[2];
	char	szIP[16];
} TPacketGDBoot;

typedef struct SPacketGuild
{
	DWORD	dwGuild;
	DWORD	dwInfo;
} TPacketGuild;

typedef struct SPacketGDGuildAddMember
{
	DWORD	dwPID;
	DWORD	dwGuild;
	BYTE	bGrade;
} TPacketGDGuildAddMember;

typedef struct SPacketDGGuildMember
{
	DWORD	dwPID;
	DWORD	dwGuild;
	BYTE	bGrade;
	BYTE	isGeneral;
	BYTE	bJob;
	BYTE	bLevel;
	DWORD	dwOffer;
	char	szName[CHARACTER_NAME_MAX_LEN + 1];
} TPacketDGGuildMember;

typedef struct SPacketGuildWar
{
	BYTE	bType;
	BYTE	bWar;
	DWORD	dwGuildFrom;
	DWORD	dwGuildTo;
	long	lInitialScore;
#ifdef __GUILD_WAR_REWORK__
	int		iMaxPlayer;
	int		iMaxScore;
#endif
} TPacketGuildWar;

typedef struct SPacketGuildWarScore
{
	DWORD dwGuildGainPoint;
	DWORD dwGuildOpponent;
	long lScore;
} TPacketGuildWarScore;

typedef struct SRefineMaterial
{
	DWORD vnum;
	int count;
} TRefineMaterial;

typedef struct SRefineTable
{
	DWORD id;
#ifdef __ITEM_COUNT_LIMIT__
	DWORD material_count;
#else
	BYTE material_count;
#endif
#ifdef __GOLD_LIMIT_REWORK__
	long long cost;
#else
	int cost;
#endif
	int prob;
	TRefineMaterial materials[REFINE_MATERIAL_MAX_NUM];
} TRefineTable;

typedef struct SPacketGDChangeName
{
	DWORD pid;
	char name[CHARACTER_NAME_MAX_LEN + 1];
} TPacketGDChangeName;

typedef struct SPacketDGChangeName
{
	DWORD pid;
	char name[CHARACTER_NAME_MAX_LEN + 1];
} TPacketDGChangeName;

typedef struct SPacketGuildLadder
{
	DWORD dwGuild;
	long lLadderPoint;
	long lWin;
	long lDraw;
	long lLoss;
} TPacketGuildLadder;

typedef struct SPacketGuildLadderPoint
{
	DWORD dwGuild;
	long lChange;
} TPacketGuildLadderPoint;

typedef struct SPacketGuildUseSkill
{
	DWORD dwGuild;
	DWORD dwSkillVnum;
	DWORD dwCooltime;
} TPacketGuildUseSkill;

typedef struct SPacketGuildSkillUsableChange
{
	DWORD dwGuild;
	DWORD dwSkillVnum;
	BYTE bUsable;
} TPacketGuildSkillUsableChange;

typedef struct SPacketGDAuthLogin
{
	DWORD	dwID;
	DWORD	dwLoginKey;
	char	szLogin[LOGIN_MAX_LEN + 1];
	char	szSocialID[SOCIAL_ID_MAX_LEN + 1];
	DWORD	adwClientKey[4];
	int		iPremiumTimes[PREMIUM_MAX_NUM];
#ifdef __HWID_SECURITY_UTILITY__
	char		cpu_id[CPU_ID_MAX_LEN + 1];
	char		hdd_model[HDD_MODEL_MAX_LEN + 1];
	char		machine_guid[MACHINE_GUID_MAX_LEN + 1];
	char		mac_addr[MAC_ADDR_MAX_LEN + 1];
	char		hdd_serial[HDD_SERIAL_MAX_LEN + 1];
	char		bios_id[BIOS_ID_MAX_LEN + 1];
	char		pc_name[PC_NAME_MAX_NUM + 1];
	char		user_name[USER_NAME_MAX_NUM + 1];
	char		os_version[OS_VERSION_MAX_NUM + 1];
#endif // __HWID_SECURITY_UTILITY__
} TPacketGDAuthLogin;

typedef struct SPacketGDLoginByKey
{
	char	szLogin[LOGIN_MAX_LEN + 1];
	DWORD	dwLoginKey;
	DWORD	adwClientKey[4];
	char	szIP[MAX_HOST_LENGTH + 1];
} TPacketGDLoginByKey;

typedef struct SPacketGiveEmpirePriv
{
	BYTE type;
	int value;
	BYTE empire;
	time_t duration_sec;
} TPacketGiveEmpirePriv;

typedef struct SPacketRemoveEmpirePriv
{
	BYTE type;
	BYTE empire;
} TPacketRemoveEmpirePriv;

typedef struct SPacketDGChangeEmpirePriv
{
	BYTE type;
	int value;
	BYTE empire;
	BYTE bLog;
	time_t end_time_sec;
} TPacketDGChangeEmpirePriv;

typedef struct SPacketSetEventFlag
{
	char	szFlagName[EVENT_FLAG_NAME_MAX_LEN + 1];
	long	lValue;
} TPacketSetEventFlag;

typedef struct SPacketLoginOnSetup
{
	DWORD   dwID;
	char    szLogin[LOGIN_MAX_LEN + 1];
	char    szSocialID[SOCIAL_ID_MAX_LEN + 1];
	char    szHost[MAX_HOST_LENGTH + 1];
	DWORD   dwLoginKey;
	DWORD   adwClientKey[4];
#ifdef __HWID_SECURITY_UTILITY__
	char		cpu_id[CPU_ID_MAX_LEN + 1];
	char		hdd_model[HDD_MODEL_MAX_LEN + 1];
	char		machine_guid[MACHINE_GUID_MAX_LEN + 1];
	char		mac_addr[MAC_ADDR_MAX_LEN + 1];
	char		hdd_serial[HDD_SERIAL_MAX_LEN + 1];
	char		bios_id[BIOS_ID_MAX_LEN + 1];
	char		pc_name[PC_NAME_MAX_NUM + 1];
	char		user_name[USER_NAME_MAX_NUM + 1];
	char		os_version[OS_VERSION_MAX_NUM + 1];
#endif // __HWID_SECURITY_UTILITY__
} TPacketLoginOnSetup;

#ifdef __CHEQUE_SYSTEM__
typedef struct SItemPriceType
{
#ifdef __GOLD_LIMIT_REWORK__
	SItemPriceType() { llPrice = dwCheque = 0; }
	SItemPriceType(long long gold, DWORD cheque)
	{
		llPrice = gold;
		dwCheque = cheque;
	}
	long long	llPrice;
	DWORD	dwCheque;
#else
	SItemPriceType() { dwPrice = dwCheque = 0; }
	SItemPriceType(DWORD gold, DWORD cheque)
	{
		dwPrice = gold;
		dwCheque = cheque;
	}
	DWORD	dwPrice;
	DWORD	dwCheque;
#endif
} TItemPriceType;
#endif

typedef struct SItemPriceInfo
{
	DWORD	dwVnum;
#ifdef __GOLD_LIMIT_REWORK__
#ifdef __CHEQUE_SYSTEM__
	TItemPriceType	price;
#else
	long long	llPrice;
#endif
#else
#ifdef __CHEQUE_SYSTEM__
	TItemPriceType	price;
#else
	DWORD	dwPrice;
#endif
#endif
} TItemPriceInfo;

typedef struct
{
	char szName[CHARACTER_NAME_MAX_LEN + 1];
	long lDuration;
} TPacketBlockChat;

typedef struct TAdminInfo
{
	int m_ID;
	char m_szAccount[32];
	char m_szName[32];
	char m_szContactIP[16];
	char m_szServerIP[16];
	int m_Authority;
} tAdminInfo;

struct tLocale
{
	char szValue[32];
	char szKey[32];
};

typedef struct SPacketReloadAdmin
{
	char szIP[16];
} TPacketReloadAdmin;

typedef struct tChangeGuildMaster
{
	DWORD dwGuildID;
	DWORD idFrom;
	DWORD idTo;
} TPacketChangeGuildMaster;

typedef struct tItemIDRange
{
	DWORD dwMin;
	DWORD dwMax;
	DWORD dwUsableItemIDMin;
} TItemIDRangeTable;

typedef struct tUpdateHorseName
{
	DWORD dwPlayerID;
	char szHorseName[CHARACTER_NAME_MAX_LEN + 1];
} TPacketUpdateHorseName;

typedef struct tDC
{
	char	login[LOGIN_MAX_LEN + 1];
} TPacketDC;

typedef struct tNeedLoginLogInfo
{
	DWORD dwPlayerID;
} TPacketNeedLoginLogInfo;

typedef struct SChannelStatus
{
	WORD nPort;
	BYTE bStatus;
} TChannelStatus;
#ifdef __OFFLINE_SHOP__
//common
typedef struct {
	BYTE bSubHeader;
} TPacketGDNewOfflineShop;

typedef struct {
	BYTE bSubHeader;
} TPacketDGNewOfflineShop;

namespace offlineshop
{
	//patch 08-03-2020
	enum class ExpirationType {
		EXPIRE_NONE,
		EXPIRE_REAL_TIME,
		EXPIRE_REAL_TIME_FIRST_USE,
	};

	enum FilterCategory
	{
		SEARCH_CATEGORY_ALL,
		SEARCH_CATEGORY_PREMIUM,
		SEARCH_CATEGORY_WEAPON,
		SEARCH_CATEGORY_ARMOR,
		SEARCH_CATEGORY_ACCESSORY,
		SEARCH_CATEGORY_PENDANT,
		SEARCH_CATEGORY_ACCE,
		SEARCH_CATEGORY_AURA,
		SEARCH_CATEGORY_COSTUME,
		SEARCH_CATEGORY_EQUIPABLE,
		SEARCH_CATEGORY_DSS,
		SEARCH_CATEGORY_PET,
		SEARCH_CATEGORY_SKILL,
		SEARCH_CATEGORY_POTION,
		SEARCH_CATEGORY_UPGRADE,
		SEARCH_CATEGORY_MAX,
	};

	typedef struct SPriceInfo
	{
		long long	illYang;
#ifdef __CHEQUE_SYSTEM__
		int iCheque;
#endif

		SPriceInfo() : illYang(0)
#ifdef __CHEQUE_SYSTEM__
			,iCheque(0)
#endif
		{}

		long long GetTotalYangAmount() const
		{
			// long long total = illYang;
			return illYang;
		}

#ifdef __CHEQUE_SYSTEM__
		int GetTotalChequeAmount() const
		{
			return iCheque;
		}
#endif

	} TPriceInfo;

	typedef struct
	{
		DWORD	dwVnum;
		DWORD	dwCount;
		long	alSockets[ITEM_SOCKET_MAX_NUM];
		TPlayerItemAttribute    aAttr[ITEM_ATTRIBUTE_MAX_NUM];

#ifdef __ITEM_CHANGELOOK__
		DWORD	dwTransmutation;
#endif
#ifdef __ITEM_EVOLUTION__
		DWORD	dwEvolution;
#endif

		//patch 08-03-2020
		ExpirationType	expiration;

	} TItemInfoEx;

	typedef struct SItemInfo
	{
		DWORD		dwOwnerID, dwItemID;
		TPriceInfo	price;
		TItemInfoEx	item;
		char		szOwnerName[CHARACTER_NAME_MAX_LEN + 1];
		SItemInfo() {
			memset(szOwnerName, 0, sizeof(szOwnerName));
		}
	} TItemInfo;

	typedef struct SValutesInfoa
	{
		long long	illYang;
#ifdef __CHEQUE_SYSTEM__
		int 		iCheque;
#endif

		void operator +=(const SValutesInfoa& r)
		{
			illYang += r.illYang;
#ifdef __CHEQUE_SYSTEM__
			iCheque += r.iCheque;
#endif
		}

		void operator -=(const SValutesInfoa& r)
		{
			illYang -= r.illYang;
#ifdef __CHEQUE_SYSTEM__
			iCheque -= r.iCheque;
#endif
		}

		SValutesInfoa() : illYang(0)
#ifdef __CHEQUE_SYSTEM__
			, iCheque(0)
#endif
		{}

	} TValutesInfo;


	typedef struct {
		DWORD	dwOwnerID;
		DWORD	dwDuration;
		char	szName[OFFLINE_SHOP_NAME_MAX_LEN];
		DWORD	dwCount;
		long	lMapIndex;
		long	x, y;
		BYTE	bChannel;
	} TShopInfo;

	typedef struct {
		long lMapIndex;
		long x, y;
		BYTE bChannel;
	} TShopPosition;

	// ### GAME TO DB ###

	enum eNewOfflineshopSubHeaderGD
	{
		SUBHEADER_GD_BUY_ITEM = 0,
		SUBHEADER_GD_BUY_LOCK_ITEM,
		SUBHEADER_GD_CANNOT_BUY_LOCK_ITEM, //topatch
		SUBHEADER_GD_REMOVE_ITEM,
		SUBHEADER_GD_ADD_ITEM,

		SUBHEADER_GD_SHOP_FORCE_CLOSE,
		SUBHEADER_GD_SHOP_CREATE_NEW,

		SUBHEADER_GD_SAFEBOX_GET_ITEM,
		SUBHEADER_GD_SAFEBOX_GET_VALUTES,
		SUBHEADER_GD_SAFEBOX_ADD_ITEM,
	};

	typedef struct {
		DWORD dwOwnerID, dwItemID, dwGuestID;
		bool isSearch;
	} TSubPacketGDBuyItem;


	typedef struct {
		DWORD dwOwnerID, dwItemID, dwGuestID;
		bool isSearch;
	} TSubPacketGDLockBuyItem;

	typedef struct SSubPacketGDCannotBuyLockItem //topatch
	{
		DWORD dwOwnerID, dwItemID;
	} TSubPacketGDCannotBuyLockItem;

	typedef struct {
		DWORD dwOwnerID;
		DWORD dwItemID;
	} TSubPacketGDRemoveItem;

	typedef struct {
		DWORD		dwOwnerID;
		TItemInfo	itemInfo;
	} TSubPacketGDAddItem;

	typedef struct {
		DWORD dwOwnerID;
	} TSubPacketGDShopForceClose;

	typedef struct {
		TShopInfo shop;
	} TSubPacketGDShopCreateNew;

	typedef struct {
		DWORD			dwOwnerID;
		DWORD			dwItemID;
	} TSubPacketGDSafeboxGetItem;

	typedef struct {
		DWORD			dwOwnerID;
		TItemInfoEx		item;
	} TSubPacketGDSafeboxAddItem;

	typedef struct {
		DWORD			dwOwnerID;
		TValutesInfo	valute;
	} TSubPacketGDSafeboxGetValutes;
	// ### DB TO GAME

	enum eSubHeaderDGNewOfflineshop
	{
		SUBHEADER_DG_BUY_ITEM,
		SUBHEADER_DG_LOCKED_BUY_ITEM,
		SUBHEADER_DG_REMOVE_ITEM,
		SUBHEADER_DG_ADD_ITEM,

		SUBHEADER_DG_SHOP_FORCE_CLOSE,
		SUBHEADER_DG_SHOP_CREATE_NEW,
		SUBHEADER_DG_SHOP_EXPIRED,

		SUBHEADER_DG_LOAD_TABLES,

		SUBHEADER_DG_SAFEBOX_GET_ITEM,
		SUBHEADER_DG_SAFEBOX_GET_VALUTES,
		SUBHEADER_DG_SAFEBOX_ADD_ITEM,
		SUBHEADER_DG_SAFEBOX_ADD_VALUTES,
		SUBHEADER_DG_SAFEBOX_LOAD,
		//patch 08-03-2020
		SUBHEADER_DG_SAFEBOX_EXPIRED_ITEM,
	};

	typedef struct {
		DWORD dwOwnerID, dwItemID, dwBuyerID;
		bool isSearch;
	} TSubPacketDGBuyItem;

	typedef struct {
		DWORD dwOwnerID, dwItemID, dwBuyerID;
		bool isSearch;
	} TSubPacketDGLockedBuyItem;

	typedef struct {
		DWORD dwOwnerID, dwItemID;
	} TSubPacketDGRemoveItem;

	typedef struct {
		DWORD		dwOwnerID, dwItemID;
		TItemInfo	item;
	} TSubPacketDGAddItem;

	typedef struct {
		DWORD dwOwnerID;
	} TSubPacketDGShopForceClose;

	typedef struct {
		TShopInfo shop;
	} TSubPacketDGShopCreateNew;

	typedef struct {
		DWORD	dwShopCount;
	} TSubPacketDGLoadTables;

	typedef struct {
		DWORD dwOwnerID;
	} TSubPacketDGShopExpired;

	typedef struct {
		DWORD dwOwnerID, dwItemID;
	} TSubPacketDGSafeboxGetItem;

	typedef struct {
		DWORD dwOwnerID;
		TValutesInfo	valute;
	} TSubPacketDGSafeboxGetValutes;

	typedef struct {
		DWORD dwOwnerID, dwItemID;
		TItemInfoEx item;
	} TSubPacketDGSafeboxAddItem;

	typedef struct {
		DWORD			dwOwnerID;
		TValutesInfo	valute;
	} TSubPacketDGSafeboxAddValutes;

	typedef struct {
		DWORD			dwOwnerID;
		TValutesInfo	valute;

		DWORD			dwItemCount;
	} TSubPacketDGSafeboxLoad;

	//patch 08-03-2020
	typedef struct {
		DWORD dwOwnerID;
		DWORD dwItemID;
	} TSubPacketDGSafeboxExpiredItem;
}
#endif

#ifdef __CHANNEL_CHANGER__
typedef struct
{
	long lMapIndex;
	int channel;
} TPacketChangeChannel;

typedef struct
{
	long lAddr;
	WORD port;
} TPacketReturnChannel;
#endif

#ifdef __SKILL_COLOR__
typedef struct
{
	DWORD	player_id;
	DWORD	dwSkillColor[ESkillColorLength::MAX_SKILL_COUNT + ESkillColorLength::MAX_BUFF_COUNT][ESkillColorLength::MAX_EFFECT_COUNT];
} TSkillColor;
#endif

#ifdef __SWITCHBOT__
struct TSwitchbotAttributeAlternativeTable
{
	TPlayerItemAttribute attributes[MAX_NORM_ATTR_NUM];

	bool IsConfigured() const
	{
		for (const auto& it : attributes)
		{
			if (it.bType && it.sValue)
			{
				return true;
			}
		}

		return false;
	}
};

struct TSwitchbotTable
{
	DWORD player_id;
	bool active[SWITCHBOT_SLOT_COUNT];
	bool finished[SWITCHBOT_SLOT_COUNT];
	DWORD items[SWITCHBOT_SLOT_COUNT];
	TSwitchbotAttributeAlternativeTable alternatives[SWITCHBOT_SLOT_COUNT][SWITCHBOT_ALTERNATIVE_COUNT];

	TSwitchbotTable() : player_id(0)
	{
		memset(&items, 0, sizeof(items));
		memset(&alternatives, 0, sizeof(alternatives));
		memset(&active, false, sizeof(active));
		memset(&finished, false, sizeof(finished));
	}
};

struct TSwitchbottAttributeTable
{
	BYTE attribute_set;
	int apply_num;
	long max_value;
};
#endif

#ifdef __BATTLE_PASS_SYSTEM__
typedef struct SPlayerBattlePassMission
{
	DWORD dwPlayerId;
	DWORD dwMissionId;
	DWORD dwBattlePassId;
	DWORD dwExtraInfo;
	BYTE bCompleted;
	BYTE bIsUpdated;
} TPlayerBattlePassMission;

typedef struct SBattlePassRewardItem
{
	DWORD dwVnum;
	BYTE bCount;
} TBattlePassRewardItem;

typedef struct SBattlePassMissionInfo
{
	BYTE bMissionType;
	DWORD dwMissionInfo[3];
	TBattlePassRewardItem aRewardList[MISSION_REWARD_COUNT];
	DWORD dwSkipCost;
} TBattlePassMissionInfo;

typedef struct SBattlePassRanking
{
	BYTE bPos;
	char playerName[CHARACTER_NAME_MAX_LEN + 1];
	DWORD dwFinishTime;
} TBattlePassRanking;

typedef struct SBattlePassRegisterRanking
{
	BYTE bBattlePassId;
	char playerName[CHARACTER_NAME_MAX_LEN + 1];
} TBattlePassRegisterRanking;
#endif

#ifdef ENABLE_MULTI_FARM_BLOCK
typedef struct SMultiFarm
{
	uint32_t playerID;
	bool farmStatus;
	uint8_t affectType;
	int affectTime;
	char playerName[CHARACTER_NAME_MAX_LEN + 1];
	SMultiFarm(uint32_t id_, const char* playerName_, bool status_, uint8_t type_, int time_) : playerID(id_), farmStatus(status_), affectType(type_), affectTime(time_) {
		strlcpy(playerName, playerName_, sizeof(playerName));
	}
}TMultiFarm;
#endif

#pragma pack()