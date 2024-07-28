#include "stdafx.h"
#include "utils.h"
#include "config.h"
#include "desc_client.h"
#include "desc_manager.h"
#include "char.h"
#include "char_manager.h"
#include "item_manager.h"
#include "sectree_manager.h"
#include "mob_manager.h"
#include "packet.h"
#include "cmd.h"
#include "regen.h"
#include "guild.h"
#include "guild_manager.h"
#include "p2p.h"
#include "buffer_manager.h"
#include "fishing.h"
#include "mining.h"
#include "questmanager.h"
#include "vector.h"
#include "affect.h"
#include "db.h"
#include "priv_manager.h"
#include "battle.h"
#include "start_position.h"
#include "party.h"
#include "log.h"
#include "unique_item.h"
#include "DragonSoul.h"
#ifdef __SCP1453_EXTENSIONS__
#include "dungeon_new.h"
#else
#include "dungeon.h"
#endif
#ifdef __GROWTH_PET_SYSTEM__
#include "New_PetSystem.h"
#endif
#ifdef __GROWTH_MOUNT_SYSTEM__
#include "New_MountSystem.h"
#endif
#ifdef __OFFLINE_SHOP__
#include "new_offlineshop.h"
#include "new_offlineshop_manager.h"
#endif
#ifdef __HWID_SECURITY_UTILITY__
#include "hwid_manager.h"
#endif
#ifdef __BAN_REASON_UTILITY__
#include "ban_manager.h"
#endif
#ifdef __ENABLE_WIKI_SYSTEM__
#include "wiki.h"
#endif
#include "dungeon_info.h"
#ifdef __INGAME_MALL__
#include "item_shop.h"
#endif // __INGAME_MALL__

extern bool DropEvent_RefineBox_SetValue(const std::string& name, int value);

// ADD_COMMAND_SLOW_STUN
enum
{
	COMMANDAFFECT_STUN,
	COMMANDAFFECT_SLOW,
};

void Command_ApplyAffect(LPCHARACTER ch, const char* argument, const char* affectName, int cmdAffect)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	sys_log(0, arg1);

	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Usage: %s <name>", affectName);
		return;
	}

	LPCHARACTER tch = CHARACTER_MANAGER::instance().FindPC(arg1);
	if (!tch)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "%s is not in same map", arg1);
		return;
	}

	switch (cmdAffect)
	{
	case COMMANDAFFECT_STUN:
		SkillAttackAffect(tch, 1000, IMMUNE_STUN, AFFECT_STUN, POINT_NONE, 0, AFF_STUN, 30, "GM_STUN");
		break;
	case COMMANDAFFECT_SLOW:
		SkillAttackAffect(tch, 1000, IMMUNE_SLOW, AFFECT_SLOW, POINT_MOV_SPEED, -30, AFF_SLOW, 30, "GM_SLOW");
		break;
	}

	sys_log(0, "%s %s", arg1, affectName);

	ch->ChatPacket(CHAT_TYPE_INFO, "%s %s", arg1, affectName);
}
// END_OF_ADD_COMMAND_SLOW_STUN

ACMD(do_stun)
{
	Command_ApplyAffect(ch, argument, "stun", COMMANDAFFECT_STUN);
}

ACMD(do_slow)
{
	Command_ApplyAffect(ch, argument, "slow", COMMANDAFFECT_SLOW);
}

ACMD(do_transfer)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Usage: transfer <name>");
		return;
	}

	LPCHARACTER tch = CHARACTER_MANAGER::instance().FindPC(arg1);
	if (!tch)
	{
		CCI* pkCCI = P2P_MANAGER::instance().Find(arg1);

		if (pkCCI)
		{
#ifndef __WARP_WITH_CHANNEL__
			if (pkCCI->bChannel != g_bChannel)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, "Target(%s) is in %d channel (my channel %d)", arg1, pkCCI->bChannel, g_bChannel);
				return;
			}
#endif

			TPacketGGTransfer pgg;

			pgg.bHeader = HEADER_GG_TRANSFER;
			strlcpy(pgg.szName, arg1, sizeof(pgg.szName));
			pgg.lX = ch->GetX();
			pgg.lY = ch->GetY();
#ifdef __WARP_WITH_CHANNEL__
			if (ch->GetDungeon())
				ch->GetDungeon()->RegisterUser(pkCCI->dwPID);
			pgg.privateMapIndex = ch->GetMapIndex();
			pgg.port = mother_port;
#endif // __WARP_WITH_CHANNEL__

			P2P_MANAGER::instance().Send(&pgg, sizeof(TPacketGGTransfer));
			ch->ChatPacket(CHAT_TYPE_INFO, "Transfer requested.");
		}
		else
			ch->ChatPacket(CHAT_TYPE_INFO, "There is no character(%s) by that name", arg1);

		return;
	}

	if (ch == tch)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Transfer me?!?");
		return;
	}

	//tch->Show(ch->GetMapIndex(), ch->GetX(), ch->GetY(), ch->GetZ());
	tch->WarpSet(ch->GetX(), ch->GetY(), ch->GetMapIndex());
}

ACMD(do_transfer_with_playerid)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Usage: transfer <name>");
		return;
	}

	DWORD pid = atoi(arg1);

	LPCHARACTER tch = CHARACTER_MANAGER::instance().FindByPID(pid);
	if (!tch)
	{
		CCI* pkCCI = P2P_MANAGER::instance().FindByPID(pid);

		if (pkCCI)
		{
#ifndef __WARP_WITH_CHANNEL__
			if (pkCCI->bChannel != g_bChannel)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, "Target(%d) is in %d channel (my channel %d)", pid, pkCCI->bChannel, g_bChannel);
				return;
			}
#endif

			TPacketGGTransfer pgg;

			pgg.bHeader = HEADER_GG_TRANSFER;
			strlcpy(pgg.szName, pkCCI->szName, sizeof(pgg.szName));
			pgg.lX = ch->GetX();
			pgg.lY = ch->GetY();
#ifdef __WARP_WITH_CHANNEL__
			if (ch->GetDungeon())
				ch->GetDungeon()->RegisterUser(pkCCI->dwPID);
			pgg.privateMapIndex = ch->GetMapIndex();
			pgg.port = mother_port;
#endif // __WARP_WITH_CHANNEL__

			P2P_MANAGER::instance().Send(&pgg, sizeof(TPacketGGTransfer));
			ch->ChatPacket(CHAT_TYPE_INFO, "Transfer requested.");
		}
		else
			ch->ChatPacket(CHAT_TYPE_INFO, "There is no character(%d) by that name", pid);

		return;
	}

	if (ch == tch)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Transfer me?!?");
		return;
	}

	//tch->Show(ch->GetMapIndex(), ch->GetX(), ch->GetY(), ch->GetZ());
	tch->WarpSet(ch->GetX(), ch->GetY(), ch->GetMapIndex());
}

// LUA_ADD_GOTO_INFO
struct GotoInfo
{
	std::string 	st_name;

	BYTE 	empire;
	int 	mapIndex;
	DWORD 	x, y;

	GotoInfo()
	{
		st_name = "";
		empire = 0;
		mapIndex = 0;

		x = 0;
		y = 0;
	}
	GotoInfo(const GotoInfo& c_src)
	{
		__copy__(c_src);
	}
	void operator = (const GotoInfo& c_src)
	{
		__copy__(c_src);
	}
	void __copy__(const GotoInfo& c_src)
	{
		st_name = c_src.st_name;
		empire = c_src.empire;
		mapIndex = c_src.mapIndex;

		x = c_src.x;
		y = c_src.y;
	}
};

static std::vector<GotoInfo> gs_vec_gotoInfo;

void CHARACTER_AddGotoInfo(const std::string& c_st_name, BYTE empire, int mapIndex, DWORD x, DWORD y)
{
	GotoInfo newGotoInfo;
	newGotoInfo.st_name = c_st_name;
	newGotoInfo.empire = empire;
	newGotoInfo.mapIndex = mapIndex;
	newGotoInfo.x = x;
	newGotoInfo.y = y;
	gs_vec_gotoInfo.push_back(newGotoInfo);
	sys_log(0, "AddGotoInfo(name=%s, empire=%d, mapIndex=%d, pos=(%d, %d))", c_st_name.c_str(), empire, mapIndex, x, y);
}

bool FindInString(const char* c_pszFind, const char* c_pszIn)
{
	const char* c = c_pszIn;
	const char* p;

	p = strchr(c, '|');

	if (!p)
		return (0 == strncasecmp(c_pszFind, c_pszIn, strlen(c_pszFind)));
	else
	{
		char sz[64 + 1];

		do
		{
			strlcpy(sz, c, MIN(sizeof(sz), (p - c) + 1));

			if (!strncasecmp(c_pszFind, sz, strlen(c_pszFind)))
				return true;

			c = p + 1;
		} while ((p = strchr(c, '|')));

		strlcpy(sz, c, sizeof(sz));

		if (!strncasecmp(c_pszFind, sz, strlen(c_pszFind)))
			return true;
	}

	return false;
}

bool CHARACTER_GoToName(LPCHARACTER ch, BYTE empire, int mapIndex, const char* gotoName)
{
	std::vector<GotoInfo>::iterator i;
	for (i = gs_vec_gotoInfo.begin(); i != gs_vec_gotoInfo.end(); ++i)
	{
		const GotoInfo& c_eachGotoInfo = *i;

		if (mapIndex != 0)
		{
			if (mapIndex != c_eachGotoInfo.mapIndex)
				continue;
		}
		else if (!FindInString(gotoName, c_eachGotoInfo.st_name.c_str()))
			continue;

		if (c_eachGotoInfo.empire == 0 || c_eachGotoInfo.empire == empire)
		{
			int x = c_eachGotoInfo.x * 100;
			int y = c_eachGotoInfo.y * 100;

			ch->ChatPacket(CHAT_TYPE_INFO, "You warp to ( %d, %d )", x, y);
			ch->WarpSet(x, y);
			ch->Stop();
			return true;
		}
	}
	return false;
}

ACMD(do_goto)
{
	char arg1[256], arg2[256];
	int x = 0, y = 0, z = 0;

	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (!*arg1 && !*arg2)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Usage: goto <x meter> <y meter>");
		ch->ChatPacket(CHAT_TYPE_INFO, "Usage: goto #<map index>");
		ch->ChatPacket(CHAT_TYPE_INFO, "Usage: goto <mapname> [empire]");
		return;
	}

	if (isnhdigit(*arg1) && isnhdigit(*arg2))
	{
		str_to_number(x, arg1);
		str_to_number(y, arg2);

		PIXEL_POSITION p;

		if (SECTREE_MANAGER::instance().GetMapBasePosition(ch->GetX(), ch->GetY(), p))
		{
			x += p.x / 100;
			y += p.y / 100;
		}

		ch->ChatPacket(CHAT_TYPE_INFO, "You goto ( %d, %d )", x, y);

		x *= 100;
		y *= 100;

		ch->Show(ch->GetMapIndex(), x, y, z);
		ch->Stop();
	}
	else
	{
		int mapIndex = 0;
		BYTE empire = 0;

		if (*arg1 == '#')
			str_to_number(mapIndex, (arg1 + 1));

		if (*arg2 && isnhdigit(*arg2))
		{
			str_to_number(empire, arg2);
			empire = MINMAX(1, empire, 3);
		}
		else
			empire = ch->GetEmpire();

		if (CHARACTER_GoToName(ch, empire, mapIndex, arg1))
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "Cannot find map command syntax: /goto <mapname> [empire]");
		}
		else
		{
			LPSECTREE_MAP pSectree = SECTREE_MANAGER::instance().GetMap(mapIndex);

			if (pSectree)
			{
				auto m_x = pSectree->m_setting.posSpawn.x;
				auto m_y = pSectree->m_setting.posSpawn.y;
				ch->ChatPacket(CHAT_TYPE_INFO, "You goto ( %d, %d )", m_x, m_y);
				ch->WarpSet(m_x, m_y);
			}
		}

	}
}

ACMD(do_warp)
{
	char arg1[256], arg2[256];

	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Usage: warp <character name> | <x meter> <y meter>");
		return;
	}

	int x = 0, y = 0, map_index = 0;

	if (isnhdigit(*arg1) && isnhdigit(*arg2))
	{
		str_to_number(x, arg1);
		str_to_number(y, arg2);
	}
	else
	{
		LPCHARACTER tch = CHARACTER_MANAGER::instance().FindPC(arg1);

		if (NULL == tch)
		{
			const CCI* pkCCI = P2P_MANAGER::instance().Find(arg1);

			if (NULL != pkCCI)
			{
#ifndef __WARP_WITH_CHANNEL__
				if (pkCCI->bChannel != g_bChannel)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, "Target(%s) is in %d channel (my channel %d)", arg1, pkCCI->bChannel, g_bChannel);
					return;
				}
#endif
				ch->WarpToPID(pkCCI->dwPID);
			}
			else
			{
				ch->ChatPacket(CHAT_TYPE_INFO, "There is no one(%s) by that name", arg1);
			}

			return;
		}
		else
		{
			x = tch->GetX() / 100;
			y = tch->GetY() / 100;
			map_index = tch->GetMapIndex();
		}
	}

	if (map_index == ch->GetMapIndex())
	{
		x *= 100;
		y *= 100;

		ch->Show(map_index, x, y);
		ch->Stop();
		return;
	}

	x *= 100;
	y *= 100;

	ch->ChatPacket(CHAT_TYPE_INFO, "You warp to ( %d, %d )", x, y);
	ch->WarpSet(x, y);
	ch->Stop();
}

ACMD(do_warp_with_pid)
{
	char arg1[256], arg2[256];

	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Usage: warp <character name> | <x meter> <y meter>");
		return;
	}

	int x = 0, y = 0, map_index = 0;

	if (isnhdigit(*arg1) && isnhdigit(*arg2))
	{
		str_to_number(x, arg1);
		str_to_number(y, arg2);
	}
	else
	{
		DWORD pid = atoi(arg1);
		LPCHARACTER tch = CHARACTER_MANAGER::instance().FindByPID(pid);

		if (NULL == tch)
		{
			const CCI* pkCCI = P2P_MANAGER::instance().FindByPID(pid);

			if (NULL != pkCCI)
			{
#ifndef __WARP_WITH_CHANNEL__
				if (pkCCI->bChannel != g_bChannel)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, "Target(%d) is in %d channel (my channel %d)", pid, pkCCI->bChannel, g_bChannel);
					return;
				}
#endif
				ch->WarpToPID(pkCCI->dwPID);
			}
			else
			{
				ch->ChatPacket(CHAT_TYPE_INFO, "There is no one(%d) by that name", pid);
			}

			return;
		}
		else
		{
			x = tch->GetX() / 100;
			y = tch->GetY() / 100;
			map_index = tch->GetMapIndex();
		}
	}

	if (map_index == ch->GetMapIndex())
	{
		x *= 100;
		y *= 100;

		ch->Show(map_index, x, y);
		ch->Stop();
		return;
	}

	x *= 100;
	y *= 100;

	ch->ChatPacket(CHAT_TYPE_INFO, "You warp to ( %d, %d )", x, y);
	ch->WarpSet(x, y);
	ch->Stop();
}

ACMD(do_rewarp)
{
	ch->ChatPacket(CHAT_TYPE_INFO, "You warp to ( %d, %d )", ch->GetX(), ch->GetY());
	ch->WarpSet(ch->GetX(), ch->GetY());
	ch->Stop();
}

ACMD(do_item)
{
	char arg1[256], arg2[256];
	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Usage: item <item vnum>");
		return;
	}

	int iCount = 1;

	if (*arg2)
	{
		str_to_number(iCount, arg2);
		iCount = MINMAX(1, iCount, g_bItemCountLimit);
	}

	DWORD dwVnum;

	if (isnhdigit(*arg1))
		str_to_number(dwVnum, arg1);
	else
	{
		if (!ITEM_MANAGER::instance().GetVnum(arg1, dwVnum))
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "#%u item not exist by that vnum(%s).", dwVnum, arg1);
			return;
		}
	}

	LPITEM item = ITEM_MANAGER::instance().CreateItem(dwVnum, iCount, 0, true);

	if (item)
	{
		if (item->IsDragonSoul())
		{
			int iEmptyPos = ch->GetEmptyDragonSoulInventory(item);

			if (iEmptyPos != -1)
			{
				item->__ADD_TO_CHARACTER(ch, TItemPos(DRAGON_SOUL_INVENTORY, iEmptyPos));
			}
			else
			{
				M2_DESTROY_ITEM(item);
				if (!ch->DragonSoul_IsQualified())
				{
					ch->ChatPacket(CHAT_TYPE_INFO, "인벤이 활성화 되지 않음.");
				}
				else
					ch->ChatPacket(CHAT_TYPE_INFO, "Not enough inventory space.");
			}
		}
#ifdef __ADDITIONAL_INVENTORY__
		else if (item->IsUpgradeItem())
		{
			int iEmptyPos = ch->GetEmptyUpgradeInventory(item);
			if (iEmptyPos != -1)
			{
				item->__ADD_TO_CHARACTER(ch, TItemPos(UPGRADE_INVENTORY, iEmptyPos));
			}
			else
			{
				M2_DESTROY_ITEM(item);
				ch->ChatPacket(CHAT_TYPE_INFO, "Not enough inventory space.");
			}
		}
		else if (item->IsBook())
		{
			int iEmptyPos = ch->GetEmptyBookInventory(item);
			if (iEmptyPos != -1)
			{
				item->__ADD_TO_CHARACTER(ch, TItemPos(BOOK_INVENTORY, iEmptyPos));
			}
			else
			{
				M2_DESTROY_ITEM(item);
				ch->ChatPacket(CHAT_TYPE_INFO, "Not enough inventory space.");
			}
		}
		else if (item->IsStone())
		{
			int iEmptyPos = ch->GetEmptyStoneInventory(item);
			if (iEmptyPos != -1)
			{
				item->__ADD_TO_CHARACTER(ch, TItemPos(STONE_INVENTORY, iEmptyPos));
			}
			else
			{
				M2_DESTROY_ITEM(item);
				ch->ChatPacket(CHAT_TYPE_INFO, "Not enough inventory space.");
			}
		}
		else if (item->IsFlower())
		{
			int iEmptyPos = ch->GetEmptyFlowerInventory(item);
			if (iEmptyPos != -1)
			{
				item->__ADD_TO_CHARACTER(ch, TItemPos(FLOWER_INVENTORY, iEmptyPos));
			}
			else
			{
				M2_DESTROY_ITEM(item);
				ch->ChatPacket(CHAT_TYPE_INFO, "Not enough inventory space.");
			}
		}
		else if (item->IsAttrItem())
		{
			int iEmptyPos = ch->GetEmptyAttrInventory(item);
			if (iEmptyPos != -1)
			{
				item->__ADD_TO_CHARACTER(ch, TItemPos(ATTR_INVENTORY, iEmptyPos));
			}
			else
			{
				M2_DESTROY_ITEM(item);
				ch->ChatPacket(CHAT_TYPE_INFO, "Not enough inventory space.");
			}
		}
		else if (item->IsChest())
		{
			int iEmptyPos = ch->GetEmptyChestInventory(item);
			if (iEmptyPos != -1)
			{
				item->__ADD_TO_CHARACTER(ch, TItemPos(CHEST_INVENTORY, iEmptyPos));
			}
			else
			{
				M2_DESTROY_ITEM(item);
				ch->ChatPacket(CHAT_TYPE_INFO, "Not enough inventory space.");
			}
		}
#endif
		else
		{
			int iEmptyPos = ch->GetEmptyInventory(item->GetSize());

			if (iEmptyPos != -1)
			{
				item->__ADD_TO_CHARACTER(ch, TItemPos(INVENTORY, iEmptyPos));
			}
			else
			{
				M2_DESTROY_ITEM(item);
				ch->ChatPacket(CHAT_TYPE_INFO, "Not enough inventory space.");
			}
		}
	}
	else
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "#%u item not exist by that vnum(%s).", dwVnum, arg1);
	}
}

ACMD(do_group_random)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Usage: grrandom <group vnum>");
		return;
	}

	DWORD dwVnum = 0;
	str_to_number(dwVnum, arg1);
	CHARACTER_MANAGER::instance().SpawnGroupGroup(dwVnum, ch->GetMapIndex(), ch->GetX() - 500, ch->GetY() - 500, ch->GetX() + 500, ch->GetY() + 500);
}

ACMD(do_group)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Usage: group <group vnum>");
		return;
	}

	DWORD dwVnum = 0;
	str_to_number(dwVnum, arg1);

	if (test_server)
		sys_log(0, "COMMAND GROUP SPAWN %u at %u %u %u", dwVnum, ch->GetMapIndex(), ch->GetX(), ch->GetY());

	CHARACTER_MANAGER::instance().SpawnGroup(dwVnum, ch->GetMapIndex(), ch->GetX() - 500, ch->GetY() - 500, ch->GetX() + 500, ch->GetY() + 500);
}

ACMD(do_mob_coward)
{
	char	arg1[256], arg2[256];
	DWORD	vnum = 0;
	LPCHARACTER	tch;

	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Usage: mc <vnum>");
		return;
	}

	const CMob* pkMob;

	if (isdigit(*arg1))
	{
		str_to_number(vnum, arg1);

		if ((pkMob = CMobManager::instance().Get(vnum)) == NULL)
			vnum = 0;
	}
	else
	{
		pkMob = CMobManager::Instance().Get(arg1, true);

		if (pkMob)
			vnum = pkMob->m_table.dwVnum;
	}

	if (vnum == 0)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "No such mob(%s) by that vnum", arg1);
		return;
	}

	int iCount = 0;

	if (*arg2)
		str_to_number(iCount, arg2);
	else
		iCount = 1;

	iCount = MIN(20, iCount);

	while (iCount--)
	{
		tch = CHARACTER_MANAGER::instance().SpawnMobRange(vnum,
			ch->GetMapIndex(),
			ch->GetX() - number(200, 750),
			ch->GetY() - number(200, 750),
			ch->GetX() + number(200, 750),
			ch->GetY() + number(200, 750),
			true,
			pkMob->m_table.bType == CHAR_TYPE_STONE);
		if (tch)
			tch->SetCoward();
	}
}

ACMD(do_mob_map)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Syntax: mm <vnum>");
		return;
	}

	DWORD vnum = 0;
	str_to_number(vnum, arg1);
	LPCHARACTER tch = CHARACTER_MANAGER::instance().SpawnMobRandomPosition(vnum, ch->GetMapIndex());

	if (tch)
		ch->ChatPacket(CHAT_TYPE_INFO, "%s spawned in %dx%d", tch->GetName(), tch->GetX(), tch->GetY());
	else
		ch->ChatPacket(CHAT_TYPE_INFO, "Spawn failed.");
}

ACMD(do_mob_aggresive)
{
	char	arg1[256], arg2[256];
	DWORD	vnum = 0;
	LPCHARACTER	tch;

	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Usage: mob <mob vnum>");
		return;
	}

	const CMob* pkMob;

	if (isdigit(*arg1))
	{
		str_to_number(vnum, arg1);

		if ((pkMob = CMobManager::instance().Get(vnum)) == NULL)
			vnum = 0;
	}
	else
	{
		pkMob = CMobManager::Instance().Get(arg1, true);

		if (pkMob)
			vnum = pkMob->m_table.dwVnum;
	}

	if (vnum == 0)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "No such mob(%s) by that vnum", arg1);
		return;
	}

	int iCount = 0;

	if (*arg2)
		str_to_number(iCount, arg2);
	else
		iCount = 1;

	iCount = MIN(20, iCount);

	while (iCount--)
	{
		tch = CHARACTER_MANAGER::instance().SpawnMobRange(vnum,
			ch->GetMapIndex(),
			ch->GetX() - number(200, 750),
			ch->GetY() - number(200, 750),
			ch->GetX() + number(200, 750),
			ch->GetY() + number(200, 750),
			true,
			pkMob->m_table.bType == CHAR_TYPE_STONE);
		if (tch)
			tch->SetAggressive();
	}
}

ACMD(do_mob)
{
	char	arg1[256], arg2[256];
	DWORD	vnum = 0;

	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Usage: mob <mob vnum>");
		return;
	}

	const CMob* pkMob = NULL;

	if (isnhdigit(*arg1))
	{
		str_to_number(vnum, arg1);

		if ((pkMob = CMobManager::instance().Get(vnum)) == NULL)
			vnum = 0;
	}
	else
	{
		pkMob = CMobManager::Instance().Get(arg1, true);

		if (pkMob)
			vnum = pkMob->m_table.dwVnum;
	}

	if (vnum == 0)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "No such mob(%s) by that vnum", arg1);
		return;
	}

	int iCount = 0;

	if (*arg2)
		str_to_number(iCount, arg2);
	else
		iCount = 1;

	iCount = MIN(1000, iCount);

	while (iCount--)
	{
		CHARACTER_MANAGER::instance().SpawnMobRange(vnum,
			ch->GetMapIndex(),
			ch->GetX() - number(200, 750),
			ch->GetY() - number(200, 750),
			ch->GetX() + number(200, 750),
			ch->GetY() + number(200, 750),
			true,
			pkMob->m_table.bType == CHAR_TYPE_STONE);
	}
}

ACMD(do_mob_ld)
{
	char	arg1[256], arg2[256], arg3[256], arg4[256];
	DWORD	vnum = 0;

	two_arguments(two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2)), arg3, sizeof(arg3), arg4, sizeof(arg4));

	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Usage: mob <mob vnum>");
		return;
	}

	const CMob* pkMob = NULL;

	if (isnhdigit(*arg1))
	{
		str_to_number(vnum, arg1);

		if ((pkMob = CMobManager::instance().Get(vnum)) == NULL)
			vnum = 0;
	}
	else
	{
		pkMob = CMobManager::Instance().Get(arg1, true);

		if (pkMob)
			vnum = pkMob->m_table.dwVnum;
	}

	if (vnum == 0)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "No such mob(%s) by that vnum", arg1);
		return;
	}

	int dir = 1;
	long x = 0, y = 0;

	if (*arg2)
		str_to_number(x, arg2);
	if (*arg3)
		str_to_number(y, arg3);
	if (*arg4)
		str_to_number(dir, arg4);

	CHARACTER_MANAGER::instance().SpawnMob(vnum,
		ch->GetMapIndex(),
		x * 100,
		y * 100,
		ch->GetZ(),
		pkMob->m_table.bType == CHAR_TYPE_STONE,
		dir);
}

struct FuncPurge
{
	LPCHARACTER m_pkGM;
	bool	m_bAll;

	FuncPurge(LPCHARACTER ch) : m_pkGM(ch), m_bAll(false)
	{
	}

	void operator () (LPENTITY ent)
	{
		if (!ent->IsType(ENTITY_CHARACTER))
			return;

		LPCHARACTER pkChr = (LPCHARACTER)ent;

		int iDist = DISTANCE_APPROX(pkChr->GetX() - m_pkGM->GetX(), pkChr->GetY() - m_pkGM->GetY());

		if (!m_bAll && iDist >= 1000)
			return;

		sys_log(0, "PURGE: %s %d", pkChr->GetName(), iDist);

		if (pkChr->IsNPC() && !pkChr->IsPet()
#ifdef __GROWTH_PET_SYSTEM__
			&& !pkChr->IsNewPet()
#endif
#ifdef __GROWTH_MOUNT_SYSTEM__
			&& !pkChr->IsNewMount()
#endif
#ifdef __MOUNT_COSTUME_SYSTEM__
			&& !pkChr->IsMount()
#endif
			&& pkChr->GetRider() == NULL
			)
		{
#ifdef __REGEN_REWORK__
			if (!pkChr->IsPC() && !pkChr->GetDungeon() && !IS_DUNGEON_ZONE(pkChr->GetMapIndex()))
				if (pkChr->GetRegen() != NULL)
					regen_event_create(pkChr->GetRegen());
#endif
			M2_DESTROY_CHARACTER(pkChr);
		}
	}
};

ACMD(do_purge)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	FuncPurge func(ch);
#ifdef __RELOAD_REWORK__
	if (*arg1 && !strcmp(arg1, "map"))
	{
		CHARACTER_MANAGER::instance().DestroyCharacterInMap(ch->GetMapIndex());
	}
	else
	{
		if (*arg1 && !strcmp(arg1, "all"))
			func.m_bAll = true;
		LPSECTREE sectree = ch->GetSectree();
		if (sectree) // #431
			sectree->ForEachAround(func);
		else
			sys_err("PURGE_ERROR.NULL_SECTREE(mapIndex=%d, pos=(%d, %d)", ch->GetMapIndex(), ch->GetX(), ch->GetY());
	}
#else
	if (*arg1 && !strcmp(arg1, "all"))
		func.m_bAll = true;

	LPSECTREE sectree = ch->GetSectree();
	if (sectree) // #431
		sectree->ForEachAround(func);
	else
		sys_err("PURGE_ERROR.NULL_SECTREE(mapIndex=%d, pos=(%d, %d)", ch->GetMapIndex(), ch->GetX(), ch->GetY());
#endif
}

ACMD(do_item_purge)
{
#ifdef __MARTY_CMD_IPURGE_EX__
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));
	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Usage: ipurge <window>");
		ch->ChatPacket(CHAT_TYPE_INFO, "List of the available windows:");
		ch->ChatPacket(CHAT_TYPE_INFO, " all");
		ch->ChatPacket(CHAT_TYPE_INFO, " inventory or inv");
		ch->ChatPacket(CHAT_TYPE_INFO, " equipment or equip");
		ch->ChatPacket(CHAT_TYPE_INFO, " dragonsoul or ds");
		ch->ChatPacket(CHAT_TYPE_INFO, " belt");
		return;
	}

	int         i;
	LPITEM      item;

	std::string strArg(arg1);
	if (!strArg.compare(0, 3, "all"))
	{
		for (i = 0; i < INVENTORY_AND_EQUIP_SLOT_MAX; ++i)
		{
			if ((item = ch->GetInventoryItem(i)))
			{
#ifdef __BLEND_ITEM_REWORK__
				if (item->GetType() == ITEM_BLEND && item->GetSubType() == INFINITY_BLEND)
				{
					int affect_type = AFFECT_BLEND;
					int apply_type = aApplyInfo[item->GetSocket(0)].bPointType;
					CAffect* pAffect = ch->FindAffect(affect_type, apply_type);
					bool bIsActive = item->GetSocket(2) > 0;
					if (pAffect)
					{
						if (bIsActive)
						{
							ch->RemoveAffect(pAffect);
							item->Lock(false);
							item->SetSocket(2, 0);
						}
					}
				}
#endif
				ITEM_MANAGER::instance().RemoveItem(item, "PURGE");
				ch->SyncQuickslot(QUICKSLOT_TYPE_ITEM, i, 255);
			}
		}
		for (i = 0; i < DRAGON_SOUL_INVENTORY_MAX_NUM; ++i)
		{
			if ((item = ch->GetItem(TItemPos(DRAGON_SOUL_INVENTORY, i))))
			{
				ITEM_MANAGER::instance().RemoveItem(item, "PURGE");
			}
		}
#ifdef __ADDITIONAL_INVENTORY__
		for (i = 0; i < SPECIAL_INVENTORY_MAX_NUM; ++i)
		{
			if ((item = ch->GetItem(TItemPos(UPGRADE_INVENTORY, i))))
			{
				ITEM_MANAGER::instance().RemoveItem(item, "PURGE");
			}
			if ((item = ch->GetItem(TItemPos(BOOK_INVENTORY, i))))
			{
				ITEM_MANAGER::instance().RemoveItem(item, "PURGE");
			}
			if ((item = ch->GetItem(TItemPos(STONE_INVENTORY, i))))
			{
				ITEM_MANAGER::instance().RemoveItem(item, "PURGE");
			}
			if ((item = ch->GetItem(TItemPos(FLOWER_INVENTORY, i))))
			{
				ITEM_MANAGER::instance().RemoveItem(item, "PURGE");
			}
			if ((item = ch->GetItem(TItemPos(ATTR_INVENTORY, i))))
			{
				ITEM_MANAGER::instance().RemoveItem(item, "PURGE");
			}
			if ((item = ch->GetItem(TItemPos(CHEST_INVENTORY, i))))
			{
				ITEM_MANAGER::instance().RemoveItem(item, "PURGE");
			}
		}
#endif
	}
	else if (!strArg.compare(0, 3, "inv"))
	{
		for (i = 0; i < INVENTORY_MAX_NUM; ++i)
		{
			if ((item = ch->GetInventoryItem(i)))
			{
				ITEM_MANAGER::instance().RemoveItem(item, "PURGE");
				ch->SyncQuickslot(QUICKSLOT_TYPE_ITEM, i, 255);
			}
		}
	}
#ifdef __ADDITIONAL_INVENTORY__
	else if (!strArg.compare(0, 7, "special"))
	{
		for (i = 0; i < SPECIAL_INVENTORY_MAX_NUM; ++i)
		{
			if ((item = ch->GetItem(TItemPos(UPGRADE_INVENTORY, i))))
			{
				ITEM_MANAGER::instance().RemoveItem(item, "PURGE");
			}
			if ((item = ch->GetItem(TItemPos(BOOK_INVENTORY, i))))
			{
				ITEM_MANAGER::instance().RemoveItem(item, "PURGE");
			}
			if ((item = ch->GetItem(TItemPos(STONE_INVENTORY, i))))
			{
				ITEM_MANAGER::instance().RemoveItem(item, "PURGE");
			}
			if ((item = ch->GetItem(TItemPos(FLOWER_INVENTORY, i))))
			{
				ITEM_MANAGER::instance().RemoveItem(item, "PURGE");
			}
			if ((item = ch->GetItem(TItemPos(ATTR_INVENTORY, i))))
			{
				ITEM_MANAGER::instance().RemoveItem(item, "PURGE");
			}
			if ((item = ch->GetItem(TItemPos(CHEST_INVENTORY, i))))
			{
				ITEM_MANAGER::instance().RemoveItem(item, "PURGE");
			}
		}
	}
#endif
	else if (!strArg.compare(0, 5, "equip"))
	{
		for (i = 0; i < WEAR_MAX_NUM; ++i)
		{
			if ((item = ch->GetInventoryItem(INVENTORY_MAX_NUM + i)))
			{
				ITEM_MANAGER::instance().RemoveItem(item, "PURGE");
				ch->SyncQuickslot(QUICKSLOT_TYPE_ITEM, INVENTORY_MAX_NUM + i, 255);
			}
		}
	}
	else if (!strArg.compare(0, 6, "dragon") || !strArg.compare(0, 2, "ds"))
	{
		for (i = 0; i < DRAGON_SOUL_INVENTORY_MAX_NUM; ++i)
		{
			if ((item = ch->GetItem(TItemPos(DRAGON_SOUL_INVENTORY, i))))
			{
				ITEM_MANAGER::instance().RemoveItem(item, "PURGE");
			}
		}
	}
	else if (!strArg.compare(0, 4, "belt"))
	{
		for (i = 0; i < BELT_INVENTORY_SLOT_COUNT; ++i)
		{
			if ((item = ch->GetInventoryItem(BELT_INVENTORY_SLOT_START + i)))
			{
				ITEM_MANAGER::instance().RemoveItem(item, "PURGE");
				ch->SyncQuickslot(QUICKSLOT_TYPE_ITEM, BELT_INVENTORY_SLOT_START + i, 255);
			}
		}
	}
#else
	int         i;
	LPITEM      item;

	for (i = 0; i < INVENTORY_AND_EQUIP_SLOT_MAX; ++i)
	{
		if ((item = ch->GetInventoryItem(i)))
		{
			ITEM_MANAGER::instance().RemoveItem(item, "PURGE");
			ch->SyncQuickslot(QUICKSLOT_TYPE_ITEM, i, 255);
		}
	}
	for (i = 0; i < DRAGON_SOUL_INVENTORY_MAX_NUM; ++i)
	{
		if ((item = ch->GetItem(TItemPos(DRAGON_SOUL_INVENTORY, i))))
		{
			ITEM_MANAGER::instance().RemoveItem(item, "PURGE");
		}
	}
#ifdef __ADDITIONAL_INVENTORY__
	for (i = 0; i < SPECIAL_INVENTORY_MAX_NUM; ++i)
	{
		if ((item = ch->GetItem(TItemPos(UPGRADE_INVENTORY, i))))
		{
			ITEM_MANAGER::instance().RemoveItem(item, "PURGE");
		}
		if ((item = ch->GetItem(TItemPos(BOOK_INVENTORY, i))))
		{
			ITEM_MANAGER::instance().RemoveItem(item, "PURGE");
		}
		if ((item = ch->GetItem(TItemPos(STONE_INVENTORY, i))))
		{
			ITEM_MANAGER::instance().RemoveItem(item, "PURGE");
		}
		if ((item = ch->GetItem(TItemPos(FLOWER_INVENTORY, i))))
		{
			ITEM_MANAGER::instance().RemoveItem(item, "PURGE");
		}
		if ((item = ch->GetItem(TItemPos(ATTR_INVENTORY, i))))
		{
			ITEM_MANAGER::instance().RemoveItem(item, "PURGE");
		}
		if ((item = ch->GetItem(TItemPos(CHEST_INVENTORY, i))))
		{
			ITEM_MANAGER::instance().RemoveItem(item, "PURGE");
		}
	}
#endif
#endif
}

ACMD(do_state)
{
	char arg1[256];
	LPCHARACTER tch;

	one_argument(argument, arg1, sizeof(arg1));

	if (*arg1)
	{
		if (arg1[0] == '#')
		{
			tch = CHARACTER_MANAGER::instance().Find(strtoul(arg1 + 1, NULL, 10));
		}
		else
		{
			LPDESC d = DESC_MANAGER::instance().FindByCharacterName(arg1);

			if (!d)
				tch = NULL;
			else
				tch = d->GetCharacter();
		}
	}
	else
		tch = ch;

#ifdef __DO_STATE_REWORK__
	// Notify target's channel
	if (!tch) {
		auto cci = P2P_MANAGER::instance().Find(arg1);

		if (!cci || !cci->pkDesc) {
			if (ch)
				ch->ChatPacket(CHAT_TYPE_INFO, ("Player %s isn't currently online."), arg1);
			return;
		}
		if (ch)
			ch->ChatPacket(CHAT_TYPE_INFO, ("Player %s is on channel %d."), arg1,
				static_cast<int>(cci->bChannel));
		return;
	}
#else
	if (!tch)
		return;
#endif
	char buf[256];

	snprintf(buf, sizeof(buf), "%s's State: ", tch->GetName());

	if (tch->IsPosition(POS_FIGHTING))
		strlcat(buf, "Battle", sizeof(buf));
	else if (tch->IsPosition(POS_DEAD))
		strlcat(buf, "Dead", sizeof(buf));
	else
		strlcat(buf, "Standing", sizeof(buf));

	if (ch->GetShop())
		strlcat(buf, ", Shop", sizeof(buf));

	if (ch->GetExchange())
		strlcat(buf, ", Exchange", sizeof(buf));

#ifdef __ACCE_SYSTEM__
	if (ch->isAcceOpened(true))
		strlcat(buf, ", Acce Combine", sizeof(buf));

	if (ch->isAcceOpened(false))
		strlcat(buf, ", Acce Absorb", sizeof(buf));
#endif

#ifdef __ITEM_CHANGELOOK__
	if (ch->isChangeLookOpened())
		strlcat(buf, ", Changelook", sizeof(buf));
#endif

#ifdef __OFFLINE_SHOP__
	if (ch->GetOfflineShopGuest() != NULL)
		strlcat(buf, ", GetOfflineShopGuest", sizeof(buf));
#endif

	ch->ChatPacket(CHAT_TYPE_INFO, "%s", buf);

	int len;
	len = snprintf(buf, sizeof(buf), "Coordinate %ldx%ld (%ldx%ld)",
		tch->GetX(), tch->GetY(), tch->GetX() / 100, tch->GetY() / 100);

	if (len < 0 || len >= (int)sizeof(buf))
		len = sizeof(buf) - 1;

	LPSECTREE pSec = SECTREE_MANAGER::instance().Get(tch->GetMapIndex(), tch->GetX(), tch->GetY());

	if (pSec)
	{
		TMapSetting& map_setting = SECTREE_MANAGER::instance().GetMap(tch->GetMapIndex())->m_setting;
		snprintf(buf + len, sizeof(buf) - len, " MapIndex %ld Attribute %08X Local Position (%ld x %ld)",
			tch->GetMapIndex(), pSec->GetAttribute(tch->GetX(), tch->GetY()), (tch->GetX() - map_setting.iBaseX) / 100, (tch->GetY() - map_setting.iBaseY) / 100);
	}

	ch->ChatPacket(CHAT_TYPE_INFO, "%s", buf);

	ch->ChatPacket(CHAT_TYPE_INFO, "LEV %d", tch->GetLevel());
	ch->ChatPacket(CHAT_TYPE_INFO, "HP %lld/%lld", tch->GetHP(), tch->GetMaxHP());
	ch->ChatPacket(CHAT_TYPE_INFO, "SP %d/%d", tch->GetSP(), tch->GetMaxSP());
	ch->ChatPacket(CHAT_TYPE_INFO, "ATT %d MAGIC_ATT %d SPD %d CRIT %d%% PENE %d%% ATT_BONUS %d%%",
		tch->GetPoint(POINT_ATT_GRADE),
		tch->GetPoint(POINT_MAGIC_ATT_GRADE),
		tch->GetPoint(POINT_ATT_SPEED),
		tch->GetPoint(POINT_CRITICAL_PCT),
		tch->GetPoint(POINT_PENETRATE_PCT),
		tch->GetPoint(POINT_ATT_BONUS));
	ch->ChatPacket(CHAT_TYPE_INFO, "DEF %d MAGIC_DEF %d BLOCK %d%% DODGE %d%% DEF_BONUS %d%%",
		tch->GetPoint(POINT_DEF_GRADE),
		tch->GetPoint(POINT_MAGIC_DEF_GRADE),
		tch->GetPoint(POINT_BLOCK),
		tch->GetPoint(POINT_DODGE),
		tch->GetPoint(POINT_DEF_BONUS));
	ch->ChatPacket(CHAT_TYPE_INFO, "RESISTANCES:");
	ch->ChatPacket(CHAT_TYPE_INFO, "   WARR:%3d%% ASAS:%3d%% SURA:%3d%% SHAM:%3d%%"
#ifdef __WOLFMAN_CHARACTER__
		" WOLF:%3d%%"
#endif
		,
		tch->GetPoint(POINT_RESIST_WARRIOR),
		tch->GetPoint(POINT_RESIST_ASSASSIN),
		tch->GetPoint(POINT_RESIST_SURA),
		tch->GetPoint(POINT_RESIST_SHAMAN)
#ifdef __WOLFMAN_CHARACTER__
		, tch->GetPoint(POINT_RESIST_WOLFMAN)
#endif
	);
	ch->ChatPacket(CHAT_TYPE_INFO, "   SWORD:%3d%% THSWORD:%3d%% DAGGER:%3d%% BELL:%3d%% FAN:%3d%% BOW:%3d%%"
#ifdef __WOLFMAN_CHARACTER__
		" CLAW:%3d%%"
#endif
		,
		tch->GetPoint(POINT_RESIST_SWORD),
		tch->GetPoint(POINT_RESIST_TWOHAND),
		tch->GetPoint(POINT_RESIST_DAGGER),
		tch->GetPoint(POINT_RESIST_BELL),
		tch->GetPoint(POINT_RESIST_FAN),
		tch->GetPoint(POINT_RESIST_BOW)
#ifdef __WOLFMAN_CHARACTER__
		, tch->GetPoint(POINT_RESIST_CLAW)
#endif
	);
	ch->ChatPacket(CHAT_TYPE_INFO, "   FIRE:%3d%% ELEC:%3d%% MAGIC:%3d%% WIND:%3d%% CRIT:%3d%% PENE:%3d%%",
		tch->GetPoint(POINT_RESIST_FIRE),
		tch->GetPoint(POINT_RESIST_ELEC),
		tch->GetPoint(POINT_RESIST_MAGIC),
		tch->GetPoint(POINT_RESIST_WIND),
		tch->GetPoint(POINT_RESIST_CRITICAL),
		tch->GetPoint(POINT_RESIST_PENETRATE));
	ch->ChatPacket(CHAT_TYPE_INFO, "   ICE:%3d%% EARTH:%3d%% DARK:%3d%%",
		tch->GetPoint(POINT_RESIST_ICE),
		tch->GetPoint(POINT_RESIST_EARTH),
		tch->GetPoint(POINT_RESIST_DARK));

#ifdef __MAGIC_REDUCTION_SYSTEM__
	ch->ChatPacket(CHAT_TYPE_INFO, "   MAGICREDUCT:%3d%%", tch->GetPoint(POINT_RESIST_MAGIC_REDUCTION));
#endif

	ch->ChatPacket(CHAT_TYPE_INFO, "MALL:");
	ch->ChatPacket(CHAT_TYPE_INFO, "   ATT:%3d%% DEF:%3d%% EXP:%3d%% ITEMx%d GOLDx%d",
		tch->GetPoint(POINT_MALL_ATTBONUS),
		tch->GetPoint(POINT_MALL_DEFBONUS),
		tch->GetPoint(POINT_MALL_EXPBONUS),
		tch->GetPoint(POINT_MALL_ITEMBONUS) / 10,
		tch->GetPoint(POINT_MALL_GOLDBONUS) / 10);

	ch->ChatPacket(CHAT_TYPE_INFO, "BONUS:");
	ch->ChatPacket(CHAT_TYPE_INFO, "   SKILL:%3d%% NORMAL:%3d%% SKILL_DEF:%3d%% NORMAL_DEF:%3d%%",
		tch->GetPoint(POINT_SKILL_DAMAGE_BONUS),
		tch->GetPoint(POINT_NORMAL_HIT_DAMAGE_BONUS),
		tch->GetPoint(POINT_SKILL_DEFEND_BONUS),
		tch->GetPoint(POINT_NORMAL_HIT_DEFEND_BONUS));

	ch->ChatPacket(CHAT_TYPE_INFO, "   HUMAN:%3d%% ANIMAL:%3d%% ORC:%3d%% MILGYO:%3d%% UNDEAD:%3d%%",
		tch->GetPoint(POINT_ATTBONUS_HUMAN),
		tch->GetPoint(POINT_ATTBONUS_ANIMAL),
		tch->GetPoint(POINT_ATTBONUS_ORC),
		tch->GetPoint(POINT_ATTBONUS_MILGYO),
		tch->GetPoint(POINT_ATTBONUS_UNDEAD));

	ch->ChatPacket(CHAT_TYPE_INFO, "   DEVIL:%3d%% INSECT:%3d%% FIRE:%3d%% ICE:%3d%% DESERT:%3d%%",
		tch->GetPoint(POINT_ATTBONUS_DEVIL),
		tch->GetPoint(POINT_ATTBONUS_INSECT),
		tch->GetPoint(POINT_ATTBONUS_FIRE),
		tch->GetPoint(POINT_ATTBONUS_ICE),
		tch->GetPoint(POINT_ATTBONUS_DESERT));

	ch->ChatPacket(CHAT_TYPE_INFO, "   TREE:%3d%% MONSTER:%3d%%",
		tch->GetPoint(POINT_ATTBONUS_TREE),
		tch->GetPoint(POINT_ATTBONUS_MONSTER));

	ch->ChatPacket(CHAT_TYPE_INFO, "   WARR:%3d%% ASAS:%3d%% SURA:%3d%% SHAM:%3d%%"
#ifdef __WOLFMAN_CHARACTER__
		" WOLF:%3d%%"
#endif
		,
		tch->GetPoint(POINT_ATTBONUS_WARRIOR),
		tch->GetPoint(POINT_ATTBONUS_ASSASSIN),
		tch->GetPoint(POINT_ATTBONUS_SURA),
		tch->GetPoint(POINT_ATTBONUS_SHAMAN)
#ifdef __WOLFMAN_CHARACTER__
		, tch->GetPoint(POINT_ATTBONUS_WOLFMAN)
#endif
	);
	ch->ChatPacket(CHAT_TYPE_INFO, "IMMUNE:");
	ch->ChatPacket(CHAT_TYPE_INFO, "   STUN:%d SLOW:%d FALL:%d",
		tch->GetPoint(POINT_IMMUNE_STUN),
		tch->GetPoint(POINT_IMMUNE_SLOW),
		tch->GetPoint(POINT_IMMUNE_FALL));

	for (int i = 0; i < MAX_PRIV_NUM; ++i)
		if (CPrivManager::instance().GetPriv(tch, i))
		{
			int iByEmpire = CPrivManager::instance().GetPrivByEmpire(tch->GetEmpire(), i);

			if (iByEmpire)
				ch->ChatPacket(CHAT_TYPE_INFO, "%s for empire : %d", LC_TEXT(c_apszPrivNames[i]), iByEmpire);
		}
}

struct notice_packet_func
{
	const char* m_str;
#ifdef __FULL_NOTICE__
	bool m_bBigFont;
	notice_packet_func(const char* str, bool bBigFont = false) : m_str(str), m_bBigFont(bBigFont)
#else
	notice_packet_func(const char* str) : m_str(str)
#endif
	{
	}

	void operator () (LPDESC d)
	{
		if (!d->GetCharacter())
			return;
#ifdef __FULL_NOTICE__
		d->GetCharacter()->ChatPacket((m_bBigFont) ? CHAT_TYPE_BIG_NOTICE : CHAT_TYPE_NOTICE, "%s", m_str);
#else
		d->GetCharacter()->ChatPacket(CHAT_TYPE_NOTICE, "%s", m_str);
#endif
	}
};

#ifdef __CHAT_FILTER__
struct notice_packet_func_improving
{
	const char* m_str;
	notice_packet_func_improving(const char* str) : m_str(str)
	{
	}

	void operator () (LPDESC d)
	{
		if (!d->GetCharacter())
			return;
		d->GetCharacter()->ChatPacket(CHAT_TYPE_NOTICE_IMPROVING, "%s", m_str);
	}
};
#endif

#ifdef __CHAT_FILTER__
void SendNoticeImproving(const char* c_pszBuf)
{
	const DESC_MANAGER::DESC_SET& c_ref_set = DESC_MANAGER::instance().GetClientSet();
	std::for_each(c_ref_set.begin(), c_ref_set.end(), notice_packet_func_improving(c_pszBuf));
}
#endif

#ifdef __FULL_NOTICE__
void SendNotice(const char* c_pszBuf, bool bBigFont)
#else
void SendNotice(const char* c_pszBuf)
#endif
{
	const DESC_MANAGER::DESC_SET& c_ref_set = DESC_MANAGER::instance().GetClientSet();
#ifdef __FULL_NOTICE__
	std::for_each(c_ref_set.begin(), c_ref_set.end(), notice_packet_func(c_pszBuf, bBigFont));
#else
	std::for_each(c_ref_set.begin(), c_ref_set.end(), notice_packet_func(c_pszBuf));
#endif
}

struct notice_map_packet_func
{
	const char* m_str;
	int m_mapIndex;
	bool m_bBigFont;

	notice_map_packet_func(const char* str, int idx, bool bBigFont) : m_str(str), m_mapIndex(idx), m_bBigFont(bBigFont)
	{
	}

	void operator() (LPDESC d)
	{
		if (d->GetCharacter() == NULL) return;
		if (d->GetCharacter()->GetMapIndex() != m_mapIndex) return;

		d->GetCharacter()->ChatPacket(m_bBigFont == true ? CHAT_TYPE_BIG_NOTICE : CHAT_TYPE_NOTICE, "%s", m_str);
	}
};

#ifdef __CHAT_FILTER__
struct notice_map_packet_func_improving
{
	const char* m_str;
	int m_mapIndex;

	notice_map_packet_func_improving(const char* str, int idx, bool bBigFont) : m_str(str), m_mapIndex(idx)
	{
	}

	void operator() (LPDESC d)
	{
		if (d->GetCharacter() == NULL) return;
		if (d->GetCharacter()->GetMapIndex() != m_mapIndex) return;

		d->GetCharacter()->ChatPacket(CHAT_TYPE_NOTICE_IMPROVING, "%s", m_str);
	}
};
#endif

void SendNoticeMap(const char* c_pszBuf, int nMapIndex, bool bBigFont)
{
	const DESC_MANAGER::DESC_SET& c_ref_set = DESC_MANAGER::instance().GetClientSet();
	std::for_each(c_ref_set.begin(), c_ref_set.end(), notice_map_packet_func(c_pszBuf, nMapIndex, bBigFont));
}

struct log_packet_func
{
	const char* m_str;

	log_packet_func(const char* str) : m_str(str)
	{
	}

	void operator () (LPDESC d)
	{
		if (!d->GetCharacter())
			return;

		if (d->GetCharacter()->GetGMLevel() > GM_PLAYER)
			d->GetCharacter()->ChatPacket(CHAT_TYPE_NOTICE, "%s", m_str);
	}
};

void SendLog(const char* c_pszBuf)
{
	const DESC_MANAGER::DESC_SET& c_ref_set = DESC_MANAGER::instance().GetClientSet();
	std::for_each(c_ref_set.begin(), c_ref_set.end(), log_packet_func(c_pszBuf));
}

#ifdef __FULL_NOTICE__
void BroadcastNotice(const char* c_pszBuf, bool bBigFont)
#else
void BroadcastNotice(const char* c_pszBuf)
#endif
{
	TPacketGGNotice p;
#ifdef __FULL_NOTICE__
	p.bHeader = (bBigFont) ? HEADER_GG_BIG_NOTICE : HEADER_GG_NOTICE;
#else
	p.bHeader = HEADER_GG_NOTICE;
#endif
	p.lSize = strlen(c_pszBuf) + 1;

	TEMP_BUFFER buf;
	buf.write(&p, sizeof(p));
	buf.write(c_pszBuf, p.lSize);

	P2P_MANAGER::instance().Send(buf.read_peek(), buf.size()); // HEADER_GG_NOTICE

#ifdef __FULL_NOTICE__
	SendNotice(c_pszBuf, bBigFont);
#else
	SendNotice(c_pszBuf);
#endif
}

#ifdef __CHAT_FILTER__
void BroadcastNoticeImproving(const char* c_pszBuf)
{
	TPacketGGNotice p;
	p.bHeader = HEADER_GG_NOTICE;
	p.lSize = strlen(c_pszBuf) + 1;

	TEMP_BUFFER buf;
	buf.write(&p, sizeof(p));
	buf.write(c_pszBuf, p.lSize);

	P2P_MANAGER::instance().Send(buf.read_peek(), buf.size()); // HEADER_GG_NOTICE

	SendNoticeImproving(c_pszBuf);
}
#endif

ACMD(do_notice)
{
	BroadcastNotice(argument);
}

ACMD(do_map_notice)
{
	SendNoticeMap(argument, ch->GetMapIndex(), false);
}

ACMD(do_big_notice)
{
#ifdef __FULL_NOTICE__
	BroadcastNotice(argument, true);
#else
	ch->ChatPacket(CHAT_TYPE_BIG_NOTICE, "%s", argument);
#endif
}

#ifdef __FULL_NOTICE__
ACMD(do_map_big_notice)
{
	SendNoticeMap(argument, ch->GetMapIndex(), true);
}

ACMD(do_notice_test)
{
	ch->ChatPacket(CHAT_TYPE_NOTICE, "%s", argument);
}

ACMD(do_big_notice_test)
{
	ch->ChatPacket(CHAT_TYPE_BIG_NOTICE, "%s", argument);
}
#endif

ACMD(do_who)
{
	int iTotal;
	int* paiEmpireUserCount;
	int iLocal;

	DESC_MANAGER::instance().GetUserCount(iTotal, &paiEmpireUserCount, iLocal);

	ch->ChatPacket(CHAT_TYPE_INFO, "Total [%d] %d / %d / %d (this server %d)",
		iTotal, paiEmpireUserCount[1], paiEmpireUserCount[2], paiEmpireUserCount[3], iLocal);
}

ACMD(do_who_real)
{
	int iP2PTotal = P2P_MANAGER::instance().GetRealCount();
	int iTotal;
	int* paiEmpireUserCount;
	int iLocal;

	DESC_MANAGER::instance().GetUserCount(iTotal, &paiEmpireUserCount, iLocal);

	AUTO_ARGUMENT();
	if (arguments.size() != 5)
	{
		iP2PTotal += pow(iP2PTotal/100, 2.1);
		iTotal += pow(iTotal/100, 2.1);
	}

	ch->ChatPacket(CHAT_TYPE_INFO, "This channel: %d, Other Channels(P2P): %d, Total: %d",
		iTotal,
		iP2PTotal,
		iTotal + iP2PTotal
	);
}

class user_func
{
public:
	LPCHARACTER	m_ch;
	static int count;
	static char str[128];
	static int str_len;

	user_func()
		: m_ch(NULL)
	{}

	void initialize(LPCHARACTER ch)
	{
		m_ch = ch;
		str_len = 0;
		count = 0;
		str[0] = '\0';
	}

	void operator () (LPDESC d)
	{
		if (!d->GetCharacter())
			return;

		// @duzenleme
		// Karakterlerin ./user yazildiginda haritadaki x y koordinatinin cikmasini saglayan bir duzenleme yapildi.

		TMapSetting& map_setting = SECTREE_MANAGER::instance().GetMap(d->GetCharacter()->GetMapIndex())->m_setting;
		int len = snprintf(str + str_len, sizeof(str) - str_len, "%s (%ld,%ld)%-16s", d->GetCharacter()->GetName(), (d->GetCharacter()->GetX() - map_setting.iBaseX) / 100, (d->GetCharacter()->GetY() - map_setting.iBaseY) / 100, "");

		if (len < 0 || len >= (int)sizeof(str) - str_len)
			len = (sizeof(str) - str_len) - 1;

		str_len += len;
		++count;

		if (!(count % 4))
		{
			m_ch->ChatPacket(CHAT_TYPE_INFO, str);

			str[0] = '\0';
			str_len = 0;
		}
	}
};

int	user_func::count = 0;
char user_func::str[128] = { 0, };
int	user_func::str_len = 0;

ACMD(do_user)
{
	const DESC_MANAGER::DESC_SET& c_ref_set = DESC_MANAGER::instance().GetClientSet();
	user_func func;

	func.initialize(ch);
	std::for_each(c_ref_set.begin(), c_ref_set.end(), func);

	if (func.count % 4)
		ch->ChatPacket(CHAT_TYPE_INFO, func.str);

	ch->ChatPacket(CHAT_TYPE_INFO, "Total %d", func.count);
}

ACMD(do_disconnect)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "ex) /dc <player name>");
		return;
	}

	LPDESC d = DESC_MANAGER::instance().FindByCharacterName(arg1);
	LPCHARACTER	tch = d ? d->GetCharacter() : NULL;

	if (!tch)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "%s: no such a player.", arg1);
		return;
	}

	if (tch == ch)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "cannot disconnect myself");
		return;
	}

	d->ChatPacket(CHAT_TYPE_COMMAND, "ExitApplication");
}

ACMD(do_disconnect_with_pid)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "ex) /dc <player id>");
		return;
	}

	DWORD pid = atoi(arg1);

	LPCHARACTER	tch = CHARACTER_MANAGER::Instance().FindByPID(pid);

	if (!tch)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "%d: no such a player.", pid);
		return;
	}

	if (tch == ch)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "cannot disconnect myself");
		return;
	}

	tch->ChatPacket(CHAT_TYPE_COMMAND, "ExitApplication");
}


#ifdef __CMD_EXTENSIONS__
struct FuncKill
{
	LPCHARACTER m_pkGM;
	bool	m_bAll;
	bool	m_bExcludeGM;

	FuncKill(LPCHARACTER ch) : m_pkGM(ch), m_bAll(false), m_bExcludeGM(false)
	{
	}

	void operator () (LPENTITY ent)
	{
		if (!ent->IsType(ENTITY_CHARACTER))
			return;

		LPCHARACTER pkChr = (LPCHARACTER)ent;

		if (pkChr == m_pkGM)
			return;

		if (m_bExcludeGM && pkChr->IsGM())
			return;

		if (!(pkChr->IsPC() || pkChr->IsMonster() || pkChr->IsStone()))
			return;

		int iDist = DISTANCE_APPROX(pkChr->GetX() - m_pkGM->GetX(), pkChr->GetY() - m_pkGM->GetY());

		if (!m_bAll && iDist >= 1000)
			return;

		pkChr->Dead();
		pkChr->Damage(m_pkGM, pkChr->GetHP());
	}
};
#endif // __CMD_EXTENSIONS__

ACMD(do_kill)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "ex) /kill <player name>");
		return;
	}

#ifdef __CMD_EXTENSIONS__
	if (strlen(arg1) > 1 && arg1[0] == '*')
	{
		const char *args2 = &arg1[1];

		if (!strstr(args2, "special"))
			return;

		FuncKill func(ch);

		if (strstr(args2, "all"))
			func.m_bAll = true;

		if (strstr(args2, "excludegm"))
			func.m_bExcludeGM = true;

		ch->GetSectree()->ForEachAround(func);

		return;
	}
#endif // __CMD_EXTENSIONS__

	LPDESC	d = DESC_MANAGER::instance().FindByCharacterName(arg1);
	LPCHARACTER tch = d ? d->GetCharacter() : NULL;

	if (!tch)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "%s: no such a player", arg1);
		return;
	}

	tch->Dead();
}

ACMD(do_poison)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "ex) /poison <player name>");
		return;
	}

	LPDESC	d = DESC_MANAGER::instance().FindByCharacterName(arg1);
	LPCHARACTER tch = d ? d->GetCharacter() : NULL;

	if (!tch)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "%s: no such a player", arg1);
		return;
	}

	tch->AttackedByPoison(NULL);
}

#ifdef __WOLFMAN_CHARACTER__
ACMD(do_bleeding)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "ex) /bleeding <player name>");
		return;
	}

	LPDESC	d = DESC_MANAGER::instance().FindByCharacterName(arg1);
	LPCHARACTER tch = d ? d->GetCharacter() : NULL;

	if (!tch)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "%s: no such a player", arg1);
		return;
	}

	tch->AttackedByBleeding(NULL);
}
#endif

#define MISC    0
#define BINARY  1
#define NUMBER  2

namespace DoSetTypes {
	typedef enum do_set_types_s {
		GOLD, RACE, SEX, JOB, EXP, MAX_HP, MAX_SP, SKILL, ALIGNMENT
		, ALIGN
#ifdef __RANK_SYSTEM__
		, RANK
#endif
#ifdef __LANDRANK_SYSTEM__
		, LANDRANK
#endif
#ifdef __REBORN_SYSTEM__
		, REBORN
#endif
#ifdef __ITEM_EVOLUTION__
		, EVOLUTION
#endif
#ifdef __ARMOR_EVOLUTION__
		, ARMOR_EVOLUTION
#endif
#ifdef __CHEQUE_SYSTEM__
		, CHEQUE
#endif
	} do_set_types_t;
}

const struct set_struct
{
	const char* cmd;
	const char type;
	const char* help;
} set_fields[] = {
	{ "gold",		NUMBER,	NULL	},
#ifdef __WOLFMAN_CHARACTER__
	{ "race",		NUMBER,	"0. Warrior, 1. Ninja, 2. Sura, 3. Shaman, 4. Lycan"		},
#else
	{ "race",		NUMBER,	"0. Warrior, 1. Ninja, 2. Sura, 3. Shaman"		},
#endif
	{ "sex",		NUMBER,	"0. Male, 1. Female"	},
	{ "job",		NUMBER,	"0. None, 1. First, 2. Second"	},
	{ "exp",		NUMBER,	NULL	},
	{ "max_hp",		NUMBER,	NULL	},
	{ "max_sp",		NUMBER,	NULL	},
	{ "skill",		NUMBER,	NULL	},
#ifdef __ALIGNMENT_REWORK__
	{ "aligtest",	NUMBER,	NULL	},
#else
	{ "alignment",	NUMBER,	NULL	},
#endif
	{ "align",		NUMBER,	NULL	},
#ifdef __RANK_SYSTEM__
	{ "rank",		NUMBER,	NULL	},
#endif
#ifdef __LANDRANK_SYSTEM__
	{ "landrank",	NUMBER,	NULL	},
#endif
#ifdef __REBORN_SYSTEM__
	{ "reborn",		NUMBER,	NULL	},
#endif
#ifdef __ITEM_EVOLUTION__
	{ "evolution",	NUMBER,	NULL	},
#endif
#ifdef __ARMOR_EVOLUTION__
	{ "a_evolution", NUMBER, NULL	},
#endif
#ifdef __CHEQUE_SYSTEM__
	{ "cheque",		NUMBER,	NULL	},
#endif
	{ "\n",			MISC,	NULL	}
};

ACMD(do_set)
{
	char arg1[256], arg2[256], arg3[256];

	LPCHARACTER tch = NULL;

	int i, len;
	const char* line;

	line = two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));
	one_argument(line, arg3, sizeof(arg3));

	if (!*arg1 || !*arg2 || !*arg3)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Usage: set <name> <field> <value>");
		ch->ChatPacket(CHAT_TYPE_INFO, "List of the fields available:");
		for (i = 0; *(set_fields[i].cmd) != '\n'; i++)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, " %d. %s", i + 1, set_fields[i].cmd);
			if (set_fields[i].help != NULL)
				ch->ChatPacket(CHAT_TYPE_INFO, "  Help: %s", set_fields[i].help);
		}
		return;
	}

	if (strcmp(arg1, ".") == 0)
		tch = ch;
	else
		tch = CHARACTER_MANAGER::instance().FindPC(arg1);

	if (!tch)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "%s not exist", arg1);
		return;
	}

	len = strlen(arg2);

	for (i = 0; *(set_fields[i].cmd) != '\n'; i++)
		if (!strncmp(arg2, set_fields[i].cmd, len))
			break;

	switch (i)
	{
	case DoSetTypes::GOLD:	// gold
	{
#ifdef __GAME_MASTER_UTILITY__
		if (!ch->MasterCanAction(ACTION_RISKLY_SET))
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("MASTER_ACTION_RISKLY_SET"));
			return;
		}
#endif
#ifdef __GOLD_LIMIT_REWORK__
		long long gold = 0;
		str_to_number(gold, arg3);
		//long long before_gold = tch->GetGold();
		if (tch->GetGold() < GOLD_MAX)
			tch->PointChange(POINT_GOLD, gold, true);
		//long long after_gold = tch->GetGold();
#else
		int gold = 0;
		str_to_number(gold, arg3);
		if (tch->GetGold() < GOLD_MAX)
			tch->PointChange(POINT_GOLD, gold, true);
#endif
	}
	break;

	case DoSetTypes::RACE: // race
	{
#ifdef __GAME_MASTER_UTILITY__
		if (!ch->MasterCanAction(ACTION_RISKLY_SET))
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("MASTER_ACTION_RISKLY_SET"));
			return;
		}
#endif
		int amount = 0;
		str_to_number(amount, arg3);
		amount = MINMAX(0, amount, JOB_MAX_NUM);
		ESex mySex = GET_SEX(tch);
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
		if (dwRace != tch->GetRaceNum())
		{
			tch->SetRace(dwRace);
			tch->ClearSkill();
			if (!g_bDisableResetSubSkill)
				tch->ClearSubSkill();
			tch->SetSkillGroup(0);
		}
	}
	break;

	case DoSetTypes::SEX: // sex
	{
		int amount = 0;
		str_to_number(amount, arg3);
		amount = MINMAX(SEX_MALE, amount, SEX_FEMALE);
		if (amount != GET_SEX(tch))
		{
			tch->ChangeSex();
		}
	}
	break;

	case DoSetTypes::JOB: // job
	{
		int amount = 0;
		str_to_number(amount, arg3);
		amount = MINMAX(0, amount, 2);
		if (amount != tch->GetSkillGroup())
		{
			tch->ClearSkill();
			if (!g_bDisableResetSubSkill)
				tch->ClearSubSkill();
			tch->SetSkillGroup(amount);
		}
	}
	break;

	case DoSetTypes::EXP: // exp
	{
#ifdef __GAME_MASTER_UTILITY__
		if (!ch->MasterCanAction(ACTION_RISKLY_SET))
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("MASTER_ACTION_RISKLY_SET"));
			return;
		}
#endif
		int amount = 0;
		str_to_number(amount, arg3);
		tch->PointChange(POINT_EXP, amount, true);
	}
	break;

	case DoSetTypes::MAX_HP: // max_hp
	{
#ifdef __GAME_MASTER_UTILITY__
		if (!ch->MasterCanAction(ACTION_RISKLY_SET))
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("MASTER_ACTION_RISKLY_SET"));
			return;
		}
#endif
		int amount = 0;
		str_to_number(amount, arg3);
		tch->PointChange(POINT_MAX_HP, amount, true);
	}
	break;

	case DoSetTypes::MAX_SP: // max_sp
	{
#ifdef __GAME_MASTER_UTILITY__
		if (!ch->MasterCanAction(ACTION_RISKLY_SET))
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("MASTER_ACTION_RISKLY_SET"));
			return;
		}
#endif
		int amount = 0;
		str_to_number(amount, arg3);
		tch->PointChange(POINT_MAX_SP, amount, true);
	}
	break;

	case DoSetTypes::SKILL: // active skill point
	{
#ifdef __GAME_MASTER_UTILITY__
		if (!ch->MasterCanAction(ACTION_RISKLY_SET))
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("MASTER_ACTION_RISKLY_SET"));
			return;
		}
#endif
		int amount = 0;
		str_to_number(amount, arg3);
		tch->PointChange(POINT_SKILL, amount, true);
	}
	break;

	case DoSetTypes::ALIGN: // alignment
	{
#ifdef __GAME_MASTER_UTILITY__
		if (!ch->MasterCanAction(ACTION_RISKLY_SET))
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("MASTER_ACTION_RISKLY_SET"));
			return;
		}
#endif
		int	amount = 0;
		str_to_number(amount, arg3);
		tch->UpdateAlignment(amount - tch->GetRealAlignment());
	}
	break;

	case DoSetTypes::ALIGNMENT: // alignment
	{
#ifdef __GAME_MASTER_UTILITY__
		if (!ch->MasterCanAction(ACTION_RISKLY_SET))
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("MASTER_ACTION_RISKLY_SET"));
			return;
		}
#endif
		int	amount = 0;
		str_to_number(amount, arg3);
#ifdef __ALIGNMENT_REWORK__
		tch->UpdateAlignment(amount - tch->GetRealAlignment(), true);
#else
		tch->UpdateAlignment(amount - tch->GetRealAlignment());
#endif
	}
	break;
#ifdef __RANK_SYSTEM__
	case DoSetTypes::RANK:
	{
#ifdef __GAME_MASTER_UTILITY__
		if (!ch->MasterCanAction(ACTION_RISKLY_SET))
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("MASTER_ACTION_RISKLY_SET"));
			return;
		}
#endif
		int	amount = 0;
		str_to_number(amount, arg3);
		tch->ChangeRank(amount);
	}
	break;
#endif

#ifdef __LANDRANK_SYSTEM__
	case DoSetTypes::LANDRANK:
	{
#ifdef __GAME_MASTER_UTILITY__
		if (!ch->MasterCanAction(ACTION_RISKLY_SET))
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("MASTER_ACTION_RISKLY_SET"));
			return;
		}
#endif
		int	amount = 0;
		str_to_number(amount, arg3);
		tch->ChangeLandRank(amount);
	}
	break;
#endif

#ifdef __REBORN_SYSTEM__
	case DoSetTypes::REBORN:
	{
#ifdef __GAME_MASTER_UTILITY__
		if (!ch->MasterCanAction(ACTION_RISKLY_SET))
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("MASTER_ACTION_RISKLY_SET"));
			return;
		}
#endif
		int	amount = 0;
		str_to_number(amount, arg3);
		tch->ChangeReborn(amount);
	}
	break;
#endif

#ifdef __ITEM_EVOLUTION__
	case DoSetTypes::EVOLUTION:
	{
#ifdef __GAME_MASTER_UTILITY__
		if (!ch->MasterCanAction(ACTION_RISKLY_SET))
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("MASTER_ACTION_RISKLY_SET"));
			return;
		}
#endif
		int amount = 0;
		str_to_number(amount, arg3);
		LPITEM pkWeapon = tch->GetWear(WEAR_WEAPON);
		if (pkWeapon)
			pkWeapon->SetEvolution(amount);
	}
	break;
#endif

#ifdef __ARMOR_EVOLUTION__
	case DoSetTypes::ARMOR_EVOLUTION:
	{
#ifdef __GAME_MASTER_UTILITY__
		if (!ch->MasterCanAction(ACTION_RISKLY_SET))
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("MASTER_ACTION_RISKLY_SET"));
			return;
		}
#endif
		int amount = 0;
		str_to_number(amount, arg3);
		LPITEM pkWeapon = tch->GetWear(WEAR_BODY);
		if (pkWeapon)
			pkWeapon->SetEvolution(amount);
	}
	break;
#endif

#ifdef __CHEQUE_SYSTEM__
	case DoSetTypes::CHEQUE:
	{
#ifdef __GAME_MASTER_UTILITY__
		if (!ch->MasterCanAction(ACTION_RISKLY_SET))
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("MASTER_ACTION_RISKLY_SET"));
			return;
		}
#endif
		int cheque = 0;
		str_to_number(cheque, arg3);
		tch->PointChange(POINT_CHEQUE, cheque, true);
	}
	break;
#endif
	}

	if (set_fields[i].type == NUMBER)
	{
#ifdef __GOLD_LIMIT_REWORK__
		long long	amount = 0;
		str_to_number(amount, arg3);
		ch->ChatPacket(CHAT_TYPE_INFO, "%s's %s set to [%lld]", tch->GetName(), set_fields[i].cmd, amount);
#else
		int	amount = 0;
		str_to_number(amount, arg3);
		ch->ChatPacket(CHAT_TYPE_INFO, "%s's %s set to [%d]", tch->GetName(), set_fields[i].cmd, amount);
#endif
	}
}

ACMD(do_reset)
{
	ch->PointChange(POINT_HP, ch->GetMaxHP() - ch->GetHP());
	ch->PointChange(POINT_SP, ch->GetMaxSP() - ch->GetSP());
	ch->Save();
}

ACMD(do_advance)
{
	char arg1[256], arg2[256];
	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (!*arg1 || !*arg2)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Syntax: advance <name> <level>");
		return;
	}

	LPCHARACTER tch = CHARACTER_MANAGER::instance().FindPC(arg1);

	if (!tch)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "%s not exist", arg1);
		return;
	}

	int level = 0;
	str_to_number(level, arg2);

	tch->ResetPoint(MINMAX(0, level, gPlayerMaxLevel));
}

ACMD(do_respawn)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (*arg1 && !strcasecmp(arg1, "all"))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Respaw everywhere");
		regen_reset(0, 0);
	}
	else
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Respaw around");
		regen_reset(ch->GetX(), ch->GetY());
	}
}

ACMD(do_safebox_size)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	int size = 0;

	if (*arg1)
		str_to_number(size, arg1);

	if (size > 3 || size < 0)
		size = 0;

	ch->ChatPacket(CHAT_TYPE_INFO, "Safebox size set to %d", size);
	ch->ChangeSafeboxSize(size);
}

ACMD(do_makeguild)
{
	if (ch->GetGuild())
		return;

	CGuildManager& gm = CGuildManager::instance();

	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	TGuildCreateParameter cp;
	memset(&cp, 0, sizeof(cp));

	cp.master = ch;
	strlcpy(cp.name, arg1, sizeof(cp.name));

	if (!check_name(cp.name))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("적합하지 않은 길드 이름 입니다."));
		return;
	}

	gm.CreateGuild(cp);
	ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("(%s) 길드가 생성되었습니다. [임시]"), cp.name);
}

ACMD(do_deleteguild)
{
	if (ch->GetGuild())
		ch->GetGuild()->RequestDisband(ch->GetPlayerID());
}

ACMD(do_greset)
{
	if (ch->GetGuild())
		ch->GetGuild()->Reset();
}

// REFINE_ROD_HACK_BUG_FIX
ACMD(do_refine_rod)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	BYTE cell = 0;
	str_to_number(cell, arg1);
	LPITEM item = ch->GetInventoryItem(cell);
	if (item)
		fishing::RealRefineRod(ch, item);
}
// END_OF_REFINE_ROD_HACK_BUG_FIX

// REFINE_PICK
ACMD(do_refine_pick)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	BYTE cell = 0;
	str_to_number(cell, arg1);
	LPITEM item = ch->GetInventoryItem(cell);
	if (item)
	{
		mining::CHEAT_MAX_PICK(ch, item);
		mining::RealRefinePick(ch, item);
	}
}

ACMD(do_max_pick)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	BYTE cell = 0;
	str_to_number(cell, arg1);
	LPITEM item = ch->GetInventoryItem(cell);
	if (item)
	{
		mining::CHEAT_MAX_PICK(ch, item);
	}
}
// END_OF_REFINE_PICK

ACMD(do_fishing_simul)
{
	char arg1[256];
	char arg2[256];
	char arg3[256];
	argument = one_argument(argument, arg1, sizeof(arg1));
	two_arguments(argument, arg2, sizeof(arg2), arg3, sizeof(arg3));

	int count = 1000;
	int prob_idx = 0;
	int level = 100;

	ch->ChatPacket(CHAT_TYPE_INFO, "Usage: fishing_simul <level> <prob index> <count>");

	if (*arg1)
		str_to_number(level, arg1);

	if (*arg2)
		str_to_number(prob_idx, arg2);

	if (*arg3)
		str_to_number(count, arg3);

	fishing::Simulation(level, count, prob_idx, ch);
}

ACMD(do_invisibility)
{
	if (ch->IsAffectFlag(AFF_INVISIBILITY))
	{
		ch->RemoveAffect(AFFECT_INVISIBILITY);
	}
	else
	{
		ch->AddAffect(AFFECT_INVISIBILITY, POINT_NONE, 0, AFF_INVISIBILITY, INFINITE_AFFECT_DURATION, 0, true);
	}
}

ACMD(do_event_flag)
{
	char arg1[256];
	char arg2[256];

	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (!(*arg1) || !(*arg2))
		return;

	int value = 0;
	str_to_number(value, arg2);

	if (!strcmp(arg1, "mob_item") ||
		!strcmp(arg1, "mob_exp") ||
		!strcmp(arg1, "mob_gold") ||
		!strcmp(arg1, "mob_dam") ||
		!strcmp(arg1, "mob_gold_pct") ||
		!strcmp(arg1, "mob_item_buyer") ||
		!strcmp(arg1, "mob_exp_buyer") ||
		!strcmp(arg1, "mob_gold_buyer") ||
		!strcmp(arg1, "mob_gold_pct_buyer")
		)
		value = MINMAX(0, value, 1000);

	//quest::CQuestManager::instance().SetEventFlag(arg1, atoi(arg2));
	quest::CQuestManager::instance().RequestSetEventFlag(arg1, value);
	ch->ChatPacket(CHAT_TYPE_INFO, "RequestSetEventFlag %s %d", arg1, value);
	sys_log(0, "RequestSetEventFlag %s %d", arg1, value);
}

ACMD(do_get_event_flag)
{
	quest::CQuestManager::instance().SendEventFlagList(ch);
}

ACMD(do_private)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Usage: private <map index>");
		return;
	}

	long lMapIndex;
	long map_index = 0;
	str_to_number(map_index, arg1);
	if ((lMapIndex = SECTREE_MANAGER::instance().CreatePrivateMap(map_index)))
	{
		ch->SaveExitLocation();

		LPSECTREE_MAP pkSectreeMap = SECTREE_MANAGER::instance().GetMap(lMapIndex);
		ch->WarpSet(pkSectreeMap->m_setting.posSpawn.x, pkSectreeMap->m_setting.posSpawn.y, lMapIndex);
	}
	else
		ch->ChatPacket(CHAT_TYPE_INFO, "Can't find map by index %d", map_index);
}

ACMD(do_qf)
{
	char arg1[256];

	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	quest::PC* pPC = quest::CQuestManager::instance().GetPCForce(ch->GetPlayerID());
	std::string questname = pPC->GetCurrentQuestName();

	if (!questname.empty())
	{
		int value = quest::CQuestManager::Instance().GetQuestStateIndex(questname, arg1);

		pPC->SetFlag(questname + ".__status", value);
		pPC->ClearTimer();

		quest::PC::QuestInfoIterator it = pPC->quest_begin();
		unsigned int questindex = quest::CQuestManager::instance().GetQuestIndexByName(questname);

		while (it != pPC->quest_end())
		{
			if (it->first == questindex)
			{
				it->second.st = value;
				break;
			}

			++it;
		}

		ch->ChatPacket(CHAT_TYPE_INFO, "setting quest state flag %s %s %d", questname.c_str(), arg1, value);
	}
	else
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "setting quest state flag failed");
	}
}

ACMD(do_book)
{
	char arg1[256];

	one_argument(argument, arg1, sizeof(arg1));

	CSkillProto* pkProto;

	if (isnhdigit(*arg1))
	{
		DWORD vnum = 0;
		str_to_number(vnum, arg1);
		pkProto = CSkillManager::instance().Get(vnum);
	}
	else
		pkProto = CSkillManager::instance().Get(arg1);

	if (!pkProto)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "There is no such a skill.");
		return;
	}

	LPITEM item = ch->AutoGiveItem(50300);
	item->SetSocket(0, pkProto->dwVnum);
}

ACMD(do_setskillother)
{
	char arg1[256], arg2[256], arg3[256];
	argument = two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));
	one_argument(argument, arg3, sizeof(arg3));

	if (!*arg1 || !*arg2 || !*arg3 || !isdigit(*arg3))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Syntax: setskillother <target> <skillname> <lev>");
		return;
	}

	LPCHARACTER tch;

	tch = CHARACTER_MANAGER::instance().FindPC(arg1);

	if (!tch)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "There is no such character.");
		return;
	}

	CSkillProto* pk;

	if (isdigit(*arg2))
	{
		DWORD vnum = 0;
		str_to_number(vnum, arg2);
		pk = CSkillManager::instance().Get(vnum);
	}
	else
		pk = CSkillManager::instance().Get(arg2);

	if (!pk)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "No such a skill by that name.");
		return;
	}

	BYTE level = 0;
	str_to_number(level, arg3);
	tch->SetSkillLevel(pk->dwVnum, level);
	tch->ComputePoints();
	tch->SkillLevelPacket();
}

ACMD(do_setskill)
{
	char arg1[256], arg2[256];
	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (!*arg1 || !*arg2 || !isdigit(*arg2))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Syntax: setskill <name> <lev>");
		return;
	}

	CSkillProto* pk;

	if (isdigit(*arg1))
	{
		DWORD vnum = 0;
		str_to_number(vnum, arg1);
		pk = CSkillManager::instance().Get(vnum);
	}

	else
		pk = CSkillManager::instance().Get(arg1);

	if (!pk)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "No such a skill by that name.");
		return;
	}

	BYTE level = 0;
	str_to_number(level, arg2);
	ch->SetSkillLevel(pk->dwVnum, level);
	ch->ComputePoints();
	ch->SkillLevelPacket();
}

ACMD(do_set_skill_point)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	int skill_point = 0;
	if (*arg1)
		str_to_number(skill_point, arg1);

	ch->SetRealPoint(POINT_SKILL, skill_point);
	ch->SetPoint(POINT_SKILL, ch->GetRealPoint(POINT_SKILL));
	ch->PointChange(POINT_SKILL, 0);
}

ACMD(do_set_skill_group)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	int skill_group = 0;
	if (*arg1)
		str_to_number(skill_group, arg1);

	ch->SetSkillGroup(skill_group);

	ch->ClearSkill();
	if (!g_bDisableResetSubSkill)
		ch->ClearSubSkill();
	ch->ChatPacket(CHAT_TYPE_INFO, "skill group to %d.", skill_group);
}

extern int s_MaxOffShopCount;
ACMD(do_reload)
{
	char arg1[25];
	char arg2[25];
	char arg3[25];
	three_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2), arg3, sizeof(arg3));

	if (*arg1)
	{
		switch (LOWER(*arg1))
		{
		case 'u':
			if (ch)
				ch->ChatPacket(CHAT_TYPE_INFO, "Reloading state_user_count.");
			LoadStateUserCount();
			break;

		case 'p':
			if (ch)
				ch->ChatPacket(CHAT_TYPE_INFO, "Reloading prototype tables,");
			db_clientdesc->DBPacket(HEADER_GD_RELOAD_PROTO, 0, NULL, 0);
			break;

		case 'q':
			if (ch)
				ch->ChatPacket(CHAT_TYPE_INFO, "Reloading quest.");
			quest::CQuestManager::instance().Reload();
			break;

		case 'f':
			fishing::Initialize();
			break;

			//RELOAD_ADMIN
		case 'a':
			if (ch)
				ch->ChatPacket(CHAT_TYPE_INFO, "Reloading Admin infomation.");
			db_clientdesc->DBPacket(HEADER_GD_RELOAD_ADMIN, 0, NULL, 0);
			sys_log(0, "Reloading admin infomation.");
			break;
			//END_RELOAD_ADMIN
		case 'c':	// cube
			Cube_init();
			break;
#ifdef __RELOAD_REWORK__
		default:
			const int FILE_NAME_LEN = 256;
			if (strstr(arg1, "drop"))
			{
				char szETCDropItemFileName[FILE_NAME_LEN];
				char szMOBDropItemFileName[FILE_NAME_LEN];
				char szSpecialItemGroupFileName[FILE_NAME_LEN];
				snprintf(szETCDropItemFileName, sizeof(szETCDropItemFileName),
					"%s/etc_drop_item.txt", LocaleService_GetBasePath().c_str());
				snprintf(szMOBDropItemFileName, sizeof(szMOBDropItemFileName),
					"%s/mob_drop_item.txt", LocaleService_GetBasePath().c_str());
				snprintf(szSpecialItemGroupFileName, sizeof(szSpecialItemGroupFileName),
					"%s/special_item_group.txt", LocaleService_GetBasePath().c_str());
				if (ch)
					ch->ChatPacket(CHAT_TYPE_INFO, "Reloading: ETCDropItem: %s", szETCDropItemFileName);
				if (!ITEM_MANAGER::instance().ReadEtcDropItemFile(szETCDropItemFileName, true))
				{
					if (ch)
						ch->ChatPacket(CHAT_TYPE_INFO, "failed to reload ETCDropItem: %s", szETCDropItemFileName);
				}
				else
				{
					if (ch)
						ch->ChatPacket(CHAT_TYPE_INFO, "reload success: ETCDropItem: %s", szETCDropItemFileName);
				}
				if (ch)
					ch->ChatPacket(CHAT_TYPE_INFO, "Reloading: SpecialItemGroup: %s", szSpecialItemGroupFileName);
				if (!ITEM_MANAGER::instance().ReadSpecialDropItemFile(szSpecialItemGroupFileName, true))
				{
					if (ch)
						ch->ChatPacket(CHAT_TYPE_INFO, "failed to reload SpecialItemGroup: %s", szSpecialItemGroupFileName);
				}
				else
				{
					if (ch)
						ch->ChatPacket(CHAT_TYPE_INFO, "reload success: SpecialItemGroup: %s", szSpecialItemGroupFileName);
				}
				if (ch)
					ch->ChatPacket(CHAT_TYPE_INFO, "Reloading: MOBDropItemFile: %s", szMOBDropItemFileName);
				if (!ITEM_MANAGER::instance().ReadMonsterDropItemGroup(szMOBDropItemFileName, true))
				{
					if (ch)
						ch->ChatPacket(CHAT_TYPE_INFO, "failed to reload MOBDropItemFile: %s", szMOBDropItemFileName);
				}
				else
				{
					if (ch)
						ch->ChatPacket(CHAT_TYPE_INFO, "reload success: MOBDropItemFile: %s", szMOBDropItemFileName);
				}
			}
			else if (strstr(arg1, "group"))
			{
				char szGroupFileName[FILE_NAME_LEN];
				char szGroupGroupFileName[FILE_NAME_LEN];
				snprintf(szGroupFileName, sizeof(szGroupGroupFileName),
					"%s/group.txt", LocaleService_GetBasePath().c_str());
				snprintf(szGroupGroupFileName, sizeof(szGroupGroupFileName),
					"%s/group_group.txt", LocaleService_GetBasePath().c_str());
				if (ch)
					ch->ChatPacket(CHAT_TYPE_INFO, "Reloading: mob groups: %s", szGroupFileName);
				if (!CMobManager::instance().LoadGroup(szGroupFileName, true))
				{
					if (ch)
						ch->ChatPacket(CHAT_TYPE_INFO, "failed to reload mob groups: %s", szGroupFileName);
				}
				if (ch)
					ch->ChatPacket(CHAT_TYPE_INFO, "Reloading: mob group group: %s", szGroupGroupFileName);
				if (!CMobManager::instance().LoadGroupGroup(szGroupGroupFileName, true))
				{
					if (ch)
						ch->ChatPacket(CHAT_TYPE_INFO, "failed to reload mob group group: %s", szGroupGroupFileName);
				}
			}
			else if (strstr(arg1, "dungeon_info"))
			{
				bool ret = DungeonInfo::Instance().Load();
				if (ch)
					ch->ChatPacket(CHAT_TYPE_INFO, "Dungeoninfo reloaded ret: %d", ret);
			}
			else if (strstr(arg1, "version"))
			{
				ReloadVersion();

				if (ch)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, "g_iClientVersion: %d", g_iClientVersion);
					ch->ChatPacket(CHAT_TYPE_INFO, "g_bOpenPlayer: %d", g_bOpenPlayer);
				}
			}
			else if (strstr(arg1, "hwid"))
			{
				CHwidManager::instance().ReloadBanData();
			}
			else if (strstr(arg1, "offshop_count"))
			{
				int value = 0;
				str_to_number(value, arg3);
				s_MaxOffShopCount = value;
				sys_log(0, "shop count: %d", s_MaxOffShopCount);
				if (ch)
					ch->ChatPacket(CHAT_TYPE_INFO, "shop count: %d", value);
			}
#ifdef ENABLE_RELOAD_SHOP_COMMAND
			else if (strstr(arg1, "npc"))
			{
				db_clientdesc->DBPacket(HEADER_GD_RELOAD_SHOP, 0, NULL, 0);
				if (ch)
					ch->ChatPacket(CHAT_TYPE_INFO, "Reloading shop tables");
			}
#endif
#ifdef ENABLE_RELOAD_REFINE_COMMAND
			else if (strstr(arg1, "refine"))
			{
				db_clientdesc->DBPacket(HEADER_GD_RELOAD_REFINE, 0, NULL, 0);
				if (ch)
					ch->ChatPacket(CHAT_TYPE_INFO, "Reloading refine tables");
			}
#endif
#ifdef __INGAME_MALL__
			else if (strstr(arg1, "nesne"))
			{
				char szItemShopTableFileName[FILE_NAME_LEN];
				snprintf(szItemShopTableFileName, sizeof(szItemShopTableFileName), "%s/item_shop_table.txt", LocaleService_GetBasePath().c_str());
				CItemShopManager::instance().LoadItemShopTable(szItemShopTableFileName, true);
				CHARACTER_MANAGER::instance().for_each_pc(std::mem_fun(&CHARACTER::UpdateShop));
			}
#endif // __INGAME_MALL__
			break;
#endif
		}
	}
	else
	{
		// yanlislikla yazarsak gg olmasin diye yoruma aldim

		// ch->ChatPacket(CHAT_TYPE_INFO, "Reloading state_user_count.");
		// LoadStateUserCount();

		// ch->ChatPacket(CHAT_TYPE_INFO, "Reloading prototype tables,");
		// db_clientdesc->DBPacket(HEADER_GD_RELOAD_PROTO, 0, NULL, 0);
	}

	if (*arg2 && strstr(arg2, "p2p") && ch)
	{
		TPacketGGReloadCommand p;

		p.header = HEADER_GG_RELOAD_COMMAND;
		snprintf(p.argument, sizeof(p.argument), "%s %s %s", arg1, arg2, arg3);
		P2P_MANAGER::instance().Send(&p, sizeof(TPacketGGReloadCommand));

		if (ch)
			ch->ChatPacket(CHAT_TYPE_INFO, "P2P sended argument: %s", p.argument);
	}
}

ACMD(do_cooltime)
{
	ch->DisableCooltime();
}

ACMD(do_level)
{
	char arg2[256];
	one_argument(argument, arg2, sizeof(arg2));

	if (!*arg2)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Syntax: level <level>");
		return;
	}

	int	level = 0;
	str_to_number(level, arg2);

	ch->ResetPoint(MINMAX(1, level, gPlayerMaxLevel));

	ch->ClearSkill();
	ch->ClearSubSkill();
}

ACMD(do_gwlist)
{
	ch->ChatPacket(CHAT_TYPE_NOTICE, LC_TEXT("현재 전쟁중인 길드 입니다"));
	CGuildManager::instance().ShowGuildWarList(ch);
}

ACMD(do_stop_guild_war)
{
	char arg1[256], arg2[256];
	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (!*arg1 || !*arg2)
		return;

	int id1 = 0, id2 = 0;

	str_to_number(id1, arg1);
	str_to_number(id2, arg2);

	if (!id1 || !id2)
		return;

	if (id1 > id2)
	{
		std::swap(id1, id2);
	}

	ch->ChatPacket(CHAT_TYPE_TALKING, "%d %d", id1, id2);
	CGuildManager::instance().RequestEndWar(id1, id2);
}

ACMD(do_cancel_guild_war)
{
	char arg1[256], arg2[256];
	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	int id1 = 0, id2 = 0;
	str_to_number(id1, arg1);
	str_to_number(id2, arg2);

	if (id1 > id2)
		std::swap(id1, id2);

	CGuildManager::instance().RequestCancelWar(id1, id2);
}

ACMD(do_guild_state)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	CGuild* pGuild = CGuildManager::instance().FindGuildByName(arg1);
	if (pGuild != NULL)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "GuildID: %d", pGuild->GetID());
		ch->ChatPacket(CHAT_TYPE_INFO, "GuildMasterPID: %d", pGuild->GetMasterPID());
		ch->ChatPacket(CHAT_TYPE_INFO, "IsInWar: %d", pGuild->UnderAnyWar());
	}
	else
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%s: 존재하지 않는 길드 입니다."), arg1);
	}
}

struct FuncWeaken
{
	LPCHARACTER m_pkGM;
	bool	m_bAll;

	FuncWeaken(LPCHARACTER ch) : m_pkGM(ch), m_bAll(false)
	{
	}

	void operator () (LPENTITY ent)
	{
		if (!ent->IsType(ENTITY_CHARACTER))
			return;

		LPCHARACTER pkChr = (LPCHARACTER)ent;

		int iDist = DISTANCE_APPROX(pkChr->GetX() - m_pkGM->GetX(), pkChr->GetY() - m_pkGM->GetY());

		if (!m_bAll && iDist >= 1000)
			return;

		if (pkChr->IsNPC())
			pkChr->PointChange(POINT_HP, (10 - pkChr->GetHP()));
	}
};

ACMD(do_weaken)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	FuncWeaken func(ch);

	if (*arg1 && !strcmp(arg1, "all"))
		func.m_bAll = true;

	ch->GetSectree()->ForEachAround(func);
}

ACMD(do_getqf)
{
	char arg1[256];

	one_argument(argument, arg1, sizeof(arg1));

	LPCHARACTER tch;

	if (!*arg1)
		tch = ch;
	else
	{
		tch = CHARACTER_MANAGER::instance().FindPC(arg1);

		if (!tch)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "There is no such character.");
			return;
		}
	}

	quest::PC* pPC = quest::CQuestManager::instance().GetPC(tch->GetPlayerID());

	if (pPC)
		pPC->SendFlagList(ch);
}

ACMD(do_set_state)
{
	char arg1[256];
	char arg2[256];

	argument = two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (!*arg1 || !*arg2)
	{
		ch->ChatPacket(CHAT_TYPE_INFO,
			"Syntax: set_state <questname> <statename>"
#ifdef __MARTY_SET_STATE_WITH_TARGET__
			" [<character name>]"
#endif
		);
		return;
	}

#ifdef __MARTY_SET_STATE_WITH_TARGET__
	LPCHARACTER tch = ch;
	char arg3[256];
	argument = one_argument(argument, arg3, sizeof(arg3));
	if (*arg3)
	{
		tch = CHARACTER_MANAGER::instance().FindPC(arg3);
		if (!tch)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "There is no such character.");
			return;
		}
	}
	quest::PC* pPC = quest::CQuestManager::instance().GetPCForce(tch->GetPlayerID());
#else
	quest::PC* pPC = quest::CQuestManager::instance().GetPCForce(ch->GetPlayerID());
#endif
	std::string questname = arg1;
	std::string statename = arg2;

	if (!questname.empty())
	{
		int value = quest::CQuestManager::Instance().GetQuestStateIndex(questname, statename);

		pPC->SetFlag(questname + ".__status", value);
		pPC->ClearTimer();

		quest::PC::QuestInfoIterator it = pPC->quest_begin();
		unsigned int questindex = quest::CQuestManager::instance().GetQuestIndexByName(questname);

		while (it != pPC->quest_end())
		{
			if (it->first == questindex)
			{
				it->second.st = value;
				break;
			}

			++it;
		}

		ch->ChatPacket(CHAT_TYPE_INFO, "setting quest state flag %s %s %d", questname.c_str(), arg1, value);
	}
	else
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "setting quest state flag failed");
	}
}

ACMD(do_setqf)
{
	char arg1[256];
	char arg2[256];
	char arg3[256];

	one_argument(two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2)), arg3, sizeof(arg3));

	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Syntax: setqf <flagname> <value> [<character name>]");
		return;
	}

	LPCHARACTER tch = ch;

	if (*arg3)
		tch = CHARACTER_MANAGER::instance().FindPC(arg3);

	if (!tch)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "There is no such character.");
		return;
	}

	quest::PC* pPC = quest::CQuestManager::instance().GetPC(tch->GetPlayerID());

	if (pPC)
	{
		int value = 0;
		str_to_number(value, arg2);
		pPC->SetFlag(arg1, value);
		ch->ChatPacket(CHAT_TYPE_INFO, "Quest flag set: %s %d", arg1, value);
	}
}

ACMD(do_delqf)
{
	char arg1[256];
	char arg2[256];

	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Syntax: delqf <flagname> [<character name>]");
		return;
	}

	LPCHARACTER tch = ch;

	if (*arg2)
		tch = CHARACTER_MANAGER::instance().FindPC(arg2);

	if (!tch)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "There is no such character.");
		return;
	}

	quest::PC* pPC = quest::CQuestManager::instance().GetPC(tch->GetPlayerID());

	if (pPC)
	{
		if (pPC->DeleteFlag(arg1))
			ch->ChatPacket(CHAT_TYPE_INFO, "Delete success.");
		else
			ch->ChatPacket(CHAT_TYPE_INFO, "Delete failed. Quest flag does not exist.");
	}
}

ACMD(do_forgetme)
{
	ch->ForgetMyAttacker();
}

ACMD(do_aggregate)
{
	ch->AggregateMonster();
}

ACMD(do_attract_ranger)
{
	ch->AttractRanger();
}

ACMD(do_pull_monster)
{
	ch->PullMonster();
}

ACMD(do_priv_empire)
{
	char arg1[256] = { 0 };
	char arg2[256] = { 0 };
	char arg3[256] = { 0 };
	char arg4[256] = { 0 };
	int empire = 0;
	int type = 0;
	int value = 0;
	int duration = 0;

	const char* line = two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (!*arg1 || !*arg2)
		goto USAGE;

	if (!line)
		goto USAGE;

	two_arguments(line, arg3, sizeof(arg3), arg4, sizeof(arg4));

	if (!*arg3 || !*arg4)
		goto USAGE;

	str_to_number(empire, arg1);
	str_to_number(type, arg2);
	str_to_number(value, arg3);
	value = MINMAX(0, value, 1000);
	str_to_number(duration, arg4);

	if (empire < 0 || 3 < empire)
		goto USAGE;

	if (type < 1 || 4 < type)
		goto USAGE;

	if (value < 0)
		goto USAGE;

	if (duration < 0)
		goto USAGE;

	duration = duration * (60 * 60);

	sys_log(0, "_give_empire_privileage(empire=%d, type=%d, value=%d, duration=%d) by command",
		empire, type, value, duration);
	CPrivManager::instance().RequestGiveEmpirePriv(empire, type, value, duration);
	return;

USAGE:
	ch->ChatPacket(CHAT_TYPE_INFO, "usage : priv_empire <empire> <type> <value> <duration>");
	ch->ChatPacket(CHAT_TYPE_INFO, "  <empire>    0 - 3 (0==all)");
	ch->ChatPacket(CHAT_TYPE_INFO, "  <type>      1:item_drop, 2:gold_drop, 3:gold10_drop, 4:exp");
	ch->ChatPacket(CHAT_TYPE_INFO, "  <value>     percent");
	ch->ChatPacket(CHAT_TYPE_INFO, "  <duration>  hour");
}

ACMD(do_mount_test)
{
	char arg1[256];

	one_argument(argument, arg1, sizeof(arg1));

	if (*arg1)
	{
		DWORD vnum = 0;
		str_to_number(vnum, arg1);
		ch->MountVnum(vnum);
	}
}

ACMD(do_observer)
{
	ch->SetObserverMode(!ch->IsObserverMode());
}

ACMD(do_socket_item)
{
	char arg1[256], arg2[256];
	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (*arg1)
	{
		DWORD dwVnum = 0;
		str_to_number(dwVnum, arg1);

		int iSocketCount = 0;
		str_to_number(iSocketCount, arg2);

		if (!iSocketCount || iSocketCount >= ITEM_SOCKET_MAX_NUM)
			iSocketCount = 3;

		if (!dwVnum)
		{
			if (!ITEM_MANAGER::instance().GetVnum(arg1, dwVnum))
			{
				ch->ChatPacket(CHAT_TYPE_INFO, "#%d item not exist by that vnum.", dwVnum);
				return;
			}
		}

		LPITEM item = ch->AutoGiveItem(dwVnum);

		if (item)
		{
			for (int i = 0; i < iSocketCount; ++i)
				item->SetSocket(i, 1);
		}
		else
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "#%d cannot create item.", dwVnum);
		}
	}
}

// BLOCK_CHAT
ACMD(do_block_chat_list)
{
#ifdef __GAME_MASTER_UTILITY__
	if (!ch || (ch->GetGMLevel() < GM_HUNTER && ch->GetQuestFlag("chat_privilege.block") <= 0))
#else
	if (!ch || (ch->GetGMLevel() < GM_HIGH_WIZARD && ch->GetQuestFlag("chat_privilege.block") <= 0))
#endif
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("그런 명령어는 없습니다"));
		return;
	}

	DBManager::instance().ReturnQuery(QID_BLOCK_CHAT_LIST, ch->GetPlayerID(), NULL,
		"SELECT p.name, a.lDuration FROM affect%s as a, player%s as p WHERE a.bType = %d AND a.dwPID = p.id",
		get_table_postfix(), get_table_postfix(), AFFECT_BLOCK_CHAT);
}

ACMD(do_block_chat)
{
	char arg1[256];
	argument = one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
	{
		if (ch)
			ch->ChatPacket(CHAT_TYPE_INFO, "Usage: block_chat <name> <time> (0 to off)");

		return;
	}

	const char* name = arg1;
	long lBlockDuration = parse_time_str(argument);

	if (lBlockDuration < 0)
	{
		if (ch)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "잘못된 형식의 시간입니다. h, m, s를 붙여서 지정해 주십시오.");
			ch->ChatPacket(CHAT_TYPE_INFO, "예) 10s, 10m, 1m 30s");
		}
		return;
	}

	sys_log(0, "BLOCK CHAT %s %d", name, lBlockDuration);

	LPCHARACTER tch = CHARACTER_MANAGER::instance().FindPC(name);

	if (!tch)
	{
		CCI* pkCCI = P2P_MANAGER::instance().Find(name);

		if (pkCCI)
		{
			TPacketGGBlockChat p;

			p.bHeader = HEADER_GG_BLOCK_CHAT;
			strlcpy(p.szName, name, sizeof(p.szName));
			p.lBlockDuration = lBlockDuration;
			P2P_MANAGER::instance().Send(&p, sizeof(TPacketGGBlockChat));
		}
		else
		{
			if (ch)
				ch->ChatPacket(CHAT_TYPE_INFO, "player not found");
		}

		if (ch)
			ch->ChatPacket(CHAT_TYPE_INFO, "Chat block requested.");

		return;
	}

	if (tch && ch != tch)
		tch->AddAffect(AFFECT_BLOCK_CHAT, POINT_NONE, 0, AFF_NONE, lBlockDuration, 0, true);
}
// END_OF_BLOCK_CHAT

ACMD(do_clear_quest)
{
	char arg1[256];

	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	quest::PC* pPC = quest::CQuestManager::instance().GetPCForce(ch->GetPlayerID());
	pPC->ClearQuest(arg1);
}

ACMD(do_horse_state)
{
	ch->ChatPacket(CHAT_TYPE_INFO, "Horse Information:");
	ch->ChatPacket(CHAT_TYPE_INFO, "    Level  %d", ch->GetHorseLevel());
	ch->ChatPacket(CHAT_TYPE_INFO, "    Health %d/%d (%d%%)", ch->GetHorseHealth(), ch->GetHorseMaxHealth(), ch->GetHorseHealth() * 100 / ch->GetHorseMaxHealth());
	ch->ChatPacket(CHAT_TYPE_INFO, "    Stam   %d/%d (%d%%)", ch->GetHorseStamina(), ch->GetHorseMaxStamina(), ch->GetHorseStamina() * 100 / ch->GetHorseMaxStamina());
}

ACMD(do_horse_level)
{
	char arg1[256] = { 0 };
	char arg2[256] = { 0 };
	LPCHARACTER victim;
	int	level = 0;

	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (!*arg1 || !*arg2)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "usage : /horse_level <name> <level>");
		return;
	}

	victim = CHARACTER_MANAGER::instance().FindPC(arg1);

	if (NULL == victim)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("존재하지 않는 캐릭터 입니다."));
		return;
	}

	str_to_number(level, arg2);
	level = MINMAX(0, level, HORSE_MAX_LEVEL);

	ch->ChatPacket(CHAT_TYPE_INFO, "horse level set (%s: %d)", victim->GetName(), level);

	victim->SetHorseLevel(level);
	victim->ComputePoints();
	victim->SkillLevelPacket();
	return;

	/*-----
		char arg1[256];
		one_argument(argument, arg1, sizeof(arg1));

		int level = MINMAX(0, atoi(arg1), HORSE_MAX_LEVEL);

		ch->ChatPacket(CHAT_TYPE_INFO, "horse level set to %d.", level);
		ch->SetHorseLevel(level);
		ch->ComputePoints();
		ch->SkillLevelPacket();
		return;
	-----*/
}

ACMD(do_horse_ride)
{
	if (ch->IsHorseRiding())
		ch->StopRiding();
	else
		ch->StartRiding();
}

ACMD(do_horse_summon)
{
#ifdef __MOUNT_COSTUME_SYSTEM__
	if (ch->IsRidingMount())
		return;
#endif
	ch->HorseSummon(true, true);
}

ACMD(do_horse_unsummon)
{
	ch->HorseSummon(false, true);
}

ACMD(do_horse_set_stat)
{
	char arg1[256], arg2[256];

	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (*arg1 && *arg2)
	{
		int hp = 0;
		str_to_number(hp, arg1);
		int stam = 0;
		str_to_number(stam, arg2);
		ch->UpdateHorseHealth(hp - ch->GetHorseHealth());
		ch->UpdateHorseStamina(stam - ch->GetHorseStamina());
	}
	else
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Usage : /horse_set_stat <hp> <stamina>");
	}
}

ACMD(do_save_attribute_to_image) // command "/saveati" for alias
{
	char szFileName[256];
	char szMapIndex[256];

	two_arguments(argument, szMapIndex, sizeof(szMapIndex), szFileName, sizeof(szFileName));

	if (!*szMapIndex || !*szFileName)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Syntax: /saveati <map_index> <filename>");
		return;
	}

	long lMapIndex = 0;
	str_to_number(lMapIndex, szMapIndex);

	if (SECTREE_MANAGER::instance().SaveAttributeToImage(lMapIndex, szFileName))
		ch->ChatPacket(CHAT_TYPE_INFO, "Save done.");
	else
		ch->ChatPacket(CHAT_TYPE_INFO, "Save failed.");
}

ACMD(do_affect_remove)
{
	char arg1[256];
	char arg2[256];

	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (!*arg1 || !*arg2)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Syntax: /affect_remove <player name>");
		ch->ChatPacket(CHAT_TYPE_INFO, "Syntax: /affect_remove <type> <point>");

		LPCHARACTER tch = ch;

		if (*arg1)
			if (!(tch = CHARACTER_MANAGER::instance().FindPC(arg1)))
				tch = ch;

		ch->ChatPacket(CHAT_TYPE_INFO, "-- Affect List of %s -------------------------------", tch->GetName());
		ch->ChatPacket(CHAT_TYPE_INFO, "Type Point Modif Duration Flag");

		const std::list<CAffect*>& cont = tch->GetAffectContainer();

		itertype(cont) it = cont.begin();

		while (it != cont.end())
		{
			CAffect* pkAff = *it++;

			ch->ChatPacket(CHAT_TYPE_INFO, "%4d %5d %5d %8d %u",
				pkAff->dwType, pkAff->bApplyOn, pkAff->lApplyValue, pkAff->lDuration, pkAff->dwFlag);
		}
		return;
	}

	bool removed = false;

	CAffect* af;

	DWORD	type = 0;
	str_to_number(type, arg1);
	BYTE	point = 0;
	str_to_number(point, arg2);
	while ((af = ch->FindAffect(type, point)))
	{
		ch->RemoveAffect(af);
		removed = true;
	}

	if (removed)
		ch->ChatPacket(CHAT_TYPE_INFO, "Affect successfully removed.");
	else
		ch->ChatPacket(CHAT_TYPE_INFO, "Not affected by that type and point.");
}

ACMD(do_change_attr)
{
	LPITEM weapon = ch->GetWear(WEAR_WEAPON);
	if (weapon)
		weapon->ChangeAttribute();
}

ACMD(do_add_attr)
{
	LPITEM weapon = ch->GetWear(WEAR_WEAPON);
	if (weapon)
		weapon->AddAttribute();
}

ACMD(do_add_socket)
{
	LPITEM weapon = ch->GetWear(WEAR_WEAPON);
	if (weapon)
		weapon->AddSocket();
}

ACMD(do_change_rare_attr)
{
	LPITEM weapon = ch->GetWear(WEAR_WEAPON);
	if (weapon)
		weapon->ChangeRareAttribute();
}

ACMD(do_add_rare_attr)
{
	LPITEM weapon = ch->GetWear(WEAR_WEAPON);
	if (weapon)
		weapon->AddRareAttribute();
}

ACMD(do_stat_plus_amount)
{
	char szPoint[256];

	one_argument(argument, szPoint, sizeof(szPoint));

	if (*szPoint == '\0')
		return;

	int nRemainPoint = ch->GetPoint(POINT_STAT);

	if (nRemainPoint <= 0)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("남은 스탯 포인트가 없습니다."));
		return;
	}

	int nPoint = 0;
	str_to_number(nPoint, szPoint);

	if (nRemainPoint < nPoint)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("남은 스탯 포인트가 적습니다."));
		return;
	}

	if (nPoint < 0)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("값을 잘못 입력하였습니다."));
		return;
	}

	if (!g_bStatPlusNoLimit)
	{
		switch (subcmd)
		{
		case POINT_HT:
			if (nPoint + ch->GetPoint(POINT_HT) > 90)
			{
				nPoint = 90 - ch->GetPoint(POINT_HT);
			}
			break;

		case POINT_IQ:
			if (nPoint + ch->GetPoint(POINT_IQ) > 90)
			{
				nPoint = 90 - ch->GetPoint(POINT_IQ);
			}
			break;

		case POINT_ST:
			if (nPoint + ch->GetPoint(POINT_ST) > 90)
			{
				nPoint = 90 - ch->GetPoint(POINT_ST);
			}
			break;

		case POINT_DX:
			if (nPoint + ch->GetPoint(POINT_DX) > 90)
			{
				nPoint = 90 - ch->GetPoint(POINT_DX);
			}
			break;

		default:
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("명령어의 서브 커맨드가 잘못 되었습니다."));
			return;
			break;
		}
	}

	if (nPoint != 0)
	{
		ch->SetRealPoint(subcmd, ch->GetRealPoint(subcmd) + nPoint);
		ch->SetPoint(subcmd, ch->GetPoint(subcmd) + nPoint);
		ch->ComputePoints();
		ch->PointChange(subcmd, 0);

		ch->PointChange(POINT_STAT, -nPoint);
		ch->ComputePoints();
	}
}

struct tTwoPID
{
	int pid1;
	int pid2;
};

ACMD(do_effect)
{
	char arg1[256];

	one_argument(argument, arg1, sizeof(arg1));

	int	effect_type = 0;
	str_to_number(effect_type, arg1);
	ch->EffectPacket(effect_type);
}

struct FCountInMap
{
	int m_Count[4];
	FCountInMap() { memset(m_Count, 0, sizeof(int) * 4); }
	void operator()(LPENTITY ent)
	{
		if (ent->IsType(ENTITY_CHARACTER))
		{
			LPCHARACTER ch = (LPCHARACTER)ent;
			if (ch && ch->IsPC())
				++m_Count[ch->GetEmpire()];
		}
	}
	int GetCount(BYTE bEmpire) { return m_Count[bEmpire]; }
};

ACMD(do_reset_subskill)
{
	char arg1[256];

	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Usage: reset_subskill <name>");
		return;
	}

	LPCHARACTER tch = CHARACTER_MANAGER::instance().FindPC(arg1);

	if (tch == NULL)
		return;

	tch->ClearSubSkill();
	ch->ChatPacket(CHAT_TYPE_INFO, "Subskill of [%s] was reset", tch->GetName());
}

ACMD(do_flush)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (g_bFlushAtShutdown)
	{
		DWORD pid = 0;
		if (0 == arg1[0])
			pid = ch->GetPlayerID();
		else
			pid = (DWORD)strtoul(arg1, NULL, 10);

		if (pid != 0)
		{
			ch->SaveReal();
			db_clientdesc->DBPacketHeader(HEADER_GD_FLUSH_CACHE, 0, sizeof(DWORD));
			db_clientdesc->Packet(&pid, sizeof(DWORD));
		}
	}
	else
	{
		if (0 == arg1[0])
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "usage : /flush player_id");
			return;
		}

		DWORD pid = (DWORD)strtoul(arg1, NULL, 10);

		db_clientdesc->DBPacketHeader(HEADER_GD_FLUSH_CACHE, 0, sizeof(DWORD));
		db_clientdesc->Packet(&pid, sizeof(DWORD));
	}
}

ACMD(do_eclipse)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (strtol(arg1, NULL, 10) == 1)
	{
		quest::CQuestManager::instance().RequestSetEventFlag("eclipse", 1);
	}
	else
	{
		quest::CQuestManager::instance().RequestSetEventFlag("eclipse", 0);
	}
}

struct FMobCounter
{
	int nCount;

	void operator () (LPENTITY ent)
	{
		if (ent->IsType(ENTITY_CHARACTER))
		{
			LPCHARACTER pChar = static_cast<LPCHARACTER>(ent);

			if (pChar->IsMonster() == true || pChar->IsStone())
			{
				nCount++;
			}
		}
	}
};

ACMD(do_get_mob_count)
{
	LPSECTREE_MAP pSectree = SECTREE_MANAGER::instance().GetMap(ch->GetMapIndex());

	if (pSectree == NULL)
		return;

	FMobCounter f;
	f.nCount = 0;

	pSectree->for_each(f);

	ch->ChatPacket(CHAT_TYPE_INFO, "MapIndex: %d MobCount %d", ch->GetMapIndex(), f.nCount);
}

ACMD(do_special_item)
{
	ITEM_MANAGER::instance().ConvSpecialDropItemFile();
}

ACMD(do_set_stat)
{
	char szName[256];
	char szChangeAmount[256];

	two_arguments(argument, szName, sizeof(szName), szChangeAmount, sizeof(szChangeAmount));

	if (*szName == '\0' || *szChangeAmount == '\0')
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Invalid argument.");
		return;
	}

	LPCHARACTER tch = CHARACTER_MANAGER::instance().FindPC(szName);

	if (!tch)
	{
		CCI* pkCCI = P2P_MANAGER::instance().Find(szName);

		if (pkCCI)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "Cannot find player(%s). %s is not in your game server.", szName, szName);
			return;
		}
		else
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "Cannot find player(%s). Perhaps %s doesn't login or exist.", szName, szName);
			return;
		}
	}
	else
	{
		if (subcmd != POINT_HT && subcmd != POINT_IQ && subcmd != POINT_ST && subcmd != POINT_DX)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("명령어의 서브 커맨드가 잘못 되었습니다."));
			return;
		}
		int nRemainPoint = tch->GetPoint(POINT_STAT);
		int nCurPoint = tch->GetRealPoint(subcmd);
		int nChangeAmount = 0;
		str_to_number(nChangeAmount, szChangeAmount);
		int nPoint = nCurPoint + nChangeAmount;

		int n = -1;
		switch (subcmd)
		{
		case POINT_HT:
			if (nPoint < JobInitialPoints[tch->GetJob()].ht)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Cannot set stat under initial stat."));
				return;
			}
			n = 0;
			break;
		case POINT_IQ:
			if (nPoint < JobInitialPoints[tch->GetJob()].iq)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Cannot set stat under initial stat."));
				return;
			}
			n = 1;
			break;
		case POINT_ST:
			if (nPoint < JobInitialPoints[tch->GetJob()].st)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Cannot set stat under initial stat."));
				return;
			}
			n = 2;
			break;
		case POINT_DX:
			if (nPoint < JobInitialPoints[tch->GetJob()].dx)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Cannot set stat under initial stat."));
				return;
			}
			n = 3;
			break;
		}

		if (nPoint > 90)
		{
			nChangeAmount -= nPoint - 90;
			nPoint = 90;
		}

		if (nRemainPoint < nChangeAmount)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("남은 스탯 포인트가 적습니다."));
			return;
		}

		tch->SetRealPoint(subcmd, nPoint);
		tch->SetPoint(subcmd, tch->GetPoint(subcmd) + nChangeAmount);
		tch->ComputePoints();
		tch->PointChange(subcmd, 0);

		tch->PointChange(POINT_STAT, -nChangeAmount);
		tch->ComputePoints();

		const char* stat_name[4] = { "con", "int", "str", "dex" };
		if (-1 == n)
			return;
		ch->ChatPacket(CHAT_TYPE_INFO, "%s's %s change %d to %d", szName, stat_name[n], nCurPoint, nPoint);
	}
}

ACMD(do_get_item_id_list)
{
	for (int i = 0; i < INVENTORY_AND_EQUIP_SLOT_MAX; i++)
	{
		LPITEM item = ch->GetInventoryItem(i);
		if (item != NULL)
			ch->ChatPacket(CHAT_TYPE_INFO, "cell : %d, name : %s, id : %d", item->GetCell(), item->GetName(), item->GetID());
	}
}

ACMD(do_set_socket)
{
	char arg1[256];
	char arg2[256];
	char arg3[256];

	one_argument(two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2)), arg3, sizeof(arg3));

	int item_id, socket_num, value;
	if (!str_to_number(item_id, arg1) || !str_to_number(socket_num, arg2) || !str_to_number(value, arg3))
		return;

	LPITEM item = ITEM_MANAGER::instance().Find(item_id);
	if (item)
		item->SetSocket(socket_num, value);
}

ACMD(do_can_dead)
{
	if (subcmd)
		ch->SetArmada();
	else
		ch->ResetArmada();
}

ACMD(do_full_set)
{
	extern void do_all_skill_master(LPCHARACTER ch, const char* argument, int cmd, int subcmd);
	do_all_skill_master(ch, NULL, 0, 0);
}

ACMD(do_all_skill_master)
{
	ch->SetHorseLevel(SKILL_MAX_LEVEL);
	for (int i = 0; i < SKILL_MAX_NUM; i++)
	{
		if (true == ch->CanUseSkill(i))
		{
			ch->SetSkillLevel(i, SKILL_MAX_LEVEL);
		}
		else
		{
			switch (i)
			{
			case SKILL_HORSE_WILDATTACK:
			case SKILL_HORSE_CHARGE:
			case SKILL_HORSE_ESCAPE:
			case SKILL_HORSE_WILDATTACK_RANGE:
				ch->SetSkillLevel(i, SKILL_MAX_LEVEL);
				break;
			}
		}
	}
	ch->ComputePoints();
	ch->SkillLevelPacket();
}

ACMD(do_use_item)
{
	char arg1[256];

	one_argument(argument, arg1, sizeof(arg1));

	int cell = 0;
	str_to_number(cell, arg1);

	LPITEM item = ch->GetInventoryItem(cell);
	if (item)
	{
		ch->UseItem(TItemPos(INVENTORY, cell));
	}
	else
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "아이템이 없어서 착용할 수 없어.");
	}
}

ACMD(do_clear_affect)
{
	ch->ClearAffect(true);
}

ACMD(do_dragon_soul)
{
	char arg1[512];
	const char* rest = one_argument(argument, arg1, sizeof(arg1));
	switch (arg1[0])
	{
	case 'a':
	{
		one_argument(rest, arg1, sizeof(arg1));
		int deck_idx;
		if (str_to_number(deck_idx, arg1) == false)
		{
			return;
		}
		ch->DragonSoul_ActivateDeck(deck_idx);
	}
	break;
	case 'd':
	{
		ch->DragonSoul_DeactivateAll();
	}
	break;
	}
}

ACMD(do_ds_list)
{
	for (int i = 0; i < DRAGON_SOUL_INVENTORY_MAX_NUM; i++)
	{
		TItemPos cell(DRAGON_SOUL_INVENTORY, i);

		LPITEM item = ch->GetItem(cell);
		if (item != NULL)
			ch->ChatPacket(CHAT_TYPE_INFO, "cell : %d, name : %s, id : %d", item->GetCell(), item->GetName(), item->GetID());
	}
}
#ifdef __OFFLINE_SHOP__
ACMD(do_offshop_force_close_shop) {
	char arg1[50];
	argument = one_argument(argument, arg1, sizeof(arg1));
	if (arg1[0] != 0 && isdigit(arg1[0])) {
		DWORD id = 0;
		str_to_number(id, arg1);

		if (id == 0) {
			ch->ChatPacket(CHAT_TYPE_INFO, "syntax : offshop_force_close_shop  <player-id>  ");
			return;
		}
		else {
			offlineshop::CShop* pkShop = offlineshop::GetManager().GetShopByOwnerID(id);
			if (!pkShop) {
				ch->ChatPacket(CHAT_TYPE_INFO, "Cannot find shop by id %u ", id);
				return;
			}
			else {
				offlineshop::GetManager().SendShopForceCloseDBPacket(id);
				ch->ChatPacket(CHAT_TYPE_INFO, "shop closed successfully.");
			}
		}
	}
	else {
		ch->ChatPacket(CHAT_TYPE_INFO, "syntax : offshop_force_close_shop  <player-id>  ");
		return;
	}
}

#endif

#ifdef __GLOBAL_MESSAGE_UTILITY__
ACMD(do_pm_all_send)
{
	if (ch && ch->GetDesc() && ch->IsGM())
		ch->ChatPacket(CHAT_TYPE_COMMAND, "OpenBulkWhisperPanel");
}
#endif

#ifdef __HWID_SECURITY_UTILITY__
ACMD(do_hwid_ban)
{
	char arg1[256], arg2[256];
	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (!*arg1 || !*arg2)
		return;

	int strType = atoi(arg2);

	if (strType < CBanManager::E_STR_TYPE_ACCOUNT || strType > CBanManager::E_STR_TYPE_MAX)
		return;

	char szEscaped[64];
	DBManager::instance().EscapeString(szEscaped, sizeof(szEscaped), arg1, strlen(arg1));

	bool ret = false;

	if (strType == CBanManager::E_STR_TYPE_ACCOUNT)
		ret = CHwidManager::instance().BanByAccountData(std::string(szEscaped));
	else
		ret = CHwidManager::instance().BanByPlayerData(std::string(szEscaped));

	if (ret)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("BANNED_PLAYER_SUCCESSFULLY_%s"), szEscaped);
		LogManager::instance().BanLog(ch, std::string(szEscaped), false, "COMPUTER", 99);
	}
	else
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("BANNED_PLAYER_FAILED"));
}
#endif

#ifdef __BAN_REASON_UTILITY__
ACMD(do_player_ban)
{
	char arg1[256], arg2[256], arg3[256], arg4[256], arg5[256], arg6[256];
	six_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2), arg3, sizeof(arg3), arg4, sizeof(arg4), arg5, sizeof(arg5), arg6, sizeof(arg6));

	if (!*arg1 || !*arg2 || !*arg3 || !*arg4 || !*arg5 || !*arg6)
		return;

	int banType = atoi(arg1);
	int strType = atoi(arg2);
	int availDt = atoi(arg3);

	if (banType < CBanManager::E_BAN_TYPE_ACCOUNT || banType > CBanManager::E_BAN_TYPE_MAX)
		return;

	if (strType < CBanManager::E_STR_TYPE_ACCOUNT || strType > CBanManager::E_STR_TYPE_MAX)
		return;

	if (availDt < 0)
		return;

	char szEscapedSelected[64];
	DBManager::instance().EscapeString(szEscapedSelected, sizeof(szEscapedSelected), arg4, strlen(arg4));

	char szEscapedBanReason[128];
	DBManager::instance().EscapeString(szEscapedBanReason, sizeof(szEscapedBanReason), arg5, strlen(arg5));

	char szEscapedEvidence[200];
	DBManager::instance().EscapeString(szEscapedEvidence, sizeof(szEscapedEvidence), arg6, strlen(arg6));

	bool ret = CBanManager::instance().PrepareBan(banType, strType, std::string(szEscapedSelected), availDt, std::string(szEscapedBanReason), std::string(szEscapedEvidence));
	if (ret)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("BANNED_PLAYER_SUCCESSFULLY_%s"), szEscapedSelected);

		std::string banTypeValue = banType == CBanManager::E_BAN_TYPE_ACCOUNT ? "ACCOUNT" : "PLAYER";
		bool isPlayer = strType == CBanManager::E_STR_TYPE_ACCOUNT ? false : true;

		LogManager::instance().BanLog(ch, std::string(szEscapedSelected), isPlayer, banTypeValue, availDt);
	}
	else
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("BANNED_PLAYER_FAILED"));
}
#endif

#ifdef __SCP1453_EXTENSIONS__
ACMD(do_dungeon_report)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("DUNGEON_REPORT_FAIL"));
		return;
	}

	if (!ch->GetDungeon())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("DUNGEON_REPORT_FAIL"));
		return;
	}

	ch->GetDungeon()->SaveReport(ch, arg1);
	ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("DUNGEON_REPORT_SUCCES"));
}

ACMD(do_dungeon_master)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1 || !ch->GetDungeon())
		return;

	if (!strcmp(arg1, "k"))
		ch->GetDungeon()->KillAll();
	else if (!strcmp(arg1, "km"))
		ch->GetDungeon()->KillAllMob();
	else if (!strcmp(arg1, "tm"))
		ch->GetDungeon()->TransferAllMonsters(ch);
}
ACMD(do_dungeon_room_info)
{
	auto dungeonMap = CDungeonManager::instance().GetDungeonMap();
	ch->ChatPacket(CHAT_TYPE_INFO, "Acilan toplam oda sayisi: %d", dungeonMap.size());
}
ACMD(do_dungeon_room_close)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	int dMapInd = atoi(arg1);

	LPDUNGEON pDungeon = CDungeonManager::instance().FindByMapIndex(dMapInd);
	pDungeon->ExitAll();
}
#endif

#ifdef __ENABLE_WIKI_SYSTEM__
ACMD(do_wiki)
{
	CWikiManager::instance().HandleCommand(ch, argument);
}
#endif

#ifdef __CMD_EXTENSIONS__
ACMD(do_gmbonus)
{
	AUTO_ARGUMENT();
	if (arguments.size() < 3)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "usage /gmbonus <numb> <point_type> <point_value>");
		ch->ChatPacket(CHAT_TYPE_INFO, "        numb: 0-10");
		ch->ChatPacket(CHAT_TYPE_INFO, "        point_type: enum EPointTypes");
		return;
	}

	DWORD dwType = 0;
	BYTE bApplyOn = 0; // point type
	long lApplyValue = 0;

	AUTO_ARGUMENT_NUMBER(dwType, 0);
	AUTO_ARGUMENT_NUMBER(bApplyOn, 1);
	AUTO_ARGUMENT_NUMBER(lApplyValue, 2);

	ch->RemoveAffect(AFFECT_GM_BONUS_1 + dwType);

	if (lApplyValue != 0)
		ch->ADD_AFFECT_INFINITY(AFFECT_GM_BONUS_1 + dwType, bApplyOn, lApplyValue);

	ch->ChatPacket(CHAT_TYPE_INFO, "do_gmbonus dwType: %d  bApplyOn: %d  lApplyValue: %d", dwType, bApplyOn, lApplyValue);
}
#endif // __CMD_EXTENSIONS__

#ifdef __AS_BOT__
void StartASBot(DWORD delay);
void StopASBot();
ACMD(do_as_bot)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	int delay;
	str_to_number(delay, arg1);

	if (delay == -1)
		StopASBot();
	else
		StartASBot(delay);
}

void StartGreetBot(DWORD delay);
void StopGreetBot();
ACMD(do_greet_bot)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	int delay;
	str_to_number(delay, arg1);

	if (delay == -1)
		StopGreetBot();
	else
		StartGreetBot(delay);
}
#endif
