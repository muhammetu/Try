#pragma once
#ifdef __OFFLINE_SHOP__
namespace offlineshop
{
	template <class T>
	void ZeroObject(T& obj) {
		obj = {};
	}

	template <class T>
	void CopyObject(T& objDest, const T& objSrc) {
		memcpy(&objDest, &objSrc, sizeof(objDest));
	}

	template <class T>
	void CopyContainer(T& objDest, const T& objSrc) {
		objDest = objSrc;
	}

	template <class T>
	void DeletePointersContainer(T& obj) {
		typename T::iterator it = obj.begin();
		for (; it != obj.end(); it++)
			delete(*it);
	}

	//typedefs
	struct SQueryInfoAddItem;
	struct SQueryInfoCreateShop;

	class CShopCache
	{
	public:
		typedef struct SShopCacheItemInfo {
			TPriceInfo		price;
			TItemInfoEx		item;
			bool			bLock;

			SShopCacheItemInfo()
			{
				ZeroObject(price);
				ZeroObject(item);

				bLock = false;
			}
		} TShopCacheItemInfo;

		typedef std::map<DWORD, TShopCacheItemInfo> SHOPCACHE_MAP;
		typedef struct {
			DWORD	dwDuration;
			char	szName[OFFLINE_SHOP_NAME_MAX_LEN];

			SHOPCACHE_MAP 	itemsmap;
			long x;
			long y;
			long lMapIndex;
			BYTE bChannel;
		} TShopCacheInfo;

		typedef std::map<DWORD, TShopCacheItemInfo>::iterator ITEM_ITER;

	public:
		CShopCache();
		~CShopCache();

		bool		Get(DWORD dwOwnerID, TShopCacheInfo** ppCache) const;

		bool		AddItem(DWORD dwOwnerID, const TShopCacheItemInfo& rItem);
		bool		RemoveItem(DWORD dwOwnerID, DWORD dwItemID);
		bool		SellItem(DWORD dwOwnerID, DWORD dwItemID);
		bool		LockSellItem(DWORD dwOwnerID, DWORD dwItemID);
		bool		UnlockSellItem(DWORD dwOwnerID, DWORD dwItemID);//topatch

		bool		CloseShop(DWORD dwOwnerID);
		bool		CreateShop(DWORD dwOwnerID, DWORD dwDuration, const char* szName, std::vector<TShopCacheItemInfo>& items, long x, long y, long lMapIndex, BYTE bChannel);
		bool		CreateShopAddItem(SQueryInfoCreateShop* qi, const TShopCacheItemInfo& rItem);

		bool		PutItem(DWORD dwOwnerID, DWORD dwItemID, const TShopCacheItemInfo& rItem);
		bool		PutShop(DWORD dwOwnerID, DWORD dwDuration, const char* szName, long x, long y, long lMapIndex, BYTE bChannel);

		DWORD		GetCount() const { return m_shopsMap.size(); }
		void		EncodeCache(CPeer* peer) const;
		DWORD		GetItemCount() const;

		void		ShopDurationProcess();
		void		UpdateDurationQuery(DWORD dwOwnerID, const TShopCacheInfo& rShop);

	private:
		typedef std::map<DWORD, TShopCacheInfo>::iterator CACHEITER;
		typedef std::map<DWORD, TShopCacheInfo>::const_iterator CONST_CACHEITER;
		std::map<DWORD, TShopCacheInfo> m_shopsMap;
	};

	class CSafeboxCache
	{
	public:
		typedef struct
		{
			TValutesInfo	valutes;
			std::map<DWORD, TItemInfoEx> itemsmap;
		} TSafeboxCacheInfo;

		typedef std::map<DWORD, TSafeboxCacheInfo> CACHEMAP;
		typedef std::map<DWORD, TSafeboxCacheInfo>::iterator CHACHEITER;
		typedef std::map<DWORD, TSafeboxCacheInfo>::const_iterator CHACHECONSTITER;

	public:
		CSafeboxCache();
		~CSafeboxCache();

		bool				Get(DWORD dwOwnerID, TSafeboxCacheInfo** ppSafebox) const;
		// @@ memory consumption
		bool				Del(DWORD dwOwnerID);

		bool				PutSafebox(DWORD dwOwnerID, const TSafeboxCacheInfo& rSafebox);
		bool				PutItem(DWORD dwOwnerID, DWORD dwItem, const TItemInfoEx& item);

		bool				RemoveItem(DWORD dwOwner, DWORD dwItemID);
		bool				AddItem(DWORD dwOwnerID, const TItemInfoEx& item);

		bool				AddValutes(DWORD dwOwnerID, const TValutesInfo& val);
		bool				RemoveValutes(DWORD dwOwnerID, const TValutesInfo& val);
		void				AddValutesAsQuery(DWORD dwOwnerID, const TValutesInfo& val);

		TSafeboxCacheInfo* CreateSafebox(DWORD dwOwnerID);
		DWORD				GetCount() const { return m_safeboxMap.size(); }
		DWORD				GetItemCount() const;

		TSafeboxCacheInfo* LoadSafebox(DWORD dwPID);

		//patch 08-03-2020
		//void				ItemExpirationProcess();

	private:
		CACHEMAP	m_safeboxMap;
	};

	//QUERY INFO STRUCT
	//shopcache
	struct SQueryInfoAddItem
	{
		DWORD dwOwnerID;
		CShopCache::TShopCacheItemInfo item;
	};

	struct SQueryInfoCreateShop
	{
		DWORD dwOwnerID;
		DWORD dwDuration;
		char  szName[OFFLINE_SHOP_NAME_MAX_LEN];
		std::vector<CShopCache::TShopCacheItemInfo> items;
		std::vector<DWORD> ids;
		DWORD dwItemIndex;
		long x;
		long y;
		long lMapIndex;
		BYTE bChannel;
	};

	//safeboxcache
	struct SQueryInfoSafeboxAddItem
	{
		DWORD		dwOwnerID;
		TItemInfoEx item;
	};
}

// SQueryInfoAddItem
// SQueryInfoCreateShop
// SQueryInfoSafeboxAddItem

#define OFFSHOP_NEW_WITH_COUNTER(ind, datatype) new(datatype); OFFSHOP_NEW_COUNTER[ind] += 1;

#define OFFSHOP_DELETE_WITH_COUNTER(ind, data) OFFSHOP_NEW_COUNTER[ind] -= 1; delete(data);

#endif //__OFFLINE_SHOP__
