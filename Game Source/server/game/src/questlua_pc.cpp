#include "stdafx.h"

#include "config.h"
#include "questmanager.h"
#include "sectree_manager.h"
#include "char.h"
#include "char_manager.h"
#include "affect.h"
#include "item.h"
#include "item_manager.h"
#include "guild_manager.h"
#include "war_map.h"
#include "start_position.h"
#include "mining.h"
#include "p2p.h"
#include "desc_client.h"
#include "messenger_manager.h"
#include "log.h"
#include "utils.h"
#include "unique_item.h"
#include "mob_manager.h"

#include "pvp.h"

#ifdef __DICE_SYSTEM__
#include "party.h"
#endif

#ifdef __BATTLE_PASS_SYSTEM__
#include "battle_pass.h"
#endif

#include <cctype>
#undef sys_err
#ifndef __WIN32__
#define sys_err(fmt, args...) quest::CQuestManager::instance().QuestError(__FUNCTION__, __LINE__, fmt, ##args)
#else
#define sys_err(fmt, ...) quest::CQuestManager::instance().QuestError(__FUNCTION__, __LINE__, fmt, __VA_ARGS__)
#endif

const int ITEM_BROKEN_METIN_VNUM = 28960;
static int ejderde_kac_kisiyiz = 0;

namespace quest
{
	//
	// "pc" Lua functions
	//
	ALUA(pc_has_master_skill)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		bool bHasMasterSkill = false;
		for (int i = 0; i < SKILL_MAX_NUM; i++)
			if (ch->GetSkillMasterType(i) >= SKILL_MASTER && ch->GetSkillLevel(i) >= 21)
			{
				bHasMasterSkill = true;
				break;
			}

		lua_pushboolean(L, bHasMasterSkill);
		return 1;
	}

	ALUA(pc_remove_skill_book_no_delay)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		ch->RemoveAffect(AFFECT_SKILL_NO_BOOK_DELAY);
		return 0;
	}

	ALUA(pc_is_skill_book_no_delay)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();

		lua_pushboolean(L, ch->FindAffect(AFFECT_SKILL_NO_BOOK_DELAY) ? true : false);
		return 1;
	}

	ALUA(pc_learn_grand_master_skill)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();

		if (!lua_isnumber(L, 1))
		{
			sys_err("wrong skill index");
			return 0;
		}

		lua_pushboolean(L, ch->LearnGrandMasterSkill((long)lua_tonumber(L, 1)));
		return 1;
	}

#ifdef __SAGE_SKILL__
	ALUA(pc_learn_sage_master_skill)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();

		if (!lua_isnumber(L, 1))
		{
			sys_err("wrong skill index");
			return 0;
		}

		lua_pushboolean(L, ch->LearnSageMasterSkill((long)lua_tonumber(L, 1)));
		return 1;
	}
#endif

	ALUA(pc_set_warp_location)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();

		if (!lua_isnumber(L, 1))
		{
			sys_err("wrong map index");
			return 0;
		}

		if (!lua_isnumber(L, 2) || !lua_isnumber(L, 3))
		{
			sys_err("wrong coodinate");
			return 0;
		}

		ch->SetWarpLocation((long)lua_tonumber(L, 1), (long)lua_tonumber(L, 2), (long)lua_tonumber(L, 3));
		return 0;
	}

	ALUA(pc_set_warp_location_local)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();

		if (!lua_isnumber(L, 1))
		{
			sys_err("wrong map index");
			return 0;
		}

		if (!lua_isnumber(L, 2) || !lua_isnumber(L, 3))
		{
			sys_err("wrong coodinate");
			return 0;
		}

		long lMapIndex = (long)lua_tonumber(L, 1);
		const TMapRegion* region = SECTREE_MANAGER::instance().GetMapRegion(lMapIndex);

		if (!region)
		{
			sys_err("invalid map index %d", lMapIndex);
			return 0;
		}

		int x = (int)lua_tonumber(L, 2);
		int y = (int)lua_tonumber(L, 3);

		if (x > region->ex - region->sx)
		{
			sys_err("x coordinate overflow max: %d input: %d", region->ex - region->sx, x);
			return 0;
		}

		if (y > region->ey - region->sy)
		{
			sys_err("y coordinate overflow max: %d input: %d", region->ey - region->sy, y);
			return 0;
		}

		ch->SetWarpLocation(lMapIndex, x, y);
		return 0;
	}

	ALUA(pc_get_start_location)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();

		lua_pushnumber(L, g_start_map[ch->GetEmpire()]);
		lua_pushnumber(L, g_start_position[ch->GetEmpire()][0] / 100);
		lua_pushnumber(L, g_start_position[ch->GetEmpire()][1] / 100);
		return 3;
	}

	ALUA(pc_warp)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();

		if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2))
		{
			lua_pushboolean(L, false);
			return 1;
		}

		long map_index = 0;

		if (lua_isnumber(L, 3))
			map_index = (int)lua_tonumber(L, 3);

		//PREVENT_HACK
		if (ch->IsHack())
		{
			lua_pushboolean(L, false);
			return 1;
		}
		//END_PREVENT_HACK

		if (test_server)
			ch->ChatPacket(CHAT_TYPE_INFO, "pc_warp %d %d %d", (int)lua_tonumber(L, 1),
				(int)lua_tonumber(L, 2), map_index);
		ch->WarpSet((int)lua_tonumber(L, 1), (int)lua_tonumber(L, 2), map_index);

		lua_pushboolean(L, true);

		return 1;
	}

	ALUA(pc_warp_local)
	{
		if (!lua_isnumber(L, 1))
		{
			sys_err("no map index argument");
			return 0;
		}

		if (!lua_isnumber(L, 2) || !lua_isnumber(L, 3))
		{
			sys_err("no coodinate argument");
			return 0;
		}

		long lMapIndex = (long)lua_tonumber(L, 1);
		const TMapRegion* region = SECTREE_MANAGER::instance().GetMapRegion(lMapIndex);

		if (!region)
		{
			sys_err("invalid map index %d", lMapIndex);
			return 0;
		}

		int x = (int)lua_tonumber(L, 2);
		int y = (int)lua_tonumber(L, 3);

		if (x > region->ex - region->sx)
		{
			sys_err("x coordinate overflow max: %d input: %d", region->ex - region->sx, x);
			return 0;
		}

		if (y > region->ey - region->sy)
		{
			sys_err("y coordinate overflow max: %d input: %d", region->ey - region->sy, y);
			return 0;
		}

		CQuestManager::instance().GetCurrentCharacterPtr()->WarpSet(region->sx + x, region->sy + y);
		return 0;
	}

	ALUA(pc_warp_exit)
	{
		CQuestManager::instance().GetCurrentCharacterPtr()->ExitToSavedLocation();
		return 0;
	}

	ALUA(pc_in_dungeon)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		lua_pushboolean(L, ch ? ch->GetDungeon() ? 1 : 0 : 0);
		return 1;
	}

	ALUA(pc_hasguild)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		lua_pushboolean(L, ch->GetGuild() ? 1 : 0);
		return 1;
	}

	ALUA(pc_getguild)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		lua_pushnumber(L, ch->GetGuild() ? ch->GetGuild()->GetID() : 0);
		return 1;
	}

	ALUA(pc_isguildmaster)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		CGuild* g = ch->GetGuild();

		if (g)
			lua_pushboolean(L, (ch->GetPlayerID() == g->GetMasterPID()));
		else
			lua_pushboolean(L, 0);

		return 1;
	}

	ALUA(pc_destroy_guild)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		CGuild* g = ch->GetGuild();

		if (g)
		{
			// @DIKKAT @duzenleme lonca savasi mevcut iken loncayi kapatmayi engelliyor.
			// bu arkadasin queste tasinmasi lazim.
			if (g->UnderAnyWar())
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Lonca savasi icerisindeyken loncayi kapatamazsiniz."));
				return 0;
			}
			else
				g->RequestDisband(ch->GetPlayerID());
		}
		return 0;
	}

	ALUA(pc_remove_from_guild)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		CGuild* g = ch->GetGuild();

		if (g)
			g->RequestRemoveMember(ch->GetPlayerID());

		return 0;
	}

	ALUA(pc_give_gold)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();

		if (!lua_isnumber(L, 1))
		{
			sys_err("QUEST : wrong argument");
			return 0;
		}

#ifdef __GOLD_LIMIT_REWORK__
		long long iAmount = (long long)lua_tonumber(L, 1);
#else
		int iAmount = (int)lua_tonumber(L, 1);
#endif

		if (iAmount <= 0)
		{
			sys_err("QUEST : gold amount less then zero");
			return 0;
		}

		if (ch->GetGold() < GOLD_MAX)
			ch->PointChange(POINT_GOLD, iAmount, true);
		return 0;
	}

	ALUA(pc_warp_to_guild_war_observer_position)
	{
		luaL_checknumber(L, 1);
		luaL_checknumber(L, 2);

		DWORD gid1 = (DWORD)lua_tonumber(L, 1);
		DWORD gid2 = (DWORD)lua_tonumber(L, 2);

		CGuild* g1 = CGuildManager::instance().FindGuild(gid1);
		CGuild* g2 = CGuildManager::instance().FindGuild(gid2);

		if (!g1 || !g2)
		{
			luaL_error(L, "no such guild with id %d %d", gid1, gid2);
		}

		PIXEL_POSITION pos;

		DWORD dwMapIndex = g1->GetGuildWarMapIndex(gid2);

		if (!CWarMapManager::instance().GetStartPosition(dwMapIndex, 2, pos))
		{
			luaL_error(L, "not under warp guild war between guild %d %d", gid1, gid2);
			return 0;
		}

		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();

		//PREVENT_HACK
		if (ch->IsHack())
			return 0;
		//END_PREVENT_HACK

		ch->SetQuestFlag("war.is_war_member", 0);
		ch->SaveExitLocation();
		ch->WarpSet(pos.x, pos.y, dwMapIndex);
		return 0;
	}

	ALUA(pc_enough_inventory)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		if (!lua_isnumber(L, 1))
		{
			lua_pushboolean(L, 0);
			return 1;
		}

		DWORD item_vnum = (DWORD)lua_tonumber(L, 1);
		TItemTable* pTable = ITEM_MANAGER::instance().GetTable(item_vnum);
		if (!pTable)
		{
			lua_pushboolean(L, 0);
			return 1;
		}

#ifdef __ADDITIONAL_INVENTORY__
		LPITEM pItem = ITEM_MANAGER::instance().CreateItem(item_vnum);
		bool bEnoughInventoryForItem;
		if (pItem->IsUpgradeItem())
		{
			bEnoughInventoryForItem = ch->GetEmptyUpgradeInventory(pItem) != -1;
		}
		else if (pItem->IsBook())
		{
			bEnoughInventoryForItem = ch->GetEmptyBookInventory(pItem) != -1;
		}
		else if (pItem->IsStone())
		{
			bEnoughInventoryForItem = ch->GetEmptyStoneInventory(pItem) != -1;
		}
		else if (pItem->IsFlower())
		{
			bEnoughInventoryForItem = ch->GetEmptyFlowerInventory(pItem) != -1;
		}
		else if (pItem->IsAttrItem())
		{
			bEnoughInventoryForItem = ch->GetEmptyAttrInventory(pItem) != -1;
		}
		else if (pItem->IsChest())
		{
			bEnoughInventoryForItem = ch->GetEmptyChestInventory(pItem) != -1;
		}
		else
		{
			bEnoughInventoryForItem = ch->GetEmptyInventory(pTable->bSize) != -1;
		}
#else
		bool bEnoughInventoryForItem = ch->GetEmptyInventory(pTable->bSize) != -1;
#endif
		lua_pushboolean(L, bEnoughInventoryForItem);
		return 1;
	}

	ALUA(pc_give_item)
	{
		PC* pPC = CQuestManager::instance().GetCurrentPC();
		//LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		if (!lua_isstring(L, 1) || !(lua_isstring(L, 2) || lua_isnumber(L, 2)))
		{
			sys_err("QUEST : wrong argument");
			return 0;
		}

		DWORD dwVnum;

		if (lua_isnumber(L, 2))
			dwVnum = (int)lua_tonumber(L, 2);
		else if (!ITEM_MANAGER::instance().GetVnum(lua_tostring(L, 2), dwVnum))
		{
			sys_err("QUEST Make item call error : wrong item name : %s", lua_tostring(L, 1));
			return 0;
		}

		int icount = 1;

		if (lua_isnumber(L, 3) && lua_tonumber(L, 3) > 0)
		{
			icount = (int)rint(lua_tonumber(L, 3));

			if (icount <= 0)
			{
				sys_err("QUEST Make item call error : wrong item count : %g", lua_tonumber(L, 2));
				return 0;
			}
		}

		pPC->GiveItem(lua_tostring(L, 1), dwVnum, icount);

		return 0;
	}

	ALUA(pc_give_or_drop_item)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();

		if (!lua_isstring(L, 1) && !lua_isnumber(L, 1))
		{
			sys_err("QUEST Make item call error : wrong argument");
			lua_pushnumber(L, 0);
			return 1;
		}

		DWORD dwVnum;

		if (lua_isnumber(L, 1))
		{
			dwVnum = (int)lua_tonumber(L, 1);
		}
		else if (!ITEM_MANAGER::instance().GetVnum(lua_tostring(L, 1), dwVnum))
		{
			sys_err("QUEST Make item call error : wrong item name : %s", lua_tostring(L, 1));
			lua_pushnumber(L, 0);

			return 1;
		}

		int icount = 1;
		if (lua_isnumber(L, 2) && lua_tonumber(L, 2) > 0)
		{
			icount = (int)rint(lua_tonumber(L, 2));
			if (icount <= 0)
			{
				sys_err("QUEST Make item call error : wrong item count : %g", lua_tonumber(L, 2));
				lua_pushnumber(L, 0);
				return 1;
			}
		}

		sys_log(0, "QUEST [REWARD] item %s to %s", lua_tostring(L, 1), ch->GetName());

		LPITEM item = ch->AutoGiveItem(dwVnum, icount);

		if (NULL != item)
			lua_pushnumber(L, item->GetID());
		else
			lua_pushnumber(L, 0);
		return 1;
	}

#ifdef __DICE_SYSTEM__
	ALUA(pc_give_or_drop_item_with_dice)
	{
		if (!lua_isstring(L, 1) && !lua_isnumber(L, 1))
		{
			sys_err("QUEST Make item call error : wrong argument");
			lua_pushnumber(L, 0);
			return 1;
		}

		DWORD dwVnum;

		if (lua_isnumber(L, 1))
		{
			dwVnum = (int)lua_tonumber(L, 1);
		}
		else if (!ITEM_MANAGER::instance().GetVnum(lua_tostring(L, 1), dwVnum))
		{
			sys_err("QUEST Make item call error : wrong item name : %s", lua_tostring(L, 1));
			lua_pushnumber(L, 0);

			return 1;
		}

		int icount = 1;
		if (lua_isnumber(L, 2) && lua_tonumber(L, 2) > 0)
		{
			icount = (int)rint(lua_tonumber(L, 2));
			if (icount <= 0)
			{
				sys_err("QUEST Make item call error : wrong item count : %g", lua_tonumber(L, 2));
				lua_pushnumber(L, 0);
				return 1;
			}
		}

		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		LPITEM item = ITEM_MANAGER::instance().CreateItem(dwVnum, icount);
		if (ch->GetParty())
		{
			FPartyDropDiceRoll f(item, ch);
			f.Process(NULL);
			f.GetItemOwner()->AutoGiveItem(item);
		}
		else
			ch->AutoGiveItem(item);

		sys_log(0, "QUEST [REWARD] item %s to %s", lua_tostring(L, 1), ch->GetName());

		lua_pushnumber(L, (item) ? item->GetID() : 0);
		return 1;
	}
#endif

	ALUA(pc_give_or_drop_item_and_select)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();

		if (!lua_isstring(L, 1) && !lua_isnumber(L, 1))
		{
			sys_err("QUEST Make item call error : wrong argument");
			return 0;
		}

		DWORD dwVnum;

		if (lua_isnumber(L, 1))
		{
			dwVnum = (int)lua_tonumber(L, 1);
		}
		else if (!ITEM_MANAGER::instance().GetVnum(lua_tostring(L, 1), dwVnum))
		{
			sys_err("QUEST Make item call error : wrong item name : %s", lua_tostring(L, 1));
			return 0;
		}

		int icount = 1;
		if (lua_isnumber(L, 2) && lua_tonumber(L, 2) > 0)
		{
			icount = (int)rint(lua_tonumber(L, 2));
			if (icount <= 0)
			{
				sys_err("QUEST Make item call error : wrong item count : %g", lua_tonumber(L, 2));
				return 0;
			}
		}

		sys_log(0, "QUEST [REWARD] item %s to %s", lua_tostring(L, 1), ch->GetName());

		LPITEM item = ch->AutoGiveItem(dwVnum, icount);

		if (NULL != item)
			CQuestManager::Instance().SetCurrentItem(item);

		return 0;
	}

	ALUA(pc_get_current_map_index)
	{
		lua_pushnumber(L, CQuestManager::instance().GetCurrentCharacterPtr()->GetMapIndex());
		return 1;
	}

	ALUA(pc_get_x)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		lua_pushnumber(L, ch->GetX() / 100);
		return 1;
	}

	ALUA(pc_get_y)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		lua_pushnumber(L, ch->GetY() / 100);
		return 1;
	}

	ALUA(pc_get_local_x)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		LPSECTREE_MAP pMap = SECTREE_MANAGER::instance().GetMap(ch->GetMapIndex());

		if (pMap)
			lua_pushnumber(L, (ch->GetX() - pMap->m_setting.iBaseX) / 100);
		else
			lua_pushnumber(L, ch->GetX() / 100);

		return 1;
	}

	ALUA(pc_get_local_y)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		LPSECTREE_MAP pMap = SECTREE_MANAGER::instance().GetMap(ch->GetMapIndex());

		if (pMap)
			lua_pushnumber(L, (ch->GetY() - pMap->m_setting.iBaseY) / 100);
		else
			lua_pushnumber(L, ch->GetY() / 100);

		return 1;
	}

	ALUA(pc_count_item)
	{
		if (lua_isnumber(L, -1))
			lua_pushnumber(L, CQuestManager::instance().GetCurrentCharacterPtr()->CountSpecifyItem((DWORD)lua_tonumber(L, -1)));
		else if (lua_isstring(L, -1))
		{
			DWORD item_vnum;

			if (!ITEM_MANAGER::instance().GetVnum(lua_tostring(L, 1), item_vnum))
			{
				sys_err("QUEST count_item call error : wrong item name : %s", lua_tostring(L, 1));
				lua_pushnumber(L, 0);
			}
			else
			{
				lua_pushnumber(L, CQuestManager::instance().GetCurrentCharacterPtr()->CountSpecifyItem(item_vnum));
			}
		}
		else
			lua_pushnumber(L, 0);

		return 1;
	}

	ALUA(pc_remove_item)
	{
		if (lua_gettop(L) == 1)
		{
			DWORD item_vnum;

			if (lua_isnumber(L, 1))
			{
				item_vnum = (DWORD)lua_tonumber(L, 1);
			}
			else if (lua_isstring(L, 1))
			{
				if (!ITEM_MANAGER::instance().GetVnum(lua_tostring(L, 1), item_vnum))
				{
					sys_err("QUEST remove_item call error : wrong item name : %s", lua_tostring(L, 1));
					return 0;
				}
			}
			else
			{
				sys_err("QUEST remove_item wrong argument");
				return 0;
			}

			sys_log(0, "QUEST remove a item vnum %d of %s[%d]", item_vnum, CQuestManager::instance().GetCurrentCharacterPtr()->GetName(), CQuestManager::instance().GetCurrentCharacterPtr()->GetPlayerID());
			CQuestManager::instance().GetCurrentCharacterPtr()->RemoveSpecifyItem(item_vnum);
		}
		else if (lua_gettop(L) == 2)
		{
			DWORD item_vnum;

			if (lua_isnumber(L, 1))
			{
				item_vnum = (DWORD)lua_tonumber(L, 1);
			}
			else if (lua_isstring(L, 1))
			{
				if (!ITEM_MANAGER::instance().GetVnum(lua_tostring(L, 1), item_vnum))
				{
					sys_err("QUEST remove_item call error : wrong item name : %s", lua_tostring(L, 1));
					return 0;
				}
			}
			else
			{
				sys_err("QUEST remove_item wrong argument");
				return 0;
			}

			DWORD item_count = (DWORD)lua_tonumber(L, 2);
			sys_log(0, "QUEST remove items(vnum %d) count %d of %s[%d]",
				item_vnum,
				item_count,
				CQuestManager::instance().GetCurrentCharacterPtr()->GetName(),
				CQuestManager::instance().GetCurrentCharacterPtr()->GetPlayerID());

			CQuestManager::instance().GetCurrentCharacterPtr()->RemoveSpecifyItem(item_vnum, item_count);
		}
		return 0;
	}

	ALUA(pc_get_leadership)
	{
		lua_pushnumber(L, CQuestManager::instance().GetCurrentCharacterPtr()->GetLeadershipSkillLevel());
		return 1;
	}

	ALUA(pc_reset_point)
	{
		CQuestManager::instance().GetCurrentCharacterPtr()->ResetPoint(CQuestManager::instance().GetCurrentCharacterPtr()->GetLevel());
		return 0;
	}

	ALUA(pc_get_playtime)
	{
		lua_pushnumber(L, CQuestManager::instance().GetCurrentCharacterPtr()->GetRealPoint(POINT_PLAYTIME));
		return 1;
	}

	ALUA(pc_get_vid)
	{
		lua_pushnumber(L, CQuestManager::instance().GetCurrentCharacterPtr()->GetVID());
		return 1;
	}
	ALUA(pc_get_name)
	{
		lua_pushstring(L, CQuestManager::instance().GetCurrentCharacterPtr()->GetName());
		return 1;
	}

	ALUA(pc_get_next_exp)
	{
		lua_pushnumber(L, CQuestManager::instance().GetCurrentCharacterPtr()->GetNextExp());
		return 1;
	}

	ALUA(pc_get_exp)
	{
		lua_pushnumber(L, CQuestManager::instance().GetCurrentCharacterPtr()->GetExp());
		return 1;
	}

	ALUA(pc_get_race)
	{
		lua_pushnumber(L, CQuestManager::instance().GetCurrentCharacterPtr()->GetRaceNum());
		return 1;
	}

	ALUA(pc_change_sex)
	{
		lua_pushnumber(L, CQuestManager::instance().GetCurrentCharacterPtr()->ChangeSex());
		return 1;
	}

	ALUA(pc_get_job)
	{
		lua_pushnumber(L, CQuestManager::instance().GetCurrentCharacterPtr()->GetJob());
		return 1;
	}

	ALUA(pc_get_max_sp)
	{
		lua_pushnumber(L, CQuestManager::instance().GetCurrentCharacterPtr()->GetMaxSP());
		return 1;
	}

	ALUA(pc_get_sp)
	{
		lua_pushnumber(L, CQuestManager::instance().GetCurrentCharacterPtr()->GetSP());
		return 1;
	}

	ALUA(pc_change_sp)
	{
		if (!lua_isnumber(L, 1))
		{
			sys_err("invalid argument");
			lua_pushboolean(L, 0);
			return 1;
		}

		long val = (long)lua_tonumber(L, 1);

		if (val == 0)
		{
			lua_pushboolean(L, 0);
			return 1;
		}

		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();

		if (val > 0)
			ch->PointChange(POINT_SP, val);
		else if (val < 0)
		{
			if (ch->GetSP() < -val)
			{
				lua_pushboolean(L, 0);
				return 1;
			}

			ch->PointChange(POINT_SP, val);
		}

		lua_pushboolean(L, 1);
		return 1;
	}

	ALUA(pc_get_max_hp)
	{
		lua_pushnumber(L, CQuestManager::instance().GetCurrentCharacterPtr()->GetMaxHP());
		return 1;
	}

	ALUA(pc_get_hp)
	{
		lua_pushnumber(L, CQuestManager::instance().GetCurrentCharacterPtr()->GetHP());
		return 1;
	}

	ALUA(pc_get_level)
	{
		lua_pushnumber(L, CQuestManager::instance().GetCurrentCharacterPtr()->GetLevel());
		return 1;
	}

	ALUA(pc_set_level)
	{
		if (!lua_isnumber(L, 1))
		{
			sys_err("invalid argument");
			return 0;
		}
		else
		{
			int newLevel = lua_tonumber(L, 1);
			LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();

			sys_log(0, "QUEST [LEVEL] %s jumpint to level %d", ch->GetName(), (int)rint(lua_tonumber(L, 1)));

			ch->PointChange(POINT_SKILL, newLevel - ch->GetLevel());
			ch->PointChange(POINT_SUB_SKILL, newLevel < 10 ? 0 : newLevel - MAX(ch->GetLevel(), 9));
			ch->PointChange(POINT_STAT, ((MINMAX(1, newLevel, gPlayerMaxLevel) - ch->GetLevel()) * 3) + ch->GetPoint(POINT_LEVEL_STEP));

			ch->SetExp(0);
			ch->PointChange(POINT_LEVEL, newLevel - ch->GetLevel());
			//HP, SP
			ch->SetRandomHP((newLevel - 1) * number(JobInitialPoints[ch->GetJob()].hp_per_lv_begin, JobInitialPoints[ch->GetJob()].hp_per_lv_end));
			ch->SetRandomSP((newLevel - 1) * number(JobInitialPoints[ch->GetJob()].sp_per_lv_begin, JobInitialPoints[ch->GetJob()].sp_per_lv_end));

			ch->PointChange(POINT_HP, ch->GetMaxHP() - ch->GetHP());
			ch->PointChange(POINT_SP, ch->GetMaxSP() - ch->GetSP());

			ch->ComputePoints();
			ch->PointsPacket();
			ch->SkillLevelPacket();

			return 0;
		}
	}

	ALUA(pc_get_weapon)
	{
		LPITEM item = CQuestManager::instance().GetCurrentCharacterPtr()->GetWear(WEAR_WEAPON);

		if (!item)
			lua_pushnumber(L, 0);
		else
			lua_pushnumber(L, item->GetVnum());

		return 1;
	}

	ALUA(pc_get_armor)
	{
		LPITEM item = CQuestManager::instance().GetCurrentCharacterPtr()->GetWear(WEAR_BODY);

		if (!item)
			lua_pushnumber(L, 0);
		else
			lua_pushnumber(L, item->GetVnum());

		return 1;
	}

	ALUA(pc_get_wear)
	{
		if (!lua_isnumber(L, 1))
		{
			sys_err("QUEST wrong set flag");
			return 0;
		}

		BYTE bCell = (BYTE)lua_tonumber(L, 1);

		LPITEM item = CQuestManager::instance().GetCurrentCharacterPtr()->GetWear(bCell);

		if (!item)
			lua_pushnil(L);
		else
			lua_pushnumber(L, item->GetVnum());

		return 1;
	}

	ALUA(pc_get_money)
	{
		lua_pushnumber(L, CQuestManager::instance().GetCurrentCharacterPtr()->GetGold());
		return 1;
	}

	ALUA(pc_get_real_alignment)
	{
		lua_pushnumber(L, CQuestManager::instance().GetCurrentCharacterPtr()->GetRealAlignment() / 10);
		return 1;
	}

	ALUA(pc_get_alignment)
	{
		lua_pushnumber(L, CQuestManager::instance().GetCurrentCharacterPtr()->GetAlignment() / 10);
		return 1;
	}

	ALUA(pc_change_alignment)
	{
		int alignment = (int)(lua_tonumber(L, 1) * 10);
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();

		if (ch) // @DIKKAT @duzenleme buradaki kontrollerin hepsini alip lua process olayina tasimamiz lazim.
#ifdef __ALIGNMENT_REWORK__
			ch->UpdateAlignment(alignment, lua_toboolean(L, 2));
#else
			ch->UpdateAlignment(alignment);
#endif
		return 0;
	}

	ALUA(pc_change_money)
	{
#ifdef __GOLD_LIMIT_REWORK__
		long long gold = (long long)lua_tonumber(L, -1);

		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();

		if (gold + ch->GetGold() < 0)
			sys_err("QUEST wrong ChangeGold %lld (now %lld)", gold, ch->GetGold());
#else
		int gold = (int)lua_tonumber(L, -1);

		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();

		if (gold + ch->GetGold() < 0)
			sys_err("QUEST wrong ChangeGold %d (now %d)", gold, ch->GetGold());
#endif
		else
		{
			ch->PointChange(POINT_GOLD, gold, true);
		}

		return 0;
	}

	ALUA(pc_set_another_quest_flag)
	{
		if (!lua_isstring(L, 1) || !lua_isstring(L, 2) || !lua_isnumber(L, 3))
		{
			sys_err("QUEST wrong set flag");
			return 0;
		}
		else
		{
			const char* sz = lua_tostring(L, 1);
			const char* sz2 = lua_tostring(L, 2);
			CQuestManager& q = CQuestManager::Instance();
			PC* pPC = q.GetCurrentPC();
			pPC->SetFlag(string(sz) + "." + sz2, int(rint(lua_tonumber(L, 3))));
			return 0;
		}
	}

	ALUA(pc_get_another_quest_flag)
	{
		if (!lua_isstring(L, 1) || !lua_isstring(L, 2))
		{
			sys_err("QUEST wrong get flag");
			return 0;
		}
		else
		{
			const char* sz = lua_tostring(L, 1);
			const char* sz2 = lua_tostring(L, 2);
			CQuestManager& q = CQuestManager::Instance();
			PC* pPC = q.GetCurrentPC();
			if (!pPC)
			{
				return 0;
			}
			lua_pushnumber(L, pPC->GetFlag(string(sz) + "." + sz2));
			return 1;
		}
	}

	ALUA(pc_get_flag)
	{
		if (!lua_isstring(L, -1))
		{
			sys_err("QUEST wrong get flag");
			return 0;
		}
		else
		{
			const char* sz = lua_tostring(L, -1);
			CQuestManager& q = CQuestManager::Instance();
			PC* pPC = q.GetCurrentPC();
			lua_pushnumber(L, pPC->GetFlag(sz));
			return 1;
		}
	}

	ALUA(pc_get_quest_flag)
	{
		if (!lua_isstring(L, -1))
		{
			sys_err("QUEST wrong get flag");
			return 0;
		}
		else
		{
			const char* sz = lua_tostring(L, -1);
			CQuestManager& q = CQuestManager::Instance();
			PC* pPC = q.GetCurrentPC();
			lua_pushnumber(L, pPC->GetFlag(pPC->GetCurrentQuestName() + "." + sz));
			if (test_server)
				sys_log(0, "GetQF ( %s . %s )", pPC->GetCurrentQuestName().c_str(), sz);
		}
		return 1;
	}

	ALUA(pc_set_flag)
	{
		if (!lua_isstring(L, 1) || !lua_isnumber(L, 2))
		{
			sys_err("QUEST wrong set flag");
		}
		else
		{
			const char* sz = lua_tostring(L, 1);
			CQuestManager& q = CQuestManager::Instance();
			PC* pPC = q.GetCurrentPC();
			pPC->SetFlag(sz, int(rint(lua_tonumber(L, 2))));
		}
		return 0;
	}

	ALUA(pc_set_quest_flag)
	{
		if (!lua_isstring(L, 1) || !lua_isnumber(L, 2))
		{
			sys_err("QUEST wrong set flag");
		}
		else
		{
			const char* sz = lua_tostring(L, 1);
			CQuestManager& q = CQuestManager::Instance();
			PC* pPC = q.GetCurrentPC();
			pPC->SetFlag(pPC->GetCurrentQuestName() + "." + sz, int(rint(lua_tonumber(L, 2))));
		}
		return 0;
	}

	ALUA(pc_del_quest_flag)
	{
		if (!lua_isstring(L, 1))
		{
			sys_err("argument error");
			return 0;
		}

		const char* sz = lua_tostring(L, 1);
		PC* pPC = CQuestManager::instance().GetCurrentPC();
		pPC->DeleteFlag(pPC->GetCurrentQuestName() + "." + sz);
		return 0;
	}

	ALUA(pc_give_exp2)
	{
		CQuestManager& q = CQuestManager::instance();
		LPCHARACTER ch = q.GetCurrentCharacterPtr();
		if (!lua_isnumber(L, 1))
			return 0;

		sys_log(0, "QUEST [REWARD] %s give exp2 %d", ch->GetName(), (int)rint(lua_tonumber(L, 1)));

		DWORD exp = (DWORD)rint(lua_tonumber(L, 1));

		ch->PointChange(POINT_EXP, exp);
		return 0;
	}

	ALUA(pc_give_exp)
	{
		if (!lua_isstring(L, 1) || !lua_isnumber(L, 2))
			return 0;

		CQuestManager& q = CQuestManager::instance();
		LPCHARACTER ch = q.GetCurrentCharacterPtr();

		sys_log(0, "QUEST [REWARD] %s give exp %s %d", ch->GetName(), lua_tostring(L, 1), (int)rint(lua_tonumber(L, 2)));

		DWORD exp = (DWORD)rint(lua_tonumber(L, 2));

		PC* pPC = CQuestManager::instance().GetCurrentPC();

		pPC->GiveExp(lua_tostring(L, 1), exp);
		return 0;
	}

	ALUA(pc_give_exp_perc)
	{
		CQuestManager& q = CQuestManager::instance();
		LPCHARACTER ch = q.GetCurrentCharacterPtr();

		if (!ch || !lua_isstring(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3))
			return 0;

		int lev = (int)rint(lua_tonumber(L, 2));
		double proc = (lua_tonumber(L, 3));

		sys_log(0, "QUEST [REWARD] %s give exp %s lev %d percent %g%%", ch->GetName(), lua_tostring(L, 1), lev, proc);

		DWORD exp = (DWORD)((exp_table[MINMAX(0, lev, PLAYER_MAX_LEVEL_CONST)] * proc) / 100);
		PC* pPC = CQuestManager::instance().GetCurrentPC();

		pPC->GiveExp(lua_tostring(L, 1), exp);
		return 0;
	}

	ALUA(pc_get_empire)
	{
		lua_pushnumber(L, CQuestManager::instance().GetCurrentCharacterPtr()->GetEmpire());
		return 1;
	}

	ALUA(pc_get_part)
	{
		CQuestManager& q = CQuestManager::instance();
		LPCHARACTER ch = q.GetCurrentCharacterPtr();
		if (!lua_isnumber(L, 1))
		{
			lua_pushnumber(L, 0);
			return 1;
		}
		int part_idx = (int)lua_tonumber(L, 1);
		lua_pushnumber(L, ch->GetPart(part_idx));
		return 1;
	}

	ALUA(pc_set_part)
	{
		CQuestManager& q = CQuestManager::instance();
		LPCHARACTER ch = q.GetCurrentCharacterPtr();
		if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2))
		{
			return 0;
		}
		int part_idx = (int)lua_tonumber(L, 1);
		int part_value = (int)lua_tonumber(L, 2);
		ch->SetPart(part_idx, part_value);
		ch->UpdatePacket();
		return 0;
	}

	ALUA(pc_get_skillgroup)
	{
		lua_pushnumber(L, CQuestManager::instance().GetCurrentCharacterPtr()->GetSkillGroup());
		return 1;
	}

	ALUA(pc_set_skillgroup)
	{
		if (!lua_isnumber(L, 1))
			sys_err("QUEST wrong skillgroup number");
		else
		{
			CQuestManager& q = CQuestManager::Instance();
			LPCHARACTER ch = q.GetCurrentCharacterPtr();
			ch->RemoveOnlySkillAffect();
			ch->SetSkillGroup((BYTE)rint(lua_tonumber(L, 1)));
		}
		return 0;
	}

	ALUA(pc_is_mount)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		lua_pushboolean(L, ch->GetMountVnum());
		return 1;
	}

	ALUA(pc_mount)
	{
		if (!lua_isnumber(L, 1))
			return 0;

		int length = 60;

		if (lua_isnumber(L, 2))
			length = (int)lua_tonumber(L, 2);

		DWORD mount_vnum = (DWORD)lua_tonumber(L, 1);

		if (length < 0)
			length = 60;

		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();

		ch->RemoveAffect(AFFECT_MOUNT);
		ch->RemoveAffect(AFFECT_MOUNT_BONUS);

		if (g_NoMountAtGuildWar && ch->GetWarMap())
		{
			if (ch->IsRiding())
				ch->StopRiding();
			return 0;
		}

		if (ch->GetHorse())
			ch->HorseSummon(false);

		if (mount_vnum)
		{
			ch->AddAffect(AFFECT_MOUNT, POINT_MOUNT, mount_vnum, AFF_NONE, length, 0, true);
			switch (mount_vnum)
			{
			case 20201:
			case 20202:
			case 20203:
			case 20204:
			case 20213:
			case 20216:
				ch->AddAffect(AFFECT_MOUNT, POINT_MOV_SPEED, 30, AFF_NONE, length, 0, true, true);
				break;

			case 20205:
			case 20206:
			case 20207:
			case 20208:
			case 20214:
			case 20217:
				ch->AddAffect(AFFECT_MOUNT, POINT_MOV_SPEED, 40, AFF_NONE, length, 0, true, true);
				break;

			case 20209:
			case 20210:
			case 20211:
			case 20212:
			case 20215:
			case 20218:
				ch->AddAffect(AFFECT_MOUNT, POINT_MOV_SPEED, 50, AFF_NONE, length, 0, true, true);
				break;
			}
		}

		return 0;
	}

	ALUA(pc_mount_bonus)
	{
		BYTE applyOn = static_cast<BYTE>(lua_tonumber(L, 1));
		long value = static_cast<long>(lua_tonumber(L, 2));
		long duration = static_cast<long>(lua_tonumber(L, 3));

		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();

		if (NULL != ch)
		{
			// @duzenleme
			// eger karakter binege binmemis ise binek bonusu yuklenmeyecek.
			if (!ch->GetMountVnum())
				return 0;
			ch->RemoveAffect(AFFECT_MOUNT_BONUS);
			ch->AddAffect(AFFECT_MOUNT_BONUS, aApplyInfo[applyOn].bPointType, value, AFF_NONE, duration, 0, false);
		}

		return 0;
	}

	ALUA(pc_unmount)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		ch->RemoveAffect(AFFECT_MOUNT);
		ch->RemoveAffect(AFFECT_MOUNT_BONUS);
		if (ch->IsHorseRiding())
			ch->StopRiding();
		return 0;
	}

	ALUA(pc_get_horse_level)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		lua_pushnumber(L, ch->GetHorseLevel());
		return 1;
	}

	ALUA(pc_get_horse_hp)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		if (ch->GetHorseLevel())
			lua_pushnumber(L, ch->GetHorseHealth());
		else
			lua_pushnumber(L, 0);

		return 1;
	}

	ALUA(pc_get_horse_stamina)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		if (ch->GetHorseLevel())
			lua_pushnumber(L, ch->GetHorseStamina());
		else
			lua_pushnumber(L, 0);

		return 1;
	}

	ALUA(pc_is_horse_alive)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		lua_pushboolean(L, ch->GetHorseLevel() > 0 && ch->GetHorseHealth() > 0);
		return 1;
	}

	ALUA(pc_revive_horse)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		ch->ReviveHorse();
		return 0;
	}

	ALUA(pc_have_map_scroll)
	{
		if (!lua_isstring(L, 1))
		{
			lua_pushboolean(L, 0);
			return 1;
		}

		const char* szMapName = lua_tostring(L, 1);
		const TMapRegion* region = SECTREE_MANAGER::instance().FindRegionByPartialName(szMapName);

		if (!region)
		{
			lua_pushboolean(L, 0);
			return 1;
		}

		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();

		bool bFind = false;
		for (int iCell = 0; iCell < INVENTORY_MAX_NUM; iCell++)
		{
			LPITEM item = ch->GetInventoryItem(iCell);
			if (!item)
				continue;

			if (item->GetType() == ITEM_USE &&
				item->GetSubType() == USE_TALISMAN &&
				(item->GetValue(0) == 1 || item->GetValue(0) == 2))
			{
				int x = item->GetSocket(0);
				int y = item->GetSocket(1);
				//if ((x-item_x)*(x-item_x)+(y-item_y)*(y-item_y)<r*r)
				if (region->sx <= x && region->sy <= y && x <= region->ex && y <= region->ey)
				{
					bFind = true;
					break;
				}
			}
		}

		lua_pushboolean(L, bFind);
		return 1;
	}

	ALUA(pc_get_war_map)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		lua_pushnumber(L, ch->GetWarMap() ? ch->GetWarMap()->GetMapIndex() : 0);
		return 1;
	}

	ALUA(pc_have_pos_scroll)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();

		if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2))
		{
			sys_err("invalid x y position");
			lua_pushboolean(L, 0);
			return 1;
		}

		if (!lua_isnumber(L, 2))
		{
			sys_err("invalid radius");
			lua_pushboolean(L, 0);
			return 1;
		}

		int x = (int)lua_tonumber(L, 1);
		int y = (int)lua_tonumber(L, 2);
		float r = (float)lua_tonumber(L, 3);

		bool bFind = false;
		for (int iCell = 0; iCell < INVENTORY_MAX_NUM; iCell++)
		{
			LPITEM item = ch->GetInventoryItem(iCell);
			if (!item)
				continue;

			if (item->GetType() == ITEM_USE &&
				item->GetSubType() == USE_TALISMAN &&
				(item->GetValue(0) == 1 || item->GetValue(0) == 2))
			{
				int item_x = item->GetSocket(0);
				int item_y = item->GetSocket(1);
				if ((x - item_x) * (x - item_x) + (y - item_y) * (y - item_y) < r * r)
				{
					bFind = true;
					break;
				}
			}
		}

		lua_pushboolean(L, bFind);
		return 1;
	}

	ALUA(pc_get_equip_refine_level)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		int cell = (int)lua_tonumber(L, 1);
		if (cell < 0 || cell >= WEAR_MAX_NUM)
		{
			sys_err("invalid wear position %d", cell);
			lua_pushnumber(L, 0);
			return 1;
		}

		LPITEM item = ch->GetWear(cell);
		if (!item)
		{
			lua_pushnumber(L, 0);
			return 1;
		}

		lua_pushnumber(L, item->GetRefineLevel());
		return 1;
	}

	ALUA(pc_refine_equip)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2))
		{
			sys_err("invalid argument");
			lua_pushboolean(L, 0);
			return 1;
		}

		int cell = (int)lua_tonumber(L, 1);
		int level_limit = (int)lua_tonumber(L, 2);
		int pct = lua_isnumber(L, 3) ? (int)lua_tonumber(L, 3) : 100;

		LPITEM item = ch->GetWear(cell);
		if (!item)
		{
			lua_pushboolean(L, 0);
			return 1;
		}

		if (item->GetRefinedVnum() == 0)
		{
			lua_pushboolean(L, 0);
			return 1;
		}

		if (item->GetRefineLevel() > level_limit)
		{
			lua_pushboolean(L, 0);
			return 1;
		}

		if (pct == 100 || number(1, 100) <= pct)
		{
			lua_pushboolean(L, 1);

			LPITEM pkNewItem = ITEM_MANAGER::instance().CreateItem(item->GetRefinedVnum(), 1, 0, false);

			if (pkNewItem)
			{
				for (int i = 0; i < ITEM_SOCKET_MAX_NUM; ++i)
					if (!item->GetSocket(i))
						break;
					else
						pkNewItem->SetSocket(i, 1);

				int set = 0;
				for (int i = 0; i < ITEM_SOCKET_MAX_NUM; i++)
				{
					long socket = item->GetSocket(i);
					if (socket > 2 && socket != 28960)
					{
						pkNewItem->SetSocket(set++, socket);
					}
				}

				item->CopyAttributeTo(pkNewItem);

				ITEM_MANAGER::instance().RemoveItem(item, "REMOVE (REFINE SUCCESS)");

				// some tuits need here -_- pkNewItem->__ADD_TO_CHARACTER(this, bCell);
				pkNewItem->EquipTo(ch, cell);

				ITEM_MANAGER::instance().FlushDelayedSave(pkNewItem);
			}
		}
		else
		{
			lua_pushboolean(L, 0);
		}

		return 1;
	}

	ALUA(pc_get_skill_level)
	{
		if (!lua_isnumber(L, 1))
		{
			sys_err("invalid argument");
			lua_pushnumber(L, 0);
			return 1;
		}

		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();

		DWORD dwVnum = (DWORD)lua_tonumber(L, 1);
		lua_pushnumber(L, ch->GetSkillLevel(dwVnum));

		return 1;
	}

	ALUA(pc_aggregate_monster)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		ch->AggregateMonster();
		return 0;
	}

#ifdef __STONE_DETECT_REWORK__
	ALUA(pc_aggregate_stone)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		lua_pushboolean(L, ch->AggregateStone());
		return 1;
	}
#endif

	ALUA(pc_is_moving)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		lua_pushboolean(L, ch->IsStateMove());
		return 1;
	}

	ALUA(pc_forget_my_attacker)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		ch->ForgetMyAttacker();
		return 0;
	}

	ALUA(pc_attract_ranger)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		ch->AttractRanger();
		return 0;
	}

	ALUA(pc_select_pid)
	{
		DWORD pid = (DWORD)lua_tonumber(L, 1);

		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		LPCHARACTER new_ch = CHARACTER_MANAGER::instance().FindByPID(pid);

		if (new_ch)
		{
			CQuestManager::instance().GetPC(new_ch->GetPlayerID());

			lua_pushnumber(L, ch ? ch->GetPlayerID() : 0); // @DIKKAT @duzenleme buradaki kontrollerin hepsini alip lua process olayina tasimamiz lazim.
		}
		else
		{
			lua_pushnumber(L, 0);
		}

		return 1;
	}

	ALUA(pc_select_vid)
	{
		DWORD vid = (DWORD)lua_tonumber(L, 1);

		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		LPCHARACTER new_ch = CHARACTER_MANAGER::instance().Find(vid);

		if (new_ch)
		{
			CQuestManager::instance().GetPC(new_ch->GetPlayerID());

			lua_pushnumber(L, ch ? (DWORD)ch->GetVID() : 0); // @DIKKAT @duzenleme buradaki kontrollerin hepsini alip lua process olayina tasimamiz lazim.
		}
		else
		{
			lua_pushnumber(L, 0);
		}

		return 1;
	}

	ALUA(pc_get_sex)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		lua_pushnumber(L, GET_SEX(ch)); /* 0==MALE, 1==FEMALE */
		return 1;
	}

	ALUA(pc_is_gm)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
#ifdef __GAME_MASTER_UTILITY__
		lua_pushboolean(L, ch->GetGMLevel() >= GM_IMPLEMENTOR);
#else
		lua_pushboolean(L, ch->GetGMLevel() >= GM_HIGH_WIZARD);
#endif
		return 1;
	}

	ALUA(pc_get_gm_level)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		lua_pushnumber(L, ch->GetGMLevel());
		return 1;
	}

	ALUA(pc_mining)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		LPCHARACTER npc = CQuestManager::instance().GetCurrentNPCCharacterPtr();
		ch->mining(npc);
		return 0;
	}

	ALUA(pc_diamond_refine)
	{
		if (!lua_isnumber(L, 1))
		{
			lua_pushboolean(L, 0);
			return 1;
		}

		int cost = (int)lua_tonumber(L, 1);
		int pct = (int)lua_tonumber(L, 2);

		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		LPCHARACTER npc = CQuestManager::instance().GetCurrentNPCCharacterPtr();
		LPITEM item = CQuestManager::instance().GetCurrentItem();

		if (item)
			lua_pushboolean(L, mining::OreRefine(ch, npc, item, cost, pct, NULL));
		else
			lua_pushboolean(L, 0);

		return 1;
	}

	ALUA(pc_ore_refine)
	{
		if (!lua_isnumber(L, 1))
		{
			lua_pushboolean(L, 0);
			return 1;
		}

		int cost = (int)lua_tonumber(L, 1);
		int pct = (int)lua_tonumber(L, 2);
		int metinstone_cell = (int)lua_tonumber(L, 3);

		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		LPCHARACTER npc = CQuestManager::instance().GetCurrentNPCCharacterPtr();
		LPITEM item = CQuestManager::instance().GetCurrentItem();

		LPITEM metinstone_item = ch->GetInventoryItem(metinstone_cell);

		if (item && metinstone_item)
			lua_pushboolean(L, mining::OreRefine(ch, npc, item, cost, pct, metinstone_item));
		else
			lua_pushboolean(L, 0);

		return 1;
	}

	ALUA(pc_clear_skill)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		if (ch == NULL) return 0;

		ch->ClearSkill();

		return 0;
	}

	ALUA(pc_clear_sub_skill)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		if (ch == NULL) return 0;

		ch->ClearSubSkill();

		return 0;
	}

	ALUA(pc_set_skill_point)
	{
		if (!lua_isnumber(L, 1))
		{
			return 0;
		}

		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		int newPoint = (int)lua_tonumber(L, 1);

		ch->SetRealPoint(POINT_SKILL, newPoint);
		ch->SetPoint(POINT_SKILL, ch->GetRealPoint(POINT_SKILL));
		ch->PointChange(POINT_SKILL, 0);
		ch->ComputePoints();
		ch->PointsPacket();

		return 0;
	}

	// RESET_ONE_SKILL
	ALUA(pc_clear_one_skill)
	{
		int vnum = (int)lua_tonumber(L, 1);
		sys_log(0, "%d skill clear", vnum);

		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		if (ch == NULL)
		{
			sys_log(0, "skill clear fail");
			lua_pushnumber(L, 0);
			return 1;
		}

		sys_log(0, "%d skill clear", vnum);

		ch->ResetOneSkill(vnum);

		return 0;
	}
	// END_RESET_ONE_SKILL

	ALUA(pc_is_clear_skill_group)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();

		lua_pushboolean(L, ch->GetQuestFlag("skill_group_clear.clear") == 1);

		return 1;
	}

	ALUA(pc_save_exit_location)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();

		ch->SaveExitLocation();

		return 0;
	}

	ALUA(pc_teleport)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		int x = 0, y = 0;
		if (lua_isnumber(L, 1))
		{
			const int TOWN_NUM = 10;
			struct warp_by_town_name
			{
				const char* name;
				DWORD x;
				DWORD y;
			} ws[TOWN_NUM] =
			{
				{"¿µ¾ÈÀ¾¼º",	4743,	9548},
				{"ÀÓÁö°î",		3235,	9086},
				{"ÀÚ¾çÇö",		3531,	8829},
				{"Á¶¾ÈÀ¾¼º",	638,	1664},
				{"½Â·æ°î",		1745,	1909},
				{"º¹Á¤Çö",		1455,	2400},
				{"Æò¹«À¾¼º",	9599,	2692},
				{"¹æ»ê°î",		8036,	2984},
				{"¹Ú¶óÇö",		8639,	2460},
				{"¼­ÇÑ»ê",		4350,	2143},
			};
			int idx = (int)lua_tonumber(L, 1);

			x = ws[idx].x;
			y = ws[idx].y;
			goto teleport_area;
		}

		else
		{
			const char* arg1 = lua_tostring(L, 1);

			LPCHARACTER tch = CHARACTER_MANAGER::instance().FindPC(arg1);

			if (!tch)
			{
				const CCI* pkCCI = P2P_MANAGER::instance().Find(arg1);

				if (pkCCI)
				{
					if (pkCCI->bChannel != g_bChannel)
					{
						ch->ChatPacket(CHAT_TYPE_INFO, "Target is in %d channel (my channel %d)", pkCCI->bChannel, g_bChannel);
					}
					else
					{
						PIXEL_POSITION pos;

						if (!SECTREE_MANAGER::instance().GetCenterPositionOfMap(pkCCI->lMapIndex, pos))
						{
							ch->ChatPacket(CHAT_TYPE_INFO, "Cannot find map (index %d)", pkCCI->lMapIndex);
						}
						else
						{
							ch->ChatPacket(CHAT_TYPE_INFO, "You warp to ( %d, %d )", pos.x, pos.y);
							ch->WarpSet(pos.x, pos.y);
							lua_pushnumber(L, 1);
						}
					}
				}
				else if (NULL == CHARACTER_MANAGER::instance().FindPC(arg1))
				{
					ch->ChatPacket(CHAT_TYPE_INFO, "There is no one by that name");
				}

				lua_pushnumber(L, 0);

				return 1;
			}
			else
			{
				x = tch->GetX() / 100;
				y = tch->GetY() / 100;
			}
		}

	teleport_area:

		x *= 100;
		y *= 100;

		ch->ChatPacket(CHAT_TYPE_INFO, "You warp to ( %d, %d )", x, y);
		ch->WarpSet(x, y);
		ch->Stop();
		lua_pushnumber(L, 1);
		return 1;
	}

	ALUA(pc_set_skill_level)
	{
		DWORD dwVnum = (DWORD)lua_tonumber(L, 1);
		BYTE byLev = (BYTE)lua_tonumber(L, 2);

		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		ch->SetSkillLevel(dwVnum, byLev);

		ch->SkillLevelPacket();

		return 0;
	}

	ALUA(pc_get_premium_remain_sec)
	{
		int	remain_seconds = 0;
		int	premium_type = 0;
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();

		if (!lua_isnumber(L, 1))
		{
			sys_err("wrong premium index (is not number)");
			return 0;
		}

		premium_type = (int)lua_tonumber(L, 1);
		switch (premium_type)
		{
		case PREMIUM_EXP:
		case PREMIUM_ITEM:
		case PREMIUM_SAFEBOX:
		case PREMIUM_AUTOLOOT:
		case PREMIUM_FISH_MIND:
		case PREMIUM_GOLD:
			break;

		default:
			sys_err("wrong premium index %d", premium_type);
			return 0;
		}

		remain_seconds = ch->GetPremiumRemainSeconds(premium_type);

		lua_pushnumber(L, remain_seconds);
		return 1;
	}

	ALUA(pc_send_block_mode)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();

		ch->SetBlockModeForce((BYTE)lua_tonumber(L, 1));

		return 0;
	}

	ALUA(pc_change_empire)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		lua_pushnumber(L, ch->ChangeEmpire((unsigned char)lua_tonumber(L, 1)));

		return 1;
	}

	ALUA(pc_get_change_empire_count)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();

		lua_pushnumber(L, ch->GetChangeEmpireCount());

		return 1;
	}

	ALUA(pc_set_change_empire_count)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();

		ch->SetChangeEmpireCount();

		return 0;
	}

	ALUA(pc_change_name)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();

		if (ch->GetNewName().size() != 0)
		{
			lua_pushnumber(L, 0);
			return 1;
		}

		if (lua_isstring(L, 1) != true)
		{
			lua_pushnumber(L, 1);
			return 1;
		}

		const char* szName = lua_tostring(L, 1);

		if (check_name(szName) == false)
		{
			lua_pushnumber(L, 2);
			return 1;
		}

		char szQuery[1024];
		snprintf(szQuery, sizeof(szQuery), "SELECT COUNT(*) FROM player%s WHERE name='%s';", get_table_postfix(), szName);
		std::unique_ptr<SQLMsg> pmsg(DBManager::instance().DirectQuery(szQuery));

		if (pmsg->Get()->uiNumRows > 0)
		{
			MYSQL_ROW row = mysql_fetch_row(pmsg->Get()->pSQLResult);

			int	count = 0;
			str_to_number(count, row[0]);

			if (count != 0)
			{
				lua_pushnumber(L, 3);
				return 1;
			}
		}

		DWORD pid = ch->GetPlayerID();
		ch->Save();
		ch->SetExchangeTime();
		ch->GetDesc()->DelayedDisconnect(3);

		/* delete messenger list */
		MessengerManager::instance().RemoveAllList(ch->GetName());
		/* change_name_log */
		LogManager::instance().ChangeNameLog(pid, ch->GetName(), szName, ch->GetDesc()->GetHostName());

		snprintf(szQuery, sizeof(szQuery), "UPDATE player%s SET name='%s' WHERE id=%u;", get_table_postfix(), szName, pid);
		SQLMsg* msg = DBManager::instance().DirectQuery(szQuery);
		M2_DELETE(msg);

		ch->SetNewName(szName);
		lua_pushnumber(L, 4);
		return 1;
	}

	ALUA(pc_is_dead)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();

		if (ch != NULL)
		{
			lua_pushboolean(L, ch->IsDead());
			return 1;
		}

		lua_pushboolean(L, true);

		return 1;
	}

	ALUA(pc_reset_status)
	{
		if (lua_isnumber(L, 1) == true)
		{
			int idx = (int)lua_tonumber(L, 1);

			if (idx >= 0 && idx < 4)
			{
				LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
				int point = POINT_NONE;
				char buf[128];

				switch (idx)
				{
				case 0: point = POINT_HT; break;
				case 1: point = POINT_IQ; break;
				case 2: point = POINT_ST; break;
				case 3: point = POINT_DX; break;
				default: lua_pushboolean(L, false); return 1;
				}

				int old_val = ch->GetRealPoint(point);
				int old_stat = ch->GetRealPoint(POINT_STAT);

				ch->SetRealPoint(point, 1);
				ch->SetPoint(point, ch->GetRealPoint(point));

				ch->PointChange(POINT_STAT, old_val - 1);

				if (point == POINT_HT)
				{
					BYTE job = ch->GetJob();
					ch->SetRandomHP((ch->GetLevel() - 1) * number(JobInitialPoints[job].hp_per_lv_begin, JobInitialPoints[job].hp_per_lv_end));
				}
				else if (point == POINT_IQ)
				{
					BYTE job = ch->GetJob();
					ch->SetRandomSP((ch->GetLevel() - 1) * number(JobInitialPoints[job].sp_per_lv_begin, JobInitialPoints[job].sp_per_lv_end));
				}

				ch->ComputePoints();
				ch->PointsPacket();

				if (point == POINT_HT)
				{
					ch->PointChange(POINT_HP, ch->GetMaxHP() - ch->GetHP());
				}
				else if (point == POINT_IQ)
				{
					ch->PointChange(POINT_SP, ch->GetMaxSP() - ch->GetSP());
				}

				switch (idx)
				{
				case 0:
					snprintf(buf, sizeof(buf), "reset ht(%d)->1 stat_point(%d)->(%lld)", old_val, old_stat, ch->GetRealPoint(POINT_STAT));
					break;
				case 1:
					snprintf(buf, sizeof(buf), "reset iq(%d)->1 stat_point(%d)->(%lld)", old_val, old_stat, ch->GetRealPoint(POINT_STAT));
					break;
				case 2:
					snprintf(buf, sizeof(buf), "reset st(%d)->1 stat_point(%d)->(%lld)", old_val, old_stat, ch->GetRealPoint(POINT_STAT));
					break;
				case 3:
					snprintf(buf, sizeof(buf), "reset dx(%d)->1 stat_point(%d)->(%lld)", old_val, old_stat, ch->GetRealPoint(POINT_STAT));
					break;
				}

				lua_pushboolean(L, true);
				return 1;
			}
		}

		lua_pushboolean(L, false);
		return 1;
	}

	ALUA(pc_get_ht)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		lua_pushnumber(L, ch->GetRealPoint(POINT_HT));
		return 1;
	}

	ALUA(pc_set_ht)
	{
		if (lua_isnumber(L, 1) == false)
			return 1;

		int newPoint = (int)lua_tonumber(L, 1);

		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		int usedPoint = newPoint - ch->GetRealPoint(POINT_HT);
		ch->SetRealPoint(POINT_HT, newPoint);
		ch->PointChange(POINT_HT, 0);
		ch->PointChange(POINT_STAT, -usedPoint);
		ch->ComputePoints();
		ch->PointsPacket();
		return 1;
	}

	ALUA(pc_get_iq)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		lua_pushnumber(L, ch->GetRealPoint(POINT_IQ));
		return 1;
	}

	ALUA(pc_set_iq)
	{
		if (lua_isnumber(L, 1) == false)
			return 1;

		int newPoint = (int)lua_tonumber(L, 1);

		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		int usedPoint = newPoint - ch->GetRealPoint(POINT_IQ);
		ch->SetRealPoint(POINT_IQ, newPoint);
		ch->PointChange(POINT_IQ, 0);
		ch->PointChange(POINT_STAT, -usedPoint);
		ch->ComputePoints();
		ch->PointsPacket();
		return 1;
	}

	ALUA(pc_get_st)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		lua_pushnumber(L, ch->GetRealPoint(POINT_ST));
		return 1;
	}

	ALUA(pc_set_st)
	{
		if (lua_isnumber(L, 1) == false)
			return 1;

		int newPoint = (int)lua_tonumber(L, 1);

		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		int usedPoint = newPoint - ch->GetRealPoint(POINT_ST);
		ch->SetRealPoint(POINT_ST, newPoint);
		ch->PointChange(POINT_ST, 0);
		ch->PointChange(POINT_STAT, -usedPoint);
		ch->ComputePoints();
		ch->PointsPacket();
		return 1;
	}

	ALUA(pc_get_dx)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		lua_pushnumber(L, ch->GetRealPoint(POINT_DX));
		return 1;
	}

	ALUA(pc_set_dx)
	{
		if (lua_isnumber(L, 1) == false)
			return 1;

		int newPoint = (int)lua_tonumber(L, 1);

		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		int usedPoint = newPoint - ch->GetRealPoint(POINT_DX);
		ch->SetRealPoint(POINT_DX, newPoint);
		ch->PointChange(POINT_DX, 0);
		ch->PointChange(POINT_STAT, -usedPoint);
		ch->ComputePoints();
		ch->PointsPacket();
		return 1;
	}

	ALUA(pc_is_near_vid)
	{
		if (lua_isnumber(L, 1) != true || lua_isnumber(L, 2) != true)
		{
			lua_pushboolean(L, false);
		}
		else
		{
			LPCHARACTER pMe = CQuestManager::instance().GetCurrentCharacterPtr();
			LPCHARACTER pOther = CHARACTER_MANAGER::instance().Find((DWORD)lua_tonumber(L, 1));

			if (pMe != NULL && pOther != NULL)
			{
				lua_pushboolean(L, (DISTANCE_APPROX(pMe->GetX() - pOther->GetX(), pMe->GetY() - pOther->GetY()) < (int)lua_tonumber(L, 2) * 100));
			}
			else
			{
				lua_pushboolean(L, false);
			}
		}

		return 1;
	}

	ALUA(pc_get_socket_items)
	{
		LPCHARACTER pChar = CQuestManager::instance().GetCurrentCharacterPtr();

		lua_newtable(L);

		if (pChar == NULL) return 1;

		int idx = 1;

		for (int i = 0; i < INVENTORY_MAX_NUM + WEAR_MAX_NUM; i++)
		{
			LPITEM pItem = pChar->GetInventoryItem(i);

			if (pItem != NULL)
			{
				if (pItem->IsEquipped() == false)
				{
					int j = 0;
					for (; j < ITEM_SOCKET_MAX_NUM; j++)
					{
						long socket = pItem->GetSocket(j);

						if (socket > 2 && socket != ITEM_BROKEN_METIN_VNUM)
						{
							TItemTable* pItemInfo = ITEM_MANAGER::instance().GetTable(socket);
							if (pItemInfo != NULL)
							{
								if (pItemInfo->bType == ITEM_METIN) break;
							}
						}
					}

					if (j >= ITEM_SOCKET_MAX_NUM) continue;

					lua_newtable(L);

					{
						lua_pushstring(L, pItem->GetName());
						lua_rawseti(L, -2, 1);

						lua_pushnumber(L, i);
						lua_rawseti(L, -2, 2);
					}

					lua_rawseti(L, -2, idx++);
				}
			}
		}

		return 1;
	}

	ALUA(pc_get_empty_inventory_count)
	{
		LPCHARACTER pChar = CQuestManager::instance().GetCurrentCharacterPtr();

		if (pChar != NULL)
		{
			lua_pushnumber(L, pChar->CountEmptyInventory());
		}
		else
		{
			lua_pushnumber(L, 0);
		}

		return 1;
	}

	ALUA(pc_get_logoff_interval)
	{
		LPCHARACTER pChar = CQuestManager::instance().GetCurrentCharacterPtr();

		if (pChar != NULL)
		{
			lua_pushnumber(L, pChar->GetLogOffInterval());
		}
		else
		{
			lua_pushnumber(L, 0);
		}

		return 1;
	}

	ALUA(pc_get_player_id)
	{
		LPCHARACTER pChar = CQuestManager::instance().GetCurrentCharacterPtr();

		if (pChar != NULL)
		{
			lua_pushnumber(L, pChar->GetPlayerID());
		}
		else
		{
			lua_pushnumber(L, 0);
		}

		return 1;
	}

	ALUA(pc_get_account_id)
	{
		LPCHARACTER pChar = CQuestManager::instance().GetCurrentCharacterPtr();

		if (pChar != NULL)
		{
			if (pChar->GetDesc() != NULL)
			{
				lua_pushnumber(L, pChar->GetDesc()->GetAccountTable().id);
				return 1;
			}
		}

		lua_pushnumber(L, 0);
		return 1;
	}

	ALUA(pc_get_account)
	{
		LPCHARACTER pChar = CQuestManager::instance().GetCurrentCharacterPtr();

		if (NULL != pChar)
		{
			if (NULL != pChar->GetDesc())
			{
				lua_pushstring(L, pChar->GetDesc()->GetAccountTable().login);
				return 1;
			}
		}

		lua_pushstring(L, "");
		return 1;
	}

	ALUA(pc_is_riding)
	{
		LPCHARACTER pChar = CQuestManager::instance().GetCurrentCharacterPtr();

		if (NULL != pChar)
		{
			bool is_riding = pChar->IsRiding();

			lua_pushboolean(L, is_riding);

			return 1;
		}

		lua_pushboolean(L, false);
		return 1;
	}

	ALUA(pc_get_special_ride_vnum)
	{
		LPCHARACTER pChar = CQuestManager::instance().GetCurrentCharacterPtr();

		if (NULL != pChar)
		{
			LPITEM Unique1 = pChar->GetWear(WEAR_UNIQUE1);
			LPITEM Unique2 = pChar->GetWear(WEAR_UNIQUE2);
			if (NULL != Unique1)
			{
				if (UNIQUE_GROUP_SPECIAL_RIDE == Unique1->GetSpecialGroup())
				{
					lua_pushnumber(L, Unique1->GetVnum());
					lua_pushnumber(L, Unique1->GetSocket(0)); // @duzenleme martySama adli arkadasin yaptigi birsey buralari hep kontrol etmek lazim.
					return 2;
				}
			}

			if (NULL != Unique2)
			{
				if (UNIQUE_GROUP_SPECIAL_RIDE == Unique2->GetSpecialGroup())
				{
					lua_pushnumber(L, Unique2->GetVnum());
					lua_pushnumber(L, Unique2->GetSocket(0)); // @duzenleme martySama adli arkadasin yaptigi birsey buralari hep kontrol etmek lazim.
					return 2;
				}
			}
		}

		lua_pushnumber(L, 0);
		lua_pushnumber(L, 0);

		return 2;
	}

	ALUA(pc_can_warp)
	{
		LPCHARACTER pChar = CQuestManager::instance().GetCurrentCharacterPtr();

		if (NULL != pChar)
		{
			lua_pushboolean(L, pChar->CanWarp());
		}
		else
		{
			lua_pushboolean(L, false);
		}

		return 1;
	}

	ALUA(pc_dec_skill_point)
	{
		LPCHARACTER pChar = CQuestManager::instance().GetCurrentCharacterPtr();

		if (NULL != pChar)
		{
			pChar->PointChange(POINT_SKILL, -1);
		}

		return 0;
	}

	ALUA(pc_get_skill_point)
	{
		LPCHARACTER pChar = CQuestManager::instance().GetCurrentCharacterPtr();

		if (NULL != pChar)
		{
			lua_pushnumber(L, pChar->GetPoint(POINT_SKILL));
		}
		else
		{
			lua_pushnumber(L, 0);
		}

		return 1;
	}

	ALUA(pc_get_channel_id)
	{
		lua_pushnumber(L, g_bChannel);

		return 1;
	}

	ALUA(pc_get_sig_items)
	{
		DWORD group_vnum = (DWORD)lua_tonumber(L, 1);
		const LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		int count = 0;
		for (int i = 0; i < INVENTORY_MAX_NUM; ++i)
		{
			if (ch->GetInventoryItem(i) != NULL && ch->GetInventoryItem(i)->GetSIGVnum() == group_vnum)
			{
				lua_pushnumber(L, ch->GetInventoryItem(i)->GetID());
				count++;
			}
		}

		return count;
	}

	ALUA(pc_charge_cash)
	{
		TRequestChargeCash packet;

		int amount = lua_isnumber(L, 1) ? (int)lua_tonumber(L, 1) : 0;
		std::string strChargeType = lua_isstring(L, 2) ? lua_tostring(L, 2) : "";

		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();

		if (NULL == ch || NULL == ch->GetDesc() || 1 > amount || 50000 < amount)
		{
			lua_pushboolean(L, 0);
			return 1;
		}

		packet.dwAID = ch->GetDesc()->GetAccountTable().id;
		packet.dwAmount = (DWORD)amount;
		packet.eChargeType = ERequestCharge_Cash;

		if (0 < strChargeType.length())
			std::transform(strChargeType.begin(), strChargeType.end(), strChargeType.begin(), (int(*)(int))std::tolower);

		if ("mileage" == strChargeType)
			packet.eChargeType = ERequestCharge_Mileage;

		db_clientdesc->DBPacketHeader(HEADER_GD_REQUEST_CHARGE_CASH, 0, sizeof(TRequestChargeCash));
		db_clientdesc->Packet(&packet, sizeof(packet));

		lua_pushboolean(L, 1);
		return 1;
	}

	ALUA(pc_get_killee_drop_pct)
	{
		LPCHARACTER pChar = CQuestManager::instance().GetCurrentCharacterPtr();
		LPCHARACTER pKillee = pChar->GetQuestNPC();

		int iDeltaPercent, iRandRange;
		if (NULL == pKillee || !ITEM_MANAGER::instance().GetDropPct(pKillee, pChar, iDeltaPercent, iRandRange))
		{
			sys_err("killee is NULL");
			lua_pushnumber(L, -1);
			lua_pushnumber(L, -1);

			return 2;
		}

		lua_pushnumber(L, iDeltaPercent);
		lua_pushnumber(L, iRandRange);

		return 2;
	}

	enum eMakeItemType { PCMI0_GIVE, PCMI0_DROP, PCMI0_DROPWP, PCMI0_MAX };
	ALUA(pc_make_item0)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();

		if (!lua_istable(L, 1) && !lua_istable(L, 2) && !lua_istable(L, 3) && !lua_isnumber(L, 4))
			return 0;

		int m_idx = 0;
		// config arg1
		DWORD m_vnum = 0;
		int m_count = 0;
		// start arg1
		lua_pushnil(L);
		while (lua_next(L, 1))
		{
			switch (m_idx)
			{
			case 0:
				if (lua_isnumber(L, -1))
				{
					if ((m_vnum = lua_tonumber(L, -1)) <= 0)
					{
						ch->ChatPacket(CHAT_TYPE_INFO, "#%u item not valid by that vnum.", m_vnum);
						return 0;
					}
				}
				else if (lua_isstring(L, -1))
				{
					if (!ITEM_MANAGER::instance().GetVnum(lua_tostring(L, -1), m_vnum))
					{
						ch->ChatPacket(CHAT_TYPE_INFO, "#%u item not exist by that vnum.", m_vnum);
						return 0;
					}
				}
				else
				{
					ch->ChatPacket(CHAT_TYPE_INFO, "#%u item not exist by that unknown vnum.");
					return 0;
				}
				break;
			case 1:
				if (lua_isnumber(L, -1))
				{
					// if ((m_count = MINMAX(1, lua_tonumber(L, -1), ITEM_MAX_COUNT))<=0)
					if ((m_count = lua_tonumber(L, -1)) <= 0)
					{
						ch->ChatPacket(CHAT_TYPE_INFO, "#%u item not valid by that count.", m_count);
						return 0;
					}
				}
				else
				{
					ch->ChatPacket(CHAT_TYPE_INFO, "#%u item not exist by that unknown count.");
					return 0;
				}
				break;
			default:
				ch->ChatPacket(CHAT_TYPE_INFO, "arg1(%d) index found", m_idx);
				break;
			}
			m_idx++;
			lua_pop(L, 1);
		}
		// ch->ChatPacket(CHAT_TYPE_INFO, "arg1 is %u %d", m_vnum, m_count);
		// end arg1
		// config arg2
		int m_socket[ITEM_SOCKET_MAX_NUM] = { 0 };
		// start arg2
		// ch->ChatPacket(CHAT_TYPE_INFO, "%d: %s", lua_type(L, 1), lua_typename(L, lua_type(L, 1)));
		m_idx = 0;
		lua_pushnil(L);
		while (lua_next(L, 2) && m_idx < ITEM_SOCKET_MAX_NUM)
		{
			if (!lua_isnumber(L, -1))
				return 0;
			m_socket[m_idx++] = lua_tonumber(L, -1);
			lua_pop(L, 1);
		}
		// ch->ChatPacket(CHAT_TYPE_INFO, "arg2 is %d %d %d", m_socket[0], m_socket[1], m_socket[2]);
		// end arg2
		// config arg3
		int m_attr[ITEM_ATTRIBUTE_MAX_NUM * 2] = { 0 };
		// start arg3
		m_idx = 0;
		lua_pushnil(L);
		while (lua_next(L, 3) && m_idx < (ITEM_ATTRIBUTE_MAX_NUM * 2))
		{
			if (!lua_isnumber(L, -1))
				return 0;
			m_attr[m_idx++] = lua_tonumber(L, -1);
			lua_pop(L, 1);
		}
		// ch->ChatPacket(CHAT_TYPE_INFO, "arg3 is %d %d %d %d %d %d %d %d %d %d %d %d %d %d",
			// m_attr[0], m_attr[1], m_attr[2], m_attr[3], m_attr[4], m_attr[5],
			// m_attr[6], m_attr[7], m_attr[8], m_attr[9], m_attr[10], m_attr[11],
			// m_attr[12], m_attr[13]
		// );
		// end arg3
		// config arg4
		DWORD m_state = 0;
		// start arg4
		if ((m_state = lua_tonumber(L, 4)) >= PCMI0_MAX)
			return 0;
		// ch->ChatPacket(CHAT_TYPE_INFO, "arg4 is %d", m_state);
		// end arg4

		// create item
		LPITEM pkNewItem = ITEM_MANAGER::instance().CreateItem(m_vnum, 1, 0, false);
		if (pkNewItem)
		{
			// socket
			for (int i = 0; i < ITEM_SOCKET_MAX_NUM; i++)
				pkNewItem->SetSocket(i, m_socket[i]);
			// attr
			for (int i = 0; i < ITEM_ATTRIBUTE_MAX_NUM; i++)
				pkNewItem->SetForceAttribute(i, m_attr[(i * 2) + 0], m_attr[(i * 2) + 1]);
			// state
			int iEmptyCell = -1;
			int m_sec = 0;
			PIXEL_POSITION pos;
			switch (m_state)
			{
			case PCMI0_GIVE:
				iEmptyCell = ch->GetEmptyInventory(pkNewItem->GetSize());
				if (-1 == iEmptyCell)
				{
					M2_DESTROY_ITEM(pkNewItem);
					lua_pushboolean(L, false);
					return 1;
				}
				pkNewItem->__ADD_TO_CHARACTER(ch, TItemPos(INVENTORY, iEmptyCell));
				break;
			case PCMI0_DROPWP:
				if (lua_isnumber(L, 5) && (m_sec = lua_tonumber(L, 5)))
					pkNewItem->SetOwnership(ch, m_sec <= 0 ? 1 : m_sec); //, ch->ChatPacket(CHAT_TYPE_INFO, "arg5 is %d", m_sec<=0?1:m_sec);
				else
					pkNewItem->SetOwnership(ch);
			case PCMI0_DROP:
				pos.x = ch->GetX() + number(-200, 200);
				pos.y = ch->GetY() + number(-200, 200);

				pkNewItem->AddToGround(ch->GetMapIndex(), pos);
				pkNewItem->StartDestroyEvent();
				break;
			default:
				lua_pushboolean(L, false);
				return 1;
			}
			lua_pushboolean(L, true);
		}
		else
			lua_pushboolean(L, false);

		return 1;
	}

	ALUA(pc_set_race0)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		int amount = MINMAX(0, lua_tonumber(L, 1), JOB_MAX_NUM);
		ESex mySex = GET_SEX(ch);
		DWORD dwRace = MAIN_RACE_WARRIOR_M;
		switch (amount)
		{
		case JOB_WARRIOR:
			dwRace = (mySex == SEX_MALE) ? MAIN_RACE_WARRIOR_M : MAIN_RACE_WARRIOR_W;
			break;
		case JOB_ASSASSIN:
			dwRace = (mySex == SEX_MALE) ? MAIN_RACE_ASSASSIN_M : MAIN_RACE_ASSASSIN_W;
			break;
		case JOB_SURA:
			dwRace = (mySex == SEX_MALE) ? MAIN_RACE_SURA_M : MAIN_RACE_SURA_W;
			break;
		case JOB_SHAMAN:
			dwRace = (mySex == SEX_MALE) ? MAIN_RACE_SHAMAN_M : MAIN_RACE_SHAMAN_W;
			break;
#ifdef __WOLFMAN_CHARACTER__
		case JOB_WOLFMAN:
			dwRace = (mySex == SEX_MALE) ? MAIN_RACE_WOLFMAN_M : MAIN_RACE_WOLFMAN_M;
			break;
#endif
		}
		if (dwRace != ch->GetRaceNum())
		{
			ch->SetRace(dwRace);
			ch->ClearSkill();
			if (!g_bDisableResetSubSkill)
				ch->ClearSubSkill();
			ch->SetSkillGroup(0);
		}
		return 0;
	}

	ALUA(pc_del_another_quest_flag)
	{
		if (!lua_isstring(L, 1) || !lua_isstring(L, 2))
		{
			sys_err("QUEST wrong del flag");
			return 0;
		}
		else
		{
			const char* sz = lua_tostring(L, 1);
			const char* sz2 = lua_tostring(L, 2);
			CQuestManager& q = CQuestManager::Instance();
			PC* pPC = q.GetCurrentPC();
			lua_pushboolean(L, pPC->DeleteFlag(string(sz) + "." + sz2));
			return 1;
		}
	}

	ALUA(pc_pointchange)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		ch->PointChange(lua_tonumber(L, 1), lua_tonumber(L, 2), lua_toboolean(L, 3), lua_toboolean(L, 4));
		return 0;
	}

	ALUA(pc_pullmob)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		ch->PullMonster();
		return 0;
	}

	ALUA(pc_set_level0)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();

		ch->ResetPoint(lua_tonumber(L, 1));
		ch->ClearSkill();
		ch->ClearSubSkill();
		return 0;
	}

	ALUA(pc_set_gm_level)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		ch->SetGMLevel();
		return 0;
	}

	ALUA(pc_if_fire)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		lua_pushboolean(L, ch->IsAffectFlag(AFF_FIRE));
		return 1;
	}
	ALUA(pc_if_invisible)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		lua_pushboolean(L, ch->IsAffectFlag(AFF_INVISIBILITY));
		return 1;
	}
	ALUA(pc_if_poison)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		lua_pushboolean(L, ch->IsAffectFlag(AFF_POISON));
		return 1;
	}
#ifdef __WOLFMAN_CHARACTER__
	ALUA(pc_if_bleeding)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		lua_pushboolean(L, ch->IsAffectFlag(AFF_BLEEDING));
		return 1;
	}
#endif
	ALUA(pc_if_slow)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		lua_pushboolean(L, ch->IsAffectFlag(AFF_SLOW));
		return 1;
	}
	ALUA(pc_if_stun)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		lua_pushboolean(L, ch->IsAffectFlag(AFF_STUN));
		return 1;
	}

	ALUA(pc_sf_fire)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		if (lua_toboolean(L, 1))
			ch->AddAffect(AFFECT_FIRE, 0, 0, AFF_FIRE, 3 * 5 + 1, 0, 1, 0);
		else
			ch->RemoveAffect(AFFECT_FIRE);
		return 0;
	}
	ALUA(pc_sf_invisible)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		if (lua_toboolean(L, 1))
			ch->AddAffect(AFFECT_INVISIBILITY, 0, 0, AFF_INVISIBILITY, 60 * 60 * 24 * 365 * 60 + 1, 0, 1, 0);
		else
			ch->RemoveAffect(AFFECT_INVISIBILITY);
		return 0;
	}
	ALUA(pc_sf_poison)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		if (lua_toboolean(L, 1))
			ch->AddAffect(AFFECT_POISON, 0, 0, AFF_POISON, 30 + 1, 0, 1, 0);
		else
			ch->RemoveAffect(AFFECT_POISON);
		return 0;
	}
#ifdef __WOLFMAN_CHARACTER__
	ALUA(pc_sf_bleeding)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		if (lua_toboolean(L, 1))
			ch->AddAffect(AFFECT_BLEEDING, 0, 0, AFF_BLEEDING, 30 + 1, 0, 1, 0);
		else
			ch->RemoveAffect(AFFECT_BLEEDING);
		return 0;
	}
#endif
	ALUA(pc_sf_slow)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		if (lua_toboolean(L, 1))
			ch->AddAffect(AFFECT_SLOW, 19, -30, AFF_SLOW, 30, 0, 1, 0);
		else
			ch->RemoveAffect(AFFECT_SLOW);
		return 0;
	}
	ALUA(pc_sf_stun)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		if (lua_toboolean(L, 1))
			ch->AddAffect(AFFECT_STUN, 0, 0, AFF_STUN, 30, 0, 1, 0);
		else
			ch->RemoveAffect(AFFECT_STUN);
		return 0;
	}

	ALUA(pc_sf_kill)
	{
		LPCHARACTER ch = CHARACTER_MANAGER::instance().FindPC(lua_tostring(L, 1));
		if (ch)
			ch->Dead(0, 0);
		return 0;
	}

	ALUA(pc_sf_dead)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		if (ch)
			ch->Dead(0, 0);
		return 0;
	}

	ALUA(pc_get_exp_level)
	{
		if (!lua_isnumber(L, 1))
		{
			sys_err("arg1 must be a number");
			return 0;
		}
		lua_pushnumber(L, (DWORD)(exp_table[MINMAX(0, lua_tonumber(L, 1), PLAYER_MAX_LEVEL_CONST)] / 100));
		return 1;
	}

	ALUA(pc_get_exp_level0)
	{
		if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2))
		{
			sys_err("arg1 and arg2 must be numbers");
			return 0;
		}
		lua_pushnumber(L, (DWORD)((exp_table[MINMAX(0, lua_tonumber(L, 1), PLAYER_MAX_LEVEL_CONST)] / 100) * MINMAX(1, lua_tonumber(L, 2), 100)));
		return 1;
	}

	ALUA(pc_set_max_health)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		ch->PointChange(POINT_HP, ch->GetMaxHP() - ch->GetHP());
		ch->PointChange(POINT_SP, ch->GetMaxSP() - ch->GetSP());
		return 0;
	}

	ALUA(pc_get_ip0)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		lua_pushstring(L, ch->GetDesc()->GetHostName());
		return 1;
	}

	ALUA(pc_dc_delayed0)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		if (!ch)
			return 0;
		bool bRet = ch->GetDesc()->DelayedDisconnect(MINMAX(0, lua_tonumber(L, 1), 60 * 5));
		lua_pushboolean(L, bRet);
		return 1;
	}

	ALUA(pc_dc_direct0)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		ch->Disconnect(lua_tostring(L, 1));
		return 0;
	}

	ALUA(pc_is_trade0)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		lua_pushboolean(L, ch->GetExchange() != NULL);
		return 1;
	}

	enum EBusyReturnValue
	{
		RETURN_VALUE_UNKNOWN = 0,
		RETURN_VALUE_OK = 1,
		RETURN_VALUE_OPENED_WINDOW = 2,
		RETURN_VALUE_TIME_WAIT = 3,
	};

	ALUA(pc_is_busy)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("!-- Karakter Allaha emanet --! pc_is_busy");
			lua_pushnumber(L, RETURN_VALUE_UNKNOWN);
			return 1;
		}

		if (ch->GetExchange() || ch->GetMyShop() || ch->GetShopOwner() || ch->IsOpenSafebox() || ch->IsCubeOpen() || ch->IsUnderRefine())
		{
			lua_pushnumber(L, RETURN_VALUE_OPENED_WINDOW);
			return 1;
		}

#ifdef __ACCE_SYSTEM__
		if (ch->isAcceOpened(true))
		{
			lua_pushnumber(L, RETURN_VALUE_OPENED_WINDOW);
			return 1;
		}

		if (ch->isAcceOpened(false))
		{
			lua_pushnumber(L, RETURN_VALUE_OPENED_WINDOW);
			return 1;
		}
#endif

#ifdef __ITEM_CHANGELOOK__
		if (ch->isChangeLookOpened())
		{
			lua_pushnumber(L, RETURN_VALUE_OPENED_WINDOW);
			return 1;
		}
#endif

#ifdef __OFFLINE_SHOP__
		if (ch->GetOfflineShopGuest() != NULL || ch->GetShopSafebox())
		{
			lua_pushnumber(L, RETURN_VALUE_OPENED_WINDOW);
			return 1;
		}
#endif

		for (int x = 0; x < VAR_VAL_MAX_NUM; ++x)
		{
			if (ch->GetVarValue(x) == true)
			{
				lua_pushnumber(L, RETURN_VALUE_OPENED_WINDOW);
				return 1;
			}
		}

		// TIME LIMITS
		const int iPulse = thecore_pulse();
		const int limit_time = PASSES_PER_SEC(g_nPortalLimitTime);

		const int var_limit_time[VAR_TIME_MAX_NUM] = {
			0,
#ifdef __CHANNEL_CHANGER__
			10,
#endif
#ifdef __SKILLBOOK_SYSTEM__
			0,
#endif
#ifdef __SOULSTONE_SYSTEM__
			0,
#endif
#ifdef __GROWTH_PET_SYSTEM__
			5,
#endif
#ifdef __GROWTH_MOUNT_SYSTEM__
			5,
#endif
#ifdef __ITEM_EVOLUTION__
			10,
#endif
#ifdef __INGAME_MALL__
			10,
#endif
#ifdef __REMOVE_SKILL_AFFECT__
			0,
#endif
#ifdef __SPLIT_ITEMS__
			0,
#endif
#ifdef __BIOLOG_SYSTEM__
			0,
#endif
		};

		if ((iPulse - ch->GetSafeboxLoadTime()) < limit_time)
		{
			lua_pushnumber(L, RETURN_VALUE_TIME_WAIT);
			return 1;
		}

		if ((iPulse - ch->GetExchangeTime()) < limit_time)
		{
			lua_pushnumber(L, RETURN_VALUE_TIME_WAIT);
			return 1;
		}

		if ((iPulse - ch->GetMyShopTime()) < limit_time)
		{
			lua_pushnumber(L, RETURN_VALUE_TIME_WAIT);
			return 1;
		}

		if ((iPulse - ch->GetRefineTime()) < limit_time)
		{
			lua_pushnumber(L, RETURN_VALUE_TIME_WAIT);
			return 1;
		}

#ifdef __ACCE_SYSTEM__
		if ((iPulse - ch->GetAcceTime()) < limit_time)
		{
			lua_pushnumber(L, RETURN_VALUE_TIME_WAIT);
			return 1;
		}
#endif

#ifdef __ITEM_CHANGELOOK__
		if ((iPulse - ch->GetChangeLookTime()) < limit_time)
		{
			lua_pushnumber(L, RETURN_VALUE_TIME_WAIT);
			return 1;
		}
#endif

#ifdef __OFFLINE_SHOP__
		if ((iPulse - ch->GetOfflineShopTime()) < limit_time)
		{
			lua_pushnumber(L, RETURN_VALUE_TIME_WAIT);
			return 1;
		}
#endif

		for (int x = 0; x < VAR_TIME_MAX_NUM; ++x)
		{
			if ((iPulse - ch->GetVarTime(x)) < var_limit_time[x])
			{
				lua_pushnumber(L, RETURN_VALUE_TIME_WAIT);
				return 1;
			}
		}

		lua_pushnumber(L, RETURN_VALUE_OK);
		return 1;
	}

	ALUA(pc_equip_slot0)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();

		LPITEM item = ch->GetInventoryItem(lua_tonumber(L, 1));
		lua_pushboolean(L, (item) ? ch->EquipItem(item) : false);
		return 1;
	}

	ALUA(pc_unequip_slot0)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();

		LPITEM item = ch->GetWear(lua_tonumber(L, 1));
		lua_pushboolean(L, (item) ? ch->UnequipItem(item) : false);
		return 1;
	}
	ALUA(pc_is_available0)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();

		lua_pushboolean(L, ch != NULL);
		return 1;
	}

	ALUA(pc_give_random_book0)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		LPITEM item = ch->AutoGiveItem(50300);
		if (item)
		{
			if (lua_isnumber(L, 1))
				item->SetSocket(0, ::GetRandomSkillVnum(lua_tonumber(L, 1)));
			else
				item->SetSocket(0, ::GetRandomSkillVnum());
		}
		lua_pushboolean(L, item != NULL);
		return 1;
	}

	ALUA(pc_is_pvp0)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();

		lua_pushboolean(L, (ch) ? CPVPManager::instance().IsFighting(ch->GetPlayerID()) : false);
		return 1;
	}

	ALUA(pc_is_affect_flag)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		if (!ch)
			lua_pushboolean(L, false);
		unsigned int iAffect = (int)lua_tonumber(L, 1);
		lua_pushboolean(L, ch->IsAffectFlag(iAffect));
		return 1;
	}

#ifdef __SCP1453_EXTENSIONS__
	ALUA(pc_is_unequipped_all) // scp1453
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		if (!ch)
			return 0;
		for (int i = WEAR_BODY; i < WEAR_MAX; i++)
		{
			if (i == WEAR_UNIQUE1 || i == WEAR_UNIQUE2 || i == WEAR_RING1 || i == WEAR_RING2 || i == WEAR_ARROW)
				continue;

			if (ch->GetWear(i))
			{
				lua_pushboolean(L, false);
				return 1;
			}
		}

		lua_pushboolean(L, true);
		return 1;
	}
#endif

#ifdef __ACCE_SYSTEM__
	ALUA(pc_open_acce)
	{
		if (lua_isboolean(L, 1))
		{
			CQuestManager& qMgr = CQuestManager::instance();
			LPCHARACTER pkChar = qMgr.GetCurrentCharacterPtr();
			if (pkChar)
				pkChar->OpenAcce(lua_toboolean(L, 1));
		}
		else
			sys_err("Invalid argument: arg1 must be boolean.");
		return 0;
	}
#endif

	struct EjderKisi
	{
		EjderKisi() {};
		void operator()(LPENTITY ent)
		{
			if (ent->IsType(ENTITY_CHARACTER))
			{
				LPCHARACTER ch = (LPCHARACTER)ent;
				if (ch->IsPC())
				{
					if (!ch->IsPC() || ch->IsObserverMode())
						return;

					ejderde_kac_kisiyiz = ejderde_kac_kisiyiz + 1;
				}
			}
		}
	};

	ALUA(pc_ejder_kontrol)
	{
		LPSECTREE_MAP pSecMap = SECTREE_MANAGER::instance().GetMap(lua_tonumber(L, 1));

		if (NULL != pSecMap)
		{
			EjderKisi f;
			ejderde_kac_kisiyiz = 0;
			pSecMap->for_each(f);
			lua_pushnumber(L, ejderde_kac_kisiyiz);
		}

		return 1;
	}

#ifdef __SKILL_CHOOSE_WINDOW__
	ALUA(pc_open_skillchoose)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		if (!ch || !ch->GetDesc()) return 0;
		if (ch->GetSkillGroup()) return 0;

		TPacketGCSkillChoose packet;
		packet.bHeader = HEADER_GC_SKILLCHOOSE;
		packet.job = ch->GetJob();

		ch->GetDesc()->Packet(&packet, sizeof(TPacketGCSkillChoose));
		lua_pushnumber(L, 1);
		return 1;
	}
#endif

#ifdef __ITEM_CHANGELOOK__
	ALUA(pc_open_changelook)
	{
		CQuestManager& qMgr = CQuestManager::instance();
		LPCHARACTER pkChar = qMgr.GetCurrentCharacterPtr();
		if (pkChar)
		{
#ifdef __MOUNT_CHANGELOOK__
			bool bMount = lua_toboolean(L, 1);
			pkChar->ChangeLookWindow(true, false, bMount);
#else
			pkChar->ChangeLookWindow(true);
#endif
		}

		return 0;
	}
#endif

#ifdef __BRAVERY_CAPE_REWORK__
	ALUA(pc_get_bravery_cape_use_count)
	{
		lua_pushnumber(L, CQuestManager::instance().GetCurrentCharacterPtr()->GetBraveryCapeUseCount());
		return 1;
	}

	ALUA(pc_set_bravery_cape_use_count)
	{
		if (!lua_isnumber(L, 1))
			sys_err("QUEST wrong bravery cape use count number");
		else
		{
			CQuestManager& q = CQuestManager::Instance();
			LPCHARACTER ch = q.GetCurrentCharacterPtr();
			if (ch)
				ch->SetBraveryCapeUseCount(lua_tonumber(L, 1));
		}
		return 0;
	}

	ALUA(pc_get_bravery_cape_use_time)
	{
		lua_pushnumber(L, CQuestManager::instance().GetCurrentCharacterPtr()->GetBraveryCapeUseTime());
		return 1;
	}

	ALUA(pc_set_bravery_cape_use_time)
	{
		if (!lua_isnumber(L, 1))
			sys_err("QUEST wrong bravery cape use count number");
		else
		{
			CQuestManager& q = CQuestManager::Instance();
			LPCHARACTER ch = q.GetCurrentCharacterPtr();
			if (ch)
				ch->SetBraveryCapeUseTime(lua_tonumber(L, 1));
		}
		return 0;
	}
#endif

	ALUA(pc_specific_effect)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		if (!ch) return 0;

		if (!lua_isstring(L, 1))
		{
			sys_err("Argument error.");
			lua_pushboolean(L, false);
			return 1;
		}

		ch->SpecificEffectPacket(lua_tostring(L, 1));
		lua_pushboolean(L, true);
		return 1;
	}

	ALUA(pc_disconnect_with_delay)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		if (!ch) return 0;

		ch->GetDesc()->DelayedDisconnect(lua_isnumber(L, 1) ? (int)lua_tonumber(L, 1) : 10);
		return 0;
	}

	ALUA(pc_get_max_level)
	{
		lua_pushnumber(L, gPlayerMaxLevel);
		return 1;
	}

	ALUA(pc_update_packet)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		if (!ch) return 0;

		ch->UpdatePacket();
		return 0;
	}

	ALUA(pc_view_reencode)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		if (!ch) return 0;

		ch->ViewReencode();
		return 0;
	}

#ifdef __ALIGNMENT_REWORK__
	ALUA(pc_get_alignment_grade)
	{
		lua_pushnumber(L, CQuestManager::instance().GetCurrentCharacterPtr()->GetAlignGrade());
		return 1;
	}
#endif

#ifdef __RANK_SYSTEM__
	ALUA(pc_change_rank)
	{
		if (!lua_isnumber(L, 1))
		{
			sys_err("arg1 must be number");
			return 0;
		}
		CQuestManager& qMgr = CQuestManager::instance();
		LPCHARACTER pkChar = qMgr.GetCurrentCharacterPtr();
		if (pkChar)
			pkChar->ChangeRank(lua_tonumber(L, 1));
		return 0;
	}
	ALUA(pc_get_rank)
	{
		CQuestManager& qMgr = CQuestManager::instance();
		LPCHARACTER pkChar = qMgr.GetCurrentCharacterPtr();
		if (pkChar)
		{
			lua_pushnumber(L, pkChar->GetRank());
			return 1;
		}
		else
		{
			sys_err("pkChar invalid!");
			return 0;
		}
	}
#endif
#ifdef __TEAM_SYSTEM__
	ALUA(pc_change_team)
	{
		if (!lua_isnumber(L, 1))
		{
			sys_err("arg1 must be number");
			return 0;
		}
		CQuestManager& qMgr = CQuestManager::instance();
		LPCHARACTER pkChar = qMgr.GetCurrentCharacterPtr();
		if (pkChar)
			pkChar->ChangeTeam(lua_tonumber(L, 1));
		return 0;
	}
	ALUA(pc_get_team)
	{
		CQuestManager& qMgr = CQuestManager::instance();
		LPCHARACTER pkChar = qMgr.GetCurrentCharacterPtr();
		if (pkChar)
		{
			lua_pushnumber(L, pkChar->GetTeam());
			return 1;
		}
		else
		{
			sys_err("pkChar invalid!");
			return 0;
		}
	}
#endif
#ifdef __LANDRANK_SYSTEM__
	ALUA(pc_change_landrank)
	{
		if (!lua_isnumber(L, 1))
		{
			sys_err("arg1 must be number");
			return 0;
		}
		CQuestManager& qMgr = CQuestManager::instance();
		LPCHARACTER pkChar = qMgr.GetCurrentCharacterPtr();
		if (pkChar)
			pkChar->ChangeLandRank(lua_tonumber(L, 1));
		return 0;
	}
	ALUA(pc_get_landrank)
	{
		CQuestManager& qMgr = CQuestManager::instance();
		LPCHARACTER pkChar = qMgr.GetCurrentCharacterPtr();
		if (pkChar)
		{
			lua_pushnumber(L, pkChar->GetLandRank());
			return 1;
		}
		else
		{
			sys_err("pkChar invalid!");
			return 0;
		}
	}
#endif
#ifdef __REBORN_SYSTEM__
	ALUA(pc_change_reborn)
	{
		if (!lua_isnumber(L, 1))
		{
			sys_err("arg1 must be number");
			return 0;
		}
		CQuestManager& qMgr = CQuestManager::instance();
		LPCHARACTER pkChar = qMgr.GetCurrentCharacterPtr();
		if (pkChar)
			pkChar->ChangeReborn(lua_tonumber(L, 1));
		return 0;
	}
	ALUA(pc_get_reborn)
	{
		CQuestManager& qMgr = CQuestManager::instance();
		LPCHARACTER pkChar = qMgr.GetCurrentCharacterPtr();
		if (pkChar)
		{
			lua_pushnumber(L, pkChar->GetReborn());
			return 1;
		}
		else
		{
			sys_err("pkChar invalid!");
			return 0;
		}
	}
#endif
#ifdef __MONIKER_SYSTEM__
	ALUA(pc_change_moniker)
	{
		if (!lua_isstring(L, 1))
		{
			sys_err("arg1 must be number");
			return 0;
		}
		CQuestManager& qMgr = CQuestManager::instance();
		LPCHARACTER pkChar = qMgr.GetCurrentCharacterPtr();
		const char* moniker = lua_tostring(L, 1);
		if (pkChar)
		{
			for (size_t i = 0; i < strlen(moniker); i++)
			{
				if (!isalnum(moniker[i]))
				{
					lua_pushboolean(L, false);
					return 1;
				}
			}

			pkChar->ChangeMoniker(moniker);
			lua_pushboolean(L, true);
			return 1;
		}

		lua_pushboolean(L, false);
		return 1;
	}
	ALUA(pc_get_moniker)
	{
		CQuestManager& qMgr = CQuestManager::instance();
		LPCHARACTER pkChar = qMgr.GetCurrentCharacterPtr();
		if (pkChar)
		{
			lua_pushstring(L, pkChar->GetMoniker());
			return 1;
		}
		else
		{
			sys_err("pkChar invalid!");
			return 0;
		}
	}
#endif
#ifdef ENABLE_LOVE_SYSTEM
	ALUA(pc_change_love1)
	{
		if (!lua_isstring(L, 1))
		{
			sys_err("arg1 must be number");
			return 0;
		}
		CQuestManager& qMgr = CQuestManager::instance();
		LPCHARACTER pkChar = qMgr.GetCurrentCharacterPtr();
		const char* moniker = lua_tostring(L, 1);
		if (pkChar)
		{
			for (size_t i = 0; i < strlen(moniker); i++)
			{
				if (!isalnum(moniker[i]))
				{
					lua_pushboolean(L, false);
					return 1;
				}
			}

			pkChar->ChangeLove1(moniker);
			lua_pushboolean(L, true);
			return 1;
		}

		lua_pushboolean(L, false);
		return 1;
	}
	ALUA(pc_change_love2)
	{
		if (!lua_isstring(L, 1))
		{
			sys_err("arg1 must be number");
			return 0;
		}
		CQuestManager& qMgr = CQuestManager::instance();
		LPCHARACTER pkChar = qMgr.GetCurrentCharacterPtr();
		const char* moniker = lua_tostring(L, 1);
		if (pkChar)
		{
			for (size_t i = 0; i < strlen(moniker); i++)
			{
				if (!isalnum(moniker[i]))
				{
					lua_pushboolean(L, false);
					return 1;
				}
			}

			pkChar->ChangeLove2(moniker);
			lua_pushboolean(L, true);
			return 1;
		}

		lua_pushboolean(L, false);
		return 1;
	}
	ALUA(pc_get_love1)
	{
		CQuestManager& qMgr = CQuestManager::instance();
		LPCHARACTER pkChar = qMgr.GetCurrentCharacterPtr();
		if (pkChar)
		{
			lua_pushstring(L, pkChar->GetLove1());
			return 1;
		}
		else
		{
			sys_err("pkChar invalid!");
			return 0;
		}
	}
	ALUA(pc_get_love2)
	{
		CQuestManager& qMgr = CQuestManager::instance();
		LPCHARACTER pkChar = qMgr.GetCurrentCharacterPtr();
		if (pkChar)
		{
			lua_pushstring(L, pkChar->GetLove2());
			return 1;
		}
		else
		{
			sys_err("pkChar invalid!");
			return 0;
		}
	}
#endif // ENABLE_LOVE_SYSTEM
#ifdef ENABLE_WORD_SYSTEM
	ALUA(pc_change_word)
	{
		if (!lua_isstring(L, 1))
		{
			sys_err("arg1 must be number");
			return 0;
		}
		CQuestManager& qMgr = CQuestManager::instance();
		LPCHARACTER pkChar = qMgr.GetCurrentCharacterPtr();
		const char* moniker = lua_tostring(L, 1);
		if (pkChar)
		{
			for (size_t i = 0; i < strlen(moniker); i++)
			{
				if (!isalnum(moniker[i]))
				{
					lua_pushboolean(L, false);
					return 1;
				}
			}

			// kontrol
			if (strlen(moniker) > 24)
			{
				lua_pushboolean(L, false);
				return 1;
			}

			pkChar->ChangeWord(moniker);
			lua_pushboolean(L, true);
			return 1;
		}

		lua_pushboolean(L, false);
		return 1;
	}

	ALUA(pc_get_word)
	{
		CQuestManager& qMgr = CQuestManager::instance();
		LPCHARACTER pkChar = qMgr.GetCurrentCharacterPtr();
		if (pkChar)
		{
			lua_pushstring(L, pkChar->GetWord());
			return 1;
		}
		else
		{
			sys_err("pkChar invalid!");
			return 0;
		}
	}
#endif // ENABLE_WORD_SYSTEM

#ifdef __BIOLOG_SYSTEM__
	ALUA(pc_set_grand_biolog)
	{
		CQuestManager& qMgr = CQuestManager::instance();
		LPCHARACTER pkChar = qMgr.GetCurrentCharacterPtr();
		if (pkChar)
			pkChar->SetGrandBiolog((BYTE)lua_tonumber(L, 1));
		return 0;
	}
	ALUA(pc_fix_biolog)
	{
		CQuestManager& qMgr = CQuestManager::instance();
		LPCHARACTER pkChar = qMgr.GetCurrentCharacterPtr();
		PC* pPC = qMgr.GetCurrentPC();

		if (pkChar)
		{
			pkChar->SetGrandBiolog(pPC->GetFlag("bio.level"));
			pkChar->ChatPacket(CHAT_TYPE_INFO, "Biyolog özellikleriniz silinip %d. seviyeye kadar tekrar verilmiþtir.", pPC->GetFlag("bio.level"));
		}

		return 0;
	}
#endif

#ifdef ENABLE_PLAYER_STATISTICS
	ALUA(pc_on_dungeon_end)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();

		if (ch)
			ch->AddToCompletedDungeon();

		return 0;
	}
#endif

#ifdef __SKILL_SET_BONUS__
	ALUA(pc_refresh_skill_set_bonus)
	{
		CQuestManager& qMgr = CQuestManager::instance();
		LPCHARACTER pkChar = qMgr.GetCurrentCharacterPtr();
		if (pkChar)
			pkChar->UpdateSkillSetBonus();
		return 0;
	}
#endif

#ifdef __CHEQUE_SYSTEM__
	ALUA(pc_give_cheque)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();

		if (!lua_isnumber(L, 1))
		{
			sys_err("QUEST : wrong argument");
			return 0;
		}

		int iAmount = (int)lua_tonumber(L, 1);

		if (iAmount <= 0)
		{
			sys_err("QUEST : cheque amount less then zero");
			return 0;
		}

		ch->PointChange(POINT_CHEQUE, iAmount, true);
		return 0;
	}

	ALUA(pc_change_cheque)
	{
		int cheque = (int)lua_tonumber(L, -1);

		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();

		if (cheque + ch->GetCheque() < 0)
			sys_err("QUEST wrong ChangeCheque %d (now %d)", cheque, ch->GetCheque());
		else
			ch->PointChange(POINT_CHEQUE, cheque, true);

		return 0;
	}

	ALUA(pc_get_cheque)
	{
		lua_pushnumber(L, CQuestManager::instance().GetCurrentCharacterPtr()->GetCheque());
		return 1;
	}
#endif

	ALUA(pc_reset_pos)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		ch->Show(ch->GetMapIndex(), ch->GetX(), ch->GetY());
		ch->Stop();
		return 0;
	}

	ALUA(pc_fix_kenvanter)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			lua_pushboolean(L, false);
			return 1;
		}

		char szQuery[1024] = { 0, };
		snprintf(szQuery, sizeof(szQuery),
			"SELECT vnum, count "
			"FROM kenvanter_fix_item WHERE owner_id=%d",
			ch->GetPlayerID());

		std::unique_ptr<SQLMsg> pmsg2(DBManager::instance().DirectQuery(szQuery));

		MYSQL_ROW row = NULL;
		DWORD vnum = 0;
		DWORD count = 0;
		while (NULL != (row = mysql_fetch_row(pmsg2->Get()->pSQLResult)))
		{
			int cur = 0;

			str_to_number(vnum, row[cur++]);
			str_to_number(count, row[cur++]);

			if (vnum && count)
				ch->AutoGiveItem(vnum, count);
		}

		snprintf(szQuery, sizeof(szQuery),
			"DELETE FROM kenvanter_fix_item WHERE owner_id=%d",
			ch->GetPlayerID());
		std::unique_ptr<SQLMsg> pmsg3(DBManager::instance().DirectQuery(szQuery));

		lua_pushboolean(L, true);
		return 1;
	}

#ifdef __DUNGEON_LIMIT__
	ALUA(pc_insert_remain_time)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();

		if (!ch)
			return 0;

		int type = lua_tonumber(L, 1);
		const char* dungeon_name = lua_tostring(L, 2);
		int time = lua_tonumber(L, 3);

		char value[256];
		char value2[256];
		switch (type)
		{
		case 1:
			strcpy(value, ch->GetDesc()->GetCPUID());
			break;
		case 2:
			strcpy(value, ch->GetDesc()->GetHDDModel());
			break;
		case 3:
			strcpy(value, ch->GetDesc()->GetMachineGuid());
			strcpy(value2, ch->GetDesc()->GetCPUID());
			break;
		case 4:
			strcpy(value, ch->GetDesc()->GetMacAddr());
			break;
		case 5:
			strcpy(value, ch->GetDesc()->GetHDDSerial());
			break;
		case 6:
			strcpy(value, ch->GetDesc()->GetBiosID());
			break;

		default:
			return 0;
		}

		std::unique_ptr<SQLMsg> szQuery(DBManager::instance().DirectQuery("INSERT INTO player.dungeon_duration (type, value, value2, dungeon_name, date) VALUES (%d, '%s', '%s', '%s', DATE_ADD(NOW(), INTERVAL %d SECOND))", type, value, value2, dungeon_name, time));

		sys_log(0, "dungeon_set_remain_time %d %s %d", type, value, value);
		return 0;
	}
	ALUA(pc_reset_remain_time)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();

		if (!ch)
			return 0;

		int type = lua_tonumber(L, 1);
		const char* dungeon_name = lua_tostring(L, 2);
		char value[256];
		char value2[256];

		switch (type)
		{
		case 1:
			strcpy(value, ch->GetDesc()->GetCPUID());
			break;
		case 2:
			strcpy(value, ch->GetDesc()->GetHDDModel());
			break;
		case 3:
			strcpy(value, ch->GetDesc()->GetMachineGuid());
			strcpy(value2, ch->GetDesc()->GetCPUID());
			break;
		case 4:
			strcpy(value, ch->GetDesc()->GetMacAddr());
			break;
		case 5:
			strcpy(value, ch->GetDesc()->GetHDDSerial());
			break;
		case 6:
			strcpy(value, ch->GetDesc()->GetBiosID());
			break;

		default:
			return 0;
		}

		std::unique_ptr<SQLMsg> szQuery(DBManager::instance().DirectQuery("DELETE FROM player.dungeon_duration type = %d and value = '%s' and value2 = '%s' and dungeon_name = '%s'", type, value, value2, dungeon_name));

		sys_log(0, "pc_reset_remain_time %d %s %s", type, value, dungeon_name);
		return 0;
	}
	ALUA(pc_get_remain_time)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();

		if (!ch || !ch->GetDesc())
			return 0;

		int type = lua_tonumber(L, 1);
		const char* dungeon_name = lua_tostring(L, 2);

		char value[256];
		char value2[256];
		switch (type)
		{
		case 1:
			strcpy(value, ch->GetDesc()->GetCPUID());
			break;
		case 2:
			strcpy(value, ch->GetDesc()->GetHDDModel());
			break;
		case 3:
			strcpy(value, ch->GetDesc()->GetMachineGuid());
			strcpy(value2, ch->GetDesc()->GetCPUID());
			break;
		case 4:
			strcpy(value, ch->GetDesc()->GetMacAddr());
			break;
		case 5:
			strcpy(value, ch->GetDesc()->GetHDDSerial());
			break;
		case 6:
			strcpy(value, ch->GetDesc()->GetBiosID());
			break;

		default:
			return 0;
		}

		char szQuery[1024];
		snprintf(szQuery, sizeof(szQuery), "SELECT UNIX_TIMESTAMP(date) FROM player.dungeon_duration WHERE type = %d and value = '%s' and value2 = '%s' and dungeon_name = '%s' and date > NOW() ORDER BY DATE", type, value, value2, dungeon_name);
		std::unique_ptr<SQLMsg> pmsg(DBManager::instance().DirectQuery(szQuery));

		lua_pushnumber(L, pmsg->Get()->uiNumRows);

		if (pmsg->Get()->uiNumRows > 0)
		{
			MYSQL_ROW row = mysql_fetch_row(pmsg->Get()->pSQLResult);
			int date = 0;
			str_to_number(date, row[0]);

			lua_pushnumber(L, date);
		}
		else
		{
			lua_pushnumber(L, 0);
		}

		return 2;
	}

#endif // __DUNGEON_LIMIT__

#ifdef __BATTLE_PASS_SYSTEM__
	int pc_update_dungeon_progress_low(lua_State* L)
	{
		if (!lua_isnumber(L, 1))
		{
			sys_err("arg1 must be number");
			return 0;
		}

		BYTE bDungeonType = (int)lua_tonumber(L, 1);
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();

		if (ch)
		{
			BYTE bBattlePassId = ch->GetBattlePassId();
			if (bBattlePassId)
			{
				DWORD dwDungeonID, dwCount;
				if (CBattlePass::instance().BattlePassMissionGetInfo(bBattlePassId, COMPLETE_LOW_DUNGEON, &dwDungeonID, &dwCount))
				{
					if (ch->GetMissionProgress(COMPLETE_LOW_DUNGEON, bBattlePassId) < dwCount)
						ch->UpdateMissionProgress(COMPLETE_LOW_DUNGEON, bBattlePassId, 1, dwCount);
				}

#ifdef ENABLE_BATTLE_PASS_EX
				BYTE nextMissionId = ch->GetNextMissionByType(DUNGEON_DONE1, DUNGEON_DONE30, bBattlePassId);
				if (nextMissionId != 0)
				{
					DWORD dwDungeonID, dwCount;
					if (CBattlePass::instance().BattlePassMissionGetInfo(bBattlePassId, nextMissionId, &dwDungeonID, &dwCount))
					{
						if (ch->GetMissionProgress(nextMissionId, bBattlePassId) < dwCount)
							ch->UpdateMissionProgress(nextMissionId, bBattlePassId, 1, dwCount);
					}
				}
#endif // ENABLE_BATTLE_PASS_EX
			}
		}

		return 0;
	}

	int pc_update_dungeon_progress_high(lua_State* L)
	{
		if (!lua_isnumber(L, 1))
		{
			sys_err("arg1 must be number");
			return 0;
		}

		BYTE bDungeonType = (int)lua_tonumber(L, 1);
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();

		if (ch)
		{
			BYTE bBattlePassId = ch->GetBattlePassId();
			if (bBattlePassId)
			{
				DWORD dwDungeonID, dwCount;
				if (CBattlePass::instance().BattlePassMissionGetInfo(bBattlePassId, COMPLETE_HIGH_DUNGEON, &dwDungeonID, &dwCount))
				{
					if (ch->GetMissionProgress(COMPLETE_HIGH_DUNGEON, bBattlePassId) < dwCount)
						ch->UpdateMissionProgress(COMPLETE_HIGH_DUNGEON, bBattlePassId, 1, dwCount);
				}

#ifdef ENABLE_BATTLE_PASS_EX
				BYTE nextMissionId = ch->GetNextMissionByType(DUNGEON_DONE1, DUNGEON_DONE30, bBattlePassId);
				if (nextMissionId != 0)
				{
					DWORD dwDungeonID, dwCount;
					if (CBattlePass::instance().BattlePassMissionGetInfo(bBattlePassId, nextMissionId, &dwDungeonID, &dwCount))
					{
						if (ch->GetMissionProgress(nextMissionId, bBattlePassId) < dwCount)
							ch->UpdateMissionProgress(nextMissionId, bBattlePassId, 1, dwCount);
					}
				}
#endif // ENABLE_BATTLE_PASS_EX
			}
		}

		return 0;
	}

	int pc_open_battle_pass_ranking(lua_State* L)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		if (ch)
		{
			DWORD dwPlayerId = ch->GetPlayerID();
			BYTE bIsGlobal = 1;

			db_clientdesc->DBPacketHeader(HEADER_GD_BATTLE_PASS_RANKING, ch->GetDesc()->GetHandle(), sizeof(DWORD) + sizeof(BYTE));
			db_clientdesc->Packet(&dwPlayerId, sizeof(DWORD));
			db_clientdesc->Packet(&bIsGlobal, sizeof(BYTE));
		}

		return 0;
	}
#endif

#ifdef ENABLE_NAME_CHANGE_SYSTEM
	ALUA(pc_change_name_premium)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();

		if (ch->GetNewName().size() != 0)
		{
			lua_pushnumber(L, 0);
			return 1;
		}

		if (lua_isstring(L, 1) != true)
		{
			lua_pushnumber(L, 1);
			return 1;
		}

		const char* szName = lua_tostring(L, 1);

		if (strchr(szName, '"') || strchr(szName, '[') || strchr(szName, ']') || strchr(szName, '{') || strchr(szName, ' ') || strchr(szName, ';') || strchr(szName, '}') || strchr(szName, '=') || strchr(szName, '^') || strchr(szName, '%'))
		{
			lua_pushnumber(L, 2);
			return 1;
		}

		char szEscapedName[CHARACTER_NAME_MAX_LEN + 1];
		DBManager::instance().EscapeString(szEscapedName, sizeof(szEscapedName), szName, strlen(szName));

		char szQuery[1024];
		snprintf(szQuery, sizeof(szQuery), "SELECT COUNT(*) FROM player%s WHERE name='%s';", get_table_postfix(), szEscapedName);
		std::unique_ptr<SQLMsg> pmsg(DBManager::instance().DirectQuery(szQuery));

		if (pmsg->Get()->uiNumRows > 0)
		{
			MYSQL_ROW row = mysql_fetch_row(pmsg->Get()->pSQLResult);

			int	count = 0;
			str_to_number(count, row[0]);

			if (count != 0)
			{
				lua_pushnumber(L, 3);
				return 1;
			}
		}

		// affecti siliyoruz var ise
		if (ch->FindAffect(AFFECT_PB2_CHAR_NAME))
			ch->RemoveAffect(AFFECT_PB2_CHAR_NAME);

		ch->AddAffect(AFFECT_PB2_CHAR_NAME, POINT_NORMAL_HIT_DAMAGE_BONUS, 20, AFF_NONE, INFINITE_AFFECT_DURATION, 0, true, true);

		DWORD pid = ch->GetPlayerID();
		ch->Save();
		ch->SetExchangeTime();
		ch->GetDesc()->DelayedDisconnect(3);

		/* delete messenger list */
		MessengerManager::instance().RemoveAllList(ch->GetName());
		/* change_name_log */
		LogManager::instance().ChangeNameLog(pid, ch->GetName(), szEscapedName, ch->GetDesc()->GetHostName());

		snprintf(szQuery, sizeof(szQuery), "UPDATE player%s SET name='%s' WHERE id=%u;", get_table_postfix(), szEscapedName, pid);
		SQLMsg* msg = DBManager::instance().DirectQuery(szQuery);
		M2_DELETE(msg);

		ch->SetNewName(szEscapedName);
		lua_pushnumber(L, 4);
		return 1;
	}
#endif // ENABLE_NAME_CHANGE_SYSTEM


	void RegisterPCFunctionTable()
	{
		luaL_reg pc_functions[] =
		{
			{ "get_wear",		pc_get_wear			},
			{ "get_player_id",	pc_get_player_id	},
			{ "get_account_id", pc_get_account_id	},
			{ "get_account",	pc_get_account		},
			{ "get_level",		pc_get_level		},
			{ "set_level",		pc_set_level		},
			{ "get_next_exp",		pc_get_next_exp		},
			{ "get_exp",		pc_get_exp		},
			{ "get_job",		pc_get_job		},
			{ "get_race",		pc_get_race		},
			{ "change_sex",		pc_change_sex	},
			{ "gethp",			pc_get_hp		},
			{ "get_hp",			pc_get_hp		},
			{ "getmaxhp",		pc_get_max_hp		},
			{ "get_max_hp",		pc_get_max_hp		},
			{ "getsp",			pc_get_sp		},
			{ "get_sp",			pc_get_sp		},
			{ "getmaxsp",		pc_get_max_sp		},
			{ "get_max_sp",		pc_get_max_sp		},
			{ "change_sp",		pc_change_sp		},
			{ "getmoney",		pc_get_money		},
			{ "get_money",		pc_get_money		},
			{ "get_real_alignment",	pc_get_real_alignment	},
			{ "get_alignment",		pc_get_alignment	},
			{ "getweapon",		pc_get_weapon		},
			{ "get_weapon",		pc_get_weapon		},
			{ "getarmor",		pc_get_armor		},
			{ "get_armor",		pc_get_armor		},
			{ "getgold",		pc_get_money		},
			{ "get_gold",		pc_get_money		},
			{ "changegold",		pc_change_money		},
			{ "changemoney",		pc_change_money		},
			{ "changealignment",	pc_change_alignment	},
			{ "change_gold",		pc_change_money		},
			{ "change_money",		pc_change_money		},
			{ "change_alignment",	pc_change_alignment	},
			{ "getname",		pc_get_name		},
			{ "get_name",		pc_get_name		},
			{ "get_vid",		pc_get_vid		},
			{ "getplaytime",		pc_get_playtime		},
			{ "get_playtime",		pc_get_playtime		},
			{ "getleadership",		pc_get_leadership	},
			{ "get_leadership",		pc_get_leadership	},
			{ "getqf",			pc_get_quest_flag	},
			{ "setqf",			pc_set_quest_flag	},
			{ "delqf",			pc_del_quest_flag	},
			{ "getf",			pc_get_another_quest_flag},
			{ "setf",			pc_set_another_quest_flag},
			{ "get_x",			pc_get_x		},
			{ "get_y",			pc_get_y		},
			{ "getx",			pc_get_x		},
			{ "gety",			pc_get_y		},
			{ "get_local_x",		pc_get_local_x		},
			{ "get_local_y",		pc_get_local_y		},
			{ "getcurrentmapindex",	pc_get_current_map_index},
			{ "get_map_index",		pc_get_current_map_index},
			{ "give_exp",		pc_give_exp		},
			{ "give_exp_perc",		pc_give_exp_perc	},
			{ "give_exp2",		pc_give_exp2		},
			// { "give_item",		pc_give_item		},
			{ "give_item2",		pc_give_or_drop_item	},
#ifdef __DICE_SYSTEM__
			{ "give_item2_with_dice",	pc_give_or_drop_item_with_dice	},
#endif
			{ "give_item2_select",		pc_give_or_drop_item_and_select	},
			{ "give_gold",		pc_give_gold		},
			{ "count_item",		pc_count_item		},
			{ "remove_item",		pc_remove_item		},
			{ "countitem",		pc_count_item		},
			{ "removeitem",		pc_remove_item		},
			{ "reset_point",		pc_reset_point		},
			{ "has_guild",		pc_hasguild		},
			{ "hasguild",		pc_hasguild		},
			{ "get_guild",		pc_getguild		},
			{ "getguild",		pc_getguild		},
			{ "isguildmaster",		pc_isguildmaster	},
			{ "is_guild_master",	pc_isguildmaster	},
			{ "destroy_guild",		pc_destroy_guild	},
			{ "remove_from_guild",	pc_remove_from_guild	},
			{ "in_dungeon",		pc_in_dungeon		},
			{ "getempire",		pc_get_empire		},
			{ "get_empire",		pc_get_empire		},
			{ "get_skill_group",	pc_get_skillgroup	},
			{ "set_skill_group",	pc_set_skillgroup	},
			{ "warp",			pc_warp			},
			{ "warp_local",		pc_warp_local		},
			{ "warp_exit",		pc_warp_exit		},
			{ "set_warp_location",	pc_set_warp_location	},
			{ "set_warp_location_local",pc_set_warp_location_local },
			{ "get_start_location",	pc_get_start_location	},
			{ "has_master_skill",	pc_has_master_skill	},
			{ "set_part",		pc_set_part		},
			{ "get_part",		pc_get_part		},
			{ "is_mount",		pc_is_mount		},
			{ "mount",			pc_mount		},
			{ "mount_bonus",	pc_mount_bonus	},
			{ "unmount",		pc_unmount		},
			{ "warp_to_guild_war_observer_position", pc_warp_to_guild_war_observer_position	},
			{ "learn_grand_master_skill", pc_learn_grand_master_skill	},
#ifdef __SAGE_SKILL__
			{"learn_sage_master_skill", pc_learn_sage_master_skill},
#endif
			{ "is_skill_book_no_delay",	pc_is_skill_book_no_delay},
			{ "remove_skill_book_no_delay",	pc_remove_skill_book_no_delay},

			{ "enough_inventory",	pc_enough_inventory	},
			{ "get_horse_level",	pc_get_horse_level	}, // TO BE DELETED XXX
			{ "is_horse_alive",		pc_is_horse_alive	}, // TO BE DELETED XXX
			{ "revive_horse",		pc_revive_horse		}, // TO BE DELETED XXX
			{ "have_pos_scroll",	pc_have_pos_scroll	},
			{ "have_map_scroll",	pc_have_map_scroll	},
			{ "get_war_map",		pc_get_war_map		},
			{ "get_equip_refine_level",	pc_get_equip_refine_level },
			{ "refine_equip",		pc_refine_equip		},
			{ "get_skill_level",	pc_get_skill_level	},
			{ "aggregate_monster",	pc_aggregate_monster	},
#ifdef __STONE_DETECT_REWORK__
			{ "aggregate_stone",	pc_aggregate_stone	},
#endif
			{ "is_moving",	pc_is_moving	},
			{ "forget_my_attacker",	pc_forget_my_attacker	},
			{ "pc_attract_ranger",	pc_attract_ranger	},
			{ "select",			pc_select_vid		},
			{ "get_sex",		pc_get_sex		},
			{ "is_gm",			pc_is_gm		},
			{ "get_gm_level",		pc_get_gm_level		},
			{ "mining",			pc_mining		},
			{ "ore_refine",		pc_ore_refine		},
			{ "diamond_refine",		pc_diamond_refine	},

			// RESET_ONE_SKILL
			{ "clear_one_skill",        pc_clear_one_skill      },
			// END_RESET_ONE_SKILL

			{ "clear_skill",                pc_clear_skill          },
			{ "clear_sub_skill",    pc_clear_sub_skill      },
			{ "set_skill_point",    pc_set_skill_point      },

			{ "is_clear_skill_group",	pc_is_clear_skill_group		},

			{ "save_exit_location",		pc_save_exit_location		},
			{ "teleport",				pc_teleport },

			{ "set_skill_level",        pc_set_skill_level      },

			{ "get_premium_remain_sec", pc_get_premium_remain_sec },

			{ "send_block_mode",		pc_send_block_mode	},

			{ "change_empire",			pc_change_empire	},
			{ "get_change_empire_count",	pc_get_change_empire_count	},
			{ "set_change_empire_count",	pc_set_change_empire_count	},

			{ "change_name",			pc_change_name },

			{ "is_dead",				pc_is_dead	},

			{ "reset_status",		pc_reset_status	},
			{ "get_ht",				pc_get_ht	},
			{ "set_ht",				pc_set_ht	},
			{ "get_iq",				pc_get_iq	},
			{ "set_iq",				pc_set_iq	},
			{ "get_st",				pc_get_st	},
			{ "set_st",				pc_set_st	},
			{ "get_dx",				pc_get_dx	},
			{ "set_dx",				pc_set_dx	},

			{ "is_near_vid",		pc_is_near_vid	},

			{ "get_socket_items",	pc_get_socket_items	},
			{ "get_empty_inventory_count",	pc_get_empty_inventory_count	},

			{ "get_logoff_interval",	pc_get_logoff_interval	},

			{ "is_riding",			pc_is_riding	},
			{ "get_special_ride_vnum",	pc_get_special_ride_vnum	},

			{ "can_warp",			pc_can_warp		},

			{ "dec_skill_point",	pc_dec_skill_point	},
			{ "get_skill_point",	pc_get_skill_point	},

			{ "get_channel_id",		pc_get_channel_id	},

			{ "get_sig_items",		pc_get_sig_items	},

			{ "charge_cash",		pc_charge_cash		},

			{ "get_killee_drop_pct",	pc_get_killee_drop_pct	},

			//pc.set_race0(race=[0. Warrior, 1. Ninja, 2. Sura, 3. Shaman, 4. Lycan])
			{ "set_race0",			pc_set_race0			},
			//if pc.delf("game_option", "block_cocks") then syschat("now you are unsafe") end
			{ "delf",				pc_del_another_quest_flag},	// delete quest flag [return lua boolean: successfulness]
			//pc.make_item0({vnum|locale_name, count}, {socket1,2,3}, {type1, value1, ... , type7, value7}, gstate(=0: giveitem, 1: dropitem, 2: drop_item_with_leadership)[, countdown_in_secs_before_ownership_vanish(=if <=10 would be 30; default=10->30)])
			{ "make_item0",			pc_make_item0		},	// [return lua boolean: successfulness]
			//pc.pointchange(uint type, int amount, bool bAmount, bool bBroadcast)
			{ "pointchange",		pc_pointchange		},	// [return nothing]
			//pc.pullmob()
			{ "pullmob",			pc_pullmob			},	// [return nothing]
			//pc.select_pid(pc.get_player_id())
			{ "select_pid",			pc_select_pid		},	// [return lua number: old pid]
			//pc.select renamed in pc.select_vid
			{ "select_vid",			pc_select_vid		},	// [return lua number: old vid]
			//pc.set_level0(level)
			{ "set_level0",			pc_set_level0		},	// [return nothing]
			//pc.set_gm_level(); instead of `/reload a` (note: this only refresh gm privileges if you already are gm on common.gm[host|list])
			{ "set_gm_level",		pc_set_gm_level		},	// [return nothing]
			//is_flags (void) [return lua boolean]
			{ "is_flag_fire",			pc_if_fire			},
			{ "is_flag_invisible",		pc_if_invisible		},
			{ "is_flag_poison",			pc_if_poison		},
#ifdef __WOLFMAN_CHARACTER__
			{ "is_flag_bleeding",		pc_if_bleeding		},
#endif
			{ "is_flag_slow",			pc_if_slow			},
			{ "is_flag_stun",			pc_if_stun			},
			//set_flags (bool) [return nothing]
			{ "set_flag_fire",			pc_sf_fire			},
			{ "set_flag_invisible",		pc_sf_invisible		},
			{ "set_flag_poison",		pc_sf_poison		},
#ifdef __WOLFMAN_CHARACTER__
			{ "set_flag_bleeding",		pc_sf_bleeding		},
#endif
			{ "set_flag_slow",			pc_sf_slow			},
			{ "set_flag_stun",			pc_sf_stun			},
			//pc.set_flag_kill(char_name) [return nothing]
			{ "set_flag_kill",			pc_sf_kill			},
			//pc.set_flag_dead()
			{ "set_flag_dead",			pc_sf_dead			},	// kill themselves [return nothing]
			//pc.get_exp_level(level) [return: lua number]
			{ "get_exp_level",		pc_get_exp_level	},	// get needed exp for <level> level [return: lua number]
			//pc.get_exp_level(level, perc)
			{ "get_exp_level0",		pc_get_exp_level0	},	// get needed exp for <level> level / 100 * perc [return: lua number]
			//pc.set_max_health()
			{ "set_max_health",		pc_set_max_health	},	// [return nothing]
			{ "get_ip0",			pc_get_ip0			},	// [return lua string]
			{ "dc_delayed0",		pc_dc_delayed0	},	// crash a player after x secs [return lua boolean: successfulness]
			{ "dc_direct0",			pc_dc_direct0	},	// crash the <nick> player [return nothing]
			{ "is_trade0",			pc_is_trade0	},	// get if player is trading [return lua boolean]
			{ "is_busy0",			pc_is_busy		},	// get if player is "busy" (if trade, safebox, npc/myshop, cube are open) [return lua boolean]
			{ "is_busy",			pc_is_busy		},
			{ "is_hack", 			pc_is_busy 		},
			// pc.equip_slot0(cell)
			{ "equip_slot0",		pc_equip_slot0		},	// [return lua boolean: successfulness]
			// pc.unequip_slot0(cell)
			{ "unequip_slot0",		pc_unequip_slot0	},	// [return lua boolean: successfulness]
			{ "is_available0",		pc_is_available0	},	// [return lua boolean]
			{ "give_random_book0",	pc_give_random_book0},	// [return lua boolean]
			{ "is_pvp0",			pc_is_pvp0			},	// [return lua boolean]
			{ "show_effect", 		pc_specific_effect	},
			{ "is_affect_flag",		pc_is_affect_flag	},
			{ "update_packet",		pc_update_packet	},
			{ "view_reencode",		pc_view_reencode	},
#ifdef __SCP1453_EXTENSIONS__
			{ "is_unequipped_all",	pc_is_unequipped_all},	// [return lua boolean]
#endif
#ifdef __ACCE_SYSTEM__
			{"open_acce", pc_open_acce},
			{"open_sash", pc_open_acce},
#endif
#ifdef __SKILL_CHOOSE_WINDOW__
			{ "open_skillchoose",			pc_open_skillchoose },
#endif
#ifdef __ITEM_CHANGELOOK__
			{"open_changelook", pc_open_changelook},
#endif
#ifdef __BRAVERY_CAPE_REWORK__
			{"get_bravery_cape_use_count", pc_get_bravery_cape_use_count},
			{"set_bravery_cape_use_count", pc_set_bravery_cape_use_count},
			{"get_bravery_cape_use_time", pc_get_bravery_cape_use_time},
			{"set_bravery_cape_use_time", pc_set_bravery_cape_use_time},
#endif
			{ "ejder_kontrol", pc_ejder_kontrol },
			{ "disconnect_with_delay", pc_disconnect_with_delay },
			{ "get_max_level", pc_get_max_level },
#ifdef __ALIGNMENT_REWORK__
			{ "get_alignment_grade",		pc_get_alignment_grade	},
#endif
#ifdef __RANK_SYSTEM__
			{"get_rank", pc_get_rank},
			{"change_rank", pc_change_rank},
#endif
#ifdef __TEAM_SYSTEM__
			{"get_team", pc_get_team},
			{"change_team", pc_change_team},
#endif
#ifdef __LANDRANK_SYSTEM__
			{"get_landrank", pc_get_landrank},
			{"change_landrank", pc_change_landrank},
#endif
#ifdef __REBORN_SYSTEM__
			{"get_reborn", pc_get_reborn},
			{"change_reborn", pc_change_reborn},
#endif
#ifdef __MONIKER_SYSTEM__
			{"get_moniker", pc_get_moniker},
			{"change_moniker", pc_change_moniker},
#endif
#ifdef ENABLE_LOVE_SYSTEM
			{"get_love1", pc_get_love1},
			{"get_love2", pc_get_love2},
			{"change_love1", pc_change_love1},
			{"change_love2", pc_change_love2},
#endif // ENABLE_LOVE_SYSTEM
#ifdef ENABLE_WORD_SYSTEM
			{"get_word", pc_get_word},
			{"change_word", pc_change_word},
#endif // ENABLE_WORD_SYSTEM
#ifdef __BIOLOG_SYSTEM__
			{"set_grand_biolog", pc_set_grand_biolog},
			{"fix_biolog", pc_fix_biolog},
#endif
#ifdef __SKILL_SET_BONUS__
			{"refresh_skill_set_bonus", pc_refresh_skill_set_bonus},
#endif

#ifdef ENABLE_PLAYER_STATISTICS
			{"on_dungeon_end",		pc_on_dungeon_end},
#endif

#ifdef __CHEQUE_SYSTEM__
			{"give_cheque", pc_give_cheque},
			{"get_cheque", pc_get_cheque},
			{"change_cheque", pc_change_cheque},
			{"getcheque", pc_get_cheque},
			{"changecheque", pc_change_cheque},
			{"get_won", pc_get_cheque},
			{"change_won", pc_change_cheque},
#endif
			{"fix_kenvanter", pc_fix_kenvanter},
			{"reset_pos", pc_reset_pos},
#ifdef __DUNGEON_LIMIT__
			{"insert_remain_time", pc_insert_remain_time},
			{"reset_remain_time", pc_reset_remain_time},
			{"get_remain_time", pc_get_remain_time},
#endif // __DUNGEON_LIMIT__
#ifdef __BATTLE_PASS_SYSTEM__
			{ "update_dungeon_progress_low", pc_update_dungeon_progress_low },
			{ "update_dungeon_progress_high", pc_update_dungeon_progress_high },
			{ "open_battle_pass_ranking", pc_open_battle_pass_ranking },
#endif
#ifdef ENABLE_NAME_CHANGE_SYSTEM
			{ "change_name_premium",	pc_change_name_premium },
#endif // ENABLE_NAME_CHANGE_SYSTEM
			{ NULL,			NULL			}
		};

		CQuestManager::instance().AddLuaFunctionTable("pc", pc_functions);
	}
};