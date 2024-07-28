#pragma once

#include "../gamelib/RaceData.h"
#include "../gamelib/ActorInstance.h"
#include "../gamelib/GameLibDefines.h"

#ifdef ENABLE_ACCE_SYSTEM
#include "../eterlib/GrpObjectInstance.h"
#endif
#include "AffectFlagContainer.h"

#include <intrin.h>

class CInstanceBase
{
public:
	struct SCreateData
	{
			BYTE	m_bType;
			DWORD	m_dwStateFlags;
			DWORD	m_dwEmpireID;
			DWORD	m_dwGuildID;
			DWORD	m_dwLevel;
			DWORD	m_dwVID;
			DWORD	m_dwRace;
			WORD	m_dwMovSpd;
			WORD	m_dwAtkSpd;
			LONG	m_lPosX;
			LONG	m_lPosY;
			FLOAT	m_fRot;
			DWORD	m_dwMountVnum;
			DWORD	m_dwArmor;
			DWORD	m_dwWeapon;
			DWORD	m_dwHair;
#ifdef ENABLE_ACCE_SYSTEM
			DWORD	m_dwAcce;
			DWORD	m_dwAcceEffect;
#endif // ENABLE_ACCE_SYSTEM
#ifdef ENABLE_AURA_COSTUME_SYSTEM
			DWORD	m_dwAura;
#endif // ENABLE_AURA_COSTUME_SYSTEM
#ifdef ENABLE_QUIVER_SYSTEM
			DWORD	m_dwQuiver;
#endif // ENABLE_QUIVER_SYSTEM
#ifdef ENABLE_MOB_AGGR_LVL_INFO
			DWORD	m_dwAIFlag;
#endif
#ifdef ENABLE_ALIGNMENT_SYSTEM
			int	m_alignment;
#else
			short	m_alignment;
#endif
#ifdef ENABLE_RANK_SYSTEM
			short	m_sRank;
#endif
#ifdef ENABLE_LANDRANK_SYSTEM
			short	m_sLandRank;
#endif
#ifdef ENABLE_REBORN_SYSTEM
			short	m_sReborn;
#endif
#ifdef ENABLE_TEAM_SYSTEM
			short	m_sTeam;
#endif
#ifdef ENABLE_MONIKER_SYSTEM
			std::string m_Moniker;
#endif
#ifdef ENABLE_LOVE_SYSTEM
			std::string m_Love1;
			std::string m_Love2;
#endif // ENABLE_LOVE_SYSTEM
#ifdef ENABLE_WORD_SYSTEM
			std::string m_word;
#endif // ENABLE_WORD_SYSTEM

#ifdef ENABLE_GUILD_LEADER_SYSTEM
			BYTE	m_bMemberType;
#endif
#ifdef ENABLE_ITEM_EVOLUTION_SYSTEM
			WORD	m_wArmorEvolution;
			WORD	m_wWeaponEvolution;
#endif
#ifdef ENABLE_SKILL_COLOR_SYSTEM
			DWORD	m_dwSkillColor[ESkillColorLength::MAX_SKILL_COUNT + MAX_BUFF_COUNT][ESkillColorLength::MAX_EFFECT_COUNT];
#endif
#ifdef ENABLE_SHINING_ITEM_SYSTEM
			DWORD	m_dwShining[6];
#endif
			BYTE	m_byPKMode;
			CAffectFlagContainer	m_kAffectFlags;
			std::string m_stName;
			BYTE	m_bLevel;
			bool	m_isMain;
	};


public:
	typedef DWORD TType;

	enum EDirection
	{
		DIR_NORTH,
		DIR_NORTHEAST,
		DIR_EAST,
		DIR_SOUTHEAST,
		DIR_SOUTH,
		DIR_SOUTHWEST,
		DIR_WEST,
		DIR_NORTHWEST,
		DIR_MAX_NUM,
	};

	enum
	{
		FUNC_WAIT,
		FUNC_MOVE,
		FUNC_ATTACK,
		FUNC_COMBO,
		FUNC_MOB_SKILL,
		FUNC_EMOTION,
		FUNC_SKILL = 0x80,
	};

	enum EMobAIFlags
	{
		AIFLAG_AGGRESSIVE = (1 << 0),
		AIFLAG_NOMOVE = (1 << 1),
		AIFLAG_COWARD = (1 << 2),
		AIFLAG_NOATTACKSHINSU = (1 << 3),
		AIFLAG_NOATTACKJINNO = (1 << 4),
		AIFLAG_NOATTACKCHUNJO = (1 << 5),
		AIFLAG_ATTACKMOB = (1 << 6),
		AIFLAG_BERSERK = (1 << 7),
		AIFLAG_STONESKIN = (1 << 8),
		AIFLAG_GODSPEED = (1 << 9),
		AIFLAG_DEATHBLOW = (1 << 10),
		AIFLAG_REVIVE = (1 << 11),
	};

	enum
	{
		AFFECT_YMIR,
		AFFECT_INVISIBILITY,
		AFFECT_SPAWN,

		AFFECT_POISON,
		AFFECT_SLOW,
		AFFECT_STUN,

		AFFECT_DUNGEON_READY,
		AFFECT_SHOW_ALWAYS,

		AFFECT_MOV_SPEED_POTION,
		AFFECT_ATT_SPEED_POTION,

		AFFECT_FISH_MIND,

		AFFECT_JEONGWI,
		AFFECT_GEOMGYEONG,
		AFFECT_CHEONGEUN,
		AFFECT_GYEONGGONG,
		AFFECT_EUNHYEONG,
		AFFECT_GWIGEOM,
		AFFECT_GONGPO,
		AFFECT_JUMAGAP,
		AFFECT_HOSIN,
		AFFECT_BOHO,
		AFFECT_KWAESOK,
		AFFECT_HEUKSIN,
		AFFECT_MUYEONG,
		AFFECT_REVIVE_INVISIBILITY,
		AFFECT_FIRE,
		AFFECT_GICHEON,
		AFFECT_JEUNGRYEOK,
		AFFECT_DASH,
		AFFECT_PABEOP,
		AFFECT_FALLEN_CHEONGEUN,
		AFFECT_WAR_FLAG1,
		AFFECT_WAR_FLAG2,
		AFFECT_WAR_FLAG3,
		AFFECT_CHINA_FIREWORK,
		AFFECT_PREMIUM_SILVER,
		AFFECT_PREMIUM_GOLD,
		AFFECT_RAMADAN_RING,
#ifdef ENABLE_WOLFMAN_CHARACTER
		AFFECT_BLEEDING,
		AFFECT_RED_POSSESSION,
		AFFECT_BLUE_POSSESSION,
#endif
#ifdef ENABLE_DRAGON_BONE_EFFECT
		AFFECT_DBONE1,
		AFFECT_DBONE2,
#endif
#ifdef ENABLE_POTION_ITEMS
		AFF_18385,
		AFF_18386,
		AFF_18387,
		AFF_18388,
		AFF_18389,
		AFF_18390,
#endif
#ifdef ENABLE_DSS_ACTIVE_EFFECT
		AFFECT_DS,
#endif
#ifdef ENABLE_PB2_NEW_TAG_SYSTEM
		AFFECT_PB2_TAG_VIP,
		AFFECT_PB2_TAG_KING,
		AFFECT_PB2_TAG_QUEEN,
		AFFECT_PB2_TAG_MAFIA,
		AFFECT_PB2_TAG_BARON,
		AFFECT_PB2_TAG_STONE_KING,
		AFFECT_PB2_TAG_BOSS_KING,
		AFFECT_PB2_TAG_BARRIER,
#endif
		AFF_OTOAV,
		AFFECT_NUM = 64,

		AFFECT_HWAYEOM = AFFECT_GEOMGYEONG,
	};

	enum
	{
		NEW_AFFECT_MOV_SPEED = 200,
		NEW_AFFECT_ATT_SPEED,		//201
		NEW_AFFECT_ATT_GRADE,		//202
		NEW_AFFECT_INVISIBILITY,	//203
		NEW_AFFECT_STR,				//204
		NEW_AFFECT_DEX,				//205
		NEW_AFFECT_CON,				//206
		NEW_AFFECT_INT,				//207
		NEW_AFFECT_FISH_MIND_PILL,	//208

		NEW_AFFECT_POISON,			//209
		NEW_AFFECT_STUN,			//210
		NEW_AFFECT_SLOW,			//211
		NEW_AFFECT_DUNGEON_READY,	//212
		NEW_AFFECT_DUNGEON_UNIQUE,	//213

		NEW_AFFECT_REVIVE_INVISIBLE,//214
		NEW_AFFECT_FIRE,			//215
		NEW_AFFECT_CAST_SPEED,		//216
		NEW_AFFECT_HP_RECOVER_CONTINUE,//217
		NEW_AFFECT_SP_RECOVER_CONTINUE,//218

		NEW_AFFECT_MOUNT,			//219

		NEW_AFFECT_WAR_FLAG,		//220

		NEW_AFFECT_BLOCK_CHAT,		//221
		NEW_AFFECT_CHINA_FIREWORK,	//222

		NEW_AFFECT_BOW_DISTANCE,	//223
		NEW_AFFECT_DEF_GRADE,		//224
#ifdef ENABLE_WOLFMAN_CHARACTER
		NEW_AFFECT_BLEEDING,		//225
#endif

		NEW_AFFECT_EXP_BONUS = 500,
		NEW_AFFECT_ITEM_BONUS = 501,
		NEW_AFFECT_SAFEBOX = 502, // PREMIUM_SAFEBOX,
		NEW_AFFECT_AUTOLOOT = 503, // PREMIUM_AUTOLOOT,
		NEW_AFFECT_FISH_MIND = 504, // PREMIUM_FISH_MIND,
		NEW_AFFECT_GOLD_BONUS = 506,

		NEW_AFFECT_MALL = 510,
		NEW_AFFECT_NO_DEATH_PENALTY = 511,
		NEW_AFFECT_SKILL_BOOK_BONUS = 512,
		NEW_AFFECT_SKILL_BOOK_NO_DELAY = 513,

		NEW_AFFECT_HAIR = 514,
		NEW_AFFECT_COLLECT = 515,
		NEW_AFFECT_EXP_BONUS_EURO_FREE = 516,
		NEW_AFFECT_EXP_BONUS_EURO_FREE_UNDER_15 = 517,
		NEW_AFFECT_UNIQUE_ABILITY = 518,

		NEW_AFFECT_BLEND = 531,
		NEW_AFFECT_HORSE_NAME = 532,
		NEW_AFFECT_MOUNT_BONUS = 533,

		NEW_AFFECT_AUTO_HP_RECOVERY = 534,
		NEW_AFFECT_AUTO_SP_RECOVERY = 535,

		NEW_AFFECT_DRAGON_SOUL_QUALIFIED = 540,
		NEW_AFFECT_DRAGON_SOUL_DECK1 = 541,
		NEW_AFFECT_DRAGON_SOUL_DECK2 = 542,
#ifdef ENABLE_DS_SET
		NEW_AFFECT_DS_SET = 543,
#endif

#ifdef ENABLE_GROWTH_PET_SYSTEM
		AFFECT_NEW_PET = 559,
#endif

#ifdef ENABLE_ICECREAM_ITEM
		AFFECT_ICECREAM_ABILITY = 560,
#endif

#ifdef ENABLE_GROWTH_MOUNT_SYSTEM
		AFFECT_NEW_MOUNT = 561,
#endif
		NEW_AFFECT_RAMADAN_ABILITY = 600,
		NEW_AFFECT_RAMADAN_RING,

		NEW_AFFECT_NOG_POCKET_ABILITY,
#ifdef ENABLE_POTION_AS_REWORK
		RED_BLEND_POTION,
		ORANGE_BLEND_POTION,
		YELLOW_BLEND_POTION,
		GREEN_BLEND_POTION,
		BLUE_BLEND_POTION,
		WHITE_BLEND_POTION,
		JADE_BLEND_POTION,
		DARK_BLEND_POTION,
		SAFE_BLEND_POTION,
		DEVIL_BLEND_POTION,
		HEAL_BLEND_POTION,
		NORMAL_HIT_BLEND_POTION,
		UNDEAD_BLEND_POTION,
		DEVIL_ATT_BLEND_POTION,
		ANIMAL_BLEND_POTION,
		MILGYO_BLEND_POTION,
#endif
#ifdef ENABLE_DRAGON_BONE_EFFECT
		AFFECT_DBONE_1,
		AFFECT_DBONE_2,
#endif
#ifdef ENABLE_ALIGNMENT_SYSTEM
		AFFECT_ALIGN_BONUS_TYPE_1,// 621
		AFFECT_ALIGN_BONUS_TYPE_2,// 622
		AFFECT_ALIGN_BONUS_TYPE_3,// 623
		AFFECT_ALIGN_BONUS_TYPE_4,// 624
		AFFECT_ALIGN_BONUS_TYPE_5,// 625
		AFFECT_ALIGN_BONUS_TYPE_6,// 626
		AFFECT_ALIGN_BONUS_TYPE_7,// 627
		AFFECT_ALIGN_BONUS_TYPE_8,// 628
#endif
#ifdef ENABLE_POTION_ITEMS
		AFFECT_18385,
		AFFECT_18386,
		AFFECT_18387,
		AFFECT_18388,
		AFFECT_18389,
		AFFECT_18390,
#endif
#ifdef ENABLE_RANK_SYSTEM
		AFFECT_RANK_BONUS_TYPE_1,//636+2
		AFFECT_RANK_BONUS_TYPE_2,//637+2
		AFFECT_RANK_BONUS_TYPE_3,//638+2
		AFFECT_RANK_BONUS_TYPE_4,//639+2
		AFFECT_RANK_BONUS_TYPE_5,//640+2
		AFFECT_RANK_BONUS_TYPE_6,//641+2
		AFFECT_RANK_BONUS_TYPE_7,//642+2
		AFFECT_RANK_BONUS_TYPE_8,//643+2
		AFFECT_RANK_BONUS_TYPE_9,//644+2
		AFFECT_RANK_BONUS_TYPE_10,//645+2
		AFFECT_RANK_BONUS_TYPE_11,//646+2
		AFFECT_RANK_BONUS_TYPE_12,//647+2
		AFFECT_RANK_BONUS_TYPE_13,//648+2
		AFFECT_RANK_BONUS_TYPE_14,//649+2
		AFFECT_RANK_BONUS_TYPE_15,//650+2
		AFFECT_RANK_BONUS_TYPE_16,//651+2
		AFFECT_RANK_BONUS_TYPE_17,//652+2
#endif
#ifdef ENABLE_LANDRANK_SYSTEM
		AFFECT_LANDRANK_BONUS_TYPE_1,//653+2
		AFFECT_LANDRANK_BONUS_TYPE_2,//654+2
		AFFECT_LANDRANK_BONUS_TYPE_3,//655+2
		AFFECT_LANDRANK_BONUS_TYPE_4,//656+2
		AFFECT_LANDRANK_BONUS_TYPE_5,//657+2
		AFFECT_LANDRANK_BONUS_TYPE_6,//658+2
		AFFECT_LANDRANK_BONUS_TYPE_7,//659+2
		AFFECT_LANDRANK_BONUS_TYPE_8,//660+2
		AFFECT_LANDRANK_BONUS_TYPE_9,//661+2
		AFFECT_LANDRANK_BONUS_TYPE_10,//662+2
		AFFECT_LANDRANK_BONUS_TYPE_11,//663+2
		AFFECT_LANDRANK_BONUS_TYPE_12,//664+2
		AFFECT_LANDRANK_BONUS_TYPE_13,//665+2
		AFFECT_LANDRANK_BONUS_TYPE_14,//666+2
		AFFECT_LANDRANK_BONUS_TYPE_15,//667+2
		AFFECT_LANDRANK_BONUS_TYPE_16,//668+2
		AFFECT_LANDRANK_BONUS_TYPE_17,//669+2
		AFFECT_LANDRANK_BONUS_TYPE_18,//670+2
		AFFECT_LANDRANK_BONUS_TYPE_19,//671+2
		AFFECT_LANDRANK_BONUS_TYPE_20,//672+2
#endif
#ifdef ENABLE_REBORN_SYSTEM
		AFFECT_REBORN_BONUS_TYPE_1,//673+2
		AFFECT_REBORN_BONUS_TYPE_2,//674+2
		AFFECT_REBORN_BONUS_TYPE_3,//675+2
		AFFECT_REBORN_BONUS_TYPE_4,//676+2
		AFFECT_REBORN_BONUS_TYPE_5,//677+2
		AFFECT_REBORN_BONUS_TYPE_6,//678+2
		AFFECT_REBORN_BONUS_TYPE_7,//679+2
		AFFECT_REBORN_BONUS_TYPE_8,//680+2
		AFFECT_REBORN_BONUS_TYPE_9,//681+2
		AFFECT_REBORN_BONUS_TYPE_10,//682+2
#endif
#ifdef ENABLE_SAGE_SKILL
		AFFECT_SKILL_BOOK_BONUS2,
#endif
#ifdef ENABLE_BIOLOG_SYSTEM
		AFFECT_BIO_ITEM_DURATION,//725
		AFFECT_BIO_PACKET_DURATION,//726
		AFFECT_BIO_BONUS1,//727
		AFFECT_BIO_BONUS2,//728
		AFFECT_BIO_BONUS3,//729
		AFFECT_BIO_BONUS4,//730
		AFFECT_BIO_BONUS5,//731
		AFFECT_BIO_BONUS6,//731
		AFFECT_BIO_BONUS7,//731
		AFFECT_BIO_BONUS8,//731
#endif
#ifdef ENABLE_FISHBONUS_REWORK
		AFFECT_FISH_EXP_BONUS,
		AFFECT_FISH_CAST_SPEED,
		AFFECT_FISH_MAX_HP,
		AFFECT_FISH_MAX_SP,
		AFFECT_FISH_ATTBONUS_UNDEAD,
		AFFECT_FISH_ATTBONUS_ANIMAL,
		AFFECT_FISH_ATTBONUS_MILGYO,
		AFFECT_FISH_ATTBONUS_ORC,
		AFFECT_FISH_ATTBONUS_DEVIL,
		AFFECT_FISH_ATTBONUS_INSECT,
		AFFECT_FISH_ATTBONUS_BOSS,
		AFFECT_FISH_ATTBONUS_MONSTER,
		AFFECT_FISH_ATTBONUS_HUMAN,
		AFFECT_FISH_ATTBONUS_STONE,
		AFFECT_FISH_NORMAL_HIT_BONUS,
		AFFECT_FISH_APPLY_STR,
		AFFECT_FISH_APPLY_DEX,
		AFFECT_FISH_APPLY_CON,
		AFFECT_FISH_APPLY_INT,
		AFFECT_FISH_APPLY_CAST_SPEED,
		AFFECT_FISH_APPLY_ATT_GRADE,
		AFFECT_FISH_APPLY_DEF_GRADE,
#endif
#ifdef ENABLE_GROWTH_PET_SYSTEM
		AFFECT_NEW_PET_DURATION,
#endif

		AFFECT_GM_BONUS_1,
		AFFECT_GM_BONUS_2,
		AFFECT_GM_BONUS_3,
		AFFECT_GM_BONUS_4,
		AFFECT_GM_BONUS_5,
		AFFECT_GM_BONUS_6,
		AFFECT_GM_BONUS_7,
		AFFECT_GM_BONUS_8,
		AFFECT_GM_BONUS_9,
		AFFECT_GM_BONUS_10,

#ifdef ENABLE_EXPERT_SKILL
		AFFECT_SKILL_BOOK_BONUS_EXPERT,
#endif

		AFFECT_RAMADAN_NEW1,
		AFFECT_RAMADAN_NEW2,
		AFFECT_RAMADAN_NEW3,
		AFFECT_RAMADAN_NEW4,
		AFFECT_RAMADAN_NEW5,
		AFFECT_RAMADAN_NEW6,
		AFFECT_RAMADAN_NEW7,
		AFFECT_RAMADAN_NEW8,

		AFFECT_SEKILL_SET_BONUS_1,
		AFFECT_SEKILL_SET_BONUS_2,
		AFFECT_SEKILL_SET_BONUS_3,
		AFFECT_SEKILL_SET_BONUS_4,
		AFFECT_SEKILL_SET_BONUS_5,
		AFFECT_SEKILL_SET_BONUS_6,

		AFFECT_ADAMSINIZ,
		AFFECT_KINDER,
		AFFECT_CIKOLATA,
		AFFECT_GUL,
		AFFECT_LOLIPOP,



#ifdef ENABLE_EXPERT_SKILL
		AFFECT_SKILL_BOOK_BONUS_DESTANSI,
		AFFECT_SKILL_BOOK_BONUS_TANRISAL,
#endif

		AFFECT_LEVEL_SET_BONUS_1,
		AFFECT_LEVEL_SET_BONUS_2,
		AFFECT_LEVEL_SET_BONUS_3,
		AFFECT_LEVEL_SET_BONUS_4,
		AFFECT_LEVEL_SET_BONUS_5,

		AFFECT_ITEM_SET_BONUS_1,
		AFFECT_ITEM_SET_BONUS_2,
		AFFECT_ITEM_SET_BONUS_3,
		AFFECT_ITEM_SET_BONUS_4,
		AFFECT_ITEM_SET_BONUS_5,

#ifdef ENABLE_MONIKER_SYSTEM
		AFFECT_MONIKER_BONUS_TYPE_1,
		AFFECT_MONIKER_BONUS_TYPE_2,
		AFFECT_MONIKER_BONUS_TYPE_3,
		AFFECT_MONIKER_BONUS_TYPE_4,
		AFFECT_MONIKER_BONUS_TYPE_5,
#endif

#ifdef ENABLE_TEAM_SYSTEM
		AFFECT_TEAM_BONUS_TYPE_1,
		AFFECT_TEAM_BONUS_TYPE_2,
		AFFECT_TEAM_BONUS_TYPE_3,
		AFFECT_TEAM_BONUS_TYPE_4,
		AFFECT_TEAM_BONUS_TYPE_5,
#endif

		AFFECT_NEW_SEBO_1,
		AFFECT_NEW_SEBO_2,

		AFFECT_NEW_EXP1,
		AFFECT_NEW_EXP2,
		AFFECT_NEW_EXP3,
		AFFECT_NEW_EXP4,
		AFFECT_NEW_EXP5,
		AFFECT_NEW_EXP6,

#ifdef ENABLE_BATTLE_PASS_SYSTEM
		AFFECT_BATTLE_PASS,
		AFFECT_BATTLE_PASS_PREMIUM,
#endif
#ifdef ENABLE_BATTLE_PASS_EX
		AFFECT_BATTLE_PASS_METIN,
		AFFECT_BATTLE_PASS_BOSS,
		AFFECT_BATTLE_PASS_MONSTER,
		AFFECT_BATTLE_PASS_BERSERKER,
		AFFECT_BATTLE_PASS_GUCLENDIRME,
		AFFECT_BATTLE_PASS_ORTALAMA,
#endif // ENABLE_BATTLE_PASS_EX

		AFFECT_OTOMATIKAV,

#ifdef ENABLE_PB2_PREMIUM_SYSTEM
		AFFECT_PB2_SKILL_COLOR,
		AFFECT_PB2_GLOBAL_CHAT,
#endif // ENABLE_PB2_PREMIUM_SYSTEM

#ifdef ENABLE_PB2_NEW_TAG_SYSTEM
		NEW_AFFECT_PB2_TAG_VIP,
		NEW_AFFECT_PB2_TAG_KING,
		NEW_AFFECT_PB2_TAG_QUEEN,
		NEW_AFFECT_PB2_TAG_MAFIA,
		NEW_AFFECT_PB2_TAG_BARON,
		NEW_AFFECT_PB2_TAG_STONE_KING,
		NEW_AFFECT_PB2_TAG_BOSS_KING,
		NEW_AFFECT_PB2_TAG_BARRIER,
#endif // ENABLE_PB2_NEW_TAG_SYSTEM

		AFFECT_BATTLE_PASS_BERSERKER_2,
		AFFECT_YILBASI,
		AFFECT_PB2_PET_NAME,
		AFFECT_PB2_MOUNT_NAME,
		AFFECT_PB2_CHAR_NAME,
		AFFECT_LOVE,
		AFFECT_WORD,
		AFFECT_BASLANGIC_ESYA,

		AFFECT_OTOAV_EFFECT,
		
#ifdef ENABLE_GROWTH_MOUNT_SYSTEM
		AFFECT_NEW_MOUNT_DURATION,
#endif
		AFFECT_KANAL_FIX,

#ifdef ENABLE_MULTIFARM_BLOCK
		NEW_AFFECT_MULTI_FARM,
#endif

		NEW_AFFECT_QUEST_START_IDX = 1000,
	};

	enum
	{
		STONE_SMOKE1 = 0,	// 99%
		STONE_SMOKE2 = 1,	// 85%
		STONE_SMOKE3 = 2,	// 80%
		STONE_SMOKE4 = 3,	// 60%
		STONE_SMOKE5 = 4,	// 45%
		STONE_SMOKE6 = 5,	// 40%
		STONE_SMOKE7 = 6,	// 20%
		STONE_SMOKE8 = 7,	// 10%
		STONE_SMOKE_NUM = 4,
	};

	enum
	{
		WEAPON_DUALHAND,
		WEAPON_ONEHAND,
		WEAPON_TWOHAND,
		WEAPON_NUM,
	};

	enum
	{
		EMPIRE_NONE,
		EMPIRE_A,
		EMPIRE_B,
		EMPIRE_C,
		EMPIRE_NUM,
	};

	enum
	{
		NAMECOLOR_MOB,
		NAMECOLOR_NPC,
		NAMECOLOR_PC,
		NAMECOLOR_PC_END = NAMECOLOR_PC + EMPIRE_NUM,
		NAMECOLOR_NORMAL_MOB,
		NAMECOLOR_NORMAL_NPC,
		NAMECOLOR_NORMAL_PC,
		NAMECOLOR_NORMAL_PC_END = NAMECOLOR_NORMAL_PC + EMPIRE_NUM,
		NAMECOLOR_EMPIRE_MOB,
		NAMECOLOR_EMPIRE_NPC,
		NAMECOLOR_EMPIRE_PC,
		NAMECOLOR_EMPIRE_PC_END = NAMECOLOR_EMPIRE_PC + EMPIRE_NUM,
		NAMECOLOR_FUNC,
		NAMECOLOR_PK,
		NAMECOLOR_PVP,
		NAMECOLOR_PARTY,
		NAMECOLOR_WARP,
		NAMECOLOR_WAYPOINT,
#ifdef ENABLE_METINSTONE_ON_MINIMAP
		NAMECOLOR_STONE,
#endif
#ifdef ENABLE_BOSS_ON_MINIMAP
		NAMECOLOR_BOSS,
#endif
		NAMECOLOR_EXTRA = NAMECOLOR_FUNC + 10,
		NAMECOLOR_NUM = NAMECOLOR_EXTRA + 10,
	};

	enum
	{
		ALIGNMENT_TYPE_WHITE,
		ALIGNMENT_TYPE_NORMAL,
		ALIGNMENT_TYPE_DARK,
	};

	enum
	{
		EMOTICON_EXCLAMATION = 1,
		EMOTICON_FISH = 11,
		EMOTICON_NUM = 128,

#ifdef ENABLE_LANDRANK_SYSTEM
		TITLE_NONE_LANDRANK = 0, // -
		TITLE_NUM_LANDRANK_BASE = 81, // Giresun
		TITLE_NUM_LANDRANK_L = 101, // GiresunL
		TITLE_NUM_LANDRANK = 201, // GiresunLU
#endif

#ifdef ENABLE_REBORN_SYSTEM
		TITLE_NONE_REBORN = 0,
#endif

#ifdef ENABLE_ALIGNMENT_SYSTEM
		TITLE_NUM = 18+5+1+12,
		TITLE_NONE = 13+5+1+12,
#else
		TITLE_NUM = 9,
		TITLE_NONE = 4,
#endif
	};

	enum
	{
		EFFECT_REFINED_NONE,

		EFFECT_SWORD_REFINED7,
		EFFECT_SWORD_REFINED8,
		EFFECT_SWORD_REFINED9,

		EFFECT_BOW_REFINED7,
		EFFECT_BOW_REFINED8,
		EFFECT_BOW_REFINED9,

		EFFECT_FANBELL_REFINED7,
		EFFECT_FANBELL_REFINED8,
		EFFECT_FANBELL_REFINED9,

		EFFECT_SMALLSWORD_REFINED7,
		EFFECT_SMALLSWORD_REFINED8,
		EFFECT_SMALLSWORD_REFINED9,

		EFFECT_SMALLSWORD_REFINED7_LEFT,
		EFFECT_SMALLSWORD_REFINED8_LEFT,
		EFFECT_SMALLSWORD_REFINED9_LEFT,

		EFFECT_BODYARMOR_REFINED7,
		EFFECT_BODYARMOR_REFINED8,
		EFFECT_BODYARMOR_REFINED9,

		EFFECT_BODYARMOR_SPECIAL,
		EFFECT_BODYARMOR_SPECIAL2,

#ifdef ENABLE_WOLFMAN_CHARACTER
		EFFECT_CLAW_REFINED7,
		EFFECT_CLAW_REFINED8,
		EFFECT_CLAW_REFINED9,

		EFFECT_CLAW_REFINED7_LEFT,
		EFFECT_CLAW_REFINED8_LEFT,
		EFFECT_CLAW_REFINED9_LEFT,
#endif

#ifdef ENABLE_LVL115_ARMOR_EFFECT
		EFFECT_BODYARMOR_SPECIAL3,	// 5-1
#endif
#ifdef ENABLE_ACCE_SYSTEM
		EFFECT_ACCE,
#endif
#ifdef ENABLE_ITEM_EVOLUTION_SYSTEM
		EFFECT_SWORD_RARITY_FIRE,
		EFFECT_SWORD_RARITY_ICE,
		EFFECT_SWORD_RARITY_EARTH,
		EFFECT_SWORD_RARITY_WIND,
		EFFECT_SWORD_RARITY_DARK,
		EFFECT_SWORD_RARITY_ELEC,

		EFFECT_BOW_RARITY_FIRE,
		EFFECT_BOW_RARITY_ICE,
		EFFECT_BOW_RARITY_EARTH,
		EFFECT_BOW_RARITY_WIND,
		EFFECT_BOW_RARITY_DARK,
		EFFECT_BOW_RARITY_ELEC,

		EFFECT_FANBELL_RARITY_FIRE,
		EFFECT_FANBELL_RARITY_ICE,
		EFFECT_FANBELL_RARITY_EARTH,
		EFFECT_FANBELL_RARITY_WIND,
		EFFECT_FANBELL_RARITY_DARK,
		EFFECT_FANBELL_RARITY_ELEC,

		EFFECT_SMALLSWORD_RARITY_FIRE,
		EFFECT_SMALLSWORD_RARITY_ICE,
		EFFECT_SMALLSWORD_RARITY_EARTH,
		EFFECT_SMALLSWORD_RARITY_WIND,
		EFFECT_SMALLSWORD_RARITY_DARK,
		EFFECT_SMALLSWORD_RARITY_ELEC,

		EFFECT_SMALLSWORD_RARITY_LEFT_FIRE,
		EFFECT_SMALLSWORD_RARITY_LEFT_ICE,
		EFFECT_SMALLSWORD_RARITY_LEFT_EARTH,
		EFFECT_SMALLSWORD_RARITY_LEFT_WIND,
		EFFECT_SMALLSWORD_RARITY_LEFT_DARK,
		EFFECT_SMALLSWORD_RARITY_LEFT_ELEC,

		EFFECT_CLAW_RARITY_FIRE,
		EFFECT_CLAW_RARITY_ICE,
		EFFECT_CLAW_RARITY_EARTH,
		EFFECT_CLAW_RARITY_WIND,
		EFFECT_CLAW_RARITY_DARK,
		EFFECT_CLAW_RARITY_ELEC,

		EFFECT_CLAW_RARITY_LEFT_FIRE,
		EFFECT_CLAW_RARITY_LEFT_ICE,
		EFFECT_CLAW_RARITY_LEFT_EARTH,
		EFFECT_CLAW_RARITY_LEFT_WIND,
		EFFECT_CLAW_RARITY_LEFT_DARK,
		EFFECT_CLAW_RARITY_LEFT_ELEC,
#endif
#ifdef ENABLE_ARMOR_EVOLUTION_SYSTEM
		EFFECT_ARMOR_RARITY_FIRE,
		EFFECT_ARMOR_RARITY_ICE,
		EFFECT_ARMOR_RARITY_EARTH,
		EFFECT_ARMOR_RARITY_WIND,
		EFFECT_ARMOR_RARITY_DARK,
		EFFECT_ARMOR_RARITY_ELEC,
#endif
#ifdef ENABLE_ITEM_EVOLUTION_SYSTEM
		EFFECT_SWORD_RARITY_ALL,
		EFFECT_BOW_RARITY_ALL,
		EFFECT_FAN_RARITY_ALL,
		EFFECT_OTHER_RARITY_ALL,
		EFFECT_OTHER_RARITY_ALL_LEFT,
		EFFECT_ARMOR_RARITY_ALL,
#endif
		EFFECT_REFINED_NUM,
	};

	enum DamageFlag
	{
		DAMAGE_NORMAL = (1 << 0),
		DAMAGE_POISON = (1 << 1),
		DAMAGE_DODGE = (1 << 2),
		DAMAGE_BLOCK = (1 << 3),
		DAMAGE_PENETRATE = (1 << 4),
		DAMAGE_CRITICAL = (1 << 5),
#ifdef ENABLE_DAMAGE_PROCESS_SYSTEM
		DAMAGE_BLEEDING = (1 << 6),
		DAMAGE_FIRE = (1 << 7),
#endif
	};
#ifdef ENABLE_SHINING_ITEM_SYSTEM
	enum EFFECT_SHINING_WEAPON
	{
		EFFECT_SHINING_WEAPON_SWORD,
		EFFECT_SHINING_WEAPON_SWORD_END = EFFECT_SHINING_WEAPON_SWORD + 49, //20x effect
		EFFECT_SHINING_WEAPON_BOW,
		EFFECT_SHINING_WEAPON_BOW_END = EFFECT_SHINING_WEAPON_BOW + 49, //20x effect
		EFFECT_SHINING_WEAPON_FAN,
		EFFECT_SHINING_WEAPON_FAN_END = EFFECT_SHINING_WEAPON_FAN + 49, //20x effect
		EFFECT_SHINING_WEAPON_DAGGER_R,
		EFFECT_SHINING_WEAPON_DAGGER_R_END = EFFECT_SHINING_WEAPON_DAGGER_R + 49, //20x effect
		EFFECT_SHINING_WEAPON_DAGGER_L,
		EFFECT_SHINING_WEAPON_DAGGER_L_END = EFFECT_SHINING_WEAPON_DAGGER_L + 49, //20x effect
#ifdef ENABLE_WOLFMAN_CHARACTER
		EFFECT_SHINING_WEAPON_CLAW_R,
		EFFECT_SHINING_WEAPON_CLAW_R_END = EFFECT_SHINING_WEAPON_CLAW_R + 49, //20x effect
		EFFECT_SHINING_WEAPON_CLAW_L,
		EFFECT_SHINING_WEAPON_CLAW_L_END = EFFECT_SHINING_WEAPON_CLAW_L + 49, //20x effect
#endif
		EFFECT_SHINING_WEAPON_NUM,
	};
#endif
	enum
	{
		EFFECT_DUST,
		EFFECT_STUN,
		EFFECT_HIT,
		EFFECT_FLAME_ATTACK,
		EFFECT_FLAME_HIT,
		EFFECT_FLAME_ATTACH,
		EFFECT_ELECTRIC_ATTACK,
		EFFECT_ELECTRIC_HIT,
		EFFECT_ELECTRIC_ATTACH,
		EFFECT_SPAWN_APPEAR,
		EFFECT_SPAWN_DISAPPEAR,
		EFFECT_LEVELUP,
		EFFECT_SKILLUP,
		EFFECT_HPUP_RED,
		EFFECT_SPUP_BLUE,
		EFFECT_SPEEDUP_GREEN,
		EFFECT_DXUP_PURPLE,
		EFFECT_CRITICAL,
		EFFECT_PENETRATE,
		EFFECT_BLOCK,
		EFFECT_DODGE,
		EFFECT_FIRECRACKER,
		EFFECT_SPIN_TOP,
		EFFECT_WEAPON,
		EFFECT_WEAPON_END = EFFECT_WEAPON + WEAPON_NUM,
		EFFECT_AFFECT,
		EFFECT_AFFECT_GYEONGGONG = EFFECT_AFFECT + AFFECT_GYEONGGONG,
		EFFECT_AFFECT_KWAESOK = EFFECT_AFFECT + AFFECT_KWAESOK,
		EFFECT_AFFECT_END = EFFECT_AFFECT + AFFECT_NUM,
		EFFECT_EMOTICON,
		EFFECT_EMOTICON_END = EFFECT_EMOTICON + EMOTICON_NUM,
		EFFECT_SELECT,
		EFFECT_TARGET,
		EFFECT_EMPIRE,
		EFFECT_EMPIRE_END = EFFECT_EMPIRE + EMPIRE_NUM,
		EFFECT_HORSE_DUST,
		EFFECT_REFINED,
		EFFECT_REFINED_END = EFFECT_REFINED + EFFECT_REFINED_NUM,
		EFFECT_DAMAGE_TARGET,
		EFFECT_DAMAGE_NOT_TARGET,
		EFFECT_DAMAGE_SELFDAMAGE,
		EFFECT_DAMAGE_SELFDAMAGE2,
		EFFECT_DAMAGE_POISON,
		EFFECT_DAMAGE_MISS,
		EFFECT_DAMAGE_TARGETMISS,
		EFFECT_DAMAGE_CRITICAL,
		EFFECT_SUCCESS,
		EFFECT_FAIL,
		EFFECT_FR_SUCCESS,
		EFFECT_PERCENT_DAMAGE1,
		EFFECT_PERCENT_DAMAGE2,
		EFFECT_PERCENT_DAMAGE3,
		EFFECT_AUTO_HPUP,
		EFFECT_AUTO_SPUP,
		EFFECT_RAMADAN_RING_EQUIP,
		EFFECT_HALLOWEEN_CANDY_EQUIP,
		EFFECT_HAPPINESS_RING_EQUIP,
		EFFECT_LOVE_PENDANT_EQUIP,
#ifdef ENABLE_ACCE_SYSTEM
		EFFECT_ACCE_SUCCEDED,
		EFFECT_ACCE_EQUIP,
#endif
#ifdef ENABLE_PVP_EFFECT
		EFFECT_PVP_WIN,
		EFFECT_PVP_OPEN1,
		EFFECT_PVP_OPEN2,
		EFFECT_PVP_BEGIN1,
		EFFECT_PVP_BEGIN2,
#endif
#ifdef ENABLE_SMITH_EFFECT
		EFFECT_FR_FAIL,
#endif
#ifdef ENABLE_DAMAGE_PROCESS_SYSTEM
		EFFECT_DAMAGE_BLEEDING,
		EFFECT_DAMAGE_FIRE,
#endif
#ifdef ENABLE_AGGR_MONSTER_EFFECT
		EFFECT_AGGREGATE_MONSTER,
#endif
#ifdef ENABLE_SHINING_ITEM_SYSTEM
		EFFECT_SHINING_WEAPON,
		EFFECT_SHINING_WEAPON_END = EFFECT_SHINING_WEAPON + EFFECT_SHINING_WEAPON_NUM,
		EFFECT_SHINING_ARMOR,
		EFFECT_SHINING_ARMOR_END = EFFECT_SHINING_ARMOR + 50,//armor num = 20
		EFFECT_SHINING_SPECIAL,
		EFFECT_SHINING_SPECIAL_END = EFFECT_SHINING_SPECIAL + 5,// special num = 1
		EFFECT_SHINING_SPECIAL2,
		EFFECT_SHINING_SPECIAL2_END = EFFECT_SHINING_SPECIAL2 + 5,// special num = 1
		EFFECT_SHINING_SPECIAL3,
		EFFECT_SHINING_SPECIAL3_END = EFFECT_SHINING_SPECIAL3 + 10,// special num = 6
		EFFECT_SHINING_WING,
		EFFECT_SHINING_WING_END = EFFECT_SHINING_WING + 50,// wing num = 30
#endif
		EFFECT_TEMP,
		EFFECT_NUM,
	};

	enum
	{
		DUEL_NONE,
		DUEL_CANNOTATTACK,
		DUEL_START,
	};

public:
	static void DestroySystem();
	static void CreateSystem(UINT uCapacity);
#ifdef ENABLE_SKILL_COLOR_SYSTEM
	static bool RegisterEffect(UINT eEftType, const char* c_szEftAttachBone, const char* c_szEftName, bool isCache, const char* name = NULL);
#else
	static bool RegisterEffect(UINT eEftType, const char* c_szEftAttachBone, const char* c_szEftName, bool isCache);
#endif
	static void RegisterTitleName(int iIndex, const char* c_szTitleName);
	static bool RegisterNameColor(UINT uIndex, UINT r, UINT g, UINT b);
	static bool RegisterTitleColor(UINT uIndex, UINT r, UINT g, UINT b);

	static void SetDustGap(float fDustGap);
	static void SetHorseDustGap(float fDustGap);

	static void SetEmpireNameMode(bool isEnable);
	static const D3DXCOLOR& GetIndexedNameColor(UINT eNameColor);
public:
	void SetMainInstance();

	void OnSelected();
	void OnUnselected();
	void OnTargeted();
	void OnUntargeted();

protected:
	bool __IsExistMainInstance();
	bool __IsMainInstance();
	bool __MainCanSeeHiddenThing();
	float __GetBowRange();

protected:
#ifdef ENABLE_GRAPHIC_OPTIMIZATION
	DWORD	__AttachEffect(UINT eEftType, bool ignoreFrustum = false, float fScale = 1.0f);
#else
	DWORD	__AttachEffect(UINT eEftType, float fScale = 1.0f);
#endif
	void		__DetachEffect(DWORD dwEID);

public:
#ifdef ENABLE_GRAPHIC_OPTIMIZATION
	void CreateSpecialEffect(DWORD iEffectIndex, bool ignoreFrustum = false, float fScale = 1.0f);
	void AttachSpecialEffect(DWORD effect, bool ignoreFrustum = false, float fScale = 1.0f);
#else
	void CreateSpecialEffect(DWORD iEffectIndex, float fScale = 1.0f);
	void AttachSpecialEffect(DWORD effect, float fScale = 1.0f);
#endif

protected:
	static std::string ms_astAffectEffectAttachBone[EFFECT_NUM];
	static DWORD ms_adwCRCAffectEffect[EFFECT_NUM];
	static float ms_fDustGap;
	static float ms_fHorseDustGap;

public:
	CInstanceBase();
	virtual ~CInstanceBase();

	bool LessRenderOrder(CInstanceBase* pkInst);

	void MountHorse(UINT eRace);
	void DismountHorse();

	void SCRIPT_SetAffect(UINT eAffect, bool isVisible);

	float CalculateDistanceSq3d(const TPixelPosition& c_rkPPosDst);

	// Instance Data
	bool IsFlyTargetObject();
	void ClearFlyTargetInstance();
	void SetFlyTargetInstance(CInstanceBase& rkInstDst);
	void AddFlyTargetInstance(CInstanceBase& rkInstDst);
	void AddFlyTargetPosition(const TPixelPosition& c_rkPPosDst);

	float GetFlyTargetDistance();

	void SetAlpha(float fAlpha);

	void DeleteBlendOut();

	void					AttachTextTail();
	void					DetachTextTail();
	void					UpdateTextTailLevel(DWORD level);

	void					RefreshTextTail();
	void					RefreshTextTailTitle();

	bool					Create(const SCreateData& c_rkCreateData);

	bool					CreateDeviceObjects();
	void					DestroyDeviceObjects();

	void					Destroy();

	void					Update();
	bool					UpdateDeleting();

	void					Transform();
	void					Deform();
	void					Render();
	void					RenderTrace();
	void					RenderToShadowMap();
	void					RenderCollision();
	void					RegisterBoundingSphere();

	// Temporary
	void					GetBoundBox(D3DXVECTOR3* vtMin, D3DXVECTOR3* vtMax);

	void					SetNameString(const char* c_szName, int len);
#ifdef ENABLE_CHARACTER_WINDOW_RENEWAL
	bool					SetRace(DWORD dwRaceIndex, bool isIntro=false);
#else
	bool					SetRace(DWORD dwRaceIndex);
#endif
	void					SetVirtualID(DWORD wVirtualNumber);
	void					SetVirtualNumber(DWORD dwVirtualNumber);
	void					SetInstanceType(int iInstanceType);
#ifdef ENABLE_ALIGNMENT_SYSTEM
	void					SetAlignment(int sAlignment);
#else
	void					SetAlignment(short sAlignment);
#endif
	void					SetLevelText(int mLevel);
	void					SetPKMode(BYTE byPKMode);
	void					SetKiller(bool bFlag);
	void					SetPartyMemberFlag(bool bFlag);
	void					SetStateFlags(DWORD dwStateFlags);
#ifdef ENABLE_ARMOR_EVOLUTION_SYSTEM
	void					SetArmorEvolution(DWORD dwEvolution);
#endif
#ifdef ENABLE_ARMOR_EVOLUTION_SYSTEM
	void					SetArmor(DWORD dwArmor, DWORD dwArmorEvolution);
#else
	void					SetArmor(DWORD dwArmor);
#endif
	void					SetShape(DWORD eShape, float fSpecular = 0.0f);
	void					SetHair(DWORD eHair);
#ifdef ENABLE_ACCE_SYSTEM
	DWORD					GetAcceSpecular() const { return m_dwAcceSpecular; };
	bool					SetAcce(DWORD dwAcce, DWORD dwAcceSpecular);
	void					ChangeAcce(DWORD dwAcce, DWORD dwAcceSpecular);
#endif
#ifdef ENABLE_AURA_COSTUME_SYSTEM
	bool					SetAura(DWORD eAura);
	void					ChangeAura(DWORD eAura);
	void					ClearAuraEffect();
#endif
#ifdef ENABLE_SHINING_ITEM_SYSTEM
	void					SetShining(BYTE num, DWORD dwShining);
#endif
#ifdef ENABLE_ITEM_EVOLUTION_SYSTEM
	void					SetWeaponEvolution(DWORD dwEvolution);
	BYTE					GetEvolutionType(int evolution);
	void					__GetEvolutionEffect(BYTE bWeapon, DWORD dwEvolution, BYTE refine);
#endif
#ifdef ENABLE_ITEM_EVOLUTION_SYSTEM
	bool					SetWeapon(DWORD eWeapon, DWORD dwWeaponEvolution = 0);
#else
	bool					SetWeapon(DWORD eWeapon);
#endif
#ifdef ENABLE_QUIVER_SYSTEM
	bool					SetArrow(DWORD eArrow);
#endif
#ifdef ENABLE_ARMOR_EVOLUTION_SYSTEM
	bool					ChangeArmor(DWORD dwArmor, DWORD dwArmorEvolution = 0);
#else
	bool					ChangeArmor(DWORD dwArmor);
#endif
#ifdef ENABLE_ITEM_EVOLUTION_SYSTEM
	void					ChangeWeapon(DWORD eWeapon, DWORD dwWeaponEvolution = 0);
#else
	void					ChangeWeapon(DWORD eWeapon);
#endif
	void					ChangeHair(DWORD eHair);
	void					ChangeGuild(DWORD dwGuildID);
#ifdef ENABLE_SKILL_COLOR_SYSTEM
	void					ChangeSkillColor(const DWORD* dwSkillColor);
#endif
	DWORD				GetWeaponType();

	void					SetComboType(UINT uComboType);
	void					SetAttackSpeed(UINT uAtkSpd);
	void					SetMoveSpeed(UINT uMovSpd);
	void					SetRotationSpeed(float fRotSpd);

	const char* GetNameString();
#ifdef ENABLE_EXCHANGE_REWORK
	DWORD					GetLevel();
#endif
	void					SetLevel(DWORD dwLevel);
#ifdef ENABLE_GUILD_LEADER_SYSTEM
	BYTE					GetGuildMemberType();
	void					SetGuildMemberType(BYTE type);
#endif
	int						GetInstanceType();
	DWORD					GetPart(CRaceData::EParts part);
	DWORD					GetShape();
	DWORD					GetRace();
	DWORD					GetVirtualID();
	DWORD					GetVirtualNumber();
	DWORD					GetEmpireID();
	DWORD					GetGuildID();
#ifdef ENABLE_MOB_AGGR_LVL_INFO
	DWORD					GetAIFlag();
#endif
#ifdef ENABLE_SKILL_COLOR_SYSTEM
	DWORD* GetSkillColor(const DWORD dwSkillIndex);
#endif
	int						GetAlignment();
	UINT					GetAlignmentGrade();
	int						GetAlignmentType();
#ifdef ENABLE_RANK_SYSTEM
	int						GetRank();
	void					SetRank(short sRank);
#endif
#ifdef ENABLE_LANDRANK_SYSTEM
	int						GetLandRank();
	static void				GetLandRankString(string& sLandRank, short val);
	void					SetLandRank(short sLandRank);
#endif
#ifdef ENABLE_REBORN_SYSTEM
	int						GetReborn();
	void					SetReborn(short sReborn);
#endif
#ifdef ENABLE_TEAM_SYSTEM
	int						GetTeam();
	void					SetTeam(short sTeam);
#endif
#ifdef ENABLE_MONIKER_SYSTEM
	const char*				GetMoniker();
	void					SetMoniker(const char * moniker);
#endif
#ifdef ENABLE_LOVE_SYSTEM
	const char*				GetLove1();
	const char*				GetLove2();

	void					SetLove1(const char* love1);
	void					SetLove2(const char* love2);
#endif // ENABLE_LOVE_SYSTEM
#ifdef ENABLE_WORD_SYSTEM
	const char*				GetWord();
	void					SetWord(const char* word);
#endif
	BYTE					GetPKMode();
	bool					IsKiller();
	bool					IsPartyMember();

	void					ActDualEmotion(CInstanceBase& rkDstInst, WORD dwMotionNumber1, WORD dwMotionNumber2);
	void					ActEmotion(DWORD dwMotionNumber);
	void					LevelUp();
	void					SkillUp();
	void					Revive();
	void					Stun();
	void					Die();
	void					Hide();
	void					Show();

	bool					CanAct();
	bool					CanMove();
	bool					CanAttack();
	bool					CanUseSkill();
	bool					CanFishing();
	bool					IsConflictAlignmentInstance(CInstanceBase& rkInstVictim);
	bool					IsAttackableInstance(CInstanceBase& rkInstVictim);
	bool					IsTargetableInstance(CInstanceBase& rkInstVictim);
	bool					IsPVPInstance(CInstanceBase& rkInstVictim);
	bool					CanChangeTarget();
	bool					CanPickInstance();
	bool					CanViewTargetHP(CInstanceBase& rkInstVictim);

	// Movement
	BOOL					IsGoing();
	bool					NEW_Goto(const TPixelPosition& c_rkPPosDst, float fDstRot);
	void					EndGoing();

	void					SetRunMode();
	void					SetWalkMode();

	bool					IsAffect(UINT uAffect);
	BOOL					IsInvisibility();
	BOOL					IsParalysis();
	BOOL					IsGameMaster();
	BOOL					IsSameEmpire(CInstanceBase& rkInstDst);
	BOOL					IsBowMode();
	BOOL					IsHandMode();
	BOOL					IsFishingMode();
	BOOL					IsFishing();

	BOOL					IsHoldingPickAxe();
	BOOL					IsMountingHorse();
	BOOL					IsForceVisible();
	BOOL					IsInSafe();
#ifdef ENABLE_GROWTH_PET_SYSTEM
	BOOL					IsNewPet();
#endif
#ifdef ENABLE_GROWTH_MOUNT_SYSTEM
	BOOL					IsNewExMount();
#endif
	BOOL					IsEnemy();
	BOOL					IsStone();
#ifdef ENABLE_BOSS_ON_MINIMAP
	BOOL					IsBoss();
#endif
	BOOL					IsResource();
	BOOL					IsNPC();
	BOOL					IsPC();
	BOOL					IsWarp();
	BOOL					IsGoto();
	BOOL					IsObject();
	BOOL					IsDoor();
	BOOL					IsWoodenDoor();
	BOOL					IsStoneDoor();
	BOOL					IsFlag();
	BOOL					IsGuildWall();
	BOOL					IsPet();
	BOOL					IsMount();
	BOOL					IsDead();
	BOOL					IsStun();
	BOOL					IsSleep();
	BOOL					__IsSyncing();
	BOOL					IsWaiting();
	BOOL					IsWalking();
	BOOL					IsPushing();
	BOOL					IsAttacking();
	BOOL					IsActingEmotion();
	BOOL					IsAttacked();
	BOOL					IsKnockDown();
	BOOL					IsUsingSkill();
	BOOL					IsUsingMovingSkill();
	BOOL					CanCancelSkill();
	BOOL					CanAttackHorseLevel();

	bool					NEW_CanMoveToDestPixelPosition(const TPixelPosition& c_rkPPosDst);

	void					NEW_SetAdvancingRotationFromPixelPosition(const TPixelPosition& c_rkPPosSrc, const TPixelPosition& c_rkPPosDst);
	void					NEW_SetAdvancingRotationFromDirPixelPosition(const TPixelPosition& c_rkPPosDir);
	bool					NEW_SetAdvancingRotationFromDestPixelPosition(const TPixelPosition& c_rkPPosDst);
	void					SetAdvancingRotation(float fRotation);

	void					EndWalking(float fBlendingTime = 0.15f);
	void					EndWalkingWithoutBlending();

	// Battle
	void					SetEventHandler(CActorInstance::IEventHandler* pkEventHandler);

	void					PushTCPState(DWORD dwCmdTime, const TPixelPosition& c_rkPPosDst, float fDstRot, UINT eFunc, UINT uArg);
	void					PushTCPStateExpanded(DWORD dwCmdTime, const TPixelPosition& c_rkPPosDst, float fDstRot, UINT eFunc, UINT uArg, UINT uTargetVID);

	void					NEW_Stop();

	bool					NEW_UseSkill(UINT uSkill, UINT uMot, UINT uMotLoopCount, bool isMovingSkill);
	void					NEW_Attack();
	void					NEW_Attack(float fDirRot);
	void					NEW_AttackToDestPixelPositionDirection(const TPixelPosition& c_rkPPosDst);
	bool					NEW_AttackToDestInstanceDirection(CInstanceBase& rkInstDst, IFlyEventHandler* pkFlyHandler);
	bool					NEW_AttackToDestInstanceDirection(CInstanceBase& rkInstDst);

	bool					NEW_MoveToDestPixelPositionDirection(const TPixelPosition& c_rkPPosDst);
	void					NEW_MoveToDestInstanceDirection(CInstanceBase& rkInstDst);
	void					NEW_MoveToDirection(float fDirRot);

	float					NEW_GetDistanceFromDirPixelPosition(const TPixelPosition& c_rkPPosDir);
	float					NEW_GetDistanceFromDestPixelPosition(const TPixelPosition& c_rkPPosDst);
	float					NEW_GetDistanceFromDestInstance(CInstanceBase& rkInstDst);

	float					NEW_GetRotation();
	float					NEW_GetRotationFromDestPixelPosition(const TPixelPosition& c_rkPPosDst);
	float					NEW_GetRotationFromDirPixelPosition(const TPixelPosition& c_rkPPosDir);
	float					NEW_GetRotationFromDestInstance(CInstanceBase& rkInstDst);

	float					NEW_GetAdvancingRotationFromDirPixelPosition(const TPixelPosition& c_rkPPosDir);
	float					NEW_GetAdvancingRotationFromDestPixelPosition(const TPixelPosition& c_rkPPosDst);
	float					NEW_GetAdvancingRotationFromPixelPosition(const TPixelPosition& c_rkPPosSrc, const TPixelPosition& c_rkPPosDst);

	BOOL					NEW_IsClickableDistanceDestPixelPosition(const TPixelPosition& c_rkPPosDst);
	BOOL					NEW_IsClickableDistanceDestInstance(CInstanceBase& rkInstDst);

	bool					NEW_GetFrontInstance(CInstanceBase** ppoutTargetInstance, float fDistance);
	void					NEW_GetRandomPositionInFanRange(CInstanceBase& rkInstTarget, TPixelPosition* pkPPosDst);
	bool					NEW_GetInstanceVectorInFanRange(float fSkillDistance, CInstanceBase& rkInstTarget, std::vector<CInstanceBase*>* pkVct_pkInst);
	bool					NEW_GetInstanceVectorInCircleRange(float fSkillDistance, std::vector<CInstanceBase*>* pkVct_pkInst);

	void					NEW_SetOwner(DWORD dwOwnerVID);
	void					NEW_SyncPixelPosition(long& nPPosX, long& nPPosY);

	void					NEW_SetPixelPosition(const TPixelPosition& c_rkPPosDst);

	bool					NEW_IsLastPixelPosition();
	const TPixelPosition& NEW_GetLastPixelPositionRef();

	// Battle
	BOOL					isNormalAttacking();
	BOOL					isComboAttacking();
	MOTION_KEY				GetNormalAttackIndex();
	DWORD					GetComboIndex();
	float					GetAttackingElapsedTime();
	void					InputNormalAttack(float fAtkDirRot);
	void					InputComboAttack(float fAtkDirRot);

	void					RunNormalAttack(float fAtkDirRot);
	void					RunComboAttack(float fAtkDirRot, DWORD wMotionIndex);

	BOOL					CheckAdvancing();

	bool					AvoidObject(const CGraphicObjectInstance& c_rkBGObj);
	bool					IsBlockObject(const CGraphicObjectInstance& c_rkBGObj);
	void					BlockMovement();

public:
	BOOL					CheckAttacking(CInstanceBase& rkInstVictim);
	void					ProcessHitting(DWORD dwMotionKey, CInstanceBase* pVictimInstance);
	void					ProcessHitting(DWORD dwMotionKey, BYTE byEventIndex, CInstanceBase* pVictimInstance);
	void					GetBlendingPosition(TPixelPosition* pPixelPosition);
	void					SetBlendingPosition(const TPixelPosition& c_rPixelPosition);

	// Fishing
	void					StartFishing(float frot);
	void					StopFishing();
	void					ReactFishing();
	void					CatchSuccess();
	void					CatchFail();
	BOOL					GetFishingRot(int* pirot);

	// Render Mode
	void					RestoreRenderMode();
	void					SetAddRenderMode();
	void					SetModulateRenderMode();
	void					SetRenderMode(int iRenderMode);
	void					SetAddColor(const D3DXCOLOR& c_rColor);

	// Position
	void					SCRIPT_SetPixelPosition(float fx, float fy);
	void					NEW_GetPixelPosition(TPixelPosition* pPixelPosition);

	// Rotation
	void					NEW_LookAtFlyTarget();
	void					NEW_LookAtDestInstance(CInstanceBase& rkInstDst);
	void					NEW_LookAtDestPixelPosition(const TPixelPosition& c_rkPPosDst);

	float					GetRotation();
	float					GetAdvancingRotation();
	void					SetRotation(float fRotation);
	void					BlendRotation(float fRotation, float fBlendTime = 0.1f);

	void					SetDirection(int dir);
	void					BlendDirection(int dir, float blendTime);
	float					GetDegreeFromDirection(int dir);

	// Motion
	//	Motion Deque
	BOOL					isLock();

	void					SetMotionMode(int iMotionMode);
	int						GetMotionMode(DWORD dwMotionIndex);

	// Motion
	//	Pushing Motion
	void					ResetLocalTime();
	void					SetLoopMotion(WORD wMotion, float fBlendTime = 0.1f, float fSpeedRatio = 1.0f);
	void					PushOnceMotion(WORD wMotion, float fBlendTime = 0.1f, float fSpeedRatio = 1.0f);
	void					PushLoopMotion(WORD wMotion, float fBlendTime = 0.1f, float fSpeedRatio = 1.0f);
	void					SetEndStopMotion();

	// Intersect
	bool					IntersectDefendingSphere();
	bool					IntersectBoundingBox();

	// Part
	//void					SetParts(const WORD * c_pParts);
	void					Refresh(DWORD dwMotIndex, bool isLoop);

	float					GetDistance(CInstanceBase* pkTargetInst);
	float					GetDistance(const TPixelPosition& c_rPixelPosition);

	// ETC
	CActorInstance& GetGraphicThingInstanceRef();
	CActorInstance* GetGraphicThingInstancePtr();

	bool __Background_IsWaterPixelPosition(const TPixelPosition& c_rkPPos);
	bool __Background_GetWaterHeight(const TPixelPosition& c_rkPPos, float* pfHeight);

	/////////////////////////////////////////////////////////////
	void __ClearAffectFlagContainer();
	void __ClearAffects();
	/////////////////////////////////////////////////////////////

	void __SetAffect(UINT eAffect, bool isVisible);

	void SetAffectFlagContainer(const CAffectFlagContainer& c_rkAffectFlagContainer);

	void __SetNormalAffectFlagContainer(const CAffectFlagContainer& c_rkAffectFlagContainer);
	void __SetStoneSmokeFlagContainer(const CAffectFlagContainer& c_rkAffectFlagContainer);

	void SetEmoticon(UINT eEmoticon);
	void SetFishEmoticon();
	bool IsPossibleEmoticon();

protected:
	UINT					__LessRenderOrder_GetLODLevel();
	void					__Initialize();
	void					__InitializeRotationSpeed();

	void					__Create_SetName(const SCreateData& c_rkCreateData);
	void					__Create_SetWarpName(const SCreateData& c_rkCreateData);

	CInstanceBase* __GetMainInstancePtr();
	CInstanceBase* __FindInstancePtr(DWORD dwVID);

	bool  __FindRaceType(DWORD dwRace, BYTE* pbType);
	DWORD __GetRaceType();

	void __EnableSkipCollision();
	void __DisableSkipCollision();

	void __ClearMainInstance();

	void __Shaman_SetParalysis(bool isParalysis);
	void __Warrior_SetGeomgyeongAffect(bool isVisible);
	void __Assassin_SetEunhyeongAffect(bool isVisible);
	void __SetReviveInvisibilityAffect(bool isVisible);

	BOOL __CanProcessNetworkStatePacket();

	bool __IsInDustRange();

	// Emotion
	void __ProcessFunctionEmotion(DWORD dwMotionNumber, DWORD dwTargetVID, const TPixelPosition& c_rkPosDst);
	void __EnableChangingTCPState();
	void __DisableChangingTCPState();
	BOOL __IsEnableTCPProcess(UINT eCurFunc);

	bool __CanRender();
	bool __IsInViewFrustum();

	// HORSE
	void __AttachHorseSaddle();
	void __DetachHorseSaddle();

	struct SHORSE
	{
		bool m_isMounting;
		CActorInstance* m_pkActor;

		SHORSE();
		~SHORSE();

		void Destroy();
		void Create(const TPixelPosition& c_rkPPos, UINT eRace, UINT eHitEffect);

		void SetAttackSpeed(UINT uAtkSpd);
		void SetMoveSpeed(UINT uMovSpd);
		void Deform();
		void Render();
		CActorInstance& GetActorRef();
		CActorInstance* GetActorPtr();

		bool IsMounting();
		bool CanAttack();
		bool CanUseSkill();

		UINT GetLevel();
		void __Initialize();
	} m_kHorse;

protected:
	// Blend Mode
	void					__SetBlendRenderingMode();
	void					__SetAlphaValue(float fAlpha);
	float					__GetAlphaValue();

	void					MovementProcess();
	void					StateProcess();
	void					AttackProcess();

	void					StartWalking();
	float					GetLocalTime();

#ifdef ENABLE_MODEL_RENDER_TARGET
	// @@
	// sadece render icin
	public:
		void					RefreshState(DWORD dwMotIndex, bool isLoop);
	protected:
#else
		void					RefreshState(DWORD dwMotIndex, bool isLoop);
#endif
	void					RefreshActorInstance();

protected:
	void					OnSyncing();
	void					OnWaiting();
	void					OnMoving();

	void					NEW_SetSrcPixelPosition(const TPixelPosition& c_rkPPosDst);
	void					NEW_SetDstPixelPosition(const TPixelPosition& c_rkPPosDst);
	void					NEW_SetDstPixelPositionZ(FLOAT z);

	const TPixelPosition& NEW_GetCurPixelPositionRef();
	const TPixelPosition& NEW_GetSrcPixelPositionRef();

public:
	const TPixelPosition& NEW_GetDstPixelPositionRef();

protected:
	BOOL m_isTextTail;

	// Instance Data
	std::string					m_stName;

	DWORD					m_awPart[CRaceData::PART_MAX_NUM];

	DWORD					m_dwLevel;
#if defined(ENABLE_MOB_AGGR_LVL_INFO)
	DWORD					m_dwAIFlag;
#endif
#ifdef ENABLE_ITEM_EVOLUTION_SYSTEM
	DWORD					m_dwWeaponEvolution;
#endif
#ifdef ENABLE_ARMOR_EVOLUTION_SYSTEM
	DWORD					m_dwArmorEvolution;
#endif
	DWORD					m_dwEmpireID;
	DWORD					m_dwGuildID;

protected:
	CAffectFlagContainer		m_kAffectFlagContainer;
	DWORD					m_adwCRCAffectEffect[AFFECT_NUM];
#ifdef ENABLE_SKILL_COLOR_SYSTEM
	DWORD* m_dwSkillColor[ESkillColorLength::MAX_SKILL_COUNT + MAX_BUFF_COUNT][ESkillColorLength::MAX_EFFECT_COUNT];
#endif
#ifdef ENABLE_ITEM_EVOLUTION_SYSTEM
#ifdef ENABLE_ARMOR_EVOLUTION_SYSTEM
	UINT	__GetRefinedEffect(CItemData* pItem, DWORD dwWeaponEvolution = 0, DWORD dwArmorEvolution = 0);
#else
	UINT	__GetRefinedEffect(CItemData* pItem, DWORD dwWeaponEvolution = 0);
#endif
#else
	UINT	__GetRefinedEffect(CItemData* pItem);
#endif
	void	__ClearWeaponRefineEffect();
	void	__ClearArmorRefineEffect();

#ifdef ENABLE_ACCE_SYSTEM
	void	__ClearAcceRefineEffect();
	DWORD	m_dwAcceSpecular;
#endif
#ifdef ENABLE_SHINING_EFFECT_UTILITY
	void	__GetShiningEffect(CItemData* pItem);
	void	__ClearWeaponShiningEffect(bool detaching = true);
	void	__ClearArmorShiningEffect(bool detaching = true);
	void	__ClearAcceShiningEffect(bool detaching = true);
	void	__AttachWeaponShiningEffect(int effectIndex, const char* effectFileName, const char* boneName = "Bip01", const char* = "None");
	void	__AttachArmorShiningEffect(int effectIndex, const char* effectFileName, const char* boneName = "Bip01", const char* = "None");
	void	__AttachAcceShiningEffect(int effectIndex, const char* effectFileName, const char* boneName = "Bip01", const char* = "None");
#endif

#ifdef ENABLE_SHINING_ITEM_SYSTEM
	INT		__GetShiningEffect(WORD slot);
	bool	ClearShiningEffect();
	bool	AttachShiningEffect(DWORD dwWeapon);
#endif

protected:
	void __AttachSelectEffect();
	void __DetachSelectEffect();

	void __AttachTargetEffect();
	void __DetachTargetEffect();

	void __AttachEmpireEffect(DWORD eEmpire);

protected:
	struct SEffectContainer
	{
		typedef std::map<DWORD, DWORD> Dict;
		Dict m_kDct_dwEftID;
	} m_kEffectContainer;

	void __EffectContainer_Initialize();
	void __EffectContainer_Destroy();
public:
	void DestroyContainer();
	void AttachContainer();
protected:
#ifdef ENABLE_GRAPHIC_OPTIMIZATION
	DWORD __EffectContainer_AttachEffect(DWORD eEffect, bool ignoreFrustum=false);
#else
	DWORD __EffectContainer_AttachEffect(DWORD eEffect);
#endif
	void __EffectContainer_DetachEffect(DWORD eEffect);

	SEffectContainer::Dict& __EffectContainer_GetDict();

protected:
	struct SStoneSmoke
	{
		DWORD m_dwEftID;
	} m_kStoneSmoke;

	void __StoneSmoke_Inialize();
	void __StoneSmoke_Destroy();
	void __StoneSmoke_Create(DWORD eSmoke);

protected:
	// Emoticon
	//DWORD					m_adwCRCEmoticonEffect[EMOTICON_NUM];

	BYTE					m_eType;
	BYTE					m_eRaceType;
	DWORD					m_eShape;
	DWORD					m_dwRace;
	DWORD					m_dwVirtualNumber;
#ifdef ENABLE_ALIGNMENT_SYSTEM
	int						m_sAlignment;
#else
	short					m_sAlignment;
#endif
#ifdef ENABLE_RANK_SYSTEM
	short					m_sRank;
#endif
#ifdef ENABLE_LANDRANK_SYSTEM
	short					m_sLandRank;
#endif
#ifdef ENABLE_REBORN_SYSTEM
	short					m_sReborn;
#endif
#ifdef ENABLE_TEAM_SYSTEM
	short					m_sTeam;
#endif
#ifdef ENABLE_MONIKER_SYSTEM
	std::string				m_Moniker;
#endif
#ifdef ENABLE_LOVE_SYSTEM
	std::string				m_Love1;
	std::string				m_Love2;
#endif // ENABLE_LOVE_SYSTEM
#ifdef ENABLE_WORD_SYSTEM
	std::string				m_word;
#endif // ENABLE_WORD_SYSTEM

	BYTE					m_byPKMode;
	bool					m_isKiller;
	bool					m_isPartyMember;

	// Movement
	int						m_iRotatingDirection;

	DWORD					m_dwAdvActorVID;
	DWORD					m_dwLastDmgActorVID;

	LONG					m_nAverageNetworkGap;
	DWORD					m_dwNextUpdateHeightTime;

	bool					m_isGoing;

	TPixelPosition			m_kPPosDust;

	DWORD					m_dwLastComboIndex;

	DWORD					m_swordRefineEffectRight;
	DWORD					m_swordRefineEffectLeft;
	DWORD					m_armorRefineEffect;
#ifdef ENABLE_ACCE_SYSTEM
	DWORD					m_acceRefineEffect;
#endif
#ifdef ENABLE_AURA_COSTUME_SYSTEM
	DWORD					m_auraRefineEffect;
#endif
#ifdef ENABLE_SHINING_EFFECT_UTILITY
	DWORD					m_weaponShiningEffects[2][CItemData::ITEM_SHINING_MAX_COUNT];
	DWORD					m_armorShiningEffects[CItemData::ITEM_SHINING_MAX_COUNT];
	DWORD					m_acceShiningEffects[CItemData::ITEM_SHINING_MAX_COUNT];
#endif
#ifdef ENABLE_SHINING_ITEM_SYSTEM
public:
	DWORD					m_dwShining[6];//CHR_SHINING_NUM
	DWORD					m_armorShining;
	DWORD					m_swordShiningRight;
	DWORD					m_swordShiningLeft;
	DWORD					m_specialShining;
	DWORD					m_specialShining2;
	DWORD					m_specialShining3;
	DWORD					m_wingShining;
protected:
#endif

#ifdef ENABLE_GUILD_LEADER_SYSTEM
	BYTE					m_bMemberType;
#endif

	struct SMoveAfterFunc
	{
		UINT eFunc;
		UINT uArg;

		// For Emotion Function
		UINT uArgExpanded;
		TPixelPosition kPosDst;
	};

	SMoveAfterFunc m_kMovAfterFunc;

	float m_fDstRot;
	float m_fAtkPosTime;
	float m_fRotSpd;
	float m_fMaxRotSpd;

	BOOL m_bEnableTCPState;

	// Graphic Instance
	CActorInstance m_GraphicThingInstance;

protected:
	struct SCommand
	{
		DWORD	m_dwChkTime;
		DWORD	m_dwCmdTime;
		float	m_fDstRot;
		UINT 	m_eFunc;
		UINT 	m_uArg;
		UINT	m_uTargetVID;
		TPixelPosition m_kPPosDst;
	};

	typedef std::list<SCommand> CommandQueue;

	DWORD		m_dwBaseChkTime;
	DWORD		m_dwBaseCmdTime;

	DWORD		m_dwSkipTime;

	CommandQueue m_kQue_kCmdNew;

	BOOL		m_bDamageEffectType;

	struct SEffectDamage
	{
#ifdef ENABLE_DAMAGE_LIMIT_REWORK
		unsigned long long damage;
#else
		DWORD damage;
#endif
		BYTE flag;
		BOOL bSelf;
		BOOL bTarget;
	};

	typedef std::list<SEffectDamage> CommandDamageQueue;
	CommandDamageQueue m_DamageQueue;

	void ProcessDamage();

public:
#ifdef ENABLE_DAMAGE_LIMIT_REWORK
	void AddDamageEffect(unsigned long long damage, BYTE flag, BOOL bSelf, BOOL bTarget);
#else
	void AddDamageEffect(DWORD damage, BYTE flag, BOOL bSelf, BOOL bTarget);
#endif
protected:
	struct SWarrior
	{
		DWORD m_dwGeomgyeongEffect;
	};

	SWarrior m_kWarrior;

	void __Warrior_Initialize();

public:
	static void ClearPVPKeySystem();

	static void InsertPVPKey(DWORD dwSrcVID, DWORD dwDstVID);
	static void InsertPVPReadyKey(DWORD dwSrcVID, DWORD dwDstVID);
	static void RemovePVPKey(DWORD dwSrcVID, DWORD dwDstVID);

	static void InsertGVGKey(DWORD dwSrcGuildVID, DWORD dwDstGuildVID);
	static void RemoveGVGKey(DWORD dwSrcGuildVID, DWORD dwDstGuildVID);

	static void InsertDUELKey(DWORD dwSrcVID, DWORD dwDstVID);

	UINT GetNameColorIndex();

	const D3DXCOLOR& GetNameColor();
	const D3DXCOLOR& GetTitleColor();

protected:
	static DWORD __GetPVPKey(DWORD dwSrcVID, DWORD dwDstVID);
	static bool __FindPVPKey(DWORD dwSrcVID, DWORD dwDstVID);
	static bool __FindPVPReadyKey(DWORD dwSrcVID, DWORD dwDstVID);
	static bool __FindGVGKey(DWORD dwSrcGuildID, DWORD dwDstGuildID);
	static bool __FindDUELKey(DWORD dwSrcGuildID, DWORD dwDstGuildID);

protected:
	CActorInstance::IEventHandler* GetEventHandlerPtr();
	CActorInstance::IEventHandler& GetEventHandlerRef();

protected:
	static float __GetBackgroundHeight(float x, float y);
	static DWORD __GetShadowMapColor(float x, float y);

public:
	static void ResetPerformanceCounter();
	static void GetInfo(std::string* pstInfo);

public:
	static CInstanceBase* New();
	static void Delete(CInstanceBase* pkInst);

	static CDynamicPool<CInstanceBase>	ms_kPool;

protected:
	static DWORD ms_dwUpdateCounter;
	static DWORD ms_dwRenderCounter;
	static DWORD ms_dwDeformCounter;

public:
	DWORD					GetDuelMode();
	void					SetDuelMode(DWORD type);
protected:
	DWORD					m_dwDuelMode;
	DWORD					m_dwEmoticonTime;
#ifdef ENABLE_MODEL_RENDER_TARGET
protected:
	bool					m_IsAlwaysRender;
public:
	bool					IsAlwaysRender();
	void					SetAlwaysRender(bool val);
#endif
#ifdef ENABLE_HIDE_BODY_PARTS
public:
	void					UpdatePartsBySetting(BYTE setting, bool hide);
#endif // ENABLE_HIDE_BODY_PARTS
#ifdef ENABLE_DOG_MODE
public:
	DWORD GetOriginalRace();
	void SetOriginalRace(DWORD race);
	void ChangeRace(DWORD eRace, DWORD eShape);

protected:
	DWORD m_dwOriginalRace;
#endif
#ifdef ENABLE_AUTO_HUNT_SYSTEM
public:
	void	SetHasAttacker(DWORD f) { m_hasAttacker = f; }
	DWORD	HasAttacker() { return m_hasAttacker; }
	void	SetAttacked(DWORD f) {  m_dwAttacked = f; }
	DWORD	GetAttacked() const { return m_dwAttacked; }
protected:
	DWORD		m_hasAttacker;
	DWORD		m_dwAttacked;
#endif // ENABLE_AUTO_HUNT_SYSTEM
};

inline int RaceToJob(int race)
{
#ifdef ENABLE_WOLFMAN_CHARACTER
	if (race == 8)
		return 4;
#endif
	const int JOB_NUM = 4;
	return race % JOB_NUM;
}

inline int RaceToSex(int race)
{
	switch (race)
	{
	case 0:
	case 2:
	case 5:
	case 7:
#ifdef ENABLE_WOLFMAN_CHARACTER
	case 8:
#endif
		return 1;
	case 1:
	case 3:
	case 4:
	case 6:
		return 0;
	}
	return 0;
}

inline int DISTANCE_APPROX(int dx, int dy)
{
	int min, max;

	if (dx < 0)
		dx = -dx;

	if (dy < 0)
		dy = -dy;

	if (dx < dy)
	{
		min = dx;
		max = dy;
	}
	else
	{
		min = dy;
		max = dx;
	}

	// coefficients equivalent to ( 123/128 * max ) and ( 51/128 * min )
	return (((max << 8) + (max << 3) - (max << 4) - (max << 1) +
		(min << 7) - (min << 5) + (min << 3) - (min << 1)) >> 8);
}