#pragma once
enum
{
	SHOP_MAX_DISTANCE = 1000
};

class CGrid;

/* ---------------------------------------------------------------------------------- */
class CShop
{
public:
	typedef struct shop_item
	{
		DWORD	vnum;
#ifdef __GOLD_LIMIT_REWORK__
		long long	price;
#else
		long	price;
#endif
#ifdef __CHEQUE_SYSTEM__
		long	cheque;
#endif
#ifdef __ITEM_COUNT_LIMIT__
		DWORD	count;
#else
		BYTE	count;
#endif

		LPITEM	pkItem;
		int		itemid;
#ifdef __SHOP_PRICE_TYPE_ITEM__
		DWORD	witemVnum;
#endif

		shop_item()
		{
			vnum = 0;
			price = 0;
#ifdef __CHEQUE_SYSTEM__
			cheque = 0;
#endif
			count = 0;
			itemid = 0;
			pkItem = NULL;
#ifdef __SHOP_PRICE_TYPE_ITEM__
			witemVnum = 0;
#endif
		}
	} SHOP_ITEM;

	CShop();
	virtual ~CShop(); // @shopex de virtual burda degildi o yuzden MartySama boyle birsey yapmis.

	bool	Create(DWORD dwVnum, DWORD dwNPCVnum, TShopItemTable* pItemTable);
#ifdef __ITEM_COUNT_LIMIT__
	void	SetShopItems(TShopItemTable* pItemTable, DWORD bItemCount);
#else
	void	SetShopItems(TShopItemTable* pItemTable, BYTE bItemCount);
#endif
	virtual void	SetPCShop(LPCHARACTER ch);
	virtual bool	IsPCShop() { return m_pkPC ? true : false; }
#ifdef ENABLE_REMOTE_SHOP_SYSTEM
	virtual bool	AddGuest(LPCHARACTER ch, DWORD owner_vid, bool bOtherEmpire, bool bRemoteShop = false);
#else
	virtual bool	AddGuest(LPCHARACTER ch, DWORD owner_vid, bool bOtherEmpire);
#endif

	void	RemoveGuest(LPCHARACTER ch);
#ifdef __RELOAD_REWORK__
	void	RemoveAllGuests();
#endif
	virtual int	Buy(LPCHARACTER ch, BYTE pos);

	void	BroadcastUpdateItem(BYTE pos);

	int		GetNumberByVnum(DWORD dwVnum);

	virtual bool	IsSellingItem(DWORD itemID);

	DWORD	GetVnum() { return m_dwVnum; }
	DWORD	GetNPCVnum() { return m_dwNPCVnum; }
	LPCHARACTER	GetPC() { return m_pkPC; }

protected:
	void	Broadcast(const void* data, int bytes);
protected:
	DWORD				m_dwVnum;
	DWORD				m_dwNPCVnum;
	CGrid* m_pGrid;

	typedef std::unordered_map<LPCHARACTER, bool> GuestMapType;
	GuestMapType m_map_guest;
	std::vector<SHOP_ITEM>		m_itemVector;

	LPCHARACTER			m_pkPC;

#ifdef __ENABLE_WIKI_SYSTEM__
	friend class CWikiManager;
#endif
};