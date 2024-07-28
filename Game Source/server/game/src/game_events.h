#pragma once
#include "stdafx.h"
enum
{
	EVENT_DOUBLEMETIN,
	EVENT_DOUBLEBOSS,
	EVENT_MAXDAMAGE,
	EVENT_X2DUYURU,
	EVENT_X2SURE,
	EVENT_UPGRADE,
	EVENT_MAX_NUM,
};

class CGameEventsManager : public singleton<CGameEventsManager>
{
public:
	CGameEventsManager();
	~CGameEventsManager();

	void	SendEventCharacter(LPCHARACTER ch);
	void	SetActivateEvent(BYTE bEventID, bool isActivate);
	void	SetEventTime(BYTE bEventID, DWORD eventTime, bool bP2P = true);
	bool	IsActivateEvent(BYTE bEventID);
	bool	IsLimitedEvent(BYTE bEventID);
	DWORD	GetEventTime(BYTE bEventID);
	bool	IsNeedUpdate() { return m_bNeedUpdate; };
protected:
	bool	m_pkActivateEvents[EVENT_MAX_NUM];
	DWORD	m_dwEventEndTime[EVENT_MAX_NUM];
	bool	m_bLimitedEvent[EVENT_MAX_NUM];
	bool	m_bNeedUpdate;
};