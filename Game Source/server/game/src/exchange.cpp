#include "stdafx.h"
#include "../../libgame/include/grid.h"
#include "utils.h"
#include "desc.h"
#include "desc_client.h"
#include "char.h"
#include "item.h"
#include "item_manager.h"
#include "packet.h"
#include "log.h"
#include "db.h"
#include "locale_service.h"
#include "../../common/length.h"
#include "exchange.h"
#include "DragonSoul.h"
#include "questmanager.h"
#include "config.h"

#ifdef __GROWTH_PET_SYSTEM__
#include "New_PetSystem.h"
#endif

#ifdef __GROWTH_MOUNT_SYSTEM__
#include "New_MountSystem.h"
#endif

#ifdef __GOLD_LIMIT_REWORK__
void exchange_packet(LPCHARACTER ch, BYTE sub_header, bool is_me, long long arg1, TItemPos arg2, DWORD arg3, void* pvData = NULL);
#else
void exchange_packet(LPCHARACTER ch, BYTE sub_header, bool is_me, DWORD arg1, TItemPos arg2, DWORD arg3, void* pvData = NULL);
#endif

#ifdef __GOLD_LIMIT_REWORK__
void exchange_packet(LPCHARACTER ch, BYTE sub_header, bool is_me, long long arg1, TItemPos arg2, DWORD arg3, void* pvData)
#else
void exchange_packet(LPCHARACTER ch, BYTE sub_header, bool is_me, DWORD arg1, TItemPos arg2, DWORD arg3, void* pvData)
#endif
{
	if (!ch->GetDesc())
		return;

	struct packet_exchange pack_exchg;

	pack_exchg.header = HEADER_GC_EXCHANGE;
	pack_exchg.sub_header = sub_header;
	pack_exchg.is_me = is_me;
	pack_exchg.arg1 = arg1;
	pack_exchg.arg2 = arg2;
	pack_exchg.arg3 = arg3;

	if (sub_header == EXCHANGE_SUBHEADER_GC_ITEM_ADD && pvData)
	{
		thecore_memcpy(&pack_exchg.alSockets, ((LPITEM)pvData)->GetSockets(), sizeof(pack_exchg.alSockets));
		thecore_memcpy(&pack_exchg.aAttr, ((LPITEM)pvData)->GetAttributes(), sizeof(pack_exchg.aAttr));
#ifdef __ITEM_EVOLUTION__
		pack_exchg.evolution = ((LPITEM)pvData)->GetEvolution();
#endif
#ifdef __ITEM_CHANGELOOK__
		pack_exchg.dwTransmutation = ((LPITEM)pvData)->GetTransmutation();
#endif
	}
	else
	{
		memset(&pack_exchg.alSockets, 0, sizeof(pack_exchg.alSockets));
		memset(&pack_exchg.aAttr, 0, sizeof(pack_exchg.aAttr));
#ifdef __ITEM_EVOLUTION__
		pack_exchg.evolution = 0;
#endif
#ifdef __ITEM_CHANGELOOK__
		pack_exchg.dwTransmutation = 0;
#endif
	}

	ch->GetDesc()->Packet(&pack_exchg, sizeof(pack_exchg));
}

bool CHARACTER::ExchangeStart(LPCHARACTER victim)
{
	if (this == victim)
		return false;

	if (IsObserverMode())
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("관전 상태에서는 교환을 할 수 없습니다."));
		return false;
	}

	if (victim->IsNPC())
		return false;

	if (IsDead() || IsStun())
		return false;

	if (victim->IsDead() || victim->IsStun())
		return false;

	if (GetDungeon() || GetWarMap() || IS_DUNGEON_ZONE(GetMapIndex()))
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You cannot open the safebox in dungeon or at war."));
		return false;
	}

	if (victim->GetDungeon() || victim->GetWarMap() || IS_DUNGEON_ZONE(victim->GetMapIndex()))
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Target cannot open the safebox in dungeon or at war."));
		return false;
	}

#ifdef __GAME_MASTER_UTILITY__
	if (IsGM() && !MasterCanAction(ACTION_EXCHANGE))
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("MASTER_ACTION_EXCHANGE"));
		return false;
	}

	if (victim->IsGM() && !victim->MasterCanAction(ACTION_EXCHANGE))
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("MASTER_ACTION_EXCHANGE"));
		return false;
	}
#endif

#ifdef __GROWTH_PET_SYSTEM__
	if (GetNewPetSystem()->IsActivePet())
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("PET_YANINDAYKEN_TICARET_ACAMAZSIN."));
		return false;
	}

	if (victim->GetNewPetSystem()->IsActivePet())
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("KARSI_TARAFIN_PETI_YANINDA_OLDUGU_ICIN_TICARET_ACAMAZSIN."));
		return false;
	}
#endif

#ifdef __GROWTH_MOUNT_SYSTEM__
	if (GetNewMountSystem()->IsActiveMount())
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("MOUNT_YANINDAYKEN_TICARET_ACAMAZSIN."));
		return false;
	}

	if (victim->GetNewMountSystem()->IsActiveMount())
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("KARSI_TARAFIN_MOUNTU_YANINDA_OLDUGU_ICIN_TICARET_ACAMAZSIN."));
		return false;
	}
#endif

	if (quest::CQuestManager::instance().GetPCForce(GetPlayerID())->IsRunning() == true)
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_OPERATION_IS_FAILED_QUEST_WINDOW_IS_OPENED"));
		return false;
	}

	if (!CanAct(true, true, VAR_TIME_NONE, nullptr, VAR_SAFEBOX | VAR_MYSHOP | VAR_REFINE | VAR_ACCE | VAR_CHANGELOOK | VAR_OFFLINESHOP))
		return false;

	if (quest::CQuestManager::instance().GetPCForce(victim->GetPlayerID())->IsRunning() == true)
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_OPERATION_IS_FAILED_QUEST_WINDOW_IS_OPENED"));
		return false;
	}

	if (!victim->CanAct(true, true, VAR_TIME_NONE, this, VAR_SAFEBOX | VAR_MYSHOP | VAR_REFINE | VAR_ACCE | VAR_CHANGELOOK | VAR_OFFLINESHOP))
		return false;

	//END_PREVENT_TRADE_WINDOW
	int iDist = DISTANCE_APPROX(GetX() - victim->GetX(), GetY() - victim->GetY());

	if (iDist >= EXCHANGE_MAX_DISTANCE)
		return false;

	if (GetExchange())
		return false;

	if (victim->GetExchange())
	{
		exchange_packet(this, EXCHANGE_SUBHEADER_GC_ALREADY, 0, 0, NPOS, 0);
		return false;
	}

	if (victim->IsBlockMode(BLOCK_EXCHANGE))
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("상대방이 교환 거부 상태입니다."));
		return false;
	}

	SetExchange(M2_NEW CExchange(this));
	victim->SetExchange(M2_NEW CExchange(victim));

	victim->GetExchange()->SetCompany(GetExchange());
	GetExchange()->SetCompany(victim->GetExchange());

	//
	SetExchangeTime();
	victim->SetExchangeTime();

	exchange_packet(victim, EXCHANGE_SUBHEADER_GC_START, 0, GetVID(), NPOS, 0);
	exchange_packet(this, EXCHANGE_SUBHEADER_GC_START, 0, victim->GetVID(), NPOS, 0);

	return true;
}

CExchange::CExchange(LPCHARACTER pOwner)
{
	m_pCompany = NULL;

	m_bAccept = false;

	for (int i = 0; i < EXCHANGE_ITEM_MAX_NUM; ++i)
	{
		m_apItems[i] = NULL;
		m_aItemPos[i] = NPOS;
		m_abItemDisplayPos[i] = 0;
	}

	m_lGold = 0;
#ifdef __CHEQUE_SYSTEM__
	m_bCheque = 0;
#endif

	m_pOwner = pOwner;
	pOwner->SetExchange(this);

	m_pGrid = M2_NEW CGrid(EXCHANGE_GRID_WIDTH, EXCHANGE_GRID_HEIGHT);
}

CExchange::~CExchange()
{
	M2_DELETE(m_pGrid);
}

bool CExchange::AddItem(TItemPos item_pos, BYTE display_pos)
{
	assert(m_pOwner != NULL && GetCompany());

	if (!item_pos.IsValidItemPosition())
		return false;

	if (item_pos.IsEquipPosition())
		return false;

	LPITEM item;

	if (!(item = m_pOwner->GetItem(item_pos)))
		return false;

	if (IS_SET(item->GetAntiFlag(), ITEM_ANTIFLAG_GIVE))
	{
		m_pOwner->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("아이템을 건네줄 수 없습니다."));
		return false;
	}

	if (true == item->isLocked())
	{
		return false;
	}
#ifdef __BEGINNER_ITEM__
	if (item->IsBasicItem())
	{
		m_pOwner->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ITEM_IS_BASIC_CANNOT_DO"));
		return false;
	}
#endif
	if (item->IsExchanging())
	{
		sys_log(0, "EXCHANGE under exchanging");
		return false;
	}

	if (!m_pGrid->IsEmpty(display_pos, 1, item->GetSize()))
	{
		sys_log(0, "EXCHANGE not empty item_pos %d %d %d", display_pos, 1, item->GetSize());
		return false;
	}

	if (m_pOwner->GetQuestItemPtr() == item) // @duzenleme eger esya quest tarafindan kullaniliyo ise esya ticarete eklenemiyor.
	{
		sys_log(0, "EXCHANGE %s trying to cheat by using a current quest item in trade", m_pOwner->GetName());
		return false;
	}

	Accept(false);
	GetCompany()->Accept(false);

	for (int i = 0; i < EXCHANGE_ITEM_MAX_NUM; ++i)
	{
		if (m_apItems[i])
			continue;

		m_apItems[i] = item;
		m_aItemPos[i] = item_pos;
		m_abItemDisplayPos[i] = display_pos;
		m_pGrid->Put(display_pos, 1, item->GetSize());

		item->SetExchanging(true);

		exchange_packet(m_pOwner,
			EXCHANGE_SUBHEADER_GC_ITEM_ADD,
			true,
			item->GetVnum(),
			TItemPos(RESERVED_WINDOW, display_pos),
			item->GetCount(),
			item);

		exchange_packet(GetCompany()->GetOwner(),
			EXCHANGE_SUBHEADER_GC_ITEM_ADD,
			false,
			item->GetVnum(),
			TItemPos(RESERVED_WINDOW, display_pos),
			item->GetCount(),
			item);

		sys_log(0, "EXCHANGE AddItem success %s pos(%d, %d) %d", item->GetName(), item_pos.window_type, item_pos.cell, display_pos);

		return true;
	}

	return false;
}

bool CExchange::RemoveItem(BYTE pos)
{
	return false;

	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return false;

	if (!m_apItems[pos])
		return false;

	TItemPos PosOfInventory = m_aItemPos[pos];
	m_apItems[pos]->SetExchanging(false);

	m_pGrid->Get(m_abItemDisplayPos[pos], 1, m_apItems[pos]->GetSize());

	exchange_packet(GetOwner(), EXCHANGE_SUBHEADER_GC_ITEM_DEL, true, pos, NPOS, 0);
	exchange_packet(GetCompany()->GetOwner(), EXCHANGE_SUBHEADER_GC_ITEM_DEL, false, pos, PosOfInventory, 0);

	Accept(false);
	GetCompany()->Accept(false);

	m_apItems[pos] = NULL;
	m_aItemPos[pos] = NPOS;
	m_abItemDisplayPos[pos] = 0;
	return true;
}

#ifdef __GOLD_LIMIT_REWORK__
bool CExchange::AddGold(long long gold)
#else
bool CExchange::AddGold(long gold)
#endif
{
	if (gold <= 0)
		return false;

	if (GetOwner()->GetGold() < gold)
	{
		exchange_packet(GetOwner(), EXCHANGE_SUBHEADER_GC_LESS_GOLD, 0, 0, NPOS, 0);
		return false;
	}

	if (m_lGold > 0)
		return false;

	Accept(false);
	GetCompany()->Accept(false);

	m_lGold = gold;

	exchange_packet(GetOwner(), EXCHANGE_SUBHEADER_GC_GOLD_ADD, true, m_lGold, NPOS, 0);
	exchange_packet(GetCompany()->GetOwner(), EXCHANGE_SUBHEADER_GC_GOLD_ADD, false, m_lGold, NPOS, 0);
	return true;
}

#ifdef __CHEQUE_SYSTEM__
bool CExchange::AddCheque(int Cheque)
{
	if (Cheque <= 0)
		return false;

	if (GetOwner()->GetCheque() < Cheque)
	{
		exchange_packet(GetOwner(), EXCHANGE_SUBHEADER_GC_LESS_CHEQUE, false, 0, NPOS, 0);
		return false;
	}

	if (m_bCheque > 0)
		return false;

	Accept(false);
	GetCompany()->Accept(false);

	m_bCheque = Cheque;

	exchange_packet(GetOwner(), EXCHANGE_SUBHEADER_GC_CHEQUE_ADD, true, m_bCheque, NPOS, 0);
	exchange_packet(GetCompany()->GetOwner(), EXCHANGE_SUBHEADER_GC_CHEQUE_ADD, false, m_bCheque, NPOS, 0);
	return true;
}
#endif

bool CExchange::Check(int* piItemCount)
{
	if (GetOwner()->GetGold() < m_lGold)
		return false;

#ifdef __CHEQUE_SYSTEM__
	if (GetOwner()->GetCheque() < m_bCheque)
		return false;
#endif

	int item_count = 0;

	for (int i = 0; i < EXCHANGE_ITEM_MAX_NUM; ++i)
	{
		if (!m_apItems[i])
			continue;

		if (!m_aItemPos[i].IsValidItemPosition())
			return false;

		if (m_apItems[i] != GetOwner()->GetItem(m_aItemPos[i]))
			return false;

		++item_count;
	}

	*piItemCount = item_count;
	return true;
}

bool CExchange::CheckSpace()
{
	static CGrid s_grid1(INVENTORY_PAGE_COLUMN, INVENTORY_PAGE_ROW); // inven page 1
	static CGrid s_grid2(INVENTORY_PAGE_COLUMN, INVENTORY_PAGE_ROW); // inven page 2
	static CGrid s_grid3(INVENTORY_PAGE_COLUMN, INVENTORY_PAGE_ROW); // inven page 3
	static CGrid s_grid4(INVENTORY_PAGE_COLUMN, INVENTORY_PAGE_ROW); // inven page 4

#ifdef __ADDITIONAL_INVENTORY__
	static CGrid s_upgradeGrid(INVENTORY_PAGE_COLUMN, INVENTORY_PAGE_ROW*SPECIAL_INVENTORY_PAGE_COUNT);
	static CGrid s_bookGrid(INVENTORY_PAGE_COLUMN, INVENTORY_PAGE_ROW*SPECIAL_INVENTORY_PAGE_COUNT);
	static CGrid s_stoneGrid(INVENTORY_PAGE_COLUMN, INVENTORY_PAGE_ROW*SPECIAL_INVENTORY_PAGE_COUNT);
	static CGrid s_flowerGrid(INVENTORY_PAGE_COLUMN, INVENTORY_PAGE_ROW*SPECIAL_INVENTORY_PAGE_COUNT);
	static CGrid s_attrGrid(INVENTORY_PAGE_COLUMN, INVENTORY_PAGE_ROW*SPECIAL_INVENTORY_PAGE_COUNT);
	static CGrid s_chestGrid(INVENTORY_PAGE_COLUMN, INVENTORY_PAGE_ROW*SPECIAL_INVENTORY_PAGE_COUNT);
#endif

	s_grid1.Clear();
	s_grid2.Clear();
	s_grid3.Clear();
	s_grid4.Clear();

#ifdef __ADDITIONAL_INVENTORY__
	s_upgradeGrid.Clear();
	s_bookGrid.Clear();
	s_stoneGrid.Clear();
	s_flowerGrid.Clear();
	s_attrGrid.Clear();
	s_chestGrid.Clear();
#endif
	LPCHARACTER	victim = GetCompany()->GetOwner();
	LPITEM item;

	int i;

	for (i = 0; i < INVENTORY_PAGE_SIZE * 1; ++i)
	{
		if (!(item = victim->GetInventoryItem(i)))
			continue;

		s_grid1.Put(i, 1, item->GetSize());
	}
	for (i = INVENTORY_PAGE_SIZE * 1; i < INVENTORY_PAGE_SIZE * 2; ++i)
	{
		if (!(item = victim->GetInventoryItem(i)))
			continue;

		s_grid2.Put(i - INVENTORY_PAGE_SIZE * 1, 1, item->GetSize());
	}
	for (i = INVENTORY_PAGE_SIZE * 2; i < INVENTORY_PAGE_SIZE * 3; ++i)
	{
		if (!(item = victim->GetInventoryItem(i)))
			continue;

		s_grid3.Put(i - INVENTORY_PAGE_SIZE * 2, 1, item->GetSize());
	}
	for (i = INVENTORY_PAGE_SIZE * 3; i < INVENTORY_PAGE_SIZE * 4; ++i)
	{
		if (!(item = victim->GetInventoryItem(i)))
			continue;

		s_grid4.Put(i - INVENTORY_PAGE_SIZE * 3, 1, item->GetSize());
	}

#ifdef __ADDITIONAL_INVENTORY__
	for (BYTE bStorageType = 0; bStorageType < 6; bStorageType++)
	{
		for (size_t ii = 0; ii < SPECIAL_INVENTORY_MAX_NUM; ii++)
		{
			if (bStorageType == 0)
				item = victim->GetUpgradeInventoryItem(ii);
			else if (bStorageType == 1)
				item = victim->GetBookInventoryItem(ii);
			else if (bStorageType == 2)
				item = victim->GetStoneInventoryItem(ii);
			else if (bStorageType == 3)
				item = victim->GetFlowerInventoryItem(ii);
			else if (bStorageType == 4)
				item = victim->GetAttrInventoryItem(ii);
			else if (bStorageType == 5)
				item = victim->GetChestInventoryItem(ii);

			if (!item)
				continue;

			if (bStorageType == 0)
			{
				s_upgradeGrid.Put(ii, 1, item->GetSize());
			}
			else if (bStorageType == 1)
			{
				s_bookGrid.Put(ii, 1, item->GetSize());
			}
			else if (bStorageType == 2)
			{
				s_stoneGrid.Put(ii, 1, item->GetSize());
			}
			else if (bStorageType == 3)
			{
				s_flowerGrid.Put(ii, 1, item->GetSize());
			}
			else if (bStorageType == 4)
			{
				s_attrGrid.Put(ii, 1, item->GetSize());
			}
			else if (bStorageType == 5)
			{
				s_chestGrid.Put(ii, 1, item->GetSize());
			}
		}
	}
#endif

	bool bDSInitialized = false;

	static std::vector <WORD> s_vDSGrid(DRAGON_SOUL_INVENTORY_MAX_NUM);
	std::vector<WORD> exceptDSCells;
	exceptDSCells.clear();

	for (i = 0; i < EXCHANGE_ITEM_MAX_NUM; ++i)
	{
		if (!(item = m_apItems[i]))
			continue;

		if (item->IsDragonSoul())
		{
			if (!victim->DragonSoul_IsQualified())
			{
				return false;
			}

			if (!bDSInitialized)
			{
				bDSInitialized = true;
				victim->CopyDragonSoulItemGrid(s_vDSGrid);
			}

			bool bExistEmptySpace = false;
			WORD wBasePos = DSManager::instance().GetBasePosition(item);
			if (wBasePos >= DRAGON_SOUL_INVENTORY_MAX_NUM)
				return false;

			for (int i = 0; i < DRAGON_SOUL_BOX_SIZE; i++)
			{
				WORD wPos = wBasePos + i;
				if (0 == s_vDSGrid[wPos]) // @fixme159 (wBasePos to wPos)
				{
					bool bEmpty = true;
					for (int j = 1; j < item->GetSize(); j++)
					{
						if (s_vDSGrid[wPos + j * DRAGON_SOUL_BOX_COLUMN_NUM])
						{
							bEmpty = false;
							break;
						}
					}
					if (bEmpty)
					{
						for (int j = 0; j < item->GetSize(); j++)
						{
							s_vDSGrid[wPos + j * DRAGON_SOUL_BOX_COLUMN_NUM] =  wPos + 1;
						}
						bExistEmptySpace = true;
						break;
					}
				}
				if (bExistEmptySpace)
					break;
			}
			if (!bExistEmptySpace)
				return false;
		}
#ifdef __ADDITIONAL_INVENTORY__
		else if (item->IsUpgradeItem())
		{
			BYTE itemSize = item->GetSize();
			int iPos = s_upgradeGrid.FindBlank(1, itemSize);
			if (iPos >= 0)
			{
				s_upgradeGrid.Put(iPos, 1, itemSize);
				continue;
			}

			return false;
		}
		else if (item->IsBook())
		{
			BYTE itemSize = item->GetSize();
			int iPos = s_bookGrid.FindBlank(1, itemSize);
			if (iPos >= 0)
			{
				s_bookGrid.Put(iPos, 1, itemSize);
				continue;
			}

			return false;
		}
		else if (item->IsStone())
		{
			BYTE itemSize = item->GetSize();
			int iPos = s_stoneGrid.FindBlank(1, itemSize);
			if (iPos >= 0)
			{
				s_stoneGrid.Put(iPos, 1, itemSize);
				continue;
			}

			return false;
		}
		else if (item->IsFlower())
		{
			BYTE itemSize = item->GetSize();
			int iPos = s_flowerGrid.FindBlank(1, itemSize);
			if (iPos >= 0)
			{
				s_flowerGrid.Put(iPos, 1, itemSize);
				continue;
			}

			return false;
		}
		else if (item->IsAttrItem())
		{
			BYTE itemSize = item->GetSize();
			int iPos = s_attrGrid.FindBlank(1, itemSize);
			if (iPos >= 0)
			{
				s_attrGrid.Put(iPos, 1, itemSize);
				continue;
			}

			return false;
		}
		else if (item->IsChest())
		{
			BYTE itemSize = item->GetSize();
			int iPos = s_chestGrid.FindBlank(1, itemSize);
			if (iPos >= 0)
			{
				s_chestGrid.Put(iPos, 1, itemSize);
				continue;
			}

			return false;
		}
#endif
		else
		{
			int iPos;

			if ((iPos = s_grid1.FindBlank(1, item->GetSize())) >= 0)
			{
				s_grid1.Put(iPos, 1, item->GetSize());
			}
			else if ((iPos = s_grid2.FindBlank(1, item->GetSize())) >= 0)
			{
				s_grid2.Put(iPos, 1, item->GetSize());
			}
			else if ((iPos = s_grid3.FindBlank(1, item->GetSize())) >= 0)
			{
				s_grid3.Put(iPos, 1, item->GetSize());
			}
			else if ((iPos = s_grid4.FindBlank(1, item->GetSize())) >= 0)
			{
				s_grid4.Put(iPos, 1, item->GetSize());
			}
			else
				return false;
		}
	}

	return true;
}

bool CExchange::Done(DWORD tradeID, bool firstPlayer)
{
	int		empty_pos, i;
	LPITEM	item;

	LPCHARACTER	victim = GetCompany()->GetOwner();

	for (i = 0; i < EXCHANGE_ITEM_MAX_NUM; ++i)
	{
		if (!(item = m_apItems[i]))
			continue;

		if (item->IsDragonSoul())
			empty_pos = victim->GetEmptyDragonSoulInventory(item);
#ifdef __ADDITIONAL_INVENTORY__
		else if (item->IsUpgradeItem())
			empty_pos = victim->GetEmptyUpgradeInventory(item);
		else if (item->IsBook())
			empty_pos = victim->GetEmptyBookInventory(item);
		else if (item->IsStone())
			empty_pos = victim->GetEmptyStoneInventory(item);
		else if (item->IsFlower())
			empty_pos = victim->GetEmptyFlowerInventory(item);
		else if (item->IsAttrItem())
			empty_pos = victim->GetEmptyAttrInventory(item);
		else if (item->IsChest())
			empty_pos = victim->GetEmptyChestInventory(item);
#endif
		else
			empty_pos = victim->GetEmptyInventory(item->GetSize());

		if (empty_pos < 0)
		{
			sys_err("Exchange::Done : Cannot find blank position in inventory %s <-> %s item %s",
				m_pOwner->GetName(), victim->GetName(), item->GetName());
			continue;
		}

		assert(empty_pos >= 0);
#ifdef __ADDITIONAL_INVENTORY__
		if (!item->IsDragonSoul() && !item->IsUpgradeItem() && !item->IsBook() && !item->IsStone() && !item->IsFlower() && !item->IsAttrItem() && !item->IsChest())
#endif
			m_pOwner->SyncQuickslot(QUICKSLOT_TYPE_ITEM, item->GetCell(), 255);

		item->RemoveFromCharacter();
		if (item->IsDragonSoul())
			item->__ADD_TO_CHARACTER(victim, TItemPos(DRAGON_SOUL_INVENTORY, empty_pos));
#ifdef __ADDITIONAL_INVENTORY__
		else if (item->IsUpgradeItem())
			item->__ADD_TO_CHARACTER(victim, TItemPos(UPGRADE_INVENTORY, empty_pos));
		else if (item->IsBook())
			item->__ADD_TO_CHARACTER(victim, TItemPos(BOOK_INVENTORY, empty_pos));
		else if (item->IsStone())
			item->__ADD_TO_CHARACTER(victim, TItemPos(STONE_INVENTORY, empty_pos));
		else if (item->IsFlower())
			item->__ADD_TO_CHARACTER(victim, TItemPos(FLOWER_INVENTORY, empty_pos));
		else if (item->IsAttrItem())
			item->__ADD_TO_CHARACTER(victim, TItemPos(ATTR_INVENTORY, empty_pos));
		else if (item->IsChest())
			item->__ADD_TO_CHARACTER(victim, TItemPos(CHEST_INVENTORY, empty_pos));
#endif
		else
			item->__ADD_TO_CHARACTER(victim, TItemPos(INVENTORY, empty_pos));

		ITEM_MANAGER::instance().FlushDelayedSave(item);

		item->SetExchanging(false);
		LogManager::instance().ExchangeItemLog(tradeID, item, firstPlayer ? GetOwner()->GetName() : victim->GetName());

		m_apItems[i] = NULL;
	}

	if (m_lGold)
	{
		GetOwner()->PointChange(POINT_GOLD, -m_lGold, true);
		victim->PointChange(POINT_GOLD, m_lGold, true);
	}

#ifdef __CHEQUE_SYSTEM__
	if (m_bCheque)
	{
		GetOwner()->PointChange(POINT_CHEQUE, -m_bCheque, true);
		victim->PointChange(POINT_CHEQUE, m_bCheque, true);
	}
#endif

	m_pGrid->Clear();
	return true;
}

bool CExchange::Accept(bool bAccept)
{
	if (m_bAccept == bAccept)
		return true;

	m_bAccept = bAccept;

	LPCHARACTER	victim = GetCompany()->GetOwner();

	if (m_bAccept && GetCompany()->m_bAccept)
	{
		int	iItemCount;

		//PREVENT_PORTAL_AFTER_EXCHANGE
		GetOwner()->SetExchangeTime();
		victim->SetExchangeTime();
		//END_PREVENT_PORTAL_AFTER_EXCHANGE

		// @duzenleme
		// gorev penceresi acik iken bu islemin yapilmamasini saglar fakat bu kontroller detaylandirilabilir.
		if (quest::CQuestManager::instance().GetPCForce(GetOwner()->GetPlayerID())->IsRunning() == true)
		{
			GetOwner()->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You cannot trade if you're using quests"));
			victim->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You cannot trade if the other part using quests"));
			goto EXCHANGE_END;
		}
		else if (quest::CQuestManager::instance().GetPCForce(victim->GetPlayerID())->IsRunning() == true)
		{
			victim->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You cannot trade if you're using quests"));
			GetOwner()->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You cannot trade if the other part using quests"));
			goto EXCHANGE_END;
		}

		if (!Check(&iItemCount))
		{
			GetOwner()->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("돈이 부족하거나 아이템이 제자리에 없습니다."));
			victim->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("상대방의 돈이 부족하거나 아이템이 제자리에 없습니다."));
			goto EXCHANGE_END;
		}

		if (!CheckSpace())
		{
			GetOwner()->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("상대방의 소지품에 빈 공간이 없습니다."));
			victim->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("소지품에 빈 공간이 없습니다."));
			goto EXCHANGE_END;
		}

		if (!GetCompany()->Check(&iItemCount))
		{
			victim->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("돈이 부족하거나 아이템이 제자리에 없습니다."));
			GetOwner()->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("상대방의 돈이 부족하거나 아이템이 제자리에 없습니다."));
			goto EXCHANGE_END;
		}

		if (!GetCompany()->CheckSpace())
		{
			victim->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("상대방의 소지품에 빈 공간이 없습니다."));
			GetOwner()->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("소지품에 빈 공간이 없습니다."));
			goto EXCHANGE_END;
		}

		if (db_clientdesc->GetSocket() == INVALID_SOCKET)
		{
			sys_err("Cannot use exchange feature while DB cache connection is dead.");
			victim->ChatPacket(CHAT_TYPE_INFO, "Unknown error");
			GetOwner()->ChatPacket(CHAT_TYPE_INFO, "Unknown error");
			goto EXCHANGE_END;
		}

		//LPCHARACTER otherPlayer = GetCompany()->GetOwner();


		{
#if defined(__GOLD_LIMIT_REWORK__) && defined(__HWID_SECURITY_UTILITY__) && defined(__CHEQUE_SYSTEM__)
			LPDESC ownerDesc = GetOwner()->GetDesc();
			LPDESC victimDesc = victim->GetDesc();

			DWORD tradeID = LogManager::instance().ExchangeLog(EXCHANGE_TYPE_TRADE, 0, victim->GetPlayerID(), victim->GetName(), GetOwner()->GetPlayerID(), GetOwner()->GetName(), GetOwner()->GetX(), GetOwner()->GetY(), GetCompany()->m_lGold, m_lGold, GetCompany()->m_bCheque, m_bCheque, victimDesc->GetMacAddr(), ownerDesc->GetMacAddr(), victimDesc->GetHDDSerial(), ownerDesc->GetHDDSerial());
#else
			DWORD tradeID = LogManager::instance().ExchangeLog(EXCHANGE_TYPE_TRADE, 0, GetOwner()->GetPlayerID(), victim->GetPlayerID(), GetOwner()->GetX(), GetOwner()->GetY(), m_lGold, GetCompany()->m_lGold);
#endif

			if (Done(tradeID, true))
			{
				GetOwner()->Save();

				if (GetCompany()->Done(tradeID, false))
				{
					victim->Save();

					// INTERNATIONAL_VERSION
					GetOwner()->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%s 님과의 교환이 성사 되었습니다."), victim->GetName());
					victim->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%s 님과의 교환이 성사 되었습니다."), GetOwner()->GetName());
					// END_OF_INTERNATIONAL_VERSION
				}

				if (victim && GetOwner())
				{
					if (m_lGold >= 1000000000)
					{
						char buf[512];
						snprintf(buf, sizeof(buf), "High Gold %lld Yang, TargetName : %s , MeLevel : %d", m_lGold, victim->GetName(), GetOwner()->GetLevel());
						LogManager::instance().PayToSellLog(GetOwner(), buf);
					}


	#ifdef __CHEQUE_SYSTEM__
					if (m_bCheque >= 100)
					{
						char buf[512];
						snprintf(buf, sizeof(buf), "High Cheque %d Won, TargetName : %s , MeLevel : %d", m_bCheque, victim->GetName(), GetOwner()->GetLevel());
						LogManager::instance().PayToSellLog(GetOwner(), buf);
					}
	#endif
				}
			}
		}


EXCHANGE_END:
		Cancel();
		return false;
	}
	else
	{
		exchange_packet(GetOwner(), EXCHANGE_SUBHEADER_GC_ACCEPT, true, m_bAccept, NPOS, 0);
		exchange_packet(GetCompany()->GetOwner(), EXCHANGE_SUBHEADER_GC_ACCEPT, false, m_bAccept, NPOS, 0);
		return true;
	}
}

void CExchange::Cancel()
{
	exchange_packet(GetOwner(), EXCHANGE_SUBHEADER_GC_END, 0, 0, NPOS, 0);
	GetOwner()->SetExchange(NULL);

	for (int i = 0; i < EXCHANGE_ITEM_MAX_NUM; ++i)
	{
		if (m_apItems[i])
			m_apItems[i]->SetExchanging(false);
	}

	if (GetCompany())
	{
		GetCompany()->SetCompany(NULL);
		GetCompany()->Cancel();
	}

	M2_DELETE(this);
}
