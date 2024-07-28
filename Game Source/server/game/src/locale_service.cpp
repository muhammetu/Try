#include "stdafx.h"
#include "locale_service.h"
#include "constants.h"
#include "utils.h"
#include "mob_manager.h"
#include "config.h"
#include "skill_power.h"

using namespace std;

extern string		g_stQuestDir;
extern set<string> 	g_setQuestObjectDir;

string g_stServiceName;
string g_stServiceBasePath = ".";
string g_stServiceMapPath = "data/map";

string g_stLocale = "latin5";
string g_stLocaleFilename;

BYTE PK_PROTECT_LEVEL = 30;

string 			g_stLocal = "";
eLocalization 	g_eLocalType = LC_NOSET;

int (*check_name) (const char* str) = NULL;
int (*is_twobyte) (const char* str) = NULL;
bool LC_InitLocalization(const std::string& szLocal);

int is_twobyte_turkey(const char* str)
{
	return ishan(*str);
}

int check_name_independent(const char* str)
{
	char szTmp[256];
	str_lower(str, szTmp, sizeof(szTmp));

	if (CMobManager::instance().Get(szTmp, false))
		return 0;

	return 1;
}

int check_name_turkey(const char* str)
{
	int		code;
	const char* tmp;

	if (!str || !*str)
		return 0;

	if (strlen(str) < 4 || strlen(str) > 12)
		return 0;

	for (tmp = str; *tmp; ++tmp)
	{
		if (isnhspace(*tmp))
			return 0;

		if (isnhdigit(*tmp))
			continue;

		if (!ishan(*tmp) && isalpha(*tmp))
			continue;

		code = *tmp;
		code += 256;

		if (code < 176 || code > 200)
			return 0;

		++tmp;

		if (!*tmp)
			break;
	}

	return check_name_independent(str);
}

int check_name_alphabet(const char* str)
{
	const char* tmp;

	if (!str || !*str)
		return 0;

	if (strlen(str) < 2)
		return 0;

	for (tmp = str; *tmp; ++tmp)
	{
		if (isdigit(*tmp) || isalpha(*tmp))
			continue;
		else
			return 0;
	}

	return check_name_independent(str);
}

void LocaleService_LoadLocaleStringFile()
{
	if (g_stLocaleFilename.empty())
		return;

	if (g_bAuthServer)
		return;

	fprintf(stderr, "LocaleService %s\n", g_stLocaleFilename.c_str());

	locale_init(g_stLocaleFilename.c_str());
}


static void __LocaleService_Init_Turkey()
{
	g_stLocale = "latin5";
	g_stServiceBasePath = "locale/turkey";
	g_stQuestDir = "locale/turkey/quest";
	g_stServiceMapPath = "locale/turkey/map";

	g_setQuestObjectDir.clear();
	g_setQuestObjectDir.insert("locale/turkey/quest/object");
	g_stLocaleFilename = "locale/turkey/locale_string.txt";

	g_iUseLocale = TRUE;

	check_name = check_name_alphabet;

	PK_PROTECT_LEVEL = 15;
}

static void __CheckPlayerSlot(const std::string& service_name)
{
#ifndef __PLAYER_PER_ACCOUNT_5__
	if (PLAYER_PER_ACCOUNT != 4)
	{
		printf("<ERROR> PLAYER_PER_ACCOUNT = %d\n", PLAYER_PER_ACCOUNT);
		exit(0);
	}
#endif
}

bool LocaleService_Init(const std::string& c_rstServiceName)
{
	if (!g_stServiceName.empty())
	{
		sys_err("ALREADY exist service");
		return false;
	}

	g_stServiceName = c_rstServiceName;

	if ("turkey" == g_stServiceName)
		__LocaleService_Init_Turkey();

	fprintf(stdout, "Setting Locale \"%s\" (Path: %s)\n", g_stServiceName.c_str(), g_stServiceBasePath.c_str());

	__CheckPlayerSlot(g_stServiceName);

	if (false == LC_InitLocalization(c_rstServiceName))
		return false;

	return true;
}

void LocaleService_TransferDefaultSetting()
{
	if (!check_name)
		check_name = check_name_turkey;

	if (!is_twobyte)
		is_twobyte = is_twobyte_turkey;

	if (!exp_table)
		exp_table = exp_table_common;

#ifdef __GROWTH_PET_SYSTEM__
	if (!exppet_table)
		exppet_table = exppet_table_common;
#endif

#ifdef __GROWTH_MOUNT_SYSTEM__
	if (!expmount_table)
		expmount_table = expmount_table_common;
#endif

	if (!CTableBySkill::instance().Check())
		exit(1);

	if (!aiPercentByDeltaLevForBoss)
		aiPercentByDeltaLevForBoss = aiPercentByDeltaLevForBoss_turkey;

	if (!aiPercentByDeltaLev)
		aiPercentByDeltaLev = aiPercentByDeltaLev_turkey;

	if (!aiChainLightningCountBySkillLevel)
		aiChainLightningCountBySkillLevel = aiChainLightningCountBySkillLevel_turkey;
}

const std::string& LocaleService_GetBasePath()
{
	return g_stServiceBasePath;
}

const std::string& LocaleService_GetMapPath()
{
	return g_stServiceMapPath;
}

const std::string& LocaleService_GetQuestPath()
{
	return g_stQuestDir;
}

bool LC_InitLocalization(const std::string& szLocal)
{
	g_stLocal = szLocal;

	if (!g_stLocal.compare("turkey"))
		g_eLocalType = LC_TURKEY;
	else
		return false;

	return true;
}

eLocalization LC_GetLocalType()
{
	return g_eLocalType;
}

bool LC_IsLocale(const eLocalization t)
{
	return LC_GetLocalType() == t ? true : false;
}

bool LC_IsEurope()
{
	eLocalization val = LC_GetLocalType();

	switch ((int)val)
	{
	case LC_TURKEY:
		return true;
	}

	return false;
}