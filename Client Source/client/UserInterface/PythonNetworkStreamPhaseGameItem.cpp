#include "StdAfx.h"
#include "PythonNetworkStream.h"
#include "PythonPlayer.h"
#include "PythonItem.h"
#include "PythonShop.h"
#include "PythonExchange.h"
#include "PythonSafeBox.h"
#include "PythonCharacterManager.h"

#include "AbstractPlayer.h"

//////////////////////////////////////////////////////////////////////////
// SafeBox

bool CPythonNetworkStream::SendSafeBoxCheckinPacket(TItemPos InventoryPos, BYTE bySafeBoxPos)
{
	__PlayInventoryItemDropSound(InventoryPos);

	TPacketCGSafeboxCheckin kSafeboxCheckin;
	kSafeboxCheckin.bHeader = HEADER_CG_SAFEBOX_CHECKIN;
	kSafeboxCheckin.ItemPos = InventoryPos;
	kSafeboxCheckin.bSafePos = bySafeBoxPos;
	if (!Send(sizeof(kSafeboxCheckin), &kSafeboxCheckin))
		return false;

	return true;
}

bool CPythonNetworkStream::SendSafeBoxCheckoutPacket(BYTE bySafeBoxPos, TItemPos InventoryPos)
{
	__PlaySafeBoxItemDropSound(bySafeBoxPos);

	TPacketCGSafeboxCheckout kSafeboxCheckout;
	kSafeboxCheckout.bHeader = HEADER_CG_SAFEBOX_CHECKOUT;
	kSafeboxCheckout.bSafePos = bySafeBoxPos;
	kSafeboxCheckout.ItemPos = InventoryPos;
	if (!Send(sizeof(kSafeboxCheckout), &kSafeboxCheckout))
		return false;

	return true;
}

#ifdef ENABLE_ITEM_COUNT_LIMIT
bool CPythonNetworkStream::SendSafeBoxItemMovePacket(BYTE bySourcePos, BYTE byTargetPos, DWORD byCount)
#else
bool CPythonNetworkStream::SendSafeBoxItemMovePacket(BYTE bySourcePos, BYTE byTargetPos, BYTE byCount)
#endif
{
	__PlaySafeBoxItemDropSound(bySourcePos);

	TPacketCGItemMove kItemMove;
	kItemMove.header = HEADER_CG_SAFEBOX_ITEM_MOVE;
	kItemMove.pos = TItemPos(INVENTORY, bySourcePos);
	kItemMove.num = byCount;
	kItemMove.change_pos = TItemPos(INVENTORY, byTargetPos);
	if (!Send(sizeof(kItemMove), &kItemMove))
		return false;

	return true;
}

bool CPythonNetworkStream::RecvSafeBoxSetPacket()
{
	TPacketGCItemSet kItemSet;
	if (!Recv(sizeof(kItemSet), &kItemSet))
		return false;

	TItemData kItemData;
	kItemData.vnum = kItemSet.vnum;
	kItemData.count = kItemSet.count;
	kItemData.flags = kItemSet.flags;
	kItemData.anti_flags = kItemSet.anti_flags;

	for (int isocket = 0; isocket < ITEM_SOCKET_SLOT_MAX_NUM; ++isocket)
		kItemData.alSockets[isocket] = kItemSet.alSockets[isocket];
	for (int iattr = 0; iattr < ITEM_ATTRIBUTE_SLOT_MAX_NUM; ++iattr)
		kItemData.aAttr[iattr] = kItemSet.aAttr[iattr];
#ifdef ENABLE_BEGINNER_ITEM_SYSTEM
	kItemData.is_basic = false;
#endif
#ifdef ENABLE_ITEM_EVOLUTION_SYSTEM
	kItemData.evolution = kItemSet.evolution;
#endif
#ifdef ENABLE_CHANGELOOK_SYSTEM
	kItemData.transmutation = kItemSet.transmutation;
#endif
	CPythonSafeBox::Instance().SetItemData(kItemSet.Cell.cell, kItemData);

	__RefreshSafeboxWindow();

	return true;
}

bool CPythonNetworkStream::RecvSafeBoxDelPacket()
{
	TPacketGCItemDel kItemDel;
	if (!Recv(sizeof(kItemDel), &kItemDel))
		return false;

	CPythonSafeBox::Instance().DelItemData(kItemDel.pos);

	__RefreshSafeboxWindow();

	return true;
}

bool CPythonNetworkStream::RecvSafeBoxWrongPasswordPacket()
{
	TPacketGCSafeboxWrongPassword kSafeboxWrongPassword;

	if (!Recv(sizeof(kSafeboxWrongPassword), &kSafeboxWrongPassword))
		return false;

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnSafeBoxError", Py_BuildValue("()"));

	return true;
}

bool CPythonNetworkStream::RecvSafeBoxSizePacket()
{
	TPacketGCSafeboxSize kSafeBoxSize;
	if (!Recv(sizeof(kSafeBoxSize), &kSafeBoxSize))
		return false;

	CPythonSafeBox::Instance().OpenSafeBox(kSafeBoxSize.bSize);
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OpenSafeboxWindow", Py_BuildValue("(i)", kSafeBoxSize.bSize));

	return true;
}

// SafeBox
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// Mall
bool CPythonNetworkStream::SendMallCheckoutPacket(BYTE byMallPos, TItemPos InventoryPos)
{
	__PlayMallItemDropSound(byMallPos);

	TPacketCGMallCheckout kMallCheckoutPacket;
	kMallCheckoutPacket.bHeader = HEADER_CG_MALL_CHECKOUT;
	kMallCheckoutPacket.bMallPos = byMallPos;
	kMallCheckoutPacket.ItemPos = InventoryPos;
	if (!Send(sizeof(kMallCheckoutPacket), &kMallCheckoutPacket))
		return false;

	return true;
}

bool CPythonNetworkStream::RecvMallOpenPacket()
{
	TPacketGCMallOpen kMallOpen;
	if (!Recv(sizeof(kMallOpen), &kMallOpen))
		return false;

	CPythonSafeBox::Instance().OpenMall(kMallOpen.bSize);
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OpenMallWindow", Py_BuildValue("(i)", kMallOpen.bSize));

	return true;
}
bool CPythonNetworkStream::RecvMallItemSetPacket()
{
	TPacketGCItemSet kItemSet;
	if (!Recv(sizeof(kItemSet), &kItemSet))
		return false;

	TItemData kItemData;
	kItemData.vnum = kItemSet.vnum;
	kItemData.count = kItemSet.count;
	kItemData.flags = kItemSet.flags;
	kItemData.anti_flags = kItemSet.anti_flags;

	for (int isocket = 0; isocket < ITEM_SOCKET_SLOT_MAX_NUM; ++isocket)
		kItemData.alSockets[isocket] = kItemSet.alSockets[isocket];
	for (int iattr = 0; iattr < ITEM_ATTRIBUTE_SLOT_MAX_NUM; ++iattr)
		kItemData.aAttr[iattr] = kItemSet.aAttr[iattr];
#ifdef ENABLE_BEGINNER_ITEM_SYSTEM
	kItemData.is_basic = false;
#endif
#ifdef ENABLE_ITEM_EVOLUTION_SYSTEM
	kItemData.evolution = kItemSet.evolution;
#endif
#ifdef ENABLE_CHANGELOOK_SYSTEM
	kItemData.transmutation = kItemSet.transmutation;
#endif
	CPythonSafeBox::Instance().SetMallItemData(kItemSet.Cell.cell, kItemData);

	__RefreshMallWindow();

	return true;
}
bool CPythonNetworkStream::RecvMallItemDelPacket()
{
	TPacketGCItemDel kItemDel;
	if (!Recv(sizeof(kItemDel), &kItemDel))
		return false;

	CPythonSafeBox::Instance().DelMallItemData(kItemDel.pos);

	__RefreshMallWindow();
	Tracef(" >> CPythonNetworkStream::RecvMallItemDelPacket\n");

	return true;
}
// Mall
//////////////////////////////////////////////////////////////////////////

// Item
// Recieve
bool CPythonNetworkStream::RecvItemDelPacket()
{
	TPacketGCItemDelDeprecated packet_item_deldeprecated;

	if (!Recv(sizeof(TPacketGCItemDelDeprecated), &packet_item_deldeprecated))
		return false;

	TItemData kItemData;
	kItemData.vnum = packet_item_deldeprecated.vnum;
	kItemData.count = packet_item_deldeprecated.count;
	kItemData.flags = 0;
	for (int i = 0; i < ITEM_SOCKET_SLOT_MAX_NUM; ++i)
		kItemData.alSockets[i] = packet_item_deldeprecated.alSockets[i];
	for (int j = 0; j < ITEM_ATTRIBUTE_SLOT_MAX_NUM; ++j)
		kItemData.aAttr[j] = packet_item_deldeprecated.aAttr[j];
#ifdef ENABLE_BEGINNER_ITEM_SYSTEM
	kItemData.is_basic = packet_item_deldeprecated.is_basic;
#endif
#ifdef ENABLE_ITEM_EVOLUTION_SYSTEM
	kItemData.evolution = packet_item_deldeprecated.evolution;
#endif
#ifdef ENABLE_CHANGELOOK_SYSTEM
	kItemData.transmutation = packet_item_deldeprecated.transmutation;
#endif
	IAbstractPlayer& rkPlayer = IAbstractPlayer::GetSingleton();
	rkPlayer.SetItemData(packet_item_deldeprecated.Cell, kItemData);

#ifdef ENABLE_SWITCHBOT_SYSTEM
	if (packet_item_deldeprecated.Cell.window_type == SWITCHBOT)
	{
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshSwitchbotWindow", Py_BuildValue("()"));
		return true;
	}
#endif

#ifdef ENABLE_ADDITIONAL_INVENTORY
	__RefreshInventoryWindow(packet_item_deldeprecated.Cell.window_type);
#else
	__RefreshInventoryWindow();
#endif
	return true;
}

bool CPythonNetworkStream::RecvItemSetPacket()
{
	TPacketGCItemSet packet_item_set;

	if (!Recv(sizeof(TPacketGCItemSet), &packet_item_set))
		return false;

	TItemData kItemData;
	kItemData.vnum = packet_item_set.vnum;
	kItemData.count = packet_item_set.count;
	kItemData.flags = packet_item_set.flags;
	kItemData.anti_flags = packet_item_set.anti_flags;

	for (int i = 0; i < ITEM_SOCKET_SLOT_MAX_NUM; ++i)
		kItemData.alSockets[i] = packet_item_set.alSockets[i];
	for (int j = 0; j < ITEM_ATTRIBUTE_SLOT_MAX_NUM; ++j)
		kItemData.aAttr[j] = packet_item_set.aAttr[j];
#ifdef ENABLE_BEGINNER_ITEM_SYSTEM
	kItemData.is_basic = packet_item_set.is_basic;
#endif
#ifdef ENABLE_ITEM_EVOLUTION_SYSTEM
	kItemData.evolution = packet_item_set.evolution;
#endif
#ifdef ENABLE_CHANGELOOK_SYSTEM
	kItemData.transmutation = packet_item_set.transmutation;
#endif
	IAbstractPlayer& rkPlayer = IAbstractPlayer::GetSingleton();
	rkPlayer.SetItemData(packet_item_set.Cell, kItemData);

	if (packet_item_set.highlight)
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_Highlight_Item", Py_BuildValue("(ii)", packet_item_set.Cell.window_type, packet_item_set.Cell.cell));

#ifdef ENABLE_SWITCHBOT_SYSTEM
	if (packet_item_set.Cell.window_type == SWITCHBOT)
	{
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshSwitchbotWindow", Py_BuildValue("()"));
		return true;
	}
#endif

#ifdef ENABLE_ADDITIONAL_INVENTORY
	__RefreshInventoryWindow(packet_item_set.Cell.window_type);
#else
	__RefreshInventoryWindow();
#endif
	return true;
}

bool CPythonNetworkStream::RecvItemUsePacket()
{
	TPacketGCItemUse packet_item_use;

	if (!Recv(sizeof(TPacketGCItemUse), &packet_item_use))
		return false;

#ifdef ENABLE_ADDITIONAL_INVENTORY
	__RefreshInventoryWindow(packet_item_use.Cell.window_type);
#else
	__RefreshInventoryWindow();
#endif
	return true;
}


bool CPythonNetworkStream::RecvItemUpdatePacket()
{
	TPacketGCItemUpdate packet_item_update;

	if (!Recv(sizeof(TPacketGCItemUpdate), &packet_item_update))
		return false;

	IAbstractPlayer& rkPlayer = IAbstractPlayer::GetSingleton();

	if (rkPlayer.GetItemCount(packet_item_update.Cell) != packet_item_update.count)
		rkPlayer.SetItemCount(packet_item_update.Cell, packet_item_update.count);

	for (int i = 0; i < ITEM_SOCKET_SLOT_MAX_NUM; ++i)
	{
		if (rkPlayer.GetItemMetinSocket(packet_item_update.Cell, i) != packet_item_update.alSockets[i])
			rkPlayer.SetItemMetinSocket(packet_item_update.Cell, i, packet_item_update.alSockets[i]);
	}

	for (int j = 0; j < ITEM_ATTRIBUTE_SLOT_MAX_NUM; ++j)
	{
		if (rkPlayer.GetItemAttributeValue(packet_item_update.Cell, j) != packet_item_update.aAttr[j].sValue || rkPlayer.GetItemAttributeType(packet_item_update.Cell, j) != packet_item_update.aAttr[j].bType)
			rkPlayer.SetItemAttribute(packet_item_update.Cell, j, packet_item_update.aAttr[j].bType, packet_item_update.aAttr[j].sValue);
	}

#ifdef ENABLE_BEGINNER_ITEM_SYSTEM
	if (rkPlayer.IsBasicItem(packet_item_update.Cell) != packet_item_update.is_basic)
		rkPlayer.SetBasicItem(packet_item_update.Cell, packet_item_update.is_basic);
#endif
#ifdef ENABLE_ITEM_EVOLUTION_SYSTEM
	if (rkPlayer.GetItemEvolution(packet_item_update.Cell) != packet_item_update.evolution)
		rkPlayer.SetItemEvolution(packet_item_update.Cell, packet_item_update.evolution);
#endif
#ifdef ENABLE_CHANGELOOK_SYSTEM
	if (rkPlayer.GetItemTransmutation(packet_item_update.Cell) != packet_item_update.transmutation)
		rkPlayer.SetItemTransmutation(packet_item_update.Cell, packet_item_update.transmutation);
#endif

#ifdef ENABLE_ADDITIONAL_INVENTORY
	__RefreshInventoryWindow(packet_item_update.Cell.window_type);
#else
	__RefreshInventoryWindow();
#endif
	return true;
}

bool CPythonNetworkStream::RecvItemGroundAddPacket()
{
	TPacketGCItemGroundAdd packet_item_ground_add;

	if (!Recv(sizeof(TPacketGCItemGroundAdd), &packet_item_ground_add))
		return false;

	__GlobalPositionToLocalPosition(packet_item_ground_add.lX, packet_item_ground_add.lY);

#ifdef ENABLE_ITEM_ENTITY_UTILITY
	CPythonItem::Instance().CreateItem(packet_item_ground_add.dwVID, packet_item_ground_add.dwVnum, packet_item_ground_add.lX, packet_item_ground_add.lY, packet_item_ground_add.lZ, true, packet_item_ground_add.alSockets, packet_item_ground_add.wCount);
#else
	CPythonItem::Instance().CreateItem(packet_item_ground_add.dwVID, packet_item_ground_add.dwVnum, packet_item_ground_add.lX, packet_item_ground_add.lY, packet_item_ground_add.lZ);
#endif
	return true;
}

bool CPythonNetworkStream::RecvItemOwnership()
{
	TPacketGCItemOwnership p;

	if (!Recv(sizeof(TPacketGCItemOwnership), &p))
		return false;

	CPythonItem::Instance().SetOwnership(p.dwVID, p.szName);
	return true;
}

bool CPythonNetworkStream::RecvItemGroundDelPacket()
{
	TPacketGCItemGroundDel	packet_item_ground_del;

	if (!Recv(sizeof(TPacketGCItemGroundDel), &packet_item_ground_del))
		return false;

	CPythonItem::Instance().DeleteItem(packet_item_ground_del.vid);
	return true;
}

bool CPythonNetworkStream::RecvQuickSlotAddPacket()
{
	TPacketGCQuickSlotAdd packet_quick_slot_add;

	if (!Recv(sizeof(TPacketGCQuickSlotAdd), &packet_quick_slot_add))
		return false;

	IAbstractPlayer& rkPlayer = IAbstractPlayer::GetSingleton();
	rkPlayer.AddQuickSlot(packet_quick_slot_add.pos, packet_quick_slot_add.slot.Type, packet_quick_slot_add.slot.Position);

#ifdef ENABLE_ADDITIONAL_INVENTORY
	__RefreshInventoryWindow(INVENTORY);
#else
	__RefreshInventoryWindow();
#endif

	return true;
}

bool CPythonNetworkStream::RecvQuickSlotDelPacket()
{
	TPacketGCQuickSlotDel packet_quick_slot_del;

	if (!Recv(sizeof(TPacketGCQuickSlotDel), &packet_quick_slot_del))
		return false;

	IAbstractPlayer& rkPlayer = IAbstractPlayer::GetSingleton();
	rkPlayer.DeleteQuickSlot(packet_quick_slot_del.pos);

#ifdef ENABLE_ADDITIONAL_INVENTORY
	__RefreshInventoryWindow(INVENTORY);
#else
	__RefreshInventoryWindow();
#endif

	return true;
}

bool CPythonNetworkStream::RecvQuickSlotMovePacket()
{
	TPacketGCQuickSlotSwap packet_quick_slot_swap;

	if (!Recv(sizeof(TPacketGCQuickSlotSwap), &packet_quick_slot_swap))
		return false;

	IAbstractPlayer& rkPlayer = IAbstractPlayer::GetSingleton();
	rkPlayer.MoveQuickSlot(packet_quick_slot_swap.pos, packet_quick_slot_swap.change_pos);

#ifdef ENABLE_ADDITIONAL_INVENTORY
	__RefreshInventoryWindow(INVENTORY);
#else
	__RefreshInventoryWindow();
#endif

	return true;
}

bool CPythonNetworkStream::SendShopEndPacket()
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGShop packet_shop;
	packet_shop.header = HEADER_CG_SHOP;
	packet_shop.subheader = SHOP_SUBHEADER_CG_END;

	if (!Send(sizeof(packet_shop), &packet_shop))
	{
		Tracef("SendShopEndPacket Error\n");
		return false;
	}

	return true;
}

bool CPythonNetworkStream::SendShopBuyPacket(BYTE bPos)
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGShop PacketShop;
	PacketShop.header = HEADER_CG_SHOP;
	PacketShop.subheader = SHOP_SUBHEADER_CG_BUY;

	if (!Send(sizeof(TPacketCGShop), &PacketShop))
	{
		Tracef("SendShopBuyPacket Error\n");
		return false;
	}

	BYTE bCount = 1;
	if (!Send(sizeof(BYTE), &bCount))
	{
		Tracef("SendShopBuyPacket Error\n");
		return false;
	}

	if (!Send(sizeof(BYTE), &bPos))
	{
		Tracef("SendShopBuyPacket Error\n");
		return false;
	}

	return true;
}

bool CPythonNetworkStream::SendShopSellPacket(BYTE bySlot)
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGShop PacketShop;
	PacketShop.header = HEADER_CG_SHOP;
	PacketShop.subheader = SHOP_SUBHEADER_CG_SELL;

	if (!Send(sizeof(TPacketCGShop), &PacketShop))
	{
		Tracef("SendShopSellPacket Error\n");
		return false;
	}
	if (!Send(sizeof(BYTE), &bySlot))
	{
		Tracef("SendShopAddSellPacket Error\n");
		return false;
	}

	return true;
}

#ifdef ENABLE_ADDITIONAL_INVENTORY
#ifdef ENABLE_ITEM_COUNT_LIMIT
bool CPythonNetworkStream::SendShopSellPacketNew(WORD bySlot, DWORD byCount, BYTE byType)
#else
bool CPythonNetworkStream::SendShopSellPacketNew(WORD bySlot, BYTE byCount, BYTE byType)
#endif
#else
#ifdef ENABLE_ITEM_COUNT_LIMIT
bool CPythonNetworkStream::SendShopSellPacketNew(WORD bySlot, DWORD byCount)
#else
bool CPythonNetworkStream::SendShopSellPacketNew(WORD bySlot, BYTE byCount)
#endif
#endif
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGShop PacketShop;
	PacketShop.header = HEADER_CG_SHOP;
	PacketShop.subheader = SHOP_SUBHEADER_CG_SELL2;

	if (!Send(sizeof(TPacketCGShop), &PacketShop))
	{
		Tracef("SendShopSellPacket Error\n");
		return false;
	}
	if (!Send(sizeof(WORD), &bySlot))
	{
		Tracef("SendShopAddSellPacket Error\n");
		return false;
	}
#ifdef ENABLE_ITEM_COUNT_LIMIT
	if (!Send(sizeof(DWORD), &byCount))
#else
	if (!Send(sizeof(BYTE), &byCount))
#endif
	{
		Tracef("SendShopAddSellPacket Error\n");
		return false;
	}
#ifdef ENABLE_ADDITIONAL_INVENTORY
	if (!Send(sizeof(BYTE), &byType))
	{
		Tracef("SendShopAddSellPacket Error\n");
		return false;
	}
#endif
#ifdef ENABLE_ADDITIONAL_INVENTORY
	Tracef(" SendShopSellPacketNew(bySlot=%d, byCount=%d, byType=%d)\n", bySlot, byCount, byType);
#else
	Tracef(" SendShopSellPacketNew(bySlot=%d, byCount=%d)\n", bySlot, byCount);
#endif
	return true;
}

// Send
#ifdef ENABLE_USE_ITEM_COUNT
bool CPythonNetworkStream::SendItemUsePacket(TItemPos pos, DWORD count)
#else // ENABLE_USE_ITEM_COUNT
bool CPythonNetworkStream::SendItemUsePacket(TItemPos pos)
#endif // ENABLE_USE_ITEM_COUNT
{
	if (!__CanActMainInstance())
		return true;

	if (__IsEquipItemInSlot(pos))
	{
		if (CPythonExchange::Instance().isTrading())
		{
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_AppendNotifyMessage", Py_BuildValue("(s)", "CANNOT_EQUIP_EXCHANGE"));
			return true;
		}

		if (CPythonShop::Instance().IsOpen())
		{
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_AppendNotifyMessage", Py_BuildValue("(s)", "CANNOT_EQUIP_SHOP"));
			return true;
		}

		if (__IsPlayerAttacking())
			return true;
	}

	__PlayInventoryItemUseSound(pos);

	const TItemData* itemData = CPythonPlayer::Instance().GetItemData(pos);
#ifdef ENABLE_USE_ITEM_COUNT
	if (51511 <= itemData->vnum && itemData->vnum <= 51516)
		count = min(30, count);
#endif // ENABLE_USE_ITEM_COUNT

	TPacketCGItemUse itemUsePacket;
	itemUsePacket.header = HEADER_CG_ITEM_USE;
	itemUsePacket.pos = pos;
#ifdef ENABLE_USE_ITEM_COUNT
	itemUsePacket.count = count;
#endif // ENABLE_USE_ITEM_COUNT

	if (!Send(sizeof(TPacketCGItemUse), &itemUsePacket))
	{
		Tracen("SendItemUsePacket Error");
		return false;
	}

	return true;
}

bool CPythonNetworkStream::SendItemUseToItemPacket(TItemPos source_pos, TItemPos target_pos)
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGItemUseToItem itemUseToItemPacket;
	itemUseToItemPacket.header = HEADER_CG_ITEM_USE_TO_ITEM;
	itemUseToItemPacket.source_pos = source_pos;
	itemUseToItemPacket.target_pos = target_pos;

	if (!Send(sizeof(TPacketCGItemUseToItem), &itemUseToItemPacket))
	{
		Tracen("SendItemUseToItemPacket Error");
		return false;
	}

#ifdef _DEBUG
	Tracef(" << SendItemUseToItemPacket(src=%d, dst=%d)\n", source_pos, target_pos);
#endif

	return true;
}

#ifdef ENABLE_DROP_DIALOG_SYSTEM
bool CPythonNetworkStream::SendItemDeletePacket(TItemPos item_pos)
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGItemDelete itemDeletePacket;
	itemDeletePacket.header = HEADER_CG_ITEM_DELETE;
	itemDeletePacket.item_pos = item_pos;

	if (!Send(sizeof(TPacketCGItemDelete), &itemDeletePacket))
	{
		Tracen("SendItemDeletePacket Error");
		return false;
	}

#ifdef _DEBUG
	Tracef(" << SendItemDeletePacket(item_pos=%d)\n", item_pos);
#endif

	return true;
}

bool CPythonNetworkStream::SendItemSellPacket(TItemPos item_pos)
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGItemSell itemSellPacket;
	itemSellPacket.header = HEADER_CG_ITEM_SELL;
	itemSellPacket.item_pos = item_pos;

	if (!Send(sizeof(TPacketCGItemSell), &itemSellPacket))
	{
		Tracen("SendItemSellPacket Error");
		return false;
	}

#ifdef _DEBUG
	Tracef(" << SendItemSellPacket(item_pos=%d)\n", item_pos);
#endif

	return true;
}
#endif
bool CPythonNetworkStream::__IsEquipItemInSlot(TItemPos uSlotPos)
{
	IAbstractPlayer& rkPlayer = IAbstractPlayer::GetSingleton();
	return rkPlayer.IsEquipItemInSlot(uSlotPos);
}

void CPythonNetworkStream::__PlayInventoryItemUseSound(TItemPos uSlotPos)
{
	IAbstractPlayer& rkPlayer = IAbstractPlayer::GetSingleton();
	DWORD dwItemID = rkPlayer.GetItemIndex(uSlotPos);

	CPythonItem& rkItem = CPythonItem::Instance();
	rkItem.PlayUseSound(dwItemID);
}

void CPythonNetworkStream::__PlayInventoryItemDropSound(TItemPos uSlotPos)
{
	IAbstractPlayer& rkPlayer = IAbstractPlayer::GetSingleton();
	DWORD dwItemID = rkPlayer.GetItemIndex(uSlotPos);

	CPythonItem& rkItem = CPythonItem::Instance();
	rkItem.PlayDropSound(dwItemID);
}

//void CPythonNetworkStream::__PlayShopItemDropSound(UINT uSlotPos)
//{
//	DWORD dwItemID;
//	CPythonShop& rkShop=CPythonShop::Instance();
//	if (!rkShop.GetSlotItemID(uSlotPos, &dwItemID))
//		return;
//
//	CPythonItem& rkItem=CPythonItem::Instance();
//	rkItem.PlayDropSound(dwItemID);
//}

void CPythonNetworkStream::__PlaySafeBoxItemDropSound(UINT uSlotPos)
{
	DWORD dwItemID;
	CPythonSafeBox& rkSafeBox = CPythonSafeBox::Instance();
	if (!rkSafeBox.GetSlotItemID(uSlotPos, &dwItemID))
		return;

	CPythonItem& rkItem = CPythonItem::Instance();
	rkItem.PlayDropSound(dwItemID);
}

void CPythonNetworkStream::__PlayMallItemDropSound(UINT uSlotPos)
{
	DWORD dwItemID;
	CPythonSafeBox& rkSafeBox = CPythonSafeBox::Instance();
	if (!rkSafeBox.GetSlotMallItemID(uSlotPos, &dwItemID))
		return;

	CPythonItem& rkItem = CPythonItem::Instance();
	rkItem.PlayDropSound(dwItemID);
}

#ifdef ENABLE_ITEM_COUNT_LIMIT
bool CPythonNetworkStream::SendItemMovePacket(TItemPos pos, TItemPos change_pos, DWORD num)
#else
bool CPythonNetworkStream::SendItemMovePacket(TItemPos pos, TItemPos change_pos, BYTE num)
#endif
{
	if (!__CanActMainInstance())
		return true;

	if (__IsEquipItemInSlot(pos))
	{
		if (CPythonExchange::Instance().isTrading())
		{
			if (pos.IsEquipCell() || change_pos.IsEquipCell())
			{
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_AppendNotifyMessage", Py_BuildValue("(s)", "CANNOT_EQUIP_EXCHANGE"));
				return true;
			}
		}

		if (CPythonShop::Instance().IsOpen())
		{
			if (pos.IsEquipCell() || change_pos.IsEquipCell())
			{
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_AppendNotifyMessage", Py_BuildValue("(s)", "CANNOT_EQUIP_SHOP"));
				return true;
			}
		}

		if (__IsPlayerAttacking())
			return true;
	}

	__PlayInventoryItemDropSound(pos);

	TPacketCGItemMove	itemMovePacket;
	itemMovePacket.header = HEADER_CG_ITEM_MOVE;
	itemMovePacket.pos = pos;
	itemMovePacket.change_pos = change_pos;
	itemMovePacket.num = num;

	if (!Send(sizeof(TPacketCGItemMove), &itemMovePacket))
	{
		Tracen("SendItemMovePacket Error");
		return false;
	}

	return true;
}

bool CPythonNetworkStream::SendItemPickUpPacket(DWORD vid)
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGItemPickUp	itemPickUpPacket;
	itemPickUpPacket.header = HEADER_CG_ITEM_PICKUP;
	itemPickUpPacket.vid = vid;

	if (!Send(sizeof(TPacketCGItemPickUp), &itemPickUpPacket))
	{
		Tracen("SendItemPickUpPacket Error");
		return false;
	}

	return true;
}

bool CPythonNetworkStream::SendQuickSlotAddPacket(BYTE wpos, BYTE type, BYTE pos)
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGQuickSlotAdd quickSlotAddPacket;

	quickSlotAddPacket.header = HEADER_CG_QUICKSLOT_ADD;
	quickSlotAddPacket.pos = wpos;
	quickSlotAddPacket.slot.Type = type;
	quickSlotAddPacket.slot.Position = pos;

	if (!Send(sizeof(TPacketCGQuickSlotAdd), &quickSlotAddPacket))
	{
		Tracen("SendQuickSlotAddPacket Error");
		return false;
	}

	return true;
}

bool CPythonNetworkStream::SendQuickSlotDelPacket(BYTE pos)
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGQuickSlotDel quickSlotDelPacket;

	quickSlotDelPacket.header = HEADER_CG_QUICKSLOT_DEL;
	quickSlotDelPacket.pos = pos;

	if (!Send(sizeof(TPacketCGQuickSlotDel), &quickSlotDelPacket))
	{
		Tracen("SendQuickSlotDelPacket Error");
		return false;
	}

	return true;
}

bool CPythonNetworkStream::SendQuickSlotMovePacket(BYTE pos, BYTE change_pos)
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGQuickSlotSwap quickSlotSwapPacket;

	quickSlotSwapPacket.header = HEADER_CG_QUICKSLOT_SWAP;
	quickSlotSwapPacket.pos = pos;
	quickSlotSwapPacket.change_pos = change_pos;

	if (!Send(sizeof(TPacketCGQuickSlotSwap), &quickSlotSwapPacket))
	{
		Tracen("SendQuickSlotSwapPacket Error");
		return false;
	}

	return true;
}

bool CPythonNetworkStream::RecvSpecialEffect()
{
	TPacketGCSpecialEffect kSpecialEffect;
	if (!Recv(sizeof(kSpecialEffect), &kSpecialEffect))
		return false;

	DWORD effect = -1;
	bool bPlayPotionSound = false;
	bool bAttachEffect = true;
	switch (kSpecialEffect.type)
	{
	case SE_HPUP_RED:
		effect = CInstanceBase::EFFECT_HPUP_RED;
		bPlayPotionSound = true;
		break;
	case SE_SPUP_BLUE:
		effect = CInstanceBase::EFFECT_SPUP_BLUE;
		bPlayPotionSound = true;
		break;
	case SE_SPEEDUP_GREEN:
		effect = CInstanceBase::EFFECT_SPEEDUP_GREEN;
		bPlayPotionSound = true;
		break;
	case SE_DXUP_PURPLE:
		effect = CInstanceBase::EFFECT_DXUP_PURPLE;
		bPlayPotionSound = true;
		break;
	case SE_CRITICAL:
		effect = CInstanceBase::EFFECT_CRITICAL;
		break;
	case SE_PENETRATE:
		effect = CInstanceBase::EFFECT_PENETRATE;
		break;
	case SE_BLOCK:
		effect = CInstanceBase::EFFECT_BLOCK;
		break;
	case SE_DODGE:
		effect = CInstanceBase::EFFECT_DODGE;
		break;
	case SE_CHINA_FIREWORK:
		effect = CInstanceBase::EFFECT_FIRECRACKER;
		bAttachEffect = false;
		break;
	case SE_SPIN_TOP:
		effect = CInstanceBase::EFFECT_SPIN_TOP;
		bAttachEffect = false;
		break;
	case SE_SUCCESS:
		effect = CInstanceBase::EFFECT_SUCCESS;
		bAttachEffect = false;
		break;
	case SE_FAIL:
		effect = CInstanceBase::EFFECT_FAIL;
		break;
	case SE_FR_SUCCESS:
		effect = CInstanceBase::EFFECT_FR_SUCCESS;
		bAttachEffect = false;
		break;
	case SE_PERCENT_DAMAGE1:
		effect = CInstanceBase::EFFECT_PERCENT_DAMAGE1;
		break;
	case SE_PERCENT_DAMAGE2:
		effect = CInstanceBase::EFFECT_PERCENT_DAMAGE2;
		break;
	case SE_PERCENT_DAMAGE3:
		effect = CInstanceBase::EFFECT_PERCENT_DAMAGE3;
		break;
	case SE_AUTO_HPUP:
		effect = CInstanceBase::EFFECT_AUTO_HPUP;
		break;
	case SE_AUTO_SPUP:
		effect = CInstanceBase::EFFECT_AUTO_SPUP;
		break;
	case SE_EQUIP_RAMADAN_RING:
		effect = CInstanceBase::EFFECT_RAMADAN_RING_EQUIP;
		break;
	case SE_EQUIP_HALLOWEEN_CANDY:
		effect = CInstanceBase::EFFECT_HALLOWEEN_CANDY_EQUIP;
		break;
	case SE_EQUIP_HAPPINESS_RING:
		effect = CInstanceBase::EFFECT_HAPPINESS_RING_EQUIP;
		break;
	case SE_EQUIP_LOVE_PENDANT:
		effect = CInstanceBase::EFFECT_LOVE_PENDANT_EQUIP;
		break;
#ifdef ENABLE_ACCE_SYSTEM
	case SE_EFFECT_ACCE_SUCCEDED:
		effect = CInstanceBase::EFFECT_ACCE_SUCCEDED;
		break;

	case SE_EFFECT_ACCE_EQUIP:
		effect = CInstanceBase::EFFECT_ACCE_EQUIP;
		break;
#endif
#ifdef ENABLE_PVP_EFFECT
	case SE_PVP_WIN:
		effect = CInstanceBase::EFFECT_PVP_WIN;
		break;
	case SE_PVP_OPEN1:
		effect = CInstanceBase::EFFECT_PVP_OPEN1;
		break;
	case SE_PVP_OPEN2:
		effect = CInstanceBase::EFFECT_PVP_OPEN2;
		break;
	case SE_PVP_BEGIN1:
		effect = CInstanceBase::EFFECT_PVP_BEGIN1;
		break;
	case SE_PVP_BEGIN2:
		effect = CInstanceBase::EFFECT_PVP_BEGIN2;
		break;
#endif
#ifdef ENABLE_SMITH_EFFECT
	case SE_FR_FAIL:
		effect = CInstanceBase::EFFECT_FR_FAIL;
		break;
#endif
#ifdef ENABLE_AGGR_MONSTER_EFFECT
	case SE_AGGREGATE_MONSTER_EFFECT:
		effect = CInstanceBase::EFFECT_AGGREGATE_MONSTER;
		break;
#endif
	default:
		TraceError("%d is not a special effect number. TPacketGCSpecialEffect", kSpecialEffect.type);
		break;
	}

	if (bPlayPotionSound)
	{
		IAbstractPlayer& rkPlayer = IAbstractPlayer::GetSingleton();
		if (rkPlayer.IsMainCharacterIndex(kSpecialEffect.vid))
		{
			CPythonItem& rkItem = CPythonItem::Instance();
			rkItem.PlayUsePotionSound();
		}
	}

	if (-1 != effect)
	{
		CInstanceBase* pInstance = CPythonCharacterManager::Instance().GetInstancePtr(kSpecialEffect.vid);
		if (pInstance)
		{
			if (bAttachEffect)
#ifndef ENABLE_GRAPHIC_OPTIMIZATION
				pInstance->AttachSpecialEffect(effect);
#else
			{
				if (effect == CInstanceBase::EFFECT_CRITICAL || effect == CInstanceBase::EFFECT_PENETRATE)
					pInstance->AttachSpecialEffect(effect, true);
				else
					pInstance->AttachSpecialEffect(effect);
			}
#endif
			else
				pInstance->CreateSpecialEffect(effect);
		}
	}

	return true;
}

bool CPythonNetworkStream::RecvSpecificEffect()
{
	TPacketGCSpecificEffect kSpecificEffect;
	if (!Recv(sizeof(kSpecificEffect), &kSpecificEffect))
		return false;

	CInstanceBase* pInstance = CPythonCharacterManager::Instance().GetInstancePtr(kSpecificEffect.vid);
	//EFFECT_TEMP
	if (pInstance)
	{
		CInstanceBase::RegisterEffect(CInstanceBase::EFFECT_TEMP, "", kSpecificEffect.effect_file, false);
		pInstance->AttachSpecialEffect(CInstanceBase::EFFECT_TEMP);
	}

	return true;
}

bool CPythonNetworkStream::RecvDragonSoulRefine()
{
	TPacketGCDragonSoulRefine kDragonSoul;

	if (!Recv(sizeof(kDragonSoul), &kDragonSoul))
		return false;

	switch (kDragonSoul.bSubType)
	{
	case DS_SUB_HEADER_OPEN:
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_DragonSoulRefineWindow_Open", Py_BuildValue("()"));
		break;
	case DS_SUB_HEADER_REFINE_FAIL:
	case DS_SUB_HEADER_REFINE_FAIL_MAX_REFINE:
	case DS_SUB_HEADER_REFINE_FAIL_INVALID_MATERIAL:
	case DS_SUB_HEADER_REFINE_FAIL_NOT_ENOUGH_MONEY:
	case DS_SUB_HEADER_REFINE_FAIL_NOT_ENOUGH_MATERIAL:
	case DS_SUB_HEADER_REFINE_FAIL_TOO_MUCH_MATERIAL:
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_DragonSoulRefineWindow_RefineFail", Py_BuildValue("(iii)",
			kDragonSoul.bSubType, kDragonSoul.Pos.window_type, kDragonSoul.Pos.cell));
		break;
	case DS_SUB_HEADER_REFINE_SUCCEED:
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_DragonSoulRefineWindow_RefineSucceed",
			Py_BuildValue("(ii)", kDragonSoul.Pos.window_type, kDragonSoul.Pos.cell));
		break;
	}

	return true;
}

#ifdef ENABLE_ANCIENT_ATTR_SYSTEM
bool CPythonNetworkStream::SendItemNewAttributePacket(TItemPos source_pos, TItemPos target_pos, BYTE* bValues)
{
	if (!__CanActMainInstance())
		return true;
	TPacketCGItemNewAttribute itemNewAttributePacket;
	itemNewAttributePacket.header = HEADER_CG_ITEM_USE_NEW_ATTRIBUTE;
	itemNewAttributePacket.source_pos = source_pos;
	itemNewAttributePacket.target_pos = target_pos;
	itemNewAttributePacket.bValues[0] = bValues[0];
	itemNewAttributePacket.bValues[1] = bValues[1];
	itemNewAttributePacket.bValues[2] = bValues[2];
	itemNewAttributePacket.bValues[3] = bValues[3];

	if (!Send(sizeof(TPacketCGItemNewAttribute), &itemNewAttributePacket))
	{
		Tracen("SendItemNewAttributePacket Error");
		return false;
	}
	return true;
}
#endif