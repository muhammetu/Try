#include "stdafx.h"
#include "char.h"
#include "item.h"
#include "desc.h"
#include "DragonSoul.h"
#include "log.h"
#include "utils.h"

void CHARACTER::DragonSoul_Initialize()
{
	for (int i = INVENTORY_MAX_NUM + WEAR_MAX_NUM; i < DRAGON_SOUL_EQUIP_SLOT_END; i++)
	{
		LPITEM pItem = GetItem(TItemPos(INVENTORY, i));
		if (NULL != pItem)
			pItem->SetSocket(ITEM_SOCKET_DRAGON_SOUL_ACTIVE_IDX, 0);
	}

	if (FindAffect(AFFECT_DRAGON_SOUL_DECK_0))
	{
		DragonSoul_ActivateDeck(DRAGON_SOUL_DECK_0);
	}
	else if (FindAffect(AFFECT_DRAGON_SOUL_DECK_1))
	{
		DragonSoul_ActivateDeck(DRAGON_SOUL_DECK_1);
	}
}

int CHARACTER::DragonSoul_GetActiveDeck() const
{
	return m_pointsInstant.iDragonSoulActiveDeck;
}

bool CHARACTER::DragonSoul_IsDeckActivated() const
{
	return m_pointsInstant.iDragonSoulActiveDeck >= 0;
}

bool CHARACTER::DragonSoul_IsQualified() const
{
	if (g_bNonCheckDSQualified)
		return true;

	return FindAffect(AFFECT_DRAGON_SOUL_QUALIFIED) != nullptr;
}

void CHARACTER::DragonSoul_GiveQualification()
{
	AddAffect(AFFECT_DRAGON_SOUL_QUALIFIED, APPLY_NONE, 0, AFF_NONE, INFINITE_AFFECT_DURATION, 0, false, false);
	//SetQuestFlag("dragon_soul.is_qualified", 1);
	//PointChange(POINT_DRAGON_SOUL_IS_QUALIFIED, 1 - GetPoint(POINT_DRAGON_SOUL_IS_QUALIFIED));
}

bool CHARACTER::DragonSoul_ActivateDeck(int deck_idx)
{
#ifdef __DSS_ACTIVE_EFFECT__
	if ((GetDragonSoulActivateTime() + 5) > get_global_time())
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("DRAGON_SOUL_ACTIVATE_EFFECT_TIME"));
		return false;
	}
#endif

	if (deck_idx < DRAGON_SOUL_DECK_0 || deck_idx >= DRAGON_SOUL_DECK_MAX_NUM)
	{
		return false;
	}

	if (DragonSoul_GetActiveDeck() == deck_idx)
		return true;

	DragonSoul_DeactivateAll();

	if (!DragonSoul_IsQualified())
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("용혼석 상자가 활성화되지 않았습니다."));
		return false;
	}

#ifdef __DSS_ACTIVE_EFFECT__
	AddAffect(AFFECT_DRAGON_SOUL_DECK_0 + deck_idx, APPLY_NONE, 0, AFF_DRAGON_SOUL, INFINITE_AFFECT_DURATION, 0, false);
#else
	AddAffect(AFFECT_DRAGON_SOUL_DECK_0 + deck_idx, APPLY_NONE, 0, 0, INFINITE_AFFECT_DURATION, 0, false);
#endif

#ifdef __DSS_ACTIVE_EFFECT__
	if (deck_idx == DRAGON_SOUL_DECK_0)
	{
		SpecificEffectPacket("d:/ymir work/effect/etc/dragonsoul/dragonsoul_earth.mse");
		SetDragonSoulActivateTime(get_global_time());
	}
	else
	{
		SpecificEffectPacket("d:/ymir work/effect/etc/dragonsoul/dragonsoul_sky.mse");
		SetDragonSoulActivateTime(get_global_time());
	}
#endif

	/*Eskisi(efektveaktiflik)
	AddAffect(AFFECT_DRAGON_SOUL_DECK_0 + deck_idx, APPLY_NONE, 0, 0, INFINITE_AFFECT_DURATION, 0, false);
#ifdef __DSS_ACTIVE_EFFECT__
	SpecificEffectPacket("d:\\ymir work\\effect\\etc\\buff\\buff_runes.mse");
	SetDragonSoulActivateTime(get_global_time());
#endif	*/

	m_pointsInstant.iDragonSoulActiveDeck = deck_idx;

	for (int i = DRAGON_SOUL_EQUIP_SLOT_START + DS_SLOT_MAX * deck_idx;
		i < DRAGON_SOUL_EQUIP_SLOT_START + DS_SLOT_MAX * (deck_idx + 1); i++)
	{
		LPITEM pItem = GetInventoryItem(i);
		if (NULL != pItem)
			DSManager::instance().ActivateDragonSoul(pItem);
	}

#ifdef __DS_SET_BONUS__
	DragonSoul_HandleSetBonus();
#endif

	return true;
}

void CHARACTER::DragonSoul_DeactivateAll()
{
	for (int i = DRAGON_SOUL_EQUIP_SLOT_START; i < DRAGON_SOUL_EQUIP_SLOT_END; i++)
	{
		DSManager::instance().DeactivateDragonSoul(GetInventoryItem(i), true);
	}
#ifdef __DS_SET_BONUS__
	DragonSoul_HandleSetBonus();
#endif

	m_pointsInstant.iDragonSoulActiveDeck = -1;
	RemoveAffect(AFFECT_DRAGON_SOUL_DECK_0);
	RemoveAffect(AFFECT_DRAGON_SOUL_DECK_1);
#ifdef __DS_SET_BONUS__
	RemoveAffect(NEW_AFFECT_DS_SET);
#endif
}

#ifdef __DS_SET_BONUS__
void CHARACTER::DragonSoul_HandleSetBonus()
{
	bool bAdd = true;
	uint8_t iSetGrade;

	if (!DSManager::instance().GetDSSetGrade(this, iSetGrade))
	{
		CAffect* pkAffect = FindAffect(NEW_AFFECT_DS_SET);
		if (!pkAffect)
			return;

		iSetGrade = pkAffect->lApplyValue;
		bAdd = false;
	}
	else
	{
		AddAffect(NEW_AFFECT_DS_SET, APPLY_NONE, iSetGrade, 0, INFINITE_AFFECT_DURATION, 0, true);
	}

	const uint8_t iDeckIdx = DragonSoul_GetActiveDeck();
	const uint8_t iStartSlotIndex = WEAR_MAX_NUM + (iDeckIdx * DS_SLOT_MAX);
	const uint8_t iEndSlotIndex = iStartSlotIndex + DS_SLOT_MAX;

	for (uint8_t iSlotIndex = iStartSlotIndex; iSlotIndex < iEndSlotIndex; ++iSlotIndex)
	{
		const LPITEM pkItem = GetWear(iSlotIndex);
		if (!pkItem)
			return;

		for (uint8_t i = 0; i < ITEM_ATTRIBUTE_MAX_NUM; ++i)
		{
			if (pkItem->GetAttributeType(i))
			{
				const TPlayerItemAttribute& ia = pkItem->GetAttribute(i);
				int iSetValue = DSManager::instance().GetDSSetValue(i, ia.bType, pkItem->GetVnum(), iSetGrade);

				if (ia.bType == APPLY_SKILL)
				{
					iSetValue = bAdd ? iSetValue : iSetValue ^ 0x00800000;
				}
				else
				{
					iSetValue = bAdd ? iSetValue : -iSetValue;
				}

				ApplyPoint(ia.bType, iSetValue);
			}
		}
	}
}
#endif

void CHARACTER::DragonSoul_CleanUp()
{
	for (int i = DRAGON_SOUL_EQUIP_SLOT_START; i < DRAGON_SOUL_EQUIP_SLOT_END; i++)
	{
		DSManager::instance().DeactivateDragonSoul(GetInventoryItem(i), true);
	}
}

bool CHARACTER::DragonSoul_RefineWindow_Open(LPENTITY pEntity)
{
	if (NULL == m_pointsInstant.m_pDragonSoulRefineWindowOpener)
	{
		m_pointsInstant.m_pDragonSoulRefineWindowOpener = pEntity;
	}

	TPacketGCDragonSoulRefine PDS;
	PDS.header = HEADER_GC_DRAGON_SOUL_REFINE;
	PDS.bSubType = DS_SUB_HEADER_OPEN;
	LPDESC d = GetDesc();

	if (NULL == d)
	{
		sys_err("User(%s)'s DESC is NULL POINT.", GetName());
		return false;
	}

	d->Packet(&PDS, sizeof(PDS));
	return true;
}

bool CHARACTER::DragonSoul_RefineWindow_Close()
{
	m_pointsInstant.m_pDragonSoulRefineWindowOpener = NULL;
	return true;
}

bool CHARACTER::DragonSoul_RefineWindow_CanRefine()
{
	return NULL != m_pointsInstant.m_pDragonSoulRefineWindowOpener;
}