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
#include "desc_client.h"
#include "shop_manager.h"
#include "group_text_parse_tree.h"
#include "shop_manager.h"

#ifdef __BATTLE_PASS_SYSTEM__
#include "battle_pass.h"
#endif

CShopManager::CShopManager()
{
}

CShopManager::~CShopManager()
{
	Destroy();
}

bool CShopManager::Initialize(TShopTable* table, int size)
{
	if (!m_map_pkShop.empty())
#ifdef __RELOAD_REWORK__
	{
		for (TShopMap::iterator it = m_map_pkShop.begin(); it != m_map_pkShop.end(); it++)
		{
			it->second->RemoveAllGuests();
		}
	}

	m_map_pkShop.clear();
	m_map_pkShopByNPCVnum.clear();
#else
		return false;
#endif

	int i;

	for (i = 0; i < size; ++i, ++table)
	{
		LPSHOP shop = M2_NEW CShop;

		if (!shop->Create(table->dwVnum, table->dwNPCVnum, table->items))
		{
			M2_DELETE(shop);
			continue;
		}

		m_map_pkShop.insert(TShopMap::value_type(table->dwVnum, shop));
		m_map_pkShopByNPCVnum.insert(TShopMap::value_type(table->dwNPCVnum, shop));
	}

	return true;
}

// @duzenleme memoryleak suphesi mevcuttu bu yuzden blackdragonx61'in bu kodunu ekledik fakat bu kod shopex icin calismakta sanirim.
// ama bizde shopex mevcut degil.
void CShopManager::Destroy()
{
	for (auto it = m_map_pkShopByNPCVnum.begin(); it != m_map_pkShopByNPCVnum.end(); ++it)
		delete it->second;
	m_map_pkShopByNPCVnum.clear();
	m_map_pkShop.clear();
}

LPSHOP CShopManager::Get(DWORD dwVnum)
{
	TShopMap::const_iterator it = m_map_pkShop.find(dwVnum);

	if (it == m_map_pkShop.end())
		return NULL;

	return (it->second);
}

LPSHOP CShopManager::GetByNPCVnum(DWORD dwVnum)
{
	TShopMap::const_iterator it = m_map_pkShopByNPCVnum.find(dwVnum);

	if (it == m_map_pkShopByNPCVnum.end())
		return NULL;

	return (it->second);
}

bool CShopManager::StartShopping(LPCHARACTER pkChr, LPCHARACTER pkChrShopKeeper, int iShopVnum)
{
	if (pkChr->GetShopOwner() == pkChrShopKeeper)
		return false;
	// this method is only for NPC
	if (pkChrShopKeeper->IsPC())
		return false;

	if (!pkChr->CanAct(true, false, VAR_TIME_NONE))
		return false;

	long distance = DISTANCE_APPROX(pkChr->GetX() - pkChrShopKeeper->GetX(), pkChr->GetY() - pkChrShopKeeper->GetY());

	if (distance >= SHOP_MAX_DISTANCE)
	{
		sys_log(1, "SHOP: TOO_FAR: %s distance %d", pkChr->GetName(), distance);
		return false;
	}

	LPSHOP pkShop;

	if (iShopVnum)
		pkShop = Get(iShopVnum);
	else
		pkShop = GetByNPCVnum(pkChrShopKeeper->GetRaceNum());

	if (!pkShop)
	{
		sys_log(1, "SHOP: NO SHOP");
		return false;
	}

	bool bOtherEmpire = false;

	if (pkChr->GetEmpire() != pkChrShopKeeper->GetEmpire())
		bOtherEmpire = true;

	pkShop->AddGuest(pkChr, pkChrShopKeeper->GetVID(), bOtherEmpire);
	pkChr->SetShopOwner(pkChrShopKeeper);
	sys_log(0, "SHOP: START: %s", pkChr->GetName());
	return true;
}

LPSHOP CShopManager::FindPCShop(DWORD dwVID)
{
	TShopMap::iterator it = m_map_pkShopByPC.find(dwVID);

	if (it == m_map_pkShopByPC.end())
		return NULL;

	return it->second;
}

#ifdef __ITEM_COUNT_LIMIT__
LPSHOP CShopManager::CreatePCShop(LPCHARACTER ch, TShopItemTable* pTable, DWORD bItemCount)
#else
LPSHOP CShopManager::CreatePCShop(LPCHARACTER ch, TShopItemTable* pTable, BYTE bItemCount)
#endif
{
	if (FindPCShop(ch->GetVID()))
		return NULL;

	LPSHOP pkShop = M2_NEW CShop;
	pkShop->SetPCShop(ch);
	pkShop->SetShopItems(pTable, bItemCount);

	m_map_pkShopByPC.insert(TShopMap::value_type(ch->GetVID(), pkShop));
	return pkShop;
}

void CShopManager::DestroyPCShop(LPCHARACTER ch)
{
	LPSHOP pkShop = FindPCShop(ch->GetVID());

	if (!pkShop)
		return;

	//PREVENT_ITEM_COPY;
	ch->SetMyShopTime();
	//END_PREVENT_ITEM_COPY

	m_map_pkShopByPC.erase(ch->GetVID());
	M2_DELETE(pkShop);
}

void CShopManager::StopShopping(LPCHARACTER ch)
{
	LPSHOP shop;

	if (!(shop = ch->GetShop()))
		return;

	//PREVENT_ITEM_COPY;
	ch->SetMyShopTime();
	//END_PREVENT_ITEM_COPY

	shop->RemoveGuest(ch);
	sys_log(0, "SHOP: END: %s", ch->GetName());
}

void CShopManager::Buy(LPCHARACTER ch, BYTE pos)
{
	if (!ch->GetShop())
		return;

	if (!ch->GetShopOwner())
		return;

	if (DISTANCE_APPROX(ch->GetX() - ch->GetShopOwner()->GetX(), ch->GetY() - ch->GetShopOwner()->GetY()) > 2000)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("상점과의 거리가 너무 멀어 물건을 살 수 없습니다."));
		return;
	}

	CShop* pkShop = ch->GetShop();

	if (!pkShop)
		return;

	//PREVENT_ITEM_COPY
	ch->SetMyShopTime();
	//END_PREVENT_ITEM_COPY

	int ret = pkShop->Buy(ch, pos);

	if (SHOP_SUBHEADER_GC_OK != ret)
	{
		TPacketGCShop pack;

		pack.header = HEADER_GC_SHOP;
		pack.subheader = ret;
		pack.size = sizeof(TPacketGCShop);

		ch->GetDesc()->Packet(&pack, sizeof(pack));
	}
}

#ifdef __ADDITIONAL_INVENTORY__
#ifdef __ITEM_COUNT_LIMIT__
void CShopManager::Sell(LPCHARACTER ch, BYTE bCell, DWORD bCount, BYTE bType)
#else
void CShopManager::Sell(LPCHARACTER ch, BYTE bCell, BYTE bCount, BYTE bType)
#endif
#else
#ifdef __ITEM_COUNT_LIMIT__
void CShopManager::Sell(LPCHARACTER ch, BYTE bCell, DWORD bCount)
#else
void CShopManager::Sell(LPCHARACTER ch, BYTE bCell, BYTE bCount)
#endif
#endif
{
	if (g_bDisableSellItem)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("NPCYE_ITEM_SATAMAZSIN"));
		return;
	}

	if (!ch->GetShop())
		return;

	if (!ch->GetShopOwner())
		return;

	if (!ch->CanHandleItem())
		return;

	if (ch->GetShop()->IsPCShop())
		return;

	if (DISTANCE_APPROX(ch->GetX() - ch->GetShopOwner()->GetX(), ch->GetY() - ch->GetShopOwner()->GetY()) > 2000)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("상점과의 거리가 너무 멀어 물건을 팔 수 없습니다."));
		return;
	}

#ifdef __ADDITIONAL_INVENTORY__
	LPITEM item = ch->GetItem(TItemPos(bType, bCell));
#else
	LPITEM item = ch->GetInventoryItem(bCell);
#endif

	if (!item)
		return;

	if (item->IsEquipped() == true)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("착용 중인 아이템은 판매할 수 없습니다."));
		return;
	}

	if (true == item->isLocked())
	{
		return;
	}
#ifdef __BEGINNER_ITEM__
	if (item->IsBasicItem())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ITEM_IS_BASIC_CANNOT_DO"));
		return;
	}
#endif
	if (IS_SET(item->GetAntiFlag(), ITEM_ANTIFLAG_SELL))
		return;

#ifdef __GOLD_LIMIT_REWORK__
	long long llPrice;
#else
	DWORD dwPrice;
#endif

	if (bCount == 0 || bCount > item->GetCount())
		bCount = item->GetCount();

#ifdef __GOLD_LIMIT_REWORK__
	llPrice = item->GetShopBuyPrice();
#else
	dwPrice = item->GetShopBuyPrice();
#endif

	if (IS_SET(item->GetFlag(), ITEM_FLAG_COUNT_PER_1GOLD))
	{
#ifdef __GOLD_LIMIT_REWORK__
		if (llPrice == 0)
			llPrice = bCount;
		else
			llPrice = bCount / llPrice;
	}
	else
		llPrice *= bCount;
#else
		if (dwPrice == 0)
			dwPrice = bCount;
		else
			dwPrice = bCount / dwPrice;
}
	else
		dwPrice *= bCount;
#endif

	//	dwPrice /= 5;

	//int iVal = 3;

//	{
//		dwTax = dwPrice * iVal / 100;
//		dwPrice -= dwTax;
//	}

	if (test_server)
		sys_log(0, "Sell Item price id %d %s itemid %d", ch->GetPlayerID(), ch->GetName(), item->GetID());

#ifdef __GOLD_LIMIT_REWORK__
	const long long nTotalMoney = static_cast<long long>(ch->GetGold()) + static_cast<long long>(llPrice);

	if (GOLD_MAX <= nTotalMoney)
	{
		//sys_err("[OVERFLOW_GOLD] id %u name %s gold %lld", ch->GetPlayerID(), ch->GetName(), ch->GetGold());
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("20억냥이 초과하여 물품을 팔수 없습니다."));
		return;
	}

	sys_log(0, "SHOP: SELL: %s item name: %s(x%d):%u price: %llu", ch->GetName(), item->GetName(), bCount, item->GetID(), llPrice);
#else
	const int64_t nTotalMoney = static_cast<int64_t>(ch->GetGold()) + static_cast<int64_t>(dwPrice);

	if (GOLD_MAX <= nTotalMoney)
	{
		//sys_err("[OVERFLOW_GOLD] id %u name %s gold %u", ch->GetPlayerID(), ch->GetName(), ch->GetGold());
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("20억냥이 초과하여 물품을 팔수 없습니다."));
		return;
	}

	sys_log(0, "SHOP: SELL: %s item name: %s(x%d):%u price: %u", ch->GetName(), item->GetName(), bCount, item->GetID(), dwPrice);
#endif
	//	if (iVal > 0)

#ifdef __BATTLE_PASS_SYSTEM__
	BYTE bBattlePassId = ch->GetBattlePassId();
	if(bBattlePassId)
	{
		DWORD dwItemVnum, dwSellCount;
		if(CBattlePass::instance().BattlePassMissionGetInfo(bBattlePassId, SELL_ITEM, &dwItemVnum, &dwSellCount))
		{
			if(dwItemVnum == item->GetVnum() && ch->GetMissionProgress(SELL_ITEM, bBattlePassId) < dwSellCount)
				ch->UpdateMissionProgress(SELL_ITEM, bBattlePassId, bCount, dwSellCount);
		}
	}
#endif

	if (bCount == item->GetCount())
		ITEM_MANAGER::instance().RemoveItem(item, "SELL");
	else
		item->SetCount(item->GetCount() - bCount);

	if (ch->GetGold() < GOLD_MAX)
#ifdef __GOLD_LIMIT_REWORK__
		ch->PointChange(POINT_GOLD, llPrice, false);
#else
		ch->PointChange(POINT_GOLD, dwPrice, false);
#endif
}