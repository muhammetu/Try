#pragma once
#include <mysql/mysql.h>

#include "../../libsql/AsyncSQL.h"

#define SQL_SAFE_LENGTH(size)	(size * 2 + 1)
#define QUERY_SAFE_LENGTH(size)	(1024 + SQL_SAFE_LENGTH(size))

class CQueryInfo
{
public:
	int	iType;
	DWORD	dwIdent;
	void* pvData;
};

enum eSQL_SLOT
{
	SQL_PLAYER,
	// birlikte calisan quesryler ayni olmali cunku zamanlamalari farkli olabilir!
	SQL_PLAYER_ITEM = SQL_PLAYER,
	SQL_PLAYER_SAFEBOX = SQL_PLAYER,
	SQL_PLAYER_AFFECT = SQL_PLAYER,
	SQL_PLAYER_QUEST = SQL_PLAYER,
	SQL_PLAYER_SKILLCOLOR = SQL_PLAYER,
	SQL_PLAYER_STATISTICS = SQL_PLAYER,
	SQL_PLAYER_OPTIONS = SQL_PLAYER,
	SQL_PLAYER_GUILD = SQL_PLAYER,
	SQL_PLAYER_OFFLINESHOP = SQL_PLAYER,
	SQL_PLAYER_BATTLE_PASS = SQL_PLAYER,
	SQL_ACCOUNT,
	SQL_COMMON,
	SQL_MAX_NUM,
};

class CDBManager : public singleton<CDBManager>
{
protected:
	void			Initialize();
	void			Destroy();

public:
	CDBManager();
	virtual ~CDBManager();

	void			Clear();
	void			Quit();

	int			Connect(int iSlot, const char* host, int port, const char* dbname, const char* user, const char* pass);

	void			ReturnQuery(const char* c_pszQuery, int iType, DWORD dwIdent, void* pvData, int iSlot = SQL_PLAYER);
	void			AsyncQuery(const char* c_pszQuery, int iSlot = SQL_PLAYER);
	SQLMsg* DirectQuery(const char* c_pszQuery, int iSlot = SQL_PLAYER);

	SQLMsg* PopResult();
	SQLMsg* PopResult(eSQL_SLOT slot);

	unsigned long		EscapeString(void* to, const void* from, unsigned long length, int iSlot = SQL_PLAYER);

	DWORD			CountReturnQuery(int i) { return m_mainSQL[i] ? m_mainSQL[i]->CountQuery() : 0; }
	DWORD			CountReturnResult(int i) { return m_mainSQL[i] ? m_mainSQL[i]->CountResult() : 0; }
	DWORD			CountReturnQueryFinished(int i) { return m_mainSQL[i] ? m_mainSQL[i]->CountQueryFinished() : 0; }
	DWORD			CountReturnCopiedQuery(int i) { return m_mainSQL[i] ? m_mainSQL[i]->GetCopiedQueryCount() : 0; }

	DWORD			CountAsyncQuery(int i) { return m_asyncSQL[i] ? m_asyncSQL[i]->CountQuery() : 0; }
	DWORD			CountAsyncResult(int i) { return m_asyncSQL[i] ? m_asyncSQL[i]->CountResult() : 0; }
	DWORD			CountAsyncQueryFinished(int i) { return m_asyncSQL[i] ? m_asyncSQL[i]->CountQueryFinished() : 0; }
	DWORD			CountAsyncCopiedQuery(int i) { return m_asyncSQL[i] ? m_asyncSQL[i]->GetCopiedQueryCount() : 0; }

	void			ResetCounter()
	{
		for (int i = 0; i < SQL_MAX_NUM; ++i)
		{
			if (m_mainSQL[i])
			{
				m_mainSQL[i]->ResetQueryFinished();
				m_mainSQL[i]->ResetCopiedQueryCount();
			}

			if (m_asyncSQL[i])
			{
				m_asyncSQL[i]->ResetQueryFinished();
				m_asyncSQL[i]->ResetCopiedQueryCount();
			}
		}
	}

private:
	CAsyncSQL2* m_mainSQL[SQL_MAX_NUM];
	CAsyncSQL2* m_directSQL[SQL_MAX_NUM];
	CAsyncSQL2* m_asyncSQL[SQL_MAX_NUM];

	int			m_quit;		// looping flag
	//CHARSET
public:
	void SetLocale(const char* szLocale);
	void QueryLocaleSet();
	//END_CHARSET
};