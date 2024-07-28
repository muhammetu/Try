#include "stdafx.h"
#include "../../common/tables.h"
#include "packet.h"
#include "item.h"
#include "char.h"
#include "item_manager.h"
#include "desc.h"
#include "char_manager.h"
#include "DragonSoul.h"
#include "buffer_manager.h"
#include "desc_client.h"
#include "config.h"
#include "event.h"
#include "locale_service.h"
#include <fstream>

#include "sectree_manager.h"
#include "sectree.h"
#include "config.h"

#include "new_offlineshop.h"
#include "new_offlineshop_manager.h"

#include <offlineshop/Offlineshop.h>
#include "../../libgame/include/grid.h"

#include "db.h"
#include "questmanager.h"
#include "log.h"

#include <boost/unordered_set.hpp>

DWORD s_MaxOffShopCount = 7000;

#define IS_CONTAIN(collection, value) (collection.find(value) != collection.end())

#ifdef __OFFLINE_SHOP__
void PositionHelper(long mapIndex, long* x, long* y)
{
	if (!mapIndex)
	{
		sys_log(0, "OpenMyShop: invalid mapindex for char %d", mapIndex);
		return;
	}
	long curX = *x;
	long curY = *y;

	int deltaX = 0;
	int deltaY = 0;

	{
		// Create the grid
		int gridSide = 33;
		assert(gridSide % 2 != 0); //Must be odd!
		CGrid grid(gridSide, gridSide);

		int gridToCoordRatio = 70 * 5 / 2; // One grid cell will equal to a N units area in the world. 70 is the minimum
		int centerPos = (gridSide + 1) / 2 * (gridSide - 1); // Center position of the grid
		int centerPosColumn = grid.GetPosColumn(centerPos);
		int centerPosRow = grid.GetPosRow(centerPos);

		int coordVar = (gridSide - 1) / 2 + 1; // Coordinate variance (considering we are assuming the centerPos is our current position).

		// Inspect terrain
		LPSECTREE tree;
		for (long col = 0; col < gridSide; ++col)
		{
			for (long row = 0; row < gridSide; ++row)
			{
				int x = (col - coordVar) * gridToCoordRatio + curX;
				int y = (row - coordVar) * gridToCoordRatio + curY;
				tree = SECTREE_MANAGER::instance().Get(mapIndex, x, y);
				if (!tree)
					continue;

				if (tree->IsAttr(x, y, ATTR_BLOCK | ATTR_OBJECT | ATTR_WATER))
					grid.Put(col + row * gridSide, 1, 1);
			}
		}

		// Add shops to the grid
		CharacterVectorInteractor cvi;
		if (CHARACTER_MANAGER::instance().GetCharactersByRaceNum(30000, cvi))
		{
			for (const auto shopChar : cvi)
			{
				long shopX = shopChar->GetX();
				long shopY = shopChar->GetY();

				// Is shop within our grid?
				if (shopX >= curX - coordVar * gridToCoordRatio && shopX <= curX + coordVar * gridToCoordRatio &&
					shopY >= curY - coordVar * gridToCoordRatio && shopY <= curY + coordVar * gridToCoordRatio)
				{
					int col = (shopX - curX) / gridToCoordRatio + centerPosColumn;
					int row = (shopY - curY) / gridToCoordRatio + centerPosRow;

					int pos = col + row * gridSide;
					grid.Put(pos, 1, 1);
				}
			}
		}

		// Grid is created, let's see where we place our new shop.
		bool placed = false;

		// First stick to the position we are at if possible (center position of the grid)

		if (grid.IsEmpty(centerPos, 1, 1))
		{
			placed = true;
			grid.Put(centerPos, 1, 1);
		}
		// Otherwise, find a blank position in the grid, closest to the center
		else
		{
			int bestDist = -1;
			int bestPos = -1;

			for (int col = 0; col < gridSide; ++col)
			{
				for (int row = 0; row < gridSide; ++row)
				{
					int pos = row * gridSide + col;
					if (grid.IsEmpty(pos, 1, 1))
					{
						int dist = abs(row - centerPosRow) + abs(col - centerPosColumn);
						if (bestDist < 0 || bestDist > dist)
						{
							bestDist = dist;
							bestPos = pos;
						}
					}
				}
			}

			if (bestPos > -1)
			{
				placed = true;
				deltaX = (grid.GetPosColumn(bestPos) - centerPosColumn) * gridToCoordRatio;
				deltaY = (grid.GetPosRow(bestPos) - centerPosRow) * gridToCoordRatio;

				grid.Put(bestPos, 1, 1);
			}
		}

		if (!placed)
		{
			OFFSHOP_DEBUG("Shop could not placed at coords (%ld, %ld) - Mapindex %ld, channel %d", curX, curY, mapIndex, g_bChannel);
		}
		else
		{
			OFFSHOP_DEBUG("Shop placed around (%ld, %ld) with an offset of (%d, %d)", curX, curY, deltaX, deltaY);
		}

		/*if (test_server)
			grid.Print();*/
	}

	//Reposition shop if needed
	if (deltaX != 0 || deltaY != 0)
	{
		*x = curX + deltaX;
		*y = curY + deltaY;
	}
}

std::string StringToLower(const char* name, size_t len)
{
	std::string res;
	res.resize(len);

	for (size_t i = 0; i < len; i++)
		res[i] = tolower(*(name + i));
	return res;
}

//topatch

//updated 25-01-2020
bool IsGoodSalePrice(const offlineshop::TPriceInfo price) {
	if (price.illYang <= 0 || price.illYang >= GOLD_MAX) {
		return false;
	}

#ifdef __CHEQUE_SYSTEM__
	else if (price.iCheque >= CHEQUE_MAX) {
		return false;
	}
#endif

	else {
		return true;
	}
}

bool MatchItemName(WORD nameMode, std::string stName, const char* table, const size_t tablelen)
{
	/*
	std::string stTable(table, tablelen) , stName(name, namelen);

	//checking about refinegrade into tablename
	size_t refineGrade = stTable.find('+');
	if(refineGrade != std::string::npos && refineGrade > stTable.length() - 4)
		stTable = stTable.substr(0, refineGrade);

	//checking about refinegrade into itemname
	refineGrade = stName.find('+');
	if(refineGrade != std::string::npos && refineGrade > stName.length() - 4)
		stName = stName.substr(0, refineGrade);

	return strncasecmp(stName.c_str() , stTable.c_str() , stName.length()) == 0;
	*/

	std::string stTable = StringToLower(table, tablelen);

	switch (nameMode)
	{
		case 0:
			return stTable.compare(stName) == 0;

		case 1:
			return stTable.find(stName) != std::string::npos;
	}

	return false;
}

long FindApplyValue(const TPlayerItemAttribute* pAttr, BYTE bApplyType)
{
	for (int i = 0; i < ITEM_ATTRIBUTE_MAX_NUM; ++i)
	{
		if (pAttr[i].bType == bApplyType)
			return pAttr[i].sValue;
	}

	return 0;
}

BYTE GetDSGradeIdx(DWORD dwVnum)
{
	return (dwVnum / 1000) % 10;
}

BYTE GetDSStepIdx(DWORD dwVnum)
{
	return (dwVnum / 100) % 10;
}

BYTE GetDSLevelIdx(DWORD dwVnum)
{
	return (dwVnum / 10) % 10;
}

bool MatchSearchCategory(TItemTable* pTable, BYTE category, BYTE subcategory)
{
	if (category == offlineshop::SEARCH_CATEGORY_ALL)
		return true;

	switch (category)
	{
		case offlineshop::SEARCH_CATEGORY_ALL:
			return true;

		case offlineshop::SEARCH_CATEGORY_PREMIUM:
			return false;

		case offlineshop::SEARCH_CATEGORY_WEAPON:
			{
				if (pTable->bType != ITEM_WEAPON)
					return false;

				switch (subcategory)
				{
					case 0:
						return pTable->bSubType == WEAPON_SWORD;
					case 1:
						return pTable->bSubType == WEAPON_TWO_HANDED;
					case 2:
						return pTable->bSubType == WEAPON_BOW;
					case 3:
						return pTable->bSubType == WEAPON_DAGGER;
					case 4:
						return pTable->bSubType == WEAPON_BELL;
					case 5:
						return pTable->bSubType == WEAPON_FAN;
					case 6:
						return pTable->bSubType == WEAPON_CLAW;
					case 255:
						{
							switch (pTable->bSubType)
							{
							case WEAPON_SWORD:
							case WEAPON_TWO_HANDED:
							case WEAPON_BOW:
							case WEAPON_DAGGER:
							case WEAPON_BELL:
							case WEAPON_FAN:
							case WEAPON_CLAW:
								return true;
							}
						}

					default:
						return false;
				}

				return false;
			}
			break;

		case offlineshop::SEARCH_CATEGORY_ARMOR:
			{
				if (pTable->bType == ITEM_ARMOR)
				{
					switch (subcategory)
					{
					case 0:
						return pTable->bSubType == ARMOR_BODY;
					case 1:
						return pTable->bSubType == ARMOR_HEAD;
					case 2:
						return pTable->bSubType == ARMOR_SHIELD;
					case 3:
						return pTable->bSubType == ARMOR_EAR;
					case 4:
						return pTable->bSubType == ARMOR_WRIST;
					case 5:
						return pTable->bSubType == ARMOR_NECK;
					case 6:
						return pTable->bSubType == ARMOR_FOOTS;
					case 255:
						{
							switch (pTable->bSubType)
							{
							case ARMOR_BODY:
							case ARMOR_HEAD:
							case ARMOR_SHIELD:
							case ARMOR_EAR:
							case ARMOR_WRIST:
							case ARMOR_NECK:
							case ARMOR_FOOTS:
								return true;
							}
						}
					}
				}
				else if (pTable->bType == ITEM_BELT)
					return subcategory == 7;

				return false;
			}
			break;

		case offlineshop::SEARCH_CATEGORY_ACCESSORY:
			{
				static boost::unordered_set<DWORD> list = { 39032, 70071, 70072, 70073, 70074, 99403, 99404, 99405, 99406, 99406, 99406, 99407, 99408 };

				switch (subcategory)
				{
					case 0:
						return (pTable->dwVnum == 99496);
					case 1:
						return (pTable->dwVnum == 25104);
					case 2:
						return (pTable->dwVnum == 99414 || pTable->dwVnum == 99495);
					case 3:
						return (pTable->dwVnum == 99497 || pTable->dwVnum == 99415);
					case 4:
						return (pTable->dwVnum == 99413 || pTable->dwVnum == 99490 || pTable->dwVnum == 99491);
					case 5:
						{
							return IS_CONTAIN(list, pTable->dwVnum);
						}
						break;
					case 255:
						{
							switch (pTable->dwVnum)
							{
							case 99496:
							case 25104:
							case 99414:
							case 99495:
							case 99497:
							case 99415:
							case 99413:
							case 99490:
							case 99491:
								return true;
							}

							if (IS_CONTAIN(list, pTable->dwVnum)) return true;
						}
				}

				return false;
			}
			break;

		case offlineshop::SEARCH_CATEGORY_PENDANT:
		{
			if (!(pTable->bType == ITEM_ARMOR && pTable->bSubType == ARMOR_PENDANT))
				return false;

			switch (subcategory)
			{
				case 0:
					return (pTable->dwVnum >= 9600 && pTable->dwVnum <= 9800);
				case 1:
					return (pTable->dwVnum >= 9830 && pTable->dwVnum <= 10030);
				case 2:
					return (pTable->dwVnum >= 10060 && pTable->dwVnum <= 10260);
				case 3:
					return (pTable->dwVnum >= 10290 && pTable->dwVnum <= 10490);
				case 4:
					return (pTable->dwVnum >= 10520 && pTable->dwVnum <= 10720);
				case 5:
					return (pTable->dwVnum >= 10750 && pTable->dwVnum <= 10950);
				case 6:
					{
						if (pTable->bType == ITEM_USE)
						{
							switch (pTable->bSubType)
							{
								case USE_ADD_PENDANT_ATTRIBUTE:
								case USE_ADD_PENDANT_FIVE_ATTRIBUTE:
								case USE_CHANGE_PENDANT_ATTRIBUTE:
									return true;
							}
						}
					}
					break;

				case 255:
					{
						if (pTable->dwVnum >= 9600 && pTable->dwVnum <= 9800) return true;
						if (pTable->dwVnum >= 9830 && pTable->dwVnum <= 10030) return true;
						if (pTable->dwVnum >= 10060 && pTable->dwVnum <= 10260) return true;
						if (pTable->dwVnum >= 10290 && pTable->dwVnum <= 10490) return true;
						if (pTable->dwVnum >= 10520 && pTable->dwVnum <= 10720) return true;
						if (pTable->dwVnum >= 10750 && pTable->dwVnum <= 10950) return true;
						if (pTable->bType == ITEM_USE)
						{
							switch (pTable->bSubType)
							{
								case USE_ADD_PENDANT_ATTRIBUTE:
								case USE_ADD_PENDANT_FIVE_ATTRIBUTE:
								case USE_CHANGE_PENDANT_ATTRIBUTE:
									return true;
							}
						}
					}
			}

			return false;
		}

		case offlineshop::SEARCH_CATEGORY_DSS:
			{
				boost::unordered_set<DWORD> list = { 80039, 80040, 100300, 100400, 100500, 100000, 100001, 100002, 100003, 100004 };
				switch (subcategory)
				{
					case 0:
						return (pTable->bType == ITEM_DS && pTable->bSubType == DS_SLOT1);
					case 1:
						return (pTable->bType == ITEM_DS && pTable->bSubType == DS_SLOT2);
					case 2:
						return (pTable->bType == ITEM_DS && pTable->bSubType == DS_SLOT3);
					case 3:
						return (pTable->bType == ITEM_DS && pTable->bSubType == DS_SLOT4);
					case 4:
						return (pTable->bType == ITEM_DS && pTable->bSubType == DS_SLOT5);
					case 5:
						return (pTable->bType == ITEM_DS && pTable->bSubType == DS_SLOT6);
					case 6:
						{
							return IS_CONTAIN(list, pTable->dwVnum);
						}
						break;
					case 255:
					{
						if (pTable->bType == ITEM_DS && pTable->bSubType == DS_SLOT1) return true;
						if (pTable->bType == ITEM_DS && pTable->bSubType == DS_SLOT2) return true;
						if (pTable->bType == ITEM_DS && pTable->bSubType == DS_SLOT3) return true;
						if (pTable->bType == ITEM_DS && pTable->bSubType == DS_SLOT4) return true;
						if (pTable->bType == ITEM_DS && pTable->bSubType == DS_SLOT5) return true;
						if (pTable->bType == ITEM_DS && pTable->bSubType == DS_SLOT6) return true;
						if (IS_CONTAIN(list, pTable->dwVnum)) return true;
					}
				}

				return false;
			}
			break;

		case offlineshop::SEARCH_CATEGORY_COSTUME:
			{
				static boost::unordered_set<DWORD> list2 = { 71158, 96000, 96040, 96036, 96046, 57002, 57006, 57010, 57014, 57018 };
				static boost::unordered_set<DWORD> list3 = { 71202, 96032, 96043, 96037, 96047, 57003, 57007, 57011, 57015, 57019 };
				static boost::unordered_set<DWORD> list4 = { 71135, 96031, 96042, 96038, 96048, 57004, 57008, 57012, 57016, 57020 };
				static boost::unordered_set<DWORD> list5 = { 72054, 96001, 96041, 96035, 96045, 57001, 57005, 57009, 57013, 57017 };
				switch (subcategory)
				{
					case 0:
						return (pTable->bType == ITEM_COSTUME && pTable->bSubType == COSTUME_HAIR);
					case 1:
						return (pTable->bType == ITEM_COSTUME && pTable->bSubType == COSTUME_WEAPON);
					case 2:
						{
							return IS_CONTAIN(list2, pTable->dwVnum);
						}
						break;
					case 3:
						{
							return IS_CONTAIN(list3, pTable->dwVnum);
						}
						break;
					case 4:
						{
							return IS_CONTAIN(list4, pTable->dwVnum);
						}
						break;
					case 5:
						{
							return IS_CONTAIN(list5, pTable->dwVnum);
						}
						break;
					case 6:
						return (pTable->bType == ITEM_SHINING && pTable->bSubType == SHINING_WING);
					case 255:
						{
							if (pTable->bType == ITEM_COSTUME && pTable->bSubType == COSTUME_HAIR) return true;
							if (pTable->bType == ITEM_COSTUME && pTable->bSubType == COSTUME_WEAPON) return true;
							if (IS_CONTAIN(list2, pTable->dwVnum)) return true;
							if (IS_CONTAIN(list3, pTable->dwVnum)) return true;
							if (IS_CONTAIN(list4, pTable->dwVnum)) return true;
							if (IS_CONTAIN(list5, pTable->dwVnum)) return true;
						}
				}

				return false;
			}
			break;

		case offlineshop::SEARCH_CATEGORY_ACCE:
			{
				static boost::unordered_set<DWORD> list = { 71135, 96031, 96042, 96038, 96048, 57004, 57008, 57012, 57016 };
				switch (subcategory)
				{
					case 0:
						return (pTable->bType == ITEM_COSTUME && pTable->bSubType == COSTUME_ACCE);
					case 1:
						{
							return IS_CONTAIN(list, pTable->dwVnum);
						}
						break;
					case 255:
						{
							if (pTable->bType == ITEM_COSTUME && pTable->bSubType == COSTUME_ACCE) return true;
							if (IS_CONTAIN(list, pTable->dwVnum)) return true;
						}
				}

				return false;
			}
			break;

		case offlineshop::SEARCH_CATEGORY_AURA:
			{
				static boost::unordered_set<DWORD> list0 = { 70063, 70064, 30544 };
				static boost::unordered_set<DWORD> list1 = { 30171, 30172, 30543 };
				static boost::unordered_set<DWORD> list2 = { 30097, 30098, 30541 };
				static boost::unordered_set<DWORD> list3 = { 30093, 30094, 30540 };
				static boost::unordered_set<DWORD> list4 = { 30595, 30596, 30597 };
				static boost::unordered_set<DWORD> list5 = { 30173, 30174, 30545 };
				static boost::unordered_set<DWORD> list6 = { 71130, 71095, 71091 };
				static boost::unordered_set<DWORD> list7 = { 99424, 99423, 99425 };
				static boost::unordered_set<DWORD> list8 = { 30548, 30547, 30546 };
				static boost::unordered_set<DWORD> list9 = { 826, 827, 99426 };
				static boost::unordered_set<DWORD> list10 = { 30099, 30100, 30542 };
				static boost::unordered_set<DWORD> list11 = { 71086, 71087, 99421, 56500 };
				static boost::unordered_set<DWORD> list12 = { 71051, 71052 };

				switch (subcategory)
				{
					case 0:
						{
							return IS_CONTAIN(list0, pTable->dwVnum);
						}
						break;

					case 1:
						{
							return IS_CONTAIN(list1, pTable->dwVnum);
						}
						break;

					case 2:
						{
							return IS_CONTAIN(list2, pTable->dwVnum);
						}
						break;

					case 3:
						{
							return IS_CONTAIN(list3, pTable->dwVnum);
						}
						break;

					case 4:
						{
							return IS_CONTAIN(list4, pTable->dwVnum);
						}
						break;

					case 5:
						{
							return IS_CONTAIN(list5, pTable->dwVnum);
						}
						break;

					case 6:
						{
							return IS_CONTAIN(list6, pTable->dwVnum);
						}
						break;

					case 7:
						{
							return IS_CONTAIN(list7, pTable->dwVnum);
						}
						break;

					case 8:
						{
							return IS_CONTAIN(list8, pTable->dwVnum);
						}
						break;

					case 9:
						{
							return IS_CONTAIN(list9, pTable->dwVnum);
						}
						break;

					case 10:
						{
							return IS_CONTAIN(list10, pTable->dwVnum);
						}
						break;

					case 11:
						{
							return IS_CONTAIN(list11, pTable->dwVnum);
						}
						break;

					case 12:
						{
							return IS_CONTAIN(list12, pTable->dwVnum);
						}
						break;

					case 13:
						{
							if (pTable->dwVnum == 99427 || pTable->dwVnum == 30500 || pTable->dwVnum == 30521)
								return true;
						}
						break;

					case 255:
						{
							if (IS_CONTAIN(list0, pTable->dwVnum)) return true;
							if (IS_CONTAIN(list1, pTable->dwVnum)) return true;
							if (IS_CONTAIN(list2, pTable->dwVnum)) return true;
							if (IS_CONTAIN(list3, pTable->dwVnum)) return true;
							if (IS_CONTAIN(list4, pTable->dwVnum)) return true;
							if (IS_CONTAIN(list5, pTable->dwVnum)) return true;
							if (IS_CONTAIN(list6, pTable->dwVnum)) return true;
							if (IS_CONTAIN(list7, pTable->dwVnum)) return true;
							if (IS_CONTAIN(list8, pTable->dwVnum)) return true;
							if (IS_CONTAIN(list9, pTable->dwVnum)) return true;
							if (IS_CONTAIN(list10, pTable->dwVnum)) return true;
							if (IS_CONTAIN(list11, pTable->dwVnum)) return true;
							if (IS_CONTAIN(list12, pTable->dwVnum)) return true;
						}
						break;
				}

				return false;
			}
			break;

		case offlineshop::SEARCH_CATEGORY_SKILL:
			{
				if (!(pTable->bType == ITEM_SKILLBOOK || pTable->bType == ITEM_SKILLFORGET))
					return false;
				
				static boost::unordered_set<DWORD> warrior = { 50401, 50402, 50403, 50404, 50405, 50406, 50416, 50417, 50418, 50419, 50420, 50421 };
				static boost::unordered_set<DWORD> assassin = { 50431, 50432, 50433, 50434, 50435, 50436, 50446, 50447, 50448, 50449, 50450, 50451 };
				static boost::unordered_set<DWORD> sura = { 50461, 50462, 50463, 50464, 50465, 50466, 50476, 50477, 50478, 50479, 50480, 50481 };
				static boost::unordered_set<DWORD> shaman = { 50491, 50492, 50493, 50494, 50495, 50496, 50506, 50507, 50508, 50509, 50510, 50511 };

				static boost::unordered_set<DWORD> list0 = { 50301, 50302, 50303, 50327, 50334, 50326, 50328, 50329 };
				static boost::unordered_set<DWORD> list1 = { 55010, 55011, 55012, 55013, 55014, 55015, 55016 };
				static boost::unordered_set<DWORD> list2 = { 50513, 99430, 99850, 99854, 99410, 70102 };
				static boost::unordered_set<DWORD> list3 = { 71094, 99411, 99431, 99051, 99055 };
				static boost::unordered_set<DWORD> list4 = { 71001, 99412, 99432, 99052, 99056 };

				switch (subcategory)
				{
					case 0:
						{
							return IS_CONTAIN(warrior, pTable->dwVnum);
						}
						break;

					case 1:
						{
							return IS_CONTAIN(assassin, pTable->dwVnum);
						}
						break;

					case 2:
						{
							return IS_CONTAIN(sura, pTable->dwVnum);
						}
						break;
					
					case 3:
						{
							return IS_CONTAIN(shaman, pTable->dwVnum);
						}
						break;

					case 4:
						{
							return IS_CONTAIN(list0, pTable->dwVnum);
						}
						break;

					case 5:
						{
							return IS_CONTAIN(list1, pTable->dwVnum);
						}
						break;

					case 6:
						{
							return IS_CONTAIN(list2, pTable->dwVnum);
						}
						break;

					case 7:
						{
							return IS_CONTAIN(list3, pTable->dwVnum);
						}
						break;
					
					case 8:
						{
							return IS_CONTAIN(list4, pTable->dwVnum);
						}
						break;

					case 255:
						{
							if (IS_CONTAIN(warrior, pTable->dwVnum)) return true;
							if (IS_CONTAIN(assassin, pTable->dwVnum)) return true;
							if (IS_CONTAIN(sura, pTable->dwVnum)) return true;
							if (IS_CONTAIN(shaman, pTable->dwVnum)) return true;
							if (IS_CONTAIN(list0, pTable->dwVnum)) return true;
							if (IS_CONTAIN(list1, pTable->dwVnum)) return true;
							if (IS_CONTAIN(list2, pTable->dwVnum)) return true;
							if (IS_CONTAIN(list3, pTable->dwVnum)) return true;
						}
						break;
				}

				return false;
			}
			break;

		case offlineshop::SEARCH_CATEGORY_EQUIPABLE:
			{
				if (!(pTable->bType == ITEM_SHINING || pTable->bType == ITEM_CAKRA))
					return false;

				switch (subcategory)
				{
					case 0:
						return (pTable->bType == ITEM_SHINING && pTable->bSubType == SHINING_WEAPON);
					case 1:
						return (pTable->bType == ITEM_SHINING && pTable->bSubType == SHINING_ARMOR);
					case 2:
						return (pTable->bType == ITEM_SHINING && pTable->bSubType == SHINING_SPECIAL);
					case 3:
						return (pTable->bType == ITEM_SHINING && pTable->bSubType == SHINING_SPECIAL2);
					case 4:
						return (pTable->bType == ITEM_SHINING && pTable->bSubType == SHINING_SPECIAL3);
					case 5:
						return pTable->bType == ITEM_CAKRA;
					case 6:
						return (pTable->bType == ITEM_COSTUME && pTable->bSubType == COSTUME_AURA);
					case 255:
						{
							return true;
						}
						break;
				}

				return false;
			}
			break;

		case offlineshop::SEARCH_CATEGORY_POTION:
			{
				static boost::unordered_set<DWORD> list = { 57040, 57041, 57042, 57043, 57044, 57045, 57046, 57047, 57048, 57049, 57050, 57051, 57052, 57053, 57054, 57055, 57056, 57057, 57058, 57059, 57060, 57061, 57062, 57063, 57064, 57065, 57066, 57067, 57068, 57069, 57070, 57071, 57072, 57073, 57074, 57075, 57076, 57077, 57078, 57079, 57080, 57081, 57082, 57083, 57084, 57085, 57086, 57087, 57088, 57089, 57090, 57091, 57092, 57093, 57094, 57095, 57096, 57097, 57098, 57099, 50950, 50951, 50952, 50955, 50956, 50957, 50960, 50961, 50962, 50965, 50966, 50967, 50970, 50971, 50972, 50975, 50976, 50977, 50980, 50981, 50982, 50985, 50986, 50987, 99355, 99356, 99357, 99358, 99359, 99360, 99361, 99362, 99363, 99364, 99365, 99366, 99367, 99368, 99369, 99370, 99371, 99372, 99373, 99374, 99375, 99376, 99377 };
				return IS_CONTAIN(list, pTable->dwVnum);
			}
			break;

		case offlineshop::SEARCH_CATEGORY_PET:
			{
				switch (subcategory)
				{
					case 0:
						return (pTable->bType == ITEM_UNIQUE && pTable->bSubType == USE_PET);
					case 1:
						return (pTable->dwVnum == 55002) || (pTable->dwVnum >= 55401 && pTable->dwVnum <= 55410);
					case 2:
						return (pTable->bType == ITEM_COSTUME && pTable->bSubType == COSTUME_MOUNT);
					case 3:
						return (pTable->dwVnum == 55032 || pTable->dwVnum == 55033);
					case 4:
						return (pTable->dwVnum == 801 || pTable->dwVnum == 803);
					case 255:
						{
							if ((pTable->bType == ITEM_UNIQUE && pTable->bSubType == USE_PET)) return true;
							if ((pTable->dwVnum == 55002) || (pTable->dwVnum >= 55401 && pTable->dwVnum <= 55410)) return true;
							if (pTable->bType == ITEM_COSTUME && pTable->bSubType == COSTUME_MOUNT) return true;
							if (pTable->dwVnum == 55042 || pTable->dwVnum == 55043) return true;
						}
				}

				return false;
			}
			break;

		case offlineshop::SEARCH_CATEGORY_UPGRADE:
			{
				switch (subcategory)
				{
					case 0:
						{
							return (pTable->dwVnum == 98083 || pTable->dwVnum == 98073 || pTable->dwVnum == 98074 || pTable->dwVnum == 98087 || pTable->dwVnum == 98091 || pTable->dwVnum == 98076);
						}
						break;

					case 1:
						return pTable->bType == ITEM_MATERIAL;

					case 255:
					{
						if (pTable->dwVnum == 98083 || pTable->dwVnum == 98073 || pTable->dwVnum == 98074 || pTable->dwVnum == 98087 || pTable->dwVnum == 98091 || pTable->dwVnum == 98076) return true;
						if (pTable->bType == ITEM_MATERIAL) return true;
					}
				}

				return false;
			}
			break;
	}

	return false;
}

bool CheckCharacterActions(LPCHARACTER ch)
{
	if (!ch)
		return false;

	const int iPulse = thecore_pulse();
	const int limit_time = PASSES_PER_SEC(5);

	const int var_limit_time[VAR_TIME_MAX_NUM] = {
		0,
#ifdef __CHANNEL_CHANGER__
		10,
#endif
#ifdef __SKILLBOOK_SYSTEM__
		0,
#endif
#ifdef __SOULSTONE_SYSTEM__
		0,
#endif
#ifdef __GROWTH_PET_SYSTEM__
		5,
#endif
#ifdef __GROWTH_PET_SYSTEM__
		5,
#endif
#ifdef __ITEM_EVOLUTION__
		5,
#endif
#ifdef __INGAME_MALL__
		5,
#endif
#ifdef __REMOVE_SKILL_AFFECT__
		0,
#endif
#ifdef __SPLIT_ITEMS__
		3,
#endif
#ifdef __BIOLOG_SYSTEM__
		0,
#endif
	};

	if ((iPulse - ch->GetSafeboxLoadTime()) < limit_time)
		return false;

	if ((iPulse - ch->GetExchangeTime()) < limit_time)
		return false;

	if ((iPulse - ch->GetMyShopTime()) < limit_time)
		return false;

	if ((iPulse - ch->GetRefineTime()) < limit_time)
		return false;

#ifdef __ACCE_SYSTEM__
	if ((iPulse - ch->GetAcceTime()) < limit_time)
		return false;
#endif

#ifdef __ITEM_CHANGELOOK__
	if ((iPulse - ch->GetChangeLookTime()) < limit_time)
		return false;
#endif

	for (int x = 0; x < VAR_TIME_MAX_NUM; ++x)
	{
		if ((iPulse - ch->GetVarTime(x)) < var_limit_time[x])
			return false;
	}

	if (ch->IsObserverMode())
		return false;

	if (ch->IsUnderRefine())
		return false;

	if (NULL != ch->DragonSoul_RefineWindow_GetOpener())
		return false;

	if (ch->IsWarping())
		return false;

	if (ch->FindAffect(AFFECT_KANAL_FIX))
		return false;

	if (ch->GetExchange())
		return false;

	if (ch->GetMyShop())
		return false;

	if (ch->GetShopOwner())
		return false;

	if (ch->IsOpenSafebox())
		return false;

	if (ch->IsCubeOpen())
		return false;

	if (ch->IsUnderRefine())
		return false;

#ifdef __ACCE_SYSTEM__
	if (ch->isAcceOpened(true))
		return false;

	if (ch->isAcceOpened(false))
		return false;
#endif

#ifdef __ITEM_CHANGELOOK__
	if (ch->isChangeLookOpened())
		return false;
#endif

	for (int x = 0; x < VAR_VAL_MAX_NUM; ++x)
	{
		if (ch->GetVarValue(x) == true)
			return false;
	}

	return true;
}

namespace offlineshop
{
	EVENTINFO(offlineshopempty_info)
	{
		int empty;

		offlineshopempty_info()
			: empty(0)
		{
		}
	};

	EVENTFUNC(func_offlineshop_update_duration)
	{
		offlineshop::GetManager().UpdateShopsDuration();
		return OFFLINESHOP_DURATION_UPDATE_TIME;
	}

	offlineshop::CShopManager& GetManager()
	{
		return offlineshop::CShopManager::instance();
	}

	offlineshop::CShop* CShopManager::PutsNewShop(TShopInfo* pInfo)
	{
		OFFSHOP_DEBUG("puts new shop %s ", pInfo->szName);

		SHOPMAP::iterator it = m_mapShops.insert(std::make_pair(pInfo->dwOwnerID, offlineshop::CShop())).first;
		offlineshop::CShop& rShop = it->second;

		rShop.SetDuration(pInfo->dwDuration);
		rShop.SetOwnerPID(pInfo->dwOwnerID);
		rShop.SetName(pInfo->szName);
		rShop.SetPosInfo(pInfo->lMapIndex, pInfo->x, pInfo->y, pInfo->bChannel);

#ifdef __OFFLINE_SHOP_ENTITY_CITIES__
		CreateNewShopEntities(rShop);
#endif
		return &rShop;
	}

	offlineshop::CShop* CShopManager::GetShopByOwnerID(DWORD dwPID)
	{
		SHOPMAP::iterator it = m_mapShops.find(dwPID);
		if (it == m_mapShops.end())
			return nullptr;

		return &(it->second);
	}

	void CShopManager::RemoveSafeboxFromCache(DWORD dwOwnerID)
	{
		SAFEBOXMAP::iterator it = m_mapSafeboxs.find(dwOwnerID);
		if (it == m_mapSafeboxs.end())
			return;

		m_mapSafeboxs.erase(it);
	}

	void CShopManager::RemoveGuestFromShops(LPCHARACTER ch)
	{
		if (ch->GetOfflineShopGuest())
			ch->GetOfflineShopGuest()->RemoveGuest(ch);

		ch->SetOfflineShopGuest(NULL);

		if (ch->GetOfflineShop())
			ch->GetOfflineShop()->RemoveGuest(ch);

		ch->SetOfflineShop(NULL);
	}

	int CShopManager::GetMapIndexAllowsList(int iMapIndex)
	{
		int index = 0;

		for (auto it = s_set_offlineshop_map_allows.begin(); it != s_set_offlineshop_map_allows.end(); it++)
		{
			if (*it == iMapIndex)
				return index;

			index++;
		}
		return -1;
	}

	CShopManager::CShopManager()
	{
		offlineshopempty_info* info = AllocEventInfo<offlineshopempty_info>();
		m_eventShopDuration = event_create(func_offlineshop_update_duration, info, OFFLINESHOP_DURATION_UPDATE_TIME);
#ifdef USE_OFFLINE_SHOP_CONFIG
		m_vecCities.resize(shop_cities_size());
#else
		m_vecCities.resize(Offlineshop_GetMapCount());
#endif // USE_OFFLINE_SHOP_CONFIG
	}

	CShopManager::~CShopManager()
	{
		Destroy();
	}

	void CShopManager::Destroy()
	{
		//deleting event
		if (m_eventShopDuration)
			event_cancel(&m_eventShopDuration);

		m_eventShopDuration = nullptr;

		//clearing containers
		m_mapSafeboxs.clear();
		m_mapShops.clear();

#ifdef __OFFLINE_SHOP_ENTITY_CITIES__
		//deleting entities
		for (itertype(m_vecCities) itCities = m_vecCities.begin(); itCities != m_vecCities.end(); itCities++)
		{
			TCityShopInfo& city = *itCities;

			for (itertype(city.entitiesByPID) it = city.entitiesByPID.begin(); it != city.entitiesByPID.end(); it++)
				delete(it->second);

			city.entitiesByPID.clear();
			city.entitiesByVID.clear();
		}

		m_vecCities.clear();
#endif
	}

#ifdef __OFFLINE_SHOP_ENTITY_CITIES__

	bool IsEmptyString(const std::string& st) {
		return st.find_first_not_of(" \t\r\n") == std::string::npos;
	}

	void CShopManager::__UpdateEntity(const offlineshop::CShop& rShop)
	{
		itertype(m_vecCities) it = m_vecCities.begin();
		for (; it != m_vecCities.end(); it++)
		{
			itertype(it->entitiesByPID) itMap = it->entitiesByPID.find(rShop.GetOwnerPID());
			if (itMap == it->entitiesByPID.end())
				continue;

			ShopEntity& ent = *(itMap->second);
			ent.SetShopName(rShop.GetName());

			if (ent.GetSectree())
				ent.ViewReencode();

#ifdef __OFFLINESHOP_DEBUG__
			else
			{
				fprintf(stderr, "cant find sectree for entity : name %s , pid %u ", ent.GetShopName(), ent.GetShop()->GetOwnerPID());
			}
#endif
		}
	}

	void CShopManager::CreateNewShopEntities(offlineshop::CShop& rShop)
	{
		if (rShop.GetChannel() == g_bChannel)
		{
			const TShopPosition& rPosition = *rShop.GetPosInfo();

			WORD cityIndex = get_shop_cities_index(rPosition.lMapIndex);
			if (cityIndex == -1) // TODO: mavi bayrak 1 de baslatilacak
				return;

			long x = rPosition.x, y = rPosition.y, lMapIndex = rPosition.lMapIndex;
			PositionHelper(lMapIndex, &x, &y);

			if (!map_allow_find(lMapIndex))
				return;

			LPSECTREE sectree = SECTREE_MANAGER::Instance().Get(rPosition.lMapIndex, x, y);
			if (sectree)
			{
				OFFSHOP_DEBUG("map ok! map_index %d (%d, %d)", rPosition.lMapIndex, x, y);
				ShopEntity* pEntity = new ShopEntity();
				pEntity->SetShopName(rShop.GetName());
				pEntity->SetShopType(0); //TODO: add differents shop skins
				pEntity->SetMapIndex(lMapIndex);
				pEntity->SetXYZ(x, y, 0);
				pEntity->SetShop(&rShop);
				sectree->InsertEntity(pEntity);
				pEntity->UpdateSectree();
				m_vecCities[cityIndex].entitiesByPID.insert(std::make_pair(rShop.GetOwnerPID(), pEntity));
				m_vecCities[cityIndex].entitiesByVID.insert(std::make_pair(pEntity->GetVID(), pEntity));
			}
			else
			{
				sys_err("map error! map_index %d (%d, %d)", rShop.GetMapIndex(), rShop.GetMapX(), rShop.GetMapY());
			}
		}
	}

	void CShopManager::DestroyNewShopEntities(const offlineshop::CShop& rShop)
	{
		itertype(m_vecCities) it = m_vecCities.begin();
		for (; it != m_vecCities.end(); it++)
		{
			TCityShopInfo& city = *it;

			itertype(city.entitiesByPID) iter = city.entitiesByPID.find(rShop.GetOwnerPID());

			if (iter == city.entitiesByPID.end())
			{
				//sys_err("CANNOT FOUND NEW SHOP ENTITY : %u ",rShop.GetOwnerPID());
				continue;
			}

			ShopEntity* entity = iter->second;

			if (!entity)
				continue;

			if (!entity->IsType(ENTITY_NEWSHOPS))
			{
				sys_err("if (!entity->IsType(ENTITY_NEWSHOPS)) entitiy: %d", entity->GetType());
				continue;
			}

			DWORD dwVID = entity->GetVID();

			if (entity->GetSectree())
			{
				entity->ViewCleanup();
				entity->GetSectree()->RemoveEntity(entity);
			}

			entity->Destroy();

			delete(entity);
			city.entitiesByPID.erase(iter);
			city.entitiesByVID.erase(city.entitiesByVID.find(dwVID));
		}
	}

	void CShopManager::EncodeInsertShopEntity(ShopEntity& shop, LPCHARACTER ch)
	{
		if (!ch->GetDesc())
			return;

		TPacketGCNewOfflineshop pack;
		pack.bHeader = HEADER_GC_NEW_OFFLINESHOP;
		pack.bSubHeader = SUBHEADER_GC_INSERT_SHOP_ENTITY;
		pack.wSize = sizeof(pack) + sizeof(TSubPacketGCInsertShopEntity);

		const PIXEL_POSITION pos = shop.GetXYZ();

		TSubPacketGCInsertShopEntity subpack;
		subpack.dwVID = shop.GetVID();
		subpack.iType = shop.GetShopType();
		subpack.x = pos.x;
		subpack.y = pos.y;
		subpack.z = pos.z;
		subpack.owner = shop.GetShop()->GetOwnerPID();

		strncpy(subpack.szName, shop.GetShopName(), sizeof(subpack.szName));

		ch->GetDesc()->BufferedPacket(&pack, sizeof(pack));
		ch->GetDesc()->Packet(&subpack, sizeof(subpack));
	}

	void CShopManager::EncodeRemoveShopEntity(ShopEntity& shop, LPCHARACTER ch)
	{
		if (!ch->GetDesc())
			return;

		TPacketGCNewOfflineshop pack;
		pack.bHeader = HEADER_GC_NEW_OFFLINESHOP;
		pack.bSubHeader = SUBHEADER_GC_REMOVE_SHOP_ENTITY;
		pack.wSize = sizeof(pack) + sizeof(TSubPacketGCRemoveShopEntity);

		TSubPacketGCRemoveShopEntity subpack;
		subpack.dwVID = shop.GetVID();

		ch->GetDesc()->BufferedPacket(&pack, sizeof(pack));
		ch->GetDesc()->Packet(&subpack, sizeof(subpack));
	}
#endif

	CShopSafebox* CShopManager::GetShopSafeboxByOwnerID(DWORD dwPID)
	{
		SAFEBOXMAP::iterator it = m_mapSafeboxs.find(dwPID);
		if (it == m_mapSafeboxs.end())
			return nullptr;
		return &(it->second);
	}

	//db packets exchanging
	void CShopManager::SendShopBuyDBPacket(DWORD dwBuyerID, DWORD dwOwnerID, DWORD dwItemID, bool isSearch)
	{
		TPacketGDNewOfflineShop pack;
		pack.bSubHeader = SUBHEADER_GD_BUY_ITEM;

		TSubPacketGDBuyItem subpack;
		subpack.dwGuestID = dwBuyerID;
		subpack.dwOwnerID = dwOwnerID;
		subpack.dwItemID = dwItemID;
		subpack.isSearch = isSearch;

		TEMP_BUFFER buff;
		buff.write(&pack, sizeof(pack));
		buff.write(&subpack, sizeof(subpack));

		OFFSHOP_DEBUG("sending for shop %u and item %u (buyer %u) ", dwOwnerID, dwItemID, dwBuyerID);

		db_clientdesc->DBPacket(HEADER_GD_NEW_OFFLINESHOP, 0, buff.read_peek(), buff.size());
	}

	bool CShopManager::RecvShopBuyDBPacket(DWORD dwBuyerID, DWORD dwOwnerID, DWORD dwItemID, bool isSearch)
	{
		OFFSHOP_DEBUG("buyer %u , owner %u , itemid %u ", dwBuyerID, dwOwnerID, dwItemID);

		LPCHARACTER ch = CHARACTER_MANAGER::instance().FindByPID(dwBuyerID);

		CShop* pkShop = GetShopByOwnerID(dwOwnerID);
		if (!pkShop)
		{
			if (ch)
				SendPopupPacket(ch, OFFLINESHOP_WINDOW_VIEW, SUBERROR_GC_SHOP_BUY_SOLD);
			return false;
		}

		CShopItem* pItem = nullptr;
		if (!pkShop->GetItem(dwItemID, &pItem))
		{
			if (ch)
				SendPopupPacket(ch, OFFLINESHOP_WINDOW_VIEW, SUBERROR_GC_SHOP_BUY_SOLD);
			return false;
		}

		OFFSHOP_DEBUG("checked %s", "successful");

		LPCHARACTER pkOwner = CHARACTER_MANAGER::instance().FindByPID(dwOwnerID);

		if (ch)
		{
			OFFSHOP_DEBUG("buyer is online , name %s , item id %u ", ch->GetName(), dwItemID);

			LPITEM pkItem = pItem->CreateItem();
			if (!pkItem)
			{
				sys_err("cannot create item ( dwItemID %u , dwVnum %u, dwShopOwner %u, dwBuyer %u ) ", dwItemID, pItem->GetInfo()->dwVnum, dwOwnerID, dwBuyerID);
				return false;
			}

			int cell;
			if (pkItem->IsDragonSoul())
				cell = ch->GetEmptyDragonSoulInventory(pkItem);
#ifdef __ADDITIONAL_INVENTORY__
			else if (pkItem->IsUpgradeItem())
				cell = ch->GetEmptyUpgradeInventory(pkItem);
			else if (pkItem->IsBook())
				cell = ch->GetEmptyBookInventory(pkItem);
			else if (pkItem->IsStone())
				cell = ch->GetEmptyStoneInventory(pkItem);
			else if (pkItem->IsFlower())
				cell = ch->GetEmptyFlowerInventory(pkItem);
			else if (pkItem->IsAttrItem())
				cell = ch->GetEmptyAttrInventory(pkItem);
			else if (pkItem->IsChest())
				cell = ch->GetEmptyChestInventory(pkItem);
#endif
			else
				cell = ch->GetEmptyInventory(pkItem->GetSize());

			if (cell != -1)
			{
				if (pkItem->IsDragonSoul())
					pkItem->__ADD_TO_CHARACTER(ch, TItemPos(DRAGON_SOUL_INVENTORY, cell));
#ifdef __ADDITIONAL_INVENTORY__
				else if (pkItem->IsUpgradeItem())
					pkItem->__ADD_TO_CHARACTER(ch, TItemPos(UPGRADE_INVENTORY, cell));
				else if (pkItem->IsBook())
					pkItem->__ADD_TO_CHARACTER(ch, TItemPos(BOOK_INVENTORY, cell));
				else if (pkItem->IsStone())
					pkItem->__ADD_TO_CHARACTER(ch, TItemPos(STONE_INVENTORY, cell));
				else if (pkItem->IsFlower())
					pkItem->__ADD_TO_CHARACTER(ch, TItemPos(FLOWER_INVENTORY, cell));
				else if (pkItem->IsAttrItem())
					pkItem->__ADD_TO_CHARACTER(ch, TItemPos(ATTR_INVENTORY, cell));
				else if (pkItem->IsChest())
					pkItem->__ADD_TO_CHARACTER(ch, TItemPos(CHEST_INVENTORY, cell));
#endif
				else
					pkItem->__ADD_TO_CHARACTER(ch, TItemPos(INVENTORY, cell));
			}
			else
			{
				pkItem->AddToGround(ch->GetMapIndex(), ch->GetXYZ());
				pkItem->StartDestroyEvent(g_aiItemDestroyTime[ITEM_DESTROY_TIME_AUTOGIVE]);
				pkItem->SetOwnership(ch);
			}

			// @@ Offline Shop Gold Log & Info Fix
#ifdef __GOLD_LIMIT_REWORK__
			long long gold = pItem->GetPrice()->illYang;
#else
			DWORD gold = pItem->GetPrice()->illYang;
#endif
#ifdef __CHEQUE_SYSTEM__
			DWORD cheque = pItem->GetPrice()->iCheque;
#endif
			// @@

			DWORD dwItemID = pItem->GetID();
			pkShop->BuyItem(dwItemID);

			LPDESC pkChrDesc = ch->GetDesc();

			ch->SetOfflineShopTime();

			if (pkOwner && pkOwner->GetDesc())
			{
				if (pkChrDesc)
				{
					DWORD tradeID = LogManager::instance().ExchangeLog(EXCHANGE_TYPE_SHOP, 1, ch->GetPlayerID(), ch->GetName(), dwOwnerID, "OfflineShop", ch->GetX(), ch->GetY(), gold, 0, cheque, 0, pkChrDesc->GetMacAddr(), "Null[OfflineShop]", pkChrDesc->GetHDDSerial(), "Null[OfflineShop]");
					LogManager::instance().ExchangeItemLog(tradeID, pkItem, ch->GetName());
				}
				else
				{
					LogManager::instance().HackLogEx("OROSPU_EVLADI","TICARETLOGBUGU");
				}
			}
			else
			{
				if (pkChrDesc)
				{
					DWORD tradeID = LogManager::instance().ExchangeLog(EXCHANGE_TYPE_SHOP, 1, ch->GetPlayerID(), ch->GetName(), dwOwnerID, "OfflineShop", ch->GetX(), ch->GetY(), gold, 0, cheque, 0, pkChrDesc->GetMacAddr(), "Null[OfflineShop]", pkChrDesc->GetHDDSerial(), "Null[OfflineShop]");
					LogManager::instance().ExchangeItemLog(tradeID, pkItem, ch->GetName());
				}
				else
				{
					LogManager::instance().HackLogEx("OROSPU_EVLADI","TICARETLOGBUGUOWNERYOK");
				}
			}

			if (pkChrDesc)
			{
				if (isSearch)
				{
					SendShopBuyItemFromSearchClientPacket(ch, dwOwnerID, dwItemID);
				}
				else
				{
					TPacketGCNewOfflineshop pack;
					pack.bHeader = HEADER_GC_NEW_OFFLINESHOP;
					pack.bSubHeader = SUBHEADER_GC_SHOP_BUY_RESULT;
					pack.wSize = sizeof(pack) + sizeof(TSubPacketGCShopBuyResult);

					TSubPacketGCShopBuyResult subpack;
					subpack.bSuccess = true;

					pkChrDesc->BufferedPacket(&pack, sizeof(pack));
					pkChrDesc->Packet(&subpack, sizeof(subpack));
				}
			}
		}
		else
		{
			OFFSHOP_DEBUG("buyer isn't online , item removed %u (shop %u)", dwItemID, pkShop->GetOwnerPID());

			DWORD dwItemID = pItem->GetID();
			pkShop->BuyItem(dwItemID);
		}

		return true;
	}

	void CShopManager::SendShopRemoveItemDBPacket(DWORD dwOwnerID, DWORD dwItemID)
	{
		TPacketGDNewOfflineShop pack;
		pack.bSubHeader = SUBHEADER_GD_REMOVE_ITEM;

		TSubPacketGDRemoveItem subpack;
		subpack.dwOwnerID = dwOwnerID;
		subpack.dwItemID = dwItemID;

		TEMP_BUFFER buff;
		buff.write(&pack, sizeof(pack));
		buff.write(&subpack, sizeof(subpack));

		OFFSHOP_DEBUG("owner %u , item %u ", dwOwnerID, dwItemID);

		db_clientdesc->DBPacket(HEADER_GD_NEW_OFFLINESHOP, 0, buff.read_peek(), buff.size());
	}

	bool CShopManager::RecvShopRemoveItemDBPacket(DWORD dwOwnerID, DWORD dwItemID)
	{
		CShop* pkShop = GetShopByOwnerID(dwOwnerID);
		if (!pkShop)
			return false;

		OFFSHOP_DEBUG("owner %u , item %u", dwOwnerID, dwItemID);
		return pkShop->RemoveItem(dwItemID);
	}

	void CShopManager::SendShopAddItemDBPacket(DWORD dwOwnerID, const TItemInfo& rItemInfo)
	{
		TPacketGDNewOfflineShop pack;
		pack.bSubHeader = SUBHEADER_GD_ADD_ITEM;

		TSubPacketGDAddItem subpack;
		subpack.dwOwnerID = dwOwnerID;
		CopyObject(subpack.itemInfo, rItemInfo);

		TEMP_BUFFER buff;
		buff.write(&pack, sizeof(pack));
		buff.write(&subpack, sizeof(subpack));

		OFFSHOP_DEBUG("owner %u , item vnum %u , item count %u ", dwOwnerID, rItemInfo.item.dwVnum, rItemInfo.item.dwCount);
		db_clientdesc->DBPacket(HEADER_GD_NEW_OFFLINESHOP, 0, buff.read_peek(), buff.size());
	}

	bool CShopManager::RecvShopAddItemDBPacket(DWORD dwOwnerID, const TItemInfo& rItemInfo)
	{
		CShop* pkShop = GetShopByOwnerID(dwOwnerID);
		if (!pkShop)
			return false;

		CShopItem newItem(rItemInfo.dwItemID);
		newItem.SetInfo(rItemInfo.item);
		newItem.SetPrice(rItemInfo.price);
		newItem.SetOwnerID(rItemInfo.dwOwnerID);

		OFFSHOP_DEBUG("owner %u , item id %u ", dwOwnerID, rItemInfo.dwItemID);
		return pkShop->AddItem(newItem);
	}

	//SHOPS
	void CShopManager::SendShopForceCloseDBPacket(DWORD dwPID)
	{
		TPacketGDNewOfflineShop pack;
		pack.bSubHeader = SUBHEADER_GD_SHOP_FORCE_CLOSE;

		TSubPacketGDShopForceClose subpack;
		subpack.dwOwnerID = dwPID;

		TEMP_BUFFER buff;
		buff.write(&pack, sizeof(pack));
		buff.write(&subpack, sizeof(subpack));

		OFFSHOP_DEBUG("shop %u ", dwPID);
		db_clientdesc->DBPacket(HEADER_GD_NEW_OFFLINESHOP, 0, buff.read_peek(), buff.size());
	}

	bool CShopManager::RecvShopForceCloseDBPacket(DWORD dwPID)
	{
		CShop* pkShop = GetShopByOwnerID(dwPID);
		if (!pkShop)
			return false;

#ifdef __OFFLINE_SHOP_ENTITY_CITIES__
		DestroyNewShopEntities(*pkShop);
#endif

		LPCHARACTER ch = CHARACTER_MANAGER::instance().FindByPID(dwPID);

		if (ch)
			ch->SetOfflineShop(NULL);

		CShop::LISTGUEST* guests = pkShop->GetGuests();

		for (CShop::LISTGUEST::iterator it = guests->begin(); it != guests->end(); it++)
		{
			LPCHARACTER chGuest = AS_LPGUEST(*it);
			if (!chGuest) {
				continue;
			}

			if (ch && ch == chGuest)
				SendShopOpenMyShopNoShopClientPacket(chGuest);

			chGuest->SetOfflineShopGuest(NULL);
		}

		pkShop->Clear();

		m_mapShops.erase(m_mapShops.find(pkShop->GetOwnerPID()));
		return true;
	}

	void CShopManager::SendShopLockBuyItemDBPacket(DWORD dwBuyerID, DWORD dwOwnerID, DWORD dwItemID, bool isSearch)
	{
		TPacketGDNewOfflineShop pack;
		pack.bSubHeader = SUBHEADER_GD_BUY_LOCK_ITEM;

		TSubPacketGDLockBuyItem subpack;
		subpack.dwGuestID = dwBuyerID;
		subpack.dwOwnerID = dwOwnerID;
		subpack.dwItemID = dwItemID;
		subpack.isSearch = isSearch;

		TEMP_BUFFER buff;
		buff.write(&pack, sizeof(pack));
		buff.write(&subpack, sizeof(subpack));

		OFFSHOP_DEBUG("shop %u , buyer %u , item %u (size %u) ", dwOwnerID, dwBuyerID, dwItemID, buff.size());
		db_clientdesc->DBPacket(HEADER_GD_NEW_OFFLINESHOP, 0, buff.read_peek(), buff.size());
	}

	bool CShopManager::RecvShopLockedBuyItemDBPacket(DWORD dwBuyerID, DWORD dwOwnerID, DWORD dwItemID, bool isSearch)
	{
		CShop* pkShop = GetShopByOwnerID(dwOwnerID);
		LPCHARACTER ch = CHARACTER_MANAGER::instance().FindByPID(dwBuyerID);

		if (!ch || !pkShop)
		{
			SendShopCannotBuyLockedItemDBPacket(dwOwnerID, dwItemID);
			return false;
		}

		OFFSHOP_DEBUG("found shop %u ", dwBuyerID);

		CShopItem* pkItem = nullptr;
		if (!pkShop->GetItem(dwItemID, &pkItem))
		{
			SendShopCannotBuyLockedItemDBPacket(dwOwnerID, dwItemID);
			return false;
		}

		OFFSHOP_DEBUG("found item %u", dwItemID);

		if (!pkItem->CanBuy(ch))
		{
			SendShopCannotBuyLockedItemDBPacket(dwOwnerID, dwItemID);
			return false;
		}

		OFFSHOP_DEBUG("can buy %u", dwItemID);

		TPriceInfo* pPrice = pkItem->GetPrice();
		ch->PointChange(POINT_GOLD, -pPrice->illYang);
#ifdef __CHEQUE_SYSTEM__
		ch->PointChange(POINT_CHEQUE, -pPrice->iCheque);
#endif

		SendShopBuyDBPacket(dwBuyerID, dwOwnerID, dwItemID, isSearch);
		return true;
	}

	void CShopManager::SendShopCannotBuyLockedItemDBPacket(DWORD dwOwnerID, DWORD dwItemID) //topatch
	{
		TPacketGDNewOfflineShop pack;
		pack.bSubHeader = SUBHEADER_GD_CANNOT_BUY_LOCK_ITEM;

		TSubPacketGDCannotBuyLockItem subpack;
		subpack.dwOwnerID = dwOwnerID;
		subpack.dwItemID = dwItemID;

		TEMP_BUFFER buff;
		buff.write(&pack, sizeof(pack));
		buff.write(&subpack, sizeof(subpack));

		OFFSHOP_DEBUG("item id %u, owner shop %u", dwItemID, dwOwnerID);
		db_clientdesc->DBPacket(HEADER_GD_NEW_OFFLINESHOP, 0, buff.read_peek(), buff.size());
	}

	bool CShopManager::RecvShopExpiredDBPacket(DWORD dwPID) //topatch
	{
		CShop* pkShop = GetShopByOwnerID(dwPID);
		if (!pkShop)
			return false;

		LPCHARACTER ch = CHARACTER_MANAGER::instance().FindByPID(dwPID);

		if (ch)
			ch->SetOfflineShop(NULL);

		//*getting the guest list before to remove the shop
		//*that is necessary to send the shop list packets
		CShop::LISTGUEST guests = *pkShop->GetGuests();

#ifdef __OFFLINE_SHOP_ENTITY_CITIES__
		DestroyNewShopEntities(*pkShop);
#endif
		pkShop->Clear();
		m_mapShops.erase(m_mapShops.find(pkShop->GetOwnerPID()));

		for (CShop::LISTGUEST::iterator it = guests.begin(); it != guests.end(); it++)
		{
			LPCHARACTER chGuest = AS_LPGUEST(*it);
			if (!chGuest) {
				continue;
			}

			if (ch && ch == chGuest)
				SendShopOpenMyShopNoShopClientPacket(chGuest);

			chGuest->SetOfflineShopGuest(NULL);
		}

		return true;
	}

	void CShopManager::SendShopCreateNewDBPacket(const TShopInfo& shop, std::vector<TItemInfo>& vec)
	{
		TPacketGDNewOfflineShop pack;
		pack.bSubHeader = SUBHEADER_GD_SHOP_CREATE_NEW;

		TSubPacketGDShopCreateNew subpack;
		CopyObject(subpack.shop, shop);

		TEMP_BUFFER buff;
		buff.write(&pack, sizeof(pack));
		buff.write(&subpack, sizeof(subpack));

		for (DWORD i = 0; i < vec.size(); i++)
			buff.write(&vec[i], sizeof(TItemInfo));

		db_clientdesc->DBPacket(HEADER_GD_NEW_OFFLINESHOP, 0, buff.read_peek(), buff.size());
	}

	bool CShopManager::RecvShopCreateNewDBPacket(const TShopInfo& shop, std::vector<TItemInfo>& vec)
	{
		OFFSHOP_DEBUG("shop %s , shop id %u ", shop.szName, shop.dwOwnerID);

		if (m_mapShops.find(shop.dwOwnerID) != m_mapShops.end())
			return false;

		CShop newShop;
		newShop.SetOwnerPID(shop.dwOwnerID);
		newShop.SetDuration(shop.dwDuration);
		newShop.SetName(shop.szName);
		newShop.SetPosInfo(shop.lMapIndex, shop.x, shop.y, shop.bChannel);

		std::vector<CShopItem> items;
		items.reserve(vec.size());

		for (DWORD i = 0; i < vec.size(); i++)
		{
			const TItemInfo& rItem = vec[i];

			CShopItem shopItem(rItem.dwItemID);

			shopItem.SetOwnerID(rItem.dwOwnerID);
			shopItem.SetPrice(rItem.price);
			shopItem.SetInfo(rItem.item);

			OFFSHOP_DEBUG("item id %u , item vnum %u , item count %u ", rItem.dwItemID, rItem.item.dwVnum, rItem.item.dwCount);

			items.push_back(shopItem);
		}

		newShop.SetItems(&items);

		OFFSHOP_DEBUG("shop %s , shop id %u inserted into map (items count %d)", shop.szName, shop.dwOwnerID, shop.dwCount);
		SHOPMAP::iterator it = m_mapShops.insert(std::make_pair(newShop.GetOwnerPID(), newShop)).first;

#ifdef __OFFLINE_SHOP_ENTITY_CITIES__
		CreateNewShopEntities(it->second);
#endif

		LPCHARACTER chOwner = it->second.FindOwnerCharacter();
		if (chOwner)
		{
			chOwner->SetOfflineShop(&(it->second));
			chOwner->SetOfflineShopGuest(&(it->second));

#ifdef __GOLD_LIMIT_REWORK__
			long long price = 0;
#else
			DWORD price = 0;
#endif

			if (shop.dwDuration == 10080)
				price = 72000000;

			chOwner->PointChange(POINT_GOLD, -price);

			it->second.AddGuest(chOwner);
			SendShopOpenMyShopClientPacket(chOwner);
			SendShopCloseBuilderBoard(chOwner); // Send Client to close offshop builder
		}

		return true;
	}

	void CShopManager::SendShopCloseBuilderBoard(LPCHARACTER chOwner)
	{
		TPacketGCNewOfflineshop pack;
		pack.bHeader = HEADER_GC_NEW_OFFLINESHOP;
		pack.bSubHeader = SUBHEADER_GC_SHOP_CREATE_SUCCESS;
		pack.wSize = sizeof(pack);

		chOwner->GetDesc()->Packet(&pack, sizeof(pack));
	}

	bool CShopManager::RecvShopSafeboxGetItemDBPacket(DWORD dwOwnerID, DWORD dwItemID)
	{
		CShopSafebox* pkSafebox = GetShopSafeboxByOwnerID(dwOwnerID);

		if (!pkSafebox)
			return false;

		if (pkSafebox->RemoveItem(dwItemID))
			return true;

		return false;
	}

	bool CShopManager::RecvShopSafeboxGetValutesDBPacket(DWORD dwOwnerID, const TValutesInfo& valutes)
	{
		CShopSafebox* pkSafebox = GetShopSafeboxByOwnerID(dwOwnerID);

		if (!pkSafebox)
			return false;

		if (pkSafebox->RemoveValute(valutes))
			return true;

		return false;
	}

	bool CShopManager::RecvShopSafeboxAddItemDBPacket(DWORD dwOwnerID, DWORD dwItemID, const TItemInfoEx& item)
	{
		LPCHARACTER ch = CHARACTER_MANAGER::instance().FindByPID(dwOwnerID);
		CShopSafebox* pkSafebox = ch && ch->GetShopSafebox() ? ch->GetShopSafebox() : GetShopSafeboxByOwnerID(dwOwnerID);

		if (!pkSafebox)
			return false;

		CShopItem shopItem(dwItemID);
		shopItem.SetInfo(item);
		shopItem.SetOwnerID(dwOwnerID);

		pkSafebox->AddItem(&shopItem);
		if (ch && ch->GetShopSafebox())
			pkSafebox->RefreshToOwner(ch);

		OFFSHOP_DEBUG("safebox owner %u , item %u ", dwOwnerID, dwItemID);
		return true;
	}

	bool CShopManager::SendShopSafeboxAddItemDBPacket(DWORD dwOwnerID, const CShopItem& item) {
		TPacketGDNewOfflineShop pack;
		pack.bSubHeader = SUBHEADER_GD_SAFEBOX_ADD_ITEM;

		TSubPacketGDSafeboxAddItem subpack;
		subpack.dwOwnerID = dwOwnerID;
		CopyObject(subpack.item, *item.GetInfo());

		TEMP_BUFFER buff;
		buff.write(&pack, sizeof(pack));
		buff.write(&subpack, sizeof(subpack));

		db_clientdesc->DBPacket(HEADER_GD_NEW_OFFLINESHOP, 0, buff.read_peek(), buff.size());
		//* add return true
		return true;
	}

	bool CShopManager::RecvShopSafeboxAddValutesDBPacket(DWORD dwOwnerID, const TValutesInfo& valute)
	{
		LPCHARACTER ch = CHARACTER_MANAGER::instance().FindByPID(dwOwnerID);
		CShopSafebox* pkSafebox = ch && ch->GetShopSafebox() ? ch->GetShopSafebox() : GetShopSafeboxByOwnerID(dwOwnerID);

		if (!pkSafebox)
			return false;

		pkSafebox->AddValute(valute);
		if (ch && ch->GetShopSafebox())
			pkSafebox->RefreshToOwner(ch);
		return true;
	}

	void CShopManager::SendShopSafeboxGetItemDBPacket(DWORD dwOwnerID, DWORD dwItemID)
	{
		TPacketGDNewOfflineShop pack;
		pack.bSubHeader = SUBHEADER_GD_SAFEBOX_GET_ITEM;

		TSubPacketGDSafeboxGetItem subpack;
		subpack.dwOwnerID = dwOwnerID;
		subpack.dwItemID = dwItemID;

		TEMP_BUFFER buff;
		buff.write(&pack, sizeof(pack));
		buff.write(&subpack, sizeof(subpack));

		OFFSHOP_DEBUG(" owner % u , item %u ", dwOwnerID, dwItemID);
		db_clientdesc->DBPacket(HEADER_GD_NEW_OFFLINESHOP, 0, buff.read_peek(), buff.size());
	}

	void CShopManager::SendShopSafeboxGetValutesDBPacket(DWORD dwOwnerID, const TValutesInfo& valutes)
	{
		TPacketGDNewOfflineShop pack;
		pack.bSubHeader = SUBHEADER_GD_SAFEBOX_GET_VALUTES;

		TSubPacketGDSafeboxGetValutes subpack;
		subpack.dwOwnerID = dwOwnerID;
		CopyObject(subpack.valute, valutes);

		TEMP_BUFFER buff;
		buff.write(&pack, sizeof(pack));
		buff.write(&subpack, sizeof(subpack));

		db_clientdesc->DBPacket(HEADER_GD_NEW_OFFLINESHOP, 0, buff.read_peek(), buff.size());
	}

	bool CShopManager::RecvShopSafeboxLoadDBPacket(UINT dwOwnerID, const TValutesInfo& valute, const std::vector<UINT>& ids, const std::vector<TItemInfoEx>& items)
	{
		/*if(GetShopSafeboxByOwnerID(dwOwnerID))
			return false;*/

		CShopSafebox::VECITEM vec;
		vec.reserve(ids.size());

		for (UINT i = 0; i < ids.size(); i++)
		{
			CShopItem item(ids[i]);
			item.SetInfo(items[i]);
			item.SetOwnerID(dwOwnerID);
			vec.push_back(item);
		}


		CShopSafebox safebox;
		safebox.SetItems(&vec);
		safebox.SetValuteAmount(valute);

		m_mapSafeboxs[dwOwnerID] = safebox;
		return true;
	}

	//patch 08-03-2020
	bool CShopManager::RecvShopSafeboxExpiredItemDBPacket(DWORD dwOwnerID, DWORD dwItemID) {
		CShopSafebox* data = GetShopSafeboxByOwnerID(dwOwnerID);
		if (data) {
			data->RemoveItem(dwItemID);
			data->RefreshToOwner();
		} return true;
	}

	//client packets exchanging
	bool CShopManager::RecvShopCreateNewClientPacket(LPCHARACTER ch, TShopInfo& rShopInfo, std::vector<TShopItemInfo>& vec)
	{
		if (!ch || ch->GetOfflineShop())
			return false;

		if (!CheckCharacterActions(ch))
		{
			SendPopupPacket(ch, OFFLINESHOP_WINDOW_BUILDER, SUBERROR_GC_SHOP_CHARACTER_ACTIONS);
			return false;
		}

		if (ch->GetDungeon())
		{
			SendPopupPacket(ch, OFFLINESHOP_WINDOW_VIEW, SUBERROR_GC_SHOP_DUNGEON);
			return false;
		}

		if (g_bChannel != 1)
		{
			SendPopupPacket(ch, OFFLINESHOP_WINDOW_BUILDER, SUBERROR_GC_SHOP_CHANNEL_LIMIT);
			return false;
		}

		if (!g_bOfflineShopServer)
		{
			SendPopupPacket(ch, OFFLINESHOP_WINDOW_BUILDER, SUBERROR_GC_SHOP_CHANNEL_LIMIT);
			return false;
		}

		if (vec.size() > OFFLINESHOP_MAX_ITEM_COUNT)
		{
			SendPopupPacket(ch, OFFLINESHOP_WINDOW_BUILDER, SUBERROR_GC_SHOP_ITEM_LIMIT);
			return false;
		}

		if (ch->GetLevel() < 15)
		{
			SendPopupPacket(ch, OFFLINESHOP_WINDOW_BUILDER, SUBERROR_GC_SHOP_LEVEL_LIMIT);
			return false;
		}

		if (rShopInfo.dwDuration <= 0 && rShopInfo.dwDuration > 3)
		{
			SendPopupPacket(ch, OFFLINESHOP_WINDOW_BUILDER, SUBERROR_GC_SHOP_DURATION_NON);
			return false;
		}

		if (GetShopMapSize() > s_MaxOffShopCount)
		{
			SendPopupPacket(ch, OFFLINESHOP_WINDOW_BUILDER, SUBERROR_GC_SHOP_MAP_LIMIT_GLOBAL);
			return false;
		}

#ifdef __GOLD_LIMIT_REWORK__
		long long needGold = 0;
#else
		DWORD needGold = 0;
#endif
		DWORD dwRealDuration = 0;
		if (rShopInfo.dwDuration == 1)
		{
			dwRealDuration = 10080;
			needGold = 72000000;
		}

		if (rShopInfo.dwDuration <= 0)
		{
			SendPopupPacket(ch, OFFLINESHOP_WINDOW_BUILDER, SUBERROR_GC_SHOP_DURATION_MIN);
			return false;
		}

		if (rShopInfo.dwDuration > 3)
		{
			SendPopupPacket(ch, OFFLINESHOP_WINDOW_BUILDER, SUBERROR_GC_SHOP_DURATION_MAX);
			return false;
		}

		if (ch->GetGold() < needGold)
		{
			SendPopupPacket(ch, OFFLINESHOP_WINDOW_BUILDER, SUBERROR_GC_SHOP_ENOUGH_GOLD);
			return false;
		}

		OFFSHOP_DEBUG("ch name %s , item count %u , duration %u, realDuration %u ", ch->GetName(), rShopInfo.dwCount, rShopInfo.dwDuration, dwRealDuration);

		if (!strcmp(rShopInfo.szName, "%") ||
			!strcmp(rShopInfo.szName, "/") ||
			!strcmp(rShopInfo.szName, ">") ||
			!strcmp(rShopInfo.szName, "|") ||
			!strcmp(rShopInfo.szName, ";") ||
			!strcmp(rShopInfo.szName, ":") ||
			!strcmp(rShopInfo.szName, "}") ||
			!strcmp(rShopInfo.szName, "{") ||
			!strcmp(rShopInfo.szName, "[") ||
			!strcmp(rShopInfo.szName, "]") ||
			!strcmp(rShopInfo.szName, "%") ||
			!strcmp(rShopInfo.szName, "#") ||
			!strcmp(rShopInfo.szName, "@") ||
			!strcmp(rShopInfo.szName, "^") ||
			!strcmp(rShopInfo.szName, "&") ||
			!strcmp(rShopInfo.szName, "'")
			)
		{
			SendPopupPacket(ch, OFFLINESHOP_WINDOW_BUILDER, SUBERROR_GC_SHOP_NAME);
			return false;
		}

		ch->SetOfflineShopTime();

		static char szNameChecked[OFFLINE_SHOP_NAME_MAX_LEN];
		strncpy(szNameChecked, rShopInfo.szName, sizeof(szNameChecked));

		//making full name
		snprintf(rShopInfo.szName, sizeof(rShopInfo.szName), "%s@%s", ch->GetName(), szNameChecked);

		std::vector<TItemInfo> vecItem;
		vecItem.reserve(vec.size());

		rShopInfo.dwOwnerID = ch->GetPlayerID();
		rShopInfo.lMapIndex = ch->GetMapIndex();
		rShopInfo.x = ch->GetX();
		rShopInfo.y = ch->GetY();
		rShopInfo.bChannel = g_bChannel;
		TItemInfo itemInfo;

		for (DWORD i = 0; i < vec.size(); i++)
		{
			TShopItemInfo& rShopItem = vec[i];
			TPriceInfo& price = rShopItem.price;

			if (price.GetTotalYangAmount() < 0)
			{
				SendPopupPacket(ch, OFFLINESHOP_WINDOW_BUILDER, SUBERROR_GC_SHOP_ZERO_GOLD_CHEQUE);
				LogManager::Instance().HackLogEx(ch, "Trying gold negative bug... (CrashCore) (CShopManager::RecvShopCreateNewClientPacket)");
				return false;
			}

#ifdef __CHEQUE_SYSTEM__
			if (price.GetTotalChequeAmount() < 0)
			{
				SendPopupPacket(ch, OFFLINESHOP_WINDOW_BUILDER, SUBERROR_GC_SHOP_ZERO_GOLD_CHEQUE);
				LogManager::Instance().HackLogEx(ch, "Trying cheque negative bug... (CrashCore) (CShopManager::RecvShopCreateNewClientPacket)");
				return false;
			}
#endif

			LPITEM item = ch->GetItem(rShopItem.pos);
			if (!item)
			{
				SendPopupPacket(ch, OFFLINESHOP_WINDOW_BUILDER, SUBERROR_GC_SHOP_CANT_FIND_ITEM);
				return false;
			}

			if (IS_SET(item->GetAntiFlag(), ITEM_ANTIFLAG_GIVE))
			{
				SendPopupPacket(ch, OFFLINESHOP_WINDOW_BUILDER, SUBERROR_GC_SHOP_ANTIFLAG_ITEM);
				return false;
			}

			if (IS_SET(item->GetAntiFlag(), ITEM_ANTIFLAG_MYSHOP))
			{
				SendPopupPacket(ch, OFFLINESHOP_WINDOW_BUILDER, SUBERROR_GC_SHOP_ANTIFLAG_ITEM);
				return false;
			}

			if (item->isLocked() || item->IsEquipped() || item->IsExchanging())
			{
				SendPopupPacket(ch, OFFLINESHOP_WINDOW_BUILDER, SUBERROR_GC_SHOP_LOCKED_ITEM);
				return false;
			}

#ifdef __BEGINNER_ITEM__
			if (item->IsBasicItem()) {
				SendPopupPacket(ch, OFFLINESHOP_WINDOW_BUILDER, SUBERROR_GC_SHOP_BEGINNER_ITEM);
				return false;
			}
#endif

			//checking about double insert same item
			for (DWORD j = 0; j < vec.size(); j++)
			{
				if (i == j)
					continue;

				TShopItemInfo& rShopItemCheck = vec[j];
				if (rShopItemCheck.pos == rShopItem.pos)
					return false;
			}

			ZeroObject(itemInfo);

			itemInfo.dwOwnerID = ch->GetPlayerID();
			memcpy(itemInfo.item.aAttr, item->GetAttributes(), sizeof(itemInfo.item.aAttr));
			memcpy(itemInfo.item.alSockets, item->GetSockets(), sizeof(itemInfo.item.alSockets));

			itemInfo.item.dwVnum = item->GetVnum();
			itemInfo.item.dwCount = item->GetCount();
			//patch 08-03-2020
			itemInfo.item.expiration = GetItemExpiration(item);

#ifdef __ITEM_CHANGELOOK__
			itemInfo.item.dwTransmutation = item->GetTransmutation();
#endif
#ifdef __ITEM_EVOLUTION__
			itemInfo.item.dwEvolution = item->GetEvolution();
#endif

			CopyObject(itemInfo.price, rShopItem.price);
			vecItem.push_back(itemInfo);
		}

		for (DWORD i = 0; i < vec.size(); i++)
		{
			TShopItemInfo& rShopItem = vec[i];
			LPITEM item = ch->GetItem(rShopItem.pos);
			M2_DESTROY_ITEM(item->RemoveFromCharacter());
		}

		OFFSHOP_DEBUG("ch name %s , checked successful , send to db ", ch->GetName());

		rShopInfo.dwDuration = MIN(dwRealDuration, OFFLINESHOP_DURATION_MAX_MINUTES);
		SendShopCreateNewDBPacket(rShopInfo, vecItem);
		return true;
	}

	bool CShopManager::RecvShopForceCloseClientPacket(LPCHARACTER ch)
	{
		if (!ch || !ch->GetOfflineShop())
			return false;

		if (!CheckCharacterActions(ch))
		{
			SendPopupPacket(ch, OFFLINESHOP_WINDOW_EDIT, SUBERROR_GC_SHOP_CHARACTER_ACTIONS);
			return false;
		}

		ch->SetOfflineShopTime();
		SendShopForceCloseDBPacket(ch->GetPlayerID());
		return true;
	}

	bool CShopManager::RecvShopOpenClientPacket(LPCHARACTER ch, DWORD dwOwnerID)
	{
		if (!ch || !ch->GetDesc())
			return false;

		CShop* pkShop = GetShopByOwnerID(dwOwnerID);
		if (!pkShop)
			return false;

		ch->SetOfflineShopTime();

		//fixing double guesting 26-06-2020
		if (ch->GetOfflineShopGuest())
			ch->GetOfflineShopGuest()->RemoveGuest(ch);

		//offlineshop-updated 04/08/19
		if (ch->GetPlayerID() == dwOwnerID)
			SendShopOpenMyShopClientPacket(ch);
		else
			SendShopOpenClientPacket(ch, pkShop);

		pkShop->AddGuest(ch);
		ch->SetOfflineShopGuest(pkShop);
		return true;
	}

	bool CShopManager::RecvShopOpenMyShopClientPacket(LPCHARACTER ch)
	{
		if (!ch || !ch->GetDesc())
			return false;

		ch->SetOfflineShopTime();
		if (!ch->GetOfflineShop())
		{
			SendShopOpenMyShopNoShopClientPacket(ch);
		}
		else
		{
			SendShopOpenMyShopClientPacket(ch);
			ch->GetOfflineShop()->AddGuest(ch);
			ch->SetOfflineShopGuest(ch->GetOfflineShop());
		}

		return true;
	}

	bool CShopManager::RecvShopBuyItemClientPacket(LPCHARACTER ch, DWORD dwOwnerID, DWORD dwItemID, bool isSearch, const offlineshop::TPriceInfo& price) //fix-edit-price
	{
		OFFSHOP_DEBUG("owner %u , item id %u ", dwOwnerID, dwItemID);

		if (!ch)
			return false;

		if (!CheckCharacterActions(ch))
		{
			SendPopupPacket(ch, OFFLINESHOP_WINDOW_VIEW, SUBERROR_GC_SHOP_CHARACTER_ACTIONS);
			return false;
		}

		if (ch->GetDungeon())
		{
			SendPopupPacket(ch, OFFLINESHOP_WINDOW_VIEW, SUBERROR_GC_SHOP_DUNGEON);
			return false;
		}

		CShop* pkShop = nullptr;
		if (!(pkShop = GetShopByOwnerID(dwOwnerID)))
		{
			SendPopupPacket(ch, OFFLINESHOP_WINDOW_VIEW, SUBERROR_GC_SHOP_BUY_SOLD);
			return false;
		}

		OFFSHOP_DEBUG("phase 1 %s", "successful");

		CShopItem* pitem = nullptr;
		if (!pkShop->GetItem(dwItemID, &pitem))
		{
			SendPopupPacket(ch, OFFLINESHOP_WINDOW_VIEW, SUBERROR_GC_SHOP_REPORT_ADMIN);
			return false;
		}
		//ADD
		//fix-edit-price
		if (price.illYang != 0)
		{
			if (pitem->GetPrice()->GetTotalYangAmount() != price.GetTotalYangAmount())
			{
				SendPopupPacket(ch, OFFLINESHOP_WINDOW_VIEW, SUBERROR_GC_SHOP_GOLD_LIMIT_MIN);
				return false;
			}
		}

#ifdef __CHEQUE_SYSTEM__
		if (price.iCheque != 0)
		{
			if (pitem->GetPrice()->GetTotalChequeAmount() != price.GetTotalChequeAmount())
			{
				SendPopupPacket(ch, OFFLINESHOP_WINDOW_VIEW, SUBERROR_GC_SHOP_CHEQUE_LIMIT_MIN);
				return false;
			}
		}
#endif

		if (!pitem->CanBuy(ch))
		{
			SendPopupPacket(ch, OFFLINESHOP_WINDOW_VIEW, SUBERROR_GC_SHOP_BUY_FAIL);
			return false;
		}

		ch->SetOfflineShopTime();

		OFFSHOP_DEBUG("sending packet to db (buyer %u , owner %u , item %u )", ch->GetPlayerID(), dwOwnerID, dwItemID);

		SendShopLockBuyItemDBPacket(ch->GetPlayerID(), dwOwnerID, dwItemID, isSearch);
		return true;
	}

#ifdef __OFFLINE_SHOP_ENTITY_CITIES__
	bool CShopManager::RecvShopClickEntity(LPCHARACTER ch, DWORD dwShopEntityVID)
	{
		for (itertype(m_vecCities) it = m_vecCities.begin(); it != m_vecCities.end(); it++)
		{
			itertype(it->entitiesByVID) iterMap = it->entitiesByVID.find(dwShopEntityVID);
			if (it->entitiesByVID.end() == iterMap)
				continue;

			DWORD dwPID = iterMap->second->GetShop()->GetOwnerPID();

			ch->SetOfflineShopTime();
			RecvShopOpenClientPacket(ch, dwPID);
			return true;
		}

		sys_err("cannot found clicked entity , %s vid %u ", ch->GetName(), dwShopEntityVID);
		return false;
	}
#endif

	void CShopManager::SendShopOpenClientPacket(LPCHARACTER ch, CShop* pkShop)
	{
		if (!ch->GetDesc())
			return;

		CShop::VECSHOPITEM* pVec = pkShop->GetItems();
		TEMP_BUFFER buff;
		TPacketGCNewOfflineshop pack;
		pack.bHeader = HEADER_GC_NEW_OFFLINESHOP;
		pack.bSubHeader = SUBHEADER_GC_SHOP_OPEN;
		pack.wSize = sizeof(pack) + sizeof(TSubPacketGCShopOpen) + sizeof(TItemInfo) * pVec->size();

		buff.write(&pack, sizeof(pack));

		TSubPacketGCShopOpen subPack;
		subPack.shop.dwCount = pVec->size();
		subPack.shop.dwDuration = pkShop->GetDuration();
		subPack.shop.dwOwnerID = pkShop->GetOwnerPID();
		strncpy(subPack.shop.szName, pkShop->GetName(), sizeof(subPack.shop.szName));

		buff.write(&subPack, sizeof(subPack));

		TItemInfo itemInfo;

		for (DWORD i = 0; i < pVec->size(); i++)
		{
			CShopItem& rItem = pVec->at(i);
			ZeroObject(itemInfo);

			itemInfo.dwItemID = rItem.GetID();
			itemInfo.dwOwnerID = pkShop->GetOwnerPID();
			CopyObject(itemInfo.item, *(rItem.GetInfo()));
			CopyObject(itemInfo.price, *(rItem.GetPrice()));

			buff.write(&itemInfo, sizeof(itemInfo));
		}

		ch->GetDesc()->Packet(buff.read_peek(), buff.size());
	}

	void CShopManager::SendShopOpenMyShopNoShopClientPacket(LPCHARACTER ch)
	{
		if (!ch->GetDesc())
			return;

		TPacketGCNewOfflineshop pack;
		pack.bHeader = HEADER_GC_NEW_OFFLINESHOP;
		pack.bSubHeader = SUBHEADER_GC_SHOP_OPEN_OWNER_NO_SHOP;
		pack.wSize = sizeof(pack);

		ch->GetDesc()->Packet(&pack, sizeof(pack));
	}

	void CShopManager::SendShopBuyItemFromSearchClientPacket(LPCHARACTER ch, DWORD dwOwnerID, DWORD dwItemID)
	{
		if (!ch->GetDesc())
			return;

		TPacketGCNewOfflineshop pack;
		pack.bHeader = HEADER_GC_NEW_OFFLINESHOP;
		pack.bSubHeader = SUBHEADER_GC_SHOP_BUY_ITEM_FROM_SEARCH;
		pack.wSize = sizeof(pack) + sizeof(TSubPacketGCShopBuyItemFromSearch);

		TSubPacketGCShopBuyItemFromSearch subpack;
		subpack.dwOwnerID = dwOwnerID;
		subpack.dwItemID = dwItemID;

		TEMP_BUFFER buff;
		buff.write(&pack, sizeof(pack));
		buff.write(&subpack, sizeof(subpack));

		ch->GetDesc()->Packet(buff.read_peek(), buff.size());
	}

	void CShopManager::SendShopOpenMyShopClientPacket(LPCHARACTER ch)
	{
		if (!ch->GetDesc())
			return;

		if (!ch->GetOfflineShop())
			return;

		CShop* pkShop = ch->GetOfflineShop();
		DWORD dwOwnerID = ch->GetPlayerID();

		CShop::VECSHOPITEM* pVec = pkShop->GetItems();

		TEMP_BUFFER buff;
		TPacketGCNewOfflineshop pack;
		pack.bHeader = HEADER_GC_NEW_OFFLINESHOP;
		pack.bSubHeader = SUBHEADER_GC_SHOP_OPEN_OWNER;
		pack.wSize = sizeof(pack) + sizeof(TSubPacketGCShopOpenOwner) + sizeof(TItemInfo) * pVec->size();

		buff.write(&pack, sizeof(pack));

		TSubPacketGCShopOpenOwner subPack;
		subPack.shop.dwCount = pVec->size();
		subPack.shop.dwDuration = pkShop->GetDuration();
		subPack.shop.dwOwnerID = dwOwnerID;

		strncpy(subPack.shop.szName, pkShop->GetName(), sizeof(subPack.shop.szName));

		OFFSHOP_DEBUG("owner %u , item count %u , duration %u ", subPack.shop.dwOwnerID, subPack.shop.dwCount, subPack.shop.dwDuration);

		buff.write(&subPack, sizeof(subPack));

		TItemInfo itemInfo;

		for (DWORD i = 0; i < pVec->size(); i++)
		{
			CShopItem& rItem = pVec->at(i);
			ZeroObject(itemInfo);

			itemInfo.dwItemID = rItem.GetID();
			itemInfo.dwOwnerID = dwOwnerID;
			CopyObject(itemInfo.item, *(rItem.GetInfo()));
			CopyObject(itemInfo.price, *(rItem.GetPrice()));

			OFFSHOP_DEBUG("item id %u , item vnum %u , item count %u ", itemInfo.dwItemID, itemInfo.item.dwVnum, itemInfo.item.dwCount);
			buff.write(&itemInfo, sizeof(itemInfo));
		}

		ch->GetDesc()->Packet(buff.read_peek(), buff.size());
	}

	void CShopManager::SendShopForceClosedClientPacket(DWORD dwOwnerID)
	{
		LPCHARACTER ch = CHARACTER_MANAGER::instance().FindByPID(dwOwnerID);
		if (!ch || !ch->GetDesc())
			return;

		TPacketGCNewOfflineshop pack;
		pack.bHeader = HEADER_GC_NEW_OFFLINESHOP;
		pack.bSubHeader = SUBHEADER_GC_SHOP_OPEN_OWNER;

		pack.wSize = sizeof(pack);
		ch->GetDesc()->Packet(&pack, sizeof(pack));
	}

	//ITEMS
	bool CShopManager::RecvShopAddItemClientPacket(LPCHARACTER ch, const TItemPos& pos, const TPriceInfo& price)
	{
		if (!ch || !ch->GetOfflineShop())
			return false;

		CShop* pkShop = nullptr;
		if (!(pkShop = GetShopByOwnerID(ch->GetPlayerID())))
			return false;

		if (!CheckCharacterActions(ch))
		{
			SendPopupPacket(ch, OFFLINESHOP_WINDOW_EDIT, SUBERROR_GC_SHOP_CHARACTER_ACTIONS);
			return false;
		}

		if (ch->GetDungeon())
		{
			SendPopupPacket(ch, OFFLINESHOP_WINDOW_EDIT, SUBERROR_GC_SHOP_DUNGEON);
			return false;
		}

		if (price.GetTotalYangAmount() < 0)
		{
			SendPopupPacket(ch, OFFLINESHOP_WINDOW_EDIT, SUBERROR_GC_SHOP_ZERO_GOLD_CHEQUE);
			LogManager::Instance().HackLogEx(ch, "Trying gold negative bug... (CrashCore) (CShopManager::AddItem)");
			return false;
		}

#ifdef __CHEQUE_SYSTEM__
		if (price.GetTotalChequeAmount() < 0)
		{
			SendPopupPacket(ch, OFFLINESHOP_WINDOW_EDIT, SUBERROR_GC_SHOP_ZERO_GOLD_CHEQUE);
			LogManager::Instance().HackLogEx(ch, "Trying cheque negative bug... (CrashCore) (CShopManager::AddItem)");
			return false;
		}
#endif

		if (pkShop->GetItems()->size() >= OFFLINESHOP_MAX_ITEM_COUNT)
		{
			SendPopupPacket(ch, OFFLINESHOP_WINDOW_EDIT, SUBERROR_GC_SHOP_ITEM_LIMIT);
			return false;
		}

		LPITEM pkItem = ch->GetItem(pos);
		if (!pkItem)
			return false;

		if (pkItem->isLocked() || pkItem->IsEquipped() || pkItem->IsExchanging())
		{
			SendPopupPacket(ch, OFFLINESHOP_WINDOW_EDIT, SUBERROR_GC_SHOP_LOCKED_ITEM);
			return false;
		}

#ifdef __BEGINNER_ITEM__
		if (pkItem->IsBasicItem()) {
			SendPopupPacket(ch, OFFLINESHOP_WINDOW_EDIT, SUBERROR_GC_SHOP_BEGINNER_ITEM);
			return false;
		}
#endif
		if (IS_SET(pkItem->GetAntiFlag(), ITEM_ANTIFLAG_GIVE))
		{
			SendPopupPacket(ch, OFFLINESHOP_WINDOW_EDIT, SUBERROR_GC_SHOP_ANTIFLAG_ITEM);
			return false;
		}

		if (IS_SET(pkItem->GetAntiFlag(), ITEM_ANTIFLAG_MYSHOP))
		{
			SendPopupPacket(ch, OFFLINESHOP_WINDOW_EDIT, SUBERROR_GC_SHOP_ANTIFLAG_ITEM);
			return false;
		}

		//updated 25 - 01 - 2020  //topatch
#if !defined(__GOLD_LIMIT_REWORK__) && !defined(ENABLE_FULL_YANG) && !defined(REMOVE_YANG_LIMIT)
		if (!IsGoodSalePrice(price))
			return false;
#endif

		ch->SetOfflineShopTime();

		TItemInfo itemInfo;
		ZeroObject(itemInfo);

		itemInfo.dwOwnerID = ch->GetPlayerID();
		itemInfo.item.dwVnum = pkItem->GetVnum();
		itemInfo.item.dwCount = pkItem->GetCount();
		//patch 08-03-2020
		itemInfo.item.expiration = GetItemExpiration(pkItem);

		memcpy(itemInfo.item.aAttr, pkItem->GetAttributes(), sizeof(itemInfo.item.aAttr));
		memcpy(itemInfo.item.alSockets, pkItem->GetSockets(), sizeof(itemInfo.item.alSockets));

#ifdef __ITEM_CHANGELOOK__
		itemInfo.item.dwTransmutation = pkItem->GetTransmutation();
#endif
#ifdef __ITEM_EVOLUTION__
		itemInfo.item.dwEvolution = pkItem->GetEvolution();
#endif
		CopyObject(itemInfo.price, price);

		M2_DESTROY_ITEM(pkItem->RemoveFromCharacter());

		SendShopAddItemDBPacket(ch->GetPlayerID(), itemInfo);
		return true;
	}

	bool CShopManager::RecvShopRemoveItemClientPacket(LPCHARACTER ch, DWORD dwItemID)
	{
		if (!ch || !ch->GetOfflineShop())
			return false;

		CShop* pkShop = ch->GetOfflineShop();
		CShopItem* pItem = nullptr;

		OFFSHOP_DEBUG("owner %u , item id %u ", ch->GetPlayerID(), dwItemID);

		if (!CheckCharacterActions(ch))
		{
			SendPopupPacket(ch, OFFLINESHOP_WINDOW_EDIT, SUBERROR_GC_SHOP_CHARACTER_ACTIONS);
			return false;
		}

		if (ch->GetDungeon())
		{
			SendPopupPacket(ch, OFFLINESHOP_WINDOW_EDIT, SUBERROR_GC_SHOP_DUNGEON);
			return false;
		}

		if (pkShop->GetItems()->size() == 1)
		{
			SendPopupPacket(ch, OFFLINESHOP_WINDOW_EDIT, SUBERROR_GC_SHOP_REMOVE_LAST_ITEM);
			return false;
		}

		if (!pkShop->GetItem(dwItemID, &pItem))
		{
			SendPopupPacket(ch, OFFLINESHOP_WINDOW_EDIT, SUBERROR_GC_SHOP_REPORT_ADMIN);
			return false;
		}

		ch->SetOfflineShopTime();
		SendShopRemoveItemDBPacket(pkShop->GetOwnerPID(), pItem->GetID());
		return true;
	}

	//FILTER
	bool CShopManager::RecvShopFilterRequestClientPacket(LPCHARACTER ch, const TFilterInfo& filter)
	{
		if (!ch)
			return false;

		if (ch->GetDungeon())
		{
			SendPopupPacket(ch, OFFLINESHOP_WINDOW_VIEW, SUBERROR_GC_SHOP_DUNGEON);
			return false;
		}

		//offlineshop-updated 03/08/2019
		std::vector<TItemInfo> vec;

		if (ch->GetLastShopSearchPulse() < thecore_pulse())
			ch->ClearShopSearchCounter();

		if (ch->GetShopSearchCounter() > g_dwShopSearchLimitCounter && ch->GetLastShopSearchPulse() > thecore_pulse())
		{
			SendPopupPacket(ch, OFFLINESHOP_WINDOW_SEARCH, SUBERROR_GC_SHOP_CHARACTER_ACTIONS); // RUHA BAGLA
			return true;
		}

		ch->SetOfflineShopTime();

		const bool searchByName = strnlen(filter.szName, sizeof(filter.szName)) != 0;

		// sys_err("---------  FILTER   ------------");
		// sys_err("(type:%d %d) nm:%d n:%s af:%d (count:%d %d) (dss:%d %d) (level:%d %d) (yang:%lld %lld) (absorb:%d %d) dslevel:%d (dsVal:%d %d %d)", filter.bType, filter.bSubType, filter.wNameMode, filter.szName, filter.antiflag, filter.wCount[0], filter.wCount[1], filter.wDSS[0], filter.wDSS[1], filter.wLevel[0], filter.wLevel[1], filter.llYang[0], filter.llYang[1], filter.wAcceAbsorb[0], filter.wAcceAbsorb[1], filter.wDSLevel, filter.wDSVal[0], filter.wDSVal[1], filter.wDSVal[2]);

		itertype(m_mapShops) cit = m_mapShops.begin();
		for (; cit != m_mapShops.end(); cit++)
		{
			const CShop& rcShop = cit->second;

			//offlineshop-updated 04/08/19
			if (rcShop.GetOwnerPID() == ch->GetPlayerID())
				continue;

			CShop::VECSHOPITEM* pShopItems = rcShop.GetItems();

			itertype(*pShopItems) cItemIter = pShopItems->begin();
			for (; cItemIter != pShopItems->end(); cItemIter++)
			{
				const CShopItem& rItem = *cItemIter;
				const TItemInfoEx& rItemInfo = *rItem.GetInfo();
				const TPriceInfo& rItemPrice = *rItem.GetPrice();

				TItemTable* pTable = ITEM_MANAGER::instance().GetTable(rItemInfo.dwVnum);
				if (!pTable)
				{
					sys_err("CANNOT FIND ITEM TABLE [%d]");
					continue;
				}

				// sys_err("1");
				if (filter.wLevel[0] != 0 )
				{
					if (filter.wLevel[0] > pTable->GetLevelLimit())
						continue;
				}
				if (filter.wLevel[1] != 0)
				{
					if (filter.wLevel[1] < pTable->GetLevelLimit())
						continue;
				}

				// sys_err("2");
				if (filter.llYang[0] != 0)
				{
					if (filter.llYang[0] > rItemPrice.GetTotalYangAmount())
						continue;
				}

				if (filter.llYang[1] != 0)
				{
					if (filter.llYang[1] < rItemPrice.GetTotalYangAmount())
						continue;
				}

				if (filter.wCheque[0] != 0)
				{
					if (filter.wCheque[0] > static_cast<DWORD>(rItemPrice.GetTotalChequeAmount()))
						continue;
				}

				if (filter.wCheque[1] != 0)
				{
					if (filter.wCheque[1] < static_cast<DWORD>(rItemPrice.GetTotalChequeAmount()))
						continue;
				}

				// sys_err("3");
				if (!MatchSearchCategory(pTable, filter.bType, filter.bSubType))
					continue;

				// sys_err("4");
				if (searchByName)
				{
					if (filter.wNameMode == 2)
					{
						std::string stName = filter.szName;
						if (stName.compare(rcShop.GetOwnerName()) != 0)
							continue;
					}
					else
					{
						std::string stName = StringToLower(filter.szName, strnlen(filter.szName, sizeof(filter.szName)));
						if (!MatchItemName(filter.wNameMode, stName, pTable->szLocaleName, strnlen(pTable->szLocaleName, ITEM_NAME_MAX_LEN)))
							continue;
					}
				}
				else
				{
					if (filter.bType == 0)
					{
						ch->ChatPacket(CHAT_TYPE_INFO, "");
						return false;
					}
				}

				// sys_err("5");
				if ((pTable->dwAntiFlags | filter.antiflag) != pTable->dwAntiFlags)
				// if ((pTable->dwAntiFlags & filter.antiflag) == filter.antiflag)
				{
					continue;
				}

				// sys_err("6");
				if (pTable->bType == ITEM_WEAPON)
				{
				// sys_err("7");
					if (filter.wDSS[0] != 0)
					{
						if (FindApplyValue(rItemInfo.aAttr, APPLY_NORMAL_HIT_DAMAGE_BONUS) < filter.wDSS[0])
							continue;
					}

					if (filter.wDSS[1] != 0)
					{
						if (FindApplyValue(rItemInfo.aAttr, APPLY_NORMAL_HIT_DAMAGE_BONUS) > filter.wDSS[1])
							continue;
					}
				}
				else if (pTable->bType == ITEM_COSTUME && pTable->bSubType == COSTUME_ACCE)
				{
				// sys_err("8");
					if (filter.wDSS[0] != 0)
					{
						if (FindApplyValue(rItemInfo.aAttr, APPLY_NORMAL_HIT_DAMAGE_BONUS) < filter.wDSS[0])
							continue;
					}

					if (filter.wDSS[1] != 0)
					{
						if (FindApplyValue(rItemInfo.aAttr, APPLY_NORMAL_HIT_DAMAGE_BONUS) > filter.wDSS[1])
							continue;
					}

					if (filter.wAcceAbsorb[0] != 0)
					{
						if (rItemInfo.alSockets[ACCE_ABSORPTION_SOCKET] < filter.wAcceAbsorb[0])
							continue;
					}

					if (filter.wAcceAbsorb[1] != 0)
					{
						if (rItemInfo.alSockets[ACCE_ABSORPTION_SOCKET] > filter.wAcceAbsorb[1])
							continue;
					}
				}
				// else if (pTable->bType == ITEM_DS || pTable->bType == ITEM_SPECIAL_DS)
				else if (pTable->bType == ITEM_DS)
				{
					auto dsLevelFilter = filter.wDSLevel;
					auto dsVal1Filter = filter.wDSVal[0];
					auto dsVal2Filter = filter.wDSVal[1];
					auto dsVal3Filter = filter.wDSVal[2];

					auto dsLevelItem = GetDSLevelIdx(pTable->dwVnum);
					auto dsValTypeItem = GetDSGradeIdx(pTable->dwVnum);
					auto dsValValItem = GetDSStepIdx(pTable->dwVnum);

					if (dsLevelFilter != 10 && dsLevelFilter != dsLevelItem)
						continue;

					switch (dsValTypeItem)
					{
					case 5:
						if (dsVal1Filter != 10 && dsVal1Filter != dsValValItem)
							continue;
						break;
					case 6:
						if (dsVal2Filter != 10 && dsVal2Filter != dsValValItem)
							continue;
						break;
					case 7:
						if (dsVal3Filter != 10 && dsVal3Filter != dsValValItem)
							continue;
						break;
					}

				}

				// sys_err("10");
				if (filter.wCount[0] != 0)
				{
					if (rItemInfo.dwCount < filter.wCount[0])
						continue;
				}

				// sys_err("12");
				if (filter.wCount[1] != 0)
				{
					if (rItemInfo.dwCount > filter.wCount[1])
						continue;
				}

				// sys_err("12");
				TItemInfo itemInfo;
				CopyObject(itemInfo.item, rItemInfo);
				CopyObject(itemInfo.price, rItemPrice);

				itemInfo.dwItemID = rItem.GetID();
				itemInfo.dwOwnerID = rcShop.GetOwnerPID();
				strlcpy(itemInfo.szOwnerName, rcShop.GetName(), sizeof(itemInfo));

				vec.push_back(itemInfo);

				if (vec.size() >= OFFLINESHOP_MAX_SEARCH_RESULT)
					break;
			}

			if (vec.size() >= OFFLINESHOP_MAX_SEARCH_RESULT)
				break;
		}

		SendShopFilterResultClientPacket(ch, vec);
		// @@ flood search
		ch->IncreaseShopSearchCounter();
		ch->SetLastShopSearchPulse();
		return true;
	}

	void CShopManager::SendShopFilterResultClientPacket(LPCHARACTER ch, const std::vector<TItemInfo>& items)
	{
		if (!ch || !ch->GetDesc())
			return;

		TEMP_BUFFER buff;

		TPacketGCNewOfflineshop pack;
		pack.bHeader = HEADER_GC_NEW_OFFLINESHOP;
		pack.bSubHeader = SUBHEADER_GC_SHOP_FILTER_RESULT;
		pack.wSize = sizeof(pack) + sizeof(TSubPacketGCShopFilterResult) + sizeof(TItemInfo) * items.size();
		buff.write(&pack, sizeof(pack));

		TSubPacketGCShopFilterResult subpack;
		subpack.dwCount = items.size();
		buff.write(&subpack, sizeof(subpack));

		for (DWORD i = 0; i < items.size(); i++)
		{
			const TItemInfo& rItemInfo = items[i];
			buff.write(&rItemInfo, sizeof(rItemInfo));
		}

		// Oyundan Atma Fix
		ch->GetDesc()->LargePacket(buff.read_peek(), buff.size());
	}

	//SAFEBOX
	bool CShopManager::RecvShopSafeboxOpenClientPacket(LPCHARACTER ch)
	{
		if (!ch || ch->GetShopSafebox())
			return false;

		CShopSafebox* pkSafebox = GetShopSafeboxByOwnerID(ch->GetPlayerID());
		if (!pkSafebox)
			return false;

		if (ch->GetDungeon())
		{
			SendPopupPacket(ch, OFFLINESHOP_WINDOW_VIEW, SUBERROR_GC_SHOP_DUNGEON);
			return false;
		}

		ch->SetOfflineShopTime();
		ch->SetShopSafebox(pkSafebox);
		pkSafebox->RefreshToOwner(ch);
		return true;
	}

	bool CShopManager::RecvShopSafeboxGetItemClientPacket(LPCHARACTER ch, DWORD dwItemID)
	{
		if (!ch || !ch->GetShopSafebox())
			return false;

		if (!CheckCharacterActions(ch))
		{
			SendPopupPacket(ch, OFFLINESHOP_WINDOW_EDIT, SUBERROR_GC_SHOP_CHARACTER_ACTIONS);
			return false;
		}

		if (ch->GetDungeon())
		{
			SendPopupPacket(ch, OFFLINESHOP_WINDOW_VIEW, SUBERROR_GC_SHOP_DUNGEON);
			return false;
		}

		CShopSafebox* pkSafebox = ch->GetShopSafebox();
		CShopItem* pItem = nullptr;

		if (!pkSafebox->GetItem(dwItemID, &pItem))
		{
			SendPopupPacket(ch, OFFLINESHOP_WINDOW_EDIT, SUBERROR_GC_SHOP_CANT_FIND_ITEM);
			return false;
		}

		LPITEM pkItem = pItem->CreateItem();
		if (!pkItem)
			return false;

		int iEmptyPos = -1;
		if (pkItem->IsDragonSoul())
			iEmptyPos = ch->GetEmptyDragonSoulInventory(pkItem);
#ifdef __ADDITIONAL_INVENTORY__
		else if (pkItem->IsUpgradeItem())
			iEmptyPos = ch->GetEmptyUpgradeInventory(pkItem);
		else if (pkItem->IsBook())
			iEmptyPos = ch->GetEmptyBookInventory(pkItem);
		else if (pkItem->IsStone())
			iEmptyPos = ch->GetEmptyStoneInventory(pkItem);
		else if (pkItem->IsFlower())
			iEmptyPos = ch->GetEmptyFlowerInventory(pkItem);
		else if (pkItem->IsAttrItem())
			iEmptyPos = ch->GetEmptyAttrInventory(pkItem);
		else if (pkItem->IsChest())
			iEmptyPos = ch->GetEmptyChestInventory(pkItem);
#endif
		else
			iEmptyPos = ch->GetEmptyInventory(pkItem->GetSize());

		if (iEmptyPos == -1)
		{
			M2_DESTROY_ITEM(pkItem);
			return false;
		}

		ch->SetOfflineShopTime();

		if (pkSafebox->RemoveItem(dwItemID))
		{
			pkSafebox->RefreshToOwner();
			if (pkItem->IsDragonSoul())
				pkItem->__ADD_TO_CHARACTER(ch, TItemPos(DRAGON_SOUL_INVENTORY, iEmptyPos));
#ifdef __ADDITIONAL_INVENTORY__
			else if (pkItem->IsUpgradeItem())
				pkItem->__ADD_TO_CHARACTER(ch, TItemPos(UPGRADE_INVENTORY, iEmptyPos));
			else if (pkItem->IsBook())
				pkItem->__ADD_TO_CHARACTER(ch, TItemPos(BOOK_INVENTORY, iEmptyPos));
			else if (pkItem->IsStone())
				pkItem->__ADD_TO_CHARACTER(ch, TItemPos(STONE_INVENTORY, iEmptyPos));
			else if (pkItem->IsFlower())
				pkItem->__ADD_TO_CHARACTER(ch, TItemPos(FLOWER_INVENTORY, iEmptyPos));
			else if (pkItem->IsAttrItem())
				pkItem->__ADD_TO_CHARACTER(ch, TItemPos(ATTR_INVENTORY, iEmptyPos));
			else if (pkItem->IsChest())
				pkItem->__ADD_TO_CHARACTER(ch, TItemPos(CHEST_INVENTORY, iEmptyPos));
#endif
			else
				pkItem->__ADD_TO_CHARACTER(ch, TItemPos(INVENTORY, iEmptyPos));
		}

		SendShopSafeboxGetItemDBPacket(ch->GetPlayerID(), dwItemID);
		return true;
	}

	bool CShopManager::RecvShopSafeboxGetValutesClientPacket(LPCHARACTER ch, const TValutesInfo& valutes)
	{
		if (!ch || !ch->GetShopSafebox())
			return false;

		if (!CheckCharacterActions(ch))
		{
			SendPopupPacket(ch, OFFLINESHOP_WINDOW_EDIT, SUBERROR_GC_SHOP_CHARACTER_ACTIONS);
			return false;
		}

#if !defined(ENABLE_FULL_YANG) && !defined(FULL_YANG)
		if ((long long)ch->GetGold() + valutes.illYang >= (long long)GOLD_MAX)
		{
			SendPopupPacket(ch, OFFLINESHOP_WINDOW_EDIT, SUBERROR_GC_SHOP_GOLD_LIMIT_MAX);
			return false;
		}
#endif

#ifdef __CHEQUE_SYSTEM__
		if (ch->GetCheque() + valutes.iCheque >= CHEQUE_MAX)
		{
			SendPopupPacket(ch, OFFLINESHOP_WINDOW_EDIT, SUBERROR_GC_SHOP_CHEQUE_LIMIT_MAX);
			return false;
		}
#endif

		if (valutes.illYang < 0)
		{
			LogManager::instance().HackLogEx(ch, "Bir orospu evladi tespit edildi...");
			SendPopupPacket(ch, OFFLINESHOP_WINDOW_EDIT, SUBERROR_GC_SHOP_GOLD_LIMIT_MIN);
			return false;
		}

#ifdef __CHEQUE_SYSTEM__
		if (valutes.iCheque < 0)
		{
			LogManager::instance().HackLogEx(ch, "Bir orospu evladi tespit edildi...");
			SendPopupPacket(ch, OFFLINESHOP_WINDOW_EDIT, SUBERROR_GC_SHOP_CHEQUE_LIMIT_MIN);
			return false;
		}
#endif

		CShopSafebox* pkSafebox = ch->GetShopSafebox();
		CShopSafebox::SValuteAmount peekAmount(valutes);

		if (!pkSafebox->RemoveValute(peekAmount))
			return false;

		ch->SetOfflineShopTime();

		ch->PointChange(POINT_GOLD, valutes.illYang);
#ifdef __CHEQUE_SYSTEM__
		ch->PointChange(POINT_CHEQUE, valutes.iCheque);
#endif

		pkSafebox->RefreshToOwner();
		SendShopSafeboxGetValutesDBPacket(ch->GetPlayerID(), valutes);
		return true;
	}

	bool CShopManager::RecvShopSafeboxCloseClientPacket(LPCHARACTER ch)
	{
		if (!ch || !ch->GetShopSafebox())
			return false;

		ch->SetOfflineShopTime();
		ch->SetShopSafebox(NULL);
		return true;
	}

	void CShopManager::SendShopSafeboxRefresh(LPCHARACTER ch, const TValutesInfo& valute, const std::vector<CShopItem>& vec)
	{
		if (!ch || !ch->GetDesc() || !ch->GetShopSafebox())
			return;

		ch->SetOfflineShopTime();

		TPacketGCNewOfflineshop pack;
		pack.bHeader = HEADER_GC_NEW_OFFLINESHOP;
		pack.wSize = sizeof(pack) + sizeof(TSubPacketGCShopSafeboxRefresh) + ((sizeof(DWORD) + sizeof(TItemInfoEx)) * vec.size());
		pack.bSubHeader = SUBHEADER_GC_SHOP_SAFEBOX_REFRESH;

		TSubPacketGCShopSafeboxRefresh subpack;
		subpack.dwItemCount = vec.size();
		CopyObject(subpack.valute, valute);

		TEMP_BUFFER buff;
		buff.write(&pack, sizeof(pack));
		buff.write(&subpack, sizeof(subpack));

		TItemInfoEx item;
		DWORD dwItemID = 0;

		for (itertype(vec) it = vec.begin(); it != vec.end(); it++)
		{
			const CShopItem& shopitem = *it;

			dwItemID = shopitem.GetID();
			CopyObject(item, *shopitem.GetInfo());

			buff.write(&dwItemID, sizeof(dwItemID));
			buff.write(&item, sizeof(item));
		}

		ch->GetDesc()->Packet(buff.read_peek(), buff.size());
	}

	void CShopManager::RecvCloseBoardClientPacket(LPCHARACTER ch)
	{
		if (!ch || !ch->GetDesc())
			return;

		ch->SetOfflineShopTime();

		//safebox
		if (ch->GetShopSafebox())
			ch->SetShopSafebox(NULL);

		//shop
		if (ch->GetOfflineShopGuest())
		{
			ch->GetOfflineShopGuest()->RemoveGuest(ch);
			ch->SetOfflineShopGuest(NULL);
		}

		if (ch->GetOfflineShop())
			ch->GetOfflineShop()->RemoveGuest(ch);
	}

	void CShopManager::RecvTeleportClientPacket(LPCHARACTER ch)
	{
		if (!ch || !ch->GetDesc())
			return;

		if (!CheckCharacterActions(ch))
		{
			SendPopupPacket(ch, OFFLINESHOP_WINDOW_VIEW, SUBERROR_GC_SHOP_CHARACTER_ACTIONS);
			return;
		}

		if (ch->GetDungeon())
		{
			SendPopupPacket(ch, OFFLINESHOP_WINDOW_VIEW, SUBERROR_GC_SHOP_DUNGEON);
			return;
		}

		if (ch->GetWarMap())
		{
			SendPopupPacket(ch, OFFLINESHOP_WINDOW_VIEW, SUBERROR_GC_WAR_MAP);
			return;
		}

		CShop* pkShop = ch->GetOfflineShop();
		if (!pkShop)
		{
			sys_err("%s is hacker!!!! he/she's doesnt have offline shop.", ch->GetName());
			return;
		}

		const TShopPosition& rPosition = *pkShop->GetPosInfo();

		long x = rPosition.x, y = rPosition.y;
		PositionHelper(rPosition.lMapIndex, &x, &y);
		ch->WarpSet(x, y, rPosition.lMapIndex);
	}

	void CShopManager::UpdateShopsDuration()
	{
		SHOPMAP::iterator it = m_mapShops.begin();
		for (; it != m_mapShops.end(); it++)
		{
			CShop& shop = it->second;

			if (shop.GetDuration() > 0)
				shop.DecreaseDuration();
		}
	}

	void CShopManager::SendPopupPacket(LPCHARACTER ch, BYTE windowNum, BYTE errorNum)
	{
		TPacketGCNewOfflineshop pack;
		pack.bHeader = HEADER_GC_NEW_OFFLINESHOP;
		pack.bSubHeader = SUBHEADER_GC_SHOP_POPUP;
		pack.wSize = sizeof(pack) + sizeof(TSubPacketGCShopPopup);

		TSubPacketGCShopPopup subpack;
		subpack.bWindow = windowNum;
		subpack.bErrorHeader = errorNum;

		ch->GetDesc()->BufferedPacket(&pack, sizeof(pack));
		ch->GetDesc()->Packet(&subpack, sizeof(subpack));
	}
}

#endif //__OFFLINE_SHOP__
