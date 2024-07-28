#pragma once

#include "Packet.h"

class CPythonExchange : public CSingleton<CPythonExchange>
{
public:
	enum
	{
#ifdef ENABLE_EXCHANGE_REWORK
		EXCHANGE_ITEM_MAX_NUM = 45,
#else
		EXCHANGE_ITEM_MAX_NUM = 12,
#endif
	};

	typedef struct trade
	{
		char					name[CHARACTER_NAME_MAX_LEN + 1];
#ifdef ENABLE_EXCHANGE_REWORK
		DWORD					level;
#endif
		DWORD					item_vnum[EXCHANGE_ITEM_MAX_NUM];
#ifdef ENABLE_ITEM_COUNT_LIMIT
		DWORD					item_count[EXCHANGE_ITEM_MAX_NUM];
#else
		BYTE					item_count[EXCHANGE_ITEM_MAX_NUM];
#endif
		DWORD					item_metin[EXCHANGE_ITEM_MAX_NUM][ITEM_SOCKET_SLOT_MAX_NUM];
#ifdef ENABLE_ITEM_EVOLUTION_SYSTEM
		DWORD					item_evolution[EXCHANGE_ITEM_MAX_NUM];
#endif
#ifdef ENABLE_CHANGELOOK_SYSTEM
		DWORD					transmutation[EXCHANGE_ITEM_MAX_NUM];
#endif
		TPlayerItemAttribute	item_attr[EXCHANGE_ITEM_MAX_NUM][ITEM_ATTRIBUTE_SLOT_MAX_NUM];

		BYTE					accept;
#ifdef ENABLE_GOLD_LIMIT_REWORK
		long long				elk;
#else
		DWORD					elk;
#endif
#ifdef ENABLE_CHEQUE_SYSTEM
		DWORD					cheque;
#endif
	} TExchangeData;

public:
	CPythonExchange();
	virtual ~CPythonExchange();

	void			Clear();

	void			Start();
	void			End();
	bool			isTrading();

	// Interface

	void			SetSelfName(const char* name);
	void			SetTargetName(const char* name);

	char* GetNameFromSelf();
	char* GetNameFromTarget();

#ifdef ENABLE_EXCHANGE_REWORK
	void			SetSelfLevel(DWORD level);
	void			SetTargetLevel(DWORD level);

	DWORD			GetLevelFromSelf();
	DWORD			GetLevelFromTarget();
#endif

#ifdef ENABLE_GOLD_LIMIT_REWORK
	void			SetElkToTarget(long long elk);
	void			SetElkToSelf(long long elk);

	long long		GetElkFromTarget();
	long long		GetElkFromSelf();
#else
	void			SetElkToTarget(DWORD elk);
	void			SetElkToSelf(DWORD elk);

	DWORD			GetElkFromTarget();
	DWORD			GetElkFromSelf();
#endif

#ifdef ENABLE_CHEQUE_SYSTEM
	void			SetChequeToTarget(DWORD cheque);
	void			SetChequeToSelf(DWORD cheque);

	DWORD			GetChequeFromTarget();
	DWORD			GetChequeFromSelf();
#endif

#ifdef ENABLE_ITEM_COUNT_LIMIT
	void			SetItemToTarget(DWORD pos, DWORD vnum, DWORD count);
	void			SetItemToSelf(DWORD pos, DWORD vnum, DWORD count);
#else
	void			SetItemToTarget(DWORD pos, DWORD vnum, BYTE count);
	void			SetItemToSelf(DWORD pos, DWORD vnum, BYTE count);
#endif

	void			SetItemMetinSocketToTarget(int pos, int imetinpos, DWORD vnum);
	void			SetItemMetinSocketToSelf(int pos, int imetinpos, DWORD vnum);

	void			SetItemAttributeToTarget(int pos, int iattrpos, BYTE byType, short sValue);
	void			SetItemAttributeToSelf(int pos, int iattrpos, BYTE byType, short sValue);
#ifdef ENABLE_ITEM_EVOLUTION_SYSTEM
	void			SetItemEvolutionToTarget(int pos, DWORD evolution);
	void			SetItemEvolutionToSelf(int pos, DWORD evolution);

	DWORD			GetItemEvolutionFromTarget(int pos);
	DWORD			GetItemEvolutionFromSelf(int pos);
#endif

#ifdef ENABLE_CHANGELOOK_SYSTEM
	void			SetItemTransmutation(int iPos, DWORD dwTransmutation, bool bSelf);
	DWORD			GetItemTransmutation(int iPos, bool bSelf);
#endif

	void			DelItemOfTarget(BYTE pos);
	void			DelItemOfSelf(BYTE pos);

	DWORD			GetItemVnumFromTarget(BYTE pos);
	DWORD			GetItemVnumFromSelf(BYTE pos);

#ifdef ENABLE_ITEM_COUNT_LIMIT
	DWORD			GetItemCountFromTarget(BYTE pos);
	DWORD			GetItemCountFromSelf(BYTE pos);
#else
	BYTE			GetItemCountFromTarget(BYTE pos);
	BYTE			GetItemCountFromSelf(BYTE pos);
#endif

	DWORD			GetItemMetinSocketFromTarget(BYTE pos, int iMetinSocketPos);
	DWORD			GetItemMetinSocketFromSelf(BYTE pos, int iMetinSocketPos);

	void			GetItemAttributeFromTarget(BYTE pos, int iAttrPos, BYTE* pbyType, short* psValue);
	void			GetItemAttributeFromSelf(BYTE pos, int iAttrPos, BYTE* pbyType, short* psValue);

	void			SetAcceptToTarget(BYTE Accept);
	void			SetAcceptToSelf(BYTE Accept);

	bool			GetAcceptFromTarget();
	bool			GetAcceptFromSelf();

	bool			GetElkMode();
	void			SetElkMode(bool value);

protected:
	bool				m_isTrading;

	bool				m_elk_mode;
	TExchangeData		m_self;
	TExchangeData		m_victim;
};
