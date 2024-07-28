#pragma once
#include "stdafx.h"

#define PACKET_CG_START	150
#define PACKET_GC_START	150

enum EPacketsClientToGame
{
	HEADER_CG_HANDSHAKE						= 0xff,
	HEADER_CG_TIME_SYNC						= 0xfc,
	HEADER_CG_PONG							= 0xfe,
	HEADER_CG_ATTACK						= 1,
	HEADER_CG_CHAT							= 2,
	HEADER_CG_CHARACTER_CREATE				= 3,
	HEADER_CG_CHARACTER_DELETE				= 4,
	HEADER_CG_CHARACTER_SELECT				= 5,
	HEADER_CG_MOVE							= 6,
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
	HEADER_CG_ITEM_GIVE						= 44,
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
	HEADER_CG_TEXT							= 58,

#ifdef __DROP_DIALOG__
	HEADER_CG_ITEM_DELETE					= PACKET_CG_START + 0,
	HEADER_CG_ITEM_SELL						= PACKET_CG_START + 1,
#endif
#ifdef __SKILL_CHOOSE_WINDOW__
	HEADER_CG_SKILLCHOOSE					= PACKET_CG_START + 2,
#endif
#ifdef __GLOBAL_MESSAGE_UTILITY__
	HEADER_CG_BULK_WHISPER					= PACKET_CG_START + 3,
#endif
#ifdef __CHEST_INFO_SYSTEM__
	HEADER_CG_CHEST_DROP_INFO				= PACKET_CG_START + 4,
#endif
#ifdef __OFFLINE_SHOP__
	HEADER_CG_NEW_OFFLINESHOP				= PACKET_CG_START + 5,
#endif
#ifdef __ANCIENT_ATTR_ITEM__
	HEADER_CG_ITEM_USE_NEW_ATTRIBUTE		= PACKET_CG_START + 6,
#endif
#ifdef __ITEM_CHANGELOOK__
	HEADER_CG_CL							= PACKET_CG_START + 7,
#endif
#ifdef __SKILL_COLOR__
	HEADER_CG_SKILL_COLOR					= PACKET_CG_START + 8,
#endif
#ifdef __DUNGEON_INFORMATION__
	HEADER_CG_TELEPORT						= PACKET_CG_START + 9,
#endif
#ifdef __SWITCHBOT__
	HEADER_CG_SWITCHBOT						= PACKET_CG_START + 10,
#endif
#ifdef __CUBE_WINDOW__
	HEADER_CG_CUBE_RENEWAL					= PACKET_CG_START + 11,
#endif
#ifdef __GROWTH_PET_SYSTEM__
	HEADER_CG_PetSetName					= PACKET_CG_START + 12,
#endif
#ifdef __FAST_CHEQUE_TRANSFER__
	HEADER_CG_WON_EXCHANGE					= PACKET_CG_START + 13,
#endif
#ifdef __ACCE_SYSTEM__
	HEADER_CG_ACCE							= PACKET_CG_START + 14,
#endif
#ifdef __TARGET_BOARD_RENEWAL__
	HEADER_CG_MOB_INFO						= PACKET_CG_START + 15,
#endif
#ifdef __GROWTH_MOUNT_SYSTEM__
	HEADER_CG_MountSetName					= PACKET_CG_START + 16,
#endif
#ifdef __BATTLE_PASS_SYSTEM__
	HEADER_CG_BATTLE_PASS					= PACKET_CG_START + 17,
#endif

	HEADER_CG_KEY_AGREEMENT					= 0xfb, // __IMPROVED_PACKET_ENCRYPTION__
};

enum EPacketsGameToClient
{
	/********************************************************/
	HEADER_GC_TIME_SYNC								= 0xfc,
	HEADER_GC_PHASE									= 0xfd,
	HEADER_GC_HANDSHAKE								= 0xff,

	HEADER_GC_CHARACTER_ADD							= 1,
	HEADER_GC_CHARACTER_DEL							= 2,
	HEADER_GC_MOVE									= 3,
	HEADER_GC_CHAT									= 4,
	HEADER_GC_SYNC_POSITION							= 5,
	HEADER_GC_LOGIN_SUCCESS							= 6,
	HEADER_GC_LOGIN_FAILURE							= 7,
	HEADER_GC_CHARACTER_CREATE_SUCCESS				= 8,
	HEADER_GC_CHARACTER_CREATE_FAILURE				= 9,
	HEADER_GC_CHARACTER_DELETE_SUCCESS				= 10,
	HEADER_GC_CHARACTER_DELETE_WRONG_SOCIAL_ID		= 11,
	HEADER_GC_STUN									= 12,
	HEADER_GC_DEAD									= 13,
	HEADER_GC_CHARACTER_POINTS						= 14,
	HEADER_GC_CHARACTER_POINT_CHANGE				= 15,
	HEADER_GC_CHARACTER_UPDATE						= 16,
	HEADER_GC_ITEM_DEL								= 17,
	HEADER_GC_ITEM_SET								= 18,
	HEADER_GC_ITEM_USE								= 19,
	HEADER_GC_ITEM_UPDATE							= 20,
	HEADER_GC_ITEM_GROUND_ADD						= 21,
	HEADER_GC_ITEM_GROUND_DEL						= 22,
	HEADER_GC_QUICKSLOT_ADD							= 23,
	HEADER_GC_QUICKSLOT_DEL							= 24,
	HEADER_GC_QUICKSLOT_SWAP						= 25,
	HEADER_GC_ITEM_OWNERSHIP						= 26,
	HEADER_GC_WHISPER								= 27,
	HEADER_GC_MOTION								= 28,
	HEADER_GC_SHOP									= 29,
	HEADER_GC_SHOP_SIGN								= 30,
	HEADER_GC_PVP									= 31,
	HEADER_GC_EXCHANGE								= 32,
	HEADER_GC_CHARACTER_POSITION					= 33,

	HEADER_GC_SCRIPT								= 34,
	HEADER_GC_QUEST_CONFIRM							= 35,
	HEADER_GC_OWNERSHIP								= 36,
	HEADER_GC_TARGET								= 37,
	HEADER_GC_WARP									= 38,
	HEADER_GC_ADD_FLY_TARGETING						= 39,
	HEADER_GC_CREATE_FLY							= 40,
	HEADER_GC_FLY_TARGETING							= 41,
	HEADER_GC_MESSENGER								= 42,
	HEADER_GC_GUILD									= 43,
	HEADER_GC_SKILL_LEVEL							= 44,
	HEADER_GC_PARTY_INVITE							= 45,
	HEADER_GC_PARTY_ADD								= 46,
	HEADER_GC_PARTY_UPDATE							= 47,
	HEADER_GC_PARTY_REMOVE							= 48,
	HEADER_GC_QUEST_INFO							= 49,
	HEADER_GC_REQUEST_MAKE_GUILD					= 50,
	HEADER_GC_PARTY_PARAMETER						= 51,
	HEADER_GC_SAFEBOX_SET							= 52,
	HEADER_GC_SAFEBOX_DEL							= 53,
	HEADER_GC_SAFEBOX_WRONG_PASSWORD				= 54,
	HEADER_GC_SAFEBOX_SIZE							= 55,
	HEADER_GC_FISHING								= 56,
	HEADER_GC_EMPIRE								= 57,
	HEADER_GC_PARTY_LINK							= 58,
	HEADER_GC_PARTY_UNLINK							= 59,
	HEADER_GC_MARK_BLOCK							= 60,
	HEADER_GC_MARK_IDXLIST							= 61,
	HEADER_GC_TIME									= 62,
	HEADER_GC_CHANGE_NAME							= 63,
	HEADER_GC_DUNGEON								= 64,
	HEADER_GC_WALK_MODE								= 65,
	HEADER_GC_SKILL_GROUP							= 66,
	HEADER_GC_MAIN_CHARACTER						= 67,
	HEADER_GC_SEPCIAL_EFFECT						= 68,
	HEADER_GC_REFINE_INFORMATION					= 69,
	HEADER_GC_CHANNEL								= 70,
	HEADER_GC_MALL_OPEN								= 71,
	HEADER_GC_TARGET_UPDATE							= 72,
	HEADER_GC_TARGET_DELETE							= 73,
	HEADER_GC_TARGET_CREATE							= 74,
	HEADER_GC_AFFECT_ADD							= 75,
	HEADER_GC_AFFECT_REMOVE							= 76,
	HEADER_GC_MALL_SET								= 77,
	HEADER_GC_MALL_DEL								= 78,
	HEADER_GC_DIG_MOTION							= 79,
	HEADER_GC_DAMAGE_INFO							= 80,
	HEADER_GC_CHAR_ADDITIONAL_INFO					= 81,
	HEADER_GC_AUTH_SUCCESS							= 82,
	HEADER_GC_SPECIFIC_EFFECT						= 83,
	HEADER_GC_DRAGON_SOUL_REFINE					= 84,
	HEADER_GC_RESPOND_CHANNELSTATUS					= 85,
	HEADER_GC_PING									= 86,

#ifdef __CHEST_INFO_SYSTEM__
	HEADER_GC_CHEST_DROP_INFO						= PACKET_GC_START + 1,
#endif
#ifdef __GLOBAL_MESSAGE_UTILITY__
	HEADER_GC_BULK_WHISPER							= PACKET_GC_START + 2,
#endif
#ifdef __EVENT_SYSTEM__
	HEADER_GC_EVENT_INFO							= PACKET_GC_START + 3,
#endif
#ifdef __SKILL_CHOOSE_WINDOW__
	HEADER_GC_SKILLCHOOSE							= PACKET_GC_START + 4,
#endif
#ifdef __ITEM_CHANGELOOK__
	HEADER_GC_CL									= PACKET_GC_START + 5,
#endif
#ifdef __SWITCHBOT__
	HEADER_GC_SWITCHBOT								= PACKET_GC_START + 6,
#endif
#ifdef __CUBE_WINDOW__
	HEADER_GC_CUBE_RENEWAL							= PACKET_GC_START + 7,
#endif
#ifdef __OFFLINE_SHOP__
	HEADER_GC_NEW_OFFLINESHOP						= PACKET_GC_START + 8,
#endif
#ifdef __ACCE_SYSTEM__
	HEADER_GC_ACCE									= PACKET_GC_START + 9,
#endif
#ifdef ENABLE_PLAYER_STATISTICS
	HEADER_GC_PLAYER_STATISTICS						= PACKET_GC_START + 10,
#endif
#ifdef __TARGET_BOARD_RENEWAL__
	HEADER_GC_MOB_INFO								= PACKET_GC_START + 12,
#endif
#ifdef __INGAME_MALL__
	HEADER_GC_ITEM_SHOP								= PACKET_GC_START + 13,
#endif // __INGAME_MALL__
#ifdef __BATTLE_PASS_SYSTEM__
	HEADER_GC_BATTLE_PASS_OPEN						= PACKET_GC_START + 14,
	HEADER_GC_BATTLE_PASS_UPDATE					= PACKET_GC_START + 15,
	HEADER_GC_BATTLE_PASS_RANKING					= PACKET_GC_START + 16,
#endif

	HEADER_GC_KEY_AGREEMENT_COMPLETED				= 0xfa, // __IMPROVED_PACKET_ENCRYPTION__
	HEADER_GC_KEY_AGREEMENT							= 0xfb, // __IMPROVED_PACKET_ENCRYPTION__
};

enum EPacketsGameToGame
{
	HEADER_GG_LOGIN = 1,
	HEADER_GG_LOGOUT = 2,
	HEADER_GG_RELAY = 3,
	HEADER_GG_NOTICE = 4,
	HEADER_GG_SHUTDOWN = 5,
	HEADER_GG_GUILD = 6,
	HEADER_GG_DISCONNECT = 7,
	HEADER_GG_SHOUT = 8,
	HEADER_GG_SETUP = 9,
	HEADER_GG_MESSENGER_ADD = 10,
	HEADER_GG_MESSENGER_REMOVE = 11,
	HEADER_GG_FIND_POSITION = 12,
	HEADER_GG_WARP_CHARACTER = 13,
	HEADER_GG_GUILD_WAR_ZONE_MAP_INDEX = 14,
	HEADER_GG_TRANSFER = 15,
	HEADER_GG_LOGIN_PING = 16,
	HEADER_GG_BLOCK_CHAT = 17,

#ifdef __FULL_NOTICE__
	HEADER_GG_BIG_NOTICE = 18,
#endif
#ifdef __RELOAD_REWORK__
	HEADER_GG_RELOAD_COMMAND = 19,
#endif
#ifdef __GLOBAL_MESSAGE_UTILITY__
	HEADER_GG_BULK_WHISPER = 20,
#endif
#ifdef __EVENT_SYSTEM__
	HEADER_GG_EVENT_TIME = 21,
#endif
#ifdef __SWITCHBOT__
	HEADER_GG_SWITCHBOT = 22,
#endif
#ifdef __CHAT_FILTER__
	HEADER_GG_NOTICE_IMPROVING = 23,
#endif
#ifdef ENABLE_MULTI_FARM_BLOCK
	HEADER_GG_MULTI_FARM = 28,
	HEADER_GG_MULTI_FARM_QF = 29,
#endif
};

#pragma pack(1)
typedef struct SPacketGGSetup
{
	BYTE	bHeader;
	WORD	wPort;
	BYTE	bChannel;
} TPacketGGSetup;

typedef struct SPacketGGLogin
{
	BYTE	bHeader;
	char	szName[CHARACTER_NAME_MAX_LEN + 1];
	DWORD	dwPID;
	BYTE	bEmpire;
	long	lMapIndex;
	BYTE	bChannel;
} TPacketGGLogin;

typedef struct SPacketGGLogout
{
	BYTE	bHeader;
	char	szName[CHARACTER_NAME_MAX_LEN + 1];
} TPacketGGLogout;

typedef struct SPacketGGRelay
{
	BYTE	bHeader;
	char	szName[CHARACTER_NAME_MAX_LEN + 1];
	long	lSize;
} TPacketGGRelay;

typedef struct SPacketGGNotice
{
	BYTE	bHeader;
	long	lSize;
} TPacketGGNotice;

typedef struct SPacketGGShutdown
{
	BYTE	bHeader;
} TPacketGGShutdown;

typedef struct SPacketGGGuild
{
	BYTE	bHeader;
	BYTE	bSubHeader;
	DWORD	dwGuild;
} TPacketGGGuild;

enum
{
	GUILD_SUBHEADER_GG_CHAT,
	GUILD_SUBHEADER_GG_SET_MEMBER_COUNT_BONUS,
};

typedef struct SPacketGGGuildChat
{
	BYTE	bHeader;
	BYTE	bSubHeader;
	DWORD	dwGuild;
	char	szText[CHAT_MAX_LEN + 1];
} TPacketGGGuildChat;

enum
{
	PARTY_SUBHEADER_GG_CREATE,
	PARTY_SUBHEADER_GG_DESTROY,
	PARTY_SUBHEADER_GG_JOIN,
	PARTY_SUBHEADER_GG_QUIT,
};

typedef struct SPacketGGDisconnect
{
	BYTE	bHeader;
	char	szLogin[CHARACTER_NAME_MAX_LEN + 1];
#ifdef __BAN_REASON_UTILITY__
	bool	bBanned;
#endif
} TPacketGGDisconnect;

typedef struct SPacketGGShout
{
	BYTE	bHeader;
	BYTE	bEmpire;
	char	szText[CHAT_MAX_LEN + 1];
} TPacketGGShout;

typedef struct SPacketGGMessenger
{
	BYTE        bHeader;
	char        szAccount[CHARACTER_NAME_MAX_LEN + 1];
	char        szCompanion[CHARACTER_NAME_MAX_LEN + 1];
} TPacketGGMessenger;

typedef struct SPacketGGFindPosition
{
	BYTE header;
	DWORD dwFromPID;
	DWORD dwTargetPID;
} TPacketGGFindPosition;

typedef struct SPacketGGWarpCharacter
{
	BYTE header;
	DWORD pid;
	long x;
	long y;
	long privateMapIndex;
#ifdef __WARP_WITH_CHANNEL__
	WORD port;
#endif
} TPacketGGWarpCharacter;

typedef struct SPacketGGGuildWarMapIndex
{
	BYTE bHeader;
	DWORD dwGuildID1;
	DWORD dwGuildID2;
	long lMapIndex;
} TPacketGGGuildWarMapIndex;

typedef struct SPacketGGTransfer
{
	BYTE	bHeader;
	char	szName[CHARACTER_NAME_MAX_LEN + 1];
	long	lX, lY;
#ifdef __WARP_WITH_CHANNEL__
	long	privateMapIndex;
	WORD	port;
#endif
} TPacketGGTransfer;

typedef struct SPacketGGBlockChat
{
	BYTE	bHeader;
	char	szName[CHARACTER_NAME_MAX_LEN + 1];
	long	lBlockDuration;
} TPacketGGBlockChat;

typedef struct command_text
{
	BYTE	bHeader;
} TPacketCGText;

typedef struct command_handshake
{
	BYTE	bHeader;
	DWORD	dwHandshake;
	DWORD	dwTime;
	long	lDelta;
} TPacketCGHandshake;

typedef struct command_login
{
	BYTE	header;
	char	login[LOGIN_MAX_LEN + 1];
	char	passwd[PASSWD_MAX_LEN + 1];
} TPacketCGLogin;

typedef struct command_login2
{
	BYTE	header;
	char	login[LOGIN_MAX_LEN + 1];
	DWORD	dwLoginKey;
	DWORD	adwClientKey[4];
} TPacketCGLogin2;

typedef struct command_login3
{
	BYTE	header;
	char	login[LOGIN_MAX_LEN + 1];
	char	passwd[PASSWD_MAX_LEN + 1];
	DWORD	adwClientKey[4];
#ifdef __CLIENT_VERSION_UTILITY__
	char	clientversion[60 + 1];
#endif
#ifdef __PIN_SECURITY__
	char	pin[PASSWD_MAX_LEN + 1];
#endif
#ifdef __HWID_SECURITY_UTILITY__
	char	cpu_id[CPU_ID_MAX_LEN + 1];
	char	hdd_model[HDD_MODEL_MAX_LEN + 1];
	char	machine_guid[MACHINE_GUID_MAX_LEN + 1];
	char	mac_addr[MAC_ADDR_MAX_LEN + 1];
	char	hdd_serial[HDD_SERIAL_MAX_LEN + 1];
	char	bios_id[BIOS_ID_MAX_LEN + 1];
	char	pc_name[PC_NAME_MAX_NUM + 1];
	char	user_name[USER_NAME_MAX_NUM + 1];
	char	os_version[OS_VERSION_MAX_NUM + 1];
	int		banned_val;
#endif
#ifdef USE_DISCORD_NETWORK
	char	discordAddress[DISCORD_ADDRESS_MAX_NUM + 1];
#endif // USE_DISCORD_NETWORK
} TPacketCGLogin3;

typedef struct command_player_select
{
	BYTE	header;
	BYTE	index;
} TPacketCGPlayerSelect;

typedef struct command_player_delete
{
	BYTE	header;
	BYTE	index;
	char	private_code[8];
} TPacketCGPlayerDelete;

typedef struct command_player_create
{
	BYTE	header;
	BYTE	index;
	char	name[CHARACTER_NAME_MAX_LEN + 1];
	WORD	job;
	BYTE	shape;
} TPacketCGPlayerCreate;

typedef struct command_player_create_success
{
	BYTE		header;
	BYTE		bAccountCharacterIndex;
	TSimplePlayer	player;
} TPacketGCPlayerCreateSuccess;

typedef struct command_attack
{
	BYTE	bHeader;
	BYTE	bType;
	DWORD	dwVID;
} TPacketCGAttack;

enum EMoveFuncType
{
	FUNC_WAIT,
	FUNC_MOVE,
	FUNC_ATTACK,
	FUNC_COMBO,
	FUNC_MOB_SKILL,
	_FUNC_SKILL,
	FUNC_MAX_NUM,
	FUNC_SKILL = 0x80,
};


typedef struct command_move
{
	BYTE	bHeader;
	BYTE	bFunc;
	BYTE	bArg;
	BYTE	bRot;
	long	lX;
	long	lY;
	DWORD	dwTime;
} TPacketCGMove;

typedef struct command_sync_position_element
{
	DWORD	dwVID;
	long	lX;
	long	lY;
} TPacketCGSyncPositionElement;

typedef struct command_sync_position
{
	BYTE	bHeader;
	WORD	wSize;
} TPacketCGSyncPosition;

typedef struct command_chat
{
	BYTE	header;
	WORD	size;
	BYTE	type;
} TPacketCGChat;

typedef struct command_whisper
{
	BYTE	bHeader;
	WORD	wSize;
	char 	szNameTo[CHARACTER_NAME_MAX_LEN + 1];
} TPacketCGWhisper;
typedef struct command_entergame
{
	BYTE	header;
} TPacketCGEnterGame;
typedef struct command_item_use
{
	BYTE 	header;
	TItemPos 	Cell;
#ifdef __USE_ITEM_COUNT__
	DWORD	count;
#endif // __USE_ITEM_COUNT__
} TPacketCGItemUse;

typedef struct command_item_use_to_item
{
	BYTE	header;
	TItemPos	Cell;
	TItemPos	TargetCell;
} TPacketCGItemUseToItem;

#ifdef __DROP_DIALOG__
typedef struct command_item_delete
{
	BYTE	header;
	TItemPos	Cell;
} TPacketCGItemDelete;

typedef struct command_item_sell
{
	BYTE		header;
	TItemPos	Cell;
} TPacketCGItemSell;
#endif

typedef struct command_item_move
{
	BYTE 	header;
	TItemPos	Cell;
	TItemPos	CellTo;
#ifdef __ITEM_COUNT_LIMIT__
	DWORD	count;
#else
	BYTE	count;
#endif
} TPacketCGItemMove;

typedef struct command_item_pickup
{
	BYTE 	header;
	DWORD	vid;
} TPacketCGItemPickup;

typedef struct command_quickslot_add
{
	BYTE	header;
	BYTE	pos;
	TQuickslot	slot;
} TPacketCGQuickslotAdd;

typedef struct command_quickslot_del
{
	BYTE	header;
	BYTE	pos;
} TPacketCGQuickslotDel;

typedef struct command_quickslot_swap
{
	BYTE	header;
	BYTE	pos;
	BYTE	change_pos;
} TPacketCGQuickslotSwap;

enum
{
	SHOP_SUBHEADER_CG_END,
	SHOP_SUBHEADER_CG_BUY,
	SHOP_SUBHEADER_CG_SELL,
	SHOP_SUBHEADER_CG_SELL2,
};

typedef struct command_shop
{
	BYTE	header;
	BYTE	subheader;
} TPacketCGShop;

typedef struct command_on_click
{
	BYTE	header;
	DWORD	vid;
} TPacketCGOnClick;

enum
{
	EXCHANGE_SUBHEADER_CG_START,	/* arg1 == vid of target character */
	EXCHANGE_SUBHEADER_CG_ITEM_ADD,	/* arg1 == position of item */
	EXCHANGE_SUBHEADER_CG_ITEM_DEL,	/* arg1 == position of item */
	EXCHANGE_SUBHEADER_CG_ELK_ADD,	/* arg1 == amount of gold */
#ifdef __CHEQUE_SYSTEM__
	EXCHANGE_SUBHEADER_CG_CHEQUE_ADD,	/* arg1 == amount of cheque */
#endif
	EXCHANGE_SUBHEADER_CG_ACCEPT,	/* arg1 == not used */
	EXCHANGE_SUBHEADER_CG_CANCEL,	/* arg1 == not used */
};

typedef struct command_exchange
{
	BYTE	header;
	BYTE	sub_header;
#ifdef __GOLD_LIMIT_REWORK__
	long long	arg1;
#else
	DWORD	arg1;
#endif
	BYTE	arg2;
	TItemPos	Pos;
} TPacketCGExchange;

typedef struct command_position
{
	BYTE	header;
	BYTE	position;
} TPacketCGPosition;

typedef struct command_script_answer
{
	BYTE	header;
	BYTE	answer;
} TPacketCGScriptAnswer;

typedef struct command_script_button
{
	BYTE        header;
	unsigned int	idx;
} TPacketCGScriptButton;

typedef struct command_quest_input_string
{
	BYTE header;
	char msg[64 + 1];
} TPacketCGQuestInputString;

typedef struct command_quest_confirm
{
	BYTE header;
	BYTE answer;
	DWORD requestPID;
} TPacketCGQuestConfirm;

typedef struct packet_quest_confirm
{
	BYTE header;
	char msg[64 + 1];
	long timeout;
	DWORD requestPID;
} TPacketGCQuestConfirm;

typedef struct packet_handshake
{
	BYTE	bHeader;
	DWORD	dwHandshake;
	DWORD	dwTime;
	long	lDelta;
} TPacketGCHandshake;

enum EPhase
{
	PHASE_CLOSE,
	PHASE_HANDSHAKE,
	PHASE_LOGIN,
	PHASE_SELECT,
	PHASE_LOADING,
	PHASE_GAME,
	PHASE_DEAD,

	PHASE_CLIENT_CONNECTING,
	PHASE_DBCLIENT,
	PHASE_P2P,
	PHASE_AUTH,
};

typedef struct packet_phase
{
	BYTE	header;
	BYTE	phase;
} TPacketGCPhase;

enum
{
	LOGIN_FAILURE_ALREADY = 1,
	LOGIN_FAILURE_ID_NOT_EXIST = 2,
	LOGIN_FAILURE_WRONG_PASS = 3,
	LOGIN_FAILURE_FALSE = 4,
	LOGIN_FAILURE_NOT_TESTOR = 5,
	LOGIN_FAILURE_NOT_TEST_TIME = 6,
	LOGIN_FAILURE_FULL = 7
};

typedef struct packet_login_success
{
	BYTE				bHeader;
	TSimplePlayer		players[PLAYER_PER_ACCOUNT];
	DWORD				guild_id[PLAYER_PER_ACCOUNT];
	char				guild_name[PLAYER_PER_ACCOUNT][GUILD_NAME_MAX_LEN + 1];
	DWORD				handle;
	DWORD				random_key;
} TPacketGCLoginSuccess;

typedef struct packet_auth_success
{
	BYTE	bHeader;
	DWORD	dwLoginKey;
	BYTE	bResult;
} TPacketGCAuthSuccess;

typedef struct packet_login_failure
{
	BYTE	header;
	char	szStatus[ACCOUNT_STATUS_MAX_LEN + 1];
#ifdef __BAN_REASON_UTILITY__
	DWORD	availDate;
	char	szBanWebLink[BAN_WEB_LINK_MAX_LEN + 1];
#endif
} TPacketGCLoginFailure;

typedef struct packet_create_failure
{
	BYTE	header;
	BYTE	bType;
} TPacketGCCreateFailure;

enum
{
	ADD_CHARACTER_STATE_DEAD = (1 << 0),
	ADD_CHARACTER_STATE_SPAWN = (1 << 1),
	ADD_CHARACTER_STATE_GUNGON = (1 << 2),
	ADD_CHARACTER_STATE_KILLER = (1 << 3),
	ADD_CHARACTER_STATE_PARTY = (1 << 4),
};

typedef struct packet_add_char
{
	BYTE	header;
	DWORD	dwVID;
#if defined(__MOB_AGGR_LVL_INFO__)
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

#ifdef __SHINING_ITEM_SYSTEM__
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

enum ECharacterEquipmentPart
{
	CHR_EQUIPPART_ARMOR,
	CHR_EQUIPPART_WEAPON,
	CHR_EQUIPPART_HEAD,
	CHR_EQUIPPART_HAIR,
#ifdef __ACCE_SYSTEM__
	CHR_EQUIPPART_ACCE,
	CHR_EQUIPPART_ACCE_EFFECT,
#endif
#ifdef __AURA_COSTUME_SYSTEM__
	CHR_EQUIPPART_AURA,
#endif
#ifdef __QUIVER_SYSTEM__
	CHR_EQUIPPART_ARROW,
#endif
	CHR_EQUIPPART_NUM,
};

#ifdef __ITEM_EVOLUTION__
enum EItemEvolution
{
	EVOLUTION_WEAPON,
#ifdef __ARMOR_EVOLUTION__
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
	DWORD   dwGuildID;
	BYTE	bLevel;
	BYTE	bPKMode;
	DWORD	dwMountVnum;
#ifdef __ALIGNMENT_REWORK__
	int		alignment;
#else
	short	alignment;
#endif
#ifdef __RANK_SYSTEM__
	short	rank;
#endif
#ifdef __LANDRANK_SYSTEM__
	short	land_rank;
#endif
#ifdef __REBORN_SYSTEM__
	short	reborn;
#endif
#ifdef __TEAM_SYSTEM__
	short	team;
#endif
#ifdef __MONIKER_SYSTEM__
	char moniker[MONIKER_MAX_LEN + 1];
#endif
#ifdef ENABLE_LOVE_SYSTEM
	char love1[2];
	char love2[2];
#endif // ENABLE_LOVE_SYSTEM
#ifdef ENABLE_WORD_SYSTEM
	char word[24+1];
#endif // ENABLE_WORD_SYSTEM
#ifdef __GUILD_LEADER_SYSTEM__
	BYTE	guild_member_type;
#endif
#ifdef __ITEM_EVOLUTION__
	WORD	itemEvolution[EVOLUTION_MAX];
#endif
#ifdef __SKILL_COLOR__
	DWORD	dwSkillColor[ESkillColorLength::MAX_SKILL_COUNT + ESkillColorLength::MAX_BUFF_COUNT][ESkillColorLength::MAX_EFFECT_COUNT];
#endif
#ifdef __SHINING_ITEM_SYSTEM__
	DWORD	adwShining[CHR_SHINING_NUM];
#endif
} TPacketGCCharacterAdditionalInfo;

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
#ifdef __ALIGNMENT_REWORK__
	int		alignment;
#else
	short	alignment;
#endif
#ifdef __RANK_SYSTEM__
	short	rank;
#endif
#ifdef __LANDRANK_SYSTEM__
	short	land_rank;
#endif
#ifdef __REBORN_SYSTEM__
	short	reborn;
#endif
#ifdef __TEAM_SYSTEM__
	short	team;
#endif
#ifdef __MONIKER_SYSTEM__
	char moniker[MONIKER_MAX_LEN];
#endif
#ifdef ENABLE_LOVE_SYSTEM
	char love1[2];
	char love2[2];
#endif // ENABLE_LOVE_SYSTEM
#ifdef ENABLE_WORD_SYSTEM
	char word[24+1];
#endif // ENABLE_WORD_SYSTEM
#ifdef __GUILD_LEADER_SYSTEM__
	BYTE	guild_member_type;
#endif
#ifdef __ITEM_EVOLUTION__
	WORD	itemEvolution[EVOLUTION_MAX];
#endif
#ifdef __SKILL_COLOR__
	DWORD	dwSkillColor[ESkillColorLength::MAX_SKILL_COUNT + ESkillColorLength::MAX_BUFF_COUNT][ESkillColorLength::MAX_EFFECT_COUNT];
#endif
#ifdef __SHINING_ITEM_SYSTEM__
	DWORD	adwShining[CHR_SHINING_NUM];
#endif
#ifdef ENABLE_AUTO_HUNT_SYSTEM
	DWORD	hasAttacker;
#endif // ENABLE_AUTO_HUNT_SYSTEM
} TPacketGCCharacterUpdate;

typedef struct packet_del_char
{
	BYTE	header;
	DWORD	id;
} TPacketGCCharacterDelete;

typedef struct packet_chat
{
	BYTE	header;
	WORD	size;
	BYTE	type;
	DWORD	id;
	BYTE	bEmpire;
} TPacketGCChat;

typedef struct packet_whisper
{
	BYTE	bHeader;
	WORD	wSize;
	BYTE	bType;
	char	szNameFrom[CHARACTER_NAME_MAX_LEN + 1];
} TPacketGCWhisper;

typedef struct packet_main_character
{
	enum
	{
		MUSIC_NAME_LEN = 24,
	};

	BYTE	header;
	DWORD	dwVID;
	WORD	wRaceNum;
	char	szChrName[CHARACTER_NAME_MAX_LEN + 1];
	char	szBGMName[MUSIC_NAME_LEN + 1];
	DWORD	dwPlayerID;
	float	fBGMVol;
	long	lx, ly, lz;
	BYTE	empire;
	BYTE	skill_group;
} TPacketGCMainCharacter;

typedef struct packet_points
{
	BYTE	header;
#ifdef __GOLD_LIMIT_REWORK__
	long long	points[POINT_MAX_NUM];
#else
	INT		points[POINT_MAX_NUM];
#endif
} TPacketGCPoints;

typedef struct packet_skill_level
{
	BYTE		bHeader;
	TPlayerSkill	skills[SKILL_MAX_NUM];
} TPacketGCSkillLevel;

typedef struct packet_point_change
{
	int		header;
	DWORD	dwVID;
	BYTE	type;
#ifdef __GOLD_LIMIT_REWORK__
	long long	amount;
	long long	value;
#else
	long	amount;
	long	value;
#endif
} TPacketGCPointChange;

typedef struct packet_stun
{
	BYTE	header;
	DWORD	vid;
} TPacketGCStun;

typedef struct packet_dead
{
	BYTE	header;
	DWORD	vid;
} TPacketGCDead;

typedef struct packet_item_deldeprecated
{
	BYTE		header;
	TItemPos	Cell;
	DWORD		vnum;
#ifdef __ITEM_COUNT_LIMIT__
	DWORD	count;
#else
	BYTE	count;
#endif // __ITEM_COUNT_LIMIT__
	long		alSockets[ITEM_SOCKET_MAX_NUM];
	TPlayerItemAttribute aAttr[ITEM_ATTRIBUTE_MAX_NUM];
#ifdef __BEGINNER_ITEM__
	bool	is_basic;
#endif
#ifdef __ITEM_EVOLUTION__
	DWORD	evolution;
#endif
#ifdef __ITEM_CHANGELOOK__
	DWORD	transmutation;
#endif
} TPacketGCItemDelDeprecated;

typedef struct packet_item_set
{
	BYTE	header;
	TItemPos Cell;
	DWORD	vnum;
#ifdef __ITEM_COUNT_LIMIT__
	DWORD	count;
#else
	BYTE	count;
#endif
	DWORD	flags;
	DWORD	anti_flags;
	bool	highlight;
	long	alSockets[ITEM_SOCKET_MAX_NUM];
	TPlayerItemAttribute aAttr[ITEM_ATTRIBUTE_MAX_NUM];
#ifdef __BEGINNER_ITEM__
	bool	is_basic;
#endif
#ifdef __ITEM_EVOLUTION__
	DWORD	evolution;
#endif
#ifdef __ITEM_CHANGELOOK__
	DWORD	transmutation;
#endif
} TPacketGCItemSet;

typedef struct packet_item_del
{
	BYTE	header;
	BYTE	pos;
} TPacketGCItemDel;

struct packet_item_use
{
	BYTE	header;
	TItemPos Cell;
	DWORD	ch_vid;
	DWORD	victim_vid;
	DWORD	vnum;
};

struct packet_item_move
{
	BYTE	header;
	TItemPos Cell;
	TItemPos CellTo;
};

typedef struct packet_item_update
{
	BYTE	header;
	TItemPos Cell;
#ifdef __ITEM_COUNT_LIMIT__
	DWORD	count;
#else
	BYTE	count;
#endif
	long	alSockets[ITEM_SOCKET_MAX_NUM];
	TPlayerItemAttribute aAttr[ITEM_ATTRIBUTE_MAX_NUM];
#ifdef __BEGINNER_ITEM__
	bool	is_basic;
#endif
#ifdef __ITEM_EVOLUTION__
	DWORD	evolution;
#endif
#ifdef __ITEM_CHANGELOOK__
	DWORD	transmutation;
#endif
} TPacketGCItemUpdate;

typedef struct packet_item_ground_add
{
	BYTE	bHeader;
	long 	x, y, z;
	DWORD	dwVID;
	DWORD	dwVnum;
#ifdef __ITEM_ENTITY_UTILITY__
	long	alSockets[ITEM_SOCKET_MAX_NUM];
	DWORD	wCount;
#endif

#ifdef __ITEM_ENTITY_UTILITY__
	packet_item_ground_add()
	{
		memset(&alSockets, 0, sizeof(alSockets));
	}
#endif
} TPacketGCItemGroundAdd;

typedef struct packet_item_ownership
{
	BYTE	bHeader;
	DWORD	dwVID;
	char	szName[CHARACTER_NAME_MAX_LEN + 1];
} TPacketGCItemOwnership;

typedef struct packet_item_ground_del
{
	BYTE	bHeader;
	DWORD	dwVID;
} TPacketGCItemGroundDel;

struct packet_quickslot_add
{
	BYTE	header;
	BYTE	pos;
	TQuickslot	slot;
};

struct packet_quickslot_del
{
	BYTE	header;
	BYTE	pos;
};

struct packet_quickslot_swap
{
	BYTE	header;
	BYTE	pos;
	BYTE	pos_to;
};

struct packet_motion
{
	BYTE	header;
	DWORD	vid;
	DWORD	victim_vid;
	WORD	motion;
};

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
#ifdef __SHOP_PRICE_TYPE_ITEM__
	SHOP_SUBHEADER_GC_NOT_ENOUGH_ITEM,
#endif
#ifdef __CHEQUE_SYSTEM__
	SHOP_SUBHEADER_GC_NOT_ENOUGH_CHEQUE,
	SHOP_SUBHEADER_GC_NOT_ENOUGH_CHEQUE_MONEY,
#endif
};

struct packet_shop_item
{
	DWORD		vnum;
#ifdef __GOLD_LIMIT_REWORK__
	long long	price;
#else
	DWORD		price;
#endif
#ifdef __CHEQUE_SYSTEM__
	DWORD		cheque;
#endif
#ifdef __ITEM_COUNT_LIMIT__
	DWORD		count;
#else
	BYTE		count;
#endif
	BYTE		display_pos;
	long	alSockets[ITEM_SOCKET_MAX_NUM];
	TPlayerItemAttribute aAttr[ITEM_ATTRIBUTE_MAX_NUM];
#ifdef __SHOP_PRICE_TYPE_ITEM__
	DWORD		witemVnum;
#endif
#ifdef __ITEM_EVOLUTION__
	DWORD	evolution;
#endif
#ifdef __ITEM_CHANGELOOK__
	DWORD	transmutation;
#endif
};

typedef struct packet_shop_start
{
	DWORD   owner_vid;
	struct packet_shop_item	items[SHOP_HOST_ITEM_MAX_NUM];
} TPacketGCShopStart;

typedef struct packet_shop_update_item
{
	BYTE			pos;
	struct packet_shop_item	item;
} TPacketGCShopUpdateItem;

typedef struct packet_shop_update_price
{
#ifdef __GOLD_LIMIT_REWORK__
	long long		llPrice;
#else
	int				iPrice;
#endif
#ifdef __CHEQUE_SYSTEM__
	int				iCheque;
#endif
} TPacketGCShopUpdatePrice;

typedef struct packet_shop
{
	BYTE        header;
	WORD	size;
	BYTE        subheader;
} TPacketGCShop;

struct packet_exchange
{
	BYTE	header;
	BYTE	sub_header;
	BYTE	is_me;
#ifdef __GOLD_LIMIT_REWORK__
	long long	arg1;
#else
	DWORD	arg1;	// vnum
#endif
	TItemPos	arg2;	// cell
	DWORD	arg3;	// count
	long	alSockets[ITEM_SOCKET_MAX_NUM];
	TPlayerItemAttribute aAttr[ITEM_ATTRIBUTE_MAX_NUM];
#ifdef __ITEM_EVOLUTION__
	DWORD	evolution;
#endif
#ifdef __ITEM_CHANGELOOK__
	DWORD	dwTransmutation;
#endif
};

enum EPacketTradeSubHeaders
{
	EXCHANGE_SUBHEADER_GC_START,	/* arg1 == vid */
	EXCHANGE_SUBHEADER_GC_ITEM_ADD,	/* arg1 == vnum  arg2 == pos  arg3 == count */
	EXCHANGE_SUBHEADER_GC_ITEM_DEL,
	EXCHANGE_SUBHEADER_GC_GOLD_ADD,	/* arg1 == gold */
	EXCHANGE_SUBHEADER_GC_ACCEPT,	/* arg1 == accept */
	EXCHANGE_SUBHEADER_GC_END,		/* arg1 == not used */
	EXCHANGE_SUBHEADER_GC_ALREADY,	/* arg1 == not used */
	EXCHANGE_SUBHEADER_GC_LESS_GOLD,	/* arg1 == not used */
#ifdef __CHEQUE_SYSTEM__
	EXCHANGE_SUBHEADER_GC_CHEQUE_ADD,
	EXCHANGE_SUBHEADER_GC_LESS_CHEQUE,
#endif
};

struct packet_position
{
	BYTE	header;
	DWORD	vid;
	BYTE	position;
};

struct packet_script
{
	BYTE	header;
	WORD	size;
	BYTE	skin;
	WORD	src_size;
};

typedef struct packet_move
{
	BYTE		bHeader;
	BYTE		bFunc;
	BYTE		bArg;
	BYTE		bRot;
	DWORD		dwVID;
	long		lX;
	long		lY;
	DWORD		dwTime;
	DWORD		dwDuration;
} TPacketGCMove;

typedef struct packet_ownership
{
	BYTE		bHeader;
	DWORD		dwOwnerVID;
	DWORD		dwVictimVID;
} TPacketGCOwnership;

typedef struct packet_sync_position
{
	BYTE	bHeader;
	WORD	wSize;
} TPacketGCSyncPosition;

typedef struct packet_fly
{
	BYTE	bHeader;
	BYTE	bType;
	DWORD	dwStartVID;
	DWORD	dwEndVID;
} TPacketGCCreateFly;

typedef struct command_fly_targeting
{
	BYTE		bHeader;
	DWORD		dwTargetVID;
	long		x, y;
} TPacketCGFlyTargeting;

typedef struct packet_fly_targeting
{
	BYTE		bHeader;
	DWORD		dwShooterVID;
	DWORD		dwTargetVID;
	long		x, y;
} TPacketGCFlyTargeting;

typedef struct packet_shoot
{
	BYTE		bHeader;
	BYTE		bType;
} TPacketCGShoot;

typedef struct packet_duel_start
{
	BYTE	header;
	WORD	wSize;
} TPacketGCDuelStart;

enum EPVPModes
{
	PVP_MODE_NONE,
	PVP_MODE_AGREE,
	PVP_MODE_FIGHT,
	PVP_MODE_REVENGE
};

typedef struct packet_pvp
{
	BYTE        bHeader;
	DWORD       dwVIDSrc;
	DWORD       dwVIDDst;
	BYTE        bMode;
} TPacketGCPVP;

typedef struct command_use_skill
{
	BYTE	bHeader;
	DWORD	dwVnum;
	DWORD	dwVID;
} TPacketCGUseSkill;

typedef struct command_target
{
	BYTE	header;
	DWORD	dwVID;
} TPacketCGTarget;

typedef struct packet_target
{
	BYTE	header;
	DWORD	dwVID;
	BYTE	bHPPercent;
#ifdef __TARGET_BOARD_RENEWAL__
	HPTYPE	dwHP;
	HPTYPE dwMaxHP;
#endif

	packet_target()
	{
		dwVID = 0;
		bHPPercent = 0;
#ifdef __TARGET_BOARD_RENEWAL__
		dwHP = 0;
		dwMaxHP = 0;
#endif
	};
} TPacketGCTarget;

typedef struct packet_warp
{
	BYTE	bHeader;
	long	lX;
	long	lY;
	long	lAddr;
	WORD	wPort;
} TPacketGCWarp;

struct packet_quest_info
{
	BYTE header;
	WORD size;
	WORD index;
#ifdef __QUEST_CATEGORY__
	WORD c_index;
#endif
	BYTE flag;
};

enum
{
	MESSENGER_SUBHEADER_GC_LIST,
	MESSENGER_SUBHEADER_GC_LOGIN,
	MESSENGER_SUBHEADER_GC_LOGOUT,
	MESSENGER_SUBHEADER_GC_REMOVE_FRIEND, // @duzenleme karsi taraf messenger uzerinden silindiginde paket gondermemiz icin eklendi.
	MESSENGER_SUBHEADER_GC_INVITE,
};

typedef struct packet_messenger
{
	BYTE header;
	WORD size;
	BYTE subheader;
} TPacketGCMessenger;

typedef struct packet_messenger_guild_list
{
	BYTE connected;
	BYTE length;
} TPacketGCMessengerGuildList;

typedef struct packet_messenger_guild_login
{
	BYTE length;
} TPacketGCMessengerGuildLogin;

typedef struct packet_messenger_guild_logout
{
	BYTE length;
} TPacketGCMessengerGuildLogout;

typedef struct packet_messenger_list_offline
{
	BYTE connected; // always 0
	BYTE length;
} TPacketGCMessengerListOffline;

typedef struct packet_messenger_list_online
{
	BYTE connected; // always 1
	BYTE length;
} TPacketGCMessengerListOnline;

enum EMessengerSubheaderClientGame
{
	MESSENGER_SUBHEADER_CG_ADD_BY_VID,
	MESSENGER_SUBHEADER_CG_ADD_BY_NAME,
	MESSENGER_SUBHEADER_CG_REMOVE,
	MESSENGER_SUBHEADER_CG_INVITE_ANSWER,
};

typedef struct command_messenger
{
	BYTE header;
	BYTE subheader;
} TPacketCGMessenger;

typedef struct command_messenger_add_by_vid
{
	DWORD vid;
} TPacketCGMessengerAddByVID;

typedef struct command_messenger_add_by_name
{
	BYTE length;
} TPacketCGMessengerAddByName;

typedef struct command_messenger_remove
{
	char login[LOGIN_MAX_LEN + 1];
} TPacketCGMessengerRemove;

typedef struct command_safebox_checkout
{
	BYTE	bHeader;
	BYTE	bSafePos;
	TItemPos	ItemPos;
} TPacketCGSafeboxCheckout;

typedef struct command_safebox_checkin
{
	BYTE	bHeader;
	BYTE	bSafePos;
	TItemPos	ItemPos;
} TPacketCGSafeboxCheckin;

typedef struct command_party_parameter
{
	BYTE	bHeader;
	BYTE	bDistributeMode;
} TPacketCGPartyParameter;

typedef struct paryt_parameter
{
	BYTE	bHeader;
	BYTE	bDistributeMode;
} TPacketGCPartyParameter;

typedef struct packet_party_add
{
	BYTE	header;
	DWORD	pid;
	char	name[CHARACTER_NAME_MAX_LEN + 1];
} TPacketGCPartyAdd;

typedef struct command_party_invite
{
	BYTE	header;
	DWORD	vid;
} TPacketCGPartyInvite;

typedef struct packet_party_invite
{
	BYTE	header;
	DWORD	leader_vid;
} TPacketGCPartyInvite;

typedef struct command_party_invite_answer
{
	BYTE	header;
	DWORD	leader_vid;
	BYTE	accept;
} TPacketCGPartyInviteAnswer;

typedef struct packet_party_update
{
	BYTE	header;
	DWORD	pid;
	BYTE	role;
	BYTE	percent_hp;
#ifdef __PARTY_ROLE_REWORK__
	short	affects[10];
#else
	short	affects[7];
#endif
} TPacketGCPartyUpdate;

typedef struct packet_party_remove
{
	BYTE header;
	DWORD pid;
} TPacketGCPartyRemove;

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

typedef struct command_party_remove
{
	BYTE header;
	DWORD pid;
} TPacketCGPartyRemove;

typedef struct command_party_set_state
{
	BYTE header;
	DWORD pid;
	BYTE byRole;
	BYTE flag;
} TPacketCGPartySetState;

typedef struct packet_safebox_size
{
	BYTE bHeader;
	BYTE bSize;
} TPacketCGSafeboxSize;

typedef struct packet_safebox_wrong_password
{
	BYTE	bHeader;
} TPacketCGSafeboxWrongPassword;

typedef struct command_empire
{
	BYTE	bHeader;
	BYTE	bEmpire;
} TPacketCGEmpire;

typedef struct packet_empire
{
	BYTE	bHeader;
	BYTE	bEmpire;
} TPacketGCEmpire;

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
	GUILD_SUBHEADER_GC_WAR_SCORE,
};

enum GUILD_SUBHEADER_CG
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
};

typedef struct packet_guild
{
	BYTE header;
	WORD size;
	BYTE subheader;
} TPacketGCGuild;

typedef struct packet_guild_name_t
{
	BYTE header;
	WORD size;
	BYTE subheader;
	DWORD	guildID;
	char	guildName[GUILD_NAME_MAX_LEN];
} TPacketGCGuildName;

typedef struct packet_guild_war
{
	DWORD	dwGuildSelf;
	DWORD	dwGuildOpp;
	BYTE	bType;
	BYTE 	bWarState;
#ifdef __GUILD_WAR_REWORK__
	int		iMaxPlayer;
	int		iMaxScore;
#endif
} TPacketGCGuildWar;

typedef struct command_guild
{
	BYTE header;
	BYTE subheader;
} TPacketCGGuild;

typedef struct command_guild_answer_make_guild
{
	BYTE header;
	char guild_name[GUILD_NAME_MAX_LEN + 1];
} TPacketCGAnswerMakeGuild;

typedef struct command_guild_use_skill
{
	DWORD	dwVnum;
	DWORD	dwPID;
} TPacketCGGuildUseSkill;

typedef struct command_mark_login
{
	BYTE    header;
	DWORD   handle;
	DWORD   random_key;
} TPacketCGMarkLogin;

typedef struct command_mark_upload
{
	BYTE	header;
	DWORD	gid;
	BYTE	image[16 * 12 * 4];
} TPacketCGMarkUpload;

typedef struct command_mark_idxlist
{
	BYTE	header;
} TPacketCGMarkIDXList;

typedef struct command_mark_crclist
{
	BYTE	header;
	BYTE	imgIdx;
	DWORD	crclist[80];
} TPacketCGMarkCRCList;

typedef struct packet_mark_idxlist
{
	BYTE    header;
	DWORD	bufSize;
	WORD	count;
} TPacketGCMarkIDXList;

typedef struct packet_mark_block
{
	BYTE	header;
	DWORD	bufSize;
	BYTE	imgIdx;
	DWORD	count;
} TPacketGCMarkBlock;

typedef struct command_fishing
{
	BYTE header;
	BYTE dir;
} TPacketCGFishing;

typedef struct packet_fishing
{
	BYTE header;
	BYTE subheader;
	DWORD info;
	BYTE dir;
} TPacketGCFishing;

enum
{
	FISHING_SUBHEADER_GC_START,
	FISHING_SUBHEADER_GC_STOP,
	FISHING_SUBHEADER_GC_REACT,
	FISHING_SUBHEADER_GC_SUCCESS,
	FISHING_SUBHEADER_GC_FAIL,
	FISHING_SUBHEADER_GC_FISH,
};

typedef struct command_give_item
{
	BYTE byHeader;
	DWORD dwTargetVID;
	TItemPos ItemPos;
#ifdef __ITEM_COUNT_LIMIT__
	DWORD byItemCount;
#else
	BYTE byItemCount;
#endif
} TPacketCGGiveItem;

typedef struct SPacketCGHack
{
	BYTE	bHeader;
	char	szBuf[255 + 1];
} TPacketCGHack;

enum
{
	DUNGEON_SUBHEADER_GC_TIME_ATTACK_START = 0,
	DUNGEON_SUBHEADER_GC_DESTINATION_POSITION = 1,
};

typedef struct packet_dungeon
{
	BYTE bHeader;
	WORD size;
	BYTE subheader;
} TPacketGCDungeon;

typedef struct packet_dungeon_dest_position
{
	long x;
	long y;
} TPacketGCDungeonDestPosition;

typedef struct SPacketGCShopSign
{
	BYTE	bHeader;
	DWORD	dwVID;
	char	szSign[SHOP_SIGN_MAX_LEN + 1];
} TPacketGCShopSign;

typedef struct SPacketCGMyShop
{
	BYTE	bHeader;
	char	szSign[SHOP_SIGN_MAX_LEN + 1];
	BYTE	bCount;
} TPacketCGMyShop;

typedef struct SPacketGCTime
{
	BYTE	bHeader;
	time_t	time;
} TPacketGCTime;

enum
{
	WALKMODE_RUN,
	WALKMODE_WALK,
};

typedef struct SPacketGCWalkMode
{
	BYTE	header;
	DWORD	vid;
	BYTE	mode;
} TPacketGCWalkMode;

typedef struct SPacketGCChangeSkillGroup
{
	BYTE        header;
	BYTE        skill_group;
} TPacketGCChangeSkillGroup;

typedef struct SPacketCGRefine
{
	BYTE	header;
	BYTE	pos;
	BYTE	type;
} TPacketCGRefine;

typedef struct SPacketCGRequestRefineInfo
{
	BYTE	header;
	BYTE	pos;
} TPacketCGRequestRefineInfo;

typedef struct SPacketGCRefineInformaion
{
	BYTE	header;
	BYTE	type;
	BYTE	pos;
	DWORD	src_vnum;
	DWORD	result_vnum;
#ifdef __ITEM_COUNT_LIMIT__
	DWORD	material_count;
#else
	BYTE	material_count;
#endif
#ifdef __GOLD_LIMIT_REWORK__
	long long	cost;
#else
	int		cost;
#endif
	int		prob;
#ifdef __REFINE_REWORK__
	int		success_prob;
#endif
	TRefineMaterial materials[REFINE_MATERIAL_MAX_NUM];
} TPacketGCRefineInformation;

typedef struct SPacketGCSpecialEffect
{
	BYTE header;
	BYTE type;
	DWORD vid;
} TPacketGCSpecialEffect;

typedef struct SPacketCGChangeName
{
	BYTE header;
	BYTE index;
	char name[CHARACTER_NAME_MAX_LEN + 1];
} TPacketCGChangeName;

typedef struct SPacketGCChangeName
{
	BYTE header;
	DWORD pid;
	char name[CHARACTER_NAME_MAX_LEN + 1];
} TPacketGCChangeName;

typedef struct packet_channel
{
	BYTE header;
	BYTE channel;
} TPacketGCChannel;

typedef struct
{
	BYTE	bHeader;
	long	lID;
	char	szName[32 + 1];
	DWORD	dwVID;
	BYTE	bType;
} TPacketGCTargetCreate;

typedef struct
{
	BYTE	bHeader;
	long	lID;
	long	lX, lY;
} TPacketGCTargetUpdate;

typedef struct
{
	BYTE	bHeader;
	long	lID;
} TPacketGCTargetDelete;

typedef struct
{
	BYTE		bHeader;
	bool		bByLoad;
	TPacketAffectElement elem;
} TPacketGCAffectAdd;

typedef struct
{
	BYTE	bHeader;
	DWORD	dwType;
	BYTE	bApplyOn;
} TPacketGCAffectRemove;

typedef struct packet_dig_motion
{
	BYTE header;
	DWORD vid;
	DWORD target_vid;
	BYTE count;
} TPacketGCDigMotion;

typedef struct command_script_select_item
{
	BYTE header;
	DWORD selection;
} TPacketCGScriptSelectItem;

typedef struct packet_damage_info
{
	BYTE header;
	DWORD dwVID;
	BYTE flag;
#ifdef __DAMAGE_LIMIT_REWORK__
	long long damage;
#else
	int damage;
#endif
} TPacketGCDamageInfo;

#define MAX_EFFECT_FILE_NAME 128
typedef struct SPacketGCSpecificEffect
{
	BYTE header;
	DWORD vid;
	char effect_file[MAX_EFFECT_FILE_NAME];
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
	DS_SUB_HEADER_DO_REFINE_GRADE,
	DS_SUB_HEADER_DO_REFINE_STEP,
	DS_SUB_HEADER_DO_REFINE_STRENGTH,
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
	TItemPos ItemGrid[DRAGON_SOUL_REFINE_GRID_SIZE];
} TPacketCGDragonSoulRefine;

typedef struct SPacketGCDragonSoulRefine
{
	SPacketGCDragonSoulRefine() : header(HEADER_GC_DRAGON_SOUL_REFINE)
	{}
	BYTE header;
	BYTE bSubType;
	TItemPos Pos;
} TPacketGCDragonSoulRefine;

typedef struct SPacketCGStateCheck
{
	BYTE header;
	unsigned long key;
	unsigned long index;
} TPacketCGStateCheck;

#ifdef __OFFLINE_SHOP__
//ACTIONS PACKETS
typedef struct
{
	BYTE bHeader;
#ifdef __LARGE_DYNAMIC_PACKET__
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
	//common
	typedef struct
	{
		BYTE		bType;
		BYTE		bSubType;
		WORD		wNameMode;
		char		szName[ITEM_NAME_MAX_LEN];
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
#ifdef __OFFLINE_SHOP_ENTITY_CITIES__
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
#ifdef __BEGINNER_ITEM__
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
#ifdef __CHEQUE_SYSTEM__
		SUBERROR_GC_SHOP_CHEQUE_LIMIT_MAX,
#endif
		SUBERROR_GC_SHOP_GOLD_LIMIT_MIN,
#ifdef __CHEQUE_SYSTEM__
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
		DWORD dwOwnerID;
		DWORD dwItemID;
	}TSubPacketGCShopBuyItemFromSearch;

	typedef struct {
		DWORD dwCount;
	} TSubPacketGCShopFilterResult;

	typedef struct {
		TValutesInfo	valute;
		DWORD			dwItemCount;
	}TSubPacketGCShopSafeboxRefresh;

#ifdef __OFFLINE_SHOP_ENTITY_CITIES__
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
#ifdef __OFFLINE_SHOP_ENTITY_CITIES__
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

#ifdef __OFFLINE_SHOP_ENTITY_CITIES__
	typedef struct {
		DWORD dwShopVID;
	} TSubPacketCGShopClickEntity;
#endif
}
#endif

#ifdef __ACCE_SYSTEM__
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
#ifdef __GOLD_LIMIT_REWORK__
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
#endif

#ifdef __ANCIENT_ATTR_ITEM__
typedef struct command_item_new_attribute
{
	BYTE  header;
	TItemPos source_pos;
	TItemPos target_pos;
	BYTE bValues[3 + 1];
} TPacketCGItemNewAttribute;
#endif

#ifdef __CHEST_INFO_SYSTEM__
typedef struct SPacketCGChestDropInfo
{
	BYTE	header;
	DWORD	dwChestVnum;
	TItemPos	pos;
} TPacketCGChestDropInfo;

typedef struct SChestDropInfoTable
{
	BYTE	bPageIndex;
	WORD	bSlotIndex;
	DWORD	dwItemVnum;
#ifdef __ITEM_COUNT_LIMIT__
	DWORD	bItemCount;
#else
	BYTE	bItemCount;
#endif
} TChestDropInfoTable;

typedef struct SPacketGCChestDropInfo
{
	BYTE	bHeader;
	int	wSize;
	DWORD	dwChestVnum;
	TItemPos	pos;
} TPacketGCChestDropInfo;
#endif

#ifdef __RELOAD_REWORK__
typedef struct SPacketGGReloadCommand
{
	BYTE	header;
	char	argument[200];
} TPacketGGReloadCommand;
#endif

#ifdef __GLOBAL_MESSAGE_UTILITY__
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

typedef struct SPacketGGBulkWhisper
{
	BYTE	bHeader;
	long	lSize;
} TPacketGGBulkWhisper;
#endif

#ifdef __GROWTH_PET_SYSTEM__
typedef struct packet_RequestPetName
{
	BYTE byHeader;
	char petname[13];
}TPacketCGRequestPetName;
#endif

#ifdef __EVENT_SYSTEM__
typedef struct packet_event_info
{
	BYTE		bHeader;
	BYTE		event_id;
	bool		isActivate;
	DWORD		event_time;
} TPacketGCEventInfo;
#endif

#ifdef __SKILL_CHOOSE_WINDOW__
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

#ifdef __ITEM_CHANGELOOK__
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
#ifdef __GOLD_LIMIT_REWORK__
	long long	llCost;
#else
	DWORD	dwCost;
#endif
	BYTE	bPos;
	TItemPos	tPos;
#ifdef __MOUNT_CHANGELOOK__
	mutable bool	bMount;
#endif
} TPacketChangeLook;
#endif

#ifdef __EVENT_SYSTEM__
typedef struct SPacketGGEventInfo
{
	BYTE	header;
	BYTE	event_id;
	DWORD event_time;
} TPacketGGEventInfo;
#endif

#ifdef __SKILL_COLOR__
typedef struct packet_skill_color
{
	BYTE	bheader;
	BYTE	skill;
	DWORD	col1;
	DWORD	col2;
	DWORD	col3;
	DWORD	col4;
	DWORD	col5;
} TPacketCGSkillColor;
#endif

#ifdef __DUNGEON_INFORMATION__
typedef struct command_send_teleport
{
	BYTE	header;
	BYTE	dungeonIndex;
}TPacketGCTeleport;
#endif

#ifdef __SWITCHBOT__
struct TPacketGGSwitchbot
{
	BYTE bHeader;
	WORD wPort;
	TSwitchbotTable table;

	TPacketGGSwitchbot() : bHeader(HEADER_GG_SWITCHBOT), wPort(0)
	{
		table = {};
	}
};

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
#ifdef __ITEM_COUNT_LIMIT__
	DWORD	count;
#else
	BYTE	count;
#endif
	long	alSockets[ITEM_SOCKET_MAX_NUM];
	TPlayerItemAttribute aAttr[ITEM_ATTRIBUTE_MAX_NUM];
};
#endif

#ifdef __CUBE_WINDOW__
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
#ifdef __GOLD_LIMIT_REWORK__
	long long	gold;
#else
	int 	gold;
#endif
#ifdef __GEM_SYSTEM__
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

#ifdef __IMPROVED_PACKET_ENCRYPTION__
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
#endif // __IMPROVED_PACKET_ENCRYPTION__

#ifdef __FAST_CHEQUE_TRANSFER__
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

#ifdef __TARGET_BOARD_RENEWAL__
enum EMobInformation
{
	MOB_INFORMATION_DROP,
	MOB_INFORMATION_BONUS,
	MOB_INFORMATION_SPECIALITY,
#ifdef __DUNGEON_INFORMATION__
	MOB_INFORMATION_DUNGEON_DROP,
#endif
	MOB_INFORMATION_MAX,
};

struct TMobInformationDropItem
{
	DWORD	vnum;
#ifdef __ITEM_COUNT_LIMIT__
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

typedef struct SPacketGGLoginPing
{
	BYTE	bHeader;
	char	szLogin[LOGIN_MAX_LEN + 1];
} TPacketGGLoginPing;

typedef struct packet_ping
{
	BYTE	header;
} TPacketGCPing;

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
#ifdef __INGAME_MALL__
typedef struct SPacketItemShopData
{
	BYTE	header;
	DWORD	id, category, sub_category, vnum, count, coins, socketzero;
} TPacketItemShopData;
#endif // __INGAME_MALL__

#ifdef __BATTLE_PASS_SYSTEM__
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

#ifdef ENABLE_MULTI_FARM_BLOCK
enum
{
	MULTI_FARM_SET,
	MULTI_FARM_REMOVE,
};

typedef struct SPacketGGMultiFarm
{
	uint8_t header;
	uint32_t size;
	uint8_t subHeader;
	char playerIP[OS_VERSION_MAX_NUM + 1];
	uint32_t playerID;
	char playerName[CHARACTER_NAME_MAX_LEN + 1];
	bool farmStatus;
	uint8_t affectType;
	int affectTime;
} TPacketGGMultiFarm;

typedef struct SPacketGGMultiFarmQuestFlag
{
	uint8_t header;
	char playerIP[OS_VERSION_MAX_NUM + 1];
	char szFlag[QUEST_NAME_MAX_LEN + 1 + QUEST_STATE_MAX_LEN + 1];
	DWORD value;
} TPacketGGMultiFarmQuestFlag;
#endif


#pragma pack()