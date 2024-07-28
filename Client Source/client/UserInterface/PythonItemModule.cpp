#include "StdAfx.h"
#include "PythonItem.h"

#include "../gamelib/ItemManager.h"
#include "../gamelib/GameLibDefines.h"
#include "InstanceBase.h"
#include "AbstractApplication.h"
#ifdef ENABLE_DS_SET
#include "../GameLib/DragonSoulTable.h"
#endif

extern int TWOHANDED_WEWAPON_ATT_SPEED_DECREASE_VALUE;

#ifdef ENABLE_WIKI_SYSTEM
PyObject* itemFindItemSearchBox(PyObject* poSelf, PyObject* poArgs)
{
	char* sItemName;
	if (!PyTuple_GetString(poArgs, 0, &sItemName))
		return Py_BadArgument();
	int iIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &iIndex))
		return Py_BadArgument();
	bool reverse;
	if (!PyTuple_GetBoolean(poArgs, 2, &reverse))
		return Py_BadArgument();

	return Py_BuildValue("i", CItemManager::Instance().FindItemSearchBox(sItemName, iIndex, reverse));
}

PyObject* itemFindItems(PyObject* poSelf, PyObject* poArgs)
{
	PyObject* poHandler;
	if (!PyTuple_GetObject(poArgs, 0, &poHandler))
		return Py_BuildException();

	int itemType;
	if (!PyTuple_GetInteger(poArgs, 1, &itemType))
		return Py_BadArgument();

	CItemManager::Instance().FindItem(poHandler, itemType);

	return Py_BuildNone();
}
#endif

PyObject* itemSelectItem(PyObject* poSelf, PyObject* poArgs)
{
	int iIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iIndex))
		return Py_BadArgument();

	if (!CItemManager::Instance().SelectItemData(iIndex))
	{
		TraceError("Cannot find item by %d", iIndex);
		CItemManager::Instance().SelectItemData(60001);
	}

	return Py_BuildNone();
}

PyObject* itemIsAvaible(PyObject* poSelf, PyObject* poArgs)
{
	int iIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iIndex))
		return Py_BadArgument();

	CItemData *iData = nullptr;

	if (!CItemManager::Instance().GetItemDataPointer(iIndex, &iData))
	{
		return Py_BuildValue("i", false);
	}

	return Py_BuildValue("i", iData && iData->HasRealIcon());
}

PyObject* itemGetItemName(PyObject* poSelf, PyObject* poArgs)
{
	CItemData* pItemData = CItemManager::Instance().GetSelectedItemDataPointer();
	if (!pItemData)
		return Py_BuildValue("s", "None");

	return Py_BuildValue("s", pItemData->GetName());
}

PyObject* itemGetItemDescription(PyObject* poSelf, PyObject* poArgs)
{
	CItemData* pItemData = CItemManager::Instance().GetSelectedItemDataPointer();
	if (!pItemData)
		return Py_BuildValue("s", "None");

	return Py_BuildValue("s", pItemData->GetDescription());
}

PyObject* itemGetItemSummary(PyObject* poSelf, PyObject* poArgs)
{
	CItemData* pItemData = CItemManager::Instance().GetSelectedItemDataPointer();
	if (!pItemData)
		return Py_BuildValue("s", "None");

	return Py_BuildValue("s", pItemData->GetSummary());
}

PyObject* itemGetIconImage(PyObject* poSelf, PyObject* poArgs)
{
	CItemData* pItemData = CItemManager::Instance().GetSelectedItemDataPointer();
	if (!pItemData)
		return Py_BuildValue("s", "None");

	//	if (CItemData::ITEM_TYPE_SKILLBOOK == pItemData->GetType())
	//	{
	//		char szItemName[64+1];
	//		_snprintf(szItemName, "d:/ymir work/ui/items/etc/book_%02d.sub", );
	//		CGraphicImage * pImage = (CGraphicImage *)CResourceManager::Instance().GetResourcePointer(szItemName);
	//	}

	return Py_BuildValue("i", pItemData->GetIconImage());
}

PyObject* itemGetIconImageFileName(PyObject* poSelf, PyObject* poArgs)
{
	CItemData* pItemData = CItemManager::Instance().GetSelectedItemDataPointer();
	if (!pItemData)
		return Py_BuildValue("s", "None");

	CGraphicSubImage* pImage = pItemData->GetIconImage();
	if (!pImage)
		return Py_BuildValue("s", "Noname");

	return Py_BuildValue("s", pImage->GetFileName());
}

PyObject* itemGetItemSize(PyObject* poSelf, PyObject* poArgs)
{
	CItemData* pItemData = CItemManager::Instance().GetSelectedItemDataPointer();
	if (!pItemData)
		return Py_BuildValue("ii", 0, 0);

	return Py_BuildValue("(ii)", 1, pItemData->GetSize());
}

PyObject* itemGetItemType(PyObject* poSelf, PyObject* poArgs)
{
	CItemData* pItemData = CItemManager::Instance().GetSelectedItemDataPointer();
	if (!pItemData)
		return Py_BuildValue("i", 0);

	return Py_BuildValue("i", pItemData->GetType());
}

PyObject* itemGetItemSubType(PyObject* poSelf, PyObject* poArgs)
{
	CItemData* pItemData = CItemManager::Instance().GetSelectedItemDataPointer();
	if (!pItemData)
		return Py_BuildValue("i", 0);

	return Py_BuildValue("i", pItemData->GetSubType());
}

PyObject* itemGetIBuyItemPrice(PyObject* poSelf, PyObject* poArgs)
{
	CItemData* pItemData = CItemManager::Instance().GetSelectedItemDataPointer();

	if (!pItemData)
		return Py_BuildValue("i", 0);
#ifdef ENABLE_GOLD_LIMIT_REWORK
	return Py_BuildValue("L", pItemData->GetIBuyItemPrice());
#else
	return Py_BuildValue("i", pItemData->GetIBuyItemPrice());
#endif
}

PyObject* itemGetISellItemPrice(PyObject* poSelf, PyObject* poArgs)
{
	CItemData* pItemData = CItemManager::Instance().GetSelectedItemDataPointer();

	if (!pItemData)
		return Py_BuildValue("i", 0);
#ifdef ENABLE_GOLD_LIMIT_REWORK
	return Py_BuildValue("L", pItemData->GetISellItemPrice());
#else
	return Py_BuildValue("i", pItemData->GetISellItemPrice());
#endif
}

PyObject* itemIsAntiFlag(PyObject* poSelf, PyObject* poArgs)
{
	int iFlag;
	if (!PyTuple_GetInteger(poArgs, 0, &iFlag))
		return Py_BadArgument();

	CItemData* pItemData = CItemManager::Instance().GetSelectedItemDataPointer();
	if (!pItemData)
		return Py_BuildValue("i", 0);

	return Py_BuildValue("i", pItemData->IsAntiFlag(iFlag));
}

PyObject* itemIsFlag(PyObject* poSelf, PyObject* poArgs)
{
	int iFlag;
	if (!PyTuple_GetInteger(poArgs, 0, &iFlag))
		return Py_BadArgument();

	CItemData* pItemData = CItemManager::Instance().GetSelectedItemDataPointer();
	if (!pItemData)
		return Py_BuildValue("i", 0);

	return Py_BuildValue("i", pItemData->IsFlag(iFlag));
}

PyObject* itemIsWearableFlag(PyObject* poSelf, PyObject* poArgs)
{
	int iFlag;
	if (!PyTuple_GetInteger(poArgs, 0, &iFlag))
		return Py_BadArgument();

	CItemData* pItemData = CItemManager::Instance().GetSelectedItemDataPointer();
	if (!pItemData)
		return Py_BuildValue("i", 0);

	return Py_BuildValue("i", pItemData->IsWearableFlag(iFlag));
}

PyObject* itemIs1GoldItem(PyObject* poSelf, PyObject* poArgs)
{
	CItemData* pItemData = CItemManager::Instance().GetSelectedItemDataPointer();
	if (!pItemData)
		return Py_BuildValue("i", 0);

	return Py_BuildValue("i", pItemData->IsFlag(CItemData::ITEM_FLAG_COUNT_PER_1GOLD));
}

PyObject* itemGetLimit(PyObject* poSelf, PyObject* poArgs)
{
	int iValueIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iValueIndex))
		return Py_BadArgument();

	CItemData* pItemData = CItemManager::Instance().GetSelectedItemDataPointer();
	if (!pItemData)
		return Py_BuildException("Not yet select item data");

	CItemData::TItemLimit ItemLimit;
	if (!pItemData->GetLimit(iValueIndex, &ItemLimit))
		return Py_BuildException();

	return Py_BuildValue("ii", ItemLimit.bType, ItemLimit.lValue);
}

PyObject* itemGetAffect(PyObject* poSelf, PyObject* poArgs)
{
	int iValueIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iValueIndex))
		return Py_BadArgument();

	CItemData* pItemData = CItemManager::Instance().GetSelectedItemDataPointer();
	if (!pItemData)
		return Py_BuildException("Not yet select item data");

	CItemData::TItemApply ItemApply;
	if (!pItemData->GetApply(iValueIndex, &ItemApply))
		return Py_BuildException();

	if ((CItemData::APPLY_ATT_SPEED == ItemApply.bType) && (CItemData::ITEM_TYPE_WEAPON == pItemData->GetType()) && (CItemData::WEAPON_TWO_HANDED == pItemData->GetSubType()))
	{
		ItemApply.lValue -= TWOHANDED_WEWAPON_ATT_SPEED_DECREASE_VALUE;
	}

	return Py_BuildValue("ii", ItemApply.bType, ItemApply.lValue);
}

PyObject* itemGetValue(PyObject* poSelf, PyObject* poArgs)
{
	int iValueIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iValueIndex))
		return Py_BadArgument();

	CItemData* pItemData = CItemManager::Instance().GetSelectedItemDataPointer();
	if (!pItemData)
		return Py_BuildException("Not yet select item data");

	return Py_BuildValue("i", pItemData->GetValue(iValueIndex));
}

PyObject* itemGetSocket(PyObject* poSelf, PyObject* poArgs)
{
	int iValueIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iValueIndex))
		return Py_BadArgument();

	CItemData* pItemData = CItemManager::Instance().GetSelectedItemDataPointer();
	if (!pItemData)
		return Py_BuildException("Not yet select item data");

	return Py_BuildValue("i", pItemData->GetSocket(iValueIndex));
}

PyObject* itemGetIconInstance(PyObject* poSelf, PyObject* poArgs)
{
	CItemData* pItemData = CItemManager::Instance().GetSelectedItemDataPointer();
	if (!pItemData)
		return Py_BuildException("Not yet select item data");

	CGraphicSubImage* pImage = pItemData->GetIconImage();
	if (!pImage)
		return Py_BuildException("Cannot get icon image by %d", pItemData->GetIndex());

	CGraphicImageInstance* pImageInstance = CGraphicImageInstance::New();
	pImageInstance->SetImagePointer(pImage);

	return Py_BuildValue("i", pImageInstance);
}

PyObject* itemDeleteIconInstance(PyObject* poSelf, PyObject* poArgs)
{
	int iHandle;
	if (!PyTuple_GetInteger(poArgs, 0, &iHandle))
		return Py_BadArgument();

	CGraphicImageInstance::Delete((CGraphicImageInstance*)iHandle);

	return Py_BuildNone();
}

PyObject* itemIsEquipmentVID(PyObject* poSelf, PyObject* poArgs)
{
	int iItemVID;
	if (!PyTuple_GetInteger(poArgs, 0, &iItemVID))
		return Py_BadArgument();

	CItemManager::Instance().SelectItemData(iItemVID);
	CItemData* pItemData = CItemManager::Instance().GetSelectedItemDataPointer();
	if (!pItemData)
		return Py_BuildException("Not yet select item data");

	return Py_BuildValue("i", pItemData->IsEquipment());
}

PyObject* itemGetUseType(PyObject* poSelf, PyObject* poArgs)
{
	int iItemVID;
	if (!PyTuple_GetInteger(poArgs, 0, &iItemVID))
		return Py_BadArgument();

	CItemManager::Instance().SelectItemData(iItemVID);
	CItemData* pItemData = CItemManager::Instance().GetSelectedItemDataPointer();
	if (!pItemData)
		return Py_BuildException("Can't find select item data");

	return Py_BuildValue("s", pItemData->GetUseTypeString());
}

PyObject* itemIsRefineScroll(PyObject* poSelf, PyObject* poArgs)
{
	int iItemIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iItemIndex))
		return Py_BadArgument();

	CItemManager::Instance().SelectItemData(iItemIndex);
	CItemData* pItemData = CItemManager::Instance().GetSelectedItemDataPointer();
	if (!pItemData)
		return Py_BuildException("Can't find select item data");

	if (pItemData->GetType() != CItemData::ITEM_TYPE_USE)
		return Py_BuildValue("i", FALSE);

	switch (pItemData->GetSubType())
	{
	case CItemData::USE_TUNING:
		return Py_BuildValue("i", TRUE);
		break;
	}

	return Py_BuildValue("i", FALSE);
}

PyObject* itemIsDetachScroll(PyObject* poSelf, PyObject* poArgs)
{
	int iItemIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iItemIndex))
		return Py_BadArgument();

	CItemManager::Instance().SelectItemData(iItemIndex);
	CItemData* pItemData = CItemManager::Instance().GetSelectedItemDataPointer();
	if (!pItemData)
		return Py_BuildException("Can't find select item data");

	int iType = pItemData->GetType();
	int iSubType = pItemData->GetSubType();
	if (iType == CItemData::ITEM_TYPE_USE)
		if (iSubType == CItemData::USE_DETACHMENT)
		{
			return Py_BuildValue("i", TRUE);
		}

	return Py_BuildValue("i", FALSE);
}

PyObject* itemCanAddToQuickSlotItem(PyObject* poSelf, PyObject* poArgs)
{
	int iItemIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iItemIndex))
		return Py_BadArgument();

	CItemManager::Instance().SelectItemData(iItemIndex);
	CItemData* pItemData = CItemManager::Instance().GetSelectedItemDataPointer();
	if (!pItemData)
		return Py_BuildException("Can't find select item data");

	if (CItemData::ITEM_TYPE_USE == pItemData->GetType() || CItemData::ITEM_TYPE_QUEST == pItemData->GetType())
	{
		return Py_BuildValue("i", TRUE);
	}

	return Py_BuildValue("i", FALSE);
}

PyObject* itemIsKey(PyObject* poSelf, PyObject* poArgs)
{
	int iItemIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iItemIndex))
		return Py_BadArgument();

	CItemManager::Instance().SelectItemData(iItemIndex);
	CItemData* pItemData = CItemManager::Instance().GetSelectedItemDataPointer();
	if (!pItemData)
		return Py_BuildException("Can't find select item data");

	if (CItemData::ITEM_TYPE_TREASURE_KEY == pItemData->GetType())
	{
		return Py_BuildValue("i", TRUE);
	}

	return Py_BuildValue("i", FALSE);
}

PyObject* itemIsMetin(PyObject* poSelf, PyObject* poArgs)
{
	int iItemIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iItemIndex))
		return Py_BadArgument();

	CItemManager::Instance().SelectItemData(iItemIndex);
	CItemData* pItemData = CItemManager::Instance().GetSelectedItemDataPointer();
	if (!pItemData)
		return Py_BuildException("Can't find select item data");

	if (CItemData::ITEM_TYPE_METIN == pItemData->GetType())
	{
		return Py_BuildValue("i", TRUE);
	}

	return Py_BuildValue("i", FALSE);
}

PyObject* itemPick(PyObject* poSelf, PyObject* poArgs)
{
	DWORD dwItemID;
	if (CPythonItem::Instance().GetPickedItemID(&dwItemID))
		return Py_BuildValue("i", dwItemID);
	else
		return Py_BuildValue("i", -1);
}

PyObject* itemLoadItemTable(PyObject* poSelf, PyObject* poArgs)
{
	char* szFileName;
	if (!PyTuple_GetString(poArgs, 0, &szFileName))
		return Py_BadArgument();

	CItemManager::Instance().LoadItemTable(szFileName);
	return Py_BuildNone();
}

#ifdef ENABLE_SHOP_PRICE_TYPE_ITEM
PyObject* itemGtVnum(PyObject* poSelf, PyObject* poArgs)
{
	CItemData* pItemData = CItemManager::Instance().GetSelectedItemDataPointer();
	if (!pItemData)
		return Py_BuildValue("i", 0);

	return Py_BuildValue("i", pItemData->GetIndex());
}
#endif

#ifdef ENABLE_DETAILS_INTERFACE
PyObject* itemGetApplyPoint(PyObject* poSelf, PyObject* poArgs)
{
	int applyType = 0;
	if (!PyTuple_GetInteger(poArgs, 0, &applyType))
		return Py_BadArgument();

	return Py_BuildValue("i", ApplyTypeToPointType((BYTE)applyType));
}
PyObject* itemGetPointApply(PyObject* poSelf, PyObject* poArgs)
{
	int pointType = 0;
	if (!PyTuple_GetInteger(poArgs, 0, &pointType))
		return Py_BadArgument();

	return Py_BuildValue("i", PointTypeToApplyType((BYTE)pointType));
}
#endif

#ifdef ENABLE_EXTRA_SOCKET_SYSTEM
PyObject* itemIsUpgradeSocketScroll(PyObject* poSelf, PyObject* poArgs)
{
	int iItemIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iItemIndex))
		return Py_BadArgument();

	if (iItemIndex == 81100)
		return Py_BuildValue("i", 1);
	else if (iItemIndex == 81101)
		return Py_BuildValue("i", 2);
	else if (iItemIndex == 81102)
		return Py_BuildValue("i", 3);

	return Py_BuildValue("i", false);
}
#endif

#ifdef ENABLE_GROWTH_PET_SYSTEM
PyObject* itemIsNewPetBooks(PyObject* poSelf, PyObject* poArgs)
{
	int iItemIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iItemIndex))
		return Py_BadArgument();

	CItemManager::Instance().SelectItemData(iItemIndex);
	CItemData* pItemData = CItemManager::Instance().GetSelectedItemDataPointer();
	if (!pItemData)
		return Py_BuildException("Can't find select item data");

	if (pItemData->GetIndex() >= 55010 && pItemData->GetIndex() <= 55016)
		return Py_BuildValue("i", 1);

	return Py_BuildValue("i", 0);
}
#endif

#ifdef ENABLE_GROWTH_MOUNT_SYSTEM
PyObject* itemIsNewMountBooks(PyObject* poSelf, PyObject* poArgs)
{
	int iItemIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iItemIndex))
		return Py_BadArgument();

	CItemManager::Instance().SelectItemData(iItemIndex);
	CItemData* pItemData = CItemManager::Instance().GetSelectedItemDataPointer();
	if (!pItemData)
		return Py_BuildException("Can't find select item data");

	if (pItemData->GetIndex() >= 55210 && pItemData->GetIndex() <= 55216) // TODO:
		return Py_BuildValue("i", 1);

	return Py_BuildValue("i", 0);
}
#endif

#ifdef ENABLE_ADDITIONAL_INVENTORY
PyObject* itemIsAttrItem(PyObject* poSelf, PyObject* poArgs)
{
	int iItemIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iItemIndex))
		return Py_BadArgument();

	CItemManager::Instance().SelectItemData(iItemIndex);
	CItemData* pItemData = CItemManager::Instance().GetSelectedItemDataPointer();
	if (!pItemData)
		return Py_BuildException("Can't find select item data");

	switch (pItemData->GetIndex())
	{
		case 39028:
		case 39029:
		case 71084:
		case 71085:
		case 71189:
		case 71151:
		case 71152:
		case 76023:
		case 76024:
		case 76014:
		case 70024:
		case 826:
		case 827:
		case 99423:
		case 99424:
		case 99425:
		case 99426:
		case 99421:
		case 71086:
		case 71087:
		case 99422:
		case 30093:
		case 30094:
		case 30095:
		case 30096:
		case 30097:
		case 30100:
		case 30099:
		case 30098:
		case 30500:
		case 30501:
		case 30502:
		case 30503:
		case 30504:
		case 30505:
		case 30506:
		case 30507:
		case 30508:
		case 30509:
		case 30510:
		case 30511:
		case 30512:
		case 30513:
		case 30514:
		case 30515:
		case 30516:
		case 30517:
		case 30518:
		case 30519:
		case 30520:
		case 30521:
		case 30171:
		case 30172:
		case 70063:
		case 70064:
		case 30174:
		case 30173:
		case 30590:
		case 30591:
		case 30592:
		case 30593:
		case 56500:
		case 56501:
		case 56502:
		case 56503:
		case 56504:
		case 50390:
		case 50391:
		case 50392:
		case 50393:
		case 50595:
		case 30548:
		case 50596:
		case 30547:
		case 30543:
		case 30544:
		case 30541:
		case 30540:
		case 50597:
		case 30542:
		case 30545:
		case 30546:
		case 99427:
			return Py_BuildValue("i", TRUE);
	}

	return Py_BuildValue("i", FALSE);
}
#endif

#ifdef ENABLE_BLEND_REWORK
PyObject* itemIsBlend(PyObject* poSelf, PyObject* poArgs)
{
	int iItemIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iItemIndex))
		return Py_BadArgument();

	CItemManager::TBlendInfo* pBlendInfo;
	return Py_BuildValue("b", CItemManager::Instance().GetBlendInfoPointer(iItemIndex, &pBlendInfo));
}

PyObject* itemGetBlendApplyType(PyObject* poSelf, PyObject* poArgs)
{
	int iItemIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iItemIndex))
		return Py_BadArgument();

	CItemManager::TBlendInfo* pBlendInfo;
	if (!CItemManager::Instance().GetBlendInfoPointer(iItemIndex, &pBlendInfo))
		return Py_BuildValue("i", 0);

	return Py_BuildValue("i", pBlendInfo->bApplyType);
}

PyObject* itemGetBlendDataCount(PyObject* poSelf, PyObject* poArgs)
{
	int iItemIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iItemIndex))
		return Py_BadArgument();

	CItemManager::TBlendInfo* pBlendInfo;
	if (!CItemManager::Instance().GetBlendInfoPointer(iItemIndex, &pBlendInfo))
		return Py_BuildValue("i", 0);

	return Py_BuildValue("i", pBlendInfo->vec_iApplyValue.size());
}

PyObject* itemGetBlendData(PyObject* poSelf, PyObject* poArgs)
{
	int iItemIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iItemIndex))
		return Py_BadArgument();
	int iDataIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &iDataIndex))
		return Py_BadArgument();

	CItemManager::TBlendInfo* pBlendInfo;
	if (!CItemManager::Instance().GetBlendInfoPointer(iItemIndex, &pBlendInfo))
		return Py_BuildValue("(ii)", 0, 0);

	if (iDataIndex >= pBlendInfo->vec_iApplyValue.size())
	{
		TraceError("invalid data index for blend item %u [%d]", iItemIndex, iDataIndex);
		return Py_BuildException();
	}

	return Py_BuildValue("(ii)", pBlendInfo->vec_iApplyValue[iDataIndex], pBlendInfo->vec_iApplyDuration[iDataIndex]);
}
#endif

#ifdef ENABLE_DS_SET
PyObject* itemGetDSSetWeight(PyObject* poSelf, PyObject* poArgs)
{
	uint8_t iSetGrade;
	if (!PyTuple_GetInteger(poArgs, 0, &iSetGrade))
	{
		return Py_BadArgument();
	}

	CDragonSoulTable* dsTable = CItemManager::Instance().GetDragonSoulTable();
	if (!dsTable)
	{
		return Py_BuildException("DragonSoulTable not initalized");
	}

	return Py_BuildValue("i", dsTable->GetDSSetWeight(iSetGrade));
}

PyObject* itemGetDSBasicApplyCount(PyObject* poSelf, PyObject* poArgs)
{
	uint8_t iDSType;
	if (!PyTuple_GetInteger(poArgs, 0, &iDSType))
	{
		return Py_BadArgument();
	}

	CDragonSoulTable* dsTable = CItemManager::Instance().GetDragonSoulTable();
	if (!dsTable)
	{
		return Py_BuildException("DragonSoulTable not initalized");
	}

	return Py_BuildValue("i", dsTable->GetDSBasicApplyCount(iDSType));
}

PyObject* itemGetDSBasicApplyValue(PyObject* poSelf, PyObject* poArgs)
{
	uint8_t iDSType;
	if (!PyTuple_GetInteger(poArgs, 0, &iDSType))
	{
		return Py_BadArgument();
	}

	uint16_t iApplyType;
	if (!PyTuple_GetInteger(poArgs, 1, &iApplyType))
	{
		return Py_BadArgument();
	}

	CDragonSoulTable* dsTable = CItemManager::Instance().GetDragonSoulTable();
	if (!dsTable)
	{
		return Py_BuildException("DragonSoulTable not initalized");
	}

	return Py_BuildValue("i", dsTable->GetDSBasicApplyValue(iDSType, iApplyType));
}

PyObject* itemGetDSAdditionalApplyValue(PyObject* poSelf, PyObject* poArgs)
{
	uint8_t iDSType;
	if (!PyTuple_GetInteger(poArgs, 0, &iDSType))
	{
		return Py_BadArgument();
	}

	uint16_t iApplyType;
	if (!PyTuple_GetInteger(poArgs, 1, &iApplyType))
	{
		return Py_BadArgument();
	}

	CDragonSoulTable* dsTable = CItemManager::Instance().GetDragonSoulTable();
	if (!dsTable)
	{
		return Py_BuildException("DragonSoulTable not initalized");
	}

	return Py_BuildValue("i", dsTable->GetDSAdditionalApplyValue(iDSType, iApplyType));
}
#endif

void initItem()
{
	static PyMethodDef s_methods[] =
	{
#ifdef ENABLE_WIKI_SYSTEM
		{ "FindItemSearchBox",				itemFindItemSearchBox,					METH_VARARGS },
		{ "FindItems",						itemFindItems,							METH_VARARGS },
#endif
		{ "SelectItem",						itemSelectItem,							METH_VARARGS },
		{ "IsAvaible",						itemIsAvaible,							METH_VARARGS },

		{ "GetItemName",					itemGetItemName,						METH_VARARGS },
		{ "GetItemDescription",				itemGetItemDescription,					METH_VARARGS },
		{ "GetItemSummary",					itemGetItemSummary,						METH_VARARGS },
		{ "GetIconImage",					itemGetIconImage,						METH_VARARGS },
		{ "GetIconImageFileName",			itemGetIconImageFileName,				METH_VARARGS },
		{ "GetItemSize",					itemGetItemSize,						METH_VARARGS },
		{ "GetItemType",					itemGetItemType,						METH_VARARGS },
		{ "GetItemSubType",					itemGetItemSubType,						METH_VARARGS },
		{ "GetIBuyItemPrice",				itemGetIBuyItemPrice,					METH_VARARGS },
		{ "GetISellItemPrice",				itemGetISellItemPrice,					METH_VARARGS },
		{ "IsAntiFlag",						itemIsAntiFlag,							METH_VARARGS },
		{ "IsFlag",							itemIsFlag,								METH_VARARGS },
		{ "IsWearableFlag",					itemIsWearableFlag,						METH_VARARGS },
		{ "Is1GoldItem",					itemIs1GoldItem,						METH_VARARGS },
		{ "GetLimit",						itemGetLimit,							METH_VARARGS },
		{ "GetAffect",						itemGetAffect,							METH_VARARGS },
		{ "GetValue",						itemGetValue,							METH_VARARGS },
		{ "GetSocket",						itemGetSocket,							METH_VARARGS },
		{ "GetIconInstance",				itemGetIconInstance,					METH_VARARGS },
		{ "GetUseType",						itemGetUseType,							METH_VARARGS },
		{ "DeleteIconInstance",				itemDeleteIconInstance,					METH_VARARGS },
		{ "IsEquipmentVID",					itemIsEquipmentVID,						METH_VARARGS },
		{ "IsRefineScroll",					itemIsRefineScroll,						METH_VARARGS },
		{ "IsDetachScroll",					itemIsDetachScroll,						METH_VARARGS },
		{ "IsKey",							itemIsKey,								METH_VARARGS },
		{ "IsMetin",						itemIsMetin,							METH_VARARGS },
#ifdef ENABLE_ADDITIONAL_INVENTORY
		{ "IsAttrItem",						itemIsAttrItem,							METH_VARARGS },
#endif
		{ "CanAddToQuickSlotItem",			itemCanAddToQuickSlotItem,				METH_VARARGS },

		{ "Pick",							itemPick,								METH_VARARGS },

		{ "LoadItemTable",					itemLoadItemTable,						METH_VARARGS },
#ifdef ENABLE_SHOP_PRICE_TYPE_ITEM
		{ "GetVnum",						itemGtVnum,								METH_VARARGS },
#endif
#ifdef ENABLE_DETAILS_INTERFACE
		{ "GetApplyPoint",					itemGetApplyPoint,						METH_VARARGS },
		{ "GetPointApply",					itemGetPointApply,						METH_VARARGS },
#endif
#ifdef ENABLE_EXTRA_SOCKET_SYSTEM
		{ "IsUpgradeSocketScroll",			itemIsUpgradeSocketScroll,				METH_VARARGS },
#endif
#ifdef ENABLE_GROWTH_PET_SYSTEM
		{ "IsNewPetBooks",					itemIsNewPetBooks,						METH_VARARGS },
#endif
#ifdef ENABLE_GROWTH_MOUNT_SYSTEM
		{ "IsNewMountBooks",				itemIsNewMountBooks,					METH_VARARGS },
#endif
#ifdef ENABLE_BLEND_REWORK
		{ "IsBlend",						itemIsBlend,							METH_VARARGS },
		{ "GetBlendApplyType",				itemGetBlendApplyType,					METH_VARARGS },
		{ "GetBlendDataCount",				itemGetBlendDataCount,					METH_VARARGS },
		{ "GetBlendData",					itemGetBlendData,						METH_VARARGS },
#endif
#ifdef ENABLE_DS_SET
		{ "GetDSSetWeight",					itemGetDSSetWeight,						METH_VARARGS },
		{ "GetDSBasicApplyCount",			itemGetDSBasicApplyCount,				METH_VARARGS },
		{ "GetDSBasicApplyValue",			itemGetDSBasicApplyValue,				METH_VARARGS },
		{ "GetDSAdditionalApplyValue",		itemGetDSAdditionalApplyValue,			METH_VARARGS },
#endif
		{ NULL,								NULL,									NULL		 },
	};

	PyObject* poModule = Py_InitModule("item", s_methods);

	PyModule_AddIntConstant(poModule, "USESOUND_ACCESSORY", CPythonItem::USESOUND_ACCESSORY);
	PyModule_AddIntConstant(poModule, "USESOUND_ARMOR", CPythonItem::USESOUND_ARMOR);
	PyModule_AddIntConstant(poModule, "USESOUND_BOW", CPythonItem::USESOUND_BOW);
	PyModule_AddIntConstant(poModule, "USESOUND_DEFAULT", CPythonItem::USESOUND_DEFAULT);
	PyModule_AddIntConstant(poModule, "USESOUND_WEAPON", CPythonItem::USESOUND_WEAPON);
	PyModule_AddIntConstant(poModule, "USESOUND_POTION", CPythonItem::USESOUND_POTION);
	PyModule_AddIntConstant(poModule, "USESOUND_PORTAL", CPythonItem::USESOUND_PORTAL);

	PyModule_AddIntConstant(poModule, "DROPSOUND_ACCESSORY", CPythonItem::DROPSOUND_ACCESSORY);
	PyModule_AddIntConstant(poModule, "DROPSOUND_ARMOR", CPythonItem::DROPSOUND_ARMOR);
	PyModule_AddIntConstant(poModule, "DROPSOUND_BOW", CPythonItem::DROPSOUND_BOW);
	PyModule_AddIntConstant(poModule, "DROPSOUND_DEFAULT", CPythonItem::DROPSOUND_DEFAULT);
	PyModule_AddIntConstant(poModule, "DROPSOUND_WEAPON", CPythonItem::DROPSOUND_WEAPON);

	PyModule_AddIntConstant(poModule, "EQUIPMENT_COUNT", c_Equipment_Count);
	PyModule_AddIntConstant(poModule, "EQUIPMENT_HEAD", c_Equipment_Head);
	PyModule_AddIntConstant(poModule, "EQUIPMENT_BODY", c_Equipment_Body);
	PyModule_AddIntConstant(poModule, "EQUIPMENT_WEAPON", c_Equipment_Weapon);
	PyModule_AddIntConstant(poModule, "EQUIPMENT_WRIST", c_Equipment_Wrist);
	PyModule_AddIntConstant(poModule, "EQUIPMENT_SHOES", c_Equipment_Shoes);
	PyModule_AddIntConstant(poModule, "EQUIPMENT_NECK", c_Equipment_Neck);
	PyModule_AddIntConstant(poModule, "EQUIPMENT_EAR", c_Equipment_Ear);
	PyModule_AddIntConstant(poModule, "EQUIPMENT_UNIQUE1", c_Equipment_Unique1);
	PyModule_AddIntConstant(poModule, "EQUIPMENT_UNIQUE2", c_Equipment_Unique2);
	PyModule_AddIntConstant(poModule, "EQUIPMENT_ARROW", c_Equipment_Arrow);
	PyModule_AddIntConstant(poModule, "EQUIPMENT_SHIELD", c_Equipment_Shield);

#ifdef ENABLE_NEW_EQUIPMENT_SYSTEM
	PyModule_AddIntConstant(poModule, "EQUIPMENT_RING1", c_Equipment_Ring1);
	PyModule_AddIntConstant(poModule, "EQUIPMENT_RING2", c_Equipment_Ring2);
	PyModule_AddIntConstant(poModule, "EQUIPMENT_BELT", c_Equipment_Belt);
#endif
#ifdef ENABLE_PENDANT_SYSTEM
	PyModule_AddIntConstant(poModule, "EQUIPMENT_PENDANT", c_Equipment_Pendant);
#endif
#ifdef ENABLE_EXTENDED_PET_ITEM
	PyModule_AddIntConstant(poModule, "EQUIPMENT_PET", c_Equipment_Pet);
#endif
	PyModule_AddIntConstant(poModule, "ITEM_TYPE_NONE", CItemData::ITEM_TYPE_NONE);
	PyModule_AddIntConstant(poModule, "ITEM_TYPE_WEAPON", CItemData::ITEM_TYPE_WEAPON);
	PyModule_AddIntConstant(poModule, "ITEM_TYPE_ARMOR", CItemData::ITEM_TYPE_ARMOR);
	PyModule_AddIntConstant(poModule, "ITEM_TYPE_USE", CItemData::ITEM_TYPE_USE);
	PyModule_AddIntConstant(poModule, "ITEM_TYPE_AUTOUSE", CItemData::ITEM_TYPE_AUTOUSE);
	PyModule_AddIntConstant(poModule, "ITEM_TYPE_MATERIAL", CItemData::ITEM_TYPE_MATERIAL);
	PyModule_AddIntConstant(poModule, "ITEM_TYPE_SPECIAL", CItemData::ITEM_TYPE_SPECIAL);
	PyModule_AddIntConstant(poModule, "ITEM_TYPE_TOOL", CItemData::ITEM_TYPE_TOOL);
	PyModule_AddIntConstant(poModule, "ITEM_TYPE_LOTTERY", CItemData::ITEM_TYPE_LOTTERY);
	PyModule_AddIntConstant(poModule, "ITEM_TYPE_ELK", CItemData::ITEM_TYPE_ELK);
	PyModule_AddIntConstant(poModule, "ITEM_TYPE_METIN", CItemData::ITEM_TYPE_METIN);
	PyModule_AddIntConstant(poModule, "ITEM_TYPE_CONTAINER", CItemData::ITEM_TYPE_CONTAINER);
	PyModule_AddIntConstant(poModule, "ITEM_TYPE_FISH", CItemData::ITEM_TYPE_FISH);
	PyModule_AddIntConstant(poModule, "ITEM_TYPE_ROD", CItemData::ITEM_TYPE_ROD);
	PyModule_AddIntConstant(poModule, "ITEM_TYPE_RESOURCE", CItemData::ITEM_TYPE_RESOURCE);
	PyModule_AddIntConstant(poModule, "ITEM_TYPE_CAMPFIRE", CItemData::ITEM_TYPE_CAMPFIRE);
	PyModule_AddIntConstant(poModule, "ITEM_TYPE_UNIQUE", CItemData::ITEM_TYPE_UNIQUE);
	PyModule_AddIntConstant(poModule, "ITEM_TYPE_SKILLBOOK", CItemData::ITEM_TYPE_SKILLBOOK);
	PyModule_AddIntConstant(poModule, "ITEM_TYPE_QUEST", CItemData::ITEM_TYPE_QUEST);
	PyModule_AddIntConstant(poModule, "ITEM_TYPE_POLYMORPH", CItemData::ITEM_TYPE_POLYMORPH);
	PyModule_AddIntConstant(poModule, "ITEM_TYPE_TREASURE_BOX", CItemData::ITEM_TYPE_TREASURE_BOX);
	PyModule_AddIntConstant(poModule, "ITEM_TYPE_TREASURE_KEY", CItemData::ITEM_TYPE_TREASURE_KEY);
	PyModule_AddIntConstant(poModule, "ITEM_TYPE_SKILLFORGET", CItemData::ITEM_TYPE_SKILLFORGET);
	PyModule_AddIntConstant(poModule, "ITEM_TYPE_GIFTBOX", CItemData::ITEM_TYPE_GIFTBOX);
	PyModule_AddIntConstant(poModule, "ITEM_TYPE_PICK", CItemData::ITEM_TYPE_PICK);
	PyModule_AddIntConstant(poModule, "ITEM_TYPE_BLEND", CItemData::ITEM_TYPE_BLEND);
	PyModule_AddIntConstant(poModule, "ITEM_TYPE_DS", CItemData::ITEM_TYPE_DS);
	PyModule_AddIntConstant(poModule, "ITEM_TYPE_SPECIAL_DS", CItemData::ITEM_TYPE_SPECIAL_DS);
	PyModule_AddIntConstant(poModule, "ITEM_TYPE_RING", CItemData::ITEM_TYPE_RING);
	PyModule_AddIntConstant(poModule, "ITEM_TYPE_BELT", CItemData::ITEM_TYPE_BELT);
	PyModule_AddIntConstant(poModule, "ITEM_TYPE_PET", CItemData::ITEM_TYPE_PET);
	PyModule_AddIntConstant(poModule, "ITEM_TYPE_MEDIUM", CItemData::ITEM_TYPE_MEDIUM);
	PyModule_AddIntConstant(poModule, "ITEM_TYPE_GACHA", CItemData::ITEM_TYPE_GACHA);
	PyModule_AddIntConstant(poModule, "ITEM_TYPE_SOUL", CItemData::ITEM_TYPE_SOUL);
#ifdef ENABLE_COSTUME_SYSTEM
	PyModule_AddIntConstant(poModule, "ITEM_TYPE_COSTUME", CItemData::ITEM_TYPE_COSTUME);

	// Item Sub Type
	PyModule_AddIntConstant(poModule, "COSTUME_TYPE_BODY", CItemData::COSTUME_BODY);
	PyModule_AddIntConstant(poModule, "COSTUME_TYPE_HAIR", CItemData::COSTUME_HAIR);
#ifdef ENABLE_MOUNT_COSTUME_SYSTEM
	PyModule_AddIntConstant(poModule, "COSTUME_TYPE_MOUNT", CItemData::COSTUME_MOUNT);
#endif
#ifdef ENABLE_ACCE_SYSTEM
	PyModule_AddIntConstant(poModule, "COSTUME_TYPE_ACCE", CItemData::COSTUME_ACCE);
#endif
#ifdef ENABLE_AURA_COSTUME_SYSTEM
	PyModule_AddIntConstant(poModule, "COSTUME_TYPE_AURA", CItemData::COSTUME_AURA);
#endif
#ifdef ENABLE_WEAPON_COSTUME_SYSTEM
	PyModule_AddIntConstant(poModule, "COSTUME_TYPE_WEAPON", CItemData::COSTUME_WEAPON);
#endif
#ifdef ENABLE_BLEND_REWORK
	PyModule_AddIntConstant(poModule, "NORMAL_BLEND", CItemData::NORMAL_BLEND);
	PyModule_AddIntConstant(poModule, "INFINITY_BLEND", CItemData::INFINITY_BLEND);
#endif
	PyModule_AddIntConstant(poModule, "COSTUME_SLOT_START", c_Costume_Slot_Start);
	PyModule_AddIntConstant(poModule, "COSTUME_SLOT_COUNT", c_Costume_Slot_Count);
	PyModule_AddIntConstant(poModule, "COSTUME_SLOT_BODY", c_Costume_Slot_Body);
	PyModule_AddIntConstant(poModule, "COSTUME_SLOT_HAIR", c_Costume_Slot_Hair);
#ifdef ENABLE_MOUNT_COSTUME_SYSTEM
	PyModule_AddIntConstant(poModule, "COSTUME_SLOT_MOUNT", c_Costume_Slot_Mount);
#endif
#ifdef ENABLE_ACCE_SYSTEM
	PyModule_AddIntConstant(poModule, "COSTUME_TYPE_ACCE", CItemData::COSTUME_ACCE);
	PyModule_AddIntConstant(poModule, "COSTUME_SLOT_ACCE", c_Costume_Slot_Acce);
#endif
#ifdef ENABLE_AURA_COSTUME_SYSTEM
	PyModule_AddIntConstant(poModule, "COSTUME_TYPE_AURA", CItemData::COSTUME_AURA);
	PyModule_AddIntConstant(poModule, "COSTUME_SLOT_AURA", c_Costume_Slot_Aura);
#endif
#ifdef ENABLE_WEAPON_COSTUME_SYSTEM
	PyModule_AddIntConstant(poModule, "COSTUME_SLOT_WEAPON", c_Costume_Slot_Weapon);
#endif
	PyModule_AddIntConstant(poModule, "COSTUME_SLOT_END", c_Costume_Slot_End);
#endif

#ifdef ENABLE_NEW_EQUIPMENT_SYSTEM
	PyModule_AddIntConstant(poModule, "BELT_INVENTORY_SLOT_START", c_Belt_Inventory_Slot_Start);
	PyModule_AddIntConstant(poModule, "BELT_INVENTORY_SLOT_COUNT", c_Belt_Inventory_Slot_Count);
	PyModule_AddIntConstant(poModule, "BELT_INVENTORY_SLOT_END", c_Belt_Inventory_Slot_End);

#endif

	PyModule_AddIntConstant(poModule, "WEAPON_SWORD", CItemData::WEAPON_SWORD);
	PyModule_AddIntConstant(poModule, "WEAPON_DAGGER", CItemData::WEAPON_DAGGER);
	PyModule_AddIntConstant(poModule, "WEAPON_BOW", CItemData::WEAPON_BOW);
	PyModule_AddIntConstant(poModule, "WEAPON_TWO_HANDED", CItemData::WEAPON_TWO_HANDED);
	PyModule_AddIntConstant(poModule, "WEAPON_BELL", CItemData::WEAPON_BELL);
	PyModule_AddIntConstant(poModule, "WEAPON_FAN", CItemData::WEAPON_FAN);
	PyModule_AddIntConstant(poModule, "WEAPON_ARROW", CItemData::WEAPON_ARROW);
	PyModule_AddIntConstant(poModule, "WEAPON_MOUNT_SPEAR", CItemData::WEAPON_MOUNT_SPEAR);
	PyModule_AddIntConstant(poModule, "WEAPON_CLAW", CItemData::WEAPON_CLAW);
	PyModule_AddIntConstant(poModule, "WEAPON_QUIVER", CItemData::WEAPON_QUIVER);
	PyModule_AddIntConstant(poModule, "WEAPON_NUM_TYPES", CItemData::WEAPON_NUM_TYPES);

	PyModule_AddIntConstant(poModule, "USE_POTION", CItemData::USE_POTION);
	PyModule_AddIntConstant(poModule, "USE_TALISMAN", CItemData::USE_TALISMAN);
	PyModule_AddIntConstant(poModule, "USE_TUNING", CItemData::USE_TUNING);
	PyModule_AddIntConstant(poModule, "USE_MOVE", CItemData::USE_MOVE);
	PyModule_AddIntConstant(poModule, "USE_TREASURE_BOX", CItemData::USE_TREASURE_BOX);
	PyModule_AddIntConstant(poModule, "USE_MONEYBAG", CItemData::USE_MONEYBAG);
	PyModule_AddIntConstant(poModule, "USE_BAIT", CItemData::USE_BAIT);
	PyModule_AddIntConstant(poModule, "USE_ABILITY_UP", CItemData::USE_ABILITY_UP);
	PyModule_AddIntConstant(poModule, "USE_AFFECT", CItemData::USE_AFFECT);
	PyModule_AddIntConstant(poModule, "USE_CREATE_STONE", CItemData::USE_CREATE_STONE);
	PyModule_AddIntConstant(poModule, "USE_SPECIAL", CItemData::USE_SPECIAL);
	PyModule_AddIntConstant(poModule, "USE_POTION_NODELAY", CItemData::USE_POTION_NODELAY);
	PyModule_AddIntConstant(poModule, "USE_CLEAR", CItemData::USE_CLEAR);
	PyModule_AddIntConstant(poModule, "USE_INVISIBILITY", CItemData::USE_INVISIBILITY);
	PyModule_AddIntConstant(poModule, "USE_DETACHMENT", CItemData::USE_DETACHMENT);
	PyModule_AddIntConstant(poModule, "USE_TIME_CHARGE_PER", CItemData::USE_TIME_CHARGE_PER);
	PyModule_AddIntConstant(poModule, "USE_TIME_CHARGE_FIX", CItemData::USE_TIME_CHARGE_FIX);
	PyModule_AddIntConstant(poModule, "USE_PUT_INTO_BELT_SOCKET", CItemData::USE_PUT_INTO_BELT_SOCKET);
	PyModule_AddIntConstant(poModule, "USE_PUT_INTO_RING_SOCKET", CItemData::USE_PUT_INTO_RING_SOCKET);
	PyModule_AddIntConstant(poModule, "USE_ADD_ACCESSORY_SOCKET", CItemData::USE_ADD_ACCESSORY_SOCKET);
	PyModule_AddIntConstant(poModule, "USE_CHANGE_COSTUME_ATTR", CItemData::USE_CHANGE_COSTUME_ATTR);
	PyModule_AddIntConstant(poModule, "USE_RESET_COSTUME_ATTR", CItemData::USE_RESET_COSTUME_ATTR);
	PyModule_AddIntConstant(poModule, "USE_FLOWER", CItemData::USE_FLOWER);
	PyModule_AddIntConstant(poModule, "USE_UNKNOWN_TYPE", CItemData::USE_UNKNOWN_TYPE);
	PyModule_AddIntConstant(poModule, "USE_PET", CItemData::USE_PET);

	PyModule_AddIntConstant(poModule, "MATERIAL_DS_REFINE_NORMAL", CItemData::MATERIAL_DS_REFINE_NORMAL);
	PyModule_AddIntConstant(poModule, "MATERIAL_DS_REFINE_BLESSED", CItemData::MATERIAL_DS_REFINE_BLESSED);
	PyModule_AddIntConstant(poModule, "MATERIAL_DS_REFINE_HOLLY", CItemData::MATERIAL_DS_REFINE_HOLLY);
#ifdef ENABLE_DSS_REFINE_ITEM
	PyModule_AddIntConstant(poModule, "MATERIAL_DS_REFINE_1", CItemData::MATERIAL_DS_REFINE_1);
	PyModule_AddIntConstant(poModule, "MATERIAL_DS_REFINE_2", CItemData::MATERIAL_DS_REFINE_2);
	PyModule_AddIntConstant(poModule, "MATERIAL_DS_REFINE_3", CItemData::MATERIAL_DS_REFINE_3);
	PyModule_AddIntConstant(poModule, "MATERIAL_DS_REFINE_4", CItemData::MATERIAL_DS_REFINE_4);
	PyModule_AddIntConstant(poModule, "MATERIAL_DS_REFINE_5", CItemData::MATERIAL_DS_REFINE_5);
#endif
	PyModule_AddIntConstant(poModule, "FISH_ALIVE", 0);
	PyModule_AddIntConstant(poModule, "FISH_DEAD", 1);

	PyModule_AddIntConstant(poModule, "RESOURCE_FISHBONE", 0);
	PyModule_AddIntConstant(poModule, "RESOURCE_WATERSTONEPIECE", 1);
	PyModule_AddIntConstant(poModule, "RESOURCE_WATERSTONE", 2);
	PyModule_AddIntConstant(poModule, "RESOURCE_BLOOD_PEARL", 3);
	PyModule_AddIntConstant(poModule, "RESOURCE_BLUE_PEARL", 4);
	PyModule_AddIntConstant(poModule, "RESOURCE_WHITE_PEARL", 5);
	PyModule_AddIntConstant(poModule, "RESOURCE_BUCKET", 6);
	PyModule_AddIntConstant(poModule, "RESOURCE_CRYSTAL", 7);
	PyModule_AddIntConstant(poModule, "RESOURCE_GEM", 8);
	PyModule_AddIntConstant(poModule, "RESOURCE_STONE", 9);
	PyModule_AddIntConstant(poModule, "RESOURCE_METIN", 10);
	PyModule_AddIntConstant(poModule, "RESOURCE_ORE", 11);

	PyModule_AddIntConstant(poModule, "DS_WHITE", 0);
	PyModule_AddIntConstant(poModule, "DS_RED", 1);
	PyModule_AddIntConstant(poModule, "DS_GREEN", 2);
	PyModule_AddIntConstant(poModule, "DS_BLUE", 3);
	PyModule_AddIntConstant(poModule, "DS_YELLOW", 4);
	PyModule_AddIntConstant(poModule, "DS_BLACK", 5);

	PyModule_AddIntConstant(poModule, "METIN_NORMAL", CItemData::METIN_NORMAL);
	PyModule_AddIntConstant(poModule, "METIN_GOLD", CItemData::METIN_GOLD);

	PyModule_AddIntConstant(poModule, "LIMIT_NONE", CItemData::LIMIT_NONE);
	PyModule_AddIntConstant(poModule, "LIMIT_LEVEL", CItemData::LIMIT_LEVEL);
	PyModule_AddIntConstant(poModule, "LIMIT_STR", CItemData::LIMIT_STR);
	PyModule_AddIntConstant(poModule, "LIMIT_DEX", CItemData::LIMIT_DEX);
	PyModule_AddIntConstant(poModule, "LIMIT_INT", CItemData::LIMIT_INT);
	PyModule_AddIntConstant(poModule, "LIMIT_CON", CItemData::LIMIT_CON);
	PyModule_AddIntConstant(poModule, "LIMIT_UNUSED", CItemData::LIMIT_UNUSED);
	PyModule_AddIntConstant(poModule, "LIMIT_REAL_TIME", CItemData::LIMIT_REAL_TIME);
	PyModule_AddIntConstant(poModule, "LIMIT_REAL_TIME_START_FIRST_USE", CItemData::LIMIT_REAL_TIME_START_FIRST_USE);
	PyModule_AddIntConstant(poModule, "LIMIT_TIMER_BASED_ON_WEAR", CItemData::LIMIT_TIMER_BASED_ON_WEAR);
	PyModule_AddIntConstant(poModule, "LIMIT_TYPE_MAX_NUM", CItemData::LIMIT_MAX_NUM);
	PyModule_AddIntConstant(poModule, "LIMIT_MAX_NUM", CItemData::ITEM_LIMIT_MAX_NUM);

	PyModule_AddIntConstant(poModule, "ITEM_ANTIFLAG_FEMALE", CItemData::ITEM_ANTIFLAG_FEMALE);
	PyModule_AddIntConstant(poModule, "ITEM_ANTIFLAG_MALE", CItemData::ITEM_ANTIFLAG_MALE);
	PyModule_AddIntConstant(poModule, "ITEM_ANTIFLAG_WARRIOR", CItemData::ITEM_ANTIFLAG_WARRIOR);
	PyModule_AddIntConstant(poModule, "ITEM_ANTIFLAG_ASSASSIN", CItemData::ITEM_ANTIFLAG_ASSASSIN);
	PyModule_AddIntConstant(poModule, "ITEM_ANTIFLAG_SURA", CItemData::ITEM_ANTIFLAG_SURA);
	PyModule_AddIntConstant(poModule, "ITEM_ANTIFLAG_SHAMAN", CItemData::ITEM_ANTIFLAG_SHAMAN);
	PyModule_AddIntConstant(poModule, "ITEM_ANTIFLAG_WOLFMAN", CItemData::ITEM_ANTIFLAG_WOLFMAN);
	PyModule_AddIntConstant(poModule, "ITEM_ANTIFLAG_GET", CItemData::ITEM_ANTIFLAG_GET);
	PyModule_AddIntConstant(poModule, "ITEM_ANTIFLAG_DROP", CItemData::ITEM_ANTIFLAG_DROP);
	PyModule_AddIntConstant(poModule, "ITEM_ANTIFLAG_SELL", CItemData::ITEM_ANTIFLAG_SELL);
	PyModule_AddIntConstant(poModule, "ITEM_ANTIFLAG_EMPIRE_A", CItemData::ITEM_ANTIFLAG_EMPIRE_A);
	PyModule_AddIntConstant(poModule, "ITEM_ANTIFLAG_EMPIRE_B", CItemData::ITEM_ANTIFLAG_EMPIRE_B);
	PyModule_AddIntConstant(poModule, "ITEM_ANTIFLAG_EMPIRE_R", CItemData::ITEM_ANTIFLAG_EMPIRE_R);
	PyModule_AddIntConstant(poModule, "ITEM_ANTIFLAG_SAVE", CItemData::ITEM_ANTIFLAG_SAVE);
	PyModule_AddIntConstant(poModule, "ITEM_ANTIFLAG_GIVE", CItemData::ITEM_ANTIFLAG_GIVE);
	PyModule_AddIntConstant(poModule, "ITEM_ANTIFLAG_PKDROP", CItemData::ITEM_ANTIFLAG_PKDROP);
	PyModule_AddIntConstant(poModule, "ITEM_ANTIFLAG_STACK", CItemData::ITEM_ANTIFLAG_STACK);
	PyModule_AddIntConstant(poModule, "ITEM_ANTIFLAG_MYSHOP", CItemData::ITEM_ANTIFLAG_MYSHOP);
	PyModule_AddIntConstant(poModule, "ITEM_ANTIFLAG_SAFEBOX", CItemData::ITEM_ANTIFLAG_SAFEBOX);
#ifdef ENABLE_SLOT_MARKING_SYSTEM
	PyModule_AddIntConstant(poModule, "ITEM_ANTIFLAG_QUICKSLOT", CItemData::ITEM_ANTIFLAG_QUICKSLOT);
	PyModule_AddIntConstant(poModule, "ITEM_ANTIFLAG_PET", CItemData::ITEM_ANTIFLAG_PET);
	PyModule_AddIntConstant(poModule, "ITEM_ANTIFLAG_CHANGELOOK", CItemData::ITEM_ANTIFLAG_CHANGELOOK);
	PyModule_AddIntConstant(poModule, "ITEM_ANTIFLAG_REINFORCE", CItemData::ITEM_ANTIFLAG_REINFORCE);
	PyModule_AddIntConstant(poModule, "ITEM_ANTIFLAG_ENCHANT", CItemData::ITEM_ANTIFLAG_ENCHANT);
	PyModule_AddIntConstant(poModule, "ITEM_ANTIFLAG_ENERGY", CItemData::ITEM_ANTIFLAG_ENERGY);
	PyModule_AddIntConstant(poModule, "ITEM_ANTIFLAG_PETFEED", CItemData::ITEM_ANTIFLAG_PETFEED);
	PyModule_AddIntConstant(poModule, "ITEM_ANTIFLAG_APPLY", CItemData::ITEM_ANTIFLAG_APPLY);
	PyModule_AddIntConstant(poModule, "ITEM_ANTIFLAG_ACCE", CItemData::ITEM_ANTIFLAG_ACCE);
	PyModule_AddIntConstant(poModule, "ITEM_ANTIFLAG_MAIL", CItemData::ITEM_ANTIFLAG_MAIL);
	PyModule_AddIntConstant(poModule, "ITEM_ANTIFLAG_AURA", CItemData::ITEM_ANTIFLAG_AURA);
	PyModule_AddIntConstant(poModule, "ITEM_ANTIFLAG_FORCE_ALL", CItemData::ITEM_ANTIFLAG_FORCE_ALL);
	PyModule_AddIntConstant(poModule, "ITEM_ANTIFLAG_COMB", CItemData::ITEM_ANTIFLAG_COMB);
#endif
	PyModule_AddIntConstant(poModule, "ITEM_FLAG_RARE", CItemData::ITEM_FLAG_RARE);
	PyModule_AddIntConstant(poModule, "ITEM_FLAG_UNIQUE", CItemData::ITEM_FLAG_UNIQUE);
	PyModule_AddIntConstant(poModule, "ITEM_FLAG_CONFIRM_WHEN_USE", CItemData::ITEM_FLAG_CONFIRM_WHEN_USE);

	PyModule_AddIntConstant(poModule, "ANTIFLAG_FEMALE", CItemData::ITEM_ANTIFLAG_FEMALE);
	PyModule_AddIntConstant(poModule, "ANTIFLAG_MALE", CItemData::ITEM_ANTIFLAG_MALE);
	PyModule_AddIntConstant(poModule, "ANTIFLAG_WARRIOR", CItemData::ITEM_ANTIFLAG_WARRIOR);
	PyModule_AddIntConstant(poModule, "ANTIFLAG_ASSASSIN", CItemData::ITEM_ANTIFLAG_ASSASSIN);
	PyModule_AddIntConstant(poModule, "ANTIFLAG_SURA", CItemData::ITEM_ANTIFLAG_SURA);
	PyModule_AddIntConstant(poModule, "ANTIFLAG_SHAMAN", CItemData::ITEM_ANTIFLAG_SHAMAN);
	PyModule_AddIntConstant(poModule, "ANTIFLAG_GET", CItemData::ITEM_ANTIFLAG_GET);
	PyModule_AddIntConstant(poModule, "ANTIFLAG_DROP", CItemData::ITEM_ANTIFLAG_DROP);
	PyModule_AddIntConstant(poModule, "ANTIFLAG_SELL", CItemData::ITEM_ANTIFLAG_SELL);
	PyModule_AddIntConstant(poModule, "ANTIFLAG_EMPIRE_A", CItemData::ITEM_ANTIFLAG_EMPIRE_A);
	PyModule_AddIntConstant(poModule, "ANTIFLAG_EMPIRE_B", CItemData::ITEM_ANTIFLAG_EMPIRE_B);
	PyModule_AddIntConstant(poModule, "ANTIFLAG_EMPIRE_R", CItemData::ITEM_ANTIFLAG_EMPIRE_R);
	PyModule_AddIntConstant(poModule, "ANTIFLAG_SAVE", CItemData::ITEM_ANTIFLAG_SAVE);
	PyModule_AddIntConstant(poModule, "ANTIFLAG_GIVE", CItemData::ITEM_ANTIFLAG_GIVE);
	PyModule_AddIntConstant(poModule, "ANTIFLAG_PKDROP", CItemData::ITEM_ANTIFLAG_PKDROP);
	PyModule_AddIntConstant(poModule, "ANTIFLAG_STACK", CItemData::ITEM_ANTIFLAG_STACK);
	PyModule_AddIntConstant(poModule, "ANTIFLAG_MYSHOP", CItemData::ITEM_ANTIFLAG_MYSHOP);
	PyModule_AddIntConstant(poModule, "ANTIFLAG_SAFEBOX", CItemData::ITEM_ANTIFLAG_SAFEBOX);
	PyModule_AddIntConstant(poModule, "ANTIFLAG_WOLFMAN", CItemData::ITEM_ANTIFLAG_WOLFMAN);
	PyModule_AddIntConstant(poModule, "ANTIFLAG_QUICKSLOT", CItemData::ITEM_ANTIFLAG_QUICKSLOT);
	PyModule_AddIntConstant(poModule, "ANTIFLAG_PET", CItemData::ITEM_ANTIFLAG_PET);
	PyModule_AddIntConstant(poModule, "ANTIFLAG_CHANGELOOK", CItemData::ITEM_ANTIFLAG_CHANGELOOK);
	PyModule_AddIntConstant(poModule, "ANTIFLAG_REINFORCE", CItemData::ITEM_ANTIFLAG_REINFORCE);
	PyModule_AddIntConstant(poModule, "ANTIFLAG_ENCHANT", CItemData::ITEM_ANTIFLAG_ENCHANT);
	PyModule_AddIntConstant(poModule, "ANTIFLAG_ENERGY", CItemData::ITEM_ANTIFLAG_ENERGY);
	PyModule_AddIntConstant(poModule, "ANTIFLAG_PETFEED", CItemData::ITEM_ANTIFLAG_PETFEED);
	PyModule_AddIntConstant(poModule, "ANTIFLAG_APPLY", CItemData::ITEM_ANTIFLAG_APPLY);
	PyModule_AddIntConstant(poModule, "ANTIFLAG_ACCE", CItemData::ITEM_ANTIFLAG_ACCE);
	PyModule_AddIntConstant(poModule, "ANTIFLAG_MAIL", CItemData::ITEM_ANTIFLAG_MAIL);

	PyModule_AddIntConstant(poModule, "ITEM_FLAG_REFINEABLE", CItemData::ITEM_FLAG_REFINEABLE);
	PyModule_AddIntConstant(poModule, "ITEM_FLAG_SAVE", CItemData::ITEM_FLAG_SAVE);
	PyModule_AddIntConstant(poModule, "ITEM_FLAG_STACKABLE", CItemData::ITEM_FLAG_STACKABLE);
	PyModule_AddIntConstant(poModule, "ITEM_FLAG_COUNT_PER_1GOLD", CItemData::ITEM_FLAG_COUNT_PER_1GOLD);
	PyModule_AddIntConstant(poModule, "ITEM_FLAG_SLOW_QUERY", CItemData::ITEM_FLAG_SLOW_QUERY);
	PyModule_AddIntConstant(poModule, "ITEM_FLAG_RARE", CItemData::ITEM_FLAG_RARE);
	PyModule_AddIntConstant(poModule, "ITEM_FLAG_UNIQUE", CItemData::ITEM_FLAG_UNIQUE);
	PyModule_AddIntConstant(poModule, "ITEM_FLAG_MAKECOUNT", CItemData::ITEM_FLAG_MAKECOUNT);
	PyModule_AddIntConstant(poModule, "ITEM_FLAG_IRREMOVABLE", CItemData::ITEM_FLAG_IRREMOVABLE);
	PyModule_AddIntConstant(poModule, "ITEM_FLAG_CONFIRM_WHEN_USE", CItemData::ITEM_FLAG_CONFIRM_WHEN_USE);
	PyModule_AddIntConstant(poModule, "ITEM_FLAG_QUEST_USE", CItemData::ITEM_FLAG_QUEST_USE);
	PyModule_AddIntConstant(poModule, "ITEM_FLAG_QUEST_USE_MULTIPLE", CItemData::ITEM_FLAG_QUEST_USE_MULTIPLE);
	PyModule_AddIntConstant(poModule, "ITEM_FLAG_QUEST_GIVE", CItemData::ITEM_FLAG_QUEST_GIVE);
	PyModule_AddIntConstant(poModule, "ITEM_FLAG_LOG", CItemData::ITEM_FLAG_LOG);
	PyModule_AddIntConstant(poModule, "ITEM_FLAG_APPLICABLE", CItemData::ITEM_FLAG_APPLICABLE);
	PyModule_AddIntConstant(poModule, "ITEM_FLAG_RARE_ABILITY", CItemData::ITEM_FLAG_RARE_ABILITY);
	PyModule_AddIntConstant(poModule, "ITEM_FLAG_LOG_SPECIAL", CItemData::ITEM_FLAG_LOG_SPECIAL);

	PyModule_AddIntConstant(poModule, "WEARABLE_BODY", CItemData::WEARABLE_BODY);
	PyModule_AddIntConstant(poModule, "WEARABLE_HEAD", CItemData::WEARABLE_HEAD);
	PyModule_AddIntConstant(poModule, "WEARABLE_FOOTS", CItemData::WEARABLE_FOOTS);
	PyModule_AddIntConstant(poModule, "WEARABLE_WRIST", CItemData::WEARABLE_WRIST);
	PyModule_AddIntConstant(poModule, "WEARABLE_WEAPON", CItemData::WEARABLE_WEAPON);
	PyModule_AddIntConstant(poModule, "WEARABLE_NECK", CItemData::WEARABLE_NECK);
	PyModule_AddIntConstant(poModule, "WEARABLE_EAR", CItemData::WEARABLE_EAR);
	PyModule_AddIntConstant(poModule, "WEARABLE_UNIQUE", CItemData::WEARABLE_UNIQUE);
	PyModule_AddIntConstant(poModule, "WEARABLE_SHIELD", CItemData::WEARABLE_SHIELD);
	PyModule_AddIntConstant(poModule, "WEARABLE_ARROW", CItemData::WEARABLE_ARROW);
	PyModule_AddIntConstant(poModule, "WEARABLE_PENDANT", CItemData::WEARABLE_PENDANT);

	PyModule_AddIntConstant(poModule, "ARMOR_BODY", CItemData::ARMOR_BODY);
	PyModule_AddIntConstant(poModule, "ARMOR_HEAD", CItemData::ARMOR_HEAD);
	PyModule_AddIntConstant(poModule, "ARMOR_SHIELD", CItemData::ARMOR_SHIELD);
	PyModule_AddIntConstant(poModule, "ARMOR_WRIST", CItemData::ARMOR_WRIST);
	PyModule_AddIntConstant(poModule, "ARMOR_FOOTS", CItemData::ARMOR_FOOTS);
	PyModule_AddIntConstant(poModule, "ARMOR_NECK", CItemData::ARMOR_NECK);
	PyModule_AddIntConstant(poModule, "ARMOR_EAR", CItemData::ARMOR_EAR);
	PyModule_AddIntConstant(poModule, "ARMOR_PENDANT", CItemData::ARMOR_PENDANT);

	PyModule_AddIntConstant(poModule, "ITEM_APPLY_MAX_NUM", CItemData::ITEM_APPLY_MAX_NUM);
	PyModule_AddIntConstant(poModule, "ITEM_SOCKET_MAX_NUM", CItemData::ITEM_SOCKET_MAX_NUM);

	PyModule_AddIntConstant(poModule, "APPLY_NONE", CItemData::APPLY_NONE);
	PyModule_AddIntConstant(poModule, "APPLY_STR", CItemData::APPLY_STR);
	PyModule_AddIntConstant(poModule, "APPLY_DEX", CItemData::APPLY_DEX);
	PyModule_AddIntConstant(poModule, "APPLY_CON", CItemData::APPLY_CON);
	PyModule_AddIntConstant(poModule, "APPLY_INT", CItemData::APPLY_INT);
	PyModule_AddIntConstant(poModule, "APPLY_MAX_HP", CItemData::APPLY_MAX_HP);
	PyModule_AddIntConstant(poModule, "APPLY_MAX_SP", CItemData::APPLY_MAX_SP);
	PyModule_AddIntConstant(poModule, "APPLY_HP_REGEN", CItemData::APPLY_HP_REGEN);
	PyModule_AddIntConstant(poModule, "APPLY_SP_REGEN", CItemData::APPLY_SP_REGEN);
	PyModule_AddIntConstant(poModule, "APPLY_DEF_GRADE_BONUS", CItemData::APPLY_DEF_GRADE_BONUS);
	PyModule_AddIntConstant(poModule, "APPLY_ATT_GRADE_BONUS", CItemData::APPLY_ATT_GRADE_BONUS);
	PyModule_AddIntConstant(poModule, "APPLY_ATT_SPEED", CItemData::APPLY_ATT_SPEED);
	PyModule_AddIntConstant(poModule, "APPLY_MOV_SPEED", CItemData::APPLY_MOV_SPEED);
	PyModule_AddIntConstant(poModule, "APPLY_CAST_SPEED", CItemData::APPLY_CAST_SPEED);
	PyModule_AddIntConstant(poModule, "APPLY_MAGIC_ATT_GRADE", CItemData::APPLY_MAGIC_ATT_GRADE);
	PyModule_AddIntConstant(poModule, "APPLY_MAGIC_DEF_GRADE", CItemData::APPLY_MAGIC_DEF_GRADE);
	PyModule_AddIntConstant(poModule, "APPLY_SKILL", CItemData::APPLY_SKILL);
	PyModule_AddIntConstant(poModule, "APPLY_ATTBONUS_ANIMAL", CItemData::APPLY_ATTBONUS_ANIMAL);
	PyModule_AddIntConstant(poModule, "APPLY_ATTBONUS_UNDEAD", CItemData::APPLY_ATTBONUS_UNDEAD);
	PyModule_AddIntConstant(poModule, "APPLY_ATTBONUS_DEVIL", CItemData::APPLY_ATTBONUS_DEVIL);
	PyModule_AddIntConstant(poModule, "APPLY_ATTBONUS_HUMAN", CItemData::APPLY_ATTBONUS_HUMAN);
	PyModule_AddIntConstant(poModule, "APPLY_BOW_DISTANCE", CItemData::APPLY_BOW_DISTANCE);
	PyModule_AddIntConstant(poModule, "APPLY_RESIST_BOW", CItemData::APPLY_RESIST_BOW);
	PyModule_AddIntConstant(poModule, "APPLY_RESIST_FIRE", CItemData::APPLY_RESIST_FIRE);
	PyModule_AddIntConstant(poModule, "APPLY_RESIST_ELEC", CItemData::APPLY_RESIST_ELEC);
	PyModule_AddIntConstant(poModule, "APPLY_RESIST_MAGIC", CItemData::APPLY_RESIST_MAGIC);
	PyModule_AddIntConstant(poModule, "APPLY_POISON_PCT", CItemData::APPLY_POISON_PCT);
	PyModule_AddIntConstant(poModule, "APPLY_SLOW_PCT", CItemData::APPLY_SLOW_PCT);
	PyModule_AddIntConstant(poModule, "APPLY_STUN_PCT", CItemData::APPLY_STUN_PCT);
	PyModule_AddIntConstant(poModule, "APPLY_CRITICAL_PCT", CItemData::APPLY_CRITICAL_PCT);
	PyModule_AddIntConstant(poModule, "APPLY_PENETRATE_PCT", CItemData::APPLY_PENETRATE_PCT);
	PyModule_AddIntConstant(poModule, "APPLY_ATTBONUS_ORC", CItemData::APPLY_ATTBONUS_ORC);
	PyModule_AddIntConstant(poModule, "APPLY_ATTBONUS_MILGYO", CItemData::APPLY_ATTBONUS_MILGYO);
	PyModule_AddIntConstant(poModule, "APPLY_ATTBONUS_UNDEAD", CItemData::APPLY_ATTBONUS_UNDEAD);
	PyModule_AddIntConstant(poModule, "APPLY_ATTBONUS_DEVIL", CItemData::APPLY_ATTBONUS_DEVIL);
	PyModule_AddIntConstant(poModule, "APPLY_STEAL_HP", CItemData::APPLY_STEAL_HP);
	PyModule_AddIntConstant(poModule, "APPLY_STEAL_SP", CItemData::APPLY_STEAL_SP);
	PyModule_AddIntConstant(poModule, "APPLY_MANA_BURN_PCT", CItemData::APPLY_MANA_BURN_PCT);
	PyModule_AddIntConstant(poModule, "APPLY_DAMAGE_SP_RECOVER", CItemData::APPLY_DAMAGE_SP_RECOVER);
	PyModule_AddIntConstant(poModule, "APPLY_BLOCK", CItemData::APPLY_BLOCK);
	PyModule_AddIntConstant(poModule, "APPLY_DODGE", CItemData::APPLY_DODGE);
	PyModule_AddIntConstant(poModule, "APPLY_RESIST_SWORD", CItemData::APPLY_RESIST_SWORD);
	PyModule_AddIntConstant(poModule, "APPLY_RESIST_TWOHAND", CItemData::APPLY_RESIST_TWOHAND);
	PyModule_AddIntConstant(poModule, "APPLY_RESIST_DAGGER", CItemData::APPLY_RESIST_DAGGER);
	PyModule_AddIntConstant(poModule, "APPLY_RESIST_BELL", CItemData::APPLY_RESIST_BELL);
	PyModule_AddIntConstant(poModule, "APPLY_RESIST_FAN", CItemData::APPLY_RESIST_FAN);
	PyModule_AddIntConstant(poModule, "APPLY_RESIST_WIND", CItemData::APPLY_RESIST_WIND);
	PyModule_AddIntConstant(poModule, "APPLY_REFLECT_MELEE", CItemData::APPLY_REFLECT_MELEE);
	PyModule_AddIntConstant(poModule, "APPLY_REFLECT_CURSE", CItemData::APPLY_REFLECT_CURSE);
	PyModule_AddIntConstant(poModule, "APPLY_POISON_REDUCE", CItemData::APPLY_POISON_REDUCE);
	PyModule_AddIntConstant(poModule, "APPLY_KILL_SP_RECOVER", CItemData::APPLY_KILL_SP_RECOVER);
	PyModule_AddIntConstant(poModule, "APPLY_EXP_DOUBLE_BONUS", CItemData::APPLY_EXP_DOUBLE_BONUS);
	PyModule_AddIntConstant(poModule, "APPLY_GOLD_DOUBLE_BONUS", CItemData::APPLY_GOLD_DOUBLE_BONUS);
	PyModule_AddIntConstant(poModule, "APPLY_ITEM_DROP_BONUS", CItemData::APPLY_ITEM_DROP_BONUS);
	PyModule_AddIntConstant(poModule, "APPLY_POTION_BONUS", CItemData::APPLY_POTION_BONUS);
	PyModule_AddIntConstant(poModule, "APPLY_KILL_HP_RECOVER", CItemData::APPLY_KILL_HP_RECOVER);
	PyModule_AddIntConstant(poModule, "APPLY_IMMUNE_STUN", CItemData::APPLY_IMMUNE_STUN);
	PyModule_AddIntConstant(poModule, "APPLY_IMMUNE_SLOW", CItemData::APPLY_IMMUNE_SLOW);
	PyModule_AddIntConstant(poModule, "APPLY_IMMUNE_FALL", CItemData::APPLY_IMMUNE_FALL);
	PyModule_AddIntConstant(poModule, "APPLY_MAX_STAMINA", CItemData::APPLY_MAX_STAMINA);
	PyModule_AddIntConstant(poModule, "APPLY_ATTBONUS_WARRIOR", CItemData::APPLY_ATT_BONUS_TO_WARRIOR);
	PyModule_AddIntConstant(poModule, "APPLY_ATTBONUS_ASSASSIN", CItemData::APPLY_ATT_BONUS_TO_ASSASSIN);
	PyModule_AddIntConstant(poModule, "APPLY_ATTBONUS_SURA", CItemData::APPLY_ATT_BONUS_TO_SURA);
	PyModule_AddIntConstant(poModule, "APPLY_ATTBONUS_SHAMAN", CItemData::APPLY_ATT_BONUS_TO_SHAMAN);
	PyModule_AddIntConstant(poModule, "APPLY_ATTBONUS_MONSTER", CItemData::APPLY_ATT_BONUS_TO_MONSTER);
	PyModule_AddIntConstant(poModule, "APPLY_MALL_ATTBONUS", CItemData::APPLY_MALL_ATTBONUS);
	PyModule_AddIntConstant(poModule, "APPLY_MALL_DEFBONUS", CItemData::APPLY_MALL_DEFBONUS);
	PyModule_AddIntConstant(poModule, "APPLY_MALL_EXPBONUS", CItemData::APPLY_MALL_EXPBONUS);
	PyModule_AddIntConstant(poModule, "APPLY_MALL_ITEMBONUS", CItemData::APPLY_MALL_ITEMBONUS);
	PyModule_AddIntConstant(poModule, "APPLY_MALL_GOLDBONUS", CItemData::APPLY_MALL_GOLDBONUS);
	PyModule_AddIntConstant(poModule, "APPLY_MAX_HP_PCT", CItemData::APPLY_MAX_HP_PCT);
	PyModule_AddIntConstant(poModule, "APPLY_MAX_SP_PCT", CItemData::APPLY_MAX_SP_PCT);
	PyModule_AddIntConstant(poModule, "APPLY_SKILL_DAMAGE_BONUS", CItemData::APPLY_SKILL_DAMAGE_BONUS);
	PyModule_AddIntConstant(poModule, "APPLY_NORMAL_HIT_DAMAGE_BONUS", CItemData::APPLY_NORMAL_HIT_DAMAGE_BONUS);
	PyModule_AddIntConstant(poModule, "APPLY_SKILL_DEFEND_BONUS", CItemData::APPLY_SKILL_DEFEND_BONUS);
	PyModule_AddIntConstant(poModule, "APPLY_NORMAL_HIT_DEFEND_BONUS", CItemData::APPLY_NORMAL_HIT_DEFEND_BONUS);

	PyModule_AddIntConstant(poModule, "APPLY_UNUSED_EXP_BONUS", CItemData::APPLY_UNUSED_EXP_BONUS);
	PyModule_AddIntConstant(poModule, "APPLY_UNUSED_DROP_BONUS", CItemData::APPLY_UNUSED_DROP_BONUS);

	PyModule_AddIntConstant(poModule, "APPLY_RESIST_WARRIOR", CItemData::APPLY_RESIST_WARRIOR);
	PyModule_AddIntConstant(poModule, "APPLY_RESIST_ASSASSIN", CItemData::APPLY_RESIST_ASSASSIN);
	PyModule_AddIntConstant(poModule, "APPLY_RESIST_SURA", CItemData::APPLY_RESIST_SURA);
	PyModule_AddIntConstant(poModule, "APPLY_RESIST_SHAMAN", CItemData::APPLY_RESIST_SHAMAN);
	PyModule_AddIntConstant(poModule, "APPLY_ENERGY", CItemData::APPLY_ENERGY);
	PyModule_AddIntConstant(poModule, "APPLY_COSTUME_ATTR_BONUS", CItemData::APPLY_COSTUME_ATTR_BONUS);

	PyModule_AddIntConstant(poModule, "APPLY_MAGIC_ATTBONUS_PER", CItemData::APPLY_MAGIC_ATTBONUS_PER);
	PyModule_AddIntConstant(poModule, "APPLY_MELEE_MAGIC_ATTBONUS_PER", CItemData::APPLY_MELEE_MAGIC_ATTBONUS_PER);
	PyModule_AddIntConstant(poModule, "APPLY_RESIST_ICE", CItemData::APPLY_RESIST_ICE);
	PyModule_AddIntConstant(poModule, "APPLY_RESIST_EARTH", CItemData::APPLY_RESIST_EARTH);
	PyModule_AddIntConstant(poModule, "APPLY_RESIST_DARK", CItemData::APPLY_RESIST_DARK);
	PyModule_AddIntConstant(poModule, "APPLY_ANTI_CRITICAL_PCT", CItemData::APPLY_ANTI_CRITICAL_PCT);
	PyModule_AddIntConstant(poModule, "APPLY_ANTI_PENETRATE_PCT", CItemData::APPLY_ANTI_PENETRATE_PCT);
#ifdef ENABLE_WOLFMAN_CHARACTER
	PyModule_AddIntConstant(poModule, "APPLY_BLEEDING_PCT", CItemData::APPLY_BLEEDING_PCT);
	PyModule_AddIntConstant(poModule, "APPLY_BLEEDING_REDUCE", CItemData::APPLY_BLEEDING_REDUCE);
	PyModule_AddIntConstant(poModule, "APPLY_ATTBONUS_WOLFMAN", CItemData::APPLY_ATTBONUS_WOLFMAN);
	PyModule_AddIntConstant(poModule, "APPLY_RESIST_WOLFMAN", CItemData::APPLY_RESIST_WOLFMAN);
	PyModule_AddIntConstant(poModule, "APPLY_RESIST_CLAW", CItemData::APPLY_RESIST_CLAW);
#endif
#ifdef ENABLE_ACCE_SYSTEM
	PyModule_AddIntConstant(poModule, "APPLY_ACCEDRAIN_RATE", CItemData::APPLY_ACCEDRAIN_RATE);
#endif
#ifdef ENABLE_MAGIC_REDUCTION_SYSTEM
	PyModule_AddIntConstant(poModule, "APPLY_RESIST_MAGIC_REDUCTION", CItemData::APPLY_RESIST_MAGIC_REDUCTION);
#endif
#ifdef ENABLE_PENDANT_SYSTEM
	PyModule_AddIntConstant(poModule, "APPLY_ENCHANT_ELECT", CItemData::APPLY_ENCHANT_ELECT);
	PyModule_AddIntConstant(poModule, "APPLY_ENCHANT_FIRE", CItemData::APPLY_ENCHANT_FIRE);
	PyModule_AddIntConstant(poModule, "APPLY_ENCHANT_ICE", CItemData::APPLY_ENCHANT_ICE);
	PyModule_AddIntConstant(poModule, "APPLY_ENCHANT_WIND", CItemData::APPLY_ENCHANT_WIND);
	PyModule_AddIntConstant(poModule, "APPLY_ENCHANT_EARTH", CItemData::APPLY_ENCHANT_EARTH);
	PyModule_AddIntConstant(poModule, "APPLY_ENCHANT_DARK", CItemData::APPLY_ENCHANT_DARK);
	PyModule_AddIntConstant(poModule, "APPLY_RESIST_HUMAN", CItemData::APPLY_RESIST_HUMAN);
	PyModule_AddIntConstant(poModule, "APPLY_ATTBONUS_SWORD", CItemData::APPLY_ATTBONUS_SWORD);
	PyModule_AddIntConstant(poModule, "APPLY_ATTBONUS_TWOHAND", CItemData::APPLY_ATTBONUS_TWOHAND);
	PyModule_AddIntConstant(poModule, "APPLY_ATTBONUS_DAGGER", CItemData::APPLY_ATTBONUS_DAGGER);
	PyModule_AddIntConstant(poModule, "APPLY_ATTBONUS_BELL", CItemData::APPLY_ATTBONUS_BELL);
	PyModule_AddIntConstant(poModule, "APPLY_ATTBONUS_FAN", CItemData::APPLY_ATTBONUS_FAN);
	PyModule_AddIntConstant(poModule, "APPLY_ATTBONUS_BOW", CItemData::APPLY_ATTBONUS_BOW);
	PyModule_AddIntConstant(poModule, "APPLY_ATTBONUS_CZ", CItemData::APPLY_ATTBONUS_CZ);
	PyModule_AddIntConstant(poModule, "APPLY_ATTBONUS_DESERT", CItemData::APPLY_ATTBONUS_DESERT);
	PyModule_AddIntConstant(poModule, "APPLY_ATTBONUS_INSECT", CItemData::APPLY_ATTBONUS_INSECT);
	PyModule_AddIntConstant(poModule, "APPLY_ATTBONUS_CLAW", CItemData::APPLY_ATTBONUS_CLAW);
#endif
#ifdef ENABLE_ATTR_ADDONS
	PyModule_AddIntConstant(poModule, "APPLY_ATTBONUS_STONE", CItemData::APPLY_ATTBONUS_STONE);
	PyModule_AddIntConstant(poModule, "APPLY_ATTBONUS_BOSS", CItemData::APPLY_ATTBONUS_BOSS);
	PyModule_AddIntConstant(poModule, "APPLY_ATTBONUS_ELEMENTS", CItemData::APPLY_ATTBONUS_ELEMENTS);
	PyModule_AddIntConstant(poModule, "APPLY_ENCHANT_ELEMENTS", CItemData::APPLY_ENCHANT_ELEMENTS);
	PyModule_AddIntConstant(poModule, "APPLY_ATTBONUS_CHARACTERS", CItemData::APPLY_ATTBONUS_CHARACTERS);
	PyModule_AddIntConstant(poModule, "APPLY_ENCHANT_CHARACTERS", CItemData::APPLY_ENCHANT_CHARACTERS);
#endif
#ifdef ENABLE_CHEST_DROP_POINT
	PyModule_AddIntConstant(poModule, "APPLY_CHEST_BONUS", CItemData::APPLY_CHEST_BONUS);
#endif
#ifdef ENABLE_MOUNT_COSTUME_SYSTEM
	PyModule_AddIntConstant(poModule, "APPLY_MOUNT", CItemData::APPLY_MOUNT);
#endif
#ifdef ENABLE_GROWTH_PET_SYSTEM
	PyModule_AddIntConstant(poModule, "PET_HATCHING_MONEY", PET_HATCHING_MONEY);
	PyModule_AddIntConstant(poModule, "PET_NAME_MIN_SIZE", PET_NAME_MIN_SIZE);
	PyModule_AddIntConstant(poModule, "PET_NAME_MAX_SIZE", PET_NAME_MAX_SIZE);
#endif
#ifdef ENABLE_GROWTH_MOUNT_SYSTEM
	PyModule_AddIntConstant(poModule, "MOUNT_HATCHING_MONEY", MOUNT_HATCHING_MONEY);
	PyModule_AddIntConstant(poModule, "MOUNT_NAME_MIN_SIZE", MOUNT_NAME_MIN_SIZE);
	PyModule_AddIntConstant(poModule, "MOUNT_NAME_MAX_SIZE", MOUNT_NAME_MAX_SIZE);
#endif
#ifdef ENABLE_DSS_RECHARGE_ITEM
	PyModule_AddIntConstant(poModule, "MIN_INFINITE_DURATION", CItemData::MIN_INFINITE_DURATION);
	PyModule_AddIntConstant(poModule, "MAX_INFINITE_DURATION", CItemData::MAX_INFINITE_DURATION);
#endif
#ifdef ENABLE_PENDANT_ATTRIBUTE_SYSTEM
	PyModule_AddIntConstant(poModule, "USE_ADD_PENDANT_ATTRIBUTE", CItemData::USE_ADD_PENDANT_ATTRIBUTE);
	PyModule_AddIntConstant(poModule, "USE_ADD_PENDANT_FIVE_ATTRIBUTE", CItemData::USE_ADD_PENDANT_FIVE_ATTRIBUTE);
	PyModule_AddIntConstant(poModule, "USE_CHANGE_PENDANT_ATTRIBUTE", CItemData::USE_CHANGE_PENDANT_ATTRIBUTE);
#endif
#ifdef ENABLE_AURA_COSTUME_SYSTEM
	PyModule_AddIntConstant(poModule, "USE_ADD_AURA_ATTRIBUTE", CItemData::USE_ADD_AURA_ATTRIBUTE);
	PyModule_AddIntConstant(poModule, "USE_ADD_AURA_FIVE_ATTRIBUTE", CItemData::USE_ADD_AURA_FIVE_ATTRIBUTE);
	PyModule_AddIntConstant(poModule, "USE_CHANGE_AURA_ATTRIBUTE", CItemData::USE_CHANGE_AURA_ATTRIBUTE);
#endif
#ifdef ENABLE_MOUNT_COSTUME_SYSTEM
	PyModule_AddIntConstant(poModule, "USE_ADD_MOUNT_ATTRIBUTE", CItemData::USE_ADD_MOUNT_ATTRIBUTE);
	PyModule_AddIntConstant(poModule, "USE_CHANGE_MOUNT_ATTRIBUTE", CItemData::USE_CHANGE_MOUNT_ATTRIBUTE);
#endif
#ifdef ENABLE_WEAPON_COSTUME_SYSTEM
	PyModule_AddIntConstant(poModule, "USE_ADD_WEAPON_COSTUME_ATTRIBUTE", CItemData::USE_ADD_WEAPON_COSTUME_ATTRIBUTE);
	PyModule_AddIntConstant(poModule, "USE_CHANGE_WEAPON_COSTUME_ATTRIBUTE", CItemData::USE_CHANGE_WEAPON_COSTUME_ATTRIBUTE);
#endif
#ifdef ENABLE_NEW_EQUIPMENT_SYSTEM
	PyModule_AddIntConstant(poModule, "USE_ADD_BELT_ATTRIBUTE", CItemData::USE_ADD_BELT_ATTRIBUTE);
	PyModule_AddIntConstant(poModule, "USE_CHANGE_BELT_ATTRIBUTE", CItemData::USE_CHANGE_BELT_ATTRIBUTE);
#endif
#ifdef ENABLE_SHINING_ITEM_SYSTEM
	PyModule_AddIntConstant(poModule, "ITEM_TYPE_SHINING", CItemData::ITEM_TYPE_SHINING);
	PyModule_AddIntConstant(poModule, "SHINING_SLOT_START", c_Shining_Slot_Start);
	PyModule_AddIntConstant(poModule, "SHINING_SLOT_COUNT", c_Shining_Slot_Count);
	PyModule_AddIntConstant(poModule, "SHINING_WEAPON", CItemData::SHINING_WEAPON);
	PyModule_AddIntConstant(poModule, "SHINING_ARMOR", CItemData::SHINING_ARMOR);
	PyModule_AddIntConstant(poModule, "SHINING_SPECIAL", CItemData::SHINING_SPECIAL);
	PyModule_AddIntConstant(poModule, "SHINING_SPECIAL2", CItemData::SHINING_SPECIAL2);
	PyModule_AddIntConstant(poModule, "SHINING_SPECIAL3", CItemData::SHINING_SPECIAL3);
	PyModule_AddIntConstant(poModule, "SHINING_WING", CItemData::SHINING_WING);
#endif
#ifdef ENABLE_HAIR_COSTUME_ATTRIBUTE
	PyModule_AddIntConstant(poModule, "USE_ADD_HAIR_COSTUME_ATTRIBUTE", CItemData::USE_ADD_HAIR_COSTUME_ATTRIBUTE);
	PyModule_AddIntConstant(poModule, "USE_CHANGE_HAIR_COSTUME_ATTRIBUTE", CItemData::USE_CHANGE_HAIR_COSTUME_ATTRIBUTE);
#endif
#ifdef ENABLE_BODY_COSTUME_ATTRIBUTE
	PyModule_AddIntConstant(poModule, "USE_ADD_BODY_COSTUME_ATTRIBUTE", CItemData::USE_ADD_BODY_COSTUME_ATTRIBUTE);
	PyModule_AddIntConstant(poModule, "USE_CHANGE_BODY_COSTUME_ATTRIBUTE", CItemData::USE_CHANGE_BODY_COSTUME_ATTRIBUTE);
#endif
#ifdef ENABLE_SHINING_ITEM_SYSTEM
	PyModule_AddIntConstant(poModule, "USE_ADD_ACCE_SHINING_ATTRIBUTE", CItemData::USE_ADD_ACCE_SHINING_ATTRIBUTE);
	PyModule_AddIntConstant(poModule, "USE_CHANGE_ACCE_SHINING_ATTRIBUTE", CItemData::USE_CHANGE_ACCE_SHINING_ATTRIBUTE);
	PyModule_AddIntConstant(poModule, "USE_ADD_SOUL_SHINING_ATTRIBUTE", CItemData::USE_ADD_SOUL_SHINING_ATTRIBUTE);
	PyModule_AddIntConstant(poModule, "USE_ADD_SOUL_SHINING_FIVE_ATTRIBUTE", CItemData::USE_ADD_SOUL_SHINING_FIVE_ATTRIBUTE);
	PyModule_AddIntConstant(poModule, "USE_CHANGE_SOUL_SHINING_ATTRIBUTE", CItemData::USE_CHANGE_SOUL_SHINING_ATTRIBUTE);
#endif
#ifdef ENABLE_MOUNT_COSTUME_SYSTEM
	PyModule_AddIntConstant(poModule, "USE_ADD_FIVE_MOUNT_ATTRIBUTE", CItemData::USE_ADD_FIVE_MOUNT_ATTRIBUTE);
#endif
#ifdef ENABLE_WEAPON_COSTUME_SYSTEM
	PyModule_AddIntConstant(poModule, "USE_ADD_FIVE_WEAPON_COSTUME_ATTRIBUTE", CItemData::USE_ADD_FIVE_WEAPON_COSTUME_ATTRIBUTE);
#endif
#ifdef ENABLE_NEW_EQUIPMENT_SYSTEM
	PyModule_AddIntConstant(poModule, "USE_ADD_FIVE_BELT_ATTRIBUTE", CItemData::USE_ADD_FIVE_BELT_ATTRIBUTE);
#endif
#ifdef ENABLE_HAIR_COSTUME_ATTRIBUTE
	PyModule_AddIntConstant(poModule, "USE_ADD_FIVE_HAIR_COSTUME_ATTRIBUTE", CItemData::USE_ADD_FIVE_HAIR_COSTUME_ATTRIBUTE);
#endif
#ifdef ENABLE_BODY_COSTUME_ATTRIBUTE
	PyModule_AddIntConstant(poModule, "USE_ADD_FIVE_BODY_COSTUME_ATTRIBUTE", CItemData::USE_ADD_FIVE_BODY_COSTUME_ATTRIBUTE);
#endif
#ifdef ENABLE_SHINING_ITEM_SYSTEM
	PyModule_AddIntConstant(poModule, "USE_ADD_FIVE_ACCE_SHINING_ATTRIBUTE", CItemData::USE_ADD_FIVE_ACCE_SHINING_ATTRIBUTE);
#endif
#ifdef ENABLE_CAKRA_ITEM_SYSTEM
	PyModule_AddIntConstant(poModule, "ITEM_TYPE_CAKRA", CItemData::ITEM_TYPE_CAKRA);
	PyModule_AddIntConstant(poModule, "CAKRA_SLOT_START", c_Cakra_Slot_Start);
	PyModule_AddIntConstant(poModule, "CAKRA_SLOT_COUNT", c_Cakra_Slot_Count);
	PyModule_AddIntConstant(poModule, "CAKRA_ITEM_1", CItemData::CAKRA_ITEM_1);
	PyModule_AddIntConstant(poModule, "CAKRA_ITEM_2", CItemData::CAKRA_ITEM_2);
	PyModule_AddIntConstant(poModule, "CAKRA_ITEM_3", CItemData::CAKRA_ITEM_3);
	PyModule_AddIntConstant(poModule, "CAKRA_ITEM_4", CItemData::CAKRA_ITEM_4);
	PyModule_AddIntConstant(poModule, "CAKRA_ITEM_5", CItemData::CAKRA_ITEM_5);
	PyModule_AddIntConstant(poModule, "CAKRA_ITEM_6", CItemData::CAKRA_ITEM_6);
	PyModule_AddIntConstant(poModule, "CAKRA_ITEM_7", CItemData::CAKRA_ITEM_7);
	PyModule_AddIntConstant(poModule, "CAKRA_ITEM_8", CItemData::CAKRA_ITEM_8);
	PyModule_AddIntConstant(poModule, "USE_ADD_CAKRA_ITEM_ATTRIBUTE", CItemData::USE_ADD_CAKRA_ITEM_ATTRIBUTE);
	PyModule_AddIntConstant(poModule, "USE_ADD_FIVE_CAKRA_ITEM_ATTRIBUTE", CItemData::USE_ADD_FIVE_CAKRA_ITEM_ATTRIBUTE);
	PyModule_AddIntConstant(poModule, "USE_CHANGE_CAKRA_ITEM_ATTRIBUTE", CItemData::USE_CHANGE_CAKRA_ITEM_ATTRIBUTE);
#endif
#ifdef ENABLE_EXTENDED_PET_ITEM
	PyModule_AddIntConstant(poModule, "USE_ADD_PET_ITEM_ATTRIBUTE", CItemData::USE_ADD_PET_ITEM_ATTRIBUTE);
	PyModule_AddIntConstant(poModule, "USE_ADD_FIVE_PET_ITEM_ATTRIBUTE", CItemData::USE_ADD_FIVE_PET_ITEM_ATTRIBUTE);
	PyModule_AddIntConstant(poModule, "USE_CHANGE_PET_ITEM_ATTRIBUTE", CItemData::USE_CHANGE_PET_ITEM_ATTRIBUTE);
#endif
#ifdef ENABLE_SEBNEM_ITEM_SYSTEM
	PyModule_AddIntConstant(poModule, "ITEM_TYPE_SEBNEM", CItemData::ITEM_TYPE_SEBNEM);
	PyModule_AddIntConstant(poModule, "SEBNEM_SLOT_START", c_Sebnem_Slot_Start);
	PyModule_AddIntConstant(poModule, "SEBNEM_SLOT_COUNT", c_Sebnem_Slot_Count);
	PyModule_AddIntConstant(poModule, "SEBNEM_ITEM_1", CItemData::SEBNEM_ITEM_1);
	PyModule_AddIntConstant(poModule, "SEBNEM_ITEM_2", CItemData::SEBNEM_ITEM_2);
	PyModule_AddIntConstant(poModule, "SEBNEM_ITEM_3", CItemData::SEBNEM_ITEM_3);
	PyModule_AddIntConstant(poModule, "SEBNEM_ITEM_4", CItemData::SEBNEM_ITEM_4);
	PyModule_AddIntConstant(poModule, "SEBNEM_ITEM_5", CItemData::SEBNEM_ITEM_5);
	PyModule_AddIntConstant(poModule, "SEBNEM_ITEM_6", CItemData::SEBNEM_ITEM_6);
	PyModule_AddIntConstant(poModule, "USE_ADD_SEBNEM_ITEM_ATTRIBUTE", CItemData::USE_ADD_SEBNEM_ITEM_ATTRIBUTE);
	PyModule_AddIntConstant(poModule, "USE_ADD_FIVE_SEBNEM_ITEM_ATTRIBUTE", CItemData::USE_ADD_FIVE_SEBNEM_ITEM_ATTRIBUTE);
	PyModule_AddIntConstant(poModule, "USE_CHANGE_SEBNEM_ITEM_ATTRIBUTE", CItemData::USE_CHANGE_SEBNEM_ITEM_ATTRIBUTE);
#endif
}