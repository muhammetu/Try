#pragma once

#ifndef __INC_METIN_II_BATTLE_PASS_H__
#define __INC_METIN_II_BATTLE_PASS_H__

#include <boost/unordered_map.hpp>

#include "../../common/stl.h"
#include "../../common/length.h"
#include "../../common/tables.h"
#include "group_text_parse_tree.h"

#include "packet.h"

class CBattlePass : public singleton<CBattlePass>
{
public:
	CBattlePass();
	virtual ~CBattlePass();

	bool ReadBattlePassFile();
	bool ReadBattlePassGroup();
	bool ReadBattlePassMissions();

	BYTE GetMissionTypeByName(std::string stMissionName);
	std::string GetMissionNameByType(BYTE bType);
	std::string GetBattlePassNameByID(BYTE bID);
	bool IsBattlePassPremiumByID(BYTE bID);

	void GetMissionSearchName(BYTE bMissionType, std::string*, std::string*);

	void BattlePassRequestOpen(LPCHARACTER pkChar);
	bool BattlePassMissionGetInfo(BYTE bBattlePassId, BYTE bMissionType, DWORD* dwFirstInfo, DWORD* dwSecondInfo);
	void BattlePassRewardMission(LPCHARACTER pkChar, DWORD bMissionType, DWORD bBattlePassId);

	void BattlePassRequestReward(LPCHARACTER pkChar);
	bool CanUseBattlePass(LPCHARACTER pkChar, BYTE bBattlePassID);
	void BattlePassRequestSkip(LPCHARACTER pkChar, BYTE bMissionID);
	void BattlePassReward(LPCHARACTER pkChar);

private:
	CGroupTextParseTreeLoader* m_pLoader;

	typedef std::map <BYTE, std::string> TMapBattlePassName;
	typedef std::map <std::string, std::vector<TBattlePassRewardItem>> TMapBattlePassReward;
	typedef std::map <std::string, std::vector<TBattlePassMissionInfo>> TMapBattleMissionInfo;

	TMapBattlePassName m_map_battle_pass_name;
	TMapBattlePassReward m_map_battle_pass_reward;
	TMapBattleMissionInfo m_map_battle_pass_mission_info;
};

#endif
