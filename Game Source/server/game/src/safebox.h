#pragma once
class CHARACTER;
class CItem;
class CGrid;

class CSafebox
{
public:
#ifdef __GOLD_LIMIT_REWORK__
	CSafebox(LPCHARACTER pkChrOwner, int iSize, long long llGold);
#else
	CSafebox(LPCHARACTER pkChrOwner, int iSize, DWORD dwGold);
#endif
	~CSafebox();

	bool		Add(DWORD dwPos, LPITEM pkItem);
	LPITEM		Get(DWORD dwPos);
	LPITEM		Remove(DWORD dwPos);
	void		ChangeSize(int iSize);

#ifdef __ITEM_COUNT_LIMIT__
	bool		MoveItem(BYTE bCell, BYTE bDestCell, DWORD count);
#else
	bool		MoveItem(BYTE bCell, BYTE bDestCell, BYTE count);
#endif
	LPITEM		GetItem(BYTE bCell);

	void		Save();

	bool		IsEmpty(DWORD dwPos, BYTE bSize);
	bool		IsValidPosition(DWORD dwPos);

	void		SetWindowMode(BYTE bWindowMode);

protected:
	void		__Destroy();

	LPCHARACTER	m_pkChrOwner;
	LPITEM		m_pkItems[SAFEBOX_MAX_NUM];
	CGrid* m_pkGrid;
	int		m_iSize;
#ifdef __GOLD_LIMIT_REWORK__
	long long		m_lGold;
#else
	long		m_lGold;
#endif
	BYTE		m_bWindowMode;
};