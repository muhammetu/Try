#pragma once
#include "Peer.h"
#include <queue>
#include <utility>

struct TPrivEmpireData
{
	BYTE type;
	int value;
	bool bRemoved;
	BYTE empire;

	// ADD_EMPIRE_PRIV_TIME
	time_t end_time_sec;

	TPrivEmpireData(BYTE type, int value, BYTE empire, time_t end_time_sec)
		: type(type), value(value), bRemoved(false), empire(empire), end_time_sec(end_time_sec)
	{}
	// END_OF_ADD_EMPIRE_PRIV_TIME
};

class CPrivManager : public singleton<CPrivManager>
{
public:
	CPrivManager();
	virtual ~CPrivManager();

	// ADD_EMPIRE_PRIV_TIME
	void AddEmpirePriv(BYTE empire, BYTE type, int value, time_t time_sec);
	// END_OF_ADD_EMPIRE_PRIV_TIME

	void Update();

	void SendPrivOnSetup(CPeer* peer);

private:

	// ADD_EMPIRE_PRIV_TIME
	void SendChangeEmpirePriv(BYTE empire, BYTE type, int value, time_t end_time_sec);
	// END_OF_ADD_EMPIRE_PRIV_TIME

	typedef std::pair<time_t, TPrivEmpireData*> stPairEmpire;
	std::priority_queue<stPairEmpire, std::vector<stPairEmpire>, std::greater<stPairEmpire> > m_pqPrivEmpire;
	TPrivEmpireData* m_aaPrivEmpire[MAX_PRIV_NUM][EMPIRE_MAX_NUM];
};