#pragma once

#include "ItemData.h"
#ifdef ENABLE_DS_SET
#include "DragonSoulTable.h"
#endif
#ifdef ENABLE_WIKI_SYSTEM
#include "../EterPythonLib/StdAfx.h"
#endif

class CItemManager : public CSingleton<CItemManager>
{
public:
	enum EItemDescCol
	{
		ITEMDESC_COL_VNUM,
		ITEMDESC_COL_NAME,
		ITEMDESC_COL_DESC,
		ITEMDESC_COL_SUMM,
		ITEMDESC_COL_NUM,
	};

#ifdef ENABLE_ACCE_SYSTEM
	enum EItemScaleCol
	{
		ITEMSCALE_COL_VNUM,
		ITEMSCALE_COL_JOB,
		ITEMSCALE_COL_SEX,
		ITEMSCALE_COL_SCALE_X,
		ITEMSCALE_COL_SCALE_Y,
		ITEMSCALE_COL_SCALE_Z,
		ITEMSCALE_COL_PARTICLE_SCALE,
	};
#endif

#ifdef ENABLE_AURA_COSTUME_SYSTEM
	enum EAuraScaleCol
	{
		AURA_SCALE_COL_VNUM,
		AURA_SCALE_COL_JOB,
		AURA_SCALE_COL_SEX,
		AURA_SCALE_COL_MESH_SCALE_X,
		AURA_SCALE_COL_MESH_SCALE_Y,
		AURA_SCALE_COL_MESH_SCALE_Z,
		AURA_SCALE_COL_PARTICLE_SCALE,
		AURA_SCALE_COL_NUM,
	};

	bool	LoadAuraScale(const char* c_szFileName);
#endif

public:
	typedef std::map<DWORD, CItemData*> TItemMap;
	typedef std::map<std::string, CItemData*> TItemNameMap;

#ifdef ENABLE_BLEND_REWORK
	typedef struct SBlendInfo {
		DWORD	dwVnum;
		BYTE	bApplyType;
		std::vector<int>	vec_iApplyValue;
		std::vector<int>	vec_iApplyDuration;
	} TBlendInfo;
	typedef std::map<DWORD, TBlendInfo> TBlendMap;
#endif

public:
	CItemManager();
	virtual ~CItemManager();

	void			Destroy();

#ifdef ENABLE_WIKI_SYSTEM
	DWORD			FindItemSearchBox(const char* itemName, DWORD index, bool reverse = false);
	void			FindItem(PyObject* poHandler, int itemType);
#endif
	BOOL			SelectItemData(DWORD dwIndex);
	CItemData* GetSelectedItemDataPointer();

	BOOL			GetItemDataPointer(DWORD dwItemID, CItemData** ppItemData);

	/////
	bool			LoadItemDesc(const char* c_szFileName);
	bool			LoadItemList(const char* c_szFileName);
	bool			LoadItemTable(const char* c_szFileName);
	CItemData* MakeItemData(DWORD dwIndex);
#ifdef ENABLE_OFFLINE_SHOP
	void			GetItemsNameMap(std::map<DWORD, std::string>& inMap);
#endif
#ifdef ENABLE_ACCE_SYSTEM
	bool			LoadItemScale(const char* c_szFileName);
#endif

#ifdef ENABLE_SHINING_EFFECT_UTILITY
	bool			LoadShiningTable(const char* szShiningTable);
#endif

#ifdef ENABLE_BLEND_REWORK
	BOOL			GetBlendInfoPointer(DWORD dwItemID, TBlendInfo** ppBlendInfo);
	BYTE			GetAttrTypeByName(const std::string& stAttrType);
	bool			LoadItemBlend(const char* c_szFileName);
#endif

#ifdef ENABLE_DS_SET
public:
	bool				LoadDragonSoulTable(const char* szDragonSoulTable);
	CDragonSoulTable* GetDragonSoulTable();

protected:
	CDragonSoulTable* m_DragonSoulTable;
#endif

protected:
	TItemMap m_ItemMap;
	std::vector<CItemData*>  m_vec_ItemRange;
	CItemData* m_pSelectedItemData;
#ifdef ENABLE_BLEND_REWORK
	TBlendMap m_BlendMap;
#endif
};
