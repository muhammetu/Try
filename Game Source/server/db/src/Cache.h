#pragma once

#include "../../common/cache.h"

class CItemCache : public cache<TPlayerItem>
{
public:
	CItemCache();
	virtual ~CItemCache();

	void Delete();
	virtual void OnFlush();
};

class CQuestCache : public cache<TQuestTable>
{
public:
	CQuestCache();
	virtual ~CQuestCache();

	void Delete();
	virtual void OnFlush();
};

class CAffectCache : public cache<TPacketGDAddAffect>
{
public:
	CAffectCache();
	virtual ~CAffectCache();

	void Delete();
	virtual void OnFlush();
};

class CPlayerTableCache : public cache<TPlayerTable>
{
public:
	CPlayerTableCache();
	virtual ~CPlayerTableCache();

	virtual void OnFlush();

	DWORD GetLastUpdateTime() { return m_lastUpdateTime; }
};

#ifdef __SKILL_COLOR__
class CSKillColorCache : public cache<TSkillColor>
{
public:
	CSKillColorCache();
	virtual ~CSKillColorCache();

	virtual void OnFlush();
};
#endif

#ifdef __BATTLE_PASS_SYSTEM__
class CBattlePassCache : public cache<TPlayerBattlePassMission>
{
public:
	CBattlePassCache();
	virtual ~CBattlePassCache();

	void Delete();
	virtual void OnFlush();
};
#endif // __BATTLE_PASS_SYSTEM__
