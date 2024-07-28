#pragma once
/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
/********************************************************** [Main Defines] ***********************************************************/
/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
#define ENABLE_DRAGON_SOUL_SYSTEM																												// Like Official Dragon Soul System
#define ENABLE_ENERGY_SYSTEM																													// Like Official Energy System
#define ENABLE_NEW_EQUIPMENT_SYSTEM																												// Like Official New Equipment (Belt) System
#define ENABLE_DICE_SYSTEM																														// Like Official Dice System
#define ENABLE_COSTUME_SYSTEM																													// Like Official Costume System
#define ENABLE_MAGIC_REDUCTION_SYSTEM																											// Like Official Magic Reduction System
#define ENABLE_ACCE_SYSTEM																														// Like Official Acce System
#define ENABLE_HIGHLIGHT_ITEM																													// Like Official Highlight Item System
#define ENABLE_CHANNEL_CHANGER_SYSTEM																											// Like Official Channel Changer System
#define ENABLE_QUIVER_SYSTEM																													// Like Official Quiver System
#define ENABLE_ITEM_GACHA_SYSTEM																												// Like Official Gacha System
#define ENABLE_CHARACTER_WINDOW_RENEWAL																											// Like Official Character Select/Create Window System
#define ENABLE_PENDANT_SYSTEM																													// Like Official Pendant Armor System
#define ENABLE_GROWTH_PET_SYSTEM																												// Like Official Growth Pet System
#define ENABLE_GROWTH_MOUNT_SYSTEM
#define ENABLE_CHANGELOOK_SYSTEM																												// Like Official Item Changelook System
#define ENABLE_7AND8TH_SKILLS																													// Like Official 7&8th Skills System
#define ENABLE_MOB_AGGR_LVL_INFO																												// Like Official Mob Aggr/Level Information System
#define ENABLE_WOLFMAN_CHARACTER																												// Like Official Wolfman Character
#define ENABLE_GEM_SYSTEM																														// Like Official Gem System
#define ENABLE_CUBE_RENEWAL																														// Like Official Cube Window System
#define ENABLE_ITEM_EVOLUTION_SYSTEM																											// Like Official Item Evolution System
#define ENABLE_QUEST_CATEGORY_SYSTEM																											// Like Official Quest Category System
#define ENABLE_EVENT_SYSTEM																														// Like Official Event Banner & System
#define ENABLE_SLOT_MARKING_SYSTEM																												// Like Official Slot Marking System
#define ENABLE_COSTUME_ATTR_SWITCH																												// Like Official Costume Attr Switch System
#define ENABLE_CHEQUE_SYSTEM																													// Like Official Cheque System
#define ENABLE_FOG_FIX																															// Like Official Fog Fix
#define ENABLE_SHOW_NIGHT_SYSTEM																												// Like Official Day/Night system
#define ENABLE_SNOWFALL_MODE																													// Like Official SnowFall system
#define ENABLE_DETAILS_INTERFACE																												// Like Official Details Inferface system

#ifdef ENABLE_CHEQUE_SYSTEM
#define ENABLE_FAST_CHEQUE_TRANSFER																											// Like Official Fast Cheque Transfer System
#endif

#ifdef ENABLE_COSTUME_SYSTEM
#define ENABLE_WEAPON_COSTUME_SYSTEM																										// Like Official Weapon Costume System
#define ENABLE_MOUNT_COSTUME_SYSTEM																											// Like Official Costume Mount System
#define ENABLE_AURA_COSTUME_SYSTEM																											// Like Official Costume Aura System
#endif

#ifdef ENABLE_CHANGELOOK_SYSTEM
#define ENABLE_MOUNT_CHANGELOOK_SYSTEM																										// Like Official Mount Changelook System
#endif

#ifdef ENABLE_DRAGON_SOUL_SYSTEM
#define ENABLE_DS_GRADE_MYTH																												// Like Official DS Grade Myth System (Update)
#define ENABLE_DS_GRADE_EPIC																												// DS Grade Epic System
#define ENABLE_DS_GRADE_GODLIKE																												// DS Grade Godlike System
#ifdef ENABLE_DS_GRADE_MYTH
#define ENABLE_DS_SET																													// Like Official DS Set Bonus System
#endif
#endif

#ifdef ENABLE_ITEM_EVOLUTION_SYSTEM
#define ENABLE_ARMOR_EVOLUTION_SYSTEM																										// Official Item Evolution Armor Extension
#endif

#define ENABLE_ANCIENT_ATTR_SYSTEM																												// Ancient attribute item like Metin2.TC
#define ENABLE_CHEST_INFO_SYSTEM																												// Chest Content Information on Right click
#define ENABLE_ADDITIONAL_INVENTORY																												// Additional inventory on press K Button
#define ENABLE_INGAME_MALL_SYSTEM																												// Ingame Mall system from taskbar
#define ENABLE_SWITCHBOT_SYSTEM																													// Switchbot system on core
#define ENABLE_DROP_DIALOG_SYSTEM																												// Drop dialog sell/delete system
#define ENABLE_SKILL_COLOR_SYSTEM																												// Skill color system
#define ENABLE_LOTTERY_SYSTEM																													// Lottery system
#define ENABLE_SKILL_CHOOSE_SYSTEM																												// Skill choose window when levelup to five
#define ENABLE_PIN_SYSTEM																														// Pin Security System
#define ENABLE_SKILLBOOK_SYSTEM																													// Skill book gui system
#define ENABLE_SOULSTONE_SYSTEM																													// Soul Stone gui system
#define ENABLE_SPLIT_ITEMS_SYSTEM																												// Split item system
#define ENABLE_CHAT_FILTER																														// Chat Filter System
#define ENABLE_BEGINNER_ITEM_SYSTEM																												// Beginner(untradable) item system
#define ENABLE_DUNGEON_INFO_SYSTEM																												// Dungeon information system
// #define ENABLE_COSTUME_EXTEND_TIME_SYSTEM																										// Costume Extend Time
#define ENABLE_SKYBOX_OPTION_SYSTEM																												// Skybox Option System
#define ENABLE_HOUR_HISTORY_SYSTEM																												// Hour history system
#define ENABLE_UNBUG_CHARACTER_SYSTEM																											// Unbug character system
#define ENABLE_ITEM_DELETE_SYSTEM																												// Multiple item delete system
#define ENABLE_FONT_MODE_SYSTEM																													// Font mode system
#define ENABLE_CHAT_MODE_SYSTEM																													// Chat mode system
#define ENABLE_TEXTURE_MODE																														// Texture mode system
#define ENABLE_WEB_LINK_SYSTEM																													// Web link system
#define ENABLE_SKILL_BOOK_SYSTEM																												// Skill book system
#define ENABLE_INVENTORY_ADDITION																												// Inventory addition system
#define ENABLE_LOGIN_DLG_RENEWAL																												// Login Dlf Renewal System
#define ENABLE_TARGET_BOARD_RENEWAL																												// Target Board Renewal
#define ENABLE_ATLAS_RENDER_OPTION																												// Atlas render option

#define ENABLE_OFFLINE_SHOP																														// Offline Shop System
#ifdef ENABLE_OFFLINE_SHOP
#define ENABLE_OFFLINE_SHOP_CITIES																											// Offline Shop Cities Entity

/*#ifndef LIVE_SERVER
#define ENABLE_OFFLINESHOP_DEBUG																											// Offline Shop Debug
#endif*/

#if defined(ENABLE_OFFLINESHOP_DEBUG) && defined(_DEBUG)
#define OFFSHOP_DEBUG(fmt , ...) Tracenf("%s:%d >> " fmt , __FUNCTION__ , __LINE__, __VA_ARGS__)
#else
#define OFFSHOP_DEBUG(...)
#endif
#endif

#define ENABLE_SHINING_ITEM_SYSTEM																												// Shining Item System

#define ENABLE_SORT_INVENTORY_ITEM																												// sort inventory

#ifdef ENABLE_TARGET_BOARD_RENEWAL
#define MOB_INFO_ITEM_LIST_MAX	200
#endif
#define ENABLE_CAKRA_ITEM_SYSTEM
#define ENABLE_NEW_DRAGON_SOUL_SINIF
#define ENABLE_TELEPORT_SYSTEM
#define ENABLE_NEW_ATTR_PHEBIA
#define ENABLE_SPAM_BOT_SYSTEM
#define ENABLE_DISCORD_UTILITY																											// Enables Discord Support (QOL)
#if defined(ENABLE_DISCORD_UTILITY)
	#define USE_DISCORD_RPC_MODULE																											// Enables Discord Activity
	#define USE_DISCORD_LOBBY_MODULE																										// Enables Discord Lobby Module
	#define USE_DISCORD_NETWORK																												// Enables Discord Sync Server
	#define USE_DISCORD_PRESENCE_NON_SDK
#endif // defined(ENABLE_DISCORD_UTILITY)
#define ENABLE_GAME_OPTION_DLG_RENEWAL
#define ENABLE_CHAT_FILTER_OPTION																										// Enable Chat Filter Option (System)
#if defined(ENABLE_CHAT_FILTER_OPTION)
enum EChatFilter
{
	CHAT_FILTER_PARTY,
	CHAT_FILTER_GUILD,
	CHAT_FILTER_SHOUT,
	CHAT_FILTER_MAX_NUM,
};
#endif // defined(ENABLE_CHAT_FILTER_OPTION)
#define ENABLE_ENVIRONMENT_EFFECT_OPTION																								// Environment Effect Option (System)
#define ENABLE_PICKUP_OPTION
#define USE_LOADING_DLG_OPTIMIZATION																									// Enables Loading Dlg Thread.
#define ENABLE_HIDE_BODY_PARTS
#define ENABLE_PLAYER_STATISTICS
#define ENABLE_REMOTE_SHOP_SYSTEM
#define ENABLE_SEBNEM_ITEM_SYSTEM
/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
/********************************************************** [Guild Defines] ***********************************************************/
/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
#define ENABLE_GUILD_LEADER_SYSTEM																												// Guild Leader System
#define ENABLE_GUILD_WAR_REWORK																													// Guild War Rework
/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
/********************************************************** [Extensions Defines] ***********************************************************/
/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
#define ENABLE_REFINE_REWORK																													// Refine skill & success prob extensions
#define ENABLE_REFINE_MSG_REWORK																												// Like Official Refine Message Rework
#define ENABLE_FAST_SWAP_ITEM																													// Like Official Fast Swap Item extension
#define ENABLE_CHANNEL_INFO_UPDATE																												// Like Official Channel Info Update
#define ENABLE_MILES_REWORK																														// Like Official Editable Miles Volume Information

#define ENABLE_REMOVE_SKILL_AFFECT																												// Remove skill affect extension
#define ENABLE_EXTRA_SOCKET_SYSTEM																												// Extra item socket extensions
#define ENABLE_SHOP_PRICE_TYPE_ITEM																												// Shop price type yang -> item extension
#define ENABLE_DAMAGE_PROCESS_SYSTEM																											// Damage Process Rework
#define ENABLE_PARTY_ROLE_REWORK																												// New party role extensions
#define ENABLE_PLAYER_PER_ACCOUNT5																												// Player Per Account Five Extension
#define ENABLE_STATUS_UP_REWORK																													// Status Up Rework
#define ENABLE_POTION_AS_REWORK																													// Potion affect shower extension
#define ENABLE_BLEND_REWORK																														// Blend rework extension
#define ENABLE_DAMAGE_BAR																														// Enable damage bar extension on target board
//#define ENABLE_TOOLTIP_DROP_INFORMATION																											// Enable tooltip drop information extension
#define ENABLE_SIGN_PLUS_ITEMS																													// Sign plus item extension
#define ENABLE_EXCHANGE_REWORK																													// Exchange rework
#define ENABLE_GOLD_INFO_TEXT																													// Gold Info text
#define ENABLE_EXP_INFO_TEXT																													// Exp info text
#define ENABLE_UNLIMITED_SKILL_SYSTEM																											// Unlimited Skill extension
#define ENABLE_MOUSEWHEEL_EVENT																													// Mouse wheel event for python gui
#define ENABLE_PICKUP_ITEM_REWORK																												// Pickup item time/multiple rework
#define ENABLE_CANSEEHIDDENTHING_FOR_GM																											// Can see hidden things for game masters
#define ENABLE_CHAT_SHOUT_REWORK																												// Chat shout rework
#define ENABLE_ATTR_ADDONS																														// New attribute types
#define ENABLE_AS_REWORK																														// New affects for affect shower
#define ENABLE_MAP_ALGORITHM_RENEWAL																											// Map original name on atlas window
#define ENABLE_PICKUP_ITEM_SOUND																												// Pickup item sound on when pick
//#define ENABLE_RENDER_EFFECT_MINIMIZE_FIX																										// Render Effect Minimize

#ifdef ENABLE_PENDANT_SYSTEM
#define ENABLE_PENDANT_ATTRIBUTE_SYSTEM																										// Pendant attribute system
#endif

#ifndef ENABLE_CUBE_RENEWAL
#define ENABLE_CUBE_PERCENT_RENEWAL																											// Cube Percent extension
#define ENABLE_CUBE_RELOAD_FIX																												// Cube reload extension
#endif

#ifdef ENABLE_MAP_ALGORITHM_RENEWAL
	#define ENABLE_ATLAS_POSITION_EX
	#ifdef ENABLE_ATLAS_POSITION_EX
		#define ENABLE_METINSTONE_ON_MINIMAP																										// Metin stone on minimap
		#define ENABLE_BOSS_ON_MINIMAP																												// Boss on minimap
	#endif
#endif
/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
/********************************************************** [Utility Defines] ***********************************************************/
/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
#define ENABLE_GLOBAL_MESSAGE_UTILITY																											// Global Message Utility
#define ENABLE_BAN_REASON_UTILITY																												// Ban Reason Utility
#define ENABLE_HWID_SECURITY_UTILITY																											// Hwid security Utility
#define ENABLE_CLIENT_VERSION_UTILITY																											// Client version Utility
#define ENABLE_ITEM_ENTITY_UTILITY																												// Item Entity Utility
#define ENABLE_GOLD_LIMIT_REWORK																												// Gold Limit to (long long) variable
#define ENABLE_ITEM_COUNT_LIMIT																													// Item count limit utility
#define ENABLE_IMPROVED_PACKET_ENCRYPTION																										// Improved Packet Encryption
#define ENABLE_LARGE_DYNAMIC_PACKET																												// Large dynamic packet Utility
#define ENABLE_SHINING_EFFECT_UTILITY																											// Shining Effect Utility
#define ENABLE_EMOJI_UTILITY																													// Emoji Utility
#define ENABLE_GAME_MASTER_UTILITY																												// Game Master Utility
#if defined(LIVE_SERVER) && !defined(LIVE_SERVER_TEST)
//#define ENABLE_SPLASH
#define ENABLE_CYTHON_MAIN																													// Cython Main
#define ENABLE_CYTHON_EXTRA																													// Cython Extra
//#define ENABLE_CRASH_MINIDUMP																												// When crash minidump on logs folder
#else
#define ENABLE_CRASH_MINIDUMP																												// When crash minidump on logs folder
#endif

#define ENABLE_PACK_TYPE_DIO																													// Pack Type Dio Utility
#ifdef ENABLE_PACK_TYPE_DIO
#define USE_PACK_TYPE_DIO_ONLY																												// Use Pack Type Dio Only
#define ENABLE_NEW_PACK_ENCRYPTION																											// Pack encryption Utility
#define USE_EPK_NEW_SIGNATURE	11051984
#define APP_ENABLE_IMPORT_MODULE																											// App enable import module
#define ENABLE_PACK_IMPORT_MODULE																										// Pack import module
#define ENABLE_PREVENT_FILE_READ_FROM_DRIVE																									// Prevent file read from drive
#endif

// #if defined(LIVE_SERVER) && !defined(DEBUG)
// #define ENABLE_PRINT_RECV_PACKET_DEBUG																											// Recv Packet Debug Print (From Packet)
// #endif
#define ENABLE_DAMAGE_LIMIT_REWORK																												// Damage limit to long long
#define ENABLE_MODEL_RENDER_TARGET																												// Model Render Target Utility
#ifdef ENABLE_MODEL_RENDER_TARGET
	#define USE_MODEL_RENDER_TARGET_UI
	#define USE_MODEL_RENDER_TARGET_TEXTTAIL
	#define USE_MODEL_RENDER_TARGET_EFFECT
#endif

#define ENABLE_GRAPHIC_OPTIMIZATION
#ifdef ENABLE_GRAPHIC_OPTIMIZATION
	#define USE_GRAPHIC_OPTIMIZATION_TEST
	#define USE_GRAPHIC_OPTIMIZATION_EFFECT
#endif

#define ENABLE_FOV_OPTION
//#define ENABLE_LOADING_PLAYER_CACHE
#define ENABLE_DAMAGE_DOT
/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
/********************************************************** [Item Defines] ***********************************************************/
/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
#define ENABLE_COSTUME_SWITCH_ITEM																												// Like Official costume switch item

#define ENABLE_BLEND_REWORK																														// Perma blend item
#define ENABLE_PERMA_ACCESSORY_ITEM																												// Perma accessory item
#define ENABLE_ICECREAM_ITEM																													// Icecream item
#define ENABLE_DSS_RECHARGE_ITEM																												// DSS Recharge item
#define ENABLE_DSS_REFINE_ITEM																													// Refine items for dss
#define ENABLE_EXTENDED_PET_ITEM																												// Extended pet item
#define ENABLE_POTION_ITEMS																														// New blends / potion
#define ENABLE_HAIR_COSTUME_ATTRIBUTE																											// Hair attribute item
#define ENABLE_BODY_COSTUME_ATTRIBUTE																											// Body atrribute item
/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
/********************************************************** [Event Defines] ***********************************************************/
/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
#ifdef ENABLE_EVENT_SYSTEM
#define ENABLE_EVENT_CALENDAR_SYSTEM																										// Event calendar gui
#endif
/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
/********************************************************** [Effect Defines] ***********************************************************/
/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
#define ENABLE_AGGR_MONSTER_EFFECT																												// Like Official Aggreate Monster Effect
#define ENABLE_PVP_EFFECT																														// Duel (PvP) Effect
#define ENABLE_SMITH_EFFECT																														// Refine Fail/Success Effect
#define ENABLE_DRAGON_BONE_EFFECT																												// Dragonbone Effect Like Metin2.TC
#define ENABLE_DSS_ACTIVE_EFFECT																												// DSS Active Effect
#define ENABLE_LVL115_ARMOR_EFFECT																												// Lv115 Armor Effect
#define ENABLE_FISHBONUS_REWORK
/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
/********************************************************** [Scp Defines] ***********************************************************/
/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
// System
#define ENABLE_RANK_SYSTEM																														// Rank system
#define ENABLE_LANDRANK_SYSTEM																													// Land Rank system
#define ENABLE_REBORN_SYSTEM																													// Reborn System
#define ENABLE_BIOLOG_SYSTEM																													// Biolog System
#define ENABLE_SAGE_SKILL																														// Sage Skill System
#define ENABLE_EXPERT_SKILL																														// Expert Skill System
#define ENABLE_ALIGNMENT_SYSTEM																													// Alignment System
#define ENABLE_CLEAR_CHAT_SYSTEM																												// Clear chat system
#define ENABLE_CHAT_STACK_SYSTEM																												// Chat stack system
#define ENABLE_WIKI_SYSTEM																														// Wiki system
//#define ENABLE_CHAT_DISCREDIT																													// Chat discredit block system
#define ENABLE_CHAT_STOP																														// Stop chat
#define ENABLE_SCP1453_EXTENTIONS																												// Scp1453 Extensions
#define ENABLE_NEW_DUNGEON_TYPE																													// New Dungeon Type Utility
#define ENABLE_NEW_ACCESORY_ITEMS																												// New accessory items
#define ENABLE_ACC_STATUS_CHECHKUP																												//
#define ENABLE_USE_ITEM_COUNT																													//
#define ENABLE_CHEST_DROP_POINT																													//
/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
/********************************************************** [Marty Defines] ***********************************************************/
/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
#define AUTODETECT_LYCAN_RODNPICK_BONE
#define ENABLE_LOAD_ALTER_ITEMICON
#define ENABLE_SKIN_EXTENDED
#define ENABLE_FIX_MOBS_LAG
/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
/********************************************************** [Python Defines] ***********************************************************/
/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
//#define ENABLE_RANDOM_CHANNEL_SEL																												// Default(0)
//#define ENABLE_CLEAN_DATA_IF_FAIL_LOGIN																											// Default(0)
#define ENABLE_PASTE_FEATURE																													// Default(1)
#define ENABLE_FULLSTONE_DETAILS																												// Default(1)
#define ENABLE_REFINE_PCT																														// Default(1)
#define ENABLE_CHAT_EMOTION_BLOCK																												// Default(1)
#define ENABLE_AFFECT_SHOWER_TOOLTIP_RENEWAL																									// Default(1)
#define ENABLE_HIGHLIGHT_TOOLTIP																												// Default(1)
//#define ENABLE_RACE_FLAG_RENEWAL																												// Default(1)
#define ENABLE_ANTIFLAG_TEXTLINE																												// Default(1)
#define ENABLE_FRIEND_ONLINE_NOTIFICATION																										// Default(1)
#define ENABLE_FAST_EXIT_SYSTEM																													// Default(1)
#define ENABLE_VNUM_TOOLTIP_TEXT																												// Default(1)
#define ENABLE_QUESTION_DLG_TIME_EXTENSION																										// Default(1)
#define ENABLE_UNLIMITED_AUTOPOTION																												// Default(1)
/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
#define LOCALE_SERVICE_EUROPE																													// Localization Europe

#define ENABLE_DOG_MODE
#define ENABLE_BATTLE_PASS_SYSTEM // Battle pass system
#define ENABLE_BATTLE_PASS_EX
#define ENABLE_AUTO_HUNT_SYSTEM																														// Like GF Auto Hunting System
#define ENABLE_PB2_PREMIUM_SYSTEM
#define ENABLE_PB2_NEW_TAG_SYSTEM
#define ENABLE_CHAT_COLOR_SYSTEM
#define ENABLE_NAME_CHANGE_SYSTEM
#define ENABLE_LOVE_SYSTEM
#define ENABLE_WORD_SYSTEM
#define ENABLE_MONIKER_SYSTEM
#define ENABLE_TEAM_SYSTEM
#define MONIKER_MAX_LEN 32

#define APP_VERSION				"V.21.3.1.6"
#define CLIENT_VERSION			"1903"
#ifdef ENABLE_ITEM_EVOLUTION_SYSTEM
#define EV_CLEAN_VALUE0			20
#endif

#if defined(ENABLE_DISCORD_UTILITY)
static const long long discordID = 1155211754910011504;																							// Discord App ID
#endif // defined(ENABLE_DISCORD_UTILITY)

#ifdef LIVE_SERVER
#define ENABLE_ANTICHEAT
#endif
#define ENABLE_MULTIFARM_BLOCK

enum EPhebia
{
	CHANNEL_MAX_NUM = 8,
};
/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/