#include "StdAfx.h"
#include "NetworkActorManager.h"
#include "PythonCharacterManager.h"
#include "PythonItem.h"
#include "AbstractPlayer.h"

void SNetworkActorData::UpdatePosition()
{
	DWORD dwClientCurTime = ELTimer_GetMSec();
	DWORD dwElapsedTime = dwClientCurTime - m_dwClientSrcTime;

	if (dwElapsedTime < m_dwDuration)
	{
		float fRate = float(dwElapsedTime) / float(m_dwDuration);
		m_lCurX = LONG((m_lDstX - m_lSrcX) * fRate + m_lSrcX);
		m_lCurY = LONG((m_lDstY - m_lSrcY) * fRate + m_lSrcY);
	}
	else
	{
		m_lCurX = m_lDstX;
		m_lCurY = m_lDstY;
	}
}

void SNetworkActorData::SetDstPosition(DWORD dwServerTime, LONG lDstX, LONG lDstY, DWORD dwDuration)
{
	m_lSrcX = m_lCurX;
	m_lSrcY = m_lCurY;
	m_lDstX = lDstX;
	m_lDstY = lDstY;

	m_dwDuration = dwDuration;
	m_dwServerSrcTime = dwServerTime;
	m_dwClientSrcTime = ELTimer_GetMSec();
}

void SNetworkActorData::SetPosition(LONG lPosX, LONG lPosY)
{
	m_lDstX = m_lSrcX = m_lCurX = lPosX;
	m_lDstY = m_lSrcY = m_lCurY = lPosY;
}

// NETWORK_ACTOR_DATA_COPY
SNetworkActorData::SNetworkActorData(const SNetworkActorData& src)
{
	__copy__(src);
}

void SNetworkActorData::operator=(const SNetworkActorData& src)
{
	__copy__(src);
}

void SNetworkActorData::__copy__(const SNetworkActorData& src)
{
	m_stName = src.m_stName;
	m_kAffectFlags.CopyInstance(src.m_kAffectFlags);
	m_bType = src.m_bType;
	m_dwVID = src.m_dwVID;
	m_dwStateFlags = src.m_dwStateFlags;
	m_bEmpireID = src.m_bEmpireID;
	m_dwRace = src.m_dwRace;
	m_wMovSpd = src.m_wMovSpd;
	m_wAtkSpd = src.m_wAtkSpd;
	m_fRot = src.m_fRot;
	m_lCurX = src.m_lCurX;
	m_lCurY = src.m_lCurY;
	m_lSrcX = src.m_lSrcX;
	m_lSrcY = src.m_lSrcY;
	m_lDstX = src.m_lDstX;
	m_lDstY = src.m_lDstY;
	m_dwServerSrcTime = src.m_dwServerSrcTime;
	m_dwClientSrcTime = src.m_dwClientSrcTime;
	m_dwDuration = src.m_dwDuration;
	m_dwArmor = src.m_dwArmor;
	m_dwWeapon = src.m_dwWeapon;
	m_dwHair = src.m_dwHair;
#ifdef ENABLE_ACCE_SYSTEM
	m_dwAcce = src.m_dwAcce;
	m_dwAcceEffect = src.m_dwAcceEffect;
#endif // ENABLE_ACCE_SYSTEM
#ifdef ENABLE_AURA_COSTUME_SYSTEM
	m_dwAura = src.m_dwAura;
#endif // ENABLE_AURA_COSTUME_SYSTEM
#ifdef ENABLE_QUIVER_SYSTEM
	m_dwQuiver = src.m_dwQuiver;
#endif // ENABLE_QUIVER_SYSTEM
	m_dwOwnerVID = src.m_dwOwnerVID;
	m_byPKMode = src.m_byPKMode;
	m_dwMountVnum = src.m_dwMountVnum;
	m_dwGuildID = src.m_dwGuildID;
	m_bLevel = src.m_bLevel;
#ifdef ENABLE_MOB_AGGR_LVL_INFO
	m_dwAIFlag = src.m_dwAIFlag;
#endif
#ifdef ENABLE_ALIGNMENT_SYSTEM
	m_alignment = src.m_alignment;
#endif
#ifdef ENABLE_RANK_SYSTEM
	m_sRank = src.m_sRank;
#endif
#ifdef ENABLE_LANDRANK_SYSTEM
	m_sLandRank = src.m_sLandRank;
#endif
#ifdef ENABLE_REBORN_SYSTEM
	m_sReborn = src.m_sReborn;
#endif
#ifdef ENABLE_TEAM_SYSTEM
	m_sTeam = src.m_sTeam;
#endif
#ifdef ENABLE_MONIKER_SYSTEM
	m_Moniker = src.m_Moniker;
#endif
#ifdef ENABLE_LOVE_SYSTEM
	m_Love1 = src.m_Love1;
	m_Love2 = src.m_Love2;
#endif // ENABLE_LOVE_SYSTEM
#ifdef ENABLE_WORD_SYSTEM
	m_word = src.m_word;
#endif // ENABLE_WORD_SYSTEM

#ifdef ENABLE_GUILD_LEADER_SYSTEM
	m_bMemberType = src.m_bMemberType;
#endif
#ifdef ENABLE_ITEM_EVOLUTION_SYSTEM
	m_wArmorEvolution = src.m_wArmorEvolution;
	m_wWeaponEvolution = src.m_wWeaponEvolution;
#endif
#ifdef ENABLE_SKILL_COLOR_SYSTEM
	memcpy(m_dwSkillColor, src.m_dwSkillColor, sizeof(m_dwSkillColor));
#endif
#ifdef ENABLE_SHINING_ITEM_SYSTEM
	memcpy(m_dwShining, src.m_dwShining, sizeof(m_dwShining));
#endif
}

SNetworkActorData::SNetworkActorData()
{
	SetPosition(0, 0);

	m_bType=0;
	m_dwVID=0;
	m_dwStateFlags=0;
	m_dwRace=0;
	m_wMovSpd=0;
	m_wAtkSpd=0;
	m_fRot=0.0f;

	m_bEmpireID=0;

	m_dwOwnerVID=0;

	m_dwDuration=0;
	m_dwClientSrcTime=0;
	m_dwServerSrcTime=0;

	m_alignment=0;
	m_byPKMode=0;
	m_dwMountVnum=0;

	m_stName = "";
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
#if defined(ENABLE_MOB_AGGR_LVL_INFO)
	m_dwAIFlag=0;
#endif
#ifdef ENABLE_RANK_SYSTEM
	m_sRank=0;
#endif
#ifdef ENABLE_LANDRANK_SYSTEM
	m_sLandRank=0;
#endif
#ifdef ENABLE_REBORN_SYSTEM
	m_sReborn=0;
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
	m_bMemberType=0;
#endif
#ifdef ENABLE_ITEM_EVOLUTION_SYSTEM
	m_wArmorEvolution = 0;
	m_wWeaponEvolution = 0;
#endif
#ifdef ENABLE_SKILL_COLOR_SYSTEM
	memset(m_dwSkillColor, 0, sizeof(m_dwSkillColor));
#endif
#ifdef ENABLE_SHINING_ITEM_SYSTEM
	std::fill_n(m_dwShining, CHR_SHINING_NUM, 0);
#endif

	m_kAffectFlags.Clear();
}
// END_OF_NETWORK_ACTOR_DATA_COPY
////////////////////////////////////////////////////////////////////////////////

CNetworkActorManager::CNetworkActorManager()
{
	m_dwMainVID = 0;
}

CNetworkActorManager::~CNetworkActorManager()
{
}

void CNetworkActorManager::Destroy()
{
	m_kNetActorDict.clear();

	m_dwMainVID = 0;
	m_lMainPosX = 0;
	m_lMainPosY = 0;
}

void CNetworkActorManager::SetMainActorVID(DWORD dwVID)
{
	m_dwMainVID = dwVID;
	m_lMainPosX = 0;
	m_lMainPosY = 0;

	m_kNetActorDict.clear();
}

void CNetworkActorManager::Update()
{
	__OLD_Update();
}

void CNetworkActorManager::__OLD_Update()
{
	__UpdateMainActor();

	CPythonCharacterManager& rkChrMgr = __GetCharacterManager();

	std::map<DWORD, SNetworkActorData>::iterator i;
	for (i = m_kNetActorDict.begin(); i != m_kNetActorDict.end(); ++i)
	{
		SNetworkActorData& rkNetActorData = i->second;
		rkNetActorData.UpdatePosition();

		CInstanceBase* pkInstFind = rkChrMgr.GetInstancePtr(rkNetActorData.m_dwVID);
		if (!pkInstFind)
		{
			if (__IsVisibleActor(rkNetActorData))
				__AppendCharacterManagerActor(rkNetActorData);
		}
	}
}

CInstanceBase* CNetworkActorManager::__FindActor(SNetworkActorData& rkNetActorData, LONG lDstX, LONG lDstY)
{
	CPythonCharacterManager& rkChrMgr = __GetCharacterManager();
	CInstanceBase* pkInstFind = rkChrMgr.GetInstancePtr(rkNetActorData.m_dwVID);
	if (!pkInstFind)
	{
		if (__IsVisiblePos(lDstX, lDstY))
			return __AppendCharacterManagerActor(rkNetActorData);

		return NULL;
	}

	return pkInstFind;
}

CInstanceBase* CNetworkActorManager::__FindActor(SNetworkActorData& rkNetActorData)
{
	CPythonCharacterManager& rkChrMgr = __GetCharacterManager();
	CInstanceBase* pkInstFind = rkChrMgr.GetInstancePtr(rkNetActorData.m_dwVID);
	if (!pkInstFind)
	{
		if (__IsVisibleActor(rkNetActorData))
			return __AppendCharacterManagerActor(rkNetActorData);

		return NULL;
	}

	return pkInstFind;
}

void CNetworkActorManager::__RemoveAllGroundItems()
{
	CPythonItem& rkItemMgr = CPythonItem::Instance();
	rkItemMgr.DeleteAllItems();
}

void CNetworkActorManager::__RemoveAllActors()
{
	m_kNetActorDict.clear();

	CPythonCharacterManager& rkChrMgr = CPythonCharacterManager::Instance();
	rkChrMgr.DeleteAllInstances();
}

void CNetworkActorManager::__RemoveDynamicActors()
{
	//copy it
	std::vector<DWORD> dwCharacterVIDList;

	CPythonCharacterManager& rkChrMgr = CPythonCharacterManager::Instance();
	for (CPythonCharacterManager::CharacterIterator i = rkChrMgr.CharacterInstanceBegin(); i != rkChrMgr.CharacterInstanceEnd(); ++i)
	{
		dwCharacterVIDList.push_back((*i)->GetVirtualID());
	}

	for (int i = 0; i < dwCharacterVIDList.size(); ++i)
	{
		CInstanceBase* pkInstEach = rkChrMgr.GetInstancePtr(dwCharacterVIDList[i]);
		if (!pkInstEach)
			continue;

		CActorInstance* rkActorEach = pkInstEach->GetGraphicThingInstancePtr();
		if (rkActorEach->IsPC() || rkActorEach->IsNPC() || rkActorEach->IsEnemy() || rkActorEach->IsStone()) // when teleport frozen screen fix
		{
			rkChrMgr.DeleteInstance(dwCharacterVIDList[i]);
			std::map<DWORD, SNetworkActorData>::iterator it = m_kNetActorDict.find(dwCharacterVIDList[i]);
			if (it != m_kNetActorDict.end())
			{
				m_kNetActorDict.erase(it);
			}
		}
	}

	rkChrMgr.DestroyDeadInstanceList();
}

void CNetworkActorManager::__UpdateMainActor()
{
	CPythonCharacterManager& rkChrMgr = CPythonCharacterManager::Instance();
	CInstanceBase* pkInstMain = rkChrMgr.GetMainInstancePtr();
	if (!pkInstMain)
		return;

	TPixelPosition kPPosMain;
	pkInstMain->NEW_GetPixelPosition(&kPPosMain);

	m_lMainPosX = LONG(kPPosMain.x);
	m_lMainPosY = LONG(kPPosMain.y);
}

extern bool IsWall(unsigned race);

bool CNetworkActorManager::__IsVisibleActor(const SNetworkActorData& c_rkNetActorData)
{
	if (__IsMainActorVID(c_rkNetActorData.m_dwVID))
		return true;

	if (c_rkNetActorData.m_kAffectFlags.IsSet(CInstanceBase::AFFECT_SHOW_ALWAYS))
		return true;

	if (__IsVisiblePos(c_rkNetActorData.m_lCurX, c_rkNetActorData.m_lCurY))
		return true;

	if (IsWall(c_rkNetActorData.m_dwRace))
		return true;

	return false;
}

bool CNetworkActorManager::__IsVisiblePos(LONG lPosX, LONG lPosY)
{
	LONG dx = lPosX - m_lMainPosX;
	LONG dy = lPosY - m_lMainPosY;
	LONG len = (LONG)sqrtf((float(dx) * float(dx) + float(dy) * float(dy)));

	extern int CHAR_STAGE_VIEW_BOUND;
	if (len < CHAR_STAGE_VIEW_BOUND && len > -CHAR_STAGE_VIEW_BOUND)
		return true;

	return false;
}

bool CNetworkActorManager::__IsMainActorVID(DWORD dwVID)
{
	if (dwVID == m_dwMainVID)
		return true;

	return false;
}

CPythonCharacterManager& CNetworkActorManager::__GetCharacterManager()
{
	return CPythonCharacterManager::Instance();
}

void CNetworkActorManager::__RemoveCharacterManagerActor(SNetworkActorData& rkNetActorData)
{
	CPythonCharacterManager& rkChrMgr = CPythonCharacterManager::Instance();

	if (__IsMainActorVID(rkNetActorData.m_dwVID))
	{
		rkChrMgr.DeleteInstance(rkNetActorData.m_dwVID);
	}
	else
	{
		rkChrMgr.DeleteInstanceByFade(rkNetActorData.m_dwVID);
	}
}

CInstanceBase* CNetworkActorManager::__AppendCharacterManagerActor(SNetworkActorData& rkNetActorData)
{
	Lognf(0, "CNetworkActorManager::__AppendCharacterManagerActor(%d, %s)", rkNetActorData.m_dwVID, rkNetActorData.m_stName.c_str());

	DWORD dwVID = rkNetActorData.m_dwVID;

	CPythonCharacterManager& rkChrMgr = CPythonCharacterManager::Instance();

	CInstanceBase::SCreateData kCreateData;
	kCreateData.m_bType = rkNetActorData.m_bType;
	kCreateData.m_bLevel = rkNetActorData.m_bLevel;
	kCreateData.m_dwGuildID = rkNetActorData.m_dwGuildID;
	kCreateData.m_dwEmpireID = rkNetActorData.m_bEmpireID;
	kCreateData.m_dwVID = rkNetActorData.m_dwVID;
	kCreateData.m_dwMountVnum = rkNetActorData.m_dwMountVnum;
	kCreateData.m_dwRace = rkNetActorData.m_dwRace;
	kCreateData.m_fRot = rkNetActorData.m_fRot;
	kCreateData.m_lPosX = rkNetActorData.m_lCurX;
	kCreateData.m_lPosY = rkNetActorData.m_lCurY;
	kCreateData.m_stName = rkNetActorData.m_stName;
	kCreateData.m_dwStateFlags = rkNetActorData.m_dwStateFlags;
	kCreateData.m_dwMovSpd = rkNetActorData.m_wMovSpd;
	kCreateData.m_dwAtkSpd = rkNetActorData.m_wAtkSpd;
	kCreateData.m_alignment = rkNetActorData.m_alignment;
	kCreateData.m_byPKMode = rkNetActorData.m_byPKMode;
	kCreateData.m_kAffectFlags = rkNetActorData.m_kAffectFlags;
	kCreateData.m_dwArmor = rkNetActorData.m_dwArmor;
	kCreateData.m_dwWeapon = rkNetActorData.m_dwWeapon;
	kCreateData.m_dwHair = rkNetActorData.m_dwHair;
#ifdef ENABLE_ACCE_SYSTEM
	kCreateData.m_dwAcce = rkNetActorData.m_dwAcce;
	kCreateData.m_dwAcceEffect = rkNetActorData.m_dwAcceEffect;
#endif // ENABLE_ACCE_SYSTEM
#ifdef ENABLE_AURA_COSTUME_SYSTEM
	kCreateData.m_dwAura = rkNetActorData.m_dwAura;
#endif // ENABLE_AURA_COSTUME_SYSTEM
#ifdef ENABLE_QUIVER_SYSTEM
	kCreateData.m_dwQuiver = rkNetActorData.m_dwQuiver;
#endif // ENABLE_QUIVER_SYSTEM
#ifdef ENABLE_MOB_AGGR_LVL_INFO
	kCreateData.m_dwAIFlag = rkNetActorData.m_dwAIFlag;
#endif
#ifdef ENABLE_RANK_SYSTEM
	kCreateData.m_sRank = rkNetActorData.m_sRank;
#endif
#ifdef ENABLE_LANDRANK_SYSTEM
	kCreateData.m_sLandRank = rkNetActorData.m_sLandRank;
#endif
#ifdef ENABLE_REBORN_SYSTEM
	kCreateData.m_sReborn = rkNetActorData.m_sReborn;
#endif
#ifdef ENABLE_TEAM_SYSTEM
	kCreateData.m_sTeam = rkNetActorData.m_sTeam;
#endif
#ifdef ENABLE_MONIKER_SYSTEM
	kCreateData.m_Moniker = rkNetActorData.m_Moniker;
#endif
#ifdef ENABLE_LOVE_SYSTEM
	kCreateData.m_Love1 = rkNetActorData.m_Love1;
	kCreateData.m_Love2 = rkNetActorData.m_Love2;
#endif // ENABLE_LOVE_SYSTEM
#ifdef ENABLE_WORD_SYSTEM
	kCreateData.m_word = rkNetActorData.m_word;
#endif // ENABLE_WORD_SYSTEM

#ifdef ENABLE_GUILD_LEADER_SYSTEM
	kCreateData.m_bMemberType = rkNetActorData.m_bMemberType;
#endif
#ifdef ENABLE_ITEM_EVOLUTION_SYSTEM
	kCreateData.m_wArmorEvolution = rkNetActorData.m_wArmorEvolution;
	kCreateData.m_wWeaponEvolution = rkNetActorData.m_wWeaponEvolution;
#endif
#ifdef ENABLE_SHINING_ITEM_SYSTEM
	memcpy(kCreateData.m_dwShining, rkNetActorData.m_dwShining, sizeof(kCreateData.m_dwShining));
#endif
#ifdef ENABLE_SKILL_COLOR_SYSTEM
	memcpy(kCreateData.m_dwSkillColor, rkNetActorData.m_dwSkillColor, sizeof(kCreateData.m_dwSkillColor));
#endif

	kCreateData.m_isMain = __IsMainActorVID(dwVID);

	CInstanceBase* pOldInstance = rkChrMgr.GetInstancePtr(dwVID);
	if (pOldInstance)
	{
		TPixelPosition oldPos;
		pOldInstance->GetGraphicThingInstancePtr()->GetPixelPosition(&oldPos);

		BOOL bIsMountingHorse = pOldInstance->IsMountingHorse();
		if( (bIsMountingHorse && kCreateData.m_dwMountVnum == 0) ||
			(!bIsMountingHorse && kCreateData.m_dwMountVnum != 0) )
		{
			//Override Pos
			kCreateData.m_lPosX = oldPos.x;
			kCreateData.m_lPosY = -oldPos.y;
		}

		rkChrMgr.DeleteInstance(dwVID);
	}

	CInstanceBase* pNewInstance = rkChrMgr.CreateInstance(kCreateData);
	if (!pNewInstance)
		return NULL;

	if (kCreateData.m_isMain)
	{
		IAbstractPlayer& rkPlayer = IAbstractPlayer::GetSingleton();
		rkPlayer.SetMainCharacterIndex(dwVID);

		m_lMainPosX = rkNetActorData.m_lCurX;
		m_lMainPosY = rkNetActorData.m_lCurY;
	}

	DWORD dwClientCurTime = ELTimer_GetMSec();
	DWORD dwElapsedTime = dwClientCurTime - rkNetActorData.m_dwClientSrcTime;

	if (dwElapsedTime < rkNetActorData.m_dwDuration)
	{
		TPixelPosition kPPosDst;
		kPPosDst.x = float(rkNetActorData.m_lDstX);
		kPPosDst.y = float(rkNetActorData.m_lDstY);
		kPPosDst.z = 0;
		pNewInstance->PushTCPState(rkNetActorData.m_dwServerSrcTime + dwElapsedTime, kPPosDst, rkNetActorData.m_fRot, CInstanceBase::FUNC_MOVE, 0);
	}
	return pNewInstance;
}

void CNetworkActorManager::AppendActor(const SNetworkActorData& c_rkNetActorData)
{
	if (__IsMainActorVID(c_rkNetActorData.m_dwVID))
	{
		bool bChangeMountStatus = false;

		CInstanceBase* pMainInstance = CPythonCharacterManager::Instance().GetInstancePtr(c_rkNetActorData.m_dwVID);
		if (pMainInstance)
		{
			BOOL bIsMountingHorse = pMainInstance->IsMountingHorse();
			bChangeMountStatus = (bIsMountingHorse && c_rkNetActorData.m_dwMountVnum == 0) || (!bIsMountingHorse && c_rkNetActorData.m_dwMountVnum != 0);
#ifdef ENABLE_SKILL_COLOR_SYSTEM
			pMainInstance->ChangeSkillColor(*c_rkNetActorData.m_dwSkillColor);
#endif
		}

		if (!bChangeMountStatus)
		{
			//__RemoveAllActors();
			__RemoveDynamicActors();
			__RemoveAllGroundItems();
		}
	}

	SNetworkActorData& rkNetActorData = m_kNetActorDict[c_rkNetActorData.m_dwVID];
	rkNetActorData = c_rkNetActorData;

	if (__IsVisibleActor(rkNetActorData))
	{
		if (!__AppendCharacterManagerActor(rkNetActorData))
			m_kNetActorDict.erase(c_rkNetActorData.m_dwVID);
	}
}

void CNetworkActorManager::RemoveActor(DWORD dwVID)
{
	std::map<DWORD, SNetworkActorData>::iterator f = m_kNetActorDict.find(dwVID);
	if (m_kNetActorDict.end() == f)
	{
#ifdef _DEBUG
		TraceError("CNetworkActorManager::RemoveActor(dwVID=%d) - NOT EXIST VID", dwVID);
#endif
		return;
	}

	SNetworkActorData& rkNetActorData = f->second;
	__RemoveCharacterManagerActor(rkNetActorData);

	m_kNetActorDict.erase(f);
}

void CNetworkActorManager::UpdateActor(const SNetworkUpdateActorData& c_rkNetUpdateActorData)
{
	std::map<DWORD, SNetworkActorData>::iterator f = m_kNetActorDict.find(c_rkNetUpdateActorData.m_dwVID);
	if (m_kNetActorDict.end() == f)
	{
#ifdef _DEBUG
		TraceError("CNetworkActorManager::UpdateActor(dwVID=%d) - NOT EXIST VID", c_rkNetUpdateActorData.m_dwVID);
#endif
		return;
	}

	SNetworkActorData& rkNetActorData = f->second;

	CInstanceBase* pkInstFind = __FindActor(rkNetActorData);

	if (pkInstFind)
	{
#ifdef ENABLE_AUTO_HUNT_SYSTEM
		pkInstFind->SetHasAttacker(c_rkNetUpdateActorData.m_hasAttacker);
#endif // ENABLE_AUTO_HUNT_SYSTEM
#ifdef ENABLE_ARMOR_EVOLUTION_SYSTEM
		pkInstFind->ChangeArmor(c_rkNetUpdateActorData.m_dwArmor, c_rkNetUpdateActorData.m_wArmorEvolution);
#else
		pkInstFind->ChangeArmor(c_rkNetUpdateActorData.m_dwArmor);
#endif
#ifdef ENABLE_ITEM_EVOLUTION_SYSTEM
		pkInstFind->ChangeWeapon(c_rkNetUpdateActorData.m_dwWeapon, c_rkNetUpdateActorData.m_wWeaponEvolution);
#else
		pkInstFind->ChangeWeapon(c_rkNetUpdateActorData.m_dwWeapon);
#endif
		pkInstFind->ChangeHair(c_rkNetUpdateActorData.m_dwHair);
#ifdef ENABLE_ACCE_SYSTEM
		pkInstFind->ChangeAcce(c_rkNetUpdateActorData.m_dwAcce, c_rkNetUpdateActorData.m_dwAcceEffect);
#endif
#ifdef ENABLE_AURA_COSTUME_SYSTEM
		pkInstFind->ChangeAura(c_rkNetUpdateActorData.m_dwAura);
#endif
#ifdef ENABLE_QUIVER_SYSTEM
		pkInstFind->SetArrow(c_rkNetUpdateActorData.m_dwQuiver);
#endif
#ifdef ENABLE_GUILD_LEADER_SYSTEM
		pkInstFind->SetGuildMemberType(c_rkNetUpdateActorData.m_bMemberType);
#endif
		pkInstFind->ChangeGuild(c_rkNetUpdateActorData.m_dwGuildID);
		pkInstFind->SetAffectFlagContainer(c_rkNetUpdateActorData.m_kAffectFlags);
		pkInstFind->SetMoveSpeed(c_rkNetUpdateActorData.m_wMovSpd);
		pkInstFind->SetAttackSpeed(c_rkNetUpdateActorData.m_wAtkSpd);
		pkInstFind->SetAlignment(c_rkNetUpdateActorData.m_alignment);
		pkInstFind->SetPKMode(c_rkNetUpdateActorData.m_byPKMode);
		pkInstFind->SetStateFlags(c_rkNetUpdateActorData.m_dwStateFlags);
		// Level
		pkInstFind->SetLevelText(c_rkNetUpdateActorData.m_bLevel);
#ifdef ENABLE_RANK_SYSTEM
		pkInstFind->SetRank(c_rkNetUpdateActorData.m_sRank);
#endif
#ifdef ENABLE_LANDRANK_SYSTEM
		pkInstFind->SetLandRank(c_rkNetUpdateActorData.m_sLandRank);
#endif
#ifdef ENABLE_REBORN_SYSTEM
		pkInstFind->SetReborn(c_rkNetUpdateActorData.m_sReborn);
#endif
#ifdef ENABLE_TEAM_SYSTEM
		pkInstFind->SetTeam(c_rkNetUpdateActorData.m_sTeam);
#endif
#ifdef ENABLE_MONIKER_SYSTEM
		pkInstFind->SetMoniker(c_rkNetUpdateActorData.m_Moniker.c_str());
#endif
#ifdef ENABLE_LOVE_SYSTEM
		pkInstFind->SetLove1(c_rkNetUpdateActorData.m_Love1.c_str());
		pkInstFind->SetLove2(c_rkNetUpdateActorData.m_Love2.c_str());
#endif // ENABLE_LOVE_SYSTEM
#ifdef ENABLE_WORD_SYSTEM
		pkInstFind->SetWord(c_rkNetUpdateActorData.m_word.c_str());
#endif // ENABLE_WORD_SYSTEM

#ifdef ENABLE_ITEM_EVOLUTION_SYSTEM
		pkInstFind->SetWeaponEvolution(c_rkNetUpdateActorData.m_wWeaponEvolution);
#endif
#ifdef ENABLE_ARMOR_EVOLUTION_SYSTEM
		pkInstFind->SetArmorEvolution(c_rkNetUpdateActorData.m_wArmorEvolution);
#endif
#ifdef ENABLE_SKILL_COLOR_SYSTEM
		pkInstFind->ChangeSkillColor(*c_rkNetUpdateActorData.m_dwSkillColor);
#endif
#ifdef ENABLE_SHINING_ITEM_SYSTEM
		for (int i = 0; i < CHR_SHINING_NUM; i++)
			pkInstFind->SetShining(i, c_rkNetUpdateActorData.m_dwShining[i]);
#endif
	}

	rkNetActorData.m_kAffectFlags = c_rkNetUpdateActorData.m_kAffectFlags;
	rkNetActorData.m_dwGuildID = c_rkNetUpdateActorData.m_dwGuildID;
	rkNetActorData.m_bLevel = c_rkNetUpdateActorData.m_bLevel;
	rkNetActorData.m_wMovSpd = c_rkNetUpdateActorData.m_wMovSpd;
	rkNetActorData.m_wAtkSpd = c_rkNetUpdateActorData.m_wAtkSpd;
	rkNetActorData.m_alignment = c_rkNetUpdateActorData.m_alignment;
	rkNetActorData.m_byPKMode = c_rkNetUpdateActorData.m_byPKMode;
	rkNetActorData.m_dwArmor = c_rkNetUpdateActorData.m_dwArmor;
	rkNetActorData.m_dwWeapon = c_rkNetUpdateActorData.m_dwWeapon;
	rkNetActorData.m_dwHair = c_rkNetUpdateActorData.m_dwHair;
#ifdef ENABLE_ACCE_SYSTEM
	rkNetActorData.m_dwAcce = c_rkNetUpdateActorData.m_dwAcce;
	rkNetActorData.m_dwAcceEffect = c_rkNetUpdateActorData.m_dwAcceEffect;
#endif // ENABLE_ACCE_SYSTEM
#ifdef ENABLE_AURA_COSTUME_SYSTEM
	rkNetActorData.m_dwAura = c_rkNetUpdateActorData.m_dwAura;
#endif // ENABLE_AURA_COSTUME_SYSTEM
#ifdef ENABLE_QUIVER_SYSTEM
	rkNetActorData.m_dwQuiver = c_rkNetUpdateActorData.m_dwQuiver;
#endif // ENABLE_QUIVER_SYSTEM
#ifdef ENABLE_RANK_SYSTEM
	rkNetActorData.m_sRank = c_rkNetUpdateActorData.m_sRank;
#endif
#ifdef ENABLE_LANDRANK_SYSTEM
	rkNetActorData.m_sLandRank = c_rkNetUpdateActorData.m_sLandRank;
#endif
#ifdef ENABLE_REBORN_SYSTEM
	rkNetActorData.m_sReborn = c_rkNetUpdateActorData.m_sReborn;
#endif
#ifdef ENABLE_TEAM_SYSTEM
	rkNetActorData.m_sTeam = c_rkNetUpdateActorData.m_sTeam;
#endif
#ifdef ENABLE_MONIKER_SYSTEM
	rkNetActorData.m_Moniker = c_rkNetUpdateActorData.m_Moniker;
#endif
#ifdef ENABLE_LOVE_SYSTEM
	rkNetActorData.m_Love1 = c_rkNetUpdateActorData.m_Love1;
	rkNetActorData.m_Love2 = c_rkNetUpdateActorData.m_Love2;
#endif // ENABLE_LOVE_SYSTEM
#ifdef ENABLE_WORD_SYSTEM
	rkNetActorData.m_word =  c_rkNetUpdateActorData.m_word;
#endif // ENABLE_WORD_SYSTEM

#ifdef ENABLE_GUILD_LEADER_SYSTEM
	rkNetActorData.m_bMemberType = c_rkNetUpdateActorData.m_bMemberType;
#endif
#ifdef ENABLE_ITEM_EVOLUTION_SYSTEM
	rkNetActorData.m_wArmorEvolution = c_rkNetUpdateActorData.m_wArmorEvolution;
	rkNetActorData.m_wWeaponEvolution = c_rkNetUpdateActorData.m_wWeaponEvolution;
#endif
#ifdef ENABLE_SKILL_COLOR_SYSTEM
	memcpy(rkNetActorData.m_dwSkillColor, c_rkNetUpdateActorData.m_dwSkillColor, sizeof(rkNetActorData.m_dwSkillColor));
#endif
#ifdef ENABLE_SHINING_ITEM_SYSTEM
	memcpy(rkNetActorData.m_dwShining, c_rkNetUpdateActorData.m_dwShining, sizeof(rkNetActorData.m_dwShining));
#endif
#ifdef ENABLE_AUTO_HUNT_SYSTEM
	rkNetActorData.m_hasAttacker = c_rkNetUpdateActorData.m_hasAttacker;
#endif // ENABLE_AUTO_HUNT_SYSTEM
}

void CNetworkActorManager::MoveActor(const SNetworkMoveActorData& c_rkNetMoveActorData)
{
	std::map<DWORD, SNetworkActorData>::iterator f = m_kNetActorDict.find(c_rkNetMoveActorData.m_dwVID);
	if (m_kNetActorDict.end() == f)
	{
#ifdef _DEBUG
		TraceError("CNetworkActorManager::MoveActor(dwVID=%d) - NOT EXIST VID", c_rkNetMoveActorData.m_dwVID);
#endif
		return;
	}

	SNetworkActorData& rkNetActorData = f->second;

	CInstanceBase* pkInstFind = __FindActor(rkNetActorData, c_rkNetMoveActorData.m_lPosX, c_rkNetMoveActorData.m_lPosY);
	if (pkInstFind)
	{
		TPixelPosition kPPosDst;
		kPPosDst.x = float(c_rkNetMoveActorData.m_lPosX);
		kPPosDst.y = float(c_rkNetMoveActorData.m_lPosY);
		kPPosDst.z = 0.0f;

		pkInstFind->PushTCPState(c_rkNetMoveActorData.m_dwTime, kPPosDst,
			c_rkNetMoveActorData.m_fRot, c_rkNetMoveActorData.m_dwFunc, c_rkNetMoveActorData.m_dwArg);
	}

	rkNetActorData.SetDstPosition(c_rkNetMoveActorData.m_dwTime,
		c_rkNetMoveActorData.m_lPosX, c_rkNetMoveActorData.m_lPosY, c_rkNetMoveActorData.m_dwDuration);
	rkNetActorData.m_fRot = c_rkNetMoveActorData.m_fRot;
}

void CNetworkActorManager::SyncActor(DWORD dwVID, LONG lPosX, LONG lPosY)
{
	std::map<DWORD, SNetworkActorData>::iterator f = m_kNetActorDict.find(dwVID);
	if (m_kNetActorDict.end() == f)
	{
#ifdef _DEBUG
		TraceError("CNetworkActorManager::SyncActor(dwVID=%d) - NOT EXIST VID", dwVID);
#endif
		return;
	}

	SNetworkActorData& rkNetActorData = f->second;

	CInstanceBase* pkInstFind = __FindActor(rkNetActorData);
	if (pkInstFind)
	{
		pkInstFind->NEW_SyncPixelPosition(lPosX, lPosY);
	}

	rkNetActorData.SetPosition(lPosX, lPosY);
}

void CNetworkActorManager::SetActorOwner(DWORD dwOwnerVID, DWORD dwVictimVID)
{
	std::map<DWORD, SNetworkActorData>::iterator f = m_kNetActorDict.find(dwVictimVID);
	if (m_kNetActorDict.end() == f)
	{
#ifdef _DEBUG
		TraceError("CNetworkActorManager::SetActorOwner(dwOwnerVID=%d, dwVictimVID=%d) - NOT EXIST VID", dwOwnerVID, dwVictimVID);
#endif
		return;
	}

	SNetworkActorData& rkNetActorData = f->second;
	rkNetActorData.m_dwOwnerVID = dwOwnerVID;

	CInstanceBase* pkInstFind = __FindActor(rkNetActorData);
	if (pkInstFind)
	{
		pkInstFind->NEW_SetOwner(rkNetActorData.m_dwOwnerVID);
	}
}