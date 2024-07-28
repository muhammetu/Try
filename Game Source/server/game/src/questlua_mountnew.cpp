#include "stdafx.h"

#include "questlua.h"
#include "questmanager.h"
#include "horsename_manager.h"
#include "char.h"
#include "affect.h"
#include "config.h"
#include "utils.h"
#include "db.h"
#include "war_map.h"
#ifdef __GROWTH_MOUNT_SYSTEM__
#include "New_MountSystem.h"

#undef sys_err
#ifndef __WIN32__
#define sys_err(fmt, args...) quest::CQuestManager::instance().QuestError(__FUNCTION__, __LINE__, fmt, ##args)
#else
#define sys_err(fmt, ...) quest::CQuestManager::instance().QuestError(__FUNCTION__, __LINE__, fmt, __VA_ARGS__)
#endif

namespace quest
{
	int newmount_summon(lua_State* L)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		CNewMountSystem* mountSystem = ch->GetNewMountSystem();
		LPITEM pItem = CQuestManager::instance().GetCurrentItem();
		if (!ch || !mountSystem || !pItem)
		{
			lua_pushnumber(L, 0);
			return 1;
		}

		if (CWarMapManager::instance().IsWarMap(ch->GetMapIndex()))
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("LONCA_PET_BINEK_SAMAN"));
			return 1;
		}

		if (0 == mountSystem)
		{
			lua_pushnumber(L, 0);
			return 1;
		}

		if (ch->FindAffect(AFFECT_NEW_MOUNT_DURATION))
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "Tekrar kullanmak icin bir sure bekleyiniz.");
			return false;
		}
		ch->AddAffect(AFFECT_NEW_MOUNT_DURATION, APPLY_NONE, 0, AFF_NONE, 6, 0, false);


		DWORD mobVnum = lua_isnumber(L, 1) ? lua_tonumber(L, 1) : 0;

		const char* mountName = lua_isstring(L, 2) ? lua_tostring(L, 2) : 0;

		bool bFromFar = lua_isboolean(L, 3) ? lua_toboolean(L, 3) : false;

		CNewMountActor* mount = mountSystem->Summon(mobVnum, pItem, mountName, bFromFar);

		if (mount != NULL)
			lua_pushnumber(L, mount->GetVID());
		else
			lua_pushnumber(L, 0);

		return 1;
	}

	int newmount_unsummon(lua_State* L)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		CNewMountSystem* mountSystem = ch->GetNewMountSystem();

		if (0 == mountSystem)
			return 0;

		DWORD mobVnum = lua_isnumber(L, 1) ? lua_tonumber(L, 1) : 0;

		mountSystem->Unsummon(mobVnum);
		return 1;
	}

	int newmount_count_summoned(lua_State* L)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		CNewMountSystem* mountSystem = ch->GetNewMountSystem();

		lua_Number count = 0;

		if (0 != mountSystem)
			count = (lua_Number)mountSystem->CountSummoned();

		lua_pushnumber(L, count);

		return 1;
	}

	int newmount_is_summon(lua_State* L)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		CNewMountSystem* mountSystem = ch->GetNewMountSystem();

		if (0 == mountSystem)
			return 0;

		DWORD mobVnum = lua_isnumber(L, 1) ? lua_tonumber(L, 1) : 0;

		CNewMountActor* mountActor = mountSystem->GetByVnum(mobVnum);

		if (!(mountActor))
			lua_pushboolean(L, false);
		else
			lua_pushboolean(L, mountActor->IsSummoned());

		return 1;
	}

	int newmount_increaseskill(lua_State* L)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		CNewMountSystem* mountSystem = ch->GetNewMountSystem();

		if (0 == mountSystem)
			return 0;

		DWORD skill = lua_isnumber(L, 1) ? lua_tonumber(L, 1) : 0;

		bool mountActor = mountSystem->IncreaseMountSkill(skill);

		if (!(mountActor))
			lua_pushboolean(L, false);
		else
			lua_pushboolean(L, mountActor);
		return 1;
	}

	int newmount_resetskill(lua_State* L)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		CNewMountSystem* mountSystem = ch->GetNewMountSystem();

		if (0 == mountSystem)
			return 0;

		DWORD skill = lua_isnumber(L, 1) ? lua_tonumber(L, 1) : 0;

		bool mountActor = mountSystem->ResetSkill(skill);

		if (!(mountActor))
			lua_pushboolean(L, false);
		else
			lua_pushboolean(L, mountActor);
		return 1;
	}

	int newmount_increaseevolution(lua_State* L)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		CNewMountSystem* mountSystem = ch->GetNewMountSystem();

		if (0 == mountSystem)
			return 0;

		bool mountActor = mountSystem->IncreaseMountEvolution();

		if (!(mountActor))
			lua_pushboolean(L, false);
		else
			lua_pushboolean(L, mountActor);
		return 1;
	}

	int newmount_get_level(lua_State* L)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		CNewMountSystem* mountSystem = ch->GetNewMountSystem();

		if (0 == mountSystem) {
			lua_pushnumber(L, -1);
			return 0;
		}
		int mount_level = mountSystem->GetLevel();

		if (!(mount_level))
			lua_pushnumber(L, -1);
		else
			lua_pushnumber(L, mount_level);

		return 1;
	}

	int newmount_get_evo(lua_State* L)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		CNewMountSystem* mountSystem = ch->GetNewMountSystem();

		if (0 == mountSystem) {
			lua_pushnumber(L, -1);
			return 0;
		}
		int mount_evo = mountSystem->GetEvolution();

		if (!(mount_evo))
			lua_pushnumber(L, -1);
		else
			lua_pushnumber(L, mount_evo);

		return 1;
	}

	int newmount_restore_mount(lua_State* L)
	{
		DWORD id = lua_isnumber(L, 1) ? lua_tonumber(L, 1) : 0;
		if (id == 0) {
			lua_pushboolean(L, false);
			return 0;
		}

		char szQuery1[1024];
		snprintf(szQuery1, sizeof(szQuery1), "SELECT duration,tduration FROM new_mountsystem WHERE id = %u;", id);
		std::unique_ptr<SQLMsg> pmsg2(DBManager::instance().DirectQuery(szQuery1));
		if (pmsg2->Get()->uiNumRows > 0) {
			MYSQL_ROW row = mysql_fetch_row(pmsg2->Get()->pSQLResult);
			if (atoi(row[0]) <= 0) {
				std::unique_ptr<SQLMsg> pMsg(DBManager::instance().DirectQuery("UPDATE new_mountsystem SET duration=%d WHERE id = %u;", atoi(row[1]), id));
				lua_pushboolean(L, true);
			}
			else {
				lua_pushboolean(L, false);
			}
		}
		else {
			lua_pushboolean(L, false);
		}

		return 1;
	}

	int newmount_spawn_effect(lua_State* L)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		CNewMountSystem* mountSystem = ch->GetNewMountSystem();

		if (0 == mountSystem)
			return 0;

		DWORD mobVnum = lua_isnumber(L, 1) ? lua_tonumber(L, 1) : 0;

		CNewMountActor* mountActor = mountSystem->GetByVnum(mobVnum);
		if (NULL == mountActor)
			return 0;
		LPCHARACTER mount_ch = mountActor->GetCharacter();
		if (NULL == mount_ch)
			return 0;

		if (lua_isstring(L, 2))
		{
			mount_ch->SpecificEffectPacket(lua_tostring(L, 2));
		}
		return 0;
	}

	int newmount_eggrequest(lua_State* L)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		int evid = lua_isnumber(L, 0) ? lua_tonumber(L, 0) : 0;
		ch->SetMountEggVid(evid);
		return 1;
	}

	void RegisterNewMountFunctionTable()
	{
		luaL_reg mount_functions[] =
		{
			{ "EggRequest",		newmount_eggrequest},
			{ "summon",			newmount_summon },
			{ "unsummon",		newmount_unsummon },
			{ "is_summon",		newmount_is_summon },
			{ "count_summoned",	newmount_count_summoned },
			{ "spawn_effect",	newmount_spawn_effect },
			{ "increaseskill",	newmount_increaseskill},
			{ "increaseevo",	newmount_increaseevolution},
			{ "getlevel",		newmount_get_level },
			{ "getevo",			newmount_get_evo },
			{ "restoremount",	newmount_restore_mount},
			{ "resetskill",		newmount_resetskill},
			{ NULL,				NULL }
		};

		CQuestManager::instance().AddLuaFunctionTable("newmount", mount_functions);
	}
}
#endif