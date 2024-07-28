#include "stdafx.h"
#include "char.h"
#include "dungeon_info.h"
TJobInitialPoints JobInitialPoints[JOB_MAX_NUM] =
/*
   {
   int st, ht, dx, iq;
   int max_hp, max_sp;
   int hp_per_ht, sp_per_iq;
   int hp_per_lv_begin, hp_per_lv_end;
   int sp_per_lv_begin, sp_per_lv_end;
   int max_stamina;
   int stamina_per_con;
   int stamina_per_lv_begin, stamina_per_lv_end;
   }
 */
{
	// 36 >= 44
	// 18 >= 22
	// stat reset bugfix
	{   6,  6,  6,  6,  700,   200,     40,    20,    44, 44,     22, 22,     800,      5,      1, 3  }, // JOB_WARRIOR  16
	{   6,  6,  6,  6,  700,   200,     40,    20,    44, 44,     22, 22,     800,      5,      1, 3  }, // JOB_ASSASSIN 16
	{   6,  6,  6,  6,  700,   200,     40,    20,    44, 44,     22, 22,     800,      5,      1, 3  }, // JOB_SURA	 16
	{   6,  6,  6,  6,  700,   200,     40,    20,    44, 44,     22, 22,     800,      5,      1, 3  },  // JOB_SHAMANa  16
#ifdef __WOLFMAN_CHARACTER__
	{   6,  6,  6,  6,  700,   200,     40,    20,    44, 44,     22, 22,     800,      5,      1, 3  },
#endif
};

const TMobRankStat MobRankStats[MOB_RANK_MAX_NUM] =
/*
   {
   int         iGoldPercent;
   }
 */
{
	{  20,  }, // MOB_RANK_PAWN,
	{  20,  }, // MOB_RANK_S_PAWN,
	{  25,  }, // MOB_RANK_KNIGHT,
	{  30,  }, // MOB_RANK_S_KNIGHT,
	{  50,  }, // MOB_RANK_BOSS,
	{ 100,  }  // MOB_RANK_KING,
};

TBattleTypeStat BattleTypeStats[BATTLE_TYPE_MAX_NUM] =
/*
   {
   int         AttGradeBias;
   int         DefGradeBias;
   int         MagicAttGradeBias;
   int         MagicDefGradeBias;
   }
 */
{
	{	  0,	  0,	  0,	-10	}, // BATTLE_TYPE_MELEE,
	{	 10,	-20,	-10,	-15	}, // BATTLE_TYPE_RANGE,
	{	 -5,	 -5,	 10,	 10	}, // BATTLE_TYPE_MAGIC,
	{	  0,	  0,	  0,	  0	}, // BATTLE_TYPE_SPECIAL,
	{	 10,	-10,	  0,	-15	}, // BATTLE_TYPE_POWER,
	{	-10,	 10,	-10,	  0	}, // BATTLE_TYPE_TANKER,
	{	 20,	-20,	  0,	-10	}, // BATTLE_TYPE_SUPER_POWER,
	{	-20,	 20,	-10,	  0	}, // BATTLE_TYPE_SUPER_TANKER,
};

#ifdef __GROWTH_PET_SYSTEM__
DWORD* exppet_table = nullptr;
const DWORD Pet_Table[10][2] =
{
	{ 55701, 34041 },
	{ 55702, 34045 },
	{ 55703, 34049 },
	{ 55704, 34053 },
	{ 55705, 34053 },
	{ 55706, 34064 },
	{ 55707, 34073 },
	{ 55708, 34075 },
	{ 55709, 34080 },
	{ 55710, 34082 }
};

const DWORD Pet_Skill_Table[7][23] =
{
	//Level Like Index ->> Bonus Value --> typeSkill --> CoolDown -->> Percentage
	{ APPLY_ATTBONUS_STONE, 0, 0, 1, 2, 2, 2, 3, 3, 3, 4, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 10 }, // Metin Avcýsý
	{ APPLY_ATTBONUS_MONSTER, 0, 0, 1, 2, 2, 2, 3, 3, 3, 4, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 10 }, // Canavar Avcýsý
	{ APPLY_ATTBONUS_BOSS, 0, 0, 1, 2, 2, 2, 3, 3, 3, 4, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 10 }, // Patron Avcýsý
	{ APPLY_MALL_ATTBONUS, 0, 0, 1, 2, 2, 2, 3, 3, 3, 4, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 10}, // Berserker
	{ APPLY_ATTBONUS_HUMAN, 0, 0, 1, 2, 2, 2, 3, 3, 3, 4, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 10 }, // Yarý Ýnsanlara Karþý Güç
	{ APPLY_RESIST_HUMAN, 0, 0, 1, 2, 2, 2, 3, 3, 3, 4, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 10 }, // Yarý Ýnsanlara Karþý Direnç
	{ APPLY_CAST_SPEED, 0, 0, 1, 2, 2, 2, 3, 3, 3, 4, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 10} // Hýzlandýrma
};

const DWORD Pet_SKill_TablePerc[3][20] =
{
	{ 2, 5, 8, 11, 13, 16, 19, 22, 24, 27, 30, 33, 35, 38, 41, 44, 46, 49, 52, 55 },
	{ 2, 5, 7, 10, 12, 15, 17, 20, 23, 25, 27, 30, 32, 35, 37, 40, 42, 45, 47, 50 },
	{ 2, 5, 8, 11, 13, 16, 19, 22, 25, 27, 30, 33, 36, 38, 41, 44, 47, 50, 52, 55 }
};

DWORD exppet_table_common[250];
#endif

#ifdef __GROWTH_MOUNT_SYSTEM__
DWORD* expmount_table = nullptr;
const DWORD Mount_Table[11][2] =
{
	{ 55770, 9917 },
	{ 55771, 9918 },
	{ 55772, 9919 },
	{ 55773, 9920 },
	{ 55774, 9921 },
	{ 55775, 9922 },
	{ 55776, 9923 },
	{ 55777, 9924 },
	{ 55778, 9925 },
	{ 55779, 9926 },
	{ 55780, 9927 }
};

const DWORD Mount_Skill_Table[7][23] =
{
	//Level Like Index ->> Bonus Value --> typeSkill --> CoolDown -->> Percentage
	{ APPLY_ATTBONUS_STONE, 0, 0, 1, 2, 2, 2, 3, 3, 3, 4, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 10 }, // Metin Avcýsý
	{ APPLY_ATTBONUS_MONSTER, 0, 0, 1, 2, 2, 2, 3, 3, 3, 4, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 10 }, // Canavar Avcýsý
	{ APPLY_ATTBONUS_BOSS, 0, 0, 1, 2, 2, 2, 3, 3, 3, 4, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 10 }, // Patron Avcýsý
	{ APPLY_MALL_ATTBONUS, 0, 0, 1, 2, 2, 2, 3, 3, 3, 4, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 10}, // Berserker
	{ APPLY_ATTBONUS_HUMAN, 0, 0, 1, 2, 2, 2, 3, 3, 3, 4, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 10 }, // Yarý Ýnsanlara Karþý Güç
	{ APPLY_RESIST_HUMAN, 0, 0, 1, 2, 2, 2, 3, 3, 3, 4, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 10 }, // Yarý Ýnsanlara Karþý Direnç
	{ APPLY_CAST_SPEED, 0, 0, 1, 2, 2, 2, 3, 3, 3, 4, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 10} // Hýzlandýrma
};

const DWORD Mount_SKill_TablePerc[3][20] =
{
	{ 2, 5, 8, 11, 13, 16, 19, 22, 24, 27, 30, 33, 35, 38, 41, 44, 46, 49, 52, 55 },
	{ 2, 5, 7, 10, 12, 15, 17, 20, 23, 25, 27, 30, 32, 35, 37, 40, 42, 45, 47, 50 },
	{ 2, 5, 8, 11, 13, 16, 19, 22, 25, 27, 30, 33, 36, 38, 41, 44, 47, 50, 52, 55 }
};

DWORD expmount_table_common[250];
#endif

const DWORD* exp_table = NULL;

const DWORD exp_table_common[PLAYER_MAX_LEVEL_CONST + 1] =
{
	0,	//	0
	300,
	800,
	1500,
	2500,
	4300,
	7200,
	11000,
	17000,
	24000,
	33000,	//	10
	43000,
	58000,
	76000,
	100000,
	130000,
	169000,
	219000,
	283000,
	365000,
	472000,	//	20
	610000,
	705000,
	813000,
	937000,
	1077000,
	1237000,
	1418000,
	1624000,
	1857000,
	2122000,	//	30
	2421000,
	2761000,
	3145000,
	3580000,
	4073000,
	4632000,
	5194000,
	5717000,
	6264000,
	6837000,	//	40
	7600000,
	8274000,
	8990000,
	9753000,
	10560000,
	11410000,
	12320000,
	13270000,
	14280000,
	15340000,	//	50
	16870000,
	18960000,
	19980000,
	21420000,
	22930000,
	24530000,
	26200000,
	27960000,
	29800000,
	32780000,	//	60
	36060000,
	39670000,
	43640000,
	48000000,
	52800000,
	58080000,
	63890000,
	70280000,
	77310000,
	85040000,	//	70
	93540000,
	102900000,
	113200000,
	124500000,
	137000000,
	150700000,
	165700000,
	236990000,
	260650000,
	286780000,	//	80
	315380000,
	346970000,
	381680000,
	419770000,
	461760000,
	508040000,
	558740000,
	614640000,
	676130000,
	743730000,	//	90
	1041222000,
	1145344200,
	1259878620,
	1385866482,
	1524453130,
	1676898443,
	1844588288,
	2029047116,
	2050000000,
	2150000000u,	//	100
	2210000000u,
	2250000000u,
	2280000000u,
	2310000000u,
	2330000000u,	//	105
	2350000000u,
	2370000000u,
	2390000000u,
	2400000000u,
	2410000000u,	//	110
	2420000000u,
	2430000000u,
	2440000000u,
	2450000000u,
	2460000000u,	//	115
	2470000000u,
	2480000000u,
	2490000000u,
	2490000000u,
	2500000000u,	//	120
	// extra
	2500000000u,2500000000u,2500000000u,2500000000u,2500000000u,2500000000u,2500000000u,2500000000u,2500000000u,2500000000u, // 130
	2500000000u,2500000000u,2500000000u,2500000000u,2500000000u,2500000000u,2500000000u,2500000000u,2500000000u,2500000000u, // 140
	2500000000u,2500000000u,2500000000u,2500000000u,2500000000u,2500000000u,2500000000u,2500000000u,2500000000u,2500000000u, // 150
	2500000000u,2500000000u,2500000000u,2500000000u,2500000000u,2500000000u,2500000000u,2500000000u,2500000000u,2500000000u, // 160
	2500000000u,2500000000u,2500000000u,2500000000u,2500000000u,2500000000u,2500000000u,2500000000u,2500000000u,2500000000u, // 170
	2500000000u,2500000000u,2500000000u,2500000000u,2500000000u,2500000000u,2500000000u,2500000000u,2500000000u,2500000000u, // 180
	2500000000u,2500000000u,2500000000u,2500000000u,2500000000u,2500000000u,2500000000u,2500000000u,2500000000u,2500000000u, // 190
	2500000000u,2500000000u,2500000000u,2500000000u,2500000000u,2500000000u,2500000000u,2500000000u,2500000000u,2500000000u, // 200
	2500000000u,2500000000u,2500000000u,2500000000u,2500000000u,2500000000u,2500000000u,2500000000u,2500000000u,2500000000u, // 210
	2500000000u,2500000000u,2500000000u,2500000000u,2500000000u,2500000000u,2500000000u,2500000000u,2500000000u,2500000000u, // 220
	2500000000u,2500000000u,2500000000u,2500000000u,2500000000u,2500000000u,2500000000u,2500000000u,2500000000u,2500000000u, // 230
	2500000000u,2500000000u,2500000000u,2500000000u,2500000000u,2500000000u,2500000000u,2500000000u,2500000000u,2500000000u, // 240
	2500000000u,2500000000u,2500000000u,2500000000u,2500000000u,2500000000u,2500000000u,2500000000u,2500000000u,2500000000u, // 250
};

const int* aiPercentByDeltaLev = NULL;
const int* aiPercentByDeltaLevForBoss = NULL;

const int aiPercentByDeltaLevForBoss_turkey[MAX_EXP_DELTA_OF_LEV] =
{
	1,      // -15  0
	3,          // -14  1
	5,          // -13  2
	7,          // -12  3
	15,         // -11  4
	30,         // -10  5
	60,         // -9   6
	90,         // -8   7
	91,         // -7   8
	92,         // -6   9
	93,         // -5   10
	94,         // -4   11
	95,         // -3   12
	97,         // -2   13
	99,         // -1   14
	100,        // 0    15
	105,        // 1    16
	110,        // 2    17
	115,        // 3    18
	120,        // 4    19
	125,        // 5    20
	130,        // 6    21
	135,        // 7    22
	140,        // 8    23
	145,        // 9    24
	150,        // 10   25
	155,        // 11   26
	160,        // 12   27
	165,        // 13   28
	170,        // 14   29
	180         // 15   30
};

const int aiPercentByDeltaLev_turkey[MAX_EXP_DELTA_OF_LEV] =
{
	1,  //  -15 0
	5,  //  -14 1
	10, //  -13 2
	20, //  -12 3
	30, //  -11 4
	50, //  -10 5
	70, //  -9  6
	80, //  -8  7
	85, //  -7  8
	90, //  -6  9
	92, //  -5  10
	94, //  -4  11
	96, //  -3  12
	98, //  -2  13
	100,    //  -1  14
	100,    //  0   15
	105,    //  1   16
	110,    //  2   17
	115,    //  3   18
	120,    //  4   19
	125,    //  5   20
	130,    //  6   21
	135,    //  7   22
	140,    //  8   23
	145,    //  9   24
	150,    //  10  25
	155,    //  11  26
	160,    //  12  27
	165,    //  13  28
	170,    //  14  29
	180,    //  15  30
};

const DWORD party_exp_distribute_table[PLAYER_EXP_TABLE_MAX + 1] =
{
	0,
	10,		10,		10,		10,		15,		15,		20,		25,		30,		40,		// 1 - 10
	50,		60,		80,		100,	120,	140,	160,	184,	210,	240,	// 11 - 20
	270,	300,	330,	360,	390,	420,	450,	480,	510,	550,	// 21 - 30
	600,	640,	700,	760,	820,	880,	940,	1000,	1100,	1180,	// 31 - 40
	1260,	1320,	1380,	1440,	1500,	1560,	1620,	1680,	1740,	1800,	// 41 - 50
	1860,	1920,	2000,	2100,	2200,	2300,	2450,	2600,	2750,	2900,	// 51 - 60
	3050,	3200,	3350,	3500,	3650,	3800,	3950,	4100,	4250,	4400,	// 61 - 70
	4600,	4800,	5000,	5200,	5400,	5600,	5800,	6000,	6200,	6400,	// 71 - 80
	6600,	6900,	7100,	7300,	7600,	7800,	8000,	8300,	8500,	8800,	// 81 - 90
	9000,	9000,	9000,	9000,	9000,	9000,	9000,	9000,	9000,	9000,	// 91 - 100
	10000,	10000,	10000,	10000,	10000,	10000,	10000,	10000,	10000,	10000,	// 101 - 110
	12000,	12000,	12000,	12000,	12000,	12000,	12000,	12000,	12000,	12000,	// 111 - 120
	// 14000,	14000,	14000,	14000,	14000,	14000,	14000,	14000,	14000,	14000,	// 130
	// 14000,	14000,	14000,	14000,	14000,	14000,	14000,	14000,	14000,	14000,	// 140
	// 14000,	14000,	14000,	14000,	14000,	14000,	14000,	14000,	14000,	14000,	// 150
	// 14000,	14000,	14000,	14000,	14000,	14000,	14000,	14000,	14000,	14000,	// 160
	// 14000,	14000,	14000,	14000,	14000,	14000,	14000,	14000,	14000,	14000,	// 170
	// 14000,	14000,	14000,	14000,	14000,	14000,	14000,	14000,	14000,	14000,	// 180
	// 14000,	14000,	14000,	14000,	14000,	14000,	14000,	14000,	14000,	14000,	// 190
	// 14000,	14000,	14000,	14000,	14000,	14000,	14000,	14000,	14000,	14000,	// 200
	// 14000,	14000,	14000,	14000,	14000,	14000,	14000,	14000,	14000,	14000,	// 210
	// 14000,	14000,	14000,	14000,	14000,	14000,	14000,	14000,	14000,	14000,	// 220
	// 14000,	14000,	14000,	14000,	14000,	14000,	14000,	14000,	14000,	14000,	// 230
	// 14000,	14000,	14000,	14000,	14000,	14000,	14000,	14000,	14000,	14000,	// 240
	// 14000,	14000,	14000,	14000,	14000,	14000,	14000,	14000,	14000,	14000,	// 250
};

Coord aArroundCoords[ARROUND_COORD_MAX_NUM] =
{
	{	     0,	      0	    },
	{        0,      50     },
	{       35,     35      },
	{       50,     -0      },
	{       35,     -35     },
	{       0,      -50     },
	{       -35,    -35     },
	{       -50,    0       },
	{       -35,    35      },
	{       0,      100     },
	{       71,     71      },
	{       100,    -0      },
	{       71,     -71     },
	{       0,      -100    },
	{       -71,    -71     },
	{       -100,   0       },
	{       -71,    71      },
	{       0,      150     },
	{       106,    106     },
	{       150,    -0      },
	{       106,    -106    },
	{       0,      -150    },
	{       -106,   -106    },
	{       -150,   0       },
	{       -106,   106     },
	{       0,      200     },
	{       141,    141     },
	{       200,    -0      },
	{       141,    -141    },
	{       0,      -200    },
	{       -141,   -141    },
	{       -200,   0       },
	{       -141,   141     },
	{       0,      250     },
	{       177,    177     },
	{       250,    -0      },
	{       177,    -177    },
	{       0,      -250    },
	{       -177,   -177    },
	{       -250,   0       },
	{       -177,   177     },
	{       0,      300     },
	{       212,    212     },
	{       300,    -0      },
	{       212,    -212    },
	{       0,      -300    },
	{       -212,   -212    },
	{       -300,   0       },
	{       -212,   212     },
	{       0,      350     },
	{       247,    247     },
	{       350,    -0      },
	{       247,    -247    },
	{       0,      -350    },
	{       -247,   -247    },
	{       -350,   0       },
	{       -247,   247     },
	{       0,      400     },
	{       283,    283     },
	{       400,    -0      },
	{       283,    -283    },
	{       0,      -400    },
	{       -283,   -283    },
	{       -400,   0       },
	{       -283,   283     },
	{       0,      450     },
	{       318,    318     },
	{       450,    -0      },
	{       318,    -318    },
	{       0,      -450    },
	{       -318,   -318    },
	{       -450,   0       },
	{       -318,   318     },
	{       0,      500     },
	{       354,    354     },
	{       500,    -0      },
	{       354,    -354    },
	{       0,      -500    },
	{       -354,   -354    },
	{       -500,   0       },
	{       -354,   354     },
	{       0,      550     },
	{       389,    389     },
	{       550,    -0      },
	{       389,    -389    },
	{       0,      -550    },
	{       -389,   -389    },
	{       -550,   0       },
	{       -389,   389     },
	{       0,      600     },
	{       424,    424     },
	{       600,    -0      },
	{       424,    -424    },
	{       0,      -600    },
	{       -424,   -424    },
	{       -600,   0       },
	{       -424,   424     },
	{       0,      650     },
	{       460,    460     },
	{       650,    -0      },
	{       460,    -460    },
	{       0,      -650    },
	{       -460,   -460    },
	{       -650,   0       },
	{       -460,   460     },
	{       0,      700     },
	{       495,    495     },
	{       700,    -0      },
	{       495,    -495    },
	{       0,      -700    },
	{       -495,   -495    },
	{       -700,   0       },
	{       -495,   495     },
	{       0,      750     },
	{       530,    530     },
	{       750,    -0      },
	{       530,    -530    },
	{       0,      -750    },
	{       -530,   -530    },
	{       -750,   0       },
	{       -530,   530     },
	{       0,      800     },
	{       566,    566     },
	{       800,    -0      },
	{       566,    -566    },
	{       0,      -800    },
	{       -566,   -566    },
	{       -800,   0       },
	{       -566,   566     },
	{       0,      850     },
	{       601,    601     },
	{       850,    -0      },
	{       601,    -601    },
	{       0,      -850    },
	{       -601,   -601    },
	{       -850,   0       },
	{       -601,   601     },
	{       0,      900     },
	{       636,    636     },
	{       900,    -0      },
	{       636,    -636    },
	{       0,      -900    },
	{       -636,   -636    },
	{       -900,   0       },
	{       -636,   636     },
	{       0,      950     },
	{       672,    672     },
	{       950,    -0      },
	{       672,    -672    },
	{       0,      -950    },
	{       -672,   -672    },
	{       -950,   0       },
	{       -672,   672     },
	{       0,      1000    },
	{       707,    707     },
	{       1000,   -0      },
	{       707,    -707    },
	{       0,      -1000   },
	{       -707,   -707    },
	{       -1000,  0       },
	{       -707,   707     },
};

const DWORD guild_exp_table[GUILD_MAX_LEVEL + 1] =
{
	0,
	15000UL,
	45000UL,
	90000UL,
	160000UL,
	235000UL,
	325000UL,
	430000UL,
	550000UL,
	685000UL,
	835000UL,
	1000000UL,
	1500000UL,
	2100000UL,
	2800000UL,
	3600000UL,
	4500000UL,
	6500000UL,
	8000000UL,
	10000000UL,
	42000000UL, // 20
	44000000UL,
	46000000UL,
	48000000UL,
	50000000UL,
	52000000UL,
	54000000UL,
	56000000UL,
	58000000UL,
	60000000UL,
	62000000UL, // 30
	64000000UL,
	66000000UL,
	68000000UL,
	70000000UL,
	72000000UL,
	74000000UL,
	76000000UL,
	78000000UL,
	80000000UL,
	82000000UL, // 40
};

const DWORD guild_exp_table2[GUILD_MAX_LEVEL + 1] =
{
	0,
	6000UL,
	18000UL,
	36000UL,
	64000UL,
	94000UL,
	130000UL,
	172000UL,
	220000UL,
	274000UL,
	334000UL,
	400000UL,
	600000UL,
	840000UL,
	1120000UL,
	1440000UL,
	1800000UL,
	2600000UL,
	3200000UL,
	4000000UL,
	4200000UL, // 20
	4400000UL,
	4600000UL,
	4800000UL,
	5000000UL,
	6000000UL,
	7000000UL,
	8000000UL,
	9000000UL,
	10000000UL,
	11000000UL, // 30
	12000000UL,
	13000000UL,
	14000000UL,
	15000000UL,
	16000000UL,
	17000000UL,
	18000000UL,
	19000000UL,
	20000000UL,
	21000000UL, // 40
};

const int aiMobEnchantApplyIdx[MOB_ENCHANTS_MAX_NUM] =
{
	APPLY_CURSE_PCT,
	APPLY_SLOW_PCT,
	APPLY_POISON_PCT,
	APPLY_STUN_PCT,
	APPLY_CRITICAL_PCT,
	APPLY_PENETRATE_PCT,
#if defined(__WOLFMAN_CHARACTER__) && !defined(USE_MOB_BLEEDING_AS_POISON)
	APPLY_BLEEDING_PCT,
#endif
};

const int aiMobResistsApplyIdx[MOB_RESISTS_MAX_NUM] =
{
	APPLY_RESIST_SWORD,
	APPLY_RESIST_TWOHAND,
	APPLY_RESIST_DAGGER,
	APPLY_RESIST_BELL,
	APPLY_RESIST_FAN,
	APPLY_RESIST_BOW,
	APPLY_RESIST_FIRE,
	APPLY_RESIST_ELEC,
	APPLY_RESIST_MAGIC,
	APPLY_RESIST_WIND,
	APPLY_POISON_REDUCE,
#if defined(__WOLFMAN_CHARACTER__) && !defined(USE_MOB_CLAW_AS_DAGGER)
	APPLY_RESIST_CLAW,
#endif
#if defined(__WOLFMAN_CHARACTER__) && !defined(USE_MOB_BLEEDING_AS_POISON)
	APPLY_BLEEDING_REDUCE,
#endif
};

const int aiSocketPercentByQty[5][4] =
{
	{  0,  0,  0,  0 },
	{  3,  0,  0,  0 },
	{ 10,  1,  0,  0 },
	{ 15, 10,  1,  0 },
	{ 20, 15, 10,  1 }
};

const int aiWeaponSocketQty[WEAPON_NUM_TYPES] =
{
#ifdef __ITEM_EXTRA_SOCKET__
	4, // WEAPON_SWORD,
	4, // WEAPON_DAGGER,
	4, // WEAPON_BOW,
	4, // WEAPON_TWO_HANDED,
	4, // WEAPON_BELL,
	4, // WEAPON_FAN,
#else
	3, // WEAPON_SWORD,
	3, // WEAPON_DAGGER,
	3, // WEAPON_BOW,
	3, // WEAPON_TWO_HANDED,
	3, // WEAPON_BELL,
	3, // WEAPON_FAN,
#endif
	0, // WEAPON_ARROW,
	0, // WEAPON_MOUNT_SPEAR
#ifdef __ITEM_EXTRA_SOCKET__
#ifdef __WOLFMAN_CHARACTER__
	4, // WEAPON_CLAW
#endif
#else
#ifdef __WOLFMAN_CHARACTER__
	3, // WEAPON_CLAW
#endif
#endif
};

const int aiArmorSocketQty[ARMOR_NUM_TYPES] =
{
#ifdef __ITEM_EXTRA_SOCKET__
	4, // ARMOR_BODY,
#else
	3, // ARMOR_BODY,
#endif
	1, // ARMOR_HEAD,
	1, // ARMOR_SHIELD,
	0, // ARMOR_WRIST,
	0, // ARMOR_FOOTS,
	0  // ARMOR_ACCESSORY
};

TItemAttrMap g_map_itemAttr;
TItemAttrMap g_map_itemRare;

const TApplyInfo aApplyInfo[MAX_APPLY_NUM] =
/*
{
   DWORD dwPointType;
}
 */
{
	// Point Type
	{ POINT_NONE,			},   // APPLY_NONE,		0
	{ POINT_MAX_HP,		        },   // APPLY_MAX_HP,		1
	{ POINT_MAX_SP,		        },   // APPLY_MAX_SP,		2
	{ POINT_HT,			        },   // APPLY_CON,		3
	{ POINT_IQ,			        },   // APPLY_INT,		4
	{ POINT_ST,			        },   // APPLY_STR,		5
	{ POINT_DX,			        },   // APPLY_DEX,		6
	{ POINT_ATT_SPEED,		        },   // APPLY_ATT_SPEED,	7
	{ POINT_MOV_SPEED,		        },   // APPLY_MOV_SPEED,	8
	{ POINT_CASTING_SPEED,	        },   // APPLY_CAST_SPEED,	9
	{ POINT_HP_REGEN,			},   // APPLY_HP_REGEN,		10
	{ POINT_SP_REGEN,			},   // APPLY_SP_REGEN,		11
	{ POINT_POISON_PCT,		        },   // APPLY_POISON_PCT,	12
	{ POINT_STUN_PCT,		        },   // APPLY_STUN_PCT,		13
	{ POINT_SLOW_PCT,		        },   // APPLY_SLOW_PCT,		14
	{ POINT_CRITICAL_PCT,		},   // APPLY_CRITICAL_PCT,	15
	{ POINT_PENETRATE_PCT,	        },   // APPLY_PENETRATE_PCT,	16
	{ POINT_ATTBONUS_HUMAN,	        },   // APPLY_ATTBONUS_HUMAN,	17
	{ POINT_ATTBONUS_ANIMAL,	        },   // APPLY_ATTBONUS_ANIMAL,	18
	{ POINT_ATTBONUS_ORC,		},   // APPLY_ATTBONUS_ORC,	19
	{ POINT_ATTBONUS_MILGYO,	        },   // APPLY_ATTBONUS_MILGYO,	20
	{ POINT_ATTBONUS_UNDEAD,	        },   // APPLY_ATTBONUS_UNDEAD,	21
	{ POINT_ATTBONUS_DEVIL,	        },   // APPLY_ATTBONUS_DEVIL,	22
	{ POINT_STEAL_HP,		        },   // APPLY_STEAL_HP,		23
	{ POINT_STEAL_SP,		        },   // APPLY_STEAL_SP,		24
	{ POINT_MANA_BURN_PCT,	        },   // APPLY_MANA_BURN_PCT,	25
	{ POINT_DAMAGE_SP_RECOVER,	        },   // APPLY_DAMAGE_SP_RECOVER,26
	{ POINT_BLOCK,		        },   // APPLY_BLOCK,		27
	{ POINT_DODGE,		        },   // APPLY_DODGE,		28
	{ POINT_RESIST_SWORD,		},   // APPLY_RESIST_SWORD,	29
	{ POINT_RESIST_TWOHAND,	        },   // APPLY_RESIST_TWOHAND,	30
	{ POINT_RESIST_DAGGER,	        },   // APPLY_RESIST_DAGGER,	31
	{ POINT_RESIST_BELL,		},   // APPLY_RESIST_BELL,	32
	{ POINT_RESIST_FAN,		        },   // APPLY_RESIST_FAN,	33
	{ POINT_RESIST_BOW,		        },   // APPLY_RESIST_BOW,	34
	{ POINT_RESIST_FIRE,		},   // APPLY_RESIST_FIRE,	35
	{ POINT_RESIST_ELEC,		},   // APPLY_RESIST_ELEC,	36
	{ POINT_RESIST_MAGIC,		},   // APPLY_RESIST_MAGIC,	37
	{ POINT_RESIST_WIND,		},   // APPLY_RESIST_WIND,	38
	{ POINT_REFLECT_MELEE,	        },   // APPLY_REFLECT_MELEE,	39
	{ POINT_REFLECT_CURSE,	        },   // APPLY_REFLECT_CURSE,	40
	{ POINT_POISON_REDUCE,	        },   // APPLY_POISON_REDUCE,	41
	{ POINT_KILL_SP_RECOVER,	        },   // APPLY_KILL_SP_RECOVER,	42
	{ POINT_EXP_DOUBLE_BONUS,	        },   // APPLY_EXP_DOUBLE_BONUS,	43
	{ POINT_GOLD_DOUBLE_BONUS,	        },   // APPLY_GOLD_DOUBLE_BONUS,44
	{ POINT_ITEM_DROP_BONUS,	        },   // APPLY_ITEM_DROP_BONUS,	45
	{ POINT_POTION_BONUS,		},   // APPLY_POTION_BONUS,	46
	{ POINT_KILL_HP_RECOVERY,	        },   // APPLY_KILL_HP_RECOVER,	47
	{ POINT_IMMUNE_STUN,		},   // APPLY_IMMUNE_STUN,	48
	{ POINT_IMMUNE_SLOW,		},   // APPLY_IMMUNE_SLOW,	49
	{ POINT_IMMUNE_FALL,		},   // APPLY_IMMUNE_FALL,	50
	{ POINT_NONE,			},   // APPLY_SKILL,		51
	{ POINT_BOW_DISTANCE,		},   // APPLY_BOW_DISTANCE,	52
	{ POINT_ATT_GRADE_BONUS,	        },   // APPLY_ATT_GRADE,	53
	{ POINT_DEF_GRADE_BONUS,	        },   // APPLY_DEF_GRADE,	54
	{ POINT_MAGIC_ATT_GRADE_BONUS,      },   // APPLY_MAGIC_ATT_GRADE,	55
	{ POINT_MAGIC_DEF_GRADE_BONUS,      },   // APPLY_MAGIC_DEF_GRADE,	56
	{ POINT_CURSE_PCT,			},   // APPLY_CURSE_PCT,	57
	{ POINT_MAX_STAMINA			},   // APPLY_MAX_STAMINA	58
	{ POINT_ATTBONUS_WARRIOR		},   // APPLY_ATTBONUS_WARRIOR  59
	{ POINT_ATTBONUS_ASSASSIN		},   // APPLY_ATTBONUS_ASSASSIN 60
	{ POINT_ATTBONUS_SURA		},   // APPLY_ATTBONUS_SURA     61
	{ POINT_ATTBONUS_SHAMAN		},   // APPLY_ATTBONUS_SHAMAN   62
	{ POINT_ATTBONUS_MONSTER		},   //	APPLY_ATTBONUS_MONSTER  63
	{ POINT_MALL_ATTBONUS		},   // 64 // APPLY_MALL_ATTBONUS
	{ POINT_MALL_DEFBONUS		},   // 65
	{ POINT_MALL_EXPBONUS		},   // 66 APPLY_MALL_EXPBONUS
	{ POINT_MALL_ITEMBONUS		},   // 67
	{ POINT_MALL_GOLDBONUS		},   // 68
	{ POINT_MAX_HP_PCT			},		// 69
	{ POINT_MAX_SP_PCT			},		// 70
	{ POINT_SKILL_DAMAGE_BONUS		},	// 71
	{ POINT_NORMAL_HIT_DAMAGE_BONUS	},	// 72

	// DEFEND_BONUS_ATTRIBUTES
	{ POINT_SKILL_DEFEND_BONUS		},	// 73
	{ POINT_NORMAL_HIT_DEFEND_BONUS	},	// 74
	// END_OF_DEFEND_BONUS_ATTRIBUTES

	{ POINT_NONE	},		// 75
	{ POINT_NONE	},		// 76

	{ POINT_NONE,		},

	{ POINT_RESIST_WARRIOR,		},
	{ POINT_RESIST_ASSASSIN,	},
	{ POINT_RESIST_SURA,		},
	{ POINT_RESIST_SHAMAN,		},
	{ POINT_ENERGY				},
	{ POINT_DEF_GRADE			},
	{ POINT_COSTUME_ATTR_BONUS	},
	{ POINT_MAGIC_ATT_BONUS_PER },
	{ POINT_MELEE_MAGIC_ATT_BONUS_PER		},			// 86 APPLY_MELEE_MAGIC_ATTBONUS_PER
	{ POINT_RESIST_ICE,			},   // APPLY_RESIST_ICE,	87
	{ POINT_RESIST_EARTH,		},   // APPLY_RESIST_EARTH,	88
	{ POINT_RESIST_DARK,		},   // APPLY_RESIST_DARK,	89
	{ POINT_RESIST_CRITICAL,		},   // APPLY_ANTI_CRITICAL_PCT,	90
	{ POINT_RESIST_PENETRATE,		},   // APPLY_ANTI_PENETRATE_PCT,	91

#ifdef __WOLFMAN_CHARACTER__
	{ POINT_BLEEDING_REDUCE,		},	// APPLY_BLEEDING_REDUCE, 		92
	{ POINT_BLEEDING_PCT,			},	// APPLY_BLEEDING_PCT, 			93
	{ POINT_ATTBONUS_WOLFMAN,		},
	{ POINT_RESIST_WOLFMAN,			},
	{ POINT_RESIST_CLAW,			},
#else
	{ POINT_NONE,					},	// APPLY_BLEEDING_REDUCE, 		92
	{ POINT_NONE,					},	// APPLY_BLEEDING_PCT, 			93
	{ POINT_NONE,					},
	{ POINT_NONE,					},
	{ POINT_NONE,					},
#endif

#ifdef __ACCE_SYSTEM__
	{ POINT_ACCEDRAIN_RATE,			},	// APPLY_ACCEDRAIN_RATE,		97
#else
	{ POINT_NONE,			},	// APPLY_ACCEDRAIN_RATE,		97
#endif

#ifdef __MAGIC_REDUCTION_SYSTEM__
	{ POINT_RESIST_MAGIC_REDUCTION,	},	// APPLY_RESIST_MAGIC_REDUCTION,98
#else
	{ POINT_NONE,					},	// APPLY_RESIST_MAGIC_REDUCTION,98
#endif

	{ POINT_NONE,					}, // APPLY_MOUNT,99
#ifdef __PENDANT_SYSTEM__
	{ POINT_ENCHANT_FIRE, },
	{ POINT_ENCHANT_ICE, },
	{ POINT_ENCHANT_EARTH, },
	{ POINT_ENCHANT_DARK, },
	{ POINT_ENCHANT_WIND, },
	{ POINT_ENCHANT_ELECT, },
	{ POINT_RESIST_HUMAN, },
	{ POINT_ATTBONUS_SWORD, },
	{ POINT_ATTBONUS_TWOHAND, },
	{ POINT_ATTBONUS_DAGGER, },
	{ POINT_ATTBONUS_BELL, },
	{ POINT_ATTBONUS_FAN, },
	{ POINT_ATTBONUS_BOW, },
#ifdef __WOLFMAN_CHARACTER__
	{ POINT_ATTBONUS_CLAW, },
#endif
	{ POINT_ATTBONUS_CZ, },
	{ POINT_ATTBONUS_DESERT, },
	{ POINT_ATTBONUS_INSECT, },
#endif
#ifdef __ATTRIBUTES_TYPES__
	{ POINT_ATTBONUS_STONE, },
	{ POINT_ATTBONUS_BOSS, },
	{ POINT_ATTBONUS_ELEMENTS, },
	{ POINT_ENCHANT_ELEMENTS, },
	{ POINT_ATTBONUS_CHARACTERS, },
	{ POINT_ENCHANT_CHARACTERS, },
#endif
#ifdef __CHEST_DROP_POINT__
	{ POINT_CHEST_BONUS, },
#endif
};

const int aiItemMagicAttributePercentHigh[ITEM_ATTRIBUTE_MAX_LEVEL] =
{
	//25, 25, 40, 8, 2,
	30, 40, 20, 8, 2
};

const int aiItemMagicAttributePercentLow[ITEM_ATTRIBUTE_MAX_LEVEL] =
{
	//45, 25, 20, 10, 0,
	50, 40, 10, 0, 0
};

// ADD_ITEM_ATTRIBUTE
const int aiItemAttributeAddPercent[ITEM_ATTRIBUTE_MAX_NUM] =
{
	75, 75, 75, 75, 75, 0, 0,
};
// END_OF_ADD_ITEM_ATTRIBUTE

#ifdef __COSTUME_ATTR_SWITCH__
int aiCostumeAttributeAddPercent[COSTUME_ATTRIBUTE_MAX_NUM] =
{
	40, 40, 40,
};

int aiCostumeAttributeLevelPercent[ITEM_ATTRIBUTE_MAX_LEVEL] =
{
	75, 75, 75, 75, 75,
};
#endif

const int aiExpLossPercents[PLAYER_EXP_TABLE_MAX + 1] =
{
	0,
	5, 5, 5, 5, 5, 5, 5, 5, 5, 4, // 1 - 10
	4, 4, 4, 4, 4, 4, 4, 4, 4, 4, // 11 - 20
	4, 4, 4, 4, 4, 4, 4, 3, 3, 3, // 21 - 30
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, // 31 - 40
	3, 3, 3, 3, 2, 2, 2, 2, 2, 2, // 41 - 50
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, // 51 - 60
	2, 2, 1, 1, 1, 1, 1, 1, 1, 1, // 61 - 70
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 71 - 80
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 81 - 90
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 91 - 100
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 101 - 110
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 111 - 120
	// 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 130
	// 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 140
	// 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 150
	// 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 160
	// 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 170
	// 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 180
	// 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 190
	// 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 200
	// 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 210
	// 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 220
	// 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 230
	// 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 240
	// 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 250
};

const int aiSkillBookCountForLevelUp[10] =
{
	3, 3, 3, 3, 3, 4, 4, 5, 5, 6
};

// ADD_GRANDMASTER_SKILL
const int aiGrandMasterSkillBookCountForLevelUp[10] =
{
	3, 3, 5, 5, 7, 7, 10, 10, 10, 20,
};

const int aiGrandMasterSkillBookMinCount[10] =
{
	//	1, 1, 3, 5, 10, 15, 20, 30, 40, 50,
	//	3, 3, 5, 5, 10, 10, 15, 15, 20, 30
		1, 1, 1, 2,  2,  3,  3,  4,  5,  6
};

const int aiGrandMasterSkillBookMaxCount[10] =
{
	//	6, 15, 30, 45, 60, 80, 100, 120, 160, 200,
	//	6, 10, 15, 20, 30, 40, 50, 60, 70, 80
		5,  7,  9, 11, 13, 15, 20, 25, 30, 35
};
// END_OF_ADD_GRANDMASTER_SKILL

#ifdef __PARTY_ROLE_REWORK__
const int CHN_aiPartyBonusExpPercentByMemberCount[13] =
{
	0, 0, 24, 36, 52, 80, 106, 140, 200, 220, 240, 260, 280
};
#else
const int CHN_aiPartyBonusExpPercentByMemberCount[9] =
{
	0, 0, 12, 18, 26, 40, 53, 70, 100
};
#endif

// UPGRADE_PARTY_BONUS
const int KOR_aiPartyBonusExpPercentByMemberCount[9] =
{
	0,
	0,
	30, // 66% * 2 - 100
	60, // 53% * 3 - 100
	75, // 44% * 4 - 100
	90, // 38% * 5 - 100
	105, // 34% * 6 - 100
	110, // 30% * 7 - 100
	140, // 30% * 8 - 100
};

const int KOR_aiUniqueItemPartyBonusExpPercentByMemberCount[9] =
{
	0,
	0,
	15 * 2,
	14 * 3,
	13 * 4,
	12 * 5,
	11 * 6,
	10 * 7,
	10 * 8,
};
// END_OF_UPGRADE_PARTY_BONUS

const int* aiChainLightningCountBySkillLevel = NULL;

const int aiChainLightningCountBySkillLevel_turkey[SKILL_MAX_LEVEL + 1] =
{
	0,	// 0
	2,	// 1
	2,	// 2
	2,	// 3
	2,	// 4
	2,	// 5
	2,	// 6
	2,	// 7
	2,	// 8
	3,	// 9
	3,	// 10
	3,	// 11
	3,	// 12
	3,	// 13
	3,	// 14
	3,	// 15
	3,	// 16
	3,	// 17
	3,	// 18
	4,	// 19
	4,	// 20
	4,	// 21
	4,	// 22
	4,	// 23
	5,	// 24
	5,	// 25
	5,	// 26
	5,	// 27
	5,	// 28
	5,	// 29
	5,	// 30
	5,	// 31
	5,	// 32
	5,	// 33
	5,	// 34
	5,	// 35
	5,	// 36
	5,	// 37
	5,	// 38
	5,	// 39
	5,	// 40
};

const SStoneDropInfo aStoneDrop[STONE_INFO_MAX_NUM] =
{
	//  mob		pct	{+0	+1	+2	+3	+4}
	{8005,	60,	{30,	30,	30,	9,	1}	},
	{8006,	60,	{28,	29,	31,	11,	1}	},
	{8007,	60,	{24,	29,	32,	13,	2}	},
	{8008,	60,	{22,	28,	33,	15,	2}	},
	{8009,	60,	{21,	27,	33,	17,	2}	},
	{8010,	60,	{18,	26,	34,	20,	2}	},
	{8011,	60,	{14,	26,	35,	22,	3}	},
	{8012,	60,	{10,	26,	37,	24,	3}	},
	{8013,	60,	{2,	26,	40,	29,	3}	},
	{8014,	60,	{0,	26,	41,	30,	3}	},
};

const char* c_apszEmpireNames[EMPIRE_MAX_NUM] =
{
	"ÀüÁ¦±¹",
	"½Å¼ö±¹",
	"ÃµÁ¶±¹",
	"Áø³ë±¹"
};

const char* c_apszPrivNames[MAX_PRIV_NUM] =
{
	"",
	"¾ÆÀÌÅÛÀÌ ³ª¿Ã È®·ü",
	"µ·ÀÌ ³ª¿Ã È®·ü",
	"µ· ´ë¹ÚÀÌ ³ª¿Ã È®·ü",
	"°æÇèÄ¡ ¹èÀ²",
};

TGuildWarInfo KOR_aGuildWarInfo[GUILD_WAR_TYPE_MAX_NUM] =
/*
   {
   long lMapIndex;
   int iWinnerPotionRewardPctToWinner;
   int iLoserPotionRewardPctToWinner;
   int iInitialScore;
   int iEndScore;
   };
 */
{
	{ 0,            0,      0,      0,      0       },
	{ 110,          100,    50,     0,      100     },
	{ 111,        100,    50,     0,      10      },
};

//

//

const int aiAccessorySocketAddPct[ITEM_ACCESSORY_SOCKET_MAX_NUM] =
{
#ifdef __ITEM_EXTRA_SOCKET__
	50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50
#else
	50, 50, 50
#endif
};

const int aiAccessorySocketEffectivePct[ITEM_ACCESSORY_SOCKET_MAX_NUM + 1] =
{
#ifdef __ITEM_EXTRA_SOCKET__
	0, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100, 110, 120, 130, 140, 150, 160, 170, 180, 190, 200
#else
	0, 10, 20, 40
#endif
};

const int aiAccessorySocketDegradeTime[ITEM_ACCESSORY_SOCKET_MAX_NUM + 1] =
{
#ifdef __ITEM_EXTRA_SOCKET__
	0, 3600 * 60, 3600 * 57, 3600 * 54, 3600 * 51, 3600 * 48, 3600 * 45, 3600 * 42, 3600 * 39, 3600 * 36, 3600 * 33, 3600 * 30, 3600 * 27, 3600 * 24, 3600 * 21, 3600 * 18, 3600 * 15,
	3600 * 12, 3600 * 9, 3600 * 6, 3600 * 3
#else
	0, 3600 * 24, 3600 * 12, 3600 * 6
#endif
};

const int aiAccessorySocketPutPct[ITEM_ACCESSORY_SOCKET_MAX_NUM + 1] =
{
#ifdef __ITEM_EXTRA_SOCKET__
	90, 90, 80, 80, 70, 70, 60, 60, 50, 50, 40, 40, 30, 30, 20, 20, 15, 15, 10, 10, 0
#else
	90, 80, 70, 0
#endif
};
// END_OF_ACCESSORY_REFINE

#include "../../common/length.h"
// from import_item_proto.c
typedef struct SValueName
{
	const char* c_pszName;
	long		lValue;
} TValueName;

TValueName c_aApplyTypeNames[] =
{
	{ "STR",		APPLY_STR		},
	{ "DEX",		APPLY_DEX		},
	{ "CON",		APPLY_CON		},
	{ "INT",		APPLY_INT		},
	{ "MAX_HP",		APPLY_MAX_HP		},
	{ "MAX_SP",		APPLY_MAX_SP		},
	{ "MAX_STAMINA",	APPLY_MAX_STAMINA	},
	{ "POISON_REDUCE",	APPLY_POISON_REDUCE	},
	{ "EXP_DOUBLE_BONUS", APPLY_EXP_DOUBLE_BONUS },
	{ "GOLD_DOUBLE_BONUS", APPLY_GOLD_DOUBLE_BONUS },
	{ "ITEM_DROP_BONUS", APPLY_ITEM_DROP_BONUS	},
	{ "HP_REGEN",	APPLY_HP_REGEN		},
	{ "SP_REGEN",	APPLY_SP_REGEN		},
	{ "ATTACK_SPEED",	APPLY_ATT_SPEED		},
	{ "MOVE_SPEED",	APPLY_MOV_SPEED		},
	{ "CAST_SPEED",	APPLY_CAST_SPEED	},
	{ "ATT_BONUS",	APPLY_ATT_GRADE_BONUS	},
	{ "DEF_BONUS",	APPLY_DEF_GRADE_BONUS	},
	{ "MAGIC_ATT_GRADE",APPLY_MAGIC_ATT_GRADE	},
	{ "MAGIC_DEF_GRADE",APPLY_MAGIC_DEF_GRADE	},
	{ "SKILL",		APPLY_SKILL		},
	{ "ATTBONUS_ANIMAL",APPLY_ATTBONUS_ANIMAL	},
	{ "ATTBONUS_MILGYO",APPLY_ATTBONUS_MILGYO	},
	{ "ATTBONUS_UNDEAD",APPLY_ATTBONUS_UNDEAD	},
	{ "ATTBONUS_DEVIL", APPLY_ATTBONUS_DEVIL	},
	{ "ATTBONUS_HUMAN", APPLY_ATTBONUS_HUMAN	},
	{ "ADD_BOW_DISTANCE",APPLY_BOW_DISTANCE	},
	{ "DODGE",		APPLY_DODGE		},
	{ "BLOCK",		APPLY_BLOCK		},
	{ "RESIST_SWORD",	APPLY_RESIST_SWORD	},
	{ "RESIST_TWOHAND",	APPLY_RESIST_TWOHAND	},
	{ "RESIST_DAGGER",	APPLY_RESIST_DAGGER    },
	{ "RESIST_BELL",	APPLY_RESIST_BELL	},
	{ "RESIST_FAN",	APPLY_RESIST_FAN	},
	{ "RESIST_BOW",	APPLY_RESIST_BOW	},
	{ "RESIST_FIRE",	APPLY_RESIST_FIRE	},
	{ "RESIST_ELEC",	APPLY_RESIST_ELEC	},
	{ "RESIST_MAGIC",	APPLY_RESIST_MAGIC	},
	{ "RESIST_WIND",	APPLY_RESIST_WIND	},
	{ "REFLECT_MELEE",	APPLY_REFLECT_MELEE },
	{ "REFLECT_CURSE",	APPLY_REFLECT_CURSE },
	{ "RESIST_ICE",		APPLY_RESIST_ICE	},
	{ "RESIST_EARTH",	APPLY_RESIST_EARTH	},
	{ "RESIST_DARK",	APPLY_RESIST_DARK	},
	{ "RESIST_CRITICAL",	APPLY_ANTI_CRITICAL_PCT	},
	{ "RESIST_PENETRATE",	APPLY_ANTI_PENETRATE_PCT	},
	{ "POISON",		APPLY_POISON_PCT	},
	{ "SLOW",		APPLY_SLOW_PCT		},
	{ "STUN",		APPLY_STUN_PCT		},
	{ "STEAL_HP",	APPLY_STEAL_HP		},
	{ "STEAL_SP",	APPLY_STEAL_SP		},
	{ "MANA_BURN_PCT",	APPLY_MANA_BURN_PCT	},
	{ "CRITICAL",	APPLY_CRITICAL_PCT	},
	{ "PENETRATE",	APPLY_PENETRATE_PCT	},
	{ "KILL_SP_RECOVER",APPLY_KILL_SP_RECOVER	},
	{ "KILL_HP_RECOVER",APPLY_KILL_HP_RECOVER	},
	{ "PENETRATE_PCT",	APPLY_PENETRATE_PCT	},
	{ "CRITICAL_PCT",	APPLY_CRITICAL_PCT	},
	{ "POISON_PCT",	APPLY_POISON_PCT	},
	{ "STUN_PCT",	APPLY_STUN_PCT		},
	{ "ATT_BONUS_TO_WARRIOR",	APPLY_ATTBONUS_WARRIOR  },
	{ "ATT_BONUS_TO_ASSASSIN",	APPLY_ATTBONUS_ASSASSIN },
	{ "ATT_BONUS_TO_SURA",	APPLY_ATTBONUS_SURA	    },
	{ "ATT_BONUS_TO_SHAMAN",	APPLY_ATTBONUS_SHAMAN   },
	{ "ATT_BONUS_TO_MONSTER",	APPLY_ATTBONUS_MONSTER  },
	{ "ATT_BONUS_TO_MOB",	APPLY_ATTBONUS_MONSTER  },
	{ "MALL_ATTBONUS",	APPLY_MALL_ATTBONUS	},
	{ "MALL_EXPBONUS",	APPLY_MALL_EXPBONUS	},
	{ "MALL_DEFBONUS",	APPLY_MALL_DEFBONUS	},
	{ "MALL_ITEMBONUS",	APPLY_MALL_ITEMBONUS	},
	{ "MALL_GOLDBONUS", APPLY_MALL_GOLDBONUS	},
	{ "MAX_HP_PCT",	APPLY_MAX_HP_PCT	},
	{ "MAX_SP_PCT",	APPLY_MAX_SP_PCT	},
	{ "SKILL_DAMAGE_BONUS",	APPLY_SKILL_DAMAGE_BONUS	},
	{ "NORMAL_HIT_DAMAGE_BONUS",APPLY_NORMAL_HIT_DAMAGE_BONUS	},
	{ "SKILL_DEFEND_BONUS",	APPLY_SKILL_DEFEND_BONUS	},
	{ "NORMAL_HIT_DEFEND_BONUS",APPLY_NORMAL_HIT_DEFEND_BONUS	},

	{ "RESIST_WARRIOR",	APPLY_RESIST_WARRIOR},
	{ "RESIST_ASSASSIN",	APPLY_RESIST_ASSASSIN},
	{ "RESIST_SURA",		APPLY_RESIST_SURA},
	{ "RESIST_SHAMAN",	APPLY_RESIST_SHAMAN},
	{ "INFINITE_AFFECT_DURATION", 0x1FFFFFFF	},
	{ "ENERGY", APPLY_ENERGY },
	{ "COSTUME_ATTR_BONUS", APPLY_COSTUME_ATTR_BONUS },
	{ "MAGIC_ATTBONUS_PER",	APPLY_MAGIC_ATTBONUS_PER	},
	{ "MELEE_MAGIC_ATTBONUS_PER",	APPLY_MELEE_MAGIC_ATTBONUS_PER	},

#ifdef __WOLFMAN_CHARACTER__
	{ "BLEEDING_REDUCE",APPLY_BLEEDING_REDUCE },
	{ "BLEEDING_PCT",APPLY_BLEEDING_PCT },
	{ "ATT_BONUS_TO_WOLFMAN",APPLY_ATTBONUS_WOLFMAN },
	{ "RESIST_WOLFMAN",APPLY_RESIST_WOLFMAN },
	{ "RESIST_CLAW",APPLY_RESIST_CLAW },
#endif

#ifdef __ACCE_SYSTEM__
	{ "ACCEDRAIN_RATE",APPLY_ACCEDRAIN_RATE },
#endif
#ifdef __MAGIC_REDUCTION_SYSTEM__
	{ "RESIST_MAGIC_REDUCTION",APPLY_RESIST_MAGIC_REDUCTION },
#endif
#ifdef __PENDANT_SYSTEM__
	{ "ENCHANT_FIRE",APPLY_ENCHANT_FIRE },
	{ "ENCHANT_ICE",APPLY_ENCHANT_ICE },
	{ "ENCHANT_EARTH",APPLY_ENCHANT_EARTH },
	{ "ENCHANT_DARK",APPLY_ENCHANT_DARK },
	{ "ENCHANT_WIND",APPLY_ENCHANT_WIND },
	{ "ENCHANT_ELECT",APPLY_ENCHANT_ELECT },
	{ "RESIST_HUMAN",APPLY_RESIST_HUMAN },
	{ "ATTBONUS_SWORD",APPLY_ATTBONUS_SWORD },
	{ "ATTBONUS_TWOHAND",APPLY_ATTBONUS_TWOHAND },
	{ "ATTBONUS_DAGGER",APPLY_ATTBONUS_DAGGER },
	{ "ATTBONUS_BELL",APPLY_ATTBONUS_BELL },
	{ "ATTBONUS_FAN",APPLY_ATTBONUS_FAN },
	{ "ATTBONUS_BOW",APPLY_ATTBONUS_BOW },
	{ "ATTBONUS_CLAW",APPLY_ATTBONUS_CLAW },
	{ "ATTBONUS_CZ",APPLY_ATTBONUS_CZ },
	{ "ATTBONUS_DESERT",APPLY_ATTBONUS_DESERT },
	{ "ATTBONUS_INSECT",APPLY_ATTBONUS_INSECT },
#endif
#ifdef __ATTRIBUTES_TYPES__
	{ "ATTBONUS_BOSS",	APPLY_ATTBONUS_BOSS	},
	{ "ATTBONUS_STONE",	APPLY_ATTBONUS_STONE	},
	{ "ATTBONUS_ELEMENTS",	APPLY_ATTBONUS_ELEMENTS	},
	{ "ENCHANT_ELEMENTS",	APPLY_ENCHANT_ELEMENTS	},
	{ "ATTBONUS_CHARACTERS",	APPLY_ATTBONUS_CHARACTERS	},
	{ "ENCHANT_CHARACTERS",	APPLY_ENCHANT_CHARACTERS	},
#endif
#ifdef __CHEST_DROP_POINT__
	{ "CHEST_BONUS",			APPLY_CHEST_BONUS	},
#endif
	{ NULL,		0			}
};
// from import_item_proto.c

long FN_get_apply_type(const char* apply_type_string)
{
	TValueName* value_name;
	for (value_name = c_aApplyTypeNames; value_name->c_pszName; ++value_name)
	{
		if (0 == strcasecmp(value_name->c_pszName, apply_type_string))
			return value_name->lValue;
	}
	return 0;
}

const char* GetMapName(int mapindex)
{
	switch (mapindex)
	{
	case 1: return LC_TEXT("metin2_map_a1");
	case 3: return LC_TEXT("metin2_map_a3");
	case 4: return LC_TEXT("metin2_map_guild_01");
	case 6: return LC_TEXT("metin2_guild_village_01");
	case 21: return LC_TEXT("metin2_map_b1");
	case 23: return LC_TEXT("metin2_map_b3");
	case 24: return LC_TEXT("metin2_map_guild_02");
	case 26: return LC_TEXT("metin2_guild_village_02");
	case 41: return LC_TEXT("metin2_map_c1");
	case 43: return LC_TEXT("metin2_map_c3");
	case 44: return LC_TEXT("metin2_map_guild_03");
	case 46: return LC_TEXT("metin2_guild_village_03");
	case 51: return LC_TEXT("metin2_map_wolf");
	case 61: return LC_TEXT("map_n_snowm_01");
	case 62: return LC_TEXT("metin2_map_n_flame_01");
	case 63: return LC_TEXT("metin2_map_n_desert_01");
	case 64: return LC_TEXT("map_n_threeway");
	case 65: return LC_TEXT("metin2_map_milgyo");
	case 66: return LC_TEXT("metin2_map_deviltower1");
	case 67: return LC_TEXT("metin2_map_trent");
	case 68: return LC_TEXT("metin2_map_trent02");
	case 69: return LC_TEXT("metin2_map_WL_01");
	case 70: return LC_TEXT("metin2_map_nusluck01");
	case 71: return LC_TEXT("metin2_map_spiderdungeon_02");
	case 72: return LC_TEXT("metin2_map_skipia_dungeon_01");
	case 73: return LC_TEXT("metin2_map_skipia_dungeon_02");
	case 100: return LC_TEXT("metin2_map_fielddungeon");
	case 101: return LC_TEXT("metin2_map_resources_zon");
	case 103: return LC_TEXT("metin2_map_t1");
	case 104: return LC_TEXT("metin2_map_spiderdungeon");
	case 105: return LC_TEXT("metin2_map_t2");
	case 110: return LC_TEXT("metin2_map_t3");
	case 111: return LC_TEXT("metin2_map_t4");
	case 112: return LC_TEXT("metin2_map_duel");
	case 113: return LC_TEXT("metin2_map_oxevent");
	case 114: return LC_TEXT("metin2_map_sungzi");
	case 118: return LC_TEXT("metin2_map_sungzi_flame_hill_01");
	case 119: return LC_TEXT("metin2_map_sungzi_flame_hill_02");
	case 120: return LC_TEXT("metin2_map_sungzi_flame_hill_03");
	case 121: return LC_TEXT("metin2_map_sungzi_snow");
	case 122: return LC_TEXT("metin2_map_sungzi_snow_pass01");
	case 123: return LC_TEXT("metin2_map_sungzi_snow_pass02");
	case 124: return LC_TEXT("metin2_map_sungzi_snow_pass03");
	case 125: return LC_TEXT("metin2_map_sungzi_desert_01");
	case 126: return LC_TEXT("metin2_map_sungzi_desert_hill_01");
	case 127: return LC_TEXT("metin2_map_sungzi_desert_hill_02");
	case 128: return LC_TEXT("metin2_map_sungzi_desert_hill_03");
	case 181: return LC_TEXT("metin2_map_empirewar01");
	case 182: return LC_TEXT("metin2_map_empirewar02");
	case 183: return LC_TEXT("metin2_map_empirewar03");
	case 200: return LC_TEXT("gm_guild_build");
	case 207: return LC_TEXT("metin2_map_skipia_dungeon_boss");
	case 212: return LC_TEXT("metin2_map_n_flame_dragon");
	case 216: return LC_TEXT("metin2_map_devilcatacomb");
	case 217: return LC_TEXT("metin2_map_spiderdungeon_03");
	case 246: return LC_TEXT("metin2_map_battlefied");
	case 301: return LC_TEXT("Metin2_map_CapeDragonHead");
	case 302: return LC_TEXT("metin2_map_dawnmistwood");
	case 303: return LC_TEXT("metin2_map_BayBlackSand");
	case 304: return LC_TEXT("metin2_map_Mt_Thunder");
	case 305: return LC_TEXT("metin2_map_sungzi_flame_pass01");
	case 306: return LC_TEXT("metin2_map_sungzi_flame_pass02");
	case 307: return LC_TEXT("metin2_map_sungzi_flame_pass03");
	case 351: return LC_TEXT("metin2_map_n_flame_dungeon_01");
	case 352: return LC_TEXT("metin2_map_n_snow_dungeon_01");
	case 353: return LC_TEXT("metin2_map_dawnmist_dungeon_01");
	case 354: return LC_TEXT("metin2_map_mt_th_dungeon_01");
	case 355: return LC_TEXT("metin2_map_dawnmist_dungeon_02");
	case 356: return LC_TEXT("metin2_12zi_stage");
	default:
	{
		for (auto &&i : dungeonTable)
		{
			if (i.map_index == mapindex)
				return i.map_name.c_str();
		}

		return LC_TEXT("metin2_map_unknown");
	}
	}
}

#ifdef __ALIGNMENT_REWORK__
const DWORD cAlignBonusTable[ALIGN_GRADE_COUNT + 1][ALIGN_BONUS_COUNT] =
{
	{POINT_ATTBONUS_MONSTER,	POINT_MAX_HP,	POINT_ATTBONUS_STONE,	POINT_ATTBONUS_BOSS, POINT_ATTBONUS_ELEMENTS, POINT_MALL_ATTBONUS, POINT_NORMAL_HIT_DAMAGE_BONUS},	/*Bonus Types*/
	{1500,	5000,	1500,	1500,	750, 100, 75},
	{1300,	5000,	1300,	1300,	650, 80, 60},
	{1200,	5000,	1200,	1200,	600, 75, 55},
	{1100,	5000,	1100,	1100,	550, 70, 50},
	{1000,	5000,	1000,	1000,	500, 65, 45},
	{900,	5000,	900,	900,	450, 60, 40},
	{800,	5000,	800,	800,	400, 55, 35},
	{700,	5000,	700,	700,	350, 50, 30},
	{600,	5000,	600,	600,	300, 45, 25},
	{500,	5000,	500,	500,	250, 40, 20},
	{400,	5000,	400,	400,	200, 35, 15},
	{300,	5000,	300,	300,	150, 30, 10},

	{200,	5000,	200,	200,	100, 25, 0},
	{75,	4000,	35,	35,	35, 10, 0},
	{60,	3600,	30,	30,	30, 7, 0},
	{50,	3200,	25,	25,	25, 5, 0},
	{40,	2800,	20,	20,	20, 3, 0},
	{30,	2400,	15,	15,	15, 1, 0},
	{20,	2000,	10,	10,	10, 0, 0},
	{16,	1600,	8,	8,	0, 0, 0},
	{14,	1400,	7,	7,	0, 0, 0},
	{12,	1200,	6,	6,	0, 0, 0},
	{10,	1000,	5,	5,	0, 0, 0},
	{8,		800,	4,	4,	0, 0, 0},
	{6,		600,	3,	3,	0, 0, 0},
	{4,		400,	2,	2,	0, 0, 0},
	{2,		200,	1,	1,	0, 0, 0},
	{0,		0,		0,	0,	0, 0, 0},
	{0,		0,		0,	0,	0, 0, 0},
	{0,		0,		0,	0,	0, 0, 0},
	{0,		0,		0,	0,	0, 0, 0},
};
#endif

#ifdef __RANK_SYSTEM__
const DWORD cRankBonusTable[RANK_GRADE_COUNT][RANK_BONUS_COUNT] =
{
	{
		POINT_ATTBONUS_MONSTER,
		POINT_ATTBONUS_STONE,
		POINT_ATTBONUS_BOSS,
		POINT_MALL_ATTBONUS,
		POINT_NORMAL_HIT_DAMAGE_BONUS,
	},
	{10,	10,	10,	0,	0},
	{20,	20,	20,	0,	0},
	{30,	30,	30,	0,	0},
	{40,	40,	40,	0,	0},
	{50,	50,	50,	0,	0},
	{60,	60,	60,	0,	0},
	{70,	70,	70,	0,	0},
	{80,	80,	80,	0,	0},
	{90,	90,	90,	0,	0},
	{100,	100,	100,	0,	0},
	{110,	110,	110,	0,	0},
	{120,	120,	120,	0,	0},
	{130,	130,	130,	0,	0},
	{140,	140,	140,	0,	0},
	{150,	150,	150,	0,	0},
	{160,	160,	160,	0,	0},
	{170,	170,	170,	0,	0},
	{180,	180,	180,	0,	0},
	{190,	190,	190,	0,	0},
	{200,	200,	200,	0,	0},
	{250,	250,	250,	20,	20},
	{500,	500,	500,	50,	50},
};
#endif

#ifdef __LANDRANK_SYSTEM__
const DWORD cLandRankBonusTable[LANDRANK_GRADE_COUNT + 1 - 1][LANDRANK_BONUS_COUNT] =
{
	{
	POINT_ATTBONUS_MONSTER,
	POINT_ATTBONUS_STONE,
	POINT_ATTBONUS_BOSS,
	POINT_MALL_ATTBONUS,
	POINT_NORMAL_HIT_DAMAGE_BONUS,
	POINT_SKILL_DAMAGE_BONUS,
	POINT_ATTBONUS_HUMAN,
	},
	{100,	100,	100,	10,	10,	5,	5},
	{200,	200,	200,	20,	20,	10,	10},
	{300,	300,	300,	30,	30,	15,	15},
};
#endif

#ifdef __REBORN_SYSTEM__
DWORD cRebornBonusTable[REBORN_GRADE_COUNT + 1][REBORN_BONUS_COUNT] =
{
	{
	POINT_ATTBONUS_MONSTER,
	POINT_ATTBONUS_STONE,
	POINT_ATTBONUS_BOSS,
	POINT_ATT_GRADE_BONUS,
	POINT_MALL_ATTBONUS,
	POINT_NORMAL_HIT_DAMAGE_BONUS,
	},
	{4,		2,	2,	10, 0, 0}, // 1
	{8,		4,	4,	20, 0, 0},
	{12,	6,	6,	30, 0, 0},
	{16,	8,	8,	40, 0, 0},
	{20,	10,	10,	50, 0, 0},
	{24,	12,	12,	60, 0, 0},
	{28,	14,	14,	70, 0, 0},
	{32,	16,	16,	80, 0, 0},
	{36,	18,	18,	90, 0, 0},
	{40,	20,	20,	100, 0, 0}, // 10
	{40,	20,	20,	100, 0, 0}, // 11 // load from charmanager
	{40,	20,	20,	100, 0, 0},
	{40,	20,	20,	100, 0, 0},
	{40,	20,	20,	100, 0, 0},
	{40,	20,	20,	100, 0, 0},
	{40,	20,	20,	100, 0, 0},
	{40,	20,	20,	100, 0, 0},
	{40,	20,	20,	100, 0, 0},
	{40,	20,	20,	100, 0, 0},
	{40,	20,	20,	100, 0, 0}, // 20
	{40,	20,	20,	100, 0, 0}, // 21
	{40,	20,	20,	100, 0, 0},
	{40,	20,	20,	100, 0, 0},
	{40,	20,	20,	100, 0, 0},
	{40,	20,	20,	100, 0, 0},
	{40,	20,	20,	100, 0, 0},
	{40,	20,	20,	100, 0, 0},
	{40,	20,	20,	100, 0, 0},
	{40,	20,	20,	100, 0, 0},
	{40,	20,	20,	100, 0, 0}, // 30
	{40,	20,	20,	100, 0, 0}, // 31
	{40,	20,	20,	100, 0, 0},
	{40,	20,	20,	100, 0, 0},
	{40,	20,	20,	100, 0, 0},
	{40,	20,	20,	100, 0, 0},
	{40,	20,	20,	100, 0, 0},
	{40,	20,	20,	100, 0, 0},
	{40,	20,	20,	100, 0, 0},
	{40,	20,	20,	100, 0, 0},
	{40,	20,	20,	100, 0, 0}, // 40
	{40,	20,	20,	100, 0, 0}, // 41
	{40,	20,	20,	100, 0, 0},
	{40,	20,	20,	100, 0, 0},
	{40,	20,	20,	100, 0, 0},
	{40,	20,	20,	100, 0, 0},
	{40,	20,	20,	100, 0, 0},
	{40,	20,	20,	100, 0, 0},
	{40,	20,	20,	100, 0, 0},
	{40,	20,	20,	100, 0, 0},
	{40,	20,	20,	100, 0, 0}, // 50
};
#endif

#ifdef __BIOLOG_SYSTEM__
const DWORD cBiologSettings[20 + 1][20] =
{
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{55,	81160,	20*5,	100,			0,		30220,	2,	0,		POINT_ATTBONUS_MONSTER,		25,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0},
	{60,	81161,	40*5,	100,			0,		30221,	2,	0,		POINT_ATTBONUS_BOSS,		25,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0},
	{65,	81162,	40*5,	100,			0,		30222,	2,	0,		POINT_ATTBONUS_STONE,		25,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0},
	{70,	81163,	20*5,	100,			0,		30223,	2,	0,		POINT_ATT_GRADE_BONUS,		250,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0},
	{75,	30165,	50*5,	50,			0,	30224,	2,	0,			POINT_ATTBONUS_MONSTER,		50,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0},
	{80,	30166,	50*5,	50,			0,	30225,	3,	0,			POINT_ATTBONUS_BOSS,		50,	0,		0,	0,		0,	0,	0,	0,	0,	0,	0},
	{85,	30167,	50*5,	50,			0,	30226,	4,	0,			POINT_ATTBONUS_STONE,		50,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0},
	{90,	30168,	50*5,	45,			0,	30227,	5,	0,			POINT_ATTBONUS_MONSTER,		50,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0},
	{95,	30251,	20*5,	45,			0,	30228,	5,	0,			POINT_ATTBONUS_BOSS,		75,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0},
	{100,	30252,	20*5,	40,			0,	30228,	5,	0,			POINT_ATTBONUS_STONE,		75,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0},
	{105,	81164,	50*5,	30,			120 * 60,	81175,	5,	0,	POINT_ATTBONUS_MONSTER,		75,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0},
	{110,	81165,	50*5,	30,			120 * 60,	81176,	5,	0,	POINT_ATTBONUS_BOSS,		75,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0},
	{120,	81166,	50*5,	25,			180 * 60,	81177,	5,	0,	POINT_ATTBONUS_STONE,		100,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0},
	{125,	81167,	30*5,	20,			180 * 60,	81178,	5,	0,	POINT_ATTBONUS_MONSTER,		100,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0},
	{130,	81168,	30*5,	20,			180 * 60,	81179,	5,	0,	POINT_ATTBONUS_BOSS,		100,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0},
	{135,	81169,	30*5,	15,			360 * 60,	81180,	10,	0,	POINT_ATTBONUS_STONE,		100,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0},
	{140,	81170,	30*5,	15,			360 * 60,	81181,	10,	0,	POINT_ATTBONUS_CHARACTERS,	10,	POINT_ENCHANT_CHARACTERS,	10,	POINT_MAX_HP,	2100,	POINT_ATT_GRADE_BONUS,	500,	0,	0,	0,	0},
	{145,	81171,	30*5,	15,			360 * 60,	81182,	10,	0,	POINT_ATT_GRADE_BONUS,	500,	POINT_NORMAL_HIT_DAMAGE_BONUS,	30,	POINT_MALL_ATTBONUS,	30,	POINT_ATTBONUS_MONSTER,	125,	POINT_ATTBONUS_STONE,	125,	POINT_ATTBONUS_BOSS,	125},
	{150,	81172,	30*5,	15,			360 * 60,	81183,	10,	0,	POINT_ATT_GRADE_BONUS,	1500,	POINT_NORMAL_HIT_DAMAGE_BONUS,	50,	POINT_MALL_ATTBONUS,	50,	POINT_ATTBONUS_MONSTER,	150,	POINT_ATTBONUS_STONE,	150,	POINT_ATTBONUS_BOSS,	150},
	{150,	81173,	30*5,	15,			720 * 60,	81184,	10,	0,	POINT_ATT_GRADE_BONUS,	2000,	POINT_NORMAL_HIT_DAMAGE_BONUS,	75,	POINT_MALL_ATTBONUS,	75,	POINT_ATTBONUS_MONSTER,	200,	POINT_ATTBONUS_STONE,	200,	POINT_ATTBONUS_BOSS,	200},
};
#endif

#ifdef __SKILL_SET_BONUS__
const DWORD cSkillSetBonus[3+2][3] =
{
	{
	POINT_ATTBONUS_MONSTER,
	POINT_ATTBONUS_STONE,
	POINT_ATTBONUS_BOSS
	},
	{100, 100, 100}, // SKILL_SAGE_MASTER
	{200, 200, 200}, // SKILL_EXPERT_MASTER
	{300, 300, 300}, // destansi
	{400, 400, 400}, // tanrisal
};
#endif // __SKILL_SET_BONUS__

#ifdef __LEVEL_SET_BONUS__
const DWORD cLevelSetBonus[8+1][4] =
{
	{
	POINT_ATTBONUS_MONSTER,
	POINT_ATTBONUS_STONE,
	POINT_ATTBONUS_BOSS,
	POINT_MALL_ATTBONUS,
	},
	{100, 100, 100, 10},
	{200, 200, 200, 20},
	{300, 300, 300, 30},
	{400, 400, 400, 40},
	{500, 500, 500, 50},
	{600, 600, 600, 60},
	{700, 700, 700, 70},
	{800, 800, 800, 80},
};
#endif // __LEVEL_SET_BONUS__

#ifdef __ITEM_SET_BONUS__
const DWORD cItemSetBonus[5+1][4] =
{
	{
	POINT_ATTBONUS_MONSTER,
	POINT_ATTBONUS_STONE,
	POINT_ATTBONUS_BOSS,
	POINT_ATT_GRADE_BONUS,
	},
	{100, 50, 50, 10},
	{200, 100, 100, 20},
	{300, 150, 150, 30},
	{400, 200, 200, 40},
	{500, 250, 250, 50},
};
#endif // __ITEM_SET_BONUS__