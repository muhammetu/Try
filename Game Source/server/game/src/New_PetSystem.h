#pragma once
#include "../../common/service.h"
#ifdef __GROWTH_PET_SYSTEM__
class CHARACTER;

struct SNewPetAbility
{
};

/**
*/
class CNewPetActor //: public CHARACTER
{
public:
	enum ENewPetOptions
	{
		EPetOption_Followable = 1 << 0,
		EPetOption_Mountable = 1 << 1,
		EPetOption_Summonable = 1 << 2,
		EPetOption_Combatable = 1 << 3,
	};

protected:
	friend class CNewPetSystem;

	CNewPetActor(LPCHARACTER owner, DWORD vnum, DWORD options = EPetOption_Followable | EPetOption_Summonable);
	//	CPetActor(LPCHARACTER owner, DWORD vnum, const SPetAbility& petAbility, DWORD options = EPetOption_Followable | EPetOption_Summonable);

	virtual ~CNewPetActor();

	virtual bool	Update(DWORD deltaTime);

protected:
	virtual bool	_UpdateFollowAI();
	virtual bool	_UpdatAloneActionAI(float fMinDist, float fMaxDist);

	/// @TODO
	//virtual bool	_UpdateCombatAI();

private:
	bool Follow(float fMinDistance = 50.f);

public:
	LPCHARACTER		GetCharacter()	const { return m_pkChar; }
	LPCHARACTER		GetOwner()	const { return m_pkOwner; }
	DWORD			GetVID() const { return m_dwVID; }
	DWORD			GetVnum() const { return m_dwVnum; }

	bool			HasOption(ENewPetOptions option) const { return m_dwOptions & option; }

	void			SetName(const char* petName);
	void			SetLevel(DWORD level);

	bool			Mount();
	void			Unmount();

	DWORD			Summon(const char* petName, LPITEM pSummonItem, bool bSpawnFar = false);
	void			Unsummon();

	int             GetSkillCount();

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
	int				GetPetType() { return m_dwPetType; }
	void			SetPetType(int pettype);

	void			IncreasePetBonus();
	void			SetItemCube(int pos, int invpos);
	void			ItemCubeFeed(int type);
	void			DoPetSkill(int skillslot);
	void			UpdateTime();

	bool			ResetSkill(int skill);
	bool			IncreasePetSkill(int skill);
	bool			IncreasePetEvolution();

	void			GiveBuff();
	void			ClearBuff();

private:
	int			m_dwlevelstep; //Step livello del pet da da 0 a 4
	int			m_dwExpFromMob; //Exp richiesta per il level 90% del tot
	int			m_dwExpFromItem; //Exp richiesta per il level 10% del tot
	int			m_dwexpitem; // Exp corrente presa dagli item
	int			m_dwevolution; //Stato evoluzione del pet da 1 a 4
	int			m_dwTimePet; //Tempo per il pet
	int			m_dwslotimm;
	int			m_dwPetType;

	DWORD		m_dwImmTime;

	int				m_dwpetslotitem[25];
	DWORD			m_dwskill[3];
	int				m_dwskillslot[3];
	int				m_dwbonuspet[3][2];
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

	LPCHARACTER		m_pkChar;					// Instance of pet(CHARACTER)
	LPCHARACTER		m_pkOwner;
};

/**
*/
class CNewPetSystem
{
public:
	typedef	boost::unordered_map<DWORD, CNewPetActor*>		TNewPetActorMap;

public:
	CNewPetSystem(LPCHARACTER owner);
	virtual ~CNewPetSystem();

	CNewPetActor* GetByVID(DWORD vid) const;
	CNewPetActor* GetByVnum(DWORD vnum) const;

	bool		Update(DWORD deltaTime);
	void		Destroy();

	size_t		CountSummoned() const;

public:
	CNewPetActor* Summon(DWORD mobVnum, LPITEM pSummonItem, const char* petName, bool bSpawnFar, DWORD options = CNewPetActor::EPetOption_Followable | CNewPetActor::EPetOption_Summonable);

	void		Unsummon(DWORD mobVnum, bool bDeleteFromList = false);

	bool		ResetSkill(int skill);
	bool		IncreasePetSkill(int skill);
	bool		IncreasePetEvolution();

	void		DeletePet(DWORD mobVnum);
	void		DeletePet(CNewPetActor* petActor);
	void		RefreshBuff();
	bool		IsActivePet();
	DWORD		GetNewPetITemID();
	bool		SetExp(int iExp, int mode);
	void		SetPetType(int type);
	void		SetArtis(int val);
	int			GetEvolution();
	int			GetLevel();
	int			GetExp();
	int			GetAge();
	int			GetLevelStep();
	int			GetPetType();
	void		SetItemCube(int pos, int invpos);
	void		ItemCubeFeed(int type);
	void		DoPetSkill(int skillslot);
	void		UpdateTime();

	CNewPetActor* GetSummoned();

private:
	TNewPetActorMap	m_petActorMap;
	LPCHARACTER		m_pkOwner;
	DWORD			m_dwUpdatePeriod;
	DWORD			m_dwLastUpdateTime;
	LPEVENT			m_pkNewPetSystemUpdateEvent;
	LPEVENT			m_pkNewPetSystemExpireEvent;
};

/**
// Summon Pet
CPetSystem* petSystem = mainChar->GetPetSystem();
CPetActor* petActor = petSystem->Summon(~~~);

DWORD petVID = petActor->GetVID();
if (0 == petActor)
{
	ERROR_LOG(...)
};

// Unsummon Pet
petSystem->Unsummon(petVID);

// Mount Pet
petActor->Mount()..

CPetActor::Update(...)
{
	// AI : Follow, actions, etc...
}

*/
#endif