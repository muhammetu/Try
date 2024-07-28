#pragma once
#include "entity.h"

class CItem : public CEntity
{
protected:
	// override methods from ENTITY class
	virtual void	EncodeInsertPacket(LPENTITY entity);
	virtual void	EncodeRemovePacket(LPENTITY entity);

public:
	CItem(DWORD dwVnum);
	virtual ~CItem();

	int			GetLevelLimit();

	bool		CheckItemUseLevel(int nLevel);

	long		FindApplyValue(BYTE bApplyType);

	void		Initialize();
	void		Destroy();

	void		Save();

	void		SetWindow(BYTE b) { m_bWindow = b; }
	BYTE		GetWindow() { return m_bWindow; }

	void		SetID(DWORD id) { m_dwID = id; }
	DWORD		GetID() { return m_dwID; }

	void			SetProto(const TItemTable* table);
	TItemTable const* GetProto() { return m_pProto; }

#ifdef __GOLD_LIMIT_REWORK__
	long long		GetGold();
#else
	int		GetGold();
#endif
#ifdef __GOLD_LIMIT_REWORK__
	long long	GetShopBuyPrice();
#else
	int		GetShopBuyPrice();
#endif
#ifdef __ITEM_ENTITY_UTILITY__
	const char* GetName();
	std::string GetNameString();
#else
	const char* GetName() { return m_pProto ? m_pProto->szLocaleName : NULL; }
#endif
	const char* GetBaseName() { return m_pProto ? m_pProto->szName : NULL; }
	BYTE		GetSize() { return m_pProto ? m_pProto->bSize : 0; }

	void		SetFlag(long flag) { m_lFlag = flag; }
	long		GetFlag() { return m_lFlag; }

	void		AddFlag(long bit);
	void		RemoveFlag(long bit);

	DWORD		GetWearFlag() { return m_pProto ? m_pProto->dwWearFlags : 0; }
	DWORD		GetAntiFlag() { return m_pProto ? m_pProto->dwAntiFlags : 0; }
	DWORD		GetImmuneFlag() { return m_pProto ? m_pProto->dwImmuneFlag : 0; }

	void		SetVID(DWORD vid) { m_dwVID = vid; }
	DWORD		GetVID() { return m_dwVID; }

	bool		SetCount(DWORD count);
	DWORD		GetCount();

	//
	DWORD		GetVnum() const { return m_dwMaskVnum ? m_dwMaskVnum : m_dwVnum; }
	DWORD		GetOriginalVnum() const { return m_dwVnum; }
	BYTE		GetType() const { return m_pProto ? m_pProto->bType : 0; }
	BYTE		GetSubType() const { return m_pProto ? m_pProto->bSubType : 0; }
	BYTE		GetLimitType(DWORD idx) const { return m_pProto ? m_pProto->aLimits[idx].bType : 0; }
	long		GetLimitValue(DWORD idx) const { return m_pProto ? m_pProto->aLimits[idx].lValue : 0; }

	long		GetValue(DWORD idx);

	void		SetCell(LPCHARACTER ch, WORD pos) { m_pOwner = ch, m_wCell = pos; }
	WORD		GetCell() { return m_wCell; }

	LPITEM		RemoveFromCharacter();
	bool		AddToCharacter(LPCHARACTER ch, TItemPos Cell, const char* file, int line);
	#define __ADD_TO_CHARACTER(ch, Cell) AddToCharacter(ch, Cell, __FILE__, __LINE__)
	LPCHARACTER	GetOwner() { return m_pOwner; }

	LPITEM		RemoveFromGround();
	bool		AddToGround(long lMapIndex, const PIXEL_POSITION& pos, bool skipOwnerCheck = false);

	int			FindEquipCell(LPCHARACTER ch, int bCandidateCell = -1);
	bool		IsEquipped() const { return m_bEquipped; }
	bool		EquipTo(LPCHARACTER ch, BYTE bWearCell);
	bool		IsEquipable() const;

	bool		CanUsedBy(LPCHARACTER ch);

	bool		DistanceValid(LPCHARACTER ch);

	void		UpdatePacket();
	void		UsePacketEncode(LPCHARACTER ch, LPCHARACTER victim, struct packet_item_use* packet);

	void		SetExchanging(bool isOn = true);
	bool		IsExchanging() { return m_bExchanging; }

	void		ModifyPoints(bool bAdd);

#ifdef __GOLD_LIMIT_REWORK__
	bool		CreateSocket(BYTE bSlot, long long llGold);
#else
	bool		CreateSocket(BYTE bSlot, BYTE bGold);
#endif
	const long* GetSockets() { return &m_alSockets[0]; }
	long		GetSocket(int i) { return m_alSockets[i]; }

	void		SetSockets(const long* al);
	void		SetSocket(int i, long v, bool bLog = true);

	int		GetSocketCount();
	bool		AddSocket();

	const TPlayerItemAttribute* GetAttributes() { return m_aAttr; }
	const TPlayerItemAttribute& GetAttribute(int i) { return m_aAttr[i]; }

	BYTE		GetAttributeType(int i) { return m_aAttr[i].bType; }
	short		GetAttributeValue(int i) { return m_aAttr[i].sValue; }

	void		SetAttributes(const TPlayerItemAttribute* c_pAttribute);

	int		FindAttribute(BYTE bType);
	bool		RemoveAttributeAt(int index);
	bool		RemoveAttributeType(BYTE bType);

#ifdef __MULTIPLE_MONSTER_ATTR__
	bool		HasAttrEx(BYTE alreadyType, BYTE addedType);
#endif
	bool		HasAttr(BYTE bApply);
	bool		HasRareAttr(BYTE bApply);

	void		SetDestroyEvent(LPEVENT pkEvent);
	void		StartDestroyEvent(int iSec = 180);

	DWORD		GetRefinedVnum() { return m_pProto ? m_pProto->dwRefinedVnum : 0; }
	DWORD		GetRefineFromVnum();
	int		GetRefineLevel();

	void		SetSkipSave(bool b) { m_bSkipSave = b; }
	bool		GetSkipSave() { return m_bSkipSave; }

	bool		IsOwnership(LPCHARACTER ch);
	void		SetOwnership(LPCHARACTER ch, int iSec = 10);
	void		SetOwnershipEvent(LPEVENT pkEvent);

	DWORD		GetLastOwnerPID() { return m_dwLastOwnerPID; }
#ifdef __HIGHLIGHT_ITEM__
	void		SetLastOwnerPID(DWORD pid) { m_dwLastOwnerPID = pid; }
#endif
#ifdef __BATTLE_PASS_SYSTEM__
	bool HaveOwnership() { return (m_pkOwnershipEvent ? true : false); }
#endif
	int		GetAttributeSetIndex();
	void		AlterToMagicItem();
	void		AlterToSocketItem(int iSocketCount);

	DWORD		GetRefineSet() { return m_pProto ? m_pProto->dwRefineSet : 0; }
	void		StartUniqueExpireEvent();
	void		SetUniqueExpireEvent(LPEVENT pkEvent);

	void		StartTimerBasedOnWearExpireEvent();
	void		SetTimerBasedOnWearExpireEvent(LPEVENT pkEvent);
	void		StartRealTimeExpireEvent();
#ifdef __BEGINNER_ITEM__
	bool		IsRealTimeItem();
#endif

	void		StopUniqueExpireEvent();
	void		StopTimerBasedOnWearExpireEvent();
	void		StopAccessorySocketExpireEvent();

	int			GetDuration();

	int			GetAttributeCount();
#ifdef __ANCIENT_ATTR_ITEM__
	void		ClearAttribute(bool clearMode = true);
#else
	void		ClearAttribute();
#endif
	void		ChangeAttribute(const int* aiChangeProb = NULL);
	void		AddAttribute();
#ifdef __ATTR_ADDON_ITEMS__
	void		AddAttributeEx();
#endif
	void		AddAttribute(BYTE bType, short sValue);

	void		ApplyAddon(int iAddonType);

	int		GetSpecialGroup() const;
	bool	IsSameSpecialGroup(const LPITEM item) const;

	// ACCESSORY_REFINE

	bool		IsAccessoryForSocket();
	bool		IsDeattachAccessoryForSocket();

	int		GetAccessorySocketGrade();
	int		GetAccessorySocketMaxGrade();
	int		GetAccessorySocketDownGradeTime();

	void		SetAccessorySocketGrade(int iGrade);
	void		SetAccessorySocketMaxGrade(int iMaxGrade);
	void		SetAccessorySocketDownGradeTime(DWORD time);

	void		AccessorySocketDegrade();

	void		StartAccessorySocketExpireEvent();
	void		SetAccessorySocketExpireEvent(LPEVENT pkEvent);

	bool		CanPutInto(LPITEM item);
#ifdef __PERMA_ACCESSORY__
	bool		CanPutIntoPerma(LPITEM item);
#endif
	DWORD		GetPutItemVnum();
	DWORD		GetPutItemVnumPerma();
	// END_OF_ACCESSORY_REFINE

	void		CopyAttributeTo(LPITEM pItem);
	void		CopySocketTo(LPITEM pItem);

	int			GetRareAttrCount();
	bool		AddRareAttribute();
	bool		ChangeRareAttribute();

	void		Lock(bool f) { m_isLocked = f; }
	bool		isLocked() const { return m_isLocked; }

private:
	void		SetAttribute(int i, BYTE bType, short sValue);
public:
	void		SetForceAttribute(int i, BYTE bType, short sValue);

protected:
	bool		Unequip();

	void		AddAttr(BYTE bApply, BYTE bLevel);
	void		PutAttribute(const int* aiAttrPercentTable);
	void		PutAttributeWithLevel(BYTE bLevel);

public:
	void		AddRareAttribute2(const int* aiAttrPercentTable = NULL);
protected:
	void		AddRareAttr(BYTE bApply, BYTE bLevel);
	void		PutRareAttribute(const int* aiAttrPercentTable);
	void		PutRareAttributeWithLevel(BYTE bLevel);

public:
	bool		OnAfterCreatedItem();

public:
	bool		IsRideItem();
	bool		IsRamadanRing();
#ifdef __PET_SYSTEM_PROTO__
	bool		IsPetItem();
#endif
	void		ClearMountAttributeAndAffect();
	bool		IsNewMountItem();
#ifdef __MOUNT_COSTUME_SYSTEM__
	bool		IsMountItem();
#endif
#ifdef __GROWTH_PET_SYSTEM__
	bool		IsNewPetItem() { return GetVnum() >= 55701 && GetVnum() <= 55710; }
#endif
#ifdef __GROWTH_MOUNT_SYSTEM__
	bool		IsNewMountExItem() { return GetVnum() >= 55770 && GetVnum() <= 55780; } // TODO:
#endif
	void		SetMaskVnum(DWORD vnum) { m_dwMaskVnum = vnum; }
	DWORD		GetMaskVnum() { return m_dwMaskVnum; }
	bool		IsMaskedItem() { return m_dwMaskVnum != 0; }

	bool		IsDragonSoul();
	int			GiveMoreTime_Per(float fPercent);
	int			GiveMoreTime_Fix(DWORD dwTime);
#ifdef __DSS_RECHARGE_ITEM__
	int		GiveMoreTime_Extend(DWORD dwTime);
#endif
#ifdef __ADDITIONAL_INVENTORY__
	bool		IsUpgradeItem();
	bool		IsBook();
	bool		IsStone();
	bool		IsFlower();
	bool		IsAttrItem();
	bool		IsChest();
#endif
#ifdef __BOSS_SCROLL__
	bool		IsBossScroll();
#endif
	bool		IsTilsimable();
private:
	TItemTable const* m_pProto;

	DWORD		m_dwVnum;
	LPCHARACTER	m_pOwner;

	BYTE		m_bWindow;
	DWORD		m_dwID;
	bool		m_bEquipped;
	DWORD		m_dwVID;		// VID
	WORD		m_wCell;
	DWORD		m_dwCount;

	long		m_lFlag;
	DWORD		m_dwLastOwnerPID;

	bool		m_bExchanging;

	long		m_alSockets[ITEM_SOCKET_MAX_NUM];
	TPlayerItemAttribute	m_aAttr[ITEM_ATTRIBUTE_MAX_NUM];

	LPEVENT		m_pkDestroyEvent;
	LPEVENT		m_pkExpireEvent;
	LPEVENT		m_pkUniqueExpireEvent;
	LPEVENT		m_pkTimerBasedOnWearExpireEvent;
	LPEVENT		m_pkRealTimeExpireEvent;
	LPEVENT		m_pkAccessorySocketExpireEvent;
	LPEVENT		m_pkOwnershipEvent;

	DWORD		m_dwOwnershipPID;

	bool		m_bSkipSave;

	bool		m_isLocked;

	DWORD		m_dwMaskVnum;
	DWORD		m_dwSIGVnum;
#ifdef __BEGINNER_ITEM__
protected:
	bool		is_basic;
public:
	bool		IsBasicItem() const { return is_basic; }
	void		SetBasic(bool b);
#endif
#ifdef __ITEM_EVOLUTION__
public:
	void		SetEvolution(DWORD evolution);
	DWORD		GetEvolution();
	DWORD		GetRealEvolution();
	BYTE		GetEvolutionType();
	DWORD		GetEvolutionRealAtk();
#ifdef __ARMOR_EVOLUTION__
	DWORD		GetEvolutionArmorElementAtk();
	DWORD		GetEvolutionRealArmorAtk();
	DWORD		GetEvolutionRealArmorAtkAll();
#endif
	DWORD		GetEvolutionElementAtk();
	DWORD		GetEvolutionElementAtkAll();
private:
	DWORD		m_dwEvolution;
#endif
#ifdef __ANCIENT_ATTR_ITEM__
public:
	int AddNewStyleAttribute(BYTE* bValues);
	void ChangeAttribute5TH();
#endif
#ifdef __ITEM_CHANGELOOK__
public:
	DWORD		GetTransmutation() const;
	void		SetTransmutation(DWORD dwVnum, bool bLog = false);
private:
	DWORD		m_dwTransmutation;
#endif
public:
	void SetSIGVnum(DWORD dwSIG)
	{
		m_dwSIGVnum = dwSIG;
	}
	DWORD	GetSIGVnum() const
	{
		return m_dwSIGVnum;
	}
	bool		IsStackable() { return ((GetFlag() & ITEM_FLAG_STACKABLE) && (!is_basic)) ? true : false; }
#ifdef __PENDANT_SYSTEM__
public:
	DWORD GetElement();
#endif
};

EVENTINFO(item_event_info)
{
	LPITEM item;
	char szOwnerName[CHARACTER_NAME_MAX_LEN];

	item_event_info()
		: item(0)
	{
		::memset(szOwnerName, 0, CHARACTER_NAME_MAX_LEN);
	}
};

EVENTINFO(item_vid_event_info)
{
	DWORD item_vid;

	item_vid_event_info()
		: item_vid(0)
	{
	}
};