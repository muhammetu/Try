#include "stdafx.h"
#include "constants.h"
#include "log.h"
#include "item.h"
#include "char.h"
#include "desc.h"
#include "item_manager.h"
#include "config.h"

#ifndef __SWITCHBOT__
const int MAX_NORM_ATTR_NUM = ITEM_MANAGER::MAX_NORM_ATTR_NUM;
const int MAX_RARE_ATTR_NUM = ITEM_MANAGER::MAX_RARE_ATTR_NUM;
#endif

int CItem::GetAttributeSetIndex()
{
	if (GetType() == ITEM_WEAPON)
	{
		if (GetSubType() == WEAPON_ARROW)
			return -1;
#ifdef __QUIVER_SYSTEM__
		if (GetSubType() == WEAPON_QUIVER)
			return -1;
#endif
		return ATTRIBUTE_SET_WEAPON;
	}

	if (GetType() == ITEM_ARMOR)
	{
		switch (GetSubType())
		{
		case ARMOR_BODY:
			return ATTRIBUTE_SET_BODY;

		case ARMOR_WRIST:
			return ATTRIBUTE_SET_WRIST;

		case ARMOR_FOOTS:
			return ATTRIBUTE_SET_FOOTS;

		case ARMOR_NECK:
			return ATTRIBUTE_SET_NECK;

		case ARMOR_HEAD:
			return ATTRIBUTE_SET_HEAD;

		case ARMOR_SHIELD:
			return ATTRIBUTE_SET_SHIELD;

		case ARMOR_EAR:
			return ATTRIBUTE_SET_EAR;

#ifdef __PENDANT_SYSTEM__
		case ARMOR_PENDANT:
			return ATTRIBUTE_SET_PENDANT;
#endif
		}
	}
	else if (GetType() == ITEM_COSTUME)
	{
		switch (GetSubType())
		{
		case COSTUME_BODY:
#ifdef __ITEM_ATTR_COSTUME__
			return ATTRIBUTE_SET_COSTUME_BODY;
#else
			return ATTRIBUTE_SET_BODY;
#endif

		case COSTUME_HAIR:
#ifdef __ITEM_ATTR_COSTUME__
			return ATTRIBUTE_SET_COSTUME_HAIR;
#else
			return ATTRIBUTE_SET_HEAD;
#endif

#ifdef __MOUNT_COSTUME_SYSTEM__
		case COSTUME_MOUNT:
			return ATTRIBUTE_SET_COSTUME_MOUNT;
#endif

#ifdef __WEAPON_COSTUME_SYSTEM__
		case COSTUME_WEAPON:
#ifdef __ITEM_ATTR_COSTUME__
			return ATTRIBUTE_SET_COSTUME_WEAPON;
#else
			return ATTRIBUTE_SET_WEAPON;
#endif
#endif

#ifdef __AURA_COSTUME_SYSTEM__
		case COSTUME_AURA:
			return ATTRIBUTE_SET_COSTUME_AURA;
#endif
		}
	}
#ifdef __MARTY_BELT_INVENTORY__
	else if (GetType() == ITEM_BELT)
		return ATTRIBUTE_SET_BELT;
#endif
#ifdef __SHINING_ITEM_SYSTEM__
	else if (GetType() == ITEM_SHINING)
	{
		switch (GetSubType())
		{
		case SHINING_WING:
			return ATTRIBUTE_SET_ACCE_SHINING;
		case SHINING_SPECIAL3:
			return ATTRIBUTE_SET_SOUL_SHINING;
		}
	}
#endif
#ifdef __CAKRA_ITEM_SYSTEM__
	else if (GetType() == ITEM_CAKRA)
	{
		return ATTRIBUTE_SET_CAKRA;
	}
#endif
#ifdef __PET_SYSTEM_PROTO__
	else if (GetType() == ITEM_UNIQUE)
	{
		switch (GetSubType())
		{
			case USE_PET:
				return ATTRIBUTE_SET_PET;
		}
	}
#endif
#ifdef __SEBNEM_ITEM_SYSTEM__
	else if (GetType() == ITEM_SEBNEM)
	{
		return ATTRIBUTE_SET_SEBNEM;
	}
#endif
	return -1;
}

#ifdef __MULTIPLE_MONSTER_ATTR__
bool CItem::HasAttrEx(BYTE alreadyType, BYTE addedType)
{
	if (GetType() == ITEM_COSTUME)
	{
		if (GetSubType() == COSTUME_HAIR)
		{
			if (addedType == APPLY_ATTBONUS_MONSTER)
				return false;
			if (addedType == APPLY_ATTBONUS_BOSS)
				return false;
			if (addedType == APPLY_ATTBONUS_STONE)
				return false;
			if (addedType == APPLY_MALL_ATTBONUS)
				return false;
			if (addedType == APPLY_ATT_GRADE_BONUS)
				return false;
		}
		else if (GetSubType() == COSTUME_MOUNT)
		{
			if (addedType == APPLY_ATTBONUS_MONSTER)
				return false;
			if (addedType == APPLY_ATTBONUS_BOSS)
				return false;
			if (addedType == APPLY_ATTBONUS_STONE)
				return false;
			if (addedType == APPLY_MALL_ATTBONUS)
				return false;
			if (addedType == APPLY_ATT_GRADE_BONUS)
				return false;
		}
		else if (GetSubType() == COSTUME_BODY)
		{
			if (addedType == APPLY_ATTBONUS_MONSTER)
				return false;
			if (addedType == APPLY_ATTBONUS_BOSS)
				return false;
			if (addedType == APPLY_ATTBONUS_STONE)
				return false;
			if (addedType == APPLY_MALL_ATTBONUS)
				return false;
			if (addedType == APPLY_ATT_GRADE_BONUS)
				return false;
		}
		else if (GetSubType() == COSTUME_ACCE)
		{
			if (addedType == APPLY_ATTBONUS_MONSTER)
				return false;
			if (addedType == APPLY_ATTBONUS_BOSS)
				return false;
			if (addedType == APPLY_ATTBONUS_STONE)
				return false;
			if (addedType == APPLY_MALL_ATTBONUS)
				return false;
			if (addedType == APPLY_ATT_GRADE_BONUS)
				return false;
		}
		else if (GetSubType() == COSTUME_AURA)
		{
			if (addedType == APPLY_ATTBONUS_MONSTER)
				return false;
			if (addedType == APPLY_ATTBONUS_BOSS)
				return false;
			if (addedType == APPLY_ATTBONUS_STONE)
				return false;
			if (addedType == APPLY_NORMAL_HIT_DAMAGE_BONUS)
				return false;
			if (addedType == APPLY_MALL_ATTBONUS)
				return false;
			if (addedType == APPLY_ATT_GRADE_BONUS)
				return false;
		}
		else if (GetSubType() == COSTUME_WEAPON)
		{
			if (addedType == APPLY_ATTBONUS_MONSTER)
				return false;
			if (addedType == APPLY_ATTBONUS_BOSS)
				return false;
			if (addedType == APPLY_ATTBONUS_STONE)
				return false;
			if (addedType == APPLY_NORMAL_HIT_DAMAGE_BONUS)
				return false;
			if (addedType == APPLY_MALL_ATTBONUS)
				return false;
			if (addedType == APPLY_ATT_GRADE_BONUS)
				return false;
		}
	}
	else if (GetType() == ITEM_BELT)
	{
		if (addedType == APPLY_ATTBONUS_MONSTER)
			return false;
		if (addedType == APPLY_ATTBONUS_BOSS)
			return false;
		if (addedType == APPLY_ATTBONUS_STONE)
			return false;
		if (addedType == APPLY_MALL_ATTBONUS)
			return false;
		if (addedType == APPLY_ATT_GRADE_BONUS)
			return false;
	}
	else if (GetType() == ITEM_CAKRA)
	{
		if (addedType == APPLY_ATTBONUS_MONSTER)
			return false;
		if (addedType == APPLY_ATTBONUS_BOSS)
			return false;
		if (addedType == APPLY_ATTBONUS_STONE)
			return false;
		if (addedType == APPLY_NORMAL_HIT_DAMAGE_BONUS)
			return false;
		if (addedType == APPLY_MALL_ATTBONUS)
			return false;
		if (addedType == APPLY_ATT_GRADE_BONUS)
			return false;
	}
	else if (GetType() == ITEM_ARMOR)
	{
		if (GetSubType() == ARMOR_PENDANT)
		{
			if (addedType == APPLY_ATTBONUS_MONSTER)
				return false;
			if (addedType == APPLY_ATTBONUS_BOSS)
				return false;
			if (addedType == APPLY_ATTBONUS_STONE)
				return false;
			if (addedType == APPLY_MALL_ATTBONUS)
				return false;
			if (addedType == APPLY_ATT_GRADE_BONUS)
				return false;
		}
	}
	else if (GetType() == ITEM_SHINING)
	{
		if (GetSubType() == SHINING_WING || GetSubType() == SHINING_SPECIAL3)
		{
			if (addedType == APPLY_ATTBONUS_MONSTER)
				return false;
			if (addedType == APPLY_ATTBONUS_BOSS)
				return false;
			if (addedType == APPLY_ATTBONUS_STONE)
				return false;
			if (addedType == APPLY_MALL_ATTBONUS)
				return false;
			if (addedType == APPLY_ATT_GRADE_BONUS)
				return false;
		}
	}
	else if (GetType() == ITEM_UNIQUE)
	{
		if (GetSubType() == USE_PET)
		{
			if (addedType == APPLY_ATTBONUS_MONSTER)
				return false;
			if (addedType == APPLY_ATTBONUS_BOSS)
				return false;
			if (addedType == APPLY_ATTBONUS_STONE)
				return false;
			if (addedType == APPLY_NORMAL_HIT_DAMAGE_BONUS)
				return false;
			if (addedType == APPLY_MALL_ATTBONUS)
				return false;
			if (addedType == APPLY_ATT_GRADE_BONUS)
				return false;
		}
	}
	else if (GetType() == ITEM_SEBNEM)
	{
		if (addedType == APPLY_ATTBONUS_MONSTER)
			return false;
		if (addedType == APPLY_ATTBONUS_BOSS)
			return false;
		if (addedType == APPLY_ATTBONUS_STONE)
			return false;
		if (addedType == APPLY_NORMAL_HIT_DAMAGE_BONUS)
			return false;
		if (addedType == APPLY_MALL_ATTBONUS)
			return false;
		if (addedType == APPLY_ATT_GRADE_BONUS)
			return false;
	}
	return alreadyType == addedType;
}
#endif

bool CItem::HasAttr(BYTE bApply)
{
	for (int i = 0; i < ITEM_APPLY_MAX_NUM; ++i)
#ifdef __MULTIPLE_MONSTER_ATTR__
		if (HasAttrEx(m_pProto->aApplies[i].bType, bApply))
#else
		if (m_pProto->aApplies[i].bType == bApply)
#endif
			return true;

	for (int i = 0; i < MAX_NORM_ATTR_NUM; ++i)
		if (GetAttributeType(i) == bApply)
			return true;

	return false;
}

bool CItem::HasRareAttr(BYTE bApply)
{
	for (int i = 0; i < MAX_RARE_ATTR_NUM; ++i)
		if (GetAttributeType(i + 5) == bApply)
			return true;

	return false;
}

void CItem::AddAttribute(BYTE bApply, short sValue)
{
	if (HasAttr(bApply))
		return;

	int i = GetAttributeCount();

	if (i >= MAX_NORM_ATTR_NUM)
		sys_err("item attribute overflow!");
	else
	{
		if (sValue)
			SetAttribute(i, bApply, sValue);
	}
}

void CItem::AddAttr(BYTE bApply, BYTE bLevel)
{
	if (HasAttr(bApply))
		return;

	if (bLevel <= 0)
		return;

	int i = GetAttributeCount();

	if (i == MAX_NORM_ATTR_NUM)
		sys_err("item attribute overflow!");
	else
	{
		const TItemAttrTable& r = g_map_itemAttr[bApply];
		long lVal = r.lValues[MIN(4, bLevel - 1)];

		if (lVal)
			SetAttribute(i, bApply, lVal);
	}
}

void CItem::PutAttributeWithLevel(BYTE bLevel)
{
	int iAttributeSet = GetAttributeSetIndex();
	if (iAttributeSet < 0)
		return;

	if (bLevel > ITEM_ATTRIBUTE_MAX_LEVEL)
		return;

	std::vector<int> avail;

	int total = 0;

	for (int i = 0; i < MAX_APPLY_NUM; ++i)
	{
		const TItemAttrTable& r = g_map_itemAttr[i];

		if (r.bMaxLevelBySet[iAttributeSet] && !HasAttr(i))
		{
			avail.push_back(i);
			total += r.dwProb;
		}
	}

	unsigned int prob = number(1, total);
	int attr_idx = APPLY_NONE;

	for (DWORD i = 0; i < avail.size(); ++i)
	{
		const TItemAttrTable& r = g_map_itemAttr[avail[i]];

		if (prob <= r.dwProb)
		{
			attr_idx = avail[i];
			break;
		}

		prob -= r.dwProb;
	}

	if (!attr_idx)
	{
		sys_err("Cannot put item attribute %d %d", iAttributeSet, bLevel);
		return;
	}

	const TItemAttrTable& r = g_map_itemAttr[attr_idx];

	if (bLevel > r.bMaxLevelBySet[iAttributeSet])
		bLevel = r.bMaxLevelBySet[iAttributeSet];

	AddAttr(attr_idx, bLevel);
}

void CItem::PutAttribute(const int* aiAttrPercentTable)
{
	int iAttrLevelPercent = number(1, 100);
	int i;

	for (i = 0; i < ITEM_ATTRIBUTE_MAX_LEVEL; ++i)
	{
		if (iAttrLevelPercent <= aiAttrPercentTable[i])
			break;

		iAttrLevelPercent -= aiAttrPercentTable[i];
	}

	PutAttributeWithLevel(i + 1);
}

void CItem::ChangeAttribute(const int* aiChangeProb)
{
	int iAttributeCount = GetAttributeCount();

	ClearAttribute();

	if (iAttributeCount == 0)
		return;

	TItemTable const* pProto = GetProto();

	if (pProto && pProto->sAddonType)
	{
		ApplyAddon(pProto->sAddonType);
	}

	static const int tmpChangeProb[ITEM_ATTRIBUTE_MAX_LEVEL] =
	{
		0, 10, 40, 35, 15,
	};

	for (int i = GetAttributeCount(); i < iAttributeCount; ++i)
	{
		if (aiChangeProb == NULL)
		{
			PutAttribute(tmpChangeProb);
		}
		else
		{
			PutAttribute(aiChangeProb);
		}
	}
}

void CItem::AddAttribute()
{
	static const int aiItemAddAttributePercent[ITEM_ATTRIBUTE_MAX_LEVEL] =
	{
		5, 10, 40, 35, 15,
	};

	if (GetAttributeCount() < MAX_NORM_ATTR_NUM)	
#ifdef __COSTUME_ATTR_SWITCH__
	{
		bool switchEx = (GetType() == ITEM_COSTUME || GetType() == ITEM_BELT || GetType() == ITEM_CAKRA || GetType() == ITEM_SHINING || (GetType() == ITEM_ARMOR && GetSubType() == ARMOR_PENDANT) || (GetType() == ITEM_UNIQUE && GetSubType() == USE_PET));
		PutAttribute(switchEx == true ? aiCostumeAttributeLevelPercent : aiItemAddAttributePercent);
	}
#else
		PutAttribute(aiItemAddAttributePercent);
#endif
}

#ifdef __ATTR_ADDON_ITEMS__
void CItem::AddAttributeEx()
{
	static const int addon_struct[ITEM_ATTRIBUTE_MAX_LEVEL] =
	{
		0, 0, 0, 0, 100,
	};

	PutAttribute(addon_struct);
}
#endif

#ifdef __ANCIENT_ATTR_ITEM__
void CItem::ClearAttribute(bool clearMode)
#else
void CItem::ClearAttribute()
#endif
{
#ifdef __ANCIENT_ATTR_ITEM__
	int iCount = (clearMode ? MAX_NORM_ATTR_NUM : 4);
	for (int i = 0; i < iCount; ++i)
	{
		m_aAttr[i].bType = 0;
		m_aAttr[i].sValue = 0;
	}
#else
	for (int i = 0; i < MAX_NORM_ATTR_NUM; ++i)
	{
		m_aAttr[i].bType = 0;
		m_aAttr[i].sValue = 0;
	}
#endif
}

int CItem::GetAttributeCount()
{
	int i;

	for (i = 0; i < MAX_NORM_ATTR_NUM; ++i)
	{
		if (GetAttributeType(i) == 0)
			break;
	}

	return i;
}

int CItem::FindAttribute(BYTE bType)
{
	for (int i = 0; i < MAX_NORM_ATTR_NUM; ++i)
	{
		if (GetAttributeType(i) == bType)
			return i;
	}

	return -1;
}

bool CItem::RemoveAttributeAt(int index)
{
	if (GetAttributeCount() <= index)
		return false;

	for (int i = index; i < MAX_NORM_ATTR_NUM - 1; ++i)
	{
		SetAttribute(i, GetAttributeType(i + 1), GetAttributeValue(i + 1));
	}

	SetAttribute(MAX_NORM_ATTR_NUM - 1, APPLY_NONE, 0);
	return true;
}

bool CItem::RemoveAttributeType(BYTE bType)
{
	int index = FindAttribute(bType);
	return index != -1 && RemoveAttributeType(index);
}

void CItem::SetAttributes(const TPlayerItemAttribute* c_pAttribute)
{
	thecore_memcpy(m_aAttr, c_pAttribute, sizeof(m_aAttr));
	Save();
}

void CItem::SetAttribute(int i, BYTE bType, short sValue)
{
	assert(i < MAX_NORM_ATTR_NUM);

	m_aAttr[i].bType = bType;
	m_aAttr[i].sValue = sValue;
	UpdatePacket();
	Save();
}

void CItem::SetForceAttribute(int i, BYTE bType, short sValue)
{
	assert(i < ITEM_ATTRIBUTE_MAX_NUM);

	m_aAttr[i].bType = bType;
	m_aAttr[i].sValue = sValue;
	UpdatePacket();
	Save();
}

void CItem::CopyAttributeTo(LPITEM pItem)
{
	pItem->SetAttributes(m_aAttr);
}

int CItem::GetRareAttrCount()
{
	int ret = 0;

	for (DWORD dwIdx = ITEM_ATTRIBUTE_RARE_START; dwIdx < ITEM_ATTRIBUTE_RARE_END; dwIdx++)
	{
		if (m_aAttr[dwIdx].bType != 0)
			ret++;
	}

	return ret;
}

bool CItem::ChangeRareAttribute()
{
	if (GetRareAttrCount() == 0)
		return false;

	int cnt = GetRareAttrCount();

	for (int i = 0; i < cnt; ++i)
	{
		m_aAttr[i + ITEM_ATTRIBUTE_RARE_START].bType = 0;
		m_aAttr[i + ITEM_ATTRIBUTE_RARE_START].sValue = 0;
	}

	for (int i = 0; i < cnt; ++i)
	{
		AddRareAttribute();
	}

	return true;
}

bool CItem::AddRareAttribute()
{
	int count = GetRareAttrCount();

	if (count >= ITEM_ATTRIBUTE_RARE_NUM)
		return false;

	int pos = count + ITEM_ATTRIBUTE_RARE_START;
	TPlayerItemAttribute& attr = m_aAttr[pos];

	int nAttrSet = GetAttributeSetIndex();
	std::vector<int> avail;

	for (int i = 0; i < MAX_APPLY_NUM; ++i)
	{
		const TItemAttrTable& r = g_map_itemRare[i];

		if (r.dwApplyIndex != 0 && r.bMaxLevelBySet[nAttrSet] > 0 && HasRareAttr(i) != true)
		{
			avail.push_back(i);
		}
	}

	if (avail.size() == 0) // @duzenleme eger item_attr_rare tablosu bos ise core veriyordu.
		return false;

	const TItemAttrTable& r = g_map_itemRare[avail[number(0, avail.size() - 1)]];
	int nAttrLevel = 5;

	if (nAttrLevel > r.bMaxLevelBySet[nAttrSet])
		nAttrLevel = r.bMaxLevelBySet[nAttrSet];

	attr.bType = r.dwApplyIndex;
	attr.sValue = r.lValues[nAttrLevel - 1];

	UpdatePacket();

	Save();

	return true;
}

void CItem::AddRareAttribute2(const int* aiAttrPercentTable)
{
	static const int aiItemAddAttributePercent[ITEM_ATTRIBUTE_MAX_LEVEL] =
	{
		40, 50, 10, 0, 0
	};
	if (aiAttrPercentTable == NULL)
		aiAttrPercentTable = aiItemAddAttributePercent;

	if (GetRareAttrCount() < MAX_RARE_ATTR_NUM)
		PutRareAttribute(aiAttrPercentTable);
}

void CItem::PutRareAttribute(const int* aiAttrPercentTable)
{
	int iAttrLevelPercent = number(1, 100);
	int i;

	for (i = 0; i < ITEM_ATTRIBUTE_MAX_LEVEL; ++i)
	{
		if (iAttrLevelPercent <= aiAttrPercentTable[i])
			break;

		iAttrLevelPercent -= aiAttrPercentTable[i];
	}

	PutRareAttributeWithLevel(i + 1);
}

void CItem::PutRareAttributeWithLevel(BYTE bLevel)
{
	int iAttributeSet = GetAttributeSetIndex();
	if (iAttributeSet < 0)
		return;

	if (bLevel > ITEM_ATTRIBUTE_MAX_LEVEL)
		return;

	std::vector<int> avail;

	int total = 0;

	for (int i = 0; i < MAX_APPLY_NUM; ++i)
	{
		const TItemAttrTable& r = g_map_itemRare[i];

		if (r.bMaxLevelBySet[iAttributeSet] && !HasRareAttr(i))
		{
			avail.push_back(i);
			total += r.dwProb;
		}
	}

	unsigned int prob = number(1, total);
	int attr_idx = APPLY_NONE;

	for (DWORD i = 0; i < avail.size(); ++i)
	{
		const TItemAttrTable& r = g_map_itemRare[avail[i]];

		if (prob <= r.dwProb)
		{
			attr_idx = avail[i];
			break;
		}

		prob -= r.dwProb;
	}

	if (!attr_idx)
	{
		sys_err("Cannot put item rare attribute %d %d", iAttributeSet, bLevel);
		return;
	}

	const TItemAttrTable& r = g_map_itemRare[attr_idx];

	if (bLevel > r.bMaxLevelBySet[iAttributeSet])
		bLevel = r.bMaxLevelBySet[iAttributeSet];

	AddRareAttr(attr_idx, bLevel);
}

void CItem::AddRareAttr(BYTE bApply, BYTE bLevel)
{
	if (HasRareAttr(bApply))
		return;

	if (bLevel <= 0)
		return;

	int i = ITEM_ATTRIBUTE_RARE_START + GetRareAttrCount();

	if (i == ITEM_ATTRIBUTE_RARE_END)
		sys_err("item rare attribute overflow!");
	else
	{
		const TItemAttrTable& r = g_map_itemRare[bApply];
		long lVal = r.lValues[MIN(4, bLevel - 1)];

		if (lVal)
			SetForceAttribute(i, bApply, lVal);
	}
}
#ifdef __ANCIENT_ATTR_ITEM__
int CItem::AddNewStyleAttribute(BYTE* bValues)
{
	int iAttributeSet = GetAttributeSetIndex();

	if (iAttributeSet < 0)
		return 1;

	// 1 = unknown attribute
	// 2 = not enough attribute size
	// 3 = success

	bool have_addon = false;

	TItemTable const* pProto = GetProto();

	if (pProto && pProto->sAddonType)
		have_addon = true;

	for (int apply = 0; apply < MAX_APPLY_NUM; ++apply)
	{
		const TItemAttrTable& r = g_map_itemAttr[apply];

		for (int i = 0; i < (have_addon ? 2 : 4); ++i)
		{
			if (!r.bMaxLevelBySet[iAttributeSet] && (apply == bValues[i]))
			{
				sys_err("Trying to add unknown attribute to item: bValue: %d", bValues[i]);
				return 1;
			}
		}
	}
	std::vector<BYTE> vec_bTypes;

	for (int i = 0; i < (have_addon ? 2 : 4); ++i)
		vec_bTypes.push_back(bValues[i]);

	std::sort(vec_bTypes.begin(), vec_bTypes.end());
	vec_bTypes.erase(std::unique(vec_bTypes.begin(), vec_bTypes.end()), vec_bTypes.end());

	if (have_addon && vec_bTypes.size() == 2)
	{
		ClearAttribute(false);

		ApplyAddon(pProto->sAddonType);

		AddAttr(vec_bTypes[0], number(4, 5));
		AddAttr(vec_bTypes[1], number(4, 5));

		sys_log(0, "CItem::AddNewStyleAttribute(vec_bTypes[0]: %d, vec_bTypes[1]: %d, vec_bTypes[2]: %d, vec_bTypes[3]: %d)", vec_bTypes[0], vec_bTypes[1], vec_bTypes[2], vec_bTypes[3]);

		return 3;
	}

	if (vec_bTypes.size() == 4)
	{
		ClearAttribute(false);

		for (itertype(vec_bTypes) it = vec_bTypes.begin(); it != vec_bTypes.end(); ++it)
		{
			AddAttr(*it, number(4, 5));
		}

		sys_log(0, "CItem::AddNewStyleAttribute(vec_bTypes[0]: %d, vec_bTypes[1]: %d, vec_bTypes[2]: %d, vec_bTypes[3]: %d)", vec_bTypes[0], vec_bTypes[1], vec_bTypes[2], vec_bTypes[3]);

		return 3;
	}
	else
	{
		sys_err("Trying to add not enough attribute to item, vecSize: %d", vec_bTypes.size());
		return 2;
	}

	return 1;
}

void CItem::ChangeAttribute5TH()
{
	if (GetAttributeCount() != 5) // - 1
		return;

	m_aAttr[4].bType = 0;
	m_aAttr[4].sValue = 0;

	UpdatePacket();
	Save();

	PutAttributeWithLevel(number(4, 5));
}
#endif