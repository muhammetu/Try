#pragma once
class CPrivManager : public singleton<CPrivManager>
{
public:
	CPrivManager();

	void RequestGiveEmpirePriv(BYTE empire, BYTE type, int value, time_t dur_time_sec);

	void GiveEmpirePriv(BYTE empire, BYTE type, int value, BYTE bLog, time_t end_time_sec);

	void RemoveEmpirePriv(BYTE empire, BYTE type);

	int GetPriv(LPCHARACTER ch, BYTE type);
	int GetPrivByEmpire(BYTE bEmpire, BYTE type);

public:
	struct SPrivEmpireData
	{
		int m_value;
		time_t m_end_time_sec;
	};

	SPrivEmpireData* GetPrivByEmpireEx(BYTE bEmpire, BYTE type);

private:
	SPrivEmpireData m_aakPrivEmpireData[MAX_PRIV_NUM][EMPIRE_MAX_NUM];
};