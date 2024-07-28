#pragma once
#include "../../libsql/AsyncSQL.h"
#include "locale_service.h"

enum log_level { LOG_LEVEL_NONE = 0, LOG_LEVEL_MIN = 1, LOG_LEVEL_MID = 2, LOG_LEVEL_MAX = 3 };

#define LOG_LEVEL_CHECK_N_RET(x) { if (g_iDbLogLevel < x) return; }

#define LOG_LEVEL_CHECK(x, fnc)	\
	{\
		if (g_iDbLogLevel >= (x))\
			fnc;\
	}

enum ExchangeType //Matches Enum in table
{
	EXCHANGE_TYPE_SHOP = 1,
	EXCHANGE_TYPE_TRADE = 2,
};

class LogManager : public singleton<LogManager>
{
public:
	LogManager();
	virtual ~LogManager();

	bool		IsConnected();

	bool		Connect(const char* host, const int port, const char* user, const char* pwd, const char* db);

	void		HackLog(const char* c_pszHackName, const char* c_pszLogin, const char* c_pszName, const char* c_pszIP);
	void		HackLog(const char* c_pszHackName, LPCHARACTER ch);
	void		GMCommandLog(DWORD dwPID, const char* szName, const char* szIP, BYTE byChannel, const char* szCommand);
	void		ChangeNameLog(DWORD pid, const char* old_name, const char* new_name, const char* ip);
	void		BootLog(const char* c_pszHostName, BYTE bChannel);
#if defined(__GOLD_LIMIT_REWORK__) && defined(__HWID_SECURITY_UTILITY__) && defined(__CHEQUE_SYSTEM__)
	DWORD		ExchangeLog(int type, DWORD item_count, DWORD dwPID1, const char* playerA, DWORD dwPID2, const char* playerB, long x, long y, long long goldA = 0, long long goldB = 0, DWORD chequeA = 0, DWORD chequeB = 0, const char* playerAHwid = "NULL", const char* playerBHwid = "NULL", const char* playerAHDDSerial = "NULL", const char* playerBHDDSerial = "NULL");
#else
	DWORD		ExchangeLog(int type, DWORD item_count, DWORD dwPID1, const char* playerA, DWORD dwPID2, const char* playerB, long x, long y, int goldA = 0, int goldB = 0);
#endif
	void		ExchangeItemLog(DWORD tradeID, LPITEM item, const char* player);
#ifdef __MARTY_CHAT_LOGGING__
	void		ChatLog(DWORD where, DWORD who_id, const char* who_name, DWORD whom_id, const char* whom_name, const char* type, const char* msg, const char* ip);
#endif
#ifdef __INGAME_MALL__
	void		ItemShopLog(LPCHARACTER ch, LPITEM item, std::string logType, long ep);
#endif
	void		HackLogEx(LPCHARACTER ch, const char* textLine);
	void		HackLogEx(LPDESC desc, const char* textLine);
	void		HackLogEx(std::string stLogin, const char* textLine);
#ifdef __BAN_REASON_UTILITY__
	void		BanLog(LPCHARACTER ch, std::string bannedAccountName, bool isPlayer, std::string banType, DWORD banDuration);
#endif
	void		PayToSellLog(LPCHARACTER ch, const char* textLine);
	void		DetailLoginLog(bool isLogin, LPCHARACTER ch);

	size_t EscapeString(char* dst, size_t dstSize, const char* src, size_t srcSize);
private:
	void		Query(const char* c_pszFormat, ...);
	SQLMsg* DirectQuery(const char* c_pszFormat, ...);
	CAsyncSQL	m_sql;
	CAsyncSQL	m_sql_direct;
	bool		m_bIsConnect;
};
