#include "stdafx.h"
#include "questmanager.h"
#include "char.h"
#include "item.h"
#include "item_manager.h"
#include "log.h"
#include "db.h"
#include "config.h"
#ifdef __GROWTH_PET_SYSTEM__
#include "New_PetSystem.h"
#endif
#ifdef __GROWTH_MOUNT_SYSTEM__
#include "New_MountSystem.h"
#endif

#undef sys_err
#ifndef __WIN32__
#define sys_err(fmt, args...) quest::CQuestManager::instance().QuestError(__FUNCTION__, __LINE__, fmt, ##args)
#else
#define sys_err(fmt, ...) quest::CQuestManager::instance().QuestError(__FUNCTION__, __LINE__, fmt, __VA_ARGS__)
#endif

namespace quest
{
	//
	// "item" Lua functions
	//

	ALUA(item_get_cell)
	{
		CQuestManager& q = CQuestManager::instance();

		if (q.GetCurrentItem())
		{
			lua_pushnumber(L, q.GetCurrentItem()->GetCell());
		}
		else
			lua_pushnumber(L, 0);
		return 1;
	}

	ALUA(item_select_cell)
	{
		lua_pushboolean(L, 0);
		if (!lua_isnumber(L, 1))
		{
			return 1;
		}
		DWORD cell = (DWORD)lua_tonumber(L, 1);

		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		LPITEM item = ch ? ch->GetInventoryItem(cell) : NULL;

		if (!item)
		{
			return 1;
		}

		CQuestManager::instance().SetCurrentItem(item);
		lua_pushboolean(L, 1);

		return 1;
	}

	ALUA(item_select)
	{
		lua_pushboolean(L, 0);
		if (!lua_isnumber(L, 1))
		{
			return 1;
		}
		DWORD id = (DWORD)lua_tonumber(L, 1);
		LPITEM item = ITEM_MANAGER::instance().Find(id);

		if (!item)
		{
			return 1;
		}

		CQuestManager::instance().SetCurrentItem(item);
		lua_pushboolean(L, 1);

		return 1;
	}

	ALUA(item_get_id)
	{
		CQuestManager& q = CQuestManager::instance();

		if (q.GetCurrentItem())
		{
			lua_pushnumber(L, q.GetCurrentItem()->GetID());
		}
		else
			lua_pushnumber(L, 0);
		return 1;
	}

	ALUA(item_remove)
	{
		CQuestManager& q = CQuestManager::instance();
		LPITEM item = q.GetCurrentItem();
		if (item != NULL) {
			if (q.GetCurrentCharacterPtr() == item->GetOwner()) {
				ITEM_MANAGER::instance().RemoveItem(item);
			}
			else {
				sys_err("Tried to remove invalid item %p", get_pointer(item));
			}
			q.ClearCurrentItem();
		}

		return 0;
	}

	ALUA(item_get_socket)
	{
		CQuestManager& q = CQuestManager::instance();
		if (q.GetCurrentItem() && lua_isnumber(L, 1))
		{
			int idx = (int)lua_tonumber(L, 1);
			if (idx < 0 || idx >= ITEM_SOCKET_MAX_NUM)
				lua_pushnumber(L, 0);
			else
				lua_pushnumber(L, q.GetCurrentItem()->GetSocket(idx));
		}
		else
		{
			lua_pushnumber(L, 0);
		}
		return 1;
	}

	ALUA(item_set_socket)
	{
		CQuestManager& q = CQuestManager::instance();
		if (q.GetCurrentItem() && lua_isnumber(L, 1) && lua_isnumber(L, 2))
		{
			int idx = (int)lua_tonumber(L, 1);
			int value = (int)lua_tonumber(L, 2);
			if (idx >= 0 && idx < ITEM_SOCKET_MAX_NUM)
				q.GetCurrentItem()->SetSocket(idx, value);
		}
		return 0;
	}

	ALUA(item_get_vnum)
	{
		CQuestManager& q = CQuestManager::instance();
		LPITEM item = q.GetCurrentItem();

		if (item)
			lua_pushnumber(L, item->GetVnum());
		else
			lua_pushnumber(L, 0);

		return 1;
	}

	ALUA(item_has_flag)
	{
		CQuestManager& q = CQuestManager::instance();
		LPITEM item = q.GetCurrentItem();

		if (!lua_isnumber(L, 1))
		{
			sys_err("flag is not a number.");
			lua_pushboolean(L, 0);
			return 1;
		}

		if (!item)
		{
			lua_pushboolean(L, 0);
			return 1;
		}

		long lCheckFlag = (long)lua_tonumber(L, 1);
		lua_pushboolean(L, IS_SET(item->GetFlag(), lCheckFlag));

		return 1;
	}

	ALUA(item_get_value)
	{
		CQuestManager& q = CQuestManager::instance();
		LPITEM item = q.GetCurrentItem();

		if (!item)
		{
			lua_pushnumber(L, 0);
			return 1;
		}

		if (!lua_isnumber(L, 1))
		{
			sys_err("index is not a number");
			lua_pushnumber(L, 0);
			return 1;
		}

		int index = (int)lua_tonumber(L, 1);

		if (index < 0 || index >= ITEM_VALUES_MAX_NUM)
		{
			sys_err("index(%d) is out of range (0..%d)", index, ITEM_VALUES_MAX_NUM);
			lua_pushnumber(L, 0);
		}
		else
			lua_pushnumber(L, item->GetValue(index));

		return 1;
	}

	ALUA(item_set_value)
	{
		CQuestManager& q = CQuestManager::instance();
		LPITEM item = q.GetCurrentItem();

		if (!item)
		{
			lua_pushnumber(L, 0);
			return 1;
		}

		if (false == (lua_isnumber(L, 1) && lua_isnumber(L, 2) && lua_isnumber(L, 3)))
		{
			sys_err("index is not a number");
			lua_pushnumber(L, 0);
			return 1;
		}

		item->SetForceAttribute(
			lua_tonumber(L, 1),		// index
			lua_tonumber(L, 2),		// apply type
			lua_tonumber(L, 3)		// apply value
		);

		return 0;
	}

	ALUA(item_get_name)
	{
		CQuestManager& q = CQuestManager::instance();
		LPITEM item = q.GetCurrentItem();

		if (item)
			lua_pushstring(L, item->GetName());
		else
			lua_pushstring(L, "");

		return 1;
	}

	ALUA(item_get_size)
	{
		CQuestManager& q = CQuestManager::instance();
		LPITEM item = q.GetCurrentItem();

		if (item)
			lua_pushnumber(L, item->GetSize());
		else
			lua_pushnumber(L, 0);

		return 1;
	}

	ALUA(item_get_count)
	{
		CQuestManager& q = CQuestManager::instance();
		LPITEM item = q.GetCurrentItem();

		if (item)
			lua_pushnumber(L, item->GetCount());
		else
			lua_pushnumber(L, 0);

		return 1;
	}

	ALUA(item_get_type)
	{
		CQuestManager& q = CQuestManager::instance();
		LPITEM item = q.GetCurrentItem();

		if (item)
			lua_pushnumber(L, item->GetType());
		else
			lua_pushnumber(L, 0);

		return 1;
	}

	ALUA(item_get_sub_type)
	{
		CQuestManager& q = CQuestManager::instance();
		LPITEM item = q.GetCurrentItem();

		if (item)
			lua_pushnumber(L, item->GetSubType());
		else
			lua_pushnumber(L, 0);

		return 1;
	}

	ALUA(item_get_refine_vnum)
	{
		CQuestManager& q = CQuestManager::instance();
		LPITEM item = q.GetCurrentItem();

		if (item)
			lua_pushnumber(L, item->GetRefinedVnum());
		else
			lua_pushnumber(L, 0);

		return 1;
	}

	ALUA(item_next_refine_vnum)
	{
		DWORD vnum = 0;
		if (lua_isnumber(L, 1))
			vnum = (DWORD)lua_tonumber(L, 1);

		TItemTable* pTable = ITEM_MANAGER::instance().GetTable(vnum);
		if (pTable)
		{
			lua_pushnumber(L, pTable->dwRefinedVnum);
		}
		else
		{
			sys_err("Cannot find item table of vnum %u", vnum);
			lua_pushnumber(L, 0);
		}
		return 1;
	}

	ALUA(item_get_level)
	{
		CQuestManager& q = CQuestManager::instance();
		LPITEM item = q.GetCurrentItem();

		if (item)
			lua_pushnumber(L, item->GetRefineLevel());
		else
			lua_pushnumber(L, 0);

		return 1;
	}

	ALUA(item_get_level_limit)
	{
		CQuestManager& q = CQuestManager::instance();

		if (q.GetCurrentItem())
		{
			if (q.GetCurrentItem()->GetType() != ITEM_WEAPON && q.GetCurrentItem()->GetType() != ITEM_ARMOR)
			{
				return 0;
			}
			lua_pushnumber(L, q.GetCurrentItem()->GetLevelLimit());
			return 1;
		}
		return 0;
	}

	ALUA(item_start_realtime_expire)
	{
		CQuestManager& q = CQuestManager::instance();
		LPITEM pItem = q.GetCurrentItem();

		if (pItem)
		{
			pItem->StartRealTimeExpireEvent();
			return 1;
		}

		return 0;
	}

	ALUA(item_copy_and_give_before_remove)
	{
		lua_pushboolean(L, 0);
		if (!lua_isnumber(L, 1))
			return 1;

		DWORD vnum = (DWORD)lua_tonumber(L, 1);

		CQuestManager& q = CQuestManager::instance();
		LPITEM pItem = q.GetCurrentItem();
		LPCHARACTER pChar = q.GetCurrentCharacterPtr();

		LPITEM pkNewItem = ITEM_MANAGER::instance().CreateItem(vnum, 1, 0, false);

		if (pkNewItem)
		{
			ITEM_MANAGER::CopyAllAttrTo(pItem, pkNewItem);
			WORD bCell = pItem->GetCell();

			ITEM_MANAGER::instance().RemoveItem(pItem, "REMOVE (COPY SUCCESS)");

			pkNewItem->__ADD_TO_CHARACTER(pChar, TItemPos(INVENTORY, bCell));
			ITEM_MANAGER::instance().FlushDelayedSave(pkNewItem);
			lua_pushboolean(L, 1);
		}

		return 1;
	}

	ALUA(item_get_wearflag0)
	{
		CQuestManager& q = CQuestManager::instance();
		LPITEM item = q.GetCurrentItem();

		if (item)
			lua_pushnumber(L, item->GetWearFlag());
		else
			lua_pushnumber(L, 0);

		return 1;
	}

	ALUA(item_has_wearflag0)
	{
		CQuestManager& q = CQuestManager::instance();
		LPITEM item = q.GetCurrentItem();

		if (!lua_isnumber(L, 1))
		{
			sys_err("wearflag is not a number.");
			lua_pushboolean(L, 0);
			return 1;
		}

		if (item)
			lua_pushboolean(L, IS_SET(item->GetWearFlag(), (long)lua_tonumber(L, 1)));
		else
			lua_pushboolean(L, false);

		return 1;
	}

	ALUA(item_get_antiflag0)
	{
		CQuestManager& q = CQuestManager::instance();
		LPITEM item = q.GetCurrentItem();

		if (item)
			lua_pushnumber(L, item->GetAntiFlag());
		else
			lua_pushnumber(L, 0);

		return 1;
	}

	ALUA(item_has_antiflag0)
	{
		CQuestManager& q = CQuestManager::instance();
		LPITEM item = q.GetCurrentItem();

		if (!lua_isnumber(L, 1))
		{
			sys_err("antiflag is not a number.");
			lua_pushboolean(L, false);
			return 1;
		}

		if (item)
			lua_pushboolean(L, IS_SET(item->GetAntiFlag(), static_cast<DWORD>(lua_tonumber(L, 1))));
		else
			lua_pushboolean(L, false);

		return 1;
	}

	ALUA(item_get_immuneflag0)
	{
		CQuestManager& q = CQuestManager::instance();
		LPITEM item = q.GetCurrentItem();

		if (item)
			lua_pushnumber(L, item->GetImmuneFlag());
		else
			lua_pushnumber(L, 0);

		return 1;
	}

	ALUA(item_has_immuneflag0)
	{
		CQuestManager& q = CQuestManager::instance();
		LPITEM item = q.GetCurrentItem();

		if (!lua_isnumber(L, 1))
		{
			sys_err("immuneflag is not a number.");
			lua_pushboolean(L, false);
			return 1;
		}

		if (item)
			lua_pushboolean(L, IS_SET(item->GetImmuneFlag(), static_cast<DWORD>(lua_tonumber(L, 1))));
		else
			lua_pushboolean(L, false);

		return 1;
	}

#define NS_ITEM_GETMODE0(x)	\
		int x = 0;\
		if(lua_isnumber(L, 1))\
			x = MINMAX(0, lua_tonumber(L, 1), 2);

	ALUA(item_add_attr0)
	{
		NS_ITEM_GETMODE0(m_mode);

		CQuestManager& q = CQuestManager::instance();
		LPITEM item = q.GetCurrentItem();

		if (item)
		{
			int m_count = 0;
			int m_reqsf = 1;
			if (lua_isnumber(L, 2))
				m_reqsf = lua_tonumber(L, 2);

			if (m_mode == 1 || m_mode == 0)
			{
				m_count = ITEM_ATTRIBUTE_NORM_NUM - item->GetAttributeCount();
				if (m_count > m_reqsf && m_reqsf != 0)
					m_count = m_reqsf;
				for (int i = 0; i < m_count; i++)
					item->AddAttribute();
			}
			if (m_mode == 2 || m_mode == 0)
			{
				m_count = ITEM_ATTRIBUTE_RARE_NUM - item->GetRareAttrCount();
				if (m_count > m_reqsf && m_reqsf != 0)
					m_count = m_reqsf;
				for (int i = 0; i < m_count; i++)
					item->AddRareAttribute();
			}
		}
		return 0;
	}

	ALUA(item_change_attr0)
	{
		NS_ITEM_GETMODE0(m_mode);

		CQuestManager& q = CQuestManager::instance();
		LPITEM item = q.GetCurrentItem();

		if (item)
		{
			if (m_mode == 0 || m_mode == 1)
				item->ChangeAttribute();
			if (m_mode == 0 || m_mode == 2)
				item->ChangeRareAttribute();
		}
		return 0;
	}

	ALUA(item_clear_attr0)
	{
		NS_ITEM_GETMODE0(m_mode);

		CQuestManager& q = CQuestManager::instance();
		LPITEM item = q.GetCurrentItem();

		int m_start = 0;
		int m_end = ITEM_ATTRIBUTE_MAX_NUM;

		if (m_mode == 1)
			m_end = ITEM_ATTRIBUTE_NORM_NUM;
		else if (m_mode == 2)
			m_start = ITEM_ATTRIBUTE_NORM_NUM;

		for (int i = m_start; i < m_end; i++)
			item->SetForceAttribute(i, 0, 0);
		return 0;
	}

	ALUA(item_count_attr0)
	{
		NS_ITEM_GETMODE0(m_mode);

		CQuestManager& q = CQuestManager::instance();
		LPITEM item = q.GetCurrentItem();

		if (item)
		{
			if (m_mode == 1)
				lua_pushnumber(L, item->GetAttributeCount());
			else if (m_mode == 2)
				lua_pushnumber(L, item->GetRareAttrCount());
			else //0
			{
				lua_newtable(L);
				{
					lua_pushnumber(L, item->GetAttributeCount());
					lua_rawseti(L, -2, 1);
				}
				{
					lua_pushnumber(L, item->GetRareAttrCount());
					lua_rawseti(L, -2, 2);
				}
			}
		}
		else
			lua_pushnumber(L, 0.0);

		return 1;
	}

	ALUA(item_get_attr0)
	{
		CQuestManager& q = CQuestManager::instance();
		LPITEM item = q.GetCurrentItem();

		TPlayerItemAttribute m_attr;
		if (item)
		{
			// it returns a table like:
			// {id, value, id, value, id, value, id, value, id, value, id, value, id, value}
			// es. {1, 1000, 2, 500, 73, 15, 23, 20, 0, 0, 71, 15, 72, 15}
			lua_newtable(L);
			for (int i = 0; i < ITEM_ATTRIBUTE_MAX_NUM; i++)
			{
				m_attr = item->GetAttribute(i);
				// push type
				lua_pushnumber(L, m_attr.bType);
				lua_rawseti(L, -2, (i * 2) + 1);
				// push value
				lua_pushnumber(L, m_attr.sValue);
				lua_rawseti(L, -2, (i * 2) + 2);
			}
		}
		else
			lua_pushnumber(L, 0.0);

		return 1;
	}

	ALUA(item_set_attr0)
	{
		if (!lua_istable(L, 1))
			return 0;

		CQuestManager& q = CQuestManager::instance();
		LPITEM item = q.GetCurrentItem();

		int m_attr[ITEM_ATTRIBUTE_MAX_NUM * 2] = { 0 };
		int m_idx = 0;
		// start
		lua_pushnil(L);
		while (lua_next(L, 1) && m_idx < (ITEM_ATTRIBUTE_MAX_NUM * 2))
		{
			m_attr[m_idx++] = lua_tonumber(L, -1);
			lua_pop(L, 1);
		}
		// end
		for (int i = 0; i < ITEM_ATTRIBUTE_MAX_NUM; i++)
			item->SetForceAttribute(i, m_attr[(i * 2) + 0], m_attr[(i * 2) + 1]);
		return 0;
	}

	ALUA(item_set_count0)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		LPITEM item = CQuestManager::instance().GetCurrentItem();

		if (!item || !ch)
		{
			sys_err("No item selected or no character instance wtf?!");
			return 0;
		}

		if (!lua_isnumber(L, 1))
		{
			sys_err("Invalid argument.");
			return 0;
		}

		int count = (int)lua_tonumber(L, 1);
		if (count > g_bItemCountLimit)
		{
			sys_err("Item count overflowing.. (%d)", count);
			return 0;
		}

		if (IS_SET(item->GetAntiFlag(), ITEM_ANTIFLAG_STACK) || !item->IsStackable())
			return 0;

		if (count > 0)
			item->SetCount(count);
		else
		{
			item->RemoveFromCharacter();
			M2_DESTROY_ITEM(item);
			//ITEM_MANAGER::instance().RemoveItem(item);
		}

		return 0;
	}

	ALUA(item_is_available0)
	{
		CQuestManager& q = CQuestManager::instance();
		LPITEM item = q.GetCurrentItem();

		lua_pushboolean(L, item != NULL);
		return 1;
	}

#ifdef __GROWTH_PET_SYSTEM__
	ALUA(item_pet_death)
	{
		CQuestManager& q = CQuestManager::instance();
		DWORD itemid = 0;
		if (q.GetCurrentItem())
		{
			itemid = q.GetCurrentItem()->GetID();
			char szQuery1[1024];
			snprintf(szQuery1, sizeof(szQuery1), "SELECT duration FROM new_petsystem WHERE id = %u LIMIT 1", itemid);
			std::unique_ptr<SQLMsg> pmsg2(DBManager::instance().DirectQuery(szQuery1));
			if (pmsg2->Get()->uiNumRows > 0) {
				MYSQL_ROW row = mysql_fetch_row(pmsg2->Get()->pSQLResult);
				lua_pushboolean(L, atoi(row[0]) <= 0);
				return 0;
			}
			else {
				lua_pushboolean(L, false);
				sys_err("[NewPetSystem]Error no item founded!On item.pet.death");
				return 0;
			}
		}
		return 0;
	}

	ALUA(item_pet_revive)
	{
		CQuestManager& q = CQuestManager::instance();
		DWORD itemid = 0;
		if (q.GetCurrentItem())
		{
			itemid = q.GetCurrentItem()->GetID();
			delete DBManager::instance().DirectQuery("UPDATE new_petsystem SET duration =(tduration/2) WHERE id = %u", itemid);
		}
		return 0;
	}
#endif

#ifdef __ITEM_CHANGELOOK__
	ALUA(item_is_transmulated)
	{
		CQuestManager& qMgr = CQuestManager::instance();
		LPITEM pkItem = qMgr.GetCurrentItem();
		if (pkItem)
		{
			if (pkItem->GetTransmutation() > 0)
				lua_pushboolean(L, true);
			else
				lua_pushboolean(L, false);
		}
		else
			lua_pushboolean(L, false);

		return 1;
	}

	ALUA(item_set_transmutation)
	{
		CQuestManager& qMgr = CQuestManager::instance();
		LPITEM pkItem = qMgr.GetCurrentItem();
		if ((pkItem) && (lua_isnumber(L, 1)))
		{
			DWORD dwTransmutation = (DWORD)lua_tonumber(L, 1);
			pkItem->SetTransmutation(dwTransmutation);
		}

		return 0;
	}

	ALUA(item_get_transmutation)
	{
		CQuestManager& qMgr = CQuestManager::instance();
		LPITEM pkItem = qMgr.GetCurrentItem();
		if (pkItem)
			lua_pushnumber(L, pkItem->GetTransmutation());
		else
			lua_pushnumber(L, 0);

		return 1;
	}
#endif

	ALUA(item_set_attribute)
	{
		LPITEM item = CQuestManager::instance().GetCurrentItem();

		if (!item) return 0;

		if (!lua_isnumber(L, 1))
		{
			sys_err("Wrong argument[AttrIdx] #1.");
			lua_pushboolean(L, false);
			return 1;
		}
		else if (!lua_isnumber(L, 2))
		{
			sys_err("Wrong argument[AttrType] #2.");
			lua_pushboolean(L, false);
			return 1;
		}
		else if (!lua_isnumber(L, 3))
		{
			sys_err("Wrong argument[AttrValue] #3.");
			lua_pushboolean(L, false);
			return 1;
		}

		int bAttrIndex = (int)lua_tonumber(L, 1);
		if (bAttrIndex < 0 || bAttrIndex >= ITEM_ATTRIBUTE_MAX_NUM - 2)
		{
			sys_err("Invalid AttrIndex %d. AttrIndex out of range(0..4)", bAttrIndex);
			lua_pushboolean(L, false);
			return 1;
		}

		int bAttrType = (int)lua_tonumber(L, 2);
		if (bAttrType < 1 || bAttrType >= MAX_APPLY_NUM)
		{
			sys_err("Invalid AttrType %d. AttrType out of range(1..%d)", MAX_APPLY_NUM);
			lua_pushboolean(L, false);
			return 1;
		}

		if (item->HasAttr(bAttrType) && (item->GetAttribute(bAttrIndex).bType != bAttrType))
		{
			sys_err("AttrType[%d] multiplicated.", bAttrType);
			lua_pushboolean(L, false);
			return 1;
		}

		int bAttrValue = (int)lua_tonumber(L, 3);
		if (bAttrValue < 1 || bAttrValue >= 32768)
		{
			sys_err("Invalid AttrValue %d. AttrValue should be between 1 and 32767!", bAttrValue);
			lua_pushboolean(L, false);
			return 1;
		}

		bool bRet = TRUE;
		int bAttrCount = item->GetAttributeCount();
		if (bAttrCount <= 4 && bAttrCount >= 0)
		{
			if (bAttrCount < bAttrIndex)
				bAttrIndex = bAttrCount;

			item->SetForceAttribute(bAttrIndex, bAttrType, bAttrValue);
		}
		else
			bRet = FALSE;

		lua_pushboolean(L, bRet);
		return 1;
	}
#ifdef __BEGINNER_ITEM__
	ALUA(item_is_basic)
	{
		CQuestManager& q = CQuestManager::instance();

		if (q.GetCurrentItem())
		{
			lua_pushnumber(L, q.GetCurrentItem()->IsBasicItem());
		}
		else
			lua_pushnumber(L, 0);
		return 1;
	}

	ALUA(item_set_basic)
	{
		CQuestManager& q = CQuestManager::instance();
		LPITEM item = q.GetCurrentItem();
		bool is_basic = lua_toboolean(L, 1);
		if (item)
			item->SetBasic(is_basic);
		return 0;
	}
#endif
	void RegisterITEMFunctionTable()
	{
		luaL_reg item_functions[] =
		{
			{ "get_id",		item_get_id		},
			{ "get_cell",		item_get_cell		},
			{ "select",		item_select		},
			{ "select_cell",	item_select_cell	},
			{ "remove",		item_remove		},
			{ "get_socket",		item_get_socket		},
			{ "set_socket",		item_set_socket		},
			{ "get_vnum",		item_get_vnum		},
			{ "has_flag",		item_has_flag		},
			{ "get_value",		item_get_value		},
			{ "set_value",		item_set_value		},
			{ "get_name",		item_get_name		},
			{ "get_size",		item_get_size		},
			{ "get_count",		item_get_count		},
			{ "get_type",		item_get_type		},
			{ "get_sub_type",	item_get_sub_type	},
			{ "get_refine_vnum",	item_get_refine_vnum	},
			{ "get_level",		item_get_level		},
			{ "next_refine_vnum",	item_next_refine_vnum	},
			{ "get_level_limit", 				item_get_level_limit },
			{ "start_realtime_expire", 			item_start_realtime_expire },
			{ "copy_and_give_before_remove",	item_copy_and_give_before_remove},
			{ "get_wearflag0",			item_get_wearflag0},	// [return lua number]
			{ "has_wearflag0",			item_has_wearflag0},	// [return lua boolean]
			{ "get_antiflag0",			item_get_antiflag0},	// [return lua number]
			{ "has_antiflag0",			item_has_antiflag0},	// [return lua boolean]
			{ "get_immuneflag0",		item_get_immuneflag0},	// [return lua number]
			{ "has_immuneflag0",		item_has_immuneflag0},	// [return lua boolean]
			// item.add_attr0(0|1|2[, cnt]) -- (0: baseeraro, 1: base, 2: raro)
			// item.add_attr0(0) -- add one 1-5 and one 6-7 bonus
			// item.add_attr0(0, 0) -- add all 1-7 bonuses
			// item.add_attr0(1|2) -- add one 1-5|6-7 bonus
			// item.add_attr0(1|2, 0) -- add all 1-5|6-7 bonuses
			// item.add_attr0(1|2, 4) -- add four 1-5|6-7 bonuses
			{ "add_attr0",			item_add_attr0},
			// item.change_attr0(0|1|2) -- (0: baseerari, 1: base, 2: rari)
			{ "change_attr0",		item_change_attr0},
			// item.clear_attr0(0|1|2) -- (0: baseerari, 1: base, 2: rari)
			{ "clear_attr0",		item_clear_attr0},
			// item.count_attr0(0|1|2) -- (0: [cnt(base), cnt(rari)], 1: cnt(base), 2: cnt(rari))
			{ "count_attr0",		item_count_attr0},
			// item.get_attr0() -- return a table containing all the item attrs {1,11,2,22,...,7,77}
			{ "get_attr0",			item_get_attr0},	// [return lua table]
			// item.set_attr0({1,11,2,22,...,7,77}) use a table to set the item attrs
			{ "set_attr0",			item_set_attr0},	// [return nothing]
			// item.set_count(count)
			{ "set_count0",			item_set_count0},	// [return nothing]
			{ "set_count",			item_set_count0},
			// { "equip_to0",			item_equip_to0},	// [return lua boolean=successfulness]
			// { "unequip0",			item_unequip0},		// [return lua boolean=successfulness]
			{ "is_available0",		item_is_available0	},	// [return lua boolean]
#ifdef __GROWTH_PET_SYSTEM__
			{ "petdeath",						item_pet_death},
			{ "petrevive",						item_pet_revive},
#endif
#ifdef __ITEM_CHANGELOOK__
			{"is_transmulated", item_is_transmulated},
			{"set_transmutation", item_set_transmutation},
			{"get_transmutation", item_get_transmutation},
#endif
			{"set_attribute", item_set_attribute},
#ifdef __BEGINNER_ITEM__
			{"is_basic", item_is_basic},
			{"set_basic", item_set_basic},
#endif
			{ NULL,			NULL			}
		};
		CQuestManager::instance().AddLuaFunctionTable("item", item_functions);
	}
}