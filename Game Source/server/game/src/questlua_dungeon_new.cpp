#include "stdafx.h"
#include "constants.h"
#include "questmanager.h"
#include "questlua.h"
#ifdef __SCP1453_EXTENSIONS__
#include "dungeon_new.h"
#else
#include "dungeon.h"
#endif
#include "char.h"
#include "buffer_manager.h"
#include "char_manager.h"
#include "packet.h"
#include "desc_client.h"
#include "desc_manager.h"
#include "guild.h"
#include "utils.h"
#include "config.h"
#include "guild_manager.h"
#include "../../common/stl.h"
#include "db.h"
#include "affect.h"
#include "p2p.h"
#include "war_map.h"
#include "sectree_manager.h"
#include "locale_service.h"
#include "dungeon_info.h"
#include "item_manager.h"
#include "questevent.h"
#include "item.h"
#include "party.h"

#undef sys_err
#ifndef __WIN32__
#define sys_err(fmt, args...) quest::CQuestManager::instance().QuestError(__FUNCTION__, __LINE__, fmt, ##args)
#else
#define sys_err(fmt, ...) quest::CQuestManager::instance().QuestError(__FUNCTION__, __LINE__, fmt, __VA_ARGS__)
#endif

namespace quest
{
	//
	// "dungeon" lua functions
	//
	ALUA(dungeon_notice)
	{
		if (!lua_isstring(L, 1))
			return 0;

		CQuestManager& q = CQuestManager::instance();
		LPDUNGEON pDungeon = q.GetCurrentDungeon();

		if (pDungeon)
			pDungeon->Notice(lua_tostring(L, 1));
		return 0;
	}

	ALUA(dungeon_set_flag)
	{
		if (!lua_isstring(L, 1) || !lua_isnumber(L, 2))
		{
			sys_err("wrong set flag");
		}
		else
		{
			CQuestManager& q = CQuestManager::instance();
			LPDUNGEON pDungeon = q.GetCurrentDungeon();

			if (pDungeon)
			{
				const char* sz = lua_tostring(L, 1);
				int value = int(lua_tonumber(L, 2));
				pDungeon->SetFlag(sz, value);
			}
			else
			{
				sys_err("no dungeon !!!");
			}
		}
		return 0;
	}

	ALUA(dungeon_get_flag)
	{
		if (!lua_isstring(L, 1))
		{
			sys_err("wrong get flag");
		}

		CQuestManager& q = CQuestManager::instance();
		LPDUNGEON pDungeon = q.GetCurrentDungeon();

		if (pDungeon)
		{
			const char* sz = lua_tostring(L, 1);
			lua_pushnumber(L, pDungeon->GetFlag(sz));
		}
		else
		{
			sys_err("no dungeon !!!");
			lua_pushnumber(L, 0);
		}

		return 1;
	}

	ALUA(dungeon_get_flag_from_map_index)
	{
		if (!lua_isstring(L, 1) || !lua_isnumber(L, 2))
		{
			sys_err("wrong get flag");
		}

		DWORD dwMapIndex = (DWORD)lua_tonumber(L, 2);
		if (dwMapIndex)
		{
			LPDUNGEON pDungeon = CDungeonManager::instance().FindByMapIndex(dwMapIndex);
			if (pDungeon)
			{
				const char* sz = lua_tostring(L, 1);
				lua_pushnumber(L, pDungeon->GetFlag(sz));
			}
			else
			{
				// sys_err("no dungeon !!!");
				lua_pushnumber(L, 0);
			}
		}
		else
		{
			lua_pushboolean(L, 0);
		}
		return 1;
	}

	ALUA(dungeon_get_map_index)
	{
		CQuestManager& q = CQuestManager::instance();
		LPDUNGEON pDungeon = q.GetCurrentDungeon();

		if (pDungeon)
		{
			sys_log(0, "Dungeon GetMapIndex %d", pDungeon->GetMapIndex());
			lua_pushnumber(L, pDungeon->GetMapIndex());
		}
		else
		{
			sys_err("no dungeon !!!");
			lua_pushnumber(L, 0);
		}

		return 1;
	}

	ALUA(dungeon_regen_file)
	{
		if (!lua_isstring(L, 1))
		{
			sys_err("wrong filename");
			return 0;
		}

		CQuestManager& q = CQuestManager::instance();
		LPDUNGEON pDungeon = q.GetCurrentDungeon();

		if (pDungeon)
			pDungeon->SpawnRegen(lua_tostring(L, 1));

		return 0;
	}

	ALUA(dungeon_set_regen_file)
	{
		if (!lua_isstring(L, 1))
		{
			sys_err("wrong filename");
			return 0;
		}
		CQuestManager& q = CQuestManager::instance();
		LPDUNGEON pDungeon = q.GetCurrentDungeon();

		if (pDungeon)
			pDungeon->SpawnRegen(lua_tostring(L, 1), false);
		return 0;
	}

	ALUA(dungeon_clear_regen)
	{
		CQuestManager& q = CQuestManager::instance();
		LPDUNGEON pDungeon = q.GetCurrentDungeon();
		if (pDungeon)
			pDungeon->ClearRegen();
		return 0;
	}

	struct FRegisterToDungeon
	{
		FRegisterToDungeon(LPDUNGEON d)
			: m_pkDungeon(d)
			{
			}

		void operator () (LPCHARACTER ch)
		{
			m_pkDungeon->RegisterUser(ch);
		}

		LPDUNGEON m_pkDungeon;
	};

	ALUA(dungeon_new_jump_party)
	{
		if (lua_gettop(L) < 1)
		{
			sys_err("not enough argument");
			lua_pushboolean(L, false);
			return 1;
		}
		if (!lua_isnumber(L, 1))
		{
			sys_err("wrong argument");
			lua_pushboolean(L, false);
			return 1;
		}

		long lMapIndex = (long)lua_tonumber(L, 1);

		LPDUNGEON pDungeon = CDungeonManager::instance().Create(lMapIndex);
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		LPSECTREE_MAP pSectree = SECTREE_MANAGER::instance().GetMap(lMapIndex);
		LPPARTY pParty = ch->GetParty();

		if (!pDungeon)
		{
			sys_err("cannot create dungeon %d", lMapIndex);
			lua_pushnumber(L, -1);
			return 1;
		}
		if (!ch)
		{
			sys_err("no ch!");
			lua_pushnumber(L, -2);
			return 1;
		}
		if (!pSectree)
		{
			sys_err("no pSectree!");
			lua_pushnumber(L, -3);
			return 1;
		}
		// if (!pParty)
		// {
		// 	sys_err("no party!");
		// 	lua_pushnumber(L, -4);
		// 	return 1;
		// }

		if (pParty)
		{
			FRegisterToDungeon f(pDungeon);
			pParty->ForEachOnlineMember(f);
		}
		else
		{
			pDungeon->RegisterUser(ch);
		}

		ch->SaveExitLocation();
		auto m_x = pSectree->m_setting.posSpawn.x;
		auto m_y = pSectree->m_setting.posSpawn.y;
		pDungeon->SetJoinCoordinate(m_x, m_y);
		lua_pushboolean(L, ch->WarpSet(m_x, m_y, pDungeon->GetMapIndex()));
		return 1;
	}

	ALUA(dungeon_new_jump)
	{
		if (lua_gettop(L) < 1)
		{
			sys_err("not enough argument");
			lua_pushboolean(L, false);
			return 1;
		}
		if (!lua_isnumber(L, 1))
		{
			sys_err("wrong argument");
			lua_pushboolean(L, false);
			return 1;
		}

		long lMapIndex = (long)lua_tonumber(L, 1);

		LPDUNGEON pDungeon = CDungeonManager::instance().Create(lMapIndex);
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		LPSECTREE_MAP pSectree = SECTREE_MANAGER::instance().GetMap(lMapIndex);

		if (!pDungeon)
		{
			sys_err("cannot create dungeon %d", lMapIndex);
			lua_pushboolean(L, false);
			return 1;
		}
		if (!ch)
		{
			sys_err("no ch!");
			lua_pushboolean(L, false);
			return 1;
		}
		if (!pSectree)
		{
			sys_err("no pSectree!");
			lua_pushboolean(L, false);
			return 1;
		}
		pDungeon->RegisterUser(ch);

		ch->SaveExitLocation();
		auto m_x = pSectree->m_setting.posSpawn.x;
		auto m_y = pSectree->m_setting.posSpawn.y;
		pDungeon->SetJoinCoordinate(m_x, m_y);
		lua_pushboolean(L, ch->WarpSet(m_x, m_y, pDungeon->GetMapIndex()));
		return 1;
	}

	ALUA(dungeon_warp_all)
	{
		if (lua_gettop(L) < 2 || !lua_isnumber(L, 1) || !lua_isnumber(L, 2))
			return 0;

		LPDUNGEON pDungeon = CQuestManager::instance().GetCurrentDungeon();

		if (!pDungeon)
			return 0;

		pDungeon->WarpAll(pDungeon->GetMapIndex(), (int)lua_tonumber(L, 1), (int)lua_tonumber(L, 2));
		return 0;
	}

	ALUA(dungeon_is_use_potion)
	{
		if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2))
		{
			lua_pushboolean(L, 1);
			return 1;
		}

		CQuestManager& q = CQuestManager::instance();
		LPDUNGEON pDungeon = q.GetCurrentDungeon();

		if (pDungeon)
		{
			lua_pushboolean(L, pDungeon->IsUsePotion());
			return 1;
		}

		lua_pushboolean(L, 1);
		return 1;
	}

	ALUA(dungeon_revived)
	{
		if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2))
		{
			lua_pushboolean(L, 1);
			return 1;
		}

		CQuestManager& q = CQuestManager::instance();
		LPDUNGEON pDungeon = q.GetCurrentDungeon();

		if (pDungeon)
		{
			lua_pushboolean(L, pDungeon->IsUseRevive());
			return 1;
		}

		lua_pushboolean(L, 1);
		return 1;
	}

	ALUA(dungeon_unique_set_maxhp)
	{
		if (!lua_isstring(L, 1) || !lua_isnumber(L, 2))
			return 0;

		CQuestManager& q = CQuestManager::instance();
		LPDUNGEON pDungeon = q.GetCurrentDungeon();

		if (pDungeon)
			pDungeon->UniqueSetMaxHP(lua_tostring(L, 1), (HPTYPE)lua_tonumber(L, 2));

		return 0;
	}

	ALUA(dungeon_unique_set_hp)
	{
		if (!lua_isstring(L, 1) || !lua_isnumber(L, 2))
			return 0;

		CQuestManager& q = CQuestManager::instance();
		LPDUNGEON pDungeon = q.GetCurrentDungeon();

		if (pDungeon)
			pDungeon->UniqueSetHP(lua_tostring(L, 1), (HPTYPE)lua_tonumber(L, 2));

		return 0;
	}

	ALUA(dungeon_unique_set_def_grade)
	{
		if (!lua_isstring(L, 1) || !lua_isnumber(L, 2))
			return 0;

		CQuestManager& q = CQuestManager::instance();
		LPDUNGEON pDungeon = q.GetCurrentDungeon();

		if (pDungeon)
			pDungeon->UniqueSetDefGrade(lua_tostring(L, 1), (int)lua_tonumber(L, 2));

		return 0;
	}

	ALUA(dungeon_unique_get_hp_perc)
	{
		if (!lua_isstring(L, 1))
		{
			lua_pushnumber(L, 0);
			return 1;
		}

		CQuestManager& q = CQuestManager::instance();
		LPDUNGEON pDungeon = q.GetCurrentDungeon();

		if (pDungeon)
		{
			lua_pushnumber(L, pDungeon->GetUniqueHpPerc(lua_tostring(L, 1)));
			return 1;
		}

		lua_pushnumber(L, 0);
		return 1;
	}

	ALUA(dungeon_is_unique_dead)
	{
		if (!lua_isstring(L, 1))
		{
			lua_pushboolean(L, 0);
			return 1;
		}

		CQuestManager& q = CQuestManager::instance();
		LPDUNGEON pDungeon = q.GetCurrentDungeon();

		if (pDungeon)
		{
			lua_pushboolean(L, pDungeon->IsUniqueDead(lua_tostring(L, 1)) ? 1 : 0);
			return 1;
		}

		lua_pushboolean(L, 0);
		return 1;
	}

	ALUA(dungeon_purge_unique)
	{
		if (!lua_isstring(L, 1))
			return 0;
		sys_log(0, "QUEST_DUNGEON_PURGE_UNIQUE %s", lua_tostring(L, 1));

		CQuestManager& q = CQuestManager::instance();
		LPDUNGEON pDungeon = q.GetCurrentDungeon();

		if (pDungeon)
			pDungeon->PurgeUnique(lua_tostring(L, 1));

		return 0;
	}

	struct FPurgeArea
	{
		int x1, y1, x2, y2;
		LPCHARACTER ExceptChar;

		FPurgeArea(int a, int b, int c, int d, LPCHARACTER p)
			: x1(a), y1(b), x2(c), y2(d),
			ExceptChar(p)
		{}

		void operator () (LPENTITY ent)
		{
			if (true == ent->IsType(ENTITY_CHARACTER))
			{
				LPCHARACTER pChar = static_cast<LPCHARACTER>(ent);

				if (pChar == ExceptChar)
					return;

				if (!pChar->IsPet() &&
#ifdef __GROWTH_PET_SYSTEM__
					!pChar->IsNewPet() &&
#endif
#ifdef __GROWTH_MOUNT_SYSTEM__
					!pChar->IsNewMount() &&
#endif
#ifdef __MOUNT_COSTUME_SYSTEM__
					!pChar->IsMount() &&
#endif
				(true == pChar->IsMonster() || true == pChar->IsStone()))
				{
					if (x1 <= pChar->GetX() && pChar->GetX() <= x2 && y1 <= pChar->GetY() && pChar->GetY() <= y2)
					{
						M2_DESTROY_CHARACTER(pChar);
					}
				}
			}
		}
	};

	ALUA(dungeon_purge_area)
	{
		if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3) || !lua_isnumber(L, 4))
			return 0;
		sys_log(0, "QUEST_DUNGEON_PURGE_AREA");

		int x1 = lua_tonumber(L, 1);
		int y1 = lua_tonumber(L, 2);
		int x2 = lua_tonumber(L, 3);
		int y2 = lua_tonumber(L, 4);

		CQuestManager& q = CQuestManager::instance();
		LPDUNGEON pDungeon = q.GetCurrentDungeon();

		const int mapIndex = pDungeon->GetMapIndex();

		if (0 == mapIndex)
		{
			sys_err("_purge_area: cannot get a map index with (%u, %u)", x1, y1);
			return 0;
		}

		LPSECTREE_MAP pSectree = SECTREE_MANAGER::instance().GetMap(mapIndex);

		if (NULL != pSectree)
		{
			FPurgeArea func(x1, y1, x2, y2, CQuestManager::instance().GetCurrentNPCCharacterPtr());

			pSectree->for_each(func);
		}

		return 0;
	}

	ALUA(dungeon_kill_unique)
	{
		if (!lua_isstring(L, 1))
			return 0;
		sys_log(0, "QUEST_DUNGEON_KILL_UNIQUE %s", lua_tostring(L, 1));

		CQuestManager& q = CQuestManager::instance();
		LPDUNGEON pDungeon = q.GetCurrentDungeon();

		if (pDungeon)
			pDungeon->KillUnique(lua_tostring(L, 1));

		return 0;
	}

	ALUA(dungeon_set_unique)
	{
		if (!lua_isstring(L, 1) || !lua_isnumber(L, 2))
			return 0;

		CQuestManager& q = CQuestManager::instance();
		LPDUNGEON pDungeon = q.GetCurrentDungeon();

		DWORD vid = (DWORD)lua_tonumber(L, 2);

		if (pDungeon)
			pDungeon->SetUnique(lua_tostring(L, 1), vid);
		return 0;
	}

	ALUA(dungeon_is_available0)
	{
		CQuestManager& q = CQuestManager::instance();
		LPDUNGEON pDungeon = q.GetCurrentDungeon();

		lua_pushboolean(L, pDungeon != NULL);
		return 1;
	}

	ALUA(dungeon_get_unique_vid)
	{
		if (!lua_isstring(L, 1))
		{
			lua_pushnumber(L, 0);
			return 1;
		}

		CQuestManager& q = CQuestManager::instance();
		LPDUNGEON pDungeon = q.GetCurrentDungeon();

		if (pDungeon)
		{
			lua_pushnumber(L, pDungeon->GetUniqueVid(lua_tostring(L, 1)));
			return 1;
		}

		lua_pushnumber(L, 0);
		return 1;
	}

	ALUA(dungeon_spawn_mob)
	{
		if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3))
		{
			sys_err("invalid argument");
			return 0;
		}

		CQuestManager& q = CQuestManager::instance();
		LPDUNGEON pDungeon = q.GetCurrentDungeon();

		DWORD vid = 0;

		if (pDungeon)
		{
			DWORD dwVnum = (DWORD)lua_tonumber(L, 1);
			long x = (long)lua_tonumber(L, 2);
			long y = (long)lua_tonumber(L, 3);
			float radius = lua_isnumber(L, 4) ? (float)lua_tonumber(L, 4) : 0;
			DWORD count = (lua_isnumber(L, 5)) ? (DWORD)lua_tonumber(L, 5) : 1;

			sys_log(0, "dungeon_spawn_mob %u %d %d", dwVnum, x, y);

			if (count == 0)
				count = 1;

			while (count--)
			{
				if (radius < 1)
				{
					LPCHARACTER ch = pDungeon->SpawnMob(dwVnum, x, y);
					if (ch && !vid)
						vid = ch->GetVID();
				}
				else
				{
					float angle = number(0, 999) * M_PI * 2 / 1000;
					float r = number(0, 999) * radius / 1000;

					long nx = x + (long)(r * cos(angle));
					long ny = y + (long)(r * sin(angle));

					LPCHARACTER ch = pDungeon->SpawnMob(dwVnum, nx, ny);
					if (ch && !vid)
						vid = ch->GetVID();
				}
			}
		}

		lua_pushnumber(L, vid);
		return 1;
	}

	ALUA(dungeon_spawn_mob_dir)
	{
		if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3) || !lua_isnumber(L, 4))
		{
			sys_err("invalid argument");
			return 0;
		}

		CQuestManager& q = CQuestManager::instance();
		LPDUNGEON pDungeon = q.GetCurrentDungeon();

		DWORD vid = 0;

		if (pDungeon)
		{
			DWORD dwVnum = (DWORD)lua_tonumber(L, 1);
			long x = (long)lua_tonumber(L, 2);
			long y = (long)lua_tonumber(L, 3);
			BYTE dir = (int)lua_tonumber(L, 4);

			LPCHARACTER ch = pDungeon->SpawnMob(dwVnum, x, y, dir);
			if (ch && !vid)
				vid = ch->GetVID();
		}
		lua_pushnumber(L, vid);
		return 1;
	}

	ALUA(dungeon_spawn_mob_ac_dir)
	{
		if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3) || !lua_isnumber(L, 4))
		{
			sys_err("invalid argument");
			return 0;
		}

		CQuestManager& q = CQuestManager::instance();
		LPDUNGEON pDungeon = q.GetCurrentDungeon();

		DWORD vid = 0;

		if (pDungeon)
		{
			DWORD dwVnum = (DWORD)lua_tonumber(L, 1);
			long x = (long)lua_tonumber(L, 2);
			long y = (long)lua_tonumber(L, 3);
			BYTE dir = (int)lua_tonumber(L, 4);

			LPCHARACTER ch = pDungeon->SpawnMob_ac_dir(dwVnum, x, y, dir);
			if (ch && !vid)
				vid = ch->GetVID();
		}
		lua_pushnumber(L, vid);
		return 1;
	}

	ALUA(dungeon_spawn_goto_mob)
	{
		if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3) || !lua_isnumber(L, 4))
			return 0;

		long lFromX = (long)lua_tonumber(L, 1);
		long lFromY = (long)lua_tonumber(L, 2);
		long lToX = (long)lua_tonumber(L, 3);
		long lToY = (long)lua_tonumber(L, 4);

		CQuestManager& q = CQuestManager::instance();
		LPDUNGEON pDungeon = q.GetCurrentDungeon();

		if (pDungeon)
			pDungeon->SpawnGotoMob(lFromX, lFromY, lToX, lToY);

		return 0;
	}

	ALUA(dungeon_spawn_name_mob)
	{
		if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3) || !lua_isstring(L, 4))
			return 0;

		CQuestManager& q = CQuestManager::instance();
		LPDUNGEON pDungeon = q.GetCurrentDungeon();

		if (pDungeon)
		{
			DWORD dwVnum = (DWORD)lua_tonumber(L, 1);
			long x = (long)lua_tonumber(L, 2);
			long y = (long)lua_tonumber(L, 3);
			pDungeon->SpawnNameMob(dwVnum, x, y, lua_tostring(L, 4));
		}
		return 0;
	}

	ALUA(dungeon_spawn_group)
	{
		//
		// argument: vnum,x,y,radius,aggressive,count
		//
		if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3) || !lua_isnumber(L, 4) || !lua_isnumber(L, 6))
		{
			sys_err("invalid argument");
			return 0;
		}

		DWORD vid = 0;

		CQuestManager& q = CQuestManager::instance();
		LPDUNGEON pDungeon = q.GetCurrentDungeon();

		if (pDungeon)
		{
			DWORD group_vnum = (DWORD)lua_tonumber(L, 1);
			long local_x = (long)lua_tonumber(L, 2) * 100;
			long local_y = (long)lua_tonumber(L, 3) * 100;
			float radius = (float)lua_tonumber(L, 4) * 100;
			bool bAggressive = lua_toboolean(L, 5);
			DWORD count = (DWORD)lua_tonumber(L, 6);

			LPCHARACTER chRet = pDungeon->SpawnGroup(group_vnum, local_x, local_y, radius, bAggressive, count);
			if (chRet)
				vid = chRet->GetVID();
		}

		lua_pushnumber(L, vid);
		return 1;
	}

	ALUA(dungeon_join)
	{
		if (lua_gettop(L) < 1 || !lua_isnumber(L, 1))
			return 0;

		long lMapIndex = (long)lua_tonumber(L, 1);
		LPDUNGEON pDungeon = CDungeonManager::instance().Create(lMapIndex);

		if (!pDungeon)
			return 0;

		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();

		pDungeon->Join(ch);

		return 0;
	}

	ALUA(dungeon_exit)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		if (!ch)
			return 0;

		ch->ExitToSavedLocation();
		return 0;
	}

	ALUA(dungeon_exit_all)
	{
		CQuestManager& q = CQuestManager::instance();
		LPDUNGEON pDungeon = q.GetCurrentDungeon();

		if (pDungeon)
			pDungeon->ExitAll();

		return 0;
	}

	ALUA(dungeon_kill_all)
	{
		CQuestManager& q = CQuestManager::instance();
		LPDUNGEON pDungeon = q.GetCurrentDungeon();

		if (pDungeon)
			pDungeon->KillAll();

		return 0;
	}

	ALUA(dungeon_purge)
	{
		CQuestManager& q = CQuestManager::instance();
		LPDUNGEON pDungeon = q.GetCurrentDungeon();

		if (pDungeon)
			pDungeon->Purge(CQuestManager::instance().GetCurrentNPCCharacterPtr());

		return 0;
	}

	ALUA(dungeon_exit_all_to_start_position)
	{
		CQuestManager& q = CQuestManager::instance();
		LPDUNGEON pDungeon = q.GetCurrentDungeon();

		if (pDungeon)
			pDungeon->ExitAllToStartPosition();

		return 0;
	}

	ALUA(dungeon_count_monster)
	{
		CQuestManager& q = CQuestManager::instance();
		LPDUNGEON pDungeon = q.GetCurrentDungeon();

		if (pDungeon)
			lua_pushnumber(L, pDungeon->CountMonster());
		else
		{
			sys_err("not in a dungeon");
			lua_pushnumber(L, LONG_MAX);
		}

		return 1;
	}

	ALUA(dungeon_select)
	{
		DWORD dwMapIndex = (DWORD)lua_tonumber(L, 1);
		if (dwMapIndex)
		{
			LPDUNGEON pDungeon = CDungeonManager::instance().FindByMapIndex(dwMapIndex);
			if (pDungeon)
			{
				CQuestManager::instance().SelectDungeon(pDungeon);
				lua_pushboolean(L, 1);
			}
			else
			{
				CQuestManager::instance().SelectDungeon(NULL);
				lua_pushboolean(L, 0);
			}
		}
		else
		{
			CQuestManager::instance().SelectDungeon(NULL);
			lua_pushboolean(L, 0);
		}
		return 1;
	}

	ALUA(dungeon_find)
	{
		DWORD dwMapIndex = (DWORD)lua_tonumber(L, 1);
		if (dwMapIndex)
		{
			LPDUNGEON pDungeon = CDungeonManager::instance().FindByMapIndex(dwMapIndex);
			if (pDungeon)
			{
				lua_pushboolean(L, 1);
			}
			else
			{
				lua_pushboolean(L, 0);
			}
		}
		else
		{
			lua_pushboolean(L, 0);
		}
		return 1;
	}

	ALUA(dungeon_all_near_to)
	{
		LPDUNGEON pDungeon = CQuestManager::instance().GetCurrentDungeon();

		if (pDungeon != NULL)
		{
			lua_pushboolean(L, pDungeon->IsAllPCNearTo((int)lua_tonumber(L, 1), (int)lua_tonumber(L, 2), 30));
		}
		else
		{
			lua_pushboolean(L, false);
		}

		return 1;
	}

	ALUA(dungeon_exit_all_with_delay)
	{
		CQuestManager& q = CQuestManager::instance();
		LPDUNGEON pDungeon;

		if (lua_gettop(L) == 2 && lua_isnumber(L, 2))
			pDungeon = CDungeonManager::instance().FindByMapIndex((DWORD)lua_tonumber(L, 2));
		else
			pDungeon = q.GetCurrentDungeon();

		int delay = (int)lua_tonumber(L, 1);

		if (pDungeon)
			pDungeon->ExitAllWithDelay(delay);

		return 0;
	}
	ALUA(dungeon_get_remain_time)
	{
		CQuestManager& q = CQuestManager::instance();
		LPDUNGEON pDungeon;

		if (lua_gettop(L) == 1 && lua_isnumber(L, 1))
			pDungeon = CDungeonManager::instance().FindByMapIndex((DWORD)lua_tonumber(L, 1));
		else
			pDungeon = q.GetCurrentDungeon();

		if (pDungeon)
		{
			lua_pushnumber(L, pDungeon->GetExitRemainTime());
		}
		else
		{
			sys_err("no dungeon (lua_gettop(L) == 1: %d)", lua_gettop(L) == 1);
			lua_pushnumber(L, 0);
		}

		return 1;
	}
	ALUA(dungeon_jump_all)
	{
		if (lua_gettop(L) < 2 || !lua_isnumber(L, 1) || !lua_isnumber(L, 2))
			return 0;

		LPDUNGEON pDungeon = CQuestManager::instance().GetCurrentDungeon();

		if (!pDungeon)
			return 0;

		pDungeon->JumpAll(pDungeon->GetMapIndex(), (int)lua_tonumber(L, 1), (int)lua_tonumber(L, 2));
		pDungeon->SetJoinCoordinate((int)lua_tonumber(L, 1) * 100, (int)lua_tonumber(L, 2) * 100);
		return 0;
	}

	ALUA(dungeon_pc_warp)
	{
		if (lua_gettop(L) < 1)
		{
			sys_log(1, "not enough argument");
			lua_pushboolean(L, false);
			return 1;
		}

		if (!lua_isnumber(L, 1))
		{
			sys_log(1, "wrong argument");
			lua_pushboolean(L, false);
			return 1;
		}

		CQuestManager& q = CQuestManager::instance();
		long lMapIndex = (long)lua_tonumber(L, 1);
		LPDUNGEON pDungeon = CDungeonManager::instance().FindByMapIndex(lMapIndex);
		LPCHARACTER ch = q.GetCurrentCharacterPtr();

		if (!pDungeon || !ch)
		{
			lua_pushboolean(L, false);
			return 1;
		}

		pDungeon->RegisterUser(ch);
		ch->SaveExitLocation();

		lua_pushboolean(L, ch->WarpSet(pDungeon->GetJoinX(), pDungeon->GetJoinY(), pDungeon->GetMapIndex()));

		return 1;
	}
	ALUA(dungeon_count_monster_by_vnum)
	{
		CQuestManager& q = CQuestManager::instance();
		LPDUNGEON pDungeon = q.GetCurrentDungeon();

		if (pDungeon)
		{
			if (lua_gettop(L) == 1)
				lua_pushnumber(L, pDungeon->CountRealMonster((DWORD)lua_tonumber(L, 1)));
			else
				lua_pushnumber(L, pDungeon->CountRealMonster(q.GetCurrentNPCRace()));
		}
		else
		{
			sys_log(1, "not in a dungeon");
			lua_pushnumber(L, LONG_MAX);
		}

		return 1;
	}
	ALUA(dungeon_is_eliminated)
	{
		CQuestManager& q = CQuestManager::instance();
		LPDUNGEON pDungeon = q.GetCurrentDungeon();

		lua_pushboolean(L, pDungeon ? pDungeon->IsEliminated() : false);
		return 1;
	}
	ALUA(dungeon_drop_item)
	{
		DWORD item_vnum = (DWORD)lua_tonumber(L, 1);
		int count = 1;
		if (lua_gettop(L) == 2)
			count = (int)lua_tonumber(L, 2);

		CQuestManager& q = CQuestManager::instance();
		LPDUNGEON pDungeon = q.GetCurrentDungeon();

		if (!pDungeon)
		{
			sys_log(1, "not in a dungeon");
			return 0;
		}

		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();

		if (!ch)
		{
			ch = pDungeon->GetRandomMember();
			if (!ch)
			{
				sys_err("merte goster burayi no ch");
				return 0;
			}
		}

		long x = ch->GetX();
		long y = ch->GetY();

		LPITEM item = ITEM_MANAGER::instance().CreateItem(item_vnum, count);

		if (!item)
		{
			sys_log(1, "cannot create item vnum %d count %d", item_vnum, count);
			return 0;
		}

		PIXEL_POSITION pos;
		pos.x = x + number(-200, 200);
		pos.y = y + number(-200, 200);

		pDungeon->RegisterItem(item->GetID());

		item->AddToGround(ch->GetMapIndex(), pos);
		item->SetSocket(ITEM_SOCKET_UNIQUE_REMAIN_TIME, get_global_time() + 12 * 60 * 60);

		return 0;
	}
	ALUA(dungeon_give_item)
	{
		CQuestManager& q = CQuestManager::instance();
		LPDUNGEON pDungeon = q.GetCurrentDungeon();
		LPCHARACTER ch = q.GetCurrentCharacterPtr();

		if (!pDungeon)
		{
			sys_log(1, "not in a dungeon");
			lua_pushnumber(L, 0);
			return 1;
		}

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

		LPITEM item = ch->AutoGiveItem(dwVnum, icount);

		if (NULL != item)
		{
			pDungeon->RegisterItem(item->GetID());
			item->SetSocket(ITEM_SOCKET_UNIQUE_REMAIN_TIME, get_global_time() + 12 * 60 * 60);
			lua_pushnumber(L, item->GetID());
		}
		else
			lua_pushnumber(L, 0);

		return 0;
	}
	ALUA(dungeon_big_notice)
	{
		if (!lua_isstring(L, 1))
			return 0;

		CQuestManager& q = CQuestManager::instance();
		LPDUNGEON pDungeon = q.GetCurrentDungeon();

		if (pDungeon)
			pDungeon->Big_Notice(lua_tostring(L, 1));
		return 0;
	}

	ALUA(dungeon_cmdchat)
	{
		if (!lua_isstring(L, 1))
			return 0;

		CQuestManager& q = CQuestManager::instance();
		LPDUNGEON pDungeon = q.GetCurrentDungeon();

		if (pDungeon)
			pDungeon->cmdchat(lua_tostring(L, 1));
		return 0;
	}

	ALUA(dungeon_set_mission)
	{
		if (!lua_isstring(L, 1))
			return 0;

		CQuestManager& q = CQuestManager::instance();
		LPDUNGEON pDungeon = q.GetCurrentDungeon();

		if (pDungeon)
			pDungeon->SetMission(lua_tostring(L, 1));
		return 0;
	}

	ALUA(dungeon_set_submission)
	{
		if (!lua_isstring(L, 1))
			return 0;

		CQuestManager& q = CQuestManager::instance();
		LPDUNGEON pDungeon = q.GetCurrentDungeon();

		if (pDungeon)
			pDungeon->SetSubMission(lua_tostring(L, 1));
		return 0;
	}

	ALUA(dungeon_update_pc_mission)
	{
		CQuestManager& q = CQuestManager::instance();
		LPDUNGEON pDungeon = q.GetCurrentDungeon();
		LPCHARACTER ch = q.GetCurrentCharacterPtr();

		if (!pDungeon || !ch)
			return 0;

		ch->ChatPacket(CHAT_TYPE_DUNGEON_MISSION, "%s", pDungeon->mission.c_str());
		ch->ChatPacket(CHAT_TYPE_DUNGEON_SUBMISSION, "%s", pDungeon->submission.c_str());

		return 0;
	}

	ALUA(dungeon_get_orig_map_index)
	{
		CQuestManager& q = CQuestManager::instance();
		LPDUNGEON pDungeon = q.GetCurrentDungeon();

		if (pDungeon)
		{
			sys_log(0, "Dungeon GetOrigMapIndex %d", pDungeon->GetOrigMapIndex());
			lua_pushnumber(L, pDungeon->GetOrigMapIndex());
		}
		else
		{
			lua_pushnumber(L, 0);
			return 0;
		}

		return 1;
	}

	ALUA(dungeon_set_flag_to_map_index)
	{
		if (!lua_isstring(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3))
		{
			sys_log(1, "wrong get flag");
			return 0;
		}

		DWORD dwMapIndex = (DWORD)lua_tonumber(L, 3);
		if (dwMapIndex)
		{
			LPDUNGEON pDungeon = CDungeonManager::instance().FindByMapIndex(dwMapIndex);
			if (pDungeon)
			{
				const char* sz = lua_tostring(L, 1);
				int value = int(lua_tonumber(L, 2));
				pDungeon->SetFlag(sz, value);
				lua_pushnumber(L, 1);
			}
			else
			{
				sys_log(1, "no dungeon !!!");
				lua_pushnumber(L, 0);
			}
		}
		else
		{
			lua_pushnumber(L, 0);
		}
		return 1;
	}

	ALUA(dungeon_kill_all_mob)
	{
		CQuestManager& q = CQuestManager::instance();
		LPDUNGEON pDungeon = q.GetCurrentDungeon();

		if (pDungeon)
			pDungeon->KillAllMob();

		return 0;
	}

	ALUA(dungeon_purge_mob)
	{
		CQuestManager& q = CQuestManager::instance();
		LPDUNGEON pDungeon = q.GetCurrentDungeon();

		if (pDungeon)
			pDungeon->PurgeMob(CQuestManager::instance().GetCurrentNPCCharacterPtr());

		return 0;
	}

	ALUA(dungeon_count_npc)
	{
		CQuestManager& q = CQuestManager::instance();
		LPDUNGEON pDungeon = q.GetCurrentDungeon();

		if (pDungeon)
			lua_pushnumber(L, pDungeon->CountNpc());
		else
		{
			sys_log(1, "not in a dungeon");
			lua_pushnumber(L, LONG_MAX);
		}

		return 1;
	}
	ALUA(dungeon_real_count_monster)
	{
		CQuestManager& q = CQuestManager::instance();
		LPDUNGEON pDungeon = q.GetCurrentDungeon();

		if (pDungeon)
			lua_pushnumber(L, pDungeon->CountRealMonster());
		else
		{
			sys_log(1, "not in a dungeon");
			lua_pushnumber(L, LONG_MAX);
		}

		return 1;
	}

	ALUA(dungeon_register_user)
	{
		if (!lua_isnumber(L, 1))
		{
			sys_err("Invalid Argument dungeon_register_user");
		}

		// LPDUNGEON pDungeon = nullptr;
		// if (lua_gettop(L) == 2)
		// 	pDungeon = CDungeonManager::instance().FindByMapIndex((long)lua_tonumber(L, 2));
		// else
		// 	pDungeon = CQuestManager::instance().GetCurrentDungeon();

		// if (pDungeon)
			// pDungeon->RegisterUser(lua_tonumber(L, 1));

		return 0;
	}
	ALUA(dungeon_is_registered_user)
	{
		if (!lua_isnumber(L, 1))
		{
			sys_err("Invalid Argument dungeon_register_user");
			return 0;
		}

		LPDUNGEON pDungeon;
		if (lua_gettop(L) == 2)
			pDungeon = CDungeonManager::instance().FindByMapIndex((long)lua_tonumber(L, 2));
		else
			pDungeon = CQuestManager::instance().GetCurrentDungeon();

		if (!pDungeon || !pDungeon->IsRegisteredUser(lua_tonumber(L, 1)))
			lua_pushboolean(L, 0);
		else
			lua_pushboolean(L, 1);

		return 1;
	}
	ALUA(dungeon_reward_user)
	{
		if (!lua_isnumber(L, 1))
		{
			sys_err("Invalid Argument dungeon_register_user");
		}

		LPDUNGEON pDungeon = nullptr;
		if (lua_gettop(L) == 2)
			pDungeon = CDungeonManager::instance().FindByMapIndex((long)lua_tonumber(L, 2));
		else
			pDungeon = CQuestManager::instance().GetCurrentDungeon();

		if (pDungeon)
			pDungeon->RewardUser(lua_tonumber(L, 1));

		return 0;
	}
	ALUA(dungeon_unreward_user)
	{
		if (!lua_isnumber(L, 1))
		{
			sys_err("Invalid Argument dungeon_register_user");
		}

		LPDUNGEON pDungeon = nullptr;
		if (lua_gettop(L) == 2)
			pDungeon = CDungeonManager::instance().FindByMapIndex((long)lua_tonumber(L, 2));
		else
			pDungeon = CQuestManager::instance().GetCurrentDungeon();

		if (pDungeon)
			pDungeon->UnRewardUser(lua_tonumber(L, 1));

		return 0;
	}
	ALUA(dungeon_is_reward_user)
	{
		if (!lua_isnumber(L, 1))
		{
			sys_err("Invalid Argument dungeon_register_user");
			return 0;
		}

		LPDUNGEON pDungeon;
		if (lua_gettop(L) == 2)
			pDungeon = CDungeonManager::instance().FindByMapIndex((long)lua_tonumber(L, 2));
		else
			pDungeon = CQuestManager::instance().GetCurrentDungeon();

		if (!pDungeon || !pDungeon->IsRewardUser(lua_tonumber(L, 1)))
			lua_pushboolean(L, 0);
		else
			lua_pushboolean(L, 1);

		return 1;
	}
	ALUA(dungeon_timer)
	{
		if (!lua_isstring(L, 1) || !lua_isnumber(L, 2))
		{
			sys_err("wrong arg");
		}
		else
		{
			CQuestManager& q = CQuestManager::instance();
			LPDUNGEON pDungeon = q.GetCurrentDungeon();

			if (pDungeon == nullptr) return 0;

			const char* name = lua_tostring(L, 1);
			double t = lua_tonumber(L, 2);
			DWORD arg = pDungeon->GetMapIndex();

			int timernpc = q.LoadTimerScript(name);
			LPEVENT event = quest_create_server_timer_event(name, t, timernpc, false, arg);
			q.ClearServerTimer(name, arg);
			q.AddServerTimer(name, arg, event);
		}
		return 0;
	}
	ALUA(dungeon_loop_timer)
	{
		if (!lua_isstring(L, 1) || !lua_isnumber(L, 2))
		{
			sys_err("wrong arg");
		}
		else
		{
			CQuestManager& q = CQuestManager::instance();
			LPDUNGEON pDungeon = q.GetCurrentDungeon();

			if (pDungeon == nullptr) return 0;

			const char* name = lua_tostring(L, 1);
			double t = lua_tonumber(L, 2);
			DWORD arg = pDungeon->GetMapIndex();

			int timernpc = q.LoadTimerScript(name);
			LPEVENT event = quest_create_server_timer_event(name, t, timernpc, true, arg);
			q.ClearServerTimer(name, arg);
			q.AddServerTimer(name, arg, event);
		}
		return 0;
	}
	ALUA(dungeon_clear_timer)
	{
		if (!lua_isstring(L, 1))
		{
			sys_err("wrong arg");
		}
		else
		{
			CQuestManager& q = CQuestManager::instance();
			LPDUNGEON pDungeon = q.GetCurrentDungeon();

			if (pDungeon == nullptr) return 0;

			const char* name = lua_tostring(L, 1);
			DWORD arg = pDungeon->GetMapIndex();

			q.ClearServerTimer(name, arg);
		}
		return 0;
	}
	ALUA(dungeon_pc_get_rooms)
	{
		long pid = (long)lua_tonumber(L, 1);
		if (!pid)
			return 0;

		LPCHARACTER ch = CHARACTER_MANAGER::instance().FindByPID(lua_tonumber(L, 1));
		if (!ch || !ch->GetDesc())
			return 0;

// #ifdef __GAME_OPTION_DLG_RENEWAL__
// 		if (ch->GetGameOptionValue(OPTION_HIDE_DUNGEON) == 1)
// 			return 0;
// #endif

		std::set<LPDUNGEON> dungSet;
		CDungeonManager::instance().FindPcRegisteredDungeon(pid, dungSet);

		char title[] = "Zindan Bilgi";
		char msg[256];

		{
			memset(msg, 0, sizeof(msg));
			snprintf(msg, sizeof(msg), "Kanal-%d Kayitli oldugunuz zindanlar:", g_bChannel);

			TPacketGCWhisper pack;
			int len = MIN(CHAT_MAX_LEN, strlen(msg) + 1);
			pack.bHeader = HEADER_GC_WHISPER;
			pack.wSize = sizeof(TPacketGCWhisper) + len;
			pack.bType = WHISPER_TYPE_SYSTEM;
			strlcpy(pack.szNameFrom, title, sizeof(pack.szNameFrom));
			TEMP_BUFFER buf;
			buf.write(&pack, sizeof(TPacketGCWhisper));
			buf.write(msg, len);
			ch->GetDesc()->Packet(buf.read_peek(), buf.size());
		}
		for (auto&& i : dungSet)
		{
			memset(msg, 0, sizeof(msg));
			snprintf(msg, sizeof(msg), "%s:: No: %ld Sifre: %ld", GetMapName(i->GetOrigMapIndex()), i->GetRoomNo(), i->GetRoomPass());

			TPacketGCWhisper pack;
			int len = MIN(CHAT_MAX_LEN, strlen(msg) + 1);
			pack.bHeader = HEADER_GC_WHISPER;
			pack.wSize = sizeof(TPacketGCWhisper) + len;
			pack.bType = WHISPER_TYPE_SYSTEM;
			strlcpy(pack.szNameFrom, title, sizeof(pack.szNameFrom));
			TEMP_BUFFER buf;
			buf.write(&pack, sizeof(TPacketGCWhisper));
			buf.write(msg, len);
			ch->GetDesc()->Packet(buf.read_peek(), buf.size());
		}
		{
			memset(msg, 0, sizeof(msg));
			snprintf(msg, sizeof(msg), "Açýk olan odalarýnýza tekrar girebilmeniz için Odaya katýl kýsmýna týkladýktan sonra oda numaranýzý yazmanýz gerekmektedir.(Baþka bir oyuncunun odasýna girmek için oda no yazdýktan sonra çýkan kutucuða odanýn þifresini girmeniz gerekmektedir) ");

			TPacketGCWhisper pack;
			int len = MIN(CHAT_MAX_LEN, strlen(msg) + 1);
			pack.bHeader = HEADER_GC_WHISPER;
			pack.wSize = sizeof(TPacketGCWhisper) + len;
			pack.bType = WHISPER_TYPE_SYSTEM;
			strlcpy(pack.szNameFrom, title, sizeof(pack.szNameFrom));
			TEMP_BUFFER buf;
			buf.write(&pack, sizeof(TPacketGCWhisper));
			buf.write(msg, len);
			ch->GetDesc()->Packet(buf.read_peek(), buf.size());
		}
		return 0;
	}
	ALUA(dungeon_info_by_name)
	{
		if (!lua_isstring(L, 1))
		{
			sys_err("wrong arg");
			return 0;
		}

		const char* name = lua_tostring(L, 1);

		for (const auto& entry : dungeonTable)
		{
			if (!strcmp(entry.quest_name.c_str(), name))
			{
				lua_pushstring(L, entry.quest_name.c_str());
				lua_pushstring(L, entry.map_name.c_str());
				lua_pushnumber(L, entry.min_level);
				lua_pushnumber(L, entry.max_level);
				lua_pushnumber(L, entry.item_vnum);
				lua_pushnumber(L, entry.map_index);
				lua_pushnumber(L, entry.cooldown);
				lua_pushnumber(L, entry.duration);
				lua_pushnumber(L, entry.type);

				return 9;
			}
		}

		return 0;
	}
	ALUA(dungeon_get_last)
	{
		if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2))
		{
			sys_err("wrong arg");
			return 0;
		}

		DWORD pid = lua_tonumber(L, 1);
		DWORD mapIndex = lua_tonumber(L, 2);

		LPDUNGEON dungeon = CDungeonManager::instance().FindLastRegisteredDungeon(pid, mapIndex);

		lua_pushnumber(L, dungeon ? dungeon->GetMapIndex() : 0);
		return 1;
	}
	ALUA(dungeon_info_by_index)
	{
		if (!lua_isnumber(L, 1))
		{
			sys_err("wrong arg");
			return 0;
		}

		int ind = lua_tonumber(L, 1);

		const auto& entry = dungeonTable.at(ind);

		lua_pushstring(L, entry.quest_name.c_str());
		lua_pushstring(L, entry.map_name.c_str());
		lua_pushnumber(L, entry.min_level);
		lua_pushnumber(L, entry.max_level);
		lua_pushnumber(L, entry.item_vnum);
		lua_pushnumber(L, entry.map_index);
		lua_pushnumber(L, entry.cooldown);
		lua_pushnumber(L, entry.duration);
		lua_pushnumber(L, entry.type);

		return 8+1;
	}
	ALUA(dungeon_info_by_map_index)
	{
		if (!lua_isnumber(L, 1))
		{
			sys_err("wrong arg");
			return 0;
		}

		int ind = lua_tonumber(L, 1);

		for (const auto& entry : dungeonTable)
		{
			if (entry.map_index == ind)
			{
				lua_pushstring(L, entry.quest_name.c_str());
				lua_pushstring(L, entry.map_name.c_str());
				lua_pushnumber(L, entry.min_level);
				lua_pushnumber(L, entry.max_level);
				lua_pushnumber(L, entry.item_vnum);
				lua_pushnumber(L, entry.map_index);
				lua_pushnumber(L, entry.cooldown);
				lua_pushnumber(L, entry.duration);
				lua_pushnumber(L, entry.type);

				return 8+1;
			}
		}

		return 0;
	}
	ALUA(dungeon_info_size)
	{
		lua_pushnumber(L, dungeonTable.size());
		return 1;
	}
	int dungeon_is_registered_item(lua_State* L)
	{
		if (!lua_isnumber(L, 1))
		{
			sys_err("Invalid Argument dungeon_register_user");
			return 0;
		}

		LPDUNGEON pDungeon = CQuestManager::instance().GetCurrentDungeon();

		if (!pDungeon || !pDungeon->IsRegisteredItem(lua_tonumber(L, 1)))
			lua_pushboolean(L, 0);
		else
			lua_pushboolean(L, 1);

		return 1;
	}

#ifdef __DUNGEON_TEST_MODE__
	ALUA(dungeon_set_test_mode)
	{
		LPDUNGEON pDungeon = CQuestManager::instance().GetCurrentDungeon();

		if (pDungeon)
			pDungeon->SetTest(lua_toboolean(L, 1));

		sys_log(0, "dungeon_set_test_mode %d", pDungeon->GetMapIndex());
		return 0;
	}
	ALUA(dungeon_is_test_mode)
	{
		LPDUNGEON pDungeon;

		if (lua_gettop(L) == 1 && lua_isnumber(L, 1))
			pDungeon = CDungeonManager::instance().FindByMapIndex((DWORD)lua_tonumber(L, 1));
		else
			pDungeon = CQuestManager::instance().GetCurrentDungeon();

		lua_pushboolean(L, pDungeon && pDungeon->IsTest());
		return 1;
	}
#endif // __DUNGEON_TEST_MODE__

	void RegisterDungeonFunctionTable()
	{
		luaL_reg dungeon_functions[] =
			{
				{"join", dungeon_join},
				{"exit", dungeon_exit},
				{"exit_all", dungeon_exit_all},
				{"purge", dungeon_purge},
				{"kill_all", dungeon_kill_all},
				{"spawn_mob", dungeon_spawn_mob},
				{"spawn_mob_dir", dungeon_spawn_mob_dir},
				{"spawn_mob_ac_dir", dungeon_spawn_mob_ac_dir},
				{"spawn_name_mob", dungeon_spawn_name_mob},
				{"spawn_goto_mob", dungeon_spawn_goto_mob},
				{"spawn_group", dungeon_spawn_group},
				{"purge_unique", dungeon_purge_unique},
				{"purge_area", dungeon_purge_area},
				{"kill_unique", dungeon_kill_unique},
				{"is_unique_dead", dungeon_is_unique_dead},
				{"unique_get_hp_perc", dungeon_unique_get_hp_perc},
				{"unique_set_def_grade", dungeon_unique_set_def_grade},
				{"unique_set_hp", dungeon_unique_set_hp},
				{"unique_set_maxhp", dungeon_unique_set_maxhp},
				{"get_unique_vid", dungeon_get_unique_vid},
				{"is_use_potion", dungeon_is_use_potion},
				{"revived", dungeon_revived},
				{"jump_all", dungeon_jump_all},
				{"warp_all", dungeon_warp_all},
				{"regen_file", dungeon_regen_file},
				{"set_regen_file", dungeon_set_regen_file},
				{"clear_regen", dungeon_clear_regen},
				{"get_map_index", dungeon_get_map_index},
				{"count_monster", dungeon_count_monster},
				{"setf", dungeon_set_flag},
				{"getf", dungeon_get_flag},
				{"getf_from_map_index", dungeon_get_flag_from_map_index},
				{"set_unique", dungeon_set_unique},
				{"is_available0", dungeon_is_available0}, // [return lua boolean]
				{"select", dungeon_select},
				{"find", dungeon_find},
				{"notice", dungeon_notice},
				{"all_near_to", dungeon_all_near_to},
				{"exit_all_with_delay", dungeon_exit_all_with_delay},
				{"get_remain_time", dungeon_get_remain_time},
				{"pc_warp", dungeon_pc_warp},
				{"count_monster_by_vnum", dungeon_count_monster_by_vnum},
				{"is_eliminated", dungeon_is_eliminated},
				{"drop_item", dungeon_drop_item},
				{"give_item", dungeon_give_item},
				{"big_notice", dungeon_big_notice},
				{"cmdchat", dungeon_cmdchat},
				{"set_mission", dungeon_set_mission},
				{"set_submission", dungeon_set_submission},
				{"update_pc_mission", dungeon_update_pc_mission},
				{"get_orig_map_index", dungeon_get_orig_map_index},
				{"setf_from_to_index", dungeon_set_flag_to_map_index},
				{"purge_mob", dungeon_purge_mob},
				{"kill_all_mob", dungeon_kill_all_mob},
				{"count_npc", dungeon_count_npc},
				{"count_real_monster", dungeon_real_count_monster},
				{"register_user", dungeon_register_user},
				{"is_registered_user", dungeon_is_registered_user},
				{"reward_user", dungeon_reward_user},
				{"unreward_user", dungeon_unreward_user},
				{"is_reward_user", dungeon_is_reward_user},
				{"timer", dungeon_timer},
				{"loop_timer", dungeon_loop_timer},
				{"clear_timer", dungeon_clear_timer},
				{"pc_get_rooms", dungeon_pc_get_rooms},
				{"info_by_name", dungeon_info_by_name},
				{"get_last", dungeon_get_last},
				{"info_by_index", dungeon_info_by_index},
				{"info_by_map_index", dungeon_info_by_map_index},
				{"info_size", dungeon_info_size},
				{"new_jump", dungeon_new_jump},
				{"new_jump_party", dungeon_new_jump_party},
				{"is_registered_item", dungeon_is_registered_item},
#ifdef __DUNGEON_TEST_MODE__
				{"set_test_mode", dungeon_set_test_mode},
				{"is_test_mode", dungeon_is_test_mode},
#endif // __DUNGEON_TEST_MODE__
				{NULL, NULL},
			};

		CQuestManager::instance().AddLuaFunctionTable("d", dungeon_functions);
	}
}