#include "stdafx.h"
#ifdef __FreeBSD__
#include <md5.h>
#else
#include "../../libthecore/include/xmd5.h"
#endif

#include "utils.h"
#include "config.h"
#include "desc_client.h"
#include "desc_manager.h"
#include "char.h"
#include "char_manager.h"
#include "motion.h"
#include "packet.h"
#include "affect.h"
#include "pvp.h"
#include "start_position.h"
#include "party.h"
#include "guild_manager.h"
#include "p2p.h"
#ifdef __SCP1453_EXTENSIONS__
#include "dungeon_new.h"
#else
#include "dungeon.h"
#endif
#include "messenger_manager.h"
#include "war_map.h"
#include "questmanager.h"
#include "item_manager.h"
#include "mob_manager.h"
#include "item.h"
#include "buffer_manager.h"
#include "unique_item.h"
#include "log.h"
#include "../../common/VnumHelper.h"
#ifdef __GROWTH_PET_SYSTEM__
#include "New_PetSystem.h"
#endif
#ifdef __GROWTH_MOUNT_SYSTEM__
#include "New_MountSystem.h"
#endif
#ifdef __INGAME_MALL__
#include "item_shop.h"
#endif
#ifdef __MOUNT_COSTUME_SYSTEM__
#include "MountSystem.h"
#endif

#ifdef ENABLE_REMOTE_SHOP_SYSTEM
#include "shop.h"
#include "shop_manager.h"
#endif
ACMD(do_user_horse_ride)
{
	if (ch->IsObserverMode())
		return;

	if (ch->IsDead() || ch->IsStun())
		return;

	if (ch->IsHorseRiding() == false)
	{
		if (ch->GetMountVnum())
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ¹Ì Å»°ÍÀ» ÀÌ¿ëÁßÀÔ´Ï´Ù."));
			return;
		}

		if (ch->GetHorse() == NULL)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¸»À» ¸ÕÀú ¼ÒÈ¯ÇØÁÖ¼¼¿ä."));
			return;
		}

		ch->StartRiding();
	}
	else
	{
		ch->StopRiding();
	}
}

ACMD(do_user_horse_back)
{
	if (ch->GetHorse() != NULL)
	{
		ch->HorseSummon(false);
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¸»À» µ¹·Áº¸³Â½À´Ï´Ù."));
	}
	else if (ch->IsHorseRiding() == true)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¸»¿¡¼­ ¸ÕÀú ³»·Á¾ß ÇÕ´Ï´Ù."));
	}
	else
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¸»À» ¸ÕÀú ¼ÒÈ¯ÇØÁÖ¼¼¿ä."));
	}
}

ACMD(do_user_horse_feed)
{
	if (ch->GetMyShop())
		return;

	if (ch->GetHorse() == NULL)
	{
		if (ch->IsHorseRiding() == false)
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¸»À» ¸ÕÀú ¼ÒÈ¯ÇØÁÖ¼¼¿ä."));
		else
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¸»À» Åº »óÅÂ¿¡¼­´Â ¸ÔÀÌ¸¦ ÁÙ ¼ö ¾ø½À´Ï´Ù."));
		return;
	}

	DWORD dwFood = ch->GetHorseGrade() + 50054 - 1;

	if (ch->CountSpecifyItem(dwFood) > 0)
	{
		ch->RemoveSpecifyItem(dwFood, 1);
		ch->FeedHorse();
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¸»¿¡°Ô %s%s ÁÖ¾ú½À´Ï´Ù."),
			ITEM_MANAGER::instance().GetTable(dwFood)->szLocaleName,
			"");
	}
	else
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%s ¾ÆÀÌÅÛÀÌ ÇÊ¿äÇÕ´Ï´Ù"), ITEM_MANAGER::instance().GetTable(dwFood)->szLocaleName);
	}
}

#define MAX_REASON_LEN		128

EVENTINFO(TimedEventInfo)
{
	DynamicCharacterPtr ch;
	int		subcmd;
	int         	left_second;
	char		szReason[MAX_REASON_LEN];

	TimedEventInfo()
		: ch()
		, subcmd(0)
		, left_second(0)
	{
		::memset(szReason, 0, MAX_REASON_LEN);
	}
};

struct SendDisconnectFunc
{
	void operator () (LPDESC d)
	{
		if (d->GetCharacter())
		{
			if (g_bFlushAtShutdown)
			{
				d->GetCharacter()->SaveReal();
				DWORD pid = d->GetCharacter()->GetPlayerID();
				db_clientdesc->DBPacketHeader(HEADER_GD_FLUSH_CACHE, 0, sizeof(DWORD));
				db_clientdesc->Packet(&pid, sizeof(DWORD));
			}

			if (d->GetCharacter()->GetGMLevel() == GM_PLAYER)
				d->GetCharacter()->ChatPacket(CHAT_TYPE_COMMAND, "quit Shutdown(SendDisconnectFunc)");
		}
	}
};

struct DisconnectFunc
{
	void operator () (LPDESC d)
	{
		if (d->GetType() == DESC_TYPE_CONNECTOR)
			return;

		if (d->IsPhase(PHASE_P2P))
			return;

		if (d->GetCharacter())
			d->GetCharacter()->Disconnect("Shutdown(DisconnectFunc)");

		d->SetPhase(PHASE_CLOSE);
	}
};

EVENTINFO(shutdown_event_data)
{
	int seconds;

	shutdown_event_data()
		: seconds(0)
	{
	}
};

EVENTFUNC(shutdown_event)
{
	shutdown_event_data* info = dynamic_cast<shutdown_event_data*>(event->info);

	if (info == NULL)
	{
		sys_err("shutdown_event> <Factor> Null pointer");
		return 0;
	}

	int* pSec = &(info->seconds);

	if (*pSec < 0)
	{
		sys_log(0, "shutdown_event sec %d", *pSec);

		if (-- * pSec == -10)
		{
			const DESC_MANAGER::DESC_SET& c_set_desc = DESC_MANAGER::instance().GetClientSet();
			std::for_each(c_set_desc.begin(), c_set_desc.end(), DisconnectFunc());
			return passes_per_sec;
		}
		else if (*pSec < -10)
			return 0;

		return passes_per_sec;
	}
	else if (*pSec == 0)
	{
		const DESC_MANAGER::DESC_SET& c_set_desc = DESC_MANAGER::instance().GetClientSet();
		std::for_each(c_set_desc.begin(), c_set_desc.end(), SendDisconnectFunc());
		g_bNoMoreClient = true;
		--* pSec;
		return passes_per_sec;
	}
	else
	{
		char buf[64];
		snprintf(buf, sizeof(buf), LC_TEXT("¼Ë´Ù¿îÀÌ %dÃÊ ³²¾Ò½À´Ï´Ù."), *pSec);
		SendNotice(buf);

		--* pSec;
		return passes_per_sec;
	}
}

void Shutdown(int iSec)
{
	if (g_bNoMoreClient)
	{
		thecore_shutdown();
		return;
	}

	CWarMapManager::instance().OnShutdown();
#ifdef __SCP1453_EXTENSIONS__
	CDungeonManager::instance().OnShutdown();
#endif

	// char buf[64];
	// snprintf(buf, sizeof(buf), LC_TEXT("%dÃÊ ÈÄ °ÔÀÓÀÌ ¼Ë´Ù¿î µË´Ï´Ù."), iSec);

	// SendNotice(buf);

	shutdown_event_data* info = AllocEventInfo<shutdown_event_data>();
	info->seconds = iSec;

	event_create(shutdown_event, info, 1);
}

ACMD(do_shutdown)
{
	if (!ch)
		return;

	TPacketGGShutdown p;
	p.bHeader = HEADER_GG_SHUTDOWN;
	P2P_MANAGER::instance().Send(&p, sizeof(TPacketGGShutdown));

	Shutdown(10);
}

extern int g_shutdown_disconnect_pulse;
extern int g_shutdown_disconnect_force_pulse;
extern int g_shutdown_core_pulse;
extern bool g_bShutdown;
ACMD(do_shutdown_this)
{
	if (!ch)
		return;

	const DESC_MANAGER::DESC_SET& c_set_desc = DESC_MANAGER::instance().GetClientSet();
	std::for_each(c_set_desc.begin(), c_set_desc.end(), SendDisconnectFunc());
	g_bNoMoreClient = true;
	g_bShutdown = true;
	thecore_shutdown();

	// Shutdown(0);


	// g_bShutdown = true;
	// g_bNoMoreClient = true;

	// g_shutdown_disconnect_pulse = thecore_pulse() + PASSES_PER_SEC(10);
	// g_shutdown_disconnect_force_pulse = thecore_pulse() + PASSES_PER_SEC(20);
	// g_shutdown_core_pulse = thecore_pulse() + PASSES_PER_SEC(30);
}

EVENTFUNC(timed_event)
{
	TimedEventInfo* info = dynamic_cast<TimedEventInfo*>(event->info);

	if (info == NULL)
	{
		sys_err("timed_event> <Factor> Null pointer");
		return 0;
	}

	LPCHARACTER	ch = info->ch;
	if (ch == NULL) { // <Factor>
		return 0;
	}
	LPDESC d = ch->GetDesc();

	if (info->left_second <= 0)
	{
		ch->m_pkTimedEvent = NULL;

		switch (info->subcmd)
		{
		case SCMD_LOGOUT:
		case SCMD_QUIT:
		case SCMD_PHASE_SELECT:
		{
			TPacketNeedLoginLogInfo acc_info;
			acc_info.dwPlayerID = ch->GetDesc()->GetAccountTable().id;
			db_clientdesc->DBPacket(HEADER_GD_VALID_LOGOUT, 0, &acc_info, sizeof(acc_info));
		}
		break;
		}

		switch (info->subcmd)
		{
		case SCMD_LOGOUT:
		{
			if (d)
			{
				d->SetPhase(PHASE_CLOSE);
			}
		}
		break;

		case SCMD_QUIT:
			ch->ChatPacket(CHAT_TYPE_COMMAND, "quit");
			if (d) // @fixme197
				d->DelayedDisconnect(1);
			break;

		case SCMD_PHASE_SELECT:
		{
			ch->Disconnect("timed_event - SCMD_PHASE_SELECT");

			if (d)
			{
				d->SetPhase(PHASE_SELECT);
			}
		}
		break;
		}

		return 0;
	}
	else
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%dÃÊ ³²¾Ò½À´Ï´Ù."), info->left_second);
		--info->left_second;
	}

	return PASSES_PER_SEC(1);
}

ACMD(do_cmd)
{
	if (ch->m_pkTimedEvent)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Ãë¼Ò µÇ¾ú½À´Ï´Ù."));
		event_cancel(&ch->m_pkTimedEvent);
		return;
	}

	switch (subcmd)
	{
	case SCMD_LOGOUT:
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("·Î±×ÀÎ È­¸éÀ¸·Î µ¹¾Æ °©´Ï´Ù. Àá½Ã¸¸ ±â´Ù¸®¼¼¿ä."));
		break;

	case SCMD_QUIT:
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("°ÔÀÓÀ» Á¾·á ÇÕ´Ï´Ù. Àá½Ã¸¸ ±â´Ù¸®¼¼¿ä."));
		break;

	case SCMD_PHASE_SELECT:
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Ä³¸¯ÅÍ¸¦ ÀüÈ¯ ÇÕ´Ï´Ù. Àá½Ã¸¸ ±â´Ù¸®¼¼¿ä."));
		break;
	}

	int nExitLimitTime = 10;

	if (ch->IsHack(false, true, nExitLimitTime) &&
		(!ch->GetWarMap() || ch->GetWarMap()->GetType() == GUILD_WAR_TYPE_FLAG))
	{
		return;
	}

	switch (subcmd)
	{
	case SCMD_LOGOUT:
	case SCMD_QUIT:
	case SCMD_PHASE_SELECT:
	{
		TimedEventInfo* info = AllocEventInfo<TimedEventInfo>();

		{
			if (ch->IsPosition(POS_FIGHTING))
				info->left_second = 10;
			else
				info->left_second = 3;
		}

		info->ch = ch;
		info->subcmd = subcmd;
		strlcpy(info->szReason, argument, sizeof(info->szReason));

		ch->m_pkTimedEvent = event_create(timed_event, info, 1);
	}
	break;
	}
}

ACMD(do_fishing)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	ch->SetRotation(atof(arg1));
	ch->fishing();
}

ACMD(do_console)
{
	ch->ChatPacket(CHAT_TYPE_COMMAND, "ConsoleEnable");
}

ACMD(do_restart)
{
	if (false == ch->IsDead())
	{
		ch->ChatPacket(CHAT_TYPE_COMMAND, "CloseRestartWindow");
		ch->StartRecoveryEvent();
		return;
	}

	if (NULL == ch->m_pkDeadEvent)
		return;

	int iTimeToDead = (event_time(ch->m_pkDeadEvent) / passes_per_sec);

	if (subcmd != SCMD_RESTART_TOWN && (!ch->GetWarMap() || ch->GetWarMap()->GetType() == GUILD_WAR_TYPE_FLAG))
	{
		if (!test_server)
		{
			if (ch->IsHack())
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¾ÆÁ÷ Àç½ÃÀÛ ÇÒ ¼ö ¾ø½À´Ï´Ù. (%dÃÊ ³²À½)"), iTimeToDead - (180 - g_nPortalLimitTime));
				return;
			}
#define eFRS_HERESEC	170

			if (iTimeToDead > eFRS_HERESEC)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¾ÆÁ÷ Àç½ÃÀÛ ÇÒ ¼ö ¾ø½À´Ï´Ù. (%dÃÊ ³²À½)"), iTimeToDead - eFRS_HERESEC);
				return;
			}
		}
	}

	//PREVENT_HACK

	if (subcmd == SCMD_RESTART_TOWN)
	{
		if (ch->IsHack())
		{
			if (!ch->GetWarMap() || ch->GetWarMap()->GetType() == GUILD_WAR_TYPE_FLAG)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¾ÆÁ÷ Àç½ÃÀÛ ÇÒ ¼ö ¾ø½À´Ï´Ù. (%dÃÊ ³²À½)"), iTimeToDead - (180 - g_nPortalLimitTime));
				return;
			}
		}

#define eFRS_TOWNSEC	173
		if (iTimeToDead > eFRS_TOWNSEC)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¾ÆÁ÷ ¸¶À»¿¡¼­ Àç½ÃÀÛ ÇÒ ¼ö ¾ø½À´Ï´Ù. (%d ÃÊ ³²À½)"), iTimeToDead - eFRS_TOWNSEC);
			return;
		}
	}

	//END_PREVENT_HACK

	ch->ChatPacket(CHAT_TYPE_COMMAND, "CloseRestartWindow");

	ch->GetDesc()->SetPhase(PHASE_GAME);
	ch->SetPosition(POS_STANDING);
	ch->StartRecoveryEvent();

	if (ch->GetDungeon())
		ch->GetDungeon()->UseRevive(ch);

	if (ch->GetWarMap() && !ch->IsObserverMode())
	{
		CWarMap* pMap = ch->GetWarMap();
		DWORD dwGuildOpponent = pMap ? pMap->GetGuildOpponent(ch) : 0;

		if (dwGuildOpponent)
		{
			switch (subcmd)
			{
			case SCMD_RESTART_TOWN:
			{
				sys_log(0, "do_restart: restart town");
				PIXEL_POSITION pos;

				if (CWarMapManager::instance().GetStartPosition(ch->GetMapIndex(), ch->GetGuild()->GetID() < dwGuildOpponent ? 0 : 1, pos))
					ch->Show(ch->GetMapIndex(), pos.x, pos.y);
				else
					ch->ExitToSavedLocation();

				ch->PointChange(POINT_HP, ch->GetMaxHP() - ch->GetHP());
				ch->PointChange(POINT_SP, ch->GetMaxSP() - ch->GetSP());
				ch->ReviveInvisible(5);
#ifdef __PET_SYSTEM_PROTO__
				ch->CheckPet();
#endif
#ifdef __MOUNT_COSTUME_SYSTEM__
				ch->CheckMount();
#endif
			}
			break;

			case SCMD_RESTART_HERE:
			{
				sys_log(0, "do_restart: restart here");
				ch->RestartAtSamePos();
				//ch->Show(ch->GetMapIndex(), ch->GetX(), ch->GetY());
				ch->PointChange(POINT_HP, ch->GetMaxHP() - ch->GetHP());
				ch->PointChange(POINT_SP, ch->GetMaxSP() - ch->GetSP());
				ch->ReviveInvisible(5);
#ifdef __PET_SYSTEM_PROTO__
				ch->CheckPet();
#endif
#ifdef __MOUNT_COSTUME_SYSTEM__
				ch->CheckMount();
#endif
			}
			break;
			}

			return;
		}
	}
	switch (subcmd)
	{
	case SCMD_RESTART_TOWN:
	{
		sys_log(0, "do_restart: restart town");
		PIXEL_POSITION pos;

		if (SECTREE_MANAGER::instance().GetRecallPositionByEmpire(ch->GetMapIndex(), ch->GetEmpire(), pos))
			ch->WarpSet(pos.x, pos.y);
		else
			ch->WarpSet(EMPIRE_START_X(ch->GetEmpire()), EMPIRE_START_Y(ch->GetEmpire()));

		ch->PointChange(POINT_HP, 50 - ch->GetHP());
		ch->DeathPenalty(1);
	}
	break;

	case SCMD_RESTART_HERE:
	{
		sys_log(0, "do_restart: restart here");
		ch->RestartAtSamePos();
		ch->PointChange(POINT_HP, 50 - ch->GetHP());
		ch->DeathPenalty(0);
		ch->ReviveInvisible(5);
#ifdef __PET_SYSTEM_PROTO__
		ch->CheckPet();
#endif
#ifdef __MOUNT_COSTUME_SYSTEM__
		ch->CheckMount();
#endif
		break;
	}
	}
}

#define MAX_STAT g_iStatusPointSetMaxValue

ACMD(do_stat_reset)
{
	ch->PointChange(POINT_STAT_RESET_COUNT, 12 - ch->GetPoint(POINT_STAT_RESET_COUNT));
}

ACMD(do_stat_minus)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	if (ch->GetPoint(POINT_STAT_RESET_COUNT) <= 0)
		return;

	if (!strcmp(arg1, "st"))
	{
		if (ch->GetRealPoint(POINT_ST) <= JobInitialPoints[ch->GetJob()].st)
			return;

		ch->SetRealPoint(POINT_ST, ch->GetRealPoint(POINT_ST) - 1);
		ch->SetPoint(POINT_ST, ch->GetPoint(POINT_ST) - 1);
		ch->ComputePoints();
		ch->PointChange(POINT_ST, 0);
	}
	else if (!strcmp(arg1, "dx"))
	{
		if (ch->GetRealPoint(POINT_DX) <= JobInitialPoints[ch->GetJob()].dx)
			return;

		ch->SetRealPoint(POINT_DX, ch->GetRealPoint(POINT_DX) - 1);
		ch->SetPoint(POINT_DX, ch->GetPoint(POINT_DX) - 1);
		ch->ComputePoints();
		ch->PointChange(POINT_DX, 0);
	}
	else if (!strcmp(arg1, "ht"))
	{
		if (ch->GetRealPoint(POINT_HT) <= JobInitialPoints[ch->GetJob()].ht)
			return;

		ch->SetRealPoint(POINT_HT, ch->GetRealPoint(POINT_HT) - 1);
		ch->SetPoint(POINT_HT, ch->GetPoint(POINT_HT) - 1);
		ch->ComputePoints();
		ch->PointChange(POINT_HT, 0);
		ch->PointChange(POINT_MAX_HP, 0);
	}
	else if (!strcmp(arg1, "iq"))
	{
		if (ch->GetRealPoint(POINT_IQ) <= JobInitialPoints[ch->GetJob()].iq)
			return;

		ch->SetRealPoint(POINT_IQ, ch->GetRealPoint(POINT_IQ) - 1);
		ch->SetPoint(POINT_IQ, ch->GetPoint(POINT_IQ) - 1);
		ch->ComputePoints();
		ch->PointChange(POINT_IQ, 0);
		ch->PointChange(POINT_MAX_SP, 0);
	}
	else
		return;

	ch->PointChange(POINT_STAT, +1);
	ch->PointChange(POINT_STAT_RESET_COUNT, -1);
	ch->ComputePoints();
}

ACMD(do_stat)
{
	char arg1[256];
#ifdef __STATUS_UP_REWORK__
	char arg2[256];
	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));
#else
	one_argument(argument, arg1, sizeof(arg1));
#endif

	if (!*arg1)
		return;

#ifdef __STATUS_UP_REWORK__
	int iStatUp = 1;
	if (*arg2)
		iStatUp = atoi(arg2);
#endif

#ifdef __STATUS_UP_REWORK__
	if (ch->GetPoint(POINT_STAT) < iStatUp)
		iStatUp = ch->GetPoint(POINT_STAT);
#else
	if (ch->GetPoint(POINT_STAT) <= 0)
		return;
#endif

#ifdef __STATUS_UP_REWORK__
	if (iStatUp > 10)
		return;
#endif

	BYTE idx = 0;

	if (!strcmp(arg1, "st"))
		idx = POINT_ST;
	else if (!strcmp(arg1, "dx"))
		idx = POINT_DX;
	else if (!strcmp(arg1, "ht"))
		idx = POINT_HT;
	else if (!strcmp(arg1, "iq"))
		idx = POINT_IQ;
	else
		return;

	// ch->ChatPacket(CHAT_TYPE_INFO, "%s GRP(%d) idx(%u), MAX_STAT(%d), expr(%d)", __FUNCTION__, ch->GetRealPoint(idx), idx, MAX_STAT, ch->GetRealPoint(idx) >= MAX_STAT);
#ifdef __STATUS_UP_REWORK__
	if ((ch->GetRealPoint(idx) + iStatUp) > MAX_STAT)
		iStatUp = MAX_STAT - ch->GetRealPoint(idx);

	if (iStatUp < 1)
#else
	if (ch->GetRealPoint(idx) >= MAX_STAT)
#endif
		return;
#ifdef __STATUS_UP_REWORK__
	ch->SetRealPoint(idx, ch->GetRealPoint(idx) + iStatUp);
	ch->SetPoint(idx, ch->GetPoint(idx) + iStatUp);
#else
	ch->SetRealPoint(idx, ch->GetRealPoint(idx) + 1);
	ch->SetPoint(idx, ch->GetPoint(idx) + 1);
#endif
	ch->ComputePoints();
	ch->PointChange(idx, 0);

	if (idx == POINT_IQ)
	{
		ch->PointChange(POINT_MAX_HP, 0);
	}
	else if (idx == POINT_HT)
	{
		ch->PointChange(POINT_MAX_SP, 0);
	}
#ifdef __STATUS_UP_REWORK__
	ch->PointChange(POINT_STAT, -iStatUp);
#else
	ch->PointChange(POINT_STAT, -1);
#endif
	ch->ComputePoints();
}

ACMD(do_pvp)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	DWORD vid = 0;
	str_to_number(vid, arg1);
	LPCHARACTER pkVictim = CHARACTER_MANAGER::instance().Find(vid);

	if (!pkVictim)
		return;

	if (pkVictim->IsNPC())
		return;

	CPVPManager::instance().Insert(ch, pkVictim);
}

ACMD(do_guildskillup)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	if (!ch->GetGuild())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<±æµå> ±æµå¿¡ ¼ÓÇØÀÖÁö ¾Ê½À´Ï´Ù."));
		return;
	}

	CGuild* g = ch->GetGuild();
	TGuildMember* gm = g->GetMember(ch->GetPlayerID());
	if (gm->grade == GUILD_LEADER_GRADE)
	{
		DWORD vnum = 0;
		str_to_number(vnum, arg1);
		g->SkillLevelUp(vnum);
	}
	else
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<±æµå> ±æµå ½ºÅ³ ·¹º§À» º¯°æÇÒ ±ÇÇÑÀÌ ¾ø½À´Ï´Ù."));
	}
}

ACMD(do_skillup)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	DWORD vnum = 0;
	str_to_number(vnum, arg1);

	if (true == ch->CanUseSkill(vnum))
	{
		ch->SkillLevelUp(vnum);
	}
	else
	{
		switch (vnum)
		{
		case SKILL_HORSE_WILDATTACK:
		case SKILL_HORSE_CHARGE:
		case SKILL_HORSE_ESCAPE:
		case SKILL_HORSE_WILDATTACK_RANGE:
#ifdef __7AND8TH_SKILLS__
		case SKILL_ANTI_PALBANG:
		case SKILL_ANTI_AMSEOP:
		case SKILL_ANTI_SWAERYUNG:
		case SKILL_ANTI_YONGBI:
		case SKILL_ANTI_GIGONGCHAM:
		case SKILL_ANTI_HWAJO:
		case SKILL_ANTI_MARYUNG:
		case SKILL_ANTI_BYEURAK:
#ifdef __WOLFMAN_CHARACTER__
		case SKILL_ANTI_SALPOONG:
#endif
		case SKILL_HELP_PALBANG:
		case SKILL_HELP_AMSEOP:
		case SKILL_HELP_SWAERYUNG:
		case SKILL_HELP_YONGBI:
		case SKILL_HELP_GIGONGCHAM:
		case SKILL_HELP_HWAJO:
		case SKILL_HELP_MARYUNG:
		case SKILL_HELP_BYEURAK:
#ifdef __WOLFMAN_CHARACTER__
		case SKILL_HELP_SALPOONG:
#endif
#endif
		case SKILL_7_A_ANTI_TANHWAN:
		case SKILL_7_B_ANTI_AMSEOP:
		case SKILL_7_C_ANTI_SWAERYUNG:
		case SKILL_7_D_ANTI_YONGBI:

		case SKILL_8_A_ANTI_GIGONGCHAM:
		case SKILL_8_B_ANTI_YEONSA:
		case SKILL_8_C_ANTI_MAHWAN:
		case SKILL_8_D_ANTI_BYEURAK:
			// deleted passive skills
			ch->SkillLevelUp(vnum);
			break;
		}
	}
}

//
//
ACMD(do_safebox_close)
{
	ch->CloseSafebox();
}

//
//
ACMD(do_safebox_password)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));
	ch->ReqSafeboxLoad(arg1);
}

ACMD(do_safebox_change_password)
{
	char arg1[256];
	char arg2[256];

	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (!*arg1 || strlen(arg1) > 6)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<Ã¢°í> Àß¸øµÈ ¾ÏÈ£¸¦ ÀÔ·ÂÇÏ¼Ì½À´Ï´Ù."));
		return;
	}

	if (!*arg2 || strlen(arg2) > 6)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<Ã¢°í> Àß¸øµÈ ¾ÏÈ£¸¦ ÀÔ·ÂÇÏ¼Ì½À´Ï´Ù."));
		return;
	}

	TSafeboxChangePasswordPacket p;

	p.dwID = ch->GetDesc()->GetAccountTable().id;
	strlcpy(p.szOldPassword, arg1, sizeof(p.szOldPassword));
	strlcpy(p.szNewPassword, arg2, sizeof(p.szNewPassword));

	db_clientdesc->DBPacket(HEADER_GD_SAFEBOX_CHANGE_PASSWORD, ch->GetDesc()->GetHandle(), &p, sizeof(p));
}

ACMD(do_mall_password)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1 || strlen(arg1) > 6)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<Ã¢°í> Àß¸øµÈ ¾ÏÈ£¸¦ ÀÔ·ÂÇÏ¼Ì½À´Ï´Ù."));
		return;
	}

	int iPulse = thecore_pulse();

	if (ch->GetMall())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<Ã¢°í> Ã¢°í°¡ ÀÌ¹Ì ¿­·ÁÀÖ½À´Ï´Ù."));
		return;
	}

	if (iPulse - ch->GetMallLoadTime() < passes_per_sec * 10)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<Ã¢°í> Ã¢°í¸¦ ´ÝÀºÁö 10ÃÊ ¾È¿¡´Â ¿­ ¼ö ¾ø½À´Ï´Ù."));
		return;
	}

	ch->SetMallLoadTime(iPulse);

	TSafeboxLoadPacket p;
	p.dwID = ch->GetDesc()->GetAccountTable().id;
	strlcpy(p.szLogin, ch->GetDesc()->GetAccountTable().login, sizeof(p.szLogin));
	strlcpy(p.szPassword, arg1, sizeof(p.szPassword));

	db_clientdesc->DBPacket(HEADER_GD_MALL_LOAD, ch->GetDesc()->GetHandle(), &p, sizeof(p));
}

ACMD(do_mall_close)
{
	if (ch->GetMall())
	{
		ch->SetMallLoadTime(thecore_pulse());
		ch->CloseMall();
		ch->Save();
	}
}

ACMD(do_ungroup)
{
	if (!ch->GetParty())
		return;

	if (!CPartyManager::instance().IsEnablePCParty())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<ÆÄÆ¼> ¼­¹ö ¹®Á¦·Î ÆÄÆ¼ °ü·Ã Ã³¸®¸¦ ÇÒ ¼ö ¾ø½À´Ï´Ù."));
		return;
	}

	if (ch->GetDungeon())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<ÆÄÆ¼> ´øÀü ¾È¿¡¼­´Â ÆÄÆ¼¿¡¼­ ³ª°¥ ¼ö ¾ø½À´Ï´Ù."));
		return;
	}

	if (IS_DUNGEON_ZONE(ch->GetMapIndex()))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<ÆÄÆ¼> ´øÀü ¾È¿¡¼­´Â ÆÄÆ¼¿¡¼­ ³ª°¥ ¼ö ¾ø½À´Ï´Ù."));
		return;
	}

	LPPARTY pParty = ch->GetParty();

	if (pParty->GetMemberCount() == 2)
	{
		// party disband
		CPartyManager::instance().DeleteParty(pParty);
	}
	else
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<ÆÄÆ¼> ÆÄÆ¼¿¡¼­ ³ª°¡¼Ì½À´Ï´Ù."));
		//pParty->SendPartyRemoveOneToAll(ch);
		pParty->Quit(ch->GetPlayerID());
		//pParty->SendPartyRemoveAllToOne(ch);
	}
}

ACMD(do_close_shop)
{
	if (ch->GetMyShop())
	{
		ch->CloseMyShop();
		return;
	}
}

ACMD(do_set_walk_mode)
{
	ch->SetNowWalking(true);
	ch->SetWalking(true);
}

ACMD(do_set_run_mode)
{
	ch->SetNowWalking(false);
	ch->SetWalking(false);
}

ACMD(do_war)
{
	CGuild* g = ch->GetGuild();

	if (!g)
		return;

	if (g->UnderAnyWar())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<±æµå> ÀÌ¹Ì ´Ù¸¥ ÀüÀï¿¡ ÂüÀü Áß ÀÔ´Ï´Ù."));
		return;
	}
#ifdef __GUILD_WAR_REWORK__
	char arg1[256], arg2[256], arg3[256], arg4[256];
	one_argument(three_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2), arg3, sizeof(arg3)), arg4, sizeof(arg4));
#else
	char arg1[256], arg2[256];
	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));
#endif
	DWORD type = GUILD_WAR_TYPE_FIELD; //@duzenleme 2014 yilinda cikan var acigi fixi

	if (!*arg1)
		return;

	if (*arg2)
	{
		str_to_number(type, arg2);

		if (type < 0 || type >= GUILD_WAR_TYPE_MAX_NUM)
			type = GUILD_WAR_TYPE_FIELD;
	}

	DWORD gm_pid = g->GetMasterPID();

	if (gm_pid != ch->GetPlayerID())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<±æµå> ±æµåÀü¿¡ ´ëÇÑ ±ÇÇÑÀÌ ¾ø½À´Ï´Ù."));
		return;
	}

	CGuild* opp_g = CGuildManager::instance().FindGuildByName(arg1);

	if (!opp_g)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<±æµå> ±×·± ±æµå°¡ ¾ø½À´Ï´Ù."));
		return;
	}

	switch (g->GetGuildWarState(opp_g->GetID()))
	{
	case GUILD_WAR_NONE:
	{
		if (opp_g->UnderAnyWar())
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<±æµå> »ó´ë¹æ ±æµå°¡ ÀÌ¹Ì ÀüÀï Áß ÀÔ´Ï´Ù."));
			return;
		}
	}
	break;

	case GUILD_WAR_SEND_DECLARE:
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ¹Ì ¼±ÀüÆ÷°í ÁßÀÎ ±æµåÀÔ´Ï´Ù."));
		return;
	}
	break;

	case GUILD_WAR_RECV_DECLARE:
	{
		if (opp_g->UnderAnyWar())
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<±æµå> »ó´ë¹æ ±æµå°¡ ÀÌ¹Ì ÀüÀï Áß ÀÔ´Ï´Ù."));
			g->RequestRefuseWar(opp_g->GetID()
#ifdef __GUILD_WAR_REWORK__
				, 0, 0
#endif
			);
			return;
		}
	}
	break;

	case GUILD_WAR_END:
		return;

	default:
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<±æµå> ÀÌ¹Ì ÀüÀï ÁßÀÎ ±æµåÀÔ´Ï´Ù."));
		g->RequestRefuseWar(opp_g->GetID()
#ifdef __GUILD_WAR_REWORK__
			, 0, 0
#endif
		);
		return;
	}

	if (!g->CanStartWar(type))
	{
		if (g->GetLadderPoint() == 0)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<±æµå> ·¹´õ Á¡¼ö°¡ ¸ðÀÚ¶ó¼­ ±æµåÀüÀ» ÇÒ ¼ö ¾ø½À´Ï´Ù."));
			sys_log(0, "GuildWar.StartError.NEED_LADDER_POINT");
		}
		else if (g->GetMemberCount() < GUILD_WAR_MIN_MEMBER_COUNT)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<±æµå> ±æµåÀüÀ» ÇÏ±â À§ÇØ¼± ÃÖ¼ÒÇÑ %d¸íÀÌ ÀÖ¾î¾ß ÇÕ´Ï´Ù."), GUILD_WAR_MIN_MEMBER_COUNT);
			sys_log(0, "GuildWar.StartError.NEED_MINIMUM_MEMBER[%d]", GUILD_WAR_MIN_MEMBER_COUNT);
		}
		else
		{
			sys_log(0, "GuildWar.StartError.UNKNOWN_ERROR");
		}
		return;
	}

	if (!opp_g->CanStartWar(GUILD_WAR_TYPE_FIELD))
	{
		if (opp_g->GetLadderPoint() == 0)
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<±æµå> »ó´ë¹æ ±æµåÀÇ ·¹´õ Á¡¼ö°¡ ¸ðÀÚ¶ó¼­ ±æµåÀüÀ» ÇÒ ¼ö ¾ø½À´Ï´Ù."));
		else if (opp_g->GetMemberCount() < GUILD_WAR_MIN_MEMBER_COUNT)
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<±æµå> »ó´ë¹æ ±æµåÀÇ ±æµå¿ø ¼ö°¡ ºÎÁ·ÇÏ¿© ±æµåÀüÀ» ÇÒ ¼ö ¾ø½À´Ï´Ù."));
		return;
	}

	do
	{
		if (g->GetMasterCharacter() != NULL)
			break;

		CCI* pCCI = P2P_MANAGER::instance().FindByPID(g->GetMasterPID());

		if (pCCI != NULL)
			break;

		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<±æµå> »ó´ë¹æ ±æµåÀÇ ±æµåÀåÀÌ Á¢¼ÓÁßÀÌ ¾Æ´Õ´Ï´Ù."));
		g->RequestRefuseWar(opp_g->GetID()
#ifdef __GUILD_WAR_REWORK__
			, 0, 0
#endif
		);
		return;
	} while (false);

	do
	{
		if (opp_g->GetMasterCharacter() != NULL)
			break;

		CCI* pCCI = P2P_MANAGER::instance().FindByPID(opp_g->GetMasterPID());

		if (pCCI != NULL)
			break;

		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<±æµå> »ó´ë¹æ ±æµåÀÇ ±æµåÀåÀÌ Á¢¼ÓÁßÀÌ ¾Æ´Õ´Ï´Ù."));
		g->RequestRefuseWar(opp_g->GetID()
#ifdef __GUILD_WAR_REWORK__
			, 0, 0
#endif
		);
		return;
	} while (false);

#ifdef __GUILD_WAR_REWORK__
	int iMaxPlayer = 0;
	if (!*arg3) iMaxPlayer = 1000;
	else str_to_number(iMaxPlayer, arg3);
	if (iMaxPlayer < 1) iMaxPlayer = 1000;
	int iMaxScore = 0;
	if (!*arg4) iMaxScore = KOR_aGuildWarInfo[type].iEndScore;
	else str_to_number(iMaxScore, arg4);
	if (iMaxScore < 1) iMaxScore = KOR_aGuildWarInfo[type].iEndScore;
	g->RequestDeclareWar(opp_g->GetID(), type, iMaxPlayer, iMaxScore);
#else
	g->RequestDeclareWar(opp_g->GetID(), type);
#endif
}

ACMD(do_nowar)
{
	CGuild* g = ch->GetGuild();
	if (!g)
		return;

	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	DWORD gm_pid = g->GetMasterPID();

	if (gm_pid != ch->GetPlayerID())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<±æµå> ±æµåÀü¿¡ ´ëÇÑ ±ÇÇÑÀÌ ¾ø½À´Ï´Ù."));
		return;
	}

	CGuild* opp_g = CGuildManager::instance().FindGuildByName(arg1);

	if (!opp_g)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<±æµå> ±×·± ±æµå°¡ ¾ø½À´Ï´Ù."));
		return;
	}

	g->RequestRefuseWar(opp_g->GetID()
#ifdef __GUILD_WAR_REWORK__
		, 0, 0
#endif
	);
}

ACMD(do_pkmode)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	BYTE mode = 0;
	str_to_number(mode, arg1);

	if (mode == PK_MODE_PROTECT)
		return;

	if (ch->GetLevel() < PK_PROTECT_LEVEL && mode != 0)
		return;

	ch->SetPKMode(mode);
}

ACMD(do_messenger_auth)
{
	char arg1[256], arg2[256];
	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (!*arg1 || !*arg2)
		return;

	char answer = LOWER(*arg1);
	// @duzenleme
	// Kickhack engeli (2014 Part 1)
	bool bIsDenied = answer != 'y';
	bool bIsAdded = MessengerManager::instance().AuthToAdd(ch->GetName(), arg2, bIsDenied); // DENY
	if (bIsAdded && bIsDenied)
	{
		LPCHARACTER tch = CHARACTER_MANAGER::instance().FindPC(arg2);

		if (tch)
			tch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%s ´ÔÀ¸·Î ºÎÅÍ Ä£±¸ µî·ÏÀ» °ÅºÎ ´çÇß½À´Ï´Ù."), ch->GetName());
	}
}

ACMD(do_setblockmode)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (*arg1)
	{
		DWORD flag = 0;
		str_to_number(flag, arg1);
		ch->SetBlockMode(flag);

		//
		ch->UpdatePacket();
	}
}

ACMD(do_unmount)
{
#ifdef __MOUNT_COSTUME_SYSTEM__
	// balik tutarken ata binmicek.
	LPITEM rod = ch->GetWear(WEAR_WEAPON);
	if (rod && rod->GetType() == ITEM_ROD)
		return;

	if (ch->GetWear(WEAR_COSTUME_MOUNT))
	{
		CMountSystem* mountSystem = ch->GetMountSystem();
		LPITEM mount = ch->GetWear(WEAR_COSTUME_MOUNT);
		DWORD mobVnum = 0;

		if (!mountSystem || !mount)
			return;
#ifdef __MOUNT_CHANGELOOK__
		TItemTable* pkItemTransmutation = ITEM_MANAGER::instance().GetTable(mount->GetTransmutation());
		if (pkItemTransmutation)
		{
			for (int i = 0; i < ITEM_APPLY_MAX_NUM; ++i)
			{
				BYTE bType = pkItemTransmutation->aApplies[i].bType;
				long value = pkItemTransmutation->aApplies[i].lValue;
				if (bType == APPLY_MOUNT)
					mobVnum = value;
			}
		}
		else
		{
			if (mount->FindApplyValue(APPLY_MOUNT) != 0)
				mobVnum = mount->FindApplyValue(APPLY_MOUNT);
		}
#else
		if (mount->FindApplyValue(APPLY_MOUNT) != 0)
			mobVnum = mount->FindApplyValue(APPLY_MOUNT);
#endif
		if (ch->GetMountVnum())
		{
			if (mountSystem->CountSummoned() == 0)
			{
				mountSystem->Unmount(mobVnum);
			}
		}
		return;
	}
#endif
	if (true == ch->UnEquipSpecialRideUniqueItem())
	{
		ch->RemoveAffect(AFFECT_MOUNT);
		ch->RemoveAffect(AFFECT_MOUNT_BONUS);

		if (ch->IsHorseRiding())
		{
			ch->StopRiding();
		}
	}
	else
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÎº¥Åä¸®°¡ ²Ë Â÷¼­ ³»¸± ¼ö ¾ø½À´Ï´Ù."));
	}
}

ACMD(do_observer_exit)
{
	if (ch->IsObserverMode())
	{
		if (ch->GetWarMap())
			ch->SetWarMap(NULL);

		ch->ExitToSavedLocation();
		ch->SetObserverMode(false);
	}
}

ACMD(do_party_request)
{
	if (ch->GetParty())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ¹Ì ÆÄÆ¼¿¡ ¼ÓÇØ ÀÖÀ¸¹Ç·Î °¡ÀÔ½ÅÃ»À» ÇÒ ¼ö ¾ø½À´Ï´Ù."));
		return;
	}

	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	DWORD vid = 0;
	str_to_number(vid, arg1);
	LPCHARACTER tch = CHARACTER_MANAGER::instance().Find(vid);

	if (tch)
		if (!ch->RequestToParty(tch))
			ch->ChatPacket(CHAT_TYPE_COMMAND, "PartyRequestDenied");
}

ACMD(do_party_request_accept)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	DWORD vid = 0;
	str_to_number(vid, arg1);
	LPCHARACTER tch = CHARACTER_MANAGER::instance().Find(vid);

	if (tch)
		ch->AcceptToParty(tch);
}

ACMD(do_party_request_deny)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	DWORD vid = 0;
	str_to_number(vid, arg1);
	LPCHARACTER tch = CHARACTER_MANAGER::instance().Find(vid);

	if (tch)
		ch->DenyToParty(tch);
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

ACMD(do_inventory)
{
	int	index = 0;
	int	count = 1;

	char arg1[256];
	char arg2[256];

	LPITEM	item;

	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Usage: inventory <start_index> <count>");
		return;
	}

	if (!*arg2)
	{
		index = 0;
		str_to_number(count, arg1);
	}
	else
	{
		str_to_number(index, arg1); index = MIN(index, INVENTORY_MAX_NUM);
		str_to_number(count, arg2); count = MIN(count, INVENTORY_MAX_NUM);
	}

	for (int i = 0; i < count; ++i)
	{
		if (index >= INVENTORY_MAX_NUM)
			break;

		item = ch->GetInventoryItem(index);

		ch->ChatPacket(CHAT_TYPE_INFO, "inventory [%d] = %s",
			index, item ? item->GetName() : "<NONE>");
		++index;
	}
}

#ifdef __CUBE_WINDOW__
ACMD(do_cube)
{
	if (ch && !ch->CanAct(true, true, VAR_TIME_NONE, nullptr, 0))
		return;

	const char* line;
	char arg1[256], arg2[256], arg3[256];
	line = two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));
	one_argument(line, arg3, sizeof(arg3));

	if (0 == arg1[0])
	{
		return;
	}

	switch (LOWER(arg1[0]))
	{
	case 'o':	// open
		Cube_open(ch);
		break;

	default:
		return;
	}
}
#else
ACMD(do_cube)
{
	if (ch && !ch->CanAct(true, true, VAR_TIME_NONE, nullptr, 0))
		return;

	if (test_server)
		sys_log(0, "CUBE COMMAND <%s>: %s", ch->GetName(), argument);
	int cube_index = 0, inven_index = 0;
#ifdef __ADDITIONAL_INVENTORY__
	int inven_type = 0;
	char arg1[256], arg2[256], arg3[256], arg4[256];
	four_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2), arg3, sizeof(arg3), arg4, sizeof(arg4));
#else
	const char* line;
	char arg1[256], arg2[256], arg3[256];
	line = two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));
	one_argument(line, arg3, sizeof(arg3));
#endif
	if (0 == arg1[0])
	{
		// print usage
		ch->ChatPacket(CHAT_TYPE_INFO, "Usage: cube open");
		ch->ChatPacket(CHAT_TYPE_INFO, "       cube close");
		ch->ChatPacket(CHAT_TYPE_INFO, "       cube add <inveltory_index>");
		ch->ChatPacket(CHAT_TYPE_INFO, "       cube delete <cube_index>");
		ch->ChatPacket(CHAT_TYPE_INFO, "       cube list");
		ch->ChatPacket(CHAT_TYPE_INFO, "       cube cancel");
		ch->ChatPacket(CHAT_TYPE_INFO, "       cube make [all]");
		return;
	}

	const std::string& strArg1 = std::string(arg1);

	// r_info (request information)

	//					    (Server -> Client) /cube r_list npcVNUM resultCOUNT 123,1/125,1/128,1/130,5
	//

	//					   (Server -> Client) /cube m_info startIndex count 125,1|126,2|127,2|123,5&555,5&555,4/120000@125,1|126,2|127,2|123,5&555,5&555,4/120000
	//
	if (strArg1 == "r_info")
	{
		if (0 == arg2[0])
			Cube_request_result_list(ch);
		else
		{
			if (isdigit(*arg2))
			{
				int listIndex = 0, requestCount = 1;
				str_to_number(listIndex, arg2);

				if (0 != arg3[0] && isdigit(*arg3))
					str_to_number(requestCount, arg3);

				Cube_request_material_info(ch, listIndex, requestCount);
			}
		}

		return;
	}

	switch (LOWER(arg1[0]))
	{
	case 'o':	// open
		Cube_open(ch);
		break;

	case 'c':	// close
		Cube_close(ch);
		break;

	case 'l':	// list
		Cube_show_list(ch);
		break;

	case 'a':	// add cue_index inven_index
	{
#ifdef __ADDITIONAL_INVENTORY__
		if (0 == arg2[0] || !isdigit(*arg2) || 0 == arg3[0] || !isdigit(*arg3) || 0 == arg4[0] || !isdigit(*arg4))
#else
		if (0 == arg2[0] || !isdigit(*arg2) || 0 == arg3[0] || !isdigit(*arg3))
#endif
			return;

		str_to_number(cube_index, arg2);
		str_to_number(inven_index, arg3);
#ifdef __ADDITIONAL_INVENTORY__
		str_to_number(inven_type, arg4);
		Cube_add_item(ch, cube_index, inven_index, inven_type);
#else
		Cube_add_item(ch, cube_index, inven_index);
#endif
	}
	break;

	case 'd':	// delete
	{
		if (0 == arg2[0] || !isdigit(*arg2))
			return;

		str_to_number(cube_index, arg2);
		Cube_delete_item(ch, cube_index);
	}
	break;

	case 'm':	// make
		if (0 != arg2[0])
		{
			while (true == Cube_make(ch))
			{
				if (test_server)
					sys_log(0, "cube make success");
			}
		}
		else
			Cube_make(ch);
		break;

	default:
		return;
	}
}
#endif

ACMD(do_dice)
{
	char arg1[256], arg2[256];
	int start = 1, end = 100;

	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (*arg1 && *arg2)
	{
		start = atoi(arg1);
		end = atoi(arg2);
	}
	else if (*arg1 && !*arg2)
	{
		start = 1;
		end = atoi(arg1);
	}

	end = MAX(start, end);
	start = MIN(start, end);

	int n = number(start, end);

#ifdef __DICE_SYSTEM__
	if (ch->GetParty())
		ch->GetParty()->ChatPacketToAllMember(CHAT_TYPE_DICE_INFO, LC_TEXT("%s´ÔÀÌ ÁÖ»çÀ§¸¦ ±¼·Á %d°¡ ³ª¿Ô½À´Ï´Ù. (%d-%d)"), ch->GetName(), n, start, end);
	else
		ch->ChatPacket(CHAT_TYPE_DICE_INFO, LC_TEXT("´ç½ÅÀÌ ÁÖ»çÀ§¸¦ ±¼·Á %d°¡ ³ª¿Ô½À´Ï´Ù. (%d-%d)"), n, start, end);
#else
	if (ch->GetParty())
		ch->GetParty()->ChatPacketToAllMember(CHAT_TYPE_INFO, LC_TEXT("%s´ÔÀÌ ÁÖ»çÀ§¸¦ ±¼·Á %d°¡ ³ª¿Ô½À´Ï´Ù. (%d-%d)"), ch->GetName(), n, start, end);
	else
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´ç½ÅÀÌ ÁÖ»çÀ§¸¦ ±¼·Á %d°¡ ³ª¿Ô½À´Ï´Ù. (%d-%d)"), n, start, end);
#endif
}

ACMD(do_click_safebox)
{
	if ((ch->GetGMLevel() <= GM_PLAYER) && (ch->GetDungeon() || ch->GetWarMap() || IS_DUNGEON_ZONE(ch->GetMapIndex())))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You cannot open the safebox in dungeon or at war."));
		return;
	}
	if (ch->IsDead() || ch->IsStun())
		return;

	if (quest::CQuestManager::instance().GetPCForce(ch->GetPlayerID())->IsRunning() == true)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_OPERATION_IS_FAILED_QUEST_WINDOW_IS_OPENED"));
		return;
	}

	if (!ch->CanAct(true, true, VAR_TIME_NONE, nullptr, VAR_MYSHOP | VAR_REFINE | VAR_ACCE | VAR_CHANGELOOK | VAR_OFFLINESHOP | VAR_EXCHANGE))
		return;

	ch->SetSafeboxOpenPosition();
	ch->ChatPacket(CHAT_TYPE_COMMAND, "ShowMeSafeboxPassword");
}

ACMD(do_force_logout)
{
	LPDESC pDesc = DESC_MANAGER::instance().FindByCharacterName(ch->GetName());
	if (!pDesc)
		return;
	pDesc->DelayedDisconnect(0);
}

ACMD(do_click_mall)
{
	ch->ChatPacket(CHAT_TYPE_COMMAND, "ShowMeMallPassword");
}
#ifdef __MOUNT_COSTUME_SYSTEM__
ACMD(do_ride)
{
	if (ch->IsDead() || ch->IsStun())
		return;

	// balik tutarken ata binmicek.
	LPITEM rod = ch->GetWear(WEAR_WEAPON);
	if (rod && rod->GetType() == ITEM_ROD)
		return;

	if (ch->GetWear(WEAR_COSTUME_MOUNT))
	{
		CMountSystem* mountSystem = ch->GetMountSystem();
		LPITEM mount = ch->GetWear(WEAR_COSTUME_MOUNT);
		DWORD mobVnum = 0;
		if (!mountSystem || !mount)
			return;
#ifdef __MOUNT_CHANGELOOK__
		TItemTable* pkItemTransmutation = ITEM_MANAGER::instance().GetTable(mount->GetTransmutation());
		if (pkItemTransmutation)
		{
			for (int i = 0; i < ITEM_APPLY_MAX_NUM; ++i)
			{
				BYTE bType = pkItemTransmutation->aApplies[i].bType;
				long value = pkItemTransmutation->aApplies[i].lValue;
				if (bType == APPLY_MOUNT)
					mobVnum = value;
			}
		}
		else
		{
			if (mount->FindApplyValue(APPLY_MOUNT) != 0)
				mobVnum = mount->FindApplyValue(APPLY_MOUNT);
		}
#else
		if (mount->FindApplyValue(APPLY_MOUNT) != 0)
			mobVnum = mount->FindApplyValue(APPLY_MOUNT);
#endif
		if (ch->GetMountVnum())
		{
			if (mountSystem->CountSummoned() == 0)
				mountSystem->Unmount(mobVnum);
		}
		else
		{
			if (mountSystem->CountSummoned() == 1)
				mountSystem->Mount(mobVnum, mount);
		}

		return;
	}
	if (ch->IsHorseRiding())
	{
		ch->StopRiding();
		return;
	}

	if (ch->GetHorse() != NULL)
	{
		ch->StartRiding();
		return;
	}

	for (BYTE i = 0; i < INVENTORY_MAX_NUM; ++i)
	{
		LPITEM item = ch->GetInventoryItem(i);
		if (NULL == item)
			continue;

		if (item->GetType() == ITEM_COSTUME && item->GetSubType() == COSTUME_MOUNT) {
			ch->UseItem(TItemPos(INVENTORY, i));
			return;
		}
	}

	ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¸»À» ¸ÕÀú ¼ÒÈ¯ÇØÁÖ¼¼¿ä."));
}
#else
ACMD(do_ride)
{
	if (test_server)
		sys_log(0, "[DO_RIDE] start");

	if (ch->IsDead() || ch->IsStun())
		return;

	if (ch->IsHorseRiding())
	{
		if (test_server)
			sys_log(0, "[DO_RIDE] stop riding");
		ch->StopRiding();
		return;
	}

	if (ch->GetMountVnum())
	{
		if (test_server)
			sys_log(0, "[DO_RIDE] unmount");
		do_unmount(ch, NULL, 0, 0);
		return;
	}

	if (ch->GetHorse() != NULL)
	{
		if (test_server)
			sys_log(0, "[DO_RIDE] start riding");
		ch->StartRiding();
		return;
	}

	for (BYTE i = 0; i < INVENTORY_MAX_NUM; ++i)
	{
		LPITEM item = ch->GetInventoryItem(i);
		if (NULL == item)
			continue;

		if (item->IsRideItem())
		{
			if (
				NULL == ch->GetWear(WEAR_UNIQUE1)
				|| NULL == ch->GetWear(WEAR_UNIQUE2)
#ifdef __MOUNT_COSTUME_SYSTEM__
				|| NULL == ch->GetWear(WEAR_COSTUME_MOUNT)
#endif
				)
			{
				if (test_server)
					sys_log(0, "[DO_RIDE] USE UNIQUE ITEM");
				//ch->EquipItem(item);
				ch->UseItem(TItemPos(INVENTORY, i));
				return;
			}
		}

		switch (item->GetVnum())
		{
		case 71114:
		case 71116:
		case 71118:
		case 71120:
			if (test_server)
				sys_log(0, "[DO_RIDE] USE QUEST ITEM");
			ch->UseItem(TItemPos(INVENTORY, i));
			return;
		}

		if ((item->GetVnum() > 52000) && (item->GetVnum() < 52091)) {
			if (test_server)
				sys_log(0, "[DO_RIDE] USE QUEST ITEM");
			ch->UseItem(TItemPos(INVENTORY, i));
			return;
		}
	}

	ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¸»À» ¸ÕÀú ¼ÒÈ¯ÇØÁÖ¼¼¿ä."));
}
#endif
#ifdef __CHANNEL_CHANGER__
ACMD(do_change_channel)
{
	if (!ch)
		return;

	if (quest::CQuestManager::instance().GetPCForce(ch->GetPlayerID())->IsRunning() == true)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_OPERATION_IS_FAILED_QUEST_WINDOW_IS_OPENED"));
		return;
	}

	if (!ch->CanAct(true, true, VAR_TIME_CHANNEL_CHANGE))
		return;

	if (ch->m_pkTimedEvent)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Ae¨ùO ¥ìC¨úu¨öA¢¥I¢¥U."));
		event_cancel(&ch->m_pkTimedEvent);
		return;
	}

	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Usage: channel <new channel>"));
		return;
	}

	short channel;
	str_to_number(channel, arg1);

	if (channel < 0 || channel > CHANNEL_MAX_COUNT)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Please enter a valid channel."));
		return;
	}

	if (channel == g_bChannel)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You are already on channel %d."), g_bChannel);
		return;
	}

	if (g_bChannel == 99)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("The map you are at is cross-channel, changing won't have any effect."));
		return;
	}

	if (ch->GetDungeon())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You cannot change channel while in a dungeon."));
		return;
	}

	if (IS_DUNGEON_ZONE(ch->GetMapIndex()))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You cannot change channel while in a dungeon."));
		return;
	}

	TPacketChangeChannel p;
	p.channel = channel;
	p.lMapIndex = ch->GetMapIndex();

	db_clientdesc->DBPacket(HEADER_GD_FIND_CHANNEL, ch->GetDesc()->GetHandle(), &p, sizeof(p));
}
#endif

#ifdef __SKILLBOOK_SYSTEM__
LPEVENT bktimer = NULL;

EVENTINFO(TMainEventInfo2)
{
	LPCHARACTER	kim;
	long skillindexx;

	TMainEventInfo2()
		: kim(NULL)
		, skillindexx(0)
	{
	}
};

EVENTFUNC(bk_event)
{
	TMainEventInfo2* info = dynamic_cast<TMainEventInfo2*>(event->info);

	if (info == NULL)
	{
		sys_err("ruh_event> <Factor> Null pointer");
		return 0;
	}

	LPCHARACTER	ch = info->kim;
	long skillindex = info->skillindexx;

	if (!ch || skillindex == 0)
		return 0;

	if (!ch->GetDesc())
		return 0;

	if (!ch->CanAct(true, true, VAR_TIME_READ_SKILLBOOK, nullptr, 0))
		return 0;

	if (ch->CountSpecifyItem(50300) < 1)
		return 0;

	int skilllevel = ch->GetSkillLevel(skillindex);
	if (skilllevel >= 30)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("BK_SYSTEM_SKILL_FINISH"));
		return 0;
	}

	int dwVnum = ch->BKBul(skillindex);
	if (dwVnum == 999)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("BK_SYSTEM_NOT_SKILL"));
		return 0;
	}

#ifdef __ADDITIONAL_INVENTORY__
	LPITEM item = ch->GetBookInventoryItem(dwVnum);
#else
	LPITEM item = ch->GetInventoryItem(dwVnum);
#endif

	if (!item)
		return 0;

	if (item->GetVnum() != 50300 || item->GetSocket(0) != skillindex)
		return 0;

	if (ch->CountSpecifyItem(71001) > 0)
	{
		if (!ch->IsAffectFlag(AFFECT_SKILL_NO_BOOK_DELAY))
		{
			ch->AddAffect(AFFECT_SKILL_NO_BOOK_DELAY, aApplyInfo[0].bPointType, 0, 0, 536870911, 0, false);
			ch->RemoveSpecifyItem(71001, 1);
		}
	}

	if (ch->CountSpecifyItem(71001) < 1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("BK_SYSTEM_NOT_KR_ITEM"));
		return 0;
	}

	if (ch->CountSpecifyItem(71094) >= 1)
	{
		if (!ch->IsAffectFlag(AFFECT_SKILL_BOOK_BONUS))
		{
			ch->AddAffect(AFFECT_SKILL_BOOK_BONUS, aApplyInfo[0].bPointType, 0, 0, 536870911, 0, false);
			ch->RemoveSpecifyItem(71094, 1);
		}
	}

	if (item->GetVnum() == 50300)
	{
		if (true == ch->LearnSkillByBook(skillindex))
		{
			if (item->GetVnum() == 50300 && item->GetSocket(0) == skillindex)
			{
				item->SetCount(item->GetCount() - 1);
				int iReadDelay;
				iReadDelay = number(SKILLBOOK_DELAY_MIN, SKILLBOOK_DELAY_MAX);
				ch->SetSkillNextReadTime(skillindex, get_global_time() + iReadDelay);
			}
		}
		else
		{
			if (item->GetVnum() == 50300 && item->GetSocket(0) == skillindex)
			{
				item->SetCount(item->GetCount() - 1);
				int iReadDelay;
				iReadDelay = number(SKILLBOOK_DELAY_MIN, SKILLBOOK_DELAY_MAX);
				ch->SetSkillNextReadTime(skillindex, get_global_time() + iReadDelay);
			}
		}
	}
	return 1;
}
ACMD(do_bkoku)
{
	char arg1[256], arg2[256];
	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));
	if (!*arg1 || !*arg2)
		return;

	int gelen = 0;
	long skillindex = 0;

	str_to_number(gelen, arg1);
	str_to_number(skillindex, arg2);

	if (gelen < 0 || skillindex < 0)
		return;

	if (!ch)
		return;

	if (!ch->IsPC())
		return;

	if (ch->IsDead() || ch->IsStun())
		return;

	if (quest::CQuestManager::instance().GetPCForce(ch->GetPlayerID())->IsRunning() == true)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_OPERATION_IS_FAILED_QUEST_WINDOW_IS_OPENED"));
		return;
	}

	if (!ch->CanAct(true, true, VAR_TIME_READ_SKILLBOOK, nullptr, 0))
		return;

	int skillgrup = ch->GetSkillGroup();
	if (skillgrup == 0)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("BK_SYSTEM_SKILL_NOT_N"));
		return;
	}

	if (ch->CountSpecifyItem(50300) < 1)
		return;

	int skilllevel = ch->GetSkillLevel(skillindex);

	if (skilllevel >= 30)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("BK_SYSTEM_SKILL_MAXLEVEL"));
		return;
	}

	if (gelen == 1)
	{
		if (ch->GetQuestFlag("bk.yenisure") > get_global_time())
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("BK_SYSTEM_TIME_OUT_ONE_SECOND"));
			return;
		}

		int dwVnum = ch->BKBul(skillindex);
		if (dwVnum == 999)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("BK_SYSTEM_NOT_SKILL"));
			return;
		}

#ifdef __ADDITIONAL_INVENTORY__
		LPITEM item = ch->GetBookInventoryItem(dwVnum);
#else
		LPITEM item = ch->GetInventoryItem(dwVnum);
#endif

		if (!item)
			return;

		if (item->GetVnum() != 50300 || item->GetSocket(0) != skillindex)
			return;

		if (ch->CountSpecifyItem(71001) > 0)
		{
			if (!ch->IsAffectFlag(AFFECT_SKILL_NO_BOOK_DELAY))
			{
				ch->AddAffect(AFFECT_SKILL_NO_BOOK_DELAY, aApplyInfo[0].bPointType, 0, 0, 536870911, 0, false);
				ch->RemoveSpecifyItem(71001, 1);
			}
		}

		if (ch->CountSpecifyItem(71094) >= 1)
		{
			if (!ch->IsAffectFlag(AFFECT_SKILL_BOOK_BONUS))
			{
				ch->AddAffect(AFFECT_SKILL_BOOK_BONUS, aApplyInfo[0].bPointType, 0, 0, 536870911, 0, false);
				ch->RemoveSpecifyItem(71094, 1);
			}
		}

		if (item->GetVnum() == 50300)
		{
			if (ch->FindAffect(AFFECT_SKILL_NO_BOOK_DELAY) == false && ch->CountSpecifyItem(71001) < 1)
			{
				ch->SkillLearnWaitMoreTimeMessage(ch->GetSkillNextReadTime(skillindex) - get_global_time());
				return;
			}

			if (true == ch->LearnSkillByBook(skillindex))
			{
				if (item->GetVnum() == 50300 && item->GetSocket(0) == skillindex)
				{
					item->SetCount(item->GetCount() - 1);
					int iReadDelay;
					iReadDelay = number(SKILLBOOK_DELAY_MIN, SKILLBOOK_DELAY_MAX);
					ch->SetSkillNextReadTime(skillindex, get_global_time() + iReadDelay);
				}
			}
			else
			{
				if (item->GetVnum() == 50300 && item->GetSocket(0) == skillindex)
				{
					item->SetCount(item->GetCount() - 1);
					int iReadDelay;
					iReadDelay = number(SKILLBOOK_DELAY_MIN, SKILLBOOK_DELAY_MAX);
					ch->SetSkillNextReadTime(skillindex, get_global_time() + iReadDelay);
				}
			}
			ch->SetQuestFlag("bk.yenisure", get_global_time() + 1);
		}
	}
	else if (gelen == 0) ///hepsi
	{
		if (bktimer)
			event_cancel(&bktimer);

		TMainEventInfo2* info = AllocEventInfo<TMainEventInfo2>();
		info->kim = ch;
		info->skillindexx = skillindex;
		bktimer = event_create(bk_event, info, PASSES_PER_SEC(1));
	}

	return;
}
#endif

#ifdef __SOULSTONE_SYSTEM__
LPEVENT ruhtimer = NULL;

EVENTINFO(TMainEventInfo)
{
	LPCHARACTER	kim;
	long skillindexx;
	int count;

	TMainEventInfo()
		: kim(NULL)
		, skillindexx(0)
		, count(0)
	{
	}
};

EVENTFUNC(ruh_event)
{
	TMainEventInfo* info = dynamic_cast<TMainEventInfo*>(event->info);

	if (info == NULL)
	{
		sys_err("ruh_event> <Factor> Null pointer");
		return 0;
	}

	LPCHARACTER	ch = info->kim;
	long skillindex = info->skillindexx;

	if (!ch || skillindex == 0)
		return 0;

	if (!ch->GetDesc())
		return 0;

	if (!ch->IsPC())
		return 0;

	if (ch->IsDead() || ch->IsStun())
		return 0;

	if (!ch->CanAct(true, true, VAR_TIME_READ_SOULSTONE, nullptr, 0))
		return 0;

	int skillLevel = ch->GetSkillLevel(skillindex)-20;
	int skillGrade = ch->GetSkillMasterType(skillindex);
	// int skilllevel = ch->GetSkillLevel(skillindex);

	DWORD ruhtasiVnum, munzeviVnum, koturuhVnum, munzeviAffectType;
	int needAlign;

	switch (skillGrade)
	{
	case SKILL_GRAND_MASTER:
		ruhtasiVnum = 50513;
		munzeviVnum = 71094;
		munzeviAffectType = AFFECT_SKILL_BOOK_BONUS;
		koturuhVnum = 71001;
		skillLevel -= 10;

		needAlign = (1000 + 500 * skillLevel);
		break;
	case SKILL_PERFECT_MASTER:
		ruhtasiVnum = SKILL_OVER_P_ITEM_VNUM;
		koturuhVnum = SKILL_OVER_P_KOTU_RUH_VNUM;
		munzeviVnum = SKILL_OVER_P_MUNZEVI_VNUM;
		munzeviAffectType = AFFECT_SKILL_BOOK_BONUS2;
		skillLevel -= 20;

		needAlign = 3 * (1000 + 500 * skillLevel);
		break;
	case SKILL_SAGE_MASTER:
		ruhtasiVnum = SKILL_EXPERT_ITEM_VNUM;
		koturuhVnum = SKILL_EXPERT_KOTU_RUH_VNUM;
		munzeviVnum = SKILL_EXPERT_MUNZEVI_VNUM;
		munzeviAffectType = AFFECT_SKILL_BOOK_BONUS_EXPERT;
		skillLevel -= 30;

		needAlign = 3 * (1500 + 700 * (skillLevel + 10));
		break;
	case SKILL_EXPERT_MASTER:
		ruhtasiVnum = SKILL_DESTANSI_ITEM_VNUM;
		koturuhVnum = SKILL_DESTANSI_KOTU_RUH_VNUM;
		munzeviVnum = SKILL_DESTANSI_MUNZEVI_VNUM;
		munzeviAffectType = AFFECT_SKILL_BOOK_BONUS_DESTANSI;
		skillLevel -= 40;

		needAlign = 3 * (1750 + 850 * (skillLevel + 20));
		break;
	case SKILL_DESTANSI_MASTER:
		ruhtasiVnum = SKILL_TANRISAL_ITEM_VNUM;
		koturuhVnum = SKILL_TANRISAL_KOTU_RUH_VNUM;
		munzeviVnum = SKILL_TANRISAL_MUNZEVI_VNUM;
		munzeviAffectType = AFFECT_SKILL_BOOK_BONUS_TANRISAL;
		skillLevel -= 50;

		needAlign = 3 * (2000 + 1000 * (skillLevel + 20));
		break;

	default:
		return 0;
	}

	if (ch->CountSpecifyItem(ruhtasiVnum) < 1)
		return 0;

	if (ch->GetRealAlignment() < 0)
	{
		int needItemCount = ch->GetRealAlignment() / -needAlign;
		if (ch->CountSpecifyItem(70102) < needItemCount) {
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ruhzenbitti %d"), needItemCount);
			return 0;
		}
		ch->RemoveSpecifyItem(70102, needItemCount);
		ch->UpdateAlignment(needItemCount * needAlign);
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ruhzenbastim"));
	}

	if (ch->GetQuestFlag("ruh.sure") > get_global_time())
	{
		if (ch->CountSpecifyItem(koturuhVnum) < 1)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ruhsuredolmadi"));
			return 0;
		}
		else
			ch->RemoveSpecifyItem(koturuhVnum, 1);
	}

	if (ch->CountSpecifyItem(munzeviVnum) >= 1)
	{
		ch->AddAffect(munzeviAffectType, aApplyInfo[0].bPointType, 0, 0, 536870911, 0, false);
		ch->RemoveSpecifyItem(munzeviVnum, 1);
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ruhmunzevikullandim"));
	}

	ch->UpdateAlignment(-needAlign * 10);

	switch (skillGrade)
	{
	case SKILL_GRAND_MASTER:
		ch->LearnGrandMasterSkill(skillindex);
		break;
	case SKILL_PERFECT_MASTER:
		ch->LearnSageMasterSkill(skillindex);
		break;
	case SKILL_SAGE_MASTER:
		ch->LearnExpertMasterSkill(skillindex);
		break;
	case SKILL_EXPERT_MASTER:
		ch->LearnDestansiMasterSkill(skillindex);
		break;
	case SKILL_DESTANSI_MASTER:
		ch->LearnTanrisalMasterSkill(skillindex);
		break;
	}

	ch->RemoveSpecifyItem(ruhtasiVnum, 1);
	ch->SetQuestFlag("ruh.sure", get_global_time() + 60 * 60 * 24);

	info->count--;

	if (info->count)
		return 1;
	else
		return 0;
}

ACMD(do_ruhoku)
{
	char arg1[256], arg2[256];
	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (!*arg1 || !*arg2)
		return;

	int count = 0;
	long skillindex = 0;

	str_to_number(count, arg1);
	str_to_number(skillindex, arg2);

	if (count < 0 || skillindex < 0)
		return;

	if (!ch)
		return;

	if (!ch->IsPC())
		return;

	if (ch->IsDead() || ch->IsStun())
		return;

	if (quest::CQuestManager::instance().GetPCForce(ch->GetPlayerID())->IsRunning() == true)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_OPERATION_IS_FAILED_QUEST_WINDOW_IS_OPENED"));
		return;
	}

	if (!ch->CanAct(true, true, VAR_TIME_READ_SOULSTONE, nullptr, 0))
		return;

	LPITEM slot1 = ch->GetWear(WEAR_UNIQUE1);
	LPITEM slot2 = ch->GetWear(WEAR_UNIQUE2);

	if (NULL != slot1)
	{
		if (slot1->GetVnum() == 70048)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("pelerin_cikar"));
			return;
		}
	}

	if (NULL != slot2)
	{
		if (slot2->GetVnum() == 70048)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("pelerin_cikar"));
			return;
		}
	}

	int skillgrup = ch->GetSkillGroup();
	if (skillgrup == 0)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ruhokuyamazsin"));
		return;
	}

	if (ruhtimer)
		event_cancel(&ruhtimer);

	TMainEventInfo* info = AllocEventInfo<TMainEventInfo>();
	info->kim = ch;
	info->skillindexx = skillindex;
	info->count = count;
	ruhtimer = event_create(ruh_event, info, PASSES_PER_SEC(1));

	return;
}
#endif

#ifdef __GROWTH_PET_SYSTEM__
ACMD(do_CubePetAdd)
{
	int pos = 0;
	int invpos = 0;

	const char* line;
	char arg1[256], arg2[256], arg3[256];

	line = two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));
	one_argument(line, arg3, sizeof(arg3));

	if (0 == arg1[0])
		return;
	//const std::string & strArg1 = std::string(arg1);
	switch (LOWER(arg1[0]))
	{
	case 'a':	// add cue_index inven_index
	{
		if (0 == arg2[0] || !isdigit(*arg2) ||
			0 == arg3[0] || !isdigit(*arg3))
			return;

		str_to_number(pos, arg2);
		str_to_number(invpos, arg3);
	}
	break;

	default:
		return;
	}

	if (pos < 0 || invpos < 0)
	{
		LogManager::instance().HackLogEx(ch, "do_CubePetAdd trying crash game core... (pos_invpos)");
		return;
	}

	if (quest::CQuestManager::instance().GetPCForce(ch->GetPlayerID())->IsRunning() == true)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_OPERATION_IS_FAILED_QUEST_WINDOW_IS_OPENED"));
		return;
	}

	if (!ch->CanAct(true, true, VAR_TIME_PET_ACTION, nullptr, 0))
		return;

	if (ch->GetNewPetSystem()->IsActivePet())
		ch->GetNewPetSystem()->SetItemCube(pos, invpos);
	else
		return;
}

ACMD(do_PetSkill)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));
	if (!*arg1)
		return;

	DWORD skillslot = 0;
	str_to_number(skillslot, arg1);
	if (skillslot > 2 || skillslot < 0)
		return;

	if (quest::CQuestManager::instance().GetPCForce(ch->GetPlayerID())->IsRunning() == true)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_OPERATION_IS_FAILED_QUEST_WINDOW_IS_OPENED"));
		return;
	}

	if (ch->GetNewPetSystem()->GetLevel() < 40)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Petin leveli henüz hazýr deðil."));
		return;
	}

	if (!ch->CanAct(true, true, VAR_TIME_PET_ACTION, nullptr, 0))
		return;

	if (ch->GetNewPetSystem()->IsActivePet())
		ch->GetNewPetSystem()->DoPetSkill(skillslot);
	else
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Devam etmek icin petini cagir.!"));
}

ACMD(do_FeedCubePet)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));
	if (!*arg1)
		return;

	DWORD feedtype = 0;
	str_to_number(feedtype, arg1);

	if (feedtype > 3)
	{
		LogManager::instance().HackLogEx(ch, "do_FeedCubePet trying crash game core... (feedtype)");
		return;
	}

	if (quest::CQuestManager::instance().GetPCForce(ch->GetPlayerID())->IsRunning() == true)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_OPERATION_IS_FAILED_QUEST_WINDOW_IS_OPENED"));
		return;
	}

	if (!ch->CanAct(true, true, VAR_TIME_PET_ACTION, nullptr, 0))
		return;

	if (ch->GetNewPetSystem()->IsActivePet())
		ch->GetNewPetSystem()->ItemCubeFeed(feedtype);
	else
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Once petini cagir!"));
}

ACMD(do_PetEvo)
{
	if (quest::CQuestManager::instance().GetPCForce(ch->GetPlayerID())->IsRunning() == true)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_OPERATION_IS_FAILED_QUEST_WINDOW_IS_OPENED"));
		return;
	}

	if (!ch->CanAct(true, true, VAR_TIME_PET_ACTION, nullptr, 0))
		return;

	if (ch->GetNewPetSystem()->IsActivePet()) {
		int it[3][1] = {
			{ 55003, }, //Here Modify Items to request for 1 evo
			{ 55004, }, //Here Modify Items to request for 2 evo
			{ 55005}  //Here Modify Items to request for 3 evo
		};
		int ic[3][1] = { { 20},
		{ 30},
		{ 40}
		};
		int tmpevo = ch->GetNewPetSystem()->GetEvolution();

		if ((ch->GetNewPetSystem()->GetLevel() == 40 && tmpevo == 0) ||
			(ch->GetNewPetSystem()->GetLevel() == 60 && tmpevo == 1) ||
			(ch->GetNewPetSystem()->GetLevel() == 80 && tmpevo == 2)) {
			for (int b = 0; b < 1; b++) {
				if (ch->CountSpecifyItem(it[tmpevo][b]) < ic[tmpevo][b]) {
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[Pet Gelistirme] Gereken itemler:"));
					for (int c = 0; c < 1; c++) {
						DWORD vnum = it[tmpevo][c];
						ch->ChatPacket(CHAT_TYPE_INFO, "%s X%d", ITEM_MANAGER::instance().GetTable(vnum)->szLocaleName, ic[tmpevo][c]);
					}
					return;
				}
			}
			for (int c = 0; c < 1; c++) {
				ch->RemoveSpecifyItem(it[tmpevo][c], ic[tmpevo][c]);
			}
			ch->GetNewPetSystem()->IncreasePetEvolution();
		}
		else {
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Suan evcil hayvanin gelistirilemez!"));
			return;
		}
	}
	else
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Zaten evcil hayvanin var!"));
}

ACMD(do_PetChangeName)
{
	char arg1[256], arg2[256];
	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (!*arg1 || !*arg2)
		return;

	int bCell = 0;
	str_to_number(bCell, arg1);

	if (bCell < 0)
	{
		LogManager::instance().HackLogEx(ch, "do_PetChangeName trying crash game core... (bCell)");
		return;
	}

	if (quest::CQuestManager::instance().GetPCForce(ch->GetPlayerID())->IsRunning() == true)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_OPERATION_IS_FAILED_QUEST_WINDOW_IS_OPENED"));
		return;
	}

	if (!ch->CanAct(true, true, VAR_TIME_PET_ACTION, nullptr, 0))
		return;

	LPITEM item = ch->GetInventoryItem(bCell);
	if (!item)
		return;
	if (ch->CountSpecifyItem(55008) < 1)
		return;

	if (strchr(arg2, '%') ||
		strchr(arg2, '/') ||
		strchr(arg2, '>') ||
		strchr(arg2, '|') ||
		strchr(arg2, ';') ||
		strchr(arg2, ':') ||
		strchr(arg2, '}') ||
		strchr(arg2, '{') ||
		strchr(arg2, '[') ||
		strchr(arg2, ']') ||
		strchr(arg2, '%') ||
		strchr(arg2, '#') ||
		strchr(arg2, '@') ||
		strchr(arg2, '^') ||
		strchr(arg2, '&') ||
		strchr(arg2, '"')
		)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[Pet-Kulucka] Hatali isim girdiniz"));
		return;
	}

	if (ch->GetNewPetSystem()->IsActivePet())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Once petini gonder. "));
		return;
	}

	char szName[CHARACTER_NAME_MAX_LEN + 1];
	DBManager::instance().EscapeString(szName, sizeof(szName), arg2, strlen(arg2));

	std::unique_ptr<SQLMsg> pMsg(DBManager::instance().DirectQuery("UPDATE new_petsystem SET name = '%s' WHERE id = '%lu';", szName, item->GetID()));
	ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Pet Ismi Basarili Bir Sekilde Degistirildi!"));
	ch->RemoveSpecifyItem(55008, 1);
}

ACMD(do_IncreasePetSkill)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));
	int itempos = atoi(arg1);

	if (itempos < 0)
	{
		LogManager::instance().HackLogEx(ch, "do_IncreasePetSkill trying crash game core... (itempos)");
		return;
	}

	if (quest::CQuestManager::instance().GetPCForce(ch->GetPlayerID())->IsRunning() == true)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_OPERATION_IS_FAILED_QUEST_WINDOW_IS_OPENED"));
		return;
	}

	if (!ch->CanAct(true, true, VAR_TIME_PET_ACTION, nullptr, 0))
		return;

#ifdef __ADDITIONAL_INVENTORY__
	LPITEM item = ch->GetBookInventoryItem(itempos);
#else
	LPITEM item = ch->GetInventoryItem(itempos);
#endif
	if (!item)
		return;

	int skill = item->GetValue(0);
	CNewPetSystem* petSystem = ch->GetNewPetSystem();
	if (!petSystem)
		return;
	if (item->GetVnum() < 55010 && item->GetVnum() > 55016)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("PET_UNKNOWN_SKILL"));
		return;
	}
	bool ret = petSystem->IncreasePetSkill(skill);
	if (ret)
		item->SetCount(item->GetCount() - 1);
}

ACMD(do_determine_pet)
{
	if (ch->CountSpecifyItem(55032) < 1)
		return;

	if (quest::CQuestManager::instance().GetPCForce(ch->GetPlayerID())->IsRunning() == true)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_OPERATION_IS_FAILED_QUEST_WINDOW_IS_OPENED"));
		return;
	}

	if (!ch->CanAct(true, true, VAR_TIME_PET_ACTION, nullptr, 0))
		return;

	int newpettype = number(0, 7);

	if (ch->GetNewPetSystem()->IsActivePet())
	{
		if (ch->GetNewPetSystem()->GetLevel() != 1)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("PET_BELIRLEME_1_LEVELDE_KULLANILABILIR."));
			return;
		}

		ch->RemoveSpecifyItem(55032, 1);
		ch->GetNewPetSystem()->SetPetType(newpettype);
		ch->ChatPacket(CHAT_TYPE_COMMAND, "OnResultPetAttrDetermine %d", newpettype);
	}
}

ACMD(do_change_pet)
{
	char arg1[256], arg2[256], arg3[256], arg4[256];
	four_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2), arg3, sizeof(arg3), arg4, sizeof(arg4));

	if (!*arg1 || !*arg2 || !*arg3 || !*arg4)
		return;

	int firstInvenType = atoi(arg1);
	int firstInvenPos = atoi(arg2);
	int secondInvenType = atoi(arg3);
	int secondInvenPos = atoi(arg4);

	if (firstInvenType < 0 || secondInvenType < 0)
		return;

	if (firstInvenType == INVENTORY)
	{
		if (firstInvenPos >= INVENTORY_MAX_NUM)
			return;
	}
	else
	{
		if (firstInvenPos >= SPECIAL_INVENTORY_MAX_NUM)
			return;
	}

	if (secondInvenType == INVENTORY)
	{
		if (secondInvenPos >= INVENTORY_MAX_NUM)
			return;
	}
	else
	{
		if (secondInvenPos >= SPECIAL_INVENTORY_MAX_NUM)
			return;
	}

	LPITEM petItem = ch->GetItem(TItemPos(firstInvenType, firstInvenPos));
	LPITEM secondItem = ch->GetItem(TItemPos(secondInvenType, secondInvenPos));

	if (!petItem || !secondItem)
		return;

	if (secondItem->GetVnum() != 55033)
		return;

	if (ch->GetNewPetSystem()->IsActivePet())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("AKTIF_BIR_PETI_EFSUNLAYAMAZSIN."));
		return;
	}

	if (petItem->GetAttributeType(0) != 1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("PETI_SADECE_1_LVLDE_EFSUNLAYABILIRSIN."));
		return;
	}

	if (quest::CQuestManager::instance().GetPCForce(ch->GetPlayerID())->IsRunning() == true)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_OPERATION_IS_FAILED_QUEST_WINDOW_IS_OPENED"));
		return;
	}

	if (!ch->CanAct(true, true, VAR_TIME_PET_ACTION, nullptr, 0))
		return;

	int	m_dwbonuspet[3][2];

	for (int x = 0; x < 3; ++x) //Inizializzazione bonus del pet
	{
		int btype[3] = { APPLY_MAX_HP, APPLY_DEF_GRADE_BONUS, APPLY_ATT_GRADE_BONUS };
		m_dwbonuspet[x][0] = btype[x];
		m_dwbonuspet[x][1] = 0;
	}

	int newpettype = petItem->GetAttributeType(1);
	int artis = petItem->GetAttributeType(2);

	for (int i = 0; i < 3; ++i)
	{
		m_dwbonuspet[i][1] = number(10, 45);
	}

	secondItem->SetCount(secondItem->GetCount() - 1);

	std::unique_ptr<SQLMsg> pMsg(DBManager::instance().DirectQuery("UPDATE new_petsystem SET bonus0 = '%d', bonus1 = '%d', bonus2 = '%d', pet_type = '%d', artis = '%d' WHERE id = '%lu';", m_dwbonuspet[0][1], m_dwbonuspet[1][1], m_dwbonuspet[2][1], newpettype, artis, petItem->GetID()));
	
	petItem->SetForceAttribute(0, petItem->GetAttributeType(0), m_dwbonuspet[0][1]);
	petItem->SetForceAttribute(1, petItem->GetAttributeType(1), m_dwbonuspet[1][1]);
	petItem->SetForceAttribute(2, petItem->GetAttributeType(2), m_dwbonuspet[2][1]);

	ch->ChatPacket(CHAT_TYPE_COMMAND, "OnResultPetAttrChange %d", newpettype);
}
#endif

#ifdef __GROWTH_MOUNT_SYSTEM__
ACMD(do_CubeMountAdd)
{
	int pos = 0;
	int invpos = 0;

	const char* line;
	char arg1[256], arg2[256], arg3[256];

	line = two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));
	one_argument(line, arg3, sizeof(arg3));

	if (0 == arg1[0])
		return;
	//const std::string & strArg1 = std::string(arg1);
	switch (LOWER(arg1[0]))
	{
	case 'a':	// add cue_index inven_index
	{
		if (0 == arg2[0] || !isdigit(*arg2) ||
			0 == arg3[0] || !isdigit(*arg3))
			return;

		str_to_number(pos, arg2);
		str_to_number(invpos, arg3);
	}
	break;

	default:
		return;
	}

	if (pos < 0 || invpos < 0)
	{
		LogManager::instance().HackLogEx(ch, "do_CubeMountAdd trying crash game core... (pos_invpos)");
		return;
	}

	if (quest::CQuestManager::instance().GetPCForce(ch->GetPlayerID())->IsRunning() == true)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_OPERATION_IS_FAILED_QUEST_WINDOW_IS_OPENED"));
		return;
	}

	if (!ch->CanAct(true, true, VAR_TIME_MOUNT_ACTION, nullptr, 0))
		return;

	if (ch->GetNewMountSystem()->IsActiveMount())
		ch->GetNewMountSystem()->SetItemCube(pos, invpos);
	else
		return;
}

ACMD(do_MountSkill)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));
	if (!*arg1)
		return;

	DWORD skillslot = 0;
	str_to_number(skillslot, arg1);
	if (skillslot > 2 || skillslot < 0)
		return;

	if (quest::CQuestManager::instance().GetPCForce(ch->GetPlayerID())->IsRunning() == true)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_OPERATION_IS_FAILED_QUEST_WINDOW_IS_OPENED"));
		return;
	}

	if (ch->GetNewMountSystem()->GetLevel() < 40)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Mountin leveli henüz hazýr deðil."));
		return;
	}

	if (!ch->CanAct(true, true, VAR_TIME_MOUNT_ACTION, nullptr, 0))
		return;

	if (ch->GetNewMountSystem()->IsActiveMount())
		ch->GetNewMountSystem()->DoMountSkill(skillslot);
	else
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Devam etmek icin mountini cagir.!"));
}

ACMD(do_FeedCubeMount)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));
	if (!*arg1)
		return;

	DWORD feedtype = 0;
	str_to_number(feedtype, arg1);

	if (feedtype > 3)
	{
		LogManager::instance().HackLogEx(ch, "do_FeedCubeMount trying crash game core... (feedtype)");
		return;
	}

	if (quest::CQuestManager::instance().GetPCForce(ch->GetPlayerID())->IsRunning() == true)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_OPERATION_IS_FAILED_QUEST_WINDOW_IS_OPENED"));
		return;
	}

	if (!ch->CanAct(true, true, VAR_TIME_MOUNT_ACTION, nullptr, 0))
		return;

	if (ch->GetNewMountSystem()->IsActiveMount())
		ch->GetNewMountSystem()->ItemCubeFeed(feedtype);
	else
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Once Mountini cagir!"));
}

ACMD(do_MountEvo)
{
	if (quest::CQuestManager::instance().GetPCForce(ch->GetPlayerID())->IsRunning() == true)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_OPERATION_IS_FAILED_QUEST_WINDOW_IS_OPENED"));
		return;
	}

	if (!ch->CanAct(true, true, VAR_TIME_MOUNT_ACTION, nullptr, 0))
		return;

	if (ch->GetNewMountSystem()->IsActiveMount()) {
		int it[3][1] = {
			{ 55003, }, //Here Modify Items to request for 1 evo
			{ 55004, }, //Here Modify Items to request for 2 evo
			{ 55005}  //Here Modify Items to request for 3 evo
		};
		int ic[3][1] = { { 20}, { 30}, { 40} };
		int tmpevo = ch->GetNewMountSystem()->GetEvolution();

		if ((ch->GetNewMountSystem()->GetLevel() == 40 && tmpevo == 0) ||
			(ch->GetNewMountSystem()->GetLevel() == 60 && tmpevo == 1) ||
			(ch->GetNewMountSystem()->GetLevel() == 80 && tmpevo == 2)) {
			for (int b = 0; b < 1; b++) {
				if (ch->CountSpecifyItem(it[tmpevo][b]) < ic[tmpevo][b]) {
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[Mount Gelistirme] Gereken itemler:"));
					for (int c = 0; c < 1; c++) {
						DWORD vnum = it[tmpevo][c];
						ch->ChatPacket(CHAT_TYPE_INFO, "%s X%d", ITEM_MANAGER::instance().GetTable(vnum)->szLocaleName, ic[tmpevo][c]);
					}
					return;
				}
			}
			for (int c = 0; c < 1; c++) {
				ch->RemoveSpecifyItem(it[tmpevo][c], ic[tmpevo][c]);
			}
			ch->GetNewMountSystem()->IncreaseMountEvolution();
		}
		else {
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Suan evcil hayvanin gelistirilemez!"));
			return;
		}
	}
	else
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Zaten evcil hayvanin var!"));
}

ACMD(do_MountChangeName)
{
	char arg1[256], arg2[256];
	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (!*arg1 || !*arg2)
		return;

	int bCell = 0;
	str_to_number(bCell, arg1);

	if (bCell < 0)
	{
		LogManager::instance().HackLogEx(ch, "do_MountChangeName trying crash game core... (bCell)");
		return;
	}

	if (quest::CQuestManager::instance().GetPCForce(ch->GetPlayerID())->IsRunning() == true)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_OPERATION_IS_FAILED_QUEST_WINDOW_IS_OPENED"));
		return;
	}

	if (!ch->CanAct(true, true, VAR_TIME_MOUNT_ACTION, nullptr, 0))
		return;

	LPITEM item = ch->GetInventoryItem(bCell);
	if (!item)
		return;
	if (ch->CountSpecifyItem(55098) < 1)
		return;

	if (!strcmp(arg2, "%") ||
		!strcmp(arg2, "/") ||
		!strcmp(arg2, ">") ||
		!strcmp(arg2, "|") ||
		!strcmp(arg2, ";") ||
		!strcmp(arg2, ":") ||
		!strcmp(arg2, "}") ||
		!strcmp(arg2, "{") ||
		!strcmp(arg2, "[") ||
		!strcmp(arg2, "]") ||
		!strcmp(arg2, "%") ||
		!strcmp(arg2, "#") ||
		!strcmp(arg2, "@") ||
		!strcmp(arg2, "^") ||
		!strcmp(arg2, "&") ||
		!strcmp(arg2, "'")
		)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[Mount-Kulucka] Hatali isim girdiniz"));
		return;
	}

	if (ch->GetNewMountSystem()->IsActiveMount())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Once mountini gonder. "));
		return;
	}

	char szName[CHARACTER_NAME_MAX_LEN + 1];
	DBManager::instance().EscapeString(szName, sizeof(szName), arg2, strlen(arg2));

	std::unique_ptr<SQLMsg> pMsg(DBManager::instance().DirectQuery("UPDATE new_mountsystem SET name = '%s' WHERE id = '%lu';", szName, item->GetID()));
	ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Mount Ismi Basarili Bir Sekilde Degistirildi!"));
	ch->RemoveSpecifyItem(55098, 1);
}

ACMD(do_IncreaseMountSkill)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));
	int itempos = atoi(arg1);

	if (itempos < 0)
	{
		LogManager::instance().HackLogEx(ch, "do_IncreaseMountSkill trying crash game core... (itempos)");
		return;
	}

	if (quest::CQuestManager::instance().GetPCForce(ch->GetPlayerID())->IsRunning() == true)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_OPERATION_IS_FAILED_QUEST_WINDOW_IS_OPENED"));
		return;
	}

	if (!ch->CanAct(true, true, VAR_TIME_MOUNT_ACTION, nullptr, 0))
		return;

#ifdef __ADDITIONAL_INVENTORY__
	LPITEM item = ch->GetBookInventoryItem(itempos);
#else
	LPITEM item = ch->GetInventoryItem(itempos);
#endif
	if (!item)
		return;

	int skill = item->GetValue(0);
	CNewMountSystem* MountSystem = ch->GetNewMountSystem();
	if (!MountSystem)
		return;
	if (item->GetVnum() < 55210 && item->GetVnum() > 55216)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("MOUNT_UNKNOWN_SKILL"));
		return;
	}
	bool ret = MountSystem->IncreaseMountSkill(skill);
	if (ret)
		item->SetCount(item->GetCount() - 1);
}

ACMD(do_determine_mount)
{
	if (ch->CountSpecifyItem(55200) < 1)
		return;

	if (quest::CQuestManager::instance().GetPCForce(ch->GetPlayerID())->IsRunning() == true)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_OPERATION_IS_FAILED_QUEST_WINDOW_IS_OPENED"));
		return;
	}

	if (!ch->CanAct(true, true, VAR_TIME_MOUNT_ACTION, nullptr, 0))
		return;

	int newmounttype = number(0, 7);

	if (ch->GetNewMountSystem()->IsActiveMount())
	{
		if (ch->GetNewMountSystem()->GetLevel() != 1)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("MOUNT_BELIRLEME_1_LEVELDE_KULLANILABILIR."));
			return;
		}
		ch->RemoveSpecifyItem(55200, 1);
		ch->GetNewMountSystem()->SetMountType(newmounttype);
		ch->ChatPacket(CHAT_TYPE_COMMAND, "OnResultMountAttrDetermine %d", newmounttype);
	}
}

ACMD(do_change_mount)
{
	char arg1[256], arg2[256], arg3[256], arg4[256];
	four_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2), arg3, sizeof(arg3), arg4, sizeof(arg4));

	if (!*arg1 || !*arg2 || !*arg3 || !*arg4)
		return;

	int firstInvenType = atoi(arg1);
	int firstInvenPos = atoi(arg2);
	int secondInvenType = atoi(arg3);
	int secondInvenPos = atoi(arg4);

	if (firstInvenType < 0 || secondInvenType < 0)
		return;

	if (firstInvenType == INVENTORY)
	{
		if (firstInvenPos >= INVENTORY_MAX_NUM)
			return;
	}
	else
	{
		if (firstInvenPos >= SPECIAL_INVENTORY_MAX_NUM)
			return;
	}

	if (secondInvenType == INVENTORY)
	{
		if (secondInvenPos >= INVENTORY_MAX_NUM)
			return;
	}
	else
	{
		if (secondInvenPos >= SPECIAL_INVENTORY_MAX_NUM)
			return;
	}

	LPITEM mountItem = ch->GetItem(TItemPos(firstInvenType, firstInvenPos));
	LPITEM secondItem = ch->GetItem(TItemPos(secondInvenType, secondInvenPos));

	if (!mountItem || !secondItem)
		return;

	if (secondItem->GetVnum() != 55201)
		return;

	if (ch->GetNewMountSystem()->IsActiveMount())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("AKTIF_BIR_MOUNTI_EFSUNLAYAMAZSIN."));
		return;
	}

	if (mountItem->GetAttributeType(0) != 1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("MOUNTI_SADECE_1_LVLDE_EFSUNLAYABILIRSIN."));
		return;
	}

	if (quest::CQuestManager::instance().GetPCForce(ch->GetPlayerID())->IsRunning() == true)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_OPERATION_IS_FAILED_QUEST_WINDOW_IS_OPENED"));
		return;
	}

	if (!ch->CanAct(true, true, VAR_TIME_MOUNT_ACTION, nullptr, 0))
		return;

	int	m_dwbonusmount[3][2];

	for (int x = 0; x < 3; ++x) //Inizializzazione bonus del mount
	{
		int btype[3] = { APPLY_ATTBONUS_MONSTER, APPLY_ATTBONUS_STONE, APPLY_ATT_GRADE_BONUS };
		m_dwbonusmount[x][0] = btype[x];
		m_dwbonusmount[x][1] = 0;
	}

	int newmounttype = mountItem->GetAttributeType(1);
	int artis = mountItem->GetAttributeType(2);

	for (int i = 0; i < 3; ++i)
	{
		m_dwbonusmount[i][1] = number(10, 45);
	}

	secondItem->SetCount(secondItem->GetCount() - 1);

	std::unique_ptr<SQLMsg> pMsg(DBManager::instance().DirectQuery("UPDATE new_mountsystem SET bonus0 = '%d', bonus1 = '%d', bonus2 = '%d', mount_type = '%d', artis = '%d' WHERE id = '%lu';", m_dwbonusmount[0][1], m_dwbonusmount[1][1], m_dwbonusmount[2][1], newmounttype, artis, mountItem->GetID()));

	mountItem->SetForceAttribute(0, mountItem->GetAttributeType(0), m_dwbonusmount[0][1]);
	mountItem->SetForceAttribute(1, mountItem->GetAttributeType(1), m_dwbonusmount[1][1]);
	mountItem->SetForceAttribute(2, mountItem->GetAttributeType(2), m_dwbonusmount[2][1]);

	ch->ChatPacket(CHAT_TYPE_COMMAND, "OnResultMountAttrChange %d", newmounttype);
}
#endif

#ifdef __ITEM_EVOLUTION__
ACMD(do_refine_rarity)
{
	char arg1[256], arg2[256];
	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));
	if (!*arg1 || !*arg2)
		return;

	if (quest::CQuestManager::instance().GetPCForce(ch->GetPlayerID())->IsRunning() == true)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_OPERATION_IS_FAILED_QUEST_WINDOW_IS_OPENED"));
		return;
	}

	if (!ch->CanAct(true, true, VAR_TIME_REFINE_RARITY, nullptr, 0))
		return;

	BYTE slot = atoi(arg1);
	BYTE slot_scroll = atoi(arg2);

	if (slot < 0 || slot >= INVENTORY_MAX_NUM)
		return;

	if (slot_scroll < 0 || slot_scroll >= INVENTORY_MAX_NUM)
		return;

	LPITEM item = ch->GetInventoryItem(slot);
	LPITEM scroll = ch->GetInventoryItem(slot_scroll);

	if (!item || item->IsExchanging() || item->IsEquipped())
		return;

	if (!scroll || scroll->IsExchanging() || scroll->IsEquipped())
		return;

	if ((item->GetLimitType(0) == 1 && item->GetLimitValue(0) < 75) || item->GetRealEvolution() >= 20)
		return;

	if (item->GetType() != ITEM_WEAPON && (item->GetType() == ITEM_ARMOR && item->GetSubType() != ARMOR_BODY))
		return;

	if ((scroll->GetVnum() >= 56002 && scroll->GetVnum() <= 56007))
		ch->DoRefine_Rarity(item, scroll);
	else if (scroll->GetVnum() == 56011)
		ch->DoRefine_Rarity(item, scroll);
	else if (scroll->GetVnum() == 56014)
		ch->DoRefine_Rarity(item, scroll);
	else if ((scroll->GetVnum() >= 53140 && scroll->GetVnum() <= 53146))
		ch->DoRefine_Rarity(item, scroll);
}
#endif

#ifdef __INGAME_MALL__
ACMD(do_nesne_market)
{
	char arg1[256], arg2[256];
	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	DWORD id = 0;
	DWORD count = 0;

	if (!*arg1 || !*arg2)
		return;

	if (quest::CQuestManager::instance().GetPCForce(ch->GetPlayerID())->IsRunning() == true)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_OPERATION_IS_FAILED_QUEST_WINDOW_IS_OPENED"));
		return;
	}

	if (!ch->CanAct(true, true, VAR_TIME_INGAME_MALL))
		return;

	str_to_number(id, arg1);
	str_to_number(count, arg2);

	bool bRes = CItemShopManager::instance().Buy(ch, id, count); // buy func
	if (bRes)
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("nesnemarketbasarili"));
}
#endif

#ifdef __REMOVE_SKILL_AFFECT__
bool isRemovableSkill(int skill)
{
	switch (skill)
	{
	case AFF_HOSIN:
	case AFF_BOHO:
	case AFF_GICHEON:
	case AFF_KWAESOK:
	case AFF_JEUNGRYEOK:
	case AFF_JEONGWIHON:
	case AFF_GEOMGYEONG:
	case AFF_CHEONGEUN:
	case AFF_EUNHYUNG:
	case AFF_GYEONGGONG:
	case AFF_GWIGUM:
	case AFF_JUMAGAP:
	case AFF_TERROR:
#ifdef __WOLFMAN_CHARACTER__
	case AFF_BLUE_POSSESSION:
#endif
		return true;
	}

	return false;
}

ACMD(do_remove_skill_affect)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	DWORD affectvnum = 0;
	str_to_number(affectvnum, arg1);

	if (!ch->IsAffectFlag(affectvnum))
		return;

	if (!isRemovableSkill(affectvnum))
		return;

	CAffect* pAffect = ch->FindAffectByFlag(affectvnum);
	if (!pAffect)
		return;

	ch->RemoveAffect(pAffect);
}
#endif

#ifdef __SPLIT_ITEMS__
ACMD(do_split_items) //SPLIT ITEMS
{
	if (!ch)
		return;

	const char* line;
	char arg1[256], arg2[256], arg3[256], arg4[256];
	line = two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));
	two_arguments(line, arg3, sizeof(arg3), arg4, sizeof(arg4));

	if (!*arg1 || !*arg2 || !*arg3 || !*arg4)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Wrong command use."));
		return;
	}

	if (quest::CQuestManager::instance().GetPCForce(ch->GetPlayerID())->IsRunning() == true)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_OPERATION_IS_FAILED_QUEST_WINDOW_IS_OPENED"));
		return;
	}

	if (!ch->CanAct(true, true, VAR_TIME_SPLIT_ITEMS))
		return;

	DWORD count = 0; //zamien se to na bajta jak nie masz zwiekszonych slot?.
	WORD cell = 0;
	WORD destCell = 0;
#ifdef __ADDITIONAL_INVENTORY__
	WORD window_type = 0;
#endif

	str_to_number(cell, arg1);
	str_to_number(count, arg2);
	str_to_number(destCell, arg3);
#ifdef __ADDITIONAL_INVENTORY__
	str_to_number(window_type, arg4);
	LPITEM item = ch->GetItem(TItemPos(window_type, cell));
#else
	LPITEM item = ch->GetInventoryItem(cell);
#endif

	if (count == 0)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Wrong command use."));
		return;
	}

	if (item != NULL)
	{
		DWORD itemCount = item->GetCount(); //to tez se kurwa zamien na byte'a jak nie masz slot? zwiekszonych.
		while (itemCount > 0)
		{
			if (count > itemCount)
				count = itemCount;

#ifdef __ADDITIONAL_INVENTORY__
			int iEmptyPosition = -1;
			if (item->IsUpgradeItem())
				iEmptyPosition = ch->GetEmptyUpgradeInventoryFromIndex(destCell, item->GetSize());
			else if (item->IsBook())
				iEmptyPosition = ch->GetEmptyBookInventoryFromIndex(destCell, item->GetSize());
			else if (item->IsStone())
				iEmptyPosition = ch->GetEmptyStoneInventoryFromIndex(destCell, item->GetSize());
			else if (item->IsFlower())
				iEmptyPosition = ch->GetEmptyFlowerInventoryFromIndex(destCell, item->GetSize());
			else if (item->IsAttrItem())
				iEmptyPosition = ch->GetEmptyAttrInventoryFromIndex(destCell, item->GetSize());
			else if (item->IsChest())
				iEmptyPosition = ch->GetEmptyChestInventoryFromIndex(destCell, item->GetSize());
			else if (item->IsDragonSoul())
				iEmptyPosition = -1;
			else
				iEmptyPosition = ch->GetEmptyInventoryFromIndex(destCell, item->GetSize());
#else
			int iEmptyPosition = ch->GetEmptyInventoryFromIndex(destCell, item->GetSize());
#endif
			if (iEmptyPosition == -1)
				break;

			itemCount -= count;
#ifdef __ADDITIONAL_INVENTORY__
			if (item->IsUpgradeItem())
				ch->MoveItem(TItemPos(UPGRADE_INVENTORY, cell), TItemPos(UPGRADE_INVENTORY, iEmptyPosition), count);
			else if (item->IsBook())
				ch->MoveItem(TItemPos(BOOK_INVENTORY, cell), TItemPos(BOOK_INVENTORY, iEmptyPosition), count);
			else if (item->IsStone())
				ch->MoveItem(TItemPos(STONE_INVENTORY, cell), TItemPos(STONE_INVENTORY, iEmptyPosition), count);
			else if (item->IsFlower())
				ch->MoveItem(TItemPos(FLOWER_INVENTORY, cell), TItemPos(FLOWER_INVENTORY, iEmptyPosition), count);
			else if (item->IsAttrItem())
				ch->MoveItem(TItemPos(ATTR_INVENTORY, cell), TItemPos(ATTR_INVENTORY, iEmptyPosition), count);
			else if (item->IsChest())
				ch->MoveItem(TItemPos(CHEST_INVENTORY, cell), TItemPos(CHEST_INVENTORY, iEmptyPosition), count);
			else if (item->IsDragonSoul())
				break;
			else
				ch->MoveItem(TItemPos(INVENTORY, cell), TItemPos(INVENTORY, iEmptyPosition), count);
#else
			ch->MoveItem(TItemPos(INVENTORY, cell), TItemPos(INVENTORY, iEmptyPosition), count);
#endif
		}
	}
}
#endif

#ifdef __BIOLOG_SYSTEM__
ACMD(do_open_biolog)
{
	ch->OpenBiologWindow();
}
ACMD(do_set_biolog_item)
{
	char arg1[256], arg2[256], arg3[256], arg4[256];
	four_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2), arg3, sizeof(arg3), arg4, sizeof(arg4));

	DWORD all = 0;
	str_to_number(all, arg1);
	DWORD use1 = 0;
	str_to_number(use1, arg2);
	DWORD use2 = 0;
	str_to_number(use2, arg3);
	DWORD use3 = 0;
	str_to_number(use3, arg4);

	ch->SetBiologItem(all, use1, use2, use3);
}
#endif

ACMD(do_open_var)
{
	char arg1[256], arg2[256];
	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	BYTE bVarType = 0;
	BYTE bVarValue = 0;
	str_to_number(bVarType, arg1);
	str_to_number(bVarValue, arg2);

	if (bVarType > VAR_VAL_MAX_NUM)
	{
		sys_err("!--> He's hacker (var value manuel send by injector) %s", ch->GetName());
		return;
	}

	ch->SetVarValue(bVarType, bVarValue);
}

#ifdef __INVENTORY_SORT__
ACMD (do_sort_inventory_single)
{
	if (quest::CQuestManager::instance().GetPCForce(ch->GetPlayerID())->IsRunning() == true)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_OPERATION_IS_FAILED_QUEST_WINDOW_IS_OPENED"));
		return;
	}

	if (ch && !ch->CanAct(true, true, VAR_TIME_SORT_INVENTORY))
		return;

	char arg1[256], arg2[256];
	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (!*arg1 || !*arg2)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Wrong command use."));
		return;
	}

	WORD cell = 0;
	WORD window_type = 0;
	str_to_number(cell, arg1);
	str_to_number(window_type, arg2);

	LPITEM item = ch->GetItem(TItemPos(window_type, cell));

	if(!item)
		return;

	if (item->isLocked())
		return;

	if (item->GetCount() == g_bItemCountLimit)
		return;

	if (!item->IsStackable() || IS_SET(item->GetAntiFlag(), ITEM_ANTIFLAG_STACK))
		return;

	size_t invMax;
	switch (window_type)
	{
	case INVENTORY:
		invMax = INVENTORY_MAX_NUM;
		break;
#ifdef __ADDITIONAL_INVENTORY__
	case UPGRADE_INVENTORY:
	case BOOK_INVENTORY:
	case STONE_INVENTORY:
	case FLOWER_INVENTORY:
	case ATTR_INVENTORY:
	case CHEST_INVENTORY:
		invMax = SPECIAL_INVENTORY_MAX_NUM;
		break;
#endif

	default:
		invMax = 0;
		break;
	}

	for (int j = 0; j < invMax; ++j)
	{
		LPITEM item2 = ch->GetItem(TItemPos(window_type, j));

		if (item2 == item)
			continue;

		if(!item2)
			continue;

		if(item2->isLocked())
			continue;

		if (item2->GetVnum() != item->GetVnum())
			continue;

		bool bStopSockets = false;
		for (int k = 0; k < ITEM_SOCKET_MAX_NUM; ++k)
		{
			if (item2->GetSocket(k) != item->GetSocket(k))
			{
				bStopSockets = true;
				break;
			}
		}
		if(bStopSockets)
			continue;

#ifdef __ITEM_COUNT_LIMIT__
		DWORD bAddCount = MIN(g_bItemCountLimit - item->GetCount(), item2->GetCount());
#else
		BYTE bAddCount = MIN(g_bItemCountLimit - item->GetCount(), item2->GetCount());
#endif

		item->SetCount(item->GetCount() + bAddCount);
		item2->SetCount(item2->GetCount() - bAddCount);

		if (item->GetCount() == g_bItemCountLimit)
			return;
	}
}
#endif

#ifdef ENABLE_REMOTE_SHOP_SYSTEM
ACMD(do_open_range_npc)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	DWORD vnum = 0;
	str_to_number(vnum, arg1);

	if (!ch)
		return;

	if (!ch->IsPC())
		return;

	if (ch->IsDead() || ch->IsStun())
		return;

	if (quest::CQuestManager::instance().GetPCForce(ch->GetPlayerID())->IsRunning() == true)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_OPERATION_IS_FAILED_QUEST_WINDOW_IS_OPENED"));
		return;
	}

	if (!ch->CanAct(true, true, VAR_TIME_NONE, nullptr, VAR_REFINE | VAR_SAFEBOX | VAR_EXCHANGE | VAR_ACCE | VAR_CHANGELOOK | VAR_OFFLINESHOP))
		return;

	LPSHOP shop = CShopManager::instance().Get(vnum);
	if (!shop)
		return;

	shop->AddGuest(ch, 0, false, true);
	ch->SetShopOwner(ch);
}
#endif

#ifdef ENABLE_CHAT_COLOR_SYSTEM
ACMD(do_set_chat_color)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	if (!ch)
		return;

#ifdef ENABLE_PB2_PREMIUM_SYSTEM
	auto pkAffect = ch->FindAffect(AFFECT_PB2_GLOBAL_CHAT);
	if (!pkAffect)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("GLOBAL_CHAT_PREMIUM_ONLY"));
		return;
	}
#endif // ENABLE_PB2_PREMIUM_SYSTEM

	BYTE color = 0;
	str_to_number(color, arg1);
	ch->SetChatColor(color);
}
#endif


#ifdef ENABLE_NAME_CHANGE_SYSTEM
ACMD(do_PetChangeNamePremiumPerma)
{
	char arg1[256], arg2[256];
	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (!*arg1 || !*arg2)
		return;

	BYTE bCell = 0;
	str_to_number(bCell, arg1);

	if (bCell < 0)
	{
		LogManager::instance().HackLogEx(ch, "do_PetChangeName trying crash game core... (bCell)");
		return;
	}

	if (quest::CQuestManager::instance().GetPCForce(ch->GetPlayerID())->IsRunning() == true)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_OPERATION_IS_FAILED_QUEST_WINDOW_IS_OPENED"));
		return;
	}

	if (!ch->CanAct(true, true, VAR_TIME_PET_ACTION, nullptr, 0))
		return;

	LPITEM item = ch->GetInventoryItem(bCell);
	if (!item)
		return;
	if (ch->CountSpecifyItem(19103) < 1)
		return;

	if (strchr(arg2, '%') ||
		strchr(arg2, '/') ||
		strchr(arg2, '>') ||
		strchr(arg2, '|') ||
		strchr(arg2, ';') ||
		strchr(arg2, ':') ||
		strchr(arg2, '}') ||
		strchr(arg2, '{') ||
		strchr(arg2, '[') ||
		strchr(arg2, ']') ||
		strchr(arg2, '%') ||
		strchr(arg2, '#') ||
		strchr(arg2, '@') ||
		strchr(arg2, '^') ||
		strchr(arg2, '&') ||
		strchr(arg2, '"')
		)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[Pet-Kulucka] Hatali isim girdiniz"));
		return;
	}

	if (ch->GetNewPetSystem()->IsActivePet())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Once petini gonder. "));
		return;
	}

	char szName[CHARACTER_NAME_MAX_LEN + 1];
	DBManager::instance().EscapeString(szName, sizeof(szName), arg2, strlen(arg2));

	std::unique_ptr<SQLMsg> pMsg(DBManager::instance().DirectQuery("UPDATE new_petsystem SET name = '%s' WHERE id = '%lu';", szName, item->GetID()));
	ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Pet Ismi Basarili Bir Sekilde Degistirildi!"));
	ch->RemoveSpecifyItem(19103, 1);
	// affecti siliyoruz var ise
	if (ch->FindAffect(AFFECT_PB2_PET_NAME))
		ch->RemoveAffect(AFFECT_PB2_PET_NAME);

	ch->AddAffect(AFFECT_PB2_PET_NAME, POINT_MALL_ATTBONUS, 20, AFF_NONE, INFINITE_AFFECT_DURATION, 0, true, true);
}

ACMD(do_MountChangeNamePremiumPerma)
{
	char arg1[256], arg2[256];
	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (!*arg1 || !*arg2)
		return;

	BYTE bCell = 0;
	str_to_number(bCell, arg1);

	if (bCell < 0)
	{
		LogManager::instance().HackLogEx(ch, "do_MountChangeName trying crash game core... (bCell)");
		return;
	}

	if (quest::CQuestManager::instance().GetPCForce(ch->GetPlayerID())->IsRunning() == true)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_OPERATION_IS_FAILED_QUEST_WINDOW_IS_OPENED"));
		return;
	}

	if (!ch->CanAct(true, true, VAR_TIME_MOUNT_ACTION, nullptr, 0))
		return;

	LPITEM item = ch->GetInventoryItem(bCell);
	if (!item)
		return;
	if (ch->CountSpecifyItem(19105) < 1)
		return;

	if (strchr(arg2, '%') ||
		strchr(arg2, '/') ||
		strchr(arg2, '>') ||
		strchr(arg2, '|') ||
		strchr(arg2, ';') ||
		strchr(arg2, ':') ||
		strchr(arg2, '}') ||
		strchr(arg2, '{') ||
		strchr(arg2, '[') ||
		strchr(arg2, ']') ||
		strchr(arg2, '%') ||
		strchr(arg2, '#') ||
		strchr(arg2, '@') ||
		strchr(arg2, '^') ||
		strchr(arg2, '&') ||
		strchr(arg2, '"')
		)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[Mount-Kulucka] Hatali isim girdiniz"));
		return;
	}

	if (ch->GetNewMountSystem()->IsActiveMount())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Once mountini gonder. "));
		return;
	}

	char szName[CHARACTER_NAME_MAX_LEN + 1];
	DBManager::instance().EscapeString(szName, sizeof(szName), arg2, strlen(arg2));

	std::unique_ptr<SQLMsg> pMsg(DBManager::instance().DirectQuery("UPDATE new_mountsystem SET name = '%s' WHERE id = '%lu';", szName, item->GetID()));
	ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Mount Ismi Basarili Bir Sekilde Degistirildi!"));
	ch->RemoveSpecifyItem(19105, 1);
	// affecti siliyoruz var ise
	if (ch->FindAffect(AFFECT_PB2_MOUNT_NAME))
		ch->RemoveAffect(AFFECT_PB2_MOUNT_NAME);

	ch->AddAffect(AFFECT_PB2_MOUNT_NAME, POINT_MALL_ATTBONUS, 20, AFF_NONE, INFINITE_AFFECT_DURATION, 0, true, true);

}
#endif // ENABLE_NAME_CHANGE_SYSTEM

#ifdef ENABLE_FULL_SYSTEM
ACMD(do_kanalduzenle)
{
	char arg1[256], arg2[256];
	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (!*arg1 || !*arg2)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "kullanim: /kanalduzenle kanal (0:orjinal,1:norm,2:busy,3:full)");
		return;
	}

	int iChannel = 0;
	int iDuzen = 0;

	str_to_number(iChannel, arg1);
	str_to_number(iDuzen, arg2);

	if (iChannel < 1 || iChannel > CHANNEL_MAX_COUNT)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Kanali yanlis girdin 1-8 arasi yapabilirsin.");
		return;
	}

	if (iDuzen < 0 || iDuzen > 3)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "duzeni yanlis girdin 0-3 arasi yapabilirsin.");
		return;
	}

	const char* duzenList[] = {"orjinal","norm","busy","full"};

	if (iDuzen == 0)
	{
		for (auto i = 0; i < _countof(duzenList); ++i)
		{
			char szBuf[128];
			snprintf(szBuf, sizeof(szBuf), "force_%s_ch%d", duzenList[i], iChannel);
			quest::CQuestManager::instance().RequestSetEventFlag(szBuf, 0);
		}
	}
	else
	{
		char szBuf[128];
		snprintf(szBuf, sizeof(szBuf), "force_%s_ch%d", duzenList[iDuzen], iChannel);
		quest::CQuestManager::instance().RequestSetEventFlag(szBuf, 1);
	}
}
#endif // ENABLE_FULL_SYSTEM

#ifdef ENABLE_TELEPORT_SYSTEM
ACMD (do_teleportace)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Wrong command use."));
		return;
	}

	BYTE mapnumber = 0;
	str_to_number(mapnumber, arg1);

	if (quest::CQuestManager::instance().GetPCForce(ch->GetPlayerID())->IsRunning() == true)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_OPERATION_IS_FAILED_QUEST_WINDOW_IS_OPENED"));
		return;
	}

	if (ch && !ch->CanAct(true, true, VAR_TIME_NONE))
		return;

	if (ch->IsDead() || !ch->IsPC())
		return;

	if (mapnumber == 1)
	{
		if (ch->GetLevel() >= 140)
		{
			ch->WarpSet(3250900,8562700,0);
		}
		else
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("YETERSIZ_SEVIYE"));
			return;
		}
	}
	else if (mapnumber == 2)
	{
		if (ch->GetLevel() >= 150)
		{
			ch->WarpSet(3250900,9562700,0);
		}
		else
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("YETERSIZ_SEVIYE"));
			return;
		}
	}
	else if (mapnumber == 3)
	{
		if (ch->GetLevel() >= 160)
		{
			ch->WarpSet(3250900,15062700,0);
		}
		else
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("YETERSIZ_SEVIYE"));
			return;
		}
	}
	else if (mapnumber == 4)
	{
		if (ch->GetLevel() >= 170)
		{
			ch->WarpSet(3250900,20062700,0);
		}
		else
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("YETERSIZ_SEVIYE"));
			return;
		}
	}

	//if (mapnumber == 1)
	//{
	//	if (ch->GetLevel() >= 120)
	//	{
	//		ch->WarpSet(3250900,30062700,0);
	//	}
	//	else
	//	{
	//		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("YETERSIZ_SEVIYE"));
	//		return;
	//	}
	//}
	//else if (mapnumber == 2)
	//{
	//	if (ch->GetLevel() >= 130)
	//	{
	//		ch->WarpSet(3250900,9062700,0);
	//	}
	//	else
	//	{
	//		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("YETERSIZ_SEVIYE"));
	//		return;
	//	}
	//}
	//else if (mapnumber == 7)
	//{
	//	if (ch->GetLevel() >= 180)
	//	{
	//		ch->WarpSet(3250900,25062700,0);
	//	}
	//	else
	//	{
	//		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("YETERSIZ_SEVIYE"));
	//		return;
	//	}
	//}
	//else if (mapnumber == 8)
	//{
	//	if (ch->GetLevel() >= 190)
	//	{
	//		ch->WarpSet(3250900,10062700,0);
	//	}
	//	else
	//	{
	//		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("YETERSIZ_SEVIYE"));
	//		return;
	//	}
	//}
	else
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("BILINMEYEN_HARITA"));
		return;
	}
}
#endif

#ifdef ENABLE_MULTI_FARM_BLOCK
ACMD(do_multi_farm)
{
	if (!ch->GetDesc())
		return;

	if (ch->GetProtectTime("multi-farm") > get_global_time())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Bu iþlemi gerçekleþtirebilmek için %d saniye beklemelisin.", ch->GetProtectTime("multi-farm") - get_global_time());
		return;
	}

	ch->SetProtectTime("multi-farm", get_global_time() + 10);
	CHARACTER_MANAGER::Instance().CheckMultiFarmAccount(ch->GetDesc()->GetComputerOS(), ch->GetPlayerID(), ch->GetName(), !ch->GetMultiStatus());
}
#endif