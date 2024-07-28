#include "StdAfx.h"
#include "Locale.h"
#include "PythonApplication.h"
#include "resource.h"
#include "../eterBase/CRC32.h"
#include "../eterpack/EterPackManager.h"
#include <windowsx.h>

char	MULTI_LOCALE_SERVICE[256] = "TURKEY";
char	MULTI_LOCALE_PATH[256] = "locale/tr";
char	MULTI_LOCALE_NAME[256] = "tr";
int		MULTI_LOCALE_CODE = 1254;

void LocaleService_LoadConfig(const char* fileName)
{
	FILE* fp = fopen(fileName, "rt");

	if (fp)
	{
		char	line[256];
		char	name[256];
		int		code;
		int	id;
		if (fgets(line, sizeof(line) - 1, fp))
		{
			line[sizeof(line) - 1] = '\0';
			sscanf(line, "%d %d %s", &id, &code, name);

			MULTI_LOCALE_CODE = code;
			strcpy(MULTI_LOCALE_NAME, name);
			sprintf(MULTI_LOCALE_PATH, "locale/%s", MULTI_LOCALE_NAME);
		}
		fclose(fp);
	}
}

unsigned LocaleService_GetLastExp(int level)
{
	static const int GUILD_LEVEL_MAX = 40;

	static DWORD INTERNATIONAL_GUILDEXP_LIST[GUILD_LEVEL_MAX + 1] =
	{
		0,
		6000UL,
		18000UL,
		36000UL,
		64000UL,
		94000UL,
		130000UL,
		172000UL,
		220000UL,
		274000UL,
		334000UL,
		400000UL,
		600000UL,
		840000UL,
		1120000UL,
		1440000UL,
		1800000UL,
		2600000UL,
		3200000UL,
		4000000UL,
		4200000UL, // 20
		4400000UL,
		4600000UL,
		4800000UL,
		5000000UL,
		6000000UL,
		7000000UL,
		8000000UL,
		9000000UL,
		10000000UL,
		11000000UL, // 30
		12000000UL,
		13000000UL,
		14000000UL,
		15000000UL,
		16000000UL,
		17000000UL,
		18000000UL,
		19000000UL,
		20000000UL,
		21000000UL // 40
	};

	if (level < 0 || level >= GUILD_LEVEL_MAX)
		return 0;

	return INTERNATIONAL_GUILDEXP_LIST[level];
}

int LocaleService_GetSkillPower(unsigned level)
{
#ifdef ENABLE_SAGE_SKILL // ENABLE_EXPERT_SKILL
	static const unsigned SKILL_POWER_NUM = 60+10+10+10;
#else
	static const unsigned SKILL_POWER_NUM = 50;
#endif

	if (level >= SKILL_POWER_NUM)
		return 0;

	// 0 5 6 8 10 12 14 16 18 20 22 24 26 28 30 32 34 36 38 40 50 52 54 56 58 60 63 66 69 72 82 85 88 91 94 98 102 106 110 115 125 125 125 125 125
	static unsigned INTERNATIONAL_SKILL_POWERS[SKILL_POWER_NUM] =
	{
		0,
			5,  6,  8, 10, 12,
			14, 16, 18, 20, 22,
			24, 26, 28, 30, 32,
			34, 36, 38, 40, 50, // master
			52, 54, 56, 58, 60,
			63, 66, 69, 72, 82, // grand_master
			85, 88, 91, 94, 98,
			102,106,110,115,125,// perfect_master
#ifdef ENABLE_SAGE_SKILL // ENABLE_EXPERT_SKILL
			130, 135, 146, 162, 169, 176, 184, 193, 203, 215,// sage_master
			220, 227, 235, 244, 254, 265, 277, 290, 304, 319,// expert_master
			330, 340, 350, 360, 370, 380, 390, 400, 410, 420,// destansi
			435, 450, 465, 480, 495, 510, 525, 540, 555, 570,// tanrisal
			585,
#else
			125,
#endif
	};
	return INTERNATIONAL_SKILL_POWERS[level];
}

const char* LocaleService_GetName() { return "EUROPE"; }
unsigned int LocaleService_GetCodePage() { return MULTI_LOCALE_CODE; }
const char* LocaleService_GetLocaleName() { return MULTI_LOCALE_NAME; }
const char* LocaleService_GetLocalePath() { return MULTI_LOCALE_PATH; }

void LocaleService_ForceSetLocale(const char* name, const char* localePath)
{
	strcpy(MULTI_LOCALE_NAME, name);
	strcpy(MULTI_LOCALE_PATH, localePath);
}

bool LocaleService_LoadGlobal(HINSTANCE hInstance)
{
	return false;
}