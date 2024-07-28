#pragma once
#include <boost/unordered_map.hpp>
#include "../../common/stl.h"
#include "entity.h"
#include "FSM.h"
#include "horse_rider.h"
#include "vid.h"
#include "constants.h"
#include "affect.h"
#include "affect_flag.h"
#ifndef __CUBE_WINDOW__
#include "cube.h"
#else
#include "cuberenewal.h"
#endif
#include "mining.h"
#include <stdint.h>
#include <cinttypes>
#ifdef __ACCE_SYSTEM__
#include <vector>
#endif

#define MAX_EFFECT_FILE_NAME 128

const DWORD GetRandomSkillVnum(BYTE bJob = JOB_MAX_NUM);

extern bool IS_DUNGEON_ZONE(int map_index); // char_item.cpp

class CBuffOnAttributes;
class CPetSystem;
#ifdef __GROWTH_PET_SYSTEM__
class CNewPetSystem;
#endif
#ifdef __GROWTH_MOUNT_SYSTEM__
class CNewMountSystem;
#endif
#ifdef __MOUNT_COSTUME_SYSTEM__
class CMountSystem;
#endif
#ifdef __OFFLINE_SHOP__
namespace offlineshop
{
	class CShop;
	class CShopSafebox;
}
#endif
#define INSTANT_FLAG_DEATH_PENALTY		(1 << 0)
#define INSTANT_FLAG_SHOP			(1 << 1)
#define INSTANT_FLAG_EXCHANGE			(1 << 2)
#define INSTANT_FLAG_STUN			(1 << 3)
#define INSTANT_FLAG_NO_REWARD			(1 << 4)
#define AI_FLAG_NPC				(1 << 0)
#define AI_FLAG_AGGRESSIVE			(1 << 1)
#define AI_FLAG_HELPER				(1 << 2)
#define AI_FLAG_STAYZONE			(1 << 3)

extern int g_nPortalLimitTime;

#ifdef ENABLE_PLAYER_STATISTICS
typedef struct player_statistics
{
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
} PLAYER_STATISTICS;
#endif

enum RaceTypes
{
	MAIN_RACE_WARRIOR_M,
	MAIN_RACE_ASSASSIN_W,
	MAIN_RACE_SURA_M,
	MAIN_RACE_SHAMAN_W,
	MAIN_RACE_WARRIOR_W,
	MAIN_RACE_ASSASSIN_M,
	MAIN_RACE_SURA_W,
	MAIN_RACE_SHAMAN_M,
#ifdef __WOLFMAN_CHARACTER__
	MAIN_RACE_WOLFMAN_M,
#endif
	MAIN_RACE_MAX_NUM,
};

enum EOther
{
	POISON_LENGTH = 30,
#ifdef __WOLFMAN_CHARACTER__
	BLEEDING_LENGTH = 30,
#endif
	STAMINA_PER_STEP = 1,
	SAFEBOX_PAGE_SIZE = 9,
	AI_CHANGE_ATTACK_POISITION_TIME_NEAR = 10000,
	AI_CHANGE_ATTACK_POISITION_TIME_FAR = 1000,
	AI_CHANGE_ATTACK_POISITION_DISTANCE = 100,
	SUMMON_MONSTER_COUNT = 3,
};

enum FlyTypes
{
	FLY_NONE,
	FLY_EXP,
	FLY_HP_MEDIUM,
	FLY_HP_BIG,
	FLY_SP_SMALL,
	FLY_SP_MEDIUM,
	FLY_SP_BIG,
	FLY_FIREWORK1,
	FLY_FIREWORK2,
	FLY_FIREWORK3,
	FLY_FIREWORK4,
	FLY_FIREWORK5,
	FLY_FIREWORK6,
	FLY_FIREWORK_CHRISTMAS,
	FLY_CHAIN_LIGHTNING,
	FLY_HP_SMALL,
	FLY_SKILL_MUYEONG,
#ifdef __QUIVER_SYSTEM__
	FLY_QUIVER_ATTACK_NORMAL,
#endif
};

enum EDamageType
{
	DAMAGE_TYPE_NONE,
	DAMAGE_TYPE_NORMAL,
	DAMAGE_TYPE_NORMAL_RANGE,
	DAMAGE_TYPE_MELEE,
	DAMAGE_TYPE_RANGE,
	DAMAGE_TYPE_FIRE,
	DAMAGE_TYPE_ICE,
	DAMAGE_TYPE_ELEC,
	DAMAGE_TYPE_MAGIC,
	DAMAGE_TYPE_POISON,
	DAMAGE_TYPE_SPECIAL,
#ifdef __WOLFMAN_CHARACTER__
	DAMAGE_TYPE_BLEEDING,
#endif
};

enum DamageFlag
{
	DAMAGE_NORMAL = (1 << 0),
	DAMAGE_POISON = (1 << 1),
	DAMAGE_DODGE = (1 << 2),
	DAMAGE_BLOCK = (1 << 3),
	DAMAGE_PENETRATE = (1 << 4),
	DAMAGE_CRITICAL = (1 << 5),
#if defined(__WOLFMAN_CHARACTER__) && !defined(USE_MOB_BLEEDING_AS_POISON)
	DAMAGE_BLEEDING = (1 << 6),
#endif
};

enum EPointTypes
{
	POINT_NONE,
	POINT_LEVEL,
	POINT_EXP,
	POINT_NEXT_EXP,
	POINT_HP,
	POINT_MAX_HP,
	POINT_SP,
	POINT_MAX_SP,
	POINT_STAMINA,
	POINT_MAX_STAMINA,

	POINT_GOLD,
	POINT_ST,
	POINT_HT,
	POINT_DX,
	POINT_IQ,
	POINT_DEF_GRADE,
	POINT_ATT_SPEED,
	POINT_ATT_GRADE,
	POINT_MOV_SPEED,
	POINT_CLIENT_DEF_GRADE,
	POINT_CASTING_SPEED,
	POINT_MAGIC_ATT_GRADE,
	POINT_MAGIC_DEF_GRADE,
	POINT_EMPIRE_POINT,
	POINT_LEVEL_STEP,
	POINT_STAT,
	POINT_SUB_SKILL,
	POINT_SKILL,
	POINT_WEAPON_MIN,
	POINT_WEAPON_MAX,
	POINT_PLAYTIME,
	POINT_HP_REGEN,
	POINT_SP_REGEN,

	POINT_BOW_DISTANCE,

	POINT_HP_RECOVERY,
	POINT_SP_RECOVERY,

	POINT_POISON_PCT,
	POINT_STUN_PCT,
	POINT_SLOW_PCT,
	POINT_CRITICAL_PCT,
	POINT_PENETRATE_PCT,
	POINT_CURSE_PCT,

	POINT_ATTBONUS_HUMAN,
	POINT_ATTBONUS_ANIMAL,
	POINT_ATTBONUS_ORC,
	POINT_ATTBONUS_MILGYO,
	POINT_ATTBONUS_UNDEAD,
	POINT_ATTBONUS_DEVIL,
	POINT_ATTBONUS_INSECT,
	POINT_ATTBONUS_FIRE,
	POINT_ATTBONUS_ICE,
	POINT_ATTBONUS_DESERT,
	POINT_ATTBONUS_MONSTER,
	POINT_ATTBONUS_WARRIOR,
	POINT_ATTBONUS_ASSASSIN,
	POINT_ATTBONUS_SURA,
	POINT_ATTBONUS_SHAMAN,
	POINT_ATTBONUS_TREE,

	POINT_RESIST_WARRIOR,
	POINT_RESIST_ASSASSIN,
	POINT_RESIST_SURA,
	POINT_RESIST_SHAMAN,

	POINT_STEAL_HP,
	POINT_STEAL_SP,

	POINT_MANA_BURN_PCT,
	POINT_DAMAGE_SP_RECOVER,

	POINT_BLOCK,
	POINT_DODGE,

	POINT_RESIST_SWORD,
	POINT_RESIST_TWOHAND,
	POINT_RESIST_DAGGER,
	POINT_RESIST_BELL,
	POINT_RESIST_FAN,
	POINT_RESIST_BOW,
	POINT_RESIST_FIRE,
	POINT_RESIST_ELEC,
	POINT_RESIST_MAGIC,
	POINT_RESIST_WIND,

	POINT_REFLECT_MELEE,

	POINT_REFLECT_CURSE,
	POINT_POISON_REDUCE,

	POINT_KILL_SP_RECOVER,
	POINT_EXP_DOUBLE_BONUS,
	POINT_GOLD_DOUBLE_BONUS,
	POINT_ITEM_DROP_BONUS,

	POINT_POTION_BONUS,
	POINT_KILL_HP_RECOVERY,

	POINT_IMMUNE_STUN,
	POINT_IMMUNE_SLOW,
	POINT_IMMUNE_FALL,
	//////////////////

	POINT_PARTY_ATTACKER_BONUS,
	POINT_PARTY_TANKER_BONUS,

	POINT_ATT_BONUS,
	POINT_DEF_BONUS,

	POINT_ATT_GRADE_BONUS,
	POINT_DEF_GRADE_BONUS,
	POINT_MAGIC_ATT_GRADE_BONUS,
	POINT_MAGIC_DEF_GRADE_BONUS,

	POINT_RESIST_NORMAL_DAMAGE,

	POINT_HIT_HP_RECOVERY,
	POINT_HIT_SP_RECOVERY,
	POINT_MANASHIELD,

	POINT_PARTY_BUFFER_BONUS,
	POINT_PARTY_SKILL_MASTER_BONUS,

	POINT_HP_RECOVER_CONTINUE,
	POINT_SP_RECOVER_CONTINUE,

	POINT_STEAL_GOLD,
	POINT_POLYMORPH,
	POINT_MOUNT,

	POINT_PARTY_HASTE_BONUS,
	POINT_PARTY_DEFENDER_BONUS,
	POINT_STAT_RESET_COUNT,

	POINT_HORSE_SKILL,

	POINT_MALL_ATTBONUS,
	POINT_MALL_DEFBONUS,
	POINT_MALL_EXPBONUS,
	POINT_MALL_ITEMBONUS,
	POINT_MALL_GOLDBONUS,

	POINT_MAX_HP_PCT,
	POINT_MAX_SP_PCT,

	POINT_SKILL_DAMAGE_BONUS,
	POINT_NORMAL_HIT_DAMAGE_BONUS,

	// DEFEND_BONUS_ATTRIBUTES
	POINT_SKILL_DEFEND_BONUS,
	POINT_NORMAL_HIT_DEFEND_BONUS,
	POINT_UNUSED_EXP_BONUS,
	POINT_UNUSED_DROP_BONUS,
	// END_OF_DEFEND_BONUS_ATTRIBUTES

	POINT_RAMADAN_CANDY_BONUS_EXP,

	POINT_ENERGY = 128,

	POINT_ENERGY_END_TIME = 129,

	POINT_COSTUME_ATTR_BONUS = 130,
	POINT_MAGIC_ATT_BONUS_PER = 131,
	POINT_MELEE_MAGIC_ATT_BONUS_PER = 132,

	POINT_RESIST_ICE = 133,
	POINT_RESIST_EARTH = 134,
	POINT_RESIST_DARK = 135,

	POINT_RESIST_CRITICAL = 136,
	POINT_RESIST_PENETRATE = 137,

#ifdef __WOLFMAN_CHARACTER__
	POINT_BLEEDING_REDUCE = 138,
	POINT_BLEEDING_PCT = 139,

	POINT_ATTBONUS_WOLFMAN = 140,
	POINT_RESIST_WOLFMAN = 141,
	POINT_RESIST_CLAW = 142,
#endif

#ifdef __ACCE_SYSTEM__
	POINT_ACCEDRAIN_RATE = 143,
#endif
#ifdef __MAGIC_REDUCTION_SYSTEM__
	POINT_RESIST_MAGIC_REDUCTION = 144,
#endif
#ifdef __CHEQUE_SYSTEM__
	POINT_CHEQUE = 145,
#endif
#ifdef __PENDANT_SYSTEM__
	POINT_ENCHANT_FIRE = 146,
	POINT_ENCHANT_ICE = 147,
	POINT_ENCHANT_EARTH = 148,
	POINT_ENCHANT_DARK = 149,
	POINT_ENCHANT_WIND = 150,
	POINT_ENCHANT_ELECT = 151,

	POINT_RESIST_HUMAN = 152,

	POINT_ATTBONUS_SWORD = 153,
	POINT_ATTBONUS_TWOHAND = 154,
	POINT_ATTBONUS_DAGGER = 155,
	POINT_ATTBONUS_BELL = 156,
	POINT_ATTBONUS_FAN = 157,
	POINT_ATTBONUS_BOW = 158,
#ifdef __WOLFMAN_CHARACTER__
	POINT_ATTBONUS_CLAW = 159,
#endif
	POINT_ATTBONUS_CZ = 160,
#endif
#ifdef __ATTRIBUTES_TYPES__
	POINT_ATTBONUS_STONE = 161,
	POINT_ATTBONUS_BOSS = 162,
#endif
#ifdef __PARTY_ROLE_REWORK__
	POINT_PARTY_ATTACKER_MONSTER_BONUS = 163,
	POINT_PARTY_ATTACKER_STONE_BONUS = 164,
	POINT_PARTY_ATTACKER_BOSS_BONUS = 165,
#endif
#ifdef __ATTRIBUTES_TYPES__
	POINT_ATTBONUS_ELEMENTS = 166,
	POINT_ENCHANT_ELEMENTS = 167,
	POINT_ATTBONUS_CHARACTERS = 168,
	POINT_ENCHANT_CHARACTERS = 169,
#endif
#ifdef __CHEST_DROP_POINT__
	POINT_CHEST_BONUS = 170,
#endif
#ifdef __BATTLE_PASS_SYSTEM__
	POINT_BATTLE_PASS_ID = 171,
#endif
	//POINT_MAX_NUM = 129	common/length.h
};

enum EPKModes
{
	PK_MODE_PEACE,
	PK_MODE_REVENGE,
	PK_MODE_FREE,
	PK_MODE_PROTECT,
	PK_MODE_GUILD,
	PK_MODE_MAX_NUM
};

enum EPositions
{
	POS_DEAD,
	POS_SLEEPING,
	POS_RESTING,
	POS_SITTING,
	POS_FISHING,
	POS_FIGHTING,
	POS_MOUNTING,
	POS_STANDING
};

enum EBlockAction
{
	BLOCK_EXCHANGE = (1 << 0),
	BLOCK_PARTY_INVITE = (1 << 1),
	BLOCK_GUILD_INVITE = (1 << 2),
	BLOCK_WHISPER = (1 << 3),
	BLOCK_MESSENGER_INVITE = (1 << 4),
	BLOCK_PARTY_REQUEST = (1 << 5),
	BLOCK_POINT_EXP = (1 << 6),
	BLOCK_COSTUME_BODY = (1 << 7),
	BLOCK_COSTUME_HAIR = (1 << 8),
	BLOCK_COSTUME_WEAPON = (1 << 9),
	BLOCK_COSTUME_ACCE = (1 << 10),
	BLOCK_COSTUME_ACCE_EX = (1 << 11),
	BLOCK_COSTUME_AURA = (1 << 12),
};

// <Factor> Dynamically evaluated CHARACTER* equivalent.
// Referring to SCharDeadEventInfo.
struct DynamicCharacterPtr {
	DynamicCharacterPtr() : is_pc(false), id(0) {}
	DynamicCharacterPtr(const DynamicCharacterPtr& o)
		: is_pc(o.is_pc), id(o.id) {}

	// Returns the LPCHARACTER found in CHARACTER_MANAGER.
	LPCHARACTER Get() const;
	// Clears the current settings.
	void Reset() {
		is_pc = false;
		id = 0;
	}

	// Basic assignment operator.
	DynamicCharacterPtr& operator=(const DynamicCharacterPtr& rhs) {
		is_pc = rhs.is_pc;
		id = rhs.id;
		return *this;
	}
	// Supports assignment with LPCHARACTER type.
	DynamicCharacterPtr& operator=(LPCHARACTER character);
	// Supports type casting to LPCHARACTER.
	operator LPCHARACTER() const {
		return Get();
	}

	bool is_pc;
	uint32_t id;
};

typedef struct character_point
{
#ifdef __GOLD_LIMIT_REWORK__
	long long		points[POINT_MAX_NUM];
#else
	long			points[POINT_MAX_NUM];
#endif

	BYTE			job;

	BYTE			level;
	DWORD			exp;
#ifdef __GOLD_LIMIT_REWORK__
	long long		gold;
#else
	long			gold;
#endif
#ifdef __CHEQUE_SYSTEM__
	int				cheque;
#endif
	HPTYPE			hp;
	int				sp;

	HPTYPE				iRandomHP;
	int				iRandomSP;

	int				stamina;

	BYTE			skill_group;

#ifdef ENABLE_CHAT_COLOR_SYSTEM
	BYTE color;
#endif
} CHARACTER_POINT;

typedef struct character_point_instant
{
#ifdef __GOLD_LIMIT_REWORK__
	long long		points[POINT_MAX_NUM];
#else
	long			points[POINT_MAX_NUM];
#endif

	float			fRot;

	HPTYPE			iMaxHP;
	int				iMaxSP;

	long			position;

	long			instant_flag;
	DWORD			dwAIFlag;
	DWORD			dwImmuneFlag;
	DWORD			dwLastShoutPulse;

	DWORD			parts[PART_MAX_NUM];

	LPCHARACTER		pCubeNpc;
	LPCHARACTER			battle_victim;

	BYTE			gm_level;

	BYTE			bBasePart;

	int				iMaxStamina;

	int				bBlockMode;
	int				iDragonSoulActiveDeck;
	LPENTITY		m_pDragonSoulRefineWindowOpener;
} CHARACTER_POINT_INSTANT;

struct PlayerSlotT {
	LPITEM			pItems[INVENTORY_AND_EQUIP_SLOT_MAX];
	WORD			bItemGrid[INVENTORY_AND_EQUIP_SLOT_MAX];

	LPITEM			pDSItems[DRAGON_SOUL_INVENTORY_MAX_NUM];
	WORD			wDSItemGrid[DRAGON_SOUL_INVENTORY_MAX_NUM];
#ifdef __ADDITIONAL_INVENTORY__
	LPITEM			pSSUItems[SPECIAL_INVENTORY_MAX_NUM];
	WORD			wSSUItemGrid[SPECIAL_INVENTORY_MAX_NUM];
	LPITEM			pSSBItems[SPECIAL_INVENTORY_MAX_NUM];
	WORD			wSSBItemGrid[SPECIAL_INVENTORY_MAX_NUM];
	LPITEM			pSSSItems[SPECIAL_INVENTORY_MAX_NUM];
	WORD			wSSSItemGrid[SPECIAL_INVENTORY_MAX_NUM];
	LPITEM			pSSFItems[SPECIAL_INVENTORY_MAX_NUM];
	WORD			wSSFItemGrid[SPECIAL_INVENTORY_MAX_NUM];
	LPITEM			pSSAItems[SPECIAL_INVENTORY_MAX_NUM];
	WORD			wSSAItemGrid[SPECIAL_INVENTORY_MAX_NUM];
	LPITEM			pSSCItems[SPECIAL_INVENTORY_MAX_NUM];
	WORD			wSSCItemGrid[SPECIAL_INVENTORY_MAX_NUM];
#endif
	LPITEM			pCubeItems[CUBE_MAX_NUM];
#ifdef __ACCE_SYSTEM__
	TItemPosEx		pAcceMaterials[ACCE_WINDOW_MAX_MATERIALS];
#endif
#ifdef __ITEM_CHANGELOOK__
	LPITEM			pClMaterials[CL_WINDOW_MAX_MATERIALS];
#endif
#ifdef __SWITCHBOT__
	LPITEM			pSwitchbotItems[SWITCHBOT_SLOT_COUNT];
#endif
	TQuickslot		pQuickslot[QUICKSLOT_MAX_NUM];
};
// @fixme199 END


#define TRIGGERPARAM		LPCHARACTER ch, LPCHARACTER causer

typedef struct trigger
{
	BYTE	type;
	int		(*func) (TRIGGERPARAM);
	long	value;
} TRIGGER;

class CTrigger
{
public:
	CTrigger() : bType(0), pFunc(NULL)
	{
	}

	BYTE	bType;
	int	(*pFunc) (TRIGGERPARAM);
};

EVENTINFO(char_event_info)
{
	DynamicCharacterPtr ch;
};

typedef std::map<VID, size_t> target_map;
struct TSkillUseInfo
{
	int	    iHitCount;
	int	    iMaxHitCount;
	int	    iSplashCount;
	DWORD   dwNextSkillUsableTime;
	int	    iRange;
	bool    bUsed;
	DWORD   dwVID;
	bool    isGrandMaster;
	bool	cooldown{false};
	int		skillCount{0};

	target_map TargetVIDMap;

	TSkillUseInfo()
		: iHitCount(0), iMaxHitCount(0), iSplashCount(0), dwNextSkillUsableTime(0), iRange(0), bUsed(false),
		dwVID(0), isGrandMaster(false)
	{}

	bool    HitOnce(DWORD dwVnum = 0);
	bool    UseSkill(bool isGrandMaster, DWORD vid, DWORD dwCooltime, int splashcount = 1, int hitcount = -1, int range = -1);
	DWORD   GetMainTargetVID() const { return dwVID; }
	void    SetMainTargetVID(DWORD vid) { dwVID = vid; }
	void    ResetHitCount() { if (iSplashCount) { iHitCount = iMaxHitCount; iSplashCount--; } }
	bool	IsOnCooldown(int vnum);

};

typedef struct packet_party_update TPacketGCPartyUpdate;
class CExchange;
class CSkillProto;
class CParty;
class CDungeon;
class CWarMap;
class CAffect;
class CGuild;
class CSafebox;

class CShop;
typedef class CShop* LPSHOP;

class CMob;
class CMobInstance;
typedef struct SMobSkillInfo TMobSkillInfo;

//SKILL_POWER_BY_LEVEL
extern int GetSkillPowerByLevelFromType(int job, int skillgroup, int skilllevel);
//END_SKILL_POWER_BY_LEVEL

class CHARACTER : public CEntity, public CFSM, public CHorseRider
{
protected:
	//////////////////////////////////////////////////////////////////////////////////
	virtual void	EncodeInsertPacket(LPENTITY entity);
	virtual void	EncodeRemovePacket(LPENTITY entity);
	//////////////////////////////////////////////////////////////////////////////////

public:
	LPCHARACTER			FindCharacterInView(const char* name, bool bFindPCOnly);
	void				UpdatePacket();

	//////////////////////////////////////////////////////////////////////////////////
protected:
	CStateTemplate<CHARACTER>	m_stateMove;
	CStateTemplate<CHARACTER>	m_stateBattle;
	CStateTemplate<CHARACTER>	m_stateIdle;

public:
	virtual void		StateMove();
	virtual void		StateBattle();
	virtual void		StateIdle();
	virtual void		StateFlag();
	virtual void		StateFlagBase();
	void				StateHorse();

protected:
	// STATE_IDLE_REFACTORING
	void				__StateIdle_Monster();
	void				__StateIdle_Stone();
	void				__StateIdle_NPC();
	// END_OF_STATE_IDLE_REFACTORING

public:
	DWORD GetAIFlag() const { return m_pointsInstant.dwAIFlag; }

	void				SetAggressive();
	bool				IsAggressive() const;

	void				SetCoward();
	bool				IsCoward() const;
	void				CowardEscape();

	void				SetNoAttackShinsu();
	bool				IsNoAttackShinsu() const;

	void				SetNoAttackChunjo();
	bool				IsNoAttackChunjo() const;

	void				SetNoAttackJinno();
	bool				IsNoAttackJinno() const;

	void				SetAttackMob();
	bool				IsAttackMob() const;

	virtual void			BeginStateEmpty();
	virtual void			EndStateEmpty() {}

	void				RestartAtSamePos();

protected:
	DWORD				m_dwStateDuration;
	//////////////////////////////////////////////////////////////////////////////////

public:
	CHARACTER();
	virtual ~CHARACTER();

	void			Create(const char* c_pszName, DWORD vid, bool isPC);
	void			Destroy();

	void			Disconnect(const char* c_pszReason);

protected:
	void			Initialize();

	//////////////////////////////////////////////////////////////////////////////////
	// Basic Points
public:
	DWORD			GetPlayerID() const { return m_dwPlayerID; }

	void			SetPlayerProto(const TPlayerTable* table);
	void			CreatePlayerProto(TPlayerTable& tab);

	void			SetProto(const CMob* c_pkMob);
	DWORD			GetRaceNum() const;

	void			Save();		// DelayedSave
	void			SaveReal();
	void			FlushDelayedSaveItem();

	const char* GetName() const;
	const VID& GetVID() const { return m_vid; }

	void			SetName(const std::string& name) { m_stName = name; }
	void			SetRace(BYTE race);
	bool			ChangeSex();

	DWORD			GetAID() const;
	int				GetChangeEmpireCount() const;
	void			SetChangeEmpireCount();
	int				ChangeEmpire(BYTE empire);

	BYTE			GetJob() const;
	BYTE			GetCharType() const;

	bool			IsPC() const { return GetDesc() ? true : false; }
	bool			IsNPC()	const { return m_bCharType != CHAR_TYPE_PC; }
	bool			IsMonster()	const { return m_bCharType == CHAR_TYPE_MONSTER; }
	bool			IsStone() const { return m_bCharType == CHAR_TYPE_STONE; }
	bool			IsDoor() const { return m_bCharType == CHAR_TYPE_DOOR; }
	bool			IsWarp() const { return m_bCharType == CHAR_TYPE_WARP; }
	bool			IsGoto() const { return m_bCharType == CHAR_TYPE_GOTO; }
	bool			IsBoss();
	bool			IsHizli();
	bool			IsBoss() const;
	//		bool			IsPet() const		{ return m_bCharType == CHAR_TYPE_PET; }

	DWORD			GetLastShoutPulse() const { return m_pointsInstant.dwLastShoutPulse; }
	void			SetLastShoutPulse(DWORD pulse) { m_pointsInstant.dwLastShoutPulse = pulse; }
	int				GetLevel() const { return m_points.level; }
	void			SetLevel(BYTE level);

	BYTE			GetGMLevel() const;
	bool 			IsGM() const;
	void			SetGMLevel();

	DWORD			GetExp() const { return m_points.exp; }
	void			SetExp(DWORD exp) { m_points.exp = exp; }
	DWORD			GetNextExp() const;
#ifdef __GROWTH_PET_SYSTEM__
	DWORD			PetGetNextExp() const;
#endif
#ifdef __GROWTH_MOUNT_SYSTEM__
	DWORD			GetMountNextExp() const;
#endif
	LPCHARACTER		DistributeExp();
	void			DistributeSP(LPCHARACTER pkKiller, int iMethod = 0);
	void			SetPosition(int pos);
	bool			IsPosition(int pos) const { return m_pointsInstant.position == pos ? true : false; }
	int				GetPosition() const { return m_pointsInstant.position; }

	void			SetPart(BYTE bPartPos, DWORD wVal);
	DWORD			GetPart(BYTE bPartPos) const;
	DWORD			GetOriginalPart(BYTE bPartPos) const;

	void			SetHP(HPTYPE hp) { m_points.hp = hp; }
	HPTYPE		GetHP() const { return m_points.hp; }

	void			SetSP(int sp) { m_points.sp = sp; }
	int				GetSP() const { return m_points.sp; }

	void			SetStamina(int stamina) { m_points.stamina = stamina; }
	int				GetStamina() const { return m_points.stamina; }

	void			SetMaxHP(HPTYPE iVal) { m_pointsInstant.iMaxHP = iVal; }
	HPTYPE				GetMaxHP() const { return m_pointsInstant.iMaxHP; }

	void			SetMaxSP(int iVal) { m_pointsInstant.iMaxSP = iVal; }
	int				GetMaxSP() const { return m_pointsInstant.iMaxSP; }

	void			SetMaxStamina(int iVal) { m_pointsInstant.iMaxStamina = iVal; }
	int				GetMaxStamina() const { return m_pointsInstant.iMaxStamina; }

	void			SetRandomHP(HPTYPE v) { m_points.iRandomHP = v; }
	void			SetRandomSP(int v) { m_points.iRandomSP = v; }

	HPTYPE			GetRandomHP() const { return m_points.iRandomHP; }
	int				GetRandomSP() const { return m_points.iRandomSP; }

	int				GetHPPct() const;

	void			SetRealPoint(BYTE idx, long long val);
	long long				GetRealPoint(BYTE idx) const;

#ifdef __GOLD_LIMIT_REWORK__
	void			SetPoint(BYTE idx, long long val);
	long long		GetPoint(BYTE idx) const;
#else
	void			SetPoint(BYTE idx, int val);
	int				GetPoint(BYTE idx) const;
#endif
	int				GetLimitPoint(BYTE idx) const;

	const TMobTable& GetMobTable() const;
	BYTE				GetMobRank() const;
	BYTE				GetMobBattleType() const;
	BYTE				GetMobSize() const;
	DWORD				GetMobDamageMin() const;
	DWORD				GetMobDamageMax() const;
	WORD				GetMobAttackRange() const;
	DWORD				GetMobDropItemVnum() const;
	float				GetMobDamageMultiply() const;

	// NEWAI
	bool			IsBerserker() const;
	bool			IsBerserk() const;
	void			SetBerserk(bool mode);

	bool			IsStoneSkinner() const;

	bool			IsGodSpeeder() const;
	bool			IsGodSpeed() const;
	void			SetGodSpeed(bool mode);

	bool			IsDeathBlower() const;
	bool			IsDeathBlow() const;

	bool			IsReviver() const;
	bool			HasReviverInParty() const;
	bool			IsRevive() const;
	void			SetRevive(bool mode);
	// NEWAI END

	bool			IsRaceFlag(DWORD dwBit) const;
	bool			IsSummonMonster() const;
	DWORD			GetSummonVnum() const;

	DWORD			GetMonsterDrainSPPoint() const;

	void			MainCharacterPacket();

	void			ComputePoints();
	void			ComputeBattlePoints();
#ifdef __GOLD_LIMIT_REWORK__
	void			PointChange(BYTE type, long long amount, bool bAmount = false, bool bBroadcast = false);
#else
	void			PointChange(BYTE type, int amount, bool bAmount = false, bool bBroadcast = false);
#endif
	void			PointsPacket();
	void			ApplyPoint(BYTE bApplyType, int iVal);
#if defined(__GROWTH_PET_SYSTEM__) || defined(__GROWTH_MOUNT_SYSTEM__)
	void			SendPetLevelUpEffect(int vid, int type, int value, int amount);
#endif
	void			CheckMaximumPoints();

	bool			Show(long lMapIndex, long x, long y, long z = LONG_MAX, bool bShowSpawnMotion = false);
	void			Sitdown(int is_ground);
	void			Standup();

	void			SetRotation(float fRot);
	void			SetRotationToXY(long x, long y);
	float			GetRotation() const { return m_pointsInstant.fRot; }

	void			MotionPacketEncode(BYTE motion, LPCHARACTER victim, struct packet_motion* packet);
	void			Motion(BYTE motion, LPCHARACTER victim = NULL);

	void			ChatPacket(BYTE type, const char* format, ...);
	void			MonsterChat(BYTE bMonsterChatType);

	void			ResetPoint(int iLv);
	void			SetBlockMode(int bFlag);
	void			SetBlockModeForce(int bFlag);
	bool			IsBlockMode(int bFlag) const { return (m_pointsInstant.bBlockMode & bFlag) ? true : false; }

	// FISING
	bool			IS_VALID_FISHING_POSITION(long* returnPosx, long* returnPosy) const;
	void			fishing();
	void			fishing_take();
	// END_OF_FISHING

	// MINING
	void			mining(LPCHARACTER chLoad);
	void			mining_cancel();
	void			mining_take();
	// END_OF_MINING

	void			ResetPlayTime(DWORD dwTimeRemain = 0);

	void			CreateFly(BYTE bType, LPCHARACTER pkVictim);

	void			ResetChatCounter();
	BYTE			IncreaseChatCounter();
	BYTE			GetChatCounter() const;

protected:
	DWORD			m_dwLoginPlayTime;
	DWORD			m_dwPlayerID;
	VID				m_vid;
	std::string		m_stName;
#if defined(__GROWTH_PET_SYSTEM__) || defined(__GROWTH_MOUNT_SYSTEM__)
	BYTE			m_stImmortalSt;
#endif

	BYTE			m_bCharType;
#ifdef __GROWTH_PET_SYSTEM__
	DWORD			m_newpetskillcd[3];
#endif
#ifdef __GROWTH_MOUNT_SYSTEM__
	DWORD			m_newmountskillcd[3];
#endif
	CHARACTER_POINT		m_points;
	CHARACTER_POINT_INSTANT	m_pointsInstant;
	std::unique_ptr<PlayerSlotT> m_PlayerSlots; // @fixme199
#ifdef ENABLE_PLAYER_STATISTICS
	PLAYER_STATISTICS	m_playerstatistics;
#endif
	DWORD			m_dwPlayStartTime;
	BYTE			m_bAddChrState;
	bool			m_bSkipSave;
	BYTE			m_bChatCounter;
#ifdef __MOUNT_COSTUME_SYSTEM__
	BYTE			m_bMountCounter;
#endif
	// End of Basic Points

	//////////////////////////////////////////////////////////////////////////////////
	// Move & Synchronize Positions
	//////////////////////////////////////////////////////////////////////////////////
public:
	bool			IsStateMove() const { return IsState((CState&)m_stateMove); }
	bool			IsStateIdle() const { return IsState((CState&)m_stateIdle); }
	bool			IsWalking() const { return m_bNowWalking || GetStamina() <= 0; }
	void			SetWalking(bool bWalkFlag) { m_bWalking = bWalkFlag; }
	void			SetNowWalking(bool bWalkFlag);
	void			ResetWalking() { SetNowWalking(m_bWalking); }

	bool			Goto(long x, long y);
	void			Stop();

	bool			CanMove() const;

	void			SyncPacket();
	bool			Sync(long x, long y);
	bool			Move(long x, long y);
	void			OnMove(bool bIsAttack = false);
	DWORD			GetMotionMode() const;
	float			GetMoveMotionSpeed() const;
	float			GetMoveSpeed() const;
	void			CalculateMoveDuration();
	void			SendMovePacket(BYTE bFunc, BYTE bArg, DWORD x, DWORD y, DWORD dwDuration, DWORD dwTime = 0, int iRot = -1);
	DWORD			GetCurrentMoveDuration() const { return m_dwMoveDuration; }
	DWORD			GetWalkStartTime() const { return m_dwWalkStartTime; }
	DWORD			GetLastMoveTime() const { return m_dwLastMoveTime; }
	DWORD			GetLastAttackTime() const { return m_dwLastAttackTime; }
	void			SetLastAttacked(DWORD time);

	bool			SetSyncOwner(LPCHARACTER ch, bool bRemoveFromList = true);
	bool			IsSyncOwner(LPCHARACTER ch) const;

	bool			WarpSet(long x, long y, long lRealMapIndex = 0);
	bool			WarpSet(long x, long y, long lRealMapIndex, long addr, WORD port);
	void			SetWarpLocation(long lMapIndex, long x, long y);
	const PIXEL_POSITION& GetWarpPosition() const { return m_posWarp; }
	bool			WarpToPID(DWORD dwPID);
	void			SaveExitLocation();
	void			ExitToSavedLocation();

	void			StartStaminaConsume();
	void			StopStaminaConsume();
	bool			IsStaminaConsume() const;
	bool			IsStaminaHalfConsume() const;

	void			ResetStopTime();
	DWORD			GetStopTime() const;

protected:
	void			ClearSync();

	float			m_fSyncTime;
	LPCHARACTER		m_pkChrSyncOwner;
	CHARACTER_LIST	m_kLst_pkChrSyncOwned;

	PIXEL_POSITION	m_posDest;
	PIXEL_POSITION	m_posStart;
	PIXEL_POSITION	m_posWarp;
	long			m_lWarpMapIndex;

	PIXEL_POSITION	m_posExit;
	long			m_lExitMapIndex;

	DWORD			m_dwMoveStartTime;
	DWORD			m_dwMoveDuration;

	DWORD			m_dwLastMoveTime;
	DWORD			m_dwLastAttackTime;
	DWORD			m_dwWalkStartTime;
	DWORD			m_dwStopTime;
	bool			m_bWalking;
	bool			m_bNowWalking;
	bool			m_bStaminaConsume;
	// End

#ifdef __OFFLINE_SHOP__
public:
	offlineshop::CShop* GetOfflineShop() { return m_pkOfflineShop; }
	void					SetOfflineShop(offlineshop::CShop* pkShop) { m_pkOfflineShop = pkShop; }

	offlineshop::CShop* GetOfflineShopGuest() const { return m_pkOfflineShopGuest; }
	void					SetOfflineShopGuest(offlineshop::CShop* pkShop) { m_pkOfflineShopGuest = pkShop; }

	offlineshop::CShopSafebox* GetShopSafebox() const { return m_pkShopSafebox; }
	void					SetShopSafebox(offlineshop::CShopSafebox* pk);

	int		GetOfflineShopTime() const { return m_iOfflineShopTime; }
	void	SetOfflineShopTime() { m_iOfflineShopTime = thecore_pulse(); }
	int		m_iOfflineShopTime;

private:
	offlineshop::CShop* m_pkOfflineShop;
	offlineshop::CShop* m_pkOfflineShopGuest;
	offlineshop::CShopSafebox* m_pkShopSafebox;
#endif

public:
	void			SyncQuickslot(BYTE bType, BYTE bOldPos, BYTE bNewPos);
	bool			GetQuickslot(BYTE pos, TQuickslot** ppSlot);
	bool			SetQuickslot(BYTE pos, TQuickslot& rSlot);
	bool			DelQuickslot(BYTE pos);
	bool			SwapQuickslot(BYTE a, BYTE b);
	void			ChainQuickslotItem(LPITEM pItem, BYTE bType, BYTE bOldPos);

	////////////////////////////////////////////////////////////////////////////////////////
	// Affect
public:
	void			StartAffectEvent();
#ifdef __NOT_BUFF_CLEAR__
	void			ClearAffect(bool bSave = false, bool bSomeAffect = false);
#else
	void			ClearAffect(bool bSave = false);
#endif
	void			ComputeAffect(CAffect* pkAff, bool bAdd);
	bool			AddAffect(DWORD dwType, BYTE bApplyOn, long lApplyValue, DWORD dwFlag, long lDuration, long lSPCost, bool bOverride, bool IsCube = false);
	#define ADD_AFFECT_INFINITY(addType, bonusType, bonusValue) \
	AddAffect(addType, bonusType, bonusValue, 0, INFINITE_AFFECT_DURATION, 0, true);

	void			RefreshAffect();
	int				CheckAffectValue(DWORD dwType, BYTE bApply);
	bool			RemoveAffect(DWORD dwType);
	bool			IsAffectFlag(DWORD dwAff) const;

	bool			UpdateAffect();	// called from EVENT
	int				ProcessAffect();

	void			LoadAffect(DWORD dwCount, TPacketAffectElement* pElements);
	void			SaveAffect();

public:
	bool			IsLoadedAffect() const { return m_bIsLoadedAffect; }

	bool			IsGoodAffect(BYTE bAffectType) const;

	void			RemoveGoodAffect();
	void			RemoveOnlySkillAffect();
	void			RemoveBadAffect();

	CAffect* FindAffect(DWORD dwType, BYTE bApply = APPLY_NONE) const;
#ifdef __REMOVE_SKILL_AFFECT__
	CAffect* FindAffectByFlag(DWORD dwFlag) const;
#endif
	const std::list<CAffect*>& GetAffectContainer() const { return m_list_pkAffect; }
	bool		RemoveAffect(CAffect* pkAff);

protected:
	bool			m_bIsLoadedAffect;
	TAffectFlag		m_afAffectFlag;
	std::list<CAffect*>	m_list_pkAffect;

public:
	// PARTY_JOIN_BUG_FIX
	void			SetParty(LPPARTY pkParty);
	LPPARTY			GetParty() const { return m_pkParty; }

	bool			RequestToParty(LPCHARACTER leader);
	void			DenyToParty(LPCHARACTER member);
	void			AcceptToParty(LPCHARACTER member);

	void			PartyInvite(LPCHARACTER pchInvitee);

	void			PartyInviteAccept(LPCHARACTER pchInvitee);

	void			PartyInviteDeny(DWORD dwPID);

	bool			BuildUpdatePartyPacket(TPacketGCPartyUpdate& out);
	int				GetLeadershipSkillLevel() const;

	bool			CanSummon(int iLeaderShip);

	void			SetPartyRequestEvent(LPEVENT pkEvent) { m_pkPartyRequestEvent = pkEvent; }

protected:

	void			PartyJoin(LPCHARACTER pkLeader);

	enum PartyJoinErrCode {
		PERR_NONE = 0,
		PERR_SERVER,
		PERR_DUNGEON,
		PERR_OBSERVER,
		PERR_LVBOUNDARY,
		PERR_LOWLEVEL,
		PERR_HILEVEL,
		PERR_ALREADYJOIN,
		PERR_PARTYISFULL,
		PERR_SEPARATOR,			///< Error type separator.
		PERR_DIFFEMPIRE,
		PERR_MAX
	};

	static PartyJoinErrCode	IsPartyJoinableCondition(const LPCHARACTER pchLeader, const LPCHARACTER pchGuest);

	static PartyJoinErrCode	IsPartyJoinableMutableCondition(const LPCHARACTER pchLeader, const LPCHARACTER pchGuest);

	LPPARTY			m_pkParty;
	DWORD			m_dwLastDeadTime;
	LPEVENT			m_pkPartyRequestEvent;

	typedef std::map< DWORD, LPEVENT >	EventMap;
	EventMap		m_PartyInviteEventMap;

	// END_OF_PARTY_JOIN_BUG_FIX

	////////////////////////////////////////////////////////////////////////////////////////
	// Dungeon
public:
	void			SetDungeon(LPDUNGEON pkDungeon);
	LPDUNGEON		GetDungeon() const { return m_pkDungeon; }
	LPDUNGEON		GetDungeonForce() const;
protected:
	LPDUNGEON	m_pkDungeon;
	int			m_iEventAttr;

	////////////////////////////////////////////////////////////////////////////////////////
	// Guild
public:
	void			SetGuild(CGuild* pGuild);
	CGuild* GetGuild() const { return m_pGuild; }

	void			SetWarMap(CWarMap* pWarMap);
	CWarMap* GetWarMap() const { return m_pWarMap; }

protected:
	CGuild* m_pGuild;
	DWORD			m_dwUnderGuildWarInfoMessageTime;
	CWarMap* m_pWarMap;

	////////////////////////////////////////////////////////////////////////////////////////
	// Item related
public:
	bool			CanHandleItem(bool bSkipRefineCheck = false, bool bSkipObserver = false);

	bool			IsItemLoaded() const { return m_bItemLoaded; }
	void			SetItemLoaded() { m_bItemLoaded = true; }

	void			ClearItem();
#ifdef __HIGHLIGHT_ITEM__
	void			SetItem(TItemPos Cell, LPITEM item, bool bWereMine, const char* file, int line);
#else
	void			SetItem(TItemPos Cell, LPITEM item);
#endif
	#define __SET_ITEM(Cell, item) SetItem(Cell, item, false, __FILE__, __LINE__)
	#define __SET_ITEM_HG(Cell, item, hg) SetItem(Cell, item, hg, __FILE__, __LINE__)
	LPITEM			GetItem(TItemPos Cell) const;
	LPITEM			GetInventoryItem(WORD wCell) const;
	LPITEM			GetDragonSoulInventoryItem(WORD wCell) const;
#ifdef __ADDITIONAL_INVENTORY__
	LPITEM			GetUpgradeInventoryItem(WORD wCell) const;
	LPITEM			GetBookInventoryItem(WORD wCell) const;
	LPITEM			GetStoneInventoryItem(WORD wCell) const;
	LPITEM			GetFlowerInventoryItem(WORD wCell) const;
	LPITEM			GetAttrInventoryItem(WORD wCell) const;
	LPITEM			GetChestInventoryItem(WORD wCell) const;
#endif
#ifdef __SKILLBOOK_SYSTEM__
	int				BKBul(long skillindex) const;
#endif
	bool			IsEmptyItemGrid(TItemPos Cell, BYTE size, int iExceptionCell = -1) const;
	bool			IsEmptyItemGridSpecial(const TItemPos& Cell, BYTE bSize, int iExceptionCell, std::vector<WORD>& vec) const;

	void			SetWear(BYTE bCell, LPITEM item);
	LPITEM			GetWear(BYTE bCell) const;

	void			UseSilkBotary();

#ifdef __USE_ITEM_COUNT__
	bool			UseItem(TItemPos Cell, TItemPos DestCell = NPOS, DWORD count = 1);
	bool			UseItemEx(LPITEM item, TItemPos DestCell, DWORD count);
#else // __USE_ITEM_COUNT__
	bool			UseItem(TItemPos Cell, TItemPos DestCell = NPOS);
	bool			UseItemEx(LPITEM item, TItemPos DestCell);
#endif // __USE_ITEM_COUNT__

	// ADD_REFINE_BUILDING
#ifdef __GOLD_LIMIT_REWORK__
	long long				ComputeRefineFee(long long llCost, int iMultiply = 5) const;
#else
	int				ComputeRefineFee(int iCost, int iMultiply = 5) const;
#endif
#ifdef __GOLD_LIMIT_REWORK__
	void			PayRefineFee(long long llTotalMoney);
#else
	void			PayRefineFee(int iTotalMoney);
#endif
	void			SetRefineNPC(LPCHARACTER ch);
	LPCHARACTER		GetRefineNPC() const; // @duzenleme uzaktan arti basmamalari icin boyle bir engel aldik (Rubinum Metin2AR)
	// END_OF_ADD_REFINE_BUILDING

	bool			RefineItem(LPITEM pkItem, LPITEM pkTarget);
#ifdef __DROP_DIALOG__
	bool			DeleteItem(TItemPos Cell);
	bool			SellItem(TItemPos Cell);
#endif
	bool			GiveRecallItem(LPITEM item);
	void			ProcessRecallItem(LPITEM item);

	//	void			PotionPacket(int iPotionType);
	void			EffectPacket(int enumEffectType);
	void			SpecificEffectPacket(const std::string& sFileName);

	// ADD_MONSTER_REFINE
	bool			DoRefine(LPITEM item, bool bMoneyOnly = false);
	// END_OF_ADD_MONSTER_REFINE

	bool			DoRefineWithScroll(LPITEM item);
#ifdef __ITEM_EVOLUTION__
	bool			CleanEvolution(LPITEM pkItem, LPITEM pkTarget);
	bool			DoRefine_Rarity(LPITEM item, LPITEM scroll);
#endif
	bool			RefineInformation(BYTE bCell, BYTE bType, int iAdditionalCell = -1);

	void			SetRefineMode(int iAdditionalCell = -1);
	void			ClearRefineMode();

	bool			GiveItem(LPCHARACTER victim, TItemPos Cell);
	bool			CanReceiveItem(LPCHARACTER from, LPITEM item, bool ignoreDist = false) const; // @duzenleme uzaktan arti basmamalari icin boyle bir engel aldik (Rubinum Metin2AR)
	void			ReceiveItem(LPCHARACTER from, LPITEM item);
#ifdef __RENEWAL_CHEST_USE__
	int			GiveItemFromSpecialItemGroup(DWORD dwGroupNum, DWORD count, boost::unordered_map<DWORD, DWORD>& items);
#endif

#ifdef __ITEM_COUNT_LIMIT__
	bool			MoveItem(TItemPos pos, TItemPos change_pos, DWORD num);
#else
	bool			MoveItem(TItemPos pos, TItemPos change_pos, BYTE num);
#endif
	bool			PickupItem(DWORD vid, bool forced=false);
	bool			EquipItem(LPITEM item, int iCandidateCell = -1);
	bool			UnequipItem(LPITEM item);

	bool			CanEquipNow(const LPITEM item, const TItemPos& srcCell = NPOS, const TItemPos& destCell = NPOS);

	bool			CanUnequipNow(const LPITEM item, const TItemPos& srcCell = NPOS, const TItemPos& destCell = NPOS);

	bool			SwapItem(BYTE bCell, BYTE bDestCell);
#ifdef __ITEM_COUNT_LIMIT__
	LPITEM			AutoGiveItem(DWORD dwItemVnum, DWORD bCount = 1, int iRarePct = -1, bool bMsg = true);
#else
	LPITEM			AutoGiveItem(DWORD dwItemVnum, BYTE bCount = 1, int iRarePct = -1, bool bMsg = true);
#endif
	void			AutoGiveItem(LPITEM item, bool longOwnerShip = false);

	int				GetEmptyInventory(BYTE size) const;
	bool			CanTakeInventoryItem(LPITEM item, TItemPos* pos);
#ifdef __SPLIT_ITEMS__
	int				GetEmptyInventoryFromIndex(WORD index, BYTE itemSize) const; //SPLIT ITEMS
#ifdef __ADDITIONAL_INVENTORY__
	int				GetEmptyUpgradeInventoryFromIndex(WORD index, BYTE itemSize) const; //SPLIT ITEMS
	int				GetEmptyBookInventoryFromIndex(WORD index, BYTE itemSize) const; //SPLIT ITEMS
	int				GetEmptyStoneInventoryFromIndex(WORD index, BYTE itemSize) const; //SPLIT ITEMS
	int				GetEmptyFlowerInventoryFromIndex(WORD index, BYTE itemSize) const; //SPLIT ITEMS
	int				GetEmptyAttrInventoryFromIndex(WORD index, BYTE itemSize) const; //SPLIT ITEMS
	int				GetEmptyChestInventoryFromIndex(WORD index, BYTE itemSize) const; //SPLIT ITEMS
#endif
#endif
	int				GetEmptyDragonSoulInventory(LPITEM pItem) const;
	int				GetEmptyDragonSoulInventoryWithExceptions(LPITEM pItem, std::vector<WORD>& vec /*= -1*/) const;
	void			CopyDragonSoulItemGrid(std::vector<WORD>& vDragonSoulItemGrid) const;
#ifdef __ADDITIONAL_INVENTORY__
	int				GetSameUpgradeInventory(LPITEM pItem) const;
	int				GetSameBookInventory(LPITEM pItem) const;
	int				GetSameStoneInventory(LPITEM pItem) const;
	int				GetSameFlowerInventory(LPITEM pItem) const;
	int				GetSameAttrInventory(LPITEM pItem) const;
	int				GetSameChestInventory(LPITEM pItem) const;
	int				GetEmptyUpgradeInventory(LPITEM pItem) const;
	int				GetEmptyBookInventory(LPITEM pItem) const;
	int				GetEmptyStoneInventory(LPITEM pItem) const;
	int				GetEmptyFlowerInventory(LPITEM pItem) const;
	int				GetEmptyAttrInventory(LPITEM pItem) const;
	int				GetEmptyChestInventory(LPITEM pItem) const;
#endif
	int				CountEmptyInventory() const;

	int				CountSpecifyItem(DWORD vnum) const;
	void			RemoveSpecifyItem(DWORD vnum, DWORD count = 1);
	LPITEM			FindSpecifyItem(DWORD vnum) const;
#ifdef __ADDITIONAL_INVENTORY__
	LPITEM			FindSpecifyItemSpecial(DWORD vnum) const;
#endif
	LPITEM			FindItemByID(DWORD id) const;

	int				CountSpecifyTypeItem(BYTE type) const;
	void			RemoveSpecifyTypeItem(BYTE type, DWORD count = 1);

	bool			IsEquipUniqueItem(DWORD dwItemVnum) const;

	// CHECK_UNIQUE_GROUP
	bool			IsEquipUniqueGroup(DWORD dwGroupVnum) const;
	// END_OF_CHECK_UNIQUE_GROUP
	// End of Item

	bool			IsUnderRefine() const { return m_bUnderRefine; };

protected:
	bool			m_bNoOpenedShop;

	bool			m_bItemLoaded;
	int				m_iRefineAdditionalCell;
	bool			m_bUnderRefine;
	DWORD			m_dwRefineNPCVID;

public:
	////////////////////////////////////////////////////////////////////////////////////////
	// Money related
#ifdef __GOLD_LIMIT_REWORK__
	long long		GetGold() const { return m_points.gold; }
	void			SetGold(long long gold) { m_points.gold = gold; }
	void			GiveGold(long long iAmount);
#else
	INT				GetGold() const { return m_points.gold; }
	void			SetGold(INT gold) { m_points.gold = gold; }
	void			GiveGold(INT iAmount);
#endif
#ifdef __CHEQUE_SYSTEM__
	////////////////////////////////////////////////////////////////////////////////////////
	// Cheque related
	int				GetCheque() const { return m_points.cheque; }
	void			SetCheque(int cheque) { m_points.cheque = cheque; }
	// End of Cheque
#endif
	// End of Money
	////////////////////////////////////////////////////////////////////////////////////////
	// Shop related
public:
	void			SetShop(LPSHOP pkShop);
	LPSHOP			GetShop() const { return m_pkShop; }

	void			SetShopOwner(LPCHARACTER ch) { m_pkChrShopOwner = ch; }
	LPCHARACTER		GetShopOwner() const { return m_pkChrShopOwner; }

#ifdef __ITEM_COUNT_LIMIT__
	void			OpenMyShop(const char* c_pszSign, TShopItemTable* pTable, DWORD bItemCount);
#else
	void			OpenMyShop(const char* c_pszSign, TShopItemTable* pTable, BYTE bItemCount);
#endif

	LPSHOP			GetMyShop() const { return m_pkMyShop; }
	void			CloseMyShop();

protected:

	LPSHOP			m_pkShop;
	LPSHOP			m_pkMyShop;
	std::string		m_stShopSign;
	LPCHARACTER		m_pkChrShopOwner;
	// End of shop

	////////////////////////////////////////////////////////////////////////////////////////
	// Exchange related
public:
	bool			ExchangeStart(LPCHARACTER victim);
	void			SetExchange(CExchange* pkExchange);
	CExchange* GetExchange() const { return m_pkExchange; }

protected:
	CExchange* m_pkExchange;
	// End of Exchange

	////////////////////////////////////////////////////////////////////////////////////////
	// Battle
public:
	struct TBattleInfo
	{
#ifdef __DAMAGE_LIMIT_REWORK__
		long long iTotalDamage;
		long long iAggro;

		TBattleInfo(long long iTot, long long iAggr)
			: iTotalDamage(iTot), iAggro(iAggr)
#else
		int iTotalDamage;
		int iAggro;

		TBattleInfo(int iTot, int iAggr)
			: iTotalDamage(iTot), iAggro(iAggr)
#endif
		{}
	};
	typedef std::map<VID, TBattleInfo>	TDamageMap;

#ifdef __DAMAGE_LIMIT_REWORK__
	bool				Damage(LPCHARACTER pAttacker, long long dam, EDamageType type = DAMAGE_TYPE_NORMAL);
#else
	bool				Damage(LPCHARACTER pAttacker, int dam, EDamageType type = DAMAGE_TYPE_NORMAL);
#endif
	void				DeathPenalty(BYTE bExpLossPercent);
	void				ReviveInvisible(int iDur);

	bool				Attack(LPCHARACTER pkVictim, BYTE bType = 0);
	bool				IsAlive() const { return m_pointsInstant.position == POS_DEAD ? false : true; }
	bool				CanFight() const;

	bool				CanBeginFight() const;
	void				BeginFight(LPCHARACTER pkVictim);

	bool				IsStun() const;
	void				Stun();
	bool				IsDead() const;
	void				Dead(LPCHARACTER pkKiller = NULL, bool bImmediateDead = false);
#if defined(__GROWTH_PET_SYSTEM__) || defined(__GROWTH_MOUNT_SYSTEM__)
	void				SetImmortal(BYTE st) { m_stImmortalSt = st; };
	bool				IsImmortal() { return 1 == m_stImmortalSt; };
#endif

#ifdef __GROWTH_PET_SYSTEM__
	void				SetNewPetSkillCD(int s, DWORD time) { m_newpetskillcd[s] = time; };
	DWORD				GetNewPetSkillCD(int s) { return m_newpetskillcd[s]; };
#endif
#ifdef __ENABLE_KILL_EVENT_FIX__
	LPCHARACTER			GetMostAttacked();
#endif
	void				Reward(bool bItemDrop);
	void				RewardGold(LPCHARACTER pkAttacker);

	bool				Shoot(BYTE bType);
	void				FlyTarget(DWORD dwTargetVID, long x, long y, BYTE bHeader);

	void				ForgetMyAttacker();
	void				AggregateMonster();
#ifdef __STONE_DETECT_REWORK__
	bool				AggregateStone();
#endif
	void				AttractRanger();
	void				PullMonster();

	int					GetArrowAndBow(LPITEM* ppkBow, LPITEM* ppkArrow, int iArrowCount = 1);
	void				UseArrow(LPITEM pkArrow, DWORD dwArrowCount);

	void				AttackedByPoison(LPCHARACTER pkAttacker);
	void				RemovePoison();
#ifdef __WOLFMAN_CHARACTER__
	void				AttackedByBleeding(LPCHARACTER pkAttacker);
	void				RemoveBleeding();
#endif
	void				AttackedByFire(LPCHARACTER pkAttacker, int amount, int count);
	void				RemoveFire();
#ifdef __ALIGNMENT_REWORK__
	void				UpdateAlignment(int iAmount, bool hard = false);
#else
	void				UpdateAlignment(int iAmount);
#endif
	int					GetAlignment() const;
#ifdef __RANK_SYSTEM__
	void				ChangeRank(int iAmount);
	int					GetRank() const;
	void				RefreshRankBonus();
#endif
#ifdef __LANDRANK_SYSTEM__
	void				ChangeLandRank(int iAmount);
	int					GetLandRank() const;
	void				RefreshLandRankBonus();
#endif
#ifdef __REBORN_SYSTEM__
	void				ChangeReborn(int iAmount);
	int					GetReborn() const;
	void				RefreshRebornBonus();
#endif
#ifdef __TEAM_SYSTEM__
	void				ChangeTeam(int iAmount);
	int					GetTeam() const;
	void				RefreshTeamBonus();
#endif
#ifdef __MONIKER_SYSTEM__
	void				ChangeMoniker(const char* moniker);
	const char*			GetMoniker() const;
	void				RefreshMonikerBonus();
#endif
#ifdef ENABLE_LOVE_SYSTEM
	void				ChangeLove1(const char* love1);
	void				ChangeLove2(const char* love2);
	void				RefreshLoveBonus();

	const char*			GetLove1() const;
	const char*			GetLove2() const;
#endif // ENABLE_LOVE_SYSTEM
#ifdef ENABLE_WORD_SYSTEM
	void				ChangeWord(const char* word);
	const char*			GetWord() const;
	void				RefreshWordBonus();
#endif // ENABLE_WORD_SYSTEM
	int					GetRealAlignment() const;
	void				ShowAlignment(bool bShow);

	void				SetKillerMode(bool bOn);
	bool				IsKillerMode() const;
	void				UpdateKillerMode();

	BYTE				GetPKMode() const;
	void				SetPKMode(BYTE bPKMode);

	void				ItemDropPenalty(LPCHARACTER pkKiller);

#ifdef __DAMAGE_LIMIT_REWORK__
	void				UpdateAggrPoint(LPCHARACTER ch, EDamageType type, long long dam);
#else
	void				UpdateAggrPoint(LPCHARACTER ch, EDamageType type, int dam);
#endif

#ifdef __PET_SYSTEM_PROTO__
public:
	void				CheckPet();
#endif

public:
	BYTE GetComboIndex() const;

	void SkipComboAttackByTime(int interval);
	DWORD GetSkipComboAttackByTime() const;

protected:
	BYTE m_bComboIndex;
	DWORD m_dwSkipComboAttackByTime;

protected:
#ifdef __DAMAGE_LIMIT_REWORK__
	void				UpdateAggrPointEx(LPCHARACTER ch, EDamageType type, long long dam, TBattleInfo& info);
#else
	void				UpdateAggrPointEx(LPCHARACTER ch, EDamageType type, int dam, TBattleInfo& info);
#endif
	void				ChangeVictimByAggro(int iNewAggro, LPCHARACTER pNewVictim);

	DWORD				m_dwFlyTargetID;
	std::vector<DWORD>	m_vec_dwFlyTargets;
	TDamageMap			m_map_kDamage;
	DWORD				m_dwKillerPID;

	int					m_iAlignment;		// Lawful/Chaotic value -200000 ~ 200000
	int					m_iRealAlignment;
#ifdef __RANK_SYSTEM__
	int					m_iRank;
#endif
#ifdef __LANDRANK_SYSTEM__
	int					m_iLandRank;
#endif
#ifdef __REBORN_SYSTEM__
	int					m_iReborn;
#endif
#ifdef __TEAM_SYSTEM__
	int					m_iTeam;
#endif
#ifdef __MONIKER_SYSTEM__
	char m_moniker[MONIKER_MAX_LEN];
#endif
#ifdef ENABLE_LOVE_SYSTEM
	char m_love1[2];
	char m_love2[2];
#endif // ENABLE_LOVE_SYSTEM
#ifdef ENABLE_WORD_SYSTEM
	char m_word[24+1];
#endif
	int					m_iKillerModePulse;
	BYTE				m_bPKMode;

	// Aggro
	DWORD				m_dwLastVictimSetTime;
	int					m_iMaxAggro;
	// End of Battle

	// Stone
public:
	void				SetStone(LPCHARACTER pkChrStone);
	void				ClearStone();
	void				DetermineDropMetinStone();
	DWORD				GetDropMetinStoneVnum() const { return m_dwDropMetinStone; }
	BYTE				GetDropMetinStonePct() const { return m_bDropMetinStonePct; }

protected:
	LPCHARACTER			m_pkChrStone;
	CHARACTER_SET		m_set_pkChrSpawnedBy;
	DWORD				m_dwDropMetinStone;
	BYTE				m_bDropMetinStonePct;
	// End of Stone

public:
	enum
	{
		SKILL_UP_BY_POINT,
		SKILL_UP_BY_BOOK,
		SKILL_UP_BY_TRAIN,

		// ADD_GRANDMASTER_SKILL
		SKILL_UP_BY_QUEST,
		// END_OF_ADD_GRANDMASTER_SKILL
	};

	void				SkillLevelPacket();
	void				SkillLevelUp(DWORD dwVnum, BYTE bMethod = SKILL_UP_BY_POINT);
	bool				SkillLevelDown(DWORD dwVnum);
	// ADD_GRANDMASTER_SKILL
	bool				UseSkill(DWORD dwVnum, LPCHARACTER pkVictim, bool bUseGrandMaster = true);
	void				ResetSkill();
	void				SetSkillLevel(DWORD dwVnum, BYTE bLev);
	int					GetUsedSkillMasterType(DWORD dwVnum);

	bool				IsLearnableSkill(DWORD dwSkillVnum) const;
	// END_OF_ADD_GRANDMASTER_SKILL

	bool				CheckSkillHitCount(const BYTE SkillID, const VID dwTargetVID);
	bool				CanUseSkill(DWORD dwSkillVnum) const;
	bool				IsUsableSkillMotion(DWORD dwMotionIndex) const;
	int					GetSkillLevel(DWORD dwVnum) const;
	int					GetSkillMasterType(DWORD dwVnum) const;
	int					GetSkillPower(DWORD dwVnum, BYTE bLevel = 0) const;

	time_t				GetSkillNextReadTime(DWORD dwVnum) const;
	void				SetSkillNextReadTime(DWORD dwVnum, time_t time);
	void				SkillLearnWaitMoreTimeMessage(DWORD dwVnum);

	void				ComputePassiveSkill(DWORD dwVnum);
	int					ComputeSkill(DWORD dwVnum, LPCHARACTER pkVictim, BYTE bSkillLevel = 0);
#ifdef __WOLFMAN_CHARACTER__
	int					ComputeSkillParty(DWORD dwVnum, LPCHARACTER pkVictim, BYTE bSkillLevel = 0);
#endif
	int					ComputeSkillAtPosition(DWORD dwVnum, const PIXEL_POSITION& posTarget, BYTE bSkillLevel = 0);
	void				ComputeSkillPoints();

	void				SetSkillGroup(BYTE bSkillGroup);
	BYTE				GetSkillGroup() const { return m_points.skill_group; }

	int					ComputeCooltime(int time);

	void				GiveRandomSkillBook();

	void				DisableCooltime();
	bool				LearnSkillByBook(DWORD dwSkillVnum, BYTE bProb = 0);
	bool				LearnGrandMasterSkill(DWORD dwSkillVnum);
#ifdef __SAGE_SKILL__
	bool				LearnSageMasterSkill(DWORD dwSkillVnum);
#endif
#ifdef __EXPERT_SKILL__
	bool				LearnExpertMasterSkill(DWORD dwSkillVnum);
	bool				LearnDestansiMasterSkill(DWORD dwSkillVnum);
	bool				LearnTanrisalMasterSkill(DWORD dwSkillVnum);
#endif
	bool				SkillIsOnCooldown(int vnum) { return m_SkillUseInfo[vnum].IsOnCooldown(vnum); }

private:
	bool				m_bDisableCooltime;
	DWORD				m_dwLastSkillTime;
	// End of Skill

	// MOB_SKILL
public:
	bool				HasMobSkill() const;
	size_t				CountMobSkill() const;
	const TMobSkillInfo* GetMobSkill(unsigned int idx) const;
	bool				CanUseMobSkill(unsigned int idx) const;
	bool				UseMobSkill(unsigned int idx);
	void				ResetMobSkillCooltime();
protected:
	DWORD				m_adwMobSkillCooltime[MOB_SKILL_MAX_NUM];
	// END_OF_MOB_SKILL

	// for SKILL_MUYEONG
public:
	void				StartMuyeongEvent();
	void				StopMuyeongEvent();

private:
	LPEVENT				m_pkMuyeongEvent;

	// for SKILL_CHAIN lighting
public:
	int					GetChainLightningIndex() const { return m_iChainLightingIndex; }
	void				IncChainLightningIndex() { ++m_iChainLightingIndex; }
	void				AddChainLightningExcept(LPCHARACTER ch) { m_setExceptChainLighting.insert(ch); }
	void				ResetChainLightningIndex() { m_iChainLightingIndex = 0; m_setExceptChainLighting.clear(); }
	int					GetChainLightningMaxCount() const;
	const CHARACTER_SET& GetChainLightingExcept() const { return m_setExceptChainLighting; }

private:
	int					m_iChainLightingIndex;
	CHARACTER_SET m_setExceptChainLighting;

	// for SKILL_EUNHYUNG
public:
	void				SetAffectedEunhyung();
	void				ClearAffectedEunhyung() { m_dwAffectedEunhyungLevel = 0; }
	bool				GetAffectedEunhyung() const { return m_dwAffectedEunhyungLevel; }

private:
	DWORD				m_dwAffectedEunhyungLevel;

	//
	// Skill levels
	//
protected:
	TPlayerSkill* m_pSkillLevels;
	boost::unordered_map<BYTE, int>		m_SkillDamageBonus;
	std::map<int, TSkillUseInfo>	m_SkillUseInfo;

	////////////////////////////////////////////////////////////////////////////////////////
	// AI related
public:
	void			AssignTriggers(const TMobTable* table);
	LPCHARACTER		GetVictim() const;
	void			SetVictim(LPCHARACTER pkVictim);
	LPCHARACTER		GetNearestVictim(LPCHARACTER pkChr);
	LPCHARACTER		GetProtege() const;

	bool			Follow(LPCHARACTER pkChr, float fMinimumDistance = 150.0f);
	bool			Return();
	bool			IsGuardNPC() const;
	bool			IsChangeAttackPosition(LPCHARACTER target) const;
	void			ResetChangeAttackPositionTime() { m_dwLastChangeAttackPositionTime = get_dword_time() - AI_CHANGE_ATTACK_POISITION_TIME_NEAR; }
	void			SetChangeAttackPositionTime() { m_dwLastChangeAttackPositionTime = get_dword_time(); }

	bool			OnIdle();

	void			OnClick(LPCHARACTER pkChrCauser);

	VID				m_kVIDVictim;

protected:
	DWORD			m_dwLastChangeAttackPositionTime;
	CTrigger		m_triggerOnClick;
	// End of AI

	////////////////////////////////////////////////////////////////////////////////////////
	// Target
protected:
	LPCHARACTER				m_pkChrTarget;
	CHARACTER_SET	m_set_pkChrTargetedBy;

public:
	void				SetTarget(LPCHARACTER pkChrTarget);
	void				BroadcastTargetPacket();
	void				ClearTarget();
	void				CheckTarget();
	LPCHARACTER			GetTarget() const { return m_pkChrTarget; }

	////////////////////////////////////////////////////////////////////////////////////////
	// Safebox
public:
	int					GetSafeboxSize() const;
	void				QuerySafeboxSize();
	void				SetSafeboxSize(int size);

	CSafebox* GetSafebox() const;
#ifdef __GOLD_LIMIT_REWORK__
	void				LoadSafebox(int iSize, long long llGold, int iItemCount, TPlayerItem* pItems);
#else
	void				LoadSafebox(int iSize, DWORD dwGold, int iItemCount, TPlayerItem* pItems);
#endif
	void				ChangeSafeboxSize(BYTE bSize);
	void				CloseSafebox();

	void				ReqSafeboxLoad(const char* pszPassword);

	void				CancelSafeboxLoad(void) { m_bOpeningSafebox = false; }

	void				SetMallLoadTime(int t) { m_iMallLoadTime = t; }
	int					GetMallLoadTime() const { return m_iMallLoadTime; }

	CSafebox* GetMall() const;
	void				LoadMall(int iItemCount, TPlayerItem* pItems);
	void				CloseMall();

	void				SetSafeboxOpenPosition();
	float				GetDistanceFromSafeboxOpen() const;

protected:
	CSafebox* m_pkSafebox;
	int					m_iSafeboxSize;
	int					m_iSafeboxLoadTime;
	bool				m_bOpeningSafebox;

	CSafebox* m_pkMall;
	int					m_iMallLoadTime;

	PIXEL_POSITION		m_posSafeboxOpen;

	////////////////////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////////////////////////
	// Mounting
public:
	void				MountVnum(DWORD vnum);
	DWORD				GetMountVnum() const { return m_dwMountVnum; }
	DWORD				GetLastMountTime() const { return m_dwMountTime; }

	bool				CanUseHorseSkill();

	// Horse
	virtual	void		SetHorseLevel(int iLevel);

	virtual	bool		StartRiding();
	virtual	bool		StopRiding();

	virtual	DWORD		GetMyHorseVnum() const;

	virtual	void		HorseDie();
	virtual bool		ReviveHorse();

	virtual void		SendHorseInfo();
	virtual	void		ClearHorseInfo();

	void				HorseSummon(bool bSummon, bool bFromFar = false, DWORD dwVnum = 0, const char* name = 0);

	LPCHARACTER			GetHorse() const { return m_chHorse; }
	LPCHARACTER			GetRider() const; // rider on horse
	void				SetRider(LPCHARACTER ch);

	bool				IsRiding() const;

#ifdef __PET_SYSTEM__
public:
	CPetSystem* GetPetSystem() { return m_petSystem; }

protected:
	CPetSystem* m_petSystem;

public:
#endif

#ifdef __GROWTH_PET_SYSTEM__
public:
	CNewPetSystem* GetNewPetSystem() { return m_newpetSystem; }

protected:
	CNewPetSystem* m_newpetSystem;

public:
#endif

#ifdef __GROWTH_PET_SYSTEM__
public:
	CNewMountSystem* GetNewMountSystem() { return m_newmountSystem; }

protected:
	CNewMountSystem* m_newmountSystem;

public:
#endif

protected:
	LPCHARACTER			m_chHorse;
	LPCHARACTER			m_chRider;

	DWORD				m_dwMountVnum;
	DWORD				m_dwMountTime;

	BYTE				m_bSendHorseLevel;
	BYTE				m_bSendHorseHealthGrade;
	BYTE				m_bSendHorseStaminaGrade;

	// Empire

public:
	void 				SetEmpire(BYTE bEmpire);
	BYTE				GetEmpire() const { return m_bEmpire; }

protected:
	BYTE				m_bEmpire;

	////////////////////////////////////////////////////////////////////////////////////////
	// Regen
public:
	void				SetRegen(LPREGEN pkRegen);
#ifdef __REGEN_REWORK__
	LPREGEN				GetRegen();
#endif
protected:
	PIXEL_POSITION			m_posRegen;
	float				m_fRegenAngle;
	LPREGEN				m_pkRegen;
	size_t				regen_id_; // to help dungeon regen identification
	// End of Regen

	////////////////////////////////////////////////////////////////////////////////////////
	// Resists & Proofs
public:
	bool				CannotMoveByAffect() const;
	bool				IsImmune(DWORD dwImmuneFlag);
	void				SetImmuneFlag(DWORD dw) { m_pointsInstant.dwImmuneFlag = dw; }

protected:
	void				ApplyMobAttribute(const TMobTable* table);
	// End of Resists & Proofs

	////////////////////////////////////////////////////////////////////////////////////////
	// QUEST
	//
public:
	void				SetQuestNPCID(DWORD vid);
	DWORD				GetQuestNPCID() const { return m_dwQuestNPCVID; }
	LPCHARACTER			GetQuestNPC() const;

	void				SetQuestItemPtr(LPITEM item);
	void				ClearQuestItemPtr();
	LPITEM				GetQuestItemPtr() const;

	void				SetQuestBy(DWORD dwQuestVnum) { m_dwQuestByVnum = dwQuestVnum; }
	DWORD				GetQuestBy() const { return m_dwQuestByVnum; }

	int					GetQuestFlag(const std::string& flag) const;
	void				SetQuestFlag(const std::string& flag, int value);

	void				ConfirmWithMsg(const char* szMsg, int iTimeout, DWORD dwRequestPID);

private:
	DWORD				m_dwQuestNPCVID;
	DWORD				m_dwQuestByVnum;
	LPITEM				m_pQuestItem;

	// Events
public:
	bool				StartStateMachine(int iPulse = 1);
	void				StopStateMachine();
	void				UpdateStateMachine(DWORD dwPulse);
	void				SetNextStatePulse(int iPulseNext);

	void				UpdateCharacter(DWORD dwPulse);

protected:
	DWORD				m_dwNextStatePulse;

	// Warp Character
public:
	void				StartWarpNPCEvent();

public:
	void				StartSaveEvent();
	void				StartRecoveryEvent();
	void				StartDestroyWhenIdleEvent();

	LPEVENT				m_pkDeadEvent;
	LPEVENT				m_pkStunEvent;
	LPEVENT				m_pkSaveEvent;
	LPEVENT				m_pkRecoveryEvent;
	LPEVENT				m_pkTimedEvent;
	LPEVENT				m_pkFishingEvent;
	LPEVENT				m_pkAffectEvent;
	LPEVENT				m_pkPoisonEvent;
#ifdef __WOLFMAN_CHARACTER__
	LPEVENT				m_pkBleedingEvent;
#endif
	LPEVENT				m_pkFireEvent;
	LPEVENT				m_pkWarpNPCEvent;
	//DELAYED_WARP
	//END_DELAYED_WARP

	// MINING
	LPEVENT				m_pkMiningEvent;
	// END_OF_MINING
	LPEVENT				m_pkWarpEvent;
	LPEVENT				m_pkDestroyWhenIdleEvent;
	LPEVENT				m_pkPetSystemUpdateEvent;
#ifdef __GROWTH_PET_SYSTEM__
	LPEVENT				m_pkNewPetSystemUpdateEvent;
	LPEVENT				m_pkNewPetSystemExpireEvent;
#endif
#ifdef __GROWTH_MOUNT_SYSTEM__
	LPEVENT				m_pkNewMountSystemUpdateEvent;
	LPEVENT				m_pkNewMountSystemExpireEvent;
#endif
	bool IsWarping() const { return m_pkWarpEvent ? true : false; }

	bool				m_bHasPoisoned;
#ifdef __WOLFMAN_CHARACTER__
	bool				m_bHasBled;
#endif

	const CMob* m_pkMobData;
	CMobInstance* m_pkMobInst;

	std::map<int, LPEVENT> m_mapMobSkillEvent;

	friend struct FuncSplashDamage;
	friend struct FuncSplashAffect;
	friend class CFuncShoot;

public:
	int				GetPremiumRemainSeconds(BYTE bType) const;

private:
	int				m_aiPremiumTimes[PREMIUM_MAX_NUM];

	// NEW_HAIR_STYLE_ADD
public:
	bool ItemProcess_Hair(LPITEM item, int iDestCell);
	// END_NEW_HAIR_STYLE_ADD

public:
	void ClearSkill();
	void ClearSubSkill();

	// RESET_ONE_SKILL
	bool ResetOneSkill(DWORD dwVnum);
	// END_RESET_ONE_SKILL

private:
#ifdef __DAMAGE_LIMIT_REWORK__
	void SendDamagePacket(LPCHARACTER pAttacker, long long Damage, BYTE DamageFlag);
#else
	void SendDamagePacket(LPCHARACTER pAttacker, int Damage, BYTE DamageFlag);
#endif

	//PREVENT_TRADE_WINDOW
public:
	bool	IsOpenSafebox() const { return m_isOpenSafebox ? true : false; }
	void 	SetOpenSafebox(bool b) { m_isOpenSafebox = b; }

	int		GetSafeboxLoadTime() const { return m_iSafeboxLoadTime; }
	void	SetSafeboxLoadTime() { m_iSafeboxLoadTime = thecore_pulse(); }
	//END_PREVENT_TRADE_WINDOW
private:
	bool	m_isOpenSafebox;

public:
	int		GetSkillPowerByLevel(int level, bool bMob = false) const;

	//PREVENT_REFINE_HACK
	int		GetRefineTime() const { return m_iRefineTime; }
	void	SetRefineTime() { m_iRefineTime = thecore_pulse(); }
	int		m_iRefineTime;
	//END_PREVENT_REFINE_HACK

	//PREVENT_PORTAL_AFTER_EXCHANGE
	int		GetExchangeTime() const { return m_iExchangeTime; }
	void	SetExchangeTime() { m_iExchangeTime = thecore_pulse(); }
	int		m_iExchangeTime;
	//END_PREVENT_PORTAL_AFTER_EXCHANGE

	int 	m_iMyShopTime;
	int		GetMyShopTime() const { return m_iMyShopTime; }
	void	SetMyShopTime() { m_iMyShopTime = thecore_pulse(); }

	bool	IsHack(bool bSendMsg = true, bool bCheckShopOwner = true, int limittime = g_nPortalLimitTime);

	void Say(const std::string& s);

public:
	// by mhh
	LPITEM*	GetCubeItem() { return (m_PlayerSlots) ? m_PlayerSlots->pCubeItems : nullptr; }
	bool IsCubeOpen() const { return (m_pointsInstant.pCubeNpc ? true : false); }
	void SetCubeNpc(LPCHARACTER npc) { m_pointsInstant.pCubeNpc = npc; }

public:
	bool IsSiegeNPC() const;

private:
	int		m_deposit_pulse;

public:
	void	UpdateDepositPulse();
	bool	CanDeposit() const;

private:
	void	__OpenPrivateShop();

private:
	std::string m_strNewName;

public:
	const std::string GetNewName() const { return this->m_strNewName; }
	void SetNewName(const std::string name) { this->m_strNewName = name; }

public:
	void GoHome();

private:
	std::set<DWORD>	m_known_guild;

public:
	void SendGuildName(CGuild* pGuild);
	void SendGuildName(DWORD dwGuildID);

private:
	DWORD m_dwLogOffInterval;

public:
	DWORD GetLogOffInterval() const { return m_dwLogOffInterval; }

public:
	bool UnEquipSpecialRideUniqueItem();

	bool CanWarp() const;

public:
	void AutoRecoveryItemProcess(const EAffectTypes);

public:
	void BuffOnAttr_AddBuffsFromItem(LPITEM pItem);
	void BuffOnAttr_RemoveBuffsFromItem(LPITEM pItem);

private:
	void BuffOnAttr_ValueChange(BYTE bType, BYTE bOldValue, BYTE bNewValue);
	void BuffOnAttr_ClearAll();

	typedef std::map <BYTE, CBuffOnAttributes*> TMapBuffOnAttrs;
	TMapBuffOnAttrs m_map_buff_on_attrs;

public:
	void SetArmada() { cannot_dead = true; }
	void ResetArmada() { cannot_dead = false; }
private:
	bool cannot_dead;

#ifdef __PET_SYSTEM__
private:
	bool m_bIsPet;
public:
	void SetPet() { m_bIsPet = true; }
	bool IsPet() { return m_bIsPet; }
#endif
#ifdef __GROWTH_PET_SYSTEM__
private:
	bool m_bIsNewPet;
	int m_eggvid;
public:
	void SetNewPet() { m_bIsNewPet = true; }
	bool IsNewPet() const { return m_bIsNewPet ? true : false; }
	void SetEggVid(int vid) { m_eggvid = vid; }
	int GetEggVid() { return m_eggvid; }
#endif

#ifdef __GROWTH_MOUNT_SYSTEM__
private:
	bool m_bIsNewMount;
	int m_Mounteggvid;
public:
	void SetNewMount() { m_bIsNewMount = true; }
	bool IsNewMount() const { return m_bIsNewMount ? true : false; }
	void SetMountEggVid(int vid) { m_Mounteggvid = vid; }
	int GetMountEggVid() { return m_Mounteggvid; }
#endif

#ifdef __MOUNT_COSTUME_SYSTEM__
private:
	bool m_bIsMount;
public:
	void SetMount() { m_bIsMount = true; }
	bool IsMount() { return m_bIsMount; }
#endif

private:
	float m_fAttMul;
	float m_fDamMul;
public:
	float GetAttMul() { return this->m_fAttMul; }
	void SetAttMul(float newAttMul) { this->m_fAttMul = newAttMul; }
	float GetDamMul() { return this->m_fDamMul; }
	void SetDamMul(float newDamMul) { this->m_fDamMul = newDamMul; }

private:
	bool IsValidItemPosition(TItemPos Pos) const;

public:

	void	DragonSoul_Initialize();

	bool	DragonSoul_IsQualified() const;
	void	DragonSoul_GiveQualification();

	int		DragonSoul_GetActiveDeck() const;
	bool	DragonSoul_IsDeckActivated() const;
	bool	DragonSoul_ActivateDeck(int deck_idx);

	void	DragonSoul_DeactivateAll();

	//

	void	DragonSoul_CleanUp();
#ifdef __DS_SET_BONUS__
	void	DragonSoul_HandleSetBonus();
#endif
public:
	bool		DragonSoul_RefineWindow_Open(LPENTITY pEntity);
	bool		DragonSoul_RefineWindow_Close();
	LPENTITY	DragonSoul_RefineWindow_GetOpener() { return  m_pointsInstant.m_pDragonSoulRefineWindowOpener; }
	bool		DragonSoul_RefineWindow_CanRefine();
private:
	timeval		m_tvLastSyncTime;
	int			m_iSyncHackCount;
public:
	void			SetLastSyncTime(const timeval& tv) { memcpy(&m_tvLastSyncTime, &tv, sizeof(timeval)); }
	const timeval& GetLastSyncTime() { return m_tvLastSyncTime; }
	void			SetSyncHackCount(int iCount) { m_iSyncHackCount = iCount; }
	int				GetSyncHackCount() { return m_iSyncHackCount; }
#ifdef __ACCE_SYSTEM__
protected:
	bool	m_bAcceCombination, m_bAcceAbsorption;

public:
	bool	isAcceOpened(bool bCombination) { return bCombination ? m_bAcceCombination : m_bAcceAbsorption; }
	bool	isAcceOpened(bool bCombination) const { return bCombination ? m_bAcceCombination : m_bAcceAbsorption; }
	void	OpenAcce(bool bCombination);
	void	CloseAcce();
	void	ClearAcceMaterials();
	bool	CleanAcceAttr(LPITEM pkItem, LPITEM pkTarget);
	std::vector<LPITEM>	GetAcceMaterials();
	const TItemPosEx* GetAcceMaterialsInfo();
	void	SetAcceMaterial(int pos, LPITEM ptr);
	bool	AcceIsSameGrade(long lGrade);
#ifdef __GOLD_LIMIT_REWORK__
	long long	GetAcceCombinePrice(long lGrade);
#else
	DWORD	GetAcceCombinePrice(long lGrade);
#endif
	void	GetAcceCombineResult(DWORD& dwItemVnum, DWORD& dwMinAbs, DWORD& dwMaxAbs);
	BYTE	CheckEmptyMaterialSlot();
	void	AddAcceMaterial(TItemPos tPos, BYTE bPos);
	void	RemoveAcceMaterial(BYTE bPos);
	BYTE	CanRefineAcceMaterials();
	void	RefineAcceMaterials();
	bool	IsAcceOpen() { return m_bAcceCombination || m_bAcceAbsorption; }
	int		GetAcceTime() const { return m_iAcceTime; }
	void	SetAcceTime() { m_iAcceTime = thecore_pulse(); }
	int		m_iAcceTime;
#endif
#ifdef __CHANNEL_CHANGER__
public:
	bool	SwitchChannel(long newAddr, WORD newPort);
	bool	StartChannelSwitch(long newAddr, WORD newPort);
#endif
#ifdef __ANCIENT_ATTR_ITEM__
public:
	bool	UseItemNewAttribute(TItemPos source_pos, TItemPos target_pos, BYTE* bValues);
#endif
protected:
	int		LastCampFireUse;

#ifdef __INGAME_MALL__
public:
	DWORD			GetDragonCoin();
	DWORD			GetDragonMark();
	void			SetDragonCoin(DWORD amount);
	void			SetDragonMark(DWORD amount);
	void			RefreshDragonCoin();
	void			SetRefreshDragonCoinTime(DWORD dwTime) { m_dwRefreshDragonCoinTime = dwTime; };
	bool			IsRefreshDragonCoinTimeOut();
	void			UpdateShop();
protected:
	DWORD			m_dwRefreshDragonCoinTime;
#endif
#ifdef __ITEM_CHANGELOOK__
protected:
	bool	m_bChangeLook;
#ifdef __MOUNT_CHANGELOOK__
	bool	m_bMountChangeLook;
#endif
public:
	bool	isChangeLookOpened() { return m_bChangeLook; }
	bool	isChangeLookOpened() const { return m_bChangeLook; }
#ifdef __MOUNT_CHANGELOOK__
	void	ChangeLookWindow(bool bOpen = false, bool bRequest = false, bool bMount = false);
#else
	void	ChangeLookWindow(bool bOpen = false, bool bRequest = false);
#endif
	void	ClearClWindowMaterials();
	LPITEM* GetClWindowMaterials() { return (m_PlayerSlots) ? m_PlayerSlots->pClMaterials : nullptr; }
	BYTE	CheckClEmptyMaterialSlot();
	void	AddClMaterial(TItemPos tPos, BYTE bPos);
	void	RemoveClMaterial(BYTE bPos);
	void	RefineClMaterials();
	bool	CleanTransmutation(LPITEM pkItem, LPITEM pkTarget);

	int		m_iChangeLookTime;
	int		GetChangeLookTime() const { return m_iChangeLookTime; }
	void	SetChangeLookTime() { m_iChangeLookTime = thecore_pulse(); }
#endif
#ifdef __BRAVERY_CAPE_REWORK__
public:
	int		GetBraveryCapeUseCount() const { return m_iBraveryCapeUseCount; }
	void	SetBraveryCapeUseCount(int value) { m_iBraveryCapeUseCount = value; }
	DWORD	GetBraveryCapeUseTime() const { return m_dwBraveryCapeUseTime; }
	void	SetBraveryCapeUseTime(DWORD value) { m_dwBraveryCapeUseTime = value; }
protected:
	int		m_iBraveryCapeUseCount;
	DWORD	m_dwBraveryCapeUseTime;
#endif
#ifdef __7AND8TH_SKILLS__
public:
	bool SkillCanUp(DWORD dwVnum);
#endif
#ifdef __CHECK_ITEMS_ON_TELEPORT__
public:
	void CheckTeleportItems();
#endif
#ifdef __SKILL_COLOR__
public:
	void			SetSkillColor(DWORD* dwSkillColor);
	DWORD* GetSkillColor() { return m_dwSkillColor[0]; }

protected:
	DWORD			m_dwSkillColor[ESkillColorLength::MAX_SKILL_COUNT + ESkillColorLength::MAX_BUFF_COUNT][ESkillColorLength::MAX_EFFECT_COUNT];
#endif
#ifdef __DSS_ACTIVE_EFFECT__
public:
	DWORD			GetDragonSoulActivateTime() { return m_dwDragonSoulActivateTime; };
	void			SetDragonSoulActivateTime(DWORD val) { m_dwDragonSoulActivateTime = val; };
protected:
	DWORD			m_dwDragonSoulActivateTime;
#endif
public:
	bool IsInSafezone() const;
#ifdef __ALIGNMENT_REWORK__
public:
	int	GetAlignGrade();
	void	RefreshAlignBonus();
#endif
protected:
	int	LastStatResetUse;
#ifdef __BIOLOG_SYSTEM__
public:
	void			OpenBiologWindow();
	void			SetBiologItem(bool all, bool useItem1, bool useItem2, bool useItem3);
	void			SetGrandBiolog(BYTE level);
#endif
#ifdef __SCP1453_EXTENSIONS__
public:
	void			SetAllSkillLevel(BYTE level);
#endif
#ifdef __SKILL_SET_BONUS__
	int				GetMinSkillGrade();
	void			UpdateSkillSetBonus();
#endif // __SKILL_SET_BONUS__
#ifdef __LEVEL_SET_BONUS__
	bool			CheckLevelSetBonus();
#endif // __LEVEL_SET_BONUS__
#ifdef __ITEM_SET_BONUS__
	bool			CheckItemSetBonus();
#endif // __ITEM_SET_BONUS__
public:
	void				SetLastFriendRequestTime() { LastFriendRequestTime = thecore_pulse(); }
	int					GetLastFriendRequestTime() { return LastFriendRequestTime; }
protected:
	int				LastFriendRequestTime;
#ifdef __FAST_CHEQUE_TRANSFER__
public:
	void	WonExchange(BYTE bOption, DWORD wValue);
#endif
#ifdef __GAME_MASTER_UTILITY__
public:
	bool	MasterCanAction(BYTE bAction);
#endif
public:
	bool	CanAct(bool bSendMsg = true, bool bCheckTime = true, BYTE passVarCheckValue = 0, LPCHARACTER chMessager = NULL, DWORD varflags = VAR_SAFEBOX | VAR_EXCHANGE | VAR_MYSHOP | VAR_REFINE | VAR_ACCE | VAR_CHANGELOOK | VAR_OFFLINESHOP);
	void	SetVarTime(BYTE varType) { m_iVarTime[varType] = thecore_pulse(); };
	void	SetVarValue(BYTE varType, bool varValue) { m_bVarValue[varType] = varValue; };
	int		GetVarTime(BYTE varType) { return m_iVarTime[varType]; };
	int		GetVarTimeConst(BYTE varType) const { return m_iVarTime[varType]; };
	int		GetVarValue(BYTE varType) { return m_bVarValue[varType]; };
	int		GetVarValueConst(BYTE varType) const { return m_bVarValue[varType]; };
protected:
	int		m_iVarTime[VAR_TIME_MAX_NUM];
	bool	m_bVarValue[VAR_VAL_MAX_NUM];
public:
	DWORD	GetPartMainWithoutCostume() const;
#ifdef __WEAPON_COSTUME_SYSTEM__
	DWORD	GetPartWeaponWithoutCostume() const;
#endif // __WEAPON_COSTUME_SYSTEM__
#ifdef __MOUNT_COSTUME_SYSTEM__
public:
	CMountSystem* GetMountSystem() { return m_mountSystem; }
	void	MountSummon(LPITEM mountItem);
	void	MountUnsummon(LPITEM mountItem);
	void	CheckMount();
	bool	IsRidingMount();
protected:
	CMountSystem* m_mountSystem;
#endif
public:
	DWORD	GetLastHackLogTime() { return m_dwLastHackLogTime; };
	void	SetLastHackLogTime(DWORD time) { m_dwLastHackLogTime = time; };
protected:
	DWORD	m_dwLastHackLogTime;
#ifdef __MARTY_ANTI_CMD_FLOOD__
	private:
		int m_dwCmdAntiFloodPulse;
		DWORD m_dwCmdAntiFloodCount;
	public:
		int GetCmdAntiFloodPulse(){return m_dwCmdAntiFloodPulse;}
		DWORD GetCmdAntiFloodCount(){return m_dwCmdAntiFloodCount;}
		DWORD IncreaseCmdAntiFloodCount(){return ++m_dwCmdAntiFloodCount;}
		void SetCmdAntiFloodPulse(int dwPulse){m_dwCmdAntiFloodPulse=dwPulse;}
		void SetCmdAntiFloodCount(DWORD dwCount){m_dwCmdAntiFloodCount=dwCount;}
#endif

protected:
	// ItemUse Counter
	int m_iLastItemUsePulse;
	int m_iItemUseCounter;
public:
	// ItemUse Counter
	void ClearItemUseCounter(void) { m_iItemUseCounter = 0; }
	void IncreaseItemUseCounter(void) { m_iItemUseCounter++; }
	void SetLastItemUsePulse(void);
	int GetItemUseCounter(void) const { return m_iItemUseCounter; }
	int GetLastItemUsePulse(void) const { return m_iLastItemUsePulse; }

#ifdef __DUNGEON_LIMIT__
protected:
	int m_iDungeonTimeIndex;
public:
	int GetDungeonTimeIndex() { return m_iDungeonTimeIndex; }
	void RefreshDungeonTimeIndex();
	bool UpdateDungeonTime();
#endif // __DUNGEON_LIMIT__

#ifdef __OFFLINE_SHOP__
	void ClearShopSearchCounter(void) { m_iShopSearchCounter = 0; }
	void IncreaseShopSearchCounter(void) { m_iShopSearchCounter++; }
	void SetLastShopSearchPulse(void);
	int32_t GetShopSearchCounter(void) const { return m_iShopSearchCounter; }
	int32_t GetLastShopSearchPulse(void) const { return m_iLastShopSearchPulse; }
protected:
	int32_t						m_iLastShopSearchPulse;
	int32_t						m_iShopSearchCounter;
#endif // __OFFLINE_SHOP__
#ifdef ENABLE_PLAYER_STATISTICS
public:
	void			SendPlayerStatisticsPacket();

	void			AddToKilledShinsoo() { m_playerstatistics.iKilledShinsoo += 1; }
	int				GetKilledShinsoo() const { return m_playerstatistics.iKilledShinsoo; }

	void			AddToKilledChunjo() { m_playerstatistics.iKilledChunjo += 1; }
	int				GetKilledChunjo() const { return m_playerstatistics.iKilledChunjo; }

	void			AddToKilledJinno() { m_playerstatistics.iKilledJinno += 1; }
	int				GetKilledJinno() const { return m_playerstatistics.iKilledJinno; }

	void			AddToTotalKill() { m_playerstatistics.iTotalKill += 1; }
	int				GetTotalKill() const { return m_playerstatistics.iTotalKill; }

	void			AddToDuelWon() { m_playerstatistics.iDuelWon += 1; }
	int				GetDuelWon() const { return m_playerstatistics.iDuelWon; }

	void			AddToDuelLost() { m_playerstatistics.iDuelLost += 1; }
	int				GetDuelLost() const { return m_playerstatistics.iDuelLost; }

	void			AddToKilledMonster() { m_playerstatistics.iKilledMonster += 1; }
	int				GetKilledMonster() const { return m_playerstatistics.iKilledMonster; }

	void			AddToKilledStone() { m_playerstatistics.iKilledStone += 1; }
	int				GetKilledStone() const { return m_playerstatistics.iKilledStone; }

	void			AddToKilledBoss() { m_playerstatistics.iKilledBoss += 1; }
	int				GetKilledBoss() const { return m_playerstatistics.iKilledBoss; }

	void			AddToCompletedDungeon() { m_playerstatistics.iCompletedDungeon += 1; }
	int				GetCompletedDungeon() const { return m_playerstatistics.iCompletedDungeon; }

	void			AddToTakedFish() { m_playerstatistics.iTakedFish += 1; }
	int				GetTakedFish() const { return m_playerstatistics.iTakedFish; }

	void			SetBestStoneDamage(long long iDamage) { m_playerstatistics.iBestStoneDamage = iDamage; }
	long long		GetBestStoneDamage() const { return m_playerstatistics.iBestStoneDamage; }

	void			SetBestBossDamage(long long iDamage) { m_playerstatistics.iBestBossDamage = iDamage; }
	long long		GetBestBossDamage() const { return m_playerstatistics.iBestBossDamage; }
#endif
#ifdef __BATTLE_PASS_SYSTEM__
	typedef std::list<TPlayerBattlePassMission*> ListBattlePassMap;
public:
	void LoadBattlePass(DWORD dwCount, TPlayerBattlePassMission* data);
	DWORD GetMissionProgress(DWORD dwMissionID, DWORD dwBattlePassID);
	void UpdateMissionProgress(DWORD dwMissionID, DWORD dwBattlePassID, DWORD dwUpdateValue, DWORD dwTotalValue, bool isOverride = false);
	void SkipBattlePassMission(DWORD dwMissionID, DWORD dwBattlePassID);
	bool IsCompletedMission(BYTE bMissionType, DWORD dwBattlePassID);
	DWORD GetNextMissionByType(BYTE bMissionType, BYTE bEndMissionType, DWORD dwBattlePassID);
	bool IsLoadedBattlePass() const { return m_bIsLoadedBattlePass; }
	BYTE GetBattlePassId();
private:
	bool m_bIsLoadedBattlePass;
	ListBattlePassMap m_listBattlePass;
#endif

#ifdef __BATTLE_PASS_SYSTEM__
public:
	int 			GetSecondsTillNextWeek();
	int 			GetBattlePassEndTime() { return (m_dwBattlePassEndTime - time(0)); }
	void			ResetBattlePassEndTime() { m_dwBattlePassEndTime = 0; }
	bool			IsBetaMapBoss();
	bool			IsDungeonBoss();
	void			SaveBattlePass();

	void			CreateLoginTime() { m_dwLoginTime = time(0); }
protected:
	DWORD			m_dwBattlePassEndTime;
	DWORD			m_dwLoginTime;
	DWORD			m_iBattlePassLoadFailCounter;
#endif
#ifdef ENABLE_CHAT_COLOR_SYSTEM
public:
	void SetChatColor(BYTE color) { m_points.color = color; }
	BYTE GetChatColor() const;
#endif
#ifdef ENABLE_AUTO_HUNT_SYSTEM
public:
	void SetHasAttacker(bool f) { m_bHasAttacker = f; }
	bool HasAttacker() { return m_bHasAttacker; }
	DWORD GetLastAttacked();
	void SetNormalPlayer(LPCHARACTER player) { m_pkNormalPlayer = player; }
	LPCHARACTER GetNormalPlayer() { return m_pkNormalPlayer; }
protected:
	bool m_bHasAttacker;
	LPCHARACTER m_pkNormalPlayer;
#endif // ENABLE_AUTO_HUNT_SYSTEM
#ifdef ENABLE_MULTI_FARM_BLOCK
public:
	bool GetMultiStatus() { return m_bmultiFarmStatus; }
	void SetMultiStatus(bool bValue) { m_bmultiFarmStatus = bValue; }

	void SetProtectTime(const std::string& flagname, int value);
	int GetProtectTime(const std::string& flagname) const;

protected:
	bool m_bmultiFarmStatus;
	std::map<std::string, int>  m_protection_Time;
#endif
};

ESex GET_SEX(LPCHARACTER ch);
