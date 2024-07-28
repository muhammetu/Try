#include "stdafx.h"

#include "../../common/VnumHelper.h"
#include "../../libgame/include/grid.h"

#include "ClientManager.h"

#include "Main.h"
#include "Config.h"
#include "DBManager.h"
#include "QID.h"
#include "GuildManager.h"
#include "PrivManager.h"
#include "ItemIDRangeManager.h"
#include "Cache.h"
#include <sstream>

extern int g_iPlayerCacheFlushSeconds;
extern int g_iItemCacheFlushSeconds;
extern int g_iQuestCacheFlushSeconds;
extern int g_iAffectCacheFlushSeconds;

#ifdef __BATTLE_PASS_SYSTEM__
extern int g_iBattlePassCacheFlushSeconds;
#endif // __BATTLE_PASS_SYSTEM__

extern int g_test_server;
extern int g_log;
extern std::string g_stLocale;
extern std::string g_stLocaleNameColumn;
bool CreateItemTableFromRes(MYSQL_RES* res, std::vector<TPlayerItem>* pVec, DWORD dwPID);

CClientManager::CClientManager() :
	m_pkAuthPeer(NULL),
	m_iPlayerIDStart(0),
	m_iPlayerDeleteLevelLimit(0),
	m_iPlayerDeleteLevelLimitLower(0),
	m_iShopTableSize(0),
	m_pShopTable(NULL),
	m_iRefineTableSize(0),
	m_pRefineTable(NULL),
	m_bShutdowned(FALSE),
	m_iCacheFlushCount(0),
	m_iCacheFlushCountLimit(200)
{
}

CClientManager::~CClientManager()
{
	Destroy();
}

void CClientManager::SetPlayerIDStart(int iIDStart)
{
	m_iPlayerIDStart = iIDStart;
}

void CClientManager::Destroy()
{
	m_mChannelStatus.clear();
	for (itertype(m_peerList) i = m_peerList.begin(); i != m_peerList.end(); ++i)
		(*i)->Destroy();

	m_peerList.clear();

	if (m_fdAccept > 0)
	{
		socket_close(m_fdAccept);
		m_fdAccept = -1;
	}
}

#ifdef __DEFAULT_PRIV__
static bool bCleanOldPriv = true;
static bool __InitializeDefaultPriv()
{
	if (bCleanOldPriv)
	{
		std::unique_ptr<SQLMsg> pCleanStuff(CDBManager::instance().DirectQuery("DELETE FROM priv_settings WHERE value <= 0 OR duration <= NOW();", SQL_COMMON));
		printf("DEFAULT_PRIV_EMPIRE: removed %u expired priv settings.\n", pCleanStuff->Get()->uiAffectedRows);
	}
	std::unique_ptr<SQLMsg> pMsg(CDBManager::instance().DirectQuery("SELECT priv_type, id, type, value, UNIX_TIMESTAMP(duration) FROM priv_settings", SQL_COMMON));
	if (pMsg->Get()->uiNumRows == 0)
		return false;
	MYSQL_ROW row = NULL;
	while ((row = mysql_fetch_row(pMsg->Get()->pSQLResult)))
	{
		if (!strcmp(row[0], "EMPIRE"))
		{
			// init
			BYTE empire = 0;
			BYTE type = 1;
			int value = 0;
			time_t duration_sec = 0;
			// set
			str_to_number(empire, row[1]);
			str_to_number(type, row[2]);
			str_to_number(value, row[3]);
			str_to_number(duration_sec, row[4]);
			// recalibrate time
			time_t now_time_sec = CClientManager::instance().GetCurrentTime();
			if (now_time_sec > duration_sec)
				duration_sec = 0;
			else
				duration_sec -= now_time_sec;
			// send priv
			printf("DEFAULT_PRIV_EMPIRE: set empire(%u), type(%u), value(%d), duration(%u)\n", empire, type, value, duration_sec);
			CPrivManager::instance().AddEmpirePriv(empire, type, value, duration_sec);
		}
	}
	return true;
}

static bool __UpdateDefaultPriv(const char* priv_type, DWORD id, BYTE type, int value, time_t duration_sec)
{
	char szQuery[1024];
	snprintf(szQuery, 1024,
		"REPLACE INTO priv_settings SET priv_type='%s', id=%u, type=%u, value=%d, duration=DATE_ADD(NOW(), INTERVAL %u SECOND);",
		priv_type, id, type, value, duration_sec
	);
	std::unique_ptr<SQLMsg> pMsg(CDBManager::instance().DirectQuery(szQuery, SQL_COMMON));
	return pMsg->Get()->uiAffectedRows;
}
#endif

bool CClientManager::Initialize()
{
	int tmpValue;

	//BOOT_LOCALIZATION
	if (!InitializeLocalization())
	{
		fprintf(stderr, "Failed Localization Infomation so exit\n");
		return false;
	}
#ifdef __DEFAULT_PRIV__
	if (!__InitializeDefaultPriv())
	{
		fprintf(stderr, "Failed Default Priv Setting so exit\n");
		// return false;
	}
#endif
	//END_BOOT_LOCALIZATION

	if (!InitializeTables())
	{
		sys_err("Table Initialize FAILED");
		return false;
	}

	if (!CConfig::instance().GetValue("BIND_PORT", &tmpValue))
		tmpValue = 5300;

	char szBindIP[128];

	if (!CConfig::instance().GetValue("BIND_IP", szBindIP, 128))
		strlcpy(szBindIP, "0", sizeof(szBindIP));

	m_fdAccept = socket_tcp_bind(szBindIP, tmpValue);

	if (m_fdAccept < 0)
	{
		perror("socket");
		return false;
	}

	sys_log(0, "ACCEPT_HANDLE: %u", m_fdAccept);
	fdwatch_add_fd(m_fdWatcher, m_fdAccept, NULL, FDW_READ, false);

	if (!CConfig::instance().GetValue("BACKUP_LIMIT_SEC", &tmpValue))
		tmpValue = 600;

	m_looping = true;

	if (!CConfig::instance().GetValue("PLAYER_DELETE_LEVEL_LIMIT", &m_iPlayerDeleteLevelLimit))
	{
		sys_err("conf.txt: Cannot find PLAYER_DELETE_LEVEL_LIMIT, use default level %d", PLAYER_MAX_LEVEL_CONST + 1);
		m_iPlayerDeleteLevelLimit = PLAYER_MAX_LEVEL_CONST + 1;
	}

	if (!CConfig::instance().GetValue("PLAYER_DELETE_LEVEL_LIMIT_LOWER", &m_iPlayerDeleteLevelLimitLower))
	{
		m_iPlayerDeleteLevelLimitLower = 60;
	}

	sys_log(0, "PLAYER_DELETE_LEVEL_LIMIT set to %d", m_iPlayerDeleteLevelLimit);
	sys_log(0, "PLAYER_DELETE_LEVEL_LIMIT_LOWER set to %d", m_iPlayerDeleteLevelLimitLower);

	LoadEventFlag();

	return true;
}

void CClientManager::MainLoop()
{
	SQLMsg* tmp;

	sys_log(0, "ClientManager pointer is %p", this);

	while (!m_bShutdowned)
	{
		while ((tmp = CDBManager::instance().PopResult()))
		{
			AnalyzeQueryResult(tmp);
			delete tmp;
		}

#ifdef __NOT_ENOUGH_DB_QUERY_SPEED_ERROR__
		/**
		 * Eger bir loop icerisinde atilmasi gereken querlerin sayisi 0 degilse
		 * (loop donmesine ragmen subthreat bize yetisememisse)
		 * 60 saniyede bir syserr ver
		 */
		static time_t lastErrorTime = 0;

		for (size_t i = SQL_PLAYER; i < SQL_MAX_NUM; i++)
		{
			auto crq = CDBManager::instance().CountReturnQuery(i);
			auto caq = CDBManager::instance().CountAsyncQuery(i);
			if ((crq + caq > 5000) && lastErrorTime + 60 < time(0))
			{
				sys_err("!!!!!!! __NOT_ENOUGH_DB_QUERY_SPEED_ERROR__ :: slot %d return: %u async: %u total %d !!!!!!!", i, crq, caq, crq + caq);
			}
		}

		lastErrorTime = time(0);
#endif // __NOT_ENOUGH_DB_QUERY_SPEED_ERROR__

		if (!Process())
			break;

		log_rotate();
	}

	//

	//
	sys_log(0, "MainLoop exited, Starting cache flushing");

	signal_timer_disable();

	itertype(m_map_playerCache) it = m_map_playerCache.begin();


	while (it != m_map_playerCache.end())
	{
		CPlayerTableCache * c = (it++)->second;

		c->Flush();
		delete c;
	}
	m_map_playerCache.clear();


	itertype(m_map_itemCache) it2 = m_map_itemCache.begin();

	while (it2 != m_map_itemCache.end())
	{
		CItemCache * c = (it2++)->second;

		c->Flush();
		delete c;
	}
	m_map_itemCache.clear();

#ifdef __SKILL_COLOR__
	itertype(m_map_SkillColorCache) it3 = m_map_SkillColorCache.begin();
	while (it3 != m_map_SkillColorCache.end())
	{
		CSKillColorCache* c = it3->second;
		c->Flush();
		m_map_SkillColorCache.erase(it3++);
		delete c;
	}
	m_map_SkillColorCache.clear();
#endif

	itertype(m_map_pkQuestCacheSetPtr) it6 = m_map_pkQuestCacheSetPtr.begin();
	while (it6 != m_map_pkQuestCacheSetPtr.end())
	{
		FlushQuestCacheSet((it6++)->first);
	}
	m_map_pkQuestCacheSetPtr.clear();

	itertype(m_map_pkAffectCacheSetPtr) it7 = m_map_pkAffectCacheSetPtr.begin();
	while (it7 != m_map_pkAffectCacheSetPtr.end())
	{
		FlushAffectCacheSet((it7++)->first);
	}
	m_map_pkAffectCacheSetPtr.clear();
#ifdef __BATTLE_PASS_SYSTEM__
	itertype(m_map_pkBattlePassCacheSetPtr) it8 = m_map_pkBattlePassCacheSetPtr.begin();
	while (it8 != m_map_pkBattlePassCacheSetPtr.end())
	{
		FlushBattlePassCacheSet((it8++)->first);
	}
	m_map_pkBattlePassCacheSetPtr.clear();
#endif // __BATTLE_PASS_SYSTEM__
}

void CClientManager::Quit()
{
	m_bShutdowned = TRUE;
}

void CClientManager::QUERY_BOOT(CPeer* peer, TPacketGDBoot* p)
{
	const BYTE bPacketVersion = 6;

	std::vector<tAdminInfo> vAdmin;

	__GetAdminInfo(p->szIP, vAdmin);

	sys_log(0, "QUERY_BOOT : AdminInfo (Request ServerIp %s) ", p->szIP);

	DWORD dwPacketSize =
		sizeof(DWORD) +
		sizeof(BYTE) +
		sizeof(WORD) + sizeof(WORD) + sizeof(TMobTable) * m_vec_mobTable.size() +
		sizeof(WORD) + sizeof(WORD) + sizeof(TItemTable) * m_vec_itemTable.size() +
		sizeof(WORD) + sizeof(WORD) + sizeof(TShopTable) * m_iShopTableSize +
		sizeof(WORD) + sizeof(WORD) + sizeof(TSkillTable) * m_vec_skillTable.size() +
		sizeof(WORD) + sizeof(WORD) + sizeof(TRefineTable) * m_iRefineTableSize +
		sizeof(WORD) + sizeof(WORD) + sizeof(TItemAttrTable) * m_vec_itemAttrTable.size() +
		sizeof(WORD) + sizeof(WORD) + sizeof(TItemAttrTable) * m_vec_itemRareTable.size() +
		sizeof(time_t) +
		sizeof(WORD) + sizeof(WORD) + sizeof(TItemIDRangeTable) * 2 +
		//ADMIN_MANAGER
		sizeof(WORD) + sizeof(WORD) + sizeof(tAdminInfo) * vAdmin.size() +
		//END_ADMIN_MANAGER
		sizeof(WORD);

	peer->EncodeHeader(HEADER_DG_BOOT, 0, dwPacketSize);
	peer->Encode(&dwPacketSize, sizeof(DWORD));
	peer->Encode(&bPacketVersion, sizeof(BYTE));

	sys_log(0, "BOOT: PACKET: %d", dwPacketSize);
	sys_log(0, "BOOT: VERSION: %d", bPacketVersion);

	sys_log(0, "sizeof(TMobTable) = %d", sizeof(TMobTable));
	sys_log(0, "sizeof(TItemTable) = %d", sizeof(TItemTable));
	sys_log(0, "sizeof(TShopTable) = %d", sizeof(TShopTable));
	sys_log(0, "sizeof(TSkillTable) = %d", sizeof(TSkillTable));
	sys_log(0, "sizeof(TRefineTable) = %d", sizeof(TRefineTable));
	sys_log(0, "sizeof(TItemAttrTable) = %d", sizeof(TItemAttrTable));
	sys_log(0, "sizeof(TItemRareTable) = %d", sizeof(TItemAttrTable));
	//ADMIN_MANAGER
	sys_log(0, "sizeof(tAdminInfo) = %d * %d ", sizeof(tAdminInfo) * vAdmin.size());
	//END_ADMIN_MANAGER

	peer->EncodeWORD(sizeof(TMobTable));
	peer->EncodeWORD(m_vec_mobTable.size());
	peer->Encode(&m_vec_mobTable[0], sizeof(TMobTable) * m_vec_mobTable.size());

	peer->EncodeWORD(sizeof(TItemTable));
	peer->EncodeWORD(m_vec_itemTable.size());
	peer->Encode(&m_vec_itemTable[0], sizeof(TItemTable) * m_vec_itemTable.size());

	peer->EncodeWORD(sizeof(TShopTable));
	peer->EncodeWORD(m_iShopTableSize);
	peer->Encode(m_pShopTable, sizeof(TShopTable) * m_iShopTableSize);

	peer->EncodeWORD(sizeof(TSkillTable));
	peer->EncodeWORD(m_vec_skillTable.size());
	peer->Encode(&m_vec_skillTable[0], sizeof(TSkillTable) * m_vec_skillTable.size());

	peer->EncodeWORD(sizeof(TRefineTable));
	peer->EncodeWORD(m_iRefineTableSize);
	peer->Encode(m_pRefineTable, sizeof(TRefineTable) * m_iRefineTableSize);

	peer->EncodeWORD(sizeof(TItemAttrTable));
	peer->EncodeWORD(m_vec_itemAttrTable.size());
	peer->Encode(&m_vec_itemAttrTable[0], sizeof(TItemAttrTable) * m_vec_itemAttrTable.size());

	peer->EncodeWORD(sizeof(TItemAttrTable));
	peer->EncodeWORD(m_vec_itemRareTable.size());
	peer->Encode(&m_vec_itemRareTable[0], sizeof(TItemAttrTable) * m_vec_itemRareTable.size());

	time_t now = time(0);
	peer->Encode(&now, sizeof(time_t));

	TItemIDRangeTable itemRange = CItemIDRangeManager::instance().GetRange();
	TItemIDRangeTable itemRangeSpare = CItemIDRangeManager::instance().GetRange();

	peer->EncodeWORD(sizeof(TItemIDRangeTable));
	peer->EncodeWORD(1);
	peer->Encode(&itemRange, sizeof(TItemIDRangeTable));
	peer->Encode(&itemRangeSpare, sizeof(TItemIDRangeTable));

	peer->SetItemIDRange(itemRange);
	peer->SetSpareItemIDRange(itemRangeSpare);

	//ADMIN_MANAGER
	peer->EncodeWORD(sizeof(tAdminInfo));
	peer->EncodeWORD(vAdmin.size());

	for (size_t n = 0; n < vAdmin.size(); ++n)
	{
		peer->Encode(&vAdmin[n], sizeof(tAdminInfo));
		sys_log(0, "Admin name %s ConntactIP %s", vAdmin[n].m_szName, vAdmin[n].m_szContactIP);
	}
	//END_ADMIN_MANAGER

	peer->EncodeWORD(0xffff);

#ifdef __OFFLINE_SHOP__
	SendOfflineshopTable(peer);
#endif
}

void CClientManager::SendPartyOnSetup(CPeer* pkPeer)
{
	TPartyMap& pm = m_map_pkChannelParty[pkPeer->GetChannel()];

	for (itertype(pm) it_party = pm.begin(); it_party != pm.end(); ++it_party)
	{
		sys_log(0, "PARTY SendPartyOnSetup Party [%u]", it_party->first);
		pkPeer->EncodeHeader(HEADER_DG_PARTY_CREATE, 0, sizeof(TPacketPartyCreate));
		pkPeer->Encode(&it_party->first, sizeof(DWORD));

		for (itertype(it_party->second) it_member = it_party->second.begin(); it_member != it_party->second.end(); ++it_member)
		{
			sys_log(0, "PARTY SendPartyOnSetup Party [%u] Member [%u]", it_party->first, it_member->first);
			pkPeer->EncodeHeader(HEADER_DG_PARTY_ADD, 0, sizeof(TPacketPartyAdd));
			pkPeer->Encode(&it_party->first, sizeof(DWORD));
			pkPeer->Encode(&it_member->first, sizeof(DWORD));
			pkPeer->Encode(&it_member->second.bRole, sizeof(BYTE));

			pkPeer->EncodeHeader(HEADER_DG_PARTY_SET_MEMBER_LEVEL, 0, sizeof(TPacketPartySetMemberLevel));
			pkPeer->Encode(&it_party->first, sizeof(DWORD));
			pkPeer->Encode(&it_member->first, sizeof(DWORD));
			pkPeer->Encode(&it_member->second.bLevel, sizeof(BYTE));
		}
	}
}

void CClientManager::QUERY_PLAYER_COUNT(CPeer* pkPeer, TPlayerCountPacket* pPacket)
{
	pkPeer->SetUserCount(pPacket->dwCount);
}


void CClientManager::QUERY_SAFEBOX_LOAD(CPeer* pkPeer, DWORD dwHandle, TSafeboxLoadPacket* packet, bool bMall)
{
	ClientHandleInfo* pi = new ClientHandleInfo(dwHandle);
	strlcpy(pi->safebox_password, packet->szPassword, sizeof(pi->safebox_password));
	pi->account_id = packet->dwID;
	pi->account_index = 0;
	pi->ip[0] = bMall ? 1 : 0;
	strlcpy(pi->login, packet->szLogin, sizeof(pi->login));

	char szQuery[QUERY_MAX_LEN];
	snprintf(szQuery, sizeof(szQuery),
		"SELECT account_id, size, password FROM safebox%s WHERE account_id=%u",
		GetTablePostfix(), packet->dwID);

	if (g_log)
		sys_log(0, "HEADER_GD_SAFEBOX_LOAD (handle: %d account.id %u is_mall %d)", dwHandle, packet->dwID, bMall ? 1 : 0);

	CDBManager::instance().ReturnQuery(szQuery, QID_SAFEBOX_LOAD, pkPeer->GetHandle(), pi, SQL_PLAYER_SAFEBOX);
}

void CClientManager::RESULT_SAFEBOX_LOAD(CPeer* pkPeer, SQLMsg* msg)
{
	CQueryInfo* qi = (CQueryInfo*)msg->pvUserData;
	ClientHandleInfo* pi = (ClientHandleInfo*)qi->pvData;
	DWORD dwHandle = pi->dwHandle;

	if (pi->account_index == 0)
	{
		char szSafeboxPassword[SAFEBOX_PASSWORD_MAX_LEN + 1];
		strlcpy(szSafeboxPassword, pi->safebox_password, sizeof(szSafeboxPassword));

		TSafeboxTable* pSafebox = new TSafeboxTable;
		memset(pSafebox, 0, sizeof(TSafeboxTable));

		SQLResult* res = msg->Get();

		if (res->uiNumRows == 0)
		{
			if (strcmp("000000", szSafeboxPassword))
			{
				pkPeer->EncodeHeader(HEADER_DG_SAFEBOX_WRONG_PASSWORD, dwHandle, 0);
				delete pSafebox;
				delete pi;
				return;
			}
		}
		else
		{
			MYSQL_ROW row = mysql_fetch_row(res->pSQLResult);

			if (((!row[2] || !*row[2]) && strcmp("000000", szSafeboxPassword)) ||
				((row[2] && *row[2]) && strcmp(row[2], szSafeboxPassword)))
			{
				pkPeer->EncodeHeader(HEADER_DG_SAFEBOX_WRONG_PASSWORD, dwHandle, 0);
				delete pSafebox;
				delete pi;
				return;
			}

			if (!row[0])
				pSafebox->dwID = 0;
			else
				str_to_number(pSafebox->dwID, row[0]);

			if (!row[1])
				pSafebox->bSize = 0;
			else
				str_to_number(pSafebox->bSize, row[1]);
			/*
			   if (!row[3])
			   pSafebox->dwGold = 0;
			   else
			   pSafebox->dwGold = atoi(row[3]);
			   */
			if (pi->ip[0] == 1)
			{
				pSafebox->bSize = 1;
				sys_log(0, "MALL id[%d] size[%d]", pSafebox->dwID, pSafebox->bSize);
			}
			else
				sys_log(0, "SAFEBOX id[%d] size[%d]", pSafebox->dwID, pSafebox->bSize);
		}

		if (0 == pSafebox->dwID)
			pSafebox->dwID = pi->account_id;

		pi->pSafebox = pSafebox;

		char szQuery[QUERY_MAX_LEN];
		snprintf(szQuery, sizeof(szQuery),
			"SELECT id, `window`+0, pos, count, vnum, "
#ifdef __BEGINNER_ITEM__
			"is_basic, "
#endif
#ifdef __ITEM_EVOLUTION__
			"evolution, "
#endif
#ifdef __ITEM_CHANGELOOK__
			"transmutation, "
#endif
			"socket0, socket1, socket2, "
#ifdef __ITEM_EXTRA_SOCKET__
			"socket3, "
#endif
			"attrtype0, attrvalue0, "
			"attrtype1, attrvalue1, "
			"attrtype2, attrvalue2, "
			"attrtype3, attrvalue3, "
			"attrtype4, attrvalue4, "
			"attrtype5, attrvalue5, "
			"attrtype6, attrvalue6 "
			"FROM item%s WHERE owner_id=%d AND `window`='%s'",
			GetTablePostfix(), pi->account_id, pi->ip[0] == 0 ? "SAFEBOX" : "MALL");

		pi->account_index = 1;

		CDBManager::instance().ReturnQuery(szQuery, QID_SAFEBOX_LOAD, pkPeer->GetHandle(), pi, SQL_PLAYER_ITEM);
	}
	else
	{
		if (!pi->pSafebox)
		{
			sys_err("null safebox pointer!");
			delete pi;
			return;
		}

		if (!msg->Get()->pSQLResult)
		{
			sys_err("null safebox result");
			delete pi;
			return;
		}

		static std::vector<TPlayerItem> s_items;
		CreateItemTableFromRes(msg->Get()->pSQLResult, &s_items, pi->account_id);

		pi->pSafebox->wItemCount = s_items.size();

		pkPeer->EncodeHeader(pi->ip[0] == 0 ? HEADER_DG_SAFEBOX_LOAD : HEADER_DG_MALL_LOAD, dwHandle, sizeof(TSafeboxTable) + sizeof(TPlayerItem) * s_items.size());

		pkPeer->Encode(pi->pSafebox, sizeof(TSafeboxTable));

		if (!s_items.empty())
			pkPeer->Encode(&s_items[0], sizeof(TPlayerItem) * s_items.size());

		delete pi;
	}
}

void CClientManager::QUERY_SAFEBOX_CHANGE_SIZE(CPeer* pkPeer, DWORD dwHandle, TSafeboxChangeSizePacket* p)
{
	ClientHandleInfo* pi = new ClientHandleInfo(dwHandle);
	pi->account_index = p->bSize;

	char szQuery[QUERY_MAX_LEN];

	if (p->bSize == 1)
		snprintf(szQuery, sizeof(szQuery), "INSERT INTO safebox%s (account_id, size) VALUES(%u, %u)", GetTablePostfix(), p->dwID, p->bSize);
	else
		snprintf(szQuery, sizeof(szQuery), "UPDATE safebox%s SET size=%u WHERE account_id=%u", GetTablePostfix(), p->bSize, p->dwID);

	CDBManager::instance().ReturnQuery(szQuery, QID_SAFEBOX_CHANGE_SIZE, pkPeer->GetHandle(), pi, SQL_PLAYER_SAFEBOX);
}

void CClientManager::RESULT_SAFEBOX_CHANGE_SIZE(CPeer* pkPeer, SQLMsg* msg)
{
	CQueryInfo* qi = (CQueryInfo*)msg->pvUserData;
	ClientHandleInfo* p = (ClientHandleInfo*)qi->pvData;
	DWORD dwHandle = p->dwHandle;
	BYTE bSize = p->account_index;

	delete p;

	if (msg->Get()->uiNumRows > 0)
	{
		pkPeer->EncodeHeader(HEADER_DG_SAFEBOX_CHANGE_SIZE, dwHandle, sizeof(BYTE));
		pkPeer->EncodeBYTE(bSize);
	}
}

void CClientManager::QUERY_SAFEBOX_CHANGE_PASSWORD(CPeer* pkPeer, DWORD dwHandle, TSafeboxChangePasswordPacket* p)
{
	ClientHandleInfo* pi = new ClientHandleInfo(dwHandle);
	strlcpy(pi->safebox_password, p->szNewPassword, sizeof(pi->safebox_password));
	strlcpy(pi->login, p->szOldPassword, sizeof(pi->login));
	pi->account_id = p->dwID;

	char szQuery[QUERY_MAX_LEN];
	snprintf(szQuery, sizeof(szQuery), "SELECT password FROM safebox%s WHERE account_id=%u", GetTablePostfix(), p->dwID);

	CDBManager::instance().ReturnQuery(szQuery, QID_SAFEBOX_CHANGE_PASSWORD, pkPeer->GetHandle(), pi, SQL_PLAYER_SAFEBOX);
}

void CClientManager::RESULT_SAFEBOX_CHANGE_PASSWORD(CPeer* pkPeer, SQLMsg* msg)
{
	CQueryInfo* qi = (CQueryInfo*)msg->pvUserData;
	ClientHandleInfo* p = (ClientHandleInfo*)qi->pvData;
	DWORD dwHandle = p->dwHandle;

	if (msg->Get()->uiNumRows > 0)
	{
		MYSQL_ROW row = mysql_fetch_row(msg->Get()->pSQLResult);

		if ((row[0] && *row[0] && !strcasecmp(row[0], p->login)) || ((!row[0] || !*row[0]) && !strcmp("000000", p->login)))
		{
			char szQuery[QUERY_MAX_LEN];
			char escape_pwd[64];
			CDBManager::instance().EscapeString(escape_pwd, p->safebox_password, strlen(p->safebox_password));

			snprintf(szQuery, sizeof(szQuery), "UPDATE safebox%s SET password='%s' WHERE account_id=%u", GetTablePostfix(), escape_pwd, p->account_id);

			CDBManager::instance().ReturnQuery(szQuery, QID_SAFEBOX_CHANGE_PASSWORD_SECOND, pkPeer->GetHandle(), p, SQL_PLAYER_SAFEBOX);
			return;
		}
	}

	delete p;

	// Wrong old password
	pkPeer->EncodeHeader(HEADER_DG_SAFEBOX_CHANGE_PASSWORD_ANSWER, dwHandle, sizeof(BYTE));
	pkPeer->EncodeBYTE(0);
}

void CClientManager::RESULT_SAFEBOX_CHANGE_PASSWORD_SECOND(CPeer* pkPeer, SQLMsg* msg)
{
	CQueryInfo* qi = (CQueryInfo*)msg->pvUserData;
	ClientHandleInfo* p = (ClientHandleInfo*)qi->pvData;
	DWORD dwHandle = p->dwHandle;
	delete p;

	pkPeer->EncodeHeader(HEADER_DG_SAFEBOX_CHANGE_PASSWORD_ANSWER, dwHandle, sizeof(BYTE));
	pkPeer->EncodeBYTE(1);
}

void CClientManager::QUERY_SAFEBOX_SAVE(CPeer* pkPeer, TSafeboxTable* pTable)
{
	char szQuery[QUERY_MAX_LEN];

	snprintf(szQuery, sizeof(szQuery),

#ifdef __GOLD_LIMIT_REWORK__
		"UPDATE safebox%s SET gold='%lld' WHERE account_id=%u",
		GetTablePostfix(), pTable->llGold, pTable->dwID);
#else
		"UPDATE safebox%s SET gold='%u' WHERE account_id=%u",
		GetTablePostfix(), pTable->dwGold, pTable->dwID);
#endif
	CDBManager::instance().AsyncQuery(szQuery, SQL_PLAYER_SAFEBOX);
}

void CClientManager::QUERY_EMPIRE_SELECT(CPeer* pkPeer, DWORD dwHandle, TEmpireSelectPacket* p)
{
	char szQuery[QUERY_MAX_LEN];

	snprintf(szQuery, sizeof(szQuery), "UPDATE player_index%s SET empire=%u WHERE id=%u", GetTablePostfix(), p->bEmpire, p->dwAccountID);
	delete CDBManager::instance().DirectQuery(szQuery);

	sys_log(0, "EmpireSelect: %s", szQuery);
	{
#ifdef __PLAYER_PER_ACCOUNT_5__
		snprintf(szQuery, sizeof(szQuery),
			"SELECT pid1, pid2, pid3, pid4, pid5 FROM player_index%s WHERE id=%u", GetTablePostfix(), p->dwAccountID);
#else
		snprintf(szQuery, sizeof(szQuery),
			"SELECT pid1, pid2, pid3, pid4 FROM player_index%s WHERE id=%u", GetTablePostfix(), p->dwAccountID);
#endif

		std::unique_ptr<SQLMsg> pmsg(CDBManager::instance().DirectQuery(szQuery));

		SQLResult* pRes = pmsg->Get();

		if (pRes->uiNumRows)
		{
			sys_log(0, "EMPIRE %lu", pRes->uiNumRows);

			MYSQL_ROW row = mysql_fetch_row(pRes->pSQLResult);
			DWORD pids[PLAYER_PER_ACCOUNT]; // @warme701

			UINT g_start_map[4] =
			{
				0,  // reserved
				1,
				21,
				41
			};

			// FIXME share with game
			DWORD g_start_position[4][2] =
			{
				{      0,      0 },
				{ 469300, 964200 },
				{  55700, 157900 },
				{ 969600, 278400 }
			};

			for (long i = 0; i < PLAYER_PER_ACCOUNT; ++i) // @warme701
			{
				str_to_number(pids[i], row[i]);
				sys_log(0, "EMPIRE PIDS[%d]", pids[i]);

				if (pids[i])
				{
					sys_log(0, "EMPIRE move to pid[%d] to villiage of %u, map_index %d",
						pids[i], p->bEmpire, g_start_map[p->bEmpire]);

					snprintf(szQuery, sizeof(szQuery), "UPDATE player%s SET map_index=%u,x=%u,y=%u WHERE id=%u",
						GetTablePostfix(),
						g_start_map[p->bEmpire],
						g_start_position[p->bEmpire][0],
						g_start_position[p->bEmpire][1],
						pids[i]);

					std::unique_ptr<SQLMsg> pmsg2(CDBManager::instance().DirectQuery(szQuery));
				}
			}
		}
	}

	pkPeer->EncodeHeader(HEADER_DG_EMPIRE_SELECT, dwHandle, sizeof(BYTE));
	pkPeer->EncodeBYTE(p->bEmpire);
}

void CClientManager::QUERY_SETUP(CPeer* peer, DWORD dwHandle, const char* c_pData)
{
	TPacketGDSetup* p = (TPacketGDSetup*)c_pData;
	c_pData += sizeof(TPacketGDSetup);

	if (p->bAuthServer)
	{
		//Buradaki syslog geniþletildi hangi kanal hangi port ve hangi peer olduðu yazýyor artýk.
		sys_log(0, "AUTH_LOGIN on ch: %d port: %d ptr %p", p->bChannel, p->wListenPort, peer);
		m_pkAuthPeer = peer;
		return;
	}

	peer->SetPublicIP(p->szPublicIP);
	peer->SetChannel(p->bChannel);
	peer->SetListenPort(p->wListenPort);
	peer->SetP2PPort(p->wP2PPort);
	peer->SetMaps(p->alMaps);

	//

	//
	TMapLocation kMapLocations;

	strlcpy(kMapLocations.szHost, peer->GetPublicIP(), sizeof(kMapLocations.szHost));
	kMapLocations.wPort = peer->GetListenPort();
	thecore_memcpy(kMapLocations.alMaps, peer->GetMaps(), sizeof(kMapLocations.alMaps));

	BYTE bMapCount;

	std::vector<TMapLocation> vec_kMapLocations;

	if (peer->GetChannel() == 1)
	{
		for (itertype(m_peerList) i = m_peerList.begin(); i != m_peerList.end(); ++i)
		{
			CPeer* tmp = *i;

			if (tmp == peer)
				continue;

			if (!tmp->GetChannel())
				continue;

			if (tmp->GetChannel() == GUILD_WARP_WAR_CHANNEL || tmp->GetChannel() == peer->GetChannel())
			{
				TMapLocation kMapLocation2;
				strlcpy(kMapLocation2.szHost, tmp->GetPublicIP(), sizeof(kMapLocation2.szHost));
				kMapLocation2.wPort = tmp->GetListenPort();
				thecore_memcpy(kMapLocation2.alMaps, tmp->GetMaps(), sizeof(kMapLocation2.alMaps));
				vec_kMapLocations.push_back(kMapLocation2);

				tmp->EncodeHeader(HEADER_DG_MAP_LOCATIONS, 0, sizeof(BYTE) + sizeof(TMapLocation));
				bMapCount = 1;
				tmp->EncodeBYTE(bMapCount);
				tmp->Encode(&kMapLocations, sizeof(TMapLocation));
			}
		}
	}
	else if (peer->GetChannel() == GUILD_WARP_WAR_CHANNEL)
	{
		for (itertype(m_peerList) i = m_peerList.begin(); i != m_peerList.end(); ++i)
		{
			CPeer* tmp = *i;

			if (tmp == peer)
				continue;

			if (!tmp->GetChannel())
				continue;

			if (tmp->GetChannel() == 1 || tmp->GetChannel() == peer->GetChannel())
			{
				TMapLocation kMapLocation2;
				strlcpy(kMapLocation2.szHost, tmp->GetPublicIP(), sizeof(kMapLocation2.szHost));
				kMapLocation2.wPort = tmp->GetListenPort();
				thecore_memcpy(kMapLocation2.alMaps, tmp->GetMaps(), sizeof(kMapLocation2.alMaps));
				vec_kMapLocations.push_back(kMapLocation2);
			}

			tmp->EncodeHeader(HEADER_DG_MAP_LOCATIONS, 0, sizeof(BYTE) + sizeof(TMapLocation));
			bMapCount = 1;
			tmp->EncodeBYTE(bMapCount);
			tmp->Encode(&kMapLocations, sizeof(TMapLocation));
		}
	}
	else
	{
		for (itertype(m_peerList) i = m_peerList.begin(); i != m_peerList.end(); ++i)
		{
			CPeer* tmp = *i;

			if (tmp == peer)
				continue;

			if (!tmp->GetChannel())
				continue;

			if (tmp->GetChannel() == GUILD_WARP_WAR_CHANNEL || tmp->GetChannel() == peer->GetChannel())
			{
				TMapLocation kMapLocation2;

				strlcpy(kMapLocation2.szHost, tmp->GetPublicIP(), sizeof(kMapLocation2.szHost));
				kMapLocation2.wPort = tmp->GetListenPort();
				thecore_memcpy(kMapLocation2.alMaps, tmp->GetMaps(), sizeof(kMapLocation2.alMaps));

				vec_kMapLocations.push_back(kMapLocation2);
			}

			if (tmp->GetChannel() == peer->GetChannel())
			{
				tmp->EncodeHeader(HEADER_DG_MAP_LOCATIONS, 0, sizeof(BYTE) + sizeof(TMapLocation));
				bMapCount = 1;
				tmp->EncodeBYTE(bMapCount);
				tmp->Encode(&kMapLocations, sizeof(TMapLocation));
			}
		}
	}

	vec_kMapLocations.push_back(kMapLocations);

	peer->EncodeHeader(HEADER_DG_MAP_LOCATIONS, 0, sizeof(BYTE) + sizeof(TMapLocation) * vec_kMapLocations.size());
	bMapCount = vec_kMapLocations.size();
	peer->EncodeBYTE(bMapCount);
	peer->Encode(&vec_kMapLocations[0], sizeof(TMapLocation) * vec_kMapLocations.size());

	//

	//
	sys_log(0, "SETUP: channel %u listen %u p2p %u count %u", peer->GetChannel(), p->wListenPort, p->wP2PPort, bMapCount);

	TPacketDGP2P p2pSetupPacket;
	p2pSetupPacket.wPort = peer->GetP2PPort();
	p2pSetupPacket.bChannel = peer->GetChannel();
	strlcpy(p2pSetupPacket.szHost, peer->GetPublicIP(), sizeof(p2pSetupPacket.szHost));

	for (itertype(m_peerList) i = m_peerList.begin(); i != m_peerList.end(); ++i)
	{
		CPeer* tmp = *i;

		if (tmp == peer)
			continue;

		if (0 == tmp->GetChannel())
			continue;

		tmp->EncodeHeader(HEADER_DG_P2P, 0, sizeof(TPacketDGP2P));
		tmp->Encode(&p2pSetupPacket, sizeof(TPacketDGP2P));
	}

	//

	//
	TPacketLoginOnSetup* pck = (TPacketLoginOnSetup*)c_pData;

	for (DWORD c = 0; c < p->dwLoginCount; ++c, ++pck)
	{
		CLoginData* pkLD = new CLoginData;

		pkLD->SetKey(pck->dwLoginKey);
		pkLD->SetClientKey(pck->adwClientKey);
		pkLD->SetIP(pck->szHost);

		TAccountTable& r = pkLD->GetAccountRef();

		r.id = pck->dwID;
		trim_and_lower(pck->szLogin, r.login, sizeof(r.login));
		strlcpy(r.social_id, pck->szSocialID, sizeof(r.social_id));
		strlcpy(r.passwd, "TEMP", sizeof(r.passwd));
#ifdef __HWID_SECURITY_UTILITY__
		strlcpy(r.cpu_id, pck->cpu_id, sizeof(r.cpu_id));
		strlcpy(r.hdd_model, pck->hdd_model, sizeof(r.hdd_model));
		strlcpy(r.machine_guid, pck->machine_guid, sizeof(r.machine_guid));
		strlcpy(r.mac_addr, pck->mac_addr, sizeof(r.mac_addr));
		strlcpy(r.hdd_serial, pck->hdd_serial, sizeof(r.hdd_serial));
		strlcpy(r.bios_id, pck->bios_id, sizeof(r.bios_id));
		strlcpy(r.pc_name, pck->pc_name, sizeof(r.pc_name));
		strlcpy(r.user_name, pck->user_name, sizeof(r.user_name));
		strlcpy(r.os_version, pck->os_version, sizeof(r.os_version));
#endif // __HWID_SECURITY_UTILITY__
		InsertLoginData(pkLD);

		if (InsertLogonAccount(pck->szLogin, peer->GetHandle(), pck->szHost))
		{
			sys_log(0, "SETUP: login %u %s login_key %u host %s", pck->dwID, pck->szLogin, pck->dwLoginKey, pck->szHost);
			pkLD->SetPlay(true);
		}
		else
			sys_log(0, "SETUP: login_fail %u %s login_key %u", pck->dwID, pck->szLogin, pck->dwLoginKey);
	}

	SendPartyOnSetup(peer);
	CGuildManager::instance().OnSetup(peer);
	CPrivManager::instance().SendPrivOnSetup(peer);
	SendEventFlagsOnSetup(peer);
}

#ifdef __SKILL_COLOR__
void CClientManager::QUERY_SKILL_COLOR_SAVE(const char* c_pData)
{
	PutSkillColorCache((TSkillColor*)c_pData);
}
#endif


void CClientManager::QUERY_ITEM_FLUSH(CPeer* pkPeer, const char* c_pData)
{
	DWORD dwID = *(DWORD*)c_pData;

	if (g_log)
		sys_log(0, "HEADER_GD_ITEM_FLUSH: %u", dwID);

	CItemCache* c = GetItemCache(dwID);

	if (c)
		c->Flush();
}

void CClientManager::QUERY_ITEM_SAVE(CPeer* pkPeer, const char* c_pData)
{
	TPlayerItem* p = (TPlayerItem*)c_pData;

	if (g_log)
		sys_log(0, "QUERY_ITEM_SAVE id %u ownerid %u", p->id, p->owner);

	if (p->window == SAFEBOX || p->window == MALL)
	{
		CItemCache* c = GetItemCache(p->id);

		if (c)
		{
			TItemCacheSetPtrMap::iterator it = m_map_pkItemCacheSetPtr.find(c->Get()->owner);

			if (it != m_map_pkItemCacheSetPtr.end())
			{
				if (g_test_server)
					sys_log(0, "ITEM_CACHE: safebox owner %u id %u", c->Get()->owner, c->Get()->id);

				it->second->erase(c);
			}

			m_map_itemCache.erase(p->id);

			delete c;
		}

		char szQuery[QUERY_MAX_LEN];

		snprintf(szQuery, sizeof(szQuery),
			"REPLACE INTO item%s (id, owner_id, `window`, pos, count, vnum, "
#ifdef __BEGINNER_ITEM__
			"is_basic, "
#endif
#ifdef __ITEM_EVOLUTION__
			"evolution, "
#endif
#ifdef __ITEM_CHANGELOOK__
			"transmutation, "
#endif
			"socket0, socket1, socket2, "
#ifdef __ITEM_EXTRA_SOCKET__
			"socket3, "
#endif
			"attrtype0, attrvalue0, "
			"attrtype1, attrvalue1, "
			"attrtype2, attrvalue2, "
			"attrtype3, attrvalue3, "
			"attrtype4, attrvalue4, "
			"attrtype5, attrvalue5, "
			"attrtype6, attrvalue6) "
			"VALUES(%u, %u, %d, %d, %u, %u, "
#ifdef __BEGINNER_ITEM__
			"%d, "
#endif
#ifdef __ITEM_EVOLUTION__
			"%d, "
#endif
#ifdef __ITEM_CHANGELOOK__
			"%u, "
#endif
			"%ld, %ld, %ld, "
#ifdef __ITEM_EXTRA_SOCKET__
			"%ld, "
#endif
			"%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d)",
			GetTablePostfix(),
			p->id,
			p->owner,
			p->window,
			p->pos,
			p->count,
			p->vnum,
#ifdef __BEGINNER_ITEM__
			p->is_basic,
#endif
#ifdef __ITEM_EVOLUTION__
			p->evolution,
#endif
#ifdef __ITEM_CHANGELOOK__
			p->transmutation,
#endif
			p->alSockets[0],
			p->alSockets[1],
			p->alSockets[2],
#ifdef __ITEM_EXTRA_SOCKET__
			p->alSockets[3],
#endif
			p->aAttr[0].bType, p->aAttr[0].sValue,
			p->aAttr[1].bType, p->aAttr[1].sValue,
			p->aAttr[2].bType, p->aAttr[2].sValue,
			p->aAttr[3].bType, p->aAttr[3].sValue,
			p->aAttr[4].bType, p->aAttr[4].sValue,
			p->aAttr[5].bType, p->aAttr[5].sValue,
			p->aAttr[6].bType, p->aAttr[6].sValue);

		CDBManager::instance().AsyncQuery(szQuery, SQL_PLAYER_ITEM);
	}
	else
	{
		if (g_test_server)
			sys_log(0, "QUERY_ITEM_SAVE => PutItemCache() owner %d id %d vnum %d ", p->owner, p->id, p->vnum);

		PutItemCache(p);
	}
}

CClientManager::TItemCacheSet* CClientManager::GetItemCacheSet(DWORD pid)
{
	TItemCacheSetPtrMap::iterator it = m_map_pkItemCacheSetPtr.find(pid);

	if (it == m_map_pkItemCacheSetPtr.end())
		return NULL;

	return it->second;
}

void CClientManager::QUERY_QUEST_SAVE(CPeer* pkPeer, const char* c_pData, DWORD dwLen)
{
	TQuestTable* p = (TQuestTable*)c_pData;

	// if (g_test_server)
	// 	sys_log(0, "QUERY_QUEST_SAVE => PutQuestCache() owner %d id %d vnum %d ", p->owner, p->id, p->vnum);

	int iSize = dwLen / sizeof(TQuestTable);

	TQuestCacheSetPtrMap::iterator it = m_map_pkQuestCacheSetPtr.find(p->dwPID);

	TQuestCacheSet* pSet = nullptr;

	if (it != m_map_pkQuestCacheSetPtr.end())
		pSet = it->second;
	else
	{
		sys_err("player not in quest cache %u %d", p->dwPID, iSize);
		// pSet = CreateQuestCacheSet(p->dwPID);
		return;
	}

	for (int i = 0; i < iSize; ++i, ++p)
	{
		auto itt = pSet->begin();
		// sys_log(0, "QUEST_CACHE: itt %p, end %p", itt, pSet->end());
		for (; itt != pSet->end(); ++itt)
		{
			// sys_log(0, "QUEST_CACHE: in for %p", itt);
			auto ttb = (*itt)->Get();

			if (strcmp(ttb->szName, p->szName)==0 && strcmp(ttb->szState, p->szState)==0)
			{
				// sys_log(0, "QUEST_CACHE: found %u %s %s %ld", p->dwPID, p->szName, p->szState, p->lValue);
				break;
			}
		}

		// sys_log(0, "QUEST_CACHE: save %u %s %s %ld", p->dwPID, p->szName, p->szState, p->lValue);

		if (p->lValue == 0)
		{
			sys_log(0, "QUEST_CACHE: erase %u %s %s %ld", p->dwPID, p->szName, p->szState, p->lValue);
			if (itt != pSet->end())
				pSet->erase(itt);
		}
		else
		{
			CQuestCache* c;

			if (itt != pSet->end())
			{
				sys_log(0, "QUEST_CACHE: change %u %s %s %ld", p->dwPID, p->szName, p->szState, p->lValue);
				c = *itt;
			}
			else
			{
				sys_log(0, "QUEST_CACHE: add %u %s %s %ld", p->dwPID, p->szName, p->szState, p->lValue);
				c = new CQuestCache;
				pSet->insert(c);
			}

			if (!c)
			{
				sys_err("bu ne moruk");
				continue;
			}

			c->Put(p);
		}

	}

}
CClientManager::TQuestCacheSet* CClientManager::GetQuestCacheSet(DWORD pid)
{
	TQuestCacheSetPtrMap::iterator it = m_map_pkQuestCacheSetPtr.find(pid);

	if (it == m_map_pkQuestCacheSetPtr.end())
		return NULL;

	return it->second;
}

CClientManager::TQuestCacheSet* CClientManager::CreateQuestCacheSet(DWORD pid)
{
	// ! bu fonksiyonu kullanmadan once kesinlikle find yap yoksa leak olur
	TQuestCacheSet* pSet = new TQuestCacheSet;
	m_map_pkQuestCacheSetPtr.insert(TQuestCacheSetPtrMap::value_type(pid, pSet));

	if (g_log)
		sys_log(0, "QUEST_CACHE: new cache %u", pid);

	return pSet;
}

void CClientManager::FlushQuestCacheSet(DWORD pid)
{
	TQuestCacheSetPtrMap::iterator it = m_map_pkQuestCacheSetPtr.find(pid);

	if (it == m_map_pkQuestCacheSetPtr.end())
	{
		sys_log(0, "FLUSH_QUESTCACHESET : No QuestCacheSet pid(%d)", pid);
		return;
	}

	char szQuery[1024];
	snprintf(szQuery, sizeof(szQuery), "DELETE FROM quest%s WHERE dwPID = %u", GetTablePostfix(), pid);
	CDBManager::instance().AsyncQuery(szQuery, SQL_PLAYER_QUEST);

	TQuestCacheSet* pSet = it->second;
	TQuestCacheSet::iterator it_set = pSet->begin();

	while (it_set != pSet->end())
	{
		CQuestCache* c = *it_set++;
		c->Flush();

		delete c;
	}

	pSet->clear();
	delete pSet;

	m_map_pkQuestCacheSetPtr.erase(it);

	if (g_log)
		sys_log(0, "FLUSH_QUESTCACHESET : Deleted pid(%d)", pid);
}

void CClientManager::QUERY_AFFECT_SAVE(CPeer* pkPeer, const char* c_pData, DWORD dwLen)
{
	TPacketGDAddAffect* p = (TPacketGDAddAffect*)c_pData;

	int iSize = dwLen / sizeof(TPacketGDAddAffect);

	if (!iSize)
		return;

	sys_log(0, "QUERY_AFFECT_SAVE pid %u size %d", p->dwPID, iSize);

	TAffectCacheSetPtrMap::iterator it = m_map_pkAffectCacheSetPtr.find(p->dwPID);

	TAffectCacheSet* pSet = nullptr;

	if (it != m_map_pkAffectCacheSetPtr.end())
	{ // eger butun affleri save paketi geldiyse, cachemizde varsa hepsini temizle ve yenilerini ekle.
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
	{ // yoksa olustur ve yenilerini ekle
		pSet = CreateAffectCacheSet(p->dwPID);
		sys_err("pset new %d", p->dwPID);
	}

	if (!pSet)
	{
		sys_err("psetcannot ");
		return;
	}

	for (int i = 0; i < iSize; ++i, ++p)
	{
		CAffectCache* c;
		c = new CAffectCache;

		c->Put(p);
		pSet->insert(c);

	}
}
void CClientManager::QUERY_AFFECT_ADD(CPeer* pkPeer, const char* c_pData, DWORD dwLen)
{
	TPacketGDAddAffect* p = (TPacketGDAddAffect*)c_pData;

	int iSize = dwLen / sizeof(TPacketGDAddAffect);

	TAffectCacheSetPtrMap::iterator it = m_map_pkAffectCacheSetPtr.find(p->dwPID);

	TAffectCacheSet* pSet = nullptr;

	if (it != m_map_pkAffectCacheSetPtr.end())
	{
		pSet = it->second;
	}
	else
	{
		sys_err("player not in affect cache %u %d", p->dwPID, iSize);
		// pSet = CreateAffectCacheSet(p->dwPID);
		return;
	}

	if (!pSet)
	{
		sys_err("psetcannot ");
		return;
	}

	for (int i = 0; i < iSize; ++i, ++p)
	{
		CAffectCache* c;

		//sys_log(0, "AFFECT_CACHE: add %u %u %d", p->dwPID, p->elem.dwType, p->elem.bApplyOn);
		c = new CAffectCache;

		c->Put(p);
		pSet->insert(c);
	}
}
void CClientManager::QUERY_AFFECT_CHANGE(CPeer* pkPeer, const char* c_pData, DWORD dwLen)
{
	TPacketGDAddAffect* p = (TPacketGDAddAffect*)c_pData;

	// if (g_test_server)

	TAffectCacheSetPtrMap::iterator it = m_map_pkAffectCacheSetPtr.find(p->dwPID);

	TAffectCacheSet* pSet = nullptr;

	if (it != m_map_pkAffectCacheSetPtr.end())
		pSet = it->second;
	else
	{
		sys_err("psetcannot found %u", p->dwPID);
		return;
	}

	auto itt = pSet->begin();
	// sys_log(0, "AFFECT_CACHE: itt %p, end %p", itt, pSet->end());
	for (; itt != pSet->end(); ++itt)
	{
		// sys_log(0, "AFFECT_CACHE: in for %p", itt);
		auto ttb = (*itt)->Get();

		if (ttb->elem.dwType == p->elem.dwType && ttb->elem.bApplyOn == p->elem.bApplyOn)
		{
			sys_log(0, "AFFECT_CACHE: found %u %u %d", p->dwPID, p->elem.dwType, p->elem.bApplyOn);
			break;
		}
	}

	CAffectCache* c;

	if (itt != pSet->end())
	{
		sys_log(0, "AFFECT_CACHE: change %u %u %d", p->dwPID, p->elem.dwType, p->elem.bApplyOn);
		c = *itt;
	}
	else
	{
		sys_err(0, "QUERY_AFFECT_CHANGE not fount %u %u %d", p->dwPID, p->elem.dwType, p->elem.bApplyOn);
		c = new CAffectCache;
	}

	if (!c)
	{
		sys_err("bu ne moruk");
		return;
	}

	c->Put(p);
	pSet->insert(c);

}
void CClientManager::QUERY_AFFECT_REMOVE(CPeer* pkPeer, const char* c_pData, DWORD dwLen)
{
	TPacketGDRemoveAffect* p = (TPacketGDRemoveAffect*)c_pData;

	TAffectCacheSetPtrMap::iterator it = m_map_pkAffectCacheSetPtr.find(p->dwPID);

	TAffectCacheSet* pSet = nullptr;

	if (it != m_map_pkAffectCacheSetPtr.end())
		pSet = it->second;
	else
	{
		sys_err("psetcannot found %u", p->dwPID);
		return;
	}

	auto itt = pSet->begin();
	// sys_log(0, "AFFECT_CACHE: itt %p, end %p", itt, pSet->end());
	for (; itt != pSet->end();)
	{
		// sys_log(0, "AFFECT_CACHE: in for %p", itt);
		auto ttb = (*itt)->Get();

		if (ttb->elem.dwType == p->dwType && ttb->elem.bApplyOn == p->bApplyOn)
		{
			//sys_log(0, "AFFECT_CACHE: delete %u %u %d", p->dwPID, p->dwType, p->bApplyOn);
			itt = pSet->erase(itt);
		}
		else
		{
			++itt;
		}
	}

}
CClientManager::TAffectCacheSet* CClientManager::GetAffectCacheSet(DWORD pid)
{
	TAffectCacheSetPtrMap::iterator it = m_map_pkAffectCacheSetPtr.find(pid);

	if (it == m_map_pkAffectCacheSetPtr.end())
		return NULL;

	return it->second;
}

CClientManager::TAffectCacheSet* CClientManager::CreateAffectCacheSet(DWORD pid)
{
	// ! bu fonksiyonu kullanmadan once kesinlikle find yap yoksa leak olur
	TAffectCacheSet* pSet = new TAffectCacheSet;
	m_map_pkAffectCacheSetPtr.insert(TAffectCacheSetPtrMap::value_type(pid, pSet));

	if (g_log)
		sys_log(0, "AFFECT_CACHE: new cache %u", pid);

	return pSet;
}

void CClientManager::FlushAffectCacheSet(DWORD pid)
{
	TAffectCacheSetPtrMap::iterator it = m_map_pkAffectCacheSetPtr.find(pid);

	if (it == m_map_pkAffectCacheSetPtr.end())
	{
		sys_log(0, "FLUSH_AFFECTCACHESET : No AffectCacheSet pid(%d)", pid);
		return;
	}

	char szQuery[1024];
	snprintf(szQuery, sizeof(szQuery), "DELETE FROM affect%s WHERE dwPID = %u", GetTablePostfix(), pid);
	CDBManager::instance().AsyncQuery(szQuery, SQL_PLAYER_AFFECT);

	TAffectCacheSet* pSet = it->second;
	TAffectCacheSet::iterator it_set = pSet->begin();

	while (it_set != pSet->end())
	{
		CAffectCache* c = *it_set++;
		c->Flush();

		delete c;
	}

	pSet->clear();
	delete pSet;

	m_map_pkAffectCacheSetPtr.erase(it);

	if (g_log)
		sys_log(0, "FLUSH_AFFECTCACHESET : Deleted pid(%d)", pid);
}

#ifdef __BATTLE_PASS_SYSTEM__
CClientManager::TBattlePassCacheSet* CClientManager::GetBattlePassCacheSet(DWORD pid)
{
	TBattlePassCacheSetPtrMap::iterator it = m_map_pkBattlePassCacheSetPtr.find(pid);

	if (it == m_map_pkBattlePassCacheSetPtr.end())
		return NULL;

	return it->second;
}

CClientManager::TBattlePassCacheSet* CClientManager::CreateBattlePassCacheSet(DWORD pid)
{
	TBattlePassCacheSet* pSet = new TBattlePassCacheSet;
	m_map_pkBattlePassCacheSetPtr.insert(TBattlePassCacheSetPtrMap::value_type(pid, pSet));

	if (g_log)
		sys_log(0, "BATTLE_PASS: new cache %u", pid);

	return pSet;
}

void CClientManager::QUERY_SAVE_BATTLE_PASS(CPeer* pkPeer, const char* c_pData, DWORD dwLen)
{
	TPlayerBattlePassMission* p = (TPlayerBattlePassMission*)c_pData;

	int iSize = dwLen / sizeof(TPlayerBattlePassMission);

	sys_log(0, "QUERY_SAVE_BATTLE_PASS pid %u size %d", p->dwPlayerId, iSize);

	TBattlePassCacheSetPtrMap::iterator it = m_map_pkBattlePassCacheSetPtr.find(p->dwPlayerId);
	TBattlePassCacheSet* pSet = nullptr;

	if (it != m_map_pkBattlePassCacheSetPtr.end())
	{
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
		pSet = CreateBattlePassCacheSet(p->dwPlayerId);
		sys_err("pset new %d", p->dwPlayerId);
	}

	if (!pSet)
	{
		sys_err("psetcannot ");
		return;
	}

	for (int i = 0; i < iSize; ++i, ++p)
	{
		CBattlePassCache* c;
		c = new CBattlePassCache;

		c->Put(p);
		pSet->insert(c);
	}
}

void CClientManager::QUERY_BATTLE_PASS_ADD(CPeer* pkPeer, const char* c_pData, DWORD dwLen)
{
	TPlayerBattlePassMission* p = (TPlayerBattlePassMission*)c_pData;

	int iSize = dwLen / sizeof(TPlayerBattlePassMission);

	TBattlePassCacheSetPtrMap::iterator it = m_map_pkBattlePassCacheSetPtr.find(p->dwPlayerId);

	TBattlePassCacheSet* pSet = nullptr;

	if (it != m_map_pkBattlePassCacheSetPtr.end())
	{
		pSet = it->second;
	}
	else
	{
		sys_err("player not in bp cache %u %d", p->dwPlayerId, iSize);
		return;
	}

	if (!pSet)
	{
		sys_err("psetcannot ");
		return;
	}

	for (int i = 0; i < iSize; ++i, ++p)
	{
		CBattlePassCache* c;
		c = new CBattlePassCache;

		c->Put(p);
		pSet->insert(c);
	}
}

void CClientManager::FlushBattlePassCacheSet(DWORD pid)
{
	TBattlePassCacheSetPtrMap::iterator it = m_map_pkBattlePassCacheSetPtr.find(pid);

	if (it == m_map_pkBattlePassCacheSetPtr.end())
	{
		sys_log(0, "FlushBattlePassCacheSet : No BattlePassCacheSet pid(%d)", pid);
		return;
	}

	char szQuery[1024];
	snprintf(szQuery, sizeof(szQuery), "DELETE FROM battle_pass%s WHERE player_id = %u", GetTablePostfix(), pid);
	CDBManager::instance().AsyncQuery(szQuery, SQL_PLAYER_BATTLE_PASS);

	TBattlePassCacheSet* pSet = it->second;
	TBattlePassCacheSet::iterator it_set = pSet->begin();

	while (it_set != pSet->end())
	{
		CBattlePassCache* c = *it_set++;
		c->Flush();

		delete c;
	}

	pSet->clear();
	delete pSet;

	m_map_pkBattlePassCacheSetPtr.erase(it);

	if (g_log)
		sys_log(0, "FlushBattlePassCacheSet : Deleted pid(%d)", pid);
}
#endif // __BATTLE_PASS_SYSTEM__

void CClientManager::CreateItemCacheSet(DWORD pid)
{
	if (m_map_pkItemCacheSetPtr.find(pid) != m_map_pkItemCacheSetPtr.end())
		return;

	TItemCacheSet* pSet = new TItemCacheSet;
	m_map_pkItemCacheSetPtr.insert(TItemCacheSetPtrMap::value_type(pid, pSet));

	if (g_log)
		sys_log(0, "ITEM_CACHE: new cache %u", pid);
}

void CClientManager::FlushItemCacheSet(DWORD pid)
{
	TItemCacheSetPtrMap::iterator it = m_map_pkItemCacheSetPtr.find(pid);

	if (it == m_map_pkItemCacheSetPtr.end())
	{
		sys_log(0, "FLUSH_ITEMCACHESET : No ItemCacheSet pid(%d)", pid);
		return;
	}

	TItemCacheSet* pSet = it->second;
	TItemCacheSet::iterator it_set = pSet->begin();

	while (it_set != pSet->end())
	{
		CItemCache* c = *it_set++;
		c->Flush();

		m_map_itemCache.erase(c->Get()->id);
		delete c;
	}

	pSet->clear();
	delete pSet;

	m_map_pkItemCacheSetPtr.erase(it);

	if (g_log)
		sys_log(0, "FLUSH_ITEMCACHESET : Deleted pid(%d)", pid);
}

CItemCache* CClientManager::GetItemCache(DWORD id)
{
	TItemCacheMap::iterator it = m_map_itemCache.find(id);

	if (it == m_map_itemCache.end())
		return NULL;

	return it->second;
}

void CClientManager::PutItemCache(TPlayerItem* pNew, bool bSkipQuery)
{
	CItemCache* c;

	c = GetItemCache(pNew->id);

	if (!c)
	{
		if (g_log)
			sys_log(0, "ITEM_CACHE: PutItemCache ==> New CItemCache id%d vnum%d new owner%d", pNew->id, pNew->vnum, pNew->owner);

		c = new CItemCache;
		m_map_itemCache.insert(TItemCacheMap::value_type(pNew->id, c));
	}

	else
	{
		if (g_log)
			sys_log(0, "ITEM_CACHE: PutItemCache ==> Have Cache");

		if (pNew->owner != c->Get()->owner)
		{
			TItemCacheSetPtrMap::iterator it = m_map_pkItemCacheSetPtr.find(c->Get()->owner);

			if (it != m_map_pkItemCacheSetPtr.end())
			{
				if (g_log)
					sys_log(0, "ITEM_CACHE: delete owner %u id %u new owner %u", c->Get()->owner, c->Get()->id, pNew->owner);
				it->second->erase(c);
			}
		}
	}

	c->Put(pNew, bSkipQuery);

	TItemCacheSetPtrMap::iterator it = m_map_pkItemCacheSetPtr.find(c->Get()->owner);

	if (it != m_map_pkItemCacheSetPtr.end())
	{
		if (g_log)
			sys_log(0, "ITEM_CACHE: save %u id %u", c->Get()->owner, c->Get()->id);
		else
			sys_log(1, "ITEM_CACHE: save %u id %u", c->Get()->owner, c->Get()->id);
		it->second->insert(c);
	}
	else
	{
		if (g_log)
			sys_log(0, "ITEM_CACHE: direct save %u id %u", c->Get()->owner, c->Get()->id);
		else
			sys_log(1, "ITEM_CACHE: direct save %u id %u", c->Get()->owner, c->Get()->id);

		c->OnFlush();
	}
}

bool CClientManager::DeleteItemCache(DWORD dwID)
{
	CItemCache* c = GetItemCache(dwID);

	if (!c)
		return false;

	c->Delete();
	return true;
}

void CClientManager::UpdatePlayerCache()
{
	TPlayerTableCacheMap::iterator it = m_map_playerCache.begin();

	while (it != m_map_playerCache.end())
	{
		CPlayerTableCache* c = (it++)->second;

		if (c->CheckTimeout())
		{
			if (g_log)
				sys_log(0, "UPDATE : UpdatePlayerCache() ==> FlushPlayerCache %d %s ", c->Get(false)->id, c->Get(false)->name);

			c->Flush();

			UpdateItemCacheSet(c->Get()->id);
		}
		else if (c->CheckFlushTimeout())
			c->Flush();
	}
}

#ifdef __SKILL_COLOR__
CSKillColorCache* CClientManager::GetSkillColorCache(DWORD id)
{
	TSkillColorCacheMap::iterator it = m_map_SkillColorCache.find(id);

	if (it == m_map_SkillColorCache.end())
		return NULL;

	return it->second;
}

void CClientManager::PutSkillColorCache(const TSkillColor* pNew)
{
	CSKillColorCache* pCache = GetSkillColorCache(pNew->player_id);

	if (!pCache)
	{
		pCache = new CSKillColorCache;
		m_map_SkillColorCache.insert(TSkillColorCacheMap::value_type(pNew->player_id, pCache));
	}

	pCache->Put(const_cast<TSkillColor*>(pNew), false);
}

void CClientManager::UpdateSkillColorCache()
{
	TSkillColorCacheMap::iterator it = m_map_SkillColorCache.begin();

	while (it != m_map_SkillColorCache.end())
	{
		CSKillColorCache* pCache = it->second;

		if (pCache->CheckFlushTimeout())
		{
			pCache->Flush();
			m_map_SkillColorCache.erase(it++);
			delete pCache;
		}
		else
			++it;
	}
}
#endif
void CClientManager::SetCacheFlushCountLimit(int iLimit)
{
	m_iCacheFlushCountLimit = MAX(10, iLimit);
	sys_log(0, "CACHE_FLUSH_LIMIT_PER_SECOND: %d", m_iCacheFlushCountLimit);
}

void CClientManager::UpdateItemCache()
{
	if (m_iCacheFlushCount >= m_iCacheFlushCountLimit)
		return;

	TItemCacheMap::iterator it = m_map_itemCache.begin();

	while (it != m_map_itemCache.end())
	{
		CItemCache* c = (it++)->second;

		if (c->CheckFlushTimeout())
		{
			if (g_test_server)
				sys_log(0, "UpdateItemCache ==> Flush() vnum %d id owner %d", c->Get()->vnum, c->Get()->id, c->Get()->owner);

			c->Flush();

			if (++m_iCacheFlushCount >= m_iCacheFlushCountLimit)
				break;
		}
	}
}

void CClientManager::QUERY_ITEM_DESTROY(CPeer* pkPeer, const char* c_pData)
{
	DWORD dwID = *(DWORD*)c_pData;
	c_pData += sizeof(DWORD);

	DWORD dwPID = *(DWORD*)c_pData;

	if (!DeleteItemCache(dwID))
	{
		char szQuery[64];
		snprintf(szQuery, sizeof(szQuery), "DELETE FROM item%s WHERE id=%u", GetTablePostfix(), dwID);

		if (g_log)
			sys_log(0, "HEADER_GD_ITEM_DESTROY: PID %u ID %u", dwPID, dwID);

		CDBManager::instance().AsyncQuery(szQuery, SQL_PLAYER_ITEM);
	}
}

void CClientManager::QUERY_FLUSH_CACHE(CPeer* pkPeer, const char* c_pData)
{
	DWORD dwPID = *(DWORD*)c_pData;

	CPlayerTableCache* pkCache = GetPlayerCache(dwPID);

	if (!pkCache)
		return;

	sys_log(0, "FLUSH_CACHE: %u", dwPID);

	pkCache->Flush();
	FlushItemCacheSet(dwPID);
	// FlushQuestCacheSet(dwPID);
	// FlushAffectCacheSet(dwPID);

	m_map_playerCache.erase(dwPID);
	delete pkCache;
}

void CClientManager::QUERY_RELOAD_PROTO()
{
	if (!InitializeTables())
	{
		sys_err("QUERY_RELOAD_PROTO: cannot load tables");
		return;
	}

	for (TPeerList::iterator i = m_peerList.begin(); i != m_peerList.end(); ++i)
	{
		CPeer* tmp = *i;

		if (!tmp->GetChannel())
			continue;

		tmp->EncodeHeader(HEADER_DG_RELOAD_PROTO, 0,
			sizeof(WORD) + sizeof(TSkillTable) * m_vec_skillTable.size() +
			sizeof(WORD) + sizeof(TItemTable) * m_vec_itemTable.size() +
#ifdef __RELOAD_REWORK__
			sizeof(WORD) + sizeof(TMobTable) * m_vec_mobTable.size() +
			sizeof(WORD) + sizeof(TShopTable) * m_iShopTableSize +
			sizeof(WORD) + sizeof(TRefineTable) * m_iRefineTableSize +
			sizeof(WORD) + sizeof(TItemAttrTable) * m_vec_itemAttrTable.size() +
			sizeof(WORD) + sizeof(TItemAttrTable) * m_vec_itemRareTable.size());
#else
			sizeof(WORD) + sizeof(TMobTable) * m_vec_mobTable.size());
#endif
		tmp->EncodeWORD(m_vec_skillTable.size());
		tmp->Encode(&m_vec_skillTable[0], sizeof(TSkillTable) * m_vec_skillTable.size());

		tmp->EncodeWORD(m_vec_itemTable.size());
		tmp->Encode(&m_vec_itemTable[0], sizeof(TItemTable) * m_vec_itemTable.size());

		tmp->EncodeWORD(m_vec_mobTable.size());
		tmp->Encode(&m_vec_mobTable[0], sizeof(TMobTable) * m_vec_mobTable.size());
#ifdef __RELOAD_REWORK__
		tmp->EncodeWORD(m_iShopTableSize);
		tmp->Encode(m_pShopTable, sizeof(TShopTable) * m_iShopTableSize);

		tmp->EncodeWORD(m_iRefineTableSize);
		tmp->Encode(m_pRefineTable, sizeof(TRefineTable) * m_iRefineTableSize);

		tmp->EncodeWORD(m_vec_itemAttrTable.size());
		tmp->Encode(&m_vec_itemAttrTable[0], sizeof(TItemAttrTable) * m_vec_itemAttrTable.size());

		tmp->EncodeWORD(m_vec_itemRareTable.size());
		tmp->Encode(&m_vec_itemRareTable[0], sizeof(TItemAttrTable) * m_vec_itemRareTable.size());
#endif
	}
}

#ifdef ENABLE_RELOAD_SHOP_COMMAND
void CClientManager::QUERY_RELOAD_SHOP()
{
	if (!InitializeShopTable())
	{
		sys_err("QUERY_RELOAD_SHOP InitializeShopTable FAILED");
		return;
	}

	for (TPeerList::iterator i = m_peerList.begin(); i != m_peerList.end(); ++i)
	{
		CPeer* tmp = *i;

		if (!tmp->GetChannel())
			continue;

		tmp->EncodeHeader(HEADER_DG_RELOAD_SHOP, 0,
			sizeof(WORD) + sizeof(TShopTable) * m_iShopTableSize);

		tmp->EncodeWORD(m_iShopTableSize);
		tmp->Encode(m_pShopTable, sizeof(TShopTable) * m_iShopTableSize);
	}
}
#endif

#ifdef ENABLE_RELOAD_REFINE_COMMAND
void CClientManager::QUERY_RELOAD_REFINE()
{
	if (!InitializeRefineTable())
	{
		sys_err("QUERY_RELOAD_REFINE InitializeRefineTable FAILED");
		return;
	}

	for (TPeerList::iterator i = m_peerList.begin(); i != m_peerList.end(); ++i)
	{
		CPeer* tmp = *i;

		if (!tmp->GetChannel())
			continue;

		tmp->EncodeHeader(HEADER_DG_RELOAD_REFINE, 0,
			sizeof(WORD) + sizeof(TRefineTable) * m_iRefineTableSize);

		tmp->EncodeWORD(m_iRefineTableSize);
		tmp->Encode(m_pRefineTable, sizeof(TRefineTable) * m_iRefineTableSize);
	}
}
#endif
void CClientManager::AddEmpirePriv(TPacketGiveEmpirePriv* p)
{
	CPrivManager::instance().AddEmpirePriv(p->empire, p->type, p->value, p->duration_sec);
#ifdef __DEFAULT_PRIV__
	__UpdateDefaultPriv("EMPIRE", p->empire, p->type, p->value, p->duration_sec);
#endif
}

CLoginData* CClientManager::GetLoginData(DWORD dwKey)
{
	TLoginDataByLoginKey::iterator it = m_map_pkLoginData.find(dwKey);

	if (it == m_map_pkLoginData.end())
		return NULL;

	return it->second;
}

CLoginData* CClientManager::GetLoginDataByLogin(const char* c_pszLogin)
{
	char szLogin[LOGIN_MAX_LEN + 1];
	trim_and_lower(c_pszLogin, szLogin, sizeof(szLogin));

	TLoginDataByLogin::iterator it = m_map_pkLoginDataByLogin.find(szLogin);

	if (it == m_map_pkLoginDataByLogin.end())
		return NULL;

	return it->second;
}

CLoginData* CClientManager::GetLoginDataByAID(DWORD dwAID)
{
	TLoginDataByAID::iterator it = m_map_pkLoginDataByAID.find(dwAID);

	if (it == m_map_pkLoginDataByAID.end())
		return NULL;

	return it->second;
}

void CClientManager::InsertLoginData(CLoginData* pkLD)
{
	char szLogin[LOGIN_MAX_LEN + 1];
	trim_and_lower(pkLD->GetAccountRef().login, szLogin, sizeof(szLogin));

	m_map_pkLoginData.insert(std::make_pair(pkLD->GetKey(), pkLD));
	m_map_pkLoginDataByLogin.insert(std::make_pair(szLogin, pkLD));
	m_map_pkLoginDataByAID.insert(std::make_pair(pkLD->GetAccountRef().id, pkLD));
}

void CClientManager::DeleteLoginData(CLoginData* pkLD)
{
	m_map_pkLoginData.erase(pkLD->GetKey());
	m_map_pkLoginDataByLogin.erase(pkLD->GetAccountRef().login);
	m_map_pkLoginDataByAID.erase(pkLD->GetAccountRef().id);
	if (m_map_kLogonAccount.find(pkLD->GetAccountRef().login) == m_map_kLogonAccount.end())
		delete pkLD;
	else
		pkLD->SetDeleted(true);
}

void CClientManager::QUERY_AUTH_LOGIN(CPeer* pkPeer, DWORD dwHandle, TPacketGDAuthLogin* p)
{
	if (g_test_server)
		sys_log(0, "QUERY_AUTH_LOGIN %d %d %s", p->dwID, p->dwLoginKey, p->szLogin);
	CLoginData* pkLD = GetLoginDataByLogin(p->szLogin);

	if (pkLD)
	{
		DeleteLoginData(pkLD);
	}

	BYTE bResult;

	if (GetLoginData(p->dwLoginKey))
	{
		sys_err("LoginData already exist key %u login %s", p->dwLoginKey, p->szLogin);
		bResult = 0;

		pkPeer->EncodeHeader(HEADER_DG_AUTH_LOGIN, dwHandle, sizeof(BYTE));
		pkPeer->EncodeBYTE(bResult);
	}
	else
	{
		CLoginData* pkLD = new CLoginData;

		pkLD->SetKey(p->dwLoginKey);
		pkLD->SetClientKey(p->adwClientKey);
		pkLD->SetPremium(p->iPremiumTimes);

		TAccountTable& r = pkLD->GetAccountRef();

		r.id = p->dwID;
		trim_and_lower(p->szLogin, r.login, sizeof(r.login));
		strlcpy(r.social_id, p->szSocialID, sizeof(r.social_id));
		strlcpy(r.passwd, "TEMP", sizeof(r.passwd));
#ifdef __HWID_SECURITY_UTILITY__
		strlcpy(r.cpu_id, p->cpu_id, sizeof(r.cpu_id));
		strlcpy(r.hdd_model, p->hdd_model, sizeof(r.hdd_model));
		strlcpy(r.machine_guid, p->machine_guid, sizeof(r.machine_guid));
		strlcpy(r.mac_addr, p->mac_addr, sizeof(r.mac_addr));
		strlcpy(r.hdd_serial, p->hdd_serial, sizeof(r.hdd_serial));
		strlcpy(r.bios_id, p->bios_id, sizeof(r.bios_id));
		strlcpy(r.pc_name, p->pc_name, sizeof(r.pc_name));
		strlcpy(r.user_name, p->user_name, sizeof(r.user_name));
		strlcpy(r.os_version, p->os_version, sizeof(r.os_version));
#endif // __HWID_SECURITY_UTILITY__

		sys_log(0, "AUTH_LOGIN id(%u) login(%s) social_id(%s) login_key(%u), client_key(%u %u %u %u)",
			p->dwID, p->szLogin, p->szSocialID, p->dwLoginKey,
			p->adwClientKey[0], p->adwClientKey[1], p->adwClientKey[2], p->adwClientKey[3]);

		bResult = 1;

		InsertLoginData(pkLD);

		pkPeer->EncodeHeader(HEADER_DG_AUTH_LOGIN, dwHandle, sizeof(BYTE));
		pkPeer->EncodeBYTE(bResult);
	}
}

void CPacketInfo::Add(int header)
{
	itertype(m_map_info) it = m_map_info.find(header);

	if (it == m_map_info.end())
		m_map_info.insert(std::map<int, int>::value_type(header, 1));
	else
		++it->second;
}

void CPacketInfo::Reset()
{
	m_map_info.clear();
}

void CClientManager::ProcessPackets(CPeer* peer)
{
	BYTE		header;
	DWORD		dwHandle;
	DWORD		dwLength;
	const char* data = NULL;
	int			i = 0;
	int			iCount = 0;

	while (peer->PeekPacket(i, header, dwHandle, dwLength, &data))
	{
		// DISABLE_DB_HEADER_LOG
		// sys_log(0, "header %d %p size %d", header, this, dwLength);
		// END_OF_DISABLE_DB_HEADER_LOG
		m_bLastHeader = header;
		++iCount;

#ifdef _TEST
		if (header != 10)
			sys_log(0, " ProcessPacket Header [%d] Handle[%d] Length[%d] iCount[%d]", header, dwHandle, dwLength, iCount);
#endif
		if (g_test_server)
		{
			if (header != 10)
				sys_log(0, " ProcessPacket Header [%d] Handle[%d] Length[%d] iCount[%d]", header, dwHandle, dwLength, iCount);
		}

		switch (header)
		{
		case HEADER_GD_BOOT:
			QUERY_BOOT(peer, (TPacketGDBoot*)data);
			break;

		case HEADER_GD_LOGIN_BY_KEY:
			QUERY_LOGIN_BY_KEY(peer, dwHandle, (TPacketGDLoginByKey*)data);
			break;

		case HEADER_GD_LOGOUT:
			//sys_log(0, "HEADER_GD_LOGOUT (handle: %d length: %d)", dwHandle, dwLength);
			QUERY_LOGOUT(peer, dwHandle, data);
			break;

		case HEADER_GD_PLAYER_LOAD:
			sys_log(1, "HEADER_GD_PLAYER_LOAD (handle: %d length: %d)", dwHandle, dwLength);
			QUERY_PLAYER_LOAD(peer, dwHandle, (TPlayerLoadPacket*)data);
#ifdef __SKILL_COLOR__
			QUERY_SKILL_COLOR_LOAD(peer, dwHandle, (TPlayerLoadPacket*)data);
#endif
			break;

		case HEADER_GD_PLAYER_SAVE:
			sys_log(1, "HEADER_GD_PLAYER_SAVE (handle: %d length: %d)", dwHandle, dwLength);
			QUERY_PLAYER_SAVE(peer, dwHandle, (TPlayerTable*)data);
			break;

		case HEADER_GD_PLAYER_CREATE:
			sys_log(0, "HEADER_GD_PLAYER_CREATE (handle: %d length: %d)", dwHandle, dwLength);
			__QUERY_PLAYER_CREATE(peer, dwHandle, (TPlayerCreatePacket*)data);
			sys_log(0, "END");
			break;

		case HEADER_GD_PLAYER_DELETE:
			sys_log(1, "HEADER_GD_PLAYER_DELETE (handle: %d length: %d)", dwHandle, dwLength);
			__QUERY_PLAYER_DELETE(peer, dwHandle, (TPlayerDeletePacket*)data);
			break;

		case HEADER_GD_PLAYER_COUNT:
			QUERY_PLAYER_COUNT(peer, (TPlayerCountPacket*)data);
			break;

		case HEADER_GD_QUEST_SAVE:
			QUERY_QUEST_SAVE(peer, data, dwLength);
			break;

		case HEADER_GD_SAFEBOX_LOAD:
			QUERY_SAFEBOX_LOAD(peer, dwHandle, (TSafeboxLoadPacket*)data, 0);
			break;

		case HEADER_GD_SAFEBOX_SAVE:
			sys_log(1, "HEADER_GD_SAFEBOX_SAVE (handle: %d length: %d)", dwHandle, dwLength);
			QUERY_SAFEBOX_SAVE(peer, (TSafeboxTable*)data);
			break;

		case HEADER_GD_SAFEBOX_CHANGE_SIZE:
			QUERY_SAFEBOX_CHANGE_SIZE(peer, dwHandle, (TSafeboxChangeSizePacket*)data);
			break;

		case HEADER_GD_SAFEBOX_CHANGE_PASSWORD:
			QUERY_SAFEBOX_CHANGE_PASSWORD(peer, dwHandle, (TSafeboxChangePasswordPacket*)data);
			break;

		case HEADER_GD_MALL_LOAD:
			QUERY_SAFEBOX_LOAD(peer, dwHandle, (TSafeboxLoadPacket*)data, 1);
			break;

		case HEADER_GD_EMPIRE_SELECT:
			QUERY_EMPIRE_SELECT(peer, dwHandle, (TEmpireSelectPacket*)data);
			break;

		case HEADER_GD_SETUP:
			QUERY_SETUP(peer, dwHandle, data);
			break;

		case HEADER_GD_GUILD_CREATE:
			GuildCreate(peer, *(DWORD*)data);
			break;

		case HEADER_GD_GUILD_SKILL_UPDATE:
			GuildSkillUpdate(peer, (TPacketGuildSkillUpdate*)data);
			break;

		case HEADER_GD_GUILD_EXP_UPDATE:
			GuildExpUpdate(peer, (TPacketGuildExpUpdate*)data);
			break;

		case HEADER_GD_GUILD_ADD_MEMBER:
			GuildAddMember(peer, (TPacketGDGuildAddMember*)data);
			break;

		case HEADER_GD_GUILD_REMOVE_MEMBER:
			GuildRemoveMember(peer, (TPacketGuild*)data);
			break;

		case HEADER_GD_GUILD_CHANGE_GRADE:
			GuildChangeGrade(peer, (TPacketGuild*)data);
			break;

		case HEADER_GD_GUILD_CHANGE_MEMBER_DATA:
			GuildChangeMemberData(peer, (TPacketGuildChangeMemberData*)data);
			break;

		case HEADER_GD_GUILD_DISBAND:
			GuildDisband(peer, (TPacketGuild*)data);
			break;

		case HEADER_GD_GUILD_WAR:
			GuildWar(peer, (TPacketGuildWar*)data);
			break;

		case HEADER_GD_GUILD_WAR_SCORE:
			GuildWarScore(peer, (TPacketGuildWarScore*)data);
			break;

		case HEADER_GD_GUILD_CHANGE_LADDER_POINT:
			GuildChangeLadderPoint((TPacketGuildLadderPoint*)data);
			break;

		case HEADER_GD_GUILD_USE_SKILL:
			GuildUseSkill((TPacketGuildUseSkill*)data);
			break;

		case HEADER_GD_FLUSH_CACHE:
			QUERY_FLUSH_CACHE(peer, data);
			break;

		case HEADER_GD_ITEM_SAVE:
			QUERY_ITEM_SAVE(peer, data);
			break;

		case HEADER_GD_ITEM_DESTROY:
			QUERY_ITEM_DESTROY(peer, data);
			break;

		case HEADER_GD_ITEM_FLUSH:
			QUERY_ITEM_FLUSH(peer, data);
			break;

		case HEADER_GD_SAVE_AFFECT:
			QUERY_AFFECT_SAVE(peer, data, dwLength);
			break;

		case HEADER_GD_ADD_AFFECT:
			QUERY_AFFECT_ADD(peer, data, dwLength);
			break;

		case HEADER_GD_CHANGE_AFFECT:
			QUERY_AFFECT_CHANGE(peer, data, dwLength);
			break;

		case HEADER_GD_REMOVE_AFFECT:
			QUERY_AFFECT_REMOVE(peer, data, dwLength);
			break;

		case HEADER_GD_PARTY_CREATE:
			QUERY_PARTY_CREATE(peer, (TPacketPartyCreate*)data);
			break;

		case HEADER_GD_PARTY_DELETE:
			QUERY_PARTY_DELETE(peer, (TPacketPartyDelete*)data);
			break;

		case HEADER_GD_PARTY_ADD:
			QUERY_PARTY_ADD(peer, (TPacketPartyAdd*)data);
			break;

		case HEADER_GD_PARTY_REMOVE:
			QUERY_PARTY_REMOVE(peer, (TPacketPartyRemove*)data);
			break;

		case HEADER_GD_PARTY_STATE_CHANGE:
			QUERY_PARTY_STATE_CHANGE(peer, (TPacketPartyStateChange*)data);
			break;

		case HEADER_GD_PARTY_SET_MEMBER_LEVEL:
			QUERY_PARTY_SET_MEMBER_LEVEL(peer, (TPacketPartySetMemberLevel*)data);
			break;

		case HEADER_GD_RELOAD_PROTO:
			QUERY_RELOAD_PROTO();
			break;

#ifdef ENABLE_RELOAD_SHOP_COMMAND
		case HEADER_GD_RELOAD_SHOP:
			QUERY_RELOAD_SHOP();
			break;
#endif

#ifdef ENABLE_RELOAD_REFINE_COMMAND
		case HEADER_GD_RELOAD_REFINE:
			QUERY_RELOAD_REFINE();
			break;
#endif
		case HEADER_GD_CHANGE_NAME:
			QUERY_CHANGE_NAME(peer, dwHandle, (TPacketGDChangeName*)data);
			break;

		case HEADER_GD_AUTH_LOGIN:
			QUERY_AUTH_LOGIN(peer, dwHandle, (TPacketGDAuthLogin*)data);
			break;

		case HEADER_GD_REQUEST_EMPIRE_PRIV:
			AddEmpirePriv((TPacketGiveEmpirePriv*)data);
			break;

		case HEADER_GD_SET_EVENT_FLAG:
			SetEventFlag((TPacketSetEventFlag*)data);
			break;

			//RELOAD_ADMIN
		case HEADER_GD_RELOAD_ADMIN:
			ReloadAdmin(peer, (TPacketReloadAdmin*)data);
			break;
			//END_RELOAD_ADMIN

		case HEADER_GD_REQ_SPARE_ITEM_ID_RANGE:
			SendSpareItemIDRange(peer);
			break;

		case HEADER_GD_REQ_CHANGE_GUILD_MASTER:
			GuildChangeMaster((TPacketChangeGuildMaster*)data);
			break;

		case HEADER_GD_UPDATE_HORSE_NAME:
			UpdateHorseName((TPacketUpdateHorseName*)data, peer);
			break;

		case HEADER_GD_REQ_HORSE_NAME:
			AckHorseName(*(DWORD*)data, peer);
			break;

		case HEADER_GD_VALID_LOGOUT:
			ResetLastPlayerID((TPacketNeedLoginLogInfo*)data);
			break;

		case HEADER_GD_REQUEST_CHARGE_CASH:
			ChargeCash((TRequestChargeCash*)data);
			break;

		case HEADER_GD_UPDATE_CHANNELSTATUS:
			UpdateChannelStatus((SChannelStatus*)data);
			break;
		case HEADER_GD_REQUEST_CHANNELSTATUS:
			RequestChannelStatus(peer, dwHandle);
			break;
#ifdef __OFFLINE_SHOP__
		case HEADER_GD_NEW_OFFLINESHOP:
			RecvOfflineShopPacket(peer, data);
			break;
#endif
#ifdef __CHANNEL_CHANGER__
		case HEADER_GD_FIND_CHANNEL:
			FindChannel(peer, dwHandle, (TPacketChangeChannel*)data);
			break;
#endif

#ifdef __SKILL_COLOR__
		case HEADER_GD_SKILL_COLOR_SAVE:
			QUERY_SKILL_COLOR_SAVE(data);
			break;
#endif

#ifdef __BATTLE_PASS_SYSTEM__
		case HEADER_GD_SAVE_BATTLE_PASS:
			QUERY_SAVE_BATTLE_PASS(peer, data, dwLength);
			break;

		case HEADER_GD_ADD_BATTLE_PASS:
			QUERY_BATTLE_PASS_ADD(peer, data, dwLength);
			break;
	
		case HEADER_GD_REGISTER_BP_RANKING:
			QUERY_REGISTER_RANKING(peer, dwHandle, (TBattlePassRegisterRanking*)data);
			break;

		case HEADER_GD_BATTLE_PASS_RANKING:
			RequestLoadBattlePassRanking(peer, dwHandle, data);
			break;
#endif

		default:
			sys_err("Unknown header (header: %d handle: %d length: %d)", header, dwHandle, dwLength);
			break;
		}
	}

	peer->RecvEnd(i);
}

void CClientManager::AddPeer(socket_t fd)
{
	CPeer* pPeer = new CPeer;

	if (pPeer->Accept(fd))
		m_peerList.push_front(pPeer);
	else
		delete pPeer;
}

void CClientManager::RemovePeer(CPeer* pPeer)
{
	if (m_pkAuthPeer == pPeer)
	{
		m_pkAuthPeer = NULL;
	}
	else
	{
		TLogonAccountMap::iterator it = m_map_kLogonAccount.begin();
		while (it != m_map_kLogonAccount.end())
		{
			CLoginData* pkLD = it->second;

			if (pkLD->GetConnectedPeerHandle() == pPeer->GetHandle())
			{
				if (pkLD->IsPlay())
				{
					pkLD->SetPlay(false);
				}

				if (pkLD->IsDeleted())
				{
					sys_log(0, "DELETING LoginData");
					delete pkLD;
				}

				m_map_kLogonAccount.erase(it++);
			}
			else
				++it;
		}
	}

	m_peerList.remove(pPeer);
	delete pPeer;
}

CPeer* CClientManager::GetPeer(IDENT ident)
{
	for (itertype(m_peerList) i = m_peerList.begin(); i != m_peerList.end(); ++i)
	{
		CPeer* tmp = *i;

		if (tmp->GetHandle() == ident)
			return tmp;
	}

	return NULL;
}

CPeer* CClientManager::GetAnyPeer()
{
	if (m_peerList.empty())
		return NULL;

	return m_peerList.front();
}

//
int CClientManager::AnalyzeQueryResult(SQLMsg* msg)
{
	CQueryInfo* qi = (CQueryInfo*)msg->pvUserData;
	CPeer* peer = GetPeer(qi->dwIdent);

	switch (qi->iType)
	{
	case QID_GUILD_RANKING:
		CGuildManager::instance().ResultRanking(msg->Get()->pSQLResult);
		break;
	}

	if (!peer)
	{
#ifdef __OFFLINE_SHOP__
		switch (qi->iType)
		{
		case QID_OFFLINESHOP_ADD_ITEM:
		case QID_OFFLINESHOP_CREATE_SHOP:
		case QID_OFFLINESHOP_CREATE_SHOP_ADD_ITEM:
		case QID_OFFLINESHOP_SAFEBOX_ADD_ITEM:
			OfflineShopResultQuery(peer, msg, qi);
			break;

		default: break;
		}
#endif
		//sys_err("CClientManager::AnalyzeQueryResult: peer not exist anymore. (ident: %d)", qi->dwIdent);
		delete qi;
		return true;
	}

	switch (qi->iType)
	{
	case QID_PLAYER:
	case QID_ITEM:
	case QID_QUEST:
	case QID_AFFECT:
#ifdef __SKILL_COLOR__
	case QID_SKILL_COLOR:
#endif
#ifdef __BATTLE_PASS_SYSTEM__
	case QID_BATTLE_PASS:
#endif
		RESULT_COMPOSITE_PLAYER(peer, msg, qi->iType);
		break;

	case QID_LOGIN:
		RESULT_LOGIN(peer, msg);
		break;

	case QID_SAFEBOX_LOAD:
		sys_log(0, "QUERY_RESULT: HEADER_GD_SAFEBOX_LOAD");
		RESULT_SAFEBOX_LOAD(peer, msg);
		break;

	case QID_SAFEBOX_CHANGE_SIZE:
		sys_log(0, "QUERY_RESULT: HEADER_GD_SAFEBOX_CHANGE_SIZE");
		RESULT_SAFEBOX_CHANGE_SIZE(peer, msg);
		break;

	case QID_SAFEBOX_CHANGE_PASSWORD:
		sys_log(0, "QUERY_RESULT: HEADER_GD_SAFEBOX_CHANGE_PASSWORD %p", msg);
		RESULT_SAFEBOX_CHANGE_PASSWORD(peer, msg);
		break;

	case QID_SAFEBOX_CHANGE_PASSWORD_SECOND:
		sys_log(0, "QUERY_RESULT: HEADER_GD_SAFEBOX_CHANGE_PASSWORD %p", msg);
		RESULT_SAFEBOX_CHANGE_PASSWORD_SECOND(peer, msg);
		break;

		// PLAYER_INDEX_CREATE_BUG_FIX
	case QID_PLAYER_INDEX_CREATE:
		RESULT_PLAYER_INDEX_CREATE(peer, msg);
		break;
		// END_PLAYER_INDEX_CREATE_BUG_FIX

	case QID_PLAYER_DELETE:
		__RESULT_PLAYER_DELETE(peer, msg);
		break;

	case QID_LOGIN_BY_KEY:
		RESULT_LOGIN_BY_KEY(peer, msg);
		break;

#ifdef __OFFLINE_SHOP__
	case QID_OFFLINESHOP_ADD_ITEM:
	case QID_OFFLINESHOP_CREATE_SHOP:
	case QID_OFFLINESHOP_CREATE_SHOP_ADD_ITEM:
	case QID_OFFLINESHOP_SAFEBOX_ADD_ITEM:
		OfflineShopResultQuery(peer, msg, qi);
		break;

#endif

	default:
		sys_log(0, "CClientManager::AnalyzeQueryResult unknown query result type: %d, str: %s", qi->iType, msg->stQuery.c_str());
		break;
	}

	delete qi;
	return true;
}

extern DWORD OFFSHOP_NEW_COUNTER[3];
int CClientManager::Process()
{
	int pulses;

	if (!(pulses = thecore_idle()))
		return 0;

	while (pulses--)
	{
		++thecore_heart->pulse;

		if (!(thecore_heart->pulse % thecore_heart->passes_per_sec))
		{
			if ((thecore_heart->pulse % (thecore_heart->passes_per_sec * 60)) == thecore_heart->passes_per_sec * 30)
			{
				sys_log(0, "[%9d] return %d/%d/%d async %d/%d/%d",
					thecore_heart->pulse,
					CDBManager::instance().CountReturnQuery(SQL_PLAYER),
					CDBManager::instance().CountReturnResult(SQL_PLAYER),
					CDBManager::instance().CountReturnQueryFinished(SQL_PLAYER),
					CDBManager::instance().CountAsyncQuery(SQL_PLAYER),
					CDBManager::instance().CountAsyncResult(SQL_PLAYER),
					CDBManager::instance().CountAsyncQueryFinished(SQL_PLAYER));
			}

			CDBManager::instance().ResetCounter();


			memset(&thecore_profiler[0], 0, sizeof(thecore_profiler));

			m_iCacheFlushCount = 0;

			UpdatePlayerCache();

			UpdateItemCache();
			UpdateLogoutPlayer();

			CGuildManager::instance().Update();
			CPrivManager::instance().Update();
		}
#ifdef __OFFLINE_SHOP__
		if ((thecore_heart->pulse % (thecore_heart->passes_per_sec * 60)) == thecore_heart->passes_per_sec * 30) //to be async relative to the money log
			OfflineshopDurationProcess();
#endif

		if (!(thecore_heart->pulse % (thecore_heart->passes_per_sec * 30)))
		{
#ifdef __SKILL_COLOR__
			// Skill color fush
			UpdateSkillColorCache();
#endif
		}
#ifdef __BATTLE_PASS_SYSTEM__
		if (!(thecore_heart->pulse % (thecore_heart->passes_per_sec * 3600))) // Refresh ranking cache every hour
		{
			CClientManager::instance().LoadBattlePassRanking();
		}
#endif

		if (!(thecore_heart->pulse % (thecore_heart->passes_per_sec * 60)))
			CClientManager::instance().SendTime();
	}

	int num_events = fdwatch(m_fdWatcher, 0);
	int idx;
	CPeer* peer;

	for (idx = 0; idx < num_events; ++idx)
	{
		peer = (CPeer*)fdwatch_get_client_data(m_fdWatcher, idx);

		if (!peer)
		{
			if (fdwatch_check_event(m_fdWatcher, m_fdAccept, idx) == FDW_READ)
			{
				AddPeer(m_fdAccept);
				fdwatch_clear_event(m_fdWatcher, m_fdAccept, idx);
			}
			else
			{
				sys_log(0, "FDWATCH: peer null in event: ident %d", fdwatch_get_ident(m_fdWatcher, idx)); // @warme012
			}

			continue;
		}

		switch (fdwatch_check_event(m_fdWatcher, peer->GetFd(), idx))
		{
		case FDW_READ:
			if (peer->Recv() < 0)
			{
				sys_err("Recv failed");
				RemovePeer(peer);
			}
			else
			{
				if (g_log && peer == m_pkAuthPeer)
					sys_log(0, "AUTH_PEER_READ: size %d", peer->GetRecvLength());

				ProcessPackets(peer);
			}
			break;

		case FDW_WRITE:
			if (peer == m_pkAuthPeer)
				if (g_log && peer->GetSendLength() > 0)
					sys_log(0, "AUTH_PEER_WRITE: size %d", peer->GetSendLength());

			if (peer->Send() < 0)
			{
				sys_err("Send failed");
				RemovePeer(peer);
			}

			break;

		case FDW_EOF:
			RemovePeer(peer);
			break;

		default:
			sys_err("fdwatch_check_fd returned unknown result");
			RemovePeer(peer);
			break;
		}
	}

#ifdef __WIN32__
	if (_kbhit()) {
		int c = _getch();
		switch (c) {
		case 0x1b: // Esc
			return 0; // shutdown
			break;
		default:
			break;
		}
	}
#endif

	return 1;
}

DWORD CClientManager::GetUserCount()
{
	return m_map_kLogonAccount.size();
}

void CClientManager::SendAllGuildSkillRechargePacket()
{
	ForwardPacket(HEADER_DG_GUILD_SKILL_RECHARGE, NULL, 0);
}

void CClientManager::SendTime()
{
	time_t now = GetCurrentTime();
	ForwardPacket(HEADER_DG_TIME, &now, sizeof(time_t));
}

void CClientManager::ForwardPacket(BYTE header, const void* data, int size, BYTE bChannel, CPeer* except)
{
	for (itertype(m_peerList) it = m_peerList.begin(); it != m_peerList.end(); ++it)
	{
		CPeer* peer = *it;

		if (peer == except)
			continue;

		if (!peer->GetChannel())
			continue;

		if (bChannel && peer->GetChannel() != bChannel)
			continue;

		peer->EncodeHeader(header, 0, size);

		if (size > 0 && data)
			peer->Encode(data, size);
	}
}

void CClientManager::SendNotice(const char* c_pszFormat, ...)
{
	char szBuf[255 + 1];
	va_list args;

	va_start(args, c_pszFormat);
	int len = vsnprintf(szBuf, sizeof(szBuf), c_pszFormat, args);
	va_end(args);
	szBuf[len] = '\0';

	ForwardPacket(HEADER_DG_NOTICE, szBuf, len + 1);
}

time_t CClientManager::GetCurrentTime()
{
	return time(0);
}

//BOOT_LOCALIZATION

bool CClientManager::InitializeLocalization()
{
	char szQuery[512];
	snprintf(szQuery, sizeof(szQuery), "SELECT mValue, mKey FROM locale");
	std::unique_ptr<SQLMsg> pMsg(CDBManager::instance().DirectQuery(szQuery, SQL_COMMON));

	if (pMsg->Get()->uiNumRows == 0)
	{
		sys_err("InitializeLocalization() ==> DirectQuery failed(%s)", szQuery);
		return false;
	}

	sys_log(0, "InitializeLocalization() - LoadLocaleTable(count:%d)", pMsg->Get()->uiNumRows);

	m_vec_Locale.clear();

	MYSQL_ROW row = NULL;

	for (int n = 0; (row = mysql_fetch_row(pMsg->Get()->pSQLResult)) != NULL; ++n)
	{
		int col = 0;
		tLocale locale;

		strlcpy(locale.szValue, row[col++], sizeof(locale.szValue));
		strlcpy(locale.szKey, row[col++], sizeof(locale.szKey));

		//DB_NAME_COLUMN Setting
		if (strcmp(locale.szKey, "LOCALE") == 0)
		{
			if (strcmp(locale.szValue, "turkey") == 0)
			{
				sys_log(0, "locale[LOCALE] = %s", locale.szValue);

				if (g_stLocale != locale.szValue)
					sys_log(0, "Changed g_stLocale %s to %s", g_stLocale.c_str(), "latin5");

				g_stLocale = "latin5";
				g_stLocaleNameColumn = "locale_name";
			}
			else
			{
				sys_err("locale[LOCALE] = UNKNOWN(%s)", locale.szValue);
				exit(0);
			}
		}
		else if (strcmp(locale.szKey, "DB_NAME_COLUMN") == 0)
		{
			sys_log(0, "locale[DB_NAME_COLUMN] = %s", locale.szValue);
			g_stLocaleNameColumn = locale.szValue;
		}
		else
		{
			sys_log(0, "locale[UNKNOWN_KEY(%s)] = %s", locale.szKey, locale.szValue);
		}
		m_vec_Locale.push_back(locale);
	}

	return true;
}
//END_BOOT_LOCALIZATION
//ADMIN_MANAGER

bool CClientManager::__GetAdminInfo(const char* szIP, std::vector<tAdminInfo>& rAdminVec)
{
	char szQuery[512];
	snprintf(szQuery, sizeof(szQuery),
		"SELECT mID,mAccount,mName,mContactIP,mServerIP,mAuthority FROM gmlist WHERE mServerIP='ALL' or mServerIP='%s'",
		szIP ? szIP : "ALL");

	std::unique_ptr<SQLMsg> pMsg(CDBManager::instance().DirectQuery(szQuery, SQL_COMMON));

	if (pMsg->Get()->uiNumRows == 0)
	{
		// sys_err("__GetAdminInfo() ==> DirectQuery failed(%s)", szQuery); // @warme013
		return false;
	}

	MYSQL_ROW row;
	rAdminVec.reserve(pMsg->Get()->uiNumRows);

	while ((row = mysql_fetch_row(pMsg->Get()->pSQLResult)))
	{
		int idx = 0;
		tAdminInfo Info;

		str_to_number(Info.m_ID, row[idx++]);
		trim_and_lower(row[idx++], Info.m_szAccount, sizeof(Info.m_szAccount));
		strlcpy(Info.m_szName, row[idx++], sizeof(Info.m_szName));
		strlcpy(Info.m_szContactIP, row[idx++], sizeof(Info.m_szContactIP));
		strlcpy(Info.m_szServerIP, row[idx++], sizeof(Info.m_szServerIP));
		std::string stAuth = row[idx++];

#ifdef __GAME_MASTER_UTILITY__
		if (!stAuth.compare("IMPLEMENTOR"))
			Info.m_Authority = GM_IMPLEMENTOR;
		else if (!stAuth.compare("GAME_ADMIN"))
			Info.m_Authority = GM_DEFAULT;
		else if (!stAuth.compare("GAME_MASTER"))
			Info.m_Authority = GM_DEFAULT;
		else if (!stAuth.compare("DISCORD_ADMIN"))
			Info.m_Authority = GM_DCADMIN;
		else if (!stAuth.compare("SUPER_MODERATOR"))
			Info.m_Authority = GM_SMOD;
		else if (!stAuth.compare("MODERATOR"))
			Info.m_Authority = GM_MOD;
		else if (!stAuth.compare("HUNTER"))
			Info.m_Authority = GM_HUNTER;
		else
			continue;
#else
		if (!stAuth.compare("IMPLEMENTOR"))
			Info.m_Authority = GM_IMPLEMENTOR;
		else if (!stAuth.compare("GOD"))
			Info.m_Authority = GM_GOD;
		else if (!stAuth.compare("HIGH_WIZARD"))
			Info.m_Authority = GM_HIGH_WIZARD;
		else if (!stAuth.compare("LOW_WIZARD"))
			Info.m_Authority = GM_LOW_WIZARD;
		else if (!stAuth.compare("WIZARD"))
			Info.m_Authority = GM_WIZARD;
		else
			continue;
#endif

		rAdminVec.push_back(Info);

		sys_log(0, "GM: PID %u Login %s Character %s ContactIP %s ServerIP %s Authority %d[%s]",
			Info.m_ID, Info.m_szAccount, Info.m_szName, Info.m_szContactIP, Info.m_szServerIP, Info.m_Authority, stAuth.c_str());
	}

	return true;
}
//END_ADMIN_MANAGER

void CClientManager::ReloadAdmin(CPeer*, TPacketReloadAdmin* p)
{
	std::vector<tAdminInfo> vAdmin;
	__GetAdminInfo(p->szIP, vAdmin);

	DWORD dwPacketSize = sizeof(WORD) + sizeof(WORD) + sizeof(tAdminInfo) * vAdmin.size();

	for (itertype(m_peerList) it = m_peerList.begin(); it != m_peerList.end(); ++it)
	{
		CPeer * peer = *it;

		if (!peer->GetChannel())
			continue;

		peer->EncodeHeader(HEADER_DG_RELOAD_ADMIN, 0, dwPacketSize);

		peer->EncodeWORD(sizeof(tAdminInfo));
		peer->EncodeWORD(vAdmin.size());

		for (size_t n = 0; n < vAdmin.size(); ++n)
			peer->Encode(&vAdmin[n], sizeof(tAdminInfo));
	}

	sys_log(0, "ReloadAdmin End %s", p->szIP);
}

void CClientManager::UpdateItemCacheSet(DWORD pid)
{
	itertype(m_map_pkItemCacheSetPtr) it = m_map_pkItemCacheSetPtr.find(pid);

	if (it == m_map_pkItemCacheSetPtr.end())
	{
		if (g_test_server)
			sys_log(0, "UPDATE_ITEMCACHESET : UpdateItemCacheSet ==> No ItemCacheSet pid(%d)", pid);
		return;
	}

	TItemCacheSet* pSet = it->second;
	TItemCacheSet::iterator it_set = pSet->begin();

	while (it_set != pSet->end())
	{
		CItemCache* c = *it_set++;
		c->Flush();
	}

	if (g_log)
		sys_log(0, "UPDATE_ITEMCACHESET : UpdateItemCachsSet pid(%d)", pid);
}

void CClientManager::UpdateQuestCacheSet(DWORD pid)
{
	itertype(m_map_pkQuestCacheSetPtr) it = m_map_pkQuestCacheSetPtr.find(pid);

	if (it == m_map_pkQuestCacheSetPtr.end())
	{
		if (g_test_server)
			sys_log(0, "UPDATE_ITEMCACHESET : UpdateQuestCacheSet ==> No QuestCacheSet pid(%d)", pid);
		return;
	}

	TQuestCacheSet* pSet = it->second;
	TQuestCacheSet::iterator it_set = pSet->begin();

	while (it_set != pSet->end())
	{
		CQuestCache* c = *it_set++;
		c->Flush();
	}

	if (g_log)
		sys_log(0, "UPDATE_ITEMCACHESET : UpdateQuestCachsSet pid(%d)", pid);
}

void CClientManager::SendSpareItemIDRange(CPeer* peer)
{
	peer->SendSpareItemIDRange();
}

void CClientManager::DeleteLoginKey(TPacketDC* data)
{
	char login[LOGIN_MAX_LEN + 1] = { 0 };
	trim_and_lower(data->login, login, sizeof(login));

	CLoginData* pkLD = GetLoginDataByLogin(login);

	if (pkLD)
	{
		TLoginDataByLoginKey::iterator it = m_map_pkLoginData.find(pkLD->GetKey());

		if (it != m_map_pkLoginData.end())
			m_map_pkLoginData.erase(it);
	}
}

void CClientManager::UpdateChannelStatus(TChannelStatus* pData)
{
	TChannelStatusMap::iterator it = m_mChannelStatus.find(pData->nPort);
	if (it != m_mChannelStatus.end()) {
		it->second = pData->bStatus;
	}
	else {
		m_mChannelStatus.insert(TChannelStatusMap::value_type(pData->nPort, pData->bStatus));
	}
}

void CClientManager::RequestChannelStatus(CPeer* peer, DWORD dwHandle)
{
	const int nSize = m_mChannelStatus.size();
	peer->EncodeHeader(HEADER_DG_RESPOND_CHANNELSTATUS, dwHandle, sizeof(TChannelStatus) * nSize + sizeof(int));
	peer->Encode(&nSize, sizeof(int));
	for (TChannelStatusMap::iterator it = m_mChannelStatus.begin(); it != m_mChannelStatus.end(); it++) {
		peer->Encode(&it->first, sizeof(short));
		peer->Encode(&it->second, sizeof(BYTE));
	}
}

void CClientManager::ResetLastPlayerID(const TPacketNeedLoginLogInfo* data)
{
	CLoginData* pkLD = GetLoginDataByAID(data->dwPlayerID);

	if (NULL != pkLD)
	{
		pkLD->SetLastPlayerID(0);
	}
}

void CClientManager::ChargeCash(const TRequestChargeCash* packet)
{
	char szQuery[512];

	if (ERequestCharge_Cash == packet->eChargeType)
		sprintf(szQuery, "update account set cash = cash + %d where id = %d limit 1", packet->dwAmount, packet->dwAID);
	else if (ERequestCharge_Mileage == packet->eChargeType)
		sprintf(szQuery, "update account set mileage = mileage + %d where id = %d limit 1", packet->dwAmount, packet->dwAID);
	else
	{
		sys_err("Invalid request charge type (type : %d, amount : %d, aid : %d)", packet->eChargeType, packet->dwAmount, packet->dwAID);
		return;
	}

	CDBManager::Instance().AsyncQuery(szQuery, SQL_ACCOUNT);
}

#ifdef __CHANNEL_CHANGER__
void CClientManager::FindChannel(CPeer* requestPeer, DWORD dwHandle, TPacketChangeChannel* p)
{
	if (!p->lMapIndex || !p->channel)
		return;
	long lAddr = 0;
	WORD port = 0;
	for (const auto peer : m_peerList)
	{
		if (peer->GetChannel() != p->channel) //not the channel we are looking for!
			continue;
		TMapLocation kMapLocation;
		thecore_memcpy(kMapLocation.alMaps, peer->GetMaps(), sizeof(kMapLocation.alMaps));
		for (const auto midx : kMapLocation.alMaps)
		{
			if (midx == p->lMapIndex)
			{
				char host[16];
				strlcpy(host, peer->GetPublicIP(), sizeof(kMapLocation.szHost));
				lAddr = inet_addr(host);
				port = peer->GetListenPort();
				break;
			}
		}
		if (lAddr && port) //We already obtained them
			break;
	}
	TPacketReturnChannel r;
	r.lAddr = lAddr;
	r.port = port;
	requestPeer->EncodeHeader(HEADER_DG_CHANNEL_RESULT, dwHandle, sizeof(r));
	requestPeer->Encode(&r, sizeof(r));
}
#endif