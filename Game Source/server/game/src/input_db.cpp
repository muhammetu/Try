#include "stdafx.h"
#include "constants.h"
#include "config.h"
#include "utils.h"
#include "desc_manager.h"
#include "char.h"
#include "char_manager.h"
#include "item.h"
#include "item_manager.h"
#include "packet.h"
#include "protocol.h"
#include "mob_manager.h"
#include "shop_manager.h"
#include "sectree_manager.h"
#include "skill.h"
#include "questmanager.h"
#include "p2p.h"
#include "guild.h"
#include "guild_manager.h"
#include "start_position.h"
#include "party.h"
#include "refine.h"
#include "priv_manager.h"
#include "db.h"
#include "login_data.h"
#include "unique_item.h"

#include "affect.h"
#include "motion.h"

#include "log.h"

#include "horsename_manager.h"
#include "gm.h"
#include "map_location.h"

#include "DragonSoul.h"

#include "shutdown_manager.h"

#ifdef __OFFLINE_SHOP__
#include "new_offlineshop.h"
#include "new_offlineshop_manager.h"
#endif

#ifdef __INGAME_MALL__
#include "item_shop.h"
#endif

#ifdef __BATTLE_PASS_SYSTEM__
#include "battle_pass.h"
#endif

#include "desc_client.h"

#ifdef __CHANNEL_CHANGER__
void CInputDB::ChangeChannel(LPDESC d, const char* pcData)
{
	if (!d || !d->GetCharacter())
	{
		sys_err("Change channel request with empty or invalid description handle!");
		return;
	}
	TPacketReturnChannel* p = (TPacketReturnChannel*)pcData;
	if (!p->lAddr || !p->port)
	{
		std::string pName = d->GetCharacter()->GetName();
		d->GetCharacter()->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CANNOT_CHANGE_CHANNEL"));
		// @@
		// buradaki syserre gerek yok kanal kapaliysa degistiremeyincede veriyor.
		//sys_err("Can't switch channel for player %s!", pName.c_str());
		return;
	}
	d->GetCharacter()->StartChannelSwitch(p->lAddr, p->port);
}
#endif

bool GetServerLocation(TAccountTable& rTab, BYTE bEmpire)
{
	bool bFound = false;

	for (int i = 0; i < PLAYER_PER_ACCOUNT; ++i)
	{
		if (0 == rTab.players[i].dwID)
			continue;

		bFound = true;
		long lIndex = 0;

		if (!CMapLocation::instance().Get(rTab.players[i].x,
			rTab.players[i].y,
			lIndex,
			rTab.players[i].lAddr,
			rTab.players[i].wPort))
		{
			sys_err("location error name %s mapindex %d %d x %d empire %d",
				rTab.players[i].szName, lIndex, rTab.players[i].x, rTab.players[i].y, rTab.bEmpire);

			rTab.players[i].x = EMPIRE_START_X(rTab.bEmpire);
			rTab.players[i].y = EMPIRE_START_Y(rTab.bEmpire);

			lIndex = 0;

			if (!CMapLocation::instance().Get(rTab.players[i].x, rTab.players[i].y, lIndex, rTab.players[i].lAddr, rTab.players[i].wPort))
			{
				sys_err("cannot find server for mapindex %d %d x %d (name %s)",
					lIndex,
					rTab.players[i].x,
					rTab.players[i].y,
					rTab.players[i].szName);
				if (!g_stProxyIP.empty())
					rTab.players[i].lAddr = inet_addr(g_stProxyIP.c_str());
				continue;
			}
		}
		if (!g_stProxyIP.empty())
			rTab.players[i].lAddr = inet_addr(g_stProxyIP.c_str());
		struct in_addr in;
		in.s_addr = rTab.players[i].lAddr;
		sys_log(0, "success to %s:%d", inet_ntoa(in), rTab.players[i].wPort);
	}

	return bFound;
}

void CInputDB::LoginSuccess(DWORD dwHandle, const char* data)
{
	sys_log(0, "LoginSuccess");

	TAccountTable* pTab = (TAccountTable*)data;

	LPDESC d = DESC_MANAGER::instance().FindByHandle(dwHandle);

	if (!d)
	{
		sys_log(0, "CInputDB::LoginSuccess - cannot find handle [%s]", pTab->login);

		TLogoutPacket pack;

		strlcpy(pack.login, pTab->login, sizeof(pack.login));
		db_clientdesc->DBPacket(HEADER_GD_LOGOUT, dwHandle, &pack, sizeof(pack));
		return;
	}

	if (strcmp(pTab->status, "OK"))
	{
		sys_log(0, "CInputDB::LoginSuccess - status[%s] is not OK [%s]", pTab->status, pTab->login);

		TLogoutPacket pack;

		strlcpy(pack.login, pTab->login, sizeof(pack.login));
		db_clientdesc->DBPacket(HEADER_GD_LOGOUT, dwHandle, &pack, sizeof(pack));

		LoginFailure(d, pTab->status);
		return;
	}

	for (int i = 0; i != PLAYER_PER_ACCOUNT; ++i)
	{
		TSimplePlayer& player = pTab->players[i];
		sys_log(0, "\tplayer(%s).job(%d)", player.szName, player.byJob);
	}

	bool bFound = GetServerLocation(*pTab, pTab->bEmpire);

	d->BindAccountTable(pTab);

	if (!bFound)
	{
		TPacketGCEmpire pe;
		pe.bHeader = HEADER_GC_EMPIRE;
		pe.bEmpire = number(1, 3);
		d->Packet(&pe, sizeof(pe));
	}
	else
	{
		TPacketGCEmpire pe;
		pe.bHeader = HEADER_GC_EMPIRE;
		pe.bEmpire = d->GetEmpire();
		d->Packet(&pe, sizeof(pe));
	}

	d->SetPhase(PHASE_SELECT);
	d->SendLoginSuccessPacket();

	// __SHUTDOWN::Shutdown Register
	CShutdownManager::Instance().AddDesc(d);

	sys_log(0, "InputDB::login_success: %s", pTab->login);
}

void CInputDB::PlayerCreateFailure(LPDESC d, BYTE bType)
{
	if (!d)
		return;

	TPacketGCCreateFailure pack;

	pack.header = HEADER_GC_CHARACTER_CREATE_FAILURE;
	pack.bType = bType;

	d->Packet(&pack, sizeof(pack));
}

void CInputDB::PlayerCreateSuccess(LPDESC d, const char* data)
{
	if (!d)
		return;

	TPacketDGCreateSuccess* pPacketDB = (TPacketDGCreateSuccess*)data;

	if (pPacketDB->bAccountCharacterIndex >= PLAYER_PER_ACCOUNT)
	{
		d->Packet(encode_byte(HEADER_GC_CHARACTER_CREATE_FAILURE), 1);
		return;
	}

	long lIndex = 0;

	if (!CMapLocation::instance().Get(pPacketDB->player.x,
		pPacketDB->player.y,
		lIndex,
		pPacketDB->player.lAddr,
		pPacketDB->player.wPort))
	{
		sys_err("InputDB::PlayerCreateSuccess: cannot find server for mapindex %d %d x %d (name %s)",
			lIndex,
			pPacketDB->player.x,
			pPacketDB->player.y,
			pPacketDB->player.szName);
	}

	TAccountTable& r_Tab = d->GetAccountTable();
	r_Tab.players[pPacketDB->bAccountCharacterIndex] = pPacketDB->player;

	TPacketGCPlayerCreateSuccess pack;

	pack.header = HEADER_GC_CHARACTER_CREATE_SUCCESS;
	pack.bAccountCharacterIndex = pPacketDB->bAccountCharacterIndex;
	pack.player = pPacketDB->player;
	if (!g_stProxyIP.empty())
		pack.player.lAddr = inet_addr(g_stProxyIP.c_str());
	d->Packet(&pack, sizeof(TPacketGCPlayerCreateSuccess));

#ifdef __BEGINNING_REWORK__
	// NEW_GIVE_BASIC
	TPlayerItem t;
	memset(&t, 0, sizeof(t));

	t.owner = r_Tab.players[pPacketDB->bAccountCharacterIndex].dwID;

	struct SInitialItem
	{
		BYTE	window;
		WORD	pos;
		DWORD	count;

		DWORD	dwVnum;
		long	alSockets[4];
		TPlayerItemAttribute	aAttr[5];
#ifdef __BEGINNER_ITEM__
		bool	is_basic;
#endif
	};

	const int MAX_INITIAL_ITEM = 40;

	static SInitialItem initialItems[MAIN_RACE_MAX_NUM][MAX_INITIAL_ITEM] =
	{
		{//Erkek Savaþçý
			{EQUIPMENT, WEAR_WEAPON, 1, 21933, {get_global_time() + 604800,0,0,0}, {{72,100},{0,0},{0,0},{0,0},{0,0}}, false},// Silah
			{EQUIPMENT, WEAR_BODY, 1, 11299, {0,0,0,0}, {{1,2000},{23,10},{24,10},{53,50},{9,20}}, false},// Zýrh
			{EQUIPMENT, WEAR_HEAD, 1, 12249, {0,0,0,0}, {{18,20},{20,20},{21,20},{22,20},{12,8}}, false},// Kask
			{EQUIPMENT, WEAR_SHIELD, 1, 13049, {0,0,0,0}, {{48,1},{18,20},{20,20},{21,20},{22,20}}, false},// Kalkan
			{EQUIPMENT, WEAR_WRIST, 1, 14209, {0,0,0,0}, {{1,2000},{18,20},{20,20},{21,20},{22,20}}, false},// Bilezik
			{EQUIPMENT, WEAR_FOOTS, 1, 15209, {0,0,0,0}, {{1,2000},{28,15},{15,10},{0,0},{0,0}}, false},// Ayakkabý
			{EQUIPMENT, WEAR_NECK, 1, 16209, {0,0,0,0}, {{1,2000},{15,10},{16,10},{18,20},{19,20}}, false},// Kolye
			{EQUIPMENT, WEAR_EAR, 1, 17109, {0,0,0,0}, {{18,20},{20,20},{21,20},{22,20},{0,0}}, false},// Küpe

			{EQUIPMENT, WEAR_COSTUME_BODY, 1, 43430, {get_global_time() + 86313600,0,0,0}, {{0,0},{0,0},{0,0},{0,0},{0,0}}, false},// Zýrh Kostümü
			{EQUIPMENT, WEAR_COSTUME_HAIR, 1, 43432, {get_global_time() + 86313600,0,0,0}, {{0,0},{0,0},{0,0},{0,0},{0,0}}, false},// Saç Kostümü
			{EQUIPMENT, WEAR_COSTUME_MOUNT, 1, 15592, {get_global_time() + 86313600,0,0,0}, {{0,0},{0,0},{0,0},{0,0},{0,0}}, false},// Binek Kostümü
			{EQUIPMENT, WEAR_COSTUME_WEAPON, 1, 43403, {get_global_time() + 86313600,0,0,0}, {{0,0},{0,0},{0,0},{0,0},{0,0}}, false},// Silah Kostümü
			{EQUIPMENT, WEAR_PET, 1, 43101, {get_global_time() + 86313600,0,0,0}, {{0,0},{0,0},{0,0},{0,0},{0,0}}, false},// Pet

			{INVENTORY, 37, 1, 72726, {0,0,7000000,0}, {{0,0},{0,0},{0,0},{0,0},{0,0}}, true},// Güneþ Özütü
			{INVENTORY, 35, 1, 72730, {0,0,700000,0}, {{0,0},{0,0},{0,0},{0,0},{0,0}}, true},// Ay Özütü
			{INVENTORY, 27, 1, 70058, {0,0,0,0}, {{0,0},{0,0},{0,0},{0,0},{0,0}}, true},// Iþýnlanma Yüzüðü

			{INVENTORY, 28, 1, 39006, {0,0,0,0}, {{0,0},{0,0},{0,0},{0,0},{0,0}}, true},// Pelerin

			{INVENTORY, 0, 1, 21930, {get_global_time() + 604800,0,0,0}, {{72,100},{0,0},{0,0},{0,0},{0,0}}, false},// Tek El
			{INVENTORY, 1, 1, 43405, {get_global_time() + 86313600,0,0,0}, {{0,0},{0,0},{0,0},{0,0},{0,0}}, false},// Tek El Kostümü
			{INVENTORY, 2, 1, 43402, {get_global_time() + 86313600,0,0,0}, {{0,0},{0,0},{0,0},{0,0},{0,0}}, false},// Tek El Kostümü
			{INVENTORY, 3, 1, 19958, {0,0,0,0}, {{0,0},{0,0},{0,0},{0,0},{0,0}}, true},// Battlepass
		},
		{//Kadýn Ninja
			{EQUIPMENT, WEAR_WEAPON, 1, 21930, {get_global_time() + 604800,0,0,0}, {{72,100},{0,0},{0,0},{0,0},{0,0}}, false},// Silah
			{EQUIPMENT, WEAR_BODY, 1, 11499, {0,0,0,0}, {{1,2000},{23,10},{24,10},{53,50},{9,20}}, false},// Zýrh
			{EQUIPMENT, WEAR_HEAD, 1, 12389, {0,0,0,0}, {{18,20},{20,20},{21,20},{22,20},{12,8}}, false},// Kask
			{EQUIPMENT, WEAR_SHIELD, 1, 13049, {0,0,0,0}, {{48,1},{18,20},{20,20},{21,20},{22,20}}, false},// Kalkan
			{EQUIPMENT, WEAR_WRIST, 1, 14209, {0,0,0,0}, {{1,2000},{18,20},{20,20},{21,20},{22,20}}, false},// Bilezik
			{EQUIPMENT, WEAR_FOOTS, 1, 15209, {0,0,0,0}, {{1,2000},{28,15},{15,10},{0,0},{0,0}}, false},// Ayakkabý
			{EQUIPMENT, WEAR_NECK, 1, 16209, {0,0,0,0}, {{1,2000},{15,10},{16,10},{18,20},{19,20}}, false},// Kolye
			{EQUIPMENT, WEAR_EAR, 1, 17109, {0,0,0,0}, {{18,20},{20,20},{21,20},{22,20},{0,0}}, false},// Küpe

			{EQUIPMENT, WEAR_COSTUME_BODY, 1, 43431, {get_global_time() + 86313600,0,0,0}, {{0,0},{0,0},{0,0},{0,0},{0,0}}, false},// Zýrh Kostümü
			{EQUIPMENT, WEAR_COSTUME_HAIR, 1, 43433, {get_global_time() + 86313600,0,0,0}, {{0,0},{0,0},{0,0},{0,0},{0,0}}, false},// Saç Kostümü
			{EQUIPMENT, WEAR_COSTUME_MOUNT, 1, 15592, {get_global_time() + 86313600,0,0,0}, {{0,0},{0,0},{0,0},{0,0},{0,0}}, false},// Binek Kostümü
			{EQUIPMENT, WEAR_COSTUME_WEAPON, 1, 43405, {get_global_time() + 86313600,0,0,0}, {{0,0},{0,0},{0,0},{0,0},{0,0}}, false},// Silah Kostümü
			{EQUIPMENT, WEAR_PET, 1, 43101, {get_global_time() + 86313600,0,0,0}, {{0,0},{0,0},{0,0},{0,0},{0,0}}, false},// Pet

			{INVENTORY, 37, 1, 72726, {0,0,7000000,0}, {{0,0},{0,0},{0,0},{0,0},{0,0}}, true},// Güneþ Özütü
			{INVENTORY, 35, 1, 72730, {0,0,700000,0}, {{0,0},{0,0},{0,0},{0,0},{0,0}}, true},// Ay Özütü
			{INVENTORY, 27, 1, 70058, {0,0,0,0}, {{0,0},{0,0},{0,0},{0,0},{0,0}}, true},// Iþýnlanma Yüzüðü

			{INVENTORY, 28, 1, 39006, {0,0,0,0}, {{0,0},{0,0},{0,0},{0,0},{0,0}}, true},// Pelerin
			

			{INVENTORY, 0, 1, 21932, {get_global_time() + 604800,0,0,0}, {{72,100},{0,0},{0,0},{0,0},{0,0}}, false},// Yay
			{INVENTORY, 1, 1, 43406, {get_global_time() + 86313600,0,0,0}, {{0,0},{0,0},{0,0},{0,0},{0,0}}, false},// Yay Kostümü
			{INVENTORY, 2, 1, 21931, {get_global_time() + 604800,0,0,0}, {{72,100},{0,0},{0,0},{0,0},{0,0}}, false},// Býçak
			{INVENTORY, 3, 1, 43401, {get_global_time() + 86313600,0,0,0}, {{0,0},{0,0},{0,0},{0,0},{0,0}}, false},// Býçak Kostümü
			{INVENTORY, 4, 1, 43402, {get_global_time() + 86313600,0,0,0}, {{0,0},{0,0},{0,0},{0,0},{0,0}}, false},// Tek El Kostümü
			{INVENTORY, 10, 1, 19958, {0,0,0,0}, {{0,0},{0,0},{0,0},{0,0},{0,0}}, true},// Battlepass
		},
		{//Erkek Sura
			{EQUIPMENT, WEAR_WEAPON, 1, 21930, {get_global_time() + 604800,0,0,0}, {{72,100},{0,0},{0,0},{0,0},{0,0}}, false},// Silah
			{EQUIPMENT, WEAR_BODY, 1, 11699, {0,0,0,0}, {{1,2000},{23,10},{24,10},{53,50},{9,20}}, false},// Zýrh
			{EQUIPMENT, WEAR_HEAD, 1, 12529, {0,0,0,0}, {{18,20},{20,20},{21,20},{22,20},{12,8}}, false},// Kask
			{EQUIPMENT, WEAR_SHIELD, 1, 13049, {0,0,0,0}, {{48,1},{18,20},{20,20},{21,20},{22,20}}, false},// Kalkan
			{EQUIPMENT, WEAR_WRIST, 1, 14209, {0,0,0,0}, {{1,2000},{18,20},{20,20},{21,20},{22,20}}, false},// Bilezik
			{EQUIPMENT, WEAR_FOOTS, 1, 15209, {0,0,0,0}, {{1,2000},{28,15},{15,10},{0,0},{0,0}}, false},// Ayakkabý
			{EQUIPMENT, WEAR_NECK, 1, 16209, {0,0,0,0}, {{1,2000},{15,10},{16,10},{18,20},{19,20}}, false},// Kolye
			{EQUIPMENT, WEAR_EAR, 1, 17109, {0,0,0,0}, {{18,20},{20,20},{21,20},{22,20},{0,0}}, false},// Küpe

			{EQUIPMENT, WEAR_COSTUME_BODY, 1, 43430, {get_global_time() + 86313600,0,0,0}, {{0,0},{0,0},{0,0},{0,0},{0,0}}, false},// Zýrh Kostümü
			{EQUIPMENT, WEAR_COSTUME_HAIR, 1, 43432, {get_global_time() + 86313600,0,0,0}, {{0,0},{0,0},{0,0},{0,0},{0,0}}, false},// Saç Kostümü
			{EQUIPMENT, WEAR_COSTUME_MOUNT, 1, 15592, {get_global_time() + 86313600,0,0,0}, {{0,0},{0,0},{0,0},{0,0},{0,0}}, false},// Binek Kostümü
			{EQUIPMENT, WEAR_COSTUME_WEAPON, 1, 43405, {get_global_time() + 86313600,0,0,0}, {{0,0},{0,0},{0,0},{0,0},{0,0}}, false},// Silah Kostümü
			{EQUIPMENT, WEAR_PET, 1, 43101, {get_global_time() + 86313600,0,0,0}, {{0,0},{0,0},{0,0},{0,0},{0,0}}, false},// Pet

			{INVENTORY, 37, 1, 72726, {0,0,7000000,0}, {{0,0},{0,0},{0,0},{0,0},{0,0}}, true},// Güneþ Özütü
			{INVENTORY, 35, 1, 72730, {0,0,700000,0}, {{0,0},{0,0},{0,0},{0,0},{0,0}}, true},// Ay Özütü
			{INVENTORY, 27, 1, 70058, {0,0,0,0}, {{0,0},{0,0},{0,0},{0,0},{0,0}}, true},// Iþýnlanma Yüzüðü

			{INVENTORY, 28, 1, 39006, {0,0,0,0}, {{0,0},{0,0},{0,0},{0,0},{0,0}}, true},// Pelerin
			

			{INVENTORY, 0, 1, 43402, {get_global_time() + 86313600,0,0,0}, {{0,0},{0,0},{0,0},{0,0},{0,0}}, false},// Tek El Kostümü
			{INVENTORY, 1, 1, 19958, {0,0,0,0}, {{0,0},{0,0},{0,0},{0,0},{0,0}}, true},// Battlepass
		},
		{//Kadýn Þaman
			{EQUIPMENT, WEAR_WEAPON, 1, 21934, {get_global_time() + 604800,0,0,0}, {{72,100},{0,0},{0,0},{0,0},{0,0}}, false},// Silah
			{EQUIPMENT, WEAR_BODY, 1, 11899, {0,0,0,0}, {{1,2000},{23,10},{24,10},{53,50},{9,20}}, false},// Zýrh
			{EQUIPMENT, WEAR_HEAD, 1, 12669, {0,0,0,0}, {{18,20},{20,20},{21,20},{22,20},{12,8}}, false},// Kask
			{EQUIPMENT, WEAR_SHIELD, 1, 13049, {0,0,0,0}, {{48,1},{18,20},{20,20},{21,20},{22,20}}, false},// Kalkan
			{EQUIPMENT, WEAR_WRIST, 1, 14209, {0,0,0,0}, {{1,2000},{18,20},{20,20},{21,20},{22,20}}, false},// Bilezik
			{EQUIPMENT, WEAR_FOOTS, 1, 15209, {0,0,0,0}, {{1,2000},{28,15},{15,10},{0,0},{0,0}}, false},// Ayakkabý
			{EQUIPMENT, WEAR_NECK, 1, 16209, {0,0,0,0}, {{1,2000},{15,10},{16,10},{18,20},{19,20}}, false},// Kolye
			{EQUIPMENT, WEAR_EAR, 1, 17109, {0,0,0,0}, {{18,20},{20,20},{21,20},{22,20},{0,0}}, false},// Küpe

			{EQUIPMENT, WEAR_COSTUME_BODY, 1, 43431, {get_global_time() + 86313600,0,0,0}, {{0,0},{0,0},{0,0},{0,0},{0,0}}, false},// Zýrh Kostümü
			{EQUIPMENT, WEAR_COSTUME_HAIR, 1, 43433, {get_global_time() + 86313600,0,0,0}, {{0,0},{0,0},{0,0},{0,0},{0,0}}, false},// Saç Kostümü
			{EQUIPMENT, WEAR_COSTUME_MOUNT, 1, 15592, {get_global_time() + 86313600,0,0,0}, {{0,0},{0,0},{0,0},{0,0},{0,0}}, false},// Binek Kostümü
			{EQUIPMENT, WEAR_COSTUME_WEAPON, 1, 43400, {get_global_time() + 86313600,0,0,0}, {{0,0},{0,0},{0,0},{0,0},{0,0}}, false},// Silah Kostümü
			{EQUIPMENT, WEAR_PET, 1, 43101, {get_global_time() + 86313600,0,0,0}, {{0,0},{0,0},{0,0},{0,0},{0,0}}, false},// Pet

			{INVENTORY, 37, 1, 72726, {0,0,7000000,0}, {{0,0},{0,0},{0,0},{0,0},{0,0}}, true},// Güneþ Özütü
			{INVENTORY, 35, 1, 72730, {0,0,700000,0}, {{0,0},{0,0},{0,0},{0,0},{0,0}}, true},// Ay Özütü
			{INVENTORY, 27, 1, 70058, {0,0,0,0}, {{0,0},{0,0},{0,0},{0,0},{0,0}}, true},// Iþýnlanma Yüzüðü

			{INVENTORY, 28, 1, 39006, {0,0,0,0}, {{0,0},{0,0},{0,0},{0,0},{0,0}}, true},// Pelerin
			

			{INVENTORY, 0, 1, 21935, {get_global_time() + 604800,0,0,0}, {{72,100},{0,0},{0,0},{0,0},{0,0}}, false},// Yelpaze
			{INVENTORY, 1, 1, 43407, {get_global_time() + 86313600,0,0,0}, {{0,0},{0,0},{0,0},{0,0},{0,0}}, false},// Yelpaze Kostümü
			{INVENTORY, 2, 1, 19958, {0,0,0,0}, {{0,0},{0,0},{0,0},{0,0},{0,0}}, true},// Battlepass
		},
		{//Kadýn Savaþçý
			{EQUIPMENT, WEAR_WEAPON, 1, 21933, {get_global_time() + 604800,0,0,0}, {{72,100},{0,0},{0,0},{0,0},{0,0}}, false},// Silah
			{EQUIPMENT, WEAR_BODY, 1, 11299, {0,0,0,0}, {{1,2000},{23,10},{24,10},{53,50},{9,20}}, false},// Zýrh
			{EQUIPMENT, WEAR_HEAD, 1, 12249, {0,0,0,0}, {{18,20},{20,20},{21,20},{22,20},{12,8}}, false},// Kask
			{EQUIPMENT, WEAR_SHIELD, 1, 13049, {0,0,0,0}, {{48,1},{18,20},{20,20},{21,20},{22,20}}, false},// Kalkan
			{EQUIPMENT, WEAR_WRIST, 1, 14209, {0,0,0,0}, {{1,2000},{18,20},{20,20},{21,20},{22,20}}, false},// Bilezik
			{EQUIPMENT, WEAR_FOOTS, 1, 15209, {0,0,0,0}, {{1,2000},{28,15},{15,10},{0,0},{0,0}}, false},// Ayakkabý
			{EQUIPMENT, WEAR_NECK, 1, 16209, {0,0,0,0}, {{1,2000},{15,10},{16,10},{18,20},{19,20}}, false},// Kolye
			{EQUIPMENT, WEAR_EAR, 1, 17109, {0,0,0,0}, {{18,20},{20,20},{21,20},{22,20},{0,0}}, false},// Küpe

			{EQUIPMENT, WEAR_COSTUME_BODY, 1, 43431, {get_global_time() + 86313600,0,0,0}, {{0,0},{0,0},{0,0},{0,0},{0,0}}, false},// Zýrh Kostümü
			{EQUIPMENT, WEAR_COSTUME_HAIR, 1, 43433, {get_global_time() + 86313600,0,0,0}, {{0,0},{0,0},{0,0},{0,0},{0,0}}, false},// Saç Kostümü
			{EQUIPMENT, WEAR_COSTUME_MOUNT, 1, 15592, {get_global_time() + 86313600,0,0,0}, {{0,0},{0,0},{0,0},{0,0},{0,0}}, false},// Binek Kostümü
			{EQUIPMENT, WEAR_COSTUME_WEAPON, 1, 43403, {get_global_time() + 86313600,0,0,0}, {{0,0},{0,0},{0,0},{0,0},{0,0}}, false},// Silah Kostümü
			{EQUIPMENT, WEAR_PET, 1, 43101, {get_global_time() + 86313600,0,0,0}, {{0,0},{0,0},{0,0},{0,0},{0,0}}, false},// Pet

			{INVENTORY, 37, 1, 72726, {0,0,7000000,0}, {{0,0},{0,0},{0,0},{0,0},{0,0}}, true},// Güneþ Özütü
			{INVENTORY, 35, 1, 72730, {0,0,700000,0}, {{0,0},{0,0},{0,0},{0,0},{0,0}}, true},// Ay Özütü
			{INVENTORY, 27, 1, 70058, {0,0,0,0}, {{0,0},{0,0},{0,0},{0,0},{0,0}}, true},// Iþýnlanma Yüzüðü

			{INVENTORY, 28, 1, 39006, {0,0,0,0}, {{0,0},{0,0},{0,0},{0,0},{0,0}}, true},// Pelerin
			

			{INVENTORY, 0, 1, 21930, {get_global_time() + 604800,0,0,0}, {{72,100},{0,0},{0,0},{0,0},{0,0}}, false},// Tek El
			{INVENTORY, 1, 1, 43405, {get_global_time() + 86313600,0,0,0}, {{0,0},{0,0},{0,0},{0,0},{0,0}}, false},// Tek El Kostümü
			{INVENTORY, 2, 1, 43402, {get_global_time() + 86313600,0,0,0}, {{0,0},{0,0},{0,0},{0,0},{0,0}}, false},// Tek El Kostümü
			{INVENTORY, 3, 1, 19958, {0,0,0,0}, {{0,0},{0,0},{0,0},{0,0},{0,0}}, true},// Battlepass
		},
		{//Erkek Ninja
			{EQUIPMENT, WEAR_WEAPON, 1, 21930, {get_global_time() + 604800,0,0,0}, {{72,100},{0,0},{0,0},{0,0},{0,0}}, false},// Silah
			{EQUIPMENT, WEAR_BODY, 1, 11499, {0,0,0,0}, {{1,2000},{23,10},{24,10},{53,50},{9,20}}, false},// Zýrh
			{EQUIPMENT, WEAR_HEAD, 1, 12389, {0,0,0,0}, {{18,20},{20,20},{21,20},{22,20},{12,8}}, false},// Kask
			{EQUIPMENT, WEAR_SHIELD, 1, 13049, {0,0,0,0}, {{48,1},{18,20},{20,20},{21,20},{22,20}}, false},// Kalkan
			{EQUIPMENT, WEAR_WRIST, 1, 14209, {0,0,0,0}, {{1,2000},{18,20},{20,20},{21,20},{22,20}}, false},// Bilezik
			{EQUIPMENT, WEAR_FOOTS, 1, 15209, {0,0,0,0}, {{1,2000},{28,15},{15,10},{0,0},{0,0}}, false},// Ayakkabý
			{EQUIPMENT, WEAR_NECK, 1, 16209, {0,0,0,0}, {{1,2000},{15,10},{16,10},{18,20},{19,20}}, false},// Kolye
			{EQUIPMENT, WEAR_EAR, 1, 17109, {0,0,0,0}, {{18,20},{20,20},{21,20},{22,20},{0,0}}, false},// Küpe

			{EQUIPMENT, WEAR_COSTUME_BODY, 1, 43430, {get_global_time() + 86313600,0,0,0}, {{0,0},{0,0},{0,0},{0,0},{0,0}}, false},// Zýrh Kostümü
			{EQUIPMENT, WEAR_COSTUME_HAIR, 1, 43432, {get_global_time() + 86313600,0,0,0}, {{0,0},{0,0},{0,0},{0,0},{0,0}}, false},// Saç Kostümü
			{EQUIPMENT, WEAR_COSTUME_MOUNT, 1, 15592, {get_global_time() + 86313600,0,0,0}, {{0,0},{0,0},{0,0},{0,0},{0,0}}, false},// Binek Kostümü
			{EQUIPMENT, WEAR_COSTUME_WEAPON, 1, 43405, {get_global_time() + 86313600,0,0,0}, {{0,0},{0,0},{0,0},{0,0},{0,0}}, false},// Silah Kostümü
			{EQUIPMENT, WEAR_PET, 1, 43101, {get_global_time() + 86313600,0,0,0}, {{0,0},{0,0},{0,0},{0,0},{0,0}}, false},// Pet

			{INVENTORY, 37, 1, 72726, {0,0,7000000,0}, {{0,0},{0,0},{0,0},{0,0},{0,0}}, true},// Güneþ Özütü
			{INVENTORY, 35, 1, 72730, {0,0,700000,0}, {{0,0},{0,0},{0,0},{0,0},{0,0}}, true},// Ay Özütü
			{INVENTORY, 27, 1, 70058, {0,0,0,0}, {{0,0},{0,0},{0,0},{0,0},{0,0}}, true},// Iþýnlanma Yüzüðü

			{INVENTORY, 28, 1, 39006, {0,0,0,0}, {{0,0},{0,0},{0,0},{0,0},{0,0}}, true},// Pelerin
			

			{INVENTORY, 0, 1, 21932, {get_global_time() + 604800,0,0,0}, {{72,100},{0,0},{0,0},{0,0},{0,0}}, false},// Yay
			{INVENTORY, 1, 1, 43406, {get_global_time() + 86313600,0,0,0}, {{0,0},{0,0},{0,0},{0,0},{0,0}}, false},// Yay Kostümü
			{INVENTORY, 2, 1, 21931, {get_global_time() + 604800,0,0,0}, {{72,100},{0,0},{0,0},{0,0},{0,0}}, false},// Býçak
			{INVENTORY, 3, 1, 43401, {get_global_time() + 86313600,0,0,0}, {{0,0},{0,0},{0,0},{0,0},{0,0}}, false},// Býçak Kostümü
			{INVENTORY, 4, 1, 43402, {get_global_time() + 86313600,0,0,0}, {{0,0},{0,0},{0,0},{0,0},{0,0}}, false},// Tek El Kostümü
			{INVENTORY, 10, 1, 19958, {0,0,0,0}, {{0,0},{0,0},{0,0},{0,0},{0,0}}, true},// Battlepass
		},
		{//Kadýn Sura
			{EQUIPMENT, WEAR_WEAPON, 1, 21930, {get_global_time() + 604800,0,0,0}, {{72,100},{0,0},{0,0},{0,0},{0,0}}, false},// Silah
			{EQUIPMENT, WEAR_BODY, 1, 11699, {0,0,0,0}, {{1,2000},{23,10},{24,10},{53,50},{9,20}}, false},// Zýrh
			{EQUIPMENT, WEAR_HEAD, 1, 12529, {0,0,0,0}, {{18,20},{20,20},{21,20},{22,20},{12,8}}, false},// Kask
			{EQUIPMENT, WEAR_SHIELD, 1, 13049, {0,0,0,0}, {{48,1},{18,20},{20,20},{21,20},{22,20}}, false},// Kalkan
			{EQUIPMENT, WEAR_WRIST, 1, 14209, {0,0,0,0}, {{1,2000},{18,20},{20,20},{21,20},{22,20}}, false},// Bilezik
			{EQUIPMENT, WEAR_FOOTS, 1, 15209, {0,0,0,0}, {{1,2000},{28,15},{15,10},{0,0},{0,0}}, false},// Ayakkabý
			{EQUIPMENT, WEAR_NECK, 1, 16209, {0,0,0,0}, {{1,2000},{15,10},{16,10},{18,20},{19,20}}, false},// Kolye
			{EQUIPMENT, WEAR_EAR, 1, 17109, {0,0,0,0}, {{18,20},{20,20},{21,20},{22,20},{0,0}}, false},// Küpe

			{EQUIPMENT, WEAR_COSTUME_BODY, 1, 43431, {get_global_time() + 86313600,0,0,0}, {{0,0},{0,0},{0,0},{0,0},{0,0}}, false},// Zýrh Kostümü
			{EQUIPMENT, WEAR_COSTUME_HAIR, 1, 43433, {get_global_time() + 86313600,0,0,0}, {{0,0},{0,0},{0,0},{0,0},{0,0}}, false},// Saç Kostümü
			{EQUIPMENT, WEAR_COSTUME_MOUNT, 1, 15592, {get_global_time() + 86313600,0,0,0}, {{0,0},{0,0},{0,0},{0,0},{0,0}}, false},// Binek Kostümü
			{EQUIPMENT, WEAR_COSTUME_WEAPON, 1, 43405, {get_global_time() + 86313600,0,0,0}, {{0,0},{0,0},{0,0},{0,0},{0,0}}, false},// Silah Kostümü
			{EQUIPMENT, WEAR_PET, 1, 43101, {get_global_time() + 86313600,0,0,0}, {{0,0},{0,0},{0,0},{0,0},{0,0}}, false},// Pet

			{INVENTORY, 37, 1, 72726, {0,0,7000000,0}, {{0,0},{0,0},{0,0},{0,0},{0,0}}, true},// Güneþ Özütü
			{INVENTORY, 35, 1, 72730, {0,0,700000,0}, {{0,0},{0,0},{0,0},{0,0},{0,0}}, true},// Ay Özütü
			{INVENTORY, 27, 1, 70058, {0,0,0,0}, {{0,0},{0,0},{0,0},{0,0},{0,0}}, true},// Iþýnlanma Yüzüðü

			{INVENTORY, 28, 1, 39006, {0,0,0,0}, {{0,0},{0,0},{0,0},{0,0},{0,0}}, true},// Pelerin
			

			{INVENTORY, 0, 1, 43402, {get_global_time() + 86313600,0,0,0}, {{0,0},{0,0},{0,0},{0,0},{0,0}}, false},// Tek El Kostümü
			{INVENTORY, 1, 1, 19958, {0,0,0,0}, {{0,0},{0,0},{0,0},{0,0},{0,0}}, true},// Battlepass
		},
		{//Erkek Þaman
			{EQUIPMENT, WEAR_WEAPON, 1, 21934, {get_global_time() + 604800,0,0,0}, {{72,100},{0,0},{0,0},{0,0},{0,0}}, false},// Silah
			{EQUIPMENT, WEAR_BODY, 1, 11899, {0,0,0,0}, {{1,2000},{23,10},{24,10},{53,50},{9,20}}, false},// Zýrh
			{EQUIPMENT, WEAR_HEAD, 1, 12669, {0,0,0,0}, {{18,20},{20,20},{21,20},{22,20},{12,8}}, false},// Kask
			{EQUIPMENT, WEAR_SHIELD, 1, 13049, {0,0,0,0}, {{48,1},{18,20},{20,20},{21,20},{22,20}}, false},// Kalkan
			{EQUIPMENT, WEAR_WRIST, 1, 14209, {0,0,0,0}, {{1,2000},{18,20},{20,20},{21,20},{22,20}}, false},// Bilezik
			{EQUIPMENT, WEAR_FOOTS, 1, 15209, {0,0,0,0}, {{1,2000},{28,15},{15,10},{0,0},{0,0}}, false},// Ayakkabý
			{EQUIPMENT, WEAR_NECK, 1, 16209, {0,0,0,0}, {{1,2000},{15,10},{16,10},{18,20},{19,20}}, false},// Kolye
			{EQUIPMENT, WEAR_EAR, 1, 17109, {0,0,0,0}, {{18,20},{20,20},{21,20},{22,20},{0,0}}, false},// Küpe

			{EQUIPMENT, WEAR_COSTUME_BODY, 1, 43430, {get_global_time() + 86313600,0,0,0}, {{0,0},{0,0},{0,0},{0,0},{0,0}}, false},// Zýrh Kostümü
			{EQUIPMENT, WEAR_COSTUME_HAIR, 1, 43432, {get_global_time() + 86313600,0,0,0}, {{0,0},{0,0},{0,0},{0,0},{0,0}}, false},// Saç Kostümü
			{EQUIPMENT, WEAR_COSTUME_MOUNT, 1, 15592, {get_global_time() + 86313600,0,0,0}, {{0,0},{0,0},{0,0},{0,0},{0,0}}, false},// Binek Kostümü
			{EQUIPMENT, WEAR_COSTUME_WEAPON, 1, 43400, {get_global_time() + 86313600,0,0,0}, {{0,0},{0,0},{0,0},{0,0},{0,0}}, false},// Silah Kostümü
			{EQUIPMENT, WEAR_PET, 1, 43101, {get_global_time() + 86313600,0,0,0}, {{0,0},{0,0},{0,0},{0,0},{0,0}}, false},// Pet

			{INVENTORY, 37, 1, 72726, {0,0,7000000,0}, {{0,0},{0,0},{0,0},{0,0},{0,0}}, true},// Güneþ Özütü
			{INVENTORY, 35, 1, 72730, {0,0,700000,0}, {{0,0},{0,0},{0,0},{0,0},{0,0}}, true},// Ay Özütü
			{INVENTORY, 27, 1, 70058, {0,0,0,0}, {{0,0},{0,0},{0,0},{0,0},{0,0}}, true},// Iþýnlanma Yüzüðü

			{INVENTORY, 28, 1, 39006, {0,0,0,0}, {{0,0},{0,0},{0,0},{0,0},{0,0}}, true},// Pelerin
			

			{INVENTORY, 0, 1, 21935, {get_global_time() + 604800,0,0,0}, {{72,100},{0,0},{0,0},{0,0},{0,0}}, false},// Yelpaze
			{INVENTORY, 1, 1, 43407, {get_global_time() + 86313600,0,0,0}, {{0,0},{0,0},{0,0},{0,0},{0,0}}, false},// Yelpaze Kostümü
			{INVENTORY, 2, 1, 19958, {0,0,0,0}, {{0,0},{0,0},{0,0},{0,0},{0,0}}, true},// Battlepass
		},
#ifdef __WOLFMAN_CHARACTER__
		{//lycan
			{EQUIPMENT, WEAR_WEAPON, 1, 21936, {get_global_time() + 604800,0,0,0}, {{72,100},{0,0},{0,0},{0,0},{0,0}}, false},// Silah
			{EQUIPMENT, WEAR_BODY, 1, 21079, {0,0,0,0}, {{1,2000},{23,10},{24,10},{53,50},{9,20}}, false},// Zýrh
			{EQUIPMENT, WEAR_HEAD, 1, 21529, {0,0,0,0}, {{18,20},{20,20},{21,20},{22,20},{12,8}}, false},// Kask
			{EQUIPMENT, WEAR_SHIELD, 1, 13049, {0,0,0,0}, {{48,1},{18,20},{20,20},{21,20},{22,20}}, false},// Kalkan
			{EQUIPMENT, WEAR_WRIST, 1, 14209, {0,0,0,0}, {{1,2000},{18,20},{20,20},{21,20},{22,20}}, false},// Bilezik
			{EQUIPMENT, WEAR_FOOTS, 1, 15209, {0,0,0,0}, {{1,2000},{28,15},{15,10},{0,0},{0,0}}, false},// Ayakkabý
			{EQUIPMENT, WEAR_NECK, 1, 16209, {0,0,0,0}, {{1,2000},{15,10},{16,10},{18,20},{19,20}}, false},// Kolye
			{EQUIPMENT, WEAR_EAR, 1, 17109, {0,0,0,0}, {{18,20},{20,20},{21,20},{22,20},{0,0}}, false},// Küpe

			{EQUIPMENT, WEAR_COSTUME_BODY, 1, 43430, {get_global_time() + 86313600,0,0,0}, {{0,0},{0,0},{0,0},{0,0},{0,0}}, false},// Zýrh Kostümü
			{EQUIPMENT, WEAR_COSTUME_HAIR, 1, 43432, {get_global_time() + 86313600,0,0,0}, {{0,0},{0,0},{0,0},{0,0},{0,0}}, false},// Saç Kostümü
			{EQUIPMENT, WEAR_COSTUME_MOUNT, 1, 15592, {get_global_time() + 86313600,0,0,0}, {{0,0},{0,0},{0,0},{0,0},{0,0}}, false},// Binek Kostümü
			{EQUIPMENT, WEAR_COSTUME_WEAPON, 1, 43404, {get_global_time() + 86313600,0,0,0}, {{0,0},{0,0},{0,0},{0,0},{0,0}}, false},// Silah Kostümü
			{EQUIPMENT, WEAR_PET, 1, 43101, {get_global_time() + 86313600,0,0,0}, {{0,0},{0,0},{0,0},{0,0},{0,0}}, false},// Pet

			{INVENTORY, 0, 1, 19958, {0,0,0,0}, {{0,0},{0,0},{0,0},{0,0},{0,0}}, true},// Battlepass

			{INVENTORY, 37, 1, 72726, {0,0,7000000,0}, {{0,0},{0,0},{0,0},{0,0},{0,0}}, true},// Güneþ Özütü
			{INVENTORY, 35, 1, 72730, {0,0,700000,0}, {{0,0},{0,0},{0,0},{0,0},{0,0}}, true},// Ay Özütü
			{INVENTORY, 27, 1, 70058, {0,0,0,0}, {{0,0},{0,0},{0,0},{0,0},{0,0}}, true},// Iþýnlanma Yüzüðü

			{INVENTORY, 28, 1, 39006, {0,0,0,0}, {{0,0},{0,0},{0,0},{0,0},{0,0}}, true},// Pelerin
		},
#endif
	};

	unsigned job = pPacketDB->player.byJob;

	//RaceToJob(job, &job);

	for (int i = 0; i < MAX_INITIAL_ITEM; i++)
	{
		if (initialItems[job][i].dwVnum == 0)
			continue;

		t.id = ITEM_MANAGER::instance().GetNewID();
		t.window = initialItems[job][i].window;
		t.pos = initialItems[job][i].pos;
		t.count = initialItems[job][i].count;
		t.vnum = initialItems[job][i].dwVnum;
		for (int x = 0; x < ITEM_SOCKET_MAX_NUM; ++x)
			t.alSockets[x] = initialItems[job][i].alSockets[x];
		for (int x = 0; x < 5; ++x)
		{
			t.aAttr[x].bType = initialItems[job][i].aAttr[x].bType;
			t.aAttr[x].sValue = initialItems[job][i].aAttr[x].sValue;
		}
#ifdef __BEGINNER_ITEM__
		t.is_basic = initialItems[job][i].is_basic;
#endif

		db_clientdesc->DBPacketHeader(HEADER_GD_ITEM_SAVE, 0, sizeof(TPlayerItem));
		db_clientdesc->Packet(&t, sizeof(TPlayerItem));
	}
	// END_OF_NEW_GIVE_BASIC
#endif
}

void CInputDB::PlayerDeleteSuccess(LPDESC d, const char* data)
{
	if (!d)
		return;

	BYTE account_index;
	account_index = decode_byte(data);
	d->BufferedPacket(encode_byte(HEADER_GC_CHARACTER_DELETE_SUCCESS), 1);
	d->Packet(encode_byte(account_index), 1);

	d->GetAccountTable().players[account_index].dwID = 0;
}

void CInputDB::PlayerDeleteFail(LPDESC d)
{
	if (!d)
		return;

	d->Packet(encode_byte(HEADER_GC_CHARACTER_DELETE_WRONG_SOCIAL_ID), 1);
	//d->Packet(encode_byte(account_index),			1);

	//d->GetAccountTable().players[account_index].dwID = 0;
}

void CInputDB::ChangeName(LPDESC d, const char* data)
{
	if (!d)
		return;

	TPacketDGChangeName* p = (TPacketDGChangeName*)data;

	TAccountTable& r = d->GetAccountTable();

	if (!r.id)
		return;

	for (size_t i = 0; i < PLAYER_PER_ACCOUNT; ++i)
		if (r.players[i].dwID == p->pid)
		{
			strlcpy(r.players[i].szName, p->name, sizeof(r.players[i].szName));
			r.players[i].bChangeName = 0;

			TPacketGCChangeName pgc;

			pgc.header = HEADER_GC_CHANGE_NAME;
			pgc.pid = p->pid;
			strlcpy(pgc.name, p->name, sizeof(pgc.name));

			d->Packet(&pgc, sizeof(TPacketGCChangeName));
			break;
		}
}

void CInputDB::PlayerLoad(LPDESC d, const char* data)
{
	TPlayerTable* pTab = (TPlayerTable*)data;

	if (!d)
		return;

	long lMapIndex = pTab->lMapIndex;
	PIXEL_POSITION pos;

	if (lMapIndex == 0)
	{
		lMapIndex = SECTREE_MANAGER::instance().GetMapIndex(pTab->x, pTab->y);

		if (lMapIndex == 0)
		{
			lMapIndex = EMPIRE_START_MAP(d->GetAccountTable().bEmpire);
			pos.x = EMPIRE_START_X(d->GetAccountTable().bEmpire);
			pos.y = EMPIRE_START_Y(d->GetAccountTable().bEmpire);
		}
		else
		{
			pos.x = pTab->x;
			pos.y = pTab->y;
		}
	}
	pTab->lMapIndex = lMapIndex;

	// ----

	// by rtsummit
	if (!SECTREE_MANAGER::instance().GetValidLocation(pTab->lMapIndex, pTab->x, pTab->y, lMapIndex, pos, d->GetEmpire()))
	{
		sys_err("InputDB::PlayerLoad : cannot find valid location %d x %d (name: %s)", pTab->x, pTab->y, pTab->name);
		// @duzenleme adamin haritasini oyunda bulamazsa adam bugda kalmiyor direkt olarak pozisyonu krallik pozisyonu olarak ayarlaniyor.
		lMapIndex = EMPIRE_START_MAP(d->GetAccountTable().bEmpire);
		pos.x = EMPIRE_START_X(d->GetAccountTable().bEmpire);
		pos.y = EMPIRE_START_Y(d->GetAccountTable().bEmpire);
	}

	pTab->x = pos.x;
	pTab->y = pos.y;
	pTab->lMapIndex = lMapIndex;

	if (d->GetCharacter() || d->IsPhase(PHASE_GAME))
	{
		LPCHARACTER p = d->GetCharacter();
		sys_err("login state already has main state (character %s %p)", p->GetName(), get_pointer(p));
		return;
	}

	if (NULL != CHARACTER_MANAGER::Instance().FindPC(pTab->name))
	{
		sys_err("InputDB: PlayerLoad : %s already exist in game", pTab->name);
		return;
	}

	LPCHARACTER ch = CHARACTER_MANAGER::instance().CreateCharacter(pTab->name, pTab->id);

	ch->BindDesc(d);
	ch->SetPlayerProto(pTab);
	ch->SetEmpire(d->GetEmpire());

	d->BindCharacter(ch);

	{
		// P2P Login
		TPacketGGLogin p;

		p.bHeader = HEADER_GG_LOGIN;
		strlcpy(p.szName, ch->GetName(), sizeof(p.szName));
		p.dwPID = ch->GetPlayerID();
		p.bEmpire = ch->GetEmpire();
		p.lMapIndex = SECTREE_MANAGER::instance().GetMapIndex(ch->GetX(), ch->GetY());
		p.bChannel = g_bChannel;

		P2P_MANAGER::instance().Send(&p, sizeof(TPacketGGLogin));
	}

	d->SetPhase(PHASE_LOADING);
	ch->MainCharacterPacket();

	long lPublicMapIndex = lMapIndex >= 10000 ? lMapIndex / 10000 : lMapIndex;

	//if (!map_allow_find(lMapIndex >= 10000 ? lMapIndex / 10000 : lMapIndex) || !CheckEmpire(ch, lMapIndex))
	if (!map_allow_find(lPublicMapIndex))
	{
		sys_err("InputDB::PlayerLoad : entering %d map is not allowed here (name: %s, empire %u)",
			lMapIndex, pTab->name, d->GetEmpire());

		ch->SetWarpLocation(EMPIRE_START_MAP(d->GetEmpire()),
			EMPIRE_START_X(d->GetEmpire()) / 100,
			EMPIRE_START_Y(d->GetEmpire()) / 100);

		d->SetPhase(PHASE_CLOSE);
		return;
	}

	quest::CQuestManager::instance().BroadcastEventFlagOnLogin(ch);

	for (int i = 0; i < QUICKSLOT_MAX_NUM; ++i)
		ch->SetQuickslot(i, pTab->quickslot[i]);

	ch->PointsPacket();
	ch->SkillLevelPacket();

	sys_log(0, "InputDB: player_load %s %dx%dx%d LEVEL %d MOV_SPEED %d JOB %d ATG %d DFG %d GMLv %d", pTab->name, ch->GetX(), ch->GetY(), ch->GetZ(), ch->GetLevel(), ch->GetPoint(POINT_MOV_SPEED), ch->GetJob(), ch->GetPoint(POINT_ATT_GRADE), ch->GetPoint(POINT_DEF_GRADE), ch->GetGMLevel());

	ch->QuerySafeboxSize();

#ifdef __HWID_SECURITY_UTILITY__
	d->SetCPUID(d->GetAccountTable().cpu_id);
	d->SetHDDModel(d->GetAccountTable().hdd_model);
	d->SetMachineGuid(d->GetAccountTable().machine_guid);
	d->SetMacAddr(d->GetAccountTable().mac_addr);
	d->SetHDDSerial(d->GetAccountTable().hdd_serial);
	d->SetBiosID(d->GetAccountTable().bios_id);
	d->SetComputerName(d->GetAccountTable().pc_name);
	d->SetComputerUserName(d->GetAccountTable().user_name);
	d->SetComputerOS(d->GetAccountTable().os_version);
#endif // __HWID_SECURITY_UTILITY__
}

void CInputDB::Boot(const char* data)
{
	signal_timer_disable();

	DWORD dwPacketSize = decode_4bytes(data);
	data += 4;

	BYTE bVersion = decode_byte(data);
	data += 1;

	sys_log(0, "BOOT: PACKET: %d", dwPacketSize);
	sys_log(0, "BOOT: VERSION: %d", bVersion);
	if (bVersion != 6)
	{
		sys_err("boot version error");
		thecore_shutdown();
	}

	sys_log(0, "sizeof(TMobTable) = %d", sizeof(TMobTable));
	sys_log(0, "sizeof(TItemTable) = %d", sizeof(TItemTable));
	sys_log(0, "sizeof(TShopTable) = %d", sizeof(TShopTable));
	sys_log(0, "sizeof(TSkillTable) = %d", sizeof(TSkillTable));
	sys_log(0, "sizeof(TRefineTable) = %d", sizeof(TRefineTable));
	sys_log(0, "sizeof(TItemAttrTable) = %d", sizeof(TItemAttrTable));
	sys_log(0, "sizeof(TItemRareTable) = %d", sizeof(TItemAttrTable));
	//ADMIN_MANAGER
	sys_log(0, "sizeof(TAdminManager) = %d", sizeof(TAdminInfo));
	//END_ADMIN_MANAGER

	WORD size;

	/*
	 * MOB
	 */

	if (decode_2bytes(data) != sizeof(TMobTable))
	{
		sys_err("mob table size error");
		thecore_shutdown();
		return;
	}
	data += 2;

	size = decode_2bytes(data);
	data += 2;
	sys_log(0, "BOOT: MOB: %d", size);

	if (size)
	{
		CMobManager::instance().Initialize((TMobTable*)data, size);
		data += size * sizeof(TMobTable);
	}

	/*
	 * ITEM
	 */

	if (decode_2bytes(data) != sizeof(TItemTable))
	{
		sys_err("item table size error");
		thecore_shutdown();
		return;
	}
	data += 2;

	size = decode_2bytes(data);
	data += 2;
	sys_log(0, "BOOT: ITEM: %d", size);

	if (size)
	{
		ITEM_MANAGER::instance().Initialize((TItemTable*)data, size);
		data += size * sizeof(TItemTable);
	}

	/*
	 * SHOP
	 */

	if (decode_2bytes(data) != sizeof(TShopTable))
	{
		sys_err("shop table size error");
		thecore_shutdown();
		return;
	}
	data += 2;

	size = decode_2bytes(data);
	data += 2;
	sys_log(0, "BOOT: SHOP: %d", size);

	if (size)
	{
		if (!CShopManager::instance().Initialize((TShopTable*)data, size))
		{
			sys_err("shop table Initialize error");
			thecore_shutdown();
			return;
		}
		data += size * sizeof(TShopTable);
	}

	/*
	 * SKILL
	 */

	if (decode_2bytes(data) != sizeof(TSkillTable))
	{
		sys_err("skill table size error");
		thecore_shutdown();
		return;
	}
	data += 2;

	size = decode_2bytes(data);
	data += 2;
	sys_log(0, "BOOT: SKILL: %d", size);

	if (size)
	{
		if (!CSkillManager::instance().Initialize((TSkillTable*)data, size))
		{
			sys_err("cannot initialize skill table");
			thecore_shutdown();
			return;
		}

		data += size * sizeof(TSkillTable);
	}
	/*
	 * REFINE RECIPE
	 */
	if (decode_2bytes(data) != sizeof(TRefineTable))
	{
		sys_err("refine table size error");
		thecore_shutdown();
		return;
	}
	data += 2;

	size = decode_2bytes(data);
	data += 2;
	sys_log(0, "BOOT: REFINE: %d", size);
	if (size)
	{
		CRefineManager::instance().Initialize((TRefineTable*)data, size);
		data += size * sizeof(TRefineTable);
	}

	/*
	 * ITEM ATTR
	 */
	if (decode_2bytes(data) != sizeof(TItemAttrTable))
	{
		sys_err("item attr table size error");
		thecore_shutdown();
		return;
	}
	data += 2;

	size = decode_2bytes(data);
	data += 2;
	sys_log(0, "BOOT: ITEM_ATTR: %d", size);

	if (size)
	{
		TItemAttrTable* p = (TItemAttrTable*)data;

		for (int i = 0; i < size; ++i, ++p)
		{
			if (p->dwApplyIndex >= MAX_APPLY_NUM)
				continue;

			g_map_itemAttr[p->dwApplyIndex] = *p;
			sys_log(0, "ITEM_ATTR[%d]: %s %u", p->dwApplyIndex, p->szApply, p->dwProb);
		}
	}

	data += size * sizeof(TItemAttrTable);

	/*
	 * ITEM RARE
	 */
	if (decode_2bytes(data) != sizeof(TItemAttrTable))
	{
		sys_err("item rare table size error");
		thecore_shutdown();
		return;
	}
	data += 2;

	size = decode_2bytes(data);
	data += 2;
	sys_log(0, "BOOT: ITEM_RARE: %d", size);

	if (size)
	{
		TItemAttrTable* p = (TItemAttrTable*)data;

		for (int i = 0; i < size; ++i, ++p)
		{
			if (p->dwApplyIndex >= MAX_APPLY_NUM)
				continue;

			g_map_itemRare[p->dwApplyIndex] = *p;
			sys_log(0, "ITEM_RARE[%d]: %s %u", p->dwApplyIndex, p->szApply, p->dwProb);
		}
	}

	data += size * sizeof(TItemAttrTable);

	set_global_time(*(time_t*)data);
	data += sizeof(time_t);

	if (decode_2bytes(data) != sizeof(TItemIDRangeTable))
	{
		sys_err("ITEM ID RANGE size error");
		thecore_shutdown();
		return;
	}
	data += 2;

	size = decode_2bytes(data);
	data += 2;

	TItemIDRangeTable* range = (TItemIDRangeTable*)data;
	data += size * sizeof(TItemIDRangeTable);

	TItemIDRangeTable* rangespare = (TItemIDRangeTable*)data;
	data += size * sizeof(TItemIDRangeTable);

	//ADMIN_MANAGER

	data += 2;
	int adminsize = decode_2bytes(data);
	data += 2;

	for (int n = 0; n < adminsize; ++n)
	{
		tAdminInfo& rAdminInfo = *(tAdminInfo*)data;

		gm_new_insert(rAdminInfo);

		data += sizeof(rAdminInfo);
	}

	sys_log(0, "[ADMIN] Size %d Count %d", sizeof(tAdminInfo), adminsize);

	//END_ADMIN_MANAGER

	WORD endCheck = decode_2bytes(data);
	if (endCheck != 0xffff)
	{
		sys_err("boot packet end check error [%x]!=0xffff", endCheck);
		thecore_shutdown();
		return;
	}
	else
		sys_log(0, "boot packet end check ok [%x]==0xffff", endCheck);
	data += 2;

	if (!ITEM_MANAGER::instance().SetMaxItemID(*range))
	{
		sys_err("not enough item id contact your administrator!");
		thecore_shutdown();
		return;
	}

	if (!ITEM_MANAGER::instance().SetMaxSpareItemID(*rangespare))
	{
		sys_err("not enough item id for spare contact your administrator!");
		thecore_shutdown();
		return;
	}

	// LOCALE_SERVICE
	const int FILE_NAME_LEN = 256;
	char szCommonDropItemFileName[FILE_NAME_LEN];
	char szETCDropItemFileName[FILE_NAME_LEN];
	char szMOBDropItemFileName[FILE_NAME_LEN];
	char szDropItemGroupFileName[FILE_NAME_LEN];
	char szSpecialItemGroupFileName[FILE_NAME_LEN];
	char szMapIndexFileName[FILE_NAME_LEN];
	char szItemVnumMaskTableFileName[FILE_NAME_LEN];
	char szDragonSoulTableFileName[FILE_NAME_LEN];
#ifdef __INGAME_MALL__
	char szItemShopTableFileName[FILE_NAME_LEN];
#endif

	snprintf(szCommonDropItemFileName, sizeof(szCommonDropItemFileName), "%s/common_drop_item.txt", LocaleService_GetBasePath().c_str());
	snprintf(szETCDropItemFileName, sizeof(szETCDropItemFileName), "%s/etc_drop_item.txt", LocaleService_GetBasePath().c_str());
	snprintf(szMOBDropItemFileName, sizeof(szMOBDropItemFileName), "%s/mob_drop_item.txt", LocaleService_GetBasePath().c_str());
	snprintf(szSpecialItemGroupFileName, sizeof(szSpecialItemGroupFileName), "%s/special_item_group.txt", LocaleService_GetBasePath().c_str());
	snprintf(szDropItemGroupFileName, sizeof(szDropItemGroupFileName), "%s/drop_item_group.txt", LocaleService_GetBasePath().c_str());
	snprintf(szMapIndexFileName, sizeof(szMapIndexFileName), "%s/index", LocaleService_GetMapPath().c_str());
	snprintf(szItemVnumMaskTableFileName, sizeof(szItemVnumMaskTableFileName), "%s/ori_to_new_table.txt", LocaleService_GetBasePath().c_str());
	snprintf(szDragonSoulTableFileName, sizeof(szDragonSoulTableFileName), "%s/dragon_soul_table.txt", LocaleService_GetBasePath().c_str());
#ifdef __INGAME_MALL__
	snprintf(szItemShopTableFileName, sizeof(szItemShopTableFileName), "%s/item_shop_table.txt", LocaleService_GetBasePath().c_str());
#endif

	sys_log(0, "Initializing Informations of Cube System");
	Cube_InformationInitialize();

	sys_log(0, "LoadLocaleFile: CommonDropItem: %s", szCommonDropItemFileName);
	if (!ITEM_MANAGER::instance().ReadCommonDropItemFile(szCommonDropItemFileName))
	{
		sys_err("cannot load CommonDropItem: %s", szCommonDropItemFileName);
		thecore_shutdown();
		return;
	}

	sys_log(0, "LoadLocaleFile: ETCDropItem: %s", szETCDropItemFileName);
	if (!ITEM_MANAGER::instance().ReadEtcDropItemFile(szETCDropItemFileName))
	{
		sys_err("cannot load ETCDropItem: %s", szETCDropItemFileName);
		thecore_shutdown();
		return;
	}

	sys_log(0, "LoadLocaleFile: DropItemGroup: %s", szDropItemGroupFileName);
	if (!ITEM_MANAGER::instance().ReadDropItemGroup(szDropItemGroupFileName))
	{
		sys_err("cannot load DropItemGroup: %s", szDropItemGroupFileName);
		thecore_shutdown();
		return;
	}

	sys_log(0, "LoadLocaleFile: SpecialItemGroup: %s", szSpecialItemGroupFileName);
	if (!ITEM_MANAGER::instance().ReadSpecialDropItemFile(szSpecialItemGroupFileName))
	{
		sys_err("cannot load SpecialItemGroup: %s", szSpecialItemGroupFileName);
		thecore_shutdown();
		return;
	}

	sys_log(0, "LoadLocaleFile: ItemVnumMaskTable : %s", szItemVnumMaskTableFileName);
	if (!ITEM_MANAGER::instance().ReadItemVnumMaskTable(szItemVnumMaskTableFileName))
	{
		sys_log(0, "Could not open MaskItemTable");
	}

	sys_log(0, "LoadLocaleFile: MOBDropItemFile: %s", szMOBDropItemFileName);
	if (!ITEM_MANAGER::instance().ReadMonsterDropItemGroup(szMOBDropItemFileName))
	{
		sys_err("cannot load MOBDropItemFile: %s", szMOBDropItemFileName);
		thecore_shutdown();
		return;
	}

	sys_log(0, "LoadLocaleFile: MapIndex: %s", szMapIndexFileName);
	if (!SECTREE_MANAGER::instance().Build(szMapIndexFileName, LocaleService_GetMapPath().c_str()))
	{
		sys_err("cannot load MapIndex: %s", szMapIndexFileName);
		thecore_shutdown();
		return;
	}

	sys_log(0, "LoadLocaleFile: DragonSoulTable: %s", szDragonSoulTableFileName);
	if (!DSManager::instance().ReadDragonSoulTableFile(szDragonSoulTableFileName))
	{
		sys_err("cannot load DragonSoulTable: %s", szDragonSoulTableFileName);
		//thecore_shutdown();
		//return;
	}

#ifdef __INGAME_MALL__
	sys_log(0, "LoadLocaleFile: ItemShopTable: %s", szItemShopTableFileName);
	if (!CItemShopManager::instance().LoadItemShopTable(szItemShopTableFileName))
	{
		sys_err("cannot load ItemShopTable: %s", szItemShopTableFileName);
		thecore_shutdown();
		return;
	}
#endif

#ifdef __BATTLE_PASS_SYSTEM__
	if (!CBattlePass::instance().ReadBattlePassFile())
		sys_err("Cannot load battle_pass.txt");
#endif

	// END_OF_LOCALE_SERVICE

	CMotionManager::instance().Build();

	signal_timer_enable(30);

	if (test_server)
	{
		CMobManager::instance().DumpRegenCount("mob_count");
	}
}

EVENTINFO(quest_login_event_info)
{
	DWORD dwPID;

	quest_login_event_info()
		: dwPID(0)
	{
	}
};

EVENTFUNC(quest_login_event)
{
	quest_login_event_info* info = dynamic_cast<quest_login_event_info*>(event->info);

	if (info == NULL)
	{
		sys_err("quest_login_event> <Factor> Null pointer");
		return 0;
	}

	DWORD dwPID = info->dwPID;

	LPCHARACTER ch = CHARACTER_MANAGER::instance().FindByPID(dwPID);

	if (!ch)
		return 0;

	LPDESC d = ch->GetDesc();

	if (!d)
		return 0;

	if (d->IsPhase(PHASE_HANDSHAKE) ||
		d->IsPhase(PHASE_LOGIN) ||
		d->IsPhase(PHASE_SELECT) ||
		d->IsPhase(PHASE_DEAD) ||
		d->IsPhase(PHASE_LOADING))
	{
		return PASSES_PER_SEC(1);
	}
	else if (d->IsPhase(PHASE_CLOSE))
	{
		return 0;
	}
	else if (d->IsPhase(PHASE_GAME))
	{
		sys_log(0, "QUEST_LOAD: Login pc %d by event", ch->GetPlayerID());
		quest::CQuestManager::instance().Login(ch->GetPlayerID());
		return 0;
	}
	else
	{
		sys_err("input_db.cpp:quest_login_event INVALID PHASE pid %d", ch->GetPlayerID());
		return 0;
	}
}

void CInputDB::QuestLoad(LPDESC d, const char* c_pData)
{
	if (NULL == d)
		return;

	LPCHARACTER ch = d->GetCharacter();

	if (NULL == ch)
		return;

	quest::PC* pkPC = quest::CQuestManager::instance().GetPCForce(ch->GetPlayerID());

	if (!pkPC)
	{
		sys_err("null quest::PC with id %u", ch->GetPlayerID());
		return;
	}

	if (pkPC->IsLoaded())
		return;

	const DWORD dwCount = decode_4bytes(c_pData);

	sys_log(0, "QUEST_LOAD: count %d", dwCount);

	const TQuestTable* pQuestTable = reinterpret_cast<const TQuestTable*>(c_pData + 4);

	if (NULL != pQuestTable)
	{
		if (dwCount != 0)
		{
			if (ch->GetPlayerID() != pQuestTable[0].dwPID)
			{
				sys_err("PID differs %u %u", ch->GetPlayerID(), pQuestTable[0].dwPID);
				return;
			}
		}

		for (unsigned int i = 0; i < dwCount; ++i)
		{
			std::string st(pQuestTable[i].szName);

			st += ".";
			st += pQuestTable[i].szState;

			// sys_log(0, "            %s %d", st.c_str(), pQuestTable[i].lValue);
			pkPC->SetFlag(st.c_str(), pQuestTable[i].lValue, true);
		}
	}

	pkPC->SetLoaded();
	pkPC->Build();

	if (ch->GetDesc()->IsPhase(PHASE_GAME))
	{
		sys_log(0, "QUEST_LOAD: Login pc %d", pQuestTable[0].dwPID);
		quest::CQuestManager::instance().Login(pQuestTable[0].dwPID);
	}
	else
	{
		quest_login_event_info* info = AllocEventInfo<quest_login_event_info>();
		info->dwPID = ch->GetPlayerID();

		event_create(quest_login_event, info, PASSES_PER_SEC(1));
	}
}

void CInputDB::SafeboxLoad(LPDESC d, const char* c_pData)
{
	if (!d)
		return;

	TSafeboxTable* p = (TSafeboxTable*)c_pData;

	if (d->GetAccountTable().id != p->dwID)
	{
		sys_err("SafeboxLoad: safebox has different id %u != %u", d->GetAccountTable().id, p->dwID);
		return;
	}

	if (!d->GetCharacter())
		return;

	BYTE bSize = 3;
	LPCHARACTER ch = d->GetCharacter();

	if (quest::CQuestManager::instance().GetPCForce(ch->GetPlayerID())->IsRunning() == true)
	{
		d->GetCharacter()->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_OPERATION_IS_FAILED_QUEST_WINDOW_IS_OPENED"));
		d->GetCharacter()->CancelSafeboxLoad();
		return;
	}

	if (!ch->CanAct(true, false, VAR_TIME_NONE, nullptr, VAR_REFINE | VAR_MYSHOP | VAR_EXCHANGE | VAR_ACCE | VAR_CHANGELOOK | VAR_OFFLINESHOP))
	{
		d->GetCharacter()->CancelSafeboxLoad();
		return;
	}
	// ADD_PREMIUM
	if (d->GetCharacter()->GetPremiumRemainSeconds(PREMIUM_SAFEBOX) > 0 || d->GetCharacter()->IsEquipUniqueItem(UNIQUE_ITEM_SAFEBOX_EXPAND) ||
		d->GetCharacter()->IsEquipUniqueGroup(UNIQUE_GROUP_LARGE_SAFEBOX))
		bSize = 3;
	// END_OF_ADD_PREMIUM

	//d->GetCharacter()->LoadSafebox(p->bSize * SAFEBOX_PAGE_SIZE, p->dwGold, p->wItemCount, (TPlayerItem *) (c_pData + sizeof(TSafeboxTable)));
#ifdef __GOLD_LIMIT_REWORK__
	d->GetCharacter()->LoadSafebox(bSize * SAFEBOX_PAGE_SIZE, p->llGold, p->wItemCount, (TPlayerItem*)(c_pData + sizeof(TSafeboxTable)));
#else
	d->GetCharacter()->LoadSafebox(bSize * SAFEBOX_PAGE_SIZE, p->dwGold, p->wItemCount, (TPlayerItem*)(c_pData + sizeof(TSafeboxTable)));
#endif
}

void CInputDB::SafeboxChangeSize(LPDESC d, const char* c_pData)
{
	if (!d)
		return;

	BYTE bSize = *(BYTE*)c_pData;

	if (!d->GetCharacter())
		return;

	d->GetCharacter()->ChangeSafeboxSize(bSize);
}

//
//
void CInputDB::SafeboxWrongPassword(LPDESC d)
{
	if (!d)
		return;

	if (!d->GetCharacter())
		return;

	TPacketCGSafeboxWrongPassword p;
	p.bHeader = HEADER_GC_SAFEBOX_WRONG_PASSWORD;
	d->Packet(&p, sizeof(p));

	d->GetCharacter()->CancelSafeboxLoad();
}

void CInputDB::SafeboxChangePasswordAnswer(LPDESC d, const char* c_pData)
{
	if (!d)
		return;

	if (!d->GetCharacter())
		return;

	TSafeboxChangePasswordPacketAnswer* p = (TSafeboxChangePasswordPacketAnswer*)c_pData;
	if (p->flag)
	{
		d->GetCharacter()->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<Ã¢°í> Ã¢°í ºñ¹Ð¹øÈ£°¡ º¯°æµÇ¾ú½À´Ï´Ù."));
	}
	else
	{
		d->GetCharacter()->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<Ã¢°í> ±âÁ¸ ºñ¹Ð¹øÈ£°¡ Æ²·È½À´Ï´Ù."));
	}
}

void CInputDB::MallLoad(LPDESC d, const char* c_pData)
{
	if (!d)
		return;

	TSafeboxTable* p = (TSafeboxTable*)c_pData;

	if (d->GetAccountTable().id != p->dwID)
	{
		sys_err("safebox has different id %u != %u", d->GetAccountTable().id, p->dwID);
		return;
	}

	if (!d->GetCharacter())
		return;

	d->GetCharacter()->LoadMall(p->wItemCount, (TPlayerItem*)(c_pData + sizeof(TSafeboxTable)));
}

void CInputDB::LoginAlready(LPDESC d, const char* c_pData)
{
	if (!d)
		return;

	{
		TPacketDGLoginAlready* p = (TPacketDGLoginAlready*)c_pData;

		LPDESC d2 = DESC_MANAGER::instance().FindByLoginName(p->szLogin);

		if (d2)
			d2->DisconnectOfSameLogin();
		else
		{
			TPacketGGDisconnect pgg;

			pgg.bHeader = HEADER_GG_DISCONNECT;
			strlcpy(pgg.szLogin, p->szLogin, sizeof(pgg.szLogin));
#ifdef __BAN_REASON_UTILITY__
			pgg.bBanned = false;
#endif // __BAN_REASON_UTILITY__

			P2P_MANAGER::instance().Send(&pgg, sizeof(TPacketGGDisconnect));
		}
	}
	// END_OF_INTERNATIONAL_VERSION

	LoginFailure(d, "ALREADY");
}

void CInputDB::EmpireSelect(LPDESC d, const char* c_pData)
{
	sys_log(0, "EmpireSelect %p", get_pointer(d));

	if (!d)
		return;

	TAccountTable& rTable = d->GetAccountTable();
	rTable.bEmpire = *(BYTE*)c_pData;

	TPacketGCEmpire pe;
	pe.bHeader = HEADER_GC_EMPIRE;
	pe.bEmpire = rTable.bEmpire;
	d->Packet(&pe, sizeof(pe));

	for (int i = 0; i < PLAYER_PER_ACCOUNT; ++i)
		if (rTable.players[i].dwID)
		{
			rTable.players[i].x = EMPIRE_START_X(rTable.bEmpire);
			rTable.players[i].y = EMPIRE_START_Y(rTable.bEmpire);
		}

	GetServerLocation(d->GetAccountTable(), rTable.bEmpire);

	d->SendLoginSuccessPacket();
}

void CInputDB::MapLocations(const char* c_pData)
{
	BYTE bCount = *(BYTE*)(c_pData++);

	sys_log(0, "InputDB::MapLocations %d", bCount);

	TMapLocation* pLoc = (TMapLocation*)c_pData;

	while (bCount--)
	{
		for (int i = 0; i < MAP_ALLOW_LIMIT; ++i)
		{
			if (0 == pLoc->alMaps[i])
				break;

			CMapLocation::instance().Insert(pLoc->alMaps[i], pLoc->szHost, pLoc->wPort);
		}

		pLoc++;
	}
}

void CInputDB::P2P(const char* c_pData)
{
	extern LPFDWATCH main_fdw;

	TPacketDGP2P* p = (TPacketDGP2P*)c_pData;

	P2P_MANAGER& mgr = P2P_MANAGER::instance();

	if (false == DESC_MANAGER::instance().IsP2PDescExist(p->szHost, p->wPort))
	{
		LPCLIENT_DESC pkDesc = NULL;
		sys_log(0, "InputDB:P2P %s:%u", p->szHost, p->wPort);
		pkDesc = DESC_MANAGER::instance().CreateConnectionDesc(main_fdw, p->szHost, p->wPort, PHASE_P2P, false);
		mgr.RegisterConnector(pkDesc);

		pkDesc->SetP2P(p->szHost, p->wPort, p->bChannel);
	}
}

void CInputDB::GuildLoad(const char* c_pData)
{
	CGuildManager::instance().LoadGuild(*(DWORD*)c_pData);
}

void CInputDB::GuildSkillUpdate(const char* c_pData)
{
	TPacketGuildSkillUpdate* p = (TPacketGuildSkillUpdate*)c_pData;

	CGuild* g = CGuildManager::instance().TouchGuild(p->guild_id);

	if (g)
	{
		g->UpdateSkill(p->skill_point, p->skill_levels);
		g->GuildPointChange(POINT_SP, p->amount, p->save ? true : false);
	}
}

void CInputDB::GuildWar(const char* c_pData)
{
	TPacketGuildWar* p = (TPacketGuildWar*)c_pData;
#ifdef __GUILD_WAR_REWORK__
	sys_log(0, "InputDB::GuildWar %u %u state %d maxplayer %d maxscore %d", p->dwGuildFrom, p->dwGuildTo, p->bWar, p->iMaxPlayer, p->iMaxScore);
#else
	sys_log(0, "InputDB::GuildWar %u %u state %d", p->dwGuildFrom, p->dwGuildTo, p->bWar);
#endif
	switch (p->bWar)
	{
	case GUILD_WAR_SEND_DECLARE:
	case GUILD_WAR_RECV_DECLARE:
		CGuildManager::instance().DeclareWar(p->dwGuildFrom, p->dwGuildTo, p->bType
#ifdef __GUILD_WAR_REWORK__
			, p->iMaxPlayer, p->iMaxScore
#endif
		);
		break;

	case GUILD_WAR_REFUSE:
		CGuildManager::instance().RefuseWar(p->dwGuildFrom, p->dwGuildTo
#ifdef __GUILD_WAR_REWORK__
			, p->iMaxPlayer, p->iMaxScore
#endif
		);
		break;

	case GUILD_WAR_WAIT_START:
		CGuildManager::instance().WaitStartWar(p->dwGuildFrom, p->dwGuildTo
#ifdef __GUILD_WAR_REWORK__
			, p->iMaxPlayer, p->iMaxScore
#endif
		);
		break;

	case GUILD_WAR_CANCEL:
		CGuildManager::instance().CancelWar(p->dwGuildFrom, p->dwGuildTo
#ifdef __GUILD_WAR_REWORK__
			, p->iMaxPlayer, p->iMaxScore
#endif
		);
		break;

	case GUILD_WAR_ON_WAR:
		CGuildManager::instance().StartWar(p->dwGuildFrom, p->dwGuildTo
#ifdef __GUILD_WAR_REWORK__
			, p->iMaxPlayer, p->iMaxScore
#endif
		);
		break;

	case GUILD_WAR_END:
		CGuildManager::instance().EndWar(p->dwGuildFrom, p->dwGuildTo);
		break;

	case GUILD_WAR_OVER:
		CGuildManager::instance().WarOver(p->dwGuildFrom, p->dwGuildTo, p->bType);
		break;

	default:
		sys_err("Unknown guild war state");
		break;
	}
}

void CInputDB::GuildWarScore(const char* c_pData)
{
	TPacketGuildWarScore* p = (TPacketGuildWarScore*)c_pData;
	CGuild* g = CGuildManager::instance().TouchGuild(p->dwGuildGainPoint);
	g->SetWarScoreAgainstTo(p->dwGuildOpponent, p->lScore);
}

void CInputDB::GuildSkillRecharge()
{
	CGuildManager::instance().SkillRecharge();
}

void CInputDB::GuildExpUpdate(const char* c_pData)
{
	TPacketGuildSkillUpdate* p = (TPacketGuildSkillUpdate*)c_pData;
	sys_log(1, "GuildExpUpdate %d", p->amount);

	CGuild* g = CGuildManager::instance().TouchGuild(p->guild_id);

	if (g)
		g->GuildPointChange(POINT_EXP, p->amount);
}

void CInputDB::GuildAddMember(const char* c_pData)
{
	TPacketDGGuildMember* p = (TPacketDGGuildMember*)c_pData;
	CGuild* g = CGuildManager::instance().TouchGuild(p->dwGuild);

	if (g)
		g->AddMember(p);
}

void CInputDB::GuildRemoveMember(const char* c_pData)
{
	TPacketGuild* p = (TPacketGuild*)c_pData;
	CGuild* g = CGuildManager::instance().TouchGuild(p->dwGuild);

	if (g)
		g->RemoveMember(p->dwInfo);
}

void CInputDB::GuildChangeGrade(const char* c_pData)
{
	TPacketGuild* p = (TPacketGuild*)c_pData;
	CGuild* g = CGuildManager::instance().TouchGuild(p->dwGuild);

	if (g)
		g->P2PChangeGrade((BYTE)p->dwInfo);
}

void CInputDB::GuildChangeMemberData(const char* c_pData)
{
	sys_log(0, "Recv GuildChangeMemberData");
	TPacketGuildChangeMemberData* p = (TPacketGuildChangeMemberData*)c_pData;
	CGuild* g = CGuildManager::instance().TouchGuild(p->guild_id);

	if (g)
		g->ChangeMemberData(p->pid, p->offer, p->level, p->grade);
}

void CInputDB::GuildDisband(const char* c_pData)
{
	TPacketGuild* p = (TPacketGuild*)c_pData;
	CGuildManager::instance().DisbandGuild(p->dwGuild);
}

void CInputDB::GuildLadder(const char* c_pData)
{
	TPacketGuildLadder* p = (TPacketGuildLadder*)c_pData;
	sys_log(0, "Recv GuildLadder %u %d / w %d d %d l %d", p->dwGuild, p->lLadderPoint, p->lWin, p->lDraw, p->lLoss);
	CGuild* g = CGuildManager::instance().TouchGuild(p->dwGuild);

	g->SetLadderPoint(p->lLadderPoint);
	g->SetWarData(p->lWin, p->lDraw, p->lLoss);
}

#ifdef __SKILL_COLOR__
void CInputDB::SkillColorLoad(LPDESC d, const char* c_pData)
{
	LPCHARACTER ch;

	if (!d || !(ch = d->GetCharacter()))
		return;

	ch->SetSkillColor((DWORD*)c_pData);
}
#endif

void CInputDB::ItemLoad(LPDESC d, const char* c_pData)
{
	LPCHARACTER ch;

	if (!d || !(ch = d->GetCharacter()))
		return;

	if (ch->IsItemLoaded())
		return;

	DWORD dwCount = decode_4bytes(c_pData);
	c_pData += sizeof(DWORD);

	sys_log(0, "ITEM_LOAD: COUNT %s %u", ch->GetName(), dwCount);

	std::vector<LPITEM> v;

	TPlayerItem* p = (TPlayerItem*)c_pData;

	for (DWORD i = 0; i < dwCount; ++i, ++p)
	{
		LPITEM item = ITEM_MANAGER::instance().CreateItem(p->vnum, p->count, p->id);

		if (!item)
		{
			sys_err("cannot create item by vnum %u (name %s id %u)", p->vnum, ch->GetName(), p->id);
			continue;
		}

		item->SetSkipSave(true);
		item->SetSockets(p->alSockets);
		item->SetAttributes(p->aAttr);
#ifdef __BEGINNER_ITEM__
		item->SetBasic(p->is_basic);
#endif
#ifdef __HIGHLIGHT_ITEM__
		item->SetLastOwnerPID(p->owner);
#endif
#ifdef __ITEM_EVOLUTION__
		item->SetEvolution(p->evolution);
#endif
#ifdef __ITEM_CHANGELOOK__
		item->SetTransmutation(p->transmutation);
#endif

#ifdef __MARTY_BELT_INVENTORY__
		if (p->window == BELT_INVENTORY)
		{
			p->window = INVENTORY;
			p->pos = p->pos + BELT_INVENTORY_SLOT_START;
		}
#endif

		if ((p->window == INVENTORY && ch->GetInventoryItem(p->pos)) ||
#ifdef __ADDITIONAL_INVENTORY__
			(p->window == UPGRADE_INVENTORY && ch->GetUpgradeInventoryItem(p->pos)) ||
			(p->window == BOOK_INVENTORY && ch->GetBookInventoryItem(p->pos)) ||
			(p->window == STONE_INVENTORY && ch->GetStoneInventoryItem(p->pos)) ||
			(p->window == FLOWER_INVENTORY && ch->GetFlowerInventoryItem(p->pos)) ||
			(p->window == ATTR_INVENTORY && ch->GetAttrInventoryItem(p->pos)) ||
			(p->window == CHEST_INVENTORY && ch->GetChestInventoryItem(p->pos)) ||
#endif
			(p->window == EQUIPMENT && ch->GetWear(p->pos))
			// @@
			// burada eger envanter disi esya varsa restore etmesi gerekiyor
			// edemedigi icin bazi sikintilar yaratabilir.
			)
		{
			sys_log(0, "ITEM_RESTORE: %s %s", ch->GetName(), item->GetName());
			v.push_back(item);
		}
		else
		{
			switch (p->window)
			{
			case INVENTORY:
			case DRAGON_SOUL_INVENTORY:
#ifdef __ADDITIONAL_INVENTORY__
			case UPGRADE_INVENTORY:
			case BOOK_INVENTORY:
			case STONE_INVENTORY:
			case FLOWER_INVENTORY:
			case ATTR_INVENTORY:
			case CHEST_INVENTORY:
#endif
#ifdef __SWITCHBOT__
			case SWITCHBOT:
#endif
				item->__ADD_TO_CHARACTER(ch, TItemPos(p->window, p->pos));
				break;

			case EQUIPMENT:
				if (item->CheckItemUseLevel(ch->GetLevel()) == true)
				{
					if (item->EquipTo(ch, p->pos) == false)
					{
						v.push_back(item);
					}
				}
				else
				{
					v.push_back(item);
				}
				break;
			}
		}

		if (false == item->OnAfterCreatedItem())
			sys_err("Failed to call ITEM::OnAfterCreatedItem (vnum: %d, id: %d)", item->GetVnum(), item->GetID());

		item->SetSkipSave(false);
	}

	itertype(v) it = v.begin();

	while (it != v.end())
	{
		LPITEM item = *(it++);

		int pos;
#ifdef __ADDITIONAL_INVENTORY__
		if (item->IsUpgradeItem())
			pos = ch->GetEmptyUpgradeInventory(item);
		else if (item->IsBook())
			pos = ch->GetEmptyBookInventory(item);
		else if (item->IsStone())
			pos = ch->GetEmptyStoneInventory(item);
		else if (item->IsFlower())
			pos = ch->GetEmptyFlowerInventory(item);
		else if (item->IsAttrItem())
			pos = ch->GetEmptyAttrInventory(item);
		else if (item->IsChest())
			pos = ch->GetEmptyChestInventory(item);
		else
#endif
			pos = ch->GetEmptyInventory(item->GetSize());

		if (pos < 0)
		{
			PIXEL_POSITION coord;
			coord.x = ch->GetX();
			coord.y = ch->GetY();

			item->AddToGround(ch->GetMapIndex(), coord);
			item->SetOwnership(ch, 180);
			item->StartDestroyEvent();
		}
		else
		{
#ifdef __ADDITIONAL_INVENTORY__
			if (item->IsUpgradeItem())
				item->__ADD_TO_CHARACTER(ch, TItemPos(UPGRADE_INVENTORY, pos));
			else if (item->IsBook())
				item->__ADD_TO_CHARACTER(ch, TItemPos(BOOK_INVENTORY, pos));
			else if (item->IsStone())
				item->__ADD_TO_CHARACTER(ch, TItemPos(STONE_INVENTORY, pos));
			else if (item->IsFlower())
				item->__ADD_TO_CHARACTER(ch, TItemPos(FLOWER_INVENTORY, pos));
			else if (item->IsAttrItem())
				item->__ADD_TO_CHARACTER(ch, TItemPos(ATTR_INVENTORY, pos));
			else if (item->IsChest())
				item->__ADD_TO_CHARACTER(ch, TItemPos(CHEST_INVENTORY, pos));
#endif
			else
				item->__ADD_TO_CHARACTER(ch, TItemPos(INVENTORY, pos));
		}
	}

	ch->CheckMaximumPoints();
	ch->PointsPacket();

	ch->SetItemLoaded();
}

void CInputDB::AffectLoad(LPDESC d, const char* c_pData)
{
	if (!d)
		return;

	if (!d->GetCharacter())
		return;

	LPCHARACTER ch = d->GetCharacter();

	DWORD dwPID = decode_4bytes(c_pData);
	c_pData += sizeof(DWORD);

	DWORD dwCount = decode_4bytes(c_pData);
	c_pData += sizeof(DWORD);

	if (ch->GetPlayerID() != dwPID)
		return;

	ch->LoadAffect(dwCount, (TPacketAffectElement*)c_pData);
}

void CInputDB::PartyCreate(const char* c_pData)
{
	TPacketPartyCreate* p = (TPacketPartyCreate*)c_pData;
	CPartyManager::instance().P2PCreateParty(p->dwLeaderPID);
}

void CInputDB::PartyDelete(const char* c_pData)
{
	TPacketPartyDelete* p = (TPacketPartyDelete*)c_pData;
	CPartyManager::instance().P2PDeleteParty(p->dwLeaderPID);
}

void CInputDB::PartyAdd(const char* c_pData)
{
	TPacketPartyAdd* p = (TPacketPartyAdd*)c_pData;
	CPartyManager::instance().P2PJoinParty(p->dwLeaderPID, p->dwPID, p->bState);
}

void CInputDB::PartyRemove(const char* c_pData)
{
	TPacketPartyRemove* p = (TPacketPartyRemove*)c_pData;
	CPartyManager::instance().P2PQuitParty(p->dwPID);
}

void CInputDB::PartyStateChange(const char* c_pData)
{
	TPacketPartyStateChange* p = (TPacketPartyStateChange*)c_pData;
	LPPARTY pParty = CPartyManager::instance().P2PCreateParty(p->dwLeaderPID);

	if (!pParty)
		return;

	pParty->SetRole(p->dwPID, p->bRole, p->bFlag);
}

void CInputDB::PartySetMemberLevel(const char* c_pData)
{
	TPacketPartySetMemberLevel* p = (TPacketPartySetMemberLevel*)c_pData;
	LPPARTY pParty = CPartyManager::instance().P2PCreateParty(p->dwLeaderPID);

	if (!pParty)
		return;

	pParty->P2PSetMemberLevel(p->dwPID, p->bLevel);
}

void CInputDB::Time(const char* c_pData)
{
	set_global_time(*(time_t*)c_pData);
}

void CInputDB::ReloadProto(const char* c_pData)
{
	WORD wSize;

	/*
	 * Skill
	 */
	wSize = decode_2bytes(c_pData);
	c_pData += sizeof(WORD);
	if (wSize) CSkillManager::instance().Initialize((TSkillTable*)c_pData, wSize);
	c_pData += sizeof(TSkillTable) * wSize;

	/*
	 * ITEM
	 */
	wSize = decode_2bytes(c_pData);
	c_pData += 2;
	sys_log(0, "RELOAD: ITEM: %d", wSize);

	if (wSize)
	{
		ITEM_MANAGER::instance().Initialize((TItemTable*)c_pData, wSize);
		c_pData += wSize * sizeof(TItemTable);
	}

	/*
	 * MONSTER
	 */
	wSize = decode_2bytes(c_pData);
	c_pData += 2;
	sys_log(0, "RELOAD: MOB: %d", wSize);

	if (wSize)
	{
		CMobManager::instance().Initialize((TMobTable*)c_pData, wSize);
		c_pData += wSize * sizeof(TMobTable);
	}

#ifdef __RELOAD_REWORK__

	wSize = decode_2bytes(c_pData);
	c_pData += sizeof(WORD);
	sys_log(0, "RELOAD: SHOP: %d", wSize);
	if (wSize)
	{
		CShopManager::instance().Initialize((TShopTable*)c_pData, wSize);
		c_pData += wSize * sizeof(TShopTable);
	}
	wSize = decode_2bytes(c_pData);
	c_pData += 2;
	sys_log(0, "RELOAD: REFINE: %d", wSize);
	if (wSize)
	{
		CRefineManager::instance().Initialize((TRefineTable*)c_pData, wSize);
		c_pData += wSize * sizeof(TRefineTable);
	}
	wSize = decode_2bytes(c_pData);
	c_pData += 2;
	sys_log(0, "RELOAD: ItemAtt: %d", wSize);
	if (wSize)
	{
		TItemAttrTable* p = (TItemAttrTable*)c_pData;
		g_map_itemAttr.clear();
		for (int i = 0; i < wSize; ++i, ++p)
		{
			if (p->dwApplyIndex >= MAX_APPLY_NUM)
				continue;
			g_map_itemAttr[p->dwApplyIndex] = *p;
			sys_log(0, "ITEM_ATTR[%d]: %s %u", p->dwApplyIndex, p->szApply, p->dwProb);
		}
		c_pData += wSize * sizeof(TItemAttrTable);
	}
	wSize = decode_2bytes(c_pData);
	c_pData += 2;
	sys_log(0, "RELOAD: ItemRareAtt: %d", wSize);
	if (wSize)
	{
		TItemAttrTable* p = (TItemAttrTable*)c_pData;
		g_map_itemRare.clear();
		for (int i = 0; i < wSize; ++i, ++p)
		{
			if (p->dwApplyIndex >= MAX_APPLY_NUM)
				continue;
			g_map_itemRare[p->dwApplyIndex] = *p;
			sys_log(0, "ITEM_RARE[%d]: %s %u", p->dwApplyIndex, p->szApply, p->dwProb);
		}
		c_pData += wSize * sizeof(TItemAttrTable);
	}
#endif

	CMotionManager::instance().Build();

	CHARACTER_MANAGER::instance().for_each_pc(std::mem_fun(&CHARACTER::ComputePoints));
}

#ifdef ENABLE_RELOAD_SHOP_COMMAND
void CInputDB::ReloadShopProto(const char* c_pData)
{
	WORD wSize;
	wSize = decode_2bytes(c_pData);
	c_pData += sizeof(WORD);
	sys_log(0, "RELOAD: SHOP: %d", wSize);

	if (wSize)
	{
		CShopManager::instance().Initialize((TShopTable*)c_pData, wSize);
		c_pData += wSize * sizeof(TShopTable);
	}
}
#endif

#ifdef ENABLE_RELOAD_REFINE_COMMAND
void CInputDB::ReloadRefineProto(const char* c_pData)
{
	WORD wSize;
	wSize = decode_2bytes(c_pData);
	c_pData += 2;
	sys_log(0, "RELOAD: REFINE: %d", wSize);

	if (wSize)
	{
		CRefineManager::instance().Initialize((TRefineTable*)c_pData, wSize);
		c_pData += wSize * sizeof(TRefineTable);
	}
}
#endif

void CInputDB::GuildSkillUsableChange(const char* c_pData)
{
	TPacketGuildSkillUsableChange* p = (TPacketGuildSkillUsableChange*)c_pData;

	CGuild* g = CGuildManager::instance().TouchGuild(p->dwGuild);

	g->SkillUsableChange(p->dwSkillVnum, p->bUsable ? true : false);
}

void CInputDB::AuthLogin(LPDESC d, const char* c_pData)
{
	if (!d)
		return;

	BYTE bResult = *(BYTE*)c_pData;

	TPacketGCAuthSuccess ptoc;

	ptoc.bHeader = HEADER_GC_AUTH_SUCCESS;

	if (bResult)
	{
		ptoc.dwLoginKey = d->GetLoginKey();
	}
	else
	{
		ptoc.dwLoginKey = 0;
	}

	ptoc.bResult = bResult;

	d->Packet(&ptoc, sizeof(TPacketGCAuthSuccess));
	sys_log(0, "AuthLogin result %u key %u", bResult, d->GetLoginKey());
}

void CInputDB::ChangeEmpirePriv(const char* c_pData)
{
	TPacketDGChangeEmpirePriv* p = (TPacketDGChangeEmpirePriv*)c_pData;

	// ADD_EMPIRE_PRIV_TIME
	CPrivManager::instance().GiveEmpirePriv(p->empire, p->type, p->value, p->bLog, p->end_time_sec);
	// END_OF_ADD_EMPIRE_PRIV_TIME
}

void CInputDB::SetEventFlag(const char* c_pData)
{
	TPacketSetEventFlag* p = (TPacketSetEventFlag*)c_pData;
	quest::CQuestManager::instance().SetEventFlag(p->szFlagName, p->lValue);
}

void CInputDB::Notice(const char* c_pData)
{
	char szBuf[256 + 1];
	strlcpy(szBuf, c_pData, sizeof(szBuf));

	sys_log(0, "InputDB:: Notice: %s", szBuf);

	//SendNotice(LC_TEXT(szBuf));
	SendNotice(szBuf);
}

//RELOAD_ADMIN
void CInputDB::ReloadAdmin(const char* c_pData)
{
	gm_new_clear();

	c_pData += 2;
	int size = decode_2bytes(c_pData);
	c_pData += 2;

	for (int n = 0; n < size; ++n)
	{
		tAdminInfo& rAdminInfo = *(tAdminInfo*)c_pData;

		gm_new_insert(rAdminInfo);

		c_pData += sizeof(tAdminInfo);

		LPCHARACTER pChar = CHARACTER_MANAGER::instance().FindPC(rAdminInfo.m_szName);
		if (pChar)
		{
			pChar->SetGMLevel();
		}
	}
}
#ifdef __OFFLINE_SHOP__
template <class T>
const char* Decode(T*& pObj, const char* data) {
	pObj = (T*)data;
	return data + sizeof(T);
}

void OfflineShopLoadTables(const char* data)
{
	offlineshop::TSubPacketDGLoadTables* pSubPack = nullptr;
	data = Decode(pSubPack, data);
	offlineshop::CShopManager& rManager = offlineshop::GetManager();

	OFFSHOP_DEBUG("shop count %u ", pSubPack->dwShopCount);

	for (DWORD i = 0; i < pSubPack->dwShopCount; i++)
	{
		offlineshop::TShopInfo* pShop = nullptr;
		offlineshop::TItemInfo* pItem = nullptr;

		data = Decode(pShop, data);

		OFFSHOP_DEBUG("shop %u %s", pShop->dwOwnerID, pShop->szName);

		offlineshop::CShop* pkShop = rManager.PutsNewShop(pShop);

		for (DWORD j = 0; j < pShop->dwCount; j++)
		{
			data = Decode(pItem, data);
			offlineshop::CShopItem kItem(pItem->dwItemID);

			kItem.SetOwnerID(pItem->dwOwnerID);
			kItem.SetInfo(pItem->item);
			kItem.SetPrice(pItem->price);
			kItem.SetWindow(NEW_OFFSHOP);

			OFFSHOP_DEBUG("for sale item %u ", pItem->dwItemID);
			pkShop->AddItem(kItem);
		}
	}
}

void OfflineShopBuyItemPacket(const char* data)
{
	offlineshop::TSubPacketDGBuyItem* subpack;
	data = Decode(subpack, data);

	offlineshop::CShopManager& rManager = offlineshop::GetManager();
	rManager.RecvShopBuyDBPacket(subpack->dwBuyerID, subpack->dwOwnerID, subpack->dwItemID, subpack->isSearch);
}

void OfflineShopLockedBuyItemPacket(const char* data)
{
	offlineshop::TSubPacketDGLockedBuyItem* subpack;
	data = Decode(subpack, data);

	offlineshop::CShopManager& rManager = offlineshop::GetManager();
	rManager.RecvShopLockedBuyItemDBPacket(subpack->dwBuyerID, subpack->dwOwnerID, subpack->dwItemID, subpack->isSearch);
}

void OfflineShopRemoveItemPacket(const char* data)
{
	offlineshop::TSubPacketDGRemoveItem* subpack;
	data = Decode(subpack, data);

	offlineshop::CShopManager& rManager = offlineshop::GetManager();
	rManager.RecvShopRemoveItemDBPacket(subpack->dwOwnerID, subpack->dwItemID);
}

void OfflineShopAddItemPacket(const char* data)
{
	offlineshop::TSubPacketDGAddItem* subpack;
	data = Decode(subpack, data);

	offlineshop::CShopManager& rManager = offlineshop::GetManager();
	rManager.RecvShopAddItemDBPacket(subpack->dwOwnerID, subpack->item);
}

void OfflineShopForceClosePacket(const char* data)
{
	offlineshop::TSubPacketDGShopForceClose* subpack;
	data = Decode(subpack, data);

	offlineshop::CShopManager& rManager = offlineshop::GetManager();
	rManager.RecvShopForceCloseDBPacket(subpack->dwOwnerID);
}

void OfflineShopShopCreateNewPacket(const char* data)
{
	offlineshop::TSubPacketDGShopCreateNew* subpack;
	data = Decode(subpack, data);

	OFFSHOP_DEBUG("shop %u , dur %u , count %u ", subpack->shop.dwOwnerID, subpack->shop.dwDuration, subpack->shop.dwCount);

	offlineshop::CShopManager& rManager = offlineshop::GetManager();

	std::vector<offlineshop::TItemInfo> vec;
	vec.reserve(subpack->shop.dwCount);

	offlineshop::TItemInfo* pItemInfo = nullptr;

	for (DWORD i = 0; i < subpack->shop.dwCount; i++)
	{
		data = Decode(pItemInfo, data);
		vec.push_back(*pItemInfo);

		OFFSHOP_DEBUG("item id %u , item vnum %u , item count %u ", pItemInfo->dwItemID, pItemInfo->item.dwVnum, pItemInfo->item.dwCount);
	}

	rManager.RecvShopCreateNewDBPacket(subpack->shop, vec);
}

void OfflineShopSafeboxGetItemPacket(const char* data)
{
	offlineshop::TSubPacketDGSafeboxGetItem* subpack;
	data = Decode(subpack, data);

	offlineshop::CShopManager& rManager = offlineshop::GetManager();
	rManager.RecvShopSafeboxGetItemDBPacket(subpack->dwOwnerID, subpack->dwItemID);
}

void OfflineShopSafeboxGetValutesPacket(const char* data)
{
	offlineshop::TSubPacketDGSafeboxGetValutes* subpack;
	data = Decode(subpack, data);

	offlineshop::CShopManager& rManager = offlineshop::GetManager();
	rManager.RecvShopSafeboxGetValutesDBPacket(subpack->dwOwnerID, subpack->valute);
}

void OfflineShopSafeboxAddItemPacket(const char* data)
{
	offlineshop::TSubPacketDGSafeboxAddItem* subpack;
	data = Decode(subpack, data);

	offlineshop::CShopManager& rManager = offlineshop::GetManager();
	rManager.RecvShopSafeboxAddItemDBPacket(subpack->dwOwnerID, subpack->dwItemID, subpack->item);
}

void OfflineShopSafeboxAddValutesPacket(const char* data)
{
	offlineshop::TSubPacketDGSafeboxAddValutes* subpack;
	data = Decode(subpack, data);

	offlineshop::CShopManager& rManager = offlineshop::GetManager();
	rManager.RecvShopSafeboxAddValutesDBPacket(subpack->dwOwnerID, subpack->valute);
}

void OfflineShopSafeboxLoad(const char* data)
{
	offlineshop::TSubPacketDGSafeboxLoad* subpack;
	data = Decode(subpack, data);

	std::vector<DWORD> ids;
	std::vector<offlineshop::TItemInfoEx> items;

	ids.reserve(subpack->dwItemCount);
	items.reserve(subpack->dwItemCount);

	DWORD* pdwItemID = nullptr;
	offlineshop::TItemInfoEx* temp;

	for (DWORD i = 0; i < subpack->dwItemCount; i++)
	{
		data = Decode(pdwItemID, data);
		data = Decode(temp, data);

		ids.push_back(*pdwItemID);
		items.push_back(*temp);
	}

	offlineshop::CShopManager& rManager = offlineshop::GetManager();
	rManager.RecvShopSafeboxLoadDBPacket(subpack->dwOwnerID, subpack->valute, ids, items);
}

//patch 08-03-2020
void OfflineshopSafeboxExpiredItem(const char* data)
{
	offlineshop::TSubPacketDGSafeboxExpiredItem* subpack;
	data = Decode(subpack, data);

	offlineshop::CShopManager& rManager = offlineshop::GetManager();
	rManager.RecvShopSafeboxExpiredItemDBPacket(subpack->dwOwnerID, subpack->dwItemID);
}

void OfflineshopShopExpired(const char* data)
{
	offlineshop::TSubPacketDGShopExpired* subpack;
	data = Decode(subpack, data);

	offlineshop::CShopManager& rManager = offlineshop::GetManager();
	rManager.RecvShopExpiredDBPacket(subpack->dwOwnerID);
}

void OfflineshopPacket(const char* data)
{
	TPacketDGNewOfflineShop* pPack = nullptr;
	data = Decode(pPack, data);

	OFFSHOP_DEBUG("recv subheader %d", pPack->bSubHeader);

	switch (pPack->bSubHeader)
	{
	case offlineshop::SUBHEADER_DG_LOAD_TABLES:
		OfflineShopLoadTables(data);
		return;

	case offlineshop::SUBHEADER_DG_BUY_ITEM:
		OfflineShopBuyItemPacket(data);
		return;

	case offlineshop::SUBHEADER_DG_LOCKED_BUY_ITEM:
		OfflineShopLockedBuyItemPacket(data);
		return;

	case offlineshop::SUBHEADER_DG_REMOVE_ITEM:
		OfflineShopRemoveItemPacket(data);
		return;

	case offlineshop::SUBHEADER_DG_ADD_ITEM:
		OfflineShopAddItemPacket(data);
		return;

	case offlineshop::SUBHEADER_DG_SHOP_FORCE_CLOSE:
		OfflineShopForceClosePacket(data);
		return;

	case offlineshop::SUBHEADER_DG_SHOP_CREATE_NEW:
		OfflineShopShopCreateNewPacket(data);
		return;

	case offlineshop::SUBHEADER_DG_SHOP_EXPIRED:
		OfflineshopShopExpired(data);
		break;

	case offlineshop::SUBHEADER_DG_SAFEBOX_GET_ITEM:
		OfflineShopSafeboxGetItemPacket(data);
		break;

	case offlineshop::SUBHEADER_DG_SAFEBOX_GET_VALUTES:
		OfflineShopSafeboxGetValutesPacket(data);
		break;

	case offlineshop::SUBHEADER_DG_SAFEBOX_ADD_ITEM:
		OfflineShopSafeboxAddItemPacket(data);
		return;

	case offlineshop::SUBHEADER_DG_SAFEBOX_ADD_VALUTES:
		OfflineShopSafeboxAddValutesPacket(data);
		return;

	case offlineshop::SUBHEADER_DG_SAFEBOX_LOAD:
		OfflineShopSafeboxLoad(data);
		return;

		//patch 08-03-2020
	case offlineshop::SUBHEADER_DG_SAFEBOX_EXPIRED_ITEM:
		OfflineshopSafeboxExpiredItem(data);
		return;

	default:
		sys_err("UKNOWN SUB HEADER %d ", pPack->bSubHeader);
		return;
	}
}
#endif
//END_RELOAD_ADMIN

#ifdef __BATTLE_PASS_SYSTEM__
void CInputDB::BattlePassLoad(LPDESC d, const char* c_pData)
{
	if (!d || !d->GetCharacter())
		return;

	LPCHARACTER ch = d->GetCharacter();
	if (!ch)
		return;

	DWORD dwPID = decode_4bytes(c_pData);
	c_pData += sizeof(DWORD);

	DWORD dwCount = decode_4bytes(c_pData);
	c_pData += sizeof(DWORD);

	if (ch->GetPlayerID() != dwPID)
		return;

	ch->LoadBattlePass(dwCount, (TPlayerBattlePassMission*)c_pData);
}

void CInputDB::BattlePassLoadRanking(LPDESC d, const char* c_pData)
{
	if (!d || !d->GetCharacter())
		return;

	LPCHARACTER ch = d->GetCharacter();
	if (!ch)
		return;

	DWORD dwPID = decode_4bytes(c_pData);
	c_pData += sizeof(DWORD);

	BYTE bIsGlobal = decode_byte(c_pData);
	c_pData += sizeof(BYTE);

	DWORD dwCount = decode_4bytes(c_pData);
	c_pData += sizeof(DWORD);

	sys_err("BattlePassLoadRanking count %d playerid %d", dwCount, dwPID);

	if (ch->GetPlayerID() != dwPID)
		return;

	if (dwCount)
	{
		std::vector<TBattlePassRanking> sendVector;
		sendVector.resize(dwCount);

		TBattlePassRanking* p = (TBattlePassRanking*)c_pData;

		for (int i = 0; i < dwCount; ++i, ++p)
		{
			TBattlePassRanking newRanking;
			newRanking.bPos = p->bPos;
			strlcpy(newRanking.playerName, p->playerName, sizeof(newRanking.playerName));
			newRanking.dwFinishTime = p->dwFinishTime;

			sendVector.push_back(newRanking);
		}

		if (!sendVector.empty())
		{
			TPacketGCBattlePassRanking packet;
			packet.bHeader = HEADER_GC_BATTLE_PASS_RANKING;
			packet.wSize = sizeof(packet) + sizeof(TBattlePassRanking) * sendVector.size();
			packet.bIsGlobal = bIsGlobal;

			ch->GetDesc()->BufferedPacket(&packet, sizeof(packet));
			ch->GetDesc()->Packet(&sendVector[0], sizeof(TBattlePassRanking) * sendVector.size());
		}
	}
	else
	{
		ch->ChatPacket(CHAT_TYPE_COMMAND, "BINARY_BattlePassRefreshRanking");
	}
}
#endif

////////////////////////////////////////////////////////////////////
// Analyze
////////////////////////////////////////////////////////////////////
int CInputDB::Analyze(LPDESC d, BYTE bHeader, const char* c_pData)
{
	switch (bHeader)
	{
	case HEADER_DG_BOOT:
		Boot(c_pData);
		break;

	case HEADER_DG_LOGIN_SUCCESS:
		LoginSuccess(m_dwHandle, c_pData);
		break;

	case HEADER_DG_LOGIN_NOT_EXIST:
		LoginFailure(DESC_MANAGER::instance().FindByHandle(m_dwHandle), "NOID");
		break;

	case HEADER_DG_LOGIN_WRONG_PASSWD:
		LoginFailure(DESC_MANAGER::instance().FindByHandle(m_dwHandle), "NOID"); // DEFAULT WRONGPWD
		break;

	case HEADER_DG_LOGIN_ALREADY:
		LoginAlready(DESC_MANAGER::instance().FindByHandle(m_dwHandle), c_pData);
		break;

	case HEADER_DG_PLAYER_LOAD_SUCCESS:
		PlayerLoad(DESC_MANAGER::instance().FindByHandle(m_dwHandle), c_pData);
		break;

	case HEADER_DG_PLAYER_CREATE_SUCCESS:
		PlayerCreateSuccess(DESC_MANAGER::instance().FindByHandle(m_dwHandle), c_pData);
		break;

	case HEADER_DG_PLAYER_CREATE_FAILED:
		PlayerCreateFailure(DESC_MANAGER::instance().FindByHandle(m_dwHandle), 0);
		break;

	case HEADER_DG_PLAYER_CREATE_ALREADY:
		PlayerCreateFailure(DESC_MANAGER::instance().FindByHandle(m_dwHandle), 1);
		break;

	case HEADER_DG_PLAYER_DELETE_SUCCESS:
		PlayerDeleteSuccess(DESC_MANAGER::instance().FindByHandle(m_dwHandle), c_pData);
		break;

	case HEADER_DG_PLAYER_LOAD_FAILED:
		//sys_log(0, "PLAYER_LOAD_FAILED");
		break;

	case HEADER_DG_PLAYER_DELETE_FAILED:
		//sys_log(0, "PLAYER_DELETE_FAILED");
		PlayerDeleteFail(DESC_MANAGER::instance().FindByHandle(m_dwHandle));
		break;

	case HEADER_DG_ITEM_LOAD:
		ItemLoad(DESC_MANAGER::instance().FindByHandle(m_dwHandle), c_pData);
		break;

	case HEADER_DG_QUEST_LOAD:
		QuestLoad(DESC_MANAGER::instance().FindByHandle(m_dwHandle), c_pData);
		break;

	case HEADER_DG_AFFECT_LOAD:
		AffectLoad(DESC_MANAGER::instance().FindByHandle(m_dwHandle), c_pData);
		break;

	case HEADER_DG_SAFEBOX_LOAD:
		SafeboxLoad(DESC_MANAGER::instance().FindByHandle(m_dwHandle), c_pData);
		break;

	case HEADER_DG_SAFEBOX_CHANGE_SIZE:
		SafeboxChangeSize(DESC_MANAGER::instance().FindByHandle(m_dwHandle), c_pData);
		break;

	case HEADER_DG_SAFEBOX_WRONG_PASSWORD:
		SafeboxWrongPassword(DESC_MANAGER::instance().FindByHandle(m_dwHandle));
		break;

	case HEADER_DG_SAFEBOX_CHANGE_PASSWORD_ANSWER:
		SafeboxChangePasswordAnswer(DESC_MANAGER::instance().FindByHandle(m_dwHandle), c_pData);
		break;

	case HEADER_DG_MALL_LOAD:
		MallLoad(DESC_MANAGER::instance().FindByHandle(m_dwHandle), c_pData);
		break;

	case HEADER_DG_EMPIRE_SELECT:
		EmpireSelect(DESC_MANAGER::instance().FindByHandle(m_dwHandle), c_pData);
		break;

	case HEADER_DG_MAP_LOCATIONS:
		MapLocations(c_pData);
		break;

	case HEADER_DG_P2P:
		P2P(c_pData);
		break;

	case HEADER_DG_GUILD_SKILL_UPDATE:
		GuildSkillUpdate(c_pData);
		break;

	case HEADER_DG_GUILD_LOAD:
		GuildLoad(c_pData);
		break;

	case HEADER_DG_GUILD_SKILL_RECHARGE:
		GuildSkillRecharge();
		break;

	case HEADER_DG_GUILD_EXP_UPDATE:
		GuildExpUpdate(c_pData);
		break;

	case HEADER_DG_PARTY_CREATE:
		PartyCreate(c_pData);
		break;

	case HEADER_DG_PARTY_DELETE:
		PartyDelete(c_pData);
		break;

	case HEADER_DG_PARTY_ADD:
		PartyAdd(c_pData);
		break;

	case HEADER_DG_PARTY_REMOVE:
		PartyRemove(c_pData);
		break;

	case HEADER_DG_PARTY_STATE_CHANGE:
		PartyStateChange(c_pData);
		break;

	case HEADER_DG_PARTY_SET_MEMBER_LEVEL:
		PartySetMemberLevel(c_pData);
		break;

	case HEADER_DG_TIME:
		Time(c_pData);
		break;

	case HEADER_DG_GUILD_ADD_MEMBER:
		GuildAddMember(c_pData);
		break;

	case HEADER_DG_GUILD_REMOVE_MEMBER:
		GuildRemoveMember(c_pData);
		break;

	case HEADER_DG_GUILD_CHANGE_GRADE:
		GuildChangeGrade(c_pData);
		break;

	case HEADER_DG_GUILD_CHANGE_MEMBER_DATA:
		GuildChangeMemberData(c_pData);
		break;

	case HEADER_DG_GUILD_DISBAND:
		GuildDisband(c_pData);
		break;

	case HEADER_DG_RELOAD_PROTO:
		ReloadProto(c_pData);
		break;

#ifdef ENABLE_RELOAD_SHOP_COMMAND
	case HEADER_DG_RELOAD_SHOP:
		ReloadShopProto(c_pData);
		break;
#endif

#ifdef ENABLE_RELOAD_REFINE_COMMAND
	case HEADER_DG_RELOAD_REFINE:
		ReloadRefineProto(c_pData);
		break;
#endif

	case HEADER_DG_GUILD_WAR:
		GuildWar(c_pData);
		break;

	case HEADER_DG_GUILD_WAR_SCORE:
		GuildWarScore(c_pData);
		break;

	case HEADER_DG_GUILD_LADDER:
		GuildLadder(c_pData);
		break;

	case HEADER_DG_GUILD_SKILL_USABLE_CHANGE:
		GuildSkillUsableChange(c_pData);
		break;

	case HEADER_DG_CHANGE_NAME:
		ChangeName(DESC_MANAGER::instance().FindByHandle(m_dwHandle), c_pData);
		break;

	case HEADER_DG_AUTH_LOGIN:
		AuthLogin(DESC_MANAGER::instance().FindByHandle(m_dwHandle), c_pData);
		break;

	case HEADER_DG_CHANGE_EMPIRE_PRIV:
		ChangeEmpirePriv(c_pData);
		break;

	case HEADER_DG_SET_EVENT_FLAG:
		SetEventFlag(c_pData);
		break;

	case HEADER_DG_NOTICE:
		Notice(c_pData);
		break;

	// RELOAD_ADMIN
	case HEADER_DG_RELOAD_ADMIN:
		ReloadAdmin(c_pData);
		break;
		//END_RELOAD_ADMIN

	case HEADER_DG_ACK_CHANGE_GUILD_MASTER:
		this->GuildChangeMaster((TPacketChangeGuildMaster*)c_pData);
		break;
	case HEADER_DG_ACK_SPARE_ITEM_ID_RANGE:
		ITEM_MANAGER::instance().SetMaxSpareItemID(*((TItemIDRangeTable*)c_pData));
		break;

	case HEADER_DG_UPDATE_HORSE_NAME:
	case HEADER_DG_ACK_HORSE_NAME:
		CHorseNameManager::instance().UpdateHorseName(
			((TPacketUpdateHorseName*)c_pData)->dwPlayerID,
			((TPacketUpdateHorseName*)c_pData)->szHorseName);
		break;

	case HEADER_DG_NEED_LOGIN_LOG:
		DetailLog((TPacketNeedLoginLogInfo*)c_pData);
		break;

	case HEADER_DG_RESPOND_CHANNELSTATUS:
		RespondChannelStatus(DESC_MANAGER::instance().FindByHandle(m_dwHandle), c_pData);
		break;
#ifdef __OFFLINE_SHOP__
	case HEADER_DG_NEW_OFFLINESHOP:
		OfflineshopPacket(c_pData);
		break;
#endif
#ifdef __CHANNEL_CHANGER__
	case HEADER_DG_CHANNEL_RESULT:
		ChangeChannel(DESC_MANAGER::instance().FindByHandle(m_dwHandle), c_pData);
		break;
#endif

#ifdef __SKILL_COLOR__
	case HEADER_DG_SKILL_COLOR_LOAD:
		SkillColorLoad(DESC_MANAGER::instance().FindByHandle(m_dwHandle), c_pData);
		break;
#endif

#ifdef __BATTLE_PASS_SYSTEM__
	case HEADER_DG_BATTLE_PASS_LOAD:
		BattlePassLoad(DESC_MANAGER::instance().FindByHandle(m_dwHandle), c_pData);
		break;

	case HEADER_DG_BATTLE_PASS_LOAD_RANKING:
		BattlePassLoadRanking(DESC_MANAGER::instance().FindByHandle(m_dwHandle), c_pData);
		break;
#endif

	default:
		return (-1);
	}

	return 0;
}

bool CInputDB::Process(LPDESC d, const void* orig, int bytes, int& r_iBytesProceed)
{
	const char* c_pData = (const char*)orig;
	BYTE		bHeader, bLastHeader = 0;
	int			iSize;
	int			iLastPacketLen = 0;

	for (m_iBufferLeft = bytes; m_iBufferLeft > 0;)
	{
		if (m_iBufferLeft < 9)
			return true;

		bHeader = *((BYTE*)(c_pData));	// 1
		m_dwHandle = *((DWORD*)(c_pData + 1));	// 4
		iSize = *((DWORD*)(c_pData + 5));	// 4

		sys_log(1, "DBCLIENT: header %d handle %d size %d bytes %d", bHeader, m_dwHandle, iSize, bytes);

		if (m_iBufferLeft - 9 < iSize)
			return true;

		const char* pRealData = (c_pData + 9);

		if (Analyze(d, bHeader, pRealData) < 0)
		{
			sys_err("in InputDB: UNKNOWN HEADER: %d, LAST HEADER: %d(%d), REMAIN BYTES: %d, DESC: %d",
				bHeader, bLastHeader, iLastPacketLen, m_iBufferLeft, d->GetSocket());

			//printdata((BYTE*) orig, bytes);
			//d->SetPhase(PHASE_CLOSE);
		}

		c_pData += 9 + iSize;
		m_iBufferLeft -= 9 + iSize;
		r_iBytesProceed += 9 + iSize;

		iLastPacketLen = 9 + iSize;
		bLastHeader = bHeader;
	}

	return true;
}

void CInputDB::GuildChangeMaster(TPacketChangeGuildMaster* p)
{
	CGuildManager::instance().ChangeMaster(p->dwGuildID);
}

void CInputDB::DetailLog(const TPacketNeedLoginLogInfo* info)
{
	LPCHARACTER pChar = CHARACTER_MANAGER::instance().FindByPID( info->dwPlayerID );

	if (NULL != pChar)
	{
		LogManager::instance().DetailLoginLog(true, pChar);
	}
}

void CInputDB::RespondChannelStatus(LPDESC desc, const char* pcData)
{
	if (!desc) {
		return;
	}
	const int nSize = decode_4bytes(pcData);
	pcData += sizeof(nSize);

	BYTE bHeader = HEADER_GC_RESPOND_CHANNELSTATUS;
	desc->BufferedPacket(&bHeader, sizeof(BYTE));
	desc->BufferedPacket(&nSize, sizeof(nSize));
	if (0 < nSize) {
		desc->BufferedPacket(pcData, sizeof(TChannelStatus) * nSize);
	}
	BYTE bSuccess = 1;
	desc->Packet(&bSuccess, sizeof(bSuccess));
	desc->SetChannelStatusRequested(false);
}
