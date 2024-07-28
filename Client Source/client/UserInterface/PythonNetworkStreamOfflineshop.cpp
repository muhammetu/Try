#include "StdAfx.h"
#include "PythonNetworkStream.h"
#include "Packet.h"
#include "PythonOfflineshop.h"

#ifdef ENABLE_OFFLINE_SHOP
#define SendObj(obj) (Send(sizeof(obj) , &obj))

bool CPythonNetworkStream::RecvOfflineshopPacket()
{
	TPacketGCNewOfflineshop pack;
	if (!Recv(sizeof(pack), &pack))
		return false;

	switch (pack.bSubHeader)
	{
	case offlineshop::SUBHEADER_GC_SHOP_OPEN:					return RecvOfflineshopShopOpen();
	case offlineshop::SUBHEADER_GC_SHOP_OPEN_OWNER:				return RecvOfflineshopShopOpenOwner();
	case offlineshop::SUBHEADER_GC_SHOP_OPEN_OWNER_NO_SHOP:		return RecvOfflineshopShopOpenOwnerNoShop();
	case offlineshop::SUBHEADER_GC_SHOP_CLOSE:					return RecvOfflineshopShopClose();
	case offlineshop::SUBHEADER_GC_SHOP_BUY_ITEM_FROM_SEARCH:	return RecvOfflineshopShopBuyItemFromSearch();
	case offlineshop::SUBHEADER_GC_SHOP_FILTER_RESULT:			return RecvOfflineshopShopFilterResult();
	case offlineshop::SUBHEADER_GC_SHOP_SAFEBOX_REFRESH:		return RecvOfflineshopShopSafeboxRefresh();

#ifdef ENABLE_OFFLINE_SHOP_CITIES
	case offlineshop::SUBHEADER_GC_INSERT_SHOP_ENTITY:			return RecvOfflineshopInsertEntity();
	case offlineshop::SUBHEADER_GC_REMOVE_SHOP_ENTITY:			return RecvOfflineshopRemoveEntity();
#endif

	case offlineshop::SUBHEADER_GC_SHOP_BUY_RESULT:				return RecvOfflineshopBuyResult();
	case offlineshop::SUBHEADER_GC_SHOP_CREATE_SUCCESS:			return RecvOfflineshopCreateSuccess();
	case offlineshop::SUBHEADER_GC_SHOP_POPUP:					return RecvOfflineshopPopup();

	default:
		TraceError("UNKNOWN OFFLINESHOP SUBHEADER : %d ", pack.bSubHeader);
		return false;
	}
}

bool CPythonNetworkStream::RecvOfflineshopShopOpen()
{
	offlineshop::TSubPacketGCShopOpen subpack;
	if (!Recv(sizeof(subpack), &subpack))
		return false;

	offlineshop::TItemInfo itemInfo;
	std::vector<offlineshop::TItemInfo> items;

	for (DWORD i = 0; i < subpack.shop.dwCount; i++)
	{
		if (!Recv(sizeof(itemInfo), &itemInfo))
			return false;

		items.push_back(itemInfo);
	}

	CPythonOfflineshop::Instance().OpenShop(subpack.shop, items);
	return true;
}

bool CPythonNetworkStream::RecvOfflineshopShopOpenOwner()
{
	offlineshop::TSubPacketGCShopOpenOwner subpack;
	if (!Recv(sizeof(subpack), &subpack))
		return false;

	OFFSHOP_DEBUG("owner id %u , count %u , duration %u , name %s", subpack.shop.dwOwnerID, subpack.shop.dwCount, subpack.shop.dwDuration, subpack.shop.szName);

	//offlineshop::TItemInfo itemInfo;
	std::vector<offlineshop::TItemInfo> items;

	items.resize(subpack.shop.dwCount);

	if (subpack.shop.dwCount != 0)
		if (!Recv(sizeof(offlineshop::TItemInfo) * subpack.shop.dwCount, &items[0]))
			return false;

	OFFSHOP_DEBUG("recv %d item info for sale", subpack.shop.dwCount);

	CPythonOfflineshop::Instance().OpenShopOwner(subpack.shop, items);
	return true;
}

bool CPythonNetworkStream::RecvOfflineshopShopOpenOwnerNoShop()
{
	CPythonOfflineshop::instance().OpenShopOwnerNoShop();
	return true;
}

bool CPythonNetworkStream::RecvOfflineshopShopBuyItemFromSearch()
{
	offlineshop::TSubPacketGCShopBuyItemFromSearch subpack;
	if (!Recv(sizeof(subpack), &subpack))
		return false;

	CPythonOfflineshop::Instance().BuyFromSearch(subpack.dwOwnerID, subpack.dwItemID);
	return true;
}

bool CPythonNetworkStream::RecvOfflineshopShopClose()
{
	CPythonOfflineshop::instance().ShopClose();
	return true;
}

bool CPythonNetworkStream::RecvOfflineshopShopFilterResult()
{
	offlineshop::TSubPacketGCShopFilterResult subpack;
	if (!Recv(sizeof(subpack), &subpack))
		return false;

	offlineshop::TItemInfo itemInfo;
	std::vector<offlineshop::TItemInfo> items;

	for (DWORD i = 0; i < subpack.dwCount; i++)
	{
		if (!Recv(sizeof(itemInfo), &itemInfo))
			return false;

		items.push_back(itemInfo);
	}

	CPythonOfflineshop::Instance().ShopFilterResult(items);
	return true;
}

bool CPythonNetworkStream::RecvOfflineshopShopSafeboxRefresh()
{
	offlineshop::TSubPacketGCShopSafeboxRefresh subpack;
	if (!Recv(sizeof(subpack), &subpack))
		return false;

	std::vector<DWORD> ids;
	std::vector<offlineshop::TItemInfoEx> items;

	ids.resize(subpack.dwItemCount);
	items.resize(subpack.dwItemCount);

	OFFSHOP_DEBUG("item count %u ", subpack.dwItemCount);

	for (DWORD i = 0; i < subpack.dwItemCount; i++)
	{
		if (!Recv(sizeof(DWORD), &ids[i]))
			return false;

		if (!Recv(sizeof(offlineshop::TItemInfoEx), &items[i]))
			return false;
	}

	CPythonOfflineshop::instance().SafeboxRefresh(subpack.valute, ids, items);
	return true;
}

#ifdef ENABLE_OFFLINE_SHOP_CITIES
bool CPythonNetworkStream::RecvOfflineshopInsertEntity()
{
	offlineshop::TSubPacketGCInsertShopEntity subpack;
	if (!Recv(sizeof(subpack), &subpack))
		return false;

	OFFSHOP_DEBUG("inserting shop instance %u %s ", subpack.dwVID, subpack.szName);
	CPythonOfflineshop::Instance().InsertEntity(subpack.dwVID, subpack.iType, subpack.szName, subpack.x, subpack.y, subpack.z, subpack.owner);
	return true;
}

bool CPythonNetworkStream::RecvOfflineshopRemoveEntity()
{
	offlineshop::TSubPacketGCRemoveShopEntity subpack;
	if (!Recv(sizeof(subpack), &subpack))
		return false;

	OFFSHOP_DEBUG("removing shop entity %u ", subpack.dwVID);

	CPythonOfflineshop::Instance().RemoveEntity(subpack.dwVID);
	return true;
}

bool CPythonNetworkStream::RecvOfflineshopBuyResult()
{
	offlineshop::TSubPacketGCShopBuyResult subpack;
	if (!Recv(sizeof(subpack), &subpack))
		return false;

	return true;
}

bool CPythonNetworkStream::RecvOfflineshopCreateSuccess()
{
	CPythonOfflineshop::instance().ShopCreateSuccess();
	return true;
}

bool CPythonNetworkStream::RecvOfflineshopPopup()
{
	offlineshop::TSubPacketGCShopPopup subpack;
	if (!Recv(sizeof(subpack), &subpack))
		return false;

	CPythonOfflineshop::Instance().ShopPopupError(subpack.bWindow, subpack.bErrorHeader);
	return true;
}
void CPythonNetworkStream::SendOfflineshopOnClickShopEntity(DWORD dwPickedShopVID)
{
	TPacketCGNewOfflineShop pack;
	pack.bHeader = HEADER_CG_NEW_OFFLINESHOP;
	pack.bSubHeader = offlineshop::SUBHEADER_CG_CLICK_ENTITY;
	pack.wSize = sizeof(pack) + sizeof(offlineshop::TSubPacketCGShopClickEntity);

	offlineshop::TSubPacketCGShopClickEntity subpack;
	subpack.dwShopVID = dwPickedShopVID;

	if (!SendObj(pack))
	{
		TraceError("CANNOT SEND OFFLINESHOP PACKET : SUBHEADER %d - pack", pack.bSubHeader);
		return;
	}

	if (!SendObj(subpack))
	{
		TraceError("CANNOT SEND OFFLINESHOP PACKET : SUBHEADER %d - subpack ", pack.bSubHeader);
		return;
	}
}
#endif

void CPythonNetworkStream::SendOfflineshopShopCreate(const offlineshop::TShopInfo& shopInfo, const std::vector<offlineshop::TShopItemInfo>& items)
{
	TPacketCGNewOfflineShop pack;
	pack.bHeader = HEADER_CG_NEW_OFFLINESHOP;
	pack.bSubHeader = offlineshop::SUBHEADER_CG_SHOP_CREATE_NEW;
	pack.wSize = (WORD)(sizeof(pack) + sizeof(offlineshop::TSubPacketCGShopCreate) + items.size() * sizeof(offlineshop::TShopItemInfo));

	offlineshop::TSubPacketCGShopCreate subpack;
	offlineshop::CopyObject(subpack.shop, shopInfo);
	subpack.shop.dwCount = items.size();

	if (!SendObj(pack))
	{
		TraceError("CANNOT SEND OFFLINESHOP PACKET : SUBHEADER %d - pack", pack.bSubHeader);
		return;
	}

	if (!SendObj(subpack))
	{
		TraceError("CANNOT SEND OFFLINESHOP PACKET : SUBHEADER %d - subpack ", pack.bSubHeader);
		return;
	}

	for (DWORD i = 0; i < items.size(); i++)
	{
		const offlineshop::TShopItemInfo& info = items[i];
		if (!SendObj(info))
		{
			TraceError("CANNOT SEND OFFLINESHOP PACKET : SUBHEADER %d - items forloop", pack.bSubHeader);
			return;
		}
	}
}

void CPythonNetworkStream::SendOfflineshopForceCloseShop()
{
	TPacketCGNewOfflineShop pack;
	pack.bHeader = HEADER_CG_NEW_OFFLINESHOP;
	pack.bSubHeader = offlineshop::SUBHEADER_CG_SHOP_FORCE_CLOSE;
	pack.wSize = sizeof(pack);

	if (!SendObj(pack))
	{
		TraceError("CANNOT SEND OFFLINESHOP PACKET : SUBHEADER %d - pack", pack.bSubHeader);
		return;
	}
}

void CPythonNetworkStream::SendOfflineshopOpenShop(DWORD dwOwnerID)
{
	TPacketCGNewOfflineShop pack;
	pack.bHeader = HEADER_CG_NEW_OFFLINESHOP;
	pack.bSubHeader = offlineshop::SUBHEADER_CG_SHOP_OPEN;
	pack.wSize = sizeof(pack) + sizeof(offlineshop::TSubPacketCGShopOpen);

	offlineshop::TSubPacketCGShopOpen subpack;
	subpack.dwOwnerID = dwOwnerID;

	OFFSHOP_DEBUG("shop id %u ", dwOwnerID);

	if (!SendObj(pack))
	{
		TraceError("CANNOT SEND OFFLINESHOP PACKET : SUBHEADER %d - pack", pack.bSubHeader);
		return;
	}

	if (!SendObj(subpack))
	{
		TraceError("CANNOT SEND OFFLINESHOP PACKET : SUBHEADER %d - subpack ", pack.bSubHeader);
		return;
	}
}

void CPythonNetworkStream::SendOfflineshopOpenShopOwner()
{
	TPacketCGNewOfflineShop pack;
	pack.bHeader = HEADER_CG_NEW_OFFLINESHOP;
	pack.bSubHeader = offlineshop::SUBHEADER_CG_SHOP_OPEN_OWNER;
	pack.wSize = sizeof(pack);

	if (!SendObj(pack))
	{
		TraceError("CANNOT SEND OFFLINESHOP PACKET : SUBHEADER %d - pack", pack.bSubHeader);
		return;
	}
}

void CPythonNetworkStream::SendOfflineshopBuyItem(DWORD dwOwnerID, DWORD dwItemID, bool isSearch, const offlineshop::TPriceInfo& price) //fix-edit-price
{
	TPacketCGNewOfflineShop pack;
	pack.bHeader = HEADER_CG_NEW_OFFLINESHOP;
	pack.bSubHeader = offlineshop::SUBHEADER_CG_SHOP_BUY_ITEM;
	pack.wSize = sizeof(pack) + sizeof(offlineshop::TSubPacketCGShopBuyItem);

	offlineshop::TSubPacketCGShopBuyItem subpack;
	subpack.dwItemID = dwItemID;
	subpack.dwOwnerID = dwOwnerID;
	subpack.bIsSearch = isSearch;
	subpack.Price = price;

	OFFSHOP_DEBUG("shop owner %u , item id %u ", dwOwnerID, dwItemID);

	if (!SendObj(pack))
	{
		TraceError("CANNOT SEND OFFLINESHOP PACKET : SUBHEADER %d - pack", pack.bSubHeader);
		return;
	}

	if (!SendObj(subpack))
	{
		TraceError("CANNOT SEND OFFLINESHOP PACKET : SUBHEADER %d - subpack ", pack.bSubHeader);
		return;
	}
}

void CPythonNetworkStream::SendOfflineshopAddItem(offlineshop::TShopItemInfo& itemInfo)
{
	TPacketCGNewOfflineShop pack;
	pack.bHeader = HEADER_CG_NEW_OFFLINESHOP;
	pack.bSubHeader = offlineshop::SUBHEADER_CG_SHOP_ADD_ITEM;
	pack.wSize = sizeof(pack) + sizeof(offlineshop::TSubPacketCGAddItem);

	offlineshop::TSubPacketCGAddItem subpack;
	offlineshop::CopyObject(subpack.pos, itemInfo.pos);
	offlineshop::CopyObject(subpack.price, itemInfo.price);

	if (!SendObj(pack))
	{
		TraceError("CANNOT SEND OFFLINESHOP PACKET : SUBHEADER %d - pack", pack.bSubHeader);
		return;
	}

	if (!SendObj(subpack))
	{
		TraceError("CANNOT SEND OFFLINESHOP PACKET : SUBHEADER %d - subpack ", pack.bSubHeader);
		return;
	}
}

void CPythonNetworkStream::SendOfflineshopRemoveItem(DWORD dwItemID)
{
	TPacketCGNewOfflineShop pack;
	pack.bHeader = HEADER_CG_NEW_OFFLINESHOP;
	pack.bSubHeader = offlineshop::SUBHEADER_CG_SHOP_REMOVE_ITEM;
	pack.wSize = sizeof(pack) + sizeof(offlineshop::TSubPacketCGRemoveItem);

	offlineshop::TSubPacketCGRemoveItem subpack;
	subpack.dwItemID = dwItemID;

	OFFSHOP_DEBUG("sending remove item %u ", dwItemID);

	if (!SendObj(pack))
	{
		TraceError("CANNOT SEND OFFLINESHOP PACKET : SUBHEADER %d - pack", pack.bSubHeader);
		return;
	}

	if (!SendObj(subpack))
	{
		TraceError("CANNOT SEND OFFLINESHOP PACKET : SUBHEADER %d - subpack ", pack.bSubHeader);
		return;
	}
}

void CPythonNetworkStream::SendOfflineshopFilterRequest(const offlineshop::TFilterInfo& filter)
{
	TPacketCGNewOfflineShop pack;
	pack.bHeader = HEADER_CG_NEW_OFFLINESHOP;
	pack.bSubHeader = offlineshop::SUBHEADER_CG_SHOP_FILTER_REQUEST;
	pack.wSize = sizeof(pack) + sizeof(offlineshop::TSubPacketCGFilterRequest);

	offlineshop::TSubPacketCGFilterRequest subpack;
	offlineshop::CopyObject(subpack.filter, filter);

	if (!SendObj(pack))
	{
		TraceError("CANNOT SEND OFFLINESHOP PACKET : SUBHEADER %d - pack", pack.bSubHeader);
		return;
	}

	if (!SendObj(subpack))
	{
		TraceError("CANNOT SEND OFFLINESHOP PACKET : SUBHEADER %d - subpack ", pack.bSubHeader);
		return;
	}
}

void CPythonNetworkStream::SendOfflineshopSafeboxOpen()
{
	TPacketCGNewOfflineShop pack;
	pack.bHeader = HEADER_CG_NEW_OFFLINESHOP;
	pack.bSubHeader = offlineshop::SUBHEADER_CG_SHOP_SAFEBOX_OPEN;
	pack.wSize = sizeof(pack);

	if (!SendObj(pack))
	{
		TraceError("CANNOT SEND OFFLINESHOP PACKET : SUBHEADER %d - pack", pack.bSubHeader);
		return;
	}
}

void CPythonNetworkStream::SendOfflineshopSafeboxGetItem(DWORD dwItemID)
{
	TPacketCGNewOfflineShop pack;
	pack.bHeader = HEADER_CG_NEW_OFFLINESHOP;
	pack.bSubHeader = offlineshop::SUBHEADER_CG_SHOP_SAFEBOX_GET_ITEM;

	offlineshop::TSubPacketCGShopSafeboxGetItem subpack;
	subpack.dwItemID = dwItemID;

	if (!SendObj(pack))
	{
		TraceError("CANNOT SEND OFFLINESHOP PACKET : SUBHEADER %d - pack", pack.bSubHeader);
		return;
	}

	if (!SendObj(subpack))
	{
		TraceError("CANNOT SEND OFFLINESHOP PACKET : SUBHEADER %d - subpack ", pack.bSubHeader);
		return;
	}
}

void CPythonNetworkStream::SendOfflineshopSafeboxGetValutes(const offlineshop::TValutesInfo& valutes)
{
	TPacketCGNewOfflineShop pack;
	pack.bHeader = HEADER_CG_NEW_OFFLINESHOP;
	pack.bSubHeader = offlineshop::SUBHEADER_CG_SHOP_SAFEBOX_GET_VALUTES;
	pack.wSize = sizeof(pack) + sizeof(offlineshop::TSubPacketCGShopSafeboxGetValutes);

	offlineshop::TSubPacketCGShopSafeboxGetValutes subpack;
	offlineshop::CopyObject(subpack.valutes, valutes);

	if (!SendObj(pack))
	{
		TraceError("CANNOT SEND OFFLINESHOP PACKET : SUBHEADER %d - pack", pack.bSubHeader);
		return;
	}

	if (!SendObj(subpack))
	{
		TraceError("CANNOT SEND OFFLINESHOP PACKET : SUBHEADER %d - subpack ", pack.bSubHeader);
		return;
	}
}

void CPythonNetworkStream::SendOfflineshopSafeboxClose()
{
	TPacketCGNewOfflineShop pack;
	pack.bHeader = HEADER_CG_NEW_OFFLINESHOP;
	pack.bSubHeader = offlineshop::SUBHEADER_CG_SHOP_SAFEBOX_CLOSE;
	pack.wSize = sizeof(pack);

	if (!SendObj(pack))
	{
		TraceError("CANNOT SEND OFFLINESHOP PACKET : SUBHEADER %d - pack", pack.bSubHeader);
		return;
	}
}

void CPythonNetworkStream::SendOfflineshopCloseBoard()
{
	if (m_strPhase != "Game")
		return;

	TPacketCGNewOfflineShop pack;
	pack.bHeader = HEADER_CG_NEW_OFFLINESHOP;
	pack.bSubHeader = offlineshop::SUBHEADER_CG_CLOSE_BOARD;
	pack.wSize = sizeof(pack);

	if (!SendObj(pack))
	{
		TraceError("CANNOT SEND OFFLINESHOP PACKET : SUBHEADER %d - pack", pack.bSubHeader);
		return;
	}
}

void CPythonNetworkStream::SendOfflineShopTeleport()
{
	if (m_strPhase != "Game")
		return;

	TPacketCGNewOfflineShop pack;
	pack.bHeader = HEADER_CG_NEW_OFFLINESHOP;
	pack.bSubHeader = offlineshop::SUBHEADER_CG_TELEPORT;
	pack.wSize = sizeof(pack);

	if (!SendObj(pack))
	{
		TraceError("CANNOT SEND OFFLINESHOP PACKET : SUBHEADER %d - pack", pack.bSubHeader);
		return;
	}
}

#endif