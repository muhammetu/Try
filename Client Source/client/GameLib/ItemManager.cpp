#include "StdAfx.h"
#include "../eterPack/EterPackManager.h"
#include "../eterLib/ResourceManager.h"

#include "ItemManager.h"

static DWORD s_adwItemProtoKey[4] =
{
	3162696231,
	31313162,
	62626231,
	31316228
};

#ifdef ENABLE_WIKI_SYSTEM
#include <boost/algorithm/string.hpp>
DWORD CItemManager::FindItemSearchBox(const char* itemName, DWORD index, bool reverse)
{
	std::string itemName2 = itemName;
	boost::algorithm::to_lower(itemName2);

	auto it = m_ItemMap.find(index);

	if (reverse)
	{
		auto rev_it = TItemMap::reverse_iterator(it);
		if (rev_it == m_ItemMap.rend())
			rev_it = m_ItemMap.rbegin();
		// ++rev_it;
		for (; rev_it != m_ItemMap.rend(); ++rev_it)
		{
			auto itemData = rev_it->second;
			std::string itemName_ = itemData->GetName();
			boost::algorithm::to_lower(itemName_);
			if (itemName_.rfind(itemName2, 0) == 0)
				return rev_it->first;
		}
	}
	else
	{
		if (it == m_ItemMap.end())
			it = m_ItemMap.begin();
		++it;
		for (; it != m_ItemMap.end(); ++it)
		{
			auto itemData = it->second;
			std::string itemName_ = itemData->GetName();
			boost::algorithm::to_lower(itemName_);
			if (itemName_.rfind(itemName2, 0) == 0)
				return it->first;
		}
	}

	return index;
}
void CItemManager::FindItem(PyObject* poHandler, int itemType)
{
	for (auto &&i : m_ItemMap)
	{
		auto itemData = i.second;
		if (itemData->GetType() == itemType)
			PyCallClassMemberFunc(poHandler, "BINARY_FindItem", Py_BuildValue("(i)", i.first));
		if (itemType == CItemData::ITEM_TYPE_PET)
			if (itemData->GetType() == CItemData::ITEM_TYPE_UNIQUE)
				if (itemData->GetSubType() == CItemData::USE_PET)
					PyCallClassMemberFunc(poHandler, "BINARY_FindItem", Py_BuildValue("(i)", i.first));

	}
}
#endif

BOOL CItemManager::SelectItemData(DWORD dwIndex)
{
	TItemMap::iterator f = m_ItemMap.find(dwIndex);

	if (m_ItemMap.end() == f)
	{
		int n = m_vec_ItemRange.size();
		for (int i = 0; i < n; i++)
		{
			CItemData* p = m_vec_ItemRange[i];
			const CItemData::TItemTable* pTable = p->GetTable();
			if ((pTable->dwVnum < dwIndex) &&
				dwIndex < (pTable->dwVnum + pTable->dwVnumRange))
			{
				m_pSelectedItemData = p;
				return TRUE;
			}
		}
		Tracef(" CItemManager::SelectItemData - FIND ERROR [%d]\n", dwIndex);
		return FALSE;
	}

	m_pSelectedItemData = f->second;

	return TRUE;
}

CItemData* CItemManager::GetSelectedItemDataPointer()
{
	return m_pSelectedItemData;
}

BOOL CItemManager::GetItemDataPointer(DWORD dwItemID, CItemData** ppItemData)
{
	if (0 == dwItemID)
		return FALSE;

	TItemMap::iterator f = m_ItemMap.find(dwItemID);

	if (m_ItemMap.end() == f)
	{
		int n = m_vec_ItemRange.size();
		for (int i = 0; i < n; i++)
		{
			CItemData* p = m_vec_ItemRange[i];
			const CItemData::TItemTable* pTable = p->GetTable();
			if ((pTable->dwVnum < dwItemID) &&
				dwItemID < (pTable->dwVnum + pTable->dwVnumRange))
			{
				*ppItemData = p;
				return TRUE;
			}
		}
		Tracef(" CItemManager::GetItemDataPointer - FIND ERROR [%d]\n", dwItemID);
		return FALSE;
	}

	*ppItemData = f->second;

	return TRUE;
}

#ifdef ENABLE_BLEND_REWORK
BOOL CItemManager::GetBlendInfoPointer(DWORD dwItemID, TBlendInfo** ppBlendInfo)
{
	if (0 == dwItemID)
		return FALSE;

	TBlendMap::iterator f = m_BlendMap.find(dwItemID);
	if (f == m_BlendMap.end())
		return FALSE;

	*ppBlendInfo = &f->second;
	return TRUE;
}
#endif

CItemData* CItemManager::MakeItemData(DWORD dwIndex)
{
	TItemMap::iterator f = m_ItemMap.find(dwIndex);

	if (m_ItemMap.end() == f)
	{
		CItemData* pItemData = CItemData::New();

		m_ItemMap.insert(TItemMap::value_type(dwIndex, pItemData));

		return pItemData;
	}

	return f->second;
}

////////////////////////////////////////////////////////////////////////////////////////
// Load Item Table

bool CItemManager::LoadItemList(const char* c_szFileName)
{
	CMappedFile File;
	LPCVOID pData;

	if (!CEterPackManager::Instance().Get(File, c_szFileName, &pData))
		return false;

	CMemoryTextFileLoader textFileLoader;
	textFileLoader.Bind(File.Size(), pData);

	CTokenVector TokenVector;
	for (DWORD i = 0; i < textFileLoader.GetLineCount(); ++i)
	{
		if (!textFileLoader.SplitLine(i, &TokenVector, "\t"))
			continue;

		if (!(TokenVector.size() == 3 || TokenVector.size() == 4))
		{
			TraceError(" CItemManager::LoadItemList(%s) - StrangeLine in %d\n", c_szFileName, i);
			continue;
		}

		const std::string& c_rstrID = TokenVector[0];
		const std::string& c_rstrType = TokenVector[1];
		const std::string& c_rstrIcon = TokenVector[2];

		DWORD dwItemVNum = atoi(c_rstrID.c_str());

		CItemData* pItemData = MakeItemData(dwItemVNum);

		if (4 == TokenVector.size())
		{
#ifdef ENABLE_AURA_COSTUME_SYSTEM
			if (!strcmp(c_rstrType.c_str(), "AURA"))
			{
				const std::string& c_rstrAuraEffectFileName = TokenVector[3];
				pItemData->SetAuraEffectID(c_rstrAuraEffectFileName.c_str());
				pItemData->SetDefaultItemData(c_rstrIcon.c_str());
			}
			else
			{
				const std::string& c_rstrModelFileName = TokenVector[3];
				pItemData->SetDefaultItemData(c_rstrIcon.c_str(), c_rstrModelFileName.c_str());
			}
#else
			const std::string& c_rstrModelFileName = TokenVector[3];
			pItemData->SetDefaultItemData(c_rstrIcon.c_str(), c_rstrModelFileName.c_str());
#endif
		}
		else
		{
			pItemData->SetDefaultItemData(c_rstrIcon.c_str());
		}
	}

	return true;
}

const std::string& __SnapString(const std::string& c_rstSrc, std::string& rstTemp)
{
	UINT uSrcLen = c_rstSrc.length();
	if (uSrcLen < 2)
		return c_rstSrc;

	if (c_rstSrc[0] != '"')
		return c_rstSrc;

	UINT uLeftCut = 1;

	UINT uRightCut = uSrcLen;
	if (c_rstSrc[uSrcLen - 1] == '"')
		uRightCut = uSrcLen - 1;

	rstTemp = c_rstSrc.substr(uLeftCut, uRightCut - uLeftCut);
	return rstTemp;
}

#ifdef ENABLE_OFFLINE_SHOP
void CItemManager::GetItemsNameMap(std::map<DWORD, std::string>& inMap)
{
	inMap.clear();

	for (auto& it : m_ItemMap)
		inMap.insert(std::make_pair(it.first, it.second->GetName()));
}
#endif
bool CItemManager::LoadItemDesc(const char* c_szFileName)
{
	const VOID* pvData;
	CMappedFile kFile;
	if (!CEterPackManager::Instance().Get(kFile, c_szFileName, &pvData))
	{
		Tracenf("CItemManager::LoadItemDesc(c_szFileName=%s) - Load Error", c_szFileName);
		return false;
	}

	CMemoryTextFileLoader kTextFileLoader;
	kTextFileLoader.Bind(kFile.Size(), pvData);

	std::string stTemp;

	CTokenVector kTokenVector;
	for (DWORD i = 0; i < kTextFileLoader.GetLineCount(); ++i)
	{
		if (!kTextFileLoader.SplitLineByTab(i, &kTokenVector))
			continue;

		while (kTokenVector.size() < ITEMDESC_COL_NUM)
			kTokenVector.push_back("");

		//assert(kTokenVector.size()==ITEMDESC_COL_NUM);

		DWORD dwVnum = atoi(kTokenVector[ITEMDESC_COL_VNUM].c_str());
		const std::string& c_rstDesc = kTokenVector[ITEMDESC_COL_DESC];
		const std::string& c_rstSumm = kTokenVector[ITEMDESC_COL_SUMM];
		TItemMap::iterator f = m_ItemMap.find(dwVnum);
		if (m_ItemMap.end() == f)
			continue;

		CItemData* pkItemDataFind = f->second;

		pkItemDataFind->SetDescription(__SnapString(c_rstDesc, stTemp));
		pkItemDataFind->SetSummary(__SnapString(c_rstSumm, stTemp));
	}
	return true;
}

DWORD GetHashCode(const char* pString)
{
	unsigned long i, len;
	unsigned long ch;
	unsigned long result;

	len = strlen(pString);
	result = 5381;
	for (i = 0; i < len; i++)
	{
		ch = (unsigned long)pString[i];
		result = ((result << 5) + result) + ch; // hash * 33 + ch
	}

	return result;
}

bool CItemManager::LoadItemTable(const char* c_szFileName)
{
	CMappedFile file;
	LPCVOID pvData;

	if (!CEterPackManager::Instance().Get(file, c_szFileName, &pvData))
	{
		TraceError("cannot find item_proto %s", c_szFileName);
		return false;
	}

	DWORD dwFourCC, dwElements, dwDataSize;
	DWORD dwVersion = 0;
	DWORD dwStride = 0;

	file.Read(&dwFourCC, sizeof(DWORD));

	if (dwFourCC == MAKEFOURCC('M', 'I', 'P', 'X'))
	{
		file.Read(&dwVersion, sizeof(DWORD));
		file.Read(&dwStride, sizeof(DWORD));

		if (dwVersion != 1)
		{
			TraceError("CPythonItem::LoadItemTable: invalid item_proto[%s] VERSION[%d]", c_szFileName, dwVersion);
			return false;
		}

		if (dwStride != sizeof(CItemData::TItemTable))
		{
			TraceError("CPythonItem::LoadItemTable: invalid item_proto[%s] STRIDE[%d] != sizeof(SItemTable)",
				c_szFileName, dwStride, sizeof(CItemData::TItemTable));
			return false;
		}
	}
	else if (dwFourCC != MAKEFOURCC('M', 'I', 'P', 'T'))
	{
		TraceError("CPythonItem::LoadItemTable: invalid item proto type %s", c_szFileName);
		return false;
	}

	file.Read(&dwElements, sizeof(DWORD));
	file.Read(&dwDataSize, sizeof(DWORD));

	std::unique_ptr<BYTE[]> pbData(new BYTE[dwDataSize]());
	file.Read(pbData.get(), dwDataSize);

	/////

	CLZObject zObj;

	if (!CLZO::Instance().Decompress(zObj, pbData.get(), s_adwItemProtoKey))
	{
		return false;
	}

	/////

	char szName[64 + 1];
	std::map<DWORD, DWORD> itemNameMap;

	for (DWORD i = 0; i < dwElements; ++i)
	{
		CItemData::TItemTable& t = *((CItemData::TItemTable*)zObj.GetBuffer() + i);
		CItemData::TItemTable* table = &t;

		CItemData* pItemData;
		DWORD dwVnum = table->dwVnum;

		TItemMap::iterator f = m_ItemMap.find(dwVnum);
		if (m_ItemMap.end() == f)
		{
			_snprintf(szName, sizeof(szName), "icon/item/%05d.tga", dwVnum);


			pItemData = CItemData::New();

			pItemData->SetDefaultItemData(szName);
			m_ItemMap.insert(TItemMap::value_type(dwVnum, pItemData));
		}
		else
		{
			pItemData = f->second;
		}
		if (itemNameMap.find(GetHashCode(table->szName)) == itemNameMap.end())
			itemNameMap.insert(std::map<DWORD, DWORD>::value_type(GetHashCode(table->szName), table->dwVnum));
		pItemData->SetItemTableData(table);

		if (0 != table->dwVnumRange)
		{
			m_vec_ItemRange.push_back(pItemData);
		}
	}

	return true;
}

#ifdef ENABLE_SHINING_EFFECT_UTILITY
bool CItemManager::LoadShiningTable(const char* szShiningTable)
{
	CMappedFile File;
	LPCVOID pData;

	if (!CEterPackManager::Instance().Get(File, szShiningTable, &pData))
		return false;

	CMemoryTextFileLoader textFileLoader;
	textFileLoader.Bind(File.Size(), pData);

	CTokenVector TokenVector;
	for (DWORD i = 0; i < textFileLoader.GetLineCount(); ++i)
	{
		if (!textFileLoader.SplitLine(i, &TokenVector, "\t"))
			continue;

		if (TokenVector.size() > (1 + CItemData::ITEM_SHINING_MAX_COUNT))
		{
			TraceError("CItemManager::LoadShiningTable(%s) - LoadShiningTable in %d\n - RowSize: %d MaxRowSize: %d", szShiningTable, i, TokenVector.size(), CItemData::ITEM_SHINING_MAX_COUNT);
		}

		const std::string& c_rstrID = TokenVector[0];

		int pos = c_rstrID.find("~");

		if (std::string::npos == pos)
		{
			DWORD dwItemVNum = atoi(c_rstrID.c_str());

			CItemData* pItemData = MakeItemData(dwItemVNum);
			if (pItemData)
			{
				for (BYTE i = 0; i < CItemData::ITEM_SHINING_MAX_COUNT; i++)
				{
					if (i < (TokenVector.size() - 1))
					{
						const std::string& c_rstrEffectPath = TokenVector[i + 1];
						pItemData->SetItemShiningTableData(i, c_rstrEffectPath.c_str());
					}
					else
					{
						pItemData->SetItemShiningTableData(i, "");
					}
				}
			}
		}
		else
		{
			std::string s_start_vnum(c_rstrID.substr(0, pos));
			std::string s_end_vnum(c_rstrID.substr(pos + 1));

			int start_vnum = atoi(s_start_vnum.c_str());
			int end_vnum = atoi(s_end_vnum.c_str());
			DWORD vnum = start_vnum;

			while (vnum <= end_vnum)
			{
				CItemData* pItemData = MakeItemData(vnum);
				if (pItemData)
				{
					for (BYTE i = 0; i < CItemData::ITEM_SHINING_MAX_COUNT; i++)
					{
						if (i < (TokenVector.size() - 1))
						{
							const std::string& c_rstrEffectPath = TokenVector[i + 1];
							pItemData->SetItemShiningTableData(i, c_rstrEffectPath.c_str());
						}
						else
						{
							pItemData->SetItemShiningTableData(i, "");
						}
					}
				}

				++vnum;
			}
		}
	}

	return true;
}
#endif

#ifdef ENABLE_ACCE_SYSTEM
bool CItemManager::LoadItemScale(const char* szItemScale)
{
	CMappedFile File;
	LPCVOID pData;
	if (!CEterPackManager::Instance().Get(File, szItemScale, &pData))
		return false;

	CMemoryTextFileLoader textFileLoader;
	textFileLoader.Bind(File.Size(), pData);

	CTokenVector TokenVector;
	for (DWORD i = 0; i < textFileLoader.GetLineCount(); ++i)
	{
		if (!textFileLoader.SplitLine(i, &TokenVector, "\t"))
			continue;

		if (!(TokenVector.size() == 6 || TokenVector.size() == 7))
		{
			TraceError(" CItemManager::LoadItemScale(%s) - Error on line %d\n", szItemScale, i);
			continue;
		}

		const std::string& c_rstrID = TokenVector[ITEMSCALE_COL_VNUM];
		const std::string& c_rstrJob = TokenVector[ITEMSCALE_COL_JOB];
		const std::string& c_rstrSex = TokenVector[ITEMSCALE_COL_SEX];
		const std::string& c_rstrScaleX = TokenVector[ITEMSCALE_COL_SCALE_X];
		const std::string& c_rstrScaleY = TokenVector[ITEMSCALE_COL_SCALE_Y];
		const std::string& c_rstrScaleZ = TokenVector[ITEMSCALE_COL_SCALE_Z];

		DWORD dwItemVnum = atoi(c_rstrID.c_str());
		BYTE byJob = 0;
		if (!strcmp(c_rstrJob.c_str(), "JOB_WARRIOR")) byJob = NRaceData::JOB_WARRIOR;
		if (!strcmp(c_rstrJob.c_str(), "JOB_ASSASSIN")) byJob = NRaceData::JOB_ASSASSIN;
		if (!strcmp(c_rstrJob.c_str(), "JOB_SURA")) byJob = NRaceData::JOB_SURA;
		if (!strcmp(c_rstrJob.c_str(), "JOB_SHAMAN")) byJob = NRaceData::JOB_SHAMAN;
		if (!strcmp(c_rstrJob.c_str(), "JOB_WOLFMAN")) byJob = NRaceData::JOB_WOLFMAN;
		BYTE bySex = c_rstrSex[0] == 'M';

		float fScaleX = atof(c_rstrScaleX.c_str()) * 0.01f;
		float fScaleY = atof(c_rstrScaleY.c_str()) * 0.01f;
		float fScaleZ = atof(c_rstrScaleZ.c_str()) * 0.01f;
		float fParticleScale = 1.0f;
		if (TokenVector.size() == 7)
		{
			const std::string& c_rstrParticleScale = TokenVector[ITEMSCALE_COL_PARTICLE_SCALE];
			fParticleScale = atof(c_rstrParticleScale.c_str());
		}

		CItemData* pItemData = MakeItemData(dwItemVnum);
		BYTE bGradeMax = CItemData::ACCE_GRADE_MAX_NUM;

		for (int i = 0; i < bGradeMax; ++i)
		{
			pItemData = MakeItemData(dwItemVnum + i);
			if (pItemData)
				pItemData->SetItemTableScaleData(byJob, bySex, fScaleX, fScaleY, fScaleZ, fParticleScale);
		}
	}

	return true;
}
#endif

#ifdef ENABLE_BLEND_REWORK
#define ATTR_INFO std::pair<const char*, BYTE>
BYTE CItemManager::GetAttrTypeByName(const std::string& stAttrType)
{
	if (stAttrType == "")
		return CItemData::APPLY_NONE;

	static const ATTR_INFO astAttrInfo[] = {
		ATTR_INFO("NONE", CItemData::APPLY_NONE),
		ATTR_INFO("MAX_HP", CItemData::APPLY_MAX_HP),
		ATTR_INFO("MAX_SP", CItemData::APPLY_MAX_SP),
		ATTR_INFO("CON", CItemData::APPLY_CON),
		ATTR_INFO("STR", CItemData::APPLY_STR),
		ATTR_INFO("INT", CItemData::APPLY_INT),
		ATTR_INFO("DEX", CItemData::APPLY_DEX),
		ATTR_INFO("ATT_SPEED", CItemData::APPLY_ATT_SPEED),
		ATTR_INFO("ATTACK_SPEED", CItemData::APPLY_ATT_SPEED),
		ATTR_INFO("MOV_SPEED", CItemData::APPLY_MOV_SPEED),
		ATTR_INFO("STUN_PCT", CItemData::APPLY_STUN_PCT),
		ATTR_INFO("CRITICAL_PCT", CItemData::APPLY_CRITICAL_PCT),
		ATTR_INFO("PENETRATE_PCT", CItemData::APPLY_PENETRATE_PCT),
		ATTR_INFO("POISON_PCT", CItemData::APPLY_POISON_PCT),
		ATTR_INFO("BLOCK", CItemData::APPLY_BLOCK),
		ATTR_INFO("DODGE", CItemData::APPLY_DODGE),
		ATTR_INFO("RESIST_MAGIC", CItemData::APPLY_RESIST_MAGIC),
		ATTR_INFO("ITEM_DROP_BONUS", CItemData::APPLY_ITEM_DROP_BONUS),
		ATTR_INFO("ATT_BONUS", CItemData::APPLY_ATT_GRADE_BONUS),
		ATTR_INFO("ATT_GRADE_BONUS", CItemData::APPLY_ATT_GRADE_BONUS),
		ATTR_INFO("ATTBONUS_HUMAN", CItemData::APPLY_ATTBONUS_HUMAN),
		ATTR_INFO("ATT_BONUS_TO_MONSTER", CItemData::APPLY_ATT_BONUS_TO_MONSTER),
		ATTR_INFO("DEF_BONUS", CItemData::APPLY_DEF_GRADE_BONUS),
		ATTR_INFO("DEF_GRADE_BONUS", CItemData::APPLY_DEF_GRADE_BONUS),
		ATTR_INFO("NORMAL_HIT_DAMAGE_BONUS", CItemData::APPLY_NORMAL_HIT_DAMAGE_BONUS),
		ATTR_INFO("NORMAL_HIT_DEFEND_BONUS", CItemData::APPLY_NORMAL_HIT_DEFEND_BONUS),
		ATTR_INFO("SKILL_DAMAGE_BONUS", CItemData::APPLY_SKILL_DAMAGE_BONUS),
		ATTR_INFO("SKILL_DEFEND_BONUS", CItemData::APPLY_SKILL_DEFEND_BONUS),
		ATTR_INFO("ANTI_PENETRATE_PCT", CItemData::APPLY_ANTI_PENETRATE_PCT),
		ATTR_INFO("ATTBONUS_UNDEAD", CItemData::APPLY_ATTBONUS_UNDEAD),
		ATTR_INFO("CAST_SPEED", CItemData::APPLY_CAST_SPEED),
		ATTR_INFO("ENERGY", CItemData::APPLY_ENERGY),
		ATTR_INFO("ATTBONUS_ANIMAL", CItemData::APPLY_ATTBONUS_ANIMAL),
		ATTR_INFO("ATTBONUS_ORC", CItemData::APPLY_ATTBONUS_ORC),
		ATTR_INFO("ATTBONUS_MILGYO", CItemData::APPLY_ATTBONUS_MILGYO),
		ATTR_INFO("ATTBONUS_DEVIL", CItemData::APPLY_ATTBONUS_DEVIL),
		ATTR_INFO("RESIST_ICE", CItemData::APPLY_RESIST_ICE),
		ATTR_INFO("RESIST_EARTH", CItemData::APPLY_RESIST_EARTH),
		ATTR_INFO("RESIST_DARK", CItemData::APPLY_RESIST_DARK),
		ATTR_INFO("RESIST_FIRE", CItemData::APPLY_RESIST_FIRE),
		ATTR_INFO("RESIST_ELEC", CItemData::APPLY_RESIST_ELEC),
		ATTR_INFO("RESIST_WIND", CItemData::APPLY_RESIST_WIND),
		ATTR_INFO("ANTI_CRITICAL_PCT", CItemData::APPLY_ANTI_CRITICAL_PCT),
		ATTR_INFO("REFLECT_MELEE", CItemData::APPLY_REFLECT_MELEE),
		ATTR_INFO("HP_REGEN", CItemData::APPLY_HP_REGEN),
		ATTR_INFO("SP_REGEN", CItemData::APPLY_SP_REGEN),
		ATTR_INFO("STEAL_HP", CItemData::APPLY_STEAL_HP),
		ATTR_INFO("STEAL_SP", CItemData::APPLY_STEAL_SP),
		ATTR_INFO("POTION_BONUS", CItemData::APPLY_POTION_BONUS),
		ATTR_INFO("MALL_ATTBONUS", CItemData::APPLY_MALL_ATTBONUS),
		ATTR_INFO("MALL_DEFBONUS", CItemData::APPLY_MALL_DEFBONUS),
		ATTR_INFO("MAX_HP_PCT", CItemData::APPLY_MAX_HP_PCT),
		ATTR_INFO("MAX_SP_PCT", CItemData::APPLY_MAX_SP_PCT),
		ATTR_INFO("IMMUNE_STUN", CItemData::APPLY_IMMUNE_STUN),
		ATTR_INFO("ATTBONUS_STONE", CItemData::APPLY_ATTBONUS_STONE),
		ATTR_INFO("ATTBONUS_BOSS", CItemData::APPLY_ATTBONUS_BOSS),
	};

	for (int i = 0; i < sizeof(astAttrInfo) / sizeof(ATTR_INFO); ++i)
	{
		if (!stricmp(stAttrType.c_str(), astAttrInfo[i].first))
			return astAttrInfo[i].second;
	}

	TraceError("GetAttrTypeByName(%s) - cannot get attribute type\n", stAttrType.c_str());
	return CItemData::APPLY_NONE;
}
#undef ATTR_INFO

bool CItemManager::LoadItemBlend(const char* c_szFileName)
{
	CMappedFile File;
	LPCVOID pData;

	if (!CEterPackManager::Instance().Get(File, c_szFileName, &pData))
		return false;

	CMemoryTextFileLoader textFileLoader;
	textFileLoader.Bind(File.Size(), pData);

	TBlendInfo kBlendInfo;
	BYTE bIndex = 0;

	CTokenVector TokenVector;
	for (DWORD i = 0; i < textFileLoader.GetLineCount(); ++i)
	{
		if (!textFileLoader.SplitLine(i, &TokenVector, "\t"))
			continue;

		switch (bIndex)
		{
		case 0:
		{
			if (TokenVector.size() >= 1 && TokenVector[0].find_first_of("section") == 0)
			{
				kBlendInfo.dwVnum = 0;
				kBlendInfo.bApplyType = 0;
				kBlendInfo.vec_iApplyValue.clear();
				kBlendInfo.vec_iApplyDuration.clear();
				++bIndex;
			}
		}
		break;

		case 1:
		{
			if (TokenVector.size() >= 2)
			{
				if (TokenVector[0].find_first_of("item_vnum") != 0)
				{
					TraceError("invalid section line [item_vnum] %u", i);
					bIndex = 0;
					break;
				}

				kBlendInfo.dwVnum = atoi(TokenVector[1].c_str());
				++bIndex;
			}
		}
		break;

		case 2:
		{
			if (TokenVector.size() >= 2)
			{
				if (TokenVector[0].find_first_of("apply_type") != 0)
				{
					TraceError("invalid section line [apply_type] %u", i);
					bIndex = 0;
					break;
				}

				kBlendInfo.bApplyType = GetAttrTypeByName(TokenVector[1]);
				++bIndex;
			}
		}
		break;

		case 3:
		{
			if (TokenVector.size() >= 2)
			{
				if (TokenVector[0].find_first_of("apply_value") != 0)
				{
					TraceError("invalid section line [apply_value] %u", i);
					bIndex = 0;
					break;
				}

				for (int j = 1; j < TokenVector.size(); ++j)
				{
					kBlendInfo.vec_iApplyValue.push_back(atoi(TokenVector[j].c_str()));
				}
				++bIndex;
			}
		}
		break;

		case 4:
		{
			if (TokenVector.size() >= 2)
			{
				if (TokenVector[0].find_first_of("apply_duration") != 0)
				{
					TraceError("invalid section line [apply_duration] %u", i);
					bIndex = 0;
					break;
				}

				for (int j = 1; j < TokenVector.size(); ++j)
				{
					kBlendInfo.vec_iApplyDuration.push_back(atoi(TokenVector[j].c_str()));
				}
				++bIndex;
			}
		}
		break;

		case 5:
		{
			if (TokenVector.size() >= 1 && TokenVector[0].find_first_of("end") == 0)
			{
				bIndex = 0;

				if (kBlendInfo.dwVnum == 0)
				{
					TraceError("invalid vnum for blend line %u vnum 0", i);
					break;
				}

				if (kBlendInfo.bApplyType == 0)
				{
					TraceError("invalid apply type for blend vnum %u", kBlendInfo.dwVnum);
					break;
				}

				if (kBlendInfo.vec_iApplyValue.size() != kBlendInfo.vec_iApplyDuration.size())
				{
					TraceError("invalid size compare of iApplyValue and iApplyDuration vnum %u", kBlendInfo.dwVnum);
					break;
				}

				TBlendInfo& rkInfo = m_BlendMap[kBlendInfo.dwVnum];
				rkInfo.dwVnum = kBlendInfo.dwVnum;
				rkInfo.bApplyType = kBlendInfo.bApplyType;
				for (int j = 0; j < kBlendInfo.vec_iApplyValue.size(); ++j)
				{
					rkInfo.vec_iApplyValue.push_back(kBlendInfo.vec_iApplyValue[j]);
					rkInfo.vec_iApplyDuration.push_back(kBlendInfo.vec_iApplyDuration[j]);
				}
			}
		}
		break;
		}
	}

	return true;
}
#endif

#ifdef ENABLE_DS_SET
bool CItemManager::LoadDragonSoulTable(const char* szDragonSoulTable)
{
	if (!m_DragonSoulTable)
	{
		TraceError("DragonSoulTable not initialized!");
		return false;
	}

	return m_DragonSoulTable->Load(szDragonSoulTable);
}
CDragonSoulTable* CItemManager::GetDragonSoulTable()
{
	return m_DragonSoulTable;
}
#endif

#ifdef ENABLE_AURA_COSTUME_SYSTEM
bool CItemManager::LoadAuraScale(const char* c_szFileName)
{
	CMappedFile File;
	LPCVOID pData;
	if (!CEterPackManager::Instance().Get(File, c_szFileName, &pData))
		return false;

	CMemoryTextFileLoader textFileLoader;
	textFileLoader.Bind(File.Size(), pData);

	CTokenVector TokenVector;
	for (DWORD i = 0; i < textFileLoader.GetLineCount(); ++i)
	{
		if (!textFileLoader.SplitLine(i, &TokenVector, "\t"))
			continue;

		if (TokenVector.size() != AURA_SCALE_COL_NUM)
		{
			TraceError(" CItemManager::LoadAuraScale(%s) - Error on line %d\n", c_szFileName, i);
			continue;
		}

		const std::string& c_rstrID = TokenVector[AURA_SCALE_COL_VNUM];
		const std::string& c_rstrJob = TokenVector[AURA_SCALE_COL_JOB];
		const std::string& c_rstrSex = TokenVector[AURA_SCALE_COL_SEX];
		const std::string& c_rstrMeshScaleX = TokenVector[AURA_SCALE_COL_MESH_SCALE_X];
		const std::string& c_rstrMeshScaleY = TokenVector[AURA_SCALE_COL_MESH_SCALE_Y];
		const std::string& c_rstrMeshScaleZ = TokenVector[AURA_SCALE_COL_MESH_SCALE_Z];
		const std::string& c_rstrParticleScale = TokenVector[AURA_SCALE_COL_PARTICLE_SCALE];

		DWORD dwItemVnum = atoi(c_rstrID.c_str());
		BYTE byJob = 0;
		if (!strcmp(c_rstrJob.c_str(), "JOB_WARRIOR"))
			byJob = NRaceData::JOB_WARRIOR;
		if (!strcmp(c_rstrJob.c_str(), "JOB_ASSASSIN"))
			byJob = NRaceData::JOB_ASSASSIN;
		if (!strcmp(c_rstrJob.c_str(), "JOB_SURA"))
			byJob = NRaceData::JOB_SURA;
		if (!strcmp(c_rstrJob.c_str(), "JOB_SHAMAN"))
			byJob = NRaceData::JOB_SHAMAN;
		if (!strcmp(c_rstrJob.c_str(), "JOB_WOLFMAN"))
			byJob = NRaceData::JOB_WOLFMAN;
		BYTE bySex = c_rstrSex[0] == 'M';

		float fMeshScaleX = atof(c_rstrMeshScaleX.c_str()) * 0.01f;
		float fMeshScaleY = atof(c_rstrMeshScaleY.c_str()) * 0.01f;
		float fMeshScaleZ = atof(c_rstrMeshScaleZ.c_str()) * 0.01f;
		float fParticleScale = atof(c_rstrParticleScale.c_str());

		for (BYTE i = 0; i < CItemData::AURA_GRADE_MAX_NUM; ++i)
		{
			CItemData* pItemData = MakeItemData(dwItemVnum + i);
			if (pItemData)
				pItemData->SetAuraScaleTableData(byJob, bySex, fMeshScaleX, fMeshScaleY, fMeshScaleZ, fParticleScale);
		}
	}
	return true;
}
#endif


void CItemManager::Destroy()
{
	TItemMap::iterator i;
	for (i = m_ItemMap.begin(); i != m_ItemMap.end(); ++i)
		CItemData::Delete(i->second);

	m_ItemMap.clear();
#ifdef ENABLE_BLEND_REWORK
	m_BlendMap.clear();
#endif
}

CItemManager::CItemManager() : m_pSelectedItemData(NULL)
{
#ifdef ENABLE_DS_SET
	m_DragonSoulTable = new CDragonSoulTable;
#endif
}
CItemManager::~CItemManager()
{
	Destroy();
}