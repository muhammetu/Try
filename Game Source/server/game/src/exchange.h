#pragma once
class CGrid;

enum EExchangeValues
{
	EXCHANGE_ITEM_MAX_NUM = 45,
	EXCHANGE_MAX_DISTANCE = 1000
};

class CExchange
{
public:
	CExchange(LPCHARACTER pOwner);
	~CExchange();
	bool		Accept(bool bIsAccept = true);
	void		Cancel();

#ifdef __GOLD_LIMIT_REWORK__
	bool		AddGold(long long lGold);
#else
	bool		AddGold(long lGold);
#endif
#ifdef __CHEQUE_SYSTEM__
	bool		AddCheque(int bCheque);
#endif

	bool		AddItem(TItemPos item_pos, BYTE display_pos);
	bool		RemoveItem(BYTE pos);

	LPCHARACTER	GetOwner() { return m_pOwner; }
	CExchange* GetCompany() { return m_pCompany; }

	bool		GetAcceptStatus() { return m_bAccept; }
	CItem *		GetItemByPosition(int i) const { return m_apItems[i]; }
	void		SetCompany(CExchange* pExchange) { m_pCompany = pExchange; }

private:
	bool		Done(DWORD tradeID, bool firstPlayer);
	bool		Check(int* piItemCount);
	bool		CheckSpace();

private:
	CExchange* m_pCompany;

	LPCHARACTER	m_pOwner;

	TItemPos		m_aItemPos[EXCHANGE_ITEM_MAX_NUM];
	LPITEM		m_apItems[EXCHANGE_ITEM_MAX_NUM];
	BYTE		m_abItemDisplayPos[EXCHANGE_ITEM_MAX_NUM];

	bool 		m_bAccept;
#ifdef __GOLD_LIMIT_REWORK__
	long long	m_lGold;
#else
	long		m_lGold;
#endif
#ifdef __CHEQUE_SYSTEM__
	int			m_bCheque;
#endif
	CGrid* m_pGrid;
};