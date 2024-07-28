#include "StdAfx.h"
#ifdef ENABLE_OFFLINE_SHOP
#include <fstream>
#include "../GameLib/ItemManager.h"
#include "../EterLib/Camera.h"

#include "Packet.h"
#include "PythonNetworkStream.h"
#include "PythonTextTail.h"
#include "PythonSystem.h"
#include "PythonOfflineshop.h"

#define ApplyPyMethod(str, ...)		if(m_poWindow)PyCallClassMemberFunc(m_poWindow, str, __VA_ARGS__); else TraceError("OFFLINESHOP:: CANNOT CALL CLASS MEMBER %s",str);
#define ApplyPyMethodView(str, ...)		if(m_poWindowView)PyCallClassMemberFunc(m_poWindowView, str, __VA_ARGS__); else TraceError("OFFLINESHOP:: CANNOT CALL CLASS MEMBER %s",str);
#define ApplyPyMethodSearch(str, ...)		if(m_poWindowSearch)PyCallClassMemberFunc(m_poWindowSearch, str, __VA_ARGS__); else TraceError("OFFLINESHOP:: CANNOT CALL CLASS MEMBER %s",str);
#define ApplyPyMethodBuilder(str, ...)		if(m_poWindowBuilder)PyCallClassMemberFunc(m_poWindowBuilder, str, __VA_ARGS__); else TraceError("OFFLINESHOP:: CANNOT CALL CLASS MEMBER %s",str);

#define GetPyLongLong(tuple,p)		if(!PyTuple_GetLongLong(tuple,		 iArg++,  (p))){	TraceError("%s : cant get %s ",__FUNCTION__, #p);return Py_BadArgument();}
#define GetPyUnsignedLongLong(tuple,p)		if(!PyTuple_GetUnsignedLongLong(tuple,		 iArg++,  (p))){	TraceError("%s : cant get %s ",__FUNCTION__, #p);return Py_BadArgument();}
#define GetPyInteger(tuple,p)		if(!PyTuple_GetInteger(tuple,		 iArg++,  (p))){	TraceError("%s : cant get %s ",__FUNCTION__, #p);return Py_BadArgument();}
#define GetPyUnsignedInteger(tuple,p)		if(!PyTuple_GetUnsignedInteger(tuple,		 iArg++,  (p))){	TraceError("%s : cant get %s ",__FUNCTION__, #p);return Py_BadArgument();}
#define GetPyDWORD(tuple,p)			if(!PyTuple_GetUnsignedLong(tuple,   iArg++,  (p))){	TraceError("%s : cant get %s ",__FUNCTION__, #p);return Py_BadArgument();}
#define GetPyString(tuple,p)		if(!PyTuple_GetString(tuple,		 iArg++,  (p))){	TraceError("%s : cant get %s ",__FUNCTION__, #p);return Py_BadArgument();}
#define GetPyObject(tuple,p)		if(!PyTuple_GetObject(tuple,		 iArg++,  (p))){	TraceError("%s : cant get %s ",__FUNCTION__, #p);return Py_BadArgument();}

#define NOARGS Py_BuildValue("()")

#define PutsError(fmt, ...) TraceError("In function %s line %d : " ##fmt , __FUNCTION__,__LINE__, __VA_ARGS__)
#define UNUSED_VAR(var) var;

CPythonOfflineshop::CPythonOfflineshop()
{
	m_poWindow = nullptr;
	m_poWindowView = nullptr;
	m_poWindowSearch = nullptr;
	m_poWindowBuilder = nullptr;
#ifdef ENABLE_OFFLINE_SHOP_CITIES
	m_bIsShowName = false;
#endif
}

//starts
void CPythonOfflineshop::SetWindowObjectPointer(PyObject* poWindow)
{
	m_poWindow = poWindow;
}

void CPythonOfflineshop::SetWindowObjectViewPointer(PyObject* poWindow)
{
	m_poWindowView = poWindow;
}

void CPythonOfflineshop::SetWindowObjectSearchPointer(PyObject* poWindow)
{
	m_poWindowSearch = poWindow;
}

void CPythonOfflineshop::SetWindowObjectBuilderPointer(PyObject* poWindow)
{
	m_poWindowBuilder = poWindow;
}

PyObject* CPythonOfflineshop::GetOfflineshopBoard()
{
	return m_poWindow;
}

PyObject* CPythonOfflineshop::GetOfflineShopViewBoard()
{
	return m_poWindowView;
}

PyObject* CPythonOfflineshop::GetOfflineShopSearchBoard()
{
	return m_poWindowSearch;
}

PyObject* CPythonOfflineshop::GetOfflineShopBuilderBoard()
{
	return m_poWindowBuilder;
}

void CPythonOfflineshop::ShopCreateSuccess()
{
	ApplyPyMethodBuilder("BINARY_RecvCreateSuccess", Py_BuildValue("()"));
}

void CPythonOfflineshop::ShopPopupError(BYTE bWindow, BYTE bErrorHeader)
{
	switch (bWindow)
	{
		case offlineshop::OFFLINESHOP_WINDOW_BUILDER:
			{
				ApplyPyMethodBuilder("BINARY_RecvPopupDialog", Py_BuildValue("(i)", bErrorHeader));
			}
			break;

		case offlineshop::OFFLINESHOP_WINDOW_EDIT:
			{
				ApplyPyMethod("BINARY_RecvPopupDialog", Py_BuildValue("(i)", bErrorHeader));
			}
			break;

		case offlineshop::OFFLINESHOP_WINDOW_VIEW:
			{
				ApplyPyMethodView("BINARY_RecvPopupDialog", Py_BuildValue("(i)", bErrorHeader));
			}
			break;

		case offlineshop::OFFLINESHOP_WINDOW_SEARCH:
			{
				ApplyPyMethodSearch("BINARY_RecvPopupDialog", Py_BuildValue("(i)", bErrorHeader));
			}
			break;

		default:
#ifndef LIVE_SERVER
			TraceError("Unknown offshop popup subheader window %d", bWindow);
#endif
			break;
	}
}

void CPythonOfflineshop::BuyFromSearch(DWORD dwOwnerID, DWORD dwItemID)
{
	ApplyPyMethodSearch("SearchFilter_BuyFromSearch", Py_BuildValue("(ii)", dwOwnerID, dwItemID));
}

void CPythonOfflineshop::OpenShop(const offlineshop::TShopInfo& shop, const std::vector<offlineshop::TItemInfo>& vec)
{
	ApplyPyMethodView("OpenShop", Py_BuildValue("(iiis)", shop.dwOwnerID, shop.dwDuration, shop.dwCount, shop.szName));

	for (DWORD i = 0; i < vec.size(); i++)
	{
		const offlineshop::TItemInfo& item = vec[i];

		ApplyPyMethodView("OpenShopItem_Alloc", NOARGS);

		{
			ApplyPyMethodView("OpenShopItem_SetValue", Py_BuildValue("(sii)", "id", i, item.dwItemID));
			ApplyPyMethodView("OpenShopItem_SetValue", Py_BuildValue("(sii)", "vnum", i, item.item.dwVnum));
			ApplyPyMethodView("OpenShopItem_SetValue", Py_BuildValue("(sii)", "count", i, item.item.dwCount));
#ifdef ENABLE_CHANGELOOK_SYSTEM
			ApplyPyMethodView("OpenShopItem_SetValue", Py_BuildValue("(sii)", "trans", i, item.item.dwTransmutation));
#endif
#ifdef ENABLE_ITEM_EVOLUTION_SYSTEM
			ApplyPyMethodView("OpenShopItem_SetValue", Py_BuildValue("(sii)", "evolution", i, item.item.dwEvolution));
#endif

			for (int j = 0; j < ITEM_ATTRIBUTE_SLOT_MAX_NUM; j++)
				ApplyPyMethodView("OpenShopItem_SetValue", Py_BuildValue("(siiii)", "attr", i, j, item.item.aAttr[j].bType, item.item.aAttr[j].sValue));

			for (int j = 0; j < ITEM_SOCKET_SLOT_MAX_NUM; j++)
				ApplyPyMethodView("OpenShopItem_SetValue", Py_BuildValue("(siii)", "socket", i, j, item.item.alSockets[j]));

			ApplyPyMethodView("OpenShopItem_SetValue", Py_BuildValue("(siK)", "price", i, item.price.illYang));
#ifdef ENABLE_CHEQUE_SYSTEM
			ApplyPyMethodView("OpenShopItem_SetValue", Py_BuildValue("(sii)", "cheque", i, item.price.iCheque));
#endif
		}
	}

	ApplyPyMethodView("OpenShop_End", NOARGS);
}

void CPythonOfflineshop::OpenShopOwner(
	const offlineshop::TShopInfo& shop,
	const std::vector<offlineshop::TItemInfo>& vec
)
{
	ApplyPyMethod("OpenShopOwner_Start", Py_BuildValue("(iiis)", shop.dwOwnerID, shop.dwDuration, shop.dwCount, shop.szName));

	for (DWORD i = 0; i < vec.size(); i++)
	{
		const offlineshop::TItemInfo& item = vec[i];

		ApplyPyMethod("OpenShopOwnerItem_Alloc", NOARGS);

		{
			ApplyPyMethod("OpenShopOwnerItem_SetValue", Py_BuildValue("(sii)", "id", i, item.dwItemID));
			ApplyPyMethod("OpenShopOwnerItem_SetValue", Py_BuildValue("(sii)", "vnum", i, item.item.dwVnum));
			ApplyPyMethod("OpenShopOwnerItem_SetValue", Py_BuildValue("(sii)", "count", i, item.item.dwCount));
#ifdef ENABLE_CHANGELOOK_SYSTEM
			ApplyPyMethod("OpenShopOwnerItem_SetValue", Py_BuildValue("(sii)", "trans", i, item.item.dwTransmutation));
#endif
#ifdef ENABLE_ITEM_EVOLUTION_SYSTEM
			ApplyPyMethod("OpenShopOwnerItem_SetValue", Py_BuildValue("(sii)", "evolution", i, item.item.dwEvolution));
#endif

			for (int j = 0; j < ITEM_ATTRIBUTE_SLOT_MAX_NUM; j++)
				ApplyPyMethod("OpenShopOwnerItem_SetValue", Py_BuildValue("(siiii)", "attr", i, j, item.item.aAttr[j].bType, item.item.aAttr[j].sValue));

			for (int j = 0; j < ITEM_SOCKET_SLOT_MAX_NUM; j++)
				ApplyPyMethod("OpenShopOwnerItem_SetValue", Py_BuildValue("(siii)", "socket", i, j, item.item.alSockets[j]));

			ApplyPyMethod("OpenShopOwnerItem_SetValue", Py_BuildValue("(siK)", "price", i, item.price.illYang));
#ifdef ENABLE_CHEQUE_SYSTEM
			ApplyPyMethod("OpenShopOwnerItem_SetValue", Py_BuildValue("(sii)", "cheque", i, item.price.iCheque));
#endif
		}

		ApplyPyMethod("OpenShopOwnerItem_Show", NOARGS);
	}

	ApplyPyMethod("OpenShopOwner_End", NOARGS);
}

void CPythonOfflineshop::OpenShopOwnerNoShop()
{
	ApplyPyMethod("Open", Py_BuildValue("(b)", false));
}

void CPythonOfflineshop::ShopClose()
{
	ApplyPyMethod("Close", NOARGS);
}

void CPythonOfflineshop::ShopFilterResult(const std::vector<offlineshop::TItemInfo>& vec)
{
	ApplyPyMethodSearch("BINARY_ShopFilterResult_Begin", Py_BuildValue("(i)", vec.size()));

	for (DWORD i = 0; i < vec.size(); i++)
	{
		const offlineshop::TItemInfo& item = vec[i];

		ApplyPyMethodSearch("BINARY_AppendItemData",
			Py_BuildValue("("
				"iiii"
				"s"
				"K"
				"i"
				"i"
				"i"
				"ii"
				"ii"
				"ii"
				"ii"
				"ii"
				"ii"
				"ii"
				"iiii"
			")",
				item.dwOwnerID, item.dwItemID, item.item.dwVnum, item.item.dwCount,
				item.szOwnerName,
				item.price.illYang,
				item.price.iCheque,
				item.item.dwTransmutation,
				item.item.dwEvolution,
				item.item.aAttr[0].bType,
				item.item.aAttr[1].bType,
				item.item.aAttr[2].bType,
				item.item.aAttr[3].bType,
				item.item.aAttr[4].bType,
				item.item.aAttr[5].bType,
				item.item.aAttr[6].bType,
				item.item.aAttr[0].sValue,
				item.item.aAttr[1].sValue,
				item.item.aAttr[2].sValue,
				item.item.aAttr[3].sValue,
				item.item.aAttr[4].sValue,
				item.item.aAttr[5].sValue,
				item.item.aAttr[6].sValue,
				item.item.alSockets[0], item.item.alSockets[1], item.item.alSockets[2], item.item.alSockets[3]
				)
			);
	}

	ApplyPyMethodSearch("BINARY_ShopFilterResult_End", NOARGS);
}

void CPythonOfflineshop::SafeboxRefresh(const offlineshop::TValutesInfo& valute, const std::vector<DWORD>& ids, const std::vector<offlineshop::TItemInfoEx>& items)
{
	ApplyPyMethod("ShopSafebox_Clear", NOARGS);
#ifdef ENABLE_CHEQUE_SYSTEM
	ApplyPyMethod("ShopSafebox_SetValutes", Py_BuildValue("(Ki)", valute.illYang, valute.iCheque));
#else
	ApplyPyMethod("ShopSafebox_SetValutes", Py_BuildValue("(K)", valute.illYang));
#endif

	for (DWORD i = 0; i < ids.size(); i++)
	{
		const offlineshop::TItemInfoEx& itemInfo = items[i];

		ApplyPyMethod("ShopSafebox_AllocItem", NOARGS);

		ApplyPyMethod("ShopSafebox_SetValue", Py_BuildValue("(si)", "id", ids[i]));
		ApplyPyMethod("ShopSafebox_SetValue", Py_BuildValue("(si)", "vnum", itemInfo.dwVnum));
		ApplyPyMethod("ShopSafebox_SetValue", Py_BuildValue("(si)", "count", itemInfo.dwCount));

#ifdef ENABLE_CHANGELOOK_SYSTEM
		ApplyPyMethod("ShopSafebox_SetValue", Py_BuildValue("(si)", "trans", itemInfo.dwTransmutation));
#endif

#ifdef ENABLE_ITEM_EVOLUTION_SYSTEM
		ApplyPyMethod("ShopSafebox_SetValue", Py_BuildValue("(si)", "evolution", itemInfo.dwEvolution));
#endif

		for (int j = 0; j < ITEM_SOCKET_SLOT_MAX_NUM; j++)
			ApplyPyMethod("ShopSafebox_SetValue", Py_BuildValue("(sii)", "socket", j, itemInfo.alSockets[j]));

		for (int j = 0; j < ITEM_ATTRIBUTE_SLOT_MAX_NUM; j++)
		{
			int iType = itemInfo.aAttr[j].bType;
			int iValue = itemInfo.aAttr[j].sValue;

			ApplyPyMethod("ShopSafebox_SetValue", Py_BuildValue("(sii)", "attr_type", j, iType));
			ApplyPyMethod("ShopSafebox_SetValue", Py_BuildValue("(sii)", "attr_value", j, iValue));
		}
	}

	ApplyPyMethod("ShopSafebox_RefreshEnd", NOARGS);
}

void CPythonOfflineshop::EnableGuiRefreshSymbol()
{
	ApplyPyMethod("BINARY_EnableRefreshSymbol", NOARGS);
}

#ifdef ENABLE_OFFLINE_SHOP_CITIES
void CPythonOfflineshop::InsertEntity(DWORD dwVID, int iType, const char* szName, long x, long y, long z, DWORD owner)
{
	offlineshop::ShopInstance& shop = *(new offlineshop::ShopInstance());
	shop.SetVID(dwVID);
	shop.SetShopType(iType);
	shop.SetSign(szName);
	shop.SetOwner(owner);
	shop.SetNPCGrannyResource();

	CPythonBackground& rkBgMgr = CPythonBackground::Instance();
	rkBgMgr.GlobalPositionToLocalPosition(x, y);

	OFFSHOP_DEBUG(" x %ld , y %ld, z %ld ", x, y, z);
	z = CPythonBackground::Instance().GetHeight(x, y) + 10.0f;
	shop.Show(x, y, z);

	std::string shopname = szName;
	size_t pos = 0;

	if ((pos = shopname.find('@')) != std::string::npos && ++pos != shopname.length())
		shopname = shopname.substr(pos);

	//making shop texttail
	CPythonTextTail::instance().RegisterShopInstanceTextTail(dwVID, shopname.c_str(), shop.GetThingInstancePtr());

	m_vecShopInstance.push_back(&shop);
}

void CPythonOfflineshop::RemoveEntity(DWORD dwVID)
{
	for (auto it = m_vecShopInstance.begin(); it != m_vecShopInstance.end(); it++)
	{
		offlineshop::ShopInstance& shop = *(*it);
		if (shop.GetVID() == dwVID)
		{
			shop.Clear();
			//deleting texttail
			CPythonTextTail::Instance().DeleteShopTextTail(dwVID);
			delete(*it);

			m_vecShopInstance.erase(it);
			return;
		}
	}
}

// @mesafe
constexpr float offlineShopDistance[5] = {25000.0f, 5000.0f, 3500.0f, 2500.0f, 1500.0f};

void CPythonOfflineshop::RenderEntities()
{
	if (CPythonSystem::Instance().IsPrivateShopStatus() == false)
		return;

	// @mesafe
	int shopDistanceLevel = CPythonSystem::Instance().GetPrivateShopLevel();
	const D3DXVECTOR3& c_rv3TargetPosition = CCameraManager::Instance().GetCurrentCamera()->GetTarget();

	for (auto& iter : m_vecShopInstance)
	{
		// @mesafe
		auto shop = iter->GetThingInstancePtr();
		if (shop == nullptr)
			continue;

		const D3DXVECTOR3& shopPosition = shop->GetPosition();
		float fDistance = sqrtf((c_rv3TargetPosition.x - shopPosition.x) * (c_rv3TargetPosition.x - shopPosition.x) + (c_rv3TargetPosition.y - shopPosition.y) * (c_rv3TargetPosition.y - shopPosition.y));

		if (offlineShopDistance[shopDistanceLevel] < fDistance)
			continue;

		iter->Render();
		iter->BlendRender();
	}
}

void CPythonOfflineshop::UpdateEntities()
{
	//OFFSHOP_DEBUG("rendering shops");

	for (auto& iter : m_vecShopInstance)
	{
		iter->Update();
	}
}

bool CPythonOfflineshop::GetShowNameFlag()
{
	return m_bIsShowName;
}

void CPythonOfflineshop::SetShowNameFlag(bool flag)
{
	m_bIsShowName = flag;
}

void CPythonOfflineshop::DeleteEntities()
{
	for (auto& iter : m_vecShopInstance)
	{
		//iter->Clear(); put comment here
		CPythonTextTail::instance().DeleteShopTextTail(iter->GetVID());
		iter->Clear(); //move it here
		delete(iter);
	}

	m_vecShopInstance.clear();
}
#endif

/*
	* Initializing Module
	* Used to send packet via CPythonNetworkStream
	* Converting the pyArg in right argument
*/

PyObject* offlineshopSendShopCreate(PyObject* poSelf, PyObject* poArgs)
{
	//eg		: offlineshop.SendShopCreate(name, duration, items)
	//items		: a tuple which containing iteminfo
	//iteminfo	: a tuple which containing ordered data (windowtype, position, price)

	char* pszName = nullptr;
	DWORD dwDur = 0;
	int iArg = 0;

	PyObject* poTupleItems = nullptr;

	GetPyString(poArgs, &pszName);
	GetPyDWORD(poArgs, &dwDur);
	GetPyObject(poArgs, &poTupleItems);

	offlineshop::TShopInfo shopInfo;
	shopInfo.dwCount = PyTuple_Size(poTupleItems);
	shopInfo.dwDuration = dwDur;
	shopInfo.dwOwnerID = 0;

	strncpy(shopInfo.szName, pszName, sizeof(shopInfo.szName));

	std::vector<offlineshop::TShopItemInfo> vec;
	offlineshop::TShopItemInfo temp;

	vec.reserve(shopInfo.dwCount);

	for (int i = 0; i < shopInfo.dwCount; i++)
	{
		iArg = i;
		PyObject* poItemInfo = nullptr;
		GetPyObject(poTupleItems, &poItemInfo);

		int iWindow = 0, iPos = 0;
		long long illYang = 0;
		iArg = 0;

		GetPyInteger(poItemInfo, &iWindow);
		GetPyInteger(poItemInfo, &iPos);
		GetPyLongLong(poItemInfo, &illYang);
#ifdef ENABLE_CHEQUE_SYSTEM
		GetPyInteger(poItemInfo, &temp.price.iCheque);
#endif

		temp.pos = TItemPos((BYTE)iWindow, iPos);
		temp.price.illYang = illYang;

		bool push = true;
#ifdef ENABLE_CHEQUE_SYSTEM
		if (temp.price.illYang < 0 || temp.price.iCheque < 0)
			push = false;
#else
		if (temp.price.illYang < 0)
			push = false;
#endif

		if (push)
			vec.push_back(temp);
	}

	CPythonNetworkStream::instance().SendOfflineshopShopCreate(shopInfo, vec);
	return Py_BuildNone();
}

PyObject* offlineshopSendForceCloseShop(PyObject* poSelf, PyObject* poArgs)
{
	//eg		: offlineshop.SendShopForceClose()

	CPythonNetworkStream::instance().SendOfflineshopForceCloseShop();
	return Py_BuildNone();
}

PyObject* offlineshopSendOpenShop(PyObject* poSelf, PyObject* poArgs)
{
	//eg		: offlineshop.SendOpenShop(uid)
	int iArg = 0;
	DWORD dwID = 0;

	GetPyDWORD(poArgs, &dwID);

	CPythonNetworkStream::Instance().SendOfflineshopOpenShop(dwID);
	return Py_BuildNone();
}

PyObject* offlineshopSendOpenShopOwner(PyObject* poSelf, PyObject* poArgs)
{
	//eg		: offlineshop.SendOpenShop()
	CPythonNetworkStream::Instance().SendOfflineshopOpenShopOwner();
	return Py_BuildNone();
}

PyObject* offlineshopSendBuyItem(PyObject* poSelf, PyObject* poArgs)
{
	//eg		: offlineshop.SendBuyItem(owner,itemid)
	int iArg = 0;
	DWORD dwOwnerID = 0, dwItemID = 0;

	GetPyDWORD(poArgs, &dwOwnerID);
	GetPyDWORD(poArgs, &dwItemID);

	//fix-edit-price
	offlineshop::TPriceInfo price;
	GetPyLongLong(poArgs, &price.illYang);
#ifdef ENABLE_CHEQUE_SYSTEM
	GetPyInteger(poArgs, &price.iCheque);
#endif

	CPythonNetworkStream::instance().SendOfflineshopBuyItem(dwOwnerID, dwItemID, false, price); //fix-edit-price
	return Py_BuildNone();
}

PyObject* offlineshopSendBuyItemFromSearch(PyObject* poSelf, PyObject* poArgs)
{
	//eg		: offlineshop.SendBuyItem(owner,itemid)
	int iArg = 0;
	DWORD dwOwnerID = 0, dwItemID = 0;

	GetPyDWORD(poArgs, &dwOwnerID);
	GetPyDWORD(poArgs, &dwItemID);
	//fix-edit-price
	offlineshop::TPriceInfo price;
	GetPyLongLong(poArgs, &price.illYang);
#ifdef ENABLE_CHEQUE_SYSTEM
	GetPyInteger(poArgs, &price.iCheque);
#endif

	CPythonNetworkStream::instance().SendOfflineshopBuyItem(dwOwnerID, dwItemID, true, price); //fix-edit-price
	return Py_BuildNone();
}

PyObject* offlineshopSendAddItem(PyObject* poSelf, PyObject* poArgs)
{
	//eg		: offlineshop.SendAddItem(window_type,  slot,  price)

	int iArg = 0, iWindow = 0, iSlot = 0;
	long long illYang = 0;
#ifdef ENABLE_CHEQUE_SYSTEM
	int iCheque = 0;
#endif

	GetPyInteger(poArgs, &iWindow);
	GetPyInteger(poArgs, &iSlot);
	GetPyLongLong(poArgs, &illYang);
#ifdef  ENABLE_CHEQUE_SYSTEM
	GetPyInteger(poArgs, &iCheque);
#endif //  ENABLE_CHEQUE_SYSTEM

	offlineshop::TShopItemInfo info;
	TItemPos temp((BYTE)iWindow, iSlot);

	info.price.illYang = illYang;
#ifdef ENABLE_CHEQUE_SYSTEM
	info.price.iCheque = iCheque;
#endif
	offlineshop::CopyObject(info.pos, temp);

	CPythonNetworkStream::instance().SendOfflineshopAddItem(info);
	return Py_BuildNone();
}

PyObject* offlineshopSendRemoveItem(PyObject* poSelf, PyObject* poArgs)
{
	//eg			: offlineshop.SendRemoveItem(itemid)

	int iArg = 0;
	DWORD dwItemID = 0;
	GetPyDWORD(poArgs, &dwItemID);

	CPythonNetworkStream::instance().SendOfflineshopRemoveItem(dwItemID);
	return Py_BuildNone();
}

PyObject* offlineshopSendFilterRequest(PyObject* poSelf, PyObject* poArgs)
{
	//eg			: offlineshop.SendFilterRequest(itemtype, itemsubtype, itemname, prices, levels, wearflag, attrs)

	//prices		: a tuple which contain the start price valutes and the end price valutes
	//levels		: a tuple which contain the start level and end level
	//attrs			: a tuple which contain ITEM_ATTRIBUTE_SLOT_NORM_NUM tuples (which they contain : (type, value) attribute info)

	offlineshop::TFilterInfo info;

	char* szName = nullptr;

	int iArg = 0;
	GetPyInteger(poArgs, &info.bType);
	GetPyInteger(poArgs, &info.bSubType);
	GetPyInteger(poArgs, &info.wNameMode);
	GetPyString(poArgs, &szName);
	strncpy(info.szName, szName, sizeof(info.szName));
	GetPyUnsignedInteger(poArgs, &info.antiflag);
	GetPyDWORD(poArgs, &info.wCount[0]);
	GetPyDWORD(poArgs, &info.wCount[1]);
	GetPyInteger(poArgs, &info.wDSS[0]);
	GetPyInteger(poArgs, &info.wDSS[1]);
	GetPyInteger(poArgs, &info.wLevel[0]);
	GetPyInteger(poArgs, &info.wLevel[1]);
	GetPyLongLong(poArgs, &info.llYang[0]);
	GetPyLongLong(poArgs, &info.llYang[1]);
	GetPyDWORD(poArgs, &info.wCheque[0]);
	GetPyDWORD(poArgs, &info.wCheque[1]);
	GetPyInteger(poArgs, &info.wAcceAbsorb[0]);
	GetPyInteger(poArgs, &info.wAcceAbsorb[1]);
	GetPyInteger(poArgs, &info.wDSLevel);
	GetPyInteger(poArgs, &info.wDSVal[0]);
	GetPyInteger(poArgs, &info.wDSVal[1]);
	GetPyInteger(poArgs, &info.wDSVal[2]);

	CPythonNetworkStream::instance().SendOfflineshopFilterRequest(info);
	return Py_BuildNone();
}

PyObject* offlineshopSendSafeboxOpen(PyObject* poSelf, PyObject* poArgs)
{
	CPythonNetworkStream::instance().SendOfflineshopSafeboxOpen();
	return Py_BuildNone();
}

PyObject* offlineshopSendSafeboxGetItem(PyObject* poSelf, PyObject* poArgs)
{
	//eg			: offlineshop.SendSafeboxGetItem(itemid)
	int iArg = 0;
	DWORD dwItemID = 0;

	GetPyDWORD(poArgs, &dwItemID);
	CPythonNetworkStream::instance().SendOfflineshopSafeboxGetItem(dwItemID);
	return Py_BuildNone();
}

PyObject* offlineshopSendSafeboxGetValutes(PyObject* poSelf, PyObject* poArgs)
{
	//eg			: offlineshop.SendSafeboxGetValutes(amount)
	int iArg = 0;
	long long illYang = 0;
#ifdef ENABLE_CHEQUE_SYSTEM
	int iCheque = 0;
#endif

	GetPyLongLong(poArgs, &illYang);
#ifdef ENABLE_CHEQUE_SYSTEM
	GetPyInteger(poArgs, &iCheque);
#endif

	offlineshop::TValutesInfo info;
	info.illYang = illYang;
#ifdef ENABLE_CHEQUE_SYSTEM
	info.iCheque = iCheque;
#endif

	CPythonNetworkStream::instance().SendOfflineshopSafeboxGetValutes(info);
	return Py_BuildNone();
}

PyObject* offlineshopSendSafeboxClose(PyObject* poSelf, PyObject* poArgs)
{
	CPythonNetworkStream::instance().SendOfflineshopSafeboxOpen();
	return Py_BuildNone();
}

PyObject* offlineshopSendCloseBoard(PyObject* poSelf, PyObject* poArgs)
{
	CPythonNetworkStream::instance().SendOfflineshopCloseBoard();
	return Py_BuildNone();
}

PyObject* offlineshopSetOfflineshopBoard(PyObject* poSelf, PyObject* poArgs)
{
	PyObject* poWin = nullptr;
	int iArg = 0;

	GetPyObject(poArgs, &poWin);

	CPythonOfflineshop::instance().SetWindowObjectPointer(poWin);
	return Py_BuildNone();
}

PyObject* offlineshopGetOfflineshopBoard(PyObject* poSelf, PyObject* poArgs)
{
	PyObject* poInterface = CPythonOfflineshop::instance().GetOfflineshopBoard();

	if (!poInterface)
		return Py_BuildNone();

	Py_IncRef(poInterface);
	return poInterface;
}

PyObject* offlineshopSetOfflineshopBoardView(PyObject* poSelf, PyObject* poArgs)
{
	PyObject* poWin = nullptr;
	int iArg = 0;

	GetPyObject(poArgs, &poWin);

	CPythonOfflineshop::instance().SetWindowObjectViewPointer(poWin);
	return Py_BuildNone();
}

PyObject* offlineshopGetOfflineshopBoardView(PyObject* poSelf, PyObject* poArgs)
{
	PyObject* poInterface = CPythonOfflineshop::instance().GetOfflineShopViewBoard();

	if (!poInterface)
		return Py_BuildNone();

	Py_IncRef(poInterface);
	return poInterface;
}

PyObject* offlineshopSetOfflineshopBoardSearch(PyObject* poSelf, PyObject* poArgs)
{
	PyObject* poWin = nullptr;
	int iArg = 0;

	GetPyObject(poArgs, &poWin);

	CPythonOfflineshop::instance().SetWindowObjectSearchPointer(poWin);
	return Py_BuildNone();
}

PyObject* offlineshopGetOfflineshopBoardSearch(PyObject* poSelf, PyObject* poArgs)
{
	PyObject* poInterface = CPythonOfflineshop::instance().GetOfflineShopSearchBoard();

	if (!poInterface)
		return Py_BuildNone();

	Py_IncRef(poInterface);
	return poInterface;
}

PyObject* offlineshopSetOfflineshopBoardBuilder(PyObject* poSelf, PyObject* poArgs)
{
	PyObject* poWin = nullptr;
	int iArg = 0;

	GetPyObject(poArgs, &poWin);

	CPythonOfflineshop::instance().SetWindowObjectBuilderPointer(poWin);
	return Py_BuildNone();
}

PyObject* offlineshopGetOfflineshopBoardBuilder(PyObject* poSelf, PyObject* poArgs)
{
	PyObject* poInterface = CPythonOfflineshop::instance().GetOfflineShopBuilderBoard();

	if (!poInterface)
		return Py_BuildNone();

	Py_IncRef(poInterface);
	return poInterface;
}

#ifdef ENABLE_OFFLINE_SHOP_CITIES
PyObject* offlineshopHideShopNames(PyObject* poSelf, PyObject* poArgs)
{
	CPythonOfflineshop::instance().SetShowNameFlag(false);
	return Py_BuildNone();
}

PyObject* offlineshopShowShopNames(PyObject* poSelf, PyObject* poArgs)
{
	CPythonOfflineshop::instance().SetShowNameFlag(true);
	return Py_BuildNone();
}
#endif

PyObject* offlineshopSendTeleport(PyObject* poSelf, PyObject* poArgs)
{
	CPythonNetworkStream::instance().SendOfflineShopTeleport();
	return Py_BuildNone();
}

void initofflineshop()
{
	static PyMethodDef s_methods[] =
	{
		//packets exchanging

		{ "SendShopCreate",				offlineshopSendShopCreate,				METH_VARARGS },
		{ "SendForceCloseShop",			offlineshopSendForceCloseShop,			METH_VARARGS },
		{ "SendOpenShop",				offlineshopSendOpenShop,				METH_VARARGS },
		{ "SendOpenShopOwner",			offlineshopSendOpenShopOwner,			METH_VARARGS },
		{ "SendBuyItem",				offlineshopSendBuyItem,					METH_VARARGS },
		{ "SendBuyItemFromSearch",		offlineshopSendBuyItemFromSearch,		METH_VARARGS },
		{ "SendAddItem",				offlineshopSendAddItem,					METH_VARARGS },
		{ "SendRemoveItem",				offlineshopSendRemoveItem,				METH_VARARGS },
		{ "SendFilterRequest",			offlineshopSendFilterRequest,			METH_VARARGS },
		{ "SendSafeboxOpen",			offlineshopSendSafeboxOpen,				METH_VARARGS },
		{ "SendSafeboxGetItem",			offlineshopSendSafeboxGetItem,			METH_VARARGS },
		{ "SendSafeboxGetValutes",		offlineshopSendSafeboxGetValutes,		METH_VARARGS },
		{ "SendSafeboxClose",			offlineshopSendSafeboxClose,			METH_VARARGS },

		{ "SendCloseBoard",					offlineshopSendCloseBoard,				METH_VARARGS },

		//interfaces methods
		{ "SetOfflineshopBoard",		offlineshopSetOfflineshopBoard,				METH_VARARGS },
		{ "GetOfflineshopBoard",		offlineshopGetOfflineshopBoard,				METH_VARARGS },

		{ "SetOfflineshopBoardView",		offlineshopSetOfflineshopBoardView,				METH_VARARGS },
		{ "GetOfflineshopBoardView",		offlineshopGetOfflineshopBoardView,				METH_VARARGS },

		{ "SetOfflineshopBoardSearch",		offlineshopSetOfflineshopBoardSearch,				METH_VARARGS },
		{ "GetOfflineshopBoardSearch",		offlineshopGetOfflineshopBoardSearch,				METH_VARARGS },

		{ "SetOfflineshopBoardBuilder",		offlineshopSetOfflineshopBoardBuilder,				METH_VARARGS },
		{ "GetOfflineshopBoardBuilder",			offlineshopGetOfflineshopBoardBuilder,				METH_VARARGS},

#ifdef ENABLE_OFFLINE_SHOP_CITIES
		{ "HideShopNames",						offlineshopHideShopNames,						METH_VARARGS },
		{ "ShowShopNames",						offlineshopShowShopNames,						METH_VARARGS },
#endif

		{ "SendTeleport",						offlineshopSendTeleport,						METH_VARARGS },
		{ NULL,									NULL,									NULL		 },
	};

	PyObject* poModule = Py_InitModule("offlineshop", s_methods);

	PyModule_AddIntConstant(poModule, "OFFLINESHOP_ITEM_PAGE_MAX", offlineshop::OFFLINESHOP_ITEM_PAGE_MAX);
	PyModule_AddIntConstant(poModule, "OFFLINESHOP_MAX_ITEM_COUNT", offlineshop::OFFLINESHOP_MAX_ITEM_COUNT);
	PyModule_AddIntConstant(poModule, "OFFLINESHOP_TAB_NAME_MAX", offlineshop::OFFLINESHOP_TAB_NAME_MAX);
	PyModule_AddIntConstant(poModule, "OFFLINESHOP_SEARCH_CATEGORY_MAX", offlineshop::OFFLINESHOP_SEARCH_CATEGORY_MAX);
	PyModule_AddIntConstant(poModule, "OFFLINESHOP_SEARCH_RESULT_PER_PAGE_ITEM_COUNT", offlineshop::OFFLINESHOP_SEARCH_RESULT_PER_PAGE_ITEM_COUNT);
	PyModule_AddIntConstant(poModule, "OFFLINESHOP_SEARCH_TIME", offlineshop::OFFLINESHOP_SEARCH_TIME);

	PyModule_AddIntConstant(poModule, "SUBERROR_GC_SHOP_CHARACTER_ACTIONS", offlineshop::SUBERROR_GC_SHOP_CHARACTER_ACTIONS);
	PyModule_AddIntConstant(poModule, "SUBERROR_GC_SHOP_CHANNEL_LIMIT", offlineshop::SUBERROR_GC_SHOP_CHANNEL_LIMIT);
	PyModule_AddIntConstant(poModule, "SUBERROR_GC_SHOP_CHANNEL_LIMIT_MAP", offlineshop::SUBERROR_GC_SHOP_CHANNEL_LIMIT_MAP);
	PyModule_AddIntConstant(poModule, "SUBERROR_GC_SHOP_DURATION_MIN", offlineshop::SUBERROR_GC_SHOP_DURATION_MIN);
	PyModule_AddIntConstant(poModule, "SUBERROR_GC_SHOP_DURATION_MAX", offlineshop::SUBERROR_GC_SHOP_DURATION_MAX);
	PyModule_AddIntConstant(poModule, "SUBERROR_GC_SHOP_ENOUGH_GOLD", offlineshop::SUBERROR_GC_SHOP_ENOUGH_GOLD);
	PyModule_AddIntConstant(poModule, "SUBERROR_GC_SHOP_ALLOWED_MAP_INDEX", offlineshop::SUBERROR_GC_SHOP_ALLOWED_MAP_INDEX);
	PyModule_AddIntConstant(poModule, "SUBERROR_GC_SHOP_ZERO_GOLD_CHEQUE", offlineshop::SUBERROR_GC_SHOP_ZERO_GOLD_CHEQUE);
	PyModule_AddIntConstant(poModule, "SUBERROR_GC_SHOP_LOCKED_ITEM", offlineshop::SUBERROR_GC_SHOP_LOCKED_ITEM);
#ifdef ENABLE_BEGINNER_ITEM_SYSTEM
	PyModule_AddIntConstant(poModule, "SUBERROR_GC_SHOP_BEGINNER_ITEM", offlineshop::SUBERROR_GC_SHOP_BEGINNER_ITEM);
#endif
	PyModule_AddIntConstant(poModule, "SUBERROR_GC_SHOP_ANTIFLAG_ITEM", offlineshop::SUBERROR_GC_SHOP_ANTIFLAG_ITEM);
	PyModule_AddIntConstant(poModule, "SUBERROR_GC_SHOP_REMOVE_LAST_ITEM", offlineshop::SUBERROR_GC_SHOP_REMOVE_LAST_ITEM);
	PyModule_AddIntConstant(poModule, "SUBERROR_GC_SHOP_REPORT_ADMIN", offlineshop::SUBERROR_GC_SHOP_REPORT_ADMIN);
	PyModule_AddIntConstant(poModule, "SUBERROR_GC_SHOP_CANT_FIND_ITEM", offlineshop::SUBERROR_GC_SHOP_CANT_FIND_ITEM);
	PyModule_AddIntConstant(poModule, "SUBERROR_GC_SHOP_ITEM_LIMIT", offlineshop::SUBERROR_GC_SHOP_ITEM_LIMIT);
	PyModule_AddIntConstant(poModule, "SUBERROR_GC_SHOP_LEVEL_LIMIT", offlineshop::SUBERROR_GC_SHOP_LEVEL_LIMIT);
	PyModule_AddIntConstant(poModule, "SUBERROR_GC_SHOP_DURATION_NON", offlineshop::SUBERROR_GC_SHOP_DURATION_NON);
	PyModule_AddIntConstant(poModule, "SUBERROR_GC_SHOP_GOLD_LIMIT_MAX", offlineshop::SUBERROR_GC_SHOP_GOLD_LIMIT_MAX);
#ifdef ENABLE_CHEQUE_SYSTEM
	PyModule_AddIntConstant(poModule, "SUBERROR_GC_SHOP_CHEQUE_LIMIT_MAX", offlineshop::SUBERROR_GC_SHOP_CHEQUE_LIMIT_MAX);
#endif
	PyModule_AddIntConstant(poModule, "SUBERROR_GC_SHOP_GOLD_LIMIT_MIN", offlineshop::SUBERROR_GC_SHOP_GOLD_LIMIT_MIN);
#ifdef ENABLE_CHEQUE_SYSTEM
	PyModule_AddIntConstant(poModule, "SUBERROR_GC_SHOP_CHEQUE_LIMIT_MIN", offlineshop::SUBERROR_GC_SHOP_CHEQUE_LIMIT_MIN);
#endif
	PyModule_AddIntConstant(poModule, "SUBERROR_GC_SHOP_DUNGEON", offlineshop::SUBERROR_GC_SHOP_DUNGEON);
	PyModule_AddIntConstant(poModule, "SUBERROR_GC_WAR_MAP", offlineshop::SUBERROR_GC_WAR_MAP);
	PyModule_AddIntConstant(poModule, "SUBERROR_GC_SHOP_NAME", offlineshop::SUBERROR_GC_SHOP_NAME);
	PyModule_AddIntConstant(poModule, "SUBERROR_GC_SHOP_BUY_FAIL", offlineshop::SUBERROR_GC_SHOP_BUY_FAIL);
	PyModule_AddIntConstant(poModule, "SUBERROR_GC_SHOP_COUNT_LIMIT", offlineshop::SUBERROR_GC_SHOP_COUNT_LIMIT);
	PyModule_AddIntConstant(poModule, "SUBERROR_GC_SHOP_BUY_SOLD", offlineshop::SUBERROR_GC_SHOP_BUY_SOLD);
	PyModule_AddIntConstant(poModule, "SUBERROR_GC_SHOP_MAP_LIMIT_FIRST", offlineshop::SUBERROR_GC_SHOP_MAP_LIMIT_FIRST);
	PyModule_AddIntConstant(poModule, "SUBERROR_GC_SHOP_MAP_LIMIT_GLOBAL", offlineshop::SUBERROR_GC_SHOP_MAP_LIMIT_GLOBAL);

	PyModule_AddIntConstant(poModule, "OFFLINESHOP_WINDOW_BUILDER", offlineshop::OFFLINESHOP_WINDOW_BUILDER);
	PyModule_AddIntConstant(poModule, "OFFLINESHOP_WINDOW_EDIT", offlineshop::OFFLINESHOP_WINDOW_EDIT);
	PyModule_AddIntConstant(poModule, "OFFLINESHOP_WINDOW_VIEW", offlineshop::OFFLINESHOP_WINDOW_VIEW);
	PyModule_AddIntConstant(poModule, "OFFLINESHOP_WINDOW_SEARCH", offlineshop::OFFLINESHOP_WINDOW_SEARCH);
	PyModule_AddIntConstant(poModule, "OFFLINESHOP_WINDOW_MAX", offlineshop::OFFLINESHOP_WINDOW_MAX);

	PyModule_AddIntConstant(poModule, "OFFLINESHOP_SEARCH_PER_PAGE_RESULT_MAX", offlineshop::OFFLINESHOP_SEARCH_PER_PAGE_RESULT_MAX);

	PyModule_AddIntConstant(poModule, "OFFLINESHOP_CAT_PREMIUM", offlineshop::OFFLINESHOP_CAT_PREMIUM);
	PyModule_AddIntConstant(poModule, "OFFLINESHOP_CAT_WEAPON", offlineshop::OFFLINESHOP_CAT_WEAPON);
	PyModule_AddIntConstant(poModule, "OFFLINESHOP_CAT_ARMOR", offlineshop::OFFLINESHOP_CAT_ARMOR);
	PyModule_AddIntConstant(poModule, "OFFLINESHOP_CAT_ACCESSORY", offlineshop::OFFLINESHOP_CAT_ACCESSORY);
	PyModule_AddIntConstant(poModule, "OFFLINESHOP_CAT_PENDANT", offlineshop::OFFLINESHOP_CAT_PENDANT);
	PyModule_AddIntConstant(poModule, "OFFLINESHOP_CAT_ACCE", offlineshop::OFFLINESHOP_CAT_ACCE);
	PyModule_AddIntConstant(poModule, "OFFLINESHOP_CAT_AURA", offlineshop::OFFLINESHOP_CAT_AURA);
	PyModule_AddIntConstant(poModule, "OFFLINESHOP_CAT_COSTUME", offlineshop::OFFLINESHOP_CAT_COSTUME);
	PyModule_AddIntConstant(poModule, "OFFLINESHOP_CAT_EQUIPABLE", offlineshop::OFFLINESHOP_CAT_EQUIPABLE);
	PyModule_AddIntConstant(poModule, "OFFLINESHOP_CAT_DSS", offlineshop::OFFLINESHOP_CAT_DSS);
	PyModule_AddIntConstant(poModule, "OFFLINESHOP_CAT_PET", offlineshop::OFFLINESHOP_CAT_PET);
	PyModule_AddIntConstant(poModule, "OFFLINESHOP_CAT_BOOK", offlineshop::OFFLINESHOP_CAT_BOOK);
	PyModule_AddIntConstant(poModule, "OFFLINESHOP_CAT_POTION", offlineshop::OFFLINESHOP_CAT_POTION);
	PyModule_AddIntConstant(poModule, "OFFLINESHOP_CAT_UPGRADE", offlineshop::OFFLINESHOP_CAT_UPGRADE);
	PyModule_AddIntConstant(poModule, "OFFLINESHOP_CAT_MAX", offlineshop::OFFLINESHOP_CAT_MAX);

	PyModule_AddIntConstant(poModule, "OFFLINESHOP_SUB_CAT_WEAPON_SWORD", offlineshop::OFFLINESHOP_SUB_CAT_WEAPON_SWORD);
	PyModule_AddIntConstant(poModule, "OFFLINESHOP_SUB_CAT_WEAPON_TWOHAND", offlineshop::OFFLINESHOP_SUB_CAT_WEAPON_TWOHAND);
	PyModule_AddIntConstant(poModule, "OFFLINESHOP_SUB_CAT_WEAPON_BOW", offlineshop::OFFLINESHOP_SUB_CAT_WEAPON_BOW);
	PyModule_AddIntConstant(poModule, "OFFLINESHOP_SUB_CAT_WEAPON_DAGGER", offlineshop::OFFLINESHOP_SUB_CAT_WEAPON_DAGGER);
	PyModule_AddIntConstant(poModule, "OFFLINESHOP_SUB_CAT_WEAPON_BELL", offlineshop::OFFLINESHOP_SUB_CAT_WEAPON_BELL);
	PyModule_AddIntConstant(poModule, "OFFLINESHOP_SUB_CAT_WEAPON_FAN", offlineshop::OFFLINESHOP_SUB_CAT_WEAPON_FAN);
	PyModule_AddIntConstant(poModule, "OFFLINESHOP_SUB_CAT_WEAPON_CLAW", offlineshop::OFFLINESHOP_SUB_CAT_WEAPON_CLAW);
	PyModule_AddIntConstant(poModule, "OFFLINESHOP_SUB_CAT_WEAPON_MAX", offlineshop::OFFLINESHOP_SUB_CAT_WEAPON_MAX);
}

CPythonOfflineshop::~CPythonOfflineshop()
{
	for (auto& iter : m_vecShopInstance)
		delete(iter);

	m_vecShopInstance.clear();
}

#endif