#pragma once

struct DungeonInfoTable {
	std::string quest_name;
	int type;
	int organization;
	int min_level;
	int party_members;
	int map_index;
	std::string map_name;
	std::string map_entrance;
	int map_coords[2];
	int cooldown;
	int duration;
	int max_level;
	int strength_bonus;
	int resistance_bonus;
	int item_vnum;
	int boss_vnum;
};

extern std::vector<DungeonInfoTable> dungeonTable;

class DungeonInfo : public singleton<DungeonInfo>
{
public:
	DungeonInfo();
	~DungeonInfo();

	bool	WarpPC(LPCHARACTER ch, BYTE dungeonIndex);
	void	Update(LPCHARACTER ch);
	bool	Load();
};