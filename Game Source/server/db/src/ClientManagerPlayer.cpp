#include "stdafx.h"

#include "ClientManager.h"

#include "Main.h"
#include "QID.h"
#include "Cache.h"
#ifdef __SKILL_COLOR__
#include "../../common/length.h"
#endif
extern std::string g_stLocale;
extern int g_test_server;
extern int g_log;

//
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// !!!!!!!!!!! IMPORTANT !!!!!!!!!!!!
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//
// Check all SELECT syntax on item table before change this function!!!
//
bool CreateItemTableFromRes(MYSQL_RES* res, std::vector<TPlayerItem>* pVec, DWORD dwPID)
{
	if (!res)
	{
		pVec->clear();
		return true;
	}

	int rows;

	if ((rows = mysql_num_rows(res)) <= 0)
	{
		pVec->clear();
		return true;
	}

	pVec->resize(rows);

	for (int i = 0; i < rows; ++i)
	{
		MYSQL_ROW row = mysql_fetch_row(res);
		TPlayerItem& item = pVec->at(i);

		int cur = 0;

		// Check all SELECT syntax on item table before change this function!!!
		// Check all SELECT syntax on item table before change this function!!!
		// Check all SELECT syntax on item table before change this function!!!
		str_to_number(item.id, row[cur++]);
		str_to_number(item.window, row[cur++]);
		str_to_number(item.pos, row[cur++]);
		str_to_number(item.count, row[cur++]);
		str_to_number(item.vnum, row[cur++]);
#ifdef __BEGINNER_ITEM__
		str_to_number(item.is_basic, row[cur++]);
#endif
#ifdef __ITEM_EVOLUTION__
		str_to_number(item.evolution, row[cur++]);
#endif
#ifdef __ITEM_CHANGELOOK__
		str_to_number(item.transmutation, row[cur++]);
#endif
		str_to_number(item.alSockets[0], row[cur++]);
		str_to_number(item.alSockets[1], row[cur++]);
		str_to_number(item.alSockets[2], row[cur++]);
#ifdef __ITEM_EXTRA_SOCKET__
		str_to_number(item.alSockets[3], row[cur++]);
#endif
		for (int j = 0; j < ITEM_ATTRIBUTE_MAX_NUM; j++)
		{
			str_to_number(item.aAttr[j].bType, row[cur++]);
			str_to_number(item.aAttr[j].sValue, row[cur++]);
		}

		item.owner = dwPID;
	}

	return true;
}

bool CreateQuestTableFromRes(MYSQL_RES* res, std::vector<TQuestTable>* pVec, DWORD dwPID)
{
	if (!res)
	{
		pVec->clear();
		return true;
	}

	int rows;

	if ((rows = mysql_num_rows(res)) <= 0)
	{
		pVec->clear();
		return true;
	}

	pVec->resize(rows);

	for (int i = 0; i < rows; ++i)
	{
		MYSQL_ROW row = mysql_fetch_row(res);
		TQuestTable& qf = pVec->at(i);

		int cur = 0;

		str_to_number(qf.dwPID, row[cur++]);
		strlcpy(qf.szName, row[cur++], sizeof(qf.szName));
		strlcpy(qf.szState, row[cur++], sizeof(qf.szState));
		str_to_number(qf.lValue, row[cur++]);

		// sys_log(0, "CreateQuestTableFromRes %d %s %s %d", qf.dwPID, qf.szName, qf.szState, qf.lValue);
	}

	return true;
}

bool CreateAffectTableFromRes(MYSQL_RES* res, std::vector<TPacketAffectElement>* pVec, DWORD dwPID)
{
	if (!res)
	{
		pVec->clear();
		return true;
	}

	int rows;

	if ((rows = mysql_num_rows(res)) <= 0)
	{
		pVec->clear();
		return true;
	}

	pVec->resize(rows);

	for (int i = 0; i < rows; ++i)
	{
		MYSQL_ROW row = mysql_fetch_row(res);
		TPacketAffectElement& aff = pVec->at(i);

		int cur = 1; // pidyi gec

		str_to_number(aff.dwType, row[cur++]);
		str_to_number(aff.bApplyOn, row[cur++]);
		str_to_number(aff.lApplyValue, row[cur++]);
		str_to_number(aff.dwFlag, row[cur++]);
		str_to_number(aff.lDuration, row[cur++]);
		str_to_number(aff.lSPCost, row[cur++]);
	}

	return true;
}

#ifdef __SKILL_COLOR__
bool CreateSkillColorTableFromRes(MYSQL_RES* res, DWORD* dwSkillColor)
{
	if (mysql_num_rows(res) == 0)
		return false;

	MYSQL_ROW row = mysql_fetch_row(res);

	for (int x = 0; x < ESkillColorLength::MAX_SKILL_COUNT + ESkillColorLength::MAX_BUFF_COUNT; ++x)
	{
		for (int i = 0; i < ESkillColorLength::MAX_EFFECT_COUNT; ++i)
		{
			*(dwSkillColor++) = atoi(row[i + (x * ESkillColorLength::MAX_EFFECT_COUNT)]);
		}
	}

	return true;
}
#endif

#ifdef __BATTLE_PASS_SYSTEM__
bool CreateBattlePassTableFromRes(MYSQL_RES* res, std::vector<TPlayerBattlePassMission>* pVec, DWORD dwPID)
{
	if (!res)
	{
		pVec->clear();
		return true;
	}

	int rows;

	if ((rows = mysql_num_rows(res)) <= 0)
	{
		pVec->clear();
		return true;
	}

	pVec->resize(rows);

	for (int i = 0; i < rows; ++i)
	{
		MYSQL_ROW row = mysql_fetch_row(res);
		TPlayerBattlePassMission& aff = pVec->at(i);

		int cur = 1; // pidyi gec
		aff.dwPlayerId = dwPID;
		str_to_number(aff.dwMissionId, row[cur++]);
		str_to_number(aff.dwBattlePassId, row[cur++]);
		str_to_number(aff.dwExtraInfo, row[cur++]);
		str_to_number(aff.bCompleted, row[cur++]);
		aff.bIsUpdated = 0;
	}

	return true;
}
#endif // __BATTLE_PASS_SYSTEM__

size_t CreatePlayerSaveQuery(char* pszQuery, size_t querySize, TPlayerTable* pkTab)
{
	size_t queryLen;

	queryLen = snprintf(pszQuery, querySize,
		"UPDATE player%s SET "
		"job = %d, "
		"dir = %d, "
		"x = %d, "
		"y = %d, "
		"z = %d, "
		"map_index = %d, "
		"exit_x = %ld, "
		"exit_y = %ld, "
		"exit_map_index = %ld, "
		"hp = %lld, "
		"mp = %d, "
		"stamina = %d, "
		"random_hp = %lld, "
		"random_sp = %d, "
		"playtime = %d, "
		"level = %d, "
		"level_step = %d, "
		"st = %d, "
		"ht = %d, "
		"dx = %d, "
		"iq = %d, "
#ifdef __GOLD_LIMIT_REWORK__
		"gold = %lld, "
#else
		"gold = %d, "
#endif
#ifdef __CHEQUE_SYSTEM__
		"cheque = %d,"
#endif
		"exp = %u, "
		"stat_point = %d, "
		"skill_point = %d, "
		"sub_skill_point = %d, "
		"stat_reset_count = %d, "
		"ip = '%s', "
		"part_main = %d, "
		"part_hair = %d, "
#ifdef __ACCE_SYSTEM__
		"part_acce = %d, "
#endif
		"last_play = NOW(), "
		"skill_group = %d, "
		"alignment = %ld, "
#ifdef __RANK_SYSTEM__
		"rank = %ld, "
#endif
#ifdef __LANDRANK_SYSTEM__
		"land_rank = %ld, "
#endif
#ifdef __REBORN_SYSTEM__
		"reborn = %ld, "
#endif
		"horse_level = %d, "
		"horse_riding = %d, "
		"horse_hp = %d, "
		"horse_hp_droptime = %u, "
		"horse_stamina = %d, "
		"horse_skill_point = %d, "
#ifdef ENABLE_PLAYER_STATISTICS
		"statistics_killed_shinsoo = %d, "
		"statistics_killed_chunjo = %d, "
		"statistics_killed_jinno = %d, "
		"statistics_total_kill = %d, "
		"statistics_duel_won = %d, "
		"statistics_duel_lost = %d, "
		"statistics_killed_monster = %d, "
		"statistics_killed_stone = %d, "
		"statistics_killed_boss = %d, "
		"statistics_completed_dungeon = %d, "
		"statistics_taked_fish = %d, "
		"statistics_best_stone_damage = %lld, "
		"statistics_best_boss_damage = %lld, "
#endif
#ifdef __BATTLE_PASS_SYSTEM__
		"battle_pass_end = FROM_UNIXTIME(%d), "
#endif
#ifdef ENABLE_CHAT_COLOR_SYSTEM
		"color = %d, "
#endif
#ifdef __TEAM_SYSTEM__
		"team = %ld, "
#endif
#ifdef __MONIKER_SYSTEM__
		"moniker = '%s', "
#endif
#ifdef ENABLE_LOVE_SYSTEM
		"love1 = '%s',"
		"love2 = '%s',"
#endif // ENABLE_LOVE_SYSTEM
#ifdef ENABLE_WORD_SYSTEM
		"word = '%s',"
#endif // ENABLE_WORD_SYSTEM
		,
		GetTablePostfix(),
		pkTab->job,
		pkTab->dir,
		pkTab->x,
		pkTab->y,
		pkTab->z,
		pkTab->lMapIndex,
		pkTab->lExitX,
		pkTab->lExitY,
		pkTab->lExitMapIndex,
		pkTab->hp,
		pkTab->sp,
		pkTab->stamina,
		pkTab->sRandomHP,
		pkTab->sRandomSP,
		pkTab->playtime,
		pkTab->level,
		pkTab->level_step,
		pkTab->st,
		pkTab->ht,
		pkTab->dx,
		pkTab->iq,
		pkTab->gold,
#ifdef __CHEQUE_SYSTEM__
		pkTab->cheque,
#endif
		pkTab->exp,
		pkTab->stat_point,
		pkTab->skill_point,
		pkTab->sub_skill_point,
		pkTab->stat_reset_count,
		pkTab->ip,
		pkTab->parts[PART_MAIN],
		pkTab->parts[PART_HAIR],
#ifdef __ACCE_SYSTEM__
		pkTab->parts[PART_ACCE],
#endif
		pkTab->skill_group,
		pkTab->lAlignment,
#ifdef __RANK_SYSTEM__
		pkTab->lRank,
#endif
#ifdef __LANDRANK_SYSTEM__
		pkTab->lLandRank,
#endif
#ifdef __REBORN_SYSTEM__
		pkTab->lReborn,
#endif
		pkTab->horse.bLevel,
		pkTab->horse.bRiding,
		pkTab->horse.sHealth,
		pkTab->horse.dwHorseHealthDropTime,
		pkTab->horse.sStamina,
		pkTab->horse_skill_point
#ifdef ENABLE_PLAYER_STATISTICS
		, pkTab->iKilledShinsoo
		, pkTab->iKilledChunjo
		, pkTab->iKilledJinno
		, pkTab->iTotalKill
		, pkTab->iDuelWon
		, pkTab->iDuelLost
		, pkTab->iKilledMonster
		, pkTab->iKilledStone
		, pkTab->iKilledBoss
		, pkTab->iCompletedDungeon
		, pkTab->iTakedFish
		, pkTab->iBestStoneDamage
		, pkTab->iBestBossDamage
#endif
#ifdef __BATTLE_PASS_SYSTEM__
		, pkTab->dwBattlePassEndTime
#endif
#ifdef ENABLE_CHAT_COLOR_SYSTEM
		, pkTab->color
#endif
#ifdef __TEAM_SYSTEM__
		, pkTab->lTeam
#endif
#ifdef __MONIKER_SYSTEM__
		, pkTab->moniker
#endif
#ifdef ENABLE_LOVE_SYSTEM
		, pkTab->love1
		, pkTab->love2
#endif // ENABLE_LOVE_SYSTEM
#ifdef ENABLE_WORD_SYSTEM
		, pkTab->word
#endif // ENABLE_WORD_SYSTEM
	);

	static char text[8192 + 1];

	CDBManager::instance().EscapeString(text, pkTab->skills, sizeof(pkTab->skills));
	queryLen += snprintf(pszQuery + queryLen, querySize - queryLen, "skill_level = '%s', ", text);

	CDBManager::instance().EscapeString(text, pkTab->quickslot, sizeof(pkTab->quickslot));
	queryLen += snprintf(pszQuery + queryLen, querySize - queryLen, "quickslot = '%s' ", text);

	queryLen += snprintf(pszQuery + queryLen, querySize - queryLen, " WHERE id=%d", pkTab->id);
	return queryLen;
}

CPlayerTableCache* CClientManager::GetPlayerCache(DWORD id)
{
	TPlayerTableCacheMap::iterator it = m_map_playerCache.find(id);

	if (it == m_map_playerCache.end())
		return NULL;

	TPlayerTable* pTable = it->second->Get(false);
	pTable->logoff_interval = GetCurrentTime() - it->second->GetLastUpdateTime();
	return it->second;
}

void CClientManager::PutPlayerCache(TPlayerTable* pNew)
{
	CPlayerTableCache* c;

	c = GetPlayerCache(pNew->id);

	if (!c)
	{
		c = new CPlayerTableCache;
		m_map_playerCache.insert(TPlayerTableCacheMap::value_type(pNew->id, c));
	}

	c->Put(pNew);
}

#ifdef __SKILL_COLOR__
void CClientManager::QUERY_SKILL_COLOR_LOAD(CPeer* peer, DWORD dwHandle, TPlayerLoadPacket* packet)
{
	CSKillColorCache* c;
	TSkillColor* p;
	if ((c = GetSkillColorCache(packet->player_id)))
	{
		p = c->Get();
		peer->EncodeHeader(HEADER_DG_SKILL_COLOR_LOAD, dwHandle, sizeof(p->dwSkillColor));
		peer->Encode(p->dwSkillColor, sizeof(p->dwSkillColor));
	}
	else
	{
		char szQuery[QUERY_MAX_LEN];
		snprintf(szQuery, sizeof(szQuery), "SELECT "
			"s1_col1, s1_col2, s1_col3, s1_col4, s1_col5"
			", s2_col1, s2_col2, s2_col3, s2_col4, s2_col5"
			", s3_col1, s3_col2, s3_col3, s3_col4, s3_col5"
			", s4_col1, s4_col2, s4_col3, s4_col4, s4_col5"
			", s5_col1, s5_col2, s5_col3, s5_col4, s5_col5"
			", s6_col1, s6_col2, s6_col3, s6_col4, s6_col5" // end of skills
			", s7_col1, s7_col2, s7_col3, s7_col4, s7_col5" // begin of buffs
			", s8_col1, s8_col2, s8_col3, s8_col4, s8_col5"
			", s9_col1, s9_col2, s9_col3, s9_col4, s9_col5"
			", s10_col1, s10_col2, s10_col3, s10_col4, s10_col5"
			", s11_col1, s11_col2, s11_col3, s11_col4, s11_col5"
#ifdef __WOLFMAN_CHARACTER__
			", s12_col1, s12_col2, s12_col3, s12_col4, s12_col5"
#endif
			" FROM skill_color%s WHERE player_id=%d",
			GetTablePostfix(), packet->player_id);
		CDBManager::instance().ReturnQuery(szQuery, QID_SKILL_COLOR, peer->GetHandle(), new ClientHandleInfo(dwHandle, packet->player_id), SQL_PLAYER_SKILLCOLOR);
	}
}
#endif
/*
 * PLAYER LOAD
 */
void CClientManager::QUERY_PLAYER_LOAD(CPeer* peer, DWORD dwHandle, TPlayerLoadPacket* packet)
{
	CLoginData * pLoginData = GetLoginDataByAID(packet->account_id);

	if (pLoginData)
	{
		for (int n = 0; n < PLAYER_PER_ACCOUNT; ++n)
			if (pLoginData->GetAccountRef().players[n].dwID != 0)
				DeleteLogoutPlayer(pLoginData->GetAccountRef().players[n].dwID);
	}

#ifdef __AUTH_LOGIN_TIMEOUT__
	if (pkLD->GetAuthLoginTimeOut() > 120)
	{
		sys_log(0, "PLAYER_LOAD_ERROR: GetAuthLoginTimeOut %d", pkLD->GetAuthLoginTimeOut());
		peer->EncodeHeader(HEADER_DG_PLAYER_LOAD_FAILED, dwHandle, 0);
		return;
	}
#endif // __AUTH_LOGIN_TIMEOUT__

	if (CPlayerTableCache * c = GetPlayerCache(packet->player_id))
	{
		CLoginData * pkLD = GetLoginDataByAID(packet->account_id);

		if (!pkLD || pkLD->IsPlay())
		{
			sys_log(0, "PLAYER_LOAD_ERROR: LoginData %p IsPlay %d", pkLD, pkLD ? pkLD->IsPlay() : 0);
			peer->EncodeHeader(HEADER_DG_PLAYER_LOAD_FAILED, dwHandle, 0);
			return;
		}

		TPlayerTable * pTab = c->Get();
		pkLD->SetPlay(true);

		thecore_memcpy(pTab->aiPremiumTimes, pkLD->GetPremiumPtr(), sizeof(pTab->aiPremiumTimes));

		peer->EncodeHeader(HEADER_DG_PLAYER_LOAD_SUCCESS, dwHandle, sizeof(TPlayerTable));
		peer->Encode(pTab, sizeof(TPlayerTable));

		if (packet->player_id != pkLD->GetLastPlayerID())
		{
			TPacketNeedLoginLogInfo logInfo;
			logInfo.dwPlayerID = packet->player_id;

			pkLD->SetLastPlayerID(packet->player_id);
			peer->EncodeHeader( HEADER_DG_NEED_LOGIN_LOG, dwHandle, sizeof(TPacketNeedLoginLogInfo) );
			peer->Encode( &logInfo, sizeof(TPacketNeedLoginLogInfo) );
		}

		char szQuery[1024] = { 0, };

#if defined(__GOLD_LIMIT_REWORK__) && defined(__CHEQUE_SYSTEM__)
		sys_log(0, "[PLAYER_LOAD] ID %s pid %d gold %lld cheque %d ", pTab->name, pTab->id, pTab->gold, pTab->cheque);
#elif defined(__GOLD_LIMIT_REWORK__) && !defined(__CHEQUE_SYSTEM__)
		sys_log(0, "[PLAYER_LOAD] ID %s pid %d gold %lld ", pTab->name, pTab->id, pTab->gold);
#elif !defined(__GOLD_LIMIT_REWORK__) && defined(__CHEQUE_SYSTEM__)
		sys_log(0, "[PLAYER_LOAD] ID %s pid %d gold %d cheque %d ", pTab->name, pTab->id, pTab->gold, pTab->cheque);
#else
		sys_log(0, "[PLAYER_LOAD] ID %s pid %d gold %d ", pTab->name, pTab->id, pTab->gold);
#endif

		{ // itemcacheset
			TItemCacheSet* pSet = GetItemCacheSet(pTab->id);

			if (pSet)
			{
				static std::vector<TPlayerItem> s_items;
				s_items.resize(pSet->size());

				DWORD dwCount = 0;
				TItemCacheSet::iterator it = pSet->begin();

				while (it != pSet->end())
				{
					CItemCache* c = *it++;
					TPlayerItem* p = c->Get();

					if (p->vnum)
						thecore_memcpy(&s_items[dwCount++], p, sizeof(TPlayerItem));
				}

				if (g_test_server)
					sys_log(0, "ITEM_CACHE: HIT! %s count: %u", pTab->name, dwCount);

				peer->EncodeHeader(HEADER_DG_ITEM_LOAD, dwHandle, sizeof(DWORD) + sizeof(TPlayerItem) * dwCount);
				peer->EncodeDWORD(dwCount);

				if (dwCount)
					peer->Encode(&s_items[0], sizeof(TPlayerItem) * dwCount);
			}
			else
			{
				snprintf(szQuery, sizeof(szQuery),
					"SELECT id,window+0,pos,count,vnum"
	#ifdef __BEGINNER_ITEM__
					",is_basic"
	#endif
	#ifdef __ITEM_EVOLUTION__
					",evolution"
	#endif
	#ifdef __ITEM_CHANGELOOK__
					",transmutation "
	#endif
					",socket0,socket1,socket2,"
	#ifdef __ITEM_EXTRA_SOCKET__
					"socket3,"
	#endif
					"attrtype0,attrvalue0,attrtype1,attrvalue1,attrtype2,attrvalue2,attrtype3,attrvalue3,attrtype4,attrvalue4,attrtype5,attrvalue5,attrtype6,attrvalue6 "
	#if defined(__ADDITIONAL_INVENTORY__) && defined(__SWITCHBOT__)
					"FROM item%s WHERE owner_id=%d AND (`window` in ('INVENTORY','EQUIPMENT','DRAGON_SOUL_INVENTORY','BELT_INVENTORY','UPGRADE_INVENTORY','BOOK_INVENTORY','STONE_INVENTORY','FLOWER_INVENTORY','ATTR_INVENTORY','CHEST_INVENTORY','SWITCHBOT'))",
	#elif defined(__ADDITIONAL_INVENTORY__) && !defined(__SWITCHBOT__)
					"FROM item%s WHERE owner_id=%d AND (`window` in ('INVENTORY','EQUIPMENT','DRAGON_SOUL_INVENTORY','BELT_INVENTORY','UPGRADE_INVENTORY','BOOK_INVENTORY','STONE_INVENTORY','FLOWER_INVENTORY','ATTR_INVENTORY','CHEST_INVENTORY'))",
	#elif !defined(__ADDITIONAL_INVENTORY__) && defined(__SWITCHBOT__)
					"FROM item%s WHERE owner_id=%d AND (`window` in ('INVENTORY','EQUIPMENT','DRAGON_SOUL_INVENTORY','BELT_INVENTORY','SWITCHBOT'))",
	#else
					"FROM item%s WHERE owner_id=%d AND (`window` in ('INVENTORY','EQUIPMENT','DRAGON_SOUL_INVENTORY','BELT_INVENTORY'))",
	#endif
					GetTablePostfix(), pTab->id);

				CDBManager::instance().ReturnQuery(szQuery,
					QID_ITEM,
					peer->GetHandle(),
					new ClientHandleInfo(dwHandle, pTab->id), SQL_PLAYER_ITEM);
			}
		} // itemcacheset
		{ // questcacheset
			TQuestCacheSet* pSet = GetQuestCacheSet(pTab->id);

			if (pSet)
			{
				static std::vector<TQuestTable> s_quests;
				s_quests.resize(pSet->size());

				DWORD dwCount = 0;
				TQuestCacheSet::iterator it = pSet->begin();

				while (it != pSet->end())
				{
					CQuestCache* c = *it++;
					TQuestTable* p = c->Get();

					if (p->dwPID)
						thecore_memcpy(&s_quests[dwCount++], p, sizeof(TQuestTable));

					// sys_log(0, "QUEST_CACHE: HIT! %u %s %d", p->dwPID, p->szName, p->lValue);
				}

				if (g_test_server)
					sys_log(0, "QUEST_CACHE: HIT! %u count: %u", pTab->id, dwCount);

				peer->EncodeHeader(HEADER_DG_QUEST_LOAD, dwHandle, sizeof(DWORD) + sizeof(TQuestTable) * dwCount);
				peer->EncodeDWORD(dwCount);

				if (dwCount)
					peer->Encode(&s_quests[0], sizeof(TQuestTable) * dwCount);
			}
			else
			{
				sys_err("player in cache but quest not, recaching %u", pTab->id);

				snprintf(szQuery, sizeof(szQuery),
					"SELECT dwPID,szName,szState,lValue FROM quest%s WHERE dwPID=%u AND lValue<>0",
					GetTablePostfix(), pTab->id);

				CDBManager::instance().ReturnQuery(szQuery,
					QID_QUEST,
					peer->GetHandle(),
					new ClientHandleInfo(dwHandle, pTab->id), SQL_PLAYER_QUEST);
			}
		} // questcacheset
		{ // affectcacheset
			TAffectCacheSet* pSet = GetAffectCacheSet(pTab->id);

			if (pSet)
			{
				static std::vector<TPacketAffectElement> s_affects;
				TPacketAffectElement* pElem;
				s_affects.resize(pSet->size());

				DWORD dwCount = 0;
				TAffectCacheSet::iterator it = pSet->begin();

				while (it != pSet->end())
				{
					CAffectCache* c = *it++;
					TPacketGDAddAffect* p = c->Get();

					pElem = &s_affects.at(dwCount++);
					pElem->dwType = p->elem.dwType;
					pElem->bApplyOn = p->elem.bApplyOn;
					pElem->lApplyValue = p->elem.lApplyValue;
					pElem->dwFlag = p->elem.dwFlag;
					pElem->lDuration = p->elem.lDuration;
					pElem->lSPCost = p->elem.lSPCost;

					// sys_log(0, "AFFECT_CACHE: HIT! %u %d %d", p->dwPID, p->elem.dwType, p->elem.bApplyOn);
				}

				if (g_test_server)
					sys_log(0, "AFFECT_CACHE: HIT! %u count: %u", pTab->id, dwCount);

				peer->EncodeHeader(HEADER_DG_AFFECT_LOAD, dwHandle, sizeof(DWORD) + sizeof(DWORD) + sizeof(TPacketAffectElement) * dwCount);
				peer->EncodeDWORD(pTab->id);
				peer->EncodeDWORD(dwCount);

				if (dwCount)
					peer->Encode(&s_affects[0], sizeof(TPacketAffectElement) * dwCount);
			}
			else
			{
				sys_err("player in cache but affect not, recaching %u", pTab->id);

				snprintf(szQuery, sizeof(szQuery),
					"SELECT dwPID,bType,bApplyOn,lApplyValue,dwFlag,lDuration,lSPCost FROM affect%s WHERE dwPID=%d",
					GetTablePostfix(), pTab->id);
				CDBManager::instance().ReturnQuery(szQuery, QID_AFFECT, peer->GetHandle(), new ClientHandleInfo(dwHandle, pTab->id), SQL_PLAYER_AFFECT);
			}
		} // affectcacheset
#ifdef __BATTLE_PASS_SYSTEM__
		{
			TBattlePassCacheSet* pSet = GetBattlePassCacheSet(pTab->id);

			if (pSet)
			{
				static std::vector<TPlayerBattlePassMission> s_pass;
				TPlayerBattlePassMission* pElem;
				s_pass.resize(pSet->size());

				DWORD dwCount = 0;
				TBattlePassCacheSet::iterator it = pSet->begin();

				while (it != pSet->end())
				{
					CBattlePassCache* c = *it++;
					TPlayerBattlePassMission* p = c->Get();

					pElem = &s_pass.at(dwCount++);
					pElem->dwPlayerId = pTab->id;
					pElem->dwMissionId = p->dwMissionId;
					pElem->dwBattlePassId = p->dwBattlePassId;
					pElem->dwExtraInfo = p->dwExtraInfo;
					pElem->bCompleted = p->bCompleted;
					pElem->bIsUpdated = 0;
				}

				if (g_test_server)
					sys_log(0, "BATTLE_PASS_CACHE: HIT! %u count: %u", pTab->id, dwCount);


				peer->EncodeHeader(HEADER_DG_BATTLE_PASS_LOAD, dwHandle, sizeof(DWORD) + sizeof(DWORD) + sizeof(TPlayerBattlePassMission) * dwCount);
				peer->EncodeDWORD(pTab->id);
				peer->EncodeDWORD(dwCount);
				if (dwCount)
					peer->Encode(&s_pass[0], sizeof(TPlayerBattlePassMission) * dwCount);
			}
			else
			{
				// Load all missions from table
				snprintf(szQuery, sizeof(szQuery),
					"SELECT player_id, mission_id, battle_pass_id, extra_info, completed FROM battle_pass%s WHERE player_id = %d", GetTablePostfix(), pTab->id);
				CDBManager::instance().ReturnQuery(szQuery, QID_BATTLE_PASS, peer->GetHandle(), new ClientHandleInfo(dwHandle, pTab->id), SQL_PLAYER_BATTLE_PASS);
			}
		}
#endif // __BATTLE_PASS_SYSTEM__

	}
	else
	{
		sys_log(0, "[PLAYER_LOAD] Load from PlayerDB pid[%d]", packet->player_id);

		char queryStr[QUERY_MAX_LEN];

		snprintf(queryStr, sizeof(queryStr),
			"SELECT "
			"id,name,job,dir,x,y,z,map_index,exit_x,exit_y,exit_map_index,hp,mp,stamina,random_hp,random_sp,playtime,"
			"gold,"
#ifdef __CHEQUE_SYSTEM__
			"cheque,"
#endif
			"level,level_step,st,ht,dx,iq,exp,"
			"stat_point,skill_point,sub_skill_point,stat_reset_count,part_base,part_hair,"
#ifdef __ACCE_SYSTEM__
			"part_acce,"
#endif
			"skill_level,quickslot,skill_group,alignment,"
#ifdef __RANK_SYSTEM__
			"rank,"
#endif
#ifdef __LANDRANK_SYSTEM__
			"land_rank,"
#endif
#ifdef __REBORN_SYSTEM__
			"reborn,"
#endif
			"horse_level,horse_riding,horse_hp,horse_hp_droptime,horse_stamina,"
			"UNIX_TIMESTAMP(NOW())-UNIX_TIMESTAMP(last_play),"
#ifdef __CHARACTER_WINDOW_RENEWAL__
			"UNIX_TIMESTAMP(last_play),"
#endif
			"horse_skill_point"
#ifdef ENABLE_PLAYER_STATISTICS
			",statistics_killed_shinsoo"
			",statistics_killed_chunjo"
			",statistics_killed_jinno"
			",statistics_total_kill"
			",statistics_duel_won"
			",statistics_duel_lost"
			",statistics_killed_monster"
			",statistics_killed_stone"
			",statistics_killed_boss"
			",statistics_completed_dungeon"
			",statistics_taked_fish"
			",statistics_best_stone_damage"
			",statistics_best_boss_damage"
#endif
#ifdef __BATTLE_PASS_SYSTEM__
			",UNIX_TIMESTAMP(battle_pass_end)"
#endif
#ifdef ENABLE_CHAT_COLOR_SYSTEM
			",color"
#endif
#ifdef __TEAM_SYSTEM__
			",team"
#endif
#ifdef __MONIKER_SYSTEM__
			",moniker"
#endif
#ifdef ENABLE_LOVE_SYSTEM
			",love1,love2"
#endif // ENABLE_LOVE_SYSTEM
#ifdef ENABLE_WORD_SYSTEM
			",word"
#endif // ENABLE_WORD_SYSTEM
			" FROM player%s WHERE id=%d",
			GetTablePostfix(), packet->player_id);

		ClientHandleInfo* pkInfo = new ClientHandleInfo(dwHandle, packet->player_id);
		pkInfo->account_id = packet->account_id;
		CDBManager::instance().ReturnQuery(queryStr, QID_PLAYER, peer->GetHandle(), pkInfo);

		snprintf(queryStr, sizeof(queryStr),
			"SELECT id,`window`+0,pos,count,vnum"
#ifdef __BEGINNER_ITEM__
			",is_basic"
#endif
#ifdef __ITEM_EVOLUTION__
			",evolution"
#endif
#ifdef __ITEM_CHANGELOOK__
			",transmutation"
#endif
			",socket0,socket1,socket2,"
#ifdef __ITEM_EXTRA_SOCKET__
			"socket3,"
#endif
			"attrtype0,attrvalue0,attrtype1,attrvalue1,attrtype2,attrvalue2,attrtype3,attrvalue3,attrtype4,attrvalue4,attrtype5,attrvalue5,attrtype6,attrvalue6 "
#if defined(__ADDITIONAL_INVENTORY__) && defined(__SWITCHBOT__)
			"FROM item%s WHERE owner_id=%d AND (`window` in ('INVENTORY','EQUIPMENT','DRAGON_SOUL_INVENTORY','BELT_INVENTORY','UPGRADE_INVENTORY','BOOK_INVENTORY','STONE_INVENTORY','FLOWER_INVENTORY','ATTR_INVENTORY','CHEST_INVENTORY', 'SWITCHBOT'))",
#elif defined(__ADDITIONAL_INVENTORY__) && !defined(__SWITCHBOT__)
			"FROM item%s WHERE owner_id=%d AND (`window` in ('INVENTORY','EQUIPMENT','DRAGON_SOUL_INVENTORY','BELT_INVENTORY','UPGRADE_INVENTORY','BOOK_INVENTORY','STONE_INVENTORY','FLOWER_INVENTORY','ATTR_INVENTORY','CHEST_INVENTORY'))",
#elif !defined(__ADDITIONAL_INVENTORY__) && defined(__SWITCHBOT__)
			"FROM item%s WHERE owner_id=%d AND (`window` in ('INVENTORY','EQUIPMENT','DRAGON_SOUL_INVENTORY','BELT_INVENTORY','SWITCHBOT'))",
#else
			"FROM item%s WHERE owner_id=%d AND (`window` in ('INVENTORY','EQUIPMENT','DRAGON_SOUL_INVENTORY','BELT_INVENTORY'))",
#endif
			GetTablePostfix(), packet->player_id);
		CDBManager::instance().ReturnQuery(queryStr, QID_ITEM, peer->GetHandle(), new ClientHandleInfo(dwHandle, packet->player_id), SQL_PLAYER_ITEM);

		// Quest
		snprintf(queryStr, sizeof(queryStr),
			"SELECT dwPID,szName,szState,lValue FROM quest%s WHERE dwPID=%d AND lValue<>0",
			GetTablePostfix(), packet->player_id);
		CDBManager::instance().ReturnQuery(queryStr, QID_QUEST, peer->GetHandle(), new ClientHandleInfo(dwHandle, packet->player_id), SQL_PLAYER_QUEST);


		// Affect
		snprintf(queryStr, sizeof(queryStr),
			"SELECT dwPID,bType,bApplyOn,lApplyValue,dwFlag,lDuration,lSPCost FROM affect%s WHERE dwPID=%d",
			GetTablePostfix(), packet->player_id);
		CDBManager::instance().ReturnQuery(queryStr, QID_AFFECT, peer->GetHandle(), new ClientHandleInfo(dwHandle, packet->player_id, packet->account_id), SQL_PLAYER_AFFECT);

#ifdef __BATTLE_PASS_SYSTEM__
		// Load all missions from table
		snprintf(queryStr, sizeof(queryStr),
			"SELECT player_id, mission_id, battle_pass_id, extra_info, completed FROM battle_pass%s WHERE player_id = %d", GetTablePostfix(), packet->player_id);
		CDBManager::instance().ReturnQuery(queryStr, QID_BATTLE_PASS, peer->GetHandle(), new ClientHandleInfo(dwHandle, packet->player_id), SQL_PLAYER_BATTLE_PASS);
#endif
	}

#ifdef __OFFLINE_SHOP__
	OfflineshopLoadShopSafebox(peer, packet->player_id);
#endif
}

bool CreatePlayerTableFromRes(MYSQL_RES* res, TPlayerTable* pkTab)
{
	if (mysql_num_rows(res) == 0)
		return false;

	memset(pkTab, 0, sizeof(TPlayerTable));

	MYSQL_ROW row = mysql_fetch_row(res);

	int	col = 0;

	str_to_number(pkTab->id, row[col++]);
	strlcpy(pkTab->name, row[col++], sizeof(pkTab->name));
	str_to_number(pkTab->job, row[col++]);
	str_to_number(pkTab->dir, row[col++]);
	str_to_number(pkTab->x, row[col++]);
	str_to_number(pkTab->y, row[col++]);
	str_to_number(pkTab->z, row[col++]);
	str_to_number(pkTab->lMapIndex, row[col++]);
	str_to_number(pkTab->lExitX, row[col++]);
	str_to_number(pkTab->lExitY, row[col++]);
	str_to_number(pkTab->lExitMapIndex, row[col++]);
	str_to_number(pkTab->hp, row[col++]);
	str_to_number(pkTab->sp, row[col++]);
	str_to_number(pkTab->stamina, row[col++]);
	str_to_number(pkTab->sRandomHP, row[col++]);
	str_to_number(pkTab->sRandomSP, row[col++]);
	str_to_number(pkTab->playtime, row[col++]);
	str_to_number(pkTab->gold, row[col++]);
#ifdef __CHEQUE_SYSTEM__
	str_to_number(pkTab->cheque, row[col++]);
#endif
	str_to_number(pkTab->level, row[col++]);
	str_to_number(pkTab->level_step, row[col++]);
	str_to_number(pkTab->st, row[col++]);
	str_to_number(pkTab->ht, row[col++]);
	str_to_number(pkTab->dx, row[col++]);
	str_to_number(pkTab->iq, row[col++]);
	str_to_number(pkTab->exp, row[col++]);
	str_to_number(pkTab->stat_point, row[col++]);
	str_to_number(pkTab->skill_point, row[col++]);
	str_to_number(pkTab->sub_skill_point, row[col++]);
	str_to_number(pkTab->stat_reset_count, row[col++]);
	str_to_number(pkTab->part_base, row[col++]);
	str_to_number(pkTab->parts[PART_HAIR], row[col++]);
#ifdef __ACCE_SYSTEM__
	str_to_number(pkTab->parts[PART_ACCE], row[col++]);
#endif

	if (row[col])
		thecore_memcpy(pkTab->skills, row[col], sizeof(pkTab->skills));
	else
		memset(&pkTab->skills, 0, sizeof(pkTab->skills));

	col++;

	if (row[col])
		thecore_memcpy(pkTab->quickslot, row[col], sizeof(pkTab->quickslot));
	else
		memset(pkTab->quickslot, 0, sizeof(pkTab->quickslot));

	col++;

	str_to_number(pkTab->skill_group, row[col++]);
	str_to_number(pkTab->lAlignment, row[col++]);
#ifdef __RANK_SYSTEM__
	str_to_number(pkTab->lRank, row[col++]);
#endif
#ifdef __LANDRANK_SYSTEM__
	str_to_number(pkTab->lLandRank, row[col++]);
#endif
#ifdef __REBORN_SYSTEM__
	str_to_number(pkTab->lReborn, row[col++]);
#endif
	str_to_number(pkTab->horse.bLevel, row[col++]);
	str_to_number(pkTab->horse.bRiding, row[col++]);
	str_to_number(pkTab->horse.sHealth, row[col++]);
	str_to_number(pkTab->horse.dwHorseHealthDropTime, row[col++]);
	str_to_number(pkTab->horse.sStamina, row[col++]);
	str_to_number(pkTab->logoff_interval, row[col++]);
#ifdef __CHARACTER_WINDOW_RENEWAL__
	str_to_number(pkTab->last_play, row[col++]);
#endif
	str_to_number(pkTab->horse_skill_point, row[col++]);
#ifdef ENABLE_PLAYER_STATISTICS
	str_to_number(pkTab->iKilledShinsoo, row[col++]);
	str_to_number(pkTab->iKilledChunjo, row[col++]);
	str_to_number(pkTab->iKilledJinno, row[col++]);
	str_to_number(pkTab->iTotalKill, row[col++]);
	str_to_number(pkTab->iDuelWon, row[col++]);
	str_to_number(pkTab->iDuelLost, row[col++]);
	str_to_number(pkTab->iKilledMonster, row[col++]);
	str_to_number(pkTab->iKilledStone, row[col++]);
	str_to_number(pkTab->iKilledBoss, row[col++]);
	str_to_number(pkTab->iCompletedDungeon, row[col++]);
	str_to_number(pkTab->iTakedFish, row[col++]);
	str_to_number(pkTab->iBestStoneDamage, row[col++]);
	str_to_number(pkTab->iBestBossDamage, row[col++]);
#endif
#ifdef __BATTLE_PASS_SYSTEM__
	str_to_number(pkTab->dwBattlePassEndTime, row[col++]);
#endif
#ifdef ENABLE_CHAT_COLOR_SYSTEM
	str_to_number(pkTab->color, row[col++]);
#endif
#ifdef __TEAM_SYSTEM__
	str_to_number(pkTab->lTeam, row[col++]);
#endif
#ifdef __MONIKER_SYSTEM__
	strcpy(pkTab->moniker, row[col++]);
#endif
#ifdef ENABLE_LOVE_SYSTEM
	strcpy(pkTab->love1, row[col++]);
	strcpy(pkTab->love2, row[col++]);
#endif // ENABLE_LOVE_SYSTEM
#ifdef ENABLE_WORD_SYSTEM
	strcpy(pkTab->word, row[col++]);
#endif // ENABLE_WORD_SYSTEM
	// reset sub_skill_point
	{
		pkTab->skills[123].bLevel = 0; // SKILL_CREATE

		if (pkTab->level > 9)
		{
			int max_point = pkTab->level - 9;

			int skill_point =
				MIN(20, pkTab->skills[121].bLevel) +
				MIN(20, pkTab->skills[124].bLevel) +
				MIN(10, pkTab->skills[131].bLevel) +
				MIN(20, pkTab->skills[141].bLevel) +
				MIN(20, pkTab->skills[142].bLevel);

			pkTab->sub_skill_point = max_point - skill_point;
		}
		else
			pkTab->sub_skill_point = 0;
	}

	return true;
}

void CClientManager::RESULT_COMPOSITE_PLAYER(CPeer* peer, SQLMsg* pMsg, DWORD dwQID)
{
	CQueryInfo* qi = (CQueryInfo*)pMsg->pvUserData;
	std::unique_ptr<ClientHandleInfo> info((ClientHandleInfo*)qi->pvData);

	MYSQL_RES* pSQLResult = pMsg->Get()->pSQLResult;
	if (!pSQLResult)
	{
		sys_err("null MYSQL_RES QID %u", dwQID);
		return;
	}

	switch (dwQID)
	{
	case QID_PLAYER:
		sys_log(0, "QID_PLAYER %u %u", info->dwHandle, info->player_id);
		RESULT_PLAYER_LOAD(peer, pSQLResult, info.get());

		break;

	case QID_ITEM:
		sys_log(0, "QID_ITEM %u", info->dwHandle);
		RESULT_ITEM_LOAD(peer, pSQLResult, info->dwHandle, info->player_id);
		break;

	case QID_QUEST:
		sys_log(0, "QID_QUEST %u", info->dwHandle);
		RESULT_QUEST_LOAD(peer, pSQLResult, info->dwHandle, info->player_id);
		break;

	case QID_AFFECT:
		sys_log(0, "QID_AFFECT %u", info->dwHandle);
		RESULT_AFFECT_LOAD(peer, pSQLResult, info->dwHandle, info->player_id);
		break;

#ifdef __SKILL_COLOR__
	case QID_SKILL_COLOR:
		sys_log(0, "QID_SKILL_COLOR %u %u", info->dwHandle, info->player_id);
		RESULT_SKILL_COLOR_LOAD(peer, pSQLResult, info->dwHandle);
		break;
#endif

#ifdef __BATTLE_PASS_SYSTEM__
	case QID_BATTLE_PASS:
		sys_log(0, "QID_BATTLE_PASS %u", info->dwHandle);
		RESULT_BATTLE_PASS_LOAD(peer, pSQLResult, info->dwHandle, info->player_id);
		break;
#endif

	}
}

void CClientManager::RESULT_PLAYER_LOAD(CPeer* peer, MYSQL_RES* pRes, ClientHandleInfo* pkInfo)
{
	TPlayerTable tab;

	if (!CreatePlayerTableFromRes(pRes, &tab))
	{
		peer->EncodeHeader(HEADER_DG_PLAYER_LOAD_FAILED, pkInfo->dwHandle, 0);
		return;
	}

	CLoginData* pkLD = GetLoginDataByAID(pkInfo->account_id);

	if (!pkLD || pkLD->IsPlay())
	{
		sys_log(0, "PLAYER_LOAD_ERROR: LoginData %p IsPlay %d", pkLD, pkLD ? pkLD->IsPlay() : 0);
		peer->EncodeHeader(HEADER_DG_PLAYER_LOAD_FAILED, pkInfo->dwHandle, 0);
		return;
	}

	pkLD->SetPlay(true);
	thecore_memcpy(tab.aiPremiumTimes, pkLD->GetPremiumPtr(), sizeof(tab.aiPremiumTimes));

	peer->EncodeHeader(HEADER_DG_PLAYER_LOAD_SUCCESS, pkInfo->dwHandle, sizeof(TPlayerTable));
	peer->Encode(&tab, sizeof(TPlayerTable));

	if (tab.id != pkLD->GetLastPlayerID())
	{
		TPacketNeedLoginLogInfo logInfo;
		logInfo.dwPlayerID = tab.id;

		pkLD->SetLastPlayerID(tab.id);
		peer->EncodeHeader( HEADER_DG_NEED_LOGIN_LOG, pkInfo->dwHandle, sizeof(TPacketNeedLoginLogInfo) );
		peer->Encode( &logInfo, sizeof(TPacketNeedLoginLogInfo) );
	}
}

void CClientManager::RESULT_ITEM_LOAD(CPeer* peer, MYSQL_RES* pRes, DWORD dwHandle, DWORD dwPID)
{
	static std::vector<TPlayerItem> s_items;

	CreateItemTableFromRes(pRes, &s_items, dwPID);

	DWORD dwCount = s_items.size();

	peer->EncodeHeader(HEADER_DG_ITEM_LOAD, dwHandle, sizeof(DWORD) + sizeof(TPlayerItem) * dwCount);
	peer->EncodeDWORD(dwCount);

	CreateItemCacheSet(dwPID);

	// ITEM_LOAD_LOG_ATTACH_PID
	sys_log(0, "ITEM_LOAD: count %u pid %u", dwCount, dwPID);
	// END_OF_ITEM_LOAD_LOG_ATTACH_PID

	if (dwCount)
	{
		peer->Encode(&s_items[0], sizeof(TPlayerItem) * dwCount);

		for (DWORD i = 0; i < dwCount; ++i)
			PutItemCache(&s_items[i], true);
	}
}

#ifdef __SKILL_COLOR__
void CClientManager::RESULT_SKILL_COLOR_LOAD(CPeer* peer, MYSQL_RES* pRes, DWORD dwHandle)
{
	DWORD dwSkillColor[ESkillColorLength::MAX_SKILL_COUNT + ESkillColorLength::MAX_BUFF_COUNT][ESkillColorLength::MAX_EFFECT_COUNT];
	memset(dwSkillColor, 0, sizeof(dwSkillColor));

	CreateSkillColorTableFromRes(pRes, *dwSkillColor);
	sys_log(0, "SKILL_COLOR_LOAD %i, %i, %i, %i,", dwSkillColor[0][0], dwSkillColor[1][1], dwSkillColor[2][2], dwSkillColor[3][3]);
	peer->EncodeHeader(HEADER_DG_SKILL_COLOR_LOAD, dwHandle, sizeof(dwSkillColor));
	peer->Encode(&dwSkillColor, sizeof(dwSkillColor));
}
#endif

void CClientManager::RESULT_AFFECT_LOAD(CPeer* peer, MYSQL_RES* pRes, DWORD dwHandle, DWORD dwRealPID)
{
	static std::vector<TPacketAffectElement> s_affects;

	CreateAffectTableFromRes(pRes, &s_affects, dwRealPID);
	DWORD dwCount = s_affects.size();

	peer->EncodeHeader(HEADER_DG_AFFECT_LOAD, dwHandle, sizeof(DWORD) + sizeof(DWORD) + sizeof(TPacketAffectElement) * dwCount);
	peer->Encode(&dwRealPID, sizeof(DWORD));
	peer->Encode(&dwCount, sizeof(DWORD));

	sys_log(0, "AFFECT_LOAD: count %u pid %u", dwCount, dwRealPID);

	TAffectCacheSetPtrMap::iterator it = m_map_pkAffectCacheSetPtr.find(dwRealPID);

	TAffectCacheSet* pSet = nullptr;

	if (it != m_map_pkAffectCacheSetPtr.end())
	{
		sys_err("affload already in cache!");
		pSet = it->second;

		TAffectCacheSet::iterator it_set = pSet->begin();

		while (it_set != pSet->end())
		{
			CAffectCache* c = *it_set++;
			delete c;
		}

		pSet->clear();
	}
	else
	{
		pSet = CreateAffectCacheSet(dwRealPID);
	}

	if (dwCount)
	{
		peer->Encode(&s_affects[0], sizeof(TPacketAffectElement) * dwCount);

		TPacketGDAddAffect p;
		p.dwPID = dwRealPID;
		for (DWORD i = 0; i < dwCount; ++i)
		{
			p.elem.dwType = s_affects.at(i).dwType;
			p.elem.bApplyOn = s_affects.at(i).bApplyOn;
			p.elem.lApplyValue = s_affects.at(i).lApplyValue;
			p.elem.dwFlag = s_affects.at(i).dwFlag;
			p.elem.lDuration = s_affects.at(i).lDuration;
			p.elem.lSPCost = s_affects.at(i).lSPCost;

			// sys_log(0, "AFFCACHE %d %d %d", p.elem.dwType, p.elem.bApplyOn, p.elem.lApplyValue);

			CAffectCache* c = new CAffectCache;
			c->Put(&p);
			pSet->insert(c);
		}
	}
}

void CClientManager::RESULT_QUEST_LOAD(CPeer* peer, MYSQL_RES* pRes, DWORD dwHandle, DWORD dwPID)
{
	static std::vector<TQuestTable> s_quests;

	CreateQuestTableFromRes(pRes, &s_quests, dwPID);
	DWORD dwCount = s_quests.size();

	peer->EncodeHeader(HEADER_DG_QUEST_LOAD, dwHandle, sizeof(DWORD) + sizeof(TQuestTable) * dwCount);
	peer->EncodeDWORD(dwCount);

	sys_log(0, "QUEST_LOAD: count %u pid %u", dwCount, dwPID);

	TQuestCacheSetPtrMap::iterator it = m_map_pkQuestCacheSetPtr.find(dwPID);

	TQuestCacheSet* pSet = nullptr;

	if (it != m_map_pkQuestCacheSetPtr.end())
	{
		sys_err("questload already in cache!");
		pSet = it->second;

		TQuestCacheSet::iterator it_set = pSet->begin();

		while (it_set != pSet->end())
		{
			CQuestCache* c = *it_set++;
			delete c;
		}

		pSet->clear();
	}
	else
	{
		pSet = CreateQuestCacheSet(dwPID);
	}

	if (dwCount)
	{
		peer->Encode(&s_quests[0], sizeof(TQuestTable) * dwCount);

		for (DWORD i = 0; i < dwCount; ++i)
		{
			CQuestCache* c = new CQuestCache;
			c->Put(&s_quests.at(i));
			pSet->insert(c);
		}
	}
}

/*
 * PLAYER SAVE
 */
void CClientManager::QUERY_PLAYER_SAVE(CPeer* peer, DWORD dwHandle, TPlayerTable* pkTab)
{
	if (g_test_server)
		sys_log(0, "PLAYER_SAVE: %s", pkTab->name);

	PutPlayerCache(pkTab);
}

typedef std::map<DWORD, time_t> time_by_id_map_t;
static time_by_id_map_t s_createTimeByAccountID;

/*
 * PLAYER CREATE
 */
void CClientManager::__QUERY_PLAYER_CREATE(CPeer* peer, DWORD dwHandle, TPlayerCreatePacket* packet)
{
	char	queryStr[QUERY_MAX_LEN];
	int		queryLen;
	int		player_id;

	time_by_id_map_t::iterator it = s_createTimeByAccountID.find(packet->account_id);

	if (it != s_createTimeByAccountID.end())
	{
		time_t curtime = time(0);

		if (curtime - it->second < 30)
		{
			peer->EncodeHeader(HEADER_DG_PLAYER_CREATE_FAILED, dwHandle, 0);
			return;
		}
	}

	queryLen = snprintf(queryStr, sizeof(queryStr),
		"SELECT pid%u FROM player_index%s WHERE id=%d", packet->account_index + 1, GetTablePostfix(), packet->account_id);

	std::unique_ptr<SQLMsg> pMsg0(CDBManager::instance().DirectQuery(queryStr));

	if (pMsg0->Get()->uiNumRows != 0)
	{
		if (!pMsg0->Get()->pSQLResult)
		{
			peer->EncodeHeader(HEADER_DG_PLAYER_CREATE_FAILED, dwHandle, 0);
			return;
		}

		MYSQL_ROW row = mysql_fetch_row(pMsg0->Get()->pSQLResult);

		DWORD dwPID = 0; str_to_number(dwPID, row[0]);
		if (row[0] && dwPID > 0)
		{
			peer->EncodeHeader(HEADER_DG_PLAYER_CREATE_ALREADY, dwHandle, 0);
			sys_log(0, "ALREADY EXIST AccountChrIdx %d ID %d", packet->account_index, dwPID);
			return;
		}
	}
	else
	{
		peer->EncodeHeader(HEADER_DG_PLAYER_CREATE_FAILED, dwHandle, 0);
		return;
	}

	snprintf(queryStr, sizeof(queryStr),
		"SELECT COUNT(*) as count FROM player%s WHERE name='%s'", GetTablePostfix(), packet->player_table.name);

	std::unique_ptr<SQLMsg> pMsg1(CDBManager::instance().DirectQuery(queryStr));

	if (pMsg1->Get()->uiNumRows)
	{
		if (!pMsg1->Get()->pSQLResult)
		{
			peer->EncodeHeader(HEADER_DG_PLAYER_CREATE_FAILED, dwHandle, 0);
			return;
		}

		MYSQL_ROW row = mysql_fetch_row(pMsg1->Get()->pSQLResult);

		if (*row[0] != '0')
		{
			sys_log(0, "ALREADY EXIST name %s, row[0] %s query %s", packet->player_table.name, row[0], queryStr);
			peer->EncodeHeader(HEADER_DG_PLAYER_CREATE_ALREADY, dwHandle, 0);
			return;
		}
	}
	else
	{
		peer->EncodeHeader(HEADER_DG_PLAYER_CREATE_FAILED, dwHandle, 0);
		return;
	}

	queryLen = snprintf(queryStr, sizeof(queryStr),
		"INSERT INTO player%s ("
		"id,"
		"account_id,"
		"name,"
		"st,"
		"ht,"
		"dx,"
		"iq,"
		"job,"
		"x,"
		"y,"
		"z,"
		"hp,"
		"mp,"
		"random_hp,"
		"random_sp,"
		"stat_point,"
		"stamina,"
		"part_base,"
		"part_main,"
		"part_hair,"
#ifdef __ACCE_SYSTEM__
		"part_acce,"
#endif
		"gold,"
#ifdef __CHEQUE_SYSTEM__
		"cheque,"
#endif
		"playtime,"
		"skill_level,"
		"quickslot"
		")"
		"VALUES("
		"0,"
		"%u,"
		"'%s',"
		"%d," // st
		"%d," // ht
		"%d," // dx
		"%d," // iq
		"%d," // job
		"%d," // x
		"%d," // y
		"%d," // z
		"%lld," // hp
		"%d," // sp
		"%lld," // randomHP
		"%d," // randomSP
		"%d," // statpoint
		"%d," // stamina
		"%d," // part base
		"%d," // part main
		"%d," // part hair
		"%d," // part acce
#ifdef __GOLD_LIMIT_REWORK__
		"%lld,"
#else
		"%d,"
#endif
#ifdef __CHEQUE_SYSTEM__
		"%d,"
#endif
		"0,",
		GetTablePostfix(),
		packet->account_id,
		packet->player_table.name,
		packet->player_table.st,
		packet->player_table.ht,
		packet->player_table.dx,
		packet->player_table.iq,
		packet->player_table.job,
		packet->player_table.x,
		packet->player_table.y,
		packet->player_table.z,
		packet->player_table.hp,
		packet->player_table.sp,
		packet->player_table.sRandomHP,
		packet->player_table.sRandomSP,
		packet->player_table.stat_point,
		packet->player_table.stamina,
		packet->player_table.part_base,
		packet->player_table.parts[PART_MAIN],
		packet->player_table.parts[PART_HAIR],
#ifdef __ACCE_SYSTEM__
		packet->player_table.parts[PART_ACCE],
#endif
		packet->player_table.gold,
#ifdef __CHEQUE_SYSTEM__
		packet->player_table.cheque
#endif
	);

#ifdef __GOLD_LIMIT_REWORK__
	sys_log(0, "PlayerCreate accountid %d name %s level %d gold %lld, st %d ht %d job %d",
#else
	sys_log(0, "PlayerCreate accountid %d name %s level %d gold %d, st %d ht %d job %d",
#endif
		packet->account_id,
		packet->player_table.name,
		packet->player_table.level,
		packet->player_table.gold,
		packet->player_table.st,
		packet->player_table.ht,
		packet->player_table.job);

	static char text[4096 + 1];

	CDBManager::instance().EscapeString(text, packet->player_table.skills, sizeof(packet->player_table.skills));
	queryLen += snprintf(queryStr + queryLen, sizeof(queryStr) - queryLen, "'%s', ", text);
	if (g_test_server)
		sys_log(0, "Create_Player queryLen[%d] TEXT[%s]", queryLen, text);

	CDBManager::instance().EscapeString(text, packet->player_table.quickslot, sizeof(packet->player_table.quickslot));
	queryLen += snprintf(queryStr + queryLen, sizeof(queryStr) - queryLen, "'%s'", text);

	queryLen += snprintf(queryStr + queryLen, sizeof(queryStr) - queryLen, ")");

	std::unique_ptr<SQLMsg> pMsg2(CDBManager::instance().DirectQuery(queryStr));
	if (g_test_server)
		sys_log(0, "Create_Player queryLen[%d] TEXT[%s]", queryLen, text);

	if (pMsg2->Get()->uiAffectedRows <= 0)
	{
		peer->EncodeHeader(HEADER_DG_PLAYER_CREATE_ALREADY, dwHandle, 0);
		sys_log(0, "ALREADY EXIST3 query: %s AffectedRows %lu", queryStr, pMsg2->Get()->uiAffectedRows);
		return;
	}

	player_id = pMsg2->Get()->uiInsertID;

	snprintf(queryStr, sizeof(queryStr), "UPDATE player_index%s SET pid%d=%d WHERE id=%d",
		GetTablePostfix(), packet->account_index + 1, player_id, packet->account_id);
	std::unique_ptr<SQLMsg> pMsg3(CDBManager::instance().DirectQuery(queryStr));

	if (pMsg3->Get()->uiAffectedRows <= 0)
	{
		sys_err("QUERY_ERROR: %s", queryStr);

		snprintf(queryStr, sizeof(queryStr), "DELETE FROM player%s WHERE id=%d", GetTablePostfix(), player_id);
		delete CDBManager::instance().DirectQuery(queryStr);

		peer->EncodeHeader(HEADER_DG_PLAYER_CREATE_FAILED, dwHandle, 0);
		return;
	}

	TPacketDGCreateSuccess pack;
	memset(&pack, 0, sizeof(pack));

	pack.bAccountCharacterIndex = packet->account_index;

	pack.player.dwID = player_id;
	strlcpy(pack.player.szName, packet->player_table.name, sizeof(pack.player.szName));
	pack.player.byJob = packet->player_table.job;
	pack.player.byLevel = packet->player_table.level;
	pack.player.dwPlayMinutes = 0;
	pack.player.byST = packet->player_table.st;
	pack.player.byHT = packet->player_table.ht;
	pack.player.byDX = packet->player_table.dx;
	pack.player.byIQ = packet->player_table.iq;
	pack.player.wMainPart = packet->player_table.parts[PART_MAIN];
	pack.player.wHairPart = packet->player_table.parts[PART_HAIR];
#ifdef __ACCE_SYSTEM__
	pack.player.wAccePart = packet->player_table.parts[PART_ACCE];
#endif
	pack.player.x = packet->player_table.x;
	pack.player.y = packet->player_table.y;
#ifdef __CHARACTER_WINDOW_RENEWAL__
	pack.player.last_play = packet->player_table.last_play;
#endif

	peer->EncodeHeader(HEADER_DG_PLAYER_CREATE_SUCCESS, dwHandle, sizeof(TPacketDGCreateSuccess));
	peer->Encode(&pack, sizeof(TPacketDGCreateSuccess));

	sys_log(0, "CREATE_PLAYER name %s job %d", pack.player.szName, pack.player.byJob);

	s_createTimeByAccountID[packet->account_id] = time(0);
}

/*
 * PLAYER DELETE
 */
void CClientManager::__QUERY_PLAYER_DELETE(CPeer* peer, DWORD dwHandle, TPlayerDeletePacket* packet)
{
	if (!packet->login[0] || !packet->player_id || packet->account_index >= PLAYER_PER_ACCOUNT)
		return;

	CLoginData* ld = GetLoginDataByLogin(packet->login);

	if (!ld)
	{
		peer->EncodeHeader(HEADER_DG_PLAYER_DELETE_FAILED, dwHandle, 1);
		peer->EncodeBYTE(packet->account_index);
		return;
	}

	// CHECK
	TAccountTable & r = ld->GetAccountRef();
	if (strlen(r.social_id) < 7 || strncmp(packet->private_code, r.social_id + strlen(r.social_id) - 7, 7))
	{
		sys_log(0, "PLAYER_DELETE FAILED len(%d)", strlen(r.social_id));
		peer->EncodeHeader(HEADER_DG_PLAYER_DELETE_FAILED, dwHandle, 1);
		peer->EncodeBYTE(packet->account_index);
		return;
	}

	CPlayerTableCache * pkPlayerCache = GetPlayerCache(packet->player_id);
	if (pkPlayerCache)
	{
		TPlayerTable * pTab = pkPlayerCache->Get();

		if (pTab->level > m_iPlayerDeleteLevelLimit)
		{
			sys_log(0, "PLAYER_DELETE FAILED LEVEL %u > DELETE LIMIT %d", pTab->level, m_iPlayerDeleteLevelLimit);
			peer->EncodeHeader(HEADER_DG_PLAYER_DELETE_FAILED, dwHandle, 1);
			peer->EncodeBYTE(packet->account_index);
			return;
		}

		if (pTab->level < m_iPlayerDeleteLevelLimitLower)
		{
			sys_log(0, "PLAYER_DELETE FAILED LEVEL %u < DELETE LIMIT %d", pTab->level, m_iPlayerDeleteLevelLimitLower);
			peer->EncodeHeader(HEADER_DG_PLAYER_DELETE_FAILED, dwHandle, 1);
			peer->EncodeBYTE(packet->account_index);
			return;
		}
	}

#ifdef __OFFLINE_SHOP__
	if (IsUsingOfflineshopSystem(packet->player_id)) {
		sys_log(0, "PLAYER_DELETE FAILED %u IS USING OFFLINESHOP SYSTEM", packet->player_id);
		peer->EncodeHeader(HEADER_DG_PLAYER_DELETE_FAILED, dwHandle, 1);
		peer->EncodeBYTE(packet->account_index);
		return;
	}
#endif

	char szQuery[128];
	snprintf(szQuery, sizeof(szQuery), "SELECT p.id, p.level, p.name FROM player_index%s AS i, player%s AS p WHERE pid%u=%u AND pid%u=p.id",
		GetTablePostfix(), GetTablePostfix(), packet->account_index + 1, packet->player_id, packet->account_index + 1);

	ClientHandleInfo* pi = new ClientHandleInfo(dwHandle, packet->player_id);
	pi->account_index = packet->account_index;

	sys_log(0, "PLAYER_DELETE TRY: %s %d pid%d", packet->login, packet->player_id, packet->account_index + 1);
	CDBManager::instance().ReturnQuery(szQuery, QID_PLAYER_DELETE, peer->GetHandle(), pi);
}

//

//
void CClientManager::__RESULT_PLAYER_DELETE(CPeer* peer, SQLMsg* msg)
{
	CQueryInfo* qi = (CQueryInfo*)msg->pvUserData;
	ClientHandleInfo* pi = (ClientHandleInfo*)qi->pvData;

	if (msg->Get() && msg->Get()->uiNumRows)
	{
		MYSQL_ROW row = mysql_fetch_row(msg->Get()->pSQLResult);

		DWORD dwPID = 0;
		str_to_number(dwPID, row[0]);

		int deletedLevelLimit = 0;
		str_to_number(deletedLevelLimit, row[1]);

		char szName[64];
		strlcpy(szName, row[2], sizeof(szName));

		if (deletedLevelLimit > m_iPlayerDeleteLevelLimit)
		{
			sys_log(0, "PLAYER_DELETE FAILED LEVEL %u > DELETE LIMIT %d", deletedLevelLimit, m_iPlayerDeleteLevelLimit);
			peer->EncodeHeader(HEADER_DG_PLAYER_DELETE_FAILED, pi->dwHandle, 1);
			peer->EncodeBYTE(pi->account_index);
			return;
		}

		if (deletedLevelLimit < m_iPlayerDeleteLevelLimitLower)
		{
			sys_log(0, "PLAYER_DELETE FAILED LEVEL %u < DELETE LIMIT %d", deletedLevelLimit, m_iPlayerDeleteLevelLimitLower);
			peer->EncodeHeader(HEADER_DG_PLAYER_DELETE_FAILED, pi->dwHandle, 1);
			peer->EncodeBYTE(pi->account_index);
			return;
		}

		char queryStr[QUERY_MAX_LEN];

		snprintf(queryStr, sizeof(queryStr), "INSERT INTO player%s_deleted SELECT * FROM player%s WHERE id=%d",
			GetTablePostfix(), GetTablePostfix(), pi->player_id);
		std::unique_ptr<SQLMsg> pIns(CDBManager::instance().DirectQuery(queryStr));

		if (pIns->Get()->uiAffectedRows == 0 || pIns->Get()->uiAffectedRows == (uint32_t)-1)
		{
			sys_log(0, "PLAYER_DELETE FAILED %u CANNOT INSERT TO player%s_deleted", dwPID, GetTablePostfix());

			peer->EncodeHeader(HEADER_DG_PLAYER_DELETE_FAILED, pi->dwHandle, 1);
			peer->EncodeBYTE(pi->account_index);
			return;
		}

		sys_log(0, "PLAYER_DELETE SUCCESS %u", dwPID);

		char account_index_string[16];

		snprintf(account_index_string, sizeof(account_index_string), "player_id%d", m_iPlayerIDStart + pi->account_index);

		CPlayerTableCache* pkPlayerCache = GetPlayerCache(pi->player_id);

		if (pkPlayerCache)
		{
			m_map_playerCache.erase(pi->player_id);
			delete pkPlayerCache;
		}

#ifdef __SKILL_COLOR__
		CSKillColorCache* pkSkillColorCache = GetSkillColorCache(pi->player_id);
		if (pkSkillColorCache)
		{
			m_map_SkillColorCache.erase(pi->player_id);
			delete pkSkillColorCache;
		}
#endif

		TItemCacheSet* pSet = GetItemCacheSet(pi->player_id);

		if (pSet)
		{
			TItemCacheSet::iterator it = pSet->begin();

			while (it != pSet->end())
			{
				CItemCache* pkItemCache = *it++;
				DeleteItemCache(pkItemCache->Get()->id);
			}

			pSet->clear();
			delete pSet;

			m_map_pkItemCacheSetPtr.erase(pi->player_id);
		}

		TQuestCacheSet* pQuestSet = GetQuestCacheSet(pi->player_id);
		if (pQuestSet)
		{
			TQuestCacheSet::iterator it_set = pQuestSet->begin();
			while (it_set != pQuestSet->end())
			{
				CQuestCache* c = *it_set++;
				delete c;
			}

			pQuestSet->clear();
			delete pQuestSet;

			m_map_pkQuestCacheSetPtr.erase(pi->player_id);
		}

		TAffectCacheSet* pAffectSet = GetAffectCacheSet(pi->player_id);
		if (pAffectSet)
		{
			TAffectCacheSet::iterator it_set = pAffectSet->begin();
			while (it_set != pAffectSet->end())
			{
				CAffectCache* c = *it_set++;
				delete c;
			}

			pAffectSet->clear();
			delete pAffectSet;

			m_map_pkAffectCacheSetPtr.erase(pi->player_id);
		}

#ifdef __BATTLE_PASS_SYSTEM__
		TBattlePassCacheSet* pBattlePassCacheSet = GetBattlePassCacheSet(pi->player_id);
		if (pBattlePassCacheSet)
		{
			TBattlePassCacheSet::iterator it_set = pBattlePassCacheSet->begin();
			while (it_set != pBattlePassCacheSet->end())
			{
				CBattlePassCache* c = *it_set++;
				delete c;
			}

			pBattlePassCacheSet->clear();
			delete pBattlePassCacheSet;

			m_map_pkBattlePassCacheSetPtr.erase(pi->player_id);
		}
#endif // __BATTLE_PASS_SYSTEM__

		snprintf(queryStr, sizeof(queryStr), "UPDATE player_index%s SET pid%u=0 WHERE pid%u=%d",
			GetTablePostfix(),
			pi->account_index + 1,
			pi->account_index + 1,
			pi->player_id);

		std::unique_ptr<SQLMsg> pMsg(CDBManager::instance().DirectQuery(queryStr));

		if (pMsg->Get()->uiAffectedRows == 0 || pMsg->Get()->uiAffectedRows == (uint32_t)-1)
		{
			sys_log(0, "PLAYER_DELETE FAIL WHEN UPDATE account table");
			peer->EncodeHeader(HEADER_DG_PLAYER_DELETE_FAILED, pi->dwHandle, 1);
			peer->EncodeBYTE(pi->account_index);
			return;
		}

		snprintf(queryStr, sizeof(queryStr), "DELETE FROM player%s WHERE id=%d", GetTablePostfix(), pi->player_id);
		delete CDBManager::instance().DirectQuery(queryStr);

#ifdef __ADDITIONAL_INVENTORY__
		// __SWITCHBOT__
		snprintf(queryStr, sizeof(queryStr), "DELETE FROM item%s WHERE owner_id=%d AND (`window` in ('INVENTORY','EQUIPMENT','DRAGON_SOUL_INVENTORY','BELT_INVENTORY','UPGRADE_INVENTORY','BOOK_INVENTORY','STONE_INVENTORY','FLOWER_INVENTORY','ATTR_INVENTORY','CHEST_INVENTORY','SWITCHBOT'))", GetTablePostfix(), pi->player_id);
#else
		// __SWITCHBOT__
		snprintf(queryStr, sizeof(queryStr), "DELETE FROM item%s WHERE owner_id=%d AND (`window` in ('INVENTORY','EQUIPMENT','DRAGON_SOUL_INVENTORY','BELT_INVENTORY','SWITCHBOT'))", GetTablePostfix(), pi->player_id);
#endif
		delete CDBManager::instance().DirectQuery(queryStr, SQL_PLAYER_ITEM);

		snprintf(queryStr, sizeof(queryStr), "DELETE FROM quest%s WHERE dwPID=%d", GetTablePostfix(), pi->player_id);
		CDBManager::instance().AsyncQuery(queryStr, SQL_PLAYER_QUEST);

		snprintf(queryStr, sizeof(queryStr), "DELETE FROM affect%s WHERE dwPID=%d", GetTablePostfix(), pi->player_id);
		CDBManager::instance().AsyncQuery(queryStr, SQL_PLAYER_AFFECT);

		snprintf(queryStr, sizeof(queryStr), "DELETE FROM guild_member%s WHERE pid=%d", GetTablePostfix(), pi->player_id);
		CDBManager::instance().AsyncQuery(queryStr, SQL_PLAYER_GUILD);

		snprintf(queryStr, sizeof(queryStr), "DELETE FROM messenger_list%s WHERE account='%s' OR companion='%s'", GetTablePostfix(), szName, szName);
		CDBManager::instance().AsyncQuery(queryStr);

#ifdef __SKILL_COLOR__
		snprintf(queryStr, sizeof(queryStr), "DELETE FROM skill_color%s WHERE player_id=%d", GetTablePostfix(), pi->player_id);
		CDBManager::instance().AsyncQuery(queryStr, SQL_PLAYER_SKILLCOLOR);
#endif

		peer->EncodeHeader(HEADER_DG_PLAYER_DELETE_SUCCESS, pi->dwHandle, 1);
		peer->EncodeBYTE(pi->account_index);
	}
	else
	{
		sys_log(0, "PLAYER_DELETE FAIL NO ROW");
		peer->EncodeHeader(HEADER_DG_PLAYER_DELETE_FAILED, pi->dwHandle, 1);
		peer->EncodeBYTE(pi->account_index);
	}
}


void CClientManager::InsertLogoutPlayer(DWORD pid)
{
	TLogoutPlayerMap::iterator it = m_map_logout.find(pid);

	if (it != m_map_logout.end())
	{
		if (g_log)
			sys_log(0, "LOGOUT: Update player time pid(%d)", pid);

		it->second->time = time(0);
		return;
	}

	TLogoutPlayer* pLogout = new TLogoutPlayer;
	pLogout->pid = pid;
	pLogout->time = time(0);
	m_map_logout.insert(std::make_pair(pid, pLogout));

	if (g_log)
		sys_log(0, "LOGOUT: Insert player pid(%d)", pid);
}

void CClientManager::DeleteLogoutPlayer(DWORD pid)
{
	TLogoutPlayerMap::iterator it = m_map_logout.find(pid);

	if (it != m_map_logout.end())
	{
		delete it->second;
		m_map_logout.erase(it);
	}
}

extern int g_iLogoutSeconds;

void CClientManager::UpdateLogoutPlayer()
{
	time_t now = time(0);

	TLogoutPlayerMap::iterator it = m_map_logout.begin();

	while (it != m_map_logout.end())
	{
		TLogoutPlayer* pLogout = it->second;

		if (now - g_iLogoutSeconds > pLogout->time)
		{
			FlushItemCacheSet(pLogout->pid);
			FlushPlayerCacheSet(pLogout->pid);
			FlushQuestCacheSet(pLogout->pid);
			FlushAffectCacheSet(pLogout->pid);
#ifdef __BATTLE_PASS_SYSTEM__
			FlushBattlePassCacheSet(pLogout->pid);
#endif // __BATTLE_PASS_SYSTEM__
#ifdef __OFFLINE_SHOP__
			// Oyundan ciktiginda oyuncunun deposuna ihtiyaci kalmiyor.
			OfflineshopDelShopSafebox(pLogout->pid);
#endif

			delete pLogout;
			m_map_logout.erase(it++);
		}
		else
			++it;
	}
}

void CClientManager::FlushPlayerCacheSet(DWORD pid)
{
	TPlayerTableCacheMap::iterator it = m_map_playerCache.find(pid);

	if (it != m_map_playerCache.end())
	{
		CPlayerTableCache* c = it->second;
		m_map_playerCache.erase(it);

		c->Flush();
		delete c;
	}
}

#ifdef __BATTLE_PASS_SYSTEM__
void CClientManager::RESULT_BATTLE_PASS_LOAD(CPeer * peer, MYSQL_RES * pRes, DWORD dwHandle, DWORD dwRealPID)
{
	static std::vector<TPlayerBattlePassMission> s_missions;

	CreateBattlePassTableFromRes(pRes, &s_missions, dwRealPID);
	DWORD dwCount = s_missions.size();

	peer->EncodeHeader(HEADER_DG_BATTLE_PASS_LOAD, dwHandle, sizeof(DWORD) + sizeof(DWORD) + sizeof(TPlayerBattlePassMission) * dwCount);
	peer->Encode(&dwRealPID, sizeof(DWORD));
	peer->Encode(&dwCount, sizeof(DWORD));

	sys_log(0, "BATTLE_PASS_LOAD: count %u pid %u", dwCount, dwRealPID);

	TBattlePassCacheSetPtrMap::iterator it = m_map_pkBattlePassCacheSetPtr.find(dwRealPID);

	TBattlePassCacheSet* pSet = nullptr;

	if (it != m_map_pkBattlePassCacheSetPtr.end())
	{
		sys_err("battlepassload already in cache!");
		pSet = it->second;

		TBattlePassCacheSet::iterator it_set = pSet->begin();

		while (it_set != pSet->end())
		{
			CBattlePassCache* c = *it_set++;
			delete c;
		}

		pSet->clear();
	}
	else
	{
		pSet = CreateBattlePassCacheSet(dwRealPID);
	}

	if (dwCount)
	{
		peer->Encode(&s_missions[0], sizeof(TPlayerBattlePassMission) * dwCount);

		TPlayerBattlePassMission p;
		p.dwPlayerId = dwRealPID;
		for (DWORD i = 0; i < dwCount; ++i)
		{
			p.dwMissionId = s_missions.at(i).dwMissionId;
			p.dwBattlePassId = s_missions.at(i).dwBattlePassId;
			p.dwExtraInfo = s_missions.at(i).dwExtraInfo;
			p.bCompleted = s_missions.at(i).bCompleted;
			p.bIsUpdated = s_missions.at(i).bIsUpdated;

			CBattlePassCache* c = new CBattlePassCache;
			c->Put(&p);
			pSet->insert(c);
		}
	}
}

extern int g_iCurrentBattlePassId;
bool CClientManager::LoadBattlePassRanking()
{
	char query[256];
	snprintf(query, sizeof(query), "SELECT player_name, UNIX_TIMESTAMP(finish_time) "
				"FROM battle_pass_ranking WHERE battle_pass_id = %d OR battle_pass_id = %d ORDER BY finish_time ASC LIMIT 45;", g_iCurrentBattlePassId, g_iCurrentBattlePassId+10);

	std::unique_ptr<SQLMsg> pkMsg(CDBManager::instance().DirectQuery(query));
	SQLResult * pRes = pkMsg->Get();

	if (!pRes->uiNumRows)
		return true; // No ranking for battle pass with id %i%

	if(!m_vec_battlePassRanking.empty())
		m_vec_battlePassRanking.clear();

	BYTE posContor = 1;

	MYSQL_ROW row;
	while ((row = mysql_fetch_row(pRes->pSQLResult)))
	{
		TBattlePassRanking * ranking = new TBattlePassRanking;
		ranking->bPos = posContor;
		strlcpy(ranking->playerName, row[0], sizeof(ranking->playerName));
		str_to_number(ranking->dwFinishTime, row[1]);

		m_vec_battlePassRanking.push_back(ranking);
		posContor++;
	}

	return true;
}

void CClientManager::RequestLoadBattlePassRanking(CPeer * peer, DWORD dwHandle, const char* data)
{
	DWORD dwPlayerID = *(DWORD *)data;
	data += sizeof(DWORD);

	BYTE bIsGlobal = *(BYTE *)data;
	data += sizeof(BYTE);

	if(m_vec_battlePassRanking.empty())
	{
		DWORD dwCount = 0;
		TBattlePassRanking pbpTable = {0};

		sys_log(0, "RequestLoadBattlePassRanking: count %u PID %d", dwCount, dwPlayerID);

		peer->EncodeHeader(HEADER_DG_BATTLE_PASS_LOAD_RANKING, dwHandle, sizeof(DWORD) + sizeof(BYTE) + sizeof(DWORD) + sizeof(TBattlePassRanking) * dwCount);
		peer->Encode(&dwPlayerID, sizeof(DWORD));
		peer->Encode(&bIsGlobal, sizeof(BYTE));
		peer->Encode(&dwCount, sizeof(DWORD));
		peer->Encode(&pbpTable, sizeof(TBattlePassRanking) * dwCount);
		return;
	}
	else
	{
		static std::vector<TBattlePassRanking> sendVector;
		sendVector.resize(m_vec_battlePassRanking.size());

		MYSQL_ROW row;

		int rankingSize = m_vec_battlePassRanking.size();
		for (int i = 0; i < rankingSize; ++i)
		{
			TBattlePassRanking * pkRanking = m_vec_battlePassRanking[i];
			TBattlePassRanking & r = sendVector[i];

			r.bPos = pkRanking->bPos;
			strlcpy(r.playerName, pkRanking->playerName, sizeof(r.playerName));
			r.dwFinishTime = pkRanking->dwFinishTime;
		}

		sys_log(0, "RequestLoadBattlePassRanking: count %d PID %u", sendVector.size(), dwPlayerID);

		DWORD dwCount = sendVector.size();

		peer->EncodeHeader(HEADER_DG_BATTLE_PASS_LOAD_RANKING, dwHandle, sizeof(DWORD) + sizeof(BYTE) + sizeof(DWORD) + sizeof(TBattlePassRanking) * dwCount);
		peer->Encode(&dwPlayerID, sizeof(DWORD));
		peer->Encode(&bIsGlobal, sizeof(BYTE));
		peer->Encode(&dwCount, sizeof(DWORD));
		peer->Encode(&sendVector[0], sizeof(TBattlePassRanking) * dwCount);
	}
}

void CClientManager::QUERY_REGISTER_RANKING(CPeer * peer, DWORD dwHandle, TBattlePassRegisterRanking* pRanking)
{
	if (g_test_server)
		sys_log(0, "REGISTER_RANKING: %s id %d", pRanking->playerName, pRanking->bBattlePassId);

	char szQuery[QUERY_MAX_LEN];
	snprintf(szQuery, sizeof(szQuery),
		"REPLACE INTO battle_pass_ranking (player_name, battle_pass_id, finish_time) VALUES ('%s', %d, NOW())", pRanking->playerName, pRanking->bBattlePassId);

	CDBManager::instance().AsyncQuery(szQuery);
}
#endif