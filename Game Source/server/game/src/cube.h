#pragma once
#include "../../common/service.h"
#ifndef __CUBE_WINDOW__
#define CUBE_MAX_NUM	24	// OLD:INVENTORY_MAX_NUM
#define CUBE_MAX_DISTANCE	1000

struct CUBE_VALUE
{
	DWORD	vnum;
	int		count;

	bool operator == (const CUBE_VALUE& b)
	{
		return (this->count == b.count) && (this->vnum == b.vnum);
	}
};

struct CUBE_DATA
{
	std::vector<WORD>		npc_vnum;
	std::vector<CUBE_VALUE>	item;
	std::vector<CUBE_VALUE>	reward;
	int						percent;
#ifdef __GOLD_LIMIT_REWORK__
	unsigned long long			gold;
#else
	unsigned int			gold;
#endif
#ifdef __CUBE_WINDOW__
	bool					allowCopyAttr;
	bool					allowCopySocket;
#ifdef __ITEM_EVOLUTION__
	bool					allowCopyEvolution;
#endif
#ifdef __ITEM_CHANGELOOK__
	bool					allowCopyTransmutation;
#endif
#endif
	CUBE_DATA();

	bool		can_make_item(LPITEM* items, WORD npc_vnum);
	CUBE_VALUE* reward_value();
	void		remove_material(LPCHARACTER ch);
};

void Cube_init();
bool Cube_load(const char* file);

bool Cube_make(LPCHARACTER ch);

void Cube_clean_item(LPCHARACTER ch);
void Cube_open(LPCHARACTER ch);
void Cube_close(LPCHARACTER ch);

void Cube_show_list(LPCHARACTER ch);
#ifdef __ADDITIONAL_INVENTORY__
void Cube_add_item(LPCHARACTER ch, int cube_index, int inven_index, int inven_type);
#else
void Cube_add_item(LPCHARACTER ch, int cube_index, int inven_index);
#endif
void Cube_delete_item(LPCHARACTER ch, int cube_index);

void Cube_request_result_list(LPCHARACTER ch);
void Cube_request_material_info(LPCHARACTER ch, int request_start_index, int request_count = 1);

// test print code
void Cube_print();

bool Cube_InformationInitialize();
#endif