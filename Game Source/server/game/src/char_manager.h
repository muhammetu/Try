#pragma once
#include "../../common/stl.h"
#include "../../common/length.h"

#include "vid.h"
#include "utils.h"
#include "event.h"

class CDungeon;
class CHARACTER;
class CharacterVectorInteractor;

class CHARACTER_MANAGER : public singleton<CHARACTER_MANAGER>
{
public:
	typedef std::unordered_map<std::string, LPCHARACTER> NAME_MAP;

	CHARACTER_MANAGER();
	virtual ~CHARACTER_MANAGER();

	void                    InitRebornTable();
	void                    Destroy();

	void			GracefulShutdown();

	DWORD			AllocVID();

	LPCHARACTER             CreateCharacter(const char* name, DWORD dwPID = 0);
#ifndef DEBUG_ALLOC
	void DestroyCharacter(LPCHARACTER ch);
#else
	void DestroyCharacter(LPCHARACTER ch, const char* file, size_t line);
#endif
#ifdef __RELOAD_REWORK__
	void DestroyCharacterInMap(long lMapIndex);
#endif

	void			Update(int iPulse);
	LPCHARACTER		SpawnMob(DWORD dwVnum, long lMapIndex, long x, long y, long z, bool bSpawnMotion = false, int iRot = -1, bool bShow = true);
	LPCHARACTER		SpawnMobRange(DWORD dwVnum, long lMapIndex, int sx, int sy, int ex, int ey, bool bIsException = false, bool bSpawnMotion = false, bool bAggressive = false);
	LPCHARACTER		SpawnGroup(DWORD dwVnum, long lMapIndex, int sx, int sy, int ex, int ey, LPREGEN pkRegen = NULL, bool bAggressive_ = false, LPDUNGEON pDungeon = NULL);
	bool			SpawnGroupGroup(DWORD dwVnum, long lMapIndex, int sx, int sy, int ex, int ey, LPREGEN pkRegen = NULL, bool bAggressive_ = false, LPDUNGEON pDungeon = NULL);

	bool			SpawnMoveGroup(DWORD dwVnum, long lMapIndex, int sx, int sy, int ex, int ey, int tx, int ty, LPREGEN pkRegen = NULL, bool bAggressive_ = false);
	LPCHARACTER		SpawnMobRandomPosition(DWORD dwVnum, long lMapIndex);

	void			SelectStone(LPCHARACTER pkChrStone);

	NAME_MAP& GetPCMap() { return m_map_pkPCChr; }

	LPCHARACTER		Find(DWORD dwVID);
	LPCHARACTER		Find(const VID& vid);
	LPCHARACTER		FindPC(const char* name);
	LPCHARACTER		FindByPID(DWORD dwPID);

	bool			AddToStateList(LPCHARACTER ch);
	void			RemoveFromStateList(LPCHARACTER ch);

	void                    DelayedSave(LPCHARACTER ch);
	bool                    FlushDelayedSave(LPCHARACTER ch);
	void			ProcessDelayedSave();

	template<class Func>	Func for_each_pc(Func f);

	void			RegisterRaceNum(DWORD dwVnum);
	void			RegisterRaceNumMap(LPCHARACTER ch);
	void			UnregisterRaceNumMap(LPCHARACTER ch);
	bool			GetCharactersByRaceNum(DWORD dwRaceNum, CharacterVectorInteractor& i);

	LPCHARACTER		FindSpecifyPC(unsigned int uiJobFlag, long lMapIndex, LPCHARACTER except = NULL, int iMinLevel = 1, int iMaxLevel = PLAYER_MAX_LEVEL_CONST);

	void			SetMobItemRate(int value) { m_iMobItemRate = value; }
	void			SetMobDamageRate(int value) { m_iMobDamageRate = value; }
	void			SetMobGoldAmountRate(int value) { m_iMobGoldAmountRate = value; }
	void			SetMobGoldDropRate(int value) { m_iMobGoldDropRate = value; }
	void			SetMobExpRate(int value) { m_iMobExpRate = value; }

	void			SetMobItemRatePremium(int value) { m_iMobItemRatePremium = value; }
	void			SetMobGoldAmountRatePremium(int value) { m_iMobGoldAmountRatePremium = value; }
	void			SetMobGoldDropRatePremium(int value) { m_iMobGoldDropRatePremium = value; }
	void			SetMobExpRatePremium(int value) { m_iMobExpRatePremium = value; }

	void			SetUserDamageRatePremium(int value) { m_iUserDamageRatePremium = value; }
	void			SetUserDamageRate(int value) { m_iUserDamageRate = value; }
	int			GetMobItemRate(LPCHARACTER ch);
	int			GetMobDamageRate(LPCHARACTER ch);
	int			GetMobGoldAmountRate(LPCHARACTER ch);
	int			GetMobGoldDropRate(LPCHARACTER ch);
	int			GetMobExpRate(LPCHARACTER ch);

#ifdef __DAMAGE_LIMIT_REWORK__
	long long	GetUserDamageRate(LPCHARACTER ch);
#else
	int			GetUserDamageRate(LPCHARACTER ch);
#endif
	void		SendScriptToMap(long lMapIndex, const std::string& s);

	bool			BeginPendingDestroy();
	void			FlushPendingDestroy();

private:
	int					m_iMobItemRate;
	int					m_iMobDamageRate;
	int					m_iMobGoldAmountRate;
	int					m_iMobGoldDropRate;
	int					m_iMobExpRate;

	int					m_iMobItemRatePremium;
	int					m_iMobGoldAmountRatePremium;
	int					m_iMobGoldDropRatePremium;
	int					m_iMobExpRatePremium;
#ifdef __DAMAGE_LIMIT_REWORK__
	long long					m_iUserDamageRate;
#else
	int					m_iUserDamageRate;
#endif
	int					m_iUserDamageRatePremium;
	int					m_iVIDCount;

	std::unordered_map<DWORD, LPCHARACTER> m_map_pkChrByVID;
	std::unordered_map<DWORD, LPCHARACTER> m_map_pkChrByPID;
	NAME_MAP			m_map_pkPCChr;

	char				dummy1[1024];	// memory barrier
	CHARACTER_SET		m_set_pkChrState;
	CHARACTER_SET		m_set_pkChrForDelayedSave;

	LPCHARACTER			m_pkChrSelectedStone;

	std::set<DWORD>		m_set_dwRegisteredRaceNum;
	std::map<DWORD, CHARACTER_SET> m_map_pkChrByRaceNum;

	bool				m_bUsePendingDestroy;
	CHARACTER_SET		m_set_pkChrPendingDestroy;

#ifdef ENABLE_MULTI_FARM_BLOCK
public:
	int GetMultiFarmCount(const char* playerIP, std::map<uint32_t, std::pair<std::string, bool>>& m_mapNames);
	void CheckMultiFarmAccount(const char* szIP, const uint32_t playerID, const char* playerName, const bool bStatus, uint8_t affectType = 0, int affectDuration = 0, bool isP2P = false);
	void SetMultiFarm(const char* szIP, const uint32_t playerID, const char* playerName, const bool bStatus, const uint8_t affectType, const int affectTime);
	void RemoveMultiFarm(const char* szIP, const uint32_t playerID, const bool isP2P);
	void CheckMultiFarmAccounts(const char* szIP);

protected:
	std::map<std::string, std::vector<TMultiFarm>> m_mapmultiFarm;

protected:
	std::map<std::pair<std::string,std::string>, DWORD> m_mapDungeonTimes;
public:
	void SetDungeonTime(const std::string& ip, const std::string& questName, DWORD value, bool sendP2P = true);
	DWORD GetDungeonTime(const std::string& ip, const std::string& questName);
#endif

};

template<class Func>
Func CHARACTER_MANAGER::for_each_pc(Func f)
{
	std::unordered_map<DWORD, LPCHARACTER>::iterator it;

	for (it = m_map_pkChrByPID.begin(); it != m_map_pkChrByPID.end(); ++it)
		f(it->second);

	return f;
}

class CharacterVectorInteractor : public CHARACTER_VECTOR
{
public:
	CharacterVectorInteractor() : m_bMyBegin(false) { }

	CharacterVectorInteractor(const CHARACTER_SET& r);
	virtual ~CharacterVectorInteractor();

private:
	bool m_bMyBegin;
};

#ifndef DEBUG_ALLOC
#define M2_DESTROY_CHARACTER(ptr) CHARACTER_MANAGER::instance().DestroyCharacter(ptr)
#else
#define M2_DESTROY_CHARACTER(ptr) CHARACTER_MANAGER::instance().DestroyCharacter(ptr, __FILE__, __LINE__)
#endif