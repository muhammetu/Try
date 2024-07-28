#include "stdafx.h"
#include <string>
#include <fstream>
#include <algorithm>
#include <iostream>
#include "utils.h"
#include "config.h"
#include "constants.h"
#include "auto_event_manager.h"
#include "questmanager.h"
#include "../../common/service.h"
#include "cmd.h"
#ifdef __EVENT_SYSTEM__
#include "priv_manager.h"
#include "game_events.h"
#endif

#ifdef __AUTO_EVENT_SYSTEM__

static LPEVENT running_event = NULL;

EVENTINFO(EventsManagerInfoData)
{
	CAutoEventSystem* pEvents;

	EventsManagerInfoData()
		: pEvents(0)
	{
	}
};

EVENTFUNC(automatic_event_timer)
{
	if (event == NULL)
		return 0;

	if (event->info == NULL)
		return 0;

	EventsManagerInfoData* info = dynamic_cast<EventsManagerInfoData*>(event->info);

	if (info == NULL)
		return 0;

	CAutoEventSystem* pInstance = info->pEvents;

	if (pInstance == NULL)
		return 0;

	CAutoEventSystem::instance().PrepareChecker();
	return PASSES_PER_SEC(1);
}

void CAutoEventSystem::PrepareChecker()
{
	time_t cur_Time = time(NULL);
	struct tm vKey = *localtime(&cur_Time);

	int day = vKey.tm_wday;
	int hour = vKey.tm_hour;
	int minute = vKey.tm_min;
	int second = vKey.tm_sec;
#ifdef __EVENT_SYSTEM__
	CAutoEventSystem::instance().CheckGameEvent(day, hour, minute, second);
#endif
}

#ifdef __EVENT_SYSTEM__
void CAutoEventSystem::CheckGameEvent(int day, int hour, int minute, int second)
{
	for (int i = 0; i < EVENT_MAX_NUM; ++i)
	{
		if (CGameEventsManager::instance().IsActivateEvent(i) == true)
		{
			if (CGameEventsManager::instance().IsLimitedEvent(i) == false)
				continue;

			if (CGameEventsManager::instance().GetEventTime(i) <= get_global_time())
			{
				CGameEventsManager::instance().SetActivateEvent(i, false);
			}
		}
	}
}
#endif

bool CAutoEventSystem::Initialize()
{
	if (running_event != NULL)
	{
		event_cancel(&running_event);
		running_event = NULL;
	}

	EventsManagerInfoData* info = AllocEventInfo<EventsManagerInfoData>();
	info->pEvents = this;

	running_event = event_create(automatic_event_timer, info, PASSES_PER_SEC(30));
	return true;
}

void CAutoEventSystem::Destroy()
{
	if (running_event != NULL)
	{
		event_cancel(&running_event);
		running_event = NULL;
	}
}

#endif