#include "stdafx.h"
#include <sstream>
#ifndef __WIN32__
#include <ifaddrs.h>
#endif

#include "constants.h"
#include "utils.h"
#include "log.h"
#include "desc.h"
#include "desc_manager.h"
#include "item_manager.h"
#include "p2p.h"
#include "char.h"
#include "war_map.h"
#include "locale_service.h"
#include "config.h"
#include "db.h"
#include "skill_power.h"

using std::string;

BYTE	g_bChannel = 0;
WORD	mother_port = 50080;
int		passes_per_sec = 25;
WORD	db_port = 0;
WORD	p2p_port = 50900;
char	db_addr[ADDRESS_MAX_LEN + 1];
int		save_event_second_cycle = passes_per_sec * 120;
int		ping_event_second_cycle = passes_per_sec * 60;
bool	g_bNoMoreClient = false;
bool	g_bNoRegen = false;
bool	g_bGlobalShoutEnable = false;
bool	g_bDisablePrismNeed = false;
bool	g_bDisableEmotionMask = false;
#ifdef __ITEM_COUNT_LIMIT__
DWORD	g_bItemCountLimit = 10000000;
#else
BYTE	g_bItemCountLimit = 200;
#endif
bool	g_bGMHostCheck = false;
bool	g_bGuildInviteLimit = false;
bool	g_bGuildInfiniteMembers = false;
int		g_iStatusPointGetLevelLimit = 90;
int		g_iStatusPointSetMaxValue = 90;
int		g_iShoutLimitLevel = 15;
int		g_iDbLogLevel = LOG_LEVEL_MAX;
int		g_aiItemDestroyTime[ITEM_DESTROY_TIME_MAX] = { 300, 300 }; // autoitem, dropitem
DWORD	g_dwSkillBookNextReadMin = 28800;
DWORD	g_dwSkillBookNextReadMax = 43200;
bool	g_bNewChatView = true;
bool	g_bPoisonUnaggr = true;
int		g_iFishingTimeMin = 5;
int		g_iFishingTimeMax = 10;
bool	g_bUnlimitedAutoPotion = true;
bool	g_bDisableSellItem = true;
bool	g_NoDropMetinStone = true;
bool	g_bEndPvpWhenDisconnect = true;
bool	g_bNonCheckDSQualified = true;
bool	g_bInfiniteHorseHealth = true;
bool	g_bCantSkillForBoss = true;
bool	g_bStoneExpConst = true;
bool	g_bSafeboxWithoutDistance = true;
bool	g_bDisableResetSubSkill = false;
bool	g_bStatPlusNoLimit = true;
bool	g_bDisableStopWhenRidingDie = false;
bool	g_bFlushAtShutdown = true;
#ifdef __OFFLINE_SHOP__
bool	g_bOfflineShopServer = false;
#endif
std::string	g_stProxyIP = "";

int			test_server = 0;
bool		guild_mark_server = true;
BYTE		guild_mark_min_level = 3;
int			g_iUserLimit = 32768;

char		g_szPublicIP[16] = "0";
char		g_szInternalIP[16] = "0";
bool		g_bSkillDisable = false;
int			g_iFullUserCount = 800;
int			g_iBusyUserCount = 200;
BYTE		g_bAuthServer = false;

string	g_stAuthMasterIP;
WORD		g_wAuthMasterPort = 0;

string g_stHostname = "";
string g_table_postfix = "";
string g_stQuestDir = "./quest";
//string g_stQuestObjectDir = "./quest/object";
string g_stDefaultQuestObjectDir = "./quest/object";
std::set<string> g_setQuestObjectDir;

string g_stBlockDate = "30000705";

extern string g_stLocale;

int VIEW_RANGE = 5000;
int VIEW_BONUS_RANGE = 500;

void		LoadStateUserCount();
#ifdef USE_OFFLINE_SHOP_CONFIG
bool		LoadOfflineShopConfig();
#endif
bool            g_protectNormalPlayer = false;

int gPlayerMaxLevel = 200;
int gShutdownAge = 0;
int gShutdownEnable = 0;

#ifdef USE_OFFLINE_SHOP_CONFIG
bool g_bShopGlobal = true; // Active P2P Cores
#endif

#ifdef __CLIENT_VERSION_UTILITY__
bool g_bOpenPlayer = false;
int g_iClientVersion = 0;

void ReloadVersion()
{
	std::unique_ptr<SQLMsg> _clientversionmsg(DBManager::instance().DirectQuery("SELECT version,status FROM account.version"));
	if (!_clientversionmsg->uiSQLErrno)
	{
		if (_clientversionmsg->Get()->uiNumRows)
		{
			while (MYSQL_ROW row_clientv = mysql_fetch_row(_clientversionmsg->Get()->pSQLResult)) {
				str_to_number(g_iClientVersion, row_clientv[0]);
				str_to_number(g_bOpenPlayer, row_clientv[1]);
			}
		}
	}

	sys_log(0, "g_iClientVersion: %d", g_iClientVersion);
	sys_log(0, "g_bOpenPlayer: %d", g_bOpenPlayer);
}
#endif

bool is_string_true(const char* string)
{
	bool	result = 0;
	if (isnhdigit(*string))
	{
		str_to_number(result, string);
		return result > 0 ? true : false;
	}
	else if (LOWER(*string) == 't')
		return true;
	else
		return false;
}

static std::set<int> s_set_map_allows;

bool map_allow_find(int index)
{
	if (g_bAuthServer)
		return false;

	if (s_set_map_allows.find(index) == s_set_map_allows.end())
		return false;

	return true;
}

void map_allow_log()
{
	std::set<int>::iterator i;

	for (i = s_set_map_allows.begin(); i != s_set_map_allows.end(); ++i)
		sys_log(0, "MAP_ALLOW: %d", *i);
}

void map_allow_add(int index)
{
	if (map_allow_find(index) == true)
	{
		fprintf(stdout, "!!! FATAL ERROR !!! MULTI MAP_ALLOW : > %i <!!\n", index); // @duzenleme eger bir coreye ayni map indexi iki defa girersek diye koyduk.
		exit(1);
	}

	fprintf(stdout, "MAP ALLOW %d\n", index);
	s_set_map_allows.insert(index);
}

void map_allow_copy(long* pl, int size)
{
	int iCount = 0;
	std::set<int>::iterator it = s_set_map_allows.begin();

	while (it != s_set_map_allows.end())
	{
		int i = *(it++);
		*(pl++) = i;

		if (++iCount > size)
			break;
	}
}

bool GetIPInfo()
{
#ifndef __WIN32__
	struct ifaddrs* ifaddrp = NULL;

	if (0 != getifaddrs(&ifaddrp))
		return false;

	for (struct ifaddrs* ifap = ifaddrp; NULL != ifap; ifap = ifap->ifa_next)
	{
		struct sockaddr_in* sai = (struct sockaddr_in*)ifap->ifa_addr;

		if (!ifap->ifa_netmask ||  // ignore if no netmask
			sai->sin_addr.s_addr == 0 || // ignore if address is 0.0.0.0
			sai->sin_addr.s_addr == 16777343) // ignore if address is 127.0.0.1
			continue;
#else
	WSADATA wsa_data;
	char host_name[100];
	HOSTENT* host_ent;
	int n = 0;

	if (WSAStartup(0x0101, &wsa_data)) {
		return false;
	}

	gethostname(host_name, sizeof(host_name));
	host_ent = gethostbyname(host_name);
	if (host_ent == NULL) {
		return false;
	}
	for (; host_ent->h_addr_list[n] != NULL; ++n) {
		struct sockaddr_in addr;
		struct sockaddr_in* sai = &addr;
		memcpy(&sai->sin_addr.s_addr, host_ent->h_addr_list[n], host_ent->h_length);
#endif

		char* netip = inet_ntoa(sai->sin_addr);

		if (!strncmp(netip, "999.999", 7)) // ignore if address is starting with 192
		{
			strlcpy(g_szInternalIP, netip, sizeof(g_szInternalIP));
#ifndef __WIN32__
			fprintf(stderr, "INTERNAL_IP: %s interface %s\n", netip, ifap->ifa_name);
#else
			fprintf(stderr, "INTERNAL_IP: %s\n", netip);
#endif
		}
		else if (!strncmp(netip, "10.", 3))
		{
			strlcpy(g_szInternalIP, netip, sizeof(g_szInternalIP));
#ifndef __WIN32__
			fprintf(stderr, "INTERNAL_IP: %s interface %s\n", netip, ifap->ifa_name);
#else
			fprintf(stderr, "INTERNAL_IP: %s\n", netip);
#endif
		}
		else if (g_szPublicIP[0] == '0')
		{
			strlcpy(g_szPublicIP, netip, sizeof(g_szPublicIP));
#ifndef __WIN32__
			fprintf(stderr, "PUBLIC_IP: %s interface %s\n", netip, ifap->ifa_name);
#else
			fprintf(stderr, "PUBLIC_IP: %s\n", netip);
#endif
		}
	}

#ifndef __WIN32__
	freeifaddrs(ifaddrp);
#else
	WSACleanup();
#endif

	if (g_szPublicIP[0] != '0')
		return true;
	else
	{
		if (g_szInternalIP[0] == '0')
			return false;
		else
		{
			strlcpy(g_szPublicIP, g_szInternalIP, sizeof(g_szPublicIP));
			fprintf(stderr, "INTERNAL_IP -> PUBLIC_IP: %s\n", g_szPublicIP);
			return true;
		}
	}
	}

static bool __LoadConnectConfigFile(const char* configName)
{
	char	buf[256];
	char	token_string[256];
	char	value_string[256];

	char db_host[2][64], db_user[2][64], db_pwd[2][64], db_db[2][64];

	int mysql_db_port[2];

	for (int n = 0; n < 2; ++n)
	{
		*db_host[n] = '\0';
		*db_user[n] = '\0';
		*db_pwd[n] = '\0';
		*db_db[n] = '\0';
		mysql_db_port[n] = 0;
	}

	char log_host[64], log_user[64], log_pwd[64], log_db[64];
	int log_port = 0;

	*log_host = '\0';
	*log_user = '\0';
	*log_pwd = '\0';
	*log_db = '\0';

	bool isCommonSQL = false;
	bool isPlayerSQL = false;

	FILE* fpOnlyForDB;

	if (!(fpOnlyForDB = fopen(configName, "r")))
	{
		fprintf(stderr, "Can not open [%s]\n", configName);
		exit(1);
	}

	while (fgets(buf, 256, fpOnlyForDB))
	{
		parse_token(buf, token_string, value_string);

		TOKEN("hostname")
		{
			g_stHostname = value_string;
			fprintf(stdout, "HOSTNAME: %s\n", g_stHostname.c_str());
			continue;
		}

		TOKEN("channel")
		{
			str_to_number(g_bChannel, value_string);
			continue;
		}

		TOKEN("player_sql")
		{
			const char* line = two_arguments(value_string, db_host[0], sizeof(db_host[0]), db_user[0], sizeof(db_user[0]));
			line = two_arguments(line, db_pwd[0], sizeof(db_pwd[0]), db_db[0], sizeof(db_db[0]));

			if ('\0' != line[0])
			{
				char buf[256];
				one_argument(line, buf, sizeof(buf));
				str_to_number(mysql_db_port[0], buf);
			}

			if (!*db_host[0] || !*db_user[0] || !*db_pwd[0] || !*db_db[0])
			{
				fprintf(stderr, "PLAYER_SQL syntax: logsql <host user password db>\n");
				exit(1);
			}

			char buf[1024];
			snprintf(buf, sizeof(buf), "PLAYER_SQL: %s %s %s %s %d", db_host[0], db_user[0], db_pwd[0], db_db[0], mysql_db_port[0]);
			isPlayerSQL = true;
			continue;
		}

		TOKEN("common_sql")
		{
			const char* line = two_arguments(value_string, db_host[1], sizeof(db_host[1]), db_user[1], sizeof(db_user[1]));
			line = two_arguments(line, db_pwd[1], sizeof(db_pwd[1]), db_db[1], sizeof(db_db[1]));

			if ('\0' != line[0])
			{
				char buf[256];
				one_argument(line, buf, sizeof(buf));
				str_to_number(mysql_db_port[1], buf);
			}

			if (!*db_host[1] || !*db_user[1] || !*db_pwd[1] || !*db_db[1])
			{
				fprintf(stderr, "COMMON_SQL syntax: logsql <host user password db>\n");
				exit(1);
			}

			char buf[1024];
			snprintf(buf, sizeof(buf), "COMMON_SQL: %s %s %s %s %d", db_host[1], db_user[1], db_pwd[1], db_db[1], mysql_db_port[1]);
			isCommonSQL = true;
			continue;
		}

		TOKEN("log_sql")
		{
			const char* line = two_arguments(value_string, log_host, sizeof(log_host), log_user, sizeof(log_user));
			line = two_arguments(line, log_pwd, sizeof(log_pwd), log_db, sizeof(log_db));

			if ('\0' != line[0])
			{
				char buf[256];
				one_argument(line, buf, sizeof(buf));
				str_to_number(log_port, buf);
			}

			if (!*log_host || !*log_user || !*log_pwd || !*log_db)
			{
				fprintf(stderr, "LOG_SQL syntax: logsql <host user password db>\n");
				exit(1);
			}

			char buf[1024];
			snprintf(buf, sizeof(buf), "LOG_SQL: %s %s %s %s %d", log_host, log_user, log_pwd, log_db, log_port);
			continue;
		}
	}

	fclose(fpOnlyForDB);

	// CONFIG_SQL_INFO_ERROR
	if (!isCommonSQL)
	{
		puts("LOAD_COMMON_SQL_INFO_FAILURE:");
		puts("");
		puts("CONFIG:");
		puts("------------------------------------------------");
		puts("COMMON_SQL: HOST USER PASSWORD DATABASE");
		puts("");
		exit(1);
	}

	if (!isPlayerSQL)
	{
		puts("LOAD_PLAYER_SQL_INFO_FAILURE:");
		puts("");
		puts("CONFIG:");
		puts("------------------------------------------------");
		puts("PLAYER_SQL: HOST USER PASSWORD DATABASE");
		puts("");
		exit(1);
	}

	AccountDB::instance().Connect(db_host[1], mysql_db_port[1], db_user[1], db_pwd[1], db_db[1]);

	if (false == AccountDB::instance().IsConnected())
	{
		fprintf(stderr, "cannot start server while no common sql connected\n");
		exit(1);
	}

	fprintf(stdout, "CommonSQL connected\n");

	{
		char szQuery[512];
		snprintf(szQuery, sizeof(szQuery), "SELECT mKey, mValue FROM locale");

		std::unique_ptr<SQLMsg> pMsg(AccountDB::instance().DirectQuery(szQuery));

		if (pMsg->Get()->uiNumRows == 0)
		{
			fprintf(stderr, "COMMON_SQL: DirectQuery failed : %s\n", szQuery);
			exit(1);
		}

		MYSQL_ROW row;

		while (NULL != (row = mysql_fetch_row(pMsg->Get()->pSQLResult)))
		{
			if (strcasecmp(row[0], "LOCALE") == 0)
			{
				if (LocaleService_Init(row[1]) == false)
				{
					fprintf(stderr, "COMMON_SQL: invalid locale key %s\n", row[1]);
					exit(1);
				}
			}
		}
	}

	fprintf(stdout, "Setting DB to locale %s\n", g_stLocale.c_str());

	AccountDB::instance().SetLocale(g_stLocale);

	AccountDB::instance().ConnectAsync(db_host[1], mysql_db_port[1], db_user[1], db_pwd[1], db_db[1], g_stLocale.c_str());

	DBManager::instance().Connect(db_host[0], mysql_db_port[0], db_user[0], db_pwd[0], db_db[0]);

	if (!DBManager::instance().IsConnected())
	{
		fprintf(stderr, "PlayerSQL.ConnectError\n");
		exit(1);
	}

	fprintf(stdout, "PlayerSQL connected\n");

	if (false == g_bAuthServer)
	{
		LogManager::instance().Connect(log_host, log_port, log_user, log_pwd, log_db);

		if (!LogManager::instance().IsConnected())
		{
			fprintf(stderr, "LogSQL.ConnectError\n");
			exit(1);
		}

		fprintf(stdout, "LogSQL connected\n");

		LogManager::instance().BootLog(g_stHostname.c_str(), g_bChannel);
	}

	// SKILL_POWER_BY_LEVEL

	{
		char szQuery[256];
		snprintf(szQuery, sizeof(szQuery), "SELECT mValue FROM locale WHERE mKey='SKILL_POWER_BY_LEVEL'");
		std::unique_ptr<SQLMsg> pMsg(AccountDB::instance().DirectQuery(szQuery));

		if (pMsg->Get()->uiNumRows == 0)
		{
			fprintf(stderr, "[SKILL_PERCENT] Query failed: %s", szQuery);
			exit(1);
		}

		MYSQL_ROW row;

		row = mysql_fetch_row(pMsg->Get()->pSQLResult);

		const char* p = row[0];
		int cnt = 0;
		char num[128];
		int aiBaseSkillPowerByLevelTable[SKILL_MAX_LEVEL + 1];

		fprintf(stdout, "SKILL_POWER_BY_LEVEL %s\n", p);
		while (*p != '\0' && cnt < (SKILL_MAX_LEVEL + 1))
		{
			p = one_argument(p, num, sizeof(num));
			aiBaseSkillPowerByLevelTable[cnt++] = atoi(num);

			//fprintf(stdout, "%d %d\n", cnt - 1, aiBaseSkillPowerByLevelTable[cnt - 1]);
			if (*p == '\0')
			{
				if (cnt != (SKILL_MAX_LEVEL + 1))
				{
					fprintf(stderr, "[SKILL_PERCENT] locale table has not enough skill information! (count: %d query: %s)", cnt, szQuery);
					exit(1);
				}

				fprintf(stdout, "SKILL_POWER_BY_LEVEL: Done! (count %d)\n", cnt);
				break;
			}
		}

		for (int job = 0; job < JOB_MAX_NUM * 2; ++job)
		{
			snprintf(szQuery, sizeof(szQuery), "SELECT mValue from locale where mKey='SKILL_POWER_BY_LEVEL_TYPE%d' ORDER BY CAST(mValue AS unsigned)", job);
			std::unique_ptr<SQLMsg> pMsg(AccountDB::instance().DirectQuery(szQuery));

			if (pMsg->Get()->uiNumRows == 0)
			{
				CTableBySkill::instance().SetSkillPowerByLevelFromType(job, aiBaseSkillPowerByLevelTable);
				continue;
			}

			row = mysql_fetch_row(pMsg->Get()->pSQLResult);
			cnt = 0;
			p = row[0];
			int aiSkillTable[SKILL_MAX_LEVEL + 1];

			fprintf(stdout, "SKILL_POWER_BY_JOB %d %s\n", job, p);
			while (*p != '\0' && cnt < (SKILL_MAX_LEVEL + 1))
			{
				p = one_argument(p, num, sizeof(num));
				aiSkillTable[cnt++] = atoi(num);

				//fprintf(stdout, "%d %d\n", cnt - 1, aiBaseSkillPowerByLevelTable[cnt - 1]);
				if (*p == '\0')
				{
					if (cnt != (SKILL_MAX_LEVEL + 1))
					{
						fprintf(stderr, "[SKILL_PERCENT] locale table has not enough skill information! (count: %d query: %s)", cnt, szQuery);
						exit(1);
					}

					fprintf(stdout, "SKILL_POWER_BY_JOB: Done! (job: %d count: %d)\n", job, cnt);
					break;
				}
			}

			CTableBySkill::instance().SetSkillPowerByLevelFromType(job, aiSkillTable);
		}
	}
	// END_SKILL_POWER_BY_LEVEL

	// LOG_KEEP_DAYS_EXTEND
	log_set_expiration_days(2);
	// END_OF_LOG_KEEP_DAYS_EXTEND
	return true;
}

static bool __LoadDefaultConfigFile(const char* configName)
{
	FILE* fp;

	char	buf[256];
	char	token_string[256];
	char	value_string[256];

	if (!(fp = fopen(configName, "r")))
		return false;

	while (fgets(buf, 256, fp))
	{
		parse_token(buf, token_string, value_string);

		TOKEN("port")
		{
			str_to_number(mother_port, value_string);
			continue;
		}

		TOKEN("p2p_port")
		{
			str_to_number(p2p_port, value_string);
			continue;
		}

		TOKEN("map_allow")
		{
			char* p = value_string;
			string stNum;

			for (; *p; p++)
			{
				if (isnhspace(*p))
				{
					if (stNum.length())
					{
						int	index = 0;
						str_to_number(index, stNum.c_str());
						map_allow_add(index);
						stNum.clear();
					}
				}
				else
					stNum += *p;
			}

			if (stNum.length())
			{
				int	index = 0;
				str_to_number(index, stNum.c_str());
				map_allow_add(index);
			}

			continue;
		}

		TOKEN("auth_server")
		{
			char szIP[32];
			char szPort[32];

			two_arguments(value_string, szIP, sizeof(szIP), szPort, sizeof(szPort));

			if (!*szIP || (!*szPort && strcasecmp(szIP, "master")))
			{
				fprintf(stderr, "AUTH_SERVER: syntax error: <ip|master> <port>\n");
				exit(1);
			}

			g_bAuthServer = true;

			if (!strcasecmp(szIP, "master"))
				fprintf(stdout, "AUTH_SERVER: I am the master\n");
			else
			{
				g_stAuthMasterIP = szIP;
				str_to_number(g_wAuthMasterPort, szPort);

				fprintf(stdout, "AUTH_SERVER: master %s %u\n", g_stAuthMasterIP.c_str(), g_wAuthMasterPort);
			}
			continue;
		}
	}

	fclose(fp);
	return true;
}

static bool __LoadGeneralConfigFile(const char* configName)
{
	FILE* fp;

	char	buf[256];
	char	token_string[256];
	char	value_string[256];

	if (!(fp = fopen(configName, "r")))
		return false;

	while (fgets(buf, 256, fp))
	{
		parse_token(buf, token_string, value_string);

		// DB_ONLY_BEGIN
		TOKEN("BLOCK_LOGIN")
		{
			g_stBlockDate = value_string;
		}
		// DB_ONLY_END

		// CONNECTION_BEGIN
		TOKEN("db_port")
		{
			str_to_number(db_port, value_string);
			continue;
		}

		TOKEN("db_addr")
		{
			strlcpy(db_addr, value_string, sizeof(db_addr));

			for (int n = 0; n < ADDRESS_MAX_LEN; ++n)
			{
				if (db_addr[n] == ' ')
					db_addr[n] = '\0';
			}

			continue;
		}
		// CONNECTION_END

		TOKEN("mark_server")
		{
			guild_mark_server = is_string_true(value_string);
			continue;
		}

		TOKEN("mark_min_level")
		{
			str_to_number(guild_mark_min_level, value_string);
			guild_mark_min_level = MINMAX(0, guild_mark_min_level, GUILD_MAX_LEVEL);
			continue;
		}

		TOKEN("log_keep_days")
		{
			int i = 0;
			str_to_number(i, value_string);
			log_set_expiration_days(MINMAX(1, i, 90));
			continue;
		}

		TOKEN("passes_per_sec")
		{
			str_to_number(passes_per_sec, value_string);
			continue;
		}

		TOKEN("save_event_second_cycle")
		{
			int	cycle = 0;
			str_to_number(cycle, value_string);
			save_event_second_cycle = cycle * passes_per_sec;
			continue;
		}

		TOKEN("table_postfix")
		{
			g_table_postfix = value_string;
			continue;
		}

		TOKEN("test_server")
		{
			printf("-----------------------------------------------\n");
			printf("TEST_SERVER\n");
			printf("-----------------------------------------------\n");
			str_to_number(test_server, value_string);
			continue;
		}

		TOKEN("shutdowned")
		{
			g_bNoMoreClient = true;
			continue;
		}

		TOKEN("no_regen")
		{
			g_bNoRegen = true;
			continue;
		}

		TOKEN("item_count_limit")
		{
			str_to_number(g_bItemCountLimit, value_string);
			fprintf(stdout, "ITEM_COUNT_LIMIT: %d\n", g_bItemCountLimit);
			continue;
		}

		TOKEN("disable_prism_item")
		{
			g_bDisablePrismNeed = true;
			fprintf(stdout, "PRISM_ITEM_REQUIREMENT: DISABLED\n");
			continue;
		}

		TOKEN("prism_item_require") //alternative
		{
			int flag = 0;
			str_to_number(flag, value_string);
			g_bDisablePrismNeed = !flag;
			fprintf(stdout, "PRISM_ITEM_REQUIRE: %s\n", (!g_bDisablePrismNeed) ? "ENABLED" : "DISABLED");
			continue;
		}

		TOKEN("enable_global_shout")
		{
			g_bGlobalShoutEnable = true;
			fprintf(stdout, "GLOBAL_SHOUT: ENABLED\n");
			continue;
		}

		TOKEN("global_shout") //alternative
		{
			int flag = 0;
			str_to_number(flag, value_string);
			g_bGlobalShoutEnable = !!flag;
			fprintf(stdout, "GLOBAL_SHOUT: %s\n", (g_bGlobalShoutEnable) ? "ENABLED" : "DISABLED");
			continue;
		}

		TOKEN("disable_emotion_mask")
		{
			g_bDisableEmotionMask = true;
			fprintf(stdout, "EMOTION_MASK_REQUIREMENT: DISABLED\n");
			continue;
		}

		TOKEN("emotion_mask_require") //alternative
		{
			int flag = 0;
			str_to_number(flag, value_string);
			g_bDisableEmotionMask = !flag;
			fprintf(stdout, "EMOTION_MASK_REQUIRE: %s\n", (g_bDisableEmotionMask) ? "ENABLED" : "DISABLED");
			continue;
		}

		TOKEN("status_point_get_level_limit")
		{
			int flag = 0;
			str_to_number(flag, value_string);
			if (flag <= 0) continue;

			g_iStatusPointGetLevelLimit = MINMAX(0, flag, PLAYER_MAX_LEVEL_CONST);
			fprintf(stdout, "STATUS_POINT_GET_LEVEL_LIMIT: %d\n", g_iStatusPointGetLevelLimit);
			continue;
		}

		TOKEN("status_point_set_max_value")
		{
			int flag = 0;
			str_to_number(flag, value_string);
			if (flag <= 0) continue;

			g_iStatusPointSetMaxValue = flag;
			fprintf(stdout, "STATUS_POINT_SET_MAX_VALUE: %d\n", g_iStatusPointSetMaxValue);
			continue;
		}

		TOKEN("shout_limit_level")
		{
			int flag = 0;
			str_to_number(flag, value_string);
			if (flag <= 0) continue;

			g_iShoutLimitLevel = flag;
			fprintf(stdout, "SHOUT_LIMIT_LEVEL: %d\n", g_iShoutLimitLevel);
			continue;
		}

		TOKEN("db_log_level")
		{
			int flag = 0;
			str_to_number(flag, value_string);

			g_iDbLogLevel = flag;
			fprintf(stdout, "DB_LOG_LEVEL: %d\n", g_iDbLogLevel);
			continue;
		}

		TOKEN("item_destroy_time_autogive")
		{
			int flag = 0;
			str_to_number(flag, value_string);

			g_aiItemDestroyTime[ITEM_DESTROY_TIME_AUTOGIVE] = flag;
			fprintf(stdout, "ITEM_DESTROY_TIME_AUTOGIVE: %d\n", g_aiItemDestroyTime[ITEM_DESTROY_TIME_AUTOGIVE]);
			continue;
		}

		TOKEN("item_destroy_time_dropitem")
		{
			int flag = 0;
			str_to_number(flag, value_string);

			g_aiItemDestroyTime[ITEM_DESTROY_TIME_DROPITEM] = flag;
			fprintf(stdout, "ITEM_DESTROY_TIME_DROPITEM: %d\n", g_aiItemDestroyTime[ITEM_DESTROY_TIME_DROPITEM]);
			continue;
		}

		TOKEN("gm_host_check")
		{
			DWORD flag = 0;
			str_to_number(flag, value_string);

			g_bGMHostCheck = !!flag;
			fprintf(stdout, "GM_HOST_CHECK: %d\n", g_bGMHostCheck);
			continue;
		}

		TOKEN("guild_invite_limit")
		{
			DWORD flag = 0;
			str_to_number(flag, value_string);

			g_bGuildInviteLimit = !!flag;
			fprintf(stdout, "GUILD_INVITE_LIMIT: %d\n", g_bGuildInviteLimit);
			continue;
		}

		TOKEN("guild_infinite_members")
		{
			DWORD flag = 0;
			str_to_number(flag, value_string);

			g_bGuildInfiniteMembers = !!flag;
			fprintf(stdout, "GUILD_INFINITE_MEMBERS: %d\n", g_bGuildInfiniteMembers);
			continue;
		}

		TOKEN("skillbook_nextread_min")
		{
			DWORD flag = 0;
			str_to_number(flag, value_string);
			g_dwSkillBookNextReadMin = flag;
			fprintf(stdout, "SKILLBOOK_NEXTREAD_MIN: %u\n", g_dwSkillBookNextReadMin);
			continue;
		}

		TOKEN("skillbook_nextread_max")
		{
			DWORD flag = 0;
			str_to_number(flag, value_string);
			g_dwSkillBookNextReadMax = flag;
			fprintf(stdout, "SKILLBOOK_NEXTREAD_MAX: %u\n", g_dwSkillBookNextReadMax);
			continue;
		}

		TOKEN("enable_chat_extension")
		{
			DWORD flag = 0;
			str_to_number(flag, value_string);
			g_bNewChatView = flag;
			fprintf(stdout, "ENABLE_CHAT_EXTENSION: %u\n", g_bNewChatView);
			continue;
		}

		TOKEN("enable_poison_unaggr")
		{
			DWORD flag = 0;
			str_to_number(flag, value_string);
			g_bPoisonUnaggr = flag;
			fprintf(stdout, "ENABLE_POISON_UNAGGR: %u\n", g_bPoisonUnaggr);
			continue;
		}

		TOKEN("fishing_time_min")
		{
			str_to_number(g_iFishingTimeMin, value_string);
			fprintf(stdout, "FISHING_TIME_MIN: %d\n", g_iFishingTimeMin);
			continue;
		}

		TOKEN("fishing_time_max")
		{
			str_to_number(g_iFishingTimeMax, value_string);
			fprintf(stdout, "FISHING_TIME_MAX: %d\n", g_iFishingTimeMax);
			continue;
		}

		TOKEN("enable_unlimited_auto_potion")
		{
			DWORD flag = 0;
			str_to_number(flag, value_string);
			g_bUnlimitedAutoPotion = flag;
			fprintf(stdout, "ENABLE_UNLIMITED_AUTO_POTION: %u\n", g_bUnlimitedAutoPotion);
			continue;
		}

		TOKEN("disable_sell_item")
		{
			DWORD flag = 0;
			str_to_number(flag, value_string);
			g_bDisableSellItem = flag;
			fprintf(stdout, "DISABLE_SELL_ITEM: %u\n", g_bDisableSellItem);
			continue;
		}

		TOKEN("no_drop_metin_stone")
		{
			DWORD flag = 0;
			str_to_number(flag, value_string);
			g_NoDropMetinStone = flag;
			fprintf(stdout, "NO_DROP_METIN_STONE: %u\n", g_NoDropMetinStone);
			continue;
		}

		TOKEN("enable_end_pvp_when_disconect")
		{
			DWORD flag = 0;
			str_to_number(flag, value_string);
			g_bEndPvpWhenDisconnect = flag;
			fprintf(stdout, "ENABLE_END_PVP_WHEN_DISCONNECT: %u\n", g_bEndPvpWhenDisconnect);
			continue;
		}

		TOKEN("enable_non_check_ds_qualified")
		{
			DWORD flag = 0;
			str_to_number(flag, value_string);
			g_bNonCheckDSQualified = flag;
			fprintf(stdout, "ENABLE_NON_CHECK_DS_QUALIFIED: %u\n", g_bNonCheckDSQualified);
			continue;
		}

		TOKEN("enable_infinite_horse_health")
		{
			DWORD flag = 0;
			str_to_number(flag, value_string);
			g_bInfiniteHorseHealth = flag;
			fprintf(stdout, "ENABLE_INFINITE_HORSE_HEALTH: %u\n", g_bInfiniteHorseHealth);
			continue;
		}

		TOKEN("enable_cant_skill_for_boss")
		{
			DWORD flag = 0;
			str_to_number(flag, value_string);
			g_bCantSkillForBoss = flag;
			fprintf(stdout, "ENABLE_CANT_SKILL_FOR_BOSS: %u\n", g_bCantSkillForBoss);
			continue;
		}

		TOKEN("enable_stone_exp_const")
		{
			DWORD flag = 0;
			str_to_number(flag, value_string);
			g_bStoneExpConst = flag;
			fprintf(stdout, "ENABLE_STONE_EXP_CONST: %u\n", g_bStoneExpConst);
			continue;
		}

		TOKEN("enable_safebox_without_distance")
		{
			DWORD flag = 0;
			str_to_number(flag, value_string);
			g_bSafeboxWithoutDistance = flag;
			fprintf(stdout, "ENABLE_SAFEBOX_WITHOUT_DISTANCE: %u\n", g_bSafeboxWithoutDistance);
			continue;
		}

		TOKEN("disable_reset_sub_skill")
		{
			DWORD flag = 0;
			str_to_number(flag, value_string);
			g_bDisableResetSubSkill = flag;
			fprintf(stdout, "DISABLE_RESET_SUB_SKILL: %u\n", g_bDisableResetSubSkill);
			continue;
		}

		TOKEN("enable_stat_plus_no_limit")
		{
			DWORD flag = 0;
			str_to_number(flag, value_string);
			g_bStatPlusNoLimit = flag;
			fprintf(stdout, "ENABLE_STAT_PLUS_NO_LIMIT: %u\n", g_bStatPlusNoLimit);
			continue;
		}

		TOKEN("disable_stop_when_riding_die")
		{
			DWORD flag = 0;
			str_to_number(flag, value_string);
			g_bDisableStopWhenRidingDie = flag;
			fprintf(stdout, "DISABLE_STOP_WHEN_RIDING_DIE: %u\n", g_bDisableStopWhenRidingDie);
			continue;
		}

		TOKEN("enable_flush_at_shutdown")
		{
			DWORD flag = 0;
			str_to_number(flag, value_string);
			g_bFlushAtShutdown = flag;
			fprintf(stdout, "ENABLE_FLUSH_AT_SHUTDOWN: %u\n", g_bFlushAtShutdown);
			continue;
		}

		TOKEN("proxy_ip")
		{
			g_stProxyIP = value_string;
		}

		TOKEN("user_limit")
		{
			str_to_number(g_iUserLimit, value_string);
			continue;
		}

		TOKEN("skill_disable")
		{
			str_to_number(g_bSkillDisable, value_string);
			continue;
		}

		TOKEN("quest_dir")
		{
			sys_log(0, "QUEST_DIR SETTING : %s", value_string);
			g_stQuestDir = value_string;
		}

		TOKEN("quest_object_dir")
		{
			//g_stQuestObjectDir = value_string;
			std::istringstream is(value_string);
			sys_log(0, "QUEST_OBJECT_DIR SETTING : %s", value_string);
			string dir;
			while (!is.eof())
			{
				is >> dir;
				if (is.fail())
					break;
				g_setQuestObjectDir.insert(dir);
				sys_log(0, "QUEST_OBJECT_DIR INSERT : %s", dir.c_str());
			}
		}

		TOKEN("bind_ip")
		{
			strlcpy(g_szPublicIP, value_string, sizeof(g_szPublicIP));
		}

		TOKEN("view_range")
		{
			str_to_number(VIEW_RANGE, value_string);
		}

		TOKEN("protect_normal_player")
		{
			str_to_number(g_protectNormalPlayer, value_string);
		}

		TOKEN("pk_protect_level")
		{
			str_to_number(PK_PROTECT_LEVEL, value_string);
			fprintf(stderr, "PK_PROTECT_LEVEL: %d", PK_PROTECT_LEVEL);
		}

		TOKEN("max_level")
		{
			str_to_number(gPlayerMaxLevel, value_string);

			gPlayerMaxLevel = MINMAX(1, gPlayerMaxLevel, PLAYER_MAX_LEVEL_CONST);

			fprintf(stderr, "PLAYER_MAX_LEVEL: %d\n", gPlayerMaxLevel);
		}

		TOKEN("shutdown_age")
		{
			str_to_number(gShutdownAge, value_string);
			fprintf(stderr, "SHUTDOWN_AGE: %d\n", gShutdownAge);
		}

		TOKEN("shutdown_enable")
		{
			str_to_number(gShutdownEnable, value_string);
			fprintf(stderr, "SHUTDOWN_ENABLE: %d\n", gShutdownEnable);
		}

		TOKEN("limit_desc_busy")
		{
			str_to_number(g_iBusyUserCount, value_string);
			fprintf(stderr, "BUSY_USER_COUNT: %d\n", g_iBusyUserCount);
		}

		TOKEN("limit_desc_full")
		{
			str_to_number(g_iFullUserCount, value_string);
			fprintf(stderr, "FULL_USER_COUNT: %d\n", g_iFullUserCount);
		}

		TOKEN("offline_shop_server")
		{
			str_to_number(g_bOfflineShopServer, value_string);
			fprintf(stderr, "OFFLINE_SHOP_SERVER: %d\n", g_bOfflineShopServer);
		}
	}
	fclose(fp);
	return true;
}

#ifdef __MARTY_EXPTABLE_FROMDB__
static bool __LoadExpTableFromDB(void)
{
	std::unique_ptr<SQLMsg> pMsg(AccountDB::instance().DirectQuery("SELECT level, exp FROM exp_table"));
	if (pMsg->Get()->uiNumRows == 0)
		return false;

	static DWORD new_exp_table[PLAYER_MAX_LEVEL_CONST + 1];
	if (exp_table != NULL)
		memcpy(new_exp_table, exp_table, (PLAYER_MAX_LEVEL_CONST + 1) * sizeof(DWORD));

	MYSQL_ROW row = NULL;
	while ((row = mysql_fetch_row(pMsg->Get()->pSQLResult)))
	{
		DWORD level = 0;
		DWORD exp = 0;
		str_to_number(level, row[0]);
		str_to_number(exp, row[1]);
		if (level > PLAYER_MAX_LEVEL_CONST)
			continue;
		new_exp_table[level] = exp;
		// printf("new_exp_table[%u] = %u;\n", level, exp);
	}
	exp_table = new_exp_table;
	return true;
}
#endif

void config_init(const string & st_localeServiceName)
{
	// LOCALE_SERVICE
	string	st_configFileName;

	st_configFileName.reserve(32);
	st_configFileName = "CONFIG";

	if (!st_localeServiceName.empty())
	{
		st_configFileName += ".";
		st_configFileName += st_localeServiceName;
	}
	// END_OF_LOCALE_SERVICE

	if (!GetIPInfo())
	{
		//	fprintf(stderr, "Can not get public ip address\n");
		//	exit(1);
	}

	// default config load (REQUIRED)
	if (!__LoadConnectConfigFile(st_configFileName.c_str()) ||
		!__LoadDefaultConfigFile(st_configFileName.c_str()) ||
		!__LoadGeneralConfigFile(st_configFileName.c_str())
		)
	{
		fprintf(stderr, "Can not open [%s]\n", st_configFileName.c_str());
		exit(1);
	}

	if (g_setQuestObjectDir.empty())
		g_setQuestObjectDir.insert(g_stDefaultQuestObjectDir);

	if (0 == db_port)
	{
		fprintf(stderr, "DB_PORT not configured\n");
		exit(1);
	}

	if (0 == g_bChannel)
	{
		fprintf(stderr, "CHANNEL not configured\n");
		exit(1);
	}

	if (g_stHostname.empty())
	{
		fprintf(stderr, "HOSTNAME must be configured.\n");
		exit(1);
	}

	// LOCALE_SERVICE
	LocaleService_LoadLocaleStringFile();
	LocaleService_TransferDefaultSetting();
	// END_OF_LOCALE_SERVICE

#ifdef __MARTY_EXPTABLE_FROMDB__
	if (!__LoadExpTableFromDB())
	{
		// do as you please to manage this
		fprintf(stderr, "Failed to Load ExpTable from DB so exit\n");
		// exit(1);
	}
#endif

#ifdef __CLIENT_VERSION_UTILITY__
	ReloadVersion();
#endif

	assert(test_server);

	LoadStateUserCount();
#ifdef USE_OFFLINE_SHOP_CONFIG
	LoadOfflineShopConfig();
#endif

	CWarMapManager::instance().LoadWarMapInfo(NULL);

	if (g_szPublicIP[0] == '0')
	{
		fprintf(stderr, "Can not get public ip address\n");
		exit(1);
	}
}

const char* get_table_postfix()
{
	return g_table_postfix.c_str();
}

void LoadStateUserCount()
{
	FILE* fp = fopen("state_user_count", "r");

	if (!fp)
		return;

	fscanf(fp, " %d %d ", &g_iFullUserCount, &g_iBusyUserCount);

	fclose(fp);
}

#ifdef USE_OFFLINE_SHOP_CONFIG
static std::set<int> s_set_shop_cities;
bool shop_cities_find(int index)
{
	if (g_bAuthServer)
		return false;

	if (s_set_shop_cities.find(index) == s_set_shop_cities.end())
		return false;

	return true;
}

void shop_cities_add(int index)
{
	if (shop_cities_find(index) == true)
	{
		fprintf(stdout, "!!! FATAL ERROR !!! MULTI SHOP_CITIES : > %i <!!\n", index);
		exit(1);
	}

	fprintf(stdout, "SHOP_CITIES %d\n", index);
	s_set_shop_cities.insert(index);
}

size_t shop_cities_size()
{
	return s_set_shop_cities.size();
}

WORD get_shop_cities_index(WORD wMap)
{
	WORD wIndex = 0;
	for (auto it = s_set_shop_cities.begin(); it != s_set_shop_cities.end(); it++)
	{
		if (*it == wMap)
			return wIndex;

		wIndex++;
	}
	return -1;
}

bool LoadOfflineShopConfig()
{
	std::string	st_configFileName = "SHOP_CONFIG";
	FILE* fp = nullptr;

	char	buf[256];
	char	token_string[256];
	char	value_string[256];

	if (!(fp = fopen(st_configFileName.c_str(), "r")))
		return false;

	while (fgets(buf, 256, fp))
	{
		parse_token(buf, token_string, value_string);

		TOKEN("shop_cities")
		{
			char * p = value_string;
			string stNum;

			for (; *p; p++)
			{
				if (isnhspace(*p))
				{
					if (stNum.length())
					{
						int	index = 0;
						str_to_number(index, stNum.c_str());
						shop_cities_add(index);
						stNum.clear();
					}
				}
				else
					stNum += *p;
			}

			if (stNum.length())
			{
				int	index = 0;
				str_to_number(index, stNum.c_str());
				shop_cities_add(index);
			}
			continue;
		}

		TOKEN("shop_global")
		{
			int32_t flag = 0;
			str_to_number(flag, value_string);
			g_bShopGlobal = !flag;
			fprintf(stdout, "SHOP_GLOBAL: %s\n", (!g_bShopGlobal)?"ENABLED":"DISABLED");
			continue;
		}
	}

	fclose(fp);
	return true;
}
#endif