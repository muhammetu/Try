#include "stdafx.h"
#include "PrivManager.h"
#include "ClientManager.h"

const int PRIV_DURATION = 60 * 60 * 12;

CPrivManager::CPrivManager()
{
	for (int type = 0; type < MAX_PRIV_NUM; ++type)
	{
		for (int empire = 0; empire < EMPIRE_MAX_NUM; ++empire)
			m_aaPrivEmpire[type][empire] = 0;
	}
}

CPrivManager::~CPrivManager()
{
}

//

//
void CPrivManager::Update()
{
	time_t now = CClientManager::instance().GetCurrentTime();

	while (!m_pqPrivEmpire.empty() && m_pqPrivEmpire.top().first <= now)
	{
		TPrivEmpireData* p = (m_pqPrivEmpire.top().second);
		m_pqPrivEmpire.pop();

		if (p->value != 0 && !p->bRemoved)
		{
			SendChangeEmpirePriv(p->empire, p->type, 0, 0);
			m_aaPrivEmpire[p->type][p->empire] = 0;
		}

		delete p;
	}
}

void CPrivManager::AddEmpirePriv(BYTE empire, BYTE type, int value, time_t duration_sec)
{
	if (MAX_PRIV_NUM <= type)
	{
		sys_err("PRIV_MANAGER: AddEmpirePriv: wrong empire priv type(%u) recved", type);
		return;
	}

	/*
		Problem : DB Core crash.
		* I have no idea what this problem is causing, but the flag with a maximum value is absurdly coming at a high value.
		* As a solution, i put value control and it improved.
		* I will do some research about the reason later.
	*/
	if (empire >= EMPIRE_MAX_NUM)
	{
		sys_err("PRIV_MANAGER: AddEmpirePriv: Empire limit (%u) exceeded.", empire);
		return;
	}

	if (duration_sec < 0)
		duration_sec = 0;

	time_t now = CClientManager::instance().GetCurrentTime();
	time_t end = now + duration_sec;

	{
		if (m_aaPrivEmpire[type][empire])
			m_aaPrivEmpire[type][empire]->bRemoved = true;
	}

	TPrivEmpireData* p = new TPrivEmpireData(type, value, empire, end);
	m_pqPrivEmpire.push(std::make_pair(end, p));
	m_aaPrivEmpire[type][empire] = p;

	// ADD_EMPIRE_PRIV_TIME
	SendChangeEmpirePriv(empire, type, value, end);
	// END_OF_ADD_EMPIRE_PRIV_TIME

	sys_log(0, "Empire Priv empire(%d) type(%d) value(%d) duration_sec(%d)", empire, type, value, duration_sec);
}

struct FSendChangeEmpirePriv
{
	FSendChangeEmpirePriv(BYTE empire, BYTE type, int value, time_t end_time_sec)
	{
		p.empire = empire;
		p.type = type;
		p.value = value;
		p.bLog = 1;
		// ADD_EMPIRE_PRIV_TIME
		p.end_time_sec = end_time_sec;
		// END_OF_ADD_EMPIRE_PRIV_TIME
	}

	void operator ()(CPeer* peer)
	{
		peer->EncodeHeader(HEADER_DG_CHANGE_EMPIRE_PRIV, 0, sizeof(TPacketDGChangeEmpirePriv));
		peer->Encode(&p, sizeof(TPacketDGChangeEmpirePriv));
		p.bLog = 0;
	}

	TPacketDGChangeEmpirePriv p;
};

// ADD_EMPIRE_PRIV_TIME
void CPrivManager::SendChangeEmpirePriv(BYTE empire, BYTE type, int value, time_t end_time_sec)
{
	CClientManager::instance().for_each_peer(FSendChangeEmpirePriv(empire, type, value, end_time_sec));
}
// END_OF_ADD_EMPIRE_PRIV_TIME

void CPrivManager::SendPrivOnSetup(CPeer* peer)
{
	for (int i = 1; i < MAX_PRIV_NUM; ++i)
	{
		for (int e = 0; e < EMPIRE_MAX_NUM; ++e)
		{
			// ADD_EMPIRE_PRIV_TIME
			TPrivEmpireData* pPrivEmpireData = m_aaPrivEmpire[i][e];
			if (pPrivEmpireData)
			{
				FSendChangeEmpirePriv(e, i, pPrivEmpireData->value, pPrivEmpireData->end_time_sec)(peer);
			}
			// END_OF_ADD_EMPIRE_PRIV_TIME
		}
	}
}