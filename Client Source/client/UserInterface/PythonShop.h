#pragma once

#include "Packet.h"

typedef enum SShopCoinType
{
	SHOP_COIN_TYPE_GOLD, // DEFAULT VALUE
	SHOP_COIN_TYPE_SECONDARY_COIN,
#ifdef ENABLE_CHEQUE_SYSTEM
	SHOP_COIN_TYPE_CHEQUE,
#endif
} EShopCoinType;

class CPythonShop : public CSingleton<CPythonShop>
{
public:
	CPythonShop(void);
	virtual ~CPythonShop(void);

	void Clear();

	void SetItemData(DWORD dwIndex, const TShopItemData& c_rShopItemData);
	BOOL GetItemData(DWORD dwIndex, const TShopItemData** c_ppItemData);

	void SetItemData(BYTE tabIdx, DWORD dwSlotPos, const TShopItemData& c_rShopItemData);
	BOOL GetItemData(BYTE tabIdx, DWORD dwSlotPos, const TShopItemData** c_ppItemData);

	void SetTabCount(BYTE bTabCount) { m_bTabCount = bTabCount; }
	BYTE GetTabCount() { return m_bTabCount; }

	void SetTabCoinType(BYTE tabIdx, BYTE coinType);
	BYTE GetTabCoinType(BYTE tabIdx);

	void SetTabName(BYTE tabIdx, const char* name);
	const char* GetTabName(BYTE tabIdx);

	//BOOL GetSlotItemID(DWORD dwSlotPos, DWORD* pdwItemID);

	void Open(BOOL isPrivateShop, BOOL isMainPrivateShop);

	void Close();
	BOOL IsOpen();
	BOOL IsPrivateShop();
	BOOL IsMainPlayerPrivateShop();

	void ClearPrivateShopStock();
#ifdef ENABLE_GOLD_LIMIT_REWORK
#ifdef ENABLE_CHEQUE_SYSTEM
	void AddPrivateShopItemStock(TItemPos ItemPos, BYTE byDisplayPos, long long dwPrice, DWORD dwCheque);
#else
	void AddPrivateShopItemStock(TItemPos ItemPos, BYTE byDisplayPos, long long dwPrice);
#endif
#else
#ifdef ENABLE_CHEQUE_SYSTEM
	void AddPrivateShopItemStock(TItemPos ItemPos, BYTE byDisplayPos, DWORD dwPrice, DWORD dwCheque);
#else
	void AddPrivateShopItemStock(TItemPos ItemPos, BYTE byDisplayPos, DWORD dwPrice);
#endif
#endif
	void DelPrivateShopItemStock(TItemPos ItemPos);
	bool GetPrivateShopItemStock(TItemPos ItemPos);
#ifdef ENABLE_GOLD_LIMIT_REWORK
	long long GetPrivateShopItemPrice(TItemPos ItemPos);
#else
	int GetPrivateShopItemPrice(TItemPos ItemPos);
#endif
#ifdef ENABLE_CHEQUE_SYSTEM
	int GetPrivateShopItemCheque(TItemPos ItemPos);
#endif
	void BuildPrivateShop(const char* c_szName);

protected:
	BOOL				m_isShoping;
	BOOL				m_isPrivateShop;
	BOOL				m_isMainPlayerPrivateShop;

	struct ShopTab
	{
		ShopTab()
		{
			coinType = SHOP_COIN_TYPE_GOLD;
		}
		BYTE				coinType;
		std::string			name;
		TShopItemData		items[SHOP_HOST_ITEM_MAX_NUM];
	};

	BYTE m_bTabCount;
	ShopTab m_aShoptabs[3];

	typedef std::map<TItemPos, TShopItemTable> TPrivateShopItemStock;
	TPrivateShopItemStock	m_PrivateShopItemStock;
};
