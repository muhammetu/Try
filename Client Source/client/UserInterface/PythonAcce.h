#pragma once
#include "Locale_inc.h"
#ifdef ENABLE_ACCE_SYSTEM
#include "Packet.h"

class CPythonAcce : public CSingleton<CPythonAcce>
{
public:
#ifdef ENABLE_GOLD_LIMIT_REWORK
	long long	llPrice;
#else
	DWORD	dwPrice;
#endif
	typedef std::vector<TAcceMaterial> TAcceMaterials;

public:
	CPythonAcce();
	virtual ~CPythonAcce();

	void	Clear();
#ifdef ENABLE_GOLD_LIMIT_REWORK
	void	AddMaterial(long long llRefPrice, BYTE bPos, TItemPos tPos);
#else
	void	AddMaterial(DWORD dwRefPrice, BYTE bPos, TItemPos tPos);
#endif
	void	AddResult(DWORD dwItemVnum, DWORD dwMinAbs, DWORD dwMaxAbs);
#ifdef ENABLE_GOLD_LIMIT_REWORK
	void	RemoveMaterial(long long llRefPrice, BYTE bPos);
	long long	GetPrice() { return llPrice; }
#else
	void	RemoveMaterial(DWORD dwRefPrice, BYTE bPos);
	DWORD	GetPrice() { return dwPrice; }
#endif
	bool	GetAttachedItem(BYTE bPos, BYTE& bHere, WORD& wCell);
	void	GetResultItem(DWORD& dwItemVnum, DWORD& dwMinAbs, DWORD& dwMaxAbs);

protected:
	TAcceResult	m_vAcceResult;
	TAcceMaterials	m_vAcceMaterials;
};
#endif
