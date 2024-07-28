#include "stdafx.h"
#include "utils.h"
#include "config.h"
#include "char.h"
#include "item.h"
#include "item_manager.h"
#include "wiki.h"
#include "refine.h"
#include "cube.h"
#include "mob_manager.h"
#include "shop_manager.h"
#include "shop.h"
#include "regen.h"
#include <bits/stdc++.h>

std::unordered_map<std::string, CWikiManager::EMakeTypes> makeStringMap = {
	{"all", CWikiManager::WIKI_MAKE_ALL},
	{"item", CWikiManager::WIKI_MAKE_ITEM},
	{"refine", CWikiManager::WIKI_MAKE_REFINE},
	{"chest", CWikiManager::WIKI_MAKE_CHEST},
	{"cube", CWikiManager::WIKI_MAKE_CUBE},
	{"stone", CWikiManager::WIKI_MAKE_STONE},
	{"boss", CWikiManager::WIKI_MAKE_BOSS},
	{"chest_drop_info", CWikiManager::WIKI_MAKE_CHESTDROP_INFO},
	{"mob_drop_info", CWikiManager::WIKI_MAKE_MOBDROP_INFO},
	{"shop", CWikiManager::WIKI_MAKE_SHOP},
};

extern bool read_line(FILE *fp, LPREGEN regen);
extern std::vector<CUBE_RENEWAL_DATA*>	s_cube_proto;

CWikiManager::CWikiManager() : m_regenInfoFile(nullptr)
{
	system("mkdir -p ./locale/../conf/wiki");
}
CWikiManager::~CWikiManager()
{
}

void CWikiManager::HandleCommand(LPCHARACTER ch, const char* argument)
{
	char argList[6][256];
	six_arguments(argument, argList[0], 256, argList[1], 256, argList[2], 256, argList[3], 256, argList[4], 256, argList[5], 256);

	int ret = -1;

	for (auto &&i : makeStringMap)
	{
		if (argList[0] == i.first)
		{
			if (i.second == WIKI_MAKE_CHESTDROP_INFO)
			{
				std::map<DWORD, DWORD> ret_vec;
				DWORD _vnum = 0;
				DWORD _count = 0;
				str_to_number(_vnum, argList[1]);
				str_to_number(_count, argList[2]);
				CreateGMDropInfoVector(_vnum, VNUM_TYPE_CHEST, _count, ret_vec);
				ch->ChatPacket(CHAT_TYPE_INFO, "<------------------------------------------------------->");
				ch->ChatPacket(CHAT_TYPE_INFO, "%d nesnesinin %d kere kullanimi sonuclari:", _vnum, _count);
				for (auto &&ret_vec_each : ret_vec)
				{
					const auto itemTable = ITEM_MANAGER::Instance().GetTable(ret_vec_each.first);
					if (!itemTable)
						continue;
					ch->ChatPacket(CHAT_TYPE_INFO, "%s nesnesinden(vnum %d) %d adet.", itemTable->szLocaleName, ret_vec_each.first, ret_vec_each.second);
				}
				ch->ChatPacket(CHAT_TYPE_INFO, "<------------------------------------------------------->");
				ret = true;
			}
			else if (i.second == WIKI_MAKE_MOBDROP_INFO)
			{
				std::map<DWORD, DWORD> ret_vec;
				DWORD _vnum = 0;
				DWORD _count = 0;
				str_to_number(_vnum, argList[1]);
				str_to_number(_count, argList[2]);
				CreateGMDropInfoVector(_vnum, VNUM_TYPE_MOB, _count, ret_vec);
				ch->ChatPacket(CHAT_TYPE_INFO, "<------------------------------------------------------->");
				ch->ChatPacket(CHAT_TYPE_INFO, "%d mobunun %d kere kesimi sonuclari:", _vnum, _count);
				for (auto &&ret_vec_each : ret_vec)
				{
					const auto itemTable = ITEM_MANAGER::Instance().GetTable(ret_vec_each.first);
					if (!itemTable)
						continue;
					ch->ChatPacket(CHAT_TYPE_INFO, "%s nesnesinden(vnum %d) %d adet.", itemTable->szLocaleName, ret_vec_each.first, ret_vec_each.second);
				}
				ch->ChatPacket(CHAT_TYPE_INFO, "<------------------------------------------------------->");
				ret = true;
			}
			else
				ret = Make(i.second);
			break;
		}
	}

	switch (ret)
	{
	case -1:
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "Wiki command cant handle! Active commands:");
			for (auto &&i : makeStringMap)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, "       %s", i.first.c_str());
			}
			break;
		}
	case 0:
		ch->ChatPacket(CHAT_TYPE_INFO, "Wiki command failed!");
	case 1:
		ch->ChatPacket(CHAT_TYPE_INFO, "Wiki command succes!");

	default:
		break;
	}
}

void CWikiManager::StartRegenInfoFile()
{
	if (m_regenInfoFile)
		fclose(m_regenInfoFile);

	m_regenInfoFile = fopen("./locale/../conf/wiki/regen_info.txt", "w");
}
void CWikiManager::AddRegenInfoFile(const char* filename, long lMapIndex)
{
	if (!m_regenInfoFile)
		return;

	if (g_bAuthServer)
		return;

	// LPREGEN regen = NULL;
	FILE* fp = fopen(filename, "rt");

	if (NULL == fp)
	{
		sys_log(0, "SYSTEM: regen_load: %s: file not found", filename);
		return;
	}

	std::set<std::pair<DWORD, DWORD>> regenSet;

	while (true)
	{
		REGEN tmp{};

		if (!read_line(fp, &tmp))
			break;

		switch (tmp.type)
		{
		case REGEN_TYPE_MOB:
			{
				auto mob = CMobManager::Instance().Get(tmp.vnum);
				if (!mob)
					continue;
				if (!IS_SET(mob->m_table.dwRaceFlag, RACE_FLAG_BOSS) && mob->m_table.bType != CHAR_TYPE_STONE)
					continue;

				std::pair<DWORD, DWORD> mpair = std::make_pair(tmp.vnum, lMapIndex);
				if (regenSet.find(mpair) != regenSet.end())
					continue;

				regenSet.insert(mpair);
				fprintf(m_regenInfoFile, "%d:%ld:%d\n", tmp.vnum, lMapIndex, tmp.time);
				break;
			}

		case REGEN_TYPE_GROUP:
			{
				CMobGroup* pkGroup = CMobManager::Instance().GetGroup(tmp.vnum);
				if (!pkGroup)
					return;
				const std::vector<DWORD>& c_rdwMembers = pkGroup->GetMemberVector();

				for (auto &&vnum : c_rdwMembers)
				{
					auto mob = CMobManager::Instance().Get(vnum);
					if (!mob)
						continue;
					if (!IS_SET(mob->m_table.dwRaceFlag, RACE_FLAG_BOSS) && mob->m_table.bType != CHAR_TYPE_STONE)
						continue;

					std::pair<DWORD, DWORD> mpair = std::make_pair(vnum, lMapIndex);
					if (regenSet.find(mpair) != regenSet.end())
						continue;

					regenSet.insert(mpair);
					fprintf(m_regenInfoFile, "%d:%ld:%d\n", vnum, lMapIndex, tmp.time);
				}
				break;
			}

		case REGEN_TYPE_GROUP_GROUP:
			{
				CMobGroupGroup* pkGroupGroup = CMobManager::Instance().GetGroupGroup(tmp.vnum);
				if (!pkGroupGroup)
					return;

				for (auto &&vnum : pkGroupGroup->m_vec_dwMemberVnum)
				{
					auto mob = CMobManager::Instance().Get(vnum);
					if (!mob)
						continue;
					if (!IS_SET(mob->m_table.dwRaceFlag, RACE_FLAG_BOSS) && mob->m_table.bType != CHAR_TYPE_STONE)
						continue;

					std::pair<DWORD, DWORD> mpair = std::make_pair(vnum, lMapIndex);
					if (regenSet.find(mpair) != regenSet.end())
						continue;

					regenSet.insert(mpair);
					fprintf(m_regenInfoFile, "%d:%ld:%d\n", vnum, lMapIndex, tmp.time);
				}
				break;
			}
		}

	}
}
void CWikiManager::EndRegenInfoFile()
{
	if (m_regenInfoFile)
		fclose(m_regenInfoFile);
}

bool CWikiManager::Make(EMakeTypes type)
{
	switch (type)
	{
		case WIKI_MAKE_ALL:
		case WIKI_MAKE_REFINE:
		{
			std::ofstream refineFile("./locale/../conf/wiki/refine_info.txt", std::ios::trunc);

			if (!refineFile.is_open())
				return false;

			for (auto it = CRefineManager::instance().m_map_RefineRecipe.begin(); it != CRefineManager::instance().m_map_RefineRecipe.end(); it++)
			{
				const TRefineTable& refineTable = it->second;

				std::string line = "";
				line += std::to_string(it->first);
				line += ";";

				for (size_t ii = 0; ii < REFINE_MATERIAL_MAX_NUM; ii++) // REFINE_MATERIAL_MAX_NUM
				{
					const TRefineMaterial& refineMaterial = refineTable.materials[ii];

					// if (refineMaterial.vnum == 0)
					// 	break;

					line += std::to_string(refineMaterial.vnum);
					line += ",";
					line += std::to_string(refineMaterial.count);
					line += " ";
				}

				line += std::to_string(refineTable.cost);
				// line += "|";

				refineFile << line.c_str() << std::endl;
			}

			refineFile.close();

			if (type != WIKI_MAKE_ALL)
				break;
		}

		case WIKI_MAKE_ITEM:
		{
			std::map<int, std::string> wearableMap = { // EWearPositions
				{WEARABLE_BODY, "body"},
				{WEARABLE_HEAD, "head"},
				{WEARABLE_FOOTS, "foots"},
				{WEARABLE_WRIST, "wrist"},
				{WEARABLE_WEAPON, "weapon"},
				{WEARABLE_NECK, "neck"},
				{WEARABLE_EAR, "ear"},
				{WEARABLE_UNIQUE, "unique"},
				{WEARABLE_SHIELD, "shield"},
				{WEARABLE_PENDANT, "pendant"},
				{70000, "belt"},
				// {WEARABLE_ARROW, "shield"},
				// {WEARABLE_ABILITY, "shield"},
			};
			std::map<EJobs, std::string> jobNameMap = {
				{JOB_WARRIOR, "warrior"},
				{JOB_ASSASSIN, "assassin"},
				{JOB_SURA, "sura"},
				{JOB_SHAMAN, "shaman"},
				{JOB_WOLFMAN, "wolfman"},
			};

			std::map<std::pair<int, EJobs>, std::ofstream> fileList;// (jobNameMap.size() * wearableMap.size())

			for (auto it = wearableMap.begin(); it != wearableMap.end(); ++it)
			{
				for (auto itt = jobNameMap.begin(); itt != jobNameMap.end(); ++itt)
				{
					auto val = *it;
					auto val2 = *itt;

					std::string fileName = "./locale/../conf/wiki/";
					fileName += val.second;

					{
						std::string dirName = "mkdir -p ";
						dirName += fileName;
						system(dirName.c_str());
					}

					fileName += "/";
					fileName += val2.second;
					fileName += ".txt";
					fileList.emplace(std::make_pair(val.first, val2.first), std::ofstream{ fileName, std::ios::trunc });
				}
			}

			for (auto it = fileList.begin(); it != fileList.end(); ++it)
			{
				if (!it->second.is_open())
					return false;
			}

			for (auto it = ITEM_MANAGER::instance().GetVecProto().begin(); it != ITEM_MANAGER::instance().GetVecProto().end(); ++it)
			{
				auto val = *it;

				if (val.dwVnum % 10 != 0)
					continue;

				if (val.dwRefinedVnum == 0)
					continue;

				std::string line = "";
				line += std::to_string(val.dwVnum);
				line += ";";

				line += std::to_string(val.dwRefineSet);
				line += ",";

				TItemTable *itemTable = ITEM_MANAGER::instance().GetTable(val.dwRefinedVnum);

				if (itemTable == nullptr)
					continue;

				while (itemTable != nullptr)
				{
					if (itemTable->dwVnum % 10 == 0)
						break;

					if (itemTable->dwRefinedVnum == 0)
						break;

					line += std::to_string(itemTable->dwRefineSet);
					line += ",";

					itemTable = ITEM_MANAGER::instance().GetTable(itemTable->dwRefinedVnum);
				}

				// else
				{
					for (auto itt = wearableMap.begin(); itt != wearableMap.end(); ++itt)
					{
						auto vall = *itt;

						// if(vall.first == WEARABLE_WEAPON)
						// {

						// }

						for (auto itt2 = jobNameMap.begin(); itt2 != jobNameMap.end(); ++itt2)
						{
							auto vall2 = *itt2;

							if (vall.first == 70000)
							{
								if (val.bType == ITEM_BELT)
									fileList[std::make_pair(vall.first, vall2.first)] << line.c_str() << std::endl;
								continue;
							}

							else if (IS_SET(val.dwWearFlags, vall.first))
							{
								switch (vall2.first)
								{
									case JOB_WARRIOR:
										if (val.dwAntiFlags & ITEM_ANTIFLAG_WARRIOR)
											continue;
										break;

									case JOB_ASSASSIN:
										if (val.dwAntiFlags & ITEM_ANTIFLAG_ASSASSIN)
											continue;
										switch (vall.first)
										{
										case WEARABLE_WEAPON:
											if (!(val.dwAntiFlags & ITEM_ANTIFLAG_WARRIOR))
												continue;
										}
										break;

									case JOB_SHAMAN:
										if (val.dwAntiFlags & ITEM_ANTIFLAG_SHAMAN)
											continue;
										break;

									case JOB_SURA:
										if (val.dwAntiFlags & ITEM_ANTIFLAG_SURA)
											continue;
										break;

									case JOB_WOLFMAN:
										if (val.dwAntiFlags & ITEM_ANTIFLAG_WOLFMAN)
											continue;
										break;

									case JOB_MAX_NUM:
										break;
								}

								fileList[std::make_pair(vall.first, vall2.first)] << line.c_str() << std::endl;
							}

							// break;
						}
					}
				}
			}

			for (auto it = fileList.begin(); it != fileList.end(); ++it)
			{
				it->second.close();
			}

			if (type != WIKI_MAKE_ALL)
				break;
		}

		case WIKI_MAKE_CHEST:
		{
#ifdef __CHEST_INFO_SYSTEM__
			std::ofstream file("./locale/../conf/wiki/chest_info.txt", std::ios::trunc);

			if (!file.is_open())
				return false;


			for (auto it = ITEM_MANAGER::instance().GetVecProto().begin(); it != ITEM_MANAGER::instance().GetVecProto().end(); ++it)
			{
				auto val = *it;

				std::vector<TChestDropInfoTable> vec_ItemList;
				ITEM_MANAGER::instance().GetChestItemList(val.dwVnum, vec_ItemList);
				if (vec_ItemList.size() == 0)
					continue;

				std::string line = "";
				line += std::to_string(val.dwVnum);
				line += ":";

				for (auto &&i : vec_ItemList)
				{
					line += std::to_string(i.dwItemVnum);
					line += ",";
					line += std::to_string(i.bItemCount);
					line += " ";
				}

				file << line.c_str() << std::endl;

			}

			file.close();

			if (type != WIKI_MAKE_ALL)
				break;
#endif
		}

		case WIKI_MAKE_CUBE:
		{
			std::ofstream file("./locale/../conf/wiki/cube_info.txt", std::ios::trunc);

			if (!file.is_open())
				return false;

			for (auto &&i : s_cube_proto)
			{
				auto resultVnum = i->reward[0].vnum;

				std::string line = "";
				line += std::to_string(resultVnum);
				line += ":";

				for (auto &&i : i->item)
				{
					line += std::to_string(i.vnum);
					line += ",";
					line += std::to_string(i.count);
					line += " ";
				}


				file << line.c_str() << std::endl;
			}


			file.close();

			if (type != WIKI_MAKE_ALL)
				break;
		}

		case WIKI_MAKE_STONE:
		{
			std::ofstream file("./locale/../conf/wiki/stone_info.txt", std::ios::trunc);

			if (!file.is_open())
				return false;


			for (auto it = CMobManager::instance().begin(); it != CMobManager::instance().end(); ++it)
			{
				auto val = *it;
				if (val.second->m_table.bType != CHAR_TYPE_STONE)
					continue;

				std::string line = "";
				line += std::to_string(val.first);
				line += ":";

				std::vector<std::pair<DWORD, DWORD>> vec_item;
				CreateDropItemVector(*val.second, vec_item);
				if (vec_item.size() == 0)
					continue;

				for (auto &&i : vec_item)
				{
					line += std::to_string(i.first);
					line += ".";
					line += std::to_string(i.second);

					line += " ";
				}

				file << line.c_str() << std::endl;
			}


			file.close();

			if (type != WIKI_MAKE_ALL)
				break;
		}

		case WIKI_MAKE_BOSS:
		{
			std::ofstream file("./locale/../conf/wiki/boss_info.txt", std::ios::trunc);

			if (!file.is_open())
				return false;


			for (auto it = CMobManager::instance().begin(); it != CMobManager::instance().end(); ++it)
			{
				auto val = *it;
				// if (val.second->m_table.bType != CHAR_TYPE_MONSTER)
				// 	continue;
				// if (val.second->m_table.bRank != MOB_RANK_BOSS)
				// 	continue;
				if (!IS_SET(val.second->m_table.dwRaceFlag, RACE_FLAG_BOSS))
					continue;

				std::string line = "";
				line += std::to_string(val.first);
				line += ":";

				std::vector<std::pair<DWORD, DWORD>> vec_item;
				CreateDropItemVector(*val.second, vec_item);
				if (vec_item.size() == 0)
					continue;

				for (auto &&i : vec_item)
				{
					line += std::to_string(i.first);
					line += ".";
					line += std::to_string(i.second);

					line += " ";
				}

				file << line.c_str() << std::endl;
			}


			file.close();

			if (type != WIKI_MAKE_ALL)
				break;
		}

		case WIKI_MAKE_SHOP:
		{
			std::ofstream file("./locale/../conf/wiki/shop.txt", std::ios::trunc);

			if (!file.is_open())
				return false;

			for (auto &&i : CShopManager::instance().m_map_pkShopByNPCVnum)
			{
				std::string line = "";
				line += std::to_string(i.first);
				line += ":";

				for (auto &&j : i.second->m_itemVector)
				{
					line += std::to_string(j.vnum); line += ",";
					line += std::to_string(j.count); line += ",";
					line += std::to_string(j.witemVnum); line += ",";
					line += std::to_string(j.price); line += ",";
					line += std::to_string(j.cheque); line += ",";
					// line += std::to_string(j.pkItem->GetVnum()); line += " ";

					line += " ";
				}

				file << line.c_str() << std::endl;

			}

			file.close();


			if (type != WIKI_MAKE_ALL)
				break;
		}

		// case WIKI_MAKE_BOSS:
		// {
		// 	if (type != WIKI_MAKE_ALL)
		// 		break;
		// }

		default:
			break;
	}

	return true;
}

void CWikiManager::CreateDropItemVector(CMob& mob, std::vector<std::pair<DWORD, DWORD>>& vec_item)
{
	DWORD vnum = mob.m_table.dwVnum;

	// Drop Item Group
	{
		itertype(ITEM_MANAGER::instance().m_map_pkDropItemGroup) it;
		it = ITEM_MANAGER::instance().m_map_pkDropItemGroup.find(vnum);

		if (it != ITEM_MANAGER::instance().m_map_pkDropItemGroup.end())
		{
			typeof(it->second->GetVector()) v = it->second->GetVector();

			for (DWORD i = 0; i < v.size(); ++i)
			{
				vec_item.push_back({v[i].dwVnum, v[i].iCount});
				// item = CreateItem(v[i].dwVnum, v[i].iCount, 0, true);

				// if (item)
				// {
				// 	if (item->GetType() == ITEM_POLYMORPH)
				// 	{
						// if (item->GetVnum() == pkChr->GetPolymorphItemVnum())
						// {
						// 	item->SetSocket(0, vnum);
						// }
						// continue; // TODO::
				// 	}

				// 	vec_item.push_back(item);
				// }
			}
		}
	}

	// MobDropItem Group
	{
		itertype(ITEM_MANAGER::instance().m_map_pkMobItemGroup) it;
		it = ITEM_MANAGER::instance().m_map_pkMobItemGroup.find(vnum);

		if (it != ITEM_MANAGER::instance().m_map_pkMobItemGroup.end())
		{
			CMobItemGroup* pGroup = it->second;

			// MOB_DROP_ITEM_BUG_FIX
			if (pGroup && !pGroup->IsEmpty())
			{
				const CMobItemGroup::SMobItemGroupInfo& info = pGroup->GetOne();
				vec_item.push_back({info.dwItemVnum, info.iCount});
			}
			// END_OF_MOB_DROP_ITEM_BUG_FIX
		}
	}

	// Level Item Group
	{
		itertype(ITEM_MANAGER::instance().m_map_pkLevelItemGroup) it;
		it = ITEM_MANAGER::instance().m_map_pkLevelItemGroup.find(vnum);

		if (it != ITEM_MANAGER::instance().m_map_pkLevelItemGroup.end())
		{
			typeof(it->second->GetVector()) v = it->second->GetVector();

			for (DWORD i = 0; i < v.size(); i++)
			{
				vec_item.push_back({v[i].dwVNum, v[i].iCount});
			}
		}
	}

	if (mob.m_table.dwDropItemVnum)
	{
		itertype(ITEM_MANAGER::instance().m_map_dwEtcItemDropProb) it = ITEM_MANAGER::instance().m_map_dwEtcItemDropProb.find(mob.m_table.dwDropItemVnum);

		if (it != ITEM_MANAGER::instance().m_map_dwEtcItemDropProb.end())
		{
			vec_item.push_back({mob.m_table.dwDropItemVnum, 1});
		}
	}

	if (mob.m_table.bType == CHAR_TYPE_STONE && !g_NoDropMetinStone)
	{
		for (int i = 0; i < 14; ++i)
		{
			vec_item.push_back({28030 + i, 1});
		}
	}
}


void CWikiManager::CreateGMDropInfoVector(DWORD vnum, EVnumType vnumType, DWORD count, std::map<DWORD, DWORD>& vec_item)
{
	switch (vnumType)
	{
	case VNUM_TYPE_CHEST:
		{
			const auto pGroup = ITEM_MANAGER::instance().GetSpecialItemGroup(vnum);
			if (!pGroup)
				return;

			for (size_t i = 0; i < count; i++)
			{
				size_t index = pGroup->GetOneIndex();
				auto selectedItem = pGroup->m_vecItems.at(index);

				std::map<DWORD, DWORD>::iterator it = vec_item.find(selectedItem.vnum);
				if (it == vec_item.end())
					vec_item[selectedItem.vnum] = selectedItem.count;
				else
					vec_item[selectedItem.vnum] += selectedItem.count;
			}
		}
		break;

	case VNUM_TYPE_MOB:
		{
			itertype(ITEM_MANAGER::instance().m_map_pkDropItemGroup) it;
			it = ITEM_MANAGER::instance().m_map_pkDropItemGroup.find(vnum);

			if (it == ITEM_MANAGER::instance().m_map_pkDropItemGroup.end())
				return;

			for (size_t i = 0; i < count; i++)
			{
				for (auto &&ii : it->second->GetVector())
				{
					if (ii.dwPct >= number(1, 100))
					{
						std::map<DWORD, DWORD>::iterator itt = vec_item.find(ii.dwVnum);
						if (itt == vec_item.end())
							vec_item[ii.dwVnum] = ii.iCount;
						else
							vec_item[ii.dwVnum] += ii.iCount;
					}
				}

			}
		}
		break;

	default:
		break;
	}
}
