#include "stdafx.h"
#include "start_position.h"

char g_nation_name[4][32] =
{
	"",
	"신수국",
	"천조국",
	"진노국",
};

long g_start_map[4] =
{
	0,	// reserved
	1,
	21,
	41
};

DWORD g_start_position[4][2] =
{
	{      0,      0 },	// reserved
	{ 469300, 964200 },
	{  55700, 157900 },
	{ 969600, 278400 }
};

DWORD g_create_position[4][2] =
{
	{		0,		0 },
	{ 96000, 39000 },
	{ 96000, 39000 },
	{ 96000, 39000 },
};