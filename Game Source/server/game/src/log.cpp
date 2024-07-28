#include "stdafx.h"
#include "constants.h"
#include "config.h"
#include "log.h"

#include "char.h"
#include "desc.h"
#include "item.h"
#include "db.h"

static char	__escape_hint[1024];

LogManager::LogManager() : m_bIsConnect(false)
{
}

LogManager::~LogManager()
{
}

bool LogManager::Connect(const char* host, const int port, const char* user, const char* pwd, const char* db)
{
	if (m_sql.Setup(host, user, pwd, db, g_stLocale.c_str(), false, port))
		m_bIsConnect = true;

	if (!m_sql_direct.Setup(host, user, pwd, db, g_stLocale.c_str(), true, port))
		m_bIsConnect = false;

	return m_bIsConnect;
}

void LogManager::Query(const char* c_pszFormat, ...)
{
	LOG_LEVEL_CHECK_N_RET(LOG_LEVEL_MIN);

	char szQuery[4096];
	va_list args;

	va_start(args, c_pszFormat);
	vsnprintf(szQuery, sizeof(szQuery), c_pszFormat, args);
	va_end(args);

	if (test_server)
		sys_log(0, "LOG: %s", szQuery);

	// @duzenleme
	// sql injectler icin satir sonuna engel koyuldu.
	std::string sQuery(szQuery);
	m_sql.AsyncQuery(sQuery.substr(0, sQuery.find_first_of(";") == std::string::npos ? sQuery.length() : sQuery.find_first_of(";")).c_str());
}

SQLMsg* LogManager::DirectQuery(const char* c_pszFormat, ...)
{
	char szQuery[4096];
	va_list args;

	va_start(args, c_pszFormat);
	vsnprintf(szQuery, sizeof(szQuery), c_pszFormat, args);
	va_end(args);

	if (test_server)
		sys_log(1, "LOG: %s", szQuery);

	// @duzenleme
	// sql injectler icin satir sonuna engel koyuldu.
	std::string sQuery(szQuery);
	return m_sql_direct.DirectQuery(sQuery.substr(0, sQuery.find_first_of(";") == std::string::npos ? sQuery.length() : sQuery.find_first_of(";")).c_str());
}

bool LogManager::IsConnected()
{
	return m_bIsConnect;
}

size_t LogManager::EscapeString(char* dst, size_t dstSize, const char* src, size_t srcSize)
{
	return m_sql.EscapeString(dst, dstSize, src, srcSize);
}

void LogManager::HackLog(const char* c_pszHackName, const char* c_pszLogin, const char* c_pszName, const char* c_pszIP)
{
	LOG_LEVEL_CHECK_N_RET(LOG_LEVEL_MIN);
	m_sql.EscapeString(__escape_hint, sizeof(__escape_hint), c_pszHackName, strlen(c_pszHackName));

	Query("INSERT INTO `log`.`log_hack` (`time`, `login`, `name`, `ip`, `server`, `why`) VALUES(NOW(), '%s', '%s', '%s', '%s', '%s')", c_pszLogin, c_pszName, c_pszIP, g_stHostname.c_str(), __escape_hint);
}

void LogManager::HackLog(const char* c_pszHackName, LPCHARACTER ch)
{
	LOG_LEVEL_CHECK_N_RET(LOG_LEVEL_MIN);
	if (ch->GetDesc())
	{
		HackLog(c_pszHackName,
			ch->GetDesc()->GetAccountTable().login,
			ch->GetName(),
			ch->GetDesc()->GetHostName());
	}
}

void LogManager::ChangeNameLog(DWORD pid, const char* old_name, const char* new_name, const char* ip)
{
	LOG_LEVEL_CHECK_N_RET(LOG_LEVEL_MIN);
	Query("INSERT INTO `log`.`log_change_name` (`pid`, `old_name`, `new_name`, `time`, `ip`) "
		"VALUES(%u, '%s', '%s', NOW(), '%s') ",
		pid, old_name, new_name, ip);
}

void LogManager::GMCommandLog(DWORD dwPID, const char* szName, const char* szIP, BYTE byChannel, const char* szCommand)
{
	LOG_LEVEL_CHECK_N_RET(LOG_LEVEL_MIN);
	m_sql.EscapeString(__escape_hint, sizeof(__escape_hint), szCommand, strlen(szCommand));

	Query("INSERT INTO `log`.`log_command` (`userid`, `server`, `ip`, `port`, `username`, `command`, `date`) "
		"VALUES(%u, 999, '%s', %u, '%s', '%s', NOW()) ",
		dwPID, szIP, byChannel, szName, __escape_hint);
}

void LogManager::BootLog(const char* c_pszHostName, BYTE bChannel)
{
	LOG_LEVEL_CHECK_N_RET(LOG_LEVEL_MIN);
	Query("INSERT INTO `log`.`log_boot` (`time`, `hostname`, `channel`) VALUES(NOW(), '%s', %d)",
		c_pszHostName, bChannel);
}

#ifdef __MARTY_CHAT_LOGGING__
void LogManager::ChatLog(DWORD where, DWORD who_id, const char* who_name, DWORD whom_id, const char* whom_name, const char* type, const char* msg, const char* ip)
{
	Query("INSERT INTO `log`.`log_chat` (`where`, `who_id`, `who_name`, `whom_id`, `whom_name`, `type`, `msg`, `when`, `ip`) "
		"VALUES (%u, %u, '%s', %u, '%s', '%s', '%s', NOW(), '%s');",
		where, who_id, who_name, whom_id, whom_name, type, msg, ip);
}
#endif

#if defined(__GOLD_LIMIT_REWORK__) && defined(__HWID_SECURITY_UTILITY__) && defined(__CHEQUE_SYSTEM__)
DWORD LogManager::ExchangeLog(int type, DWORD item_count, DWORD dwPID1, const char* playerA, DWORD dwPID2, const char* playerB, long x, long y, long long goldA /*=0*/, long long goldB /*=0*/, DWORD chequeA, DWORD chequeB, const char* playerAHwid, const char* playerBHwid, const char* playerAHDDSerial, const char* playerBHDDSerial)
#else
DWORD LogManager::ExchangeLog(int type, DWORD item_count, DWORD dwPID1, const char* playerA, DWORD dwPID2, const char* playerB, long x, long y, int goldA /*=0*/, int goldB /*=0*/)
#endif
{
	char szQuery[QUERY_MAX_LEN];
#if defined(__GOLD_LIMIT_REWORK__) && defined(__HWID_SECURITY_UTILITY__) && defined(__CHEQUE_SYSTEM__)
	snprintf(szQuery, sizeof(szQuery), "INSERT INTO `log`.`log_exchanges` (`type`, `item_count`, `player_giver_id`, `player_giver_name`, `player_recver_id`, `player_recver_name`, `player_giver_hwid`, `player_recver_hwid`, `player_giver_hdd_serial`, `player_recver_hdd_serial`, `player_giver_gold`, `player_recver_gold`, `player_giver_cheque`, `player_recver_cheque`, `x`, `y`, `date`) VALUES (%d, %u, %u, '%s', %u, '%s', '%s', '%s', '%s', '%s', %lld, %lld, %d, %d, %ld, %ld, NOW())", type, item_count, dwPID1, playerA, dwPID2, playerB, playerAHwid, playerBHwid, playerAHDDSerial, playerBHDDSerial, goldA, goldB, chequeA, chequeB, x, y);
#else
	snprintf(szQuery, sizeof(szQuery), "INSERT INTO `log`.`log_exchanges` (`type`, `item_count`, `player_giver_id`, `player_giver_name`, `player_recver_id`, `player_recver_name`, `player_giver_gold`, `player_recver_gold`, `x`, `y`, `date`) VALUES (%d, %u, %u, `%s`, %u, `%s`, `%s`, `%s`, %d, %d, %ld, %ld, NOW())", type, item_count, dwPID1, playerA, dwPID2, playerB, goldA, goldB, x, y);
#endif
	std::unique_ptr<SQLMsg> msg(DirectQuery(szQuery));

	if (!msg || msg->Get()->uiAffectedRows == 0 || msg->Get()->uiAffectedRows == (DWORD)-1) {
		sys_err("Issue logging trade. Query: %s", szQuery);
		return 0;
	}

	return (DWORD)msg->Get()->uiInsertID;
}

void LogManager::ExchangeItemLog(DWORD tradeID, LPITEM item, const char* player)
{
	if (!item)
		return;

	if (!tradeID)
	{
		sys_err("Lost trade due to mysql error (tradeID = 0)");
		return;
	}

	Query("INSERT INTO `log`.`log_exchange_items`"
		"(trade_id, `toPlayer`, `item_id`, `vnum`, `count`, `socket0`, `socket1`, `socket2`, `socket3`,`date`)"
		"VALUES ("
		"%lu, '%s', %lu, %lu, %lu,"
		" %ld, %ld, %ld, %ld,"
		" NOW())"
		, tradeID, player, item->GetID(), item->GetVnum(), item->GetCount()
		, item->GetSocket(0), item->GetSocket(1), item->GetSocket(2), item->GetSocket(3));
}

#ifdef __INGAME_MALL__
void LogManager::ItemShopLog(LPCHARACTER ch, LPITEM item, std::string logType, long ep)
{
	if (!ch)
		return;

	if (!item)
		return;

	Query("INSERT INTO `log`.`log_item_shop` "
		"(`owner`, `old_item_id`, `vnum`, `count`, `log_type`, "
		"`oyuncu_ad`, `oyuncu_ep`, `oyuncu_mark`, `item_ep`, "
		"`date`) "
		"VALUES ("
		"%lu, %lu, %lu, %lu, '%s', '%s', "
		"%d, %d, %d, "
		"NOW()) "
		, item->GetLastOwnerPID(), item->GetID(), item->GetVnum(), item->GetCount(), logType.c_str()
		, ch->GetName()
		, ch->GetDragonCoin()
		, ch->GetDragonMark()
		, ep
	);
}
#endif

void LogManager::HackLogEx(LPCHARACTER ch, const char* textLine)
{
	if (!ch)
		return;

	LPDESC desc = ch->GetDesc();

	if (!desc)
		return;

	Query("INSERT INTO `log`.`log_hack_ex` (`player`, `player_name`, `account`, `account_name`, `mac_addr`, `bios_id`, `hdd_serial`, `machine_guid`, `textLine`, `time`) VALUES(%u, '%s', %u, '%s', '%s', '%s', '%s', '%s', '%s', NOW())", ch->GetPlayerID(), ch->GetName(), ch->GetAID(), desc->GetAccountTable().login, desc->GetMacAddr(), desc->GetBiosID(), desc->GetHDDSerial(), desc->GetMachineGuid(), textLine);
}

void LogManager::HackLogEx(LPDESC desc, const char* textLine)
{
	if (!desc)
		return;

	DWORD player_id = 0;
	DWORD account_id = 0;

	LPCHARACTER ch = desc->GetCharacter();

	if (ch)
	{
		player_id = ch->GetPlayerID();
		account_id = ch->GetAID();
	}

	Query("INSERT INTO `log`.`log_hack_ex` (`player`, `player_name`, `account`, `account_name`, `mac_addr`, `bios_id`, `hdd_serial`, `machine_guid`, `textLine`, `time`) VALUES(%u, %u, 'Unknown', '%s', 'Unknown', '%s', '%s', '%s', '%s', NOW()", player_id, account_id, desc->GetMacAddr(), desc->GetBiosID(), desc->GetHDDSerial(), desc->GetMachineGuid(), textLine);
}

void LogManager::HackLogEx(std::string stLogin, const char* textLine)
{
	Query("INSERT INTO log.log_hack_auth (account, textLine, time) VALUES('%s', `%s`, NOW())", stLogin.c_str(), textLine);
}

#ifdef __BAN_REASON_UTILITY__
void LogManager::BanLog(LPCHARACTER ch, std::string bannedAccountName, bool isPlayer, std::string banType, DWORD banDuration)
{
	if (!ch)
		return;

	Query("INSERT INTO `log`.`log_ban` (`master`, `master_name`, `banned_name`, `is_player`, `ban_type`, `ban_duration`, `time`) VALUES (%u, '%s', '%s', %u, '%s', %u, NOW())", ch->GetPlayerID(), ch->GetName(), bannedAccountName.c_str(), isPlayer, banType.c_str(), banDuration);
}
#endif

void LogManager::PayToSellLog(LPCHARACTER ch, const char* textLine)
{
	if (!ch)
		return;

	LPDESC desc = ch->GetDesc();

	if (!desc)
		return;

	Query("INSERT INTO `log`.`log_pay_to_sell` (`player`, `player_name`, `account`, `account_name`, `mac_addr`, `bios_id`, `hdd_serial`, `machine_guid`, `textLine`, `time`) VALUES(%u, '%s', %u, '%s', '%s', '%s', '%s', '%s', '%s', NOW())", ch->GetPlayerID(), ch->GetName(), ch->GetAID(), desc->GetAccountTable().login, desc->GetMacAddr(), desc->GetBiosID(), desc->GetHDDSerial(), desc->GetMachineGuid(), textLine);
}

void LogManager::DetailLoginLog(bool isLogin, LPCHARACTER ch)
{
/*	LOG_LEVEL_CHECK_N_RET(LOG_LEVEL_MID);
	if (NULL == ch->GetDesc())
		return;

	if (true == isLogin)
	{
		Query("INSERT INTO loginlog2(type, is_gm, login_time, channel, account_id, pid, ip, client_version) "
				"VALUES('INVALID', %s, NOW(), %d, %u, %u, inet_aton('%s'), '%s')",
				ch->IsGM() == true ? "'Y'" : "'N'",
				g_bChannel,
				ch->GetDesc()->GetAccountTable().id,
				ch->GetPlayerID(),
				ch->GetDesc()->GetHostName(),
				ch->GetDesc()->GetClientVersion());
	}
	else
	{
		Query("SET @i = (SELECT MAX(id) FROM loginlog2 WHERE account_id=%u AND pid=%u)",
				ch->GetDesc()->GetAccountTable().id,
				ch->GetPlayerID());

		Query("UPDATE loginlog2 SET type='VALID', logout_time=NOW(), playtime=TIMEDIFF(logout_time,login_time) WHERE id=@i");
	}*/
}
