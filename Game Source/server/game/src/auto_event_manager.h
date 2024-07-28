#pragma once
class CAutoEventSystem : public singleton<CAutoEventSystem>
{
	enum days
	{
		MONDAY = 1,
		TUESDAY = 2,
		WEDNESDAY = 3,
		THURSDAY = 4,
		FRIDAY = 5,
		SATURDAY = 6,
		SUNDAY = 0,
	};

public:
	bool		Initialize();
	void		Destroy();
	void		PrepareChecker();
#ifdef __EVENT_SYSTEM__
	void		CheckGameEvent(int day, int hour, int minute, int second);
#endif
};
