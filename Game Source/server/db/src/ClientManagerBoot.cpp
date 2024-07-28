// vim:ts=4 sw=4
#include <map>
#include "stdafx.h"
#include "ClientManager.h"
#include "Main.h"
#include "../../libgame/include/CsvReader.h"
#include "ProtoReader.h"

using namespace std;

extern int g_test_server;
extern std::string g_stLocaleNameColumn;

bool CClientManager::InitializeTables()
{
	if (!InitializeMobTable())
	{
		sys_err("InitializeMobTable FAILED");
		return false;
	}
	if (!InitializeItemTable())
	{
		sys_err("InitializeItemTable FAILED");
		return false;
	}

	if (!InitializeShopTable())
	{
		sys_err("InitializeShopTable FAILED");
		return false;
	}

	if (!InitializeSkillTable())
	{
		sys_err("InitializeSkillTable FAILED");
		return false;
	}

	if (!InitializeRefineTable())
	{
		sys_err("InitializeRefineTable FAILED");
		return false;
	}

	if (!InitializeItemAttrTable())
	{
		sys_err("InitializeItemAttrTable FAILED");
		return false;
	}

	if (!InitializeItemRareTable())
	{
		sys_err("InitializeItemRareTable FAILED");
		return false;
	}

#ifdef __OFFLINE_SHOP__
	if (!InitializeOfflineshopTable())
	{
		sys_err("InitializeOfflineshopTable FAILED");
		return false;
	}
#endif

#ifdef __BATTLE_PASS_SYSTEM__
	if (!LoadBattlePassRanking())
	{
		sys_err("LoadBattlePassRanking FAILED");
		return false;
	}
#endif
#ifdef __SCP1453_EXTENSIONS__
	DeleteDungeonRejoinTimes();
#endif

	return true;
}

#ifdef __SCP1453_EXTENSIONS__
bool CClientManager::DeleteDungeonRejoinTimes()
{
	char query[2048];

	snprintf(query, sizeof(query),
		"DELETE FROM quest%s WHERE szState='rejoin_time' or szState='rejoin_time_test'",
		GetTablePostfix());

	std::unique_ptr<SQLMsg> pkMsg(CDBManager::instance().DirectQuery(query));
	return true;
}
#endif

bool CClientManager::InitializeRefineTable()
{
	char query[2048];

	snprintf(query, sizeof(query),
		"SELECT id, cost, prob, vnum0, count0, vnum1, count1, vnum2, count2, vnum3, count3, vnum4, count4, vnum5, count5, vnum6, count6, vnum7, count7, vnum8, count8, vnum9, count9 FROM refine_proto%s",
		GetTablePostfix());

	std::unique_ptr<SQLMsg> pkMsg(CDBManager::instance().DirectQuery(query));
	SQLResult* pRes = pkMsg->Get();

	if (!pRes->uiNumRows)
		return true;

	if (m_pRefineTable)
	{
		sys_log(0, "RELOAD: refine_proto");
		delete[] m_pRefineTable;
		m_pRefineTable = NULL;
	}

	m_iRefineTableSize = pRes->uiNumRows;

	m_pRefineTable = new TRefineTable[m_iRefineTableSize];
	memset(m_pRefineTable, 0, sizeof(TRefineTable) * m_iRefineTableSize);

	TRefineTable* prt = m_pRefineTable;
	MYSQL_ROW data;

	while ((data = mysql_fetch_row(pRes->pSQLResult)))
	{
		//const char* s_szQuery = "SELECT src_vnum, result_vnum, cost, prob, "
		//"vnum0, count0, vnum1, count1, vnum2, count2,  vnum3, count3, vnum4, count4 "

		int col = 0;
		//prt->src_vnum = atoi(data[col++]);
		//prt->result_vnum = atoi(data[col++]);
		str_to_number(prt->id, data[col++]);
		str_to_number(prt->cost, data[col++]);
		str_to_number(prt->prob, data[col++]);

		//@ikd tofix material == 0
		prt->material_count = REFINE_MATERIAL_MAX_NUM;

		for (int i = 0; i < REFINE_MATERIAL_MAX_NUM; i++)
		{
			str_to_number(prt->materials[i].vnum, data[col++]);
			str_to_number(prt->materials[i].count, data[col++]);
			if (prt->materials[i].vnum == 0)
			{
				prt->material_count = i;
				break;
			}
		}

		sys_log(1, "REFINE: id %ld cost %d prob %d mat1 %lu cnt1 %d", prt->id, prt->cost, prt->prob, prt->materials[0].vnum, prt->materials[0].count);
		prt++;
	}
	return true;
}

class FCompareVnum
{
public:
	bool operator () (const TEntityTable& a, const TEntityTable& b) const
	{
		return (a.dwVnum < b.dwVnum);
	}
};

bool CClientManager::InitializeMobTable()
{
	//_______________________________________________//

	//===============================================//

	map<int, const char*> localMap;
	//bool isNameFile = true;

	cCsvTable nameData;
	if (!nameData.Load("mob_names.txt", '\t'))
	{
		fprintf(stderr, "Could not load mob_names.txt\n");
	}
	else {
		nameData.Next();
		while (nameData.Next()) {
			localMap[atoi(nameData.AsStringByIndex(0))] = nameData.AsStringByIndex(1);
		}
	}
	//________________________________________________//

	cCsvTable data;

	if (!data.Load("mob_proto.txt", '\t'))
	{
		fprintf(stderr, "Could not load mob_proto.txt. Wrong file format?\n");
		return false;
	}
	data.Next();

	if (!m_vec_mobTable.empty())
	{
		sys_log(0, "RELOAD: mob_proto");
		m_vec_mobTable.clear();
	}
	m_vec_mobTable.resize(data.m_File.GetRowCount() - 1);
	memset(&m_vec_mobTable[0], 0, sizeof(TMobTable) * m_vec_mobTable.size());
	TMobTable* mob_table = &m_vec_mobTable[0];

	while (data.Next())
	{
		if (!Set_Proto_Mob_Table(mob_table, data, localMap))
		{
			fprintf(stderr, "Could not process entry.\n");
		}

		//sys_log(1, "MOB #%-12d %-64s %-64s level: %-3u rank: %u empire: %d maxhp: %lld",
		//	mob_table->dwVnum,
		//	mob_table->szName,
		//	mob_table->szLocaleName,
		//	mob_table->bLevel,
		//	mob_table->bRank,
		//	mob_table->bEmpire,
		//	mob_table->dwMaxHP
		//);
		++mob_table;
	}
	//_____________________________________________________//

	sort(m_vec_mobTable.begin(), m_vec_mobTable.end(), FCompareVnum());
	return true;
}

bool CClientManager::InitializeShopTable()
{
	MYSQL_ROW	data;
	int		col;

	static const char* s_szQuery =
		"SELECT "
		"shop.vnum, "
		"shop.npc_vnum, "
		"shop_item.item_vnum, "
		"shop_item.count "
#ifdef __SHOP_PRICE_SQL__
		",shop_item.price "
#ifdef __CHEQUE_SYSTEM__
		",shop_item.cheque "
#endif
#endif
#ifdef __SHOP_PRICE_TYPE_ITEM__
		",shop_item.witemVnum "
#endif
		"FROM shop LEFT JOIN shop_item "
		"ON shop.vnum = shop_item.shop_vnum ORDER BY shop.vnum, shop_item.item_vnum";

	std::unique_ptr<SQLMsg> pkMsg2(CDBManager::instance().DirectQuery(s_szQuery));

	SQLResult* pRes2 = pkMsg2->Get();

	if (!pRes2->uiNumRows)
	{
		sys_err("InitializeShopTable : Table count is zero.");
		return false;
	}

	std::map<int, TShopTable*> map_shop;

	if (m_pShopTable)
	{
		delete[](m_pShopTable);
		m_pShopTable = NULL;
	}

	TShopTable* shop_table = m_pShopTable;

	while ((data = mysql_fetch_row(pRes2->pSQLResult)))
	{
		col = 0;

		int iShopVnum = 0;
		str_to_number(iShopVnum, data[col++]);

		if (map_shop.end() == map_shop.find(iShopVnum))
		{
			shop_table = new TShopTable{};
			shop_table->dwVnum = iShopVnum;

			map_shop[iShopVnum] = shop_table;
		}
		else
			shop_table = map_shop[iShopVnum];

		str_to_number(shop_table->dwNPCVnum, data[col++]);

		if (!data[col])
			continue;

		TShopItemTable* pItem = &shop_table->items[shop_table->byItemCount];

		str_to_number(pItem->vnum, data[col++]);
		str_to_number(pItem->count, data[col++]);
#ifdef __SHOP_PRICE_SQL__
		str_to_number(pItem->price, data[col++]);
#ifdef __CHEQUE_SYSTEM__
		str_to_number(pItem->cheque, data[col++]);
#endif
#endif
#ifdef __SHOP_PRICE_TYPE_ITEM__
		str_to_number(pItem->witemVnum, data[col++]);
#endif
		++shop_table->byItemCount;
	}

	m_pShopTable = new TShopTable[map_shop.size()];
	m_iShopTableSize = map_shop.size();

	typeof(map_shop.begin()) it = map_shop.begin();

	int i = 0;

	while (it != map_shop.end())
	{
		thecore_memcpy((m_pShopTable + i), (it++)->second, sizeof(TShopTable));
		sys_log(1, "SHOP: #%d items: %d", (m_pShopTable + i)->dwVnum, (m_pShopTable + i)->byItemCount);
		++i;
	}

	return true;
}

bool CClientManager::InitializeItemTable()
{
	//_______________________________________________//

	//=================================================================================//
	//=================================================================================//
	map<int, const char*> localMap;
	cCsvTable nameData;
	if (!nameData.Load("item_names.txt", '\t'))
	{
		fprintf(stderr, "Could not load item_names.txt.\n");
	}
	else {
		nameData.Next();
		while (nameData.Next()) {
			localMap[atoi(nameData.AsStringByIndex(0))] = nameData.AsStringByIndex(1);
		}
	}
	//_________________________________________________________________//

	cCsvTable data;
	if (!data.Load("item_proto.txt", '\t'))
	{
		fprintf(stderr, "Could not load item_proto.txt. Wrong file format?\n");
		return false;
	}
	data.Next();

	if (!m_vec_itemTable.empty())
	{
		sys_log(0, "RELOAD: item_proto");
		m_vec_itemTable.clear();
		m_map_itemTableByVnum.clear();
	}

	data.Destroy();
	if (!data.Load("item_proto.txt", '\t'))
	{
		fprintf(stderr, "Could not load item_proto.txt. Wrong file format?\n");
		return false;
	}
	data.Next();

	m_vec_itemTable.resize(data.m_File.GetRowCount() - 1);
	memset(&m_vec_itemTable[0], 0, sizeof(TItemTable) * m_vec_itemTable.size());

	TItemTable* item_table = &m_vec_itemTable[0];

	while (data.Next())
	{
		if (!Set_Proto_Item_Table(item_table, data, localMap))
		{
			fprintf(stderr, "Failed to load item_proto table.\n");
		}

		m_map_itemTableByVnum.insert(std::map<DWORD, TItemTable*>::value_type(item_table->dwVnum, item_table));
		++item_table;
	}
	//_______________________________________________________________________//

	m_map_itemTableByVnum.clear();

	itertype(m_vec_itemTable) it = m_vec_itemTable.begin();

	while (it != m_vec_itemTable.end())
	{
		TItemTable* item_table = &(*(it++));

		//sys_log(1, "ITEM: #%-12lu %-64s %-64s VAL: %ld %ld %ld %ld %ld %ld WEAR %lu ANTI %lu IMMUNE %lu REFINE %lu REFINE_SET %u MAGIC_PCT %u",
		//	item_table->dwVnum,
		//	item_table->szName,
		//	item_table->szLocaleName,
		//	item_table->alValues[0],
		//	item_table->alValues[1],
		//	item_table->alValues[2],
		//	item_table->alValues[3],
		//	item_table->alValues[4],
		//	item_table->alValues[5],
		//	item_table->dwWearFlags,
		//	item_table->dwAntiFlags,
		//	item_table->dwImmuneFlag,
		//	item_table->dwRefinedVnum,
		//	item_table->dwRefineSet,
		//	item_table->bAlterToMagicItemPct);

		m_map_itemTableByVnum.insert(std::map<DWORD, TItemTable*>::value_type(item_table->dwVnum, item_table));
	}
	sort(m_vec_itemTable.begin(), m_vec_itemTable.end(), FCompareVnum());
	return true;
}

bool CClientManager::InitializeSkillTable()
{
	char query[4096];
	snprintf(query, sizeof(query),
		"SELECT dwVnum, szName, bType, bMaxLevel, dwSplashRange, "
		"szPointOn, szPointPoly, szSPCostPoly, szDurationPoly, szDurationSPCostPoly, "
		"szCooldownPoly, szMasterBonusPoly, setFlag+0, setAffectFlag+0, "
		"szPointOn2, szPointPoly2, szDurationPoly2, setAffectFlag2+0, "
		"szPointOn3, szPointPoly3, szDurationPoly3, szGrandMasterAddSPCostPoly, "
		"bLevelStep, bLevelLimit, prerequisiteSkillVnum, prerequisiteSkillLevel, iMaxHit, szSplashAroundDamageAdjustPoly, eSkillType+0, dwTargetRange "
		"FROM skill_proto%s ORDER BY dwVnum",
		GetTablePostfix());

	std::unique_ptr<SQLMsg> pkMsg(CDBManager::instance().DirectQuery(query));
	SQLResult* pRes = pkMsg->Get();

	if (!pRes->uiNumRows)
	{
		sys_err("no result from skill_proto");
		return false;
	}

	if (!m_vec_skillTable.empty())
	{
		sys_log(0, "RELOAD: skill_proto");
		m_vec_skillTable.clear();
	}

	m_vec_skillTable.reserve(pRes->uiNumRows);

	MYSQL_ROW	data;
	int		col;

	while ((data = mysql_fetch_row(pRes->pSQLResult)))
	{
		TSkillTable t;
		memset(&t, 0, sizeof(t));

		col = 0;

		str_to_number(t.dwVnum, data[col++]);
		strlcpy(t.szName, data[col++], sizeof(t.szName));
		str_to_number(t.bType, data[col++]);
		str_to_number(t.bMaxLevel, data[col++]);
		str_to_number(t.dwSplashRange, data[col++]);

		strlcpy(t.szPointOn, data[col++], sizeof(t.szPointOn));
		strlcpy(t.szPointPoly, data[col++], sizeof(t.szPointPoly));
		strlcpy(t.szSPCostPoly, data[col++], sizeof(t.szSPCostPoly));
		strlcpy(t.szDurationPoly, data[col++], sizeof(t.szDurationPoly));
		strlcpy(t.szDurationSPCostPoly, data[col++], sizeof(t.szDurationSPCostPoly));
		strlcpy(t.szCooldownPoly, data[col++], sizeof(t.szCooldownPoly));
		strlcpy(t.szMasterBonusPoly, data[col++], sizeof(t.szMasterBonusPoly));

		str_to_number(t.dwFlag, data[col++]);
		str_to_number(t.dwAffectFlag, data[col++]);

		strlcpy(t.szPointOn2, data[col++], sizeof(t.szPointOn2));
		strlcpy(t.szPointPoly2, data[col++], sizeof(t.szPointPoly2));
		strlcpy(t.szDurationPoly2, data[col++], sizeof(t.szDurationPoly2));
		str_to_number(t.dwAffectFlag2, data[col++]);

		// ADD_GRANDMASTER_SKILL
		strlcpy(t.szPointOn3, data[col++], sizeof(t.szPointOn3));
		strlcpy(t.szPointPoly3, data[col++], sizeof(t.szPointPoly3));
		strlcpy(t.szDurationPoly3, data[col++], sizeof(t.szDurationPoly3));

		strlcpy(t.szGrandMasterAddSPCostPoly, data[col++], sizeof(t.szGrandMasterAddSPCostPoly));
		// END_OF_ADD_GRANDMASTER_SKILL

		str_to_number(t.bLevelStep, data[col++]);
		str_to_number(t.bLevelLimit, data[col++]);
		str_to_number(t.preSkillVnum, data[col++]);
		str_to_number(t.preSkillLevel, data[col++]);

		str_to_number(t.lMaxHit, data[col++]);

		strlcpy(t.szSplashAroundDamageAdjustPoly, data[col++], sizeof(t.szSplashAroundDamageAdjustPoly));

		str_to_number(t.bSkillAttrType, data[col++]);
		str_to_number(t.dwTargetRange, data[col++]);
		sys_log(1, "SKILL: #%d %s flag %u point %s affect %u cooldown %s", t.dwVnum, t.szName, t.dwFlag, t.szPointOn, t.dwAffectFlag, t.szCooldownPoly);
		m_vec_skillTable.push_back(t);
	}

	return true;
}

bool CClientManager::InitializeItemAttrTable()
{
	char query[4096];
	snprintf(query, sizeof(query),
		"SELECT apply, apply+0, prob, lv1, lv2, lv3, lv4, lv5, weapon, body, wrist, foots, neck, head, shield, ear "
#ifdef __ITEM_ATTR_COSTUME__
		", costume_body, costume_hair"
#if defined(__ITEM_ATTR_COSTUME__) && defined(__WEAPON_COSTUME_SYSTEM__)
		", costume_weapon"
#endif
#endif
#ifdef __PENDANT_SYSTEM__
		", pendant"
#endif
#ifdef __AURA_COSTUME_SYSTEM__
		", costume_aura"
#endif
#ifdef __MOUNT_COSTUME_SYSTEM__
		", costume_mount"
#endif
#ifdef __MARTY_BELT_INVENTORY__
		", belt"
#endif
#ifdef __SHINING_ITEM_SYSTEM__
		", acce_shining"
		", soul_shining"
#endif
#ifdef __CAKRA_ITEM_SYSTEM__
		", cakra"
#endif
#ifdef __PET_SYSTEM_PROTO__
		", pet"
#endif
#ifdef __SEBNEM_ITEM_SYSTEM__
		", sebnem"
#endif
		" FROM item_attr%s ORDER BY apply",
		GetTablePostfix());

	std::unique_ptr<SQLMsg> pkMsg(CDBManager::instance().DirectQuery(query));
	SQLResult* pRes = pkMsg->Get();

	if (!pRes->uiNumRows)
	{
		sys_err("no result from item_attr");
		return false;
	}

	if (!m_vec_itemAttrTable.empty())
	{
		sys_log(0, "RELOAD: item_attr");
		m_vec_itemAttrTable.clear();
	}

	m_vec_itemAttrTable.reserve(pRes->uiNumRows);

	MYSQL_ROW	data;

	while ((data = mysql_fetch_row(pRes->pSQLResult)))
	{
		TItemAttrTable t{};

		int col = 0;

		strlcpy(t.szApply, data[col++], sizeof(t.szApply));
		str_to_number(t.dwApplyIndex, data[col++]);
		str_to_number(t.dwProb, data[col++]);
		str_to_number(t.lValues[0], data[col++]);
		str_to_number(t.lValues[1], data[col++]);
		str_to_number(t.lValues[2], data[col++]);
		str_to_number(t.lValues[3], data[col++]);
		str_to_number(t.lValues[4], data[col++]);
		str_to_number(t.bMaxLevelBySet[ATTRIBUTE_SET_WEAPON], data[col++]);
		str_to_number(t.bMaxLevelBySet[ATTRIBUTE_SET_BODY], data[col++]);
		str_to_number(t.bMaxLevelBySet[ATTRIBUTE_SET_WRIST], data[col++]);
		str_to_number(t.bMaxLevelBySet[ATTRIBUTE_SET_FOOTS], data[col++]);
		str_to_number(t.bMaxLevelBySet[ATTRIBUTE_SET_NECK], data[col++]);
		str_to_number(t.bMaxLevelBySet[ATTRIBUTE_SET_HEAD], data[col++]);
		str_to_number(t.bMaxLevelBySet[ATTRIBUTE_SET_SHIELD], data[col++]);
		str_to_number(t.bMaxLevelBySet[ATTRIBUTE_SET_EAR], data[col++]);
#ifdef __ITEM_ATTR_COSTUME__
		str_to_number(t.bMaxLevelBySet[ATTRIBUTE_SET_COSTUME_BODY], data[col++]);
		str_to_number(t.bMaxLevelBySet[ATTRIBUTE_SET_COSTUME_HAIR], data[col++]);
#if defined(__ITEM_ATTR_COSTUME__) && defined(__WEAPON_COSTUME_SYSTEM__)
		str_to_number(t.bMaxLevelBySet[ATTRIBUTE_SET_COSTUME_WEAPON], data[col++]);
#endif
#endif
#ifdef __PENDANT_SYSTEM__
		str_to_number(t.bMaxLevelBySet[ATTRIBUTE_SET_PENDANT], data[col++]);
#endif
#ifdef __AURA_COSTUME_SYSTEM__
		str_to_number(t.bMaxLevelBySet[ATTRIBUTE_SET_COSTUME_AURA], data[col++]);
#endif
#ifdef __MOUNT_COSTUME_SYSTEM__
		str_to_number(t.bMaxLevelBySet[ATTRIBUTE_SET_COSTUME_MOUNT], data[col++]);
#endif
#ifdef __MARTY_BELT_INVENTORY__
		str_to_number(t.bMaxLevelBySet[ATTRIBUTE_SET_BELT], data[col++]);
#endif
#ifdef __SHINING_ITEM_SYSTEM__
		str_to_number(t.bMaxLevelBySet[ATTRIBUTE_SET_ACCE_SHINING], data[col++]);
		str_to_number(t.bMaxLevelBySet[ATTRIBUTE_SET_SOUL_SHINING], data[col++]);
#endif
#ifdef __CAKRA_ITEM_SYSTEM__
		str_to_number(t.bMaxLevelBySet[ATTRIBUTE_SET_CAKRA], data[col++]);
#endif
#ifdef __PET_SYSTEM_PROTO__
		str_to_number(t.bMaxLevelBySet[ATTRIBUTE_SET_PET], data[col++]);
#endif
#ifdef __SEBNEM_ITEM_SYSTEM__
		str_to_number(t.bMaxLevelBySet[ATTRIBUTE_SET_SEBNEM], data[col++]);
#endif
		sys_log(1, "ITEM_ATTR: %-20s %4lu { %3d %3d %3d %3d %3d } { %d %d %d %d %d %d %d"
#ifdef __ITEM_ATTR_COSTUME__
			" %d %d"
#if defined(__ITEM_ATTR_COSTUME__) && defined(__WEAPON_COSTUME_SYSTEM__)
			" %d"
#endif
#endif
#ifdef __PENDANT_SYSTEM__
			" %d"
#endif
#ifdef __AURA_COSTUME_SYSTEM__
			" %d"
#endif
#ifdef __MOUNT_COSTUME_SYSTEM__
			" %d"
#endif
#ifdef __MARTY_BELT_INVENTORY__
			" %d"
#endif
#ifdef __SHINING_ITEM_SYSTEM__
			" %d"
			" %d"
#endif
#ifdef __CAKRA_ITEM_SYSTEM__
			" %d"
#endif
#ifdef __PET_SYSTEM_PROTO__
			" %d"
#endif
#ifdef __SEBNEM_ITEM_SYSTEM__
			" %d"
#endif
			" }",
			t.szApply,
			t.dwProb,
			t.lValues[0],
			t.lValues[1],
			t.lValues[2],
			t.lValues[3],
			t.lValues[4],
			t.bMaxLevelBySet[ATTRIBUTE_SET_WEAPON],
			t.bMaxLevelBySet[ATTRIBUTE_SET_BODY],
			t.bMaxLevelBySet[ATTRIBUTE_SET_WRIST],
			t.bMaxLevelBySet[ATTRIBUTE_SET_FOOTS],
			t.bMaxLevelBySet[ATTRIBUTE_SET_NECK],
			t.bMaxLevelBySet[ATTRIBUTE_SET_HEAD],
			t.bMaxLevelBySet[ATTRIBUTE_SET_SHIELD],
			t.bMaxLevelBySet[ATTRIBUTE_SET_EAR]
#ifdef __ITEM_ATTR_COSTUME__
			, t.bMaxLevelBySet[ATTRIBUTE_SET_COSTUME_BODY]
			, t.bMaxLevelBySet[ATTRIBUTE_SET_COSTUME_HAIR]
#if defined(__ITEM_ATTR_COSTUME__) && defined(__WEAPON_COSTUME_SYSTEM__)
			, t.bMaxLevelBySet[ATTRIBUTE_SET_COSTUME_WEAPON]
#endif
#endif
#ifdef __PENDANT_SYSTEM__
			, t.bMaxLevelBySet[ATTRIBUTE_SET_PENDANT]
#endif
#ifdef __AURA_COSTUME_SYSTEM__
			, t.bMaxLevelBySet[ATTRIBUTE_SET_COSTUME_AURA]
#endif
#ifdef __MOUNT_COSTUME_SYSTEM__
			, t.bMaxLevelBySet[ATTRIBUTE_SET_COSTUME_MOUNT]
#endif
#ifdef __MARTY_BELT_INVENTORY__
			, t.bMaxLevelBySet[ATTRIBUTE_SET_BELT]
#endif
#ifdef __SHINING_ITEM_SYSTEM__
			, t.bMaxLevelBySet[ATTRIBUTE_SET_ACCE_SHINING]
			, t.bMaxLevelBySet[ATTRIBUTE_SET_SOUL_SHINING]
#endif
#ifdef __CAKRA_ITEM_SYSTEM__
			, t.bMaxLevelBySet[ATTRIBUTE_SET_CAKRA]
#endif
#ifdef __PET_SYSTEM_PROTO__
			, t.bMaxLevelBySet[ATTRIBUTE_SET_PET]
#endif
#ifdef __SEBNEM_ITEM_SYSTEM__
			, t.bMaxLevelBySet[ATTRIBUTE_SET_SEBNEM]
#endif
		);

		m_vec_itemAttrTable.push_back(t);
	}

	return true;
}

bool CClientManager::InitializeItemRareTable()
{
	char query[4096];
	snprintf(query, sizeof(query),
		"SELECT apply, apply+0, prob, lv1, lv2, lv3, lv4, lv5, weapon, body, wrist, foots, neck, head, shield, ear "
#ifdef __ITEM_ATTR_COSTUME__
		", costume_body, costume_hair"
#if defined(__ITEM_ATTR_COSTUME__) && defined(__WEAPON_COSTUME_SYSTEM__)
		", costume_weapon"
#endif
#endif
#ifdef __PENDANT_SYSTEM__
		", pendant"
#endif
#ifdef __AURA_COSTUME_SYSTEM__
		", costume_aura"
#endif
#ifdef __MOUNT_COSTUME_SYSTEM__
		", costume_mount"
#endif
#ifdef __MARTY_BELT_INVENTORY__
		", belt"
#endif
#ifdef __SHINING_ITEM_SYSTEM__
		", acce_shining"
		", soul_shining"
#endif
#ifdef __CAKRA_ITEM_SYSTEM__
		", cakra"
#endif
#ifdef __PET_SYSTEM_PROTO__
		", pet"
#endif
#ifdef __SEBNEM_ITEM_SYSTEM__
		", sebnem"
#endif
		" FROM item_attr_rare%s ORDER BY apply",
		GetTablePostfix());

	std::unique_ptr<SQLMsg> pkMsg(CDBManager::instance().DirectQuery(query));
	SQLResult* pRes = pkMsg->Get();

	if (!pRes->uiNumRows)
	{
		sys_err("no result from item_attr_rare");
		return false;
	}

	if (!m_vec_itemRareTable.empty())
	{
		sys_log(0, "RELOAD: item_attr_rare");
		m_vec_itemRareTable.clear();
	}

	m_vec_itemRareTable.reserve(pRes->uiNumRows);

	MYSQL_ROW	data;

	while ((data = mysql_fetch_row(pRes->pSQLResult)))
	{
		TItemAttrTable t{};

		int col = 0;

		strlcpy(t.szApply, data[col++], sizeof(t.szApply));
		str_to_number(t.dwApplyIndex, data[col++]);
		str_to_number(t.dwProb, data[col++]);
		str_to_number(t.lValues[0], data[col++]);
		str_to_number(t.lValues[1], data[col++]);
		str_to_number(t.lValues[2], data[col++]);
		str_to_number(t.lValues[3], data[col++]);
		str_to_number(t.lValues[4], data[col++]);
		str_to_number(t.bMaxLevelBySet[ATTRIBUTE_SET_WEAPON], data[col++]);
		str_to_number(t.bMaxLevelBySet[ATTRIBUTE_SET_BODY], data[col++]);
		str_to_number(t.bMaxLevelBySet[ATTRIBUTE_SET_WRIST], data[col++]);
		str_to_number(t.bMaxLevelBySet[ATTRIBUTE_SET_FOOTS], data[col++]);
		str_to_number(t.bMaxLevelBySet[ATTRIBUTE_SET_NECK], data[col++]);
		str_to_number(t.bMaxLevelBySet[ATTRIBUTE_SET_HEAD], data[col++]);
		str_to_number(t.bMaxLevelBySet[ATTRIBUTE_SET_SHIELD], data[col++]);
		str_to_number(t.bMaxLevelBySet[ATTRIBUTE_SET_EAR], data[col++]);
#ifdef __ITEM_ATTR_COSTUME__
		str_to_number(t.bMaxLevelBySet[ATTRIBUTE_SET_COSTUME_BODY], data[col++]);
		str_to_number(t.bMaxLevelBySet[ATTRIBUTE_SET_COSTUME_HAIR], data[col++]);
#if defined(__ITEM_ATTR_COSTUME__) && defined(__WEAPON_COSTUME_SYSTEM__)
		str_to_number(t.bMaxLevelBySet[ATTRIBUTE_SET_COSTUME_WEAPON], data[col++]);
#endif
#endif
#ifdef __PENDANT_SYSTEM__
		str_to_number(t.bMaxLevelBySet[ATTRIBUTE_SET_PENDANT], data[col++]);
#endif
#ifdef __AURA_COSTUME_SYSTEM__
		str_to_number(t.bMaxLevelBySet[ATTRIBUTE_SET_COSTUME_AURA], data[col++]);
#endif
#ifdef __MOUNT_COSTUME_SYSTEM__
		str_to_number(t.bMaxLevelBySet[ATTRIBUTE_SET_COSTUME_MOUNT], data[col++]);
#endif
#ifdef __MARTY_BELT_INVENTORY__
		str_to_number(t.bMaxLevelBySet[ATTRIBUTE_SET_BELT], data[col++]);
#endif
#ifdef __SHINING_ITEM_SYSTEM__
		str_to_number(t.bMaxLevelBySet[ATTRIBUTE_SET_ACCE_SHINING], data[col++]);
		str_to_number(t.bMaxLevelBySet[ATTRIBUTE_SET_SOUL_SHINING], data[col++]);
#endif
#ifdef __CAKRA_ITEM_SYSTEM__
		str_to_number(t.bMaxLevelBySet[ATTRIBUTE_SET_CAKRA], data[col++]);
#endif
#ifdef __PET_SYSTEM_PROTO__
		str_to_number(t.bMaxLevelBySet[ATTRIBUTE_SET_PET], data[col++]);
#endif
#ifdef __SEBNEM_ITEM_SYSTEM__
		str_to_number(t.bMaxLevelBySet[ATTRIBUTE_SET_SEBNEM], data[col++]);
#endif
		sys_log(1, "ITEM_RARE: %-20s %4lu { %3d %3d %3d %3d %3d } { %d %d %d %d %d %d %d"
#ifdef __ITEM_ATTR_COSTUME__
			" %d %d"
#if defined(__ITEM_ATTR_COSTUME__) && defined(__WEAPON_COSTUME_SYSTEM__)
			" %d"
#endif
#endif
#ifdef __PENDANT_SYSTEM__
			" %d"
#endif
#ifdef __AURA_COSTUME_SYSTEM__
			" %d"
#endif
#ifdef __MOUNT_COSTUME_SYSTEM__
			" %d"
#endif
#ifdef __MARTY_BELT_INVENTORY__
			" %d"
#endif
#ifdef __SHINING_ITEM_SYSTEM__
			" %d"
			" %d"
#endif
#ifdef __CAKRA_ITEM_SYSTEM__
			" %d"
#endif
#ifdef __PET_SYSTEM_PROTO__
			" %d"
#endif
#ifdef __SEBNEM_ITEM_SYSTEM__
			" %d"
#endif
			" }",
			t.szApply,
			t.dwProb,
			t.lValues[0],
			t.lValues[1],
			t.lValues[2],
			t.lValues[3],
			t.lValues[4],
			t.bMaxLevelBySet[ATTRIBUTE_SET_WEAPON],
			t.bMaxLevelBySet[ATTRIBUTE_SET_BODY],
			t.bMaxLevelBySet[ATTRIBUTE_SET_WRIST],
			t.bMaxLevelBySet[ATTRIBUTE_SET_FOOTS],
			t.bMaxLevelBySet[ATTRIBUTE_SET_NECK],
			t.bMaxLevelBySet[ATTRIBUTE_SET_HEAD],
			t.bMaxLevelBySet[ATTRIBUTE_SET_SHIELD],
			t.bMaxLevelBySet[ATTRIBUTE_SET_EAR]
#ifdef __ITEM_ATTR_COSTUME__
			, t.bMaxLevelBySet[ATTRIBUTE_SET_COSTUME_BODY]
			, t.bMaxLevelBySet[ATTRIBUTE_SET_COSTUME_HAIR]
#if defined(__ITEM_ATTR_COSTUME__) && defined(__WEAPON_COSTUME_SYSTEM__)
			, t.bMaxLevelBySet[ATTRIBUTE_SET_COSTUME_WEAPON]
#endif
#endif
#ifdef __PENDANT_SYSTEM__
			, t.bMaxLevelBySet[ATTRIBUTE_SET_PENDANT]
#endif
#ifdef __AURA_COSTUME_SYSTEM__
			, t.bMaxLevelBySet[ATTRIBUTE_SET_COSTUME_AURA]
#endif
#ifdef __MOUNT_COSTUME_SYSTEM__
			, t.bMaxLevelBySet[ATTRIBUTE_SET_COSTUME_MOUNT]
#endif
#ifdef __MARTY_BELT_INVENTORY__
			, t.bMaxLevelBySet[ATTRIBUTE_SET_BELT]
#endif
#ifdef __SHINING_ITEM_SYSTEM__
			, t.bMaxLevelBySet[ATTRIBUTE_SET_ACCE_SHINING]
			, t.bMaxLevelBySet[ATTRIBUTE_SET_SOUL_SHINING]
#endif
#ifdef __CAKRA_ITEM_SYSTEM__
			, t.bMaxLevelBySet[ATTRIBUTE_SET_CAKRA]
#endif
#ifdef __PET_SYSTEM_PROTO__
			, t.bMaxLevelBySet[ATTRIBUTE_SET_PET]
#endif
#ifdef __SEBNEM_ITEM_SYSTEM__
			, t.bMaxLevelBySet[ATTRIBUTE_SET_SEBNEM]
#endif
		);
		m_vec_itemRareTable.push_back(t);
	}

	return true;
}

void parse_pair_number_string(const char* c_pszString, std::vector<std::pair<int, int> >& vec)
{
	// format: 10,1/20,3/300,50
	const char* t = c_pszString;
	const char* p = strchr(t, '/');
	std::pair<int, int> k;

	char szNum[32 + 1];
	char* comma;

	while (p)
	{
		if (isnhdigit(*t))
		{
			strlcpy(szNum, t, MIN(sizeof(szNum), (p - t) + 1));

			comma = strchr(szNum, ',');

			if (comma)
			{
				*comma = '\0';
				str_to_number(k.second, comma + 1);
			}
			else
				k.second = 0;

			str_to_number(k.first, szNum);
			vec.push_back(k);
		}

		t = p + 1;
		p = strchr(t, '/');
	}

	if (isnhdigit(*t))
	{
		strlcpy(szNum, t, sizeof(szNum));

		comma = strchr(const_cast<char*>(t), ',');

		if (comma)
		{
			*comma = '\0';
			str_to_number(k.second, comma + 1);
		}
		else
			k.second = 0;

		str_to_number(k.first, szNum);
		vec.push_back(k);
	}
}

