#pragma once

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

class CPythonGameEvents : public CSingleton<CPythonGameEvents>
{
public:
	CPythonGameEvents();
	virtual ~CPythonGameEvents();

	void	SetActivateEvent(bool isActivate, BYTE bEventID);
	void	SetEventTime(BYTE bEventID, DWORD event_time);
	bool	IsActivateEvent(BYTE bEventID);
	DWORD	GetEventTime(BYTE bEventID);
protected:
	bool	m_pkActivateEvents[EVENT_MAX_NUM];
	DWORD	m_dwEventEndTime[EVENT_MAX_NUM];
};