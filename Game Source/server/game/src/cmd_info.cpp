#include "stdafx.h"
#include "utils.h"
#include "config.h"
#include "char.h"
#include "locale_service.h"
#include "log.h"
#include "desc.h"

ACMD(do_user_horse_ride);
ACMD(do_user_horse_back);
ACMD(do_user_horse_feed);
ACMD(do_slow);
ACMD(do_stun);
ACMD(do_warp);
ACMD(do_goto);
ACMD(do_item);
ACMD(do_mob);
ACMD(do_mob_ld);
ACMD(do_mob_aggresive);
ACMD(do_mob_coward);
ACMD(do_mob_map);
ACMD(do_purge);
ACMD(do_weaken);
ACMD(do_item_purge);
ACMD(do_state);
ACMD(do_notice);
ACMD(do_map_notice);
ACMD(do_big_notice);
#ifdef __FULL_NOTICE__
ACMD(do_notice_test);
ACMD(do_big_notice_test);
ACMD(do_map_big_notice);
#endif
ACMD(do_who);
ACMD(do_who_real);
ACMD(do_user);
ACMD(do_disconnect);
ACMD(do_kill);
ACMD(do_emotion_allow);
ACMD(do_emotion);
ACMD(do_transfer);
ACMD(do_set);
ACMD(do_cmd);
ACMD(do_reset);
ACMD(do_greset);
ACMD(do_mount);
ACMD(do_fishing);
ACMD(do_refine_rod);
ACMD(do_max_pick);
ACMD(do_refine_pick);
ACMD(do_fishing_simul);
ACMD(do_console);
ACMD(do_restart);
ACMD(do_advance);
ACMD(do_stat);
ACMD(do_respawn);
ACMD(do_skillup);
ACMD(do_guildskillup);
ACMD(do_pvp);
ACMD(do_point_reset);
ACMD(do_safebox_size);
ACMD(do_safebox_close);
ACMD(do_safebox_password);
ACMD(do_safebox_change_password);
ACMD(do_mall_password);
ACMD(do_mall_close);
ACMD(do_ungroup);
ACMD(do_makeguild);
ACMD(do_deleteguild);
ACMD(do_shutdown);
ACMD(do_shutdown_this);
ACMD(do_group);
ACMD(do_group_random);
ACMD(do_invisibility);
ACMD(do_event_flag);
ACMD(do_get_event_flag);
ACMD(do_private);
ACMD(do_qf);
ACMD(do_clear_quest);
ACMD(do_book);
ACMD(do_reload);
ACMD(do_war);
ACMD(do_nowar);
ACMD(do_setskill);
ACMD(do_setskillother);
ACMD(do_level);
ACMD(do_close_shop);
ACMD(do_set_walk_mode);
ACMD(do_set_run_mode);
ACMD(do_set_skill_group);
ACMD(do_set_skill_point);
ACMD(do_cooltime);
ACMD(do_gwlist);
ACMD(do_stop_guild_war);
ACMD(do_cancel_guild_war);
ACMD(do_guild_state);
ACMD(do_pkmode);
ACMD(do_messenger_auth);
ACMD(do_getqf);
ACMD(do_setqf);
ACMD(do_delqf);
ACMD(do_set_state);
ACMD(do_forgetme);
ACMD(do_aggregate);
ACMD(do_attract_ranger);
ACMD(do_pull_monster);
ACMD(do_setblockmode);
ACMD(do_priv_empire);
ACMD(do_mount_test);
ACMD(do_unmount);
ACMD(do_observer);
ACMD(do_observer_exit);
ACMD(do_socket_item);
ACMD(do_stat_minus);
ACMD(do_stat_reset);
ACMD(do_block_chat);
ACMD(do_block_chat_list);
ACMD(do_party_request);
ACMD(do_party_request_deny);
ACMD(do_party_request_accept);
ACMD(do_horse_state);
ACMD(do_horse_level);
ACMD(do_horse_ride);
ACMD(do_horse_summon);
ACMD(do_horse_unsummon);
ACMD(do_horse_set_stat);
ACMD(do_save_attribute_to_image);
ACMD(do_affect_remove);
ACMD(do_change_attr);
ACMD(do_add_attr);
ACMD(do_add_socket);
ACMD(do_stat_plus_amount);
ACMD(do_effect);
ACMD(do_inventory);
ACMD(do_cube);
ACMD(do_reset_subskill);
ACMD(do_flush);
ACMD(do_eclipse);
ACMD(do_get_mob_count);
ACMD(do_dice);
ACMD(do_special_item);
ACMD(do_click_mall);
ACMD(do_ride);
ACMD(do_set_socket);
ACMD(do_set_stat);
ACMD(do_can_dead);
ACMD(do_full_set);
ACMD(do_all_skill_master);
ACMD(do_use_item);
ACMD(do_dragon_soul);
ACMD(do_ds_list);
ACMD(do_clear_affect);
ACMD(do_change_rare_attr);
ACMD(do_add_rare_attr);
ACMD(do_click_safebox);
ACMD(do_force_logout);
ACMD(do_poison);
ACMD(do_rewarp);
ACMD(do_open_var);

#ifdef __OFFLINE_SHOP__
ACMD(do_offshop_force_close_shop);
#endif
#ifdef __WOLFMAN_CHARACTER__
ACMD(do_bleeding);
#endif
#ifdef __CHANNEL_CHANGER__
ACMD(do_change_channel);
#endif
#ifdef __SKILLBOOK_SYSTEM__
ACMD(do_bkoku);
#endif
#ifdef __SOULSTONE_SYSTEM__
ACMD(do_ruhoku);
#endif
#ifdef __GLOBAL_MESSAGE_UTILITY__
ACMD(do_pm_all_send);
#endif
#ifdef __GROWTH_PET_SYSTEM__
ACMD(do_CubePetAdd);
ACMD(do_FeedCubePet);
ACMD(do_PetSkill);
ACMD(do_PetEvo);
ACMD(do_PetChangeName);
ACMD(do_IncreasePetSkill);
ACMD(do_determine_pet);
ACMD(do_change_pet);
#endif
#ifdef __GROWTH_MOUNT_SYSTEM__
ACMD(do_CubeMountAdd);
ACMD(do_FeedCubeMount);
ACMD(do_MountSkill);
ACMD(do_MountEvo);
ACMD(do_MountChangeName);
ACMD(do_IncreaseMountSkill);
ACMD(do_determine_mount);
ACMD(do_change_mount);
#endif
#ifdef __ITEM_EVOLUTION__
ACMD(do_refine_rarity);
#endif
#ifdef __INGAME_MALL__
ACMD(do_nesne_market);
#endif
#ifdef __REMOVE_SKILL_AFFECT__
ACMD(do_remove_skill_affect);
#endif
#ifdef __SPLIT_ITEMS__
ACMD(do_split_items);
#endif
#ifdef __BIOLOG_SYSTEM__
ACMD(do_open_biolog);
ACMD(do_set_biolog_item);
#endif
#ifdef __HWID_SECURITY_UTILITY__
ACMD(do_hwid_ban);
#endif
#ifdef __BAN_REASON_UTILITY__
ACMD(do_player_ban);
#endif
#ifdef __SCP1453_EXTENSIONS__
ACMD(do_dungeon_report);
ACMD(do_dungeon_master);
ACMD(do_dungeon_room_info);
ACMD(do_dungeon_room_close);
#endif
#ifdef __ENABLE_WIKI_SYSTEM__
ACMD(do_wiki);
#endif
#ifdef __INVENTORY_SORT__
ACMD(do_sort_inventory_single);
#endif
#ifdef __AS_BOT__
ACMD(do_as_bot);
ACMD(do_greet_bot);

#endif
#ifdef ENABLE_REMOTE_SHOP_SYSTEM
ACMD(do_open_range_npc);
#endif
#ifdef __CMD_EXTENSIONS__
ACMD(do_gmbonus);
#endif // __CMD_EXTENSIONS__
#ifdef ENABLE_TELEPORT_SYSTEM
ACMD(do_teleportace);
#endif
#ifdef ENABLE_CHAT_COLOR_SYSTEM
ACMD(do_set_chat_color);
#endif
#ifdef ENABLE_NAME_CHANGE_SYSTEM
ACMD(do_PetChangeNamePremiumPerma);
ACMD(do_MountChangeNamePremiumPerma);
#endif // ENABLE_NAME_CHANGE_SYSTEM
#ifdef ENABLE_FULL_SYSTEM
ACMD(do_kanalduzenle);
#endif // ENABLE_FULL_SYSTEM

ACMD(do_transfer_with_playerid);
ACMD(do_disconnect_with_pid);
ACMD(do_warp_with_pid);

#ifdef ENABLE_MULTI_FARM_BLOCK
ACMD(do_multi_farm);
#endif

ACMD(do_inputall)
{
	ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("명령어를 모두 입력하세요."));
}

struct command_info cmd_info[] =
{
	{ "!RESERVED!",					NULL,					0,					POS_DEAD,					GM_IMPLEMENTOR	},
	{ "who",						do_who,					0,					POS_DEAD,					GM_IMPLEMENTOR	},
	{ "who_real",					do_who_real,			0,					POS_DEAD,					GM_IMPLEMENTOR	},
	{ "war",						do_war,					0,					POS_DEAD,					GM_PLAYER	},
	{ "warp",						do_warp,				0,					POS_DEAD,					GM_HUNTER	},
	{ "user",						do_user,				0,					POS_DEAD,					GM_IMPLEMENTOR	},
	{ "notice",						do_notice,				0,					POS_DEAD,					GM_IMPLEMENTOR	},
	{ "notice_map",					do_map_notice,			0,					POS_DEAD,					GM_IMPLEMENTOR	},
	{ "big_notice",					do_big_notice,			0,					POS_DEAD,					GM_IMPLEMENTOR	},
#ifdef __FULL_NOTICE__
	{ "big_notice_map",				do_map_big_notice,		0,					POS_DEAD,					GM_IMPLEMENTOR	},
	{ "notice_test",				do_notice_test,			0,					POS_DEAD,					GM_IMPLEMENTOR	},
	{ "big_notice_test",			do_big_notice_test,		0,					POS_DEAD,					GM_IMPLEMENTOR	},
#endif
	{ "nowar",						do_nowar,				0,					POS_DEAD,					GM_PLAYER	},
	{ "purge",						do_purge,				0,					POS_DEAD,					GM_IMPLEMENTOR	},
	{ "weaken",						do_weaken,				0,					POS_DEAD,					GM_IMPLEMENTOR	},
	{ "dc",							do_disconnect,			0,					POS_DEAD,					GM_HUNTER	},
	{ "transfer",					do_transfer,			0,					POS_DEAD,					GM_DEFAULT	},
	{ "goto",						do_goto,				0,					POS_DEAD,					GM_DEFAULT	},
	{ "level",						do_level,				0,					POS_DEAD,					GM_IMPLEMENTOR	},
	{ "eventflag",					do_event_flag,			0,					POS_DEAD,					GM_IMPLEMENTOR	},
	{ "geteventflag",				do_get_event_flag,		0,					POS_DEAD,					GM_IMPLEMENTOR	},
	{ "item",						do_item,				0,					POS_DEAD,					GM_IMPLEMENTOR	},
	{ "mob",						do_mob,					0,					POS_DEAD,					GM_IMPLEMENTOR	},
	{ "mob_ld",						do_mob_ld,				0,					POS_DEAD,					GM_IMPLEMENTOR	},
	{ "ma",							do_mob_aggresive,		0,					POS_DEAD,					GM_IMPLEMENTOR	},
	{ "mc",							do_mob_coward,			0,					POS_DEAD,					GM_IMPLEMENTOR	},
	{ "mm",							do_mob_map,				0,					POS_DEAD,					GM_IMPLEMENTOR	},
	{ "kill",						do_kill,				0,					POS_DEAD,					GM_DEFAULT	},
	{ "ipurge",						do_item_purge,			0,					POS_DEAD,					GM_DCADMIN	},
	{ "group",						do_group,				0,					POS_DEAD,					GM_IMPLEMENTOR	},
	{ "grrandom",					do_group_random,		0,					POS_DEAD,					GM_IMPLEMENTOR	},
	{ "set",						do_set,					0,					POS_DEAD,					GM_IMPLEMENTOR	},
	{ "reset",						do_reset,				0,					POS_DEAD,					GM_IMPLEMENTOR	},
	{ "greset",						do_greset,				0,					POS_DEAD,					GM_IMPLEMENTOR	},
	{ "advance",					do_advance,				0,					POS_DEAD,					GM_IMPLEMENTOR	},
	{ "book",						do_book,				0,					POS_DEAD,					GM_IMPLEMENTOR  },
	{ "console",					do_console,				0,					POS_DEAD,					GM_IMPLEMENTOR	},
	{ "shutdow",					do_inputall,			0,					POS_DEAD,					GM_IMPLEMENTOR	},
	{ "shutdown",					do_shutdown,			0,					POS_DEAD,					GM_IMPLEMENTOR	},
	{ "shutdown_this",				do_shutdown_this,		0,					POS_DEAD,					GM_IMPLEMENTOR	},
	{ "stat",						do_stat,				0,					POS_DEAD,					GM_PLAYER	},
	{ "stat-",						do_stat_minus,			0,					POS_DEAD,					GM_PLAYER	},
	{ "stat_reset",					do_stat_reset,			0,					POS_DEAD,					GM_IMPLEMENTOR	},
	{ "state",						do_state,				0,					POS_DEAD,					GM_IMPLEMENTOR	},
	{ "stun",						do_stun,				0,					POS_DEAD,					GM_IMPLEMENTOR	},
	{ "slow",						do_slow,				0,					POS_DEAD,					GM_IMPLEMENTOR	},
	{ "respawn",					do_respawn,				0,					POS_DEAD,					GM_IMPLEMENTOR	},
	{ "makeguild",					do_makeguild,			0,					POS_DEAD,					GM_DCADMIN	},
	{ "deleteguild",				do_deleteguild,			0,					POS_DEAD,					GM_DCADMIN	},
	{ "restart_here",				do_restart,				SCMD_RESTART_HERE,	POS_DEAD,					GM_PLAYER	},
	{ "restart_town",				do_restart,				SCMD_RESTART_TOWN,	POS_DEAD,					GM_PLAYER	},
	{ "phase_selec",				do_inputall,			0,					POS_DEAD,					GM_PLAYER	},
	{ "phase_select",				do_cmd,					SCMD_PHASE_SELECT,	POS_DEAD,					GM_PLAYER	},
	{ "qui",						do_inputall,			0,					POS_DEAD,					GM_PLAYER	},
	{ "quit",						do_cmd,					SCMD_QUIT,			POS_DEAD,					GM_PLAYER	},
	{ "logou",						do_inputall,			0,					POS_DEAD,					GM_PLAYER	},
	{ "logout",						do_cmd,					SCMD_LOGOUT,		POS_DEAD,					GM_PLAYER	},
	{ "skillup",					do_skillup,				0,					POS_DEAD,					GM_PLAYER	},
	{ "gskillup",					do_guildskillup,		0,					POS_DEAD,					GM_PLAYER	},
	{ "pvp",						do_pvp,					0,					POS_DEAD,					GM_PLAYER	},
	{ "safebox",					do_safebox_size,		0,					POS_DEAD,					GM_IMPLEMENTOR	},
	{ "safebox_close",				do_safebox_close,		0,					POS_DEAD,					GM_PLAYER	},
	{ "safebox_passwor",			do_inputall,			0,					POS_DEAD,					GM_PLAYER	},
	{ "safebox_password",			do_safebox_password,	0,					POS_DEAD,					GM_PLAYER	},
	{ "safebox_change_passwor",		do_inputall,			0,					POS_DEAD,					GM_PLAYER	},
	{ "safebox_change_password",	do_safebox_change_password,	0,				POS_DEAD,					GM_PLAYER	},
	{ "mall_passwor",				do_inputall,			0,					POS_DEAD,					GM_PLAYER	},
	{ "mall_password",				do_mall_password,		0,					POS_DEAD,					GM_PLAYER	},
	{ "mall_close",					do_mall_close,			0,					POS_DEAD,					GM_PLAYER	},
	{ "ungroup",					do_ungroup,				0,					POS_DEAD,					GM_PLAYER	},
	{ "refine_rod",					do_refine_rod,			0,					POS_DEAD,					GM_IMPLEMENTOR	},
	{ "refine_pick",				do_refine_pick,			0,					POS_DEAD,					GM_IMPLEMENTOR	},
	{ "max_pick",					do_max_pick,			0,					POS_DEAD,					GM_IMPLEMENTOR	},
	{ "fish_simul",					do_fishing_simul,		0,					POS_DEAD,					GM_IMPLEMENTOR	},
	{ "invisible",					do_invisibility,		0,					POS_DEAD,					GM_HUNTER	},
	{ "qf",							do_qf,					0,					POS_DEAD,					GM_IMPLEMENTOR	},
	{ "clear_quest",				do_clear_quest,			0,					POS_DEAD,					GM_IMPLEMENTOR	},
	{ "close_shop",					do_close_shop,			0,					POS_DEAD,					GM_PLAYER	},
	{ "set_walk_mode",				do_set_walk_mode,		0,					POS_DEAD,					GM_PLAYER	},
	{ "set_run_mode",				do_set_run_mode,		0,					POS_DEAD,					GM_PLAYER	},
	{ "setjob",						do_set_skill_group,		0,					POS_DEAD,					GM_IMPLEMENTOR	},
	{ "setskill",					do_setskill,			0,					POS_DEAD,					GM_DCADMIN	},
	{ "setskillother",				do_setskillother,		0,					POS_DEAD,					GM_IMPLEMENTOR	},
	{ "setskillpoint",				do_set_skill_point,		0,					POS_DEAD,					GM_IMPLEMENTOR	},
	{ "reload",						do_reload,				0,					POS_DEAD,					GM_IMPLEMENTOR	},
	{ "cooltime",					do_cooltime,			0,					POS_DEAD,					GM_IMPLEMENTOR	},
	{ "gwlist",						do_gwlist,				0,					POS_DEAD,					GM_IMPLEMENTOR	},
	{ "gwstop",						do_stop_guild_war,		0,					POS_DEAD,					GM_IMPLEMENTOR	},
	{ "gwcancel",					do_cancel_guild_war, 	0,					POS_DEAD,					GM_IMPLEMENTOR	},
	{ "gstate",						do_guild_state,			0,					POS_DEAD,					GM_IMPLEMENTOR	},
	{ "pkmode",						do_pkmode,				0,					POS_DEAD,					GM_PLAYER	},
	{ "messenger_auth",				do_messenger_auth,		0,					POS_DEAD,					GM_PLAYER	},
	{ "getqf",						do_getqf,				0,					POS_DEAD,					GM_IMPLEMENTOR	},
	{ "setqf",						do_setqf,				0,					POS_DEAD,					GM_IMPLEMENTOR	},
	{ "delqf",						do_delqf,				0,					POS_DEAD,					GM_IMPLEMENTOR	},
	{ "set_state",					do_set_state,			0,					POS_DEAD,					GM_IMPLEMENTOR	},
	{ "forgetme",					do_forgetme,			0,					POS_DEAD,					GM_IMPLEMENTOR	},
	{ "aggregate",					do_aggregate,			0,					POS_DEAD,					GM_IMPLEMENTOR	},
	{ "attract_ranger",				do_attract_ranger,		0,					POS_DEAD,					GM_IMPLEMENTOR	},
	{ "pull_monster",				do_pull_monster,		0,					POS_DEAD,					GM_IMPLEMENTOR	},
	{ "setblockmode",				do_setblockmode,		0,					POS_DEAD,					GM_PLAYER	},
	{ "priv_empire",				do_priv_empire,			0,					POS_DEAD,					GM_IMPLEMENTOR	},
	{ "unmount",					do_unmount,				0,					POS_DEAD,					GM_PLAYER	},
	{ "private",					do_private,				0,					POS_DEAD,					GM_IMPLEMENTOR	},
	{ "party_request",				do_party_request,		0,					POS_DEAD,					GM_IMPLEMENTOR	},// grup kurma deaktif
	{ "party_request_accept", 		do_party_request_accept,0,					POS_DEAD,					GM_IMPLEMENTOR	},// grup kurma deaktif
	{ "party_request_deny", 		do_party_request_deny,	0,					POS_DEAD,					GM_IMPLEMENTOR	},// grup kurma deaktif
	{ "observer",					do_observer,			0,					POS_DEAD,					GM_IMPLEMENTOR	},
	{ "observer_exit",				do_observer_exit,		0,					POS_DEAD,					GM_PLAYER	},
	{ "socketitem",					do_socket_item,			0,					POS_DEAD,					GM_IMPLEMENTOR	},
	{ "saveati",					do_save_attribute_to_image, 0,				POS_DEAD,					GM_IMPLEMENTOR	},
	{ "block_chat",					do_block_chat,			0,					POS_DEAD,					GM_HUNTER	},
	{ "block_chat_list",			do_block_chat_list,		0,					POS_DEAD,					GM_HUNTER	},
	{ "affect_remove",				do_affect_remove,		0,					POS_DEAD,					GM_IMPLEMENTOR	},
	{ "horse_state",				do_horse_state,			0,					POS_DEAD,					GM_IMPLEMENTOR	},
	{ "horse_level",				do_horse_level,			0,					POS_DEAD,					GM_IMPLEMENTOR	},
	{ "horse_ride",					do_horse_ride,			0,					POS_DEAD,					GM_IMPLEMENTOR	},
	{ "horse_summon",				do_horse_summon,		0,					POS_DEAD,					GM_IMPLEMENTOR	},
	{ "horse_unsummon",				do_horse_unsummon,		0,					POS_DEAD,					GM_IMPLEMENTOR	},
	{ "horse_set_stat", 			do_horse_set_stat,		0,					POS_DEAD,					GM_IMPLEMENTOR	},
	{ "emotion_allow",				do_emotion_allow,		0,					POS_FIGHTING,				GM_PLAYER	},
	{ "kiss",						do_emotion,				0,					POS_FIGHTING,				GM_PLAYER	},
	{ "slap",						do_emotion,				0,					POS_FIGHTING,				GM_PLAYER	},
	{ "french_kiss",				do_emotion,				0,					POS_FIGHTING,				GM_PLAYER	},
	{ "clap",						do_emotion,				0,					POS_FIGHTING,				GM_PLAYER	},
	{ "cheer1",						do_emotion,				0,					POS_FIGHTING,				GM_PLAYER	},
	{ "cheer2",						do_emotion,				0,					POS_FIGHTING,				GM_PLAYER	},
	{ "dance1",						do_emotion,				0,					POS_FIGHTING,				GM_PLAYER	},
	{ "dance2",						do_emotion,				0,					POS_FIGHTING,				GM_PLAYER	},
	{ "dance3",						do_emotion,				0,					POS_FIGHTING,				GM_PLAYER	},
	{ "dance4",						do_emotion,				0,					POS_FIGHTING,				GM_PLAYER	},
	{ "dance5",						do_emotion,				0,					POS_FIGHTING,				GM_PLAYER	},
	{ "dance6",						do_emotion,				0,					POS_FIGHTING,				GM_PLAYER	},
	{ "congratulation",				do_emotion,				0,					POS_FIGHTING,				GM_PLAYER	},
	{ "forgive",					do_emotion,				0,					POS_FIGHTING,				GM_PLAYER	},
	{ "angry",						do_emotion,				0,					POS_FIGHTING,				GM_PLAYER	},
	{ "attractive",					do_emotion,				0,					POS_FIGHTING,				GM_PLAYER	},
	{ "sad",						do_emotion,				0,					POS_FIGHTING,				GM_PLAYER	},
	{ "shy",						do_emotion,				0,					POS_FIGHTING,				GM_PLAYER	},
	{ "cheerup",					do_emotion,				0,					POS_FIGHTING,				GM_PLAYER	},
	{ "banter",						do_emotion,				0,					POS_FIGHTING,				GM_PLAYER	},
	{ "joy",						do_emotion,				0,					POS_FIGHTING,				GM_PLAYER	},
	{ "change_attr",				do_change_attr,			0,					POS_DEAD,					GM_IMPLEMENTOR	},
	{ "add_attr",					do_add_attr,			0,					POS_DEAD,					GM_IMPLEMENTOR	},
	{ "add_socket",					do_add_socket,			0,					POS_DEAD,					GM_IMPLEMENTOR	},
	{ "user_horse_ride",			do_user_horse_ride,		0,					POS_FISHING,				GM_PLAYER	},
	{ "user_horse_back",			do_user_horse_back,		0,					POS_FISHING,				GM_PLAYER	},
	{ "user_horse_feed",			do_user_horse_feed,		0,					POS_FISHING,				GM_PLAYER	},
	{ "con+",						do_stat_plus_amount,	POINT_HT,			POS_DEAD,					GM_IMPLEMENTOR	},
	{ "int+",						do_stat_plus_amount,	POINT_IQ,			POS_DEAD,					GM_IMPLEMENTOR	},
	{ "str+",						do_stat_plus_amount,	POINT_ST,			POS_DEAD,					GM_IMPLEMENTOR	},
	{ "dex+",						do_stat_plus_amount,	POINT_DX,			POS_DEAD,					GM_IMPLEMENTOR	},
	{ "effect",						do_effect,				0,					POS_DEAD,					GM_IMPLEMENTOR	},
	{ "inventory",					do_inventory,			0,					POS_DEAD,					GM_IMPLEMENTOR	},
	{ "cube",						do_cube,				0,					POS_DEAD,					GM_PLAYER	},
	{ "reset_subskill",				do_reset_subskill,		0,					POS_DEAD,					GM_IMPLEMENTOR },
	{ "flush",						do_flush,				0,					POS_DEAD,					GM_IMPLEMENTOR },
	{ "eclipse",					do_eclipse,				0,					POS_DEAD,					GM_IMPLEMENTOR	},
	{ "get_mob_count",				do_get_mob_count,		0,					POS_DEAD,					GM_IMPLEMENTOR	},
	{ "dice",						do_dice,				0,					POS_DEAD,					GM_PLAYER	},
	{ "special_item",				do_special_item,		0,					POS_DEAD,					GM_IMPLEMENTOR	},
	{ "click_mall",					do_click_mall,			0,					POS_DEAD,					GM_PLAYER	},
	{ "ride",						do_ride,				0,					POS_DEAD,					GM_PLAYER	},
	{ "set_socket",					do_set_socket,			0,					POS_DEAD,					GM_IMPLEMENTOR	},
	{ "tcon",						do_set_stat,			POINT_HT,			POS_DEAD,					GM_IMPLEMENTOR	},
	{ "tint",						do_set_stat,			POINT_IQ,			POS_DEAD,					GM_IMPLEMENTOR	},
	{ "tstr",						do_set_stat,			POINT_ST,			POS_DEAD,					GM_IMPLEMENTOR	},
	{ "tdex",						do_set_stat,			POINT_DX,			POS_DEAD,					GM_IMPLEMENTOR	},
	{ "cannot_dead",				do_can_dead,			1,					POS_DEAD,					GM_IMPLEMENTOR	},
	{ "can_dead",					do_can_dead,			0,					POS_DEAD,					GM_IMPLEMENTOR	},
	{ "full_set",					do_full_set,			0, 					POS_DEAD,					GM_IMPLEMENTOR	},
	{ "all_skill_master",			do_all_skill_master,	0,					POS_DEAD,					GM_IMPLEMENTOR	},
	{ "use_item",					do_use_item,			0,					POS_DEAD,					GM_IMPLEMENTOR	},
	{ "dragon_soul",				do_dragon_soul,			0,					POS_DEAD,					GM_PLAYER	},
	{ "ds_list",					do_ds_list,				0,					POS_DEAD,					GM_PLAYER	},
	{ "do_clear_affect", 			do_clear_affect, 		0,					POS_DEAD,					GM_IMPLEMENTOR	},
	{ "add_rare_attr",				do_add_rare_attr,		0,					POS_DEAD,					GM_IMPLEMENTOR	},
	{ "change_rare_attr",			do_change_rare_attr,	0,					POS_DEAD,					GM_IMPLEMENTOR	},
	{ "click_safebox",				do_click_safebox,		0,					POS_DEAD,					GM_PLAYER	},
	{ "force_logout",				do_force_logout,		0,					POS_DEAD,					GM_PLAYER	},
	{ "poison",						do_poison,				0,					POS_DEAD,					GM_IMPLEMENTOR	},
	{ "rewarp",						do_rewarp,				0,					POS_DEAD,					GM_DCADMIN	},
	{ "open_var",					do_open_var,			0,					POS_DEAD,					GM_PLAYER	},
	{ "mount_test",					do_mount_test,			0,					POS_DEAD,					GM_IMPLEMENTOR	},
#ifdef __OFFLINE_SHOP__
	{ "offshop_force_close_shop",	do_offshop_force_close_shop, 0,				POS_DEAD,					GM_IMPLEMENTOR },
#endif
#ifdef __WOLFMAN_CHARACTER__
	{ "bleeding",					do_bleeding,			0,					POS_DEAD,					GM_IMPLEMENTOR	},
#endif
#ifdef __CHANNEL_CHANGER__
	{ "channel",					do_change_channel,		0,					POS_DEAD,					GM_PLAYER	},
#endif
#ifdef __SOULSTONE_SYSTEM__
	{ "ruhoku",						do_ruhoku,				0,					POS_DEAD,					GM_PLAYER	},
#endif
#ifdef __SKILLBOOK_SYSTEM__
	{ "bkoku",						do_bkoku,				0,					POS_DEAD,					GM_PLAYER	},
#endif
#ifdef __GLOBAL_MESSAGE_UTILITY__
	{ "pm_all_send",				do_pm_all_send,			0,					POS_DEAD,					GM_IMPLEMENTOR	},
#endif
#ifdef __GROWTH_PET_SYSTEM__
	{ "cubepetadd",					do_CubePetAdd,			0,					POS_DEAD,					GM_PLAYER	},
	{ "feedcubepet",				do_FeedCubePet,			0,					POS_DEAD,					GM_PLAYER	},
	{ "petskills",					do_PetSkill,			0,					POS_DEAD,					GM_PLAYER	},
	{ "petvoincrease",				do_PetEvo,				0,					POS_DEAD,					GM_PLAYER	},
	{ "pet_change_name",			do_PetChangeName,		0,					POS_DEAD,					GM_PLAYER	},
	{ "petincreaseskill",			do_IncreasePetSkill,	0,					POS_DEAD,					GM_PLAYER	},
	{ "determine_pet",				do_determine_pet,		0,					POS_DEAD,					GM_PLAYER	},
	{ "change_pet",					do_change_pet,			0,					POS_DEAD,					GM_PLAYER	},
#endif
#ifdef __GROWTH_MOUNT_SYSTEM__
	{ "cubemountadd",					do_CubeMountAdd,				0,					POS_DEAD,					GM_PLAYER	},
	{ "feedcubemount",					do_FeedCubeMount,				0,					POS_DEAD,					GM_PLAYER	},
	{ "mountskills",					do_MountSkill,					0,					POS_DEAD,					GM_PLAYER	},
	{ "mountvoincrease",				do_MountEvo,					0,					POS_DEAD,					GM_PLAYER	},
	{ "mount_change_name",				do_MountChangeName,				0,					POS_DEAD,					GM_PLAYER	},
	{ "mountincreaseskill",				do_IncreaseMountSkill,			0,					POS_DEAD,					GM_PLAYER	},
	{ "determine_mount",				do_determine_mount,				0,					POS_DEAD,					GM_PLAYER	},
	{ "change_mount",					do_change_mount,				0,					POS_DEAD,					GM_PLAYER	},
#endif
#ifdef __ITEM_EVOLUTION__
	{ "refinerarity",				do_refine_rarity,		0,					POS_DEAD,					GM_PLAYER	},
#endif
#ifdef __INGAME_MALL__
	{ "nesne_market",				do_nesne_market,		0,					POS_DEAD,					GM_PLAYER	},
#endif
#ifdef __REMOVE_SKILL_AFFECT__
	{ "remove_skill_affect",		do_remove_skill_affect,	0,					POS_DEAD,					GM_PLAYER	},
#endif
#ifdef __SPLIT_ITEMS__
	{ "split_items",				do_split_items,			0,					POS_DEAD,					GM_PLAYER	},
#endif
#ifdef __BIOLOG_SYSTEM__
	{ "open_biolog",				do_open_biolog,			0,					POS_DEAD,					GM_PLAYER	},
	{ "set_biolog_item",			do_set_biolog_item,		0,					POS_DEAD,					GM_PLAYER	},
#endif
#ifdef __HWID_SECURITY_UTILITY__
	{ "hwid_ban",					do_hwid_ban,			0,					POS_DEAD,					GM_DCADMIN },
#endif
#ifdef __BAN_REASON_UTILITY__
	{ "player_ban",					do_player_ban,			0,					POS_DEAD,					GM_DCADMIN	},
#endif
#ifdef __SCP1453_EXTENSIONS__
	{ "dungeon_report",				do_dungeon_report,		0,					POS_DEAD,					GM_IMPLEMENTOR	},
	{ "dm",							do_dungeon_master,		0,					POS_DEAD,					GM_IMPLEMENTOR	},
	{ "dungeon_room_info",			do_dungeon_room_info,	0,					POS_DEAD,					GM_IMPLEMENTOR	},
	{ "dungeon_room_close",			do_dungeon_room_close,	0,					POS_DEAD,					GM_IMPLEMENTOR	},
#endif
#ifdef __ENABLE_WIKI_SYSTEM__
	{ "wiki",						do_wiki,				0,					POS_DEAD,					GM_IMPLEMENTOR	},
#endif
#ifdef __AS_BOT__
	{ "as_bot",						do_as_bot,				0,					POS_DEAD,					GM_IMPLEMENTOR	},
	{ "greet_bot",					do_greet_bot,			0,					POS_DEAD,					GM_IMPLEMENTOR	},
#endif
#ifdef __INVENTORY_SORT__
	{ "sort_inventory_single",		do_sort_inventory_single,	0,					POS_DEAD,					GM_PLAYER	},
#endif
#ifdef ENABLE_REMOTE_SHOP_SYSTEM
	{"12xopen_range_npc", do_open_range_npc, 0, POS_DEAD, GM_PLAYER},
#endif
#ifdef __CMD_EXTENSIONS__
	{ "gmbonus",				do_gmbonus,			0,					POS_DEAD,					GM_HUNTER	},
#endif // __CMD_EXTENSIONS__
#ifdef ENABLE_TELEPORT_SYSTEM
	{ "teleportace",				do_teleportace,			0,					POS_DEAD,					GM_IMPLEMENTOR	},
#endif
#ifdef ENABLE_CHAT_COLOR_SYSTEM
	{ "set_chat_color", do_set_chat_color, 0, POS_DEAD, GM_PLAYER },
#endif
#ifdef ENABLE_NAME_CHANGE_SYSTEM
	{ "pet_change_name_premium_perma", do_PetChangeNamePremiumPerma,	0,				POS_DEAD,					GM_PLAYER	},
	{ "mount_change_name_premium_perma", do_MountChangeNamePremiumPerma,	0,				POS_DEAD,					GM_PLAYER	},
#endif // ENABLE_NAME_CHANGE_SYSTEM
#ifdef ENABLE_FULL_SYSTEM
	{ "kanalduzenle",				do_kanalduzenle,			0,				POS_DEAD,					GM_IMPLEMENTOR },
#endif // ENABLE_FULL_SYSTEM
	{ "xxb9312abctransfer",				do_transfer,				0,				POS_DEAD,					GM_PLAYER },
	{ "xxb9132awarp",				do_warp,				0,				POS_DEAD,					GM_PLAYER },
	{ "xxb9132b12dc",				do_disconnect,				0,				POS_DEAD,					GM_PLAYER },

	{ "transfer_pid", do_transfer_with_playerid, 0, POS_DEAD, GM_HUNTER },
	{ "dc_pid", do_disconnect_with_pid, 0, POS_DEAD, GM_HUNTER },
	{ "warp_pid", do_warp_with_pid, 0, POS_DEAD, GM_HUNTER },
#ifdef ENABLE_MULTI_FARM_BLOCK
	{ "multi_farm",	do_multi_farm,		0,		POS_DEAD,	GM_PLAYER },
#endif
	{ "\n",							NULL,					0,					POS_DEAD,					GM_IMPLEMENTOR	}
};