#pragma once
#include "../../common/service.h"
#ifdef __GROWTH_MOUNT_SYSTEM__
class CHARACTER;

/**
*/
class CNewMountActor //: public CHARACTER
{
public:
	enum ENewMountOptions
	{
		EMountOption_Followable = 1 << 0,
		EMountOption_Mountable = 1 << 1,
		EMountOption_Summonable = 1 << 2,
		EMountOption_Combatable = 1 << 3,
	};

protected:
	friend class CNewMountSystem;

	CNewMountActor(LPCHARACTER owner, DWORD vnum, DWORD options = EMountOption_Followable | EMountOption_Summonable);
	virtual ~CNewMountActor();
	virtual bool	Update(DWORD deltaTime);

protected:
	virtual bool	_UpdateFollowAI();
	virtual bool	_UpdatAloneActionAI(float fMinDist, float fMaxDist);

private:
	bool Follow(float fMinDistance = 50.f);

public:
	LPCHARACTER		GetCharacter()	const { return m_pkChar; }
	LPCHARACTER		GetOwner()	const { return m_pkOwner; }
	DWORD			GetVID() const { return m_dwVID; }
	DWORD			GetVnum() const { return m_dwVnum; }

	bool			HasOption(ENewMountOptions option) const { return m_dwOptions & option; }

	void			SetName(const char* mountName);
	void			SetLevel(DWORD level);

	bool			Mount();
	void			Unmount();

	DWORD			Summon(const char* mountName, LPITEM pSummonItem, bool bSpawnFar = false);
	void			Unsummon();

	int				GetSkillCount();

	bool			IsSummoned() const { return 0 != m_pkChar; }
	void			SetSummonItem(LPITEM pItem);
	DWORD			GetSummonItemVID() { return m_dwSummonItemVID; }
	DWORD			GetSummonItemID() { return m_dwSummonItemID; }
	DWORD			GetEvolution() { return m_dwevolution; }
	DWORD			GetLevel() { return m_dwlevel; }
	void			SetEvolution(int lv);
	void			SetArtis(int val);
	bool			SetExp(DWORD exp, int mode);
	DWORD			GetExp() { return m_dwexp; }
	DWORD			GetExpI() { return m_dwexpitem; }
	DWORD			GetAge() { return m_dwAgeDuration; }
	void			SetNextExp(int nextExp);
	DWORD			GetNextExpFromMob() { return m_dwExpFromMob; }
	DWORD			GetNextExpFromItem() { return m_dwExpFromItem; }
	int				GetLevelStep() { return m_dwlevelstep; }
	int				GetMountType() { return m_dwMountType; }
	void			SetMountType(int mounttype);

	void			IncreaseMountBonus();
	void			SetItemCube(int pos, int invpos);
	void			ItemCubeFeed(int type);
	void			DoMountSkill(int skillslot);
	void			UpdateTime();

	bool			ResetSkill(int skill);
	bool			IncreaseMountSkill(int skill);
	bool			IncreaseMountEvolution();

	void			GiveBuff();
	void			ClearBuff();

private:
	int			m_dwlevelstep; //Step livello del mount da da 0 a 4
	int			m_dwExpFromMob; //Exp richiesta per il level 90% del tot
	int			m_dwExpFromItem; //Exp richiesta per il level 10% del tot
	int			m_dwexpitem; // Exp corrente presa dagli item
	int			m_dwevolution; //Stato evoluzione del mount da 1 a 4
	int			m_dwTimeMount; //Tempo per il mount
	int			m_dwslotimm;
	int			m_dwMountType;

	DWORD		m_dwImmTime;

	int				m_dwmountslotitem[25];
	int				m_dwskill[3];
	int				m_dwskillslot[3];
	int				m_dwbonusmount[3][2];
	DWORD			m_dwVnum;
	DWORD			m_dwVID;
	DWORD			m_dwlevel;
	DWORD			m_dwexp;
	DWORD			m_dwOptions;
	DWORD			m_dwLastActionTime;
	DWORD			m_dwSummonItemVID;
	DWORD			m_dwSummonItemID;
	DWORD			m_dwSummonItemVnum;

	DWORD			m_dwduration;
	DWORD			m_dwtduration;
	DWORD			m_dwAgeDuration;

	DWORD			m_dwArtis;

	short			m_originalMoveSpeed;

	std::string		m_name;

	LPCHARACTER		m_pkChar;					// Instance of mount(CHARACTER)
	LPCHARACTER		m_pkOwner;
};

/**
*/
class CNewMountSystem
{
public:
	typedef	boost::unordered_map<DWORD, CNewMountActor*>		TNewMountActorMap;

public:
	CNewMountSystem(LPCHARACTER owner);
	virtual ~CNewMountSystem();

	CNewMountActor* GetByVID(DWORD vid) const;
	CNewMountActor* GetByVnum(DWORD vnum) const;

	bool		Update(DWORD deltaTime);
	void		Destroy();

	size_t		CountSummoned() const;

public:
	CNewMountActor* Summon(DWORD mobVnum, LPITEM pSummonItem, const char* mountName, bool bSpawnFar, DWORD options = CNewMountActor::EMountOption_Followable | CNewMountActor::EMountOption_Summonable);

	void		Unsummon(DWORD mobVnum, bool bDeleteFromList = false);

	bool		ResetSkill(int skill);
	bool		IncreaseMountSkill(int skill);
	bool		IncreaseMountEvolution();

	void		DeleteMount(DWORD mobVnum);
	void		DeleteMount(CNewMountActor* mountActor);
	void		RefreshBuff();
	bool		IsActiveMount();
	DWORD		GetNewMountITemID();
	bool		SetExp(int iExp, int mode);
	void		SetMountType(int type);
	void		SetArtis(int val);
	int			GetEvolution();
	int			GetLevel();
	int			GetExp();
	int			GetAge();
	int			GetLevelStep();
	int			GetMountType();
	void		SetItemCube(int pos, int invpos);
	void		ItemCubeFeed(int type);
	void		DoMountSkill(int skillslot);
	void		UpdateTime();

	CNewMountActor* GetSummoned();

private:
	TNewMountActorMap	m_mountActorMap;
	LPCHARACTER		m_pkOwner;
	DWORD			m_dwUpdatePeriod;
	DWORD			m_dwLastUpdateTime;
	LPEVENT			m_pkNewMountSystemUpdateEvent;
	LPEVENT			m_pkNewMountSystemExpireEvent;
};
#endif
