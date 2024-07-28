#include "stdafx.h"
#include "game_events.h"
#include "char.h"
#include "packet.h"
#include "desc.h"
#include "quest.h"
#include "questmanager.h"
#include "utils.h"
#include "p2p.h"

CGameEventsManager::CGameEventsManager()
{
	for (int i = 0; i < EVENT_MAX_NUM; ++i)
	{
		m_pkActivateEvents[i] = false;
		m_dwEventEndTime[i] = 0;
		m_bLimitedEvent[i] = false;
	}

	m_bNeedUpdate = false;
}

CGameEventsManager::~CGameEventsManager()
{
}

void CGameEventsManager::SendEventCharacter(LPCHARACTER ch)
{
	if (!ch)
		return;

	for (BYTE i = 0; i < EVENT_MAX_NUM; ++i)
	{
		TPacketGCEventInfo pPacket;
		pPacket.bHeader = HEADER_GC_EVENT_INFO;
		pPacket.event_id = i;
		pPacket.isActivate = IsActivateEvent(i);
		pPacket.event_time = GetEventTime(i);
		ch->GetDesc()->Packet(&pPacket, sizeof(TPacketGCEventInfo));
	}
}

void CGameEventsManager::SetActivateEvent(BYTE bEventID, bool isActivate)
{
	// Event info.
	if (isActivate)
		sys_log(0, "<--! Event Activated (%d) !-->", bEventID);
	else
		sys_log(0, "<--! Event Deactivated (%d) !-->", bEventID);

	if (IsActivateEvent(bEventID) != isActivate)
		m_bNeedUpdate = true;

	m_pkActivateEvents[bEventID] = isActivate;
}

void CGameEventsManager::SetEventTime(BYTE bEventID, DWORD eventTime, bool bP2P)
{
	// Event info.
	sys_log(0, "<--! Event SetTime (%d) !-->", eventTime);

	m_dwEventEndTime[bEventID] = get_global_time() + eventTime;
	m_bLimitedEvent[bEventID] = true;

	// P2P
	if (bP2P)
	{
		TPacketGGEventInfo pack_p2p;
		pack_p2p.header = HEADER_GG_EVENT_TIME;
		pack_p2p.event_id = bEventID;
		pack_p2p.event_time = eventTime;
		P2P_MANAGER::instance().Send(&pack_p2p, sizeof(pack_p2p));
	}
}

bool CGameEventsManager::IsActivateEvent(BYTE bEventID)
{
	return m_pkActivateEvents[bEventID];
}

DWORD CGameEventsManager::GetEventTime(BYTE bEventID)
{
	return m_dwEventEndTime[bEventID];
}

bool CGameEventsManager::IsLimitedEvent(BYTE bEventID)
{
	return m_bLimitedEvent[bEventID];
}