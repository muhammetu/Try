#include "StdAfx.h"
#include "GameType.h"

std::string g_strResourcePath = "d:/ymir work/";
std::string g_strImagePath = "d:/ymir work/ui/";

// DEFAULT_FONT
static std::string gs_strDefaultFontName = "±¼¸²Ã¼:12.fnt";
static std::string gs_strDefaultItalicFontName = "±¼¸²Ã¼:12i.fnt";
static CResource* gs_pkDefaultFont = NULL;
static CResource* gs_pkDefaultItalicFont = NULL;

static bool gs_isReloadDefaultFont = false;

void DefaultFont_Startup()
{
	gs_pkDefaultFont = NULL;
}

void DefaultFont_Cleanup()
{
	if (gs_pkDefaultFont)
		gs_pkDefaultFont->Release();
}

void DefaultFont_SetName(const char* c_szFontName)
{
	gs_strDefaultFontName = c_szFontName;
	gs_strDefaultFontName += ".fnt";

	gs_strDefaultItalicFontName = c_szFontName;
	if (strchr(c_szFontName, ':'))
		gs_strDefaultItalicFontName += "i";
	gs_strDefaultItalicFontName += ".fnt";

	gs_isReloadDefaultFont = true;
}

bool ReloadDefaultFonts()
{
	CResourceManager& rkResMgr = CResourceManager::Instance();

	gs_isReloadDefaultFont = false;

	CResource* pkNewFont = rkResMgr.GetResourcePointer(gs_strDefaultFontName.c_str());
	pkNewFont->AddReference();
	if (gs_pkDefaultFont)
		gs_pkDefaultFont->Release();
	gs_pkDefaultFont = pkNewFont;

	CResource* pkNewItalicFont = rkResMgr.GetResourcePointer(gs_strDefaultItalicFontName.c_str());
	pkNewItalicFont->AddReference();
	if (gs_pkDefaultItalicFont)
		gs_pkDefaultItalicFont->Release();
	gs_pkDefaultItalicFont = pkNewItalicFont;

	return true;
}

CResource* DefaultFont_GetResource()
{
	if (!gs_pkDefaultFont || gs_isReloadDefaultFont)
		ReloadDefaultFonts();
	return gs_pkDefaultFont;
}

// END_OF_DEFAULT_FONT

BYTE c_aSlotTypeToInvenType[SLOT_TYPE_MAX] =
{
	RESERVED_WINDOW,		// SLOT_TYPE_NONE
	INVENTORY,				// SLOT_TYPE_INVENTORY
	RESERVED_WINDOW,		// SLOT_TYPE_SKILL
	RESERVED_WINDOW,		// SLOT_TYPE_EMOTION
	RESERVED_WINDOW,		// SLOT_TYPE_SHOP
	RESERVED_WINDOW,		// SLOT_TYPE_EXCHANGE_OWNER
	RESERVED_WINDOW,		// SLOT_TYPE_EXCHANGE_TARGET
	RESERVED_WINDOW,		// SLOT_TYPE_QUICK_SLOT
	RESERVED_WINDOW,
	RESERVED_WINDOW,		// SLOT_TYPE_PRIVATE_SHOP
	RESERVED_WINDOW,
	DRAGON_SOUL_INVENTORY,	// SLOT_TYPE_DRAGON_SOUL_INVENTORY
#ifdef ENABLE_ADDITIONAL_INVENTORY
	UPGRADE_INVENTORY,
	BOOK_INVENTORY,
	STONE_INVENTORY,
	FLOWER_INVENTORY,
	ATTR_INVENTORY,
	CHEST_INVENTORY,
#endif
#ifdef ENABLE_SWITCHBOT_SYSTEM
	SWITCHBOT,
#endif
};

BYTE SlotTypeToInvenType(BYTE bSlotType)
{
	if (bSlotType >= SLOT_TYPE_MAX)
		return RESERVED_WINDOW;
	else
		return c_aSlotTypeToInvenType[bSlotType];
}

#ifdef ENABLE_DETAILS_INTERFACE
#include "Packet.h"
typedef struct SApplyInfo
{
	BYTE	bPointType;                          // APPLY -> POINT
} TApplyInfo;
const TApplyInfo aApplyInfo[CItemData::MAX_APPLY_NUM] =
{
	{ CItemData::POINT_NONE,						},	// APPLY_NONE,
	{ CItemData::POINT_MAX_HP,		        		},	// APPLY_MAX_HP,
	{ CItemData::POINT_MAX_SP,		        		},	// APPLY_MAX_SP,
	{ CItemData::POINT_HT,			        		},	// APPLY_CON,
	{ CItemData::POINT_IQ,			        		},	// APPLY_INT,
	{ CItemData::POINT_ST,			        		},	// APPLY_STR,
	{ CItemData::POINT_DX,			        		},	// APPLY_DEX,
	{ CItemData::POINT_ATT_SPEED,		    		},	// APPLY_ATT_SPEED,
	{ CItemData::POINT_MOV_SPEED,		    		},	// APPLY_MOV_SPEED,
	{ CItemData::POINT_CASTING_SPEED,	    		},	// APPLY_CAST_SPEED,
	{ CItemData::POINT_HP_REGEN,					},	// APPLY_HP_REGEN,
	{ CItemData::POINT_SP_REGEN,					},	// APPLY_SP_REGEN,
	{ CItemData::POINT_POISON_PCT,		    		},	// APPLY_POISON_PCT,
	{ CItemData::POINT_STUN_PCT,		    		},	// APPLY_STUN_PCT,
	{ CItemData::POINT_SLOW_PCT,		    		},	// APPLY_SLOW_PCT,
	{ CItemData::POINT_CRITICAL_PCT,				},	// APPLY_CRITICAL_PCT,
	{ CItemData::POINT_PENETRATE_PCT,				},	// APPLY_PENETRATE_PCT,
	{ CItemData::POINT_ATTBONUS_HUMAN,				},	// APPLY_ATTBONUS_HUMAN,
	{ CItemData::POINT_ATTBONUS_ANIMAL,			},	// APPLY_ATTBONUS_ANIMAL,
	{ CItemData::POINT_ATTBONUS_ORC,				},	// APPLY_ATTBONUS_ORC,
	{ CItemData::POINT_ATTBONUS_MILGYO,			},	// APPLY_ATTBONUS_MILGYO,
	{ CItemData::POINT_ATTBONUS_UNDEAD,			},	// APPLY_ATTBONUS_UNDEAD,
	{ CItemData::POINT_ATTBONUS_DEVIL,				},	// APPLY_ATTBONUS_DEVIL,
	{ CItemData::POINT_STEAL_HP,					},	// APPLY_STEAL_HP,
	{ CItemData::POINT_STEAL_SP,					},	// APPLY_STEAL_SP,
	{ CItemData::POINT_MANA_BURN_PCT,				},	// APPLY_MANA_BURN_PCT,
	{ CItemData::POINT_DAMAGE_SP_RECOVER,			},	// APPLY_DAMAGE_SP_RECOVER,
	{ CItemData::POINT_BLOCK,		        		},	// APPLY_BLOCK,
	{ CItemData::POINT_DODGE,		        		},	// APPLY_DODGE,
	{ CItemData::POINT_RESIST_SWORD,				},	// APPLY_RESIST_SWORD,
	{ CItemData::POINT_RESIST_TWOHAND,				},	// APPLY_RESIST_TWOHAND,
	{ CItemData::POINT_RESIST_DAGGER,				},	// APPLY_RESIST_DAGGER,
	{ CItemData::POINT_RESIST_BELL,				},	// APPLY_RESIST_BELL,
	{ CItemData::POINT_RESIST_FAN,					},	// APPLY_RESIST_FAN,
	{ CItemData::POINT_RESIST_BOW,					},	// APPLY_RESIST_BOW,
	{ CItemData::POINT_RESIST_FIRE,				},	// APPLY_RESIST_FIRE,
	{ CItemData::POINT_RESIST_ELEC,				},	// APPLY_RESIST_ELEC,
	{ CItemData::POINT_RESIST_MAGIC,				},	// APPLY_RESIST_MAGIC,
	{ CItemData::POINT_RESIST_WIND,				},	// APPLY_RESIST_WIND,
	{ CItemData::POINT_REFLECT_MELEE,				},	// APPLY_REFLECT_MELEE,
	{ CItemData::POINT_REFLECT_CURSE,				},	// APPLY_REFLECT_CURSE,
	{ CItemData::POINT_POISON_REDUCE,				},	// APPLY_POISON_REDUCE,
	{ CItemData::POINT_KILL_SP_RECOVER,			},	// APPLY_KILL_SP_RECOVER,
	{ CItemData::POINT_EXP_DOUBLE_BONUS,			},	// APPLY_EXP_DOUBLE_BONUS,
	{ CItemData::POINT_GOLD_DOUBLE_BONUS,			},	// APPLY_GOLD_DOUBLE_BONUS,
	{ CItemData::POINT_ITEM_DROP_BONUS,			},	// APPLY_ITEM_DROP_BONUS,
	{ CItemData::POINT_POTION_BONUS,				},	// APPLY_POTION_BONUS,
	{ CItemData::POINT_NONE,			},	// APPLY_KILL_HP_RECOVER,
	{ CItemData::POINT_IMMUNE_STUN,				},	// APPLY_IMMUNE_STUN,
	{ CItemData::POINT_IMMUNE_SLOW,				},	// APPLY_IMMUNE_SLOW,
	{ CItemData::POINT_IMMUNE_FALL,				},	// APPLY_IMMUNE_FALL,
	{ CItemData::POINT_NONE,						},	// APPLY_SKILL,
	{ CItemData::POINT_BOW_DISTANCE,				},	// APPLY_BOW_DISTANCE,
	{ CItemData::POINT_ATT_GRADE_BONUS,			},	// APPLY_ATT_GRADE,
	{ CItemData::POINT_DEF_GRADE_BONUS,			},	// APPLY_DEF_GRADE,
	{ CItemData::POINT_MAGIC_ATT_GRADE_BONUS,		},	// APPLY_MAGIC_ATT_GRADE,
	{ CItemData::POINT_MAGIC_DEF_GRADE_BONUS,		},	// APPLY_MAGIC_DEF_GRADE,
	{ CItemData::POINT_CURSE_PCT,					},	// APPLY_CURSE_PCT,
	{ CItemData::POINT_MAX_STAMINA					},	// APPLY_MAX_STAMINA
	{ CItemData::POINT_ATTBONUS_UNUSED1, },	// APPLY_ATTBONUS_WARRIOR
	{ CItemData::POINT_ATTBONUS_UNUSED2,},	// APPLY_ATTBONUS_ASSASSIN
	{ CItemData::POINT_ATTBONUS_UNUSED3,},	// APPLY_ATTBONUS_SURA
	{ CItemData::POINT_ATTBONUS_UNUSED4,},	// APPLY_ATTBONUS_SHAMAN
	{ CItemData::POINT_ATTBONUS_UNUSED0, },	// APPLY_ATTBONUS_MONSTER
	{ CItemData::POINT_MALL_ATTBONUS					},	// APPLY_MALL_ATTBONUS
	{ CItemData::POINT_MALL_DEFBONUS				},	// APPLY_MALL_DEFBONUS
	{ CItemData::POINT_MALL_EXPBONUS				},	// APPLY_MALL_EXPBONUS
	{ CItemData::POINT_MALL_ITEMBONUS				},	// APPLY_MALL_ITEMBONUS
	{ CItemData::POINT_MALL_GOLDBONUS				},	// APPLY_MALL_GOLDBONUS
	{ CItemData::POINT_MAX_HP_PCT					},	// APPLY_MAX_HP_PCT
	{ CItemData::POINT_MAX_SP_PCT					},	// APPLY_MAX_SP_PCT
	{ CItemData::POINT_SKILL_DAMAGE_BONUS			},	// APPLY_SKILL_DAMAGE_BONUS
	{ CItemData::POINT_NORMAL_HIT_DAMAGE_BONUS		},	// APPLY_NORMAL_HIT_DAMAGE_BONUS
	{ CItemData::POINT_SKILL_DEFEND_BONUS			},	// APPLY_SKILL_DEFEND_BONUS
	{ CItemData::POINT_NORMAL_HIT_DEFEND_BONUS		},	// APPLY_NORMAL_HIT_DEFEND_BONUS
	{ CItemData::POINT_NONE, },
	{ CItemData::POINT_NONE, },
	{ CItemData::POINT_NONE,						},	// APPLY_EXTRACT_HP_PCT
	{ CItemData::POINT_ATTBONUS_UNUSED6, },	// APPLY_RESIST_WARRIOR
	{ CItemData::POINT_ATTBONUS_UNUSED7, },	// APPLY_RESIST_ASSASSIN
	{ CItemData::POINT_ATTBONUS_UNUSED8, },	// APPLY_RESIST_SURA
	{ CItemData::POINT_ATTBONUS_UNUSED9, },	// APPLY_RESIST_SHAMAN
	{ CItemData::POINT_ENERGY						},	// APPLY_ENERGY
	{ CItemData::POINT_DEF_GRADE					},	// APPLY_DEF_GRADE
	{ CItemData::POINT_NONE							},	// APPLY_COSTUME_ATTR_BONUS
	{ CItemData::POINT_NONE							},	// APPLY_MAGIC_ATTBONUS_PER
	{ CItemData::POINT_NONE							}, 	// APPLY_MELEE_MAGIC_ATTBONUS_PER
	{ CItemData::POINT_RESIST_ICE					},	// APPLY_RESIST_ICE
	{ CItemData::POINT_RESIST_EARTH					},	// APPLY_RESIST_EARTH
	{ CItemData::POINT_RESIST_DARK					},	// APPLY_RESIST_DARK
	{ CItemData::POINT_RESIST_CRITICAL				},	// APPLY_ANTI_CRITICAL_PCT
	{ CItemData::POINT_RESIST_PENETRATE				},	// APPLY_ANTI_PENETRATE_PCT
#ifdef ENABLE_WOLFMAN_CHARACTER
	{ CItemData::POINT_BLEEDING_REDUCE				},	// APPLY_BLEEDING_REDUCE
	{ CItemData::POINT_BLEEDING_PCT					},	// APPLY_BLEEDING_PCT
	{ CItemData::POINT_ATTBONUS_WOLFMAN				},	// APPLY_ATTBONUS_WOLFMAN
	{ CItemData::POINT_RESIST_WOLFMAN				},	// APPLY_RESIST_WOLFMAN
	{ CItemData::POINT_RESIST_CLAW					},	// APPLY_RESIST_CLAW
#endif
#ifdef ENABLE_ACCE_SYSTEM
	{ CItemData::POINT_ACCEDRAIN_RATE				},	// APPLY_ACCEDRAIN_RATE
#endif
#ifdef ENABLE_MAGIC_REDUCTION_SYSTEM
	{ CItemData::POINT_RESIST_MAGIC_REDUCTION		},	// APPLY_RESIST_MAGIC_REDUCTION
#endif
	{ CItemData::POINT_NONE							},	// APPLY_MOUNT
#ifdef ENABLE_PENDANT_SYSTEM
	{ CItemData::POINT_ENCHANT_FIRE					},	// APPLY_ENCHANT_FIRE
	{ CItemData::POINT_ENCHANT_ICE					},	// APPLY_ENCHANT_ICE
	{ CItemData::POINT_ENCHANT_EARTH				},	// APPLY_ENCHANT_EARTH
	{ CItemData::POINT_ENCHANT_DARK					},	// APPLY_ENCHANT_DARK
	{ CItemData::POINT_ENCHANT_WIND					},	// APPLY_ENCHANT_WIND
	{ CItemData::POINT_ENCHANT_ELECT				},	// APPLY_ENCHANT_ELECT
	{ CItemData::POINT_RESIST_HUMAN					},	// APPLY_RESIST_HUMAN
	{ CItemData::POINT_ATTBONUS_SWORD				},	// APPLY_ATTBONUS_SWORD
	{ CItemData::POINT_ATTBONUS_TWOHAND				},	// APPLY_ATTBONUS_TWOHAND
	{ CItemData::POINT_ATTBONUS_DAGGER				},	// APPLY_ATTBONUS_DAGGER
	{ CItemData::POINT_ATTBONUS_BELL				},	// APPLY_ATTBONUS_BELL
	{ CItemData::POINT_ATTBONUS_FAN					},	// APPLY_ATTBONUS_FAN
	{ CItemData::POINT_ATTBONUS_BOW					},	// APPLY_ATTBONUS_BOW
#ifdef ENABLE_WOLFMAN_CHARACTER
	{ CItemData::POINT_ATTBONUS_CLAW				},	// APPLY_ATTBONUS_CLAW
#endif
	{ CItemData::POINT_ATTBONUS_CZ					},	// APPLY_ATTBONUS_CZ
#endif
	{ CItemData::POINT_ATTBONUS_DESERT				},	// APPLY_ATTBONUS_DESERT
	{ CItemData::POINT_ATTBONUS_INSECT				},	// APPLY_ATTBONUS_INSECT
#ifdef ENABLE_ATTR_ADDONS
	{ CItemData::POINT_ATTBONUS_STONE				},	// APPLY_ATTBONUS_STONE
	{ CItemData::POINT_ATTBONUS_BOSS				},	// APPLY_ATTBONUS_BOSS
	{ CItemData::POINT_ATTBONUS_ELEMENTS				},	// APPLY_ATTBONUS_ELEMENTS
	{ CItemData::POINT_ENCHANT_ELEMENTS				},	// APPLY_ENCHANT_ELEMENTS
	{ CItemData::POINT_ATTBONUS_CHARACTERS				},	// APPLY_ATTBONUS_CHARACTERS
	{ CItemData::POINT_ENCHANT_CHARACTERS				},	// APPLY_ENCHANT_CHARACTERS
#endif
#ifdef ENABLE_CHEST_DROP_POINT
	{ CItemData::POINT_CHEST_BONUS				},	// APPLY_CHEST_BONUS
#endif
};
BYTE ApplyTypeToPointType(BYTE bApplyType)
{
	if (bApplyType >= CItemData::MAX_APPLY_NUM)
		return CItemData::POINT_NONE;
	else
		return aApplyInfo[bApplyType].bPointType;
}
BYTE PointTypeToApplyType(BYTE bPointType)
{
	for (BYTE i = 0; i < CItemData::MAX_APPLY_NUM; i++)
	{
		if (aApplyInfo[i].bPointType == bPointType)
			return i;
	}
	return CItemData::APPLY_NONE;
}
#endif