#include "stdafx.h"
#include "../eterPack/EterPackManager.h"
#include "pythonnonplayer.h"
#include "InstanceBase.h"
#include "PythonCharacterManager.h"

#ifdef ENABLE_WIKI_SYSTEM
#include <boost/algorithm/string.hpp>
DWORD CPythonNonPlayer::FindMobSearchBox(const char* mobName, DWORD index, bool reverse)
{
	std::string itemName2 = mobName;
	boost::algorithm::to_lower(itemName2);

	auto it = m_NonPlayerDataMap.find(index);

	if (reverse)
	{
		auto rev_it = TNonPlayerDataMap::reverse_iterator(it);
		if (rev_it == m_NonPlayerDataMap.rend())
			rev_it = m_NonPlayerDataMap.rbegin();
		// ++rev_it;
		for (; rev_it != m_NonPlayerDataMap.rend(); ++rev_it)
		{
			auto itemData = rev_it->second.get();
			std::string itemName_ = itemData->szLocaleName;
			boost::algorithm::to_lower(itemName_);
			if (itemName_.rfind(itemName2, 0) == 0)
				return rev_it->first;
		}
	}
	else
	{
		if (it == m_NonPlayerDataMap.end())
			it = m_NonPlayerDataMap.begin();
		++it;
		for (; it != m_NonPlayerDataMap.end(); ++it)
		{
			auto itemData = it->second.get();
			std::string itemName_ = itemData->szLocaleName;
			boost::algorithm::to_lower(itemName_);
			if (itemName_.rfind(itemName2, 0) == 0)
				return it->first;
		}
	}

	return index;
}
#endif

bool CPythonNonPlayer::LoadNonPlayerData(const char* c_szFileName)
{
	static DWORD s_adwMobProtoKey[4] =
	{
		3162696231,
		31313162,
		62626231,
		31316243
	};

	CMappedFile file;
	LPCVOID pvData;

	Tracef("CPythonNonPlayer::LoadNonPlayerData: %s, sizeof(TMobTable)=%u\n", c_szFileName, sizeof(TMobTable));

	if (!CEterPackManager::Instance().Get(file, c_szFileName, &pvData))
		return false;

	DWORD dwFourCC, dwElements, dwDataSize;

	file.Read(&dwFourCC, sizeof(DWORD));

	if (dwFourCC != MAKEFOURCC('M', 'M', 'P', 'T'))
	{
		TraceError("CPythonNonPlayer::LoadNonPlayerData: invalid Mob proto type %s", c_szFileName);
		return false;
	}

	file.Read(&dwElements, sizeof(DWORD));
	file.Read(&dwDataSize, sizeof(DWORD));

	BYTE* pbData = new BYTE[dwDataSize];
	file.Read(pbData, dwDataSize);
	/////

	CLZObject zObj;

	if (!CLZO::Instance().Decompress(zObj, pbData, s_adwMobProtoKey))
	{
		delete[] pbData;
		return false;
	}

	DWORD structSize = zObj.GetSize() / dwElements;
	DWORD structDiff = zObj.GetSize() % dwElements;
	if ((zObj.GetSize() % sizeof(TMobTable)) != 0)
	{
		TraceError("CPythonNonPlayer::LoadNonPlayerData: invalid size %u check data format. structSize %u, structDiff %u", zObj.GetSize(), structSize, structDiff);
		return false;
	}

	for (DWORD i = 0; i < dwElements; ++i)
	{
		CPythonNonPlayer::TMobTable& t = *((CPythonNonPlayer::TMobTable*)zObj.GetBuffer() + i);
		TMobTable* pTable = &t;

		std::unique_ptr<TMobTable> pNonPlayerData = std::make_unique<TMobTable>();
		memcpy(pNonPlayerData.get(), pTable, sizeof(TMobTable));

		m_NonPlayerDataMap.insert(std::make_pair(pNonPlayerData->dwVnum, std::move(pNonPlayerData)));
	}

	delete[] pbData;
	return true;
}

bool CPythonNonPlayer::GetName(DWORD dwVnum, const char** c_pszName)
{
	const TMobTable* p = GetTable(dwVnum);

	if (!p)
		return false;

	*c_pszName = p->szLocaleName;

	return true;
}

bool CPythonNonPlayer::GetInstanceType(DWORD dwVnum, BYTE* pbType)
{
	const TMobTable* p = GetTable(dwVnum);

	if (!p)
		return false;

	*pbType = p->bType;

	return true;
}

const CPythonNonPlayer::TMobTable* CPythonNonPlayer::GetTable(DWORD dwVnum)
{
	TNonPlayerDataMap::iterator itor = m_NonPlayerDataMap.find(dwVnum);

	if (itor == m_NonPlayerDataMap.end())
		return NULL;

	return itor->second.get();
}

BYTE CPythonNonPlayer::GetEventType(DWORD dwVnum)
{
	const TMobTable* p = GetTable(dwVnum);

	if (!p)
	{
		//Tracef("CPythonNonPlayer::GetEventType - Failed to find virtual number\n");
		return ON_CLICK_EVENT_NONE;
	}

	return p->bOnClickType;
}

BYTE CPythonNonPlayer::GetEventTypeByVID(DWORD dwVID)
{
	CInstanceBase* pInstance = CPythonCharacterManager::Instance().GetInstancePtr(dwVID);

	if (NULL == pInstance)
	{
		//Tracef("CPythonNonPlayer::GetEventTypeByVID - There is no Virtual Number\n");
		return ON_CLICK_EVENT_NONE;
	}

	WORD dwVnum = pInstance->GetVirtualNumber();
	return GetEventType(dwVnum);
}

const char* CPythonNonPlayer::GetMonsterName(DWORD dwVnum)
{
	const CPythonNonPlayer::TMobTable* c_pTable = GetTable(dwVnum);
	if (!c_pTable)
	{
		static const char* sc_szEmpty = "";
		return sc_szEmpty;
	}

	return c_pTable->szLocaleName;
}

DWORD CPythonNonPlayer::GetMonsterColor(DWORD dwVnum)
{
	const CPythonNonPlayer::TMobTable* c_pTable = GetTable(dwVnum);
	if (!c_pTable)
		return 0;

	return c_pTable->dwMonsterColor;
}

DWORD CPythonNonPlayer::GetMonsterRaceFlag(DWORD dwVnum)
{
	const CPythonNonPlayer::TMobTable* c_pTable = GetTable(dwVnum);
	if (!c_pTable)
		return 0;

	return c_pTable->dwRaceFlag;
}
#ifdef ENABLE_PENDANT_SYSTEM
DWORD CPythonNonPlayer::GetMonsterRaceFlagAtt(DWORD dwVnum)
{
	DWORD dwRaceFlag = GetMonsterRaceFlag(dwVnum);
	DWORD dwReturnRaceFlag = 0;

	if (IS_SET(dwRaceFlag, RACE_FLAG_ATT_ELEC))
		dwReturnRaceFlag |= RACE_FLAG_ATT_ELEC;
	if (IS_SET(dwRaceFlag, RACE_FLAG_ATT_FIRE))
		dwReturnRaceFlag |= RACE_FLAG_ATT_FIRE;
	if (IS_SET(dwRaceFlag, RACE_FLAG_ATT_ICE))
		dwReturnRaceFlag |= RACE_FLAG_ATT_ICE;
	if (IS_SET(dwRaceFlag, RACE_FLAG_ATT_WIND))
		dwReturnRaceFlag |= RACE_FLAG_ATT_WIND;
	if (IS_SET(dwRaceFlag, RACE_FLAG_ATT_EARTH))
		dwReturnRaceFlag |= RACE_FLAG_ATT_EARTH;
	if (IS_SET(dwRaceFlag, RACE_FLAG_ATT_DARK))
		dwReturnRaceFlag |= RACE_FLAG_ATT_DARK;

	return dwReturnRaceFlag;
}
#endif

#ifdef ENABLE_TARGET_BOARD_RENEWAL
BYTE CPythonNonPlayer::GetTableValueBYTE(DWORD dwVnum, std::string itorName)
{
	const CPythonNonPlayer::TMobTable* c_pTable = GetTable(dwVnum);
	if (!c_pTable)
		return 0;

	if (itorName == "TYPE")
		return c_pTable->bType;
	else if (itorName == "RANK")
		return c_pTable->bRank;
	else if (itorName == "BATTLE_TYPE")
		return c_pTable->bBattleType;
	else if (itorName == "LEVEL")
		return c_pTable->bLevel;
	else if (itorName == "SIZE")
		return c_pTable->bSize;
	else if (itorName == "REGEN_CYCLE")
		return c_pTable->bRegenCycle;
	else if (itorName == "REGEN_PERCENT")
		return c_pTable->bRegenPercent;
	else if (itorName == "STR")
		return c_pTable->bStr;
	else if (itorName == "DEX")
		return c_pTable->bDex;
	else if (itorName == "CON")
		return c_pTable->bCon;
	else if (itorName == "INT")
		return c_pTable->bInt;
	else if (itorName == "AGGRESSIVE_HP_PCT")
		return c_pTable->bAggresiveHPPct;
	else if (itorName == "MOUNT_CAPACITY")
		return c_pTable->bMountCapacity;
	else if (itorName == "ONCLICK_TYPE")
		return c_pTable->bOnClickType;
	else if (itorName == "EMPIRE")
		return c_pTable->bEmpire;
	else if (itorName == "BERSERK_POINT")
		return c_pTable->bBerserkPoint;
	else if (itorName == "STONESKIN_POINT")
		return c_pTable->bStoneSkinPoint;
	else if (itorName == "GODSPEED_POINT")
		return c_pTable->bGodSpeedPoint;
	else if (itorName == "DEATHBLOW_POINT")
		return c_pTable->bDeathBlowPoint;
	else if (itorName == "REVIVE_POINT")
		return c_pTable->bRevivePoint;

	return 0;
}

LLONG CPythonNonPlayer::GetTableValueDWORD(DWORD dwVnum, std::string itorName)
{
	const CPythonNonPlayer::TMobTable* c_pTable = GetTable(dwVnum);
	if (!c_pTable)
		return 0;

	if (itorName == "EXP")
		return c_pTable->dwExp;
	else if (itorName == "MAX_HP")
		return c_pTable->dwMaxHP;
#ifndef ENABLE_GOLD_LIMIT_REWORK
	else if (itorName == "GOLD_MIN")
		return c_pTable->dwGoldMin;
	else if (itorName == "GOLD_MAX")
		return c_pTable->dwGoldMax;
#endif
	else if (itorName == "AI_FLAG")
		return c_pTable->dwAIFlag;
	else if (itorName == "RACE_FLAG")
		return c_pTable->dwRaceFlag;
	else if (itorName == "IMMUNE_FLAG")
		return c_pTable->dwImmuneFlag;
	else if (itorName == "DAMAGE_RANGE_0")
		return c_pTable->dwDamageRange[0];
	else if (itorName == "DAMAGE_RANGE_1")
		return c_pTable->dwDamageRange[1];
	else if (itorName == "DAMAGE_RANGE_00")
		return ((c_pTable->dwDamageRange[0]*int(c_pTable->fDamMultiply * c_pTable->bBerserkPoint != 0 ? 2 : 1)))+(c_pTable->bLevel * 2 + c_pTable->bStr * 2);
	else if (itorName == "DAMAGE_RANGE_11")
		return ((c_pTable->dwDamageRange[1]*int(c_pTable->fDamMultiply * c_pTable->bBerserkPoint != 0 ? 2 : 1)))+(c_pTable->bLevel * 2 + c_pTable->bStr * 2);
	else if (itorName == "AGGRESSIVE_SIGHT")
		return c_pTable->wAggressiveSight;
	else if (itorName == "ATTACK_RANGE")
		return c_pTable->wAttackRange;
	else if (itorName == "RESSURRECTION_VNUM")
		return c_pTable->dwResurrectionVnum;
	else if (itorName == "DROP_ITEM_VNUM")
		return c_pTable->dwDropItemVnum;
	else if (itorName == "SUMMON_VNUM")
		return c_pTable->dwSummonVnum;
	else if (itorName == "DRAIN_SP")
		return c_pTable->dwDrainSP;
	else if (itorName == "MONSTER_COLOR")
		return c_pTable->dwMonsterColor;
	else if (itorName == "POLY_ITEM_VNUM")
		return c_pTable->dwPolymorphItemVnum;
	else if (itorName == "ATTACK_SPEED")
		return c_pTable->sAttackSpeed;
	else if (itorName == "ATTACK_SPEED0")
		return c_pTable->bBerserkPoint != 0 ? 250 : c_pTable->sAttackSpeed;
	else if (itorName == "MOVE_SPEED")
		return c_pTable->sMovingSpeed;
	else if (itorName == "MOB_RESIST_SWORD")
		return c_pTable->cResists[MOB_RESIST_SWORD];
	else if (itorName == "MOB_RESIST_TWOHAND")
		return c_pTable->cResists[MOB_RESIST_TWOHAND];
	else if (itorName == "MOB_RESIST_DAGGER")
		return c_pTable->cResists[MOB_RESIST_DAGGER];
	else if (itorName == "MOB_RESIST_BELL")
		return c_pTable->cResists[MOB_RESIST_BELL];
	else if (itorName == "MOB_RESIST_FAN")
		return c_pTable->cResists[MOB_RESIST_FAN];
	else if (itorName == "MOB_RESIST_BOW")
		return c_pTable->cResists[MOB_RESIST_BOW];
	else if (itorName == "MOB_RESIST_FIRE")
		return c_pTable->cResists[MOB_RESIST_FIRE];
	else if (itorName == "MOB_RESIST_ELECT")
		return c_pTable->cResists[MOB_RESIST_ELECT];
	else if (itorName == "MOB_RESIST_MAGIC")
		return c_pTable->cResists[MOB_RESIST_MAGIC];
	else if (itorName == "MOB_RESIST_WIND")
		return c_pTable->cResists[MOB_RESIST_WIND];
	else if (itorName == "MOB_RESIST_POISON")
		return c_pTable->cResists[MOB_RESIST_POISON];
	else if (itorName == "MOB_ENCHANT_CURSE")
		return c_pTable->cEnchants[MOB_ENCHANT_CURSE];
	else if (itorName == "MOB_ENCHANT_SLOW")
		return c_pTable->cEnchants[MOB_ENCHANT_SLOW];
	else if (itorName == "MOB_ENCHANT_POISON")
		return c_pTable->cEnchants[MOB_ENCHANT_POISON];
	else if (itorName == "MOB_ENCHANT_STUN")
		return c_pTable->cEnchants[MOB_ENCHANT_STUN];
	else if (itorName == "MOB_ENCHANT_CRITICAL")
		return c_pTable->cEnchants[MOB_ENCHANT_CRITICAL];
	else if (itorName == "MOB_ENCHANT_PENETRATE")
		return c_pTable->cEnchants[MOB_ENCHANT_PENETRATE];

	return 0;
}

long long CPythonNonPlayer::GetTableValueLongLong(DWORD dwVnum, std::string itorName)
{
	const CPythonNonPlayer::TMobTable* c_pTable = GetTable(dwVnum);
	if (!c_pTable)
		return 0;

#ifdef ENABLE_GOLD_LIMIT_REWORK
	else if (itorName == "GOLD_MIN")
		return c_pTable->llGoldMin;
	else if (itorName == "GOLD_MAX")
		return c_pTable->llGoldMax;
#endif

	return 0;
}

float CPythonNonPlayer::GetTableValueFloat(DWORD dwVnum, std::string itorName)
{
	const CPythonNonPlayer::TMobTable* c_pTable = GetTable(dwVnum);
	if (!c_pTable)
		return 0.0f;

	if (itorName == "DAM_MULTIPLY")
		return c_pTable->fDamMultiply;

	return 0.0f;
}

bool CPythonNonPlayer::HasMonsterRaceFlag(DWORD dwFlag, DWORD dwVnum)
{
	DWORD dwRaceFlag = GetMonsterRaceFlag(dwVnum);
	return (dwFlag & dwRaceFlag) != 0;
}
#endif


float CPythonNonPlayer::GetMonsterHitRange(DWORD dwVnum)
{
	const TMobTable* p = GetTable(dwVnum);
	if (!p)
		return 70.0f;

	if (p->wAttackRange)
		return p->wAttackRange;

	return 100.0f;
}

void CPythonNonPlayer::Clear()
{
}

void CPythonNonPlayer::Destroy()
{
	for (TNonPlayerDataMap::iterator itor = m_NonPlayerDataMap.begin(); itor != m_NonPlayerDataMap.end(); ++itor)
	{
		delete itor->second.release();
	}
	m_NonPlayerDataMap.clear();
}

CPythonNonPlayer::CPythonNonPlayer()
{
	Clear();
}

CPythonNonPlayer::~CPythonNonPlayer(void)
{
	Destroy();
}