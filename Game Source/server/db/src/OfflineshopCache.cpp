#include "stdafx.h"

#ifdef __OFFLINE_SHOP__
#include "Main.h"
#include "Config.h"
#include "DBManager.h"
#include "QID.h"
#include "Peer.h"
#include "ClientManager.h"

#include "OfflineshopCache.h"

DWORD OFFSHOP_NEW_COUNTER[3] = {0, 0, 0};

namespace offlineshop
{
	//SHOPS
	std::string CreateShopCacheInsertItemQuery(DWORD dwOwner, const CShopCache::TShopCacheItemInfo& rItem);
	std::string CreateShopCacheUpdateItemQuery(DWORD dwItemID, const TPriceInfo& rItemPrice);
	std::string CreateShopCacheDeleteShopQuery(DWORD dwOwner);
	std::string CreateShopCacheDeleteShopItemQuery(DWORD dwOwner);
	std::string CreateShopCacheInsertShopQuery(DWORD dwOwnerID, DWORD dwDuration, const char* name, long x, long y, long lMapIndex, BYTE bChannel);
	std::string CreateShopCacheUpdateDurationQuery(DWORD dwOwnerID, DWORD dwDuration);
	std::string CreateShopCacheDeleteItemQuery(DWORD dwOwnerID, DWORD dwItemID);

	//SAFEBOX
	std::string CreateSafeboxCacheDeleteItemQuery(DWORD dwItem);
	std::string CreateSafeboxCacheInsertItemQuery(DWORD dwOwner, const TItemInfoEx& item);
	std::string CreateSafeboxCacheUpdateValutes(DWORD dwOwner, const TValutesInfo& val);
	std::string CreateSafeboxCacheInsertSafeboxValutesQuery(DWORD dwOwnerID);
	std::string CreateSafeboxCacheUpdateValutesByAdding(DWORD dwOwner, const TValutesInfo& val);
	std::string CreateSafeboxCacheLoadItemsQuery(DWORD dwOwnerID);
	std::string CreateSafeboxCacheLoadValutesQuery(DWORD dwOwnerID);

	/*
			CSHOPCACHE
	*/

	CShopCache::CShopCache()
	{
	}

	CShopCache::~CShopCache()
	{
	}

	bool CShopCache::Get(DWORD dwOwnerID, TShopCacheInfo** ppCache) const
	{
		CONST_CACHEITER it = m_shopsMap.find(dwOwnerID);
		if (it == m_shopsMap.end())
			return false;

		*ppCache = (TShopCacheInfo*)&(it->second);
		return true;
	}

	bool CShopCache::AddItem(DWORD dwOwnerID, const TShopCacheItemInfo& rItem)
	{
		TShopCacheInfo* pCache;
		if (!Get(dwOwnerID, &pCache))
			return false;

		SQueryInfoAddItem* qi = OFFSHOP_NEW_WITH_COUNTER(0, SQueryInfoAddItem);
		qi->dwOwnerID = dwOwnerID;
		CopyObject(qi->item, rItem);

		std::string query = CreateShopCacheInsertItemQuery(dwOwnerID, rItem);
		CDBManager::instance().ReturnQuery(query.c_str(), QID_OFFLINESHOP_ADD_ITEM, 0, qi, SQL_PLAYER_OFFLINESHOP);
		return true;
	}

	bool CShopCache::RemoveItem(DWORD dwOwnerID, DWORD dwItemID)
	{
		OFFSHOP_DEBUG("owner %u , item id %u ", dwOwnerID, dwItemID);

		TShopCacheInfo* pCache;
		if (!Get(dwOwnerID, &pCache))
			return false;

		OFFSHOP_DEBUG("found successful (shop %u)", dwOwnerID);

		std::map<DWORD, TShopCacheItemInfo>::iterator it = pCache->itemsmap.find(dwItemID);
		if (it == pCache->itemsmap.end())
			return false;

		OFFSHOP_DEBUG("found successful (item %u)", dwItemID);

		if (it->second.bLock)
			return false;

		OFFSHOP_DEBUG("is not locked (item %u)", dwItemID);

		pCache->itemsmap.erase(it);

		std::string query = CreateShopCacheDeleteItemQuery(dwOwnerID, dwItemID);
		CDBManager::instance().AsyncQuery(query.c_str(), SQL_PLAYER_OFFLINESHOP);

		if (pCache->itemsmap.empty())
			CClientManager::instance().OfflineshopExpiredShop(dwOwnerID);

		return true;
	}

	bool CShopCache::SellItem(DWORD dwOwnerID, DWORD dwItemID)
	{
		TShopCacheInfo* pCache;
		if (!Get(dwOwnerID, &pCache))
		{
			return false;
		}

		std::map<DWORD, TShopCacheItemInfo>::iterator it = pCache->itemsmap.find(dwItemID);
		if (it == pCache->itemsmap.end())
		{
			return false;
		}

		OFFSHOP_DEBUG("found item %u ", dwItemID);

		if (!it->second.bLock)
		{
			return false;
		}

		pCache->itemsmap.erase(it);

		std::string query = CreateShopCacheDeleteItemQuery(dwOwnerID, dwItemID);
		CDBManager::instance().AsyncQuery(query.c_str(), SQL_PLAYER_OFFLINESHOP);

		//offlineshop-updated 03/08/19
		if (pCache->itemsmap.empty())
			CClientManager::instance().OfflineshopExpiredShop(dwOwnerID);
		//end

		return true;
	}

	bool CShopCache::LockSellItem(DWORD dwOwnerID, DWORD dwItemID)
	{
		TShopCacheInfo* pCache;
		if (!Get(dwOwnerID, &pCache))
		{
			return false;
		}

		std::map<DWORD, TShopCacheItemInfo>::iterator it = pCache->itemsmap.find(dwItemID);
		if (it == pCache->itemsmap.end())
		{
			return false;
		}

		if (it->second.bLock)
		{
			return false;
		}

		OFFSHOP_DEBUG("locked success %u ", dwItemID);
		it->second.bLock = true;
		return true;
	}

	bool CShopCache::UnlockSellItem(DWORD dwOwnerID, DWORD dwItemID)//topatch
	{
		TShopCacheInfo* pCache;
		if (!Get(dwOwnerID, &pCache))
		{
			return false;
		}

		std::map<DWORD, TShopCacheItemInfo>::iterator it = pCache->itemsmap.find(dwItemID);
		if (it == pCache->itemsmap.end())
		{
			return false;
		}

		if (!it->second.bLock)
		{
			return false;
		}

		OFFSHOP_DEBUG("Unlocked success %u ", dwItemID);
		it->second.bLock = false;
		return true;
	}

	bool CShopCache::CloseShop(DWORD dwOwnerID)
	{
		CACHEITER it = m_shopsMap.find(dwOwnerID);
		if (it == m_shopsMap.end())
			return false;

		m_shopsMap.erase(it);

		std::string query = CreateShopCacheDeleteShopQuery(dwOwnerID);
		CDBManager::instance().AsyncQuery(query.c_str(), SQL_PLAYER_OFFLINESHOP);

		query = CreateShopCacheDeleteShopItemQuery(dwOwnerID);
		CDBManager::instance().AsyncQuery(query.c_str(), SQL_PLAYER_OFFLINESHOP);
		return true;
	}

	bool CShopCache::CreateShop(DWORD dwOwnerID, DWORD dwDuration, const char* szName, std::vector<TShopCacheItemInfo>& items, long x, long y, long lMapIndex, BYTE bChannel)
	{
		CACHEITER it = m_shopsMap.find(dwOwnerID);
		if (it != m_shopsMap.end())
			return false;

		SQueryInfoCreateShop* qi = OFFSHOP_NEW_WITH_COUNTER(1, SQueryInfoCreateShop);
		qi->dwOwnerID = dwOwnerID;
		qi->dwDuration = dwDuration;
		qi->x = x;
		qi->y = y;
		qi->lMapIndex = lMapIndex;
		qi->bChannel = bChannel;

		strlcpy(qi->szName, szName, sizeof(qi->szName));
		CopyContainer(qi->items, items);

		std::string query = CreateShopCacheInsertShopQuery(dwOwnerID, dwDuration, szName, x, y, lMapIndex, bChannel);
		CDBManager::instance().ReturnQuery(query.c_str(), QID_OFFLINESHOP_CREATE_SHOP, 0, qi, SQL_PLAYER_OFFLINESHOP);

		OFFSHOP_DEBUG("Sent query %s", query.c_str());
		return true;
	}

	bool CShopCache::CreateShopAddItem(SQueryInfoCreateShop* qi, const TShopCacheItemInfo& rItem)
	{
		CACHEITER it = m_shopsMap.find(qi->dwOwnerID);
		if (it == m_shopsMap.end())
			return false;

		std::string query = CreateShopCacheInsertItemQuery(qi->dwOwnerID, rItem);
		CDBManager::instance().ReturnQuery(query.c_str(), QID_OFFLINESHOP_CREATE_SHOP_ADD_ITEM, 0, qi, SQL_PLAYER_OFFLINESHOP);
		OFFSHOP_DEBUG("Sent query %s", query.c_str());
		return true;
	}

	bool CShopCache::PutItem(DWORD dwOwnerID, DWORD dwItemID, const TShopCacheItemInfo& rItem)
	{
		CACHEITER it = m_shopsMap.find(dwOwnerID);
		if (it == m_shopsMap.end())
			return false;

		TShopCacheInfo& rShop = it->second;
		SHOPCACHE_MAP& rMap = rShop.itemsmap;

		if (rMap.find(dwItemID) != rMap.end())
			return false;

		rMap.insert(std::make_pair(dwItemID, rItem));
		return true;
	}

	bool CShopCache::PutShop(DWORD dwOwnerID, DWORD dwDuration, const char* szName, long x, long y, long lMapIndex, BYTE bChannel)
	{
		CACHEITER it = m_shopsMap.find(dwOwnerID);
		if (it != m_shopsMap.end())
			return false;

		TShopCacheInfo sShop;
		sShop.dwDuration = dwDuration;
		strlcpy(sShop.szName, szName, sizeof(sShop.szName));
		sShop.x = x;
		sShop.y = y;
		sShop.lMapIndex = lMapIndex;
		sShop.bChannel = bChannel;

		m_shopsMap.insert(std::make_pair(dwOwnerID, sShop));
		return true;
	}

	void CShopCache::EncodeCache(CPeer* peer) const
	{
		TShopInfo shopInfo;
		itertype(m_shopsMap) it = m_shopsMap.begin();

		while (it != m_shopsMap.end())
		{
			DWORD dwOwnerID = it->first;
			const TShopCacheInfo& rShop = it->second;

			strncpy(shopInfo.szName, rShop.szName, sizeof(shopInfo.szName));
			shopInfo.dwDuration = rShop.dwDuration;
			shopInfo.dwOwnerID = dwOwnerID;
			shopInfo.dwCount = rShop.itemsmap.size();
			shopInfo.x = rShop.x;
			shopInfo.y = rShop.y;
			shopInfo.lMapIndex = rShop.lMapIndex;
			shopInfo.bChannel = rShop.bChannel;

			peer->Encode(&shopInfo, sizeof(shopInfo));

			OFFSHOP_DEBUG("encoding shop %u %s", shopInfo.dwOwnerID, shopInfo.szName);

			itertype(rShop.itemsmap) itItem = rShop.itemsmap.begin();
			TItemInfo itemInfo;

			for (; itItem != rShop.itemsmap.end(); itItem++)
			{
				DWORD dwItemID = itItem->first;
				const TShopCacheItemInfo& rItem = itItem->second;

				itemInfo.dwOwnerID = dwOwnerID;
				itemInfo.dwItemID = dwItemID;

				CopyObject(itemInfo.item, rItem.item);
				CopyObject(itemInfo.price, rItem.price);

				OFFSHOP_DEBUG("encoding for sale item %u ", itemInfo.dwItemID);
				peer->Encode(&itemInfo, sizeof(itemInfo));
			}

			it++;
		}
	}

	DWORD CShopCache::GetItemCount() const
	{
		DWORD dwItemCount = 0;
		CONST_CACHEITER it = m_shopsMap.begin();
		for (; it != m_shopsMap.end(); it++)
		{
			dwItemCount += it->second.itemsmap.size();
		}

		return dwItemCount;
	}

	//patch 08-03-2020
	void CShopCache::ShopDurationProcess()
	{
		CACHEITER it = m_shopsMap.begin();
		for (; it != m_shopsMap.end(); it++)
			if (--it->second.dwDuration != 0 && it->second.dwDuration % 30 == 0) // 30 dakikada bir yenileyecek.
				UpdateDurationQuery(it->first, it->second);

		//expired check
		std::vector<DWORD> vec;

		//item expired check
		std::vector<std::pair<DWORD, DWORD>> item_vec;
		const time_t now = time(0);

		it = m_shopsMap.begin();
		for (; it != m_shopsMap.end(); it++)
		{
			CShopCache::TShopCacheInfo& shop = it->second;

			if (shop.dwDuration == 0 || shop.dwDuration > OFFLINESHOP_DURATION_MAX_MINUTES) // DWORD negative value bug fix.
			{
				vec.push_back(it->first);
				continue;
			}

			itertype(shop.itemsmap) it_item = shop.itemsmap.begin();
			itertype(shop.itemsmap) end_item = shop.itemsmap.end();

			for (; it_item != end_item; it_item++)
			{
				TItemInfoEx& item_info = it_item->second.item;
				if (item_info.expiration == offlineshop::ExpirationType::EXPIRE_REAL_TIME)
				{
					if (now > item_info.alSockets[0])
						item_vec.push_back(std::make_pair(it->first, it_item->first));
				}
				else if (item_info.expiration == offlineshop::ExpirationType::EXPIRE_REAL_TIME_FIRST_USE)
				{
					if (item_info.alSockets[1] != 0 && item_info.alSockets[0] < now)
						item_vec.push_back(std::make_pair(it->first, it_item->first));
				}
			}
		}

		for (DWORD i = 0; i < vec.size(); i++)
			CClientManager::instance().OfflineshopExpiredShop(vec[i]);

		itertype(item_vec) item_it = item_vec.begin();
		itertype(item_vec) item_end = item_vec.end();
		for (; item_it != item_end; item_it++) {
			CClientManager::Instance().SendOfflineShopRemoveItemPacket(item_it->first, item_it->second);
			RemoveItem(item_it->first, item_it->second);
		}
	}

	void CShopCache::UpdateDurationQuery(DWORD dwOwnerID, const TShopCacheInfo& rShop)
	{
		std::string query = CreateShopCacheUpdateDurationQuery(dwOwnerID, rShop.dwDuration);
		CDBManager::instance().AsyncQuery(query.c_str(), SQL_PLAYER_OFFLINESHOP);
	}

	//SAFEBOX CHACHE
	CSafeboxCache::CSafeboxCache()
	{
	}

	CSafeboxCache::~CSafeboxCache()
	{
	}

	bool CSafeboxCache::Get(DWORD dwOwnerID, TSafeboxCacheInfo** ppSafebox) const
	{
		CHACHECONSTITER it = m_safeboxMap.find(dwOwnerID);
		if (it == m_safeboxMap.end())
			return false;

		*ppSafebox = (TSafeboxCacheInfo*)&(it->second);
		return true;
	}

	// @@
	// oyuncu oyundan ciktiktan sonraki zaman diliminde kendi deposundaki esyalar silinmiyor idi.
	// bununla alakali duzenleme yapildi.
	bool CSafeboxCache::Del(DWORD dwOwnerPID)
	{
		CHACHEITER it = m_safeboxMap.find(dwOwnerPID);
		if (it == m_safeboxMap.end())
			return false;

		TSafeboxCacheInfo* pSafebox = &it->second;
		if (!pSafebox)
			return false;

		for (auto it = pSafebox->itemsmap.begin(); it != pSafebox->itemsmap.end(); ++it)
		{
			pSafebox->itemsmap.erase(it);
		}

		m_safeboxMap.erase(it);
		return true;
	}

	bool CSafeboxCache::PutSafebox(DWORD dwOwnerID, const TSafeboxCacheInfo& rSafebox)
	{
		CHACHECONSTITER it = m_safeboxMap.find(dwOwnerID);
		if (it != m_safeboxMap.end())
			return false;

		m_safeboxMap.insert(std::make_pair(dwOwnerID, rSafebox));
		return true;
	}

	bool CSafeboxCache::PutItem(DWORD dwOwnerID, DWORD dwItem, const TItemInfoEx& item)
	{
		TSafeboxCacheInfo* pSafebox = nullptr;
		if (!Get(dwOwnerID, &pSafebox))
			return false;

		std::map<DWORD, TItemInfoEx>::iterator it = pSafebox->itemsmap.find(dwItem);
		if (it != pSafebox->itemsmap.end())
			return false;

		pSafebox->itemsmap.insert(std::make_pair(dwItem, item));
		return true;
	}

	bool CSafeboxCache::RemoveItem(DWORD dwOwnerID, DWORD dwItemID)
	{
		TSafeboxCacheInfo* pSafebox = nullptr;
		if (!Get(dwOwnerID, &pSafebox))
			return false;

		std::map<DWORD, TItemInfoEx>::iterator it = pSafebox->itemsmap.find(dwItemID);
		if (it == pSafebox->itemsmap.end())
			return false;

		pSafebox->itemsmap.erase(it);

		std::string query = CreateSafeboxCacheDeleteItemQuery(dwItemID);
		CDBManager::instance().AsyncQuery(query.c_str(), SQL_PLAYER_OFFLINESHOP);
		return true;
	}

	bool CSafeboxCache::AddItem(DWORD dwOwnerID, const TItemInfoEx& item)
	{
		//TSafeboxCacheInfo* pSafebox = nullptr;
		//offlineshop-updated 04/08/19
		/*
		if (!Get(dwOwnerID, &pSafebox))
			return false;
		*/

		SQueryInfoSafeboxAddItem* qi = OFFSHOP_NEW_WITH_COUNTER(2, SQueryInfoSafeboxAddItem);
		qi->dwOwnerID = dwOwnerID;
		CopyObject(qi->item, item);

		std::string query = CreateSafeboxCacheInsertItemQuery(dwOwnerID, item);
		CDBManager::instance().ReturnQuery(query.c_str(), QID_OFFLINESHOP_SAFEBOX_ADD_ITEM, 0, qi, SQL_PLAYER_OFFLINESHOP);
		return true;
	}

	bool CSafeboxCache::AddValutes(DWORD dwOwnerID, const TValutesInfo& val)
	{
		TSafeboxCacheInfo* pSafebox = nullptr;
		if (!Get(dwOwnerID, &pSafebox))
		{
			AddValutesAsQuery(dwOwnerID, val);
			return true;
		}

		pSafebox->valutes += val;

		std::string query = CreateSafeboxCacheUpdateValutes(dwOwnerID, pSafebox->valutes);
		CDBManager::instance().AsyncQuery(query.c_str(), SQL_PLAYER_OFFLINESHOP);
		return true;
	}

	void CSafeboxCache::AddValutesAsQuery(DWORD dwOwnerID, const TValutesInfo& val)
	{
		std::string query = CreateSafeboxCacheUpdateValutesByAdding(dwOwnerID, val);
		CDBManager::instance().AsyncQuery(query.c_str(), SQL_PLAYER_OFFLINESHOP);
	}

	bool CSafeboxCache::RemoveValutes(DWORD dwOwnerID, const TValutesInfo& val)
	{
		TSafeboxCacheInfo* pSafebox = nullptr;
		if (!Get(dwOwnerID, &pSafebox))
			return false;

		pSafebox->valutes -= val;

		std::string query = CreateSafeboxCacheUpdateValutes(dwOwnerID, pSafebox->valutes);
		CDBManager::instance().AsyncQuery(query.c_str(), SQL_PLAYER_OFFLINESHOP);
		return true;
	}

	CSafeboxCache::TSafeboxCacheInfo* CSafeboxCache::CreateSafebox(DWORD dwOwnerID)
	{
		if (!PutSafebox(dwOwnerID, TSafeboxCacheInfo()))
			return nullptr;

		std::string query = CreateSafeboxCacheInsertSafeboxValutesQuery(dwOwnerID);
		CDBManager::instance().AsyncQuery(query.c_str(), SQL_PLAYER_OFFLINESHOP);

		CSafeboxCache::TSafeboxCacheInfo* pInfo = nullptr;
		Get(dwOwnerID, &pInfo);

		return pInfo;
	}

	CSafeboxCache::TSafeboxCacheInfo* CSafeboxCache::LoadSafebox(DWORD dwPID)
	{
		TSafeboxCacheInfo* pSafebox = nullptr;
		if (Get(dwPID, &pSafebox))
			return pSafebox;

		TSafeboxCacheInfo safebox;
		MYSQL_ROW row;

		{
			std::unique_ptr<SQLMsg> pMsg(CDBManager::instance().DirectQuery(CreateSafeboxCacheLoadValutesQuery(dwPID).c_str(), SQL_PLAYER_OFFLINESHOP));
			if (pMsg->Get()->uiAffectedRows == 0)
				return CreateSafebox(dwPID);

			if (pMsg->Get()->uiAffectedRows != 1)
			{
				sys_err("multiple safebox rows for id %d ", dwPID);
				return nullptr;
			}

			if ((row = mysql_fetch_row(pMsg->Get()->pSQLResult)))
			{
				str_to_number(safebox.valutes.illYang, row[0]);
#ifdef __CHEQUE_SYSTEM__
				str_to_number(safebox.valutes.iCheque, row[1]);
#endif
			}

			else
			{
				sys_err("cannot fetch safebox row for id %d ", dwPID);
				return nullptr;
			}
		}

		{
			std::unique_ptr<SQLMsg> pMsg(CDBManager::instance().DirectQuery(CreateSafeboxCacheLoadItemsQuery(dwPID).c_str(), SQL_PLAYER_OFFLINESHOP));
			DWORD dwItemID = 0;
			TItemInfoEx item;

			while ((row = mysql_fetch_row(pMsg->Get()->pSQLResult)))
			{
				//item_id, vnum , count , sockets, attrs

				int col = 0;

				str_to_number(dwItemID, row[col++]);
				str_to_number(item.dwVnum, row[col++]);
				str_to_number(item.dwCount, row[col++]);

				for (int i = 0; i < ITEM_SOCKET_MAX_NUM; i++)
					str_to_number(item.alSockets[i], row[col++]);

				for (int i = 0; i < ITEM_ATTRIBUTE_MAX_NUM; i++)
				{
					str_to_number(item.aAttr[i].bType, row[col++]);
					str_to_number(item.aAttr[i].sValue, row[col++]);
				}

#ifdef __ITEM_CHANGELOOK__
				str_to_number(item.dwTransmutation, row[col++]);
#endif
#ifdef __ITEM_EVOLUTION__
				str_to_number(item.dwEvolution, row[col++]);
#endif
				//patch 08-03-2020
				BYTE exp = 0;
				str_to_number(exp, row[col++]);
				item.expiration = offlineshop::ExpirationType(exp);

				safebox.itemsmap.insert(std::make_pair(dwItemID, item));
			}
		}

		CACHEMAP::iterator it = m_safeboxMap.insert(std::make_pair(dwPID, safebox)).first;
		return &it->second;
	}

	DWORD CSafeboxCache::GetItemCount() const
	{
		DWORD dwItemCount = 0;
		CACHEMAP::const_iterator it = m_safeboxMap.begin();
		for (; it != m_safeboxMap.end(); it++)
			dwItemCount += it->second.itemsmap.size();

		return dwItemCount;
	}

	// QUERY MAKE
	std::string CreateShopCacheInsertItemQuery(DWORD dwOwner, const CShopCache::TShopCacheItemInfo& rItem)
	{
		//item_id, owner_id, price_yang, price_cheque, vnum, count,	socket0, socket1, socket2, attr0, attrval0,	attr1, attrval1
		//attr2, attrval2,attr3,attrval3,attr4,attrval4,attr5,attrval5,attr6,attrval6

		char szQuery[2048] = "INSERT INTO `player`.`offlineshop_shop_items` (`item_id`, `owner_id`, `price_yang`, "
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

		len += snprintf(szQuery + len, sizeof(szQuery) - len, "%s", " "
#ifdef __ITEM_CHANGELOOK__
			", `trans` "
#endif
#ifdef __ITEM_EVOLUTION__
			", `evolution` "
#endif
			//patch 08-03-2020
			", expiration "

			") VALUES (");
		len += snprintf(szQuery + len, sizeof(szQuery) - len, "0, %u, %lld,"
#ifdef __CHEQUE_SYSTEM__
			" %d, "
#endif
			" %u, %u ",
			dwOwner, rItem.price.illYang,
#ifdef __CHEQUE_SYSTEM__
			rItem.price.iCheque,
#endif
			rItem.item.dwVnum, rItem.item.dwCount

		);

		for (int i = 0; i < ITEM_SOCKET_MAX_NUM; i++)
			len += snprintf(szQuery + len, sizeof(szQuery) - len,
				",%ld ", rItem.item.alSockets[i]);

		for (int i = 0; i < ITEM_ATTRIBUTE_MAX_NUM; i++)
			len += snprintf(szQuery + len, sizeof(szQuery) - len,
				", %d , %d ", rItem.item.aAttr[i].bType, rItem.item.aAttr[i].sValue);

#ifdef __ITEM_CHANGELOOK__
		len += snprintf(szQuery + len, sizeof(szQuery) - len, " , %u ", rItem.item.dwTransmutation);
#endif
#ifdef __ITEM_EVOLUTION__
		len += snprintf(szQuery + len, sizeof(szQuery) - len, " , %u ", rItem.item.dwEvolution);
#endif
		//patch 08-03-2020
		len += snprintf(szQuery + len, sizeof(szQuery) - len, ", %u ", (BYTE)rItem.item.expiration);

		std::string query = szQuery;
		query += ");";

		return query;
	}

	std::string CreateShopCacheUpdateItemQuery(DWORD dwItemID, const TPriceInfo& rItemPrice)
	{
		static char szQuery[256];
		snprintf(szQuery, sizeof(szQuery), "UPDATE `player`.`offlineshop_shop_items` SET `price_yang` = %lld "
#ifdef __CHEQUE_SYSTEM__
			", `price_cheque` = %d "
#endif
			" WHERE `item_id` = %u;",
			rItemPrice.illYang,
#ifdef __CHEQUE_SYSTEM__
			rItemPrice.iCheque,
#endif

			dwItemID);

		return szQuery;
	}

	std::string CreateShopCacheDeleteShopQuery(DWORD dwOwner)
	{
		static char szQuery[128];
		snprintf(szQuery, sizeof(szQuery), "DELETE from `player`.`offlineshop_shops` WHERE `owner_id` = %d;", dwOwner);
		return szQuery;
	}

	std::string CreateShopCacheDeleteShopItemQuery(DWORD dwOwner)
	{
		static char szQuery[128];
		snprintf(szQuery, sizeof(szQuery), "DELETE from `player`.`offlineshop_shop_items` WHERE `owner_id` = %d;", dwOwner);
		return szQuery;
	}

	std::string CreateShopCacheInsertShopQuery(DWORD dwOwnerID, DWORD dwDuration, const char* name, long x, long y, long lMapIndex, BYTE bChannel)
	{
		static char szQuery[512];
		char szEscapeString[OFFLINE_SHOP_NAME_MAX_LEN + 32];

		CDBManager::instance().EscapeString(szEscapeString, name, strlen(name));

		snprintf(szQuery, sizeof(szQuery), "INSERT INTO `player`.`offlineshop_shops` (`owner_id`, `duration`, `name`, `pos_x`, `pos_y`, `map_index`, `channel`) VALUES(%u, %u, '%s', %ld, %ld, %ld, %d);",
			dwOwnerID, dwDuration, szEscapeString, x, y, lMapIndex, bChannel);

		return szQuery;
	}

	std::string CreateShopCacheUpdateDurationQuery(DWORD dwOwnerID, DWORD dwDuration)
	{
		static char szQuery[256];
		snprintf(szQuery, sizeof(szQuery), "UPDATE `player`.`offlineshop_shops` SET `duration` = '%u' WHERE `owner_id` = %u;", dwDuration, dwOwnerID);
		return szQuery;
	}

	std::string CreateSafeboxCacheDeleteItemQuery(DWORD dwItem)
	{
		static char szQuery[128];
		snprintf(szQuery, sizeof(szQuery), "DELETE from `player`.`offlineshop_safebox_items` WHERE `item_id` = %d;", dwItem);
		return szQuery;
	}

	std::string CreateSafeboxCacheInsertItemQuery(DWORD dwOwner, const TItemInfoEx& item)
	{
		char szQuery[2048] = "INSERT INTO `player`.`offlineshop_safebox_items` (`item_id`, `owner_id`, `vnum`, `count` ";
		size_t len = strlen(szQuery);

		for (int i = 0; i < ITEM_SOCKET_MAX_NUM; i++)
			len += snprintf(szQuery + len, sizeof(szQuery) - len,
				",`socket%d` ", i);

		for (int i = 0; i < ITEM_ATTRIBUTE_MAX_NUM; i++)
			len += snprintf(szQuery + len, sizeof(szQuery) - len,
				",`attr%d` , `attrval%d` ", i, i);

		len += snprintf(szQuery + len, sizeof(szQuery) - len, "%s", "  "
#ifdef __ITEM_CHANGELOOK__
			" , `trans` "
#endif
#ifdef __ITEM_EVOLUTION__
			" , `evolution` "
#endif
			//patch 08-03-2020
			", expiration "

			") VALUES (");
		len += snprintf(szQuery + len, sizeof(szQuery) - len, "0, %u, %u, %u ",
			dwOwner, item.dwVnum, item.dwCount
		);

		for (int i = 0; i < ITEM_SOCKET_MAX_NUM; i++)
			len += snprintf(szQuery + len, sizeof(szQuery) - len,
				",%ld ", item.alSockets[i]);

		for (int i = 0; i < ITEM_ATTRIBUTE_MAX_NUM; i++)
			len += snprintf(szQuery + len, sizeof(szQuery) - len,
				", %d , %d ", item.aAttr[i].bType, item.aAttr[i].sValue);

#ifdef __ITEM_CHANGELOOK__
		len += snprintf(szQuery + len, sizeof(szQuery) - len,
			", %u ", item.dwTransmutation);
#endif
#ifdef __ITEM_EVOLUTION__
		len += snprintf(szQuery + len, sizeof(szQuery) - len, " , %u ", item.dwEvolution);
#endif
		//patch 08-03-2020
		len += snprintf(szQuery + len, sizeof(szQuery) - len, ", %u ", (BYTE)item.expiration);

		std::string query = szQuery;
		query += ");";

		return query;
	}

	std::string CreateSafeboxCacheUpdateValutes(DWORD dwOwner, const TValutesInfo& val)
	{
		static char szQuery[256];
		snprintf(szQuery, sizeof(szQuery), "UPDATE `player`.`offlineshop_safebox_valutes` SET `yang` = %lld "
#ifdef __CHEQUE_SYSTEM__
			", `cheque` = %d "
#endif
			" WHERE `owner_id` = %u ;",
			val.illYang,
#ifdef __CHEQUE_SYSTEM__
			val.iCheque,
#endif
			dwOwner);

		return szQuery;
	}

	std::string CreateSafeboxCacheUpdateValutesByAdding(DWORD dwOwner, const TValutesInfo& val)
	{
		static char szQuery[256];
		snprintf(szQuery, sizeof(szQuery), "UPDATE `player`.`offlineshop_safebox_valutes` SET `yang` = `yang`+%lld "
#ifdef __CHEQUE_SYSTEM__
			", `cheque` = `cheque`+%d "
#endif
			" WHERE `owner_id` = %u ;", val.illYang,
#ifdef __CHEQUE_SYSTEM__
			val.iCheque,
#endif
			dwOwner);

		return szQuery;
	}

	std::string CreateSafeboxCacheInsertSafeboxValutesQuery(DWORD dwOwnerID)
	{
		static char szQuery[256];
		snprintf(szQuery, sizeof(szQuery), "INSERT INTO `player`.`offlineshop_safebox_valutes` (`owner_id`,`yang`,`cheque`) VALUES(%u, 0, 0);", dwOwnerID);
		return szQuery;
	}

	std::string CreateSafeboxCacheLoadValutesQuery(DWORD dwOwnerID)
	{
		static char szQuery[256];
		snprintf(szQuery, sizeof(szQuery), "SELECT `yang` "
#ifdef __CHEQUE_SYSTEM__
			", `cheque` "
#endif
			" from `player`.`offlineshop_safebox_valutes` WHERE `owner_id` = %d;", dwOwnerID);
		return szQuery;
	}

	std::string CreateSafeboxCacheLoadItemsQuery(DWORD dwOwnerID)
	{
		char szQuery[2048] = "SELECT `item_id`, `vnum`, `count` ";
		size_t len = strlen(szQuery);

		for (int i = 0; i < ITEM_SOCKET_MAX_NUM; i++)
			len += snprintf(szQuery + len, sizeof(szQuery) - len,
				",`socket%d` ", i);

		for (int i = 0; i < ITEM_ATTRIBUTE_MAX_NUM; i++)
			len += snprintf(szQuery + len, sizeof(szQuery) - len,
				",`attr%d` , `attrval%d` ", i, i);

		len += snprintf(szQuery + len, sizeof(szQuery) - len, "%s%u;", " "

#ifdef __ITEM_CHANGELOOK__
			" , `trans` "
#endif
#ifdef __ITEM_EVOLUTION__
			" , `evolution` "
#endif
			//patch 08-03-2020
			", expiration "

			" from `player`.`offlineshop_safebox_items` WHERE `owner_id`=", dwOwnerID);

		return szQuery;
	}

	std::string CreateShopCacheDeleteItemQuery(DWORD dwOwnerID, DWORD dwItemID)
	{
		static char szQuery[256];
		snprintf(szQuery, sizeof(szQuery), "DELETE from `player`.`offlineshop_shop_items`  WHERE `owner_id` = %u AND item_id = '%u' ;", dwOwnerID, dwItemID);
		return szQuery;
	}
}

#endif