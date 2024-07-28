#include "stdafx.h"
#include "../../libgame/include/grid.h"
#include "constants.h"
#include "utils.h"
#include "config.h"
#include "shop.h"
#include "desc.h"
#include "desc_manager.h"
#include "char.h"
#include "char_manager.h"
#include "item.h"
#include "item_manager.h"
#include "buffer_manager.h"
#include "packet.h"
#include "log.h"
#include "db.h"
#include "questmanager.h"
#include "mob_manager.h"
#include "locale_service.h"

#ifdef __BATTLE_PASS_SYSTEM__
#include "battle_pass.h"
#endif

/* ------------------------------------------------------------------------------------ */
CShop::CShop()
	: m_dwVnum(0), m_dwNPCVnum(0), m_pkPC(NULL)
{
	m_pGrid = M2_NEW CGrid(SHOP_GRID_WIDTH, SHOP_GRID_HEIGHT);
}

CShop::~CShop()
{
	TPacketGCShop pack;

	pack.header = HEADER_GC_SHOP;
	pack.subheader = SHOP_SUBHEADER_GC_END;
	pack.size = sizeof(TPacketGCShop);

	Broadcast(&pack, sizeof(pack));

	GuestMapType::iterator it;

	it = m_map_guest.begin();

	while (it != m_map_guest.end())
	{
		LPCHARACTER ch = it->first;
		ch->SetShop(NULL);
		++it;
	}

	M2_DELETE(m_pGrid);
}

void CShop::SetPCShop(LPCHARACTER ch)
{
	m_pkPC = ch;
}

bool CShop::Create(DWORD dwVnum, DWORD dwNPCVnum, TShopItemTable* pTable)
{
	sys_log(0, "SHOP #%d (Shopkeeper %d)", dwVnum, dwNPCVnum);

	m_dwVnum = dwVnum;
	m_dwNPCVnum = dwNPCVnum;

#ifdef __ITEM_COUNT_LIMIT__
	DWORD bItemCount;
#else
	BYTE bItemCount;
#endif

	for (bItemCount = 0; bItemCount < SHOP_HOST_ITEM_MAX_NUM; ++bItemCount)
		if (0 == (pTable + bItemCount)->vnum)
			break;

	SetShopItems(pTable, bItemCount);
	return true;
}

#ifdef __ITEM_COUNT_LIMIT__
void CShop::SetShopItems(TShopItemTable* pTable, DWORD bItemCount)
#else
void CShop::SetShopItems(TShopItemTable* pTable, BYTE bItemCount)
#endif
{
	if (bItemCount > SHOP_HOST_ITEM_MAX_NUM)
		return;

	m_pGrid->Clear();

	m_itemVector.resize(SHOP_HOST_ITEM_MAX_NUM);
	msl::refill(m_itemVector);

	for (int i = 0; i < bItemCount; ++i)
	{
		LPITEM pkItem = NULL;
		const TItemTable* item_table;

		if (m_pkPC)
		{
			pkItem = m_pkPC->GetItem(pTable->pos);

			if (!pkItem)
			{
				sys_err("cannot find item on pos (%d, %d) (name: %s)", pTable->pos.window_type, pTable->pos.cell, m_pkPC->GetName());
				continue;
			}

			item_table = pkItem->GetProto();
		}
		else
		{
			if (!pTable->vnum)
				continue;

			item_table = ITEM_MANAGER::instance().GetTable(pTable->vnum);
		}

		if (!item_table)
		{
			sys_err("Shop: no item table by item vnum #%d", pTable->vnum);
			continue;
		}

		int iPos;

		if (IsPCShop())
		{
			sys_log(0, "MyShop: use position %d", pTable->display_pos);
			iPos = pTable->display_pos;
		}
		else
			iPos = m_pGrid->FindBlank(1, item_table->bSize);

		if (iPos < 0)
		{
			sys_err("not enough shop window");
			continue;
		}

		if (!m_pGrid->IsEmpty(iPos, 1, item_table->bSize))
		{
			if (IsPCShop())
			{
				sys_err("not empty position for pc shop %s[%d]", m_pkPC->GetName(), m_pkPC->GetPlayerID());
			}
			else
			{
				sys_err("not empty position for npc shop");
			}
			continue;
		}

		m_pGrid->Put(iPos, 1, item_table->bSize);

		SHOP_ITEM& item = m_itemVector[iPos];

		item.pkItem = pkItem;
		item.itemid = 0;

		if (item.pkItem)
		{
			item.vnum = pkItem->GetVnum();
			item.count = pkItem->GetCount();
			item.price = pTable->price;
#ifdef __CHEQUE_SYSTEM__
			item.cheque = pTable->cheque;
#endif
			item.itemid = pkItem->GetID();
		}
		else
		{
			item.vnum = pTable->vnum;
			item.count = pTable->count;
#ifdef __SHOP_PRICE_SQL__
			item.price = pTable->price;
#ifdef __CHEQUE_SYSTEM__
			item.cheque = pTable->cheque;
#endif
			if (item.price == -1)//Tablodaki deger -1 ise protodan calisir
#endif
			{
				if (IS_SET(item_table->dwFlags, ITEM_FLAG_COUNT_PER_1GOLD))
				{
#ifdef __GOLD_LIMIT_REWORK__
					if (item_table->llGold == 0)
						item.price = item.count;
					else
						item.price = item.count / item_table->llGold;
				}
				else
					item.price = item_table->llGold * item.count;
#else
					if (item_table->dwGold == 0)
						item.price = item.count;
					else
						item.price = item.count / item_table->dwGold;
			}
				else
					item.price = item_table->dwGold * item.count;
#endif
		}
#ifdef __SHOP_PRICE_TYPE_ITEM__
			item.witemVnum = pTable->witemVnum;
#endif
	}
		++pTable;
}
}
int CShop::Buy(LPCHARACTER ch, BYTE pos)
{
	if (pos >= m_itemVector.size())
	{
		sys_log(0, "Shop::Buy : invalid position %d : %s", pos, ch->GetName());
		return SHOP_SUBHEADER_GC_INVALID_POS;
	}

	sys_log(0, "Shop::Buy : name %s pos %d", ch->GetName(), pos);

	GuestMapType::iterator it = m_map_guest.find(ch);

	if (it == m_map_guest.end())
		return SHOP_SUBHEADER_GC_END;

	SHOP_ITEM& r_item = m_itemVector[pos];
#ifdef __SHOP_PRICE_TYPE_ITEM__
	DWORD dwWItemVnum = r_item.witemVnum;
#endif
#ifdef __GOLD_LIMIT_REWORK__
	long long llPrice = r_item.price;
#else
	DWORD dwPrice = r_item.price;
#endif
#ifdef __SHOP_PRICE_TYPE_ITEM__
	if (dwWItemVnum > 0)
	{
#ifdef __GOLD_LIMIT_REWORK__
		if (ch->CountSpecifyItem(dwWItemVnum) < llPrice)
#else
		if (ch->CountSpecifyItem(dwWItemVnum) < dwPrice)
#endif
		{
			return SHOP_SUBHEADER_GC_NOT_ENOUGH_ITEM;
		}
	}
#ifdef __GOLD_LIMIT_REWORK__
	else if (ch->GetGold() < (long long)llPrice)
#else
	else if (ch->GetGold() < (int)dwPrice)
#endif
#else
#ifdef __GOLD_LIMIT_REWORK__
	if (ch->GetGold() < llPrice)
#else
	if (ch->GetGold() < dwPrice)
#endif
#endif
	{
#ifdef __GOLD_LIMIT_REWORK__
		sys_log(1, "Shop::Buy : Not enough money : %s has %lld, price %lld", ch->GetName(), ch->GetGold(), llPrice);
#else
		sys_log(1, "Shop::Buy : Not enough money : %s has %d, price %d", ch->GetName(), ch->GetGold(), dwPrice);
#endif
		return SHOP_SUBHEADER_GC_NOT_ENOUGH_MONEY;
	}
	LPITEM pkSelectedItem = ITEM_MANAGER::instance().Find(r_item.itemid);

	if (IsPCShop())
	{
		if (!pkSelectedItem)
		{
			sys_log(0, "Shop::Buy : Critical: This user seems to be a hacker : invalid pcshop item : BuyerPID:%d SellerPID:%d",
				ch->GetPlayerID(),
				m_pkPC->GetPlayerID());

			return SHOP_SUBHEADER_GC_SOLD_OUT; // @duzenleme eger iki adam ayni anda pazardan item alirsa birisini oyundan atiyordu.
		}

		if ((pkSelectedItem->GetOwner() != m_pkPC))
		{
			sys_log(0, "Shop::Buy : Critical: This user seems to be a hacker : invalid pcshop item : BuyerPID:%d SellerPID:%d",
				ch->GetPlayerID(),
				m_pkPC->GetPlayerID());

			return SHOP_SUBHEADER_GC_SOLD_OUT; // @duzenleme eger iki adam ayni anda pazardan item alirsa birisini oyundan atiyordu.
		}
	}

#ifdef __CHEQUE_SYSTEM__
	DWORD dwCheque = r_item.cheque;
#ifdef __GOLD_LIMIT_REWORK__
	if (ch->GetCheque() < (int)dwCheque && ch->GetGold() < (long long)llPrice)
	{
		sys_log(1, "Shop::Buy : Not enough cheque and money : %s has %d, cheque %d, price %lld", ch->GetName(), ch->GetCheque(), dwCheque, llPrice);
		return SHOP_SUBHEADER_GC_NOT_ENOUGH_CHEQUE_MONEY;
	}
#else
	if (ch->GetCheque() < (int)dwCheque && ch->GetGold() < (int)dwPrice)
	{
		sys_log(1, "Shop::Buy : Not enough cheque and money : %s has %d, cheque %d, price %d", ch->GetName(), ch->GetCheque(), dwCheque, dwPrice);
		return SHOP_SUBHEADER_GC_NOT_ENOUGH_CHEQUE_MONEY;
	}
#endif
	if (ch->GetCheque() < (int)dwCheque)
	{
		sys_log(1, "Shop::Buy : Not enough cheque : %s has %d, price %d", ch->GetName(), ch->GetCheque(), dwCheque);
		return SHOP_SUBHEADER_GC_NOT_ENOUGH_CHEQUE;
	}
#endif

	//if (it->second)	// if other empire, price is triple
	//	dwPrice *= 3;

#ifdef __GOLD_LIMIT_REWORK__
	if (ch->GetGold() < (long long)llPrice)
	{
		sys_log(1, "Shop::Buy : Not enough money : %s has %lld, price %lld", ch->GetName(), ch->GetGold(), llPrice);
		return SHOP_SUBHEADER_GC_NOT_ENOUGH_MONEY;
	}
#else
	if (ch->GetGold() < (int)dwPrice)
	{
		sys_log(1, "Shop::Buy : Not enough money : %s has %d, price %d", ch->GetName(), ch->GetGold(), dwPrice);
		return SHOP_SUBHEADER_GC_NOT_ENOUGH_MONEY;
	}
#endif

	LPITEM item;

	if (m_pkPC)
		item = r_item.pkItem;
	else
		item = ITEM_MANAGER::instance().CreateItem(r_item.vnum, r_item.count);

	if (!item)
		return SHOP_SUBHEADER_GC_SOLD_OUT;

	int iEmptyPos;
	if (item->IsDragonSoul())
		iEmptyPos = ch->GetEmptyDragonSoulInventory(item);
#ifdef __ADDITIONAL_INVENTORY__
	else if (item->IsUpgradeItem())
		iEmptyPos = ch->GetEmptyUpgradeInventory(item);
	else if (item->IsBook())
		iEmptyPos = ch->GetEmptyBookInventory(item);
	else if (item->IsStone())
		iEmptyPos = ch->GetEmptyStoneInventory(item);
	else if (item->IsFlower())
		iEmptyPos = ch->GetEmptyFlowerInventory(item);
	else if (item->IsAttrItem())
		iEmptyPos = ch->GetEmptyAttrInventory(item);
	else if (item->IsChest())
		iEmptyPos = ch->GetEmptyChestInventory(item);
#endif
	else
		iEmptyPos = ch->GetEmptyInventory(item->GetSize());

	if (iEmptyPos < 0)
	{
		if (m_pkPC)
		{
			sys_log(1, "Shop::Buy at PC Shop : Inventory full : %s size %d", ch->GetName(), item->GetSize());
			return SHOP_SUBHEADER_GC_INVENTORY_FULL;
		}
		else
		{
			sys_log(1, "Shop::Buy : Inventory full : %s size %d", ch->GetName(), item->GetSize());
			M2_DESTROY_ITEM(item);
			return SHOP_SUBHEADER_GC_INVENTORY_FULL;
		}
	}

#ifdef __BATTLE_PASS_SYSTEM__
	bool bIsBattlePass = false;
	long dwPriceCount = 0;
#endif

#ifdef __SHOP_PRICE_TYPE_ITEM__
	if (dwWItemVnum > 0)
#ifdef __GOLD_LIMIT_REWORK__
		ch->RemoveSpecifyItem(dwWItemVnum, llPrice);
#else
		ch->RemoveSpecifyItem(dwWItemVnum, dwPrice);
#endif
	else
#ifdef __GOLD_LIMIT_REWORK__
		ch->PointChange(POINT_GOLD, -llPrice, false);
#else
		ch->PointChange(POINT_GOLD, -dwPrice, false);
#endif
#else
#ifdef __GOLD_LIMIT_REWORK__
	ch->PointChange(POINT_GOLD, -llPrice, false);
#else
	ch->PointChange(POINT_GOLD, -dwPrice, false);
#endif
#endif
#ifdef __CHEQUE_SYSTEM__
	if (dwCheque > 0)
		ch->PointChange(POINT_CHEQUE, -(int)dwCheque, false);
#endif

#ifdef __BATTLE_PASS_SYSTEM__
	if (m_pkPC)
	{
		bIsBattlePass = true;
		dwPriceCount = llPrice;
	}
#endif

#ifdef __GOLD_LIMIT_REWORK__
	long long iVal = 0;
	long long dwTax = 0;
#else
	int iVal = 0;
	DWORD dwTax = 0;
#endif

	{
		iVal = 3;

		if (0 < iVal)
		{
			if (iVal > 100)
				iVal = 100;
#ifdef __GOLD_LIMIT_REWORK__
			dwTax = llPrice * iVal / 100;
			llPrice = llPrice - dwTax;
#else
			dwTax = dwPrice * iVal / 100;
			dwPrice = dwPrice - dwTax;
#endif
		}
		else
		{
			iVal = 0;
			dwTax = 0;
		}
	}

	if (m_pkPC)
	{
#ifdef __ADDITIONAL_INVENTORY__
		if (!item->IsDragonSoul() && !item->IsUpgradeItem() && !item->IsBook() && !item->IsStone() && !item->IsFlower() && !item->IsAttrItem() && !item->IsChest())
#endif
			m_pkPC->SyncQuickslot(QUICKSLOT_TYPE_ITEM, item->GetCell(), 255);

		char buf[512];
		if (item->GetVnum() >= 80003 && item->GetVnum() <= 80007)
		{
#ifdef __GOLD_LIMIT_REWORK__
			snprintf(buf, sizeof(buf), "%s FROM: %u TO: %u PRICE: %lld", item->GetName(), ch->GetPlayerID(), m_pkPC->GetPlayerID(), llPrice);
#else
			snprintf(buf, sizeof(buf), "%s FROM: %u TO: %u PRICE: %u", item->GetName(), ch->GetPlayerID(), m_pkPC->GetPlayerID(), dwPrice);
#endif
		}

		item->RemoveFromCharacter();
		if (item->IsDragonSoul())
			item->__ADD_TO_CHARACTER(ch, TItemPos(DRAGON_SOUL_INVENTORY, iEmptyPos));
#ifdef __ADDITIONAL_INVENTORY__
		else if (item->IsUpgradeItem())
			item->__ADD_TO_CHARACTER(ch, TItemPos(UPGRADE_INVENTORY, iEmptyPos));
		else if (item->IsBook())
			item->__ADD_TO_CHARACTER(ch, TItemPos(BOOK_INVENTORY, iEmptyPos));
		else if (item->IsStone())
			item->__ADD_TO_CHARACTER(ch, TItemPos(STONE_INVENTORY, iEmptyPos));
		else if (item->IsFlower())
			item->__ADD_TO_CHARACTER(ch, TItemPos(FLOWER_INVENTORY, iEmptyPos));
		else if (item->IsAttrItem())
			item->__ADD_TO_CHARACTER(ch, TItemPos(ATTR_INVENTORY, iEmptyPos));
		else if (item->IsChest())
			item->__ADD_TO_CHARACTER(ch, TItemPos(CHEST_INVENTORY, iEmptyPos));
#endif
		else
			item->__ADD_TO_CHARACTER(ch, TItemPos(INVENTORY, iEmptyPos));

		ITEM_MANAGER::instance().FlushDelayedSave(item);

		r_item.pkItem = NULL;

		BroadcastUpdateItem(pos);

#ifdef __GOLD_LIMIT_REWORK__
		m_pkPC->PointChange(POINT_GOLD, llPrice, false);
#else
		m_pkPC->PointChange(POINT_GOLD, dwPrice, false);
#endif
#ifdef __CHEQUE_SYSTEM__
		m_pkPC->PointChange(POINT_CHEQUE, dwCheque, false);
#endif
		if (iVal > 0)
			m_pkPC->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("판매금액의 %d %% 가 세금으로 나가게됩니다"), iVal);

#ifdef __BATTLE_PASS_SYSTEM__
		if(bIsBattlePass)
		{
			BYTE bBattlePassId = ch->GetBattlePassId();
			if(bBattlePassId)
			{
				DWORD dwYangCount, dwNotUsed;
				if(CBattlePass::instance().BattlePassMissionGetInfo(bBattlePassId, SPENT_YANG, &dwNotUsed, &dwYangCount))
				{
					if(ch->GetMissionProgress(SPENT_YANG, bBattlePassId) < dwYangCount)
						ch->UpdateMissionProgress(SPENT_YANG, bBattlePassId, dwPriceCount, dwYangCount);
				}
			}
		}
#endif

	}
	else
	{
		if (item->IsDragonSoul())
			item->__ADD_TO_CHARACTER(ch, TItemPos(DRAGON_SOUL_INVENTORY, iEmptyPos));
#ifdef __ADDITIONAL_INVENTORY__
		else if (item->IsUpgradeItem())
			item->__ADD_TO_CHARACTER(ch, TItemPos(UPGRADE_INVENTORY, iEmptyPos));
		else if (item->IsBook())
			item->__ADD_TO_CHARACTER(ch, TItemPos(BOOK_INVENTORY, iEmptyPos));
		else if (item->IsStone())
			item->__ADD_TO_CHARACTER(ch, TItemPos(STONE_INVENTORY, iEmptyPos));
		else if (item->IsFlower())
			item->__ADD_TO_CHARACTER(ch, TItemPos(FLOWER_INVENTORY, iEmptyPos));
		else if (item->IsAttrItem())
			item->__ADD_TO_CHARACTER(ch, TItemPos(ATTR_INVENTORY, iEmptyPos));
		else if (item->IsChest())
			item->__ADD_TO_CHARACTER(ch, TItemPos(CHEST_INVENTORY, iEmptyPos));
#endif
		else
			item->__ADD_TO_CHARACTER(ch, TItemPos(INVENTORY, iEmptyPos));
		ITEM_MANAGER::instance().FlushDelayedSave(item);
	}

	if (item)
#ifdef __GOLD_LIMIT_REWORK__
		sys_log(0, "SHOP: BUY: name %s %s(x %d):%u price %lld", ch->GetName(), item->GetName(), item->GetCount(), item->GetID(), llPrice);
#else
		sys_log(0, "SHOP: BUY: name %s %s(x %d):%u price %u", ch->GetName(), item->GetName(), item->GetCount(), item->GetID(), dwPrice);
#endif

	ch->Save();

	return (SHOP_SUBHEADER_GC_OK);
}

#ifdef ENABLE_REMOTE_SHOP_SYSTEM
bool CShop::AddGuest(LPCHARACTER ch, DWORD owner_vid, bool bOtherEmpire, bool bRemoteShop)
#else
bool CShop::AddGuest(LPCHARACTER ch, DWORD owner_vid, bool bOtherEmpire)
#endif
{
	if (!ch)
		return false;

	if (ch->GetExchange())
		return false;

	if (ch->GetShop())
		return false;

	ch->SetShop(this);

	m_map_guest.insert(GuestMapType::value_type(ch, bOtherEmpire));

	TPacketGCShop pack;

	pack.header = HEADER_GC_SHOP;
	pack.subheader = SHOP_SUBHEADER_GC_START;

	TPacketGCShopStart pack2;

	memset(&pack2, 0, sizeof(pack2));
	pack2.owner_vid = owner_vid;
#ifdef ENABLE_REMOTE_SHOP_SYSTEM
	if (bRemoteShop)
		pack2.owner_vid = REMOTE_SHOP_VID;
#endif
	for (DWORD i = 0; i < m_itemVector.size() && i < SHOP_HOST_ITEM_MAX_NUM; ++i)
	{
		const SHOP_ITEM& item = m_itemVector[i];

		//END_HIVALUE_ITEM_EVENT
		if (m_pkPC && !item.pkItem)
			continue;

		pack2.items[i].vnum = item.vnum;

		pack2.items[i].price = item.price;

		pack2.items[i].count = item.count;

#ifdef __SHOP_PRICE_TYPE_ITEM__
		pack2.items[i].witemVnum = item.witemVnum;
#endif

#ifdef __CHEQUE_SYSTEM__
		pack2.items[i].cheque = item.cheque;
#endif

		if (item.pkItem)
		{
			thecore_memcpy(pack2.items[i].alSockets, item.pkItem->GetSockets(), sizeof(pack2.items[i].alSockets));
			thecore_memcpy(pack2.items[i].aAttr, item.pkItem->GetAttributes(), sizeof(pack2.items[i].aAttr));
#ifdef __ITEM_EVOLUTION__
			pack2.items[i].evolution = item.pkItem->GetEvolution();
#endif
#ifdef __ITEM_CHANGELOOK__
			pack2.items[i].transmutation = item.pkItem->GetTransmutation();
#endif
		}
	}

	pack.size = sizeof(pack) + sizeof(pack2);

	ch->GetDesc()->BufferedPacket(&pack, sizeof(TPacketGCShop));
	ch->GetDesc()->Packet(&pack2, sizeof(TPacketGCShopStart));
	return true;
}

void CShop::RemoveGuest(LPCHARACTER ch)
{
	if (ch->GetShop() != this)
		return;

	m_map_guest.erase(ch);
	ch->SetShop(NULL);

	TPacketGCShop pack;

	pack.header = HEADER_GC_SHOP;
	pack.subheader = SHOP_SUBHEADER_GC_END;
	pack.size = sizeof(TPacketGCShop);

	ch->GetDesc()->Packet(&pack, sizeof(pack));
}

void CShop::Broadcast(const void* data, int bytes)
{
	sys_log(1, "Shop::Broadcast %p %d", data, bytes);

	GuestMapType::iterator it;

	it = m_map_guest.begin();

	while (it != m_map_guest.end())
	{
		LPCHARACTER ch = it->first;

		if (ch->GetDesc())
			ch->GetDesc()->Packet(data, bytes);

		++it;
	}
}

void CShop::BroadcastUpdateItem(BYTE pos)
{
	TPacketGCShop pack;
	TPacketGCShopUpdateItem pack2;

	TEMP_BUFFER	buf;

	pack.header = HEADER_GC_SHOP;
	pack.subheader = SHOP_SUBHEADER_GC_UPDATE_ITEM;
	pack.size = sizeof(pack) + sizeof(pack2);

	pack2.pos = pos;

	if (m_pkPC && !m_itemVector[pos].pkItem)
		pack2.item.vnum = 0;
	else
	{
		pack2.item.vnum = m_itemVector[pos].vnum;
		if (m_itemVector[pos].pkItem)
		{
			thecore_memcpy(pack2.item.alSockets, m_itemVector[pos].pkItem->GetSockets(), sizeof(pack2.item.alSockets));
			thecore_memcpy(pack2.item.aAttr, m_itemVector[pos].pkItem->GetAttributes(), sizeof(pack2.item.aAttr));
#ifdef __ITEM_CHANGELOOK__
			pack2.item.transmutation = m_itemVector[pos].pkItem->GetTransmutation();
#endif
#ifdef __ITEM_EVOLUTION__
			pack2.item.evolution = m_itemVector[pos].pkItem->GetEvolution();
#endif
		}
		else
		{
			memset(pack2.item.alSockets, 0, sizeof(pack2.item.alSockets));
			memset(pack2.item.aAttr, 0, sizeof(pack2.item.aAttr));
		}
	}

	pack2.item.price = m_itemVector[pos].price;
	pack2.item.count = m_itemVector[pos].count;
#ifdef __SHOP_PRICE_TYPE_ITEM__
	pack2.item.witemVnum = m_itemVector[pos].witemVnum;
#endif
#ifdef __CHEQUE_SYSTEM__
	pack2.item.cheque = m_itemVector[pos].cheque;
#endif
	buf.write(&pack, sizeof(pack));
	buf.write(&pack2, sizeof(pack2));

	Broadcast(buf.read_peek(), buf.size());
}

int CShop::GetNumberByVnum(DWORD dwVnum)
{
	int itemNumber = 0;

	for (DWORD i = 0; i < m_itemVector.size() && i < SHOP_HOST_ITEM_MAX_NUM; ++i)
	{
		const SHOP_ITEM& item = m_itemVector[i];

		if (item.vnum == dwVnum)
		{
			itemNumber += item.count;
		}
	}

	return itemNumber;
}

bool CShop::IsSellingItem(DWORD itemID)
{
	bool isSelling = false;

	for (DWORD i = 0; i < m_itemVector.size() && i < SHOP_HOST_ITEM_MAX_NUM; ++i)
	{
		if ((unsigned int)(m_itemVector[i].itemid) == itemID)
		{
			isSelling = true;
			break;
		}
	}

	return isSelling;
}

#ifdef __RELOAD_REWORK__
void CShop::RemoveAllGuests()
{
	if (m_map_guest.empty())
		return;
	for (GuestMapType::iterator it = m_map_guest.begin(); it != m_map_guest.end(); it++)
	{
		LPCHARACTER ch = it->first;
		if (ch)
		{
			if (ch->GetDesc() && ch->GetShop() == this)
			{
				ch->SetShop(NULL);
				TPacketGCShop pack;
				pack.header = HEADER_GC_SHOP;
				pack.subheader = SHOP_SUBHEADER_GC_END;
				pack.size = sizeof(TPacketGCShop);
				ch->GetDesc()->Packet(&pack, sizeof(pack));
			}
		}
	}
	m_map_guest.clear();
}
#endif