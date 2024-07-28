#include "stdafx.h"
#include "battle_pass.h"

#include "p2p.h"
#include "locale_service.h"
#include "char.h"
#include "desc_client.h"
#include "desc_manager.h"
#include "buffer_manager.h"
#include "packet.h"
#include "questmanager.h"
#include "questlua.h"
#include "start_position.h"
#include "char_manager.h"
#include "item_manager.h"
#include "sectree_manager.h"
#include "regen.h"
#include "log.h"
#include "db.h"
#include "target.h"
#include "party.h"

#include <string>
#include <algorithm>

const std::string g_astMissionType[MISSION_TYPE_MAX] = {
	"",
	"MONSTER_KILL",
	"STONE_KILL",
	"BOSS_KILL",
	"BETA_BOSS_KILL",
	"PLAYER_KILL",
	"MONSTER_DAMAGE",
	"PLAYER_DAMAGE",
	"USE_ITEM",
	"SELL_ITEM",
	"CRAFT_ITEM",
	"REFINE_ITEM",
	"DESTROY_ITEM",
	"COLLECT_ITEM",
	"FRY_FISH",
	"CATCH_FISH",
	"SPENT_YANG",
	"FARM_YANG",
	"COMPLETE_LOW_DUNGEON",
	"COMPLETE_HIGH_DUNGEON",
	"COMPLETE_MINIGAME",
	"SHOUT",
	"PLAYTIME",
	"METIN_KILL1",
	"METIN_KILL2",
	"METIN_KILL3",
	"METIN_KILL4",
	"METIN_KILL5",
	"METIN_KILL6",
	"METIN_KILL7",
	"METIN_KILL8",
	"METIN_KILL9",
	"METIN_KILL10",
	"METIN_KILL11",
	"METIN_KILL12",
	"METIN_KILL13",
	"METIN_KILL14",
	"METIN_KILL15",
	"METIN_KILL16",
	"METIN_KILL17",
	"METIN_KILL18",
	"METIN_KILL19",
	"METIN_KILL20",
	"METIN_KILL21",
	"METIN_KILL22",
	"METIN_KILL23",
	"METIN_KILL24",
	"METIN_KILL25",
	"METIN_KILL26",
	"METIN_KILL27",
	"METIN_KILL28",
	"METIN_KILL29",
	"METIN_KILL30",
	"METIN_KILL31",
	"METIN_KILL32",
	"METIN_KILL33",
	"METIN_KILL34",
	"METIN_KILL35",
	"METIN_KILL36",
	"METIN_KILL37",
	"METIN_KILL38",
	"METIN_KILL39",
	"METIN_KILL40",
	"METIN_KILL41",
	"METIN_KILL42",
	"METIN_KILL43",
	"METIN_KILL44",
	"METIN_KILL45",
	"METIN_KILL46",
	"METIN_KILL47",
	"METIN_KILL48",
	"METIN_KILL49",
	"METIN_KILL50",
	"DUNGEON_DONE1",
	"DUNGEON_DONE2",
	"DUNGEON_DONE3",
	"DUNGEON_DONE4",
	"DUNGEON_DONE5",
	"DUNGEON_DONE6",
	"DUNGEON_DONE7",
	"DUNGEON_DONE8",
	"DUNGEON_DONE9",
	"DUNGEON_DONE10",
	"DUNGEON_DONE11",
	"DUNGEON_DONE12",
	"DUNGEON_DONE13",
	"DUNGEON_DONE14",
	"DUNGEON_DONE15",
	"DUNGEON_DONE16",
	"DUNGEON_DONE17",
	"DUNGEON_DONE18",
	"DUNGEON_DONE19",
	"DUNGEON_DONE20",
	"DUNGEON_DONE21",
	"DUNGEON_DONE22",
	"DUNGEON_DONE23",
	"DUNGEON_DONE24",
	"DUNGEON_DONE25",
	"DUNGEON_DONE26",
	"DUNGEON_DONE27",
	"DUNGEON_DONE28",
	"DUNGEON_DONE29",
	"DUNGEON_DONE30",
	"MONSTER_KILL1",
	"MONSTER_KILL2",
	"MONSTER_KILL3",
	"MONSTER_KILL4",
	"MONSTER_KILL5",
	"MONSTER_KILL6",
	"MONSTER_KILL7",
	"MONSTER_KILL8",
	"MONSTER_KILL9",
	"MONSTER_KILL10",
	"MONSTER_KILL11",
	"MONSTER_KILL12",
	"MONSTER_KILL13",
	"MONSTER_KILL14",
	"MONSTER_KILL15",
	"MONSTER_KILL16",
	"MONSTER_KILL17",
	"MONSTER_KILL18",
	"MONSTER_KILL19",
	"MONSTER_KILL20",
	"MONSTER_KILL21",
	"MONSTER_KILL22",
	"MONSTER_KILL23",
	"MONSTER_KILL24",
	"MONSTER_KILL25",
	"MONSTER_KILL26",
	"MONSTER_KILL27",
	"MONSTER_KILL28",
	"MONSTER_KILL29",
	"MONSTER_KILL30",
	"PLAY_TIME1",
	"PLAY_TIME2",
	"PLAY_TIME3",
	"PLAY_TIME4",
	"PLAY_TIME5",
	"PLAY_TIME6",
	"PLAY_TIME7",
	"PLAY_TIME8",
	"PLAY_TIME9",
	"PLAY_TIME10",
	"PLAY_TIME11",
	"PLAY_TIME12",
	"PLAY_TIME13",
	"PLAY_TIME14",
	"PLAY_TIME15",
	"PLAY_TIME16",
	"PLAY_TIME17",
	"PLAY_TIME18",
	"PLAY_TIME19",
	"PLAY_TIME20",
	"PLAY_TIME21",
	"PLAY_TIME22",
	"PLAY_TIME23",
	"PLAY_TIME24",
	"PLAY_TIME25",
	"PLAY_TIME26",
	"PLAY_TIME27",
	"PLAY_TIME28",
	"PLAY_TIME29",
	"PLAY_TIME30",
	"FISH_DESTROY1",
	"FISH_DESTROY2",
	"FISH_DESTROY3",
	"FISH_DESTROY4",
	"FISH_DESTROY5",
	"FISH_DESTROY6",
	"FISH_DESTROY7",
	"FISH_DESTROY8",
	"FISH_DESTROY9",
	"FISH_DESTROY10",
	"FISH_DESTROY11",
	"FISH_DESTROY12",
	"FISH_DESTROY13",
	"FISH_DESTROY14",
	"FISH_DESTROY15",
	"FISH_DESTROY16",
	"FISH_DESTROY17",
	"FISH_DESTROY18",
	"FISH_DESTROY19",
	"FISH_DESTROY20",
	"FISH_DESTROY21",
	"FISH_DESTROY22",
	"FISH_DESTROY23",
	"FISH_DESTROY24",
	"FISH_DESTROY25",
	"FISH_DESTROY26",
	"FISH_DESTROY27",
	"FISH_DESTROY28",
	"FISH_DESTROY29",
	"FISH_DESTROY30",
	"FISH_DESTROY31",
	"FISH_DESTROY32",
	"FISH_DESTROY33",
	"FISH_DESTROY34",
	"FISH_DESTROY35",
	"FISH_DESTROY36",
	"FISH_DESTROY37",
	"FISH_DESTROY38",
	"FISH_DESTROY39",
	"FISH_DESTROY40",
	"FISH_DESTROY41",
	"FISH_DESTROY42",
	"FISH_DESTROY43",
	"FISH_DESTROY44",
	"FISH_DESTROY45",
	"FISH_DESTROY46",
	"FISH_DESTROY47",
	"FISH_DESTROY48",
	"FISH_DESTROY49",
	"FISH_DESTROY50",
	"KAFA_DESTROY1",
	"KAFA_DESTROY2",
	"KAFA_DESTROY3",
	"KAFA_DESTROY4",
	"KAFA_DESTROY5",
	"KAFA_DESTROY6",
	"KAFA_DESTROY7",
	"KAFA_DESTROY8",
	"KAFA_DESTROY9",
	"KAFA_DESTROY10",
	"SEND_SHOUT1",
	"SEND_SHOUT2",
	"SEND_SHOUT3",
	"SEND_SHOUT4",
	"SEND_SHOUT5",
	"SEND_SHOUT6",
	"SEND_SHOUT7",
	"SEND_SHOUT8",
	"SEND_SHOUT9",
	"SEND_SHOUT10",
	"SEND_SHOUT11",
	"SEND_SHOUT12",
	"SEND_SHOUT13",
	"SEND_SHOUT14",
	"SEND_SHOUT15",
	"SEND_SHOUT16",
	"SEND_SHOUT17",
	"SEND_SHOUT18",
	"SEND_SHOUT19",
	"SEND_SHOUT20",
	"SEND_SHOUT21",
	"SEND_SHOUT22",
	"SEND_SHOUT23",
	"SEND_SHOUT24",
	"SEND_SHOUT25",
	"SEND_SHOUT26",
	"SEND_SHOUT27",
	"SEND_SHOUT28",
	"SEND_SHOUT29",
	"SEND_SHOUT30",
	"SEND_SHOUT31",
};

CBattlePass::CBattlePass()
{
	m_pLoader = NULL;
}

CBattlePass::~CBattlePass()
{
	if (m_pLoader)
		delete m_pLoader;
}

bool CBattlePass::ReadBattlePassFile()
{
	char szBattlePassFileName[256];
	snprintf(szBattlePassFileName, sizeof(szBattlePassFileName), "%s/battle_pass.txt", LocaleService_GetBasePath().c_str());

	m_pLoader = new CGroupTextParseTreeLoader;
	CGroupTextParseTreeLoader& loader = *m_pLoader;

	if (false == loader.Load(szBattlePassFileName))
	{
		sys_err("battle_pass.txt erreur de lecture");
		return false;
	}

	if (!ReadBattlePassGroup())
		return false;

	if (!ReadBattlePassMissions())
		return false;

	return true;
}

bool CBattlePass::ReadBattlePassGroup()
{
	std::string stName;

	CGroupNode* pGroupNode = m_pLoader->GetGroup("battlepass");

	if (NULL == pGroupNode)
	{
		sys_err("battle_pass.txt a besoin d'un groupe de battle pass.");
		return false;
	}

	int n = pGroupNode->GetRowCount();
	if (0 == n)
	{
		sys_err("Le group battle pass est vide.");
		return false;
	}

	std::set<BYTE> setIDs;

	for (int i = 0; i < n; i++)
	{
		const CGroupNode::CGroupNodeRow* pRow;
		pGroupNode->GetRow(i, &pRow);

		std::string stBattlePassName;
		BYTE battlePassId;

		if (!pRow->GetValue("battlepassname", stBattlePassName))
		{
			sys_err("In Group BattlePass, No BattlePassName column.");
			return false;
		}

		if (!pRow->GetValue("battlepassid", battlePassId))
		{
			sys_err("In Group BattlePass, %s's ID is invalid", stBattlePassName.c_str());
			return false;
		}

		if (setIDs.end() != setIDs.find(battlePassId))
		{
			sys_err("In Group BattlePass, duplicated id exist.");
			return false;
		}

		setIDs.insert(battlePassId);

		m_map_battle_pass_name.insert(TMapBattlePassName::value_type(battlePassId, stBattlePassName));
	}

	return true;
}

bool CBattlePass::ReadBattlePassMissions()
{
	TMapBattlePassName::iterator it = m_map_battle_pass_name.begin();
	while (it != m_map_battle_pass_name.end())
	{
		std::string battlePassName = (it++)->second;

		CGroupNode* pGroupNode = m_pLoader->GetGroup(battlePassName.c_str());

		if (NULL == pGroupNode)
		{
			sys_err("battle_pass.txt need group %s.", battlePassName.c_str());
			return false;
		}

		int n = pGroupNode->GetChildNodeCount();
		if (n < 2)
		{
			sys_err("Group %s need to have at least one grup for Reward and one Mission. Row: %d", battlePassName.c_str(), n);
			return false;
		}

		{
			CGroupNode* pChild;
			if (NULL == (pChild = pGroupNode->GetChildNode("reward")))
			{
				sys_err("In Group %s, Reward group is not defined.", battlePassName.c_str());
				return false;
			}

			int m = pChild->GetRowCount();
			std::vector<TBattlePassRewardItem> rewardVector;

			for (int j = 1; j <= m; j++)
			{
				std::stringstream ss;
				ss << j;
				const CGroupNode::CGroupNodeRow* pRow = NULL;

				pChild->GetRow(ss.str(), &pRow);
				if (NULL == pRow)
				{
					sys_err("In Group %s, subgroup Reward, No %d row.", battlePassName.c_str(), j);
					return false;
				}

				TBattlePassRewardItem itemReward;

				if (!pRow->GetValue("itemvnum", itemReward.dwVnum))
				{
					sys_err("In Group %s, subgroup Reward, ItemVnum is empty.", battlePassName.c_str());
					return false;
				}

				if (!pRow->GetValue("itemcount", itemReward.bCount))
				{
					sys_err("In Group %s, subgroup Reward, ItemCount is empty.", battlePassName.c_str());
					return false;
				}

				rewardVector.push_back(itemReward);
			}

			m_map_battle_pass_reward.insert(TMapBattlePassReward::value_type(battlePassName.c_str(), rewardVector));
		}

		std::vector<TBattlePassMissionInfo> missionInfoVector;

		for (int i = 1; i < n; i++)
		{
			std::stringstream ss;
			ss << "mission_" << i;

			CGroupNode* pChild;
			if (NULL == (pChild = pGroupNode->GetChildNode(ss.str())))
			{
				sys_err("In Group %s, %s subgroup is not defined.", battlePassName.c_str(), ss.str().c_str());
				return false;
			}

			int m = pChild->GetRowCount();

			std::string stMissionSearch[] = { "", "" };
			bool bAlreadySearched = false;
			BYTE bRewardContor = 0;
			TBattlePassMissionInfo missionInfo;
			memset(&missionInfo, 0, sizeof(TBattlePassMissionInfo));

			for (int j = 0; j < m; j++)
			{
				const CGroupNode::CGroupNodeRow* pRow = NULL;

				pChild->GetRow(j, &pRow);
				if (NULL == pRow)
				{
					sys_err("In Group %s and subgroup %s null row.", battlePassName.c_str(), ss.str().c_str());
					return false;
				}

				// InfoDesc = ItemVnum from reward
				// InfoName = ItemCount from reward

				std::string stInfoDesc;
				if (!pRow->GetValue("infodesc", stInfoDesc))
				{
					sys_err("In Group %s and subgroup %s InfoDesc does not exist.", battlePassName.c_str(), ss.str().c_str());
					return false;
				}

				if (stInfoDesc == "type")
				{
					std::string stInfoName;
					if (!pRow->GetValue("infoname", stInfoName))
					{
						sys_err("In Group %s and subgroup %s InfoName does not exist.", battlePassName.c_str(), ss.str().c_str());
						return false;
					}

					missionInfo.bMissionType = GetMissionTypeByName(stInfoName);
				}

				if (missionInfo.bMissionType <= MISSION_TYPE_NONE || missionInfo.bMissionType >= MISSION_TYPE_MAX)
				{
					sys_err("In Group %s and subgroup %s Wrong mission type: %d.", battlePassName.c_str(), ss.str().c_str(), missionInfo.bMissionType);
					return false;
				}

				if (!bAlreadySearched)
				{
					GetMissionSearchName(missionInfo.bMissionType, &stMissionSearch[0], &stMissionSearch[1]);
					bAlreadySearched = true;
				}

				for (int k = 0; k < 2; k++)
				{
					if (stMissionSearch[k] != "")
					{
						if (stInfoDesc == stMissionSearch[k])
						{
							if (!pRow->GetValue("infoname", missionInfo.dwMissionInfo[k]))
							{
								sys_err("In Group %s and subgroup %s InfoDesc %s InfoName does not exist.", battlePassName.c_str(), ss.str().c_str(), stMissionSearch[k].c_str());
								return false;
							}

							sys_log(0, "BattlePassInfo: Group %s // Subgroup %s // InfoName %s // InfoValue %d",
								battlePassName.c_str(), ss.str().c_str(), stMissionSearch[k].c_str(), missionInfo.dwMissionInfo[k]);

							stMissionSearch[k] = "";
						}
					}
				}

				if (bRewardContor >= MISSION_REWARD_COUNT)
				{
					sys_err("In Group %s and subgroup %s More than 3 rewards.", battlePassName.c_str(), ss.str().c_str());
					return false;
				}

				if (isdigit(*stInfoDesc.c_str()))
				{
					DWORD dwVnum = atoi(stInfoDesc.c_str());
					DWORD bCount = 1;

					if (!pRow->GetValue("infoname", bCount))
					{
						sys_err("In Group %s and subgroup %s Wrong ItemCount.", battlePassName.c_str(), ss.str().c_str());
						return false;
					}

					missionInfo.aRewardList[bRewardContor].dwVnum = dwVnum;
					missionInfo.aRewardList[bRewardContor].bCount = bCount;
					bRewardContor++;
				}

				if (stInfoDesc == "dragoncoin")
				{
					DWORD dwSkipCost;
					if (!pRow->GetValue("infoname", dwSkipCost))
					{
						sys_err("In Group %s and subgroup %s InfoName does not exist.", battlePassName.c_str(), ss.str().c_str());
						return false;
					}

					missionInfo.dwSkipCost = dwSkipCost;
				}
			}

			missionInfoVector.push_back(missionInfo);
		}

		m_map_battle_pass_mission_info.insert(TMapBattleMissionInfo::value_type(battlePassName.c_str(), missionInfoVector));
	}

	return true;
}

BYTE CBattlePass::GetMissionTypeByName(std::string stMissionName)
{
	for (int i = 0; i < MISSION_TYPE_MAX; i++)
	{
		if (g_astMissionType[i] == stMissionName)
			return i;
	}

	return 0;
}

std::string CBattlePass::GetMissionNameByType(BYTE bType)
{
	for (int i = 0; i < MISSION_TYPE_MAX; i++)
	{
		if (i == bType)
			return g_astMissionType[i];
	}

	return "";
}

std::string CBattlePass::GetBattlePassNameByID(BYTE bID)
{
	TMapBattlePassName::iterator it = m_map_battle_pass_name.find(bID);

	if (it == m_map_battle_pass_name.end())
	{
		return "";
	}

	return it->second;
}

bool CBattlePass::IsBattlePassPremiumByID(BYTE bID)
{
	std::string passName = GetBattlePassNameByID(bID);
	if (passName.find("premium") != std::string::npos)
		return true;
	
	return false;
}

void CBattlePass::GetMissionSearchName(BYTE bMissionType, std::string* st_name_1, std::string* st_name_2)
{
#ifdef ENABLE_BATTLE_PASS_EX
	if (bMissionType >= METIN_KILL1 && bMissionType <= METIN_KILL50)
	{
		*st_name_1 = "vnum";
		*st_name_2 = "count";
		return;
	}
	else if (bMissionType >= DUNGEON_DONE1 && bMissionType <= DUNGEON_DONE30)
	{
		*st_name_1 = "vnum";
		*st_name_2 = "count";
		return;
	}
	else if (bMissionType >= MONSTER_KILL1 && bMissionType <= MONSTER_KILL30)
	{
		*st_name_1 = "vnum";
		*st_name_2 = "count";
		return;
	}
	else if (bMissionType >= PLAY_TIME1 && bMissionType <= PLAY_TIME30)
	{
		*st_name_1 = "vnum";
		*st_name_2 = "count";
		return;
	}
	else if (bMissionType >= FISH_DESTROY1 && bMissionType <= FISH_DESTROY50)
	{
		*st_name_1 = "vnum";
		*st_name_2 = "count";
		return;
	}
	else if (bMissionType >= KAFA_DESTROY1 && bMissionType <= KAFA_DESTROY10)
	{
		*st_name_1 = "vnum";
		*st_name_2 = "count";
		return;
	}
	else if (bMissionType >= SEND_SHOUT1 && bMissionType <= SEND_SHOUT31)
	{
		*st_name_1 = "vnum";
		*st_name_2 = "count";
		return;
	}
#endif // ENABLE_BATTLE_PASS_EX
	switch (bMissionType)
	{
	case MONSTER_KILL:
	case STONE_KILL:
	case BOSS_KILL:
	case BETA_BOSS_KILL:
	case USE_ITEM:
	case SELL_ITEM:
	case CRAFT_ITEM:
	case REFINE_ITEM:
	case DESTROY_ITEM:
	case COLLECT_ITEM:
		*st_name_1 = "vnum";
		*st_name_2 = "count";
		break;

	case PLAYER_KILL:
		*st_name_1 = "min_level";
		*st_name_2 = "count";
		break;

	case MONSTER_DAMAGE:
		*st_name_1 = "vnum";
		*st_name_2 = "value";
		break;

	case PLAYER_DAMAGE:
		*st_name_1 = "min_level";
		*st_name_2 = "value";
		break;

	case FRY_FISH:
	case CATCH_FISH:
		*st_name_1 = "vnum";
		*st_name_2 = "count";
		break;

	case SPENT_YANG:
	case FARM_YANG:
		*st_name_1 = "";
		*st_name_2 = "value";
		break;

	case COMPLETE_LOW_DUNGEON:
	case COMPLETE_HIGH_DUNGEON:
	case COMPLETE_MINIGAME:
		*st_name_1 = "id";
		*st_name_2 = "count";
		break;
	
	case SHOUT:
		*st_name_1 = "";
		*st_name_2 = "count";
		break;

	case PLAYTIME:
		*st_name_1 = "";
		*st_name_2 = "value";
		break;

	default:
		*st_name_1 = "";
		*st_name_2 = "";
		break;
	}
}

void CBattlePass::BattlePassRequestOpen(LPCHARACTER pkChar)
{
	if (!pkChar)
	{
		sys_err("pkChar null");
		return;
	}

	if (!pkChar->GetDesc())
	{
		sys_err("pkChar GetDesc null");
		return;
	}

	if (!pkChar->IsLoadedBattlePass())
	{
		pkChar->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("BATTLE_PASS_NOT_LOADED_YET"));
		return;
	}

	BYTE bBattlePassId = pkChar->GetBattlePassId();
	if (!bBattlePassId)
	{
		pkChar->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("BATTLE_PASS_CANT_FIND"));
		return;
	}

	TMapBattlePassName::iterator it = m_map_battle_pass_name.find(bBattlePassId);

	if (it == m_map_battle_pass_name.end())
		return;

	std::string battlePassName = it->second;
	TMapBattleMissionInfo::iterator itInfo = m_map_battle_pass_mission_info.find(battlePassName);

	if (itInfo == m_map_battle_pass_mission_info.end())
		return;

	TMapBattlePassReward::iterator itReward = m_map_battle_pass_reward.find(battlePassName);
	if (itReward == m_map_battle_pass_reward.end())
		return;

	std::vector<TBattlePassRewardItem> rewardInfo = itReward->second;
	std::vector<TBattlePassMissionInfo> missionInfo = itInfo->second;

	for (int i = 0; i < missionInfo.size(); i++)
	{
		missionInfo[i].dwMissionInfo[2] = pkChar->GetMissionProgress(missionInfo[i].bMissionType, bBattlePassId);
	}

	if (!missionInfo.empty())
	{
		TPacketGCBattlePass packet;
		packet.bHeader = HEADER_GC_BATTLE_PASS_OPEN;
		packet.wSize = sizeof(packet) + sizeof(TBattlePassMissionInfo) * missionInfo.size();
		packet.wRewardSize = sizeof(TBattlePassRewardItem) * rewardInfo.size();

		pkChar->GetDesc()->BufferedPacket(&packet, sizeof(packet));
		pkChar->GetDesc()->BufferedPacket(&missionInfo[0], sizeof(TBattlePassMissionInfo) * missionInfo.size());
		pkChar->GetDesc()->Packet(&rewardInfo[0], sizeof(TBattlePassRewardItem) * rewardInfo.size());
	}
}

void CBattlePass::BattlePassRewardMission(LPCHARACTER pkChar, DWORD bMissionType, DWORD bBattlePassId)
{
	if (!pkChar)
		return;

	if (!pkChar->GetDesc())
		return;

	if (!bBattlePassId)
	{
		pkChar->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("BATTLE_PASS_CANT_FIND"));
		return;
	}

	TMapBattlePassName::iterator it = m_map_battle_pass_name.find(bBattlePassId);

	if (it == m_map_battle_pass_name.end())
		return;

	std::string battlePassName = it->second;
	TMapBattleMissionInfo::iterator itInfo = m_map_battle_pass_mission_info.find(battlePassName);

	if (itInfo == m_map_battle_pass_mission_info.end())
		return;

	std::vector<TBattlePassMissionInfo> missionInfo = itInfo->second;

#ifdef ENABLE_BATTLE_PASS_EX
	if (bMissionType >= METIN_KILL1 && bMissionType <= METIN_KILL50)
	{
		int realNum = bMissionType-PLAYTIME;
		int givedValue = realNum*50;

		if (pkChar->FindAffect(AFFECT_BATTLE_PASS_METIN))
			pkChar->RemoveAffect(AFFECT_BATTLE_PASS_METIN);
		
		pkChar->AddAffect(AFFECT_BATTLE_PASS_METIN, POINT_ATTBONUS_STONE, givedValue, AFF_NONE, INFINITE_AFFECT_DURATION, 0, true);
	}
	else if (bMissionType >= DUNGEON_DONE1 && bMissionType <= DUNGEON_DONE30)
	{
		int realNum = bMissionType-METIN_KILL50;
		int givedValue = realNum*50;

		if (pkChar->FindAffect(AFFECT_BATTLE_PASS_BOSS))
			pkChar->RemoveAffect(AFFECT_BATTLE_PASS_BOSS);
		
		pkChar->AddAffect(AFFECT_BATTLE_PASS_BOSS, POINT_ATTBONUS_BOSS, givedValue, AFF_NONE, INFINITE_AFFECT_DURATION, 0, true);
	}
	else if (bMissionType >= MONSTER_KILL1 && bMissionType <= MONSTER_KILL30)
	{
		int realNum = bMissionType-DUNGEON_DONE30;
		int givedValue = realNum*50;

		if (pkChar->FindAffect(AFFECT_BATTLE_PASS_MONSTER))
			pkChar->RemoveAffect(AFFECT_BATTLE_PASS_MONSTER);
		
		pkChar->AddAffect(AFFECT_BATTLE_PASS_MONSTER, POINT_ATTBONUS_MONSTER, givedValue, AFF_NONE, INFINITE_AFFECT_DURATION, 0, true);
	}
	else if (bMissionType >= PLAY_TIME1 && bMissionType <= PLAY_TIME30)
	{
		int realNum = bMissionType-MONSTER_KILL30;
		int givedValue = realNum*75;

		if (pkChar->FindAffect(AFFECT_BATTLE_PASS_GUCLENDIRME))
			pkChar->RemoveAffect(AFFECT_BATTLE_PASS_GUCLENDIRME);
		
		pkChar->AddAffect(AFFECT_BATTLE_PASS_GUCLENDIRME, POINT_ATT_GRADE_BONUS, givedValue, AFF_NONE, INFINITE_AFFECT_DURATION, 0, true);
	}
	else if (bMissionType >= FISH_DESTROY1 && bMissionType <= FISH_DESTROY50)
	{
		int realNum = bMissionType-PLAY_TIME30;
		int givedValue = realNum*5;

		if (pkChar->FindAffect(AFFECT_BATTLE_PASS_BERSERKER_2))
			pkChar->RemoveAffect(AFFECT_BATTLE_PASS_BERSERKER_2);
		
		pkChar->AddAffect(AFFECT_BATTLE_PASS_BERSERKER_2, POINT_MALL_ATTBONUS, givedValue, AFF_NONE, INFINITE_AFFECT_DURATION, 0, true);
	}
	else if (bMissionType >= SEND_SHOUT1 && bMissionType <= SEND_SHOUT31)
	{
		int realNum = bMissionType-KAFA_DESTROY10;
		int givedValue = realNum*5;

		if (pkChar->FindAffect(AFFECT_BATTLE_PASS_BERSERKER))
			pkChar->RemoveAffect(AFFECT_BATTLE_PASS_BERSERKER);
		
		pkChar->AddAffect(AFFECT_BATTLE_PASS_BERSERKER, POINT_MALL_ATTBONUS, givedValue, AFF_NONE, INFINITE_AFFECT_DURATION, 0, true);
	}
	// else if (bMissionType >= KAFA_DESTROY1 && bMissionType <= KAFA_DESTROY10)
	// {
		// int realNum = bMissionType-FISH_DESTROY50;
		// int givedValue = realNum*5;

		// if (pkChar->FindAffect(AFFECT_BATTLE_PASS_BERSERKER_2))
			// pkChar->RemoveAffect(AFFECT_BATTLE_PASS_BERSERKER_2);
		
		// pkChar->AddAffect(AFFECT_BATTLE_PASS_BERSERKER_2, POINT_MALL_ATTBONUS, givedValue, AFF_NONE, INFINITE_AFFECT_DURATION, 0, true);
	// }
#endif // ENABLE_BATTLE_PASS_EX

	for (int i = 0; i < missionInfo.size(); i++)
	{
		if (missionInfo[i].bMissionType == bMissionType)
		{
			for (int j = 0; j < MISSION_REWARD_COUNT; j++)
			{
				if (missionInfo[i].aRewardList[j].dwVnum && missionInfo[i].aRewardList[j].bCount > 0)
					pkChar->AutoGiveItem(missionInfo[i].aRewardList[j].dwVnum, missionInfo[i].aRewardList[j].bCount);
			}

			break;
		}
	}
}

void CBattlePass::BattlePassRequestReward(LPCHARACTER pkChar)
{
	if (!pkChar)
		return;

	if (!pkChar->GetDesc())
		return;

	BYTE bBattlePassId = pkChar->GetBattlePassId();
	if (!bBattlePassId)
		return;

	TMapBattlePassName::iterator it = m_map_battle_pass_name.find(bBattlePassId);
	if (it == m_map_battle_pass_name.end())
		return;

	std::string battlePassName = it->second;
	TMapBattleMissionInfo::iterator itInfo = m_map_battle_pass_mission_info.find(battlePassName);

	if (itInfo == m_map_battle_pass_mission_info.end())
		return;

	std::vector<TBattlePassMissionInfo> missionInfo = itInfo->second;

	bool bIsCompleted = true;
	for (int i = 0; i < missionInfo.size(); i++)
	{
		if (!pkChar->IsCompletedMission(missionInfo[i].bMissionType, bBattlePassId))
		{
			bIsCompleted = false;
			break;
		}
	}

	if (bIsCompleted)
	{
		char buf[256] = { 0 };
		snprintf(buf, sizeof(buf), LC_TEXT("Le joueur %s a reussi avec succes le pass de combat."), pkChar->GetName());
		BroadcastNotice(buf);

		BattlePassReward(pkChar);
	}
}

bool CBattlePass::CanUseBattlePass(LPCHARACTER pkChar, BYTE bBattlePassId)
{
	if (!pkChar)
		return false;

	if (!pkChar->GetDesc())
		return false;

	TMapBattlePassName::iterator it = m_map_battle_pass_name.find(bBattlePassId);
	if (it == m_map_battle_pass_name.end())
		return false;

	std::string battlePassName = it->second;
	TMapBattleMissionInfo::iterator itInfo = m_map_battle_pass_mission_info.find(battlePassName);

	if (itInfo == m_map_battle_pass_mission_info.end())
		return false;

	std::vector<TBattlePassMissionInfo> missionInfo = itInfo->second;

	bool bIsCompleted = true;
	for (int i = 0; i < missionInfo.size(); i++)
	{
		if (!pkChar->IsCompletedMission(missionInfo[i].bMissionType, bBattlePassId))
		{
			bIsCompleted = false;
			break;
		}
		else
		{
			return false;
		}
	}

	if (bIsCompleted)
		return false;
	
	return true;
}

void CBattlePass::BattlePassRequestSkip(LPCHARACTER pkChar, BYTE bMissionID)
{
	if (!pkChar)
		return;

	if (!pkChar->GetDesc())
		return;

	BYTE bBattlePassId = pkChar->GetBattlePassId();
	if (!bBattlePassId)
	{
		pkChar->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("BATTLE_PASS_CANT_FIND"));
		return;
	}

	TMapBattlePassName::iterator it = m_map_battle_pass_name.find(bBattlePassId);
	if (it == m_map_battle_pass_name.end())
		return;

	std::string battlePassName = it->second;
	TMapBattleMissionInfo::iterator itInfo = m_map_battle_pass_mission_info.find(battlePassName);

	if (itInfo == m_map_battle_pass_mission_info.end())
		return;

	std::vector<TBattlePassMissionInfo> missionInfo = itInfo->second;

	DWORD dwSkipCost = 0;
	for (int i = 0; i < missionInfo.size(); i++)
	{
		if (missionInfo[i].bMissionType == bMissionID)
			dwSkipCost = missionInfo[i].dwSkipCost;
	}

#ifdef ENABLE_BATTLE_PASS_EX
	if (bMissionID > METIN_KILL1 && bMissionID <= METIN_KILL50)
	{
		if (!pkChar->IsCompletedMission(bMissionID-1, bBattlePassId))
		{
			pkChar->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("BATTLE_PASS_SKIP_CANNOT"));
			return;
		}
	}
	else if (bMissionID > DUNGEON_DONE1 && bMissionID <= DUNGEON_DONE30)
	{
		if (!pkChar->IsCompletedMission(bMissionID-1, bBattlePassId))
		{
			pkChar->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("BATTLE_PASS_SKIP_CANNOT"));
			return;
		}
	}
	else if (bMissionID > MONSTER_KILL1 && bMissionID <= MONSTER_KILL30)
	{
		if (!pkChar->IsCompletedMission(bMissionID-1, bBattlePassId))
		{
			pkChar->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("BATTLE_PASS_SKIP_CANNOT"));
			return;
		}
	}
	else if (bMissionID > PLAY_TIME1 && bMissionID <= PLAY_TIME30)
	{
		if (!pkChar->IsCompletedMission(bMissionID-1, bBattlePassId))
		{
			pkChar->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("BATTLE_PASS_SKIP_CANNOT"));
			return;
		}
	}
	else if (bMissionID > FISH_DESTROY1 && bMissionID <= FISH_DESTROY50)
	{
		if (!pkChar->IsCompletedMission(bMissionID-1, bBattlePassId))
		{
			pkChar->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("BATTLE_PASS_SKIP_CANNOT"));
			return;
		}
	}
	else if (bMissionID > KAFA_DESTROY1 && bMissionID <= KAFA_DESTROY10)
	{
		if (!pkChar->IsCompletedMission(bMissionID-1, bBattlePassId))
		{
			pkChar->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("BATTLE_PASS_SKIP_CANNOT"));
			return;
		}
	}
	else if (bMissionID > SEND_SHOUT1 && bMissionID <= SEND_SHOUT31)
	{
		if (!pkChar->IsCompletedMission(bMissionID-1, bBattlePassId))
		{
			pkChar->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("BATTLE_PASS_SKIP_CANNOT"));
			return;
		}
	}
#endif // ENABLE_BATTLE_PASS_EX

	if (pkChar->GetDragonCoin() < dwSkipCost)
	{
		pkChar->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("BATTLE_PASS_SKIP_COST_IS_NOT_ENOUGH"));
		return;
	}

	if (pkChar->IsCompletedMission(bMissionID, bBattlePassId))
	{
		pkChar->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("BATTLE_PASS_ALREADY_FINISHED"));
		return;
	}

	pkChar->SkipBattlePassMission(bMissionID, bBattlePassId);
	pkChar->SetDragonCoin(pkChar->GetDragonCoin()-dwSkipCost);
}

void CBattlePass::BattlePassReward(LPCHARACTER pkChar)
{
	if (!pkChar)
		return;

	if (!pkChar->GetDesc())
		return;

	BYTE bBattlePassId = pkChar->GetBattlePassId();
	if (!bBattlePassId)
	{
		pkChar->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("BATTLE_PASS_CANT_FIND"));
		return;
	}

	TMapBattlePassName::iterator it = m_map_battle_pass_name.find(bBattlePassId);

	if (it == m_map_battle_pass_name.end())
		return;

	std::string battlePassName = it->second;
	TMapBattlePassReward::iterator itReward = m_map_battle_pass_reward.find(battlePassName);
	if (itReward == m_map_battle_pass_reward.end())
		return;
	
	if (IsBattlePassPremiumByID(bBattlePassId))
	{
		pkChar->RemoveAffect(AFFECT_BATTLE_PASS_PREMIUM);
	}
	else
	{
		pkChar->RemoveAffect(AFFECT_BATTLE_PASS);
	}

	pkChar->ResetBattlePassEndTime();

	TBattlePassRegisterRanking ranking;
	ranking.bBattlePassId = bBattlePassId;
	strlcpy(ranking.playerName, pkChar->GetName(), sizeof(ranking.playerName));
	db_clientdesc->DBPacket(HEADER_GD_REGISTER_BP_RANKING, 0, &ranking, sizeof(TBattlePassRegisterRanking));
}

bool CBattlePass::BattlePassMissionGetInfo(BYTE bBattlePassId, BYTE bMissionType, DWORD* dwFirstInfo, DWORD* dwSecondInfo)
{
	TMapBattlePassName::iterator it = m_map_battle_pass_name.find(bBattlePassId);
	if (it == m_map_battle_pass_name.end())
		return false;

	std::string battlePassName = it->second;
	TMapBattleMissionInfo::iterator itInfo = m_map_battle_pass_mission_info.find(battlePassName);

	if (itInfo == m_map_battle_pass_mission_info.end())
		return false;

	std::vector<TBattlePassMissionInfo> missionInfo = itInfo->second;

	for (int i = 0; i < missionInfo.size(); i++)
	{
		if (missionInfo[i].bMissionType == bMissionType)
		{
			*dwFirstInfo = missionInfo[i].dwMissionInfo[0];
			*dwSecondInfo = missionInfo[i].dwMissionInfo[1];
			return true;
		}
	}

	return false;
}
