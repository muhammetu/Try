#include "stdafx.h"
#include "utils.h"
#include "config.h"
#include "char.h"
#include "char_manager.h"
#include "desc_client.h"
#include "db.h"
#include "log.h"
#include "skill.h"
#include "text_file_loader.h"
#include "priv_manager.h"
#include "questmanager.h"
#include "unique_item.h"
#include "safebox.h"
#include "blend_item.h"
#include "locale_service.h"
#include "item.h"
#include "item_manager.h"

#include "../../common/VnumHelper.h"
#include "DragonSoul.h"
#ifndef __CUBE_WINDOW__
#include "cube.h"
#else
#include "cuberenewal.h"
#endif

#ifdef __EVENT_SYSTEM__
#include "PetSystem.h"
#endif

#include "game_events.h"

ITEM_MANAGER::ITEM_MANAGER()
	: m_iTopOfTable(0), m_dwVIDCount(0), m_dwCurrentID(0)
{
	m_ItemIDRange.dwMin = m_ItemIDRange.dwMax = m_ItemIDRange.dwUsableItemIDMin = 0;
	m_ItemIDSpareRange.dwMin = m_ItemIDSpareRange.dwMax = m_ItemIDSpareRange.dwUsableItemIDMin = 0;
}

ITEM_MANAGER::~ITEM_MANAGER()
{
	Destroy();
}

void ITEM_MANAGER::Destroy()
{
	itertype(m_VIDMap) it = m_VIDMap.begin();
	for (; it != m_VIDMap.end(); ++it) {
		M2_DELETE(it->second);
	}
	m_VIDMap.clear();
}

void ITEM_MANAGER::GracefulShutdown()
{
	std::unordered_set<LPITEM>::iterator it = m_set_pkItemForDelayedSave.begin();

	while (it != m_set_pkItemForDelayedSave.end())
		SaveSingleItem(*(it++));

	m_set_pkItemForDelayedSave.clear();
}

bool ITEM_MANAGER::Initialize(TItemTable* table, int size)
{
	if (!m_vec_prototype.empty())
		m_vec_prototype.clear();

	int	i;

	m_vec_prototype.resize(size);
	thecore_memcpy(&m_vec_prototype[0], table, sizeof(TItemTable) * size);
	for (int i = 0; i < size; i++)
	{
		if (0 != m_vec_prototype[i].dwVnumRange)
		{
			m_vec_item_vnum_range_info.push_back(&m_vec_prototype[i]);
		}
	}

	m_map_ItemRefineFrom.clear();
	for (i = 0; i < size; ++i)
	{
		if (m_vec_prototype[i].dwRefinedVnum)
			m_map_ItemRefineFrom.insert(std::make_pair(m_vec_prototype[i].dwRefinedVnum, m_vec_prototype[i].dwVnum));

		if (m_vec_prototype[i].bType == ITEM_QUEST || IS_SET(m_vec_prototype[i].dwFlags, ITEM_FLAG_QUEST_USE | ITEM_FLAG_QUEST_USE_MULTIPLE))
			quest::CQuestManager::instance().RegisterNPCVnum(m_vec_prototype[i].dwVnum);

		m_map_vid.insert(std::map<DWORD, TItemTable>::value_type(m_vec_prototype[i].dwVnum, m_vec_prototype[i]));
		if (test_server)
			sys_log(0, "ITEM_INFO %d %s ", m_vec_prototype[i].dwVnum, m_vec_prototype[i].szName);
	}

	/*int len = 0, len2;
	char buf[512];

	for (i = 0; i < size; ++i)
	{
		len2 = snprintf(buf + len, sizeof(buf) - len, "%5u %-16s", m_vec_prototype[i].dwVnum, m_vec_prototype[i].szLocaleName);

		if (len2 < 0 || len2 >= (int)sizeof(buf) - len)
			len += (sizeof(buf) - len) - 1;
		else
			len += len2;

		if (!((i + 1) % 4))
		{
			if (!test_server)
				sys_log(0, "%s", buf);
			len = 0;
		}
		else
		{
			buf[len++] = '\t';
			buf[len] = '\0';
		}
	}

	if ((i + 1) % 4)
	{
		if (!test_server)
			sys_log(0, "%s", buf);
	}*/

	ITEM_VID_MAP::iterator it = m_VIDMap.begin();

	sys_log(1, "ITEM_VID_MAP %d", m_VIDMap.size());

	while (it != m_VIDMap.end())
	{
		LPITEM item = it->second;
		++it;

		const TItemTable* tableInfo = GetTable(item->GetOriginalVnum());

		if (NULL == tableInfo)
		{
			sys_err("cannot reset item table");
			item->SetProto(NULL);
		}

		item->SetProto(tableInfo);
	}

	return true;
}

LPITEM ITEM_MANAGER::CreateItem(DWORD vnum, DWORD count, DWORD id, bool bTryMagic, int iRarePct, bool bSkipSave)
{
	if (0 == vnum)
		return NULL;

	DWORD dwMaskVnum = 0;
	if (GetMaskVnum(vnum))
	{
		dwMaskVnum = GetMaskVnum(vnum);
	}

	const TItemTable* table = GetTable(vnum);

	if (NULL == table)
		return NULL;

	LPITEM item = NULL;
//#define DUP_SIMDILIK_DENEME
#ifdef DUP_SIMDILIK_DENEME
	if (quest::CQuestManager::instance().GetEventFlag("DUP_SIMDILIK_DENEME"))
	{
		if (m_map_pkItemByID.find(id) != m_map_pkItemByID.end())
		{
			item = m_map_pkItemByID[id];
			LPCHARACTER owner = item->GetOwner();
			sys_err("ITEM_ID_DUP: %u %s %d (%u %d)", id, item->GetName(), item->GetVnum(), id, item->GetVnum());
			item->SetCell(NULL, 0);
			M2_DESTROY_ITEM(item);
		}
	}
	else
	{
		if (m_map_pkItemByID.find(id) != m_map_pkItemByID.end())
		{
			item = m_map_pkItemByID[id];
			LPCHARACTER owner = item->GetOwner();
			sys_err("ITEM_ID_DUP: %u %s %d (%u %d)", id, item->GetName(), item->GetVnum(), id, item->GetVnum());
			return NULL;
		}
	}
#else
	if (m_map_pkItemByID.find(id) != m_map_pkItemByID.end())
	{
		item = m_map_pkItemByID[id];
		//LPCHARACTER owner = item->GetOwner();
		sys_err("ITEM_ID_DUP: vnum %u, count %u, id %u, (vnum %u, count %u, name %s)", vnum, count, id, item->GetVnum(), item->GetCount(), item->GetName());
		return NULL;
	}
#endif

	item = M2_NEW CItem(vnum);

	bool bIsNewItem = (0 == id);

	item->Initialize();
	item->SetProto(table);
	item->SetMaskVnum(dwMaskVnum);

	if (item->GetType() == ITEM_ELK)
		item->SetSkipSave(true);

	else if (!bIsNewItem)
	{
		item->SetID(id);
		item->SetSkipSave(true);
	}
	else
	{
		item->SetID(GetNewID());

		if (item->GetType() == ITEM_UNIQUE)
		{
			if (item->GetValue(2) == 0)
				item->SetSocket(ITEM_SOCKET_UNIQUE_REMAIN_TIME, item->GetValue(0));
			else
			{
				//int globalTime = get_global_time();
				//int lastTime = item->GetValue(0);
				//int endTime = get_global_time() + item->GetValue(0);
				item->SetSocket(ITEM_SOCKET_UNIQUE_REMAIN_TIME, get_global_time() + item->GetValue(0));
			}
		}
	}

	switch (item->GetVnum())
	{
	case ITEM_AUTO_HP_RECOVERY_S:
	case ITEM_AUTO_HP_RECOVERY_M:
	case ITEM_AUTO_HP_RECOVERY_L:
	case ITEM_AUTO_HP_RECOVERY_X:
	case ITEM_AUTO_SP_RECOVERY_S:
	case ITEM_AUTO_SP_RECOVERY_M:
	case ITEM_AUTO_SP_RECOVERY_L:
	case ITEM_AUTO_SP_RECOVERY_X:
	case REWARD_BOX_ITEM_AUTO_SP_RECOVERY_XS:
	case REWARD_BOX_ITEM_AUTO_SP_RECOVERY_S:
	case REWARD_BOX_ITEM_AUTO_HP_RECOVERY_XS:
	case REWARD_BOX_ITEM_AUTO_HP_RECOVERY_S:
	case FUCKING_BRAZIL_ITEM_AUTO_SP_RECOVERY_S:
	case FUCKING_BRAZIL_ITEM_AUTO_HP_RECOVERY_S:
		if (bIsNewItem)
			item->SetSocket(2, item->GetValue(0), true);
		else
			item->SetSocket(2, item->GetValue(0), false);
		break;
	}

	if (item->GetType() == ITEM_ELK)
		;
	else if (item->IsStackable())
	{
		count = MINMAX(1, count, g_bItemCountLimit);

		if (bTryMagic && count <= 1 && IS_SET(item->GetFlag(), ITEM_FLAG_MAKECOUNT))
			count = item->GetValue(1);
	}
	else
		count = 1;

	item->SetVID(++m_dwVIDCount);

	if (bSkipSave == false)
		m_VIDMap.insert(ITEM_VID_MAP::value_type(item->GetVID(), item));

	if (item->GetID() != 0 && bSkipSave == false)
		m_map_pkItemByID.insert(std::map<DWORD, LPITEM>::value_type(item->GetID(), item));

	if (!item->SetCount(count))
		return NULL;

	item->SetSkipSave(false);

	if (item->GetType() == ITEM_UNIQUE && item->GetValue(2) != 0)
		item->StartUniqueExpireEvent();

	for (int i = 0; i < ITEM_LIMIT_MAX_NUM; i++)
	{
		if (LIMIT_REAL_TIME == item->GetLimitType(i))
		{
			if (item->GetLimitValue(i))
			{
				item->SetSocket(0, time(0) + item->GetLimitValue(i));
			}
			else
			{
				item->SetSocket(0, time(0) + 60 * 60 * 24 * 7);
			}

			item->StartRealTimeExpireEvent();
		}

		else if (LIMIT_TIMER_BASED_ON_WEAR == item->GetLimitType(i))
		{
			if (true == item->IsEquipped())
			{
				item->StartTimerBasedOnWearExpireEvent();
			}
			else if (0 == id)
			{
				long duration = item->GetSocket(0);
				if (0 == duration)
					duration = item->GetLimitValue(i);

				if (0 == duration)
					duration = 60 * 60 * 10;

				item->SetSocket(0, duration);
			}
		}
	}

	if (id == 0)
	{
#ifdef __ITEM_GACHA__
		if (item->GetType() == ITEM_GACHA)
			item->SetSocket(0, item->GetLimitValue(1));
#endif
		if (ITEM_BLEND == item->GetType())
		{
			if (Blend_Item_find(item->GetVnum()))
			{
				Blend_Item_set_value(item);
				return item;
			}
		}

		if (table->sAddonType)
		{
			item->ApplyAddon(table->sAddonType);
		}

		if (bTryMagic)
		{
			if (iRarePct == -1)
				iRarePct = table->bAlterToMagicItemPct;

			if (number(1, 100) <= iRarePct)
				item->AlterToMagicItem();
		}

		if (table->bGainSocketPct)
			item->AlterToSocketItem(table->bGainSocketPct);

		if (vnum == 50300 || vnum == ITEM_SKILLFORGET_VNUM)
		{
			extern const DWORD GetRandomSkillVnum(BYTE bJob = JOB_MAX_NUM);
			item->SetSocket(0, GetRandomSkillVnum());
		}
		else if (ITEM_SKILLFORGET2_VNUM == vnum)
		{
			DWORD dwSkillVnum;

			do
			{
				dwSkillVnum = number(112, 119);

				if (NULL != CSkillManager::instance().Get(dwSkillVnum))
					break;
			} while (true);

			item->SetSocket(0, dwSkillVnum);
		}
	}
	else
	{
		if (100 == table->bAlterToMagicItemPct && 0 == item->GetAttributeCount())
		{
			item->AlterToMagicItem();
		}
	}

	if (item->GetType() == ITEM_QUEST)
	{
		for (itertype(m_map_pkQuestItemGroup) it = m_map_pkQuestItemGroup.begin(); it != m_map_pkQuestItemGroup.end(); it++)
		{
			if (it->second->m_bType == CSpecialItemGroup::QUEST && it->second->Contains(vnum))
			{
				item->SetSIGVnum(it->first);
			}
		}
	}
	else if (item->GetType() == ITEM_UNIQUE)
	{
		for (itertype(m_map_pkSpecialItemGroup) it = m_map_pkSpecialItemGroup.begin(); it != m_map_pkSpecialItemGroup.end(); it++)
		{
			if (it->second->m_bType == CSpecialItemGroup::SPECIAL && it->second->Contains(vnum))
			{
				item->SetSIGVnum(it->first);
			}
		}
	}
	// @duzenleme yuzukleri special_item_group uzerinden okumuyordu bu yuzden boyle bir sey yaptik artik okuyor.
	else if (item->GetType() == ITEM_RING)
	{
		for (itertype(m_map_pkSpecialItemGroup) it = m_map_pkSpecialItemGroup.begin(); it != m_map_pkSpecialItemGroup.end(); it++)
		{
			if (it->second->m_bType == CSpecialItemGroup::SPECIAL && it->second->Contains(vnum))
			{
				item->SetSIGVnum(it->first);
			}
		}
	}

	if (item->IsDragonSoul() && 0 == id)
	{
		DSManager::instance().DragonSoulItemInitialize(item);
	}

	return item;
}

void ITEM_MANAGER::DelayedSave(LPITEM item)
{
	if (item->GetID() != 0)
		m_set_pkItemForDelayedSave.insert(item);
}

bool ITEM_MANAGER::FlushDelayedSave(LPITEM item)
{
	std::unordered_set<LPITEM>::iterator it = m_set_pkItemForDelayedSave.find(item);

	if (it == m_set_pkItemForDelayedSave.end())
	{
		// sys_log(0, "FlushDelayedSave returned false id %u, ownerid %u", item->GetID(), item->GetOwner() ? item->GetOwner()->GetPlayerID() : 0);
		return false;
	}

	m_set_pkItemForDelayedSave.erase(it);
	SaveSingleItem(item);
	return true;
}

static bool IsBossSandigi(unsigned int vnum)
{
	switch (vnum)
	{
		case 99211:
		case 99212:
		case 99213:
		case 99214:
		case 99215:
		case 99216:
		case 99217:
		case 99220:
		case 99221:
		case 99222:
		case 99223:
		case 99224:
		case 99225:
		case 99226:
		case 99227:
		case 99228:
		case 99229:
		case 99230:
		case 99231:
		case 99232:
		case 99233:
		case 99234:
		case 99235:
		case 99236:
		case 99237:
		case 99238:
		case 99239:
		case 99240:
		case 99241:
		case 99242:
		case 99243:
		case 99244:
		case 99245:
		case 99246:
		case 99247:
		case 99248:
		case 75845:
		case 75846:
		case 75847:
		case 75848:
		case 75849:
		case 75850:
			return true;
		default:
			return false;
	}

	return false;
}

void ITEM_MANAGER::SaveSingleItem(LPITEM item)
{
	if (!item->GetOwner())
	{
		DWORD dwID = item->GetID();
		DWORD dwOwnerID = item->GetLastOwnerPID();

		db_clientdesc->DBPacketHeader(HEADER_GD_ITEM_DESTROY, 0, sizeof(DWORD) + sizeof(DWORD));
		db_clientdesc->Packet(&dwID, sizeof(DWORD));
		db_clientdesc->Packet(&dwOwnerID, sizeof(DWORD));

		// sys_log(1, "ITEM_DELETE %s:%u", item->GetName(), dwID);
		return;
	}

	// sys_log(1, "ITEM_SAVE %s:%d in %s window %d", item->GetName(), item->GetID(), item->GetOwner()->GetName(), item->GetWindow());

	TPlayerItem t;

	t.id = item->GetID();
	t.window = item->GetWindow();
	switch (t.window)
	{
	case EQUIPMENT:
		t.pos = item->GetCell() - INVENTORY_MAX_NUM;
		break;
#ifdef __MARTY_BELT_INVENTORY__
	case INVENTORY:
		if (BELT_INVENTORY_SLOT_START <= item->GetCell() && BELT_INVENTORY_SLOT_END > item->GetCell())
		{
			t.window = BELT_INVENTORY;
			t.pos = item->GetCell() - BELT_INVENTORY_SLOT_START;
			break;
		}
#endif
	default:
		t.pos = item->GetCell();
		break;
	}
	t.count = item->GetCount();
	t.vnum = item->GetOriginalVnum();
	switch (t.window)
	{
	case SAFEBOX:
	case MALL:
		t.owner = item->GetOwner()->GetDesc()->GetAccountTable().id;
		break;
	default:
		t.owner = item->GetOwner()->GetPlayerID();
		break;
	}
	thecore_memcpy(t.alSockets, item->GetSockets(), sizeof(t.alSockets));
	thecore_memcpy(t.aAttr, item->GetAttributes(), sizeof(t.aAttr));
#ifdef __BEGINNER_ITEM__
	t.is_basic = item->IsBasicItem();
#endif
#ifdef __ITEM_EVOLUTION__
	t.evolution = item->GetEvolution();
#endif
#ifdef __ITEM_CHANGELOOK__
	t.transmutation = item->GetTransmutation();
#endif
	db_clientdesc->DBPacketHeader(HEADER_GD_ITEM_SAVE, 0, sizeof(TPlayerItem));
	db_clientdesc->Packet(&t, sizeof(TPlayerItem));
}

void ITEM_MANAGER::Update()
{
	std::unordered_set<LPITEM>::iterator it = m_set_pkItemForDelayedSave.begin();
	std::unordered_set<LPITEM>::iterator this_it;

	while (it != m_set_pkItemForDelayedSave.end())
	{
		this_it = it++;
		LPITEM item = *this_it;

		if (!item) // @duzenleme eger olmayan esyayi kaydetmeye kalkarsa diye koyduk fakat olmayan esyada burasinin donmemesi lazim.
			continue;

		if (item->GetOwner() && IS_SET(item->GetFlag(), ITEM_FLAG_SLOW_QUERY))
			continue;

		SaveSingleItem(item);
		m_set_pkItemForDelayedSave.erase(this_it);
	}
}

void ITEM_MANAGER::RemoveItem(LPITEM item, const char* c_pszReason)
{
	LPCHARACTER o;

	if ((o = item->GetOwner()))
	{
		// SAFEBOX_TIME_LIMIT_ITEM_BUG_FIX
		if (item->GetWindow() == MALL || item->GetWindow() == SAFEBOX)
		{
			CSafebox* pSafebox = item->GetWindow() == MALL ? o->GetMall() : o->GetSafebox();
			if (pSafebox)
			{
				pSafebox->Remove(item->GetCell());
			}
		}
		// END_OF_SAFEBOX_TIME_LIMIT_ITEM_BUG_FIX
		else
		{
#ifdef __ADDITIONAL_INVENTORY__
			if (!item->IsDragonSoul() && !item->IsUpgradeItem() && !item->IsBook() && !item->IsStone() && !item->IsFlower() && !item->IsAttrItem() && !item->IsChest())
#endif
				o->SyncQuickslot(QUICKSLOT_TYPE_ITEM, item->GetCell(), 255);
			item->RemoveFromCharacter();
		}
	}

	M2_DESTROY_ITEM(item);
}

#ifndef DEBUG_ALLOC
void ITEM_MANAGER::DestroyItem(LPITEM item)
#else
void ITEM_MANAGER::DestroyItem(LPITEM item, const char* file, size_t line)
#endif
{
	if (item->GetSectree())
		item->RemoveFromGround();

	if (item->GetOwner())
	{
		if (CHARACTER_MANAGER::instance().Find(item->GetOwner()->GetPlayerID()) != NULL)
		{
			sys_err("DestroyItem (%lu): GetOwner %s %s!!", item->GetID(), item->GetName(), item->GetOwner()->GetName());
			item->RemoveFromCharacter();
		}
		else
		{
			sys_err("Item (%lu): WTH! Invalid item owner. owner pointer : %p", item->GetID(), item->GetOwner());
		}
	}

	std::unordered_set<LPITEM>::iterator it = m_set_pkItemForDelayedSave.find(item);

	if (it != m_set_pkItemForDelayedSave.end())
		m_set_pkItemForDelayedSave.erase(it);

	if (test_server)
		sys_log(0, "ITEM_MANAGER::DestroyItem: ptr %p id %u vnum %u name %s count %u", item, item->GetID(), item->GetVnum(), item->GetName(), item->GetCount());

	DWORD dwID = item->GetID();
	sys_log(2, "ITEM_DESTROY %s:%u", item->GetName(), dwID);

	if (!item->GetSkipSave() && dwID)
	{
		DWORD dwOwnerID = item->GetLastOwnerPID();

		db_clientdesc->DBPacketHeader(HEADER_GD_ITEM_DESTROY, 0, sizeof(DWORD) + sizeof(DWORD));
		db_clientdesc->Packet(&dwID, sizeof(DWORD));
		db_clientdesc->Packet(&dwOwnerID, sizeof(DWORD));
	}
	else
	{
		sys_log(2, "ITEM_DESTROY_SKIP %s:%u (skip=%d)", item->GetName(), dwID, item->GetSkipSave());
	}

	if (dwID)
		m_map_pkItemByID.erase(dwID);

	m_VIDMap.erase(item->GetVID());

#ifndef DEBUG_ALLOC
	M2_DELETE(item);
#else
	M2_DELETE_EX(item, file, line);
#endif
}

LPITEM ITEM_MANAGER::Find(DWORD id)
{
	itertype(m_map_pkItemByID) it = m_map_pkItemByID.find(id);
	if (it == m_map_pkItemByID.end())
		return NULL;
	return it->second;
}

LPITEM ITEM_MANAGER::FindByVID(DWORD vid)
{
	ITEM_VID_MAP::iterator it = m_VIDMap.find(vid);

	if (it == m_VIDMap.end())
		return NULL;

	return (it->second);
}

TItemTable* ITEM_MANAGER::GetTable(DWORD vnum)
{
	int rnum = RealNumber(vnum);

	if (rnum < 0)
	{
		for (size_t i = 0; i < m_vec_item_vnum_range_info.size(); i++)
		{
			TItemTable* p = m_vec_item_vnum_range_info[i];
			if ((p->dwVnum < vnum) &&
				vnum < (p->dwVnum + p->dwVnumRange))
			{
				return p;
			}
		}

		return NULL;
	}

	return &m_vec_prototype[rnum];
}

int ITEM_MANAGER::RealNumber(DWORD vnum)
{
	int bot, top, mid;

	bot = 0;
	top = m_vec_prototype.size();

	TItemTable* pTable = &m_vec_prototype[0];

	while (1)
	{
		mid = (bot + top) >> 1;

		if ((pTable + mid)->dwVnum == vnum)
			return (mid);

		if (bot >= top)
			return (-1);

		if ((pTable + mid)->dwVnum > vnum)
			top = mid - 1;
		else
			bot = mid + 1;
	}
}

bool ITEM_MANAGER::GetVnum(const char* c_pszName, DWORD& r_dwVnum)
{
	int len = strlen(c_pszName);

	TItemTable* pTable = &m_vec_prototype[0];

	for (DWORD i = 0; i < m_vec_prototype.size(); ++i, ++pTable)
	{
		if (!strncasecmp(c_pszName, pTable->szLocaleName, len))
		{
			r_dwVnum = pTable->dwVnum;
			return true;
		}
	}

	return false;
}

bool ITEM_MANAGER::GetVnumByOriginalName(const char* c_pszName, DWORD& r_dwVnum)
{
	int len = strlen(c_pszName);

	TItemTable* pTable = &m_vec_prototype[0];

	for (DWORD i = 0; i < m_vec_prototype.size(); ++i, ++pTable)
	{
		if (!strncasecmp(c_pszName, pTable->szName, len))
		{
			r_dwVnum = pTable->dwVnum;
			return true;
		}
	}

	return false;
}

class CItemDropInfo
{
public:
	CItemDropInfo(int iLevelStart, int iLevelEnd, int iPercent, DWORD dwVnum) :
		m_iLevelStart(iLevelStart), m_iLevelEnd(iLevelEnd), m_iPercent(iPercent), m_dwVnum(dwVnum)
	{
	}

	int	m_iLevelStart;
	int	m_iLevelEnd;
	int	m_iPercent; // 1 ~ 1000
	DWORD	m_dwVnum;

	friend bool operator < (const CItemDropInfo& l, const CItemDropInfo& r)
	{
		return l.m_iLevelEnd < r.m_iLevelEnd;
	}
};

extern std::vector<CItemDropInfo> g_vec_pkCommonDropItem[MOB_RANK_MAX_NUM];

// 20050503.ipkn.

int GetDropPerKillPct(int iMinimum, int iDefault, int iDeltaPercent, const char* c_pszFlag)
{
	int iVal = 0;

	if ((iVal = quest::CQuestManager::instance().GetEventFlag(c_pszFlag)))
	{
		if (!test_server)
		{
			if (iVal < iMinimum)
				iVal = iDefault;

			if (iVal < 0)
				iVal = iDefault;
		}
	}

	if (iVal == 0)
		return 0;

	return (40000 * iDeltaPercent / iVal);
}

bool ITEM_MANAGER::GetDropPct(LPCHARACTER pkChr, LPCHARACTER pkKiller, OUT int& iDeltaPercent, OUT int& iRandRange)
{
	if (NULL == pkChr || NULL == pkKiller)
		return false;

	int iLevel = pkKiller->GetLevel();
	iDeltaPercent = 100;

	if (!pkChr->IsStone() && pkChr->GetMobRank() >= MOB_RANK_BOSS)
		iDeltaPercent = PERCENT_LVDELTA_BOSS(pkKiller->GetLevel(), pkChr->GetLevel());
	else
		iDeltaPercent = PERCENT_LVDELTA(pkKiller->GetLevel(), pkChr->GetLevel());

	BYTE bRank = pkChr->GetMobRank();

	if (1 == number(1, 50000))
		iDeltaPercent += 1000;
	else if (1 == number(1, 10000))
		iDeltaPercent += 500;

	sys_log(3, "CreateDropItem for level: %d rank: %u pct: %d", iLevel, bRank, iDeltaPercent);
	iDeltaPercent = iDeltaPercent * CHARACTER_MANAGER::instance().GetMobItemRate(pkKiller) / 100;

	// ADD_PREMIUM
	if (pkKiller->GetPremiumRemainSeconds(PREMIUM_ITEM) > 0 ||
		pkKiller->IsEquipUniqueGroup(UNIQUE_GROUP_DOUBLE_ITEM))
		iDeltaPercent += iDeltaPercent;
	// END_OF_ADD_PREMIUM

	iRandRange = 4000000;
	iRandRange = iRandRange * 100 /
		(100 +
			CPrivManager::instance().GetPriv(pkKiller, PRIV_ITEM_DROP) +
			(pkKiller->IsEquipUniqueItem(UNIQUE_ITEM_DOUBLE_ITEM) ? 100 : 0));

	return true;
}

bool ITEM_MANAGER::CreateDropItem(LPCHARACTER pkChr, LPCHARACTER pkKiller, std::vector<LPITEM>& vec_item)
{
	int iLevel = pkKiller->GetLevel();

	// Scp1453 level korumasý
	if ((pkChr->IsStone() || pkChr->IsBoss()) && pkKiller->GetLevel() > pkChr->GetLevel())
	{
		pkKiller->ChatPacket(CHAT_TYPE_INFO, "Bu hesabýnýz canavar seviyesinden yüksek olduðu için eþya düþüremez!");
		return false;
	}


	int iDeltaPercent, iRandRange;
	if (!GetDropPct(pkChr, pkKiller, iDeltaPercent, iRandRange))
		return false;

	BYTE bRank = pkChr->GetMobRank();
	LPITEM item = NULL;

	// Common Drop Items
	std::vector<CItemDropInfo>::iterator it = g_vec_pkCommonDropItem[bRank].begin();

	while (it != g_vec_pkCommonDropItem[bRank].end())
	{
		const CItemDropInfo& c_rInfo = *(it++);

		if (iLevel < c_rInfo.m_iLevelStart || iLevel > c_rInfo.m_iLevelEnd)
			continue;

		int iPercent = (c_rInfo.m_iPercent * iDeltaPercent) / 100;
		sys_log(3, "CreateDropItem %d ~ %d %d(%d)", c_rInfo.m_iLevelStart, c_rInfo.m_iLevelEnd, c_rInfo.m_dwVnum, iPercent, c_rInfo.m_iPercent);

		if (iPercent >= number(1, iRandRange))
		{
			TItemTable* table = GetTable(c_rInfo.m_dwVnum);

			if (!table)
				continue;

			item = NULL;
			DWORD wMultipler = 1;
			if (CGameEventsManager::instance().IsActivateEvent(EVENT_DOUBLEMETIN) == true && pkChr->IsStone())
				wMultipler *= 2;
			if (IsBossSandigi(c_rInfo.m_dwVnum) && CGameEventsManager::instance().IsActivateEvent(EVENT_DOUBLEBOSS) == true && (pkChr->IsBoss()))
				wMultipler *= 2;
			item = CreateItem(c_rInfo.m_dwVnum, wMultipler, 0, true);

			if (item)
				vec_item.push_back(item);
		}
	}

	// Drop Item Group
	{
		itertype(m_map_pkDropItemGroup) it;
		it = m_map_pkDropItemGroup.find(pkChr->GetRaceNum());

		if (it != m_map_pkDropItemGroup.end())
		{
			typeof(it->second->GetVector()) v = it->second->GetVector();

			for (DWORD i = 0; i < v.size(); ++i)
			{
				int iPercent = v[i].dwPct;

				if (iPercent >= number(1, 100))
				{
					DWORD wMultipler = v[i].iCount;
					if (CGameEventsManager::instance().IsActivateEvent(EVENT_DOUBLEMETIN) == true && pkChr->IsStone())
						wMultipler *= 2;
					if (IsBossSandigi(v[i].dwVnum) && CGameEventsManager::instance().IsActivateEvent(EVENT_DOUBLEBOSS) == true && (pkChr->IsBoss()))
						wMultipler *= 2;
					item = CreateItem(v[i].dwVnum, wMultipler, 0, true);

					if (item)
						vec_item.push_back(item);
				}
			}
		}
	}

	// MobDropItem Group
	{
		itertype(m_map_pkMobItemGroup) it;
		it = m_map_pkMobItemGroup.find(pkChr->GetRaceNum());

		if (it != m_map_pkMobItemGroup.end())
		{
			CMobItemGroup* pGroup = it->second;

			// MOB_DROP_ITEM_BUG_FIX

			if (pGroup && !pGroup->IsEmpty())
			{
				int iPercent = 40000 * iDeltaPercent / pGroup->GetKillPerDrop();
				if (iPercent >= number(1, iRandRange))
				{
					const CMobItemGroup::SMobItemGroupInfo& info = pGroup->GetOne();
					DWORD wMultipler = info.iCount;
					if (CGameEventsManager::instance().IsActivateEvent(EVENT_DOUBLEMETIN) == true && pkChr->IsStone())
						wMultipler *= 2;
					if (IsBossSandigi(info.dwItemVnum) && CGameEventsManager::instance().IsActivateEvent(EVENT_DOUBLEBOSS) == true && (pkChr->IsBoss()))
						wMultipler *= 2;
					item = CreateItem(info.dwItemVnum, wMultipler, 0, true, info.iRarePct);

					if (item)
						vec_item.push_back(item);
				}
			}
			// END_OF_MOB_DROP_ITEM_BUG_FIX
		}
	}

	// Level Item Group
	{
		itertype(m_map_pkLevelItemGroup) it;
		it = m_map_pkLevelItemGroup.find(pkChr->GetRaceNum());

		if (it != m_map_pkLevelItemGroup.end())
		{
			if (it->second->GetLevelLimit() <= (DWORD)iLevel)
			{
				typeof(it->second->GetVector()) v = it->second->GetVector();

				for (DWORD i = 0; i < v.size(); i++)
				{
					if (v[i].dwPct >= (DWORD)number(1, 1000000/*iRandRange*/))
					{
						DWORD dwVnum = v[i].dwVNum;
						DWORD wMultipler = v[i].iCount;
						if (CGameEventsManager::instance().IsActivateEvent(EVENT_DOUBLEMETIN) == true && pkChr->IsStone())
							wMultipler *= 2;
						if (IsBossSandigi(dwVnum) && CGameEventsManager::instance().IsActivateEvent(EVENT_DOUBLEBOSS) == true && (pkChr->IsBoss()))
							wMultipler *= 2;
						item = CreateItem(dwVnum, wMultipler, 0, true);
						if (item)
							vec_item.push_back(item);
					}
				}
			}
		}
	}

	// BuyerTheitGloves Item Group
	{
		if (pkKiller->GetPremiumRemainSeconds(PREMIUM_ITEM) > 0 ||
			pkKiller->IsEquipUniqueGroup(UNIQUE_GROUP_DOUBLE_ITEM))
		{
			itertype(m_map_pkGloveItemGroup) it;
			it = m_map_pkGloveItemGroup.find(pkChr->GetRaceNum());

			if (it != m_map_pkGloveItemGroup.end())
			{
				typeof(it->second->GetVector()) v = it->second->GetVector();

				for (DWORD i = 0; i < v.size(); ++i)
				{
					int iPercent = (v[i].dwPct * iDeltaPercent) / 100;

					if (iPercent >= number(1, iRandRange))
					{
						DWORD dwVnum = v[i].dwVnum;
						DWORD wMultipler = v[i].iCount;
						if (CGameEventsManager::instance().IsActivateEvent(EVENT_DOUBLEMETIN) == true && pkChr->IsStone())
							wMultipler *= 2;
						if (IsBossSandigi(dwVnum) && CGameEventsManager::instance().IsActivateEvent(EVENT_DOUBLEBOSS) == true && (pkChr->IsBoss()))
							wMultipler *= 2;

						item = CreateItem(dwVnum, wMultipler, 0, true);
						if (item)
							vec_item.push_back(item);
					}
				}
			}
		}
	}

	if (pkChr->GetMobDropItemVnum())
	{
		itertype(m_map_dwEtcItemDropProb) it = m_map_dwEtcItemDropProb.find(pkChr->GetMobDropItemVnum());

		if (it != m_map_dwEtcItemDropProb.end())
		{
			int iPercent = (it->second * iDeltaPercent) / 100;

			if (iPercent >= number(1, iRandRange))
			{
				DWORD wMultipler = 1;
				if (CGameEventsManager::instance().IsActivateEvent(EVENT_DOUBLEMETIN) == true && pkChr->IsStone())
					wMultipler *= 2;
				if (IsBossSandigi(pkChr->GetMobDropItemVnum()) && CGameEventsManager::instance().IsActivateEvent(EVENT_DOUBLEBOSS) == true && (pkChr->IsBoss()))
					wMultipler *= 2;

				item = CreateItem(pkChr->GetMobDropItemVnum(), wMultipler, 0, true);
				if (item)
					vec_item.push_back(item);
			}
		}
	}

	if (pkChr->IsStone())
	{
		if (pkChr->GetDropMetinStoneVnum())
		{
			int iPercent = (pkChr->GetDropMetinStonePct() * iDeltaPercent) * 400;

			if (iPercent >= number(1, iRandRange))
			{
				item = CreateItem(pkChr->GetDropMetinStoneVnum(), 1, 0, true);
				if (item)
					vec_item.push_back(item);
			}
		}
	}

	if (pkKiller->IsHorseRiding() &&
		GetDropPerKillPct(1000, 1000000, iDeltaPercent, "horse_skill_book_drop") >= number(1, iRandRange))
	{
		sys_log(0, "EVENT HORSE_SKILL_BOOK_DROP");

		if ((item = CreateItem(ITEM_HORSE_SKILL_TRAIN_BOOK, 1, 0, true)))
			vec_item.push_back(item);
	}

	//
	CreateQuestDropItem(pkChr, pkKiller, vec_item, iDeltaPercent, iRandRange);
	return vec_item.size();
}

// ADD_GRANDMASTER_SKILL
int GetThreeSkillLevelAdjust(int level)
{
	if (level < 40)
		return 32;
	if (level < 45)
		return 16;
	if (level < 50)
		return 8;
	if (level < 55)
		return 4;
	if (level < 60)
		return 2;
	return 1;
}
// END_OF_ADD_GRANDMASTER_SKILL

// DROPEVENT_CHARSTONE
// drop_char_stone 1
// drop_char_stone.percent_lv01_10 5
// drop_char_stone.percent_lv11_30 10
// drop_char_stone.percent_lv31_MX 15
// drop_char_stone.level_range	   10
static struct DropEvent_CharStone
{
	int percent_lv01_10;
	int percent_lv11_30;
	int percent_lv31_MX;
	int level_range;
	bool alive;

	DropEvent_CharStone()
	{
		percent_lv01_10 = 100;
		percent_lv11_30 = 200;
		percent_lv31_MX = 300;
		level_range = 10;
		alive = false;
	}
} gs_dropEvent_charStone;

static int __DropEvent_CharStone_GetDropPercent(int killer_level)
{
	int killer_levelStep = (killer_level - 1) / 10;

	switch (killer_levelStep)
	{
	case 0:
		return gs_dropEvent_charStone.percent_lv01_10;

	case 1:
	case 2:
		return gs_dropEvent_charStone.percent_lv11_30;
	}

	return gs_dropEvent_charStone.percent_lv31_MX;
}

static void __DropEvent_CharStone_DropItem(CHARACTER& killer, CHARACTER& victim, ITEM_MANAGER& itemMgr, std::vector<LPITEM>& vec_item)
{
	if (!gs_dropEvent_charStone.alive)
		return;

	int killer_level = killer.GetLevel();
	int dropPercent = __DropEvent_CharStone_GetDropPercent(killer_level);

	int MaxRange = 10000;

	if (number(1, MaxRange) <= dropPercent)
	{
#ifdef __GAME_MASTER_UTILITY__
		int log_level = (test_server || killer.GetGMLevel() >= GM_HUNTER) ? 0 : 1;
#else
		int log_level = (test_server || killer.GetGMLevel() >= GM_LOW_WIZARD) ? 0 : 1;
#endif
		int victim_level = victim.GetLevel();
		int level_diff = victim_level - killer_level;

		if (level_diff >= +gs_dropEvent_charStone.level_range || level_diff <= -gs_dropEvent_charStone.level_range)
		{
			sys_log(log_level,
				"dropevent.drop_char_stone.level_range_over: killer(%s: lv%d), victim(%s: lv:%d), level_diff(%d)",
				killer.GetName(), killer.GetLevel(), victim.GetName(), victim.GetLevel(), level_diff);
			return;
		}

		static const int Stones[] = { 30210, 30211, 30212, 30213, 30214, 30215, 30216, 30217, 30218, 30219, 30258, 30259, 30260, 30261, 30262, 30263 };
		int item_vnum = Stones[number(0, _countof(Stones))];

		LPITEM p_item = NULL;

		if ((p_item = itemMgr.CreateItem(item_vnum, 1, 0, true)))
		{
			vec_item.push_back(p_item);

			sys_log(log_level,
				"dropevent.drop_char_stone.item_drop: killer(%s: lv%d), victim(%s: lv:%d), item_name(%s)",
				killer.GetName(), killer.GetLevel(), victim.GetName(), victim.GetLevel(), p_item->GetName());
		}
	}
}

bool DropEvent_CharStone_SetValue(const std::string& name, int value)
{
	if (name == "drop_char_stone")
	{
		gs_dropEvent_charStone.alive = value;

		if (value)
			sys_log(0, "dropevent.drop_char_stone = on");
		else
			sys_log(0, "dropevent.drop_char_stone = off");
	}
	else if (name == "drop_char_stone.percent_lv01_10")
		gs_dropEvent_charStone.percent_lv01_10 = value;
	else if (name == "drop_char_stone.percent_lv11_30")
		gs_dropEvent_charStone.percent_lv11_30 = value;
	else if (name == "drop_char_stone.percent_lv31_MX")
		gs_dropEvent_charStone.percent_lv31_MX = value;
	else if (name == "drop_char_stone.level_range")
		gs_dropEvent_charStone.level_range = value;
	else
		return false;

	sys_log(0, "dropevent.drop_char_stone: %d", gs_dropEvent_charStone.alive ? true : false);
	sys_log(0, "dropevent.drop_char_stone.percent_lv01_10: %f", gs_dropEvent_charStone.percent_lv01_10 / 100.0f);
	sys_log(0, "dropevent.drop_char_stone.percent_lv11_30: %f", gs_dropEvent_charStone.percent_lv11_30 / 100.0f);
	sys_log(0, "dropevent.drop_char_stone.percent_lv31_MX: %f", gs_dropEvent_charStone.percent_lv31_MX / 100.0f);
	sys_log(0, "dropevent.drop_char_stone.level_range: %d", gs_dropEvent_charStone.level_range);

	return true;
}

// END_OF_DROPEVENT_CHARSTONE

static struct DropEvent_RefineBox
{
	int percent_low;
	int low;
	int percent_mid;
	int mid;
	int percent_high;
	//int level_range;
	bool alive;

	DropEvent_RefineBox()
	{
		percent_low = 100;
		low = 20;
		percent_mid = 100;
		mid = 45;
		percent_high = 100;
		//level_range = 10;
		alive = false;
	}
} gs_dropEvent_refineBox;

static LPITEM __DropEvent_RefineBox_GetDropItem(CHARACTER& killer, CHARACTER& victim, ITEM_MANAGER& itemMgr)
{
	static const int lowerBox[] = { 50197, 50198, 50199 };
	static const int lowerBox_range = 3;
	static const int midderBox[] = { 50203, 50204, 50205, 50206 };
	static const int midderBox_range = 4;
	static const int higherBox[] = { 50207, 50208, 50209, 50210, 50211 };
	static const int higherBox_range = 5;

	if (victim.GetMobRank() < MOB_RANK_KNIGHT)
		return NULL;

	int killer_level = killer.GetLevel();
	//int level_diff = victim_level - killer_level;

	//if (level_diff >= +gs_dropEvent_refineBox.level_range || level_diff <= -gs_dropEvent_refineBox.level_range)
	//{
	//	sys_log(log_level,
	//		"dropevent.drop_refine_box.level_range_over: killer(%s: lv%d), victim(%s: lv:%d), level_diff(%d)",
	//		killer.GetName(), killer.GetLevel(), victim.GetName(), victim.GetLevel(), level_diff);
	//	return NULL;
	//}

	if (killer_level <= gs_dropEvent_refineBox.low)
	{
		if (number(1, gs_dropEvent_refineBox.percent_low) == 1)
		{
			return itemMgr.CreateItem(lowerBox[number(1, lowerBox_range) - 1], 1, 0, true);
		}
	}
	else if (killer_level <= gs_dropEvent_refineBox.mid)
	{
		if (number(1, gs_dropEvent_refineBox.percent_mid) == 1)
		{
			return itemMgr.CreateItem(midderBox[number(1, midderBox_range) - 1], 1, 0, true);
		}
	}
	else
	{
		if (number(1, gs_dropEvent_refineBox.percent_high) == 1)
		{
			return itemMgr.CreateItem(higherBox[number(1, higherBox_range) - 1], 1, 0, true);
		}
	}
	return NULL;
}

static void __DropEvent_RefineBox_DropItem(CHARACTER& killer, CHARACTER& victim, ITEM_MANAGER& itemMgr, std::vector<LPITEM>& vec_item)
{
	if (!gs_dropEvent_refineBox.alive)
		return;

#ifdef __GAME_MASTER_UTILITY__
	int log_level = (test_server || killer.GetGMLevel() >= GM_HUNTER) ? 0 : 1;
#else
	int log_level = (test_server || killer.GetGMLevel() >= GM_LOW_WIZARD) ? 0 : 1;
#endif

	LPITEM p_item = __DropEvent_RefineBox_GetDropItem(killer, victim, itemMgr);

	if (p_item)
	{
		vec_item.push_back(p_item);

		sys_log(log_level,
			"dropevent.drop_refine_box.item_drop: killer(%s: lv%d), victim(%s: lv:%d), item_name(%s)",
			killer.GetName(), killer.GetLevel(), victim.GetName(), victim.GetLevel(), p_item->GetName());
	}
}

bool DropEvent_RefineBox_SetValue(const std::string& name, int value)
{
	if (name == "refine_box_drop")
	{
		gs_dropEvent_refineBox.alive = value;

		if (value)
			sys_log(0, "refine_box_drop = on");
		else
			sys_log(0, "refine_box_drop = off");
	}
	else if (name == "refine_box_low")
		gs_dropEvent_refineBox.percent_low = value < 100 ? 100 : value;
	else if (name == "refine_box_mid")
		gs_dropEvent_refineBox.percent_mid = value < 100 ? 100 : value;
	else if (name == "refine_box_high")
		gs_dropEvent_refineBox.percent_high = value < 100 ? 100 : value;
	//else if (name == "refine_box_level_range")
	//	gs_dropEvent_refineBox.level_range = value;
	else
		return false;

	sys_log(0, "refine_box_drop: %d", gs_dropEvent_refineBox.alive ? true : false);
	sys_log(0, "refine_box_low: %d", gs_dropEvent_refineBox.percent_low);
	sys_log(0, "refine_box_mid: %d", gs_dropEvent_refineBox.percent_mid);
	sys_log(0, "refine_box_high: %d", gs_dropEvent_refineBox.percent_high);
	//sys_log(0, "refine_box_low_level_range: %d", gs_dropEvent_refineBox.level_range);

	return true;
}

void ITEM_MANAGER::CreateQuestDropItem(LPCHARACTER pkChr, LPCHARACTER pkKiller, std::vector<LPITEM>& vec_item, int iDeltaPercent, int iRandRange)
{
	LPITEM item = NULL;

	if (!pkChr)
		return;

	if (!pkKiller)
		return;

	sys_log(1, "CreateQuestDropItem victim(%s), killer(%s)", pkChr->GetName(), pkKiller->GetName());

	// DROPEVENT_CHARSTONE
	__DropEvent_CharStone_DropItem(*pkKiller, *pkChr, *this, vec_item);
	// END_OF_DROPEVENT_CHARSTONE
	__DropEvent_RefineBox_DropItem(*pkKiller, *pkChr, *this, vec_item);

#ifdef __EVENT_SYSTEM__
	if (quest::CQuestManager::instance().GetEventFlag("monday_event"))
	{
		switch (pkChr->GetRaceNum())
		{
			case 8817:
			case 8815:
			case 8819:
			case 8816:
			case 8818:
			case 8442:
			case 8061:
			case 8408:
			case 8483:
			case 8407:
			case 8835:
			case 8836:
			case 8837:
			case 8443:
			case 8469:
			case 8456:
			case 8414:
			case 8450:
			case 8406:
			case 8479:
			case 8461:
				if (number(1, 100) <= 30)
				{
					if ((item = CreateItem(817, 1, 0, true)))
						vec_item.push_back(item);
				}
				break;
		}
	}
#endif
}

DWORD ITEM_MANAGER::GetRefineFromVnum(DWORD dwVnum)
{
	itertype(m_map_ItemRefineFrom) it = m_map_ItemRefineFrom.find(dwVnum);
	if (it != m_map_ItemRefineFrom.end())
		return it->second;
	return 0;
}

const CSpecialItemGroup* ITEM_MANAGER::GetSpecialItemGroup(DWORD dwVnum)
{
	itertype(m_map_pkSpecialItemGroup) it = m_map_pkSpecialItemGroup.find(dwVnum);
	if (it != m_map_pkSpecialItemGroup.end())
	{
		return it->second;
	}
	return NULL;
}

const CSpecialAttrGroup* ITEM_MANAGER::GetSpecialAttrGroup(DWORD dwVnum)
{
	itertype(m_map_pkSpecialAttrGroup) it = m_map_pkSpecialAttrGroup.find(dwVnum);
	if (it != m_map_pkSpecialAttrGroup.end())
	{
		return it->second;
	}
	return NULL;
}

DWORD ITEM_MANAGER::GetMaskVnum(DWORD dwVnum)
{
	TMapDW2DW::iterator it = m_map_new_to_ori.find(dwVnum);
	if (it != m_map_new_to_ori.end())
	{
		return it->second;
	}
	else
		return 0;
}

void ITEM_MANAGER::CopyAllAttrTo(LPITEM pkOldItem, LPITEM pkNewItem)
{
	// ACCESSORY_REFINE
	if (pkOldItem->IsAccessoryForSocket())
	{
		for (int i = 0; i < ITEM_SOCKET_MAX_NUM; ++i)
		{
			pkNewItem->SetSocket(i, pkOldItem->GetSocket(i));
		}
		//pkNewItem->StartAccessorySocketExpireEvent();
	}
	// END_OF_ACCESSORY_REFINE
	else
	{
		for (int i = 0; i < ITEM_SOCKET_MAX_NUM; ++i)
		{
			if (!pkOldItem->GetSocket(i))
				break;
			else
				pkNewItem->SetSocket(i, 1);
		}

		int slot = 0;

		for (int i = 0; i < ITEM_SOCKET_MAX_NUM; ++i)
		{
			long socket = pkOldItem->GetSocket(i);
			const int ITEM_BROKEN_METIN_VNUM = 28960;
			if (socket > 2 && socket != ITEM_BROKEN_METIN_VNUM)
				pkNewItem->SetSocket(slot++, socket);
		}
	}

#ifdef __ITEM_EVOLUTION__
	pkNewItem->SetEvolution(pkOldItem->GetEvolution());
#endif
	pkOldItem->CopyAttributeTo(pkNewItem);
}
#ifdef __CHEST_INFO_SYSTEM__
#include "../../libgame/include/grid.h"
void ITEM_MANAGER::GetChestItemList(const DWORD& dwChestVnum, std::vector<TChestDropInfoTable>& vec_item)
{
	const auto pGroup = GetSpecialItemGroup(dwChestVnum);
	if (!pGroup)
		return;
	std::vector<std::unique_ptr<CGrid>> pGrids;
	pGrids.emplace_back(std::make_unique<CGrid>(20, 5));
	for (int i = 0; i < pGroup->GetGroupSize(); i++) {
		const auto itemTable = GetTable(pGroup->GetVnum(i));
		//pass if exp, yang or duplicate item
		if (!itemTable || !itemTable->dwVnum || itemTable->dwVnum < 3)
			continue;
		bool PushNewGrid = true;
		for (size_t iPage = 0; iPage < pGrids.size(); iPage++) {
			const auto iPos = pGrids.at(iPage)->FindBlank(1, itemTable->bSize);
			if (iPos >= 0) {
				pGrids.at(iPage)->Put(iPos, 1, itemTable->bSize);
#ifdef __ITEM_COUNT_LIMIT__
				vec_item.push_back({ static_cast<BYTE>(iPage + 1), static_cast<BYTE>(iPos), static_cast<DWORD>(pGroup->GetVnum(i)), static_cast<DWORD>(pGroup->GetCount(i)) });
#else
				vec_item.push_back({ static_cast<BYTE>(iPage + 1), static_cast<BYTE>(iPos), static_cast<DWORD>(pGroup->GetVnum(i)), static_cast<BYTE>(pGroup->GetCount(i)) });
#endif
				PushNewGrid = false;
				break;
			}
		}
		if (PushNewGrid) {
			pGrids.emplace_back(std::make_unique<CGrid>(20, 5));
			i--;//again
		}
	}
	//sort by itemvnum..
	std::sort(vec_item.begin(), vec_item.end(), [](const TChestDropInfoTable& a, const TChestDropInfoTable& b) { return a.dwItemVnum < b.dwItemVnum; });
}
#endif

#ifdef __TARGET_BOARD_RENEWAL__
bool ITEM_MANAGER::CreateDropItemVector(LPCHARACTER pkChr, LPCHARACTER pkKiller, std::vector<TMobInformationDropItem>& vec_item)
{
	if (pkChr->IsPC() || pkChr->IsDoor() || pkChr->IsWarp() || pkChr->IsGoto())
		return false;

	auto it = m_map_pkDropItemGroup.find(pkChr->GetRaceNum());
	if (it != m_map_pkDropItemGroup.end())
	{
		typeof(it->second->GetVector()) v = it->second->GetVector();
		for (DWORD i = 0; i < v.size(); ++i)
		{
			vec_item.push_back({v[i].dwVnum, v[i].iCount});
		}
	}

	auto it2 = m_map_pkMobItemGroup.find(pkChr->GetRaceNum());
	if (it2 != m_map_pkMobItemGroup.end())
	{
		CMobItemGroup* pGroup = it2->second;
		if (pGroup && !pGroup->IsEmpty())
		{
			const CMobItemGroup::SMobItemGroupInfo& info = pGroup->GetOne();
			vec_item.push_back({info.dwItemVnum, info.iCount});
		}
	}

	auto it3 = m_map_pkLevelItemGroup.find(pkChr->GetRaceNum());
	if (it3 != m_map_pkLevelItemGroup.end())
	{
		typeof(it3->second->GetVector()) v = it3->second->GetVector();
		for (DWORD i = 0; i < v.size(); i++)
		{
			vec_item.push_back({v[i].dwVNum, v[i].iCount});
		}
	}

	auto it4 = m_map_pkGloveItemGroup.find(pkChr->GetRaceNum());
	if (it4 != m_map_pkGloveItemGroup.end())
	{
		typeof(it4->second->GetVector()) v = it4->second->GetVector();
		for (DWORD i = 0; i < v.size(); ++i)
		{
			vec_item.push_back({v[i].dwVnum, v[i].iCount});
		}
	}

	if (pkChr->GetMobDropItemVnum())
	{
		auto it5 = m_map_dwEtcItemDropProb.find(pkChr->GetMobDropItemVnum());
		if (it5 != m_map_dwEtcItemDropProb.end())
		{
			vec_item.push_back({pkChr->GetMobDropItemVnum(), 1});
		}
	}

	return true;
}
#endif
