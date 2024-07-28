#pragma once
// #define ENABLE_NEW_MOB_PROTO_STRUCT_20141125	// bleeding resistance 2014/11/25
// #define ENABLE_NEW_MOB_PROTO_STRUCT_20151020	// claw resistance 2015/10/20

typedef long long		LLONG;
typedef LLONG HPTYPE;

class CPythonNonPlayer : public CSingleton<CPythonNonPlayer>
{
public:
	enum  EClickEvent
	{
		ON_CLICK_EVENT_NONE = 0,
		ON_CLICK_EVENT_BATTLE = 1,
		ON_CLICK_EVENT_SHOP = 2,
		ON_CLICK_EVENT_TALK = 3,
		ON_CLICK_EVENT_VEHICLE = 4,

		ON_CLICK_EVENT_MAX_NUM,
	};

	enum EMobRank
	{
		MOB_RANK_PAWN = 0,
		MOB_RANK_S_PAWN = 1,
		MOB_RANK_KNIGHT = 2,
		MOB_RANK_S_KNIGHT = 3,
		MOB_RANK_BOSS = 4,
		MOB_RANK_KING = 5,
	};

	enum EMobEnchants
	{
		MOB_ENCHANT_CURSE,
		MOB_ENCHANT_SLOW,
		MOB_ENCHANT_POISON,
		MOB_ENCHANT_STUN,
		MOB_ENCHANT_CRITICAL,
		MOB_ENCHANT_PENETRATE,
		MOB_ENCHANTS_MAX_NUM
	};
	enum EMobResists
	{
		MOB_RESIST_SWORD,
		MOB_RESIST_TWOHAND,
		MOB_RESIST_DAGGER,
		MOB_RESIST_BELL,
		MOB_RESIST_FAN,
		MOB_RESIST_BOW,
		MOB_RESIST_FIRE,
		MOB_RESIST_ELECT,
		MOB_RESIST_MAGIC,
		MOB_RESIST_WIND,
		MOB_RESIST_POISON,
		MOB_RESISTS_MAX_NUM
	};

	enum ERaceFlags
	{
		RACE_FLAG_ANIMAL = (1 << 0),
		RACE_FLAG_UNDEAD = (1 << 1),
		RACE_FLAG_DEVIL = (1 << 2),
		RACE_FLAG_HUMAN = (1 << 3),
		RACE_FLAG_ORC = (1 << 4),
		RACE_FLAG_MILGYO = (1 << 5),
		RACE_FLAG_INSECT = (1 << 6),
		RACE_FLAG_FIRE = (1 << 7),
		RACE_FLAG_ICE = (1 << 8),
		RACE_FLAG_DESERT = (1 << 9),
		RACE_FLAG_TREE = (1 << 10),
		RACE_FLAG_ATT_ELEC = (1 << 11),
		RACE_FLAG_ATT_FIRE = (1 << 12),
		RACE_FLAG_ATT_ICE = (1 << 13),
		RACE_FLAG_ATT_WIND = (1 << 14),
		RACE_FLAG_ATT_EARTH = (1 << 15),
		RACE_FLAG_ATT_DARK = (1 << 16),
		RACE_FLAG_BOSS = (1 << 17),
		RACE_FLAG_PET = (1 << 18),
		RACE_FLAG_MOUNT = (1 << 19),
	};

	enum EMobMaxNum
	{
		MOB_ATTRIBUTE_MAX_NUM = 12,
		MOB_SKILL_MAX_NUM = 5,
	};

#pragma pack(push)
#pragma pack(1)
	typedef struct SMobSkillLevel
	{
		DWORD       dwVnum;
		BYTE        bLevel;
	} TMobSkillLevel;

	typedef struct SMobTable
	{
		DWORD       dwVnum;
		char        szName[64 + 1];
		char        szLocaleName[64 + 1];

		BYTE        bType;                  // Monster, NPC
		BYTE        bRank;                  // PAWN, KNIGHT, KING
		BYTE        bBattleType;            // MELEE, etc..
		BYTE        bLevel;                 // Level
		BYTE        bSize;

#ifdef ENABLE_GOLD_LIMIT_REWORK
		long long       llGoldMin;
		long long       llGoldMax;
#else
		DWORD       dwGoldMin;
		DWORD       dwGoldMax;
#endif
		DWORD       dwExp;
		HPTYPE      dwMaxHP;
		BYTE        bRegenCycle;
		BYTE        bRegenPercent;
		WORD        wDef;

		DWORD       dwAIFlag;
		DWORD       dwRaceFlag;
		DWORD       dwImmuneFlag;

		BYTE        bStr, bDex, bCon, bInt;
		DWORD       dwDamageRange[2];

		short       sAttackSpeed;
		short       sMovingSpeed;
		BYTE        bAggresiveHPPct;
		WORD        wAggressiveSight;
		WORD        wAttackRange;

		char        cEnchants[MOB_ENCHANTS_MAX_NUM];
		char        cResists[MOB_RESISTS_MAX_NUM];

		DWORD       dwResurrectionVnum;
		DWORD       dwDropItemVnum;

		BYTE        bMountCapacity;
		BYTE        bOnClickType;

		BYTE        bEmpire;
		char        szFolder[64 + 1];
		float       fDamMultiply;
		DWORD       dwSummonVnum;
		DWORD       dwDrainSP;
		DWORD		dwMonsterColor;
		DWORD       dwPolymorphItemVnum;

		TMobSkillLevel	Skills[MOB_SKILL_MAX_NUM];

		BYTE		bBerserkPoint;
		BYTE		bStoneSkinPoint;
		BYTE		bGodSpeedPoint;
		BYTE		bDeathBlowPoint;
		BYTE		bRevivePoint;
	} TMobTable;

#pragma pack(pop)

	typedef std::list<TMobTable*> TMobTableList;
	typedef std::map<DWORD, std::unique_ptr<TMobTable>> TNonPlayerDataMap;

public:
	CPythonNonPlayer(void);
	virtual ~CPythonNonPlayer(void);

	void Clear();
	void Destroy();

#ifdef ENABLE_WIKI_SYSTEM
	DWORD			FindMobSearchBox(const char* mobName, DWORD index, bool reverse = false);
#endif
	bool				LoadNonPlayerData(const char* c_szFileName);

	const TMobTable* GetTable(DWORD dwVnum);

	bool				GetName(DWORD dwVnum, const char** c_pszName);
	bool				GetInstanceType(DWORD dwVnum, BYTE* pbType);
	BYTE				GetEventType(DWORD dwVnum);
	BYTE				GetEventTypeByVID(DWORD dwVID);
	DWORD				GetMonsterColor(DWORD dwVnum);
	const char* GetMonsterName(DWORD dwVnum);
	DWORD				GetMonsterRaceFlag(DWORD dwVnum);
#ifdef ENABLE_PENDANT_SYSTEM
	DWORD				GetMonsterRaceFlagAtt(DWORD dwVnum);
#endif
#ifdef ENABLE_TARGET_BOARD_RENEWAL
	BYTE				GetTableValueBYTE(DWORD dwVnum, std::string itorName);
	LLONG					GetTableValueDWORD(DWORD dwVnum, std::string itorName);
	long long			GetTableValueLongLong(DWORD dwVnum, std::string itorName);
	float				GetTableValueFloat(DWORD dwVnum, std::string itorName);
	bool				HasMonsterRaceFlag(DWORD dwFlag, DWORD dwVnum);
#endif

protected:
	TNonPlayerDataMap	m_NonPlayerDataMap;
public:
	float GetMonsterHitRange(DWORD dwVnum);
};
