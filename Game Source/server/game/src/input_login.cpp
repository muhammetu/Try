#include "stdafx.h"
#include "constants.h"
#include "config.h"
#include "utils.h"
#include "input.h"
#include "desc_client.h"
#include "desc_manager.h"
#include "char.h"
#include "char_manager.h"
#include "cmd.h"
#include "buffer_manager.h"
#include "protocol.h"
#include "pvp.h"
#include "start_position.h"
#include "messenger_manager.h"
#include "guild_manager.h"
#include "party.h"
#ifdef __SCP1453_EXTENSIONS__
#include "dungeon_new.h"
#else
#include "dungeon.h"
#endif
#include "war_map.h"
#include "questmanager.h"
#include "affect.h"
#include "priv_manager.h"
#include "log.h"
#include "horsename_manager.h"
#include "MarkManager.h"
#include "dungeon_info.h"

#ifdef __EVENT_SYSTEM__
#include "game_events.h"
#endif

#ifdef __DUNGEON_INFORMATION__
#include "dungeon_info.h"
#endif

#ifdef __SWITCHBOT__
#include "new_switchbot.h"
#endif

#ifdef __OFFLINE_SHOP__
#include "new_offlineshop.h"
#include "new_offlineshop_manager.h"
#endif

#ifdef __INGAME_MALL__
#include "item_shop.h"
#endif // __INGAME_MALL__

static void _send_bonus_info(LPCHARACTER ch)
{
	int	item_drop_bonus = 0;
	int gold_drop_bonus = 0;
	int exp_bonus = 0;

	item_drop_bonus = CPrivManager::instance().GetPriv(ch, PRIV_ITEM_DROP);
	gold_drop_bonus = CPrivManager::instance().GetPriv(ch, PRIV_GOLD_DROP);
	exp_bonus = CPrivManager::instance().GetPriv(ch, PRIV_EXP_PCT);

	if (item_drop_bonus)
	{
		ch->ChatPacket(CHAT_TYPE_NOTICE,
			LC_TEXT("아이템 드롭률  %d%% 추가 이벤트 중입니다."), item_drop_bonus);
	}
	if (gold_drop_bonus)
	{
		ch->ChatPacket(CHAT_TYPE_NOTICE,
			LC_TEXT("골드 드롭률 %d%% 추가 이벤트 중입니다."), gold_drop_bonus);
	}
	if (exp_bonus)
	{
		ch->ChatPacket(CHAT_TYPE_NOTICE,
			LC_TEXT("경험치 %d%% 추가 획득 이벤트 중입니다."), exp_bonus);
	}
}

void CInputLogin::LoginByKey(LPDESC d, const char* data)
{
	TPacketCGLogin2* pinfo = (TPacketCGLogin2*)data;

	char login[LOGIN_MAX_LEN + 1];
	trim_and_lower(pinfo->login, login, sizeof(login));

	if (g_bNoMoreClient)
	{
		TPacketGCLoginFailure failurePacket;

		failurePacket.header = HEADER_GC_LOGIN_FAILURE;
		strlcpy(failurePacket.szStatus, "SHUTDOWN", sizeof(failurePacket.szStatus));
#ifdef __BAN_REASON_UTILITY__
		failurePacket.availDate = 0;
		strlcpy(failurePacket.szBanWebLink, "", sizeof(failurePacket.szBanWebLink));
#endif // __BAN_REASON_UTILITY__
		d->Packet(&failurePacket, sizeof(TPacketGCLoginFailure));
		return;
	}

	if (g_iUserLimit > 0)
	{
		int iTotal;
		int* paiEmpireUserCount;
		int iLocal;

		DESC_MANAGER::instance().GetUserCount(iTotal, &paiEmpireUserCount, iLocal);

		if (g_iUserLimit <= iTotal)
		{
			TPacketGCLoginFailure failurePacket;

			failurePacket.header = HEADER_GC_LOGIN_FAILURE;
			strlcpy(failurePacket.szStatus, "FULL", sizeof(failurePacket.szStatus));
#ifdef __BAN_REASON_UTILITY__
			failurePacket.availDate = 0;
			strlcpy(failurePacket.szBanWebLink, "", sizeof(failurePacket.szBanWebLink));
#endif // __BAN_REASON_UTILITY__
			d->Packet(&failurePacket, sizeof(TPacketGCLoginFailure));
			return;
		}
	}

	sys_log(0, "LOGIN_BY_KEY: %s key %u", login, pinfo->dwLoginKey);

	d->SetLoginKey(pinfo->dwLoginKey);
#ifndef __IMPROVED_PACKET_ENCRYPTION__
	d->SetSecurityKey(pinfo->adwClientKey);
#endif // __IMPROVED_PACKET_ENCRYPTION__

	TPacketGDLoginByKey ptod;

	strlcpy(ptod.szLogin, login, sizeof(ptod.szLogin));
	ptod.dwLoginKey = pinfo->dwLoginKey;
	thecore_memcpy(ptod.adwClientKey, pinfo->adwClientKey, sizeof(DWORD) * 4);
	strlcpy(ptod.szIP, d->GetHostName(), sizeof(ptod.szIP));

	db_clientdesc->DBPacket(HEADER_GD_LOGIN_BY_KEY, d->GetHandle(), &ptod, sizeof(TPacketGDLoginByKey));
}

void CInputLogin::ChangeName(LPDESC d, const char* data)
{
	TPacketCGChangeName* p = (TPacketCGChangeName*)data;
	const TAccountTable& c_r = d->GetAccountTable();

	if (!c_r.id)
	{
		sys_err("no account table");
		return;
	}

	if (p->index >= PLAYER_PER_ACCOUNT) // @fixme190
	{
		sys_err("index overflow %d, login: %s", p->index, c_r.login);
		d->SetPhase(PHASE_CLOSE);
		return;
	}

	if (!c_r.players[p->index].dwID) // @fixme190
	{
		sys_err("player index not found, login: %s", c_r.login);
		d->SetPhase(PHASE_CLOSE);
		return;
	}


	if (!c_r.players[p->index].bChangeName)
		return;

	// @duzenleme
	// adamin ismi 20 den buyuk 3 ten kucukse yapmisiz.
	// bu olayi her yere uyarlamamiz lazim cunku randumanli calismiyor.
	if (strlen(p->name) > PLAYER_NAME_MAX || strlen(p->name) < PLAYER_NAME_MIN)
	{
		TPacketGCCreateFailure pack;
		pack.header = HEADER_GC_CHARACTER_CREATE_FAILURE;
		pack.bType = 5;
		d->Packet(&pack, sizeof(pack));
		return;
	}

	if (!check_name(p->name))
	{
		TPacketGCCreateFailure pack;
		pack.header = HEADER_GC_CHARACTER_CREATE_FAILURE;
		pack.bType = 0;
		d->Packet(&pack, sizeof(pack));
		return;
	}

	TPacketGDChangeName pdb;

	pdb.pid = c_r.players[p->index].dwID;
	strlcpy(pdb.name, p->name, sizeof(pdb.name));
	db_clientdesc->DBPacket(HEADER_GD_CHANGE_NAME, d->GetHandle(), &pdb, sizeof(TPacketGDChangeName));
}

void CInputLogin::CharacterSelect(LPDESC d, const char* data)
{
	struct command_player_select* pinfo = (struct command_player_select*)data;
	const TAccountTable& c_r = d->GetAccountTable();

	sys_log(0, "player_select: login: %s index: %d", c_r.login, pinfo->index);

	if (!c_r.id)
	{
		sys_err("no account table");
		return;
	}

	// @duzenleme eger krallik 3 den buyukse return cekiyor.
	if (d->GetEmpire() < 0 || d->GetEmpire() > 3)
	{
		LogManager::instance().HackLogEx(d, "CInputLogin::CharacterSelect try crash to game core. (GetEmpire)");
		d->SetPhase(PHASE_CLOSE);
		return;
	}

	if (pinfo->index >= PLAYER_PER_ACCOUNT)
	{
		LogManager::instance().HackLogEx(d, "CInputLogin::CharacterSelect try crash to game core. (index)");
		sys_err("index overflow %d, login: %s", pinfo->index, c_r.login);
		return;
	}

	if (!c_r.players[pinfo->index].dwID) // fixme190
	{
		sys_err("player index not found, login: %s", c_r.login);
		d->SetPhase(PHASE_CLOSE);
		return;
	}

	if (c_r.players[pinfo->index].bChangeName)
	{
		sys_err("name must be changed idx %d, login %s, name %s",
			pinfo->index, c_r.login, c_r.players[pinfo->index].szName);
		return;
	}

	TPlayerLoadPacket player_load_packet;

	player_load_packet.account_id = c_r.id;
	player_load_packet.player_id = c_r.players[pinfo->index].dwID;
	player_load_packet.account_index = pinfo->index;

	db_clientdesc->DBPacket(HEADER_GD_PLAYER_LOAD, d->GetHandle(), &player_load_packet, sizeof(TPlayerLoadPacket));
}

#ifdef __BEGINNING_REWORK__
static DWORD GET_BEGINNER_PART_MAIN(BYTE race)
{
	switch (race)
	{
	case 0:
		return 43430;
	case 4:
		return 43431;
	case 1:
		return 43431;
	case 5:
		return 43430;
	case 2:
		return 43430;
	case 6:
		return 43431;
	case 3:
		return 43431;
	case 7:
		return 43430;
#ifdef __WOLFMAN_CHARACTER__
	case 8:
		return 43430;
#endif
	}
	return 0;
}

static DWORD GET_BEGINNER_PART_HAIR(BYTE race)
{
	switch (race)
	{
	case 0:
		return 728;
	case 4:
		return 728;
	case 1:
		return 728;
	case 5:
		return 728;
	case 2:
		return 728;
	case 6:
		return 728;
	case 3:
		return 728;
	case 7:
		return 728;
#ifdef __WOLFMAN_CHARACTER__
	case 8:
		return 728;
#endif
	}

	return 0;
}
#endif

bool RaceToJob(unsigned race, unsigned* ret_job)
{
	*ret_job = 0;

	if (race >= MAIN_RACE_MAX_NUM)
		return false;

	switch (race)
	{
	case MAIN_RACE_WARRIOR_M:
		*ret_job = JOB_WARRIOR;
		break;

	case MAIN_RACE_WARRIOR_W:
		*ret_job = JOB_WARRIOR;
		break;

	case MAIN_RACE_ASSASSIN_M:
		*ret_job = JOB_ASSASSIN;
		break;

	case MAIN_RACE_ASSASSIN_W:
		*ret_job = JOB_ASSASSIN;
		break;

	case MAIN_RACE_SURA_M:
		*ret_job = JOB_SURA;
		break;

	case MAIN_RACE_SURA_W:
		*ret_job = JOB_SURA;
		break;

	case MAIN_RACE_SHAMAN_M:
		*ret_job = JOB_SHAMAN;
		break;

	case MAIN_RACE_SHAMAN_W:
		*ret_job = JOB_SHAMAN;
		break;
#ifdef __WOLFMAN_CHARACTER__
	case MAIN_RACE_WOLFMAN_M:
		*ret_job = JOB_WOLFMAN;
		break;
#endif
	default:
		return false;
		break;
	}
	return true;
}

bool NewPlayerTable2(TPlayerTable* table, const char* name, BYTE race, BYTE shape, BYTE bEmpire)
{
	if (race >= MAIN_RACE_MAX_NUM)
	{
		sys_err("NewPlayerTable2.OUT_OF_RACE_RANGE(%d >= max(%d))\n", race, MAIN_RACE_MAX_NUM);
		return false;
	}

	unsigned job;

	if (!RaceToJob(race, &job))
	{
		sys_err("NewPlayerTable2.RACE_TO_JOB_ERROR(%d)\n", race);
		return false;
	}

	sys_log(0, "NewPlayerTable2(name=%s, race=%d, job=%d)", name, race, job);

	memset(table, 0, sizeof(TPlayerTable));

	strlcpy(table->name, name, sizeof(table->name));
	table->level = STARTUP_LEVEL;
	table->job = race;

	table->part_base = shape;

	table->st = JobInitialPoints[job].st;
	table->dx = JobInitialPoints[job].dx;
	table->ht = JobInitialPoints[job].ht;
	table->iq = JobInitialPoints[job].iq;

	table->hp = JobInitialPoints[job].max_hp + table->ht * JobInitialPoints[job].hp_per_ht;
	table->sp = JobInitialPoints[job].max_sp + table->iq * JobInitialPoints[job].sp_per_iq;
	table->stamina = JobInitialPoints[job].max_stamina;

	table->x = CREATE_START_X(bEmpire) + number(-300, 300);
	table->y = CREATE_START_Y(bEmpire) + number(-300, 300);
	table->z = 0;

	table->playtime = 0;
	table->gold = 0;
#ifdef __CHEQUE_SYSTEM__
	table->cheque = 0;
#endif
	table->skill_group = 0;
#ifdef __BEGINNING_REWORK__
	table->parts[PART_MAIN] = GET_BEGINNER_PART_MAIN(race);
	table->parts[PART_HAIR] = GET_BEGINNER_PART_HAIR(race);
	table->stat_point = 162;
#endif
#ifdef __ACCE_SYSTEM__
	table->parts[PART_ACCE] = 0;
#endif

	return true;
}

void CInputLogin::CharacterCreate(LPDESC d, const char* data)
{
	struct command_player_create* pinfo = (struct command_player_create*)data;
	TPlayerCreatePacket player_create_packet;

	sys_log(0, "PlayerCreate: name %s pos %d job %d shape %d", pinfo->name, pinfo->index, pinfo->job, pinfo->shape);

	TPacketGCLoginFailure packFailure;
	memset(&packFailure, 0, sizeof(packFailure));
	packFailure.header = HEADER_GC_CHARACTER_CREATE_FAILURE;
#ifdef __BAN_REASON_UTILITY__
	packFailure.availDate = 0;
	strlcpy(packFailure.szBanWebLink, "", sizeof(packFailure.szBanWebLink));
#endif // __BAN_REASON_UTILITY__

	if (pinfo->index >= PLAYER_PER_ACCOUNT) // @fixme190
	{
		sys_err("index overflow %d, login: %s", pinfo->index, pinfo->name);
		d->SetPhase(PHASE_CLOSE);
		return;
	}

	if (strlen(pinfo->name) > 12 || strlen(pinfo->name) < 3)
	{
		TPacketGCCreateFailure pack;
		pack.header = HEADER_GC_CHARACTER_CREATE_FAILURE;
		pack.bType = 5;

		d->Packet(&pack, sizeof(pack));
		return;
	}

	if (!check_name(pinfo->name))
	{
		TPacketGCCreateFailure pack;
		pack.header = HEADER_GC_CHARACTER_CREATE_FAILURE;
		pack.bType = 0;

		d->Packet(&pack, sizeof(pack));
		return;
	}

	if (pinfo->shape > 1)
	{
		TPacketGCCreateFailure pack;
		pack.header = HEADER_GC_CHARACTER_CREATE_FAILURE;
		pack.bType = 4;

		d->Packet(&pack, sizeof(pack));
		return;
	}

	const TAccountTable& c_rAccountTable = d->GetAccountTable();

	if (0 == strcmp(c_rAccountTable.login, pinfo->name))
	{
		TPacketGCCreateFailure pack;
		pack.header = HEADER_GC_CHARACTER_CREATE_FAILURE;
		pack.bType = 1;

		d->Packet(&pack, sizeof(pack));
		return;
	}

	memset(&player_create_packet, 0, sizeof(TPlayerCreatePacket));

	if (!NewPlayerTable2(&player_create_packet.player_table, pinfo->name, pinfo->job, pinfo->shape, d->GetEmpire()))
	{
		sys_err("player_prototype error: job %d face %d ", pinfo->job);
		d->Packet(&packFailure, sizeof(packFailure));
		return;
	}

	trim_and_lower(c_rAccountTable.login, player_create_packet.login, sizeof(player_create_packet.login));
	strlcpy(player_create_packet.passwd, c_rAccountTable.passwd, sizeof(player_create_packet.passwd));

	player_create_packet.account_id = c_rAccountTable.id;
	player_create_packet.account_index = pinfo->index;

#ifdef __GOLD_LIMIT_REWORK__
	sys_log(0, "PlayerCreate: name %s account_id %d, TPlayerCreatePacketSize(%d), Packet->Gold %lld",
#else
	sys_log(0, "PlayerCreate: name %s account_id %d, TPlayerCreatePacketSize(%d), Packet->Gold %d",
#endif
		pinfo->name,
		pinfo->index,
		sizeof(TPlayerCreatePacket),
		player_create_packet.player_table.gold);

	db_clientdesc->DBPacket(HEADER_GD_PLAYER_CREATE, d->GetHandle(), &player_create_packet, sizeof(TPlayerCreatePacket));
}

void CInputLogin::CharacterDelete(LPDESC d, const char* data)
{
	struct command_player_delete* pinfo = (struct command_player_delete*)data;
	const TAccountTable& c_rAccountTable = d->GetAccountTable();

	if (!c_rAccountTable.id)
	{
		sys_err("PlayerDelete: no login data");
		return;
	}

	sys_log(0, "PlayerDelete: login: %s index: %d, social_id %s", c_rAccountTable.login, pinfo->index, pinfo->private_code);

	if (pinfo->index >= PLAYER_PER_ACCOUNT)
	{
		sys_err("PlayerDelete: index overflow %d, login: %s", pinfo->index, c_rAccountTable.login);
		return;
	}

	if (!c_rAccountTable.players[pinfo->index].dwID)
	{
		sys_err("PlayerDelete: Wrong Social ID index %d, login: %s", pinfo->index, c_rAccountTable.login);
		d->Packet(encode_byte(HEADER_GC_CHARACTER_DELETE_WRONG_SOCIAL_ID), 1);
		return;
	}

	TPlayerDeletePacket	player_delete_packet;

	trim_and_lower(c_rAccountTable.login, player_delete_packet.login, sizeof(player_delete_packet.login));
	player_delete_packet.player_id = c_rAccountTable.players[pinfo->index].dwID;
	player_delete_packet.account_index = pinfo->index;
	strlcpy(player_delete_packet.private_code, pinfo->private_code, sizeof(player_delete_packet.private_code));

	db_clientdesc->DBPacket(HEADER_GD_PLAYER_DELETE, d->GetHandle(), &player_delete_packet, sizeof(TPlayerDeletePacket));
}

void CInputLogin::Entergame(LPDESC d, const char* data)
{
	LPCHARACTER ch;

	if (!(ch = d->GetCharacter()))
	{
		d->SetPhase(PHASE_CLOSE);
		return;
	}

#ifdef __SCP1453_EXTENSIONS__
	if (!ch->IsGM())
	{
		for (auto &&i : dungeonTable)
		{
			if (i.map_index == 210)
				continue;

			if (i.map_index == ch->GetMapIndex())
			{
				ch->GoHome();
				return;
			}
		}
	}
#endif

	PIXEL_POSITION pos = ch->GetXYZ();

	if (!SECTREE_MANAGER::instance().GetMovablePosition(ch->GetMapIndex(), pos.x, pos.y, pos))
	{
		PIXEL_POSITION pos2;
		SECTREE_MANAGER::instance().GetRecallPositionByEmpire(ch->GetMapIndex(), ch->GetEmpire(), pos2);

		sys_err("!GetMovablePosition (name %s %dx%d map %d changed to %dx%d)",
			ch->GetName(),
			pos.x, pos.y,
			ch->GetMapIndex(),
			pos2.x, pos2.y);
		pos = pos2;
	}

	CGuildManager::instance().LoginMember(ch);

	ch->Show(ch->GetMapIndex(), pos.x, pos.y, pos.z);
	ch->ReviveInvisible(5);

	d->SetPhase(PHASE_GAME);

	sys_log(0, "ENTERGAME: %s %dx%dx%d %s map_index %d", ch->GetName(), ch->GetX(), ch->GetY(), ch->GetZ(), d->GetHostName(), ch->GetMapIndex());

	if (ch->GetHorseLevel() > 0)
		ch->EnterHorse();

	ch->ResetPlayTime();

	ch->StartSaveEvent();
	ch->StartRecoveryEvent();
#ifdef ENABLE_PLAYER_STATISTICS
	ch->SendPlayerStatisticsPacket();
#endif
#ifdef ENABLE_MULTI_FARM_BLOCK
	CHARACTER_MANAGER::Instance().CheckMultiFarmAccount(d->GetComputerOS(), ch->GetPlayerID(), ch->GetName(), true, false);
	//CHARACTER_MANAGER::Instance().CheckMultiFarmAccount(d->GetAccountTable().hwid, ch->GetPlayerID(), ch->GetName(), true, false);
#endif
	CPVPManager::instance().Connect(ch);
	CPVPManager::instance().SendList(d);

	MessengerManager::instance().Login(ch->GetName());

	CPartyManager::instance().SetParty(ch);
	CGuildManager::instance().SendGuildWar(ch);

#ifdef __EVENT_SYSTEM__
	CGameEventsManager::instance().SendEventCharacter(ch);
#endif // __EVENT_SYSTEM__

#ifdef __INGAME_MALL__
	CItemShopManager::instance().SendClientPacket(ch);
	ch->RefreshDragonCoin();
#endif // __INGAME_MALL__

	TPacketGCTime p;
	p.bHeader = HEADER_GC_TIME;
	p.time = get_global_time();
	d->Packet(&p, sizeof(p));

	TPacketGCChannel p2;
	p2.header = HEADER_GC_CHANNEL;
	p2.channel = g_bChannel;
	d->Packet(&p2, sizeof(p2));

	_send_bonus_info(ch);

	for (int i = 0; i <= PREMIUM_MAX_NUM; ++i)
	{
		int remain = ch->GetPremiumRemainSeconds(i);

		if (remain <= 0)
			continue;

		ch->AddAffect(AFFECT_PREMIUM_START + i, POINT_NONE, 0, 0, remain, 0, true);
		sys_log(0, "PREMIUM: %s type %d %dmin", ch->GetName(), i, remain);
	}

	if (ch->GetMapIndex() >= 10000)
	{
		if (CWarMapManager::instance().IsWarMap(ch->GetMapIndex()))
		{
#ifdef __GUILD_WAR_REWORK__
			CWarMap* pMap = CWarMapManager::instance().Find(ch->GetMapIndex());
			if (pMap)
			{
				BYTE bIdx;
				if (ch->GetGuild() && pMap->GetTeamIndex(ch->GetGuild()->GetID(), bIdx))
				{
					if (pMap->GetCurrentPlayer(bIdx) + 1 > pMap->GetMaxPlayer(bIdx)) {
						ch->ChatPacket(CHAT_TYPE_BIG_NOTICE, LC_TEXT("oyuncu limiti dolu!"));
						ch->GoHome();
						return;
					}
				}
			}
			ch->SetWarMap(pMap);
#else
			ch->SetWarMap(CWarMapManager::instance().Find(ch->GetMapIndex()));
#endif
		}
		else {
			ch->SetDungeon(CDungeonManager::instance().FindByMapIndex(ch->GetMapIndex()));
		}
	}
	else
	{
		if (CWarMapManager::instance().IsWarMap(ch->GetMapIndex()))
		{
			if (!test_server)
				ch->WarpSet(EMPIRE_START_X(ch->GetEmpire()), EMPIRE_START_Y(ch->GetEmpire()));
		}
	}

	if (ch->GetHorseLevel() > 0)
	{
		DWORD pid = ch->GetPlayerID();

		if (pid != 0 && CHorseNameManager::instance().GetHorseName(pid) == NULL)
			db_clientdesc->DBPacket(HEADER_GD_REQ_HORSE_NAME, 0, &pid, sizeof(DWORD));
		// @duzenleme
		// atimin levelini zaten yukarda aliyor neden bana paket gonderiyorki tekrardan.
		ch->SetHorseLevel(ch->GetHorseLevel());
		ch->SkillLevelPacket();
	}

#ifdef __MOUNT_COSTUME_SYSTEM__
	if (!CWarMapManager::instance().IsWarMap(ch->GetMapIndex()))
		ch->CheckMount();
#endif
#ifdef __PET_SYSTEM_PROTO__
	if (!CWarMapManager::instance().IsWarMap(ch->GetMapIndex()))
		ch->CheckPet();
#endif
#ifdef __DUNGEON_INFORMATION__
	DungeonInfo::instance().Update(ch);
#endif
#ifdef __SWITCHBOT__
	CSwitchbotManager::Instance().EnterGame(ch);
#endif
#ifndef __GAME_MASTER_UTILITY__
	if (ch->GetGMLevel() > GM_PLAYER)
		ch->ChatPacket(CHAT_TYPE_COMMAND, "MasterEnable");
#endif

#ifdef __OFFLINE_SHOP__
	if (ch->IsPC())
	{
		offlineshop::CShop* pkShop = offlineshop::GetManager().GetShopByOwnerID(ch->GetPlayerID());
		if (pkShop)
			ch->SetOfflineShop(pkShop);
	}
#endif

#ifdef __BATTLE_PASS_SYSTEM__
	ch->CreateLoginTime();

	char tillNext[128];
	snprintf(tillNext, sizeof(tillNext), "SpecialEventUntil %d", ch->GetSecondsTillNextWeek());
	ch->ChatPacket(CHAT_TYPE_COMMAND, tillNext);
#endif // __BATTLE_PASS_SYSTEM__
}

void CInputLogin::Empire(LPDESC d, const char* c_pData)
{
	const TPacketCGEmpire* p = reinterpret_cast<const TPacketCGEmpire*>(c_pData);

	if (EMPIRE_MAX_NUM <= p->bEmpire)
	{
		d->SetPhase(PHASE_CLOSE);
		return;
	}

	const TAccountTable& r = d->GetAccountTable();

	if (r.bEmpire)
	{
		for (int i = 0; i < PLAYER_PER_ACCOUNT; ++i)
		{
			if (r.players[i].dwID)
			{
				sys_err("EmpireSelectFailed %d", r.players[i].dwID);
				d->SetPhase(PHASE_CLOSE);
				return;
			}
		}
	}

	TEmpireSelectPacket pd;

	pd.dwAccountID = r.id;
	pd.bEmpire = p->bEmpire;

	db_clientdesc->DBPacket(HEADER_GD_EMPIRE_SELECT, d->GetHandle(), &pd, sizeof(pd));
}

void CInputLogin::GuildMarkUpload(LPDESC d, const char* c_pData)
{
	TPacketCGMarkUpload* p = (TPacketCGMarkUpload*)c_pData;
	CGuildManager& rkGuildMgr = CGuildManager::instance();
	CGuild* pkGuild;

	if (!(pkGuild = rkGuildMgr.FindGuild(p->gid)))
	{
		sys_err("MARK_SERVER: GuildMarkUpload: no guild. gid %u", p->gid);
		return;
	}

	if (pkGuild->GetLevel() < guild_mark_min_level)
	{
		sys_log(0, "MARK_SERVER: GuildMarkUpload: level < %u (%u)", guild_mark_min_level, pkGuild->GetLevel());
		return;
	}

	CGuildMarkManager& rkMarkMgr = CGuildMarkManager::instance();

	sys_log(0, "MARK_SERVER: GuildMarkUpload: gid %u", p->gid);

	bool isEmpty = true;

	for (DWORD iPixel = 0; iPixel < SGuildMark::SIZE; ++iPixel)
		if (*((DWORD*)p->image + iPixel) != 0x00000000)
			isEmpty = false;

	if (isEmpty)
		rkMarkMgr.DeleteMark(p->gid);
	else
		rkMarkMgr.SaveMark(p->gid, p->image);
}

void CInputLogin::GuildMarkIDXList(LPDESC d, const char* c_pData)
{
	CGuildMarkManager& rkMarkMgr = CGuildMarkManager::instance();

	DWORD bufSize = sizeof(WORD) * 2 * rkMarkMgr.GetMarkCount();
	char* buf = NULL;

	if (bufSize > 0)
	{
		buf = (char*)malloc(bufSize);
		rkMarkMgr.CopyMarkIdx(buf);
	}

	TPacketGCMarkIDXList p;
	p.header = HEADER_GC_MARK_IDXLIST;
	p.bufSize = sizeof(p) + bufSize;
	p.count = rkMarkMgr.GetMarkCount();

	if (buf)
	{
		d->BufferedPacket(&p, sizeof(p));
		d->LargePacket(buf, bufSize);
		free(buf);
	}
	else
		d->Packet(&p, sizeof(p));

	sys_log(0, "MARK_SERVER: GuildMarkIDXList %d bytes sent.", p.bufSize);
}

void CInputLogin::GuildMarkCRCList(LPDESC d, const char* c_pData)
{
	TPacketCGMarkCRCList* pCG = (TPacketCGMarkCRCList*)c_pData;

	std::map<BYTE, const SGuildMarkBlock*> mapDiffBlocks;
	CGuildMarkManager::instance().GetDiffBlocks(pCG->imgIdx, pCG->crclist, mapDiffBlocks);

	DWORD blockCount = 0;
	TEMP_BUFFER buf(1024 * 1024);

	for (itertype(mapDiffBlocks) it = mapDiffBlocks.begin(); it != mapDiffBlocks.end(); ++it)
	{
		BYTE posBlock = it->first;
		const SGuildMarkBlock& rkBlock = *it->second;

		buf.write(&posBlock, sizeof(BYTE));
		buf.write(&rkBlock.m_sizeCompBuf, sizeof(DWORD));
		buf.write(rkBlock.m_abCompBuf, rkBlock.m_sizeCompBuf);

		++blockCount;
	}

	TPacketGCMarkBlock pGC;

	pGC.header = HEADER_GC_MARK_BLOCK;
	pGC.imgIdx = pCG->imgIdx;
	pGC.bufSize = buf.size() + sizeof(TPacketGCMarkBlock);
	pGC.count = blockCount;

	sys_log(0, "MARK_SERVER: Sending blocks. (imgIdx %u diff %u size %u)", pCG->imgIdx, mapDiffBlocks.size(), pGC.bufSize);

	if (buf.size() > 0)
	{
		d->BufferedPacket(&pGC, sizeof(TPacketGCMarkBlock));
		d->LargePacket(buf.read_peek(), buf.size());
	}
	else
		d->Packet(&pGC, sizeof(TPacketGCMarkBlock));
}

int CInputLogin::Analyze(LPDESC d, BYTE bHeader, const char* c_pData)
{
	int iExtraLen = 0;

	switch (bHeader)
	{
	case HEADER_CG_PONG:
		Pong(d);
		break;

	case HEADER_CG_TIME_SYNC:
		Handshake(d, c_pData);
		break;

	case HEADER_CG_LOGIN2:
		LoginByKey(d, c_pData);
		break;

	case HEADER_CG_CHARACTER_SELECT:
		CharacterSelect(d, c_pData);
		break;

	case HEADER_CG_CHARACTER_CREATE:
		CharacterCreate(d, c_pData);
		break;

	case HEADER_CG_CHARACTER_DELETE:
		CharacterDelete(d, c_pData);
		break;
	case HEADER_CG_ENTERGAME:
		Entergame(d, c_pData);
		break;
	case HEADER_CG_EMPIRE:
		Empire(d, c_pData);
		break;

	// @duzenleme MartySama arkadasin yaptigi login phase paketlerinin kontrol edilmemesi olayi..
	case HEADER_CG_MOVE:
	case HEADER_CG_ITEM_USE:
	case HEADER_CG_TARGET:
	case HEADER_CG_CHAT:
	case HEADER_CG_WHISPER:
	case HEADER_CG_MARK_LOGIN:
	case HEADER_CG_ITEM_PICKUP:
	case HEADER_CG_HANDSHAKE: // unprocessed packet.
		break;

		///////////////////////////////////////
		// Guild Mark
		/////////////////////////////////////
	case HEADER_CG_MARK_CRCLIST:
		GuildMarkCRCList(d, c_pData);
		break;

	case HEADER_CG_MARK_IDXLIST:
		GuildMarkIDXList(d, c_pData);
		break;

	case HEADER_CG_MARK_UPLOAD:
		GuildMarkUpload(d, c_pData);
		break;

	case HEADER_CG_HACK:
		break;

	case HEADER_CG_CHANGE_NAME:
		ChangeName(d, c_pData);
		break;

	default:
		sys_err("login phase does not handle this packet! header %d", bHeader);
		//d->SetPhase(PHASE_CLOSE);
		return (0);
	}

	return (iExtraLen);
}