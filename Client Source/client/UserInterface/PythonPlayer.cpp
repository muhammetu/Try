#include "StdAfx.h"
#include "PythonPlayerEventHandler.h"
#include "PythonApplication.h"
#include "PythonItem.h"
#include "../eterbase/Timer.h"

#include "AbstractPlayer.h"
#include "../gamelib/GameLibDefines.h"

#ifdef ENABLE_SCP1453_EXTENTIONS
#include "PythonNetworkStream.h"
#endif

enum
{
	MAIN_RACE_WARRIOR_M,
	MAIN_RACE_ASSASSIN_W,
	MAIN_RACE_SURA_M,
	MAIN_RACE_SHAMAN_W,
	MAIN_RACE_WARRIOR_W,
	MAIN_RACE_ASSASSIN_M,
	MAIN_RACE_SURA_W,
	MAIN_RACE_SHAMAN_M,
#ifdef ENABLE_WOLFMAN_CHARACTER
	MAIN_RACE_WOLFMAN_M,
#endif
	MAIN_RACE_MAX_NUM,
};

#ifdef ENABLE_GOLD_LIMIT_REWORK
const long long POINT_MAGIC_NUMBER = 0xe73ac1da;
#else
const DWORD POINT_MAGIC_NUMBER = 0xe73ac1da;
#endif

#ifdef ENABLE_GOLD_LIMIT_REWORK
void CPythonPlayer::SPlayerStatus::SetPoint(UINT ePoint, long long lPoint)
#else
void CPythonPlayer::SPlayerStatus::SetPoint(UINT ePoint, long lPoint)
#endif
{
	m_alPoint[ePoint] = lPoint ^ POINT_MAGIC_NUMBER;
}

#ifdef ENABLE_GOLD_LIMIT_REWORK
long long CPythonPlayer::SPlayerStatus::GetPoint(UINT ePoint)
#else
long CPythonPlayer::SPlayerStatus::GetPoint(UINT ePoint)
#endif
{
	return m_alPoint[ePoint] ^ POINT_MAGIC_NUMBER;
}

bool CPythonPlayer::AffectIndexToSkillIndex(DWORD dwAffectIndex, DWORD* pdwSkillIndex)
{
	if (m_kMap_dwAffectIndexToSkillIndex.end() == m_kMap_dwAffectIndexToSkillIndex.find(dwAffectIndex))
		return false;

	*pdwSkillIndex = m_kMap_dwAffectIndexToSkillIndex[dwAffectIndex];
	return true;
}

bool CPythonPlayer::AffectIndexToSkillSlotIndex(UINT uAffect, DWORD* pdwSkillSlotIndex)
{
	DWORD dwSkillIndex = m_kMap_dwAffectIndexToSkillIndex[uAffect];

	return GetSkillSlotIndex(dwSkillIndex, pdwSkillSlotIndex);
}

bool CPythonPlayer::__GetPickedActorPtr(CInstanceBase** ppkInstPicked)
{
	CPythonCharacterManager& rkChrMgr = CPythonCharacterManager::Instance();
	CInstanceBase* pkInstPicked = rkChrMgr.OLD_GetPickedInstancePtr();
	if (!pkInstPicked)
		return false;

	*ppkInstPicked = pkInstPicked;
	return true;
}

bool CPythonPlayer::__GetPickedActorID(DWORD* pdwActorID)
{
	CPythonCharacterManager& rkChrMgr = CPythonCharacterManager::Instance();
	return rkChrMgr.OLD_GetPickedInstanceVID(pdwActorID);
}

bool CPythonPlayer::__GetPickedItemID(DWORD* pdwItemID)
{
	CPythonItem& rkItemMgr = CPythonItem::Instance();
	return rkItemMgr.GetPickedItemID(pdwItemID);
}

bool CPythonPlayer::__GetPickedGroundPos(TPixelPosition* pkPPosPicked)
{
	CPythonBackground& rkBG = CPythonBackground::Instance();

	if (rkBG.GetPickingPoint(pkPPosPicked))
	{
		pkPPosPicked->y = -pkPPosPicked->y;
		return true;
	}

	return false;
}

void CPythonPlayer::NEW_GetMainActorPosition(TPixelPosition* pkPPosActor)
{
	IAbstractPlayer& rkPlayer = IAbstractPlayer::GetSingleton();
	CInstanceBase* pInstance = rkPlayer.NEW_GetMainActorPtr();
	if (pInstance)
	{
		pInstance->NEW_GetPixelPosition(pkPPosActor);
	}
	else
	{
		CPythonApplication::Instance().GetCenterPosition(pkPPosActor);
	}
}

bool CPythonPlayer::RegisterEffect(DWORD dwEID, const char* c_szFileName, bool isCache)
{
	if (dwEID >= EFFECT_NUM)
		return false;

	CEffectManager& rkEftMgr = CEffectManager::Instance();
	rkEftMgr.RegisterEffect2(c_szFileName, &m_adwEffect[dwEID], isCache);
	return true;
}

void CPythonPlayer::NEW_ShowEffect(int dwEID, TPixelPosition kPPosDst)
{
	if (dwEID >= EFFECT_NUM)
		return;

	D3DXVECTOR3 kD3DVt3Pos(kPPosDst.x, -kPPosDst.y, kPPosDst.z);
	D3DXVECTOR3 kD3DVt3Dir(0.0f, 0.0f, 1.0f);

	CEffectManager& rkEftMgr = CEffectManager::Instance();
#ifdef ENABLE_GRAPHIC_OPTIMIZATION
	if (dwEID == EFFECT_PICK)
		rkEftMgr.CreateEffect(m_adwEffect[dwEID], kD3DVt3Pos, kD3DVt3Dir, nullptr, true);
	else
		rkEftMgr.CreateEffect(m_adwEffect[dwEID], kD3DVt3Pos, kD3DVt3Dir);
#else
	rkEftMgr.CreateEffect(m_adwEffect[dwEID], kD3DVt3Pos, kD3DVt3Dir);
#endif
}

CInstanceBase* CPythonPlayer::NEW_FindActorPtr(DWORD dwVID)
{
	CPythonCharacterManager& rkChrMgr = CPythonCharacterManager::Instance();
	return rkChrMgr.GetInstancePtr(dwVID);
}

CInstanceBase* CPythonPlayer::NEW_GetMainActorPtr()
{
	return NEW_FindActorPtr(m_dwMainCharacterIndex);
}

///////////////////////////////////////////////////////////////////////////////////////////

void CPythonPlayer::Update()
{
	NEW_RefreshMouseWalkingDirection();

	CPythonPlayerEventHandler& rkPlayerEventHandler = CPythonPlayerEventHandler::GetSingleton();
	rkPlayerEventHandler.FlushVictimList();

	if (m_isDestPosition)
	{
		CInstanceBase* pInstance = NEW_GetMainActorPtr();
		if (pInstance)
		{
			TPixelPosition PixelPosition;
			pInstance->NEW_GetPixelPosition(&PixelPosition);

			if (abs(int(PixelPosition.x) - m_ixDestPos) + abs(int(PixelPosition.y) - m_iyDestPos) < 10000)
			{
				m_isDestPosition = FALSE;
			}
			else
			{
				if (CTimer::Instance().GetCurrentMillisecond() - m_iLastAlarmTime > 20000)
				{
					AlarmHaveToGo();
				}
			}
		}
	}

	if (m_isConsumingStamina)
	{
		float fElapsedTime = CTimer::Instance().GetElapsedSecond();
		m_fCurrentStamina -= (fElapsedTime * m_fConsumeStaminaPerSec);

		SetStatus(POINT_STAMINA, DWORD(m_fCurrentStamina));

		PyCallClassMemberFunc(m_ppyGameWindow, "RefreshStamina", Py_BuildValue("()"));
	}

	DWORD dwCurAutoTime = ELTimer_GetMSec();
	if (CPythonSystem::Instance().IsPickUpMode())
	{
		if (dwCurAutoTime >= m_dwNextTCPAutoPickTime)
		{
			m_dwNextTCPAutoPickTime = dwCurAutoTime + 500;
			PickCloseItem();
		}
	}

	__Update_AutoAttack();
#ifdef ENABLE_AUTO_HUNT_SYSTEM
	if (OtoAvDurumAl()){ UpdateOtoAv(); }
#endif
}

bool CPythonPlayer::__IsUsingChargeSkill()
{
	CInstanceBase* pkInstMain = NEW_GetMainActorPtr();
	if (!pkInstMain)
		return false;

	if (__CheckDashAffect(*pkInstMain))
		return true;

	if (MODE_USE_SKILL != m_eReservedMode)
		return false;

	if (m_dwSkillSlotIndexReserved >= SKILL_MAX_NUM)
		return false;

	TSkillInstance& rkSkillInst = m_playerStatus.aSkill[m_dwSkillSlotIndexReserved];

	CPythonSkill::TSkillData* pSkillData;
	if (!CPythonSkill::Instance().GetSkillData(rkSkillInst.dwIndex, &pSkillData))
		return false;

	return pSkillData->IsChargeSkill() ? true : false;
}

void CPythonPlayer::__Update_AutoAttack()
{
	if (0 == m_dwAutoAttackTargetVID)
		return;

	CInstanceBase* pkInstMain = NEW_GetMainActorPtr();
	if (!pkInstMain)
		return;

	if (__IsUsingChargeSkill())
		return;

	CInstanceBase* pkInstVictim = NEW_FindActorPtr(m_dwAutoAttackTargetVID);
	if (!pkInstVictim)
	{
		__ClearAutoAttackTargetActorID();
	}
	else
	{
		if (pkInstVictim->IsDead())
		{
			__ClearAutoAttackTargetActorID();
		}
#ifdef ENABLE_AUTO_HUNT_SYSTEM
		else if (OtoAvDurumAl() && !OtoAvSaldiriAl()){
			__ClearAutoAttackTargetActorID();
			return;
		}
#endif
		else if (pkInstMain->IsMountingHorse() && !pkInstMain->CanAttackHorseLevel())
		{
			__ClearAutoAttackTargetActorID();
		}
		else if (pkInstMain->IsAttackableInstance(*pkInstVictim))
		{
			// @duzenleme ok yoksa yayla saldiri yapamaz.
			if (pkInstMain->IsBowMode())
			{
				if (!__CanShot(*pkInstMain, *pkInstVictim))
					return;
			}

			__ReserveClickActor(m_dwAutoAttackTargetVID);
		}
	}
}

void CPythonPlayer::SetMainCharacterIndex(int iIndex)
{
	m_dwMainCharacterIndex = iIndex;

	CInstanceBase* pkInstMain = NEW_GetMainActorPtr();
	if (pkInstMain)
	{
		CPythonPlayerEventHandler& rkPlayerEventHandler = CPythonPlayerEventHandler::GetSingleton();
		pkInstMain->SetEventHandler(&rkPlayerEventHandler);
	}
}

#ifdef ENABLE_AUTO_HUNT_SYSTEM
void CPythonPlayer::UpdateOtoAv(){
	CInstanceBase * pkInstMain = NEW_GetMainActorPtr();
	if (!pkInstMain)
	{
		__ClearTarget();
		m_dwAutoAttackTargetVID = 0;
		return;
	}

	bool isPass = false;

	if (sonHedefBulMs != 0 && ((sonHedefBulMs + 21000) <= CTimer::Instance().GetCurrentMillisecond()))
	{
		isPass = true;
		__ClearTarget();
		m_dwAutoAttackTargetVID = 0;
		sonHedefBulMs = 0;
	}

	CInstanceBase* pkTry = CPythonCharacterManager::Instance().GetInstancePtr(m_dwAutoAttackTargetVID);
	if (pkTry) {
		if (pkTry->HasAttacker() != 0) {
			if (pkTry->HasAttacker() != pkInstMain->GetVirtualID()) {
				__ClearTarget();
				m_dwAutoAttackTargetVID = 0;
			}
			if (pkTry->GetAttacked() == 1 && OtoAvOdakAl())
			{
				__ClearTarget();
				m_dwAutoAttackTargetVID = 0;
			}
		}
	}

	if ((m_dwAutoAttackTargetVID == 0 && OtoAvSaldiriAl()) || isPass)
	{
		CInstanceBase* pkInstTarget = CPythonCharacterManager::Instance().OtomatikHedefBul(pkInstMain, isPass);
		if (pkInstTarget)
		{
			SetTarget(pkInstTarget->GetVirtualID());
			m_dwAutoAttackTargetVID = pkInstTarget->GetVirtualID();
			SetLastAttackTime(CTimer::Instance().GetCurrentMillisecond());
		}
	}
}
void CPythonPlayer::OtoAvDurumAta(bool gelenDurum){
	CInstanceBase * pkInstMain = NEW_GetMainActorPtr();
	if (!pkInstMain){ return; }
	otoAvDurum = gelenDurum;
	if (!gelenDurum){
		__ClearReservedAction();
		__ClearAutoAttackTargetActorID();
	}else{
		otoAvBaslangicKonumu = TPixelPosition(0, 0, 0);
		pkInstMain->NEW_GetPixelPosition(&otoAvBaslangicKonumu);
	}
}
#endif

DWORD CPythonPlayer::GetMainCharacterIndex()
{
	return m_dwMainCharacterIndex;
}

bool CPythonPlayer::IsMainCharacterIndex(DWORD dwIndex)
{
	return (m_dwMainCharacterIndex == dwIndex);
}

DWORD CPythonPlayer::GetGuildID()
{
	CInstanceBase* pkInstMain = NEW_GetMainActorPtr();
	if (!pkInstMain)
		return 0xffffffff;

	return pkInstMain->GetGuildID();
}

void CPythonPlayer::SetWeaponPower(DWORD dwMinPower, DWORD dwMaxPower, DWORD dwMinMagicPower, DWORD dwMaxMagicPower, DWORD dwAddPower)
{
	m_dwWeaponMinPower = dwMinPower;
	m_dwWeaponMaxPower = dwMaxPower;
	m_dwWeaponMinMagicPower = dwMinMagicPower;
	m_dwWeaponMaxMagicPower = dwMaxMagicPower;
	m_dwWeaponAddPower = dwAddPower;

	__UpdateBattleStatus();
}

void CPythonPlayer::SetRace(DWORD dwRace)
{
	m_dwRace = dwRace;
}

DWORD CPythonPlayer::GetRace()
{
	return m_dwRace;
}

DWORD CPythonPlayer::__GetRaceStat()
{
	switch (GetRace())
	{
	case MAIN_RACE_WARRIOR_M:
	case MAIN_RACE_WARRIOR_W:
		return GetStatus(POINT_ST);
		break;
	case MAIN_RACE_ASSASSIN_M:
	case MAIN_RACE_ASSASSIN_W:
		return GetStatus(POINT_DX);
		break;
	case MAIN_RACE_SURA_M:
	case MAIN_RACE_SURA_W:
		return GetStatus(POINT_ST);
		break;
	case MAIN_RACE_SHAMAN_M:
	case MAIN_RACE_SHAMAN_W:
		return GetStatus(POINT_IQ);
		break;
#ifdef ENABLE_WOLFMAN_CHARACTER
	case MAIN_RACE_WOLFMAN_M:
		return GetStatus(POINT_DX);
		break;
#endif
	}
	return GetStatus(POINT_ST);
}

DWORD CPythonPlayer::__GetLevelAtk()
{
	return 2 * GetStatus(POINT_LEVEL);
}

DWORD CPythonPlayer::__GetStatAtk()
{
	return (4 * GetStatus(POINT_ST) + 2 * __GetRaceStat()) / 3;
}

DWORD CPythonPlayer::__GetWeaponAtk(DWORD dwWeaponPower)
{
	return 2 * dwWeaponPower;
}

DWORD CPythonPlayer::__GetTotalAtk(DWORD dwWeaponPower, DWORD dwRefineBonus)
{
	auto dwWepAtk = __GetWeaponAtk(dwWeaponPower + dwRefineBonus);
	auto dwTotalAtk = __GetLevelAtk() + (__GetStatAtk() + dwWepAtk) * __GetHitRate() / 100;

	return dwTotalAtk;
}

DWORD CPythonPlayer::__GetHitRate()
{
	auto src = (GetStatus(POINT_DX) * 4 + GetStatus(POINT_LEVEL) * 2) / 6;

	return 100 * (min(90, src) + 210) / 300;
}

DWORD CPythonPlayer::__GetEvadeRate()
{
	return 30 * (2 * GetStatus(POINT_DX) + 5) / (GetStatus(POINT_DX) + 95);
}

void CPythonPlayer::__UpdateBattleStatus()
{
	m_playerStatus.SetPoint(POINT_NONE, 0);
	m_playerStatus.SetPoint(POINT_EVADE_RATE, __GetEvadeRate());
	m_playerStatus.SetPoint(POINT_HIT_RATE, __GetHitRate());
	m_playerStatus.SetPoint(POINT_MIN_WEP, m_dwWeaponMinPower + m_dwWeaponAddPower);
	m_playerStatus.SetPoint(POINT_MAX_WEP, m_dwWeaponMaxPower + m_dwWeaponAddPower);
	m_playerStatus.SetPoint(POINT_MIN_MAGIC_WEP, m_dwWeaponMinMagicPower + m_dwWeaponAddPower);
	m_playerStatus.SetPoint(POINT_MAX_MAGIC_WEP, m_dwWeaponMaxMagicPower + m_dwWeaponAddPower);
	m_playerStatus.SetPoint(POINT_MIN_ATK, __GetTotalAtk(m_dwWeaponMinPower, m_dwWeaponAddPower));
	m_playerStatus.SetPoint(POINT_MAX_ATK, __GetTotalAtk(m_dwWeaponMaxPower, m_dwWeaponAddPower));
}

#ifdef ENABLE_GOLD_LIMIT_REWORK
void CPythonPlayer::SetStatus(DWORD dwType, long long lValue)
#else
void CPythonPlayer::SetStatus(DWORD dwType, long lValue)
#endif
{
	if (dwType >= POINT_MAX_NUM)
	{
		assert(!" CPythonPlayer::SetStatus - Strange Status Type!");
		Tracef("CPythonPlayer::SetStatus - Set Status Type Error\n");
		return;
	}

	if (dwType == POINT_LEVEL)
	{
		CInstanceBase* pkPlayer = NEW_GetMainActorPtr();

		if (pkPlayer)
		{
			// @DIKKAT @duzenleme
			// bu amini siktigim niye hala burda zaten update packet geliyor ???
			// basically, just for the /level command to refresh locally
			pkPlayer->SetLevel(lValue);
			pkPlayer->UpdateTextTailLevel(lValue);
		}
	}

	switch (dwType)
	{
	case POINT_MIN_WEP:
	case POINT_MAX_WEP:
	case POINT_MIN_ATK:
	case POINT_MAX_ATK:
	case POINT_HIT_RATE:
	case POINT_EVADE_RATE:
	case POINT_LEVEL:
	case POINT_ST:
	case POINT_DX:
	case POINT_IQ:
		m_playerStatus.SetPoint(dwType, lValue);
		__UpdateBattleStatus();
		break;
	default:
		m_playerStatus.SetPoint(dwType, lValue);
		break;
	}
}

#ifdef ENABLE_GOLD_LIMIT_REWORK
long long CPythonPlayer::GetStatus(DWORD dwType)
#else
int CPythonPlayer::GetStatus(DWORD dwType)
#endif
{
	if (dwType >= POINT_MAX_NUM)
	{
		assert(!" CPythonPlayer::GetStatus - Strange Status Type!");
		Tracef("CPythonPlayer::GetStatus - Get Status Type Error\n");
		return 0;
	}

	return m_playerStatus.GetPoint(dwType);
}

const char* CPythonPlayer::GetName()
{
	return m_stName.c_str();
}

void CPythonPlayer::SetName(const char* name)
{
	m_stName = name;
}

DWORD CPythonPlayer::GetPlayerID()
{
	return m_dwPlayerID;
}

void CPythonPlayer::SetPlayerID(DWORD id)
{
	m_dwPlayerID = id;
}

void CPythonPlayer::NotifyDeletingCharacterInstance(DWORD dwVID)
{
	if (m_dwMainCharacterIndex == dwVID)
		m_dwMainCharacterIndex = 0;
}

void CPythonPlayer::NotifyCharacterDead(DWORD dwVID)
{
	if (__IsSameTargetVID(dwVID))
	{
		SetTarget(0);
	}
}

void CPythonPlayer::NotifyCharacterUpdate(DWORD dwVID)
{
	if (__IsSameTargetVID(dwVID))
	{
		CInstanceBase* pMainInstance = NEW_GetMainActorPtr();
		CInstanceBase* pTargetInstance = CPythonCharacterManager::Instance().GetInstancePtr(dwVID);
		if (pMainInstance && pTargetInstance)
		{
			if (!pMainInstance->IsTargetableInstance(*pTargetInstance))
			{
				SetTarget(0);
				PyCallClassMemberFunc(m_ppyGameWindow, "CloseTargetBoard", Py_BuildValue("()"));
			}
			else
			{
				PyCallClassMemberFunc(m_ppyGameWindow, "RefreshTargetBoardByVID", Py_BuildValue("(i)", dwVID));
			}
		}
	}
}

void CPythonPlayer::NotifyDeadMainCharacter()
{
	__ClearAutoAttackTargetActorID();
}

void CPythonPlayer::NotifyChangePKMode()
{
	PyCallClassMemberFunc(m_ppyGameWindow, "OnChangePKMode", Py_BuildValue("()"));
}

void CPythonPlayer::MoveItemData(TItemPos SrcCell, TItemPos DstCell)
{
	if (!SrcCell.IsValidCell() || !DstCell.IsValidCell())
		return;

	TItemData src_item(*GetItemData(SrcCell));
	TItemData dst_item(*GetItemData(DstCell));
	SetItemData(DstCell, src_item);
	SetItemData(SrcCell, dst_item);
}

const TItemData* CPythonPlayer::GetItemData(TItemPos Cell) const
{
	if (!Cell.IsValidCell())
		return NULL;

	switch (Cell.window_type)
	{
	case INVENTORY:
	case EQUIPMENT:
		return &m_playerStatus.aItem[Cell.cell];
	case DRAGON_SOUL_INVENTORY:
		return &m_playerStatus.aDSItem[Cell.cell];
#ifdef ENABLE_ADDITIONAL_INVENTORY
	case UPGRADE_INVENTORY:
		return &m_playerStatus.aSSUItem[Cell.cell];
	case BOOK_INVENTORY:
		return &m_playerStatus.aSSBItem[Cell.cell];
	case STONE_INVENTORY:
		return &m_playerStatus.aSSSItem[Cell.cell];
	case FLOWER_INVENTORY:
		return &m_playerStatus.aSSFItem[Cell.cell];
	case ATTR_INVENTORY:
		return &m_playerStatus.aSSAItem[Cell.cell];
	case CHEST_INVENTORY:
		return &m_playerStatus.aSSCItem[Cell.cell];
#endif
#ifdef ENABLE_SWITCHBOT_SYSTEM
	case SWITCHBOT:
		return &m_playerStatus.aSwitchbotItem[Cell.cell];
#endif
	default:
		return NULL;
	}
}

void CPythonPlayer::SetItemData(TItemPos Cell, const TItemData& c_rkItemInst)
{
	if (!Cell.IsValidCell())
		return;

	if (c_rkItemInst.vnum != 0)
	{
		CItemData* pItemData;
		if (!CItemManager::Instance().GetItemDataPointer(c_rkItemInst.vnum, &pItemData))
		{
			TraceError("CPythonPlayer::SetItemData(window_type : %d, dwSlotIndex=%d, itemIndex=%d) - Failed to item data\n", Cell.window_type, Cell.cell, c_rkItemInst.vnum);
			return;
		}
	}

	switch (Cell.window_type)
	{
	case INVENTORY:
	case EQUIPMENT:
		m_playerStatus.aItem[Cell.cell] = c_rkItemInst;
		break;
	case DRAGON_SOUL_INVENTORY:
		m_playerStatus.aDSItem[Cell.cell] = c_rkItemInst;
		break;
#ifdef ENABLE_ADDITIONAL_INVENTORY
	case UPGRADE_INVENTORY:
		m_playerStatus.aSSUItem[Cell.cell] = c_rkItemInst;
		break;
	case BOOK_INVENTORY:
		m_playerStatus.aSSBItem[Cell.cell] = c_rkItemInst;
		break;
	case STONE_INVENTORY:
		m_playerStatus.aSSSItem[Cell.cell] = c_rkItemInst;
		break;
	case FLOWER_INVENTORY:
		m_playerStatus.aSSFItem[Cell.cell] = c_rkItemInst;
		break;
	case ATTR_INVENTORY:
		m_playerStatus.aSSAItem[Cell.cell] = c_rkItemInst;
		break;
	case CHEST_INVENTORY:
		m_playerStatus.aSSCItem[Cell.cell] = c_rkItemInst;
		break;
#endif
#ifdef ENABLE_SWITCHBOT_SYSTEM
	case SWITCHBOT:
		m_playerStatus.aSwitchbotItem[Cell.cell] = c_rkItemInst;
		break;
#endif
	}
}

DWORD CPythonPlayer::GetItemIndex(TItemPos Cell)
{
	if (!Cell.IsValidCell())
		return 0;

	return GetItemData(Cell)->vnum;
}

DWORD CPythonPlayer::GetItemFlags(TItemPos Cell)
{
	if (!Cell.IsValidCell())
		return 0;
	const TItemData* pItem = GetItemData(Cell);
	assert(pItem != NULL);
	return pItem->flags;
}

#ifdef ENABLE_BEGINNER_ITEM_SYSTEM
void CPythonPlayer::SetBasicItem(TItemPos Cell, bool is_basic)
{
	const TItemData* pData = GetItemData(Cell);
	if (!pData)
	{
		TraceError("FAILED CPythonPlayer::SetBasicItem() Cell(%d, %d) item is null", Cell.window_type, Cell.cell);
		return;
	}
	(const_cast<TItemData*>(pData))->is_basic = is_basic;
}
bool CPythonPlayer::IsBasicItem(TItemPos Cell)
{
	if (!Cell.IsValidCell())
		return false;
	const TItemData* pItem = GetItemData(Cell);
	if (pItem == NULL)
		return false;
	else
		return pItem->is_basic;
}
#endif

#ifdef ENABLE_CHANGELOOK_SYSTEM
void CPythonPlayer::SetItemTransmutation(TItemPos Cell, DWORD dwVnum)
{
	if (!Cell.IsValidCell())
		return;

	(const_cast<TItemData*>(GetItemData(Cell)))->transmutation = dwVnum;
}

DWORD CPythonPlayer::GetItemTransmutation(TItemPos Cell)
{
	if (Cell.IsValidCell())
	{
		const TItemData* pkItem = GetItemData(Cell);
		if (pkItem)
			return pkItem->transmutation;
	}

	return 0;
}
#endif

DWORD CPythonPlayer::GetItemAntiFlags(TItemPos Cell)
{
	if (!Cell.IsValidCell())
		return 0;

	const TItemData* pItem = GetItemData(Cell);
	assert(pItem != NULL);
	return pItem->anti_flags;
}

BYTE CPythonPlayer::GetItemTypeBySlot(TItemPos Cell)
{
	if (!Cell.IsValidCell())
		return 0;

	CItemData* pItemDataPtr = NULL;
	if (CItemManager::Instance().GetItemDataPointer(GetItemIndex(Cell), &pItemDataPtr))
		return pItemDataPtr->GetType();
	else
	{
		TraceError("FAILED\t\tCPythonPlayer::GetItemTypeBySlot()\t\tCell(%d, %d) item is NULL", Cell.window_type, Cell.cell);
		return 0;
	}
}

BYTE CPythonPlayer::GetItemSubTypeBySlot(TItemPos Cell)
{
	if (!Cell.IsValidCell())
		return 0;

	CItemData* pItemDataPtr = NULL;
	if (CItemManager::Instance().GetItemDataPointer(GetItemIndex(Cell), &pItemDataPtr))
		return pItemDataPtr->GetSubType();
	else
	{
		TraceError("FAILED\t\tCPythonPlayer::GetItemSubTypeBySlot()\t\tCell(%d, %d) item is NULL", Cell.window_type, Cell.cell);
		return 0;
	}
}

DWORD CPythonPlayer::GetItemCount(TItemPos Cell)
{
	if (!Cell.IsValidCell())
		return 0;
	const TItemData* pItem = GetItemData(Cell);
	if (pItem == NULL)
		return 0;
	else
		return pItem->count;
}

#ifdef ENABLE_SCP1453_EXTENTIONS
bool CPythonPlayer::UseItemByVnum(DWORD dwVnum)
{
	CPythonNetworkStream& rkNetStream = CPythonNetworkStream::Instance();
	TItemPos Cell;

	for (int i = 0; i < c_Inventory_Count; ++i)
	{
		const TItemData& c_rItemData = m_playerStatus.aItem[i];
		if (c_rItemData.vnum == dwVnum)
		{
			Cell.cell = i;
			rkNetStream.SendItemUsePacket(Cell);
			return true;
		}
	}

	return false;
}
#endif

DWORD CPythonPlayer::GetItemCountByVnum(DWORD dwVnum)
{
	DWORD dwCount = 0;

	for (int i = 0; i < c_Inventory_Count; ++i)
	{
		const TItemData& c_rItemData = m_playerStatus.aItem[i];
		if (c_rItemData.vnum == dwVnum)
		{
			dwCount += c_rItemData.count;
		}
	}
#ifdef ENABLE_ADDITIONAL_INVENTORY
	for (int i = 0; i < c_Special_ItemSlot_Count; ++i)
	{
		const TItemData& c_rItemDataUpgrade = m_playerStatus.aSSUItem[i];
		const TItemData& c_rItemDataBook = m_playerStatus.aSSBItem[i];
		const TItemData& c_rItemDataStone = m_playerStatus.aSSSItem[i];
		const TItemData& c_rItemDataFlower = m_playerStatus.aSSFItem[i];
		const TItemData& c_rItemDataAttr = m_playerStatus.aSSAItem[i];
		const TItemData& c_rItemDataChest = m_playerStatus.aSSCItem[i];
		if (c_rItemDataBook.vnum == dwVnum)
		{
			dwCount += c_rItemDataBook.count;
		}
		if (c_rItemDataStone.vnum == dwVnum)
		{
			dwCount += c_rItemDataStone.count;
		}
		if (c_rItemDataUpgrade.vnum == dwVnum)
		{
			dwCount += c_rItemDataUpgrade.count;
		}

		if (c_rItemDataFlower.vnum == dwVnum)
		{
			dwCount += c_rItemDataFlower.count;
		}

		if (c_rItemDataAttr.vnum == dwVnum)
		{
			dwCount += c_rItemDataAttr.count;
		}

		if (c_rItemDataChest.vnum == dwVnum)
		{
			dwCount += c_rItemDataChest.count;
		}
	}
#endif
	return dwCount;
}

DWORD CPythonPlayer::GetItemMetinSocket(TItemPos Cell, DWORD dwMetinSocketIndex)
{
	if (!Cell.IsValidCell())
		return 0;

	if (dwMetinSocketIndex >= ITEM_SOCKET_SLOT_MAX_NUM)
		return 0;

	return GetItemData(Cell)->alSockets[dwMetinSocketIndex];
}

void CPythonPlayer::GetItemAttribute(TItemPos Cell, DWORD dwAttrSlotIndex, BYTE* pbyType, short* psValue)
{
	*pbyType = 0;
	*psValue = 0;

	if (!Cell.IsValidCell())
		return;

	if (dwAttrSlotIndex >= ITEM_ATTRIBUTE_SLOT_MAX_NUM)
		return;

	*pbyType = GetItemData(Cell)->aAttr[dwAttrSlotIndex].bType;
	*psValue = GetItemData(Cell)->aAttr[dwAttrSlotIndex].sValue;
}

BYTE CPythonPlayer::GetItemAttributeType(TItemPos Cell, DWORD dwAttrSlotIndex)
{
	BYTE pbyType;
	short unknown;
	GetItemAttribute(Cell, dwAttrSlotIndex, &pbyType, &unknown);
	return pbyType;
}

short CPythonPlayer::GetItemAttributeValue(TItemPos Cell, DWORD dwAttrSlotIndex)
{
	BYTE unknown;
	short psValue;
	GetItemAttribute(Cell, dwAttrSlotIndex, &unknown, &psValue);
	return unknown;
}

#ifdef ENABLE_ITEM_EVOLUTION_SYSTEM
void CPythonPlayer::SetItemEvolution(TItemPos Cell, DWORD evolution)
{
	if (!Cell.IsValidCell())
		return;

	(const_cast<TItemData*>(GetItemData(Cell)))->evolution = evolution;
}

DWORD CPythonPlayer::GetItemEvolution(TItemPos Cell)
{
	if (!Cell.IsValidCell())
		return 0;

	const TItemData* pItem = GetItemData(Cell);
	if (pItem == NULL)
		return 0;
	else
		return pItem->evolution;
}
#endif

#ifdef ENABLE_ITEM_COUNT_LIMIT
void CPythonPlayer::SetItemCount(TItemPos Cell, DWORD byCount)
#else
void CPythonPlayer::SetItemCount(TItemPos Cell, BYTE byCount)
#endif
{
	if (!Cell.IsValidCell())
		return;

	(const_cast<TItemData*>(GetItemData(Cell)))->count = byCount;

#ifdef ENABLE_ADDITIONAL_INVENTORY
	if (Cell.window_type == DRAGON_SOUL_INVENTORY)
		PyCallClassMemberFunc(m_ppyGameWindow, "RefreshInventory", Py_BuildValue("(s)", "OTHER"));
#endif
}

void CPythonPlayer::SetItemMetinSocket(TItemPos Cell, DWORD dwMetinSocketIndex, DWORD dwMetinNumber)
{
	if (!Cell.IsValidCell())
		return;
	if (dwMetinSocketIndex >= ITEM_SOCKET_SLOT_MAX_NUM)
		return;

	(const_cast<TItemData*>(GetItemData(Cell)))->alSockets[dwMetinSocketIndex] = dwMetinNumber;
}

void CPythonPlayer::SetItemAttribute(TItemPos Cell, DWORD dwAttrIndex, BYTE byType, short sValue)
{
	if (!Cell.IsValidCell())
		return;
	if (dwAttrIndex >= ITEM_ATTRIBUTE_SLOT_MAX_NUM)
		return;

	(const_cast<TItemData*>(GetItemData(Cell)))->aAttr[dwAttrIndex].bType = byType;
	(const_cast<TItemData*>(GetItemData(Cell)))->aAttr[dwAttrIndex].sValue = sValue;
}

int CPythonPlayer::GetQuickPage()
{
	return m_playerStatus.lQuickPageIndex;
}

void CPythonPlayer::SetQuickPage(int nQuickPageIndex)
{
	if (nQuickPageIndex < 0)
		m_playerStatus.lQuickPageIndex = QUICKSLOT_MAX_LINE + nQuickPageIndex;
	else if (nQuickPageIndex >= QUICKSLOT_MAX_LINE)
		m_playerStatus.lQuickPageIndex = nQuickPageIndex % QUICKSLOT_MAX_LINE;
	else
		m_playerStatus.lQuickPageIndex = nQuickPageIndex;

#ifndef ENABLE_ADDITIONAL_INVENTORY
	PyCallClassMemberFunc(m_ppyGameWindow, "RefreshInventory", Py_BuildValue("()"));
#else
	PyCallClassMemberFunc(m_ppyGameWindow, "RefreshInventory", Py_BuildValue("(s)", "ALL"));
#endif
}

DWORD CPythonPlayer::LocalQuickSlotIndexToGlobalQuickSlotIndex(DWORD dwLocalSlotIndex)
{
	return m_playerStatus.lQuickPageIndex * QUICKSLOT_MAX_COUNT_PER_LINE + dwLocalSlotIndex;
}

void CPythonPlayer::GetGlobalQuickSlotData(DWORD dwGlobalSlotIndex, DWORD* pdwWndType, DWORD* pdwWndItemPos)
{
	TQuickSlot& rkQuickSlot = __RefGlobalQuickSlot(dwGlobalSlotIndex);
	*pdwWndType = rkQuickSlot.Type;
	*pdwWndItemPos = rkQuickSlot.Position;
}

void CPythonPlayer::GetLocalQuickSlotData(DWORD dwSlotPos, DWORD* pdwWndType, DWORD* pdwWndItemPos)
{
	TQuickSlot& rkQuickSlot = __RefLocalQuickSlot(dwSlotPos);
	*pdwWndType = rkQuickSlot.Type;
	*pdwWndItemPos = rkQuickSlot.Position;
}

TQuickSlot& CPythonPlayer::__RefLocalQuickSlot(int SlotIndex)
{
	return __RefGlobalQuickSlot(LocalQuickSlotIndexToGlobalQuickSlotIndex(SlotIndex));
}

TQuickSlot& CPythonPlayer::__RefGlobalQuickSlot(int SlotIndex)
{
	if (SlotIndex < 0 || SlotIndex >= QUICKSLOT_MAX_NUM)
	{
		static TQuickSlot s_kQuickSlot;
		s_kQuickSlot.Type = 0;
		s_kQuickSlot.Position = 0;
		return s_kQuickSlot;
	}

	return m_playerStatus.aQuickSlot[SlotIndex];
}

void CPythonPlayer::RemoveQuickSlotByValue(int iType, int iPosition)
{
	for (BYTE i = 0; i < QUICKSLOT_MAX_NUM; ++i)
	{
		if (iType == m_playerStatus.aQuickSlot[i].Type)
			if (iPosition == m_playerStatus.aQuickSlot[i].Position)
				CPythonNetworkStream::Instance().SendQuickSlotDelPacket(i);
	}
}

char CPythonPlayer::IsItem(TItemPos Cell)
{
	if (!Cell.IsValidCell())
		return 0;

	return 0 != GetItemData(Cell)->vnum;
}

void CPythonPlayer::RequestMoveGlobalQuickSlotToLocalQuickSlot(DWORD dwGlobalSrcSlotIndex, DWORD dwLocalDstSlotIndex)
{
	//DWORD dwGlobalSrcSlotIndex=LocalQuickSlotIndexToGlobalQuickSlotIndex(dwLocalSrcSlotIndex);
	DWORD dwGlobalDstSlotIndex = LocalQuickSlotIndexToGlobalQuickSlotIndex(dwLocalDstSlotIndex);

	CPythonNetworkStream& rkNetStream = CPythonNetworkStream::Instance();
	rkNetStream.SendQuickSlotMovePacket((BYTE)dwGlobalSrcSlotIndex, (BYTE)dwGlobalDstSlotIndex);
}

void CPythonPlayer::RequestAddLocalQuickSlot(DWORD dwLocalSlotIndex, DWORD dwWndType, DWORD dwWndItemPos)
{
	if (dwLocalSlotIndex >= QUICKSLOT_MAX_COUNT_PER_LINE)
		return;

	DWORD dwGlobalSlotIndex = LocalQuickSlotIndexToGlobalQuickSlotIndex(dwLocalSlotIndex);

	CPythonNetworkStream& rkNetStream = CPythonNetworkStream::Instance();
	rkNetStream.SendQuickSlotAddPacket((BYTE)dwGlobalSlotIndex, (BYTE)dwWndType, (BYTE)dwWndItemPos);
}

void CPythonPlayer::RequestAddToEmptyLocalQuickSlot(DWORD dwWndType, DWORD dwWndItemPos)
{
	for (int i = 0; i < QUICKSLOT_MAX_COUNT_PER_LINE; ++i)
	{
		TQuickSlot& rkQuickSlot = __RefLocalQuickSlot(i);

		if (0 == rkQuickSlot.Type)
		{
			DWORD dwGlobalQuickSlotIndex = LocalQuickSlotIndexToGlobalQuickSlotIndex(i);
			CPythonNetworkStream& rkNetStream = CPythonNetworkStream::Instance();
			rkNetStream.SendQuickSlotAddPacket((BYTE)dwGlobalQuickSlotIndex, (BYTE)dwWndType, (BYTE)dwWndItemPos);
			return;
		}
	}
}

void CPythonPlayer::RequestDeleteGlobalQuickSlot(DWORD dwGlobalSlotIndex)
{
	if (dwGlobalSlotIndex >= QUICKSLOT_MAX_COUNT)
		return;

	//if (dwLocalSlotIndex>=QUICKSLOT_MAX_SLOT_PER_LINE)
	//	return;

	//DWORD dwGlobalSlotIndex=LocalQuickSlotIndexToGlobalQuickSlotIndex(dwLocalSlotIndex);

	CPythonNetworkStream& rkNetStream = CPythonNetworkStream::Instance();
	rkNetStream.SendQuickSlotDelPacket((BYTE)dwGlobalSlotIndex);
}

void CPythonPlayer::RequestUseLocalQuickSlot(DWORD dwLocalSlotIndex)
{
	if (dwLocalSlotIndex >= QUICKSLOT_MAX_COUNT_PER_LINE)
		return;

	DWORD dwRegisteredType;
	DWORD dwRegisteredItemPos;
	GetLocalQuickSlotData(dwLocalSlotIndex, &dwRegisteredType, &dwRegisteredItemPos);

	switch (dwRegisteredType)
	{
	case SLOT_TYPE_INVENTORY:
	{
		CPythonNetworkStream& rkNetStream = CPythonNetworkStream::Instance();
		rkNetStream.SendItemUsePacket(TItemPos(INVENTORY, (WORD)dwRegisteredItemPos));
		break;
	}
	case SLOT_TYPE_SKILL:
	{
		ClickSkillSlot(dwRegisteredItemPos);
		break;
	}
	case SLOT_TYPE_EMOTION:
	{
		PyCallClassMemberFunc(m_ppyGameWindow, "BINARY_ActEmotion", Py_BuildValue("(i)", dwRegisteredItemPos));
		break;
	}
	}
}

void CPythonPlayer::AddQuickSlot(int QuickSlotIndex, char IconType, char IconPosition)
{
	if (QuickSlotIndex < 0 || QuickSlotIndex >= QUICKSLOT_MAX_NUM)
		return;

	m_playerStatus.aQuickSlot[QuickSlotIndex].Type = IconType;
	m_playerStatus.aQuickSlot[QuickSlotIndex].Position = IconPosition;
}

void CPythonPlayer::DeleteQuickSlot(int QuickSlotIndex)
{
	if (QuickSlotIndex < 0 || QuickSlotIndex >= QUICKSLOT_MAX_NUM)
		return;

	m_playerStatus.aQuickSlot[QuickSlotIndex].Type = 0;
	m_playerStatus.aQuickSlot[QuickSlotIndex].Position = 0;
}

void CPythonPlayer::MoveQuickSlot(int Source, int Target)
{
	if (Source < 0 || Source >= QUICKSLOT_MAX_NUM)
		return;

	if (Target < 0 || Target >= QUICKSLOT_MAX_NUM)
		return;

	TQuickSlot& rkSrcSlot = __RefGlobalQuickSlot(Source);
	TQuickSlot& rkDstSlot = __RefGlobalQuickSlot(Target);

	std::swap(rkSrcSlot, rkDstSlot);
}

#ifdef ENABLE_NEW_EQUIPMENT_SYSTEM
bool CPythonPlayer::IsBeltInventorySlot(TItemPos Cell)
{
	return Cell.IsBeltInventoryCell();
}
#endif

bool CPythonPlayer::IsInventorySlot(TItemPos Cell)
{
	return !Cell.IsEquipCell() && Cell.IsValidCell();
}

bool CPythonPlayer::IsEquipmentSlot(TItemPos Cell)
{
	return Cell.IsEquipCell();
}

bool CPythonPlayer::IsEquipItemInSlot(TItemPos Cell)
{
	if (!Cell.IsEquipCell())
	{
		return false;
	}

	const TItemData* pData = GetItemData(Cell);

	if (NULL == pData)
	{
		return false;
	}

	DWORD dwItemIndex = pData->vnum;

	CItemManager::Instance().SelectItemData(dwItemIndex);
	CItemData* pItemData = CItemManager::Instance().GetSelectedItemDataPointer();
	if (!pItemData)
	{
		TraceError("Failed to find ItemData - CPythonPlayer::IsEquipItem(window_type=%d, iSlotindex=%d)\n", Cell.window_type, Cell.cell);
		return false;
	}

	return pItemData->IsEquipment() ? true : false;
}

void CPythonPlayer::SetSkill(DWORD dwSlotIndex, DWORD dwSkillIndex)
{
	if (dwSlotIndex >= SKILL_MAX_NUM)
		return;

	m_playerStatus.aSkill[dwSlotIndex].dwIndex = dwSkillIndex;
	m_skillSlotDict[dwSkillIndex] = dwSlotIndex;
}

int CPythonPlayer::GetSkillIndex(DWORD dwSlotIndex)
{
	if (dwSlotIndex >= SKILL_MAX_NUM)
		return 0;

	return m_playerStatus.aSkill[dwSlotIndex].dwIndex;
}

bool CPythonPlayer::GetSkillSlotIndex(DWORD dwSkillIndex, DWORD* pdwSlotIndex)
{
	std::map<DWORD, DWORD>::iterator f = m_skillSlotDict.find(dwSkillIndex);
	if (m_skillSlotDict.end() == f)
	{
		return false;
	}

	*pdwSlotIndex = f->second;

	return true;
}

int CPythonPlayer::GetSkillGrade(DWORD dwSlotIndex)
{
	if (dwSlotIndex >= SKILL_MAX_NUM)
		return 0;

	return m_playerStatus.aSkill[dwSlotIndex].iGrade;
}

int CPythonPlayer::GetSkillLevel(DWORD dwSlotIndex)
{
	if (dwSlotIndex >= SKILL_MAX_NUM)
		return 0;

	return m_playerStatus.aSkill[dwSlotIndex].iLevel;
}

float CPythonPlayer::GetSkillCurrentEfficientPercentage(DWORD dwSlotIndex)
{
	if (dwSlotIndex >= SKILL_MAX_NUM)
		return 0;

	return m_playerStatus.aSkill[dwSlotIndex].fcurEfficientPercentage;
}

float CPythonPlayer::GetSkillNextEfficientPercentage(DWORD dwSlotIndex)
{
	if (dwSlotIndex >= SKILL_MAX_NUM)
		return 0;

	return m_playerStatus.aSkill[dwSlotIndex].fnextEfficientPercentage;
}

void CPythonPlayer::SetSkillLevel(DWORD dwSlotIndex, DWORD dwSkillLevel)
{
	assert(!"CPythonPlayer::SetSkillLevel - Don't use this function");
	if (dwSlotIndex >= SKILL_MAX_NUM)
		return;

	m_playerStatus.aSkill[dwSlotIndex].iGrade = -1;
	m_playerStatus.aSkill[dwSlotIndex].iLevel = dwSkillLevel;
}

void CPythonPlayer::SetSkillLevel_(DWORD dwSkillIndex, DWORD dwSkillGrade, DWORD dwSkillLevel)
{
	DWORD dwSlotIndex;
	if (!GetSkillSlotIndex(dwSkillIndex, &dwSlotIndex))
		return;

	if (dwSlotIndex >= SKILL_MAX_NUM)
		return;

	switch (dwSkillGrade)
	{
	case 0:
		m_playerStatus.aSkill[dwSlotIndex].iGrade = dwSkillGrade;
		m_playerStatus.aSkill[dwSlotIndex].iLevel = dwSkillLevel;
		break;
	case 1:
		m_playerStatus.aSkill[dwSlotIndex].iGrade = dwSkillGrade;
		m_playerStatus.aSkill[dwSlotIndex].iLevel = dwSkillLevel - 20 + 1;
		break;
	case 2:
		m_playerStatus.aSkill[dwSlotIndex].iGrade = dwSkillGrade;
		m_playerStatus.aSkill[dwSlotIndex].iLevel = dwSkillLevel - 30 + 1;
		break;
	case 3:
		m_playerStatus.aSkill[dwSlotIndex].iGrade = dwSkillGrade;
		m_playerStatus.aSkill[dwSlotIndex].iLevel = dwSkillLevel - 40 + 1;
		break;
#ifdef ENABLE_SAGE_SKILL
	case 4:
		m_playerStatus.aSkill[dwSlotIndex].iGrade = dwSkillGrade;
		m_playerStatus.aSkill[dwSlotIndex].iLevel = dwSkillLevel - 50 + 1;
		break;
#endif
#ifdef ENABLE_EXPERT_SKILL
	case 5:
		m_playerStatus.aSkill[dwSlotIndex].iGrade = dwSkillGrade;
		m_playerStatus.aSkill[dwSlotIndex].iLevel = dwSkillLevel - 60 + 1;
		break;
	case 6:
		m_playerStatus.aSkill[dwSlotIndex].iGrade = dwSkillGrade;
		m_playerStatus.aSkill[dwSlotIndex].iLevel = dwSkillLevel - 70 + 1;
		break;
	case 7:
		m_playerStatus.aSkill[dwSlotIndex].iGrade = dwSkillGrade;
		m_playerStatus.aSkill[dwSlotIndex].iLevel = dwSkillLevel - 80 + 1;
		break;
#endif
	}

#ifdef ENABLE_SAGE_SKILL // ENABLE_EXPERT_SKILL
	const DWORD SKILL_MAX_LEVEL = 50 + 10 + 10 + 10;
#else
	const DWORD SKILL_MAX_LEVEL = 40;
#endif

	if (dwSkillLevel > SKILL_MAX_LEVEL)
	{
		m_playerStatus.aSkill[dwSlotIndex].fcurEfficientPercentage = 0.0f;
		m_playerStatus.aSkill[dwSlotIndex].fnextEfficientPercentage = 0.0f;

		TraceError("CPythonPlayer::SetSkillLevel(SlotIndex=%d, SkillLevel=%d)", dwSlotIndex, dwSkillLevel);
		return;
	}

	m_playerStatus.aSkill[dwSlotIndex].fcurEfficientPercentage = LocaleService_GetSkillPower(dwSkillLevel) / 100.0f;
	m_playerStatus.aSkill[dwSlotIndex].fnextEfficientPercentage = LocaleService_GetSkillPower(dwSkillLevel + 1) / 100.0f;
}

void CPythonPlayer::SetSkillCoolTime(DWORD dwSkillIndex)
{
	DWORD dwSlotIndex;
	if (!GetSkillSlotIndex(dwSkillIndex, &dwSlotIndex))
	{
		Tracenf("CPythonPlayer::SetSkillCoolTime(dwSkillIndex=%d) - FIND SLOT ERROR", dwSkillIndex);
		return;
	}

	if (dwSlotIndex >= SKILL_MAX_NUM)
	{
		Tracenf("CPythonPlayer::SetSkillCoolTime(dwSkillIndex=%d) - dwSlotIndex=%d/%d OUT OF RANGE", dwSkillIndex, dwSlotIndex, SKILL_MAX_NUM);
		return;
	}

	m_playerStatus.aSkill[dwSlotIndex].isCoolTime = true;
}

void CPythonPlayer::EndSkillCoolTime(DWORD dwSkillIndex)
{
	DWORD dwSlotIndex;
	if (!GetSkillSlotIndex(dwSkillIndex, &dwSlotIndex))
	{
		Tracenf("CPythonPlayer::EndSkillCoolTime(dwSkillIndex=%d) - FIND SLOT ERROR", dwSkillIndex);
		return;
	}

	if (dwSlotIndex >= SKILL_MAX_NUM)
	{
		Tracenf("CPythonPlayer::EndSkillCoolTime(dwSkillIndex=%d) - dwSlotIndex=%d/%d OUT OF RANGE", dwSkillIndex, dwSlotIndex, SKILL_MAX_NUM);
		return;
	}

	m_playerStatus.aSkill[dwSlotIndex].isCoolTime = false;
}

float CPythonPlayer::GetSkillCoolTime(DWORD dwSlotIndex)
{
	if (dwSlotIndex >= SKILL_MAX_NUM)
		return 0.0f;

	return m_playerStatus.aSkill[dwSlotIndex].fCoolTime;
}

float CPythonPlayer::GetSkillElapsedCoolTime(DWORD dwSlotIndex)
{
	if (dwSlotIndex >= SKILL_MAX_NUM)
		return 0.0f;

	return CTimer::Instance().GetCurrentSecond() - m_playerStatus.aSkill[dwSlotIndex].fLastUsedTime;
}

void CPythonPlayer::__ActivateSkillSlot(DWORD dwSlotIndex)
{
	if (dwSlotIndex >= SKILL_MAX_NUM)
	{
		Tracenf("CPythonPlayer::ActivavteSkill(dwSlotIndex=%d/%d) - OUT OF RANGE", dwSlotIndex, SKILL_MAX_NUM);
		return;
	}

	m_playerStatus.aSkill[dwSlotIndex].bActive = TRUE;
	PyCallClassMemberFunc(m_ppyGameWindow, "ActivateSkillSlot", Py_BuildValue("(i)", dwSlotIndex));
}

void CPythonPlayer::__DeactivateSkillSlot(DWORD dwSlotIndex)
{
	if (dwSlotIndex >= SKILL_MAX_NUM)
	{
		Tracenf("CPythonPlayer::DeactivavteSkill(dwSlotIndex=%d/%d) - OUT OF RANGE", dwSlotIndex, SKILL_MAX_NUM);
		return;
	}

	m_playerStatus.aSkill[dwSlotIndex].bActive = FALSE;
	PyCallClassMemberFunc(m_ppyGameWindow, "DeactivateSkillSlot", Py_BuildValue("(i)", dwSlotIndex));
}

BOOL CPythonPlayer::IsSkillCoolTime(DWORD dwSlotIndex)
{
	if (!__CheckRestSkillCoolTime(dwSlotIndex))
		return FALSE;

	return TRUE;
}

BOOL CPythonPlayer::IsSkillActive(DWORD dwSlotIndex)
{
	if (dwSlotIndex >= SKILL_MAX_NUM)
		return FALSE;

	return m_playerStatus.aSkill[dwSlotIndex].bActive;
}

BOOL CPythonPlayer::IsToggleSkill(DWORD dwSlotIndex)
{
	if (dwSlotIndex >= SKILL_MAX_NUM)
		return FALSE;

	DWORD dwSkillIndex = m_playerStatus.aSkill[dwSlotIndex].dwIndex;

	CPythonSkill::TSkillData* pSkillData;
	if (!CPythonSkill::Instance().GetSkillData(dwSkillIndex, &pSkillData))
		return FALSE;

	return pSkillData->IsToggleSkill();
}

void CPythonPlayer::SetPlayTime(DWORD dwPlayTime)
{
#ifdef ENABLE_ANTICHEAT
	tokucbabo tokucbaboex = nullptr;
	CPythonPlayer::ptrti = (void*)(&m_dwTargetVID);
	CPythonPlayer::ptrit = (void*)(&m_isAtkKey);
	tokucbaboex = (tokucbabo)GetProcAddress(GetModuleHandleA("rascal.dll"), "tokucbabo");
	tokucbaboex(ATTACKINIT, CPythonPlayer::ptrti, NULL);
	tokucbaboex(ATTACKKEYINIT, CPythonPlayer::ptrit, NULL);
#endif
	m_dwPlayTime = dwPlayTime;
}

DWORD CPythonPlayer::GetPlayTime()
{
	return m_dwPlayTime;
}

void CPythonPlayer::SendClickItemPacket(DWORD dwIID)
{
	if (IsObserverMode())
		return;

#ifndef ENABLE_PICKUP_ITEM_REWORK
	static DWORD s_dwNextTCPTime = 0;
	DWORD dwCurTime = ELTimer_GetMSec();
#endif

#ifndef ENABLE_PICKUP_ITEM_REWORK
	if (dwCurTime >= s_dwNextTCPTime)
#endif
	{
#ifndef ENABLE_PICKUP_ITEM_REWORK
		s_dwNextTCPTime = dwCurTime + 500;
#endif

		const char* c_szOwnerName;
		if (!CPythonItem::Instance().GetOwnership(dwIID, &c_szOwnerName))
			return;

		if (strlen(c_szOwnerName) > 0)
			if (0 != strcmp(c_szOwnerName, GetName()))
			{
				CItemData* pItemData;
				if (!CItemManager::Instance().GetItemDataPointer(CPythonItem::Instance().GetVirtualNumberOfGroundItem(dwIID), &pItemData))
				{
					Tracenf("CPythonPlayer::SendClickItemPacket(dwIID=%d) : Non-exist item.", dwIID);
					return;
				}
				if (!IsPartyMemberByName(c_szOwnerName) || pItemData->IsAntiFlag(CItemData::ITEM_ANTIFLAG_DROP | CItemData::ITEM_ANTIFLAG_GIVE))
				{
					PyCallClassMemberFunc(m_ppyGameWindow, "OnCannotPickItem", Py_BuildValue("()"));
					return;
				}
			}

#ifdef ENABLE_PICKUP_ITEM_SOUND
		CItemData* pItemData;
		if (!CItemManager::Instance().GetItemDataPointer(CPythonItem::Instance().GetVirtualNumberOfGroundItem(dwIID), &pItemData))
			return;

		std::string base = "sound/ui/";
		switch (pItemData->GetType())
		{
		case CItemData::ITEM_TYPE_ELK:
			base += "money.wav";
			break;
		case CItemData::ITEM_TYPE_WEAPON:
			base += "bracelet.wav";
			break;
		case CItemData::ITEM_TYPE_ARMOR:
			base += "pickup_item_in_inventory.wav";
			break;
		default:
			base += "close.wav";
		}
		CSoundManager::Instance().PlaySound2D(base.c_str());
#endif

		CPythonNetworkStream& rkNetStream = CPythonNetworkStream::Instance();
		rkNetStream.SendItemPickUpPacket(dwIID);


	}
}

void CPythonPlayer::__SendClickActorPacket(CInstanceBase& rkInstVictim)
{
	CInstanceBase* pkInstMain = NEW_GetMainActorPtr();
	if (pkInstMain)
		if (pkInstMain->IsHoldingPickAxe())
			if (pkInstMain->IsMountingHorse())
				if (rkInstVictim.IsResource())
				{
					PyCallClassMemberFunc(m_ppyGameWindow, "OnCannotMining", Py_BuildValue("()"));
					return;
				}

	static DWORD s_dwNextTCPTime = 0;

	DWORD dwCurTime = ELTimer_GetMSec();

	if (dwCurTime >= s_dwNextTCPTime)
	{
		s_dwNextTCPTime = dwCurTime + 1000;

		CPythonNetworkStream& rkNetStream = CPythonNetworkStream::Instance();

		DWORD dwVictimVID = rkInstVictim.GetVirtualID();
		rkNetStream.SendOnClickPacket(dwVictimVID);
	}
}

void CPythonPlayer::ActEmotion(DWORD dwEmotionID)
{
	CInstanceBase* pkInstTarget = __GetAliveTargetInstancePtr();
	if (!pkInstTarget)
	{
		PyCallClassMemberFunc(m_ppyGameWindow, "OnCannotShotError", Py_BuildValue("(is)", GetMainCharacterIndex(), "NEED_TARGET"));
		return;
	}

	CPythonNetworkStream::Instance().SendChatPacket(_getf("/kiss %s", pkInstTarget->GetNameString()));
}

void CPythonPlayer::StartEmotionProcess()
{
	__ClearReservedAction();
	__ClearAutoAttackTargetActorID();

	m_bisProcessingEmotion = TRUE;
}

void CPythonPlayer::EndEmotionProcess()
{
	m_bisProcessingEmotion = FALSE;
}

BOOL CPythonPlayer::__IsProcessingEmotion()
{
	return m_bisProcessingEmotion;
}

// Dungeon
void CPythonPlayer::SetDungeonDestinationPosition(int ix, int iy)
{
	m_isDestPosition = TRUE;
	m_ixDestPos = ix;
	m_iyDestPos = iy;

	AlarmHaveToGo();
}

void CPythonPlayer::AlarmHaveToGo()
{
	m_iLastAlarmTime = CTimer::Instance().GetCurrentMillisecond();

	/////

	CInstanceBase* pInstance = NEW_GetMainActorPtr();
	if (!pInstance)
		return;

	TPixelPosition PixelPosition;
	pInstance->NEW_GetPixelPosition(&PixelPosition);

	float fAngle = GetDegreeFromPosition2(PixelPosition.x, PixelPosition.y, float(m_ixDestPos), float(m_iyDestPos));
	fAngle = fmod(540.0f - fAngle, 360.0f);
	D3DXVECTOR3 v3Rotation(0.0f, 0.0f, fAngle);

	PixelPosition.y *= -1.0f;

	CEffectManager::Instance().RegisterEffect("d:/ymir work/effect/etc/compass/appear_middle.mse");
	CEffectManager::Instance().CreateEffect("d:/ymir work/effect/etc/compass/appear_middle.mse", PixelPosition, v3Rotation);
}

// Party
void CPythonPlayer::ExitParty()
{
	m_PartyMemberMap.clear();

	CPythonCharacterManager::Instance().RefreshAllPCTextTail();
}

void CPythonPlayer::AppendPartyMember(DWORD dwPID, const char* c_szName)
{
	m_PartyMemberMap.insert(make_pair(dwPID, TPartyMemberInfo(dwPID, c_szName)));
}

void CPythonPlayer::LinkPartyMember(DWORD dwPID, DWORD dwVID)
{
	TPartyMemberInfo* pPartyMemberInfo;
	if (!GetPartyMemberPtr(dwPID, &pPartyMemberInfo))
	{
		TraceError(" CPythonPlayer::LinkPartyMember(dwPID=%d, dwVID=%d) - Failed to find party member", dwPID, dwVID);
		return;
	}

	pPartyMemberInfo->dwVID = dwVID;

	CInstanceBase* pInstance = NEW_FindActorPtr(dwVID);
	if (pInstance)
		pInstance->RefreshTextTail();
}

void CPythonPlayer::UnlinkPartyMember(DWORD dwPID)
{
	TPartyMemberInfo* pPartyMemberInfo;
	if (!GetPartyMemberPtr(dwPID, &pPartyMemberInfo))
	{
		TraceError(" CPythonPlayer::UnlinkPartyMember(dwPID=%d) - Failed to find party member", dwPID);
		return;
	}

	pPartyMemberInfo->dwVID = 0;
}

void CPythonPlayer::UpdatePartyMemberInfo(DWORD dwPID, BYTE byState, BYTE byHPPercentage)
{
	TPartyMemberInfo* pPartyMemberInfo;
	if (!GetPartyMemberPtr(dwPID, &pPartyMemberInfo))
	{
		TraceError(" CPythonPlayer::UpdatePartyMemberInfo(dwPID=%d, byState=%d, byHPPercentage=%d) - Failed to find character", dwPID, byState, byHPPercentage);
		return;
	}

	pPartyMemberInfo->byState = byState;
	pPartyMemberInfo->byHPPercentage = byHPPercentage;
}

void CPythonPlayer::UpdatePartyMemberAffect(DWORD dwPID, BYTE byAffectSlotIndex, short sAffectNumber)
{
	if (byAffectSlotIndex >= PARTY_AFFECT_SLOT_MAX_NUM)
	{
		TraceError(" CPythonPlayer::UpdatePartyMemberAffect(dwPID=%d, byAffectSlotIndex=%d, sAffectNumber=%d) - Strange affect slot index", dwPID, byAffectSlotIndex, sAffectNumber);
		return;
	}

	TPartyMemberInfo* pPartyMemberInfo;
	if (!GetPartyMemberPtr(dwPID, &pPartyMemberInfo))
	{
		TraceError(" CPythonPlayer::UpdatePartyMemberAffect(dwPID=%d, byAffectSlotIndex=%d, sAffectNumber=%d) - Failed to find character", dwPID, byAffectSlotIndex, sAffectNumber);
		return;
	}

	pPartyMemberInfo->sAffects[byAffectSlotIndex] = sAffectNumber;
}

void CPythonPlayer::RemovePartyMember(DWORD dwPID)
{
	DWORD dwVID = 0;
	TPartyMemberInfo* pPartyMemberInfo;
	if (GetPartyMemberPtr(dwPID, &pPartyMemberInfo))
	{
		dwVID = pPartyMemberInfo->dwVID;
	}

	m_PartyMemberMap.erase(dwPID);

	if (dwVID > 0)
	{
		CInstanceBase* pInstance = NEW_FindActorPtr(dwVID);
		if (pInstance)
			pInstance->RefreshTextTail();
	}
}

bool CPythonPlayer::IsPartyMemberByVID(DWORD dwVID)
{
	std::map<DWORD, TPartyMemberInfo>::iterator itor = m_PartyMemberMap.begin();
	for (; itor != m_PartyMemberMap.end(); ++itor)
	{
		TPartyMemberInfo& rPartyMemberInfo = itor->second;
		if (dwVID == rPartyMemberInfo.dwVID)
			return true;
	}

	return false;
}

bool CPythonPlayer::IsPartyMemberByName(const char* c_szName)
{
	std::map<DWORD, TPartyMemberInfo>::iterator itor = m_PartyMemberMap.begin();
	for (; itor != m_PartyMemberMap.end(); ++itor)
	{
		TPartyMemberInfo& rPartyMemberInfo = itor->second;
		if (0 == rPartyMemberInfo.strName.compare(c_szName))
			return true;
	}

	return false;
}

bool CPythonPlayer::GetPartyMemberPtr(DWORD dwPID, TPartyMemberInfo** ppPartyMemberInfo)
{
	std::map<DWORD, TPartyMemberInfo>::iterator itor = m_PartyMemberMap.find(dwPID);

	if (m_PartyMemberMap.end() == itor)
		return false;

	*ppPartyMemberInfo = &(itor->second);

	return true;
}

bool CPythonPlayer::PartyMemberPIDToVID(DWORD dwPID, DWORD* pdwVID)
{
	std::map<DWORD, TPartyMemberInfo>::iterator itor = m_PartyMemberMap.find(dwPID);

	if (m_PartyMemberMap.end() == itor)
		return false;

	const TPartyMemberInfo& c_rPartyMemberInfo = itor->second;
	*pdwVID = c_rPartyMemberInfo.dwVID;

	return true;
}

bool CPythonPlayer::PartyMemberVIDToPID(DWORD dwVID, DWORD* pdwPID)
{
	std::map<DWORD, TPartyMemberInfo>::iterator itor = m_PartyMemberMap.begin();
	for (; itor != m_PartyMemberMap.end(); ++itor)
	{
		TPartyMemberInfo& rPartyMemberInfo = itor->second;
		if (dwVID == rPartyMemberInfo.dwVID)
		{
			*pdwPID = rPartyMemberInfo.dwPID;
			return true;
		}
	}

	return false;
}

bool CPythonPlayer::IsSamePartyMember(DWORD dwVID1, DWORD dwVID2)
{
	return (IsPartyMemberByVID(dwVID1) && IsPartyMemberByVID(dwVID2));
}

// PVP
void CPythonPlayer::RememberChallengeInstance(DWORD dwVID)
{
	m_RevengeInstanceSet.erase(dwVID);
	m_ChallengeInstanceSet.insert(dwVID);
}
void CPythonPlayer::RememberRevengeInstance(DWORD dwVID)
{
	m_ChallengeInstanceSet.erase(dwVID);
	m_RevengeInstanceSet.insert(dwVID);
}
void CPythonPlayer::RememberCantFightInstance(DWORD dwVID)
{
	m_CantFightInstanceSet.insert(dwVID);
}
void CPythonPlayer::ForgetInstance(DWORD dwVID)
{
	m_ChallengeInstanceSet.erase(dwVID);
	m_RevengeInstanceSet.erase(dwVID);
	m_CantFightInstanceSet.erase(dwVID);
}

bool CPythonPlayer::IsChallengeInstance(DWORD dwVID)
{
	return m_ChallengeInstanceSet.end() != m_ChallengeInstanceSet.find(dwVID);
}
bool CPythonPlayer::IsRevengeInstance(DWORD dwVID)
{
	return m_RevengeInstanceSet.end() != m_RevengeInstanceSet.find(dwVID);
}
bool CPythonPlayer::IsCantFightInstance(DWORD dwVID)
{
	return m_CantFightInstanceSet.end() != m_CantFightInstanceSet.find(dwVID);
}

void CPythonPlayer::OpenPrivateShop()
{
	m_isOpenPrivateShop = TRUE;
}
void CPythonPlayer::ClosePrivateShop()
{
	m_isOpenPrivateShop = FALSE;
}

bool CPythonPlayer::IsOpenPrivateShop()
{
	return m_isOpenPrivateShop;
}

void CPythonPlayer::SetObserverMode(bool isEnable)
{
	m_isObserverMode = isEnable;
}

bool CPythonPlayer::IsObserverMode()
{
	return m_isObserverMode;
}

void CPythonPlayer::SetFreeCameraMode(bool isEnable)
{
	m_isFreeCameraMode = isEnable;
}

bool CPythonPlayer::IsFreeCameraMode()
{
	return m_isFreeCameraMode;
}

BOOL CPythonPlayer::__ToggleCoolTime()
{
	m_sysIsCoolTime = 1 - m_sysIsCoolTime;
	return m_sysIsCoolTime;
}

BOOL CPythonPlayer::__ToggleLevelLimit()
{
	m_sysIsLevelLimit = 1 - m_sysIsLevelLimit;
	return m_sysIsLevelLimit;
}

void CPythonPlayer::StartStaminaConsume(DWORD dwConsumePerSec, DWORD dwCurrentStamina)
{
	m_isConsumingStamina = TRUE;
	m_fConsumeStaminaPerSec = float(dwConsumePerSec);
	m_fCurrentStamina = float(dwCurrentStamina);

	SetStatus(POINT_STAMINA, dwCurrentStamina);
}

void CPythonPlayer::StopStaminaConsume(DWORD dwCurrentStamina)
{
	m_isConsumingStamina = FALSE;
	m_fConsumeStaminaPerSec = 0.0f;
	m_fCurrentStamina = float(dwCurrentStamina);

	SetStatus(POINT_STAMINA, dwCurrentStamina);
}

DWORD CPythonPlayer::GetPKMode()
{
	CInstanceBase* pInstance = NEW_GetMainActorPtr();
	if (!pInstance)
		return 0;

	return pInstance->GetPKMode();
}

void CPythonPlayer::SetGameWindow(PyObject* ppyObject)
{
	m_ppyGameWindow = ppyObject;
}

void CPythonPlayer::NEW_ClearSkillData(bool bAll)
{
	std::map<DWORD, DWORD>::iterator it;

	for (it = m_skillSlotDict.begin(); it != m_skillSlotDict.end();)
	{
		if (bAll || __GetSkillType(it->first) == CPythonSkill::SKILL_TYPE_ACTIVE)
			it = m_skillSlotDict.erase(it);
		else
			++it;
	}

	for (int i = 0; i < SKILL_MAX_NUM; ++i)
	{
		ZeroMemory(&m_playerStatus.aSkill[i], sizeof(TSkillInstance));
	}

	for (int j = 0; j < SKILL_MAX_NUM; ++j)
	{
		m_playerStatus.aSkill[j].iGrade = 0;
		m_playerStatus.aSkill[j].fcurEfficientPercentage = 0.0f;
		m_playerStatus.aSkill[j].fnextEfficientPercentage = 0.05f;
	}

	if (m_ppyGameWindow)
		PyCallClassMemberFunc(m_ppyGameWindow, "BINARY_CheckGameButton", Py_BuildNone());
}

#ifdef ENABLE_INGAME_MALL_SYSTEM
DWORD CPythonPlayer::GetDragonCoin()
{
	return m_dwDragonCoin;
}

DWORD CPythonPlayer::GetDragonMark()
{
	return m_dwDragonMark;
}

void CPythonPlayer::SetDragonCoin(DWORD amount)
{
	m_dwDragonCoin = amount;
}

void CPythonPlayer::SetDragonMark(DWORD amount)
{
	m_dwDragonMark = amount;
}
#endif

#ifdef ENABLE_PB2_PREMIUM_SYSTEM
void CPythonPlayer::SetPB2GlobalChat(bool f)
{
	m_bPB2GlobalChat = f;
}
#endif // ENABLE_PB2_PREMIUM_SYSTEM

void CPythonPlayer::ClearSkillDict()
{
	// ClearSkillDict
	m_skillSlotDict.clear();

	// Game End - Player Data Reset
	m_isOpenPrivateShop = false;
	m_isObserverMode = false;
	m_isFreeCameraMode = false;
	m_isConsumingStamina = FALSE;
	m_fConsumeStaminaPerSec = 0.0f;
	m_fCurrentStamina = 0.0f;

#ifdef ENABLE_AUTO_HUNT_SYSTEM
	memset(&m_playerStatus, 0, sizeof(m_playerStatus));
	otoAvDurum = false;
	otoAvSaldir = false;
	otoAvOdak = false;
	sonHedefBulMs = 0;

	otoAvBaslangicKonumu = TPixelPosition(0, 0, 0);
#endif

	__ClearAutoAttackTargetActorID();
}

void CPythonPlayer::Clear()
{
	memset(&m_playerStatus, 0, sizeof(m_playerStatus));
	NEW_ClearSkillData(true);

	m_bisProcessingEmotion = FALSE;

	m_dwSendingTargetVID = 0;
	m_fTargetUpdateTime = 0.0f;

	// Test Code for Status Interface
	m_stName = "";
	m_dwPlayerID = 0;
	m_dwMainCharacterIndex = 0;
	m_dwRace = 0;
	m_dwWeaponMinPower = 0;
	m_dwWeaponMaxPower = 0;
	m_dwWeaponMinMagicPower = 0;
	m_dwWeaponMaxMagicPower = 0;
	m_dwWeaponAddPower = 0;

	/////
	m_MovingCursorPosition = TPixelPosition(0, 0, 0);
	m_fMovingCursorSettingTime = 0.0f;
	m_eReservedMode = MODE_NONE;
	m_fReservedDelayTime = 0.0f;
	m_kPPosReserved = TPixelPosition(0, 0, 0);
	m_dwVIDReserved = 0;
	m_dwIIDReserved = 0;
	m_dwSkillSlotIndexReserved = 0;
	m_dwSkillRangeReserved = 0;

	m_isUp = false;
	m_isDown = false;
	m_isLeft = false;
	m_isRight = false;
	m_isSmtMov = false;
	m_isDirMov = false;
	m_isDirKey = false;
	m_isAtkKey = false;

	m_isCmrRot = true;
	m_fCmrRotSpd = 20.0f;

	m_iComboOld = 0;

	m_dwVIDPicked = 0;
	m_dwIIDPicked = 0;

	m_dwcurSkillSlotIndex = DWORD(-1);

	m_dwTargetVID = 0;
	m_dwTargetEndTime = 0;

	m_PartyMemberMap.clear();

	m_ChallengeInstanceSet.clear();
	m_RevengeInstanceSet.clear();

	m_isOpenPrivateShop = false;
	m_isObserverMode = false;
	m_isFreeCameraMode = false;
	m_isConsumingStamina = FALSE;
	m_fConsumeStaminaPerSec = 0.0f;
	m_fCurrentStamina = 0.0f;

#ifdef ENABLE_INGAME_MALL_SYSTEM
	m_dwDragonCoin = 0;
	m_dwDragonMark = 0;
#endif
#ifdef ENABLE_PB2_PREMIUM_SYSTEM
	m_bPB2GlobalChat = false;
#endif // ENABLE_PB2_PREMIUM_SYSTEM
	m_dwNextTCPAutoPickTime = 0;
	__ClearAutoAttackTargetActorID();
}

CPythonPlayer::CPythonPlayer(void)
{
	SetMovableGroundDistance(40.0f);

	// AffectIndex To SkillIndex
	m_kMap_dwAffectIndexToSkillIndex.insert(make_pair(int(CInstanceBase::AFFECT_JEONGWI), 3));
	m_kMap_dwAffectIndexToSkillIndex.insert(make_pair(int(CInstanceBase::AFFECT_GEOMGYEONG), 4));
	m_kMap_dwAffectIndexToSkillIndex.insert(make_pair(int(CInstanceBase::AFFECT_CHEONGEUN), 19));
	m_kMap_dwAffectIndexToSkillIndex.insert(make_pair(int(CInstanceBase::AFFECT_GYEONGGONG), 49));
	m_kMap_dwAffectIndexToSkillIndex.insert(make_pair(int(CInstanceBase::AFFECT_EUNHYEONG), 34));
	m_kMap_dwAffectIndexToSkillIndex.insert(make_pair(int(CInstanceBase::AFFECT_GONGPO), 64));
	m_kMap_dwAffectIndexToSkillIndex.insert(make_pair(int(CInstanceBase::AFFECT_JUMAGAP), 65));
	m_kMap_dwAffectIndexToSkillIndex.insert(make_pair(int(CInstanceBase::AFFECT_HOSIN), 94));
	m_kMap_dwAffectIndexToSkillIndex.insert(make_pair(int(CInstanceBase::AFFECT_BOHO), 95));
	m_kMap_dwAffectIndexToSkillIndex.insert(make_pair(int(CInstanceBase::AFFECT_KWAESOK), 110));
	m_kMap_dwAffectIndexToSkillIndex.insert(make_pair(int(CInstanceBase::AFFECT_GICHEON), 96));
	m_kMap_dwAffectIndexToSkillIndex.insert(make_pair(int(CInstanceBase::AFFECT_JEUNGRYEOK), 111));
	m_kMap_dwAffectIndexToSkillIndex.insert(make_pair(int(CInstanceBase::AFFECT_PABEOP), 66));
	m_kMap_dwAffectIndexToSkillIndex.insert(make_pair(int(CInstanceBase::AFFECT_FALLEN_CHEONGEUN), 19));
	/////
	m_kMap_dwAffectIndexToSkillIndex.insert(make_pair(int(CInstanceBase::AFFECT_GWIGEOM), 63));
	m_kMap_dwAffectIndexToSkillIndex.insert(make_pair(int(CInstanceBase::AFFECT_MUYEONG), 78));
	m_kMap_dwAffectIndexToSkillIndex.insert(make_pair(int(CInstanceBase::AFFECT_HEUKSIN), 79));

#ifdef ENABLE_WOLFMAN_CHARACTER
	m_kMap_dwAffectIndexToSkillIndex.insert(make_pair(int(CInstanceBase::AFFECT_RED_POSSESSION), 174));
	m_kMap_dwAffectIndexToSkillIndex.insert(make_pair(int(CInstanceBase::AFFECT_BLUE_POSSESSION), 175));
#endif

	m_ppyGameWindow = NULL;

	m_sysIsCoolTime = TRUE;
	m_sysIsLevelLimit = TRUE;
	m_dwPlayTime = 0;

	m_aeMBFButton[MBT_LEFT] = CPythonPlayer::MBF_SMART;
	m_aeMBFButton[MBT_RIGHT] = CPythonPlayer::MBF_CAMERA;
	m_aeMBFButton[MBT_MIDDLE] = CPythonPlayer::MBF_CAMERA;

	memset(m_adwEffect, 0, sizeof(m_adwEffect));

	m_isDestPosition = FALSE;
	m_ixDestPos = 0;
	m_iyDestPos = 0;
	m_iLastAlarmTime = 0;

	Clear();
}

CPythonPlayer::~CPythonPlayer(void)
{
}
