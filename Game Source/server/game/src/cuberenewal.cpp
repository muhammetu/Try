#include "stdafx.h"
#include "constants.h"
#include "utils.h"
#include "log.h"
#include "char.h"
#include "locale_service.h"
#include "item.h"
#include "item_manager.h"
#include "questmanager.h"
#include <sstream>
#include "packet.h"
#include "desc_client.h"
#include "log.h"

#ifdef __BATTLE_PASS_SYSTEM__
#include "battle_pass.h"
#endif

std::vector<CUBE_RENEWAL_DATA*>	s_cube_proto;

typedef std::vector<CUBE_RENEWAL_VALUE>	TCubeValueVector;

struct SCubeMaterialInfo
{
	SCubeMaterialInfo()
	{
		bHaveComplicateMaterial = false;
	};

	CUBE_RENEWAL_VALUE			reward;
	TCubeValueVector	material;
#ifdef __GOLD_LIMIT_REWORK__
	long long				gold;
#else
	DWORD				gold;
#endif
#ifdef __GEM_SYSTEM__
	DWORD				gem;
#endif
	int 				percent;
	std::string		category;
#ifdef __CUBE_ATTR_SOCKET_EXTENSION__
	bool	allowCopyAttr;
#endif
	TCubeValueVector	complicateMaterial;

	std::string			infoText;
	bool				bHaveComplicateMaterial;		//
};

struct SItemNameAndLevel
{
	SItemNameAndLevel() { level = 0; }

	std::string		name;
	int				level;
};

typedef std::vector<SCubeMaterialInfo>								TCubeResultList;
typedef boost::unordered_map<DWORD, TCubeResultList>				TCubeMapByNPC;

TCubeMapByNPC cube_info_map;

static bool FN_check_valid_npc(WORD vnum)
{
	for (std::vector<CUBE_RENEWAL_DATA*>::iterator iter = s_cube_proto.begin(); iter != s_cube_proto.end(); iter++)
	{
		if (std::find((*iter)->npc_vnum.begin(), (*iter)->npc_vnum.end(), vnum) != (*iter)->npc_vnum.end())
			return true;
	}

	return false;
}

static bool FN_check_cube_data(CUBE_RENEWAL_DATA* cube_data)
{
	DWORD	i = 0;
	DWORD	end_index = 0;
	if (!cube_data)
		return false;
	end_index = cube_data->npc_vnum.size();
	for (i = 0; i < end_index; ++i)
	{
		if (cube_data->npc_vnum[i] == 0)	return false;
	}

	end_index = cube_data->item.size();
	for (i = 0; i < end_index; ++i)
	{
		if (cube_data->item[i].vnum == 0)		return false;
		if (cube_data->item[i].count == 0)	return false;
	}

	end_index = cube_data->reward.size();
	for (i = 0; i < end_index; ++i)
	{
		if (cube_data->reward[i].vnum == 0)	return false;
		if (cube_data->reward[i].count == 0)	return false;
	}
	return true;
}

static int FN_check_cube_item_vnum_material(const SCubeMaterialInfo& materialInfo, DWORD index)
{
	if (!index)
		return 0;
	if (index <= materialInfo.material.size()) {
		return materialInfo.material[index - 1].vnum;
	}
	return 0;
}

static int FN_check_cube_item_count_material(const SCubeMaterialInfo& materialInfo, DWORD index)
{
	if (!index)
		return 0;
	if (index <= materialInfo.material.size()) {
		return materialInfo.material[index - 1].count;
	}

	return 0;
}

CUBE_RENEWAL_DATA::CUBE_RENEWAL_DATA()
{
	this->gold = 0;
#ifdef __GEM_SYSTEM__
	this->gem = 0;
#endif
	this->category = "WORLDARD";
#ifdef __CUBE_ATTR_SOCKET_EXTENSION__
	this->allowCopyAttr = false;
#endif
}

void Cube_init()
{
	CUBE_RENEWAL_DATA* p_cube = NULL;
	std::vector<CUBE_RENEWAL_DATA*>::iterator iter;

	char file_name[256 + 1];
	snprintf(file_name, sizeof(file_name), "%s/cube.txt", LocaleService_GetBasePath().c_str());

	sys_log(0, "Cube_Init %s", file_name);

	for (iter = s_cube_proto.begin(); iter != s_cube_proto.end(); iter++)
	{
		p_cube = *iter;
		M2_DELETE(p_cube);
	}

	s_cube_proto.clear();

	if (false == Cube_load(file_name))
		sys_err("Cube_Init failed");
}

bool Cube_load(const char* file)
{
	FILE* fp;

	const char* value_string;

	char	one_line[256];
#ifdef __GOLD_LIMIT_REWORK__
	long long		value1, value2;
#else
	int		value1, value2;
#endif
	const char* delim = " \t\r\n";
	char* v, * token_string;
	//	char *v1;

	CUBE_RENEWAL_DATA* cube_data = NULL;
	CUBE_RENEWAL_VALUE	cube_value = { 0,0 };

	if (0 == file || 0 == file[0])
		return false;

	if ((fp = fopen(file, "r")) == 0)
		return false;

	while (fgets(one_line, 256, fp))
	{
		value1 = value2 = 0;

		if (one_line[0] == '#')
			continue;

		token_string = strtok(one_line, delim);

		if (NULL == token_string)
			continue;

		// set value1, value2
		if ((v = strtok(NULL, delim)))
			str_to_number(value1, v);
		value_string = v;

		if ((v = strtok(NULL, delim)))
			str_to_number(value2, v);

		TOKEN("section")
		{
			cube_data = M2_NEW CUBE_RENEWAL_DATA;
		}
		else TOKEN("npc")
		{
			cube_data->npc_vnum.push_back((WORD)value1);
		}
		else TOKEN("item")
		{
			cube_value.vnum = value1;
			cube_value.count = value2;

			cube_data->item.push_back(cube_value);
		}
		else TOKEN("reward")
		{
			cube_value.vnum = value1;
			cube_value.count = value2;

			cube_data->reward.push_back(cube_value);
		}
		else TOKEN("percent")
		{
			cube_data->percent = value1;
		}

	else TOKEN("category")
	{
		cube_data->category = value_string;
	}

	else TOKEN("gold")
	{
		cube_data->gold = value1;
	}
#ifdef __GEM_SYSTEM__
		else TOKEN("gem")
		{
		cube_data->gem = value1;
		}
#endif
#ifdef __CUBE_ATTR_SOCKET_EXTENSION__
		else TOKEN("allow_copy")
		{
			cube_data->allowCopyAttr = (value1 == 1 ? true : false);
		}
#endif
		else TOKEN("end")
		{
			// TODO : check cube data
			if (false == FN_check_cube_data(cube_data))
			{
				if (test_server)
					sys_log(0, "something wrong");
				M2_DELETE(cube_data);
				continue;
			}

			s_cube_proto.push_back(cube_data);
		}
	}

	fclose(fp);
	return true;
}

SItemNameAndLevel SplitItemNameAndLevelFromName(const std::string& name)
{
	int level = 0;
	SItemNameAndLevel info;
	info.name = name;

	size_t pos = name.find("+");

	if (std::string::npos != pos)
	{
		const std::string levelStr = name.substr(pos + 1, name.size() - pos - 1);
		str_to_number(level, levelStr.c_str());

		info.name = name.substr(0, pos);
	}

	info.level = level;

	return info;
};

bool Cube_InformationInitialize()
{
	for (DWORD i = 0; i < s_cube_proto.size(); ++i)
	{
		CUBE_RENEWAL_DATA* cubeData = s_cube_proto[i];
		const std::vector<CUBE_RENEWAL_VALUE>& rewards = cubeData->reward;

		if (1 != rewards.size())
		{
			sys_err("[CubeInfo] WARNING! Does not support multiple rewards (count: %d)", rewards.size());
			continue;
		}

		const CUBE_RENEWAL_VALUE& reward = rewards.at(0);
		const WORD& npcVNUM = cubeData->npc_vnum.at(0);
		//bool bComplicate = false;

		TCubeMapByNPC& cubeMap = cube_info_map;
		TCubeResultList& resultList = cubeMap[npcVNUM];
		SCubeMaterialInfo materialInfo;

		materialInfo.reward = reward;
		materialInfo.gold = cubeData->gold;
#ifdef __GEM_SYSTEM__
		materialInfo.gem = cubeData->gem;
#endif
		materialInfo.percent = cubeData->percent;
		materialInfo.material = cubeData->item;
		materialInfo.category = cubeData->category;

		resultList.push_back(materialInfo);
	}

	//s_isInitializedCubeMaterialInformation = true;
	return true;
}

void Cube_open(LPCHARACTER ch)
{
	if (!ch)
		return;

	LPCHARACTER	npc = ch->GetQuestNPC();

	if (!npc)
		return;

	DWORD npcVNUM = npc->GetRaceNum();

	if (FN_check_valid_npc(npcVNUM) == false)
	{
		if (test_server == 1)
		{
			sys_log(0, "cube not valid NPC");
		}
		return;
	}

	if (!ch->CanAct(true, true, VAR_TIME_NONE, nullptr, 0))
		return;

	long distance = DISTANCE_APPROX(ch->GetX() - npc->GetX(), ch->GetY() - npc->GetY());

	if (distance >= CUBE_MAX_DISTANCE)
	{
		sys_log(1, "CUBE: TOO_FAR: %s distance %d", ch->GetName(), distance);
		return;
	}

	SendDateCubeRenewalPackets(ch, CUBE_RENEWAL_SUB_HEADER_CLEAR_DATES_RECEIVE);
	SendDateCubeRenewalPackets(ch, CUBE_RENEWAL_SUB_HEADER_DATES_RECEIVE, npcVNUM);
	SendDateCubeRenewalPackets(ch, CUBE_RENEWAL_SUB_HEADER_DATES_LOADING);
	SendDateCubeRenewalPackets(ch, CUBE_RENEWAL_SUB_HEADER_OPEN_RECEIVE);

	ch->SetCubeNpc(npc);
}

void Cube_close(LPCHARACTER ch)
{
	if (ch)
		ch->SetCubeNpc(NULL);
}

void Cube_Make(LPCHARACTER ch, int index, int count_item, int index_item_improve)
{
	if (!ch)
		return;

	if (count_item < 0)
	{
		LogManager::instance().HackLogEx(ch, "OROSPU COCUGU CUBE.");
		return;
	}

	if (index_item_improve != -1)
	{
		LogManager::instance().HackLogEx(ch, "OROSPU COCUGU CUBE SANS.");
		return;
	}

	LPCHARACTER	npc = ch->GetQuestNPC();

	if (!ch->IsCubeOpen())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("To create an item you have to have the refinement window open"));
		return;
	}

	if (NULL == npc)
		return;

#ifdef __CUBE_ATTR_SOCKET_EXTENSION__
	bool canCopy = false;
#endif
	int index_value = 0;
	bool material_check = true;
	LPITEM pItem;
	int iEmptyPos;
#ifdef __CUBE_ATTR_SOCKET_EXTENSION__
	DWORD copyAttr[ITEM_ATTRIBUTE_MAX_NUM][2];
	long copySockets[ITEM_SOCKET_MAX_NUM];
	DWORD copyEvolution;

	memset(copyAttr, 0, sizeof(copyAttr));
	memset(copySockets, 0, sizeof(copySockets));
	copyEvolution = 0;
#endif

	int iEmptyIndexCount = ch->GetEmptyInventory(3);
	if (iEmptyIndexCount == -1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ARINDIRMA_ISLEMI_ICIN_3_SLOT_YER_LAZIM"));
		return;
	}

	const TCubeResultList& resultList = cube_info_map[npc->GetRaceNum()];
	// cubedeki itemleri listeliyor.
	for (TCubeResultList::const_iterator iter = resultList.begin(); resultList.end() != iter; ++iter)
	{
		// sectigimiz index cube indexine aitse burasi donuyor.
		if (index_value == index)
		{
			const SCubeMaterialInfo& materialInfo = *iter;
			// istenen esyalari envanterde kontrol ediyor.
			for (DWORD i = 0; i < materialInfo.material.size(); ++i)
			{
				if (ch->CountSpecifyItem(materialInfo.material[i].vnum) < (materialInfo.material[i].count * count_item))
					material_check = false;
			}

			// Check Gold And ETC.
			if (materialInfo.gold != 0)
			{
				if (ch->GetGold() < (materialInfo.gold * count_item))
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("He doesn't have the necessary amount of yang."));
					return;
				}
			}
#ifdef __GEM_SYSTEM__
			if (materialInfo.gem != 0) {
				if (ch->CountSpecifyItem(70075) < (materialInfo.gem * count_item))
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("He doesn't have the necessary amount of gem."));
					return;
				}
			}
#endif
			// Check Gold And ETC..

			// Stack Check
			bool pAntiStack = false;
			TItemTable* p = ITEM_MANAGER::instance().GetTable(materialInfo.reward.vnum);
			if (p)
			{
				if (p->dwFlags & ITEM_FLAG_STACKABLE)
					pAntiStack = true;
			}

			if (pAntiStack == false)
				count_item = 1;
			// Stack Check End

			if (material_check)
			{
				int percent_number;
				int total_items_give = 0;
				int porcent_item_improve = 0;

				for (int i = 0; i < count_item; ++i)
				{
					percent_number = number(1, 100);
					if (percent_number <= materialInfo.percent + porcent_item_improve)
						total_items_give++;
				}
#ifdef __CUBE_ATTR_SOCKET_EXTENSION__
				WORD objectSlot = -1; // Define -1 slot (inventory start from 0, not from 1)

				for (int i = 0; i < INVENTORY_MAX_NUM; ++i)//Starting searchinf in inventory for Material Armor
				{
					LPITEM object = ch->GetInventoryItem(i);//Select Item via LPITEM
					if (NULL == object)
						continue; // Skip if is null to avoiding crashes
					if (object->GetType() == ITEM_WEAPON || object->GetType() == ITEM_ARMOR || object->GetType() == ITEM_BELT) // Check if is armor or weapon
					{
						if (object->GetVnum() == materialInfo.material[0].vnum) // Check if Select item is same item with crafting item
						{
							objectSlot = object->GetCell(); // Copy slot
							canCopy = true;
							break; //Stop loop if item is finded
						}
					}
				}

				if (canCopy)
				{
					// esyamizin olup olmadigini tekrar kontrol ediyor.
					if (objectSlot >= 0)
					{
						LPITEM BonusItem = ch->GetInventoryItem(objectSlot);//Select Finded slot above function
						if (BonusItem)
						{
							for (int a = 0; a < ITEM_ATTRIBUTE_MAX_NUM; a++)
							{
								if (BonusItem->GetAttributeType(a) != 0)
								{
									copyAttr[a][0] = BonusItem->GetAttributeType(a);
									copyAttr[a][1] = BonusItem->GetAttributeValue(a);
								}
							}


							if (BonusItem->GetType() == ITEM_WEAPON || BonusItem->GetType() == ITEM_ARMOR || BonusItem->GetType() == ITEM_BELT)
							{
								for (int a = 0; a < ITEM_SOCKET_MAX_NUM; a++)
									copySockets[a] = BonusItem->GetSocket(a);
							}

#ifdef __ITEM_EVOLUTION__
	#ifdef __ARMOR_EVOLUTION__
							if (BonusItem->GetType() == ITEM_WEAPON || BonusItem->GetType() == ITEM_ARMOR)
	#else
							if (BonusItem->GetType() == ITEM_WEAPON)
	#endif
							{
								copyEvolution = BonusItem->GetEvolution();
							}
#endif
						}
					}
				}
#endif

				pItem = ITEM_MANAGER::instance().CreateItem(materialInfo.reward.vnum, (materialInfo.reward.count * total_items_give));

				if (!pItem)
					return;

				if (pItem->IsDragonSoul())
				{
					iEmptyPos = ch->GetEmptyDragonSoulInventory(pItem);
					if (iEmptyPos < 0)
					{
						ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You do not have enough space in your inventory."));
						M2_DESTROY_ITEM(pItem);
						return;
					}
				}
#ifdef __ADDITIONAL_INVENTORY__
				else if (pItem->IsUpgradeItem())
				{
					iEmptyPos = ch->GetEmptyUpgradeInventory(pItem);
					if (iEmptyPos < 0)
					{
						ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You do not have enough space in your inventory."));
						M2_DESTROY_ITEM(pItem);
						return;
					}
				}
				else if (pItem->IsBook())
				{
					iEmptyPos = ch->GetEmptyBookInventory(pItem);
					if (iEmptyPos < 0)
					{
						ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You do not have enough space in your inventory."));
						M2_DESTROY_ITEM(pItem);
						return;
					}
				}
				else if (pItem->IsStone())
				{
					iEmptyPos = ch->GetEmptyStoneInventory(pItem);
					if (iEmptyPos < 0)
					{
						ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You do not have enough space in your inventory."));
						M2_DESTROY_ITEM(pItem);
						return;
					}
				}
				else if (pItem->IsFlower())
				{
					iEmptyPos = ch->GetEmptyFlowerInventory(pItem);
					if (iEmptyPos < 0)
					{
						ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You do not have enough space in your inventory."));
						M2_DESTROY_ITEM(pItem);
						return;
					}
				}
				else if (pItem->IsAttrItem())
				{
					iEmptyPos = ch->GetEmptyAttrInventory(pItem);
					if (iEmptyPos < 0)
					{
						ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You do not have enough space in your inventory."));
						M2_DESTROY_ITEM(pItem);
						return;
					}
				}
				else if (pItem->IsChest())
				{
					iEmptyPos = ch->GetEmptyChestInventory(pItem);
					if (iEmptyPos < 0)
					{
						ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You do not have enough space in your inventory."));
						M2_DESTROY_ITEM(pItem);
						return;
					}
			}
#endif
				else {
					iEmptyPos = ch->GetEmptyInventory(pItem->GetSize());
					if (iEmptyPos < 0)
					{
						ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You do not have enough space in your inventory."));
						M2_DESTROY_ITEM(pItem);
						return;
					}
				}


				// envanterimizden donusum esyalarini siliyor.
				for (DWORD i = 0; i < materialInfo.material.size(); ++i)
					ch->RemoveSpecifyItem(materialInfo.material[i].vnum, (materialInfo.material[i].count * count_item));

				// envanterimizden yangi siliyor.
				if (materialInfo.gold != 0)
					ch->PointChange(POINT_GOLD, -static_cast<long long>(materialInfo.gold * count_item), false);

				// envanterimizden gayayi siliyor.
#ifdef __GEM_SYSTEM__
				if (materialInfo.gem != 0)
					ch->RemoveSpecifyItem(70075, (materialInfo.gem * count_item));
#endif

				if (total_items_give <= 0)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("It has failed."));
#ifdef __SMITH_EFFECT__
					ch->EffectPacket(SE_FR_FAIL);
#endif
					LogManager::instance().HackLogEx(ch, "OROSBU EVLADI CUBE FAIL.");
					sys_log(0, "%s - %d Cube window make failed", ch->GetName(), ch->GetPlayerID());
					return;
				}

				if (pItem->IsDragonSoul())
				{
					iEmptyPos = ch->GetEmptyDragonSoulInventory(pItem);
					pItem->__ADD_TO_CHARACTER(ch, TItemPos(DRAGON_SOUL_INVENTORY, iEmptyPos));
				}
#ifdef __ADDITIONAL_INVENTORY__
				else if (pItem->IsUpgradeItem())
				{
					iEmptyPos = ch->GetEmptyUpgradeInventory(pItem);
					pItem->__ADD_TO_CHARACTER(ch, TItemPos(UPGRADE_INVENTORY, iEmptyPos));
				}
				else if (pItem->IsBook())
				{
					iEmptyPos = ch->GetEmptyBookInventory(pItem);
					pItem->__ADD_TO_CHARACTER(ch, TItemPos(BOOK_INVENTORY, iEmptyPos));
				}
				else if (pItem->IsStone())
				{
					iEmptyPos = ch->GetEmptyStoneInventory(pItem);
					pItem->__ADD_TO_CHARACTER(ch, TItemPos(STONE_INVENTORY, iEmptyPos));
				}
				else if (pItem->IsFlower())
				{
					iEmptyPos = ch->GetEmptyFlowerInventory(pItem);
					pItem->__ADD_TO_CHARACTER(ch, TItemPos(FLOWER_INVENTORY, iEmptyPos));
				}
				else if (pItem->IsAttrItem())
				{
					iEmptyPos = ch->GetEmptyAttrInventory(pItem);
					pItem->__ADD_TO_CHARACTER(ch, TItemPos(ATTR_INVENTORY, iEmptyPos));
				}
				else if (pItem->IsChest())
				{
					iEmptyPos = ch->GetEmptyChestInventory(pItem);
					pItem->__ADD_TO_CHARACTER(ch, TItemPos(CHEST_INVENTORY, iEmptyPos));
				}
#endif
				else {
					iEmptyPos = ch->GetEmptyInventory(pItem->GetSize());
					pItem->__ADD_TO_CHARACTER(ch, TItemPos(INVENTORY, iEmptyPos));
				}

#ifdef __CUBE_ATTR_SOCKET_EXTENSION__
				if (materialInfo.allowCopyAttr == true && copyAttr != NULL)
				{
					pItem->ClearAttribute();

					for (int a = 0; a < ITEM_ATTRIBUTE_MAX_NUM; a++)
					{
						if (copyAttr[a][0] > 0)
							pItem->SetForceAttribute(a, copyAttr[a][0], copyAttr[a][1]);
					}

					if (pItem->GetType() == ITEM_WEAPON || pItem->GetType() == ITEM_ARMOR || pItem->GetType() == ITEM_BELT)
					{
						for (int a = 0; a < ITEM_SOCKET_MAX_NUM; a++)
						{
							if (copySockets[a])
								pItem->SetSocket(a, copySockets[a]);
						}
					}
#ifdef __ITEM_EVOLUTION__
#ifdef __ARMOR_EVOLUTION__
					if (pItem->GetType() == ITEM_WEAPON || pItem->GetType() == ITEM_ARMOR)
#else
					if (pItem->GetType() == ITEM_WEAPON)
#endif
					{
						pItem->SetEvolution(copyEvolution);
					}
#endif
				}
#endif

#ifdef __BATTLE_PASS_SYSTEM__
				BYTE bBattlePassId = ch->GetBattlePassId();
				if(bBattlePassId)
				{
					DWORD dwItemVnum, dwCount;
					if(CBattlePass::instance().BattlePassMissionGetInfo(bBattlePassId, CRAFT_ITEM, &dwItemVnum, &dwCount))
					{
						if(dwItemVnum == materialInfo.reward.vnum && ch->GetMissionProgress(CRAFT_ITEM, bBattlePassId) < dwCount)
							ch->UpdateMissionProgress(CRAFT_ITEM, bBattlePassId, (materialInfo.reward.count*total_items_give), dwCount);
					}
				}
#endif

				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CUBE_RENEWAL_SUCCESS"));
#ifdef __SMITH_EFFECT__
				ch->EffectPacket(SE_FR_SUCCESS);
#endif
				sys_log(0, "%s - %d Cube window make success", ch->GetName(), ch->GetPlayerID());
			}
			else
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You don't have the necessary materials."));
				sys_log(0, "%s - %d Cube window make fail material not exist", ch->GetName(), ch->GetPlayerID());
			}
		}

		// eger index secmediysek buradaki fonksiyon devam ediyor fakat yukarida indeximiz devam etse bile burasi artmaya devam edecek.
		index_value++;
	}
}

void SendDateCubeRenewalPackets(LPCHARACTER ch, BYTE subheader, DWORD npcVNUM)
{
	TPacketGCCubeRenewalReceive pack;
	pack.subheader = subheader;

	if (subheader == CUBE_RENEWAL_SUB_HEADER_DATES_RECEIVE)
	{
		const TCubeResultList& resultList = cube_info_map[npcVNUM];
		for (TCubeResultList::const_iterator iter = resultList.begin(); resultList.end() != iter; ++iter)
		{
			const SCubeMaterialInfo& materialInfo = *iter;

			pack.date_cube_renewal.vnum_reward = materialInfo.reward.vnum;
			pack.date_cube_renewal.count_reward = materialInfo.reward.count;

			TItemTable* pTable = ITEM_MANAGER::instance().GetTable(materialInfo.reward.vnum);
			if (pTable)
			{
				if (IS_SET(pTable->dwFlags, ITEM_FLAG_STACKABLE) || !IS_SET(pTable->dwAntiFlags, ITEM_ANTIFLAG_STACK))
					pack.date_cube_renewal.item_reward_stackable = true;
				else
					pack.date_cube_renewal.item_reward_stackable = false;
			}

			pack.date_cube_renewal.vnum_material_1 = FN_check_cube_item_vnum_material(materialInfo, 1);
			pack.date_cube_renewal.count_material_1 = FN_check_cube_item_count_material(materialInfo, 1);

			pack.date_cube_renewal.vnum_material_2 = FN_check_cube_item_vnum_material(materialInfo, 2);
			pack.date_cube_renewal.count_material_2 = FN_check_cube_item_count_material(materialInfo, 2);

			pack.date_cube_renewal.vnum_material_3 = FN_check_cube_item_vnum_material(materialInfo, 3);
			pack.date_cube_renewal.count_material_3 = FN_check_cube_item_count_material(materialInfo, 3);

			pack.date_cube_renewal.vnum_material_4 = FN_check_cube_item_vnum_material(materialInfo, 4);
			pack.date_cube_renewal.count_material_4 = FN_check_cube_item_count_material(materialInfo, 4);

			pack.date_cube_renewal.vnum_material_5 = FN_check_cube_item_vnum_material(materialInfo, 5);
			pack.date_cube_renewal.count_material_5 = FN_check_cube_item_count_material(materialInfo, 5);

			pack.date_cube_renewal.vnum_material_6 = FN_check_cube_item_vnum_material(materialInfo, 6);
			pack.date_cube_renewal.count_material_6 = FN_check_cube_item_count_material(materialInfo, 6);

			pack.date_cube_renewal.gold = materialInfo.gold;
#ifdef __GEM_SYSTEM__
			pack.date_cube_renewal.gem = materialInfo.gem;
#endif
			pack.date_cube_renewal.percent = materialInfo.percent;

			memcpy(pack.date_cube_renewal.category, materialInfo.category.c_str(), sizeof(pack.date_cube_renewal.category));

			LPDESC d = ch->GetDesc();

			if (NULL == d)
			{
				sys_err("User SendDateCubeRenewalPackets (%s)'s DESC is NULL POINT.", ch->GetName());
				return;
			}

			d->Packet(&pack, sizeof(pack));
		}
	}
	else {
		LPDESC d = ch->GetDesc();

		if (NULL == d)
		{
			sys_err("User SendDateCubeRenewalPackets (%s)'s DESC is NULL POINT.", ch->GetName());
			return;
		}

		d->Packet(&pack, sizeof(pack));
	}
}