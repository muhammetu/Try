#pragma once

#include "Locale.h"
#include "../gamelib/RaceData.h"
#include "../gamelib/ItemData.h"

#include <stdint.h>
#include <cinttypes>

typedef BYTE TPacketHeader;

typedef long long		LLONG;
typedef LLONG HPTYPE;

#define PACKET_CG_START	150
#define PACKET_GC_START	150

enum EPacketClientToGame
{
	HEADER_CG_TIME_SYNC						= 0xfc,
	HEADER_CG_HANDSHAKE						= 0xff,
	HEADER_CG_PONG							= 0xfe,
	HEADER_CG_ATTACK						= 1,
	HEADER_CG_CHAT							= 2,
	HEADER_CG_PLAYER_CREATE					= 3,
	HEADER_CG_PLAYER_DESTROY				= 4,
	HEADER_CG_PLAYER_SELECT					= 5,
	HEADER_CG_CHARACTER_MOVE				= 6,
	HEADER_CG_SYNC_POSITION					= 7,
	HEADER_CG_ENTERGAME						= 8,
	HEADER_CG_ITEM_USE						= 9,
	HEADER_CG_ITEM_MOVE						= 10,
	HEADER_CG_ITEM_PICKUP					= 11,
	HEADER_CG_QUICKSLOT_ADD					= 12,
	HEADER_CG_QUICKSLOT_DEL					= 13,
	HEADER_CG_QUICKSLOT_SWAP				= 14,
	HEADER_CG_WHISPER						= 15,
	HEADER_CG_ON_CLICK						= 16,
	HEADER_CG_EXCHANGE						= 17,
	HEADER_CG_CHARACTER_POSITION			= 18,
	HEADER_CG_SCRIPT_ANSWER					= 19,
	HEADER_CG_QUEST_INPUT_STRING			= 20,
	HEADER_CG_QUEST_CONFIRM					= 21,
	HEADER_CG_SHOP							= 22,
	HEADER_CG_FLY_TARGETING					= 23,
	HEADER_CG_USE_SKILL						= 24,
	HEADER_CG_ADD_FLY_TARGETING				= 25,
	HEADER_CG_SHOOT							= 26,
	HEADER_CG_MYSHOP						= 27,
	HEADER_CG_ITEM_USE_TO_ITEM				= 28,
	HEADER_CG_TARGET						= 29,
	HEADER_CG_SCRIPT_BUTTON					= 30,
	HEADER_CG_MESSENGER						= 31,
	HEADER_CG_MALL_CHECKOUT					= 32,
	HEADER_CG_SAFEBOX_CHECKIN				= 33,
	HEADER_CG_SAFEBOX_CHECKOUT				= 34,
	HEADER_CG_PARTY_INVITE					= 35,
	HEADER_CG_PARTY_INVITE_ANSWER			= 36,
	HEADER_CG_PARTY_REMOVE					= 37,
	HEADER_CG_PARTY_SET_STATE				= 38,
	HEADER_CG_SAFEBOX_ITEM_MOVE				= 39,
	HEADER_CG_PARTY_PARAMETER				= 40,
	HEADER_CG_GUILD							= 41,
	HEADER_CG_ANSWER_MAKE_GUILD				= 42,
	HEADER_CG_FISHING						= 43,
	HEADER_CG_GIVE_ITEM						= 44,
	HEADER_CG_EMPIRE						= 45,
	HEADER_CG_REFINE						= 46,
	HEADER_CG_MARK_LOGIN					= 47,
	HEADER_CG_MARK_CRCLIST					= 48,
	HEADER_CG_MARK_UPLOAD					= 49,
	HEADER_CG_MARK_IDXLIST					= 50,
	HEADER_CG_HACK							= 51,
	HEADER_CG_CHANGE_NAME					= 52,
	HEADER_CG_SCRIPT_SELECT_ITEM			= 53,
	HEADER_CG_DRAGON_SOUL_REFINE			= 54,
	HEADER_CG_STATE_CHECKER					= 55,
	HEADER_CG_LOGIN2						= 56,
	HEADER_CG_LOGIN3						= 57,

#ifdef ENABLE_DROP_DIALOG_SYSTEM
	HEADER_CG_ITEM_DELETE					= PACKET_CG_START + 0,
	HEADER_CG_ITEM_SELL						= PACKET_CG_START + 1,
#endif

#ifdef ENABLE_SKILL_CHOOSE_SYSTEM
	HEADER_CG_SKILLCHOOSE					= PACKET_CG_START + 2,
#endif
#ifdef ENABLE_GLOBAL_MESSAGE_UTILITY
	HEADER_CG_BULK_WHISPER					= PACKET_CG_START + 3,
#endif
#ifdef ENABLE_CHEST_INFO_SYSTEM
	HEADER_CG_CHEST_DROP_INFO				= PACKET_CG_START + 4,
#endif
#ifdef ENABLE_OFFLINE_SHOP
	HEADER_CG_NEW_OFFLINESHOP				= PACKET_CG_START + 5,
#endif
#ifdef ENABLE_ANCIENT_ATTR_SYSTEM
	HEADER_CG_ITEM_USE_NEW_ATTRIBUTE		= PACKET_CG_START + 6,
#endif
#ifdef ENABLE_CHANGELOOK_SYSTEM
	HEADER_CG_CL							= PACKET_CG_START + 7,
#endif
#ifdef ENABLE_SKILL_COLOR_SYSTEM
	HEADER_CG_SKILL_COLOR					= PACKET_CG_START + 8,
#endif
#ifdef ENABLE_DUNGEON_INFO_SYSTEM
	HEADER_CG_TELEPORT						= PACKET_CG_START + 9,
#endif
#ifdef ENABLE_SWITCHBOT_SYSTEM
	HEADER_CG_SWITCHBOT						= PACKET_CG_START + 10,
#endif
#ifdef ENABLE_CUBE_RENEWAL
	HEADER_CG_CUBE_RENEWAL					= PACKET_CG_START + 11,
#endif
#ifdef ENABLE_GROWTH_PET_SYSTEM
	HEADER_CG_PetSetName					= PACKET_CG_START + 12,
#endif
#ifdef ENABLE_FAST_CHEQUE_TRANSFER
	HEADER_CG_WON_EXCHANGE					= PACKET_CG_START + 13,
#endif
#ifdef ENABLE_ACCE_SYSTEM
	HEADER_CG_ACCE							= PACKET_CG_START + 14,
#endif
#ifdef ENABLE_TARGET_BOARD_RENEWAL
	HEADER_CG_MOB_INFO						= PACKET_CG_START + 15,
#endif
#ifdef ENABLE_GROWTH_MOUNT_SYSTEM
	HEADER_CG_MountSetName					= PACKET_CG_START + 16,
#endif
#ifdef ENABLE_BATTLE_PASS_SYSTEM
	HEADER_CG_BATTLE_PASS					= PACKET_CG_START + 17,
#endif
	HEADER_CG_KEY_AGREEMENT					= 0xfb, // ENABLE_IMPROVED_PACKET_ENCRYPTION
};

enum EPacketsGameToClient
{
	HEADER_GC_HANDSHAKE_OK						= 0xfc,
	HEADER_GC_PHASE								= 0xfd,
	HEADER_GC_HANDSHAKE							= 0xff,

	HEADER_GC_CHARACTER_ADD						= 1,
	HEADER_GC_CHARACTER_DEL						= 2,
	HEADER_GC_CHARACTER_MOVE					= 3,
	HEADER_GC_CHAT								= 4,
	HEADER_GC_SYNC_POSITION						= 5,
	HEADER_GC_LOGIN_SUCCESS4					= 6,
	HEADER_GC_LOGIN_FAILURE						= 7,
	HEADER_GC_PLAYER_CREATE_SUCCESS				= 8,
	HEADER_GC_PLAYER_CREATE_FAILURE				= 9,
	HEADER_GC_PLAYER_DELETE_SUCCESS				= 10,
	HEADER_GC_PLAYER_DELETE_WRONG_SOCIAL_ID		= 11,
	HEADER_GC_STUN								= 12,
	HEADER_GC_DEAD								= 13,
	HEADER_GC_PLAYER_POINTS						= 14,
	HEADER_GC_PLAYER_POINT_CHANGE				= 15,
	HEADER_GC_CHARACTER_UPDATE					= 16,
	HEADER_GC_ITEM_DEL = 17,
	HEADER_GC_ITEM_SET = 18,
	HEADER_GC_ITEM_USE							= 19,
	HEADER_GC_ITEM_UPDATE						= 20,
	HEADER_GC_ITEM_GROUND_ADD					= 21,
	HEADER_GC_ITEM_GROUND_DEL					= 22,
	HEADER_GC_QUICKSLOT_ADD						= 23,
	HEADER_GC_QUICKSLOT_DEL						= 24,
	HEADER_GC_QUICKSLOT_SWAP					= 25,
	HEADER_GC_ITEM_OWNERSHIP					= 26,
	HEADER_GC_WHISPER							= 27,
	HEADER_GC_MOTION							= 28,
	HEADER_GC_SHOP								= 29,
	HEADER_GC_SHOP_SIGN							= 30,
	HEADER_GC_PVP								= 31,
	HEADER_GC_EXCHANGE							= 32,
	HEADER_GC_CHARACTER_POSITION				= 33,

	HEADER_GC_SCRIPT							= 34,
	HEADER_GC_QUEST_CONFIRM						= 35,
	HEADER_GC_OWNERSHIP							= 36,
	HEADER_GC_TARGET							= 37,
	HEADER_GC_WARP								= 38,
	HEADER_GC_ADD_FLY_TARGETING					= 39,
	HEADER_GC_CREATE_FLY						= 40,
	HEADER_GC_FLY_TARGETING						= 41,
	HEADER_GC_MESSENGER							= 42,
	HEADER_GC_GUILD								= 43,
	HEADER_GC_SKILL_LEVEL_NEW					= 44,
	HEADER_GC_PARTY_INVITE						= 45,
	HEADER_GC_PARTY_ADD							= 46,
	HEADER_GC_PARTY_UPDATE						= 47,
	HEADER_GC_PARTY_REMOVE						= 48,
	HEADER_GC_QUEST_INFO						= 49,
	HEADER_GC_REQUEST_MAKE_GUILD				= 50,
	HEADER_GC_PARTY_PARAMETER					= 51,
	HEADER_GC_SAFEBOX_SET						= 52,
	HEADER_GC_SAFEBOX_DEL						= 53,
	HEADER_GC_SAFEBOX_WRONG_PASSWORD			= 54,
	HEADER_GC_SAFEBOX_SIZE						= 55,
	HEADER_GC_FISHING							= 56,
	HEADER_GC_EMPIRE							= 57,
	HEADER_GC_PARTY_LINK						= 58,
	HEADER_GC_PARTY_UNLINK						= 59,
	HEADER_GC_MARK_BLOCK						= 60,
	HEADER_GC_MARK_IDXLIST						= 61,
	HEADER_GC_TIME								= 62,
	HEADER_GC_CHANGE_NAME						= 63,
	HEADER_GC_DUNGEON							= 64,
	HEADER_GC_WALK_MODE							= 65,
	HEADER_GC_CHANGE_SKILL_GROUP				= 66,
	HEADER_GC_MAIN_CHARACTER					= 67,
	HEADER_GC_SEPCIAL_EFFECT					= 68,
	HEADER_GC_REFINE_INFORMATION_NEW			= 69,
	HEADER_GC_CHANNEL							= 70,
	HEADER_GC_MALL_OPEN							= 71,
	HEADER_GC_TARGET_UPDATE						= 72,
	HEADER_GC_TARGET_DELETE						= 73,
	HEADER_GC_TARGET_CREATE_NEW					= 74,
	HEADER_GC_AFFECT_ADD						= 75,
	HEADER_GC_AFFECT_REMOVE						= 76,
	HEADER_GC_MALL_SET							= 77,
	HEADER_GC_MALL_DEL							= 78,
	HEADER_GC_DIG_MOTION						= 79,
	HEADER_GC_DAMAGE_INFO						= 80,
	HEADER_GC_CHAR_ADDITIONAL_INFO				= 81,
	HEADER_GC_AUTH_SUCCESS						= 82,
	HEADER_GC_SPECIFIC_EFFECT					= 83,
	HEADER_GC_DRAGON_SOUL_REFINE				= 84,
	HEADER_GC_RESPOND_CHANNELSTATUS				= 85,
	HEADER_GC_PING								= 86,

#ifdef ENABLE_CHEST_INFO_SYSTEM
	HEADER_GC_CHEST_DROP_INFO					= PACKET_GC_START + 1,
#endif
#ifdef ENABLE_GLOBAL_MESSAGE_UTILITY
	HEADER_GC_BULK_WHISPER						= PACKET_GC_START + 2,
#endif
#ifdef ENABLE_EVENT_SYSTEM
	HEADER_GC_EVENT_INFO						= PACKET_GC_START + 3,
#endif
#ifdef ENABLE_SKILL_CHOOSE_SYSTEM
	HEADER_GC_SKILLCHOOSE						= PACKET_GC_START + 4,
#endif
#ifdef ENABLE_CHANGELOOK_SYSTEM
	HEADER_GC_CL								= PACKET_GC_START + 5,
#endif
#ifdef ENABLE_SWITCHBOT_SYSTEM
	HEADER_GC_SWITCHBOT							= PACKET_GC_START + 6,
#endif
#ifdef ENABLE_CUBE_RENEWAL
	HEADER_GC_CUBE_RENEWAL						= PACKET_GC_START + 7,
#endif
#ifdef ENABLE_OFFLINE_SHOP
	HEADER_GC_NEW_OFFLINESHOP					= PACKET_GC_START + 8,
#endif
#ifdef ENABLE_ACCE_SYSTEM
	HEADER_GC_ACCE								= PACKET_GC_START + 9,
#endif
#ifdef ENABLE_PLAYER_STATISTICS
	HEADER_GC_PLAYER_STATISTICS					= PACKET_GC_START + 10,
#endif
#ifdef ENABLE_TARGET_BOARD_RENEWAL
	HEADER_GC_MOB_INFO							= PACKET_GC_START + 12,
#endif
#ifdef ENABLE_INGAME_MALL_SYSTEM
	HEADER_GC_ITEM_SHOP							= PACKET_GC_START + 13,
#endif // ENABLE_INGAME_MALL_SYSTEM
#ifdef ENABLE_BATTLE_PASS_SYSTEM
	HEADER_GC_BATTLE_PASS_OPEN					= PACKET_GC_START + 14,
	HEADER_GC_BATTLE_PASS_UPDATE				= PACKET_GC_START + 15,
	HEADER_GC_BATTLE_PASS_RANKING				= PACKET_GC_START + 16,
#endif

	HEADER_GC_UNK_213							= 213,

	HEADER_GC_KEY_AGREEMENT_COMPLETED			= 0xfa, // ENABLE_IMPROVED_PACKET_ENCRYPTION
	HEADER_GC_KEY_AGREEMENT						= 0xfb, // ENABLE_IMPROVED_PACKET_ENCRYPTION
};

enum
{
	ID_MAX_NUM = 30,
	PASS_MAX_NUM = 16,
	CHAT_MAX_NUM = 128,
	SHOP_SIGN_MAX_LEN = 32,
#ifndef ENABLE_PLAYER_PER_ACCOUNT5
	PLAYER_PER_ACCOUNT4 = 4,
#else
	PLAYER_PER_ACCOUNT4 = 5,
	PLAYER_PER_ACCOUNT5 = 5,
#endif
	QUICKSLOT_MAX_LINE = 4,
	QUICKSLOT_MAX_COUNT_PER_LINE = 8,
	QUICKSLOT_MAX_COUNT = QUICKSLOT_MAX_LINE * QUICKSLOT_MAX_COUNT_PER_LINE,
	QUICKSLOT_MAX_NUM = 36,
	SHOP_HOST_ITEM_MAX_NUM = 45,
	METIN_SOCKET_COUNT = 6,
	PARTY_AFFECT_SLOT_MAX_NUM = 10,
	GUILD_GRADE_NAME_MAX_LEN = 8,
	GUILD_NAME_MAX_LEN = 12,
	GUILD_GRADE_COUNT = 15,
	GULID_COMMENT_MAX_LEN = 50,
	MARK_CRC_NUM = 8 * 8,
	MARK_DATA_SIZE = 16 * 12,
	QUEST_INPUT_STRING_MAX_NUM = 64,
	PRIVATE_CODE_LENGTH = 8,
	REFINE_MATERIAL_MAX_NUM = 10,
	WEAR_MAX_NUM = CItemData::WEAR_MAX_NUM,
#ifdef ENABLE_HWID_SECURITY_UTILITY
	CPU_ID_MAX_NUM = 96,
	HDD_MODEL_MAX_NUM = 96,
	MACHINE_GUID_MAX_NUM = 96,
	MAC_ADDR_MAX_NUM = 96,
	HDD_SERIAL_MAX_NUM = 96,
	BIOS_ID_MAX_NUM = 96,
	PC_NAME_MAX_NUM = 96,
	USER_NAME_MAX_NUM = 96,
	OS_VERSION_MAX_NUM = 96,
#endif
#ifdef ENABLE_BAN_REASON_UTILITY
	BAN_WEB_LINK_MAX_LEN = 128,
#endif
#ifdef USE_DISCORD_NETWORK
	DISCORD_ADDRESS_MAX_NUM = 38, // (32 + 1 + 4 + [1])
#endif
};

#pragma pack(push)
#pragma pack(1)

typedef struct command_mark_login
{
	BYTE    header;
	DWORD   handle;
	DWORD   random_key;
} TPacketCGMarkLogin;

typedef struct command_mark_upload
{
	BYTE    header;
	DWORD   gid;
	BYTE    image[16 * 12 * 4];
} TPacketCGMarkUpload;

typedef struct command_mark_idxlist
{
	BYTE    header;
} TPacketCGMarkIDXList;

typedef struct command_mark_crclist
{
	BYTE    header;
	BYTE    imgIdx;
	DWORD   crclist[80];
} TPacketCGMarkCRCList;

typedef struct packet_mark_idxlist
{
	BYTE    header;
	DWORD	bufSize;
	WORD    count;
} TPacketGCMarkIDXList;

typedef struct packet_mark_block
{
	BYTE    header;
	DWORD   bufSize;
	BYTE	imgIdx;
	DWORD   count;
} TPacketGCMarkBlock;

typedef struct command_login
{
	BYTE header;
	char name[ID_MAX_NUM + 1];
	char pwd[PASS_MAX_NUM + 1];
} TPacketCGLogin;

typedef struct command_login2
{
	BYTE	header;
	char	name[ID_MAX_NUM + 1];
	DWORD	login_key;
	DWORD	adwClientKey[4];
} TPacketCGLogin2;

typedef struct command_login3
{
	BYTE	header;
	char	name[ID_MAX_NUM + 1];
	char	pwd[PASS_MAX_NUM + 1];
	DWORD	adwClientKey[4];
#ifdef ENABLE_CLIENT_VERSION_UTILITY
	char	clientversion[60 + 1];
#endif
#ifdef ENABLE_PIN_SYSTEM
	char	pin[PASS_MAX_NUM + 1];
#endif
#ifdef ENABLE_HWID_SECURITY_UTILITY
	char	cpu_id[CPU_ID_MAX_NUM + 1];
	char	hdd_model[HDD_MODEL_MAX_NUM + 1];
	char	machine_guid[MACHINE_GUID_MAX_NUM + 1];
	char	mac_addr[MAC_ADDR_MAX_NUM + 1];
	char	hdd_serial[HDD_SERIAL_MAX_NUM + 1];
	char	bios_id[BIOS_ID_MAX_NUM + 1];
	char	pc_name[PC_NAME_MAX_NUM + 1];
	char	user_name[USER_NAME_MAX_NUM + 1];
	char	os_version[OS_VERSION_MAX_NUM + 1];
	int		banned_val;
#endif
#ifdef USE_DISCORD_NETWORK
	char	discordAddress[DISCORD_ADDRESS_MAX_NUM + 1];
#endif
} TPacketCGLogin3;

typedef struct command_player_select
{
	BYTE	header;
	BYTE	player_index;
} TPacketCGSelectCharacter;

typedef struct command_attack
{
	BYTE	header;
	BYTE	bType;
	DWORD	dwVictimVID;
} TPacketCGAttack;

typedef struct command_chat
{
	BYTE	header;
	WORD	length;
	BYTE	type;
} TPacketCGChat;

typedef struct command_whisper
{
	BYTE        bHeader;
	WORD        wSize;
	char        szNameTo[CHARACTER_NAME_MAX_LEN + 1];
} TPacketCGWhisper;

typedef struct command_EnterFrontGame
{
	BYTE header;
} TPacketCGEnterFrontGame;

typedef struct command_item_use
{
	BYTE header;
	TItemPos pos;
#ifdef ENABLE_USE_ITEM_COUNT
	DWORD count;
#endif // ENABLE_USE_ITEM_COUNT
} TPacketCGItemUse;

typedef struct command_item_use_to_item
{
	BYTE header;
	TItemPos source_pos;
	TItemPos target_pos;
} TPacketCGItemUseToItem;

#ifdef ENABLE_DROP_DIALOG_SYSTEM
typedef struct command_item_delete
{
	BYTE		header;
	TItemPos	item_pos;
} TPacketCGItemDelete;
typedef struct command_item_sell
{
	BYTE		header;
	TItemPos	item_pos;
} TPacketCGItemSell;
#endif

typedef struct command_item_move
{
	BYTE header;
	TItemPos pos;
	TItemPos change_pos;
#ifdef ENABLE_ITEM_COUNT_LIMIT
	DWORD num;
#else
	BYTE num;
#endif
} TPacketCGItemMove;

typedef struct command_item_pickup
{
	BYTE header;
	DWORD vid;
} TPacketCGItemPickUp;

typedef struct command_quickslot_add
{
	BYTE        header;
	BYTE        pos;
	TQuickSlot	slot;
}TPacketCGQuickSlotAdd;

typedef struct command_quickslot_del
{
	BYTE        header;
	BYTE        pos;
}TPacketCGQuickSlotDel;

typedef struct command_quickslot_swap
{
	BYTE        header;
	BYTE        pos;
	BYTE        change_pos;
}TPacketCGQuickSlotSwap;

typedef struct command_on_click
{
	BYTE		header;
	DWORD		vid;
} TPacketCGOnClick;

enum
{
	SHOP_SUBHEADER_CG_END,
	SHOP_SUBHEADER_CG_BUY,
	SHOP_SUBHEADER_CG_SELL,
	SHOP_SUBHEADER_CG_SELL2,
};

typedef struct command_shop
{
	BYTE        header;
	BYTE		subheader;
} TPacketCGShop;

enum
{
	EXCHANGE_SUBHEADER_CG_START,			// arg1 == vid of target character
	EXCHANGE_SUBHEADER_CG_ITEM_ADD,		// arg1 == position of item
	EXCHANGE_SUBHEADER_CG_ITEM_DEL,		// arg1 == position of item
	EXCHANGE_SUBHEADER_CG_ELK_ADD,			// arg1 == amount of elk
#ifdef ENABLE_CHEQUE_SYSTEM
	EXCHANGE_SUBHEADER_CG_CHEQUE_ADD,		// arg1 == amount of cheque
#endif
	EXCHANGE_SUBHEADER_CG_ACCEPT,			// arg1 == not used
	EXCHANGE_SUBHEADER_CG_CANCEL,			// arg1 == not used
};

typedef struct command_exchange
{
	BYTE		header;
	BYTE		subheader;
#ifdef ENABLE_GOLD_LIMIT_REWORK
	long long	arg1;
#else
	DWORD		arg1;
#endif
	BYTE		arg2;
	TItemPos	Pos;
} TPacketCGExchange;

typedef struct command_position
{
	BYTE        header;
	BYTE        position;
} TPacketCGPosition;

typedef struct command_script_answer
{
	BYTE        header;
	BYTE		answer;
} TPacketCGScriptAnswer;

typedef struct command_script_button
{
	BYTE        header;
	unsigned int			idx;
} TPacketCGScriptButton;

typedef struct command_target
{
	BYTE        header;
	DWORD       dwVID;
} TPacketCGTarget;

typedef struct command_move
{
	BYTE		bHeader;
	BYTE		bFunc;
	BYTE		bArg;
	BYTE		bRot;
	LONG		lX;
	LONG		lY;
	DWORD		dwTime;
} TPacketCGMove;

typedef struct command_sync_position_element
{
	DWORD       dwVID;
	long        lX;
	long        lY;
} TPacketCGSyncPositionElement;

typedef struct command_sync_position
{
	BYTE        bHeader;
	WORD		wSize;
} TPacketCGSyncPosition;

typedef struct command_fly_targeting
{
	BYTE		bHeader;
	DWORD		dwTargetVID;
	long		lX;
	long		lY;
} TPacketCGFlyTargeting;

typedef struct packet_fly_targeting
{
	BYTE        bHeader;
	DWORD		dwShooterVID;
	DWORD		dwTargetVID;
	long		lX;
	long		lY;
} TPacketGCFlyTargeting;

typedef struct packet_shoot
{
	BYTE		bHeader;
	BYTE		bType;
} TPacketCGShoot;

typedef struct command_warp
{
	BYTE			bHeader;
} TPacketCGWarp;

enum
{
	MESSENGER_SUBHEADER_GC_LIST,
	MESSENGER_SUBHEADER_GC_LOGIN,
	MESSENGER_SUBHEADER_GC_LOGOUT,
	MESSENGER_SUBHEADER_GC_REMOVE_FRIEND, // @duzenleme messengerden adam silindiginde paket geliyor.
	MESSENGER_SUBHEADER_GC_INVITE,
};

typedef struct packet_messenger
{
	BYTE header;
	WORD size;
	BYTE subheader;
} TPacketGCMessenger;

typedef struct packet_messenger_list_offline
{
	BYTE connected; // always 0
	BYTE length;
} TPacketGCMessengerListOffline;

enum
{
	MESSENGER_CONNECTED_STATE_OFFLINE,
	MESSENGER_CONNECTED_STATE_ONLINE,
};

typedef struct packet_messenger_list_online
{
	BYTE connected;
	BYTE length;
	//BYTE length_char_name;
} TPacketGCMessengerListOnline;

typedef struct packet_messenger_login
{
	//BYTE length_login;
	//BYTE length_char_name;
	BYTE length;
} TPacketGCMessengerLogin;

typedef struct packet_messenger_logout
{
	BYTE length;
} TPacketGCMessengerLogout;

enum
{
	MESSENGER_SUBHEADER_CG_ADD_BY_VID,
	MESSENGER_SUBHEADER_CG_ADD_BY_NAME,
	MESSENGER_SUBHEADER_CG_REMOVE,
};

typedef struct command_messenger
{
	BYTE header;
	BYTE subheader;
} TPacketCGMessenger;

typedef struct command_messenger_remove
{
	BYTE length;
} TPacketCGMessengerRemove;

typedef struct command_safebox_checkout
{
	BYTE        bHeader;
	BYTE        bSafePos;
	TItemPos	ItemPos;
} TPacketCGSafeboxCheckout;

typedef struct command_safebox_checkin
{
	BYTE        bHeader;
	BYTE        bSafePos;
	TItemPos	ItemPos;
} TPacketCGSafeboxCheckin;

typedef struct command_mall_checkout
{
	BYTE        bHeader;
	BYTE        bMallPos;
	TItemPos	ItemPos;
} TPacketCGMallCheckout;

typedef struct command_use_skill
{
	BYTE                bHeader;
	DWORD               dwVnum;
	DWORD				dwTargetVID;
} TPacketCGUseSkill;

typedef struct command_party_invite
{
	BYTE header;
	DWORD vid;
} TPacketCGPartyInvite;

typedef struct command_party_invite_answer
{
	BYTE header;
	DWORD leader_pid;
	BYTE accept;
} TPacketCGPartyInviteAnswer;

typedef struct command_party_remove
{
	BYTE header;
	DWORD pid;
} TPacketCGPartyRemove;

typedef struct command_party_set_state
{
	BYTE byHeader;
	DWORD dwVID;
	BYTE byState;
	BYTE byFlag;
} TPacketCGPartySetState;

typedef struct packet_party_link
{
	BYTE header;
	DWORD pid;
	DWORD vid;
} TPacketGCPartyLink;

typedef struct packet_party_unlink
{
	BYTE header;
	DWORD pid;
	DWORD vid;
} TPacketGCPartyUnlink;

enum
{
	GUILD_SUBHEADER_CG_ADD_MEMBER,
	GUILD_SUBHEADER_CG_REMOVE_MEMBER,
	GUILD_SUBHEADER_CG_CHANGE_GRADE_NAME,
	GUILD_SUBHEADER_CG_CHANGE_GRADE_AUTHORITY,
	GUILD_SUBHEADER_CG_OFFER,
	GUILD_SUBHEADER_CG_POST_COMMENT,
	GUILD_SUBHEADER_CG_DELETE_COMMENT,
	GUILD_SUBHEADER_CG_REFRESH_COMMENT,
	GUILD_SUBHEADER_CG_CHANGE_MEMBER_GRADE,
	GUILD_SUBHEADER_CG_USE_SKILL,
	GUILD_SUBHEADER_CG_CHANGE_MEMBER_GENERAL,
	GUILD_SUBHEADER_CG_GUILD_INVITE_ANSWER,
	GUILD_SUBHEADER_CG_CHARGE_GSP,
	GUILD_SUBHEADER_CG_DEPOSIT_MONEY,
	GUILD_SUBHEADER_CG_WITHDRAW_MONEY,
};

typedef struct command_guild
{
	BYTE byHeader;
	BYTE bySubHeader;
} TPacketCGGuild;

typedef struct command_guild_answer_make_guild
{
	BYTE header;
	char guild_name[GUILD_NAME_MAX_LEN + 1];
} TPacketCGAnswerMakeGuild;

typedef struct command_give_item
{
	BYTE byHeader;
	DWORD dwTargetVID;
	TItemPos ItemPos;
#ifdef ENABLE_ITEM_COUNT_LIMIT
	DWORD byItemCount;
#else
	BYTE byItemCount;
#endif
} TPacketCGGiveItem;

typedef struct SPacketCGHack
{
	BYTE        bHeader;
	char        szBuf[255 + 1];
} TPacketCGHack;

typedef struct command_dungeon
{
	BYTE		bHeader;
	WORD		size;
} TPacketCGDungeon;

// Private Shop
typedef struct SShopItemTable
{
	DWORD		vnum;
#ifdef ENABLE_ITEM_COUNT_LIMIT
	DWORD		count;
#else
	BYTE		count;
#endif

	TItemPos	pos;
#ifdef ENABLE_GOLD_LIMIT_REWORK
	long long	price;
#else
	DWORD		price;
#endif
#ifdef ENABLE_CHEQUE_SYSTEM
	DWORD		cheque;
#endif
	BYTE		display_pos;
#ifdef ENABLE_SHOP_PRICE_TYPE_ITEM
	DWORD		witemVnum;
#endif
} TShopItemTable;

typedef struct SPacketCGMyShop
{
	BYTE        bHeader;
	char        szSign[SHOP_SIGN_MAX_LEN + 1];
	BYTE        bCount;	// count of TShopItemTable, max 39
} TPacketCGMyShop;

typedef struct SPacketCGRefine
{
	BYTE		header;
	BYTE		pos;
	BYTE		type;
} TPacketCGRefine;

typedef struct SPacketCGChangeName
{
	BYTE header;
	BYTE index;
	char name[CHARACTER_NAME_MAX_LEN + 1];
} TPacketCGChangeName;

enum EPartyExpDistributionType
{
	PARTY_EXP_DISTRIBUTION_NON_PARITY,
	PARTY_EXP_DISTRIBUTION_PARITY,
};

typedef struct command_party_parameter
{
	BYTE        bHeader;
	BYTE        bDistributeMode;
} TPacketCGPartyParameter;

typedef struct command_quest_input_string
{
	BYTE        bHeader;
	char		szString[QUEST_INPUT_STRING_MAX_NUM + 1];
} TPacketCGQuestInputString;

typedef struct command_quest_confirm
{
	BYTE header;
	BYTE answer;
	DWORD requestPID;
} TPacketCGQuestConfirm;

typedef struct command_script_select_item
{
	BYTE header;
	DWORD selection;
} TPacketCGScriptSelectItem;

enum EPhase
{
	PHASE_CLOSE,
	PHASE_HANDSHAKE,
	PHASE_LOGIN,
	PHASE_SELECT,
	PHASE_LOADING,
	PHASE_GAME,
	PHASE_DEAD,

	PHASE_DBCLIENT_CONNECTING,
	PHASE_DBCLIENT,
	PHASE_P2P,
	PHASE_AUTH,
};

typedef struct packet_phase
{
	BYTE        header;
	BYTE        phase;
} TPacketGCPhase;

typedef struct packet_blank
{
	BYTE		header;
} TPacketGCBlank;

typedef struct packet_blank_dynamic
{
	BYTE		header;
	WORD		size;
} TPacketGCBlankDynamic;

typedef struct packet_header_handshake
{
	BYTE		header;
	DWORD		dwHandshake;
	DWORD		dwTime;
	LONG		lDelta;
} TPacketGCHandshake;

typedef struct packet_header_dynamic_size
{
	BYTE		header;
	WORD		size;
} TDynamicSizePacketHeader;

#ifdef ENABLE_LARGE_DYNAMIC_PACKET
typedef struct packet_header_large_dynamic_size
{
	BYTE		header;
	int			size;
} TLargeDynamicSizePacketHeader;
#endif

#ifdef ENABLE_SHINING_ITEM_SYSTEM
enum EShiningParts
{
	CHR_SHINING_WEAPON,
	CHR_SHINING_ARMOR,
	CHR_SHINING_SPECIAL,
	CHR_SHINING_SPECIAL2,
	CHR_SHINING_SPECIAL3,
	CHR_SHINING_WING,
	CHR_SHINING_NUM,
};
#endif

typedef struct SSimplePlayerInformation
{
	DWORD               dwID;
	char                szName[CHARACTER_NAME_MAX_LEN + 1];
	BYTE                byJob;
	BYTE                byLevel;
	DWORD               dwPlayMinutes;
#ifdef ENABLE_CHARACTER_WINDOW_RENEWAL
	DWORD				dwLastPlayTime;
#endif
	BYTE                byST, byHT, byDX, byIQ;
	//	WORD				wParts[CRaceData::PART_MAX_NUM];
	DWORD                wMainPart;
	BYTE                bChangeName;
	DWORD				wHairPart;
#ifdef ENABLE_ACCE_SYSTEM
	DWORD				wAccePart;
#endif
	BYTE                bDummy[5];
	long				x, y;
	LONG				lAddr;
	WORD				wPort;
	BYTE				bySkillGroup;
} TSimplePlayerInformation;

typedef struct packet_login_success4
{
	BYTE						header;
	TSimplePlayerInformation	akSimplePlayerInformation[PLAYER_PER_ACCOUNT4];
	DWORD						guild_id[PLAYER_PER_ACCOUNT4];
	char						guild_name[PLAYER_PER_ACCOUNT4][GUILD_NAME_MAX_LEN + 1];
	DWORD handle;
	DWORD random_key;
} TPacketGCLoginSuccess4;

#ifdef ENABLE_BAN_REASON_UTILITY
enum { LOGIN_STATUS_MAX_LEN = 24 };
#else
enum { LOGIN_STATUS_MAX_LEN = 8 };
#endif
typedef struct packet_login_failure
{
	BYTE	header;
	char	szStatus[LOGIN_STATUS_MAX_LEN + 1];
#ifdef ENABLE_BAN_REASON_UTILITY
	DWORD	availDate;
	char	szBanWebLink[BAN_WEB_LINK_MAX_LEN + 1];
#endif
} TPacketGCLoginFailure;

typedef struct command_player_create
{
	BYTE        header;
	BYTE        index;
	char        name[CHARACTER_NAME_MAX_LEN + 1];
	WORD        job;
	BYTE		shape;
} TPacketCGCreateCharacter;

typedef struct command_player_create_success
{
	BYTE						header;
	BYTE						bAccountCharacterSlot;
	TSimplePlayerInformation	kSimplePlayerInfomation;
} TPacketGCPlayerCreateSuccess;

typedef struct command_create_failure
{
	BYTE	header;
	BYTE	bType;
} TPacketGCCreateFailure;

typedef struct command_player_delete
{
	BYTE        header;
	BYTE        index;
	char		szPrivateCode[PRIVATE_CODE_LENGTH];
} TPacketCGDestroyCharacter;

typedef struct packet_player_delete_success
{
	BYTE        header;
	BYTE        account_index;
} TPacketGCDestroyCharacterSuccess;

enum
{
	ADD_CHARACTER_STATE_DEAD = (1 << 0),
	ADD_CHARACTER_STATE_SPAWN = (1 << 1),
	ADD_CHARACTER_STATE_GUNGON = (1 << 2),
	ADD_CHARACTER_STATE_KILLER = (1 << 3),
	ADD_CHARACTER_STATE_PARTY = (1 << 4),
};

enum EPKModes
{
	PK_MODE_PEACE,
	PK_MODE_REVENGE,
	PK_MODE_FREE,
	PK_MODE_PROTECT,
	PK_MODE_GUILD,
	PK_MODE_MAX_NUM,
};

enum ECharacterEquipmentPart
{
	CHR_EQUIPPART_ARMOR,
	CHR_EQUIPPART_WEAPON,
	CHR_EQUIPPART_HEAD,
	CHR_EQUIPPART_HAIR,
#ifdef ENABLE_ACCE_SYSTEM
	CHR_EQUIPPART_ACCE,
	CHR_EQUIPPART_ACCE_EFFECT,
#endif
#ifdef ENABLE_AURA_COSTUME_SYSTEM
	CHR_EQUIPPART_AURA,
#endif
#ifdef ENABLE_QUIVER_SYSTEM
	CHR_EQUIPPART_ARROW,
#endif
	CHR_EQUIPPART_NUM,
};

#ifdef ENABLE_ITEM_EVOLUTION_SYSTEM
enum EItemEvolution
{
	EVOLUTION_WEAPON,
#ifdef ENABLE_ARMOR_EVOLUTION_SYSTEM
	EVOLUTION_ARMOR,
#endif
	EVOLUTION_MAX,
};
#endif

typedef struct packet_char_additional_info
{
	BYTE	header;
	DWORD	vid;
	char	name[CHARACTER_NAME_MAX_LEN + 1];
	DWORD	parts[CHR_EQUIPPART_NUM];
	BYTE	bEmpire;
	DWORD	dwGuildID;
	BYTE	bLevel;
	BYTE	bPKMode;
	DWORD	dwMountVnum;
#ifdef ENABLE_ALIGNMENT_SYSTEM
	int		alignment;
#else
	short	alignment;
#endif
#ifdef ENABLE_RANK_SYSTEM
	short	rank;
#endif
#ifdef ENABLE_LANDRANK_SYSTEM
	short	land_rank;
#endif
#ifdef ENABLE_REBORN_SYSTEM
	short	reborn;
#endif
#ifdef ENABLE_TEAM_SYSTEM
	short	team;
#endif
#ifdef ENABLE_MONIKER_SYSTEM
	char m_Moniker[MONIKER_MAX_LEN + 1];
#endif
#ifdef ENABLE_LOVE_SYSTEM
	char m_Love1[2];
	char m_Love2[2];
#endif // ENABLE_LOVE_SYSTEM
#ifdef ENABLE_WORD_SYSTEM
	char m_word[24+1];
#endif // ENABLE_WORD_SYSTEM

#ifdef ENABLE_GUILD_LEADER_SYSTEM
	BYTE	guild_member_type;
#endif
#ifdef ENABLE_ITEM_EVOLUTION_SYSTEM
	WORD	itemEvolution[EVOLUTION_MAX];
#endif
#ifdef ENABLE_SKILL_COLOR_SYSTEM
	DWORD	dwSkillColor[ESkillColorLength::MAX_SKILL_COUNT + ESkillColorLength::MAX_BUFF_COUNT][ESkillColorLength::MAX_EFFECT_COUNT];
#endif
#ifdef ENABLE_SHINING_ITEM_SYSTEM
	DWORD	adwShining[CHR_SHINING_NUM];
#endif
} TPacketGCCharacterAdditionalInfo;

typedef struct packet_add_char
{
	BYTE	header;
	DWORD	dwVID;
#if defined(ENABLE_MOB_AGGR_LVL_INFO)
	DWORD	dwLevel;
	DWORD	dwAIFlag;
#endif
	float	angle;
	long	x;
	long	y;
	long	z;
	BYTE	bType;
	WORD	wRaceNum;
	WORD	bMovingSpeed;
	WORD	bAttackSpeed;
	BYTE	bStateFlag;
	DWORD	dwAffectFlag[2];
} TPacketGCCharacterAdd;

typedef struct packet_update_char
{
	BYTE	header;
	DWORD	vid;
	DWORD	parts[CHR_EQUIPPART_NUM];
	WORD	wMovingSpeed;
	WORD	wAttackSpeed;
	BYTE	bStateFlag;
	DWORD	dwAffectFlag[2];
	DWORD	dwGuildID;
	BYTE	bPKMode;
	DWORD	dwMountVnum;
	BYTE	bLevel;
#ifdef ENABLE_ALIGNMENT_SYSTEM
	int		alignment;
#else
	short	alignment;
#endif
#ifdef ENABLE_RANK_SYSTEM
	short	rank;
#endif
#ifdef ENABLE_LANDRANK_SYSTEM
	short	land_rank;
#endif
#ifdef ENABLE_REBORN_SYSTEM
	short	reborn;
#endif
#ifdef ENABLE_TEAM_SYSTEM
	short	team;
#endif
#ifdef ENABLE_MONIKER_SYSTEM
	char m_Moniker[MONIKER_MAX_LEN];
#endif
#ifdef ENABLE_LOVE_SYSTEM
	char m_Love1[2];
	char m_Love2[2];
#endif // ENABLE_LOVE_SYSTEM
#ifdef ENABLE_WORD_SYSTEM
	char m_word[24+1];
#endif // ENABLE_WORD_SYSTEM

#ifdef ENABLE_GUILD_LEADER_SYSTEM
	BYTE	guild_member_type;
#endif
#ifdef ENABLE_ITEM_EVOLUTION_SYSTEM
	WORD	itemEvolution[EVOLUTION_MAX];
#endif
#ifdef ENABLE_SKILL_COLOR_SYSTEM
	DWORD	dwSkillColor[ESkillColorLength::MAX_SKILL_COUNT + ESkillColorLength::MAX_BUFF_COUNT][ESkillColorLength::MAX_EFFECT_COUNT];
#endif
#ifdef ENABLE_SHINING_ITEM_SYSTEM
	DWORD	adwShining[CHR_SHINING_NUM];
#endif
#ifdef ENABLE_AUTO_HUNT_SYSTEM
	DWORD	hasAttacker;
#endif // ENABLE_AUTO_HUNT_SYSTEM
} TPacketGCCharacterUpdate;

typedef struct packet_del_char
{
	BYTE	header;
	DWORD	dwVID;
} TPacketGCCharacterDelete;

enum EChatType
{
	CHAT_TYPE_TALKING,
	CHAT_TYPE_INFO,
	CHAT_TYPE_NOTICE,
	CHAT_TYPE_PARTY,
	CHAT_TYPE_GUILD,
	CHAT_TYPE_COMMAND,
	CHAT_TYPE_SHOUT,
	CHAT_TYPE_WHISPER,
	CHAT_TYPE_BIG_NOTICE,
#ifdef ENABLE_DICE_SYSTEM
	CHAT_TYPE_DICE_INFO,
#endif
#ifdef ENABLE_CHAT_FILTER
	CHAT_TYPE_NOTICE_IMPROVING,
#endif
#ifdef ENABLE_NEW_DUNGEON_TYPE
	CHAT_TYPE_DUNGEON_MISSION,
	CHAT_TYPE_DUNGEON_SUBMISSION,
#endif
	CHAT_TYPE_MAX_NUM,
};

typedef struct packet_chatting
{
	BYTE	header;
	WORD	size;
	BYTE	type;
	DWORD	dwVID;
	BYTE	bEmpire;
} TPacketGCChat;

typedef struct packet_whisper
{
	BYTE        bHeader;
	WORD        wSize;
	BYTE        bType;
	char        szNameFrom[CHARACTER_NAME_MAX_LEN + 1];
} TPacketGCWhisper;

typedef struct packet_stun
{
	BYTE		header;
	DWORD		vid;
} TPacketGCStun;

typedef struct packet_dead
{
	BYTE		header;
	DWORD		vid;
} TPacketGCDead;

typedef struct packet_main_character
{
	enum
	{
		MUSIC_NAME_MAX_LEN = 24,
	};
	BYTE        header;
	DWORD       dwVID;
	WORD		wRaceNum;
	char        szUserName[CHARACTER_NAME_MAX_LEN + 1];
	char        szBGMName[MUSIC_NAME_MAX_LEN + 1];
	DWORD		dwPlayerID;
	float		fBGMVol;
	long        lX, lY, lZ;
	BYTE		byEmpire;
	BYTE		bySkillGroup;
} TPacketGCMainCharacter;

enum EPointTypes
{
	POINT_NONE,                 // 0
	POINT_LEVEL,                // 1
	POINT_EXP,                  // 3
	POINT_NEXT_EXP,             // 4
	POINT_HP,                   // 5
	POINT_MAX_HP,               // 6
	POINT_SP,                   // 7
	POINT_MAX_SP,               // 8
	POINT_STAMINA,				// 9
	POINT_MAX_STAMINA,			// 10

	POINT_GOLD,                 // 11
	POINT_ST,					// 12
	POINT_HT,					// 13
	POINT_DX,					// 14
	POINT_IQ,					//15
	POINT_ATT_POWER,			//16
	POINT_ATT_SPEED,			//17
	POINT_EVADE_RATE,			//18
	POINT_MOV_SPEED,			//19
	POINT_DEF_GRADE,			//20
	POINT_CASTING_SPEED,		//21
	POINT_MAGIC_ATT_GRADE,		//22
	POINT_MAGIC_DEF_GRADE,		//23
	POINT_EMPIRE_POINT,			//24
	POINT_LEVEL_STEP,			//25
	POINT_STAT,					//26
	POINT_SUB_SKILL,			//27
	POINT_SKILL,				//28
	POINT_MIN_ATK,				//29
	POINT_MAX_ATK,				//30
	POINT_PLAYTIME,				//31
	POINT_HP_REGEN,				//32
	POINT_SP_REGEN,				//33

	POINT_BOW_DISTANCE,			//34

	POINT_HP_RECOVERY,			//35
	POINT_SP_RECOVERY,			//36

	POINT_POISON_PCT,			//37
	POINT_STUN_PCT,				//38
	POINT_SLOW_PCT,				//39
	POINT_CRITICAL_PCT,			//40
	POINT_PENETRATE_PCT,		//41
	POINT_CURSE_PCT,			//42

	POINT_ATTBONUS_HUMAN,
	POINT_ATTBONUS_ANIMAL,
	POINT_ATTBONUS_ORC,
	POINT_ATTBONUS_MILGYO,
	POINT_ATTBONUS_UNDEAD,
	POINT_ATTBONUS_DEVIL,
	POINT_ATTBONUS_INSECT,
	POINT_ATTBONUS_FIRE,
	POINT_ATTBONUS_ICE,
	POINT_ATTBONUS_DESERT,
	POINT_ATTBONUS_UNUSED0,     // 53 UNUSED0
	POINT_ATTBONUS_UNUSED1,     // 54 UNUSED1
	POINT_ATTBONUS_UNUSED2,     // 55 UNUSED2
	POINT_ATTBONUS_UNUSED3,     // 56 UNUSED3
	POINT_ATTBONUS_UNUSED4,     // 57 UNUSED4
	POINT_ATTBONUS_UNUSED5,     // 58 UNUSED5
	POINT_ATTBONUS_UNUSED6,     // 59 UNUSED6
	POINT_ATTBONUS_UNUSED7,     // 60 UNUSED7
	POINT_ATTBONUS_UNUSED8,     // 61 UNUSED8
	POINT_ATTBONUS_UNUSED9,     // 62 UNUSED9

	POINT_STEAL_HP,
	POINT_STEAL_SP,

	POINT_MANA_BURN_PCT,

	POINT_DAMAGE_SP_RECOVER,

	POINT_BLOCK,
	POINT_DODGE,

	POINT_RESIST_SWORD,         // 69
	POINT_RESIST_TWOHAND,       // 70
	POINT_RESIST_DAGGER,        // 71
	POINT_RESIST_BELL,          // 72
	POINT_RESIST_FAN,           // 73
	POINT_RESIST_BOW,
	POINT_RESIST_FIRE,
	POINT_RESIST_ELEC,
	POINT_RESIST_MAGIC,
	POINT_RESIST_WIND,

	POINT_REFLECT_MELEE,

	POINT_REFLECT_CURSE,
	POINT_POISON_REDUCE,

	POINT_KILL_SP_RECOVER,
	POINT_EXP_DOUBLE_BONUS,     // 83
	POINT_GOLD_DOUBLE_BONUS,    // 84
	POINT_ITEM_DROP_BONUS,      // 85

	POINT_POTION_BONUS,         // 86
	POINT_KILL_HP_RECOVER,      // 87

	POINT_IMMUNE_STUN,          // 88
	POINT_IMMUNE_SLOW,          // 89
	POINT_IMMUNE_FALL,          // 90
	//////////////////

	POINT_PARTY_ATT_GRADE,      // 91
	POINT_PARTY_DEF_GRADE,      // 92

	POINT_ATT_BONUS,            // 93
	POINT_DEF_BONUS,            // 94

	POINT_ATT_GRADE_BONUS,			// 95
	POINT_DEF_GRADE_BONUS,			// 96
	POINT_MAGIC_ATT_GRADE_BONUS,	// 97
	POINT_MAGIC_DEF_GRADE_BONUS,	// 98

	POINT_RESIST_NORMAL_DAMAGE,		// 99

	POINT_STAT_RESET_COUNT = 111,
	POINT_HORSE_SKILL = 112,

	POINT_MALL_ATTBONUS,
	POINT_MALL_DEFBONUS,
	POINT_MALL_EXPBONUS,
	POINT_MALL_ITEMBONUS,
	POINT_MALL_GOLDBONUS,
	POINT_MAX_HP_PCT,
	POINT_MAX_SP_PCT,

	POINT_SKILL_DAMAGE_BONUS,
	POINT_NORMAL_HIT_DAMAGE_BONUS,

	POINT_SKILL_DEFEND_BONUS,
	POINT_NORMAL_HIT_DEFEND_BONUS,
	POINT_PC_BANG_EXP_BONUS,        // 125
	POINT_PC_BANG_DROP_BONUS,

	POINT_ENERGY = 128,

	POINT_ENERGY_END_TIME = 129,
	POINT_COSTUME_ATTR_BONUS = 130,
	POINT_MAGIC_ATT_BONUS_PER = 131,
	POINT_MELEE_MAGIC_ATT_BONUS_PER = 132,

	POINT_RESIST_ICE = 133,
	POINT_RESIST_EARTH = 134,
	POINT_RESIST_DARK = 135,

	POINT_RESIST_CRITICAL = 136,
	POINT_RESIST_PENETRATE = 137,

#ifdef ENABLE_WOLFMAN_CHARACTER
	POINT_BLEEDING_REDUCE = 138,
	POINT_BLEEDING_PCT = 139,

	POINT_ATTBONUS_WOLFMAN = 140,
	POINT_RESIST_WOLFMAN = 141,
	POINT_RESIST_CLAW = 142,
#endif

#ifdef ENABLE_ACCE_SYSTEM
	POINT_ACCEDRAIN_RATE = 143,
#endif
#ifdef ENABLE_MAGIC_REDUCTION_SYSTEM
	POINT_RESIST_MAGIC_REDUCTION = 144,
#endif
#ifdef ENABLE_CHEQUE_SYSTEM
	POINT_CHEQUE = 145,
#endif
#ifdef ENABLE_PENDANT_SYSTEM
	POINT_ENCHANT_FIRE = 146,
	POINT_ENCHANT_ICE = 147,
	POINT_ENCHANT_EARTH = 148,
	POINT_ENCHANT_DARK = 149,
	POINT_ENCHANT_WIND = 150,
	POINT_ENCHANT_ELECT = 151,
	POINT_RESIST_HUMAN = 152,

	POINT_ATTBONUS_SWORD = 153,
	POINT_ATTBONUS_TWOHAND = 154,
	POINT_ATTBONUS_DAGGER = 155,
	POINT_ATTBONUS_BELL = 156,
	POINT_ATTBONUS_FAN = 157,
	POINT_ATTBONUS_BOW = 158,
#ifdef ENABLE_WOLFMAN_CHARACTER
	POINT_ATTBONUS_CLAW = 159,
#endif
	POINT_ATTBONUS_CZ = 160,
#endif
#ifdef ENABLE_ATTR_ADDONS
	POINT_ATTBONUS_STONE = 161,
	POINT_ATTBONUS_BOSS = 162,
#endif
#ifdef ENABLE_PARTY_ROLE_REWORK
	POINT_PARTY_ATTACKER_MONSTER_BONUS = 163,
	POINT_PARTY_ATTACKER_STONE_BONUS = 164,
	POINT_PARTY_ATTACKER_BOSS_BONUS = 165,
#endif
#ifdef ENABLE_ATTR_ADDONS
	POINT_ATTBONUS_ELEMENTS = 166,
	POINT_ENCHANT_ELEMENTS = 167,
	POINT_ATTBONUS_CHARACTERS = 168,
	POINT_ENCHANT_CHARACTERS = 169,
#endif
#ifdef ENABLE_CHEST_DROP_POINT
	POINT_CHEST_BONUS = 170,
#endif
#ifdef ENABLE_BATTLE_PASS_SYSTEM
	POINT_BATTLE_PASS_ID = 171,
#endif
	POINT_MIN_WEP = 200,
	POINT_MAX_WEP,
	POINT_MIN_MAGIC_WEP,
	POINT_MAX_MAGIC_WEP,
	POINT_HIT_RATE,
};

typedef struct packet_points
{
	BYTE        header;
#ifdef ENABLE_GOLD_LIMIT_REWORK
	long long	points[POINT_MAX_NUM];
#else
	long		points[POINT_MAX_NUM];
#endif
} TPacketGCPoints;

typedef struct packet_point_change
{
	int         header;

	DWORD		dwVID;
	BYTE		Type;

#ifdef ENABLE_GOLD_LIMIT_REWORK
	long long	amount;
	long long	value;
#else
	long		amount;
	long		value;
#endif
} TPacketGCPointChange;

typedef struct packet_motion
{
	BYTE		header;
	DWORD		vid;
	DWORD		victim_vid;
	WORD		motion;
} TPacketGCMotion;

typedef struct packet_item_deldeprecated
{
	BYTE		header;
	TItemPos	Cell;
	DWORD		vnum;
#ifdef ENABLE_ITEM_COUNT_LIMIT
	DWORD		count;
#else
	BYTE		count;
#endif
	long		alSockets[ITEM_SOCKET_SLOT_MAX_NUM];
	TPlayerItemAttribute aAttr[ITEM_ATTRIBUTE_SLOT_MAX_NUM];
#ifdef ENABLE_BEGINNER_ITEM_SYSTEM
	bool		is_basic;
#endif
#ifdef ENABLE_ITEM_EVOLUTION_SYSTEM
	DWORD		evolution;
#endif
#ifdef ENABLE_CHANGELOOK_SYSTEM
	DWORD		transmutation;
#endif
} TPacketGCItemDelDeprecated;

typedef struct packet_set_item
{
	BYTE		header;
	TItemPos	Cell;
	DWORD		vnum;
#ifdef ENABLE_ITEM_COUNT_LIMIT
	DWORD		count;
#else
	BYTE		count;
#endif
	DWORD		flags;
	DWORD		anti_flags;
	bool		highlight;
	long		alSockets[ITEM_SOCKET_SLOT_MAX_NUM];
	TPlayerItemAttribute aAttr[ITEM_ATTRIBUTE_SLOT_MAX_NUM];
#ifdef ENABLE_BEGINNER_ITEM_SYSTEM
	bool		is_basic;
#endif
#ifdef ENABLE_ITEM_EVOLUTION_SYSTEM
	DWORD		evolution;
#endif
#ifdef ENABLE_CHANGELOOK_SYSTEM
	DWORD		transmutation;
#endif
} TPacketGCItemSet;

typedef struct packet_item_del
{
	BYTE        header;
	BYTE        pos;
} TPacketGCItemDel;

typedef struct packet_use_item
{
	BYTE		header;
	TItemPos	Cell;
	DWORD		ch_vid;
	DWORD		victim_vid;

	DWORD		vnum;
} TPacketGCItemUse;

typedef struct packet_update_item
{
	BYTE		header;
	TItemPos	Cell;
#ifdef ENABLE_ITEM_COUNT_LIMIT
	DWORD		count;
#else
	BYTE		count;
#endif
	long		alSockets[ITEM_SOCKET_SLOT_MAX_NUM];
	TPlayerItemAttribute aAttr[ITEM_ATTRIBUTE_SLOT_MAX_NUM];
#ifdef ENABLE_BEGINNER_ITEM_SYSTEM
	bool		is_basic;
#endif
#ifdef ENABLE_ITEM_EVOLUTION_SYSTEM
	DWORD		evolution;
#endif
#ifdef ENABLE_CHANGELOOK_SYSTEM
	DWORD		transmutation;
#endif
} TPacketGCItemUpdate;

typedef struct packet_ground_add_item
{
	BYTE		bHeader;
	long		lX;
	long		lY;
	long		lZ;

	DWORD		dwVID;
	DWORD		dwVnum;
#ifdef ENABLE_ITEM_ENTITY_UTILITY
	long		alSockets[ITEM_SOCKET_SLOT_MAX_NUM];
	DWORD		wCount;
#endif
#ifdef ENABLE_ITEM_ENTITY_UTILITY
	packet_ground_add_item()
	{
		memset(&alSockets, 0, sizeof(alSockets));
	}
#endif
} TPacketGCItemGroundAdd;

typedef struct packet_ground_del_item
{
	BYTE		header;
	DWORD		vid;
} TPacketGCItemGroundDel;

typedef struct packet_item_ownership
{
	BYTE        bHeader;
	DWORD       dwVID;
	char        szName[CHARACTER_NAME_MAX_LEN + 1];
} TPacketGCItemOwnership;

typedef struct packet_quickslot_add
{
	BYTE        header;
	BYTE        pos;
	TQuickSlot	slot;
} TPacketGCQuickSlotAdd;

typedef struct packet_quickslot_del
{
	BYTE        header;
	BYTE        pos;
} TPacketGCQuickSlotDel;

typedef struct packet_quickslot_swap
{
	BYTE        header;
	BYTE        pos;
	BYTE        change_pos;
} TPacketGCQuickSlotSwap;

typedef struct packet_shop_start
{
	struct packet_shop_item		items[SHOP_HOST_ITEM_MAX_NUM];
} TPacketGCShopStart;

typedef struct packet_shop_update_item
{
	BYTE						pos;
	struct packet_shop_item		item;
} TPacketGCShopUpdateItem;

typedef struct packet_shop_update_price
{
#ifdef ENABLE_GOLD_LIMIT_REWORK
	long long iElkAmount;
#else
	int iElkAmount;
#endif
} TPacketGCShopUpdatePrice;

enum EPacketShopSubHeaders
{
	SHOP_SUBHEADER_GC_START,
	SHOP_SUBHEADER_GC_END,
	SHOP_SUBHEADER_GC_UPDATE_ITEM,
	SHOP_SUBHEADER_GC_UPDATE_PRICE,
	SHOP_SUBHEADER_GC_OK,
	SHOP_SUBHEADER_GC_NOT_ENOUGH_MONEY,
	SHOP_SUBHEADER_GC_SOLDOUT,
	SHOP_SUBHEADER_GC_INVENTORY_FULL,
	SHOP_SUBHEADER_GC_INVALID_POS,
	SHOP_SUBHEADER_GC_SOLD_OUT,
	SHOP_SUBHEADER_GC_START_EX,
	SHOP_SUBHEADER_GC_NOT_ENOUGH_MONEY_EX,
#ifdef ENABLE_SHOP_PRICE_TYPE_ITEM
	SHOP_SUBHEADER_GC_NOT_ENOUGH_ITEM,
#endif
#ifdef ENABLE_CHEQUE_SYSTEM
	SHOP_SUBHEADER_GC_NOT_ENOUGH_CHEQUE,
	SHOP_SUBHEADER_GC_NOT_ENOUGH_CHEQUE_MONEY,
#endif
};

typedef struct packet_shop
{
	BYTE        header;
	WORD		size;
	BYTE        subheader;
} TPacketGCShop;

typedef struct packet_exchange
{
	BYTE        header;
	BYTE        subheader;
	BYTE        is_me;
#ifdef ENABLE_GOLD_LIMIT_REWORK
	long long	arg1;
#else
	DWORD		arg1;
#endif
	TItemPos       arg2;
	DWORD       arg3;
	long		alValues[ITEM_SOCKET_SLOT_MAX_NUM];
	TPlayerItemAttribute aAttr[ITEM_ATTRIBUTE_SLOT_MAX_NUM];
#ifdef ENABLE_ITEM_EVOLUTION_SYSTEM
	DWORD		evolution;
#endif
#ifdef ENABLE_CHANGELOOK_SYSTEM
	DWORD		dwTransmutation;
#endif
} TPacketGCExchange;

enum
{
	EXCHANGE_SUBHEADER_GC_START,			// arg1 == vid
	EXCHANGE_SUBHEADER_GC_ITEM_ADD,		// arg1 == vnum  arg2 == pos  arg3 == count
	EXCHANGE_SUBHEADER_GC_ITEM_DEL,		// arg1 == pos
	EXCHANGE_SUBHEADER_GC_ELK_ADD,			// arg1 == elk
	EXCHANGE_SUBHEADER_GC_ACCEPT,			// arg1 == accept
	EXCHANGE_SUBHEADER_GC_END,				// arg1 == not used
	EXCHANGE_SUBHEADER_GC_ALREADY,			// arg1 == not used
	EXCHANGE_SUBHEADER_GC_LESS_ELK,		// arg1 == not used
#ifdef ENABLE_CHEQUE_SYSTEM
	EXCHANGE_SUBHEADER_GC_CHEQUE_ADD,
	EXCHANGE_SUBHEADER_GC_LESS_CHEQUE,
#endif // ENABLE_CHEQUE_SYSTEM
};

typedef struct packet_position
{
	BYTE        header;
	DWORD		vid;
	BYTE        position;
} TPacketGCPosition;

typedef struct packet_script
{
	BYTE		header;
	WORD        size;
	BYTE		skin;
	WORD        src_size;
} TPacketGCScript;

typedef struct packet_target
{
	BYTE        header;
	DWORD       dwVID;
	BYTE        bHPPercent;
#ifdef ENABLE_TARGET_BOARD_RENEWAL
	HPTYPE		dwHP;
	HPTYPE		dwMaxHP;
#endif
	packet_target()
	{
		dwVID = 0;
		bHPPercent = 0;
#ifdef ENABLE_TARGET_BOARD_RENEWAL
		dwHP = 0;
		dwMaxHP = 0;
#endif
	};
} TPacketGCTarget;

typedef struct packet_damage_info
{
	BYTE header;
	DWORD dwVID;
	BYTE flag;
#ifdef ENABLE_DAMAGE_LIMIT_REWORK
	long long damage;
#else
	int  damage;
#endif
} TPacketGCDamageInfo;

typedef struct packet_move
{
	BYTE		bHeader;
	BYTE		bFunc;
	BYTE		bArg;
	BYTE		bRot;
	DWORD		dwVID;
	LONG		lX;
	LONG		lY;
	DWORD		dwTime;
	DWORD		dwDuration;
} TPacketGCMove;

enum
{
	QUEST_SEND_IS_BEGIN = 1 << 0,
	QUEST_SEND_TITLE = 1 << 1,
	QUEST_SEND_CLOCK_NAME = 1 << 2,
	QUEST_SEND_CLOCK_VALUE = 1 << 3,
	QUEST_SEND_COUNTER_NAME = 1 << 4,
	QUEST_SEND_COUNTER_VALUE = 1 << 5,
	QUEST_SEND_ICON_FILE = 1 << 6,
};

typedef struct packet_quest_info
{
	BYTE header;
	WORD size;
	WORD index;
#ifdef ENABLE_QUEST_CATEGORY_SYSTEM
	WORD c_index;
#endif
	BYTE flag;
} TPacketGCQuestInfo;

typedef struct packet_quest_confirm
{
	BYTE header;
	char msg[64 + 1];
	long timeout;
	DWORD requestPID;
} TPacketGCQuestConfirm;

typedef struct packet_attack
{
	BYTE        header;
	DWORD       dwVID;
	DWORD       dwVictimVID;
	BYTE        bType;
} TPacketGCAttack;

typedef struct packet_c2c
{
	BYTE		header;
	WORD		wSize;
} TPacketGCC2C;

typedef struct packetd_sync_position_element
{
	DWORD       dwVID;
	long        lX;
	long        lY;
} TPacketGCSyncPositionElement;

typedef struct packetd_sync_position
{
	BYTE        bHeader;
	WORD		wSize;
} TPacketGCSyncPosition;

typedef struct packet_ownership
{
	BYTE                bHeader;
	DWORD               dwOwnerVID;
	DWORD               dwVictimVID;
} TPacketGCOwnership;

#define	SKILL_MAX_NUM 255

typedef struct packet_skill_level
{
	BYTE        bHeader;
	BYTE        abSkillLevels[SKILL_MAX_NUM];
} TPacketGCSkillLevel;

typedef struct SPlayerSkill
{
	BYTE bMasterType;
	BYTE bLevel;
	time_t tNextRead;
} TPlayerSkill;

typedef struct packet_skill_level_new
{
	BYTE bHeader;
	TPlayerSkill skills[SKILL_MAX_NUM];
} TPacketGCSkillLevelNew;

// fly
typedef struct packet_fly
{
	BYTE        bHeader;
	BYTE        bType;
	DWORD       dwStartVID;
	DWORD       dwEndVID;
} TPacketGCCreateFly;

enum EPVPModes
{
	PVP_MODE_NONE,
	PVP_MODE_AGREE,
	PVP_MODE_FIGHT,
	PVP_MODE_REVENGE,
};

typedef struct packet_duel_start
{
	BYTE	header;
	WORD	wSize;
} TPacketGCDuelStart;

typedef struct packet_pvp
{
	BYTE		header;
	DWORD		dwVIDSrc;
	DWORD		dwVIDDst;
	BYTE		bMode;
} TPacketGCPVP;

typedef struct packet_skill_cooltime_end
{
	BYTE		header;
	BYTE		bSkill;
} TPacketGCSkillCoolTimeEnd;

typedef struct packet_warp
{
	BYTE			bHeader;
	LONG			lX;
	LONG			lY;
	LONG			lAddr;
	WORD			wPort;
} TPacketGCWarp;

typedef struct packet_party_invite
{
	BYTE header;
	DWORD leader_pid;
} TPacketGCPartyInvite;

typedef struct packet_party_add
{
	BYTE header;
	DWORD pid;
	char name[CHARACTER_NAME_MAX_LEN + 1];
} TPacketGCPartyAdd;

typedef struct packet_party_update
{
	BYTE header;
	DWORD pid;
	BYTE state;
	BYTE percent_hp;
	short affects[PARTY_AFFECT_SLOT_MAX_NUM];
} TPacketGCPartyUpdate;

typedef struct packet_party_remove
{
	BYTE header;
	DWORD pid;
} TPacketGCPartyRemove;

typedef TPacketCGSafeboxCheckout TPacketGCSafeboxCheckout;
typedef TPacketCGSafeboxCheckin TPacketGCSafeboxCheckin;

typedef struct packet_safebox_wrong_password
{
	BYTE        bHeader;
} TPacketGCSafeboxWrongPassword;

typedef struct packet_safebox_size
{
	BYTE bHeader;
	BYTE bSize;
} TPacketGCSafeboxSize;

typedef struct command_empire
{
	BYTE        bHeader;
	BYTE        bEmpire;
} TPacketCGEmpire;

typedef struct packet_empire
{
	BYTE        bHeader;
	BYTE        bEmpire;
} TPacketGCEmpire;

enum
{
	FISHING_SUBHEADER_GC_START,
	FISHING_SUBHEADER_GC_STOP,
	FISHING_SUBHEADER_GC_REACT,
	FISHING_SUBHEADER_GC_SUCCESS,
	FISHING_SUBHEADER_GC_FAIL,
	FISHING_SUBHEADER_GC_FISH,
};

typedef struct packet_fishing
{
	BYTE header;
	BYTE subheader;
	DWORD info;
	BYTE dir;
} TPacketGCFishing;

typedef struct paryt_parameter
{
	BYTE        bHeader;
	BYTE        bDistributeMode;
} TPacketGCPartyParameter;

enum
{
	GUILD_SUBHEADER_GC_LOGIN,
	GUILD_SUBHEADER_GC_LOGOUT,
	GUILD_SUBHEADER_GC_LIST,
	GUILD_SUBHEADER_GC_GRADE,
	GUILD_SUBHEADER_GC_ADD,
	GUILD_SUBHEADER_GC_REMOVE,
	GUILD_SUBHEADER_GC_GRADE_NAME,
	GUILD_SUBHEADER_GC_GRADE_AUTH,
	GUILD_SUBHEADER_GC_INFO,
	GUILD_SUBHEADER_GC_COMMENTS,
	GUILD_SUBHEADER_GC_CHANGE_EXP,
	GUILD_SUBHEADER_GC_CHANGE_MEMBER_GRADE,
	GUILD_SUBHEADER_GC_SKILL_INFO,
	GUILD_SUBHEADER_GC_CHANGE_MEMBER_GENERAL,
	GUILD_SUBHEADER_GC_GUILD_INVITE,
	GUILD_SUBHEADER_GC_WAR,
	GUILD_SUBHEADER_GC_GUILD_NAME,
	GUILD_SUBHEADER_GC_GUILD_WAR_LIST,
	GUILD_SUBHEADER_GC_GUILD_WAR_END_LIST,
	GUILD_SUBHEADER_GC_WAR_POINT,
};

typedef struct packet_guild
{
	BYTE header;
	WORD size;
	BYTE subheader;
} TPacketGCGuild;

enum
{
	GUILD_AUTH_ADD_MEMBER = (1 << 0),
	GUILD_AUTH_REMOVE_MEMBER = (1 << 1),
	GUILD_AUTH_NOTICE = (1 << 2),
	GUILD_AUTH_SKILL = (1 << 3),
};

typedef struct packet_guild_sub_grade
{
	char grade_name[GUILD_GRADE_NAME_MAX_LEN + 1];
	BYTE auth_flag;
} TPacketGCGuildSubGrade;

typedef struct packet_guild_sub_member
{
	DWORD pid;
	BYTE byGrade;
	BYTE byIsGeneral;
	BYTE byJob;
	BYTE byLevel;
	DWORD dwOffer;
	BYTE byNameFlag;
	// if NameFlag is TRUE, name is sent from server.
	//	char szName[CHARACTER_ME_MAX_LEN+1];
} TPacketGCGuildSubMember;

typedef struct packet_guild_sub_info
{
	WORD member_count;
	WORD max_member_count;
	DWORD guild_id;
	DWORD master_pid;
	DWORD exp;
	BYTE level;
	char name[GUILD_NAME_MAX_LEN + 1];
} TPacketGCGuildInfo;

enum EGuildWarState
{
	GUILD_WAR_NONE,
	GUILD_WAR_SEND_DECLARE,
	GUILD_WAR_REFUSE,
	GUILD_WAR_RECV_DECLARE,
	GUILD_WAR_WAIT_START,
	GUILD_WAR_CANCEL,
	GUILD_WAR_ON_WAR,
	GUILD_WAR_END,
	GUILD_WAR_DURATION = 2 * 60 * 60,
};

typedef struct packet_guild_war
{
	DWORD       dwGuildSelf;
	DWORD       dwGuildOpp;
	BYTE        bType;
	BYTE        bWarState;
#ifdef ENABLE_GUILD_WAR_REWORK
	int			iMaxPlayer;
	int			iMaxScore;
#endif
} TPacketGCGuildWar;

typedef struct SPacketGuildWarPoint
{
	DWORD dwGainGuildID;
	DWORD dwOpponentGuildID;
	long lPoint;
} TPacketGuildWarPoint;

enum
{
	DUNGEON_SUBHEADER_GC_TIME_ATTACK_START = 0,
	DUNGEON_SUBHEADER_GC_DESTINATION_POSITION = 1,
};

typedef struct packet_dungeon
{
	BYTE		bHeader;
	WORD		size;
	BYTE		subheader;
} TPacketGCDungeon;

typedef struct SPacketGCShopSign
{
	BYTE        bHeader;
	DWORD       dwVID;
	char        szSign[SHOP_SIGN_MAX_LEN + 1];
} TPacketGCShopSign;

typedef struct SPacketGCTime
{
	BYTE        bHeader;
	time_t      time;
} TPacketGCTime;

enum
{
	WALKMODE_RUN,
	WALKMODE_WALK,
};

typedef struct SPacketGCWalkMode
{
	BYTE        header;
	DWORD       vid;
	BYTE        mode;
} TPacketGCWalkMode;

typedef struct SPacketGCChangeSkillGroup
{
	BYTE        header;
	BYTE        skill_group;
} TPacketGCChangeSkillGroup;

struct TMaterial
{
	DWORD vnum;
	DWORD count;
};

typedef struct SRefineTable
{
	DWORD src_vnum;
	DWORD result_vnum;
#ifdef ENABLE_ITEM_COUNT_LIMIT
	DWORD material_count;
#else
	BYTE material_count;
#endif
#ifdef ENABLE_GOLD_LIMIT_REWORK
	long long cost;
#else
	int cost;
#endif
	int prob;
#ifdef ENABLE_REFINE_REWORK
	int success_prob;
#endif
	TMaterial materials[REFINE_MATERIAL_MAX_NUM];
} TRefineTable;

typedef struct SPacketGCRefineInformationNew
{
	BYTE			header;
	BYTE			type;
	BYTE			pos;
	TRefineTable	refine_table;
} TPacketGCRefineInformationNew;

enum SPECIAL_EFFECT
{
	SE_NONE,
	SE_HPUP_RED,
	SE_SPUP_BLUE,
	SE_SPEEDUP_GREEN,
	SE_DXUP_PURPLE,
	SE_CRITICAL,
	SE_PENETRATE,
	SE_BLOCK,
	SE_DODGE,
	SE_CHINA_FIREWORK,
	SE_SPIN_TOP,
	SE_SUCCESS,
	SE_FAIL,
	SE_FR_SUCCESS,
	SE_PERCENT_DAMAGE1,
	SE_PERCENT_DAMAGE2,
	SE_PERCENT_DAMAGE3,
	SE_AUTO_HPUP,
	SE_AUTO_SPUP,
	SE_EQUIP_RAMADAN_RING,
	SE_EQUIP_HALLOWEEN_CANDY,
	SE_EQUIP_HAPPINESS_RING,
	SE_EQUIP_LOVE_PENDANT,
#ifdef ENABLE_ACCE_SYSTEM
	SE_EFFECT_ACCE_SUCCEDED,
	SE_EFFECT_ACCE_EQUIP,
#endif
#ifdef ENABLE_PVP_EFFECT
	SE_PVP_WIN,
	SE_PVP_OPEN1,
	SE_PVP_OPEN2,
	SE_PVP_BEGIN1,
	SE_PVP_BEGIN2,
#endif
#ifdef ENABLE_SMITH_EFFECT
	SE_FR_FAIL,
#endif
#ifdef ENABLE_AGGR_MONSTER_EFFECT
	SE_AGGREGATE_MONSTER_EFFECT,
#endif
};

typedef struct SPacketGCSpecialEffect
{
	BYTE header;
	BYTE type;
	DWORD vid;
} TPacketGCSpecialEffect;

typedef struct SPacketGCNPCPosition
{
	BYTE header;
	WORD size;
	WORD count;
} TPacketGCNPCPosition;

struct TNPCPosition
{
	BYTE bType;
	char name[CHARACTER_NAME_MAX_LEN + 1];
	long x;
	long y;
};

typedef struct SPacketGCChangeName
{
	BYTE header;
	DWORD pid;
	char name[CHARACTER_NAME_MAX_LEN + 1];
} TPacketGCChangeName;

enum EBlockAction
{
	BLOCK_EXCHANGE = (1 << 0),
	BLOCK_PARTY_INVITE = (1 << 1),
	BLOCK_GUILD_INVITE = (1 << 2),
	BLOCK_WHISPER = (1 << 3),
	BLOCK_MESSENGER_INVITE = (1 << 4),
	BLOCK_PARTY_REQUEST = (1 << 5),
	BLOCK_POINT_EXP = (1 << 6),
	BLOCK_COSTUME_BODY = (1 << 7),
	BLOCK_COSTUME_HAIR = (1 << 8),
	BLOCK_COSTUME_WEAPON = (1 << 9),
	BLOCK_COSTUME_ACCE = (1 << 10),
	BLOCK_COSTUME_ACCE_EX = (1 << 11),
	BLOCK_COSTUME_AURA = (1 << 12),	
};

typedef struct packet_login_key
{
	BYTE	bHeader;
	DWORD	dwLoginKey;
} TPacketGCLoginKey;

typedef struct packet_auth_success
{
	BYTE        bHeader;
	DWORD       dwLoginKey;
	BYTE        bResult;
} TPacketGCAuthSuccess;

typedef struct packet_channel
{
	BYTE header;
	BYTE channel;
} TPacketGCChannel;

typedef struct
{
	BYTE        bHeader;
	long        lID;
	char        szTargetName[32 + 1];
} TPacketGCTargetCreate;

enum
{
	CREATE_TARGET_TYPE_NONE,
	CREATE_TARGET_TYPE_LOCATION,
	CREATE_TARGET_TYPE_CHARACTER,
};

typedef struct
{
	BYTE		bHeader;
	long		lID;
	char		szTargetName[32 + 1];
	DWORD		dwVID;
	BYTE		byType;
} TPacketGCTargetCreateNew;

typedef struct
{
	BYTE        bHeader;
	long        lID;
	long        lX, lY;
} TPacketGCTargetUpdate;

typedef struct
{
	BYTE        bHeader;
	long        lID;
} TPacketGCTargetDelete;

typedef struct
{
	DWORD       dwType;
	BYTE        bPointIdxApplyOn;
	long        lApplyValue;
	DWORD       dwFlag;
	long        lDuration;
	long        lSPCost;
} TPacketAffectElement;

typedef struct
{
	BYTE bHeader;
	bool bByLoad;
	TPacketAffectElement elem;
} TPacketGCAffectAdd;

typedef struct
{
	BYTE bHeader;
	DWORD dwType;
	BYTE bApplyOn;
} TPacketGCAffectRemove;

typedef struct packet_mall_open
{
	BYTE bHeader;
	BYTE bSize;
} TPacketGCMallOpen;

typedef struct packet_dig_motion
{
	BYTE header;
	DWORD vid;
	DWORD target_vid;
	BYTE count;
} TPacketGCDigMotion;

typedef struct packet_state
{
	BYTE			bHeader;
	BYTE			bFunc;
	BYTE			bArg;
	BYTE			bRot;
	DWORD			dwVID;
	DWORD			dwTime;
	TPixelPosition	kPPos;
} TPacketCCState;

typedef struct SPacketGCSpecificEffect
{
	BYTE header;
	DWORD vid;
	char effect_file[128];
} TPacketGCSpecificEffect;

enum EDragonSoulRefineWindowRefineType
{
	DragonSoulRefineWindow_UPGRADE,
	DragonSoulRefineWindow_IMPROVEMENT,
	DragonSoulRefineWindow_REFINE,
};

enum EPacketCGDragonSoulSubHeaderType
{
	DS_SUB_HEADER_OPEN,
	DS_SUB_HEADER_CLOSE,
	DS_SUB_HEADER_DO_UPGRADE,
	DS_SUB_HEADER_DO_IMPROVEMENT,
	DS_SUB_HEADER_DO_REFINE,
	DS_SUB_HEADER_REFINE_FAIL,
	DS_SUB_HEADER_REFINE_FAIL_MAX_REFINE,
	DS_SUB_HEADER_REFINE_FAIL_INVALID_MATERIAL,
	DS_SUB_HEADER_REFINE_FAIL_NOT_ENOUGH_MONEY,
	DS_SUB_HEADER_REFINE_FAIL_NOT_ENOUGH_MATERIAL,
	DS_SUB_HEADER_REFINE_FAIL_TOO_MUCH_MATERIAL,
	DS_SUB_HEADER_REFINE_SUCCEED,
};

typedef struct SPacketCGDragonSoulRefine
{
	SPacketCGDragonSoulRefine() : header(HEADER_CG_DRAGON_SOUL_REFINE)
	{}
	BYTE header;
	BYTE bSubType;
	TItemPos ItemGrid[DS_REFINE_WINDOW_MAX_NUM];
} TPacketCGDragonSoulRefine;

typedef struct SPacketGCDragonSoulRefine
{
	SPacketGCDragonSoulRefine() : header(HEADER_GC_DRAGON_SOUL_REFINE)
	{}
	BYTE header;
	BYTE bSubType;
	TItemPos Pos;
} TPacketGCDragonSoulRefine;

typedef struct SChannelStatus
{
	WORD nPort;
	BYTE bStatus;
} TChannelStatus;

#ifdef ENABLE_GUILD_LEADER_SYSTEM
enum EGuildMemberTypes {
	GUILD_MEMBER,
	GUILD_MEMBER_LEADER,
	GUILD_MEMBER_GENERAL,
};
#endif

#ifdef ENABLE_ACCE_SYSTEM
enum EAcceInfo
{
	ACCE_ABSORPTION_SOCKET = 0,
	ACCE_ABSORBED_SOCKET = 1,
	ACCE_CLEAN_ATTR_VALUE0 = 7,
	ACCE_WINDOW_MAX_MATERIALS = 2,
};

enum
{
	ACCE_SUBHEADER_GC_OPEN = 0,
	ACCE_SUBHEADER_GC_CLOSE,
	ACCE_SUBHEADER_GC_ADDED,
	ACCE_SUBHEADER_GC_REMOVED,
	ACCE_SUBHEADER_CG_REFINED,
	ACCE_SUBHEADER_CG_CLOSE = 0,
	ACCE_SUBHEADER_CG_ADD,
	ACCE_SUBHEADER_CG_REMOVE,
	ACCE_SUBHEADER_CG_REFINE,
};

typedef struct SPacketAcce
{
	BYTE	header;
	BYTE	subheader;
	bool	bWindow;
#ifdef ENABLE_GOLD_LIMIT_REWORK
	long long	llPrice;
#else
	DWORD	dwPrice;
#endif
	BYTE	bPos;
	TItemPos	tPos;
	DWORD	dwItemVnum;
	DWORD	dwMinAbs;
	DWORD	dwMaxAbs;
} TPacketAcce;

typedef struct SAcceMaterial
{
	BYTE	bHere;
	WORD	wCell;
} TAcceMaterial;

typedef struct SAcceResult
{
	DWORD	dwItemVnum;
	DWORD	dwMinAbs;
	DWORD	dwMaxAbs;
} TAcceResult;
#endif

#ifdef ENABLE_ANCIENT_ATTR_SYSTEM
typedef struct command_item_new_attribute
{
	BYTE  header;
	TItemPos source_pos;
	TItemPos target_pos;
	BYTE bValues[3 + 1];
} TPacketCGItemNewAttribute;
#endif
#ifdef ENABLE_CHEST_INFO_SYSTEM
typedef struct SPacketCGChestDropInfo {
	BYTE	header;
	DWORD	dwChestVnum;
	TItemPos	pos;
} TPacketCGChestDropInfo;

typedef struct SChestDropInfoTable {
	BYTE	bPageIndex;
	WORD	bSlotIndex;
	DWORD	dwItemVnum;
#ifdef ENABLE_ITEM_COUNT_LIMIT
	DWORD	bItemCount;
#else
	BYTE	bItemCount;
#endif
} TChestDropInfoTable;

typedef struct SPacketGCChestDropInfo {
	BYTE	bHeader;
	int	wSize;
	DWORD	dwChestVnum;
	TItemPos	pos;
} TPacketGCChestDropInfo;
#endif

#ifdef ENABLE_GLOBAL_MESSAGE_UTILITY
typedef struct SPacketCGBulkWhisper
{
	BYTE	header;
	char	szText[512 + 1];
} TPacketCGBulkWhisper;

typedef struct packet_bulk_whisper
{
	BYTE	header;
	WORD	size;
} TPacketGCBulkWhisper;
#endif
#ifdef ENABLE_GROWTH_PET_SYSTEM
typedef struct packet_RequestPetName
{
	BYTE byHeader;
	char petname[13];
} TPacketCGRequestPetName;
#endif
#ifdef ENABLE_EVENT_SYSTEM
typedef struct packet_event_info
{
	BYTE		bHeader;
	BYTE		event_id;
	bool		isActivate;
	DWORD		event_time;
} TPacketGCEventInfo;
#endif
#ifdef ENABLE_SKILL_CHOOSE_SYSTEM
typedef struct command_SkillChoose_Send
{
	BYTE bHeader;
	int job;
} TPacketCGSkillChoose;

typedef struct SPacketGCSkillChoose
{
	BYTE bHeader;
	WORD job;
} TPacketGCSkillChoose;
#endif

#ifdef ENABLE_CHANGELOOK_SYSTEM
enum EChangeLookInfo
{
	CL_WINDOW_MAX_MATERIALS = 3,
	CL_CLEAN_ATTR_VALUE0 = 8,
	CL_SCROLL_VNUM = 72326,
};

enum
{
	CL_SUBHEADER_OPEN = 0,
	CL_SUBHEADER_CLOSE,
	CL_SUBHEADER_ADD,
	CL_SUBHEADER_REMOVE,
	CL_SUBHEADER_REFINE,
};

typedef struct SPacketChangeLook
{
	BYTE	header;
	BYTE	subheader;
#ifdef ENABLE_GOLD_LIMIT_REWORK
	long long llCost;
#else
	DWORD	dwCost;
#endif
	BYTE	bPos;
	TItemPos	tPos;
#ifdef ENABLE_MOUNT_CHANGELOOK_SYSTEM
	mutable	bool	bMount;
#endif
} TPacketChangeLook;

typedef struct SChangeLookMaterial
{
	BYTE	bHere;
	WORD	wCell;
} TChangeLookMaterial;
#endif

#ifdef ENABLE_SKILL_COLOR_SYSTEM
typedef struct packet_skill_color
{
	BYTE		bheader;
	BYTE		skill;
	DWORD		col1;
	DWORD		col2;
	DWORD		col3;
	DWORD		col4;
	DWORD		col5;
}TPacketCGSkillColor;
#endif

#ifdef ENABLE_DUNGEON_INFO_SYSTEM
typedef struct command_send_teleport
{
	BYTE	header;
	BYTE	dungeonIndex;
}TPacketGCTeleport;
#endif

#ifdef ENABLE_SWITCHBOT_SYSTEM
enum ECGSwitchbotSubheader
{
	SUBHEADER_CG_SWITCHBOT_START,
	SUBHEADER_CG_SWITCHBOT_STOP,
	SUBHEADER_CG_SWITCHBOT_CHANGE_SPEED,
};

struct TPacketCGSwitchbot
{
	BYTE header;
	int size;
	BYTE subheader;
	BYTE slot;
};

enum EGCSwitchbotSubheader
{
	SUBHEADER_GC_SWITCHBOT_UPDATE,
	SUBHEADER_GC_SWITCHBOT_UPDATE_ITEM,
	SUBHEADER_GC_SWITCHBOT_SEND_ATTRIBUTE_INFORMATION,
};

struct TPacketGCSwitchbot
{
	BYTE header;
	int size;
	BYTE subheader;
	BYTE slot;
};

struct TSwitchbotUpdateItem
{
	BYTE	slot;
	DWORD	vnum;
#ifdef ENABLE_ITEM_COUNT_LIMIT
	DWORD	count;
#else
	BYTE	count;
#endif
	long	alSockets[ITEM_SOCKET_SLOT_MAX_NUM];
	TPlayerItemAttribute aAttr[ITEM_ATTRIBUTE_SLOT_MAX_NUM];
};
#endif

#ifdef ENABLE_CUBE_RENEWAL

enum
{
	CUBE_RENEWAL_SUB_HEADER_OPEN_RECEIVE,
	CUBE_RENEWAL_SUB_HEADER_CLEAR_DATES_RECEIVE,
	CUBE_RENEWAL_SUB_HEADER_DATES_RECEIVE,
	CUBE_RENEWAL_SUB_HEADER_DATES_LOADING,

	CUBE_RENEWAL_SUB_HEADER_MAKE_ITEM,
	CUBE_RENEWAL_SUB_HEADER_CLOSE,
};

typedef struct  packet_send_cube_renewal
{
	BYTE header;
	BYTE subheader;
	DWORD index_item;
	DWORD count_item;
	DWORD index_item_improve;
}TPacketCGCubeRenewalSend;

typedef struct dates_cube_renewal
{
	DWORD npc_vnum;
	DWORD index;
	DWORD	vnum_reward;
	int		count_reward;
	bool 	item_reward_stackable;
	DWORD	vnum_material_1;
	int		count_material_1;
	DWORD	vnum_material_2;
	int		count_material_2;
	DWORD	vnum_material_3;
	int		count_material_3;
	DWORD	vnum_material_4;
	int		count_material_4;
	DWORD	vnum_material_5;
	int		count_material_5;
	DWORD	vnum_material_6;
	int		count_material_6;
#ifdef ENABLE_GOLD_LIMIT_REWORK
	long long	gold;
#else
	int 	gold;
#endif
#ifdef ENABLE_GEM_SYSTEM
	int		gem;
#endif
	int 	percent;
	char 	category[100];
}TInfoDateCubeRenewal;

typedef struct packet_receive_cube_renewal
{
	packet_receive_cube_renewal() : header(HEADER_GC_CUBE_RENEWAL)
	{}

	BYTE header;
	BYTE subheader;
	TInfoDateCubeRenewal	date_cube_renewal;
}TPacketGCCubeRenewalReceive;
#endif

#ifdef ENABLE_OFFLINE_SHOP

//ACTIONS PACKETS
typedef struct
{
	BYTE bHeader;
#ifdef ENABLE_LARGE_DYNAMIC_PACKET
	int wSize;
#else
	WORD wSize;
#endif
	BYTE bSubHeader;
} TPacketGCNewOfflineshop;

typedef struct {
	BYTE bHeader;
	WORD wSize;
	BYTE bSubHeader;
} TPacketCGNewOfflineShop;

namespace offlineshop
{
	//patch 08-03-2020
	enum class ExpirationType {
		EXPIRE_NONE,
		EXPIRE_REAL_TIME,
		EXPIRE_REAL_TIME_FIRST_USE,
	};

	typedef struct SPriceInfo
	{
		long long	illYang;
#ifdef ENABLE_CHEQUE_SYSTEM
		int		iCheque;
#endif

		SPriceInfo() : illYang(0)
#ifdef ENABLE_CHEQUE_SYSTEM
			, iCheque(0)
#endif
		{}
	} TPriceInfo;

	typedef struct
	{
		DWORD	dwVnum;
		DWORD	dwCount;
		long	alSockets[ITEM_SOCKET_SLOT_MAX_NUM];
		TPlayerItemAttribute    aAttr[ITEM_ATTRIBUTE_SLOT_MAX_NUM];

#ifdef ENABLE_CHANGELOOK_SYSTEM
		DWORD	dwTransmutation;
#endif
#ifdef ENABLE_ITEM_EVOLUTION_SYSTEM
		DWORD	dwEvolution;
#endif

		//patch 08-03-2020
		ExpirationType expiration;
	} TItemInfoEx;

	typedef struct SItemInfo
	{
		DWORD		dwOwnerID, dwItemID;
		TPriceInfo	price;
		TItemInfoEx	item;
		char		szOwnerName[CHARACTER_NAME_MAX_LEN + 1];
		SItemInfo() {
			memset(szOwnerName, 0, sizeof(szOwnerName));
		}
	} TItemInfo;

	typedef struct SValutesInfoa
	{
		long long	illYang;
#ifdef ENABLE_CHEQUE_SYSTEM
		int			iCheque;
#endif

		void operator +=(const SValutesInfoa& r)
		{
			illYang += r.illYang;
#ifdef ENABLE_CHEQUE_SYSTEM
			iCheque += r.iCheque;
#endif
		}

		void operator -=(const SValutesInfoa& r)
		{
			illYang -= r.illYang;
#ifdef ENABLE_CHEQUE_SYSTEM
			iCheque -= r.iCheque;
#endif
		}

		SValutesInfoa() : illYang(0)
#ifdef ENABLE_CHEQUE_SYSTEM
			, iCheque(0)
#endif
		{}
	} TValutesInfo;

	typedef struct {
		DWORD	dwOwnerID;
		DWORD	dwDuration;
		char	szName[OFFLINE_SHOP_NAME_MAX_LEN];
		DWORD	dwCount;
		long	lMapIndex;
		long	x, y;
		BYTE	bChannel;
	} TShopInfo;

	enum EFilterMinMax
	{
		OFFLINESHOP_FILTER_MIN,
		OFFLINESHOP_FILTER_MAX,
		OFFLINESHOP_FILTER_NUM,
	};

	typedef struct
	{
		BYTE		bType;
		BYTE		bSubType;
		WORD		wNameMode;
		char		szName[OFFLINE_SHOP_ITEM_MAX_LEN];
		unsigned int		antiflag;
		DWORD		wCount[2];
		WORD		wDSS[2];
		WORD		wLevel[2];
		long long		llYang[2];
		DWORD			wCheque[2];
		WORD		wAcceAbsorb[2];
		WORD		wDSLevel;
		WORD		wDSVal[3];
	}TFilterInfo;

	typedef struct {
		TItemPos	pos;
		TPriceInfo	price;
	}TShopItemInfo;

	//GAME TO CLIENT
	enum eSubHeaderGC
	{
		SUBHEADER_GC_SHOP_LIST,
		SUBHEADER_GC_SHOP_OPEN,
		SUBHEADER_GC_SHOP_OPEN_OWNER,
		SUBHEADER_GC_SHOP_OPEN_OWNER_NO_SHOP,
		SUBHEADER_GC_SHOP_CLOSE,
		SUBHEADER_GC_SHOP_BUY_ITEM_FROM_SEARCH,

		SUBHEADER_GC_SHOP_FILTER_RESULT,
		SUBHEADER_GC_SHOP_SAFEBOX_REFRESH,

#ifdef ENABLE_OFFLINE_SHOP_CITIES
		SUBHEADER_GC_INSERT_SHOP_ENTITY,
		SUBHEADER_GC_REMOVE_SHOP_ENTITY,
#endif
		SUBHEADER_GC_SHOP_BUY_RESULT,
		SUBHEADER_GC_SHOP_POPUP,
		SUBHEADER_GC_SHOP_CREATE_SUCCESS,
	};

	enum eErrorHeaderGC
	{
		SUBERROR_GC_SHOP_CHARACTER_ACTIONS,
		SUBERROR_GC_SHOP_CHANNEL_LIMIT,
		SUBERROR_GC_SHOP_CHANNEL_LIMIT_MAP,
		SUBERROR_GC_SHOP_DURATION_MIN,
		SUBERROR_GC_SHOP_DURATION_MAX,
		SUBERROR_GC_SHOP_ENOUGH_GOLD,
		SUBERROR_GC_SHOP_ALLOWED_MAP_INDEX,
		SUBERROR_GC_SHOP_ZERO_GOLD_CHEQUE,
		SUBERROR_GC_SHOP_LOCKED_ITEM,
#ifdef ENABLE_BEGINNER_ITEM_SYSTEM
		SUBERROR_GC_SHOP_BEGINNER_ITEM,
#endif
		SUBERROR_GC_SHOP_ANTIFLAG_ITEM,
		SUBERROR_GC_SHOP_REMOVE_LAST_ITEM,
		SUBERROR_GC_SHOP_REPORT_ADMIN,
		SUBERROR_GC_SHOP_CANT_FIND_ITEM,
		SUBERROR_GC_SHOP_ITEM_LIMIT,
		SUBERROR_GC_SHOP_LEVEL_LIMIT,
		SUBERROR_GC_SHOP_DURATION_NON,
		SUBERROR_GC_SHOP_GOLD_LIMIT_MAX,
#ifdef ENABLE_CHEQUE_SYSTEM
		SUBERROR_GC_SHOP_CHEQUE_LIMIT_MAX,
#endif
		SUBERROR_GC_SHOP_GOLD_LIMIT_MIN,
#ifdef ENABLE_CHEQUE_SYSTEM
		SUBERROR_GC_SHOP_CHEQUE_LIMIT_MIN,
#endif
		SUBERROR_GC_SHOP_DUNGEON,
		SUBERROR_GC_WAR_MAP,
		SUBERROR_GC_SHOP_NAME,
		SUBERROR_GC_SHOP_BUY_FAIL,
		SUBERROR_GC_SHOP_COUNT_LIMIT,
		SUBERROR_GC_SHOP_BUY_SOLD,
		SUBERROR_GC_SHOP_MAP_LIMIT_FIRST,
		SUBERROR_GC_SHOP_MAP_LIMIT_GLOBAL,
	};

	enum eConstShopWindow
	{
		OFFLINESHOP_WINDOW_BUILDER,
		OFFLINESHOP_WINDOW_EDIT,
		OFFLINESHOP_WINDOW_VIEW,
		OFFLINESHOP_WINDOW_SEARCH,
		OFFLINESHOP_WINDOW_MAX,
	};

	typedef struct {
		TShopInfo	shop;
	} TSubPacketGCShopOpen;

	typedef struct {
		TShopInfo	shop;
	} TSubPacketGCShopOpenOwner;

	typedef struct {
		DWORD dwCount;
	} TSubPacketGCShopFilterResult;

	typedef struct {
		TValutesInfo	valute;
		DWORD			dwItemCount;
	}TSubPacketGCShopSafeboxRefresh;

	typedef struct {
		DWORD dwOwnerID;
		DWORD dwItemID;
	}TSubPacketGCShopBuyItemFromSearch;

#ifdef ENABLE_OFFLINE_SHOP_CITIES
	typedef struct {
		DWORD	dwVID;
		char	szName[OFFLINE_SHOP_NAME_MAX_LEN];
		int		iType;
		long 	x, y, z;
		DWORD	owner;
	} TSubPacketGCInsertShopEntity;

	typedef struct {
		DWORD dwVID;
	} TSubPacketGCRemoveShopEntity;
#endif

	typedef struct
	{
		bool	bSuccess;
	}TSubPacketGCShopBuyResult;

	typedef struct {
		BYTE	bWindow;
		BYTE	bErrorHeader;
	} TSubPacketGCShopPopup;

	// CLIENT TO GAME
	enum eSubHeaderCG
	{
		SUBHEADER_CG_SHOP_CREATE_NEW,
		SUBHEADER_CG_SHOP_FORCE_CLOSE,
		SUBHEADER_CG_SHOP_OPEN,
		SUBHEADER_CG_SHOP_OPEN_OWNER,
		SUBHEADER_CG_SHOP_BUY_ITEM,

		SUBHEADER_CG_SHOP_ADD_ITEM,
		SUBHEADER_CG_SHOP_REMOVE_ITEM,

		SUBHEADER_CG_SHOP_FILTER_REQUEST,

		SUBHEADER_CG_SHOP_SAFEBOX_OPEN,
		SUBHEADER_CG_SHOP_SAFEBOX_GET_ITEM,
		SUBHEADER_CG_SHOP_SAFEBOX_GET_VALUTES,
		SUBHEADER_CG_SHOP_SAFEBOX_CLOSE,

		SUBHEADER_CG_CLOSE_BOARD,
#ifdef ENABLE_OFFLINE_SHOP_CITIES
		SUBHEADER_CG_CLICK_ENTITY,
#endif
		SUBHEADER_CG_TELEPORT,
	};

	typedef struct
	{
		TShopInfo shop;
	}TSubPacketCGShopCreate;

	typedef struct
	{
		DWORD dwOwnerID;
	}TSubPacketCGShopOpen;

	typedef struct
	{
		TItemPos	pos;
		TPriceInfo  price;
	}TSubPacketCGAddItem;

	typedef struct
	{
		DWORD dwItemID;
	}TSubPacketCGRemoveItem;

	typedef struct
	{
		TFilterInfo filter;
	}TSubPacketCGFilterRequest;

	typedef struct
	{
		DWORD dwItemID;
	}TSubPacketCGShopSafeboxGetItem;

	typedef struct
	{
		TValutesInfo valutes;
	}TSubPacketCGShopSafeboxGetValutes;

	typedef struct
	{
		DWORD dwOwnerID;
		DWORD dwItemID;
		bool  bIsSearch;
		TPriceInfo Price;
	}TSubPacketCGShopBuyItem;

#ifdef ENABLE_OFFLINE_SHOP_CITIES
	typedef struct {
		DWORD dwShopVID;
	} TSubPacketCGShopClickEntity;
#endif
}
#endif

#ifdef ENABLE_IMPROVED_PACKET_ENCRYPTION
struct TPacketKeyAgreement
{
	static const int MAX_DATA_LEN = 256;
	BYTE bHeader;
	WORD wAgreedLength;
	WORD wDataLength;
	BYTE data[MAX_DATA_LEN];
};

struct TPacketKeyAgreementCompleted
{
	BYTE bHeader;
	BYTE data[3]; // dummy (not used)
};
#endif // ENABLE_IMPROVED_PACKET_ENCRYPTION

#ifdef ENABLE_FAST_CHEQUE_TRANSFER
enum EWonExchangeCGSubHeader
{
	WON_EXCHANGE_CG_SUBHEADER_SELL,
	WON_EXCHANGE_CG_SUBHEADER_BUY
};

typedef struct SPacketCGWonExchange
{
	SPacketCGWonExchange() :
		bHeader(HEADER_CG_WON_EXCHANGE)
	{}
	BYTE bHeader;
	BYTE bSubHeader;
	DWORD wValue;
} TPacketCGWonExchange;
#endif

#ifdef ENABLE_TARGET_BOARD_RENEWAL
enum EMobInformation
{
	MOB_INFORMATION_DROP,
	MOB_INFORMATION_BONUS,
	MOB_INFORMATION_SPECIALITY,
#ifdef ENABLE_DUNGEON_INFO_SYSTEM
	MOB_INFORMATION_DUNGEON_DROP,
#endif
	MOB_INFORMATION_MAX,
};

struct TMobInformationDropItem
{
	DWORD	vnum;
#ifdef ENABLE_ITEM_COUNT_LIMIT
	DWORD	count;
#else
	BYTE	count;
#endif
};

typedef struct SPacketCGMobInformation
{
	BYTE	header;
	DWORD	dwMobVnum;
	DWORD	dwVirtualID;
	BYTE	bType;
} TPacketCGMobInformation;

struct TMobInformationDrop
{
	DWORD	dwVID;
	DWORD	dwMobVnum;
	TMobInformationDropItem	item_list[MOB_INFO_ITEM_LIST_MAX];
};

typedef struct SPacketGCMobInformation
{
	BYTE	header;
	int	size;
	BYTE	subheader;
} TPacketGCMobInformation;
#endif

typedef struct SGameOption
{
	BYTE	header;
	BYTE	bOptionType;
	BYTE	bOptionValue;
} TGameOption;

typedef struct packet_ping
{
	BYTE		header;
} TPacketGCPing;

typedef struct packet_pong
{
	BYTE		bHeader;
} TPacketCGPong;

#ifdef ENABLE_PLAYER_STATISTICS
typedef struct command_player_statistics_packet
{
	BYTE bHeader;
	int	iKilledShinsoo;
	int	iKilledChunjo;
	int	iKilledJinno;
	int iTotalKill;
	int iDuelWon;
	int iDuelLost;
	int iKilledMonster;
	int	iKilledStone;
	int	iKilledBoss;
	int	iCompletedDungeon;
	int	iTakedFish;
	long long	iBestStoneDamage;
	long long	iBestBossDamage;
} TPacketGCPlayerStatistics;
#endif

#ifdef ENABLE_INGAME_MALL_SYSTEM
typedef struct SPacketItemShopData
{
	BYTE	header;
	DWORD	id, category, sub_category, vnum, count, coins, socketzero;
} TPacketItemShopData;
#endif // ENABLE_INGAME_MALL_SYSTEM

#ifdef ENABLE_BATTLE_PASS_SYSTEM
typedef struct SPacketGCBattlePass
{
	BYTE bHeader;
	WORD wSize;
	WORD wRewardSize;
} TPacketGCBattlePass;

typedef struct SPacketGCBattlePassUpdate
{
	BYTE bHeader;
	BYTE bMissionType;
	DWORD dwNewProgress;
} TPacketGCBattlePassUpdate;

typedef struct SPacketCGBattlePassAction
{
	BYTE bHeader;
	BYTE bAction;
	BYTE bSubAction;
} TPacketCGBattlePassAction;

typedef struct SPacketGCBattlePassRanking
{
	BYTE bHeader;
	WORD wSize;
	BYTE bIsGlobal;
} TPacketGCBattlePassRanking;
#endif

typedef struct packet_unk_213
{
	BYTE bHeader;
	BYTE bUnk2;
} TPacketGCUnk213;

#pragma pack(pop)





