#include "stdafx.h"
#include "constants.h"
#include "utils.h"
#include "desc.h"
#include "char.h"
#include "db.h"
#include "config.h"
#include "dungeon_info.h"
#include "affect.h"
#include "item.h"
#include "questmanager.h"
#include <boost/algorithm/string.hpp>
#include "locale_service.h"

std::vector<DungeonInfoTable> dungeonTable = {};
// {"UnderwaterDungeon_zone",		1,	0,	110,	0,	11,		"Sualtý Maðarasý ",		"Zindanlar Kalesi",	{1792 + 346, 0 + 325},	4*60*60,	1*60*60,	120,	118,	120,	99314,	987},
// {"infected_garden",				1,	0,	120,	0,	28,		"Enfeksiyonlu Bahçe ",	"Zindanlar Kalesi",	{2850, 8447},			6 * 60 * 60,	2 * 60 * 60,	120,	118,	120,	99328,	472},
// {"Easter2020Dungeon_zone",		1,	0,	105,	0,	55,		"Tavþan Deliði ",		"Zindanlar Kalesi",	{2339, 2301},			3*60*60,	1*60*60,	120,	104,	38,		99309,	4103},
// {"ValentineDungeon_zone",		1,	0,	105,	0,	52,		"Tatlý Orman ",			"Zindanlar Kalesi",	{2339, 2301},			3*60*60,	1*60*60,	120,	118,	120,	99311,	4089},
// {"Christmas2019Dungeon_zone",	1,	0,	110,	0,	39,		"Harikalar Diyarý ",	"Zindanlar Kalesi",	{6439, 4095},			4*60*60,	1*60*60,	120,	118,	120,	99312,	4081},
// {"devil_tower2",					1,	0,	110,	0,	10,		"Yeni Þeytan Kulesi ",	"Zindanlar Kalesi",	{2149, 286},			4*60*60,	1*60*60,	120,	118,	120,	99313,	816},
// {"plague_dungeon",				1,	0,	115,	0,	30,		"Veba Zindaný ",		"Zindanlar Kalesi",	{2408, 180},			4*60*60,	1*60*60,	120,	118,	120,	99316,	1206},
// {"v2019_01",						1,	0,	115,	0,	20,		"Aþýklar Maðarasý ",	"Zindanlar Kalesi",	{1792 + 381, 0 + 393},	4*60*60,	1*60*60,	120,	118,	120,	99317,	765},
// {"slime_dungeon",				1,	0,	70,		0,	27,		"Balçýk Maðarasý ",		"Zindanlar Kalesi",	{2850, 8447},			5*60*60,	1*60*60,	120,	118,	120,	99319,	768},
// {"easter2019_dungeon",			1,	0,	120,	0,	29,		"Tavþan Adasý ",		"Zindanlar Kalesi",	{2850, 8447},			5*60*60,	1*60*60,	120,	118,	120,	99323,	487},
// {"summer2019_dungeon",			1,	0,	120,	0,	32,		"Yaz Zindaný ",			"Zindanlar Kalesi",	{2408, 140},			6*60*60,	2*60*60,	120,	118,	120,	99324,	4024},

DungeonInfo::DungeonInfo()
{
}

DungeonInfo::~DungeonInfo()
{
}

bool DungeonInfo::WarpPC(LPCHARACTER ch, BYTE dungeonIndex)
{
	if (dungeonIndex > dungeonTable.size()-1)
		return false;

	auto entry = dungeonTable.at(dungeonIndex);

	return ch->WarpSet(entry.map_coords[0]*100, entry.map_coords[1]*100);
}

void DungeonInfo::Update(LPCHARACTER ch)
{
	if (ch == NULL)
		return;

	if (!ch->IsPC())
		return;

	ch->ChatPacket(CHAT_TYPE_COMMAND, "CleanDungeonInfo");
	for (auto &&i : dungeonTable)
	{
		if (i.type == 5)
			continue;

		std::string rejoinTimeName(i.quest_name);
		rejoinTimeName += '.'; rejoinTimeName += "rejoin_time";

		std::string mapName(i.map_name);
		boost::algorithm::replace_all(mapName, " ", "_");

		int rejoinTime = ch->GetQuestFlag(rejoinTimeName);
		// if (rejoinTime == 0 && i.type == 3)
		if (i.type == 3)
		{
			static const int duration_type = 3;
			static const int time_limit = 3;

			char value[256];
			strcpy(value, ch->GetDesc()->GetMachineGuid());

			char value2[256];
			strcpy(value2, ch->GetDesc()->GetCPUID());

			char szQuery[1024];
			snprintf(szQuery, sizeof(szQuery), "SELECT UNIX_TIMESTAMP(date) FROM player.dungeon_duration WHERE type = %d and value = '%s' and value2 = '%s' and dungeon_name = '%s' and date > NOW() ORDER BY DATE", duration_type, value, value2, i.quest_name.c_str());
			std::unique_ptr<SQLMsg> pmsg(DBManager::instance().DirectQuery(szQuery));

			if (pmsg->Get()->uiNumRows >= time_limit)
			{
				MYSQL_ROW row = mysql_fetch_row(pmsg->Get()->pSQLResult);
				str_to_number(rejoinTime, row[0]);
			}
			else
			{
				rejoinTime = 0;
			}
		}

		std::string mapEntrance;
		if (quest::CQuestManager::instance().GetEventFlag(i.quest_name))
			mapEntrance += i.map_entrance;
		else
			mapEntrance += "Aktif Degil";
		boost::algorithm::replace_all(mapEntrance, " ", "_");

		char pck[1024];
		snprintf(pck, sizeof(pck), "UpdateDungeonInfo %d %d %d %d %d " "%s %s " "%d %d %d %d %d " "%d %d %d %d %d ",
			i.type,
			i.organization,
			i.min_level,
			i.party_members,
			i.map_index,

			mapName.c_str(),
			mapEntrance.c_str(),

			i.map_coords[0],
			i.map_coords[1],
			i.cooldown,
			i.duration,
			rejoinTime,

			i.max_level,
			i.strength_bonus,
			i.resistance_bonus,
			i.item_vnum,
			i.boss_vnum
		);

		// sys_err(pck);
		ch->ChatPacket(CHAT_TYPE_COMMAND, pck);
	}
}

bool DungeonInfo::Load()
{
	dungeonTable.clear();

	char file_name[256 + 1];
	snprintf(file_name, sizeof(file_name), "%s/dungeon_info.txt", LocaleService_GetBasePath().c_str());

	FILE* fp;

	const char* value_string;

	char	one_line[256];
	const char* delim = ",\t\r\n";
	char* v, * token_string;
	int value1, value2;

	if ((fp = fopen(file_name, "r")) == 0)
		return false;

	DungeonInfoTable data;

	while (fgets(one_line, 256, fp))
	{
		value1 = value2 = 0;

		if (one_line[0] == '#')
		{
			if (one_line[1] == '#')
				break;
			else
				continue;
		}

		token_string = strtok(one_line, delim);

		if (NULL == token_string)
			continue;

		// set value1, value2
		if ((v = strtok(NULL, delim)))
			str_to_number(value1, v);
		value_string = v;
		if ((v = strtok(NULL, delim)))
			str_to_number(value2, v);

		#define STR_BIND(val) data.val = value_string;
		#define INT_BIND(val) data.val = value1;
		#define INT2_BIND(val) \
			data.val[0] = value1; \
			data.val[1] = value2;
		TOKEN("section")
		{
			;
		}
		else TOKEN("quest_name")
		{
			STR_BIND(quest_name);
		}
		else TOKEN("type")
		{
			INT_BIND(type)
		}
		else TOKEN("organization")
		{
			INT_BIND(organization)
		}
		else TOKEN("party_members")
		{
			INT_BIND(party_members)
		}
		else TOKEN("min_level")
		{
			INT_BIND(min_level)
		}
		else TOKEN("max_level")
		{
			INT_BIND(max_level)
		}
		else TOKEN("map_index")
		{
			INT_BIND(map_index)
		}
		else TOKEN("map_name")
		{
			STR_BIND(map_name)
			data.map_name += " ";
		}
		else TOKEN("map_entrance")
		{
			STR_BIND(map_entrance)
		}
		else TOKEN("map_coords")
		{
			INT2_BIND(map_coords)
		}
		else TOKEN("cooldown")
		{
			INT_BIND(cooldown)
		}
		else TOKEN("duration")
		{
			INT_BIND(duration)
		}
		else TOKEN("strength_bonus")
		{
			INT_BIND(strength_bonus)
		}
		else TOKEN("resistance_bonus")
		{
			INT_BIND(resistance_bonus)
		}
		else TOKEN("item_vnum")
		{
			INT_BIND(item_vnum)
		}
		else TOKEN("boss_vnum")
		{
			INT_BIND(boss_vnum)
		}
		else TOKEN("end")
		{
			// TODO : check
			// if (false == FN_check_data(data))
			// {
			// 	if (test_server)
			// 		sys_log(0, "something wrong");
			// 	continue;
			// }

			dungeonTable.push_back(data);
		}
		else
		{
			sys_log(0, "DungeonInfo load cant handle token string %s", token_string);
		}

	}

	sys_log(0, "DungeonInfo load total %d", dungeonTable.size());

	fclose(fp);
	return true;
}