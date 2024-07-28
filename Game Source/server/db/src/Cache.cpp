#include "stdafx.h"
#include "Cache.h"

#include "QID.h"
#include "ClientManager.h"
#include "Main.h"

extern int g_iPlayerCacheFlushSeconds;
extern int g_iItemCacheFlushSeconds;
extern int g_iQuestCacheFlushSeconds;
extern int g_iAffectCacheFlushSeconds;
#ifdef __BATTLE_PASS_SYSTEM__
extern int g_iBattlePassCacheFlushSeconds;
#endif // __BATTLE_PASS_SYSTEM__

extern int g_test_server;

CItemCache::CItemCache()
{
	m_expireTime = MIN(1800, g_iItemCacheFlushSeconds);
}

CItemCache::~CItemCache()
{
}

// fixme
// by rtsummit
void CItemCache::Delete()
{
	if (m_data.vnum == 0)
		return;

	//char szQuery[QUERY_MAX_LEN];
	//szQuery[QUERY_MAX_LEN] = '\0';
	if (g_test_server)
		sys_log(0, "ItemCache::Delete : DELETE %u", m_data.id);

	m_data.vnum = 0;
	m_bNeedQuery = true;
	m_lastUpdateTime = time(0);
	OnFlush();

	//m_bNeedQuery = false;
}

void CItemCache::OnFlush()
{
	if (m_data.vnum == 0)
	{
		char szQuery[QUERY_MAX_LEN];
		snprintf(szQuery, sizeof(szQuery), "DELETE FROM item%s WHERE id=%u", GetTablePostfix(), m_data.id);
		CDBManager::instance().AsyncQuery(szQuery, SQL_PLAYER_ITEM);

		if (g_test_server)
			sys_log(0, "ItemCache::Flush : DELETE %u %s", m_data.id, szQuery);
	}
	else
	{
		long alSockets[ITEM_SOCKET_MAX_NUM];
		TPlayerItemAttribute aAttr[ITEM_ATTRIBUTE_MAX_NUM];
		bool isSocket = false, isAttr = false;

		memset(&alSockets, 0, sizeof(long) * ITEM_SOCKET_MAX_NUM);
		memset(&aAttr, 0, sizeof(TPlayerItemAttribute) * ITEM_ATTRIBUTE_MAX_NUM);

		TPlayerItem* p = &m_data;

		if (memcmp(alSockets, p->alSockets, sizeof(long) * ITEM_SOCKET_MAX_NUM))
			isSocket = true;

		if (memcmp(aAttr, p->aAttr, sizeof(TPlayerItemAttribute) * ITEM_ATTRIBUTE_MAX_NUM))
			isAttr = true;

		char szColumns[QUERY_MAX_LEN];
		char szValues[QUERY_MAX_LEN];
		char szUpdate[QUERY_MAX_LEN];

		int iLen = snprintf(szColumns, sizeof(szColumns), "id, owner_id, `window`, pos, count, vnum");

		int iValueLen = snprintf(szValues, sizeof(szValues), "%u, %u, %d, %d, %u, %u",
			p->id, p->owner, p->window, p->pos, p->count, p->vnum);

		int iUpdateLen = snprintf(szUpdate, sizeof(szUpdate), "owner_id=%u, `window`=%d, pos=%d, count=%u, vnum=%u",
			p->owner, p->window, p->pos, p->count, p->vnum);

#ifdef __BEGINNER_ITEM__
		iLen += snprintf(szColumns + iLen, sizeof(szColumns) - iLen, ", is_basic");
		iValueLen += snprintf(szValues + iValueLen, sizeof(szValues) - iValueLen,
			", %d", p->is_basic);

		iUpdateLen += snprintf(szUpdate + iUpdateLen, sizeof(szUpdate) - iUpdateLen,
			", is_basic=%d", p->is_basic);
#endif
#ifdef __ITEM_EVOLUTION__
		iLen += snprintf(szColumns + iLen, sizeof(szColumns) - iLen, ", evolution");
		iValueLen += snprintf(szValues + iValueLen, sizeof(szValues) - iValueLen,
			", %d", p->evolution);

		iUpdateLen += snprintf(szUpdate + iUpdateLen, sizeof(szUpdate) - iUpdateLen,
			", evolution=%d", p->evolution);
#endif
#ifdef __ITEM_CHANGELOOK__
		iLen += snprintf(szColumns + iLen, sizeof(szColumns) - iLen, ", transmutation");
		iValueLen += snprintf(szValues + iValueLen, sizeof(szValues) - iValueLen,
			", %u", p->transmutation);
		iUpdateLen += snprintf(szUpdate + iUpdateLen, sizeof(szUpdate) - iUpdateLen,
			", transmutation=%u", p->transmutation);
#endif
#ifdef __ITEM_EXTRA_SOCKET__
		if (isSocket)
		{
			iLen += snprintf(szColumns + iLen, sizeof(szColumns) - iLen, ", socket0, socket1, socket2, socket3");
			iValueLen += snprintf(szValues + iValueLen, sizeof(szValues) - iValueLen,
				", %lu, %lu, %lu, %lu", p->alSockets[0], p->alSockets[1], p->alSockets[2], p->alSockets[3]);
			iUpdateLen += snprintf(szUpdate + iUpdateLen, sizeof(szUpdate) - iUpdateLen,
				", socket0=%lu, socket1=%lu, socket2=%lu, socket3=%lu", p->alSockets[0], p->alSockets[1], p->alSockets[2], p->alSockets[3]);
		}
#else
		if (isSocket)
		{
			iLen += snprintf(szColumns + iLen, sizeof(szColumns) - iLen, ", socket0, socket1, socket2");
			iValueLen += snprintf(szValues + iValueLen, sizeof(szValues) - iValueLen,
				", %lu, %lu, %lu", p->alSockets[0], p->alSockets[1], p->alSockets[2]);
			iUpdateLen += snprintf(szUpdate + iUpdateLen, sizeof(szUpdate) - iUpdateLen,
				", socket0=%lu, socket1=%lu, socket2=%lu", p->alSockets[0], p->alSockets[1], p->alSockets[2]);
		}
#endif
		if (isAttr)
		{
			iLen += snprintf(szColumns + iLen, sizeof(szColumns) - iLen,
				", attrtype0, attrvalue0, attrtype1, attrvalue1, attrtype2, attrvalue2, attrtype3, attrvalue3"
				", attrtype4, attrvalue4, attrtype5, attrvalue5, attrtype6, attrvalue6"
			);

			iValueLen += snprintf(szValues + iValueLen, sizeof(szValues) - iValueLen,
				", %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d",
				p->aAttr[0].bType, p->aAttr[0].sValue,
				p->aAttr[1].bType, p->aAttr[1].sValue,
				p->aAttr[2].bType, p->aAttr[2].sValue,
				p->aAttr[3].bType, p->aAttr[3].sValue,
				p->aAttr[4].bType, p->aAttr[4].sValue,
				p->aAttr[5].bType, p->aAttr[5].sValue,
				p->aAttr[6].bType, p->aAttr[6].sValue
			);

			iUpdateLen += snprintf(szUpdate + iUpdateLen, sizeof(szUpdate) - iUpdateLen,
				", attrtype0=%d, attrvalue0=%d"
				", attrtype1=%d, attrvalue1=%d"
				", attrtype2=%d, attrvalue2=%d"
				", attrtype3=%d, attrvalue3=%d"
				", attrtype4=%d, attrvalue4=%d"
				", attrtype5=%d, attrvalue5=%d"
				", attrtype6=%d, attrvalue6=%d"
				,
				p->aAttr[0].bType, p->aAttr[0].sValue,
				p->aAttr[1].bType, p->aAttr[1].sValue,
				p->aAttr[2].bType, p->aAttr[2].sValue,
				p->aAttr[3].bType, p->aAttr[3].sValue,
				p->aAttr[4].bType, p->aAttr[4].sValue,
				p->aAttr[5].bType, p->aAttr[5].sValue,
				p->aAttr[6].bType, p->aAttr[6].sValue
			);
		}

		char szItemQuery[QUERY_MAX_LEN + QUERY_MAX_LEN + QUERY_MAX_LEN];
		snprintf(szItemQuery, sizeof(szItemQuery), "REPLACE INTO item%s (%s) VALUES(%s)", GetTablePostfix(), szColumns, szValues);

		if (g_test_server)
			sys_log(0, "ItemCache::Flush :REPLACE  (%s)", szItemQuery);

		CDBManager::instance().AsyncQuery(szItemQuery, SQL_PLAYER_ITEM);
	}

	m_bNeedQuery = false;
}

CQuestCache::CQuestCache()
{
	m_expireTime = MIN(1800, g_iQuestCacheFlushSeconds);
}

CQuestCache::~CQuestCache()
{
}

// fixme
// by rtsummit
void CQuestCache::Delete()
{
	// hepsi birden silinecek burasi kullanilmayacak
}

void CQuestCache::OnFlush()
{
	if (m_data.lValue == 0)
	{
		// char szQuery[1024];
		// snprintf(szQuery, sizeof(szQuery), "DELETE FROM quest%s WHERE dwPID=%u AND szName='%s' AND szState='%s'", GetTablePostfix(), m_data.dwPID, m_data.szName, m_data.szState);
		// delete CDBManager::instance().DirectQuery(szQuery);

		sys_log(0, "QuestCache::Flush : DELETE %u %s %s", m_data.dwPID, m_data.szName, m_data.szState);
	}
	else
	{
		TQuestTable* p = &m_data;

		char szColumns[1024];
		char szValues[1024];

		snprintf(szColumns, sizeof(szColumns), "dwPID, szName, szState, lValue");

		snprintf(szValues, sizeof(szValues), "%u, '%s', '%s', %ld",
			p->dwPID, p->szName, p->szState, p->lValue);

		char szQuestQuery[1024 + 1024 + 1024];
		snprintf(szQuestQuery, sizeof(szQuestQuery), "INSERT INTO quest%s (%s) VALUES(%s)", GetTablePostfix(), szColumns, szValues);

		if (g_test_server)
			sys_log(0, "QuestCache::Flush :REPLACE  (%s)", szQuestQuery);

		CDBManager::instance().AsyncQuery(szQuestQuery, SQL_PLAYER_QUEST);

		//g_quest_info.Add(p->vnum);
		// ++g_quest_count;
	}

	m_bNeedQuery = false;
}

CAffectCache::CAffectCache()
{
	m_expireTime = MIN(1800, g_iAffectCacheFlushSeconds);
}

CAffectCache::~CAffectCache()
{
}

// fixme
// by rtsummit
void CAffectCache::Delete()
{
	// hepsi birden silinecek burasi kullanilmayacak
}

void CAffectCache::OnFlush()
{
	char szColumns[1024];
	char szValues[1024];

	snprintf(szColumns, sizeof(szColumns), "dwPID, bType, bApplyOn, lApplyValue, dwFlag, lDuration, lSPCost");

	snprintf(szValues, sizeof(szValues), "%u, %u, %d, %ld, %u, %ld, %ld",
		m_data.dwPID, m_data.elem.dwType, m_data.elem.bApplyOn, m_data.elem.lApplyValue, m_data.elem.dwFlag, m_data.elem.lDuration, m_data.elem.lSPCost);

	char szAffectQuery[1024 + 1024 + 1024];
	snprintf(szAffectQuery, sizeof(szAffectQuery), "INSERT INTO affect%s (%s) VALUES(%s)", GetTablePostfix(), szColumns, szValues);

	if (g_test_server)
		sys_log(0, "AffectCache::Flush :REPLACE  (%s)", szAffectQuery);

	CDBManager::instance().AsyncQuery(szAffectQuery, SQL_PLAYER_AFFECT);
}

//
// CPlayerTableCache
//
CPlayerTableCache::CPlayerTableCache()
{
	m_expireTime = MIN(1800, g_iPlayerCacheFlushSeconds);
}

CPlayerTableCache::~CPlayerTableCache()
{
}

void CPlayerTableCache::OnFlush()
{
	if (g_test_server)
		sys_log(0, "PlayerTableCache::Flush : %s", m_data.name);

	char szQuery[QUERY_MAX_LEN];
	CreatePlayerSaveQuery(szQuery, sizeof(szQuery), &m_data);
	CDBManager::instance().AsyncQuery(szQuery);
}

#ifdef __SKILL_COLOR__
extern int g_iSkillColorCacheFlushSeconds;

CSKillColorCache::CSKillColorCache()
{
	m_expireTime = MIN(1800, g_iSkillColorCacheFlushSeconds);
}

CSKillColorCache::~CSKillColorCache()
{
}

void CSKillColorCache::OnFlush()
{
	char query[QUERY_MAX_LEN];
	snprintf(query, sizeof(query),
		"REPLACE INTO skill_color%s (`player_id`"
		", `s1_col1`, `s1_col2`, `s1_col3`, `s1_col4`, `s1_col5`"
		", `s2_col1`, `s2_col2`, `s2_col3`, `s2_col4`, `s2_col5`"
		", `s3_col1`, `s3_col2`, `s3_col3`, `s3_col4`, `s3_col5`"
		", `s4_col1`, `s4_col2`, `s4_col3`, `s4_col4`, `s4_col5`"
		", `s5_col1`, `s5_col2`, `s5_col3`, `s5_col4`, `s5_col5`"
		", `s6_col1`, `s6_col2`, `s6_col3`, `s6_col4`, `s6_col5`" // end of skills
		", `s7_col1`, `s7_col2`, `s7_col3`, `s7_col4`, `s7_col5`" // begin buffs
		", `s8_col1`, `s8_col2`, `s8_col3`, `s8_col4`, `s8_col5`"
		", `s9_col1`, `s9_col2`, `s9_col3`, `s9_col4`, `s9_col5`"
		", `s10_col1`, `s10_col2`, `s10_col3`, `s10_col4`, `s10_col5`"
		", `s11_col1`, `s11_col2`, `s11_col3`, `s11_col4`, `s11_col5`"
#ifdef __WOLFMAN_CHARACTER__
		", `s12_col1`, `s12_col2`, `s12_col3`, `s12_col4`, `s12_col5`"
#endif
		") "
		"VALUES (%d"
		", %d, %d, %d, %d, %d"
		", %d, %d, %d, %d, %d"
		", %d, %d, %d, %d, %d"
		", %d, %d, %d, %d, %d"
		", %d, %d, %d, %d, %d"
		", %d, %d, %d, %d, %d" // end of skills
		", %d, %d, %d, %d, %d" // begin buffs
		", %d, %d, %d, %d, %d"
		", %d, %d, %d, %d, %d"
		", %d, %d, %d, %d, %d"
		", %d, %d, %d, %d, %d"
#ifdef __WOLFMAN_CHARACTER__
		", %d, %d, %d, %d, %d"
#endif
		")", GetTablePostfix(), m_data.player_id,
		m_data.dwSkillColor[0][0], m_data.dwSkillColor[0][1], m_data.dwSkillColor[0][2], m_data.dwSkillColor[0][3], m_data.dwSkillColor[0][4],
		m_data.dwSkillColor[1][0], m_data.dwSkillColor[1][1], m_data.dwSkillColor[1][2], m_data.dwSkillColor[1][3], m_data.dwSkillColor[1][4],
		m_data.dwSkillColor[2][0], m_data.dwSkillColor[2][1], m_data.dwSkillColor[2][2], m_data.dwSkillColor[2][3], m_data.dwSkillColor[2][4],
		m_data.dwSkillColor[3][0], m_data.dwSkillColor[3][1], m_data.dwSkillColor[3][2], m_data.dwSkillColor[3][3], m_data.dwSkillColor[3][4],
		m_data.dwSkillColor[4][0], m_data.dwSkillColor[4][1], m_data.dwSkillColor[4][2], m_data.dwSkillColor[4][3], m_data.dwSkillColor[4][4],
		m_data.dwSkillColor[5][0], m_data.dwSkillColor[5][1], m_data.dwSkillColor[5][2], m_data.dwSkillColor[5][3], m_data.dwSkillColor[5][4], // end of skills
		m_data.dwSkillColor[6][0], m_data.dwSkillColor[6][1], m_data.dwSkillColor[6][2], m_data.dwSkillColor[6][3], m_data.dwSkillColor[6][4], // begin buffs
		m_data.dwSkillColor[7][0], m_data.dwSkillColor[7][1], m_data.dwSkillColor[7][2], m_data.dwSkillColor[7][3], m_data.dwSkillColor[7][4],
		m_data.dwSkillColor[8][0], m_data.dwSkillColor[8][1], m_data.dwSkillColor[8][2], m_data.dwSkillColor[8][3], m_data.dwSkillColor[8][4],
		m_data.dwSkillColor[9][0], m_data.dwSkillColor[9][1], m_data.dwSkillColor[9][2], m_data.dwSkillColor[9][3], m_data.dwSkillColor[9][4],
		m_data.dwSkillColor[10][0], m_data.dwSkillColor[10][1], m_data.dwSkillColor[10][2], m_data.dwSkillColor[10][3], m_data.dwSkillColor[10][4]
#ifdef __WOLFMAN_CHARACTER__
		, m_data.dwSkillColor[11][0], m_data.dwSkillColor[11][1], m_data.dwSkillColor[11][2], m_data.dwSkillColor[11][3], m_data.dwSkillColor[11][4]
#endif
	);

	CDBManager::instance().AsyncQuery(query, SQL_PLAYER_SKILLCOLOR);

	if (g_test_server)
		sys_log(0, "SkillColorCache::Flush :REPLACE %u (%s)", m_data.player_id, query);

	m_bNeedQuery = false;
}
#endif

#ifdef __BATTLE_PASS_SYSTEM__
CBattlePassCache::CBattlePassCache()
{
	m_expireTime = MIN(1800, g_iBattlePassCacheFlushSeconds);
}

CBattlePassCache::~CBattlePassCache()
{
}

void CBattlePassCache::Delete()
{
}

void CBattlePassCache::OnFlush()
{
	char szQuery[QUERY_MAX_LEN];
	snprintf(szQuery, sizeof(szQuery),
		"INSERT INTO battle_pass%s (player_id, mission_id, battle_pass_id, extra_info, completed) VALUES (%u, %d, %d, %d, %d)",
		GetTablePostfix(),
		m_data.dwPlayerId,
		m_data.dwMissionId,
		m_data.dwBattlePassId,
		m_data.dwExtraInfo,
		m_data.bCompleted ? 1 : 0);
		
	CDBManager::instance().AsyncQuery(szQuery, SQL_PLAYER_BATTLE_PASS);
}
#endif // __BATTLE_PASS_SYSTEM__
