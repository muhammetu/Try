#include "stdafx.h"

#include <stack>

#include "utils.h"
#include "config.h"
#include "char.h"
#include "char_manager.h"
#include "item_manager.h"
#include "desc.h"
#include "desc_client.h"
#include "desc_manager.h"
#include "packet.h"
#include "affect.h"
#include "skill.h"
#include "start_position.h"
#include "mob_manager.h"
#include "db.h"
#include "log.h"
#include "vector.h"
#include "buffer_manager.h"
#include "questmanager.h"
#include "fishing.h"
#include "party.h"
#ifdef __SCP1453_EXTENSIONS__
#include "dungeon_new.h"
#else
#include "dungeon.h"
#endif
#include "refine.h"
#include "unique_item.h"
#include "war_map.h"
#include "blend_item.h"

#include "safebox.h"
#include "shop.h"

#include "pvp.h"
#include "../../common/VnumHelper.h"
#include "DragonSoul.h"
#include "buff_on_attributes.h"
#include "belt_inventory_helper.h"

#ifdef __PET_SYSTEM_PROTO__
#include "PetSystem.h"
#endif

#ifdef __GROWTH_PET_SYSTEM__
#include "New_PetSystem.h"
#endif

#ifdef __GROWTH_MOUNT_SYSTEM__
#include "New_MountSystem.h"
#endif

#ifdef __SWITCHBOT__
#include "new_switchbot.h"
#endif
#ifdef __BATTLE_PASS_SYSTEM__
#include "battle_pass.h"
#endif

#include "game_events.h"

const int ITEM_BROKEN_METIN_VNUM = 28960;

const BYTE g_aBuffOnAttrPoints[] = { POINT_ENERGY, POINT_COSTUME_ATTR_BONUS };

struct FFindStone
{
	std::map<DWORD, LPCHARACTER> m_mapStone;

	void operator()(LPENTITY pEnt)
	{
		if (pEnt->IsType(ENTITY_CHARACTER) == true)
		{
			LPCHARACTER pChar = (LPCHARACTER)pEnt;

			if (pChar->IsStone() == true)
			{
				m_mapStone[(DWORD)pChar->GetVID()] = pChar;
			}
		}
	}
};

bool IS_BOTARYABLE_ZONE(int nMapIndex)
{
	switch (nMapIndex)
	{
	case 1:
	case 3:
	case 21:
	case 23:
	case 41:
	case 43:
		return true;
	}

	return false;
}

bool IS_DUNGEON_ZONE(int nMapIndex)
{
	switch (nMapIndex)
	{
	case 59:
	case 66:
	case 216:
	case 362:
	case 79:
	case 351:
	case 352:
	case 58:
	case 212:
	case 47:
	case 55:
	case 37:
	case 52:
	case 39:
	case 10:
	case 11:
	case 34:
	case 30:
	case 20:
	case 54:
	case 27:
	case 31:
	case 56:
	case 9:
	case 29:
	case 32:
	case 18:
	case 35:
	case 38:
	case 28:
	case 75:
		return true;
	}

	return false;
}

static bool FN_check_item_socket(LPITEM item)
{
	for (int i = 0; i < ITEM_SOCKET_MAX_NUM; ++i)
	{
		if (item->GetSocket(i) != item->GetProto()->alSockets[i])
			return false;
	}

	return true;
}

static void FN_copy_item_socket(LPITEM dest, LPITEM src)
{
	for (int i = 0; i < ITEM_SOCKET_MAX_NUM; ++i)
	{
		dest->SetSocket(i, src->GetSocket(i));
	}
}
static bool FN_check_item_sex(LPCHARACTER ch, LPITEM item)
{
	if (IS_SET(item->GetAntiFlag(), ITEM_ANTIFLAG_MALE))
	{
		if (SEX_MALE == GET_SEX(ch))
			return false;
	}

	if (IS_SET(item->GetAntiFlag(), ITEM_ANTIFLAG_FEMALE))
	{
		if (SEX_FEMALE == GET_SEX(ch))
			return false;
	}

	return true;
}

/////////////////////////////////////////////////////////////////////////////
// ITEM HANDLING
/////////////////////////////////////////////////////////////////////////////
bool CHARACTER::CanHandleItem(bool bSkipCheckRefine, bool bSkipObserver)
{
	if (!bSkipObserver)
		if (m_bIsObserver)
			return false;

	if (GetMyShop())
		return false;

	if (!bSkipCheckRefine)
		if (m_bUnderRefine)
			return false;

	if (IsCubeOpen() || NULL != DragonSoul_RefineWindow_GetOpener())
		return false;

	if (IsWarping())
		return false;

#ifdef __ACCE_SYSTEM__
	if ((m_bAcceCombination) || (m_bAcceAbsorption))
		return false;
#endif

#ifdef __ITEM_CHANGELOOK__
	if (m_bChangeLook)
		return false;
#endif

	return true;
}

LPITEM CHARACTER::GetInventoryItem(WORD wCell) const
{
	return GetItem(TItemPos(INVENTORY, wCell));
}

LPITEM CHARACTER::GetDragonSoulInventoryItem(WORD wCell) const
{
	return GetItem(TItemPos(DRAGON_SOUL_INVENTORY, wCell));
}

#ifdef __ADDITIONAL_INVENTORY__
LPITEM CHARACTER::GetUpgradeInventoryItem(WORD wCell) const
{
	return GetItem(TItemPos(UPGRADE_INVENTORY, wCell));
}
LPITEM CHARACTER::GetBookInventoryItem(WORD wCell) const
{
	return GetItem(TItemPos(BOOK_INVENTORY, wCell));
}
LPITEM CHARACTER::GetStoneInventoryItem(WORD wCell) const
{
	return GetItem(TItemPos(STONE_INVENTORY, wCell));
}
LPITEM CHARACTER::GetFlowerInventoryItem(WORD wCell) const
{
	return GetItem(TItemPos(FLOWER_INVENTORY, wCell));
}
LPITEM CHARACTER::GetAttrInventoryItem(WORD wCell) const
{
	return GetItem(TItemPos(ATTR_INVENTORY, wCell));
}
LPITEM CHARACTER::GetChestInventoryItem(WORD wCell) const
{
	return GetItem(TItemPos(CHEST_INVENTORY, wCell));
}
#endif

LPITEM CHARACTER::GetItem(TItemPos Cell) const
{
	if (!m_PlayerSlots)
		return NULL;
	if (!IsValidItemPosition(Cell))
		return NULL;
	WORD wCell = Cell.cell;
	BYTE window_type = Cell.window_type;
	switch (window_type)
	{
	case INVENTORY:
	case EQUIPMENT:
		if (wCell >= INVENTORY_AND_EQUIP_SLOT_MAX)
		{
			sys_err("CHARACTER::GetInventoryItem: invalid item cell %d", wCell);
			return NULL;
		}
		return m_PlayerSlots->pItems[wCell];
	case DRAGON_SOUL_INVENTORY:
		if (wCell >= DRAGON_SOUL_INVENTORY_MAX_NUM)
		{
			sys_err("CHARACTER::GetInventoryItem: invalid DS item cell %d", wCell);
			return NULL;
		}
		return m_PlayerSlots->pDSItems[wCell];
#ifdef __ADDITIONAL_INVENTORY__
	case UPGRADE_INVENTORY:
		if (wCell >= SPECIAL_INVENTORY_MAX_NUM)
		{
			sys_err("CHARACTER::GetInventoryItem: invalid SSU item cell %d", wCell);
			return NULL;
		}
		return m_PlayerSlots->pSSUItems[wCell];
	case BOOK_INVENTORY:
		if (wCell >= SPECIAL_INVENTORY_MAX_NUM)
		{
			sys_err("CHARACTER::GetInventoryItem: invalid SSB item cell %d", wCell);
			return NULL;
		}
		return m_PlayerSlots->pSSBItems[wCell];
	case STONE_INVENTORY:
		if (wCell >= SPECIAL_INVENTORY_MAX_NUM)
		{
			sys_err("CHARACTER::GetInventoryItem: invalid SSS item cell %d", wCell);
			return NULL;
		}
		return m_PlayerSlots->pSSSItems[wCell];
	case FLOWER_INVENTORY:
		if (wCell >= SPECIAL_INVENTORY_MAX_NUM)
		{
			sys_err("CHARACTER::GetInventoryItem: invalid SSF item cell %d", wCell);
			return NULL;
		}
		return m_PlayerSlots->pSSFItems[wCell];
	case ATTR_INVENTORY:
		if (wCell >= SPECIAL_INVENTORY_MAX_NUM)
		{
			sys_err("CHARACTER::GetInventoryItem: invalid SSA item cell %d", wCell);
			return NULL;
		}
		return m_PlayerSlots->pSSAItems[wCell];
	case CHEST_INVENTORY:
		if (wCell >= SPECIAL_INVENTORY_MAX_NUM)
		{
			sys_err("CHARACTER::GetInventoryItem: invalid SSC item cell %d", wCell);
			return NULL;
		}
		return m_PlayerSlots->pSSCItems[wCell];
#endif

#ifdef __SWITCHBOT__
	case SWITCHBOT:
		if (wCell >= SWITCHBOT_SLOT_COUNT)
		{
			sys_err("CHARACTER::GetInventoryItem: invalid switchbot item cell %d", wCell);
			return NULL;
		}
		return m_PlayerSlots->pSwitchbotItems[wCell];
#endif

	default:
		return NULL;
	}
	return NULL;
}

#ifdef __HIGHLIGHT_ITEM__
void CHARACTER::SetItem(TItemPos Cell, LPITEM pItem, bool bWereMine, const char* file, int line)
#else
void CHARACTER::SetItem(TItemPos Cell, LPITEM pItem)
#endif
{
	if (!m_PlayerSlots)
		return;
	WORD wCell = Cell.cell;
	BYTE window_type = Cell.window_type;
	if ((unsigned long)((CItem*)pItem) == 0xff || (unsigned long)((CItem*)pItem) == 0xffffffff)
	{
		sys_err("!!! FATAL ERROR !!! item == 0xff (char: %s cell: %u)", GetName(), wCell);
		core_dump();
		return;
	}

	if (pItem && pItem->GetOwner())
	{
		assert(!"GetOwner exist");
		return;
	}

	if (wCell == WORD_MAX)
	{
		sys_err("CHARACTER::SetItem: WORD_MAX cell %s %d", file, line);
		return;
	}

	LPITEM pOld = nullptr;

	switch (window_type)
	{
	case INVENTORY:
	case EQUIPMENT:
	{
		if (wCell >= INVENTORY_AND_EQUIP_SLOT_MAX)
		{
			sys_err("CHARACTER::SetItem: invalid item cell %d", wCell);
			return;
		}

		if ((pOld = m_PlayerSlots->pItems[wCell]))
		{
			if (wCell < INVENTORY_MAX_NUM)
			{
				for (int i = 0; i < pOld->GetSize(); ++i)
				{
					int p = wCell + (i * 5);

					if (p >= INVENTORY_MAX_NUM)
						continue;

					if (m_PlayerSlots->pItems[p] && m_PlayerSlots->pItems[p] != pOld)
						continue;

					m_PlayerSlots->bItemGrid[p] = 0;
				}
			}
			else
				m_PlayerSlots->bItemGrid[wCell] = 0;
		}

		if (pItem)
		{
			if (wCell < INVENTORY_MAX_NUM)
			{
				for (int i = 0; i < pItem->GetSize(); ++i)
				{
					int p = wCell + (i * 5);

					if (p >= INVENTORY_MAX_NUM)
						continue;

					m_PlayerSlots->bItemGrid[p] = wCell + 1;
				}
			}
			else
				m_PlayerSlots->bItemGrid[wCell] = wCell + 1;
		}

		m_PlayerSlots->pItems[wCell] = pItem;
	}
	break;

	case DRAGON_SOUL_INVENTORY:
	{
		if ((pOld = m_PlayerSlots->pDSItems[wCell]))
		{
			if (wCell < DRAGON_SOUL_INVENTORY_MAX_NUM)
			{
				for (int i = 0; i < pOld->GetSize(); ++i)
				{
					int p = wCell + (i * DRAGON_SOUL_BOX_COLUMN_NUM);

					if (p >= DRAGON_SOUL_INVENTORY_MAX_NUM)
						continue;

					if (m_PlayerSlots->pDSItems[p] && m_PlayerSlots->pDSItems[p] != pOld)
						continue;

					m_PlayerSlots->wDSItemGrid[p] = 0;
				}
			}
			else
				m_PlayerSlots->wDSItemGrid[wCell] = 0;
		}

		if (pItem)
		{
			if (wCell >= DRAGON_SOUL_INVENTORY_MAX_NUM)
			{
				sys_err("CHARACTER::SetItem: invalid DS item cell %d", wCell);
				return;
			}

			if (wCell < DRAGON_SOUL_INVENTORY_MAX_NUM)
			{
				for (int i = 0; i < pItem->GetSize(); ++i)
				{
					int p = wCell + (i * DRAGON_SOUL_BOX_COLUMN_NUM);

					if (p >= DRAGON_SOUL_INVENTORY_MAX_NUM)
						continue;

					m_PlayerSlots->wDSItemGrid[p] = wCell + 1;
				}
			}
			else
				m_PlayerSlots->wDSItemGrid[wCell] = wCell + 1;
		}

		m_PlayerSlots->pDSItems[wCell] = pItem;
	}
	break;

#ifdef __ADDITIONAL_INVENTORY__
	case UPGRADE_INVENTORY:
	{
		if ((pOld = m_PlayerSlots->pSSUItems[wCell]))
		{
			if (wCell < SPECIAL_INVENTORY_MAX_NUM)
			{
				for (int i = 0; i < pOld->GetSize(); ++i)
				{
					int p = wCell + (i * 5);
					if (p >= SPECIAL_INVENTORY_MAX_NUM)
						continue;
					if (m_PlayerSlots->pSSUItems[p] && m_PlayerSlots->pSSUItems[p] != pOld)
						continue;
					m_PlayerSlots->wSSUItemGrid[p] = 0;
				}
			}
			else
				m_PlayerSlots->wSSUItemGrid[wCell] = 0;
		}
		if (pItem)
		{
			if (wCell >= SPECIAL_INVENTORY_MAX_NUM)
			{
				sys_err("CHARACTER::SetItem: invalid SSU item cell %d", wCell);
				return;
			}
			if (wCell < SPECIAL_INVENTORY_MAX_NUM)
			{
				for (int i = 0; i < pItem->GetSize(); ++i)
				{
					int p = wCell + (i * 5);
					if (p >= SPECIAL_INVENTORY_MAX_NUM)
						continue;
					m_PlayerSlots->wSSUItemGrid[p] = wCell + 1;
				}
			}
			else
				m_PlayerSlots->wSSUItemGrid[wCell] = wCell + 1;
		}
		m_PlayerSlots->pSSUItems[wCell] = pItem;
	}
	break;
	case BOOK_INVENTORY:
	{
		if ((pOld = m_PlayerSlots->pSSBItems[wCell]))
		{
			if (wCell < SPECIAL_INVENTORY_MAX_NUM)
			{
				for (int i = 0; i < pOld->GetSize(); ++i)
				{
					int p = wCell + (i * 5);
					if (p >= SPECIAL_INVENTORY_MAX_NUM)
						continue;
					if (m_PlayerSlots->pSSBItems[p] && m_PlayerSlots->pSSBItems[p] != pOld)
						continue;
					m_PlayerSlots->wSSBItemGrid[p] = 0;
				}
			}
			else
				m_PlayerSlots->wSSBItemGrid[wCell] = 0;
		}
		if (pItem)
		{
			if (wCell >= SPECIAL_INVENTORY_MAX_NUM)
			{
				sys_err("CHARACTER::SetItem: invalid SSB item cell %d", wCell);
				return;
			}
			if (wCell < SPECIAL_INVENTORY_MAX_NUM)
			{
				for (int i = 0; i < pItem->GetSize(); ++i)
				{
					int p = wCell + (i * 5);
					if (p >= SPECIAL_INVENTORY_MAX_NUM)
						continue;
					m_PlayerSlots->wSSBItemGrid[p] = wCell + 1;
				}
			}
			else
				m_PlayerSlots->wSSBItemGrid[wCell] = wCell + 1;
		}
		m_PlayerSlots->pSSBItems[wCell] = pItem;
	}
	break;
	case STONE_INVENTORY:
	{
		if ((pOld = m_PlayerSlots->pSSSItems[wCell]))
		{
			if (wCell < SPECIAL_INVENTORY_MAX_NUM)
			{
				for (int i = 0; i < pOld->GetSize(); ++i)
				{
					int p = wCell + (i * 5);
					if (p >= SPECIAL_INVENTORY_MAX_NUM)
						continue;
					if (m_PlayerSlots->pSSSItems[p] && m_PlayerSlots->pSSSItems[p] != pOld)
						continue;
					m_PlayerSlots->wSSSItemGrid[p] = 0;
				}
			}
			else
				m_PlayerSlots->wSSSItemGrid[wCell] = 0;
		}
		if (pItem)
		{
			if (wCell >= SPECIAL_INVENTORY_MAX_NUM)
			{
				sys_err("CHARACTER::SetItem: invalid SSB item cell %d", wCell);
				return;
			}
			if (wCell < SPECIAL_INVENTORY_MAX_NUM)
			{
				for (int i = 0; i < pItem->GetSize(); ++i)
				{
					int p = wCell + (i * 5);
					if (p >= SPECIAL_INVENTORY_MAX_NUM)
						continue;
					m_PlayerSlots->wSSSItemGrid[p] = wCell + 1;
				}
			}
			else
				m_PlayerSlots->wSSSItemGrid[wCell] = wCell + 1;
		}
		m_PlayerSlots->pSSSItems[wCell] = pItem;
	}
	break;

	case FLOWER_INVENTORY:
	{
		if ((pOld = m_PlayerSlots->pSSFItems[wCell]))
		{
			if (wCell < SPECIAL_INVENTORY_MAX_NUM)
			{
				for (int i = 0; i < pOld->GetSize(); ++i)
				{
					int p = wCell + (i * 5);
					if (p >= SPECIAL_INVENTORY_MAX_NUM)
						continue;
					if (m_PlayerSlots->pSSFItems[p] && m_PlayerSlots->pSSFItems[p] != pOld)
						continue;
					m_PlayerSlots->wSSFItemGrid[p] = 0;
				}
			}
			else
				m_PlayerSlots->wSSFItemGrid[wCell] = 0;
		}
		if (pItem)
		{
			if (wCell >= SPECIAL_INVENTORY_MAX_NUM)
			{
				sys_err("CHARACTER::SetItem: invalid SSF item cell %d", wCell);
				return;
			}
			if (wCell < SPECIAL_INVENTORY_MAX_NUM)
			{
				for (int i = 0; i < pItem->GetSize(); ++i)
				{
					int p = wCell + (i * 5);
					if (p >= SPECIAL_INVENTORY_MAX_NUM)
						continue;
					m_PlayerSlots->wSSFItemGrid[p] = wCell + 1;
				}
			}
			else
				m_PlayerSlots->wSSFItemGrid[wCell] = wCell + 1;
		}
		m_PlayerSlots->pSSFItems[wCell] = pItem;
	}
	break;

	case ATTR_INVENTORY:
	{
		if ((pOld = m_PlayerSlots->pSSAItems[wCell]))
		{
			if (wCell < SPECIAL_INVENTORY_MAX_NUM)
			{
				for (int i = 0; i < pOld->GetSize(); ++i)
				{
					int p = wCell + (i * 5);
					if (p >= SPECIAL_INVENTORY_MAX_NUM)
						continue;
					if (m_PlayerSlots->pSSAItems[p] && m_PlayerSlots->pSSAItems[p] != pOld)
						continue;
					m_PlayerSlots->wSSAItemGrid[p] = 0;
				}
			}
			else
				m_PlayerSlots->wSSAItemGrid[wCell] = 0;
		}
		if (pItem)
		{
			if (wCell >= SPECIAL_INVENTORY_MAX_NUM)
			{
				sys_err("CHARACTER::SetItem: invalid SSA item cell %d", wCell);
				return;
			}
			if (wCell < SPECIAL_INVENTORY_MAX_NUM)
			{
				for (int i = 0; i < pItem->GetSize(); ++i)
				{
					int p = wCell + (i * 5);
					if (p >= SPECIAL_INVENTORY_MAX_NUM)
						continue;
					m_PlayerSlots->wSSAItemGrid[p] = wCell + 1;
				}
			}
			else
				m_PlayerSlots->wSSAItemGrid[wCell] = wCell + 1;
		}
		m_PlayerSlots->pSSAItems[wCell] = pItem;
	}
	break;
	case CHEST_INVENTORY:
	{
		if ((pOld = m_PlayerSlots->pSSCItems[wCell]))
		{
			if (wCell < SPECIAL_INVENTORY_MAX_NUM)
			{
				for (int i = 0; i < pOld->GetSize(); ++i)
				{
					int p = wCell + (i * 5);
					if (p >= SPECIAL_INVENTORY_MAX_NUM)
						continue;
					if (m_PlayerSlots->pSSCItems[p] && m_PlayerSlots->pSSCItems[p] != pOld)
						continue;
					m_PlayerSlots->wSSCItemGrid[p] = 0;
				}
			}
			else
				m_PlayerSlots->wSSCItemGrid[wCell] = 0;
		}
		if (pItem)
		{
			if (wCell >= SPECIAL_INVENTORY_MAX_NUM)
			{
				sys_err("CHARACTER::SetItem: invalid SSC item cell %d", wCell);
				return;
			}
			if (wCell < SPECIAL_INVENTORY_MAX_NUM)
			{
				for (int i = 0; i < pItem->GetSize(); ++i)
				{
					int p = wCell + (i * 5);
					if (p >= SPECIAL_INVENTORY_MAX_NUM)
						continue;
					m_PlayerSlots->wSSCItemGrid[p] = wCell + 1;
				}
			}
			else
				m_PlayerSlots->wSSCItemGrid[wCell] = wCell + 1;
		}
		m_PlayerSlots->pSSCItems[wCell] = pItem;
	}
	break;
#endif

#ifdef __SWITCHBOT__
	case SWITCHBOT:
	{
		if (pItem && (pOld = m_PlayerSlots->pSwitchbotItems[wCell]))
		{
			return;
		}

		if (wCell >= SWITCHBOT_SLOT_COUNT)
		{
			sys_err("CHARACTER::SetItem: invalid switchbot item cell %d", wCell);
			return;
		}

		if (pItem)
			CSwitchbotManager::Instance().RegisterItem(GetPlayerID(), pItem->GetID(), wCell);
		else
			CSwitchbotManager::Instance().UnregisterItem(GetPlayerID(), wCell);

		m_PlayerSlots->pSwitchbotItems[wCell] = pItem;
	}
	break;
#endif

	default:
		sys_err("Invalid Inventory type %d", window_type);
		return;
	}

	if (pOld && pItem)
	{
		sys_err("old item bulundu! %u %u %s %d - %d %d - %u", pOld->GetID(), GetPlayerID(), file, line, window_type, wCell, pItem->GetID());
	}

	if (GetDesc())
	{
		if (pItem)
		{
			TPacketGCItemSet pack;
			pack.header = HEADER_GC_ITEM_SET;
			pack.Cell = Cell;

			pack.count = pItem->GetCount();
			pack.vnum = pItem->GetVnum();
			pack.flags = pItem->GetFlag();
			pack.anti_flags = pItem->GetAntiFlag();

			thecore_memcpy(pack.alSockets, pItem->GetSockets(), sizeof(pack.alSockets));
			thecore_memcpy(pack.aAttr, pItem->GetAttributes(), sizeof(pack.aAttr));

#ifdef __HIGHLIGHT_ITEM__
			pack.highlight = !bWereMine;
#else
			pack.highlight = (Cell.window_type == DRAGON_SOUL_INVENTORY);
#endif
#ifdef __ITEM_EVOLUTION__
			pack.evolution = pItem->GetEvolution();
#endif
#ifdef __BEGINNER_ITEM__
			pack.is_basic = pItem->IsBasicItem();
#endif
#ifdef __ITEM_CHANGELOOK__
			pack.transmutation = pItem->GetTransmutation();
#endif
			GetDesc()->Packet(&pack, sizeof(TPacketGCItemSet));
		}
		else
		{
			TPacketGCItemDelDeprecated pack;
			pack.header = HEADER_GC_ITEM_DEL;
			pack.Cell = Cell;
			pack.count = 0;
			pack.vnum = 0;
			memset(pack.alSockets, 0, sizeof(pack.alSockets));
			memset(pack.aAttr, 0, sizeof(pack.aAttr));
#ifdef __ITEM_EVOLUTION__
			pack.evolution = 0;
#endif
#ifdef __BEGINNER_ITEM__
			pack.is_basic = false;
#endif
#ifdef __ITEM_CHANGELOOK__
			pack.transmutation = 0;
#endif
			GetDesc()->Packet(&pack, sizeof(TPacketGCItemDelDeprecated));
		}
	}

	if (pItem)
	{
		pItem->SetCell(this, wCell);
		switch (window_type)
		{
		case INVENTORY:
		case EQUIPMENT:
			if ((wCell < INVENTORY_MAX_NUM) || (BELT_INVENTORY_SLOT_START <= wCell && BELT_INVENTORY_SLOT_END > wCell))
				pItem->SetWindow(INVENTORY);
			else
				pItem->SetWindow(EQUIPMENT);
			break;
		case DRAGON_SOUL_INVENTORY:
			pItem->SetWindow(DRAGON_SOUL_INVENTORY);
			break;
#ifdef __ADDITIONAL_INVENTORY__
		case UPGRADE_INVENTORY:
			pItem->SetWindow(UPGRADE_INVENTORY);
			break;
		case BOOK_INVENTORY:
			pItem->SetWindow(BOOK_INVENTORY);
			break;
		case STONE_INVENTORY:
			pItem->SetWindow(STONE_INVENTORY);
			break;
		case FLOWER_INVENTORY:
			pItem->SetWindow(FLOWER_INVENTORY);
			break;
		case ATTR_INVENTORY:
			pItem->SetWindow(ATTR_INVENTORY);
			break;
		case CHEST_INVENTORY:
			pItem->SetWindow(CHEST_INVENTORY);
			break;
#endif
#ifdef __SWITCHBOT__
		case SWITCHBOT:
			pItem->SetWindow(SWITCHBOT);
			break;
#endif
		}
	}
}

LPITEM CHARACTER::GetWear(BYTE bCell) const
{
	if (!m_PlayerSlots)
		return NULL;

	if (bCell >= WEAR_MAX_NUM + DRAGON_SOUL_DECK_MAX_NUM * DS_SLOT_MAX)
	{
		sys_err("CHARACTER::GetWear: invalid wear cell %d", bCell);
		return NULL;
	}

	return m_PlayerSlots->pItems[INVENTORY_MAX_NUM + bCell];
}

void CHARACTER::SetWear(BYTE bCell, LPITEM item)
{
	if (bCell >= WEAR_MAX_NUM + DRAGON_SOUL_DECK_MAX_NUM * DS_SLOT_MAX)
	{
		sys_err("CHARACTER::SetItem: invalid item cell %d", bCell);
		return;
	}

	__SET_ITEM(TItemPos(INVENTORY, INVENTORY_MAX_NUM + bCell), item);
}

void CHARACTER::ClearItem()
{
	int		i;
	LPITEM	item;

	for (i = 0; i < INVENTORY_AND_EQUIP_SLOT_MAX; ++i)
	{
		if ((item = GetInventoryItem(i)))
		{
			item->SetSkipSave(true);
			ITEM_MANAGER::instance().FlushDelayedSave(item);

			item->RemoveFromCharacter();
			M2_DESTROY_ITEM(item);

			SyncQuickslot(QUICKSLOT_TYPE_ITEM, i, 255);
		}
	}
	for (i = 0; i < DRAGON_SOUL_INVENTORY_MAX_NUM; ++i)
	{
		if ((item = GetItem(TItemPos(DRAGON_SOUL_INVENTORY, i))))
		{
			item->SetSkipSave(true);
			ITEM_MANAGER::instance().FlushDelayedSave(item);

			item->RemoveFromCharacter();
			M2_DESTROY_ITEM(item);
		}
	}
#ifdef __ADDITIONAL_INVENTORY__
	for (i = 0; i < SPECIAL_INVENTORY_MAX_NUM; ++i)
	{
		if ((item = GetItem(TItemPos(UPGRADE_INVENTORY, i))))
		{
			item->SetSkipSave(true);
			ITEM_MANAGER::instance().FlushDelayedSave(item);
			item->RemoveFromCharacter();
			M2_DESTROY_ITEM(item);
		}
	}
	for (i = 0; i < SPECIAL_INVENTORY_MAX_NUM; ++i)
	{
		if ((item = GetItem(TItemPos(BOOK_INVENTORY, i))))
		{
			item->SetSkipSave(true);
			ITEM_MANAGER::instance().FlushDelayedSave(item);
			item->RemoveFromCharacter();
			M2_DESTROY_ITEM(item);
		}
	}
	for (i = 0; i < SPECIAL_INVENTORY_MAX_NUM; ++i)
	{
		if ((item = GetItem(TItemPos(STONE_INVENTORY, i))))
		{
			item->SetSkipSave(true);
			ITEM_MANAGER::instance().FlushDelayedSave(item);
			item->RemoveFromCharacter();
			M2_DESTROY_ITEM(item);
		}
	}
	for (i = 0; i < SPECIAL_INVENTORY_MAX_NUM; ++i)
	{
		if ((item = GetItem(TItemPos(FLOWER_INVENTORY, i))))
		{
			item->SetSkipSave(true);
			ITEM_MANAGER::instance().FlushDelayedSave(item);
			item->RemoveFromCharacter();
			M2_DESTROY_ITEM(item);
		}
	}
	for (i = 0; i < SPECIAL_INVENTORY_MAX_NUM; ++i)
	{
		if ((item = GetItem(TItemPos(ATTR_INVENTORY, i))))
		{
			item->SetSkipSave(true);
			ITEM_MANAGER::instance().FlushDelayedSave(item);
			item->RemoveFromCharacter();
			M2_DESTROY_ITEM(item);
		}
	}
	for (i = 0; i < SPECIAL_INVENTORY_MAX_NUM; ++i)
	{
		if ((item = GetItem(TItemPos(CHEST_INVENTORY, i))))
		{
			item->SetSkipSave(true);
			ITEM_MANAGER::instance().FlushDelayedSave(item);
			item->RemoveFromCharacter();
			M2_DESTROY_ITEM(item);
		}
	}
#endif
#ifdef __SWITCHBOT__
	for (i = 0; i < SWITCHBOT_SLOT_COUNT; ++i)
	{
		if ((item = GetItem(TItemPos(SWITCHBOT, i))))
		{
			item->SetSkipSave(true);
			ITEM_MANAGER::instance().FlushDelayedSave(item);

			item->RemoveFromCharacter();
			M2_DESTROY_ITEM(item);
		}
	}
#endif
}

bool CHARACTER::IsEmptyItemGrid(TItemPos Cell, BYTE bSize, int iExceptionCell) const
{
	if (!m_PlayerSlots)
		return false;

	switch (Cell.window_type)
	{
	case INVENTORY:
	{
		WORD bCell = Cell.cell;

		++iExceptionCell;

		if (Cell.IsBeltInventoryPosition())
		{
			LPITEM beltItem = GetWear(WEAR_BELT);

			if (NULL == beltItem)
				return false;

			if (false == CBeltInventoryHelper::IsAvailableCell(bCell - BELT_INVENTORY_SLOT_START, beltItem->GetValue(0)))
				return false;

			if (m_PlayerSlots->bItemGrid[bCell])
			{
				if (m_PlayerSlots->bItemGrid[bCell] == iExceptionCell)
					return true;

				return false;
			}

			if (bSize == 1)
				return true;
		}
		else if (bCell >= INVENTORY_MAX_NUM)
			return false;

		if (m_PlayerSlots->bItemGrid[bCell])
		{
			if (m_PlayerSlots->bItemGrid[bCell] == iExceptionCell)
			{
				if (bSize == 1)
					return true;

				int j = 1;
				BYTE bPage = bCell / (INVENTORY_MAX_NUM / INVENTORY_PAGE_COUNT);

				do
				{
					BYTE p = bCell + (5 * j);

					if (p >= INVENTORY_MAX_NUM)
						return false;

					if (p / (INVENTORY_MAX_NUM / INVENTORY_PAGE_COUNT) != bPage)
						return false;

					if (m_PlayerSlots->bItemGrid[p])
						if (m_PlayerSlots->bItemGrid[p] != iExceptionCell)
							return false;
				} while (++j < bSize);

				return true;
			}
			else
				return false;
		}

		if (1 == bSize)
			return true;
		else
		{
			int j = 1;
			BYTE bPage = bCell / (INVENTORY_MAX_NUM / INVENTORY_PAGE_COUNT);

			do
			{
				BYTE p = bCell + (5 * j);

				if (p >= INVENTORY_MAX_NUM)
					return false;

				if (p / (INVENTORY_MAX_NUM / INVENTORY_PAGE_COUNT) != bPage)
					return false;

				if (m_PlayerSlots->bItemGrid[p])
					if (m_PlayerSlots->bItemGrid[p] != iExceptionCell)
						return false;
			} while (++j < bSize);

			return true;
		}
	}
	break;
	case DRAGON_SOUL_INVENTORY:
	{
		WORD wCell = Cell.cell;
		if (wCell >= DRAGON_SOUL_INVENTORY_MAX_NUM)
			return false;

		iExceptionCell++;

		if (m_PlayerSlots->wDSItemGrid[wCell])
		{
			if (m_PlayerSlots->wDSItemGrid[wCell] == iExceptionCell)
			{
				if (bSize == 1)
					return true;

				int j = 1;

				do
				{
					int p = wCell + (DRAGON_SOUL_BOX_COLUMN_NUM * j);

					if (p >= DRAGON_SOUL_INVENTORY_MAX_NUM)
						return false;

					if (m_PlayerSlots->wDSItemGrid[p])
						if (m_PlayerSlots->wDSItemGrid[p] != iExceptionCell)
							return false;
				} while (++j < bSize);

				return true;
			}
			else
				return false;
		}

		if (1 == bSize)
			return true;
		else
		{
			int j = 1;

			do
			{
				int p = wCell + (DRAGON_SOUL_BOX_COLUMN_NUM * j);

				if (p >= DRAGON_SOUL_INVENTORY_MAX_NUM)
					return false;

				if (m_PlayerSlots->wDSItemGrid[p])
					if (m_PlayerSlots->wDSItemGrid[p] != iExceptionCell)
						return false;
			} while (++j < bSize);

			return true;
		}
	}
	break;
#ifdef __ADDITIONAL_INVENTORY__
	case UPGRADE_INVENTORY:
	{
		WORD wCell = Cell.cell;
		if (wCell >= SPECIAL_INVENTORY_MAX_NUM)
			return false;
		iExceptionCell++;
		if (m_PlayerSlots->wSSUItemGrid[wCell])
		{
			if (m_PlayerSlots->wSSUItemGrid[wCell] == iExceptionCell)
			{
				if (bSize == 1)
					return true;
				int j = 1;
				do
				{
					int p = wCell + (5 * j);
					if (p >= SPECIAL_INVENTORY_MAX_NUM)
						return false;
					if (m_PlayerSlots->wSSUItemGrid[p])
						if (m_PlayerSlots->wSSUItemGrid[p] != iExceptionCell)
							return false;
				} while (++j < bSize);
				return true;
			}
			else
				return false;
		}
		if (1 == bSize)
			return true;
		else
		{
			int j = 1;
			do
			{
				int p = wCell + (5 * j);
				if (p >= SPECIAL_INVENTORY_MAX_NUM)
					return false;
				if (m_PlayerSlots->wSSUItemGrid[p]) // old bItemGrid
					if (m_PlayerSlots->wSSUItemGrid[p] != iExceptionCell)
						return false;
			} while (++j < bSize);
			return true;
		}
	}
	break;
	case BOOK_INVENTORY:
	{
		WORD wCell = Cell.cell;
		if (wCell >= SPECIAL_INVENTORY_MAX_NUM)
			return false;
		iExceptionCell++;
		if (m_PlayerSlots->wSSBItemGrid[wCell])
		{
			if (m_PlayerSlots->wSSBItemGrid[wCell] == iExceptionCell)
			{
				if (bSize == 1)
					return true;
				int j = 1;
				do
				{
					int p = wCell + (5 * j);
					if (p >= SPECIAL_INVENTORY_MAX_NUM)
						return false;
					if (m_PlayerSlots->wSSBItemGrid[p])
						if (m_PlayerSlots->wSSBItemGrid[p] != iExceptionCell)
							return false;
				} while (++j < bSize);
				return true;
			}
			else
				return false;
		}
		if (1 == bSize)
			return true;
		else
		{
			int j = 1;
			do
			{
				int p = wCell + (5 * j);
				if (p >= SPECIAL_INVENTORY_MAX_NUM)
					return false;
				if (m_PlayerSlots->wSSBItemGrid[p]) // old bItemGrid
					if (m_PlayerSlots->wSSBItemGrid[p] != iExceptionCell)
						return false;
			} while (++j < bSize);
			return true;
		}
	}
	break;
	case STONE_INVENTORY:
	{
		WORD wCell = Cell.cell;
		if (wCell >= SPECIAL_INVENTORY_MAX_NUM)
			return false;
		iExceptionCell++;
		if (m_PlayerSlots->wSSSItemGrid[wCell])
		{
			if (m_PlayerSlots->wSSSItemGrid[wCell] == iExceptionCell)
			{
				if (bSize == 1)
					return true;
				int j = 1;
				do
				{
					int p = wCell + (5 * j);
					if (p >= SPECIAL_INVENTORY_MAX_NUM)
						return false;
					if (m_PlayerSlots->wSSSItemGrid[p])
						if (m_PlayerSlots->wSSSItemGrid[p] != iExceptionCell)
							return false;
				} while (++j < bSize);
				return true;
			}
			else
				return false;
		}
		if (1 == bSize)
			return true;
		else
		{
			int j = 1;
			do
			{
				int p = wCell + (5 * j);
				if (p >= SPECIAL_INVENTORY_MAX_NUM)
					return false;
				if (m_PlayerSlots->wSSSItemGrid[p]) // old bItemGrid
					if (m_PlayerSlots->wSSSItemGrid[p] != iExceptionCell)
						return false;
			} while (++j < bSize);
			return true;
		}
	}
	break;

	case FLOWER_INVENTORY:
	{
		WORD wCell = Cell.cell;
		if (wCell >= SPECIAL_INVENTORY_MAX_NUM)
			return false;
		iExceptionCell++;
		if (m_PlayerSlots->wSSFItemGrid[wCell])
		{
			if (m_PlayerSlots->wSSFItemGrid[wCell] == iExceptionCell)
			{
				if (bSize == 1)
					return true;
				int j = 1;
				do
				{
					int p = wCell + (5 * j);
					if (p >= SPECIAL_INVENTORY_MAX_NUM)
						return false;
					if (m_PlayerSlots->wSSFItemGrid[p])
						if (m_PlayerSlots->wSSFItemGrid[p] != iExceptionCell)
							return false;
				} while (++j < bSize);
				return true;
			}
			else
				return false;
		}
		if (1 == bSize)
			return true;
		else
		{
			int j = 1;
			do
			{
				int p = wCell + (5 * j);
				if (p >= SPECIAL_INVENTORY_MAX_NUM)
					return false;
				if (m_PlayerSlots->wSSFItemGrid[p]) // old bItemGrid
					if (m_PlayerSlots->wSSFItemGrid[p] != iExceptionCell)
						return false;
			} while (++j < bSize);
			return true;
		}
	}
	break;
	case ATTR_INVENTORY:
	{
		WORD wCell = Cell.cell;
		if (wCell >= SPECIAL_INVENTORY_MAX_NUM)
			return false;
		iExceptionCell++;
		if (m_PlayerSlots->wSSAItemGrid[wCell])
		{
			if (m_PlayerSlots->wSSAItemGrid[wCell] == iExceptionCell)
			{
				if (bSize == 1)
					return true;
				int j = 1;
				do
				{
					int p = wCell + (5 * j);
					if (p >= SPECIAL_INVENTORY_MAX_NUM)
						return false;
					if (m_PlayerSlots->wSSAItemGrid[p])
						if (m_PlayerSlots->wSSAItemGrid[p] != iExceptionCell)
							return false;
				} while (++j < bSize);
				return true;
			}
			else
				return false;
		}
		if (1 == bSize)
			return true;
		else
		{
			int j = 1;
			do
			{
				int p = wCell + (5 * j);
				if (p >= SPECIAL_INVENTORY_MAX_NUM)
					return false;
				if (m_PlayerSlots->wSSAItemGrid[p]) // old bItemGrid
					if (m_PlayerSlots->wSSAItemGrid[p] != iExceptionCell)
						return false;
			} while (++j < bSize);
			return true;
		}
	}
	break;
	case CHEST_INVENTORY:
	{
		WORD wCell = Cell.cell;
		if (wCell >= SPECIAL_INVENTORY_MAX_NUM)
			return false;
		iExceptionCell++;
		if (m_PlayerSlots->wSSCItemGrid[wCell])
		{
			if (m_PlayerSlots->wSSCItemGrid[wCell] == iExceptionCell)
			{
				if (bSize == 1)
					return true;
				int j = 1;
				do
				{
					int p = wCell + (5 * j);
					if (p >= SPECIAL_INVENTORY_MAX_NUM)
						return false;
					if (m_PlayerSlots->wSSCItemGrid[p])
						if (m_PlayerSlots->wSSCItemGrid[p] != iExceptionCell)
							return false;
				} while (++j < bSize);
				return true;
			}
			else
				return false;
		}
		if (1 == bSize)
			return true;
		else
		{
			int j = 1;
			do
			{
				int p = wCell + (5 * j);
				if (p >= SPECIAL_INVENTORY_MAX_NUM)
					return false;
				if (m_PlayerSlots->wSSCItemGrid[p]) // old bItemGrid
					if (m_PlayerSlots->wSSCItemGrid[p] != iExceptionCell)
						return false;
			} while (++j < bSize);
			return true;
		}
	}
	break;
#endif
#ifdef __SWITCHBOT__
	case SWITCHBOT:
	{
		WORD wCell = Cell.cell;
		if (wCell >= SWITCHBOT_SLOT_COUNT)
			return false;

		if (m_PlayerSlots->pSwitchbotItems[wCell])
			return false;

		return true;
	}
	break;
#endif
	}
	return false;
}

bool CHARACTER::IsEmptyItemGridSpecial(const TItemPos &Cell, BYTE bSize, int iExceptionCell, std::vector<WORD>& vec) const
{
	if (!m_PlayerSlots)
		return false;

	if (std::find(vec.begin(), vec.end(), Cell.cell) != vec.end()) {
		return false;
	}

	switch (Cell.window_type)
	{
	case DRAGON_SOUL_INVENTORY:
	{
		WORD wCell = Cell.cell;
		if (wCell >= DRAGON_SOUL_INVENTORY_MAX_NUM)
			return false;

		iExceptionCell++;

		if (m_PlayerSlots->wDSItemGrid[wCell])
		{
			if (m_PlayerSlots->wDSItemGrid[wCell] == iExceptionCell)
			{
				if (bSize == 1)
					return true;

				int j = 1;

				do
				{
					WORD p = wCell + (DRAGON_SOUL_BOX_COLUMN_NUM * j);

					if (p >= DRAGON_SOUL_INVENTORY_MAX_NUM)
						return false;

					if (m_PlayerSlots->wDSItemGrid[p])
						if (m_PlayerSlots->wDSItemGrid[p] != iExceptionCell)
							return false;
				} while (++j < bSize);

				return true;
			}
			else
				return false;
		}

		if (1 == bSize)
			return true;
		else
		{
			int j = 1;

			do
			{
				WORD p = wCell + (DRAGON_SOUL_BOX_COLUMN_NUM * j);

				if (p >= DRAGON_SOUL_INVENTORY_MAX_NUM)
					return false;

				if (m_PlayerSlots->bItemGrid[p])
					if (m_PlayerSlots->wDSItemGrid[p] != iExceptionCell)
						return false;
			} while (++j < bSize);

			return true;
		}
	}
	break;
	}
	return false;
}

int CHARACTER::GetEmptyInventory(BYTE size) const
{
	for (int i = 0; i < INVENTORY_MAX_NUM; ++i)
		if (IsEmptyItemGrid(TItemPos(INVENTORY, i), size))
			return i;
	return -1;
}

#ifdef __SPLIT_ITEMS__
int CHARACTER::GetEmptyInventoryFromIndex(WORD index, BYTE itemSize) const //SPLIT ITEMS
{
	if (index > INVENTORY_MAX_NUM)
		return -1;

	for (WORD i = index; i < INVENTORY_MAX_NUM; ++i)
		if (IsEmptyItemGrid(TItemPos(INVENTORY, i), itemSize))
			return i;
	return -1;
}

#ifdef __ADDITIONAL_INVENTORY__
int CHARACTER::GetEmptyUpgradeInventoryFromIndex(WORD index, BYTE itemSize) const //SPLIT ITEMS
{
	if (index > SPECIAL_INVENTORY_MAX_NUM)
		return -1;

	for (WORD i = index; i < SPECIAL_INVENTORY_MAX_NUM; ++i)
		if (IsEmptyItemGrid(TItemPos(UPGRADE_INVENTORY, i), itemSize))
			return i;
	return -1;
}

int CHARACTER::GetEmptyBookInventoryFromIndex(WORD index, BYTE itemSize) const //SPLIT ITEMS
{
	if (index > SPECIAL_INVENTORY_MAX_NUM)
		return -1;

	for (WORD i = index; i < SPECIAL_INVENTORY_MAX_NUM; ++i)
		if (IsEmptyItemGrid(TItemPos(BOOK_INVENTORY, i), itemSize))
			return i;
	return -1;
}

int CHARACTER::GetEmptyStoneInventoryFromIndex(WORD index, BYTE itemSize) const //SPLIT ITEMS
{
	if (index > SPECIAL_INVENTORY_MAX_NUM)
		return -1;

	for (WORD i = index; i < SPECIAL_INVENTORY_MAX_NUM; ++i)
		if (IsEmptyItemGrid(TItemPos(STONE_INVENTORY, i), itemSize))
			return i;
	return -1;
}

int CHARACTER::GetEmptyFlowerInventoryFromIndex(WORD index, BYTE itemSize) const //SPLIT ITEMS
{
	if (index > SPECIAL_INVENTORY_MAX_NUM)
		return -1;

	for (WORD i = index; i < SPECIAL_INVENTORY_MAX_NUM; ++i)
		if (IsEmptyItemGrid(TItemPos(FLOWER_INVENTORY, i), itemSize))
			return i;
	return -1;
}

int CHARACTER::GetEmptyAttrInventoryFromIndex(WORD index, BYTE itemSize) const //SPLIT ITEMS
{
	if (index > SPECIAL_INVENTORY_MAX_NUM)
		return -1;

	for (WORD i = index; i < SPECIAL_INVENTORY_MAX_NUM; ++i)
		if (IsEmptyItemGrid(TItemPos(ATTR_INVENTORY, i), itemSize))
			return i;
	return -1;
}

int CHARACTER::GetEmptyChestInventoryFromIndex(WORD index, BYTE itemSize) const //SPLIT ITEMS
{
	if (index > SPECIAL_INVENTORY_MAX_NUM)
		return -1;

	for (WORD i = index; i < SPECIAL_INVENTORY_MAX_NUM; ++i)
		if (IsEmptyItemGrid(TItemPos(CHEST_INVENTORY, i), itemSize))
			return i;
	return -1;
}
#endif
#endif

int CHARACTER::GetEmptyDragonSoulInventory(LPITEM pItem) const
{
	if (NULL == pItem || !pItem->IsDragonSoul())
		return -1;
	if (!DragonSoul_IsQualified())
	{
		return -1;
	}
	BYTE bSize = pItem->GetSize();
	WORD wBaseCell = DSManager::instance().GetBasePosition(pItem);

	if (WORD_MAX == wBaseCell)
		return -1;

	for (int i = 0; i < DRAGON_SOUL_BOX_SIZE; ++i)
		if (IsEmptyItemGrid(TItemPos(DRAGON_SOUL_INVENTORY, i + wBaseCell), bSize))
			return i + wBaseCell;

	return -1;
}

int CHARACTER::GetEmptyDragonSoulInventoryWithExceptions(LPITEM pItem, std::vector<WORD>& vec /*= -1*/) const
{
	if (NULL == pItem || !pItem->IsDragonSoul())
		return -1;
	if (!DragonSoul_IsQualified())
	{
		return -1;
	}
	BYTE bSize = pItem->GetSize();
	WORD wBaseCell = DSManager::instance().GetBasePosition(pItem);

	if (WORD_MAX == wBaseCell)
		return -1;

	for (int i = 0; i < DRAGON_SOUL_BOX_SIZE; ++i)
		if (IsEmptyItemGridSpecial(TItemPos(DRAGON_SOUL_INVENTORY, i + wBaseCell), bSize, -1, vec))
			return i + wBaseCell;

	return -1;
}

void CHARACTER::CopyDragonSoulItemGrid(std::vector<WORD>& vDragonSoulItemGrid) const
{
	if (!m_PlayerSlots)
		return;
	vDragonSoulItemGrid.resize(DRAGON_SOUL_INVENTORY_MAX_NUM);

	std::copy(m_PlayerSlots->wDSItemGrid, m_PlayerSlots->wDSItemGrid + DRAGON_SOUL_INVENTORY_MAX_NUM, vDragonSoulItemGrid.begin());
}

#ifdef __ADDITIONAL_INVENTORY__
int CHARACTER::GetSameUpgradeInventory(LPITEM pItem) const
{
	if (NULL == pItem || !pItem->IsUpgradeItem())
		return -1;
	for (int i = 0; i < SPECIAL_INVENTORY_MAX_NUM; ++i)
		if (GetUpgradeInventoryItem(i)->GetVnum() == pItem->GetVnum())
			return i;
	return -1;
}
int CHARACTER::GetSameBookInventory(LPITEM pItem) const
{
	if (NULL == pItem || !pItem->IsBook())
		return -1;
	for (int i = 0; i < SPECIAL_INVENTORY_MAX_NUM; ++i)
		if (GetBookInventoryItem(i)->GetVnum() == pItem->GetVnum() && GetBookInventoryItem(i)->GetSocket(0) == pItem->GetSocket(0))
			return i;
	return -1;
}
int CHARACTER::GetSameStoneInventory(LPITEM pItem) const
{
	if (NULL == pItem || !pItem->IsStone())
		return -1;
	for (int i = 0; i < SPECIAL_INVENTORY_MAX_NUM; ++i)
		if (GetStoneInventoryItem(i)->GetVnum() == pItem->GetVnum())
			return i;
	return -1;
}
int CHARACTER::GetSameFlowerInventory(LPITEM pItem) const
{
	if (NULL == pItem || !pItem->IsFlower())
		return -1;
	for (int i = 0; i < SPECIAL_INVENTORY_MAX_NUM; ++i)
		if (GetFlowerInventoryItem(i)->GetVnum() == pItem->GetVnum())
			return i;
	return -1;
}
int CHARACTER::GetSameAttrInventory(LPITEM pItem) const
{
	if (NULL == pItem || !pItem->IsAttrItem())
		return -1;
	for (int i = 0; i < SPECIAL_INVENTORY_MAX_NUM; ++i)
		if (GetAttrInventoryItem(i)->GetVnum() == pItem->GetVnum())
			return i;
	return -1;
}
int CHARACTER::GetSameChestInventory(LPITEM pItem) const
{
	if (NULL == pItem || !pItem->IsChest())
		return -1;
	for (int i = 0; i < SPECIAL_INVENTORY_MAX_NUM; ++i)
		if (GetChestInventoryItem(i)->GetVnum() == pItem->GetVnum())
			return i;
	return -1;
}
int CHARACTER::GetEmptyUpgradeInventory(LPITEM pItem) const
{
	if (NULL == pItem || !pItem->IsUpgradeItem())
		return -1;
	BYTE bSize = pItem->GetSize();
	for (int i = 0; i < SPECIAL_INVENTORY_MAX_NUM; ++i)
		if (IsEmptyItemGrid(TItemPos(UPGRADE_INVENTORY, i), bSize))
			return i;
	return -1;
}
int CHARACTER::GetEmptyBookInventory(LPITEM pItem) const
{
	if (NULL == pItem || !pItem->IsBook())
		return -1;
	BYTE bSize = pItem->GetSize();
	for (int i = 0; i < SPECIAL_INVENTORY_MAX_NUM; ++i)
		if (IsEmptyItemGrid(TItemPos(BOOK_INVENTORY, i), bSize))
			return i;
	return -1;
}
int CHARACTER::GetEmptyStoneInventory(LPITEM pItem) const
{
	if (NULL == pItem || !pItem->IsStone())
		return -1;
	BYTE bSize = pItem->GetSize();
	for (int i = 0; i < SPECIAL_INVENTORY_MAX_NUM; ++i)
		if (IsEmptyItemGrid(TItemPos(STONE_INVENTORY, i), bSize))
			return i;
	return -1;
}
int CHARACTER::GetEmptyFlowerInventory(LPITEM pItem) const
{
	if (NULL == pItem || !pItem->IsFlower())
		return -1;
	BYTE bSize = pItem->GetSize();
	for (int i = 0; i < SPECIAL_INVENTORY_MAX_NUM; ++i)
		if (IsEmptyItemGrid(TItemPos(FLOWER_INVENTORY, i), bSize))
			return i;
	return -1;
}
int CHARACTER::GetEmptyAttrInventory(LPITEM pItem) const
{
	if (NULL == pItem || !pItem->IsAttrItem())
		return -1;
	BYTE bSize = pItem->GetSize();
	for (int i = 0; i < SPECIAL_INVENTORY_MAX_NUM; ++i)
		if (IsEmptyItemGrid(TItemPos(ATTR_INVENTORY, i), bSize))
			return i;
	return -1;
}
int CHARACTER::GetEmptyChestInventory(LPITEM pItem) const
{
	if (NULL == pItem || !pItem->IsChest())
		return -1;
	BYTE bSize = pItem->GetSize();
	for (int i = 0; i < SPECIAL_INVENTORY_MAX_NUM; ++i)
		if (IsEmptyItemGrid(TItemPos(CHEST_INVENTORY, i), bSize))
			return i;
	return -1;
}
#endif

int CHARACTER::CountEmptyInventory() const
{
	int	count = 0;

	for (int i = 0; i < INVENTORY_MAX_NUM; ++i)
		if (GetInventoryItem(i))
			count += GetInventoryItem(i)->GetSize();

	return (INVENTORY_MAX_NUM - count);
}

void TransformRefineItem(LPITEM pkOldItem, LPITEM pkNewItem)
{
	// ACCESSORY_REFINE
	if (pkOldItem->IsAccessoryForSocket())
	{
		for (int i = 0; i < ITEM_SOCKET_MAX_NUM; ++i)
		{
			pkNewItem->SetSocket(i, pkOldItem->GetSocket(i));
		}
		//pkNewItem->StartAccessorySocketExpireEvent();
	}
	// END_OF_ACCESSORY_REFINE
	else
	{
		for (int i = 0; i < ITEM_SOCKET_MAX_NUM; ++i)
		{
			if (!pkOldItem->GetSocket(i))
				break;
			else
				pkNewItem->SetSocket(i, 1);
		}

		int slot = 0;

		for (int i = 0; i < ITEM_SOCKET_MAX_NUM; ++i)
		{
			long socket = pkOldItem->GetSocket(i);

			if (socket > 2 && socket != ITEM_BROKEN_METIN_VNUM)
				pkNewItem->SetSocket(slot++, socket);
		}
	}

	pkOldItem->CopyAttributeTo(pkNewItem);
}

void NotifyRefineSuccess(LPCHARACTER ch, LPITEM item, const char* way)
{
	if (NULL != ch && item != NULL)
	{
		ch->ChatPacket(CHAT_TYPE_COMMAND, "RefineSuceeded");
#ifdef __SMITH_EFFECT__
		ch->EffectPacket(SE_FR_SUCCESS);
#endif
	}
}

#ifdef __REFINE_MSG_REWORK__
void NotifyRefineFailType(const LPCHARACTER ch, const LPITEM pkItem, const BYTE bType, const std::string stRefineType, const BYTE bSuccess = 0)
{
	if (ch && pkItem)
	{
		ch->ChatPacket(CHAT_TYPE_COMMAND, "RefineFailedType %d", bType);
#ifdef __SMITH_EFFECT__
		ch->EffectPacket(SE_FR_FAIL);
#endif
	}
}
#else
void NotifyRefineFail(LPCHARACTER ch, LPITEM item, const char* way, int success = 0)
{
	if (NULL != ch && NULL != item)
	{
		ch->ChatPacket(CHAT_TYPE_COMMAND, "RefineFailed");
#ifdef __SMITH_EFFECT__
		ch->EffectPacket(SE_FR_FAIL);
#endif
	}
}
#endif

void CHARACTER::SetRefineNPC(LPCHARACTER ch)
{
	if (ch != NULL)
		m_dwRefineNPCVID = ch->GetVID();
	else
		m_dwRefineNPCVID = 0;
}

bool CHARACTER::DoRefine(LPITEM item, bool bMoneyOnly)
{
	if (!CanHandleItem(true))
	{
		ClearRefineMode();
		return false;
	}

	const TRefineTable* prt = CRefineManager::instance().GetRefineRecipe(item->GetRefineSet());

	if (!prt)
		return false;

	DWORD result_vnum = item->GetRefinedVnum();

	// REFINE_COST
#ifdef __GOLD_LIMIT_REWORK__
	long long cost = ComputeRefineFee(prt->cost);
#else
	int cost = ComputeRefineFee(prt->cost);
#endif

	int RefineChance = GetQuestFlag("main_quest_lv7.refine_chance");

	if (RefineChance > 0)
	{
		if (!item->CheckItemUseLevel(20) || item->GetType() != ITEM_WEAPON)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¹«·á °³·® ±âÈ¸´Â 20 ÀÌÇÏÀÇ ¹«±â¸¸ °¡´ÉÇÕ´Ï´Ù"));
			return false;
		}

		cost = 0;
		SetQuestFlag("main_quest_lv7.refine_chance", RefineChance - 1);
	}
	// END_OF_REFINE_COST

	if (result_vnum == 0)
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´õ ÀÌ»ó °³·®ÇÒ ¼ö ¾ø½À´Ï´Ù."));
		return false;
	}

	if (item->GetType() == ITEM_USE && item->GetSubType() == USE_TUNING)
		return false;

	TItemTable* pProto = ITEM_MANAGER::instance().GetTable(item->GetRefinedVnum());

	if (!pProto)
	{
		sys_err("DoRefine NOT GET ITEM PROTO %d", item->GetRefinedVnum());
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ ¾ÆÀÌÅÛÀº °³·®ÇÒ ¼ö ¾ø½À´Ï´Ù."));
		return false;
	}

	// REFINE_COST
	if (GetGold() < cost)
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("°³·®À» ÇÏ±â À§ÇÑ µ·ÀÌ ºÎÁ·ÇÕ´Ï´Ù."));
		return false;
	}

	if (!bMoneyOnly && !RefineChance)
	{
		for (int i = 0; i < prt->material_count; ++i)
		{
			if (CountSpecifyItem(prt->materials[i].vnum) < prt->materials[i].count)
			{
				if (test_server)
				{
					ChatPacket(CHAT_TYPE_INFO, "Find %d, count %d, require %d", prt->materials[i].vnum, CountSpecifyItem(prt->materials[i].vnum), prt->materials[i].count);
				}
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("°³·®À» ÇÏ±â À§ÇÑ Àç·á°¡ ºÎÁ·ÇÕ´Ï´Ù."));
				return false;
			}
		}

		for (int i = 0; i < prt->material_count; ++i)
			RemoveSpecifyItem(prt->materials[i].vnum, prt->materials[i].count);
	}

	int prob = number(1, 100);

	if (bMoneyOnly)
		prob -= 10;

	// END_OF_REFINE_COST

#if defined(__REFINE_REWORK__)
	int success_prob = prt->prob;
#endif

#ifdef __REFINE_REWORK__
	CSkillProto* pkSk = CSkillManager::instance().Get(SKILL_REFINE);
	if (NULL != pkSk)
	{
		pkSk->SetPointVar("k", 1.0f * GetSkillPower(SKILL_REFINE) / 100.0f);
		success_prob += static_cast<int>(pkSk->kPointPoly.Eval());
	}
#endif

	if (CGameEventsManager::instance().IsActivateEvent(EVENT_UPGRADE) == true)
		success_prob += 10;

#if defined(__REFINE_REWORK__)
	if (success_prob > 100)
		success_prob = 100;
#endif

#if defined(__REFINE_REWORK__)
	if (prob <= success_prob)
#else
	if (prob <= prt->prob)
#endif
	{
		LPITEM pkNewItem = ITEM_MANAGER::instance().CreateItem(result_vnum, 1, 0, false);

		if (pkNewItem)
		{
			ITEM_MANAGER::CopyAllAttrTo(item, pkNewItem);

			BYTE bCell = item->GetCell();

#ifdef __BATTLE_PASS_SYSTEM__
			BYTE bBattlePassId = GetBattlePassId();
			if (bBattlePassId)
			{
				DWORD dwItemVnum, dwCount;
				if (CBattlePass::instance().BattlePassMissionGetInfo(bBattlePassId, REFINE_ITEM, &dwItemVnum, &dwCount))
				{
					if (dwItemVnum == item->GetVnum() && GetMissionProgress(REFINE_ITEM, bBattlePassId) < dwCount)
						UpdateMissionProgress(REFINE_ITEM, bBattlePassId, 1, dwCount);
				}
			}
#endif


			// DETAIL_REFINE_LOG
			NotifyRefineSuccess(this, item, "POWER");
			ITEM_MANAGER::instance().RemoveItem(item, "REMOVE (REFINE SUCCESS)");
			// END_OF_DETAIL_REFINE_LOG
			pkNewItem->__ADD_TO_CHARACTER(this, TItemPos(INVENTORY, bCell));
			ITEM_MANAGER::instance().FlushDelayedSave(pkNewItem);
#ifdef __GOLD_LIMIT_REWORK__
			sys_log(0, "Refine Success %lld", cost);
			//PointChange(POINT_GOLD, -cost);
			sys_log(0, "PayPee %lld", cost);
			sys_log(0, "PayPee End %lld", cost);
#else
			sys_log(0, "Refine Success %d", cost);
			//PointChange(POINT_GOLD, -cost);
			sys_log(0, "PayPee %d", cost);
			sys_log(0, "PayPee End %d", cost);
#endif
		}
		else
		{
			// DETAIL_REFINE_LOG

			sys_err("cannot create item %u", result_vnum);
#ifdef __REFINE_MSG_REWORK__
			NotifyRefineFailType(this, item, REFINE_FAIL_KEEP_GRADE, "POWER");
#else
			NotifyRefineFail(this, item, "POWER");
#endif
			// END_OF_DETAIL_REFINE_LOG
		}
	}
	else
	{
#ifdef __REFINE_MSG_REWORK__
		NotifyRefineFailType(this, item, REFINE_FAIL_DEL_ITEM, "POWER");
#else
		NotifyRefineFail(this, item, "POWER");
#endif
		ITEM_MANAGER::instance().RemoveItem(item, "REMOVE (REFINE FAIL)");

		//PointChange(POINT_GOLD, -cost);
		PayRefineFee(cost);
	}

	return true;
}

enum enum_RefineScrolls
{
	CHUKBOK_SCROLL = 0,
	HYUNIRON_CHN = 1,
	YONGSIN_SCROLL = 2,
	MUSIN_SCROLL = 3,
	YAGONG_SCROLL = 4,
	MEMO_SCROLL = 5,
	BDRAGON_SCROLL = 6,
#ifdef __REFINE_SCROLL__
	PLUS_SCROLL = 15,
#endif
};

bool CHARACTER::DoRefineWithScroll(LPITEM item)
{
	if (!CanHandleItem(true))
	{
		ClearRefineMode();
		return false;
	}

	ClearRefineMode();

	const TRefineTable* prt = CRefineManager::instance().GetRefineRecipe(item->GetRefineSet());

	if (!prt)
		return false;

	LPITEM pkItemScroll;

	if (m_iRefineAdditionalCell < 0)
		return false;

	pkItemScroll = GetInventoryItem(m_iRefineAdditionalCell);

	if (!pkItemScroll)
		return false;

	if (!(pkItemScroll->GetType() == ITEM_USE && pkItemScroll->GetSubType() == USE_TUNING))
		return false;

	if (pkItemScroll->GetVnum() == item->GetVnum())
		return false;

	DWORD result_vnum = item->GetRefinedVnum();
	DWORD result_fail_vnum = item->GetRefineFromVnum();

	if (result_vnum == 0)
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´õ ÀÌ»ó °³·®ÇÒ ¼ö ¾ø½À´Ï´Ù."));
		return false;
	}

	// MUSIN_SCROLL
	if (pkItemScroll->GetValue(0) == MUSIN_SCROLL)
	{
		if (item->GetRefineLevel() >= 4)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ °³·®¼­·Î ´õ ÀÌ»ó °³·®ÇÒ ¼ö ¾ø½À´Ï´Ù."));
			return false;
		}
	}
	// END_OF_MUSIC_SCROLL

	else if (pkItemScroll->GetValue(0) == MEMO_SCROLL)
	{
		if (item->GetRefineLevel() != pkItemScroll->GetValue(1))
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ °³·®¼­·Î °³·®ÇÒ ¼ö ¾ø½À´Ï´Ù."));
			return false;
		}
	}
	else if (pkItemScroll->GetValue(0) == BDRAGON_SCROLL)
	{
		if (item->GetType() != ITEM_METIN || item->GetRefineLevel() != 4)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ ¾ÆÀÌÅÛÀ¸·Î °³·®ÇÒ ¼ö ¾ø½À´Ï´Ù."));
			return false;
		}
	}

	TItemTable* pProto = ITEM_MANAGER::instance().GetTable(item->GetRefinedVnum());

	if (!pProto)
	{
		sys_err("DoRefineWithScroll NOT GET ITEM PROTO %d", item->GetRefinedVnum());
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ ¾ÆÀÌÅÛÀº °³·®ÇÒ ¼ö ¾ø½À´Ï´Ù."));
		return false;
	}

	if (GetGold() < prt->cost)
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("°³·®À» ÇÏ±â À§ÇÑ µ·ÀÌ ºÎÁ·ÇÕ´Ï´Ù."));
		return false;
	}

	for (int i = 0; i < prt->material_count; ++i)
	{
		if (CountSpecifyItem(prt->materials[i].vnum) < prt->materials[i].count)
		{
			if (test_server)
			{
				ChatPacket(CHAT_TYPE_INFO, "Find %d, count %d, require %d", prt->materials[i].vnum, CountSpecifyItem(prt->materials[i].vnum), prt->materials[i].count);
			}
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("°³·®À» ÇÏ±â À§ÇÑ Àç·á°¡ ºÎÁ·ÇÕ´Ï´Ù."));
			return false;
		}
	}

	for (int i = 0; i < prt->material_count; ++i)
		RemoveSpecifyItem(prt->materials[i].vnum, prt->materials[i].count);

	int prob = number(1, 100);
	int success_prob = prt->prob;
	bool bDestroyWhenFail = false;

	const char* szRefineType = "SCROLL";

	if (pkItemScroll->GetValue(0) == HYUNIRON_CHN ||
		pkItemScroll->GetValue(0) == YONGSIN_SCROLL ||
		pkItemScroll->GetValue(0) == YAGONG_SCROLL
#ifdef __REFINE_SCROLL__
		|| pkItemScroll->GetValue(0) == PLUS_SCROLL
#endif
		)
	{
		const char hyuniron_prob[9] = { 100, 75, 65, 55, 45, 40, 35, 25, 20 };
		const char yagong_prob[9] = { 100, 100, 90, 80, 70, 60, 50, 30, 20 };

		if (pkItemScroll->GetValue(0) == YONGSIN_SCROLL)
			success_prob = hyuniron_prob[MINMAX(0, item->GetRefineLevel(), 8)];
		else if (pkItemScroll->GetValue(0) == YAGONG_SCROLL)
			success_prob = yagong_prob[MINMAX(0, item->GetRefineLevel(), 8)];
		else if (pkItemScroll->GetValue(0) == HYUNIRON_CHN) {} // @duzenleme syserr veriyordu.
#ifdef __REFINE_SCROLL__
		else if (pkItemScroll->GetValue(0) == PLUS_SCROLL) {}
#endif
		else
			sys_err("REFINE : Unknown refine scroll item. Value0: %d", pkItemScroll->GetValue(0));

		if (test_server)
		{
			ChatPacket(CHAT_TYPE_INFO, "[Only Test] Success_Prob %d, RefineLevel %d ", success_prob, item->GetRefineLevel());
		}
#ifdef __REFINE_SCROLL__
		if (pkItemScroll->GetValue(0) == HYUNIRON_CHN || pkItemScroll->GetValue(0) == PLUS_SCROLL)
#else
		if (pkItemScroll->GetValue(0) == HYUNIRON_CHN)
#endif
			bDestroyWhenFail = true;

		// DETAIL_REFINE_LOG
		if (pkItemScroll->GetValue(0) == HYUNIRON_CHN)
		{
			szRefineType = "HYUNIRON";
		}
		else if (pkItemScroll->GetValue(0) == YONGSIN_SCROLL)
		{
			szRefineType = "GOD_SCROLL";
		}
		else if (pkItemScroll->GetValue(0) == YAGONG_SCROLL)
		{
			szRefineType = "YAGONG_SCROLL";
		}
#ifdef __REFINE_SCROLL__
		else if (pkItemScroll->GetValue(0) == PLUS_SCROLL)
		{
			success_prob += pkItemScroll->GetValue(2);
			szRefineType = "PLUS_SCROLL";
		}
#endif
		// END_OF_DETAIL_REFINE_LOG
	}

	// DETAIL_REFINE_LOG
	if (pkItemScroll->GetValue(0) == MUSIN_SCROLL)
	{
		success_prob = 100;

		szRefineType = "MUSIN_SCROLL";
	}
	// END_OF_DETAIL_REFINE_LOG
	else if (pkItemScroll->GetValue(0) == MEMO_SCROLL)
	{
		success_prob = 100;
		szRefineType = "MEMO_SCROLL";
	}
	else if (pkItemScroll->GetValue(0) == BDRAGON_SCROLL)
	{
		success_prob = 80;
		szRefineType = "BDRAGON_SCROLL";
	}

	pkItemScroll->SetCount(pkItemScroll->GetCount() - 1);

#ifdef __REFINE_REWORK__
	CSkillProto* pkSk = CSkillManager::instance().Get(SKILL_REFINE);
	if (NULL != pkSk)
	{
		pkSk->SetPointVar("k", 1.0f * GetSkillPower(SKILL_REFINE) / 100.0f);
		success_prob += static_cast<int>(pkSk->kPointPoly.Eval());
	}
#endif

	if (CGameEventsManager::instance().IsActivateEvent(EVENT_UPGRADE) == true)
		success_prob += 10;

#if defined(__REFINE_REWORK__)
	if (success_prob > 100)
		success_prob = 100;
#endif

	if (prob <= success_prob)
	{
		LPITEM pkNewItem = ITEM_MANAGER::instance().CreateItem(result_vnum, 1, 0, false);

		if (pkNewItem)
		{
			ITEM_MANAGER::CopyAllAttrTo(item, pkNewItem);

#ifdef __BATTLE_PASS_SYSTEM__
			BYTE bBattlePassId = GetBattlePassId();
			if (bBattlePassId)
			{
				DWORD dwItemVnum, dwCount;
				if (CBattlePass::instance().BattlePassMissionGetInfo(bBattlePassId, REFINE_ITEM, &dwItemVnum, &dwCount))
				{
					if (dwItemVnum == item->GetVnum() && GetMissionProgress(REFINE_ITEM, bBattlePassId) < dwCount)
						UpdateMissionProgress(REFINE_ITEM, bBattlePassId, 1, dwCount);
				}
			}
#endif

			BYTE bCell = item->GetCell();
			NotifyRefineSuccess(this, item, szRefineType);
			ITEM_MANAGER::instance().RemoveItem(item, "REMOVE (REFINE SUCCESS)");
			pkNewItem->__ADD_TO_CHARACTER(this, TItemPos(INVENTORY, bCell));
			ITEM_MANAGER::instance().FlushDelayedSave(pkNewItem);
			//PointChange(POINT_GOLD, -prt->cost);
			PayRefineFee(prt->cost);
		}
		else
		{
			sys_err("cannot create item %u", result_vnum);
#ifdef __REFINE_MSG_REWORK__
			NotifyRefineFailType(this, item, REFINE_FAIL_KEEP_GRADE, szRefineType);
#else
			NotifyRefineFail(this, item, szRefineType);
#endif
		}
	}
	else if (!bDestroyWhenFail && result_fail_vnum)
	{
		LPITEM pkNewItem = ITEM_MANAGER::instance().CreateItem(result_fail_vnum, 1, 0, false);

		if (pkNewItem)
		{
			ITEM_MANAGER::CopyAllAttrTo(item, pkNewItem);

#ifdef __BATTLE_PASS_SYSTEM__
			BYTE bBattlePassId = GetBattlePassId();
			if (bBattlePassId)
			{
				DWORD dwItemVnum, dwCount;
				if (CBattlePass::instance().BattlePassMissionGetInfo(bBattlePassId, REFINE_ITEM, &dwItemVnum, &dwCount))
				{
					if (dwItemVnum == item->GetVnum() && GetMissionProgress(REFINE_ITEM, bBattlePassId) < dwCount)
						UpdateMissionProgress(REFINE_ITEM, bBattlePassId, 1, dwCount);
				}
			}
#endif

			BYTE bCell = item->GetCell();
#ifdef __REFINE_MSG_REWORK__
			NotifyRefineFailType(this, item, REFINE_FAIL_GRADE_DOWN, szRefineType, -1);
#else
			NotifyRefineFail(this, item, szRefineType, -1);
#endif
			ITEM_MANAGER::instance().RemoveItem(item, "REMOVE (REFINE FAIL)");

			pkNewItem->__ADD_TO_CHARACTER(this, TItemPos(INVENTORY, bCell));
			ITEM_MANAGER::instance().FlushDelayedSave(pkNewItem);

			//PointChange(POINT_GOLD, -prt->cost);
			PayRefineFee(prt->cost);
		}
		else
		{
			sys_err("cannot create item %u", result_fail_vnum);
#ifdef __REFINE_MSG_REWORK__
			NotifyRefineFailType(this, item, REFINE_FAIL_KEEP_GRADE, szRefineType);
#else
			NotifyRefineFail(this, item, szRefineType);
#endif
		}
	}
	else
	{
#ifdef __REFINE_MSG_REWORK__
		NotifyRefineFailType(this, item, REFINE_FAIL_KEEP_GRADE, szRefineType);
#else
		NotifyRefineFail(this, item, szRefineType);
#endif
		PayRefineFee(prt->cost);
	}

	return true;
}

#ifdef __ITEM_EVOLUTION__
bool CHARACTER::CleanEvolution(LPITEM pkItem, LPITEM pkTarget)
{
	if (!CanHandleItem())
		return false;
	else if ((!pkItem) || (!pkTarget))
		return false;
	else if ((pkTarget->GetEvolution() <= 0))
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("EV_CLEAN_DOESNT_HAVE_RARITY"));
		return false;
	}
	else if (GetGold() < EV_CLEAN_VALUE_GOLD)
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("EV_CLEAN_NOT_ENOUGH_GOLD"));
		return false;
	}

	pkItem->SetCount(pkItem->GetCount() - 1);
	PointChange(POINT_GOLD, -EV_CLEAN_VALUE_GOLD);
	pkTarget->SetEvolution(0);
	return true;
}
bool CHARACTER::DoRefine_Rarity(LPITEM item, LPITEM scroll)
{
	if (!item)
		return false;

	if (!scroll)
		return false;
#ifdef __ARMOR_EVOLUTION__
	if (item->GetType() != ITEM_WEAPON && (item->GetType() == ITEM_ARMOR && item->GetSubType() != ARMOR_BODY))
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("RARITY_REFINE_ONLY_WEAPON_OR_ARMOR"));
		return false;
	}
#else
	if (item->GetType() != ITEM_WEAPON)
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("RARITY_REFINE_ONLY_WEAPON_OR_ARMOR"));
		return false;
	}
#endif
	else
	{
		if (!CanHandleItem(true))
			return false;

#ifdef __BEGINNER_ITEM__
		if (item->IsBasicItem())
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ITEM_IS_BASIC_CANNOT_DO"));
			return false;
		}
#endif

		if (quest::CQuestManager::instance().GetPCForce(GetPlayerID())->IsRunning() == true)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_OPERATION_IS_FAILED_QUEST_WINDOW_IS_OPENED"));
			return false;
		}

		if (!CanAct(true, true, VAR_TIME_REFINE_RARITY, nullptr, 0))
			return false;

		DWORD itemVnum = item->GetVnum();
		DWORD itemEvo = item->GetRealEvolution();
		int needItemCount = (itemEvo % 20) * 100;
		const DWORD rrNeedMoney[] = { 5000000, 10000000, 15000000, 20000000, 25000000, 30000000, 35000000, 40000000, 50000000 };
		const DWORD rrNeedItems[] = { 31001 , 31002, 31003, 31004, 31007, 31010, 95011, 70075 };
		const DWORD rrNeedPercents[] = { 90, 85, 80, 70, 60, 65, 50, 40, 30, 90, 90, 85, 80, 70, 60, 65, 50, 40, 30, 30 };

		int rrPercent = rrNeedPercents[itemEvo];

		if (itemEvo == 20)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ ¾ÆÀÌÅÛÀº °³·®ÇÒ ¼ö ¾ø½À´Ï´Ù."));
			return false;
		}

		if (itemVnum == 0)
		{
			sys_err("EvolutionInformation p.result_vnum == 0");
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ ¾ÆÀÌÅÛÀº °³·®ÇÒ ¼ö ¾ø½À´Ï´Ù."));
			return false;
		}

		if (scroll->GetVnum() == 56002)
		{
			if (item->GetEvolutionType() != WEAPON_EVOLUTION_TYPE_FIRE && item->GetEvolutionType() != WEAPON_EVOLUTION_TYPE_NONE)
			{
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("EVO_TYPE_IS_WRONG"));
				return false;
			}
		}
		else if (scroll->GetVnum() == 56003)
		{
			if (item->GetEvolutionType() != WEAPON_EVOLUTION_TYPE_ICE && item->GetEvolutionType() != WEAPON_EVOLUTION_TYPE_NONE)
			{
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("EVO_TYPE_IS_WRONG"));
				return false;
			}
		}
		else if (scroll->GetVnum() == 56004)
		{
			if (item->GetEvolutionType() != WEAPON_EVOLUTION_TYPE_ELEC && item->GetEvolutionType() != WEAPON_EVOLUTION_TYPE_NONE)
			{
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("EVO_TYPE_IS_WRONG"));
				return false;
			}
		}
		else if (scroll->GetVnum() == 56005)
		{
			if (item->GetEvolutionType() != WEAPON_EVOLUTION_TYPE_WIND && item->GetEvolutionType() != WEAPON_EVOLUTION_TYPE_NONE)
			{
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("EVO_TYPE_IS_WRONG"));
				return false;
			}
		}
		else if (scroll->GetVnum() == 56006)
		{
			if (item->GetEvolutionType() != WEAPON_EVOLUTION_TYPE_EARTH && item->GetEvolutionType() != WEAPON_EVOLUTION_TYPE_NONE)
			{
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("EVO_TYPE_IS_WRONG"));
				return false;
			}
		}
		else if (scroll->GetVnum() == 56007)
		{
			if (item->GetEvolutionType() != WEAPON_EVOLUTION_TYPE_DARK && item->GetEvolutionType() != WEAPON_EVOLUTION_TYPE_NONE)
			{
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("EVO_TYPE_IS_WRONG"));
				return false;
			}
		}
		else if (scroll->GetVnum() == 56011)
		{
			if (item->GetEvolutionType() != WEAPON_EVOLUTION_TYPE_ALL && item->GetEvolutionType() != WEAPON_EVOLUTION_TYPE_NONE)
			{
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("EVO_TYPE_IS_WRONG"));
				return false;
			}
		}
		else if (scroll->GetVnum() == 56014)
		{
			if (item->GetEvolutionType() != WEAPON_EVOLUTION_TYPE_HUMAN && item->GetEvolutionType() != WEAPON_EVOLUTION_TYPE_NONE)
			{
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("EVO_TYPE_IS_WRONG"));
				return false;
			}
		}
		else if (scroll->GetVnum() == 53140)
		{
			if (item->GetEvolutionType() != WEAPON_EVOLUTION_TYPE_EX_FIRE && item->GetEvolutionType() != WEAPON_EVOLUTION_TYPE_NONE)
			{
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("EVO_TYPE_IS_WRONG"));
				return false;
			}
		}
		else if (scroll->GetVnum() == 53141)
		{
			if (item->GetEvolutionType() != WEAPON_EVOLUTION_TYPE_EX_ICE && item->GetEvolutionType() != WEAPON_EVOLUTION_TYPE_NONE)
			{
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("EVO_TYPE_IS_WRONG"));
				return false;
			}
		}
		else if (scroll->GetVnum() == 53142)
		{
			if (item->GetEvolutionType() != WEAPON_EVOLUTION_TYPE_EX_ELEC && item->GetEvolutionType() != WEAPON_EVOLUTION_TYPE_NONE)
			{
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("EVO_TYPE_IS_WRONG"));
				return false;
			}
		}
		else if (scroll->GetVnum() == 53143)
		{
			if (item->GetEvolutionType() != WEAPON_EVOLUTION_TYPE_EX_WIND && item->GetEvolutionType() != WEAPON_EVOLUTION_TYPE_NONE)
			{
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("EVO_TYPE_IS_WRONG"));
				return false;
			}
		}
		else if (scroll->GetVnum() == 53144)
		{
			if (item->GetEvolutionType() != WEAPON_EVOLUTION_TYPE_EX_EARTH && item->GetEvolutionType() != WEAPON_EVOLUTION_TYPE_NONE)
			{
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("EVO_TYPE_IS_WRONG"));
				return false;
			}
		}
		else if (scroll->GetVnum() == 53145)
		{
			if (item->GetEvolutionType() != WEAPON_EVOLUTION_TYPE_EX_DARK && item->GetEvolutionType() != WEAPON_EVOLUTION_TYPE_NONE)
			{
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("EVO_TYPE_IS_WRONG"));
				return false;
			}
		}
		else if (scroll->GetVnum() == 53146)
		{
			if (item->GetEvolutionType() != WEAPON_EVOLUTION_TYPE_EX_ALL && item->GetEvolutionType() != WEAPON_EVOLUTION_TYPE_NONE)
			{
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("EVO_TYPE_IS_WRONG"));
				return false;
			}
		}

		if (itemEvo >= 20)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("EVO_REFINE_IS_MAX"));
			return false;
		}

		if (GetGold() < rrNeedMoney[(itemEvo % 20)])
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("°³·®À» ÇÏ±â À§ÇÑ µ·ÀÌ ºÎÁ·ÇÕ´Ï´Ù."));
			return false;
		}

		int i = 0;
		bool bElement = false;

		if (scroll->GetVnum() == 53146)
			bElement = true;

		for (; i < _countof(rrNeedItems); ++i)
		{
			if (bElement)
			{
				if ((itemEvo % 20) >= 10)
				{
					if (CountSpecifyItem(rrNeedItems[i]) < 50)
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("°³·®À» ÇÏ±â À§ÇÑ Àç·á°¡ ºÎÁ·ÇÕ´Ï´Ù."));
						return false;
					}
				}
				else
				{
					if (CountSpecifyItem(rrNeedItems[i]) < 25)
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("°³·®À» ÇÏ±â À§ÇÑ Àç·á°¡ ºÎÁ·ÇÕ´Ï´Ù."));
						return false;
					}
				}
			}
			else
			{
				if ((itemEvo % 20) >= 10)
				{
					if (CountSpecifyItem(rrNeedItems[i]) < 10)
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("°³·®À» ÇÏ±â À§ÇÑ Àç·á°¡ ºÎÁ·ÇÕ´Ï´Ù."));
						return false;
					}
				}
				else
				{
					if (CountSpecifyItem(rrNeedItems[i]) < 5)
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("°³·®À» ÇÏ±â À§ÇÑ Àç·á°¡ ºÎÁ·ÇÕ´Ï´Ù."));
						return false;
					}
				}
			}
		}

		int prob = number(1, 100);

		if (prob <= rrPercent)
		{
			LPITEM pkNewItem = ITEM_MANAGER::instance().CreateItem(itemVnum, 1, 0, false);

			if (pkNewItem)
			{
				ITEM_MANAGER::CopyAllAttrTo(item, pkNewItem);
				if (scroll->GetVnum() == 56002)
				{
					if (item->GetEvolution() == 0)
						pkNewItem->SetEvolution(20 + 1);
					else
						pkNewItem->SetEvolution(item->GetEvolution() + 1);
				}
				else if (scroll->GetVnum() == 56003)
				{
					if (item->GetEvolution() == 0)
						pkNewItem->SetEvolution(40 + 1);
					else
						pkNewItem->SetEvolution(item->GetEvolution() + 1);
				}
				else if (scroll->GetVnum() == 56004)
				{
					if (item->GetEvolution() == 0)
						pkNewItem->SetEvolution(120 + 1);
					else
						pkNewItem->SetEvolution(item->GetEvolution() + 1);
				}
				else if (scroll->GetVnum() == 56005)
				{
					if (item->GetEvolution() == 0)
						pkNewItem->SetEvolution(80 + 1);
					else
						pkNewItem->SetEvolution(item->GetEvolution() + 1);
				}
				else if (scroll->GetVnum() == 56006)
				{
					if (item->GetEvolution() == 0)
						pkNewItem->SetEvolution(60 + 1);
					else
						pkNewItem->SetEvolution(item->GetEvolution() + 1);
				}
				else if (scroll->GetVnum() == 56007)
				{
					if (item->GetEvolution() == 0)
						pkNewItem->SetEvolution(100 + 1);
					else
						pkNewItem->SetEvolution(item->GetEvolution() + 1);
				}
				else if (scroll->GetVnum() == 56011)
				{
					if (item->GetEvolution() == 0)
						pkNewItem->SetEvolution(140 + 1);
					else
						pkNewItem->SetEvolution(item->GetEvolution() + 1);
				}
				else if (scroll->GetVnum() == 56014)
				{
					if (item->GetEvolution() == 0)
						pkNewItem->SetEvolution(160 + 1);
					else
						pkNewItem->SetEvolution(item->GetEvolution() + 1);
				}
				else if (scroll->GetVnum() == 53140)
				{
					if (item->GetEvolution() == 0)
						pkNewItem->SetEvolution(180 + 1);
					else
						pkNewItem->SetEvolution(item->GetEvolution() + 1);
				}
				else if (scroll->GetVnum() == 53141)
				{
					if (item->GetEvolution() == 0)
						pkNewItem->SetEvolution(200 + 1);
					else
						pkNewItem->SetEvolution(item->GetEvolution() + 1);
				}
				else if (scroll->GetVnum() == 53142)
				{
					if (item->GetEvolution() == 0)
						pkNewItem->SetEvolution(280 + 1);
					else
						pkNewItem->SetEvolution(item->GetEvolution() + 1);
				}
				else if (scroll->GetVnum() == 53143)
				{
					if (item->GetEvolution() == 0)
						pkNewItem->SetEvolution(240 + 1);
					else
						pkNewItem->SetEvolution(item->GetEvolution() + 1);
				}
				else if (scroll->GetVnum() == 53144)
				{
					if (item->GetEvolution() == 0)
						pkNewItem->SetEvolution(220 + 1);
					else
						pkNewItem->SetEvolution(item->GetEvolution() + 1);
				}
				else if (scroll->GetVnum() == 53145)
				{
					if (item->GetEvolution() == 0)
						pkNewItem->SetEvolution(260 + 1);
					else
						pkNewItem->SetEvolution(item->GetEvolution() + 1);
				}
				else if (scroll->GetVnum() == 53146)
				{
					if (item->GetEvolution() == 0)
						pkNewItem->SetEvolution(300 + 1);
					else
						pkNewItem->SetEvolution(item->GetEvolution() + 1);
				}

				BYTE bCell = item->GetCell();

				ITEM_MANAGER::instance().RemoveItem(item, "REMOVE (REFINE SUCCESS)");
				pkNewItem->__ADD_TO_CHARACTER(this, TItemPos(INVENTORY, bCell));
				ITEM_MANAGER::instance().FlushDelayedSave(pkNewItem);

				int i = 0;
				for (; i < _countof(rrNeedItems); ++i)
				{
					if (bElement)
					{
						if ((itemEvo % 20) >= 10)
						{
							RemoveSpecifyItem(rrNeedItems[i], 50);
						}
						else
						{
							RemoveSpecifyItem(rrNeedItems[i], 25);
						}
					}
					else
					{
						if ((itemEvo % 20) >= 10)
						{
							RemoveSpecifyItem(rrNeedItems[i], 10);
						}
						else
						{
							RemoveSpecifyItem(rrNeedItems[i], 5);
						}
					}
				}

				PayRefineFee(rrNeedMoney[itemEvo]);
				ChatPacket(CHAT_TYPE_COMMAND, "RefineSuceeded");
#ifdef __SMITH_EFFECT__
				EffectPacket(SE_FR_SUCCESS);
#endif
			}
		}
		else
		{
			ChatPacket(CHAT_TYPE_COMMAND, "RefineFailed");
#ifdef __SMITH_EFFECT__
			EffectPacket(SE_FR_FAIL);
#endif
			PayRefineFee(rrNeedMoney[itemEvo]);
		}
		scroll->SetCount(scroll->GetCount() - 1);
	}

	return true;
}
#endif

bool CHARACTER::RefineInformation(BYTE bCell, BYTE bType, int iAdditionalCell)
{
	if (bCell > INVENTORY_MAX_NUM)
		return false;

	LPITEM item = GetInventoryItem(bCell);

	if (!item)
		return false;

	// REFINE_COST
	if (bType == REFINE_TYPE_MONEY_ONLY && !GetQuestFlag("deviltower_zone.can_refine"))
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("»ç±Í Å¸¿ö ¿Ï·á º¸»óÀº ÇÑ¹ø±îÁö »ç¿ë°¡´ÉÇÕ´Ï´Ù."));
		return false;
	}
	// END_OF_REFINE_COST

	TPacketGCRefineInformation p;

	p.header = HEADER_GC_REFINE_INFORMATION;
	p.pos = bCell;
	p.src_vnum = item->GetVnum();
	p.result_vnum = item->GetRefinedVnum();
	p.type = bType;

	if (p.result_vnum == 0)
	{
		sys_err("RefineInformation p.result_vnum == 0");
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ ¾ÆÀÌÅÛÀº °³·®ÇÒ ¼ö ¾ø½À´Ï´Ù."));
		return false;
	}

#ifdef __REFINE_REWORK__
	LPITEM itemScroll = GetInventoryItem(iAdditionalCell);
	if (bType != 0)
	{
		if (!itemScroll || item->GetVnum() == itemScroll->GetVnum())
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("°°Àº °³·®¼­¸¦ ÇÕÄ¥ ¼ö´Â ¾ø½À´Ï´Ù."));
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Ãàº¹ÀÇ ¼­¿Í ÇöÃ¶À» ÇÕÄ¥ ¼ö ÀÖ½À´Ï´Ù."));
			return false;
		}
	}
#endif

	if (item->GetType() == ITEM_USE && item->GetSubType() == USE_TUNING)
	{
		if (bType == 0)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ ¾ÆÀÌÅÛÀº ÀÌ ¹æ½ÄÀ¸·Î´Â °³·®ÇÒ ¼ö ¾ø½À´Ï´Ù."));
			return false;
		}
		else
		{
#ifndef __REFINE_REWORK__
			LPITEM itemScroll = GetInventoryItem(iAdditionalCell);
			if (!itemScroll || item->GetVnum() == itemScroll->GetVnum())
			{
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("°°Àº °³·®¼­¸¦ ÇÕÄ¥ ¼ö´Â ¾ø½À´Ï´Ù."));
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Ãàº¹ÀÇ ¼­¿Í ÇöÃ¶À» ÇÕÄ¥ ¼ö ÀÖ½À´Ï´Ù."));
				return false;
			}
#endif
		}
	}

	CRefineManager& rm = CRefineManager::instance();

	const TRefineTable* prt = rm.GetRefineRecipe(item->GetRefineSet());

	if (!prt)
	{
		sys_err("RefineInformation NOT GET REFINE SET %d", item->GetRefineSet());
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ ¾ÆÀÌÅÛÀº °³·®ÇÒ ¼ö ¾ø½À´Ï´Ù."));
		return false;
	}

	// REFINE_COST

	//MAIN_QUEST_LV7
	if (GetQuestFlag("main_quest_lv7.refine_chance") > 0)
	{
		if (!item->CheckItemUseLevel(20) || item->GetType() != ITEM_WEAPON)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¹«·á °³·® ±âÈ¸´Â 20 ÀÌÇÏÀÇ ¹«±â¸¸ °¡´ÉÇÕ´Ï´Ù"));
			return false;
		}
		p.cost = 0;
	}
	else
		p.cost = ComputeRefineFee(prt->cost);

	//END_MAIN_QUEST_LV7
	p.prob = prt->prob;
#ifdef __REFINE_REWORK__
	if (bType != 0)
	{
		p.success_prob = itemScroll ? itemScroll->GetValue(2) : 0;
		if (itemScroll && itemScroll->GetValue(2) > 100)
			p.success_prob = 100;
	}
	else
		p.success_prob = 0;

	CSkillProto* pkSk = CSkillManager::instance().Get(SKILL_REFINE);
	if (NULL != pkSk)
	{
		pkSk->SetPointVar("k", 1.0f * GetSkillPower(SKILL_REFINE) / 100.0f);
		p.success_prob += static_cast<int>(pkSk->kPointPoly.Eval());
	}
#endif
	if (CGameEventsManager::instance().IsActivateEvent(EVENT_UPGRADE) == true)
		p.success_prob += 10;

	if (bType == REFINE_TYPE_MONEY_ONLY)
	{
		p.material_count = 0;
		memset(p.materials, 0, sizeof(p.materials));
	}
	else
	{
		p.material_count = prt->material_count;
		thecore_memcpy(&p.materials, prt->materials, sizeof(prt->materials));
	}
	// END_OF_REFINE_COST

	GetDesc()->Packet(&p, sizeof(TPacketGCRefineInformation));

	SetRefineMode(iAdditionalCell);
	return true;
}

bool CHARACTER::RefineItem(LPITEM pkItem, LPITEM pkTarget)
{
	if (!CanHandleItem())
		return false;

	if (pkItem->GetSubType() == USE_TUNING)
	{
		//if (pkTarget->IsAccessoryForSocket() && pkTarget->GetRefineLevel() >= 9 && pkTarget->GetAccessorySocketGrade() != 0)
		//{
		//	ChatPacket(CHAT_TYPE_INFO, LC_TEXT("DONUSUMDE_CEVHERLERIN_SILINECEK_LUTFEN_CEVHERLERI_CIKAR"));
		//	return false;
		//}
		// MUSIN_SCROLL
		if (pkItem->GetValue(0) == MUSIN_SCROLL)
			RefineInformation(pkTarget->GetCell(), REFINE_TYPE_MUSIN, pkItem->GetCell());
		// END_OF_MUSIN_SCROLL
		else if (pkItem->GetValue(0) == HYUNIRON_CHN)
			RefineInformation(pkTarget->GetCell(), REFINE_TYPE_HYUNIRON, pkItem->GetCell());
		else if (pkItem->GetValue(0) == BDRAGON_SCROLL)
		{
			if (pkTarget->GetRefineSet() != 702) return false;
			RefineInformation(pkTarget->GetCell(), REFINE_TYPE_BDRAGON, pkItem->GetCell());
		}
#ifdef __REFINE_SCROLL__
		else if (pkItem->GetValue(0) == PLUS_SCROLL)
		{
			RefineInformation(pkTarget->GetCell(), REFINE_TYPE_PLUS_SCROLL, pkItem->GetCell());
		}
#endif
		else
		{
			if (pkTarget->GetRefineSet() == 501) return false;
			RefineInformation(pkTarget->GetCell(), REFINE_TYPE_SCROLL, pkItem->GetCell());
		}
	}
	else if (pkItem->GetSubType() == USE_DETACHMENT && IS_SET(pkTarget->GetFlag(), ITEM_FLAG_REFINEABLE) && pkItem->GetVnum() == DETACH_NORMAL)
	{
		bool bHasMetinStone = false;

		if (pkTarget->GetSubType() == ARMOR_PENDANT)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("TILSIMDA_DIGERINI_KULLAN_ANASINI_SIKTIGIM"));
			return false;
		}

		for (int i = 0; i < ITEM_SOCKET_MAX_NUM; i++)
		{
			long socket = pkTarget->GetSocket(i);
			if (socket > 2 && socket != ITEM_BROKEN_METIN_VNUM)
			{
				bHasMetinStone = true;
				break;
			}
		}

		if (bHasMetinStone)
		{
			for (int i = 0; i < ITEM_SOCKET_MAX_NUM; ++i)
			{
				long socket = pkTarget->GetSocket(i);
				if (socket > 2 && socket != ITEM_BROKEN_METIN_VNUM)
				{
					AutoGiveItem(socket);
					//TItemTable* pTable = ITEM_MANAGER::instance().GetTable(pkTarget->GetSocket(i));
					//pkTarget->SetSocket(i, pTable->alValues[2]);

					pkTarget->SetSocket(i, ITEM_BROKEN_METIN_VNUM);
				}
			}
			pkItem->SetCount(pkItem->GetCount() - 1);
			return true;
		}
		else
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("»©³¾ ¼ö ÀÖ´Â ¸ÞÆ¾¼®ÀÌ ¾ø½À´Ï´Ù."));
			return false;
		}
	}
	else if (pkItem->GetSubType() == USE_DETACHMENT && pkTarget->IsDeattachAccessoryForSocket() && pkItem->GetVnum() == DETACH_ACCESORY)
	{
		int curCount = pkTarget->GetAccessorySocketGrade();

		if (curCount < 1)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("»©³¾ ¼ö ÀÖ´Â ¸ÞÆ¾¼®ÀÌ ¾ø½À´Ï´Ù."));
			return false;
		}

		DWORD putItemVnum = 0;

		if (pkTarget->GetType() == ITEM_BELT)
		{
			if (pkTarget->GetSocket(3) == 99)
				putItemVnum = PUT_BELT_SOCKET_ITEM_PERMA;
			else
				putItemVnum = PUT_BELT_SOCKET_ITEM;
		}
#ifdef __NEW_ACCESORY_ITEMS__
		else if (pkTarget->GetSubType() == ARMOR_HEAD)
		{
			if (pkTarget->GetSocket(3) == 99)
				putItemVnum = PUT_HEAD_SOCKET_ITEM_PERMA;
			else
				putItemVnum = PUT_HEAD_SOCKET_ITEM;
		}
		else if (pkTarget->GetSubType() == ARMOR_SHIELD)
		{
			if (pkTarget->GetSocket(3) == 99)
				putItemVnum = PUT_SHIELD_SOCKET_ITEM_PERMA;
			else
				putItemVnum = PUT_SHIELD_SOCKET_ITEM;
		}
		else if (pkTarget->GetSubType() == ARMOR_FOOTS)
		{
			if (pkTarget->GetSocket(3) == 99)
				putItemVnum = PUT_FOOTS_SOCKET_ITEM_PERMA;
			else
				putItemVnum = PUT_FOOTS_SOCKET_ITEM;
		}
		else if (pkTarget->GetSubType() == ARMOR_PENDANT)
		{
			if (pkTarget->GetSocket(3) == 99)
				putItemVnum = PUT_PENDANT_SOCKET_ITEM_PERMA;
			else
				putItemVnum = PUT_PENDANT_SOCKET_ITEM;
		}
#endif // __NEW_ACCESORY_ITEMS__
		else
		{
			if (pkTarget->GetSocket(3) == 99)
				putItemVnum = pkTarget->GetPutItemVnumPerma();
			else
				putItemVnum = pkTarget->GetPutItemVnum();
		}

		if (!putItemVnum)
			return false;

		pkTarget->SetAccessorySocketGrade(0); // clear sockets
		pkTarget->StopAccessorySocketExpireEvent(); // stop expire event
		pkTarget->SetAccessorySocketDownGradeTime(0); // reset ecpire time
		pkTarget->SetSocket(3, 0); // setunperma

		AutoGiveItem(putItemVnum, curCount); // give socket item
		pkItem->SetCount(pkItem->GetCount() - 1); // remove detach item
		return true;
	}

	return false;
}

EVENTFUNC(kill_campfire_event)
{
	char_event_info* info = dynamic_cast<char_event_info*>(event->info);

	if (info == NULL)
	{
		sys_err("kill_campfire_event> <Factor> Null pointer");
		return 0;
	}

	LPCHARACTER	ch = info->ch;

	if (ch == NULL) { // <Factor>
		return 0;
	}
	ch->m_pkMiningEvent = NULL;
	M2_DESTROY_CHARACTER(ch);
	return 0;
}

bool CHARACTER::GiveRecallItem(LPITEM item)
{
	int idx = GetMapIndex();
	int iEmpireByMapIndex = -1;

	if (idx < 20)
		iEmpireByMapIndex = 1;
	else if (idx < 40)
		iEmpireByMapIndex = 2;
	else if (idx < 60)
		iEmpireByMapIndex = 3;
	else if (idx < 10000)
		iEmpireByMapIndex = 0;

	switch (idx)
	{
	case 66:
	case 216:
		iEmpireByMapIndex = -1;
		break;
	}

	if (iEmpireByMapIndex && GetEmpire() != iEmpireByMapIndex)
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("±â¾ïÇØ µÑ ¼ö ¾ø´Â À§Ä¡ ÀÔ´Ï´Ù."));
		return false;
	}

	int pos;

	if (item->GetCount() == 1)
	{
		item->SetSocket(0, GetX());
		item->SetSocket(1, GetY());
	}
	else if ((pos = GetEmptyInventory(item->GetSize())) != -1)
	{
		LPITEM item2 = ITEM_MANAGER::instance().CreateItem(item->GetVnum(), 1);

		if (NULL != item2)
		{
			item2->SetSocket(0, GetX());
			item2->SetSocket(1, GetY());
			item2->__ADD_TO_CHARACTER(this, TItemPos(INVENTORY, pos));

			item->SetCount(item->GetCount() - 1);
		}
	}
	else
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼ÒÁöÇ°¿¡ ºó °ø°£ÀÌ ¾ø½À´Ï´Ù."));
		return false;
	}

	return true;
}

void CHARACTER::ProcessRecallItem(LPITEM item)
{
	int idx;

	if ((idx = SECTREE_MANAGER::instance().GetMapIndex(item->GetSocket(0), item->GetSocket(1))) == 0)
		return;

	int iEmpireByMapIndex = -1;

	if (idx < 20)
		iEmpireByMapIndex = 1;
	else if (idx < 40)
		iEmpireByMapIndex = 2;
	else if (idx < 60)
		iEmpireByMapIndex = 3;
	else if (idx < 10000)
		iEmpireByMapIndex = 0;

	switch (idx)
	{
	case 66:
	case 216:
		iEmpireByMapIndex = -1;
		break;

	case 301:
	case 302:
	case 303:
	case 304:
		if (GetLevel() < 90)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¾ÆÀÌÅÛÀÇ ·¹º§ Á¦ÇÑº¸´Ù ·¹º§ÀÌ ³·½À´Ï´Ù."));
			return;
		}
		else
			break;
	}

	if (iEmpireByMapIndex && GetEmpire() != iEmpireByMapIndex)
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("±â¾ïµÈ À§Ä¡°¡ Å¸Á¦±¹¿¡ ¼ÓÇØ ÀÖ¾î¼­ ±ÍÈ¯ÇÒ ¼ö ¾ø½À´Ï´Ù."));
		item->SetSocket(0, 0);
		item->SetSocket(1, 0);
	}
	else
	{
		sys_log(1, "Recall: %s %d %d -> %d %d", GetName(), GetX(), GetY(), item->GetSocket(0), item->GetSocket(1));
		WarpSet(item->GetSocket(0), item->GetSocket(1));
		item->SetCount(item->GetCount() - 1);
	}
}

void CHARACTER::__OpenPrivateShop()
{
	// UNUSED
	// Eger karakterin uzerinde zirh var ise normal dukkan acilmiyordu
	// Bununla alakali duzenleme yapildi artik acilabiliyor.
	ChatPacket(CHAT_TYPE_COMMAND, "OpenPrivateShop");
}

void CHARACTER::UseSilkBotary()
{
	__OpenPrivateShop();
}

int CalculateConsume(LPCHARACTER ch)
{
	static const int WARP_NEED_LIFE_PERCENT = 30;
	static const int WARP_MIN_LIFE_PERCENT = 10;
	// CONSUME_LIFE_WHEN_USE_WARP_ITEM
	int consumeLife = 0;
	{
		// CheckNeedLifeForWarp
		const HPTYPE curLife = ch->GetHP();
		const int needPercent = WARP_NEED_LIFE_PERCENT;
		const HPTYPE needLife = ch->GetMaxHP() * needPercent / 100;
		if (curLife < needLife)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("³²Àº »ý¸í·Â ¾çÀÌ ¸ðÀÚ¶ó »ç¿ëÇÒ ¼ö ¾ø½À´Ï´Ù."));
			return -1;
		}

		consumeLife = needLife;

		const int minPercent = WARP_MIN_LIFE_PERCENT;
		const HPTYPE minLife = ch->GetMaxHP() * minPercent / 100;
		if (curLife - needLife < minLife)
			consumeLife = curLife - minLife;

		if (consumeLife < 0)
			consumeLife = 0;
	}
	// END_OF_CONSUME_LIFE_WHEN_USE_WARP_ITEM
	return consumeLife;
}

int CalculateConsumeSP(LPCHARACTER lpChar)
{
	static const int NEED_WARP_SP_PERCENT = 30;

	const int curSP = lpChar->GetSP();
	const int needSP = lpChar->GetMaxSP() * NEED_WARP_SP_PERCENT / 100;

	if (curSP < needSP)
	{
		lpChar->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("³²Àº Á¤½Å·Â ¾çÀÌ ¸ðÀÚ¶ó »ç¿ëÇÒ ¼ö ¾ø½À´Ï´Ù."));
		return -1;
	}

	return needSP;
}

#ifdef __USE_ITEM_COUNT__
bool CHARACTER::UseItemEx(LPITEM item, TItemPos DestCell, DWORD count)
#else // __USE_ITEM_COUNT__
bool CHARACTER::UseItemEx(LPITEM item, TItemPos DestCell)
#endif // __USE_ITEM_COUNT__
{
	int iLimitRealtimeStartFirstUseFlagIndex = -1;
	//int iLimitTimerBasedOnWearFlagIndex = -1;

	WORD wDestCell = DestCell.cell;
	BYTE bDestInven = DestCell.window_type;
	for (int i = 0; i < ITEM_LIMIT_MAX_NUM; ++i)
	{
		long limitValue = item->GetProto()->aLimits[i].lValue;

		switch (item->GetProto()->aLimits[i].bType)
		{
		case LIMIT_LEVEL:
			if (GetLevel() < limitValue)
			{
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¾ÆÀÌÅÛÀÇ ·¹º§ Á¦ÇÑº¸´Ù ·¹º§ÀÌ ³·½À´Ï´Ù."));
				return false;
			}
			break;

		case LIMIT_REAL_TIME_START_FIRST_USE:
			iLimitRealtimeStartFirstUseFlagIndex = i;
			break;

		case LIMIT_TIMER_BASED_ON_WEAR:
			//iLimitTimerBasedOnWearFlagIndex = i;
			break;
		}
	}

	if (test_server)
		sys_log(0, "USE_ITEM %s, Inven %d, Cell %d, ItemType %d, SubType %d", item->GetName(), bDestInven, wDestCell, item->GetType(), item->GetSubType());

	if (!IsLoadedAffect())
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Affects are not loaded yet!"));
		return false;
	}

	// @duzenleme
	// kemer envanteri ile alakali kontroller.
	// TODO: kemer envanterinin silinmesi daha uygun olur.
	if (TItemPos(item->GetWindow(), item->GetCell()).IsBeltInventoryPosition())
	{
		LPITEM beltItem = GetWear(WEAR_BELT);

		if (NULL == beltItem)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<Belt> You can't use this item if you have no equipped belt."));
			return false;
		}

		if (false == CBeltInventoryHelper::IsAvailableCell(item->GetCell() - BELT_INVENTORY_SLOT_START, beltItem->GetValue(0)))
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<Belt> You can't use this item if you don't upgrade your belt."));
			return false;
		}
	}

#ifdef __BATTLE_PASS_SYSTEM__
	BYTE bBattlePassId = GetBattlePassId();
	if (bBattlePassId)
	{
		DWORD dwItemVnum, dwUseCount;
		if (CBattlePass::instance().BattlePassMissionGetInfo(bBattlePassId, USE_ITEM, &dwItemVnum, &dwUseCount))
		{
			if (dwItemVnum == item->GetVnum() && GetMissionProgress(USE_ITEM, bBattlePassId) < dwUseCount)
				UpdateMissionProgress(USE_ITEM, bBattlePassId, 1, dwUseCount);
		}
	}
#endif

	if (-1 != iLimitRealtimeStartFirstUseFlagIndex)
	{
		if (0 == item->GetSocket(1))
		{
			long duration = (0 != item->GetSocket(0)) ? item->GetSocket(0) : item->GetProto()->aLimits[iLimitRealtimeStartFirstUseFlagIndex].lValue;

			if (0 == duration)
				duration = 60 * 60 * 24 * 7;

			item->SetSocket(0, time(0) + duration);
			item->StartRealTimeExpireEvent();
		}

		if (false == item->IsEquipped())
			item->SetSocket(1, item->GetSocket(1) + 1);
	}

#ifdef __GROWTH_PET_SYSTEM__
	else if (item->GetVnum() >= 800 && item->GetVnum() <= 803)
	{
		CNewPetSystem* petSystem;
		if (!(petSystem = GetNewPetSystem()))
			return false;

		CNewPetActor* activePet;
		if (!(activePet = petSystem->GetSummoned()))
			return false;

		if (quest::CQuestManager::instance().GetPCForce(GetPlayerID())->IsRunning() == true)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_OPERATION_IS_FAILED_QUEST_WINDOW_IS_OPENED"));
			return false;
		}

		if (!CanAct(true, true, VAR_TIME_PET_ACTION, nullptr, 0))
			return false;

		if (activePet->GetLevel() >= 157)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("SADECE_157_ALTI"));
			return false;
		}

		bool handled = false;

		switch (item->GetVnum())
		{
		case 800:
			if (activePet->GetExp() < activePet->GetNextExpFromMob())
			{
				if (activePet->SetExp(activePet->GetNextExpFromMob() * 0.6, 0))
				{
					item->SetCount(item->GetCount() - 1);
					handled = true;
				}
			}
			break;
		case 801:
			if (activePet->GetExp() < activePet->GetNextExpFromMob())
			{
				if (activePet->SetExp(activePet->GetNextExpFromMob(), 0))
				{
					item->SetCount(item->GetCount() - 1);
					handled = true;
				}
			}
			break;
		case 802:
			if (activePet->GetExpI() < activePet->GetNextExpFromItem())
			{
				if (activePet->SetExp(activePet->GetNextExpFromItem() * 0.6, 1))
				{
					item->SetCount(item->GetCount() - 1);
					handled = true;
				}
			}
			break;
		case 803:
			if (activePet->GetExpI() < activePet->GetNextExpFromItem())
			{
				if (activePet->SetExp(activePet->GetNextExpFromItem(), 1))
				{
					item->SetCount(item->GetCount() - 1);
					handled = true;
				}
			}
			break;

		default:
			break;
		}

		if (!handled)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("DAHA_FAZLA_NESNE_YEDIREMEZSIN"));
		}

		return true;
	}
	else if (item->GetVnum() >= 850 && item->GetVnum() <= 853)
	{
		CNewPetSystem* petSystem;
		if (!(petSystem = GetNewPetSystem()))
			return false;

		CNewPetActor* activePet;
		if (!(activePet = petSystem->GetSummoned()))
			return false;

		if (quest::CQuestManager::instance().GetPCForce(GetPlayerID())->IsRunning() == true)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_OPERATION_IS_FAILED_QUEST_WINDOW_IS_OPENED"));
			return false;
		}

		if (!CanAct(true, true, VAR_TIME_PET_ACTION, nullptr, 0))
			return false;

		if (activePet->GetLevel() < 157)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("SADECE_157_USTU"));
			return false;
		}

		bool handled = false;

		switch (item->GetVnum())
		{
		case 850:
			if (activePet->GetExp() < activePet->GetNextExpFromMob())
			{
				if (activePet->SetExp(activePet->GetNextExpFromMob() * 0.6, 0))
				{
					item->SetCount(item->GetCount() - 1);
					handled = true;
				}
			}
			break;
		case 851:
			if (activePet->GetExp() < activePet->GetNextExpFromMob())
			{
				if (activePet->SetExp(activePet->GetNextExpFromMob(), 0))
				{
					item->SetCount(item->GetCount() - 1);
					handled = true;
				}
			}
			break;
		case 852:
			if (activePet->GetExpI() < activePet->GetNextExpFromItem())
			{
				if (activePet->SetExp(activePet->GetNextExpFromItem() * 0.6, 1))
				{
					item->SetCount(item->GetCount() - 1);
					handled = true;
				}
			}
			break;
		case 853:
			if (activePet->GetExpI() < activePet->GetNextExpFromItem())
			{
				if (activePet->SetExp(activePet->GetNextExpFromItem(), 1))
				{
					item->SetCount(item->GetCount() - 1);
					handled = true;
				}
			}
			break;

		default:
			break;
		}

		if (!handled)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("DAHA_FAZLA_NESNE_YEDIREMEZSIN"));
		}

		return true;
	}
	if ((item->GetVnum() >= 55010 && item->GetVnum() <= 55016))
	{
		int skill = item->GetValue(0);
		CNewPetSystem* petSystem;
		if (!(petSystem = GetNewPetSystem()))
			return false;

		if (quest::CQuestManager::instance().GetPCForce(GetPlayerID())->IsRunning() == true)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_OPERATION_IS_FAILED_QUEST_WINDOW_IS_OPENED"));
			return false;
		}

		if (!CanAct(true, true, VAR_TIME_PET_ACTION, nullptr, 0))
			return false;

		if (petSystem->IsActivePet() && petSystem->GetLevel() < 40)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Petin leveli henüz hazýr deðil."));
			return false;
		}

		bool ret = petSystem->IncreasePetSkill(skill);
		if (ret && petSystem->IsActivePet())
			item->SetCount(item->GetCount() - 1);
		else
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("PET_WITH_PET_SUMMON"));
		return true;
	}
	if (item->GetVnum() == 55001)
	{
		LPITEM item2;

		if (!IsValidItemPosition(DestCell) || !(item2 = GetItem(DestCell)))
			return false;

		if (item2->IsExchanging())
			return false;

		if (quest::CQuestManager::instance().GetPCForce(GetPlayerID())->IsRunning() == true)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_OPERATION_IS_FAILED_QUEST_WINDOW_IS_OPENED"));
			return false;
		}

		if (!CanAct(true, true, VAR_TIME_PET_ACTION, nullptr, 0))
			return false;

		char szQuery1[1024];
		snprintf(szQuery1, sizeof(szQuery1), "SELECT duration,tduration FROM new_petsystem WHERE id = %u;", item2->GetID());
		std::unique_ptr<SQLMsg> pmsg2(DBManager::instance().DirectQuery(szQuery1));
		if (pmsg2->Get()->uiNumRows > 0)
		{
			MYSQL_ROW row = mysql_fetch_row(pmsg2->Get()->pSQLResult);
			int suankiDuration = atoi(row[0]);

			if (suankiDuration >= get_global_time())
			{
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("PET_ALREADY_YOUNG"));
				return false;
			}

			int insertduration0 = time(0) + atoi(row[1]);
			int insertduration1 = time(0) + (atoi(row[2]) / 2);
			if (atoi(row[0]) > 0)
			{
				std::unique_ptr<SQLMsg> pMsg(DBManager::instance().DirectQuery("UPDATE new_petsystem SET duration = %d WHERE id = %u;", insertduration0, item2->GetID()));
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("PSS_PROTEIN_D1"));
			}
			else
			{
				std::unique_ptr<SQLMsg> pMsg(DBManager::instance().DirectQuery("UPDATE new_petsystem SET duration = %d WHERE id = %u;", insertduration1, item2->GetID()));
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("PSS_PROTEIN_D2"));
			}

			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("PET_PROTEIN_INFO"));
			item->SetCount(item->GetCount() - 1);
			return true;
		}

		return false;
	}
	if (item->GetVnum() == 55101)
	{
		LPITEM item2;

		if (!IsValidItemPosition(DestCell) || !(item2 = GetItem(DestCell)))
			return false;

		if (item2->IsExchanging())
			return false;

		if (quest::CQuestManager::instance().GetPCForce(GetPlayerID())->IsRunning() == true)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_OPERATION_IS_FAILED_QUEST_WINDOW_IS_OPENED"));
			return false;
		}

		if (!CanAct(true, true, VAR_TIME_PET_ACTION, nullptr, 0))
			return false;

		char szQuery1[1024];
		snprintf(szQuery1, sizeof(szQuery1), "SELECT duration,tduration FROM new_petsystem WHERE id = %u;", item2->GetID());
		std::unique_ptr<SQLMsg> pmsg2(DBManager::instance().DirectQuery(szQuery1));
		if (pmsg2->Get()->uiNumRows > 0)
		{
			MYSQL_ROW row = mysql_fetch_row(pmsg2->Get()->pSQLResult);
			int suankiDuration = atoi(row[0]);

			if (suankiDuration >= get_global_time())
			{
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("PET_ALREADY_YOUNG"));
				return false;
			}

			int insertduration0 = time(0) + atoi(row[1]) * 2;
			int insertduration1 = time(0) + (atoi(row[2]));
			if (atoi(row[0]) > 0)
			{
				std::unique_ptr<SQLMsg> pMsg(DBManager::instance().DirectQuery("UPDATE new_petsystem SET duration = %d WHERE id = %u;", insertduration0, item2->GetID()));
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("PSS_PROTEIN_D1"));
			}
			else
			{
				std::unique_ptr<SQLMsg> pMsg(DBManager::instance().DirectQuery("UPDATE new_petsystem SET duration = %d WHERE id = %u;", insertduration1, item2->GetID()));
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("PSS_PROTEIN_D2"));
			}

			item->SetCount(item->GetCount() - 1);
			return true;
		}

		return false;
	}

	if (item->GetVnum() == 19826)
	{
		LPITEM item2;

		if (!IsValidItemPosition(DestCell) || !(item2 = GetItem(DestCell)))
			return false;

		if (item2->IsExchanging())
			return false;

		if (quest::CQuestManager::instance().GetPCForce(GetPlayerID())->IsRunning() == true)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_OPERATION_IS_FAILED_QUEST_WINDOW_IS_OPENED"));
			return false;
		}

		if (!CanAct(true, true, VAR_TIME_PET_ACTION, nullptr, 0))
			return false;

		if (GetNewPetSystem()->IsActivePet())
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("AKTIF_BIR_PETI_EFSUNLAYAMAZSIN."));
			return false;
		}

		if (item2->GetAttributeType(0) != 1)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("PETI_SADECE_1_LVLDE_EFSUNLAYABILIRSIN."));
			return false;
		}

		std::unique_ptr<SQLMsg> pMsg(DBManager::instance().DirectQuery("UPDATE new_petsystem SET bonus0 = '%d', bonus1 = '%d', bonus2 = '%d' WHERE id = '%lu';", 45, 45, 45, item2->GetID()));

		item2->SetForceAttribute(0, item2->GetAttributeType(0), 45);
		item2->SetForceAttribute(1, item2->GetAttributeType(1), 45);
		item2->SetForceAttribute(2, item2->GetAttributeType(2), 45);

		item->SetCount(item->GetCount() - 1);
		return true;
	}

	if (item->GetVnum() == 19825)
	{
		LPITEM item2;

		if (!IsValidItemPosition(DestCell) || !(item2 = GetItem(DestCell)))
			return false;

		if (item2->IsExchanging())
			return false;

		if (quest::CQuestManager::instance().GetPCForce(GetPlayerID())->IsRunning() == true)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_OPERATION_IS_FAILED_QUEST_WINDOW_IS_OPENED"));
			return false;
		}

		if (!CanAct(true, true, VAR_TIME_PET_ACTION, nullptr, 0))
			return false;

		if (GetNewPetSystem()->IsActivePet())
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("AKTIF_BIR_PETI_EFSUNLAYAMAZSIN."));
			return false;
		}

		if (item2->GetAttributeType(0) != 1)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("PET_BELIRLEME_1_LEVELDE_KULLANILABILIR."));
			return false;
		}

		std::unique_ptr<SQLMsg> pMsg(DBManager::instance().DirectQuery("UPDATE new_petsystem SET pet_type = '%d' WHERE id = '%lu';", 7, item2->GetID()));

		item2->SetForceAttribute(1, 7, item->GetAttributeValue(1));

		item->SetCount(item->GetCount() - 1);
		return true;
	}

	if (item->GetVnum() == 55043)
	{
		LPITEM item2;

		if (!IsValidItemPosition(DestCell) || !(item2 = GetItem(DestCell)))
			return false;

		if (item2->IsExchanging())
			return false;

		if (quest::CQuestManager::instance().GetPCForce(GetPlayerID())->IsRunning() == true)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_OPERATION_IS_FAILED_QUEST_WINDOW_IS_OPENED"));
			return false;
		}

		if (!CanAct(true, true, VAR_TIME_PET_ACTION, nullptr, 0))
			return false;

		if (GetNewPetSystem()->IsActivePet())
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("AKTIF_BIR_PETI_EFSUNLAYAMAZSIN."));
			return false;
		}

		if (item2->GetAttributeType(0) != 250)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("PETI_SADECE_250_LVLDE_EFSUNLAYABILIRSIN."));
			return false;
		}

		if (item2->GetAttributeType(1) != 7)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("SADECE_PRIZMATIKTE"));
			return false;
		}

		int ozellik1 = number(450, 900);
		int ozellik2 = number(450, 900);
		int ozellik3 = number(450, 900);

		std::unique_ptr<SQLMsg> pMsg(DBManager::instance().DirectQuery("UPDATE new_petsystem SET bonus0 = '%d', bonus1 = '%d', bonus2 = '%d' WHERE id = '%lu';", ozellik1, ozellik2, ozellik3, item2->GetID()));

		item2->SetForceAttribute(0, item2->GetAttributeType(0), ozellik1);
		item2->SetForceAttribute(1, item2->GetAttributeType(1), ozellik2);
		item2->SetForceAttribute(2, item2->GetAttributeType(2), ozellik3);

		item->SetCount(item->GetCount() - 1);
		return true;
	}

	if (item->GetVnum() == 55054)
	{
		LPITEM item2;

		if (!IsValidItemPosition(DestCell) || !(item2 = GetItem(DestCell)))
			return false;

		if (item2->IsExchanging())
			return false;

		if (quest::CQuestManager::instance().GetPCForce(GetPlayerID())->IsRunning() == true)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_OPERATION_IS_FAILED_QUEST_WINDOW_IS_OPENED"));
			return false;
		}

		if (!CanAct(true, true, VAR_TIME_PET_ACTION, nullptr, 0))
			return false;

		if (GetNewPetSystem()->IsActivePet())
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("AKTIF_BIR_PETI_EFSUNLAYAMAZSIN."));
			return false;
		}

		if (item2->GetAttributeType(0) != 250)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("PETI_SADECE_250_LVLDE_EFSUNLAYABILIRSIN."));
			return false;
		}

		if (item2->GetAttributeType(1) != 7)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("SADECE_PRIZMATIKTE"));
			return false;
		}

		if (item2->GetAttributeValue(0) < 450)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Kýymetli dostum bir öncekini atmadan bunu atamazsýn."));
			return false;
		}

		int ozellik1 = number(900, 1350);
		int ozellik2 = number(900, 1350);
		int ozellik3 = number(900, 1350);

		std::unique_ptr<SQLMsg> pMsg(DBManager::instance().DirectQuery("UPDATE new_petsystem SET bonus0 = '%d', bonus1 = '%d', bonus2 = '%d' WHERE id = '%lu';", ozellik1, ozellik2, ozellik3, item2->GetID()));

		item2->SetForceAttribute(0, item2->GetAttributeType(0), ozellik1);
		item2->SetForceAttribute(1, item2->GetAttributeType(1), ozellik2);
		item2->SetForceAttribute(2, item2->GetAttributeType(2), ozellik3);

		item->SetCount(item->GetCount() - 1);
		return true;
	}

	if (item->GetVnum() == 55055)
	{
		LPITEM item2;

		if (!IsValidItemPosition(DestCell) || !(item2 = GetItem(DestCell)))
			return false;

		if (item2->IsExchanging())
			return false;

		if (quest::CQuestManager::instance().GetPCForce(GetPlayerID())->IsRunning() == true)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_OPERATION_IS_FAILED_QUEST_WINDOW_IS_OPENED"));
			return false;
		}

		if (!CanAct(true, true, VAR_TIME_PET_ACTION, nullptr, 0))
			return false;

		if (GetNewPetSystem()->IsActivePet())
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("AKTIF_BIR_PETI_EFSUNLAYAMAZSIN."));
			return false;
		}

		if (item2->GetAttributeType(0) != 250)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("PETI_SADECE_250_LVLDE_EFSUNLAYABILIRSIN."));
			return false;
		}

		if (item2->GetAttributeType(1) != 7)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("SADECE_PRIZMATIKTE"));
			return false;
		}

		if (item2->GetAttributeValue(0) < 900)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Kýymetli dostum bir öncekini atmadan bunu atamazsýn."));
			return false;
		}

		int ozellik1 = number(1350, 1800);
		int ozellik2 = number(1350, 1800);
		int ozellik3 = number(1350, 1800);

		std::unique_ptr<SQLMsg> pMsg(DBManager::instance().DirectQuery("UPDATE new_petsystem SET bonus0 = '%d', bonus1 = '%d', bonus2 = '%d' WHERE id = '%lu';", ozellik1, ozellik2, ozellik3, item2->GetID()));

		item2->SetForceAttribute(0, item2->GetAttributeType(0), ozellik1);
		item2->SetForceAttribute(1, item2->GetAttributeType(1), ozellik2);
		item2->SetForceAttribute(2, item2->GetAttributeType(2), ozellik3);

		item->SetCount(item->GetCount() - 1);
		return true;
	}

	if (item->GetVnum() == 55056)
	{
		LPITEM item2;

		if (!IsValidItemPosition(DestCell) || !(item2 = GetItem(DestCell)))
			return false;

		if (item2->IsExchanging())
			return false;

		if (quest::CQuestManager::instance().GetPCForce(GetPlayerID())->IsRunning() == true)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_OPERATION_IS_FAILED_QUEST_WINDOW_IS_OPENED"));
			return false;
		}

		if (!CanAct(true, true, VAR_TIME_PET_ACTION, nullptr, 0))
			return false;

		if (GetNewPetSystem()->IsActivePet())
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("AKTIF_BIR_PETI_EFSUNLAYAMAZSIN."));
			return false;
		}

		if (item2->GetAttributeType(0) != 250)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("PETI_SADECE_250_LVLDE_EFSUNLAYABILIRSIN."));
			return false;
		}

		if (item2->GetAttributeType(1) != 7)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("SADECE_PRIZMATIKTE"));
			return false;
		}

		if (item2->GetAttributeValue(0) < 1350)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Kýymetli dostum bir öncekini atmadan bunu atamazsýn."));
			return false;
		}

		int ozellik1 = number(1800, 2250);
		int ozellik2 = number(1800, 2250);
		int ozellik3 = number(1800, 2250);

		std::unique_ptr<SQLMsg> pMsg(DBManager::instance().DirectQuery("UPDATE new_petsystem SET bonus0 = '%d', bonus1 = '%d', bonus2 = '%d' WHERE id = '%lu';", ozellik1, ozellik2, ozellik3, item2->GetID()));

		item2->SetForceAttribute(0, item2->GetAttributeType(0), ozellik1);
		item2->SetForceAttribute(1, item2->GetAttributeType(1), ozellik2);
		item2->SetForceAttribute(2, item2->GetAttributeType(2), ozellik3);

		item->SetCount(item->GetCount() - 1);
		return true;
	}

	if (item->GetVnum() == 55053)
	{
		LPITEM item2;

		if (!IsValidItemPosition(DestCell) || !(item2 = GetItem(DestCell)))
			return false;

		if (item2->IsExchanging())
			return false;

		if (quest::CQuestManager::instance().GetPCForce(GetPlayerID())->IsRunning() == true)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_OPERATION_IS_FAILED_QUEST_WINDOW_IS_OPENED"));
			return false;
		}

		if (!CanAct(true, true, VAR_TIME_PET_ACTION, nullptr, 0))
			return false;

		if (GetNewPetSystem()->IsActivePet())
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("AKTIF_BIR_PETI_EFSUNLAYAMAZSIN."));
			return false;
		}

		if (item2->GetAttributeType(0) != 250)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("PETI_SADECE_250_LVLDE_EFSUNLAYABILIRSIN."));
			return false;
		}

		if (item2->GetAttributeType(1) != 7)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("SADECE_PRIZMATIKTE"));
			return false;
		}

		if (item2->GetAttributeValue(0) < 1800)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Kýymetli dostum bir öncekini atmadan bunu atamazsýn."));
			return false;
		}

		int ozellik1 = number(2250, 3000);
		int ozellik2 = number(2250, 3000);
		int ozellik3 = number(2250, 3000);

		std::unique_ptr<SQLMsg> pMsg(DBManager::instance().DirectQuery("UPDATE new_petsystem SET bonus0 = '%d', bonus1 = '%d', bonus2 = '%d' WHERE id = '%lu';", ozellik1, ozellik2, ozellik3, item2->GetID()));

		item2->SetForceAttribute(0, item2->GetAttributeType(0), ozellik1);
		item2->SetForceAttribute(1, item2->GetAttributeType(1), ozellik2);
		item2->SetForceAttribute(2, item2->GetAttributeType(2), ozellik3);

		item->SetCount(item->GetCount() - 1);
		return true;
	}

	if (item->IsNewPetItem())
	{
		LPITEM item2 = GetItem(DestCell);

		if (quest::CQuestManager::instance().GetPCForce(GetPlayerID())->IsRunning() == true)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_OPERATION_IS_FAILED_QUEST_WINDOW_IS_OPENED"));
			return false;
		}

		if (!CanAct(true, true, VAR_TIME_PET_ACTION, nullptr, 0))
			return false;

		if (item2)
		{
			if (item2->GetVnum() == 55002)
			{
				if (item2->GetAttributeValue(0) > 0)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Zaten iceride bir hayvan kutusu bulunuyor."));
					return false;
				}
				else if (item->GetSocket(2) < 1)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("PET_SURESI_BITMISKEN_BUNU_YAPAMASSIN"));
					return false;
				}
				else
				{
					if (!IsValidItemPosition(DestCell) || !(item2 = GetItem(DestCell)))
						return false;

					if (item2->IsExchanging())
						return false;

					if (GetNewPetSystem()->IsActivePet())
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Once diger pet'i gonder."));
						return false;
					}

					item2->SetForceAttribute(0, item->GetAttributeType(0), item->GetAttributeValue(0));
					item2->SetForceAttribute(1, item->GetAttributeType(1), item->GetAttributeValue(1));
					item2->SetForceAttribute(2, item->GetAttributeType(2), item->GetAttributeValue(2));

					// Petin Skill Typeleri (type (3,5))
					item2->SetForceAttribute(3, item->GetAttributeType(3), item->GetAttributeValue(3));
					item2->SetForceAttribute(4, item->GetAttributeType(4), item->GetAttributeValue(4));
					item2->SetForceAttribute(5, item->GetAttributeType(5), item->GetAttributeValue(5));

					// Pet Evolution (type (6))
					item2->SetForceAttribute(6, 1, item->GetAttributeType(6));

					item2->SetSocket(0, item->GetVnum()-55700);

					// Pet Level
					item2->SetSocket(1, item->GetSocket(1));
					// Pet Duration
					item2->SetSocket(2, item->GetSocket(2));
					// Pet Total Duration
					item2->SetSocket(3, item->GetSocket(3));

					std::unique_ptr<SQLMsg> pMsg(DBManager::instance().DirectQuery("UPDATE new_petsystem SET id = %u WHERE id = %u;", item2->GetID(), item->GetID()));
					ITEM_MANAGER::instance().RemoveItem(item);
					return true;
				}
			}
			else
			{
				return false;
			}
		}
	}

	if (item->GetVnum() == 55002 && item->GetAttributeValue(0) > 0)
	{
		int pos = GetEmptyInventory(item->GetSize());
		if (pos == -1)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Pet Yeterli alan yok!"));
			return false;
		}

		if (item->IsExchanging())
			return false;

		DWORD vnum2 = 55700 + item->GetSocket(0);
		LPITEM item2 = AutoGiveItem(vnum2, 1);

		item2->SetForceAttribute(0, item->GetAttributeType(0), item->GetAttributeValue(0));
		item2->SetForceAttribute(1, item->GetAttributeType(1), item->GetAttributeValue(1));
		item2->SetForceAttribute(2, item->GetAttributeType(2), item->GetAttributeValue(2));

		// Petin Skill Typeleri (type (3,5))
		item2->SetForceAttribute(3, item->GetAttributeType(3), item->GetAttributeValue(3));
		item2->SetForceAttribute(4, item->GetAttributeType(4), item->GetAttributeValue(4));
		item2->SetForceAttribute(5, item->GetAttributeType(5), item->GetAttributeValue(5));

		// Pet Evolution (type (6))
		item2->SetForceAttribute(6, 1, item->GetAttributeType(6));

		// Pet Level
		item2->SetSocket(1, item->GetSocket(1));
		// Pet Duration
		item2->SetSocket(2, item->GetSocket(2));
		// Pet Total Duration
		item2->SetSocket(3, item->GetSocket(3));

		std::unique_ptr<SQLMsg> pMsg(DBManager::instance().DirectQuery("UPDATE new_petsystem SET id = %u WHERE id = %u;", item2->GetID(), item->GetID()));
		ITEM_MANAGER::instance().RemoveItem(item);
		return true;
	}
#endif

#ifdef __GROWTH_MOUNT_SYSTEM__
	else if (item->GetVnum() >= 55090 && item->GetVnum() <= 55093)
	{
		CNewMountSystem* mountSystem;
		if (!(mountSystem = GetNewMountSystem()))
			return false;

		CNewMountActor* activeMount;
		if (!(activeMount = mountSystem->GetSummoned()))
			return false;

		if (quest::CQuestManager::instance().GetPCForce(GetPlayerID())->IsRunning() == true)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_OPERATION_IS_FAILED_QUEST_WINDOW_IS_OPENED"));
			return false;
		}

		if (!CanAct(true, true, VAR_TIME_MOUNT_ACTION, nullptr, 0))
			return false;

		if (activeMount->GetLevel() >= 157)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("SADECE_157_ALTI"));
			return false;
		}

		bool handled = false;

		switch (item->GetVnum())
		{
		case 55090:
			if (activeMount->GetExp() < activeMount->GetNextExpFromMob())
			{
				if (activeMount->SetExp(activeMount->GetNextExpFromMob() * 0.5, 0))
				{
					item->SetCount(item->GetCount() - 1);
					handled = true;
				}
			}
			break;
		case 55091:
			if (activeMount->GetExp() < activeMount->GetNextExpFromMob())
			{
				if (activeMount->SetExp(activeMount->GetNextExpFromMob(), 0))
				{
					item->SetCount(item->GetCount() - 1);
					handled = true;
				}
			}
			break;
		case 55092:
			if (activeMount->GetExpI() < activeMount->GetNextExpFromItem())
			{
				if (activeMount->SetExp(activeMount->GetNextExpFromItem() * 0.5, 1))
				{
					item->SetCount(item->GetCount() - 1);
					handled = true;
				}
			}
			break;
		case 55093:
			if (activeMount->GetExpI() < activeMount->GetNextExpFromItem())
			{
				if (activeMount->SetExp(activeMount->GetNextExpFromItem(), 1))
				{
					item->SetCount(item->GetCount() - 1);
					handled = true;
				}
			}
			break;

		default:
			break;
		}

		if (!handled)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("DAHA_FAZLA_NESNE_YEDIREMEZSIN"));
		}
	}
	else if (item->GetVnum() >= 55094 && item->GetVnum() <= 55097)
	{
		CNewMountSystem* mountSystem;
		if (!(mountSystem = GetNewMountSystem()))
			return false;

		CNewMountActor* activeMount;
		if (!(activeMount = mountSystem->GetSummoned()))
			return false;

		if (quest::CQuestManager::instance().GetPCForce(GetPlayerID())->IsRunning() == true)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_OPERATION_IS_FAILED_QUEST_WINDOW_IS_OPENED"));
			return false;
		}

		if (!CanAct(true, true, VAR_TIME_MOUNT_ACTION, nullptr, 0))
			return false;

		if (activeMount->GetLevel() < 157)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("SADECE_157_USTU"));
			return false;
		}

		bool handled = false;

		switch (item->GetVnum())
		{
		case 55094:
			if (activeMount->GetExp() < activeMount->GetNextExpFromMob())
			{
				if (activeMount->SetExp(activeMount->GetNextExpFromMob() * 0.5, 0))
				{
					item->SetCount(item->GetCount() - 1);
					handled = true;
				}
			}
			break;
		case 55095:
			if (activeMount->GetExp() < activeMount->GetNextExpFromMob())
			{
				if (activeMount->SetExp(activeMount->GetNextExpFromMob(), 0))
				{
					item->SetCount(item->GetCount() - 1);
					handled = true;
				}
			}
			break;
		case 55096:
			if (activeMount->GetExpI() < activeMount->GetNextExpFromItem())
			{
				if (activeMount->SetExp(activeMount->GetNextExpFromItem() * 0.5, 1))
				{
					item->SetCount(item->GetCount() - 1);
					handled = true;
				}
			}
			break;
		case 55097:
			if (activeMount->GetExpI() < activeMount->GetNextExpFromItem())
			{
				if (activeMount->SetExp(activeMount->GetNextExpFromItem(), 1))
				{
					item->SetCount(item->GetCount() - 1);
					handled = true;
				}
			}
			break;

		default:
			break;
		}

		if (!handled)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("DAHA_FAZLA_NESNE_YEDIREMEZSIN"));
		}
	}
	if ((item->GetVnum() >= 55210 && item->GetVnum() <= 55216))
	{
		int skill = item->GetValue(0);
		CNewMountSystem* mountSystem;
		if (!(mountSystem = GetNewMountSystem()))
			return false;

		if (quest::CQuestManager::instance().GetPCForce(GetPlayerID())->IsRunning() == true)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_OPERATION_IS_FAILED_QUEST_WINDOW_IS_OPENED"));
			return false;
		}

		if (!CanAct(true, true, VAR_TIME_MOUNT_ACTION, nullptr, 0))
			return false;

		if (mountSystem->IsActiveMount() && mountSystem->GetLevel() < 40)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Bineðin leveli henüz hazýr deðil."));
			return false;
		}

		bool ret = mountSystem->IncreaseMountSkill(skill);
		if (ret && mountSystem->IsActiveMount())
			item->SetCount(item->GetCount() - 1);
		else
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("MOUNT_WITH_MOUNT_SUMMON"));
		return true;
	}
	if (item->GetVnum() == 55220)
	{
		LPITEM item2;

		if (!IsValidItemPosition(DestCell) || !(item2 = GetItem(DestCell)))
			return false;

		if (item2->IsExchanging())
			return false;

		if (quest::CQuestManager::instance().GetPCForce(GetPlayerID())->IsRunning() == true)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_OPERATION_IS_FAILED_QUEST_WINDOW_IS_OPENED"));
			return false;
		}

		if (!CanAct(true, true, VAR_TIME_MOUNT_ACTION, nullptr, 0))
			return false;

		char szQuery1[1024];
		snprintf(szQuery1, sizeof(szQuery1), "SELECT duration,tduration FROM new_mountsystem WHERE id = %u;", item2->GetID());
		std::unique_ptr<SQLMsg> pmsg2(DBManager::instance().DirectQuery(szQuery1));
		if (pmsg2->Get()->uiNumRows > 0)
		{
			MYSQL_ROW row = mysql_fetch_row(pmsg2->Get()->pSQLResult);
			int suankiDuration = atoi(row[0]);

			if (suankiDuration >= get_global_time())
			{
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("MOUNT_ALREADY_YOUNG"));
				return false;
			}

			int insertduration0 = time(0) + atoi(row[1]);
			int insertduration1 = time(0) + (atoi(row[2]) / 2);
			if (atoi(row[0]) > 0)
			{
				std::unique_ptr<SQLMsg> pMsg(DBManager::instance().DirectQuery("UPDATE new_mountsystem SET duration = %d WHERE id = %u;", insertduration0, item2->GetID()));
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("PSS_PROTEIN_D1"));
			}
			else
			{
				std::unique_ptr<SQLMsg> pMsg(DBManager::instance().DirectQuery("UPDATE new_mountsystem SET duration = %d WHERE id = %u;", insertduration1, item2->GetID()));
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("PSS_PROTEIN_D2"));
			}

			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("MOUNT_PROTEIN_INFO"));
			item->SetCount(item->GetCount() - 1);
			return true;
		}

		return false;
	}
	if (item->GetVnum() == 55221)
	{
		LPITEM item2;

		if (!IsValidItemPosition(DestCell) || !(item2 = GetItem(DestCell)))
			return false;

		if (item2->IsExchanging())
			return false;

		if (quest::CQuestManager::instance().GetPCForce(GetPlayerID())->IsRunning() == true)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_OPERATION_IS_FAILED_QUEST_WINDOW_IS_OPENED"));
			return false;
		}

		if (!CanAct(true, true, VAR_TIME_MOUNT_ACTION, nullptr, 0))
			return false;

		char szQuery1[1024];
		snprintf(szQuery1, sizeof(szQuery1), "SELECT duration,tduration FROM new_mountsystem WHERE id = %u;", item2->GetID());
		std::unique_ptr<SQLMsg> pmsg2(DBManager::instance().DirectQuery(szQuery1));
		if (pmsg2->Get()->uiNumRows > 0)
		{
			MYSQL_ROW row = mysql_fetch_row(pmsg2->Get()->pSQLResult);
			int suankiDuration = atoi(row[0]);

			if (suankiDuration >= get_global_time())
			{
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("MOUNT_ALREADY_YOUNG"));
				return false;
			}

			int insertduration0 = time(0) + atoi(row[1]) * 2;
			int insertduration1 = time(0) + (atoi(row[2]));
			if (atoi(row[0]) > 0)
			{
				std::unique_ptr<SQLMsg> pMsg(DBManager::instance().DirectQuery("UPDATE new_mountsystem SET duration = %d WHERE id = %u;", insertduration0, item2->GetID()));
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("PSS_PROTEIN_D1"));
			}
			else
			{
				std::unique_ptr<SQLMsg> pMsg(DBManager::instance().DirectQuery("UPDATE new_mountsystem SET duration = %d WHERE id = %u;", insertduration1, item2->GetID()));
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("PSS_PROTEIN_D2"));
			}

			item->SetCount(item->GetCount() - 1);
			return true;
		}

		return false;
	}

	if (item->GetVnum() == 19828)
	{
		LPITEM item2;

		if (!IsValidItemPosition(DestCell) || !(item2 = GetItem(DestCell)))
			return false;

		if (item2->IsExchanging())
			return false;

		if (quest::CQuestManager::instance().GetPCForce(GetPlayerID())->IsRunning() == true)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_OPERATION_IS_FAILED_QUEST_WINDOW_IS_OPENED"));
			return false;
		}

		if (!CanAct(true, true, VAR_TIME_MOUNT_ACTION, nullptr, 0))
			return false;

		if (GetNewMountSystem()->IsActiveMount())
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("AKTIF_BIR_MOUNTI_EFSUNLAYAMAZSIN."));
			return false;
		}

		if (item2->GetAttributeType(0) != 1)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("MOUNTI_SADECE_1_LVLDE_EFSUNLAYABILIRSIN."));
			return false;
		}

		std::unique_ptr<SQLMsg> pMsg(DBManager::instance().DirectQuery("UPDATE new_mountsystem SET bonus0 = '%d', bonus1 = '%d', bonus2 = '%d' WHERE id = '%lu';", 45, 45, 45, item2->GetID()));

		item2->SetForceAttribute(0, item2->GetAttributeType(0), 45);
		item2->SetForceAttribute(1, item2->GetAttributeType(1), 45);
		item2->SetForceAttribute(2, item2->GetAttributeType(2), 45);

		item->SetCount(item->GetCount() - 1);
		return true;
	}

	if (item->GetVnum() == 19827)
	{
		LPITEM item2;

		if (!IsValidItemPosition(DestCell) || !(item2 = GetItem(DestCell)))
			return false;

		if (item2->IsExchanging())
			return false;

		if (quest::CQuestManager::instance().GetPCForce(GetPlayerID())->IsRunning() == true)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_OPERATION_IS_FAILED_QUEST_WINDOW_IS_OPENED"));
			return false;
		}

		if (!CanAct(true, true, VAR_TIME_MOUNT_ACTION, nullptr, 0))
			return false;

		if (GetNewMountSystem()->IsActiveMount())
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("AKTIF_BIR_MOUNTI_EFSUNLAYAMAZSIN."));
			return false;
		}

		if (item2->GetAttributeType(0) != 1)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("MOUNTI_SADECE_1_LVLDE_EFSUNLAYABILIRSIN."));
			return false;
		}

		std::unique_ptr<SQLMsg> pMsg(DBManager::instance().DirectQuery("UPDATE new_mountsystem SET mount_type = '%d' WHERE id = '%lu';", 7, item2->GetID()));

		item2->SetForceAttribute(1, 7, item->GetAttributeValue(1));

		item->SetCount(item->GetCount() - 1);
		return true;
	}

	if (item->GetVnum() == 55203)
	{
		LPITEM item2;

		if (!IsValidItemPosition(DestCell) || !(item2 = GetItem(DestCell)))
			return false;

		if (item2->IsExchanging())
			return false;

		if (quest::CQuestManager::instance().GetPCForce(GetPlayerID())->IsRunning() == true)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_OPERATION_IS_FAILED_QUEST_WINDOW_IS_OPENED"));
			return false;
		}

		if (!CanAct(true, true, VAR_TIME_MOUNT_ACTION, nullptr, 0))
			return false;

		if (GetNewMountSystem()->IsActiveMount())
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("AKTIF_BIR_MOUNTI_EFSUNLAYAMAZSIN."));
			return false;
		}

		if (item2->GetAttributeType(0) != 250)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("MOUNTI_SADECE_250_LVLDE_EFSUNLAYABILIRSIN."));
			return false;
		}

		if (item2->GetAttributeType(1) != 7)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("SADECE_PRIZMATIKTE"));
			return false;
		}

		int ozellik1 = number(450, 900);
		int ozellik2 = number(450, 900);
		int ozellik3 = number(450, 900);

		std::unique_ptr<SQLMsg> pMsg(DBManager::instance().DirectQuery("UPDATE new_mountsystem SET bonus0 = '%d', bonus1 = '%d', bonus2 = '%d' WHERE id = '%lu';", ozellik1, ozellik2, ozellik3, item2->GetID()));

		item2->SetForceAttribute(0, item2->GetAttributeType(0), ozellik1);
		item2->SetForceAttribute(1, item2->GetAttributeType(1), ozellik2);
		item2->SetForceAttribute(2, item2->GetAttributeType(2), ozellik3);

		item->SetCount(item->GetCount() - 1);
		return true;
	}

	if (item->GetVnum() == 55207)
	{
		LPITEM item2;

		if (!IsValidItemPosition(DestCell) || !(item2 = GetItem(DestCell)))
			return false;

		if (item2->IsExchanging())
			return false;

		if (quest::CQuestManager::instance().GetPCForce(GetPlayerID())->IsRunning() == true)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_OPERATION_IS_FAILED_QUEST_WINDOW_IS_OPENED"));
			return false;
		}

		if (!CanAct(true, true, VAR_TIME_MOUNT_ACTION, nullptr, 0))
			return false;

		if (GetNewMountSystem()->IsActiveMount())
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("AKTIF_BIR_MOUNTI_EFSUNLAYAMAZSIN."));
			return false;
		}

		if (item2->GetAttributeType(0) != 250)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("MOUNTI_SADECE_250_LVLDE_EFSUNLAYABILIRSIN."));
			return false;
		}

		if (item2->GetAttributeType(1) != 7)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("SADECE_PRIZMATIKTE"));
			return false;
		}

		if (item2->GetAttributeValue(0) < 450)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Kýymetli dostum bir öncekini atmadan bunu atamazsýn."));
			return false;
		}

		int ozellik1 = number(900, 1350);
		int ozellik2 = number(900, 1350);
		int ozellik3 = number(900, 1350);

		std::unique_ptr<SQLMsg> pMsg(DBManager::instance().DirectQuery("UPDATE new_mountsystem SET bonus0 = '%d', bonus1 = '%d', bonus2 = '%d' WHERE id = '%lu';", ozellik1, ozellik2, ozellik3, item2->GetID()));

		item2->SetForceAttribute(0, item2->GetAttributeType(0), ozellik1);
		item2->SetForceAttribute(1, item2->GetAttributeType(1), ozellik2);
		item2->SetForceAttribute(2, item2->GetAttributeType(2), ozellik3);

		item->SetCount(item->GetCount() - 1);
		return true;
	}

	if (item->GetVnum() == 55209)
	{
		LPITEM item2;

		if (!IsValidItemPosition(DestCell) || !(item2 = GetItem(DestCell)))
			return false;

		if (item2->IsExchanging())
			return false;

		if (quest::CQuestManager::instance().GetPCForce(GetPlayerID())->IsRunning() == true)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_OPERATION_IS_FAILED_QUEST_WINDOW_IS_OPENED"));
			return false;
		}

		if (!CanAct(true, true, VAR_TIME_MOUNT_ACTION, nullptr, 0))
			return false;

		if (GetNewMountSystem()->IsActiveMount())
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("AKTIF_BIR_MOUNTI_EFSUNLAYAMAZSIN."));
			return false;
		}

		if (item2->GetAttributeType(0) != 250)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("MOUNTI_SADECE_250_LVLDE_EFSUNLAYABILIRSIN."));
			return false;
		}

		if (item2->GetAttributeType(1) != 7)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("SADECE_PRIZMATIKTE"));
			return false;
		}

		if (item2->GetAttributeValue(0) < 900)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Kýymetli dostum bir öncekini atmadan bunu atamazsýn."));
			return false;
		}

		int ozellik1 = number(1350, 1800);
		int ozellik2 = number(1350, 1800);
		int ozellik3 = number(1350, 1800);

		std::unique_ptr<SQLMsg> pMsg(DBManager::instance().DirectQuery("UPDATE new_mountsystem SET bonus0 = '%d', bonus1 = '%d', bonus2 = '%d' WHERE id = '%lu';", ozellik1, ozellik2, ozellik3, item2->GetID()));

		item2->SetForceAttribute(0, item2->GetAttributeType(0), ozellik1);
		item2->SetForceAttribute(1, item2->GetAttributeType(1), ozellik2);
		item2->SetForceAttribute(2, item2->GetAttributeType(2), ozellik3);

		item->SetCount(item->GetCount() - 1);
		return true;
	}

	if (item->GetVnum() == 55057)
	{
		LPITEM item2;

		if (!IsValidItemPosition(DestCell) || !(item2 = GetItem(DestCell)))
			return false;

		if (item2->IsExchanging())
			return false;

		if (quest::CQuestManager::instance().GetPCForce(GetPlayerID())->IsRunning() == true)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_OPERATION_IS_FAILED_QUEST_WINDOW_IS_OPENED"));
			return false;
		}

		if (!CanAct(true, true, VAR_TIME_MOUNT_ACTION, nullptr, 0))
			return false;

		if (GetNewMountSystem()->IsActiveMount())
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("AKTIF_BIR_MOUNTI_EFSUNLAYAMAZSIN."));
			return false;
		}

		if (item2->GetAttributeType(0) != 250)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("MOUNTI_SADECE_250_LVLDE_EFSUNLAYABILIRSIN."));
			return false;
		}

		if (item2->GetAttributeType(1) != 7)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("SADECE_PRIZMATIKTE"));
			return false;
		}

		if (item2->GetAttributeValue(0) < 1350)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Kýymetli dostum bir öncekini atmadan bunu atamazsýn."));
			return false;
		}

		int ozellik1 = number(1800, 2250);
		int ozellik2 = number(1800, 2250);
		int ozellik3 = number(1800, 2250);

		std::unique_ptr<SQLMsg> pMsg(DBManager::instance().DirectQuery("UPDATE new_mountsystem SET bonus0 = '%d', bonus1 = '%d', bonus2 = '%d' WHERE id = '%lu';", ozellik1, ozellik2, ozellik3, item2->GetID()));

		item2->SetForceAttribute(0, item2->GetAttributeType(0), ozellik1);
		item2->SetForceAttribute(1, item2->GetAttributeType(1), ozellik2);
		item2->SetForceAttribute(2, item2->GetAttributeType(2), ozellik3);

		item->SetCount(item->GetCount() - 1);
		return true;
	}

	if (item->GetVnum() == 55205)
	{
		LPITEM item2;

		if (!IsValidItemPosition(DestCell) || !(item2 = GetItem(DestCell)))
			return false;

		if (item2->IsExchanging())
			return false;

		if (quest::CQuestManager::instance().GetPCForce(GetPlayerID())->IsRunning() == true)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_OPERATION_IS_FAILED_QUEST_WINDOW_IS_OPENED"));
			return false;
		}

		if (!CanAct(true, true, VAR_TIME_MOUNT_ACTION, nullptr, 0))
			return false;

		if (GetNewMountSystem()->IsActiveMount())
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("AKTIF_BIR_MOUNTI_EFSUNLAYAMAZSIN."));
			return false;
		}

		if (item2->GetAttributeType(0) != 250)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("MOUNTI_SADECE_250_LVLDE_EFSUNLAYABILIRSIN."));
			return false;
		}

		if (item2->GetAttributeType(1) != 7)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("SADECE_PRIZMATIKTE"));
			return false;
		}

		if (item2->GetAttributeValue(0) < 1800)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Kýymetli dostum bir öncekini atmadan bunu atamazsýn."));
			return false;
		}

		int ozellik1 = number(2250, 3000);
		int ozellik2 = number(2250, 3000);
		int ozellik3 = number(2250, 3000);

		std::unique_ptr<SQLMsg> pMsg(DBManager::instance().DirectQuery("UPDATE new_mountsystem SET bonus0 = '%d', bonus1 = '%d', bonus2 = '%d' WHERE id = '%lu';", ozellik1, ozellik2, ozellik3, item2->GetID()));

		item2->SetForceAttribute(0, item2->GetAttributeType(0), ozellik1);
		item2->SetForceAttribute(1, item2->GetAttributeType(1), ozellik2);
		item2->SetForceAttribute(2, item2->GetAttributeType(2), ozellik3);

		item->SetCount(item->GetCount() - 1);
		return true;
	}

	if (item->IsNewMountExItem())
	{
		LPITEM item2 = GetItem(DestCell);

		if (quest::CQuestManager::instance().GetPCForce(GetPlayerID())->IsRunning() == true)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_OPERATION_IS_FAILED_QUEST_WINDOW_IS_OPENED"));
			return false;
		}

		if (!CanAct(true, true, VAR_TIME_MOUNT_ACTION, nullptr, 0))
			return false;

		if (item2)
		{
			if (item2->GetVnum() == 55225)
			{
				if (item2->GetAttributeValue(0) > 0)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Zaten iceride bir hayvan kutusu bulunuyor."));
					return false;
				}
				else if (item->GetSocket(2) < 1)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("MOUNT_SURESI_BITMISKEN_BUNU_YAPAMASSIN"));
					return false;
				}
				else
				{
					if (!IsValidItemPosition(DestCell) || !(item2 = GetItem(DestCell)))
						return false;

					if (item2->IsExchanging())
						return false;

					if (GetNewMountSystem()->IsActiveMount())
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Once diger mount'i gonder."));
						return false;
					}

					item2->SetForceAttribute(0, item->GetAttributeType(0), item->GetAttributeValue(0));
					item2->SetForceAttribute(1, item->GetAttributeType(1), item->GetAttributeValue(1));
					item2->SetForceAttribute(2, item->GetAttributeType(2), item->GetAttributeValue(2));

					item2->SetForceAttribute(3, item->GetAttributeType(3), item->GetAttributeValue(3));
					item2->SetForceAttribute(4, item->GetAttributeType(4), item->GetAttributeValue(4));
					item2->SetForceAttribute(5, item->GetAttributeType(5), item->GetAttributeValue(5));

					item2->SetForceAttribute(6, 1, item->GetAttributeType(6));

					item2->SetSocket(0, item->GetVnum()-55070);

					item2->SetSocket(1, item->GetSocket(1));
					item2->SetSocket(2, item->GetSocket(2));
					item2->SetSocket(3, item->GetSocket(3));

					std::unique_ptr<SQLMsg> pMsg(DBManager::instance().DirectQuery("UPDATE new_mountsystem SET id = %u WHERE id = %u;", item2->GetID(), item->GetID()));
					ITEM_MANAGER::instance().RemoveItem(item);
					return true;
				}
			}
			else
			{
				return false;
			}
		}
	}

	if (item->GetVnum() == 55225 && item->GetAttributeValue(0) > 0)
	{
		int pos = GetEmptyInventory(item->GetSize());
		if (pos == -1)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Mount Yeterli alan yok!"));
			return false;
		}

		if (item->IsExchanging())
			return false;

		DWORD vnum2 = 55070 + item->GetSocket(0);
		LPITEM item2 = AutoGiveItem(vnum2, 1);

		item2->SetForceAttribute(0, item->GetAttributeType(0), item->GetAttributeValue(0));
		item2->SetForceAttribute(1, item->GetAttributeType(1), item->GetAttributeValue(1));
		item2->SetForceAttribute(2, item->GetAttributeType(2), item->GetAttributeValue(2));

		item2->SetForceAttribute(3, item->GetAttributeType(3), item->GetAttributeValue(3));
		item2->SetForceAttribute(4, item->GetAttributeType(4), item->GetAttributeValue(4));
		item2->SetForceAttribute(5, item->GetAttributeType(5), item->GetAttributeValue(5));

		item2->SetForceAttribute(6, 1, item->GetAttributeType(6));

		item2->SetSocket(1, item->GetSocket(1));
		item2->SetSocket(2, item->GetSocket(2));
		item2->SetSocket(3, item->GetSocket(3));

		std::unique_ptr<SQLMsg> pMsg(DBManager::instance().DirectQuery("UPDATE new_mountsystem SET id = %u WHERE id = %u;", item2->GetID(), item->GetID()));
		ITEM_MANAGER::instance().RemoveItem(item);
		return true;
	}
#endif

	switch (item->GetType())
	{
	case ITEM_HAIR:
		return ItemProcess_Hair(item, wDestCell);

	case ITEM_POLYMORPH:
		return true;

	case ITEM_QUEST:
		if (IsObserverMode() == true)
		{
			if (item->GetVnum() == 50051 || item->GetVnum() == 50052 || item->GetVnum() == 50053)
			{
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´ë·Ã Áß¿¡´Â ÀÌ¿ëÇÒ ¼ö ¾ø´Â ¹°Ç°ÀÔ´Ï´Ù."));
				return false;
			}
		}

#ifdef __MOUNT_COSTUME_SYSTEM__
		if (GetWear(WEAR_COSTUME_MOUNT))
		{
			if (item->GetVnum() == 50051 || item->GetVnum() == 50052 || item->GetVnum() == 50053)
			{
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("YOU_CANNOT_USE_THIS_WHILE_RIDING"));
				return false;
			}
		}
#endif
		if (!IS_SET(item->GetFlag(), ITEM_FLAG_QUEST_USE | ITEM_FLAG_QUEST_USE_MULTIPLE))
		{
			if (item->GetSIGVnum() == 0)
				quest::CQuestManager::instance().UseItem(GetPlayerID(), item, false);
			else
				quest::CQuestManager::instance().SIGUse(GetPlayerID(), item->GetSIGVnum(), item, false);
		}
		break;

	case ITEM_CAMPFIRE:
	{
		// @duzenleme
		// koye 2500 tane kamp atesi koymasinlar diye yapildi.
		if (thecore_pulse() < LastCampFireUse + 60)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Please wait a second."));
			return false;
		}

		float fx, fy;
		GetDeltaByDegree(GetRotation(), 100.0f, &fx, &fy);

		LPSECTREE tree = SECTREE_MANAGER::instance().Get(GetMapIndex(), (long)(GetX() + fx), (long)(GetY() + fy));

		if (!tree)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¸ð´ÚºÒÀ» ÇÇ¿ï ¼ö ¾ø´Â ÁöÁ¡ÀÔ´Ï´Ù."));
			return false;
		}

		if (tree->IsAttr((long)(GetX() + fx), (long)(GetY() + fy), ATTR_WATER))
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¹° ¼Ó¿¡ ¸ð´ÚºÒÀ» ÇÇ¿ï ¼ö ¾ø½À´Ï´Ù."));
			return false;
		}

		LPCHARACTER campfire = CHARACTER_MANAGER::instance().SpawnMob(fishing::CAMPFIRE_MOB, GetMapIndex(), (long)(GetX() + fx), (long)(GetY() + fy), 0, false, number(0, 359));

		char_event_info* info = AllocEventInfo<char_event_info>();

		info->ch = campfire;

		campfire->m_pkMiningEvent = event_create(kill_campfire_event, info, PASSES_PER_SEC(40));

		item->SetCount(item->GetCount() - 1);
		LastCampFireUse = thecore_pulse();
	}
	break;

	case ITEM_UNIQUE:
	{
		switch (item->GetSubType())
		{
#ifdef __PET_SYSTEM_PROTO__
		case USE_PET:
		{
			if (CWarMapManager::instance().IsWarMap(GetMapIndex()))
			{
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You can't pet in this map!"));
				return false;
			}

			if (!item->IsEquipped())
			{
				if (GetPetSystem() && GetPetSystem()->CountSummoned() > 0)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Already summoned pet!"));
					return false;
				}
				EquipItem(item);
			}
			else
				UnequipItem(item);
		}
		break;
#endif
		case USE_ABILITY_UP:
		{
			switch (item->GetValue(0))
			{
			case APPLY_MOV_SPEED:
				if (FindAffect(AFFECT_UNIQUE_ABILITY, POINT_MOV_SPEED))
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("BU_ETKI_DEVAM_EDIYOR"));
					return false;
				}
				AddAffect(AFFECT_UNIQUE_ABILITY, POINT_MOV_SPEED, item->GetValue(2), AFF_MOV_SPEED_POTION, item->GetValue(1), 0, true, true);
				break;

			case APPLY_ATT_SPEED:
				if (FindAffect(AFFECT_UNIQUE_ABILITY, POINT_ATT_SPEED))
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("BU_ETKI_DEVAM_EDIYOR"));
					return false;
				}
				AddAffect(AFFECT_UNIQUE_ABILITY, POINT_ATT_SPEED, item->GetValue(2), AFF_ATT_SPEED_POTION, item->GetValue(1), 0, true, true);
				break;

			case APPLY_STR:
				if (FindAffect(AFFECT_UNIQUE_ABILITY, POINT_ST))
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("BU_ETKI_DEVAM_EDIYOR"));
					return false;
				}
				AddAffect(AFFECT_UNIQUE_ABILITY, POINT_ST, item->GetValue(2), 0, item->GetValue(1), 0, true, true);
				break;

			case APPLY_DEX:
				if (FindAffect(AFFECT_UNIQUE_ABILITY, POINT_DX))
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("BU_ETKI_DEVAM_EDIYOR"));
					return false;
				}
				AddAffect(AFFECT_UNIQUE_ABILITY, POINT_DX, item->GetValue(2), 0, item->GetValue(1), 0, true, true);
				break;

			case APPLY_CON:
				if (FindAffect(AFFECT_UNIQUE_ABILITY, POINT_HT))
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("BU_ETKI_DEVAM_EDIYOR"));
					return false;
				}
				AddAffect(AFFECT_UNIQUE_ABILITY, POINT_HT, item->GetValue(2), 0, item->GetValue(1), 0, true, true);
				break;

			case APPLY_INT:
				if (FindAffect(AFFECT_UNIQUE_ABILITY, POINT_IQ))
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("BU_ETKI_DEVAM_EDIYOR"));
					return false;
				}
				AddAffect(AFFECT_UNIQUE_ABILITY, POINT_IQ, item->GetValue(2), 0, item->GetValue(1), 0, true, true);
				break;

			case APPLY_CAST_SPEED:
				if (FindAffect(AFFECT_UNIQUE_ABILITY, POINT_CASTING_SPEED))
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("BU_ETKI_DEVAM_EDIYOR"));
					return false;
				}
				AddAffect(AFFECT_UNIQUE_ABILITY, POINT_CASTING_SPEED, item->GetValue(2), 0, item->GetValue(1), 0, true, true);
				break;

			case APPLY_RESIST_MAGIC:
				if (FindAffect(AFFECT_UNIQUE_ABILITY, POINT_RESIST_MAGIC))
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("BU_ETKI_DEVAM_EDIYOR"));
					return false;
				}
				AddAffect(AFFECT_UNIQUE_ABILITY, POINT_RESIST_MAGIC, item->GetValue(2), 0, item->GetValue(1), 0, true, true);
				break;

			case APPLY_ATT_GRADE_BONUS:
				if (FindAffect(AFFECT_UNIQUE_ABILITY, POINT_ATT_GRADE_BONUS))
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("BU_ETKI_DEVAM_EDIYOR"));
					return false;
				}
				AddAffect(AFFECT_UNIQUE_ABILITY, POINT_ATT_GRADE_BONUS,
					item->GetValue(2), 0, item->GetValue(1), 0, true, true);
				break;

			case APPLY_DEF_GRADE_BONUS:
				if (FindAffect(AFFECT_UNIQUE_ABILITY, POINT_DEF_GRADE_BONUS))
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("BU_ETKI_DEVAM_EDIYOR"));
					return false;
				}
				AddAffect(AFFECT_UNIQUE_ABILITY, POINT_DEF_GRADE_BONUS,
					item->GetValue(2), 0, item->GetValue(1), 0, true, true);
				break;
			}
		}

		if (GetDungeon())
			GetDungeon()->UsePotion(this);

		if (GetWarMap())
			GetWarMap()->UsePotion(this, item);

		item->SetCount(item->GetCount() - 1);
		break;

		default:
		{
			if (item->GetSubType() == USE_SPECIAL)
			{
				sys_log(0, "ITEM_UNIQUE: USE_SPECIAL %u", item->GetVnum());

				switch (item->GetVnum())
				{
				case 71049:
					UseSilkBotary();
					break;
				}
			}
			else
			{
				if (!item->IsEquipped())
					EquipItem(item);
				else
					UnequipItem(item);
			}
		}
		break;
		}
	}
	break;
#ifdef __SHINING_ITEM_SYSTEM__
	case ITEM_SHINING:
#endif
#ifdef __CAKRA_ITEM_SYSTEM__
	case ITEM_CAKRA:
#endif
#ifdef __SEBNEM_ITEM_SYSTEM__
	case ITEM_SEBNEM:
#endif
	case ITEM_COSTUME:
	case ITEM_WEAPON:
	case ITEM_ARMOR:
	case ITEM_ROD:
	case ITEM_RING:
	case ITEM_BELT:
		// MINING
	case ITEM_PICK:
		// END_OF_MINING
		if (!item->IsEquipped())
			EquipItem(item);
		else
			UnequipItem(item);
		break;

	case ITEM_DS:
	{
		if (!item->IsEquipped())
			return false;
		return DSManager::instance().PullOut(this, NPOS, item);
		break;
	}
	case ITEM_SPECIAL_DS:
		if (!item->IsEquipped())
			EquipItem(item);
		else
			UnequipItem(item);
		break;

	case ITEM_FISH:
	{
		if (item->GetSubType() == FISH_ALIVE)
			fishing::UseFish(this, item);
	}
	break;

	case ITEM_TREASURE_BOX:
	{
		return false;
	}
	break;

	case ITEM_TREASURE_KEY:
	{
		return false;
	}
	break;

#ifdef __RENEWAL_CHEST_USE__
	case ITEM_GIFTBOX:
	{
		if (GetMapIndex() == 1 || GetMapIndex() == 21 || GetMapIndex() == 41 || GetMapIndex() == 3 || GetMapIndex() == 23 || GetMapIndex() == 43)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("GIFTBOX_GACHA_MAP_INDEX"));
			return false;
		}

		if (GetDungeon() || GetWarMap() || IS_DUNGEON_ZONE(GetMapIndex()))
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You cannot open the safebox in dungeon or at war."));
			return false;
		}

#ifdef __BOSS_SCROLL__
		if (item->IsBossScroll())
		{
			if (GetMapIndex() == 1 || GetMapIndex() == 21 || GetMapIndex() == 41)
			{
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("BOSS_SCROLL_DONT_USE_THIS_MAP"));
				return false;
			}

			if (CWarMapManager::instance().IsWarMap(GetMapIndex()) == true) // block war map
				return false;

			if (GetDungeon())
			{
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("BOSS_SCROLL_DONT_USE_THIS_MAP"));
				return false;
			}

			if (IS_DUNGEON_ZONE(GetMapIndex()))
			{
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("BOSS_SCROLL_DONT_USE_THIS_MAP"));
				return false;
			}
		}
#endif

		DWORD dwBoxVnum = item->GetVnum();

		if (dwBoxVnum > 51500 && dwBoxVnum < 52000)
		{
			if (!(this->DragonSoul_IsQualified()))
			{
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¸ÕÀú ¿ëÈ¥¼® Äù½ºÆ®¸¦ ¿Ï·áÇÏ¼Å¾ß ÇÕ´Ï´Ù."));
				return false;
			}
		}

		if (dwBoxVnum >= 51511 && dwBoxVnum <= 51516)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("SIMYACIDAN_SATIN_AL"));
			return false;
		}

		boost::unordered_map<DWORD, DWORD> itemMap;
		int succesCount = 0;

		if ((succesCount = GiveItemFromSpecialItemGroup(dwBoxVnum, count, itemMap)) > 0)
		{
			item->SetCount(item->GetCount() - succesCount);

			for (auto &&e_item : itemMap)
			{
				LPITEM newItem = nullptr;
				DWORD totalCount = e_item.second;

				while(e_item.second > 0)
				{
					newItem = nullptr;

					if (e_item.second > g_bItemCountLimit)
						newItem = AutoGiveItem(e_item.first, g_bItemCountLimit, -1, false);
					else
						newItem = AutoGiveItem(e_item.first, e_item.second, -1, false);

					if (!newItem)
					{
						sys_err("NO ITEM! %s (%u %u)", GetName(), e_item.first, e_item.second);
						break;
					}

					if (e_item.second > g_bItemCountLimit)
					{
						if (IS_SET(newItem->GetFlag(), ITEM_FLAG_STACKABLE) && !IS_SET(newItem->GetFlag(), ITEM_ANTIFLAG_STACK))
							e_item.second -= g_bItemCountLimit;
						else
							e_item.second -= 1;
					}
					else
					{
						if (IS_SET(newItem->GetFlag(), ITEM_FLAG_STACKABLE) && !IS_SET(newItem->GetFlag(), ITEM_ANTIFLAG_STACK))
							e_item.second = 0;
						else
							e_item.second -= 1;
					}
				}

				if (!newItem)
				{
					sys_err("NO ITEM! %s (%u %u)", GetName(), e_item.first, e_item.second);
					continue;
				}
				if (newItem->IsDragonSoul())
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("DRAGON_SOUL_INVENTORY_ADDED_MULTIPLE: %s %u"), newItem->GetName(), totalCount);
				else if (newItem->IsUpgradeItem())
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("UPGRADE_INVENTORY_ADDED_MULTIPLE: %s %u"), newItem->GetName(), totalCount);
				else if (newItem->IsBook())
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("BOOK_INVENTORY_ADDED_MULTIPLE: %s %u"), newItem->GetName(), totalCount);
				else if (newItem->IsStone())
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("STONE_INVENTORY_ADDED_MULTIPLE: %s %u"), newItem->GetName(), totalCount);
				else if (newItem->IsFlower())
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("FLOWER_INVENTORY_ADDED_MULTIPLE: %s %u"), newItem->GetName(), totalCount);
				else if (newItem->IsAttrItem())
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ATTR_INVENTORY_ADDED_MULTIPLE: %s %u"), newItem->GetName(), totalCount);
				else if (newItem->IsChest())
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CHEST_INVENTORY_ADDED_MULTIPLE: %s %u"), newItem->GetName(), totalCount);
				else
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("NORMAL_INVENTORY_ADDED_MULTIPLE: %s %u"), newItem->GetName(), totalCount);
			}
		}
		else
		{
			ChatPacket(CHAT_TYPE_TALKING, LC_TEXT("¾Æ¹«°Íµµ ¾òÀ» ¼ö ¾ø¾ú½À´Ï´Ù."));
			return false;
		}
	}
	break;
#endif

	case ITEM_SKILLFORGET:
	{
		if (!item->GetSocket(0))
		{
			item->SetCount(item->GetCount() - 1);
			return false;
		}

		DWORD dwVnum = item->GetSocket(0);

		if (SkillLevelDown(dwVnum))
		{
			item->SetCount(item->GetCount() - 1);
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("½ºÅ³ ·¹º§À» ³»¸®´Âµ¥ ¼º°øÇÏ¿´½À´Ï´Ù."));
		}
		else
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("½ºÅ³ ·¹º§À» ³»¸± ¼ö ¾ø½À´Ï´Ù."));
	}
	break;

	case ITEM_SKILLBOOK:
	{
#ifdef __SKILLBOOK_SYSTEM__
		if (item->GetVnum() == 50300)
		{
			if (GetSkillGroup() == 0)
			{
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("BK_CANNOT_READ_SKILLGROUP"));
				return false;
			}
			ChatPacket(CHAT_TYPE_COMMAND, "bkekranac");
		}
		else
		{
#endif
			// @edit014 BEGIN
			if (!GetSkillGroup())
			{
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You cant read skill book beacuse you dont choise skill group."));
				return false;
			}
			// @edit014 END

			DWORD dwVnum = 0;

			if (item->GetVnum() == 50300)
				dwVnum = item->GetSocket(0);
			else
				dwVnum = item->GetValue(0);

			if (0 == dwVnum)
			{
				item->SetCount(item->GetCount() - 1);
				return false;
			}

			if (true == LearnSkillByBook(dwVnum))
			{
				item->SetCount(item->GetCount() - 1);

				int iReadDelay = number(SKILLBOOK_DELAY_MIN, SKILLBOOK_DELAY_MAX);

				SetSkillNextReadTime(dwVnum, get_global_time() + iReadDelay);
			}
#ifdef __SKILLBOOK_SYSTEM__
		}
#endif
	}
	break;

	case ITEM_USE:
	{
		if (item->GetVnum() > 50800 && item->GetVnum() <= 50820)
		{
			if (test_server)
				sys_log(0, "ADD addtional effect : vnum(%d) subtype(%d)", item->GetOriginalVnum(), item->GetSubType());

			int affect_type = AFFECT_EXP_BONUS_EURO_FREE;
			int apply_type = aApplyInfo[item->GetValue(0)].bPointType;
			int apply_value = item->GetValue(2);
			int apply_duration = item->GetValue(1);

			switch (item->GetSubType())
			{
			case USE_ABILITY_UP:
				if (FindAffect(affect_type, apply_type))
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ¹Ì È¿°ú°¡ °É·Á ÀÖ½À´Ï´Ù."));
					return false;
				}

				{
					switch (item->GetValue(0))
					{
					case APPLY_MOV_SPEED:
						AddAffect(affect_type, apply_type, apply_value, AFF_MOV_SPEED_POTION, apply_duration, 0, true, true);
						break;

					case APPLY_ATT_SPEED:
						AddAffect(affect_type, apply_type, apply_value, AFF_ATT_SPEED_POTION, apply_duration, 0, true, true);
						break;

					case APPLY_STR:
					case APPLY_DEX:
					case APPLY_CON:
					case APPLY_INT:
					case APPLY_CAST_SPEED:
					case APPLY_RESIST_MAGIC:
					case APPLY_ATT_GRADE_BONUS:
					case APPLY_DEF_GRADE_BONUS:
						AddAffect(affect_type, apply_type, apply_value, 0, apply_duration, 0, true, true);
						break;
					}
				}

				if (GetDungeon())
					GetDungeon()->UsePotion(this);

				if (GetWarMap())
					GetWarMap()->UsePotion(this, item);

				item->SetCount(item->GetCount() - 1);
				break;

			case USE_AFFECT:
			{
				if (FindAffect(AFFECT_EXP_BONUS_EURO_FREE, aApplyInfo[item->GetValue(1)].bPointType))
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ¹Ì È¿°ú°¡ °É·Á ÀÖ½À´Ï´Ù."));
				else
				{
					AddAffect(AFFECT_EXP_BONUS_EURO_FREE, aApplyInfo[item->GetValue(1)].bPointType, item->GetValue(2), 0, item->GetValue(3), 0, false, true);
					item->SetCount(item->GetCount() - 1);
				}
			}
			break;

			case USE_POTION_NODELAY:
			{
				bool used = false;

				if (item->GetValue(0) != 0)
				{
					if (GetHP() < GetMaxHP())
					{
						PointChange(POINT_HP, item->GetValue(0) * (100 + GetPoint(POINT_POTION_BONUS)) / 100);
						EffectPacket(SE_HPUP_RED);
						used = TRUE;
					}
				}

				if (item->GetValue(1) != 0)
				{
					if (GetSP() < GetMaxSP())
					{
						PointChange(POINT_SP, item->GetValue(1) * (100 + GetPoint(POINT_POTION_BONUS)) / 100);
						EffectPacket(SE_SPUP_BLUE);
						used = TRUE;
					}
				}

				if (item->GetValue(3) != 0)
				{
					if (GetHP() < GetMaxHP())
					{
						PointChange(POINT_HP, item->GetValue(3) * GetMaxHP() / 100);
						EffectPacket(SE_HPUP_RED);
						used = TRUE;
					}
				}

				if (item->GetValue(4) != 0)
				{
					if (GetSP() < GetMaxSP())
					{
						PointChange(POINT_SP, item->GetValue(4) * GetMaxSP() / 100);
						EffectPacket(SE_SPUP_BLUE);
						used = TRUE;
					}
				}

				if (used)
				{
					if (GetDungeon())
						GetDungeon()->UsePotion(this);

					if (GetWarMap())
						GetWarMap()->UsePotion(this, item);

					item->SetCount(item->GetCount() - 1);
				}
			}
			break;
			}

			return true;
		}

		if (test_server)
		{
			sys_log(0, "USE_ITEM %s Type %d SubType %d vnum %d", item->GetName(), item->GetType(), item->GetSubType(), item->GetOriginalVnum());
		}

		switch (item->GetSubType())
		{
		case USE_TIME_CHARGE_PER:
		{
			LPITEM pDestItem = GetItem(DestCell);
			if (NULL == pDestItem)
			{
				return false;
			}

			if (pDestItem->IsDragonSoul())
			{
				int ret;
				if (item->GetVnum() == DRAGON_HEART_VNUM)
					ret = pDestItem->GiveMoreTime_Per((float)item->GetSocket(ITEM_SOCKET_CHARGING_AMOUNT_IDX));
				else
					ret = pDestItem->GiveMoreTime_Per((float)item->GetValue(ITEM_VALUE_CHARGING_AMOUNT_IDX));

				if (ret > 0)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%dÃÊ ¸¸Å­ ÃæÀüµÇ¾ú½À´Ï´Ù."), ret);
					item->SetCount(item->GetCount() - 1);
					return true;
				}
				else
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÃæÀüÇÒ ¼ö ¾ø½À´Ï´Ù."));
					return false;
				}
			}
			else
				return false;
		}
		break;
		case USE_TIME_CHARGE_FIX:
		{
			LPITEM pDestItem = GetItem(DestCell);
			if (NULL == pDestItem)
			{
				return false;
			}

			if (pDestItem->IsDragonSoul())
			{
#ifdef __DSS_RECHARGE_ITEM__
				int ret = pDestItem->GiveMoreTime_Extend(item->GetValue(ITEM_VALUE_CHARGING_AMOUNT_IDX));
				if (ret)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%d days have been added."), (ret / 86400));
					item->SetCount(item->GetCount() - 1);
					return true;
				}
				else
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÃæÀüÇÒ ¼ö ¾ø½À´Ï´Ù."));
					return false;
				}
#else
				int ret = pDestItem->GiveMoreTime_Fix(item->GetValue(ITEM_VALUE_CHARGING_AMOUNT_IDX));
				if (ret)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%dÃÊ ¸¸Å­ ÃæÀüµÇ¾ú½À´Ï´Ù."), ret);
					item->SetCount(item->GetCount() - 1);
					return true;
				}
				else
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÃæÀüÇÒ ¼ö ¾ø½À´Ï´Ù."));
					return false;
				}
#endif
			}
			else
				return false;
		}
		break;
		case USE_SPECIAL:

			switch (item->GetVnum())
			{
#ifdef __ITEM_EVOLUTION__
			case 56008:
			{
				LPITEM item2;

				if (!IsValidItemPosition(DestCell) || !(item2 = GetItem(DestCell)))
					return false;

				if (item2->IsExchanging() || item2->IsEquipped()) // @duzenleme bu esya giyilemez ??? xD
					return false;

#ifdef __ARMOR_EVOLUTION__
				if (item2->GetType() != ITEM_WEAPON && item2->GetType() != ITEM_ARMOR)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("EVO_ONLY_WEAPON"));
					return false;
				}
#else
				if (item2->GetType() != ITEM_WEAPON)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("EVO_ONLY_WEAPON"));
					return false;
				}
#endif

				if (item2->GetEvolutionType() == WEAPON_EVOLUTION_TYPE_NONE)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("EVO_HAS_NONE"));
					return false;
				}

				item2->SetEvolution(0);
				item->SetCount(item->GetCount() - 1);
				ITEM_MANAGER::instance().FlushDelayedSave(item2);
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("EVO_POINT_HAS_RESET"));
			}
			break;
#endif

			case 31060:
				{
					LPITEM item2;

					if (!IsValidItemPosition(DestCell) || !(item2 = GetItem(DestCell)))
						return false;

					if (item2->IsExchanging() || item2->IsEquipped()) // @duzenleme bu esya giyilemez ??? xD
						return false;

					if (item2->GetType() != ITEM_COSTUME || item2->GetSubType() != COSTUME_ACCE)
						return false;

					if (item2->GetSocket(2) != 0)
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ITEM_ACCE_NEED_LOCKED_ITEM_6"));
						return false;
					}

					item2->SetSocket(2, 1);
					item->SetCount(item->GetCount() -1);
					ITEM_MANAGER::instance().FlushDelayedSave(item2);
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ITEM_ACCE_LOCKED_UP"));
				}
				break;

			case 31061:
				{
					LPITEM item2;

					if (!IsValidItemPosition(DestCell) || !(item2 = GetItem(DestCell)))
						return false;

					if (item2->IsExchanging() || item2->IsEquipped()) // @duzenleme bu esya giyilemez ??? xD
						return false;

					if (item2->GetType() != ITEM_COSTUME || item2->GetSubType() != COSTUME_ACCE)
						return false;

					if (item2->GetSocket(2) != 1)
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ITEM_ACCE_NEED_LOCKED_ITEM_7"));
						return false;
					}

					item2->SetSocket(2, 2);
					item->SetCount(item->GetCount() -1);
					ITEM_MANAGER::instance().FlushDelayedSave(item2);
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ITEM_ACCE_LOCKED_UP"));
				}
				break;

			case 31062:
				{
					LPITEM item2;

					if (!IsValidItemPosition(DestCell) || !(item2 = GetItem(DestCell)))
						return false;

					if (item2->IsExchanging() || item2->IsEquipped()) // @duzenleme bu esya giyilemez ??? xD
						return false;

					if (item2->GetType() != ITEM_COSTUME || item2->GetSubType() != COSTUME_ACCE)
						return false;

					if (item2->GetSocket(2) != 2)
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ITEM_ACCE_NEED_LOCKED_ITEM_8"));
						return false;
					}

					item2->SetSocket(2, 3);
					item->SetCount(item->GetCount() -1);
					ITEM_MANAGER::instance().FlushDelayedSave(item2);
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ITEM_ACCE_LOCKED_UP"));
				}
				break;

			case 31063:
				{
					LPITEM item2;

					if (!IsValidItemPosition(DestCell) || !(item2 = GetItem(DestCell)))
						return false;

					if (item2->IsExchanging() || item2->IsEquipped()) // @duzenleme bu esya giyilemez ??? xD
						return false;

					if (item2->GetType() != ITEM_COSTUME || item2->GetSubType() != COSTUME_ACCE)
						return false;

					if (item2->GetSocket(2) != 3)
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ITEM_ACCE_NEED_LOCKED_ITEM_9"));
						return false;
					}

					item2->SetSocket(2, 4);
					item->SetCount(item->GetCount() -1);
					ITEM_MANAGER::instance().FlushDelayedSave(item2);
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ITEM_ACCE_LOCKED_UP"));
				}
				break;

			case 31064:
				{
					LPITEM item2;

					if (!IsValidItemPosition(DestCell) || !(item2 = GetItem(DestCell)))
						return false;

					if (item2->IsExchanging() || item2->IsEquipped()) // @duzenleme bu esya giyilemez ??? xD
						return false;

					if (item2->GetType() != ITEM_COSTUME || item2->GetSubType() != COSTUME_ACCE)
						return false;

					if (item2->GetSocket(2) != 4)
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ITEM_ACCE_NEED_LOCKED_ITEM_10"));
						return false;
					}

					item2->SetSocket(2, 5);
					item->SetCount(item->GetCount() -1);
					ITEM_MANAGER::instance().FlushDelayedSave(item2);
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ITEM_ACCE_LOCKED_UP"));
				}
				break;

			case 31065:
				{
					LPITEM item2;

					if (!IsValidItemPosition(DestCell) || !(item2 = GetItem(DestCell)))
						return false;

					if (item2->IsExchanging() || item2->IsEquipped()) // @duzenleme bu esya giyilemez ??? xD
						return false;

					if (item2->GetType() != ITEM_COSTUME || item2->GetSubType() != COSTUME_ACCE)
						return false;

					if (item2->GetSocket(2) != 5)
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ITEM_ACCE_NEED_LOCKED_ITEM_11"));
						return false;
					}

					item2->SetSocket(2, 6);
					item->SetCount(item->GetCount() -1);
					ITEM_MANAGER::instance().FlushDelayedSave(item2);
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ITEM_ACCE_LOCKED_UP"));
				}
				break;

			case 31066:
				{
					LPITEM item2;

					if (!IsValidItemPosition(DestCell) || !(item2 = GetItem(DestCell)))
						return false;

					if (item2->IsExchanging() || item2->IsEquipped()) // @duzenleme bu esya giyilemez ??? xD
						return false;

					if (item2->GetType() != ITEM_COSTUME || item2->GetSubType() != COSTUME_ACCE)
						return false;

					if (item2->GetSocket(2) != 6)
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ITEM_ACCE_NEED_LOCKED_ITEM_12"));
						return false;
					}

					item2->SetSocket(2, 7);
					item->SetCount(item->GetCount() -1);
					ITEM_MANAGER::instance().FlushDelayedSave(item2);
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ITEM_ACCE_LOCKED_UP"));
				}
				break;

			case 31067:
				{
					LPITEM item2;

					if (!IsValidItemPosition(DestCell) || !(item2 = GetItem(DestCell)))
						return false;

					if (item2->IsExchanging() || item2->IsEquipped()) // @duzenleme bu esya giyilemez ??? xD
						return false;

					if (item2->GetType() != ITEM_COSTUME || item2->GetSubType() != COSTUME_ACCE)
						return false;

					if (item2->GetSocket(2) != 7)
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ITEM_ACCE_NEED_LOCKED_ITEM_13"));
						return false;
					}

					item2->SetSocket(2, 8);
					item->SetCount(item->GetCount() -1);
					ITEM_MANAGER::instance().FlushDelayedSave(item2);
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ITEM_ACCE_LOCKED_UP"));
				}
				break;

			case 31068:
				{
					LPITEM item2;

					if (!IsValidItemPosition(DestCell) || !(item2 = GetItem(DestCell)))
						return false;

					if (item2->IsExchanging() || item2->IsEquipped()) // @duzenleme bu esya giyilemez ??? xD
						return false;

					if (item2->GetType() != ITEM_COSTUME || item2->GetSubType() != COSTUME_ACCE)
						return false;

					if (item2->GetSocket(2) != 8)
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ITEM_ACCE_NEED_LOCKED_ITEM_14"));
						return false;
					}

					item2->SetSocket(2, 9);
					item->SetCount(item->GetCount() -1);
					ITEM_MANAGER::instance().FlushDelayedSave(item2);
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ITEM_ACCE_LOCKED_UP"));
				}
				break;

			case 31069:
				{
					LPITEM item2;

					if (!IsValidItemPosition(DestCell) || !(item2 = GetItem(DestCell)))
						return false;

					if (item2->IsExchanging() || item2->IsEquipped()) // @duzenleme bu esya giyilemez ??? xD
						return false;

					if (item2->GetType() != ITEM_COSTUME || item2->GetSubType() != COSTUME_ACCE)
						return false;

					if (item2->GetSocket(2) != 9)
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ITEM_ACCE_NEED_LOCKED_ITEM_15"));
						return false;
					}

					item2->SetSocket(2, 10);
					item->SetCount(item->GetCount() -1);
					ITEM_MANAGER::instance().FlushDelayedSave(item2);
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ITEM_ACCE_LOCKED_UP"));
				}
				break;

			case ITEM_NOG_POCKET:
			case 57050:
			case 57062:
			case 57074:
			case 57086:
			case 57098:
			{
				if (FindAffect(AFFECT_NOG_ABILITY))
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ¹Ì È¿°ú°¡ °É·Á ÀÖ½À´Ï´Ù."));
					return false;
				}
				long time = item->GetValue(0);
				long efsun1 = item->GetValue(1);
				long efsun2 = item->GetValue(2);
				long efsun3 = item->GetValue(3);
				long efsun4 = item->GetValue(4);
				long efsun5 = item->GetValue(5);
				AddAffect(AFFECT_NOG_ABILITY, POINT_MOV_SPEED, efsun1, AFF_MOV_SPEED_POTION, time, 0, true, true);
				AddAffect(AFFECT_NOG_ABILITY, POINT_MALL_ATTBONUS, efsun2, AFF_NONE, time, 0, true, true);
				AddAffect(AFFECT_NOG_ABILITY, POINT_ATTBONUS_MONSTER, efsun3, AFF_NONE, time, 0, true, true);
				AddAffect(AFFECT_NOG_ABILITY, POINT_ATTBONUS_BOSS, efsun4, AFF_NONE, time, 0, true, true);
				AddAffect(AFFECT_NOG_ABILITY, POINT_ATTBONUS_STONE, efsun5, AFF_NONE, time, 0, true, true);
				item->SetCount(item->GetCount() - 1);
			}
			break;

			case 99001:
			case 99002:
			case 99003:
			case 99004:
			case 99005:
			case 99006:
			case 99007:
			case 99008:
			case 99009:
			case 99010:
			case 99011:
			case 99012:
			case 99013:
			case 99014:
			case 99015:
			case 99016:
			case 99017:
			case 99018:
			case 99019:
			case 99020:
			case 99021:
			case 99022:
			case 99023:
			case 99024:
			case 99025:
			case 99026:
			case 99027:
			case 99028:
			case 99029:
			case 99030:
			case 99031:
			case 99032:
			case 99033:
			case 99034:
			case 99035:
			case 99036:
			case 99037:
			case 99038:
			case 99039:
			case 99040:
			{
				long efsun1 = item->GetValue(0);
				long efsun2 = item->GetValue(1);
				long efsun3 = item->GetValue(3);

				int bendeki = CheckAffectValue(AFFECT_YILBASI, POINT_ATTBONUS_MONSTER);

				if (bendeki == 0 && efsun1 != 25)
				{
					ChatPacket(CHAT_TYPE_INFO, "Önce ilk seviyedeki nesneyi kullanmalýsýn.");
					return false;
				}

				if (bendeki == 25 && efsun1 != 50)
				{
					ChatPacket(CHAT_TYPE_INFO, "Önce bir önceki seviyedeki nesneyi kullanmalýsýn.");
					return false;
				}

				if (bendeki == 50 && efsun1 != 75)
				{
					ChatPacket(CHAT_TYPE_INFO, "Önce bir önceki seviyedeki nesneyi kullanmalýsýn.");
					return false;
				}

				if (bendeki == 75 && efsun1 != 100)
				{
					ChatPacket(CHAT_TYPE_INFO, "Önce bir önceki seviyedeki nesneyi kullanmalýsýn.");
					return false;
				}

				if (bendeki == 100 && efsun1 != 125)
				{
					ChatPacket(CHAT_TYPE_INFO, "Önce bir önceki seviyedeki nesneyi kullanmalýsýn.");
					return false;
				}

				if (bendeki == 125 && efsun1 != 150)
				{
					ChatPacket(CHAT_TYPE_INFO, "Önce bir önceki seviyedeki nesneyi kullanmalýsýn.");
					return false;
				}

				if (bendeki == 150 && efsun1 != 175)
				{
					ChatPacket(CHAT_TYPE_INFO, "Önce bir önceki seviyedeki nesneyi kullanmalýsýn.");
					return false;
				}

				if (bendeki == 175 && efsun1 != 200)
				{
					ChatPacket(CHAT_TYPE_INFO, "Önce bir önceki seviyedeki nesneyi kullanmalýsýn.");
					return false;
				}

				if (bendeki == 200 && efsun1 != 225)
				{
					ChatPacket(CHAT_TYPE_INFO, "Önce bir önceki seviyedeki nesneyi kullanmalýsýn.");
					return false;
				}

				if (bendeki == 225 && efsun1 != 250)
				{
					ChatPacket(CHAT_TYPE_INFO, "Önce bir önceki seviyedeki nesneyi kullanmalýsýn.");
					return false;
				}

				if (bendeki == 250 && efsun1 != 275)
				{
					ChatPacket(CHAT_TYPE_INFO, "Önce bir önceki seviyedeki nesneyi kullanmalýsýn.");
					return false;
				}

				if (bendeki == 275 && efsun1 != 300)
				{
					ChatPacket(CHAT_TYPE_INFO, "Önce bir önceki seviyedeki nesneyi kullanmalýsýn.");
					return false;
				}

				if (bendeki == 300 && efsun1 != 325)
				{
					ChatPacket(CHAT_TYPE_INFO, "Önce bir önceki seviyedeki nesneyi kullanmalýsýn.");
					return false;
				}

				if (bendeki == 325 && efsun1 != 350)
				{
					ChatPacket(CHAT_TYPE_INFO, "Önce bir önceki seviyedeki nesneyi kullanmalýsýn.");
					return false;
				}

				if (bendeki == 350 && efsun1 != 375)
				{
					ChatPacket(CHAT_TYPE_INFO, "Önce bir önceki seviyedeki nesneyi kullanmalýsýn.");
					return false;
				}

				if (bendeki == 375 && efsun1 != 400)
				{
					ChatPacket(CHAT_TYPE_INFO, "Önce bir önceki seviyedeki nesneyi kullanmalýsýn.");
					return false;
				}

				if (bendeki == 400 && efsun1 != 425)
				{
					ChatPacket(CHAT_TYPE_INFO, "Önce bir önceki seviyedeki nesneyi kullanmalýsýn.");
					return false;
				}

				if (bendeki == 425 && efsun1 != 450)
				{
					ChatPacket(CHAT_TYPE_INFO, "Önce bir önceki seviyedeki nesneyi kullanmalýsýn.");
					return false;
				}

				if (bendeki == 450 && efsun1 != 475)
				{
					ChatPacket(CHAT_TYPE_INFO, "Önce bir önceki seviyedeki nesneyi kullanmalýsýn.");
					return false;
				}

				if (bendeki == 475 && efsun1 != 500)
				{
					ChatPacket(CHAT_TYPE_INFO, "Önce bir önceki seviyedeki nesneyi kullanmalýsýn.");
					return false;
				}

				if (bendeki == 500 && efsun1 != 525)
				{
					ChatPacket(CHAT_TYPE_INFO, "Önce bir önceki seviyedeki nesneyi kullanmalýsýn.");
					return false;
				}

				if (bendeki == 525 && efsun1 != 550)
				{
					ChatPacket(CHAT_TYPE_INFO, "Önce bir önceki seviyedeki nesneyi kullanmalýsýn.");
					return false;
				}

				if (bendeki == 550 && efsun1 != 575)
				{
					ChatPacket(CHAT_TYPE_INFO, "Önce bir önceki seviyedeki nesneyi kullanmalýsýn.");
					return false;
				}

				if (bendeki == 575 && efsun1 != 600)
				{
					ChatPacket(CHAT_TYPE_INFO, "Önce bir önceki seviyedeki nesneyi kullanmalýsýn.");
					return false;
				}

				if (bendeki == 600 && efsun1 != 625)
				{
					ChatPacket(CHAT_TYPE_INFO, "Önce bir önceki seviyedeki nesneyi kullanmalýsýn.");
					return false;
				}

				if (bendeki == 625 && efsun1 != 650)
				{
					ChatPacket(CHAT_TYPE_INFO, "Önce bir önceki seviyedeki nesneyi kullanmalýsýn.");
					return false;
				}

				if (bendeki == 650 && efsun1 != 675)
				{
					ChatPacket(CHAT_TYPE_INFO, "Önce bir önceki seviyedeki nesneyi kullanmalýsýn.");
					return false;
				}

				if (bendeki == 675 && efsun1 != 700)
				{
					ChatPacket(CHAT_TYPE_INFO, "Önce bir önceki seviyedeki nesneyi kullanmalýsýn.");
					return false;
				}

				if (bendeki == 700 && efsun1 != 725)
				{
					ChatPacket(CHAT_TYPE_INFO, "Önce bir önceki seviyedeki nesneyi kullanmalýsýn.");
					return false;
				}

				if (bendeki == 725 && efsun1 != 750)
				{
					ChatPacket(CHAT_TYPE_INFO, "Önce bir önceki seviyedeki nesneyi kullanmalýsýn.");
					return false;
				}

				if (bendeki == 750 && efsun1 != 775)
				{
					ChatPacket(CHAT_TYPE_INFO, "Önce bir önceki seviyedeki nesneyi kullanmalýsýn.");
					return false;
				}

				if (bendeki == 775 && efsun1 != 800)
				{
					ChatPacket(CHAT_TYPE_INFO, "Önce bir önceki seviyedeki nesneyi kullanmalýsýn.");
					return false;
				}

				if (bendeki == 800 && efsun1 != 825)
				{
					ChatPacket(CHAT_TYPE_INFO, "Önce bir önceki seviyedeki nesneyi kullanmalýsýn.");
					return false;
				}

				if (bendeki == 825 && efsun1 != 850)
				{
					ChatPacket(CHAT_TYPE_INFO, "Önce bir önceki seviyedeki nesneyi kullanmalýsýn.");
					return false;
				}

				if (bendeki == 850 && efsun1 != 875)
				{
					ChatPacket(CHAT_TYPE_INFO, "Önce bir önceki seviyedeki nesneyi kullanmalýsýn.");
					return false;
				}

				if (bendeki == 875 && efsun1 != 900)
				{
					ChatPacket(CHAT_TYPE_INFO, "Önce bir önceki seviyedeki nesneyi kullanmalýsýn.");
					return false;
				}

				if (bendeki == 900 && efsun1 != 925)
				{
					ChatPacket(CHAT_TYPE_INFO, "Önce bir önceki seviyedeki nesneyi kullanmalýsýn.");
					return false;
				}

				if (bendeki == 925 && efsun1 != 950)
				{
					ChatPacket(CHAT_TYPE_INFO, "Önce bir önceki seviyedeki nesneyi kullanmalýsýn.");
					return false;
				}

				if (bendeki == 950 && efsun1 != 975)
				{
					ChatPacket(CHAT_TYPE_INFO, "Önce bir önceki seviyedeki nesneyi kullanmalýsýn.");
					return false;
				}

				if (bendeki == 975 && efsun1 != 1000)
				{
					ChatPacket(CHAT_TYPE_INFO, "Önce bir önceki seviyedeki nesneyi kullanmalýsýn.");
					return false;
				}

				if (CheckAffectValue(AFFECT_YILBASI, POINT_ATTBONUS_MONSTER) < efsun1)
				{
					if (FindAffect(AFFECT_YILBASI))
						RemoveAffect(AFFECT_YILBASI);
				}
				else
				{
					ChatPacket(CHAT_TYPE_INFO, "Daha düþük bir etki gücündeki nesneyi kullanamazsýnýz.");
					return false;
				}


				AddAffect(AFFECT_YILBASI, POINT_ATTBONUS_MONSTER, efsun1, AFF_NONE, INFINITE_AFFECT_DURATION, 0, true, true);
				AddAffect(AFFECT_YILBASI, POINT_ATTBONUS_STONE, efsun1, AFF_NONE, INFINITE_AFFECT_DURATION, 0, true, true);
				AddAffect(AFFECT_YILBASI, POINT_NORMAL_HIT_DAMAGE_BONUS, efsun2, AFF_NONE, INFINITE_AFFECT_DURATION, 0, true, true);
				AddAffect(AFFECT_YILBASI, POINT_MALL_ATTBONUS, efsun2, AFF_NONE, INFINITE_AFFECT_DURATION, 0, true, true);
				AddAffect(AFFECT_YILBASI, POINT_ATT_GRADE_BONUS, efsun3, AFF_NONE, INFINITE_AFFECT_DURATION, 0, true, true);
				item->SetCount(item->GetCount() - 1);
			}
			break;

			case 30254:
			{
				if (FindAffect(AFFECT_OTOMATIKAV))
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ¹Ì È¿°ú°¡ °É·Á ÀÖ½À´Ï´Ù."));
					return false;
				}

				AddAffect(AFFECT_OTOMATIKAV, POINT_NONE, 0, AFF_NONE, get_global_time()+7*24*60*60, 0, true, true);
				item->SetCount(item->GetCount() - 1);
			}
			break;

			case 306604:
			{
				if (FindAffect(AFFECT_OTOMATIKAV))
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ¹Ì È¿°ú°¡ °É·Á ÀÖ½À´Ï´Ù."));
					return false;
				}

				AddAffect(AFFECT_OTOMATIKAV, POINT_NONE, 0, AFF_NONE, get_global_time()+7*24*60*60, 0, true, true);
				item->SetCount(item->GetCount() - 1);
			}
			break;

			case 306605:
			{
				if (FindAffect(AFFECT_OTOMATIKAV))
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ¹Ì È¿°ú°¡ °É·Á ÀÖ½À´Ï´Ù."));
					return false;
				}

				AddAffect(AFFECT_OTOMATIKAV, POINT_NONE, 0, AFF_NONE, get_global_time()+14*24*60*60, 0, true, true);
				item->SetCount(item->GetCount() - 1);
			}
			break;

			case 306606:
			{
				if (FindAffect(AFFECT_OTOMATIKAV))
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ¹Ì È¿°ú°¡ °É·Á ÀÖ½À´Ï´Ù."));
					return false;
				}

				AddAffect(AFFECT_OTOMATIKAV, POINT_NONE, 0, AFF_NONE, get_global_time()+7*24*60*60, 0, true, true);
				item->SetCount(item->GetCount() - 1);
			}
			break;

			case 306607:
			{
				if (FindAffect(AFFECT_OTOMATIKAV))
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ¹Ì È¿°ú°¡ °É·Á ÀÖ½À´Ï´Ù."));
					return false;
				}

				AddAffect(AFFECT_OTOMATIKAV, POINT_NONE, 0, AFF_NONE, get_global_time()+14*24*60*60, 0, true, true);
				item->SetCount(item->GetCount() - 1);
			}
			break;

			case 306600:
			{
				if (FindAffect(AFFECT_OTOMATIKAV))
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ¹Ì È¿°ú°¡ °É·Á ÀÖ½À´Ï´Ù."));
					return false;
				}

				AddAffect(AFFECT_OTOMATIKAV, POINT_NONE, 0, AFF_NONE, get_global_time()+1*60*60, 0, true, true);
				item->SetCount(item->GetCount() - 1);
			}
			break;

			case 306601:
			{
				if (FindAffect(AFFECT_OTOMATIKAV))
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ¹Ì È¿°ú°¡ °É·Á ÀÖ½À´Ï´Ù."));
					return false;
				}

				AddAffect(AFFECT_OTOMATIKAV, POINT_NONE, 0, AFF_NONE, get_global_time()+2*60*60, 0, true, true);
				item->SetCount(item->GetCount() - 1);
			}
			break;

			case 306602:
			{
				if (FindAffect(AFFECT_OTOMATIKAV))
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ¹Ì È¿°ú°¡ °É·Á ÀÖ½À´Ï´Ù."));
					return false;
				}

				AddAffect(AFFECT_OTOMATIKAV, POINT_NONE, 0, AFF_NONE, get_global_time()+3*60*60, 0, true, true);
				item->SetCount(item->GetCount() - 1);
			}
			break;

			case 306603:
			{
				if (FindAffect(AFFECT_OTOMATIKAV))
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ¹Ì È¿°ú°¡ °É·Á ÀÖ½À´Ï´Ù."));
					return false;
				}

				AddAffect(AFFECT_OTOMATIKAV, POINT_NONE, 0, AFF_NONE, get_global_time()+6*60*60, 0, true, true);
				item->SetCount(item->GetCount() - 1);
			}
			break;


#ifdef __EXP_RING_ITEMS__
			case NEW_EXP_RING1:
			{
				if (FindAffect(AFFECT_NEW_EXP1) || FindAffect(AFFECT_NEW_EXP2) || FindAffect(AFFECT_NEW_EXP3) || FindAffect(AFFECT_NEW_EXP4) || FindAffect(AFFECT_NEW_EXP5) || FindAffect(AFFECT_NEW_EXP6))
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ¹Ì È¿°ú°¡ °É·Á ÀÖ½À´Ï´Ù."));
					return false;
				}
				AddAffect(AFFECT_NEW_EXP1, POINT_EXP_DOUBLE_BONUS, 75, AFF_NONE, 3600, 0, true, true);
				item->SetCount(item->GetCount() - 1);
			}
			break;
			case NEW_EXP_RING2:
			{
				if (FindAffect(AFFECT_NEW_EXP1) || FindAffect(AFFECT_NEW_EXP2) || FindAffect(AFFECT_NEW_EXP3) || FindAffect(AFFECT_NEW_EXP4) || FindAffect(AFFECT_NEW_EXP5) || FindAffect(AFFECT_NEW_EXP6))
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ¹Ì È¿°ú°¡ °É·Á ÀÖ½À´Ï´Ù."));
					return false;
				}
				AddAffect(AFFECT_NEW_EXP2, POINT_EXP_DOUBLE_BONUS, 100, AFF_NONE, 3600, 0, true, true);
				item->SetCount(item->GetCount() - 1);
			}
			break;
			case NEW_EXP_RING3:
			{
				if (FindAffect(AFFECT_NEW_EXP1) || FindAffect(AFFECT_NEW_EXP2) || FindAffect(AFFECT_NEW_EXP3) || FindAffect(AFFECT_NEW_EXP4) || FindAffect(AFFECT_NEW_EXP5) || FindAffect(AFFECT_NEW_EXP6))
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ¹Ì È¿°ú°¡ °É·Á ÀÖ½À´Ï´Ù."));
					return false;
				}
				AddAffect(AFFECT_NEW_EXP3, POINT_EXP_DOUBLE_BONUS, 125, AFF_NONE, 3600, 0, true, true);
				item->SetCount(item->GetCount() - 1);
			}
			break;
			case NEW_EXP_RING4:
			{
				if (FindAffect(AFFECT_NEW_EXP1) || FindAffect(AFFECT_NEW_EXP2) || FindAffect(AFFECT_NEW_EXP3) || FindAffect(AFFECT_NEW_EXP4) || FindAffect(AFFECT_NEW_EXP5) || FindAffect(AFFECT_NEW_EXP6))
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ¹Ì È¿°ú°¡ °É·Á ÀÖ½À´Ï´Ù."));
					return false;
				}
				AddAffect(AFFECT_NEW_EXP4, POINT_EXP_DOUBLE_BONUS, 150, AFF_NONE, 3600, 0, true, true);
				item->SetCount(item->GetCount() - 1);
			}
			break;
			case NEW_EXP_RING5:
			{
				if (FindAffect(AFFECT_NEW_EXP1) || FindAffect(AFFECT_NEW_EXP2) || FindAffect(AFFECT_NEW_EXP3) || FindAffect(AFFECT_NEW_EXP4) || FindAffect(AFFECT_NEW_EXP5) || FindAffect(AFFECT_NEW_EXP6))
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ¹Ì È¿°ú°¡ °É·Á ÀÖ½À´Ï´Ù."));
					return false;
				}
				AddAffect(AFFECT_NEW_EXP5, POINT_EXP_DOUBLE_BONUS, 175, AFF_NONE, 3600, 0, true, true);
				item->SetCount(item->GetCount() - 1);
			}
			break;
			case NEW_EXP_RING6:
			{
				if (FindAffect(AFFECT_NEW_EXP1) || FindAffect(AFFECT_NEW_EXP2) || FindAffect(AFFECT_NEW_EXP3) || FindAffect(AFFECT_NEW_EXP4) || FindAffect(AFFECT_NEW_EXP5) || FindAffect(AFFECT_NEW_EXP6))
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ¹Ì È¿°ú°¡ °É·Á ÀÖ½À´Ï´Ù."));
					return false;
				}
				AddAffect(AFFECT_NEW_EXP6, POINT_EXP_DOUBLE_BONUS, 200, AFF_NONE, 3600, 0, true, true);
				item->SetCount(item->GetCount() - 1);
			}
			break;
#endif

#ifdef __POTION_AS_REWORK__
			case 57031:
				{
					if (FindAffect(AFFECT_NEW_SEBO_1))
					{
						RemoveAffect(AFFECT_NEW_SEBO_1);
						item->Lock(false);
						item->SetSocket(0, 0);
					}
					else
					{
						AddAffect(AFFECT_NEW_SEBO_1, POINT_ATTBONUS_STONE, 50, AFF_NONE, INFINITE_AFFECT_DURATION, 0, true, true);
						AddAffect(AFFECT_NEW_SEBO_1, POINT_ATTBONUS_MONSTER, 50, AFF_NONE, INFINITE_AFFECT_DURATION, 0, true, true);
						AddAffect(AFFECT_NEW_SEBO_1, POINT_ATTBONUS_BOSS, 50, AFF_NONE, INFINITE_AFFECT_DURATION, 0, true, true);
						AddAffect(AFFECT_NEW_SEBO_1, POINT_NORMAL_HIT_DAMAGE_BONUS, 30, AFF_NONE, INFINITE_AFFECT_DURATION, 0, true, true);
						AddAffect(AFFECT_NEW_SEBO_1, POINT_ATTBONUS_UNDEAD, 50, AFF_NONE, INFINITE_AFFECT_DURATION, 0, true, true);
						AddAffect(AFFECT_NEW_SEBO_1, POINT_ATTBONUS_DEVIL, 50, AFF_NONE, INFINITE_AFFECT_DURATION, 0, true, true);
						AddAffect(AFFECT_NEW_SEBO_1, POINT_ATTBONUS_ANIMAL, 50, AFF_NONE, INFINITE_AFFECT_DURATION, 0, true, true);
						AddAffect(AFFECT_NEW_SEBO_1, POINT_ATTBONUS_MILGYO, 50, AFF_NONE, INFINITE_AFFECT_DURATION, 0, true, true);
						AddAffect(AFFECT_NEW_SEBO_1, POINT_MALL_ATTBONUS, 20, AFF_NONE, INFINITE_AFFECT_DURATION, 0, true, true);
						AddAffect(AFFECT_NEW_SEBO_1, POINT_ATT_GRADE_BONUS, 200, AFF_NONE, INFINITE_AFFECT_DURATION, 0, true, true);
						item->Lock(true);
						item->SetSocket(0, 1);
					}
				}
				break;

			case 57032:
				{
					if (FindAffect(AFFECT_NEW_SEBO_2))
					{
						RemoveAffect(AFFECT_NEW_SEBO_2);
						item->Lock(false);
						item->SetSocket(0, 0);
					}
					else
					{
						AddAffect(AFFECT_NEW_SEBO_2, POINT_ATTBONUS_STONE, 50, AFF_NONE, INFINITE_AFFECT_DURATION, 0, true, true);
						AddAffect(AFFECT_NEW_SEBO_2, POINT_ATTBONUS_MONSTER, 50, AFF_NONE, INFINITE_AFFECT_DURATION, 0, true, true);
						AddAffect(AFFECT_NEW_SEBO_2, POINT_ATTBONUS_BOSS, 50, AFF_NONE, INFINITE_AFFECT_DURATION, 0, true, true);
						AddAffect(AFFECT_NEW_SEBO_2, POINT_NORMAL_HIT_DAMAGE_BONUS, 30, AFF_NONE, INFINITE_AFFECT_DURATION, 0, true, true);
						AddAffect(AFFECT_NEW_SEBO_2, POINT_ATTBONUS_UNDEAD, 50, AFF_NONE, INFINITE_AFFECT_DURATION, 0, true, true);
						AddAffect(AFFECT_NEW_SEBO_2, POINT_ATTBONUS_DEVIL, 50, AFF_NONE, INFINITE_AFFECT_DURATION, 0, true, true);
						AddAffect(AFFECT_NEW_SEBO_2, POINT_ATTBONUS_ANIMAL, 50, AFF_NONE, INFINITE_AFFECT_DURATION, 0, true, true);
						AddAffect(AFFECT_NEW_SEBO_2, POINT_ATTBONUS_MILGYO, 50, AFF_NONE, INFINITE_AFFECT_DURATION, 0, true, true);
						AddAffect(AFFECT_NEW_SEBO_2, POINT_MALL_ATTBONUS, 20, AFF_NONE, INFINITE_AFFECT_DURATION, 0, true, true);
						AddAffect(AFFECT_NEW_SEBO_2, POINT_ATT_GRADE_BONUS, 200, AFF_NONE, INFINITE_AFFECT_DURATION, 0, true, true);
						item->Lock(true);
						item->SetSocket(0, 1);
					}
				}
				break;
#endif // __POTION_AS_REWORK__

			case 50950:
			case 50951:
			case 50952:
			case 99360:
			case 99361:
			case 57040:
			case 57052:
			case 57064:
			case 57076:
			case 57088:
			{
				if (FindAffect(AFFECT_RAMADAN_NEW1))
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ¹Ì È¿°ú°¡ °É·Á ÀÖ½À´Ï´Ù."));
					return false;
				}
				long time = item->GetValue(0);
				long efsun1 = item->GetValue(1);
				AddAffect(AFFECT_RAMADAN_NEW1, POINT_ATTBONUS_MONSTER, efsun1, AFF_NONE, time, 0, true, true);
				item->SetCount(item->GetCount() - 1);
			}
			break;

			case 50955:
			case 50956:
			case 50957:
			case 99362:
			case 99363:
			case 57041:
			case 57053:
			case 57065:
			case 57077:
			case 57089:
			{
				if (FindAffect(AFFECT_RAMADAN_NEW2))
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ¹Ì È¿°ú°¡ °É·Á ÀÖ½À´Ï´Ù."));
					return false;
				}
				long time = item->GetValue(0);
				long efsun1 = item->GetValue(1);
				AddAffect(AFFECT_RAMADAN_NEW2, POINT_ATTBONUS_STONE, efsun1, AFF_NONE, time, 0, true, true);
				item->SetCount(item->GetCount() - 1);
			}
			break;

			case 50960:
			case 50961:
			case 50962:
			case 99364:
			case 99365:
			case 57042:
			case 57054:
			case 57066:
			case 57078:
			case 57090:
			{
				if (FindAffect(AFFECT_RAMADAN_NEW3))
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ¹Ì È¿°ú°¡ °É·Á ÀÖ½À´Ï´Ù."));
					return false;
				}
				long time = item->GetValue(0);
				long efsun1 = item->GetValue(1);
				AddAffect(AFFECT_RAMADAN_NEW3, POINT_ATTBONUS_BOSS, efsun1, AFF_NONE, time, 0, true, true);
				item->SetCount(item->GetCount() - 1);
			}
			break;

			case 50965:
			case 50966:
			case 50967:
			case 99366:
			case 99367:
			case 57043:
			case 57055:
			case 57067:
			case 57079:
			case 57091:
			{
				if (FindAffect(AFFECT_RAMADAN_NEW4))
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ¹Ì È¿°ú°¡ °É·Á ÀÖ½À´Ï´Ù."));
					return false;
				}
				long time = item->GetValue(0);
				long efsun1 = item->GetValue(1);
				AddAffect(AFFECT_RAMADAN_NEW4, POINT_MALL_ATTBONUS, efsun1, AFF_NONE, time, 0, true, true);
				item->SetCount(item->GetCount() - 1);
			}
			break;

			case 50970:
			case 50971:
			case 50972:
			case 99368:
			case 99369:
			case 57044:
			case 57056:
			case 57068:
			case 57080:
			case 57092:
			{
				if (FindAffect(AFFECT_RAMADAN_NEW5))
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ¹Ì È¿°ú°¡ °É·Á ÀÖ½À´Ï´Ù."));
					return false;
				}
				long time = item->GetValue(0);
				long efsun1 = item->GetValue(1);
				AddAffect(AFFECT_RAMADAN_NEW5, POINT_ATT_GRADE_BONUS, efsun1, AFF_NONE, time, 0, true, true);
				item->SetCount(item->GetCount() - 1);
			}
			break;


#ifdef __BATTLE_PASS_SYSTEM__
			case 19950:
			case 19951:
			case 19952:
			case 19953:
			case 19954:
			case 19955:
			case 19956:
			case 19957:
			{
				char szQuery[1024];
				snprintf(szQuery, sizeof(szQuery), "SELECT * FROM battle_pass_ranking WHERE player_name = '%s' AND battle_pass_id = %ld", GetName(), item->GetValue(0));
				std::unique_ptr<SQLMsg> pmsg(DBManager::instance().DirectQuery(szQuery));
				
				if (pmsg->Get()->uiNumRows > 0)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("BATTLE_PASS_ALREADY_COMPLETED"));
					return false;
				}

				if (CBattlePass::Instance().CanUseBattlePass(this, item->GetValue(0)) == false)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("BATTLE_PASS_ALREADY_COMPLETED_WAIT_NEXT_WEEK"));
					return false;
				}

				if (FindAffect(AFFECT_BATTLE_PASS_PREMIUM))
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("BATTLE_PASS_ALREADY_USED"));
					return false;
				}

				if (FindAffect(AFFECT_BATTLE_PASS))
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("BATTLE_PASS_ALREADY_USED"));
					return false;
				}
				else
				{
					m_dwBattlePassEndTime = get_global_time() + 86400*12;
					
					AddAffect(AFFECT_BATTLE_PASS, POINT_BATTLE_PASS_ID, item->GetValue(0), 0, 86400*12, 0, true);
					ITEM_MANAGER::instance().RemoveItem(item);
				}
			}
			break;

			case 19958:
			case 19959:
			case 19960:
			case 19961:
			case 19962:
			case 19963:
			case 19964:
			case 19965:
			{
				char szQuery[1024];
				snprintf(szQuery, sizeof(szQuery), "SELECT * FROM battle_pass_ranking WHERE player_name = '%s' AND battle_pass_id = %ld;", GetName(), item->GetValue(0));
				std::unique_ptr<SQLMsg> pmsg(DBManager::instance().DirectQuery(szQuery));
				
				if (pmsg->Get()->uiNumRows > 0)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("BATTLE_PASS_ALREADY_COMPLETED"));
					return false;
				}
				
				//if (CBattlePass::Instance().CanUseBattlePass(this, item->GetValue(0)) == false)
				//{
				//	ChatPacket(CHAT_TYPE_INFO, LC_TEXT("BATTLE_PASS_ALREADY_COMPLETED_WAIT_NEXT_WEEK"));
				//	return false;
				//}

				if (FindAffect(AFFECT_BATTLE_PASS))
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("BATTLE_PASS_ALREADY_USED"));
					return false;
				}
				
				if (FindAffect(AFFECT_BATTLE_PASS_PREMIUM))
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("BATTLE_PASS_ALREADY_USED"));
					return false;
				}
				else
				{
					m_dwBattlePassEndTime = get_global_time() + 86400*365;
					
					AddAffect(AFFECT_BATTLE_PASS_PREMIUM, POINT_BATTLE_PASS_ID, item->GetValue(0), 0, 86400*365, 0, true);
					ITEM_MANAGER::instance().RemoveItem(item);
				}
			}
			break;

			case 19970:
			{
				auto pkAff = FindAffect(AFFECT_BATTLE_PASS_PREMIUM);
				if (!pkAff)
				{
					pkAff = FindAffect(AFFECT_BATTLE_PASS);
					if (!pkAff)
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("BATTLE_PASS_NON_ACTIVATED"));
						return false;
					}
				}

				if (GetBattlePassEndTime() > time(0))
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("BATTLE_PASS_ALREADY_COMPLETED_WAIT_NEXT_WEEK"));
					return false;
				}

				RemoveAffect(pkAff);
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("BATTLE_PASS_RESET_SUCCESS"));
				ITEM_MANAGER::instance().RemoveItem(item);
			}
			break;
#endif // __BATTLE_PASS_SYSTEM__
			case 50975:
			case 50976:
			case 50977:
			case 99370:
			case 99371:
			case 57045:
			case 57057:
			case 57069:
			case 57081:
			case 57093:
			{
				if (FindAffect(AFFECT_RAMADAN_NEW6))
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ¹Ì È¿°ú°¡ °É·Á ÀÖ½À´Ï´Ù."));
					return false;
				}
				long time = item->GetValue(0);
				long efsun1 = item->GetValue(1);
				long efsun2 = item->GetValue(2);
				long efsun3 = item->GetValue(3);
				long efsun4 = item->GetValue(4);
				AddAffect(AFFECT_RAMADAN_NEW6, POINT_ATTBONUS_UNDEAD, efsun1, AFF_NONE, time, 0, true, true);
				AddAffect(AFFECT_RAMADAN_NEW6, POINT_ATTBONUS_DEVIL, efsun2, AFF_NONE, time, 0, true, true);
				AddAffect(AFFECT_RAMADAN_NEW6, POINT_ATTBONUS_MILGYO, efsun3, AFF_NONE, time, 0, true, true);
				AddAffect(AFFECT_RAMADAN_NEW6, POINT_ATTBONUS_ANIMAL, efsun4, AFF_NONE, time, 0, true, true);
				item->SetCount(item->GetCount() - 1);
			}
			break;

			case 50980:
			case 50981:
			case 50982:
			case 99372:
			case 99373:
			case 57046:
			case 57058:
			case 57070:
			case 57082:
			case 57094:
			{
				if (FindAffect(AFFECT_RAMADAN_NEW7))
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ¹Ì È¿°ú°¡ °É·Á ÀÖ½À´Ï´Ù."));
					return false;
				}
				long time = item->GetValue(0);
				long efsun1 = item->GetValue(1);
				AddAffect(AFFECT_RAMADAN_NEW7, POINT_ATTBONUS_ELEMENTS, efsun1, AFF_NONE, time, 0, true, true);
				item->SetCount(item->GetCount() - 1);
			}
			break;

			case 50985:
			case 50986:
			case 50987:
			case 99374:
			case 99375:
			case 57047:
			case 57059:
			case 57071:
			case 57083:
			case 57095:
			{
				if (FindAffect(AFFECT_RAMADAN_NEW8))
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ¹Ì È¿°ú°¡ °É·Á ÀÖ½À´Ï´Ù."));
					return false;
				}
				long time = item->GetValue(0);
				long efsun1 = item->GetValue(1);
				AddAffect(AFFECT_RAMADAN_NEW8, POINT_MOV_SPEED, efsun1, AFF_NONE, time, 0, true, true);
				item->SetCount(item->GetCount() - 1);
			}
			break;

#ifdef ENABLE_PB2_PREMIUM_SYSTEM
			case 19181:
			{
				if (FindAffect(AFFECT_PB2_GLOBAL_CHAT))
				{
					ChatPacket(CHAT_TYPE_INFO, "Zaten bu özelliðe sahipsiniz.");
					return false;
				}

				AddAffect(AFFECT_PB2_GLOBAL_CHAT, POINT_MALL_ATTBONUS, 20, AFF_NONE, INFINITE_AFFECT_DURATION, 0, true, true);
				ITEM_MANAGER::instance().RemoveItem(item);
			}
			break;
			case 19179:
			{
				if (FindAffect(AFFECT_PB2_SKILL_COLOR))
				{
					ChatPacket(CHAT_TYPE_INFO, "Zaten bu özelliðe sahipsiniz.");
					return false;
				}

				AddAffect(AFFECT_PB2_SKILL_COLOR, POINT_NORMAL_HIT_DAMAGE_BONUS, 20, AFF_NONE, INFINITE_AFFECT_DURATION, 0, true, true);
				ITEM_MANAGER::instance().RemoveItem(item);
			}
			break;
#endif // ENABLE_PB2_PREMIUM_SYSTEM

#ifdef ENABLE_PB2_NEW_TAG_SYSTEM
			case 19188:
			case 19189:
			case 19190:
			case 19191:
			case 19192:
			case 19193:
			case 19194:
			{
				auto affectType = item->GetValue(0);

				if (FindAffect(affectType))
				{
					ChatPacket(CHAT_TYPE_INFO, "Zaten bir taga sahipsiniz.");
					return false;
				}

				for (int i = AFFECT_PB2_TAG_VIP; i < AFFECT_PB2_TAG_BARRIER; ++i)
				{
					if (FindAffect(i))
					{
						RemoveAffect(i);
					}
				}

				auto affType = item->GetValue(2);

				AddAffect(affectType, POINT_NORMAL_HIT_DAMAGE_BONUS, 20, affType, INFINITE_AFFECT_DURATION, 0, true, true);
				ITEM_MANAGER::instance().RemoveItem(item);
			}
			break;
		
			case 19195:
				{
					bool bContinue = false;
					int bTag = 0;
					for (int i = AFFECT_PB2_TAG_VIP; i < AFFECT_PB2_TAG_BARRIER; ++i)
					{
						if (FindAffect(i))
						{
							bContinue = true;
							bTag = i;
						}
					}

					if (bContinue == false)
					{
						ChatPacket(CHAT_TYPE_INFO, "Bir taga sahip degilsiniz.");
						return false;
					}

					RemoveAffect(bTag);
					ITEM_MANAGER::instance().RemoveItem(item);
				}
				break;
#endif // ENABLE_PB2_NEW_TAG_SYSTEM


#ifdef __NOG_PACKETS__
			case 99355:
			case 99356:
			case 99357:
			case 99376:
			case 99377:
			case 57048:
			case 57060:
			case 57072:
			case 57084:
			case 57096:
			{
				if (FindAffect(AFFECT_KINDER))
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ¹Ì È¿°ú°¡ °É·Á ÀÖ½À´Ï´Ù."));
					return false;
				}
				long time = item->GetValue(0);
				long monsterPer = item->GetValue(1);
				long stonePer = item->GetValue(2);
				long bossPer = item->GetValue(3);
				long attbonusPer = item->GetValue(4);
				long hitbonusPer = item->GetValue(5);
				AddAffect(AFFECT_KINDER, POINT_ATTBONUS_MONSTER, monsterPer, AFF_NONE, time, 0, true, true);
				AddAffect(AFFECT_KINDER, POINT_ATTBONUS_STONE, stonePer, AFF_NONE, time, 0, true, true);
				AddAffect(AFFECT_KINDER, POINT_ATTBONUS_BOSS, bossPer, AFF_NONE, time, 0, true, true);
				AddAffect(AFFECT_KINDER, POINT_NORMAL_HIT_DAMAGE_BONUS, attbonusPer, AFF_NONE, time, 0, true, true);
				AddAffect(AFFECT_KINDER, POINT_ATT_GRADE_BONUS, hitbonusPer, AFF_NONE, time, 0, true, true);
				item->SetCount(item->GetCount() - 1);
			}
			break;
			case 99359:
			{
				if (FindAffect(AFFECT_GUL))
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ¹Ì È¿°ú°¡ °É·Á ÀÖ½À´Ï´Ù."));
					return false;
				}
				AddAffect(AFFECT_GUL, POINT_MAX_HP, 1000, AFF_NONE, 172800, 0, true, true);
				AddAffect(AFFECT_GUL, POINT_ATTBONUS_BOSS, 25, AFF_NONE, 172800, 0, true, true);
				AddAffect(AFFECT_GUL, POINT_NORMAL_HIT_DAMAGE_BONUS, 5, AFF_NONE, 172800, 0, true, true);
				AddAffect(AFFECT_GUL, POINT_ATT_GRADE_BONUS, 50, AFF_NONE, 172800, 0, true, true);
				AddAffect(AFFECT_GUL, POINT_EXP_DOUBLE_BONUS, 15, AFF_NONE, 172800, 0, true, true);
				AddAffect(AFFECT_GUL, POINT_ATT_SPEED, 10, AFF_NONE, 172800, 0, true, true);
				AddAffect(AFFECT_GUL, POINT_CASTING_SPEED, 10, AFF_NONE, 172800, 0, true, true);
				AddAffect(AFFECT_GUL, POINT_MOV_SPEED, 10, AFF_NONE, 172800, 0, true, true);
				AddAffect(AFFECT_GUL, POINT_MAX_HP_PCT, 10, AFF_NONE, 172800, 0, true, true);
				AddAffect(AFFECT_GUL, POINT_MAX_SP_PCT, 10, AFF_NONE, 172800, 0, true, true);
				ITEM_MANAGER::instance().RemoveItem(item);
			}
			break;
			case 50001:
			{
				if (FindAffect(AFFECT_NOG_ABILITY))
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ¹Ì È¿°ú°¡ °É·Á ÀÖ½À´Ï´Ù."));
					return false;
				}
				long time = item->GetValue(0);
				long ability = item->GetValue(1);
				AddAffect(AFFECT_NOG_ABILITY, POINT_ATTBONUS_STONE, ability, AFF_NONE, time, 0, true, true);
				item->SetCount(item->GetCount() - 1);
			}
			break;
			case 50050:
			{
				if (FindAffect(AFFECT_ICECREAM_ABILITY))
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ¹Ì È¿°ú°¡ °É·Á ÀÖ½À´Ï´Ù."));
					return false;
				}
				long time = item->GetValue(0);
				long ability = item->GetValue(1);
				AddAffect(AFFECT_ICECREAM_ABILITY, POINT_ATTBONUS_BOSS, ability, AFF_NONE, time, 0, true, true);
				item->SetCount(item->GetCount() - 1);
			}
			break;
			case 50003:
			{
				if (FindAffect(AFFECT_RAMADAN_ABILITY))
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ¹Ì È¿°ú°¡ °É·Á ÀÖ½À´Ï´Ù."));
					return false;
				}
				long time = item->GetValue(0);
				long ability = item->GetValue(1);
				AddAffect(AFFECT_RAMADAN_ABILITY, POINT_ATTBONUS_MONSTER, ability, AFF_NONE, time, 0, true, true);
				item->SetCount(item->GetCount() - 1);
			}
			break;

			case 51003:
			case 57051:
			case 57063:
			case 57075:
			case 57087:
			case 57099:
			{
				if (FindAffect(AFFECT_ICECREAM_ABILITY))
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ¹Ì È¿°ú°¡ °É·Á ÀÖ½À´Ï´Ù."));
					return false;
				}
				long time = item->GetValue(0);
				long efsun1 = item->GetValue(1);
				long efsun2 = item->GetValue(2);
				long efsun3 = item->GetValue(3);
				long efsun4 = item->GetValue(4);
				long efsun5 = item->GetValue(5);
				AddAffect(AFFECT_ICECREAM_ABILITY, POINT_MOV_SPEED, efsun1, AFF_MOV_SPEED_POTION, time, 0, true, true);
				AddAffect(AFFECT_ICECREAM_ABILITY, POINT_MALL_ATTBONUS, efsun2, AFF_NONE, time, 0, true, true);
				AddAffect(AFFECT_ICECREAM_ABILITY, POINT_ATTBONUS_MONSTER, efsun3, AFF_NONE, time, 0, true, true);
				AddAffect(AFFECT_ICECREAM_ABILITY, POINT_ATTBONUS_BOSS, efsun4, AFF_NONE, time, 0, true, true);
				AddAffect(AFFECT_ICECREAM_ABILITY, POINT_ATTBONUS_STONE, efsun5, AFF_NONE, time, 0, true, true);
				item->SetCount(item->GetCount() - 1);
			}
			break;
			case 99378:
			{
				if (FindAffect(AFFECT_BASLANGIC_ESYA))
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ¹Ì È¿°ú°¡ °É·Á ÀÖ½À´Ï´Ù."));
					return false;
				}
				AddAffect(AFFECT_BASLANGIC_ESYA, POINT_ATTBONUS_MONSTER, 1000, AFF_NONE, 604800, 0, true, true);
				AddAffect(AFFECT_BASLANGIC_ESYA, POINT_ATTBONUS_STONE, 1000, AFF_NONE, 604800, 0, true, true);
				AddAffect(AFFECT_BASLANGIC_ESYA, POINT_ATTBONUS_BOSS, 1000, AFF_NONE, 604800, 0, true, true);
				AddAffect(AFFECT_BASLANGIC_ESYA, POINT_MALL_ATTBONUS, 100, AFF_NONE, 604800, 0, true, true);
				AddAffect(AFFECT_BASLANGIC_ESYA, POINT_NORMAL_HIT_DAMAGE_BONUS, 100, AFF_NONE, 604800, 0, true, true);
				item->SetCount(item->GetCount() - 1);
			}
			break;
#endif
			case ITEM_RAMADAN_CANDY:
			case 57049:
			case 57061:
			case 57073:
			case 57085:
			case 57097:
			{
				if (FindAffect(AFFECT_RAMADAN_ABILITY))
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ¹Ì È¿°ú°¡ °É·Á ÀÖ½À´Ï´Ù."));
					return false;
				}

				long time = item->GetValue(0);
				long efsun1 = item->GetValue(1);
				long efsun2 = item->GetValue(2);
				long efsun3 = item->GetValue(3);
				long efsun4 = item->GetValue(4);
				long efsun5 = item->GetValue(5);
				AddAffect(AFFECT_RAMADAN_ABILITY, POINT_MOV_SPEED, efsun1, AFF_MOV_SPEED_POTION, time, 0, true, true);
				AddAffect(AFFECT_RAMADAN_ABILITY, POINT_MALL_ATTBONUS, efsun2, AFF_NONE, time, 0, true, true);
				AddAffect(AFFECT_RAMADAN_ABILITY, POINT_ATTBONUS_MONSTER, efsun3, AFF_NONE, time, 0, true, true);
				AddAffect(AFFECT_RAMADAN_ABILITY, POINT_ATTBONUS_BOSS, efsun4, AFF_NONE, time, 0, true, true);
				AddAffect(AFFECT_RAMADAN_ABILITY, POINT_ATTBONUS_STONE, efsun5, AFF_NONE, time, 0, true, true);
				item->SetCount(item->GetCount() - 1);
			}
			break;

			case UNIQUE_ITEM_CAPE_OF_COURAGE:
			case 70057:
			case REWARD_BOX_UNIQUE_ITEM_CAPE_OF_COURAGE:
				AggregateMonster();
				break;

			case UNIQUE_ITEM_WHITE_FLAG:
				ForgetMyAttacker();
				item->SetCount(item->GetCount() - 1);
				break;

			case UNIQUE_ITEM_TREASURE_BOX:
				break;

			/*
			case 76006:
			{
				LPSECTREE_MAP pMap = SECTREE_MANAGER::instance().GetMap(GetMapIndex());

				if (GetDungeon() || IS_DUNGEON_ZONE(GetMapIndex()))
					return false;

				if (pMap != NULL)
				{
#ifndef __STONE_DETECT_REWORK__
					item->SetSocket(0, item->GetSocket(0) + 1);
#endif
					FFindStone f;

					// <Factor> SECTREE::for_each -> SECTREE::for_each_entity
					pMap->for_each(f);

					if (f.m_mapStone.size() > 0)
					{
						std::map<DWORD, LPCHARACTER>::iterator stone = f.m_mapStone.begin();

						DWORD max = UINT_MAX;
						LPCHARACTER pTarget = stone->second;

						while (stone != f.m_mapStone.end())
						{
							DWORD dist = (DWORD)DISTANCE_SQRT(GetX() - stone->second->GetX(), GetY() - stone->second->GetY());

							if (dist != 0 && max > dist)
							{
								pTarget = stone->second;
								if (pTarget && (pTarget->GetHP() != pTarget->GetMaxHP()))
								{
									stone++;
									continue;
								}
								max = dist;
							}
							stone++;
						}

						if (pTarget != NULL)
						{
#ifdef __STONE_DETECT_REWORK__
							Show(pTarget->GetMapIndex(), pTarget->GetX(), pTarget->GetY(), pTarget->GetZ());
#else
							int val = 3;

							if (max < 10000) val = 2;
							else if (max < 70000) val = 1;

							ChatPacket(CHAT_TYPE_COMMAND, "StoneDetect %u %d %d", (DWORD)GetVID(), val,
								(int)GetDegreeFromPositionXY(GetX(), pTarget->GetY(), pTarget->GetX(), GetY()));
#endif
						}
						else
						{
							ChatPacket(CHAT_TYPE_INFO, LC_TEXT("°¨Áö±â¸¦ ÀÛ¿ëÇÏ¿´À¸³ª °¨ÁöµÇ´Â ¿µ¼®ÀÌ ¾ø½À´Ï´Ù."));
						}
					}
					else
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("°¨Áö±â¸¦ ÀÛ¿ëÇÏ¿´À¸³ª °¨ÁöµÇ´Â ¿µ¼®ÀÌ ¾ø½À´Ï´Ù."));
					}

					if (item->GetSocket(0) >= 10000)
					{
						ChatPacket(CHAT_TYPE_COMMAND, "StoneDetect %u 0 0", (DWORD)GetVID());
						//ITEM_MANAGER::instance().RemoveItem(item);
					}
				}
				break;
			}
			break;

			case 27989:
			{
				LPSECTREE_MAP pMap = SECTREE_MANAGER::instance().GetMap(GetMapIndex());

				if (GetDungeon() || IS_DUNGEON_ZONE(GetMapIndex()))
					return false;

				if (pMap != NULL)
				{
					item->SetSocket(0, item->GetSocket(0) + 1);
					FFindStone f;

					// <Factor> SECTREE::for_each -> SECTREE::for_each_entity
					pMap->for_each(f);

					if (f.m_mapStone.size() > 0)
					{
						std::map<DWORD, LPCHARACTER>::iterator stone = f.m_mapStone.begin();

						DWORD max = UINT_MAX;
						LPCHARACTER pTarget = stone->second;

						while (stone != f.m_mapStone.end())
						{
							DWORD dist = (DWORD)DISTANCE_SQRT(GetX() - stone->second->GetX(), GetY() - stone->second->GetY());

							if (dist != 0 && max > dist)
							{
								pTarget = stone->second;
								if (pTarget && (pTarget->GetHP() != pTarget->GetMaxHP()))
								{
									stone++;
									continue;
								}
								max = dist;
							}
							stone++;
						}

						if (pTarget != NULL)
						{
#ifdef __STONE_DETECT_REWORK__
							Show(pTarget->GetMapIndex(), pTarget->GetX(), pTarget->GetY(), pTarget->GetZ());
#else
							int val = 3;

							if (max < 10000) val = 2;
							else if (max < 70000) val = 1;

							ChatPacket(CHAT_TYPE_COMMAND, "StoneDetect %u %d %d", (DWORD)GetVID(), val,
								(int)GetDegreeFromPositionXY(GetX(), pTarget->GetY(), pTarget->GetX(), GetY()));
#endif
						}
						else
						{
							ChatPacket(CHAT_TYPE_INFO, LC_TEXT("°¨Áö±â¸¦ ÀÛ¿ëÇÏ¿´À¸³ª °¨ÁöµÇ´Â ¿µ¼®ÀÌ ¾ø½À´Ï´Ù."));
						}
					}
					else
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("°¨Áö±â¸¦ ÀÛ¿ëÇÏ¿´À¸³ª °¨ÁöµÇ´Â ¿µ¼®ÀÌ ¾ø½À´Ï´Ù."));
					}

					if (item->GetSocket(0) >= 10000)
					{
						ChatPacket(CHAT_TYPE_COMMAND, "StoneDetect %u 0 0", (DWORD)GetVID());
						ITEM_MANAGER::instance().RemoveItem(item);
					}
				}
				break;
			}
			break;
			*/

#ifdef __SOULSTONE_SYSTEM__
			case 50513:
				if (GetSkillGroup() == 0)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("SOUL_CANNOT_READ_SKILLGROUP"));
					return false;
				}
#ifdef __SAGE_SKILL__ // __EXPERT_SKILL__
				ChatPacket(CHAT_TYPE_COMMAND, "ruhtasiekranac 0");
#else
				ChatPacket(CHAT_TYPE_COMMAND, "ruhtasiekranac");
#endif
				break;
#endif

#ifdef __SAGE_SKILL__
			case SKILL_OVER_P_ITEM_VNUM:
				if (GetSkillGroup() == 0)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("SOUL_CANNOT_READ_SKILLGROUP"));
					return false;
				}
				ChatPacket(CHAT_TYPE_COMMAND, "ruhtasiekranac 1");
				break;
#endif

#ifdef __EXPERT_SKILL__
			case SKILL_EXPERT_ITEM_VNUM:
				if (GetSkillGroup() == 0)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("SOUL_CANNOT_READ_SKILLGROUP"));
					return false;
				}
				ChatPacket(CHAT_TYPE_COMMAND, "ruhtasiekranac 2");
				break;
			case SKILL_DESTANSI_ITEM_VNUM:
				if (GetSkillGroup() == 0)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("SOUL_CANNOT_READ_SKILLGROUP"));
					return false;
				}
				ChatPacket(CHAT_TYPE_COMMAND, "ruhtasiekranac 3");
				break;
			case SKILL_TANRISAL_ITEM_VNUM:
				if (GetSkillGroup() == 0)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("SOUL_CANNOT_READ_SKILLGROUP"));
					return false;
				}
				ChatPacket(CHAT_TYPE_COMMAND, "ruhtasiekranac 4");
				break;
#endif

			case 27996:
				item->SetCount(item->GetCount() - 1);
				AttackedByPoison(NULL); // @warme008
				break;

			case 71013:
				CreateFly(number(FLY_FIREWORK1, FLY_FIREWORK6), this);
				item->SetCount(item->GetCount() - 1);
				break;

			case 50100:
			case 50101:
			case 50102:
			case 50103:
			case 50104:
			case 50105:
			case 50106:
				CreateFly(item->GetVnum() - 50100 + FLY_FIREWORK1, this);
				item->SetCount(item->GetCount() - 1);
				break;

			case 50200:
				__OpenPrivateShop();
				break;

			case fishing::FISH_MIND_PILL_VNUM:
				AddAffect(AFFECT_FISH_MIND_PILL, POINT_NONE, 0, AFF_FISH_MIND, 20 * 60, 0, true);
				item->SetCount(item->GetCount() - 1);
				break;

#ifdef __SUB_SKILL_REWORK__
			case 50301:
			case 50350:
			case 50360:
			case 50370:
			case 50380:
			{
				// @edit014 BEGIN
				if (!GetSkillGroup())
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You cant read skill book beacuse you dont choise skill group."));
					return false;
				}
				// @edit014 END

				int lv = GetSkillLevel(SKILL_SUB_MONSTER);

				if (lv < item->GetValue(0))
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ Ã¥Àº ³Ê¹« ¾î·Á¿ö ÀÌÇØÇÏ±â°¡ Èûµì´Ï´Ù."));
					return false;
				}

				if (lv >= item->GetValue(1))
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ Ã¥Àº ¾Æ¹«¸® ºÁµµ µµ¿òÀÌ µÉ °Í °°Áö ¾Ê½À´Ï´Ù."));
					return false;
				}

				if (LearnSkillByBook(SKILL_SUB_MONSTER))
				{
					item->SetCount(item->GetCount() - 1);

					int iReadDelay = number(SKILLBOOK_DELAY_MIN, SKILLBOOK_DELAY_MAX);

					SetSkillNextReadTime(SKILL_SUB_MONSTER, get_global_time() + iReadDelay);
				}
			}
			break;

			case 50302:
			case 50351:
			case 50361:
			case 50371:
			case 50381:
			{
				// @edit014 BEGIN
				if (!GetSkillGroup())
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You cant read skill book beacuse you dont choise skill group."));
					return false;
				}
				// @edit014 END

				int lv = GetSkillLevel(SKILL_SUB_STONE);

				if (lv < item->GetValue(0))
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ Ã¥Àº ³Ê¹« ¾î·Á¿ö ÀÌÇØÇÏ±â°¡ Èûµì´Ï´Ù."));
					return false;
				}

				if (lv >= item->GetValue(1))
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ Ã¥Àº ¾Æ¹«¸® ºÁµµ µµ¿òÀÌ µÉ °Í °°Áö ¾Ê½À´Ï´Ù."));
					return false;
				}

				if (LearnSkillByBook(SKILL_SUB_STONE))
				{
					item->SetCount(item->GetCount() - 1);

					int iReadDelay = number(SKILLBOOK_DELAY_MIN, SKILLBOOK_DELAY_MAX);

					SetSkillNextReadTime(SKILL_SUB_STONE, get_global_time() + iReadDelay);
				}
			}
			break;

			case 50303:
			case 50352:
			case 50362:
			case 50372:
			case 50382:
			{
				// @edit014 BEGIN
				if (!GetSkillGroup())
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You cant read skill book beacuse you dont choise skill group."));
					return false;
				}
				// @edit014 END

				int lv = GetSkillLevel(SKILL_SUB_BOSS);

				if (lv < item->GetValue(0))
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ Ã¥Àº ³Ê¹« ¾î·Á¿ö ÀÌÇØÇÏ±â°¡ Èûµì´Ï´Ù."));
					return false;
				}

				if (lv >= item->GetValue(1))
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ Ã¥Àº ¾Æ¹«¸® ºÁµµ µµ¿òÀÌ µÉ °Í °°Áö ¾Ê½À´Ï´Ù."));
					return false;
				}

				if (LearnSkillByBook(SKILL_SUB_BOSS))
				{
					item->SetCount(item->GetCount() - 1);

					int iReadDelay = number(SKILLBOOK_DELAY_MIN, SKILLBOOK_DELAY_MAX);

					SetSkillNextReadTime(SKILL_SUB_BOSS, get_global_time() + iReadDelay);
				}
			}
			break;

			case 50326:
			case 50353:
			case 50363:
			case 50373:
			case 50383:
			{
				// @edit014 BEGIN
				if (!GetSkillGroup())
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You cant read skill book beacuse you dont choise skill group."));
					return false;
				}
				// @edit014 END

				int lv = GetSkillLevel(SKILL_SUB_HUMAN);

				if (lv < item->GetValue(0))
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ Ã¥Àº ³Ê¹« ¾î·Á¿ö ÀÌÇØÇÏ±â°¡ Èûµì´Ï´Ù."));
					return false;
				}

				if (lv >= item->GetValue(1))
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ Ã¥Àº ¾Æ¹«¸® ºÁµµ µµ¿òÀÌ µÉ °Í °°Áö ¾Ê½À´Ï´Ù."));
					return false;
				}

				if (LearnSkillByBook(SKILL_SUB_HUMAN))
				{
					item->SetCount(item->GetCount() - 1);

					int iReadDelay = number(SKILLBOOK_DELAY_MIN, SKILLBOOK_DELAY_MAX);

					SetSkillNextReadTime(SKILL_SUB_HUMAN, get_global_time() + iReadDelay);
				}
			}
			break;

			case 50327:
			case 50354:
			case 50364:
			case 50374:
			case 50384:
			{
				// @edit014 BEGIN
				if (!GetSkillGroup())
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You cant read skill book beacuse you dont choise skill group."));
					return false;
				}
				// @edit014 END

				int lv = GetSkillLevel(SKILL_SUB_BERSERKER);

				if (lv < item->GetValue(0))
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ Ã¥Àº ³Ê¹« ¾î·Á¿ö ÀÌÇØÇÏ±â°¡ Èûµì´Ï´Ù."));
					return false;
				}

				if (lv >= item->GetValue(1))
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ Ã¥Àº ¾Æ¹«¸® ºÁµµ µµ¿òÀÌ µÉ °Í °°Áö ¾Ê½À´Ï´Ù."));
					return false;
				}

				if (LearnSkillByBook(SKILL_SUB_BERSERKER))
				{
					item->SetCount(item->GetCount() - 1);

					int iReadDelay = number(SKILLBOOK_DELAY_MIN, SKILLBOOK_DELAY_MAX);

					SetSkillNextReadTime(SKILL_SUB_BERSERKER, get_global_time() + iReadDelay);
				}
			}
			break;

			case 50328:
			case 50355:
			case 50365:
			case 50375:
			case 50385:
			{
				// @edit014 BEGIN
				if (!GetSkillGroup())
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You cant read skill book beacuse you dont choise skill group."));
					return false;
				}
				// @edit014 END

				int lv = GetSkillLevel(SKILL_ADD_HP);

				if (lv < item->GetValue(0))
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ Ã¥Àº ³Ê¹« ¾î·Á¿ö ÀÌÇØÇÏ±â°¡ Èûµì´Ï´Ù."));
					return false;
				}

				if (lv >= item->GetValue(1))
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ Ã¥Àº ¾Æ¹«¸® ºÁµµ µµ¿òÀÌ µÉ °Í °°Áö ¾Ê½À´Ï´Ù."));
					return false;
				}

				if (LearnSkillByBook(SKILL_ADD_HP))
				{
					item->SetCount(item->GetCount() - 1);

					int iReadDelay = number(SKILLBOOK_DELAY_MIN, SKILLBOOK_DELAY_MAX);

					SetSkillNextReadTime(SKILL_ADD_HP, get_global_time() + iReadDelay);
				}
			}
			break;

			case 50329:
			case 50356:
			case 50366:
			case 50376:
			case 50386:
			{
				// @edit014 BEGIN
				if (!GetSkillGroup())
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You cant read skill book beacuse you dont choise skill group."));
					return false;
				}
				// @edit014 END

				int lv = GetSkillLevel(SKILL_SUB_CAST_SPEED);

				if (lv < item->GetValue(0))
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ Ã¥Àº ³Ê¹« ¾î·Á¿ö ÀÌÇØÇÏ±â°¡ Èûµì´Ï´Ù."));
					return false;
				}

				if (lv >= item->GetValue(1))
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ Ã¥Àº ¾Æ¹«¸® ºÁµµ µµ¿òÀÌ µÉ °Í °°Áö ¾Ê½À´Ï´Ù."));
					return false;
				}

				if (LearnSkillByBook(SKILL_SUB_CAST_SPEED))
				{
					item->SetCount(item->GetCount() - 1);

					int iReadDelay = number(SKILLBOOK_DELAY_MIN, SKILLBOOK_DELAY_MAX);

					SetSkillNextReadTime(SKILL_SUB_CAST_SPEED, get_global_time() + iReadDelay);
				}
			}
			break;

			case 50334:
			case 50357:
			case 50367:
			case 50377:
			case 50387:
			{
				// @edit014 BEGIN
				if (!GetSkillGroup())
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You cant read skill book beacuse you dont choise skill group."));
					return false;
				}
				// @edit014 END

				int lv = GetSkillLevel(SKILL_REFINE);

				if (lv < item->GetValue(0))
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ Ã¥Àº ³Ê¹« ¾î·Á¿ö ÀÌÇØÇÏ±â°¡ Èûµì´Ï´Ù."));
					return false;
				}

				if (lv >= item->GetValue(1))
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ Ã¥Àº ¾Æ¹«¸® ºÁµµ µµ¿òÀÌ µÉ °Í °°Áö ¾Ê½À´Ï´Ù."));
					return false;
				}

				if (LearnSkillByBook(SKILL_REFINE))
				{
					item->SetCount(item->GetCount() - 1);

					int iReadDelay = number(SKILLBOOK_DELAY_MIN, SKILLBOOK_DELAY_MAX);

					SetSkillNextReadTime(SKILL_REFINE, get_global_time() + iReadDelay);
				}
			}
			break;
#else
			case 50301:
			case 50302:
			case 50303:
			{
				// @edit014 BEGIN
				if (!GetSkillGroup())
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You cant read skill book beacuse you dont choise skill group."));
					return false;
				}
				// @edit014 END

				int lv = GetSkillLevel(SKILL_LEADERSHIP);

				if (lv < item->GetValue(0))
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ Ã¥Àº ³Ê¹« ¾î·Á¿ö ÀÌÇØÇÏ±â°¡ Èûµì´Ï´Ù."));
					return false;
				}

				if (lv >= item->GetValue(1))
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ Ã¥Àº ¾Æ¹«¸® ºÁµµ µµ¿òÀÌ µÉ °Í °°Áö ¾Ê½À´Ï´Ù."));
					return false;
				}

				if (LearnSkillByBook(SKILL_LEADERSHIP))
				{
					item->SetCount(item->GetCount() - 1);

					int iReadDelay = number(SKILLBOOK_DELAY_MIN, SKILLBOOK_DELAY_MAX);

					SetSkillNextReadTime(SKILL_LEADERSHIP, get_global_time() + iReadDelay);
				}
			}
			break;
#endif

			case 50304:
			case 50305:
			case 50306:
			{
				// @edit014 BEGIN
				if (!GetSkillGroup())
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You cant read skill book beacuse you dont choise skill group."));
					return false;
				}
				// @edit014 END

				if (GetSkillLevel(SKILL_COMBO) == 0 && GetLevel() < 30)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("·¹º§ 30ÀÌ µÇ±â Àü¿¡´Â ½ÀµæÇÒ ¼ö ÀÖÀ» °Í °°Áö ¾Ê½À´Ï´Ù."));
					return false;
				}

				if (GetSkillLevel(SKILL_COMBO) == 1 && GetLevel() < 50)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("·¹º§ 50ÀÌ µÇ±â Àü¿¡´Â ½ÀµæÇÒ ¼ö ÀÖÀ» °Í °°Áö ¾Ê½À´Ï´Ù."));
					return false;
				}

				if (GetSkillLevel(SKILL_COMBO) >= 2)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¿¬°è±â´Â ´õÀÌ»ó ¼ö·ÃÇÒ ¼ö ¾ø½À´Ï´Ù."));
					return false;
				}

				int iPct = item->GetValue(0);

				if (LearnSkillByBook(SKILL_COMBO, iPct))
				{
					item->SetCount(item->GetCount() - 1);

					int iReadDelay = number(SKILLBOOK_DELAY_MIN, SKILLBOOK_DELAY_MAX);

					SetSkillNextReadTime(SKILL_COMBO, get_global_time() + iReadDelay);
				}
			}
			break;
			case 50311:
			case 50312:
			case 50313:
			{
				// @edit014 BEGIN
				if (!GetSkillGroup())
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You cant read skill book beacuse you dont choise skill group."));
					return false;
				}
				// @edit014 END

				DWORD dwSkillVnum = item->GetValue(0);
				int iPct = MINMAX(0, item->GetValue(1), 100);
				if (GetSkillLevel(dwSkillVnum) >= 20 || dwSkillVnum - SKILL_LANGUAGE1 + 1 == GetEmpire())
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ¹Ì ¿Ïº®ÇÏ°Ô ¾Ë¾ÆµéÀ» ¼ö ÀÖ´Â ¾ð¾îÀÌ´Ù."));
					return false;
				}

				if (LearnSkillByBook(dwSkillVnum, iPct))
				{
					item->SetCount(item->GetCount() - 1);

					int iReadDelay = number(SKILLBOOK_DELAY_MIN, SKILLBOOK_DELAY_MAX);

					SetSkillNextReadTime(dwSkillVnum, get_global_time() + iReadDelay);
				}
			}
			break;

			case 50061:
			{
				// @edit014 BEGIN
				if (!GetSkillGroup())
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You cant read skill book beacuse you dont choise skill group."));
					return false;
				}
				// @edit014 END

				DWORD dwSkillVnum = item->GetValue(0);
				int iPct = MINMAX(0, item->GetValue(1), 100);

				if (GetSkillLevel(dwSkillVnum) >= 10)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´õ ÀÌ»ó ¼ö·ÃÇÒ ¼ö ¾ø½À´Ï´Ù."));
					return false;
				}

				if (LearnSkillByBook(dwSkillVnum, iPct))
				{
					item->SetCount(item->GetCount() - 1);

					int iReadDelay = number(SKILLBOOK_DELAY_MIN, SKILLBOOK_DELAY_MAX);

					SetSkillNextReadTime(dwSkillVnum, get_global_time() + iReadDelay);
				}
			}
			break;

			case 50314:
			case 50315:
			case 50316:
			{
				// @edit014 BEGIN
				if (!GetSkillGroup())
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You cant read skill book beacuse you dont choise skill group."));
					return false;
				}
				// @edit014 END

				int iSkillLevelLowLimit = item->GetValue(0);
				int iSkillLevelHighLimit = item->GetValue(1);
				int iPct = MINMAX(0, item->GetValue(2), 100);
				int iLevelLimit = item->GetValue(3);
				DWORD dwSkillVnum = 0;

				switch (item->GetVnum())
				{
				case 50314:
				case 50315:
				case 50316:
					dwSkillVnum = SKILL_POLYMORPH;
					break;

				default:
					return false;
				}

				if (0 == dwSkillVnum)
					return false;

				if (GetLevel() < iLevelLimit)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ Ã¥À» ÀÐÀ¸·Á¸é ·¹º§À» ´õ ¿Ã·Á¾ß ÇÕ´Ï´Ù."));
					return false;
				}

				if (GetSkillLevel(dwSkillVnum) >= 40)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´õ ÀÌ»ó ¼ö·ÃÇÒ ¼ö ¾ø½À´Ï´Ù."));
					return false;
				}

				if (GetSkillLevel(dwSkillVnum) < iSkillLevelLowLimit)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ Ã¥Àº ³Ê¹« ¾î·Á¿ö ÀÌÇØÇÏ±â°¡ Èûµì´Ï´Ù."));
					return false;
				}

				if (GetSkillLevel(dwSkillVnum) >= iSkillLevelHighLimit)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ Ã¥À¸·Î´Â ´õ ÀÌ»ó ¼ö·ÃÇÒ ¼ö ¾ø½À´Ï´Ù."));
					return false;
				}

				if (LearnSkillByBook(dwSkillVnum, iPct))
				{
					item->SetCount(item->GetCount() - 1);

					int iReadDelay = number(SKILLBOOK_DELAY_MIN, SKILLBOOK_DELAY_MAX);

					SetSkillNextReadTime(dwSkillVnum, get_global_time() + iReadDelay);
				}
			}
			break;

			case 50902:
			case 50903:
			case 50904:
			{
				// @edit014 BEGIN
				if (!GetSkillGroup())
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You cant read skill book beacuse you dont choise skill group."));
					return false;
				}
				// @edit014 END

				DWORD dwSkillVnum = SKILL_CREATE;
				int iPct = MINMAX(0, item->GetValue(1), 100);

				if (GetSkillLevel(dwSkillVnum) >= 40)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´õ ÀÌ»ó ¼ö·ÃÇÒ ¼ö ¾ø½À´Ï´Ù."));
					return false;
				}

				if (LearnSkillByBook(dwSkillVnum, iPct))
				{
					item->SetCount(item->GetCount() - 1);

					int iReadDelay = number(SKILLBOOK_DELAY_MIN, SKILLBOOK_DELAY_MAX);

					SetSkillNextReadTime(dwSkillVnum, get_global_time() + iReadDelay);

					if (test_server)
					{
						ChatPacket(CHAT_TYPE_INFO, "[TEST_SERVER] Success to learn skill ");
					}
				}
				else
				{
					if (test_server)
					{
						ChatPacket(CHAT_TYPE_INFO, "[TEST_SERVER] Failed to learn skill ");
					}
				}
			}
			break;

			// MINING
			case ITEM_MINING_SKILL_TRAIN_BOOK:
			{
				// @edit014 BEGIN
				if (!GetSkillGroup())
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You cant read skill book beacuse you dont choise skill group."));
					return false;
				}
				// @edit014 END

				DWORD dwSkillVnum = SKILL_MINING;
				int iPct = MINMAX(0, item->GetValue(1), 100);

				if (GetSkillLevel(dwSkillVnum) >= 40)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´õ ÀÌ»ó ¼ö·ÃÇÒ ¼ö ¾ø½À´Ï´Ù."));
					return false;
				}

				if (LearnSkillByBook(dwSkillVnum, iPct))
				{
					item->SetCount(item->GetCount() - 1);

					int iReadDelay = number(SKILLBOOK_DELAY_MIN, SKILLBOOK_DELAY_MAX);

					SetSkillNextReadTime(dwSkillVnum, get_global_time() + iReadDelay);
				}
			}
			break;
			// END_OF_MINING

			case ITEM_HORSE_SKILL_TRAIN_BOOK:
			{
				// @edit014 BEGIN
				if (!GetSkillGroup())
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You cant read skill book beacuse you dont choise skill group."));
					return false;
				}
				// @edit014 END

				DWORD dwSkillVnum = SKILL_HORSE;
				int iPct = MINMAX(0, item->GetValue(1), 100);

				if (GetLevel() < 50)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¾ÆÁ÷ ½Â¸¶ ½ºÅ³À» ¼ö·ÃÇÒ ¼ö ÀÖ´Â ·¹º§ÀÌ ¾Æ´Õ´Ï´Ù."));
					return false;
				}

				if (!test_server && get_global_time() < GetSkillNextReadTime(dwSkillVnum))
				{
					if (FindAffect(AFFECT_SKILL_NO_BOOK_DELAY))
					{
						RemoveAffect(AFFECT_SKILL_NO_BOOK_DELAY);
					}
					else
					{
						SkillLearnWaitMoreTimeMessage(GetSkillNextReadTime(dwSkillVnum) - get_global_time());
						return false;
					}
				}

				if (GetPoint(POINT_HORSE_SKILL) >= 20 ||
					GetSkillLevel(SKILL_HORSE_WILDATTACK) + GetSkillLevel(SKILL_HORSE_CHARGE) + GetSkillLevel(SKILL_HORSE_ESCAPE) >= 60 ||
					GetSkillLevel(SKILL_HORSE_WILDATTACK_RANGE) + GetSkillLevel(SKILL_HORSE_CHARGE) + GetSkillLevel(SKILL_HORSE_ESCAPE) >= 60)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´õ ÀÌ»ó ½Â¸¶ ¼ö·Ã¼­¸¦ ÀÐÀ» ¼ö ¾ø½À´Ï´Ù."));
					return false;
				}

				if (number(1, 100) <= iPct)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("½Â¸¶ ¼ö·Ã¼­¸¦ ÀÐ¾î ½Â¸¶ ½ºÅ³ Æ÷ÀÎÆ®¸¦ ¾ò¾ú½À´Ï´Ù."));
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¾òÀº Æ÷ÀÎÆ®·Î´Â ½Â¸¶ ½ºÅ³ÀÇ ·¹º§À» ¿Ã¸± ¼ö ÀÖ½À´Ï´Ù."));
					PointChange(POINT_HORSE_SKILL, 1);

					int iReadDelay = number(SKILLBOOK_DELAY_MIN, SKILLBOOK_DELAY_MAX);

					if (!test_server)
						SetSkillNextReadTime(dwSkillVnum, get_global_time() + iReadDelay);
				}
				else
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("½Â¸¶ ¼ö·Ã¼­ ÀÌÇØ¿¡ ½ÇÆÐÇÏ¿´½À´Ï´Ù."));
				}

				item->SetCount(item->GetCount() - 1);
			}
			break;

			case 70102:
			case 70103:
			{
				if (GetAlignment() >= 0)
					return false;

				int delta = MIN(-GetAlignment(), item->GetValue(0));

				sys_log(0, "%s ALIGNMENT ITEM %d", GetName(), delta);

				UpdateAlignment(delta);
				item->SetCount(item->GetCount() - 1);

				if (delta / 10 > 0)
				{
					ChatPacket(CHAT_TYPE_TALKING, LC_TEXT("¸¶À½ÀÌ ¸¼¾ÆÁö´Â±º. °¡½¿À» Áþ´©¸£´ø ¹«¾ð°¡°¡ Á» °¡º­¿öÁø ´À³¦ÀÌ¾ß."));
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼±¾ÇÄ¡°¡ %d Áõ°¡ÇÏ¿´½À´Ï´Ù."), delta / 10);
				}
			}
			break;
			case 70071:
			case 70072:
			case 70073:
			case 70074:
			case 71107:
#ifdef __ALIGNMENT_ITEMS__
			case 99403:
			case 99404:
			case 99405:
			case 99406:
			case 99407:
			case 99408:
			case 99499:
#endif
			{
				int val = item->GetValue(0);
				int interval = item->GetValue(1);
				quest::PC* pPC = quest::CQuestManager::instance().GetPC(GetPlayerID());
				if (!pPC) // @duzenleme questflag alip atabiliyorsak devam edecek.
					return false;
				int last_use_time = pPC->GetFlag("mythical_peach.last_use_time");

				if (get_global_time() - last_use_time < interval * 60 * 60)
				{
					if (test_server == false)
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¾ÆÁ÷ »ç¿ëÇÒ ¼ö ¾ø½À´Ï´Ù."));
						return false;
					}
					else
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Å×½ºÆ® ¼­¹ö ½Ã°£Á¦ÇÑ Åë°ú"));
					}
				}

#ifdef __ALIGNMENT_REWORK__
				if (GetAlignment() == 200000000)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Son Dereceye Ulastin Daha Zorlama AMK"));
					return false;
				}

				if (item->GetVnum() == 99499 || item->GetVnum() == 99403)
				{
					if (GetAlignment() < 2000000)
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Derecenizi bu nesne ile gelistiremezsiniz!"));
						return false;
					}
				}
				else
				{
					if (GetAlignment() >= 2000000)
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Derecenizi bu nesne ile gelistiremezsiniz!"));
						return false;
					}
				}
#endif

#ifdef __ALIGNMENT_REWORK__
				if (m_iRealAlignment >= 0
					&& GetLevel() < ENABLE_RENEWAL_ALIGN_UPDATE_2)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Dereceni pozitif arttýrmak için en az %d level olmalýsýn."), ENABLE_RENEWAL_ALIGN_UPDATE_2);
					return false;
				}
#endif

				//int old_alignment = GetAlignment() / 10;

				UpdateAlignment(val * 10);

				item->SetCount(item->GetCount() - 1);
				pPC->SetFlag("mythical_peach.last_use_time", get_global_time());

				//ChatPacket(CHAT_TYPE_TALKING, LC_TEXT("¸¶À½ÀÌ ¸¼¾ÆÁö´Â±º. °¡½¿À» Áþ´©¸£´ø ¹«¾ð°¡°¡ Á» °¡º­¿öÁø ´À³¦ÀÌ¾ß."));
				//ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼±¾ÇÄ¡°¡ %d Áõ°¡ÇÏ¿´½À´Ï´Ù."), val);
			}
			break;

			case 71109:
			case 72719:
			{
				LPITEM item2;

				if (!IsValidItemPosition(DestCell) || !(item2 = GetItem(DestCell)))
					return false;

				if (item2->IsExchanging() || item2->IsEquipped()) // @duzenleme bu esya giyilemez ??? xD
					return false;

				if (item2->GetSocketCount() == 0)
					return false;

				switch (item2->GetType())
				{
				case ITEM_WEAPON:
					break;
				case ITEM_ARMOR:
					switch (item2->GetSubType())
					{
					case ARMOR_EAR:
					case ARMOR_WRIST:
					case ARMOR_NECK:
#ifdef __PENDANT_SYSTEM__
					case ARMOR_PENDANT:
#endif
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("»©³¾ ¿µ¼®ÀÌ ¾ø½À´Ï´Ù"));
						return false;
					}
					break;

				default:
					return false;
				}

				std::stack<long> socket;

				for (int i = 0; i < ITEM_SOCKET_MAX_NUM; ++i)
					socket.push(item2->GetSocket(i));

				int idx = ITEM_SOCKET_MAX_NUM - 1;

				while (socket.size() > 0)
				{
					if (socket.top() > 2 && socket.top() != ITEM_BROKEN_METIN_VNUM)
						break;

					idx--;
					socket.pop();
				}

				if (socket.size() == 0)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("»©³¾ ¿µ¼®ÀÌ ¾ø½À´Ï´Ù"));
					return false;
				}

				LPITEM pItemReward = AutoGiveItem(socket.top());

				if (pItemReward != NULL)
				{
					item2->SetSocket(idx, 1);
					item->SetCount(item->GetCount() - 1);
				}
			}
			break;

			case 70201:
			case 70202:
			case 70203:
			case 70204:
			case 70205:
			case 70206:
			{
				// NEW_HAIR_STYLE_ADD
				if (GetPart(PART_HAIR) >= 1001)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÇöÀç Çì¾î½ºÅ¸ÀÏ¿¡¼­´Â ¿°»ö°ú Å»»öÀÌ ºÒ°¡´ÉÇÕ´Ï´Ù."));
				}
				// END_NEW_HAIR_STYLE_ADD
				else
				{
					quest::CQuestManager& q = quest::CQuestManager::instance();
					quest::PC* pPC = q.GetPC(GetPlayerID());

					if (pPC)
					{
						int last_dye_level = pPC->GetFlag("dyeing_hair.last_dye_level");

						if (last_dye_level == 0 ||
							last_dye_level + 3 <= GetLevel() ||
							item->GetVnum() == 70201)
						{
							SetPart(PART_HAIR, item->GetVnum() - 70201);

							if (item->GetVnum() == 70201)
								pPC->SetFlag("dyeing_hair.last_dye_level", 0);
							else
								pPC->SetFlag("dyeing_hair.last_dye_level", GetLevel());

							item->SetCount(item->GetCount() - 1);
							UpdatePacket();
						}
						else
						{
							ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%d ·¹º§ÀÌ µÇ¾î¾ß ´Ù½Ã ¿°»öÇÏ½Ç ¼ö ÀÖ½À´Ï´Ù."), last_dye_level + 3);
						}
					}
				}
			}
			break;

			case ITEM_GIVE_STAT_RESET_COUNT_VNUM:
			{
				// @duzenleme statu uzerinden esya kopyalama gibi olaylar yapiliyordu bunun bir cok yerde olabilecegi kanaatindeyim.
				if (thecore_pulse() < LastStatResetUse + 60)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Please wait a second."));
					return false;
				}

				//PointChange(POINT_GOLD, -iCost);
				PointChange(POINT_STAT_RESET_COUNT, 1);
				item->SetCount(item->GetCount() - 1);

				LastStatResetUse = thecore_pulse();
			}
			break;

			case 50107:
			{
				EffectPacket(SE_CHINA_FIREWORK);
				item->SetCount(item->GetCount() - 1);
			}
			break;

#ifdef __COSTUME_EXTEND_TIME__
			case EXTEND_TIME_HAIR:
			{
				LPITEM item2;

				if (!IsValidItemPosition(DestCell) || !(item2 = GetItem(DestCell)))
					return false;

				if (item2->IsExchanging() == true)
					return false;

				if (item2->IsEquipped())
					return false;

#ifdef __BEGINNER_ITEM__
				if (item2->IsBasicItem())
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ITEM_IS_BASIC_CANNOT_DO"));
					return false;
				}
#endif

				if (!CanAct(true, true, VAR_TIME_NONE, nullptr, 0))
					return false;

				if (item2->GetSocket(2) < 1)
				{
					if (item2->GetType() == ITEM_COSTUME && item2->GetSubType() == COSTUME_HAIR)
					{
						long day = item->GetValue(0) * 60 * 60 * 24;
						item->SetCount(item->GetCount() - 1);
						item2->SetSocket(0, item2->GetSocket(0) + day);
						item2->SetSocket(2, (item2->GetSocket(2) + 1));
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("COSTUME_EXTEND_HAIR_SUCCESS"));
					}
					else
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("COSTUME_EXTEND_ITEM_ISNT_COSTUME_HAIR"));
						return false;
					}
				}
				else
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("COSTUME_EXTEND_EXTEND_LIMIT"));
					return false;
				}
			}
			break;

			case EXTEND_TIME_BODY:
			{
				LPITEM item2;

				if (!IsValidItemPosition(DestCell) || !(item2 = GetItem(DestCell)))
					return false;

				if (item2->IsExchanging() == true)
					return false;

				if (item2->IsEquipped())
					return false;

#ifdef __BEGINNER_ITEM__
				if (item2->IsBasicItem())
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ITEM_IS_BASIC_CANNOT_DO"));
					return false;
				}
#endif

				if (!CanAct(true, true, VAR_TIME_NONE, nullptr, 0))
					return false;

				if (item2->GetSocket(2) < 1)
				{
					if (item2->GetType() == ITEM_COSTUME && item2->GetSubType() == COSTUME_BODY)
					{
						long day = item->GetValue(0) * 60 * 60 * 24;
						item->SetCount(item->GetCount() - 1);
						item2->SetSocket(0, item2->GetSocket(0) + day);
						item2->SetSocket(2, (item2->GetSocket(2) + 1));
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("COSTUME_EXTEND_BODY_SUCCESS"));
					}
					else
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("COSTUME_EXTEND_ITEM_ISNT_COSTUME_BODY"));
						return false;
					}
				}
				else
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("COSTUME_EXTEND_EXTEND_LIMIT"));
					return false;
				}
			}
			break;

			case EXTEND_TIME_WEAPON:
			{
				LPITEM item2;

				if (!IsValidItemPosition(DestCell) || !(item2 = GetItem(DestCell)))
					return false;

				if (item2->IsExchanging() == true)
					return false;

				if (item2->IsEquipped())
					return false;

#ifdef __BEGINNER_ITEM__
				if (item2->IsBasicItem())
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ITEM_IS_BASIC_CANNOT_DO"));
					return false;
				}
#endif

				if (!CanAct(true, true, VAR_TIME_NONE, nullptr, 0))
					return false;

				if (item2->GetSocket(2) < 1)
				{
					if (item2->GetType() == ITEM_COSTUME && item2->GetSubType() == COSTUME_WEAPON)
					{
						long day = item->GetValue(0) * 60 * 60 * 24;
						item->SetCount(item->GetCount() - 1);
						item2->SetSocket(0, item2->GetSocket(0) + day);
						item2->SetSocket(2, (item2->GetSocket(2) + 1));
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("COSTUME_EXTEND_WEAPON_SUCCESS"));
					}
					else
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("COSTUME_EXTEND_ITEM_ISNT_COSTUME_WEAPON"));
						return false;
					}
				}
				else
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("COSTUME_EXTEND_EXTEND_LIMIT"));
					return false;
				}
			}
			break;
#endif

#if defined(__STRENGHT_FEED__) && defined(__MOUNT_COSTUME_SYSTEM__)
			case 38100:
			case 38101:
			case 38102:
			{
				LPITEM targetMount;

				if (!IsValidItemPosition(DestCell) || !(targetMount = GetItem(DestCell)))
					return false;

				if (targetMount->IsExchanging() == true)
					return false;

				if (!targetMount)
					return false;

#ifdef __BEGINNER_ITEM__
				if (targetMount->IsBasicItem())
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ITEM_IS_BASIC_CANNOT_DO"));
					return false;
				}
#endif

				if (!CanAct(true, true, VAR_TIME_NONE, nullptr, 0))
					return false;

				if (targetMount->IsEquipped())
					return false;

				if (targetMount->GetSocket(2) < 1)
				{
					if (targetMount->GetType() == ITEM_COSTUME && targetMount->GetSubType() == COSTUME_MOUNT)
					{
						long day = item->GetValue(0) * 60 * 60 * 24;
						item->SetCount(item->GetCount() - 1);
						targetMount->SetSocket(0, targetMount->GetSocket(0) + day);
						targetMount->SetSocket(2, (targetMount->GetSocket(2) + 1));
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Eþyanýn süresi baþarýyla uzatýldý."));
					}
					else
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Bu iþlemi bu eþyada gerçekleþtiremezsin."));
						return false;
					}
				}
				else
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("COSTUME_EXTEND_EXTEND_LIMIT"));
					return false;
				}
			}
			break;
#endif

			case 50108:
			{
				EffectPacket(SE_SPIN_TOP);
				item->SetCount(item->GetCount() - 1);
			}
			break;

			case ITEM_WONSO_BEAN_VNUM:
				PointChange(POINT_HP, GetMaxHP() - GetHP());
				item->SetCount(item->GetCount() - 1);
				break;

			case ITEM_WONSO_SUGAR_VNUM:
				PointChange(POINT_SP, GetMaxSP() - GetSP());
				item->SetCount(item->GetCount() - 1);
				break;

			case ITEM_WONSO_FRUIT_VNUM:
				PointChange(POINT_STAMINA, GetMaxStamina() - GetStamina());
				item->SetCount(item->GetCount() - 1);
				break;

			case ITEM_ELK_VNUM:
			{
#ifdef __GOLD_LIMIT_REWORK__
				long long llGold = item->GetSocket(0);
				ITEM_MANAGER::instance().RemoveItem(item);
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("µ· %lld ³ÉÀ» È¹µæÇß½À´Ï´Ù."), llGold);
				PointChange(POINT_GOLD, llGold);
#else
				int iGold = item->GetSocket(0);
				ITEM_MANAGER::instance().RemoveItem(item);
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("µ· %d ³ÉÀ» È¹µæÇß½À´Ï´Ù."), iGold);
				PointChange(POINT_GOLD, iGold);
#endif
			}
			break;

#ifdef __ANCIENT_ATTR_ITEM__
			case 71051:
			{
				LPITEM item2;

				if (!IsValidItemPosition(DestCell) || !(item2 = GetInventoryItem(wDestCell)))
					return false;

				if (ITEM_COSTUME == item2->GetType())
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼ºÀ» º¯°æÇÒ ¼ö ¾ø´Â ¾ÆÀÌÅÛÀÔ´Ï´Ù."));
					return false;
				}

				if (item2->IsExchanging() || item2->IsEquipped()) // @duzenleme bu esya giyilemez ??? xD
					return false;

				if (item2->GetAttributeSetIndex() == -1)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼ºÀ» º¯°æÇÒ ¼ö ¾ø´Â ¾ÆÀÌÅÛÀÔ´Ï´Ù."));
					return false;
				}

				if (item2->AddRareAttribute() == true)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼º°øÀûÀ¸·Î ¼Ó¼ºÀÌ Ãß°¡ µÇ¾ú½À´Ï´Ù"));
					item->SetCount(item->GetCount() - 1);
				}
				else
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´õ ÀÌ»ó ÀÌ ¾ÆÀÌÅÛÀ¸·Î ¼Ó¼ºÀ» Ãß°¡ÇÒ ¼ö ¾ø½À´Ï´Ù"));
				}
			}
			break;

			case 71052:
			{
				LPITEM item2;

				if (!IsValidItemPosition(DestCell) || !(item2 = GetItem(DestCell)))
					return false;

				if (ITEM_COSTUME == item2->GetType()) // @duzenleme bu esya giyilemez ??? xD
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼ºÀ» º¯°æÇÒ ¼ö ¾ø´Â ¾ÆÀÌÅÛÀÔ´Ï´Ù."));
					return false;
				}

				if (item2->IsExchanging() || item2->IsEquipped()) // @duzenleme bu esya giyilemez ??? xD
					return false;

				if (item2->GetAttributeSetIndex() == -1)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼ºÀ» º¯°æÇÒ ¼ö ¾ø´Â ¾ÆÀÌÅÛÀÔ´Ï´Ù."));
					return false;
				}

				if (item2->ChangeRareAttribute() == true)
				{
					item->SetCount(item->GetCount() - 1);
				}
				else
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("º¯°æ ½ÃÅ³ ¼Ó¼ºÀÌ ¾ø½À´Ï´Ù"));
				}
			}
			break;
#endif

#ifdef __ITEM_EXTRA_SOCKET__
			case ITEM_UPGRADE_SOCKET_WEAPON:
			{
				LPITEM item2;

				if (!IsValidItemPosition(DestCell) || !(item2 = GetItem(DestCell)))
					return false;

				if (item2->IsExchanging() || item2->IsEquipped())
					return false;

				if (item2->GetType() != ITEM_WEAPON)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("USE_OPEN_SOCKET_ONLY_WEAPON"));
					return false;
				}

#ifdef __BEGINNER_ITEM__
				if (item2->IsBasicItem())
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ITEM_IS_BASIC_CANNOT_DO"));
					return false;
				}
#endif
				if (item2->GetLevelLimit() < 55)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("USE_OPEN_SOCKET_WARNING_LV55"));
					return false;
				}

				if (item2->GetSocket(3) > 0)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("USE_OPEN_SOCKET_WARNING_SOCKET3"));
					return false;
				}

				item2->SetSocket(3, 1);
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("SUCCESS_OPEN_SOCKET"));
				item->SetCount(item->GetCount() - 1);
			}
			break;

			case ITEM_UPGRADE_SOCKET_ARMOR:
			{
				LPITEM item2;

				if (!IsValidItemPosition(DestCell) || !(item2 = GetItem(DestCell)))
					return false;

				if (item2->IsExchanging() || item2->IsEquipped())
					return false;

				if (item2->GetType() != ITEM_ARMOR)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("USE_OPEN_SOCKET_ONLY_ARMOR"));
					return false;
				}

#ifdef __BEGINNER_ITEM__
				if (item2->IsBasicItem())
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ITEM_IS_BASIC_CANNOT_DO"));
					return false;
				}
#endif

				if (item2->GetLevelLimit() < 55)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("USE_OPEN_SOCKET_WARNING_LV55"));
					return false;
				}

				if (item2->GetSocket(3) > 0)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("USE_OPEN_SOCKET_WARNING_SOCKET3"));
					return false;
				}

				item2->SetSocket(3, 1);
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("SUCCESS_OPEN_SOCKET"));
				item->SetCount(item->GetCount() - 1);
			}
			break;

			case ITEM_UPGRADE_SOCKET_MIX:
			{
				LPITEM item2;

				if (!IsValidItemPosition(DestCell) || !(item2 = GetItem(DestCell)))
					return false;

				if (item2->IsExchanging() || item2->IsEquipped())
					return false;

				if (item2->GetType() != ITEM_ARMOR && item2->GetType() != ITEM_WEAPON)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("USE_OPEN_SOCKET_ONLY_ARMOR_AND_WEAPON"));
					return false;
				}

#ifdef __BEGINNER_ITEM__
				if (item2->IsBasicItem())
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ITEM_IS_BASIC_CANNOT_DO"));
					return false;
				}
#endif
				if (item2->GetLevelLimit() < 55)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("USE_OPEN_SOCKET_WARNING_LV55"));
					return false;
				}

				if (item2->GetSocket(3) > 0)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("USE_OPEN_SOCKET_WARNING_SOCKET3"));
					return false;
				}

				item2->SetSocket(3, 1);
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("SUCCESS_OPEN_SOCKET"));
				item->SetCount(item->GetCount() - 1);
			}
			break;
#endif

#ifdef __BLEND_R_ITEMS__
			case NEW_MOVE_SPEED_POTION:
			case NEW_ATTACK_SPEED_POTION:
			case NEW_DRAGON_POTION_18385:
			case NEW_DRAGON_POTION_18386:
			case NEW_DRAGON_POTION_18387:
			case NEW_DRAGON_POTION_18388:
			case NEW_CRITIC_POTION_18389:
			case NEW_PENETRE_POTION_18390:
			{
				EAffectTypes type = AFFECT_NONE;

				if (item->GetVnum() == NEW_MOVE_SPEED_POTION)
					type = AFFECT_MOV_SPEED;
				else if (item->GetVnum() == NEW_ATTACK_SPEED_POTION)
					type = AFFECT_ATT_SPEED;

				// Dragon items.
				if (item->GetVnum() == NEW_DRAGON_POTION_18385)
					type = AFFECT_18385;
				else if (item->GetVnum() == NEW_DRAGON_POTION_18386)
					type = AFFECT_18386;
				else if (item->GetVnum() == NEW_DRAGON_POTION_18387)
					type = AFFECT_18387;
				else if (item->GetVnum() == NEW_DRAGON_POTION_18388)
					type = AFFECT_18388;

				// Critic/Penetre items.
				if (item->GetVnum() == NEW_CRITIC_POTION_18389)
					type = AFFECT_18389;
				else if (item->GetVnum() == NEW_PENETRE_POTION_18390)
					type = AFFECT_18390;

				if (AFFECT_NONE == type)
					break;

				CAffect* pAffect = FindAffect(type);

				if (NULL == pAffect)
				{
					EPointTypes bonus = POINT_NONE;
					EAffectBits flag = AFF_NONE;

					// Green and purple potion.
					if (item->GetVnum() == NEW_MOVE_SPEED_POTION)
					{
						bonus = POINT_MOV_SPEED;
						flag = AFF_MOV_SPEED_POTION;
					}
					else if (item->GetVnum() == NEW_ATTACK_SPEED_POTION)
					{
						bonus = POINT_ATT_SPEED;
						flag = AFF_ATT_SPEED_POTION;
					}

					// Dragon items.
					if (item->GetVnum() == NEW_DRAGON_POTION_18385)
					{
						bonus = POINT_MAX_HP_PCT;
						flag = AFF_18385;
					}
					else if (item->GetVnum() == NEW_DRAGON_POTION_18386)
					{
						bonus = POINT_MAX_SP_PCT;
						flag = AFF_18386;
					}
					else if (item->GetVnum() == NEW_DRAGON_POTION_18387)
					{
						bonus = POINT_MALL_DEFBONUS;
						flag = AFF_18387;
					}
					else if (item->GetVnum() == NEW_DRAGON_POTION_18388)
					{
						bonus = POINT_MALL_ATTBONUS;
						flag = AFF_18388;
					}

					if (item->GetVnum() == NEW_CRITIC_POTION_18389)
					{
						bonus = POINT_CRITICAL_PCT;
						flag = AFF_18389;
					}
					else if (item->GetVnum() == NEW_PENETRE_POTION_18390)
					{
						bonus = POINT_PENETRATE_PCT;
						flag = AFF_18390;
					}

					AddAffect(type, bonus, item->GetValue(2), flag, INFINITE_AFFECT_DURATION, 0, true);
					item->Lock(true);
					item->SetSocket(0, true);
				}
				else
				{
					RemoveAffect(pAffect);
					item->Lock(false);
					item->SetSocket(0, false);
				}
			}
			break;
#endif

#ifdef __ICECREAM_ITEM__
			case 50278: // Dondurma Kulah
			{
				if (FindAffect(AFFECT_ICECREAM_ABILITY))
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ¹Ì È¿°ú°¡ °É·Á ÀÖ½À´Ï´Ù."));
					return false;
				}
				long time = item->GetValue(0);
				long moveSpeedPer = item->GetValue(1);
				long attPer = item->GetValue(2);
				long expPer = item->GetValue(3);
				AddAffect(AFFECT_ICECREAM_ABILITY, POINT_MOV_SPEED, moveSpeedPer, AFF_MOV_SPEED_POTION, time, 0, true, true);
				AddAffect(AFFECT_ICECREAM_ABILITY, POINT_MALL_ATTBONUS, attPer, AFF_NONE, time, 0, true, true);
				AddAffect(AFFECT_ICECREAM_ABILITY, POINT_MALL_EXPBONUS, expPer, AFF_NONE, time, 0, true, true);

				item->SetCount(item->GetCount() - 1);
			}
			break;
#endif

			case ITEM_AUTO_HP_RECOVERY_S:
			case ITEM_AUTO_HP_RECOVERY_M:
			case ITEM_AUTO_HP_RECOVERY_L:
			case ITEM_AUTO_HP_RECOVERY_X:
			case ITEM_AUTO_SP_RECOVERY_S:
			case ITEM_AUTO_SP_RECOVERY_M:
			case ITEM_AUTO_SP_RECOVERY_L:
			case ITEM_AUTO_SP_RECOVERY_X:

			case REWARD_BOX_ITEM_AUTO_SP_RECOVERY_XS:
			case REWARD_BOX_ITEM_AUTO_SP_RECOVERY_S:
			case REWARD_BOX_ITEM_AUTO_HP_RECOVERY_XS:
			case REWARD_BOX_ITEM_AUTO_HP_RECOVERY_S:
			case FUCKING_BRAZIL_ITEM_AUTO_SP_RECOVERY_S:
			case FUCKING_BRAZIL_ITEM_AUTO_HP_RECOVERY_S:
			{
				EAffectTypes type = AFFECT_NONE;
				bool isSpecialPotion = false;

				switch (item->GetVnum())
				{
				case ITEM_AUTO_HP_RECOVERY_X:
					isSpecialPotion = true;

				case ITEM_AUTO_HP_RECOVERY_S:
				case ITEM_AUTO_HP_RECOVERY_M:
				case ITEM_AUTO_HP_RECOVERY_L:
				case REWARD_BOX_ITEM_AUTO_HP_RECOVERY_XS:
				case REWARD_BOX_ITEM_AUTO_HP_RECOVERY_S:
				case FUCKING_BRAZIL_ITEM_AUTO_HP_RECOVERY_S:
					type = AFFECT_AUTO_HP_RECOVERY;
					break;

				case ITEM_AUTO_SP_RECOVERY_X:
					isSpecialPotion = true;

				case ITEM_AUTO_SP_RECOVERY_S:
				case ITEM_AUTO_SP_RECOVERY_M:
				case ITEM_AUTO_SP_RECOVERY_L:
				case REWARD_BOX_ITEM_AUTO_SP_RECOVERY_XS:
				case REWARD_BOX_ITEM_AUTO_SP_RECOVERY_S:
				case FUCKING_BRAZIL_ITEM_AUTO_SP_RECOVERY_S:
					type = AFFECT_AUTO_SP_RECOVERY;
					break;
				}

				if (AFFECT_NONE == type)
					break;

				if (item->GetCount() > 1)
				{
					int pos = GetEmptyInventory(item->GetSize());

					if (-1 == pos)
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼ÒÁöÇ°¿¡ ºó °ø°£ÀÌ ¾ø½À´Ï´Ù."));
						break;
					}

					item->SetCount(item->GetCount() - 1);

					LPITEM item2 = ITEM_MANAGER::instance().CreateItem(item->GetVnum(), 1);
					item2->__ADD_TO_CHARACTER(this, TItemPos(INVENTORY, pos));

					if (item->GetSocket(1) != 0)
					{
						item2->SetSocket(1, item->GetSocket(1));
					}

					item = item2;
				}

				CAffect* pAffect = FindAffect(type);

				if (NULL == pAffect)
				{
					EPointTypes bonus = POINT_NONE;

					if (true == isSpecialPotion)
					{
						if (type == AFFECT_AUTO_HP_RECOVERY)
						{
							bonus = POINT_MAX_HP_PCT;
						}
						else if (type == AFFECT_AUTO_SP_RECOVERY)
						{
							bonus = POINT_MAX_SP_PCT;
						}
					}

					AddAffect(type, bonus, 4, item->GetID(), INFINITE_AFFECT_DURATION, 0, true, false);

					item->Lock(true);
					item->SetSocket(0, true);

					AutoRecoveryItemProcess(type);
				}
				else
				{
					if (item->GetID() == pAffect->dwFlag)
					{
						RemoveAffect(pAffect);

						item->Lock(false);
						item->SetSocket(0, false);
					}
					else
					{
						LPITEM old = FindItemByID(pAffect->dwFlag);

						if (NULL != old)
						{
							old->Lock(false);
							old->SetSocket(0, false);
						}

						RemoveAffect(pAffect);

						EPointTypes bonus = POINT_NONE;

						if (true == isSpecialPotion)
						{
							if (type == AFFECT_AUTO_HP_RECOVERY)
							{
								bonus = POINT_MAX_HP_PCT;
							}
							else if (type == AFFECT_AUTO_SP_RECOVERY)
							{
								bonus = POINT_MAX_SP_PCT;
							}
						}

						AddAffect(type, bonus, 4, item->GetID(), INFINITE_AFFECT_DURATION, 0, true, false);

						item->Lock(true);
						item->SetSocket(0, true);

						AutoRecoveryItemProcess(type);
					}
				}
			}
			break;
			}
			break;

		case USE_CLEAR:
		{
			switch (item->GetVnum())
			{
#ifdef __WOLFMAN_CHARACTER__
			case 27124: // Bandage
				RemoveBleeding();
				break;
#endif
			case 27874: // Grilled Perch
			default:
				RemoveBadAffect();
				break;
			}
			item->SetCount(item->GetCount() - 1);
		}
		break;

		case USE_INVISIBILITY:
		{
			if (item->GetVnum() == 70026)
			{
				quest::CQuestManager& q = quest::CQuestManager::instance();
				quest::PC* pPC = q.GetPC(GetPlayerID());

				if (pPC != NULL)
				{
					int last_use_time = pPC->GetFlag("mirror_of_disapper.last_use_time");

					if (get_global_time() - last_use_time < 10 * 60)
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¾ÆÁ÷ »ç¿ëÇÒ ¼ö ¾ø½À´Ï´Ù."));
						return false;
					}

					pPC->SetFlag("mirror_of_disapper.last_use_time", get_global_time());
				}
			}

			AddAffect(AFFECT_INVISIBILITY, POINT_NONE, 0, AFF_INVISIBILITY, 300, 0, true);
			item->SetCount(item->GetCount() - 1);
		}
		break;

		case USE_POTION_NODELAY:
		{
			bool used = false;

			if (item->GetValue(0) != 0)
			{
				if (GetHP() < GetMaxHP())
				{
					PointChange(POINT_HP, item->GetValue(0) * (100 + GetPoint(POINT_POTION_BONUS)) / 100);
					EffectPacket(SE_HPUP_RED);
					used = TRUE;
				}
			}

			if (item->GetValue(1) != 0)
			{
				if (GetSP() < GetMaxSP())
				{
					PointChange(POINT_SP, item->GetValue(1) * (100 + GetPoint(POINT_POTION_BONUS)) / 100);
					EffectPacket(SE_SPUP_BLUE);
					used = TRUE;
				}
			}

			if (item->GetValue(3) != 0)
			{
				if (GetHP() < GetMaxHP())
				{
					PointChange(POINT_HP, item->GetValue(3) * GetMaxHP() / 100);
					EffectPacket(SE_HPUP_RED);
					used = TRUE;
				}
			}

			if (item->GetValue(4) != 0)
			{
				if (GetSP() < GetMaxSP())
				{
					PointChange(POINT_SP, item->GetValue(4) * GetMaxSP() / 100);
					EffectPacket(SE_SPUP_BLUE);
					used = TRUE;
				}
			}

			if (used)
			{
				if (GetDungeon())
					GetDungeon()->UsePotion(this);

				if (GetWarMap())
					GetWarMap()->UsePotion(this, item);

				item->SetCount(item->GetCount() - 1);
			}
		}
		break;

		case USE_POTION:
			if (item->GetValue(1) != 0)
			{
				if (GetPoint(POINT_SP_RECOVERY) + GetSP() >= GetMaxSP())
				{
					return false;
				}

				PointChange(POINT_SP_RECOVERY, item->GetValue(1) * MIN(200, (100 + GetPoint(POINT_POTION_BONUS))) / 100);
				StartAffectEvent();
				EffectPacket(SE_SPUP_BLUE);
			}

			if (item->GetValue(0) != 0)
			{
				if (GetPoint(POINT_HP_RECOVERY) + GetHP() >= GetMaxHP())
				{
					return false;
				}

				PointChange(POINT_HP_RECOVERY, item->GetValue(0) * MIN(200, (100 + GetPoint(POINT_POTION_BONUS))) / 100);
				StartAffectEvent();
				EffectPacket(SE_HPUP_RED);
			}

			if (GetDungeon())
				GetDungeon()->UsePotion(this);

			if (GetWarMap())
				GetWarMap()->UsePotion(this, item);

			item->SetCount(item->GetCount() - 1);
			break;

		case USE_POTION_CONTINUE:
		{
			if (item->GetValue(0) != 0)
				AddAffect(AFFECT_HP_RECOVER_CONTINUE, POINT_HP_RECOVER_CONTINUE, item->GetValue(0), 0, item->GetValue(2), 0, true);
			else if (item->GetValue(1) != 0)
				AddAffect(AFFECT_SP_RECOVER_CONTINUE, POINT_SP_RECOVER_CONTINUE, item->GetValue(1), 0, item->GetValue(2), 0, true);
			else
				return false;
		}

		if (GetDungeon())
			GetDungeon()->UsePotion(this);

		if (GetWarMap())
			GetWarMap()->UsePotion(this, item);

		item->SetCount(item->GetCount() - 1);
		break;

		case USE_ABILITY_UP:
		{
			switch (item->GetValue(0))
			{
			case APPLY_MOV_SPEED:
				if (FindAffect(AFFECT_MOV_SPEED))
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("BU_ETKI_DEVAM_EDIYOR"));
					return false;
				}
				AddAffect(AFFECT_MOV_SPEED, POINT_MOV_SPEED, item->GetValue(2), AFF_MOV_SPEED_POTION, item->GetValue(1), 0, true);

#ifdef __EXTRAPOT_EFFECT__
				EffectPacket(SE_DXUP_PURPLE);
#endif
				break;

			case APPLY_ATT_SPEED:
				if (FindAffect(AFFECT_ATT_SPEED))
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("BU_ETKI_DEVAM_EDIYOR"));
					return false;
				}
				AddAffect(AFFECT_ATT_SPEED, POINT_ATT_SPEED, item->GetValue(2), AFF_ATT_SPEED_POTION, item->GetValue(1), 0, true);
#ifdef __EXTRAPOT_EFFECT__
				EffectPacket(SE_SPEEDUP_GREEN);
#endif
				break;

			case APPLY_STR:
				if (FindAffect(AFFECT_FISH_APPLY_STR))
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("BU_ETKI_DEVAM_EDIYOR"));
					return false;
				}
				AddAffect(AFFECT_FISH_APPLY_STR, POINT_ST, item->GetValue(2), 0, item->GetValue(1), 0, true);
				break;

			case APPLY_DEX:
				if (FindAffect(AFFECT_FISH_APPLY_DEX))
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("BU_ETKI_DEVAM_EDIYOR"));
					return false;
				}
				AddAffect(AFFECT_FISH_APPLY_DEX, POINT_DX, item->GetValue(2), 0, item->GetValue(1), 0, true);
				break;

			case APPLY_CON:
				if (FindAffect(AFFECT_FISH_APPLY_CON))
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("BU_ETKI_DEVAM_EDIYOR"));
					return false;
				}
				AddAffect(AFFECT_FISH_APPLY_CON, POINT_HT, item->GetValue(2), 0, item->GetValue(1), 0, true);
				break;

			case APPLY_INT:
				if (FindAffect(AFFECT_FISH_APPLY_INT))
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("BU_ETKI_DEVAM_EDIYOR"));
					return false;
				}
				AddAffect(AFFECT_FISH_APPLY_INT, POINT_IQ, item->GetValue(2), 0, item->GetValue(1), 0, true);
				break;

			case APPLY_CAST_SPEED:
				if (FindAffect(AFFECT_FISH_APPLY_CAST_SPEED))
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("BU_ETKI_DEVAM_EDIYOR"));
					return false;
				}
				AddAffect(AFFECT_FISH_APPLY_CAST_SPEED, POINT_CASTING_SPEED, item->GetValue(2), 0, item->GetValue(1), 0, true);
				break;

			case APPLY_ATT_GRADE_BONUS:
				if (FindAffect(AFFECT_FISH_APPLY_ATT_GRADE))
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("BU_ETKI_DEVAM_EDIYOR"));
					return false;
				}
				AddAffect(AFFECT_FISH_APPLY_ATT_GRADE, POINT_ATT_GRADE_BONUS,
					item->GetValue(2), 0, item->GetValue(1), 0, true);
				break;

			case APPLY_DEF_GRADE_BONUS:
				if (FindAffect(AFFECT_FISH_APPLY_DEF_GRADE))
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("BU_ETKI_DEVAM_EDIYOR"));
					return false;
				}
				AddAffect(AFFECT_FISH_APPLY_DEF_GRADE, POINT_DEF_GRADE_BONUS,
					item->GetValue(2), 0, item->GetValue(1), 0, true);
				break;

#ifdef __FISH_BONUS_REWORK__
			case APPLY_EXP_DOUBLE_BONUS:
				if (FindAffect(AFFECT_FISH_EXP_BONUS))
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("BU_ETKI_DEVAM_EDIYOR"));
					return false;
				}
				AddAffect(AFFECT_FISH_EXP_BONUS, POINT_EXP_DOUBLE_BONUS, item->GetValue(2), 0, item->GetValue(1), 0, true);
				break;

			case APPLY_MAX_HP:
				if (FindAffect(AFFECT_FISH_MAX_HP))
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("BU_ETKI_DEVAM_EDIYOR"));
					return false;
				}
				AddAffect(AFFECT_FISH_MAX_HP, POINT_MAX_HP, item->GetValue(2), 0, item->GetValue(1), 0, true);
				break;

			case APPLY_MAX_SP:
				if (FindAffect(AFFECT_FISH_MAX_SP))
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("BU_ETKI_DEVAM_EDIYOR"));
					return false;
				}
				AddAffect(AFFECT_FISH_MAX_SP, POINT_MAX_SP, item->GetValue(2), 0, item->GetValue(1), 0, true);
				break;

			case APPLY_ATTBONUS_UNDEAD:
				if (FindAffect(AFFECT_FISH_ATTBONUS_UNDEAD))
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("BU_ETKI_DEVAM_EDIYOR"));
					return false;
				}
				AddAffect(AFFECT_FISH_ATTBONUS_UNDEAD, POINT_ATTBONUS_UNDEAD, item->GetValue(2), 0, item->GetValue(1), 0, true);
				break;

			case APPLY_ATTBONUS_ANIMAL:
				if (FindAffect(AFFECT_FISH_ATTBONUS_ANIMAL))
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("BU_ETKI_DEVAM_EDIYOR"));
					return false;
				}
				AddAffect(AFFECT_FISH_ATTBONUS_ANIMAL, POINT_ATTBONUS_ANIMAL, item->GetValue(2), 0, item->GetValue(1), 0, true);
				break;

			case APPLY_ATTBONUS_MILGYO:
				if (FindAffect(AFFECT_FISH_ATTBONUS_MILGYO))
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("BU_ETKI_DEVAM_EDIYOR"));
					return false;
				}
				AddAffect(AFFECT_FISH_ATTBONUS_MILGYO, POINT_ATTBONUS_MILGYO, item->GetValue(2), 0, item->GetValue(1), 0, true);
				break;

			case APPLY_ATTBONUS_ORC:
				if (FindAffect(AFFECT_FISH_ATTBONUS_ORC))
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("BU_ETKI_DEVAM_EDIYOR"));
					return false;
				}
				AddAffect(AFFECT_FISH_ATTBONUS_ORC, POINT_ATTBONUS_ORC, item->GetValue(2), 0, item->GetValue(1), 0, true);
				break;

			case APPLY_ATTBONUS_DEVIL:
				if (FindAffect(AFFECT_FISH_ATTBONUS_DEVIL))
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("BU_ETKI_DEVAM_EDIYOR"));
					return false;
				}
				AddAffect(AFFECT_FISH_ATTBONUS_DEVIL, POINT_ATTBONUS_DEVIL, item->GetValue(2), 0, item->GetValue(1), 0, true);
				break;

			case APPLY_ATTBONUS_INSECT:
				if (FindAffect(AFFECT_FISH_ATTBONUS_INSECT))
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("BU_ETKI_DEVAM_EDIYOR"));
					return false;
				}
				AddAffect(AFFECT_FISH_ATTBONUS_INSECT, POINT_ATTBONUS_INSECT, item->GetValue(2), 0, item->GetValue(1), 0, true);
				break;

			case APPLY_ATTBONUS_BOSS:
				if (FindAffect(AFFECT_FISH_ATTBONUS_BOSS))
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("BU_ETKI_DEVAM_EDIYOR"));
					return false;
				}
				AddAffect(AFFECT_FISH_ATTBONUS_BOSS, POINT_ATTBONUS_BOSS, item->GetValue(2), 0, item->GetValue(1), 0, true);
				break;

			case APPLY_ATTBONUS_MONSTER:
				if (FindAffect(AFFECT_FISH_ATTBONUS_MONSTER))
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("BU_ETKI_DEVAM_EDIYOR"));
					return false;
				}
				AddAffect(AFFECT_FISH_ATTBONUS_MONSTER, POINT_ATTBONUS_MONSTER, item->GetValue(2), 0, item->GetValue(1), 0, true);
				break;

			case APPLY_ATTBONUS_HUMAN:
				if (FindAffect(AFFECT_FISH_ATTBONUS_HUMAN))
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("BU_ETKI_DEVAM_EDIYOR"));
					return false;
				}
				AddAffect(AFFECT_FISH_ATTBONUS_HUMAN, POINT_ATTBONUS_HUMAN, item->GetValue(2), 0, item->GetValue(1), 0, true);
				break;

			case APPLY_ATTBONUS_STONE:
				if (FindAffect(AFFECT_FISH_ATTBONUS_STONE))
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("BU_ETKI_DEVAM_EDIYOR"));
					return false;
				}
				AddAffect(AFFECT_FISH_ATTBONUS_STONE, POINT_ATTBONUS_STONE, item->GetValue(2), 0, item->GetValue(1), 0, true);
				break;

			case APPLY_NORMAL_HIT_DAMAGE_BONUS:
				if (FindAffect(AFFECT_FISH_NORMAL_HIT_BONUS))
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("BU_ETKI_DEVAM_EDIYOR"));
					return false;
				}
				AddAffect(AFFECT_FISH_NORMAL_HIT_BONUS, POINT_NORMAL_HIT_DAMAGE_BONUS, item->GetValue(2), 0, item->GetValue(1), 0, true);
				break;
#endif
			}
		}

		if (GetDungeon())
			GetDungeon()->UsePotion(this);

		if (GetWarMap())
			GetWarMap()->UsePotion(this, item);

		item->SetCount(item->GetCount() - 1);
		break;

		case USE_TALISMAN:
		{
			const int TOWN_PORTAL = 1;
			const int MEMORY_PORTAL = 2;

			if (GetMapIndex() == 200)
			{
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÇöÀç À§Ä¡¿¡¼­ »ç¿ëÇÒ ¼ö ¾ø½À´Ï´Ù."));
				return false;
			}

			if (m_pkWarpEvent)
			{
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌµ¿ÇÒ ÁØºñ°¡ µÇ¾îÀÖÀ½À¸·Î ±ÍÈ¯ºÎ¸¦ »ç¿ëÇÒ¼ö ¾ø½À´Ï´Ù"));
				return false;
			}

			// CONSUME_LIFE_WHEN_USE_WARP_ITEM
			int consumeLife = CalculateConsume(this);

			if (consumeLife < 0)
				return false;
			// END_OF_CONSUME_LIFE_WHEN_USE_WARP_ITEM

			if (item->GetValue(0) == TOWN_PORTAL)
			{
				if (item->GetSocket(0) == 0)
				{
					if (!GetDungeon())
						if (!GiveRecallItem(item))
							return false;

					PIXEL_POSITION posWarp;

					if (SECTREE_MANAGER::instance().GetRecallPositionByEmpire(GetMapIndex(), GetEmpire(), posWarp))
					{
						// CONSUME_LIFE_WHEN_USE_WARP_ITEM
						PointChange(POINT_HP, -consumeLife, false);
						// END_OF_CONSUME_LIFE_WHEN_USE_WARP_ITEM

						WarpSet(posWarp.x, posWarp.y);
					}
					else
					{
						sys_err("CHARACTER::UseItem : cannot find spawn position (name %s, %d x %d)", GetName(), GetX(), GetY());
					}
				}
				else
				{
					if (test_server)
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¿ø·¡ À§Ä¡·Î º¹±Í"));

					ProcessRecallItem(item);
				}
			}
			else if (item->GetValue(0) == MEMORY_PORTAL)
			{
				if (item->GetSocket(0) == 0)
				{
					if (GetDungeon())
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´øÀü ¾È¿¡¼­´Â %s%s »ç¿ëÇÒ ¼ö ¾ø½À´Ï´Ù."),
							item->GetName(),
							"");
						return false;
					}

					if (!GiveRecallItem(item))
						return false;
				}
				else
				{
					// CONSUME_LIFE_WHEN_USE_WARP_ITEM
					PointChange(POINT_HP, -consumeLife, false);
					// END_OF_CONSUME_LIFE_WHEN_USE_WARP_ITEM

					ProcessRecallItem(item);
				}
			}
		}
		break;

		case USE_TUNING:
		case USE_DETACHMENT:
		{
			LPITEM item2;

			if (!IsValidItemPosition(DestCell) || !(item2 = GetItem(DestCell)))
				return false;

			if (item2->IsExchanging() || item2->IsEquipped()) // @duzenleme eger esya giyili ise detachment falan atilmiyor fakat bunlarin dedigim gibi inputlardan engellenmesi sart.
				return false;

			if (item2->GetVnum() >= 28330 && item2->GetVnum() <= 28343)
			{
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("+3 ¿µ¼®Àº ÀÌ ¾ÆÀÌÅÛÀ¸·Î °³·®ÇÒ ¼ö ¾ø½À´Ï´Ù"));
				return false;
			}

#ifdef __BEGINNER_ITEM__
			if (item2->IsBasicItem())
			{
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ITEM_IS_BASIC_CANNOT_DO"));
				return false;
			}
#endif
#ifdef __ACCE_SYSTEM__
			if (item->GetValue(0) == ACCE_CLEAN_ATTR_VALUE0)
			{
				if (!CleanAcceAttr(item, item2))
					return false;

				return true;
			}
#endif
#ifdef __REFINE_SCROLL__
			if (item->GetValue(0) == PLUS_SCROLL)
			{
				RefineItem(item, item2);

				return true;
			}
#endif
#ifdef __ITEM_CHANGELOOK__
			if (item->GetValue(0) == CL_CLEAN_ATTR_VALUE0)
			{
				if (!CleanTransmutation(item, item2))
					return false;

				return true;
			}
#endif
#ifdef __ITEM_EVOLUTION__
			if (item->GetValue(0) == EV_CLEAN_VALUE0)
			{
				if (!CleanEvolution(item, item2))
					return false;

				return true;
			}
#endif
			RefineItem(item, item2);
		}
		break;

#ifdef __COSTUME_ATTR_SWITCH__
		case USE_CHANGE_COSTUME_ATTR:
		case USE_RESET_COSTUME_ATTR:
		{
			LPITEM item2;
			if (!IsValidItemPosition(DestCell) || !(item2 = GetItem(DestCell)))
				return false;

			if (item2->IsEquipped())
			{
				BuffOnAttr_RemoveBuffsFromItem(item2);
			}

#ifdef __AURA_COSTUME_SYSTEM__
			if (ITEM_COSTUME == item2->GetType() && COSTUME_AURA == item2->GetSubType())
			{
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼ºÀ» º¯°æÇÒ ¼ö ¾ø´Â ¾ÆÀÌÅÛÀÔ´Ï´Ù."));
				return false;
			}
#endif

			if (ITEM_COSTUME != item2->GetType())
			{
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼ºÀ» º¯°æÇÒ ¼ö ¾ø´Â ¾ÆÀÌÅÛÀÔ´Ï´Ù."));
				return false;
			}

			if (COSTUME_BODY != item2->GetSubType() && COSTUME_HAIR != item2->GetSubType())
			{
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼ºÀ» º¯°æÇÒ ¼ö ¾ø´Â ¾ÆÀÌÅÛÀÔ´Ï´Ù."));
				return false;
			}

#ifdef __BEGINNER_ITEM__
			if (item2->IsBasicItem())
			{
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ITEM_IS_BASIC_CANNOT_DO"));
				return false;
			}
#endif

			if (item2->IsExchanging() || item2->IsEquipped()) // @duzenleme eger esya giyili ise detachment falan atilmiyor fakat bunlarin dedigim gibi inputlardan engellenmesi sart.
				return false;

			if (item2->GetAttributeSetIndex() == -1)
			{
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼ºÀ» º¯°æÇÒ ¼ö ¾ø´Â ¾ÆÀÌÅÛÀÔ´Ï´Ù."));
				return false;
			}

			switch (item->GetSubType())
			{
			case USE_CHANGE_COSTUME_ATTR:
			{
				item2->ChangeAttribute(aiCostumeAttributeLevelPercent);

				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼ºÀ» º¯°æÇÏ¿´½À´Ï´Ù."));
				item->SetCount(item->GetCount() - 1);
			}
			break;

			case USE_RESET_COSTUME_ATTR:
			{
				item2->ClearAttribute();

				BYTE i;
				for (i = 0; i < COSTUME_ATTRIBUTE_MAX_NUM; i++)
				{
					//char result[64];
					if (number(1, 100) <= aiCostumeAttributeAddPercent[item2->GetAttributeCount()])
					{
						item2->AddAttribute();
					}
					else
					{
						break;
					}
				}

				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼ºÀ» º¯°æÇÏ¿´½À´Ï´Ù."));
				item->SetCount(item->GetCount() - 1);
			}
			}
		}
		break;
#endif

		//  ACCESSORY_REFINE & ADD/CHANGE_ATTRIBUTES
		case USE_PUT_INTO_BELT_SOCKET:
		case USE_PUT_INTO_RING_SOCKET:
		case USE_PUT_INTO_ACCESSORY_SOCKET:
		case USE_ADD_ACCESSORY_SOCKET:
		case USE_CLEAN_SOCKET:
		case USE_CHANGE_ATTRIBUTE:
		case USE_CHANGE_ATTRIBUTE2:
#ifdef __PENDANT_ATTR__
		case USE_CHANGE_PENDANT_ATTRIBUTE:
#endif
#ifdef __AURA_COSTUME_SYSTEM__
		case USE_CHANGE_AURA_ATTRIBUTE:
#endif
		case USE_ADD_ATTRIBUTE:
		case USE_ADD_ATTRIBUTE2:
#ifdef __PENDANT_ATTR__
		case USE_ADD_PENDANT_ATTRIBUTE:
		case USE_ADD_PENDANT_FIVE_ATTRIBUTE:
#endif
#ifdef __AURA_COSTUME_SYSTEM__
		case USE_ADD_AURA_ATTRIBUTE:
		case USE_ADD_AURA_FIVE_ATTRIBUTE:
#endif
#ifdef __MOUNT_COSTUME_SYSTEM__
		case USE_ADD_MOUNT_ATTRIBUTE:
		case USE_CHANGE_MOUNT_ATTRIBUTE:
#endif
#ifdef __WEAPON_COSTUME_SYSTEM__
		case USE_ADD_WEAPON_COSTUME_ATTRIBUTE:
		case USE_CHANGE_WEAPON_COSTUME_ATTRIBUTE:
#endif
#ifdef __MARTY_BELT_INVENTORY__
		case USE_ADD_BELT_ATTRIBUTE:
		case USE_CHANGE_BELT_ATTRIBUTE:
#endif
#ifdef __HAIR_COSTUME_ATTRIBUTE__
		case USE_ADD_HAIR_COSTUME_ATTRIBUTE:
		case USE_CHANGE_HAIR_COSTUME_ATTRIBUTE:
#endif
#ifdef __BODY_COSTUME_ATTRIBUTE__
		case USE_ADD_BODY_COSTUME_ATTRIBUTE:
		case USE_CHANGE_BODY_COSTUME_ATTRIBUTE:
#endif
#ifdef __SHINING_ITEM_SYSTEM__
		case USE_ADD_ACCE_SHINING_ATTRIBUTE:
		case USE_CHANGE_ACCE_SHINING_ATTRIBUTE:
		case USE_ADD_SOUL_SHINING_ATTRIBUTE:
		case USE_ADD_SOUL_SHINING_FIVE_ATTRIBUTE:
		case USE_CHANGE_SOUL_SHINING_ATTRIBUTE:
#endif
#ifdef __MOUNT_COSTUME_SYSTEM__
		case USE_ADD_FIVE_MOUNT_ATTRIBUTE:
#endif
#ifdef __WEAPON_COSTUME_SYSTEM__
		case USE_ADD_FIVE_WEAPON_COSTUME_ATTRIBUTE:
#endif
#ifdef __MARTY_BELT_INVENTORY__
		case USE_ADD_FIVE_BELT_ATTRIBUTE:
#endif
#ifdef __HAIR_COSTUME_ATTRIBUTE__
		case USE_ADD_FIVE_HAIR_COSTUME_ATTRIBUTE:
#endif
#ifdef __BODY_COSTUME_ATTRIBUTE__
		case USE_ADD_FIVE_BODY_COSTUME_ATTRIBUTE:
#endif
#ifdef __SHINING_ITEM_SYSTEM__
		case USE_ADD_FIVE_ACCE_SHINING_ATTRIBUTE:
#endif
#ifdef __CAKRA_ITEM_SYSTEM__
		case USE_ADD_CAKRA_ITEM_ATTRIBUTE:
		case USE_ADD_FIVE_CAKRA_ITEM_ATTRIBUTE:
		case USE_CHANGE_CAKRA_ITEM_ATTRIBUTE:
#endif
#ifdef __PET_SYSTEM_PROTO__
		case USE_ADD_PET_ITEM_ATTRIBUTE:
		case USE_ADD_FIVE_PET_ITEM_ATTRIBUTE:
		case USE_CHANGE_PET_ITEM_ATTRIBUTE:
#endif
#ifdef __SEBNEM_ITEM_SYSTEM__
		case USE_ADD_SEBNEM_ITEM_ATTRIBUTE:
		case USE_ADD_FIVE_SEBNEM_ITEM_ATTRIBUTE:
		case USE_CHANGE_SEBNEM_ITEM_ATTRIBUTE:
#endif
		{
			LPITEM item2;
			if (!IsValidItemPosition(DestCell) || !(item2 = GetItem(DestCell)))
				return false;

			if (item2->IsEquipped())
			{
				BuffOnAttr_RemoveBuffsFromItem(item2);
			}

#ifdef __NEW_ATTRIBUTE_ITEMS
			if (
#ifdef __AURA_COSTUME_SYSTEM__
				item->GetSubType() != USE_CHANGE_AURA_ATTRIBUTE && item->GetSubType() != USE_ADD_AURA_ATTRIBUTE && item->GetSubType() != USE_ADD_AURA_FIVE_ATTRIBUTE
#endif
#ifdef __MOUNT_COSTUME_SYSTEM__
				 && item->GetSubType() != USE_ADD_MOUNT_ATTRIBUTE && item->GetSubType() != USE_CHANGE_MOUNT_ATTRIBUTE && item->GetSubType() != USE_ADD_FIVE_MOUNT_ATTRIBUTE
#endif
#ifdef __WEAPON_COSTUME_SYSTEM__
				 && item->GetSubType() != USE_ADD_WEAPON_COSTUME_ATTRIBUTE && item->GetSubType() != USE_CHANGE_WEAPON_COSTUME_ATTRIBUTE && item->GetSubType() != USE_ADD_FIVE_WEAPON_COSTUME_ATTRIBUTE
#endif
#ifdef __HAIR_COSTUME_ATTRIBUTE__
				 && item->GetSubType() != USE_ADD_HAIR_COSTUME_ATTRIBUTE && item->GetSubType() != USE_CHANGE_HAIR_COSTUME_ATTRIBUTE && item->GetSubType() != USE_ADD_FIVE_HAIR_COSTUME_ATTRIBUTE
#endif
#ifdef __BODY_COSTUME_ATTRIBUTE__
				 && item->GetSubType() != USE_ADD_BODY_COSTUME_ATTRIBUTE && item->GetSubType() != USE_CHANGE_BODY_COSTUME_ATTRIBUTE && item->GetSubType() != USE_ADD_FIVE_BODY_COSTUME_ATTRIBUTE
#endif
#ifdef __NEWATTRPHEBIA__
				&& item->GetVnum() != PUT_MONSTER_ATTR_ADD && item->GetVnum() != PUT_MONSTER_ATTR_CHANGE && item->GetVnum() != PUT_STONE_ATTR_ADD && item->GetVnum() != PUT_STONE_ATTR_CHANGE
#endif
			)
			{
				if (ITEM_COSTUME == item2->GetType())
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼ºÀ» º¯°æÇÒ ¼ö ¾ø´Â ¾ÆÀÌÅÛÀÔ´Ï´Ù."));
					return false;
				}
			}
#else
			if (ITEM_COSTUME == item2->GetType())
			{
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼ºÀ» º¯°æÇÒ ¼ö ¾ø´Â ¾ÆÀÌÅÛÀÔ´Ï´Ù."));
				return false;
			}
#endif

#ifdef __MARTY_BELT_INVENTORY__
			if (item->GetSubType() != USE_ADD_BELT_ATTRIBUTE && item->GetSubType() != USE_CHANGE_BELT_ATTRIBUTE && item->GetSubType() != USE_ADD_FIVE_BELT_ATTRIBUTE && item->GetSubType() != USE_ADD_ACCESSORY_SOCKET && item->GetSubType() != USE_PUT_INTO_BELT_SOCKET
#ifdef __NEWATTRPHEBIA__
				&& item->GetVnum() != PUT_MONSTER_ATTR_ADD && item->GetVnum() != PUT_MONSTER_ATTR_CHANGE && item->GetVnum() != PUT_STONE_ATTR_ADD && item->GetVnum() != PUT_STONE_ATTR_CHANGE
#endif
			)
			{
				if (ITEM_BELT == item2->GetType())
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼ºÀ» º¯°æÇÒ ¼ö ¾ø´Â ¾ÆÀÌÅÛÀÔ´Ï´Ù."));
					return false;
				}
			}
#else
			if (item->GetSubType() != USE_ADD_ACCESSORY_SOCKET && item->GetSubType() != USE_PUT_INTO_BELT_SOCKET)
			{
				if (ITEM_BELT == item2->GetType())
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼ºÀ» º¯°æÇÒ ¼ö ¾ø´Â ¾ÆÀÌÅÛÀÔ´Ï´Ù."));
					return false;
				}
			}
#endif

#ifdef __SHINING_ITEM_SYSTEM__
			if (item->GetSubType() != USE_ADD_ACCE_SHINING_ATTRIBUTE && item->GetSubType() != USE_CHANGE_ACCE_SHINING_ATTRIBUTE && item->GetSubType() != USE_ADD_SOUL_SHINING_ATTRIBUTE && item->GetSubType() != USE_ADD_SOUL_SHINING_FIVE_ATTRIBUTE && item->GetSubType() != USE_CHANGE_SOUL_SHINING_ATTRIBUTE && item->GetSubType() != USE_ADD_FIVE_ACCE_SHINING_ATTRIBUTE
#ifdef __NEWATTRPHEBIA__
				&& item->GetVnum() != PUT_MONSTER_ATTR_ADD && item->GetVnum() != PUT_MONSTER_ATTR_CHANGE && item->GetVnum() != PUT_STONE_ATTR_ADD && item->GetVnum() != PUT_STONE_ATTR_CHANGE
#endif
			)
			{
				if (ITEM_SHINING == item2->GetType())
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼ºÀ» º¯°æÇÒ ¼ö ¾ø´Â ¾ÆÀÌÅÛÀÔ´Ï´Ù."));
					return false;
				}
			}
#endif

#ifdef __CAKRA_ITEM_SYSTEM__
			if (item->GetSubType() != USE_ADD_CAKRA_ITEM_ATTRIBUTE && item->GetSubType() != USE_ADD_FIVE_CAKRA_ITEM_ATTRIBUTE && item->GetSubType() != USE_CHANGE_CAKRA_ITEM_ATTRIBUTE
#ifdef __NEWATTRPHEBIA__
				&& item->GetVnum() != PUT_MONSTER_ATTR_ADD && item->GetVnum() != PUT_MONSTER_ATTR_CHANGE && item->GetVnum() != PUT_STONE_ATTR_ADD && item->GetVnum() != PUT_STONE_ATTR_CHANGE
#endif
			)
			{
				if (ITEM_CAKRA == item2->GetType())
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼ºÀ» º¯°æÇÒ ¼ö ¾ø´Â ¾ÆÀÌÅÛÀÔ´Ï´Ù."));
					return false;
				}
			}
#endif

#ifdef __PET_SYSTEM_PROTO__
			if (item->GetSubType() != USE_ADD_PET_ITEM_ATTRIBUTE && item->GetSubType() != USE_ADD_FIVE_PET_ITEM_ATTRIBUTE && item->GetSubType() != USE_CHANGE_PET_ITEM_ATTRIBUTE
#ifdef __NEWATTRPHEBIA__
				&& item->GetVnum() != PUT_MONSTER_ATTR_ADD && item->GetVnum() != PUT_MONSTER_ATTR_CHANGE && item->GetVnum() != PUT_STONE_ATTR_ADD && item->GetVnum() != PUT_STONE_ATTR_CHANGE
#endif
			)
			{
				if (item2->GetType() == ITEM_UNIQUE && item2->GetSubType() == USE_PET)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼ºÀ» º¯°æÇÒ ¼ö ¾ø´Â ¾ÆÀÌÅÛÀÔ´Ï´Ù."));
					return false;
				}
			}
#endif

#ifdef __SEBNEM_ITEM_SYSTEM__
			if (item->GetSubType() != USE_ADD_SEBNEM_ITEM_ATTRIBUTE && item->GetSubType() != USE_ADD_FIVE_SEBNEM_ITEM_ATTRIBUTE && item->GetSubType() != USE_CHANGE_SEBNEM_ITEM_ATTRIBUTE
#ifdef __NEWATTRPHEBIA__
				&& item->GetVnum() != PUT_MONSTER_ATTR_ADD && item->GetVnum() != PUT_MONSTER_ATTR_CHANGE && item->GetVnum() != PUT_STONE_ATTR_ADD && item->GetVnum() != PUT_STONE_ATTR_CHANGE
#endif
			)
			{
				if (ITEM_SEBNEM == item2->GetType())
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼ºÀ» º¯°æÇÒ ¼ö ¾ø´Â ¾ÆÀÌÅÛÀÔ´Ï´Ù."));
					return false;
				}
			}
#endif

			if (item2->IsExchanging() || item2->IsEquipped()) // @duzenleme eger esya giyili ise detachment falan atilmiyor fakat bunlarin dedigim gibi inputlardan engellenmesi sart.
				return false;

#ifdef __BEGINNER_ITEM__
			if (item2->IsBasicItem())
			{
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ITEM_IS_BASIC_CANNOT_DO"));
				return false;
			}
#endif

			switch (item->GetSubType())
			{
			case USE_CLEAN_SOCKET:
			{
				int i;
				for (i = 0; i < ITEM_SOCKET_MAX_NUM; ++i)
				{
					if (item2->GetSocket(i) == ITEM_BROKEN_METIN_VNUM)
						break;
				}

#ifdef __BEGINNER_ITEM__
				if (item2->IsBasicItem())
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ITEM_IS_BASIC_CANNOT_DO"));
					return false;
				}
#endif
				if (i == ITEM_SOCKET_MAX_NUM)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Ã»¼ÒÇÒ ¼®ÀÌ ¹ÚÇôÀÖÁö ¾Ê½À´Ï´Ù."));
					return false;
				}

				int j = 0;

				for (i = 0; i < ITEM_SOCKET_MAX_NUM; ++i)
				{
					if (item2->GetSocket(i) != ITEM_BROKEN_METIN_VNUM && item2->GetSocket(i) != 0)
						item2->SetSocket(j++, item2->GetSocket(i));
				}

				for (; j < ITEM_SOCKET_MAX_NUM; ++j)
				{
					if (item2->GetSocket(j) > 0)
						item2->SetSocket(j, 1);
				}

				item->SetCount(item->GetCount() - 1);
			}
			break;

			case USE_CHANGE_ATTRIBUTE:
			case USE_CHANGE_ATTRIBUTE2:
				if (item2->GetAttributeSetIndex() == -1)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼ºÀ» º¯°æÇÒ ¼ö ¾ø´Â ¾ÆÀÌÅÛÀÔ´Ï´Ù."));
					return false;
				}

#if defined(__PENDANT_SYSTEM__) && defined(__PENDANT_ATTR__)
				if ((item->IsTilsimable()) && (item2->GetType() == ITEM_ARMOR && item2->GetSubType() == ARMOR_PENDANT))
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CANT_USE_THIS_USE_PEDANT_SWITCHER"));
					return false;
				}
#endif
				if (item2->GetAttributeCount() == 0)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("º¯°æÇÒ ¼Ó¼ºÀÌ ¾ø½À´Ï´Ù."));
					return false;
				}

				if (item2->IsEquipped()) // @duzenleme eger esya giyili ise detachment falan atilmiyor fakat bunlarin dedigim gibi inputlardan engellenmesi sart.
					return false;

#ifdef __BEGINNER_ITEM__
				if (item2->IsBasicItem())
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ITEM_IS_BASIC_CANNOT_DO"));
					return false;
				}

				if (item2->IsRealTimeItem() && item2->GetType() == ITEM_WEAPON)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ITEM_IS_BEGINNER_CANNOT_DO"));
					return false;
				}
#endif

				if (item->GetSubType() == USE_CHANGE_ATTRIBUTE2)
				{
					int aiChangeProb[ITEM_ATTRIBUTE_MAX_LEVEL] =
					{
						0, 0, 30, 40, 3
					};

					item2->ChangeAttribute(aiChangeProb);
				}
				else if (item->GetVnum() == 76014)
				{
					int aiChangeProb[ITEM_ATTRIBUTE_MAX_LEVEL] =
					{
						0, 10, 50, 39, 1
					};

					item2->ChangeAttribute(aiChangeProb);
				}
#ifdef __ATTR_ADDON_ITEMS__
				else if (item->GetVnum() == SWITCH_OBJECT_NORMAL)
				{
					if (item2->GetType() != ITEM_WEAPON)
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Sadece silahlara efsun ekleyebilirsin."));
						return false;
					}

					if (item2->GetAttributeType(0) != 72 && item2->GetAttributeType(1) != 72)
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Sadece ortalama zararý efsunu olan silahlara efsun ekleyebilirsin."));
						return false;
					}

					int iNormalHitBonusLuck = number(1, 40);
					int iNormalHitBonus = 0;
					if (iNormalHitBonusLuck == 40)
						iNormalHitBonus = 150;
					else if (iNormalHitBonusLuck == 39)
						iNormalHitBonus = 148;
					else if (iNormalHitBonusLuck == 38)
						iNormalHitBonus = 146;
					else if (iNormalHitBonusLuck == 37)
						iNormalHitBonus = 144;
					else if (iNormalHitBonusLuck == 36)
						iNormalHitBonus = 143;
					else if (iNormalHitBonusLuck == 35)
						iNormalHitBonus = 142;
					else
						iNormalHitBonus = number(100, 141);

					int iSkillBonus = number(-30, -20);

					item2->ChangeAttribute();
					item2->SetForceAttribute(0, 72, iNormalHitBonus);
					item2->SetForceAttribute(1, 71, iSkillBonus);
				}
				else if (item->GetVnum() == SWITCH_OBJECT_NORMAL2)
				{
					if (item2->GetType() != ITEM_WEAPON)
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Sadece silahlara efsun ekleyebilirsin."));
						return false;
					}

					if (item2->GetAttributeType(0) != 72 && item2->GetAttributeType(1) != 72)
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Sadece ortalama zararý efsunu olan silahlara efsun ekleyebilirsin."));
						return false;
					}

					int iNormalHitBonusLuck = number(1, 40);
					int iNormalHitBonus = 0;
					if (iNormalHitBonusLuck == 40)
						iNormalHitBonus = 200;
					else if (iNormalHitBonusLuck == 39)
						iNormalHitBonus = 198;
					else if (iNormalHitBonusLuck == 38)
						iNormalHitBonus = 197;
					else if (iNormalHitBonusLuck == 37)
						iNormalHitBonus = 195;
					else if (iNormalHitBonusLuck == 36)
						iNormalHitBonus = 194;
					else if (iNormalHitBonusLuck == 35)
						iNormalHitBonus = 193;
					else
						iNormalHitBonus = number(150, 192);

					int iSkillBonus = number(-30, -20);

					item2->ChangeAttribute();
					item2->SetForceAttribute(0, 72, iNormalHitBonus);
					item2->SetForceAttribute(1, 71, iSkillBonus);
				}
				else if (item->GetVnum() == SWITCH_OBJECT_NORMAL3)
				{
					if (item2->GetType() != ITEM_WEAPON)
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Sadece silahlara efsun ekleyebilirsin."));
						return false;
					}

					if (item2->GetAttributeType(0) != 72 && item2->GetAttributeType(1) != 72)
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Sadece ortalama zararý efsunu olan silahlara efsun ekleyebilirsin."));
						return false;
					}

					int iNormalHitBonusLuck = number(1, 40);
					int iNormalHitBonus = 0;
					if (iNormalHitBonusLuck == 40)
						iNormalHitBonus = 250;
					else if (iNormalHitBonusLuck == 39)
						iNormalHitBonus = 248;
					else if (iNormalHitBonusLuck == 38)
						iNormalHitBonus = 246;
					else if (iNormalHitBonusLuck == 37)
						iNormalHitBonus = 244;
					else if (iNormalHitBonusLuck == 36)
						iNormalHitBonus = 242;
					else if (iNormalHitBonusLuck == 35)
						iNormalHitBonus = 243;
					else
						iNormalHitBonus = number(200, 241);

					int iSkillBonus = number(-30, -20);

					item2->ChangeAttribute();
					item2->SetForceAttribute(0, 72, iNormalHitBonus);
					item2->SetForceAttribute(1, 71, iSkillBonus);
				}
				else if (item->GetVnum() == SWITCH_OBJECT_NORMAL4)
				{
					if (item2->GetType() != ITEM_WEAPON)
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Sadece silahlara efsun ekleyebilirsin."));
						return false;
					}

					if (item2->GetAttributeType(0) != 72 && item2->GetAttributeType(1) != 72)
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Sadece ortalama zararý efsunu olan silahlara efsun ekleyebilirsin."));
						return false;
					}

					int iNormalHitBonusLuck = number(1, 40);
					int iNormalHitBonus = 0;
					if (iNormalHitBonusLuck == 40)
						iNormalHitBonus = 400;
					else if (iNormalHitBonusLuck == 39)
						iNormalHitBonus = 398;
					else if (iNormalHitBonusLuck == 38)
						iNormalHitBonus = 396;
					else if (iNormalHitBonusLuck == 37)
						iNormalHitBonus = 394;
					else if (iNormalHitBonusLuck == 36)
						iNormalHitBonus = 392;
					else if (iNormalHitBonusLuck == 35)
						iNormalHitBonus = 390;
					else
						iNormalHitBonus = number(300, 389);

					int iSkillBonus = number(-30, -20);

					item2->ChangeAttribute();
					item2->SetForceAttribute(0, 72, iNormalHitBonus);
					item2->SetForceAttribute(1, 71, iSkillBonus);
				}
				else if (item->GetVnum() == SWITCH_OBJECT_NORMAL5)
				{
					if (item2->GetType() != ITEM_WEAPON)
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Sadece silahlara efsun ekleyebilirsin."));
						return false;
					}

					if (item2->GetAttributeType(0) != 72 && item2->GetAttributeType(1) != 72)
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Sadece ortalama zararý efsunu olan silahlara efsun ekleyebilirsin."));
						return false;
					}

					int iNormalHitBonusLuck = number(1, 40);
					int iNormalHitBonus = 0;
					if (iNormalHitBonusLuck == 40)
						iNormalHitBonus = 500;
					else if (iNormalHitBonusLuck == 39)
						iNormalHitBonus = 498;
					else if (iNormalHitBonusLuck == 38)
						iNormalHitBonus = 496;
					else if (iNormalHitBonusLuck == 37)
						iNormalHitBonus = 494;
					else if (iNormalHitBonusLuck == 36)
						iNormalHitBonus = 492;
					else if (iNormalHitBonusLuck == 35)
						iNormalHitBonus = 490;
					else
						iNormalHitBonus = number(400, 489);

					int iSkillBonus = number(-30, -20);

					item2->ChangeAttribute();
					item2->SetForceAttribute(0, 72, iNormalHitBonus);
					item2->SetForceAttribute(1, 71, iSkillBonus);
				}
				else if (item->GetVnum() == SWITCH_OBJECT_NORMAL6)
				{
					if (item2->GetType() != ITEM_WEAPON)
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Sadece silahlara efsun ekleyebilirsin."));
						return false;
					}

					if (item2->GetAttributeType(0) != 72 && item2->GetAttributeType(1) != 72)
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Sadece ortalama zararý efsunu olan silahlara efsun ekleyebilirsin."));
						return false;
					}

					int iNormalHitBonusLuck = number(1, 40);
					int iNormalHitBonus = 0;
					if (iNormalHitBonusLuck == 40)
						iNormalHitBonus = 1000;
					else if (iNormalHitBonusLuck == 39)
						iNormalHitBonus = 950;
					else if (iNormalHitBonusLuck == 38)
						iNormalHitBonus = 900;
					else if (iNormalHitBonusLuck == 37)
						iNormalHitBonus = 930;
					else if (iNormalHitBonusLuck == 36)
						iNormalHitBonus = 920;
					else if (iNormalHitBonusLuck == 35)
						iNormalHitBonus = 870;
					else
						iNormalHitBonus = number(500, 869);

					int iSkillBonus = number(-30, -20);

					item2->ChangeAttribute();
					item2->SetForceAttribute(0, 72, iNormalHitBonus);
					item2->SetForceAttribute(1, 71, iSkillBonus);
				}
				else if (item->GetVnum() == SWITCH_OBJECT_NORMAL7)
				{
					if (item2->GetType() != ITEM_WEAPON)
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Sadece silahlara efsun ekleyebilirsin."));
						return false;
					}

					if (item2->GetAttributeType(0) != 72 && item2->GetAttributeType(1) != 72)
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Sadece ortalama zararý efsunu olan silahlara efsun ekleyebilirsin."));
						return false;
					}

					int iNormalHitBonusLuck = number(1, 40);
					int iNormalHitBonus = 0;
					if (iNormalHitBonusLuck == 40)
						iNormalHitBonus = 450;
					else if (iNormalHitBonusLuck == 39)
						iNormalHitBonus = 446;
					else if (iNormalHitBonusLuck == 38)
						iNormalHitBonus = 444;
					else if (iNormalHitBonusLuck == 37)
						iNormalHitBonus = 442;
					else if (iNormalHitBonusLuck == 36)
						iNormalHitBonus = 440;
					else if (iNormalHitBonusLuck == 35)
						iNormalHitBonus = 438;
					else
						iNormalHitBonus = number(400, 437);

					int iSkillBonus = number(-30, -20);

					item2->ChangeAttribute();
					item2->SetForceAttribute(0, 72, iNormalHitBonus);
					item2->SetForceAttribute(1, 71, iSkillBonus);
				}
				else if (item->GetVnum() == SWITCH_OBJECT_NORMAL8)
				{
					if (item2->GetType() != ITEM_WEAPON)
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Sadece silahlara efsun ekleyebilirsin."));
						return false;
					}

					if (item2->GetAttributeType(0) != 72 && item2->GetAttributeType(1) != 72)
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Sadece ortalama zararý efsunu olan silahlara efsun ekleyebilirsin."));
						return false;
					}

					int iNormalHitBonusLuck = number(1, 40);
					int iNormalHitBonus = 0;
					if (iNormalHitBonusLuck == 40)
						iNormalHitBonus = 500;
					else if (iNormalHitBonusLuck == 39)
						iNormalHitBonus = 496;
					else if (iNormalHitBonusLuck == 38)
						iNormalHitBonus = 490;
					else if (iNormalHitBonusLuck == 37)
						iNormalHitBonus = 486;
					else if (iNormalHitBonusLuck == 36)
						iNormalHitBonus = 480;
					else if (iNormalHitBonusLuck == 35)
						iNormalHitBonus = 475;
					else
						iNormalHitBonus = number(450, 474);

					int iSkillBonus = number(-30, -20);

					item2->ChangeAttribute();
					item2->SetForceAttribute(0, 72, iNormalHitBonus);
					item2->SetForceAttribute(1, 71, iSkillBonus);
				}
				else if (item->GetVnum() == SWITCH_OBJECT_SKILL)
				{
					if (item2->GetType() != ITEM_WEAPON)
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Sadece silahlara efsun ekleyebilirsin."));
						return false;
					}

					if (item2->GetAttributeType(0) != 71 && item2->GetAttributeType(1) != 71)
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Sadece beceri hasar efsunu olan silahlara efsun ekleyebilirsin."));
						return false;
					}

					int iSkillBonusLuck = number(1, 50);
					int iSkillBonus = 0;
					if (iSkillBonusLuck == 50)
						iSkillBonus = 28;
					else if (iSkillBonusLuck == 49)
						iSkillBonus = 27;
					else if (iSkillBonusLuck == 48)
						iSkillBonus = 26;
					else if (iSkillBonusLuck == 47)
						iSkillBonus = 25;
					else if (iSkillBonusLuck == 46)
						iSkillBonus = 24;
					else
						iSkillBonus = number(15, 23);

					int iNormalHitBonus = 0;
					if (iSkillBonus <= 20)
						iNormalHitBonus = -2 * iSkillBonus + abs(number(-28, 28) + number(-28, 28)) + number(1, 4);
					else
						iNormalHitBonus = -2 * iSkillBonus + number(1, 40);

					item2->ChangeAttribute();
					item2->SetForceAttribute(0, 72, iNormalHitBonus);
					item2->SetForceAttribute(1, 71, iSkillBonus);
				}
				else if (item->GetVnum() == SWITCH_OBJECT_LAST_ATTR)
				{
					if (item2->GetAttributeCount() != 5)
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("º¯°æÇÒ ¼Ó¼ºÀÌ ¾ø½À´Ï´Ù."));
						return false;
					}

					item2->SetForceAttribute(2, 0, 0);
					item2->SetForceAttribute(3, 0, 0);
					item2->SetForceAttribute(4, 0, 0);
					item2->AddAttributeEx();
					item2->AddAttributeEx();
					item2->AddAttributeEx();
				}
#endif
#ifdef __NEWATTRPHEBIA__
				else if ((item->GetVnum() == PUT_MONSTER_ATTR_CHANGE))
				{
					if (item2->GetAttributeType(5) != APPLY_ATTBONUS_MONSTER)
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("PUT_MONSTER_ATTR_CHANGE_APPLY_ATTBONUS_MONSTER"));
						return false;
					}

					int prob = number(50, 1000);

					item2->SetForceAttribute(5, APPLY_ATTBONUS_MONSTER, prob);
					item->SetCount(item->GetCount()-1);
					return true;
				}

				else if ((item->GetVnum() == PUT_STONE_ATTR_CHANGE))
				{
					if (item2->GetAttributeType(6) != APPLY_ATTBONUS_STONE)
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("PUT_STONE_ATTR_CHANGE_APPLY_ATTBONUS_STONE"));
						return false;
					}

					int prob = number(50, 1000);

					item2->SetForceAttribute(6, APPLY_ATTBONUS_STONE, prob);
					item->SetCount(item->GetCount()-1);
					return true;
				}
#endif // __NEWATTRPHEBIA__
#ifdef __ANCIENT_ATTR_ITEM__
				else if (item->GetVnum() == 71052)
				{
					if (item2->GetAttributeCount() != 5)
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("º¯°æÇÒ ¼Ó¼ºÀÌ ¾ø½À´Ï´Ù."));
						return false;
					}

					item2->ChangeAttribute5TH();
				}
#endif
				else
				{
					if (item->GetVnum() == 71151 || item->GetVnum() == 76023)
					{
						if ((item2->GetType() == ITEM_WEAPON)
							|| (item2->GetType() == ITEM_ARMOR && item2->GetSubType() == ARMOR_BODY))
						{
							bool bCanUse = true;
							for (int i = 0; i < ITEM_LIMIT_MAX_NUM; ++i)
							{
								if (item2->GetLimitType(i) == LIMIT_LEVEL && item2->GetLimitValue(i) > 40)
								{
									bCanUse = false;
									break;
								}
							}
							if (false == bCanUse)
							{
								ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Àû¿ë ·¹º§º¸´Ù ³ô¾Æ »ç¿ëÀÌ ºÒ°¡´ÉÇÕ´Ï´Ù."));
								break;
							}
						}
						else
						{
							ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¹«±â¿Í °©¿Ê¿¡¸¸ »ç¿ë °¡´ÉÇÕ´Ï´Ù."));
							break;
						}
					}
					item2->ChangeAttribute();
				}

				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼ºÀ» º¯°æÇÏ¿´½À´Ï´Ù."));

				item->SetCount(item->GetCount() - 1);
#ifdef __CHARACTER_DETAILS_WINDOW__
				SetPlayerStatisticsValue(STATISTICS_TYPE_USE_ITEM_ATTR_COUNT, 1);
#endif
				break;

#if defined(__PENDANT_SYSTEM__) && defined(__PENDANT_ATTR__)
			case USE_CHANGE_PENDANT_ATTRIBUTE:
			{
				if (item2->GetAttributeSetIndex() == -1)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼ºÀ» º¯°æÇÒ ¼ö ¾ø´Â ¾ÆÀÌÅÛÀÔ´Ï´Ù."));
					return false;
				}

				if (item2->GetAttributeCount() == 0)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("º¯°æÇÒ ¼Ó¼ºÀÌ ¾ø½À´Ï´Ù."));
					return false;
				}

				if (item2->GetType() == ITEM_ARMOR && item2->GetSubType() == ARMOR_PENDANT)
				{
					item2->ChangeAttribute();
					item->SetCount(item->GetCount() - 1);
				}
				else
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´õÀÌ»ó ÀÌ ¾ÆÀÌÅÛÀ» ÀÌ¿ëÇÏ¿© ¼Ó¼ºÀ» Ãß°¡ÇÒ ¼ö ¾ø½À´Ï´Ù."));
				}
			}
			break;
#endif
#ifdef __AURA_COSTUME_SYSTEM__
			case USE_CHANGE_AURA_ATTRIBUTE:
			{
				if (item2->GetAttributeSetIndex() == -1)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼ºÀ» º¯°æÇÒ ¼ö ¾ø´Â ¾ÆÀÌÅÛÀÔ´Ï´Ù."));
					return false;
				}

				if (item2->GetAttributeCount() == 0)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("º¯°æÇÒ ¼Ó¼ºÀÌ ¾ø½À´Ï´Ù."));
					return false;
				}

				if (item2->GetType() == ITEM_COSTUME && item2->GetSubType() == COSTUME_AURA)
				{
					item2->ChangeAttribute();
					item->SetCount(item->GetCount() - 1);
				}
				else
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´õÀÌ»ó ÀÌ ¾ÆÀÌÅÛÀ» ÀÌ¿ëÇÏ¿© ¼Ó¼ºÀ» Ãß°¡ÇÒ ¼ö ¾ø½À´Ï´Ù."));
				}
			}
			break;
#endif
			case USE_ADD_ATTRIBUTE:
				if ((item->GetVnum() >= 30500 && item->GetVnum() <= 30521) || (item->GetVnum() >= 30590 && item->GetVnum() <= 30593))
				{
					if (ITEM_COSTUME == item2->GetType())
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼ºÀ» º¯°æÇÒ ¼ö ¾ø´Â ¾ÆÀÌÅÛÀÔ´Ï´Ù."));
						return false;
					}

					if (item->GetVnum() >= 30500 && item->GetVnum() <= 30505)
					{
						if (item2->GetType() == ITEM_WEAPON)
						{
							ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼ºÀ» º¯°æÇÒ ¼ö ¾ø´Â ¾ÆÀÌÅÛÀÔ´Ï´Ù."));
							return false;
						}
						if (item2->GetType() == ITEM_ARMOR && (item2->GetSubType() == ARMOR_SHIELD || item2->GetSubType() == ARMOR_HEAD || item2->GetSubType() == ARMOR_WRIST))
						{
							ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼ºÀ» º¯°æÇÒ ¼ö ¾ø´Â ¾ÆÀÌÅÛÀÔ´Ï´Ù."));
							return false;
						}
					}
					else if (item->GetVnum() == 30506)
					{
						if (item2->GetType() == ITEM_WEAPON)
						{
							ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼ºÀ» º¯°æÇÒ ¼ö ¾ø´Â ¾ÆÀÌÅÛÀÔ´Ï´Ù."));
							return false;
						}
						if (item2->GetType() == ITEM_ARMOR && (item2->GetSubType() == ARMOR_SHIELD || item2->GetSubType() == ARMOR_HEAD || item2->GetSubType() == ARMOR_EAR))
						{
							ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼ºÀ» º¯°æÇÒ ¼ö ¾ø´Â ¾ÆÀÌÅÛÀÔ´Ï´Ù."));
							return false;
						}
					}
					else if (item->GetVnum() == 30507)
					{
						if (item2->GetType() == ITEM_ARMOR && (item2->GetSubType() == ARMOR_SHIELD || item2->GetSubType() == ARMOR_HEAD || item2->GetSubType() == ARMOR_WRIST || item2->GetSubType() == ARMOR_EAR || item2->GetSubType() == ARMOR_NECK || item2->GetSubType() == ARMOR_FOOTS))
						{
							ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼ºÀ» º¯°æÇÒ ¼ö ¾ø´Â ¾ÆÀÌÅÛÀÔ´Ï´Ù."));
							return false;
						}
					}
					else if (item->GetVnum() == 30508)
					{
						if (item2->GetType() == ITEM_WEAPON)
						{
							ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼ºÀ» º¯°æÇÒ ¼ö ¾ø´Â ¾ÆÀÌÅÛÀÔ´Ï´Ù."));
							return false;
						}
						if (item2->GetType() == ITEM_ARMOR && (item2->GetSubType() == ARMOR_SHIELD || item2->GetSubType() == ARMOR_HEAD || item2->GetSubType() == ARMOR_WRIST || item2->GetSubType() == ARMOR_EAR || item2->GetSubType() == ARMOR_NECK || item2->GetSubType() == ARMOR_FOOTS))
						{
							ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼ºÀ» º¯°æÇÒ ¼ö ¾ø´Â ¾ÆÀÌÅÛÀÔ´Ï´Ù."));
							return false;
						}
					}
					else if (item->GetVnum() == 30509)
					{
						if (item2->GetType() == ITEM_WEAPON)
						{
							ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼ºÀ» º¯°æÇÒ ¼ö ¾ø´Â ¾ÆÀÌÅÛÀÔ´Ï´Ù."));
							return false;
						}
						if (item2->GetType() == ITEM_ARMOR && (item2->GetSubType() == ARMOR_SHIELD || item2->GetSubType() == ARMOR_HEAD || item2->GetSubType() == ARMOR_EAR || item2->GetSubType() == ARMOR_NECK || item2->GetSubType() == ARMOR_FOOTS))
						{
							ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼ºÀ» º¯°æÇÒ ¼ö ¾ø´Â ¾ÆÀÌÅÛÀÔ´Ï´Ù."));
							return false;
						}
					}
					else if (item->GetVnum() == 30510)
					{
						if (item2->GetType() == ITEM_ARMOR && (item2->GetSubType() == ARMOR_NECK || item2->GetSubType() == ARMOR_FOOTS || item2->GetSubType() == ARMOR_BODY))
						{
							ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼ºÀ» º¯°æÇÒ ¼ö ¾ø´Â ¾ÆÀÌÅÛÀÔ´Ï´Ù."));
							return false;
						}
					}
					else if (item->GetVnum() >= 30511 && item->GetVnum() <= 30514)
					{
						if (item2->GetType() == ITEM_ARMOR && (item2->GetSubType() == ARMOR_BODY || item2->GetSubType() == ARMOR_HEAD || item2->GetSubType() == ARMOR_WRIST || item2->GetSubType() == ARMOR_EAR || item2->GetSubType() == ARMOR_NECK || item2->GetSubType() == ARMOR_FOOTS))
						{
							ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼ºÀ» º¯°æÇÒ ¼ö ¾ø´Â ¾ÆÀÌÅÛÀÔ´Ï´Ù."));
							return false;
						}
					}
					else if (item->GetVnum() == 30515)
					{
						if (item2->GetType() == ITEM_ARMOR && (item2->GetSubType() == ARMOR_BODY || item2->GetSubType() == ARMOR_HEAD || item2->GetSubType() == ARMOR_WRIST || item2->GetSubType() == ARMOR_EAR || item2->GetSubType() == ARMOR_SHIELD))
						{
							ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼ºÀ» º¯°æÇÒ ¼ö ¾ø´Â ¾ÆÀÌÅÛÀÔ´Ï´Ù."));
							return false;
						}
					}
					else if (item->GetVnum() == 30516)
					{
						if (item2->GetType() == ITEM_ARMOR && (item2->GetSubType() == ARMOR_BODY || item2->GetSubType() == ARMOR_HEAD || item2->GetSubType() == ARMOR_EAR || item2->GetSubType() == ARMOR_SHIELD || item2->GetSubType() == ARMOR_FOOTS))
						{
							ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼ºÀ» º¯°æÇÒ ¼ö ¾ø´Â ¾ÆÀÌÅÛÀÔ´Ï´Ù."));
							return false;
						}
					}
					else if (item->GetVnum() == 30517)
					{
						if (item2->GetType() == ITEM_ARMOR && (item2->GetSubType() == ARMOR_BODY || item2->GetSubType() == ARMOR_WRIST || item2->GetSubType() == ARMOR_NECK || item2->GetSubType() == ARMOR_EAR || item2->GetSubType() == ARMOR_SHIELD || item2->GetSubType() == ARMOR_FOOTS))
						{
							ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼ºÀ» º¯°æÇÒ ¼ö ¾ø´Â ¾ÆÀÌÅÛÀÔ´Ï´Ù."));
							return false;
						}
					}
					else if (item->GetVnum() == 30518)
					{
						if (item2->GetType() == ITEM_WEAPON)
						{
							ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼ºÀ» º¯°æÇÒ ¼ö ¾ø´Â ¾ÆÀÌÅÛÀÔ´Ï´Ù."));
							return false;
						}
						if (item2->GetType() == ITEM_ARMOR && (item2->GetSubType() == ARMOR_BODY || item2->GetSubType() == ARMOR_WRIST || item2->GetSubType() == ARMOR_NECK || item2->GetSubType() == ARMOR_EAR || item2->GetSubType() == ARMOR_SHIELD))
						{
							ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼ºÀ» º¯°æÇÒ ¼ö ¾ø´Â ¾ÆÀÌÅÛÀÔ´Ï´Ù."));
							return false;
						}
					}
					else if (item->GetVnum() == 30519)
					{
						if (item2->GetType() == ITEM_WEAPON)
						{
							ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼ºÀ» º¯°æÇÒ ¼ö ¾ø´Â ¾ÆÀÌÅÛÀÔ´Ï´Ù."));
							return false;
						}
						if (item2->GetType() == ITEM_ARMOR && (item2->GetSubType() == ARMOR_BODY || item2->GetSubType() == ARMOR_FOOTS || item2->GetSubType() == ARMOR_NECK || item2->GetSubType() == ARMOR_EAR || item2->GetSubType() == ARMOR_SHIELD))
						{
							ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼ºÀ» º¯°æÇÒ ¼ö ¾ø´Â ¾ÆÀÌÅÛÀÔ´Ï´Ù."));
							return false;
						}
					}
					else if (item->GetVnum() == 30520)
					{
						if (item2->GetType() == ITEM_WEAPON)
						{
							ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼ºÀ» º¯°æÇÒ ¼ö ¾ø´Â ¾ÆÀÌÅÛÀÔ´Ï´Ù."));
							return false;
						}
						if (item2->GetType() == ITEM_ARMOR && (item2->GetSubType() == ARMOR_BODY || item2->GetSubType() == ARMOR_FOOTS || item2->GetSubType() == ARMOR_WRIST || item2->GetSubType() == ARMOR_EAR || item2->GetSubType() == ARMOR_SHIELD))
						{
							ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼ºÀ» º¯°æÇÒ ¼ö ¾ø´Â ¾ÆÀÌÅÛÀÔ´Ï´Ù."));
							return false;
						}
					}
					else if (item->GetVnum() == 30521)
					{
						if (item2->GetType() == ITEM_WEAPON)
						{
							ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼ºÀ» º¯°æÇÒ ¼ö ¾ø´Â ¾ÆÀÌÅÛÀÔ´Ï´Ù."));
							return false;
						}
						if (item2->GetType() == ITEM_ARMOR && (item2->GetSubType() == ARMOR_BODY || item2->GetSubType() == ARMOR_FOOTS || item2->GetSubType() == ARMOR_WRIST || item2->GetSubType() == ARMOR_EAR || item2->GetSubType() == ARMOR_NECK || item2->GetSubType() == ARMOR_HEAD))
						{
							ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼ºÀ» º¯°æÇÒ ¼ö ¾ø´Â ¾ÆÀÌÅÛÀÔ´Ï´Ù."));
							return false;
						}
					}
					else if (item->GetVnum() >= 30590 && item->GetVnum() <= 30593)
					{
						if (item2->GetType() == ITEM_ARMOR && (item2->GetSubType() == ARMOR_BODY || item2->GetSubType() == ARMOR_FOOTS || item2->GetSubType() == ARMOR_NECK))
						{
							ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼ºÀ» º¯°æÇÒ ¼ö ¾ø´Â ¾ÆÀÌÅÛÀÔ´Ï´Ù."));
							return false;
						}
					}
#if defined(__PENDANT_SYSTEM__) && defined(__PENDANT_ATTR__)
					if ((item->IsTilsimable()) && (item2->GetType() == ITEM_ARMOR && item2->GetSubType() == ARMOR_PENDANT))
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CANT_USE_THIS_USE_PEDANT_ADDER"));
						return false;
					}
#endif
					if (item2->GetAttributeSetIndex() == -1)
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼ºÀ» º¯°æÇÒ ¼ö ¾ø´Â ¾ÆÀÌÅÛÀÔ´Ï´Ù."));
						return false;
					}

					if (item2->IsExchanging()) // @duzenleme inputlara alinacak bir yer daha.
					{
						return false;
					}

#ifdef __BEGINNER_ITEM__
					if (item2->IsBasicItem())
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ITEM_IS_BASIC_CANNOT_DO"));
						return false;
					}
					if (item2->IsRealTimeItem() && item2->GetType() == ITEM_WEAPON)
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ITEM_IS_BEGINNER_CANNOT_DO"));
						return false;
					}
#endif
					if (item2->IsEquipped()) // @duzenleme inputlara alinacak bir yer daha.
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼ºÀ» º¯°æÇÒ ¼ö ¾ø´Â ¾ÆÀÌÅÛÀÔ´Ï´Ù."));
						return false;
					}

					if (item2->GetAttributeCount() >= 5)
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Bu eþyaya daha fazla efsun ekleyemezsin!"));
						return false;
					}

					if (item->GetValue(0) == item2->GetAttributeType(0) ||
						item->GetValue(0) == item2->GetAttributeType(1) ||
						item->GetValue(0) == item2->GetAttributeType(2) ||
						item->GetValue(0) == item2->GetAttributeType(3) ||
						item->GetValue(0) == item2->GetAttributeType(4))
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Ayný efsun tipini birden fazla kullanamazsýn!"));
						return false;
					}

					item2->SetForceAttribute(item2->GetAttributeCount(), item->GetValue(0), item->GetValue(1));

					item->SetCount(item->GetCount() - 1);
				}
				else
				{
					if (item->IsTilsimable() && ITEM_COSTUME == item2->GetType())
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼ºÀ» º¯°æÇÒ ¼ö ¾ø´Â ¾ÆÀÌÅÛÀÔ´Ï´Ù."));
						return false;
					}
#ifdef __PENDANT_ATTR__
					if ((item->IsTilsimable()) && (item2->GetType() == ITEM_ARMOR && item2->GetSubType() == ARMOR_PENDANT))
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CANT_USE_THIS_USE_PEDANT_ADDER"));
						return false;
					}
#endif
					if (item2->GetAttributeSetIndex() == -1)
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼ºÀ» º¯°æÇÒ ¼ö ¾ø´Â ¾ÆÀÌÅÛÀÔ´Ï´Ù."));
						return false;
					}

					if (item2->IsExchanging()) // @duzenleme inputlara alinacak bir yer daha.
					{
						return false;
					}

#ifdef __BEGINNER_ITEM__
					if (item2->IsBasicItem())
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ITEM_IS_BASIC_CANNOT_DO"));
						return false;
					}
#endif
					if (item2->IsEquipped()) // @duzenleme inputlara alinacak bir yer daha.
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼ºÀ» º¯°æÇÒ ¼ö ¾ø´Â ¾ÆÀÌÅÛÀÔ´Ï´Ù."));
						return false;
					}

#ifdef __NEWATTRPHEBIA__
					if ((item->GetVnum() == PUT_MONSTER_ATTR_ADD))
					{
						if (item2->GetAttributeCount() + item2->GetRareAttrCount() < 5)
						{
							ChatPacket(CHAT_TYPE_INFO, LC_TEXT("PUT_MONSTER_ATTR_ADD_COUNT"));
							return false;
						}

						if (item2->GetAttributeType(5) == APPLY_ATTBONUS_MONSTER && item2->GetAttributeValue(5) != 0)
						{
							ChatPacket(CHAT_TYPE_INFO, LC_TEXT("PUT_MONSTER_ATTR_ADD_APPLY_ATTBONUS_MONSTER"));
							return false;
						}

						item2->SetForceAttribute(5, APPLY_ATTBONUS_MONSTER, 1);
						item->SetCount(item->GetCount()-1);
						return true;
					}

					if ((item->GetVnum() == PUT_STONE_ATTR_ADD))
					{
						if (item2->GetAttributeCount() + item2->GetRareAttrCount() < 6)
						{
							ChatPacket(CHAT_TYPE_INFO, LC_TEXT("PUT_STONE_ATTR_ADD_COUNT"));
							return false;
						}

						if (item2->GetAttributeType(5) != APPLY_ATTBONUS_MONSTER && item2->GetAttributeValue(5) != 0)
						{
							ChatPacket(CHAT_TYPE_INFO, LC_TEXT("PUT_STONE_ATTR_ADD_APPLY_ATTBONUS_MONSTER"));
							return false;
						}

						if (item2->GetAttributeType(6) == APPLY_ATTBONUS_STONE && item2->GetAttributeValue(6) != 0)
						{
							ChatPacket(CHAT_TYPE_INFO, LC_TEXT("PUT_STONE_ATTR_ADD_APPLY_ATTBONUS_STONE"));
							return false;
						}

						item2->SetForceAttribute(6, APPLY_ATTBONUS_STONE, 1);
						item->SetCount(item->GetCount()-1);
						return true;
					}
#endif // __NEWATTRPHEBIA__

					if (item2->GetAttributeCount() < 5)
					{
						if (item->GetVnum() == 71152 || item->GetVnum() == 76024)
						{
							if ((item2->GetType() == ITEM_WEAPON)
								|| (item2->GetType() == ITEM_ARMOR && item2->GetSubType() == ARMOR_BODY))
							{
								bool bCanUse = true;
								for (int i = 0; i < ITEM_LIMIT_MAX_NUM; ++i)
								{
									if (item2->GetLimitType(i) == LIMIT_LEVEL && item2->GetLimitValue(i) > 40)
									{
										bCanUse = false;
										break;
									}
								}
								if (false == bCanUse)
								{
									ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Àû¿ë ·¹º§º¸´Ù ³ô¾Æ »ç¿ëÀÌ ºÒ°¡´ÉÇÕ´Ï´Ù."));
									break;
								}
							}
							else
							{
								ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¹«±â¿Í °©¿Ê¿¡¸¸ »ç¿ë °¡´ÉÇÕ´Ï´Ù."));
								break;
							}
						}
						if (number(1, 100) <= aiItemAttributeAddPercent[item2->GetAttributeCount()])
						{
							while (item2->GetAttributeCount() < 5)
								item2->AddAttribute();
							ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼º Ãß°¡¿¡ ¼º°øÇÏ¿´½À´Ï´Ù."));
						}
						else
						{
							ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼º Ãß°¡¿¡ ½ÇÆÐÇÏ¿´½À´Ï´Ù."));
						}

						item->SetCount(item->GetCount() - 1);
					}
					else
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´õÀÌ»ó ÀÌ ¾ÆÀÌÅÛÀ» ÀÌ¿ëÇÏ¿© ¼Ó¼ºÀ» Ãß°¡ÇÒ ¼ö ¾ø½À´Ï´Ù."));
					}
				}
				break;

			case USE_ADD_ATTRIBUTE2:
				if (ITEM_COSTUME == item2->GetType())
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼ºÀ» º¯°æÇÒ ¼ö ¾ø´Â ¾ÆÀÌÅÛÀÔ´Ï´Ù."));
					return false;
				}
#ifdef __PENDANT_ATTR__
				if ((item->IsTilsimable()) && (item2->GetType() == ITEM_ARMOR && item2->GetSubType() == ARMOR_PENDANT))
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CANT_USE_THIS_USE_PEDANT_ADDER"));
					return false;
				}
#endif
				if (item2->IsExchanging()) // @duzenleme inputlara alinacak bir yer daha.
				{
					return false;
				}

#ifdef __BEGINNER_ITEM__
				if (item2->IsBasicItem())
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ITEM_IS_BASIC_CANNOT_DO"));
					return false;
				}
#endif

				if (item2->IsEquipped()) // @duzenleme inputlara alinacak bir yer daha.
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼ºÀ» º¯°æÇÒ ¼ö ¾ø´Â ¾ÆÀÌÅÛÀÔ´Ï´Ù."));
					return false;
				}
				if (item2->GetAttributeSetIndex() == -1)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼ºÀ» º¯°æÇÒ ¼ö ¾ø´Â ¾ÆÀÌÅÛÀÔ´Ï´Ù."));
					return false;
				}

				if (item2->GetAttributeCount() == 4)
				{
					if (number(1, 100) <= aiItemAttributeAddPercent[item2->GetAttributeCount()])
					{
						item2->AddAttribute();
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼º Ãß°¡¿¡ ¼º°øÇÏ¿´½À´Ï´Ù."));
					}
					else
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼º Ãß°¡¿¡ ½ÇÆÐÇÏ¿´½À´Ï´Ù."));
					}

					item->SetCount(item->GetCount() - 1);
				}
				else if (item2->GetAttributeCount() == 5)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´õ ÀÌ»ó ÀÌ ¾ÆÀÌÅÛÀ» ÀÌ¿ëÇÏ¿© ¼Ó¼ºÀ» Ãß°¡ÇÒ ¼ö ¾ø½À´Ï´Ù."));
				}
				else if (item2->GetAttributeCount() < 4)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¸ÕÀú Àç°¡ºñ¼­¸¦ ÀÌ¿ëÇÏ¿© ¼Ó¼ºÀ» Ãß°¡½ÃÄÑ ÁÖ¼¼¿ä."));
				}
				else
				{
					// wtf ?!
					sys_err("ADD_ATTRIBUTE2 : Item has wrong AttributeCount(%d)", item2->GetAttributeCount());
				}
				break;

#if defined(__PENDANT_SYSTEM__) && defined(__PENDANT_ATTR__)
			case USE_ADD_PENDANT_ATTRIBUTE:
			{
				if (item2->GetAttributeSetIndex() == -1)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼ºÀ» º¯°æÇÒ ¼ö ¾ø´Â ¾ÆÀÌÅÛÀÔ´Ï´Ù."));
					return false;
				}

				if (item2->GetType() == ITEM_ARMOR && item2->GetSubType() == ARMOR_PENDANT)
				{
					if (item2->GetAttributeCount() < 4)
					{
						if (number(1, 100) <= aiItemAttributeAddPercent[item2->GetAttributeCount()])
						{
							item2->AddAttribute();
							ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼º Ãß°¡¿¡ ¼º°øÇÏ¿´½À´Ï´Ù."));
						}
						else
						{
							ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼º Ãß°¡¿¡ ½ÇÆÐÇÏ¿´½À´Ï´Ù."));
						}

						item->SetCount(item->GetCount() - 1);
					}
					else
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´õÀÌ»ó ÀÌ ¾ÆÀÌÅÛÀ» ÀÌ¿ëÇÏ¿© ¼Ó¼ºÀ» Ãß°¡ÇÒ ¼ö ¾ø½À´Ï´Ù."));
					}
				}
				else
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´õÀÌ»ó ÀÌ ¾ÆÀÌÅÛÀ» ÀÌ¿ëÇÏ¿© ¼Ó¼ºÀ» Ãß°¡ÇÒ ¼ö ¾ø½À´Ï´Ù."));
				}
			}
			break;

			case USE_ADD_PENDANT_FIVE_ATTRIBUTE:
			{
				if (item2->GetAttributeSetIndex() == -1)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼ºÀ» º¯°æÇÒ ¼ö ¾ø´Â ¾ÆÀÌÅÛÀÔ´Ï´Ù."));
					return false;
				}
				if (item2->GetType() == ITEM_ARMOR && item2->GetSubType() == ARMOR_PENDANT)
				{
					if (item2->GetAttributeCount() == 4)
					{
						if (number(1, 100) <= aiItemAttributeAddPercent[item2->GetAttributeCount()])
						{
							item2->AddAttribute();
							ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼º Ãß°¡¿¡ ¼º°øÇÏ¿´½À´Ï´Ù."));
						}
						else
						{
							ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼º Ãß°¡¿¡ ½ÇÆÐÇÏ¿´½À´Ï´Ù."));
						}
						item->SetCount(item->GetCount() - 1);
					}
					else if (item2->GetAttributeCount() == 5)
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´õ ÀÌ»ó ÀÌ ¾ÆÀÌÅÛÀ» ÀÌ¿ëÇÏ¿© ¼Ó¼ºÀ» Ãß°¡ÇÒ ¼ö ¾ø½À´Ï´Ù."));
					}
					else if (item2->GetAttributeCount() < 4)
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¸ÕÀú Àç°¡ºñ¼­¸¦ ÀÌ¿ëÇÏ¿© ¼Ó¼ºÀ» Ãß°¡½ÃÄÑ ÁÖ¼¼¿ä."));
					}
				}
				else
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´õÀÌ»ó ÀÌ ¾ÆÀÌÅÛÀ» ÀÌ¿ëÇÏ¿© ¼Ó¼ºÀ» Ãß°¡ÇÒ ¼ö ¾ø½À´Ï´Ù."));
				}
			}
			break;
#endif
#ifdef __AURA_COSTUME_SYSTEM__
			case USE_ADD_AURA_ATTRIBUTE:
			{
				if (item2->GetAttributeSetIndex() == -1)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼ºÀ» º¯°æÇÒ ¼ö ¾ø´Â ¾ÆÀÌÅÛÀÔ´Ï´Ù."));
					return false;
				}

				if (item2->GetType() == ITEM_COSTUME && item2->GetSubType() == COSTUME_AURA)
				{
					if (item2->GetAttributeCount() < 4)
					{
						if (number(1, 100) <= aiItemAttributeAddPercent[item2->GetAttributeCount()])
						{
							item2->AddAttribute();
							ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼º Ãß°¡¿¡ ¼º°øÇÏ¿´½À´Ï´Ù."));
						}
						else
						{
							ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼º Ãß°¡¿¡ ½ÇÆÐÇÏ¿´½À´Ï´Ù."));
						}

						item->SetCount(item->GetCount() - 1);
					}
					else
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´õÀÌ»ó ÀÌ ¾ÆÀÌÅÛÀ» ÀÌ¿ëÇÏ¿© ¼Ó¼ºÀ» Ãß°¡ÇÒ ¼ö ¾ø½À´Ï´Ù."));
					}
				}
				else
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´õÀÌ»ó ÀÌ ¾ÆÀÌÅÛÀ» ÀÌ¿ëÇÏ¿© ¼Ó¼ºÀ» Ãß°¡ÇÒ ¼ö ¾ø½À´Ï´Ù."));
				}
			}
			break;

			case USE_ADD_AURA_FIVE_ATTRIBUTE:
			{
				if (item2->GetAttributeSetIndex() == -1)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼ºÀ» º¯°æÇÒ ¼ö ¾ø´Â ¾ÆÀÌÅÛÀÔ´Ï´Ù."));
					return false;
				}
				if (item2->GetType() == ITEM_COSTUME && item2->GetSubType() == COSTUME_AURA)
				{
					if (item2->GetAttributeCount() == 4)
					{
						if (number(1, 100) <= aiItemAttributeAddPercent[item2->GetAttributeCount()])
						{
							item2->AddAttribute();
							ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼º Ãß°¡¿¡ ¼º°øÇÏ¿´½À´Ï´Ù."));
						}
						else
						{
							ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼º Ãß°¡¿¡ ½ÇÆÐÇÏ¿´½À´Ï´Ù."));
						}
						item->SetCount(item->GetCount() - 1);
					}
					else if (item2->GetAttributeCount() == 5)
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´õ ÀÌ»ó ÀÌ ¾ÆÀÌÅÛÀ» ÀÌ¿ëÇÏ¿© ¼Ó¼ºÀ» Ãß°¡ÇÒ ¼ö ¾ø½À´Ï´Ù."));
					}
					else if (item2->GetAttributeCount() < 4)
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¸ÕÀú Àç°¡ºñ¼­¸¦ ÀÌ¿ëÇÏ¿© ¼Ó¼ºÀ» Ãß°¡½ÃÄÑ ÁÖ¼¼¿ä."));
					}
				}
				else
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´õÀÌ»ó ÀÌ ¾ÆÀÌÅÛÀ» ÀÌ¿ëÇÏ¿© ¼Ó¼ºÀ» Ãß°¡ÇÒ ¼ö ¾ø½À´Ï´Ù."));
				}
			}
			break;
#endif
#ifdef __MOUNT_COSTUME_SYSTEM__
			case USE_ADD_MOUNT_ATTRIBUTE:
			{
				if (item2->GetAttributeSetIndex() == -1)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼ºÀ» º¯°æÇÒ ¼ö ¾ø´Â ¾ÆÀÌÅÛÀÔ´Ï´Ù."));
					return false;
				}

				if (item2->GetType() == ITEM_COSTUME && item2->GetSubType() == COSTUME_MOUNT)
				{
					if (item2->GetAttributeCount() < 4)
					{
						if (number(1, 100) <= aiItemAttributeAddPercent[item2->GetAttributeCount()])
						{
							item2->AddAttribute();
							ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼º Ãß°¡¿¡ ¼º°øÇÏ¿´½À´Ï´Ù."));
						}
						else
						{
							ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼º Ãß°¡¿¡ ½ÇÆÐÇÏ¿´½À´Ï´Ù."));
						}

						item->SetCount(item->GetCount() - 1);
					}
					else
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´õÀÌ»ó ÀÌ ¾ÆÀÌÅÛÀ» ÀÌ¿ëÇÏ¿© ¼Ó¼ºÀ» Ãß°¡ÇÒ ¼ö ¾ø½À´Ï´Ù."));
					}
				}
				else
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´õÀÌ»ó ÀÌ ¾ÆÀÌÅÛÀ» ÀÌ¿ëÇÏ¿© ¼Ó¼ºÀ» Ãß°¡ÇÒ ¼ö ¾ø½À´Ï´Ù."));
				}
			}
			break;

			case USE_ADD_FIVE_MOUNT_ATTRIBUTE:
			{
				if (item2->GetAttributeSetIndex() == -1)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼ºÀ» º¯°æÇÒ ¼ö ¾ø´Â ¾ÆÀÌÅÛÀÔ´Ï´Ù."));
					return false;
				}
				if (item2->GetType() == ITEM_COSTUME && item2->GetSubType() == COSTUME_MOUNT)
				{
					if (item2->GetAttributeCount() == 4)
					{
						if (number(1, 100) <= aiItemAttributeAddPercent[item2->GetAttributeCount()])
						{
							item2->AddAttribute();
							ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼º Ãß°¡¿¡ ¼º°øÇÏ¿´½À´Ï´Ù."));
						}
						else
						{
							ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼º Ãß°¡¿¡ ½ÇÆÐÇÏ¿´½À´Ï´Ù."));
						}
						item->SetCount(item->GetCount() - 1);
					}
					else if (item2->GetAttributeCount() == 5)
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´õ ÀÌ»ó ÀÌ ¾ÆÀÌÅÛÀ» ÀÌ¿ëÇÏ¿© ¼Ó¼ºÀ» Ãß°¡ÇÒ ¼ö ¾ø½À´Ï´Ù."));
					}
					else if (item2->GetAttributeCount() < 4)
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¸ÕÀú Àç°¡ºñ¼­¸¦ ÀÌ¿ëÇÏ¿© ¼Ó¼ºÀ» Ãß°¡½ÃÄÑ ÁÖ¼¼¿ä."));
					}
				}
				else
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´õÀÌ»ó ÀÌ ¾ÆÀÌÅÛÀ» ÀÌ¿ëÇÏ¿© ¼Ó¼ºÀ» Ãß°¡ÇÒ ¼ö ¾ø½À´Ï´Ù."));
				}
			}
			break;

			case USE_CHANGE_MOUNT_ATTRIBUTE:
			{
				if (item2->GetAttributeSetIndex() == -1)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼ºÀ» º¯°æÇÒ ¼ö ¾ø´Â ¾ÆÀÌÅÛÀÔ´Ï´Ù."));
					return false;
				}

				if (item2->GetAttributeCount() == 0)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("º¯°æÇÒ ¼Ó¼ºÀÌ ¾ø½À´Ï´Ù."));
					return false;
				}

				if (item2->GetType() == ITEM_COSTUME && item2->GetSubType() == COSTUME_MOUNT)
				{
					item2->ChangeAttribute();
					item->SetCount(item->GetCount() - 1);
				}
				else
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´õÀÌ»ó ÀÌ ¾ÆÀÌÅÛÀ» ÀÌ¿ëÇÏ¿© ¼Ó¼ºÀ» Ãß°¡ÇÒ ¼ö ¾ø½À´Ï´Ù."));
				}
			}
			break;
#endif
#ifdef __WEAPON_COSTUME_SYSTEM__
			case USE_ADD_WEAPON_COSTUME_ATTRIBUTE:
			{
				if (item2->GetAttributeSetIndex() == -1)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼ºÀ» º¯°æÇÒ ¼ö ¾ø´Â ¾ÆÀÌÅÛÀÔ´Ï´Ù."));
					return false;
				}

				if (item2->GetType() == ITEM_COSTUME && item2->GetSubType() == COSTUME_WEAPON)
				{
					if (item2->GetAttributeCount() < 4)
					{
						if (number(1, 100) <= aiItemAttributeAddPercent[item2->GetAttributeCount()])
						{
							item2->AddAttribute();
							ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼º Ãß°¡¿¡ ¼º°øÇÏ¿´½À´Ï´Ù."));
						}
						else
						{
							ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼º Ãß°¡¿¡ ½ÇÆÐÇÏ¿´½À´Ï´Ù."));
						}

						item->SetCount(item->GetCount() - 1);
					}
					else
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´õÀÌ»ó ÀÌ ¾ÆÀÌÅÛÀ» ÀÌ¿ëÇÏ¿© ¼Ó¼ºÀ» Ãß°¡ÇÒ ¼ö ¾ø½À´Ï´Ù."));
					}
				}
				else
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´õÀÌ»ó ÀÌ ¾ÆÀÌÅÛÀ» ÀÌ¿ëÇÏ¿© ¼Ó¼ºÀ» Ãß°¡ÇÒ ¼ö ¾ø½À´Ï´Ù."));
				}
			}
			break;

			case USE_ADD_FIVE_WEAPON_COSTUME_ATTRIBUTE:
			{
				if (item2->GetAttributeSetIndex() == -1)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼ºÀ» º¯°æÇÒ ¼ö ¾ø´Â ¾ÆÀÌÅÛÀÔ´Ï´Ù."));
					return false;
				}
				if (item2->GetType() == ITEM_COSTUME && item2->GetSubType() == COSTUME_WEAPON)
				{
					if (item2->GetAttributeCount() == 4)
					{
						if (number(1, 100) <= aiItemAttributeAddPercent[item2->GetAttributeCount()])
						{
							item2->AddAttribute();
							ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼º Ãß°¡¿¡ ¼º°øÇÏ¿´½À´Ï´Ù."));
						}
						else
						{
							ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼º Ãß°¡¿¡ ½ÇÆÐÇÏ¿´½À´Ï´Ù."));
						}
						item->SetCount(item->GetCount() - 1);
					}
					else if (item2->GetAttributeCount() == 5)
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´õ ÀÌ»ó ÀÌ ¾ÆÀÌÅÛÀ» ÀÌ¿ëÇÏ¿© ¼Ó¼ºÀ» Ãß°¡ÇÒ ¼ö ¾ø½À´Ï´Ù."));
					}
					else if (item2->GetAttributeCount() < 4)
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¸ÕÀú Àç°¡ºñ¼­¸¦ ÀÌ¿ëÇÏ¿© ¼Ó¼ºÀ» Ãß°¡½ÃÄÑ ÁÖ¼¼¿ä."));
					}
				}
				else
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´õÀÌ»ó ÀÌ ¾ÆÀÌÅÛÀ» ÀÌ¿ëÇÏ¿© ¼Ó¼ºÀ» Ãß°¡ÇÒ ¼ö ¾ø½À´Ï´Ù."));
				}
			}
			break;

			case USE_CHANGE_WEAPON_COSTUME_ATTRIBUTE:
			{
				if (item2->GetAttributeSetIndex() == -1)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼ºÀ» º¯°æÇÒ ¼ö ¾ø´Â ¾ÆÀÌÅÛÀÔ´Ï´Ù."));
					return false;
				}

				if (item2->GetAttributeCount() == 0)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("º¯°æÇÒ ¼Ó¼ºÀÌ ¾ø½À´Ï´Ù."));
					return false;
				}

				if (item2->GetType() == ITEM_COSTUME && item2->GetSubType() == COSTUME_WEAPON)
				{
					item2->ChangeAttribute();
					item->SetCount(item->GetCount() - 1);
				}
				else
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´õÀÌ»ó ÀÌ ¾ÆÀÌÅÛÀ» ÀÌ¿ëÇÏ¿© ¼Ó¼ºÀ» Ãß°¡ÇÒ ¼ö ¾ø½À´Ï´Ù."));
				}
			}
			break;
#endif
#ifdef __MARTY_BELT_INVENTORY__
			case USE_ADD_BELT_ATTRIBUTE:
			{
				if (item2->GetAttributeSetIndex() == -1)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼ºÀ» º¯°æÇÒ ¼ö ¾ø´Â ¾ÆÀÌÅÛÀÔ´Ï´Ù."));
					return false;
				}

				if (item2->GetType() == ITEM_BELT)
				{
					if (item2->GetAttributeCount() < 4)
					{
						if (number(1, 100) <= aiItemAttributeAddPercent[item2->GetAttributeCount()])
						{
							item2->AddAttribute();
							ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼º Ãß°¡¿¡ ¼º°øÇÏ¿´½À´Ï´Ù."));
						}
						else
						{
							ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼º Ãß°¡¿¡ ½ÇÆÐÇÏ¿´½À´Ï´Ù."));
						}

						item->SetCount(item->GetCount() - 1);
					}
					else
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´õÀÌ»ó ÀÌ ¾ÆÀÌÅÛÀ» ÀÌ¿ëÇÏ¿© ¼Ó¼ºÀ» Ãß°¡ÇÒ ¼ö ¾ø½À´Ï´Ù."));
					}
				}
				else
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´õÀÌ»ó ÀÌ ¾ÆÀÌÅÛÀ» ÀÌ¿ëÇÏ¿© ¼Ó¼ºÀ» Ãß°¡ÇÒ ¼ö ¾ø½À´Ï´Ù."));
				}
			}
			break;

			case USE_ADD_FIVE_BELT_ATTRIBUTE:
			{
				if (item2->GetAttributeSetIndex() == -1)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼ºÀ» º¯°æÇÒ ¼ö ¾ø´Â ¾ÆÀÌÅÛÀÔ´Ï´Ù."));
					return false;
				}
				if (item2->GetType() == ITEM_BELT)
				{
					if (item2->GetAttributeCount() == 4)
					{
						if (number(1, 100) <= aiItemAttributeAddPercent[item2->GetAttributeCount()])
						{
							item2->AddAttribute();
							ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼º Ãß°¡¿¡ ¼º°øÇÏ¿´½À´Ï´Ù."));
						}
						else
						{
							ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼º Ãß°¡¿¡ ½ÇÆÐÇÏ¿´½À´Ï´Ù."));
						}
						item->SetCount(item->GetCount() - 1);
					}
					else if (item2->GetAttributeCount() == 5)
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´õ ÀÌ»ó ÀÌ ¾ÆÀÌÅÛÀ» ÀÌ¿ëÇÏ¿© ¼Ó¼ºÀ» Ãß°¡ÇÒ ¼ö ¾ø½À´Ï´Ù."));
					}
					else if (item2->GetAttributeCount() < 4)
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¸ÕÀú Àç°¡ºñ¼­¸¦ ÀÌ¿ëÇÏ¿© ¼Ó¼ºÀ» Ãß°¡½ÃÄÑ ÁÖ¼¼¿ä."));
					}
				}
				else
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´õÀÌ»ó ÀÌ ¾ÆÀÌÅÛÀ» ÀÌ¿ëÇÏ¿© ¼Ó¼ºÀ» Ãß°¡ÇÒ ¼ö ¾ø½À´Ï´Ù."));
				}
			}
			break;

			case USE_CHANGE_BELT_ATTRIBUTE:
			{
				if (item2->GetAttributeSetIndex() == -1)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼ºÀ» º¯°æÇÒ ¼ö ¾ø´Â ¾ÆÀÌÅÛÀÔ´Ï´Ù."));
					return false;
				}

				if (item2->GetAttributeCount() == 0)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("º¯°æÇÒ ¼Ó¼ºÀÌ ¾ø½À´Ï´Ù."));
					return false;
				}

				if (item2->GetType() == ITEM_BELT)
				{
					item2->ChangeAttribute();
					item->SetCount(item->GetCount() - 1);
				}
				else
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´õÀÌ»ó ÀÌ ¾ÆÀÌÅÛÀ» ÀÌ¿ëÇÏ¿© ¼Ó¼ºÀ» Ãß°¡ÇÒ ¼ö ¾ø½À´Ï´Ù."));
				}
			}
			break;
#endif
#ifdef __HAIR_COSTUME_ATTRIBUTE__
			case USE_ADD_HAIR_COSTUME_ATTRIBUTE:
			{
				if (item2->GetAttributeSetIndex() == -1)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼ºÀ» º¯°æÇÒ ¼ö ¾ø´Â ¾ÆÀÌÅÛÀÔ´Ï´Ù."));
					return false;
				}

				if (item2->GetType() == ITEM_COSTUME && item2->GetSubType() == COSTUME_HAIR)
				{
					if (item2->GetAttributeCount() < 4)
					{
						if (number(1, 100) <= aiItemAttributeAddPercent[item2->GetAttributeCount()])
						{
							item2->AddAttribute();
							ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼º Ãß°¡¿¡ ¼º°øÇÏ¿´½À´Ï´Ù."));
						}
						else
						{
							ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼º Ãß°¡¿¡ ½ÇÆÐÇÏ¿´½À´Ï´Ù."));
						}

						item->SetCount(item->GetCount() - 1);
					}
					else
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´õÀÌ»ó ÀÌ ¾ÆÀÌÅÛÀ» ÀÌ¿ëÇÏ¿© ¼Ó¼ºÀ» Ãß°¡ÇÒ ¼ö ¾ø½À´Ï´Ù."));
					}
				}
				else
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´õÀÌ»ó ÀÌ ¾ÆÀÌÅÛÀ» ÀÌ¿ëÇÏ¿© ¼Ó¼ºÀ» Ãß°¡ÇÒ ¼ö ¾ø½À´Ï´Ù."));
				}
			}
			break;

			case USE_ADD_FIVE_HAIR_COSTUME_ATTRIBUTE:
			{
				if (item2->GetAttributeSetIndex() == -1)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼ºÀ» º¯°æÇÒ ¼ö ¾ø´Â ¾ÆÀÌÅÛÀÔ´Ï´Ù."));
					return false;
				}
				if (item2->GetType() == ITEM_COSTUME && item2->GetSubType() == COSTUME_HAIR)
				{
					if (item2->GetAttributeCount() == 4)
					{
						if (number(1, 100) <= aiItemAttributeAddPercent[item2->GetAttributeCount()])
						{
							item2->AddAttribute();
							ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼º Ãß°¡¿¡ ¼º°øÇÏ¿´½À´Ï´Ù."));
						}
						else
						{
							ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼º Ãß°¡¿¡ ½ÇÆÐÇÏ¿´½À´Ï´Ù."));
						}
						item->SetCount(item->GetCount() - 1);
					}
					else if (item2->GetAttributeCount() == 5)
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´õ ÀÌ»ó ÀÌ ¾ÆÀÌÅÛÀ» ÀÌ¿ëÇÏ¿© ¼Ó¼ºÀ» Ãß°¡ÇÒ ¼ö ¾ø½À´Ï´Ù."));
					}
					else if (item2->GetAttributeCount() < 4)
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¸ÕÀú Àç°¡ºñ¼­¸¦ ÀÌ¿ëÇÏ¿© ¼Ó¼ºÀ» Ãß°¡½ÃÄÑ ÁÖ¼¼¿ä."));
					}
				}
				else
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´õÀÌ»ó ÀÌ ¾ÆÀÌÅÛÀ» ÀÌ¿ëÇÏ¿© ¼Ó¼ºÀ» Ãß°¡ÇÒ ¼ö ¾ø½À´Ï´Ù."));
				}
			}
			break;

			case USE_CHANGE_HAIR_COSTUME_ATTRIBUTE:
			{
				if (item2->GetAttributeSetIndex() == -1)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼ºÀ» º¯°æÇÒ ¼ö ¾ø´Â ¾ÆÀÌÅÛÀÔ´Ï´Ù."));
					return false;
				}

				if (item2->GetAttributeCount() == 0)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("º¯°æÇÒ ¼Ó¼ºÀÌ ¾ø½À´Ï´Ù."));
					return false;
				}

				if (item2->GetType() == ITEM_COSTUME && item2->GetSubType() == COSTUME_HAIR)
				{
					item2->ChangeAttribute();
					item->SetCount(item->GetCount() - 1);
				}
				else
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´õÀÌ»ó ÀÌ ¾ÆÀÌÅÛÀ» ÀÌ¿ëÇÏ¿© ¼Ó¼ºÀ» Ãß°¡ÇÒ ¼ö ¾ø½À´Ï´Ù."));
				}
			}
			break;
#endif
#ifdef __BODY_COSTUME_ATTRIBUTE__
			case USE_ADD_BODY_COSTUME_ATTRIBUTE:
			{
				if (item2->GetAttributeSetIndex() == -1)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼ºÀ» º¯°æÇÒ ¼ö ¾ø´Â ¾ÆÀÌÅÛÀÔ´Ï´Ù."));
					return false;
				}

				if (item2->GetType() == ITEM_COSTUME && item2->GetSubType() == COSTUME_BODY)
				{
					if (item2->GetAttributeCount() < 4)
					{
						if (number(1, 100) <= aiItemAttributeAddPercent[item2->GetAttributeCount()])
						{
							item2->AddAttribute();
							ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼º Ãß°¡¿¡ ¼º°øÇÏ¿´½À´Ï´Ù."));
						}
						else
						{
							ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼º Ãß°¡¿¡ ½ÇÆÐÇÏ¿´½À´Ï´Ù."));
						}

						item->SetCount(item->GetCount() - 1);
					}
					else
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´õÀÌ»ó ÀÌ ¾ÆÀÌÅÛÀ» ÀÌ¿ëÇÏ¿© ¼Ó¼ºÀ» Ãß°¡ÇÒ ¼ö ¾ø½À´Ï´Ù."));
					}
				}
				else
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´õÀÌ»ó ÀÌ ¾ÆÀÌÅÛÀ» ÀÌ¿ëÇÏ¿© ¼Ó¼ºÀ» Ãß°¡ÇÒ ¼ö ¾ø½À´Ï´Ù."));
				}
			}
			break;

			case USE_ADD_FIVE_BODY_COSTUME_ATTRIBUTE:
			{
				if (item2->GetAttributeSetIndex() == -1)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼ºÀ» º¯°æÇÒ ¼ö ¾ø´Â ¾ÆÀÌÅÛÀÔ´Ï´Ù."));
					return false;
				}
				if (item2->GetType() == ITEM_COSTUME && item2->GetSubType() == COSTUME_BODY)
				{
					if (item2->GetAttributeCount() == 4)
					{
						if (number(1, 100) <= aiItemAttributeAddPercent[item2->GetAttributeCount()])
						{
							item2->AddAttribute();
							ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼º Ãß°¡¿¡ ¼º°øÇÏ¿´½À´Ï´Ù."));
						}
						else
						{
							ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼º Ãß°¡¿¡ ½ÇÆÐÇÏ¿´½À´Ï´Ù."));
						}
						item->SetCount(item->GetCount() - 1);
					}
					else if (item2->GetAttributeCount() == 5)
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´õ ÀÌ»ó ÀÌ ¾ÆÀÌÅÛÀ» ÀÌ¿ëÇÏ¿© ¼Ó¼ºÀ» Ãß°¡ÇÒ ¼ö ¾ø½À´Ï´Ù."));
					}
					else if (item2->GetAttributeCount() < 4)
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¸ÕÀú Àç°¡ºñ¼­¸¦ ÀÌ¿ëÇÏ¿© ¼Ó¼ºÀ» Ãß°¡½ÃÄÑ ÁÖ¼¼¿ä."));
					}
				}
				else
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´õÀÌ»ó ÀÌ ¾ÆÀÌÅÛÀ» ÀÌ¿ëÇÏ¿© ¼Ó¼ºÀ» Ãß°¡ÇÒ ¼ö ¾ø½À´Ï´Ù."));
				}
			}
			break;

			case USE_CHANGE_BODY_COSTUME_ATTRIBUTE:
			{
				if (item2->GetAttributeSetIndex() == -1)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼ºÀ» º¯°æÇÒ ¼ö ¾ø´Â ¾ÆÀÌÅÛÀÔ´Ï´Ù."));
					return false;
				}

				if (item2->GetAttributeCount() == 0)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("º¯°æÇÒ ¼Ó¼ºÀÌ ¾ø½À´Ï´Ù."));
					return false;
				}

				if (item2->GetType() == ITEM_COSTUME && item2->GetSubType() == COSTUME_BODY)
				{
					item2->ChangeAttribute();
					item->SetCount(item->GetCount() - 1);
				}
				else
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´õÀÌ»ó ÀÌ ¾ÆÀÌÅÛÀ» ÀÌ¿ëÇÏ¿© ¼Ó¼ºÀ» Ãß°¡ÇÒ ¼ö ¾ø½À´Ï´Ù."));
				}
			}
			break;
#endif
#ifdef __SHINING_ITEM_SYSTEM__
			case USE_ADD_ACCE_SHINING_ATTRIBUTE:
			{
				if (item2->GetAttributeSetIndex() == -1)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼ºÀ» º¯°æÇÒ ¼ö ¾ø´Â ¾ÆÀÌÅÛÀÔ´Ï´Ù."));
					return false;
				}

				if (item2->GetType() == ITEM_SHINING && item2->GetSubType() == SHINING_WING)
				{
					if (item2->GetAttributeCount() < 4)
					{
						if (number(1, 100) <= aiItemAttributeAddPercent[item2->GetAttributeCount()])
						{
							item2->AddAttribute();
							ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼º Ãß°¡¿¡ ¼º°øÇÏ¿´½À´Ï´Ù."));
						}
						else
						{
							ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼º Ãß°¡¿¡ ½ÇÆÐÇÏ¿´½À´Ï´Ù."));
						}

						item->SetCount(item->GetCount() - 1);
					}
					else
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´õÀÌ»ó ÀÌ ¾ÆÀÌÅÛÀ» ÀÌ¿ëÇÏ¿© ¼Ó¼ºÀ» Ãß°¡ÇÒ ¼ö ¾ø½À´Ï´Ù."));
					}
				}
				else
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´õÀÌ»ó ÀÌ ¾ÆÀÌÅÛÀ» ÀÌ¿ëÇÏ¿© ¼Ó¼ºÀ» Ãß°¡ÇÒ ¼ö ¾ø½À´Ï´Ù."));
				}
			}
			break;

			case USE_ADD_FIVE_ACCE_SHINING_ATTRIBUTE:
			{
				if (item2->GetAttributeSetIndex() == -1)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼ºÀ» º¯°æÇÒ ¼ö ¾ø´Â ¾ÆÀÌÅÛÀÔ´Ï´Ù."));
					return false;
				}
				if (item2->GetType() == ITEM_SHINING && item2->GetSubType() == SHINING_WING)
				{
					if (item2->GetAttributeCount() == 4)
					{
						if (number(1, 100) <= aiItemAttributeAddPercent[item2->GetAttributeCount()])
						{
							item2->AddAttribute();
							ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼º Ãß°¡¿¡ ¼º°øÇÏ¿´½À´Ï´Ù."));
						}
						else
						{
							ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼º Ãß°¡¿¡ ½ÇÆÐÇÏ¿´½À´Ï´Ù."));
						}
						item->SetCount(item->GetCount() - 1);
					}
					else if (item2->GetAttributeCount() == 5)
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´õ ÀÌ»ó ÀÌ ¾ÆÀÌÅÛÀ» ÀÌ¿ëÇÏ¿© ¼Ó¼ºÀ» Ãß°¡ÇÒ ¼ö ¾ø½À´Ï´Ù."));
					}
					else if (item2->GetAttributeCount() < 4)
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¸ÕÀú Àç°¡ºñ¼­¸¦ ÀÌ¿ëÇÏ¿© ¼Ó¼ºÀ» Ãß°¡½ÃÄÑ ÁÖ¼¼¿ä."));
					}
				}
				else
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´õÀÌ»ó ÀÌ ¾ÆÀÌÅÛÀ» ÀÌ¿ëÇÏ¿© ¼Ó¼ºÀ» Ãß°¡ÇÒ ¼ö ¾ø½À´Ï´Ù."));
				}
			}
			break;

			case USE_CHANGE_ACCE_SHINING_ATTRIBUTE:
			{
				if (item2->GetAttributeSetIndex() == -1)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼ºÀ» º¯°æÇÒ ¼ö ¾ø´Â ¾ÆÀÌÅÛÀÔ´Ï´Ù."));
					return false;
				}

				if (item2->GetAttributeCount() == 0)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("º¯°æÇÒ ¼Ó¼ºÀÌ ¾ø½À´Ï´Ù."));
					return false;
				}

				if (item2->GetType() == ITEM_SHINING && item2->GetSubType() == SHINING_WING)
				{
					item2->ChangeAttribute();
					item->SetCount(item->GetCount() - 1);
				}
				else
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´õÀÌ»ó ÀÌ ¾ÆÀÌÅÛÀ» ÀÌ¿ëÇÏ¿© ¼Ó¼ºÀ» Ãß°¡ÇÒ ¼ö ¾ø½À´Ï´Ù."));
				}
			}
			break;

			case USE_ADD_SOUL_SHINING_ATTRIBUTE:
			{
				if (item2->GetAttributeSetIndex() == -1)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼ºÀ» º¯°æÇÒ ¼ö ¾ø´Â ¾ÆÀÌÅÛÀÔ´Ï´Ù."));
					return false;
				}

				if (item2->GetType() == ITEM_SHINING && item2->GetSubType() == SHINING_SPECIAL3)
				{
					if (item2->GetAttributeCount() < 4)
					{
						if (number(1, 100) <= aiItemAttributeAddPercent[item2->GetAttributeCount()])
						{
							item2->AddAttribute();
							ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼º Ãß°¡¿¡ ¼º°øÇÏ¿´½À´Ï´Ù."));
						}
						else
						{
							ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼º Ãß°¡¿¡ ½ÇÆÐÇÏ¿´½À´Ï´Ù."));
						}

						item->SetCount(item->GetCount() - 1);
					}
					else
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("YOU_CAN_HAVE_ONLY_3_ATTR"));
					}
				}
				else
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´õÀÌ»ó ÀÌ ¾ÆÀÌÅÛÀ» ÀÌ¿ëÇÏ¿© ¼Ó¼ºÀ» Ãß°¡ÇÒ ¼ö ¾ø½À´Ï´Ù."));
				}
			}
			break;

			case USE_ADD_SOUL_SHINING_FIVE_ATTRIBUTE:
			{
				if (item2->GetAttributeSetIndex() == -1)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼ºÀ» º¯°æÇÒ ¼ö ¾ø´Â ¾ÆÀÌÅÛÀÔ´Ï´Ù."));
					return false;
				}
				if (item2->GetType() == ITEM_SHINING && item2->GetSubType() == SHINING_SPECIAL3)
				{
					if (item2->GetAttributeCount() == 4)
					{
						if (number(1, 100) <= aiItemAttributeAddPercent[item2->GetAttributeCount()])
						{
							item2->AddAttribute();
							ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼º Ãß°¡¿¡ ¼º°øÇÏ¿´½À´Ï´Ù."));
						}
						else
						{
							ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼º Ãß°¡¿¡ ½ÇÆÐÇÏ¿´½À´Ï´Ù."));
						}
						item->SetCount(item->GetCount() - 1);
					}
					else if (item2->GetAttributeCount() == 5)
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´õ ÀÌ»ó ÀÌ ¾ÆÀÌÅÛÀ» ÀÌ¿ëÇÏ¿© ¼Ó¼ºÀ» Ãß°¡ÇÒ ¼ö ¾ø½À´Ï´Ù."));
					}
					else if (item2->GetAttributeCount() < 4)
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¸ÕÀú Àç°¡ºñ¼­¸¦ ÀÌ¿ëÇÏ¿© ¼Ó¼ºÀ» Ãß°¡½ÃÄÑ ÁÖ¼¼¿ä."));
					}
				}
				else
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´õÀÌ»ó ÀÌ ¾ÆÀÌÅÛÀ» ÀÌ¿ëÇÏ¿© ¼Ó¼ºÀ» Ãß°¡ÇÒ ¼ö ¾ø½À´Ï´Ù."));
				}
			}
			break;

			case USE_CHANGE_SOUL_SHINING_ATTRIBUTE:
			{
				if (item2->GetAttributeSetIndex() == -1)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼ºÀ» º¯°æÇÒ ¼ö ¾ø´Â ¾ÆÀÌÅÛÀÔ´Ï´Ù."));
					return false;
				}

				if (item2->GetAttributeCount() == 0)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("º¯°æÇÒ ¼Ó¼ºÀÌ ¾ø½À´Ï´Ù."));
					return false;
				}

				if (item2->GetType() == ITEM_SHINING && item2->GetSubType() == SHINING_SPECIAL3)
				{
					item2->ChangeAttribute();
					item->SetCount(item->GetCount() - 1);
				}
				else
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´õÀÌ»ó ÀÌ ¾ÆÀÌÅÛÀ» ÀÌ¿ëÇÏ¿© ¼Ó¼ºÀ» Ãß°¡ÇÒ ¼ö ¾ø½À´Ï´Ù."));
				}
			}
			break;
#endif
#ifdef __CAKRA_ITEM_SYSTEM__
			case USE_ADD_CAKRA_ITEM_ATTRIBUTE:
			{
				if (item2->GetAttributeSetIndex() == -1)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼ºÀ» º¯°æÇÒ ¼ö ¾ø´Â ¾ÆÀÌÅÛÀÔ´Ï´Ù."));
					return false;
				}

				if (item2->GetType() == ITEM_CAKRA)
				{
					if (item2->GetAttributeCount() < 4)
					{
						if (number(1, 100) <= aiItemAttributeAddPercent[item2->GetAttributeCount()])
						{
							item2->AddAttribute();
							ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼º Ãß°¡¿¡ ¼º°øÇÏ¿´½À´Ï´Ù."));
						}
						else
						{
							ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼º Ãß°¡¿¡ ½ÇÆÐÇÏ¿´½À´Ï´Ù."));
						}

						item->SetCount(item->GetCount() - 1);
					}
					else
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´õÀÌ»ó ÀÌ ¾ÆÀÌÅÛÀ» ÀÌ¿ëÇÏ¿© ¼Ó¼ºÀ» Ãß°¡ÇÒ ¼ö ¾ø½À´Ï´Ù."));
					}
				}
				else
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´õÀÌ»ó ÀÌ ¾ÆÀÌÅÛÀ» ÀÌ¿ëÇÏ¿© ¼Ó¼ºÀ» Ãß°¡ÇÒ ¼ö ¾ø½À´Ï´Ù."));
				}
			}
			break;

			case USE_ADD_FIVE_CAKRA_ITEM_ATTRIBUTE:
			{
				if (item2->GetAttributeSetIndex() == -1)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼ºÀ» º¯°æÇÒ ¼ö ¾ø´Â ¾ÆÀÌÅÛÀÔ´Ï´Ù."));
					return false;
				}
				if (item2->GetType() == ITEM_CAKRA)
				{
					if (item2->GetAttributeCount() == 4)
					{
						if (number(1, 100) <= aiItemAttributeAddPercent[item2->GetAttributeCount()])
						{
							item2->AddAttribute();
							ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼º Ãß°¡¿¡ ¼º°øÇÏ¿´½À´Ï´Ù."));
						}
						else
						{
							ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼º Ãß°¡¿¡ ½ÇÆÐÇÏ¿´½À´Ï´Ù."));
						}
						item->SetCount(item->GetCount() - 1);
					}
					else if (item2->GetAttributeCount() == 5)
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´õ ÀÌ»ó ÀÌ ¾ÆÀÌÅÛÀ» ÀÌ¿ëÇÏ¿© ¼Ó¼ºÀ» Ãß°¡ÇÒ ¼ö ¾ø½À´Ï´Ù."));
					}
					else if (item2->GetAttributeCount() < 4)
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¸ÕÀú Àç°¡ºñ¼­¸¦ ÀÌ¿ëÇÏ¿© ¼Ó¼ºÀ» Ãß°¡½ÃÄÑ ÁÖ¼¼¿ä."));
					}
				}
				else
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´õÀÌ»ó ÀÌ ¾ÆÀÌÅÛÀ» ÀÌ¿ëÇÏ¿© ¼Ó¼ºÀ» Ãß°¡ÇÒ ¼ö ¾ø½À´Ï´Ù."));
				}
			}
			break;

			case USE_CHANGE_CAKRA_ITEM_ATTRIBUTE:
			{
				if (item2->GetAttributeSetIndex() == -1)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼ºÀ» º¯°æÇÒ ¼ö ¾ø´Â ¾ÆÀÌÅÛÀÔ´Ï´Ù."));
					return false;
				}

				if (item2->GetAttributeCount() == 0)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("º¯°æÇÒ ¼Ó¼ºÀÌ ¾ø½À´Ï´Ù."));
					return false;
				}

				if (item2->GetType() == ITEM_CAKRA)
				{
					item2->ChangeAttribute();
					item->SetCount(item->GetCount() - 1);
				}
				else
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´õÀÌ»ó ÀÌ ¾ÆÀÌÅÛÀ» ÀÌ¿ëÇÏ¿© ¼Ó¼ºÀ» Ãß°¡ÇÒ ¼ö ¾ø½À´Ï´Ù."));
				}
			}
			break;
#endif
#ifdef __PET_SYSTEM_PROTO__
			case USE_ADD_PET_ITEM_ATTRIBUTE:
			{
				if (item2->GetAttributeSetIndex() == -1)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼ºÀ» º¯°æÇÒ ¼ö ¾ø´Â ¾ÆÀÌÅÛÀÔ´Ï´Ù."));
					return false;
				}

				if (item2->GetType() == ITEM_UNIQUE && item2->GetSubType() == USE_PET)
				{
					if (item2->GetAttributeCount() < 4)
					{
						if (number(1, 100) <= aiItemAttributeAddPercent[item2->GetAttributeCount()])
						{
							item2->AddAttribute();
							ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼º Ãß°¡¿¡ ¼º°øÇÏ¿´½À´Ï´Ù."));
						}
						else
						{
							ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼º Ãß°¡¿¡ ½ÇÆÐÇÏ¿´½À´Ï´Ù."));
						}

						item->SetCount(item->GetCount() - 1);
					}
					else
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´õÀÌ»ó ÀÌ ¾ÆÀÌÅÛÀ» ÀÌ¿ëÇÏ¿© ¼Ó¼ºÀ» Ãß°¡ÇÒ ¼ö ¾ø½À´Ï´Ù."));
					}
				}
				else
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´õÀÌ»ó ÀÌ ¾ÆÀÌÅÛÀ» ÀÌ¿ëÇÏ¿© ¼Ó¼ºÀ» Ãß°¡ÇÒ ¼ö ¾ø½À´Ï´Ù."));
				}
			}
			break;

			case USE_ADD_FIVE_PET_ITEM_ATTRIBUTE:
			{
				if (item2->GetAttributeSetIndex() == -1)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼ºÀ» º¯°æÇÒ ¼ö ¾ø´Â ¾ÆÀÌÅÛÀÔ´Ï´Ù."));
					return false;
				}
				if (item2->GetType() == ITEM_UNIQUE && item2->GetSubType() == USE_PET)
				{
					if (item2->GetAttributeCount() == 4)
					{
						if (number(1, 100) <= aiItemAttributeAddPercent[item2->GetAttributeCount()])
						{
							item2->AddAttribute();
							ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼º Ãß°¡¿¡ ¼º°øÇÏ¿´½À´Ï´Ù."));
						}
						else
						{
							ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼º Ãß°¡¿¡ ½ÇÆÐÇÏ¿´½À´Ï´Ù."));
						}
						item->SetCount(item->GetCount() - 1);
					}
					else if (item2->GetAttributeCount() == 5)
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´õ ÀÌ»ó ÀÌ ¾ÆÀÌÅÛÀ» ÀÌ¿ëÇÏ¿© ¼Ó¼ºÀ» Ãß°¡ÇÒ ¼ö ¾ø½À´Ï´Ù."));
					}
					else if (item2->GetAttributeCount() < 4)
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¸ÕÀú Àç°¡ºñ¼­¸¦ ÀÌ¿ëÇÏ¿© ¼Ó¼ºÀ» Ãß°¡½ÃÄÑ ÁÖ¼¼¿ä."));
					}
				}
				else
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´õÀÌ»ó ÀÌ ¾ÆÀÌÅÛÀ» ÀÌ¿ëÇÏ¿© ¼Ó¼ºÀ» Ãß°¡ÇÒ ¼ö ¾ø½À´Ï´Ù."));
				}
			}
			break;

			case USE_CHANGE_PET_ITEM_ATTRIBUTE:
			{
				if (item2->GetAttributeSetIndex() == -1)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼ºÀ» º¯°æÇÒ ¼ö ¾ø´Â ¾ÆÀÌÅÛÀÔ´Ï´Ù."));
					return false;
				}

				if (item2->GetAttributeCount() == 0)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("º¯°æÇÒ ¼Ó¼ºÀÌ ¾ø½À´Ï´Ù."));
					return false;
				}

				if (item2->GetType() == ITEM_UNIQUE && item2->GetSubType() == USE_PET)
				{
					item2->ChangeAttribute();
					item->SetCount(item->GetCount() - 1);
				}
				else
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´õÀÌ»ó ÀÌ ¾ÆÀÌÅÛÀ» ÀÌ¿ëÇÏ¿© ¼Ó¼ºÀ» Ãß°¡ÇÒ ¼ö ¾ø½À´Ï´Ù."));
				}
			}
			break;
#endif
#ifdef __SEBNEM_ITEM_SYSTEM__
			case USE_ADD_SEBNEM_ITEM_ATTRIBUTE:
			{
				if (item2->GetAttributeSetIndex() == -1)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼ºÀ» º¯°æÇÒ ¼ö ¾ø´Â ¾ÆÀÌÅÛÀÔ´Ï´Ù."));
					return false;
				}

				if (item2->GetType() == ITEM_SEBNEM)
				{
					if (item2->GetAttributeCount() < 4)
					{
						if (number(1, 100) <= aiItemAttributeAddPercent[item2->GetAttributeCount()])
						{
							item2->AddAttribute();
							ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼º Ãß°¡¿¡ ¼º°øÇÏ¿´½À´Ï´Ù."));
						}
						else
						{
							ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼º Ãß°¡¿¡ ½ÇÆÐÇÏ¿´½À´Ï´Ù."));
						}

						item->SetCount(item->GetCount() - 1);
					}
					else
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´õÀÌ»ó ÀÌ ¾ÆÀÌÅÛÀ» ÀÌ¿ëÇÏ¿© ¼Ó¼ºÀ» Ãß°¡ÇÒ ¼ö ¾ø½À´Ï´Ù."));
					}
				}
				else
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´õÀÌ»ó ÀÌ ¾ÆÀÌÅÛÀ» ÀÌ¿ëÇÏ¿© ¼Ó¼ºÀ» Ãß°¡ÇÒ ¼ö ¾ø½À´Ï´Ù."));
				}
			}
			break;

			case USE_ADD_FIVE_SEBNEM_ITEM_ATTRIBUTE:
			{
				if (item2->GetAttributeSetIndex() == -1)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼ºÀ» º¯°æÇÒ ¼ö ¾ø´Â ¾ÆÀÌÅÛÀÔ´Ï´Ù."));
					return false;
				}
				if (item2->GetType() == ITEM_SEBNEM)
				{
					if (item2->GetAttributeCount() == 4)
					{
						if (number(1, 100) <= aiItemAttributeAddPercent[item2->GetAttributeCount()])
						{
							item2->AddAttribute();
							ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼º Ãß°¡¿¡ ¼º°øÇÏ¿´½À´Ï´Ù."));
						}
						else
						{
							ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼º Ãß°¡¿¡ ½ÇÆÐÇÏ¿´½À´Ï´Ù."));
						}
						item->SetCount(item->GetCount() - 1);
					}
					else if (item2->GetAttributeCount() == 5)
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´õ ÀÌ»ó ÀÌ ¾ÆÀÌÅÛÀ» ÀÌ¿ëÇÏ¿© ¼Ó¼ºÀ» Ãß°¡ÇÒ ¼ö ¾ø½À´Ï´Ù."));
					}
					else if (item2->GetAttributeCount() < 4)
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¸ÕÀú Àç°¡ºñ¼­¸¦ ÀÌ¿ëÇÏ¿© ¼Ó¼ºÀ» Ãß°¡½ÃÄÑ ÁÖ¼¼¿ä."));
					}
				}
				else
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´õÀÌ»ó ÀÌ ¾ÆÀÌÅÛÀ» ÀÌ¿ëÇÏ¿© ¼Ó¼ºÀ» Ãß°¡ÇÒ ¼ö ¾ø½À´Ï´Ù."));
				}
			}
			break;

			case USE_CHANGE_SEBNEM_ITEM_ATTRIBUTE:
			{
				if (item2->GetAttributeSetIndex() == -1)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼ºÀ» º¯°æÇÒ ¼ö ¾ø´Â ¾ÆÀÌÅÛÀÔ´Ï´Ù."));
					return false;
				}

				if (item2->GetAttributeCount() == 0)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("º¯°æÇÒ ¼Ó¼ºÀÌ ¾ø½À´Ï´Ù."));
					return false;
				}

				if (item2->GetType() == ITEM_SEBNEM)
				{
					item2->ChangeAttribute();
					item->SetCount(item->GetCount() - 1);
				}
				else
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´õÀÌ»ó ÀÌ ¾ÆÀÌÅÛÀ» ÀÌ¿ëÇÏ¿© ¼Ó¼ºÀ» Ãß°¡ÇÒ ¼ö ¾ø½À´Ï´Ù."));
				}
			}
			break;
#endif
			case USE_ADD_ACCESSORY_SOCKET:
			{
				char buf[21];
				snprintf(buf, sizeof(buf), "%u", item2->GetID());

#ifdef __BEGINNER_ITEM__
				if (item2->IsBasicItem())
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ITEM_IS_BASIC_CANNOT_DO"));
					return false;
				}
#endif
				if (item2->IsAccessoryForSocket())
				{
					if (item2->GetAccessorySocketMaxGrade() < ITEM_ACCESSORY_SOCKET_MAX_NUM)
					{
#ifdef __MARTY_ADDSTONE_FAILURE__
						if (number(1, 100) <= 75)
#else
						if (1)
#endif
						{
							item2->SetAccessorySocketMaxGrade(item2->GetAccessorySocketMaxGrade() + 1);
							ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼ÒÄÏÀÌ ¼º°øÀûÀ¸·Î Ãß°¡µÇ¾ú½À´Ï´Ù."));
						}
						else
						{
							ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼ÒÄÏ Ãß°¡¿¡ ½ÇÆÐÇÏ¿´½À´Ï´Ù."));
						}

						item->SetCount(item->GetCount() - 1);
					}
					else
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ ¾×¼¼¼­¸®¿¡´Â ´õÀÌ»ó ¼ÒÄÏÀ» Ãß°¡ÇÒ °ø°£ÀÌ ¾ø½À´Ï´Ù."));
					}
				}
				else
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ ¾ÆÀÌÅÛÀ¸·Î ¼ÒÄÏÀ» Ãß°¡ÇÒ ¼ö ¾ø´Â ¾ÆÀÌÅÛÀÔ´Ï´Ù."));
				}
			}
			break;

			case USE_PUT_INTO_BELT_SOCKET:
			case USE_PUT_INTO_ACCESSORY_SOCKET:
				if (item2->IsAccessoryForSocket() && (item->CanPutInto(item2) || item->CanPutIntoPerma(item2)))
				{

#ifdef __BEGINNER_ITEM__
					if (item2->IsBasicItem())
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ITEM_IS_BASIC_CANNOT_DO"));
						return false;
					}
#endif
					if (item2->GetAccessorySocketGrade() < item2->GetAccessorySocketMaxGrade())
					{
#ifdef __PERMA_ACCESSORY__
						if (item2->GetAccessorySocketGrade() && item->GetValue(0) != 99 && item2->GetSocket(3) == 99)
							return false;

						if (item2->GetAccessorySocketGrade() && item->GetValue(0) == 99 && item2->GetSocket(3) != 99)
							return false;

						if (item->GetValue(0) == 99)
						{
							if (item2->GetAccessorySocketMaxGrade() && !item2->GetAccessorySocketGrade())
								item2->SetSocket(3, 99);
							item2->SetAccessorySocketGrade(item2->GetAccessorySocketGrade() + 1);
							ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀåÂø¿¡ ¼º°øÇÏ¿´½À´Ï´Ù."));
						}
						else if (number(1, 100) <= aiAccessorySocketPutPct[item2->GetAccessorySocketGrade()])
						{
							item2->SetAccessorySocketGrade(item2->GetAccessorySocketGrade() + 1);
							ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀåÂø¿¡ ¼º°øÇÏ¿´½À´Ï´Ù."));
						}
						else
						{
							ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀåÂø¿¡ ½ÇÆÐÇÏ¿´½À´Ï´Ù."));
						}

						item->SetCount(item->GetCount() - 1);
#else
						if (number(1, 100) <= aiAccessorySocketPutPct[item2->GetAccessorySocketGrade()])
						{
							item2->SetAccessorySocketGrade(item2->GetAccessorySocketGrade() + 1);
							ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀåÂø¿¡ ¼º°øÇÏ¿´½À´Ï´Ù."));
						}
						else
						{
							ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀåÂø¿¡ ½ÇÆÐÇÏ¿´½À´Ï´Ù."));
						}

						item->SetCount(item->GetCount() - 1);
#endif
					}
					else
					{
						if (item2->GetAccessorySocketMaxGrade() == 0)
							ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¸ÕÀú ´ÙÀÌ¾Æ¸óµå·Î ¾Ç¼¼¼­¸®¿¡ ¼ÒÄÏÀ» Ãß°¡ÇØ¾ßÇÕ´Ï´Ù."));
						else if (item2->GetAccessorySocketMaxGrade() < ITEM_ACCESSORY_SOCKET_MAX_NUM)
						{
							ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ ¾×¼¼¼­¸®¿¡´Â ´õÀÌ»ó ÀåÂøÇÒ ¼ÒÄÏÀÌ ¾ø½À´Ï´Ù."));
							ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´ÙÀÌ¾Æ¸óµå·Î ¼ÒÄÏÀ» Ãß°¡ÇØ¾ßÇÕ´Ï´Ù."));
						}
						else
							ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ ¾×¼¼¼­¸®¿¡´Â ´õÀÌ»ó º¸¼®À» ÀåÂøÇÒ ¼ö ¾ø½À´Ï´Ù."));
					}
				}
				else
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ ¾ÆÀÌÅÛÀ» ÀåÂøÇÒ ¼ö ¾ø½À´Ï´Ù."));
				}
				break;
			}
			if (item2->IsEquipped())
			{
				BuffOnAttr_AddBuffsFromItem(item2);
			}
		}
		break;
		//  END_OF_ACCESSORY_REFINE & END_OF_ADD_ATTRIBUTES & END_OF_CHANGE_ATTRIBUTES

		case USE_BAIT:
		{
			if (m_pkFishingEvent)
			{
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("³¬½Ã Áß¿¡ ¹Ì³¢¸¦ °¥¾Æ³¢¿ï ¼ö ¾ø½À´Ï´Ù."));
				return false;
			}

			LPITEM weapon = GetWear(WEAR_WEAPON);

			if (!weapon || weapon->GetType() != ITEM_ROD)
				return false;

			if (weapon->GetSocket(2))
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ¹Ì ²ÈÇôÀÖ´ø ¹Ì³¢¸¦ »©°í %s¸¦ ³¢¿ó´Ï´Ù."), item->GetName());
			else
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("³¬½Ã´ë¿¡ %s¸¦ ¹Ì³¢·Î ³¢¿ó´Ï´Ù."), item->GetName());

			weapon->SetSocket(2, item->GetValue(0));
			item->SetCount(item->GetCount() - 1);
		}
		break;

		case USE_MOVE:
		case USE_TREASURE_BOX:
		case USE_MONEYBAG:
			break;

		case USE_AFFECT:
		{
			if (FindAffect(item->GetValue(0), aApplyInfo[item->GetValue(1)].bPointType))
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ¹Ì È¿°ú°¡ °É·Á ÀÖ½À´Ï´Ù."));
			else
			{
				AddAffect(item->GetValue(0), aApplyInfo[item->GetValue(1)].bPointType, item->GetValue(2), 0, item->GetValue(3), 0, false);
				item->SetCount(item->GetCount() - 1);
			}
		}
		break;

		case USE_CREATE_STONE:
			AutoGiveItem(number(28000, 28013));
			item->SetCount(item->GetCount() - 1);
			break;

		case USE_RECIPE:
		{
			LPITEM pSource1 = FindSpecifyItem(item->GetValue(1));
			DWORD dwSourceCount1 = item->GetValue(2);

			LPITEM pSource2 = FindSpecifyItem(item->GetValue(3));
			DWORD dwSourceCount2 = item->GetValue(4);

			if (dwSourceCount1 != 0)
			{
				if (pSource1 == NULL)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¹°¾à Á¶ÇÕÀ» À§ÇÑ Àç·á°¡ ºÎÁ·ÇÕ´Ï´Ù."));
					return false;
				}
			}

			if (dwSourceCount2 != 0)
			{
				if (pSource2 == NULL)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¹°¾à Á¶ÇÕÀ» À§ÇÑ Àç·á°¡ ºÎÁ·ÇÕ´Ï´Ù."));
					return false;
				}
			}

			if (pSource1 != NULL)
			{
				if (pSource1->GetCount() < dwSourceCount1)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Àç·á(%s)°¡ ºÎÁ·ÇÕ´Ï´Ù."), pSource1->GetName());
					return false;
				}

				pSource1->SetCount(pSource1->GetCount() - dwSourceCount1);
			}

			if (pSource2 != NULL)
			{
				if (pSource2->GetCount() < dwSourceCount2)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Àç·á(%s)°¡ ºÎÁ·ÇÕ´Ï´Ù."), pSource2->GetName());
					return false;
				}

				pSource2->SetCount(pSource2->GetCount() - dwSourceCount2);
			}

			LPITEM pBottle = FindSpecifyItem(50901);

			if (!pBottle || pBottle->GetCount() < 1)
			{
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ºó º´ÀÌ ¸ðÀÚ¸¨´Ï´Ù."));
				return false;
			}

			pBottle->SetCount(pBottle->GetCount() - 1);

			if (number(1, 100) > item->GetValue(5))
			{
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¹°¾à Á¦Á¶¿¡ ½ÇÆÐÇß½À´Ï´Ù."));
				return false;
			}

			AutoGiveItem(item->GetValue(0));
		}
		break;
		}
	}
	break;

	case ITEM_METIN:
	{
		LPITEM item2;

		if (!IsValidItemPosition(DestCell) || !(item2 = GetItem(DestCell)))
			return false;

		if (item2->IsExchanging() || item2->IsEquipped()) // @duzenleme inputlara alinacak bir yer daha.
			return false;

#ifdef __BEGINNER_ITEM__
		if (item2->IsBasicItem())
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ITEM_IS_BASIC_CANNOT_DO"));
			return false;
		}
#endif

		if (item2->GetType() == ITEM_PICK) return false;
		if (item2->GetType() == ITEM_ROD) return false;

		int i;

		for (i = 0; i < ITEM_SOCKET_MAX_NUM; ++i)
		{
			DWORD dwVnum;

			if ((dwVnum = item2->GetSocket(i)) <= 2)
				continue;

			TItemTable* p = ITEM_MANAGER::instance().GetTable(dwVnum);

			if (!p)
				continue;

			if (item->GetValue(5) == p->alValues[5])
			{
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("°°Àº Á¾·ùÀÇ ¸ÞÆ¾¼®Àº ¿©·¯°³ ºÎÂøÇÒ ¼ö ¾ø½À´Ï´Ù."));
				return false;
			}
		}

		if (item2->GetType() == ITEM_ARMOR)
		{
			if (!IS_SET(item->GetWearFlag(), WEARABLE_BODY) || !IS_SET(item2->GetWearFlag(), WEARABLE_BODY))
			{
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ ¸ÞÆ¾¼®Àº Àåºñ¿¡ ºÎÂøÇÒ ¼ö ¾ø½À´Ï´Ù."));
				return false;
			}
		}
		else if (item2->GetType() == ITEM_WEAPON)
		{
			if (!IS_SET(item->GetWearFlag(), WEARABLE_WEAPON))
			{
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ ¸ÞÆ¾¼®Àº ¹«±â¿¡ ºÎÂøÇÒ ¼ö ¾ø½À´Ï´Ù."));
				return false;
			}
		}
		else
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ºÎÂøÇÒ ¼ö ÀÖ´Â ½½·ÔÀÌ ¾ø½À´Ï´Ù."));
			return false;
		}

		for (i = 0; i < ITEM_SOCKET_MAX_NUM; ++i)
			if (item2->GetSocket(i) >= 1 && item2->GetSocket(i) <= 2 && item2->GetSocket(i) >= item->GetValue(2))
			{
//#ifdef __MARTY_ADDSTONE_FAILURE__
//				if (number(1, 100) <= 100)
//#else
				if (1)
//#endif
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¸ÞÆ¾¼® ºÎÂø¿¡ ¼º°øÇÏ¿´½À´Ï´Ù."));
					item2->SetSocket(i, item->GetVnum());
				}
				else
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¸ÞÆ¾¼® ºÎÂø¿¡ ½ÇÆÐÇÏ¿´½À´Ï´Ù."));
					item2->SetSocket(i, ITEM_BROKEN_METIN_VNUM);
				}

				item->SetCount(item->GetCount() - 1);
				break;
			}

		if (i == ITEM_SOCKET_MAX_NUM)
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ºÎÂøÇÒ ¼ö ÀÖ´Â ½½·ÔÀÌ ¾ø½À´Ï´Ù."));
	}
	break;

	case ITEM_AUTOUSE:
	case ITEM_MATERIAL:
	case ITEM_SPECIAL:
	case ITEM_TOOL:
	case ITEM_LOTTERY:
		break;

	case ITEM_TOTEM:
	{
		if (!item->IsEquipped())
			EquipItem(item);
	}
	break;

	case ITEM_BLEND:
	{
		sys_log(0, "ITEM_BLEND!!");

		if (Blend_Item_find(item->GetVnum()))
		{
			int		affect_type = AFFECT_BLEND;

			// @duzenleme sebnemin apply'si yanlis ise sebnemi kullandirtmiyor.
			if (item->GetSocket(0) >= _countof(aApplyInfo))
			{
				sys_err("INVALID BLEND ITEM(id : %d, vnum : %d). APPLY TYPE IS %d.", item->GetID(), item->GetVnum(), item->GetSocket(0));
				return false;
			}

			int		apply_type = aApplyInfo[item->GetSocket(0)].bPointType;
			int		apply_value = item->GetSocket(1);
			int		apply_duration = item->GetSocket(2);

#ifdef __POTION_AS_REWORK__
			switch (apply_type)
			{
				case POINT_CRITICAL_PCT: affect_type = RED_BLEND_POTION; break;
				case POINT_PENETRATE_PCT: affect_type = ORANGE_BLEND_POTION; break;
				case POINT_ATT_SPEED: affect_type = YELLOW_BLEND_POTION; break;
				case POINT_RESIST_MAGIC: affect_type = GREEN_BLEND_POTION; break;
				case POINT_ATT_GRADE_BONUS: affect_type = BLUE_BLEND_POTION; break;
				case POINT_DEF_GRADE_BONUS: affect_type = WHITE_BLEND_POTION; break;
				case POINT_ATTBONUS_STONE: affect_type = JADE_BLEND_POTION; break;
				case POINT_ATTBONUS_MONSTER: affect_type = DARK_BLEND_POTION; break;
				case POINT_ATTBONUS_BOSS: affect_type = SAFE_BLEND_POTION; break;
				case POINT_ATTBONUS_HUMAN: affect_type = DEVIL_BLEND_POTION; break;
				case POINT_MAX_HP: affect_type = HEAL_BLEND_POTION; break;
				case POINT_NORMAL_HIT_DAMAGE_BONUS: affect_type = NORMAL_HIT_BLEND_POTION; break;
				case POINT_ATTBONUS_UNDEAD: affect_type = UNDEAD_BLEND_POTION; break;
				case POINT_ATTBONUS_DEVIL: affect_type = DEVIL_ATT_BLEND_POTION; break;
				case POINT_ATTBONUS_ANIMAL: affect_type = ANIMAL_BLEND_POTION; break;
				case POINT_ATTBONUS_MILGYO: affect_type = MILGYO_BLEND_POTION; break;
			}
#endif

#ifdef __BLEND_ITEM_REWORK__
			if (item->GetType() == ITEM_BLEND && item->GetSubType() == INFINITY_BLEND)
			{
				CAffect* pAffect = FindAffect(affect_type, apply_type);
				if (pAffect == NULL)
				{
					AddAffect(affect_type, apply_type, apply_value, item->GetID(), INFINITE_AFFECT_DURATION, 0, false);
					item->SetSocket(2, 1);
					item->Lock(true);
				}
				else
				{
					if (item->GetID() == pAffect->dwFlag)
					{
						RemoveAffect(pAffect);
						item->Lock(false);
						item->SetSocket(2, 0);
					}
					else
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ¹Ì È¿°ú°¡ °É·Á ÀÖ½À´Ï´Ù."));
				}
			}
			else
			{
				if (FindAffect(affect_type, apply_type))
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ¹Ì È¿°ú°¡ °É·Á ÀÖ½À´Ï´Ù."));
				}
				else
				{
					if (FindAffect(AFFECT_EXP_BONUS_EURO_FREE, POINT_RESIST_MAGIC))
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ¹Ì È¿°ú°¡ °É·Á ÀÖ½À´Ï´Ù."));
					}
					else
					{
						AddAffect(affect_type, apply_type, apply_value, 0, apply_duration, 0, false);
						item->SetCount(item->GetCount() - 1);
					}
				}
			}
#else
			if (FindAffect(affect_type, apply_type))
			{
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ¹Ì È¿°ú°¡ °É·Á ÀÖ½À´Ï´Ù."));
			}
			else
			{
				if (FindAffect(AFFECT_EXP_BONUS_EURO_FREE, POINT_RESIST_MAGIC))
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ¹Ì È¿°ú°¡ °É·Á ÀÖ½À´Ï´Ù."));
				}
				else
				{
					AddAffect(affect_type, apply_type, apply_value, 0, apply_duration, 0, false);
					item->SetCount(item->GetCount() - 1);
				}
			}
#endif
		}
	}
	break;
	case ITEM_EXTRACT:
	{
		LPITEM pDestItem = GetItem(DestCell);
		if (NULL == pDestItem)
		{
			return false;
		}
		switch (item->GetSubType())
		{
		case EXTRACT_DRAGON_SOUL:
			if (pDestItem->IsDragonSoul())
			{
				return DSManager::instance().PullOut(this, NPOS, pDestItem, item);
			}
			return false;
		case EXTRACT_DRAGON_HEART:
#ifdef __DSS_PUT_ATTR_ITEM__
			if (item->GetVnum() == 80039 || item->GetVnum() == 80040)
			{
				if (pDestItem->IsDragonSoul())
				{
					if (DSManager::instance().IsActiveDragonSoul(pDestItem) == true)
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("SIMYA_GIYILIYOR"));
						return false;
					}
					if (item->GetVnum() == 80039 && (((pDestItem->GetVnum() / 1000) % 10) > 6))
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("BU_NESNE_ILE_SADECE_DESTANSI"));
						return false;
					}
					if (item->GetVnum() == 80040 && (((pDestItem->GetVnum() / 1000) % 10) != 7))
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("BU_NESNE_ILE_SADECE_TANRISAL"));
						return false;
					}
					pDestItem->SetForceAttribute(0, 0, 0);
					pDestItem->SetForceAttribute(1, 0, 0);
					pDestItem->SetForceAttribute(2, 0, 0);
					pDestItem->SetForceAttribute(3, 0, 0);
					pDestItem->SetForceAttribute(4, 0, 0);
					pDestItem->SetForceAttribute(5, 0, 0);
					pDestItem->SetForceAttribute(6, 0, 0);

					bool ret = DSManager::instance().PutAttributes(pDestItem);
					if (ret == true)
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("SIMYA_DEGISTIRME_BASARILI!"));
						item->SetCount(item->GetCount() - 1);
					}
					else
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("SIMYA_DEGISTIRME_BASARISIZ!"));
					}
				}
			}
			else
			{
				if (pDestItem->IsDragonSoul())
				{
					return DSManager::instance().ExtractDragonHeart(this, pDestItem, item);
				}
			}
			return false;
#else
			if (pDestItem->IsDragonSoul())
			{
				return DSManager::instance().ExtractDragonHeart(this, pDestItem, item);
			}
			return false;
#endif
		default:
			return false;
		}
	}
	break;

#ifdef __ITEM_GACHA__
	case ITEM_GACHA:
	{
		// kullanilacagi zaman bastan yazilacak
		{
			ChatPacket(CHAT_TYPE_TALKING, LC_TEXT("¾Æ¹«°Íµµ ¾òÀ» ¼ö ¾ø¾ú½À´Ï´Ù."));
			return false;
		}
	}
	break;
#endif

	case ITEM_NONE:
		sys_err("Item type NONE %s", item->GetName());
		break;

	default:
		sys_log(0, "UseItemEx: Unknown type %s %d", item->GetName(), item->GetType());
		return false;
	}

	return true;
}

int g_nPortalLimitTime = 10;

#ifdef __USE_ITEM_COUNT__
bool CHARACTER::UseItem(TItemPos Cell, TItemPos DestCell, DWORD count)
#else // __USE_ITEM_COUNT__
bool CHARACTER::UseItem(TItemPos Cell, TItemPos DestCell)
#endif // __USE_ITEM_COUNT__
{
	//WORD wCell = Cell.cell;
	//BYTE window_type = Cell.window_type;
	//WORD wDestCell = DestCell.cell;
	//BYTE bDestInven = DestCell.window_type;
	LPITEM item;

	if (!CanHandleItem())
		return false;

	if (!IsValidItemPosition(Cell) || !(item = GetItem(Cell)))
		return false;

	sys_log(0, "%s: USE_ITEM %s %u", GetName(), item->GetName(), count);

#ifdef __USE_ITEM_COUNT__
	if (item->GetCount() < count)
	{
		sys_err("%s: USE_ITEM_COUNT ERR %s", GetName(), item->GetName());
		return false;
	}
#endif // __USE_ITEM_COUNT__

	if (item->IsExchanging())
		return false;

#ifdef __SWITCHBOT__
	if (Cell.IsSwitchbotPosition())
	{
		CSwitchbot* pkSwitchbot = CSwitchbotManager::Instance().FindSwitchbot(GetPlayerID());
		if (pkSwitchbot && pkSwitchbot->IsActive(Cell.cell))
		{
			return false;
		}

		int iEmptyCell = GetEmptyInventory(item->GetSize());

		if (iEmptyCell == -1)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Cannot remove item from switchbot. Inventory is full."));
			return false;
		}

		MoveItem(Cell, TItemPos(INVENTORY, iEmptyCell), item->GetCount());
		return true;
	}
#endif

	if (!item->CanUsedBy(this))
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("±ºÁ÷ÀÌ ¸ÂÁö¾Ê¾Æ ÀÌ ¾ÆÀÌÅÛÀ» »ç¿ëÇÒ ¼ö ¾ø½À´Ï´Ù."));
		return false;
	}

	if (IsStun())
		return false;

	if (false == FN_check_item_sex(this, item))
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼ºº°ÀÌ ¸ÂÁö¾Ê¾Æ ÀÌ ¾ÆÀÌÅÛÀ» »ç¿ëÇÒ ¼ö ¾ø½À´Ï´Ù."));
		return false;
	}

	// @duzenleme
	// gorev penceresi acik iken bu islemin yapilmamasini saglar fakat bu kontroller detaylandirilabilir.
	if (quest::CQuestManager::instance().GetPCForce(GetPlayerID())->IsRunning() == true)
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You cannot use this item if you're using quests"));
		return false;
	}

	if (IS_SET(item->GetFlag(), ITEM_FLAG_LOG_SPECIAL))
	{
#ifdef __USE_ITEM_COUNT__
		bool success = UseItemEx(item, DestCell, count);
#else // __USE_ITEM_COUNT__
		bool success = UseItemEx(item, DestCell);
#endif // __USE_ITEM_COUNT__
		return success;
	}
	else
#ifdef __USE_ITEM_COUNT__
		return UseItemEx(item, DestCell, count);
#else // __USE_ITEM_COUNT__
		return UseItemEx(item, DestCell);
#endif // __USE_ITEM_COUNT__
}

#ifdef __DROP_DIALOG__
bool CHARACTER::DeleteItem(TItemPos Cell)
{
	LPITEM item = NULL;

	if (!CanHandleItem())
		return false;

	if (IsDead() || IsStun())
		return false;

	if (!IsValidItemPosition(Cell) || !(item = GetItem(Cell)))
		return false;

	if (true == item->isLocked() || item->IsExchanging() || item->IsEquipped())
		return false;

	if (quest::CQuestManager::instance().GetPCForce(GetPlayerID())->IsRunning() == true)
		return false;

	if (!CanAct(true, true, VAR_TIME_NONE))
		return false;

#ifdef __BLEND_ITEM_REWORK__
	if (item->GetType() == ITEM_BLEND && item->GetSubType() == INFINITY_BLEND)
	{
		int affect_type = AFFECT_BLEND;
		int apply_type = aApplyInfo[item->GetSocket(0)].bPointType;
		CAffect* pAffect = FindAffect(affect_type, apply_type);
		bool bIsActive = item->GetSocket(2) > 0;

		if (pAffect)
		{
			if (bIsActive)
			{
				RemoveAffect(pAffect);
				item->Lock(false);
				item->SetSocket(2, 0);
			}
		}
	}
#endif

#ifdef __BATTLE_PASS_SYSTEM__
	BYTE bBattlePassId = GetBattlePassId();
	if (bBattlePassId)
	{
		DWORD dwItemVnum, dwDestItemCount;
		if(CBattlePass::instance().BattlePassMissionGetInfo(bBattlePassId, DESTROY_ITEM, &dwItemVnum, &dwDestItemCount))
		{
			if(dwItemVnum == item->GetVnum() && GetMissionProgress(DESTROY_ITEM, bBattlePassId) < dwDestItemCount)
				UpdateMissionProgress(DESTROY_ITEM, bBattlePassId, item->GetCount(), dwDestItemCount);
		}


#ifdef ENABLE_BATTLE_PASS_EX
		DWORD nextMissionId = GetNextMissionByType(FISH_DESTROY1, FISH_DESTROY50, bBattlePassId);
		if (nextMissionId != 0)
		{
			DWORD dwCount, dwVnum;
			if (CBattlePass::instance().BattlePassMissionGetInfo(bBattlePassId, nextMissionId, &dwVnum, &dwCount))
			{
				if (item->GetVnum() == 27805 && GetMissionProgress(nextMissionId, bBattlePassId) < dwCount)
					UpdateMissionProgress(nextMissionId, bBattlePassId, item->GetCount(), dwCount);
			}
		}
		DWORD nextMissionId2 = GetNextMissionByType(KAFA_DESTROY1, KAFA_DESTROY10, bBattlePassId);
		if (nextMissionId2 != 0)
		{
			DWORD dwCount, dwVnum;
			if (CBattlePass::instance().BattlePassMissionGetInfo(bBattlePassId, nextMissionId2, &dwVnum, &dwCount))
			{
				if (item->GetVnum() == 31126 && GetMissionProgress(nextMissionId2, bBattlePassId) < dwCount)
					UpdateMissionProgress(nextMissionId2, bBattlePassId, item->GetCount(), dwCount);
			}
		}
#endif // ENABLE_BATTLE_PASS_EX
	}
#endif

	// EXTRA_CHECK
#ifdef __GROWTH_PET_SYSTEM__
	if (item->IsNewPetItem())
		delete DBManager::instance().DirectQuery("DELETE FROM new_petsystem WHERE id = %u;", item->GetID());
#endif
#ifdef __GROWTH_MOUNT_SYSTEM__
	if (item->IsNewMountExItem())
		delete DBManager::instance().DirectQuery("DELETE FROM new_mountsystem WHERE id = %u;", item->GetID());
#endif
	ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%s has been deleted successfully."), item->GetName());
	ITEM_MANAGER::instance().RemoveItem(item);

	return true;
}

bool CHARACTER::SellItem(TItemPos Cell)
{
	LPITEM item = NULL;

	if (!CanHandleItem())
		return false;

	if (IsDead() || IsStun())
		return false;

	if (!IsValidItemPosition(Cell) || !(item = GetItem(Cell)))
		return false;

	if (true == item->isLocked() || item->IsExchanging() || item->IsEquipped())
		return false;

#ifdef __BEGINNER_ITEM__
	if (item->IsBasicItem())
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ITEM_IS_BASIC_CANNOT_DO"));
		return false;
	}
#endif

	if (GetGold() >= GOLD_MAX)
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("YOU_HAVE_MUCH_GOLD_FROM_INVENTORY"));
		return false;
	}

	if (IS_SET(item->GetAntiFlag(), ITEM_ANTIFLAG_SELL))
		return false;

	if (quest::CQuestManager::instance().GetPCForce(GetPlayerID())->IsRunning() == true)
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_OPERATION_IS_FAILED_QUEST_WINDOW_IS_OPENED"));
		return false;
	}

	if (!CanAct(true, true, VAR_TIME_NONE))
		return false;

#ifdef __GOLD_LIMIT_REWORK__
	long long llPrice;
#else
	DWORD dwPrice;
#endif
#ifdef __ITEM_COUNT_LIMIT__
	DWORD bCount;
#else
	BYTE bCount;
#endif
	bCount = item->GetCount();
#ifdef __GOLD_LIMIT_REWORK__
	llPrice = item->GetShopBuyPrice();

	if (IS_SET(item->GetFlag(), ITEM_FLAG_COUNT_PER_1GOLD))
	{
		if (llPrice == 0)
			llPrice = bCount;
		else
			llPrice = bCount / llPrice;
	}
	else
		llPrice *= bCount;
#else
	dwPrice = item->GetShopBuyPrice();

	if (IS_SET(item->GetFlag(), ITEM_FLAG_COUNT_PER_1GOLD))
	{
		if (dwPrice == 0)
			dwPrice = bCount;
		else
			dwPrice = bCount / dwPrice;
	}
	else
		dwPrice *= bCount;
#endif

#ifdef __GOLD_LIMIT_REWORK__
	const long long nTotalMoney = static_cast<long long>(GetGold()) + static_cast<long long>(llPrice);
#else
	const int64_t nTotalMoney = static_cast<int64_t>(GetGold()) + static_cast<int64_t>(dwPrice);
#endif

	if (GOLD_MAX <= nTotalMoney)
	{
#ifdef __GOLD_LIMIT_REWORK__
		//sys_err("[OVERFLOW_GOLD] id %u name %s gold %lld", GetPlayerID(), GetName(), GetGold());
#else
		//sys_err("[OVERFLOW_GOLD] id %u name %s gold %u", GetPlayerID(), GetName(), GetGold());
#endif
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("20¾ï³ÉÀÌ ÃÊ°úÇÏ¿© ¹°Ç°À» ÆÈ¼ö ¾ø½À´Ï´Ù."));
		return false;
	}

#ifdef __GOLD_LIMIT_REWORK__
	long long totalMoneyEx = GetGold() + llPrice;
	if (GOLD_MAX <= totalMoneyEx)
	{
		//sys_err("[OVERFLOW_GOLD] id %u name %s gold %lld", GetPlayerID(), GetName(), GetGold());
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("20¾ï³ÉÀÌ ÃÊ°úÇÏ¿© ¹°Ç°À» ÆÈ¼ö ¾ø½À´Ï´Ù."));
		return false;
	}
#else
	DWORD totalMoneyEx = GetGold() + dwPrice;
	if (GOLD_MAX <= totalMoneyEx)
	{
		//sys_err("[OVERFLOW_GOLD] id %u name %s gold %u", GetPlayerID(), GetName(), GetGold());
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("20¾ï³ÉÀÌ ÃÊ°úÇÏ¿© ¹°Ç°À» ÆÈ¼ö ¾ø½À´Ï´Ù."));
		return false;
	}
#endif

#ifdef __BLEND_ITEM_REWORK__
	if (item->GetType() == ITEM_BLEND && item->GetSubType() == INFINITY_BLEND)
	{
		int affect_type = AFFECT_BLEND;
		int apply_type = aApplyInfo[item->GetSocket(0)].bPointType;
		CAffect* pAffect = FindAffect(affect_type, apply_type);
		bool bIsActive = item->GetSocket(2) > 0;

		if (pAffect)
		{
			if (bIsActive)
			{
				RemoveAffect(pAffect);
				item->Lock(false);
				item->SetSocket(2, 0);
			}
		}
	}
#endif

#ifdef __GROWTH_PET_SYSTEM__
	if (item->IsNewPetItem())
		delete DBManager::instance().DirectQuery("DELETE FROM new_petsystem WHERE id = %u;", item->GetID());
#endif
#ifdef __GROWTH_MOUNT_SYSTEM__
	if (item->IsNewMountExItem())
		delete DBManager::instance().DirectQuery("DELETE FROM new_mountsystem WHERE id = %u;", item->GetID());
#endif
	ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%s has been sold successfully."), item->GetName());
	ITEM_MANAGER::instance().RemoveItem(item);
#ifdef __GOLD_LIMIT_REWORK__
	PointChange(POINT_GOLD, llPrice, false);
#else
	PointChange(POINT_GOLD, dwPrice, false);
#endif
	return true;
}
#endif

#ifdef __ITEM_COUNT_LIMIT__
bool CHARACTER::MoveItem(TItemPos Cell, TItemPos DestCell, DWORD count)
#else
bool CHARACTER::MoveItem(TItemPos Cell, TItemPos DestCell, BYTE count)
#endif
{
	LPITEM item = NULL;

	if (!IsValidItemPosition(Cell))
		return false;

	if (!(item = GetItem(Cell)))
		return false;

	if (item->IsExchanging())
		return false;

	if (item->GetCount() < count)
		return false;

	if (INVENTORY == Cell.window_type && Cell.cell >= INVENTORY_MAX_NUM && IS_SET(item->GetFlag(), ITEM_FLAG_IRREMOVABLE))
		return false;

#ifdef __ADDITIONAL_INVENTORY__ //moveitem fix
	if (item->IsUpgradeItem() && DestCell.window_type != UPGRADE_INVENTORY)
		return false;

	if (item->IsBook() && DestCell.window_type != BOOK_INVENTORY)
		return false;

	if (item->IsStone() && DestCell.window_type != STONE_INVENTORY)
		return false;

	if (item->IsFlower() && DestCell.window_type != FLOWER_INVENTORY)
		return false;

	if (item->IsAttrItem() && DestCell.window_type != ATTR_INVENTORY)
		return false;

	if (item->IsChest() && DestCell.window_type != CHEST_INVENTORY)
		return false;
#endif

	if (true == item->isLocked())
		return false;

	if (!IsValidItemPosition(DestCell))
		return false;

	if (!CanHandleItem())
	{
		if (NULL != DragonSoul_RefineWindow_GetOpener())
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("°­È­Ã¢À» ¿¬ »óÅÂ¿¡¼­´Â ¾ÆÀÌÅÛÀ» ¿Å±æ ¼ö ¾ø½À´Ï´Ù."));
		return false;
	}

	if (DestCell.IsBeltInventoryPosition() && false == CBeltInventoryHelper::CanMoveIntoBeltInventory(item))
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ ¾ÆÀÌÅÛÀº º§Æ® ÀÎº¥Åä¸®·Î ¿Å±æ ¼ö ¾ø½À´Ï´Ù."));
		return false;
	}

#ifdef __SWITCHBOT__
	if (Cell.IsSwitchbotPosition() && CSwitchbotManager::Instance().IsActive(GetPlayerID(), Cell.cell))
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Cannot move active switchbot item."));
		return false;
	}

	if (DestCell.IsSwitchbotPosition() && !SwitchbotHelper::IsValidItem(item))
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Invalid item type for switchbot."));
		return false;
	}

	if ((DestCell.IsSwitchbotPosition() && item->IsEquipped()) || (Cell.IsSwitchbotPosition() && DestCell.IsEquipPosition()))
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You cannot move equiped items here."));
		return false;
	}

#ifdef __BEGINNER_ITEM__
	if (DestCell.IsSwitchbotPosition() && item->IsBasicItem())
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Invalid item type for switchbot."));
		return false;
	}
#endif

#endif

	if (Cell.IsEquipPosition())
	{
		if (!CanUnequipNow(item))
			return false;

#ifdef __WEAPON_COSTUME_SYSTEM__
		int iWearCell = item->FindEquipCell(this);
		if (iWearCell == WEAR_WEAPON)
		{
			LPITEM costumeWeapon = GetWear(WEAR_COSTUME_WEAPON);
			if (costumeWeapon && !UnequipItem(costumeWeapon))
			{
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You cannot unequip the costume weapon. Not enough space."));
				return false;
			}

			if (!IsEmptyItemGrid(DestCell, item->GetSize(), Cell.cell))
				return UnequipItem(item);
		}
#endif
	}

	if (DestCell.IsEquipPosition())
	{
		if (GetItem(DestCell))
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ¹Ì Àåºñ¸¦ Âø¿ëÇÏ°í ÀÖ½À´Ï´Ù."));

			return false;
		}

		EquipItem(item, DestCell.cell - INVENTORY_MAX_NUM);
	}
	else
	{
		if (item->IsDragonSoul())
		{
			if (item->IsEquipped())
			{
				return DSManager::instance().PullOut(this, DestCell, item);
			}
			else
			{
				if (DestCell.window_type != DRAGON_SOUL_INVENTORY)
				{
					return false;
				}

				if (!DSManager::instance().IsValidCellForThisItem(item, DestCell))
					return false;
			}
		}

		else if (DRAGON_SOUL_INVENTORY == DestCell.window_type)
			return false;

#ifdef __ADDITIONAL_INVENTORY__
		if (!item->IsUpgradeItem() && UPGRADE_INVENTORY == DestCell.window_type)
			return false;

		if (!item->IsBook() && BOOK_INVENTORY == DestCell.window_type)
			return false;

		if (!item->IsStone() && STONE_INVENTORY == DestCell.window_type)
			return false;

		if (!item->IsFlower() && FLOWER_INVENTORY == DestCell.window_type)
			return false;

		if (!item->IsAttrItem() && ATTR_INVENTORY == DestCell.window_type)
			return false;

		if (!item->IsChest() && CHEST_INVENTORY == DestCell.window_type)
			return false;
#endif

		LPITEM item2;

		if ((item2 = GetItem(DestCell)) && item != item2 && item2->IsStackable() &&
			!IS_SET(item2->GetAntiFlag(), ITEM_ANTIFLAG_STACK) &&
			item2->GetVnum() == item->GetVnum() && !item2->IsExchanging())
		{
			for (int i = 0; i < ITEM_SOCKET_MAX_NUM; ++i)
				if (item2->GetSocket(i) != item->GetSocket(i))
					return false;

			if (count == 0)
				count = item->GetCount();

			sys_log(0, "%s: ITEM_STACK %s (window: %d, cell : %d) -> (window:%d, cell %d) count %d", GetName(), item->GetName(), Cell.window_type, Cell.cell,
				DestCell.window_type, DestCell.cell, count);

			count = MIN(g_bItemCountLimit - item2->GetCount(), count);

			item->SetCount(item->GetCount() - count);
			item2->SetCount(item2->GetCount() + count);
			return true;
		}

		if (!IsEmptyItemGrid(DestCell, item->GetSize(), Cell.cell))
			return false;

		if (count == 0 || count >= item->GetCount() || !item->IsStackable() || IS_SET(item->GetAntiFlag(), ITEM_ANTIFLAG_STACK))
		{
			sys_log(0, "%s: ITEM_MOVE %s (window: %d, cell : %d) -> (window:%d, cell %d) count %d", GetName(), item->GetName(), Cell.window_type, Cell.cell,
				DestCell.window_type, DestCell.cell, count);
			item->RemoveFromCharacter();
#ifdef __HIGHLIGHT_ITEM__
			__SET_ITEM_HG(DestCell, item, true);
#else
			SetItem(DestCell, item);
#endif
			if (INVENTORY == Cell.window_type && INVENTORY == DestCell.window_type)
				SyncQuickslot(QUICKSLOT_TYPE_ITEM, Cell.cell, DestCell.cell);
		}
		else if (count < item->GetCount())
		{
			//sys_log(0, "%s: ITEM_SPLIT %s (window: %d, cell : %d) -> (window:%d, cell %d) count %d", GetName(), item->GetName(), Cell.window_type, Cell.cell,
				//DestCell.window_type, DestCell.cell, count);

			item->SetCount(item->GetCount() - count);
			LPITEM item2 = ITEM_MANAGER::instance().CreateItem(item->GetVnum(), count);

			// copy socket -- by mhh
			FN_copy_item_socket(item2, item);
#ifdef __BEGINNER_ITEM__
			item2->SetBasic(item->IsBasicItem());
#endif
			item2->__ADD_TO_CHARACTER(this, DestCell);
		}
	}

	return true;
}

namespace NPartyPickupDistribute
{
	struct FFindOwnership
	{
		LPITEM item;
		LPCHARACTER owner;

		FFindOwnership(LPITEM item)
			: item(item), owner(NULL)
		{
		}

		void operator () (LPCHARACTER ch)
		{
			if (item->IsOwnership(ch))
				owner = ch;
		}
	};

	struct FCountNearMember
	{
		int		total;
		int		x, y;

		FCountNearMember(LPCHARACTER center)
			: total(0), x(center->GetX()), y(center->GetY())
		{
		}

		void operator () (LPCHARACTER ch)
		{
			if (DISTANCE_APPROX(ch->GetX() - x, ch->GetY() - y) <= PARTY_DEFAULT_RANGE)
				total += 1;
		}
	};

	struct FMoneyDistributor
	{
		int		total;
		LPCHARACTER	c;
		int		x, y;
#ifdef __GOLD_LIMIT_REWORK__
		long long		llMoney;

		FMoneyDistributor(LPCHARACTER center, long long llMoney)
			: total(0), c(center), x(center->GetX()), y(center->GetY()), llMoney(llMoney)
		{
		}

		void operator ()(LPCHARACTER ch)
		{
			if (ch != c)
				if (DISTANCE_APPROX(ch->GetX() - x, ch->GetY() - y) <= PARTY_DEFAULT_RANGE)
				{
					if (ch->GetGold() < GOLD_MAX)
						ch->PointChange(POINT_GOLD, llMoney, true);
				}
#else
		int		iMoney;

		FMoneyDistributor(LPCHARACTER center, int iMoney)
			: total(0), c(center), x(center->GetX()), y(center->GetY()), iMoney(iMoney)
		{
		}

		void operator ()(LPCHARACTER ch)
		{
			if (ch != c)
				if (DISTANCE_APPROX(ch->GetX() - x, ch->GetY() - y) <= PARTY_DEFAULT_RANGE)
				{
					ch->PointChange(POINT_GOLD, iMoney, true);
				}
#endif
		}
		};
	}

#ifdef __GOLD_LIMIT_REWORK__
void CHARACTER::GiveGold(long long iAmount)
#else
void CHARACTER::GiveGold(int iAmount)
#endif
{
	if (iAmount <= 0)
		return;

	if (GetParty())
	{
		LPPARTY pParty = GetParty();

#ifdef __GOLD_LIMIT_REWORK__
		long long dwTotal = iAmount;
		long long dwMyAmount = dwTotal;
#else
		DWORD dwTotal = iAmount;
		DWORD dwMyAmount = dwTotal;
#endif

		NPartyPickupDistribute::FCountNearMember funcCountNearMember(this);
		pParty->ForEachOnlineMember(funcCountNearMember);

		if (funcCountNearMember.total > 1)
		{
			DWORD dwShare = dwTotal / funcCountNearMember.total;
			dwMyAmount -= dwShare * (funcCountNearMember.total - 1);

			NPartyPickupDistribute::FMoneyDistributor funcMoneyDist(this, dwShare);

			pParty->ForEachOnlineMember(funcMoneyDist);
		}

		PointChange(POINT_GOLD, dwMyAmount, true);
	}
	else
	{
		PointChange(POINT_GOLD, iAmount, true);
	}
}

//This should be your old.
#ifdef __DROP_CH_RENEWAL__
bool CHARACTER::PickupItem(DWORD dwVID, bool forced)
#else
bool CHARACTER::PickupItem(DWORD dwVID)
#endif
{
	LPITEM item = ITEM_MANAGER::instance().FindByVID(dwVID);

#ifdef __DROP_CH_RENEWAL__
	if (!forced)
	{
		if (IsDead())
			return false;

		if (IsObserverMode())
			return false;

		if (GetExchange() || GetMyShop() || GetShopOwner() || IsOpenSafebox() || IsCubeOpen() || IsUnderRefine())
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("°Å·¡Ã¢,Ã¢°í µîÀ» ¿¬ »óÅÂ¿¡¼­´Â ±ÍÈ¯ºÎ,±ÍÈ¯±â¾ïºÎ ¸¦ »ç¿ëÇÒ¼ö ¾ø½À´Ï´Ù."));
			return false;
		}

#ifdef __ACCE_SYSTEM__
		if (isAcceOpened(true) || isAcceOpened(false))
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("PLEASE_BEFORE_CLOSE_WINDOW_AND_USE_THIS_FUNCTION"));
			return false;
		}
#endif

#ifdef __ITEM_CHANGELOOK__
		if (isChangeLookOpened() == true)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("PLEASE_BEFORE_CLOSE_WINDOW_AND_USE_THIS_FUNCTION"));
			return false;
		}
#endif

#ifdef __OFFLINE_SHOP__
		if (GetOfflineShopGuest() || GetShopSafebox())
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("PLEASE_BEFORE_CLOSE_WINDOW_AND_USE_THIS_FUNCTION"));
			return false;
		}
#endif
	}
#else
	if (IsDead())
		return false;

	if (IsObserverMode())
		return false;

	if (GetExchange() || GetMyShop() || GetShopOwner() || IsOpenSafebox() || IsCubeOpen())
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("°Å·¡Ã¢,Ã¢°í µîÀ» ¿¬ »óÅÂ¿¡¼­´Â ±ÍÈ¯ºÎ,±ÍÈ¯±â¾ïºÎ ¸¦ »ç¿ëÇÒ¼ö ¾ø½À´Ï´Ù."));
		return false;
	}

#ifdef __ACCE_SYSTEM__
	if (isAcceOpened(true) || isAcceOpened(false))
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("PLEASE_BEFORE_CLOSE_WINDOW_AND_USE_THIS_FUNCTION"));
		return false;
	}
#endif

#ifdef __ITEM_CHANGELOOK__
	if (isChangeLookOpened() == true)
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("PLEASE_BEFORE_CLOSE_WINDOW_AND_USE_THIS_FUNCTION"));
		return false;
	}
#endif

#ifdef __OFFLINE_SHOP__
	if (GetOfflineShopGuest())
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("PLEASE_BEFORE_CLOSE_WINDOW_AND_USE_THIS_FUNCTION"));
		return false;
	}
#endif
#endif // __DROP_CH_RENEWAL__

	if (!item || !item->GetSectree())
		return false;

#ifdef __BATTLE_PASS_SYSTEM__
	bool bIsBattlePass = item->HaveOwnership();
#endif

	if (item->DistanceValid(this))
	{
		// @fixme150 BEGIN
		if (item->GetType() == ITEM_QUEST)
		{
			if (quest::CQuestManager::instance().GetPCForce(GetPlayerID())->IsRunning() == true)
			{
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You cannot pickup this item if you're using quests"));
				return false;
			}
		}
		// @fixme150 END
		if (item->IsOwnership(this))
		{
			if (item->GetType() == ITEM_ELK)
			{
				GiveGold(item->GetCount());
				item->RemoveFromGround();

				M2_DESTROY_ITEM(item);

				Save();
			}

			else
			{
#ifdef __ADDITIONAL_INVENTORY__
				if (item->IsUpgradeItem() && item->IsStackable() && !IS_SET(item->GetAntiFlag(), ITEM_ANTIFLAG_STACK))
				{
#ifdef __ITEM_COUNT_LIMIT__
					DWORD bCount = item->GetCount();
#else
					BYTE bCount = item->GetCount();
#endif
					for (int i = 0; i < SPECIAL_INVENTORY_MAX_NUM; ++i)
					{
						LPITEM item2 = GetUpgradeInventoryItem(i);
						if (!item2)
							continue;
						if (item2->GetVnum() == item->GetVnum())
						{
#ifdef __ITEM_COUNT_LIMIT__
							DWORD bCount2 = MIN(g_bItemCountLimit - item2->GetCount(), bCount);
#else
							BYTE bCount2 = MIN(g_bItemCountLimit - item2->GetCount(), bCount);
#endif
							bCount -= bCount2;
							item2->SetCount(item2->GetCount() + bCount2);
							if (bCount == 0)
							{
#ifdef __BATTLE_PASS_SYSTEM__
								if (bIsBattlePass)
								{
									BYTE bBattlePassId = GetBattlePassId();
									if (bBattlePassId)
									{
										DWORD dwItemVnum, dwCount;
										if (CBattlePass::instance().BattlePassMissionGetInfo(bBattlePassId, COLLECT_ITEM, &dwItemVnum, &dwCount))
										{
											if (dwItemVnum == item->GetVnum() && GetMissionProgress(COLLECT_ITEM, bBattlePassId) < dwCount)
												UpdateMissionProgress(COLLECT_ITEM, bBattlePassId, bCount2, dwCount);
										}
									}
								}
#endif
								ChatPacket(CHAT_TYPE_INFO, LC_TEXT("UPGRADE_INVENTORY_ADDED: %s"), item2->GetName());
								M2_DESTROY_ITEM(item);
								return true;
							}
						}
					}
					item->SetCount(bCount);
				}
				else if (item->IsBook() && item->IsStackable() && !IS_SET(item->GetAntiFlag(), ITEM_ANTIFLAG_STACK))
				{
#ifdef __ITEM_COUNT_LIMIT__
					DWORD bCount = item->GetCount();
#else
					BYTE bCount = item->GetCount();
#endif
					for (int i = 0; i < SPECIAL_INVENTORY_MAX_NUM; ++i)
					{
						LPITEM item2 = GetBookInventoryItem(i);
						if (!item2)
							continue;
						if (item2->GetVnum() == item->GetVnum())
						{
							int j;
							for (j = 0; j < ITEM_SOCKET_MAX_NUM; ++j)
								if (item2->GetSocket(j) != item->GetSocket(j))
									break;
							if (j != ITEM_SOCKET_MAX_NUM)
								continue;
#ifdef __ITEM_COUNT_LIMIT__
							DWORD bCount2 = MIN(g_bItemCountLimit - item2->GetCount(), bCount);
#else
							BYTE bCount2 = MIN(g_bItemCountLimit - item2->GetCount(), bCount);
#endif
							bCount -= bCount2;
							item2->SetCount(item2->GetCount() + bCount2);

							if (bCount == 0)
							{
#ifdef __BATTLE_PASS_SYSTEM__
								if (bIsBattlePass)
								{
									BYTE bBattlePassId = GetBattlePassId();
									if (bBattlePassId)
									{
										DWORD dwItemVnum, dwCount;
										if (CBattlePass::instance().BattlePassMissionGetInfo(bBattlePassId, COLLECT_ITEM, &dwItemVnum, &dwCount))
										{
											if (dwItemVnum == item->GetVnum() && GetMissionProgress(COLLECT_ITEM, bBattlePassId) < dwCount)
												UpdateMissionProgress(COLLECT_ITEM, bBattlePassId, bCount2, dwCount);
										}
									}
								}
#endif
								ChatPacket(CHAT_TYPE_INFO, LC_TEXT("BOOK_INVENTORY_ADDED: %s"), item2->GetName());
								M2_DESTROY_ITEM(item);
								return true;
							}
						}
					}
					item->SetCount(bCount);
				}
				else if (item->IsStone() && item->IsStackable() && !IS_SET(item->GetAntiFlag(), ITEM_ANTIFLAG_STACK))
				{
#ifdef __ITEM_COUNT_LIMIT__
					DWORD bCount = item->GetCount();
#else
					BYTE bCount = item->GetCount();
#endif
					for (int i = 0; i < SPECIAL_INVENTORY_MAX_NUM; ++i)
					{
						LPITEM item2 = GetStoneInventoryItem(i);
						if (!item2)
							continue;
						if (item2->GetVnum() == item->GetVnum())
						{
#ifdef __ITEM_COUNT_LIMIT__
							DWORD bCount2 = MIN(g_bItemCountLimit - item2->GetCount(), bCount);
#else
							BYTE bCount2 = MIN(g_bItemCountLimit - item2->GetCount(), bCount);
#endif
							bCount -= bCount2;
							item2->SetCount(item2->GetCount() + bCount2);

							if (bCount == 0)
							{
#ifdef __BATTLE_PASS_SYSTEM__
								if (bIsBattlePass)
								{
									BYTE bBattlePassId = GetBattlePassId();
									if (bBattlePassId)
									{
										DWORD dwItemVnum, dwCount;
										if (CBattlePass::instance().BattlePassMissionGetInfo(bBattlePassId, COLLECT_ITEM, &dwItemVnum, &dwCount))
										{
											if (dwItemVnum == item->GetVnum() && GetMissionProgress(COLLECT_ITEM, bBattlePassId) < dwCount)
												UpdateMissionProgress(COLLECT_ITEM, bBattlePassId, bCount2, dwCount);
										}
									}
								}
#endif
								ChatPacket(CHAT_TYPE_INFO, LC_TEXT("STONE_INVENTORY_ADDED: %s"), item2->GetName());
								M2_DESTROY_ITEM(item);
								return true;
							}
						}
					}
					item->SetCount(bCount);
				}

				else if (item->IsFlower() && item->IsStackable() && !IS_SET(item->GetAntiFlag(), ITEM_ANTIFLAG_STACK))
				{
#ifdef __ITEM_COUNT_LIMIT__
					DWORD bCount = item->GetCount();
#else
					BYTE bCount = item->GetCount();
#endif
					for (int i = 0; i < SPECIAL_INVENTORY_MAX_NUM; ++i)
					{
						LPITEM item2 = GetFlowerInventoryItem(i);
						if (!item2)
							continue;
						if (item2->GetVnum() == item->GetVnum())
						{
#ifdef __ITEM_COUNT_LIMIT__
							DWORD bCount2 = MIN(g_bItemCountLimit - item2->GetCount(), bCount);
#else
							BYTE bCount2 = MIN(g_bItemCountLimit - item2->GetCount(), bCount);
#endif
							bCount -= bCount2;
							item2->SetCount(item2->GetCount() + bCount2);

							if (bCount == 0)
							{
#ifdef __BATTLE_PASS_SYSTEM__
								if (bIsBattlePass)
								{
									BYTE bBattlePassId = GetBattlePassId();
									if (bBattlePassId)
									{
										DWORD dwItemVnum, dwCount;
										if (CBattlePass::instance().BattlePassMissionGetInfo(bBattlePassId, COLLECT_ITEM, &dwItemVnum, &dwCount))
										{
											if (dwItemVnum == item->GetVnum() && GetMissionProgress(COLLECT_ITEM, bBattlePassId) < dwCount)
												UpdateMissionProgress(COLLECT_ITEM, bBattlePassId, bCount2, dwCount);
										}
									}
								}
#endif
								ChatPacket(CHAT_TYPE_INFO, LC_TEXT("FLOWER_INVENTORY_ADDED: %s"), item2->GetName());
								M2_DESTROY_ITEM(item);
								return true;
							}
						}
					}
					item->SetCount(bCount);
				}

				else if (item->IsAttrItem() && item->IsStackable() && !IS_SET(item->GetAntiFlag(), ITEM_ANTIFLAG_STACK))
				{
#ifdef __ITEM_COUNT_LIMIT__
					DWORD bCount = item->GetCount();
#else
					BYTE bCount = item->GetCount();
#endif
					for (int i = 0; i < SPECIAL_INVENTORY_MAX_NUM; ++i)
					{
						LPITEM item2 = GetAttrInventoryItem(i);
						if (!item2)
							continue;
						if (item2->GetVnum() == item->GetVnum())
						{
#ifdef __ITEM_COUNT_LIMIT__
							DWORD bCount2 = MIN(g_bItemCountLimit - item2->GetCount(), bCount);
#else
							BYTE bCount2 = MIN(g_bItemCountLimit - item2->GetCount(), bCount);
#endif
							bCount -= bCount2;
							item2->SetCount(item2->GetCount() + bCount2);

							if (bCount == 0)
							{
#ifdef __BATTLE_PASS_SYSTEM__
								if (bIsBattlePass)
								{
									BYTE bBattlePassId = GetBattlePassId();
									if (bBattlePassId)
									{
										DWORD dwItemVnum, dwCount;
										if (CBattlePass::instance().BattlePassMissionGetInfo(bBattlePassId, COLLECT_ITEM, &dwItemVnum, &dwCount))
										{
											if (dwItemVnum == item->GetVnum() && GetMissionProgress(COLLECT_ITEM, bBattlePassId) < dwCount)
												UpdateMissionProgress(COLLECT_ITEM, bBattlePassId, bCount2, dwCount);
										}
									}
								}
#endif
								ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ATTR_INVENTORY_ADDED: %s"), item2->GetName());
								M2_DESTROY_ITEM(item);
								return true;
							}
						}
					}
					item->SetCount(bCount);
				}

				else if (item->IsChest() && item->IsStackable() && !IS_SET(item->GetAntiFlag(), ITEM_ANTIFLAG_STACK))
				{
#ifdef __ITEM_COUNT_LIMIT__
					DWORD bCount = item->GetCount();
#else
					BYTE bCount = item->GetCount();
#endif
					for (int i = 0; i < SPECIAL_INVENTORY_MAX_NUM; ++i)
					{
						LPITEM item2 = GetChestInventoryItem(i);
						if (!item2)
							continue;
						if (item2->GetVnum() == item->GetVnum())
						{
#ifdef __ITEM_COUNT_LIMIT__
							DWORD bCount2 = MIN(g_bItemCountLimit - item2->GetCount(), bCount);
#else
							BYTE bCount2 = MIN(g_bItemCountLimit - item2->GetCount(), bCount);
#endif
							bCount -= bCount2;
							item2->SetCount(item2->GetCount() + bCount2);

							if (bCount == 0)
							{
#ifdef __BATTLE_PASS_SYSTEM__
								if (bIsBattlePass)
								{
									BYTE bBattlePassId = GetBattlePassId();
									if (bBattlePassId)
									{
										DWORD dwItemVnum, dwCount;
										if (CBattlePass::instance().BattlePassMissionGetInfo(bBattlePassId, COLLECT_ITEM, &dwItemVnum, &dwCount))
										{
											if (dwItemVnum == item->GetVnum() && GetMissionProgress(COLLECT_ITEM, bBattlePassId) < dwCount)
												UpdateMissionProgress(COLLECT_ITEM, bBattlePassId, bCount2, dwCount);
										}
									}
								}
#endif
								ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CHEST_INVENTORY_ADDED: %s"), item2->GetName());
								M2_DESTROY_ITEM(item);
								return true;
							}
						}
					}
					item->SetCount(bCount);
				}
				else
				{
					if (item->IsStackable() && !IS_SET(item->GetAntiFlag(), ITEM_ANTIFLAG_STACK))
					{
#ifdef __ITEM_COUNT_LIMIT__
						DWORD bCount = item->GetCount();
#else
						BYTE bCount = item->GetCount();
#endif
						for (int i = 0; i < INVENTORY_MAX_NUM; ++i)
						{
							LPITEM item2 = GetInventoryItem(i);

							if (!item2)
								continue;

							if (item2->GetVnum() == item->GetVnum())
							{
								int j;

								for (j = 0; j < ITEM_SOCKET_MAX_NUM; ++j)
									if (item2->GetSocket(j) != item->GetSocket(j))
										break;

								if (j != ITEM_SOCKET_MAX_NUM)
									continue;
#ifdef __ITEM_COUNT_LIMIT__
								DWORD bCount2 = MIN(g_bItemCountLimit - item2->GetCount(), bCount);
#else
								BYTE bCount2 = MIN(g_bItemCountLimit - item2->GetCount(), bCount);
#endif
								bCount -= bCount2;
								item2->SetCount(item2->GetCount() + bCount2);

								if (bCount == 0)
								{
#ifdef __BATTLE_PASS_SYSTEM__
									if (bIsBattlePass)
									{
										BYTE bBattlePassId = GetBattlePassId();
										if (bBattlePassId)
										{
											DWORD dwItemVnum, dwCount;
											if (CBattlePass::instance().BattlePassMissionGetInfo(bBattlePassId, COLLECT_ITEM, &dwItemVnum, &dwCount))
											{
												if (dwItemVnum == item->GetVnum() && GetMissionProgress(COLLECT_ITEM, bBattlePassId) < dwCount)
													UpdateMissionProgress(COLLECT_ITEM, bBattlePassId, bCount2, dwCount);
											}
										}
									}
#endif
									ChatPacket(CHAT_TYPE_INFO, LC_TEXT("NORMAL_INVENTORY_ADDED: %s"), item2->GetName());
									M2_DESTROY_ITEM(item);
									if (item2->GetType() == ITEM_QUEST)
										quest::CQuestManager::instance().PickupItem(GetPlayerID(), item2);
									return true;
								}
							}
						}

						item->SetCount(bCount);
					}
				}
#else
				if (item->IsStackable() && !IS_SET(item->GetAntiFlag(), ITEM_ANTIFLAG_STACK))
				{
#ifdef __ITEM_COUNT_LIMIT__
					DWORD bCount = item->GetCount();
#else
					BYTE bCount = item->GetCount();
#endif
					for (int i = 0; i < INVENTORY_MAX_NUM; ++i)
					{
						LPITEM item2 = GetInventoryItem(i);

						if (!item2)
							continue;

						if (item2->GetVnum() == item->GetVnum())
						{
							int j;

							for (j = 0; j < ITEM_SOCKET_MAX_NUM; ++j)
								if (item2->GetSocket(j) != item->GetSocket(j))
									break;

							if (j != ITEM_SOCKET_MAX_NUM)
								continue;
#ifdef __ITEM_COUNT_LIMIT__
							DWORD bCount2 = MIN(g_bItemCountLimit - item2->GetCount(), bCount);
#else
							BYTE bCount2 = MIN(g_bItemCountLimit - item2->GetCount(), bCount);
#endif
							bCount -= bCount2;
							item2->SetCount(item2->GetCount() + bCount2);

							if (bCount == 0)
							{
								ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¾ÆÀÌÅÛ È¹µæ: %s"), item2->GetName());
								M2_DESTROY_ITEM(item);
								if (item2->GetType() == ITEM_QUEST)
									quest::CQuestManager::instance().PickupItem(GetPlayerID(), item2);
								return true;
							}
						}
					}

					item->SetCount(bCount);
				}
#endif

				int iEmptyCell;
				if (item->IsDragonSoul())
				{
					if ((iEmptyCell = GetEmptyDragonSoulInventory(item)) == -1)
					{
						//sys_log(0, "No empty ds inventory pid %u size %ud itemid %u", GetPlayerID(), item->GetSize(), item->GetID());
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼ÒÁöÇÏ°í ÀÖ´Â ¾ÆÀÌÅÛÀÌ ³Ê¹« ¸¹½À´Ï´Ù."));
						return false;
					}
				}
#ifdef __ADDITIONAL_INVENTORY__
				else if (item->IsUpgradeItem())
				{
					if ((iEmptyCell = GetEmptyUpgradeInventory(item)) == -1)
					{
						//sys_log(0, "No empty ssu inventory pid %u size %ud itemid %u", GetPlayerID(), item->GetSize(), item->GetID());
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼ÒÁöÇÏ°í ÀÖ´Â ¾ÆÀÌÅÛÀÌ ³Ê¹« ¸¹½À´Ï´Ù."));
						return false;
					}
				}
				else if (item->IsBook())
				{
					if ((iEmptyCell = GetEmptyBookInventory(item)) == -1)
					{
						//sys_log(0, "No empty ssu inventory pid %u size %ud itemid %u", GetPlayerID(), item->GetSize(), item->GetID());
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼ÒÁöÇÏ°í ÀÖ´Â ¾ÆÀÌÅÛÀÌ ³Ê¹« ¸¹½À´Ï´Ù."));
						return false;
					}
				}
				else if (item->IsStone())
				{
					if ((iEmptyCell = GetEmptyStoneInventory(item)) == -1)
					{
						//sys_log(0, "No empty ssu inventory pid %u size %ud itemid %u", GetPlayerID(), item->GetSize(), item->GetID());
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼ÒÁöÇÏ°í ÀÖ´Â ¾ÆÀÌÅÛÀÌ ³Ê¹« ¸¹½À´Ï´Ù."));
						return false;
					}
				}
				else if (item->IsFlower())
				{
					if ((iEmptyCell = GetEmptyFlowerInventory(item)) == -1)
					{
						//sys_log(0, "No empty ssf inventory pid %u size %ud itemid %u", GetPlayerID(), item->GetSize(), item->GetID());
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼ÒÁöÇÏ°í ÀÖ´Â ¾ÆÀÌÅÛÀÌ ³Ê¹« ¸¹½À´Ï´Ù."));
						return false;
					}
				}
				else if (item->IsAttrItem())
				{
					if ((iEmptyCell = GetEmptyAttrInventory(item)) == -1)
					{
						//sys_log(0, "No empty ssa inventory pid %u size %ud itemid %u", GetPlayerID(), item->GetSize(), item->GetID());
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼ÒÁöÇÏ°í ÀÖ´Â ¾ÆÀÌÅÛÀÌ ³Ê¹« ¸¹½À´Ï´Ù."));
						return false;
					}
				}
				else if (item->IsChest())
				{
					if ((iEmptyCell = GetEmptyChestInventory(item)) == -1)
					{
						//sys_log(0, "No empty ssc inventory pid %u size %ud itemid %u", GetPlayerID(), item->GetSize(), item->GetID());
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼ÒÁöÇÏ°í ÀÖ´Â ¾ÆÀÌÅÛÀÌ ³Ê¹« ¸¹½À´Ï´Ù."));
						return false;
					}
				}
#endif
				else
				{
					if ((iEmptyCell = GetEmptyInventory(item->GetSize())) == -1)
					{
						//sys_log(0, "No empty inventory pid %u size %ud itemid %u", GetPlayerID(), item->GetSize(), item->GetID());
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼ÒÁöÇÏ°í ÀÖ´Â ¾ÆÀÌÅÛÀÌ ³Ê¹« ¸¹½À´Ï´Ù."));
						return false;
					}
				}

				item->RemoveFromGround();

				if (item->IsDragonSoul())
					item->__ADD_TO_CHARACTER(this, TItemPos(DRAGON_SOUL_INVENTORY, iEmptyCell));
#ifdef __ADDITIONAL_INVENTORY__
				else if (item->IsUpgradeItem())
					item->__ADD_TO_CHARACTER(this, TItemPos(UPGRADE_INVENTORY, iEmptyCell));
				else if (item->IsBook())
					item->__ADD_TO_CHARACTER(this, TItemPos(BOOK_INVENTORY, iEmptyCell));
				else if (item->IsStone())
					item->__ADD_TO_CHARACTER(this, TItemPos(STONE_INVENTORY, iEmptyCell));
				else if (item->IsFlower())
					item->__ADD_TO_CHARACTER(this, TItemPos(FLOWER_INVENTORY, iEmptyCell));
				else if (item->IsAttrItem())
					item->__ADD_TO_CHARACTER(this, TItemPos(ATTR_INVENTORY, iEmptyCell));
				else if (item->IsChest())
					item->__ADD_TO_CHARACTER(this, TItemPos(CHEST_INVENTORY, iEmptyCell));
#endif
				else
					item->__ADD_TO_CHARACTER(this, TItemPos(INVENTORY, iEmptyCell));

				if (item->IsDragonSoul())
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("DRAGON_SOUL_INVENTORY_ADDED: %s"), item->GetName());
				else if (item->IsUpgradeItem())
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("UPGRADE_INVENTORY_ADDED: %s"), item->GetName());
				else if (item->IsBook())
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("BOOK_INVENTORY_ADDED: %s"), item->GetName());
				else if (item->IsStone())
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("STONE_INVENTORY_ADDED: %s"), item->GetName());
				else if (item->IsFlower())
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("FLOWER_INVENTORY_ADDED: %s"), item->GetName());
				else if (item->IsAttrItem())
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ATTR_INVENTORY_ADDED: %s"), item->GetName());
				else if (item->IsChest())
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CHEST_INVENTORY_ADDED: %s"), item->GetName());
				else
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("NORMAL_INVENTORY_ADDED: %s"), item->GetName());

				if (item->GetType() == ITEM_QUEST)
					quest::CQuestManager::instance().PickupItem(GetPlayerID(), item);

#ifdef __BATTLE_PASS_SYSTEM__
				if (bIsBattlePass)
				{
					BYTE bBattlePassId = GetBattlePassId();
					if (bBattlePassId)
					{
						DWORD dwItemVnum, dwCount;
						if (CBattlePass::instance().BattlePassMissionGetInfo(bBattlePassId, COLLECT_ITEM, &dwItemVnum, &dwCount))
						{
							if (dwItemVnum == item->GetVnum() && GetMissionProgress(COLLECT_ITEM, bBattlePassId) < dwCount)
								UpdateMissionProgress(COLLECT_ITEM, bBattlePassId, item->GetCount(), dwCount);
						}
					}
				}
#endif

			}

			//Motion(MOTION_PICKUP);
			return true;
		}
		else if (!IS_SET(item->GetAntiFlag(), ITEM_ANTIFLAG_GIVE | ITEM_ANTIFLAG_DROP) && GetParty())
		{
			NPartyPickupDistribute::FFindOwnership funcFindOwnership(item);

			GetParty()->ForEachOnlineMember(funcFindOwnership);

			LPCHARACTER owner = funcFindOwnership.owner;

			// @duzenleme
			// grup uyesi uzaksa veya oyunda degilse adamin itemini caliyolar.
			// bu yuzden eklendi.
			if (!owner)
				return false;

#ifdef __ADDITIONAL_INVENTORY__
				if (item->IsUpgradeItem() && item->IsStackable() && !IS_SET(item->GetAntiFlag(), ITEM_ANTIFLAG_STACK))
				{
#ifdef __ITEM_COUNT_LIMIT__
					DWORD bCount = item->GetCount();
#else
					BYTE bCount = item->GetCount();
#endif
					for (int i = 0; i < SPECIAL_INVENTORY_MAX_NUM; ++i)
					{
						LPITEM item2 = owner->GetUpgradeInventoryItem(i);
						if (!item2)
							continue;
						if (item2->GetVnum() == item->GetVnum())
						{
#ifdef __ITEM_COUNT_LIMIT__
							DWORD bCount2 = MIN(g_bItemCountLimit - item2->GetCount(), bCount);
#else
							BYTE bCount2 = MIN(g_bItemCountLimit - item2->GetCount(), bCount);
#endif
							bCount -= bCount2;
							item2->SetCount(item2->GetCount() + bCount2);
							if (bCount == 0)
							{
								owner->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("UPGRADE_INVENTORY_ADDED: %s"), item2->GetName());
								M2_DESTROY_ITEM(item);
								return true;
							}
						}
					}
					item->SetCount(bCount);
				}
				else if (item->IsBook() && item->IsStackable() && !IS_SET(item->GetAntiFlag(), ITEM_ANTIFLAG_STACK))
				{
#ifdef __ITEM_COUNT_LIMIT__
					DWORD bCount = item->GetCount();
#else
					BYTE bCount = item->GetCount();
#endif
					for (int i = 0; i < SPECIAL_INVENTORY_MAX_NUM; ++i)
					{
						LPITEM item2 = owner->GetBookInventoryItem(i);
						if (!item2)
							continue;
						if (item2->GetVnum() == item->GetVnum())
						{
							int j;
							for (j = 0; j < ITEM_SOCKET_MAX_NUM; ++j)
								if (item2->GetSocket(j) != item->GetSocket(j))
									break;
							if (j != ITEM_SOCKET_MAX_NUM)
								continue;
#ifdef __ITEM_COUNT_LIMIT__
							DWORD bCount2 = MIN(g_bItemCountLimit - item2->GetCount(), bCount);
#else
							BYTE bCount2 = MIN(g_bItemCountLimit - item2->GetCount(), bCount);
#endif
							bCount -= bCount2;
							item2->SetCount(item2->GetCount() + bCount2);

							if (bCount == 0)
							{
								owner->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("BOOK_INVENTORY_ADDED: %s"), item2->GetName());
								M2_DESTROY_ITEM(item);
								return true;
							}
						}
					}
					item->SetCount(bCount);
				}
				else if (item->IsStone() && item->IsStackable() && !IS_SET(item->GetAntiFlag(), ITEM_ANTIFLAG_STACK))
				{
#ifdef __ITEM_COUNT_LIMIT__
					DWORD bCount = item->GetCount();
#else
					BYTE bCount = item->GetCount();
#endif
					for (int i = 0; i < SPECIAL_INVENTORY_MAX_NUM; ++i)
					{
						LPITEM item2 = owner->GetStoneInventoryItem(i);
						if (!item2)
							continue;
						if (item2->GetVnum() == item->GetVnum())
						{
#ifdef __ITEM_COUNT_LIMIT__
							DWORD bCount2 = MIN(g_bItemCountLimit - item2->GetCount(), bCount);
#else
							BYTE bCount2 = MIN(g_bItemCountLimit - item2->GetCount(), bCount);
#endif
							bCount -= bCount2;
							item2->SetCount(item2->GetCount() + bCount2);

							if (bCount == 0)
							{
								owner->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("STONE_INVENTORY_ADDED: %s"), item2->GetName());
								M2_DESTROY_ITEM(item);
								return true;
							}
						}
					}
					item->SetCount(bCount);
				}

				else if (item->IsFlower() && item->IsStackable() && !IS_SET(item->GetAntiFlag(), ITEM_ANTIFLAG_STACK))
				{
#ifdef __ITEM_COUNT_LIMIT__
					DWORD bCount = item->GetCount();
#else
					BYTE bCount = item->GetCount();
#endif
					for (int i = 0; i < SPECIAL_INVENTORY_MAX_NUM; ++i)
					{
						LPITEM item2 = owner->GetFlowerInventoryItem(i);
						if (!item2)
							continue;
						if (item2->GetVnum() == item->GetVnum())
						{
#ifdef __ITEM_COUNT_LIMIT__
							DWORD bCount2 = MIN(g_bItemCountLimit - item2->GetCount(), bCount);
#else
							BYTE bCount2 = MIN(g_bItemCountLimit - item2->GetCount(), bCount);
#endif
							bCount -= bCount2;
							item2->SetCount(item2->GetCount() + bCount2);

							if (bCount == 0)
							{
								owner->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("FLOWER_INVENTORY_ADDED: %s"), item2->GetName());
								M2_DESTROY_ITEM(item);
								return true;
							}
						}
					}
					item->SetCount(bCount);
				}

				else if (item->IsAttrItem() && item->IsStackable() && !IS_SET(item->GetAntiFlag(), ITEM_ANTIFLAG_STACK))
				{
#ifdef __ITEM_COUNT_LIMIT__
					DWORD bCount = item->GetCount();
#else
					BYTE bCount = item->GetCount();
#endif
					for (int i = 0; i < SPECIAL_INVENTORY_MAX_NUM; ++i)
					{
						LPITEM item2 = owner->GetAttrInventoryItem(i);
						if (!item2)
							continue;
						if (item2->GetVnum() == item->GetVnum())
						{
#ifdef __ITEM_COUNT_LIMIT__
							DWORD bCount2 = MIN(g_bItemCountLimit - item2->GetCount(), bCount);
#else
							BYTE bCount2 = MIN(g_bItemCountLimit - item2->GetCount(), bCount);
#endif
							bCount -= bCount2;
							item2->SetCount(item2->GetCount() + bCount2);

							if (bCount == 0)
							{
								owner->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ATTR_INVENTORY_ADDED: %s"), item2->GetName());
								M2_DESTROY_ITEM(item);
								return true;
							}
						}
					}
					item->SetCount(bCount);
				}

				else if (item->IsChest() && item->IsStackable() && !IS_SET(item->GetAntiFlag(), ITEM_ANTIFLAG_STACK))
				{
#ifdef __ITEM_COUNT_LIMIT__
					DWORD bCount = item->GetCount();
#else
					BYTE bCount = item->GetCount();
#endif
					for (int i = 0; i < SPECIAL_INVENTORY_MAX_NUM; ++i)
					{
						LPITEM item2 = owner->GetChestInventoryItem(i);
						if (!item2)
							continue;
						if (item2->GetVnum() == item->GetVnum())
						{
#ifdef __ITEM_COUNT_LIMIT__
							DWORD bCount2 = MIN(g_bItemCountLimit - item2->GetCount(), bCount);
#else
							BYTE bCount2 = MIN(g_bItemCountLimit - item2->GetCount(), bCount);
#endif
							bCount -= bCount2;
							item2->SetCount(item2->GetCount() + bCount2);

							if (bCount == 0)
							{
								owner->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CHEST_INVENTORY_ADDED: %s"), item2->GetName());
								M2_DESTROY_ITEM(item);
								return true;
							}
						}
					}
					item->SetCount(bCount);
				}
				else
				{
					if (item->IsStackable() && !IS_SET(item->GetAntiFlag(), ITEM_ANTIFLAG_STACK))
					{
#ifdef __ITEM_COUNT_LIMIT__
						DWORD bCount = item->GetCount();
#else
						BYTE bCount = item->GetCount();
#endif
						for (int i = 0; i < INVENTORY_MAX_NUM; ++i)
						{
							LPITEM item2 = owner->GetInventoryItem(i);

							if (!item2)
								continue;

							if (item2->GetVnum() == item->GetVnum())
							{
								int j;

								for (j = 0; j < ITEM_SOCKET_MAX_NUM; ++j)
									if (item2->GetSocket(j) != item->GetSocket(j))
										break;

								if (j != ITEM_SOCKET_MAX_NUM)
									continue;
#ifdef __ITEM_COUNT_LIMIT__
								DWORD bCount2 = MIN(g_bItemCountLimit - item2->GetCount(), bCount);
#else
								BYTE bCount2 = MIN(g_bItemCountLimit - item2->GetCount(), bCount);
#endif
								bCount -= bCount2;
								item2->SetCount(item2->GetCount() + bCount2);

								if (bCount == 0)
								{
									owner->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("NORMAL_INVENTORY_ADDED: %s"), item2->GetName());
									M2_DESTROY_ITEM(item);
									if (item2->GetType() == ITEM_QUEST)
										quest::CQuestManager::instance().PickupItem(GetPlayerID(), item2);
									return true;
								}
							}
						}

						item->SetCount(bCount);
					}
				}
#else
				if (item->IsStackable() && !IS_SET(item->GetAntiFlag(), ITEM_ANTIFLAG_STACK))
				{
#ifdef __ITEM_COUNT_LIMIT__
					DWORD bCount = item->GetCount();
#else
					BYTE bCount = item->GetCount();
#endif
					for (int i = 0; i < INVENTORY_MAX_NUM; ++i)
					{
						LPITEM item2 = owner->GetInventoryItem(i);

						if (!item2)
							continue;

						if (item2->GetVnum() == item->GetVnum())
						{
							int j;

							for (j = 0; j < ITEM_SOCKET_MAX_NUM; ++j)
								if (item2->GetSocket(j) != item->GetSocket(j))
									break;

							if (j != ITEM_SOCKET_MAX_NUM)
								continue;
#ifdef __ITEM_COUNT_LIMIT__
							DWORD bCount2 = MIN(g_bItemCountLimit - item2->GetCount(), bCount);
#else
							BYTE bCount2 = MIN(g_bItemCountLimit - item2->GetCount(), bCount);
#endif
							bCount -= bCount2;
							item2->SetCount(item2->GetCount() + bCount2);

							if (bCount == 0)
							{
								owner->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¾ÆÀÌÅÛ È¹µæ: %s"), item2->GetName());
								M2_DESTROY_ITEM(item);
								if (item2->GetType() == ITEM_QUEST)
									quest::CQuestManager::instance().PickupItem(GetPlayerID(), item2);
								return true;
							}
						}
					}

					item->SetCount(bCount);
				}
#endif

				int iEmptyCell;
				if (item->IsDragonSoul())
				{
					if ((iEmptyCell = GetEmptyDragonSoulInventory(item)) == -1)
					{
						//sys_log(0, "No empty ds inventory pid %u size %ud itemid %u", GetPlayerID(), item->GetSize(), item->GetID());
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼ÒÁöÇÏ°í ÀÖ´Â ¾ÆÀÌÅÛÀÌ ³Ê¹« ¸¹½À´Ï´Ù."));
						return false;
					}
				}
#ifdef __ADDITIONAL_INVENTORY__
				else if (item->IsUpgradeItem())
				{
					if ((iEmptyCell = owner->GetEmptyUpgradeInventory(item)) == -1)
					{
						//sys_log(0, "No empty ssu inventory pid %u size %ud itemid %u", GetPlayerID(), item->GetSize(), item->GetID());
						owner->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼ÒÁöÇÏ°í ÀÖ´Â ¾ÆÀÌÅÛÀÌ ³Ê¹« ¸¹½À´Ï´Ù."));
						return false;
					}
				}
				else if (item->IsBook())
				{
					if ((iEmptyCell = owner->GetEmptyBookInventory(item)) == -1)
					{
						//sys_log(0, "No empty ssu inventory pid %u size %ud itemid %u", GetPlayerID(), item->GetSize(), item->GetID());
						owner->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼ÒÁöÇÏ°í ÀÖ´Â ¾ÆÀÌÅÛÀÌ ³Ê¹« ¸¹½À´Ï´Ù."));
						return false;
					}
				}
				else if (item->IsStone())
				{
					if ((iEmptyCell = owner->GetEmptyStoneInventory(item)) == -1)
					{
						//sys_log(0, "No empty ssu inventory pid %u size %ud itemid %u", GetPlayerID(), item->GetSize(), item->GetID());
						owner->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼ÒÁöÇÏ°í ÀÖ´Â ¾ÆÀÌÅÛÀÌ ³Ê¹« ¸¹½À´Ï´Ù."));
						return false;
					}
				}
				else if (item->IsFlower())
				{
					if ((iEmptyCell = owner->GetEmptyFlowerInventory(item)) == -1)
					{
						//sys_log(0, "No empty ssf inventory pid %u size %ud itemid %u", GetPlayerID(), item->GetSize(), item->GetID());
						owner->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼ÒÁöÇÏ°í ÀÖ´Â ¾ÆÀÌÅÛÀÌ ³Ê¹« ¸¹½À´Ï´Ù."));
						return false;
					}
				}
				else if (item->IsAttrItem())
				{
					if ((iEmptyCell = owner->GetEmptyAttrInventory(item)) == -1)
					{
						//sys_log(0, "No empty ssa inventory pid %u size %ud itemid %u", GetPlayerID(), item->GetSize(), item->GetID());
						owner->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼ÒÁöÇÏ°í ÀÖ´Â ¾ÆÀÌÅÛÀÌ ³Ê¹« ¸¹½À´Ï´Ù."));
						return false;
					}
				}
				else if (item->IsChest())
				{
					if ((iEmptyCell = owner->GetEmptyChestInventory(item)) == -1)
					{
						//sys_log(0, "No empty ssc inventory pid %u size %ud itemid %u", GetPlayerID(), item->GetSize(), item->GetID());
						owner->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼ÒÁöÇÏ°í ÀÖ´Â ¾ÆÀÌÅÛÀÌ ³Ê¹« ¸¹½À´Ï´Ù."));
						return false;
					}
				}
#endif
				else
				{
					if ((iEmptyCell = owner->GetEmptyInventory(item->GetSize())) == -1)
					{
						//sys_log(0, "No empty inventory pid %u size %ud itemid %u", GetPlayerID(), item->GetSize(), item->GetID());
						owner->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼ÒÁöÇÏ°í ÀÖ´Â ¾ÆÀÌÅÛÀÌ ³Ê¹« ¸¹½À´Ï´Ù."));
						return false;
					}
				}

				item->RemoveFromGround();

				if (item->IsDragonSoul())
					item->__ADD_TO_CHARACTER(owner, TItemPos(DRAGON_SOUL_INVENTORY, iEmptyCell));
#ifdef __ADDITIONAL_INVENTORY__
				else if (item->IsUpgradeItem())
					item->__ADD_TO_CHARACTER(owner, TItemPos(UPGRADE_INVENTORY, iEmptyCell));
				else if (item->IsBook())
					item->__ADD_TO_CHARACTER(owner, TItemPos(BOOK_INVENTORY, iEmptyCell));
				else if (item->IsStone())
					item->__ADD_TO_CHARACTER(owner, TItemPos(STONE_INVENTORY, iEmptyCell));
				else if (item->IsFlower())
					item->__ADD_TO_CHARACTER(owner, TItemPos(FLOWER_INVENTORY, iEmptyCell));
				else if (item->IsAttrItem())
					item->__ADD_TO_CHARACTER(owner, TItemPos(ATTR_INVENTORY, iEmptyCell));
				else if (item->IsChest())
					item->__ADD_TO_CHARACTER(owner, TItemPos(CHEST_INVENTORY, iEmptyCell));
#endif
				else
					item->__ADD_TO_CHARACTER(owner, TItemPos(INVENTORY, iEmptyCell));

				if (owner == this)
				{
					if (item->IsDragonSoul())
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("DRAGON_SOUL_INVENTORY_ADDED: %s"), item->GetName());
					else if (item->IsUpgradeItem())
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("UPGRADE_INVENTORY_ADDED: %s"), item->GetName());
					else if (item->IsBook())
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("BOOK_INVENTORY_ADDED: %s"), item->GetName());
					else if (item->IsStone())
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("STONE_INVENTORY_ADDED: %s"), item->GetName());
					else if (item->IsFlower())
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("FLOWER_INVENTORY_ADDED: %s"), item->GetName());
					else if (item->IsAttrItem())
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ATTR_INVENTORY_ADDED: %s"), item->GetName());
					else if (item->IsChest())
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CHEST_INVENTORY_ADDED: %s"), item->GetName());
					else
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("NORMAL_INVENTORY_ADDED: %s"), item->GetName());
				}
				else
				{
					owner->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¾ÆÀÌÅÛ È¹µæ: %s ´ÔÀ¸·ÎºÎÅÍ %s"), GetName(), item->GetName());
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¾ÆÀÌÅÛ Àü´Þ: %s ´Ô¿¡°Ô %s"), owner->GetName(), item->GetName());
				}

				if (item->GetType() == ITEM_QUEST)
					quest::CQuestManager::instance().PickupItem(owner->GetPlayerID(), item);

#ifdef __BATTLE_PASS_SYSTEM__
			if (bIsBattlePass)
			{
				BYTE bBattlePassId = owner->GetBattlePassId();
				if (bBattlePassId)
				{
					DWORD dwItemVnum, dwCount;
					if (CBattlePass::instance().BattlePassMissionGetInfo(bBattlePassId, COLLECT_ITEM, &dwItemVnum, &dwCount))
					{
						if (dwItemVnum == item->GetVnum() && owner->GetMissionProgress(COLLECT_ITEM, bBattlePassId) < dwCount)
							owner->UpdateMissionProgress(COLLECT_ITEM, bBattlePassId, item->GetCount(), dwCount);
					}
				}
			}
#endif

				return true;
			}
		}

	return false;
}

bool CHARACTER::SwapItem(BYTE bCell, BYTE bDestCell)
{
	if (!CanHandleItem())
		return false;

	TItemPos srcCell(INVENTORY, bCell), destCell(INVENTORY, bDestCell);

	//if (bCell >= INVENTORY_MAX_NUM + WEAR_MAX_NUM || bDestCell >= INVENTORY_MAX_NUM + WEAR_MAX_NUM)
	if (srcCell.IsDragonSoulEquipPosition() || destCell.IsDragonSoulEquipPosition())
		return false;

	if (bCell == bDestCell)
		return false;

	if (srcCell.IsEquipPosition() && destCell.IsEquipPosition())
		return false;

	LPITEM item1, item2;

	if (srcCell.IsEquipPosition())
	{
		item1 = GetInventoryItem(bDestCell);
		item2 = GetInventoryItem(bCell);
	}
	else
	{
		item1 = GetInventoryItem(bCell);
		item2 = GetInventoryItem(bDestCell);
	}

	if (!item1 || !item2)
		return false;

	if (item1 == item2)
	{
		sys_log(0, "[WARNING][WARNING][HACK USER!] : %s %d %d", m_stName.c_str(), bCell, bDestCell);
		return false;
	}

	if (!IsEmptyItemGrid(TItemPos(INVENTORY, item1->GetCell()), item2->GetSize(), item1->GetCell()))
		return false;

	if (TItemPos(EQUIPMENT, item2->GetCell()).IsEquipPosition())
	{
		BYTE bEquipCell = item2->GetCell() - INVENTORY_MAX_NUM;
		BYTE bInvenCell = item1->GetCell();

		if (!CanUnequipNow(item2, TItemPos(INVENTORY, bInvenCell)) || !CanEquipNow(item1))
			return false;

		if (bEquipCell != item1->FindEquipCell(this))
			return false;

		item2->RemoveFromCharacter();

		if (item1->EquipTo(this, bEquipCell))
			item2->__ADD_TO_CHARACTER(this, TItemPos(INVENTORY, bInvenCell));
		else
			sys_err("SwapItem cannot equip %s! item1 %s", item2->GetName(), item1->GetName());
	}
	else
	{
		BYTE bCell1 = item1->GetCell();
		BYTE bCell2 = item2->GetCell();

		item1->RemoveFromCharacter();
		item2->RemoveFromCharacter();

		item1->__ADD_TO_CHARACTER(this, TItemPos(INVENTORY, bCell2));
		item2->__ADD_TO_CHARACTER(this, TItemPos(INVENTORY, bCell1));
	}

	return true;
}

bool CHARACTER::UnequipItem(LPITEM item)
{
#ifdef __WEAPON_COSTUME_SYSTEM__
	int iWearCell = item->FindEquipCell(this);
	if (iWearCell == WEAR_WEAPON)
	{
		LPITEM costumeWeapon = GetWear(WEAR_COSTUME_WEAPON);
		if (costumeWeapon && !UnequipItem(costumeWeapon))
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You cannot unequip the costume weapon. Not enough space."));
			return false;
		}
	}
#endif

#ifdef __GROWTH_PET_SYSTEM__
	if (GetNewPetSystem()->IsActivePet() && (item->GetType() == ITEM_COSTUME && item->GetSubType() == COSTUME_ACCE))
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Once diger pet'i gonder."));
		return false;
	}
#endif

#ifdef __GROWTH_MOUNT_SYSTEM__
	if (GetNewMountSystem()->IsActiveMount() && (item->GetType() == ITEM_COSTUME && item->GetSubType() == COSTUME_ACCE))
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Once diger mount'i gonder."));
		return false;
	}
#endif

	if (false == CanUnequipNow(item))
		return false;

	int pos;
	if (item->IsDragonSoul())
		pos = GetEmptyDragonSoulInventory(item);
#ifdef __ADDITIONAL_INVENTORY__
	else if (item->IsUpgradeItem())
		pos = GetEmptyUpgradeInventory(item);
	else if (item->IsBook())
		pos = GetEmptyBookInventory(item);
	else if (item->IsStone())
		pos = GetEmptyStoneInventory(item);
	else if (item->IsFlower())
		pos = GetEmptyFlowerInventory(item);
	else if (item->IsAttrItem())
		pos = GetEmptyAttrInventory(item);
	else if (item->IsChest())
		pos = GetEmptyChestInventory(item);
#endif
	else
		pos = GetEmptyInventory(item->GetSize());

	// HARD CODING
	if (item->GetVnum() == UNIQUE_ITEM_HIDE_ALIGNMENT_TITLE)
		ShowAlignment(true);

	item->RemoveFromCharacter();
	if (item->IsDragonSoul())
	{
		item->__ADD_TO_CHARACTER(this, TItemPos(DRAGON_SOUL_INVENTORY, pos));
	}
#ifdef __ADDITIONAL_INVENTORY__
	else if (item->IsUpgradeItem())
		item->__ADD_TO_CHARACTER(this, TItemPos(UPGRADE_INVENTORY, pos));
	else if (item->IsBook())
		item->__ADD_TO_CHARACTER(this, TItemPos(BOOK_INVENTORY, pos));
	else if (item->IsStone())
		item->__ADD_TO_CHARACTER(this, TItemPos(STONE_INVENTORY, pos));
	else if (item->IsFlower())
		item->__ADD_TO_CHARACTER(this, TItemPos(FLOWER_INVENTORY, pos));
	else if (item->IsAttrItem())
		item->__ADD_TO_CHARACTER(this, TItemPos(ATTR_INVENTORY, pos));
	else if (item->IsChest())
		item->__ADD_TO_CHARACTER(this, TItemPos(CHEST_INVENTORY, pos));
#endif
	else
		item->__ADD_TO_CHARACTER(this, TItemPos(INVENTORY, pos));

	CheckMaximumPoints();
#ifdef __ITEM_SET_BONUS__
	CheckItemSetBonus();
#endif // __ITEM_SET_BONUS__

#ifdef __SHINING_ITEM_SYSTEM__
	if (item->GetType() == ITEM_SHINING)
		this->UpdatePacket();
#endif

	return true;
}

bool CHARACTER::EquipItem(LPITEM item, int iCandidateCell)
{
	if (item->IsExchanging())
		return false;

	if (false == item->IsEquipable())
		return false;

	if (false == CanEquipNow(item))
		return false;

	int iWearCell = item->FindEquipCell(this, iCandidateCell);

	if (iWearCell < 0)
		return false;

	if (iWearCell == WEAR_BODY && IsRiding() && (item->GetVnum() >= 11901 && item->GetVnum() <= 11914))
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¸»À» Åº »óÅÂ¿¡¼­ ¿¹º¹À» ÀÔÀ» ¼ö ¾ø½À´Ï´Ù."));
		return false;
	}

	if (FN_check_item_sex(this, item) == false)
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼ºº°ÀÌ ¸ÂÁö¾Ê¾Æ ÀÌ ¾ÆÀÌÅÛÀ» »ç¿ëÇÒ ¼ö ¾ø½À´Ï´Ù."));
		return false;
	}

	if (item->IsRideItem() && IsRiding())
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ¹Ì Å»°ÍÀ» ÀÌ¿ëÁßÀÔ´Ï´Ù."));
		return false;
	}

#ifdef __WEAPON_COSTUME_SYSTEM__
	if (iWearCell == WEAR_WEAPON)
	{
		if (item->GetType() == ITEM_WEAPON)
		{
			LPITEM costumeWeapon = GetWear(WEAR_COSTUME_WEAPON);
			if (costumeWeapon && costumeWeapon->GetValue(3) != item->GetSubType() && !UnequipItem(costumeWeapon))
			{
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You cannot unequip the costume weapon. Not enough space."));
				return false;
			}
		}
		else //fishrod/pickaxe
		{
			LPITEM costumeWeapon = GetWear(WEAR_COSTUME_WEAPON);
			if (costumeWeapon && !UnequipItem(costumeWeapon))
			{
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You cannot unequip the costume weapon. Not enough space."));
				return false;
			}
		}
	}
	else if (iWearCell == WEAR_COSTUME_WEAPON)
	{
		if (item->GetType() == ITEM_COSTUME && item->GetSubType() == COSTUME_WEAPON)
		{
			LPITEM pkWeapon = GetWear(WEAR_WEAPON);
			if (!pkWeapon || pkWeapon->GetType() != ITEM_WEAPON || item->GetValue(3) != pkWeapon->GetSubType())
			{
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You cannot equip the costume weapon. Wrong equipped weapon."));
				return false;
			}
		}
	}
#endif

	if (item->IsDragonSoul())
	{
		if (GetInventoryItem(INVENTORY_MAX_NUM + iWearCell))
		{
			ChatPacket(CHAT_TYPE_INFO, "ÀÌ¹Ì °°Àº Á¾·ùÀÇ ¿ëÈ¥¼®À» Âø¿ëÇÏ°í ÀÖ½À´Ï´Ù.");
			return false;
		}

		if (!item->EquipTo(this, iWearCell))
			return false;
	}

	else
	{
		if (GetWear(iWearCell) && !IS_SET(GetWear(iWearCell)->GetFlag(), ITEM_FLAG_IRREMOVABLE))
		{
			if (item->GetWearFlag() == WEARABLE_ABILITY)
				return false;

			if (false == SwapItem(item->GetCell(), INVENTORY_MAX_NUM + iWearCell))
				return false;
		}
		else
		{
			BYTE bOldCell = item->GetCell();

			if (item->EquipTo(this, iWearCell))
				SyncQuickslot(QUICKSLOT_TYPE_ITEM, bOldCell, iWearCell);
		}
	}

	if (true == item->IsEquipped())
	{
		if (-1 != item->GetProto()->cLimitRealTimeFirstUseIndex)
		{
			if (0 == item->GetSocket(1))
			{
				long duration = (0 != item->GetSocket(0)) ? item->GetSocket(0) : item->GetProto()->aLimits[(unsigned char)(item->GetProto()->cLimitRealTimeFirstUseIndex)].lValue;

				if (0 == duration)
					duration = 60 * 60 * 24 * 7;

				item->SetSocket(0, time(0) + duration);
				item->StartRealTimeExpireEvent();
			}

			item->SetSocket(1, item->GetSocket(1) + 1);
		}

		if (item->GetVnum() == UNIQUE_ITEM_HIDE_ALIGNMENT_TITLE)
			ShowAlignment(false);

		const DWORD& dwVnum = item->GetVnum();

		if (true == CItemVnumHelper::IsRamadanMoonRing(dwVnum))
			this->EffectPacket(SE_EQUIP_RAMADAN_RING);
		else if (true == CItemVnumHelper::IsHalloweenCandy(dwVnum))
			this->EffectPacket(SE_EQUIP_HALLOWEEN_CANDY);
		else if (true == CItemVnumHelper::IsHappinessRing(dwVnum))
			this->EffectPacket(SE_EQUIP_HAPPINESS_RING);
		else if (true == CItemVnumHelper::IsLovePendant(dwVnum))
			this->EffectPacket(SE_EQUIP_LOVE_PENDANT);
#ifdef __DRAGON_BONE_EFFECT__
		else if (item->GetVnum() == 72705)
			SpecificEffectPacket("d:/ymir work/effect/etc/buff/buff_tigerknochenarmband.mse");
		else if (item->GetVnum() == 72706)
			SpecificEffectPacket("d:/ymir work/effect/etc/buff/buff_drachenknochenarmband.mse");
		else if (item->GetVnum() == 72707)
			SpecificEffectPacket("d:/ymir work/effect/etc/buff/buff_tigerknochenarmband.mse");
		else if (item->GetVnum() == 72708)
			SpecificEffectPacket("d:/ymir work/effect/etc/buff/buff_drachenknochenarmband.mse");
		else if (item->GetVnum() == 72703)
			SpecificEffectPacket("d:/ymir work/effect/etc/buff/buff_tigerknochenohrring.mse");
		else if (item->GetVnum() == 72704)
			SpecificEffectPacket("d:/ymir work/effect/etc/buff/buff_drachenknochenohrring.mse");
		else if (item->GetVnum() == 67073)
			SpecificEffectPacket("d:/ymir work/effect/etc/buff/buff_tigerknochenarmband.mse");
		else if (item->GetVnum() == 67074)
			SpecificEffectPacket("d:/ymir work/effect/etc/buff/buff_drachenknochenarmband.mse");
		else if (item->GetVnum() == 68973)
			SpecificEffectPacket("d:/ymir work/effect/etc/buff/buff_tigerknochenarmband.mse");
		else if (item->GetVnum() == 68974)
			SpecificEffectPacket("d:/ymir work/effect/etc/buff/buff_drachenknochenarmband.mse");
		else if (item->GetVnum() == 72790)
			SpecificEffectPacket("d:/ymir work/effect/etc/buff/buff_tigerknochenarmband.mse");
		else if (item->GetVnum() == 72791)
			SpecificEffectPacket("d:/ymir work/effect/etc/buff/buff_drachenknochenarmband.mse");
		else if (item->GetVnum() == 77073)
			SpecificEffectPacket("d:/ymir work/effect/etc/buff/buff_tigerknochenarmband.mse");
		else if (item->GetVnum() == 77074)
			SpecificEffectPacket("d:/ymir work/effect/etc/buff/buff_drachenknochenarmband.mse");
		else if (item->GetVnum() == 78073)
			SpecificEffectPacket("d:/ymir work/effect/etc/buff/buff_tigerknochenarmband.mse");
		else if (item->GetVnum() == 78074)
			SpecificEffectPacket("d:/ymir work/effect/etc/buff/buff_drachenknochenarmband.mse");
		else if (item->GetVnum() == 88373)
			SpecificEffectPacket("d:/ymir work/effect/etc/buff/buff_tigerknochenarmband.mse");
		else if (item->GetVnum() == 88374)
			SpecificEffectPacket("d:/ymir work/effect/etc/buff/buff_drachenknochenarmband.mse");
		else if (item->GetVnum() == 88448)
			SpecificEffectPacket("d:/ymir work/effect/etc/buff/buff_tigerknochenarmband.mse");
		else if (item->GetVnum() == 88449)
			SpecificEffectPacket("d:/ymir work/effect/etc/buff/buff_drachenknochenarmband.mse");
		else if (item->GetVnum() == 88523)
			SpecificEffectPacket("d:/ymir work/effect/etc/buff/buff_tigerknochenarmband.mse");
		else if (item->GetVnum() == 88524)
			SpecificEffectPacket("d:/ymir work/effect/etc/buff/buff_drachenknochenarmband.mse");
		else if (item->GetVnum() == 88598)
			SpecificEffectPacket("d:/ymir work/effect/etc/buff/buff_tigerknochenarmband.mse");
		else if (item->GetVnum() == 88599)
			SpecificEffectPacket("d:/ymir work/effect/etc/buff/buff_drachenknochenarmband.mse");
		else if (item->GetVnum() == 88673)
			SpecificEffectPacket("d:/ymir work/effect/etc/buff/buff_tigerknochenarmband.mse");
		else if (item->GetVnum() == 88674)
			SpecificEffectPacket("d:/ymir work/effect/etc/buff/buff_drachenknochenarmband.mse");
#endif
#ifdef __UNIQUE_ITEM_EFFECT__
		else if (item->GetVnum() == UNIQUE_ITEM_DOUBLE_EXP)
			SpecificEffectPacket("d:/ymir work/effect/etc/buff/buff_expring.mse");
		else if (item->GetVnum() == UNIQUE_ITEM_DOUBLE_ITEM)
			SpecificEffectPacket("d:/ymir work/effect/etc/buff/buff_handschuh.mse");
#endif
		else if (ITEM_UNIQUE == item->GetType() && 0 != item->GetSIGVnum())
		{
			const CSpecialItemGroup* pGroup = ITEM_MANAGER::instance().GetSpecialItemGroup(item->GetSIGVnum());
			if (NULL != pGroup)
			{
				const CSpecialAttrGroup* pAttrGroup = ITEM_MANAGER::instance().GetSpecialAttrGroup(pGroup->GetAttrVnum(item->GetVnum()));
				if (NULL != pAttrGroup)
				{
					const std::string& std = pAttrGroup->m_stEffectFileName;
					SpecificEffectPacket(std.c_str());
				}
			}
		}
		else if (ITEM_RING == item->GetType() && 0 != item->GetSIGVnum())
		{
			const CSpecialItemGroup* pGroup = ITEM_MANAGER::instance().GetSpecialItemGroup(item->GetSIGVnum());
			if (NULL != pGroup)
			{
				const CSpecialAttrGroup* pAttrGroup = ITEM_MANAGER::instance().GetSpecialAttrGroup(pGroup->GetAttrVnum(item->GetVnum()));
				if (NULL != pAttrGroup)
				{
					const std::string& std = pAttrGroup->m_stEffectFileName;
					SpecificEffectPacket(std.c_str());
				}
			}
		}
#ifdef __ACCE_SYSTEM__
		else if ((item->GetType() == ITEM_COSTUME) && (item->GetSubType() == COSTUME_ACCE))
			this->EffectPacket(SE_EFFECT_ACCE_EQUIP);
#endif
		if ((ITEM_UNIQUE == item->GetType() && UNIQUE_SPECIAL_RIDE == item->GetSubType() && IS_SET(item->GetFlag(), ITEM_FLAG_QUEST_USE))
			|| (ITEM_UNIQUE == item->GetType() && UNIQUE_SPECIAL_MOUNT_RIDE == item->GetSubType() && IS_SET(item->GetFlag(), ITEM_FLAG_QUEST_USE)))
		{
			quest::CQuestManager::instance().UseItem(GetPlayerID(), item, false);
		}
	}

#ifdef __ITEM_SET_BONUS__
	CheckItemSetBonus();
#endif // __ITEM_SET_BONUS__

#ifdef __SHINING_ITEM_SYSTEM__
	if (item->GetType() == ITEM_SHINING)
		this->UpdatePacket();
#endif
	return true;
}

void CHARACTER::BuffOnAttr_AddBuffsFromItem(LPITEM pItem)
{
	for (size_t i = 0; i < sizeof(g_aBuffOnAttrPoints) / sizeof(g_aBuffOnAttrPoints[0]); i++)
	{
		TMapBuffOnAttrs::iterator it = m_map_buff_on_attrs.find(g_aBuffOnAttrPoints[i]);
		if (it != m_map_buff_on_attrs.end())
		{
			it->second->AddBuffFromItem(pItem);
		}
	}
}

void CHARACTER::BuffOnAttr_RemoveBuffsFromItem(LPITEM pItem)
{
	for (size_t i = 0; i < sizeof(g_aBuffOnAttrPoints) / sizeof(g_aBuffOnAttrPoints[0]); i++)
	{
		TMapBuffOnAttrs::iterator it = m_map_buff_on_attrs.find(g_aBuffOnAttrPoints[i]);
		if (it != m_map_buff_on_attrs.end())
		{
			it->second->RemoveBuffFromItem(pItem);
		}
	}
}

void CHARACTER::BuffOnAttr_ClearAll()
{
	for (TMapBuffOnAttrs::iterator it = m_map_buff_on_attrs.begin(); it != m_map_buff_on_attrs.end(); it++)
	{
		CBuffOnAttributes* pBuff = it->second;
		if (pBuff)
		{
			pBuff->Initialize();
		}
	}
}

void CHARACTER::BuffOnAttr_ValueChange(BYTE bType, BYTE bOldValue, BYTE bNewValue)
{
	TMapBuffOnAttrs::iterator it = m_map_buff_on_attrs.find(bType);

	if (0 == bNewValue)
	{
		if (m_map_buff_on_attrs.end() == it)
			return;
		else
			it->second->Off();
	}
	else if (0 == bOldValue)
	{
		CBuffOnAttributes* pBuff = NULL;
		if (m_map_buff_on_attrs.end() == it)
		{
			switch (bType)
			{
			case POINT_ENERGY:
			{
				static BYTE abSlot[] =
				{
					WEAR_BODY,
					WEAR_HEAD,
					WEAR_FOOTS,
					WEAR_WRIST,
					WEAR_WEAPON,
					WEAR_NECK,
					WEAR_EAR,
					WEAR_SHIELD,
					WEAR_RING1,
					WEAR_RING2,
					WEAR_BELT,
#ifdef __PENDANT_SYSTEM__
					WEAR_PENDANT,
#endif
				};
				static std::vector <BYTE> vec_slots(abSlot, abSlot + _countof(abSlot));
				pBuff = M2_NEW CBuffOnAttributes(this, bType, &vec_slots);
			}
			break;
			case POINT_COSTUME_ATTR_BONUS:
			{
				static BYTE abSlot[] = {
					WEAR_COSTUME_BODY,
					WEAR_COSTUME_HAIR,
#ifdef __MOUNT_COSTUME_SYSTEM__
					WEAR_COSTUME_MOUNT,
#endif
#ifdef __ACCE_SYSTEM__
					WEAR_COSTUME_ACCE,
#endif
#ifdef __WEAPON_COSTUME_SYSTEM__
					WEAR_COSTUME_WEAPON,
#endif
#ifdef __AURA_COSTUME_SYSTEM__
					WEAR_COSTUME_AURA,
#endif
				};
				static std::vector <BYTE> vec_slots(abSlot, abSlot + _countof(abSlot));
				pBuff = M2_NEW CBuffOnAttributes(this, bType, &vec_slots);
			}
			break;
			default:
				return;
			}
			m_map_buff_on_attrs.insert(TMapBuffOnAttrs::value_type(bType, pBuff));
		}
		else
			pBuff = it->second;
		if (pBuff != NULL)
			pBuff->On(bNewValue);
	}
	else
	{
		assert(m_map_buff_on_attrs.end() != it);
		if (it->second)
			it->second->ChangeBuffValue(bNewValue);
	}
}

LPITEM CHARACTER::FindSpecifyItem(DWORD vnum) const
{
	for (int i = 0; i < INVENTORY_MAX_NUM; ++i)
		if (GetInventoryItem(i) && GetInventoryItem(i)->GetVnum() == vnum)
			return GetInventoryItem(i);
	for (int i = 0; i < DRAGON_SOUL_INVENTORY_MAX_NUM; ++i)
		if (GetDragonSoulInventoryItem(i) && GetDragonSoulInventoryItem(i)->GetVnum() == vnum)
			return GetDragonSoulInventoryItem(i);
#ifdef __ADDITIONAL_INVENTORY__
	for (int i = 0; i < SPECIAL_INVENTORY_MAX_NUM; ++i)
		if (GetUpgradeInventoryItem(i) && GetUpgradeInventoryItem(i)->GetVnum() == vnum)
			return GetUpgradeInventoryItem(i);
	for (int i = 0; i < SPECIAL_INVENTORY_MAX_NUM; ++i)
		if (GetBookInventoryItem(i) && GetBookInventoryItem(i)->GetVnum() == vnum)
			return GetBookInventoryItem(i);
	for (int i = 0; i < SPECIAL_INVENTORY_MAX_NUM; ++i)
		if (GetStoneInventoryItem(i) && GetStoneInventoryItem(i)->GetVnum() == vnum)
			return GetStoneInventoryItem(i);
	for (int i = 0; i < SPECIAL_INVENTORY_MAX_NUM; ++i)
		if (GetFlowerInventoryItem(i) && GetFlowerInventoryItem(i)->GetVnum() == vnum)
			return GetFlowerInventoryItem(i);
	for (int i = 0; i < SPECIAL_INVENTORY_MAX_NUM; ++i)
		if (GetAttrInventoryItem(i) && GetAttrInventoryItem(i)->GetVnum() == vnum)
			return GetAttrInventoryItem(i);
	for (int i = 0; i < SPECIAL_INVENTORY_MAX_NUM; ++i)
		if (GetChestInventoryItem(i) && GetChestInventoryItem(i)->GetVnum() == vnum)
			return GetChestInventoryItem(i);
#endif

	return NULL;
}

#ifdef __ADDITIONAL_INVENTORY__
LPITEM CHARACTER::FindSpecifyItemSpecial(DWORD vnum) const
{
	for (int i = 0; i < SPECIAL_INVENTORY_MAX_NUM; ++i)
	{
		if (GetUpgradeInventoryItem(i) && GetUpgradeInventoryItem(i)->GetVnum() == vnum)
			return GetUpgradeInventoryItem(i);
		if (GetBookInventoryItem(i) && GetBookInventoryItem(i)->GetVnum() == vnum)
			return GetBookInventoryItem(i);
		if (GetStoneInventoryItem(i) && GetStoneInventoryItem(i)->GetVnum() == vnum)
			return GetStoneInventoryItem(i);
		if (GetFlowerInventoryItem(i) && GetFlowerInventoryItem(i)->GetVnum() == vnum)
			return GetFlowerInventoryItem(i);
		if (GetAttrInventoryItem(i) && GetAttrInventoryItem(i)->GetVnum() == vnum)
			return GetAttrInventoryItem(i);
		if (GetChestInventoryItem(i) && GetChestInventoryItem(i)->GetVnum() == vnum)
			return GetChestInventoryItem(i);
	}
	for (int i = 0; i < INVENTORY_MAX_NUM; ++i)
	{
		if (GetInventoryItem(i) && GetInventoryItem(i)->GetVnum() == vnum)
			return GetInventoryItem(i);
	}
	for (int i = 0; i < DRAGON_SOUL_INVENTORY_MAX_NUM; ++i)
	{
		if (GetDragonSoulInventoryItem(i) && GetDragonSoulInventoryItem(i)->GetVnum() == vnum)
			return GetDragonSoulInventoryItem(i);
	}
	return NULL;
}
#endif

LPITEM CHARACTER::FindItemByID(DWORD id) const
{
	for (int i = 0; i < INVENTORY_MAX_NUM; ++i)
	{
		if (NULL != GetInventoryItem(i) && GetInventoryItem(i)->GetID() == id)
			return GetInventoryItem(i);
	}
	for (int i = 0; i < DRAGON_SOUL_INVENTORY_MAX_NUM; ++i)
	{
		if (NULL != GetDragonSoulInventoryItem(i) && GetDragonSoulInventoryItem(i)->GetID() == id)
			return GetDragonSoulInventoryItem(i);
	}

#ifdef __ADDITIONAL_INVENTORY__
	for (int i=0 ; i < SPECIAL_INVENTORY_MAX_NUM ; ++i)
	{
		if (NULL != GetUpgradeInventoryItem(i) && GetUpgradeInventoryItem(i)->GetID() == id)
			return GetUpgradeInventoryItem(i);
	}
	for (int i=0 ; i < SPECIAL_INVENTORY_MAX_NUM ; ++i)
	{
		if (NULL != GetBookInventoryItem(i) && GetBookInventoryItem(i)->GetID() == id)
			return GetBookInventoryItem(i);
	}
	for (int i=0 ; i < SPECIAL_INVENTORY_MAX_NUM ; ++i)
	{
		if (NULL != GetStoneInventoryItem(i) && GetStoneInventoryItem(i)->GetID() == id)
			return GetStoneInventoryItem(i);
	}
	for (int i=0 ; i < SPECIAL_INVENTORY_MAX_NUM ; ++i)
	{
		if (NULL != GetFlowerInventoryItem(i) && GetFlowerInventoryItem(i)->GetID() == id)
			return GetFlowerInventoryItem(i);
	}
	for (int i=0 ; i < SPECIAL_INVENTORY_MAX_NUM ; ++i)
	{
		if (NULL != GetAttrInventoryItem(i) && GetAttrInventoryItem(i)->GetID() == id)
			return GetAttrInventoryItem(i);
	}
	for (int i=0 ; i < SPECIAL_INVENTORY_MAX_NUM ; ++i)
	{
		if (NULL != GetChestInventoryItem(i) && GetChestInventoryItem(i)->GetID() == id)
			return GetChestInventoryItem(i);
	}
#endif

	for (int i = BELT_INVENTORY_SLOT_START; i < BELT_INVENTORY_SLOT_END; ++i)
	{
		if (NULL != GetInventoryItem(i) && GetInventoryItem(i)->GetID() == id)
			return GetInventoryItem(i);
	}

	return NULL;
}

int CHARACTER::CountSpecifyItem(DWORD vnum) const
{
	int	count = 0;
	LPITEM item;

	for (int i = 0; i < INVENTORY_MAX_NUM; ++i)
	{
		item = GetInventoryItem(i);
		if (NULL != item && item->GetVnum() == vnum)
		{
			if (m_pkMyShop && m_pkMyShop->IsSellingItem(item->GetID()))
			{
				continue;
			}
			else
			{
				count += item->GetCount();
			}
		}
	}
	for (int i = 0; i < DRAGON_SOUL_INVENTORY_MAX_NUM; ++i)
	{
		item = GetDragonSoulInventoryItem(i);
		if (NULL != item && item->GetVnum() == vnum)
		{
			if (m_pkMyShop && m_pkMyShop->IsSellingItem(item->GetID()))
			{
				continue;
			}
			else
			{
				count += item->GetCount();
			}
		}
	}
#ifdef __ADDITIONAL_INVENTORY__
	for (int i = 0; i < SPECIAL_INVENTORY_MAX_NUM; ++i)
	{
		item = GetUpgradeInventoryItem(i);
		if (NULL != item && item->GetVnum() == vnum)
		{
			if (m_pkMyShop && m_pkMyShop->IsSellingItem(item->GetID()))
				continue;
			else
				count += item->GetCount();
		}
	}
	for (int i = 0; i < SPECIAL_INVENTORY_MAX_NUM; ++i)
	{
		item = GetBookInventoryItem(i);
		if (NULL != item && item->GetVnum() == vnum)
		{
			if (m_pkMyShop && m_pkMyShop->IsSellingItem(item->GetID()))
				continue;
			else
				count += item->GetCount();
		}
	}
	for (int i = 0; i < SPECIAL_INVENTORY_MAX_NUM; ++i)
	{
		item = GetStoneInventoryItem(i);
		if (NULL != item && item->GetVnum() == vnum)
		{
			if (m_pkMyShop && m_pkMyShop->IsSellingItem(item->GetID()))
				continue;
			else
				count += item->GetCount();
		}
	}
	for (int i = 0; i < SPECIAL_INVENTORY_MAX_NUM; ++i)
	{
		item = GetFlowerInventoryItem(i);
		if (NULL != item && item->GetVnum() == vnum)
		{
			if (m_pkMyShop && m_pkMyShop->IsSellingItem(item->GetID()))
				continue;
			else
				count += item->GetCount();
		}
	}
	for (int i = 0; i < SPECIAL_INVENTORY_MAX_NUM; ++i)
	{
		item = GetAttrInventoryItem(i);
		if (NULL != item && item->GetVnum() == vnum)
		{
			if (m_pkMyShop && m_pkMyShop->IsSellingItem(item->GetID()))
				continue;
			else
				count += item->GetCount();
		}
	}
	for (int i = 0; i < SPECIAL_INVENTORY_MAX_NUM; ++i)
	{
		item = GetChestInventoryItem(i);
		if (NULL != item && item->GetVnum() == vnum)
		{
			if (m_pkMyShop && m_pkMyShop->IsSellingItem(item->GetID()))
				continue;
			else
				count += item->GetCount();
		}
	}
#endif
	return count;
}

void CHARACTER::RemoveSpecifyItem(DWORD vnum, DWORD count)
{
	if (0 == count)
		return;

	for (UINT i = 0; i < INVENTORY_MAX_NUM; ++i)
	{
		if (NULL == GetInventoryItem(i))
			continue;

		if (GetInventoryItem(i)->GetVnum() != vnum)
			continue;

		if (m_pkMyShop)
		{
			bool isItemSelling = m_pkMyShop->IsSellingItem(GetInventoryItem(i)->GetID());
			if (isItemSelling)
				continue;
		}

		if (count >= GetInventoryItem(i)->GetCount())
		{
			count -= GetInventoryItem(i)->GetCount();
			GetInventoryItem(i)->SetCount(0);

			if (0 == count)
				return;
		}
		else
		{
			GetInventoryItem(i)->SetCount(GetInventoryItem(i)->GetCount() - count);
			return;
		}
	}
	for (UINT i = 0; i < DRAGON_SOUL_INVENTORY_MAX_NUM; ++i)
	{
		if (NULL == GetDragonSoulInventoryItem(i))
			continue;

		if (GetDragonSoulInventoryItem(i)->GetVnum() != vnum)
			continue;

		if (m_pkMyShop)
		{
			bool isItemSelling = m_pkMyShop->IsSellingItem(GetDragonSoulInventoryItem(i)->GetID());
			if (isItemSelling)
				continue;
		}

		if (count >= GetDragonSoulInventoryItem(i)->GetCount())
		{
			count -= GetDragonSoulInventoryItem(i)->GetCount();
			GetDragonSoulInventoryItem(i)->SetCount(0);

			if (0 == count)
				return;
		}
		else
		{
			GetDragonSoulInventoryItem(i)->SetCount(GetDragonSoulInventoryItem(i)->GetCount() - count);
			return;
		}
	}
#ifdef __ADDITIONAL_INVENTORY__
	for (UINT i = 0; i < SPECIAL_INVENTORY_MAX_NUM; ++i)
	{
		if (NULL == GetUpgradeInventoryItem(i))
			continue;

		if (GetUpgradeInventoryItem(i)->GetVnum() != vnum)
			continue;
		if (m_pkMyShop)
		{
			bool isItemSelling = m_pkMyShop->IsSellingItem(GetUpgradeInventoryItem(i)->GetID());
			if (isItemSelling)
				continue;
		}
		if (count >= GetUpgradeInventoryItem(i)->GetCount())
		{
			count -= GetUpgradeInventoryItem(i)->GetCount();
			GetUpgradeInventoryItem(i)->SetCount(0);
			if (0 == count)
				return;
		}
		else
		{
			GetUpgradeInventoryItem(i)->SetCount(GetUpgradeInventoryItem(i)->GetCount() - count);
			return;
		}
	}
	for (UINT i = 0; i < SPECIAL_INVENTORY_MAX_NUM; ++i)
	{
		if (NULL == GetBookInventoryItem(i))
			continue;
		if (GetBookInventoryItem(i)->GetVnum() != vnum)
			continue;
		if (m_pkMyShop)
		{
			bool isItemSelling = m_pkMyShop->IsSellingItem(GetBookInventoryItem(i)->GetID());
			if (isItemSelling)
				continue;
		}
		if (count >= GetBookInventoryItem(i)->GetCount())
		{
			count -= GetBookInventoryItem(i)->GetCount();
			GetBookInventoryItem(i)->SetCount(0);
			if (0 == count)
				return;
		}
		else
		{
			GetBookInventoryItem(i)->SetCount(GetBookInventoryItem(i)->GetCount() - count);
			return;
		}
	}
	for (UINT i = 0; i < SPECIAL_INVENTORY_MAX_NUM; ++i)
	{
		if (NULL == GetStoneInventoryItem(i))
			continue;
		if (GetStoneInventoryItem(i)->GetVnum() != vnum)
			continue;
		if (m_pkMyShop)
		{
			bool isItemSelling = m_pkMyShop->IsSellingItem(GetStoneInventoryItem(i)->GetID());
			if (isItemSelling)
				continue;
		}
		if (count >= GetStoneInventoryItem(i)->GetCount())
		{
			count -= GetStoneInventoryItem(i)->GetCount();
			GetStoneInventoryItem(i)->SetCount(0);
			if (0 == count)
				return;
		}
		else
		{
			GetStoneInventoryItem(i)->SetCount(GetStoneInventoryItem(i)->GetCount() - count);
			return;
		}
	}

	for (UINT i = 0; i < SPECIAL_INVENTORY_MAX_NUM; ++i)
	{
		if (NULL == GetFlowerInventoryItem(i))
			continue;
		if (GetFlowerInventoryItem(i)->GetVnum() != vnum)
			continue;
		if (m_pkMyShop)
		{
			bool isItemSelling = m_pkMyShop->IsSellingItem(GetFlowerInventoryItem(i)->GetID());
			if (isItemSelling)
				continue;
		}
		if (count >= GetFlowerInventoryItem(i)->GetCount())
		{
			count -= GetFlowerInventoryItem(i)->GetCount();
			GetFlowerInventoryItem(i)->SetCount(0);
			if (0 == count)
				return;
		}
		else
		{
			GetFlowerInventoryItem(i)->SetCount(GetFlowerInventoryItem(i)->GetCount() - count);
			return;
		}
	}

	for (UINT i = 0; i < SPECIAL_INVENTORY_MAX_NUM; ++i)
	{
		if (NULL == GetAttrInventoryItem(i))
			continue;
		if (GetAttrInventoryItem(i)->GetVnum() != vnum)
			continue;
		if (m_pkMyShop)
		{
			bool isItemSelling = m_pkMyShop->IsSellingItem(GetAttrInventoryItem(i)->GetID());
			if (isItemSelling)
				continue;
		}
		if (count >= GetAttrInventoryItem(i)->GetCount())
		{
			count -= GetAttrInventoryItem(i)->GetCount();
			GetAttrInventoryItem(i)->SetCount(0);
			if (0 == count)
				return;
		}
		else
		{
			GetAttrInventoryItem(i)->SetCount(GetAttrInventoryItem(i)->GetCount() - count);
			return;
		}
	}

	for (UINT i = 0; i < SPECIAL_INVENTORY_MAX_NUM; ++i)
	{
		if (NULL == GetChestInventoryItem(i))
			continue;
		if (GetChestInventoryItem(i)->GetVnum() != vnum)
			continue;
		if (m_pkMyShop)
		{
			bool isItemSelling = m_pkMyShop->IsSellingItem(GetChestInventoryItem(i)->GetID());
			if (isItemSelling)
				continue;
		}
		if (count >= GetChestInventoryItem(i)->GetCount())
		{
			count -= GetChestInventoryItem(i)->GetCount();
			GetChestInventoryItem(i)->SetCount(0);
			if (0 == count)
				return;
		}
		else
		{
			GetChestInventoryItem(i)->SetCount(GetChestInventoryItem(i)->GetCount() - count);
			return;
		}
	}
#endif
	if (count)
		sys_log(0, "CHARACTER::RemoveSpecifyItem cannot remove enough item vnum %u, still remain %d", vnum, count);
}

int CHARACTER::CountSpecifyTypeItem(BYTE type) const
{
	int	count = 0;

	for (int i = 0; i < INVENTORY_MAX_NUM; ++i)
	{
		LPITEM pItem = GetInventoryItem(i);
		if (pItem != NULL && pItem->GetType() == type)
		{
			count += pItem->GetCount();
		}
	}

	for (int i = 0; i < DRAGON_SOUL_INVENTORY_MAX_NUM; ++i)
	{
		LPITEM pItem = GetDragonSoulInventoryItem(i);
		if (pItem != NULL && pItem->GetType() == type)
		{
			count += pItem->GetCount();
		}
	}

	return count;
}

void CHARACTER::RemoveSpecifyTypeItem(BYTE type, DWORD count)
{
	if (0 == count)
		return;

	for (UINT i = 0; i < INVENTORY_MAX_NUM; ++i)
	{
		if (NULL == GetInventoryItem(i))
			continue;

		if (GetInventoryItem(i)->GetType() != type)
			continue;

		if (m_pkMyShop)
		{
			bool isItemSelling = m_pkMyShop->IsSellingItem(GetInventoryItem(i)->GetID());
			if (isItemSelling)
				continue;
		}

		if (count >= GetInventoryItem(i)->GetCount())
		{
			count -= GetInventoryItem(i)->GetCount();
			GetInventoryItem(i)->SetCount(0);

			if (0 == count)
				return;
		}
		else
		{
			GetInventoryItem(i)->SetCount(GetInventoryItem(i)->GetCount() - count);
			return;
		}
	}

	for (UINT i = 0; i < DRAGON_SOUL_INVENTORY_MAX_NUM; ++i)
	{
		if (NULL == GetDragonSoulInventoryItem(i))
			continue;

		if (GetDragonSoulInventoryItem(i)->GetType() != type)
			continue;

		if (m_pkMyShop)
		{
			bool isItemSelling = m_pkMyShop->IsSellingItem(GetDragonSoulInventoryItem(i)->GetID());
			if (isItemSelling)
				continue;
		}

		if (count >= GetDragonSoulInventoryItem(i)->GetCount())
		{
			count -= GetDragonSoulInventoryItem(i)->GetCount();
			GetDragonSoulInventoryItem(i)->SetCount(0);

			if (0 == count)
				return;
		}
		else
		{
			GetDragonSoulInventoryItem(i)->SetCount(GetDragonSoulInventoryItem(i)->GetCount() - count);
			return;
		}
	}
}

void CHARACTER::AutoGiveItem(LPITEM item, bool longOwnerShip)
{
	if (NULL == item)
	{
		sys_err("NULL point.");
		return;
	}
	if (item->GetOwner())
	{
		sys_err("item %d 's owner exists!", item->GetID());
		return;
	}

	int cell;
	if (item->IsDragonSoul())
		cell = GetEmptyDragonSoulInventory(item);
#ifdef __ADDITIONAL_INVENTORY__
	else if (item->IsUpgradeItem())
		cell = GetEmptyUpgradeInventory(item);
	else if (item->IsBook())
		cell = GetEmptyBookInventory(item);
	else if (item->IsStone())
		cell = GetEmptyStoneInventory(item);
	else if (item->IsFlower())
		cell = GetEmptyFlowerInventory(item);
	else if (item->IsAttrItem())
		cell = GetEmptyAttrInventory(item);
	else if (item->IsChest())
		cell = GetEmptyChestInventory(item);
#endif
	else
		cell = GetEmptyInventory(item->GetSize());

	if (cell != -1)
	{
		if (item->IsDragonSoul())
			item->__ADD_TO_CHARACTER(this, TItemPos(DRAGON_SOUL_INVENTORY, cell));
#ifdef __ADDITIONAL_INVENTORY__
		else if (item->IsUpgradeItem())
			item->__ADD_TO_CHARACTER(this, TItemPos(UPGRADE_INVENTORY, cell));
		else if (item->IsBook())
			item->__ADD_TO_CHARACTER(this, TItemPos(BOOK_INVENTORY, cell));
		else if (item->IsStone())
			item->__ADD_TO_CHARACTER(this, TItemPos(STONE_INVENTORY, cell));
		else if (item->IsFlower())
			item->__ADD_TO_CHARACTER(this, TItemPos(FLOWER_INVENTORY, cell));
		else if (item->IsAttrItem())
			item->__ADD_TO_CHARACTER(this, TItemPos(ATTR_INVENTORY, cell));
		else if (item->IsChest())
			item->__ADD_TO_CHARACTER(this, TItemPos(CHEST_INVENTORY, cell));
#endif
		else
			item->__ADD_TO_CHARACTER(this, TItemPos(INVENTORY, cell));

		if (item->GetType() == ITEM_USE && item->GetSubType() == USE_POTION)
		{
			TQuickslot* pSlot;

			if (GetQuickslot(0, &pSlot) && pSlot->type == QUICKSLOT_TYPE_NONE)
			{
				TQuickslot slot;
				slot.type = QUICKSLOT_TYPE_ITEM;
				slot.pos = cell;
				SetQuickslot(0, slot);
			}
		}
	}
	else
	{
		item->AddToGround(GetMapIndex(), GetXYZ());
		item->StartDestroyEvent(g_aiItemDestroyTime[ITEM_DESTROY_TIME_AUTOGIVE]);

		if (longOwnerShip)
			item->SetOwnership(this, 300);
		else
			item->SetOwnership(this, 60);
	}
}

#ifdef __ADDITIONAL_INVENTORY__
static bool IsUpgradeItem(DWORD dwVnum)
{
	switch (dwVnum)
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

static bool IsSpecialBook(DWORD dwVnum, BYTE type)
{
	if (type == ITEM_SKILLBOOK)
		return true;

	switch (dwVnum)
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

static bool IsStoneSpecial(DWORD dwVnum, BYTE type)
{
	if (type == ITEM_METIN)
		return true;

	return false;
}

static bool IsFlowerItem(DWORD dwVnum)
{
	switch (dwVnum)
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

static bool IsAttrSpecial(DWORD dwVnum)
{
	switch (dwVnum)
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

static bool IsChestItem(DWORD dwVnum)
{
	switch (dwVnum)
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

#ifdef __ITEM_COUNT_LIMIT__
LPITEM CHARACTER::AutoGiveItem(DWORD dwItemVnum, DWORD bCount, int iRarePct, bool bMsg)
#else
LPITEM CHARACTER::AutoGiveItem(DWORD dwItemVnum, BYTE bCount, int iRarePct, bool bMsg)
#endif
{
	TItemTable* p = ITEM_MANAGER::instance().GetTable(dwItemVnum);

	if (!p)
		return NULL;

#ifdef __ADDITIONAL_INVENTORY__
	if (p->bType != ITEM_BLEND && !IsSpecialBook(dwItemVnum, p->bType))
#else
	if (p->bType != ITEM_BLEND)
#endif
	{
		if ((IS_SET(p->dwFlags,ITEM_FLAG_STACKABLE)) && !IS_SET(p->dwAntiFlags, ITEM_ANTIFLAG_STACK))
		{
#ifdef __ADDITIONAL_INVENTORY__
			if (IsUpgradeItem(dwItemVnum))
			{
				for (int i = 0; i < SPECIAL_INVENTORY_MAX_NUM; ++i)
				{
					LPITEM item = GetUpgradeInventoryItem(i);
					if (!item)
						continue;

					if (item->GetVnum() == dwItemVnum && FN_check_item_socket(item))
					{
#ifdef __ITEM_COUNT_LIMIT__
						DWORD bCount2 = MIN(g_bItemCountLimit - item->GetCount(), bCount);
#else
						BYTE bCount2 = MIN(g_bItemCountLimit - item->GetCount(), bCount);
#endif
						bCount -= bCount2;
						item->SetCount(item->GetCount() + bCount2);

						if (bCount == 0)
						{
							if (bMsg)
								ChatPacket(CHAT_TYPE_INFO, LC_TEXT("UPGRADE_INVENTORY_ADDED: %s"), item->GetName());
							return item;
						}
					}
				}
			}
			else if (IsStoneSpecial(dwItemVnum, p->bType))
			{
				for (int i = 0; i < SPECIAL_INVENTORY_MAX_NUM; ++i)
				{
					LPITEM item = GetStoneInventoryItem(i);
					if (!item)
						continue;

					if (item->GetVnum() == dwItemVnum && FN_check_item_socket(item))
					{
#ifdef __ITEM_COUNT_LIMIT__
						DWORD bCount2 = MIN(g_bItemCountLimit - item->GetCount(), bCount);
#else
						BYTE bCount2 = MIN(g_bItemCountLimit - item->GetCount(), bCount);
#endif
						bCount -= bCount2;
						item->SetCount(item->GetCount() + bCount2);

						if (bCount == 0)
						{
							if (bMsg)
								ChatPacket(CHAT_TYPE_INFO, LC_TEXT("STONE_INVENTORY_ADDED: %s"), item->GetName());
							return item;
						}
					}
				}
			}
			else if (IsFlowerItem(dwItemVnum)) //flower item
			{
				for (int i = 0; i < SPECIAL_INVENTORY_MAX_NUM; ++i)
				{
					LPITEM item = GetFlowerInventoryItem(i);
					if (!item)
						continue;

					if (item->GetVnum() == dwItemVnum && FN_check_item_socket(item))
					{
#ifdef __ITEM_COUNT_LIMIT__
						DWORD bCount2 = MIN(g_bItemCountLimit - item->GetCount(), bCount);
#else
						BYTE bCount2 = MIN(g_bItemCountLimit - item->GetCount(), bCount);
#endif
						bCount -= bCount2;
						item->SetCount(item->GetCount() + bCount2);

						if (bCount == 0)
						{
							if (bMsg)
								ChatPacket(CHAT_TYPE_INFO, LC_TEXT("FLOWER_INVENTORY_ADDED: %s"), item->GetName());
							return item;
						}
					}
				}
			}
			else if (IsAttrSpecial(dwItemVnum)) //attr item
			{
				for (int i = 0; i < SPECIAL_INVENTORY_MAX_NUM; ++i)
				{
					LPITEM item = GetAttrInventoryItem(i);
					if (!item)
						continue;

					if (item->GetVnum() == dwItemVnum && FN_check_item_socket(item))
					{
#ifdef __ITEM_COUNT_LIMIT__
						DWORD bCount2 = MIN(g_bItemCountLimit - item->GetCount(), bCount);
#else
						BYTE bCount2 = MIN(g_bItemCountLimit - item->GetCount(), bCount);
#endif
						bCount -= bCount2;
						item->SetCount(item->GetCount() + bCount2);

						if (bCount == 0)
						{
							if (bMsg)
								ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ATTR_INVENTORY_ADDED: %s"), item->GetName());
							return item;
						}
					}
				}
			}
			else if (IsChestItem(dwItemVnum)) //chest item
			{
				for (int i = 0; i < SPECIAL_INVENTORY_MAX_NUM; ++i)
				{
					LPITEM item = GetChestInventoryItem(i);
					if (!item)
						continue;

					if (item->GetVnum() == dwItemVnum && FN_check_item_socket(item))
					{
#ifdef __ITEM_COUNT_LIMIT__
						DWORD bCount2 = MIN(g_bItemCountLimit - item->GetCount(), bCount);
#else
						BYTE bCount2 = MIN(g_bItemCountLimit - item->GetCount(), bCount);
#endif
						bCount -= bCount2;
						item->SetCount(item->GetCount() + bCount2);

						if (bCount == 0)
						{
							if (bMsg)
								ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CHEST_INVENTORY_ADDED: %s"), item->GetName());
							return item;
						}
					}
				}
			}
			else
			{
				for (int i = 0; i < INVENTORY_MAX_NUM; ++i)
				{
					LPITEM item = GetInventoryItem(i);
					if (!item)
						continue;

					if (item->IsUpgradeItem() || item->IsBook() || item->IsStone() || item->IsFlower() || item->IsAttrItem() || item->IsChest() || item->IsDragonSoul())
						continue;

					if (item->GetVnum() == dwItemVnum && FN_check_item_socket(item))
					{
#ifdef __ITEM_COUNT_LIMIT__
						DWORD bCount2 = MIN(g_bItemCountLimit - item->GetCount(), bCount);
#else
						BYTE bCount2 = MIN(g_bItemCountLimit - item->GetCount(), bCount);
#endif
						bCount -= bCount2;
						item->SetCount(item->GetCount() + bCount2);

						if (bCount == 0)
						{
							if (bMsg)
								ChatPacket(CHAT_TYPE_INFO, LC_TEXT("NORMAL_INVENTORY_ADDED: %s"), item->GetName());

							return item;
						}
					}
				}
			}
#else
			{
				for (int i = 0; i < INVENTORY_MAX_NUM; ++i)
				{
					LPITEM item = GetInventoryItem(i);
					if (!item)
						continue;

					if (item->IsDragonSoul())
						continue;

					if (item->GetVnum() == dwItemVnum && FN_check_item_socket(item))
					{
#ifdef __ITEM_COUNT_LIMIT__
						DWORD bCount2 = MIN(g_bItemCountLimit - item->GetCount(), bCount);
#else
						BYTE bCount2 = MIN(g_bItemCountLimit - item->GetCount(), bCount);
#endif
						bCount -= bCount2;
						item->SetCount(item->GetCount() + bCount2);

						if (bCount == 0)
						{
							if (bMsg)
								ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¾ÆÀÌÅÛ È¹µæ: %s"), item->GetName());

							return item;
						}
					}
				}
			}
#endif
		}
	}

	LPITEM item = ITEM_MANAGER::instance().CreateItem(dwItemVnum, bCount, 0, true);

	if (!item)
	{
		sys_err("cannot create item by vnum %u (name: %s)", dwItemVnum, GetName());
		return NULL;
	}

	if (item->GetType() == ITEM_BLEND)
	{
		for (int i = 0; i < INVENTORY_MAX_NUM; i++)
		{
			LPITEM item2 = GetInventoryItem(i);

			if (!item2)
				continue;

			if (item2->GetType() == ITEM_BLEND && (IS_SET(item2->GetFlag(), ITEM_FLAG_STACKABLE) && !IS_SET(item2->GetFlag(), ITEM_ANTIFLAG_STACK)))
			{
				if (item2->GetVnum() == item->GetVnum() && item2->GetCount() < g_bItemCountLimit)
				{
					if (item2->GetSocket(0) == item->GetSocket(0) && item2->GetSocket(1) == item->GetSocket(1) && item2->GetSocket(2) == item->GetSocket(2))
					{
						item2->SetCount(item2->GetCount() + item->GetCount());
						M2_DESTROY_ITEM(item);// @duzenleme itemi item2 olarak verdigi icin silmesi lazim silmezse Allah'a emanet. fakat ITEM_ID_DUP yaratma ihtimalide mevcut.
						return item2;
					}
				}
			}
		}
	}
#ifdef __ADDITIONAL_INVENTORY__
	else if (item->IsBook())
	{
		for (int i = 0; i < SPECIAL_INVENTORY_MAX_NUM; ++i)
		{
			LPITEM item2 = GetBookInventoryItem(i);
			if (!item2)
				continue;

			if (item2->IsBook() && (IS_SET(item2->GetFlag(), ITEM_FLAG_STACKABLE) && !IS_SET(item2->GetFlag(), ITEM_ANTIFLAG_STACK)))
			{
				if (item2->GetVnum() == item->GetVnum() && item2->GetCount() < g_bItemCountLimit)
				{
					if (item2->GetSocket(0) == item->GetSocket(0))
					{
						item2->SetCount(item2->GetCount() + item->GetCount());
						if (bMsg)
							ChatPacket(CHAT_TYPE_INFO, LC_TEXT("BOOK_INVENTORY_ADDED: %s"), item2->GetName());
						M2_DESTROY_ITEM(item); // @duzenleme itemi item2 olarak verdigi icin silmesi lazim silmezse Allah'a emanet. fakat ITEM_ID_DUP yaratma ihtimalide mevcut.
						return item2;
					}
				}
			}
		}
	}
#endif

	int iEmptyCell;
	if (item->IsDragonSoul())
		iEmptyCell = GetEmptyDragonSoulInventory(item);
#ifdef __ADDITIONAL_INVENTORY__
	else if (item->IsUpgradeItem())
		iEmptyCell = GetEmptyUpgradeInventory(item);
	else if (item->IsBook())
		iEmptyCell = GetEmptyBookInventory(item);
	else if (item->IsStone())
		iEmptyCell = GetEmptyStoneInventory(item);
	else if (item->IsFlower())
		iEmptyCell = GetEmptyFlowerInventory(item);
	else if (item->IsAttrItem())
		iEmptyCell = GetEmptyAttrInventory(item);
	else if (item->IsChest())
		iEmptyCell = GetEmptyChestInventory(item);
#endif
	else
		iEmptyCell = GetEmptyInventory(item->GetSize());

	if (iEmptyCell != -1)
	{
		if (bMsg)
		{
			if (item->IsDragonSoul())
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("DRAGON_SOUL_INVENTORY_ADDED: %s"), item->GetName());
			else if (item->IsUpgradeItem())
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("UPGRADE_INVENTORY_ADDED: %s"), item->GetName());
			else if (item->IsBook())
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("BOOK_INVENTORY_ADDED: %s"), item->GetName());
			else if (item->IsStone())
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("STONE_INVENTORY_ADDED: %s"), item->GetName());
			else if (item->IsFlower())
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("FLOWER_INVENTORY_ADDED: %s"), item->GetName());
			else if (item->IsAttrItem())
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ATTR_INVENTORY_ADDED: %s"), item->GetName());
			else if (item->IsChest())
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CHEST_INVENTORY_ADDED: %s"), item->GetName());
			else
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("NORMAL_INVENTORY_ADDED: %s"), item->GetName());
		}

		if (item->IsDragonSoul())
			item->__ADD_TO_CHARACTER(this, TItemPos(DRAGON_SOUL_INVENTORY, iEmptyCell));
#ifdef __ADDITIONAL_INVENTORY__
		else if (item->IsUpgradeItem())
			item->__ADD_TO_CHARACTER(this, TItemPos(UPGRADE_INVENTORY, iEmptyCell));
		else if (item->IsBook())
			item->__ADD_TO_CHARACTER(this, TItemPos(BOOK_INVENTORY, iEmptyCell));
		else if (item->IsStone())
			item->__ADD_TO_CHARACTER(this, TItemPos(STONE_INVENTORY, iEmptyCell));
		else if (item->IsFlower())
			item->__ADD_TO_CHARACTER(this, TItemPos(FLOWER_INVENTORY, iEmptyCell));
		else if (item->IsAttrItem())
			item->__ADD_TO_CHARACTER(this, TItemPos(ATTR_INVENTORY, iEmptyCell));
		else if (item->IsChest())
			item->__ADD_TO_CHARACTER(this, TItemPos(CHEST_INVENTORY, iEmptyCell));
#endif
		else
			item->__ADD_TO_CHARACTER(this, TItemPos(INVENTORY, iEmptyCell));

		if (item->GetType() == ITEM_USE && item->GetSubType() == USE_POTION)
		{
			TQuickslot* pSlot;

			if (GetQuickslot(0, &pSlot) && pSlot->type == QUICKSLOT_TYPE_NONE)
			{
				TQuickslot slot;
				slot.type = QUICKSLOT_TYPE_ITEM;
				slot.pos = iEmptyCell;
				SetQuickslot(0, slot);
			}
		}
	}
	else
	{
		item->AddToGround(GetMapIndex(), GetXYZ());
		item->StartDestroyEvent(g_aiItemDestroyTime[ITEM_DESTROY_TIME_AUTOGIVE]);

		if (IS_SET(item->GetAntiFlag(), ITEM_ANTIFLAG_DROP))
			item->SetOwnership(this, 300);
		else
			item->SetOwnership(this, 60);
	}

	//sys_log(0, "AutoGiveItem: %d %d", dwItemVnum, bCount);
	return item;
}

bool CHARACTER::GiveItem(LPCHARACTER victim, TItemPos Cell)
{
	if (!CanHandleItem())
		return false;

	// @duzenleme
	// gorev penceresi acik iken bu islemin yapilmamasini saglar fakat bu kontroller detaylandirilabilir.
	if (quest::CQuestManager::instance().GetPCForce(GetPlayerID())->IsRunning() == true)
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You cannot take this item if you're using quests"));
		return false;
	}

	LPITEM item = GetItem(Cell);

	if (item && !item->IsExchanging())
	{
		if (victim->CanReceiveItem(this, item))
		{
			victim->ReceiveItem(this, item);
			return true;
		}
	}

	return false;
}

bool CHARACTER::CanReceiveItem(LPCHARACTER from, LPITEM item, bool ignoreDist /*= false*/) const // @duzenleme uzaktan arti basma hilesi
{
	if (IsPC())
		return false;

	// TOO_LONG_DISTANCE_EXCHANGE_BUG_FIX
	if (!ignoreDist && DISTANCE_APPROX(GetX() - from->GetX(), GetY() - from->GetY()) > 2000) // @duzenleme uzaktan arti basmamalari icin boyle bir engel aldik (Rubinum Metin2AR)
		return false;
	// END_OF_TOO_LONG_DISTANCE_EXCHANGE_BUG_FIX

#ifdef __BEGINNER_ITEM__
	if (item->IsBasicItem())
	{
		from->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ITEM_IS_BASIC_CANNOT_DO"));
		return false;
	}
#endif

	switch (GetRaceNum())
	{
	case fishing::CAMPFIRE_MOB:
		if (item->GetType() == ITEM_FISH &&
			(item->GetSubType() == FISH_ALIVE || item->GetSubType() == FISH_DEAD))
			return true;
		break;

	case fishing::FISHER_MOB:
		if (item->GetType() == ITEM_ROD)
			return true;
		break;

	case BLACKSMITH_MOB:
		if (item->GetRefinedVnum() && item->GetRefineSet() < 500)
			return true;
		else
			return false;

	case BLACKSMITH2_MOB:
		if (item->GetRefineSet() >= 500)
			return true;
		else
			return false;

	case ALCHEMIST_MOB:
		if (item->GetRefinedVnum())
			return true;
		break;

	case 20101:
	case 20102:
	case 20103:

		if (item->GetVnum() == ITEM_REVIVE_HORSE_1)
		{
			if (!IsDead())
			{
				from->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Á×Áö ¾ÊÀº ¸»¿¡°Ô ¼±ÃÊ¸¦ ¸ÔÀÏ ¼ö ¾ø½À´Ï´Ù."));
				return false;
			}
			return true;
		}
		else if (item->GetVnum() == ITEM_HORSE_FOOD_1)
		{
			if (IsDead())
			{
				from->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Á×Àº ¸»¿¡°Ô »ç·á¸¦ ¸ÔÀÏ ¼ö ¾ø½À´Ï´Ù."));
				return false;
			}
			return true;
		}
		else if (item->GetVnum() == ITEM_HORSE_FOOD_2 || item->GetVnum() == ITEM_HORSE_FOOD_3)
			return false;
		break;
	case 20104:
	case 20105:
	case 20106:

		if (item->GetVnum() == ITEM_REVIVE_HORSE_2)
		{
			if (!IsDead())
			{
				from->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Á×Áö ¾ÊÀº ¸»¿¡°Ô ¼±ÃÊ¸¦ ¸ÔÀÏ ¼ö ¾ø½À´Ï´Ù."));
				return false;
			}
			return true;
		}
		else if (item->GetVnum() == ITEM_HORSE_FOOD_2)
		{
			if (IsDead())
			{
				from->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Á×Àº ¸»¿¡°Ô »ç·á¸¦ ¸ÔÀÏ ¼ö ¾ø½À´Ï´Ù."));
				return false;
			}
			return true;
		}
		else if (item->GetVnum() == ITEM_HORSE_FOOD_1 || item->GetVnum() == ITEM_HORSE_FOOD_3)
			return false;
		break;
	}

	return true;
}

void CHARACTER::ReceiveItem(LPCHARACTER from, LPITEM item)
{
	if (IsPC())
		return;

	switch (GetRaceNum())
	{
	case fishing::CAMPFIRE_MOB:
		if (item->GetType() == ITEM_FISH && (item->GetSubType() == FISH_ALIVE || item->GetSubType() == FISH_DEAD))
			fishing::Grill(from, item);
		else
		{
			// TAKE_ITEM_BUG_FIX
			from->SetQuestNPCID(GetVID());
			// END_OF_TAKE_ITEM_BUG_FIX
			quest::CQuestManager::instance().TakeItem(from->GetPlayerID(), GetRaceNum(), item);
		}
		break;

		// DEVILTOWER_NPC
	case DEVILTOWER_BLACKSMITH_WEAPON_MOB:
	case DEVILTOWER_BLACKSMITH_ARMOR_MOB:
	case DEVILTOWER_BLACKSMITH_ACCESSORY_MOB:
		if (item->GetRefinedVnum() != 0 && item->GetRefineSet() != 0 && item->GetRefineSet() < 500)
		{
			from->SetRefineNPC(this);
			from->RefineInformation(item->GetCell(), REFINE_TYPE_MONEY_ONLY);
		}
		else
			from->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ ¾ÆÀÌÅÛÀº °³·®ÇÒ ¼ö ¾ø½À´Ï´Ù."));
		break;
		// END_OF_DEVILTOWER_NPC

	case BLACKSMITH_MOB:
	case BLACKSMITH2_MOB:
	case BLACKSMITH_WEAPON_MOB:
	case BLACKSMITH_ARMOR_MOB:
	case BLACKSMITH_ACCESSORY_MOB:
		if (item->GetRefinedVnum())
		{
			from->SetRefineNPC(this);
			from->RefineInformation(item->GetCell(), REFINE_TYPE_NORMAL);
		}
		else
			from->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ ¾ÆÀÌÅÛÀº °³·®ÇÒ ¼ö ¾ø½À´Ï´Ù."));
		break;

	case 20101:
	case 20102:
	case 20103:
	case 20104:
	case 20105:
	case 20106:
		if (item->GetVnum() == ITEM_REVIVE_HORSE_1 ||
			item->GetVnum() == ITEM_REVIVE_HORSE_2 ||
			item->GetVnum() == ITEM_REVIVE_HORSE_3)
		{
			from->ReviveHorse();
			item->SetCount(item->GetCount() - 1);
			from->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¸»¿¡°Ô ¼±ÃÊ¸¦ ÁÖ¾ú½À´Ï´Ù."));
		}
		else if (item->GetVnum() == ITEM_HORSE_FOOD_1 ||
			item->GetVnum() == ITEM_HORSE_FOOD_2 ||
			item->GetVnum() == ITEM_HORSE_FOOD_3)
		{
			from->FeedHorse();
			from->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¸»¿¡°Ô »ç·á¸¦ ÁÖ¾ú½À´Ï´Ù."));
			item->SetCount(item->GetCount() - 1);
			EffectPacket(SE_HPUP_RED);
		}
		break;

	default:
		sys_log(0, "TakeItem %s %d %s", from->GetName(), GetRaceNum(), item->GetName());
		from->SetQuestNPCID(GetVID());
		quest::CQuestManager::instance().TakeItem(from->GetPlayerID(), GetRaceNum(), item);
		break;
	}
}

bool CHARACTER::IsEquipUniqueItem(DWORD dwItemVnum) const
{
	{
		LPITEM u = GetWear(WEAR_UNIQUE1);

		if (u && u->GetVnum() == dwItemVnum)
			return true;
	}

	{
		LPITEM u = GetWear(WEAR_UNIQUE2);

		if (u && u->GetVnum() == dwItemVnum)
			return true;
	}

	if (dwItemVnum == UNIQUE_ITEM_RING_OF_LANGUAGE)
		return IsEquipUniqueItem(UNIQUE_ITEM_RING_OF_LANGUAGE_SAMPLE);

	return false;
}

// CHECK_UNIQUE_GROUP
bool CHARACTER::IsEquipUniqueGroup(DWORD dwGroupVnum) const
{
	{
		LPITEM u = GetWear(WEAR_UNIQUE1);

		if (u && u->GetSpecialGroup() == (int)dwGroupVnum)
			return true;
	}

	{
		LPITEM u = GetWear(WEAR_UNIQUE2);

		if (u && u->GetSpecialGroup() == (int)dwGroupVnum)
			return true;
	}

	return false;
}
// END_OF_CHECK_UNIQUE_GROUP

void CHARACTER::SetRefineMode(int iAdditionalCell)
{
	m_iRefineAdditionalCell = iAdditionalCell;
	m_bUnderRefine = true;
}

void CHARACTER::ClearRefineMode()
{
	m_bUnderRefine = false;
	SetRefineNPC(NULL);
}

#ifdef __RENEWAL_CHEST_USE__
int CHARACTER::GiveItemFromSpecialItemGroup(DWORD dwGroupNum, DWORD count, boost::unordered_map<DWORD, DWORD>& items)
{
	const CSpecialItemGroup* pGroup = ITEM_MANAGER::instance().GetSpecialItemGroup(dwGroupNum);

	if (!pGroup)
	{
		sys_err("cannot find special item group %d", dwGroupNum);
		return false;
	}

	int iSuccesCount = 0;
	std::vector <int> idxes;

	for (DWORD c = 0; c < count; c++)
	{
#ifdef __CHEST_DROP_POINT__
		int n = pGroup->GetMultiIndex(idxes, GetPoint(POINT_CHEST_BONUS));
#else
		int n = pGroup->GetMultiIndex(idxes);
#endif

		for (int i = 0; i < n; i++)
		{
			int idx = idxes[i];
			DWORD dwVnum = pGroup->GetVnum(idx);
			DWORD dwCount = pGroup->GetCount(idx);

			if (!dwVnum || !dwCount)
			{
				sys_err("INVALID CHEST VALUES %s %u %u (%u %u)", GetName(), dwGroupNum, count, dwVnum, dwCount);
				continue;
			}

			auto it = items.find(dwVnum);

			if (it == items.end())
			{
				// items[dwVnum] = dwCount;
				items.insert({dwVnum, dwCount});
			}
			else
			{
				it->second += dwCount;
			}
		}

		iSuccesCount++;
	}

	return iSuccesCount;
}
#endif

// NEW_HAIR_STYLE_ADD
bool CHARACTER::ItemProcess_Hair(LPITEM item, int iDestCell)
{
	if (item->CheckItemUseLevel(GetLevel()) == false)
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¾ÆÁ÷ ÀÌ ¸Ó¸®¸¦ »ç¿ëÇÒ ¼ö ¾ø´Â ·¹º§ÀÔ´Ï´Ù."));
		return false;
	}

	DWORD hair = item->GetVnum();

	switch (GetJob())
	{
	case JOB_WARRIOR:
		hair -= 72000;
		break;

	case JOB_ASSASSIN:
		hair -= 71250;
		break;

	case JOB_SURA:
		hair -= 70500;
		break;

	case JOB_SHAMAN:
		hair -= 69750;
		break;

#ifdef __WOLFMAN_CHARACTER__
	case JOB_WOLFMAN:
		break;
#endif

	default:
		return false;
		break;
	}

	if (hair == GetPart(PART_HAIR))
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("µ¿ÀÏÇÑ ¸Ó¸® ½ºÅ¸ÀÏ·Î´Â ±³Ã¼ÇÒ ¼ö ¾ø½À´Ï´Ù."));
		return true;
	}

	item->SetCount(item->GetCount() - 1);

	SetPart(PART_HAIR, hair);
	UpdatePacket();

	return true;
}
// END_NEW_HAIR_STYLE_ADD

bool CHARACTER::UnEquipSpecialRideUniqueItem()
{
	LPITEM Unique1 = GetWear(WEAR_UNIQUE1);
	LPITEM Unique2 = GetWear(WEAR_UNIQUE2);
	if (NULL != Unique1)
	{
		if (UNIQUE_GROUP_SPECIAL_RIDE == Unique1->GetSpecialGroup())
		{
			return UnequipItem(Unique1);
		}
	}

	if (NULL != Unique2)
	{
		if (UNIQUE_GROUP_SPECIAL_RIDE == Unique2->GetSpecialGroup())
		{
			return UnequipItem(Unique2);
		}
	}

	return true;
}

void CHARACTER::AutoRecoveryItemProcess(const EAffectTypes type)
{
	if (true == IsDead() || true == IsStun())
		return;

	if (false == IsPC())
		return;

	if (AFFECT_AUTO_HP_RECOVERY != type && AFFECT_AUTO_SP_RECOVERY != type)
		return;

	if (NULL != FindAffect(AFFECT_STUN))
		return;

	{
		const DWORD stunSkills[] = { SKILL_TANHWAN, SKILL_GEOMPUNG, SKILL_BYEURAK, SKILL_GIGUNG };

		for (size_t i = 0; i < sizeof(stunSkills) / sizeof(DWORD); ++i)
		{
			const CAffect* p = FindAffect(stunSkills[i]);

			if (NULL != p && AFF_STUN == p->dwFlag)
				return;
		}
	}

	const CAffect* pAffect = FindAffect(type);
	const size_t idx_of_amount_of_used = 1;
	const size_t idx_of_amount_of_full = 2;

	if (NULL != pAffect)
	{
		LPITEM pItem = FindItemByID(pAffect->dwFlag);

		if (NULL != pItem && true == pItem->GetSocket(0))
		{
			const long amount_of_used = pItem->GetSocket(idx_of_amount_of_used);
			const long amount_of_full = pItem->GetSocket(idx_of_amount_of_full);
			const int32_t avail = amount_of_full - amount_of_used;

			int32_t amount = 0;

			if (AFFECT_AUTO_HP_RECOVERY == type)
			{
				amount = GetMaxHP() - (GetHP() + GetPoint(POINT_HP_RECOVERY));
			}
			else if (AFFECT_AUTO_SP_RECOVERY == type)
			{
				amount = GetMaxSP() - (GetSP() + GetPoint(POINT_SP_RECOVERY));
			}

			if (amount > 0)
			{
				if (!g_bUnlimitedAutoPotion)
				{
					if (avail > amount)
					{
						const int pct_of_used = amount_of_used * 100 / amount_of_full;
						const int pct_of_will_used = (amount_of_used + amount) * 100 / amount_of_full;

						bool bLog = false;

						if ((pct_of_will_used / 10) - (pct_of_used / 10) >= 1)
							bLog = true;
						pItem->SetSocket(idx_of_amount_of_used, amount_of_used + amount, bLog);
					}
					else
					{
						amount = avail;
						ITEM_MANAGER::instance().RemoveItem(pItem);
					}
				}

				if (AFFECT_AUTO_HP_RECOVERY == type)
				{
					PointChange(POINT_HP_RECOVERY, amount);
					EffectPacket(SE_AUTO_HPUP);
				}
				else if (AFFECT_AUTO_SP_RECOVERY == type)
				{
					PointChange(POINT_SP_RECOVERY, amount);
					EffectPacket(SE_AUTO_SPUP);
				}
			}
		}
		else
		{
			RemoveAffect(const_cast<CAffect*>(pAffect));
		}
	}
}

bool CHARACTER::IsValidItemPosition(TItemPos Pos) const
{
	BYTE window_type = Pos.window_type;
	WORD cell = Pos.cell;

	switch (window_type)
	{
	case RESERVED_WINDOW:
		return false;

	case INVENTORY:
	case EQUIPMENT:
		return cell < (INVENTORY_AND_EQUIP_SLOT_MAX);

	case DRAGON_SOUL_INVENTORY:
		return cell < (DRAGON_SOUL_INVENTORY_MAX_NUM);

#ifdef __ADDITIONAL_INVENTORY__
	case UPGRADE_INVENTORY:
	case BOOK_INVENTORY:
	case STONE_INVENTORY:
	case FLOWER_INVENTORY:
	case ATTR_INVENTORY:
	case CHEST_INVENTORY:
		return cell < (SPECIAL_INVENTORY_MAX_NUM);
#endif
	case SAFEBOX:
		if (NULL != m_pkSafebox)
			return m_pkSafebox->IsValidPosition(cell);
		else
			return false;

	case MALL:
		if (NULL != m_pkMall)
			return m_pkMall->IsValidPosition(cell);
		else
			return false;

#ifdef __SWITCHBOT__
	case SWITCHBOT:
		return cell < SWITCHBOT_SLOT_COUNT;
#endif

	default:
		return false;
	}
}

#define VERIFY_MSG(exp, msg)  \
	if (true == (exp)) { \
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT(msg)); \
			return false; \
	}

bool CHARACTER::CanEquipNow(const LPITEM item, const TItemPos & srcCell, const TItemPos & destCell) /*const*/
{
	const TItemTable* itemTable = item->GetProto();
	//BYTE itemType = item->GetType();
	//BYTE itemSubType = item->GetSubType();

	switch (GetJob())
	{
	case JOB_WARRIOR:
		if (item->GetAntiFlag() & ITEM_ANTIFLAG_WARRIOR)
			return false;
		break;

	case JOB_ASSASSIN:
		if (item->GetAntiFlag() & ITEM_ANTIFLAG_ASSASSIN)
			return false;
		break;

	case JOB_SHAMAN:
		if (item->GetAntiFlag() & ITEM_ANTIFLAG_SHAMAN)
			return false;
		break;

	case JOB_SURA:
		if (item->GetAntiFlag() & ITEM_ANTIFLAG_SURA)
			return false;
		break;

#ifdef __WOLFMAN_CHARACTER__
	case JOB_WOLFMAN:
		if (item->GetAntiFlag() & ITEM_ANTIFLAG_WOLFMAN)
			return false;
		break;
#endif
	}

	for (int i = 0; i < ITEM_LIMIT_MAX_NUM; ++i)
	{
		long limit = itemTable->aLimits[i].lValue;
		switch (itemTable->aLimits[i].bType)
		{
		case LIMIT_LEVEL:
			if (GetLevel() < limit)
			{
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("·¹º§ÀÌ ³·¾Æ Âø¿ëÇÒ ¼ö ¾ø½À´Ï´Ù."));
				return false;
			}
			break;

		case LIMIT_STR:
			if (GetPoint(POINT_ST) < limit)
			{
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("±Ù·ÂÀÌ ³·¾Æ Âø¿ëÇÒ ¼ö ¾ø½À´Ï´Ù."));
				return false;
			}
			break;

		case LIMIT_INT:
			if (GetPoint(POINT_IQ) < limit)
			{
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Áö´ÉÀÌ ³·¾Æ Âø¿ëÇÒ ¼ö ¾ø½À´Ï´Ù."));
				return false;
			}
			break;

		case LIMIT_DEX:
			if (GetPoint(POINT_DX) < limit)
			{
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¹ÎÃ¸ÀÌ ³·¾Æ Âø¿ëÇÒ ¼ö ¾ø½À´Ï´Ù."));
				return false;
			}
			break;

		case LIMIT_CON:
			if (GetPoint(POINT_HT) < limit)
			{
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Ã¼·ÂÀÌ ³·¾Æ Âø¿ëÇÒ ¼ö ¾ø½À´Ï´Ù."));
				return false;
			}
			break;
		}
	}

	// @duzenleme eger yuzuk veya unique esyalar ayni koda sahip ise tekrar takilmiyor.
	if (item->GetType() == ITEM_UNIQUE)
	{
		LPITEM UniqueItems[2] = { GetWear(WEAR_UNIQUE1), GetWear(WEAR_UNIQUE2) };

		std::array<DWORD, 10> bekciler = { 72054, 96001, 96041, 96035, 96045, 57001, 57005, 57009, 57013, 57017 };
		std::array<DWORD, 10> kahraman = { 71158, 96000, 96040, 96036, 96046, 57002, 57006, 57010, 57014, 57018 };

		for (int i = 0; i < 2; i++)
		{
			if (UniqueItems[i])
			{
				if (UniqueItems[i]->GetVnum() == item->GetVnum())
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("iki kez bu objeyi takamazsin!"));
					return false;
				}

				if (std::any_of(bekciler.begin(), bekciler.end(), [&](DWORD itemVnum){ return item->GetVnum() == itemVnum; })
				&& std::any_of(bekciler.begin(), bekciler.end(), [&](DWORD itemVnum){ return UniqueItems[i]->GetVnum() == itemVnum; }))
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("iki kez bu objeyi takamazsin!"));
					return false;
				}

				if (std::any_of(kahraman.begin(), kahraman.end(), [&](DWORD itemVnum){ return item->GetVnum() == itemVnum; })
				&& std::any_of(kahraman.begin(), kahraman.end(), [&](DWORD itemVnum){ return UniqueItems[i]->GetVnum() == itemVnum; }))
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("iki kez bu objeyi takamazsin!"));
					return false;
				}
			}
		}
	}
	if (item->GetType() == ITEM_RING)
	{
		LPITEM ringItems[2] = { GetWear(WEAR_RING1), GetWear(WEAR_RING2) };

		std::array<DWORD, 11> hilal = { 71135, 96031, 96042, 96038, 96048, 57004, 57008, 57012, 57016, 57020 };
		std::array<DWORD, 11> nazar = { 71202, 96032, 96043, 96037, 96047, 57003, 57007, 57011, 57015, 57019 };

		for (int i = 0; i < 2; i++)
		{
			if (ringItems[i])
			{
				if (ringItems[i]->GetVnum() == item->GetVnum())
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("iki kez bu objeyi takamazsin!"));
					return false;
				}

				if (std::any_of(hilal.begin(), hilal.end(), [&](DWORD itemVnum){ return item->GetVnum() == itemVnum; })
				&& std::any_of(hilal.begin(), hilal.end(), [&](DWORD itemVnum){ return ringItems[i]->GetVnum() == itemVnum; }))
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("iki kez bu objeyi takamazsin!"));
					return false;
				}

				if (std::any_of(nazar.begin(), nazar.end(), [&](DWORD itemVnum){ return item->GetVnum() == itemVnum; })
				&& std::any_of(nazar.begin(), nazar.end(), [&](DWORD itemVnum){ return ringItems[i]->GetVnum() == itemVnum; }))
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("iki kez bu objeyi takamazsin!"));
					return false;
				}
			}
		}
	}

#ifdef __DS_SET_BONUS__
	if ((DragonSoul_IsDeckActivated()) && (item->IsDragonSoul())) {
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("SIMYA_AKTIFKEN_YAPAMAZSIN"));
		return false;
	}
#endif

	return true;
}

bool CHARACTER::CanUnequipNow(const LPITEM item, const TItemPos & srcCell, const TItemPos & destCell) /*const*/
{
	if (ITEM_BELT == item->GetType())
		VERIFY_MSG(CBeltInventoryHelper::IsExistItemInBeltInventory(this), "º§Æ® ÀÎº¥Åä¸®¿¡ ¾ÆÀÌÅÛÀÌ Á¸ÀçÇÏ¸é ÇØÁ¦ÇÒ ¼ö ¾ø½À´Ï´Ù.");

	if (IS_SET(item->GetFlag(), ITEM_FLAG_IRREMOVABLE))
		return false;

#ifdef __DS_SET_BONUS__
	if ((DragonSoul_IsDeckActivated()) && (item->IsDragonSoul())) {
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("SIMYA_AKTIFKEN_YAPAMAZSIN"));
		return false;
	}
#endif

	{
		int pos = -1;

		if (item->IsDragonSoul())
			pos = GetEmptyDragonSoulInventory(item);
#ifdef __ADDITIONAL_INVENTORY__
		else if (item->IsUpgradeItem())
			pos = GetEmptyUpgradeInventory(item);
		else if (item->IsBook())
			pos = GetEmptyBookInventory(item);
		else if (item->IsStone())
			pos = GetEmptyStoneInventory(item);
		else if (item->IsFlower())
			pos = GetEmptyFlowerInventory(item);
		else if (item->IsAttrItem())
			pos = GetEmptyAttrInventory(item);
		else if (item->IsChest())
			pos = GetEmptyChestInventory(item);
#endif
		else
			pos = GetEmptyInventory(item->GetSize());

		VERIFY_MSG(-1 == pos, "¼ÒÁöÇ°¿¡ ºó °ø°£ÀÌ ¾ø½À´Ï´Ù.");
	}

	return true;
}

#ifdef __ANCIENT_ATTR_ITEM__
bool CHARACTER::UseItemNewAttribute(TItemPos source_pos, TItemPos target_pos, BYTE * bValues)
{
	LPITEM item;
	LPITEM item_target;

	if (!CanHandleItem())
		return false;

	if (!CanAct(true, true, VAR_TIME_NONE, nullptr, 0))
		return false;

	if (!IsValidItemPosition(source_pos) || !(item = GetItem(source_pos)))
		return false;

	if (!IsValidItemPosition(target_pos) || !(item_target = GetItem(target_pos)))
		return false;

	if (item->GetVnum() != 71051)
		return false;

	if (item->IsExchanging())
		return false;

	if (item_target->IsExchanging())
		return false;

	if (item_target->IsEquipped())
		return false;

	if (ITEM_COSTUME == item_target->GetType())
		return false;

#ifdef __BEGINNER_ITEM__
	if (item_target->IsBasicItem())
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ITEM_IS_BASIC_CANNOT_DO"));
		return false;
	}
#endif

	if (item_target->GetVnum() == 50201 || item_target->GetVnum() == 50202 || item_target->GetVnum() == 11901 || item_target->GetVnum() == 11902 || item_target->GetVnum() == 11903 || item_target->GetVnum() == 11904 || item_target->GetVnum() == 11911 || item_target->GetVnum() == 11912 || item_target->GetVnum() == 11913 || item_target->GetVnum() == 11914)
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You can't apply this attribute on this item."));
		return false;
	}

	int processNewAttr = item_target->AddNewStyleAttribute(bValues);

	if (processNewAttr == 1)
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You can't apply this attribute on this item."));
		return false;
	}
	else if (processNewAttr == 2)
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You can't apply an attribute twice."));
		return false;
	}
	else if (processNewAttr == 3)
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Attributes has been added successfully!"));
		item->SetCount(item->GetCount() - 1);
		return true;
	}

	return false;
}
#endif

#ifdef __CHECK_ITEMS_ON_TELEPORT__
void CHARACTER::CheckTeleportItems()
{
	for (int i = 0; i < INVENTORY_MAX_NUM; ++i)
	{
		LPITEM item = GetInventoryItem(i);
		if (!item)
			continue;
#ifdef __GROWTH_PET_SYSTEM__
		else if (item->IsNewPetItem())
		{
			if ((item->GetSocket(0) > 0) && (GetNewPetSystem() && GetNewPetSystem()->CountSummoned() == 0))
			{
				CNewPetSystem* petNewSystem = GetNewPetSystem();
				if (petNewSystem)
					petNewSystem->Summon(item->GetValue(0), item, 0, false);
			}
		}
#endif
#ifdef __GROWTH_MOUNT_SYSTEM__
		else if (item->IsNewMountExItem())
		{
			if ((item->GetSocket(0) > 0) && (GetNewMountSystem() && GetNewMountSystem()->CountSummoned() == 0))
			{
				CNewMountSystem* mountNewSystem = GetNewMountSystem();
				if (mountNewSystem)
					mountNewSystem->Summon(item->GetValue(0), item, 0, false);
			}
		}
#endif
#ifdef __BLEND_ITEM_REWORK__
		else if ((item->GetVnum() >= 90821 && item->GetVnum() <= 90836) || (item->GetVnum() >= 57021 && item->GetVnum() <= 57032))
		{
			if (item->GetSocket(2) > 0)
				item->Lock(true);
		}
#endif
#ifdef __BLEND_R_ITEMS__
		else if (item->GetVnum() >= 18383 && item->GetVnum() <= 18390)
		{
			if (item->GetSocket(0) > 0)
				item->Lock(true);
		}
#endif
#ifdef __POTION_AS_REWORK__
		else if (item->GetVnum() == 57031 || item->GetVnum() == 57032)
		{
			if (item->GetSocket(0) > 0)
				item->Lock(true);
		}
#endif // __POTION_AS_REWORK__
	}
}
#endif

#ifdef __STONE_DETECT_REWORK__
bool CHARACTER::AggregateStone()
{
	LPSECTREE_MAP pMap = SECTREE_MANAGER::instance().GetMap(GetMapIndex());

	if (GetDungeon() || IS_DUNGEON_ZONE(GetMapIndex()))
		return false;

	if (pMap != NULL)
	{
		// item->SetSocket(0, item->GetSocket(0) + 1);
		FFindStone f;

		// <Factor> SECTREE::for_each -> SECTREE::for_each_entity
		pMap->for_each(f);

		if (f.m_mapStone.size() > 0)
		{
			std::map<DWORD, LPCHARACTER>::iterator stone = f.m_mapStone.begin();

			DWORD max = UINT_MAX;
			LPCHARACTER pTarget = stone->second;

			while (stone != f.m_mapStone.end())
			{
				DWORD dist = (DWORD)DISTANCE_SQRT(GetX() - stone->second->GetX(), GetY() - stone->second->GetY());

				if (dist != 0 && max > dist)
				{
					pTarget = stone->second;
					if (pTarget && (pTarget->GetHP() != pTarget->GetMaxHP()))
					{
						stone++;
						continue;
					}
					max = dist;
				}
				stone++;
			}

			if (pTarget != NULL)
			{
#ifdef __STONE_DETECT_REWORK__
				Show(pTarget->GetMapIndex(), pTarget->GetX(), pTarget->GetY(), pTarget->GetZ());
#else
				int val = 3;

				if (max < 10000) val = 2;
				else if (max < 70000) val = 1;

				ChatPacket(CHAT_TYPE_COMMAND, "StoneDetect %u %d %d", (DWORD)GetVID(), val,
					(int)GetDegreeFromPositionXY(GetX(), pTarget->GetY(), pTarget->GetX(), GetY()));
#endif
				return true;
			}
			else
			{
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("°¨Áö±â¸¦ ÀÛ¿ëÇÏ¿´À¸³ª °¨ÁöµÇ´Â ¿µ¼®ÀÌ ¾ø½À´Ï´Ù."));
			}
		}
		else
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("°¨Áö±â¸¦ ÀÛ¿ëÇÏ¿´À¸³ª °¨ÁöµÇ´Â ¿µ¼®ÀÌ ¾ø½À´Ï´Ù."));
		}

		// if (item->GetSocket(0) >= 10000)
		// {
		// 	ChatPacket(CHAT_TYPE_COMMAND, "StoneDetect %u 0 0", (DWORD)GetVID());
		// 	ITEM_MANAGER::instance().RemoveItem(item);
		// }
	}
	return false;
}
#endif
