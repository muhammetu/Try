#pragma once
#define __TEST__
/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
/********************************************************** [Main Defines] ***********************************************************/
/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
#define __PET_SYSTEM__																														// Like Official Pet System
#define __DICE_SYSTEM__																														// Like Official Dice System
#define __WEAPON_COSTUME_SYSTEM__																											// Like Official Weapon Costume System
#define __MAGIC_REDUCTION_SYSTEM__																											// Like Official Magic Reduction System
#define __ACCE_SYSTEM__																														// Like Official Shoulder Sash System
#define __HIGHLIGHT_ITEM__																													// Like Official Highlight Item System
#define __CHANNEL_CHANGER__																													// Like Official Channel Changer System
#define __QUIVER_SYSTEM__																													// Like Official Quiver System
#define __ITEM_GACHA__																														// Like Official Gacha System
#define __CHARACTER_WINDOW_RENEWAL__																										// Like Official Character Select/Create Window System
#define __PENDANT_SYSTEM__																													// Like Official Pendant Armor System
#define __DS_GRADE_MYTH__																													// Like Official DS Grade Myth System (Update)
#define __GROWTH_PET_SYSTEM__																												// Like Official Growth Pet System
#define __ITEM_CHANGELOOK__																													// Like Official Item Changelook System
#define __7AND8TH_SKILLS__																													// Like Official 7&8th Skills System
#define __MOB_AGGR_LVL_INFO__																												// Like Official Mob Aggr/Level Information System
#define __WOLFMAN_CHARACTER__																												// Like Official Wolfman Character
#define __GEM_SYSTEM__																														// Like Official Gem System

#define __CUBE_WINDOW__																														// Like Official Cube Window System
#define __MOUNT_COSTUME_SYSTEM__																											// Like Official Costume Mount System
#define __AURA_COSTUME_SYSTEM__																												// Like Official Costume Aura System
#define __ITEM_EVOLUTION__																													// Like Official Item Evolution (Element) System
#define __QUEST_CATEGORY__																													// Like Official Quest Category System
#define __EVENT_SYSTEM__																													// Like Official Event Banner & System
#define __SLOT_MARKING__																													// Like Official Slot Marking System
#define __COSTUME_ATTR_SWITCH__																												// Like Official Costume Attr Switch System
#define __CHEQUE_SYSTEM__																													// Like Official Cheque System
#define __DS_GRADE_EPIC__																													// DS Grade epical
#define __DS_GRADE_GODLIKE__																												// DS Grade Godlike

#ifdef __CHEQUE_SYSTEM__
	#define __FAST_CHEQUE_TRANSFER__																										// Like Official Fast Cheque Transfer System
#endif

#ifdef __ITEM_CHANGELOOK__
	#define __MOUNT_CHANGELOOK__																											// Like Official Mount Changelook System
#endif

#ifdef __DS_GRADE_MYTH__
	#define __DS_SET_BONUS__																												// Like Official DS Set Bonus System
#endif

#ifdef __ITEM_EVOLUTION__
	#define __ARMOR_EVOLUTION__																												// Official Item Evolution Armor Extension
#endif

#ifdef __MAGIC_REDUCTION_SYSTEM__
	#define USE_MAGIC_REDUCTION_STONES																										// Enable magic reduction stones
#endif

#ifdef __ACCE_SYSTEM__
	#define __ACCE_25ABS_ITEMS__																											// 30 abs acce items
#endif

#ifdef __WOLFMAN_CHARACTER__
	#define USE_MOB_BLEEDING_AS_POISON																										// Mob table bleeding as poison
	#define USE_MOB_CLAW_AS_DAGGER																											// Mob table claw as dagger
	#define USE_ITEM_CLAW_AS_DAGGER																											// Item table claw as dagger
	#define USE_WOLFMAN_STONES																												// enable lycan stones
	#define USE_WOLFMAN_BOOKS																												// enable lycan books
	#define USE_ITEM_BLEEDING_AS_POISON																										// Item table bleeding as poison
	// #define USE_LYCAN_CREATE_POSITION																										// New start position for lycan characters
#endif

#ifdef __PET_SYSTEM__
	#define __PET_SYSTEM_PROTO__																											// Pet system works on item_proto.txt
#endif

#ifdef __CUBE_WINDOW__
	#define __CUBE_ATTR_SOCKET_EXTENSION__																									// Cube attr socket extensions
#endif

#define __ANCIENT_ATTR_ITEM__																												// Ancient attribute item like Metin2.TC
#define __CHEST_INFO_SYSTEM__																												// Chest Content Information on Right click
#define __ADDITIONAL_INVENTORY__																											// Additional inventory on press K Button
#define __INGAME_MALL__																														// Ingame Mall system from taskbar
#define __SWITCHBOT__																														// Switchbot system on core
#define __DROP_DIALOG__																														// Drop dialog sell/delete system
#define __SKILL_COLOR__																														// Skill color system
#define __SKILL_CHOOSE_WINDOW__																												// Skill choose window when levelup to five
#define __PIN_SECURITY__																													// Pin Security System
#define __SKILLBOOK_SYSTEM__																												// Skill book gui system
#define __SOULSTONE_SYSTEM__																												// Soul Stone gui system
#define __SPLIT_ITEMS__																														// Split item system
#define __CHAT_FILTER__																														// Chat Filter System
#define __BEGINNER_ITEM__																													// Beginner(untradable) item system
#define __DUNGEON_INFORMATION__																												// Dungeon information system
// #define __COSTUME_EXTEND_TIME__																												// Costume Extend Time
#define __SUB_SKILL_REWORK__																												// Sub Skill Rework
#define __TARGET_BOARD_RENEWAL__																											// Target Board Renewal
#define __REGEN_REWORK__																													// Regen.txt mobs rework

#define __OFFLINE_SHOP__																													// Offline Shop System
#ifdef __OFFLINE_SHOP__
	#define __OFFLINE_SHOP_ENTITY_CITIES__																									// Offline Shop Cities Entity
	#define USE_OFFLINE_SHOP_CONFIG
	//#define __OFFLINESHOP_DEBUG__																										// Offline Shop Debug

	#ifdef __OFFLINESHOP_DEBUG__
		#ifdef __WIN32__
			#define OFFSHOP_DEBUG(fmt , ...) sys_log(0,"%s:%d >> " fmt , __FUNCTION__ , __LINE__, __VA_ARGS__)
		#else
			#define OFFSHOP_DEBUG(fmt , args...) sys_log(0,"%s:%d >> " fmt , __FUNCTION__ , __LINE__, ##args)
		#endif
	#else
		#define OFFSHOP_DEBUG(...)
	#endif
#endif

#define __SHINING_ITEM_SYSTEM__																											// Shining Item System
#define __INVENTORY_SORT__																												// inventory sort
#define __CAKRA_ITEM_SYSTEM__
#define __GROWTH_MOUNT_SYSTEM__
#define ENABLE_NEW_DRAGON_SOUL_SINIF
#define __NEWATTRPHEBIA__
#define ENABLE_TELEPORT_SYSTEM
#define __DROP_CH_RENEWAL__
#define __SEBNEM_ITEM_SYSTEM__
//#define __OLUCU_DROBU__																													// Ekmek Kirintisi Drobu
#define ENABLE_DISCORD_UTILITY																											// Enables Discord Support (QOL)
#if defined(ENABLE_DISCORD_UTILITY)
	#define USE_DISCORD_NETWORK																												// Enables Discord Sync Server
#endif // defined(ENABLE_DISCORD_UTILITY)

#define ENABLE_PLAYER_STATISTICS
#define ENABLE_REMOTE_SHOP_SYSTEM
/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
/********************************************************** [Guild Defines] ***********************************************************/
/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
#define __GUILD_LEADER_SYSTEM__																												// Guild Leader System
#define __GUILD_WAR_REWORK__																												// Guild War Rework
// #define __GUILD_LEADER_DIRECT_GOTO_WAR__																									// If you are leader of guild you will direct enter the war.
/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
/********************************************************** [Extensions Defines] ***********************************************************/
/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
#define __REFINE_REWORK__																													// Refine skill & success prob extensions
#define __REFINE_MSG_REWORK__																												// Like Official Refine Message Rework

#ifndef __CUBE_WINDOW__
	#define __CUBE_PERCENT_EXTENSION__																										// Cube percent extension
	#define __CUBE_RELOAD_REWORK__																											// Cube reload extension
#endif

#ifdef __PENDANT_SYSTEM__
	#define __PENDANT_ATTR__																												// Pendant attr extension
#endif

#define __SKILL_PARTY_FLAG__																												// PARTY flag for skills
#define __REMOVE_SKILL_AFFECT__																												// Remove skill affect extension
#define __ITEM_EXTRA_SOCKET__																												// Extra item socket extensions
#define __SHOP_PRICE_TYPE_ITEM__																											// Shop price type yang -> item extension
#define __DAMAGE_PROCESS_REWORK__																											// Damage Process Rework
#define __PARTY_ROLE_REWORK__																												// New party role extensions
#define __FISH_BONUS_REWORK__																												// Fish Bonus Rework
#define __GOLD_REWARD_REWORK__																												// Gold reward rework
#define __MONSTER_DEATHBLOW_REWORK__																										// Monster Deathblow Rework
#define __SAFEBOX_AUTO_SORT__																												// Safebox auto sort extension
#define __FULL_NOTICE__																														// Full Notice extension
#define __PLAYER_PER_ACCOUNT_5__																											// Player Per Account Five Extension
#define __BEGINNING_REWORK__																												// Start Chars Rework
#define __SHOP_PRICE_SQL__																													// Shop Price SQL
#define __STATUS_UP_REWORK__																												// Status Up Rework
#define __NOT_BUFF_CLEAR__																													// Not Buff Clear
#define __DO_STATE_REWORK__																													// Do State Rework
#define __CLEAR_SKILL_WHEN_UNJOB__																											// Clear skill when unjob
#define __CHECK_ITEMS_ON_TELEPORT__																											// Check items on teleport
#define __ITEM_ATTR_COSTUME__																												// Item Attr Costume
#define __POTION_AS_REWORK__																												// Potion affect shower extension
#define __ATTRIBUTES_TYPES__																												// Attributes Type extension
#define __MULTIPLE_MONSTER_ATTR__																											// Multiple Monster ATTR
/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
/********************************************************** [Utility Defines] ***********************************************************/
/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
#define __GLOBAL_MESSAGE_UTILITY__																											// Global Message Utility
#define __BAN_REASON_UTILITY__																												// Ban Reason Utility
#define __HWID_SECURITY_UTILITY__																											// Hwid security Utility
#define __CLIENT_VERSION_UTILITY__																											// Client version Utility
#define __ITEM_ENTITY_UTILITY__																												// Item Entity Utility
#define __GOLD_LIMIT_REWORK__																												// Gold Limit to (long long) variable
#define __ITEM_COUNT_LIMIT__																												// Item count limit utility
#define __PORT_SECURITY_UTILITY__																											// Port security utility
#define __PASSWORD_HASH_UTILITY__																											// Password Hash Utility
#define __RELOAD_REWORK__																													// Reload rework Utility
#define __DEFAULT_PRIV__																													// Default Priv Utility
#define __IMPROVED_PACKET_ENCRYPTION__																										// Improved Packet Encryption
#define __LARGE_DYNAMIC_PACKET__																											// Large dynamic packet Utility
#define __GAME_MASTER_UTILITY__																												// Game Master Utility
#define __DAMAGE_LIMIT_REWORK__																												// Damage limit to long long variable
#ifdef __RELOAD_REWORK__
#define ENABLE_RELOAD_SHOP_COMMAND
#define ENABLE_RELOAD_REFINE_COMMAND
#endif
//#define __DAMAGE_MULTIPLER__
/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
/********************************************************** [Item Defines] ***********************************************************/
/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
#define __BRAVERY_CAPE_REWORK__																												// Auto bravery cape item
#define __BLEND_ITEM_REWORK__																												// Perma blend item
#define __REFINE_SCROLL__																													// Refine scroll item
#define __PERMA_ACCESSORY__																													// Perma accessory item
#define __ICECREAM_ITEM__																													// Icecream item
#define __NOG_PACKETS__																														// Nog packets item
#define __RAMADAN_CANDY_ITEMS__																												// Ramadan Candy item
#define __STONE_DETECT_REWORK__																												// Stone detect item
#define __DSS_RECHARGE_ITEM__																												// DSS Recharge item
#define __BOSS_SCROLL__																														// Boss Scroll item
#define __EXP_RING_ITEMS__																													// Exp ring items
// #define __STRENGHT_FEED__																													// Strenght item
#define __DSS_REFINE_ITEMS__																												// Refine items for dss
#define __DSS_PUT_ATTR_ITEM__																												// Put attr for dss
#define __ALIGNMENT_ITEMS__																													// Alignment Items
#define __ATTR_ADDON_ITEMS__																												// Attr Switch Items (Only Addon)
#define __BLEND_R_ITEMS__																													// Blend Items
#define __HAIR_COSTUME_ATTRIBUTE__																											// Hair costume attribute item
#define __BODY_COSTUME_ATTRIBUTE__																											// Body costume attribute item

#if defined(__HAIR_COSTUME_ATTRIBUTE__) || defined(__BODY_COSTUME_ATTRIBUTE__) || defined(__MOUNT_COSTUME_SYSTEM__) || defined(__AURA_COSTUME_SYSTEM__) || defined(__WEAPON_COSTUME_SYSTEM__)
	#define __NEW_ATTRIBUTE_ITEMS
#endif
/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
/********************************************************** [Event Defines] ***********************************************************/
/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
#ifdef __EVENT_SYSTEM__
	#define __AUTO_EVENT_SYSTEM__																											// Auto event module for event system
#endif
/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
/********************************************************** [Effect Defines] ***********************************************************/
/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
#define __AGGR_MONSTER_EFFECT__																												// Like Official Aggreate Monster Effect
#define __PVP_EFFECT__																														// Duel (PvP) Effect
#define __SMITH_EFFECT__																													// Refine Fail/Success Effect
#define __PENETRATE_EFFECT__																												// Penetrate Effect
#define __EXTRAPOT_EFFECT__																													// Extrapot Effect
#define __DRAGON_BONE_EFFECT__																												// Dragonbone Effect Like Metin2.TC
#define __UNIQUE_ITEM_EFFECT__																												// Unique Item Effect
#define __DSS_ACTIVE_EFFECT__																												// DSS Active Effect
/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
/********************************************************** [Scp Defines] ***********************************************************/
/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
#define __RANK_SYSTEM__																														// Rank system
#define __LANDRANK_SYSTEM__																													// Land Rank system
#define __REBORN_SYSTEM__																													// Reborn System
#define __BIOLOG_SYSTEM__																													// Biolog System
#define __SAGE_SKILL__																														// Sage Skill System
#define __EXPERT_SKILL__																													// Expert Skill System
#define __ALIGNMENT_REWORK__																												// Alignment System
#define __DUNGEON_QUEUE__																													// Dungeon queue system
#define __SCP1453_EXTENSIONS__																												// Scp1453 Extensions
#define __AS_BOT__																															// a.s
#define __CHEST_DROP_POINT__																												// TODO
#define __ENABLE_KILL_EVENT_FIX__																											// TODO
#define __DIS_DAMAGE_TYPE_NORMAL__																											// Damage type normal is disabled
#define __WARP_WITH_CHANNEL__																												// /warp <name> with channel
#ifndef __WIN32__
#define __ENABLE_WIKI_SYSTEM__																												// Wiki system
#endif
#define __NEW_ACCESORY_ITEMS__																												// kask ve kalkana cevher sistemi
#define __ACC_STATUS_CHECHKUP__																												// hesap durumu incelemede
#define __DUNGEON_TEST_MODE__																												// zindan test eklentisi
#define __PRINT_BUILD_INFO__																												// start verildiginde build bilgilerini yazdir
#define __CMD_EXTENSIONS__																													// yeni gm kodlari
//#define __AUTH_LOGIN_TIMEOUT__
#define __NOT_ENOUGH_DB_QUERY_SPEED_ERROR__
#define __USE_ITEM_COUNT__
#define __DUNGEON_LIMIT__
#ifdef __USE_ITEM_COUNT__
#define __RENEWAL_CHEST_USE__
#endif // __USE_ITEM_COUNT__
#define __SKILL_SET_BONUS__
#define __LEVEL_SET_BONUS__
#define __ITEM_SET_BONUS__
/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
/********************************************************** [Marty Defines] ***********************************************************/
/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
#define __MARTY_BELT_INVENTORY__																											// Marty Belt Inventory Extensions
#define __MARTY_NULLIFYAFFECT_LIMIT__																										// Marty Sura Spell RemoveGoodAffect Limit
#define __MARTY_GM_FLAG_IF_TEST_SERVER__																									// Marty GM Affect If Test Server
#define __MARTY_NEWEXP_CALCULATION__																										// Marty New Exp Reward Algorhitm
#define __MARTY_AUTODETECT_VNUM_RANGE__																										// Marty AutoDetect Vnum Range
#define __MARTY_EXPTABLE_FROMDB__																											// Marty exp table from db
#define __MARTY_IMMUNE_FIX__																												// Marty Immune Fix
#define __MARTY_PARTYKILL__																													// Marty Party Kill
//#define __MARTY_BLOCK_CMD_SHORTCUT__																										// Marty Block CMD Shortcut
#define __MARTY_FISHINGROD_RENEWAL__																										// Marty Fishingrod Renewal
#define __MARTY_PICKAXE_RENEWAL__																											// Marty Pickaxe Renewal
#define __MARTY_SET_STATE_WITH_TARGET__																										// Marty Set State With Target
#define __MARTY_FORCE2MASTER_SKILL__																										// Marty Force2Master Skill
#define __MARTY_CMD_IPURGE_EX__																												// Marty CMD IPurge Ex
#define __MARTY_ADDSTONE_FAILURE__																											// Marty Addstone Failure
#define __MARTY_CHAT_LOGGING__																												// Marty Chat Logging
#define __MARTY_WHISPER_CHAT_SPAMLIMIT__
#define __MARTY_ANTI_CMD_FLOOD__
/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

const int MAP_ALLOW_LIMIT	= 64; // (default 32) map allow limit on single core
const int SHOUT_TIME_LIMIT	= 5; // shout limit time
const int EMPIRE_PRIV_MAX	= 1000; // empire priv
const int EXCHANGE_GRID_WIDTH	= 5;
const int EXCHANGE_GRID_HEIGHT	= 9;
const int SHOP_GRID_WIDTH	= 5;
const int SHOP_GRID_HEIGHT	= 9;
const int GOLD_MULTIPLER	= 1;
const int EXP_MULTIPLER	= 1;
const int STARTUP_LEVEL	= 55;
#ifdef __CHANNEL_CHANGER__
const int CHANNEL_MAX_COUNT	= 8; // channel changer channel extension
#endif

#ifdef __ALIGNMENT_REWORK__
const int ENABLE_RENEWAL_ALIGN_UPDATE_2	= 75; // Alignment Rework Variable 1
const int ALIGN_GRADE_COUNT	= (19+12); // Max align grade count
const int ALIGN_BONUS_COUNT	= 7; // Max align bonus count
#endif

#ifdef __RANK_SYSTEM__
const int RANK_GRADE_COUNT	= 23; // Max rank grade count
const int RANK_BONUS_COUNT	= 5; // Max rank bonus count
#endif

#ifdef __LANDRANK_SYSTEM__
const int LANDRANK_GRADE_COUNT	= 4; // Landrank max grade count
const int LANDRANK_BONUS_COUNT	= 7; // Landrank max bonus count
#endif

#ifdef __REBORN_SYSTEM__
const int REBORN_GRADE_COUNT	= 100; // Reborn max grade count
const int REBORN_BONUS_COUNT	= 6; // Reborn max bonus count
#endif

#ifdef __SAGE_SKILL__
const int SKILL_OVER_P_ITEM_VNUM	= 99410; // sage skill required item vnum
const int SKILL_OVER_P_KOTU_RUH_VNUM	= 99412; // sage skill required item vnum (2)
const int SKILL_OVER_P_MUNZEVI_VNUM	= 99411; // sage skill required item vnum (3)
#endif

#ifdef __EXPERT_SKILL__
const int SKILL_EXPERT_ITEM_VNUM	= 99430; // sage skill required item vnum
const int SKILL_EXPERT_KOTU_RUH_VNUM	= 99432; // sage skill required item vnum (2)
const int SKILL_EXPERT_MUNZEVI_VNUM	= 99431; // sage skill required item vnum (3)

const int SKILL_DESTANSI_ITEM_VNUM	= 99050;
const int SKILL_DESTANSI_KOTU_RUH_VNUM	= 99051;
const int SKILL_DESTANSI_MUNZEVI_VNUM	= 99052;

const int SKILL_TANRISAL_ITEM_VNUM	= 99054;
const int SKILL_TANRISAL_KOTU_RUH_VNUM	= 99055;
const int SKILL_TANRISAL_MUNZEVI_VNUM	= 99056;
#endif

#ifdef __BIOLOG_SYSTEM__
const int BIOLOG_SURE_SIFIRLA_ITEM	= 31029; // biolog time reset item vnum
const int BIOLOG_100_PERC_ITEM	= 31030; // biolog max prob item vnum
const int BIOLOG_DOUBLE_ITEM	= 0; // biolog unknown vnum
#endif

#ifdef __SCP1453_EXTENSIONS__
const int SKILL_START_LEVEL	= 40; // skill start level when levelup five
#endif

#ifdef __ATTR_ADDON_ITEMS__
const int SWITCH_OBJECT_NORMAL	= 99421;
const int SWITCH_OBJECT_SKILL	= 99422;
const int SWITCH_OBJECT_LAST_ATTR	= 99427;
const int SWITCH_OBJECT_NORMAL2 = 71086;
const int SWITCH_OBJECT_NORMAL3 = 71087;
const int SWITCH_OBJECT_NORMAL4 = 56500;
const int SWITCH_OBJECT_NORMAL5 = 56501;
const int SWITCH_OBJECT_NORMAL6 = 56502;
const int SWITCH_OBJECT_NORMAL7 = 56503;
const int SWITCH_OBJECT_NORMAL8 = 56504;
#endif

#ifdef __ITEM_EVOLUTION__
const int EV_CLEAN_VALUE0	= 20;
const int EV_CLEAN_VALUE_GOLD	= 10000000;
#endif

#ifdef __TARGET_BOARD_RENEWAL__
const int MOB_INFO_ITEM_LIST_MAX	= 200;
#endif
#ifdef __DAMAGE_MULTIPLER__
const int DAMAGE_MULTIPLER_1	= 1;
#endif

#ifdef __OFFLINE_SHOP__
constexpr int g_dwShopSearchLimitCounter = 1;
#endif

#ifdef ENABLE_REMOTE_SHOP_SYSTEM
const int REMOTE_SHOP_VID = 2147483647;
#endif
/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
/*
@edit014	: block use skill books when non open skill group
@edit017	: att bonus algorhitm changed
*/
#define ENABLE_EMOJI_UTILITY
#define __BATTLE_PASS_SYSTEM__ // Battlepass-System
#define ENABLE_BATTLE_PASS_EX
#define ENABLE_AUTO_HUNT_SYSTEM																												// Like GF Auto Hunting System
#define ENABLE_PB2_PREMIUM_SYSTEM
#define ENABLE_CHAT_COLOR_SYSTEM
#define ENABLE_PB2_NEW_TAG_SYSTEM
#define ENABLE_NAME_CHANGE_SYSTEM
#define ENABLE_FULL_SYSTEM																												// Enables channel full system
#define __MONIKER_SYSTEM__
#define __TEAM_SYSTEM__
#define ENABLE_LOVE_SYSTEM
#define ENABLE_WORD_SYSTEM
#define ENABLE_MULTI_FARM_BLOCK
