#pragma once
#ifdef __OFFLINE_SHOP__
#define SUBTYPE_NOSET 255
#define OFFLINESHOP_DURATION_UPDATE_TIME PASSES_PER_SEC(60)

namespace offlineshop
{
	class CShopManager : public singleton<CShopManager>
	{
	public:

#ifdef __OFFLINE_SHOP_ENTITY_CITIES__
		typedef std::map<DWORD, ShopEntity*> SHOPENTITIES_MAP;

		typedef struct SCityShopInfo {
			SHOPENTITIES_MAP	entitiesByPID;
			SHOPENTITIES_MAP	entitiesByVID;

			void Clear()
			{
				entitiesByPID.clear();
				entitiesByVID.clear();
			}

			SCityShopInfo()
			{
				Clear();
			}

			SCityShopInfo(const SCityShopInfo& rCopy)
			{
				CopyContainer(entitiesByPID, rCopy.entitiesByPID);
				CopyContainer(entitiesByVID, rCopy.entitiesByVID);
			}
		} TCityShopInfo;
#endif

		typedef std::map<DWORD, CShop>					 SHOPMAP;
		typedef std::map<DWORD, CShopSafebox>			 SAFEBOXMAP;

#ifdef __OFFLINE_SHOP_ENTITY_CITIES__
		typedef std::vector<TCityShopInfo>				 CITIESVEC;
#endif

		CShopManager();
		~CShopManager();

		offlineshop::CShop* PutsNewShop(TShopInfo* pInfo);

		offlineshop::CShop* GetShopByOwnerID(DWORD dwPID);
		CShopSafebox* GetShopSafeboxByOwnerID(DWORD dwPID);

		void					RemoveSafeboxFromCache(DWORD dwOwnerID);
		void					RemoveGuestFromShops(LPCHARACTER ch);

#ifdef __OFFLINE_SHOP_ENTITY_CITIES__
	public:
		void		CreateNewShopEntities(offlineshop::CShop& rShop);
		void		DestroyNewShopEntities(const offlineshop::CShop& rShop);

		void		EncodeInsertShopEntity(ShopEntity& shop, LPCHARACTER ch);
		void		EncodeRemoveShopEntity(ShopEntity& shop, LPCHARACTER ch);

	private:
		void		__UpdateEntity(const offlineshop::CShop& rShop);
#endif

	public:
		//packets exchanging db
		//ITEMS
		/*db*/	void		SendShopBuyDBPacket(DWORD dwBuyerID, DWORD dwOwnerID, DWORD dwItemID, bool isSearch);
		/*db*/	void		SendShopLockBuyItemDBPacket(DWORD dwBuyerID, DWORD dwOwnerID, DWORD dwItemID, bool isSearch);

		/*db*/	bool		RecvShopLockedBuyItemDBPacket(DWORD dwBuyerID, DWORD dwOwnerID, DWORD dwItemID, bool isSearch);
		/*db*/	bool		RecvShopBuyDBPacket(DWORD dwBuyerID, DWORD dwOwnerID, DWORD dwItemID, bool isSearch);
		/*db*/	void		SendShopCannotBuyLockedItemDBPacket(DWORD dwOwnerID, DWORD dwItemID); //topatch

		/*db*/	void		SendShopRemoveItemDBPacket(DWORD dwOwnerID, DWORD dwItemID);
		/*db*/	bool		RecvShopRemoveItemDBPacket(DWORD dwOwnerID, DWORD dwItemID);

		/*db*/	void		SendShopAddItemDBPacket(DWORD dwOwnerID, const TItemInfo& rItemInfo);
		/*db*/	bool		RecvShopAddItemDBPacket(DWORD dwOwnerID, const TItemInfo& rItemInfo);

		//SHOPS
		/*db*/	void		SendShopForceCloseDBPacket(DWORD dwPID);
		/*db*/	bool		RecvShopForceCloseDBPacket(DWORD dwPID);
		/*db*/	bool		RecvShopExpiredDBPacket(DWORD dwPID);

		/*db*/	void		SendShopCreateNewDBPacket(const TShopInfo&, std::vector<TItemInfo>& vec);
		/*db*/	bool		RecvShopCreateNewDBPacket(const TShopInfo&, std::vector<TItemInfo>& vec);
		/*cli.*/void		SendShopCloseBuilderBoard(LPCHARACTER ch);

		/*db*/	void		SendShopSafeboxGetItemDBPacket(DWORD dwOwnerID, DWORD dwItemID);
		/*db*/	void		SendShopSafeboxGetValutesDBPacket(DWORD dwOwnerID, const TValutesInfo& valutes);
		/*db*/  bool		SendShopSafeboxAddItemDBPacket(DWORD dwOwnerID, const CShopItem& item);
		/*db*/	bool		RecvShopSafeboxGetItemDBPacket(DWORD dwOwnerID, DWORD dwItemID);
		/*db*/	bool		RecvShopSafeboxGetValutesDBPacket(DWORD dwOwnerID, const TValutesInfo& valute);
		/*db*/	bool		RecvShopSafeboxAddItemDBPacket(DWORD dwOwnerID, DWORD dwItemID, const TItemInfoEx& item);
		/*db*/	bool		RecvShopSafeboxAddValutesDBPacket(DWORD dwOwnerID, const TValutesInfo& valute);
		/*db*/	bool		RecvShopSafeboxLoadDBPacket(DWORD dwOwnerID, const TValutesInfo& valute, const std::vector<DWORD>& ids, const std::vector<TItemInfoEx>& items);
		//patch 08-03-2020
		/*db*/  bool		RecvShopSafeboxExpiredItemDBPacket(DWORD dwOwnerID, DWORD dwItemID);

		//packets echanging clients
		//SHOPS
		/*cli.*/bool		RecvShopCreateNewClientPacket(LPCHARACTER ch, TShopInfo& rShopInfo, std::vector<TShopItemInfo>& vec);
		/*cli.*/bool		RecvShopForceCloseClientPacket(LPCHARACTER ch);
		/*cli.*/bool		RecvShopOpenClientPacket(LPCHARACTER ch, DWORD dwOwnerID);
		/*cli.*/bool		RecvShopOpenMyShopClientPacket(LPCHARACTER ch);
		/*cli.*/bool		RecvShopBuyItemClientPacket(LPCHARACTER ch, DWORD dwOwnerID, DWORD dwItemID, bool isSearch, const offlineshop::TPriceInfo& price); //fix-edit-price
#ifdef __OFFLINE_SHOP_ENTITY_CITIES__
		/*cli.*/bool		RecvShopClickEntity(LPCHARACTER ch, DWORD dwShopEntityVID);
#endif

		/*cli.*/void		SendShopOpenClientPacket(LPCHARACTER ch, CShop* pkShop);
		/*cli.*/void		SendShopOpenMyShopClientPacket(LPCHARACTER ch);
		/*cli.*/void		SendShopOpenMyShopNoShopClientPacket(LPCHARACTER ch);
		/*cli.*/void		SendShopBuyItemFromSearchClientPacket(LPCHARACTER ch, DWORD dwOwnerID, DWORD dwItemID);

		/*cli.*/void		SendShopForceClosedClientPacket(DWORD dwOwnerID);

		//ITEMS
		/*cli.*/bool		RecvShopAddItemClientPacket(LPCHARACTER ch, const TItemPos& item, const TPriceInfo& price);
		/*cli.*/bool		RecvShopRemoveItemClientPacket(LPCHARACTER ch, DWORD dwItemID);

		//FILTER
		/*cli.*/bool		RecvShopFilterRequestClientPacket(LPCHARACTER ch, const TFilterInfo& filter);
		/*cli.*/void		SendShopFilterResultClientPacket(LPCHARACTER ch, const std::vector<TItemInfo>& items);

		//SAFEBOX
		/*cli.*/bool		RecvShopSafeboxOpenClientPacket(LPCHARACTER ch);
		/*cli.*/bool		RecvShopSafeboxGetItemClientPacket(LPCHARACTER ch, DWORD dwItemID);
		/*cli.*/bool		RecvShopSafeboxGetValutesClientPacket(LPCHARACTER ch, const TValutesInfo& valutes);
		/*cli.*/bool		RecvShopSafeboxCloseClientPacket(LPCHARACTER ch);

		/*cli.*/void		SendShopSafeboxRefresh(LPCHARACTER ch, const TValutesInfo& valute, const std::vector<CShopItem>& vec);

		/*cli.*/void		RecvCloseBoardClientPacket(LPCHARACTER ch);
		/*cli.*/void		RecvTeleportClientPacket(LPCHARACTER ch);
		/*cli.*/void		SendPopupPacket(LPCHARACTER ch, BYTE windowNum, BYTE errorNum);

		//other
		void		UpdateShopsDuration();

		//search time map (checking to avoid search abouse)
		void		Destroy();
		int			GetMapIndexAllowsList(int iMapIndex);
		/* sys.*/SHOPMAP	GetShopMap() { return m_mapShops; }
		DWORD		GetShopMapSize() {return m_mapShops.size();}

	private:
		SHOPMAP			m_mapShops;
		SAFEBOXMAP		m_mapSafeboxs;

		LPEVENT			m_eventShopDuration;
#ifdef __OFFLINE_SHOP_ENTITY_CITIES__
		CITIESVEC		m_vecCities;
#endif
		std::set<int> s_set_offlineshop_map_allows;
	};

	offlineshop::CShopManager& GetManager();
}

#endif//__OFFLINE_SHOP__