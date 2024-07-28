#include "stdafx.h"
#include "utils.h"
#include "config.h"
#include "char.h"
#include "desc.h"
#include "sectree_manager.h"
#include "packet.h"
#include "protocol.h"
#include "log.h"
#include "skill.h"
#include "unique_item.h"
#include "item_addon.h"
#include "locale_service.h"
#include "item.h"
#include "item_manager.h"
#include "affect.h"
#include "DragonSoul.h"
#include "buff_on_attributes.h"
#include "belt_inventory_helper.h"
#include "../../common/VnumHelper.h"
#ifdef __ITEM_ENTITY_UTILITY__
#include "mob_manager.h"
#endif
#ifdef __PET_SYSTEM_PROTO__
#include "PetSystem.h"
#endif
#include "war_map.h"

CItem::CItem(DWORD dwVnum)
	: m_dwVnum(dwVnum), m_bWindow(0), m_dwID(0), m_bEquipped(false), m_dwVID(0), m_wCell(0), m_dwCount(0), m_lFlag(0), m_dwLastOwnerPID(0),
	m_bExchanging(false), m_pkDestroyEvent(NULL), m_pkExpireEvent(NULL), m_pkUniqueExpireEvent(NULL),
	m_pkTimerBasedOnWearExpireEvent(NULL), m_pkRealTimeExpireEvent(NULL),
	m_pkAccessorySocketExpireEvent(NULL), m_pkOwnershipEvent(NULL), m_dwOwnershipPID(0), m_bSkipSave(false), m_isLocked(false),
	m_dwMaskVnum(0), m_dwSIGVnum(0)
#ifdef __BEGINNER_ITEM__
	, is_basic(false)
#endif
#ifdef __ITEM_EVOLUTION__
	, m_dwEvolution(0)
#endif
#ifdef __ITEM_CHANGELOOK__
	, m_dwTransmutation(0)
#endif
{
	memset(&m_alSockets, 0, sizeof(m_alSockets));
	memset(&m_aAttr, 0, sizeof(m_aAttr));
}

CItem::~CItem()
{
	Destroy();
}

void CItem::Initialize()
{
	CEntity::Initialize(ENTITY_ITEM);

	m_bWindow = RESERVED_WINDOW;
	m_pOwner = NULL;
	m_dwID = 0;
	m_bEquipped = false;
	m_dwVID = m_wCell = m_dwCount = m_lFlag = 0;
	m_pProto = NULL;
	m_bExchanging = false;
	memset(&m_alSockets, 0, sizeof(m_alSockets));
	memset(&m_aAttr, 0, sizeof(m_aAttr));

	m_pkDestroyEvent = NULL;
	m_pkOwnershipEvent = NULL;
	m_dwOwnershipPID = 0;
	m_pkUniqueExpireEvent = NULL;
	m_pkTimerBasedOnWearExpireEvent = NULL;
	m_pkRealTimeExpireEvent = NULL;

	m_pkAccessorySocketExpireEvent = NULL;

	m_bSkipSave = false;
	m_dwLastOwnerPID = 0;
#ifdef __BEGINNER_ITEM__
	is_basic = false;
#endif
#ifdef __ITEM_EVOLUTION__
	m_dwEvolution = 0;
#endif
#ifdef __ITEM_CHANGELOOK__
	m_dwTransmutation = 0;
#endif
}

void CItem::Destroy()
{
	event_cancel(&m_pkDestroyEvent);
	event_cancel(&m_pkOwnershipEvent);
	event_cancel(&m_pkUniqueExpireEvent);
	event_cancel(&m_pkTimerBasedOnWearExpireEvent);
	event_cancel(&m_pkRealTimeExpireEvent);
	event_cancel(&m_pkAccessorySocketExpireEvent);
	CEntity::Destroy();

	if (GetSectree())
		GetSectree()->RemoveEntity(this);
}

EVENTFUNC(item_destroy_event)
{
	item_event_info* info = dynamic_cast<item_event_info*>(event->info);

	if (info == NULL)
	{
		sys_err("item_destroy_event> <Factor> Null pointer");
		return 0;
	}

	LPITEM pkItem = info->item;

	if (pkItem->GetOwner())
		sys_err("item_destroy_event: Owner exist. (item %s owner %s)", pkItem->GetName(), pkItem->GetOwner()->GetName());

	pkItem->SetDestroyEvent(NULL);
	M2_DESTROY_ITEM(pkItem);
	return 0;
}

void CItem::SetDestroyEvent(LPEVENT pkEvent)
{
	m_pkDestroyEvent = pkEvent;
}

void CItem::StartDestroyEvent(int iSec)
{
	if (m_pkDestroyEvent)
		return;

	item_event_info* info = AllocEventInfo<item_event_info>();
	info->item = this;

	SetDestroyEvent(event_create(item_destroy_event, info, PASSES_PER_SEC(iSec)));
}

void CItem::EncodeInsertPacket(LPENTITY ent)
{
	LPDESC d;

	if (!(d = ent->GetDesc()))
		return;

	const PIXEL_POSITION& c_pos = GetXYZ();

	struct packet_item_ground_add pack;

	pack.bHeader = HEADER_GC_ITEM_GROUND_ADD;
	pack.x = c_pos.x;
	pack.y = c_pos.y;
	pack.z = c_pos.z;
	pack.dwVnum = GetVnum();
	pack.dwVID = m_dwVID;
#ifdef __ITEM_ENTITY_UTILITY__
	for (size_t i = 0; i < ITEM_SOCKET_MAX_NUM; ++i)
		pack.alSockets[i] = GetSocket(i);

	pack.wCount = m_dwCount;
#endif

	d->Packet(&pack, sizeof(pack));

	if (m_pkOwnershipEvent != NULL)
	{
		item_event_info* info = dynamic_cast<item_event_info*>(m_pkOwnershipEvent->info);

		if (info == NULL)
		{
			sys_err("CItem::EncodeInsertPacket> <Factor> Null pointer");
			return;
		}

		TPacketGCItemOwnership p;

		p.bHeader = HEADER_GC_ITEM_OWNERSHIP;
		p.dwVID = m_dwVID;
		strlcpy(p.szName, info->szOwnerName, sizeof(p.szName));

		d->Packet(&p, sizeof(TPacketGCItemOwnership));
	}
}

void CItem::EncodeRemovePacket(LPENTITY ent)
{
	LPDESC d;

	if (!(d = ent->GetDesc()))
		return;

	struct packet_item_ground_del pack;

	pack.bHeader = HEADER_GC_ITEM_GROUND_DEL;
	pack.dwVID = m_dwVID;

	d->Packet(&pack, sizeof(pack));
	sys_log(2, "Item::EncodeRemovePacket %s to %s", GetName(), ((LPCHARACTER)ent)->GetName());
}

void CItem::SetProto(const TItemTable* table)
{
	assert(table != NULL);
	m_pProto = table;
	SetFlag(m_pProto->dwFlags);
}

void CItem::UsePacketEncode(LPCHARACTER ch, LPCHARACTER victim, struct packet_item_use* packet)
{
	if (!GetVnum())
		return;

	packet->header = HEADER_GC_ITEM_USE;
	packet->ch_vid = ch->GetVID();
	packet->victim_vid = victim->GetVID();
	packet->Cell = TItemPos(GetWindow(), m_wCell);
	packet->vnum = GetVnum();
}

void CItem::RemoveFlag(long bit)
{
	REMOVE_BIT(m_lFlag, bit);
}

void CItem::AddFlag(long bit)
{
	SET_BIT(m_lFlag, bit);
}

void CItem::UpdatePacket()
{
	if (!m_pOwner || !m_pOwner->GetDesc())
		return;

#ifdef __SWITCHBOT__
	if (m_bWindow == SWITCHBOT)
		return;
#endif

	TPacketGCItemUpdate pack;

	pack.header = HEADER_GC_ITEM_UPDATE;
	pack.Cell = TItemPos(GetWindow(), m_wCell);
	pack.count = m_dwCount;

	for (int i = 0; i < ITEM_SOCKET_MAX_NUM; ++i)
		pack.alSockets[i] = m_alSockets[i];

	thecore_memcpy(pack.aAttr, GetAttributes(), sizeof(pack.aAttr));
#ifdef __BEGINNER_ITEM__
	pack.is_basic = is_basic;
#endif
#ifdef __ITEM_EVOLUTION__
	pack.evolution = m_dwEvolution;
#endif
#ifdef __ITEM_CHANGELOOK__
	pack.transmutation = m_dwTransmutation;
#endif
	sys_log(2, "UpdatePacket %s -> %s", GetName(), m_pOwner->GetName());
	m_pOwner->GetDesc()->Packet(&pack, sizeof(pack));
}

DWORD CItem::GetCount()
{
	if (GetType() == ITEM_ELK)
		return MIN(m_dwCount, INT_MAX);
	else
	{
		return MIN(m_dwCount, g_bItemCountLimit);
	}
}

bool CItem::SetCount(DWORD count)
{
	if (GetType() == ITEM_ELK)
	{
		m_dwCount = MIN(count, INT_MAX);
	}
	else
	{
		m_dwCount = MIN(count, g_bItemCountLimit);
	}

	if (count == 0 && m_pOwner)
	{
		if (GetSubType() == USE_ABILITY_UP || GetSubType() == USE_POTION || GetVnum() == 70020)
		{
			LPCHARACTER pOwner = GetOwner();
			WORD wCell = GetCell();

			RemoveFromCharacter();

#ifdef __ADDITIONAL_INVENTORY__
			if (!IsDragonSoul() && !IsUpgradeItem() && !IsBook() && !IsStone() && !IsFlower() && !IsAttrItem() && !IsChest())
#else
			if (!IsDragonSoul())
#endif
			{
				LPITEM pItem = pOwner->FindSpecifyItem(GetVnum());

				if (NULL != pItem)
				{
					pOwner->ChainQuickslotItem(pItem, QUICKSLOT_TYPE_ITEM, wCell);
				}
				else
				{
					pOwner->SyncQuickslot(QUICKSLOT_TYPE_ITEM, wCell, 255);
				}
			}

			M2_DESTROY_ITEM(this);
		}
		else
		{
#ifdef __ADDITIONAL_INVENTORY__
			if (!IsDragonSoul() && !IsUpgradeItem() && !IsBook() && !IsStone() && !IsFlower() && !IsAttrItem() && !IsChest())
#else
			if (!IsDragonSoul())
#endif
			{
				m_pOwner->SyncQuickslot(QUICKSLOT_TYPE_ITEM, m_wCell, 255);
			}
			M2_DESTROY_ITEM(RemoveFromCharacter());
		}

		return false;
	}

	UpdatePacket();

	Save();
	return true;
}

#ifdef __ITEM_CHANGELOOK__
DWORD CItem::GetTransmutation() const
{
	return m_dwTransmutation;
}

void CItem::SetTransmutation(DWORD dwVnum, bool bLog)
{
	m_dwTransmutation = dwVnum;
	UpdatePacket();
	Save();
}
#endif

LPITEM CItem::RemoveFromCharacter()
{
	if (!m_pOwner)
	{
		sys_err("Item::RemoveFromCharacter owner null");
		return (this);
	}

	LPCHARACTER pOwner = m_pOwner;

	if (m_bEquipped)
	{
		Unequip();
		//pOwner->UpdatePacket();

		SetWindow(RESERVED_WINDOW);
		Save();
		return (this);
	}
	else
	{
#ifdef __SWITCHBOT__
		if (GetWindow() != SAFEBOX && GetWindow() != MALL && GetWindow() != SWITCHBOT)
#else
		if (GetWindow() != SAFEBOX && GetWindow() != MALL)
#endif
		{
			if (IsDragonSoul())
			{
				if (m_wCell >= DRAGON_SOUL_INVENTORY_MAX_NUM)
					sys_err("CItem::RemoveFromCharacter: pos >= DRAGON_SOUL_INVENTORY_MAX_NUM");
				else
					pOwner->__SET_ITEM(TItemPos(m_bWindow, m_wCell), NULL);
			}
#ifdef __ADDITIONAL_INVENTORY__
			else if (IsUpgradeItem())
			{
				if (m_wCell >= SPECIAL_INVENTORY_MAX_NUM)
					sys_err("CItem::RemoveFromCharacter: pos >= SPECIAL_INVENTORY_MAX_NUM IsUpgradeItem");
				else
					pOwner->__SET_ITEM(TItemPos(UPGRADE_INVENTORY, m_wCell), NULL);
			}
			else if (IsBook())
			{
				if (m_wCell >= SPECIAL_INVENTORY_MAX_NUM)
					sys_err("CItem::RemoveFromCharacter: pos >= SPECIAL_INVENTORY_MAX_NUM IsBook");
				else
					pOwner->__SET_ITEM(TItemPos(BOOK_INVENTORY, m_wCell), NULL);
			}
			else if (IsStone())
			{
				if (m_wCell >= SPECIAL_INVENTORY_MAX_NUM)
					sys_err("CItem::RemoveFromCharacter: pos >= SPECIAL_INVENTORY_MAX_NUM IsStone");
				else
					pOwner->__SET_ITEM(TItemPos(STONE_INVENTORY, m_wCell), NULL);
			}
			else if (IsAttrItem())
			{
				if (m_wCell >= SPECIAL_INVENTORY_MAX_NUM)
					sys_err("CItem::RemoveFromCharacter: pos >= SPECIAL_INVENTORY_MAX_NUM IsAttrItem");
				else
					pOwner->__SET_ITEM(TItemPos(ATTR_INVENTORY, m_wCell), NULL);
			}
			else if (IsFlower())
			{
				if (m_wCell >= SPECIAL_INVENTORY_MAX_NUM)
					sys_err("CItem::RemoveFromCharacter: pos >= SPECIAL_INVENTORY_MAX_NUM IsFlower");
				else
					pOwner->__SET_ITEM(TItemPos(FLOWER_INVENTORY, m_wCell), NULL);
			}
			else if (IsChest())
			{
				if (m_wCell >= SPECIAL_INVENTORY_MAX_NUM)
					sys_err("CItem::RemoveFromCharacter: pos >= SPECIAL_INVENTORY_MAX_NUM IsChest");
				else
					pOwner->__SET_ITEM(TItemPos(CHEST_INVENTORY, m_wCell), NULL);
			}
#endif
			else
			{
				TItemPos cell(INVENTORY, m_wCell);

				if (false == cell.IsDefaultInventoryPosition() && false == cell.IsBeltInventoryPosition())
					sys_err("CItem::RemoveFromCharacter: Invalid Item Position");
				else
				{
					pOwner->__SET_ITEM(cell, NULL);
				}
			}
		}
#ifdef __SWITCHBOT__
		else if (GetWindow() == SWITCHBOT)
		{
			if (m_wCell >= SWITCHBOT_SLOT_COUNT)
			{
				sys_err("CItem::RemoveFromCharacter: pos >= SWITCHBOT_SLOT_COUNT");
			}
			else
			{
				pOwner->__SET_ITEM(TItemPos(SWITCHBOT, m_wCell), NULL);
			}
		}
#endif

		m_pOwner = NULL;
		m_wCell = 0;

		SetWindow(RESERVED_WINDOW);
		Save();
		return (this);
	}
}

bool CItem::AddToCharacter(LPCHARACTER ch, TItemPos Cell, const char* file, int line)
{
	assert(GetSectree() == NULL);
	assert(m_pOwner == NULL);

	WORD pos = Cell.cell;
	BYTE window_type = Cell.window_type;

	if (INVENTORY == window_type)
	{
		if (m_wCell >= INVENTORY_MAX_NUM && BELT_INVENTORY_SLOT_START > m_wCell)
		{
			sys_err("CItem::AddToCharacter: cell overflow: %s to %s cell %d", m_pProto->szName, ch->GetName(), m_wCell);
			return false;
		}
	}
	else if (DRAGON_SOUL_INVENTORY == window_type)
	{
		if (m_wCell >= DRAGON_SOUL_INVENTORY_MAX_NUM)
		{
			sys_err("CItem::AddToCharacter: cell overflow: %s to %s cell %d", m_pProto->szName, ch->GetName(), m_wCell);
			return false;
		}
	}
#ifdef __ADDITIONAL_INVENTORY__
	else if (UPGRADE_INVENTORY == window_type)
	{
		if (m_wCell >= SPECIAL_INVENTORY_MAX_NUM)
		{
			sys_err("CItem::AddToCharacter: cell overflow upgrade inventory: %s to %s cell %d", m_pProto->szName, ch->GetName(), m_wCell);
			return false;
		}
	}
	else if (BOOK_INVENTORY == window_type)
	{
		if (m_wCell >= SPECIAL_INVENTORY_MAX_NUM)
		{
			sys_err("CItem::AddToCharacter: cell overflow book inventory: %s to %s cell %d", m_pProto->szName, ch->GetName(), m_wCell);
			return false;
		}
	}
	else if (STONE_INVENTORY == window_type)
	{
		if (m_wCell >= SPECIAL_INVENTORY_MAX_NUM)
		{
			sys_err("CItem::AddToCharacter: cell overflow stone inventory: %s to %s cell %d", m_pProto->szName, ch->GetName(), m_wCell);
			return false;
		}
	}
	else if (FLOWER_INVENTORY == window_type)
	{
		if (m_wCell >= SPECIAL_INVENTORY_MAX_NUM)
		{
			sys_err("CItem::AddToCharacter: cell overflow flower inventory: %s to %s cell %d", m_pProto->szName, ch->GetName(), m_wCell);
			return false;
		}
	}
	else if (ATTR_INVENTORY == window_type)
	{
		if (m_wCell >= SPECIAL_INVENTORY_MAX_NUM)
		{
			sys_err("CItem::AddToCharacter: cell overflow attr inventory: %s to %s cell %d", m_pProto->szName, ch->GetName(), m_wCell);
			return false;
		}
	}
	else if (CHEST_INVENTORY == window_type)
	{
		if (m_wCell >= SPECIAL_INVENTORY_MAX_NUM)
		{
			sys_err("CItem::AddToCharacter: cell overflow chest inventory: %s to %s cell %d", m_pProto->szName, ch->GetName(), m_wCell);
			return false;
		}
	}
#endif
#ifdef __SWITCHBOT__
	else if (SWITCHBOT == window_type)
	{
		if (m_wCell >= SWITCHBOT_SLOT_COUNT)
		{
			sys_err("CItem::AddToCharacter:switchbot cell overflow: %s to %s cell %d", m_pProto->szName, ch->GetName(), m_wCell);
			return false;
		}
	}
#endif

#ifdef __HIGHLIGHT_ITEM__
	bool bWereMine = this->GetLastOwnerPID() == ch->GetPlayerID();
#endif

	if (ch->GetDesc())
		m_dwLastOwnerPID = ch->GetPlayerID();

#ifdef __ACCE_SYSTEM__
	if ((GetType() == ITEM_COSTUME) && (GetSubType() == COSTUME_ACCE) && (GetSocket(ACCE_ABSORPTION_SOCKET) == 0))
	{
		long lVal = GetValue(ACCE_GRADE_VALUE_FIELD);
		switch (lVal)
		{
		case 2:
		{
			lVal = ACCE_GRADE_2_ABS;
		}
		break;
		case 3:
		{
			lVal = ACCE_GRADE_3_ABS;
		}
		break;
		case 4:
		{
			lVal = number(ACCE_GRADE_4_ABS_MIN, ACCE_GRADE_4_ABS_MAX_COMB);
		}
		break;
		default:
		{
			lVal = ACCE_GRADE_1_ABS;
		}
		break;
		}

		SetSocket(ACCE_ABSORPTION_SOCKET, lVal);
	}
#endif
	event_cancel(&m_pkDestroyEvent);

	LPITEM oldItem;
	if ((oldItem = ch->GetItem(Cell)))
	{
		sys_err("old item bulundu! %u %u %s %d - %d %d - %u", GetID(), ch->GetPlayerID(), file, line, window_type, pos, oldItem->GetID());
		AddToGround(ch->GetMapIndex(), ch->GetXYZ());
		StartDestroyEvent(g_aiItemDestroyTime[ITEM_DESTROY_TIME_AUTOGIVE]);
		SetOwnership(ch, 300);
		return true;
	}

#ifdef __HIGHLIGHT_ITEM__
	ch->__SET_ITEM_HG(TItemPos(window_type, pos), this, bWereMine);
#else
	ch->SetItem(TItemPos(window_type, pos), this);
#endif
	m_pOwner = ch;

	Save();
	return true;
}

LPITEM CItem::RemoveFromGround()
{
	if (GetSectree())
	{
		SetOwnership(NULL);

		GetSectree()->RemoveEntity(this);

		ViewCleanup();

		Save();
	}

	return (this);
}

bool CItem::AddToGround(long lMapIndex, const PIXEL_POSITION& pos, bool skipOwnerCheck)
{
	if (0 == lMapIndex)
	{
		sys_err("wrong map index argument: %d", lMapIndex);
		return false;
	}

	if (GetSectree())
	{
		sys_err("sectree already assigned");
		return false;
	}

	if (!skipOwnerCheck && m_pOwner)
	{
		sys_err("owner pointer not null");
		return false;
	}

	LPSECTREE tree = SECTREE_MANAGER::instance().Get(lMapIndex, pos.x, pos.y);

	if (!tree)
	{
		sys_err("cannot find sectree by %dx%d", pos.x, pos.y);
		return false;
	}

	//tree->Touch();

	SetWindow(GROUND);
	SetXYZ(pos.x, pos.y, pos.z);
	tree->InsertEntity(this);
	UpdateSectree();
	Save();
	return true;
}

bool CItem::DistanceValid(LPCHARACTER ch)
{
	if (!GetSectree())
		return false;

	if (!ch)
		return false;

	int iDist = DISTANCE_APPROX(GetX() - ch->GetX(), GetY() - ch->GetY());

	if (iDist > 1500)
		return false;

	return true;
}

bool CItem::CanUsedBy(LPCHARACTER ch)
{
	// Anti flag check
	switch (ch->GetJob())
	{
	case JOB_WARRIOR:
		if (GetAntiFlag() & ITEM_ANTIFLAG_WARRIOR)
			return false;
		break;

	case JOB_ASSASSIN:
		if (GetAntiFlag() & ITEM_ANTIFLAG_ASSASSIN)
			return false;
		break;

	case JOB_SHAMAN:
		if (GetAntiFlag() & ITEM_ANTIFLAG_SHAMAN)
			return false;
		break;

	case JOB_SURA:
		if (GetAntiFlag() & ITEM_ANTIFLAG_SURA)
			return false;
		break;
#ifdef __WOLFMAN_CHARACTER__
	case JOB_WOLFMAN:
		if (GetAntiFlag() & ITEM_ANTIFLAG_WOLFMAN)
			return false;
		break;
#endif
	}

	return true;
}

int CItem::FindEquipCell(LPCHARACTER ch, int iCandidateCell)
{
	if ((0 == GetWearFlag() || ITEM_TOTEM == GetType()) && ITEM_COSTUME != GetType() && ITEM_DS != GetType() && ITEM_SPECIAL_DS != GetType() && ITEM_RING != GetType() && ITEM_BELT != GetType()
#ifdef __SHINING_ITEM_SYSTEM__
		&& ITEM_SHINING != GetType()
#endif
#ifdef __CAKRA_ITEM_SYSTEM__
		&& ITEM_CAKRA != GetType()
#endif
#ifdef __PET_SYSTEM_PROTO__
		&& !IsPetItem()
#endif
#ifdef __SEBNEM_ITEM_SYSTEM__
		&& ITEM_SEBNEM != GetType()
#endif
		)
		return -1;

	if (GetType() == ITEM_DS || GetType() == ITEM_SPECIAL_DS)
	{
		if (iCandidateCell < 0)
		{
			return WEAR_MAX_NUM + GetSubType();
		}
		else
		{
			for (int i = 0; i < DRAGON_SOUL_DECK_MAX_NUM; i++)
			{
				if (WEAR_MAX_NUM + i * DS_SLOT_MAX + GetSubType() == iCandidateCell)
				{
					return iCandidateCell;
				}
			}
			return -1;
		}
	}
	else if (GetType() == ITEM_COSTUME)
	{
		if (GetSubType() == COSTUME_BODY)
			return WEAR_COSTUME_BODY;
		else if (GetSubType() == COSTUME_HAIR)
			return WEAR_COSTUME_HAIR;
#ifdef __MOUNT_COSTUME_SYSTEM__
		else if (GetSubType() == COSTUME_MOUNT)
			return WEAR_COSTUME_MOUNT;
#endif
#ifdef __ACCE_SYSTEM__
		else if (GetSubType() == COSTUME_ACCE)
			return WEAR_COSTUME_ACCE;
#endif
#ifdef __WEAPON_COSTUME_SYSTEM__
		else if (GetSubType() == COSTUME_WEAPON)
			return WEAR_COSTUME_WEAPON;
#endif
#ifdef __AURA_COSTUME_SYSTEM__
		else if (GetSubType() == COSTUME_AURA)
			return WEAR_COSTUME_AURA;
#endif
	}
	else if (GetType() == ITEM_RING)
	{
		if (ch->GetWear(WEAR_RING1))
			return WEAR_RING2;
		else
			return WEAR_RING1;
	}
	else if (GetType() == ITEM_BELT)
		return WEAR_BELT;
	else if (GetWearFlag() & WEARABLE_BODY)
		return WEAR_BODY;
	else if (GetWearFlag() & WEARABLE_HEAD)
		return WEAR_HEAD;
	else if (GetWearFlag() & WEARABLE_FOOTS)
		return WEAR_FOOTS;
	else if (GetWearFlag() & WEARABLE_WRIST)
		return WEAR_WRIST;
	else if (GetWearFlag() & WEARABLE_WEAPON)
		return WEAR_WEAPON;
	else if (GetWearFlag() & WEARABLE_SHIELD)
		return WEAR_SHIELD;
	else if (GetWearFlag() & WEARABLE_NECK)
		return WEAR_NECK;
	else if (GetWearFlag() & WEARABLE_EAR)
		return WEAR_EAR;
	else if (GetWearFlag() & WEARABLE_ARROW)
		return WEAR_ARROW;
#ifdef __PENDANT_SYSTEM__
	else if (GetWearFlag() & WEARABLE_PENDANT)
		return WEAR_PENDANT;
#endif
	else if (GetWearFlag() & WEARABLE_UNIQUE)
	{
		if (ch->GetWear(WEAR_UNIQUE1))
			return WEAR_UNIQUE2;
		else
			return WEAR_UNIQUE1;
	}

	else if (GetWearFlag() & WEARABLE_ABILITY)
	{
		if (!ch->GetWear(WEAR_ABILITY1))
		{
			return WEAR_ABILITY1;
		}
		else if (!ch->GetWear(WEAR_ABILITY2))
		{
			return WEAR_ABILITY2;
		}
		else if (!ch->GetWear(WEAR_ABILITY3))
		{
			return WEAR_ABILITY3;
		}
		else if (!ch->GetWear(WEAR_ABILITY4))
		{
			return WEAR_ABILITY4;
		}
		else if (!ch->GetWear(WEAR_ABILITY5))
		{
			return WEAR_ABILITY5;
		}
		else if (!ch->GetWear(WEAR_ABILITY6))
		{
			return WEAR_ABILITY6;
		}
		else if (!ch->GetWear(WEAR_ABILITY7))
		{
			return WEAR_ABILITY7;
		}
		else if (!ch->GetWear(WEAR_ABILITY8))
		{
			return WEAR_ABILITY8;
		}
		else
		{
			return -1;
		}
	}
#ifdef __SHINING_ITEM_SYSTEM__
	else if (GetType() == ITEM_SHINING)
	{
		switch (GetSubType())
		{
		case SHINING_WEAPON: return WEAR_SHINING_WEAPON;
		case SHINING_ARMOR: return WEAR_SHINING_ARMOR;
		case SHINING_SPECIAL: return WEAR_SHINING_SPECIAL;
		case SHINING_SPECIAL2: return WEAR_SHINING_SPECIAL2;
		case SHINING_SPECIAL3: return WEAR_SHINING_SPECIAL3;
		case SHINING_WING: return WEAR_SHINING_WING;
		}
	}
#endif
#ifdef __CAKRA_ITEM_SYSTEM__
	else if (GetType() == ITEM_CAKRA)
	{
		switch (GetSubType())
		{
		case CAKRA_ITEM_1: return WEAR_CAKRA_1;
		case CAKRA_ITEM_2: return WEAR_CAKRA_2;
		case CAKRA_ITEM_3: return WEAR_CAKRA_3;
		case CAKRA_ITEM_4: return WEAR_CAKRA_4;
		case CAKRA_ITEM_5: return WEAR_CAKRA_5;
		case CAKRA_ITEM_6: return WEAR_CAKRA_6;
		case CAKRA_ITEM_7: return WEAR_CAKRA_7;
		case CAKRA_ITEM_8: return WEAR_CAKRA_8;
		}
	}
#endif
#ifdef __PET_SYSTEM_PROTO__
	if (IsPetItem() && !CWarMapManager::instance().IsWarMap(m_pOwner->GetMapIndex()))
		return WEAR_PET;
#endif
#ifdef __SEBNEM_ITEM_SYSTEM__
	else if (GetType() == ITEM_SEBNEM)
	{
		switch (GetSubType())
		{
		case SEBNEM_ITEM_1: return WEAR_SEBNEM_1;
		case SEBNEM_ITEM_2: return WEAR_SEBNEM_2;
		case SEBNEM_ITEM_3: return WEAR_SEBNEM_3;
		case SEBNEM_ITEM_4: return WEAR_SEBNEM_4;
		case SEBNEM_ITEM_5: return WEAR_SEBNEM_5;
		case SEBNEM_ITEM_6: return WEAR_SEBNEM_6;
		}
	}
#endif
	return -1;
}

void CItem::ModifyPoints(bool bAdd)
{
	if (IsNewPetItem())
		return;

	int accessoryGrade;

	if (!m_pOwner)
	{
		// passed without owner
		return;
	}

	if (false == IsAccessoryForSocket())
	{
#ifdef __QUIVER_SYSTEM__
#ifdef __BEGINNER_ITEM__
		if (((m_pProto->bType == ITEM_WEAPON || m_pProto->bType == ITEM_ARMOR) && m_pProto->bSubType != WEAPON_QUIVER) && !IsRealTimeItem())
#else
		if ((m_pProto->bType == ITEM_WEAPON || m_pProto->bType == ITEM_ARMOR) && m_pProto->bSubType != WEAPON_QUIVER)
#endif
#else
		if (m_pProto->bType == ITEM_WEAPON || m_pProto->bType == ITEM_ARMOR)
#endif
		{
			for (int i = 0; i < ITEM_SOCKET_MAX_NUM; ++i)
			{
				DWORD dwVnum;
				if ((dwVnum = GetSocket(i)) <= 2)
					continue;

				TItemTable* p = ITEM_MANAGER::instance().GetTable(dwVnum);

				if (!p)
				{
					sys_err("cannot find table by vnum %u", dwVnum);
					continue;
				}

				if (ITEM_METIN == p->bType)
				{
					//m_pOwner->ApplyPoint(p->alValues[0], bAdd ? p->alValues[1] : -p->alValues[1]);
					for (int i = 0; i < ITEM_APPLY_MAX_NUM; ++i)
					{
						if (p->aApplies[i].bType == APPLY_NONE)
							continue;
#ifdef __MOUNT_COSTUME_SYSTEM__
						if (IsMountItem())
							continue;
#endif
						if (p->aApplies[i].bType == APPLY_SKILL)
							m_pOwner->ApplyPoint(p->aApplies[i].bType, bAdd ? p->aApplies[i].lValue : p->aApplies[i].lValue ^ 0x00800000);
						else
							m_pOwner->ApplyPoint(p->aApplies[i].bType, bAdd ? p->aApplies[i].lValue : -p->aApplies[i].lValue);
					}
				}
			}
		}

		accessoryGrade = 0;
	}
	else
	{
		accessoryGrade = MIN(GetAccessorySocketGrade(), ITEM_ACCESSORY_SOCKET_MAX_NUM);
	}

#ifdef __ACCE_SYSTEM__
	if ((GetType() == ITEM_COSTUME) && (GetSubType() == COSTUME_ACCE) && (GetSocket(ACCE_ABSORBED_SOCKET)))
	{
		TItemTable* pkItemAbsorbed = ITEM_MANAGER::instance().GetTable(GetSocket(ACCE_ABSORBED_SOCKET));
		if (pkItemAbsorbed)
		{
			if ((pkItemAbsorbed->bType == ITEM_ARMOR) && (pkItemAbsorbed->bSubType == ARMOR_BODY))
			{
				long lDefGrade = pkItemAbsorbed->alValues[1] + long(pkItemAbsorbed->alValues[5] * 2);
				double dValue = lDefGrade * GetSocket(ACCE_ABSORPTION_SOCKET);
				dValue = (double)dValue / 100;
				dValue = (double)dValue + .5;
				lDefGrade = (long)dValue;
				if ((pkItemAbsorbed->alValues[1] > 0 && (lDefGrade <= 0)) || (pkItemAbsorbed->alValues[5] > 0 && (lDefGrade < 1)))
					lDefGrade += 1;
				else if ((pkItemAbsorbed->alValues[1] > 0) || (pkItemAbsorbed->alValues[5] > 0))
					lDefGrade += 1;

				m_pOwner->ApplyPoint(APPLY_DEF_GRADE_BONUS, bAdd ? lDefGrade : -lDefGrade);

				long lDefMagicBonus = pkItemAbsorbed->alValues[0];
				dValue = lDefMagicBonus * GetSocket(ACCE_ABSORPTION_SOCKET);
				dValue = (double)dValue / 100;
				dValue = (double)dValue + .5;
				lDefMagicBonus = (long)dValue;
				if ((pkItemAbsorbed->alValues[0] > 0) && (lDefMagicBonus < 1))
					lDefMagicBonus += 1;
				else if (pkItemAbsorbed->alValues[0] > 0)
					lDefMagicBonus += 1;

				m_pOwner->ApplyPoint(APPLY_MAGIC_DEF_GRADE, bAdd ? lDefMagicBonus : -lDefMagicBonus);
			}
			else if (pkItemAbsorbed->bType == ITEM_WEAPON) // @duzenleme kusak efsunlariyla alakali birsey berkay sen ekledin sanirsam.
			{
				//Basic attack value from weapon
				if (pkItemAbsorbed->alValues[3] + pkItemAbsorbed->alValues[4] > 0)
				{
					long lAttGrade = pkItemAbsorbed->alValues[4] + pkItemAbsorbed->alValues[5];
					if (pkItemAbsorbed->alValues[3] > pkItemAbsorbed->alValues[4])
						lAttGrade = pkItemAbsorbed->alValues[3] + pkItemAbsorbed->alValues[5];

					double dValue = lAttGrade * GetSocket(ACCE_ABSORPTION_SOCKET);
					dValue = (double)dValue / 100;
					dValue = (double)dValue + .5;
					lAttGrade = (long)dValue;
					if (((pkItemAbsorbed->alValues[3] > 0) && (lAttGrade < 1)) || ((pkItemAbsorbed->alValues[4] > 0) && (lAttGrade < 1)))
						lAttGrade += 1;
					else if ((pkItemAbsorbed->alValues[3] > 0) || (pkItemAbsorbed->alValues[4] > 0))
						lAttGrade += 1;
					m_pOwner->ApplyPoint(APPLY_ATT_GRADE_BONUS, bAdd ? lAttGrade : -lAttGrade);
				}
				//Basic magic attack value from weapon
				if (pkItemAbsorbed->alValues[1] + pkItemAbsorbed->alValues[2] > 0)
				{
					long lAttMagicGrade = pkItemAbsorbed->alValues[2] + pkItemAbsorbed->alValues[5];
					if (pkItemAbsorbed->alValues[1] > pkItemAbsorbed->alValues[2])
						lAttMagicGrade = pkItemAbsorbed->alValues[1] + pkItemAbsorbed->alValues[5];
					double dValue = lAttMagicGrade * GetSocket(ACCE_ABSORPTION_SOCKET);
					dValue = (double)dValue / 100;
					dValue = (double)dValue + .5;
					lAttMagicGrade = (long)dValue;
					if (((pkItemAbsorbed->alValues[1] > 0) && (lAttMagicGrade < 1)) || ((pkItemAbsorbed->alValues[2] > 0) && (lAttMagicGrade < 1)))
						lAttMagicGrade += 1;
					else if ((pkItemAbsorbed->alValues[1] > 0) || (pkItemAbsorbed->alValues[2] > 0))
						lAttMagicGrade += 1;

					m_pOwner->ApplyPoint(APPLY_MAGIC_ATT_GRADE, bAdd ? lAttMagicGrade : -lAttMagicGrade);
				}
			}
		}
	}
#endif

	for (int i = 0; i < ITEM_APPLY_MAX_NUM; ++i)
	{
		if ((m_pProto->aApplies[i].bType == APPLY_NONE)
#ifdef __ACCE_SYSTEM__
			&& (GetType() != ITEM_COSTUME) && (GetSubType() != COSTUME_ACCE))
#endif
			continue;

		BYTE bType = m_pProto->aApplies[i].bType;
		long value = m_pProto->aApplies[i].lValue;
#ifdef __ACCE_SYSTEM__
		if ((GetType() == ITEM_COSTUME) && (GetSubType() == COSTUME_ACCE))
		{
			TItemTable* pkItemAbsorbed = ITEM_MANAGER::instance().GetTable(GetSocket(ACCE_ABSORBED_SOCKET));
			if (pkItemAbsorbed)
			{
				if (pkItemAbsorbed->aApplies[i].bType == APPLY_NONE)
					continue;

				bType = pkItemAbsorbed->aApplies[i].bType;
				value = pkItemAbsorbed->aApplies[i].lValue;
				if (value < 0)
					continue;

				double dValue = value * GetSocket(ACCE_ABSORPTION_SOCKET);
				dValue = (double)dValue / 100;
				dValue = (double)dValue + .5;
				value = (long)dValue;
				if ((pkItemAbsorbed->aApplies[i].lValue > 0) && (value <= 0))
					value += 1;
			}
			else
				continue;
		}
#endif
		if (bType == APPLY_SKILL)
		{
			m_pOwner->ApplyPoint(bType, bAdd ? value : value ^ 0x00800000);
		}
		else
		{
			if (0 != accessoryGrade)
				value += MAX(accessoryGrade, value * aiAccessorySocketEffectivePct[accessoryGrade] / 100);

			m_pOwner->ApplyPoint(bType, bAdd ? value : -value);
		}
	}

	if (true == CItemVnumHelper::IsRamadanMoonRing(GetVnum()) || true == CItemVnumHelper::IsHalloweenCandy(GetVnum())
		|| true == CItemVnumHelper::IsHappinessRing(GetVnum()) || true == CItemVnumHelper::IsLovePendant(GetVnum()))
	{
		// Do not anything.
	}
	else
	{
		for (int i = 0; i < ITEM_ATTRIBUTE_MAX_NUM; ++i)
		{
			if (GetAttributeType(i))
			{
				const TPlayerItemAttribute& ia = GetAttribute(i);
#ifdef __ACCE_SYSTEM__
				long sValue = ia.sValue;
				if ((GetType() == ITEM_COSTUME) && (GetSubType() == COSTUME_ACCE))
				{
					double dValue = sValue * GetSocket(ACCE_ABSORPTION_SOCKET);
					dValue = (double)dValue / 100;
					dValue = (double)dValue + .5;
					sValue = (long)dValue;
					if ((ia.sValue > 0) && (sValue <= 0))
						sValue += 1;
				}
				if (ia.bType == APPLY_SKILL)
					m_pOwner->ApplyPoint(ia.bType, bAdd ? sValue : sValue ^ 0x00800000);
				else
					m_pOwner->ApplyPoint(ia.bType, bAdd ? sValue : -sValue);
#else
				if (ia.bType == APPLY_SKILL)
					m_pOwner->ApplyPoint(ia.bType, bAdd ? ia.sValue : ia.sValue ^ 0x00800000);
				else
					m_pOwner->ApplyPoint(ia.bType, bAdd ? ia.sValue : -ia.sValue);
#endif
			}
		}
	}

	switch (m_pProto->bType)
	{
	case ITEM_PICK:
	case ITEM_ROD:
	{
		if (bAdd)
		{
			if (m_wCell == INVENTORY_MAX_NUM + WEAR_WEAPON)
				m_pOwner->SetPart(PART_WEAPON, GetVnum());
		}
		else
		{
			if (m_wCell == INVENTORY_MAX_NUM + WEAR_WEAPON)
				m_pOwner->SetPart(PART_WEAPON, 0);
		}
	}
	break;

	case ITEM_WEAPON:
	{
#ifdef __ITEM_EVOLUTION__
		m_pOwner->ApplyPoint(APPLY_ATT_GRADE_BONUS, bAdd ? GetEvolutionRealAtk() : -GetEvolutionRealAtk());
		if (GetValue(1) > 0 && GetValue(2) > 0)
			m_pOwner->ApplyPoint(APPLY_MAGIC_ATT_GRADE, bAdd ? GetEvolutionRealAtk() : -GetEvolutionRealAtk());
		if (GetEvolutionType() == WEAPON_EVOLUTION_TYPE_FIRE)
			m_pOwner->ApplyPoint(APPLY_ENCHANT_FIRE, bAdd ? GetEvolutionElementAtk() : -GetEvolutionElementAtk());
		else if (GetEvolutionType() == WEAPON_EVOLUTION_TYPE_ICE)
			m_pOwner->ApplyPoint(APPLY_ENCHANT_ICE, bAdd ? GetEvolutionElementAtk() : -GetEvolutionElementAtk());
		else if (GetEvolutionType() == WEAPON_EVOLUTION_TYPE_EARTH)
			m_pOwner->ApplyPoint(APPLY_ENCHANT_EARTH, bAdd ? GetEvolutionElementAtk() : -GetEvolutionElementAtk());
		else if (GetEvolutionType() == WEAPON_EVOLUTION_TYPE_WIND)
			m_pOwner->ApplyPoint(APPLY_ENCHANT_WIND, bAdd ? GetEvolutionElementAtk() : -GetEvolutionElementAtk());
		else if (GetEvolutionType() == WEAPON_EVOLUTION_TYPE_DARK)
			m_pOwner->ApplyPoint(APPLY_ENCHANT_DARK, bAdd ? GetEvolutionElementAtk() : -GetEvolutionElementAtk());
		else if (GetEvolutionType() == WEAPON_EVOLUTION_TYPE_ELEC)
			m_pOwner->ApplyPoint(APPLY_ENCHANT_ELECT, bAdd ? GetEvolutionElementAtk() : -GetEvolutionElementAtk());
		else if (GetEvolutionType() == WEAPON_EVOLUTION_TYPE_ALL)
			m_pOwner->ApplyPoint(APPLY_ATTBONUS_ELEMENTS, bAdd ? GetEvolutionElementAtk() : -GetEvolutionElementAtk());
		else if (GetEvolutionType() == WEAPON_EVOLUTION_TYPE_HUMAN)
			m_pOwner->ApplyPoint(APPLY_ATTBONUS_HUMAN, bAdd ? GetEvolutionElementAtk() : -GetEvolutionElementAtk());
		// news
		else if (GetEvolutionType() == WEAPON_EVOLUTION_TYPE_EX_FIRE)
			m_pOwner->ApplyPoint(APPLY_ENCHANT_FIRE, bAdd ? GetEvolutionElementAtk() : -GetEvolutionElementAtk());
		else if (GetEvolutionType() == WEAPON_EVOLUTION_TYPE_EX_ICE)
			m_pOwner->ApplyPoint(APPLY_ENCHANT_ICE, bAdd ? GetEvolutionElementAtk() : -GetEvolutionElementAtk());
		else if (GetEvolutionType() == WEAPON_EVOLUTION_TYPE_EX_EARTH)
			m_pOwner->ApplyPoint(APPLY_ENCHANT_EARTH, bAdd ? GetEvolutionElementAtk() : -GetEvolutionElementAtk());
		else if (GetEvolutionType() == WEAPON_EVOLUTION_TYPE_EX_WIND)
			m_pOwner->ApplyPoint(APPLY_ENCHANT_WIND, bAdd ? GetEvolutionElementAtk() : -GetEvolutionElementAtk());
		else if (GetEvolutionType() == WEAPON_EVOLUTION_TYPE_EX_DARK)
			m_pOwner->ApplyPoint(APPLY_ENCHANT_DARK, bAdd ? GetEvolutionElementAtk() : -GetEvolutionElementAtk());
		else if (GetEvolutionType() == WEAPON_EVOLUTION_TYPE_EX_ELEC)
			m_pOwner->ApplyPoint(APPLY_ENCHANT_ELECT, bAdd ? GetEvolutionElementAtk() : -GetEvolutionElementAtk());
		else if (GetEvolutionType() == WEAPON_EVOLUTION_TYPE_EX_ALL)
			m_pOwner->ApplyPoint(APPLY_ATTBONUS_ELEMENTS, bAdd ? GetEvolutionElementAtk() : -GetEvolutionElementAtk());
#endif

#ifdef __WEAPON_COSTUME_SYSTEM__
		if (0 != m_pOwner->GetWear(WEAR_COSTUME_WEAPON))
			break;
#endif

		if (bAdd)
		{
			if (m_wCell == INVENTORY_MAX_NUM + WEAR_WEAPON)
			{
#ifdef __ITEM_CHANGELOOK__
				DWORD dwRes = GetTransmutation() != 0 ? GetTransmutation() : GetVnum();
				m_pOwner->SetPart(PART_WEAPON, dwRes);
#else
				m_pOwner->SetPart(PART_WEAPON, GetVnum());
#endif
			}
		}
		else
		{
			if (m_wCell == INVENTORY_MAX_NUM + WEAR_WEAPON)
				m_pOwner->SetPart(PART_WEAPON, 0);
		}
	}
	break;

	case ITEM_ARMOR:
	{
#ifdef __ITEM_EVOLUTION__
		m_pOwner->ApplyPoint(APPLY_ATT_GRADE_BONUS, bAdd ? GetEvolutionRealArmorAtk() : -GetEvolutionRealArmorAtk());
		if (GetValue(1) > 0 && GetValue(2) > 0)
			m_pOwner->ApplyPoint(APPLY_MAGIC_ATT_GRADE, bAdd ? GetEvolutionRealArmorAtk() : -GetEvolutionRealArmorAtk());

		if (GetEvolutionType() == WEAPON_EVOLUTION_TYPE_FIRE)
			m_pOwner->ApplyPoint(APPLY_RESIST_FIRE, bAdd ? GetEvolutionElementAtk() : -GetEvolutionElementAtk());
		else if (GetEvolutionType() == WEAPON_EVOLUTION_TYPE_ICE)
			m_pOwner->ApplyPoint(APPLY_RESIST_ICE, bAdd ? GetEvolutionElementAtk() : -GetEvolutionElementAtk());
		else if (GetEvolutionType() == WEAPON_EVOLUTION_TYPE_EARTH)
			m_pOwner->ApplyPoint(APPLY_RESIST_EARTH, bAdd ? GetEvolutionElementAtk() : -GetEvolutionElementAtk());
		else if (GetEvolutionType() == WEAPON_EVOLUTION_TYPE_WIND)
			m_pOwner->ApplyPoint(APPLY_RESIST_WIND, bAdd ? GetEvolutionElementAtk() : -GetEvolutionElementAtk());
		else if (GetEvolutionType() == WEAPON_EVOLUTION_TYPE_DARK)
			m_pOwner->ApplyPoint(APPLY_RESIST_DARK, bAdd ? GetEvolutionElementAtk() : -GetEvolutionElementAtk());
		else if (GetEvolutionType() == WEAPON_EVOLUTION_TYPE_ELEC)
			m_pOwner->ApplyPoint(APPLY_RESIST_ELEC, bAdd ? GetEvolutionElementAtk() : -GetEvolutionElementAtk());
		else if (GetEvolutionType() == WEAPON_EVOLUTION_TYPE_ALL)
			m_pOwner->ApplyPoint(APPLY_ENCHANT_ELEMENTS, bAdd ? GetEvolutionElementAtk() : -GetEvolutionElementAtk());
		else if (GetEvolutionType() == WEAPON_EVOLUTION_TYPE_HUMAN)
			m_pOwner->ApplyPoint(APPLY_RESIST_HUMAN, bAdd ? GetEvolutionElementAtk() : -GetEvolutionElementAtk());
		// news
		else if (GetEvolutionType() == WEAPON_EVOLUTION_TYPE_EX_FIRE)
			m_pOwner->ApplyPoint(APPLY_RESIST_FIRE, bAdd ? GetEvolutionElementAtk() : -GetEvolutionElementAtk());
		else if (GetEvolutionType() == WEAPON_EVOLUTION_TYPE_EX_ICE)
			m_pOwner->ApplyPoint(APPLY_RESIST_ICE, bAdd ? GetEvolutionElementAtk() : -GetEvolutionElementAtk());
		else if (GetEvolutionType() == WEAPON_EVOLUTION_TYPE_EX_EARTH)
			m_pOwner->ApplyPoint(APPLY_RESIST_EARTH, bAdd ? GetEvolutionElementAtk() : -GetEvolutionElementAtk());
		else if (GetEvolutionType() == WEAPON_EVOLUTION_TYPE_EX_WIND)
			m_pOwner->ApplyPoint(APPLY_RESIST_WIND, bAdd ? GetEvolutionElementAtk() : -GetEvolutionElementAtk());
		else if (GetEvolutionType() == WEAPON_EVOLUTION_TYPE_EX_DARK)
			m_pOwner->ApplyPoint(APPLY_RESIST_DARK, bAdd ? GetEvolutionElementAtk() : -GetEvolutionElementAtk());
		else if (GetEvolutionType() == WEAPON_EVOLUTION_TYPE_EX_ELEC)
			m_pOwner->ApplyPoint(APPLY_RESIST_ELEC, bAdd ? GetEvolutionElementAtk() : -GetEvolutionElementAtk());
		else if (GetEvolutionType() == WEAPON_EVOLUTION_TYPE_EX_ALL)
			m_pOwner->ApplyPoint(APPLY_ENCHANT_ELEMENTS, bAdd ? GetEvolutionElementAtk() : -GetEvolutionElementAtk());
#endif

		if (0 != m_pOwner->GetWear(WEAR_COSTUME_BODY))
			break;

#ifdef __PENDANT_SYSTEM__
		if (GetSubType() == ARMOR_BODY || GetSubType() == ARMOR_HEAD || GetSubType() == ARMOR_FOOTS || GetSubType() == ARMOR_SHIELD || GetSubType() == ARMOR_PENDANT)
#else
		if (GetSubType() == ARMOR_BODY || GetSubType() == ARMOR_HEAD || GetSubType() == ARMOR_FOOTS || GetSubType() == ARMOR_SHIELD)
#endif
		{
			if (bAdd)
			{
				if (GetProto()->bSubType == ARMOR_BODY)
#ifdef __ITEM_CHANGELOOK__
				{
					DWORD dwRes = GetTransmutation() != 0 ? GetTransmutation() : GetVnum();
					m_pOwner->SetPart(PART_MAIN, dwRes);
				}
#else
					m_pOwner->SetPart(PART_MAIN, GetVnum());
#endif
			}
			else
			{
				if (GetProto()->bSubType == ARMOR_BODY)
					m_pOwner->SetPart(PART_MAIN, m_pOwner->GetOriginalPart(PART_MAIN));
			}
		}
	}
	break;

	case ITEM_COSTUME:
	{
		DWORD toSetValue = this->GetVnum();
		EParts toSetPart = PART_MAX_NUM;

		if (GetSubType() == COSTUME_BODY)
		{
			toSetPart = PART_MAIN;

			if (false == bAdd)
			{
				const CItem* pArmor = m_pOwner->GetWear(WEAR_BODY);
				toSetValue = (NULL != pArmor) ? pArmor->GetVnum() : m_pOwner->GetOriginalPart(PART_MAIN);
#ifdef __ITEM_CHANGELOOK__
				if (pArmor)
					toSetValue = pArmor->GetTransmutation() != 0 ? pArmor->GetTransmutation() : pArmor->GetVnum();
#endif
			}
#ifdef __ITEM_CHANGELOOK__
			else
				toSetValue = GetTransmutation() != 0 ? GetTransmutation() : GetVnum();
#endif
		}

		else if (GetSubType() == COSTUME_HAIR)
		{
			toSetPart = PART_HAIR;
			if (!bAdd)
				toSetValue = 0;
			else
			{
#ifdef __ITEM_CHANGELOOK__
				DWORD dwTransmutation = GetTransmutation();
				if (dwTransmutation != 0)
				{
					TItemTable* pItemTable = ITEM_MANAGER::instance().GetTable(dwTransmutation);
					toSetValue = (pItemTable != NULL) ? pItemTable->alValues[3] : GetValue(3);
				}
				else
					toSetValue = GetValue(3);
#else
				toSetValue = GetValue(3);
#endif
			}
		}

#ifdef __MOUNT_COSTUME_SYSTEM__
		else if (GetSubType() == COSTUME_MOUNT)
		{
			// not need to do a thing in here
		}
#endif

#ifdef __ACCE_SYSTEM__
		else if (GetSubType() == COSTUME_ACCE)
		{
			toSetValue = (bAdd == true) ? toSetValue : 0;
			toSetPart = PART_ACCE;
		}
#endif

#ifdef __WEAPON_COSTUME_SYSTEM__
		else if (GetSubType() == COSTUME_WEAPON)
		{
			toSetPart = PART_WEAPON;
			if (!bAdd)
			{
				const CItem* pWeapon = m_pOwner->GetWear(WEAR_WEAPON);
				toSetValue = (NULL != pWeapon) ? pWeapon->GetVnum() : m_pOwner->GetPart(PART_WEAPON);
#ifdef __ITEM_CHANGELOOK__
				if (pWeapon)
					toSetValue = pWeapon->GetTransmutation() != 0 ? pWeapon->GetTransmutation() : pWeapon->GetVnum();
#endif
			}
#ifdef __ITEM_CHANGELOOK__
			else
				toSetValue = GetTransmutation() != 0 ? GetTransmutation() : GetVnum();
#endif
		}
#endif
#ifdef __AURA_COSTUME_SYSTEM__
		else if (GetSubType() == COSTUME_AURA)
		{
			toSetValue = (bAdd == true) ? toSetValue : 0;
			toSetPart = PART_AURA;
		}
#endif
		if (PART_MAX_NUM != toSetPart)
		{
			m_pOwner->SetPart((BYTE)toSetPart, toSetValue);
			m_pOwner->UpdatePacket();
		}
	}
	break;
	case ITEM_UNIQUE:
	{
		if (0 != GetSIGVnum())
		{
			const CSpecialItemGroup* pItemGroup = ITEM_MANAGER::instance().GetSpecialItemGroup(GetSIGVnum());
			if (NULL == pItemGroup)
				break;
			DWORD dwAttrVnum = pItemGroup->GetAttrVnum(GetVnum());
			const CSpecialAttrGroup* pAttrGroup = ITEM_MANAGER::instance().GetSpecialAttrGroup(dwAttrVnum);
			if (NULL == pAttrGroup)
				break;
			for (itertype(pAttrGroup->m_vecAttrs) it = pAttrGroup->m_vecAttrs.begin(); it != pAttrGroup->m_vecAttrs.end(); it++)
			{
				m_pOwner->ApplyPoint(it->apply_type, bAdd ? it->apply_value : -it->apply_value);
			}
		}
	}
	break;

	// @duzenleme yuzukleri special_item_group uzerinden okumuyordu bu yuzden boyle bir sey yaptik artik okuyor.
	// ve yuzuk ozelliklerini proto harici special_item_group uzerindende isletiyor fakat bu ozellikler oyuncuya yansimiyor eger item_desc girilmemis ise ozelligi oyuncu bilmez.
	case ITEM_RING:
	{
		if (0 != GetSIGVnum())
		{
			const CSpecialItemGroup* pItemGroup = ITEM_MANAGER::instance().GetSpecialItemGroup(GetSIGVnum());
			if (NULL == pItemGroup)
				break;
			DWORD dwAttrVnum = pItemGroup->GetAttrVnum(GetVnum());
			const CSpecialAttrGroup* pAttrGroup = ITEM_MANAGER::instance().GetSpecialAttrGroup(dwAttrVnum);
			if (NULL == pAttrGroup)
				break;
			for (itertype(pAttrGroup->m_vecAttrs) it = pAttrGroup->m_vecAttrs.begin(); it != pAttrGroup->m_vecAttrs.end(); it++)
			{
				m_pOwner->ApplyPoint(it->apply_type, bAdd ? it->apply_value : -it->apply_value);
			}
		}
	}
	break;
	}
}

bool CItem::IsEquipable() const
{
	switch (this->GetType())
	{
	case ITEM_COSTUME:
	case ITEM_ARMOR:
	case ITEM_WEAPON:
	case ITEM_ROD:
	case ITEM_PICK:
	case ITEM_UNIQUE:
	case ITEM_DS:
	case ITEM_SPECIAL_DS:
	case ITEM_RING:
	case ITEM_BELT:
#ifdef __SHINING_ITEM_SYSTEM__
	case ITEM_SHINING:
#endif
#ifdef __CAKRA_ITEM_SYSTEM__
	case ITEM_CAKRA:
#endif
#ifdef __SEBNEM_ITEM_SYSTEM__
	case ITEM_SEBNEM:
#endif
		return true;
	}

	return false;
}

// return false on error state
bool CItem::EquipTo(LPCHARACTER ch, BYTE bWearCell)
{
	if (!ch)
	{
		sys_err("EquipTo: nil character");
		return false;
	}

	if (IsDragonSoul())
	{
		if (bWearCell < WEAR_MAX_NUM || bWearCell >= WEAR_MAX_NUM + DRAGON_SOUL_DECK_MAX_NUM * DS_SLOT_MAX)
		{
			sys_err("EquipTo: invalid dragon soul cell (this: #%d %s wearflag: %d cell: %d)", GetOriginalVnum(), GetName(), GetSubType(), bWearCell - WEAR_MAX_NUM);
			return false;
		}
	}
	else
	{
		if (bWearCell >= WEAR_MAX_NUM)
		{
			sys_err("EquipTo: invalid wear cell (this: #%d %s wearflag: %d cell: %d)", GetOriginalVnum(), GetName(), GetWearFlag(), bWearCell);
			return false;
		}
	}

	if (ch->GetWear(bWearCell))
	{
		sys_err("EquipTo: item already exist (this: #%d %s cell: %d %s)", GetOriginalVnum(), GetName(), bWearCell, ch->GetWear(bWearCell)->GetName());
		return false;
	}

	if (GetOwner())
		RemoveFromCharacter();

	ch->SetWear(bWearCell, this);

	m_pOwner = ch;
	m_bEquipped = true;
	m_wCell = INVENTORY_MAX_NUM + bWearCell;

#ifndef __MARTY_IMMUNE_FIX__
	DWORD dwImmuneFlag = 0;

	for (int i = 0; i < WEAR_MAX_NUM; ++i)
	{
		if (m_pOwner->GetWear(i))
		{
			// m_pOwner->ChatPacket(CHAT_TYPE_INFO, "unequip immuneflag(%u)", m_pOwner->GetWear(i)->m_pProto->dwImmuneFlag); // always 0
			SET_BIT(dwImmuneFlag, m_pOwner->GetWear(i)->m_pProto->dwImmuneFlag);
		}
	}

	m_pOwner->SetImmuneFlag(dwImmuneFlag);
#endif

	if (IsDragonSoul())
	{
		DSManager::instance().ActivateDragonSoul(this);
#ifdef __DS_SET_BONUS__
		ch->DragonSoul_HandleSetBonus();
#endif
	}
	else
	{
		ModifyPoints(true);
		StartUniqueExpireEvent();
		if (-1 != GetProto()->cLimitTimerBasedOnWearIndex)
			StartTimerBasedOnWearExpireEvent();

		// ACCESSORY_REFINE
		StartAccessorySocketExpireEvent();
		// END_OF_ACCESSORY_REFINE
	}

#ifdef __DRAGON_BONE_EFFECT__
	if (GetVnum() == DBONE_VNUM_1 || GetVnum() == DBONE_VNUM_3 || GetVnum() == DBONE_VNUM_5 || GetVnum() == DBONE_VNUM_7 || GetVnum() == DBONE_VNUM_9 || GetVnum() == DBONE_VNUM_11 || GetVnum() == DBONE_VNUM_13 || GetVnum() == DBONE_VNUM_15 || GetVnum() == DBONE_VNUM_17 || GetVnum() == DBONE_VNUM_19 || GetVnum() == DBONE_VNUM_21 || GetVnum() == DBONE_VNUM_23)
	{
		if (!ch->IsAffectFlag(AFFECT_DBONE_1))
			ch->AddAffect(AFFECT_DBONE_1, POINT_NONE, 0, AFF_DBONE1, INFINITE_AFFECT_DURATION, 0, false);
	}

	if (GetVnum() == DBONE_VNUM_2 || GetVnum() == DBONE_VNUM_4 || GetVnum() == DBONE_VNUM_6 || GetVnum() == DBONE_VNUM_8 || GetVnum() == DBONE_VNUM_10 || GetVnum() == DBONE_VNUM_12 || GetVnum() == DBONE_VNUM_14 || GetVnum() == DBONE_VNUM_16 || GetVnum() == DBONE_VNUM_18 || GetVnum() == DBONE_VNUM_20 || GetVnum() == DBONE_VNUM_22 || GetVnum() == DBONE_VNUM_24)
	{
		if (!ch->IsAffectFlag(AFFECT_DBONE_2))
			ch->AddAffect(AFFECT_DBONE_2, POINT_NONE, 0, AFF_DBONE2, INFINITE_AFFECT_DURATION, 0, false);
	}
#endif

#ifdef __PET_SYSTEM_PROTO__
	if (IsPetItem())
	{
		CPetSystem* petSystem = ch->GetPetSystem();
		if (petSystem)
			petSystem->Summon(GetValue(0), this, 0, false);
	}
#endif

	ch->BuffOnAttr_AddBuffsFromItem(this);

	m_pOwner->ComputeBattlePoints();
#ifdef __MOUNT_COSTUME_SYSTEM__
	if (IsMountItem())
	{
		ch->MountSummon(this);
	}
#endif
	m_pOwner->UpdatePacket();

	Save();

	return (true);
}

bool CItem::Unequip()
{
	if (!m_pOwner || GetCell() < INVENTORY_MAX_NUM)
	{
		// ITEM_OWNER_INVALID_PTR_BUG
		sys_err("%s %u m_pOwner %p, GetCell %d",
			GetName(), GetID(), get_pointer(m_pOwner), GetCell());
		// END_OF_ITEM_OWNER_INVALID_PTR_BUG
		return false;
	}

	if (this != m_pOwner->GetWear(GetCell() - INVENTORY_MAX_NUM))
	{
		sys_err("m_pOwner->GetWear() != this");
		return false;
	}

#ifdef __PET_SYSTEM_PROTO__
	if (IsPetItem())
	{
		CPetSystem* petSystem = m_pOwner->GetPetSystem();
		if (petSystem)
			petSystem->Unsummon(GetValue(0));
	}
#endif
#ifdef __MOUNT_COSTUME_SYSTEM__
	if (IsMountItem())
	{
		m_pOwner->MountUnsummon(this);
	}
#endif
	if (IsRideItem())
		ClearMountAttributeAndAffect();

#ifdef __DRAGON_BONE_EFFECT__
	if (GetVnum() == DBONE_VNUM_1 || GetVnum() == DBONE_VNUM_3 || GetVnum() == DBONE_VNUM_5 || GetVnum() == DBONE_VNUM_7 || GetVnum() == DBONE_VNUM_9 || GetVnum() == DBONE_VNUM_11 || GetVnum() == DBONE_VNUM_13 || GetVnum() == DBONE_VNUM_15 || GetVnum() == DBONE_VNUM_17 || GetVnum() == DBONE_VNUM_19 || GetVnum() == DBONE_VNUM_21 || GetVnum() == DBONE_VNUM_23) { m_pOwner->RemoveAffect(AFFECT_DBONE_1); }
	if (GetVnum() == DBONE_VNUM_2 || GetVnum() == DBONE_VNUM_4 || GetVnum() == DBONE_VNUM_6 || GetVnum() == DBONE_VNUM_8 || GetVnum() == DBONE_VNUM_10 || GetVnum() == DBONE_VNUM_12 || GetVnum() == DBONE_VNUM_14 || GetVnum() == DBONE_VNUM_16 || GetVnum() == DBONE_VNUM_18 || GetVnum() == DBONE_VNUM_20 || GetVnum() == DBONE_VNUM_22 || GetVnum() == DBONE_VNUM_24) { m_pOwner->RemoveAffect(AFFECT_DBONE_2); }
#endif

	if (IsDragonSoul())
	{
		DSManager::instance().DeactivateDragonSoul(this);
	}
	else
	{
		ModifyPoints(false);
	}

	StopUniqueExpireEvent();

	if (-1 != GetProto()->cLimitTimerBasedOnWearIndex)
		StopTimerBasedOnWearExpireEvent();

	// ACCESSORY_REFINE
	StopAccessorySocketExpireEvent();
	// END_OF_ACCESSORY_REFINE

	m_pOwner->BuffOnAttr_RemoveBuffsFromItem(this);

	m_pOwner->SetWear(GetCell() - INVENTORY_MAX_NUM, NULL);

#ifndef __MARTY_IMMUNE_FIX__
	DWORD dwImmuneFlag = 0;

	for (int i = 0; i < WEAR_MAX_NUM; ++i)
	{
		if (m_pOwner->GetWear(i))
		{
			// m_pOwner->ChatPacket(CHAT_TYPE_INFO, "unequip immuneflag(%u)", m_pOwner->GetWear(i)->m_pProto->dwImmuneFlag); // always 0
			SET_BIT(dwImmuneFlag, m_pOwner->GetWear(i)->m_pProto->dwImmuneFlag);
		}
	}

	m_pOwner->SetImmuneFlag(dwImmuneFlag);
#endif

	m_pOwner->ComputeBattlePoints();

	m_pOwner->UpdatePacket();

	m_pOwner = NULL;
	m_wCell = 0;
	m_bEquipped = false;

	return true;
}

long CItem::GetValue(DWORD idx)
{
	assert(idx < ITEM_VALUES_MAX_NUM);
	return GetProto()->alValues[idx];
}

void CItem::SetExchanging(bool bOn)
{
	m_bExchanging = bOn;
}

void CItem::Save()
{
	if (m_bSkipSave)
		return;

	ITEM_MANAGER::instance().DelayedSave(this);
}

#ifdef __GOLD_LIMIT_REWORK__
bool CItem::CreateSocket(BYTE bSlot, long long llGold)
#else
bool CItem::CreateSocket(BYTE bSlot, BYTE bGold)
#endif
{
	assert(bSlot < ITEM_SOCKET_MAX_NUM);

	if (m_alSockets[bSlot] != 0)
	{
		sys_err("Item::CreateSocket : socket already exist %s %d", GetName(), bSlot);
		return false;
	}

#ifdef __GOLD_LIMIT_REWORK__
	if (llGold)
#else
	if (bGold)
#endif
		m_alSockets[bSlot] = 2;
	else
		m_alSockets[bSlot] = 1;

	UpdatePacket();

	Save();
	return true;
}

void CItem::SetSockets(const long* c_al)
{
	thecore_memcpy(m_alSockets, c_al, sizeof(m_alSockets));
	Save();
}

void CItem::SetSocket(int i, long v, bool bLog)
{
	assert(i < ITEM_SOCKET_MAX_NUM);
	m_alSockets[i] = v;
	UpdatePacket();
	Save();
}

#ifdef __GOLD_LIMIT_REWORK__
long long CItem::GetGold()
{
	if (IS_SET(GetFlag(), ITEM_FLAG_COUNT_PER_1GOLD))
	{
		if (GetProto()->llGold == 0)
			return GetCount();
		else
			return GetCount() / GetProto()->llGold;
	}
	else
		return GetProto()->llGold;
}
#else
int CItem::GetGold()
{
	if (IS_SET(GetFlag(), ITEM_FLAG_COUNT_PER_1GOLD))
	{
		if (GetProto()->dwGold == 0)
			return GetCount();
		else
			return GetCount() / GetProto()->dwGold;
	}
	else
		return GetProto()->dwGold;
}
#endif

#ifdef __GOLD_LIMIT_REWORK__
long long CItem::GetShopBuyPrice()
{
	return GetProto()->llShopBuyPrice;
}
#else
int CItem::GetShopBuyPrice()
{
	return GetProto()->dwShopBuyPrice;
}
#endif

bool CItem::IsOwnership(LPCHARACTER ch)
{
	if (!m_pkOwnershipEvent)
		return true;

	return m_dwOwnershipPID == ch->GetPlayerID() ? true : false;
}

EVENTFUNC(ownership_event)
{
	item_event_info* info = dynamic_cast<item_event_info*>(event->info);

	if (info == NULL)
	{
		sys_err("ownership_event> <Factor> Null pointer");
		return 0;
	}

	LPITEM pkItem = info->item;

	pkItem->SetOwnershipEvent(NULL);

	TPacketGCItemOwnership p;

	p.bHeader = HEADER_GC_ITEM_OWNERSHIP;
	p.dwVID = pkItem->GetVID();
	p.szName[0] = '\0';

	pkItem->PacketAround(&p, sizeof(p));
	return 0;
}

void CItem::SetOwnershipEvent(LPEVENT pkEvent)
{
	m_pkOwnershipEvent = pkEvent;
}

void CItem::SetOwnership(LPCHARACTER ch, int iSec)
{
	if (!ch)
	{
		if (m_pkOwnershipEvent)
		{
			event_cancel(&m_pkOwnershipEvent);
			m_dwOwnershipPID = 0;

			TPacketGCItemOwnership p;

			p.bHeader = HEADER_GC_ITEM_OWNERSHIP;
			p.dwVID = m_dwVID;
			p.szName[0] = '\0';

			PacketAround(&p, sizeof(p));
		}
		return;
	}

	if (m_pkOwnershipEvent)
		return;

	if (iSec <= 10)
		iSec = 30;

	m_dwOwnershipPID = ch->GetPlayerID();

	item_event_info* info = AllocEventInfo<item_event_info>();
	strlcpy(info->szOwnerName, ch->GetName(), sizeof(info->szOwnerName));
	info->item = this;

	SetOwnershipEvent(event_create(ownership_event, info, PASSES_PER_SEC(iSec)));

	TPacketGCItemOwnership p;

	p.bHeader = HEADER_GC_ITEM_OWNERSHIP;
	p.dwVID = m_dwVID;
	strlcpy(p.szName, ch->GetName(), sizeof(p.szName));

	PacketAround(&p, sizeof(p));
}

int CItem::GetSocketCount()
{
	for (int i = 0; i < ITEM_SOCKET_MAX_NUM; i++)
	{
		if (GetSocket(i) == 0)
			return i;
	}
	return ITEM_SOCKET_MAX_NUM;
}

bool CItem::AddSocket()
{
	int count = GetSocketCount();
	if (count == ITEM_SOCKET_MAX_NUM)
		return false;
	m_alSockets[count] = 1;
	return true;
}

void CItem::AlterToSocketItem(int iSocketCount)
{
	if (iSocketCount >= ITEM_SOCKET_MAX_NUM)
	{
		sys_log(0, "Invalid Socket Count %d, set to maximum", ITEM_SOCKET_MAX_NUM);
		iSocketCount = ITEM_SOCKET_MAX_NUM;
	}

	for (int i = 0; i < iSocketCount; ++i)
		SetSocket(i, 1);
}

void CItem::AlterToMagicItem()
{
	int idx = GetAttributeSetIndex();

	if (idx < 0)
		return;

	//      Appeariance Second Third
	// Weapon 50        20     5
	// Armor  30        10     2
	// Acc    20        10     1

	int iSecondPct;
	int iThirdPct;

	switch (GetType())
	{
	case ITEM_WEAPON:
		iSecondPct = 20;
		iThirdPct = 5;
		break;

	case ITEM_ARMOR:
	case ITEM_COSTUME:
		if (GetSubType() == ARMOR_BODY)
		{
			iSecondPct = 10;
			iThirdPct = 2;
		}
		else
		{
			iSecondPct = 10;
			iThirdPct = 1;
		}
		break;

	default:
		return;
	}

	PutAttribute(aiItemMagicAttributePercentHigh);

	if (number(1, 100) <= iSecondPct)
		PutAttribute(aiItemMagicAttributePercentLow);

	if (number(1, 100) <= iThirdPct)
		PutAttribute(aiItemMagicAttributePercentLow);
}

DWORD CItem::GetRefineFromVnum()
{
	return ITEM_MANAGER::instance().GetRefineFromVnum(GetVnum());
}

int CItem::GetRefineLevel()
{
	const char* name = GetBaseName();
	char* p = const_cast<char*>(strrchr(name, '+'));

	if (!p)
		return 0;

	int	rtn = 0;
	str_to_number(rtn, p + 1);

	const char* locale_name = GetName();
	p = const_cast<char*>(strrchr(locale_name, '+'));

	if (p)
	{
		int	locale_rtn = 0;
		str_to_number(locale_rtn, p + 1);
		if (locale_rtn != rtn)
		{
			sys_err("refine_level_based_on_NAME(%d) is not equal to refine_level_based_on_LOCALE_NAME(%d).", rtn, locale_rtn);
		}
	}

	return rtn;
}

EVENTFUNC(unique_expire_event)
{
	item_event_info* info = dynamic_cast<item_event_info*>(event->info);

	if (info == NULL)
	{
		sys_err("unique_expire_event> <Factor> Null pointer");
		return 0;
	}

	LPITEM pkItem = info->item;

	if (pkItem->GetValue(2) == 0)
	{
		if (pkItem->GetSocket(ITEM_SOCKET_UNIQUE_REMAIN_TIME) <= 1)
		{
			sys_log(0, "UNIQUE_ITEM: expire %s %u", pkItem->GetName(), pkItem->GetID());
			pkItem->SetUniqueExpireEvent(NULL);
#ifdef __DRAGON_BONE_EFFECT__
			if (pkItem->GetVnum() == DBONE_VNUM_1 || pkItem->GetVnum() == DBONE_VNUM_3 || pkItem->GetVnum() == DBONE_VNUM_5 || pkItem->GetVnum() == DBONE_VNUM_7 || pkItem->GetVnum() == DBONE_VNUM_9 || pkItem->GetVnum() == DBONE_VNUM_11 || pkItem->GetVnum() == DBONE_VNUM_13 || pkItem->GetVnum() == DBONE_VNUM_15 || pkItem->GetVnum() == DBONE_VNUM_17 || pkItem->GetVnum() == DBONE_VNUM_19 || pkItem->GetVnum() == DBONE_VNUM_21 || pkItem->GetVnum() == DBONE_VNUM_23) { pkItem->GetOwner()->RemoveAffect(AFFECT_DBONE_1); }
			if (pkItem->GetVnum() == DBONE_VNUM_2 || pkItem->GetVnum() == DBONE_VNUM_4 || pkItem->GetVnum() == DBONE_VNUM_6 || pkItem->GetVnum() == DBONE_VNUM_8 || pkItem->GetVnum() == DBONE_VNUM_10 || pkItem->GetVnum() == DBONE_VNUM_12 || pkItem->GetVnum() == DBONE_VNUM_14 || pkItem->GetVnum() == DBONE_VNUM_16 || pkItem->GetVnum() == DBONE_VNUM_18 || pkItem->GetVnum() == DBONE_VNUM_20 || pkItem->GetVnum() == DBONE_VNUM_22 || pkItem->GetVnum() == DBONE_VNUM_24) { pkItem->GetOwner()->RemoveAffect(AFFECT_DBONE_2); }
#endif
			ITEM_MANAGER::instance().RemoveItem(pkItem, "UNIQUE_EXPIRE");
			return 0;
		}
		else
		{
			pkItem->SetSocket(ITEM_SOCKET_UNIQUE_REMAIN_TIME, pkItem->GetSocket(ITEM_SOCKET_UNIQUE_REMAIN_TIME) - 1);
			return PASSES_PER_SEC(60);
		}
	}
	else
	{
		time_t cur = get_global_time();

		if (pkItem->GetSocket(ITEM_SOCKET_UNIQUE_REMAIN_TIME) <= cur)
		{
			pkItem->SetUniqueExpireEvent(NULL);
#ifdef __DRAGON_BONE_EFFECT__
			if (pkItem->GetVnum() == DBONE_VNUM_1 || pkItem->GetVnum() == DBONE_VNUM_3 || pkItem->GetVnum() == DBONE_VNUM_5 || pkItem->GetVnum() == DBONE_VNUM_7 || pkItem->GetVnum() == DBONE_VNUM_9 || pkItem->GetVnum() == DBONE_VNUM_11 || pkItem->GetVnum() == DBONE_VNUM_13 || pkItem->GetVnum() == DBONE_VNUM_15 || pkItem->GetVnum() == DBONE_VNUM_17 || pkItem->GetVnum() == DBONE_VNUM_19 || pkItem->GetVnum() == DBONE_VNUM_21 || pkItem->GetVnum() == DBONE_VNUM_23) { pkItem->GetOwner()->RemoveAffect(AFFECT_DBONE_1); }
			if (pkItem->GetVnum() == DBONE_VNUM_2 || pkItem->GetVnum() == DBONE_VNUM_4 || pkItem->GetVnum() == DBONE_VNUM_6 || pkItem->GetVnum() == DBONE_VNUM_8 || pkItem->GetVnum() == DBONE_VNUM_10 || pkItem->GetVnum() == DBONE_VNUM_12 || pkItem->GetVnum() == DBONE_VNUM_14 || pkItem->GetVnum() == DBONE_VNUM_16 || pkItem->GetVnum() == DBONE_VNUM_18 || pkItem->GetVnum() == DBONE_VNUM_20 || pkItem->GetVnum() == DBONE_VNUM_22 || pkItem->GetVnum() == DBONE_VNUM_24) { pkItem->GetOwner()->RemoveAffect(AFFECT_DBONE_2); }
#endif
			ITEM_MANAGER::instance().RemoveItem(pkItem, "UNIQUE_EXPIRE");
			return 0;
		}
		else
		{
			// by rtsummit
			if (pkItem->GetSocket(ITEM_SOCKET_UNIQUE_REMAIN_TIME) - cur < 600)
				return PASSES_PER_SEC(pkItem->GetSocket(ITEM_SOCKET_UNIQUE_REMAIN_TIME) - cur);
			else
				return PASSES_PER_SEC(600);
		}
	}
}

EVENTFUNC(timer_based_on_wear_expire_event)
{
	item_event_info* info = dynamic_cast<item_event_info*>(event->info);

	if (info == NULL)
	{
		sys_err("expire_event <Factor> Null pointer");
		return 0;
	}

	LPITEM pkItem = info->item;
	int remain_time = pkItem->GetSocket(ITEM_SOCKET_REMAIN_SEC) - processing_time / passes_per_sec;
	if (remain_time <= 0)
	{
		sys_log(0, "ITEM EXPIRED : expired %s %u", pkItem->GetName(), pkItem->GetID());
		pkItem->SetTimerBasedOnWearExpireEvent(NULL);
		pkItem->SetSocket(ITEM_SOCKET_REMAIN_SEC, 0);

		if (pkItem->IsDragonSoul())
		{
			DSManager::instance().DeactivateDragonSoul(pkItem);
		}
		else
		{
			ITEM_MANAGER::instance().RemoveItem(pkItem, "TIMER_BASED_ON_WEAR_EXPIRE");
		}
		return 0;
	}
	pkItem->SetSocket(ITEM_SOCKET_REMAIN_SEC, remain_time);
	return PASSES_PER_SEC(MIN(60, remain_time));
}

void CItem::SetUniqueExpireEvent(LPEVENT pkEvent)
{
	m_pkUniqueExpireEvent = pkEvent;
}

void CItem::SetTimerBasedOnWearExpireEvent(LPEVENT pkEvent)
{
	m_pkTimerBasedOnWearExpireEvent = pkEvent;
}

EVENTFUNC(real_time_expire_event)
{
	const item_vid_event_info* info = reinterpret_cast<const item_vid_event_info*>(event->info);

	if (NULL == info)
		return 0;

	const LPITEM item = ITEM_MANAGER::instance().FindByVID(info->item_vid);

	if (NULL == item)
		return 0;

	const time_t current = get_global_time();

	if (current > item->GetSocket(0))
	{
		if (item->GetVnum() && item->IsNewMountItem()) // @duzenleme martySama adli arkadasin yaptigi birsey buralari hep kontrol etmek lazim.
			item->ClearMountAttributeAndAffect();

		ITEM_MANAGER::instance().RemoveItem(item, "REAL_TIME_EXPIRE");

		return 0;
	}

	return PASSES_PER_SEC(1);
}

void CItem::StartRealTimeExpireEvent()
{
	if (m_pkRealTimeExpireEvent)
		return;
	for (int i = 0; i < ITEM_LIMIT_MAX_NUM; i++)
	{
		if (LIMIT_REAL_TIME == GetProto()->aLimits[i].bType || LIMIT_REAL_TIME_START_FIRST_USE == GetProto()->aLimits[i].bType)
		{
			item_vid_event_info* info = AllocEventInfo<item_vid_event_info>();
			info->item_vid = GetVID();

			m_pkRealTimeExpireEvent = event_create(real_time_expire_event, info, PASSES_PER_SEC(1));

			//sys_log(0, "REAL_TIME_EXPIRE: StartRealTimeExpireEvent");

			return;
		}
	}
}

#ifdef __BEGINNER_ITEM__
bool CItem::IsRealTimeItem()
{
	if (!GetProto())
		return false;
	for (int i = 0; i < ITEM_LIMIT_MAX_NUM; i++)
	{
		if (LIMIT_REAL_TIME == GetProto()->aLimits[i].bType)
			return true;
	}
	return false;
}
#endif

void CItem::StartUniqueExpireEvent()
{
	if (GetType() != ITEM_UNIQUE)
		return;

	if (m_pkUniqueExpireEvent)
		return;

#ifdef __BEGINNER_ITEM__
	if (IsRealTimeItem())
		return;
#endif

	// HARD CODING
	if (GetVnum() == UNIQUE_ITEM_HIDE_ALIGNMENT_TITLE)
		m_pOwner->ShowAlignment(false);

	int iSec = GetSocket(ITEM_SOCKET_UNIQUE_SAVE_TIME);

	if (iSec == 0)
		iSec = 60;
	else
		iSec = MIN(iSec, 60);

	SetSocket(ITEM_SOCKET_UNIQUE_SAVE_TIME, 0);

	item_event_info* info = AllocEventInfo<item_event_info>();
	info->item = this;

	SetUniqueExpireEvent(event_create(unique_expire_event, info, PASSES_PER_SEC(iSec)));
}

void CItem::StartTimerBasedOnWearExpireEvent()
{
	if (m_pkTimerBasedOnWearExpireEvent)
		return;

#ifdef __BEGINNER_ITEM__
	if (IsRealTimeItem())
		return;
#endif

	if (-1 == GetProto()->cLimitTimerBasedOnWearIndex)
		return;

	int iSec = GetSocket(0);

	if (0 != iSec)
	{
		iSec %= 60;
		if (0 == iSec)
			iSec = 60;
	}

	item_event_info* info = AllocEventInfo<item_event_info>();
	info->item = this;

	SetTimerBasedOnWearExpireEvent(event_create(timer_based_on_wear_expire_event, info, PASSES_PER_SEC(iSec)));
}

void CItem::StopUniqueExpireEvent()
{
	if (!m_pkUniqueExpireEvent)
		return;

	if (GetValue(2) != 0)
		return;

	// HARD CODING
	if (GetVnum() == UNIQUE_ITEM_HIDE_ALIGNMENT_TITLE)
		m_pOwner->ShowAlignment(true);

	SetSocket(ITEM_SOCKET_UNIQUE_SAVE_TIME, event_time(m_pkUniqueExpireEvent) / passes_per_sec);
	event_cancel(&m_pkUniqueExpireEvent);

	ITEM_MANAGER::instance().SaveSingleItem(this);
}

void CItem::StopTimerBasedOnWearExpireEvent()
{
	if (!m_pkTimerBasedOnWearExpireEvent)
		return;

	int remain_time = GetSocket(ITEM_SOCKET_REMAIN_SEC) - event_processing_time(m_pkTimerBasedOnWearExpireEvent) / passes_per_sec;

	SetSocket(ITEM_SOCKET_REMAIN_SEC, remain_time);
	event_cancel(&m_pkTimerBasedOnWearExpireEvent);

	ITEM_MANAGER::instance().SaveSingleItem(this);
}

void CItem::ApplyAddon(int iAddonType)
{
	CItemAddonManager::instance().ApplyAddonTo(iAddonType, this);
}

int CItem::GetSpecialGroup() const
{
	return ITEM_MANAGER::instance().GetSpecialGroupFromItem(GetVnum());
}

//

//
bool CItem::IsAccessoryForSocket()
{
#ifdef __NEW_ACCESORY_ITEMS__
	return (m_pProto->bType == ITEM_ARMOR && (m_pProto->bSubType == ARMOR_WRIST || m_pProto->bSubType == ARMOR_NECK || m_pProto->bSubType == ARMOR_EAR || m_pProto->bSubType == ARMOR_HEAD || m_pProto->bSubType == ARMOR_SHIELD || m_pProto->bSubType == ARMOR_FOOTS || m_pProto->bSubType == ARMOR_PENDANT)) ||
#else
	return (m_pProto->bType == ITEM_ARMOR && (m_pProto->bSubType == ARMOR_WRIST || m_pProto->bSubType == ARMOR_NECK || m_pProto->bSubType == ARMOR_EAR)) ||
#endif // __NEW_ACCESORY_ITEMS__
		(m_pProto->bType == ITEM_BELT);
}

bool CItem::IsDeattachAccessoryForSocket()
{
	// return (m_pProto->bType == ITEM_ARMOR && (m_pProto->bSubType == ARMOR_WRIST || m_pProto->bSubType == ARMOR_NECK || m_pProto->bSubType == ARMOR_EAR)) ||
	// 	(m_pProto->bType == ITEM_BELT);
	return IsAccessoryForSocket();
}

void CItem::SetAccessorySocketGrade(int iGrade)
{
	SetSocket(0, MINMAX(0, iGrade, GetAccessorySocketMaxGrade()));

	int iDownTime = aiAccessorySocketDegradeTime[GetAccessorySocketGrade()];

	SetAccessorySocketDownGradeTime(iDownTime);
}

void CItem::SetAccessorySocketMaxGrade(int iMaxGrade)
{
	SetSocket(1, MINMAX(0, iMaxGrade, ITEM_ACCESSORY_SOCKET_MAX_NUM));
}

void CItem::SetAccessorySocketDownGradeTime(DWORD time)
{
#ifdef __PERMA_ACCESSORY__
	if (GetSocket(3) != 99)
#endif
		SetSocket(2, time);

	if (test_server && GetOwner())
		GetOwner()->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%s에서 소켓 빠질때까지 남은 시간 %d"), GetName(), time);
}

EVENTFUNC(accessory_socket_expire_event)
{
	item_vid_event_info* info = dynamic_cast<item_vid_event_info*> (event->info);

	if (info == NULL)
	{
		sys_err("accessory_socket_expire_event> <Factor> Null pointer");
		return 0;
	}

	LPITEM item = ITEM_MANAGER::instance().FindByVID(info->item_vid);

	if (item->GetAccessorySocketDownGradeTime() <= 1)
	{
	degrade:
		item->SetAccessorySocketExpireEvent(NULL);
		item->AccessorySocketDegrade();
		return 0;
	}
	else
	{
		int iTime = item->GetAccessorySocketDownGradeTime() - 60;

		if (iTime <= 1)
			goto degrade;

		item->SetAccessorySocketDownGradeTime(iTime);

		if (iTime > 60)
			return PASSES_PER_SEC(60);
		else
			return PASSES_PER_SEC(iTime);
	}
}

void CItem::StartAccessorySocketExpireEvent()
{
#ifdef __PERMA_ACCESSORY__
	if (GetSocket(3) == 99)
		return;
#endif

	if (!IsAccessoryForSocket())
		return;

	if (m_pkAccessorySocketExpireEvent)
		return;

	if (GetAccessorySocketMaxGrade() == 0)
		return;

	if (GetAccessorySocketGrade() == 0)
		return;

	int iSec = GetAccessorySocketDownGradeTime();
	SetAccessorySocketExpireEvent(NULL);

	if (iSec <= 1)
		iSec = 5;
	else
		iSec = MIN(iSec, 60);

	item_vid_event_info* info = AllocEventInfo<item_vid_event_info>();
	info->item_vid = GetVID();

	SetAccessorySocketExpireEvent(event_create(accessory_socket_expire_event, info, PASSES_PER_SEC(iSec)));
}

void CItem::StopAccessorySocketExpireEvent()
{
	if (!m_pkAccessorySocketExpireEvent)
		return;

	if (!IsAccessoryForSocket())
		return;

	int new_time = GetAccessorySocketDownGradeTime() - (60 - event_time(m_pkAccessorySocketExpireEvent) / passes_per_sec);

	event_cancel(&m_pkAccessorySocketExpireEvent);

	if (new_time <= 1)
	{
		AccessorySocketDegrade();
	}
	else
	{
		SetAccessorySocketDownGradeTime(new_time);
	}
}

bool CItem::IsRideItem()
{
	if (ITEM_UNIQUE == GetType() && UNIQUE_SPECIAL_RIDE == GetSubType())
		return true;
	if (ITEM_UNIQUE == GetType() && UNIQUE_SPECIAL_MOUNT_RIDE == GetSubType())
		return true;
#ifdef __MOUNT_COSTUME_SYSTEM__
	if (ITEM_COSTUME == GetType() && COSTUME_MOUNT == GetSubType())
		return true;
#endif
	return false;
}

bool CItem::IsRamadanRing()
{
	if (GetVnum() == UNIQUE_ITEM_RAMADAN_RING)
		return true;
	return false;
}

void CItem::ClearMountAttributeAndAffect()
{
	LPCHARACTER ch = GetOwner();
	if (!ch) // @fixme186
		return;
	ch->RemoveAffect(AFFECT_MOUNT);
	ch->RemoveAffect(AFFECT_MOUNT_BONUS);

	ch->MountVnum(0);

	ch->PointChange(POINT_ST, 0);
	ch->PointChange(POINT_DX, 0);
	ch->PointChange(POINT_HT, 0);
	ch->PointChange(POINT_IQ, 0);
}

bool CItem::IsNewMountItem()
{
	return (
		(ITEM_UNIQUE == GetType() && UNIQUE_SPECIAL_RIDE == GetSubType() && IS_SET(GetFlag(), ITEM_FLAG_QUEST_USE))
		|| (ITEM_UNIQUE == GetType() && UNIQUE_SPECIAL_MOUNT_RIDE == GetSubType() && IS_SET(GetFlag(), ITEM_FLAG_QUEST_USE))
#ifdef __MOUNT_COSTUME_SYSTEM__
		|| (ITEM_COSTUME == GetType() && COSTUME_MOUNT == GetSubType())
#endif
		); // @duzenleme martySama adli arkadasin yaptigi birsey buralari hep kontrol etmek lazim.
}

void CItem::SetAccessorySocketExpireEvent(LPEVENT pkEvent)
{
	m_pkAccessorySocketExpireEvent = pkEvent;
}

void CItem::AccessorySocketDegrade()
{
	if (GetAccessorySocketGrade() > 0)
	{
		LPCHARACTER ch = GetOwner();

		if (ch)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%s에 박혀있던 보석이 사라집니다."), GetName());
		}

		ModifyPoints(false);
		SetAccessorySocketGrade(GetAccessorySocketGrade() - 1);
		ModifyPoints(true);

		int iDownTime = aiAccessorySocketDegradeTime[GetAccessorySocketGrade()];

		if (test_server)
			iDownTime /= 60;

		SetAccessorySocketDownGradeTime(iDownTime);

		if (iDownTime)
			StartAccessorySocketExpireEvent();
	}
}

static const bool CanPutIntoRing(LPITEM ring, LPITEM item)
{
	//const DWORD vnum = item->GetVnum();
	return false;
}

struct JewelAccessoryInfo
{
	DWORD jewel;
	DWORD wrist;
	DWORD neck;
	DWORD ear;
};
const static JewelAccessoryInfo infos[] = {
	{ 50634, 14220, 16220, 17220 },
	{ 50635, 14500, 16500, 17500 },
	{ 50636, 14520, 16520, 17520 },
	{ 50637, 14540, 16540, 17540 },
	{ 50638, 14560, 16560, 17560 },
	{ 50639, 14570, 16570, 17570 },
	{ 50640, 14230, 16230, 17230 },
	{ 29001, 1790, 1830, 1750 },
	{ 29002, 1800, 1840, 1760 },
	{ 29003, 1810, 1850, 1770 },
	{ 29004, 1820, 1860, 1780 },
	{ 29009, 4580, 4590, 4570 },
	{ 29020, 8440, 8460, 8420 },
	{ 29021, 8450, 8470, 8430 },
	{ 82000, 69250, 69300, 69200 },
	{ 82002, 69260, 69310, 69210 },
	{ 82004, 69270, 69320, 69220 },
	{ 82006, 69280, 69330, 69230 },
	{ 82008, 69290, 69340, 69240 },
};
bool CItem::CanPutInto(LPITEM item)
{
	if (item->GetType() == ITEM_BELT)
		return this->GetSubType() == USE_PUT_INTO_BELT_SOCKET;

	else if (item->GetType() == ITEM_RING)
		return CanPutIntoRing(item, this);

	else if (item->GetType() != ITEM_ARMOR)
		return false;

#ifdef __NEW_ACCESORY_ITEMS__
	else if (item->GetSubType() == ARMOR_HEAD)
		return this->GetVnum() == PUT_HEAD_SOCKET_ITEM;
	else if (item->GetSubType() == ARMOR_SHIELD)
		return this->GetVnum() == PUT_SHIELD_SOCKET_ITEM;
	else if (item->GetSubType() == ARMOR_FOOTS)
		return this->GetVnum() == PUT_FOOTS_SOCKET_ITEM;
	else if (item->GetSubType() == ARMOR_PENDANT)
		return this->GetVnum() == PUT_PENDANT_SOCKET_ITEM;
#endif // __NEW_ACCESORY_ITEMS__

	DWORD vnum = item->GetVnum();

	DWORD item_type = (item->GetVnum() / 10) * 10;
	for (size_t i = 0; i < sizeof(infos) / sizeof(infos[0]); i++)
	{
		const JewelAccessoryInfo& info = infos[i];
		switch (item->GetSubType())
		{
		case ARMOR_WRIST:
			if (info.wrist == item_type)
			{
				if (info.jewel == GetVnum())
				{
					return true;
				}
				else
				{
					return false;
				}
			}
			break;
		case ARMOR_NECK:
			if (info.neck == item_type)
			{
				if (info.jewel == GetVnum())
				{
					return true;
				}
				else
				{
					return false;
				}
			}
			break;
		case ARMOR_EAR:
			if (info.ear == item_type)
			{
				if (info.jewel == GetVnum())
				{
					return true;
				}
				else
				{
					return false;
				}
			}
			break;
		}
	}
	if (item->GetSubType() == ARMOR_WRIST)
		vnum -= 14000;
	else if (item->GetSubType() == ARMOR_NECK)
		vnum -= 16000;
	else if (item->GetSubType() == ARMOR_EAR)
		vnum -= 17000;
	else
		return false;

	DWORD type = vnum / 20;

	if (type < 0 || type > 11)
	{
		type = (vnum - 170) / 20;

		if (50623 + type != GetVnum())
			return false;
		else
			return true;
	}
	else if (item->GetVnum() >= 16210 && item->GetVnum() <= 16219)
	{
		if (50625 != GetVnum())
			return false;
		else
			return true;
	}
	else if (item->GetVnum() >= 16230 && item->GetVnum() <= 16239)
	{
		if (50626 != GetVnum())
			return false;
		else
			return true;
	}

	return 50623 + type == GetVnum();
}

#ifdef __PERMA_ACCESSORY__
const static JewelAccessoryInfo infosPerma[] = {
	{ 50641, 14220, 16220, 17220 },
	{ 50642, 14500, 16500, 17500 },
	{ 50643, 14520, 16520, 17520 },
	{ 50644, 14540, 16540, 17540 },
	{ 50645, 14560, 16560, 17560 },
	{ 50646, 14570, 16570, 17570 },
	{ 50647, 14230, 16230, 17230 },
	{ 29005, 1790, 1830, 1750 },
	{ 29006, 1800, 1840, 1760 },
	{ 29007, 1810, 1850, 1770 },
	{ 29008, 1820, 1860, 1780 },
	{ 29010, 4580, 4590, 4570 },
	{ 29022, 8440, 8460, 8420 },
	{ 29023, 8450, 8470, 8430 },
	{ 82001, 69250, 69300, 69200 },
	{ 82003, 69260, 69310, 69210 },
	{ 82005, 69270, 69320, 69220 },
	{ 82007, 69280, 69330, 69230 },
	{ 82009, 69290, 69340, 69240 },
};
bool CItem::CanPutIntoPerma(LPITEM item)
{
	if (item->GetType() == ITEM_BELT)
		return this->GetSubType() == USE_PUT_INTO_BELT_SOCKET;

	else if (item->GetType() == ITEM_RING)
		return CanPutIntoRing(item, this);

	else if (item->GetType() != ITEM_ARMOR)
		return false;

#ifdef __NEW_ACCESORY_ITEMS__
	else if (item->GetSubType() == ARMOR_HEAD)
		return this->GetVnum() == PUT_HEAD_SOCKET_ITEM_PERMA;
	else if (item->GetSubType() == ARMOR_SHIELD)
		return this->GetVnum() == PUT_SHIELD_SOCKET_ITEM_PERMA;
	else if (item->GetSubType() == ARMOR_FOOTS)
		return this->GetVnum() == PUT_FOOTS_SOCKET_ITEM_PERMA;
	else if (item->GetSubType() == ARMOR_PENDANT)
		return this->GetVnum() == PUT_PENDANT_SOCKET_ITEM_PERMA;
#endif // __NEW_ACCESORY_ITEMS__

	DWORD vnum = item->GetVnum();

	DWORD item_type = (item->GetVnum() / 10) * 10;
	for (size_t i = 0; i < sizeof(infosPerma) / sizeof(infosPerma[0]); i++)
	{
		const JewelAccessoryInfo& info = infosPerma[i];
		switch (item->GetSubType())
		{
		case ARMOR_WRIST:
			if (info.wrist == item_type)
			{
				if (info.jewel == GetVnum())
				{
					return true;
				}
				else
				{
					return false;
				}
			}
			break;
		case ARMOR_NECK:
			if (info.neck == item_type)
			{
				if (info.jewel == GetVnum())
				{
					return true;
				}
				else
				{
					return false;
				}
			}
			break;
		case ARMOR_EAR:
			if (info.ear == item_type)
			{
				if (info.jewel == GetVnum())
				{
					return true;
				}
				else
				{
					return false;
				}
			}
			break;
		}
	}
	if (item->GetSubType() == ARMOR_WRIST)
		vnum -= 14000;
	else if (item->GetSubType() == ARMOR_NECK)
		vnum -= 16000;
	else if (item->GetSubType() == ARMOR_EAR)
		vnum -= 17000;
	else
		return false;

	DWORD type = vnum / 20;

	if (type < 0 || type > 11)
	{
		type = (vnum - 170) / 20;

		if (50623 + type != GetVnum())
			return false;
		else
			return true;
	}
	else if (item->GetVnum() >= 16210 && item->GetVnum() <= 16219)
	{
		if (50625 != GetVnum())
			return false;
		else
			return true;
	}
	else if (item->GetVnum() >= 16230 && item->GetVnum() <= 16239)
	{
		if (50626 != GetVnum())
			return false;
		else
			return true;
	}

	return 50623 + type == GetVnum();
}
#endif

DWORD CItem::GetPutItemVnum()
{
	DWORD item_type = (GetVnum() / 10) * 10;
	for (size_t i = 0; i < sizeof(infos) / sizeof(infos[0]); i++)
	{
		const JewelAccessoryInfo& info = infos[i];
		switch (GetSubType())
		{
		case ARMOR_WRIST:
			if (info.wrist == item_type)
			{
				return info.jewel;
			}
			break;
		case ARMOR_NECK:
			if (info.neck == item_type)
			{
				return info.jewel;
			}
			break;
		case ARMOR_EAR:
			if (info.ear == item_type)
			{
				return info.jewel;
			}
			break;
		}
	}
	return 0;
}

DWORD CItem::GetPutItemVnumPerma()
{
	DWORD item_type = (GetVnum() / 10) * 10;
	for (size_t i = 0; i < sizeof(infosPerma) / sizeof(infosPerma[0]); i++)
	{
		const JewelAccessoryInfo& info = infosPerma[i];
		switch (GetSubType())
		{
		case ARMOR_WRIST:
			if (info.wrist == item_type)
			{
				return info.jewel;
			}
			break;
		case ARMOR_NECK:
			if (info.neck == item_type)
			{
				return info.jewel;
			}
			break;
		case ARMOR_EAR:
			if (info.ear == item_type)
			{
				return info.jewel;
			}
			break;
		}
	}
	return 0;
}

bool CItem::CheckItemUseLevel(int nLevel)
{
	for (int i = 0; i < ITEM_LIMIT_MAX_NUM; ++i)
	{
		if (this->m_pProto->aLimits[i].bType == LIMIT_LEVEL)
		{
			if (this->m_pProto->aLimits[i].lValue > nLevel) return false;
			else return true;
		}
	}
	return true;
}

long CItem::FindApplyValue(BYTE bApplyType)
{
	if (m_pProto == NULL)
		return 0;

	for (int i = 0; i < ITEM_APPLY_MAX_NUM; ++i)
	{
		if (m_pProto->aApplies[i].bType == bApplyType)
			return m_pProto->aApplies[i].lValue;
	}

	return 0;
}

void CItem::CopySocketTo(LPITEM pItem)
{
	for (int i = 0; i < ITEM_SOCKET_MAX_NUM; ++i)
	{
		pItem->m_alSockets[i] = m_alSockets[i];
	}
}

int CItem::GetAccessorySocketGrade()
{
	return MINMAX(0, GetSocket(0), GetAccessorySocketMaxGrade());
}

int CItem::GetAccessorySocketMaxGrade()
{
	return MINMAX(0, GetSocket(1), ITEM_ACCESSORY_SOCKET_MAX_NUM);
}

int CItem::GetAccessorySocketDownGradeTime()
{
	return MINMAX(0, GetSocket(2), aiAccessorySocketDegradeTime[GetAccessorySocketGrade()]);
}

int CItem::GetLevelLimit()
{
	for (int i = 0; i < ITEM_LIMIT_MAX_NUM; ++i)
	{
		if (this->m_pProto->aLimits[i].bType == LIMIT_LEVEL)
		{
			return this->m_pProto->aLimits[i].lValue;
		}
	}
	return 0;
}

bool CItem::OnAfterCreatedItem()
{
	if (-1 != this->GetProto()->cLimitRealTimeFirstUseIndex)
	{
		if (0 != GetSocket(1))
		{
			StartRealTimeExpireEvent();
		}
	}

	return true;
}

bool CItem::IsDragonSoul()
{
	return GetType() == ITEM_DS;
}

#ifdef __ADDITIONAL_INVENTORY__
bool CItem::IsUpgradeItem()
{
	switch (GetVnum())
	{
		case 30003:
		case 30004:
		case 30005:
		case 30006:
		case 30007:
		case 30008:
		case 30009:
		case 30010:
		case 30011:
		case 30014:
		case 30015:
		case 30016:
		case 30017:
		case 30018:
		case 30019:
		case 30021:
		case 30022:
		case 30023:
		case 30025:
		case 30027:
		case 30028:
		case 30030:
		case 30031:
		case 30032:
		case 30033:
		case 30034:
		case 30035:
		case 30037:
		case 30038:
		case 30039:
		case 30040:
		case 30041:
		case 30042:
		case 30045:
		case 30046:
		case 30047:
		case 30048:
		case 30049:
		case 30050:
		case 30051:
		case 30052:
		case 30053:
		case 30055:
		case 30056:
		case 30057:
		case 30058:
		case 30059:
		case 30060:
		case 30061:
		case 30067:
		case 30069:
		case 30070:
		case 30071:
		case 30072:
		case 30073:
		case 30074:
		case 30075:
		case 30076:
		case 30077:
		case 30078:
		case 30079:
		case 30080:
		case 30081:
		case 30082:
		case 30083:
		case 30084:
		case 30085:
		case 30086:
		case 30087:
		case 30088:
		case 30089:
		case 30090:
		case 30091:
		case 30092:
		case 30192:
		case 30193:
		case 30194:
		case 30195:
		case 30196:
		case 30197:
		case 30198:
		case 30199:
		case 30522:
		case 30523:
		case 30524:
		case 30525:
		case 85000:
		case 70017:
		case 70018:
		case 70023:
		case 30554:
		case 30555:
		case 30557:
		case 30558:
		case 30559:
		case 80019:
		case 27992:
		case 27993:
		case 27994:
		case 70086:
		case 70087:
		case 70088:
		case 70089:
		case 70090:
		case 70091:
		case 70092:
		case 70093:
		case 70094:
		case 70095:
		case 70096:
		case 70097:
		case 811:
		case 812:
		case 813:
		case 814:
		case 815:
		case 30600:
		case 30601:
		case 30602:
		case 30603:
		case 30604:
		case 30605:
		case 30606:
		case 30607:
		case 30608:
		case 30609:
		case 30610:
		case 30611:
		case 30612:
		case 30613:
		case 30614:
		case 30615:
		case 30616:
		case 30617:
		case 30618:
		case 30619:
		case 30620:
		case 30621:
		case 30622:
		case 30623:
		case 30624:
		case 30625:
		case 30626:
		case 30627:
		case 30628:
		case 30629:
		case 30630:
		case 30639:
		case 30640:
		case 30641:
		case 30642:
		case 30643:
		case 30644:
		case 30645:
		case 30646:
		case 30647:
		case 30648:
		case 30649:
		case 30650:
		case 30651:
		case 30652:
		case 30653:
		case 30654:
		case 30655:
		case 31001:
		case 31002:
		case 31003:
		case 31004:
		case 31005:
		case 31006:
		case 31007:
		case 31008:
		case 31009:
		case 31010:
		case 31011:
		case 31012:
		case 31013:
		case 31014:
		case 31015:
		case 31016:
		case 31017:
		case 31018:
		case 31019:
		case 31020:
		case 31021:
		case 31022:
		case 31023:
		case 31024:
		case 31025:
		case 31026:
		case 31027:
		case 31028:
		case 31029:
		case 31030:
		case 31031:
		case 31032:
		case 31033:
		case 31034:
		case 31035:
		case 31036:
		case 31037:
		case 31038:
		case 31039:
		case 31040:
		case 31041:
		case 31042:
		case 31043:
		case 31073:
		case 31074:
		case 31075:
		case 31076:
		case 31077:
		case 31078:
		case 31079:
		case 31080:
		case 31081:
		case 31082:
		case 31083:
		case 31093:
		case 31094:
		case 31095:
		case 31096:
		case 31097:
		case 31098:
		case 31099:
		case 31100:
		case 31104:
		case 31105:
		case 31106:
		case 31107:
		case 31108:
		case 31109:
		case 31110:
		case 31111:
		case 31112:
		case 31113:
		case 31114:
		case 31115:
		case 31117:
		case 31118:
		case 31120:
		case 31121:
		case 31122:
		case 31123:
		case 31124:
		case 31125:
		case 31126:
		case 31127:
		case 31128:
		case 31129:
		case 31130:
		case 31131:
		case 31132:
		case 31133:
		case 31134:
		case 31135:
		case 31136:
		case 31137:
		case 31138:
		case 50067:
		case 50068:
		case 71026:
		case 71025:
		case 72308:
		case 50926:
		case 70075:
		case 50160:
		case 50161:
		case 50162:
		case 50163:
		case 50164:
		case 50165:
		case 50166:
		case 50167:
		case 50168:
		case 50169:
		case 50170:
		case 50171:
		case 50172:
		case 50173:
		case 50174:
		case 50175:
		case 50176:
		case 50177:
		case 50178:
		case 50179:
		case 30165:
		case 30166:
		case 30167:
		case 30168:
		case 30220:
		case 30221:
		case 30222:
		case 30223:
		case 30224:
		case 30225:
		case 30226:
		case 30227:
		case 30228:
		case 30251:
		case 30252:
		case 30253:
		case 90010:
		case 90011:
		case 90012:
		case 50601:
		case 50602:
		case 50603:
		case 50604:
		case 50605:
		case 50606:
		case 50607:
		case 50608:
		case 50609:
		case 50610:
		case 50611:
		case 50612:
		case 50613:
		case 50614:
		case 50615:
		case 50616:
		case 50617:
		case 50618:
		case 50619:
		case 33031:
		case 33029:
		case 33030:
		case 70035:
		case 71123:
		case 71129:
		case 71056:
		case 30550:
		case 824:
		case 825:
		case 27799:
		case 51001:
		case 70601:
		case 819:
		case 820:
		case 27621:
		case 27622:
		case 27623:
		case 27624:
		case 30526:
		case 30527:
		case 30528:
		case 30529:
		case 30530:
		case 30531:
		case 30532:
		case 30533:
		case 30534:
		case 30535:
		case 30536:
		case 30537:
		case 30538:
		case 30539:
		case 30549:
		case 30551:
		case 30552:
		case 30553:
		case 30573:
		case 30574:
		case 30575:
		case 30576:
		case 30568:
		case 30569:
		case 30570:
		case 30571:
		case 30560:
		case 30561:
		case 30562:
		case 30563:
		case 30580:
		case 30581:
		case 30582:
		case 30583:
		case 81160:
		case 81161:
		case 81162:
		case 81163:
		case 81164:
		case 81165:
		case 81166:
		case 81167:
		case 81168:
		case 81169:
		case 81170:
		case 81171:
		case 81172:
		case 81173:
		case 81174:
		case 81175:
		case 81176:
		case 81177:
		case 81178:
		case 81179:
		case 81180:
		case 30328:
		case 30564:
		case 30565:
		case 30566:
		case 30567:
		case 30572:
		case 30577:
		case 30578:
		case 30579:
		case 30584:
		case 30585:
		case 30586:
		case 31044:
		case 29011:
		case 29012:
		case 29013:
		case 29014:
		case 95011:
		case 98055:
		case 98056:
		case 98057:
		case 98058:
		case 98059:
		case 98060:
		case 98061:
		case 98062:
		case 98063:
		case 98064:
		case 98065:
		case 98066:
		case 98067:
		case 98068:
		case 98069:
		case 98070:
		case 98071:
		case 98072:
		case 98073:
		case 98074:
		case 98075:
		case 98076:
		case 98077:
		case 98078:
		case 98079:
		case 98080:
		case 98081:
		case 98082:
		case 98083:
		case 98084:
		case 98085:
		case 98086:
		case 98087:
		case 98088:
		case 98089:
		case 98090:
		case 98091:
		case 98092:
		case 98093:
		case 98094:
		case 98095:
		case 98096:
		case 98097:
		case 98098:
		case 98099:
		case 98100:
		case 98101:
		case 98102:
		case 98103:
		case 98104:
		case 98105:
		case 98106:
		case 98107:
		case 98108:
		case 98109:
		case 98110:
		case 98111:
		case 98112:
		case 98113:
		case 50180:
		case 81181:
		case 81182:
		case 81183:
		case 81184:
		case 71178:
		case 71179:
		case 71201:
		case 50151:
		case 818:
		case 51010:
		case 51011:
		case 51012:
		case 51013:
		case 51014:
		case 50255:
		case 99145:
			return true;
	}

	return false;
}

bool CItem::IsBook()
{
	if (GetType() == ITEM_SKILLBOOK)
		return true;
	
	switch (GetVnum())
	{
		case 50300:
		case 50301:
		case 50302:
		case 50303:
		case 50304:
		case 50305:
		case 50306:
		case 50307:
		case 50308:
		case 50309:
		case 50310:
		case 50311:
		case 50312:
		case 50313:
		case 50314:
		case 50315:
		case 50316:
		case 50325:
		case 50600:
		case 55010:
		case 55011:
		case 55012:
		case 55013:
		case 55014:
		case 55015:
		case 55016:
		case 55017:
		case 55018:
		case 55019:
		case 55020:
		case 55021:
		case 55022:
		case 55023:
		case 55024:
		case 55025:
		case 55026:
		case 55027:
		case 50061:
		case 50060:
		case 50514:
		case 50515:
		case 50516:
		case 50517:
		case 50518:
		case 50519:
		case 50520:
		case 50521:
		case 50522:
		case 50523:
		case 50524:
		case 50560:
		case 50561:
		case 50562:
		case 50563:
		case 50564:
		case 50565:
		case 50566:
		case 50567:
		case 50568:
		case 55034:
		case 55035:
		case 55036:
		case 55037:
		case 55038:
		case 55039:
		case 55040:
		case 50326:
		case 50327:
		case 50328:
		case 50329:
		case 55003:
		case 55004:
		case 55005:
		case 50333:
		case 50334:
		case 55210:
		case 55211:
		case 55212:
		case 55213:
		case 55214:
		case 55215:
		case 55216:
		case 50350:
		case 50351:
		case 50352:
		case 50353:
		case 50354:
		case 50355:
		case 50356:
		case 50357:
		case 50360:
		case 50361:
		case 50362:
		case 50363:
		case 50364:
		case 50365:
		case 50366:
		case 50367:
		case 50370:
		case 50371:
		case 50372:
		case 50373:
		case 50374:
		case 50375:
		case 50376:
		case 50377:
		case 50380:
		case 50381:
		case 50382:
		case 50383:
		case 50384:
		case 50385:
		case 50386:
		case 50387:
			return true;
	}

	return false;
}

bool CItem::IsStone()
{
	if (GetType() == ITEM_METIN)
		return true;

	return false;
}

bool CItem::IsFlower()
{
	switch (GetVnum())
	{
		case 50701:
		case 50702:
		case 50703:
		case 50704:
		case 50705:
		case 50706:
		case 50707:
		case 50708:
		case 50709:
		case 50710:
		case 50711:
		case 50712:
		case 50721:
		case 50722:
		case 50723:
		case 50724:
		case 50725:
		case 50726:
		case 50727:
		case 50728:
		case 50729:
		case 50730:
		case 50731:
		case 50732:
		case 19980:
			return true;
	}

	return false;
}

bool CItem::IsAttrItem()
{
	switch (GetVnum())
	{
		case 39028:
		case 39029:
		case 71084:
		case 71085:
		case 71189:
		case 71151:
		case 71152:
		case 76023:
		case 76024:
		case 76014:
		case 70024:
		case 826:
		case 827:
		case 99423:
		case 99424:
		case 99425:
		case 99426:
		case 99421:
		case 71086:
		case 71087:
		case 99422:
		case 30093:
		case 30094:
		case 30095:
		case 30096:
		case 30097:
		case 30100:
		case 30099:
		case 30098:
		case 30500:
		case 30501:
		case 30502:
		case 30503:
		case 30504:
		case 30505:
		case 30506:
		case 30507:
		case 30508:
		case 30509:
		case 30510:
		case 30511:
		case 30512:
		case 30513:
		case 30514:
		case 30515:
		case 30516:
		case 30517:
		case 30518:
		case 30519:
		case 30520:
		case 30521:
		case 30171:
		case 30172:
		case 70063:
		case 70064:
		case 30174:
		case 30173:
		case 30590:
		case 30591:
		case 30592:
		case 30593:
		case 56500:
		case 56501:
		case 56502:
		case 56503:
		case 56504:
		case 50390:
		case 50391:
		case 50392:
		case 50393:
		case 50595:
		case 30548:
		case 50596:
		case 30547:
		case 30543:
		case 30544:
		case 30541:
		case 30540:
		case 50597:
		case 30542:
		case 30545:
		case 30546:
		case 99427:
			return true;
	}
	
	return false;
}

bool CItem::IsChest()
{
	switch (GetVnum())
	{
		case 99988:
		case 99989:
		case 99990:
		case 99991:
		case 99992:
		case 99993:
		case 99994:
		case 99995:
		case 99996:
		case 99997:
		case 99998:
		case 99999:
		case 50265:
		case 50096:
		case 71150:
		case 830:
		case 831:
		case 832:
		case 833:
		case 71194:
		case 71195:
		case 71196:
		case 829:
		case 27987:
		case 33026:
		case 33027:
		case 33028:
		case 38052:
		case 38053:
		case 38054:
		case 38055:
		case 38056:
		case 38050:
		case 33033:
		case 50006:
		case 50007:
		case 50008:
		case 50009:
		case 50011:
		case 50012:
		case 50013:
		case 50014:
		case 50015:
		case 50070:
		case 50071:
		case 50072:
		case 50073:
		case 50074:
		case 50075:
		case 50076:
		case 50077:
		case 50078:
		case 50079:
		case 50080:
		case 50081:
		case 50082:
		case 50127:
		case 50128:
		case 50129:
		case 50130:
		case 50131:
		case 50132:
		case 50133:
		case 50134:
		case 50135:
		case 50136:
		case 50137:
		case 50124:
		case 50125:
		case 50126:
		case 50120:
		case 50121:
		case 50122:
		case 50123:
		case 50113:
		case 50115:
		case 50186:
		case 50182:
		case 50212:
		case 50213:
		case 50215:
		case 50249:
		case 50252:
		case 50254:
		case 50256:
		case 50257:
		case 50258:
		case 50259:
		case 50260:
		case 50266:
		case 50267:
		case 50268:
		case 50269:
		case 50270:
		case 50271:
		case 50272:
		case 50273:
		case 50274:
		case 50275:
		case 50276:
		case 50277:
		case 50278:
		case 50290:
		case 50294:
		case 50296:
		case 50297:
		case 50298:
		case 50299:
		case 50920:
		case 50921:
		case 50922:
		case 50923:
		case 50924:
		case 50925:
		case 50927:
		case 50928:
		case 50929:
		case 50930:
		case 50931:
		case 50932:
		case 50933:
		case 51501:
		case 51502:
		case 51503:
		case 51504:
		case 51505:
		case 51506:
		case 51507:
		case 51508:
		case 51509:
		case 51510:
		case 51511:
		case 51512:
		case 51513:
		case 51514:
		case 51515:
		case 51516:
		case 51517:
		case 51518:
		case 51519:
		case 51520:
		case 51521:
		case 51522:
		case 51523:
		case 51524:
		case 51525:
		case 51526:
		case 51527:
		case 51528:
		case 51529:
		case 51530:
		case 51531:
		case 51532:
		case 51533:
		case 51534:
		case 51535:
		case 51536:
		case 51537:
		case 51538:
		case 51539:
		case 51540:
		case 51541:
		case 51542:
		case 51543:
		case 51544:
		case 51545:
		case 51546:
		case 51547:
		case 51548:
		case 51549:
		case 51550:
		case 51551:
		case 51552:
		case 51553:
		case 51554:
		case 51555:
		case 51556:
		case 51557:
		case 51558:
		case 51559:
		case 51560:
		case 51561:
		case 51562:
		case 51563:
		case 51564:
		case 51565:
		case 51566:
		case 51567:
		case 51568:
		case 51569:
		case 51570:
		case 51571:
		case 51572:
		case 51573:
		case 51574:
		case 51575:
		case 51576:
		case 51577:
		case 51578:
		case 51579:
		case 51580:
		case 51581:
		case 51582:
		case 51583:
		case 51584:
		case 51585:
		case 51586:
		case 51587:
		case 51588:
		case 51589:
		case 51590:
		case 51591:
		case 51592:
		case 51593:
		case 51594:
		case 51595:
		case 51596:
		case 51597:
		case 51598:
		case 51599:
		case 51600:
		case 51601:
		case 51602:
		case 51603:
		case 51604:
		case 51605:
		case 51606:
		case 51607:
		case 51608:
		case 51609:
		case 51610:
		case 51611:
		case 51612:
		case 51613:
		case 51614:
		case 51615:
		case 51616:
		case 51617:
		case 51618:
		case 51619:
		case 51620:
		case 51621:
		case 51622:
		case 51623:
		case 51624:
		case 51625:
		case 51626:
		case 51627:
		case 51628:
		case 51629:
		case 51630:
		case 51631:
		case 51632:
		case 51633:
		case 51634:
		case 51635:
		case 51636:
		case 51637:
		case 51638:
		case 51639:
		case 51640:
		case 51641:
		case 51642:
		case 51643:
		case 51644:
		case 51645:
		case 51646:
		case 54700:
		case 54701:
		case 54702:
		case 54703:
		case 54704:
		case 54705:
		case 55007:
		case 55009:
		case 55400:
		case 55411:
		case 70009:
		case 71144:
		case 71146:
		case 71147:
		case 71159:
		case 71160:
		case 76039:
		case 76040:
		case 76044:
		case 83001:
		case 83002:
		case 83003:
		case 83004:
		case 83006:
		case 83007:
		case 83009:
		case 83010:
		case 83011:
		case 83012:
		case 83013:
		case 83014:
		case 83015:
		case 83016:
		case 83017:
		case 83018:
		case 83019:
		case 83020:
		case 83021:
		case 83028:
		case 83032:
		case 84006:
		case 84007:
		case 84008:
		case 84300:
		case 31313:
		case 31314:
		case 31315:
		case 31316:
		case 31317:
		case 31318:
		case 31319:
		case 31320:
		case 31321:
		case 31322:
		case 31323:
		case 31324:
		case 31325:
		case 31326:
		case 71503:
		case 71504:
		case 71505:
		case 71506:
		case 50109:
		case 50110:
		case 50111:
		case 50112:
		case 50114:
		case 31327:
		case 31328:
		case 31329:
		case 31330:
		case 31331:
		case 30340:
		case 30342:
		case 30343:
		case 30344:
		case 71501:
		case 71502:
		case 70079:
		case 70080:
		case 70081:
		case 70082:
		case 70083:
		case 70084:
		case 70085:
		case 78789:
		case 50037:
		case 61251:
		case 61252:
		case 61253:
		case 61254:
		case 61255:
		case 99100:
		case 99101:
		case 99102:
		case 99103:
		case 99104:
		case 99105:
		case 99106:
		case 99107:
		case 99108:
		case 99109:
		case 99110:
		case 99111:
		case 99112:
		case 99113:
		case 99114:
		case 99115:
		case 99116:
		case 99117:
		case 99118:
		case 99119:
		case 99120:
		case 99121:
		case 99122:
		case 99123:
		case 99124:
		case 99125:
		case 99126:
		case 99127:
		case 99128:
		case 99129:
		case 99130:
		case 99131:
		case 99132:
		case 99133:
		case 99134:
		case 99135:
		case 99136:
		case 99137:
		case 99138:
		case 99139:
		case 99140:
		case 99141:
		case 99142:
		case 99143:
		case 99144:
		case 99146:
		case 99147:
		case 99148:
		case 99149:
		case 99150:
		case 99151:
		case 99152:
		case 99153:
		case 99154:
		case 99155:
		case 99156:
		case 99157:
		case 99158:
		case 99159:
		case 99160:
		case 99161:
		case 99162:
		case 99163:
		case 99164:
		case 99165:
		case 99166:
		case 99167:
		case 99168:
		case 99169:
		case 99170:
		case 99171:
		case 99172:
		case 99173:
		case 99174:
		case 99175:
		case 99176:
		case 99177:
		case 99178:
		case 99179:
		case 99180:
		case 99181:
		case 99182:
		case 99183:
		case 99184:
		case 99185:
		case 99186:
		case 99187:
		case 99188:
		case 99189:
		case 99190:
		case 99191:
		case 99192:
		case 99193:
		case 99194:
		case 99195:
		case 99196:
		case 99197:
		case 99198:
		case 99199:
		case 99200:
		case 99201:
		case 99202:
		case 99203:
		case 99204:
		case 99205:
		case 99206:
		case 99207:
		case 99208:
		case 99209:
		case 99210:
		case 99211:
		case 99212:
		case 99213:
		case 99214:
		case 99215:
		case 99216:
		case 99217:
		case 99218:
		case 99219:
		case 99220:
		case 99221:
		case 99222:
		case 99223:
		case 99224:
		case 99225:
		case 99226:
		case 99227:
		case 99228:
		case 99229:
		case 99230:
		case 99231:
		case 99232:
		case 99233:
		case 99234:
		case 99235:
		case 99236:
		case 99237:
		case 99238:
		case 99239:
		case 99240:
		case 99241:
		case 99242:
		case 99243:
		case 99244:
		case 99245:
		case 99246:
		case 99247:
		case 99248:
		case 99249:
		case 99250:
		case 99251:
		case 99252:
		case 99253:
		case 99254:
		case 99255:
		case 99256:
		case 99257:
		case 99258:
		case 99259:
		case 99260:
		case 99261:
		case 99262:
		case 99263:
		case 99264:
		case 99265:
		case 99266:
		case 99267:
		case 99268:
		case 99269:
		case 99270:
		case 99271:
		case 99272:
		case 99273:
		case 99274:
		case 99275:
		case 99276:
		case 99277:
		case 99278:
		case 99279:
		case 99280:
		case 99281:
		case 99282:
		case 99283:
		case 99284:
		case 99285:
		case 99286:
		case 99287:
		case 99288:
		case 99289:
		case 99290:
		case 99291:
		case 99292:
		case 99293:
		case 99294:
		case 99295:
		case 99296:
		case 99297:
		case 99298:
		case 99299:
		case 99450:
		case 99451:
		case 99452:
		case 99453:
		case 99454:
		case 99455:
		case 99456:
		case 99457:
		case 99458:
		case 99459:
		case 99460:
		case 99461:
		case 99462:
		case 99463:
		case 99464:
		case 99465:
		case 99466:
		case 99467:
		case 99468:
		case 99469:
		case 99470:
		case 99471:
		case 99472:
		case 99473:
		case 99474:
		case 99475:
		case 99476:
		case 99477:
		case 99478:
		case 99479:
		case 99480:
		case 75760:
		case 75761:
		case 75762:
		case 75763:
		case 75764:
		case 75765:
		case 75766:
		case 75767:
		case 75768:
		case 75769:
		case 75770:
		case 75771:
		case 75772:
		case 75773:
		case 75774:
		case 75775:
		case 75776:
		case 75777:
		case 75778:
		case 75779:
		case 75780:
		case 75781:
		case 75782:
		case 75783:
		case 75784:
		case 75785:
		case 75786:
		case 75787:
		case 75788:
		case 75789:
		case 75790:
		case 75791:
		case 75792:
		case 75793:
		case 75794:
		case 75795:
		case 75796:
		case 75797:
		case 75798:
		case 75799:
		case 75800:
		case 75801:
		case 75802:
		case 75803:
		case 75804:
		case 75805:
		case 75806:
		case 75807:
		case 75808:
		case 75809:
		case 75810:
		case 75811:
		case 75812:
		case 75813:
		case 75814:
		case 75815:
		case 75816:
		case 75817:
		case 75818:
		case 75819:
		case 75820:
		case 75821:
		case 75822:
		case 75823:
		case 75824:
		case 75825:
		case 75826:
		case 75827:
		case 75828:
		case 75829:
		case 75830:
		case 75831:
		case 75832:
		case 75833:
		case 75834:
		case 75835:
		case 75836:
		case 75837:
		case 75838:
		case 75839:
		case 75840:
		case 75841:
		case 75842:
		case 75843:
		case 75844:
		case 75845:
		case 75846:
		case 75847:
		case 75848:
		case 75849:
		case 75850:
		case 99970:
		case 99971:
		case 99972:
		case 99973:
		case 99974:
		case 99975:
		case 99976:
		case 99977:
		case 99978:
		case 99979:
		case 99980:
		case 99981:
		case 99982:
		case 99983:
		case 99984:
		case 99985:
		case 99986:
		case 99987:
		case 12100:
		case 12101:
		case 12102:
		case 12103:
		case 12104:
		case 12105:
		case 12106:
		case 12107:
		case 12108:
		case 12109:
		case 12110:
		case 12111:
		case 12112:
		case 12113:
		case 12114:
		case 12115:
		case 12116:
		case 12117:
		case 12118:
		case 12119:
		case 12120:
		case 12121:
		case 12122:
		case 12123:
		case 12124:
		case 12125:
		case 12126:
		case 12127:
		case 12128:
		case 12129:
		case 12130:
		case 12131:
		case 12132:
		case 12133:
		case 12134:
		case 12135:
		case 12136:
		case 12137:
		case 12138:
		case 12139:
		case 12140:
		case 12141:
		case 12142:
		case 12143:
		case 12144:
		case 12145:
		case 12146:
		case 12147:
		case 12148:
		case 12149:
		case 12150:
		case 12151:
		case 12152:
		case 12153:
		case 12154:
		case 12155:
		case 12156:
		case 12157:
		case 12158:
		case 12159:
		case 12160:
		case 12161:
		case 12162:
		case 12163:
		case 12164:
		case 12165:
		case 12166:
		case 12167:
		case 12168:
		case 12169:
		case 12170:
		case 12171:
		case 12172:
		case 12173:
		case 12174:
		case 12175:
		case 12176:
		case 12177:
		case 12178:
		case 12179:
		case 12180:
		case 12181:
		case 12182:
		case 12183:
		case 12184:
		case 12185:
		case 12186:
		case 12187:
		case 12188:
		case 12189:
		case 12190:
		case 12191:
		case 12192:
		case 12193:
		case 12194:
		case 12195:
		case 12196:
		case 12197:
		case 12198:
		case 12199:
			return true;
	}

	return false;
}

#endif

int CItem::GiveMoreTime_Per(float fPercent)
{
	if (IsDragonSoul())
	{
		DWORD duration = DSManager::instance().GetDuration(this);
		DWORD remain_sec = GetSocket(ITEM_SOCKET_REMAIN_SEC);
		DWORD given_time = fPercent * duration / 100u;
		if (remain_sec == duration)
			return false;
		if ((given_time + remain_sec) >= duration)
		{
			SetSocket(ITEM_SOCKET_REMAIN_SEC, duration);
			return duration - remain_sec;
		}
		else
		{
			SetSocket(ITEM_SOCKET_REMAIN_SEC, given_time + remain_sec);
			return given_time;
		}
	}

	else
		return 0;
}

int CItem::GiveMoreTime_Fix(DWORD dwTime)
{
	if (IsDragonSoul())
	{
		DWORD duration = DSManager::instance().GetDuration(this);
		DWORD remain_sec = GetSocket(ITEM_SOCKET_REMAIN_SEC);
		if (remain_sec == duration)
			return false;
		if ((dwTime + remain_sec) >= duration)
		{
			SetSocket(ITEM_SOCKET_REMAIN_SEC, duration);
			return duration - remain_sec;
		}
		else
		{
			SetSocket(ITEM_SOCKET_REMAIN_SEC, dwTime + remain_sec);
			return dwTime;
		}
	}

	else
		return 0;
}

int	CItem::GetDuration()
{
	if (!GetProto())
		return -1;

	for (int i = 0; i < ITEM_LIMIT_MAX_NUM; i++)
	{
		if (LIMIT_REAL_TIME == GetProto()->aLimits[i].bType)
			return GetProto()->aLimits[i].lValue;
	}

	if (GetProto()->cLimitTimerBasedOnWearIndex >= 0)
	{
		BYTE cLTBOWI = GetProto()->cLimitTimerBasedOnWearIndex;
		return GetProto()->aLimits[cLTBOWI].lValue;
	}

	return -1;
}

bool CItem::IsSameSpecialGroup(const LPITEM item) const
{
	if (this->GetVnum() == item->GetVnum())
		return true;

	if (GetSpecialGroup() && (item->GetSpecialGroup() == GetSpecialGroup()))
		return true;

	return false;
}

#ifdef __ITEM_ENTITY_UTILITY__
const char* CItem::GetName()
{
	static char szItemName[128];
	memset(szItemName, 0, sizeof(szItemName));
	if (GetProto())
	{
		int len = 0;
		switch (GetType())
		{
		case ITEM_SKILLBOOK:
		case ITEM_SKILLFORGET:
		{
			const DWORD dwSkillVnum = (GetVnum() == ITEM_SKILLBOOK_VNUM || GetVnum() == ITEM_SKILLFORGET_VNUM) ? GetSocket(0) : 0;
			const CSkillProto* pSkill = (dwSkillVnum != 0) ? CSkillManager::instance().Get(dwSkillVnum) : NULL;
			if (pSkill)
				len = snprintf(szItemName, sizeof(szItemName), "%s", pSkill->szName);

			break;
		}
		}
		len += snprintf(szItemName + len, sizeof(szItemName) - len, (len > 0) ? " %s" : "%s", GetProto()->szLocaleName);
	}

	return szItemName;
}

std::string CItem::GetNameString()
{
	static char szItemName[128];
	memset(szItemName, 0, sizeof(szItemName));
	if (GetProto())
	{
		int len = 0;
		switch (GetType())
		{
		case ITEM_SKILLBOOK:
		case ITEM_SKILLFORGET:
		{
			const DWORD dwSkillVnum = (GetVnum() == ITEM_SKILLBOOK_VNUM || GetVnum() == ITEM_SKILLFORGET_VNUM) ? GetSocket(0) : 0;
			const CSkillProto* pSkill = (dwSkillVnum != 0) ? CSkillManager::instance().Get(dwSkillVnum) : NULL;
			if (pSkill)
				len = snprintf(szItemName, sizeof(szItemName), "%s", pSkill->szName);

			break;
		}
		}
		len += snprintf(szItemName + len, sizeof(szItemName) - len, (len > 0) ? " %s" : "%s", GetProto()->szLocaleName);
	}

	std::string returnSzItemName = szItemName;

	return returnSzItemName;
}
#endif

#ifdef __PET_SYSTEM_PROTO__
bool CItem::IsPetItem()
{
	if (GetType() == ITEM_UNIQUE && GetSubType() == USE_PET)
		return true;

	return false;
}
#endif
#ifdef __BEGINNER_ITEM__
void CItem::SetBasic(bool b)
{
	is_basic = b;
	UpdatePacket();
}
#endif

#ifdef __ITEM_EVOLUTION__
DWORD CItem::GetEvolution()
{
	return m_dwEvolution;
}
BYTE CItem::GetEvolutionType()
{
	if (m_dwEvolution > 300)
		return WEAPON_EVOLUTION_TYPE_EX_ALL;
	else if (m_dwEvolution > 280)
		return WEAPON_EVOLUTION_TYPE_EX_ELEC;
	else if (m_dwEvolution > 260)
		return WEAPON_EVOLUTION_TYPE_EX_DARK;
	else if (m_dwEvolution > 240)
		return WEAPON_EVOLUTION_TYPE_EX_WIND;
	else if (m_dwEvolution > 220)
		return WEAPON_EVOLUTION_TYPE_EX_EARTH;
	else if (m_dwEvolution > 200)
		return WEAPON_EVOLUTION_TYPE_EX_ICE;
	else if (m_dwEvolution > 180)
		return WEAPON_EVOLUTION_TYPE_EX_FIRE;
	else if (m_dwEvolution > 160)
		return WEAPON_EVOLUTION_TYPE_HUMAN;
	else if (m_dwEvolution > 140)
		return WEAPON_EVOLUTION_TYPE_ALL;
	else if (m_dwEvolution > 120)
		return WEAPON_EVOLUTION_TYPE_ELEC;
	else if (m_dwEvolution > 100)
		return WEAPON_EVOLUTION_TYPE_DARK;
	else if (m_dwEvolution > 80)
		return WEAPON_EVOLUTION_TYPE_WIND;
	else if (m_dwEvolution > 60)
		return WEAPON_EVOLUTION_TYPE_EARTH;
	else if (m_dwEvolution > 40)
		return WEAPON_EVOLUTION_TYPE_ICE;
	else if (m_dwEvolution > 20)
		return WEAPON_EVOLUTION_TYPE_FIRE;

	return WEAPON_EVOLUTION_TYPE_NONE;
}
DWORD CItem::GetRealEvolution()
{
	if (m_dwEvolution > 300)
		return m_dwEvolution - 300;
	else if (m_dwEvolution > 280)
		return m_dwEvolution - 280;
	else if (m_dwEvolution > 260)
		return m_dwEvolution - 260;
	else if (m_dwEvolution > 240)
		return m_dwEvolution - 240;
	else if (m_dwEvolution > 220)
		return m_dwEvolution - 220;
	else if (m_dwEvolution > 200)
		return m_dwEvolution - 200;
	else if (m_dwEvolution > 180)
		return m_dwEvolution - 180;
	else if (m_dwEvolution > 160)
		return m_dwEvolution - 160;
	else if (m_dwEvolution > 140)
		return m_dwEvolution - 140;
	else if (m_dwEvolution > 120)
		return m_dwEvolution - 120;
	else if (m_dwEvolution > 100)
		return m_dwEvolution - 100;
	else if (m_dwEvolution > 80)
		return m_dwEvolution - 80;
	else if (m_dwEvolution > 60)
		return m_dwEvolution - 60;
	else if (m_dwEvolution > 40)
		return m_dwEvolution - 40;
	else if (m_dwEvolution > 20)
		return m_dwEvolution - 20;

	return 0;
}
DWORD CItem::GetEvolutionRealAtk()
{
	const DWORD AtkPoints[] = { 0,20,40,60,80,100,120,140,160,180,200,220,240,260,280,300,320,340,380,420,460,500 };
	const DWORD AtkPointsEx[] = { 0,100,200,300,400,500,600,700,800,900 };
	const DWORD AtkPointsExEl[] = { 0,100,200,300,400,500,600,700,800,900,1000,1100,1200,1300,1400,1500,1600,1700,1800,1900,2000 };
	if (GetEvolutionType() == WEAPON_EVOLUTION_TYPE_ELEC)
	{
		int tempEvolutionVal = m_dwEvolution;
		tempEvolutionVal -= 120;
		return AtkPoints[tempEvolutionVal];
	}
	else if (GetEvolutionType() == WEAPON_EVOLUTION_TYPE_DARK)
	{
		int tempEvolutionVal = m_dwEvolution;
		tempEvolutionVal -= 100;
		return AtkPoints[tempEvolutionVal];
	}
	else if (GetEvolutionType() == WEAPON_EVOLUTION_TYPE_WIND)
	{
		int tempEvolutionVal = m_dwEvolution;
		tempEvolutionVal -= 80;
		return AtkPoints[tempEvolutionVal];
	}
	else if (GetEvolutionType() == WEAPON_EVOLUTION_TYPE_EARTH)
	{
		int tempEvolutionVal = m_dwEvolution;
		tempEvolutionVal -= 60;
		return AtkPoints[tempEvolutionVal];
	}
	else if (GetEvolutionType() == WEAPON_EVOLUTION_TYPE_ICE)
	{
		int tempEvolutionVal = m_dwEvolution;
		tempEvolutionVal -= 40;
		return AtkPoints[tempEvolutionVal];
	}
	else if (GetEvolutionType() == WEAPON_EVOLUTION_TYPE_FIRE)
	{
		int tempEvolutionVal = m_dwEvolution;
		tempEvolutionVal -= 20;
		return AtkPoints[tempEvolutionVal];
	}
	else if (GetEvolutionType() == WEAPON_EVOLUTION_TYPE_ALL)
	{
		int tempEvolutionVal = m_dwEvolution;
		tempEvolutionVal -= 140;
		return AtkPoints[tempEvolutionVal]*2;
	}
	else if (GetEvolutionType() == WEAPON_EVOLUTION_TYPE_HUMAN)
	{
		int tempEvolutionVal = m_dwEvolution;
		tempEvolutionVal -= 160;
		return AtkPoints[tempEvolutionVal];
	}
	else if (GetEvolutionType() == WEAPON_EVOLUTION_TYPE_EX_FIRE)
	{
		int tempEvolutionVal = m_dwEvolution;
		tempEvolutionVal -= 180;
		return AtkPointsEx[tempEvolutionVal];
	}
	else if (GetEvolutionType() == WEAPON_EVOLUTION_TYPE_EX_ICE)
	{
		int tempEvolutionVal = m_dwEvolution;
		tempEvolutionVal -= 200;
		return AtkPointsEx[tempEvolutionVal];
	}
	else if (GetEvolutionType() == WEAPON_EVOLUTION_TYPE_EX_EARTH)
	{
		int tempEvolutionVal = m_dwEvolution;
		tempEvolutionVal -= 220;
		return AtkPointsEx[tempEvolutionVal];
	}
	else if (GetEvolutionType() == WEAPON_EVOLUTION_TYPE_EX_WIND)
	{
		int tempEvolutionVal = m_dwEvolution;
		tempEvolutionVal -= 240;
		return AtkPointsEx[tempEvolutionVal];
	}
	else if (GetEvolutionType() == WEAPON_EVOLUTION_TYPE_EX_DARK)
	{
		int tempEvolutionVal = m_dwEvolution;
		tempEvolutionVal -= 260;
		return AtkPointsEx[tempEvolutionVal];
	}
	else if (GetEvolutionType() == WEAPON_EVOLUTION_TYPE_EX_ELEC)
	{
		int tempEvolutionVal = m_dwEvolution;
		tempEvolutionVal -= 280;
		return AtkPointsEx[tempEvolutionVal];
	}
	else if (GetEvolutionType() == WEAPON_EVOLUTION_TYPE_EX_ALL)
	{
		int tempEvolutionVal = m_dwEvolution;
		tempEvolutionVal -= 300;
		return AtkPointsExEl[tempEvolutionVal];
	}

	return 0;
}
#ifdef __ARMOR_EVOLUTION__
DWORD CItem::GetEvolutionArmorElementAtk()
{
	const DWORD ElementPoints[] = { 0,2,4,6,8,10,12,14,16,18,20,22,24,26,28,30,32,34,38,42,46,50 };
	const DWORD ElementPointsEx[] = { 0,5,10,15,20,25,30,35,40,45,50,55,60,65,70,75,80,85,90,100 };
	if (GetEvolutionType() == WEAPON_EVOLUTION_TYPE_ELEC)
	{
		int tempEvolutionVal = m_dwEvolution;
		tempEvolutionVal -= 120;
		return ElementPoints[tempEvolutionVal];
	}
	else if (GetEvolutionType() == WEAPON_EVOLUTION_TYPE_DARK)
	{
		int tempEvolutionVal = m_dwEvolution;
		tempEvolutionVal -= 100;
		return ElementPoints[tempEvolutionVal];
	}
	else if (GetEvolutionType() == WEAPON_EVOLUTION_TYPE_WIND)
	{
		int tempEvolutionVal = m_dwEvolution;
		tempEvolutionVal -= 80;
		return ElementPoints[tempEvolutionVal];
	}
	else if (GetEvolutionType() == WEAPON_EVOLUTION_TYPE_EARTH)
	{
		int tempEvolutionVal = m_dwEvolution;
		tempEvolutionVal -= 60;
		return ElementPoints[tempEvolutionVal];
	}
	else if (GetEvolutionType() == WEAPON_EVOLUTION_TYPE_ICE)
	{
		int tempEvolutionVal = m_dwEvolution;
		tempEvolutionVal -= 40;
		return ElementPoints[tempEvolutionVal];
	}
	else if (GetEvolutionType() == WEAPON_EVOLUTION_TYPE_FIRE)
	{
		int tempEvolutionVal = m_dwEvolution;
		tempEvolutionVal -= 20;
		return ElementPoints[tempEvolutionVal];
	}
	else if (GetEvolutionType() == WEAPON_EVOLUTION_TYPE_ALL)
	{
		int tempEvolutionVal = m_dwEvolution;
		tempEvolutionVal -= 140;
		return ElementPoints[tempEvolutionVal];
	}
	else if (GetEvolutionType() == WEAPON_EVOLUTION_TYPE_HUMAN)
	{
		int tempEvolutionVal = m_dwEvolution;
		tempEvolutionVal -= 160;
		return ElementPoints[tempEvolutionVal];
	}
	else if (GetEvolutionType() == WEAPON_EVOLUTION_TYPE_EX_FIRE)
	{
		int tempEvolutionVal = m_dwEvolution;
		tempEvolutionVal -= 180;
		return ElementPointsEx[tempEvolutionVal];
	}
	else if (GetEvolutionType() == WEAPON_EVOLUTION_TYPE_EX_ICE)
	{
		int tempEvolutionVal = m_dwEvolution;
		tempEvolutionVal -= 200;
		return ElementPointsEx[tempEvolutionVal];
	}
	else if (GetEvolutionType() == WEAPON_EVOLUTION_TYPE_EX_EARTH)
	{
		int tempEvolutionVal = m_dwEvolution;
		tempEvolutionVal -= 220;
		return ElementPointsEx[tempEvolutionVal];
	}
	else if (GetEvolutionType() == WEAPON_EVOLUTION_TYPE_EX_WIND)
	{
		int tempEvolutionVal = m_dwEvolution;
		tempEvolutionVal -= 240;
		return ElementPointsEx[tempEvolutionVal];
	}
	else if (GetEvolutionType() == WEAPON_EVOLUTION_TYPE_EX_DARK)
	{
		int tempEvolutionVal = m_dwEvolution;
		tempEvolutionVal -= 260;
		return ElementPointsEx[tempEvolutionVal];
	}
	else if (GetEvolutionType() == WEAPON_EVOLUTION_TYPE_EX_ELEC)
	{
		int tempEvolutionVal = m_dwEvolution;
		tempEvolutionVal -= 280;
		return ElementPointsEx[tempEvolutionVal];
	}
	else if (GetEvolutionType() == WEAPON_EVOLUTION_TYPE_EX_ALL)
	{
		int tempEvolutionVal = m_dwEvolution;
		tempEvolutionVal -= 300;
		return ElementPointsEx[tempEvolutionVal];
	}

	return 0;
}

DWORD CItem::GetEvolutionRealArmorAtk()
{
	const DWORD AtkPoints[] = { 0,20,40,60,80,100,120,140,160,180,200,220,240,260,280,300,320,340,380,420,460,500 };
	const DWORD AtkPointsEx[] = { 0,100,200,300,400,500,600,700,800,900 };
	const DWORD AtkPointsExEl[] = { 0,100,200,300,400,500,600,700,800,900,1000,1100,1200,1300,1400,1500,1600,1700,1800,1900,2000 };

	if (GetEvolutionType() == WEAPON_EVOLUTION_TYPE_ELEC)
	{
		int tempEvolutionVal = m_dwEvolution;
		tempEvolutionVal -= 120;
		return AtkPoints[tempEvolutionVal];
	}
	else if (GetEvolutionType() == WEAPON_EVOLUTION_TYPE_DARK)
	{
		int tempEvolutionVal = m_dwEvolution;
		tempEvolutionVal -= 100;
		return AtkPoints[tempEvolutionVal];
	}
	else if (GetEvolutionType() == WEAPON_EVOLUTION_TYPE_WIND)
	{
		int tempEvolutionVal = m_dwEvolution;
		tempEvolutionVal -= 80;
		return AtkPoints[tempEvolutionVal];
	}
	else if (GetEvolutionType() == WEAPON_EVOLUTION_TYPE_EARTH)
	{
		int tempEvolutionVal = m_dwEvolution;
		tempEvolutionVal -= 60;
		return AtkPoints[tempEvolutionVal];
	}
	else if (GetEvolutionType() == WEAPON_EVOLUTION_TYPE_ICE)
	{
		int tempEvolutionVal = m_dwEvolution;
		tempEvolutionVal -= 40;
		return AtkPoints[tempEvolutionVal];
	}
	else if (GetEvolutionType() == WEAPON_EVOLUTION_TYPE_FIRE)
	{
		int tempEvolutionVal = m_dwEvolution;
		tempEvolutionVal -= 20;
		return AtkPoints[tempEvolutionVal];
	}
	else if (GetEvolutionType() == WEAPON_EVOLUTION_TYPE_ALL)
	{
		int tempEvolutionVal = m_dwEvolution;
		tempEvolutionVal -= 140;
		return AtkPoints[tempEvolutionVal]*2;
	}
	else if (GetEvolutionType() == WEAPON_EVOLUTION_TYPE_HUMAN)
	{
		int tempEvolutionVal = m_dwEvolution;
		tempEvolutionVal -= 160;
		return AtkPoints[tempEvolutionVal];
	}
	else if (GetEvolutionType() == WEAPON_EVOLUTION_TYPE_EX_FIRE)
	{
		int tempEvolutionVal = m_dwEvolution;
		tempEvolutionVal -= 180;
		return AtkPointsEx[tempEvolutionVal];
	}
	else if (GetEvolutionType() == WEAPON_EVOLUTION_TYPE_EX_ICE)
	{
		int tempEvolutionVal = m_dwEvolution;
		tempEvolutionVal -= 200;
		return AtkPointsEx[tempEvolutionVal];
	}
	else if (GetEvolutionType() == WEAPON_EVOLUTION_TYPE_EX_EARTH)
	{
		int tempEvolutionVal = m_dwEvolution;
		tempEvolutionVal -= 220;
		return AtkPointsEx[tempEvolutionVal];
	}
	else if (GetEvolutionType() == WEAPON_EVOLUTION_TYPE_EX_WIND)
	{
		int tempEvolutionVal = m_dwEvolution;
		tempEvolutionVal -= 240;
		return AtkPointsEx[tempEvolutionVal];
	}
	else if (GetEvolutionType() == WEAPON_EVOLUTION_TYPE_EX_DARK)
	{
		int tempEvolutionVal = m_dwEvolution;
		tempEvolutionVal -= 260;
		return AtkPointsEx[tempEvolutionVal];
	}
	else if (GetEvolutionType() == WEAPON_EVOLUTION_TYPE_EX_ELEC)
	{
		int tempEvolutionVal = m_dwEvolution;
		tempEvolutionVal -= 280;
		return AtkPointsEx[tempEvolutionVal];
	}
	else if (GetEvolutionType() == WEAPON_EVOLUTION_TYPE_EX_ALL)
	{
		int tempEvolutionVal = m_dwEvolution;
		tempEvolutionVal -= 300;
		return AtkPointsExEl[tempEvolutionVal];
	}

	return 0;
}
#endif
DWORD CItem::GetEvolutionElementAtk()
{
	const DWORD ElementPoints[] = { 0,2,4,6,8,10,12,14,16,18,20,22,24,26,28,30,32,34,38,42,46,50 };
	const DWORD ElementPointsEx[] = { 0,5,10,15,20,25,30,35,40,45,50,55,60,65,70,75,80,85,90,100 };
	if (GetEvolutionType() == WEAPON_EVOLUTION_TYPE_ELEC)
	{
		int tempEvolutionVal = m_dwEvolution;
		tempEvolutionVal -= 120;
		return ElementPoints[tempEvolutionVal];
	}
	else if (GetEvolutionType() == WEAPON_EVOLUTION_TYPE_DARK)
	{
		int tempEvolutionVal = m_dwEvolution;
		tempEvolutionVal -= 100;
		return ElementPoints[tempEvolutionVal];
	}
	else if (GetEvolutionType() == WEAPON_EVOLUTION_TYPE_WIND)
	{
		int tempEvolutionVal = m_dwEvolution;
		tempEvolutionVal -= 80;
		return ElementPoints[tempEvolutionVal];
	}
	else if (GetEvolutionType() == WEAPON_EVOLUTION_TYPE_EARTH)
	{
		int tempEvolutionVal = m_dwEvolution;
		tempEvolutionVal -= 60;
		return ElementPoints[tempEvolutionVal];
	}
	else if (GetEvolutionType() == WEAPON_EVOLUTION_TYPE_ICE)
	{
		int tempEvolutionVal = m_dwEvolution;
		tempEvolutionVal -= 40;
		return ElementPoints[tempEvolutionVal];
	}
	else if (GetEvolutionType() == WEAPON_EVOLUTION_TYPE_FIRE)
	{
		int tempEvolutionVal = m_dwEvolution;
		tempEvolutionVal -= 20;
		return ElementPoints[tempEvolutionVal];
	}
	else if (GetEvolutionType() == WEAPON_EVOLUTION_TYPE_ALL)
	{
		int tempEvolutionVal = m_dwEvolution;
		tempEvolutionVal -= 140;
		return ElementPoints[tempEvolutionVal]*2;
	}
	else if (GetEvolutionType() == WEAPON_EVOLUTION_TYPE_HUMAN)
	{
		int tempEvolutionVal = m_dwEvolution;
		tempEvolutionVal -= 160;
		return ElementPoints[tempEvolutionVal];
	}
	else if (GetEvolutionType() == WEAPON_EVOLUTION_TYPE_EX_FIRE)
	{
		int tempEvolutionVal = m_dwEvolution;
		tempEvolutionVal -= 180;
		return ElementPointsEx[tempEvolutionVal];
	}
	else if (GetEvolutionType() == WEAPON_EVOLUTION_TYPE_EX_ICE)
	{
		int tempEvolutionVal = m_dwEvolution;
		tempEvolutionVal -= 200;
		return ElementPointsEx[tempEvolutionVal];
	}
	else if (GetEvolutionType() == WEAPON_EVOLUTION_TYPE_EX_EARTH)
	{
		int tempEvolutionVal = m_dwEvolution;
		tempEvolutionVal -= 220;
		return ElementPointsEx[tempEvolutionVal];
	}
	else if (GetEvolutionType() == WEAPON_EVOLUTION_TYPE_EX_WIND)
	{
		int tempEvolutionVal = m_dwEvolution;
		tempEvolutionVal -= 240;
		return ElementPointsEx[tempEvolutionVal];
	}
	else if (GetEvolutionType() == WEAPON_EVOLUTION_TYPE_EX_DARK)
	{
		int tempEvolutionVal = m_dwEvolution;
		tempEvolutionVal -= 260;
		return ElementPointsEx[tempEvolutionVal];
	}
	else if (GetEvolutionType() == WEAPON_EVOLUTION_TYPE_EX_ELEC)
	{
		int tempEvolutionVal = m_dwEvolution;
		tempEvolutionVal -= 280;
		return ElementPointsEx[tempEvolutionVal];
	}
	else if (GetEvolutionType() == WEAPON_EVOLUTION_TYPE_EX_ALL)
	{
		int tempEvolutionVal = m_dwEvolution;
		tempEvolutionVal -= 300;
		return ElementPointsEx[tempEvolutionVal];
	}

	return 0;
}
void CItem::SetEvolution(DWORD evolution)
{
	m_dwEvolution = evolution;
	UpdatePacket();
	Save();
}
#endif

#ifdef __BOSS_SCROLL__
bool CItem::IsBossScroll()
{
	switch (m_dwVnum)
	{
	case 71036:
	case 71037:
	case 71038:
	case 71039:
	case 71040:
	case 71041:
	case 71042:
	case 71043:
	case 71057:
	case 71058:
	case 71059:
	case 71060:
	case 71061:
	case 71062:
	case 71063:
	case 71064:
	case 71065:
	case 71066:
	case 71067:
		return true;
	}

	return false;
}
#endif

#ifdef __MOUNT_COSTUME_SYSTEM__
bool CItem::IsMountItem()
{
	if (GetType() == ITEM_COSTUME && GetSubType() == COSTUME_MOUNT)
		return true;
	return false;
}
#endif

#ifdef __DSS_RECHARGE_ITEM__
int CItem::GiveMoreTime_Extend(DWORD dwTime)
{
	if (IsDragonSoul())
	{
		//DWORD duration = DSManager::instance().GetDuration(this);
		long remain_sec = GetSocket(ITEM_SOCKET_REMAIN_SEC);

		long MIN_INFINITE_DURATION = 100 * 24 * 60 * 60; // 100d
		long MAX_INFINITE_DURATION = 60 * 365 * 24 * 60 * 60; // 60y

		if (remain_sec >= MIN_INFINITE_DURATION)
			return 0;

		/*if (dwTime < 86400)
			dwTime = 86400;*/

		if ((dwTime + remain_sec) >= MIN_INFINITE_DURATION)
			SetSocket(ITEM_SOCKET_REMAIN_SEC, MAX_INFINITE_DURATION);
		else
			SetSocket(ITEM_SOCKET_REMAIN_SEC, dwTime + remain_sec);

		return dwTime;
	}
	else
		return 0;
}
#endif

#ifdef __PENDANT_SYSTEM__
DWORD CItem::GetElement()
{
	DWORD returnValue = 0;
	if (FindApplyValue(APPLY_ENCHANT_FIRE) > 0)
		SET_BIT(returnValue, RACE_FLAG_ATT_FIRE);
	else if (FindApplyValue(APPLY_ENCHANT_ICE) > 0)
		SET_BIT(returnValue, RACE_FLAG_ATT_ICE);
	else if (FindApplyValue(APPLY_ENCHANT_EARTH) > 0)
		SET_BIT(returnValue, RACE_FLAG_ATT_EARTH);
	else if (FindApplyValue(APPLY_ENCHANT_DARK) > 0)
		SET_BIT(returnValue, RACE_FLAG_ATT_DARK);
	else if (FindApplyValue(APPLY_ENCHANT_WIND) > 0)
		SET_BIT(returnValue, RACE_FLAG_ATT_WIND);
	else if (FindApplyValue(APPLY_ENCHANT_ELECT) > 0)
		SET_BIT(returnValue, RACE_FLAG_ATT_ELEC);

	return returnValue;
}
#endif

bool CItem::IsTilsimable()
{
	switch (GetVnum())
	{
		case 50390:
		case 50391:
		case 50392:
		case 50393:
			return false;
	}

	return true;
}