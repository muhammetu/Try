#include "StdAfx.h"
#include "InstanceBase.h"
#include "PythonBackground.h"
#include "PythonNonPlayer.h"
#include "PythonPlayer.h"
#include "PythonCharacterManager.h"
#include "AbstractPlayer.h"
#include "AbstractApplication.h"
#include "packet.h"
#include "PythonNetworkStream.h"
#include "../eterlib/StateManager.h"
#include "../gamelib/ItemManager.h"
#include "../gamelib/RaceManager.h"
#include "../gamelib/GameLibDefines.h"
#ifdef ENABLE_GRAPHIC_OPTIMIZATION
#include "PythonSystem.h"
#endif

BOOL RIDE_HORSE_ENABLE = TRUE;
const float c_fDefaultRotationSpeed = 1200.0f;
const float c_fDefaultHorseRotationSpeed = 1500.0f;

bool IsWall(unsigned race)
{
	switch (race)
	{
	case 14201:
	case 14202:
	case 14203:
	case 14204:
		return true;
		break;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////////////////

CInstanceBase::SHORSE::SHORSE()
{
	__Initialize();
}

CInstanceBase::SHORSE::~SHORSE()
{
	assert(m_pkActor == NULL);
}

void CInstanceBase::SHORSE::__Initialize()
{
	m_isMounting = false;
	m_pkActor = NULL;
}

void CInstanceBase::SHORSE::SetAttackSpeed(UINT uAtkSpd)
{
	if (!IsMounting())
		return;

	CActorInstance& rkActor = GetActorRef();
	rkActor.SetAttackSpeed(uAtkSpd / 100.0f);
}

void CInstanceBase::SHORSE::SetMoveSpeed(UINT uMovSpd)
{
	if (!IsMounting())
		return;

	CActorInstance& rkActor = GetActorRef();
	rkActor.SetMoveSpeed(uMovSpd / 100.0f);
}

void CInstanceBase::SHORSE::Create(const TPixelPosition& c_rkPPos, UINT eRace, UINT eHitEffect)
{
	assert(NULL == m_pkActor && "CInstanceBase::SHORSE::Create - ALREADY MOUNT");

	m_pkActor = new CActorInstance;

	CActorInstance& rkActor = GetActorRef();
	rkActor.SetEventHandler(CActorInstance::IEventHandler::GetEmptyPtr());
	if (!rkActor.SetRace(eRace))
	{
		delete m_pkActor;
		m_pkActor = NULL;
		return;
	}

	rkActor.SetShape(0);
	rkActor.SetBattleHitEffect(eHitEffect);
	rkActor.SetAlphaValue(0.0f);
	rkActor.BlendAlphaValue(1.0f, 0.5f);
	rkActor.SetMoveSpeed(1.0f);
	rkActor.SetAttackSpeed(1.0f);
	rkActor.SetMotionMode(CRaceMotionData::MODE_GENERAL);
	rkActor.Stop();
	rkActor.RefreshActorInstance();
	rkActor.SetCurPixelPosition(c_rkPPos);

	m_isMounting = true;
}

void CInstanceBase::SHORSE::Destroy()
{
	if (m_pkActor)
	{
		m_pkActor->Destroy();
		delete m_pkActor;
	}

	__Initialize();
}

CActorInstance& CInstanceBase::SHORSE::GetActorRef()
{
	assert(NULL != m_pkActor && "CInstanceBase::SHORSE::GetActorRef");
	return *m_pkActor;
}

CActorInstance* CInstanceBase::SHORSE::GetActorPtr()
{
	return m_pkActor;
}

enum eMountType { MOUNT_TYPE_NONE = 0, MOUNT_TYPE_NORMAL = 1, MOUNT_TYPE_COMBAT = 2, MOUNT_TYPE_MILITARY = 3 };

UINT CInstanceBase::SHORSE::GetLevel()
{
	if (m_pkActor)
	{
		return (m_pkActor->GetRace()) ? MOUNT_TYPE_MILITARY : MOUNT_TYPE_NONE;
	}
	return 0;
}

bool CInstanceBase::SHORSE::CanUseSkill()
{
	if (IsMounting())
		return 2 < GetLevel();

	return true;
}

bool CInstanceBase::SHORSE::CanAttack()
{
	if (IsMounting())
		if (GetLevel() <= 1)
			return false;

	return true;
}

bool CInstanceBase::SHORSE::IsMounting()
{
	return m_isMounting;
}

void CInstanceBase::SHORSE::Deform()
{
	if (!IsMounting())
		return;

	CActorInstance& rkActor = GetActorRef();
	rkActor.INSTANCEBASE_Deform();
}

void CInstanceBase::SHORSE::Render()
{
	if (!IsMounting())
		return;

	CActorInstance& rkActor = GetActorRef();
	rkActor.Render();
}

void CInstanceBase::__AttachHorseSaddle()
{
	if (!IsMountingHorse())
		return;
	m_kHorse.m_pkActor->AttachModelInstance(CRaceData::PART_MAIN, "saddle", m_GraphicThingInstance, CRaceData::PART_MAIN);
}

void CInstanceBase::__DetachHorseSaddle()
{
	if (!IsMountingHorse())
		return;
	m_kHorse.m_pkActor->DetachModelInstance(CRaceData::PART_MAIN, m_GraphicThingInstance, CRaceData::PART_MAIN);
}

//////////////////////////////////////////////////////////////////////////////////////

void CInstanceBase::BlockMovement()
{
	m_GraphicThingInstance.BlockMovement();
}

bool CInstanceBase::IsBlockObject(const CGraphicObjectInstance& c_rkBGObj)
{
	return m_GraphicThingInstance.IsBlockObject(c_rkBGObj);
}

bool CInstanceBase::AvoidObject(const CGraphicObjectInstance& c_rkBGObj)
{
	return m_GraphicThingInstance.AvoidObject(c_rkBGObj);
}

///////////////////////////////////////////////////////////////////////////////////

bool __ArmorVnumToShape(int iVnum, DWORD* pdwShape)
{
	*pdwShape = iVnum;

	/////////////////////////////////////////

	if (0 == iVnum || 1 == iVnum)
		return false;

	CItemData* pItemData;
	if (!CItemManager::Instance().GetItemDataPointer(iVnum, &pItemData))
		return false;

	enum
	{
		SHAPE_VALUE_SLOT_INDEX = 3,
	};

	*pdwShape = pItemData->GetValue(SHAPE_VALUE_SLOT_INDEX);

	return true;
}

class CActorInstanceBackground : public IBackground
{
public:
	CActorInstanceBackground() {}
	virtual ~CActorInstanceBackground() {}
	bool IsBlock(int x, int y)
	{
		CPythonBackground& rkBG = CPythonBackground::Instance();
		return rkBG.isAttrOn(x, y, CTerrainImpl::ATTRIBUTE_BLOCK);
	}
};

static CActorInstanceBackground gs_kActorInstBG;

bool CInstanceBase::LessRenderOrder(CInstanceBase* pkInst)
{
	int nMainAlpha = (__GetAlphaValue() < 1.0f) ? 1 : 0;
	int nTestAlpha = (pkInst->__GetAlphaValue() < 1.0f) ? 1 : 0;

	if (nMainAlpha < nTestAlpha)
		return true;

	if (nMainAlpha > nTestAlpha)
		return false;

	if (GetRace() < pkInst->GetRace())
		return true;

	if (GetRace() > pkInst->GetRace())
		return false;

	if (GetShape() < pkInst->GetShape())
		return true;

	if (GetShape() > pkInst->GetShape())
		return false;

	UINT uLeftLODLevel = __LessRenderOrder_GetLODLevel();
	UINT uRightLODLevel = pkInst->__LessRenderOrder_GetLODLevel();

	if (uLeftLODLevel < uRightLODLevel)
		return true;

	if (uLeftLODLevel > uRightLODLevel)
		return false;

	if (m_awPart[CRaceData::PART_WEAPON] < pkInst->m_awPart[CRaceData::PART_WEAPON])
		return true;

	return false;
}

UINT CInstanceBase::__LessRenderOrder_GetLODLevel()
{
	CGrannyLODController* pLODCtrl = m_GraphicThingInstance.GetLODControllerPointer(0);
	if (!pLODCtrl)
		return 0;

	return pLODCtrl->GetLODLevel();
}

bool CInstanceBase::__Background_GetWaterHeight(const TPixelPosition& c_rkPPos, float* pfHeight)
{
	long lHeight;
	if (!CPythonBackground::Instance().GetWaterHeight(int(c_rkPPos.x), int(c_rkPPos.y), &lHeight))
		return false;

	*pfHeight = float(lHeight);

	return true;
}

bool CInstanceBase::__Background_IsWaterPixelPosition(const TPixelPosition& c_rkPPos)
{
	return CPythonBackground::Instance().isAttrOn(c_rkPPos.x, c_rkPPos.y, CTerrainImpl::ATTRIBUTE_WATER);
}

const float PC_DUST_RANGE = 2000.0f;
const float NPC_DUST_RANGE = 1000.0f;

DWORD CInstanceBase::ms_dwUpdateCounter = 0;
DWORD CInstanceBase::ms_dwRenderCounter = 0;
DWORD CInstanceBase::ms_dwDeformCounter = 0;

CDynamicPool<CInstanceBase> CInstanceBase::ms_kPool;

bool CInstanceBase::__IsInDustRange()
{
	if (!__IsExistMainInstance())
		return false;

	CInstanceBase* pkInstMain = __GetMainInstancePtr();

	float fDistance = NEW_GetDistanceFromDestInstance(*pkInstMain);

	if (IsPC())
	{
		if (fDistance <= PC_DUST_RANGE)
			return true;
	}

	if (fDistance <= NPC_DUST_RANGE)
		return true;

	return false;
}

void CInstanceBase::__EnableSkipCollision()
{
	if (__IsMainInstance())
	{
		TraceError("CInstanceBase::__EnableSkipCollision - You should not skip your own collisions!!");
		return;
	}

	m_GraphicThingInstance.EnableSkipCollision();
}

void CInstanceBase::__DisableSkipCollision()
{
	m_GraphicThingInstance.DisableSkipCollision();
}

DWORD CInstanceBase::__GetShadowMapColor(float x, float y)
{
	CPythonBackground& rkBG = CPythonBackground::Instance();
	return rkBG.GetShadowMapColor(x, y);
}

float CInstanceBase::__GetBackgroundHeight(float x, float y)
{
	CPythonBackground& rkBG = CPythonBackground::Instance();
	return rkBG.GetHeight(x, y);
}

BOOL CInstanceBase::IsInvisibility()
{
#if defined(ENABLE_CANSEEHIDDENTHING_FOR_GM)
	if (IsAffect(AFFECT_INVISIBILITY) && !__MainCanSeeHiddenThing())
		return true;
#else
	if (IsAffect(AFFECT_INVISIBILITY))
		return true;
#endif

	return false;
}

BOOL CInstanceBase::IsParalysis()
{
	return m_GraphicThingInstance.IsParalysis();
}

BOOL CInstanceBase::IsGameMaster()
{
	if (m_kAffectFlagContainer.IsSet(AFFECT_YMIR))
		return true;
	return false;
}

BOOL CInstanceBase::IsSameEmpire(CInstanceBase& rkInstDst)
{
	if (0 == rkInstDst.m_dwEmpireID)
		return TRUE;

	if (IsGameMaster())
		return TRUE;

	if (rkInstDst.IsGameMaster())
		return TRUE;

	if (rkInstDst.m_dwEmpireID == m_dwEmpireID)
		return TRUE;

	return FALSE;
}

DWORD CInstanceBase::GetEmpireID()
{
	return m_dwEmpireID;
}

DWORD CInstanceBase::GetGuildID()
{
	return m_dwGuildID;
}

#if defined(ENABLE_MOB_AGGR_LVL_INFO)
DWORD CInstanceBase::GetAIFlag()
{
	return m_dwAIFlag;
}
#endif

#ifdef ENABLE_SKILL_COLOR_SYSTEM
DWORD* CInstanceBase::GetSkillColor(DWORD dwSkillIndex)
{
	DWORD dwSkillSlot = dwSkillIndex + 1;
	CPythonSkill::SSkillData* c_pSkillData;
	if (!CPythonSkill::Instance().GetSkillData(dwSkillSlot, &c_pSkillData))
		return 0;

	WORD dwEffectID = c_pSkillData->GradeData[CPythonSkill::SKILL_GRADE_COUNT].wMotionIndex - CRaceMotionData::NAME_SKILL - (1 * 25);
	return m_GraphicThingInstance.GetSkillColorByMotionID(dwEffectID);
}
#endif

int CInstanceBase::GetAlignment()
{
	return m_sAlignment;
}

UINT CInstanceBase::GetAlignmentGrade()
{ // !! gamenin aynisi
// #ifdef ENABLE_ALIGNMENT_SYSTEM
	int iAlignIndex;

	int m_Real_align = m_sAlignment * 10;

	if (m_Real_align >= 200000000)
		iAlignIndex = 0;
	else if (m_Real_align >= 120000000)
		iAlignIndex = 1;
	else if (m_Real_align >= 110000000)
		iAlignIndex = 2;
	else if (m_Real_align >= 100000000)
		iAlignIndex = 3;
	else if (m_Real_align >= 90000000)
		iAlignIndex = 4;
	else if (m_Real_align >= 80000000)
		iAlignIndex = 5;
	else if (m_Real_align >= 70000000)
		iAlignIndex = 6;
	else if (m_Real_align >= 60000000)
		iAlignIndex = 7;
	else if (m_Real_align >= 50000000)
		iAlignIndex = 8;
	else if (m_Real_align >= 40000000)
		iAlignIndex = 9;
	else if (m_Real_align >= 30000000)
		iAlignIndex = 10;
	else if (m_Real_align >= 20000000)
		iAlignIndex = 11;
	else if (m_Real_align >= 10000000)
		iAlignIndex = 12;
	else if (m_Real_align >= 2000000)
		iAlignIndex = 13;
	else if (m_Real_align >= 1800000)
		iAlignIndex = 14;
	else if (m_Real_align >= 1600000)
		iAlignIndex = 15;
	else if (m_Real_align >= 1400000)
		iAlignIndex = 16;
	else if (m_Real_align >= 1200000)
		iAlignIndex = 17;
	else if (m_Real_align >= 1000000)
		iAlignIndex = 18;
	else if (m_Real_align >= 900000)
		iAlignIndex = 19;
	else if (m_Real_align >= 800000)
		iAlignIndex = 20;
	else if (m_Real_align >= 700000)
		iAlignIndex = 21;
	else if (m_Real_align >= 600000)
		iAlignIndex = 22;
	else if (m_Real_align >= 500000)
		iAlignIndex = 23;
	else if (m_Real_align >= 400000)
		iAlignIndex = 24;
	else if (m_Real_align >= 300000)
		iAlignIndex = 25;
	else if (m_Real_align >= 200000)
		iAlignIndex = 26;
	else if (m_Real_align >= 120000)
		iAlignIndex = 27;
	else if (m_Real_align >= 80000)
		iAlignIndex = 28;
	else if (m_Real_align >= 40000)
		iAlignIndex = 29;
	else if (m_Real_align >= 10000)
		iAlignIndex = 30;
	else if (m_Real_align >= 0)
		iAlignIndex = 31;
	else if (m_Real_align > -40000)
		iAlignIndex = 32;
	else if (m_Real_align > -80000)
		iAlignIndex = 33;
	// else if (m_Real_align > -120000)
		// iAlignIndex = 34;
	else
		iAlignIndex = 34;

	return iAlignIndex;
}

int CInstanceBase::GetAlignmentType()
{
	switch (GetAlignmentGrade())
	{
#ifdef ENABLE_ALIGNMENT_SYSTEM
	case 0:
	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
	case 8:
	case 9:
	case 10:
	case 11:
	case 12:
	case 13:
	case 14:
	case 15:
	case 16:
	case 17:
	case 18:
	case 19:
	case 20:
	case 21:
	case 22:
	case 23:
	case 24:
	case 25:
	case 26:
	case 27:
	case 28:
	case 29:
	case 30:
	{
		return ALIGNMENT_TYPE_WHITE;
		break;
	}

	case 32:
	case 33:
	case 34:
	{
		return ALIGNMENT_TYPE_DARK;
		break;
	}
#else
	case 0:
	case 1:
	case 2:
	case 3:
	{
		return ALIGNMENT_TYPE_WHITE;
		break;
	}

	case 5:
	case 6:
	case 7:
	case 8:
	{
		return ALIGNMENT_TYPE_DARK;
		break;
	}
#endif
	}

	return ALIGNMENT_TYPE_NORMAL;
}

#ifdef ENABLE_RANK_SYSTEM
int CInstanceBase::GetRank()
{
	return m_sRank;
}
#endif

#ifdef ENABLE_TEAM_SYSTEM
int CInstanceBase::GetTeam()
{
	return m_sTeam;
}
#endif

#ifdef ENABLE_LANDRANK_SYSTEM
int CInstanceBase::GetLandRank()
{
	return m_sLandRank;
}
#endif

#ifdef ENABLE_REBORN_SYSTEM
int CInstanceBase::GetReborn()
{
	return m_sReborn;
}
#endif

#ifdef ENABLE_MONIKER_SYSTEM
const char * CInstanceBase::GetMoniker()
{
	return m_Moniker.c_str();
}
#endif

#ifdef ENABLE_LOVE_SYSTEM
const char* CInstanceBase::GetLove1()
{
	return m_Love1.c_str();
}

const char* CInstanceBase::GetLove2()
{
	return m_Love2.c_str();
}
#endif // ENABLE_LOVE_SYSTEM

#ifdef ENABLE_WORD_SYSTEM
const char* CInstanceBase::GetWord()
{
	return m_word.c_str();
}
#endif // ENABLE_WORD_SYSTEM


BYTE CInstanceBase::GetPKMode()
{
	return m_byPKMode;
}

bool CInstanceBase::IsKiller()
{
	return m_isKiller;
}

bool CInstanceBase::IsPartyMember()
{
	return m_isPartyMember;
}

BOOL CInstanceBase::IsInSafe()
{
	const TPixelPosition& c_rkPPosCur = m_GraphicThingInstance.NEW_GetCurPixelPositionRef();
	if (CPythonBackground::Instance().isAttrOn(c_rkPPosCur.x, c_rkPPosCur.y, CTerrainImpl::ATTRIBUTE_BANPK))
		return TRUE;

	return FALSE;
}

float CInstanceBase::CalculateDistanceSq3d(const TPixelPosition& c_rkPPosDst)
{
	const TPixelPosition& c_rkPPosSrc = m_GraphicThingInstance.NEW_GetCurPixelPositionRef();
	return SPixelPosition_CalculateDistanceSq3d(c_rkPPosSrc, c_rkPPosDst);
}

void CInstanceBase::OnSelected()
{
	if (IsStoneDoor())
		return;

	if (IsDead())
		return;

	__AttachSelectEffect();
}

void CInstanceBase::OnUnselected()
{
	__DetachSelectEffect();
}

void CInstanceBase::OnTargeted()
{
	if (IsStoneDoor())
		return;

	if (IsDead())
		return;

	__AttachTargetEffect();
}

void CInstanceBase::OnUntargeted()
{
	__DetachTargetEffect();
}

void CInstanceBase::DestroySystem()
{
	ms_kPool.Clear();
}

void CInstanceBase::CreateSystem(UINT uCapacity)
{
	ms_kPool.Create(uCapacity);

	memset(ms_adwCRCAffectEffect, 0, sizeof(ms_adwCRCAffectEffect));

	ms_fDustGap = 250.0f;
	ms_fHorseDustGap = 500.0f;
}

CInstanceBase* CInstanceBase::New()
{
	return ms_kPool.Alloc();
}

void CInstanceBase::Delete(CInstanceBase* pkInst)
{
	pkInst->Destroy();
	ms_kPool.Free(pkInst);
}

void CInstanceBase::SetMainInstance()
{
	CPythonCharacterManager& rkChrMgr = CPythonCharacterManager::Instance();

	DWORD dwVID = GetVirtualID();
	rkChrMgr.SetMainInstance(dwVID);

	m_GraphicThingInstance.SetMainInstance();
}

CInstanceBase* CInstanceBase::__GetMainInstancePtr()
{
	CPythonCharacterManager& rkChrMgr = CPythonCharacterManager::Instance();
	return rkChrMgr.GetMainInstancePtr();
}

void CInstanceBase::__ClearMainInstance()
{
	CPythonCharacterManager& rkChrMgr = CPythonCharacterManager::Instance();
	rkChrMgr.ClearMainInstance();
}

bool CInstanceBase::__IsMainInstance()
{
	if (this == __GetMainInstancePtr())
		return true;

	return false;
}

bool CInstanceBase::__IsExistMainInstance()
{
	if (__GetMainInstancePtr())
		return true;
	else
		return false;
}

bool CInstanceBase::__MainCanSeeHiddenThing()
{
#ifdef ENABLE_CANSEEHIDDENTHING_FOR_GM
	CInstanceBase* pInstance = __GetMainInstancePtr();
	return (pInstance) ? TRUE == pInstance->IsGameMaster() : false;
#else
	return false;
#endif
}

float CInstanceBase::__GetBowRange()
{
	float fRange = 2500.0f - 100.0f;

	if (__IsMainInstance())
	{
		IAbstractPlayer& rPlayer = IAbstractPlayer::GetSingleton();
		fRange += float(rPlayer.GetStatus(POINT_BOW_DISTANCE));
	}

	return fRange;
}

CInstanceBase* CInstanceBase::__FindInstancePtr(DWORD dwVID)
{
	CPythonCharacterManager& rkChrMgr = CPythonCharacterManager::Instance();
	return rkChrMgr.GetInstancePtr(dwVID);
}

bool CInstanceBase::__FindRaceType(DWORD dwRace, BYTE* pbType)
{
	CPythonNonPlayer& rkNonPlayer = CPythonNonPlayer::Instance();
	return rkNonPlayer.GetInstanceType(dwRace, pbType);
}

bool CInstanceBase::Create(const SCreateData& c_rkCreateData)
{
	IAbstractApplication::GetSingleton().SkipRenderBuffering(300);

	SetInstanceType(c_rkCreateData.m_bType);

	DWORD dwRace = c_rkCreateData.m_dwRace;

	if (!SetRace(dwRace))
		return false;

#ifdef ENABLE_DOG_MODE
	SetOriginalRace(dwRace);
#endif

#ifdef ENABLE_BOSS_ON_MINIMAP
	if (!IsPC())
		m_GraphicThingInstance.SetRaceFlag(CPythonNonPlayer::Instance().GetMonsterRaceFlag(dwRace));
#endif

	SetVirtualID(c_rkCreateData.m_dwVID);

	if (c_rkCreateData.m_isMain)
		SetMainInstance();

	if (IsGuildWall())
	{
		unsigned center_x;
		unsigned center_y;

		c_rkCreateData.m_kAffectFlags.ConvertToPosition(&center_x, &center_y);

		float center_z = __GetBackgroundHeight(center_x, center_y);
		NEW_SetPixelPosition(TPixelPosition(float(c_rkCreateData.m_lPosX), float(c_rkCreateData.m_lPosY), center_z));
	}
	else
	{
		SCRIPT_SetPixelPosition(float(c_rkCreateData.m_lPosX), float(c_rkCreateData.m_lPosY));
	}

#if defined(ENABLE_ITEM_EVOLUTION_SYSTEM) && defined(ENABLE_ARMOR_EVOLUTION_SYSTEM)
	SetArmor(c_rkCreateData.m_dwArmor, c_rkCreateData.m_wArmorEvolution);
#else
	SetArmor(c_rkCreateData.m_dwArmor);
#endif

	if (IsPC())
	{
		if (0 != c_rkCreateData.m_dwMountVnum)
			MountHorse(c_rkCreateData.m_dwMountVnum);

#ifdef ENABLE_SHINING_ITEM_SYSTEM
		for (int s = 0; s < CHR_SHINING_NUM; s++)
			SetShining(s, c_rkCreateData.m_dwShining[s]);
#endif

		SetHair(c_rkCreateData.m_dwHair);
#ifdef ENABLE_ITEM_EVOLUTION_SYSTEM
		SetWeapon(c_rkCreateData.m_dwWeapon, c_rkCreateData.m_wWeaponEvolution);
#else
		SetWeapon(c_rkCreateData.m_dwWeapon);
#endif
#ifdef ENABLE_ACCE_SYSTEM
		SetAcce(c_rkCreateData.m_dwAcce, c_rkCreateData.m_dwAcceEffect);
#endif
#ifdef ENABLE_QUIVER_SYSTEM
		SetArrow(c_rkCreateData.m_dwQuiver);
#endif
#ifdef ENABLE_AURA_COSTUME_SYSTEM
		SetAura(c_rkCreateData.m_dwAura);
#endif
#ifdef ENABLE_RANK_SYSTEM
		SetRank(c_rkCreateData.m_sRank);
#endif
#ifdef ENABLE_LANDRANK_SYSTEM
		SetLandRank(c_rkCreateData.m_sLandRank);
#endif
#ifdef ENABLE_REBORN_SYSTEM
		SetReborn(c_rkCreateData.m_sReborn);
#endif
#ifdef ENABLE_TEAM_SYSTEM
		SetTeam(c_rkCreateData.m_sTeam);
#endif
#ifdef ENABLE_MONIKER_SYSTEM
		SetMoniker(c_rkCreateData.m_Moniker.c_str());
#endif
#ifdef ENABLE_LOVE_SYSTEM
		SetLove1(c_rkCreateData.m_Love1.c_str());
		SetLove2(c_rkCreateData.m_Love2.c_str());
#endif // ENABLE_LOVE_SYSTEM
#ifdef ENABLE_WORD_SYSTEM
		SetWord(c_rkCreateData.m_word.c_str());
#endif // ENABLE_WORD_SYSTEM
#ifdef ENABLE_SKILL_COLOR_SYSTEM
		ChangeSkillColor(*c_rkCreateData.m_dwSkillColor);
		memcpy(m_dwSkillColor, *c_rkCreateData.m_dwSkillColor, sizeof(m_dwSkillColor));
#endif
#ifdef ENABLE_SHINING_ITEM_SYSTEM
		if (!AttachShiningEffect(c_rkCreateData.m_dwWeapon))
			TraceError("Error on shining attachment!");
#endif
#ifdef ENABLE_GUILD_LEADER_SYSTEM
		m_bMemberType = c_rkCreateData.m_bMemberType;
#endif
#ifdef ENABLE_ITEM_EVOLUTION_SYSTEM
		m_dwWeaponEvolution = c_rkCreateData.m_wWeaponEvolution;
#ifdef ENABLE_ARMOR_EVOLUTION_SYSTEM
		m_dwArmorEvolution = c_rkCreateData.m_wArmorEvolution;
#endif
#endif

		m_dwGuildID = c_rkCreateData.m_dwGuildID;
		m_dwEmpireID = c_rkCreateData.m_dwEmpireID;

	}

	__Create_SetName(c_rkCreateData);

#ifdef ENABLE_GROWTH_PET_SYSTEM
	#ifdef ENABLE_GROWTH_MOUNT_SYSTEM
		if (IsEnemy() || IsStone() || IsNewPet() || IsNewExMount() || IsPC())
	#else
		if (IsEnemy() || IsStone() || IsNewPet() || IsPC())
	#endif
#else
	#ifdef ENABLE_GROWTH_MOUNT_SYSTEM
		if (IsEnemy() || IsStone() || IsPC() || IsNewExMount())
	#else
		if (IsEnemy() || IsStone() || IsPC())
	#endif
#endif
	{
		m_dwLevel = c_rkCreateData.m_bLevel;
	}

#if defined(ENABLE_MOB_AGGR_LVL_INFO)
	m_dwAIFlag = c_rkCreateData.m_dwAIFlag;
#endif

	SetVirtualNumber(c_rkCreateData.m_dwRace);
	SetRotation(c_rkCreateData.m_fRot);

	SetAlignment(c_rkCreateData.m_alignment);
	SetPKMode(c_rkCreateData.m_byPKMode);
	SetMoveSpeed(c_rkCreateData.m_dwMovSpd);
	SetAttackSpeed(c_rkCreateData.m_dwAtkSpd);

	float fScale = 1.0f;

	//if (IsStone())
	//	fScale = 1.0f + (static_cast<float>(CPythonSystem::Instance().GetStoneSize() / 10.0f));

	if (!IsBoss() && !IsPC() && !IsStone() && !IsPet() && !IsMount() && !IsNPC() && !IsNewPet() && !IsNewExMount())
		fScale = 1.0f - (static_cast<float>(CPythonSystem::Instance().GetMobSize() / 14.0f));

	if (IsNPC() && !IsPC() && !IsStone() && !IsPet() && !IsMount() && !IsNewPet() && !IsNewExMount() && !IsBoss())
		fScale = 2.5f;

	m_GraphicThingInstance.SetScaleWorld(fScale, fScale, fScale);

	m_GraphicThingInstance.SetAlphaValue(0.0f);
	m_GraphicThingInstance.BlendAlphaValue(1.0f, 0.5f);

	if (!IsGuildWall())
	{
		SetAffectFlagContainer(c_rkCreateData.m_kAffectFlags);
	}

	AttachTextTail();
	RefreshTextTail();


	if (c_rkCreateData.m_dwStateFlags & ADD_CHARACTER_STATE_SPAWN)
	{
		if (IsAffect(AFFECT_SPAWN))
			__AttachEffect(EFFECT_SPAWN_APPEAR);

		if (IsPC())
			Refresh(CRaceMotionData::NAME_WAIT, true);
		else
			Refresh(CRaceMotionData::NAME_SPAWN, false);
	}
	else
		Refresh(CRaceMotionData::NAME_WAIT, true);

	__AttachEmpireEffect(c_rkCreateData.m_dwEmpireID);

	RegisterBoundingSphere();

	if (c_rkCreateData.m_dwStateFlags & ADD_CHARACTER_STATE_DEAD)
		m_GraphicThingInstance.DieEnd();

	SetStateFlags(c_rkCreateData.m_dwStateFlags);

	m_GraphicThingInstance.SetBattleHitEffect(ms_adwCRCAffectEffect[EFFECT_HIT]);

	if (!IsPC())
	{
		DWORD dwBodyColor = CPythonNonPlayer::Instance().GetMonsterColor(dwRace);
		if (0 != dwBodyColor)
		{
			SetModulateRenderMode();
			SetAddColor(dwBodyColor);
		}
	}

	__AttachHorseSaddle();

#ifdef ENABLE_CANSEEHIDDENTHING_FOR_GM
	if (IsAffect(AFFECT_INVISIBILITY) && __MainCanSeeHiddenThing())
		m_GraphicThingInstance.BlendAlphaValue(0.5f, 0.5f);
#endif

	return true;
}

#ifdef ENABLE_SKILL_COLOR_SYSTEM
void CInstanceBase::ChangeSkillColor(const DWORD* dwSkillColor)
{
	DWORD tmpdwSkillColor[ESkillColorLength::MAX_SKILL_COUNT + MAX_BUFF_COUNT][ESkillColorLength::MAX_EFFECT_COUNT];
	memcpy(tmpdwSkillColor, dwSkillColor, sizeof(tmpdwSkillColor));

	DWORD skill[CRaceMotionData::SKILL_NUM][ESkillColorLength::MAX_EFFECT_COUNT];
	memset(skill, 0, sizeof(skill));

	for (int i = 0; i < 8; ++i) // i = skillGroup
	{
		for (int t = 0; t < ESkillColorLength::MAX_SKILL_COUNT; ++t) // t = skill
		{
			for (int x = 0; x < ESkillColorLength::MAX_EFFECT_COUNT; ++x) // x = layers
			{
				skill[i * 10 + i * (ESkillColorLength::MAX_SKILL_COUNT - 1) + t + 1][x] = *(dwSkillColor++);
			}
		}
#ifdef ENABLE_WOLFMAN_CHARACTER
		for (int i = 170; i < 170 + ESkillColorLength::MAX_SKILL_COUNT; i++)
		{
			for (int x = 0; x < ESkillColorLength::MAX_EFFECT_COUNT; ++x)
				skill[i][x] = tmpdwSkillColor[i - 170][x];
		}
#endif
		dwSkillColor -= ESkillColorLength::MAX_SKILL_COUNT * ESkillColorLength::MAX_EFFECT_COUNT;
	}

	for (int i = BUFF_BEGIN; i < MAX_SKILL_COUNT + MAX_BUFF_COUNT; i++)
	{
		// Buffs = 94 95 96 110 111
		BYTE id = 0;
		switch (i)
		{
		case BUFF_BEGIN + 0:
			id = 94;
			break;
		case BUFF_BEGIN + 1:
			id = 95;
			break;
		case BUFF_BEGIN + 2:
			id = 96;
			break;
		case BUFF_BEGIN + 3:
			id = 110;
			break;
		case BUFF_BEGIN + 4:
			id = 111;
			break;
#ifdef ENABLE_WOLFMAN_CHARACTER
		case BUFF_BEGIN + 5:
			id = 175;
			break;
#endif
		default:
			break;
		}

		if (id == 0)
			continue;

		for (int x = 0; x < ESkillColorLength::MAX_EFFECT_COUNT; ++x)
			skill[id][x] = tmpdwSkillColor[i][x];
	}

	m_GraphicThingInstance.ChangeSkillColor(*skill);
}
#endif

void CInstanceBase::__Create_SetName(const SCreateData& c_rkCreateData)
{
	if (IsGoto())
	{
		SetNameString("", 0);
		return;
	}

	if (IsWarp())
	{
		__Create_SetWarpName(c_rkCreateData);
		return;
	}

	SetNameString(c_rkCreateData.m_stName.c_str(), c_rkCreateData.m_stName.length());
}

void CInstanceBase::__Create_SetWarpName(const SCreateData& c_rkCreateData)
{
	const char* c_szName;

	if (CPythonNonPlayer::Instance().GetName(c_rkCreateData.m_dwRace, &c_szName))
	{
		std::string strName = c_szName;
		int iFindingPos = strName.find_first_of(" ", 0);
		if (iFindingPos > 0)
		{
			strName.resize(iFindingPos);
		}
		SetNameString(strName.c_str(), strName.length());
	}
	else
		SetNameString(c_rkCreateData.m_stName.c_str(), c_rkCreateData.m_stName.length());
}

void CInstanceBase::SetNameString(const char* c_szName, int len)
{
	m_stName.assign(c_szName, len);
}

#ifdef ENABLE_CHARACTER_WINDOW_RENEWAL
bool CInstanceBase::SetRace(DWORD eRace, bool isIntro)
#else
bool CInstanceBase::SetRace(DWORD eRace)
#endif
{
	m_dwRace = eRace;

#ifdef ENABLE_CHARACTER_WINDOW_RENEWAL
	if (!m_GraphicThingInstance.SetRace(eRace, isIntro))
#else
	if (!m_GraphicThingInstance.SetRace(eRace))
#endif
		return false;

	if (!__FindRaceType(m_dwRace, &m_eRaceType))
		m_eRaceType = CActorInstance::TYPE_PC;

	return true;
}

BOOL CInstanceBase::IsHoldingPickAxe()
{
	const int c_iPickAxeStart = 29101;
	const int c_iPickAxeEnd = 29110;
	return m_awPart[CRaceData::PART_WEAPON] >= c_iPickAxeStart && m_awPart[CRaceData::PART_WEAPON] <= c_iPickAxeEnd;
}

BOOL CInstanceBase::IsMountingHorse()
{
	return m_kHorse.IsMounting();
}

void CInstanceBase::MountHorse(UINT eRace)
{
	m_kHorse.Destroy();
	m_kHorse.Create(m_GraphicThingInstance.NEW_GetCurPixelPositionRef(), eRace, ms_adwCRCAffectEffect[EFFECT_HIT]);

	SetMotionMode(CRaceMotionData::MODE_HORSE);
	SetRotationSpeed(c_fDefaultHorseRotationSpeed);

	m_GraphicThingInstance.MountHorse(m_kHorse.GetActorPtr());
	m_GraphicThingInstance.Stop();
	m_GraphicThingInstance.RefreshActorInstance();
}

void CInstanceBase::DismountHorse()
{
	m_kHorse.Destroy();
}

void CInstanceBase::GetInfo(std::string* pstInfo)
{
	char szInfo[256];
	sprintf(szInfo, "Inst - UC %d, RC %d Pool - %d ",
		ms_dwUpdateCounter,
		ms_dwRenderCounter,
		ms_kPool.GetCapacity()
	);

	pstInfo->append(szInfo);
}

void CInstanceBase::ResetPerformanceCounter()
{
	ms_dwUpdateCounter = 0;
	ms_dwRenderCounter = 0;
	ms_dwDeformCounter = 0;
}

bool CInstanceBase::NEW_IsLastPixelPosition()
{
	return m_GraphicThingInstance.IsPushing();
}

const TPixelPosition& CInstanceBase::NEW_GetLastPixelPositionRef()
{
	return m_GraphicThingInstance.NEW_GetLastPixelPositionRef();
}

void CInstanceBase::NEW_SetDstPixelPositionZ(FLOAT z)
{
	m_GraphicThingInstance.NEW_SetDstPixelPositionZ(z);
}

void CInstanceBase::NEW_SetDstPixelPosition(const TPixelPosition& c_rkPPosDst)
{
	m_GraphicThingInstance.NEW_SetDstPixelPosition(c_rkPPosDst);
}

void CInstanceBase::NEW_SetSrcPixelPosition(const TPixelPosition& c_rkPPosSrc)
{
	m_GraphicThingInstance.NEW_SetSrcPixelPosition(c_rkPPosSrc);
}

const TPixelPosition& CInstanceBase::NEW_GetCurPixelPositionRef()
{
	return m_GraphicThingInstance.NEW_GetCurPixelPositionRef();
}

const TPixelPosition& CInstanceBase::NEW_GetDstPixelPositionRef()
{
	return m_GraphicThingInstance.NEW_GetDstPixelPositionRef();
}

const TPixelPosition& CInstanceBase::NEW_GetSrcPixelPositionRef()
{
	return m_GraphicThingInstance.NEW_GetSrcPixelPositionRef();
}

void CInstanceBase::OnSyncing()
{
	m_GraphicThingInstance.__OnSyncing();
}

void CInstanceBase::OnWaiting()
{
	m_GraphicThingInstance.__OnWaiting();
}

void CInstanceBase::OnMoving()
{
	m_GraphicThingInstance.__OnMoving();
}

void CInstanceBase::ChangeGuild(DWORD dwGuildID)
{
	m_dwGuildID = dwGuildID;

	DetachTextTail();
	AttachTextTail();
	RefreshTextTail();
}

DWORD CInstanceBase::GetPart(CRaceData::EParts part)
{
	assert(part >= 0 && part < CRaceData::PART_MAX_NUM);
	return m_awPart[part];
}

DWORD CInstanceBase::GetShape()
{
	return m_eShape;
}

bool CInstanceBase::CanAct()
{
	return m_GraphicThingInstance.CanAct();
}

bool CInstanceBase::CanMove()
{
	return m_GraphicThingInstance.CanMove();
}

bool CInstanceBase::CanUseSkill()
{
	if (IsHoldingPickAxe())
		return false;

	if (!m_kHorse.CanUseSkill())
		return false;

	if (!m_GraphicThingInstance.CanUseSkill())
		return false;

	return true;
}

bool CInstanceBase::CanAttack()
{
	if (!m_kHorse.CanAttack())
		return false;

	if (IsHoldingPickAxe())
		return false;

	return m_GraphicThingInstance.CanAttack();
}

bool CInstanceBase::CanFishing()
{
	return m_GraphicThingInstance.CanFishing();
}

BOOL CInstanceBase::IsBowMode()
{
	return m_GraphicThingInstance.IsBowMode();
}

BOOL CInstanceBase::IsHandMode()
{
	return m_GraphicThingInstance.IsHandMode();
}

BOOL CInstanceBase::IsFishingMode()
{
	if (CRaceMotionData::MODE_FISHING == m_GraphicThingInstance.GetMotionMode())
		return true;

	return false;
}

BOOL CInstanceBase::IsFishing()
{
	return m_GraphicThingInstance.IsFishing();
}

BOOL CInstanceBase::IsDead()
{
	return m_GraphicThingInstance.IsDead();
}

BOOL CInstanceBase::IsStun()
{
	return m_GraphicThingInstance.IsStun();
}

BOOL CInstanceBase::IsSleep()
{
	return m_GraphicThingInstance.IsSleep();
}

BOOL CInstanceBase::__IsSyncing()
{
	return m_GraphicThingInstance.__IsSyncing();
}

void CInstanceBase::NEW_SetOwner(DWORD dwVIDOwner)
{
	m_GraphicThingInstance.SetOwner(dwVIDOwner);
}

float CInstanceBase::GetLocalTime()
{
	return m_GraphicThingInstance.GetLocalTime();
}

DWORD	ELTimer_GetServerFrameMSec();

void CInstanceBase::PushTCPStateExpanded(DWORD dwCmdTime, const TPixelPosition& c_rkPPosDst, float fDstRot, UINT eFunc, UINT uArg, UINT uTargetVID)
{
	SCommand kCmdNew;
	kCmdNew.m_kPPosDst = c_rkPPosDst;
	kCmdNew.m_dwChkTime = dwCmdTime + 100;
	kCmdNew.m_dwCmdTime = dwCmdTime;
	kCmdNew.m_fDstRot = fDstRot;
	kCmdNew.m_eFunc = eFunc;
	kCmdNew.m_uArg = uArg;
	kCmdNew.m_uTargetVID = uTargetVID;
	m_kQue_kCmdNew.push_back(kCmdNew);
}

void CInstanceBase::PushTCPState(DWORD dwCmdTime, const TPixelPosition& c_rkPPosDst, float fDstRot, UINT eFunc, UINT uArg)
{
	if (__IsMainInstance())
	{
		TraceError("CInstanceBase::PushTCPState You can't send move packets to yourself!");
		return;
	}

	int nNetworkGap = ELTimer_GetServerFrameMSec() - dwCmdTime;

	m_nAverageNetworkGap = (m_nAverageNetworkGap * 70 + nNetworkGap * 30) / 100;

	SCommand kCmdNew;
	kCmdNew.m_kPPosDst = c_rkPPosDst;
	kCmdNew.m_dwChkTime = dwCmdTime + m_nAverageNetworkGap;//m_dwBaseChkTime + (dwCmdTime - m_dwBaseCmdTime);// + nNetworkGap;
	kCmdNew.m_dwCmdTime = dwCmdTime;
	kCmdNew.m_fDstRot = fDstRot;
	kCmdNew.m_eFunc = eFunc;
	kCmdNew.m_uArg = uArg;
	m_kQue_kCmdNew.push_back(kCmdNew);
}

BOOL CInstanceBase::__CanProcessNetworkStatePacket()
{
	if (m_GraphicThingInstance.IsDead())
		return FALSE;

	if (m_GraphicThingInstance.IsKnockDown())
		return FALSE;

	if (m_GraphicThingInstance.IsUsingSkill())
		if (!m_GraphicThingInstance.CanCancelSkill())
			return FALSE;

	return TRUE;
}

BOOL CInstanceBase::__IsEnableTCPProcess(UINT eCurFunc)
{
	if (m_GraphicThingInstance.IsActEmotion())
		return FALSE;

	if (!m_bEnableTCPState)
	{
		if (FUNC_EMOTION != eCurFunc)
			return FALSE;
	}

	return TRUE;
}

void CInstanceBase::StateProcess()
{
	while (true)
	{
		if (m_kQue_kCmdNew.empty())
			return;

		DWORD dwDstChkTime = m_kQue_kCmdNew.front().m_dwChkTime;
		DWORD dwCurChkTime = ELTimer_GetServerFrameMSec();

		if (dwCurChkTime < dwDstChkTime)
			return;

		SCommand kCmdTop = m_kQue_kCmdNew.front();
		m_kQue_kCmdNew.pop_front();

		TPixelPosition kPPosDst = kCmdTop.m_kPPosDst;
		FLOAT fRotDst = kCmdTop.m_fDstRot;
		UINT eFunc = kCmdTop.m_eFunc;
		UINT uArg = kCmdTop.m_uArg;
		UINT uVID = GetVirtualID();
		UINT uTargetVID = kCmdTop.m_uTargetVID;

		TPixelPosition kPPosCur;
		NEW_GetPixelPosition(&kPPosCur);

		TPixelPosition kPPosDir = kPPosDst - kPPosCur;
		float fDirLen = (float)sqrt(kPPosDir.x * kPPosDir.x + kPPosDir.y * kPPosDir.y);

		if (!__CanProcessNetworkStatePacket())
		{
			Lognf(0, "vid=%d Skip State as unable to process IsDead=%d, IsKnockDown=%d", uVID, m_GraphicThingInstance.IsDead(), m_GraphicThingInstance.IsKnockDown());
			return;
		}

		if (!__IsEnableTCPProcess(eFunc))
			return;

		switch (eFunc)
		{
		case FUNC_WAIT:
		{
			if (fDirLen > 1.0f)
			{
				NEW_SetSrcPixelPosition(kPPosCur);
				NEW_SetDstPixelPosition(kPPosDst);

				__EnableSkipCollision();

				m_fDstRot = fRotDst;
				m_isGoing = TRUE;

				m_kMovAfterFunc.eFunc = FUNC_WAIT;

				if (!IsWalking())
					StartWalking();
			}
			else
			{
				m_isGoing = FALSE;

				if (!IsWaiting())
					EndWalking();

				SCRIPT_SetPixelPosition(kPPosDst.x, kPPosDst.y);
				SetAdvancingRotation(fRotDst);
				SetRotation(fRotDst);
			}
			break;
		}

		case FUNC_MOVE:
		{
			NEW_SetSrcPixelPosition(kPPosCur);
			NEW_SetDstPixelPosition(kPPosDst);
			m_fDstRot = fRotDst;
			m_isGoing = TRUE;
			__EnableSkipCollision();

			m_kMovAfterFunc.eFunc = FUNC_MOVE;

			if (!IsWalking())
			{
				StartWalking();
			}
			else
			{
			}
			break;
		}

		case FUNC_COMBO:
		{
			if (fDirLen >= 50.0f)
			{
				NEW_SetSrcPixelPosition(kPPosCur);
				NEW_SetDstPixelPosition(kPPosDst);
				m_fDstRot = fRotDst;
				m_isGoing = TRUE;
				__EnableSkipCollision();

				m_kMovAfterFunc.eFunc = FUNC_COMBO;
				m_kMovAfterFunc.uArg = uArg;

				if (!IsWalking())
					StartWalking();
			}
			else
			{
				m_isGoing = FALSE;

				if (IsWalking())
					EndWalking();

				SCRIPT_SetPixelPosition(kPPosDst.x, kPPosDst.y);
				RunComboAttack(fRotDst, uArg);
			}
			break;
		}

		case FUNC_ATTACK:
		{
			if (fDirLen >= 50.0f)
			{
				NEW_SetSrcPixelPosition(kPPosCur);
				NEW_SetDstPixelPosition(kPPosDst);
				m_fDstRot = fRotDst;
				m_isGoing = TRUE;
				__EnableSkipCollision();

				m_kMovAfterFunc.eFunc = FUNC_ATTACK;
				
				if (!IsWalking())
					StartWalking();
			}
			else
			{
				m_isGoing = FALSE;

				if (IsWalking())
					EndWalking();

				SCRIPT_SetPixelPosition(kPPosDst.x, kPPosDst.y);
				BlendRotation(fRotDst);

				RunNormalAttack(fRotDst);
			}
			break;
		}

		case FUNC_MOB_SKILL:
		{
			if (fDirLen >= 50.0f)
			{
				NEW_SetSrcPixelPosition(kPPosCur);
				NEW_SetDstPixelPosition(kPPosDst);
				m_fDstRot = fRotDst;
				m_isGoing = TRUE;
				__EnableSkipCollision();

				m_kMovAfterFunc.eFunc = FUNC_MOB_SKILL;
				m_kMovAfterFunc.uArg = uArg;

				if (!IsWalking())
					StartWalking();
			}
			else
			{
				m_isGoing = FALSE;

				if (IsWalking())
					EndWalking();

				SCRIPT_SetPixelPosition(kPPosDst.x, kPPosDst.y);
				BlendRotation(fRotDst);

				m_GraphicThingInstance.InterceptOnceMotion(CRaceMotionData::NAME_SPECIAL_1 + uArg);
			}
			break;
		}

		case FUNC_EMOTION:
		{
			if (fDirLen > 100.0f)
			{
				NEW_SetSrcPixelPosition(kPPosCur);
				NEW_SetDstPixelPosition(kPPosDst);
				m_fDstRot = fRotDst;
				m_isGoing = TRUE;

				if (__IsMainInstance())
					__EnableSkipCollision();

				m_kMovAfterFunc.eFunc = FUNC_EMOTION;
				m_kMovAfterFunc.uArg = uArg;
				m_kMovAfterFunc.uArgExpanded = uTargetVID;
				m_kMovAfterFunc.kPosDst = kPPosDst;

				if (!IsWalking())
					StartWalking();
			}
			else
				__ProcessFunctionEmotion(uArg, uTargetVID, kPPosDst);
			break;
		}

		default:
		{
			if (eFunc & FUNC_SKILL)
			{
				if (fDirLen >= 50.0f)
				{
					NEW_SetSrcPixelPosition(kPPosCur);
					NEW_SetDstPixelPosition(kPPosDst);
					m_fDstRot = fRotDst;
					m_isGoing = TRUE;
					__EnableSkipCollision();

					m_kMovAfterFunc.eFunc = eFunc;
					m_kMovAfterFunc.uArg = uArg;

					if (!IsWalking())
						StartWalking();
				}
				else
				{
					m_isGoing = FALSE;

					if (IsWalking())
						EndWalking();

					SCRIPT_SetPixelPosition(kPPosDst.x, kPPosDst.y);
					SetAdvancingRotation(fRotDst);
					SetRotation(fRotDst);
					NEW_UseSkill(0, eFunc & 0x7f, uArg & 0x0f, (uArg >> 4) ? true : false);
				}
			}
			break;
		}
		}
	}
}

void CInstanceBase::MovementProcess()
{
	TPixelPosition kPPosCur;
	NEW_GetPixelPosition(&kPPosCur);

	TPixelPosition kPPosNext;
	{
		const D3DXVECTOR3& c_rkV3Mov = m_GraphicThingInstance.GetMovementVectorRef();

		kPPosNext.x = kPPosCur.x + (+c_rkV3Mov.x);
		kPPosNext.y = kPPosCur.y + (-c_rkV3Mov.y);
		kPPosNext.z = kPPosCur.z + (+c_rkV3Mov.z);
	}

	TPixelPosition kPPosDeltaSC = kPPosCur - NEW_GetSrcPixelPositionRef();
	TPixelPosition kPPosDeltaSN = kPPosNext - NEW_GetSrcPixelPositionRef();
	TPixelPosition kPPosDeltaSD = NEW_GetDstPixelPositionRef() - NEW_GetSrcPixelPositionRef();

	float fCurLen = sqrtf(kPPosDeltaSC.x * kPPosDeltaSC.x + kPPosDeltaSC.y * kPPosDeltaSC.y);
	float fNextLen = sqrtf(kPPosDeltaSN.x * kPPosDeltaSN.x + kPPosDeltaSN.y * kPPosDeltaSN.y);
	float fTotalLen = sqrtf(kPPosDeltaSD.x * kPPosDeltaSD.x + kPPosDeltaSD.y * kPPosDeltaSD.y);
	float fRestLen = fTotalLen - fCurLen;

	if (__IsMainInstance())
	{
		if (m_isGoing && IsWalking())
		{
			float fDstRot = NEW_GetAdvancingRotationFromPixelPosition(NEW_GetSrcPixelPositionRef(), NEW_GetDstPixelPositionRef());

			SetAdvancingRotation(fDstRot);

			if (fRestLen <= 0.0)
			{
				if (IsWalking())
					EndWalking();

				m_isGoing = FALSE;

				BlockMovement();

				if (FUNC_EMOTION == m_kMovAfterFunc.eFunc)
				{
					DWORD dwMotionNumber = m_kMovAfterFunc.uArg;
					DWORD dwTargetVID = m_kMovAfterFunc.uArgExpanded;
					__ProcessFunctionEmotion(dwMotionNumber, dwTargetVID, m_kMovAfterFunc.kPosDst);
					m_kMovAfterFunc.eFunc = FUNC_WAIT;
					return;
				}
			}
		}
	}
	else
	{
		if (m_isGoing && IsWalking())
		{
			float fDstRot = NEW_GetAdvancingRotationFromPixelPosition(NEW_GetSrcPixelPositionRef(), NEW_GetDstPixelPositionRef());

			SetAdvancingRotation(fDstRot);

			if (fRestLen < -100.0f)
			{
				NEW_SetSrcPixelPosition(kPPosCur);

				float fDstRot = NEW_GetAdvancingRotationFromPixelPosition(kPPosCur, NEW_GetDstPixelPositionRef());
				SetAdvancingRotation(fDstRot);

				if (FUNC_MOVE == m_kMovAfterFunc.eFunc)
					m_kMovAfterFunc.eFunc = FUNC_WAIT;
			}
			else if (fCurLen <= fTotalLen && fTotalLen <= fNextLen)
			{
				if (m_GraphicThingInstance.IsDead() || m_GraphicThingInstance.IsKnockDown())
				{
					__DisableSkipCollision();
					m_isGoing = FALSE;
				}
				else
				{
					switch (m_kMovAfterFunc.eFunc)
					{
					case FUNC_ATTACK:
					{
						if (IsWalking())
							EndWalking();

						__DisableSkipCollision();
						m_isGoing = FALSE;

						BlockMovement();
						SCRIPT_SetPixelPosition(NEW_GetDstPixelPositionRef().x, NEW_GetDstPixelPositionRef().y);
						SetAdvancingRotation(m_fDstRot);
						SetRotation(m_fDstRot);

						RunNormalAttack(m_fDstRot);
						break;
					}

					case FUNC_COMBO:
					{
						if (IsWalking())
							EndWalking();

						__DisableSkipCollision();
						m_isGoing = FALSE;

						BlockMovement();
						SCRIPT_SetPixelPosition(NEW_GetDstPixelPositionRef().x, NEW_GetDstPixelPositionRef().y);
						RunComboAttack(m_fDstRot, m_kMovAfterFunc.uArg);
						break;
					}

					case FUNC_EMOTION:
					{
						m_isGoing = FALSE;
						m_kMovAfterFunc.eFunc = FUNC_WAIT;
						__DisableSkipCollision();
						BlockMovement();

						DWORD dwMotionNumber = m_kMovAfterFunc.uArg;
						DWORD dwTargetVID = m_kMovAfterFunc.uArgExpanded;
						__ProcessFunctionEmotion(dwMotionNumber, dwTargetVID, m_kMovAfterFunc.kPosDst);
						break;
					}

					case FUNC_MOVE:
						break;

					case FUNC_MOB_SKILL:
					{
						if (IsWalking())
							EndWalking();

						__DisableSkipCollision();
						m_isGoing = FALSE;

						BlockMovement();
						SCRIPT_SetPixelPosition(NEW_GetDstPixelPositionRef().x, NEW_GetDstPixelPositionRef().y);
						SetAdvancingRotation(m_fDstRot);
						SetRotation(m_fDstRot);

						m_GraphicThingInstance.InterceptOnceMotion(CRaceMotionData::NAME_SPECIAL_1 + m_kMovAfterFunc.uArg);
						break;
					}

					default:
					{
						if (m_kMovAfterFunc.eFunc & FUNC_SKILL)
						{
							SetAdvancingRotation(m_fDstRot);
							BlendRotation(m_fDstRot);
							NEW_UseSkill(0, m_kMovAfterFunc.eFunc & 0x7f, m_kMovAfterFunc.uArg & 0x0f, (m_kMovAfterFunc.uArg >> 4) ? true : false);
						}
						else
						{
							__DisableSkipCollision();
							m_isGoing = FALSE;

							BlockMovement();
							SCRIPT_SetPixelPosition(NEW_GetDstPixelPositionRef().x, NEW_GetDstPixelPositionRef().y);
							SetAdvancingRotation(m_fDstRot);
							BlendRotation(m_fDstRot);
							if (!IsWaiting())
								EndWalking();
						}
						break;
					}
					}
				}
			}
		}
	}

	if (IsWalking() || m_GraphicThingInstance.IsUsingMovingSkill())
	{
		float fRotation = m_GraphicThingInstance.GetRotation();
		float fAdvancingRotation = m_GraphicThingInstance.GetAdvancingRotation();
		int iDirection = GetRotatingDirection(fRotation, fAdvancingRotation);

		if (DEGREE_DIRECTION_SAME != m_iRotatingDirection)
		{
			if (DEGREE_DIRECTION_LEFT == iDirection)
				fRotation = fmodf(fRotation + m_fRotSpd * m_GraphicThingInstance.GetSecondElapsed(), 360.0f);
			else if (DEGREE_DIRECTION_RIGHT == iDirection)
				fRotation = fmodf(fRotation - m_fRotSpd * m_GraphicThingInstance.GetSecondElapsed() + 360.0f, 360.0f);

			if (m_iRotatingDirection != GetRotatingDirection(fRotation, fAdvancingRotation))
			{
				m_iRotatingDirection = DEGREE_DIRECTION_SAME;
				fRotation = fAdvancingRotation;
			}

			m_GraphicThingInstance.SetRotation(fRotation);
		}

		if (__IsInDustRange())
		{
			float fDustDistance = NEW_GetDistanceFromDestPixelPosition(m_kPPosDust);
			if (IsMountingHorse())
			{
				if (fDustDistance > ms_fHorseDustGap)
				{
					NEW_GetPixelPosition(&m_kPPosDust);
					__AttachEffect(EFFECT_HORSE_DUST);
				}
			}
			else
			{
				if (fDustDistance > ms_fDustGap)
				{
					NEW_GetPixelPosition(&m_kPPosDust);
					__AttachEffect(EFFECT_DUST);
				}
			}
		}
	}
}

void CInstanceBase::__ProcessFunctionEmotion(DWORD dwMotionNumber, DWORD dwTargetVID, const TPixelPosition& c_rkPosDst)
{
	if (IsAttacking())
		return;

	if (IsWalking())
		EndWalkingWithoutBlending();

	__EnableChangingTCPState();
	SCRIPT_SetPixelPosition(c_rkPosDst.x, c_rkPosDst.y);

	CInstanceBase* pTargetInstance = CPythonCharacterManager::Instance().GetInstancePtr(dwTargetVID);
	if (pTargetInstance)
	{
		pTargetInstance->__EnableChangingTCPState();

		if (pTargetInstance->IsWalking())
			pTargetInstance->EndWalkingWithoutBlending();

		WORD wMotionNumber1 = HIWORD(dwMotionNumber);
		WORD wMotionNumber2 = LOWORD(dwMotionNumber);

		int src_job = RaceToJob(GetRace());
		int dst_job = RaceToJob(pTargetInstance->GetRace());

		NEW_LookAtDestInstance(*pTargetInstance);
		m_GraphicThingInstance.InterceptOnceMotion(wMotionNumber1 + dst_job);
		m_GraphicThingInstance.SetRotation(m_GraphicThingInstance.GetTargetRotation());
		m_GraphicThingInstance.SetAdvancingRotation(m_GraphicThingInstance.GetTargetRotation());

		pTargetInstance->NEW_LookAtDestInstance(*this);
		pTargetInstance->m_GraphicThingInstance.InterceptOnceMotion(wMotionNumber2 + src_job);
		pTargetInstance->m_GraphicThingInstance.SetRotation(pTargetInstance->m_GraphicThingInstance.GetTargetRotation());
		pTargetInstance->m_GraphicThingInstance.SetAdvancingRotation(pTargetInstance->m_GraphicThingInstance.GetTargetRotation());

		if (pTargetInstance->__IsMainInstance())
		{
			IAbstractPlayer& rPlayer = IAbstractPlayer::GetSingleton();
			rPlayer.EndEmotionProcess();
		}
	}

	if (__IsMainInstance())
	{
		IAbstractPlayer& rPlayer = IAbstractPlayer::GetSingleton();
		rPlayer.EndEmotionProcess();
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Update & Deform & Render

int g_iAccumulationTime = 0;

void CInstanceBase::Update()
{
	++ms_dwUpdateCounter;

	StateProcess();
	m_GraphicThingInstance.PhysicsProcess();
	m_GraphicThingInstance.RotationProcess();
	m_GraphicThingInstance.ComboProcess();
	m_GraphicThingInstance.AccumulationMovement();

	if (m_GraphicThingInstance.IsMovement())
	{
		TPixelPosition kPPosCur;
		NEW_GetPixelPosition(&kPPosCur);

		DWORD dwCurTime = ELTimer_GetFrameMSec();
		{
			m_dwNextUpdateHeightTime = dwCurTime;
			kPPosCur.z = __GetBackgroundHeight(kPPosCur.x, kPPosCur.y);
			NEW_SetPixelPosition(kPPosCur);
		}

		{
			DWORD dwMtrlColor = __GetShadowMapColor(kPPosCur.x, kPPosCur.y);
			m_GraphicThingInstance.SetMaterialColor(dwMtrlColor);
		}
	}

	m_GraphicThingInstance.UpdateAdvancingPointInstance();

	AttackProcess();
	MovementProcess();

	m_GraphicThingInstance.MotionProcess(IsPC());
	if (IsMountingHorse())
		m_kHorse.m_pkActor->HORSE_MotionProcess(FALSE);

	ProcessDamage();
}

void CInstanceBase::Transform()
{
	if (__IsSyncing())
	{
		//OnSyncing();
	}
	else
	{
		if (IsWalking() || m_GraphicThingInstance.IsUsingMovingSkill())
		{
			const D3DXVECTOR3& c_rv3Movment=m_GraphicThingInstance.GetMovementVectorRef();

			float len=(c_rv3Movment.x*c_rv3Movment.x)+(c_rv3Movment.y*c_rv3Movment.y);
			if (len>1.0f)
				OnMoving();
			else
				OnWaiting();
		}
	}

	m_GraphicThingInstance.INSTANCEBASE_Transform();
}

void CInstanceBase::Deform()
{
	if (!__CanRender())
		return;

	++ms_dwDeformCounter;

	m_GraphicThingInstance.INSTANCEBASE_Deform();

	m_kHorse.Deform();
}

void CInstanceBase::RenderTrace()
{
	if (!__CanRender())
		return;

	m_GraphicThingInstance.RenderTrace();
}

void CInstanceBase::Render()
{
	if (!__CanRender())
		return;

	++ms_dwRenderCounter;

	m_kHorse.Render();
	m_GraphicThingInstance.Render();

	if (CActorInstance::IsDirLine())
	{
		if (NEW_GetDstPixelPositionRef().x != 0.0f)
		{
			static CScreen s_kScreen;

			STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
			STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
			STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
			STATEMANAGER.SaveRenderState(D3DRS_ZENABLE, FALSE);
			STATEMANAGER.SetRenderState(D3DRS_FOGENABLE, FALSE);
			STATEMANAGER.SetRenderState(D3DRS_LIGHTING, FALSE);

			TPixelPosition px;
			m_GraphicThingInstance.GetPixelPosition(&px);
			D3DXVECTOR3 kD3DVt3Cur(px.x, px.y, px.z);
			D3DXVECTOR3 kD3DVt3Dest(NEW_GetDstPixelPositionRef().x, -NEW_GetDstPixelPositionRef().y, NEW_GetDstPixelPositionRef().z);

			s_kScreen.SetDiffuseColor(0.0f, 0.0f, 1.0f);
			s_kScreen.RenderLine3d(kD3DVt3Cur.x, kD3DVt3Cur.y, px.z, kD3DVt3Dest.x, kD3DVt3Dest.y, px.z);
			STATEMANAGER.RestoreRenderState(D3DRS_ZENABLE);
			STATEMANAGER.SetRenderState(D3DRS_FOGENABLE, TRUE);
			STATEMANAGER.SetRenderState(D3DRS_LIGHTING, TRUE);
		}
	}
}

void CInstanceBase::RenderToShadowMap()
{
	if (IsDoor())
		return;

	if (!__CanRender())
		return;

	if (!__IsExistMainInstance())
		return;

	CInstanceBase* pkInstMain = __GetMainInstancePtr();

	const float SHADOW_APPLY_DISTANCE = 2500.0f;
	float fDistance = NEW_GetDistanceFromDestInstance(*pkInstMain);
	if (fDistance >= SHADOW_APPLY_DISTANCE)
		return;

	m_GraphicThingInstance.RenderToShadowMap();
}

void CInstanceBase::RenderCollision()
{
	m_GraphicThingInstance.RenderCollisionData();
}

void CInstanceBase::SetVirtualID(DWORD dwVirtualID)
{
	m_GraphicThingInstance.SetVirtualID(dwVirtualID);
}

void CInstanceBase::SetVirtualNumber(DWORD dwVirtualNumber)
{
	m_dwVirtualNumber = dwVirtualNumber;
}

void CInstanceBase::SetInstanceType(int iInstanceType)
{
	m_GraphicThingInstance.SetActorType(iInstanceType);
}

#ifdef ENABLE_ALIGNMENT_SYSTEM
void CInstanceBase::SetAlignment(int sAlignment)
#else
void CInstanceBase::SetAlignment(short sAlignment)
#endif
{
	m_sAlignment = sAlignment;
	RefreshTextTailTitle();
}

#ifdef ENABLE_RANK_SYSTEM
void CInstanceBase::SetRank(short sRank)
{
	m_sRank = sRank;
	RefreshTextTailTitle();
}
#endif

#ifdef ENABLE_TEAM_SYSTEM
void CInstanceBase::SetTeam(short sTeam)
{
	m_sTeam = sTeam;
	RefreshTextTailTitle();
}
#endif

#ifdef ENABLE_LANDRANK_SYSTEM
void CInstanceBase::SetLandRank(short sLandRank)
{
	m_sLandRank = sLandRank;
	RefreshTextTailTitle();
}
#endif

#ifdef ENABLE_REBORN_SYSTEM
void CInstanceBase::SetReborn(short sReborn)
{
	m_sReborn = sReborn;
	RefreshTextTailTitle();
}
#endif

#ifdef ENABLE_MONIKER_SYSTEM
void CInstanceBase::SetMoniker(const char* moniker)
{
	m_Moniker = moniker;
	RefreshTextTailTitle();
}
#endif

#ifdef ENABLE_LOVE_SYSTEM
void CInstanceBase::SetLove1(const char* love1)
{
	m_Love1 = love1;
	RefreshTextTailTitle();
}

void CInstanceBase::SetLove2(const char* love2)
{
	m_Love2 = love2;
	RefreshTextTailTitle();
}
#endif // ENABLE_LOVE_SYSTEM

#ifdef ENABLE_WORD_SYSTEM
void CInstanceBase::SetWord(const char* word)
{
	m_word = word;
	RefreshTextTailTitle();
}
#endif // ENABLE_WORD_SYSTEM


void CInstanceBase::SetLevelText(int sLevel)
{
	m_dwLevel = sLevel;
	UpdateTextTailLevel(sLevel);
}

void CInstanceBase::SetPKMode(BYTE byPKMode)
{
	if (m_byPKMode == byPKMode)
		return;

	m_byPKMode = byPKMode;

	if (__IsMainInstance())
	{
		IAbstractPlayer& rPlayer = IAbstractPlayer::GetSingleton();
		rPlayer.NotifyChangePKMode();
	}
}

void CInstanceBase::SetKiller(bool bFlag)
{
	if (m_isKiller == bFlag)
		return;

	m_isKiller = bFlag;
	RefreshTextTail();
}

void CInstanceBase::SetPartyMemberFlag(bool bFlag)
{
	m_isPartyMember = bFlag;
}

void CInstanceBase::SetStateFlags(DWORD dwStateFlags)
{
	if (dwStateFlags & ADD_CHARACTER_STATE_KILLER)
		SetKiller(TRUE);
	else
		SetKiller(FALSE);

	if (dwStateFlags & ADD_CHARACTER_STATE_PARTY)
		SetPartyMemberFlag(TRUE);
	else
		SetPartyMemberFlag(FALSE);
}

void CInstanceBase::SetComboType(UINT uComboType)
{
	m_GraphicThingInstance.SetComboType(uComboType);
}

const char* CInstanceBase::GetNameString()
{
	return m_stName.c_str();
}

#ifdef ENABLE_EXCHANGE_REWORK
DWORD CInstanceBase::GetLevel()
{
	return m_dwLevel;
}
#endif

void CInstanceBase::SetLevel(DWORD dwLevel)
{
	m_dwLevel = dwLevel;
}

DWORD CInstanceBase::GetRace()
{
	return m_dwRace;
}

#ifdef ENABLE_GUILD_LEADER_SYSTEM
BYTE CInstanceBase::GetGuildMemberType()
{
	return m_bMemberType;
}

void CInstanceBase::SetGuildMemberType(BYTE type)
{
	m_bMemberType = type;
}
#endif

bool CInstanceBase::IsConflictAlignmentInstance(CInstanceBase& rkInstVictim)
{
	if (PK_MODE_PROTECT == rkInstVictim.GetPKMode())
		return false;

	switch (GetAlignmentType())
	{
	case ALIGNMENT_TYPE_NORMAL:
	case ALIGNMENT_TYPE_WHITE:
		if (ALIGNMENT_TYPE_DARK == rkInstVictim.GetAlignmentType())
			return true;
		break;

	case ALIGNMENT_TYPE_DARK:
		if (GetAlignmentType() != rkInstVictim.GetAlignmentType())
			return true;
		break;
	}

	return false;
}

void CInstanceBase::SetDuelMode(DWORD type)
{
	m_dwDuelMode = type;
}

DWORD CInstanceBase::GetDuelMode()
{
	return m_dwDuelMode;
}

bool CInstanceBase::IsAttackableInstance(CInstanceBase& rkInstVictim)
{
	if (__IsMainInstance())
	{
		CPythonPlayer& rkPlayer = CPythonPlayer::Instance();
		if (rkPlayer.IsObserverMode())
			return false;
	}

	if (GetVirtualID() == rkInstVictim.GetVirtualID())
		return false;

	if (IsStone())
	{
		if (rkInstVictim.IsPC())
			return true;
	}
	else if (IsPC())
	{
		if (rkInstVictim.IsStone())
			return true;

		if (rkInstVictim.IsPC())
		{
			if (GetDuelMode())
			{
				switch (GetDuelMode())
				{
				case DUEL_CANNOTATTACK:
					return false;
				case DUEL_START:
					if (__FindDUELKey(GetVirtualID(), rkInstVictim.GetVirtualID()))
						return true;
					else
						return false;
				}
			}
			if (PK_MODE_GUILD == GetPKMode())
				if (GetGuildID() == rkInstVictim.GetGuildID())
					return false;

			if (rkInstVictim.IsKiller())
				if (!IAbstractPlayer::GetSingleton().IsSamePartyMember(GetVirtualID(), rkInstVictim.GetVirtualID()))
					return true;

			if (PK_MODE_PROTECT != GetPKMode())
			{
				if (PK_MODE_FREE == GetPKMode())
				{
					if (PK_MODE_PROTECT != rkInstVictim.GetPKMode())
						if (!IAbstractPlayer::GetSingleton().IsSamePartyMember(GetVirtualID(), rkInstVictim.GetVirtualID()))
							return true;
				}

				if (PK_MODE_GUILD == GetPKMode())
				{
					if (PK_MODE_PROTECT != rkInstVictim.GetPKMode())
						if (!IAbstractPlayer::GetSingleton().IsSamePartyMember(GetVirtualID(), rkInstVictim.GetVirtualID()))
							if (GetGuildID() != rkInstVictim.GetGuildID())
								return true;
				}
			}

			if (IsSameEmpire(rkInstVictim))
			{
				if (IsPVPInstance(rkInstVictim))
					return true;

				if (PK_MODE_REVENGE == GetPKMode())
					if (!IAbstractPlayer::GetSingleton().IsSamePartyMember(GetVirtualID(), rkInstVictim.GetVirtualID()))
						if (IsConflictAlignmentInstance(rkInstVictim))
							return true;
			}
			else
				return true;
		}

		if (rkInstVictim.IsEnemy())
			return true;

		if (rkInstVictim.IsWoodenDoor())
			return true;
	}
	else if (IsEnemy())
	{
		if (rkInstVictim.IsPC())
			return true;
	}
	return false;
}

bool CInstanceBase::IsTargetableInstance(CInstanceBase& rkInstVictim)
{
	return rkInstVictim.CanPickInstance();
}

bool CInstanceBase::CanChangeTarget()
{
	return m_GraphicThingInstance.CanChangeTarget();
}

bool CInstanceBase::CanPickInstance()
{
	if (!__IsInViewFrustum())
		return false;
#ifdef ENABLE_GRAPHIC_OPTIMIZATION
	// @ renderlenmeyen arkadaslarimizi niye secelim ki ?
	if (IsPet() && CPythonSystem::Instance().IsPetStatus() == false)
		return false;
	else if (IsMount() && CPythonSystem::Instance().IsMountStatus() == false)
		return false;
#endif


	if (IsDoor())
	{
		if (IsDead())
			return false;
	}

	if (IsPC())
	{
		if (IsAffect(AFFECT_EUNHYEONG))
		{
			if (!__MainCanSeeHiddenThing())
				return false;
		}
#ifdef ENABLE_CANSEEHIDDENTHING_FOR_GM
		if (IsAffect(AFFECT_REVIVE_INVISIBILITY) && !__MainCanSeeHiddenThing())
			return false;
#else
		if (IsAffect(AFFECT_REVIVE_INVISIBILITY))
			return false;
#endif
#ifdef ENABLE_CANSEEHIDDENTHING_FOR_GM
		if (IsAffect(AFFECT_INVISIBILITY) && !__MainCanSeeHiddenThing())
			return false;
#else
		if (IsAffect(AFFECT_INVISIBILITY))
			return false;
#endif
	}

	if (IsDead())
		return false;

	return true;
}

bool CInstanceBase::CanViewTargetHP(CInstanceBase& rkInstVictim)
{
	if (rkInstVictim.IsStone())
		return true;

	if (rkInstVictim.IsWoodenDoor())
		return true;

	if (rkInstVictim.IsEnemy())
		return true;

	return false;
}

BOOL CInstanceBase::IsPC()
{
	return m_GraphicThingInstance.IsPC();
}

BOOL CInstanceBase::IsNPC()
{
	return m_GraphicThingInstance.IsNPC();
}

#ifdef ENABLE_GROWTH_PET_SYSTEM
BOOL CInstanceBase::IsNewPet()
{
	return m_GraphicThingInstance.IsNewPet();
}
#endif

#ifdef ENABLE_GROWTH_MOUNT_SYSTEM
BOOL CInstanceBase::IsNewExMount()
{
	return m_GraphicThingInstance.IsNewExMount();
}
#endif

BOOL CInstanceBase::IsEnemy()
{
	return m_GraphicThingInstance.IsEnemy();
}

BOOL CInstanceBase::IsStone()
{
	return m_GraphicThingInstance.IsStone();
}

#ifdef ENABLE_BOSS_ON_MINIMAP
BOOL CInstanceBase::IsBoss()
{
	return m_GraphicThingInstance.IsBoss();
}
#endif

BOOL CInstanceBase::IsGuildWall()
{
	return IsWall(m_dwRace);
}

BOOL CInstanceBase::IsResource()
{
	switch (m_dwVirtualNumber)
	{
	case 20047:
	case 20048:
	case 20049:
	case 20050:
	case 20051:
	case 20052:
	case 20053:
	case 20054:
	case 20055:
	case 20056:
	case 20057:
	case 20058:
	case 20059:
	case 30301:
	case 30302:
	case 30303:
	case 30304:
	case 30305:
	case 30306:
		return TRUE;
	}

	return FALSE;
}

BOOL CInstanceBase::IsWarp()
{
	return m_GraphicThingInstance.IsWarp();
}

BOOL CInstanceBase::IsGoto()
{
	return m_GraphicThingInstance.IsGoto();
}

BOOL CInstanceBase::IsObject()
{
	return m_GraphicThingInstance.IsObject();
}

BOOL CInstanceBase::IsDoor()
{
	return m_GraphicThingInstance.IsDoor();
}

BOOL CInstanceBase::IsWoodenDoor()
{
	if (m_GraphicThingInstance.IsDoor())
	{
		int vnum = GetVirtualNumber();

		if (vnum == 13000)
			return true;
		else if (vnum >= 30111 && vnum <= 30119)
			return true;

		return false;
	}
	else
	{
		return false;
	}
}

BOOL CInstanceBase::IsStoneDoor()
{
	return m_GraphicThingInstance.IsDoor() && 13001 == GetVirtualNumber();
}

BOOL CInstanceBase::IsFlag()
{
	if (GetRace() == 20035)
		return TRUE;

	if (GetRace() == 20036)
		return TRUE;

	if (GetRace() == 20037)
		return TRUE;

	return FALSE;
}

BOOL CInstanceBase::IsPet()
{
	return m_GraphicThingInstance.IsPet();
}

BOOL CInstanceBase::IsMount()
{
	return m_GraphicThingInstance.IsMount();
}

BOOL CInstanceBase::IsForceVisible()
{
	if (IsAffect(AFFECT_SHOW_ALWAYS))
		return TRUE;

	if (IsObject() || IsDoor())
		return TRUE;

	return FALSE;
}

int	CInstanceBase::GetInstanceType()
{
	return m_GraphicThingInstance.GetActorType();
}

DWORD CInstanceBase::GetVirtualID()
{
	return m_GraphicThingInstance.GetVirtualID();
}

DWORD CInstanceBase::GetVirtualNumber()
{
	return m_dwVirtualNumber;
}

bool CInstanceBase::__IsInViewFrustum()
{
	return m_GraphicThingInstance.isShow();
}

bool CInstanceBase::__CanRender()
{
#ifdef ENABLE_MODEL_RENDER_TARGET
	// @@render target ve surekli renderlemek istedigimiz instance classlar icin
	// kullanilacak fonksiyon.
	if (IsAlwaysRender())
		return true;
#endif

#ifdef ENABLE_GRAPHIC_OPTIMIZATION
	// @@ eger oyun seceneklerinde pet ve binek gizlemeyi aktif ettiysek
	// bu siniftaki arkadas pet mi binekmi oldugunu algilayip pythonsystemden kontrol ettirerek
	// return false cektirecegiz.
	if (IsPet() && CPythonSystem::Instance().IsPetStatus() == false)
		return false;
	else if (IsMount() && CPythonSystem::Instance().IsMountStatus() == false)
		return false;
#endif

	if (!__IsInViewFrustum())
		return false;

#ifdef ENABLE_CANSEEHIDDENTHING_FOR_GM
	if (IsAffect(AFFECT_INVISIBILITY) && !__MainCanSeeHiddenThing())
		return false;
#else
	if (IsAffect(AFFECT_INVISIBILITY))
		return false;
#endif

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Graphic Control

bool CInstanceBase::IntersectBoundingBox()
{
	float u, v, t;
	return m_GraphicThingInstance.Intersect(&u, &v, &t);
}

bool CInstanceBase::IntersectDefendingSphere()
{
	return m_GraphicThingInstance.IntersectDefendingSphere();
}

float CInstanceBase::GetDistance(CInstanceBase* pkTargetInst)
{
	TPixelPosition TargetPixelPosition;
	pkTargetInst->m_GraphicThingInstance.GetPixelPosition(&TargetPixelPosition);
	return GetDistance(TargetPixelPosition);
}

float CInstanceBase::GetDistance(const TPixelPosition& c_rPixelPosition)
{
	TPixelPosition PixelPosition;
	m_GraphicThingInstance.GetPixelPosition(&PixelPosition);

	float fdx = PixelPosition.x - c_rPixelPosition.x;
	float fdy = PixelPosition.y - c_rPixelPosition.y;

	return sqrtf((fdx * fdx) + (fdy * fdy));
}

CActorInstance& CInstanceBase::GetGraphicThingInstanceRef()
{
	return m_GraphicThingInstance;
}

CActorInstance* CInstanceBase::GetGraphicThingInstancePtr()
{
	return &m_GraphicThingInstance;
}

void CInstanceBase::RefreshActorInstance()
{
	m_GraphicThingInstance.RefreshActorInstance();
}

void CInstanceBase::Refresh(DWORD dwMotIndex, bool isLoop)
{
	RefreshState(dwMotIndex, isLoop);
}

void CInstanceBase::RestoreRenderMode()
{
	m_GraphicThingInstance.RestoreRenderMode();
}

void CInstanceBase::SetAddRenderMode()
{
	m_GraphicThingInstance.SetAddRenderMode();
}

void CInstanceBase::SetModulateRenderMode()
{
	m_GraphicThingInstance.SetModulateRenderMode();
}

void CInstanceBase::SetRenderMode(int iRenderMode)
{
	m_GraphicThingInstance.SetRenderMode(iRenderMode);
}

void CInstanceBase::SetAddColor(const D3DXCOLOR& c_rColor)
{
	m_GraphicThingInstance.SetAddColor(c_rColor);
}

void CInstanceBase::__SetBlendRenderingMode()
{
	m_GraphicThingInstance.SetBlendRenderMode();
}

void CInstanceBase::__SetAlphaValue(float fAlpha)
{
	m_GraphicThingInstance.SetAlphaValue(fAlpha);
}

float CInstanceBase::__GetAlphaValue()
{
	return m_GraphicThingInstance.GetAlphaValue();
}

void CInstanceBase::SetHair(DWORD eHair)
{
	if (IsPC() == false)
		return;
	m_awPart[CRaceData::PART_HAIR] = eHair;
	m_GraphicThingInstance.SetHair(eHair);
}

void CInstanceBase::ChangeHair(DWORD eHair)
{
	if (IsPC() == false)
		return;

	if (GetPart(CRaceData::PART_HAIR) == eHair)
		return;

	SetHair(eHair);
	RefreshState(CRaceMotionData::NAME_WAIT, true);
}

#if defined(ENABLE_ITEM_EVOLUTION_SYSTEM) && defined(ENABLE_ARMOR_EVOLUTION_SYSTEM)
void CInstanceBase::SetArmor(DWORD dwArmor, DWORD dwArmorEvolution)
#else
void CInstanceBase::SetArmor(DWORD dwArmor)
#endif
{
	DWORD dwShape;
	if (__ArmorVnumToShape(dwArmor, &dwShape))
	{
		CItemData* pItemData;
		if (CItemManager::Instance().GetItemDataPointer(dwArmor, &pItemData))
		{
			float fSpecularPower = pItemData->GetSpecularPowerf();
			SetShape(dwShape, fSpecularPower);
#if defined(ENABLE_ITEM_EVOLUTION_SYSTEM) && defined(ENABLE_ARMOR_EVOLUTION_SYSTEM)
			__GetRefinedEffect(pItemData, 0, dwArmorEvolution);
#elif defined(ENABLE_ITEM_EVOLUTION_SYSTEM) && !define(ENABLE_ARMOR_EVOLUTION_SYSTEM)
			__GetRefinedEffect(pItemData, 0);
#else
			__GetRefinedEffect(pItemData);
#endif
#ifdef ENABLE_SHINING_EFFECT_UTILITY
			__GetShiningEffect(pItemData);
#endif
			return;
		}
		else
		{
			__ClearArmorRefineEffect();
#ifdef ENABLE_SHINING_EFFECT_UTILITY
			__ClearArmorShiningEffect();
#endif
		}
	}

	SetShape(dwArmor);
}

#ifdef ENABLE_AURA_COSTUME_SYSTEM
bool CInstanceBase::SetAura(DWORD eAura)
{
	if (!IsPC())
		return false;

#ifdef ENABLE_HIDE_BODY_PARTS
	if (eAura != 0 && CPythonSystem::Instance().IsHideAura() == true)
	{
		CPythonSystem::Instance().SetPartBackup(CRaceData::PART_AURA, eAura);
		return false;
	}
#endif // ENABLE_HIDE_BODY_PARTS

	if (!eAura)
	{
		if (m_auraRefineEffect)
		{
			__DetachEffect(m_auraRefineEffect);
			m_auraRefineEffect = 0;
		}
		m_awPart[CRaceData::PART_AURA] = 0;
		return true;
	}

	CItemData* pItemData;
	if (!CItemManager::Instance().GetItemDataPointer(eAura, &pItemData))
	{
		if (m_auraRefineEffect)
		{
			__DetachEffect(m_auraRefineEffect);
			m_auraRefineEffect = 0;
		}
		m_awPart[CRaceData::PART_AURA] = 0;
		return true;
	}
	ClearAuraEffect();
	BYTE byRace = (BYTE)GetRace();
	BYTE byJob = (BYTE)RaceToJob(byRace);
	BYTE bySex = (BYTE)RaceToSex(byRace);

#ifdef ENABLE_SHINING_EFFECT_UTILITY
	if (pItemData)
		__GetShiningEffect(pItemData);
#endif

	D3DXVECTOR3 v3MeshScale = pItemData->GetAuraMeshScaleVector(byJob, bySex);
	float fParticleScale = pItemData->GetAuraParticleScale(byJob, bySex);
	m_auraRefineEffect = m_GraphicThingInstance.AttachEffectByID(0, "Bip01 Spine2", pItemData->GetAuraEffectID(), NULL, NULL, false, fParticleScale, &v3MeshScale);
	m_awPart[CRaceData::PART_AURA] = eAura;
	return true;
}

void CInstanceBase::ChangeAura(DWORD eAura)
{
	if (!IsPC())
		return;

	SetAura(eAura);
}

void CInstanceBase::ClearAuraEffect()
{
	if (!m_auraRefineEffect)
		return;

	__DetachEffect(m_auraRefineEffect);
	m_auraRefineEffect = 0;
}
#endif

#ifdef ENABLE_ACCE_SYSTEM
bool CInstanceBase::SetAcce(DWORD dwAcce, DWORD dwAcceSpecular)
{
	if (!IsPC())
		return false;

#ifdef ENABLE_HIDE_BODY_PARTS
	if (dwAcce != 0 && CPythonSystem::Instance().IsHideAcce() == true)
	{
		CPythonSystem::Instance().SetPartBackup(CRaceData::PART_ACCE, dwAcce);
		CPythonSystem::Instance().SetPartSpecularBackup(dwAcceSpecular);
		return false;
	}
#endif // ENABLE_HIDE_BODY_PARTS

	if (dwAcce == 0)
	{
		m_awPart[CRaceData::PART_ACCE] = 0;
		m_GraphicThingInstance.AttachAcce(0, 0.0f);
		__ClearAcceRefineEffect();
#ifdef ENABLE_SHINING_EFFECT_UTILITY
		__ClearAcceShiningEffect();
#endif
		return false;
	}

	__ClearAcceRefineEffect();
#ifdef ENABLE_SHINING_EFFECT_UTILITY
	__ClearAcceShiningEffect();
#endif

	float fSpecular = 65.0f;

	m_dwAcceSpecular = dwAcceSpecular;

	if (dwAcceSpecular > 18)
	{
		fSpecular += 35;

		m_acceRefineEffect = EFFECT_REFINED + EFFECT_ACCE;
		__EffectContainer_AttachEffect(m_acceRefineEffect);
	}

	fSpecular /= 100.0f;

	m_awPart[CRaceData::PART_ACCE] = dwAcce;

	CItemData* pItemData;
	if (!CItemManager::Instance().GetItemDataPointer(dwAcce, &pItemData))
	{
		__ClearAcceRefineEffect();
		m_GraphicThingInstance.SetAcceScale(1.0f, 1.0f, 1.0f);
		m_awPart[CRaceData::PART_ACCE] = 0;
		m_GraphicThingInstance.AttachAcce(NULL, 0.0f);
		return true;
	}

#ifdef ENABLE_SHINING_EFFECT_UTILITY
	if (pItemData)
		__GetShiningEffect(pItemData);
#endif

	m_GraphicThingInstance.AttachAcce(pItemData, fSpecular);

	BYTE bRace = (BYTE)GetRace();
	BYTE bJob = (BYTE)RaceToJob(bRace);
	BYTE bSex = (BYTE)RaceToSex(bRace);

	m_GraphicThingInstance.SetAcceScale(pItemData->GetItemScaleVector(bJob, bSex), bRace);
	m_awPart[CRaceData::PART_ACCE] = dwAcce;

	return true;
}

void CInstanceBase::ChangeAcce(DWORD dwAcce, DWORD dwAcceSpecular)
{
	if (!IsPC())
		return;

	SetAcce(dwAcce, dwAcceSpecular);
}

void CInstanceBase::__ClearAcceRefineEffect()
{
	if (!m_acceRefineEffect)
		return;

	__EffectContainer_DetachEffect(m_acceRefineEffect);
	m_acceRefineEffect = 0;
}
#endif

void CInstanceBase::SetShape(DWORD eShape, float fSpecular)
{
	m_GraphicThingInstance.SetShape(eShape, fSpecular);

	m_eShape = eShape;
}

DWORD CInstanceBase::GetWeaponType()
{
	DWORD dwWeapon = GetPart(CRaceData::PART_WEAPON);

	CItemData* pItemData;
	if (!CItemManager::Instance().GetItemDataPointer(dwWeapon, &pItemData))
		return CItemData::WEAPON_NONE;

	return pItemData->GetWeaponType();
}

void CInstanceBase::__ClearWeaponRefineEffect()
{
	if (m_swordRefineEffectRight)
	{
		__DetachEffect(m_swordRefineEffectRight);
		m_swordRefineEffectRight = 0;
	}

	if (m_swordRefineEffectLeft)
	{
		__DetachEffect(m_swordRefineEffectLeft);
		m_swordRefineEffectLeft = 0;
	}
}

void CInstanceBase::__ClearArmorRefineEffect()
{
	if (m_armorRefineEffect)
	{
		__DetachEffect(m_armorRefineEffect);
		m_armorRefineEffect = 0;
	}
}

#ifdef ENABLE_ITEM_EVOLUTION_SYSTEM
void CInstanceBase::__GetEvolutionEffect(BYTE bWeaponType, DWORD dwEvolution, BYTE refine)
{
	switch (bWeaponType)
	{
	case CItemData::WEAPON_DAGGER:
	{
		if (GetEvolutionType(dwEvolution) == WEAPON_EVOLUTION_TYPE_FIRE)
		{
			m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_SMALLSWORD_RARITY_FIRE;
			m_swordRefineEffectLeft = EFFECT_REFINED + EFFECT_SMALLSWORD_RARITY_LEFT_FIRE;
		}
		else if (GetEvolutionType(dwEvolution) == WEAPON_EVOLUTION_TYPE_ICE)
		{
			m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_SMALLSWORD_RARITY_ICE;
			m_swordRefineEffectLeft = EFFECT_REFINED + EFFECT_SMALLSWORD_RARITY_LEFT_ICE;
		}
		else if (GetEvolutionType(dwEvolution) == WEAPON_EVOLUTION_TYPE_EARTH)
		{
			m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_SMALLSWORD_RARITY_EARTH;
			m_swordRefineEffectLeft = EFFECT_REFINED + EFFECT_SMALLSWORD_RARITY_LEFT_EARTH;
		}
		else if (GetEvolutionType(dwEvolution) == WEAPON_EVOLUTION_TYPE_WIND)
		{
			m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_SMALLSWORD_RARITY_WIND;
			m_swordRefineEffectLeft = EFFECT_REFINED + EFFECT_SMALLSWORD_RARITY_LEFT_WIND;
		}
		else if (GetEvolutionType(dwEvolution) == WEAPON_EVOLUTION_TYPE_DARK)
		{
			m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_SMALLSWORD_RARITY_DARK;
			m_swordRefineEffectLeft = EFFECT_REFINED + EFFECT_SMALLSWORD_RARITY_LEFT_DARK;
		}
		else if (GetEvolutionType(dwEvolution) == WEAPON_EVOLUTION_TYPE_ELEC)
		{
			m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_SMALLSWORD_RARITY_ELEC;
			m_swordRefineEffectLeft = EFFECT_REFINED + EFFECT_SMALLSWORD_RARITY_LEFT_ELEC;
		}
		else if (GetEvolutionType(dwEvolution) == WEAPON_EVOLUTION_TYPE_ALL)
		{
			m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_OTHER_RARITY_ALL;
			m_swordRefineEffectLeft = EFFECT_REFINED + EFFECT_OTHER_RARITY_ALL_LEFT;
		}
		else if (GetEvolutionType(dwEvolution) == WEAPON_EVOLUTION_TYPE_HUMAN)
		{
			m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_OTHER_RARITY_ALL;
			m_swordRefineEffectLeft = EFFECT_REFINED + EFFECT_OTHER_RARITY_ALL_LEFT;
		}
		else if (GetEvolutionType(dwEvolution) == WEAPON_EVOLUTION_TYPE_EX_FIRE)
		{
			m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_SMALLSWORD_RARITY_FIRE;
			m_swordRefineEffectLeft = EFFECT_REFINED + EFFECT_SMALLSWORD_RARITY_LEFT_FIRE;
		}
		else if (GetEvolutionType(dwEvolution) == WEAPON_EVOLUTION_TYPE_EX_ICE)
		{
			m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_SMALLSWORD_RARITY_ICE;
			m_swordRefineEffectLeft = EFFECT_REFINED + EFFECT_SMALLSWORD_RARITY_LEFT_ICE;
		}
		else if (GetEvolutionType(dwEvolution) == WEAPON_EVOLUTION_TYPE_EX_EARTH)
		{
			m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_SMALLSWORD_RARITY_EARTH;
			m_swordRefineEffectLeft = EFFECT_REFINED + EFFECT_SMALLSWORD_RARITY_LEFT_EARTH;
		}
		else if (GetEvolutionType(dwEvolution) == WEAPON_EVOLUTION_TYPE_EX_WIND)
		{
			m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_SMALLSWORD_RARITY_WIND;
			m_swordRefineEffectLeft = EFFECT_REFINED + EFFECT_SMALLSWORD_RARITY_LEFT_WIND;
		}
		else if (GetEvolutionType(dwEvolution) == WEAPON_EVOLUTION_TYPE_EX_DARK)
		{
			m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_SMALLSWORD_RARITY_DARK;
			m_swordRefineEffectLeft = EFFECT_REFINED + EFFECT_SMALLSWORD_RARITY_LEFT_DARK;
		}
		else if (GetEvolutionType(dwEvolution) == WEAPON_EVOLUTION_TYPE_EX_ELEC)
		{
			m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_SMALLSWORD_RARITY_ELEC;
			m_swordRefineEffectLeft = EFFECT_REFINED + EFFECT_SMALLSWORD_RARITY_LEFT_ELEC;
		}
		else if (GetEvolutionType(dwEvolution) == WEAPON_EVOLUTION_TYPE_EX_ALL)
		{
			m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_OTHER_RARITY_ALL;
			m_swordRefineEffectLeft = EFFECT_REFINED + EFFECT_OTHER_RARITY_ALL_LEFT;
		}
		else
		{
			m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_SMALLSWORD_REFINED7 + refine - 7;
			m_swordRefineEffectLeft = EFFECT_REFINED + EFFECT_SMALLSWORD_REFINED7_LEFT + refine - 7;
		}

		if (m_swordRefineEffectRight)
			m_swordRefineEffectRight = __AttachEffect(m_swordRefineEffectRight);

		if (m_swordRefineEffectLeft)
			m_swordRefineEffectLeft = __AttachEffect(m_swordRefineEffectLeft);
	}
	break;

	case CItemData::WEAPON_FAN:
	{
		if (GetEvolutionType(dwEvolution) == WEAPON_EVOLUTION_TYPE_FIRE) m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_FANBELL_RARITY_FIRE;
		else if (GetEvolutionType(dwEvolution) == WEAPON_EVOLUTION_TYPE_ICE) m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_FANBELL_RARITY_ICE;
		else if (GetEvolutionType(dwEvolution) == WEAPON_EVOLUTION_TYPE_EARTH) m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_FANBELL_RARITY_EARTH;
		else if (GetEvolutionType(dwEvolution) == WEAPON_EVOLUTION_TYPE_WIND) m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_FANBELL_RARITY_WIND;
		else if (GetEvolutionType(dwEvolution) == WEAPON_EVOLUTION_TYPE_DARK) m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_FANBELL_RARITY_DARK;
		else if (GetEvolutionType(dwEvolution) == WEAPON_EVOLUTION_TYPE_ELEC) m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_FANBELL_RARITY_ELEC;
		else if (GetEvolutionType(dwEvolution) == WEAPON_EVOLUTION_TYPE_ALL) m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_FAN_RARITY_ALL;
		else if (GetEvolutionType(dwEvolution) == WEAPON_EVOLUTION_TYPE_HUMAN) m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_FAN_RARITY_ALL;
		else if (GetEvolutionType(dwEvolution) == WEAPON_EVOLUTION_TYPE_EX_FIRE) m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_FANBELL_RARITY_FIRE;
		else if (GetEvolutionType(dwEvolution) == WEAPON_EVOLUTION_TYPE_EX_ICE) m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_FANBELL_RARITY_ICE;
		else if (GetEvolutionType(dwEvolution) == WEAPON_EVOLUTION_TYPE_EX_EARTH) m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_FANBELL_RARITY_EARTH;
		else if (GetEvolutionType(dwEvolution) == WEAPON_EVOLUTION_TYPE_EX_WIND) m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_FANBELL_RARITY_WIND;
		else if (GetEvolutionType(dwEvolution) == WEAPON_EVOLUTION_TYPE_EX_DARK) m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_FANBELL_RARITY_DARK;
		else if (GetEvolutionType(dwEvolution) == WEAPON_EVOLUTION_TYPE_EX_ELEC) m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_FANBELL_RARITY_ELEC;
		else if (GetEvolutionType(dwEvolution) == WEAPON_EVOLUTION_TYPE_EX_ALL) m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_FAN_RARITY_ALL;
		else m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_FANBELL_REFINED7 + refine - 7;
	}
	break;

	case CItemData::WEAPON_BELL:
	{
		if (GetEvolutionType(dwEvolution) == WEAPON_EVOLUTION_TYPE_FIRE) m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_SMALLSWORD_RARITY_FIRE;
		else if (GetEvolutionType(dwEvolution) == WEAPON_EVOLUTION_TYPE_ICE) m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_SMALLSWORD_RARITY_ICE;
		else if (GetEvolutionType(dwEvolution) == WEAPON_EVOLUTION_TYPE_EARTH) m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_SMALLSWORD_RARITY_EARTH;
		else if (GetEvolutionType(dwEvolution) == WEAPON_EVOLUTION_TYPE_WIND) m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_SMALLSWORD_RARITY_WIND;
		else if (GetEvolutionType(dwEvolution) == WEAPON_EVOLUTION_TYPE_DARK) m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_SMALLSWORD_RARITY_DARK;
		else if (GetEvolutionType(dwEvolution) == WEAPON_EVOLUTION_TYPE_ELEC) m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_SMALLSWORD_RARITY_ELEC;
		else if (GetEvolutionType(dwEvolution) == WEAPON_EVOLUTION_TYPE_ALL) m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_OTHER_RARITY_ALL;
		else if (GetEvolutionType(dwEvolution) == WEAPON_EVOLUTION_TYPE_HUMAN) m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_OTHER_RARITY_ALL;
		else if (GetEvolutionType(dwEvolution) == WEAPON_EVOLUTION_TYPE_EX_FIRE) m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_SMALLSWORD_RARITY_FIRE;
		else if (GetEvolutionType(dwEvolution) == WEAPON_EVOLUTION_TYPE_EX_ICE) m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_SMALLSWORD_RARITY_ICE;
		else if (GetEvolutionType(dwEvolution) == WEAPON_EVOLUTION_TYPE_EX_EARTH) m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_SMALLSWORD_RARITY_EARTH;
		else if (GetEvolutionType(dwEvolution) == WEAPON_EVOLUTION_TYPE_EX_WIND) m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_SMALLSWORD_RARITY_WIND;
		else if (GetEvolutionType(dwEvolution) == WEAPON_EVOLUTION_TYPE_EX_DARK) m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_SMALLSWORD_RARITY_DARK;
		else if (GetEvolutionType(dwEvolution) == WEAPON_EVOLUTION_TYPE_EX_ELEC) m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_SMALLSWORD_RARITY_ELEC;
		else if (GetEvolutionType(dwEvolution) == WEAPON_EVOLUTION_TYPE_EX_ALL) m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_OTHER_RARITY_ALL;
		else m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_SMALLSWORD_REFINED7 + refine - 7;

		if (m_swordRefineEffectRight)
			m_swordRefineEffectRight = __AttachEffect(m_swordRefineEffectRight);

		if (m_swordRefineEffectLeft)
			m_swordRefineEffectLeft = __AttachEffect(m_swordRefineEffectLeft);
	}
	break;

	case CItemData::WEAPON_BOW:
	{
		if (GetEvolutionType(dwEvolution) == WEAPON_EVOLUTION_TYPE_FIRE) m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_BOW_RARITY_FIRE;
		else if (GetEvolutionType(dwEvolution) == WEAPON_EVOLUTION_TYPE_ICE) m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_BOW_RARITY_ICE;
		else if (GetEvolutionType(dwEvolution) == WEAPON_EVOLUTION_TYPE_EARTH) m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_BOW_RARITY_EARTH;
		else if (GetEvolutionType(dwEvolution) == WEAPON_EVOLUTION_TYPE_WIND) m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_BOW_RARITY_WIND;
		else if (GetEvolutionType(dwEvolution) == WEAPON_EVOLUTION_TYPE_DARK) m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_BOW_RARITY_DARK;
		else if (GetEvolutionType(dwEvolution) == WEAPON_EVOLUTION_TYPE_ELEC) m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_BOW_RARITY_ELEC;
		else if (GetEvolutionType(dwEvolution) == WEAPON_EVOLUTION_TYPE_ALL) m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_BOW_RARITY_ALL;
		else if (GetEvolutionType(dwEvolution) == WEAPON_EVOLUTION_TYPE_HUMAN) m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_BOW_RARITY_ALL;
		else if (GetEvolutionType(dwEvolution) == WEAPON_EVOLUTION_TYPE_EX_FIRE) m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_BOW_RARITY_FIRE;
		else if (GetEvolutionType(dwEvolution) == WEAPON_EVOLUTION_TYPE_EX_ICE) m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_BOW_RARITY_ICE;
		else if (GetEvolutionType(dwEvolution) == WEAPON_EVOLUTION_TYPE_EX_EARTH) m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_BOW_RARITY_EARTH;
		else if (GetEvolutionType(dwEvolution) == WEAPON_EVOLUTION_TYPE_EX_WIND) m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_BOW_RARITY_WIND;
		else if (GetEvolutionType(dwEvolution) == WEAPON_EVOLUTION_TYPE_EX_DARK) m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_BOW_RARITY_DARK;
		else if (GetEvolutionType(dwEvolution) == WEAPON_EVOLUTION_TYPE_EX_ELEC) m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_BOW_RARITY_ELEC;
		else if (GetEvolutionType(dwEvolution) == WEAPON_EVOLUTION_TYPE_EX_ALL) m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_BOW_RARITY_ALL;
		else m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_BOW_REFINED7 + refine - 7;

		if (m_swordRefineEffectRight)
			m_swordRefineEffectRight = __AttachEffect(m_swordRefineEffectRight);

		if (m_swordRefineEffectLeft)
			m_swordRefineEffectLeft = __AttachEffect(m_swordRefineEffectLeft);
	}
	break;

#ifdef ENABLE_WOLFMAN_CHARACTER
	case CItemData::WEAPON_CLAW:
	{
		if (GetEvolutionType(dwEvolution) == WEAPON_EVOLUTION_TYPE_FIRE)
		{
			m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_CLAW_RARITY_FIRE;
			m_swordRefineEffectLeft = EFFECT_REFINED + EFFECT_CLAW_RARITY_LEFT_FIRE;
		}
		else if (GetEvolutionType(dwEvolution) == WEAPON_EVOLUTION_TYPE_ICE)
		{
			m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_CLAW_RARITY_ICE;
			m_swordRefineEffectLeft = EFFECT_REFINED + EFFECT_CLAW_RARITY_LEFT_ICE;
		}
		else if (GetEvolutionType(dwEvolution) == WEAPON_EVOLUTION_TYPE_EARTH)
		{
			m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_CLAW_RARITY_ICE;
			m_swordRefineEffectLeft = EFFECT_REFINED + EFFECT_CLAW_RARITY_LEFT_ICE;
		}
		else if (GetEvolutionType(dwEvolution) == WEAPON_EVOLUTION_TYPE_WIND)
		{
			m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_CLAW_RARITY_WIND;
			m_swordRefineEffectLeft = EFFECT_REFINED + EFFECT_CLAW_RARITY_LEFT_WIND;
		}
		else if (GetEvolutionType(dwEvolution) == WEAPON_EVOLUTION_TYPE_DARK)
		{
			m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_CLAW_RARITY_DARK;
			m_swordRefineEffectLeft = EFFECT_REFINED + EFFECT_CLAW_RARITY_LEFT_DARK;
		}
		else if (GetEvolutionType(dwEvolution) == WEAPON_EVOLUTION_TYPE_ELEC)
		{
			m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_CLAW_RARITY_ELEC;
			m_swordRefineEffectLeft = EFFECT_REFINED + EFFECT_CLAW_RARITY_LEFT_ELEC;
		}
		else if (GetEvolutionType(dwEvolution) == WEAPON_EVOLUTION_TYPE_ALL)
		{
			m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_OTHER_RARITY_ALL;
			m_swordRefineEffectLeft = EFFECT_REFINED + EFFECT_OTHER_RARITY_ALL_LEFT;
		}
		else if (GetEvolutionType(dwEvolution) == WEAPON_EVOLUTION_TYPE_HUMAN)
		{
			m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_OTHER_RARITY_ALL;
			m_swordRefineEffectLeft = EFFECT_REFINED + EFFECT_OTHER_RARITY_ALL_LEFT;
		}
		else if (GetEvolutionType(dwEvolution) == WEAPON_EVOLUTION_TYPE_EX_FIRE)
		{
			m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_CLAW_RARITY_FIRE;
			m_swordRefineEffectLeft = EFFECT_REFINED + EFFECT_CLAW_RARITY_LEFT_FIRE;
		}
		else if (GetEvolutionType(dwEvolution) == WEAPON_EVOLUTION_TYPE_EX_ICE)
		{
			m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_CLAW_RARITY_ICE;
			m_swordRefineEffectLeft = EFFECT_REFINED + EFFECT_CLAW_RARITY_LEFT_ICE;
		}
		else if (GetEvolutionType(dwEvolution) == WEAPON_EVOLUTION_TYPE_EX_EARTH)
		{
			m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_CLAW_RARITY_ICE;
			m_swordRefineEffectLeft = EFFECT_REFINED + EFFECT_CLAW_RARITY_LEFT_ICE;
		}
		else if (GetEvolutionType(dwEvolution) == WEAPON_EVOLUTION_TYPE_EX_WIND)
		{
			m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_CLAW_RARITY_WIND;
			m_swordRefineEffectLeft = EFFECT_REFINED + EFFECT_CLAW_RARITY_LEFT_WIND;
		}
		else if (GetEvolutionType(dwEvolution) == WEAPON_EVOLUTION_TYPE_EX_DARK)
		{
			m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_CLAW_RARITY_DARK;
			m_swordRefineEffectLeft = EFFECT_REFINED + EFFECT_CLAW_RARITY_LEFT_DARK;
		}
		else if (GetEvolutionType(dwEvolution) == WEAPON_EVOLUTION_TYPE_EX_ELEC)
		{
			m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_CLAW_RARITY_ELEC;
			m_swordRefineEffectLeft = EFFECT_REFINED + EFFECT_CLAW_RARITY_LEFT_ELEC;
		}
		else if (GetEvolutionType(dwEvolution) == WEAPON_EVOLUTION_TYPE_EX_ALL)
		{
			m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_OTHER_RARITY_ALL;
			m_swordRefineEffectLeft = EFFECT_REFINED + EFFECT_OTHER_RARITY_ALL_LEFT;
		}
		else
		{
			m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_CLAW_REFINED7 + refine - 7;
			m_swordRefineEffectLeft = EFFECT_REFINED + EFFECT_CLAW_REFINED7_LEFT + refine - 7;
		}

		if (m_swordRefineEffectRight)
			m_swordRefineEffectRight = __AttachEffect(m_swordRefineEffectRight);

		if (m_swordRefineEffectLeft)
			m_swordRefineEffectLeft = __AttachEffect(m_swordRefineEffectLeft);
	}
	break;
#endif
	case 255:
	{
		if (GetEvolutionType(dwEvolution) == WEAPON_EVOLUTION_TYPE_FIRE)
			m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_SWORD_RARITY_FIRE;
		else if (GetEvolutionType(dwEvolution) == WEAPON_EVOLUTION_TYPE_ICE)
			m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_SWORD_RARITY_ICE;
		else if (GetEvolutionType(dwEvolution) == WEAPON_EVOLUTION_TYPE_EARTH)
			m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_SWORD_RARITY_EARTH;
		else if (GetEvolutionType(dwEvolution) == WEAPON_EVOLUTION_TYPE_WIND)
			m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_SWORD_RARITY_WIND;
		else if (GetEvolutionType(dwEvolution) == WEAPON_EVOLUTION_TYPE_DARK)
			m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_SWORD_RARITY_DARK;
		else if (GetEvolutionType(dwEvolution) == WEAPON_EVOLUTION_TYPE_ELEC)
			m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_SWORD_RARITY_ELEC;
		else if (GetEvolutionType(dwEvolution) == WEAPON_EVOLUTION_TYPE_ALL)
			m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_SWORD_RARITY_ALL;
		else if (GetEvolutionType(dwEvolution) == WEAPON_EVOLUTION_TYPE_HUMAN)
			m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_SWORD_RARITY_ALL;
		else if (GetEvolutionType(dwEvolution) == WEAPON_EVOLUTION_TYPE_EX_FIRE)
			m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_SWORD_RARITY_FIRE;
		else if (GetEvolutionType(dwEvolution) == WEAPON_EVOLUTION_TYPE_EX_ICE)
			m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_SWORD_RARITY_ICE;
		else if (GetEvolutionType(dwEvolution) == WEAPON_EVOLUTION_TYPE_EX_EARTH)
			m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_SWORD_RARITY_EARTH;
		else if (GetEvolutionType(dwEvolution) == WEAPON_EVOLUTION_TYPE_EX_WIND)
			m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_SWORD_RARITY_WIND;
		else if (GetEvolutionType(dwEvolution) == WEAPON_EVOLUTION_TYPE_EX_DARK)
			m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_SWORD_RARITY_DARK;
		else if (GetEvolutionType(dwEvolution) == WEAPON_EVOLUTION_TYPE_EX_ELEC)
			m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_SWORD_RARITY_ELEC;
		else if (GetEvolutionType(dwEvolution) == WEAPON_EVOLUTION_TYPE_EX_ALL)
			m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_SWORD_RARITY_ALL;
		else
			m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_SWORD_REFINED7 + refine - 7;

		if (m_swordRefineEffectRight)
			m_swordRefineEffectRight = __AttachEffect(m_swordRefineEffectRight);

		if (m_swordRefineEffectLeft)
			m_swordRefineEffectLeft = __AttachEffect(m_swordRefineEffectLeft);
	}
	break;

#ifdef ENABLE_ARMOR_EVOLUTION_SYSTEM
	case 99:
	{
		if (GetEvolutionType(dwEvolution) == WEAPON_EVOLUTION_TYPE_FIRE)
			m_armorRefineEffect = EFFECT_REFINED + EFFECT_ARMOR_RARITY_FIRE;
		else if (GetEvolutionType(dwEvolution) == WEAPON_EVOLUTION_TYPE_ICE)
			m_armorRefineEffect = EFFECT_REFINED + EFFECT_ARMOR_RARITY_ICE;
		else if (GetEvolutionType(dwEvolution) == WEAPON_EVOLUTION_TYPE_EARTH)
			m_armorRefineEffect = EFFECT_REFINED + EFFECT_ARMOR_RARITY_EARTH;
		else if (GetEvolutionType(dwEvolution) == WEAPON_EVOLUTION_TYPE_WIND)
			m_armorRefineEffect = EFFECT_REFINED + EFFECT_ARMOR_RARITY_WIND;
		else if (GetEvolutionType(dwEvolution) == WEAPON_EVOLUTION_TYPE_DARK)
			m_armorRefineEffect = EFFECT_REFINED + EFFECT_ARMOR_RARITY_DARK;
		else if (GetEvolutionType(dwEvolution) == WEAPON_EVOLUTION_TYPE_ELEC)
			m_armorRefineEffect = EFFECT_REFINED + EFFECT_ARMOR_RARITY_ELEC;
		else if (GetEvolutionType(dwEvolution) == WEAPON_EVOLUTION_TYPE_ALL)
			m_armorRefineEffect = EFFECT_REFINED + EFFECT_ARMOR_RARITY_ALL;
		else if (GetEvolutionType(dwEvolution) == WEAPON_EVOLUTION_TYPE_HUMAN)
			m_armorRefineEffect = EFFECT_REFINED + EFFECT_ARMOR_RARITY_ALL;
		else if (GetEvolutionType(dwEvolution) == WEAPON_EVOLUTION_TYPE_EX_FIRE)
			m_armorRefineEffect = EFFECT_REFINED + EFFECT_ARMOR_RARITY_FIRE;
		else if (GetEvolutionType(dwEvolution) == WEAPON_EVOLUTION_TYPE_EX_ICE)
			m_armorRefineEffect = EFFECT_REFINED + EFFECT_ARMOR_RARITY_ICE;
		else if (GetEvolutionType(dwEvolution) == WEAPON_EVOLUTION_TYPE_EX_EARTH)
			m_armorRefineEffect = EFFECT_REFINED + EFFECT_ARMOR_RARITY_EARTH;
		else if (GetEvolutionType(dwEvolution) == WEAPON_EVOLUTION_TYPE_EX_WIND)
			m_armorRefineEffect = EFFECT_REFINED + EFFECT_ARMOR_RARITY_WIND;
		else if (GetEvolutionType(dwEvolution) == WEAPON_EVOLUTION_TYPE_EX_DARK)
			m_armorRefineEffect = EFFECT_REFINED + EFFECT_ARMOR_RARITY_DARK;
		else if (GetEvolutionType(dwEvolution) == WEAPON_EVOLUTION_TYPE_EX_ELEC)
			m_armorRefineEffect = EFFECT_REFINED + EFFECT_ARMOR_RARITY_ELEC;
		else if (GetEvolutionType(dwEvolution) == WEAPON_EVOLUTION_TYPE_EX_ALL)
			m_armorRefineEffect = EFFECT_REFINED + EFFECT_ARMOR_RARITY_ALL;
		else
			m_armorRefineEffect = EFFECT_REFINED + EFFECT_BODYARMOR_REFINED7 + refine - 7;

		if (m_armorRefineEffect)
			m_armorRefineEffect = __AttachEffect(m_armorRefineEffect);
	}
	break;
#endif
	}
}
#endif

#if defined(ENABLE_ITEM_EVOLUTION_SYSTEM) && defined(ENABLE_ARMOR_EVOLUTION_SYSTEM)
UINT CInstanceBase::__GetRefinedEffect(CItemData* pItem, DWORD dwWeaponEvolution, DWORD dwArmorEvolution)
#elif defined(ENABLE_ITEM_EVOLUTION_SYSTEM) && !defined(ENABLE_ARMOR_EVOLUTION_SYSTEM)
UINT CInstanceBase::__GetRefinedEffect(CItemData* pItem, DWORD dwWeaponEvolution)
#else
UINT CInstanceBase::__GetRefinedEffect(CItemData* pItem)
#endif
{
#ifdef ENABLE_SHINING_ITEM_SYSTEM
	if (pItem->GetType() == CItemData::ITEM_TYPE_WEAPON || pItem->GetType() == CItemData::ITEM_TYPE_COSTUME) {
		if (!AttachShiningEffect(pItem->GetIndex()))
			TraceError("Error with Shining attachment");
	}
	else {
		if (!AttachShiningEffect(m_awPart[CRaceData::PART_WEAPON]))
			TraceError("Error with Shining attachment");
	}
#endif
#ifdef ENABLE_SHINING_EFFECT_UTILITY
	if (pItem->GetType() == CItemData::ITEM_TYPE_WEAPON || (pItem->GetType() == CItemData::ITEM_TYPE_COSTUME && pItem->GetSubType() == CItemData::COSTUME_WEAPON))
	{
		__ClearWeaponShiningEffect();
	}
	else if (pItem->GetType() == CItemData::ITEM_TYPE_ARMOR || (pItem->GetType() == CItemData::ITEM_TYPE_COSTUME && pItem->GetSubType() == CItemData::COSTUME_BODY))
	{
		__ClearArmorShiningEffect();
	}
#ifdef ENABLE_ACCE_SYSTEM
	else if (pItem->GetType() == CItemData::ITEM_TYPE_COSTUME || (pItem->GetSubType() == CItemData::COSTUME_ACCE))
	{
		__ClearAcceShiningEffect();
	}
#endif
#endif

#ifdef ENABLE_SIMPLE_REFINED_EFFECT_CHECK
	DWORD refine = pItem->GetRefine();
#else
	DWORD refine = max(pItem->GetRefine() + pItem->GetSocketCount(), CItemData::ITEM_SOCKET_MAX_NUM) - CItemData::ITEM_SOCKET_MAX_NUM;
#endif
	switch (pItem->GetType())
	{
	case CItemData::ITEM_TYPE_WEAPON:
	{
		__ClearWeaponRefineEffect();
		if (refine < 7)
			return 0;
		switch (pItem->GetSubType())
		{
		case CItemData::WEAPON_DAGGER:
		{
#ifndef ENABLE_ITEM_EVOLUTION_SYSTEM
			m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_SMALLSWORD_REFINED7 + refine - 7;
			m_swordRefineEffectLeft = EFFECT_REFINED + EFFECT_SMALLSWORD_REFINED7_LEFT + refine - 7;
#else
			__GetEvolutionEffect(CItemData::WEAPON_DAGGER, dwWeaponEvolution, refine);
#endif
		}
		break;
		case CItemData::WEAPON_FAN:
		{
#ifndef ENABLE_ITEM_EVOLUTION_SYSTEM
			m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_FANBELL_REFINED7 + refine - 7;
#else
			__GetEvolutionEffect(CItemData::WEAPON_FAN, dwWeaponEvolution, refine);
#endif
		}
		break;
		case CItemData::WEAPON_ARROW:
#ifdef ENABLE_QUIVER_SYSTEM
		case CItemData::WEAPON_QUIVER:
#endif
		case CItemData::WEAPON_BELL:
		{
#ifndef ENABLE_ITEM_EVOLUTION_SYSTEM
			m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_SMALLSWORD_REFINED7 + refine - 7;
#else
			__GetEvolutionEffect(CItemData::WEAPON_BELL, dwWeaponEvolution, refine);
#endif
		}
		break;
		case CItemData::WEAPON_BOW:
		{
#ifndef ENABLE_ITEM_EVOLUTION_SYSTEM
			m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_BOW_REFINED7 + refine - 7;
#else
			__GetEvolutionEffect(CItemData::WEAPON_BOW, dwWeaponEvolution, refine);
#endif
		}
		break;
#ifdef ENABLE_WOLFMAN_CHARACTER
		case CItemData::WEAPON_CLAW:
		{
#ifndef ENABLE_ITEM_EVOLUTION_SYSTEM
			m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_CLAW_REFINED7 + refine - 7;
			m_swordRefineEffectLeft = EFFECT_REFINED + EFFECT_CLAW_REFINED7_LEFT + refine - 7;
#else
			__GetEvolutionEffect(CItemData::WEAPON_CLAW, dwWeaponEvolution, refine);
#endif
		}
		break;
#endif
		default:
#ifndef ENABLE_ITEM_EVOLUTION_SYSTEM
			m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_SWORD_REFINED7 + refine - 7;
#else
			__GetEvolutionEffect(255, dwWeaponEvolution, refine);
#endif
		}
#ifndef ENABLE_ITEM_EVOLUTION_SYSTEM
		if (m_swordRefineEffectRight)
			m_swordRefineEffectRight = __AttachEffect(m_swordRefineEffectRight);
		if (m_swordRefineEffectLeft)
			m_swordRefineEffectLeft = __AttachEffect(m_swordRefineEffectLeft);
#endif
		break;
	}
	break;

	case CItemData::ITEM_TYPE_ARMOR:
	{
		__ClearArmorRefineEffect();

		if (pItem->GetSubType() == CItemData::ARMOR_BODY)
		{
			DWORD vnum = pItem->GetIndex();

			if ((12010 <= vnum && vnum <= 12049)
#ifdef ENABLE_WOLFMAN_CHARACTER
				|| (21080 <= vnum && vnum <= 21089)
#endif
				)
			{
				__AttachEffect(EFFECT_REFINED + EFFECT_BODYARMOR_SPECIAL);
				__AttachEffect(EFFECT_REFINED + EFFECT_BODYARMOR_SPECIAL2);
			}
#ifdef ENABLE_LVL115_ARMOR_EFFECT
			else if (20760 <= vnum && vnum <= 20959) { __AttachEffect(EFFECT_REFINED + EFFECT_BODYARMOR_SPECIAL3); }
#endif //ENABLE_LVL115_ARMOR_EFFECT
		}

		if (refine < 7)
			return 0;

		if (pItem->GetSubType() == CItemData::ARMOR_BODY)
		{
#if defined(ENABLE_ITEM_EVOLUTION_SYSTEM) && defined(ENABLE_ARMOR_EVOLUTION_SYSTEM)
			__GetEvolutionEffect(99, dwArmorEvolution, refine);
#else
			m_armorRefineEffect = EFFECT_REFINED + EFFECT_BODYARMOR_REFINED7 + refine - 7;
			__AttachEffect(m_armorRefineEffect);
#endif
		}
	}
	break;

	case CItemData::ITEM_TYPE_COSTUME:
	{
		switch (pItem->GetSubType())
		{
			case CItemData::COSTUME_WEAPON:
				{
					__ClearWeaponRefineEffect();
				}
				break;

			case CItemData::COSTUME_BODY:
				{
					__ClearArmorRefineEffect();
				}
				break;
		}
	}
	break;
	}
	return 0;
}

#ifdef ENABLE_ITEM_EVOLUTION_SYSTEM
void CInstanceBase::SetWeaponEvolution(DWORD dwEvolution)
{
	m_dwWeaponEvolution = dwEvolution;
}

#ifdef ENABLE_ARMOR_EVOLUTION_SYSTEM
void CInstanceBase::SetArmorEvolution(DWORD dwEvolution)
{
	m_dwArmorEvolution = dwEvolution;
}
#endif

BYTE CInstanceBase::GetEvolutionType(int evolution)
{
	if (evolution == -1)
		evolution = m_dwWeaponEvolution;

	if (evolution > 300)
		return WEAPON_EVOLUTION_TYPE_ALL;
	else if (evolution > 280)
		return WEAPON_EVOLUTION_TYPE_EX_ELEC;
	else if (evolution > 260)
		return WEAPON_EVOLUTION_TYPE_EX_DARK;
	else if (evolution > 240)
		return WEAPON_EVOLUTION_TYPE_EX_WIND;
	else if (evolution > 220)
		return WEAPON_EVOLUTION_TYPE_EX_EARTH;
	else if (evolution > 200)
		return WEAPON_EVOLUTION_TYPE_EX_ICE;
	else if (evolution > 180)
		return WEAPON_EVOLUTION_TYPE_EX_FIRE;
	else if (evolution > 160)
		return WEAPON_EVOLUTION_TYPE_HUMAN;
	else if (evolution > 140)
		return WEAPON_EVOLUTION_TYPE_ALL;
	else if (evolution > 120)
		return WEAPON_EVOLUTION_TYPE_ELEC;
	else if (evolution > 100)
		return WEAPON_EVOLUTION_TYPE_DARK;
	else if (evolution > 80)
		return WEAPON_EVOLUTION_TYPE_WIND;
	else if (evolution > 60)
		return WEAPON_EVOLUTION_TYPE_EARTH;
	else if (evolution > 40)
		return WEAPON_EVOLUTION_TYPE_ICE;
	else if (evolution > 20)
		return WEAPON_EVOLUTION_TYPE_FIRE;

	return WEAPON_EVOLUTION_TYPE_NONE;
}
#endif

#ifdef ENABLE_SHINING_EFFECT_UTILITY
void CInstanceBase::__GetShiningEffect(CItemData* pItem)
{
	bool removeRefineEffect = true;

	CItemData::TItemShiningTable shiningTable = pItem->GetItemShiningTable();

#if defined(ENABLE_COSTUME_SYSTEM) && defined(ENABLE_WEAPON_COSTUME_SYSTEM)
	if (pItem->GetType() == CItemData::ITEM_TYPE_WEAPON || (pItem->GetType() == CItemData::ITEM_TYPE_COSTUME && pItem->GetSubType() == CItemData::COSTUME_WEAPON))
	{
		BYTE bSubType = pItem->GetType() == CItemData::ITEM_TYPE_COSTUME && pItem->GetSubType() == CItemData::COSTUME_WEAPON ? pItem->GetValue(3) : pItem->GetSubType();
#else
	if (pItem->GetType() == CItemData::ITEM_TYPE_WEAPON)
	{
		BYTE bSubType = pItem->GetSubType();
#endif
		__ClearWeaponShiningEffect();

		if (shiningTable.Any() && removeRefineEffect)
			__ClearWeaponRefineEffect();

		for (int i = 0; i < CItemData::ITEM_SHINING_MAX_COUNT; i++)
		{
			if (strcmp(shiningTable.szShinings[i], ""))
			{
				if (bSubType == CItemData::WEAPON_BOW)
				{
					__AttachWeaponShiningEffect(i, shiningTable.szShinings[i], "PART_WEAPON_LEFT", pItem->GetName());
				}
				else
				{
#ifdef ENABLE_WOLFMAN_CHARACTER
					bool twoSidedWeapon = bSubType == CItemData::WEAPON_DAGGER || bSubType == CItemData::WEAPON_CLAW || (IsMountingHorse() && bSubType == CItemData::WEAPON_FAN);
#else
					bool twoSidedWeapon = bSubType == CItemData::WEAPON_DAGGER || (IsMountingHorse() && bSubType == CItemData::WEAPON_FAN);
#endif
					if (twoSidedWeapon)
					{
						__AttachWeaponShiningEffect(i, shiningTable.szShinings[i], "PART_WEAPON_LEFT", pItem->GetName());
					}

					__AttachWeaponShiningEffect(i, shiningTable.szShinings[i], "PART_WEAPON", pItem->GetName());
				}
			}
		}
	}
#if defined(ENABLE_COSTUME_SYSTEM) && defined(ENABLE_ACCE_SYSTEM)
	else if (pItem->GetType() == CItemData::ITEM_TYPE_COSTUME && pItem->GetSubType() == CItemData::COSTUME_ACCE)
	{
		__ClearAcceShiningEffect();

		for (int i = 0; i < CItemData::ITEM_SHINING_MAX_COUNT; i++)
		{
			if (strcmp(shiningTable.szShinings[i], ""))
			{
				__AttachAcceShiningEffect(i, shiningTable.szShinings[i], "Bip01", pItem->GetName());
			}
		}
	}
#endif
#if defined(ENABLE_COSTUME_SYSTEM)
	else if ((pItem->GetType() == CItemData::ITEM_TYPE_ARMOR && pItem->GetSubType() == CItemData::ARMOR_BODY) || (pItem->GetType() == CItemData::ITEM_TYPE_COSTUME && pItem->GetSubType() == CItemData::COSTUME_BODY))
#else
	else if (pItem->GetType() == CItemData::ITEM_TYPE_ARMOR && pItem->GetSubType() == CItemData::ARMOR_BODY)
#endif
	{
		__ClearArmorShiningEffect();

		if (shiningTable.Any() && removeRefineEffect)
			__ClearArmorRefineEffect();

		for (int i = 0; i < CItemData::ITEM_SHINING_MAX_COUNT; i++)
		{
			if (strcmp(shiningTable.szShinings[i], ""))
			{
				__AttachArmorShiningEffect(i, shiningTable.szShinings[i], "Bip01", pItem->GetName());
			}
		}
	}
}

void CInstanceBase::__AttachWeaponShiningEffect(int effectIndex, const char* effectFileName, const char* boneName, const char* itemName)
{
	if (IsAffect(AFFECT_INVISIBILITY))
	{
		return;
	}

	if (effectIndex >= CItemData::ITEM_SHINING_MAX_COUNT)
	{
		return;
	}

	CEffectManager::Instance().RegisterEffect(effectFileName, false, false);

	if (!strcmp(boneName, "PART_WEAPON"))
	{
		const char* c_szRightBoneName;
		m_GraphicThingInstance.GetAttachingBoneName(CRaceData::PART_WEAPON, &c_szRightBoneName);

		if (c_szRightBoneName == NULL)
			return;

		if (strcmp(c_szRightBoneName, ""))
		{
			m_weaponShiningEffects[0][effectIndex] = m_GraphicThingInstance.AttachEffectByName(0, c_szRightBoneName, effectFileName);
		}
	}
	else if (!strcmp(boneName, "PART_WEAPON_LEFT"))
	{
		const char* c_szLeftBoneName;
		m_GraphicThingInstance.GetAttachingBoneName(CRaceData::PART_WEAPON_LEFT, &c_szLeftBoneName);

		if (c_szLeftBoneName == NULL)
			return;

		if (strcmp(c_szLeftBoneName, ""))
		{
			m_weaponShiningEffects[1][effectIndex] = m_GraphicThingInstance.AttachEffectByName(0, c_szLeftBoneName, effectFileName);
		}
	}
	else
	{
		Tracef("Invalid partname for getting attaching bone name. %s - %s", effectFileName, boneName);
	}
}

void CInstanceBase::__AttachArmorShiningEffect(int effectIndex, const char* effectFileName, const char* boneName, const char* itemName)
{
	if (IsAffect(AFFECT_INVISIBILITY))
		return;

	if (effectIndex >= CItemData::ITEM_SHINING_MAX_COUNT)
		return;

	if (!strcmp(boneName, ""))
	{
		Tracef("Empty bone name for attaching armor shining. Effect Index: %i, EffectFileName: %s", effectIndex, effectFileName);
		return;
	}

	CEffectManager::Instance().RegisterEffect(effectFileName, false, false);
	m_armorShiningEffects[effectIndex] = m_GraphicThingInstance.AttachEffectByName(0, boneName, effectFileName);
}

#ifdef ENABLE_ACCE_SYSTEM
void CInstanceBase::__AttachAcceShiningEffect(int effectIndex, const char* effectFileName, const char* boneName, const char* itemName)
{
	if (IsAffect(AFFECT_INVISIBILITY))
		return;

	if (effectIndex >= CItemData::ITEM_SHINING_MAX_COUNT)
		return;

	if (!strcmp(boneName, ""))
	{
		Tracef("Empty bone name for attaching armor shining. Effect Index: %i, EffectFileName: %s", effectIndex, effectFileName);
		return;
	}

	CEffectManager::Instance().RegisterEffect(effectFileName, true, false, itemName);
	m_acceShiningEffects[effectIndex] = m_GraphicThingInstance.AttachEffectByName(0, boneName, effectFileName);
}
#endif

void CInstanceBase::__ClearWeaponShiningEffect(bool detaching)
{
	if (detaching)
	{
		for (int i = 0; i < CItemData::ITEM_SHINING_MAX_COUNT; i++)
		{
			if (m_weaponShiningEffects[0][i])
				__DetachEffect(m_weaponShiningEffects[0][i]);

			if (m_weaponShiningEffects[1][i])
				__DetachEffect(m_weaponShiningEffects[1][i]);
		}
	}

	memset(&m_weaponShiningEffects, 0, sizeof(m_weaponShiningEffects));
}

void CInstanceBase::__ClearArmorShiningEffect(bool detaching)
{
	if (detaching)
	{
		for (int i = 0; i < CItemData::ITEM_SHINING_MAX_COUNT; i++)
			__DetachEffect(m_armorShiningEffects[i]);
	}

	memset(&m_armorShiningEffects, 0, sizeof(m_armorShiningEffects));
}

#ifdef ENABLE_ACCE_SYSTEM
void CInstanceBase::__ClearAcceShiningEffect(bool detaching)
{
	if (detaching)
	{
		for (int i = 0; i < CItemData::ITEM_SHINING_MAX_COUNT; i++)
			__DetachEffect(m_acceShiningEffects[i]);
	}

	memset(&m_acceShiningEffects, 0, sizeof(m_acceShiningEffects));
}
#endif
#endif

#ifdef ENABLE_ITEM_EVOLUTION_SYSTEM
bool CInstanceBase::SetWeapon(DWORD eWeapon, DWORD dwWeaponEvolution)
#else
bool CInstanceBase::SetWeapon(DWORD eWeapon)
#endif
{
	m_GraphicThingInstance.AttachWeapon(eWeapon);
	m_awPart[CRaceData::PART_WEAPON] = eWeapon;

	//Weapon Effect
	CItemData* pItemData;
	if (CItemManager::Instance().GetItemDataPointer(eWeapon, &pItemData))
	{
#if defined(ENABLE_ITEM_EVOLUTION_SYSTEM) && defined(ENABLE_ARMOR_EVOLUTION_SYSTEM)
		__GetRefinedEffect(pItemData, dwWeaponEvolution, 0);
#elif defined(ENABLE_ITEM_EVOLUTION_SYSTEM) && !defined(ENABLE_ARMOR_EVOLUTION_SYSTEM)
		__GetRefinedEffect(pItemData, dwWeaponEvolution);
#else
		__GetRefinedEffect(pItemData);
#endif

#ifdef ENABLE_SHINING_EFFECT_UTILITY
		__GetShiningEffect(pItemData);
#endif
	}
	else
	{
		__ClearWeaponRefineEffect();
#ifdef ENABLE_SHINING_EFFECT_UTILITY
		__ClearWeaponShiningEffect();
#endif
#ifdef ENABLE_SHINING_ITEM_SYSTEM
		if (!AttachShiningEffect(0))
			TraceError("Error with Shining attachment");
#endif
	}

	return true;
}

#ifdef ENABLE_ITEM_EVOLUTION_SYSTEM
void CInstanceBase::ChangeWeapon(DWORD eWeapon, DWORD dwWeaponEvolution)
#else
void CInstanceBase::ChangeWeapon(DWORD eWeapon)
#endif
{
	if (eWeapon == m_GraphicThingInstance.GetPartItemID(CRaceData::PART_WEAPON))
		return;

#ifdef ENABLE_ITEM_EVOLUTION_SYSTEM
	if (SetWeapon(eWeapon, dwWeaponEvolution))
#else
	if (SetWeapon(eWeapon))
#endif
		RefreshState(CRaceMotionData::NAME_WAIT, true);
}

#ifdef ENABLE_ARMOR_EVOLUTION_SYSTEM
bool CInstanceBase::ChangeArmor(DWORD dwArmor, DWORD dwArmorEvolution)
#else
bool CInstanceBase::ChangeArmor(DWORD dwArmor)
#endif
{
	DWORD eShape;
	__ArmorVnumToShape(dwArmor, &eShape);

	if (GetShape() == eShape)
		return false;

	CAffectFlagContainer kAffectFlagContainer;
	kAffectFlagContainer.CopyInstance(m_kAffectFlagContainer);

	DWORD dwVID = GetVirtualID();
	DWORD dwRace = GetRace();
	DWORD eHair = GetPart(CRaceData::PART_HAIR);
#ifdef ENABLE_ACCE_SYSTEM
	DWORD dwAcce = GetPart(CRaceData::PART_ACCE);
	DWORD dwAcceSpecular = GetAcceSpecular();
#endif
#ifdef ENABLE_AURA_COSTUME_SYSTEM
	DWORD eAura = GetPart(CRaceData::PART_AURA);
#endif
	DWORD eWeapon = GetPart(CRaceData::PART_WEAPON);
	float fRot = GetRotation();
	float fAdvRot = GetAdvancingRotation();

	if (IsWalking())
		EndWalking();

	__ClearAffects();

	if (!SetRace(dwRace))
	{
		TraceError("CPythonCharacterManager::ChangeArmor - SetRace VID[%d] Race[%d] ERROR", dwVID, dwRace);
		return false;
	}

#if defined(ENABLE_ITEM_EVOLUTION_SYSTEM) && defined(ENABLE_ARMOR_EVOLUTION_SYSTEM)
	SetArmor(dwArmor, dwArmorEvolution);
#else
	SetArmor(dwArmor);
#endif

	SetHair(eHair);

#ifdef ENABLE_ITEM_EVOLUTION_SYSTEM
	SetWeapon(eWeapon, m_dwWeaponEvolution);
#endif

#ifdef ENABLE_ACCE_SYSTEM
	SetAcce(dwAcce, dwAcceSpecular);
#endif

#ifdef ENABLE_AURA_COSTUME_SYSTEM
	SetAura(eAura);
#endif

	SetRotation(fRot);
	SetAdvancingRotation(fAdvRot);

	__AttachHorseSaddle();

	RefreshState(CRaceMotionData::NAME_WAIT, TRUE);

	SetAffectFlagContainer(kAffectFlagContainer);

	CActorInstance::IEventHandler& rkEventHandler = GetEventHandlerRef();
	rkEventHandler.OnChangeShape();
	return true;
}

DWORD CInstanceBase::__GetRaceType()
{
	return m_eRaceType;
}

void CInstanceBase::RefreshState(DWORD dwMotIndex, bool isLoop)
{
	DWORD dwPartItemID = m_GraphicThingInstance.GetPartItemID(CRaceData::PART_WEAPON);

	BYTE byItemType = 0xff;
	BYTE bySubType = 0xff;

	CItemManager& rkItemMgr = CItemManager::Instance();
	CItemData* pItemData;

	if (rkItemMgr.GetItemDataPointer(dwPartItemID, &pItemData))
	{
		byItemType = pItemData->GetType();
		bySubType = pItemData->GetWeaponType();
	}

	if (IsHoldingPickAxe())
	{
		if (m_kHorse.IsMounting())
			SetMotionMode(CRaceMotionData::MODE_HORSE);
		else
			SetMotionMode(CRaceMotionData::MODE_GENERAL);
	}
	else if (CItemData::ITEM_TYPE_ROD == byItemType)
	{
		if (m_kHorse.IsMounting())
			SetMotionMode(CRaceMotionData::MODE_HORSE);
		else
			SetMotionMode(CRaceMotionData::MODE_FISHING);
	}
	else if (m_kHorse.IsMounting())
	{
		switch (bySubType)
		{
		case CItemData::WEAPON_SWORD:
			SetMotionMode(CRaceMotionData::MODE_HORSE_ONEHAND_SWORD);
			break;

		case CItemData::WEAPON_TWO_HANDED:
			SetMotionMode(CRaceMotionData::MODE_HORSE_TWOHAND_SWORD); // Only Warrior
			break;

		case CItemData::WEAPON_DAGGER:
			SetMotionMode(CRaceMotionData::MODE_HORSE_DUALHAND_SWORD); // Only Assassin
			break;

		case CItemData::WEAPON_FAN:
			SetMotionMode(CRaceMotionData::MODE_HORSE_FAN); // Only Shaman
			break;

		case CItemData::WEAPON_BELL:
			SetMotionMode(CRaceMotionData::MODE_HORSE_BELL); // Only Shaman
			break;

		case CItemData::WEAPON_BOW:
			SetMotionMode(CRaceMotionData::MODE_HORSE_BOW); // Only Shaman
			break;

#ifdef ENABLE_WOLFMAN_CHARACTER
		case CItemData::WEAPON_CLAW:
			SetMotionMode(CRaceMotionData::MODE_HORSE_CLAW); // Only Wolfman
			break;
#endif

		default:
			SetMotionMode(CRaceMotionData::MODE_HORSE);
			break;
		}
	}
	else
	{
		switch (bySubType)
		{
		case CItemData::WEAPON_SWORD:
			SetMotionMode(CRaceMotionData::MODE_ONEHAND_SWORD);
			break;

		case CItemData::WEAPON_TWO_HANDED:
			SetMotionMode(CRaceMotionData::MODE_TWOHAND_SWORD); // Only Warrior
			break;

		case CItemData::WEAPON_DAGGER:
			SetMotionMode(CRaceMotionData::MODE_DUALHAND_SWORD); // Only Assassin
			break;

		case CItemData::WEAPON_BOW:
			SetMotionMode(CRaceMotionData::MODE_BOW); // Only Assassin
			break;

		case CItemData::WEAPON_FAN:
			SetMotionMode(CRaceMotionData::MODE_FAN); // Only Shaman
			break;

		case CItemData::WEAPON_BELL:
			SetMotionMode(CRaceMotionData::MODE_BELL); // Only Shaman
			break;

#ifdef ENABLE_WOLFMAN_CHARACTER
		case CItemData::WEAPON_CLAW:
			SetMotionMode(CRaceMotionData::MODE_CLAW); // Only Wolfman
			break;
#endif
		case CItemData::WEAPON_ARROW:
		default:
			SetMotionMode(CRaceMotionData::MODE_GENERAL);
			break;
		}
	}

	if (isLoop)
		m_GraphicThingInstance.InterceptLoopMotion(dwMotIndex);
	else
		m_GraphicThingInstance.InterceptOnceMotion(dwMotIndex);

	RefreshActorInstance();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Device

void CInstanceBase::RegisterBoundingSphere()
{
	if (!IsStone())
		m_GraphicThingInstance.DeformNoSkin();

	m_GraphicThingInstance.RegisterBoundingSphere();
}

bool CInstanceBase::CreateDeviceObjects()
{
	return m_GraphicThingInstance.CreateDeviceObjects();
}

void CInstanceBase::DestroyDeviceObjects()
{
	m_GraphicThingInstance.DestroyDeviceObjects();
}

void CInstanceBase::Destroy()
{
	DetachTextTail();

	DismountHorse();

	m_kQue_kCmdNew.clear();

	__EffectContainer_Destroy();
	__StoneSmoke_Destroy();

	if (__IsMainInstance())
		__ClearMainInstance();

	m_GraphicThingInstance.Destroy();

	__Initialize();
}

void CInstanceBase::__InitializeRotationSpeed()
{
	SetRotationSpeed(c_fDefaultRotationSpeed);
}

void CInstanceBase::__Warrior_Initialize()
{
	m_kWarrior.m_dwGeomgyeongEffect = 0;
}

void CInstanceBase::__Initialize()
{
	__Warrior_Initialize();
	__StoneSmoke_Inialize();
	__EffectContainer_Initialize();
	__InitializeRotationSpeed();

	SetEventHandler(CActorInstance::IEventHandler::GetEmptyPtr());

	m_kAffectFlagContainer.Clear();

	m_dwLevel = 0;
	m_dwGuildID = 0;
	m_dwEmpireID = 0;

	m_eType = 0;
	m_eRaceType = 0;
	m_eShape = 0;
	m_dwRace = 0;
	m_dwVirtualNumber = 0;

	m_dwBaseCmdTime = 0;
	m_dwBaseChkTime = 0;
	m_dwSkipTime = 0;

	m_GraphicThingInstance.Initialize();

	m_dwAdvActorVID = 0;
	m_dwLastDmgActorVID = 0;

	m_nAverageNetworkGap = 0;
	m_dwNextUpdateHeightTime = 0;

	// Moving by keyboard
	m_iRotatingDirection = DEGREE_DIRECTION_SAME;

	// Moving by mouse
	m_isTextTail = FALSE;
	m_isGoing = FALSE;
	NEW_SetSrcPixelPosition(TPixelPosition(0, 0, 0));
	NEW_SetDstPixelPosition(TPixelPosition(0, 0, 0));

	m_kPPosDust = TPixelPosition(0, 0, 0);

	m_kQue_kCmdNew.clear();

	m_dwLastComboIndex = 0;

	m_swordRefineEffectRight = 0;
	m_swordRefineEffectLeft = 0;
	m_armorRefineEffect = 0;

	m_sAlignment = 0;

	m_byPKMode = 0;
	m_isKiller = false;
	m_isPartyMember = false;

	m_bEnableTCPState = TRUE;

	m_stName = "";

	memset(m_awPart, 0, sizeof(m_awPart));
	memset(m_adwCRCAffectEffect, 0, sizeof(m_adwCRCAffectEffect));
	memset(&m_kMovAfterFunc, 0, sizeof(m_kMovAfterFunc));

	m_bDamageEffectType = false;
	m_dwDuelMode = DUEL_NONE;
	m_dwEmoticonTime = 0;

#ifdef ENABLE_ACCE_SYSTEM
	m_acceRefineEffect = 0;
	m_dwAcceSpecular = 0;
#endif

#ifdef ENABLE_AURA_COSTUME_SYSTEM
	m_auraRefineEffect = 0;
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

#ifdef ENABLE_ITEM_EVOLUTION_SYSTEM
	m_dwWeaponEvolution = 0;
#ifdef ENABLE_ARMOR_EVOLUTION_SYSTEM
	m_dwArmorEvolution = 0;
#endif
#endif

#ifdef ENABLE_SHINING_EFFECT_UTILITY
	__ClearWeaponShiningEffect(false);
	__ClearArmorShiningEffect(false);
	__ClearAcceShiningEffect(false);
#endif

#if defined(ENABLE_MOB_AGGR_LVL_INFO)
	m_dwAIFlag = 0;
#endif
#ifdef ENABLE_SHINING_ITEM_SYSTEM
	for (int s = 0; s < CHR_SHINING_NUM; s++) {
		m_dwShining[s] = 0;
	}
	m_armorShining = 0;
	m_swordShiningRight = 0;
	m_swordShiningLeft = 0;
	m_specialShining = 0;
	m_specialShining2 = 0;
	m_specialShining3 = 0;
	m_wingShining = 0;
#endif

#ifdef ENABLE_MODEL_RENDER_TARGET
	m_IsAlwaysRender = false;
#endif

#ifdef ENABLE_GUILD_LEADER_SYSTEM
	m_bMemberType = 0;
#endif

#ifdef ENABLE_DOG_MODE
	m_dwOriginalRace = 0;
#endif

#ifdef ENABLE_AUTO_HUNT_SYSTEM
	m_hasAttacker = 0;
	m_dwAttacked = 0;
#endif // ENABLE_AUTO_HUNT_SYSTEM
}

CInstanceBase::CInstanceBase() :
	m_GraphicThingInstance(),
	m_kQue_kCmdNew(),
	m_DamageQueue()

{
	__Initialize();
}

CInstanceBase::~CInstanceBase()
{
	Destroy();
}

void CInstanceBase::GetBoundBox(D3DXVECTOR3* vtMin, D3DXVECTOR3* vtMax)
{
	m_GraphicThingInstance.GetBoundBox(vtMin, vtMax);
}

#ifdef ENABLE_QUIVER_SYSTEM
bool CInstanceBase::SetArrow(DWORD eArrow)
{
	if (eArrow)
	{
		CItemData* pItemData;
		if (!CItemManager::Instance().GetItemDataPointer(eArrow, &pItemData))
			return false;

		if (pItemData->GetType() == CItemData::ITEM_TYPE_WEAPON)
		{
			if (pItemData->GetSubType() == CItemData::WEAPON_ARROW)
			{
				m_GraphicThingInstance.SetQuiverEquipped(false);
				m_GraphicThingInstance.SetQuiverEffectID(0);
				return true;
			}

			if (pItemData->GetSubType() == CItemData::WEAPON_QUIVER)
			{
				m_GraphicThingInstance.SetQuiverEquipped(true);
				m_GraphicThingInstance.SetQuiverEffectID(pItemData->GetValue(0));
				return true;
			}
		}
	}

	m_GraphicThingInstance.SetQuiverEquipped(false);
	m_GraphicThingInstance.SetQuiverEffectID(0);
	return false;
}
#endif


#ifdef ENABLE_SHINING_ITEM_SYSTEM
void CInstanceBase::SetShining(BYTE num, DWORD dwShining)
{
	if (m_dwShining[num] == dwShining || dwShining < 0)
		return;

	CItemManager::Instance().SelectItemData(dwShining);
	CItemData* pItemData = CItemManager::Instance().GetSelectedItemDataPointer();

	if (pItemData && pItemData->GetType() == CItemData::ITEM_TYPE_SHINING || dwShining == 0) {
		m_dwShining[num] = dwShining;
		if (!AttachShiningEffect(m_awPart[CRaceData::PART_WEAPON]))
			TraceError("Error with shining attachment");
	}
}
INT CInstanceBase::__GetShiningEffect(WORD slot)
{
	if (m_dwShining[slot] == 0)
		return -1;

	CItemManager::Instance().SelectItemData(m_dwShining[slot]);
	CItemData* pItemData = CItemManager::Instance().GetSelectedItemDataPointer();

	if (pItemData && pItemData->GetType() == CItemData::ITEM_TYPE_SHINING)
	{
		return pItemData->GetValue(0);
	}
	return -1;
}
bool CInstanceBase::ClearShiningEffect()
{
	if (m_swordShiningRight)
	{
		__DetachEffect(m_swordShiningRight);
		m_swordShiningRight = 0;
	}
	if (m_swordShiningLeft)
	{
		__DetachEffect(m_swordShiningLeft);
		m_swordShiningLeft = 0;
	}
	if (m_armorShining)
	{
		__DetachEffect(m_armorShining);
		m_armorShining = 0;
	}
	if (m_specialShining)
	{
		__DetachEffect(m_specialShining);
		m_specialShining = 0;
	}
	if (m_specialShining2)
	{
		__DetachEffect(m_specialShining2);
		m_specialShining2 = 0;
	}
	if (m_specialShining3)
	{
		__DetachEffect(m_specialShining3);
		m_specialShining3 = 0;
	}
	if (m_wingShining)
	{
		__DetachEffect(m_wingShining);
		m_wingShining = 0;
	}
	return true;
}
bool CInstanceBase::AttachShiningEffect(DWORD dwWeapon)
{
	CItemData* pItemData = NULL;
	if (!ClearShiningEffect())
		TraceError("Error while clear shining");

	CItemData* pItem;
	if (dwWeapon > 0)
	{
		CItemManager::Instance().SelectItemData(dwWeapon);
		pItem = CItemManager::Instance().GetSelectedItemDataPointer();
	}
	else
		pItem = NULL;

	for (int i = 0; i < CHR_SHINING_NUM; i++)
	{
		if (__GetShiningEffect(i) >= 0)
		{
			if (i == 0)
			{
				if (pItem)
				{
					if (pItem->GetType() == CItemData::ITEM_TYPE_WEAPON)
					{
						switch (pItem->GetSubType())
						{
						case CItemData::WEAPON_DAGGER:
							m_swordShiningRight = EFFECT_SHINING_WEAPON + EFFECT_SHINING_WEAPON_DAGGER_R + __GetShiningEffect(i);
							m_swordShiningLeft = EFFECT_SHINING_WEAPON + EFFECT_SHINING_WEAPON_DAGGER_L + __GetShiningEffect(i);
							break;
						case CItemData::WEAPON_FAN:
							m_swordShiningRight = EFFECT_SHINING_WEAPON + EFFECT_SHINING_WEAPON_FAN + __GetShiningEffect(i);
							break;
						case CItemData::WEAPON_ARROW:
						case CItemData::WEAPON_BELL:
							m_swordShiningRight = EFFECT_SHINING_WEAPON + EFFECT_SHINING_WEAPON_FAN + __GetShiningEffect(i);
							break;
						case CItemData::WEAPON_BOW:
							m_swordShiningRight = EFFECT_SHINING_WEAPON + EFFECT_SHINING_WEAPON_BOW + __GetShiningEffect(i);
							break;
#ifdef ENABLE_WOLFMAN_CHARACTER
						case CItemData::WEAPON_CLAW:
							m_swordShiningRight = EFFECT_SHINING_WEAPON + EFFECT_SHINING_WEAPON_CLAW_R + __GetShiningEffect(i);
							m_swordShiningLeft = EFFECT_SHINING_WEAPON + EFFECT_SHINING_WEAPON_CLAW_L + __GetShiningEffect(i);
							break;
#endif
						default:
							m_swordShiningRight = EFFECT_SHINING_WEAPON + EFFECT_SHINING_WEAPON_SWORD + __GetShiningEffect(i);
						}
					}
					else if (pItem->GetType() == CItemData::ITEM_TYPE_COSTUME && pItem->GetSubType() == CItemData::COSTUME_WEAPON)
					{
						switch (pItem->GetValue(3))
						{
						case CItemData::WEAPON_DAGGER:
							m_swordShiningRight = EFFECT_SHINING_WEAPON + EFFECT_SHINING_WEAPON_DAGGER_R + __GetShiningEffect(i);
							m_swordShiningLeft = EFFECT_SHINING_WEAPON + EFFECT_SHINING_WEAPON_DAGGER_L + __GetShiningEffect(i);
							break;
						case CItemData::WEAPON_FAN:
							m_swordShiningRight = EFFECT_SHINING_WEAPON + EFFECT_SHINING_WEAPON_FAN + __GetShiningEffect(i);
							break;
						case CItemData::WEAPON_ARROW:
						case CItemData::WEAPON_BELL:
							m_swordShiningRight = EFFECT_SHINING_WEAPON + EFFECT_SHINING_WEAPON_FAN + __GetShiningEffect(i);
							break;
						case CItemData::WEAPON_BOW:
							m_swordShiningRight = EFFECT_SHINING_WEAPON + EFFECT_SHINING_WEAPON_BOW + __GetShiningEffect(i);
							break;
#ifdef ENABLE_WOLFMAN_CHARACTER
						case CItemData::WEAPON_CLAW:
							m_swordShiningRight = EFFECT_SHINING_WEAPON + EFFECT_SHINING_WEAPON_CLAW_R + __GetShiningEffect(i);
							m_swordShiningLeft = EFFECT_SHINING_WEAPON + EFFECT_SHINING_WEAPON_CLAW_L + __GetShiningEffect(i);
							break;
#endif
						default:
							m_swordShiningRight = EFFECT_SHINING_WEAPON + EFFECT_SHINING_WEAPON_SWORD + __GetShiningEffect(i);
						}
					}
				}
				else
				{
					continue;
				}
			}
			else if (i == 1)
			{
				m_armorShining = EFFECT_SHINING_ARMOR + __GetShiningEffect(i);
			}
			else if (i == 2)
				m_specialShining = EFFECT_SHINING_SPECIAL + __GetShiningEffect(i);
			else if (i == 3)
				m_specialShining2 = EFFECT_SHINING_SPECIAL2 + __GetShiningEffect(i);
			else if (i == 4)
				m_specialShining3 = EFFECT_SHINING_SPECIAL3 + __GetShiningEffect(i);
			else if (i == 5)
				m_wingShining = EFFECT_SHINING_WING + __GetShiningEffect(i);
			if (i == 0)
			{
				if (m_swordShiningRight)
					m_swordShiningRight = __AttachEffect(m_swordShiningRight);

				if (m_swordShiningLeft)
					m_swordShiningLeft = __AttachEffect(m_swordShiningLeft);
			}
			else if (i == 1)
			{
				if (m_armorShining)
					m_armorShining = __AttachEffect(m_armorShining);
			}
			else if (i == 2)
			{
				if (m_specialShining)
					m_specialShining = __AttachEffect(m_specialShining);
			}
			else if (i == 3)
			{
				if (m_specialShining2)
					m_specialShining2 = __AttachEffect(m_specialShining2);
			}
			else if (i == 4)
			{
				if (m_specialShining3)
					m_specialShining3 = __AttachEffect(m_specialShining3);
			}
			else if (i == 5)
			{
				if (m_wingShining)
					m_wingShining = __AttachEffect(m_wingShining);
			}
		}
	}
	return true;
}
#endif

#ifdef ENABLE_MODEL_RENDER_TARGET
bool CInstanceBase::IsAlwaysRender()
{
	return m_IsAlwaysRender;
}

void CInstanceBase::SetAlwaysRender(bool val)
{
	m_IsAlwaysRender = val;
}
#endif

#ifdef ENABLE_HIDE_BODY_PARTS
void CInstanceBase::UpdatePartsBySetting(BYTE setting, bool hide)
{
	switch (setting)
	{
		case CPythonSystem::HIDE_ACCE:
			{
				DWORD partAcce = GetPart(CRaceData::PART_ACCE);

				if (hide)
				{
					if (partAcce != 0)
					{
						CPythonSystem::Instance().SetPartBackup(CRaceData::PART_ACCE, partAcce);
						CPythonSystem::Instance().SetPartSpecularBackup(m_dwAcceSpecular);
						ChangeAcce(0, 0.0f);
					}
				}
				else
				{
					ChangeAcce(CPythonSystem::Instance().GetPartBackup(CRaceData::PART_ACCE), CPythonSystem::Instance().GetPartSpecularBackup());
				}
			}
			break;

		case CPythonSystem::HIDE_AURA:
			{
				DWORD partAura = GetPart(CRaceData::PART_AURA);

				if (hide)
				{
					if (partAura != 0)
					{
						CPythonSystem::Instance().SetPartBackup(CRaceData::PART_AURA, partAura);
						ChangeAura(0);
					}
				}
				else
				{
					ChangeAura(CPythonSystem::Instance().GetPartBackup(CRaceData::PART_AURA));
				}
			}
			break;
	}
}
#endif // ENABLE_HIDE_BODY_PARTS

#ifdef ENABLE_DOG_MODE
DWORD CInstanceBase::GetOriginalRace()
{
	return m_dwOriginalRace;
}

void CInstanceBase::SetOriginalRace(DWORD race)
{
	m_dwOriginalRace = race;
}

void CInstanceBase::ChangeRace(DWORD eRace, DWORD eShape)
{
	m_dwRace = eRace;

	if (!m_GraphicThingInstance.SetRace(eRace))
		return;

	m_GraphicThingInstance.SetShape(eShape, 0.0f);

	m_GraphicThingInstance.RefreshActorInstance();
	Refresh(CRaceMotionData::NAME_WAIT, false);
}
#endif