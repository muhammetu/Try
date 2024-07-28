#pragma once
#include "../GameLib/ItemData.h"
#include "StdAfx.h"

struct SAffects
{
	SAffects() : dwAffects(0) {}
	SAffects(const DWORD& c_rAffects)
	{
		__SetAffects(c_rAffects);
	}
	int operator = (const DWORD& c_rAffects)
	{
		__SetAffects(c_rAffects);
	}

	BOOL IsAffect(BYTE byIndex)
	{
		return dwAffects & (1 << byIndex);
	}

	void __SetAffects(const DWORD& c_rAffects)
	{
		dwAffects = c_rAffects;
	}

	DWORD dwAffects;
};

const DWORD c_Inventory_Page_Column = 5;
const DWORD c_Inventory_Page_Row = 9;
const DWORD c_Inventory_Page_Size = c_Inventory_Page_Column * c_Inventory_Page_Row; // x*y
const DWORD c_Inventory_Page_Count = 4;
const DWORD c_ItemSlot_Count = c_Inventory_Page_Size * c_Inventory_Page_Count;
const DWORD c_Equipment_Count = 12;

const DWORD c_Equipment_Start = c_ItemSlot_Count;

const DWORD c_Equipment_Body	= c_Equipment_Start + CItemData::WEAR_BODY;
const DWORD c_Equipment_Head	= c_Equipment_Start + CItemData::WEAR_HEAD;
const DWORD c_Equipment_Shoes	= c_Equipment_Start + CItemData::WEAR_FOOTS;
const DWORD c_Equipment_Wrist	= c_Equipment_Start + CItemData::WEAR_WRIST;
const DWORD c_Equipment_Weapon	= c_Equipment_Start + CItemData::WEAR_WEAPON;
const DWORD c_Equipment_Neck	= c_Equipment_Start + CItemData::WEAR_NECK;
const DWORD c_Equipment_Ear		= c_Equipment_Start + CItemData::WEAR_EAR;
const DWORD c_Equipment_Unique1	= c_Equipment_Start + CItemData::WEAR_UNIQUE1;
const DWORD c_Equipment_Unique2	= c_Equipment_Start + CItemData::WEAR_UNIQUE2;
const DWORD c_Equipment_Arrow	= c_Equipment_Start + CItemData::WEAR_ARROW;
const DWORD c_Equipment_Shield	= c_Equipment_Start + CItemData::WEAR_SHIELD;

//
#ifdef ENABLE_NEW_EQUIPMENT_SYSTEM
const DWORD c_New_Equipment_Start = c_Equipment_Start + CItemData::WEAR_RING1;
const DWORD c_New_Equipment_Count = 5;
const DWORD c_Equipment_Ring1 = c_Equipment_Start + CItemData::WEAR_RING1;
const DWORD c_Equipment_Ring2 = c_Equipment_Start + CItemData::WEAR_RING2;
const DWORD c_Equipment_Belt = c_Equipment_Start + CItemData::WEAR_BELT;
const DWORD c_Equipment_Pendant = c_Equipment_Start + CItemData::WEAR_PENDANT;
#ifdef ENABLE_EXTENDED_PET_ITEM
const DWORD c_Equipment_Pet = c_Equipment_Start + CItemData::WEAR_PET;
#endif
#endif

#ifdef ENABLE_SKILL_COLOR_SYSTEM
enum ESkillColorLength
{
	MAX_SKILL_COUNT = 6,
	MAX_EFFECT_COUNT = 5,
	BUFF_BEGIN = MAX_SKILL_COUNT,
#ifdef ENABLE_WOLFMAN_CHARACTER
	MAX_BUFF_COUNT = 6,
#else
	MAX_BUFF_COUNT = 5,
#endif
};
#endif

enum EDragonSoulDeckType
{
	DS_DECK_1,
	DS_DECK_2,
	DS_DECK_MAX_NUM = 2,
};

enum EDragonSoulGradeTypes
{
	DRAGON_SOUL_GRADE_NORMAL,
	DRAGON_SOUL_GRADE_BRILLIANT,
	DRAGON_SOUL_GRADE_RARE,
	DRAGON_SOUL_GRADE_ANCIENT,
	DRAGON_SOUL_GRADE_LEGENDARY,
#ifdef ENABLE_DS_GRADE_MYTH
	DRAGON_SOUL_GRADE_MYTH,
#endif
#ifdef ENABLE_DS_GRADE_EPIC
	DRAGON_SOUL_GRADE_EPIC,
#endif
#ifdef ENABLE_DS_GRADE_GODLIKE
	DRAGON_SOUL_GRADE_GODLIKE,
#endif
	DRAGON_SOUL_GRADE_MAX,
};

enum EDragonSoulStepTypes
{
	DRAGON_SOUL_STEP_LOWEST,
	DRAGON_SOUL_STEP_LOW,
	DRAGON_SOUL_STEP_MID,
	DRAGON_SOUL_STEP_HIGH,
	DRAGON_SOUL_STEP_HIGHEST,
#ifdef ENABLE_NEW_DRAGON_SOUL_SINIF
	DRAGON_SOUL_STEP_NEW1,
	DRAGON_SOUL_STEP_NEW2,
	DRAGON_SOUL_STEP_NEW3,
	DRAGON_SOUL_STEP_NEW4,
	DRAGON_SOUL_STEP_NEW5,
#endif
	DRAGON_SOUL_STEP_MAX,
};

#ifdef ENABLE_COSTUME_SYSTEM
const DWORD c_Costume_Slot_Start = c_Equipment_Start + CItemData::WEAR_COSTUME_BODY;
const DWORD	c_Costume_Slot_Body = c_Costume_Slot_Start + CItemData::COSTUME_BODY;
const DWORD	c_Costume_Slot_Hair = c_Costume_Slot_Start + CItemData::COSTUME_HAIR;
const DWORD	c_Costume_Slot_Mount = c_Costume_Slot_Start + CItemData::COSTUME_MOUNT;
const DWORD	c_Costume_Slot_Acce = c_Costume_Slot_Start + CItemData::COSTUME_ACCE;
const DWORD c_Costume_Slot_Weapon = c_Costume_Slot_Start + CItemData::COSTUME_WEAPON;
#ifdef ENABLE_AURA_COSTUME_SYSTEM
const DWORD c_Costume_Slot_Aura = c_Costume_Slot_Start + CItemData::COSTUME_AURA;
#endif
#ifdef ENABLE_AURA_COSTUME_SYSTEM
const DWORD c_Costume_Slot_Count = 6;
#else
const DWORD c_Costume_Slot_Count = 5;
#endif
const DWORD c_Costume_Slot_End = c_Costume_Slot_Start + c_Costume_Slot_Count;
#endif

#ifdef ENABLE_SHINING_ITEM_SYSTEM
const DWORD c_Shining_Slot_Start = c_Equipment_Start + CItemData::WEAR_SHINING_WEAPON;
const DWORD c_Shining_Slot_Count = 6;
#endif

#ifdef ENABLE_CAKRA_ITEM_SYSTEM
const DWORD c_Cakra_Slot_Start = c_Equipment_Start + CItemData::WEAR_CAKRA_1;
const DWORD c_Cakra_Slot_Count = 8;
#endif

#ifdef ENABLE_SEBNEM_ITEM_SYSTEM
const DWORD c_Sebnem_Slot_Start = c_Equipment_Start + CItemData::WEAR_SEBNEM_1;
const DWORD c_Sebnem_Slot_Count = 8;
#endif

const DWORD c_Wear_Max = CItemData::WEAR_MAX_NUM;
const DWORD c_DragonSoul_Equip_Start = c_ItemSlot_Count + CItemData::WEAR_MAX_NUM;
const DWORD c_DragonSoul_Equip_Slot_Max = 6;
const DWORD c_DragonSoul_Equip_End = c_DragonSoul_Equip_Start + c_DragonSoul_Equip_Slot_Max * DS_DECK_MAX_NUM;

const DWORD c_DragonSoul_Equip_Reserved_Count = c_DragonSoul_Equip_Slot_Max * 3;

#ifdef ENABLE_NEW_EQUIPMENT_SYSTEM
const DWORD c_Belt_Inventory_Slot_Start = c_DragonSoul_Equip_End + c_DragonSoul_Equip_Reserved_Count;
const DWORD c_Belt_Inventory_Width = 4;
const DWORD c_Belt_Inventory_Height = 4;
const DWORD c_Belt_Inventory_Slot_Count = c_Belt_Inventory_Width * c_Belt_Inventory_Height;
const DWORD c_Belt_Inventory_Slot_End = c_Belt_Inventory_Slot_Start + c_Belt_Inventory_Slot_Count;

const DWORD c_Inventory_Count = c_Belt_Inventory_Slot_End;
#else
const DWORD c_Inventory_Count = c_DragonSoul_Equip_End;
#endif

const DWORD c_DragonSoul_Inventory_Start = 0;
const DWORD c_DragonSoul_Inventory_Box_Size = 32;
const DWORD c_DragonSoul_Inventory_Count = CItemData::DS_SLOT_NUM_TYPES * DRAGON_SOUL_GRADE_MAX * c_DragonSoul_Inventory_Box_Size;
const DWORD c_DragonSoul_Inventory_End = c_DragonSoul_Inventory_Start + c_DragonSoul_Inventory_Count;

#ifdef ENABLE_ADDITIONAL_INVENTORY
const DWORD c_Special_Inventory_Page_Size = 5 * 9;
const DWORD c_Special_Inventory_Page_Count = 5;
const DWORD c_Special_ItemSlot_Count = c_Special_Inventory_Page_Size * c_Special_Inventory_Page_Count;
#endif

enum ESlotType
{
	SLOT_TYPE_NONE,
	SLOT_TYPE_INVENTORY,
	SLOT_TYPE_SKILL,
	SLOT_TYPE_EMOTION,
	SLOT_TYPE_SHOP,
	SLOT_TYPE_EXCHANGE_OWNER,
	SLOT_TYPE_EXCHANGE_TARGET,
	SLOT_TYPE_QUICK_SLOT,
	SLOT_TYPE_SAFEBOX,
	SLOT_TYPE_PRIVATE_SHOP,
	SLOT_TYPE_MALL,
	SLOT_TYPE_DRAGON_SOUL_INVENTORY,
#ifdef ENABLE_ADDITIONAL_INVENTORY
	SLOT_TYPE_UPGRADE_INVENTORY,
	SLOT_TYPE_BOOK_INVENTORY,
	SLOT_TYPE_STONE_INVENTORY,
	SLOT_TYPE_FLOWER_INVENTORY,
	SLOT_TYPE_ATTR_INVENTORY,
	SLOT_TYPE_CHEST_INVENTORY,
#endif
#ifdef ENABLE_SWITCHBOT_SYSTEM
	SLOT_TYPE_SWITCHBOT,
#endif
	SLOT_TYPE_MAX,
};

enum EWindows
{
	RESERVED_WINDOW,
	INVENTORY,
	EQUIPMENT,
	SAFEBOX,
	MALL,
	DRAGON_SOUL_INVENTORY,
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
	BELT_INVENTORY,
	GROUND,
	WINDOW_TYPE_MAX,
};

enum ESpecialWindows
{
	SPECIAL_WINDOW_START = 100,
#ifdef ENABLE_OFFLINE_SHOP
	SPECIAL_WINDOW_OFFLINE_SHOP,
#endif
	SPECIAL_WINDOW_TYPE_MAX,
};

#ifdef ENABLE_SWITCHBOT_SYSTEM
enum ESwitchbotValues
{
	SWITCHBOT_SLOT_COUNT = 5,
	SWITCHBOT_ALTERNATIVE_COUNT = 2,
	MAX_NORM_ATTR_NUM = 5,
};

enum EAttributeSet
{
	ATTRIBUTE_SET_WEAPON,
	ATTRIBUTE_SET_BODY,
	ATTRIBUTE_SET_WRIST,
	ATTRIBUTE_SET_FOOTS,
	ATTRIBUTE_SET_NECK,
	ATTRIBUTE_SET_HEAD,
	ATTRIBUTE_SET_SHIELD,
	ATTRIBUTE_SET_EAR,
#ifdef ENABLE_COSTUME_SWITCH_ITEM
	ATTRIBUTE_SET_COSTUME_BODY,
	ATTRIBUTE_SET_COSTUME_HAIR,
#if defined(ENABLE_COSTUME_SWITCH_ITEM) && defined(ENABLE_WEAPON_COSTUME_SYSTEM)
	ATTRIBUTE_SET_COSTUME_WEAPON,
#endif
#endif
#ifdef ENABLE_PENDANT_SYSTEM
	ATTRIBUTE_SET_PENDANT,
#endif // ENABLE_PENDANT_SYSTEM
#ifdef ENABLE_AURA_COSTUME_SYSTEM
	ATTRIBUTE_SET_COSTUME_AURA,
#endif // ENABLE_AURA_COSTUME_SYSTEM
	ATTRIBUTE_SET_MAX_NUM,
};
#endif

enum EDSInventoryMaxNum
{
	DS_INVENTORY_MAX_NUM = c_DragonSoul_Inventory_Count,
	DS_REFINE_WINDOW_MAX_NUM = 15,
};

#pragma pack (push, 1)
#define WORD_MAX 0xffff

typedef struct SItemPos
{
	BYTE window_type;
	WORD cell;
	SItemPos()
	{
		window_type = INVENTORY;
		cell = WORD_MAX;
	}
	SItemPos(BYTE _window_type, WORD _cell)
	{
		window_type = _window_type;
		cell = _cell;
	}

	//  int operator=(const int _cell)
	//  {
		  //window_type = INVENTORY;
	//      cell = _cell;
	//      return cell;
	//  }
	bool IsValidCell()
	{
		switch (window_type)
		{
		case INVENTORY:
			return cell < c_Inventory_Count;
			break;
		case EQUIPMENT:
			return cell < c_DragonSoul_Equip_End;
			break;
		case DRAGON_SOUL_INVENTORY:
			return cell < (DS_INVENTORY_MAX_NUM);
			break;
#ifdef ENABLE_ADDITIONAL_INVENTORY
		case UPGRADE_INVENTORY:
			return cell < c_Special_ItemSlot_Count;
			break;
		case BOOK_INVENTORY:
			return cell < c_Special_ItemSlot_Count;
			break;
		case STONE_INVENTORY:
			return cell < c_Special_ItemSlot_Count;
			break;
		case FLOWER_INVENTORY:
			return cell < c_Special_ItemSlot_Count;
			break;
		case ATTR_INVENTORY:
			return cell < c_Special_ItemSlot_Count;
			break;
		case CHEST_INVENTORY:
			return cell < c_Special_ItemSlot_Count;
			break;
#endif // ENABLE_ADDITIONAL_INVENTORY
#ifdef ENABLE_SWITCHBOT_SYSTEM
		case SWITCHBOT:
			return cell < SWITCHBOT_SLOT_COUNT;
			break;
#endif // ENABLE_SWITCHBOT_SYSTEM
		default:
			return false;
		}
	}
	bool IsEquipCell()
	{
		switch (window_type)
		{
		case INVENTORY:
		case EQUIPMENT:
			return (c_Equipment_Start + c_Wear_Max > cell) && (c_Equipment_Start <= cell);
			break;

		case BELT_INVENTORY:
		case DRAGON_SOUL_INVENTORY:
#ifdef ENABLE_ADDITIONAL_INVENTORY
		case UPGRADE_INVENTORY:
		case BOOK_INVENTORY:
		case STONE_INVENTORY:
		case FLOWER_INVENTORY:
		case ATTR_INVENTORY:
		case CHEST_INVENTORY:
#endif
			return false;
			break;

		default:
			return false;
		}
	}

#ifdef ENABLE_NEW_EQUIPMENT_SYSTEM
	bool IsBeltInventoryCell()
	{
		bool bResult = c_Belt_Inventory_Slot_Start <= cell && c_Belt_Inventory_Slot_End > cell;
		return bResult;
	}
#endif

	bool operator==(const struct SItemPos& rhs) const
	{
		return (window_type == rhs.window_type) && (cell == rhs.cell);
	}

	bool operator<(const struct SItemPos& rhs) const
	{
		return (window_type < rhs.window_type) || ((window_type == rhs.window_type) && (cell < rhs.cell));
	}
} TItemPos;
#pragma pack(pop)

enum EItemSocket_Attribute
{
#ifdef ENABLE_EXTRA_SOCKET_SYSTEM
	ITEM_ACCESSORY_SOCKET_MAX_NUM = 20,
#else
	ITEM_ACCESSORY_SOCKET_MAX_NUM = 3,
#endif
#ifdef ENABLE_EXTRA_SOCKET_SYSTEM
	ITEM_SOCKET_SLOT_MAX_NUM = 4,
#else
	ITEM_SOCKET_SLOT_MAX_NUM = 3,
#endif
	// refactored attribute slot begin
	ITEM_ATTRIBUTE_SLOT_NORM_NUM = 5,
	ITEM_ATTRIBUTE_SLOT_RARE_NUM = 2,

	ITEM_ATTRIBUTE_SLOT_NORM_START = 0,
	ITEM_ATTRIBUTE_SLOT_NORM_END = ITEM_ATTRIBUTE_SLOT_NORM_START + ITEM_ATTRIBUTE_SLOT_NORM_NUM,

	ITEM_ATTRIBUTE_SLOT_RARE_START = ITEM_ATTRIBUTE_SLOT_NORM_END,
	ITEM_ATTRIBUTE_SLOT_RARE_END = ITEM_ATTRIBUTE_SLOT_RARE_START + ITEM_ATTRIBUTE_SLOT_RARE_NUM,
	ITEM_ATTRIBUTE_SLOT_MAX_NUM = 7, // 7
	// refactored attribute slot end
};

#pragma pack(push)
#pragma pack(1)

typedef struct SQuickSlot
{
	BYTE Type;
	BYTE Position;
} TQuickSlot;

typedef struct TPlayerItemAttribute
{
	BYTE        bType;
	short       sValue;
} TPlayerItemAttribute;

typedef struct packet_item
{
	DWORD       vnum;
#ifdef ENABLE_ITEM_COUNT_LIMIT
	DWORD		count;
#else
	BYTE		count;
#endif
	DWORD		flags;
	DWORD		anti_flags;
	long		alSockets[ITEM_SOCKET_SLOT_MAX_NUM];
	TPlayerItemAttribute aAttr[ITEM_ATTRIBUTE_SLOT_MAX_NUM];
#ifdef ENABLE_BEGINNER_ITEM_SYSTEM
	bool		is_basic;
#endif
#ifdef ENABLE_ITEM_EVOLUTION_SYSTEM
	DWORD		evolution;
#endif
#ifdef ENABLE_CHANGELOOK_SYSTEM
	DWORD		transmutation;
#endif
} TItemData;

#ifdef ENABLE_GROWTH_PET_SYSTEM
enum
{
	PET_HATCHING_MONEY = 100000,
	PET_NAME_MIN_SIZE = 4,
	PET_NAME_MAX_SIZE = 12,
};
#endif

#ifdef ENABLE_GROWTH_MOUNT_SYSTEM
enum
{
	MOUNT_HATCHING_MONEY = 100000,
	MOUNT_NAME_MIN_SIZE = 4,
	MOUNT_NAME_MAX_SIZE = 12,
};
#endif

typedef struct packet_shop_item
{
	DWORD		vnum;
#ifdef ENABLE_GOLD_LIMIT_REWORK
	long long	price;
#else
	DWORD		price;
#endif
#ifdef ENABLE_CHEQUE_SYSTEM
	DWORD		cheque;
#endif
#ifdef ENABLE_ITEM_COUNT_LIMIT
	DWORD		count;
#else
	BYTE		count;
#endif
	BYTE		display_pos;
	long		alSockets[ITEM_SOCKET_SLOT_MAX_NUM];
	TPlayerItemAttribute aAttr[ITEM_ATTRIBUTE_SLOT_MAX_NUM];
#ifdef ENABLE_SHOP_PRICE_TYPE_ITEM
	DWORD		witemVnum;
#endif
#ifdef ENABLE_ITEM_EVOLUTION_SYSTEM
	DWORD		evolution;
#endif
#ifdef ENABLE_CHANGELOOK_SYSTEM
	DWORD		transmutation;
#endif
} TShopItemData;

#ifdef ENABLE_ITEM_EVOLUTION_SYSTEM
enum EEvolutionType
{
	WEAPON_EVOLUTION_TYPE_NONE,
	WEAPON_EVOLUTION_TYPE_FIRE,
	WEAPON_EVOLUTION_TYPE_ICE,
	WEAPON_EVOLUTION_TYPE_EARTH,
	WEAPON_EVOLUTION_TYPE_WIND,
	WEAPON_EVOLUTION_TYPE_DARK,
	WEAPON_EVOLUTION_TYPE_ELEC,
	WEAPON_EVOLUTION_TYPE_ALL,
	WEAPON_EVOLUTION_TYPE_HUMAN,
	WEAPON_EVOLUTION_TYPE_EX_FIRE,
	WEAPON_EVOLUTION_TYPE_EX_ICE,
	WEAPON_EVOLUTION_TYPE_EX_EARTH,
	WEAPON_EVOLUTION_TYPE_EX_WIND,
	WEAPON_EVOLUTION_TYPE_EX_DARK,
	WEAPON_EVOLUTION_TYPE_EX_ELEC,
	WEAPON_EVOLUTION_TYPE_EX_ALL,
};
#endif

#ifdef ENABLE_BATTLE_PASS_SYSTEM
typedef struct SBattlePassRewardItem
{
	DWORD dwVnum;
	BYTE bCount;
} TBattlePassRewardItem;

typedef struct SBattlePassMissionInfo
{
	BYTE bMissionType;
	DWORD dwMissionInfo[3];
	TBattlePassRewardItem aRewardList[3];
	DWORD dwSkipCost;
} TBattlePassMissionInfo;

typedef struct SBattlePassRanking
{
	BYTE bPos;
	char playerName[64 + 1];
	DWORD dwFinishTime;
} TBattlePassRanking;
#endif


#pragma pack(pop)

// DEFAULT_FONT
void DefaultFont_Startup();
void DefaultFont_Cleanup();
void DefaultFont_SetName(const char* c_szFontName);
CResource* DefaultFont_GetResource();
CResource* DefaultItalicFont_GetResource();
// END_OF_DEFAULT_FONT

BYTE SlotTypeToInvenType(BYTE bSlotType);
#ifdef ENABLE_DETAILS_INTERFACE
BYTE ApplyTypeToPointType(BYTE bApplyType);
BYTE PointTypeToApplyType(BYTE bPointType);
#endif