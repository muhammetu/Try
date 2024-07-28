#pragma once
class CShop;
typedef class CShop* LPSHOP;

class CShopManager : public singleton<CShopManager>
{
public:
	typedef std::map<DWORD, CShop*> TShopMap;

public:
	CShopManager();
	virtual ~CShopManager();

	bool	Initialize(TShopTable* table, int size);
	void	Destroy();

	LPSHOP	Get(DWORD dwVnum);
	LPSHOP	GetByNPCVnum(DWORD dwVnum);

	bool	StartShopping(LPCHARACTER pkChr, LPCHARACTER pkShopKeeper, int iShopVnum = 0);
	void	StopShopping(LPCHARACTER ch);

	void	Buy(LPCHARACTER ch, BYTE pos);
#ifdef __ADDITIONAL_INVENTORY__
#ifdef __ITEM_COUNT_LIMIT__
	void	Sell(LPCHARACTER ch, BYTE bCell, DWORD bCount = 0, BYTE bType = 0);
#else
	void	Sell(LPCHARACTER ch, BYTE bCell, BYTE bCount = 0, BYTE bType = 0);
#endif
#else
#ifdef __ITEM_COUNT_LIMIT__
	void	Sell(LPCHARACTER ch, BYTE bCell, BYTE bCount = 0);
#else
	void	Sell(LPCHARACTER ch, BYTE bCell, BYTE bCount = 0);
#endif
#endif

#ifdef __ITEM_COUNT_LIMIT__
	LPSHOP	CreatePCShop(LPCHARACTER ch, TShopItemTable* pTable, DWORD bItemCount);
#else
	LPSHOP	CreatePCShop(LPCHARACTER ch, TShopItemTable* pTable, BYTE bItemCount);
#endif
	LPSHOP	FindPCShop(DWORD dwVID);
	void	DestroyPCShop(LPCHARACTER ch);

private:
	TShopMap	m_map_pkShop;
	TShopMap	m_map_pkShopByNPCVnum;
	TShopMap	m_map_pkShopByPC;

#ifdef __ENABLE_WIKI_SYSTEM__
	friend class CWikiManager;
#endif

	bool	ReadShopTableEx(const char* stFileName);
};