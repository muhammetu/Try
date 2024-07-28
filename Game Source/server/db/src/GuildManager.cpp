#include "stdafx.h"
#include "GuildManager.h"
#include "Main.h"
#include "ClientManager.h"
#include "QID.h"
#include "Config.h"
#include <math.h>

extern std::string g_stLocale;

const int GUILD_RANK_MAX_NUM = 20;

bool isEurope()
{
	do
	{
		if (g_stLocale.compare("turkey") == 0) break;
		return false;
	} while (false);

	return true;
}

DWORD GetGuildWarWaitStartDuration()
{
	// const int GUILD_WAR_WAIT_START_DURATION = 60;
	// const int GUILD_WAR_WAIT_START_DURATION = 5;

	if (isEurope() == true) return 60;
	else return 5;
}

namespace
{
	struct FSendPeerWar
	{
		FSendPeerWar(BYTE bType, BYTE bWar, DWORD GID1, DWORD GID2
#ifdef __GUILD_WAR_REWORK__
			, int iMaxPlayer, int iMaxScore
#endif
		)
		{
			if (number(0, 1))
				std::swap(GID1, GID2);

			memset(&p, 0, sizeof(TPacketGuildWar));

			p.bWar = bWar;
			p.bType = bType;
			p.dwGuildFrom = GID1;
			p.dwGuildTo = GID2;
#ifdef __GUILD_WAR_REWORK__
			p.iMaxPlayer = iMaxPlayer;
			p.iMaxScore = iMaxScore;
#endif
		}

		void operator() (CPeer* peer)
		{
			if (peer->GetChannel() == 0)
				return;

			peer->EncodeHeader(HEADER_DG_GUILD_WAR, 0, sizeof(TPacketGuildWar));
			peer->Encode(&p, sizeof(TPacketGuildWar));
		}

		TPacketGuildWar p;
	};

	struct FSendGuildWarScore
	{
		FSendGuildWarScore(DWORD guild_gain, DWORD dwOppGID, int iScore)
		{
			pck.dwGuildGainPoint = guild_gain;
			pck.dwGuildOpponent = dwOppGID;
			pck.lScore = iScore;
		}

		void operator() (CPeer* peer)
		{
			if (peer->GetChannel() == 0)
				return;

			peer->EncodeHeader(HEADER_DG_GUILD_WAR_SCORE, 0, sizeof(pck));
			peer->Encode(&pck, sizeof(pck));
		}

		TPacketGuildWarScore pck;
	};
}

CGuildManager::CGuildManager()
{
}

CGuildManager::~CGuildManager()
{
	while (!m_pqOnWar.empty())
	{
		if (!m_pqOnWar.top().second->bEnd)
			delete m_pqOnWar.top().second;

		m_pqOnWar.pop();
	}
}

TGuild& CGuildManager::TouchGuild(DWORD GID)
{
	itertype(m_map_kGuild) it = m_map_kGuild.find(GID);

	if (it != m_map_kGuild.end())
		return it->second;

	TGuild info;
	m_map_kGuild.insert(std::map<DWORD, TGuild>::value_type(GID, info));
	return m_map_kGuild[GID];
}

void CGuildManager::ParseResult(SQLResult* pRes)
{
	MYSQL_ROW row;

	while ((row = mysql_fetch_row(pRes->pSQLResult)))
	{
		DWORD GID = strtoul(row[0], NULL, 10);

		TGuild& r_info = TouchGuild(GID);

		strlcpy(r_info.szName, row[1], sizeof(r_info.szName));
		str_to_number(r_info.ladder_point, row[2]);
		str_to_number(r_info.win, row[3]);
		str_to_number(r_info.draw, row[4]);
		str_to_number(r_info.loss, row[5]);
		str_to_number(r_info.level, row[7]);

		//sys_log(0,
		//	"GuildWar: %-24s ladder %-5d win %-3d draw %-3d loss %-3d",
		//	r_info.szName,
		//	r_info.ladder_point,
		//	r_info.win,
		//	r_info.draw,
		//	r_info.loss);
	}
}

void CGuildManager::Initialize()
{
	char szQuery[1024];
	snprintf(szQuery, sizeof(szQuery), "SELECT id, name, ladder_point, win, draw, loss, level FROM guild%s", GetTablePostfix());
	std::unique_ptr<SQLMsg> pmsg(CDBManager::instance().DirectQuery(szQuery, SQL_PLAYER_GUILD));

	if (pmsg->Get()->uiNumRows)
		ParseResult(pmsg->Get());

	char str[128 + 1];

	if (!CConfig::instance().GetValue("POLY_POWER", str, sizeof(str)))
		*str = '\0';

	if (!polyPower.Analyze(str))
		sys_err("cannot set power poly: %s", str);
	else
		sys_log(0, "POWER_POLY: %s", str);

	if (!CConfig::instance().GetValue("POLY_HANDICAP", str, sizeof(str)))
		*str = '\0';

	if (!polyHandicap.Analyze(str))
		sys_err("cannot set handicap poly: %s", str);
	else
		sys_log(0, "HANDICAP_POLY: %s", str);

	QueryRanking();
}

void CGuildManager::Load(DWORD dwGuildID)
{
	char szQuery[1024];

	snprintf(szQuery, sizeof(szQuery), "SELECT id, name, ladder_point, win, draw, loss, level FROM guild%s WHERE id=%u", GetTablePostfix(), dwGuildID);
	std::unique_ptr<SQLMsg> pmsg(CDBManager::instance().DirectQuery(szQuery, SQL_PLAYER_GUILD));

	if (pmsg->Get()->uiNumRows)
		ParseResult(pmsg->Get());
}

void CGuildManager::QueryRanking()
{
	char szQuery[256];
	snprintf(szQuery, sizeof(szQuery), "SELECT id,name,ladder_point FROM guild%s ORDER BY ladder_point DESC LIMIT 20", GetTablePostfix());

	CDBManager::instance().ReturnQuery(szQuery, QID_GUILD_RANKING, 0, 0, SQL_PLAYER_GUILD);
}

int CGuildManager::GetRanking(DWORD dwGID)
{
	itertype(map_kLadderPointRankingByGID) it = map_kLadderPointRankingByGID.find(dwGID);

	if (it == map_kLadderPointRankingByGID.end())
		return GUILD_RANK_MAX_NUM;

	return MINMAX(0, it->second, GUILD_RANK_MAX_NUM);
}

void CGuildManager::ResultRanking(MYSQL_RES* pRes)
{
	if (!pRes)
		return;

	int iLastLadderPoint = -1;
	int iRank = 0;

	map_kLadderPointRankingByGID.clear();

	MYSQL_ROW row;

	while ((row = mysql_fetch_row(pRes)))
	{
		DWORD	dwGID = 0; str_to_number(dwGID, row[0]);
		int	iLadderPoint = 0; str_to_number(iLadderPoint, row[2]);

		if (iLadderPoint != iLastLadderPoint)
			++iRank;

		//sys_log(0, "GUILD_RANK: %-24s %2d %d", row[1], iRank, iLadderPoint);

		map_kLadderPointRankingByGID.insert(std::make_pair(dwGID, iRank));
	}
}

void CGuildManager::Update()
{
	time_t now = CClientManager::instance().GetCurrentTime();

	if (!m_pqOnWar.empty())
	{
		// UNKNOWN_GUILD_MANAGE_UPDATE_LOG
		/*
		   sys_log(0, "GuildManager::Update size %d now %d top %d, %s(%u) vs %s(%u)",
		   m_WarMap.size(),
		   now,
		   m_pqOnWar.top().first,
		   m_map_kGuild[m_pqOnWar.top().second->GID[0]].szName,
		   m_pqOnWar.top().second->GID[0],
		   m_map_kGuild[m_pqOnWar.top().second->GID[1]].szName,
		   m_pqOnWar.top().second->GID[1]);
		   */
		   // END_OF_UNKNOWN_GUILD_MANAGE_UPDATE_LOG

		while (!m_pqOnWar.empty() && (m_pqOnWar.top().first <= now || (m_pqOnWar.top().second && m_pqOnWar.top().second->bEnd)))
		{
			TGuildWarPQElement* e = m_pqOnWar.top().second;

			m_pqOnWar.pop();

			if (e)
			{
				if (!e->bEnd)
					WarEnd(e->GID[0], e->GID[1], false);

				delete e;
			}
		}
	}

	// GUILD_SKILL_COOLTIME_BUG_FIX
	while (!m_pqSkill.empty() && m_pqSkill.top().first <= now)
	{
		const TGuildSkillUsed& s = m_pqSkill.top().second;
		CClientManager::instance().SendGuildSkillUsable(s.GID, s.dwSkillVnum, true);
		m_pqSkill.pop();
	}
	// END_OF_GUILD_SKILL_COOLTIME_BUG_FIX

	while (!m_pqWaitStart.empty() && m_pqWaitStart.top().first <= now)
	{
		const TGuildWaitStartInfo& ws = m_pqWaitStart.top().second;
		m_pqWaitStart.pop();

#ifdef __GUILD_WAR_REWORK__
		StartWar(ws.bType, ws.GID[0], ws.GID[1], ws.iMaxPlayer, ws.iMaxScore); // insert new element to m_WarMap and m_pqOnWar
#else
		StartWar(ws.bType, ws.GID[0], ws.GID[1]); // insert new element to m_WarMap and m_pqOnWar
#endif

		if (ws.lInitialScore)
		{
			UpdateScore(ws.GID[0], ws.GID[1], ws.lInitialScore);
			UpdateScore(ws.GID[1], ws.GID[0], ws.lInitialScore);
		}

		TPacketGuildWar p;

		p.bType = ws.bType;
		p.bWar = GUILD_WAR_ON_WAR;
		p.dwGuildFrom = ws.GID[0];
		p.dwGuildTo = ws.GID[1];
#ifdef __GUILD_WAR_REWORK__
		p.iMaxPlayer = ws.iMaxPlayer;
		p.iMaxScore = ws.iMaxScore;
#endif

		CClientManager::instance().ForwardPacket(HEADER_DG_GUILD_WAR, &p, sizeof(p));
		sys_log(0, "GuildWar: GUILD sending start of wait start war %d %d", ws.GID[0], ws.GID[1]);
	}
}

#define for_all(cont, it) for (typeof((cont).begin()) it = (cont).begin(); it != (cont).end(); ++it)

void CGuildManager::OnSetup(CPeer* peer)
{
	for_all(m_WarMap, it_cont)
		for_all(it_cont->second, it)
	{
		DWORD g1 = it_cont->first;
		DWORD g2 = it->first;
		TGuildWarPQElement* p = it->second.pElement;

		if (!p || p->bEnd)
			continue;

#ifdef __GUILD_WAR_REWORK__
		FSendPeerWar(p->bType, GUILD_WAR_ON_WAR, g1, g2, p->iMaxPlayer, p->iMaxScore) (peer);
#else
		FSendPeerWar(p->bType, GUILD_WAR_ON_WAR, g1, g2) (peer);
#endif
		FSendGuildWarScore(p->GID[0], p->GID[1], p->iScore[0]);
		FSendGuildWarScore(p->GID[1], p->GID[0], p->iScore[1]);
	}

	for_all(m_DeclareMap, it)
	{
#ifdef __GUILD_WAR_REWORK__
		FSendPeerWar(it->bType, GUILD_WAR_SEND_DECLARE, it->dwGuildID[0], it->dwGuildID[1], it->iMaxPlayer, it->iMaxScore) (peer);
#else
		FSendPeerWar(it->bType, GUILD_WAR_SEND_DECLARE, it->dwGuildID[0], it->dwGuildID[1]) (peer);
#endif
	}
}

void CGuildManager::GuildWarWin(DWORD GID)
{
	itertype(m_map_kGuild) it = m_map_kGuild.find(GID);

	if (it == m_map_kGuild.end())
		return;

	++it->second.win;

	char buf[1024];
	snprintf(buf, sizeof(buf), "UPDATE guild%s SET win=%d WHERE id=%u", GetTablePostfix(), it->second.win, GID);
	CDBManager::instance().AsyncQuery(buf, SQL_PLAYER_GUILD);
}

void CGuildManager::GuildWarLose(DWORD GID)
{
	itertype(m_map_kGuild) it = m_map_kGuild.find(GID);

	if (it == m_map_kGuild.end())
		return;

	++it->second.loss;

	char buf[1024];
	snprintf(buf, sizeof(buf), "UPDATE guild%s SET loss=%d WHERE id=%u", GetTablePostfix(), it->second.loss, GID);
	CDBManager::instance().AsyncQuery(buf, SQL_PLAYER_GUILD);
}

void CGuildManager::GuildWarDraw(DWORD GID)
{
	itertype(m_map_kGuild) it = m_map_kGuild.find(GID);

	if (it == m_map_kGuild.end())
		return;

	++it->second.draw;

	char buf[1024];
	snprintf(buf, sizeof(buf), "UPDATE guild%s SET draw=%d WHERE id=%u", GetTablePostfix(), it->second.draw, GID);
	CDBManager::instance().AsyncQuery(buf, SQL_PLAYER_GUILD);
}

bool CGuildManager::IsHalfWinLadderPoint(DWORD dwGuildWinner, DWORD dwGuildLoser)
{
	DWORD GID1 = dwGuildWinner;
	DWORD GID2 = dwGuildLoser;

	if (GID1 > GID2)
		std::swap(GID1, GID2);

	itertype(m_mapGuildWarEndTime[GID1]) it = m_mapGuildWarEndTime[GID1].find(GID2);

	if (it != m_mapGuildWarEndTime[GID1].end() &&
		it->second + GUILD_WAR_LADDER_HALF_PENALTY_TIME > CClientManager::instance().GetCurrentTime())
		return true;

	return false;
}

void CGuildManager::ProcessDraw(DWORD dwGuildID1, DWORD dwGuildID2)
{
	sys_log(0, "GuildWar: \tThe war between %d and %d is ended in draw", dwGuildID1, dwGuildID2);

	GuildWarDraw(dwGuildID1);
	GuildWarDraw(dwGuildID2);
	ChangeLadderPoint(dwGuildID1, 0);
	ChangeLadderPoint(dwGuildID2, 0);

	QueryRanking();
}

void CGuildManager::ProcessWinLose(DWORD dwGuildWinner, DWORD dwGuildLoser)
{
	GuildWarWin(dwGuildWinner);
	GuildWarLose(dwGuildLoser);
	sys_log(0, "GuildWar: \tWinner : %d Loser : %d", dwGuildWinner, dwGuildLoser);

	int iPoint = GetLadderPoint(dwGuildLoser);
	int gain = (int)(iPoint * 0.05);
	int loss = (int)(iPoint * 0.07);

	if (IsHalfWinLadderPoint(dwGuildWinner, dwGuildLoser))
		gain /= 2;

	sys_log(0, "GuildWar: \tgain : %d loss : %d", gain, loss);

	ChangeLadderPoint(dwGuildWinner, gain);
	ChangeLadderPoint(dwGuildLoser, -loss);

	QueryRanking();
}

void CGuildManager::RemoveWar(DWORD GID1, DWORD GID2)
{
	sys_log(0, "GuildWar: RemoveWar(%d, %d)", GID1, GID2);

	if (GID1 > GID2)
		std::swap(GID2, GID1);

	itertype(m_WarMap[GID1]) it = m_WarMap[GID1].find(GID2);

	if (it == m_WarMap[GID1].end())
	{
		if (m_WarMap[GID1].empty())
			m_WarMap.erase(GID1);

		return;
	}

	if (it->second.pElement)
		it->second.pElement->bEnd = true;

	m_mapGuildWarEndTime[GID1][GID2] = CClientManager::instance().GetCurrentTime();

	m_WarMap[GID1].erase(it);

	if (m_WarMap[GID1].empty())
		m_WarMap.erase(GID1);
}

//

//
void CGuildManager::WarEnd(DWORD GID1, DWORD GID2, bool bForceDraw)
{
	if (GID1 > GID2)
		std::swap(GID2, GID1);

	sys_log(0, "GuildWar: WarEnd %d %d", GID1, GID2);

	itertype(m_WarMap[GID1]) itWarMap = m_WarMap[GID1].find(GID2);

	if (itWarMap == m_WarMap[GID1].end())
	{
		sys_err("GuildWar: war not exist or already ended. [1]");
		return;
	}

	TGuildWarInfo gwi = itWarMap->second;
	TGuildWarPQElement* pData = gwi.pElement;

	if (!pData || pData->bEnd)
	{
		sys_err("GuildWar: war not exist or already ended. [2]");
		return;
	}

	DWORD win_guild = pData->GID[0];
	DWORD lose_guild = pData->GID[1];

	bool bDraw = false;

	if (!bForceDraw)
	{
		if (pData->iScore[0] > pData->iScore[1])
		{
			win_guild = pData->GID[0];
			lose_guild = pData->GID[1];
		}
		else if (pData->iScore[1] > pData->iScore[0])
		{
			win_guild = pData->GID[1];
			lose_guild = pData->GID[0];
		}
		else
			bDraw = true;
	}
	else
		bDraw = true;

	if (bDraw)
		ProcessDraw(win_guild, lose_guild);
	else
		ProcessWinLose(win_guild, lose_guild);

#ifdef __GUILD_WAR_REWORK__
	CClientManager::instance().for_each_peer(FSendPeerWar(0, GUILD_WAR_END, GID1, GID2, pData->iMaxPlayer, pData->iMaxScore));
#else
	CClientManager::instance().for_each_peer(FSendPeerWar(0, GUILD_WAR_END, GID1, GID2));
#endif
	RemoveWar(GID1, GID2);
}

void CGuildManager::RecvWarOver(DWORD dwGuildWinner, DWORD dwGuildLoser, bool bDraw
#ifdef __GUILD_WAR_REWORK__
	, int iMaxPlayer, int iMaxScore
#endif
)
{
#ifdef __GUILD_WAR_REWORK__
	sys_log(0, "GuildWar: RecvWarOver : winner %u vs %u draw? %d MaxPlayer %d MaxScore %d", dwGuildWinner, dwGuildLoser, bDraw ? 1 : 0, iMaxPlayer, iMaxScore);
#else
	sys_log(0, "GuildWar: RecvWarOver : winner %u vs %u draw? %d", dwGuildWinner, dwGuildLoser, bDraw ? 1 : 0);
#endif
	DWORD GID1 = dwGuildWinner;
	DWORD GID2 = dwGuildLoser;

	if (GID1 > GID2)
		std::swap(GID1, GID2);

	itertype(m_WarMap[GID1]) it = m_WarMap[GID1].find(GID2);

	if (it == m_WarMap[GID1].end())
		return;

	//TGuildWarInfo& gw = it->second;

	// Award
	if (bDraw)
	{
		ProcessDraw(dwGuildWinner, dwGuildLoser);
	}
	else
	{
		ProcessWinLose(dwGuildWinner, dwGuildLoser);
	}

	RemoveWar(GID1, GID2);
}

void CGuildManager::RecvWarEnd(DWORD GID1, DWORD GID2
#ifdef __GUILD_WAR_REWORK__
	, int iMaxPlayer, int iMaxScore
#endif
)
{
#ifdef __GUILD_WAR_REWORK__
	sys_log(0, "GuildWar: RecvWarEnded : %u vs %u with MaxPlayer %d MaxScore %d", GID1, GID2, iMaxPlayer, iMaxScore);
#else
	sys_log(0, "GuildWar: RecvWarEnded : %u vs %u", GID1, GID2);
#endif
	WarEnd(GID1, GID2, true);
}

void CGuildManager::StartWar(BYTE bType, DWORD GID1, DWORD GID2
#ifdef __GUILD_WAR_REWORK__
	, int iMaxPlayer, int iMaxScore
#endif
)
{
#ifdef __GUILD_WAR_REWORK__
	sys_log(0, "GuildWar: StartWar(%d,%d,%d,%d,%d)", bType, GID1, GID2, iMaxPlayer, iMaxScore);
#else
	sys_log(0, "GuildWar: StartWar(%d,%d,%d)", bType, GID1, GID2);
#endif
	if (GID1 > GID2)
		std::swap(GID1, GID2);

	TGuildWarInfo& gw = m_WarMap[GID1][GID2]; // map insert

	if (bType == GUILD_WAR_TYPE_FIELD)
		gw.tEndTime = CClientManager::instance().GetCurrentTime() + GUILD_WAR_DURATION;
	else
		gw.tEndTime = CClientManager::instance().GetCurrentTime() + 172800;

#ifdef __GUILD_WAR_REWORK__
	gw.pElement = new TGuildWarPQElement(bType, GID1, GID2, iMaxPlayer, iMaxScore);
#else
	gw.pElement = new TGuildWarPQElement(bType, GID1, GID2);
#endif

	m_pqOnWar.push(std::make_pair(gw.tEndTime, gw.pElement));
}

void CGuildManager::UpdateScore(DWORD dwGainGID, DWORD dwOppGID, int iScoreDelta)
{
	DWORD GID1 = dwGainGID;
	DWORD GID2 = dwOppGID;

	if (GID1 > GID2)
		std::swap(GID1, GID2);

	itertype(m_WarMap[GID1]) it = m_WarMap[GID1].find(GID2);

	if (it != m_WarMap[GID1].end())
	{
		TGuildWarPQElement* p = it->second.pElement;

		if (!p || p->bEnd)
		{
			sys_err("GuildWar: war not exist or already ended.");
			return;
		}

		int iNewScore = 0;

		if (p->GID[0] == dwGainGID)
		{
			p->iScore[0] += iScoreDelta;

			iNewScore = p->iScore[0];
		}
		else
		{
			p->iScore[1] += iScoreDelta;

			iNewScore = p->iScore[1];
		}

#ifdef __GUILD_WAR_REWORK__
		sys_log(0, "GuildWar: SendGuildWarScore guild %u wartype %u score_delta %d result %u, max score: %u",
			dwGainGID, p->bType, iScoreDelta, iNewScore, p->iMaxScore);
#else
		sys_log(0, "GuildWar: SendGuildWarScore guild %u wartype %u score_delta %d result %u",
			dwGainGID, p->bType, iScoreDelta, iNewScore);
#endif
		CClientManager::instance().for_each_peer(FSendGuildWarScore(dwGainGID, dwOppGID, iNewScore));
	}
}

void CGuildManager::AddDeclare(BYTE bType, DWORD guild_from, DWORD guild_to
#ifdef __GUILD_WAR_REWORK__
	, int iMaxPlayer, int iMaxScore
#endif
)
{
#ifdef __GUILD_WAR_REWORK__
	TGuildDeclareInfo di(bType, guild_from, guild_to, iMaxPlayer, iMaxScore);
#else
	TGuildDeclareInfo di(bType, guild_from, guild_to);
#endif
	if (m_DeclareMap.find(di) == m_DeclareMap.end())
		m_DeclareMap.insert(di);

#ifdef __GUILD_WAR_REWORK__
	sys_log(0, "GuildWar: AddDeclare(Type:%d,from:%d,to:%d,maxplayer:%d,maxscore:%d)", bType, guild_from, guild_to, iMaxPlayer, iMaxScore);
#else
	sys_log(0, "GuildWar: AddDeclare(Type:%d,from:%d,to:%d)", bType, guild_from, guild_to);
#endif
}

void CGuildManager::RemoveDeclare(DWORD guild_from, DWORD guild_to
#ifdef __GUILD_WAR_REWORK__
	, int iMaxPlayer, int iMaxScore
#endif
)
{
#ifdef __GUILD_WAR_REWORK__
	typeof(m_DeclareMap.begin()) it = m_DeclareMap.find(TGuildDeclareInfo(0, guild_from, guild_to, iMaxPlayer, iMaxScore));
#else
	typeof(m_DeclareMap.begin()) it = m_DeclareMap.find(TGuildDeclareInfo(0, guild_from, guild_to));
#endif
	if (it != m_DeclareMap.end())
		m_DeclareMap.erase(it);

#ifdef __GUILD_WAR_REWORK__
	it = m_DeclareMap.find(TGuildDeclareInfo(0, guild_to, guild_from, iMaxPlayer, iMaxScore));
#else
	it = m_DeclareMap.find(TGuildDeclareInfo(0, guild_to, guild_from));
#endif

	if (it != m_DeclareMap.end())
		m_DeclareMap.erase(it);

#ifdef __GUILD_WAR_REWORK__
	sys_log(0, "GuildWar: RemoveDeclare(from:%d,to:%d,maxplayer:%d,maxscore:%d)", guild_from, guild_to, iMaxPlayer, iMaxScore);
#else
	sys_log(0, "GuildWar: RemoveDeclare(from:%d,to:%d)", guild_from, guild_to);
#endif
}

bool CGuildManager::WaitStart(TPacketGuildWar* p)
{
	DWORD dwCurTime = CClientManager::instance().GetCurrentTime();

#ifdef __GUILD_WAR_REWORK__
	TGuildWaitStartInfo info(p->bType, p->dwGuildFrom, p->dwGuildTo, p->lInitialScore, p->iMaxPlayer, p->iMaxScore);
#else
	TGuildWaitStartInfo info(p->bType, p->dwGuildFrom, p->dwGuildTo, p->lInitialScore, NULL);
#endif
	m_pqWaitStart.push(std::make_pair(dwCurTime + GetGuildWarWaitStartDuration(), info));

	sys_log(0,
		"GuildWar: WaitStart g1 %d g2 %d start at %u"
#ifdef __GUILD_WAR_REWORK__
		"max player %d max score %d"
#endif
		,
		p->dwGuildFrom,
		p->dwGuildTo,
		dwCurTime + GetGuildWarWaitStartDuration()
#ifdef __GUILD_WAR_REWORK__
		, p->iMaxPlayer,
		p->iMaxScore
#endif
	);

	return true;
}

int CGuildManager::GetLadderPoint(DWORD GID)
{
	itertype(m_map_kGuild) it = m_map_kGuild.find(GID);

	if (it == m_map_kGuild.end())
		return 0;

	return it->second.ladder_point;
}

void CGuildManager::ChangeLadderPoint(DWORD GID, int change)
{
	itertype(m_map_kGuild) it = m_map_kGuild.find(GID);

	if (it == m_map_kGuild.end())
		return;

	TGuild& r = it->second;

	r.ladder_point += change;

	if (r.ladder_point < 0)
		r.ladder_point = 0;

	char buf[1024];
	snprintf(buf, sizeof(buf), "UPDATE guild%s SET ladder_point=%d WHERE id=%u", GetTablePostfix(), r.ladder_point, GID);
	CDBManager::instance().AsyncQuery(buf, SQL_PLAYER_GUILD);

	sys_log(0, "GuildManager::ChangeLadderPoint %u %d", GID, r.ladder_point);
	sys_log(0, "%s", buf);

	TPacketGuildLadder p;

	p.dwGuild = GID;
	p.lLadderPoint = r.ladder_point;
	p.lWin = r.win;
	p.lDraw = r.draw;
	p.lLoss = r.loss;

	CClientManager::instance().ForwardPacket(HEADER_DG_GUILD_LADDER, &p, sizeof(TPacketGuildLadder));
}

void CGuildManager::UseSkill(DWORD GID, DWORD dwSkillVnum, DWORD dwCooltime)
{
	// GUILD_SKILL_COOLTIME_BUG_FIX
	sys_log(0, "UseSkill(gid=%d, skill=%d) CoolTime(%d:%d)", GID, dwSkillVnum, dwCooltime, CClientManager::instance().GetCurrentTime() + dwCooltime);
	m_pqSkill.push(std::make_pair(CClientManager::instance().GetCurrentTime() + dwCooltime, TGuildSkillUsed(GID, dwSkillVnum)));
	// END_OF_GUILD_SKILL_COOLTIME_BUG_FIX
}

const int c_aiScoreByLevel[GUILD_MAX_LEVEL + 1] =
{
	500,	// level 0 = 500 probably error
	500,	// 1
	1000,
	2000,
	3000,
	4000,
	6000,
	8000,
	10000,
	12000,
	15000,	// 10
	18000,
	21000,
	24000,
	28000,
	32000,
	36000,
	40000,
	45000,
	50000,
	55000,
};

const int c_aiScoreByRanking[GUILD_RANK_MAX_NUM + 1] =
{
	0,
	55000,
	50000,
	45000,
	40000,
	36000,
	32000,
	28000,
	24000,
	21000,
	18000,
	15000,
	12000,
	10000,
	8000,
	6000,
	4000,
	3000,
	2000,
	1000,
	500
};

int GetAverageGuildMemberLevel(DWORD dwGID)
{
	char szQuery[QUERY_MAX_LEN];

	snprintf(szQuery, sizeof(szQuery),
		"SELECT AVG(level) FROM guild_member%s, player%s AS p WHERE guild_id=%u AND guild_member%s.pid=p.id",
		GetTablePostfix(), GetTablePostfix(), dwGID, GetTablePostfix());

	std::unique_ptr<SQLMsg> msg(CDBManager::instance().DirectQuery(szQuery, SQL_PLAYER_GUILD));

	MYSQL_ROW row;
	row = mysql_fetch_row(msg->Get()->pSQLResult);

	int nAverageLevel = 0; str_to_number(nAverageLevel, row[0]);
	return nAverageLevel;
}

int GetGuildMemberCount(DWORD dwGID)
{
	char szQuery[QUERY_MAX_LEN];

	snprintf(szQuery, sizeof(szQuery), "SELECT COUNT(*) FROM guild_member%s WHERE guild_id=%u", GetTablePostfix(), dwGID);

	std::unique_ptr<SQLMsg> msg(CDBManager::instance().DirectQuery(szQuery, SQL_PLAYER_GUILD));

	MYSQL_ROW row;
	row = mysql_fetch_row(msg->Get()->pSQLResult);

	DWORD dwCount = 0; str_to_number(dwCount, row[0]);
	return dwCount;
}

void CGuildManager::CancelWar(DWORD GID1, DWORD GID2
#ifdef __GUILD_WAR_REWORK__
	, int iMaxPlayer, int iMaxScore
#endif
)
{
#ifdef __GUILD_WAR_REWORK__
	RemoveDeclare(GID1, GID2, iMaxPlayer, iMaxScore);
#else
	RemoveDeclare(GID1, GID2);
#endif
	RemoveWar(GID1, GID2);
}

bool CGuildManager::ChangeMaster(DWORD dwGID, DWORD dwFrom, DWORD dwTo)
{
	itertype(m_map_kGuild) iter = m_map_kGuild.find(dwGID);

	if (iter == m_map_kGuild.end())
		return false;

	char szQuery[1024];

	snprintf(szQuery, sizeof(szQuery), "UPDATE guild%s SET master=%u WHERE id=%u", GetTablePostfix(), dwTo, dwGID);
	delete CDBManager::instance().DirectQuery(szQuery, SQL_PLAYER_GUILD);

	snprintf(szQuery, sizeof(szQuery), "UPDATE guild_member%s SET grade=1 WHERE pid=%u", GetTablePostfix(), dwTo);
	delete CDBManager::instance().DirectQuery(szQuery, SQL_PLAYER_GUILD);

	snprintf(szQuery, sizeof(szQuery), "UPDATE guild_member%s SET grade=15 WHERE pid=%u", GetTablePostfix(), dwFrom);
	delete CDBManager::instance().DirectQuery(szQuery, SQL_PLAYER_GUILD);

	return true;
}