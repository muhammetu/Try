#include "stdafx.h"
#ifdef __OFFLINE_SHOP__
#include "Main.h"
#include "Config.h"
#include "DBManager.h"
#include "QID.h"
#include "Peer.h"
#include "OfflineshopCache.h"
#include "ClientManager.h"

#define DIRECT_QUERY(query) CDBManager::instance().DirectQuery((query))

extern DWORD OFFSHOP_NEW_COUNTER[3];

std::string CreateOfflineshopSelectShopItemsQuery();

template <class T>
const char* Decode(T*& pObj, const char* data) {
	pObj = (T*)data;
	return data + sizeof(T);
}

bool CClientManager::InitializeOfflineshopTable()
{
	MYSQL_ROW row;
	OFFSHOP_DEBUG("starting to loading offlineshop %s", "table");
	//offlineshop_shops
	{
		const char szQuery[] = "SELECT `owner_id` , `name` , `duration`, `pos_x`, `pos_y`, `map_index`, `channel` FROM `player`.`offlineshop_shops`;";
		std::unique_ptr<SQLMsg> pMsg(DIRECT_QUERY(szQuery));

		if (pMsg->uiSQLErrno != 0)
		{
			sys_err("CANNOT LOAD offlineshop_shops TABLE , errorcode %d ", pMsg->uiSQLErrno);
			return false;
		}

		if (pMsg->Get())
		{
			while ((row = mysql_fetch_row(pMsg->Get()->pSQLResult)))
			{
				int col = 0;
				std::string name = "";
				DWORD dwOwner = 0, dwDuration = 0;
				long x = 0, y = 0, map_index = 0;
				BYTE channel = 0;

				str_to_number(dwOwner, row[col++]);
				name = row[col++];
				str_to_number(dwDuration, row[col++]);
				str_to_number(x, row[col++]);
				str_to_number(y, row[col++]);
				str_to_number(map_index, row[col++]);
				str_to_number(channel, row[col++]);

				OFFSHOP_DEBUG("owner %u , duration %u ", dwOwner, dwDuration);

				if (!m_offlineshopShopCache.PutShop(dwOwner, dwDuration, name.c_str(), x, y, map_index, channel))
					sys_err("cannot execute putsShop -> double shop id?! %u ", dwOwner);
			}
		}
	}

	//offlineshop_shop_items
	{
		//item_id,owner_id,price_yang,price_cheque,vnum,count,
		//socket0,socket1,socket2,attr0,attrval0,attr1,attrval1,
		//attr2,attrval2,attr3,attrval3,attr4,attrval4,attr5,attrval5,attr6,attrval6

		std::string query = CreateOfflineshopSelectShopItemsQuery();
		std::unique_ptr<SQLMsg> pMsg(DIRECT_QUERY(query.c_str()));

		if (pMsg->uiSQLErrno != 0)
		{
			sys_err("CANNOT LOAD offlineshop_shop_items TABLE , errorcode %d ", pMsg->uiSQLErrno);
			return false;
		}

		if (pMsg->Get())
		{
			while ((row = mysql_fetch_row(pMsg->Get()->pSQLResult)))
			{
				int col = 0;
				DWORD dwItemID = 0, dwOwnerID = 0;

				offlineshop::CShopCache::TShopCacheItemInfo item;
				offlineshop::ZeroObject(item);

				str_to_number(dwItemID, row[col++]);
				str_to_number(dwOwnerID, row[col++]);
				str_to_number(item.price.illYang, row[col++]);
#ifdef __CHEQUE_SYSTEM__
				str_to_number(item.price.iCheque, row[col++]);
#endif

				str_to_number(item.item.dwVnum, row[col++]);
				str_to_number(item.item.dwCount, row[col++]);

				for (int i = 0; i < ITEM_SOCKET_MAX_NUM; i++)
					str_to_number(item.item.alSockets[i], row[col++]);

				for (int i = 0; i < ITEM_ATTRIBUTE_MAX_NUM; i++)
				{
					str_to_number(item.item.aAttr[i].bType, row[col++]);
					str_to_number(item.item.aAttr[i].sValue, row[col++]);
				}

#ifdef __ITEM_CHANGELOOK__
				str_to_number(item.item.dwTransmutation, row[col++]);
#endif
#ifdef __ITEM_EVOLUTION__
				str_to_number(item.item.dwEvolution, row[col++]);
#endif
				//patch 08-03-2020
				BYTE bExpiration = 0;
				str_to_number(bExpiration, row[col++]);
				item.item.expiration = offlineshop::ExpirationType(bExpiration);

				OFFSHOP_DEBUG("owner %u ,itemid %u ", dwOwnerID, dwItemID);

				if (!m_offlineshopShopCache.PutItem(dwOwnerID, dwItemID, item))
					sys_err("cannot execute putitem !? owner %d , item %d , didn't deleted items when closed?", dwOwnerID, dwItemID);
			}
		}
	}

	return true;
}

void CClientManager::SendOfflineshopTable(CPeer* peer)
{
	peer->EncodeHeader(HEADER_DG_NEW_OFFLINESHOP, 0,
		sizeof(TPacketDGNewOfflineShop) + sizeof(offlineshop::TSubPacketDGLoadTables) +
		(sizeof(offlineshop::TShopInfo)) * m_offlineshopShopCache.GetCount() + sizeof(offlineshop::TItemInfo) * m_offlineshopShopCache.GetItemCount()
	);

	TPacketDGNewOfflineShop pack;
	pack.bSubHeader = offlineshop::SUBHEADER_DG_LOAD_TABLES;

	offlineshop::TSubPacketDGLoadTables subPack;
	subPack.dwShopCount = m_offlineshopShopCache.GetCount();

	peer->Encode(&pack, sizeof(pack));
	peer->Encode(&subPack, sizeof(subPack));

	OFFSHOP_DEBUG("shop count %u ", subPack.dwShopCount);

	m_offlineshopShopCache.EncodeCache(peer);
}

void CClientManager::RecvOfflineShopPacket(CPeer* peer, const char* data)
{
	TPacketGDNewOfflineShop* pack;
	data = Decode(pack, data);

	bool bRet = false;

	switch (pack->bSubHeader)
	{
	case offlineshop::SUBHEADER_GD_BUY_ITEM:			bRet = RecvOfflineShopBuyItemPacket(data);			break;
	case offlineshop::SUBHEADER_GD_BUY_LOCK_ITEM:		bRet = RecvOfflineShopLockBuyItem(peer, data);			break;
	case offlineshop::SUBHEADER_GD_REMOVE_ITEM:			bRet = RecvOfflineShopRemoveItemPacket(data);			break;
	case offlineshop::SUBHEADER_GD_ADD_ITEM:			bRet = RecvOfflineShopAddItemPacket(data);			break;

	case offlineshop::SUBHEADER_GD_SHOP_FORCE_CLOSE:	bRet = RecvOfflineShopForceClose(data);				break;
	case offlineshop::SUBHEADER_GD_SHOP_CREATE_NEW:		bRet = RecvOfflineShopCreateNew(data);				break;

	case offlineshop::SUBHEADER_GD_SAFEBOX_GET_ITEM:	bRet = RecvOfflineShopSafeboxGetItem(data);			break;
	case offlineshop::SUBHEADER_GD_SAFEBOX_GET_VALUTES:	bRet = RecvOfflineShopSafeboxGetValutes(data);		break;
	case offlineshop::SUBHEADER_GD_SAFEBOX_ADD_ITEM:	bRet = RecvOfflineShopSafeboxAddItem(data);			break;

	case offlineshop::SUBHEADER_GD_CANNOT_BUY_LOCK_ITEM:	bRet = RecvOfflineShopCannotBuyLockItem(data);	break;

	default:
		sys_err("UNKNOW NEW OFFLINESHOP SUBHEADER GD %d", pack->bSubHeader);
		break;
	}

	if (!bRet)
		sys_log(0, "maybe some error during recv offline shop subheader %d ", pack->bSubHeader);
}

bool CClientManager::RecvOfflineShopLockBuyItem(CPeer* peer, const char* data)
{
	offlineshop::TSubPacketGDLockBuyItem* subpack;
	data = Decode(subpack, data);

	DWORD dwGuest = subpack->dwGuestID;
	DWORD dwOwner = subpack->dwOwnerID;
	DWORD dwItem = subpack->dwItemID;
	bool isSearch = subpack->isSearch;

	OFFSHOP_DEBUG("lock request : guest %u , shop %u , item %u ", dwGuest, dwOwner, dwItem);

	if (m_offlineshopShopCache.LockSellItem(dwOwner, dwItem))
		SendOfflineShopBuyLockedItemPacket(peer, dwOwner, dwGuest, dwItem, isSearch);
	else
		sys_log(0, "cannot find buy target item %u (owner %u , buyer %u) ", dwItem, dwOwner, dwGuest);

	return true;
}

bool CClientManager::RecvOfflineShopCannotBuyLockItem(const char* data) //topatch
{
	offlineshop::TSubPacketGDCannotBuyLockItem* subpack;
	data = Decode(subpack, data);

	DWORD dwOwner = subpack->dwOwnerID;
	DWORD dwItem = subpack->dwItemID;

	OFFSHOP_DEBUG("lock unrequired : shop %u , item %u ", dwOwner, dwItem);

	if (!m_offlineshopShopCache.UnlockSellItem(dwOwner, dwItem))
	{
		sys_err("cannot find unlock requested item %u (owner %u ) ", dwItem, dwOwner);
		return false;
	}
	
	return true;
}

bool CClientManager::RecvOfflineShopBuyItemPacket(const char* data)
{
	offlineshop::TSubPacketGDBuyItem* subpack;
	data = Decode(subpack, data);

	DWORD dwGuest = subpack->dwGuestID;
	DWORD dwOwner = subpack->dwOwnerID;
	DWORD dwItem = subpack->dwItemID;
	bool isSearch = subpack->isSearch;

	OFFSHOP_DEBUG("guest %u, owner %u, item %u ", dwGuest, dwOwner, dwItem);

	offlineshop::CShopCache::TShopCacheInfo* pCache = nullptr;
	if (!m_offlineshopShopCache.Get(dwOwner, &pCache))
		return false;

	itertype(pCache->itemsmap) it = pCache->itemsmap.find(dwItem);
	if (pCache->itemsmap.end() == it)
		return false;

	const offlineshop::TPriceInfo& price = it->second.price;
	offlineshop::TValutesInfo valute;
	valute.illYang = price.illYang;
#ifdef __CHEQUE_SYSTEM__
	valute.iCheque = price.iCheque;
#endif

	m_offlineshopSafeboxCache.AddValutes(dwOwner, valute);
	SendOfflineshopSafeboxAddValutes(dwOwner, valute);

	OFFSHOP_DEBUG("added to safebox valute guest %u, owner %u, item %u, search %d ", dwGuest, dwOwner, dwItem, isSearch);

	//offlineshop-updated 04/08/19
	SendOfflineShopBuyItemPacket(dwOwner, dwGuest, dwItem, isSearch);
	if (!m_offlineshopShopCache.SellItem(dwOwner, dwItem))
		sys_err("some problem with sell : %u %u ", dwOwner, dwItem);

	return true;
}

bool CClientManager::RecvOfflineShopRemoveItemPacket(const char* data) //patchme
{
	offlineshop::TSubPacketGDRemoveItem* subpack;
	data = Decode(subpack, data);

	offlineshop::CShopCache::TShopCacheInfo* pShop = nullptr;
	if (!m_offlineshopShopCache.Get(subpack->dwOwnerID, &pShop))
	{
		OFFSHOP_DEBUG("cannot found shop owner %u into cache", subpack->dwOwnerID);
		return false;
	}

	offlineshop::CShopCache::SHOPCACHE_MAP::iterator it = pShop->itemsmap.find(subpack->dwItemID);
	if (it == pShop->itemsmap.end())
	{
		OFFSHOP_DEBUG("cannot found item id shop owner %u into cache (item id %u) ", subpack->dwOwnerID, subpack->dwItemID);
		return false;
	}

	//topatch
	auto copy = it->second.item;
	if (m_offlineshopShopCache.RemoveItem(subpack->dwOwnerID, subpack->dwItemID))
	{
		SendOfflineShopRemoveItemPacket(subpack->dwOwnerID, subpack->dwItemID);
		m_offlineshopSafeboxCache.AddItem(subpack->dwOwnerID, copy);
	}

	else
		sys_err("cannot remove item %u shop %u item ?!", subpack->dwOwnerID, subpack->dwItemID);

	return true;
}

bool CClientManager::RecvOfflineShopAddItemPacket(const char* data)
{
	offlineshop::TSubPacketGDAddItem* subpack;
	data = Decode(subpack, data);

	offlineshop::CShopCache::TShopCacheItemInfo item;
	offlineshop::ZeroObject(item);
	offlineshop::CopyObject(item.item, subpack->itemInfo.item);
	offlineshop::CopyObject(item.price, subpack->itemInfo.price);

	m_offlineshopShopCache.AddItem(subpack->dwOwnerID, item);
	return true;
}

bool CClientManager::RecvOfflineShopForceClose(const char* data)
{
	offlineshop::TSubPacketGDShopForceClose* subpack;
	data = Decode(subpack, data);

	offlineshop::CShopCache::TShopCacheInfo* pInfo = nullptr;
	if (!m_offlineshopShopCache.Get(subpack->dwOwnerID, &pInfo))
		return false;

	//store item in safebox
	offlineshop::CShopCache::ITEM_ITER it = pInfo->itemsmap.begin();
	for (; it != pInfo->itemsmap.end(); it++)
		m_offlineshopSafeboxCache.AddItem(subpack->dwOwnerID, it->second.item);

	m_offlineshopShopCache.CloseShop(subpack->dwOwnerID);

	SendOfflineShopForceClose(subpack->dwOwnerID);

	return true;
}

bool CClientManager::RecvOfflineShopCreateNew(const char* data)
{
	offlineshop::TSubPacketGDShopCreateNew* subpack;
	data = Decode(subpack, data);

	std::vector<offlineshop::CShopCache::TShopCacheItemInfo> vec;
	offlineshop::CShopCache::TShopCacheItemInfo itemInfo;
	offlineshop::ZeroObject(itemInfo);

	vec.reserve(subpack->shop.dwCount);

	for (DWORD i = 0; i < subpack->shop.dwCount; i++)
	{
		offlineshop::TItemInfo* pItemInfo;
		data = Decode(pItemInfo, data);

		offlineshop::CopyObject(itemInfo.item, pItemInfo->item);
		offlineshop::CopyObject(itemInfo.price, pItemInfo->price);

		vec.push_back(itemInfo);
	}

	OFFSHOP_DEBUG(" id %u , duration %u , name %s , count %u ", subpack->shop.dwOwnerID, subpack->shop.dwDuration, subpack->shop.szName, vec.size());
	OFFSHOP_DEBUG("RecvOfflineShopCreateNew! map_index %d (%d, %d)", subpack->pos.lMapIndex, subpack->pos.x, subpack->pos.y);

	m_offlineshopShopCache.CreateShop(subpack->shop.dwOwnerID, subpack->shop.dwDuration, subpack->shop.szName, vec, subpack->shop.x, subpack->shop.y, subpack->shop.lMapIndex, subpack->shop.bChannel);
	return true;
}

bool CClientManager::RecvOfflineShopSafeboxGetItem(const char* data)
{
	offlineshop::TSubPacketGDSafeboxGetItem* subpack;
	data = Decode(subpack, data);

	if (m_offlineshopSafeboxCache.RemoveItem(subpack->dwOwnerID, subpack->dwItemID))
	{
		SendOfflineshopSafeboxGetItem(subpack->dwOwnerID, subpack->dwItemID);
		return true;
	}

	return false;
}

bool CClientManager::RecvOfflineShopSafeboxGetValutes(const char* data)
{
	offlineshop::TSubPacketGDSafeboxGetValutes* subpack;
	data = Decode(subpack, data);

	if (m_offlineshopSafeboxCache.RemoveValutes(subpack->dwOwnerID, subpack->valute))
	{
		SendOfflineshopSafeboxGetValutes(subpack->dwOwnerID, subpack->valute);
		return true;
	}

	return false;
}

void CClientManager::SendOfflineshopSafeboxGetItem(DWORD dwOwnerID, DWORD dwItemID)
{
	TPacketDGNewOfflineShop pack;
	pack.bSubHeader = offlineshop::SUBHEADER_DG_SAFEBOX_GET_ITEM;

	offlineshop::TSubPacketDGSafeboxGetItem subpack;
	subpack.dwOwnerID = dwOwnerID;
	subpack.dwItemID = dwItemID;

	for (itertype(m_peerList) it = m_peerList.begin(); it != m_peerList.end(); it++)
	{
		CPeer* peer = *it;
		peer->EncodeHeader(HEADER_DG_NEW_OFFLINESHOP, 0, sizeof(pack) + sizeof(subpack));
		peer->Encode(&pack, sizeof(pack));
		peer->Encode(&subpack, sizeof(subpack));
	}
}

void CClientManager::SendOfflineshopSafeboxGetValutes(DWORD dwOwnerID, const offlineshop::TValutesInfo& valute)
{
	TPacketDGNewOfflineShop pack;
	pack.bSubHeader = offlineshop::SUBHEADER_DG_SAFEBOX_GET_VALUTES;

	offlineshop::TSubPacketDGSafeboxGetValutes subpack;
	subpack.dwOwnerID = dwOwnerID;
	offlineshop::CopyObject(subpack.valute, valute);

	for (itertype(m_peerList) it = m_peerList.begin(); it != m_peerList.end(); it++)
	{
		CPeer* peer = *it;
		peer->EncodeHeader(HEADER_DG_NEW_OFFLINESHOP, 0, sizeof(pack) + sizeof(subpack));
		peer->Encode(&pack, sizeof(pack));
		peer->Encode(&subpack, sizeof(subpack));
	}
}

bool CClientManager::RecvOfflineShopSafeboxAddItem(const char* data) {
	offlineshop::TSubPacketGDSafeboxAddItem* subpack;
	data = Decode(subpack, data);

	if (m_offlineshopSafeboxCache.AddItem(subpack->dwOwnerID, subpack->item))
		return false;
	return true;
}

bool CClientManager::SendOfflineShopBuyLockedItemPacket(CPeer* peer, DWORD dwOwner, DWORD dwGuest, DWORD dwItem, bool isSearch)
{
	TPacketDGNewOfflineShop pack;
	pack.bSubHeader = offlineshop::SUBHEADER_DG_LOCKED_BUY_ITEM;

	offlineshop::TSubPacketDGLockedBuyItem subpack;
	subpack.dwOwnerID = dwOwner;
	subpack.dwBuyerID = dwGuest;
	subpack.dwItemID = dwItem;
	subpack.isSearch = isSearch;

	peer->EncodeHeader(HEADER_DG_NEW_OFFLINESHOP, 0, sizeof(pack) + sizeof(subpack));
	peer->Encode(&pack, sizeof(pack));
	peer->Encode(&subpack, sizeof(subpack));

	OFFSHOP_DEBUG("sent locked item packet %u ", dwItem);
	return true;
}

bool CClientManager::SendOfflineShopBuyItemPacket(DWORD dwOwner, DWORD dwGuest, DWORD dwItem, bool isSearch)
{
	TPacketDGNewOfflineShop pack;
	pack.bSubHeader = offlineshop::SUBHEADER_DG_BUY_ITEM;

	offlineshop::TSubPacketDGBuyItem subpack;
	subpack.dwOwnerID = dwOwner;
	subpack.dwBuyerID = dwGuest;
	subpack.dwItemID = dwItem;
	subpack.isSearch = isSearch;

	for (itertype(m_peerList) it = m_peerList.begin(); it != m_peerList.end(); it++)
	{
		CPeer* peer = *it;
		peer->EncodeHeader(HEADER_DG_NEW_OFFLINESHOP, 0, sizeof(pack) + sizeof(subpack));
		peer->Encode(&pack, sizeof(pack));
		peer->Encode(&subpack, sizeof(subpack));
	}

	OFFSHOP_DEBUG("owner %u , guest %u , item %u ", dwOwner, dwGuest, dwItem);
	return true;
}

bool CClientManager::SendOfflineShopRemoveItemPacket(DWORD dwOwner, DWORD dwItem)
{
	TPacketDGNewOfflineShop pack;
	pack.bSubHeader = offlineshop::SUBHEADER_DG_REMOVE_ITEM;

	offlineshop::TSubPacketDGRemoveItem subpack;
	subpack.dwOwnerID = dwOwner;
	subpack.dwItemID = dwItem;

	for (itertype(m_peerList) it = m_peerList.begin(); it != m_peerList.end(); it++)
	{
		CPeer* peer = *it;
		peer->EncodeHeader(HEADER_DG_NEW_OFFLINESHOP, 0, sizeof(pack) + sizeof(subpack));
		peer->Encode(&pack, sizeof(pack));
		peer->Encode(&subpack, sizeof(subpack));
	}

	return true;
}

bool CClientManager::SendOfflineShopAddItemPacket(DWORD dwOwner, DWORD dwItemID, const offlineshop::TItemInfo& rInfo)
{
	TPacketDGNewOfflineShop pack;
	pack.bSubHeader = offlineshop::SUBHEADER_DG_ADD_ITEM;

	offlineshop::TSubPacketDGAddItem subpack;
	subpack.dwOwnerID = dwOwner;
	subpack.dwItemID = dwItemID;
	offlineshop::CopyObject(subpack.item, rInfo);

	for (itertype(m_peerList) it = m_peerList.begin(); it != m_peerList.end(); it++)
	{
		CPeer* peer = *it;
		peer->EncodeHeader(HEADER_DG_NEW_OFFLINESHOP, 0, sizeof(pack) + sizeof(subpack));
		peer->Encode(&pack, sizeof(pack));
		peer->Encode(&subpack, sizeof(subpack));
	}

	return true;
}

bool CClientManager::SendOfflineShopForceClose(DWORD dwOwnerID)
{
	TPacketDGNewOfflineShop pack;
	pack.bSubHeader = offlineshop::SUBHEADER_DG_SHOP_FORCE_CLOSE;

	offlineshop::TSubPacketDGShopForceClose subpack;
	subpack.dwOwnerID = dwOwnerID;

	for (itertype(m_peerList) it = m_peerList.begin(); it != m_peerList.end(); it++)
	{
		CPeer* peer = *it;
		peer->EncodeHeader(HEADER_DG_NEW_OFFLINESHOP, 0, sizeof(pack) + sizeof(subpack));
		peer->Encode(&pack, sizeof(pack));
		peer->Encode(&subpack, sizeof(subpack));
	}

	return true;
}

bool CClientManager::SendOfflineShopCreateNew(const offlineshop::TShopInfo& shop, const std::vector<offlineshop::TItemInfo>& vec)
{
	TPacketDGNewOfflineShop pack;
	pack.bSubHeader = offlineshop::SUBHEADER_DG_SHOP_CREATE_NEW;

	offlineshop::TSubPacketDGShopCreateNew subpack;
	offlineshop::CopyObject(subpack.shop, shop);

	for (itertype(m_peerList) it = m_peerList.begin(); it != m_peerList.end(); it++)
	{
		CPeer* peer = *it;
		peer->EncodeHeader(HEADER_DG_NEW_OFFLINESHOP, 0, sizeof(pack) + sizeof(subpack) + (sizeof(offlineshop::TItemInfo) * vec.size()));
		peer->Encode(&pack, sizeof(pack));
		peer->Encode(&subpack, sizeof(subpack));

		if (!vec.empty())
			peer->Encode(&vec[0], sizeof(offlineshop::TItemInfo) * vec.size());
	}

	OFFSHOP_DEBUG("Sent offlineshop create packet (forward) for shop %s , shop id %u ", shop.szName, shop.dwOwnerID);
#ifdef __OFFLINESHOP_DEBUG__
	for (DWORD i = 0; i < vec.size(); i++)
		OFFSHOP_DEBUG("item %u , vnum %u , count %u ", vec[i].dwItemID, vec[i].item.dwVnum, vec[i].item.dwCount);
#endif
	return true;
}

bool CClientManager::SendOfflineshopShopExpired(DWORD dwOwnerID)
{
	TPacketDGNewOfflineShop pack;
	pack.bSubHeader = offlineshop::SUBHEADER_DG_SHOP_EXPIRED;

	offlineshop::TSubPacketDGShopExpired subpack;
	subpack.dwOwnerID = dwOwnerID;

	for (itertype(m_peerList) it = m_peerList.begin(); it != m_peerList.end(); it++)
	{
		CPeer* peer = *it;
		peer->EncodeHeader(HEADER_DG_NEW_OFFLINESHOP, 0, sizeof(pack) + sizeof(subpack));
		peer->Encode(&pack, sizeof(pack));
		peer->Encode(&subpack, sizeof(subpack));
	}

	return true;
}

void CClientManager::SendOfflineshopSafeboxAddItem(DWORD dwOwnerID, DWORD dwItem, const offlineshop::TItemInfoEx& item)
{
	TPacketDGNewOfflineShop pack;
	pack.bSubHeader = offlineshop::SUBHEADER_DG_SAFEBOX_ADD_ITEM;

	offlineshop::TSubPacketDGSafeboxAddItem subpack;
	subpack.dwOwnerID = dwOwnerID;
	subpack.dwItemID = dwItem;

	offlineshop::CopyObject(subpack.item, item);

	for (itertype(m_peerList) it = m_peerList.begin(); it != m_peerList.end(); it++)
	{
		CPeer* peer = *it;
		peer->EncodeHeader(HEADER_DG_NEW_OFFLINESHOP, 0, sizeof(pack) + sizeof(subpack));
		peer->Encode(&pack, sizeof(pack));
		peer->Encode(&subpack, sizeof(subpack));
	}
}

void CClientManager::SendOfflineshopSafeboxAddValutes(DWORD dwOwnerID, const offlineshop::TValutesInfo& valute)
{
	TPacketDGNewOfflineShop pack;
	pack.bSubHeader = offlineshop::SUBHEADER_DG_SAFEBOX_ADD_VALUTES;

	offlineshop::TSubPacketDGSafeboxAddValutes subpack;
	subpack.dwOwnerID = dwOwnerID;
	offlineshop::CopyObject(subpack.valute, valute);

	for (itertype(m_peerList) it = m_peerList.begin(); it != m_peerList.end(); it++)
	{
		CPeer* peer = *it;
		peer->EncodeHeader(HEADER_DG_NEW_OFFLINESHOP, 0, sizeof(pack) + sizeof(subpack));
		peer->Encode(&pack, sizeof(pack));
		peer->Encode(&subpack, sizeof(subpack));
	}
}

void CClientManager::SendOfflineshopSafeboxLoad(CPeer* peer, DWORD dwOwnerID, const offlineshop::TValutesInfo& valute, const std::vector<offlineshop::TItemInfoEx>& items, const std::vector<DWORD>& ids)
{
	TPacketDGNewOfflineShop pack;
	pack.bSubHeader = offlineshop::SUBHEADER_DG_SAFEBOX_LOAD;

	offlineshop::TSubPacketDGSafeboxLoad subpack;
	subpack.dwOwnerID = dwOwnerID;
	subpack.dwItemCount = items.size();
	offlineshop::CopyObject(subpack.valute, valute);

	OFFSHOP_DEBUG("player %u , count %u , valutes yang %lld ", dwOwnerID, items.size(), valute.illYang);

	peer->EncodeHeader(HEADER_DG_NEW_OFFLINESHOP, 0, sizeof(pack) + sizeof(subpack) + (sizeof(offlineshop::TItemInfoEx) * items.size()) + (sizeof(DWORD) * ids.size()));
	peer->Encode(&pack, sizeof(pack));
	peer->Encode(&subpack, sizeof(subpack));

	for (DWORD i = 0; i < items.size(); i++)
	{
		peer->EncodeDWORD(ids[i]);
		peer->Encode(&items[i], sizeof(offlineshop::TItemInfoEx));
	}
}

//patch 08-03-2020
void CClientManager::SendOfflineshopSafeboxExpiredItem(DWORD dwOwnerID, DWORD itemID) {
	TPacketDGNewOfflineShop pack;
	pack.bSubHeader = offlineshop::SUBHEADER_DG_SAFEBOX_EXPIRED_ITEM;

	offlineshop::TSubPacketDGSafeboxExpiredItem subpack;
	subpack.dwOwnerID = dwOwnerID;
	subpack.dwItemID = itemID;

	for (itertype(m_peerList) it = m_peerList.begin(); it != m_peerList.end(); it++)
	{
		CPeer* peer = *it;
		if (peer->GetChannel() == 0)
			continue;
		peer->EncodeHeader(HEADER_DG_NEW_OFFLINESHOP, 0, sizeof(pack) + sizeof(subpack));
		peer->Encode(&pack, sizeof(pack));
		peer->Encode(&subpack, sizeof(subpack));
	}
}

void CClientManager::OfflineShopResultAddItemQuery(CPeer* peer, SQLMsg* msg, CQueryInfo* pQueryInfo)
{
	offlineshop::SQueryInfoAddItem* qi = (offlineshop::SQueryInfoAddItem*)pQueryInfo->pvData;

	DWORD dwItemID = msg->Get()->uiInsertID;

	offlineshop::TItemInfo info;
	offlineshop::CopyObject(info.item, qi->item.item);
	offlineshop::CopyObject(info.price, qi->item.price);

	info.dwItemID = dwItemID;
	info.dwOwnerID = qi->dwOwnerID;

	m_offlineshopShopCache.PutItem(qi->dwOwnerID, dwItemID, qi->item);
	SendOfflineShopAddItemPacket(qi->dwOwnerID, dwItemID, info);

	OFFSHOP_DELETE_WITH_COUNTER(0, qi);
}

void CClientManager::OfflineShopResultCreateShopQuery(CPeer* peer, SQLMsg* msg, CQueryInfo* pQueryInfo)
{
	offlineshop::SQueryInfoCreateShop* qi = (offlineshop::SQueryInfoCreateShop*)pQueryInfo->pvData;
	if (!m_offlineshopShopCache.PutShop(qi->dwOwnerID, qi->dwDuration, qi->szName, qi->x, qi->y, qi->lMapIndex, qi->bChannel))
		sys_err("cannot insert new shop , id %d ", qi->dwOwnerID);

	if (!qi->items.empty())
	{
		OFFSHOP_DEBUG("shop %s , Item insert %d", qi->szName, 0);

		qi->dwItemIndex = 0;
		offlineshop::CShopCache::TShopCacheItemInfo& itemInfo = qi->items[qi->dwItemIndex];
		if (!m_offlineshopShopCache.CreateShopAddItem(qi, itemInfo))
			sys_err("some problem during shop puts? cannot find the shop in the cache");
	}
	else
	{
		offlineshop::TShopInfo shop;
		shop.dwOwnerID = qi->dwOwnerID;
		shop.dwCount = 0;
		strncpy(shop.szName, qi->szName, sizeof(shop.szName));
		shop.x = qi->x;
		shop.y = qi->y;
		shop.lMapIndex = qi->lMapIndex;
		shop.bChannel = qi->bChannel;

		std::vector<offlineshop::TItemInfo> vec;
		SendOfflineShopCreateNew(shop, vec);

		OFFSHOP_DELETE_WITH_COUNTER(1, qi);
	}
}

void CClientManager::OfflineShopResultCreateShopAddItemQuery(CPeer* peer, SQLMsg* msg, CQueryInfo* pQueryInfo)
{
	offlineshop::SQueryInfoCreateShop* qi = (offlineshop::SQueryInfoCreateShop*)pQueryInfo->pvData;

	const offlineshop::CShopCache::TShopCacheItemInfo& item = qi->items[qi->dwItemIndex++];
	m_offlineshopShopCache.PutItem(qi->dwOwnerID, msg->Get()->uiInsertID, item);
	qi->ids.push_back(msg->Get()->uiInsertID);

	OFFSHOP_DEBUG("shop %s ", qi->szName);

	if (!qi->items.empty() && qi->dwItemIndex != qi->items.size())
	{
		offlineshop::CShopCache::TShopCacheItemInfo& itemInfo = qi->items[qi->dwItemIndex];
		m_offlineshopShopCache.CreateShopAddItem(qi, itemInfo);

		OFFSHOP_DEBUG("shop %s , Item insert %d", qi->szName, qi->dwItemIndex);
	}
	else
	{
		offlineshop::TShopInfo shopInfo;
		shopInfo.dwOwnerID = qi->dwOwnerID;
		shopInfo.dwDuration = qi->dwDuration;
		shopInfo.dwCount = qi->items.size();
		strncpy(shopInfo.szName, qi->szName, sizeof(shopInfo.szName));
		shopInfo.x = qi->x;
		shopInfo.y = qi->y;
		shopInfo.lMapIndex = qi->lMapIndex;
		shopInfo.bChannel = qi->bChannel;

		offlineshop::TItemInfo temp;
		std::vector<offlineshop::TItemInfo> vec;
		vec.reserve(qi->items.size());

		OFFSHOP_DEBUG("shop making a new vector TItemInfo , reserved %d ", qi->items.size());

		for (DWORD i = 0; i < qi->items.size(); i++)
		{
			offlineshop::CShopCache::TShopCacheItemInfo& itemInfo = qi->items[i];

			temp.dwItemID = qi->ids[i];
			temp.dwOwnerID = qi->dwOwnerID;
			offlineshop::CopyObject(temp.item, itemInfo.item);
			offlineshop::CopyObject(temp.price, itemInfo.price);

			vec.push_back(temp);
		}

		SendOfflineShopCreateNew(shopInfo, vec);
		OFFSHOP_DEBUG("shop %s sending forward packet (items count %d) ", shopInfo.szName, shopInfo.dwCount);
		OFFSHOP_DELETE_WITH_COUNTER(1, qi);
	}
}

void CClientManager::OfflineShopResultSafeboxAddItemQuery(CPeer* peer, SQLMsg* msg, CQueryInfo* pQueryInfo)
{
	offlineshop::SQueryInfoSafeboxAddItem* qi = (offlineshop::SQueryInfoSafeboxAddItem*)pQueryInfo->pvData;

	m_offlineshopSafeboxCache.PutItem(qi->dwOwnerID, msg->Get()->uiInsertID, qi->item);
	SendOfflineshopSafeboxAddItem(qi->dwOwnerID, msg->Get()->uiInsertID, qi->item);

	OFFSHOP_DELETE_WITH_COUNTER(2, qi);
}

void CClientManager::OfflineShopResultQuery(CPeer* peer, SQLMsg* msg, CQueryInfo* pQueryInfo)
{
	OFFSHOP_DEBUG("query type %d ", pQueryInfo->iType);

	switch (pQueryInfo->iType)
	{
	case QID_OFFLINESHOP_ADD_ITEM:
		OfflineShopResultAddItemQuery(peer, msg, pQueryInfo);
		break;

	case QID_OFFLINESHOP_CREATE_SHOP:
		OfflineShopResultCreateShopQuery(peer, msg, pQueryInfo);
		break;

	case QID_OFFLINESHOP_CREATE_SHOP_ADD_ITEM:
		OfflineShopResultCreateShopAddItemQuery(peer, msg, pQueryInfo);
		break;

	case QID_OFFLINESHOP_SAFEBOX_ADD_ITEM:
		OfflineShopResultSafeboxAddItemQuery(peer, msg, pQueryInfo);
		break;

	default:
		sys_err("Unknown offshop query type %d ", pQueryInfo->iType);
		break;
	}
}

void CClientManager::OfflineshopDurationProcess()
{
	m_offlineshopShopCache.ShopDurationProcess();
	//patch 08-03-2020
	//m_offlineshopSafeboxCache.ItemExpirationProcess();
}

void CClientManager::OfflineshopExpiredShop(DWORD dwID)
{
	offlineshop::CShopCache::TShopCacheInfo* pInfo = nullptr;
	if (!m_offlineshopShopCache.Get(dwID, &pInfo))
		return;

	//store item in safebox
	offlineshop::CShopCache::ITEM_ITER it = pInfo->itemsmap.begin();
	for (; it != pInfo->itemsmap.end(); it++)
		m_offlineshopSafeboxCache.AddItem(dwID, it->second.item);

	//close shop
	m_offlineshopShopCache.CloseShop(dwID);

	SendOfflineshopShopExpired(dwID);
}

void CClientManager::OfflineshopLoadShopSafebox(CPeer* peer, DWORD dwID)
{
	offlineshop::CSafeboxCache::TSafeboxCacheInfo* pSafebox = nullptr;

	if (!m_offlineshopSafeboxCache.Get(dwID, &pSafebox))
		pSafebox = m_offlineshopSafeboxCache.LoadSafebox(dwID);

	if (!pSafebox)
	{
		sys_err("cannot load shop safebox for pid %d ", dwID);
		return;
	}

	std::vector<offlineshop::TItemInfoEx> items;
	std::vector<DWORD> ids;

	items.reserve(pSafebox->itemsmap.size());
	ids.reserve(pSafebox->itemsmap.size());

	for (itertype(pSafebox->itemsmap) it = pSafebox->itemsmap.begin();
		it != pSafebox->itemsmap.end();
		it++)
	{
		ids.push_back(it->first);
		items.push_back(it->second);
	}

	SendOfflineshopSafeboxLoad(peer, dwID, pSafebox->valutes, items, ids);
}

// @@ memory consumption
void CClientManager::OfflineshopDelShopSafebox(uint32_t dwID)
{
	if (!m_offlineshopSafeboxCache.Del(dwID))
	{
		sys_log(0, "Offline Shop Safebox Del Failed PID(%d)", dwID);
	}
	else
	{
		sys_log(0, "Offline Shop Safebox Del Success PID(%d)", dwID);
	}
}

//updated 15-01-2020
bool CClientManager::IsUsingOfflineshopSystem(DWORD dwID)
{
	offlineshop::CShopCache::TShopCacheInfo* pInfo = nullptr;
	if (m_offlineshopShopCache.Get(dwID, &pInfo))
		return true;

	else {
		static char query[256];
		snprintf(query, sizeof(query), "SELECT owner_id FROM player.offlineshop_shops WHERE owner_id ='%u';",
			dwID);

		std::unique_ptr<SQLMsg> pMsg(CDBManager::instance().DirectQuery(query, SQL_PLAYER_OFFLINESHOP));
		return pMsg->Get()->uiAffectedRows != 0;
	}
}

std::string CreateOfflineshopSelectShopItemsQuery()
{
	char szQuery[2048] = "SELECT `item_id`, `owner_id`, `price_yang`, "
#ifdef __CHEQUE_SYSTEM__
		"`price_cheque`,"
#endif
		" `vnum`, `count` ";
	size_t len = strlen(szQuery);

	for (int i = 0; i < ITEM_SOCKET_MAX_NUM; i++)
		len += snprintf(szQuery + len, sizeof(szQuery) - len,
			",`socket%d` ", i);

	for (int i = 0; i < ITEM_ATTRIBUTE_MAX_NUM; i++)
		len += snprintf(szQuery + len, sizeof(szQuery) - len,
			",`attr%d` , `attrval%d` ", i, i);

	len += snprintf(szQuery + len, sizeof(szQuery) - len, "%s",
#ifdef __ITEM_CHANGELOOK__
		", trans "
#endif
#ifdef __ITEM_EVOLUTION__
		", `evolution` "
#endif
		//patch 08-03-2020
		", expiration "

		" FROM  `player`.`offlineshop_shop_items`;");
	return szQuery;
}

#endif