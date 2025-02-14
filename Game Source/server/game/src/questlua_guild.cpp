#include "stdafx.h"

#include "questlua.h"
#include "questmanager.h"
#include "desc_client.h"
#include "char.h"
#include "char_manager.h"
#include "utils.h"
#include "guild.h"
#include "guild_manager.h"

#undef sys_err
#ifndef __WIN32__
#define sys_err(fmt, args...) quest::CQuestManager::instance().QuestError(__FUNCTION__, __LINE__, fmt, ##args)
#else
#define sys_err(fmt, ...) quest::CQuestManager::instance().QuestError(__FUNCTION__, __LINE__, fmt, __VA_ARGS__)
#endif

namespace quest
{
	//
	// "guild" Lua functions
	//
	ALUA(guild_around_ranking_string)
	{
		CQuestManager& q = CQuestManager::instance();
		LPCHARACTER ch = q.GetCurrentCharacterPtr();
		if (!ch->GetGuild())
			lua_pushstring(L, "");
		else
		{
			char szBuf[4096 + 1];
			CGuildManager::instance().GetAroundRankString(ch->GetGuild()->GetID(), szBuf, sizeof(szBuf));
			lua_pushstring(L, szBuf);
		}
		return 1;
	}

	ALUA(guild_high_ranking_string)
	{
		CQuestManager& q = CQuestManager::instance();
		LPCHARACTER ch = q.GetCurrentCharacterPtr();
		DWORD dwMyGuild = 0;
		if (ch->GetGuild())
			dwMyGuild = ch->GetGuild()->GetID();

		char szBuf[4096 + 1];
		CGuildManager::instance().GetHighRankString(dwMyGuild, szBuf, sizeof(szBuf));
		lua_pushstring(L, szBuf);
		return 1;
	}

	ALUA(guild_get_ladder_point)
	{
		CQuestManager& q = CQuestManager::instance();
		LPCHARACTER ch = q.GetCurrentCharacterPtr();
		if (!ch->GetGuild())
		{
			lua_pushnumber(L, -1);
		}
		else
		{
			lua_pushnumber(L, ch->GetGuild()->GetLadderPoint());
		}
		return 1;
	}

	ALUA(guild_get_rank)
	{
		CQuestManager& q = CQuestManager::instance();
		LPCHARACTER ch = q.GetCurrentCharacterPtr();

		if (!ch->GetGuild())
		{
			lua_pushnumber(L, -1);
		}
		else
		{
			lua_pushnumber(L, CGuildManager::instance().GetRank(ch->GetGuild()));
		}
		return 1;
	}

	ALUA(guild_is_war)
	{
		if (!lua_isnumber(L, 1))
		{
			sys_err("invalid argument");
			return 0;
		}

		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();

		if (ch->GetGuild() && ch->GetGuild()->UnderWar((DWORD)lua_tonumber(L, 1)))
			lua_pushboolean(L, true);
		else
			lua_pushboolean(L, false);

		return 1;
	}

	ALUA(guild_name)
	{
		if (!lua_isnumber(L, 1))
		{
			sys_err("invalid argument");
			return 0;
		}

		CGuild* pkGuild = CGuildManager::instance().FindGuild((DWORD)lua_tonumber(L, 1));

		if (pkGuild)
			lua_pushstring(L, pkGuild->GetName());
		else
			lua_pushstring(L, "");

		return 1;
	}

	ALUA(guild_level)
	{
		luaL_checknumber(L, 1);

		CGuild* pkGuild = CGuildManager::instance().FindGuild((DWORD)lua_tonumber(L, 1));

		if (pkGuild)
			lua_pushnumber(L, pkGuild->GetLevel());
		else
			lua_pushnumber(L, 0);

		return 1;
	}

	ALUA(guild_war_enter)
	{
		if (!lua_isnumber(L, 1))
		{
			sys_err("invalid argument");
			return 0;
		}

		CQuestManager& q = CQuestManager::instance();
		LPCHARACTER ch = q.GetCurrentCharacterPtr();

		if (ch->GetGuild())
			ch->GetGuild()->GuildWarEntryAccept((DWORD)lua_tonumber(L, 1), ch);

		return 0;
	}

	ALUA(guild_get_any_war)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();

		if (ch->GetGuild())
			lua_pushnumber(L, ch->GetGuild()->UnderAnyWar());
		else
			lua_pushnumber(L, 0);

		return 1;
	}

	ALUA(guild_get_name)
	{
		if (!lua_isnumber(L, 1))
		{
			lua_pushstring(L, "");
			return 1;
		}

		CGuild* pkGuild = CGuildManager::instance().FindGuild((DWORD)lua_tonumber(L, 1));

		if (pkGuild)
			lua_pushstring(L, pkGuild->GetName());
		else
			lua_pushstring(L, "");

		return 1;
	}

	ALUA(guild_get_warp_war_list)
	{
		FBuildLuaGuildWarList f(L);
		CGuildManager::instance().for_each_war(f);
		return 1;
	}

	ALUA(guild_get_member_count)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();

		if (ch == NULL)
		{
			lua_pushnumber(L, 0);
			return 1;
		}

		CGuild* pGuild = ch->GetGuild();

		if (pGuild == NULL)
		{
			lua_pushnumber(L, 0);
			return 1;
		}

		lua_pushnumber(L, pGuild->GetMemberCount());

		return 1;
	}

	ALUA(guild_change_master)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();

		CGuild* pGuild = ch->GetGuild();

		if (pGuild != NULL)
		{
			if (pGuild->GetMasterPID() == ch->GetPlayerID())
			{
				if (lua_isstring(L, 1) == false)
				{
					lua_pushnumber(L, 0);
				}
				else
				{
					bool ret = pGuild->ChangeMasterTo(pGuild->GetMemberPID(lua_tostring(L, 1)));

					lua_pushnumber(L, ret == false ? 2 : 3);
				}
			}
			else
			{
				lua_pushnumber(L, 1);
			}
		}
		else
		{
			lua_pushnumber(L, 4);
		}

		return 1;
	}

	ALUA(guild_change_master_with_limit)
	{
		//

		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();

		CGuild* pGuild = ch->GetGuild();

		if (pGuild != NULL)
		{
			if (pGuild->GetMasterPID() == ch->GetPlayerID())
			{
				if (lua_isstring(L, 1) == false)
				{
					lua_pushnumber(L, 0);
				}
				else
				{
					LPCHARACTER pNewMaster = CHARACTER_MANAGER::instance().FindPC(lua_tostring(L, 1));

					if (pNewMaster != NULL)
					{
						if (pNewMaster->GetLevel() < lua_tonumber(L, 2))
						{
							lua_pushnumber(L, 6);
						}
						else
						{
							int nBeOtherLeader = pNewMaster->GetQuestFlag("change_guild_master.be_other_leader");
							CQuestManager::instance().GetPC(ch->GetPlayerID());

							if (lua_toboolean(L, 6) == true) nBeOtherLeader = 0;

							if (nBeOtherLeader > get_global_time())
							{
								lua_pushnumber(L, 7);
							}
							else
							{
								bool ret = pGuild->ChangeMasterTo(pGuild->GetMemberPID(lua_tostring(L, 1)));

								if (ret == false)
								{
									lua_pushnumber(L, 2);
								}
								else
								{
									lua_pushnumber(L, 3);

									pNewMaster->SetQuestFlag("change_guild_master.be_other_leader", 0);
									pNewMaster->SetQuestFlag("change_guild_master.be_other_member", 0);
									pNewMaster->SetQuestFlag("change_guild_master.resign_limit", (int)lua_tonumber(L, 3));

									ch->SetQuestFlag("change_guild_master.be_other_leader", (int)lua_tonumber(L, 4));
									ch->SetQuestFlag("change_guild_master.be_other_member", (int)lua_tonumber(L, 5));
									ch->SetQuestFlag("change_guild_master.resign_limit", 0);
								}
							}
						}
					}
					else
					{
						lua_pushnumber(L, 5);
					}
				}
			}
			else
			{
				lua_pushnumber(L, 1);
			}
		}
		else
		{
			lua_pushnumber(L, 4);
		}

		return 1;
	}

	ALUA(guild_get_id0)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();

		CGuild* pGuild = ch->GetGuild();

		lua_pushnumber(L, (pGuild != NULL) ? pGuild->GetID() : 0);
		return 1;
	}

	ALUA(guild_get_sp0)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();

		CGuild* pGuild = ch->GetGuild();

		lua_pushnumber(L, (pGuild != NULL) ? pGuild->GetSP() : 0);
		return 1;
	}

	ALUA(guild_get_maxsp0)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();

		CGuild* pGuild = ch->GetGuild();

		lua_pushnumber(L, (pGuild != NULL) ? pGuild->GetMaxSP() : 0);
		return 1;
	}

	ALUA(guild_get_max_member0)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();

		CGuild* pGuild = ch->GetGuild();

		lua_pushnumber(L, (pGuild != NULL) ? pGuild->GetMaxMemberCount() : 0);
		return 1;
	}

	ALUA(guild_get_total_member_level0)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();

		CGuild* pGuild = ch->GetGuild();

		lua_pushnumber(L, (pGuild != NULL) ? pGuild->GetTotalLevel() : 0);
		return 1;
	}

	ALUA(guild_get_win_count0)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();

		CGuild* pGuild = ch->GetGuild();

		lua_pushnumber(L, (pGuild != NULL) ? pGuild->GetGuildWarWinCount() : 0);
		return 1;
	}

	ALUA(guild_get_draw_count0)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();

		CGuild* pGuild = ch->GetGuild();

		lua_pushnumber(L, (pGuild != NULL) ? pGuild->GetGuildWarDrawCount() : 0);
		return 1;
	}

	ALUA(guild_get_loss_count0)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();

		CGuild* pGuild = ch->GetGuild();

		lua_pushnumber(L, (pGuild != NULL) ? pGuild->GetGuildWarLossCount() : 0);
		return 1;
	}

	ALUA(guild_add_comment0)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();

		CGuild* pGuild = ch->GetGuild();
		if (pGuild)
			pGuild->AddComment(ch, std::string(lua_tostring(L, 1)));
		return 0;
	}

	ALUA(guild_set_ladder_point0)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();

		CGuild* pGuild = ch->GetGuild();
		if (pGuild)
			pGuild->ChangeLadderPoint(lua_tonumber(L, 1));
		return 0;
	}

	// ALUA(guild_set_war_data0)
	// {
		// LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();

		// CGuild* pGuild = ch->GetGuild();
		// if (pGuild)
			// pGuild->SetWarData(lua_tonumber(L, 1), lua_tonumber(L, 2), lua_tonumber(L, 3));
		// return 0;
	// }

	ALUA(guild_get_skill_level0)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();

		CGuild* pGuild = ch->GetGuild();
		lua_pushnumber(L, (pGuild) ? pGuild->GetSkillLevel(lua_tonumber(L, 1)) : 0);
		return 1;
	}

	ALUA(guild_set_skill_level0)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();

		CGuild* pGuild = ch->GetGuild();
		if (pGuild)
			pGuild->SetSkillLevel(lua_tonumber(L, 1), lua_tonumber(L, 2), lua_isnumber(L, 3) ? lua_tonumber(L, 3) : 0);
		return 0;
	}

	ALUA(guild_get_skill_point0)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();

		CGuild* pGuild = ch->GetGuild();
		lua_pushnumber(L, (pGuild) ? pGuild->GetSkillPoint() : 0);
		return 1;
	}

	ALUA(guild_set_skill_point0)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();

		CGuild* pGuild = ch->GetGuild();
		if (pGuild)
			pGuild->SetSkillPoint(lua_tonumber(L, 1));
		return 0;
	}

	ALUA(guild_get_exp_level0)
	{
		lua_pushnumber(L, guild_exp_table2[MINMAX(0, lua_tonumber(L, 1), GUILD_MAX_LEVEL)]);
		return 1;
	}

	ALUA(guild_offer_exp0)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();

		CGuild* pGuild = ch->GetGuild();

		if (!pGuild)
		{
			lua_pushboolean(L, false);
			return 1;
		}

		DWORD offer = lua_tonumber(L, 1);

		if (pGuild->GetLevel() >= GUILD_MAX_LEVEL)
		{
			lua_pushboolean(L, false);
		}
		else
		{
			offer /= 100;
			offer *= 100;

			if (pGuild->OfferExp(ch, offer))
			{
				lua_pushboolean(L, true);
			}
			else
			{
				lua_pushboolean(L, false);
			}
		}
		return 1;
	}

	ALUA(guild_give_exp0)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();

		CGuild* pGuild = ch->GetGuild();

		if (!pGuild)
			return 0;

		pGuild->GuildPointChange(POINT_EXP, lua_tonumber(L, 1) / 100, true);
		return 0;
	}

	void RegisterGuildFunctionTable()
	{
		luaL_reg guild_functions[] =
		{
			{ "get_rank",				guild_get_rank				},
			{ "get_ladder_point",		guild_get_ladder_point		},
			{ "high_ranking_string",	guild_high_ranking_string	},
			{ "around_ranking_string",	guild_around_ranking_string	},
			{ "name",					guild_name					},
			{ "level",					guild_level					},
			{ "is_war",					guild_is_war				},
			{ "war_enter",				guild_war_enter				},
			{ "get_any_war",			guild_get_any_war			},
			{ "get_name",				guild_get_name				},
			{ "get_warp_war_list",		guild_get_warp_war_list		},
			{ "get_member_count",		guild_get_member_count		},
			{ "change_master",			guild_change_master			},
			{ "change_master_with_limit",			guild_change_master_with_limit			},
			{ "get_id0",				guild_get_id0				},	// get guild id [return lua number]
			{ "get_sp0",				guild_get_sp0				},	// get guild sp [return lua number]
			{ "get_maxsp0",				guild_get_maxsp0			},	// get guild maxsp [return lua number]
			{ "get_max_member0",		guild_get_max_member0		},	// get max joinable members [return lua number]
			{ "get_total_member_level0",	guild_get_total_member_level0	},	// get the sum of all the members' level [return lua number]
			{ "get_win_count0",			guild_get_win_count0		},	// get guild wins [return lua number]
			{ "get_draw_count0",		guild_get_draw_count0		},	// get guild draws [return lua number]
			{ "get_loss_count0",		guild_get_loss_count0		},	// get guild losses [return lua number]
			{ "add_comment0",			guild_add_comment0			},	// add a comment into guild notice board [return nothing]
			// guild.ladder_point0(point)
			{ "set_ladder_point0",		guild_set_ladder_point0		},	// set guild ladder points [return nothing]
			// guild.set_war_data0(win, draw, loss)
			// { "set_war_data0",			guild_set_war_data0			},	// set guild win/draw/loss [return nothing]
			{ "get_skill_level0",		guild_get_skill_level0		},	// get guild skill level [return lua number]
			{ "set_skill_level0",		guild_set_skill_level0		},	// set guild skill level [return nothing]
			{ "get_skill_point0",		guild_get_skill_point0		},	// get guild skill points [return lua number]
			{ "set_skill_point0",		guild_set_skill_point0		},	// set guild skill points [return nothing]
			{ "get_exp_level0",			guild_get_exp_level0		},	// get how much exp is necessary for such <level> [return lua number]
			{ "offer_exp0",				guild_offer_exp0			},	// give player's <exp> to guild [return lua boolean=successfulness]
			{ "give_exp0",				guild_give_exp0				},	// give <exp> to guild [return nothing]
			{ NULL,						NULL						}
		};

		CQuestManager::instance().AddLuaFunctionTable("guild", guild_functions);
	}
}