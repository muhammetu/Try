#include "stdafx.h"
#include "config.h"
#include "desc_client.h"
#include "desc_manager.h"
#include "char.h"
#include "char_manager.h"
#include "p2p.h"
#include "guild.h"
#include "guild_manager.h"
#include "party.h"
#include "messenger_manager.h"
#include "unique_item.h"
#include "affect.h"
#include "locale_service.h"
#include "questmanager.h"
#include "skill.h"
#ifdef __RELOAD_REWORK__
ACMD(do_reload);
#endif
#ifdef __EVENT_SYSTEM__
#include "game_events.h"
#endif
////////////////////////////////////////////////////////////////////////////////
// Input Processor

CInputP2P::CInputP2P()
{
	BindPacketInfo(&m_packetInfoGG);
}

void CInputP2P::Login(LPDESC d, const char* c_pData)
{
	P2P_MANAGER::instance().Login(d, (TPacketGGLogin*)c_pData);
}

void CInputP2P::Logout(LPDESC d, const char* c_pData)
{
	TPacketGGLogout* p = (TPacketGGLogout*)c_pData;
	P2P_MANAGER::instance().Logout(p->szName);
}

int CInputP2P::Relay(LPDESC d, const char* c_pData, size_t uiBytes)
{
	TPacketGGRelay* p = (TPacketGGRelay*)c_pData;

	if (uiBytes < sizeof(TPacketGGRelay) + p->lSize)
		return -1;

	if (p->lSize < 0)
	{
		sys_err("invalid packet length %d", p->lSize);
		d->SetPhase(PHASE_CLOSE);
		return -1;
	}

	sys_log(0, "InputP2P::Relay : %s size %d", p->szName, p->lSize);

	LPCHARACTER pkChr = CHARACTER_MANAGER::instance().FindPC(p->szName);

	const BYTE* c_pbData = (const BYTE*)(c_pData + sizeof(TPacketGGRelay));

	if (!pkChr)
		return p->lSize;

	if (*c_pbData == HEADER_GC_WHISPER)
	{
		if (pkChr->IsBlockMode(BLOCK_WHISPER))
		{
			return p->lSize;
		}

		char buf[1024];
		memcpy(buf, c_pbData, MIN(p->lSize, sizeof(buf)));

		TPacketGCWhisper* p2 = (TPacketGCWhisper*)buf;
		p2->bType = p2->bType & 0x0F;
		if (p2->bType == 0x0F) {
			p2->bType = WHISPER_TYPE_SYSTEM;
		}
		pkChr->GetDesc()->Packet(buf, p->lSize);
	}
	else
		pkChr->GetDesc()->Packet(c_pbData, p->lSize);

	return (p->lSize);
}

#ifdef __GLOBAL_MESSAGE_UTILITY__
int CInputP2P::BulkWhisperSend(LPDESC d, const char* c_pData, size_t uiBytes)
{
	TPacketGGBulkWhisper* p = (TPacketGGBulkWhisper*)c_pData;

	if (uiBytes < sizeof(TPacketGGBulkWhisper) + p->lSize)
		return -1;

	if (p->lSize < 0)
	{
		sys_err("invalid packet length %d", p->lSize);
		d->SetPhase(PHASE_CLOSE);
		return -1;
	}

	char szBuf[CHAT_MAX_LEN + 1];
	strlcpy(szBuf, c_pData + sizeof(TPacketGGBulkWhisper), MIN(p->lSize + 1, sizeof(szBuf)));
	SendBulkWhisper(szBuf);

	return (p->lSize);
}
#endif

#ifdef __FULL_NOTICE__
int CInputP2P::Notice(LPDESC d, const char* c_pData, size_t uiBytes, bool bBigFont)
#else
int CInputP2P::Notice(LPDESC d, const char* c_pData, size_t uiBytes)
#endif
{
	TPacketGGNotice* p = (TPacketGGNotice*)c_pData;

	if (uiBytes < sizeof(TPacketGGNotice) + p->lSize)
		return -1;

	if (p->lSize < 0)
	{
		sys_err("invalid packet length %d", p->lSize);
		d->SetPhase(PHASE_CLOSE);
		return -1;
	}

	char szBuf[256 + 1];
	strlcpy(szBuf, c_pData + sizeof(TPacketGGNotice), MIN(p->lSize + 1, sizeof(szBuf)));
#ifdef __FULL_NOTICE__
	SendNotice(szBuf, bBigFont);
#else
	SendNotice(szBuf);
#endif
	return (p->lSize);
}

#ifdef __CHAT_FILTER__
int CInputP2P::NoticeImproving(LPDESC d, const char* c_pData, size_t uiBytes)
{
	TPacketGGNotice* p = (TPacketGGNotice*)c_pData;

	if (uiBytes < sizeof(TPacketGGNotice) + p->lSize)
		return -1;

	if (p->lSize < 0)
	{
		sys_err("invalid packet length %d", p->lSize);
		d->SetPhase(PHASE_CLOSE);
		return -1;
	}

	char szBuf[256 + 1];
	strlcpy(szBuf, c_pData + sizeof(TPacketGGNotice), MIN(p->lSize + 1, sizeof(szBuf)));
	SendNotice(szBuf);
	return (p->lSize);
}
#endif

int CInputP2P::Guild(LPDESC d, const char* c_pData, size_t uiBytes)
{
	TPacketGGGuild* p = (TPacketGGGuild*)c_pData;
	uiBytes -= sizeof(TPacketGGGuild);
	c_pData += sizeof(TPacketGGGuild);

	CGuild* g = CGuildManager::instance().FindGuild(p->dwGuild);

	switch (p->bSubHeader)
	{
	case GUILD_SUBHEADER_GG_CHAT:
	{
		if (uiBytes < sizeof(TPacketGGGuildChat))
			return -1;

		TPacketGGGuildChat* p = (TPacketGGGuildChat*)c_pData;

		if (g)
			g->P2PChat(p->szText);

		return sizeof(TPacketGGGuildChat);
	}

	case GUILD_SUBHEADER_GG_SET_MEMBER_COUNT_BONUS:
	{
		if (uiBytes < sizeof(int))
			return -1;

		int iBonus = *((int*)c_pData);
		CGuild* pGuild = CGuildManager::instance().FindGuild(p->dwGuild);
		if (pGuild)
		{
			pGuild->SetMemberCountBonus(iBonus);
		}
		return sizeof(int);
	}
	default:
		sys_err("UNKNOWN GUILD SUB PACKET");
		break;
	}
	return 0;
}

struct FuncShout
{
	const char* m_str;
	BYTE m_bEmpire;

	FuncShout(const char* str, BYTE bEmpire) : m_str(str), m_bEmpire(bEmpire)
	{
	}

	void operator () (LPDESC d)
	{
		if (!d->GetCharacter() || (!g_bGlobalShoutEnable && d->GetCharacter()->GetGMLevel() == GM_PLAYER && d->GetEmpire() != m_bEmpire))
			return;
		d->GetCharacter()->ChatPacket(CHAT_TYPE_SHOUT, "%s", m_str);
	}
};

void SendShout(const char* szText, BYTE bEmpire)
{
	const DESC_MANAGER::DESC_SET& c_ref_set = DESC_MANAGER::instance().GetClientSet();
	std::for_each(c_ref_set.begin(), c_ref_set.end(), FuncShout(szText, bEmpire));
}

void CInputP2P::Shout(const char* c_pData)
{
	TPacketGGShout* p = (TPacketGGShout*)c_pData;
	SendShout(p->szText, p->bEmpire);
}

void CInputP2P::Disconnect(const char* c_pData)
{
	TPacketGGDisconnect* p = (TPacketGGDisconnect*)c_pData;

#ifdef __BAN_REASON_UTILITY__
	if (p->bBanned == true)
	{
		LPCHARACTER ch = CHARACTER_MANAGER::instance().FindPC(p->szLogin);
		if (ch && ch->GetDesc())
		{
			DESC_MANAGER::instance().DestroyDesc(ch->GetDesc(), true);
			return;
		}
		else
		{
			LPDESC d = DESC_MANAGER::instance().FindByLoginName(p->szLogin);
			if (d)
			{
				DESC_MANAGER::instance().DestroyDesc(d, true);
				return;
			}
		}

		return;
	}
#endif

	LPDESC d = DESC_MANAGER::instance().FindByLoginName(p->szLogin);

	if (!d)
		return;

	if (!d->GetCharacter())
	{
		d->SetPhase(PHASE_CLOSE);
	}
	else
		d->DisconnectOfSameLogin();
}

void CInputP2P::Setup(LPDESC d, const char* c_pData)
{
	TPacketGGSetup* p = (TPacketGGSetup*)c_pData;
	sys_log(0, "P2P: Setup %s:%d", d->GetHostName(), p->wPort);
	d->SetP2P(d->GetHostName(), p->wPort, p->bChannel);
}

void CInputP2P::MessengerAdd(const char* c_pData)
{
	TPacketGGMessenger* p = (TPacketGGMessenger*)c_pData;
	sys_log(0, "P2P: Messenger Add %s %s", p->szAccount, p->szCompanion);
	MessengerManager::instance().__AddToList(p->szAccount, p->szCompanion);
}

void CInputP2P::MessengerRemove(const char* c_pData)
{
	TPacketGGMessenger* p = (TPacketGGMessenger*)c_pData;
	sys_log(0, "P2P: Messenger Remove %s %s", p->szAccount, p->szCompanion);
	MessengerManager::instance().__RemoveFromList(p->szAccount, p->szCompanion);
}

void CInputP2P::FindPosition(LPDESC d, const char* c_pData)
{
	TPacketGGFindPosition* p = (TPacketGGFindPosition*)c_pData;
	LPCHARACTER ch = CHARACTER_MANAGER::instance().FindByPID(p->dwTargetPID);
	if (ch)
	{
		TPacketGGWarpCharacter pw;
		pw.header = HEADER_GG_WARP_CHARACTER;
		pw.pid = p->dwFromPID;
		pw.x = ch->GetX();
		pw.y = ch->GetY();
		pw.privateMapIndex = ch->GetMapIndex();
#ifdef __WARP_WITH_CHANNEL__
		pw.port = mother_port;
#endif
		d->Packet(&pw, sizeof(pw));
	}
}

#ifdef __WARP_WITH_CHANNEL__
void CInputP2P::WarpCharacter(LPDESC d, const char* c_pData)
#else
void CInputP2P::WarpCharacter(const char* c_pData)
#endif
{
	TPacketGGWarpCharacter* p = (TPacketGGWarpCharacter*)c_pData;
	LPCHARACTER ch = CHARACTER_MANAGER::instance().FindByPID(p->pid);
	if (ch)
	{
#ifdef __WARP_WITH_CHANNEL__
		ch->WarpSet(p->x, p->y, p->privateMapIndex, inet_addr(d->GetP2PHost()), p->port);
#else
		ch->WarpSet(p->x, p->y, p->privateMapIndex);
#endif
	}
}

void CInputP2P::GuildWarZoneMapIndex(const char* c_pData)
{
	TPacketGGGuildWarMapIndex* p = (TPacketGGGuildWarMapIndex*)c_pData;
	CGuildManager& gm = CGuildManager::instance();

	sys_log(0, "P2P: GuildWarZoneMapIndex g1(%u) vs g2(%u), mapIndex(%d)", p->dwGuildID1, p->dwGuildID2, p->lMapIndex);

	CGuild* g1 = gm.FindGuild(p->dwGuildID1);
	CGuild* g2 = gm.FindGuild(p->dwGuildID2);

	if (g1 && g2)
	{
		g1->SetGuildWarMapIndex(p->dwGuildID2, p->lMapIndex);
		g2->SetGuildWarMapIndex(p->dwGuildID1, p->lMapIndex);
	}
}

#ifdef __WARP_WITH_CHANNEL__
void CInputP2P::Transfer(LPDESC d, const char *c_pData)
#else
void CInputP2P::Transfer(const char *c_pData)
#endif
{
	TPacketGGTransfer* p = (TPacketGGTransfer*)c_pData;

	LPCHARACTER ch = CHARACTER_MANAGER::instance().FindPC(p->szName);

	if (ch)
#ifdef __WARP_WITH_CHANNEL__
		ch->WarpSet(p->lX, p->lY, p->privateMapIndex, inet_addr(d->GetP2PHost()), p->port);
#else
		ch->WarpSet(p->lX, p->lY);
#endif
}

void CInputP2P::LoginPing(LPDESC d, const char * c_pData)
{
	TPacketGGLoginPing * p = (TPacketGGLoginPing *) c_pData;

	if (!g_pkAuthMasterDesc) // If I am master, I have to broadcast
		P2P_MANAGER::instance().Send(p, sizeof(TPacketGGLoginPing), d);
}

// BLOCK_CHAT
void CInputP2P::BlockChat(const char* c_pData)
{
	TPacketGGBlockChat* p = (TPacketGGBlockChat*)c_pData;

	LPCHARACTER ch = CHARACTER_MANAGER::instance().FindPC(p->szName);

	if (ch)
	{
		sys_log(0, "BLOCK CHAT apply name %s dur %d", p->szName, p->lBlockDuration);
		ch->AddAffect(AFFECT_BLOCK_CHAT, POINT_NONE, 0, AFF_NONE, p->lBlockDuration, 0, true);
	}
	else
	{
		sys_log(0, "BLOCK CHAT fail name %s dur %d", p->szName, p->lBlockDuration);
	}
}
// END_OF_BLOCK_CHAT

#ifdef __EVENT_SYSTEM__
void CInputP2P::EventTime(const char* c_pData)
{
	TPacketGGEventInfo* p = (TPacketGGEventInfo*)c_pData;
	CGameEventsManager::instance().SetEventTime(p->event_id, p->event_time, false);
}
#endif

#ifdef __SWITCHBOT__
#include "new_switchbot.h"
void CInputP2P::Switchbot(LPDESC d, const char* c_pData)
{
	const TPacketGGSwitchbot* p = reinterpret_cast<const TPacketGGSwitchbot*>(c_pData);
	if (p->wPort != mother_port)
	{
		return;
	}

	CSwitchbotManager::Instance().P2PReceiveSwitchbot(p->table);
}
#endif

#ifdef ENABLE_MULTI_FARM_BLOCK
void CInputP2P::MultiFarm(const char* c_pData)
{
	TPacketGGMultiFarm* p = (TPacketGGMultiFarm*)c_pData;
	if (p->subHeader == MULTI_FARM_SET)
		CHARACTER_MANAGER::Instance().CheckMultiFarmAccount(p->playerIP, p->playerID, p->playerName, p->farmStatus, p->affectType, p->affectTime, true);
	else if (p->subHeader == MULTI_FARM_REMOVE)
		CHARACTER_MANAGER::Instance().RemoveMultiFarm(p->playerIP, p->playerID, true);
}
void CInputP2P::MultiFarmQF(const char* c_pData)
{
	TPacketGGMultiFarmQuestFlag* p = (TPacketGGMultiFarmQuestFlag*)c_pData;
	CHARACTER_MANAGER::Instance().SetDungeonTime(p->playerIP, p->szFlag, p->value, false);
}
#endif

int CInputP2P::Analyze(LPDESC d, BYTE bHeader, const char* c_pData)
{
	if (test_server)
		sys_log(0, "CInputP2P::Anlayze[Header %d]", bHeader);

	int iExtraLen = 0;

	switch (bHeader)
	{
	case HEADER_GG_SETUP:
		Setup(d, c_pData);
		break;

	case HEADER_GG_LOGIN:
		Login(d, c_pData);
		break;

	case HEADER_GG_LOGOUT:
		Logout(d, c_pData);
		break;

	case HEADER_GG_RELAY:
		if ((iExtraLen = Relay(d, c_pData, m_iBufferLeft)) < 0)
			return -1;
		break;
#ifdef __FULL_NOTICE__
	case HEADER_GG_BIG_NOTICE:
		if ((iExtraLen = Notice(d, c_pData, m_iBufferLeft, true)) < 0)
			return -1;
		break;
#endif
	case HEADER_GG_NOTICE:
		if ((iExtraLen = Notice(d, c_pData, m_iBufferLeft)) < 0)
			return -1;
		break;

#ifdef __CHAT_FILTER__
	case HEADER_GG_NOTICE_IMPROVING:
		if ((iExtraLen = NoticeImproving(d, c_pData, m_iBufferLeft)) < 0)
			return -1;
		break;
#endif

	case HEADER_GG_SHUTDOWN:
		sys_err("Accept shutdown p2p command from %s.", d->GetHostName());
		Shutdown(10);
		break;

	case HEADER_GG_GUILD:
		if ((iExtraLen = Guild(d, c_pData, m_iBufferLeft)) < 0)
			return -1;
		break;

	case HEADER_GG_SHOUT:
		Shout(c_pData);
		break;

	case HEADER_GG_DISCONNECT:
		Disconnect(c_pData);
		break;

	case HEADER_GG_MESSENGER_ADD:
		MessengerAdd(c_pData);
		break;

	case HEADER_GG_MESSENGER_REMOVE:
		MessengerRemove(c_pData);
		break;

	case HEADER_GG_FIND_POSITION:
		FindPosition(d, c_pData);
		break;

	case HEADER_GG_WARP_CHARACTER:
#ifdef __WARP_WITH_CHANNEL__
		WarpCharacter(d, c_pData);
#else
		WarpCharacter(c_pData);
#endif
		break;

	case HEADER_GG_GUILD_WAR_ZONE_MAP_INDEX:
		GuildWarZoneMapIndex(c_pData);
		break;

	case HEADER_GG_TRANSFER:
#ifdef __WARP_WITH_CHANNEL__
		Transfer(d, c_pData);
#else
		Transfer(c_pData);
#endif // __WARP_WITH_CHANNEL__
		break;

	case HEADER_GG_BLOCK_CHAT:
		BlockChat(c_pData);
		break;

#ifdef __RELOAD_REWORK__
	case HEADER_GG_RELOAD_COMMAND:
		do_reload(NULL, ((TPacketGGReloadCommand*)c_pData)->argument, 0, 0);
		break;
#endif
#ifdef __GLOBAL_MESSAGE_UTILITY__
	case HEADER_GG_BULK_WHISPER:
		if ((iExtraLen = BulkWhisperSend(d, c_pData, m_iBufferLeft)) < 0)
			return -1;
		break;
#endif
#ifdef __EVENT_SYSTEM__
	case HEADER_GG_EVENT_TIME:
		EventTime(c_pData);
		break;
#endif
#ifdef __SWITCHBOT__
	case HEADER_GG_SWITCHBOT:
		Switchbot(d, c_pData);
		break;
#endif
	case HEADER_GG_LOGIN_PING:
		LoginPing(d, c_pData);
		break;

#ifdef ENABLE_MULTI_FARM_BLOCK
	case HEADER_GG_MULTI_FARM:
		MultiFarm(c_pData);
		break;
	case HEADER_GG_MULTI_FARM_QF:
		MultiFarmQF(c_pData);
		break;
#endif
	}

	return (iExtraLen);
}