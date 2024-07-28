#include "stdafx.h"
#include "constants.h"
#include "config.h"
#include "utils.h"
#include "desc_client.h"
#include "desc_manager.h"
#include "buffer_manager.h"
#include "packet.h"
#include "protocol.h"
#include "char.h"
#include "char_manager.h"
#include "item.h"
#include "item_manager.h"
#include "cmd.h"
#include "shop.h"
#include "shop_manager.h"
#include "safebox.h"
#include "regen.h"
#include "battle.h"
#include "exchange.h"
#include "questmanager.h"
#include "messenger_manager.h"
#include "party.h"
#include "p2p.h"
#include "affect.h"
#include "guild.h"
#include "guild_manager.h"
#include "log.h"
#include "unique_item.h"
#include "locale_service.h"
#include "gm.h"
#include "motion.h"
#include "locale_service.h"
#include "DragonSoul.h"
#include "belt_inventory_helper.h"
#include "input.h"
#include "refine.h"
#ifdef __GROWTH_PET_SYSTEM__
#include "New_PetSystem.h"
#endif
#ifdef __GROWTH_MOUNT_SYSTEM__
#include "New_MountSystem.h"
#endif
#ifdef __DUNGEON_INFORMATION__
#include "dungeon_info.h"
#endif
#ifdef __SWITCHBOT__
#include "new_switchbot.h"
#endif
#ifdef __TARGET_BOARD_RENEWAL__
#include "mob_manager.h"
#endif

#ifdef __MARTY_CHAT_LOGGING__
static char	__escape_string[1024];
static char	__escape_string2[1024];
#endif
#ifdef __BATTLE_PASS_SYSTEM__
#include "battle_pass.h"
#endif

#include "game_events.h"


void SendBlockChatInfo(LPCHARACTER ch, int sec)
{
	if (sec <= 0)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Ã¤ÆÃ ±ÝÁö »óÅÂÀÔ´Ï´Ù."));
		return;
	}

	long hour = sec / 3600;
	sec -= hour * 3600;

	long min = (sec / 60);
	sec -= min * 60;

	char buf[128 + 1];

	if (hour > 0 && min > 0)
		snprintf(buf, sizeof(buf), LC_TEXT("%d ½Ã°£ %d ºÐ %d ÃÊ µ¿¾È Ã¤ÆÃ±ÝÁö »óÅÂÀÔ´Ï´Ù"), hour, min, sec);
	else if (hour > 0 && min == 0)
		snprintf(buf, sizeof(buf), LC_TEXT("%d ½Ã°£ %d ÃÊ µ¿¾È Ã¤ÆÃ±ÝÁö »óÅÂÀÔ´Ï´Ù"), hour, sec);
	else if (hour == 0 && min > 0)
		snprintf(buf, sizeof(buf), LC_TEXT("%d ºÐ %d ÃÊ µ¿¾È Ã¤ÆÃ±ÝÁö »óÅÂÀÔ´Ï´Ù"), min, sec);
	else
		snprintf(buf, sizeof(buf), LC_TEXT("%d ÃÊ µ¿¾È Ã¤ÆÃ±ÝÁö »óÅÂÀÔ´Ï´Ù"), sec);

	ch->ChatPacket(CHAT_TYPE_INFO, buf);
}

enum
{
	TEXT_TAG_PLAIN,
	TEXT_TAG_TAG, // ||
	TEXT_TAG_COLOR, // |cffffffff
	TEXT_TAG_HYPERLINK_START, // |H
	TEXT_TAG_HYPERLINK_END, // |h ex) |Hitem:1234:1:1:1|h
	TEXT_TAG_RESTORE_COLOR,
#ifdef ENABLE_EMOJI_UTILITY
	TEXT_TAG_EMOJI_START, // |E
	TEXT_TAG_EMOJI_END, // |e ex) |Epath/filename|h?
#endif
};

int GetTextTag(const char* src, int maxLen, int& tagLen, std::string& extraInfo)
{
	tagLen = 1;

	if (maxLen < 2 || *src != '|')
		return TEXT_TAG_PLAIN;

	const char* cur = ++src;

	if (*cur == '|')
	{
		tagLen = 2;
		return TEXT_TAG_TAG;
	}
	else if (*cur == 'c') // color |cffffffffblahblah|r
	{
		tagLen = 2;
		return TEXT_TAG_COLOR;
	}
	else if (*cur == 'H')
	{
		tagLen = 2;
		return TEXT_TAG_HYPERLINK_START;
	}
	else if (*cur == 'h') // end of hyperlink
	{
		tagLen = 2;
		return TEXT_TAG_HYPERLINK_END;
	}
#ifdef ENABLE_EMOJI_UTILITY
	else if (*cur == L'E') // emoji |Epath/emo|e
	{
		tagLen = 2;
		return TEXT_TAG_EMOJI_START;
	}
	else if (*cur == L'e') // end of emoji
	{
		tagLen = 2;
		return TEXT_TAG_EMOJI_END;
	}
#endif

	return TEXT_TAG_PLAIN;
}

#ifdef ENABLE_EMOJI_UTILITY
void GetTextTagInfo(const char* src, int src_len, int& hyperlinks, bool& colored, bool& emoji)
#else
void GetTextTagInfo(const char* src, int src_len, int& hyperlinks, bool& colored)
#endif // ENABLE_EMOJI_UTILITY
{
	colored = false;
	hyperlinks = 0;
#ifdef ENABLE_EMOJI_UTILITY
	emoji = false;
#endif // ENABLE_EMOJI_UTILITY

	int len;
	std::string extraInfo;

	for (int i = 0; i < src_len;)
	{
		int tag = GetTextTag(&src[i], src_len - i, len, extraInfo);

		if (tag == TEXT_TAG_HYPERLINK_START)
			++hyperlinks;

		if (tag == TEXT_TAG_COLOR)
			colored = true;

#ifdef ENABLE_EMOJI_UTILITY
		if (tag == TEXT_TAG_EMOJI_START)
			emoji = true;
#endif // ENABLE_EMOJI_UTILITY

		i += len;
	}
}

int ProcessTextTag(LPCHARACTER ch, const char* c_pszText, size_t len)
{
	int hyperlinks;
	bool colored;
	bool emoji;

#ifdef ENABLE_EMOJI_UTILITY
	GetTextTagInfo(c_pszText, len, hyperlinks, colored, emoji);
#else
	GetTextTagInfo(c_pszText, len, hyperlinks, colored);
#endif // ENABLE_EMOJI_UTILITY

	if (g_bDisablePrismNeed)
		return 0;

	if (colored == true && hyperlinks == 0 && emoji == 0)
		return 4;

	int nPrismCount = ch->CountSpecifyItem(ITEM_PRISM);

	if (nPrismCount < hyperlinks)
		return 1;

	if (!ch->GetMyShop())
	{
		ch->RemoveSpecifyItem(ITEM_PRISM, hyperlinks);
		return 0;
	}
	else
	{
		int sellingNumber = ch->GetMyShop()->GetNumberByVnum(ITEM_PRISM);
		if (nPrismCount - sellingNumber < hyperlinks)
		{
			return 2;
		}
		else
		{
			ch->RemoveSpecifyItem(ITEM_PRISM, hyperlinks);
			return 0;
		}
	}

	return 4;
}

int CInputMain::Whisper(LPCHARACTER ch, const char* data, size_t uiBytes)
{
	const TPacketCGWhisper* pinfo = reinterpret_cast<const TPacketCGWhisper*>(data);

	if (uiBytes < pinfo->wSize)
		return -1;

	int iExtraLen = pinfo->wSize - sizeof(TPacketCGWhisper);

	if (iExtraLen < 0)
	{
		sys_err("invalid packet length (len %d size %u buffer %u)", iExtraLen, pinfo->wSize, uiBytes);
		ch->GetDesc()->SetPhase(PHASE_CLOSE);
		return -1;
	}

	if (ch->FindAffect(AFFECT_BLOCK_CHAT))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Ã¤ÆÃ ±ÝÁö »óÅÂÀÔ´Ï´Ù."));
		return (iExtraLen);
	}

#ifdef __MARTY_WHISPER_CHAT_SPAMLIMIT__
	if (ch->IncreaseChatCounter() >= 10)
	{
		ch->GetDesc()->DelayedDisconnect(0);
		return (iExtraLen);
	}
#endif

	LPCHARACTER pkChr = CHARACTER_MANAGER::instance().FindPC(pinfo->szNameTo);

	if (pkChr == ch)
		return (iExtraLen);

	LPDESC pkDesc = NULL;

	//BYTE bOpponentEmpire = 0;

	if (test_server)
	{
		if (!pkChr)
			sys_log(0, "Whisper to %s(%s) from %s", "Null", pinfo->szNameTo, ch->GetName());
		else
			sys_log(0, "Whisper to %s(%s) from %s", pkChr->GetName(), pinfo->szNameTo, ch->GetName());
	}

	if (ch->IsBlockMode(BLOCK_WHISPER))
	{
		if (ch->GetDesc())
		{
			TPacketGCWhisper pack;
			pack.bHeader = HEADER_GC_WHISPER;
			pack.bType = WHISPER_TYPE_SENDER_BLOCKED;
			pack.wSize = sizeof(TPacketGCWhisper);
			strlcpy(pack.szNameFrom, pinfo->szNameTo, sizeof(pack.szNameFrom));
			ch->GetDesc()->Packet(&pack, sizeof(pack));
		}
		return iExtraLen;
	}

	if (!pkChr)
	{
		CCI* pkCCI = P2P_MANAGER::instance().Find(pinfo->szNameTo);

		if (pkCCI)
		{
			pkDesc = pkCCI->pkDesc;
			if (pkDesc)
				pkDesc->SetRelay(pinfo->szNameTo);
			//	bOpponentEmpire = pkCCI->bEmpire;

			if (test_server)
				sys_log(0, "Whisper to %s from %s (Channel %d Mapindex %d)", "Null", ch->GetName(), pkCCI->bChannel, pkCCI->lMapIndex);
		}

	}
	else
	{
		pkDesc = pkChr->GetDesc();
		//		bOpponentEmpire = pkChr->GetEmpire();
	}

	if (!pkDesc)
	{
		if (ch->GetDesc())
		{
			TPacketGCWhisper pack;

			pack.bHeader = HEADER_GC_WHISPER;
			pack.bType = WHISPER_TYPE_NOT_EXIST;
			pack.wSize = sizeof(TPacketGCWhisper);
			strlcpy(pack.szNameFrom, pinfo->szNameTo, sizeof(pack.szNameFrom));
			ch->GetDesc()->Packet(&pack, sizeof(TPacketGCWhisper));
			sys_log(0, "WHISPER: no player");
		}
	}
	else
	{
		if (ch->IsBlockMode(BLOCK_WHISPER))
		{
			if (ch->GetDesc())
			{
				TPacketGCWhisper pack;
				pack.bHeader = HEADER_GC_WHISPER;
				pack.bType = WHISPER_TYPE_SENDER_BLOCKED;
				pack.wSize = sizeof(TPacketGCWhisper);
				strlcpy(pack.szNameFrom, pinfo->szNameTo, sizeof(pack.szNameFrom));
				ch->GetDesc()->Packet(&pack, sizeof(pack));
			}
		}
		else if (pkChr && pkChr->IsBlockMode(BLOCK_WHISPER))
		{
			if (ch->GetDesc())
			{
				TPacketGCWhisper pack;
				pack.bHeader = HEADER_GC_WHISPER;
				pack.bType = WHISPER_TYPE_TARGET_BLOCKED;
				pack.wSize = sizeof(TPacketGCWhisper);
				strlcpy(pack.szNameFrom, pinfo->szNameTo, sizeof(pack.szNameFrom));
				ch->GetDesc()->Packet(&pack, sizeof(pack));
			}
		}
		else
		{
			BYTE bType = WHISPER_TYPE_NORMAL;

			char buf[CHAT_MAX_LEN + 1];
			strlcpy(buf, data + sizeof(TPacketCGWhisper), MIN(iExtraLen + 1, sizeof(buf)));
			const size_t buflen = strlen(buf);

			int processReturn = ProcessTextTag(ch, buf, buflen);
			if (0 != processReturn)
			{
				if (ch->GetDesc())
				{
					TItemTable* pTable = ITEM_MANAGER::instance().GetTable(ITEM_PRISM);

					if (pTable)
					{
						char buf[128];
						int len;

						if (len < 0 || len >= (int)sizeof(buf))
							len = sizeof(buf) - 1;

						if (3 == processReturn)
							len = snprintf(buf, sizeof(buf), LC_TEXT("%s ¾ÆÀÌÅÛÀÌ ÇÊ¿äÇÕ´Ï´Ù"), pTable->szLocaleName);
						else
							ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%s ¾ÆÀÌÅÛÀÌ ÇÊ¿äÇÕ´Ï´Ù"), pTable->szLocaleName);

						++len;

						TPacketGCWhisper pack;

						pack.bHeader = HEADER_GC_WHISPER;
						pack.bType = WHISPER_TYPE_ERROR;
						pack.wSize = sizeof(TPacketGCWhisper) + len;
						strlcpy(pack.szNameFrom, pinfo->szNameTo, sizeof(pack.szNameFrom));

						ch->GetDesc()->BufferedPacket(&pack, sizeof(pack));
						ch->GetDesc()->Packet(buf, len);

						sys_log(0, "WHISPER: not enough %s: char: %s", pTable->szLocaleName, ch->GetName());
					}
				}

				pkDesc->SetRelay("");
				return (iExtraLen);
			}

			if (ch->IsGM())
				bType = (bType & 0xF0) | WHISPER_TYPE_GM;

			if (buflen > 0)
			{
				TPacketGCWhisper pack;

				pack.bHeader = HEADER_GC_WHISPER;
				pack.wSize = sizeof(TPacketGCWhisper) + buflen;
				pack.bType = bType;
				strlcpy(pack.szNameFrom, ch->GetName(), sizeof(pack.szNameFrom));

				TEMP_BUFFER tmpbuf;

				tmpbuf.write(&pack, sizeof(pack));
				tmpbuf.write(buf, buflen);

				pkDesc->Packet(tmpbuf.read_peek(), tmpbuf.size());

				// @warme006
				// sys_log(0, "WHISPER: %s -> %s : %s", ch->GetName(), pinfo->szNameTo, buf);
#ifdef __MARTY_CHAT_LOGGING__
				if (ch->IsGM())
				{
					LogManager::instance().EscapeString(__escape_string, sizeof(__escape_string), buf, buflen);
					LogManager::instance().EscapeString(__escape_string2, sizeof(__escape_string2), pinfo->szNameTo, sizeof(pack.szNameFrom));
					LogManager::instance().ChatLog(ch->GetMapIndex(), ch->GetPlayerID(), ch->GetName(), 0, __escape_string2, "WHISPER", __escape_string, ch->GetDesc() ? ch->GetDesc()->GetHostName() : "");
				}
#endif
			}
		}
	}
	if (pkDesc)
		pkDesc->SetRelay("");

	return (iExtraLen);
}

struct RawPacketToCharacterFunc
{
	const void* m_buf;
	int	m_buf_len;

	RawPacketToCharacterFunc(const void* buf, int buf_len) : m_buf(buf), m_buf_len(buf_len)
	{
	}

	void operator () (LPCHARACTER c)
	{
		if (!c->GetDesc())
			return;

		c->GetDesc()->Packet(m_buf, m_buf_len);
	}
};

struct FEmpireChatPacket
{
	packet_chat& p;
	const char* orig_msg;
	int orig_len;
	char converted_msg[CHAT_MAX_LEN + 1];

	BYTE bEmpire;
	int iMapIndex;
	int namelen;

	FEmpireChatPacket(packet_chat& p, const char* chat_msg, int len, BYTE bEmpire, int iMapIndex, int iNameLen)
		: p(p), orig_msg(chat_msg), orig_len(len), bEmpire(bEmpire), iMapIndex(iMapIndex), namelen(iNameLen)
	{
		memset(converted_msg, 0, sizeof(converted_msg));
	}

	void operator () (LPDESC d)
	{
		if (!d->GetCharacter())
			return;

		if (d->GetCharacter()->GetMapIndex() != iMapIndex)
			return;

		d->BufferedPacket(&p, sizeof(packet_chat));

		if (d->GetEmpire() == bEmpire ||
			bEmpire == 0 ||
			d->GetCharacter()->GetGMLevel() > GM_PLAYER ||
			d->GetCharacter()->IsEquipUniqueGroup(UNIQUE_GROUP_RING_OF_LANGUAGE))
		{
			d->Packet(orig_msg, orig_len);
		}
		else
		{
			size_t len = strlcpy(converted_msg, orig_msg, sizeof(converted_msg));

			if (len >= sizeof(converted_msg))
				len = sizeof(converted_msg) - 1;

			d->Packet(converted_msg, orig_len);
		}
	}
};

#ifdef __GROWTH_PET_SYSTEM__
void CInputMain::BraveRequestPetName(LPCHARACTER ch, const char* c_pData)
{
	if (!ch->GetDesc()) { return; }
	int vid = ch->GetEggVid();
	if (vid == 0) { return; }

	TPacketCGRequestPetName* p = (TPacketCGRequestPetName*)c_pData;

	if (ch->GetGold() < 100000) {
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[Pet-Kulucka] 100.000 Yang gerekir"));
		return;
	}

	if (!check_name(p->petname))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[Pet-Kulucka] Hatali isim girdiniz"));
		return;
	}

	if (quest::CQuestManager::instance().GetPCForce(ch->GetPlayerID())->IsRunning() == true)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_OPERATION_IS_FAILED_QUEST_WINDOW_IS_OPENED"));
		return;
	}

	if (!ch->CanAct(true, true, VAR_TIME_PET_ACTION, nullptr, 0))
		return;

	if (ch->CountSpecifyItem(vid) > 0) {
		ch->PointChange(POINT_GOLD, -100000, true);
		ch->RemoveSpecifyItem(vid, 1);
		LPITEM item = ch->AutoGiveItem(vid + 300, 1);
		if (!item)
			return;
		int tmpslot = number(3, 3);
		int tmpskill[3] = { 0, 0, 0 };
		for (int i = 0; i < 3; ++i)
		{
			if (i > tmpslot - 1)
				tmpskill[i] = -1;
		}

		int pet_type = 0;
		int totaldur = 365 * 24 * 60 * 60;
		int tmpdur = time(0) + totaldur;
		int tmpagedur = time(0) - 86400;

		char szQuery1[QUERY_MAX_LEN];
		snprintf(szQuery1, sizeof(szQuery1), "INSERT INTO new_petsystem VALUES(%u, '%s', 1, 0, 0, 0, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d)", item->GetID(), p->petname, number(1, 30), number(1, 30), number(1, 30), tmpskill[0], 0, tmpskill[1], 0, tmpskill[2], 0, tmpdur, totaldur, tmpagedur, pet_type, number(1, 5));
		std::unique_ptr<SQLMsg> pmsg2(DBManager::instance().DirectQuery(szQuery1));
	}
}
#endif

#ifdef __GROWTH_MOUNT_SYSTEM__
void CInputMain::BraveRequestMountName(LPCHARACTER ch, const char* c_pData)
{
	if (!ch->GetDesc()) { return; }
	int vid = ch->GetMountEggVid();
	if (vid == 0) { return; }

	TPacketCGRequestPetName* p = (TPacketCGRequestPetName*)c_pData;

	if (ch->GetGold() < 100000) {
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[Mount-Kulucka] 100.000 Yang gerekir"));
		return;
	}

	if (!check_name(p->petname))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[Mount-Kulucka] Hatali isim girdiniz"));
		return;
	}

	if (quest::CQuestManager::instance().GetPCForce(ch->GetPlayerID())->IsRunning() == true)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_OPERATION_IS_FAILED_QUEST_WINDOW_IS_OPENED"));
		return;
	}

	if (!ch->CanAct(true, true, VAR_TIME_MOUNT_ACTION))
		return;

	if (ch->CountSpecifyItem(vid) > 0) {
		ch->PointChange(POINT_GOLD, -100000, true);
		ch->RemoveSpecifyItem(vid, 1);
		LPITEM item = ch->AutoGiveItem(vid + 700, 1);
		if (!item)
			return;
		int tmpslot = number(3, 3);
		int tmpskill[3] = { 0, 0, 0 };
		for (int i = 0; i < 3; ++i)
		{
			if (i > tmpslot - 1)
				tmpskill[i] = -1;
		}

		int mount_type = 0;
		int totaldur = 365 * 24 * 60 * 60;
		int tmpdur = time(0) + totaldur;
		int tmpagedur = time(0) - 86400;

		char szQuery1[QUERY_MAX_LEN];
		snprintf(szQuery1, sizeof(szQuery1), "INSERT INTO new_mountsystem VALUES(%u, '%s', 1, 0, 0, 0, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d)", item->GetID(), p->petname, number(1, 30), number(1, 30), number(1, 30), tmpskill[0], 0, tmpskill[1], 0, tmpskill[2], 0, tmpdur, totaldur, tmpagedur, mount_type, number(1, 5));
		std::unique_ptr<SQLMsg> pmsg2(DBManager::instance().DirectQuery(szQuery1));
	}
}
#endif

int CInputMain::Chat(LPCHARACTER ch, const char* data, size_t uiBytes)
{
	const TPacketCGChat* pinfo = reinterpret_cast<const TPacketCGChat*>(data);

	if (uiBytes < pinfo->size)
		return -1;

	const int iExtraLen = pinfo->size - sizeof(TPacketCGChat);

	if (iExtraLen < 0)
	{
		sys_err("invalid packet length (len %d size %u buffer %u)", iExtraLen, pinfo->size, uiBytes);
		ch->GetDesc()->SetPhase(PHASE_CLOSE);
		return -1;
	}

	char buf[CHAT_MAX_LEN - (CHARACTER_NAME_MAX_LEN + 3) + 1];
	strlcpy(buf, data + sizeof(TPacketCGChat), MIN(iExtraLen + 1, sizeof(buf)));
	const size_t buflen = strlen(buf);

	if (buflen > 1 && *buf == '/')
	{
		interpret_command(ch, buf + 1, buflen - 1);
		return iExtraLen;
	}

	if (ch->IncreaseChatCounter() >= 10)
	{
		if (ch->GetChatCounter() == 10)
		{
			sys_log(0, "CHAT_HACK: %s", ch->GetName());
			ch->GetDesc()->DelayedDisconnect(5);
		}

		return iExtraLen;
	}

	const CAffect* pAffect = ch->FindAffect(AFFECT_BLOCK_CHAT);

	if (pAffect != NULL)
	{
		SendBlockChatInfo(ch, pAffect->lDuration);
		return iExtraLen;
	}

	// @bilinmeyenduzenleme ne sikime yariyor bilmiyoruz.
	int processReturn = ProcessTextTag(ch, buf, buflen);
	if (0 != processReturn)
	{
		const TItemTable* pTable = ITEM_MANAGER::instance().GetTable(ITEM_PRISM);

		if (NULL != pTable)
		{
			if (3 == processReturn)
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("»ç¿ëÇÒ¼ö ¾ø½À´Ï´Ù."), pTable->szLocaleName);
			else
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%sÀÌ ÇÊ¿äÇÕ´Ï´Ù."), pTable->szLocaleName);
		}

		return iExtraLen;
	}

	int hyperlinks;
	bool colored;
	bool emoji;
#ifdef ENABLE_EMOJI_UTILITY
	GetTextTagInfo(buf, buflen, hyperlinks, colored, emoji);
#else
	GetTextTagInfo(buf, buflen, hyperlinks, colored);
#endif // ENABLE_EMOJI_UTILITY

#ifdef ENABLE_EMOJI_UTILITY
	if (emoji == true && !ch->FindAffect(AFFECT_PB2_GLOBAL_CHAT))
		return iExtraLen;
#endif // ENABLE_EMOJI_UTILITY

#ifdef ENABLE_CHAT_COLOR_SYSTEM
	char const* color;

	if (ch->GetChatColor() == 1)
		color = "0080FF";
	else if (ch->GetChatColor() == 2) 
		color = "FF0000";
	else if (ch->GetChatColor() == 3)
		color = "FFFF00";
	else if (ch->GetChatColor() == 4)
		color = "00FF00";
	else if (ch->GetChatColor() == 5)
		color = "FFA500";
	else if (ch->GetChatColor() == 6)
		color = "40E0D0";
	else if (ch->GetChatColor() == 7)
		color = "000000";
	else if (ch->GetChatColor() == 8)
		color = "A020F0";
	else if (ch->GetChatColor() == 9)
		color = "FFC0CB";
	else
	{
		if (pinfo->type == CHAT_TYPE_SHOUT)
			color = "A7FFD4";
		else
			color = "FFFFFF";
	}
#endif // ENABLE_CHAT_COLOR_SYSTEM


	char chatbuf[CHAT_MAX_LEN + 1];
	int len = 0;
	if (!g_bNewChatView)
	{
		static const char* colorbuf[] = { "|cFFffa200|H|h[Staff]|h|r", "|cFFff0000|H|h[Shinsoo]|h|r", "|cFFffc700|H|h[Chunjo]|h|r", "|cFF000bff|H|h[Jinno]|h|r" };
		len = snprintf(chatbuf, sizeof(chatbuf), "%s %s : %s", (ch->IsGM() ? colorbuf[0] : colorbuf[MINMAX(0, ch->GetEmpire(), 3)]), ch->GetName(), buf);
	}
	else
		len = snprintf(chatbuf, sizeof(chatbuf), "%s : %s", ch->GetName(), buf);

	if (CHAT_TYPE_SHOUT == pinfo->type)
	{
		if (g_bNewChatView)
		{
			const char* kingdoms[3] = { "[1]   ","[2]   ","[3]   " };
			if (ch->GetReborn() > 0)
				len = snprintf(chatbuf, sizeof(chatbuf), "%s%s |H%s%s|h(#)|h|r - |cFF30D5C8|H|h[R-%d]|h|r Sv.%d|h|r : |cFF%s%s|r", kingdoms[ch->GetEmpire() - 1], ch->GetName(), "whisper:", ch->GetName(), ch->GetReborn(), ch->GetLevel(), color, buf);
			else
				len = snprintf(chatbuf, sizeof(chatbuf), "%s%s |H%s%s|h(#)|h|r - Sv.%d|h|r : |cFF%s%s|r", kingdoms[ch->GetEmpire() - 1], ch->GetName(), "whisper:", ch->GetName(), ch->GetLevel(), color, buf);
		}
	}

	if (len < 0 || len >= (int)sizeof(chatbuf))
		len = sizeof(chatbuf) - 1;

	if (pinfo->type == CHAT_TYPE_SHOUT)
	{
		// const int SHOUT_LIMIT_LEVEL = 15;

		if (ch->GetLevel() < g_iShoutLimitLevel)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¿ÜÄ¡±â´Â ·¹º§ %d ÀÌ»ó¸¸ »ç¿ë °¡´É ÇÕ´Ï´Ù."), g_iShoutLimitLevel);
			return (iExtraLen);
		}

		// if (thecore_heart->pulse - (int) ch->GetLastShoutPulse() < passes_per_sec * g_iShoutLimitTime)
		if (thecore_heart->pulse - (int)ch->GetLastShoutPulse() < passes_per_sec * SHOUT_TIME_LIMIT)
			return (iExtraLen);

		ch->SetLastShoutPulse(thecore_heart->pulse);

		//		TPacketGGShout p;
		TPacketGGShout p;
		p.bHeader = HEADER_GG_SHOUT;
		p.bEmpire = ch->GetEmpire();
		strlcpy(p.szText, chatbuf, sizeof(p.szText));

		P2P_MANAGER::instance().Send(&p, sizeof(TPacketGGShout));

		SendShout(chatbuf, ch->GetEmpire());

#ifdef __BATTLE_PASS_SYSTEM__
		BYTE bBattlePassId = ch->GetBattlePassId();
		if (bBattlePassId && hyperlinks > 0)
		{
			DWORD dwUnused, dwCount;
			if(CBattlePass::instance().BattlePassMissionGetInfo(bBattlePassId, SHOUT, &dwUnused, &dwCount))
			{
				DWORD duyurusayisi;

				if (CGameEventsManager::instance().IsActivateEvent(EVENT_X2DUYURU) == true)
					duyurusayisi = 2;
				else
					duyurusayisi = 1;


				if (ch->GetMissionProgress(SHOUT, bBattlePassId) < dwCount)
					ch->UpdateMissionProgress(SHOUT, bBattlePassId, duyurusayisi, dwCount);
			}

#ifdef ENABLE_BATTLE_PASS_EX
			BYTE nextMissionId = ch->GetNextMissionByType(SEND_SHOUT1, SEND_SHOUT31, bBattlePassId);
			if (nextMissionId != 0)
			{
				DWORD dwUnused, dwCount;
				DWORD duyurusayisi;
				if (CGameEventsManager::instance().IsActivateEvent(EVENT_X2DUYURU) == true)
					duyurusayisi = 2;
				else
					duyurusayisi = 1;
				if(CBattlePass::instance().BattlePassMissionGetInfo(bBattlePassId, nextMissionId, &dwUnused, &dwCount))
				{
					if (ch->GetMissionProgress(nextMissionId, bBattlePassId) < dwCount)
						ch->UpdateMissionProgress(nextMissionId, bBattlePassId, duyurusayisi, dwCount);
				}
			}
#endif // ENABLE_BATTLE_PASS_EX
		}
#endif // __BATTLE_PASS_SYSTEM__


		return (iExtraLen);
	}

	TPacketGCChat pack_chat;

	pack_chat.header = HEADER_GC_CHAT;
	pack_chat.size = sizeof(TPacketGCChat) + len;
	pack_chat.type = pinfo->type;
	pack_chat.id = ch->GetVID();

	switch (pinfo->type)
	{
	case CHAT_TYPE_TALKING:
	{
		const DESC_MANAGER::DESC_SET& c_ref_set = DESC_MANAGER::instance().GetClientSet();
		std::for_each(c_ref_set.begin(), c_ref_set.end(), FEmpireChatPacket(pack_chat, chatbuf, len, (ch->GetGMLevel() > GM_PLAYER || ch->IsEquipUniqueGroup(UNIQUE_GROUP_RING_OF_LANGUAGE)) ? 0 : ch->GetEmpire(), ch->GetMapIndex(), strlen(ch->GetName())));

#ifdef __MARTY_CHAT_LOGGING__
		if (ch->IsGM())
		{
			LogManager::instance().EscapeString(__escape_string, sizeof(__escape_string), chatbuf, len);
			LogManager::instance().ChatLog(ch->GetMapIndex(), ch->GetPlayerID(), ch->GetName(), 0, "", "NORMAL", __escape_string, ch->GetDesc() ? ch->GetDesc()->GetHostName() : "");
		}
#endif
	}
	break;

	case CHAT_TYPE_PARTY:
	{
		if (!ch->GetParty())
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÆÄÆ¼ ÁßÀÌ ¾Æ´Õ´Ï´Ù."));
		else
		{
			TEMP_BUFFER tbuf;

			tbuf.write(&pack_chat, sizeof(pack_chat));
			tbuf.write(chatbuf, len);

			RawPacketToCharacterFunc f(tbuf.read_peek(), tbuf.size());
			ch->GetParty()->ForEachOnlineMember(f);
#ifdef __MARTY_CHAT_LOGGING__
			if (ch->IsGM())
			{
				LogManager::instance().EscapeString(__escape_string, sizeof(__escape_string), chatbuf, len);
				LogManager::instance().ChatLog(ch->GetMapIndex(), ch->GetPlayerID(), ch->GetName(), ch->GetParty()->GetLeaderPID(), "", "PARTY", __escape_string, ch->GetDesc() ? ch->GetDesc()->GetHostName() : "");
			}
#endif
		}
	}
	break;

	case CHAT_TYPE_GUILD:
	{
		if (!ch->GetGuild())
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("±æµå¿¡ °¡ÀÔÇÏÁö ¾Ê¾Ò½À´Ï´Ù."));
		else
		{
			ch->GetGuild()->Chat(chatbuf);
#ifdef __MARTY_CHAT_LOGGING__
			if (ch->IsGM())
			{
				LogManager::instance().EscapeString(__escape_string, sizeof(__escape_string), chatbuf, len);
				LogManager::instance().ChatLog(ch->GetMapIndex(), ch->GetPlayerID(), ch->GetName(), ch->GetGuild()->GetID(), ch->GetGuild()->GetName(), "GUILD", __escape_string, ch->GetDesc() ? ch->GetDesc()->GetHostName() : "");
			}
#endif
		}
	}
	break;

	default:
		sys_err("Unknown chat type %d", pinfo->type);
		break;
	}

	return (iExtraLen);
}

void CInputMain::ItemUse(LPCHARACTER ch, const char* data)
{
#ifdef __USE_ITEM_COUNT__
	ch->UseItem(((struct command_item_use*)data)->Cell, NPOS, ((struct command_item_use*)data)->count);
#else // __USE_ITEM_COUNT__
	ch->UseItem(((struct command_item_use*)data)->Cell);
#endif // __USE_ITEM_COUNT__
}

void CInputMain::ItemToItem(LPCHARACTER ch, const char* pcData)
{
	TPacketCGItemUseToItem* p = (TPacketCGItemUseToItem*)pcData;
	if (ch)
		ch->UseItem(p->Cell, p->TargetCell);
}

#ifdef __DROP_DIALOG__
void CInputMain::ItemDelete(LPCHARACTER ch, const char* data)
{
	struct command_item_delete* pinfo = (struct command_item_delete*)data;

	// @pachi001
	// Oyuncular macro yoluyla oyunu yoruyorlardi
	// bazi esyalarda updatepacket dolayisiyla buyuk sikintilar yaratiyordu.
	// bunun icin her saniye maksimum 10 esya kullanilabilecek sekline bir duzenleme yapildi.
	// ayrica HackLog tablosunada bu islemi uygulayan oyuncu kaydediliyor.

	if (ch->GetDungeon() || ch->GetWarMap() || IS_DUNGEON_ZONE(ch->GetMapIndex()))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You cannot open the safebox in dungeon or at war."));
		return;
	}

	if (ch->GetLastItemUsePulse() < thecore_pulse())
		ch->ClearItemUseCounter();

	if (ch->GetItemUseCounter() > 75 && ch->GetLastItemUsePulse() > thecore_pulse())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("you cannot use item too fast!"));
		LogManager::instance().HackLog("ITEM_USE_MACRO", ch);
		//ch->GetDesc()->SetPhase(PHASE_CLOSE);
		return;
	}

	if (ch)
		ch->DeleteItem(pinfo->Cell);
}

void CInputMain::ItemSell(LPCHARACTER ch, const char* data)
{
	struct command_item_sell* pinfo = (struct command_item_sell*)data;

	// @pachi001
	// Oyuncular macro yoluyla oyunu yoruyorlardi
	// bazi esyalarda updatepacket dolayisiyla buyuk sikintilar yaratiyordu.
	// bunun icin her saniye maksimum 10 esya kullanilabilecek sekline bir duzenleme yapildi.
	// ayrica HackLog tablosunada bu islemi uygulayan oyuncu kaydediliyor.

	if (ch->GetDungeon() || ch->GetWarMap() || IS_DUNGEON_ZONE(ch->GetMapIndex()))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You cannot open the safebox in dungeon or at war."));
		return;
	}

	if (ch->GetLastItemUsePulse() < thecore_pulse())
		ch->ClearItemUseCounter();

	if (ch->GetItemUseCounter() > 75 && ch->GetLastItemUsePulse() > thecore_pulse())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("you cannot use item too fast!"));
		LogManager::instance().HackLog("ITEM_USE_MACRO", ch);
		//ch->GetDesc()->SetPhase(PHASE_CLOSE);
		return;
	}

	if (ch)
		ch->SellItem(pinfo->Cell);
}
#endif
void CInputMain::ItemMove(LPCHARACTER ch, const char* data)
{
	struct command_item_move* pinfo = (struct command_item_move*)data;

	if (ch)
		ch->MoveItem(pinfo->Cell, pinfo->CellTo, pinfo->count);
}

void CInputMain::ItemPickup(LPCHARACTER ch, const char* data)
{
	struct command_item_pickup* pinfo = (struct command_item_pickup*)data;
	if (ch)
		ch->PickupItem(pinfo->vid);
}

void CInputMain::QuickslotAdd(LPCHARACTER ch, const char* data)
{
	struct command_quickslot_add* pinfo = (struct command_quickslot_add*)data;

	if (quest::CQuestManager::instance().GetPCForce(ch->GetPlayerID())->IsRunning() == true)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_OPERATION_IS_FAILED_QUEST_WINDOW_IS_OPENED"));
		return;
	}

	if (!ch->CanAct(true, true, VAR_TIME_NONE))
		return;

	if (pinfo->slot.type == QUICKSLOT_TYPE_ITEM)// @duzenleme quickslota oyuncu tarafindan sadece esya eklenebilir (gelistirilmesi gerekli...)
	{
		LPITEM item = NULL;
		TItemPos srcCell(INVENTORY, pinfo->slot.pos);
		if (!(item = ch->GetItem(srcCell)))
			return;
		if (item->GetType() != ITEM_USE && item->GetType() != ITEM_QUEST)
			return;
#ifdef __SLOT_MARKING__
		if (IS_SET(item->GetAntiFlag(), ITEM_ANTIFLAG_QUICKSLOT))
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("WARNING_ANTIFLAG_QUICKSLOT"));
			return;
		}
#endif
	}

	ch->SetQuickslot(pinfo->pos, pinfo->slot);
}

void CInputMain::QuickslotDelete(LPCHARACTER ch, const char* data)
{
	struct command_quickslot_del* pinfo = (struct command_quickslot_del*)data;
	ch->DelQuickslot(pinfo->pos);
}

void CInputMain::QuickslotSwap(LPCHARACTER ch, const char* data)
{
	struct command_quickslot_swap* pinfo = (struct command_quickslot_swap*)data;
	ch->SwapQuickslot(pinfo->pos, pinfo->change_pos);
}

int CInputMain::Messenger(LPCHARACTER ch, const char* c_pData, size_t uiBytes)
{
	TPacketCGMessenger* p = (TPacketCGMessenger*)c_pData;

	if (uiBytes < sizeof(TPacketCGMessenger))
		return -1;

	c_pData += sizeof(TPacketCGMessenger);
	uiBytes -= sizeof(TPacketCGMessenger);

	switch (p->subheader)
	{
	case MESSENGER_SUBHEADER_CG_ADD_BY_VID:
	{
		if (uiBytes < sizeof(TPacketCGMessengerAddByVID))
			return -1;

		TPacketCGMessengerAddByVID* p2 = (TPacketCGMessengerAddByVID*)c_pData;
		LPCHARACTER ch_companion = CHARACTER_MANAGER::instance().Find(p2->vid);

		// @duzenleme
		// hikmetin soyledigine gore adamlar bunu macroya koyarak adami siyah ekran verdiriyorlarmis.
		// bu yuzden boyle bir kontrol koyduk fakat cok detayli denemedik.
		if (thecore_pulse() < ch->GetLastFriendRequestTime() + 180)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ARKADAS_EKLEME_SURE_LIMITI_WARNING"));
			return sizeof(TPacketCGMessengerAddByVID);
		}

		if (!ch_companion)
			return sizeof(TPacketCGMessengerAddByVID);

		if (ch->IsObserverMode())
			return sizeof(TPacketCGMessengerAddByVID);

		if (ch_companion->IsBlockMode(BLOCK_MESSENGER_INVITE))
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("»ó´ë¹æÀÌ ¸Þ½ÅÁ® Ãß°¡ °ÅºÎ »óÅÂÀÔ´Ï´Ù."));
			return sizeof(TPacketCGMessengerAddByVID);
		}

		LPDESC d = ch_companion->GetDesc();

		if (!d)
			return sizeof(TPacketCGMessengerAddByVID);

		if (ch->GetGMLevel() == GM_PLAYER && ch_companion->GetGMLevel() != GM_PLAYER)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<¸Þ½ÅÁ®> ¿î¿µÀÚ´Â ¸Þ½ÅÁ®¿¡ Ãß°¡ÇÒ ¼ö ¾ø½À´Ï´Ù."));
			return sizeof(TPacketCGMessengerAddByVID);
		}

		if (ch->GetDesc() == d)
			return sizeof(TPacketCGMessengerAddByVID);

		MessengerManager::instance().RequestToAdd(ch, ch_companion);
		ch->SetLastFriendRequestTime();

		//MessengerManager::instance().AddToList(ch->GetName(), ch_companion->GetName());
	}
	return sizeof(TPacketCGMessengerAddByVID);

	case MESSENGER_SUBHEADER_CG_ADD_BY_NAME:
	{
		if (uiBytes < CHARACTER_NAME_MAX_LEN)
			return -1;

		char name[CHARACTER_NAME_MAX_LEN + 1];
		strlcpy(name, c_pData, sizeof(name));

		if (ch->GetGMLevel() == GM_PLAYER && gm_get_level(name) != GM_PLAYER)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<¸Þ½ÅÁ®> ¿î¿µÀÚ´Â ¸Þ½ÅÁ®¿¡ Ãß°¡ÇÒ ¼ö ¾ø½À´Ï´Ù."));
			return CHARACTER_NAME_MAX_LEN;
		}

		// @duzenleme
		// hikmetin soyledigine gore adamlar bunu macroya koyarak adami siyah ekran verdiriyorlarmis.
		// bu yuzden boyle bir kontrol koyduk fakat cok detayli denemedik.
		if (thecore_pulse() < ch->GetLastFriendRequestTime() + 180)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ARKADAS_EKLEME_SURE_LIMITI_WARNING"));
			return CHARACTER_NAME_MAX_LEN;
		}

		LPCHARACTER tch = CHARACTER_MANAGER::instance().FindPC(name);

		if (!tch)
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%s ´ÔÀº Á¢¼ÓµÇ ÀÖÁö ¾Ê½À´Ï´Ù."), name);
		else
		{
			if (tch == ch)
				return CHARACTER_NAME_MAX_LEN;

			if (tch->IsBlockMode(BLOCK_MESSENGER_INVITE) == true)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("»ó´ë¹æÀÌ ¸Þ½ÅÁ® Ãß°¡ °ÅºÎ »óÅÂÀÔ´Ï´Ù."));
			}
			else
			{
				MessengerManager::instance().RequestToAdd(ch, tch);
				ch->SetLastFriendRequestTime();

				//MessengerManager::instance().AddToList(ch->GetName(), tch->GetName());
			}
		}
	}
	return CHARACTER_NAME_MAX_LEN;

	case MESSENGER_SUBHEADER_CG_REMOVE:
	{
		if (uiBytes < CHARACTER_NAME_MAX_LEN)
			return -1;

		char char_name[CHARACTER_NAME_MAX_LEN + 1];
		strlcpy(char_name, c_pData, sizeof(char_name));

		MessengerManager::instance().RemoveFromList(ch->GetName(), char_name);
		MessengerManager::instance().RemoveFromList(char_name, ch->GetName()); // @DIKKAT @duzenleme buranin kontrol edilmesi lazim messenger manager uzerinde ayni islemi 2 kere yaptiriyor olabiliriz.
	}
	return CHARACTER_NAME_MAX_LEN;

	default:
		sys_err("CInputMain::Messenger : Unknown subheader %d : %s", p->subheader, ch->GetName());
		break;
	}

	return 0;
}

int CInputMain::Shop(LPCHARACTER ch, const char* data, size_t uiBytes)
{
	TPacketCGShop* p = (TPacketCGShop*)data;

	if (uiBytes < sizeof(TPacketCGShop))
		return -1;

	if (test_server)
		sys_log(0, "CInputMain::Shop() ==> SubHeader %d", p->subheader);

	const char* c_pData = data + sizeof(TPacketCGShop);
	uiBytes -= sizeof(TPacketCGShop);

	switch (p->subheader)
	{
	case SHOP_SUBHEADER_CG_END:
		sys_log(1, "INPUT: %s SHOP: END", ch->GetName());
		CShopManager::instance().StopShopping(ch);
		return 0;

	case SHOP_SUBHEADER_CG_BUY:
	{
		if (uiBytes < sizeof(BYTE) + sizeof(BYTE))
			return -1;

		BYTE bPos = *(c_pData + 1);
		sys_log(1, "INPUT: %s SHOP: BUY %d", ch->GetName(), bPos);
		CShopManager::instance().Buy(ch, bPos);
		return (sizeof(BYTE) + sizeof(BYTE));
	}

	case SHOP_SUBHEADER_CG_SELL:
	{
#ifdef __ADDITIONAL_INVENTORY__
		if (uiBytes < sizeof(BYTE) + sizeof(DWORD) + sizeof(BYTE))
			return -1;

		const BYTE wPos = *reinterpret_cast<const BYTE*>(c_pData);
#ifdef __ITEM_COUNT_LIMIT__
		const DWORD bCount = *(c_pData + sizeof(BYTE));
		const BYTE bType = *(c_pData + sizeof(BYTE) + sizeof(DWORD));
#else
		const BYTE bCount = *(c_pData + sizeof(BYTE));
		const BYTE bType = *(c_pData + sizeof(BYTE) + sizeof(BYTE));
#endif

		sys_log(0, "INPUT: %s SHOP: SELL", ch->GetName());

		CShopManager::instance().Sell(ch, wPos, bCount, bType);
		return sizeof(BYTE) + sizeof(DWORD) + sizeof(BYTE);
#else
		if (uiBytes < sizeof(BYTE))
			return -1;

		BYTE pos = *c_pData;

		sys_log(0, "INPUT: %s SHOP: SELL", ch->GetName());
		CShopManager::instance().Sell(ch, pos);
		return sizeof(BYTE);
#endif
	}

	case SHOP_SUBHEADER_CG_SELL2:
	{
#ifdef __ADDITIONAL_INVENTORY__
#ifdef __ITEM_COUNT_LIMIT__
		if (uiBytes < sizeof(WORD) + sizeof(DWORD) + sizeof(BYTE))
#else
		if (uiBytes < sizeof(WORD) + sizeof(BYTE) + sizeof(BYTE))
#endif
			return -1;

		const WORD wPos = *reinterpret_cast<const WORD*>(c_pData);
#ifdef __ITEM_COUNT_LIMIT__
		const DWORD bCount = *(c_pData + sizeof(WORD));
		const BYTE bType = *(c_pData + sizeof(WORD) + sizeof(DWORD));
#else
		const BYTE bCount = *(c_pData + sizeof(WORD));
		const BYTE bType = *(c_pData + sizeof(WORD) + sizeof(BYTE));
#endif

		sys_log(0, "INPUT: %s SHOP: SELL2", ch->GetName());

		CShopManager::instance().Sell(ch, wPos, bCount, bType);
#ifdef __ITEM_COUNT_LIMIT__
		return sizeof(WORD) + sizeof(DWORD) + sizeof(BYTE);
#else
		return sizeof(WORD) + sizeof(BYTE) + sizeof(BYTE);
#endif
#else
#ifdef __ITEM_COUNT_LIMIT__
		if (uiBytes < sizeof(BYTE) + sizeof(WORD))
#else
		if (uiBytes < sizeof(BYTE) + sizeof(BYTE))
#endif
			return -1;

		BYTE pos = *(c_pData++);
#ifdef __ITEM_COUNT_LIMIT__
		WORD count = *(c_pData);
#else
		BYTE count = *(c_pData);
#endif

		sys_log(0, "INPUT: %s SHOP: SELL2", ch->GetName());
		CShopManager::instance().Sell(ch, pos, count);
#ifdef __ITEM_COUNT_LIMIT__
		return sizeof(BYTE) + sizeof(WORD);
#else
		return sizeof(BYTE) + sizeof(BYTE);
#endif
#endif
	}

	default:
		sys_err("CInputMain::Shop : Unknown subheader %d : %s", p->subheader, ch->GetName());
		break;
	}

	return 0;
}
void CInputMain::OnClick(LPCHARACTER ch, const char* data)
{
	struct command_on_click* pinfo = (struct command_on_click*)data;
	LPCHARACTER			victim;
	if ((victim = CHARACTER_MANAGER::instance().Find(pinfo->vid)))
		victim->OnClick(ch);
	else if (test_server)
		sys_err("CInputMain::OnClick %s.Click.NOT_EXIST_VID[%d]", ch->GetName(), pinfo->vid);
}

void CInputMain::Exchange(LPCHARACTER ch, const char* data)
{
	struct command_exchange* pinfo = (struct command_exchange*)data;
	LPCHARACTER	to_ch = NULL;

	if (!ch->CanHandleItem())
		return;

	int iPulse = thecore_pulse();

	if ((to_ch = CHARACTER_MANAGER::instance().Find(pinfo->arg1)))
	{
		if (iPulse - to_ch->GetSafeboxLoadTime() < PASSES_PER_SEC(g_nPortalLimitTime))
		{
			to_ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_MESSAGER_OPERATION_IS_FAILED_SAFEBOX_TIME"));
			return;
		}

		if (true == to_ch->IsDead())
			return;
	}

	sys_log(0, "CInputMain()::Exchange()  SubHeader %d ", pinfo->sub_header);

	if (iPulse - ch->GetSafeboxLoadTime() < PASSES_PER_SEC(g_nPortalLimitTime))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_OPERATION_IS_FAILED_SAFEBOX_TIME"));
		return;
	}

	switch (pinfo->sub_header)
	{
	case EXCHANGE_SUBHEADER_CG_START:	// arg1 == vid of target character
		if (!ch->GetExchange())
		{
			if ((to_ch = CHARACTER_MANAGER::instance().Find(pinfo->arg1)))
			{
				if (iPulse - ch->GetSafeboxLoadTime() < PASSES_PER_SEC(g_nPortalLimitTime))
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_OPERATION_IS_FAILED_SAFEBOX_TIME"));
					return;
				}

				if (iPulse - to_ch->GetSafeboxLoadTime() < PASSES_PER_SEC(g_nPortalLimitTime))
				{
					to_ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_MESSAGER_OPERATION_IS_FAILED_SAFEBOX_TIME"));
					return;
				}

				if (ch->GetGold() >= GOLD_MAX)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¾×¼ö°¡ 20¾ï ³ÉÀ» ÃÊ°úÇÏ¿© °Å·¡¸¦ ÇÒ¼ö°¡ ¾ø½À´Ï´Ù.."));

#ifdef __GOLD_LIMIT_REWORK__
					sys_err("[OVERFLOG_GOLD] START (%lld) id %u name %s ", ch->GetGold(), ch->GetPlayerID(), ch->GetName());
#else
					sys_err("[OVERFLOG_GOLD] START (%u) id %u name %s ", ch->GetGold(), ch->GetPlayerID(), ch->GetName());
#endif
					return;
				}

				if (to_ch->IsPC())
				{
					if (quest::CQuestManager::instance().GiveItemToPC(ch->GetPlayerID(), to_ch))
					{
						sys_log(0, "Exchange canceled by quest %s %s", ch->GetName(), to_ch->GetName());
						return;
					}
				}

				if (ch->IsDead() || ch->IsStun())
					return;

				if (quest::CQuestManager::instance().GetPCForce(ch->GetPlayerID())->IsRunning() == true)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_OPERATION_IS_FAILED_QUEST_WINDOW_IS_OPENED"));
					return;
				}

				if (ch->GetDungeon() || ch->GetWarMap() || IS_DUNGEON_ZONE(ch->GetMapIndex()))
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You cannot open the safebox in dungeon or at war."));
					return;
				}

				if (!ch->CanAct(true, true, VAR_TIME_NONE, nullptr, VAR_SAFEBOX | VAR_REFINE | VAR_MYSHOP | VAR_ACCE | VAR_CHANGELOOK | VAR_OFFLINESHOP))
					return;

				ch->ExchangeStart(to_ch);
			}
		}
		break;

	case EXCHANGE_SUBHEADER_CG_ITEM_ADD:	// arg1 == position of item, arg2 == position in exchange window
		if (ch->GetExchange())
		{
			if (ch->GetExchange()->GetCompany()->GetAcceptStatus() != true)
				ch->GetExchange()->AddItem(pinfo->Pos, pinfo->arg2);
		}
		break;

	case EXCHANGE_SUBHEADER_CG_ITEM_DEL:	// arg1 == position of item
		if (ch->GetExchange())
		{
			if (ch->GetExchange()->GetCompany()->GetAcceptStatus() != true)
				ch->GetExchange()->RemoveItem(pinfo->arg1);
		}
		break;

	case EXCHANGE_SUBHEADER_CG_ELK_ADD:	// arg1 == amount of gold
		if (ch->GetExchange())
		{
#ifdef __GOLD_LIMIT_REWORK__
			const long long nTotalGold = static_cast<long long>(ch->GetExchange()->GetCompany()->GetOwner()->GetGold()) + static_cast<long long>(pinfo->arg1);

			if (GOLD_MAX <= nTotalGold)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("»ó´ë¹æÀÇ ÃÑ±Ý¾×ÀÌ 20¾ï ³ÉÀ» ÃÊ°úÇÏ¿© °Å·¡¸¦ ÇÒ¼ö°¡ ¾ø½À´Ï´Ù.."));

			/*	sys_err("[OVERFLOW_GOLD] ELK_ADD (%lld) id %u name %s ",
					ch->GetExchange()->GetCompany()->GetOwner()->GetGold(),
					ch->GetExchange()->GetCompany()->GetOwner()->GetPlayerID(),
					ch->GetExchange()->GetCompany()->GetOwner()->GetName());*/

				return;
			}
#else
			const int64_t nTotalGold = static_cast<int64_t>(ch->GetExchange()->GetCompany()->GetOwner()->GetGold()) + static_cast<int64_t>(pinfo->arg1);

			if (GOLD_MAX <= nTotalGold)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("»ó´ë¹æÀÇ ÃÑ±Ý¾×ÀÌ 20¾ï ³ÉÀ» ÃÊ°úÇÏ¿© °Å·¡¸¦ ÇÒ¼ö°¡ ¾ø½À´Ï´Ù.."));

				/*sys_err("[OVERFLOW_GOLD] ELK_ADD (%u) id %u name %s ",
					ch->GetExchange()->GetCompany()->GetOwner()->GetGold(),
					ch->GetExchange()->GetCompany()->GetOwner()->GetPlayerID(),
					ch->GetExchange()->GetCompany()->GetOwner()->GetName());*/

				return;
			}
#endif

			if (ch->GetExchange()->GetCompany()->GetAcceptStatus() != true)
				ch->GetExchange()->AddGold(pinfo->arg1);
		}
		break;

#ifdef __CHEQUE_SYSTEM__
	case EXCHANGE_SUBHEADER_CG_CHEQUE_ADD:	// arg1 == amount of cheque
		if (ch->GetExchange())
		{
			const int64_t nTotalCheque = static_cast<int64_t>(ch->GetExchange()->GetCompany()->GetOwner()->GetCheque()) + static_cast<int64_t>(pinfo->arg1);

			if (CHEQUE_MAX <= nTotalCheque)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("The other player has cheque over the limite."));

				sys_err("[OVERFLOW_CHEQUE] CHEQUE_ADD (%u) id %u name %s ",
					ch->GetExchange()->GetCompany()->GetOwner()->GetCheque(),
					ch->GetExchange()->GetCompany()->GetOwner()->GetPlayerID(),
					ch->GetExchange()->GetCompany()->GetOwner()->GetName());

				return;
			}

			if (ch->GetExchange()->GetCompany()->GetAcceptStatus() != true)
				ch->GetExchange()->AddCheque(pinfo->arg1);
		}
		break;
#endif

	case EXCHANGE_SUBHEADER_CG_ACCEPT:	// arg1 == not used
		if (ch->GetExchange())
		{
			sys_log(0, "CInputMain()::Exchange() ==> ACCEPT ");
			ch->GetExchange()->Accept(true);
		}

		break;

	case EXCHANGE_SUBHEADER_CG_CANCEL:	// arg1 == not used
		if (ch->GetExchange())
			ch->GetExchange()->Cancel();
		break;
	}
}

void CInputMain::Position(LPCHARACTER ch, const char* data)
{
	struct command_position* pinfo = (struct command_position*)data;

	switch (pinfo->position)
	{
	case POSITION_GENERAL:
		ch->Standup();
		break;

	case POSITION_SITTING_CHAIR:
		ch->Sitdown(0);
		break;

	case POSITION_SITTING_GROUND:
		ch->Sitdown(1);
		break;
	}
}

void CInputMain::Move(LPCHARACTER ch, const char* data)
{
	if (!ch->CanMove())
		return;

	struct command_move* pinfo = (struct command_move*)data;

	if (pinfo->bFunc >= FUNC_MAX_NUM && !(pinfo->bFunc & 0x80))
	{
		sys_err("invalid move type: %s", ch->GetName());
		return;
	}

	const float fDist = DISTANCE_SQRT((ch->GetX() - pinfo->lX) / 100, (ch->GetY() - pinfo->lY) / 100);
	// @duzenleme
	// binegin geri atmamasi icin yapilmis
	// MartySama orjinal halinde mevcut fakat yinede atma yapabiliyor bazen.
	if (((false == ch->IsRiding() && fDist > 25) || fDist > 60))
	{
		sys_log(0, "MOVE: %s trying to move too far (dist: %.1fm) Riding(%d)", ch->GetName(), fDist, ch->IsRiding());

		ch->Show(ch->GetMapIndex(), ch->GetX(), ch->GetY(), ch->GetZ());
		ch->Stop();
		return;
	}

	if (pinfo->bFunc == FUNC_MOVE)
	{
		if (ch->GetLimitPoint(POINT_MOV_SPEED) == 0)
			return;

		ch->SetRotation(pinfo->bRot * 5);
		ch->ResetStopTime();				// ""

		ch->Goto(pinfo->lX, pinfo->lY);
	}
	else
	{
		if (pinfo->bFunc == FUNC_ATTACK || pinfo->bFunc == FUNC_COMBO)
			ch->OnMove(true);
		else if (pinfo->bFunc & FUNC_SKILL)
		{
#ifdef __WOLFMAN_CHARACTER__
			const int MASK_SKILL_MOTION = 0x7F;
			unsigned int motion = pinfo->bFunc & MASK_SKILL_MOTION;
#else
			unsigned int motion = pinfo->uArg >> 8;
#endif

			if (!ch->IsUsableSkillMotion(motion))
			{
				const char* name = ch->GetName();
				unsigned int job = ch->GetJob();
				unsigned int group = ch->GetSkillGroup();

				char szBuf[256];
				snprintf(szBuf, sizeof(szBuf), "SKILL_HACK: name=%s, job=%d, group=%d, motion=%d", name, job, group, motion);
				LogManager::instance().HackLog(szBuf, ch->GetDesc()->GetAccountTable().login, ch->GetName(), ch->GetDesc()->GetHostName());
				sys_log(0, "%s", szBuf);

				if (test_server)
				{
					ch->GetDesc()->DelayedDisconnect(number(2, 8));
					ch->ChatPacket(CHAT_TYPE_INFO, szBuf);
				}
				else
				{
					ch->GetDesc()->DelayedDisconnect(number(150, 500));
				}
			}

			ch->OnMove();
		}

		ch->SetRotation(pinfo->bRot * 5);
		ch->ResetStopTime();				// ""

		ch->Move(pinfo->lX, pinfo->lY);
		ch->Stop();
		ch->StopStaminaConsume();
	}

	TPacketGCMove pack;

	pack.bHeader = HEADER_GC_MOVE;
	pack.bFunc = pinfo->bFunc;
	pack.bArg = pinfo->bArg;
	pack.bRot = pinfo->bRot;
	pack.dwVID = ch->GetVID();
	pack.lX = pinfo->lX;
	pack.lY = pinfo->lY;
	pack.dwTime = pinfo->dwTime;
	pack.dwDuration = (pinfo->bFunc == FUNC_MOVE) ? ch->GetCurrentMoveDuration() : 0;

	ch->PacketAround(&pack, sizeof(TPacketGCMove), ch);
}

void CInputMain::Attack(LPCHARACTER ch, const BYTE header, const char* data)
{
	if (NULL == ch)
		return;

	struct type_identifier
	{
		BYTE header;
		BYTE type;
	};

	const struct type_identifier* const type = reinterpret_cast<const struct type_identifier*>(data);

	if (type->type > 0)
	{
		if (false == ch->CanUseSkill(type->type))
		{
			return;
		}

		switch (type->type)
		{
		case SKILL_GEOMPUNG:
		case SKILL_SANGONG:
		case SKILL_YEONSA:
		case SKILL_KWANKYEOK:
		case SKILL_HWAJO:
		case SKILL_GIGUNG:
		case SKILL_PABEOB:
		case SKILL_MARYUNG:
		case SKILL_TUSOK:
		case SKILL_MAHWAN:
		case SKILL_BIPABU:
		case SKILL_NOEJEON:
		case SKILL_CHAIN:
		case SKILL_HORSE_WILDATTACK_RANGE:
			if (HEADER_CG_SHOOT != type->header)
			{
				if (test_server)
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Attack :name[%s] Vnum[%d] can't use skill by attack(warning)"), type->type);
				return;
			}
			break;
		}
	}

	switch (header)
	{
	case HEADER_CG_ATTACK:
	{
		if (NULL == ch->GetDesc())
			return;

		const TPacketCGAttack* const packMelee = reinterpret_cast<const TPacketCGAttack*>(data);

		LPCHARACTER	victim = CHARACTER_MANAGER::instance().Find(packMelee->dwVID);

		if (NULL == victim || ch == victim)
			return;

		switch (victim->GetCharType())
		{
		case CHAR_TYPE_NPC:
		case CHAR_TYPE_WARP:
		case CHAR_TYPE_GOTO:
			return;
		}

		if (packMelee->bType > 0)
		{
			if (g_bCantSkillForBoss)
			{
				if ((victim->IsBoss()) || victim->IsStone())
					return;
			}

			if (false == ch->CheckSkillHitCount(packMelee->bType, victim->GetVID()))
			{
				return;
			}
		}

		ch->Attack(victim, packMelee->bType);
	}
	break;

	case HEADER_CG_SHOOT:
	{
		const TPacketCGShoot* const packShoot = reinterpret_cast<const TPacketCGShoot*>(data);

		ch->Shoot(packShoot->bType);
	}
	break;
	}
}

// @duzenleme
// Bundan 2 sene once myte zamani alper myte filesinde kaldirmisti.
// Dedigine gore Rohanda buradaki kodlar yuzunden oyunu dusuruyorlardi.
// Myte filesinde kullandik kitlemis olabilirmi bir fikrim yok fakat hala board filesinde bu sekilde.
int CInputMain::SyncPosition(LPCHARACTER ch, const char* c_pcData, size_t uiBytes)
{
	const TPacketCGSyncPosition* pinfo = reinterpret_cast<const TPacketCGSyncPosition*>(c_pcData);

	if (uiBytes < pinfo->wSize)
		return -1;

	int iExtraLen = pinfo->wSize - sizeof(TPacketCGSyncPosition);

	if (iExtraLen < 0)
	{
		sys_err("invalid packet length (len %d size %u buffer %u)", iExtraLen, pinfo->wSize, uiBytes);
		ch->GetDesc()->SetPhase(PHASE_CLOSE);
		return -1;
	}

	if (0 != (iExtraLen % sizeof(TPacketCGSyncPositionElement)))
	{
		sys_err("invalid packet length %d (name: %s)", pinfo->wSize, ch->GetName());
		return iExtraLen;
	}

	int iCount = iExtraLen / sizeof(TPacketCGSyncPositionElement);

	if (iCount <= 0)
		return iExtraLen;

	TEMP_BUFFER tbuf;
	LPBUFFER lpBuf = tbuf.getptr();

	TPacketGCSyncPosition* pHeader = (TPacketGCSyncPosition*)buffer_write_peek(lpBuf);
	buffer_write_proceed(lpBuf, sizeof(TPacketGCSyncPosition));

	const TPacketCGSyncPositionElement* e =
		reinterpret_cast<const TPacketCGSyncPositionElement*>(c_pcData + sizeof(TPacketCGSyncPosition));

	timeval tvCurTime;
	gettimeofday(&tvCurTime, NULL);

	for (int i = 0; i < iCount; ++i, ++e)
	{
		LPCHARACTER victim = CHARACTER_MANAGER::instance().Find(e->dwVID);

		if (!victim)
			continue;

		switch (victim->GetCharType())
		{
			case CHAR_TYPE_NPC:
			case CHAR_TYPE_WARP:
			case CHAR_TYPE_GOTO:
				continue;
		}

		if (!victim->SetSyncOwner(ch))
			continue;
		victim->SetLastSyncTime(tvCurTime);
		victim->Sync(e->lX, e->lY);
		buffer_write(lpBuf, e, sizeof(TPacketCGSyncPositionElement));
	}

	if (buffer_size(lpBuf) != sizeof(TPacketGCSyncPosition))
	{
		pHeader->bHeader = HEADER_GC_SYNC_POSITION;
		pHeader->wSize = buffer_size(lpBuf);

		ch->PacketAround(buffer_read_peek(lpBuf), buffer_size(lpBuf), ch);
	}

	return iExtraLen;
}

void CInputMain::FlyTarget(LPCHARACTER ch, const char* pcData, BYTE bHeader)
{
	TPacketCGFlyTargeting* p = (TPacketCGFlyTargeting*)pcData;
	ch->FlyTarget(p->dwTargetVID, p->x, p->y, bHeader);
}

void CInputMain::UseSkill(LPCHARACTER ch, const char* pcData)
{
	TPacketCGUseSkill* p = (TPacketCGUseSkill*)pcData;
	ch->UseSkill(p->dwVnum, CHARACTER_MANAGER::instance().Find(p->dwVID));
}

void CInputMain::ScriptButton(LPCHARACTER ch, const void* c_pData)
{
	TPacketCGScriptButton* p = (TPacketCGScriptButton*)c_pData;
	sys_log(0, "QUEST ScriptButton pid %d idx %u", ch->GetPlayerID(), p->idx);

	quest::PC* pc = quest::CQuestManager::instance().GetPCForce(ch->GetPlayerID());
	if (pc && pc->IsConfirmWait())
	{
		quest::CQuestManager::instance().Confirm(ch->GetPlayerID(), quest::CONFIRM_TIMEOUT);
	}
	else if (p->idx & 0x80000000)
	{
		quest::CQuestManager::Instance().QuestInfo(ch->GetPlayerID(), p->idx & 0x7fffffff);
	}
	else
	{
		quest::CQuestManager::Instance().QuestButton(ch->GetPlayerID(), p->idx);
	}
}

void CInputMain::ScriptAnswer(LPCHARACTER ch, const void* c_pData)
{
	TPacketCGScriptAnswer* p = (TPacketCGScriptAnswer*)c_pData;
	sys_log(0, "QUEST ScriptAnswer pid %d answer %d", ch->GetPlayerID(), p->answer);

	if (p->answer > 250)
	{
		quest::CQuestManager::Instance().Resume(ch->GetPlayerID());
	}
	else
	{
		quest::CQuestManager::Instance().Select(ch->GetPlayerID(), p->answer);
	}
}

// SCRIPT_SELECT_ITEM
void CInputMain::ScriptSelectItem(LPCHARACTER ch, const void* c_pData)
{
	TPacketCGScriptSelectItem* p = (TPacketCGScriptSelectItem*)c_pData;
	sys_log(0, "QUEST ScriptSelectItem pid %d answer %d", ch->GetPlayerID(), p->selection);
	quest::CQuestManager::Instance().SelectItem(ch->GetPlayerID(), p->selection);
}
// END_OF_SCRIPT_SELECT_ITEM

void CInputMain::QuestInputString(LPCHARACTER ch, const void* c_pData)
{
	TPacketCGQuestInputString* p = (TPacketCGQuestInputString*)c_pData;

	char msg[65];
	strlcpy(msg, p->msg, sizeof(msg));
	sys_log(0, "QUEST InputString pid %u msg %s", ch->GetPlayerID(), msg);

	quest::CQuestManager::Instance().Input(ch->GetPlayerID(), msg);
}

void CInputMain::QuestConfirm(LPCHARACTER ch, const void* c_pData)
{
	TPacketCGQuestConfirm* p = (TPacketCGQuestConfirm*)c_pData;
	LPCHARACTER ch_wait = CHARACTER_MANAGER::instance().FindByPID(p->requestPID);
	if (p->answer)
		p->answer = quest::CONFIRM_YES;
	sys_log(0, "QuestConfirm from %s pid %u name %s answer %d", ch->GetName(), p->requestPID, (ch_wait) ? ch_wait->GetName() : "", p->answer);
	if (ch_wait)
	{
		quest::CQuestManager::Instance().Confirm(ch_wait->GetPlayerID(), (quest::EQuestConfirmType)p->answer, ch->GetPlayerID());
	}
}

void CInputMain::Target(LPCHARACTER ch, const char* pcData)
{
	TPacketCGTarget* p = (TPacketCGTarget*)pcData;
	ch->SetTarget(CHARACTER_MANAGER::instance().Find(p->dwVID));
}

void CInputMain::SafeboxCheckin(LPCHARACTER ch, const char* c_pData)
{
	if (quest::CQuestManager::instance().GetPCForce(ch->GetPlayerID())->IsRunning() == true)
		return;

	TPacketCGSafeboxCheckin* p = (TPacketCGSafeboxCheckin*)c_pData;

	if (!ch->CanHandleItem())
		return;

#ifdef __GAME_MASTER_UTILITY__
	if (ch->IsGM() && !ch->MasterCanAction(ACTION_SAFEBOX))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("MASTER_ACTION_SAFEBOX"));
		return;
	}
#endif

	CSafebox* pkSafebox = ch->GetSafebox();
	LPITEM pkItem = ch->GetItem(p->ItemPos);

	if (!pkSafebox || !pkItem)
		return;

#ifdef __SWITCHBOT__
	if (p->ItemPos.IsSwitchbotPosition())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "This item can not be moved here (1)");
		return;
	}
#endif

	if (pkItem->GetCell() >= INVENTORY_MAX_NUM && IS_SET(pkItem->GetFlag(), ITEM_FLAG_IRREMOVABLE))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<Ã¢°í> Ã¢°í·Î ¿Å±æ ¼ö ¾ø´Â ¾ÆÀÌÅÛ ÀÔ´Ï´Ù."));
		return;
	}

	if (!pkSafebox->IsEmpty(p->bSafePos, pkItem->GetSize()))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<Ã¢°í> ¿Å±æ ¼ö ¾ø´Â À§Ä¡ÀÔ´Ï´Ù."));
		return;
	}

	if (pkItem->GetVnum() == UNIQUE_ITEM_SAFEBOX_EXPAND)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<Ã¢°í> ÀÌ ¾ÆÀÌÅÛÀº ³ÖÀ» ¼ö ¾ø½À´Ï´Ù."));
		return;
	}

	if (IS_SET(pkItem->GetAntiFlag(), ITEM_ANTIFLAG_SAFEBOX))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<Ã¢°í> ÀÌ ¾ÆÀÌÅÛÀº ³ÖÀ» ¼ö ¾ø½À´Ï´Ù."));
		return;
	}

	if (true == pkItem->isLocked())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<Ã¢°í> ÀÌ ¾ÆÀÌÅÛÀº ³ÖÀ» ¼ö ¾ø½À´Ï´Ù."));
		return;
	}
#ifdef __BEGINNER_ITEM__
	if (pkItem->IsBasicItem())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ITEM_IS_BASIC_CANNOT_DO"));
		return;
	}
#endif
	if (pkItem->IsEquipped())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ITEM_GIYILI_IKEN_DEPOYA_KONULAMAZ"));
		return;
	}
#ifdef __WEAPON_COSTUME_SYSTEM__
	if (pkItem->IsEquipped())
	{
		int iWearCell = pkItem->FindEquipCell(ch);
		if (iWearCell == WEAR_WEAPON)
		{
			LPITEM costumeWeapon = ch->GetWear(WEAR_COSTUME_WEAPON);
			if (costumeWeapon && !ch->UnequipItem(costumeWeapon))
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You cannot unequip the costume weapon. Not enough space."));
				return;
			}
		}
	}
#endif

	if (pkItem->IsDragonSoul() && DSManager::instance().IsActiveDragonSoul(pkItem) == true)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("DSS_CANNOT_CHECKIN_WITH_ACTIVATE"));
		return;
	}

	// @duzenleme
	// depoya ekleyecegim esya kemer ise ve kemer envanterinde esya var ise
	// depoya koymami engelliyor.
	if (ITEM_BELT == pkItem->GetType() && CBeltInventoryHelper::IsExistItemInBeltInventory(ch))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("º§Æ® ÀÎº¥Åä¸®¿¡ ¾ÆÀÌÅÛÀÌ Á¸ÀçÇÏ¸é ÇØÁ¦ÇÒ ¼ö ¾ø½À´Ï´Ù."));
		return;
	}

	pkItem->RemoveFromCharacter();
	if (!pkItem->IsDragonSoul()
#ifdef __ADDITIONAL_INVENTORY__
		&& !pkItem->IsUpgradeItem() && !pkItem->IsBook() && !pkItem->IsStone() && !pkItem->IsFlower() && !pkItem->IsAttrItem() && !pkItem->IsChest()
#endif
		)
		ch->SyncQuickslot(QUICKSLOT_TYPE_ITEM, p->ItemPos.cell, 255);
	pkSafebox->Add(p->bSafePos, pkItem);
}

void CInputMain::SafeboxCheckout(LPCHARACTER ch, const char* c_pData, bool bMall)
{
	TPacketCGSafeboxCheckout* p = (TPacketCGSafeboxCheckout*)c_pData;

	if (!ch->CanHandleItem())
		return;

#ifdef __GAME_MASTER_UTILITY__
	if (ch->IsGM() && !ch->MasterCanAction(ACTION_SAFEBOX))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("MASTER_ACTION_SAFEBOX"));
		return;
	}
#endif

	CSafebox* pkSafebox;

	if (bMall)
		pkSafebox = ch->GetMall();
	else
		pkSafebox = ch->GetSafebox();

	if (!pkSafebox)
		return;

	LPITEM pkItem = pkSafebox->Get(p->bSafePos);

	if (!pkItem)
		return;

	if (!ch->IsEmptyItemGrid(p->ItemPos, pkItem->GetSize()))
		return;

	if (pkItem->IsDragonSoul())
	{
		if (bMall)
		{
			DSManager::instance().DragonSoulItemInitialize(pkItem);
		}

		if (DRAGON_SOUL_INVENTORY != p->ItemPos.window_type)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<Ã¢°í> ¿Å±æ ¼ö ¾ø´Â À§Ä¡ÀÔ´Ï´Ù."));
			return;
		}

		TItemPos DestPos = p->ItemPos;
		if (!DSManager::instance().IsValidCellForThisItem(pkItem, DestPos))
		{
			int iCell = ch->GetEmptyDragonSoulInventory(pkItem);
			if (iCell < 0)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<Ã¢°í> ¿Å±æ ¼ö ¾ø´Â À§Ä¡ÀÔ´Ï´Ù."));
				return;
			}
			DestPos = TItemPos(DRAGON_SOUL_INVENTORY, iCell);
		}

		pkSafebox->Remove(p->bSafePos);
		pkItem->__ADD_TO_CHARACTER(ch, DestPos);
		ITEM_MANAGER::instance().FlushDelayedSave(pkItem);
	}
#ifdef __ADDITIONAL_INVENTORY__
	else if (pkItem->IsUpgradeItem())
	{
		if (UPGRADE_INVENTORY != p->ItemPos.window_type)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<Ã¢°í> ¿Å±æ ¼ö ¾ø´Â À§Ä¡ÀÔ´Ï´Ù."));
			return;
		}

		TItemPos DestPos = p->ItemPos;

		int iCell = ch->GetEmptyUpgradeInventory(pkItem);
		if (iCell < 0)
		{
			return;
		}
		DestPos = TItemPos(UPGRADE_INVENTORY, iCell);

		pkSafebox->Remove(p->bSafePos);
		pkItem->__ADD_TO_CHARACTER(ch, DestPos);
		ITEM_MANAGER::instance().FlushDelayedSave(pkItem);
	}
	else if (pkItem->IsBook())
	{
		if (BOOK_INVENTORY != p->ItemPos.window_type)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<Ã¢°í> ¿Å±æ ¼ö ¾ø´Â À§Ä¡ÀÔ´Ï´Ù."));
			return;
		}

		TItemPos DestPos = p->ItemPos;

		int iCell = ch->GetEmptyBookInventory(pkItem);
		if (iCell < 0)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<Ã¢°í> ¿Å±æ ¼ö ¾ø´Â À§Ä¡ÀÔ´Ï´Ù."));
			return;
		}
		DestPos = TItemPos(BOOK_INVENTORY, iCell);

		pkSafebox->Remove(p->bSafePos);
		pkItem->__ADD_TO_CHARACTER(ch, DestPos);
		ITEM_MANAGER::instance().FlushDelayedSave(pkItem);
	}
	else if (pkItem->IsStone())
	{
		if (STONE_INVENTORY != p->ItemPos.window_type)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<Ã¢°í> ¿Å±æ ¼ö ¾ø´Â À§Ä¡ÀÔ´Ï´Ù."));
			return;
		}

		TItemPos DestPos = p->ItemPos;

		int iCell = ch->GetEmptyStoneInventory(pkItem);
		if (iCell < 0)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<Ã¢°í> ¿Å±æ ¼ö ¾ø´Â À§Ä¡ÀÔ´Ï´Ù."));
			return;
		}
		DestPos = TItemPos(STONE_INVENTORY, iCell);

		pkSafebox->Remove(p->bSafePos);
		pkItem->__ADD_TO_CHARACTER(ch, DestPos);
		ITEM_MANAGER::instance().FlushDelayedSave(pkItem);
	}
	else if (pkItem->IsFlower())
	{
		if (FLOWER_INVENTORY != p->ItemPos.window_type)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<Ã¢°í> ¿Å±æ ¼ö ¾ø´Â À§Ä¡ÀÔ´Ï´Ù."));
			return;
		}

		TItemPos DestPos = p->ItemPos;

		int iCell = ch->GetEmptyFlowerInventory(pkItem);
		if (iCell < 0)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<Ã¢°í> ¿Å±æ ¼ö ¾ø´Â À§Ä¡ÀÔ´Ï´Ù."));
			return;
		}
		DestPos = TItemPos(FLOWER_INVENTORY, iCell);

		pkSafebox->Remove(p->bSafePos);
		pkItem->__ADD_TO_CHARACTER(ch, DestPos);
		ITEM_MANAGER::instance().FlushDelayedSave(pkItem);
	}
	else if (pkItem->IsAttrItem())
	{
		if (ATTR_INVENTORY != p->ItemPos.window_type)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<Ã¢°í> ¿Å±æ ¼ö ¾ø´Â À§Ä¡ÀÔ´Ï´Ù."));
			return;
		}

		TItemPos DestPos = p->ItemPos;

		int iCell = ch->GetEmptyAttrInventory(pkItem);
		if (iCell < 0)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<Ã¢°í> ¿Å±æ ¼ö ¾ø´Â À§Ä¡ÀÔ´Ï´Ù."));
			return;
		}
		DestPos = TItemPos(ATTR_INVENTORY, iCell);

		pkSafebox->Remove(p->bSafePos);
		pkItem->__ADD_TO_CHARACTER(ch, DestPos);
		ITEM_MANAGER::instance().FlushDelayedSave(pkItem);
	}
	else if (pkItem->IsChest())
	{
		if (CHEST_INVENTORY != p->ItemPos.window_type)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<Ã¢°í> ¿Å±æ ¼ö ¾ø´Â À§Ä¡ÀÔ´Ï´Ù."));
			return;
		}

		TItemPos DestPos = p->ItemPos;

		int iCell = ch->GetEmptyChestInventory(pkItem);
		if (iCell < 0)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<Ã¢°í> ¿Å±æ ¼ö ¾ø´Â À§Ä¡ÀÔ´Ï´Ù."));
			return;
		}
		DestPos = TItemPos(CHEST_INVENTORY, iCell);

		pkSafebox->Remove(p->bSafePos);
		pkItem->__ADD_TO_CHARACTER(ch, DestPos);
		ITEM_MANAGER::instance().FlushDelayedSave(pkItem);
	}
#endif
	else
	{
		if (DRAGON_SOUL_INVENTORY == p->ItemPos.window_type
#ifdef __ADDITIONAL_INVENTORY__
			|| UPGRADE_INVENTORY == p->ItemPos.window_type
			|| BOOK_INVENTORY == p->ItemPos.window_type
			|| STONE_INVENTORY == p->ItemPos.window_type
			|| FLOWER_INVENTORY == p->ItemPos.window_type
			|| ATTR_INVENTORY == p->ItemPos.window_type
			|| CHEST_INVENTORY == p->ItemPos.window_type
			|| SWITCHBOT == p->ItemPos.window_type
#endif
			)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<Ã¢°í> ¿Å±æ ¼ö ¾ø´Â À§Ä¡ÀÔ´Ï´Ù."));
			return;
		}
		// @duzenleme
		// eger depodan alinan esyanin pozisyonu kemer envanteriyse ve kemer envanterine o an esya koyamiyor isem return cektiriyor.
		if (p->ItemPos.IsBeltInventoryPosition() && false == CBeltInventoryHelper::CanMoveIntoBeltInventory(pkItem))
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ ¾ÆÀÌÅÛÀº º§Æ® ÀÎº¥Åä¸®·Î ¿Å±æ ¼ö ¾ø½À´Ï´Ù."));
			return;
		}

		pkSafebox->Remove(p->bSafePos);
		pkItem->__ADD_TO_CHARACTER(ch, p->ItemPos);
		ITEM_MANAGER::instance().FlushDelayedSave(pkItem);
	}

	DWORD dwID = pkItem->GetID();
	db_clientdesc->DBPacketHeader(HEADER_GD_ITEM_FLUSH, 0, sizeof(DWORD));
	db_clientdesc->Packet(&dwID, sizeof(DWORD));
}

void CInputMain::SafeboxItemMove(LPCHARACTER ch, const char* data)
{
	struct command_item_move* pinfo = (struct command_item_move*)data;

	if (!ch->CanHandleItem())
		return;

	if (!ch->GetSafebox())
		return;

	ch->GetSafebox()->MoveItem(pinfo->Cell.cell, pinfo->CellTo.cell, pinfo->count);
}

// PARTY_JOIN_BUG_FIX
void CInputMain::PartyInvite(LPCHARACTER ch, const char* c_pData)
{
	TPacketCGPartyInvite* p = (TPacketCGPartyInvite*)c_pData;

	LPCHARACTER pInvitee = CHARACTER_MANAGER::instance().Find(p->vid);

	if (!pInvitee || !ch->GetDesc() || !pInvitee->GetDesc() || !pInvitee->IsPC() || !ch->IsPC())
	{
		sys_err("PARTY Cannot find invited character");
		return;
	}

	ch->PartyInvite(pInvitee);
}

void CInputMain::PartyInviteAnswer(LPCHARACTER ch, const char* c_pData)
{
	TPacketCGPartyInviteAnswer* p = (TPacketCGPartyInviteAnswer*)c_pData;

	LPCHARACTER pInviter = CHARACTER_MANAGER::instance().Find(p->leader_vid);

	if (!pInviter || !pInviter->IsPC())
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<ÆÄÆ¼> ÆÄÆ¼¿äÃ»À» ÇÑ Ä³¸¯ÅÍ¸¦ Ã£À»¼ö ¾ø½À´Ï´Ù."));
	else if (!p->accept)
		pInviter->PartyInviteDeny(ch->GetPlayerID());
	else
		pInviter->PartyInviteAccept(ch);
}
// END_OF_PARTY_JOIN_BUG_FIX

void CInputMain::PartySetState(LPCHARACTER ch, const char* c_pData)
{
	if (!CPartyManager::instance().IsEnablePCParty())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<ÆÄÆ¼> ¼­¹ö ¹®Á¦·Î ÆÄÆ¼ °ü·Ã Ã³¸®¸¦ ÇÒ ¼ö ¾ø½À´Ï´Ù."));
		return;
	}

	TPacketCGPartySetState* p = (TPacketCGPartySetState*)c_pData;

	if (!ch->GetParty())
		return;

	if (ch->GetParty()->GetLeaderPID() != ch->GetPlayerID())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<ÆÄÆ¼> ¸®´õ¸¸ º¯°æÇÒ ¼ö ÀÖ½À´Ï´Ù."));
		return;
	}

	if (!ch->GetParty()->IsMember(p->pid))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<ÆÄÆ¼> »óÅÂ¸¦ º¯°æÇÏ·Á´Â »ç¶÷ÀÌ ÆÄÆ¼¿øÀÌ ¾Æ´Õ´Ï´Ù."));
		return;
	}

	DWORD pid = p->pid;
	sys_log(0, "PARTY SetRole pid %d to role %d state %s", pid, p->byRole, p->flag ? "on" : "off");

	switch (p->byRole)
	{
	case PARTY_ROLE_NORMAL:
		break;

	case PARTY_ROLE_ATTACKER:
	case PARTY_ROLE_TANKER:
	case PARTY_ROLE_BUFFER:
	case PARTY_ROLE_SKILL_MASTER:
	case PARTY_ROLE_HASTE:
	case PARTY_ROLE_DEFENDER:
#ifdef __PARTY_ROLE_REWORK__
	case PARTY_ROLE_ATTACKER_MONSTER:
	case PARTY_ROLE_ATTACKER_STONE:
	case PARTY_ROLE_ATTACKER_BOSS:
#endif
		if (ch->GetParty()->SetRole(pid, p->byRole, p->flag))
		{
			TPacketPartyStateChange pack;
			pack.dwLeaderPID = ch->GetPlayerID();
			pack.dwPID = p->pid;
			pack.bRole = p->byRole;
			pack.bFlag = p->flag;
			db_clientdesc->DBPacket(HEADER_GD_PARTY_STATE_CHANGE, 0, &pack, sizeof(pack));
		}

		break;

	default:
		sys_err("wrong byRole in PartySetState Packet name %s state %d", ch->GetName(), p->byRole);
		break;
	}
}

void CInputMain::PartyRemove(LPCHARACTER ch, const char* c_pData)
{
	if (!CPartyManager::instance().IsEnablePCParty())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<ÆÄÆ¼> ¼­¹ö ¹®Á¦·Î ÆÄÆ¼ °ü·Ã Ã³¸®¸¦ ÇÒ ¼ö ¾ø½À´Ï´Ù."));
		return;
	}

	if (ch->GetDungeon())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<ÆÄÆ¼> ´øÀü ¾È¿¡¼­´Â ÆÄÆ¼¿¡¼­ Ãß¹æÇÒ ¼ö ¾ø½À´Ï´Ù."));
		return;
	}

	if (IS_DUNGEON_ZONE(ch->GetMapIndex()))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<ÆÄÆ¼> ´øÀü ¾È¿¡¼­´Â ÆÄÆ¼¿¡¼­ Ãß¹æÇÒ ¼ö ¾ø½À´Ï´Ù."));
		return;
	}

	if (!ch->GetParty())
		return;

	LPPARTY pParty = ch->GetParty();

	TPacketCGPartyRemove* p = (TPacketCGPartyRemove*)c_pData;

	if (pParty->GetLeaderPID() == ch->GetPlayerID())
	{
		if (ch->GetDungeon())
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<ÆÄÆ¼> ´øÁ¯³»¿¡¼­´Â ÆÄÆ¼¿øÀ» Ãß¹æÇÒ ¼ö ¾ø½À´Ï´Ù."));
		}
		else
		{
			if (pParty->IsPartyInDungeon(IS_DUNGEON_ZONE(ch->GetMapIndex())))
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("PARTY_DUNGEON_FIX_MESSAGE"));
				return;
			}

			// leader can remove any member
			if (p->pid == ch->GetPlayerID() || pParty->GetMemberCount() == 2)
			{
				// party disband
				CPartyManager::instance().DeleteParty(pParty);
			}
			else
			{
				LPCHARACTER B = CHARACTER_MANAGER::instance().FindByPID(p->pid);
				if (B)
				{
					//pParty->SendPartyRemoveOneToAll(B);
					B->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<ÆÄÆ¼> ÆÄÆ¼¿¡¼­ Ãß¹æ´çÇÏ¼Ì½À´Ï´Ù."));
					//pParty->Unlink(B);
					//CPartyManager::instance().SetPartyMember(B->GetPlayerID(), NULL);
				}
				pParty->Quit(p->pid);
			}
		}
	}
	else
	{
		// otherwise, only remove itself
		if (p->pid == ch->GetPlayerID())
		{
			if (ch->GetDungeon())
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<ÆÄÆ¼> ´øÁ¯³»¿¡¼­´Â ÆÄÆ¼¸¦ ³ª°¥ ¼ö ¾ø½À´Ï´Ù."));
			}
			else
			{
				if (pParty->IsPartyInDungeon(IS_DUNGEON_ZONE(ch->GetMapIndex())))
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("PARTY_DUNGEON_FIX_MESSAGE"));
					return;
				}

				if (pParty->GetMemberCount() == 2)
				{
					// party disband
					CPartyManager::instance().DeleteParty(pParty);
				}
				else
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<ÆÄÆ¼> ÆÄÆ¼¿¡¼­ ³ª°¡¼Ì½À´Ï´Ù."));
					//pParty->SendPartyRemoveOneToAll(ch);
					pParty->Quit(ch->GetPlayerID());
					//pParty->SendPartyRemoveAllToOne(ch);
					//CPartyManager::instance().SetPartyMember(ch->GetPlayerID(), NULL);
				}
			}
		}
		else
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<ÆÄÆ¼> ´Ù¸¥ ÆÄÆ¼¿øÀ» Å»Åð½ÃÅ³ ¼ö ¾ø½À´Ï´Ù."));
		}
	}
}

void CInputMain::AnswerMakeGuild(LPCHARACTER ch, const char* c_pData)
{
	TPacketCGAnswerMakeGuild* p = (TPacketCGAnswerMakeGuild*)c_pData;

	if (ch->GetGold() < 200000 || ch->GetLevel() < 40) // @duzenleme loncayi 40 level alti ve 200000 yangi olmayan adam acamasin diye yapmisiz fakat chat packet koyulmasi lazim oyuncu anlamaz.
		return;

	if (get_global_time() - ch->GetQuestFlag("guild_manage.new_disband_time") <
		CGuildManager::instance().GetDisbandDelay())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<±æµå> ÇØ»êÇÑ ÈÄ %dÀÏ ÀÌ³»¿¡´Â ±æµå¸¦ ¸¸µé ¼ö ¾ø½À´Ï´Ù."),
			quest::CQuestManager::instance().GetEventFlag("guild_disband_delay"));
		return;
	}

	if (get_global_time() - ch->GetQuestFlag("guild_manage.new_withdraw_time") <
		CGuildManager::instance().GetWithdrawDelay())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<±æµå> Å»ÅðÇÑ ÈÄ %dÀÏ ÀÌ³»¿¡´Â ±æµå¸¦ ¸¸µé ¼ö ¾ø½À´Ï´Ù."),
			quest::CQuestManager::instance().GetEventFlag("guild_withdraw_delay"));
		return;
	}

	if (ch->GetGuild())
		return;

	CGuildManager& gm = CGuildManager::instance();

	TGuildCreateParameter cp;
	memset(&cp, 0, sizeof(cp));

	cp.master = ch;
	strlcpy(cp.name, p->guild_name, sizeof(cp.name));

	if (cp.name[0] == 0 || !check_name(cp.name))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀûÇÕÇÏÁö ¾ÊÀº ±æµå ÀÌ¸§ ÀÔ´Ï´Ù."));
		return;
	}

	DWORD dwGuildID = gm.CreateGuild(cp);

	if (dwGuildID)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<±æµå> [%s] ±æµå°¡ »ý¼ºµÇ¾ú½À´Ï´Ù."), cp.name);

		int GuildCreateFee = 200000;

		ch->PointChange(POINT_GOLD, -GuildCreateFee);
		//ch->SendGuildName(dwGuildID);
	}
	else
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<±æµå> ±æµå »ý¼º¿¡ ½ÇÆÐÇÏ¿´½À´Ï´Ù."));
}

void CInputMain::PartyParameter(LPCHARACTER ch, const char* c_pData)
{
	TPacketCGPartyParameter* p = (TPacketCGPartyParameter*)c_pData;

	// @duzenleme karakter lider degilse expi duzenleyemeyecek sekilde ayarlandi.
	if (ch->GetParty() && ch->GetParty()->GetLeaderPID() == ch->GetPlayerID())
		ch->GetParty()->SetParameter(p->bDistributeMode);
}

size_t GetSubPacketSize(const GUILD_SUBHEADER_CG& header)
{
	switch (header)
	{
	case GUILD_SUBHEADER_CG_ADD_MEMBER:					return sizeof(DWORD);
	case GUILD_SUBHEADER_CG_REMOVE_MEMBER:				return sizeof(DWORD);
	case GUILD_SUBHEADER_CG_CHANGE_GRADE_NAME:			return 10;
	case GUILD_SUBHEADER_CG_CHANGE_GRADE_AUTHORITY:		return sizeof(BYTE) + sizeof(BYTE);
	case GUILD_SUBHEADER_CG_OFFER:						return sizeof(DWORD);
	case GUILD_SUBHEADER_CG_CHARGE_GSP:					return sizeof(int);
	case GUILD_SUBHEADER_CG_POST_COMMENT:				return 1;
	case GUILD_SUBHEADER_CG_DELETE_COMMENT:				return sizeof(DWORD);
	case GUILD_SUBHEADER_CG_REFRESH_COMMENT:			return 0;
	case GUILD_SUBHEADER_CG_CHANGE_MEMBER_GRADE:		return sizeof(DWORD) + sizeof(BYTE);
	case GUILD_SUBHEADER_CG_USE_SKILL:					return sizeof(TPacketCGGuildUseSkill);
	case GUILD_SUBHEADER_CG_CHANGE_MEMBER_GENERAL:		return sizeof(DWORD) + sizeof(BYTE);
	case GUILD_SUBHEADER_CG_GUILD_INVITE_ANSWER:		return sizeof(DWORD) + sizeof(BYTE);
	}

	return 0;
}

int CInputMain::Guild(LPCHARACTER ch, const char* data, size_t uiBytes)
{
	if (uiBytes < sizeof(TPacketCGGuild))
		return -1;

	const TPacketCGGuild* p = reinterpret_cast<const TPacketCGGuild*>(data);
	const char* c_pData = data + sizeof(TPacketCGGuild);

	uiBytes -= sizeof(TPacketCGGuild);

	const GUILD_SUBHEADER_CG SubHeader = static_cast<GUILD_SUBHEADER_CG>(p->subheader);
	const size_t SubPacketLen = GetSubPacketSize(SubHeader);

	if (uiBytes < SubPacketLen)
	{
		return -1;
	}

	CGuild* pGuild = ch->GetGuild();

	if (NULL == pGuild)
	{
		if (SubHeader != GUILD_SUBHEADER_CG_GUILD_INVITE_ANSWER)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<±æµå> ±æµå¿¡ ¼ÓÇØÀÖÁö ¾Ê½À´Ï´Ù."));
			return SubPacketLen;
		}
	}

	switch (SubHeader)
	{
	case GUILD_SUBHEADER_CG_ADD_MEMBER:
	{
		const DWORD vid = *reinterpret_cast<const DWORD*>(c_pData);
		LPCHARACTER newmember = CHARACTER_MANAGER::instance().Find(vid);

		if (!newmember)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<±æµå> ±×·¯ÇÑ »ç¶÷À» Ã£À» ¼ö ¾ø½À´Ï´Ù."));
			return SubPacketLen;
		}

		// @duzenleme
		// eger loncaya almaya calistigim arkadas pc degil ise return cekiyor bir nevi exploit fakat paket gonderen
		// ch adli arkadas nasil pc olmadan descsiz paket gonderiyor o tabiki Allah'a kalmis bir konu xD
		if (!ch->IsPC() || !newmember->IsPC())
			return SubPacketLen;

		pGuild->Invite(ch, newmember);
	}
	return SubPacketLen;

	case GUILD_SUBHEADER_CG_REMOVE_MEMBER:
	{
		if (pGuild->UnderAnyWar() != 0)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<±æµå> ±æµåÀü Áß¿¡´Â ±æµå¿øÀ» Å»Åð½ÃÅ³ ¼ö ¾ø½À´Ï´Ù."));
			return SubPacketLen;
		}

		const DWORD pid = *reinterpret_cast<const DWORD*>(c_pData);
		const TGuildMember* m = pGuild->GetMember(ch->GetPlayerID());

		if (NULL == m)
			return -1;

		LPCHARACTER member = CHARACTER_MANAGER::instance().FindByPID(pid);

		if (member)
		{
			if (member->GetGuild() != pGuild)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<±æµå> »ó´ë¹æÀÌ °°Àº ±æµå°¡ ¾Æ´Õ´Ï´Ù."));
				return SubPacketLen;
			}

			if (!pGuild->HasGradeAuth(m->grade, GUILD_AUTH_REMOVE_MEMBER))
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<±æµå> ±æµå¿øÀ» °­Á¦ Å»Åð ½ÃÅ³ ±ÇÇÑÀÌ ¾ø½À´Ï´Ù."));
				return SubPacketLen;
			}

			member->SetQuestFlag("guild_manage.new_withdraw_time", get_global_time());
			pGuild->RequestRemoveMember(member->GetPlayerID());

			if (g_bGuildInviteLimit)
			{
				DBManager::instance().Query("REPLACE INTO guild_invite_limit VALUES(%d, %d)", pGuild->GetID(), get_global_time());
			}
		}
		else
		{
			if (!pGuild->HasGradeAuth(m->grade, GUILD_AUTH_REMOVE_MEMBER))
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<±æµå> ±æµå¿øÀ» °­Á¦ Å»Åð ½ÃÅ³ ±ÇÇÑÀÌ ¾ø½À´Ï´Ù."));
				return SubPacketLen;
			}

			if (pGuild->RequestRemoveMember(pid))
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<±æµå> ±æµå¿øÀ» °­Á¦ Å»Åð ½ÃÄ×½À´Ï´Ù."));
			else
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<±æµå> ±×·¯ÇÑ »ç¶÷À» Ã£À» ¼ö ¾ø½À´Ï´Ù."));
		}
	}
	return SubPacketLen;

	case GUILD_SUBHEADER_CG_CHANGE_GRADE_NAME:
	{
		char gradename[GUILD_GRADE_NAME_MAX_LEN + 1];
		strlcpy(gradename, c_pData + 1, sizeof(gradename));

		const TGuildMember* m = pGuild->GetMember(ch->GetPlayerID());

		if (NULL == m)
			return -1;

		if (m->grade != GUILD_LEADER_GRADE)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<±æµå> Á÷À§ ÀÌ¸§À» º¯°æÇÒ ±ÇÇÑÀÌ ¾ø½À´Ï´Ù."));
		}
		else if (*c_pData == GUILD_LEADER_GRADE)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<±æµå> ±æµåÀåÀÇ Á÷À§ ÀÌ¸§Àº º¯°æÇÒ ¼ö ¾ø½À´Ï´Ù."));
		}
		else if (!check_name(gradename))
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<±æµå> ÀûÇÕÇÏÁö ¾ÊÀº Á÷À§ ÀÌ¸§ ÀÔ´Ï´Ù."));
		}
		else
		{
			pGuild->ChangeGradeName(*c_pData, gradename);
		}
	}
	return SubPacketLen;

	case GUILD_SUBHEADER_CG_CHANGE_GRADE_AUTHORITY:
	{
		const TGuildMember* m = pGuild->GetMember(ch->GetPlayerID());

		if (NULL == m)
			return -1;

		if (m->grade != GUILD_LEADER_GRADE)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<±æµå> Á÷À§ ±ÇÇÑÀ» º¯°æÇÒ ±ÇÇÑÀÌ ¾ø½À´Ï´Ù."));
		}
		else if (*c_pData == GUILD_LEADER_GRADE)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<±æµå> ±æµåÀåÀÇ ±ÇÇÑÀº º¯°æÇÒ ¼ö ¾ø½À´Ï´Ù."));
		}
		else
		{
			pGuild->ChangeGradeAuth(*c_pData, *(c_pData + 1));
		}
	}
	return SubPacketLen;

	case GUILD_SUBHEADER_CG_OFFER:
	{
		DWORD offer = *reinterpret_cast<const DWORD*>(c_pData);

		if (pGuild->GetLevel() >= GUILD_MAX_LEVEL)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<±æµå> ±æµå°¡ ÀÌ¹Ì ÃÖ°í ·¹º§ÀÔ´Ï´Ù."));
		}
		else
		{
			offer /= 100;
			offer *= 100;

			if (pGuild->OfferExp(ch, offer))
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<±æµå> %uÀÇ °æÇèÄ¡¸¦ ÅõÀÚÇÏ¿´½À´Ï´Ù."), offer);
			}
			else
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<±æµå> °æÇèÄ¡ ÅõÀÚ¿¡ ½ÇÆÐÇÏ¿´½À´Ï´Ù."));
			}
		}
	}
	return SubPacketLen;

	case GUILD_SUBHEADER_CG_CHARGE_GSP:
	{
		const int offer = *reinterpret_cast<const int*>(c_pData);
#ifdef __GOLD_LIMIT_REWORK__
		const long long gold = offer * 100;
#else
		const int gold = offer * 100;
#endif

		if (offer < 0 || gold < offer || gold < 0 || ch->GetGold() < gold)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<±æµå> µ·ÀÌ ºÎÁ·ÇÕ´Ï´Ù."));
			return SubPacketLen;
		}

		if (!pGuild->ChargeSP(ch, offer))
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<±æµå> ¿ë½Å·Â È¸º¹¿¡ ½ÇÆÐÇÏ¿´½À´Ï´Ù."));
		}
	}
	return SubPacketLen;

	case GUILD_SUBHEADER_CG_POST_COMMENT:
	{
		const size_t length = *c_pData;

		if (length > GUILD_COMMENT_MAX_LEN)
		{
			sys_err("POST_COMMENT: %s comment too long (length: %u)", ch->GetName(), length);
			ch->GetDesc()->SetPhase(PHASE_CLOSE);
			return -1;
		}

		if (uiBytes < 1 + length)
			return -1;

		const TGuildMember* m = pGuild->GetMember(ch->GetPlayerID());

		if (NULL == m)
			return -1;

		if (length && !pGuild->HasGradeAuth(m->grade, GUILD_AUTH_NOTICE) && *(c_pData + 1) == '!')
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<±æµå> °øÁö±ÛÀ» ÀÛ¼ºÇÒ ±ÇÇÑÀÌ ¾ø½À´Ï´Ù."));
		}
		else
		{
			std::string str(c_pData + 1, length);
			pGuild->AddComment(ch, str);
		}

		return (1 + length);
	}

	case GUILD_SUBHEADER_CG_DELETE_COMMENT:
	{
		const DWORD comment_id = *reinterpret_cast<const DWORD*>(c_pData);

		pGuild->DeleteComment(ch, comment_id);
	}
	return SubPacketLen;

	case GUILD_SUBHEADER_CG_REFRESH_COMMENT:
		pGuild->RefreshComment(ch);
		return SubPacketLen;

	case GUILD_SUBHEADER_CG_CHANGE_MEMBER_GRADE:
	{
		const DWORD pid = *reinterpret_cast<const DWORD*>(c_pData);
		const BYTE grade = *(c_pData + sizeof(DWORD));
		const TGuildMember* m = pGuild->GetMember(ch->GetPlayerID());

		if (NULL == m)
			return -1;

		if (m->grade != GUILD_LEADER_GRADE)
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<±æµå> Á÷À§¸¦ º¯°æÇÒ ±ÇÇÑÀÌ ¾ø½À´Ï´Ù."));
		else if (ch->GetPlayerID() == pid)
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<±æµå> ±æµåÀåÀÇ Á÷À§´Â º¯°æÇÒ ¼ö ¾ø½À´Ï´Ù."));
		else if (grade == 1)
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<±æµå> ±æµåÀåÀ¸·Î Á÷À§¸¦ º¯°æÇÒ ¼ö ¾ø½À´Ï´Ù."));
		else
			pGuild->ChangeMemberGrade(pid, grade);
	}
	return SubPacketLen;

	case GUILD_SUBHEADER_CG_USE_SKILL:
	{
		const TPacketCGGuildUseSkill* p = reinterpret_cast<const TPacketCGGuildUseSkill*>(c_pData);

		pGuild->UseSkill(p->dwVnum, ch, p->dwPID);
	}
	return SubPacketLen;

	case GUILD_SUBHEADER_CG_CHANGE_MEMBER_GENERAL:
	{
		const DWORD pid = *reinterpret_cast<const DWORD*>(c_pData);
		const BYTE is_general = *(c_pData + sizeof(DWORD));
		const TGuildMember* m = pGuild->GetMember(ch->GetPlayerID());

		if (NULL == m)
			return -1;

		if (m->grade != GUILD_LEADER_GRADE)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<±æµå> Àå±ºÀ» ÁöÁ¤ÇÒ ±ÇÇÑÀÌ ¾ø½À´Ï´Ù."));
		}
		else
		{
			if (pGuild->m_guildMemberGeneralPulse > thecore_pulse())
			{
				int deltaInSeconds = ((pGuild->m_guildMemberGeneralPulse / PASSES_PER_SEC(1)) - (thecore_pulse() / PASSES_PER_SEC(1)));
				int minutes = deltaInSeconds / 60;
				int seconds = (deltaInSeconds - (minutes * 60));

				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You can select a new general in: %02d minutes and %02d seconds!"), minutes, seconds);
			}
			else
			{
				if (!pGuild->ChangeMemberGeneral(pid, is_general))
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<±æµå> ´õÀÌ»ó Àå¼ö¸¦ ÁöÁ¤ÇÒ ¼ö ¾ø½À´Ï´Ù."));
				}
			}
		}
	}
	return SubPacketLen;

	case GUILD_SUBHEADER_CG_GUILD_INVITE_ANSWER:
	{
		const DWORD guild_id = *reinterpret_cast<const DWORD*>(c_pData);
		const BYTE accept = *(c_pData + sizeof(DWORD));

		CGuild* g = CGuildManager::instance().FindGuild(guild_id);

		if (g)
		{
			if (accept)
				g->InviteAccept(ch);
			else
				g->InviteDeny(ch->GetPlayerID());
		}
	}
	return SubPacketLen;
	}

	return 0;
}

void CInputMain::Fishing(LPCHARACTER ch, const char* c_pData)
{
	TPacketCGFishing* p = (TPacketCGFishing*)c_pData;
	ch->SetRotation(p->dir * 5);
	ch->fishing();
	return;
}

void CInputMain::ItemGive(LPCHARACTER ch, const char* c_pData)
{
	TPacketCGGiveItem* p = (TPacketCGGiveItem*)c_pData;
	LPCHARACTER to_ch = CHARACTER_MANAGER::instance().Find(p->dwTargetVID);

	if (to_ch)
		ch->GiveItem(to_ch, p->ItemPos);
	else
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¾ÆÀÌÅÛÀ» °Ç³×ÁÙ ¼ö ¾ø½À´Ï´Ù."));
}

void CInputMain::Hack(LPCHARACTER ch, const char* c_pData)
{
	return;
	//TPacketCGHack* p = (TPacketCGHack*)c_pData;
	//if (!p || !p->szBuf || !strlen(p->szBuf))
	//	return;
//
	//if (ch->GetLastHackLogTime() != 0)
	//{
	//	if (ch->GetLastHackLogTime() < get_global_time())
	//	{
	//		LogManager::instance().HackLogEx(ch, "Trying sql attack(CInputMain::Hack)!...");
	//		return;
	//	}
	//}
//
	//char buf[sizeof(p->szBuf)];
	//strlcpy(buf, p->szBuf, sizeof(buf));
//
	//char __escape_string[1024];
	//DBManager::instance().EscapeString(__escape_string, sizeof(__escape_string), buf, strlen(p->szBuf));
//
	//char __escape_string2[1024];
	//if (strlen(p->szInfo))
	//{
	//	char info[sizeof(p->szInfo)];
	//	strlcpy(info, p->szInfo, sizeof(info));
//
	//	DBManager::instance().EscapeString(__escape_string2, sizeof(__escape_string2), info, strlen(p->szInfo));
	//}
	//else
	//	strcpy(__escape_string2, "-");
//
	//char szFinalStr[2048];
	//sprintf(szFinalStr, "%s|%s", __escape_string, __escape_string2);
//
	//LogManager::instance().HackLog(szFinalStr, ch);
	//ch->SetLastHackLogTime(get_global_time() + 5);
}

int CInputMain::MyShop(LPCHARACTER ch, const char* c_pData, size_t uiBytes)
{
	TPacketCGMyShop* p = (TPacketCGMyShop*)c_pData;
	int iExtraLen = p->bCount * sizeof(TShopItemTable);

	if (uiBytes < sizeof(TPacketCGMyShop) + iExtraLen)
		return -1;

	if (ch->GetGold() >= GOLD_MAX)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼ÒÀ¯ µ·ÀÌ 20¾ï³ÉÀ» ³Ñ¾î °Å·¡¸¦ ÇÛ¼ö°¡ ¾ø½À´Ï´Ù."));
		sys_log(0, "MyShop ==> OverFlow Gold id %u name %s ", ch->GetPlayerID(), ch->GetName());
		return (iExtraLen);
	}

	if (ch->IsStun() || ch->IsDead())
		return (iExtraLen);

#ifdef __GAME_MASTER_UTILITY__
	if (ch->IsGM() && !ch->MasterCanAction(ACTION_SHOP))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("MASTER_ACTION_SHOP"));
		return (iExtraLen);
	}
#endif

	if (quest::CQuestManager::instance().GetPCForce(ch->GetPlayerID())->IsRunning() == true)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_OPERATION_IS_FAILED_QUEST_WINDOW_IS_OPENED"));
		return (iExtraLen);
	}

	if (!ch->CanAct(true, true, VAR_TIME_NONE, nullptr, VAR_REFINE | VAR_SAFEBOX | VAR_EXCHANGE | VAR_ACCE | VAR_CHANGELOOK | VAR_OFFLINESHOP))
		return (iExtraLen);

	sys_log(0, "MyShop count %d", p->bCount);
	ch->OpenMyShop(p->szSign, (TShopItemTable*)(c_pData + sizeof(TPacketCGMyShop)), p->bCount);
	return (iExtraLen);
}

#ifdef __GLOBAL_MESSAGE_UTILITY__
void CInputMain::BulkWhisperManager(LPCHARACTER ch, const char* c_pData)
{
	TPacketCGBulkWhisper* f = (TPacketCGBulkWhisper*)c_pData;

	if (!ch)
		return;

	if (ch->GetGMLevel() != GM_IMPLEMENTOR)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[SYSTEM] GM degilsin."));
		return;
	}

	TPacketGGBulkWhisper p;
	p.bHeader = HEADER_GG_BULK_WHISPER;
	p.lSize = strlen(f->szText) + 1;

	TEMP_BUFFER tmpbuf;
	tmpbuf.write(&p, sizeof(p));
	tmpbuf.write(f->szText, p.lSize);

	char szEscaped[CHAT_MAX_LEN * 2 + 1];
	DBManager::instance().EscapeString(szEscaped, sizeof(szEscaped), f->szText, strlen(f->szText));
	DBManager::instance().Query(
		"INSERT INTO `log`.`log_pm_all_send%s` (`sender_pid`, `time`, `ip`, `content`, `channel`) "
		"VALUES (%u, NOW(), '%s', '%s', %d);",

		get_table_postfix(),
		ch->GetPlayerID(), ch->GetDesc() ? ch->GetDesc()->GetHostName() : "", szEscaped, g_bChannel
	);

	SendBulkWhisper(f->szText);
	P2P_MANAGER::instance().Send(tmpbuf.read_peek(), tmpbuf.size());
}
#endif

void CInputMain::Refine(LPCHARACTER ch, const char* c_pData)
{
	const TPacketCGRefine* p = reinterpret_cast<const TPacketCGRefine*>(c_pData);

	if (quest::CQuestManager::instance().GetPCForce(ch->GetPlayerID())->IsRunning() == true)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_OPERATION_IS_FAILED_QUEST_WINDOW_IS_OPENED"));
		ch->ClearRefineMode();
		return;
	}

	if (!ch->CanAct(true, true, VAR_TIME_NONE, nullptr, VAR_SAFEBOX | VAR_MYSHOP | VAR_EXCHANGE | VAR_ACCE | VAR_CHANGELOOK | VAR_OFFLINESHOP))
	{
		ch->ClearRefineMode();
		return;
	}

	if (ch->GetDungeon() || ch->GetWarMap() || IS_DUNGEON_ZONE(ch->GetMapIndex()))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You cannot open the safebox in dungeon or at war."));
		ch->ClearRefineMode();
		return;
	}

	if (p->type == 255)
	{
		// DoRefine Cancel
		ch->ClearRefineMode();
		return;
	}

	if (p->pos >= INVENTORY_MAX_NUM)
	{
		ch->ClearRefineMode();
		return;
	}

	LPITEM item = ch->GetInventoryItem(p->pos);

	if (!item)
	{
		ch->ClearRefineMode();
		return;
	}

	const TRefineTable* prt = CRefineManager::instance().GetRefineRecipe(item->GetRefineSet());// @duzenleme eger refine table mevcut degilse donusum gerceklesmez.
	if (!prt)
	{
		ch->ClearRefineMode();
		return;
	}
#ifdef __BEGINNER_ITEM__
	if (item->IsBasicItem())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ITEM_IS_BASIC_CANNOT_DO"));
		ch->ClearRefineMode();
		return;
	}
#endif
	ch->SetRefineTime();

	if (p->type == REFINE_TYPE_NORMAL)
	{
		sys_log(0, "refine_type_noraml");
		ch->DoRefine(item);
	}
	else if (p->type == REFINE_TYPE_SCROLL || p->type == REFINE_TYPE_HYUNIRON || p->type == REFINE_TYPE_MUSIN || p->type == REFINE_TYPE_BDRAGON
#ifdef __REFINE_SCROLL__
		|| p->type == REFINE_TYPE_PLUS_SCROLL
#endif
		)
	{
		sys_log(0, "refine_type_scroll, ...");
		ch->DoRefineWithScroll(item);
	}
	else if (p->type == REFINE_TYPE_MONEY_ONLY)
	{
		const LPITEM item = ch->GetInventoryItem(p->pos);

		if (NULL != item)
		{
			if (500 <= item->GetRefineSet())
			{
				LogManager::instance().HackLog("DEVIL_TOWER_REFINE_HACK", ch);
			}
			else
			{
				// @duzenleme uzaktan arti basmamalari icin boyle bir engel aldik (Rubinum Metin2AR)
				CHARACTER* refNPC = ch->GetRefineNPC();
				if (!refNPC) {
					LogManager::instance().HackLog("DT_DIRECT_REFINE_HACK", ch);
				}
				else if (!refNPC->CanReceiveItem(ch, item, true)) {
					LogManager::instance().HackLog("DT_TYPE_REFINE_HACK", ch);
					return;
				}

				if (ch->GetQuestFlag("deviltower_zone.can_refine"))
				{
					ch->DoRefine(item, true);
					ch->SetQuestFlag("deviltower_zone.can_refine", 0);
				}
				else
				{
					ch->ChatPacket(CHAT_TYPE_INFO, "»ç±Í Å¸¿ö ¿Ï·á º¸»óÀº ÇÑ¹ø±îÁö »ç¿ë°¡´ÉÇÕ´Ï´Ù.");
				}
			}
		}
	}

	ch->ClearRefineMode();
}

#ifdef __ACCE_SYSTEM__
void CInputMain::Acce(LPCHARACTER pkChar, const char* c_pData)
{
	if (!pkChar)
		return;

	quest::PC* pPC = quest::CQuestManager::instance().GetPCForce(pkChar->GetPlayerID());
	if (pPC->IsRunning())
		return;

	TPacketAcce* sPacket = (TPacketAcce*)c_pData;
	switch (sPacket->subheader)
	{
	case ACCE_SUBHEADER_CG_CLOSE:
	{
		pkChar->CloseAcce();
	}
	break;
	case ACCE_SUBHEADER_CG_ADD:
	{
		pkChar->AddAcceMaterial(sPacket->tPos, sPacket->bPos);
	}
	break;
	case ACCE_SUBHEADER_CG_REMOVE:
	{
		pkChar->RemoveAcceMaterial(sPacket->bPos);
	}
	break;
	case ACCE_SUBHEADER_CG_REFINE:
	{
		pkChar->RefineAcceMaterials();
	}
	break;
	default:
		break;
	}
}
#endif

#ifdef __ANCIENT_ATTR_ITEM__
void CInputMain::ItemNewAttributes(LPCHARACTER ch, const char* pcData)
{
	TPacketCGItemNewAttribute* p = (TPacketCGItemNewAttribute*)pcData;

	if (!ch)
		return;

	ch->UseItemNewAttribute(p->source_pos, p->target_pos, p->bValues);
}
#endif

#ifdef __CHEST_INFO_SYSTEM__
void CInputMain::ChestDropInfo(LPCHARACTER ch, const char* c_pData)
{
	TPacketCGChestDropInfo* p = (TPacketCGChestDropInfo*)c_pData;
	if (!ch)
		return;
	//Fix CrashCore
	const TItemTable* itemTable = ITEM_MANAGER::instance().GetTable(p->dwChestVnum);

	if (itemTable)
	{
		if (itemTable->bType != ITEM_GIFTBOX && itemTable->bType != ITEM_GACHA)
			return;
	}
	//Fix CrashCore
	std::vector<TChestDropInfoTable> vec_ItemList;
	ITEM_MANAGER::instance().GetChestItemList(p->dwChestVnum, vec_ItemList);
	if (vec_ItemList.size() == 0)
		return;
	TPacketGCChestDropInfo packet;
	packet.bHeader = HEADER_GC_CHEST_DROP_INFO;
	packet.wSize = sizeof(packet) + sizeof(TChestDropInfoTable) * vec_ItemList.size();
	packet.dwChestVnum = p->dwChestVnum;
	packet.pos = p->pos;
	ch->GetDesc()->BufferedPacket(&packet, sizeof(packet));
	ch->GetDesc()->Packet(&vec_ItemList[0], sizeof(TChestDropInfoTable) * vec_ItemList.size());
}
#endif

#ifdef __SKILL_CHOOSE_WINDOW__
void CInputMain::SkillChoosePacket(LPCHARACTER ch, const char* c_pData)
{
	TPacketCGSkillChoose* packet = (TPacketCGSkillChoose*)c_pData;

	if (!ch)
		return;

	if (packet->job >= 3)
	{
		LogManager::instance().HackLogEx(ch, "CInputMain::SkillChoosePacket try crash to game core. (job)");
		return;
	}

	if (ch->GetSkillGroup())
		return;

	if (ch->GetJob() == JOB_WOLFMAN)
		ch->SetSkillGroup(1);
	else
		ch->SetSkillGroup(packet->job);

	ch->ClearSkill();
	if (!g_bDisableResetSubSkill)
		ch->ClearSubSkill();
	ch->SetHorseLevel(21); // 21 level at
	ch->SetSkillLevel(121, 40); // liderlik
	ch->SetSkillLevel(122, 2); // kombo
	ch->SetSkillLevel(131, 10); // at cagirma
	ch->SetSkillLevel(137, 20); // at skill
	ch->SetSkillLevel(138, 20); // at skill
	ch->SetSkillLevel(139, 20); // at skill
	if (ch->GetJob() == JOB_ASSASSIN)
		ch->SetSkillLevel(140, 20); // at skill
	ch->SetSkillLevel(125, 40);
	ch->SetSkillLevel(141, 40);
	ch->SetSkillLevel(143, 40);
	ch->SetSkillLevel(144, 40);
	ch->SetSkillLevel(145, 40);
	ch->SetSkillLevel(146, 40);
	ch->SetSkillLevel(147, 40);
	ch->SetSkillLevel(148, 40);

#ifdef __SCP1453_EXTENSIONS__
	ch->SetAllSkillLevel(SKILL_START_LEVEL);
#endif
	sys_log(0, "CInputMain::SkillChoosePacket() ==> set playerid: %d skillgroup to %d", ch->GetPlayerID(), packet->job);
}
#endif

#ifdef __ITEM_CHANGELOOK__
void CInputMain::ChangeLook(LPCHARACTER pkChar, const char* c_pData)
{
	if (!pkChar)
		return;

	quest::PC* pPC = quest::CQuestManager::instance().GetPCForce(pkChar->GetPlayerID());
	if (pPC->IsRunning())
		return;

	TPacketChangeLook* sPacket = (TPacketChangeLook*)c_pData;
	switch (sPacket->subheader)
	{
	case CL_SUBHEADER_CLOSE:
	{
		pkChar->ChangeLookWindow(false);
	}
	break;
	case CL_SUBHEADER_ADD:
	{
		pkChar->AddClMaterial(sPacket->tPos, sPacket->bPos);
	}
	break;
	case CL_SUBHEADER_REMOVE:
	{
		pkChar->RemoveClMaterial(sPacket->bPos);
	}
	break;
	case CL_SUBHEADER_REFINE:
	{
		pkChar->RefineClMaterials();
	}
	break;
	default:
		break;
	}
}
#endif

#ifdef __SKILL_COLOR__
void CInputMain::SetSkillColor(LPCHARACTER ch, const char* pcData)
{
	if (!ch)
		return;

	TPacketCGSkillColor* p = (TPacketCGSkillColor*)pcData;

	if (p->skill > ESkillColorLength::MAX_SKILL_COUNT)
	{
		LogManager::instance().HackLogEx(ch, "CInputMain::SetSkillColor try crash to game core. (skill)");
		return;
	}

#ifdef ENABLE_PB2_PREMIUM_SYSTEM
	auto pkAffect = ch->FindAffect(AFFECT_PB2_SKILL_COLOR);
	if (!pkAffect)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Skill renklendirmeyi sadece beceri renklendirme boyasý kullananlar yapabilir.");
		return;
	}

#endif // ENABLE_PB2_PREMIUM_SYSTEM

	DWORD data[ESkillColorLength::MAX_SKILL_COUNT + ESkillColorLength::MAX_BUFF_COUNT][ESkillColorLength::MAX_EFFECT_COUNT];
	memcpy(data, ch->GetSkillColor(), sizeof(data));

	data[p->skill][0] = p->col1;
	data[p->skill][1] = p->col2;
	data[p->skill][2] = p->col3;
	data[p->skill][3] = p->col4;
	data[p->skill][4] = p->col5;

	ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You have changed the color of your skill."));

	ch->SetSkillColor(data[0]);

	TSkillColor db_pack;
	memcpy(db_pack.dwSkillColor, data, sizeof(data));
	db_pack.player_id = ch->GetPlayerID();
	db_clientdesc->DBPacketHeader(HEADER_GD_SKILL_COLOR_SAVE, 0, sizeof(TSkillColor));
	db_clientdesc->Packet(&db_pack, sizeof(TSkillColor));
}
#endif

#ifdef __DUNGEON_INFORMATION__
void CInputMain::DungeonTeleport(LPCHARACTER ch, const char* c_pData)
{
	if (!ch)
		return;

	if (!ch->GetDesc())
		return;

	if (!ch->CanAct(true, true, VAR_TIME_NONE))
		return;

	TPacketGCTeleport* p = (TPacketGCTeleport*)c_pData;
	DungeonInfo::instance().WarpPC(ch, p->dungeonIndex);
}
#endif

#ifdef __FAST_CHEQUE_TRANSFER__
void CInputMain::WonExchange(LPCHARACTER ch, const char* pcData)
{
	if (!ch)
		return;

	if (ch->GetDungeon() || ch->GetWarMap() || IS_DUNGEON_ZONE(ch->GetMapIndex()))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You cannot open the safebox in dungeon or at war."));
		return;
	}

	const TPacketCGWonExchange* p = reinterpret_cast<const TPacketCGWonExchange*>(pcData);
	const EWonExchangeCGSubHeader SubHeader = static_cast<EWonExchangeCGSubHeader>(p->bSubHeader);
	switch (SubHeader)
	{
	case WON_EXCHANGE_CG_SUBHEADER_SELL:
	case WON_EXCHANGE_CG_SUBHEADER_BUY:
		ch->WonExchange(SubHeader, p->wValue);
		break;
	default:
		sys_err("invalid won exchange subheader: %u value: %u", SubHeader, p->wValue);
		break;
	}
}
#endif

#ifdef __SWITCHBOT__
int CInputMain::Switchbot(LPCHARACTER ch, const char* data, size_t uiBytes)
{
	const TPacketCGSwitchbot* p = reinterpret_cast<const TPacketCGSwitchbot*>(data);

	if (int(quest::CQuestManager::instance().GetEventFlag("switchbot_kapat")) == 1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("SYSTEM_HAS_BEEN_DISABLED_BY_MASTER"));
		return 0;
	}

	if (uiBytes < sizeof(TPacketCGSwitchbot))
	{
		return -1;
	}

	const char* c_pData = data + sizeof(TPacketCGSwitchbot);
	uiBytes -= sizeof(TPacketCGSwitchbot);

	switch (p->subheader)
	{
	case SUBHEADER_CG_SWITCHBOT_START:
	{
		size_t extraLen = sizeof(TSwitchbotAttributeAlternativeTable) * SWITCHBOT_ALTERNATIVE_COUNT;
		if (uiBytes < extraLen)
		{
			return -1;
		}

		std::vector<TSwitchbotAttributeAlternativeTable> vec_alternatives;

		for (BYTE alternative = 0; alternative < SWITCHBOT_ALTERNATIVE_COUNT; ++alternative)
		{
			const TSwitchbotAttributeAlternativeTable* pAttr = reinterpret_cast<const TSwitchbotAttributeAlternativeTable*>(c_pData);
			c_pData += sizeof(TSwitchbotAttributeAlternativeTable);

			vec_alternatives.emplace_back(*pAttr);
		}

		if (quest::CQuestManager::instance().GetPCForce(ch->GetPlayerID())->IsRunning() == true)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_OPERATION_IS_FAILED_QUEST_WINDOW_IS_OPENED"));
			return extraLen;
		}

		if (!ch->CanAct(true, true, VAR_TIME_NONE))
			return extraLen;

		CSwitchbotManager::Instance().Start(ch->GetPlayerID(), p->slot, vec_alternatives);
		return extraLen;
	}

	case SUBHEADER_CG_SWITCHBOT_STOP:
	{
		CSwitchbotManager::Instance().Stop(ch->GetPlayerID(), p->slot);
		return 0;
	}

	case SUBHEADER_CG_SWITCHBOT_CHANGE_SPEED:
	{
		CSwitchbotManager::Instance().ChangeSpeed(ch->GetPlayerID(), p->slot);
		return 0;
	}
	}

	return 0;
}
#endif

#ifdef __CUBE_WINDOW__
void CInputMain::CubeRenewalSend(LPCHARACTER ch, const char* data)
{
	if (!ch)
		return;

	struct packet_send_cube_renewal* pinfo = (struct packet_send_cube_renewal*)data;
	switch (pinfo->subheader)
	{
	case CUBE_RENEWAL_SUB_HEADER_MAKE_ITEM:
	{
		int index_item = pinfo->index_item;
		int count_item = pinfo->count_item;
		int index_item_improve = pinfo->index_item_improve;

		Cube_Make(ch, index_item, count_item, index_item_improve);
	}
	break;

	case CUBE_RENEWAL_SUB_HEADER_CLOSE:
	{
		Cube_close(ch);
	}
	break;
	}
}
#endif

#ifdef __OFFLINE_SHOP__
#include "new_offlineshop.h"
#include "new_offlineshop_manager.h"
template <class T>
bool CanDecode(T* p, int buffleft) {
	return buffleft >= (int)sizeof(T);
}

template <class T>
const char* Decode(T*& pObj, const char* data, int* pbufferLeng = nullptr, int* piBufferLeft = nullptr)
{
	pObj = (T*)data;

	if (pbufferLeng)
		*pbufferLeng += sizeof(T);

	if (piBufferLeft)
		*piBufferLeft -= sizeof(T);

	return data + sizeof(T);
}

int OfflineshopPacketCreateNewShop(LPCHARACTER ch, const char* data, int iBufferLeft)
{
	offlineshop::TSubPacketCGShopCreate* pack = nullptr;
	if (!CanDecode(pack, iBufferLeft))
		return -1;

	int iExtra = 0;
	data = Decode(pack, data, &iExtra, &iBufferLeft);

	offlineshop::TShopInfo& rShopInfo = pack->shop;

	if (rShopInfo.dwCount < 0 || rShopInfo.dwCount > (OFFLINESHOP_MAX_ITEM_COUNT))
	{
		sys_err("BUG_DENEMESI %s count %d", ch->GetName(), rShopInfo.dwCount);
		return -1;
	}

	std::vector<offlineshop::TShopItemInfo> vec;
	vec.reserve(rShopInfo.dwCount);

	offlineshop::TShopItemInfo* pItem = nullptr;

	for (DWORD i = 0; i < rShopInfo.dwCount; ++i)
	{
		if (!CanDecode(pItem, iBufferLeft))
			return -1;

		data = Decode(pItem, data, &iExtra, &iBufferLeft);
		vec.push_back(*pItem);
	}

#ifdef __GAME_MASTER_UTILITY__
	if (ch->IsGM() && !ch->MasterCanAction(ACTION_OFFLINE_SHOP))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("MASTER_ACTION_OFFLINE_SHOP"));
		return iExtra;
	}
#endif

	offlineshop::CShopManager& rManager = offlineshop::GetManager();
	rManager.RecvShopCreateNewClientPacket(ch, rShopInfo, vec);

	return iExtra;
}

int OfflineshopPacketForceCloseShop(LPCHARACTER ch, const char* data, int iBufferLeft)
{
#ifdef __GAME_MASTER_UTILITY__
	if (ch->IsGM() && !ch->MasterCanAction(ACTION_OFFLINE_SHOP))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("MASTER_ACTION_OFFLINE_SHOP"));
		return 0;
	}
#endif

	offlineshop::CShopManager& rManager = offlineshop::GetManager();
	rManager.RecvShopForceCloseClientPacket(ch);

	return 0;
}

int OfflineshopPacketOpenShop(LPCHARACTER ch, const char* data, int iBufferLeft)
{
	offlineshop::TSubPacketCGShopOpen* pack = nullptr;
	if (!CanDecode(pack, iBufferLeft))
		return -1;

	int iExtra = 0;
	data = Decode(pack, data, &iExtra, &iBufferLeft);

#ifdef __GAME_MASTER_UTILITY__
	if (ch->IsGM() && !ch->MasterCanAction(ACTION_OFFLINE_SHOP))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("MASTER_ACTION_OFFLINE_SHOP"));
		return iExtra;
	}
#endif

	offlineshop::CShopManager& rManager = offlineshop::GetManager();
	rManager.RecvShopOpenClientPacket(ch, pack->dwOwnerID);

	return iExtra;
}

int OfflineshopPacketOpenShowOwner(LPCHARACTER ch, const char* data, int iBufferLeft)
{
#ifdef __GAME_MASTER_UTILITY__
	if (ch->IsGM() && !ch->MasterCanAction(ACTION_OFFLINE_SHOP))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("MASTER_ACTION_OFFLINE_SHOP"));
		return 0;
	}
#endif

	offlineshop::CShopManager& rManager = offlineshop::GetManager();
	rManager.RecvShopOpenMyShopClientPacket(ch);

	return 0;
}

int OfflineshopPacketBuyItem(LPCHARACTER ch, const char* data, int iBufferLeft)
{
	offlineshop::TSubPacketCGShopBuyItem* pack = nullptr;
	if (!CanDecode(pack, iBufferLeft))
		return -1;

	int iExtra = 0;
	data = Decode(pack, data, &iExtra, &iBufferLeft);

#ifdef __GAME_MASTER_UTILITY__
	if (ch->IsGM() && !ch->MasterCanAction(ACTION_OFFLINE_SHOP))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("MASTER_ACTION_OFFLINE_SHOP"));
		return iExtra;
	}
#endif

	offlineshop::CShopManager& rManager = offlineshop::GetManager();
	rManager.RecvShopBuyItemClientPacket(ch, pack->dwOwnerID, pack->dwItemID, pack->bIsSearch, pack->Price);

	return iExtra;
}

int OfflineshopPacketAddItem(LPCHARACTER ch, const char* data, int iBufferLeft)
{
	offlineshop::TSubPacketCGAddItem* pack = nullptr;
	if (!CanDecode(pack, iBufferLeft))
		return -1;

	int iExtra = 0;
	data = Decode(pack, data, &iExtra, &iBufferLeft);

#ifdef __GAME_MASTER_UTILITY__
	if (ch->IsGM() && !ch->MasterCanAction(ACTION_OFFLINE_SHOP))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("MASTER_ACTION_OFFLINE_SHOP"));
		return iExtra;
	}
#endif

	offlineshop::CShopManager& rManager = offlineshop::GetManager();
	rManager.RecvShopAddItemClientPacket(ch, pack->pos, pack->price);

	return iExtra;
}

int OfflineshopPacketRemoveItem(LPCHARACTER ch, const char* data, int iBufferLeft)
{
	offlineshop::TSubPacketCGRemoveItem* pack = nullptr;
	if (!CanDecode(pack, iBufferLeft))
		return -1;

	int iExtra = 0;
	data = Decode(pack, data, &iExtra, &iBufferLeft);

#ifdef __GAME_MASTER_UTILITY__
	if (ch->IsGM() && !ch->MasterCanAction(ACTION_OFFLINE_SHOP))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("MASTER_ACTION_OFFLINE_SHOP"));
		return iExtra;
	}
#endif

	offlineshop::CShopManager& rManager = offlineshop::GetManager();
	rManager.RecvShopRemoveItemClientPacket(ch, pack->dwItemID);

	return iExtra;
}

int OfflineshopPacketFilterRequest(LPCHARACTER ch, const char* data, int iBufferLeft)
{
	offlineshop::TSubPacketCGFilterRequest* pack = nullptr;
	if (!CanDecode(pack, iBufferLeft))
		return -1;

	int iExtra = 0;
	data = Decode(pack, data, &iExtra, &iBufferLeft);

#ifdef __GAME_MASTER_UTILITY__
	if (ch->IsGM() && !ch->MasterCanAction(ACTION_OFFLINE_SHOP))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("MASTER_ACTION_OFFLINE_SHOP"));
		return iExtra;
	}
#endif

	offlineshop::CShopManager& rManager = offlineshop::GetManager();
	rManager.RecvShopFilterRequestClientPacket(ch, pack->filter);

	return iExtra;
}

int OfflineshopPacketOpenSafebox(LPCHARACTER ch, const char* data, int iBufferLeft)
{
#ifdef __GAME_MASTER_UTILITY__
	if (ch->IsGM() && !ch->MasterCanAction(ACTION_OFFLINE_SHOP))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("MASTER_ACTION_OFFLINE_SHOP"));
		return 0;
	}
#endif

	offlineshop::CShopManager& rManager = offlineshop::GetManager();
	rManager.RecvShopSafeboxOpenClientPacket(ch);

	return 0;
}

int OfflineshopPacketCloseBoard(LPCHARACTER ch)
{
#ifdef __GAME_MASTER_UTILITY__
	if (ch->IsGM() && !ch->MasterCanAction(ACTION_OFFLINE_SHOP))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("MASTER_ACTION_OFFLINE_SHOP"));
		return 0;
	}
#endif

	offlineshop::CShopManager& rManager = offlineshop::GetManager();
	rManager.RecvCloseBoardClientPacket(ch);
	return 0;
}

int OfflineShopPacketTeleport(LPCHARACTER ch)
{
#ifdef __GAME_MASTER_UTILITY__
	if (ch->IsGM() && !ch->MasterCanAction(ACTION_OFFLINE_SHOP))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("MASTER_ACTION_OFFLINE_SHOP"));
		return 0;
	}
#endif

	offlineshop::CShopManager& rManager = offlineshop::GetManager();
	rManager.RecvTeleportClientPacket(ch);
	return 0;
}

int OfflineshopPacketGetItemSafebox(LPCHARACTER ch, const char* data, int iBufferLeft)
{
	offlineshop::TSubPacketCGShopSafeboxGetItem* pack = nullptr;
	if (!CanDecode(pack, iBufferLeft))
		return -1;

	int iExtra = 0;
	data = Decode(pack, data, &iExtra, &iBufferLeft);

#ifdef __GAME_MASTER_UTILITY__
	if (ch->IsGM() && !ch->MasterCanAction(ACTION_OFFLINE_SHOP))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("MASTER_ACTION_OFFLINE_SHOP"));
		return iExtra;
	}
#endif

	offlineshop::CShopManager& rManager = offlineshop::GetManager();
	rManager.RecvShopSafeboxGetItemClientPacket(ch, pack->dwItemID);

	return iExtra;
}

int OfflineshopPacketGetValutesSafebox(LPCHARACTER ch, const char* data, int iBufferLeft)
{
	offlineshop::TSubPacketCGShopSafeboxGetValutes* pack = nullptr;
	if (!CanDecode(pack, iBufferLeft))
		return -1;

	int iExtra = 0;
	data = Decode(pack, data, &iExtra, &iBufferLeft);

#ifdef __GAME_MASTER_UTILITY__
	if (ch->IsGM() && !ch->MasterCanAction(ACTION_OFFLINE_SHOP))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("MASTER_ACTION_OFFLINE_SHOP"));
		return iExtra;
	}
#endif

	offlineshop::CShopManager& rManager = offlineshop::GetManager();
	rManager.RecvShopSafeboxGetValutesClientPacket(ch, pack->valutes);

	return iExtra;
}

int OfflineshopPacketCloseSafebox(LPCHARACTER ch, const char* data, int iBufferLeft)
{
#ifdef __GAME_MASTER_UTILITY__
	if (ch->IsGM() && !ch->MasterCanAction(ACTION_OFFLINE_SHOP))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("MASTER_ACTION_OFFLINE_SHOP"));
		return 0;
	}
#endif

	offlineshop::CShopManager& rManager = offlineshop::GetManager();
	rManager.RecvShopSafeboxCloseClientPacket(ch);

	return 0;
}

#ifdef __OFFLINE_SHOP_ENTITY_CITIES__
int OfflineshopPacketClickEntity(LPCHARACTER ch, const char* data, int iBufferLeft)
{
	offlineshop::TSubPacketCGShopClickEntity* pack = nullptr;
	if (!CanDecode(pack, iBufferLeft))
		return -1;

	int iExtra = 0;
	data = Decode(pack, data, &iExtra, &iBufferLeft);

#ifdef __GAME_MASTER_UTILITY__
	if (ch->IsGM() && !ch->MasterCanAction(ACTION_OFFLINE_SHOP))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("MASTER_ACTION_OFFLINE_SHOP"));
		return iExtra;
	}
#endif

	offlineshop::CShopManager& rManager = offlineshop::GetManager();
	rManager.RecvShopClickEntity(ch, pack->dwShopVID);
	return iExtra;
}
#endif

int OfflineshopPacket(const char* data, LPCHARACTER ch, long iBufferLeft)
{
	if (iBufferLeft < sizeof(TPacketCGNewOfflineShop))
		return -1;

	TPacketCGNewOfflineShop* pPack = nullptr;
	iBufferLeft -= sizeof(TPacketCGNewOfflineShop);
	data = Decode(pPack, data);

	switch (pPack->bSubHeader)
	{
	case offlineshop::SUBHEADER_CG_SHOP_CREATE_NEW:				return /*sizeof(TPacketCGNewOfflineShop) +*/ OfflineshopPacketCreateNewShop(ch, data, iBufferLeft);
	case offlineshop::SUBHEADER_CG_SHOP_FORCE_CLOSE:			return /*sizeof(TPacketCGNewOfflineShop) +*/ OfflineshopPacketForceCloseShop(ch, data, iBufferLeft);
	case offlineshop::SUBHEADER_CG_SHOP_OPEN:					return /*sizeof(TPacketCGNewOfflineShop) +*/ OfflineshopPacketOpenShop(ch, data, iBufferLeft);
	case offlineshop::SUBHEADER_CG_SHOP_OPEN_OWNER:				return /*sizeof(TPacketCGNewOfflineShop) +*/ OfflineshopPacketOpenShowOwner(ch, data, iBufferLeft);

	case offlineshop::SUBHEADER_CG_SHOP_BUY_ITEM:				return /*sizeof(TPacketCGNewOfflineShop) +*/ OfflineshopPacketBuyItem(ch, data, iBufferLeft);
	case offlineshop::SUBHEADER_CG_SHOP_ADD_ITEM:				return /*sizeof(TPacketCGNewOfflineShop) +*/ OfflineshopPacketAddItem(ch, data, iBufferLeft);
	case offlineshop::SUBHEADER_CG_SHOP_REMOVE_ITEM:			return /*sizeof(TPacketCGNewOfflineShop) +*/ OfflineshopPacketRemoveItem(ch, data, iBufferLeft);

	case offlineshop::SUBHEADER_CG_SHOP_FILTER_REQUEST:			return /*sizeof(TPacketCGNewOfflineShop) +*/ OfflineshopPacketFilterRequest(ch, data, iBufferLeft);

	case offlineshop::SUBHEADER_CG_SHOP_SAFEBOX_OPEN:			return /*sizeof(TPacketCGNewOfflineShop) +*/ OfflineshopPacketOpenSafebox(ch, data, iBufferLeft);
	case offlineshop::SUBHEADER_CG_SHOP_SAFEBOX_GET_ITEM:		return /*sizeof(TPacketCGNewOfflineShop) +*/ OfflineshopPacketGetItemSafebox(ch, data, iBufferLeft);
	case offlineshop::SUBHEADER_CG_SHOP_SAFEBOX_GET_VALUTES:	return /*sizeof(TPacketCGNewOfflineShop) +*/ OfflineshopPacketGetValutesSafebox(ch, data, iBufferLeft);
	case offlineshop::SUBHEADER_CG_SHOP_SAFEBOX_CLOSE:			return /*sizeof(TPacketCGNewOfflineShop) +*/ OfflineshopPacketCloseSafebox(ch, data, iBufferLeft);

	case offlineshop::SUBHEADER_CG_CLOSE_BOARD:					return /*sizeof(TPacketCGNewOfflineshop) +*/ OfflineshopPacketCloseBoard(ch);
#ifdef __OFFLINE_SHOP_ENTITY_CITIES__
	case offlineshop::SUBHEADER_CG_CLICK_ENTITY:				return /*sizeof(TPacketCGNewOfflineshop) +*/ OfflineshopPacketClickEntity(ch, data, iBufferLeft);
#endif
	case offlineshop::SUBHEADER_CG_TELEPORT:					return /*sizeof(TPacketCGNewOfflineshop) +*/ OfflineShopPacketTeleport(ch);

	default:
		sys_err("UNKNOWN SUBHEADER %d ", pPack->bSubHeader);
		return -1;
	}
}
#endif

#ifdef __TARGET_BOARD_RENEWAL__
void CInputMain::SendMobInformation(LPCHARACTER ch, LPCHARACTER pkTarget, BYTE bType)
{
	if (!ch->GetDesc())
		return;

	if (bType == MOB_INFORMATION_DROP)
	{
		static std::vector<TMobInformationDropItem> s_vec_item;
		s_vec_item.clear();

		if (!ITEM_MANAGER::instance().CreateDropItemVector(pkTarget, ch, s_vec_item))
		{
			sys_err("!--> cannot create drop item vector %d - %d <--!", pkTarget->GetRaceNum(), ch->GetPlayerID());
			return;
		}

		if (s_vec_item.size() == 0)
			return;

		TPacketGCMobInformation pack;
		pack.header = HEADER_GC_MOB_INFO;
		pack.subheader = MOB_INFORMATION_DROP;
		pack.size = sizeof(TPacketGCMobInformation) + sizeof(TMobInformationDrop);

		TMobInformationDrop sub_pack;
		sub_pack.dwMobVnum = pkTarget->GetRaceNum();
		sub_pack.dwVID = pkTarget->GetVID();

		TMobInformationDropItem item_list[MOB_INFO_ITEM_LIST_MAX];
		memset(&item_list, 0, sizeof(item_list));

		for (DWORD i = 0; i < s_vec_item.size(); i++)
		{
			if (s_vec_item[i].vnum == 0)
				continue;

			TMobInformationDropItem item;
			item.vnum = s_vec_item[i].vnum;
			item.count = s_vec_item[i].count;
			item_list[i] = item;
		}

		memcpy(sub_pack.item_list, &item_list, sizeof(sub_pack.item_list));

		ch->GetDesc()->BufferedPacket(&pack, sizeof(pack));
		ch->GetDesc()->Packet(&sub_pack, sizeof(sub_pack));
	}
#ifdef __DUNGEON_INFORMATION__
	else if (bType == MOB_INFORMATION_DUNGEON_DROP)
	{
		static std::vector<TMobInformationDropItem> s_vec_item;
		s_vec_item.clear();

		if (!pkTarget->IsBoss())
		{
			M2_DESTROY_CHARACTER(pkTarget);
			return;
		}

		if (!ITEM_MANAGER::instance().CreateDropItemVector(pkTarget, ch, s_vec_item))
		{
			sys_err("!--> cannot create drop item vector %d - %d <--!", pkTarget->GetRaceNum(), ch->GetPlayerID());
			M2_DESTROY_CHARACTER(pkTarget);
			return;
		}

		if (s_vec_item.size() == 0)
		{
			M2_DESTROY_CHARACTER(pkTarget);
			return;
		}

		TPacketGCMobInformation pack;
		pack.header = HEADER_GC_MOB_INFO;
		pack.subheader = MOB_INFORMATION_DUNGEON_DROP;
		pack.size = sizeof(TPacketGCMobInformation) + sizeof(TMobInformationDrop);

		TMobInformationDrop sub_pack;
		sub_pack.dwMobVnum = pkTarget->GetRaceNum();
		sub_pack.dwVID = pkTarget->GetVID();

		TMobInformationDropItem item_list[MOB_INFO_ITEM_LIST_MAX];
		memset(&item_list, 0, sizeof(item_list));

		for (DWORD i = 0; i < s_vec_item.size(); i++)
		{
			if (s_vec_item[i].vnum == 0)
				continue;

			TMobInformationDropItem item;
			item.vnum = s_vec_item[i].vnum;
			item.count = s_vec_item[i].count;
			item_list[i] = item;
		}

		memcpy(sub_pack.item_list, &item_list, sizeof(sub_pack.item_list));

		ch->GetDesc()->BufferedPacket(&pack, sizeof(pack));
		ch->GetDesc()->Packet(&sub_pack, sizeof(sub_pack));

		M2_DESTROY_CHARACTER(pkTarget);
	}
#endif
	else if (bType == MOB_INFORMATION_BONUS)
	{
		TPacketGCMobInformation pack;
		pack.header = HEADER_GC_MOB_INFO;
		pack.subheader = MOB_INFORMATION_BONUS;
		pack.size = sizeof(TPacketGCMobInformation);

		ch->GetDesc()->Packet(&pack, sizeof(pack));
	}
	else if (bType == MOB_INFORMATION_SPECIALITY)
	{
		TPacketGCMobInformation pack;
		pack.header = HEADER_GC_MOB_INFO;
		pack.subheader = MOB_INFORMATION_SPECIALITY;
		pack.size = sizeof(TPacketGCMobInformation);

		ch->GetDesc()->Packet(&pack, sizeof(pack));
	}
}

void CInputMain::MobInformation(LPCHARACTER ch, const char* c_pData)
{
	TPacketCGMobInformation* p = (TPacketCGMobInformation*)c_pData;

	if (!ch)
		return;

	if (p->bType > MOB_INFORMATION_MAX)
	{
		LogManager::instance().HackLogEx(ch, "CInputMain::MobInformation try crash to game core. (bType)");
		return;
	}

	const CMob* pkMob = CMobManager::instance().Get(p->dwMobVnum);
	if (!pkMob)
	{
		sys_err("!--> cannot find pkMob %d - %d <--!", p->dwMobVnum, ch->GetPlayerID());
		return;
	}

#ifdef __DUNGEON_INFORMATION__
	LPCHARACTER pkTarget = NULL;
	if (p->bType != MOB_INFORMATION_DUNGEON_DROP)
	{
		pkTarget = CHARACTER_MANAGER::instance().Find(p->dwVirtualID);
		if (!pkTarget)
		{
			sys_err("!--> cannot find pkTargetCharacter %d - %d <--!", p->dwVirtualID, ch->GetPlayerID());
			return;
		}

		if (pkTarget->GetRaceNum() != p->dwMobVnum)
		{
			sys_err("!--> mob vnum is not same %d - %d - %d <--!", p->dwMobVnum, pkTarget->GetRaceNum(), ch->GetPlayerID());
			return;
		}
	}
	else
	{
		pkTarget = CHARACTER_MANAGER::instance().CreateCharacter(pkMob->m_table.szLocaleName);
		if (!pkTarget)
		{
			sys_err("!--> cannot find pkTargetCharacter %d - %d <--!", p->dwVirtualID, ch->GetPlayerID());
			return;
		}

		pkTarget->SetProto(pkMob);
	}
#else
	LPCHARACTER pkTarget = CHARACTER_MANAGER::instance().Find(p->dwVirtualID);
	if (!pkTarget)
	{
		sys_err("!--> cannot find pkTargetCharacter %d - %d <--!", p->dwVirtualID, ch->GetPlayerID());
		return;
	}

	if (pkTarget->GetRaceNum() != p->dwMobVnum)
	{
		sys_err("!--> mob vnum is not same %d - %d - %d <--!", p->dwMobVnum, pkTarget->GetRaceNum(), ch->GetPlayerID());
		return;
	}
#endif

	SendMobInformation(ch, pkTarget, p->bType);
}
#endif

#ifdef __BATTLE_PASS_SYSTEM__
int CInputMain::BattlePass(LPCHARACTER ch, const char* data, size_t uiBytes)
{
	TPacketCGBattlePassAction* p = (TPacketCGBattlePassAction*)data;

	if (uiBytes < sizeof(TPacketCGBattlePassAction))
		return -1;

	const char* c_pData = data + sizeof(TPacketCGBattlePassAction);
	uiBytes -= sizeof(TPacketCGBattlePassAction);

	switch (p->bAction)
	{
		case 1:
			CBattlePass::instance().BattlePassRequestOpen(ch);
			break;

		case 2:
			CBattlePass::instance().BattlePassRequestReward(ch);
			break;

		case 3:
		{
			DWORD dwPlayerId = ch->GetPlayerID();
			BYTE bIsGlobal = 0;

			db_clientdesc->DBPacketHeader(HEADER_GD_BATTLE_PASS_RANKING, ch->GetDesc()->GetHandle(), sizeof(DWORD) + sizeof(BYTE));
			db_clientdesc->Packet(&dwPlayerId, sizeof(DWORD));
			db_clientdesc->Packet(&bIsGlobal, sizeof(BYTE));
		}
		break;
		
		case 4: // skip
			CBattlePass::instance().BattlePassRequestSkip(ch, p->bSubAction);
			break;

	default:
		break;
	}

	return 0;
}
#endif

int CInputMain::Analyze(LPDESC d, BYTE bHeader, const char* c_pData)
{
	LPCHARACTER ch;

	if (!(ch = d->GetCharacter()))
	{
		sys_err("no character on desc");
		d->SetPhase(PHASE_CLOSE);
		return (0);
	}

	int iExtraLen = 0;

	if (test_server && bHeader != HEADER_CG_MOVE)
		sys_log(0, "CInputMain::Analyze() ==> Header [%d] ", bHeader);

	switch (bHeader)
	{
	case HEADER_CG_PONG:
		Pong(d);
		break;

	case HEADER_CG_TIME_SYNC:
		Handshake(d, c_pData);
		break;

	case HEADER_CG_CHAT:
		if (test_server)
		{
			char* pBuf = (char*)c_pData;
			sys_log(0, "%s", pBuf + sizeof(TPacketCGChat));
		}

		if ((iExtraLen = Chat(ch, c_pData, m_iBufferLeft)) < 0)
			return -1;
		break;

	case HEADER_CG_WHISPER:
		if ((iExtraLen = Whisper(ch, c_pData, m_iBufferLeft)) < 0)
			return -1;
		break;

	case HEADER_CG_MOVE:
		Move(ch, c_pData);
		break;

	case HEADER_CG_CHARACTER_POSITION:
		Position(ch, c_pData);
		break;

	case HEADER_CG_ITEM_USE:
		if (!ch->IsObserverMode())
			ItemUse(ch, c_pData);
		break;
#ifdef __DROP_DIALOG__
	case HEADER_CG_ITEM_DELETE:
		if (!ch->IsObserverMode())
			ItemDelete(ch, c_pData);
		break;

	case HEADER_CG_ITEM_SELL:
		if (!ch->IsObserverMode())
			ItemSell(ch, c_pData);
		break;
#endif
#ifdef __ACCE_SYSTEM__
	case HEADER_CG_ACCE:
		Acce(ch, c_pData);
		break;
#endif
#ifdef __ANCIENT_ATTR_ITEM__
	case HEADER_CG_ITEM_USE_NEW_ATTRIBUTE:
		if (!ch->IsObserverMode())
			ItemNewAttributes(ch, c_pData);
		break;
#endif
#ifdef __CHEST_INFO_SYSTEM__
	case HEADER_CG_CHEST_DROP_INFO:
		ChestDropInfo(ch, c_pData);
		break;
#endif
	case HEADER_CG_ITEM_MOVE:
		if (!ch->IsObserverMode())
			ItemMove(ch, c_pData);
		break;

	case HEADER_CG_ITEM_PICKUP:
		if (!ch->IsObserverMode())
			ItemPickup(ch, c_pData);
		break;

	case HEADER_CG_ITEM_USE_TO_ITEM:
		if (!ch->IsObserverMode())
			ItemToItem(ch, c_pData);
		break;

	case HEADER_CG_ITEM_GIVE:
		if (!ch->IsObserverMode())
			ItemGive(ch, c_pData);
		break;

	case HEADER_CG_EXCHANGE:
		if (!ch->IsObserverMode())
			Exchange(ch, c_pData);
		break;

	case HEADER_CG_ATTACK:
	case HEADER_CG_SHOOT:
		if (!ch->IsObserverMode())
		{
			Attack(ch, bHeader, c_pData);
		}
		break;

	case HEADER_CG_USE_SKILL:
		if (!ch->IsObserverMode())
			UseSkill(ch, c_pData);
		break;

	case HEADER_CG_QUICKSLOT_ADD:
		QuickslotAdd(ch, c_pData);
		break;

	case HEADER_CG_QUICKSLOT_DEL:
		QuickslotDelete(ch, c_pData);
		break;

	case HEADER_CG_QUICKSLOT_SWAP:
		QuickslotSwap(ch, c_pData);
		break;

	case HEADER_CG_SHOP:
		if ((iExtraLen = Shop(ch, c_pData, m_iBufferLeft)) < 0)
			return -1;
		break;

	case HEADER_CG_MESSENGER:
		if ((iExtraLen = Messenger(ch, c_pData, m_iBufferLeft)) < 0)
			return -1;
		break;

	case HEADER_CG_ON_CLICK:
		OnClick(ch, c_pData);
		break;

	case HEADER_CG_SYNC_POSITION:
		if ((iExtraLen = SyncPosition(ch, c_pData, m_iBufferLeft)) < 0)
			return -1;
		break;

	case HEADER_CG_ADD_FLY_TARGETING:
	case HEADER_CG_FLY_TARGETING:
		FlyTarget(ch, c_pData, bHeader);
		break;

	case HEADER_CG_SCRIPT_BUTTON:
		ScriptButton(ch, c_pData);
		break;

		// SCRIPT_SELECT_ITEM
	case HEADER_CG_SCRIPT_SELECT_ITEM:
		ScriptSelectItem(ch, c_pData);
		break;
		// END_OF_SCRIPT_SELECT_ITEM

	case HEADER_CG_SCRIPT_ANSWER:
		ScriptAnswer(ch, c_pData);
		break;

	case HEADER_CG_QUEST_INPUT_STRING:
		QuestInputString(ch, c_pData);
		break;

	case HEADER_CG_QUEST_CONFIRM:
		QuestConfirm(ch, c_pData);
		break;

	case HEADER_CG_TARGET:
		Target(ch, c_pData);
		break;

	case HEADER_CG_SAFEBOX_CHECKIN:
		SafeboxCheckin(ch, c_pData);
		break;

	case HEADER_CG_SAFEBOX_CHECKOUT:
		SafeboxCheckout(ch, c_pData, false);
		break;

	case HEADER_CG_SAFEBOX_ITEM_MOVE:
		SafeboxItemMove(ch, c_pData);
		break;

	case HEADER_CG_MALL_CHECKOUT:
		SafeboxCheckout(ch, c_pData, true);
		break;

	case HEADER_CG_PARTY_INVITE:
		PartyInvite(ch, c_pData);
		break;

	case HEADER_CG_PARTY_REMOVE:
		PartyRemove(ch, c_pData);
		break;

	case HEADER_CG_PARTY_INVITE_ANSWER:
		PartyInviteAnswer(ch, c_pData);
		break;

	case HEADER_CG_PARTY_SET_STATE:
		PartySetState(ch, c_pData);
		break;

	case HEADER_CG_PARTY_PARAMETER:
		PartyParameter(ch, c_pData);
		break;

	case HEADER_CG_ANSWER_MAKE_GUILD:
		AnswerMakeGuild(ch, c_pData);
		break;

	case HEADER_CG_GUILD:
		if ((iExtraLen = Guild(ch, c_pData, m_iBufferLeft)) < 0)
			return -1;
		break;

	case HEADER_CG_FISHING:
		Fishing(ch, c_pData);
		break;

	case HEADER_CG_HACK:
		Hack(ch, c_pData);
		break;

	case HEADER_CG_MYSHOP:
		if ((iExtraLen = MyShop(ch, c_pData, m_iBufferLeft)) < 0)
			return -1;
		break;

	case HEADER_CG_REFINE:
		Refine(ch, c_pData);
		break;

	case HEADER_CG_DRAGON_SOUL_REFINE:
	{
		if (!ch)
			return (iExtraLen);

		TPacketCGDragonSoulRefine* p = reinterpret_cast <TPacketCGDragonSoulRefine*>((void*)c_pData);
		switch (p->bSubType)
		{
		case DS_SUB_HEADER_CLOSE:
			ch->DragonSoul_RefineWindow_Close();
			break;
		case DS_SUB_HEADER_DO_REFINE_GRADE:
		{
			DSManager::instance().DoRefineGrade(ch, p->ItemGrid);
		}
		break;
		case DS_SUB_HEADER_DO_REFINE_STEP:
		{
			DSManager::instance().DoRefineStep(ch, p->ItemGrid);
		}
		break;
		case DS_SUB_HEADER_DO_REFINE_STRENGTH:
		{
			DSManager::instance().DoRefineStrength(ch, p->ItemGrid);
		}
		break;
		}
	}
	break;
#ifdef __OFFLINE_SHOP__
	case HEADER_CG_NEW_OFFLINESHOP:
		if ((iExtraLen = OfflineshopPacket(c_pData, ch, m_iBufferLeft)) < 0)
			return -1;
		break;
#endif

#ifdef __GLOBAL_MESSAGE_UTILITY__
	case HEADER_CG_BULK_WHISPER:
		BulkWhisperManager(ch, c_pData);
		break;
#endif
#ifdef __GROWTH_PET_SYSTEM__
	case HEADER_CG_PetSetName:
		BraveRequestPetName(ch, c_pData);
		break;
#endif
#ifdef __GROWTH_MOUNT_SYSTEM__
	case HEADER_CG_MountSetName:
		BraveRequestMountName(ch, c_pData);
		break;
#endif
#ifdef __SKILL_CHOOSE_WINDOW__
	case HEADER_CG_SKILLCHOOSE:
		SkillChoosePacket(ch, c_pData);
		break;
#endif
#ifdef __ITEM_CHANGELOOK__
	case HEADER_CG_CL:
		ChangeLook(ch, c_pData);
		break;
#endif
#ifdef __SKILL_COLOR__
	case HEADER_CG_SKILL_COLOR:
		SetSkillColor(ch, c_pData);
		break;
#endif
#ifdef __DUNGEON_INFORMATION__
	case HEADER_CG_TELEPORT:
		DungeonTeleport(ch, c_pData);
		break;
#endif
#ifdef __SWITCHBOT__
	case HEADER_CG_SWITCHBOT:
		if ((iExtraLen = Switchbot(ch, c_pData, m_iBufferLeft)) < 0)
			return -1;
		break;
#endif
#ifdef __CUBE_WINDOW__
	case HEADER_CG_CUBE_RENEWAL:
		CubeRenewalSend(ch, c_pData);
		break;
#endif
#ifdef __FAST_CHEQUE_TRANSFER__
	case HEADER_CG_WON_EXCHANGE:
		WonExchange(ch, c_pData);
		break;
#endif
#ifdef __TARGET_BOARD_RENEWAL__
	case HEADER_CG_MOB_INFO:
		MobInformation(ch, c_pData);
		break;
#endif
#ifdef __BATTLE_PASS_SYSTEM__
	case HEADER_CG_BATTLE_PASS:
		if ((iExtraLen = BattlePass(ch, c_pData, m_iBufferLeft)) < 0)
			return -1;
		break;
#endif
	}
	return (iExtraLen);
}

int CInputDead::Analyze(LPDESC d, BYTE bHeader, const char* c_pData)
{
	LPCHARACTER ch;

	if (!(ch = d->GetCharacter()))
	{
		sys_err("no character on desc");
		return 0;
	}

	int iExtraLen = 0;

	switch (bHeader)
	{
	case HEADER_CG_PONG:
		Pong(d);
		break;

	case HEADER_CG_TIME_SYNC:
		Handshake(d, c_pData);
		break;

	case HEADER_CG_CHAT:
		if ((iExtraLen = Chat(ch, c_pData, m_iBufferLeft)) < 0)
			return -1;

		break;

	case HEADER_CG_WHISPER:
		if ((iExtraLen = Whisper(ch, c_pData, m_iBufferLeft)) < 0)
			return -1;

		break;

	case HEADER_CG_HACK:
		Hack(ch, c_pData);
		break;

	default:
		return (0);
	}

	return (iExtraLen);
}



