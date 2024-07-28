#pragma once
class CItemShopManager : public singleton<CItemShopManager>
{
	typedef struct SItemShopTable
	{
		DWORD	category;
		DWORD	sub_category;
		DWORD	id;
		DWORD	vnum;
		DWORD	count;
		DWORD	coins;
		DWORD	socketzero;
	} TItemShopTable;

	typedef std::map<DWORD, TItemShopTable*> TItemShopDataMap;
public:
	CItemShopManager(void);
	virtual ~CItemShopManager(void);

	const TItemShopTable* GetTable(DWORD id);

	bool LoadItemShopTable(const char* stFileName,bool reload=false);
	bool Buy(LPCHARACTER ch, DWORD id, DWORD count);

	void SendClientPacket(LPCHARACTER ch);

	void Destroy();
protected:
	TItemShopDataMap	m_ItemShopDataMap;
};