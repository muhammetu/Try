#pragma once
#include "Peer.h"
#include <queue>
#include <utility>
#include "../../libsql/libsql.h"
#include "../../libpoly/Poly.h"
enum
{
	GUILD_WARP_WAR_CHANNEL = 99
};

struct TGuildDeclareInfo
{
	BYTE bType;
	DWORD dwGuildID[2];
#ifdef __GUILD_WAR_REWORK__
	int iMaxPlayer;
	int iMaxScore;
#endif

	TGuildDeclareInfo(BYTE _bType, DWORD _dwGuildID1, DWORD _dwGuildID2
#ifdef __GUILD_WAR_REWORK__
		, int _iMaxPlayer, int _iMaxScore
#endif
	)
		: bType(_bType)
#ifdef __GUILD_WAR_REWORK__
		, iMaxPlayer(_iMaxPlayer), iMaxScore(_iMaxScore)
#endif
	{
		dwGuildID[0] = _dwGuildID1;
		dwGuildID[1] = _dwGuildID2;
#ifdef __GUILD_WAR_REWORK__
		iMaxPlayer = _iMaxPlayer;
		iMaxScore = _iMaxScore;
#endif
	}

	bool operator < (const TGuildDeclareInfo& r) const
	{
		return ((dwGuildID[0] < r.dwGuildID[0]) || ((dwGuildID[0] == r.dwGuildID[0]) && (dwGuildID[1] < r.dwGuildID[1])));
	}

	TGuildDeclareInfo& operator = (const TGuildDeclareInfo& r)
	{
		bType = r.bType;
		dwGuildID[0] = r.dwGuildID[0];
		dwGuildID[1] = r.dwGuildID[1];
		return *this;
	}
};

struct TGuildWaitStartInfo
{
	BYTE			bType;
	DWORD			GID[2];
	long			lInitialScore;
#ifdef __GUILD_WAR_REWORK__
	int					iMaxPlayer;
	int					iMaxScore;
#endif

	TGuildWaitStartInfo(BYTE _bType, DWORD _g1, DWORD _g2, long _lInitialScore
#ifdef __GUILD_WAR_REWORK__
		, int _iMaxPlayer, int _iMaxScore
#endif
	)
		: bType(_bType), lInitialScore(_lInitialScore)
#ifdef __GUILD_WAR_REWORK__
		, iMaxPlayer(_iMaxPlayer), iMaxScore(_iMaxScore)
#endif
	{
		GID[0] = _g1;
		GID[1] = _g2;
	}

	bool operator < (const TGuildWaitStartInfo& r) const
	{
		return ((GID[0] < r.GID[0]) || ((GID[0] == r.GID[0]) && (GID[1] < r.GID[1])));
	}
};

struct TGuildWarPQElement
{
	bool	bEnd;
	BYTE	bType;
	DWORD	GID[2];
	DWORD	iScore[2];
#ifdef __GUILD_WAR_REWORK__
	int		iMaxPlayer;
	int		iMaxScore;
#endif

	TGuildWarPQElement(BYTE _bType, DWORD GID1, DWORD GID2
#ifdef __GUILD_WAR_REWORK__
		, int _iMaxPlayer, int _iMaxScore
#endif
	)
		: bEnd(false), bType(_bType)
#ifdef __GUILD_WAR_REWORK__
		, iMaxPlayer(_iMaxPlayer), iMaxScore(_iMaxScore)
#endif
	{
		bType = _bType;
		GID[0] = GID1;
		GID[1] = GID2;
		iScore[0] = iScore[1] = 0;
#ifdef __GUILD_WAR_REWORK__
		iMaxPlayer = _iMaxPlayer;
		iMaxScore = _iMaxScore;
#endif
	}
};

struct TGuildSkillUsed
{
	DWORD GID;
	DWORD dwSkillVnum;

	// GUILD_SKILL_COOLTIME_BUG_FIX
	TGuildSkillUsed(DWORD _GID, DWORD _dwSkillVnum) : GID(_GID), dwSkillVnum(_dwSkillVnum)
	{
	}
	// END_OF_GUILD_SKILL_COOLTIME_BUG_FIX
};

inline bool operator < (const TGuildSkillUsed& a, const TGuildSkillUsed& b)
{
	return ((a.GID < b.GID) || ((a.GID == b.GID) && (a.dwSkillVnum < b.dwSkillVnum)));
}

typedef struct SGuild
{
	SGuild() : ladder_point(0), win(0), draw(0), loss(0), level(0)
	{
		memset(szName, 0, sizeof(szName));
	}

	char szName[GUILD_NAME_MAX_LEN + 1];
	int	ladder_point;
	int	win;
	int	draw;
	int	loss;
	int	level;
} TGuild;

typedef struct SGuildWarInfo
{
	time_t		tEndTime;
	TGuildWarPQElement* pElement;

	SGuildWarInfo() : pElement(NULL)
	{
	}
} TGuildWarInfo;

class CGuildManager : public singleton<CGuildManager>
{
public:
	CGuildManager();
	virtual ~CGuildManager();

	void	Initialize();

	void	Load(DWORD dwGuildID);

	TGuild& TouchGuild(DWORD GID);

	void	Update();

	void	OnSetup(CPeer* peer);
#ifdef __GUILD_WAR_REWORK__
	void	StartWar(BYTE bType, DWORD GID1, DWORD GID2, int iMaxPlayer, int iMaxScore);
#else
	void	StartWar(BYTE bType, DWORD GID1, DWORD GID2);
#endif

	void	UpdateScore(DWORD guild_gain_point, DWORD guild_opponent, int iScore);

#ifdef __GUILD_WAR_REWORK__
	void	AddDeclare(BYTE bType, DWORD guild_from, DWORD guild_to, int iMaxPlayer, int iMaxScore);
	void	RemoveDeclare(DWORD guild_from, DWORD guild_to, int iMaxPlayer, int iMaxScore);
#else
	void	AddDeclare(BYTE bType, DWORD guild_from, DWORD guild_to);
	void	RemoveDeclare(DWORD guild_from, DWORD guild_to);
#endif

	bool	WaitStart(TPacketGuildWar* p);
#ifdef __GUILD_WAR_REWORK__
	void	RecvWarEnd(DWORD GID1, DWORD GID2, int iMaxPlayer, int iMaxScore);
	void	RecvWarOver(DWORD dwGuildWinner, DWORD dwGuildLoser, bool bDraw, int iMaxPlayer, int iMaxScore);
#else
	void	RecvWarEnd(DWORD GID1, DWORD GID2);
	void	RecvWarOver(DWORD dwGuildWinner, DWORD dwGuildLoser, bool bDraw);
#endif
	void	ChangeLadderPoint(DWORD GID, int change);

	void	UseSkill(DWORD dwGuild, DWORD dwSkillVnum, DWORD dwCooltime);

	void	QueryRanking();
	void	ResultRanking(MYSQL_RES* pRes);
	int	GetRanking(DWORD dwGID);

#ifdef __GUILD_WAR_REWORK__
	void	CancelWar(DWORD GID1, DWORD GID2, int iMaxPlayer, int iMaxScore);
#else
	void	CancelWar(DWORD GID1, DWORD GID2);
#endif
	bool	ChangeMaster(DWORD dwGID, DWORD dwFrom, DWORD dwTo);

private:
	void ParseResult(SQLResult* pRes);

	void RemoveWar(DWORD GID1, DWORD GID2);	// erase war from m_WarMap and set end on priority queue

	void WarEnd(DWORD GID1, DWORD GID2, bool bDraw = false);

	int GetLadderPoint(DWORD GID);

	void GuildWarWin(DWORD GID);
	void GuildWarDraw(DWORD GID);
	void GuildWarLose(DWORD GID);

	void ProcessDraw(DWORD dwGuildID1, DWORD dwGuildID2);
	void ProcessWinLose(DWORD dwGuildWinner, DWORD dwGuildLoser);

	bool IsHalfWinLadderPoint(DWORD dwGuildWinner, DWORD dwGuildLoser);

	std::map<DWORD, TGuild>					m_map_kGuild;
	std::map<DWORD, std::map<DWORD, time_t> >		m_mapGuildWarEndTime;

	std::set<TGuildDeclareInfo>				m_DeclareMap;
	std::map<DWORD, std::map<DWORD, TGuildWarInfo> >	m_WarMap;

	typedef std::pair<time_t, TGuildWarPQElement*>	stPairGuildWar;
	typedef std::pair<time_t, TGuildSkillUsed>	stPairSkillUsed;
	typedef std::pair<time_t, TGuildWaitStartInfo>	stPairWaitStart;

	std::priority_queue<stPairGuildWar, std::vector<stPairGuildWar>, std::greater<stPairGuildWar> >
		m_pqOnWar;
	std::priority_queue<stPairWaitStart, std::vector<stPairWaitStart>, std::greater<stPairWaitStart> >
		m_pqWaitStart;
	std::priority_queue<stPairSkillUsed, std::vector<stPairSkillUsed>, std::greater<stPairSkillUsed> >
		m_pqSkill;

	CPoly							polyPower;
	CPoly							polyHandicap;

	// GID Ranking
	std::map<DWORD, int>					map_kLadderPointRankingByGID;
};