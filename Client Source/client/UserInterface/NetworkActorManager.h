#pragma once
#include "Packet.h"
#include "InstanceBase.h"

struct SNetworkActorData
{
	std::string				m_stName;
	CAffectFlagContainer	m_kAffectFlags;
	BYTE					m_bType;
	DWORD					m_dwVID;
	DWORD					m_dwStateFlags;
	BYTE					m_bEmpireID;
	DWORD					m_dwRace;
	WORD					m_wMovSpd;
	WORD					m_wAtkSpd;
	FLOAT					m_fRot;
	LONG					m_lCurX;
	LONG					m_lCurY;
	LONG					m_lSrcX;
	LONG					m_lSrcY;
	LONG					m_lDstX;
	LONG					m_lDstY;
	DWORD					m_dwServerSrcTime;
	DWORD					m_dwClientSrcTime;
	DWORD					m_dwDuration;
	DWORD					m_dwArmor;
	DWORD					m_dwWeapon;
	DWORD					m_dwHair;
#ifdef ENABLE_ACCE_SYSTEM
	DWORD					m_dwAcce;
	DWORD					m_dwAcceEffect;
#endif // ENABLE_ACCE_SYSTEM
#ifdef ENABLE_AURA_COSTUME_SYSTEM
	DWORD					m_dwAura;
#endif // ENABLE_AURA_COSTUME_SYSTEM
#ifdef ENABLE_QUIVER_SYSTEM
	DWORD					m_dwQuiver;
#endif // ENABLE_QUIVER_SYSTEM
	DWORD					m_dwOwnerVID;
	BYTE					m_byPKMode;
	DWORD					m_dwMountVnum;
	DWORD					m_dwGuildID;
	BYTE					m_bLevel;
#ifdef ENABLE_MOB_AGGR_LVL_INFO
	DWORD					m_dwAIFlag;
#endif
#ifdef ENABLE_ALIGNMENT_SYSTEM
	int						m_alignment;
#else
	short					m_alignment;
#endif
#ifdef ENABLE_RANK_SYSTEM
	short 					m_sRank;
#endif
#ifdef ENABLE_LANDRANK_SYSTEM
	short 					m_sLandRank;
#endif
#ifdef ENABLE_REBORN_SYSTEM
	short 					m_sReborn;
#endif
#ifdef ENABLE_TEAM_SYSTEM
	short 					m_sTeam;
#endif
#ifdef ENABLE_MONIKER_SYSTEM
	std::string				m_Moniker;
#endif
#ifdef ENABLE_LOVE_SYSTEM
	std::string				m_Love1;
	std::string				m_Love2;
#endif // ENABLE_LOVE_SYSTEM
#ifdef ENABLE_WORD_SYSTEM
	std::string				m_word;
#endif // ENABLE_WORD_SYSTEM

#ifdef ENABLE_GUILD_LEADER_SYSTEM
	BYTE					m_bMemberType;
#endif
#ifdef ENABLE_ITEM_EVOLUTION_SYSTEM
	WORD					m_wArmorEvolution;
	WORD					m_wWeaponEvolution;
#endif
#ifdef ENABLE_SKILL_COLOR_SYSTEM
	DWORD					m_dwSkillColor[ESkillColorLength::MAX_SKILL_COUNT + MAX_BUFF_COUNT][ESkillColorLength::MAX_EFFECT_COUNT];
#endif
#ifdef ENABLE_SHINING_ITEM_SYSTEM
	DWORD					m_dwShining[CHR_SHINING_NUM];
#endif
#ifdef ENABLE_AUTO_HUNT_SYSTEM
	DWORD					m_hasAttacker;
#endif // ENABLE_AUTO_HUNT_SYSTEM

	SNetworkActorData();

	void SetDstPosition(DWORD dwServerTime, LONG lDstX, LONG lDstY, DWORD dwDuration);
	void SetPosition(LONG lPosX, LONG lPosY);
	void UpdatePosition();

	// NETWORK_ACTOR_DATA_COPY
	SNetworkActorData(const SNetworkActorData& src);
	void operator=(const SNetworkActorData& src);
	void __copy__(const SNetworkActorData& src);
	// END_OF_NETWORK_ACTOR_DATA_COPY
};

struct SNetworkMoveActorData
{
	DWORD	m_dwVID;
	DWORD	m_dwTime;
	LONG	m_lPosX;
	LONG	m_lPosY;
	float	m_fRot;
	DWORD	m_dwFunc;
	DWORD	m_dwArg;
	DWORD	m_dwDuration;

	SNetworkMoveActorData()
	{
		m_dwVID = 0;
		m_dwTime = 0;
		m_fRot = 0.0f;
		m_lPosX = 0;
		m_lPosY = 0;
		m_dwFunc = 0;
		m_dwArg = 0;
		m_dwDuration = 0;
	}
};

struct SNetworkUpdateActorData
{
	DWORD					m_dwVID;
	DWORD					m_dwGuildID;
	DWORD					m_dwArmor;
	DWORD					m_dwWeapon;
	DWORD					m_dwHair;
#ifdef ENABLE_ACCE_SYSTEM
	DWORD					m_dwAcce;
	DWORD					m_dwAcceEffect;
#endif // ENABLE_ACCE_SYSTEM
#ifdef ENABLE_AURA_COSTUME_SYSTEM
	DWORD					m_dwAura;
#endif // ENABLE_AURA_COSTUME_SYSTEM
#ifdef ENABLE_QUIVER_SYSTEM
	DWORD					m_dwQuiver;
#endif // ENABLE_QUIVER_SYSTEM
	WORD					m_wMovSpd;
	WORD					m_wAtkSpd;
	BYTE					m_bLevel;
	BYTE					m_byPKMode;
	DWORD					m_dwMountVnum;
	DWORD					m_dwStateFlags;
#ifdef ENABLE_ALIGNMENT_SYSTEM
	int						m_alignment;
#else
	short					m_alignment;
#endif
#ifdef ENABLE_RANK_SYSTEM
	short 					m_sRank;
#endif
#ifdef ENABLE_LANDRANK_SYSTEM
	short 					m_sLandRank;
#endif
#ifdef ENABLE_REBORN_SYSTEM
	short 					m_sReborn;
#endif
#ifdef ENABLE_TEAM_SYSTEM
	short 					m_sTeam;
#endif
#ifdef ENABLE_MONIKER_SYSTEM
	std::string				m_Moniker;
#endif
#ifdef ENABLE_LOVE_SYSTEM
	std::string				m_Love1;
	std::string				m_Love2;
#endif // ENABLE_LOVE_SYSTEM
#ifdef ENABLE_WORD_SYSTEM
	std::string				m_word;
#endif // ENABLE_WORD_SYSTEM

#ifdef ENABLE_GUILD_LEADER_SYSTEM
	BYTE					m_bMemberType;
#endif
#ifdef ENABLE_ITEM_EVOLUTION_SYSTEM
	WORD					m_wArmorEvolution;
	WORD					m_wWeaponEvolution;
#endif
#ifdef ENABLE_SKILL_COLOR_SYSTEM
	DWORD					m_dwSkillColor[ESkillColorLength::MAX_SKILL_COUNT + MAX_BUFF_COUNT][ESkillColorLength::MAX_EFFECT_COUNT];
#endif
#ifdef ENABLE_SHINING_ITEM_SYSTEM
	DWORD					m_dwShining[CHR_SHINING_NUM];
#endif
#ifdef ENABLE_AUTO_HUNT_SYSTEM
	DWORD					m_hasAttacker;
#endif // ENABLE_AUTO_HUNT_SYSTEM

	CAffectFlagContainer	m_kAffectFlags;

	SNetworkUpdateActorData()
	{
		m_dwVID = 0;
		m_dwGuildID = 0;
		m_dwArmor = 0;
		m_dwWeapon = 0;
		m_dwHair = 0;
#ifdef ENABLE_ACCE_SYSTEM
		m_dwAcce = 0;
		m_dwAcceEffect = 0;
#endif // ENABLE_ACCE_SYSTEM
#ifdef ENABLE_AURA_COSTUME_SYSTEM
		m_dwAura = 0;
#endif // ENABLE_AURA_COSTUME_SYSTEM
#ifdef ENABLE_QUIVER_SYSTEM
		m_dwQuiver = 0;
#endif // ENABLE_QUIVER_SYSTEM
		m_wMovSpd = 0;
		m_wAtkSpd = 0;
		m_bLevel = 0;
		m_byPKMode = 0;
		m_dwMountVnum = 0;
		m_dwStateFlags = 0;
#ifdef ENABLE_ALIGNMENT_SYSTEM
		m_alignment = 0;
#endif
#ifdef ENABLE_RANK_SYSTEM
		m_sRank = 0;
#endif
#ifdef ENABLE_LANDRANK_SYSTEM
		m_sLandRank = 0;
#endif
#ifdef ENABLE_REBORN_SYSTEM
		m_sReborn = 0;
#endif
#ifdef ENABLE_TEAM_SYSTEM
		m_sTeam = 0;
#endif
#ifdef ENABLE_MONIKER_SYSTEM
		m_Moniker = "";
#endif
#ifdef ENABLE_LOVE_SYSTEM
		m_Love1 = "";
		m_Love2 = "";
#endif // ENABLE_LOVE_SYSTEM
#ifdef ENABLE_WORD_SYSTEM
		m_word = "";
#endif // ENABLE_WORD_SYSTEM

#ifdef ENABLE_GUILD_LEADER_SYSTEM
		m_bMemberType = 0;
#endif
#ifdef ENABLE_ITEM_EVOLUTION_SYSTEM
		m_wArmorEvolution = 0;
		m_wWeaponEvolution = 0;
#endif
#ifdef ENABLE_SKILL_COLOR_SYSTEM
		memset(m_dwSkillColor, 0, sizeof(m_dwSkillColor));
#endif
#ifdef ENABLE_SHINING_ITEM_SYSTEM
		memset(m_dwShining, 0, sizeof(m_dwShining));
#endif
#ifdef ENABLE_AUTO_HUNT_SYSTEM
		m_hasAttacker = 0;
#endif // ENABLE_AUTO_HUNT_SYSTEM
		m_kAffectFlags.Clear();
	}
};

class CPythonCharacterManager;

class CNetworkActorManager : public CReferenceObject
{
public:
	CNetworkActorManager();
	virtual ~CNetworkActorManager();

	void Destroy();

	void SetMainActorVID(DWORD dwVID);

	void RemoveActor(DWORD dwVID);
	void AppendActor(const SNetworkActorData& c_rkNetActorData);
	void UpdateActor(const SNetworkUpdateActorData& c_rkNetUpdateActorData);
	void MoveActor(const SNetworkMoveActorData& c_rkNetMoveActorData);

	void SyncActor(DWORD dwVID, LONG lPosX, LONG lPosY);
	void SetActorOwner(DWORD dwOwnerVID, DWORD dwVictimVID);

	void Update();

protected:
	void __OLD_Update();

	void __UpdateMainActor();

	bool __IsVisiblePos(LONG lPosX, LONG lPosY);
	bool __IsVisibleActor(const SNetworkActorData& c_rkNetActorData);
	bool __IsMainActorVID(DWORD dwVID);

	void __RemoveAllGroundItems();
	void __RemoveAllActors();
	void __RemoveDynamicActors();
	void __RemoveCharacterManagerActor(SNetworkActorData& rkNetActorData);

	SNetworkActorData* __FindActorData(DWORD dwVID);
	CInstanceBase* __AppendCharacterManagerActor(SNetworkActorData& rkNetActorData);
	CInstanceBase* __FindActor(SNetworkActorData& rkNetActorData);
	CInstanceBase* __FindActor(SNetworkActorData& rkNetActorData, LONG lDstX, LONG lDstY);

	CPythonCharacterManager& __GetCharacterManager();

protected:
	DWORD m_dwMainVID;

	LONG m_lMainPosX;
	LONG m_lMainPosY;

	std::map<DWORD, SNetworkActorData> m_kNetActorDict;
};
