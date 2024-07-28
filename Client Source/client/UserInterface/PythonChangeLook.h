#pragma once

#ifdef ENABLE_CHANGELOOK_SYSTEM
#include "Packet.h"

class CPythonChangeLook : public CSingleton<CPythonChangeLook>
{
public:
#ifdef ENABLE_GOLD_LIMIT_REWORK
	long long llCost;
#else
	DWORD	dwCost;
#endif
	typedef std::vector<TChangeLookMaterial> TChangeLookMaterials;

public:
	CPythonChangeLook();
	virtual ~CPythonChangeLook();

	void	Clear();
	void	AddMaterial(BYTE bPos, TItemPos tPos);
	void	RemoveMaterial(BYTE bPos);
	void	RemoveAllMaterials();
#ifdef ENABLE_GOLD_LIMIT_REWORK
	long long	GetCost() { return llCost; }
	void	SetCost(long long llCostR) { llCost = llCostR; }
#else
	DWORD	GetCost() { return dwCost; }
	void	SetCost(DWORD dwCostR) { dwCost = dwCostR; }
#endif
	bool	GetAttachedItem(BYTE bPos, BYTE& bHere, WORD& wCell);

protected:
	TChangeLookMaterials	m_vMaterials;
};
#endif
