#include "stdafx.h"
#include "char.h"
#include "utils.h"
#include "log.h"
#include "db.h"
#include "config.h"
#include "desc.h"
#include "desc_manager.h"
#include "buffer_manager.h"
#include "packet.h"
#include "desc_client.h"
#include "p2p.h"
#include "item.h"
#include "item_manager.h"
#include "item_shop.h"

const CItemShopManager::TItemShopTable* CItemShopManager::GetTable(DWORD id)
{
	TItemShopDataMap::iterator itor = m_ItemShopDataMap.find(id);

	if (itor == m_ItemShopDataMap.end())
		return NULL;

	return itor->second;
}

bool CItemShopManager::LoadItemShopTable(const char* stFileName, bool reload)
{
	FILE* fp = fopen(stFileName, "r");
	if (!fp)
		return false;

	DWORD	category = 0;
	DWORD	sub_category = 0;
	DWORD	id = 0;
	DWORD	vnum = 0;
	DWORD	count = 0;
	DWORD	coins = 0;
	DWORD	socketzero = 0;

	if (reload)
	{
		for (TItemShopDataMap::iterator itor = m_ItemShopDataMap.begin(); itor != m_ItemShopDataMap.end(); ++itor)
		{
			delete itor->second;
		}
		m_ItemShopDataMap.clear();
	}

	while (fscanf(fp, "%u\t%u\t%u\t%u\t%u\t%u\t%u", &category, &sub_category, &id, &vnum, &coins, &count, &socketzero) != EOF)
	{
		if (!ITEM_MANAGER::instance().GetTable(vnum))
		{
			sys_err("No such item (Vnum: %lu)", vnum);
			continue;
		}

		const TItemShopTable* p = GetTable(id);

		if (p)
		{
			sys_log(0, "Already Inserted List %d (ItemShop Table)", id);
			continue;
		}

		TItemShopTable* pItemShopData = new TItemShopTable;
		pItemShopData->category = category;
		pItemShopData->sub_category = sub_category;
		pItemShopData->id = id;
		pItemShopData->vnum = vnum;
		pItemShopData->count = count;
		pItemShopData->coins = coins;
		pItemShopData->socketzero = socketzero;
		m_ItemShopDataMap.insert(TItemShopDataMap::value_type(id, pItemShopData));
		sys_log(0, "ItemShop Insert ID:%d VNUM:%d COUNT:%d", id, vnum, count);
	}

	fclose(fp);
	return true;
}

// Send item data list client 
void CItemShopManager::SendClientPacket(LPCHARACTER ch)
{
	if (NULL == ch)
		return;

	if (!ch || !ch->GetDesc())
		return;
	
	ch->ChatPacket(CHAT_TYPE_COMMAND, "BINARY_ItemShop_Clear");

	for (auto itor=m_ItemShopDataMap.begin(); itor!=m_ItemShopDataMap.end(); ++itor)
	{
		TItemShopTable * pTable = itor->second;
		if (pTable)
		{
			
			TPacketItemShopData pack;
			pack.header = HEADER_GC_ITEM_SHOP;

			pack.id = pTable->id;
			pack.category = pTable->category;
			pack.sub_category = pTable->sub_category;
			pack.vnum = pTable->vnum;
			pack.count = pTable->count;
			pack.coins = pTable->coins;
			pack.socketzero = pTable->socketzero;

			ch->GetDesc()->Packet(&pack, sizeof(pack)); 
		}
	}
}

bool CItemShopManager::Buy(LPCHARACTER ch, DWORD id, DWORD count)
{
	if (!ch)
		return false;

	if (count <= 0)
		return false;

	const TItemShopTable* c_pTable = GetTable(id);

	if (!c_pTable)
	{
		sys_err("%s has request buy unknown id(%d) item", ch->GetName(), id);
		return false;
	}

	DWORD dwCoins = c_pTable->coins;
	DWORD dwVnum = c_pTable->vnum;
	DWORD dwCount = c_pTable->count;
	DWORD dwSocketZero = c_pTable->socketzero;

	DWORD dwRealCount = dwCount * count;

	if (ch->GetDragonCoin() < (dwCoins * count))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("nesnemarketyeterliepyok"));
		return false;
	}

	LPITEM pkItem = ITEM_MANAGER::instance().CreateItem(dwVnum, dwRealCount, 0, true, -1);

	if (!pkItem)
		return false;

	int iEmptyPos;
	if (pkItem->IsDragonSoul())
		iEmptyPos = ch->GetEmptyDragonSoulInventory(pkItem);
#ifdef __ADDITIONAL_INVENTORY__
	else if (pkItem->IsUpgradeItem())
		iEmptyPos = ch->GetEmptyUpgradeInventory(pkItem);
	else if (pkItem->IsBook())
		iEmptyPos = ch->GetEmptyBookInventory(pkItem);
	else if (pkItem->IsStone())
		iEmptyPos = ch->GetEmptyStoneInventory(pkItem);
	else if (pkItem->IsFlower())
		iEmptyPos = ch->GetEmptyFlowerInventory(pkItem);
	else if (pkItem->IsAttrItem())
		iEmptyPos = ch->GetEmptyAttrInventory(pkItem);
	else if (pkItem->IsChest())
		iEmptyPos = ch->GetEmptyChestInventory(pkItem);
#endif
	else
	{
		iEmptyPos = ch->GetEmptyInventory(pkItem->GetSize());
	}

	if (iEmptyPos < 0)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("nesnemarketenvanterdebosyer"));
		M2_DESTROY_ITEM(pkItem);
		return false;
	}

	ch->SetDragonCoin(ch->GetDragonCoin() - (dwCoins * count));

	if (pkItem->IsRealTimeItem())
		pkItem->SetSocket(0, get_global_time() + dwSocketZero);
	else if ((pkItem->GetType() == ITEM_ARMOR && pkItem->GetSubType() == ARMOR_BODY) || pkItem->GetType() == ITEM_WEAPON)
		pkItem->SetSocket(0, 1);
#ifdef __ACCE_SYSTEM__
	else if (pkItem->GetType() == ITEM_COSTUME && pkItem->GetSubType() == COSTUME_ACCE)
		pkItem->SetSocket(0, 0);
#endif
	else if (pkItem->GetType() == ITEM_BELT)
	{
		pkItem->SetSocket(0, 1);
	}
	else
	{
		if (pkItem->GetType() != ITEM_BLEND && !pkItem->IsAccessoryForSocket())
			pkItem->SetSocket(0, dwSocketZero);
	}

	if (pkItem->IsDragonSoul())
		pkItem->__ADD_TO_CHARACTER(ch, TItemPos(DRAGON_SOUL_INVENTORY, iEmptyPos));
#ifdef __ADDITIONAL_INVENTORY__
	else if (pkItem->IsUpgradeItem())
		pkItem->__ADD_TO_CHARACTER(ch, TItemPos(UPGRADE_INVENTORY, iEmptyPos));
	else if (pkItem->IsBook())
		pkItem->__ADD_TO_CHARACTER(ch, TItemPos(BOOK_INVENTORY, iEmptyPos));
	else if (pkItem->IsStone())
		pkItem->__ADD_TO_CHARACTER(ch, TItemPos(STONE_INVENTORY, iEmptyPos));
	else if (pkItem->IsFlower())
		pkItem->__ADD_TO_CHARACTER(ch, TItemPos(FLOWER_INVENTORY, iEmptyPos));
	else if (pkItem->IsAttrItem())
		pkItem->__ADD_TO_CHARACTER(ch, TItemPos(ATTR_INVENTORY, iEmptyPos));
	else if (pkItem->IsChest())
		pkItem->__ADD_TO_CHARACTER(ch, TItemPos(CHEST_INVENTORY, iEmptyPos));
#endif
	else
		pkItem->__ADD_TO_CHARACTER(ch, TItemPos(INVENTORY, iEmptyPos));

	long esya_ep = (dwCoins * count);
	LogManager::instance().ItemShopLog(ch, pkItem, "ITEM_SHOP", esya_ep);
	ITEM_MANAGER::instance().FlushDelayedSave(pkItem);
	ch->Save(); // karakteride kaydedelim.

	return true;
}

void CItemShopManager::Destroy()
{
	for (TItemShopDataMap::iterator itor = m_ItemShopDataMap.begin(); itor != m_ItemShopDataMap.end(); ++itor)
	{
		delete itor->second;
	}
	m_ItemShopDataMap.clear();
}

CItemShopManager::CItemShopManager()
{
}

CItemShopManager::~CItemShopManager()
{
	Destroy();
}