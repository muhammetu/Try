#pragma once
#ifdef __OFFLINE_SHOP__
#define __USE_PID_AS_GUESTLIST__

#ifdef __USE_PID_AS_GUESTLIST__
#define AS_LPGUEST(some)  (CHARACTER_MANAGER::instance().FindByPID(some))
#define AS_GUESTID(some)  (some->GetPlayerID())
#else
#define AS_LPGUEST(some) (some)
#define AS_GUESTID(some) (some)
#endif

template <class T>
void ZeroObject(T& obj)
{
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

//copyarray
template <class T, size_t size>
void CopyArray(T(&objDest)[size], const T(&objSrc)[size]) {
	if (size == 0)
		return;
	memcpy(&objDest[0], &objSrc[0], sizeof(T) * size);
}

template <class T>
void DeletePointersContainer(T& obj) {
	typename T::iterator it = obj.begin();
	for (; it != obj.end(); it++)
		delete(*it);
}

namespace offlineshop
{
	//patch 08-03-2020
	inline offlineshop::ExpirationType GetItemExpiration(LPITEM item) {
		auto proto = item->GetProto();
		for (const auto limit : proto->aLimits) {
			if (limit.bType == LIMIT_REAL_TIME)
				return offlineshop::ExpirationType::EXPIRE_REAL_TIME;
			else if (limit.bType == LIMIT_REAL_TIME_START_FIRST_USE && item->GetSocket(1) != 0)
				return offlineshop::ExpirationType::EXPIRE_REAL_TIME_FIRST_USE;
		} return offlineshop::ExpirationType::EXPIRE_NONE;
	}

#pragma pack(1)
	class CShopItem
	{
	public:

		CShopItem(DWORD dwID);
		CShopItem(const CShopItem& rCopy);
		CShopItem(LPITEM pItem, const TPriceInfo& sPrice, BYTE byWindowType, DWORD dwID = 0);

		~CShopItem();

		bool			GetTable(TItemTable** ppTable) const;
		TPriceInfo* GetPrice() const;

		LPITEM			CreateItem() const;
		TItemInfoEx* GetInfo() const;

		void			SetInfo(LPITEM pItem);
		void			SetInfo(const TItemInfoEx& info);
		void			SetPrice(const TPriceInfo& sPrice);

		void			SetWindow(BYTE byWin);
		BYTE			GetWindow() const;

		DWORD			GetID() const;
		void			SetOwnerID(DWORD dwOwnerID);
		bool			CanBuy(LPCHARACTER ch);

		void operator = (const CShopItem& rItem);

	protected:
		TItemInfoEx		m_itemInfo;
		TPriceInfo		m_priceInfo;
		BYTE			m_byWindow;
		DWORD			m_dwID;
		DWORD			m_dwOwnerID;
	};
#pragma pack()

	class CShop
	{
	public:
		typedef std::vector<CShopItem>  VECSHOPITEM;
#ifdef __USE_PID_AS_GUESTLIST__
		typedef std::list<DWORD>		LISTGUEST;
#else
		typedef std::list<LPCHARACTER>	LISTGUEST;
#endif

	public:
		CShop();
		CShop(const CShop& rCopy);

		~CShop();

		//get const
		VECSHOPITEM* GetItems() const;
		LISTGUEST* GetGuests() const;

		//duration
		void				SetDuration(DWORD dwDuration);
		DWORD				DecreaseDuration();
		DWORD				GetDuration() const;

		//owner pid
		void				SetOwnerPID(DWORD dwOwnerPID);
		DWORD				GetOwnerPID() const;

		//guests
		bool				AddGuest(LPCHARACTER ch);
		bool				RemoveGuest(LPCHARACTER ch);

		//items
		void				SetItems(VECSHOPITEM* pVec);
		bool				AddItem(CShopItem& rItem);
		bool				RemoveItem(DWORD dwItemID);
		bool				ModifyItem(DWORD dwItemID, CShopItem& rItem);
		bool				BuyItem(DWORD dwItem);
		bool				GetItem(DWORD dwItem, CShopItem** ppItem);

		LPCHARACTER			FindOwnerCharacter();
		void				Clear();

		const char* GetName() const;
		std::string GetOwnerName() const;
		void				SetName(const char* pcszName);

		void				RefreshToOwner();

		void				SetPosInfo(long lMapIndex, long x, long y, BYTE bChannel);
		TShopPosition* 		GetPosInfo() const { return (TShopPosition*)&m_posInfo; };
		long				GetMapIndex() { return GetPosInfo()->lMapIndex; };
		long				GetMapX() { return GetPosInfo()->x; };
		long				GetMapY() { return GetPosInfo()->y; };
		BYTE				GetChannel() { return GetPosInfo()->bChannel; };
	private:
		void				__RefreshItems(LPCHARACTER ch = NULL);

	private:
		VECSHOPITEM			m_vecItems;
		LISTGUEST			m_listGuests;

		DWORD				m_dwPID;
		DWORD				m_dwDuration;
		std::string			m_stName;
		TShopPosition		m_posInfo;
	};

	class CShopSafebox
	{
	public:
		typedef std::vector<CShopItem> VECITEM;

	public:
#pragma pack(1)
		struct SValuteAmount {
			long long illYang;
#ifdef __CHEQUE_SYSTEM__
			int iCheque;

#endif

			SValuteAmount() : illYang(0)
#ifdef __CHEQUE_SYSTEM__
				, iCheque(0)
#endif
			{
			}

			SValuteAmount(const TValutesInfo& rCopy)
			{
				illYang = rCopy.illYang;
#ifdef __CHEQUE_SYSTEM__
				iCheque = rCopy.iCheque;
#endif
			}

			SValuteAmount(const TPriceInfo& rCopy)
			{
				illYang = rCopy.illYang;
#ifdef __CHEQUE_SYSTEM__
				iCheque = rCopy.iCheque;
#endif
			}

			long long GetTotalYangAmount() const {
				long long total = illYang;
#ifdef __CHEQUE_SYSTEM__
				total += YANG_PER_CHEQUE * iCheque;
#endif
				return total;
			}

			bool operator < (const SValuteAmount& rVal)
			{
				return GetTotalYangAmount() < rVal.GetTotalYangAmount();
			}

			void operator -= (const SValuteAmount& rVal)
			{
				illYang -= rVal.illYang;
#ifdef __CHEQUE_SYSTEM__
				iCheque -= rVal.iCheque;
#endif
			}

			void operator += (const SValuteAmount& rVal)
			{
				illYang += rVal.illYang;
#ifdef __CHEQUE_SYSTEM__
				iCheque += rVal.iCheque;
#endif
			}
		};
#pragma pack()

	public:
		CShopSafebox(LPCHARACTER chOwner);
		CShopSafebox();
		CShopSafebox(const CShopSafebox& rCopy);
		~CShopSafebox();

		void			SetOwner(LPCHARACTER ch);
		void			SetItems(VECITEM* pVec);
		void			SetValuteAmount(SValuteAmount val);

		bool			AddItem(CShopItem* pItem);
		bool			RemoveItem(DWORD dwItemID);

		void			AddValute(SValuteAmount val);
		bool			RemoveValute(SValuteAmount val);

		VECITEM* GetItems();
		SValuteAmount	GetValutes();

		bool			GetItem(DWORD dwItemID, CShopItem** ppItem);
		LPCHARACTER		GetOwner();

		bool			RefreshToOwner(LPCHARACTER ch = nullptr);

	private:
		VECITEM			m_vecItems;
		LPCHARACTER		m_pkOwner;
		SValuteAmount	m_valutes;
	};

#ifdef __OFFLINE_SHOP_ENTITY_CITIES__
	class ShopEntity : public CEntity
	{
	public:
		ShopEntity();

		//overriding virtual CEntity methods
		void	EncodeInsertPacket(LPENTITY entity);
		void	EncodeRemovePacket(LPENTITY entity);

		const char* GetShopName() const;
		void				SetShopName(const char* name);

		int					GetShopType();
		void				SetShopType(int iType);
		void				SetShop(offlineshop::CShop* pOfflineShop);

		offlineshop::CShop* GetShop() { return m_pkShop; }

		static DWORD		AllocID() {
			static DWORD dwID = 1;
			return dwID++;
		}

		DWORD				GetVID() { return m_dwVID; }
		void				Destroy() { CEntity::Destroy(); }
	private:
		char					m_szName[OFFLINE_SHOP_NAME_MAX_LEN];
		int						m_iType;
		DWORD					m_dwVID;
		offlineshop::CShop* m_pkShop;
	};

#endif
}

#endif //__OFFLINE_SHOP__