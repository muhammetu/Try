#pragma once

#include "AbstractSingleton.h"

class CInstanceBase;

class IAbstractPlayer : public TAbstractSingleton<IAbstractPlayer>
{
public:
	IAbstractPlayer() {}
	virtual ~IAbstractPlayer() {}

	virtual DWORD	GetMainCharacterIndex() = 0;
	virtual void	SetMainCharacterIndex(int iIndex) = 0;
	virtual bool	IsMainCharacterIndex(DWORD dwIndex) = 0;

#ifdef ENABLE_GOLD_LIMIT_REWORK
	virtual long long	GetStatus(DWORD dwType) = 0;
#else
	virtual int		GetStatus(DWORD dwType) = 0;
#endif

	virtual const char* GetName() = 0;

	virtual void	SetRace(DWORD dwRace) = 0;

	virtual void	StartStaminaConsume(DWORD dwConsumePerSec, DWORD dwCurrentStamina) = 0;
	virtual void	StopStaminaConsume(DWORD dwCurrentStamina) = 0;

	virtual bool	IsPartyMemberByVID(DWORD dwVID) = 0;
	virtual bool	PartyMemberVIDToPID(DWORD dwVID, DWORD* pdwPID) = 0;
	virtual bool	IsSamePartyMember(DWORD dwVID1, DWORD dwVID2) = 0;

	virtual void	SetItemData(TItemPos itemPos, const TItemData& c_rkItemInst) = 0;
#ifdef ENABLE_ITEM_COUNT_LIMIT
	virtual void	SetItemCount(TItemPos itemPos, DWORD byCount) = 0;
#else
	virtual void	SetItemCount(TItemPos itemPos, BYTE byCount) = 0;
#endif
	virtual void	SetItemMetinSocket(TItemPos itemPos, DWORD dwMetinSocketIndex, DWORD dwMetinNumber) = 0;
	virtual void	SetItemAttribute(TItemPos itemPos, DWORD dwAttrIndex, BYTE byType, short sValue) = 0;

	virtual DWORD	GetItemIndex(TItemPos itemPos) = 0;
	virtual DWORD	GetItemFlags(TItemPos itemPos) = 0;
	virtual DWORD	GetItemCount(TItemPos itemPos) = 0;
	virtual DWORD	GetItemMetinSocket(TItemPos Cell, DWORD dwMetinSocketIndex) = 0;
	virtual BYTE	GetItemAttributeType(TItemPos Cell, DWORD dwAttrSlotIndex) = 0;
	virtual short	GetItemAttributeValue(TItemPos Cell, DWORD dwAttrSlotIndex) = 0;

	virtual bool	IsEquipItemInSlot(TItemPos itemPos) = 0;

	virtual void	AddQuickSlot(int QuickslotIndex, char IconType, char IconPosition) = 0;
	virtual void	DeleteQuickSlot(int QuickslotIndex) = 0;
	virtual void	MoveQuickSlot(int Source, int Target) = 0;

	virtual void	SetWeaponPower(DWORD dwMinPower, DWORD dwMaxPower, DWORD dwMinMagicPower, DWORD dwMaxMagicPower, DWORD dwAddPower) = 0;

	virtual void	SetTarget(DWORD dwVID, BOOL bForceChange = TRUE) = 0;
	virtual void	NotifyCharacterUpdate(DWORD dwVID) = 0;
	virtual void	NotifyCharacterDead(DWORD dwVID) = 0;
	virtual void	NotifyDeletingCharacterInstance(DWORD dwVID) = 0;
	virtual void	NotifyChangePKMode() = 0;

	virtual void	SetObserverMode(bool isEnable) = 0;
	virtual void	SetComboSkillFlag(BOOL bFlag) = 0;

	virtual void	StartEmotionProcess() = 0;
	virtual void	EndEmotionProcess() = 0;

	virtual CInstanceBase* NEW_GetMainActorPtr() = 0;
#ifdef ENABLE_BEGINNER_ITEM_SYSTEM
	virtual void	SetBasicItem(TItemPos Cell, bool is_basic) = 0;
	virtual bool	IsBasicItem(TItemPos Cell) = 0;
#endif
#ifdef ENABLE_ITEM_EVOLUTION_SYSTEM
	virtual void	SetItemEvolution(TItemPos itemPos, DWORD evolution) = 0;
	virtual	DWORD	GetItemEvolution(TItemPos itemPos) = 0;
#endif
#ifdef ENABLE_INGAME_MALL_SYSTEM
	virtual void	SetDragonCoin(DWORD amount) = 0;
	virtual	void	SetDragonMark(DWORD amount) = 0;
#endif
#ifdef ENABLE_CHANGELOOK_SYSTEM
	virtual void	SetItemTransmutation(TItemPos itemPos, DWORD dwVnum) = 0;
	virtual DWORD	GetItemTransmutation(TItemPos itemPos) = 0;
#endif
#ifdef ENABLE_AUTO_HUNT_SYSTEM
	virtual void	OtoAvDurumAta(bool durum) = 0;
	virtual void	OtoAvSaldiriAta(bool durum) = 0;
#endif
};
