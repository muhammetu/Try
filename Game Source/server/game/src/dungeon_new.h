#pragma once

#include <chrono>
#include "sectree_manager.h"
#include <boost/unordered_map.hpp>
#include "config.h"

class CDungeon
{
public:
	typedef DWORD								IdType;
	typedef std::map<std::string, int>			TFlagMap;
	typedef std::map<std::string, LPCHARACTER>	TUniqueMobMap;

private:
	IdType 		m_id; // <Factor>
public:
	~CDungeon();
	IdType GetId() const { return m_id; }

private:
	DWORD		m_lOrigMapIndex;
	DWORD		m_lMapIndex;
public:
	long		GetMapIndex() { return m_lMapIndex; }
	long 		GetOrigMapIndex() { return m_lOrigMapIndex; }

private:
	TFlagMap	m_map_Flag;
public:
	int			GetFlag(std::string name);
	void		SetFlag(std::string name, int value);
	long		GetRoomNo() { return m_lMapIndex - (m_lOrigMapIndex * 10000); }
	long		GetRoomPass() { return GetFlag("password"); }

protected:
	std::unordered_set<DWORD> m_registeredItems;
public:
	bool IsRegisteredItem(DWORD id);
	void RegisterItem(DWORD id);

public:
	void CheckEliminated();

protected:
	int		m_iMonsterCount;
public:
	void	IncMonster() { m_iMonsterCount++; }
	void	DecMonster() { m_iMonsterCount--; CheckEliminated(); }
	int		CountMonster() { return m_iMonsterCount; }
	int		CountRealMonster(DWORD vnum = 0);

protected:
	int		m_iNpcCount;
public:
	void	IncNpc() { m_iNpcCount++; }
	void	DecNpc() { m_iNpcCount--; CheckEliminated(); }
	int		CountNpc() { return m_iNpcCount; }

protected:
	std::unordered_set<DWORD> m_rewardPlayers;
public:
	bool IsRewardUser(DWORD pid) { return m_rewardPlayers.end() != std::find(m_rewardPlayers.begin(), m_rewardPlayers.end(), pid); }
	void RewardUser(DWORD pid) { m_rewardPlayers.insert(pid); }
	void UnRewardUser(DWORD pid) { m_rewardPlayers.erase(pid); }

protected:
	std::unordered_set<DWORD> m_registeredPlayers;
public:
	bool IsRegisteredUser(DWORD pid) { return m_registeredPlayers.end() != std::find(m_registeredPlayers.begin(), m_registeredPlayers.end(), pid); }
	void RegisterUser(LPCHARACTER ch);
	void RegisterUser(DWORD pid) { m_registeredPlayers.insert(pid); }

protected:
	long		m_lJoinX;
	long		m_lJoinY;
public:
	void	SetJoinCoordinate(long x, long y) { m_lJoinX = x; m_lJoinY = y; }
	int		GetJoinX() { return m_lJoinX; };
	int		GetJoinY() { return m_lJoinY; };

private:
	TUniqueMobMap	m_map_UniqueMob;
public:
	void	SetUnique(const char* key, DWORD vid);
	void	KillUnique(const std::string& key);
	void	PurgeUnique(const std::string& key);
	bool	IsUniqueDead(const std::string& key);
	float	GetUniqueHpPerc(const std::string& key);
	DWORD	GetUniqueVid(const std::string& key);
	void	DeadCharacter(LPCHARACTER ch);
	void	UniqueSetMaxHP(const std::string& key, HPTYPE iMaxHP);
	void	UniqueSetHP(const std::string& key, HPTYPE iHP);
	void	UniqueSetDefGrade(const std::string& key, int iGrade);

	std::string mission;
	std::string submission;
	void	Notice(const char* msg);
	void	Big_Notice(const char* msg);
	void	cmdchat(const char* msg);
	void	SetMission(const char* msg);
	void	SetSubMission(const char* msg);

	void	Join(LPCHARACTER ch);

	void		Purge(LPCHARACTER expect);
	void		TransferAllMonsters(LPCHARACTER ch);
	LPCHARACTER	GetRandomMember();
	void		PurgeMob(LPCHARACTER ExceptChar);
	void		KillAllMob();
	void		KillAll();

	void	IncKillCount(LPCHARACTER pkKiller, LPCHARACTER pkVictim);
	bool	IsUsePotion();
	bool	IsUseRevive();
	void	UsePotion(LPCHARACTER ch);
	void	UseRevive(LPCHARACTER ch);

	LPCHARACTER	SpawnMob(DWORD vnum, int x, int y, int dir = 0);
	LPCHARACTER	SpawnMob_ac_dir(DWORD vnum, int x, int y, int dir = 0);
	LPCHARACTER	SpawnGroup(DWORD vnum, long x, long y, float radius, bool bAggressive = false, int count = 1);

	void	SpawnNameMob(DWORD vnum, int x, int y, const char* name);
	void	SpawnGotoMob(long lFromX, long lFromY, long lToX, long lToY);

	void	SpawnRegen(const char* filename, bool bOnce = true);
	void	AddRegen(LPREGEN regen);
	void	ClearRegen();
	bool	IsValidRegen(LPREGEN regen, size_t regen_id);

	void	JumpAll(long lFromMapIndex, int x, int y);
	void	WarpAll(long lFromMapIndex, int x, int y);
	void	ExitAll();
	void	ExitAllToStartPosition();
	void	ExitAllWithDelay(int delay);
	int		GetExitRemainTime();

	template <class Func> Func ForEachMember(Func f);

	bool IsAllPCNearTo(int x, int y, int dist);

protected:
	CDungeon(IdType id, long lOriginalMapIndex, long lMapIndex);

	void	Initialize();

private:
	bool		m_bUsePotion;
	bool		m_bUseRevive;

	std::string	m_stRegenFile;

	std::vector<LPREGEN> m_regen;

	LPEVENT		deadEvent;

	LPEVENT exit_all_event_;
	size_t regen_id_;

	friend class CDungeonManager;
	friend EVENTFUNC(dungeon_dead_event);
	friend EVENTFUNC(dungeon_exit_all_event);

public:
	bool IsEliminated();

public:
	long long unsigned SaveReport(LPCHARACTER ch, char* fileName);

#ifdef __DUNGEON_TEST_MODE__
protected:
	bool isTest;
public:
	void SetTest(bool is) { isTest = is; }
	bool IsTest() { return isTest; }
#endif // __DUNGEON_TEST_MODE__
};

class CDungeonManager : public singleton<CDungeonManager>
{
	typedef std::map<CDungeon::IdType, LPDUNGEON> TDungeonMap;
	typedef std::map<long, LPDUNGEON> TMapDungeon;

public:
	CDungeonManager();
	virtual ~CDungeonManager();

	friend class CDungeon;

	LPDUNGEON	Create(long lOriginalMapIndex);
	void		Destroy(CDungeon::IdType dungeon_id);
	LPDUNGEON	Find(CDungeon::IdType dungeon_id);
	LPDUNGEON	FindByMapIndex(long lMapIndex);
	const TMapDungeon& GetDungeonMap();
	void FindPcRegisteredDungeon(DWORD pid, std::set<LPDUNGEON>& outSet);
	LPDUNGEON FindLastRegisteredDungeon(DWORD pid, long mapIndex);
	void	OnShutdown();
private:
	TDungeonMap	m_map_pkDungeon;
	TMapDungeon m_map_pkMapDungeon;

	// <Factor> Introduced unsigned 32-bit dungeon identifier
	CDungeon::IdType next_id_;
};