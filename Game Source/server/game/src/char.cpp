#ifdef __WIN32__
#define NOMINMAX
#endif
#include "stdafx.h"

#include "../../common/VnumHelper.h"

#include "char.h"

#include "config.h"
#include "utils.h"
#include "crc32.h"
#include "char_manager.h"
#include "desc_client.h"
#include "desc_manager.h"
#include "buffer_manager.h"
#include "item_manager.h"
#include "motion.h"
#include "vector.h"
#include "packet.h"
#include "cmd.h"
#include "fishing.h"
#include "exchange.h"
#include "battle.h"
#include "affect.h"
#include "shop.h"
#include "shop_manager.h"
#include "safebox.h"
#include "regen.h"
#include "pvp.h"
#include "party.h"
#include "start_position.h"
#include "questmanager.h"
#include "log.h"
#include "p2p.h"
#include "guild.h"
#include "guild_manager.h"
#ifdef __SCP1453_EXTENSIONS__
#include "dungeon_new.h"
#else
#include "dungeon.h"
#endif
#include "messenger_manager.h"
#include "unique_item.h"
#include "priv_manager.h"
#include "war_map.h"
#include "target.h"
#include "mob_manager.h"
#include "mining.h"
#include "horsename_manager.h"
#include "gm.h"
#include "map_location.h"
#include "skill_power.h"
#include "buff_on_attributes.h"

#ifdef __PET_SYSTEM__
#include "PetSystem.h"
#endif
#include "DragonSoul.h"

#ifdef __GROWTH_PET_SYSTEM__
#include "New_PetSystem.h"
#endif

#ifdef __GROWTH_MOUNT_SYSTEM__
#include "New_MountSystem.h"
#endif

#ifdef __SAFEBOX_AUTO_SORT__
#include "../../libgame/include/grid.h"
#endif

#ifdef __SWITCHBOT__
#include "new_switchbot.h"
#endif

#ifdef __OFFLINE_SHOP__
#include "new_offlineshop.h"
#include "new_offlineshop_manager.h"
#endif

#ifdef __MOUNT_COSTUME_SYSTEM__
#include "MountSystem.h"
#endif

#ifdef __BATTLE_PASS_SYSTEM__
#include "battle_pass.h"
#endif

#include "game_events.h"


#include "item_shop.h"

extern const BYTE g_aBuffOnAttrPoints;
extern bool RaceToJob(unsigned race, unsigned* ret_job);

bool CAN_ENTER_ZONE(const LPCHARACTER& ch, int map_index);

bool CAN_ENTER_ZONE(const LPCHARACTER& ch, int map_index)
{
	if (ch->GetGMLevel() > GM_PLAYER)
		return true;

	switch (map_index)
	{
	case 301:
	case 302:
	case 303:
	case 304:
	{
		if (ch->GetLevel() < 90)
			return false;
	}
	break;

	case 72:
	case 73:
	case 207:
	{
		if (ch->GetLevel() < 75)
			return false;
	}
	break;
	}
	return true;
}

void ELPlainCoord_GetRotatedPixelPosition(long centerX, long centerY, float distance, float rotDegree, long* pdstX, long* pdstY)
{
	float rotRadian = float(3.141592 * rotDegree / 180.0f);
	*pdstX = static_cast<long>(centerX + distance * float(sin(static_cast<double>(rotRadian))));
	*pdstY = static_cast<long>(centerY + distance * float(cos(static_cast<double>(rotRadian))));
}

// <Factor> DynamicCharacterPtr member function definitions

LPCHARACTER DynamicCharacterPtr::Get() const {
	LPCHARACTER p = NULL;
	if (is_pc) {
		p = CHARACTER_MANAGER::instance().FindByPID(id);
	}
	else {
		p = CHARACTER_MANAGER::instance().Find(id);
	}
	return p;
}

DynamicCharacterPtr& DynamicCharacterPtr::operator=(LPCHARACTER character) {
	if (character == NULL) {
		Reset();
		return *this;
	}
	if (character->IsPC()) {
		is_pc = true;
		id = character->GetPlayerID();
	}
	else {
		is_pc = false;
		id = character->GetVID();
	}
	return *this;
}

CHARACTER::CHARACTER()
{
	m_stateIdle.Set(this, &CHARACTER::BeginStateEmpty, &CHARACTER::StateIdle, &CHARACTER::EndStateEmpty);
	m_stateMove.Set(this, &CHARACTER::BeginStateEmpty, &CHARACTER::StateMove, &CHARACTER::EndStateEmpty);
	m_stateBattle.Set(this, &CHARACTER::BeginStateEmpty, &CHARACTER::StateBattle, &CHARACTER::EndStateEmpty);

	Initialize();
}

CHARACTER::~CHARACTER()
{
	Destroy();
}

void CHARACTER::Initialize()
{
	CEntity::Initialize(ENTITY_CHARACTER);

	m_bNoOpenedShop = true;

	m_bOpeningSafebox = false;

	m_fSyncTime = get_float_time() - 3;
	m_dwPlayerID = 0;
	m_dwKillerPID = 0;

	m_pkRegen = NULL;
	regen_id_ = 0;
	m_posRegen.x = m_posRegen.y = m_posRegen.z = 0;
	m_posStart.x = m_posStart.y = 0;
	m_posDest.x = m_posDest.y = 0;
	m_fRegenAngle = 0.0f;

	m_pkMobData = NULL;
	m_pkMobInst = NULL;

	m_pkShop = NULL;
	m_pkChrShopOwner = NULL;
	m_pkMyShop = NULL;
	m_pkExchange = NULL;
	m_pkParty = NULL;
	m_pkPartyRequestEvent = NULL;

	m_pGuild = NULL;

	m_pkChrTarget = NULL;

	m_pkMuyeongEvent = NULL;

	m_pkWarpNPCEvent = NULL;
	m_pkDeadEvent = NULL;
	m_pkStunEvent = NULL;
	m_pkSaveEvent = NULL;
	m_pkRecoveryEvent = NULL;
	m_pkTimedEvent = NULL;
	m_pkFishingEvent = NULL;
	m_pkWarpEvent = NULL;

	// MINING
	m_pkMiningEvent = NULL;
	// END_OF_MINING

	m_pkPoisonEvent = NULL;
	m_pkFireEvent = NULL;

	m_pkAffectEvent = NULL;
	m_afAffectFlag = TAffectFlag(0, 0);

	m_pkDestroyWhenIdleEvent = NULL;

	m_pkChrSyncOwner = NULL;

	m_points = {};
	m_pointsInstant = {};

	m_bCharType = CHAR_TYPE_MONSTER;

	SetPosition(POS_STANDING);

	m_dwPlayStartTime = m_dwLastMoveTime = get_dword_time();

	GotoState(m_stateIdle);
	m_dwStateDuration = 1;

	m_dwLastAttackTime = get_dword_time() - 20000;

	m_bAddChrState = 0;

	m_pkChrStone = NULL;

	m_pkSafebox = NULL;
	m_iSafeboxSize = -1;
	m_iSafeboxLoadTime = 0;

	m_pkMall = NULL;
	m_iMallLoadTime = 0;

	m_posWarp.x = m_posWarp.y = m_posWarp.z = 0;
	m_lWarpMapIndex = 0;

	m_posExit.x = m_posExit.y = m_posExit.z = 0;
	m_lExitMapIndex = 0;

	m_pSkillLevels = NULL;

	m_dwMoveStartTime = 0;
	m_dwMoveDuration = 0;

	m_dwFlyTargetID = 0;

	m_dwNextStatePulse = 0;

	m_dwLastDeadTime = get_dword_time() - 180000;

	m_bSkipSave = false;

	m_bItemLoaded = false;

	m_bHasPoisoned = false;

	m_pkDungeon = NULL;
	m_iEventAttr = 0;

	m_bNowWalking = m_bWalking = false;
	ResetChangeAttackPositionTime();

	m_bDisableCooltime = false;

	m_iAlignment = 0;
	m_iRealAlignment = 0;

	m_iKillerModePulse = 0;
	m_bPKMode = PK_MODE_PEACE;

	m_dwQuestNPCVID = 0;
	m_dwQuestByVnum = 0;
	m_pQuestItem = NULL;

	m_dwUnderGuildWarInfoMessageTime = get_dword_time() - 60000;

	m_bUnderRefine = false;

	// REFINE_NPC
	m_dwRefineNPCVID = 0;
	// END_OF_REFINE_NPC

	m_bStaminaConsume = false;

	ResetChainLightningIndex();

	m_dwMountVnum = 0;
	m_chHorse = NULL;
	m_chRider = NULL;

	m_pWarMap = NULL;
	m_bChatCounter = 0;

	ResetStopTime();

	m_dwLastVictimSetTime = get_dword_time() - 3000;
	m_iMaxAggro = -100;

	m_bSendHorseLevel = 0;
	m_bSendHorseHealthGrade = 0;
	m_bSendHorseStaminaGrade = 0;

	m_dwLoginPlayTime = 0;

	m_posSafeboxOpen.x = -1000;
	m_posSafeboxOpen.y = -1000;

	// EQUIP_LAST_SKILL_DELAY
	m_dwLastSkillTime = get_dword_time();
	// END_OF_EQUIP_LAST_SKILL_DELAY

	// MOB_SKILL_COOLTIME
	memset(m_adwMobSkillCooltime, 0, sizeof(m_adwMobSkillCooltime));
	// END_OF_MOB_SKILL_COOLTIME

	m_isOpenSafebox = 0;

	m_iRefineTime = 0;

	m_iExchangeTime = 0;

	m_iSafeboxLoadTime = 0;

	m_iMyShopTime = 0;

	m_deposit_pulse = 0;

	m_strNewName = "";

	m_known_guild.clear();

	m_dwLogOffInterval = 0;

	m_bComboIndex = 0;
	m_dwSkipComboAttackByTime = 0;

	m_dwMountTime = 0;

	m_bIsLoadedAffect = false;
	cannot_dead = false;

	m_fAttMul = 1.0f;
	m_fDamMul = 1.0f;

	m_pointsInstant.iDragonSoulActiveDeck = -1;
	memset(&m_tvLastSyncTime, 0, sizeof(m_tvLastSyncTime));
	m_iSyncHackCount = 0;

	for (int i = 0; i < VAR_TIME_MAX_NUM; ++i)
	{
		m_iVarTime[i] = 0;
		m_bVarValue[i] = false;
	}

	LastCampFireUse = 0;
	LastStatResetUse = 0;
	LastFriendRequestTime = 0;

#ifdef __PET_SYSTEM__
	m_petSystem = 0;
	m_bIsPet = false;
#endif

#ifdef __WOLFMAN_CHARACTER__
	m_pkBleedingEvent = NULL;
	m_bHasBled = false;
#endif

#ifdef __RANK_SYSTEM__
	m_iRank = 0;
#endif

#ifdef __LANDRANK_SYSTEM__
	m_iLandRank = 0;
#endif

#ifdef __REBORN_SYSTEM__
	m_iReborn = 0;
#endif

#ifdef __OFFLINE_SHOP__
	m_pkOfflineShop = NULL;
	m_pkShopSafebox = NULL;
	//offlineshop-updated 03/08/19
	m_pkOfflineShopGuest = NULL;
	m_iOfflineShopTime = 0;
	m_iLastShopSearchPulse = 0;
	m_iShopSearchCounter = 0;
#endif

#ifdef __ACCE_SYSTEM__
	m_bAcceCombination = false;
	m_bAcceAbsorption = false;
	m_iAcceTime = 0;
#endif

#if defined(__GROWTH_PET_SYSTEM__) || defined(__GROWTH_MOUNT_SYSTEM__)
	m_stImmortalSt = 0;
#endif

#ifdef __GROWTH_PET_SYSTEM__
	m_newpetskillcd[0] = 0;
	m_newpetskillcd[1] = 0;
	m_newpetskillcd[2] = 0;
	m_newpetSystem = 0;
	m_bIsNewPet = false;
	m_eggvid = 0;
#endif

#ifdef __GROWTH_MOUNT_SYSTEM__
	m_newmountskillcd[0] = 0;
	m_newmountskillcd[1] = 0;
	m_newmountskillcd[2] = 0;
	m_newmountSystem = 0;
	m_bIsNewMount = false;
	m_Mounteggvid = 0;
#endif

#ifdef __ITEM_CHANGELOOK__
	m_bChangeLook = false;
#ifdef __MOUNT_CHANGELOOK__
	m_bMountChangeLook = false;
#endif
	m_iChangeLookTime = 0;
#endif

#ifdef __BRAVERY_CAPE_REWORK__
	m_iBraveryCapeUseCount = 0;
	m_dwBraveryCapeUseTime = 0;
#endif

#ifdef __SKILL_COLOR__
	memset(&m_dwSkillColor, 0, sizeof(m_dwSkillColor));
#endif

#ifdef __DSS_ACTIVE_EFFECT__
	m_dwDragonSoulActivateTime = 0;
#endif

#ifdef __MOUNT_COSTUME_SYSTEM__
	m_mountSystem = 0;
	m_bIsMount = false;
	m_bMountCounter = 0;
#endif

	m_dwLastHackLogTime = 0;

#ifdef __INGAME_MALL__
	m_dwRefreshDragonCoinTime = 0;
#endif

#ifdef __MARTY_ANTI_CMD_FLOOD__
	m_dwCmdAntiFloodCount = 0;
	m_dwCmdAntiFloodPulse = 0;
#endif

	// ItemUse Counter
	m_iLastItemUsePulse = 0;
	m_iItemUseCounter = 0;
#ifdef ENABLE_PLAYER_STATISTICS
	memset(&m_playerstatistics, 0, sizeof(m_playerstatistics));
#endif
#ifdef __BATTLE_PASS_SYSTEM__
	m_listBattlePass.clear();
	m_bIsLoadedBattlePass = false;
	m_dwBattlePassEndTime = 0;
	m_dwLoginTime = time(0);
	m_iBattlePassLoadFailCounter = 0;
#endif
#ifdef __TEAM_SYSTEM__
	m_iTeam = 0;
#endif
#ifdef __MONIKER_SYSTEM__
	memset(m_moniker, 0, MONIKER_MAX_LEN);
#endif
#ifdef ENABLE_LOVE_SYSTEM
	memset(m_love1, 0, 2);
	memset(m_love2, 0, 2);
#endif // ENABLE_LOVE_SYSTEM
#ifdef ENABLE_WORD_SYSTEM
	memset(m_word, 0, 24+1);
#endif // ENABLE_WORD_SYSTEM
#ifdef ENABLE_AUTO_HUNT_SYSTEM
	m_bHasAttacker = false;
	m_pkNormalPlayer = NULL;
#endif // ENABLE_AUTO_HUNT_SYSTEM
#ifdef ENABLE_MULTI_FARM_BLOCK
	m_bmultiFarmStatus = false;
#endif
}

void CHARACTER::Create(const char* c_pszName, DWORD vid, bool isPC)
{
	static int s_crc = 172814;

	char crc_string[128 + 1];
	snprintf(crc_string, sizeof(crc_string), "%s%p%d", c_pszName, this, ++s_crc);
	m_vid = VID(vid, GetCRC32(crc_string, strlen(crc_string)));

	if (isPC)
	{
		m_stName = c_pszName;
	}
}

void CHARACTER::Destroy()
{
	CloseMyShop();

	if (m_pkRegen)
	{
		if (m_pkDungeon) {
			// Dungeon regen may not be valid at this point
			if (m_pkDungeon->IsValidRegen(m_pkRegen, regen_id_)) {
				--m_pkRegen->count;
			}
		}
		else {
			// Is this really safe?
			--m_pkRegen->count;
		}
		m_pkRegen = NULL;
	}

	if (m_pkDungeon)
	{
		SetDungeon(NULL);
	}

#ifdef __PET_SYSTEM__
	if (m_petSystem)
	{
		m_petSystem->Destroy();
		delete m_petSystem;

		m_petSystem = 0;
	}
#endif

#ifdef __GROWTH_PET_SYSTEM__
	if (m_newpetSystem)
	{
		m_newpetSystem->Destroy();
		delete m_newpetSystem;

		m_newpetSystem = 0;
	}
#endif

#ifdef __GROWTH_MOUNT_SYSTEM__
	if (m_newmountSystem)
	{
		m_newmountSystem->Destroy();
		delete m_newmountSystem;

		m_newmountSystem = 0;
	}
#endif

#ifdef __MOUNT_COSTUME_SYSTEM__
	if (m_mountSystem)
	{
		m_mountSystem->Destroy();
		delete m_mountSystem;
		m_mountSystem = 0;
	}
#endif

	HorseSummon(false);

	if (GetRider())
		GetRider()->ClearHorseInfo();

	if (GetDesc())
		GetDesc()->BindCharacter(NULL);

	if (m_pkExchange)
		m_pkExchange->Cancel();

	SetVictim(NULL);

	if (GetShop())
	{
		GetShop()->RemoveGuest(this);
		SetShop(NULL);
	}

	ClearStone();
	ClearSync();
	ClearTarget();

	if (NULL == m_pkMobData)
	{
		DragonSoul_CleanUp();
		ClearItem();
	}

	// <Factor> m_pkParty becomes NULL after CParty destructor call!
	LPPARTY party = m_pkParty;
	if (party)
	{
		if (party->GetLeaderPID() == GetVID() && !IsPC())
		{
			M2_DELETE(party);
		}
		else
		{
			party->Unlink(this);

			if (!IsPC())
				party->Quit(GetVID());
		}

		SetParty(NULL);
	}

	if (m_pkMobInst)
	{
		M2_DELETE(m_pkMobInst);
		m_pkMobInst = NULL;
	}

	m_pkMobData = NULL;

	if (m_pkSafebox)
	{
		M2_DELETE(m_pkSafebox);
		m_pkSafebox = NULL;
	}

	if (m_pkMall)
	{
		M2_DELETE(m_pkMall);
		m_pkMall = NULL;
	}

	for (TMapBuffOnAttrs::iterator it = m_map_buff_on_attrs.begin(); it != m_map_buff_on_attrs.end(); it++)
	{
		if (NULL != it->second)
		{
			M2_DELETE(it->second);
		}
	}
	m_map_buff_on_attrs.clear();

	m_set_pkChrSpawnedBy.clear();

	StopMuyeongEvent();
	event_cancel(&m_pkWarpNPCEvent);
	event_cancel(&m_pkRecoveryEvent);
	event_cancel(&m_pkDeadEvent);
	event_cancel(&m_pkSaveEvent);
	event_cancel(&m_pkTimedEvent);
	event_cancel(&m_pkStunEvent);
	event_cancel(&m_pkFishingEvent);
	event_cancel(&m_pkPoisonEvent);
#ifdef __WOLFMAN_CHARACTER__
	event_cancel(&m_pkBleedingEvent);
#endif
	event_cancel(&m_pkFireEvent);
	event_cancel(&m_pkPartyRequestEvent);
	event_cancel(&m_pkWarpEvent);
	event_cancel(&m_pkMiningEvent);

	for (itertype(m_mapMobSkillEvent) it = m_mapMobSkillEvent.begin(); it != m_mapMobSkillEvent.end(); ++it)
	{
		LPEVENT pkEvent = it->second;
		event_cancel(&pkEvent);
	}
	m_mapMobSkillEvent.clear();

	ClearAffect();

	event_cancel(&m_pkDestroyWhenIdleEvent);

	if (m_pSkillLevels)
	{
		M2_DELETE_ARRAY(m_pSkillLevels);
		m_pSkillLevels = NULL;
	}

	CEntity::Destroy();

	if (GetSectree())
		GetSectree()->RemoveEntity(this);
}

const char* CHARACTER::GetName() const
{
	return m_stName.empty() ? (m_pkMobData ? m_pkMobData->m_table.szLocaleName : "") : m_stName.c_str();
}

#ifdef __ITEM_COUNT_LIMIT__
void CHARACTER::OpenMyShop(const char* c_pszSign, TShopItemTable* pTable, DWORD bItemCount)
#else
void CHARACTER::OpenMyShop(const char* c_pszSign, TShopItemTable* pTable, BYTE bItemCount)
#endif
{
	if (!CanHandleItem()) // @duzenleme Bu kisimlarin detaylandirilmasi gerekiyor.
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´Ù¸¥ °Å·¡Áß(Ã¢°í,±³È¯,»óÁ¡)¿¡´Â °³ÀÎ»óÁ¡À» »ç¿ëÇÒ ¼ö ¾ø½À´Ï´Ù."));
		return;
	}

	if (GetMyShop())
	{
		CloseMyShop();
		return;
	}

	quest::PC* pPC = quest::CQuestManager::instance().GetPCForce(GetPlayerID());

	if (pPC->IsRunning())
		return;

	if (bItemCount == 0)
		return;

#ifdef __GOLD_LIMIT_REWORK__
	long long nTotalMoney = 0;

	for (int n = 0; n < bItemCount; ++n)
	{
		nTotalMoney += static_cast<long long>((pTable + n)->price);
	}

	nTotalMoney += static_cast<long long>(GetGold());

	if (GOLD_MAX <= nTotalMoney)
	{
		//sys_err("[OVERFLOW_GOLD] Overflow (GOLD_MAX) id %u name %s", GetPlayerID(), GetName());
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("20¾ï ³ÉÀ» ÃÊ°úÇÏ¿© »óÁ¡À» ¿­¼ö°¡ ¾ø½À´Ï´Ù"));
		return;
	}
#else
	int64_t nTotalMoney = 0;

	for (int n = 0; n < bItemCount; ++n)
	{
		nTotalMoney += static_cast<int64_t>((pTable + n)->price);
	}

	nTotalMoney += static_cast<int64_t>(GetGold());

	if (GOLD_MAX <= nTotalMoney)
	{
		//sys_err("[OVERFLOW_GOLD] Overflow (GOLD_MAX) id %u name %s", GetPlayerID(), GetName());
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("20¾ï ³ÉÀ» ÃÊ°úÇÏ¿© »óÁ¡À» ¿­¼ö°¡ ¾ø½À´Ï´Ù"));
		return;
	}
#endif

#ifdef __CHEQUE_SYSTEM__
	DWORD nTotalCheque = 0;
	for (int n = 0; n < bItemCount; ++n)
		nTotalCheque += static_cast<DWORD>((pTable + n)->cheque);

	nTotalCheque += static_cast<DWORD>(GetCheque());
	if (CHEQUE_MAX <= nTotalCheque)
	{
		sys_err("[OVERFLOW_CHEQUE] Overflow (CHEQUE_MAX) id %u name %s", GetPlayerID(), GetName());
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("20¾ï ³ÉÀ» ÃÊ°úÇÏ¿© »óÁ¡À» ¿­¼ö°¡ ¾ø½À´Ï´Ù"));
		return;
	}
#endif

	char szSign[SHOP_SIGN_MAX_LEN + 1];
	strlcpy(szSign, c_pszSign, sizeof(szSign));

	m_stShopSign = szSign;

	if (m_stShopSign.length() == 0)
		return;

	std::set<TItemPos> cont;
	for (BYTE i = 0; i < bItemCount; ++i) // TODO:WORD
	{
		if (cont.find((pTable + i)->pos) != cont.end())
		{
			sys_err("MYSHOP: duplicate shop item detected! (name: %s)", GetName());
			return;
		}

		// ANTI_GIVE, ANTI_MYSHOP check
		LPITEM pkItem = GetItem((pTable + i)->pos);

		if (pkItem)
		{
			const TItemTable* item_table = pkItem->GetProto();

			if (item_table && (IS_SET(item_table->dwAntiFlags, ITEM_ANTIFLAG_GIVE | ITEM_ANTIFLAG_MYSHOP)))
			{
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("À¯·áÈ­ ¾ÆÀÌÅÛÀº °³ÀÎ»óÁ¡¿¡¼­ ÆÇ¸ÅÇÒ ¼ö ¾ø½À´Ï´Ù."));
				return;
			}

			if (pkItem->IsEquipped() == true)
			{
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀåºñÁßÀÎ ¾ÆÀÌÅÛÀº °³ÀÎ»óÁ¡¿¡¼­ ÆÇ¸ÅÇÒ ¼ö ¾ø½À´Ï´Ù."));
				return;
			}

			if (true == pkItem->isLocked())
			{
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("»ç¿ëÁßÀÎ ¾ÆÀÌÅÛÀº °³ÀÎ»óÁ¡¿¡¼­ ÆÇ¸ÅÇÒ ¼ö ¾ø½À´Ï´Ù."));
				return;
			}

#ifdef __BEGINNER_ITEM__
			if (pkItem->IsBasicItem()) {
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Can't trade with sealed item."));
				return;
			}
#endif
		}

		cont.insert((pTable + i)->pos);
	}

	if (!CountSpecifyItem(71049))
	{
		if (CountSpecifyItem(50200))
			RemoveSpecifyItem(50200, 1);
		else
			return;
	}

	if (m_pkExchange)
		m_pkExchange->Cancel();

	TPacketGCShopSign p;

	p.bHeader = HEADER_GC_SHOP_SIGN;
	p.dwVID = GetVID();
	strlcpy(p.szSign, c_pszSign, sizeof(p.szSign));
	PacketAround(&p, sizeof(TPacketGCShopSign));

	m_pkMyShop = CShopManager::instance().CreatePCShop(this, pTable, bItemCount);

	if (GetHorse())
	{
		HorseSummon(false, true);
	}
	else if (GetMountVnum())
	{
		RemoveAffect(AFFECT_MOUNT);
		RemoveAffect(AFFECT_MOUNT_BONUS);
	}
}

void CHARACTER::CloseMyShop()
{
	if (GetMyShop())
	{
		m_stShopSign.clear();
		CShopManager::instance().DestroyPCShop(this);
		m_pkMyShop = NULL;

		TPacketGCShopSign p;

		p.bHeader = HEADER_GC_SHOP_SIGN;
		p.dwVID = GetVID();
		p.szSign[0] = '\0';
		PacketAround(&p, sizeof(p));
	}
}

void EncodeMovePacket(TPacketGCMove& pack, DWORD dwVID, BYTE bFunc, BYTE bArg, DWORD x, DWORD y, DWORD dwDuration, DWORD dwTime, BYTE bRot)
{
	pack.bHeader = HEADER_GC_MOVE;
	pack.bFunc = bFunc;
	pack.bArg = bArg;
	pack.dwVID = dwVID;
	pack.dwTime = dwTime ? dwTime : get_dword_time();
	pack.bRot = bRot;
	pack.lX = x;
	pack.lY = y;
	pack.dwDuration = dwDuration;
}

void CHARACTER::RestartAtSamePos()
{
	if (m_bIsObserver)
		return;

	EncodeRemovePacket(this);
	EncodeInsertPacket(this);

	ENTITY_MAP::iterator it = m_map_view.begin();

	while (it != m_map_view.end())
	{
		LPENTITY entity = (it++)->first;

		EncodeRemovePacket(entity);
		if (!m_bIsObserver)
			EncodeInsertPacket(entity);

		if (entity->IsType(ENTITY_CHARACTER))
		{
			LPCHARACTER lpChar = (LPCHARACTER)entity;
			if (lpChar->IsPC() || lpChar->IsNPC() || lpChar->IsMonster())
			{
				if (!entity->IsObserverMode())
					entity->EncodeInsertPacket(this);
			}
		}
		else
		{
			if (!entity->IsObserverMode())
			{
				entity->EncodeInsertPacket(this);
			}
		}
	}
}

void CHARACTER::EncodeInsertPacket(LPENTITY entity)
{
	LPDESC d;

	if (!(d = entity->GetDesc()))
		return;

	LPCHARACTER ch = (LPCHARACTER)entity;
	ch->SendGuildName(GetGuild());

	TPacketGCCharacterAdd pack;

	pack.header = HEADER_GC_CHARACTER_ADD;
	pack.dwVID = m_vid;

#if defined(__MOB_AGGR_LVL_INFO__) && defined(__GROWTH_PET_SYSTEM__) && defined(__GROWTH_MOUNT_SYSTEM__)
	pack.dwLevel = 0;
	pack.dwAIFlag = 0;
	if (IsMonster() || IsStone() || IsNewPet() || IsNewMount())
	{
		pack.dwLevel = GetLevel();
		pack.dwAIFlag = IsMonster() ? GetAIFlag() : 0;
	}
#elif defined(__MOB_AGGR_LVL_INFO__) && !defined(__GROWTH_PET_SYSTEM__) && !defined(__GROWTH_MOUNT_SYSTEM__)
	pack.dwLevel = 0;
	pack.dwAIFlag = 0;
	if (IsMonster() || IsStone())
	{
		pack.dwLevel = GetLevel();
		pack.dwAIFlag = IsMonster() ? GetAIFlag() : 0;
	}
#endif

	pack.bType = GetCharType();
	pack.angle = GetRotation();
	pack.x = GetX();
	pack.y = GetY();
	pack.z = GetZ();
	pack.wRaceNum = GetRaceNum();

	if (IsPet()
#ifdef __GROWTH_PET_SYSTEM__
		|| IsNewPet()
#endif
#ifdef __MOUNT_COSTUME_SYSTEM__
		|| IsMount()
#endif
#ifdef __GROWTH_MOUNT_SYSTEM__
		|| IsNewMount()
#endif
		)
	{
		pack.bMovingSpeed = 150;
	}
	else
	{
		pack.bMovingSpeed = GetLimitPoint(POINT_MOV_SPEED);
	}
	pack.bAttackSpeed = GetLimitPoint(POINT_ATT_SPEED);
	pack.dwAffectFlag[0] = m_afAffectFlag.bits[0];
	pack.dwAffectFlag[1] = m_afAffectFlag.bits[1];

	pack.bStateFlag = m_bAddChrState;

	int iDur = 0;

	if (m_posDest.x != pack.x || m_posDest.y != pack.y)
	{
		iDur = (m_dwMoveStartTime + m_dwMoveDuration) - get_dword_time();

		if (iDur <= 0)
		{
			pack.x = m_posDest.x;
			pack.y = m_posDest.y;
		}
	}

	d->Packet(&pack, sizeof(pack));

	if (IsPC() == true || m_bCharType == CHAR_TYPE_NPC)
	{
		TPacketGCCharacterAdditionalInfo addPacket;
		memset(&addPacket, 0, sizeof(TPacketGCCharacterAdditionalInfo));

		addPacket.header = HEADER_GC_CHAR_ADDITIONAL_INFO;

		addPacket.vid = m_vid;

		// name
		strlcpy(addPacket.name, GetName(), sizeof(addPacket.name));

		// parts
		addPacket.parts[CHR_EQUIPPART_ARMOR] = GetPart(PART_MAIN);
		addPacket.parts[CHR_EQUIPPART_WEAPON] = GetPart(PART_WEAPON);
		addPacket.parts[CHR_EQUIPPART_HEAD] = GetPart(PART_HEAD);
		addPacket.parts[CHR_EQUIPPART_HAIR] = GetPart(PART_HAIR);
	#ifdef __ACCE_SYSTEM__
		addPacket.parts[CHR_EQUIPPART_ACCE] = GetPart(PART_ACCE);
		addPacket.parts[CHR_EQUIPPART_ACCE_EFFECT] = GetWear(WEAR_COSTUME_ACCE) != NULL ? GetWear(WEAR_COSTUME_ACCE)->GetSocket(ACCE_ABSORPTION_SOCKET) : 0;
	#endif
	#ifdef __AURA_COSTUME_SYSTEM__
		addPacket.parts[CHR_EQUIPPART_AURA] = GetPart(PART_AURA);
	#endif
	#ifdef __QUIVER_SYSTEM__
		addPacket.parts[CHR_EQUIPPART_ARROW] = GetWear(WEAR_ARROW) != NULL ? GetWear(WEAR_ARROW)->GetOriginalVnum() : 0;
	#endif

		// empire
		addPacket.bEmpire = m_bEmpire;

		// guild
		if (GetGuild() != NULL)
		{
			addPacket.dwGuildID = GetGuild()->GetID();
	#ifdef __GUILD_LEADER_SYSTEM__
			TGuildMember* getMember = GetGuild()->GetMember(GetPlayerID());
			if (getMember)
			{
				if (getMember->grade == GUILD_LEADER_GRADE)
					addPacket.guild_member_type = 1;
				else if (getMember->is_general)
					addPacket.guild_member_type = 2;
			}
	#endif
		}
		else
		{
			addPacket.dwGuildID = 0;
	#ifdef __GUILD_LEADER_SYSTEM__
			addPacket.guild_member_type = 0;
	#endif
		}

		// level
		addPacket.bLevel = GetLevel();

		// pkmode
		addPacket.bPKMode = m_bPKMode;

		// mountvnum
		addPacket.dwMountVnum = GetMountVnum();

		// alignment
		addPacket.alignment = m_iAlignment / 10;

	#ifdef __RANK_SYSTEM__
		addPacket.rank = m_iRank;
	#endif
	#ifdef __LANDRANK_SYSTEM__
		addPacket.land_rank = m_iLandRank;
	#endif
	#ifdef __REBORN_SYSTEM__
		addPacket.reborn = m_iReborn;
	#endif
	#ifdef __TEAM_SYSTEM__
		addPacket.team = m_iTeam;
	#endif
	#ifdef __MONIKER_SYSTEM__
		strlcpy(addPacket.moniker, GetMoniker(), sizeof(addPacket.moniker));
	#endif
	#ifdef ENABLE_LOVE_SYSTEM
		strlcpy(addPacket.love1, GetLove1(), sizeof(addPacket.love1));
		strlcpy(addPacket.love2, GetLove2(), sizeof(addPacket.love2));
	#endif // ENABLE_LOVE_SYSTEM
	#ifdef ENABLE_WORD_SYSTEM
		strlcpy(addPacket.word, GetWord(), sizeof(addPacket.word));
	#endif // ENABLE_WORD_SYSTEM
	#ifdef __ITEM_EVOLUTION__
		addPacket.itemEvolution[EVOLUTION_WEAPON] = GetWear(WEAR_WEAPON) != NULL ? GetWear(WEAR_WEAPON)->GetEvolution() : 0;
	#ifdef __ARMOR_EVOLUTION__
		addPacket.itemEvolution[EVOLUTION_ARMOR] = GetWear(WEAR_BODY) != NULL ? GetWear(WEAR_BODY)->GetEvolution() : 0;
	#endif
	#endif

	#ifdef __SKILL_COLOR__
		thecore_memcpy(addPacket.dwSkillColor, GetSkillColor(), sizeof(addPacket.dwSkillColor));
	#endif

	#ifdef __SHINING_ITEM_SYSTEM__
		addPacket.adwShining[CHR_SHINING_WEAPON] = GetWear(WEAR_SHINING_WEAPON) ? GetWear(WEAR_SHINING_WEAPON)->GetVnum() : 0;
		addPacket.adwShining[CHR_SHINING_ARMOR] = GetWear(WEAR_SHINING_ARMOR) ? GetWear(WEAR_SHINING_ARMOR)->GetVnum() : 0;
		addPacket.adwShining[CHR_SHINING_SPECIAL] = GetWear(WEAR_SHINING_SPECIAL) ? GetWear(WEAR_SHINING_SPECIAL)->GetVnum() : 0;
		addPacket.adwShining[CHR_SHINING_SPECIAL2] = GetWear(WEAR_SHINING_SPECIAL2) ? GetWear(WEAR_SHINING_SPECIAL2)->GetVnum() : 0;
		addPacket.adwShining[CHR_SHINING_SPECIAL3] = GetWear(WEAR_SHINING_SPECIAL3) ? GetWear(WEAR_SHINING_SPECIAL3)->GetVnum() : 0;
		addPacket.adwShining[CHR_SHINING_WING] = IsBlockMode(BLOCK_COSTUME_ACCE_EX) ? 0 : (GetWear(WEAR_SHINING_WING) ? GetWear(WEAR_SHINING_WING)->GetVnum() : 0);
	#endif

		d->Packet(&addPacket, sizeof(TPacketGCCharacterAdditionalInfo));
	}

	if (iDur)
	{
		TPacketGCMove pack;
		EncodeMovePacket(pack, GetVID(), FUNC_MOVE, 0, m_posDest.x, m_posDest.y, iDur, 0, (BYTE)(GetRotation() / 5));
		d->Packet(&pack, sizeof(pack));

		TPacketGCWalkMode p;
		p.vid = GetVID();
		p.header = HEADER_GC_WALK_MODE;
		p.mode = m_bNowWalking ? WALKMODE_WALK : WALKMODE_RUN;

		d->Packet(&p, sizeof(p));
	}

	if (entity->IsType(ENTITY_CHARACTER) && GetDesc())
	{
		LPCHARACTER ch = (LPCHARACTER)entity;
		if (ch->IsWalking())
		{
			TPacketGCWalkMode p;
			p.vid = ch->GetVID();
			p.header = HEADER_GC_WALK_MODE;
			p.mode = ch->m_bNowWalking ? WALKMODE_WALK : WALKMODE_RUN;
			GetDesc()->Packet(&p, sizeof(p));
		}
	}

	if (GetMyShop())
	{
		TPacketGCShopSign p;

		p.bHeader = HEADER_GC_SHOP_SIGN;
		p.dwVID = GetVID();
		strlcpy(p.szSign, m_stShopSign.c_str(), sizeof(p.szSign));
		d->Packet(&p, sizeof(TPacketGCShopSign));
	}

	if (entity->IsType(ENTITY_CHARACTER))
	{
		sys_log(3, "EntityInsert %s (RaceNum %d) (%d %d) TO %s",
			GetName(), GetRaceNum(), GetX() / SECTREE_SIZE, GetY() / SECTREE_SIZE, ((LPCHARACTER)entity)->GetName());
	}
}

void CHARACTER::EncodeRemovePacket(LPENTITY entity)
{
	if (entity->GetType() != ENTITY_CHARACTER)
		return;

	LPDESC d;

	if (!(d = entity->GetDesc()))
		return;

	TPacketGCCharacterDelete pack;

	pack.header = HEADER_GC_CHARACTER_DEL;
	pack.id = m_vid;

	d->Packet(&pack, sizeof(TPacketGCCharacterDelete));

	if (entity->IsType(ENTITY_CHARACTER))
		sys_log(3, "EntityRemove %s(%d) FROM %s", GetName(), (DWORD)m_vid, ((LPCHARACTER)entity)->GetName());
}

void CHARACTER::UpdatePacket()
{
	if (GetSectree() == NULL)
		return;

	TPacketGCCharacterUpdate pack;
	memset(&pack, 0, sizeof(TPacketGCCharacterUpdate));
	pack.header = HEADER_GC_CHARACTER_UPDATE;
	pack.vid = m_vid;

	pack.parts[CHR_EQUIPPART_ARMOR] = GetPart(PART_MAIN);
	pack.parts[CHR_EQUIPPART_WEAPON] = GetPart(PART_WEAPON);
	pack.parts[CHR_EQUIPPART_HEAD] = GetPart(PART_HEAD);
	pack.parts[CHR_EQUIPPART_HAIR] = GetPart(PART_HAIR);
#ifdef __ACCE_SYSTEM__
	pack.parts[CHR_EQUIPPART_ACCE] = GetPart(PART_ACCE);
	pack.parts[CHR_EQUIPPART_ACCE_EFFECT] = GetWear(WEAR_COSTUME_ACCE) != NULL ? GetWear(WEAR_COSTUME_ACCE)->GetSocket(ACCE_ABSORPTION_SOCKET) : 0;
#endif
#ifdef __AURA_COSTUME_SYSTEM__
	pack.parts[CHR_EQUIPPART_AURA] = GetPart(PART_AURA);
#endif
#ifdef __QUIVER_SYSTEM__
	pack.parts[CHR_EQUIPPART_ARROW] = GetWear(WEAR_ARROW) != NULL ? GetWear(WEAR_ARROW)->GetOriginalVnum() : 0;
#endif

	pack.bStateFlag = m_bAddChrState;
	pack.wMovingSpeed = GetLimitPoint(POINT_MOV_SPEED);
	pack.wAttackSpeed = GetLimitPoint(POINT_ATT_SPEED);

	pack.dwAffectFlag[0] = m_afAffectFlag.bits[0];
	pack.dwAffectFlag[1] = m_afAffectFlag.bits[1];

	pack.bPKMode = m_bPKMode;
	pack.dwMountVnum = GetMountVnum();
	pack.bLevel = GetLevel();

	pack.alignment = m_iAlignment / 10;
#ifdef __RANK_SYSTEM__
	pack.rank = m_iRank;
#endif
#ifdef __LANDRANK_SYSTEM__
	pack.land_rank = m_iLandRank;
#endif
#ifdef __REBORN_SYSTEM__
	pack.reborn = m_iReborn;
#endif
#ifdef __TEAM_SYSTEM__
	pack.team = m_iTeam;
#endif
#ifdef __MONIKER_SYSTEM__
	strlcpy(pack.moniker, GetMoniker(), sizeof(pack.moniker));
#endif
#ifdef ENABLE_LOVE_SYSTEM
	strlcpy(pack.love1, GetLove1(), sizeof(pack.love1));
	strlcpy(pack.love2, GetLove2(), sizeof(pack.love2));
#endif // ENABLE_LOVE_SYSTEM
#ifdef ENABLE_WORD_SYSTEM
	strlcpy(pack.word, GetWord(), sizeof(pack.word));
#endif // ENABLE_WORD_SYSTEM
	if (GetGuild() != NULL)
	{
		pack.dwGuildID = GetGuild()->GetID();
#ifdef __GUILD_LEADER_SYSTEM__
		TGuildMember* getMember = GetGuild()->GetMember(GetPlayerID());
		if (getMember)
		{
			if (getMember->grade == GUILD_LEADER_GRADE)
				pack.guild_member_type = 1;
			else if (getMember->is_general)
				pack.guild_member_type = 2;
		}
#endif
	}
	else
	{
		pack.dwGuildID = 0;
#ifdef __GUILD_LEADER_SYSTEM__
		pack.guild_member_type = 0;
#endif
	}

#ifdef __ITEM_EVOLUTION__
	pack.itemEvolution[EVOLUTION_WEAPON] = GetWear(WEAR_WEAPON) != NULL ? GetWear(WEAR_WEAPON)->GetEvolution() : 0;
#ifdef __ARMOR_EVOLUTION__
	pack.itemEvolution[EVOLUTION_ARMOR] = GetWear(WEAR_BODY) != NULL ? GetWear(WEAR_BODY)->GetEvolution() : 0;
#endif
#endif

#ifdef __SKILL_COLOR__
	thecore_memcpy(pack.dwSkillColor, GetSkillColor(), sizeof(pack.dwSkillColor));
#endif

#ifdef __SHINING_ITEM_SYSTEM__
	pack.adwShining[CHR_SHINING_WEAPON] = GetWear(WEAR_SHINING_WEAPON) ? GetWear(WEAR_SHINING_WEAPON)->GetVnum() : 0;
	pack.adwShining[CHR_SHINING_ARMOR] = GetWear(WEAR_SHINING_ARMOR) ? GetWear(WEAR_SHINING_ARMOR)->GetVnum() : 0;
	pack.adwShining[CHR_SHINING_SPECIAL] = GetWear(WEAR_SHINING_SPECIAL) ? GetWear(WEAR_SHINING_SPECIAL)->GetVnum() : 0;
	pack.adwShining[CHR_SHINING_SPECIAL2] = GetWear(WEAR_SHINING_SPECIAL2) ? GetWear(WEAR_SHINING_SPECIAL2)->GetVnum() : 0;
	pack.adwShining[CHR_SHINING_SPECIAL3] = GetWear(WEAR_SHINING_SPECIAL3) ? GetWear(WEAR_SHINING_SPECIAL3)->GetVnum() : 0;
	pack.adwShining[CHR_SHINING_WING] = IsBlockMode(BLOCK_COSTUME_ACCE_EX) ? 0 : (GetWear(WEAR_SHINING_WING) ? GetWear(WEAR_SHINING_WING)->GetVnum() : 0);
#endif

#ifdef ENABLE_AUTO_HUNT_SYSTEM
	if (HasAttacker())
	{
		if (GetNormalPlayer() != NULL)
			pack.hasAttacker = GetNormalPlayer()->GetVID();
		else
			pack.hasAttacker = 0;
	}
#endif // ENABLE_AUTO_HUNT_SYSTEM

	PacketAround(&pack, sizeof(pack));
}

LPCHARACTER CHARACTER::FindCharacterInView(const char* c_pszName, bool bFindPCOnly)
{
	ENTITY_MAP::iterator it = m_map_view.begin();

	for (; it != m_map_view.end(); ++it)
	{
		if (!it->first->IsType(ENTITY_CHARACTER))
			continue;

		LPCHARACTER tch = (LPCHARACTER)it->first;

		if (bFindPCOnly && tch->IsNPC())
			continue;

		if (!strcasecmp(tch->GetName(), c_pszName))
			return (tch);
	}

	return NULL;
}

void CHARACTER::SetPosition(int pos)
{
	if (pos == POS_STANDING)
	{
		REMOVE_BIT(m_bAddChrState, ADD_CHARACTER_STATE_DEAD);
		REMOVE_BIT(m_pointsInstant.instant_flag, INSTANT_FLAG_STUN);

		event_cancel(&m_pkDeadEvent);
		event_cancel(&m_pkStunEvent);
	}
	else if (pos == POS_DEAD)
		SET_BIT(m_bAddChrState, ADD_CHARACTER_STATE_DEAD);

	if (!IsStone())
	{
		switch (pos)
		{
		case POS_FIGHTING:
			GotoState(m_stateBattle);
			break;

		default:
			GotoState(m_stateIdle);
			break;
		}
	}

	m_pointsInstant.position = pos;
}

void CHARACTER::Save()
{
	if (!m_bSkipSave)
		CHARACTER_MANAGER::instance().DelayedSave(this);
}

void CHARACTER::CreatePlayerProto(TPlayerTable& tab)
{
	memset(&tab, 0, sizeof(TPlayerTable));

	if (GetNewName().empty())
	{
		strlcpy(tab.name, GetName(), sizeof(tab.name));
	}
	else
	{
		strlcpy(tab.name, GetNewName().c_str(), sizeof(tab.name));
	}

	strlcpy(tab.ip, GetDesc()->GetHostName(), sizeof(tab.ip));

	tab.id = m_dwPlayerID;
	tab.level = GetLevel();
	tab.level_step = GetPoint(POINT_LEVEL_STEP);
	tab.exp = GetExp();
	tab.gold = GetGold();
#ifdef __CHEQUE_SYSTEM__
	tab.cheque = GetCheque();
#endif
#ifdef ENABLE_CHAT_COLOR_SYSTEM
	tab.color = GetChatColor();
#endif // ENABLE_CHAT_COLOR_SYSTEM
	tab.job = m_points.job;
	tab.part_base = m_pointsInstant.bBasePart;
	tab.skill_group = m_points.skill_group;

	DWORD dwPlayedTime = (get_dword_time() - m_dwPlayStartTime);

	if (dwPlayedTime > 60000)
	{
		if (IsInSafezone())
		{
			if (GetRealAlignment() < 0)
			{
				if (IsEquipUniqueItem(UNIQUE_ITEM_FASTER_ALIGNMENT_UP_BY_TIME))
					UpdateAlignment(120 * (dwPlayedTime / 60000), false);
				else
					UpdateAlignment(60 * (dwPlayedTime / 60000), false);
			}
			else
				UpdateAlignment(5 * (dwPlayedTime / 60000), false);
		}

		SetRealPoint(POINT_PLAYTIME, GetRealPoint(POINT_PLAYTIME) + dwPlayedTime / 60000);
		ResetPlayTime(dwPlayedTime % 60000);
	}

#ifdef __BATTLE_PASS_SYSTEM__
	DWORD dwPlayedTimeForBattlePass = (time(0) - m_dwLoginTime);

	if (CGameEventsManager::instance().IsActivateEvent(EVENT_X2SURE) == true)
		dwPlayedTimeForBattlePass *= 2;

	BYTE bBattlePassId = GetBattlePassId();
	if (bBattlePassId)
	{
		DWORD dwUnUsed, dwPlayTimeCount;
		if(CBattlePass::instance().BattlePassMissionGetInfo(bBattlePassId, PLAYTIME, &dwUnUsed, &dwPlayTimeCount))
		{
			if (GetMissionProgress(PLAYTIME, bBattlePassId) < dwPlayTimeCount)
				UpdateMissionProgress(PLAYTIME, bBattlePassId, dwPlayedTimeForBattlePass, dwPlayTimeCount);
		}

#ifdef ENABLE_BATTLE_PASS_EX
		BYTE nextMissionId = GetNextMissionByType(PLAY_TIME1, PLAY_TIME30, bBattlePassId);
		if (nextMissionId != 0)
		{
			DWORD dwUnUsed, dwPlayTimeCount;
			if(CBattlePass::instance().BattlePassMissionGetInfo(bBattlePassId, nextMissionId, &dwUnUsed, &dwPlayTimeCount))
			{
				if (GetMissionProgress(nextMissionId, bBattlePassId) < dwPlayTimeCount)
					UpdateMissionProgress(nextMissionId, bBattlePassId, dwPlayedTimeForBattlePass, dwPlayTimeCount);
			}
		}
#endif // ENABLE_BATTLE_PASS_EX
	}

	CreateLoginTime();
#endif // __BATTLE_PASS_SYSTEM__

	tab.playtime = GetRealPoint(POINT_PLAYTIME);
	tab.lAlignment = m_iRealAlignment;
#ifdef __RANK_SYSTEM__
	tab.lRank = m_iRank;
#endif
#ifdef __LANDRANK_SYSTEM__
	tab.lLandRank = m_iLandRank;
#endif
#ifdef __REBORN_SYSTEM__
	tab.lReborn = m_iReborn;
#endif
#ifdef __TEAM_SYSTEM__
	tab.lTeam = m_iTeam;
#endif
#ifdef __MONIKER_SYSTEM__
	strcpy(tab.moniker, m_moniker);
#endif
#ifdef ENABLE_LOVE_SYSTEM
	strcpy(tab.love1, m_love1);
	strcpy(tab.love2, m_love2);
#endif // ENABLE_LOVE_SYSTEM
#ifdef ENABLE_WORD_SYSTEM
	strcpy(tab.word, m_word);
#endif // ENABLE_WORD_SYSTEM
	if (m_posWarp.x != 0 || m_posWarp.y != 0)
	{
		tab.x = m_posWarp.x;
		tab.y = m_posWarp.y;
		tab.z = 0;
		tab.lMapIndex = m_lWarpMapIndex;
	}
	else
	{
		tab.x = GetX();
		tab.y = GetY();
		tab.z = GetZ();
		tab.lMapIndex = GetMapIndex();
	}

	if (m_lExitMapIndex == 0)
	{
		tab.lExitMapIndex = tab.lMapIndex;
		tab.lExitX = tab.x;
		tab.lExitY = tab.y;
	}
	else
	{
		tab.lExitMapIndex = m_lExitMapIndex;
		tab.lExitX = m_posExit.x;
		tab.lExitY = m_posExit.y;
	}

	sys_log(0, "SAVE: %s %dx%d", GetName(), tab.x, tab.y);

	tab.st = GetRealPoint(POINT_ST);
	tab.ht = GetRealPoint(POINT_HT);
	tab.dx = GetRealPoint(POINT_DX);
	tab.iq = GetRealPoint(POINT_IQ);

	tab.stat_point = GetPoint(POINT_STAT);
	tab.skill_point = GetPoint(POINT_SKILL);
	tab.sub_skill_point = GetPoint(POINT_SUB_SKILL);
	tab.horse_skill_point = GetPoint(POINT_HORSE_SKILL);

#ifdef ENABLE_PLAYER_STATISTICS
	tab.iKilledShinsoo = GetKilledShinsoo();
	tab.iKilledChunjo = GetKilledChunjo();
	tab.iKilledJinno = GetKilledJinno();
	tab.iTotalKill = GetTotalKill();
	tab.iDuelWon = GetDuelWon();
	tab.iDuelLost = GetDuelLost();
	tab.iKilledMonster = GetKilledMonster();
	tab.iKilledStone = GetKilledStone();
	tab.iKilledBoss = GetKilledBoss();
	tab.iCompletedDungeon = GetCompletedDungeon();
	tab.iTakedFish = GetTakedFish();
	tab.iBestStoneDamage = GetBestStoneDamage();
	tab.iBestBossDamage = GetBestBossDamage();
#endif

	tab.stat_reset_count = GetPoint(POINT_STAT_RESET_COUNT);

	tab.hp = GetHP();
	tab.sp = GetSP();

	tab.stamina = GetStamina();

	tab.sRandomHP = m_points.iRandomHP;
	tab.sRandomSP = m_points.iRandomSP;

	if (m_PlayerSlots)
	{
		for (int i = 0; i < QUICKSLOT_MAX_NUM; ++i)
			tab.quickslot[i] = m_PlayerSlots->pQuickslot[i];
	}
	thecore_memcpy(tab.parts, m_pointsInstant.parts, sizeof(tab.parts));

	// REMOVE_REAL_SKILL_LEVLES
	thecore_memcpy(tab.skills, m_pSkillLevels, sizeof(TPlayerSkill) * SKILL_MAX_NUM);
	// END_OF_REMOVE_REAL_SKILL_LEVLES

	tab.horse = GetHorseData();

#ifdef __CHARACTER_WINDOW_RENEWAL__
	tab.last_play = get_global_time();
#endif
#ifdef __BATTLE_PASS_SYSTEM__
	tab.dwBattlePassEndTime = m_dwBattlePassEndTime;
#endif
}

void CHARACTER::SaveReal()
{
	if (m_bSkipSave)
		return;

	if (!GetDesc())
	{
		sys_err("Character::Save : no descriptor when saving (name: %s)", GetName());
		return;
	}

	TPlayerTable table;
	CreatePlayerProto(table);

	db_clientdesc->DBPacket(HEADER_GD_PLAYER_SAVE, GetDesc()->GetHandle(), &table, sizeof(TPlayerTable));

	quest::PC* pkQuestPC = quest::CQuestManager::instance().GetPCForce(GetPlayerID());

	if (!pkQuestPC)
		sys_err("CHARACTER::Save : null quest::PC pointer! (name %s)", GetName());
	else
	{
		pkQuestPC->Save();
	}
}

void CHARACTER::FlushDelayedSaveItem()
{
	LPITEM item;

	// ! neden sadece inventory??
	// k envanterde save edilecekleri de save etmemiz lazim
	for (int i = 0; i < INVENTORY_AND_EQUIP_SLOT_MAX; ++i)
		if ((item = GetInventoryItem(i)))
			ITEM_MANAGER::instance().FlushDelayedSave(item);
}

void CHARACTER::Disconnect(const char* c_pszReason)
{
	assert(GetDesc() != NULL);

	sys_log(0, "DISCONNECT: %s (%s)", GetName(), c_pszReason ? c_pszReason : "unset");

	if (GetShop())
	{
		GetShop()->RemoveGuest(this);
		SetShop(NULL);
	}

	if (GetParty() != NULL)
	{
		GetParty()->UpdateOfflineState(GetPlayerID());
	}

	// P2P Logout
	TPacketGGLogout p;
	p.bHeader = HEADER_GG_LOGOUT;
	strlcpy(p.szName, GetName(), sizeof(p.szName));
	P2P_MANAGER::instance().Send(&p, sizeof(TPacketGGLogout));

	if (m_pWarMap)
		SetWarMap(NULL);

#ifdef __OFFLINE_SHOP__
	offlineshop::GetManager().RemoveSafeboxFromCache(GetPlayerID());
	offlineshop::GetManager().RemoveGuestFromShops(this);

	if (GetOfflineShop())
		SetOfflineShop(NULL);

	SetShopSafebox(NULL);
#endif
	if (GetGuild())
		GetGuild()->LogoutMember(this);

	quest::CQuestManager::instance().LogoutPC(this);

	if (GetParty())
		GetParty()->Unlink(this);

	if (IsStun() || IsDead())
	{
		DeathPenalty(0);
		PointChange(POINT_HP, 50 - GetHP());
	}

	if (!CHARACTER_MANAGER::instance().FlushDelayedSave(this))
	{
		SaveReal();
	}

#ifdef ENABLE_MULTI_FARM_BLOCK
	CHARACTER_MANAGER::Instance().CheckMultiFarmAccount(GetDesc() ? GetDesc()->GetComputerOS() : "", GetPlayerID(), GetName(), false);
#endif

	FlushDelayedSaveItem();

	SaveAffect();
	m_bIsLoadedAffect = false;
	m_bSkipSave = true;

#ifdef __BATTLE_PASS_SYSTEM__
	SaveBattlePass();
	m_bIsLoadedBattlePass = false;
	m_iBattlePassLoadFailCounter = 0;
#endif

	quest::CQuestManager::instance().DisconnectPC(this);

	CloseSafebox();

	CloseMall();

	CPVPManager::instance().Disconnect(this);

	CTargetManager::instance().Logout(GetPlayerID());

	MessengerManager::instance().Logout(GetName());

#ifdef __AUTH_LOGIN_TIMEOUT__
	// insan giderken ben cikiyom db cigim der mk
	if (GetDesc()->GetAccountTable().login[0] && GetDesc()->GetAccountTable().passwd[0])
	{
		TLogoutPacket pack;

		strlcpy(pack.login, GetDesc()->GetAccountTable().login, sizeof(pack.login));
		strlcpy(pack.passwd, GetDesc()->GetAccountTable().passwd, sizeof(pack.passwd));

		db_clientdesc->DBPacket(HEADER_GD_LOGOUT, 0, &pack, sizeof(TLogoutPacket));
	}
#endif // __AUTH_LOGIN_TIMEOUT__

	if (GetDesc())
		GetDesc()->BindCharacter(NULL);

	M2_DESTROY_CHARACTER(this);
}

bool CHARACTER::Show(long lMapIndex, long x, long y, long z, bool bShowSpawnMotion/* = false */)
{
	LPSECTREE sectree = SECTREE_MANAGER::instance().Get(lMapIndex, x, y);

	if (!sectree)
	{
		sys_log(0, "cannot find sectree by %dx%d mapindex %d", x, y, lMapIndex);
		return false;
	}

	SetMapIndex(lMapIndex);

	bool bChangeTree = false;

	if (!GetSectree() || GetSectree() != sectree)
		bChangeTree = true;

	if (bChangeTree)
	{
		if (GetSectree())
			GetSectree()->RemoveEntity(this);

		ViewCleanup();
	}

	if (!IsNPC())
	{
		sys_log(0, "SHOW: %s %dx%dx%d", GetName(), x, y, z);
		if (GetStamina() < GetMaxStamina())
			StartAffectEvent();
	}
	else if (m_pkMobData)
	{
		m_pkMobInst->m_posLastAttacked.x = x;
		m_pkMobInst->m_posLastAttacked.y = y;
		m_pkMobInst->m_posLastAttacked.z = z;
	}

	if (bShowSpawnMotion)
	{
		SET_BIT(m_bAddChrState, ADD_CHARACTER_STATE_SPAWN);
		//m_afAffectFlag.Set(AFF_SPAWN);
	}

	SetXYZ(x, y, z);

	m_posDest.x = x;
	m_posDest.y = y;
	m_posDest.z = z;

	m_posStart.x = x;
	m_posStart.y = y;
	m_posStart.z = z;

	if (bChangeTree)
	{
		EncodeInsertPacket(this);
		sectree->InsertEntity(this);

		UpdateSectree();
	}
	else
	{
		ViewReencode();
		sys_log(0, "      in same sectree");
	}

	REMOVE_BIT(m_bAddChrState, ADD_CHARACTER_STATE_SPAWN);
	return true;
}

// BGM_INFO
struct BGMInfo
{
	std::string	name;
	float		vol;
};

typedef std::map<unsigned, BGMInfo> BGMInfoMap;

static BGMInfoMap 	gs_bgmInfoMap;
static bool		gs_bgmVolEnable = false;

void CHARACTER_SetBGMVolumeEnable()
{
	gs_bgmVolEnable = true;
	sys_log(0, "bgm_info.set_bgm_volume_enable");
}

void CHARACTER_AddBGMInfo(unsigned mapIndex, const char* name, float vol)
{
	BGMInfo newInfo;
	newInfo.name = name;
	newInfo.vol = vol;

	gs_bgmInfoMap[mapIndex] = newInfo;

	sys_log(0, "bgm_info.add_info(%d, '%s', %f)", mapIndex, name, vol);
}

const BGMInfo& CHARACTER_GetBGMInfo(unsigned mapIndex)
{
	BGMInfoMap::iterator f = gs_bgmInfoMap.find(mapIndex);
	if (gs_bgmInfoMap.end() == f)
	{
		static BGMInfo s_empty = { "", 0.0f };
		return s_empty;
	}
	return f->second;
}

bool CHARACTER_IsBGMVolumeEnable()
{
	return gs_bgmVolEnable;
}
// END_OF_BGM_INFO

void CHARACTER::MainCharacterPacket()
{
	const unsigned mapIndex = GetMapIndex();
	const BGMInfo& bgmInfo = CHARACTER_GetBGMInfo(mapIndex);

	TPacketGCMainCharacter mainChrPacket;
	mainChrPacket.header = HEADER_GC_MAIN_CHARACTER;
	mainChrPacket.dwVID = m_vid;
	mainChrPacket.wRaceNum = GetRaceNum();
	mainChrPacket.dwPlayerID = GetPlayerID();
	mainChrPacket.lx = GetX();
	mainChrPacket.ly = GetY();
	mainChrPacket.lz = GetZ();
	mainChrPacket.empire = GetDesc()->GetEmpire();
	mainChrPacket.skill_group = GetSkillGroup();
	strlcpy(mainChrPacket.szChrName, GetName(), sizeof(mainChrPacket.szChrName));
	if (!bgmInfo.name.empty())
	{
		strlcpy(mainChrPacket.szBGMName, bgmInfo.name.c_str(), sizeof(mainChrPacket.szBGMName));
		if (CHARACTER_IsBGMVolumeEnable())
			mainChrPacket.fBGMVol = bgmInfo.vol;
		else
			mainChrPacket.fBGMVol = 0.0f;
	}
	else
	{
		strlcpy(mainChrPacket.szBGMName, "", sizeof(mainChrPacket.szBGMName));
		mainChrPacket.fBGMVol = 0.0f;
	}

	GetDesc()->Packet(&mainChrPacket, sizeof(TPacketGCMainCharacter));
}

void CHARACTER::PointsPacket()
{
	if (!GetDesc())
		return;

	TPacketGCPoints pack;

	pack.header = HEADER_GC_CHARACTER_POINTS;

	pack.points[POINT_LEVEL] = GetLevel();
	pack.points[POINT_EXP] = GetExp();
	pack.points[POINT_NEXT_EXP] = GetNextExp();
	pack.points[POINT_HP] = GetHP();
	pack.points[POINT_MAX_HP] = GetMaxHP();
	pack.points[POINT_SP] = GetSP();
	pack.points[POINT_MAX_SP] = GetMaxSP();
	pack.points[POINT_GOLD] = GetGold();
	pack.points[POINT_STAMINA] = GetStamina();
	pack.points[POINT_MAX_STAMINA] = GetMaxStamina();

	for (int i = POINT_ST; i < POINT_MAX_NUM; ++i)
		pack.points[i] = GetPoint(i);

#ifdef __CHEQUE_SYSTEM__
	pack.points[POINT_CHEQUE] = GetCheque();
#endif

	GetDesc()->Packet(&pack, sizeof(TPacketGCPoints));
}

bool CHARACTER::ChangeSex()
{
	int src_race = GetRaceNum();

	switch (src_race)
	{
	case MAIN_RACE_WARRIOR_M:
		m_points.job = MAIN_RACE_WARRIOR_W;
		break;

	case MAIN_RACE_WARRIOR_W:
		m_points.job = MAIN_RACE_WARRIOR_M;
		break;

	case MAIN_RACE_ASSASSIN_M:
		m_points.job = MAIN_RACE_ASSASSIN_W;
		break;

	case MAIN_RACE_ASSASSIN_W:
		m_points.job = MAIN_RACE_ASSASSIN_M;
		break;

	case MAIN_RACE_SURA_M:
		m_points.job = MAIN_RACE_SURA_W;
		break;

	case MAIN_RACE_SURA_W:
		m_points.job = MAIN_RACE_SURA_M;
		break;

	case MAIN_RACE_SHAMAN_M:
		m_points.job = MAIN_RACE_SHAMAN_W;
		break;

	case MAIN_RACE_SHAMAN_W:
		m_points.job = MAIN_RACE_SHAMAN_M;
		break;

#ifdef __WOLFMAN_CHARACTER__
	case MAIN_RACE_WOLFMAN_M:
		m_points.job = MAIN_RACE_WOLFMAN_M;
		break;
#endif

	default:
		sys_err("CHANGE_SEX: %s unknown race %d", GetName(), src_race);
		return false;
	}

	sys_log(0, "CHANGE_SEX: %s (%d -> %d)", GetName(), src_race, m_points.job);
	return true;
}

DWORD CHARACTER::GetRaceNum() const
{
	if (m_pkMobData)
		return m_pkMobData->m_table.dwVnum;

	return m_points.job;
}

void CHARACTER::SetRace(BYTE race)
{
	if (race >= MAIN_RACE_MAX_NUM)
	{
		sys_err("CHARACTER::SetRace(name=%s, race=%d).OUT_OF_RACE_RANGE", GetName(), race);
		return;
	}

	m_points.job = race;
}

BYTE CHARACTER::GetJob() const
{
	unsigned race = m_points.job;
	unsigned job;

	if (RaceToJob(race, &job))
		return job;

	sys_err("CHARACTER::GetJob(name=%s, race=%d).OUT_OF_RACE_RANGE", GetName(), race);
	return JOB_WARRIOR;
}

void CHARACTER::SetLevel(BYTE level)
{
	m_points.level = level;

	if (IsPC())
	{
		if (level < PK_PROTECT_LEVEL)
			SetPKMode(PK_MODE_PROTECT);
		else if (GetGMLevel() != GM_PLAYER)
			SetPKMode(PK_MODE_PROTECT);
		else if (m_bPKMode == PK_MODE_PROTECT)
			SetPKMode(PK_MODE_PEACE);
	}
}

void CHARACTER::SetEmpire(BYTE bEmpire)
{
	m_bEmpire = bEmpire;
}

void CHARACTER::SetPlayerProto(const TPlayerTable* t)
{
	if (!GetDesc() || !*GetDesc()->GetHostName())
		sys_err("cannot get desc or hostname");
	else
		SetGMLevel();

	m_PlayerSlots = std::make_unique<PlayerSlotT>(); // @fixme199
	m_bCharType = CHAR_TYPE_PC;

	m_dwPlayerID = t->id;

	m_iAlignment = t->lAlignment;
	m_iRealAlignment = t->lAlignment;

#ifdef __RANK_SYSTEM__
	m_iRank = t->lRank;
#endif
#ifdef __LANDRANK_SYSTEM__
	m_iLandRank = t->lLandRank;
#endif
#ifdef __REBORN_SYSTEM__
	m_iReborn = t->lReborn;
#endif
#ifdef __TEAM_SYSTEM__
	m_iTeam = t->lTeam;
#endif
#ifdef __MONIKER_SYSTEM__
	strcpy(m_moniker, t->moniker);
#endif
#ifdef ENABLE_LOVE_SYSTEM
	strcpy(m_love1, t->love1);
	strcpy(m_love2, t->love2);
#endif // ENABLE_LOVE_SYSTEM
#ifdef ENABLE_WORD_SYSTEM
	strcpy(m_word, t->word);
#endif // ENABLE_WORD_SYSTEM
	m_points.skill_group = t->skill_group;

	m_pointsInstant.bBasePart = t->part_base;
	SetPart(PART_HAIR, t->parts[PART_HAIR]);
#ifdef __ACCE_SYSTEM__
	SetPart(PART_ACCE, t->parts[PART_ACCE]);
#endif
#ifdef __AURA_COSTUME_SYSTEM__
	SetPart(PART_AURA, t->parts[PART_AURA]);
#endif
	m_points.iRandomHP = t->sRandomHP;
	m_points.iRandomSP = t->sRandomSP;

	// REMOVE_REAL_SKILL_LEVLES
	if (m_pSkillLevels)
		M2_DELETE_ARRAY(m_pSkillLevels);

	m_pSkillLevels = M2_NEW TPlayerSkill[SKILL_MAX_NUM];
	thecore_memcpy(m_pSkillLevels, t->skills, sizeof(TPlayerSkill) * SKILL_MAX_NUM);
	// END_OF_REMOVE_REAL_SKILL_LEVLES

	if (t->lMapIndex >= 10000)
	{
		m_posWarp.x = t->lExitX;
		m_posWarp.y = t->lExitY;
		m_lWarpMapIndex = t->lExitMapIndex;
	}

	SetRealPoint(POINT_PLAYTIME, t->playtime);
	m_dwLoginPlayTime = t->playtime;
	SetRealPoint(POINT_ST, t->st);
	SetRealPoint(POINT_HT, t->ht);
	SetRealPoint(POINT_DX, t->dx);
	SetRealPoint(POINT_IQ, t->iq);

	SetPoint(POINT_ST, t->st);
	SetPoint(POINT_HT, t->ht);
	SetPoint(POINT_DX, t->dx);
	SetPoint(POINT_IQ, t->iq);

	SetPoint(POINT_STAT, t->stat_point);
	SetPoint(POINT_SKILL, t->skill_point);
	SetPoint(POINT_SUB_SKILL, t->sub_skill_point);
	SetPoint(POINT_HORSE_SKILL, t->horse_skill_point);

#ifdef ENABLE_PLAYER_STATISTICS
	m_playerstatistics.iKilledShinsoo = t->iKilledShinsoo;
	m_playerstatistics.iKilledChunjo = t->iKilledChunjo;
	m_playerstatistics.iKilledJinno = t->iKilledJinno;
	m_playerstatistics.iTotalKill = t->iTotalKill;
	m_playerstatistics.iDuelWon = t->iDuelWon;
	m_playerstatistics.iDuelLost = t->iDuelLost;
	m_playerstatistics.iKilledMonster = t->iKilledMonster;
	m_playerstatistics.iKilledStone = t->iKilledStone;
	m_playerstatistics.iKilledBoss = t->iKilledBoss;
	m_playerstatistics.iCompletedDungeon = t->iCompletedDungeon;
	m_playerstatistics.iTakedFish = t->iTakedFish;
	m_playerstatistics.iBestStoneDamage = t->iBestStoneDamage;
	m_playerstatistics.iBestBossDamage = t->iBestBossDamage;
#endif
#ifdef __BATTLE_PASS_SYSTEM__
	m_dwBattlePassEndTime = t->dwBattlePassEndTime;
#endif

	SetPoint(POINT_STAT_RESET_COUNT, t->stat_reset_count);

	SetPoint(POINT_LEVEL_STEP, t->level_step);
	SetRealPoint(POINT_LEVEL_STEP, t->level_step);

	SetRace(t->job);

	SetLevel(t->level);
	SetExp(t->exp);
	SetGold(t->gold);
#ifdef __CHEQUE_SYSTEM__
	SetCheque(t->cheque);
#endif
#ifdef ENABLE_CHAT_COLOR_SYSTEM
	SetChatColor(t->color);
#endif // ENABLE_CHAT_COLOR_SYSTEM
	SetMapIndex(t->lMapIndex);
	SetXYZ(t->x, t->y, t->z);

	ComputePoints();

	SetHP(t->hp);
	SetSP(t->sp);
	SetStamina(t->stamina);

#ifndef __MARTY_GM_FLAG_IF_TEST_SERVER__
	if (!test_server)
#endif
	{
#ifdef __GAME_MASTER_UTILITY__
		if (GetGMLevel() > GM_MOD)
#else
		if (GetGMLevel() > GM_LOW_WIZARD)
#endif
		{
			m_afAffectFlag.Set(AFF_YMIR);
			m_bPKMode = PK_MODE_PROTECT;
		}
	}

	if (GetLevel() < PK_PROTECT_LEVEL)
		m_bPKMode = PK_MODE_PROTECT;

	SetHorseData(t->horse);

	if (GetHorseLevel() > 0)
		UpdateHorseDataByLogoff(t->logoff_interval);

	thecore_memcpy(m_aiPremiumTimes, t->aiPremiumTimes, sizeof(t->aiPremiumTimes));

	m_dwLogOffInterval = t->logoff_interval;

	sys_log(0, "PLAYER_LOAD: %s PREMIUM %d %d, LOGGOFF_INTERVAL %u PTR: %p", t->name, m_aiPremiumTimes[0], m_aiPremiumTimes[1], t->logoff_interval, this);

#ifdef __PET_SYSTEM__
	if (m_petSystem)
	{
		m_petSystem->Destroy();
		delete m_petSystem;
	}

	m_petSystem = M2_NEW CPetSystem(this);
#endif

#ifdef __GROWTH_PET_SYSTEM__
	if (m_newpetSystem)
	{
		m_newpetSystem->Destroy();
		delete m_newpetSystem;
	}

	m_newpetSystem = M2_NEW CNewPetSystem(this);
#endif

#ifdef __GROWTH_MOUNT_SYSTEM__
	if (m_newmountSystem)
	{
		m_newmountSystem->Destroy();
		delete m_newmountSystem;
	}

	m_newmountSystem = M2_NEW CNewMountSystem(this);
#endif

#ifdef __MOUNT_COSTUME_SYSTEM__
	if (m_mountSystem)
	{
		m_mountSystem->Destroy();
		delete m_mountSystem;
	}
	m_mountSystem = M2_NEW CMountSystem(this);
#endif
}

EVENTFUNC(kill_ore_load_event)
{
	char_event_info* info = dynamic_cast<char_event_info*>(event->info);
	if (info == NULL)
	{
		sys_err("kill_ore_load_even> <Factor> Null pointer");
		return 0;
	}

	LPCHARACTER	ch = info->ch;
	if (ch == NULL) { // <Factor>
		return 0;
	}

	ch->m_pkMiningEvent = NULL;
	M2_DESTROY_CHARACTER(ch);
	return 0;
}

void CHARACTER::SetProto(const CMob* pkMob)
{
	if (m_pkMobInst)
		M2_DELETE(m_pkMobInst);

	m_pkMobData = pkMob;
	m_pkMobInst = M2_NEW CMobInstance;

	m_bPKMode = PK_MODE_FREE;

	const TMobTable* t = &m_pkMobData->m_table;

	m_bCharType = t->bType;

	SetLevel(t->bLevel);
	SetEmpire(t->bEmpire);

	SetExp(t->dwExp);
	SetRealPoint(POINT_ST, t->bStr);
	SetRealPoint(POINT_DX, t->bDex);
	SetRealPoint(POINT_HT, t->bCon);
	SetRealPoint(POINT_IQ, t->bInt);

	ComputePoints();

	SetHP(GetMaxHP());
	SetSP(GetMaxSP());

	////////////////////
	m_pointsInstant.dwAIFlag = t->dwAIFlag;
	SetImmuneFlag(t->dwImmuneFlag);

	AssignTriggers(t);

	ApplyMobAttribute(t);

	if (IsStone())
	{
		DetermineDropMetinStone();
	}

	if (IsWarp() || IsGoto())
	{
		StartWarpNPCEvent();
	}
	// @duzenleme
	// registerRaceNum map yapamayinca oyun core veriyordu.
	// Fixi Wom2 den cektik.
	try
	{
		CHARACTER_MANAGER::instance().RegisterRaceNumMap(this);
	}
	catch (...)
	{
		sys_err("Catched exception on RegisterRaceNumMap %s %d", this->GetName(), this->GetRaceNum());
	}

	// XXX CTF GuildWar hardcoding
	if (warmap::IsWarFlag(GetRaceNum()))
	{
		m_stateIdle.Set(this, &CHARACTER::BeginStateEmpty, &CHARACTER::StateFlag, &CHARACTER::EndStateEmpty);
		m_stateMove.Set(this, &CHARACTER::BeginStateEmpty, &CHARACTER::StateFlag, &CHARACTER::EndStateEmpty);
		m_stateBattle.Set(this, &CHARACTER::BeginStateEmpty, &CHARACTER::StateFlag, &CHARACTER::EndStateEmpty);
	}

	if (warmap::IsWarFlagBase(GetRaceNum()))
	{
		m_stateIdle.Set(this, &CHARACTER::BeginStateEmpty, &CHARACTER::StateFlagBase, &CHARACTER::EndStateEmpty);
		m_stateMove.Set(this, &CHARACTER::BeginStateEmpty, &CHARACTER::StateFlagBase, &CHARACTER::EndStateEmpty);
		m_stateBattle.Set(this, &CHARACTER::BeginStateEmpty, &CHARACTER::StateFlagBase, &CHARACTER::EndStateEmpty);
	}

	if (m_bCharType == CHAR_TYPE_HORSE ||
		GetRaceNum() == 20101 ||
		GetRaceNum() == 20102 ||
		GetRaceNum() == 20103 ||
		GetRaceNum() == 20104 ||
		GetRaceNum() == 20105 ||
		GetRaceNum() == 20106
		)
	{
		m_stateIdle.Set(this, &CHARACTER::BeginStateEmpty, &CHARACTER::StateHorse, &CHARACTER::EndStateEmpty);
		m_stateMove.Set(this, &CHARACTER::BeginStateEmpty, &CHARACTER::StateMove, &CHARACTER::EndStateEmpty);
		m_stateBattle.Set(this, &CHARACTER::BeginStateEmpty, &CHARACTER::StateHorse, &CHARACTER::EndStateEmpty);
	}

	// MINING
	if (mining::IsVeinOfOre(GetRaceNum()))
	{
		char_event_info* info = AllocEventInfo<char_event_info>();

		info->ch = this;

		m_pkMiningEvent = event_create(kill_ore_load_event, info, PASSES_PER_SEC(number(7 * 60, 15 * 60)));
	}
	// END_OF_MINING
}

const TMobTable& CHARACTER::GetMobTable() const
{
	return m_pkMobData->m_table;
}

bool CHARACTER::IsRaceFlag(DWORD dwBit) const
{
	if (!m_pkMobData)
		return 0;

	return m_pkMobData ? IS_SET(m_pkMobData->m_table.dwRaceFlag, dwBit) : 0;
}

DWORD CHARACTER::GetMobDamageMin() const
{
	if (!m_pkMobData)
		return 0;

	return m_pkMobData->m_table.dwDamageRange[0];
}

DWORD CHARACTER::GetMobDamageMax() const
{
	if (!m_pkMobData)
		return 0;

	return m_pkMobData->m_table.dwDamageRange[1];
}

float CHARACTER::GetMobDamageMultiply() const
{
	if (!m_pkMobData)
		return 0.0;

	float fDamMultiply = GetMobTable().fDamMultiply;

	if (IsBerserk())
		fDamMultiply = fDamMultiply * 2.0f;

	return fDamMultiply;
}

DWORD CHARACTER::GetMobDropItemVnum() const
{
	if (!m_pkMobData)
		return 0;
	return m_pkMobData->m_table.dwDropItemVnum;
}

bool CHARACTER::IsSummonMonster() const
{
	return GetSummonVnum() != 0;
}

DWORD CHARACTER::GetSummonVnum() const
{
	return m_pkMobData ? m_pkMobData->m_table.dwSummonVnum : 0;
}

DWORD CHARACTER::GetMonsterDrainSPPoint() const
{
	return m_pkMobData ? m_pkMobData->m_table.dwDrainSP : 0;
}

BYTE CHARACTER::GetMobRank() const
{
	if (!m_pkMobData)
		return MOB_RANK_KNIGHT;

	return m_pkMobData->m_table.bRank;
}

BYTE CHARACTER::GetMobSize() const
{
	if (!m_pkMobData)
		return MOBSIZE_MEDIUM;

	return m_pkMobData->m_table.bSize;
}

WORD CHARACTER::GetMobAttackRange() const
{
	switch (GetMobBattleType())
	{
	case BATTLE_TYPE_RANGE:
	case BATTLE_TYPE_MAGIC:
		return m_pkMobData->m_table.wAttackRange + GetPoint(POINT_BOW_DISTANCE);
	default:
		return m_pkMobData->m_table.wAttackRange;
	}
}

BYTE CHARACTER::GetMobBattleType() const
{
	if (!m_pkMobData)
		return BATTLE_TYPE_MELEE;

	return (m_pkMobData->m_table.bBattleType);
}

void CHARACTER::ComputeBattlePoints()
{
	if (IsPC())
	{
		SetPoint(POINT_ATT_GRADE, 0);
		SetPoint(POINT_DEF_GRADE, 0);
		SetPoint(POINT_CLIENT_DEF_GRADE, 0);
		SetPoint(POINT_MAGIC_ATT_GRADE, GetPoint(POINT_ATT_GRADE));
		SetPoint(POINT_MAGIC_DEF_GRADE, GetPoint(POINT_DEF_GRADE));


		//
		// ATK = 2lev + 2str
		//
		int iAtk = GetLevel() * 2;
		int iStatAtk = 0;

		switch (GetJob())
		{
		case JOB_WARRIOR:
		case JOB_SURA:
			iStatAtk = (2 * GetPoint(POINT_ST));
			break;

		case JOB_ASSASSIN:
			iStatAtk = (4 * GetPoint(POINT_ST) + 2 * GetPoint(POINT_DX)) / 3;
			break;

		case JOB_SHAMAN:
			iStatAtk = (4 * GetPoint(POINT_ST) + 2 * GetPoint(POINT_IQ)) / 3;
			break;
#ifdef __WOLFMAN_CHARACTER__
		case JOB_WOLFMAN:
			iStatAtk = (2 * GetPoint(POINT_ST));
			break;
#endif
		default:
			sys_err("invalid job %d", GetJob());
			iStatAtk = (2 * GetPoint(POINT_ST));
			break;
		}

		if (iStatAtk < 2 * GetPoint(POINT_ST))
			iStatAtk = (2 * GetPoint(POINT_ST));

		iAtk += iStatAtk;

		iAtk += (iAtk * GetHorseLevel()) / 30;

		//
		// ATK Setting
		//
		iAtk += GetPoint(POINT_ATT_GRADE_BONUS);

		PointChange(POINT_ATT_GRADE, iAtk);

		// DEF = LEV + CON + ARMOR
		int iShowDef = GetLevel() + GetPoint(POINT_HT);
		int iDef = GetLevel() + (int)(GetPoint(POINT_HT) / 1.25); // For Other
		int iArmor = 0;

		LPITEM pkItem;

		for (int i = 0; i < WEAR_MAX_NUM; ++i)
			if ((pkItem = GetWear(i)) && pkItem->GetType() == ITEM_ARMOR)
			{
				if (pkItem->GetSubType() == ARMOR_BODY || pkItem->GetSubType() == ARMOR_HEAD || pkItem->GetSubType() == ARMOR_FOOTS || pkItem->GetSubType() == ARMOR_SHIELD)
				{
					iArmor += pkItem->GetValue(1);
					iArmor += (2 * pkItem->GetValue(5));
				}
			}

		if (true == IsHorseRiding())
		{
			if (iArmor < GetHorseArmor())
				iArmor = GetHorseArmor();

			const char* pHorseName = CHorseNameManager::instance().GetHorseName(GetPlayerID());

			if (pHorseName != NULL && strlen(pHorseName))
			{
				iArmor += 20;
			}
		}

		iArmor += GetPoint(POINT_DEF_GRADE_BONUS);
		iArmor += GetPoint(POINT_PARTY_DEFENDER_BONUS);

		// INTERNATIONAL_VERSION
		PointChange(POINT_DEF_GRADE, iDef + iArmor);
		PointChange(POINT_CLIENT_DEF_GRADE, (iShowDef + iArmor) - GetPoint(POINT_DEF_GRADE));
		// END_OF_INTERNATIONAL_VERSION

		PointChange(POINT_MAGIC_ATT_GRADE, GetLevel() * 2 + GetPoint(POINT_IQ) * 2 + GetPoint(POINT_MAGIC_ATT_GRADE_BONUS));
		PointChange(POINT_MAGIC_DEF_GRADE, GetLevel() + (GetPoint(POINT_IQ) * 3 + GetPoint(POINT_HT)) / 3 + iArmor / 2 + GetPoint(POINT_MAGIC_DEF_GRADE_BONUS));
	}
	else
	{
		// 2lev + str * 2
		int iAtt = GetLevel() * 2 + GetPoint(POINT_ST) * 2;
		// lev + con
		int iDef = GetLevel() + GetPoint(POINT_HT) + GetMobTable().wDef;

		SetPoint(POINT_ATT_GRADE, iAtt);
		SetPoint(POINT_DEF_GRADE, iDef);
		SetPoint(POINT_MAGIC_ATT_GRADE, GetPoint(POINT_ATT_GRADE));
		SetPoint(POINT_MAGIC_DEF_GRADE, GetPoint(POINT_DEF_GRADE));
	}
}

void CHARACTER::ComputePoints()
{
	long lStat = GetPoint(POINT_STAT);
	long lStatResetCount = GetPoint(POINT_STAT_RESET_COUNT);
	long lSkillActive = GetPoint(POINT_SKILL);
	long lSkillSub = GetPoint(POINT_SUB_SKILL);
	long lSkillHorse = GetPoint(POINT_HORSE_SKILL);
	long lLevelStep = GetPoint(POINT_LEVEL_STEP);

	long lAttackerBonus = GetPoint(POINT_PARTY_ATTACKER_BONUS);
	long lTankerBonus = GetPoint(POINT_PARTY_TANKER_BONUS);
	long lBufferBonus = GetPoint(POINT_PARTY_BUFFER_BONUS);
	long lSkillMasterBonus = GetPoint(POINT_PARTY_SKILL_MASTER_BONUS);
	long lHasteBonus = GetPoint(POINT_PARTY_HASTE_BONUS);
	long lDefenderBonus = GetPoint(POINT_PARTY_DEFENDER_BONUS);
#ifdef __PARTY_ROLE_REWORK__
	long lAttackerMonsterBonus = GetPoint(POINT_PARTY_ATTACKER_MONSTER_BONUS);
	long lAttackerStoneBonus = GetPoint(POINT_PARTY_ATTACKER_STONE_BONUS);
	long lAttackerBossBonus = GetPoint(POINT_PARTY_ATTACKER_BOSS_BONUS);
#endif

	long lHPRecovery = GetPoint(POINT_HP_RECOVERY);
	long lSPRecovery = GetPoint(POINT_SP_RECOVERY);

	memset(m_pointsInstant.points, 0, sizeof(m_pointsInstant.points));
	BuffOnAttr_ClearAll();
	m_SkillDamageBonus.clear();

	SetPoint(POINT_STAT, lStat);
	SetPoint(POINT_SKILL, lSkillActive);
	SetPoint(POINT_SUB_SKILL, lSkillSub);
	SetPoint(POINT_HORSE_SKILL, lSkillHorse);
	SetPoint(POINT_LEVEL_STEP, lLevelStep);
	SetPoint(POINT_STAT_RESET_COUNT, lStatResetCount);

	SetPoint(POINT_ST, GetRealPoint(POINT_ST));
	SetPoint(POINT_HT, GetRealPoint(POINT_HT));
	SetPoint(POINT_DX, GetRealPoint(POINT_DX));
	SetPoint(POINT_IQ, GetRealPoint(POINT_IQ));

	SetPart(PART_MAIN, GetOriginalPart(PART_MAIN));
	SetPart(PART_WEAPON, GetOriginalPart(PART_WEAPON));
	SetPart(PART_HEAD, GetOriginalPart(PART_HEAD));
	SetPart(PART_HAIR, GetOriginalPart(PART_HAIR));

#ifdef __ACCE_SYSTEM__
	SetPart(PART_ACCE, GetOriginalPart(PART_ACCE));
#endif
#ifdef __AURA_COSTUME_SYSTEM__
	SetPart(PART_AURA, GetOriginalPart(PART_AURA));
#endif

	SetPoint(POINT_PARTY_ATTACKER_BONUS, lAttackerBonus);
	SetPoint(POINT_PARTY_TANKER_BONUS, lTankerBonus);
	SetPoint(POINT_PARTY_BUFFER_BONUS, lBufferBonus);
	SetPoint(POINT_PARTY_SKILL_MASTER_BONUS, lSkillMasterBonus);
	SetPoint(POINT_PARTY_HASTE_BONUS, lHasteBonus);
	SetPoint(POINT_PARTY_DEFENDER_BONUS, lDefenderBonus);
#ifdef __PARTY_ROLE_REWORK__
	SetPoint(POINT_PARTY_ATTACKER_MONSTER_BONUS, lAttackerMonsterBonus);
	SetPoint(POINT_PARTY_ATTACKER_STONE_BONUS, lAttackerStoneBonus);
	SetPoint(POINT_PARTY_ATTACKER_BOSS_BONUS, lAttackerBossBonus);
#endif

	SetPoint(POINT_HP_RECOVERY, lHPRecovery);
	SetPoint(POINT_SP_RECOVERY, lSPRecovery);

	HPTYPE iMaxHP;
	int iMaxSP;
	int iMaxStamina;

	if (IsPC())
	{
		iMaxHP = JobInitialPoints[GetJob()].max_hp + m_points.iRandomHP + GetPoint(POINT_HT) * JobInitialPoints[GetJob()].hp_per_ht;
		iMaxSP = JobInitialPoints[GetJob()].max_sp + m_points.iRandomSP + GetPoint(POINT_IQ) * JobInitialPoints[GetJob()].sp_per_iq;
		iMaxStamina = JobInitialPoints[GetJob()].max_stamina + GetPoint(POINT_HT) * JobInitialPoints[GetJob()].stamina_per_con;

		{
			CSkillProto* pkSk = CSkillManager::instance().Get(SKILL_ADD_HP);

			if (NULL != pkSk)
			{
				pkSk->SetPointVar("k", 1.0f * GetSkillPower(SKILL_ADD_HP) / 100.0f);

				iMaxHP += static_cast<int>(pkSk->kPointPoly.Eval());
			}
		}

		int iCastSpeed = 100;

		{
			CSkillProto* pkSk = CSkillManager::instance().Get(SKILL_SUB_CAST_SPEED);

			if (NULL != pkSk)
			{
				pkSk->SetPointVar("k", 1.0f * GetSkillPower(SKILL_SUB_CAST_SPEED) / 100.0f);

				iCastSpeed += static_cast<int>(pkSk->kPointPoly.Eval());
			}
		}

		SetPoint(POINT_MOV_SPEED, 330);

		SetPoint(POINT_MALL_ATTBONUS, 20);

		SetPoint(POINT_MAX_HP_PCT, 20);
		SetPoint(POINT_MAX_SP_PCT, 20);
		SetPoint(POINT_MALL_DEFBONUS, 15);
		SetPoint(POINT_CRITICAL_PCT, 100);
		SetPoint(POINT_PENETRATE_PCT, 100);
		SetPoint(POINT_RESIST_MAGIC, 30);

		SetPoint(POINT_ATT_GRADE_BONUS, 400);
		SetPoint(POINT_DEF_GRADE_BONUS, 200);
		SetPoint(POINT_ATT_SPEED, 350);
		PointChange(POINT_ATT_SPEED, GetPoint(POINT_PARTY_HASTE_BONUS));
		SetPoint(POINT_CASTING_SPEED, iCastSpeed);
	}
	else
	{
		iMaxHP = m_pkMobData->m_table.dwMaxHP;
		iMaxSP = 0;
		iMaxStamina = 0;

		SetPoint(POINT_ATT_SPEED, m_pkMobData->m_table.sAttackSpeed);
		if (IsHizli())
			SetPoint(POINT_MOV_SPEED, m_pkMobData->m_table.sMovingSpeed+100);
		else
			SetPoint(POINT_MOV_SPEED, m_pkMobData->m_table.sMovingSpeed);
		SetPoint(POINT_CASTING_SPEED, m_pkMobData->m_table.sAttackSpeed);
	}

	if (IsPC())
	{
#ifdef __MOUNT_COSTUME_SYSTEM__
		if (GetMountVnum() && !GetWear(WEAR_COSTUME_MOUNT))
#else
		if (GetMountVnum())
#endif
		{
			if (GetHorseST() > GetPoint(POINT_ST))
				PointChange(POINT_ST, GetHorseST() - GetPoint(POINT_ST));

			if (GetHorseDX() > GetPoint(POINT_DX))
				PointChange(POINT_DX, GetHorseDX() - GetPoint(POINT_DX));

			if (GetHorseHT() > GetPoint(POINT_HT))
				PointChange(POINT_HT, GetHorseHT() - GetPoint(POINT_HT));

			if (GetHorseIQ() > GetPoint(POINT_IQ))
				PointChange(POINT_IQ, GetHorseIQ() - GetPoint(POINT_IQ));
		}
	}

	ComputeBattlePoints();

	SetMaxStamina(iMaxStamina);

	m_pointsInstant.dwImmuneFlag = 0;

	for (int i = 0; i < WEAR_MAX_NUM; i++)
	{
		LPITEM pItem = GetWear(i);
		if (pItem)
		{
			pItem->ModifyPoints(true);
			SET_BIT(m_pointsInstant.dwImmuneFlag, GetWear(i)->GetImmuneFlag());
		}
	}

	if (DragonSoul_IsDeckActivated())
	{
		for (int i = WEAR_MAX_NUM + DS_SLOT_MAX * DragonSoul_GetActiveDeck();
			i < WEAR_MAX_NUM + DS_SLOT_MAX * (DragonSoul_GetActiveDeck() + 1); i++)
		{
			LPITEM pItem = GetWear(i);
			if (pItem)
			{
				if (DSManager::instance().IsTimeLeftDragonSoul(pItem))
					pItem->ModifyPoints(true);
			}
		}

#ifdef __DS_SET_BONUS__
		DragonSoul_HandleSetBonus(); // Bunu biz sonradan fix olarak yaptýk. Iþýnlanýnca bonuslar silinmesin diye.
#endif
	}

	if (iMaxHP != GetMaxHP())
	{
		SetRealPoint(POINT_MAX_HP, iMaxHP);
	}

	PointChange(POINT_MAX_HP, 0);

	if (iMaxSP != GetMaxSP())
	{
		SetRealPoint(POINT_MAX_SP, iMaxSP);
	}

	PointChange(POINT_MAX_SP, 0);

	ComputeSkillPoints();

	RefreshAffect();

	if (GetHP() > GetMaxHP())
		PointChange(POINT_HP, GetMaxHP() - GetHP());

	if (GetSP() > GetMaxSP())
		PointChange(POINT_SP, GetMaxSP() - GetSP());

#ifndef __PET_SYSTEM_PROTO__
	CPetSystem* pPetSystem = GetPetSystem();
	if (NULL != pPetSystem)
		pPetSystem->RefreshBuff();
#endif

	PointChange(POINT_MAX_HP, 0);
	PointChange(POINT_MAX_SP, 0);

	UpdatePacket();
}

void CHARACTER::ResetPlayTime(DWORD dwTimeRemain)
{
	m_dwPlayStartTime = get_dword_time() - dwTimeRemain;
}

const int aiRecoveryPercents[10] = { 1, 5, 5, 5, 5, 5, 5, 5, 5, 5 };

EVENTFUNC(recovery_event)
{
	char_event_info* info = dynamic_cast<char_event_info*>(event->info);
	if (info == NULL)
	{
		sys_err("recovery_event> <Factor> Null pointer");
		return 0;
	}

	LPCHARACTER	ch = info->ch;

	if (ch == NULL) { // <Factor>
		return 0;
	}

	if (!ch->IsPC())
	{
		if (ch->IsAffectFlag(AFF_POISON))
			return PASSES_PER_SEC(MAX(1, ch->GetMobTable().bRegenCycle));
#ifdef __WOLFMAN_CHARACTER__
		if (ch->IsAffectFlag(AFF_BLEEDING))
			return PASSES_PER_SEC(MAX(1, ch->GetMobTable().bRegenCycle));
#endif
		if (!ch->IsDoor())
		{
			ch->PointChange(POINT_HP, MAX(1, (ch->GetMaxHP() * ch->GetMobTable().bRegenPercent) / 100));
		}

		if (ch->GetHP() >= ch->GetMaxHP())
		{
			ch->m_pkRecoveryEvent = NULL;
			return 0;
		}

		return PASSES_PER_SEC(MAX(1, ch->GetMobTable().bRegenCycle));
	}
	else
	{
		ch->CheckTarget();
		//ch->UpdateSectree();
		ch->UpdateKillerMode();

		if (ch->IsAffectFlag(AFF_POISON) == true)
			return 3;

#ifdef __WOLFMAN_CHARACTER__
		if (ch->IsAffectFlag(AFF_BLEEDING))
			return 3;
#endif
		int iSec = (get_dword_time() - ch->GetLastMoveTime()) / 3000;

		ch->DistributeSP(ch);

		if (ch->GetMaxHP() <= ch->GetHP())
			return PASSES_PER_SEC(3);

		int iPercent = 0;
		HPTYPE iAmount = 0;

		{
			iPercent = aiRecoveryPercents[MIN(9, iSec)];
			iAmount = 15 + (ch->GetMaxHP() * iPercent) / 100;
		}

		iAmount += (iAmount * ch->GetPoint(POINT_HP_REGEN)) / 100;

		sys_log(1, "RECOVERY_EVENT: %s %d HP_REGEN %d HP +%lld", ch->GetName(), iPercent, ch->GetPoint(POINT_HP_REGEN), iAmount);

		ch->PointChange(POINT_HP, iAmount, false);
		return PASSES_PER_SEC(3);
	}
}

void CHARACTER::StartRecoveryEvent()
{
	if (m_pkRecoveryEvent)
		return;

	if (IsDead() || IsStun())
		return;

#ifdef __SCP1453_EXTENSIONS__
	if (GetRaceNum() == 6118)
		return;
#endif

	if (IsNPC() && GetHP() >= GetMaxHP())
		return;

	char_event_info* info = AllocEventInfo<char_event_info>();

	info->ch = this;

	int iSec = IsPC() ? 3 : (MAX(1, GetMobTable().bRegenCycle));
	m_pkRecoveryEvent = event_create(recovery_event, info, PASSES_PER_SEC(iSec));
}

void CHARACTER::Standup()
{
	struct packet_position pack_position;

	if (!IsPosition(POS_SITTING))
		return;

	SetPosition(POS_STANDING);

	sys_log(1, "STANDUP: %s", GetName());

	pack_position.header = HEADER_GC_CHARACTER_POSITION;
	pack_position.vid = GetVID();
	pack_position.position = POSITION_GENERAL;

	PacketAround(&pack_position, sizeof(pack_position));
}

void CHARACTER::Sitdown(int is_ground)
{
	struct packet_position pack_position;

	if (IsPosition(POS_SITTING))
		return;

	SetPosition(POS_SITTING);
	sys_log(1, "SITDOWN: %s", GetName());

	pack_position.header = HEADER_GC_CHARACTER_POSITION;
	pack_position.vid = GetVID();
	pack_position.position = POSITION_SITTING_GROUND;
	PacketAround(&pack_position, sizeof(pack_position));
}

void CHARACTER::SetRotation(float fRot)
{
	m_pointsInstant.fRot = fRot;
}

void CHARACTER::SetRotationToXY(long x, long y)
{
	SetRotation(GetDegreeFromPositionXY(GetX(), GetY(), x, y));
}

bool CHARACTER::CannotMoveByAffect() const
{
	return (IsAffectFlag(AFF_STUN));
}

bool CHARACTER::CanMove() const
{
	if (CannotMoveByAffect())
		return false;

	if (GetMyShop())
		return false;

	return true;
}

bool CHARACTER::Sync(long x, long y)
{
	if (!GetSectree())
		return false;

	if (IsPC() && IsDead()) // @duzenleme rubinumdan alýnmýþ bir fix eðer adam ölüyse sync etmesin. ( Ölüyken yürüme hilesi vs. kullananlar için.)
		return false;

	LPSECTREE new_tree = SECTREE_MANAGER::instance().Get(GetMapIndex(), x, y);

	if (!new_tree)
	{
		if (GetDesc())
		{
			sys_err("cannot find tree at %ld %ld (name: %s)", x, y, GetName()); // @ACILDI
			x = GetX();
			y = GetY();
			new_tree = GetSectree();
		}
		else
		{
			if (!IsPet()
#ifdef __GROWTH_PET_SYSTEM__
				&& !IsNewPet()
#endif
#ifdef __MOUNT_COSTUME_SYSTEM__
				&& !IsMount()
#endif
#ifdef __GROWTH_MOUNT_SYSTEM__
				&& !IsNewMount()
#endif
				)
			{
				Dead();
				sys_err("no tree: %s %ld %ld %d", GetName(), x, y, GetMapIndex());
			}
		}

		return false;
	}

	SetRotationToXY(x, y);
	SetXYZ(x, y, 0);

	if (GetDungeon())
	{
		int iLastEventAttr = m_iEventAttr;
		m_iEventAttr = new_tree->GetEventAttribute(x, y);

		if (m_iEventAttr != iLastEventAttr)
		{
			if (GetParty())
			{
				quest::CQuestManager::instance().AttrOut(GetParty()->GetLeaderPID(), this, iLastEventAttr);
				quest::CQuestManager::instance().AttrIn(GetParty()->GetLeaderPID(), this, m_iEventAttr);
			}
			else
			{
				quest::CQuestManager::instance().AttrOut(GetPlayerID(), this, iLastEventAttr);
				quest::CQuestManager::instance().AttrIn(GetPlayerID(), this, m_iEventAttr);
			}
		}
	}

	if (GetSectree() != new_tree)
	{
		if (!IsNPC())
		{
			SECTREEID id = new_tree->GetID();
			SECTREEID old_id = GetSectree()->GetID();

			const float fDist = DISTANCE_SQRT(id.coord.x - old_id.coord.x, id.coord.y - old_id.coord.y);
			sys_log(0, "SECTREE DIFFER: %s %ldx%ld was %ldx%ld dist %.1fm",
				GetName(),
				id.coord.x,
				id.coord.y,
				old_id.coord.x,
				old_id.coord.y,
				fDist);
		}

		new_tree->InsertEntity(this);
	}

	return true;
}

void CHARACTER::Stop()
{
	GotoState(m_stateIdle);

	m_posDest.x = m_posStart.x = GetX();
	m_posDest.y = m_posStart.y = GetY();
}

bool CHARACTER::Goto(long x, long y)
{
	if (GetX() == x && GetY() == y)
		return false;

	if (m_posDest.x == x && m_posDest.y == y)
	{
		if (!IsState(m_stateMove))
		{
			m_dwStateDuration = 4;
			GotoState(m_stateMove);
		}
		return false;
	}

	m_posDest.x = x;
	m_posDest.y = y;

	CalculateMoveDuration();

	m_dwStateDuration = 4;

	if (!IsState(m_stateMove))
	{
		if (GetVictim())
		{
			//MonsterChat(MONSTER_CHAT_CHASE);
			MonsterChat(MONSTER_CHAT_ATTACK);
		}
	}

	GotoState(m_stateMove);

	return true;
}

DWORD CHARACTER::GetMotionMode() const
{
	DWORD dwMode = MOTION_MODE_GENERAL;

	LPITEM pkItem = GetWear(WEAR_WEAPON);

	if (pkItem != NULL && pkItem->GetProto() != NULL)
	{
		switch (pkItem->GetProto()->bSubType)
		{
		case WEAPON_SWORD:
			dwMode = MOTION_MODE_ONEHAND_SWORD;
			break;

		case WEAPON_TWO_HANDED:
			dwMode = MOTION_MODE_TWOHAND_SWORD;
			break;

		case WEAPON_DAGGER:
			dwMode = MOTION_MODE_DUALHAND_SWORD;
			break;

		case WEAPON_BOW:
			dwMode = MOTION_MODE_BOW;
			break;

		case WEAPON_BELL:
			dwMode = MOTION_MODE_BELL;
			break;

		case WEAPON_FAN:
			dwMode = MOTION_MODE_FAN;
			break;

#ifdef __WOLFMAN_CHARACTER__
		case WEAPON_CLAW:
			dwMode = MOTION_MODE_CLAW;
			break;
#endif
		}
	}
	return dwMode;
}

float CHARACTER::GetMoveMotionSpeed() const
{
	DWORD dwMode = GetMotionMode();

	const CMotion* pkMotion = NULL;

	if (!GetMountVnum())
		pkMotion = CMotionManager::instance().GetMotion(GetRaceNum(), MAKE_MOTION_KEY(dwMode, (IsWalking() && IsPC()) ? MOTION_WALK : MOTION_RUN));
	else
	{
		pkMotion = CMotionManager::instance().GetMotion(GetMountVnum(), MAKE_MOTION_KEY(MOTION_MODE_GENERAL, (IsWalking() && IsPC()) ? MOTION_WALK : MOTION_RUN));

		if (!pkMotion)
			pkMotion = CMotionManager::instance().GetMotion(GetRaceNum(), MAKE_MOTION_KEY(MOTION_MODE_HORSE, (IsWalking() && IsPC()) ? MOTION_WALK : MOTION_RUN));
	}

	if (pkMotion)
		return -pkMotion->GetAccumVector().y / pkMotion->GetDuration();
	else
	{
		sys_err("cannot find motion (name %s race %d mode %d)", GetName(), GetRaceNum(), dwMode);
		return 449.9f;// @duzenleme eger datada olmayan mob var ise hareket hizi 449.9f olarak duzenlendi Rolof fileste boyle bir sorun vardi bu sekil cozmustum fakat saglikli degil.
	}
}

float CHARACTER::GetMoveSpeed() const
{
	return GetMoveMotionSpeed() * 10000 / CalculateDuration(GetLimitPoint(POINT_MOV_SPEED), 10000);
}

void CHARACTER::CalculateMoveDuration()
{
	m_posStart.x = GetX();
	m_posStart.y = GetY();

	float fDist = DISTANCE_SQRT(m_posStart.x - m_posDest.x, m_posStart.y - m_posDest.y);

	float motionSpeed = GetMoveMotionSpeed();

	m_dwMoveDuration = CalculateDuration(GetLimitPoint(POINT_MOV_SPEED),
		(int)((fDist / motionSpeed) * 1000.0f));

	if (IsNPC())
		sys_log(1, "%s: GOTO: distance %f, spd %u, duration %u, motion speed %f pos %d %d -> %d %d",
			GetName(), fDist, GetLimitPoint(POINT_MOV_SPEED), m_dwMoveDuration, motionSpeed,
			m_posStart.x, m_posStart.y, m_posDest.x, m_posDest.y);

	m_dwMoveStartTime = get_dword_time();
}

bool CHARACTER::Move(long x, long y)
{
	if (GetX() == x && GetY() == y)
		return true;

	OnMove();
	return Sync(x, y);
}

void CHARACTER::SendMovePacket(BYTE bFunc, BYTE bArg, DWORD x, DWORD y, DWORD dwDuration, DWORD dwTime, int iRot)
{
	TPacketGCMove pack;

	if (bFunc == FUNC_WAIT)
	{
		x = m_posDest.x;
		y = m_posDest.y;
		dwDuration = m_dwMoveDuration;
	}

	EncodeMovePacket(pack, GetVID(), bFunc, bArg, x, y, dwDuration, dwTime, iRot == -1 ? (int)GetRotation() / 5 : iRot);
	PacketView(&pack, sizeof(TPacketGCMove), this);
}

long long CHARACTER::GetRealPoint(BYTE type) const
{
	return m_points.points[type];
}

void CHARACTER::SetRealPoint(BYTE type, long long val)
{
	m_points.points[type] = val;
}

#ifdef __GOLD_LIMIT_REWORK__
long long CHARACTER::GetPoint(BYTE type) const
#else
int CHARACTER::GetPoint(BYTE type) const
#endif
{
	if (type >= POINT_MAX_NUM)
	{
		sys_err("Point type overflow (type %u)", type);
		return 0;
	}

#ifdef __GOLD_LIMIT_REWORK__
	long long val = m_pointsInstant.points[type];
	long long max_val = LLONG_MAX;
#else
	int val = m_pointsInstant.points[type];
	int max_val = INT_MAX;
#endif

	switch (type)
	{
	case POINT_STEAL_HP:
	case POINT_STEAL_SP:
		max_val = 250;
		break;
	}

	if (val > max_val)
#ifdef __GOLD_LIMIT_REWORK__
		sys_err("POINT_ERROR: %s type %d val %lld (max: %lld)", GetName(), type, val, max_val); // @duzenleme syserr yanlis girilmisti bu yuzden dogru sekilde yazdirdik.
#else
		sys_err("POINT_ERROR: %s type %d val %d (max: %d)", GetName(), type, val, max_val); // @duzenleme syserr yanlis girilmisti bu yuzden dogru sekilde yazdirdik.
#endif

	return (val);
}

int CHARACTER::GetLimitPoint(BYTE type) const
{
	if (type >= POINT_MAX_NUM)
	{
		sys_err("Point type overflow (type %u)", type);
		return 0;
	}

	int val = m_pointsInstant.points[type];
	int max_val = INT_MAX;
	int limit = INT_MAX;
	int min_limit = -INT_MAX;

	switch (type)
	{
	case POINT_ATT_SPEED:
		min_limit = 0;

		if (IsPC())
			limit = 350;
		else
			limit = 250;
		break;

	case POINT_MOV_SPEED:
		min_limit = 0;

		if (IsPC())
		{
			if (GetJob() == JOB_ASSASSIN)
				limit = 550;
			else
				limit = 350;
		}
		else
			limit = 350;
		break;

	case POINT_STEAL_HP:
	case POINT_STEAL_SP:
		limit = 250;
		max_val = 250;
		break;

	case POINT_MALL_ATTBONUS:
	case POINT_MALL_DEFBONUS:
		limit = 10000;// @duzenleme mall attbonus hesaplamasi degistirildi.
		max_val = 15000;// @duzenleme mall attbonus hesaplamasi degistirildi.
		break;
	}

	if (val > max_val)
		sys_err("POINT_ERROR: %s type %d val %d (max: %d)", GetName(), type, val, max_val);//@DIKKAT @duzenleme syserr yuzunden patliyordu fakat hala patlama ihtimali var verimiz long long geliyor (d) olarak yazdiriyor (lld) yapilmasi lazim.

	if (val > limit)
		val = limit;

	if (val < min_limit)
		val = min_limit;

	return (val);
}

#ifdef __GOLD_LIMIT_REWORK__
void CHARACTER::SetPoint(BYTE type, long long val)
#else
void CHARACTER::SetPoint(BYTE type, int val)
#endif
{
	if (type >= POINT_MAX_NUM)
	{
		sys_err("Point type overflow (type %u)", type);
		return;
	}

	m_pointsInstant.points[type] = val;

	if (type == POINT_MOV_SPEED && get_dword_time() < m_dwMoveStartTime + m_dwMoveDuration)
		CalculateMoveDuration();
}

void CHARACTER::CheckMaximumPoints()
{
	if (GetMaxHP() < GetHP())
		PointChange(POINT_HP, GetMaxHP() - GetHP());

	if (GetMaxSP() < GetSP())
		PointChange(POINT_SP, GetMaxSP() - GetSP());
}

#ifdef __GOLD_LIMIT_REWORK__
void CHARACTER::PointChange(BYTE type, long long amount, bool bAmount, bool bBroadcast)
{
	long long val = 0;
#else
void CHARACTER::PointChange(BYTE type, int amount, bool bAmount, bool bBroadcast)
{
	int val = 0;
#endif

	switch (type)
	{
	case POINT_NONE:
#ifdef __BATTLE_PASS_SYSTEM__
	case POINT_BATTLE_PASS_ID:
#endif
		return;

	case POINT_LEVEL:
		if ((GetLevel() + amount) > gPlayerMaxLevel)
			return;

		SetLevel(GetLevel() + amount);
		val = GetLevel();

		sys_log(0, "LEVELUP: %s %d NEXT EXP %d", GetName(), GetLevel(), GetNextExp());

		PointChange(POINT_NEXT_EXP, GetNextExp(), false);

		if (amount)
		{
#ifdef __LEVEL_SET_BONUS__
			CheckLevelSetBonus();
#endif // __LEVEL_SET_BONUS__

			quest::CQuestManager::instance().LevelUp(GetPlayerID());

			if (GetGuild())
			{
				GetGuild()->LevelChange(GetPlayerID(), GetLevel());
			}

			if (GetParty())
			{
				GetParty()->RequestSetMemberLevel(GetPlayerID(), GetLevel());
			}
		}
		break;

	case POINT_NEXT_EXP:
		val = GetNextExp();
		bAmount = false;
		break;

	case POINT_EXP:
	{
		DWORD exp = GetExp();
		DWORD next_exp = GetNextExp();

		if ((amount < 0) && (exp <= std::abs((int)amount))) // @bilinmeyenduzenleme muhtemelen inceptiondan alindi.
		{
			sys_log(1, "%s AMOUNT < 0 %d, CUR EXP: %d", GetName(), -amount, exp);
			amount = -exp;

			SetExp(exp + amount);
			val = GetExp();
		}
		else
		{
			if (gPlayerMaxLevel <= GetLevel())
				return;

			if (test_server)
				ChatPacket(CHAT_TYPE_INFO, "You have gained %d exp.", amount);

			DWORD iExpBalance = 0;

			if (exp + amount >= next_exp)
			{
				iExpBalance = (exp + amount) - next_exp;
				amount = next_exp - exp;

				SetExp(0);
				exp = next_exp;
			}
			else
			{
				SetExp(exp + amount);
				exp = GetExp();
			}

			DWORD q = DWORD(next_exp / 4.0f);
			int iLevStep = GetRealPoint(POINT_LEVEL_STEP);

			if (iLevStep >= 4)
			{
				sys_err("%s LEVEL_STEP bigger than 4! (%d)", GetName(), iLevStep);
				iLevStep = 4;
			}

			if (exp >= next_exp && iLevStep < 4)
			{
				for (int i = 0; i < 4 - iLevStep; ++i)
					PointChange(POINT_LEVEL_STEP, 1, false, true);
			}
			else if (exp >= q * 3 && iLevStep < 3)
			{
				for (int i = 0; i < 3 - iLevStep; ++i)
					PointChange(POINT_LEVEL_STEP, 1, false, true);
			}
			else if (exp >= q * 2 && iLevStep < 2)
			{
				for (int i = 0; i < 2 - iLevStep; ++i)
					PointChange(POINT_LEVEL_STEP, 1, false, true);
			}
			else if (exp >= q && iLevStep < 1)
				PointChange(POINT_LEVEL_STEP, 1);

			if (iExpBalance)
			{
				PointChange(POINT_EXP, iExpBalance);
			}

			val = GetExp();
		}
	}
	break;

	case POINT_LEVEL_STEP:
		if (amount > 0)
		{
			val = GetPoint(POINT_LEVEL_STEP) + amount;

			switch (val)
			{
			case 1:
			case 2:
			case 3:
				if ((GetLevel() <= g_iStatusPointGetLevelLimit) &&
					(GetLevel() <= gPlayerMaxLevel)) // @duzenleme statu puanlari karakter level sinirina ulasana dek gelmeye devam eder fakat level siniri yukselirse sikinti yaratabilir.
					PointChange(POINT_STAT, 1);
				break;

			case 4:
			{
				HPTYPE iHP = number(JobInitialPoints[GetJob()].hp_per_lv_begin, JobInitialPoints[GetJob()].hp_per_lv_end);
				int iSP = number(JobInitialPoints[GetJob()].sp_per_lv_begin, JobInitialPoints[GetJob()].sp_per_lv_end);

				m_points.iRandomHP += iHP;
				m_points.iRandomSP += iSP;

				if (GetSkillGroup())
				{
					if (GetLevel() >= 5)
						PointChange(POINT_SKILL, 1);

					if (GetLevel() >= 9)
						PointChange(POINT_SUB_SKILL, 1);
				}

				PointChange(POINT_MAX_HP, iHP);
				PointChange(POINT_MAX_SP, iSP);
				PointChange(POINT_LEVEL, 1, false, true);

				val = 0;
			}
			break;
			}
			PointChange(POINT_HP, GetMaxHP() - GetHP());
			PointChange(POINT_SP, GetMaxSP() - GetSP());
			PointChange(POINT_STAMINA, GetMaxStamina() - GetStamina());

			SetPoint(POINT_LEVEL_STEP, val);
			SetRealPoint(POINT_LEVEL_STEP, val);

			Save();
		}
		else
			val = GetPoint(POINT_LEVEL_STEP);

		break;

	case POINT_HP:
	{
		if (IsDead() || IsStun())
			return;

		HPTYPE prev_hp = GetHP();

		amount = MIN(GetMaxHP() - GetHP(), amount);
		SetHP(GetHP() + amount);
		val = GetHP();

		BroadcastTargetPacket();
		if (GetParty() && IsPC() && val != prev_hp)
			GetParty()->SendPartyInfoOneToAll(this);
	}
	break;

	case POINT_SP:
	{
		if (IsDead() || IsStun())
			return;

		amount = MIN(GetMaxSP() - GetSP(), amount);
		SetSP(GetSP() + amount);
		val = GetSP();
	}
	break;

	case POINT_STAMINA:
	{
		if (IsDead() || IsStun())
			return;

		int prev_val = GetStamina();
		amount = MIN(GetMaxStamina() - GetStamina(), amount);
		SetStamina(GetStamina() + amount);
		val = GetStamina();

		if (val == 0)
		{
			// Stamina
			SetNowWalking(true);
		}
		else if (prev_val == 0)
		{
			ResetWalking();
		}

		if (amount < 0 && val != 0)
			return;
	}
	break;

	case POINT_MAX_HP:
	{
		SetPoint(type, GetPoint(type) + amount);
		HPTYPE curMaxHP = GetMaxHP();
		HPTYPE hp = GetRealPoint(POINT_MAX_HP);
		HPTYPE add_hp = MIN(3500, hp * GetPoint(POINT_MAX_HP_PCT) / 100);
		add_hp += GetPoint(POINT_MAX_HP);
		add_hp += GetPoint(POINT_PARTY_TANKER_BONUS);
		SetMaxHP(hp + add_hp);
		long double fRatio = (long double)GetMaxHP() / (long double)curMaxHP;
		PointChange(POINT_HP, GetHP() * fRatio - GetHP());
		val = GetMaxHP();
	}
	break;

	case POINT_MAX_SP:
	{
		SetPoint(type, GetPoint(type) + amount);
		int curMaxSP = GetMaxSP();
		int sp = GetRealPoint(POINT_MAX_SP);
		int add_sp = MIN(800, sp * GetPoint(POINT_MAX_SP_PCT) / 100);
		add_sp += GetPoint(POINT_MAX_SP);
		add_sp += GetPoint(POINT_PARTY_SKILL_MASTER_BONUS);
		SetMaxSP(sp + add_sp);
		float fRatio = (float)GetMaxSP() / (float)curMaxSP;
		PointChange(POINT_SP, GetSP() * fRatio - GetSP());
		val = GetMaxSP();
	}
	break;

	case POINT_MAX_HP_PCT:
		SetPoint(type, GetPoint(type) + amount);
		val = GetPoint(type);
		PointChange(POINT_MAX_HP, 0);
		break;

	case POINT_MAX_SP_PCT:
		SetPoint(type, GetPoint(type) + amount);
		val = GetPoint(type);
		PointChange(POINT_MAX_SP, 0);
		break;

	case POINT_MAX_STAMINA:
		SetMaxStamina(GetMaxStamina() + amount);
		val = GetMaxStamina();
		break;

	case POINT_GOLD:
	{
#ifdef __GOLD_LIMIT_REWORK__
		const long long nTotalMoney = static_cast<long long>(GetGold()) + static_cast<long long>(amount);

		if (GOLD_MAX <= nTotalMoney)
		{
			//sys_err("[OVERFLOW_GOLD] OriGold %lld AddedGold %lld id %u Name %s ", GetGold(), amount, GetPlayerID(), GetName());
			return;
		}
#else
		const int64_t nTotalMoney = static_cast<int64_t>(GetGold()) + static_cast<int64_t>(amount);

		if (GOLD_MAX <= nTotalMoney)
		{
			//sys_err("[OVERFLOW_GOLD] OriGold %d AddedGold %d id %u Name %s ", GetGold(), amount, GetPlayerID(), GetName());
			return;
		}
#endif

		if (nTotalMoney < 0) // @duzenleme sifirdan kucukse diye kontrol koyduk. fakat neden koydugumuz ile ilgili bir fikrim yok.
			return;

		SetGold(GetGold() + amount);
		val = GetGold();
	}
	break;

#ifdef __CHEQUE_SYSTEM__
	case POINT_CHEQUE:
	{
		const int64_t nTotalCheque = static_cast<int64_t>(GetCheque()) + static_cast<int64_t>(amount);

		if (CHEQUE_MAX <= nTotalCheque)
		{
			sys_err("[OVERFLOW_CHEQUE] OriCheque %d AddedCheque %lld id %u Name %s", GetCheque(), amount, GetPlayerID(), GetName());
			return;
		}

		if (nTotalCheque < 0) // @duzenleme sifirdan kucukse diye kontrol koyduk. fakat neden koydugumuz ile ilgili bir fikrim yok.
			return;

		SetCheque(GetCheque() + amount);
		val = GetCheque();
	}
	break;
#endif

	case POINT_SKILL:
	case POINT_STAT:
	case POINT_SUB_SKILL:
	case POINT_STAT_RESET_COUNT:
	case POINT_HORSE_SKILL:
		SetPoint(type, GetPoint(type) + amount);
		val = GetPoint(type);

		SetRealPoint(type, val);
		break;

	case POINT_DEF_GRADE:
		SetPoint(type, GetPoint(type) + amount);
		val = GetPoint(type);

		PointChange(POINT_CLIENT_DEF_GRADE, amount);
		break;

	case POINT_CLIENT_DEF_GRADE:
		SetPoint(type, GetPoint(type) + amount);
		val = GetPoint(type);
		break;

	case POINT_MOV_SPEED:
	{
		SetPoint(type, GetPoint(type) + amount);
		val = GetPoint(type);
	}
	break;

	case POINT_ST:
	case POINT_HT:
	case POINT_DX:
	case POINT_IQ:
	case POINT_HP_REGEN:
	case POINT_SP_REGEN:
	case POINT_ATT_SPEED:
	case POINT_ATT_GRADE:
	case POINT_CASTING_SPEED:
	case POINT_MAGIC_ATT_GRADE:
	case POINT_MAGIC_DEF_GRADE:
	case POINT_BOW_DISTANCE:
	case POINT_HP_RECOVERY:
	case POINT_SP_RECOVERY:

	case POINT_ATTBONUS_HUMAN:	// 42
	case POINT_ATTBONUS_ANIMAL:	// 43
	case POINT_ATTBONUS_ORC:	// 44
	case POINT_ATTBONUS_MILGYO:	// 45
	case POINT_ATTBONUS_UNDEAD:	// 46
	case POINT_ATTBONUS_DEVIL:	// 47

	case POINT_ATTBONUS_MONSTER:
	case POINT_ATTBONUS_SURA:
	case POINT_ATTBONUS_ASSASSIN:
	case POINT_ATTBONUS_WARRIOR:
	case POINT_ATTBONUS_SHAMAN:
#ifdef __WOLFMAN_CHARACTER__
	case POINT_ATTBONUS_WOLFMAN:
#endif

	case POINT_POISON_PCT:
#ifdef __WOLFMAN_CHARACTER__
	case POINT_BLEEDING_PCT:
#endif
	case POINT_STUN_PCT:
	case POINT_SLOW_PCT:

	case POINT_BLOCK:
	case POINT_DODGE:

	case POINT_CRITICAL_PCT:
	case POINT_RESIST_CRITICAL:
	case POINT_PENETRATE_PCT:
	case POINT_RESIST_PENETRATE:
	case POINT_CURSE_PCT:

	case POINT_STEAL_HP:		// 48
	case POINT_STEAL_SP:		// 49

	case POINT_MANA_BURN_PCT:	// 50
	case POINT_DAMAGE_SP_RECOVER:	// 51
	case POINT_RESIST_NORMAL_DAMAGE:
	case POINT_RESIST_SWORD:
	case POINT_RESIST_TWOHAND:
	case POINT_RESIST_DAGGER:
	case POINT_RESIST_BELL:
	case POINT_RESIST_FAN:
	case POINT_RESIST_BOW:
#ifdef __WOLFMAN_CHARACTER__
	case POINT_RESIST_CLAW:
#endif
	case POINT_RESIST_FIRE:
	case POINT_RESIST_ELEC:
	case POINT_RESIST_MAGIC:
#ifdef __ACCE_SYSTEM__
	case POINT_ACCEDRAIN_RATE:
#endif
#ifdef __MAGIC_REDUCTION_SYSTEM__
	case POINT_RESIST_MAGIC_REDUCTION:
#endif
#ifdef __PENDANT_SYSTEM__
	case POINT_ENCHANT_FIRE:
	case POINT_ENCHANT_ICE:
	case POINT_ENCHANT_EARTH:
	case POINT_ENCHANT_DARK:
	case POINT_ENCHANT_WIND:
	case POINT_ENCHANT_ELECT:

	case POINT_RESIST_HUMAN:

	case POINT_ATTBONUS_SWORD:
	case POINT_ATTBONUS_TWOHAND:
	case POINT_ATTBONUS_DAGGER:
	case POINT_ATTBONUS_BELL:
	case POINT_ATTBONUS_FAN:
	case POINT_ATTBONUS_BOW:
#ifdef __WOLFMAN_CHARACTER__
	case POINT_ATTBONUS_CLAW:
#endif
	case POINT_ATTBONUS_CZ:
	case POINT_ATTBONUS_DESERT:
	case POINT_ATTBONUS_INSECT:
#endif
#ifdef __ATTRIBUTES_TYPES__
	case POINT_ATTBONUS_STONE:
	case POINT_ATTBONUS_BOSS:
	case POINT_ATTBONUS_ELEMENTS:
	case POINT_ENCHANT_ELEMENTS:
	case POINT_ATTBONUS_CHARACTERS:
	case POINT_ENCHANT_CHARACTERS:
#endif
#ifdef __CHEST_DROP_POINT__
	case POINT_CHEST_BONUS:
#endif
	case POINT_RESIST_WIND:
	case POINT_RESIST_ICE:
	case POINT_RESIST_EARTH:
	case POINT_RESIST_DARK:
	case POINT_REFLECT_MELEE:	// 67
	case POINT_REFLECT_CURSE:	// 68
	case POINT_POISON_REDUCE:	// 69
#ifdef __WOLFMAN_CHARACTER__
	case POINT_BLEEDING_REDUCE:
#endif
	case POINT_KILL_SP_RECOVER:	// 70
	case POINT_KILL_HP_RECOVERY:	// 75
	case POINT_HIT_HP_RECOVERY:
	case POINT_HIT_SP_RECOVERY:
	case POINT_MANASHIELD:
	case POINT_ATT_BONUS:
	case POINT_DEF_BONUS:
	case POINT_SKILL_DAMAGE_BONUS:
	case POINT_NORMAL_HIT_DAMAGE_BONUS:

		// DEPEND_BONUS_ATTRIBUTES
	case POINT_SKILL_DEFEND_BONUS:
	case POINT_NORMAL_HIT_DEFEND_BONUS:
		SetPoint(type, GetPoint(type) + amount);
		val = GetPoint(type);
		break;
		// END_OF_DEPEND_BONUS_ATTRIBUTES

	case POINT_PARTY_ATTACKER_BONUS:
	case POINT_PARTY_TANKER_BONUS:
	case POINT_PARTY_BUFFER_BONUS:
	case POINT_PARTY_SKILL_MASTER_BONUS:
	case POINT_PARTY_HASTE_BONUS:
	case POINT_PARTY_DEFENDER_BONUS:
#ifdef __PARTY_ROLE_REWORK__
	case POINT_PARTY_ATTACKER_MONSTER_BONUS:
	case POINT_PARTY_ATTACKER_STONE_BONUS:
	case POINT_PARTY_ATTACKER_BOSS_BONUS:
#endif

	case POINT_RESIST_WARRIOR:
	case POINT_RESIST_ASSASSIN:
	case POINT_RESIST_SURA:
	case POINT_RESIST_SHAMAN:
#ifdef __WOLFMAN_CHARACTER__
	case POINT_RESIST_WOLFMAN:
#endif

		SetPoint(type, GetPoint(type) + amount);
		val = GetPoint(type);
		break;

	case POINT_MALL_ATTBONUS:
	case POINT_MALL_DEFBONUS:
	case POINT_MALL_EXPBONUS:
	case POINT_MALL_ITEMBONUS:
	case POINT_MALL_GOLDBONUS:
	case POINT_MELEE_MAGIC_ATT_BONUS_PER:
		if (GetPoint(type) + amount > 15000)// accelerated
		{
			sys_log(0, "MALL_BONUS exceeded over 15000!! point type: %d name: %s amount %d", type, GetName(), amount);
			amount = 5000 - GetPoint(type);
		}

		SetPoint(type, GetPoint(type) + amount);
		val = GetPoint(type);
		break;

	case POINT_RAMADAN_CANDY_BONUS_EXP:
		SetPoint(type, amount);
		val = GetPoint(type);
		break;

	case POINT_EXP_DOUBLE_BONUS:	// 71
	case POINT_GOLD_DOUBLE_BONUS:	// 72
	case POINT_ITEM_DROP_BONUS:	// 73
	case POINT_POTION_BONUS:	// 74
		if (GetPoint(type) + amount > 500)// accelerated
		{
			sys_log(0, "BONUS exceeded over 500!! point type: %d name: %s amount %d", type, GetName(), amount);
			amount = 500 - GetPoint(type);
		}

		SetPoint(type, GetPoint(type) + amount);
		val = GetPoint(type);
		break;

	case POINT_IMMUNE_STUN:		// 76
		SetPoint(type, GetPoint(type) + amount);
		val = GetPoint(type);
		if (val)
		{
			// ChatPacket(CHAT_TYPE_INFO, "IMMUNE_STUN SET_BIT type(%u) amount(%d)", type, amount);
			SET_BIT(m_pointsInstant.dwImmuneFlag, IMMUNE_STUN);
		}
		else
		{
			// ChatPacket(CHAT_TYPE_INFO, "IMMUNE_STUN REMOVE_BIT type(%u) amount(%d)", type, amount);
			REMOVE_BIT(m_pointsInstant.dwImmuneFlag, IMMUNE_STUN);
		}
		break;

	case POINT_IMMUNE_SLOW:		// 77
		SetPoint(type, GetPoint(type) + amount);
		val = GetPoint(type);
		if (val)
		{
			SET_BIT(m_pointsInstant.dwImmuneFlag, IMMUNE_SLOW);
		}
		else
		{
			REMOVE_BIT(m_pointsInstant.dwImmuneFlag, IMMUNE_SLOW);
		}
		break;

	case POINT_IMMUNE_FALL:	// 78
		SetPoint(type, GetPoint(type) + amount);
		val = GetPoint(type);
		if (val)
		{
			SET_BIT(m_pointsInstant.dwImmuneFlag, IMMUNE_FALL);
		}
		else
		{
			REMOVE_BIT(m_pointsInstant.dwImmuneFlag, IMMUNE_FALL);
		}
		break;

	case POINT_ATT_GRADE_BONUS:
		SetPoint(type, GetPoint(type) + amount);
		PointChange(POINT_ATT_GRADE, amount);
		val = GetPoint(type);
		break;

	case POINT_DEF_GRADE_BONUS:
		SetPoint(type, GetPoint(type) + amount);
		PointChange(POINT_DEF_GRADE, amount);
		val = GetPoint(type);
		break;

	case POINT_MAGIC_ATT_GRADE_BONUS:
		SetPoint(type, GetPoint(type) + amount);
		PointChange(POINT_MAGIC_ATT_GRADE, amount);
		val = GetPoint(type);
		break;

	case POINT_MAGIC_DEF_GRADE_BONUS:
		SetPoint(type, GetPoint(type) + amount);
		PointChange(POINT_MAGIC_DEF_GRADE, amount);
		val = GetPoint(type);
		break;

	case POINT_EMPIRE_POINT:
		//sys_err("CHARACTER::PointChange: %s: point cannot be changed. use SetPoint instead (type: %d)", GetName(), type);
		val = GetRealPoint(type);
		break;

	case POINT_POLYMORPH:
		SetPoint(type, GetPoint(type) + amount);
		val = GetPoint(type);
		break;

	case POINT_MOUNT:
		SetPoint(type, GetPoint(type) + amount);
		val = GetPoint(type);
		MountVnum(val);
		break;

	case POINT_ENERGY:
	case POINT_COSTUME_ATTR_BONUS:
	{
		break;
		int old_val = GetPoint(type);
		SetPoint(type, old_val + amount);
		val = GetPoint(type);
		BuffOnAttr_ValueChange(type, old_val, val);
	}
	break;

	default:
		sys_err("CHARACTER::PointChange: %s: unknown point change type %d", GetName(), type);
		return;
	}

	switch (type)
	{
	case POINT_LEVEL:
	case POINT_ST:
	case POINT_DX:
	case POINT_IQ:
	case POINT_HT:
		ComputeBattlePoints();
		break;
	case POINT_MAX_HP:
	case POINT_MAX_SP:
	case POINT_MAX_STAMINA:
		break;
	}

	if (type == POINT_HP && amount == 0)
		return;

	if (GetDesc())
	{
		struct packet_point_change pack;

		pack.header = HEADER_GC_CHARACTER_POINT_CHANGE;
		pack.dwVID = m_vid;
		pack.type = type;
		pack.value = val;

		if (bAmount)
			pack.amount = amount;
		else
			pack.amount = 0;

		if (!bBroadcast)
			GetDesc()->Packet(&pack, sizeof(struct packet_point_change));
		else
			PacketAround(&pack, sizeof(pack));
	}
}

#if defined(__GROWTH_PET_SYSTEM__) || defined(__GROWTH_MOUNT_SYSTEM__)
void CHARACTER::SendPetLevelUpEffect(int vid, int type, int value, int amount)
{
	struct packet_point_change pack;

	pack.header = HEADER_GC_CHARACTER_POINT_CHANGE;
	pack.dwVID = vid;
	pack.type = type;
	pack.value = value;
	pack.amount = amount;
	PacketAround(&pack, sizeof(pack));
}
#endif

void CHARACTER::ApplyPoint(BYTE bApplyType, int iVal)
{
	switch (bApplyType)
	{
	case APPLY_NONE:
		break; // @bilinmeyenduzenleme APPLY_NONE gelmesinin mumkun oldugunu dusunmuyorum.

	case APPLY_CON:
		PointChange(POINT_HT, iVal);
		PointChange(POINT_MAX_HP, (iVal * JobInitialPoints[GetJob()].hp_per_ht));
		PointChange(POINT_MAX_STAMINA, (iVal * JobInitialPoints[GetJob()].stamina_per_con));
		break;

	case APPLY_INT:
		PointChange(POINT_IQ, iVal);
		PointChange(POINT_MAX_SP, (iVal * JobInitialPoints[GetJob()].sp_per_iq));
		break;

	case APPLY_SKILL:
		// SKILL_DAMAGE_BONUS
	{
		// 00000000 00000000 00000000 00000000

		// vnum     ^ add       change
		BYTE bSkillVnum = (BYTE)(((DWORD)iVal) >> 24);
		int iAdd = iVal & 0x00800000;
		int iChange = iVal & 0x007fffff;

		sys_log(1, "APPLY_SKILL skill %d add? %d change %d", bSkillVnum, iAdd ? 1 : 0, iChange);

		if (0 == iAdd)
			iChange = -iChange;

		boost::unordered_map<BYTE, int>::iterator iter = m_SkillDamageBonus.find(bSkillVnum);

		if (iter == m_SkillDamageBonus.end())
			m_SkillDamageBonus.insert(std::make_pair(bSkillVnum, iChange));
		else
			iter->second += iChange;
	}
	// END_OF_SKILL_DAMAGE_BONUS
	break;

	//Think - 08/04/14: The ratio is a nice idea, but it doesn't work due to how the game updates things
	//It ends up causing reductions / increases of HP when the player gets the item buffs refreshed.
	//Removed (ratio).

	case APPLY_MAX_HP:
	case APPLY_MAX_HP_PCT:
	{
		if (GetMaxHP() == 0) break;
		PointChange(aApplyInfo[bApplyType].bPointType, iVal);
	}
	break;

	case APPLY_MAX_SP:
	case APPLY_MAX_SP_PCT:
	{
		if (GetMaxSP() == 0) break;
		PointChange(aApplyInfo[bApplyType].bPointType, iVal);
	}
	break;

	case APPLY_STR:
	case APPLY_DEX:
	case APPLY_ATT_SPEED:
	case APPLY_MOV_SPEED:
	case APPLY_CAST_SPEED:
	case APPLY_HP_REGEN:
	case APPLY_SP_REGEN:
	case APPLY_POISON_PCT:
#ifdef __WOLFMAN_CHARACTER__
	case APPLY_BLEEDING_PCT:
#endif
	case APPLY_STUN_PCT:
	case APPLY_SLOW_PCT:
	case APPLY_CRITICAL_PCT:
	case APPLY_PENETRATE_PCT:
	case APPLY_ATTBONUS_HUMAN:
	case APPLY_ATTBONUS_ANIMAL:
	case APPLY_ATTBONUS_ORC:
	case APPLY_ATTBONUS_MILGYO:
	case APPLY_ATTBONUS_UNDEAD:
	case APPLY_ATTBONUS_DEVIL:
	case APPLY_ATTBONUS_WARRIOR:	// 59
	case APPLY_ATTBONUS_ASSASSIN:	// 60
	case APPLY_ATTBONUS_SURA:	// 61
	case APPLY_ATTBONUS_SHAMAN:	// 62
#ifdef __WOLFMAN_CHARACTER__
	case APPLY_ATTBONUS_WOLFMAN:
#endif
	case APPLY_ATTBONUS_MONSTER:	// 63
	case APPLY_STEAL_HP:
	case APPLY_STEAL_SP:
	case APPLY_MANA_BURN_PCT:
	case APPLY_DAMAGE_SP_RECOVER:
	case APPLY_BLOCK:
	case APPLY_DODGE:
	case APPLY_RESIST_SWORD:
	case APPLY_RESIST_TWOHAND:
	case APPLY_RESIST_DAGGER:
	case APPLY_RESIST_BELL:
	case APPLY_RESIST_FAN:
	case APPLY_RESIST_BOW:
#ifdef __WOLFMAN_CHARACTER__
	case APPLY_RESIST_CLAW:
#endif
	case APPLY_RESIST_FIRE:
	case APPLY_RESIST_ELEC:
	case APPLY_RESIST_MAGIC:
	case APPLY_RESIST_WIND:
	case APPLY_RESIST_ICE:
	case APPLY_RESIST_EARTH:
	case APPLY_RESIST_DARK:
	case APPLY_REFLECT_MELEE:
	case APPLY_REFLECT_CURSE:
	case APPLY_ANTI_CRITICAL_PCT:
	case APPLY_ANTI_PENETRATE_PCT:
	case APPLY_POISON_REDUCE:
#ifdef __WOLFMAN_CHARACTER__
	case APPLY_BLEEDING_REDUCE:
#endif
	case APPLY_KILL_SP_RECOVER:
	case APPLY_EXP_DOUBLE_BONUS:
	case APPLY_GOLD_DOUBLE_BONUS:
	case APPLY_ITEM_DROP_BONUS:
	case APPLY_POTION_BONUS:
	case APPLY_KILL_HP_RECOVER:
	case APPLY_IMMUNE_STUN:
	case APPLY_IMMUNE_SLOW:
	case APPLY_IMMUNE_FALL:
	case APPLY_BOW_DISTANCE:
	case APPLY_ATT_GRADE_BONUS:
	case APPLY_DEF_GRADE_BONUS:
	case APPLY_MAGIC_ATT_GRADE:
	case APPLY_MAGIC_DEF_GRADE:
	case APPLY_CURSE_PCT:
	case APPLY_MAX_STAMINA:
	case APPLY_MALL_ATTBONUS:
	case APPLY_MALL_DEFBONUS:
	case APPLY_MALL_EXPBONUS:
	case APPLY_MALL_ITEMBONUS:
	case APPLY_MALL_GOLDBONUS:
	case APPLY_SKILL_DAMAGE_BONUS:
	case APPLY_NORMAL_HIT_DAMAGE_BONUS:

		// DEPEND_BONUS_ATTRIBUTES
	case APPLY_SKILL_DEFEND_BONUS:
	case APPLY_NORMAL_HIT_DEFEND_BONUS:
		// END_OF_DEPEND_BONUS_ATTRIBUTES

	case APPLY_RESIST_WARRIOR:
	case APPLY_RESIST_ASSASSIN:
	case APPLY_RESIST_SURA:
	case APPLY_RESIST_SHAMAN:
#ifdef __WOLFMAN_CHARACTER__
	case APPLY_RESIST_WOLFMAN:
#endif
	case APPLY_ENERGY:					// 82
	case APPLY_DEF_GRADE:				// 83
	case APPLY_COSTUME_ATTR_BONUS:		// 84
	case APPLY_MAGIC_ATTBONUS_PER:		// 85
	case APPLY_MELEE_MAGIC_ATTBONUS_PER:			// 86
#ifdef __ACCE_SYSTEM__
	case APPLY_ACCEDRAIN_RATE:			//97
#endif
#ifdef __MAGIC_REDUCTION_SYSTEM__
	case APPLY_RESIST_MAGIC_REDUCTION:	//98
#endif
#ifdef __PENDANT_SYSTEM__
	case APPLY_ENCHANT_FIRE:
	case APPLY_ENCHANT_ICE:
	case APPLY_ENCHANT_EARTH:
	case APPLY_ENCHANT_DARK:
	case APPLY_ENCHANT_WIND:
	case APPLY_ENCHANT_ELECT:
#ifdef __MOUNT_COSTUME_SYSTEM__
	case APPLY_MOUNT:
#endif
	case APPLY_RESIST_HUMAN:

	case APPLY_ATTBONUS_SWORD:
	case APPLY_ATTBONUS_TWOHAND:
	case APPLY_ATTBONUS_DAGGER:
	case APPLY_ATTBONUS_BELL:
	case APPLY_ATTBONUS_FAN:
	case APPLY_ATTBONUS_BOW:
#ifdef __WOLFMAN_CHARACTER__
	case APPLY_ATTBONUS_CLAW:
#endif
	case APPLY_ATTBONUS_CZ:
	case APPLY_ATTBONUS_DESERT:
	case APPLY_ATTBONUS_INSECT:
#endif
#ifdef __ATTRIBUTES_TYPES__
	case APPLY_ATTBONUS_STONE:
	case APPLY_ATTBONUS_BOSS:
	case APPLY_ATTBONUS_ELEMENTS:
	case APPLY_ENCHANT_ELEMENTS:
	case APPLY_ATTBONUS_CHARACTERS:
	case APPLY_ENCHANT_CHARACTERS:
#endif
#ifdef __CHEST_DROP_POINT__
	case APPLY_CHEST_BONUS:
#endif
		PointChange(aApplyInfo[bApplyType].bPointType, iVal);
		break;

	default:
		sys_err("Unknown apply type %d name %s", bApplyType, GetName());
		break;
	}
}

void CHARACTER::MotionPacketEncode(BYTE motion, LPCHARACTER victim, struct packet_motion* packet)
{
	packet->header = HEADER_GC_MOTION;
	packet->vid = m_vid;
	packet->motion = motion;

	if (victim)
		packet->victim_vid = victim->GetVID();
	else
		packet->victim_vid = 0;
}

void CHARACTER::Motion(BYTE motion, LPCHARACTER victim)
{
	struct packet_motion pack_motion;
	MotionPacketEncode(motion, victim, &pack_motion);
	PacketAround(&pack_motion, sizeof(struct packet_motion));
}

EVENTFUNC(save_event)
{
	char_event_info* info = dynamic_cast<char_event_info*>(event->info);
	if (info == NULL)
	{
		sys_err("save_event> <Factor> Null pointer");
		return 0;
	}

	LPCHARACTER ch = info->ch;

	if (ch == NULL) { // <Factor>
		return 0;
	}
	sys_log(1, "SAVE_EVENT: %s", ch->GetName());
	ch->Save();
	ch->FlushDelayedSaveItem();
	return (save_event_second_cycle);
}

void CHARACTER::StartSaveEvent()
{
	if (m_pkSaveEvent)
		return;

	char_event_info* info = AllocEventInfo<char_event_info>();

	info->ch = this;
	m_pkSaveEvent = event_create(save_event, info, save_event_second_cycle);
}

void CHARACTER::ChatPacket(BYTE type, const char* format, ...)
{
	LPDESC d = GetDesc();

	if (!d || !format)
		return;

	char chatbuf[CHAT_MAX_LEN + 1];
	va_list args;

	va_start(args, format);
	int len = vsnprintf(chatbuf, sizeof(chatbuf), format, args);
	va_end(args);

	struct packet_chat pack_chat;

	pack_chat.header = HEADER_GC_CHAT;
	pack_chat.size = sizeof(struct packet_chat) + len;
	pack_chat.type = type;
	pack_chat.id = 0;
	pack_chat.bEmpire = d->GetEmpire();

	TEMP_BUFFER buf;
	buf.write(&pack_chat, sizeof(struct packet_chat));
	buf.write(chatbuf, len);

	d->Packet(buf.read_peek(), buf.size());

	if (type == CHAT_TYPE_COMMAND && test_server)
		sys_log(0, "SEND_COMMAND %s %s", GetName(), chatbuf);
}

// MINING
void CHARACTER::mining_take()
{
	m_pkMiningEvent = NULL;
}

void CHARACTER::mining_cancel()
{
	if (m_pkMiningEvent)
	{
		sys_log(0, "XXX MINING CANCEL");
		event_cancel(&m_pkMiningEvent);
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Ã¤±¤À» Áß´ÜÇÏ¿´½À´Ï´Ù."));
	}
}

void CHARACTER::mining(LPCHARACTER chLoad)
{
	if (m_pkMiningEvent)
	{
		mining_cancel();
		return;
	}

	if (!chLoad)
		return;

	if (mining::GetRawOreFromLoad(chLoad->GetRaceNum()) == 0)
		return;

	LPITEM pick = GetWear(WEAR_WEAPON);

	if (!pick || pick->GetType() != ITEM_PICK)
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("°î±ªÀÌ¸¦ ÀåÂøÇÏ¼¼¿ä."));
		return;
	}

	int count = number(5, 15);

	TPacketGCDigMotion p;
	p.header = HEADER_GC_DIG_MOTION;
	p.vid = GetVID();
	p.target_vid = chLoad->GetVID();
	p.count = count;

	PacketAround(&p, sizeof(p));

	m_pkMiningEvent = mining::CreateMiningEvent(this, chLoad, count);
}
// @duzenleme
// abuk subuk yerlerde balik tutmasinlar diye koyduk.
// Rubinum Metin2AR
bool CHARACTER::IS_VALID_FISHING_POSITION(long* returnPosx, long* returnPosy) const
{
	long charX = GetX();
	long charY = GetY();

	LPSECTREE curWaterPostitionTree;

	long fX, fY;
	for (float fRot = 0.0f; fRot <= 180.0f; fRot += 10.0f) //mimics behaviour of client.
	{
		ELPlainCoord_GetRotatedPixelPosition(charX, charY, 600.0f, GetRotation() + fRot, &fX, &fY);
		curWaterPostitionTree = SECTREE_MANAGER::instance().Get(GetMapIndex(), fX, fY);
		if (curWaterPostitionTree && curWaterPostitionTree->IsAttr(fX, fY, ATTR_WATER)) {
			*returnPosx = fX;
			*returnPosy = fY;
			return true;
		}
		//No idea if thats needed client uses it.
		ELPlainCoord_GetRotatedPixelPosition(charX, charY, 600.0f, GetRotation() - fRot, &fX, &fY);
		curWaterPostitionTree = SECTREE_MANAGER::instance().Get(GetMapIndex(), fX, fY);
		if (curWaterPostitionTree && curWaterPostitionTree->IsAttr(fX, fY, ATTR_WATER)) {
			*returnPosx = fX;
			*returnPosy = fY;
			return true;
		}
	}

	return false;
}

void CHARACTER::fishing()
{
	if (m_pkFishingEvent)
	{
		fishing_take();
		return;
	}

#ifdef ENABLE_MULTI_FARM_BLOCK
	if (!GetMultiStatus())
		return;
#endif

	//@duzenleme eger elimizde olta yoksa bunu yapamiyoruz fakat inputlara tasinmasi gerekli.
	LPITEM rod = GetWear(WEAR_WEAPON);

	if (!rod || rod->GetType() != ITEM_ROD)
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("³¬½Ã´ë¸¦ ÀåÂø ÇÏ¼¼¿ä."));
		return;
	}

	if (0 == rod->GetSocket(2))
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¹Ì³¢¸¦ ³¢°í ´øÁ® ÁÖ¼¼¿ä."));
		return;
	}

	{
		LPSECTREE_MAP pkSectreeMap = SECTREE_MANAGER::instance().GetMap(GetMapIndex());

		int	x = GetX();
		int y = GetY();

		LPSECTREE tree = pkSectreeMap->Find(x, y);
		DWORD dwAttr = tree->GetAttribute(x, y);

		if (IS_SET(dwAttr, ATTR_BLOCK))
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("³¬½Ã¸¦ ÇÒ ¼ö ÀÖ´Â °÷ÀÌ ¾Æ´Õ´Ï´Ù"));
			return;
		}

		long newPosx, newPosy;
		if (!IS_VALID_FISHING_POSITION(&newPosx, &newPosy)) {
			LogManager::instance().HackLog("FISH_BOT_LOCATION", this);
			return;
		}

		SetRotationToXY(newPosx, newPosy);
	}

	m_pkFishingEvent = fishing::CreateFishingEvent(this);
}

void CHARACTER::fishing_take()
{
	LPITEM rod = GetWear(WEAR_WEAPON);
	if (rod && rod->GetType() == ITEM_ROD)
	{
		using fishing::fishing_event_info;
		if (m_pkFishingEvent)
		{
			struct fishing_event_info* info = dynamic_cast<struct fishing_event_info*>(m_pkFishingEvent->info);

			if (info)
				fishing::Take(info, this);
		}
	}
	else
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("³¬½Ã´ë°¡ ¾Æ´Ñ ¹°°ÇÀ¸·Î ³¬½Ã¸¦ ÇÒ ¼ö ¾ø½À´Ï´Ù!"));
	}

	event_cancel(&m_pkFishingEvent);
}

bool CHARACTER::StartStateMachine(int iNextPulse)
{
	if (CHARACTER_MANAGER::instance().AddToStateList(this))
	{
		m_dwNextStatePulse = thecore_heart->pulse + iNextPulse;
		return true;
	}

	return false;
}

void CHARACTER::StopStateMachine()
{
	CHARACTER_MANAGER::instance().RemoveFromStateList(this);
}

void CHARACTER::UpdateStateMachine(DWORD dwPulse)
{
	if (dwPulse < m_dwNextStatePulse)
		return;

	if (IsDead())
		return;

	Update();
	m_dwNextStatePulse = dwPulse + m_dwStateDuration;
}

void CHARACTER::SetNextStatePulse(int iNextPulse)
{
	CHARACTER_MANAGER::instance().AddToStateList(this);
	m_dwNextStatePulse = iNextPulse;
}

void CHARACTER::UpdateCharacter(DWORD dwPulse)
{
	CFSM::Update();
}

void CHARACTER::SetShop(LPSHOP pkShop)
{
	if ((m_pkShop = pkShop))
		SET_BIT(m_pointsInstant.instant_flag, INSTANT_FLAG_SHOP);
	else
	{
		REMOVE_BIT(m_pointsInstant.instant_flag, INSTANT_FLAG_SHOP);
		SetShopOwner(NULL);
	}
}

void CHARACTER::SetExchange(CExchange * pkExchange)
{
	m_pkExchange = pkExchange;
}

void CHARACTER::SetPart(BYTE bPartPos, DWORD wVal)
{
	assert(bPartPos < PART_MAX_NUM);
	m_pointsInstant.parts[bPartPos] = wVal;
}

DWORD CHARACTER::GetPart(BYTE bPartPos) const
{
	assert(bPartPos < PART_MAX_NUM);


	if ((bPartPos == PART_MAIN && IsBlockMode(BLOCK_COSTUME_BODY) == 1) && GetWear(WEAR_COSTUME_BODY) != NULL)
		return GetPartMainWithoutCostume();
	else if ((bPartPos == PART_HAIR && IsBlockMode(BLOCK_COSTUME_HAIR) == 1) && GetWear(WEAR_COSTUME_HAIR) != NULL)
		return 0;
#ifdef __WEAPON_COSTUME_SYSTEM__
	else if ((bPartPos == PART_WEAPON && IsBlockMode(BLOCK_COSTUME_WEAPON) == 1) && GetWear(WEAR_COSTUME_WEAPON) != NULL)
		return GetPartWeaponWithoutCostume();
#endif // __WEAPON_COSTUME_SYSTEM__
#ifdef __ACCE_SYSTEM__
	else if ((bPartPos == PART_ACCE && IsBlockMode(BLOCK_COSTUME_ACCE) == 1) && GetWear(WEAR_COSTUME_ACCE) != NULL)
		return 0;
#endif // __ACCE_SYSTEM__
#ifdef __AURA_COSTUME_SYSTEM__
	else if ((bPartPos == PART_AURA && IsBlockMode(BLOCK_COSTUME_AURA) == 1) && GetWear(WEAR_COSTUME_AURA) != NULL)
		return 0;
#endif // __AURA_COSTUME_SYSTEM__


	return m_pointsInstant.parts[bPartPos];
}

DWORD CHARACTER::GetOriginalPart(BYTE bPartPos) const
{
	switch (bPartPos)
	{
	case PART_MAIN:
		if (!IsPC())
			return GetPart(PART_MAIN);
		else
			return m_pointsInstant.bBasePart;

	case PART_HAIR:
		return GetPart(PART_HAIR);

#ifdef __WEAPON_COSTUME_SYSTEM__
	case PART_WEAPON:
		return GetPart(PART_WEAPON);
#endif

#ifdef __ACCE_SYSTEM__
	case PART_ACCE:
		return GetPart(PART_ACCE);
#endif

#ifdef __AURA_COSTUME_SYSTEM__
	case PART_AURA:
		return GetPart(PART_AURA);
#endif

	default:
		return 0;
	}
}

BYTE CHARACTER::GetCharType() const
{
	return m_bCharType;
}

bool CHARACTER::SetSyncOwner(LPCHARACTER ch, bool bRemoveFromList)
{
	// TRENT_MONSTER
	if (IS_SET(m_pointsInstant.dwAIFlag, AIFLAG_NOMOVE))
		return false;
	// END_OF_TRENT_MONSTER

	if (ch) // @duzenleme ch varsa demisizde kardesim call fonksiyonundan input fonksiyonuna gidiyor ch yoksa oraya konmasi lazim buraya degil.
	{
		if (!battle_is_attackable(ch, this))
		{
			SendDamagePacket(ch, 0, DAMAGE_BLOCK);
			return false;
		}
	}

	if (ch == this)
	{
		sys_err("SetSyncOwner owner == this (%p)", this);
		return false;
	}

	if (!ch)
	{
		if (bRemoveFromList && m_pkChrSyncOwner)
		{
			m_pkChrSyncOwner->m_kLst_pkChrSyncOwned.remove(this);
		}

		if (m_pkChrSyncOwner)
			sys_log(1, "SyncRelease %s %p from %s", GetName(), this, m_pkChrSyncOwner->GetName());

		m_pkChrSyncOwner = NULL;
	}
	else
	{
		if (!IsSyncOwner(ch))
			return false;

		if (DISTANCE_APPROX(GetX() - ch->GetX(), GetY() - ch->GetY()) > 250)
		{
			sys_log(1, "SetSyncOwner distance over than 250 %s %s", GetName(), ch->GetName());

			if (m_pkChrSyncOwner == ch)
				return true;

			return false;
		}

		if (m_pkChrSyncOwner != ch)
		{
			if (m_pkChrSyncOwner)
			{
				sys_log(1, "SyncRelease %s %p from %s", GetName(), this, m_pkChrSyncOwner->GetName());
				m_pkChrSyncOwner->m_kLst_pkChrSyncOwned.remove(this);
			}

			m_pkChrSyncOwner = ch;
			m_pkChrSyncOwner->m_kLst_pkChrSyncOwned.push_back(this);

			static const timeval zero_tv = { 0, 0 };
			SetLastSyncTime(zero_tv);

			sys_log(1, "SetSyncOwner set %s %p to %s", GetName(), this, ch->GetName());
		}

		m_fSyncTime = get_float_time();
	}

	TPacketGCOwnership pack;

	pack.bHeader = HEADER_GC_OWNERSHIP;
	pack.dwOwnerVID = ch ? ch->GetVID() : 0;
	pack.dwVictimVID = GetVID();

	PacketAround(&pack, sizeof(TPacketGCOwnership));
	return true;
}

struct FuncClearSync
{
	void operator () (LPCHARACTER ch)
	{
		assert(ch != NULL);
		ch->SetSyncOwner(NULL, false);	// false
	}
};

void CHARACTER::ClearSync()
{
	SetSyncOwner(NULL);

	std::for_each(m_kLst_pkChrSyncOwned.begin(), m_kLst_pkChrSyncOwned.end(), FuncClearSync());
	m_kLst_pkChrSyncOwned.clear();
}

bool CHARACTER::IsSyncOwner(LPCHARACTER ch) const
{
	if (m_pkChrSyncOwner == ch)
		return true;

	if (get_float_time() - m_fSyncTime >= 3.0f)
		return true;

	return false;
}

void CHARACTER::SetParty(LPPARTY pkParty)
{
	if (pkParty == m_pkParty)
		return;

	if (pkParty && m_pkParty)
		sys_err("%s is trying to reassigning party (current %p, new party %p)", GetName(), get_pointer(m_pkParty), get_pointer(pkParty));

	sys_log(1, "PARTY set to %p", get_pointer(pkParty));

	m_pkParty = pkParty;

	if (IsPC())
	{
		if (m_pkParty)
			SET_BIT(m_bAddChrState, ADD_CHARACTER_STATE_PARTY);
		else
			REMOVE_BIT(m_bAddChrState, ADD_CHARACTER_STATE_PARTY);

		UpdatePacket();
	}
}

// PARTY_JOIN_BUG_FIX
EVENTINFO(TPartyJoinEventInfo)
{
	DWORD	dwGuestPID;
	DWORD	dwLeaderPID;

	TPartyJoinEventInfo()
		: dwGuestPID(0)
		, dwLeaderPID(0)
	{
	}
};

EVENTFUNC(party_request_event)
{
	TPartyJoinEventInfo* info = dynamic_cast<TPartyJoinEventInfo*>(event->info);

	if (info == NULL)
	{
		sys_err("party_request_event> <Factor> Null pointer");
		return 0;
	}

	LPCHARACTER ch = CHARACTER_MANAGER::instance().FindByPID(info->dwGuestPID);

	if (ch)
	{
		sys_log(0, "PartyRequestEvent %s", ch->GetName());
		ch->ChatPacket(CHAT_TYPE_COMMAND, "PartyRequestDenied");
		ch->SetPartyRequestEvent(NULL);
	}

	return 0;
}

bool CHARACTER::RequestToParty(LPCHARACTER leader)
{
	if (leader->GetParty())
		leader = leader->GetParty()->GetLeaderCharacter();

	if (!leader)
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÆÄÆ¼ÀåÀÌ Á¢¼Ó »óÅÂ°¡ ¾Æ´Ï¶ó¼­ ¿äÃ»À» ÇÒ ¼ö ¾ø½À´Ï´Ù."));
		return false;
	}

	if (m_pkPartyRequestEvent)
		return false;

	if (!IsPC() || !leader->IsPC())
		return false;

	if (leader->IsBlockMode(BLOCK_PARTY_REQUEST))
		return false;

	PartyJoinErrCode errcode = IsPartyJoinableCondition(leader, this);

	switch (errcode)
	{
	case PERR_NONE:
		break;

	case PERR_SERVER:
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<ÆÄÆ¼> ¼­¹ö ¹®Á¦·Î ÆÄÆ¼ °ü·Ã Ã³¸®¸¦ ÇÒ ¼ö ¾ø½À´Ï´Ù."));
		return false;

	case PERR_DIFFEMPIRE:
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<ÆÄÆ¼> ´Ù¸¥ Á¦±¹°ú ÆÄÆ¼¸¦ ÀÌ·ê ¼ö ¾ø½À´Ï´Ù."));
		return false;

	case PERR_DUNGEON:
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<ÆÄÆ¼> ´øÀü ¾È¿¡¼­´Â ÆÄÆ¼ ÃÊ´ë¸¦ ÇÒ ¼ö ¾ø½À´Ï´Ù."));
		return false;

	case PERR_OBSERVER:
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<ÆÄÆ¼> °üÀü ¸ðµå¿¡¼± ÆÄÆ¼ ÃÊ´ë¸¦ ÇÒ ¼ö ¾ø½À´Ï´Ù."));
		return false;

	case PERR_LVBOUNDARY:
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<ÆÄÆ¼> -30 ~ +30 ·¹º§ ÀÌ³»ÀÇ »ó´ë¹æ¸¸ ÃÊ´ëÇÒ ¼ö ÀÖ½À´Ï´Ù."));
		return false;

	case PERR_LOWLEVEL:
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<ÆÄÆ¼> ÆÄÆ¼³» ÃÖ°í ·¹º§ º¸´Ù 30·¹º§ÀÌ ³·¾Æ ÃÊ´ëÇÒ ¼ö ¾ø½À´Ï´Ù."));
		return false;

	case PERR_HILEVEL:
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<ÆÄÆ¼> ÆÄÆ¼³» ÃÖÀú ·¹º§ º¸´Ù 30·¹º§ÀÌ ³ô¾Æ ÃÊ´ëÇÒ ¼ö ¾ø½À´Ï´Ù."));
		return false;

	case PERR_ALREADYJOIN:
		return false;

	case PERR_PARTYISFULL:
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<ÆÄÆ¼> ´õ ÀÌ»ó ÆÄÆ¼¿øÀ» ÃÊ´ëÇÒ ¼ö ¾ø½À´Ï´Ù."));
		return false;

	default:
		sys_err("Do not process party join error(%d)", errcode);
		return false;
	}

	TPartyJoinEventInfo* info = AllocEventInfo<TPartyJoinEventInfo>();

	info->dwGuestPID = GetPlayerID();
	info->dwLeaderPID = leader->GetPlayerID();

	SetPartyRequestEvent(event_create(party_request_event, info, PASSES_PER_SEC(10)));

	leader->ChatPacket(CHAT_TYPE_COMMAND, "PartyRequest %u", (DWORD)GetVID());
	ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%s ´Ô¿¡°Ô ÆÄÆ¼°¡ÀÔ ½ÅÃ»À» Çß½À´Ï´Ù."), leader->GetName());
	return true;
}

void CHARACTER::DenyToParty(LPCHARACTER member)
{
	sys_log(1, "DenyToParty %s member %s %p", GetName(), member->GetName(), get_pointer(member->m_pkPartyRequestEvent));

	if (!member->m_pkPartyRequestEvent)
		return;

	TPartyJoinEventInfo* info = dynamic_cast<TPartyJoinEventInfo*>(member->m_pkPartyRequestEvent->info);

	if (!info)
	{
		sys_err("CHARACTER::DenyToParty> <Factor> Null pointer");
		return;
	}

	if (info->dwGuestPID != member->GetPlayerID())
		return;

	if (info->dwLeaderPID != GetPlayerID())
		return;

	event_cancel(&member->m_pkPartyRequestEvent);

	member->ChatPacket(CHAT_TYPE_COMMAND, "PartyRequestDenied");
}

void CHARACTER::AcceptToParty(LPCHARACTER member)
{
	sys_log(1, "AcceptToParty %s member %s %p", GetName(), member->GetName(), get_pointer(member->m_pkPartyRequestEvent));

	if (!member->m_pkPartyRequestEvent)
		return;

	TPartyJoinEventInfo* info = dynamic_cast<TPartyJoinEventInfo*>(member->m_pkPartyRequestEvent->info);

	if (!info)
	{
		sys_err("CHARACTER::AcceptToParty> <Factor> Null pointer");
		return;
	}

	if (info->dwGuestPID != member->GetPlayerID())
		return;

	if (info->dwLeaderPID != GetPlayerID())
		return;

	event_cancel(&member->m_pkPartyRequestEvent);

	if (!GetParty())
		member->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("»ó´ë¹æÀÌ ÆÄÆ¼¿¡ ¼ÓÇØÀÖÁö ¾Ê½À´Ï´Ù."));
	else
	{
		if (GetPlayerID() != GetParty()->GetLeaderPID())
			return;

		PartyJoinErrCode errcode = IsPartyJoinableCondition(this, member);
		switch (errcode)
		{
		case PERR_NONE:
			member->PartyJoin(this);
			return;
		case PERR_SERVER:
			member->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<¨¡A¨¡¨ù> ¨ù¡©©öo ©ö¢çA|¡¤I ¨¡A¨¡¨ù ¡Æu¡¤A A©ø¢¬¢ç¢¬| CO ¨ùo ¨ú©ª¨öA¢¥I¢¥U."));
			break;
		case PERR_DUNGEON:
			member->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<¨¡A¨¡¨ù> ¢¥©ªAu ¨úE¢¯¢®¨ù¡©¢¥A ¨¡A¨¡¨ù AE¢¥e¢¬| CO ¨ùo ¨ú©ª¨öA¢¥I¢¥U."));
			break;
		case PERR_OBSERVER:
			member->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<¨¡A¨¡¨ù> ¡ÆuAu ¢¬©£¥ìa¢¯¢®¨ù¡¾ ¨¡A¨¡¨ù AE¢¥e¢¬| CO ¨ùo ¨ú©ª¨öA¢¥I¢¥U."));
			break;
		case PERR_LVBOUNDARY:
			member->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<¨¡A¨¡¨ù> -30 ~ +30 ¡¤©ö¨¬¡× AI©ø¡íAC ¡ío¢¥e©ö©¡¢¬¢¬ AE¢¥eCO ¨ùo AO¨öA¢¥I¢¥U."));
			break;
		case PERR_LOWLEVEL:
			member->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<¨¡A¨¡¨ù> ¨¡A¨¡¨ù©ø¡í AO¡Æi ¡¤©ö¨¬¡× ¨¬¢¬¢¥U 30¡¤©ö¨¬¡×AI ©ø¡¤¨ú¨¡ AE¢¥eCO ¨ùo ¨ú©ª¨öA¢¥I¢¥U."));
			break;
		case PERR_HILEVEL:
			member->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<¨¡A¨¡¨ù> ¨¡A¨¡¨ù©ø¡í AOAu ¡¤©ö¨¬¡× ¨¬¢¬¢¥U 30¡¤©ö¨¬¡×AI ©øo¨ú¨¡ AE¢¥eCO ¨ùo ¨ú©ª¨öA¢¥I¢¥U."));
			break;
		case PERR_ALREADYJOIN:
			break;
		case PERR_PARTYISFULL:
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<¨¡A¨¡¨ù> ¢¥o AI¡ío ¨¡A¨¡¨ù¢¯©ªA¡í AE¢¥eCO ¨ùo ¨ú©ª¨öA¢¥I¢¥U."));
			member->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<¨¡A¨¡¨ù> ¨¡A¨¡¨ùAC AI¢¯©ªA|CNAI AE¡ÆuCI¢¯¨Ï ¨¡A¨¡¨ù¢¯¢® Au¡Æ¢®CO ¨ùo ¨ú©ª¨öA¢¥I¢¥U."));
			break;
		}
		default:
			sys_err("Do not process party join error(%d)", errcode);
		}
	}

	member->ChatPacket(CHAT_TYPE_COMMAND, "PartyRequestDenied");
}

EVENTFUNC(party_invite_event)
{
	TPartyJoinEventInfo* pInfo = dynamic_cast<TPartyJoinEventInfo*>(event->info);

	if (pInfo == NULL)
	{
		sys_err("party_invite_event> <Factor> Null pointer");
		return 0;
	}

	LPCHARACTER pchInviter = CHARACTER_MANAGER::instance().FindByPID(pInfo->dwLeaderPID);

	if (pchInviter)
	{
		sys_log(1, "PartyInviteEvent %s", pchInviter->GetName());
		pchInviter->PartyInviteDeny(pInfo->dwGuestPID);
	}

	return 0;
}

void CHARACTER::PartyInvite(LPCHARACTER pchInvitee)
{
	if (GetParty() && GetParty()->GetLeaderPID() != GetPlayerID())
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<ÆÄÆ¼> ÆÄÆ¼¿øÀ» ÃÊ´ëÇÒ ¼ö ÀÖ´Â ±ÇÇÑÀÌ ¾ø½À´Ï´Ù."));
		return;
	}
	else if (pchInvitee->IsBlockMode(BLOCK_PARTY_INVITE))
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<ÆÄÆ¼> %s ´ÔÀÌ ÆÄÆ¼ °ÅºÎ »óÅÂÀÔ´Ï´Ù."), pchInvitee->GetName());
		return;
	}

	PartyJoinErrCode errcode = IsPartyJoinableCondition(this, pchInvitee);

	switch (errcode)
	{
	case PERR_NONE:
		break;

	case PERR_SERVER:
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<ÆÄÆ¼> ¼­¹ö ¹®Á¦·Î ÆÄÆ¼ °ü·Ã Ã³¸®¸¦ ÇÒ ¼ö ¾ø½À´Ï´Ù."));
		return;

	case PERR_DIFFEMPIRE:
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<ÆÄÆ¼> ´Ù¸¥ Á¦±¹°ú ÆÄÆ¼¸¦ ÀÌ·ê ¼ö ¾ø½À´Ï´Ù."));
		return;

	case PERR_DUNGEON:
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<ÆÄÆ¼> ´øÀü ¾È¿¡¼­´Â ÆÄÆ¼ ÃÊ´ë¸¦ ÇÒ ¼ö ¾ø½À´Ï´Ù."));
		return;

	case PERR_OBSERVER:
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<ÆÄÆ¼> °üÀü ¸ðµå¿¡¼± ÆÄÆ¼ ÃÊ´ë¸¦ ÇÒ ¼ö ¾ø½À´Ï´Ù."));
		return;

	case PERR_LVBOUNDARY:
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<ÆÄÆ¼> -30 ~ +30 ·¹º§ ÀÌ³»ÀÇ »ó´ë¹æ¸¸ ÃÊ´ëÇÒ ¼ö ÀÖ½À´Ï´Ù."));
		return;

	case PERR_LOWLEVEL:
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<ÆÄÆ¼> ÆÄÆ¼³» ÃÖ°í ·¹º§ º¸´Ù 30·¹º§ÀÌ ³·¾Æ ÃÊ´ëÇÒ ¼ö ¾ø½À´Ï´Ù."));
		return;

	case PERR_HILEVEL:
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<ÆÄÆ¼> ÆÄÆ¼³» ÃÖÀú ·¹º§ º¸´Ù 30·¹º§ÀÌ ³ô¾Æ ÃÊ´ëÇÒ ¼ö ¾ø½À´Ï´Ù."));
		return;

	case PERR_ALREADYJOIN:
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<ÆÄÆ¼> ÀÌ¹Ì %s´ÔÀº ÆÄÆ¼¿¡ ¼ÓÇØ ÀÖ½À´Ï´Ù."), pchInvitee->GetName());
		return;

	case PERR_PARTYISFULL:
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<ÆÄÆ¼> ´õ ÀÌ»ó ÆÄÆ¼¿øÀ» ÃÊ´ëÇÒ ¼ö ¾ø½À´Ï´Ù."));
		return;

	default:
		sys_err("Do not process party join error(%d)", errcode);
		return;
	}

	if (m_PartyInviteEventMap.end() != m_PartyInviteEventMap.find(pchInvitee->GetPlayerID()))
		return;

	TPartyJoinEventInfo* info = AllocEventInfo<TPartyJoinEventInfo>();

	info->dwGuestPID = pchInvitee->GetPlayerID();
	info->dwLeaderPID = GetPlayerID();

	m_PartyInviteEventMap.insert(EventMap::value_type(pchInvitee->GetPlayerID(), event_create(party_invite_event, info, PASSES_PER_SEC(10))));

	TPacketGCPartyInvite p;
	p.header = HEADER_GC_PARTY_INVITE;
	p.leader_vid = GetVID();
	pchInvitee->GetDesc()->Packet(&p, sizeof(p));
}

void CHARACTER::PartyInviteAccept(LPCHARACTER pchInvitee)
{
	EventMap::iterator itFind = m_PartyInviteEventMap.find(pchInvitee->GetPlayerID());

	if (itFind == m_PartyInviteEventMap.end())
	{
		sys_log(1, "PartyInviteAccept from not invited character(%s)", pchInvitee->GetName());
		return;
	}

	event_cancel(&itFind->second);
	m_PartyInviteEventMap.erase(itFind);

	if (GetParty() && GetParty()->GetLeaderPID() != GetPlayerID())
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<ÆÄÆ¼> ÆÄÆ¼¿øÀ» ÃÊ´ëÇÒ ¼ö ÀÖ´Â ±ÇÇÑÀÌ ¾ø½À´Ï´Ù."));
		return;
	}

	PartyJoinErrCode errcode = IsPartyJoinableMutableCondition(this, pchInvitee);

	switch (errcode)
	{
	case PERR_NONE:
		break;

	case PERR_SERVER:
		pchInvitee->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<ÆÄÆ¼> ¼­¹ö ¹®Á¦·Î ÆÄÆ¼ °ü·Ã Ã³¸®¸¦ ÇÒ ¼ö ¾ø½À´Ï´Ù."));
		return;

	case PERR_DUNGEON:
		pchInvitee->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<ÆÄÆ¼> ´øÀü ¾È¿¡¼­´Â ÆÄÆ¼ ÃÊ´ë¿¡ ÀÀÇÒ ¼ö ¾ø½À´Ï´Ù."));
		return;

	case PERR_OBSERVER:
		pchInvitee->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<ÆÄÆ¼> °üÀü ¸ðµå¿¡¼± ÆÄÆ¼ ÃÊ´ë¸¦ ÇÒ ¼ö ¾ø½À´Ï´Ù."));
		return;

	case PERR_LVBOUNDARY:
		pchInvitee->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<ÆÄÆ¼> -30 ~ +30 ·¹º§ ÀÌ³»ÀÇ »ó´ë¹æ¸¸ ÃÊ´ëÇÒ ¼ö ÀÖ½À´Ï´Ù."));
		return;

	case PERR_LOWLEVEL:
		pchInvitee->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<ÆÄÆ¼> ÆÄÆ¼³» ÃÖ°í ·¹º§ º¸´Ù 30·¹º§ÀÌ ³·¾Æ ÃÊ´ëÇÒ ¼ö ¾ø½À´Ï´Ù."));
		return;

	case PERR_HILEVEL:
		pchInvitee->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<ÆÄÆ¼> ÆÄÆ¼³» ÃÖÀú ·¹º§ º¸´Ù 30·¹º§ÀÌ ³ô¾Æ ÃÊ´ëÇÒ ¼ö ¾ø½À´Ï´Ù."));
		return;

	case PERR_ALREADYJOIN:
		pchInvitee->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<ÆÄÆ¼> ÆÄÆ¼ ÃÊ´ë¿¡ ÀÀÇÒ ¼ö ¾ø½À´Ï´Ù."));
		return;

	case PERR_PARTYISFULL:
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<ÆÄÆ¼> ´õ ÀÌ»ó ÆÄÆ¼¿øÀ» ÃÊ´ëÇÒ ¼ö ¾ø½À´Ï´Ù."));
		pchInvitee->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<ÆÄÆ¼> ÆÄÆ¼ÀÇ ÀÎ¿øÁ¦ÇÑÀÌ ÃÊ°úÇÏ¿© ÆÄÆ¼¿¡ Âü°¡ÇÒ ¼ö ¾ø½À´Ï´Ù."));
		return;

	default:
		sys_err("ignore party join error(%d)", errcode);
		return;
	}

	if (GetParty())
		pchInvitee->PartyJoin(this);
	else
	{
		LPPARTY pParty = CPartyManager::instance().CreateParty(this);

		pParty->Join(pchInvitee->GetPlayerID());
		pParty->Link(pchInvitee);
		pParty->SendPartyInfoAllToOne(this);
	}
}

void CHARACTER::PartyInviteDeny(DWORD dwPID)
{
	EventMap::iterator itFind = m_PartyInviteEventMap.find(dwPID);

	if (itFind == m_PartyInviteEventMap.end())
	{
		sys_log(1, "PartyInviteDeny to not exist event(inviter PID: %d, invitee PID: %d)", GetPlayerID(), dwPID);
		return;
	}

	event_cancel(&itFind->second);
	m_PartyInviteEventMap.erase(itFind);

	LPCHARACTER pchInvitee = CHARACTER_MANAGER::instance().FindByPID(dwPID);
	if (pchInvitee)
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<ÆÄÆ¼> %s´ÔÀÌ ÆÄÆ¼ ÃÊ´ë¸¦ °ÅÀýÇÏ¼Ì½À´Ï´Ù."), pchInvitee->GetName());
}

void CHARACTER::PartyJoin(LPCHARACTER pLeader)
{
	pLeader->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<ÆÄÆ¼> %s´ÔÀÌ ÆÄÆ¼¿¡ Âü°¡ÇÏ¼Ì½À´Ï´Ù."), GetName());
	ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<ÆÄÆ¼> %s´ÔÀÇ ÆÄÆ¼¿¡ Âü°¡ÇÏ¼Ì½À´Ï´Ù."), pLeader->GetName());

	pLeader->GetParty()->Join(GetPlayerID());
	pLeader->GetParty()->Link(this);
}

CHARACTER::PartyJoinErrCode CHARACTER::IsPartyJoinableCondition(const LPCHARACTER pchLeader, const LPCHARACTER pchGuest)
{
	if (pchLeader->GetEmpire() != pchGuest->GetEmpire())
		return PERR_DIFFEMPIRE;

	return IsPartyJoinableMutableCondition(pchLeader, pchGuest);
}

static bool __party_can_join_by_level(LPCHARACTER leader, LPCHARACTER quest)
{
	int	level_limit = 30;
	return (abs(leader->GetLevel() - quest->GetLevel()) <= level_limit);
}

CHARACTER::PartyJoinErrCode CHARACTER::IsPartyJoinableMutableCondition(const LPCHARACTER pchLeader, const LPCHARACTER pchGuest)
{
	if (!CPartyManager::instance().IsEnablePCParty())
		return PERR_SERVER;
	else if (pchLeader->GetDungeon())
		return PERR_DUNGEON;
	else if (pchGuest->IsObserverMode())
		return PERR_OBSERVER;
	else if (false == __party_can_join_by_level(pchLeader, pchGuest))
		return PERR_LVBOUNDARY;
	else if (pchGuest->GetParty())
		return PERR_ALREADYJOIN;
	else if (pchLeader->GetParty())
	{
		if (pchLeader->GetParty()->GetMemberCount() == PARTY_MAX_MEMBER)
			return PERR_PARTYISFULL;
	}

	return PERR_NONE;
}
// END_OF_PARTY_JOIN_BUG_FIX

void CHARACTER::SetDungeon(LPDUNGEON pkDungeon)
{
	if (pkDungeon && m_pkDungeon)
		sys_err("%s is trying to reassigning dungeon (current %p, new party %p)", GetName(), get_pointer(m_pkDungeon), get_pointer(pkDungeon));

	if (m_pkDungeon == pkDungeon) {
		return;
	}

	if (m_pkDungeon)
	{
		if (IsMonster() || IsStone())
		{
			m_pkDungeon->DecMonster();
		}
#ifdef __SCP1453_EXTENSIONS__
		else if (IsNPC())
		{
			m_pkDungeon->DecNpc();
		}
#endif
	}

	m_pkDungeon = pkDungeon;

	if (pkDungeon)
	{
		//sys_log(0, "%s DUNGEON set to %p, PARTY is %p", GetName(), get_pointer(pkDungeon), get_pointer(m_pkParty));

		if (IsMonster() || IsStone())
		{
			m_pkDungeon->IncMonster();
		}
#ifdef __SCP1453_EXTENSIONS__
		else if (IsNPC())
		{
			m_pkDungeon->IncNpc();
		}
#endif
	}
}

void CHARACTER::SetWarMap(CWarMap * pWarMap)
{
	if (m_pWarMap)
		m_pWarMap->DecMember(this);

	m_pWarMap = pWarMap;

	if (m_pWarMap)
		m_pWarMap->IncMember(this);
}

void CHARACTER::SetRegen(LPREGEN pkRegen)
{
	m_pkRegen = pkRegen;
	if (pkRegen != NULL) {
		regen_id_ = pkRegen->id;
	}
	m_fRegenAngle = GetRotation();
	m_posRegen = GetXYZ();
}

bool CHARACTER::OnIdle()
{
	return false;
}

void CHARACTER::OnMove(bool bIsAttack)
{
	m_dwLastMoveTime = get_dword_time();

	if (bIsAttack)
	{
		m_dwLastAttackTime = m_dwLastMoveTime;

		if (IsAffectFlag(AFF_EUNHYUNG))
		{
			RemoveAffect(SKILL_EUNHYUNG);
			SetAffectedEunhyung();
		}
		else
		{
			ClearAffectedEunhyung();
		}
	}

	// MINING
	mining_cancel();
	// END_OF_MINING
}

void CHARACTER::OnClick(LPCHARACTER pkChrCauser)
{
	if (!pkChrCauser)
	{
		sys_err("OnClick %s by NULL", GetName());
		return;
	}

	DWORD vid = GetVID();
	sys_log(0, "OnClick %s[vnum %d ServerUniqueID %d, pid %d] by %s", GetName(), GetRaceNum(), vid, GetPlayerID(), pkChrCauser->GetName());

	{
		if (pkChrCauser->GetMyShop() && pkChrCauser != this)
		{
			sys_log(0, "OnClick Fail (%s->%s) - pc has shop", pkChrCauser->GetName(), GetName());
			return;
		}
	}

	{
		if (pkChrCauser->GetExchange())
		{
			sys_log(0, "OnClick Fail (%s->%s) - pc is exchanging", pkChrCauser->GetName(), GetName());
			return;
		}
	}

	if (IsPC())
	{
		if (!CTargetManager::instance().GetTargetInfo(pkChrCauser->GetPlayerID(), TARGET_TYPE_VID, GetVID()))
		{
			// 2005.03.17.myevan.
			if (GetMyShop())
			{
				if (pkChrCauser->IsDead() == true)
					return;

				//PREVENT_TRADE_WINDOW
				if (pkChrCauser == this)
				{
					if (GetExchange())
					{
						pkChrCauser->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_OPERATION_IS_FAILED_EXCHANGE_DLG_OPENED"));
						return;
					}

					if (GetShopOwner())
					{
						pkChrCauser->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_OPERATION_IS_FAILED_SHOPOWNER_DLG_OPENED"));
						return;
					}

					if (IsOpenSafebox())
					{
						pkChrCauser->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_OPERATION_IS_FAILED_SAFEBOX_DLG_OPENED"));
						return;
					}

					if (IsCubeOpen())
					{
						pkChrCauser->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_OPERATION_IS_FAILED_CUBE_DLG_OPENED"));
						return;
					}

					if (IsUnderRefine())
					{
						pkChrCauser->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_OPERATION_IS_FAILED_REFINE_DLG_OPENED"));
						return;
					}

#ifdef __ACCE_SYSTEM__
					if (isAcceOpened(true))
					{
						pkChrCauser->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_OPERATION_IS_FAILED_ACCE_COMB_DLG_OPENED"));
						return;
					}

					if (isAcceOpened(false))
					{
						pkChrCauser->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_OPERATION_IS_FAILED_ACCE_ABSORB_DLG_OPENED"));
						return;
					}
#endif

#ifdef __ITEM_CHANGELOOK__
					if (isChangeLookOpened())
					{
						pkChrCauser->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_OPERATION_IS_FAILED_CHANGELOOK_DLG_OPENED"));
						return;
					}
#endif

#ifdef __OFFLINE_SHOP__
					if (GetOfflineShopGuest() != NULL || GetShopSafebox())
					{
						pkChrCauser->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_OPERATION_IS_FAILED_OFFSHOP_DLG_OPENED"));
						return;
					}
#endif
				}
				else
				{
					if (pkChrCauser->GetExchange())
					{
						pkChrCauser->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_OPERATION_IS_FAILED_EXCHANGE_DLG_OPENED"));
						return;
					}

					if (pkChrCauser->GetMyShop())
					{
						pkChrCauser->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_OPERATION_IS_FAILED_SHOPVIEW_DLG_OPENED"));
						return;
					}

					if (pkChrCauser->GetShopOwner())
					{
						pkChrCauser->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_OPERATION_IS_FAILED_SHOPOWNER_DLG_OPENED"));
						return;
					}

					if (pkChrCauser->IsOpenSafebox())
					{
						pkChrCauser->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_OPERATION_IS_FAILED_SAFEBOX_DLG_OPENED"));
						return;
					}

					if (pkChrCauser->IsCubeOpen())
					{
						pkChrCauser->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_OPERATION_IS_FAILED_CUBE_DLG_OPENED"));
						return;
					}

					if (pkChrCauser->IsUnderRefine())
					{
						pkChrCauser->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_OPERATION_IS_FAILED_REFINE_DLG_OPENED"));
						return;
					}

#ifdef __ACCE_SYSTEM__
					if (pkChrCauser->isAcceOpened(true))
					{
						pkChrCauser->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_OPERATION_IS_FAILED_ACCE_COMB_DLG_OPENED"));
						return;
					}

					if (pkChrCauser->isAcceOpened(false))
					{
						pkChrCauser->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_OPERATION_IS_FAILED_ACCE_ABSORB_DLG_OPENED"));
						return;
					}
#endif

#ifdef __ITEM_CHANGELOOK__
					if (pkChrCauser->isChangeLookOpened())
					{
						pkChrCauser->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_OPERATION_IS_FAILED_CHANGELOOK_DLG_OPENED"));
						return;
					}
#endif

#ifdef __OFFLINE_SHOP__
					if (pkChrCauser->GetOfflineShopGuest() != NULL || pkChrCauser->GetShopSafebox())
					{
						pkChrCauser->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_OPERATION_IS_FAILED_OFFSHOP_DLG_OPENED"));
						return;
					}
#endif

					if (GetExchange())
					{
						pkChrCauser->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_OPERATION_IS_FAILED_EXCHANGE_DLG_OPENED"));
						return;
					}

					if (IsOpenSafebox())
					{
						pkChrCauser->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_OPERATION_IS_FAILED_SAFEBOX_DLG_OPENED"));
						return;
					}

					if (IsCubeOpen())
					{
						pkChrCauser->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_OPERATION_IS_FAILED_CUBE_DLG_OPENED"));
						return;
					}

					if (IsUnderRefine())
					{
						pkChrCauser->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_OPERATION_IS_FAILED_REFINE_DLG_OPENED"));
						return;
					}

#ifdef __ACCE_SYSTEM__
					if (isAcceOpened(true))
					{
						pkChrCauser->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_OPERATION_IS_FAILED_ACCE_COMB_DLG_OPENED"));
						return;
					}

					if (isAcceOpened(false))
					{
						pkChrCauser->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_OPERATION_IS_FAILED_ACCE_ABSORB_DLG_OPENED"));
						return;
					}
#endif

#ifdef __ITEM_CHANGELOOK__
					if (isChangeLookOpened())
					{
						pkChrCauser->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_OPERATION_IS_FAILED_CHANGELOOK_DLG_OPENED"));
						return;
					}
#endif

#ifdef __OFFLINE_SHOP__
					if (GetOfflineShopGuest() != NULL || GetShopSafebox())
					{
						pkChrCauser->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_OPERATION_IS_FAILED_OFFSHOP_DLG_OPENED"));
						return;
					}
#endif
				}
				//END_PREVENT_TRADE_WINDOW

				if (pkChrCauser->GetShop())
				{
					pkChrCauser->GetShop()->RemoveGuest(pkChrCauser);
					pkChrCauser->SetShop(NULL);
				}

				GetMyShop()->AddGuest(pkChrCauser, GetVID(), false);
				pkChrCauser->SetShopOwner(this);
				return;
			}

			if (test_server)
				sys_err("%s.OnClickFailure(%s) - target is PC", pkChrCauser->GetName(), GetName());

			return;
		}
	}

	pkChrCauser->SetQuestNPCID(GetVID());

	if (quest::CQuestManager::instance().Click(pkChrCauser->GetPlayerID(), this))
		return;

	if (!IsPC())
	{
		if (!m_triggerOnClick.pFunc)
			return;

		m_triggerOnClick.pFunc(this, pkChrCauser);
	}
}

BYTE CHARACTER::GetGMLevel() const
{
	if (test_server)
		return GM_IMPLEMENTOR;
	return m_pointsInstant.gm_level;
}

void CHARACTER::SetGMLevel()
{
	if (GetDesc())
		m_pointsInstant.gm_level = gm_get_level(GetName(), GetDesc()->GetHostName(), GetDesc()->GetAccountTable().login);
	else
		m_pointsInstant.gm_level = GM_PLAYER;
}

bool CHARACTER::IsGM() const
{
	if (m_pointsInstant.gm_level != GM_PLAYER)
		return true;

	if (test_server)
		return true;

	return false;
}

void CHARACTER::SetStone(LPCHARACTER pkChrStone)
{
	m_pkChrStone = pkChrStone;

	if (m_pkChrStone)
	{
		if (pkChrStone->m_set_pkChrSpawnedBy.find(this) == pkChrStone->m_set_pkChrSpawnedBy.end())
			pkChrStone->m_set_pkChrSpawnedBy.insert(this);
	}
}

struct FuncDeadSpawnedByStone
{
	void operator () (LPCHARACTER ch)
	{
		ch->Dead(NULL);
		ch->SetStone(NULL);
	}
};

void CHARACTER::ClearStone()
{
	if (!m_set_pkChrSpawnedBy.empty())
	{
		FuncDeadSpawnedByStone f;
		std::for_each(m_set_pkChrSpawnedBy.begin(), m_set_pkChrSpawnedBy.end(), f);
		m_set_pkChrSpawnedBy.clear();
	}

	if (!m_pkChrStone)
		return;

	m_pkChrStone->m_set_pkChrSpawnedBy.erase(this);
	m_pkChrStone = NULL;
}

void CHARACTER::ClearTarget()
{
	if (m_pkChrTarget)
	{
		m_pkChrTarget->m_set_pkChrTargetedBy.erase(this);
		m_pkChrTarget = NULL;
	}

	TPacketGCTarget p;

	p.header = HEADER_GC_TARGET;
	p.dwVID = 0;
	p.bHPPercent = 0;
	CHARACTER_SET::iterator it = m_set_pkChrTargetedBy.begin();

	while (it != m_set_pkChrTargetedBy.end())
	{
		LPCHARACTER pkChr = *(it++);
		pkChr->m_pkChrTarget = NULL;

		if (!pkChr->GetDesc())
		{
			sys_err("%s %p does not have desc", pkChr->GetName(), get_pointer(pkChr));
			abort();
		}

		pkChr->GetDesc()->Packet(&p, sizeof(TPacketGCTarget));
	}

	m_set_pkChrTargetedBy.clear();
}

void CHARACTER::SetTarget(LPCHARACTER pkChrTarget)
{
	if (m_pkChrTarget == pkChrTarget)
		return;

	if (m_pkChrTarget)
		m_pkChrTarget->m_set_pkChrTargetedBy.erase(this);

	m_pkChrTarget = pkChrTarget;

	TPacketGCTarget p;

	p.header = HEADER_GC_TARGET;

	if (m_pkChrTarget)
	{
		m_pkChrTarget->m_set_pkChrTargetedBy.insert(this);
#ifdef ENABLE_AUTO_HUNT_SYSTEM
		if (!m_pkChrTarget->HasAttacker() && m_pkChrTarget->IsStone())
		{
			m_pkChrTarget->SetHasAttacker(true);
			m_pkChrTarget->SetNormalPlayer(this);
			m_pkChrTarget->SetLastAttacked(get_global_time());
			m_pkChrTarget->UpdatePacket();
		}
#endif // ENABLE_AUTO_HUNT_SYSTEM

		p.dwVID = m_pkChrTarget->GetVID();

		if ((m_pkChrTarget->IsPC()) || (m_pkChrTarget->GetMaxHP() <= 0))
		{
			p.bHPPercent = 0;
#ifdef __TARGET_BOARD_RENEWAL__
			p.dwHP = 0;
			p.dwMaxHP = 0;
#endif
		}
		else
		{
			if (m_pkChrTarget->GetRaceNum() == 20101 ||
				m_pkChrTarget->GetRaceNum() == 20102 ||
				m_pkChrTarget->GetRaceNum() == 20103 ||
				m_pkChrTarget->GetRaceNum() == 20104 ||
				m_pkChrTarget->GetRaceNum() == 20105 ||
				m_pkChrTarget->GetRaceNum() == 20106)
			{
				LPCHARACTER owner = m_pkChrTarget->GetVictim();

				if (owner)
				{
					int iHorseHealth = owner->GetHorseHealth();
					int iHorseMaxHealth = owner->GetHorseMaxHealth();

					if (iHorseMaxHealth)
					{
						p.bHPPercent = MINMAX(0, iHorseHealth * 100 / iHorseMaxHealth, 100);
#ifdef __TARGET_BOARD_RENEWAL__
						p.dwHP = iHorseHealth;
						p.dwMaxHP = iHorseMaxHealth;
#endif
					}
					else
					{
						p.bHPPercent = 100;
#ifdef __TARGET_BOARD_RENEWAL__
						p.dwHP = iHorseHealth;
						p.dwMaxHP = iHorseMaxHealth;
#endif
					}
				}
				else
				{
					p.bHPPercent = 100;
#ifdef __TARGET_BOARD_RENEWAL__
					p.dwHP = 0;
					p.dwMaxHP = 0;
#endif
				}
			}
			else
			{
				if (m_pkChrTarget->GetMaxHP() <= 0) // @duzenleme HP Zero division (MartySama)
				{
					p.bHPPercent = 0;
#ifdef __TARGET_BOARD_RENEWAL__
					p.dwHP = 0;
					p.dwMaxHP = 0;
#endif
				}
				else
				{
					// float
					p.bHPPercent = m_pkChrTarget->GetHPPct();
#ifdef __TARGET_BOARD_RENEWAL__
					p.dwHP = m_pkChrTarget->GetHP();
					p.dwMaxHP = m_pkChrTarget->GetMaxHP();
#endif
				}
			}
		}
	}
	else
	{
		p.dwVID = 0;
		p.bHPPercent = 0;
#ifdef __TARGET_BOARD_RENEWAL__
		p.dwHP = 0;
		p.dwMaxHP = 0;
#endif
	}

	GetDesc()->Packet(&p, sizeof(TPacketGCTarget));
}

void CHARACTER::BroadcastTargetPacket()
{
	if (m_set_pkChrTargetedBy.empty())
		return;

	TPacketGCTarget p;

	p.header = HEADER_GC_TARGET;
	p.dwVID = GetVID();

	if (IsPC()
#ifdef __MOUNT_COSTUME_SYSTEM__
		|| IsMount()
#endif
	)
	{
		p.bHPPercent = 0;
#ifdef __TARGET_BOARD_RENEWAL__
		p.dwHP = 0;
		p.dwMaxHP = 0;
#endif
	}
	else if (GetMaxHP() <= 0) // @duzenleme HP Zero division (MartySama)
	{
		p.bHPPercent = 0;
#ifdef __TARGET_BOARD_RENEWAL__
		p.dwHP = 0;
		p.dwMaxHP = 0;
#endif
	}
	else
	{
		p.bHPPercent = GetHPPct();
#ifdef __TARGET_BOARD_RENEWAL__
		p.dwHP = GetHP();
		p.dwMaxHP = GetMaxHP();
#endif
	}

	CHARACTER_SET::iterator it = m_set_pkChrTargetedBy.begin();

	while (it != m_set_pkChrTargetedBy.end())
	{
		LPCHARACTER pkChr = *it++;

		if (!pkChr)
			return; // @duzenleme listede karakter yoksa diye koymusuz fakat listede karakterin olmamasi zaten bir sorun.

		if (!pkChr->GetDesc())
		{
			sys_err("%s %p does not have desc", pkChr->GetName(), get_pointer(pkChr));
			abort();
		}

		pkChr->GetDesc()->Packet(&p, sizeof(TPacketGCTarget));
	}
}

void CHARACTER::CheckTarget()
{
	if (!m_pkChrTarget)
		return;

	if (DISTANCE_APPROX(GetX() - m_pkChrTarget->GetX(), GetY() - m_pkChrTarget->GetY()) >= 4800)
		SetTarget(NULL);
}

void CHARACTER::SetWarpLocation(long lMapIndex, long x, long y)
{
	m_posWarp.x = x * 100;
	m_posWarp.y = y * 100;
	m_lWarpMapIndex = lMapIndex;
}

void CHARACTER::SaveExitLocation()
{
	m_posExit = GetXYZ();
	m_lExitMapIndex = GetMapIndex();
}

void CHARACTER::ExitToSavedLocation()
{
	sys_log(0, "ExitToSavedLocation");
	WarpSet(m_posWarp.x, m_posWarp.y, m_lWarpMapIndex);

	m_posExit.x = m_posExit.y = m_posExit.z = 0;
	m_lExitMapIndex = 0;
}

bool CHARACTER::WarpSet(long x, long y, long lPrivateMapIndex)
{
	if (!IsPC())
		return false;

	long lAddr;
	long lMapIndex;
	WORD wPort;

	if (!CMapLocation::instance().Get(x, y, lMapIndex, lAddr, wPort))
	{
		SetWarpLocation(EMPIRE_START_MAP(GetEmpire()), // @duzenleme isinlandigim yer kambocya ise beni koye geri gonderiyor.
			EMPIRE_START_X(GetEmpire()) / 100,
			EMPIRE_START_Y(GetEmpire()) / 100);
		return false;
	}

	if (lPrivateMapIndex >= 10000)
	{
		// if (lPrivateMapIndex / 10000 != lMapIndex)
		// {
		// 	sys_err("Invalid map index %d, must be child of %d", lPrivateMapIndex, lMapIndex);
		// 	return false;
		// }

		lMapIndex = lPrivateMapIndex;
	}

	Stop();
	Save();

	if (GetSectree())
	{
		GetSectree()->RemoveEntity(this);
		ViewCleanup();

		EncodeRemovePacket(this);
	}

	m_lWarpMapIndex = lMapIndex;
	m_posWarp.x = x;
	m_posWarp.y = y;

	sys_log(0, "WarpSet %s %d %d current map %d target map %d", GetName(), x, y, GetMapIndex(), lMapIndex);

	TPacketGCWarp p;

	p.bHeader = HEADER_GC_WARP;
	p.lX = x;
	p.lY = y;
	p.lAddr = lAddr;
	if (!g_stProxyIP.empty())
		p.lAddr = inet_addr(g_stProxyIP.c_str());
	p.wPort = wPort;

#ifdef __SWITCHBOT__
	CSwitchbotManager::Instance().SetIsWarping(GetPlayerID(), true);

	if (p.wPort != mother_port)
		CSwitchbotManager::Instance().P2PSendSwitchbot(GetPlayerID(), p.wPort);
#endif

	GetDesc()->Packet(&p, sizeof(TPacketGCWarp));

	return true;
}

bool CHARACTER::WarpSet(long x, long y, long lPrivateMapIndex, long addr, WORD port)
{
	if (!IsPC())
		return false;

	Stop();
	Save();

	if (GetSectree())
	{
		GetSectree()->RemoveEntity(this);
		ViewCleanup();

		EncodeRemovePacket(this);
	}

	m_lWarpMapIndex = lPrivateMapIndex;
	m_posWarp.x = x;
	m_posWarp.y = y;

	sys_log(0, "WarpSet %s %d %d current map %d target map %d", GetName(), x, y, GetMapIndex(), lPrivateMapIndex);

	TPacketGCWarp p;

	p.bHeader = HEADER_GC_WARP;
	p.lX = x;
	p.lY = y;
	p.lAddr = addr;
	if (!g_stProxyIP.empty())
		p.lAddr = inet_addr(g_stProxyIP.c_str());
	p.wPort = port;

#ifdef __SWITCHBOT__
	CSwitchbotManager::Instance().SetIsWarping(GetPlayerID(), true);

	if (p.wPort != mother_port)
		CSwitchbotManager::Instance().P2PSendSwitchbot(GetPlayerID(), p.wPort);
#endif

	GetDesc()->Packet(&p, sizeof(TPacketGCWarp));

	return true;
}

bool CHARACTER::Return()
{
	if (!IsNPC())
		return false;

	int x, y;

	SetVictim(NULL);

	x = m_pkMobInst->m_posLastAttacked.x;
	y = m_pkMobInst->m_posLastAttacked.y;

	SetRotationToXY(x, y);

	if (!Goto(x, y))
		return false;

	SendMovePacket(FUNC_WAIT, 0, 0, 0, 0);

	if (test_server)
		sys_log(0, "%s %p Æ÷±âÇÏ°í µ¹¾Æ°¡ÀÚ! %d %d", GetName(), this, x, y);

	if (GetParty())
		GetParty()->SendMessage(this, PM_RETURN, x, y);

	return true;
}

bool CHARACTER::Follow(LPCHARACTER pkChr, float fMinDistance)
{
	if (IsPC())
	{
		sys_err("CHARACTER::Follow : PC cannot use this method", GetName());
		return false;
	}

	// TRENT_MONSTER
	if (IS_SET(m_pointsInstant.dwAIFlag, AIFLAG_NOMOVE))
	{
		if (pkChr->IsPC())
		{
			// If i'm in a party. I must obey party leader's AI.
			if (!GetParty() || !GetParty()->GetLeader() || GetParty()->GetLeader() == this)
			{
				if (get_dword_time() - m_pkMobInst->m_dwLastAttackedTime >= 15000)
				{
					if (m_pkMobData->m_table.wAttackRange < DISTANCE_APPROX(pkChr->GetX() - GetX(), pkChr->GetY() - GetY()))
						if (Return())
							return true;
				}
			}
		}
		return false;
	}
	// END_OF_TRENT_MONSTER

	long x = pkChr->GetX();
	long y = pkChr->GetY();

	if (pkChr->IsPC())
	{
		// If i'm in a party. I must obey party leader's AI.
		if (!GetParty() || !GetParty()->GetLeader() || GetParty()->GetLeader() == this)
		{
			if (get_dword_time() - m_pkMobInst->m_dwLastAttackedTime >= 15000)
			{
				if (5000 < DISTANCE_APPROX(m_pkMobInst->m_posLastAttacked.x - GetX(), m_pkMobInst->m_posLastAttacked.y - GetY()))
					if (Return())
						return true;
			}
		}
	}

	if (IsGuardNPC())
	{
		if (5000 < DISTANCE_APPROX(m_pkMobInst->m_posLastAttacked.x - GetX(), m_pkMobInst->m_posLastAttacked.y - GetY()))
			if (Return())
				return true;
	}

	if (pkChr->IsState(pkChr->m_stateMove) &&
		GetMobBattleType() != BATTLE_TYPE_RANGE &&
		GetMobBattleType() != BATTLE_TYPE_MAGIC &&
		false == IsPet()
#ifdef __GROWTH_PET_SYSTEM__
		&& false == IsNewPet()
#endif
#ifdef __MOUNT_COSTUME_SYSTEM__
		&& false == IsMount()
#endif
#ifdef __GROWTH_MOUNT_SYSTEM__
		&& false == IsNewMount()
#endif
		)
	{
		float rot = pkChr->GetRotation();
		float rot_delta = GetDegreeDelta(rot, GetDegreeFromPositionXY(GetX(), GetY(), pkChr->GetX(), pkChr->GetY()));

		float yourSpeed = pkChr->GetMoveSpeed();
		float mySpeed = GetMoveSpeed();

		float fDist = DISTANCE_SQRT(x - GetX(), y - GetY());
		float fFollowSpeed = mySpeed - yourSpeed * cos(rot_delta * M_PI / 180);

		if (fFollowSpeed >= 0.1f)
		{
			float fMeetTime = fDist / fFollowSpeed;
			float fYourMoveEstimateX, fYourMoveEstimateY;

			if (fMeetTime * yourSpeed <= 100000.0f)
			{
				GetDeltaByDegree(pkChr->GetRotation(), fMeetTime * yourSpeed, &fYourMoveEstimateX, &fYourMoveEstimateY);

				x += (long)fYourMoveEstimateX;
				y += (long)fYourMoveEstimateY;

				float fDistNew = sqrt(((double)x - GetX()) * (x - GetX()) + ((double)y - GetY()) * (y - GetY()));
				if (fDist < fDistNew)
				{
					x = (long)(GetX() + (x - GetX()) * fDist / fDistNew);
					y = (long)(GetY() + (y - GetY()) * fDist / fDistNew);
				}
			}
		}
	}

	SetRotationToXY(x, y);

	float fDist = DISTANCE_SQRT(x - GetX(), y - GetY());

	if (fDist <= fMinDistance)
		return false;

	float fx, fy;

	if (IsChangeAttackPosition(pkChr) && GetMobRank() < MOB_RANK_BOSS)
	{
		SetChangeAttackPositionTime();

		int retry = 16;
		int dx, dy;
		int rot = (int)GetDegreeFromPositionXY(x, y, GetX(), GetY());

		while (--retry)
		{
			if (fDist < 500.0f)
				GetDeltaByDegree((rot + number(-90, 90) + number(-90, 90)) % 360, fMinDistance, &fx, &fy);
			else
				GetDeltaByDegree(number(0, 359), fMinDistance, &fx, &fy);

			dx = x + (int)fx;
			dy = y + (int)fy;

			LPSECTREE tree = SECTREE_MANAGER::instance().Get(GetMapIndex(), dx, dy);

			if (NULL == tree)
				break;

			if (0 == (tree->GetAttribute(dx, dy) & (ATTR_BLOCK | ATTR_OBJECT)))
				break;
		}

		if (!Goto(dx, dy))
			return false;
	}
	else
	{
		float fDistToGo = fDist - fMinDistance;
		GetDeltaByDegree(GetRotation(), fDistToGo, &fx, &fy);

		if (!Goto(GetX() + (int)fx, GetY() + (int)fy))
			return false;
	}

	SendMovePacket(FUNC_WAIT, 0, 0, 0, 0);
	return true;
}

float CHARACTER::GetDistanceFromSafeboxOpen() const
{
	return DISTANCE_APPROX(GetX() - m_posSafeboxOpen.x, GetY() - m_posSafeboxOpen.y);
}

void CHARACTER::SetSafeboxOpenPosition()
{
	m_posSafeboxOpen = GetXYZ();
}

CSafebox* CHARACTER::GetSafebox() const
{
	return m_pkSafebox;
}

void CHARACTER::ReqSafeboxLoad(const char* pszPassword)
{
	if (!*pszPassword || strlen(pszPassword) > SAFEBOX_PASSWORD_MAX_LEN)
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<Ã¢°í> Àß¸øµÈ ¾ÏÈ£¸¦ ÀÔ·ÂÇÏ¼Ì½À´Ï´Ù."));
		return;
	}
	else if (m_pkSafebox)
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<Ã¢°í> Ã¢°í°¡ ÀÌ¹Ì ¿­·ÁÀÖ½À´Ï´Ù."));
		return;
	}

	int iPulse = thecore_pulse();

	if (iPulse - GetSafeboxLoadTime() < PASSES_PER_SEC(10))
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<Ã¢°í> Ã¢°í¸¦ ´ÝÀºÁö 10ÃÊ ¾È¿¡´Â ¿­ ¼ö ¾ø½À´Ï´Ù."));
		return;
	}
	else if (GetDistanceFromSafeboxOpen() > 1000 && !g_bSafeboxWithoutDistance)
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<A¡Ë¡Æi> ¡ÆA¢¬¢ç¡Æ¢® ¢¬O¨úi¨ù¡© A¡Ë¡Æi¢¬| ¢¯¡© ¨ùo ¨ú©ª¨öA¢¥I¢¥U."));
		return;
	}
	else if (m_bOpeningSafebox)
	{
		sys_log(0, "Overlapped safebox load request from %s", GetName());
		return;
	}

	SetSafeboxLoadTime();
	m_bOpeningSafebox = true;

	TSafeboxLoadPacket p;
	p.dwID = GetDesc()->GetAccountTable().id;
	strlcpy(p.szLogin, GetDesc()->GetAccountTable().login, sizeof(p.szLogin));
	strlcpy(p.szPassword, pszPassword, sizeof(p.szPassword));

	db_clientdesc->DBPacket(HEADER_GD_SAFEBOX_LOAD, GetDesc()->GetHandle(), &p, sizeof(p));
}

#ifdef __GOLD_LIMIT_REWORK__
void CHARACTER::LoadSafebox(int iSize, long long llGold, int iItemCount, TPlayerItem * pItems)
#else
void CHARACTER::LoadSafebox(int iSize, DWORD dwGold, int iItemCount, TPlayerItem * pItems)
#endif
{
	bool bLoaded = false;

	//PREVENT_TRADE_WINDOW
	SetOpenSafebox(true);
	//END_PREVENT_TRADE_WINDOW

	if (m_pkSafebox)
		bLoaded = true;

	if (!m_pkSafebox)
#ifdef __GOLD_LIMIT_REWORK__
		m_pkSafebox = M2_NEW CSafebox(this, iSize, llGold);
#else
		m_pkSafebox = M2_NEW CSafebox(this, iSize, dwGold);
#endif
	else
		m_pkSafebox->ChangeSize(iSize);

	m_iSafeboxSize = iSize;

	TPacketCGSafeboxSize p;

	p.bHeader = HEADER_GC_SAFEBOX_SIZE;
	p.bSize = iSize;

	GetDesc()->Packet(&p, sizeof(TPacketCGSafeboxSize));

	if (!bLoaded)
	{
		for (int i = 0; i < iItemCount; ++i, ++pItems)
		{
			if (!m_pkSafebox->IsValidPosition(pItems->pos))
				continue;

			LPITEM item = ITEM_MANAGER::instance().CreateItem(pItems->vnum, pItems->count, pItems->id);

			if (!item)
			{
				sys_err("cannot create item vnum %d id %u (name: %s)", pItems->vnum, pItems->id, GetName());
				continue;
			}

			item->SetSkipSave(true);
			item->SetSockets(pItems->alSockets);
			item->SetAttributes(pItems->aAttr);
#ifdef __ITEM_EVOLUTION__
			item->SetEvolution(pItems->evolution);
#endif
#ifdef __ITEM_CHANGELOOK__
			item->SetTransmutation(pItems->transmutation);
#endif
			if (!m_pkSafebox->Add(pItems->pos, item))
			{
				M2_DESTROY_ITEM(item);
			}
			else
				item->SetSkipSave(false);
		}
	}
}

void CHARACTER::ChangeSafeboxSize(BYTE bSize)
{
	//if (!m_pkSafebox)
	//return;

	TPacketCGSafeboxSize p;

	p.bHeader = HEADER_GC_SAFEBOX_SIZE;
	p.bSize = bSize;

	GetDesc()->Packet(&p, sizeof(TPacketCGSafeboxSize));

	if (m_pkSafebox)
		m_pkSafebox->ChangeSize(bSize);

	m_iSafeboxSize = bSize;
}

void CHARACTER::CloseSafebox()
{
	if (!m_pkSafebox)
		return;

	//PREVENT_TRADE_WINDOW
	SetOpenSafebox(false);
	//END_PREVENT_TRADE_WINDOW

	m_pkSafebox->Save();

	M2_DELETE(m_pkSafebox);
	m_pkSafebox = NULL;

	ChatPacket(CHAT_TYPE_COMMAND, "CloseSafebox");

	SetSafeboxLoadTime();
	m_bOpeningSafebox = false;

	Save();
}

CSafebox* CHARACTER::GetMall() const
{
	return m_pkMall;
}

void CHARACTER::LoadMall(int iItemCount, TPlayerItem * pItems)
{
	bool bLoaded = false;

	if (m_pkMall)
		bLoaded = true;

	if (!m_pkMall)
		m_pkMall = M2_NEW CSafebox(this, 3 * SAFEBOX_PAGE_SIZE, 0);
	else
		m_pkMall->ChangeSize(3 * SAFEBOX_PAGE_SIZE);

	m_pkMall->SetWindowMode(MALL);

	TPacketCGSafeboxSize p;

	p.bHeader = HEADER_GC_MALL_OPEN;
	p.bSize = 3 * SAFEBOX_PAGE_SIZE;

	GetDesc()->Packet(&p, sizeof(TPacketCGSafeboxSize));

	if (!bLoaded)
	{
#ifdef __SAFEBOX_AUTO_SORT__
		if (IsGM())
			ChatPacket(1, "<GM|MALL> gelenItemSayisi: %d", iItemCount);
		CGrid iGrid = CGrid(5, 9);
#endif
		for (int i = 0; i < iItemCount; ++i, ++pItems)
		{
			if (!m_pkMall->IsValidPosition(pItems->pos))
				continue;

			LPITEM item = ITEM_MANAGER::instance().CreateItem(pItems->vnum, pItems->count, pItems->id);

			if (!item)
			{
				sys_err("cannot create item vnum %d id %u (name: %s)", pItems->vnum, pItems->id, GetName());
				continue;
			}

			item->SetSkipSave(true);
			item->SetSockets(pItems->alSockets);
			item->SetAttributes(pItems->aAttr);
#ifdef __ITEM_EVOLUTION__
			item->SetEvolution(pItems->evolution);
#endif
#ifdef __ITEM_CHANGELOOK__
			item->SetTransmutation(pItems->transmutation);
#endif
#ifdef __SAFEBOX_AUTO_SORT__
			int iPos = -1;
			iPos = iGrid.FindBlank(1, item->GetSize());
			if (iPos >= 0)
			{
				iGrid.Put(iPos, 1, item->GetSize());
				if (!m_pkMall->Add(iPos, item))
					M2_DESTROY_ITEM(item);
				else
					item->SetSkipSave(false);
			}
			else
				M2_DESTROY_ITEM(item);
		}
#else
			if (!m_pkMall->Add(pItems->pos, item))
				M2_DESTROY_ITEM(item);
			else
				item->SetSkipSave(false);
	}
#endif
}
}

void CHARACTER::CloseMall()
{
	if (!m_pkMall)
		return;

	m_pkMall->Save();

	M2_DELETE(m_pkMall);
	m_pkMall = NULL;

	ChatPacket(CHAT_TYPE_COMMAND, "CloseMall");
}

bool CHARACTER::BuildUpdatePartyPacket(TPacketGCPartyUpdate& out)
{
	if (!GetParty())
		return false;

	memset(&out, 0, sizeof(out));

	out.header = HEADER_GC_PARTY_UPDATE;
	out.pid = GetPlayerID();
	out.percent_hp = GetHPPct();
	out.role = GetParty()->GetRole(GetPlayerID());

	sys_log(1, "PARTY %s role is %d", GetName(), out.role);

	LPCHARACTER l = GetParty()->GetLeaderCharacter();

	if (l && DISTANCE_APPROX(GetX() - l->GetX(), GetY() - l->GetY()) < PARTY_DEFAULT_RANGE)
	{
		out.affects[0] = GetParty()->GetPartyBonusExpPercent();
		out.affects[1] = GetPoint(POINT_PARTY_ATTACKER_BONUS);
		out.affects[2] = GetPoint(POINT_PARTY_TANKER_BONUS);
		out.affects[3] = GetPoint(POINT_PARTY_BUFFER_BONUS);
		out.affects[4] = GetPoint(POINT_PARTY_SKILL_MASTER_BONUS);
		out.affects[5] = GetPoint(POINT_PARTY_HASTE_BONUS);
		out.affects[6] = GetPoint(POINT_PARTY_DEFENDER_BONUS);
#ifdef __PARTY_ROLE_REWORK__
		out.affects[7] = GetPoint(POINT_PARTY_ATTACKER_MONSTER_BONUS);
		out.affects[8] = GetPoint(POINT_PARTY_ATTACKER_STONE_BONUS);
		out.affects[9] = GetPoint(POINT_PARTY_ATTACKER_BOSS_BONUS);
#endif
	}

	return true;
}

int CHARACTER::GetLeadershipSkillLevel() const
{
	return GetSkillLevel(SKILL_LEADERSHIP);
}

void CHARACTER::QuerySafeboxSize()
{
	if (m_iSafeboxSize == -1)
	{
		DBManager::instance().ReturnQuery(QID_SAFEBOX_SIZE,
			GetPlayerID(),
			NULL,
			"SELECT size FROM safebox%s WHERE account_id = %u",
			get_table_postfix(),
			GetDesc()->GetAccountTable().id);
	}
}

void CHARACTER::SetSafeboxSize(int iSize)
{
	sys_log(1, "SetSafeboxSize: %s %d", GetName(), iSize);
	m_iSafeboxSize = iSize;
	DBManager::instance().Query("UPDATE safebox%s SET size = %d WHERE account_id = %u", get_table_postfix(), iSize / SAFEBOX_PAGE_SIZE, GetDesc()->GetAccountTable().id);
}

int CHARACTER::GetSafeboxSize() const
{
	return m_iSafeboxSize;
}

void CHARACTER::SetNowWalking(bool bWalkFlag)
{
	if (m_bNowWalking != bWalkFlag)
	{
		if (bWalkFlag)
		{
			m_bNowWalking = true;
			m_dwWalkStartTime = get_dword_time();
		}
		else
		{
			m_bNowWalking = false;
		}

		//if (m_bNowWalking)
		{
			TPacketGCWalkMode p;
			p.vid = GetVID();
			p.header = HEADER_GC_WALK_MODE;
			p.mode = m_bNowWalking ? WALKMODE_WALK : WALKMODE_RUN;

			PacketView(&p, sizeof(p));
		}

		//sys_log(0, "%s is now %s", GetName(), m_bNowWalking?"walking.":"running.");
	}
}

void CHARACTER::StartStaminaConsume()
{
	if (m_bStaminaConsume)
		return;
	PointChange(POINT_STAMINA, 0);
	m_bStaminaConsume = true;
	//ChatPacket(CHAT_TYPE_COMMAND, "StartStaminaConsume %d %d", STAMINA_PER_STEP * passes_per_sec, GetStamina());
	if (IsStaminaHalfConsume())
		ChatPacket(CHAT_TYPE_COMMAND, "StartStaminaConsume %d %d", STAMINA_PER_STEP * passes_per_sec / 2, GetStamina());
	else
		ChatPacket(CHAT_TYPE_COMMAND, "StartStaminaConsume %d %d", STAMINA_PER_STEP * passes_per_sec, GetStamina());
}

void CHARACTER::StopStaminaConsume()
{
	if (!m_bStaminaConsume)
		return;
	PointChange(POINT_STAMINA, 0);
	m_bStaminaConsume = false;
	ChatPacket(CHAT_TYPE_COMMAND, "StopStaminaConsume %d", GetStamina());
}

bool CHARACTER::IsStaminaConsume() const
{
	return m_bStaminaConsume;
}

bool CHARACTER::IsStaminaHalfConsume() const
{
	return IsEquipUniqueItem(UNIQUE_ITEM_HALF_STAMINA);
}

void CHARACTER::ResetStopTime()
{
	m_dwStopTime = get_dword_time();
}

DWORD CHARACTER::GetStopTime() const
{
	return m_dwStopTime;
}

void CHARACTER::ResetPoint(int iLv)
{
	BYTE bJob = GetJob();

	PointChange(POINT_LEVEL, iLv - GetLevel());

	SetRealPoint(POINT_ST, JobInitialPoints[bJob].st);
	SetPoint(POINT_ST, GetRealPoint(POINT_ST));

	SetRealPoint(POINT_HT, JobInitialPoints[bJob].ht);
	SetPoint(POINT_HT, GetRealPoint(POINT_HT));

	SetRealPoint(POINT_DX, JobInitialPoints[bJob].dx);
	SetPoint(POINT_DX, GetRealPoint(POINT_DX));

	SetRealPoint(POINT_IQ, JobInitialPoints[bJob].iq);
	SetPoint(POINT_IQ, GetRealPoint(POINT_IQ));

	SetRandomHP((iLv - 1) * number(JobInitialPoints[GetJob()].hp_per_lv_begin, JobInitialPoints[GetJob()].hp_per_lv_end));
	SetRandomSP((iLv - 1) * number(JobInitialPoints[GetJob()].sp_per_lv_begin, JobInitialPoints[GetJob()].sp_per_lv_end));

	// @duzenleme
	// karakter statu sifirladiginde mevcut seviye siniri kadar statusu sifirlanir fakat eger seviye siniri degisirse sikinti yaratabilir.
	PointChange(POINT_STAT, (MINMAX(1, iLv, g_iStatusPointGetLevelLimit) * 3) + GetPoint(POINT_LEVEL_STEP) - GetPoint(POINT_STAT));

	ComputePoints();

	PointChange(POINT_HP, GetMaxHP() - GetHP());
	PointChange(POINT_SP, GetMaxSP() - GetSP());

	PointsPacket();
}

bool CHARACTER::IsChangeAttackPosition(LPCHARACTER target) const
{
	if (!IsNPC())
		return true;

	DWORD dwChangeTime = AI_CHANGE_ATTACK_POISITION_TIME_NEAR;

	if (DISTANCE_APPROX(GetX() - target->GetX(), GetY() - target->GetY()) >
		AI_CHANGE_ATTACK_POISITION_DISTANCE + GetMobAttackRange())
		dwChangeTime = AI_CHANGE_ATTACK_POISITION_TIME_FAR;

	return get_dword_time() - m_dwLastChangeAttackPositionTime > dwChangeTime;
}

void CHARACTER::GiveRandomSkillBook()
{
	LPITEM item = AutoGiveItem(50300);

	if (NULL != item)
	{
		extern const DWORD GetRandomSkillVnum(BYTE bJob = JOB_MAX_NUM);
		DWORD dwSkillVnum = 0;
		// 50% of getting random books or getting one of the same player's race
		if (!number(0, 1))
			dwSkillVnum = GetRandomSkillVnum(GetJob());
		else
			dwSkillVnum = GetRandomSkillVnum();
		item->SetSocket(0, dwSkillVnum);
	}
}

void CHARACTER::ReviveInvisible(int iDur)
{
	AddAffect(AFFECT_REVIVE_INVISIBLE, POINT_NONE, 0, AFF_REVIVE_INVISIBLE, iDur, 0, true);
}

void CHARACTER::SetGuild(CGuild* pGuild)
{
	if (m_pGuild != pGuild)
	{
		m_pGuild = pGuild;
		UpdatePacket();
	}
}

void CHARACTER::BeginStateEmpty()
{
}

void CHARACTER::EffectPacket(int enumEffectType)
{
	TPacketGCSpecialEffect p;

	p.header = HEADER_GC_SEPCIAL_EFFECT;
	p.type = enumEffectType;
	p.vid = GetVID();

	PacketAround(&p, sizeof(TPacketGCSpecialEffect));
}

void CHARACTER::SpecificEffectPacket(const std::string& fileName)
{
	TPacketGCSpecificEffect p;

	p.header = HEADER_GC_SPECIFIC_EFFECT;
	p.vid = GetVID();
	strlcpy(p.effect_file, fileName.c_str(), MAX_EFFECT_FILE_NAME);

	PacketAround(&p, sizeof(TPacketGCSpecificEffect));
}

void CHARACTER::MonsterChat(BYTE bMonsterChatType)
{
	if (IsPC())
		return;

	char sbuf[CHAT_MAX_LEN + 1]; // @duzenleme canavarlarin chat limiti farkli girilmisti ayni olsun diye ekledik.

	if (IsMonster())
	{
		if (number(0, 60))
			return;

		snprintf(sbuf, sizeof(sbuf),
			"(locale.monster_chat[%i] and locale.monster_chat[%i][%d] or '')",
			GetRaceNum(), GetRaceNum(), bMonsterChatType * 3 + number(1, 3));
	}
	else
	{
		if (bMonsterChatType != MONSTER_CHAT_WAIT)
			return;

		if (IsGuardNPC())
		{
			if (number(0, 6))
				return;
		}
		else
		{
			if (number(0, 30))
				return;
		}

		snprintf(sbuf, sizeof(sbuf), "(locale.monster_chat[%i] and locale.monster_chat[%i][number(1, table.getn(locale.monster_chat[%i]))] or '')", GetRaceNum(), GetRaceNum(), GetRaceNum());
	}

	std::string text = quest::ScriptToString(sbuf);

	if (text.empty())
		return;

	struct packet_chat pack_chat;

	pack_chat.header = HEADER_GC_CHAT;
	pack_chat.size = sizeof(struct packet_chat) + text.size() + 1;
	pack_chat.type = CHAT_TYPE_TALKING;
	pack_chat.id = GetVID();
	pack_chat.bEmpire = 0;

	TEMP_BUFFER buf;
	buf.write(&pack_chat, sizeof(struct packet_chat));
	buf.write(text.c_str(), text.size() + 1);

	PacketAround(buf.read_peek(), buf.size());
}

void CHARACTER::SetQuestNPCID(DWORD vid)
{
	// @bilinmeyenduzenleme nereden alindigini ne ise yaradigini bilmedigim fix.
	if (vid && m_dwQuestNPCVID)
	{
		quest::PC* pPC = quest::CQuestManager::instance().GetPCForce(GetPlayerID());
		if (pPC && pPC->IsRunning())
		{
			//sys_err("cannot reset quest npc id - already running quest [%u %s]", GetPlayerID(), GetName());
			return;
		}
	}

	m_dwQuestNPCVID = vid;
}

LPCHARACTER CHARACTER::GetQuestNPC() const
{
	return CHARACTER_MANAGER::instance().Find(m_dwQuestNPCVID);
}

void CHARACTER::SetQuestItemPtr(LPITEM item)
{
	m_pQuestItem = item;
}

void CHARACTER::ClearQuestItemPtr()
{
	m_pQuestItem = NULL;
}

LPITEM CHARACTER::GetQuestItemPtr() const
{
	return m_pQuestItem;
}

LPDUNGEON CHARACTER::GetDungeonForce() const
{
	if (m_lWarpMapIndex > 10000)
		return CDungeonManager::instance().FindByMapIndex(m_lWarpMapIndex);

	return m_pkDungeon;
}

void CHARACTER::SetBlockMode(int bFlag)
{
	m_pointsInstant.bBlockMode = bFlag;

	ChatPacket(CHAT_TYPE_COMMAND, "setblockmode %d", m_pointsInstant.bBlockMode);

	SetQuestFlag("game_option.block_exchange", bFlag & BLOCK_EXCHANGE ? 1 : 0);
	SetQuestFlag("game_option.block_party_invite", bFlag & BLOCK_PARTY_INVITE ? 1 : 0);
	SetQuestFlag("game_option.block_guild_invite", bFlag & BLOCK_GUILD_INVITE ? 1 : 0);
	SetQuestFlag("game_option.block_whisper", bFlag & BLOCK_WHISPER ? 1 : 0);
	SetQuestFlag("game_option.block_messenger_invite", bFlag & BLOCK_MESSENGER_INVITE ? 1 : 0);
	SetQuestFlag("game_option.block_party_request", bFlag & BLOCK_PARTY_REQUEST ? 1 : 0);
	SetQuestFlag("game_option.block_point_exp", bFlag & BLOCK_POINT_EXP ? 1 : 0);
	SetQuestFlag("game_option.block_costume_body", bFlag & BLOCK_COSTUME_BODY ? 1 : 0);
	SetQuestFlag("game_option.block_costume_hair", bFlag & BLOCK_COSTUME_HAIR ? 1 : 0);
	SetQuestFlag("game_option.block_costume_weapon", bFlag & BLOCK_COSTUME_WEAPON ? 1 : 0);
	SetQuestFlag("game_option.block_costume_acce", bFlag & BLOCK_COSTUME_ACCE ? 1 : 0);
	SetQuestFlag("game_option.block_costume_acce_ex", bFlag & BLOCK_COSTUME_ACCE_EX ? 1 : 0);
	SetQuestFlag("game_option.block_costume_aura", bFlag & BLOCK_COSTUME_AURA ? 1 : 0);
}

void CHARACTER::SetBlockModeForce(int bFlag)
{
	m_pointsInstant.bBlockMode = bFlag;
	ChatPacket(CHAT_TYPE_COMMAND, "setblockmode %d", m_pointsInstant.bBlockMode);
}

bool CHARACTER::IsGuardNPC() const
{
	return IsNPC() && (GetRaceNum() == 11000 || GetRaceNum() == 11002 || GetRaceNum() == 11004);
}

int CHARACTER::GetQuestFlag(const std::string& flag) const
{
	// @duzenleme eger karsi taraf pc degilse diye kontrol.
	if (!IsPC())
	{
		sys_err("Trying to get qf %s from non player character", flag.c_str());
		return 0;
	}
	DWORD pid = GetPlayerID();
	// @duzenleme null iken devam etmemesi icin kontrol.
	quest::CQuestManager& q = quest::CQuestManager::instance();
	quest::PC* pPC = q.GetPC(GetPlayerID());

	if (!pPC)
	{
		sys_err("Nullpointer when trying to access questflag %s for player with pid %u", flag.c_str(), pid);
		return 0;
	}
	return pPC->GetFlag(flag);
}

void CHARACTER::SetQuestFlag(const std::string& flag, int value)
{
	// @duzenleme questflag kontrolu yine fakat yukarda ISPC var burada yok bunlara dikkat etmek lazim.
	DWORD pid = GetPlayerID();
	quest::CQuestManager& q = quest::CQuestManager::instance();
	quest::PC* pPC = q.GetPC(GetPlayerID());

	// @duzenleme null iken devam etmemesi icin kontrol.
	if (!pPC)
	{
		sys_err("Nullpointer when trying to set questflag %s for player with pid %u", flag.c_str(), pid);
		return;
	}

	pPC->SetFlag(flag, value);
}

void CHARACTER::DetermineDropMetinStone()
{
	if (g_NoDropMetinStone)
	{
		m_dwDropMetinStone = 0;
		return;
	}

	static const DWORD c_adwMetin[] =
	{
#if defined(__WOLFMAN_CHARACTER__) && defined(USE_WOLFMAN_STONES)
		28012,
#endif
		28030,
		28031,
		28032,
		28033,
		28034,
		28035,
		28036,
		28037,
		28038,
		28039,
		28040,
		28041,
		28042,
		28043,
#if defined(__MAGIC_REDUCTION_SYSTEM__) && defined(USE_MAGIC_REDUCTION_STONES)
		28044,
		28045,
#endif
	};
	DWORD stone_num = GetRaceNum();
	int idx = std::lower_bound(aStoneDrop, aStoneDrop + STONE_INFO_MAX_NUM, stone_num) - aStoneDrop;
	if (idx >= STONE_INFO_MAX_NUM || aStoneDrop[idx].dwMobVnum != stone_num)
	{
		m_dwDropMetinStone = 0;
	}
	else
	{
		const SStoneDropInfo& info = aStoneDrop[idx];
		m_bDropMetinStonePct = info.iDropPct;
		{
			m_dwDropMetinStone = c_adwMetin[number(0, sizeof(c_adwMetin) / sizeof(DWORD) - 1)];
			int iGradePct = number(1, 100);
			for (int iStoneLevel = 0; iStoneLevel < STONE_LEVEL_MAX_NUM; iStoneLevel++)
			{
				int iLevelGradePortion = info.iLevelPct[iStoneLevel];
				if (iGradePct <= iLevelGradePortion)
				{
					break;
				}
				else
				{
					iGradePct -= iLevelGradePortion;
					m_dwDropMetinStone += 100;
				}
			}
		}
	}
}

bool CHARACTER::CanSummon(int iLeaderShip)
{
	return ((iLeaderShip >= 20) || ((iLeaderShip >= 12) && ((m_dwLastDeadTime + 180) > get_dword_time())));
}

void CHARACTER::MountVnum(DWORD vnum)
{
	if (m_dwMountVnum == vnum)
		return;
	if ((m_dwMountVnum != 0) && (vnum != 0)) // @duzenleme MartySama adli arkadasin yaptigi binek fixi.
		MountVnum(0);
	m_dwMountVnum = vnum;
	m_dwMountTime = get_dword_time();

	if (m_bIsObserver)
		return;

	m_posDest.x = m_posStart.x = GetX();
	m_posDest.y = m_posStart.y = GetY();
	EncodeInsertPacket(this);

	ENTITY_MAP::iterator it = m_map_view.begin();

	while (it != m_map_view.end())
	{
		LPENTITY entity = (it++)->first;

		EncodeInsertPacket(entity);
	}

	ComputePoints();
}

namespace {
	class FuncCheckWarp
	{
	public:
		FuncCheckWarp(LPCHARACTER pkWarp)
		{
			m_lTargetY = 0;
			m_lTargetX = 0;

			m_lX = pkWarp->GetX();
			m_lY = pkWarp->GetY();

			m_bInvalid = false;
			m_bEmpire = pkWarp->GetEmpire();

			char szTmp[64];

			if (3 != sscanf(pkWarp->GetName(), " %s %ld %ld ", szTmp, &m_lTargetX, &m_lTargetY))
			{
				if (number(1, 100) < 5)
					sys_err("Warp NPC name wrong : vnum(%d) name(%s)", pkWarp->GetRaceNum(), pkWarp->GetName());

				m_bInvalid = true;

				return;
			}

			m_lTargetX *= 100;
			m_lTargetY *= 100;

			m_bUseWarp = true;

			if (pkWarp->IsGoto())
			{
				LPSECTREE_MAP pkSectreeMap = SECTREE_MANAGER::instance().GetMap(pkWarp->GetMapIndex());
				m_lTargetX += pkSectreeMap->m_setting.iBaseX;
				m_lTargetY += pkSectreeMap->m_setting.iBaseY;
				m_bUseWarp = false;
			}
		}

		bool Valid()
		{
			return !m_bInvalid;
		}

		void operator () (LPENTITY ent)
		{
			if (!Valid())
				return;

			if (!ent->IsType(ENTITY_CHARACTER))
				return;

			LPCHARACTER pkChr = (LPCHARACTER)ent;

			if (!pkChr->IsPC())
				return;

			int iDist = DISTANCE_APPROX(pkChr->GetX() - m_lX, pkChr->GetY() - m_lY);

			if (iDist > 300)
				return;

			if (m_bEmpire && pkChr->GetEmpire() && m_bEmpire != pkChr->GetEmpire())
				return;

			if (pkChr->IsHack())
				return;

			if (!pkChr->CanHandleItem(false, true))
				return;

			if (m_bUseWarp)
				pkChr->WarpSet(m_lTargetX, m_lTargetY);
			else
			{
				pkChr->Show(pkChr->GetMapIndex(), m_lTargetX, m_lTargetY);
				pkChr->Stop();
			}
		}

		bool m_bInvalid;
		bool m_bUseWarp;

		long m_lX;
		long m_lY;
		long m_lTargetX;
		long m_lTargetY;

		BYTE m_bEmpire;
	};
}

EVENTFUNC(warp_npc_event)
{
	char_event_info* info = dynamic_cast<char_event_info*>(event->info);
	if (info == NULL)
	{
		sys_err("warp_npc_event> <Factor> Null pointer");
		return 0;
	}

	LPCHARACTER	ch = info->ch;

	if (ch == NULL) { // <Factor>
		return 0;
	}

	if (!ch->GetSectree())
	{
		ch->m_pkWarpNPCEvent = NULL;
		return 0;
	}

	FuncCheckWarp f(ch);
	if (f.Valid())
		ch->GetSectree()->ForEachAround(f);

	return passes_per_sec / 2;
}

void CHARACTER::StartWarpNPCEvent()
{
	if (m_pkWarpNPCEvent)
		return;

	if (!IsWarp() && !IsGoto())
		return;

	char_event_info* info = AllocEventInfo<char_event_info>();

	info->ch = this;

	m_pkWarpNPCEvent = event_create(warp_npc_event, info, passes_per_sec / 2);
}

void CHARACTER::SyncPacket()
{
	TEMP_BUFFER buf;

	TPacketCGSyncPositionElement elem;

	elem.dwVID = GetVID();
	elem.lX = GetX();
	elem.lY = GetY();

	TPacketGCSyncPosition pack;

	pack.bHeader = HEADER_GC_SYNC_POSITION;
	pack.wSize = sizeof(TPacketGCSyncPosition) + sizeof(elem);

	buf.write(&pack, sizeof(pack));
	buf.write(&elem, sizeof(elem));

	PacketAround(buf.read_peek(), buf.size());
}

void CHARACTER::ConfirmWithMsg(const char* szMsg, int iTimeout, DWORD dwRequestPID)
{
	if (!IsPC())
		return;

	TPacketGCQuestConfirm p;

	p.header = HEADER_GC_QUEST_CONFIRM;
	p.requestPID = dwRequestPID;
	p.timeout = iTimeout;
	strlcpy(p.msg, szMsg, sizeof(p.msg));

	GetDesc()->Packet(&p, sizeof(p));
}

int CHARACTER::GetPremiumRemainSeconds(BYTE bType) const
{
	if (bType >= PREMIUM_MAX_NUM)
		return 0;

	return m_aiPremiumTimes[bType] - get_global_time();
}

bool CHARACTER::WarpToPID(DWORD dwPID)
{
	LPCHARACTER victim;
	if ((victim = (CHARACTER_MANAGER::instance().FindByPID(dwPID))))
	{
		int mapIdx = victim->GetMapIndex();
		if (CAN_ENTER_ZONE(this, mapIdx))
		{
			WarpSet(victim->GetX(), victim->GetY(), victim->GetMapIndex());
		}
		else
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("»ó´ë¹æÀÌ ÀÖ´Â °÷À¸·Î ¿öÇÁÇÒ ¼ö ¾ø½À´Ï´Ù."));
			return false;
		}
	}
	else
	{
		CCI* pcci = P2P_MANAGER::instance().FindByPID(dwPID);

		if (!pcci)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("»ó´ë¹æÀÌ ¿Â¶óÀÎ »óÅÂ°¡ ¾Æ´Õ´Ï´Ù."));
			return false;
		}
#ifndef __WARP_WITH_CHANNEL__
		if (pcci->bChannel != g_bChannel)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("»ó´ë¹æÀÌ %d Ã¤³Î¿¡ ÀÖ½À´Ï´Ù. (ÇöÀç Ã¤³Î %d)"), pcci->bChannel, g_bChannel);
			return false;
		}
#endif
		else if (!CAN_ENTER_ZONE(this, pcci->lMapIndex))
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("»ó´ë¹æÀÌ ÀÖ´Â °÷À¸·Î ¿öÇÁÇÒ ¼ö ¾ø½À´Ï´Ù."));
			return false;
		}
		else
		{
			TPacketGGFindPosition p;
			p.header = HEADER_GG_FIND_POSITION;
			p.dwFromPID = GetPlayerID();
			p.dwTargetPID = dwPID;
			pcci->pkDesc->Packet(&p, sizeof(TPacketGGFindPosition));

			if (test_server)
				ChatPacket(CHAT_TYPE_PARTY, "sent find position packet for teleport");
		}
	}
	return true;
}

// @duzenleme uzaktan arti basmamalari icin boyle bir engel aldik (Rubinum Metin2AR)
LPCHARACTER CHARACTER::GetRefineNPC() const
{
	if (!m_dwRefineNPCVID)
		return nullptr;

	return CHARACTER_MANAGER::instance().Find(m_dwRefineNPCVID);
}

#ifdef __GOLD_LIMIT_REWORK__
long long CHARACTER::ComputeRefineFee(long long llCost, int iMultiply) const
{
	return llCost;
}
#else
int CHARACTER::ComputeRefineFee(int iCost, int iMultiply) const
{
	return iCost;
}
#endif

#ifdef __GOLD_LIMIT_REWORK__
void CHARACTER::PayRefineFee(long long llTotalMoney)
{
	PointChange(POINT_GOLD, -llTotalMoney);
}
#else
void CHARACTER::PayRefineFee(int iTotalMoney)
{
	PointChange(POINT_GOLD, -iTotalMoney);
}
#endif
//

bool CHARACTER::IsHack(bool bSendMsg, bool bCheckShopOwner, int limittime)
{
	const int iPulse = thecore_pulse();

	if (test_server)
		bSendMsg = true;

	if (iPulse - GetSafeboxLoadTime() < PASSES_PER_SEC(limittime))
	{
		if (bSendMsg)
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Ã¢°í¸¦ ¿¬ÈÄ %dÃÊ ÀÌ³»¿¡´Â ´Ù¸¥°÷À¸·Î ÀÌµ¿ÇÒ¼ö ¾ø½À´Ï´Ù."), limittime);

		if (test_server)
			ChatPacket(CHAT_TYPE_INFO, "[TestOnly]Pulse %d LoadTime %d PASS %d", iPulse, GetSafeboxLoadTime(), PASSES_PER_SEC(limittime));
		return true;
	}

	if (bCheckShopOwner)
	{
		if (GetExchange() || GetMyShop() || GetShopOwner() || IsOpenSafebox() || IsCubeOpen() || IsUnderRefine())
		{
			if (bSendMsg)
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("°Å·¡Ã¢,Ã¢°í µîÀ» ¿¬ »óÅÂ¿¡¼­´Â ´Ù¸¥°÷À¸·Î ÀÌµ¿,Á¾·á ÇÒ¼ö ¾ø½À´Ï´Ù"));

			return true;
		}
#ifdef __ACCE_SYSTEM__
		if (isAcceOpened(true) || isAcceOpened(false))
		{
			if (bSendMsg)
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("°Å·¡Ã¢,Ã¢°í µîÀ» ¿¬ »óÅÂ¿¡¼­´Â ´Ù¸¥°÷À¸·Î ÀÌµ¿,Á¾·á ÇÒ¼ö ¾ø½À´Ï´Ù"));

			return true;
		}
#endif
#ifdef __ITEM_CHANGELOOK__
		if (isChangeLookOpened())
		{
			if (bSendMsg)
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("PLEASE_BEFORE_CLOSE_WINDOW_AND_USE_THIS_FUNCTION"));

			return true;
		}
#endif

#ifdef __OFFLINE_SHOP__
		if (GetOfflineShopGuest() != NULL || GetShopSafebox())
		{
			if (bSendMsg)
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("PLEASE_BEFORE_CLOSE_WINDOW_AND_USE_THIS_FUNCTION"));

			return true;
		}
#endif
	}
	else
	{
		if (GetExchange() || GetMyShop() || IsOpenSafebox() || IsCubeOpen() || IsUnderRefine())
		{
			if (bSendMsg)
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("°Å·¡Ã¢,Ã¢°í µîÀ» ¿¬ »óÅÂ¿¡¼­´Â ´Ù¸¥°÷À¸·Î ÀÌµ¿,Á¾·á ÇÒ¼ö ¾ø½À´Ï´Ù"));

			return true;
		}
#ifdef __ACCE_SYSTEM__
		if (isAcceOpened(true) || isAcceOpened(false))
		{
			if (bSendMsg)
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("PLEASE_BEFORE_CLOSE_WINDOW_AND_USE_THIS_FUNCTION"));

			return true;
		}
#endif
#ifdef __ITEM_CHANGELOOK__
		if (isChangeLookOpened())
		{
			if (bSendMsg)
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("PLEASE_BEFORE_CLOSE_WINDOW_AND_USE_THIS_FUNCTION"));

			return true;
		}
#endif
#ifdef __OFFLINE_SHOP__
		if (GetOfflineShopGuest() != NULL || GetShopSafebox())
		{
			if (bSendMsg)
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("PLEASE_BEFORE_CLOSE_WINDOW_AND_USE_THIS_FUNCTION"));

			return true;
		}
#endif
	}

	//PREVENT_PORTAL_AFTER_EXCHANGE
	if (iPulse - GetExchangeTime() < PASSES_PER_SEC(limittime))
	{
		if (bSendMsg)
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("°Å·¡ ÈÄ %dÃÊ ÀÌ³»¿¡´Â ´Ù¸¥Áö¿ªÀ¸·Î ÀÌµ¿ ÇÒ ¼ö ¾ø½À´Ï´Ù."), limittime);
		return true;
	}
	//END_PREVENT_PORTAL_AFTER_EXCHANGE

	//PREVENT_ITEM_COPY
	if (iPulse - GetMyShopTime() < PASSES_PER_SEC(limittime))
	{
		if (bSendMsg)
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("°Å·¡ ÈÄ %dÃÊ ÀÌ³»¿¡´Â ´Ù¸¥Áö¿ªÀ¸·Î ÀÌµ¿ ÇÒ ¼ö ¾ø½À´Ï´Ù."), limittime);
		return true;
	}

	if (iPulse - GetRefineTime() < PASSES_PER_SEC(limittime))
	{
		if (bSendMsg)
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¾ÆÀÌÅÛ °³·®ÈÄ %dÃÊ ÀÌ³»¿¡´Â ±ÍÈ¯ºÎ,±ÍÈ¯±â¾ïºÎ¸¦ »ç¿ëÇÒ ¼ö ¾ø½À´Ï´Ù."), limittime);
		return true;
	}
	//END_PREVENT_ITEM_COPY

	return false;
}

void CHARACTER::Say(const std::string& s)
{
	struct ::packet_script packet_script;

	packet_script.header = HEADER_GC_SCRIPT;
	packet_script.skin = 1;
	packet_script.src_size = s.size();
	packet_script.size = packet_script.src_size + sizeof(struct packet_script);

	TEMP_BUFFER buf;

	buf.write(&packet_script, sizeof(struct packet_script));
	buf.write(&s[0], s.size());

	if (IsPC())
	{
		GetDesc()->Packet(buf.read_peek(), buf.size());
	}
}

bool CHARACTER::IsSiegeNPC() const
{
	return IsNPC() && (GetRaceNum() == 11000 || GetRaceNum() == 11002 || GetRaceNum() == 11004);
}

void CHARACTER::UpdateDepositPulse()
{
	m_deposit_pulse = thecore_pulse() + PASSES_PER_SEC(60 * 5);
}

bool CHARACTER::CanDeposit() const
{
	return (m_deposit_pulse == 0 || (m_deposit_pulse < thecore_pulse()));
}

ESex GET_SEX(LPCHARACTER ch)
{
	switch (ch->GetRaceNum())
	{
	case MAIN_RACE_WARRIOR_M:
	case MAIN_RACE_SURA_M:
	case MAIN_RACE_ASSASSIN_M:
	case MAIN_RACE_SHAMAN_M:
#ifdef __WOLFMAN_CHARACTER__
	case MAIN_RACE_WOLFMAN_M:
#endif
		return SEX_MALE;

	case MAIN_RACE_ASSASSIN_W:
	case MAIN_RACE_SHAMAN_W:
	case MAIN_RACE_WARRIOR_W:
	case MAIN_RACE_SURA_W:
		return SEX_FEMALE;
	}

	/* default sex = male */
	return SEX_MALE;
}

int CHARACTER::GetHPPct() const
{
	// return (GetMaxHP() == 0 ? 0 : ((long float)GetHP()/GetMaxHP()) * 100);
	return MINMAX(0, (GetMaxHP() == 0 ? 0 : (GetHP() * (1/(long double)(GetMaxHP()/100)))), 100);
}

bool CHARACTER::IsBerserk() const
{
	if (m_pkMobInst != NULL)
		return m_pkMobInst->m_IsBerserk;
	else
		return false;
}

void CHARACTER::SetBerserk(bool mode)
{
	if (m_pkMobInst != NULL)
		m_pkMobInst->m_IsBerserk = mode;
}

bool CHARACTER::IsGodSpeed() const
{
	if (m_pkMobInst != NULL)
	{
		return m_pkMobInst->m_IsGodSpeed;
	}
	else
	{
		return false;
	}
}

void CHARACTER::SetGodSpeed(bool mode)
{
	if (m_pkMobInst != NULL)
	{
		m_pkMobInst->m_IsGodSpeed = mode;

		if (mode == true)
		{
			SetPoint(POINT_ATT_SPEED, 250);
		}
		else
		{
			SetPoint(POINT_ATT_SPEED, m_pkMobData->m_table.sAttackSpeed);
		}
	}
}

bool CHARACTER::IsDeathBlow() const
{
#ifdef __MONSTER_DEATHBLOW_REWORK__
	if (GetHPPct() > 50) //Only deathblow when low on health
		return false;

	return (number(1, 100) <= m_pkMobData->m_table.bDeathBlowPoint);
#else
	if (number(1, 100) <= m_pkMobData->m_table.bDeathBlowPoint)
		return true;
	else
		return false;
#endif
}

struct FFindReviver
{
	FFindReviver()
	{
		pChar = NULL;
		HasReviver = false;
	}

	void operator() (LPCHARACTER ch)
	{
		if (ch->IsMonster() != true)
		{
			return;
		}

		if (ch->IsReviver() == true && pChar != ch && ch->IsDead() != true)
		{
			if (number(1, 100) <= ch->GetMobTable().bRevivePoint)
			{
				HasReviver = true;
				pChar = ch;
			}
		}
	}

	LPCHARACTER pChar;
	bool HasReviver;
};

bool CHARACTER::HasReviverInParty() const
{
	LPPARTY party = GetParty();

	if (party != NULL)
	{
		if (party->GetMemberCount() == 1) return false;

		FFindReviver f;
		party->ForEachMemberPtr(f);
		return f.HasReviver;
	}

	return false;
}

bool CHARACTER::IsRevive() const
{
	if (m_pkMobInst != NULL)
	{
		return m_pkMobInst->m_IsRevive;
	}

	return false;
}

void CHARACTER::SetRevive(bool mode)
{
	if (m_pkMobInst != NULL)
	{
		m_pkMobInst->m_IsRevive = mode;
	}
}

void CHARACTER::GoHome()
{
	WarpSet(EMPIRE_START_X(GetEmpire()), EMPIRE_START_Y(GetEmpire()));
}

void CHARACTER::SendGuildName(CGuild* pGuild)
{
	if (NULL == pGuild) return;

	DESC* desc = GetDesc();

	if (NULL == desc) return;
	if (m_known_guild.find(pGuild->GetID()) != m_known_guild.end()) return;

	m_known_guild.insert(pGuild->GetID());

	TPacketGCGuildName	pack;
	memset(&pack, 0x00, sizeof(pack));

	pack.header = HEADER_GC_GUILD;
	pack.subheader = GUILD_SUBHEADER_GC_GUILD_NAME;
	pack.size = sizeof(TPacketGCGuildName);
	pack.guildID = pGuild->GetID();
	memcpy(pack.guildName, pGuild->GetName(), GUILD_NAME_MAX_LEN);

	desc->Packet(&pack, sizeof(pack));
}

void CHARACTER::SendGuildName(DWORD dwGuildID)
{
	SendGuildName(CGuildManager::instance().FindGuild(dwGuildID));
}

EVENTFUNC(destroy_when_idle_event)
{
	char_event_info* info = dynamic_cast<char_event_info*>(event->info);
	if (info == NULL)
	{
		sys_err("destroy_when_idle_event> <Factor> Null pointer");
		return 0;
	}

	LPCHARACTER ch = info->ch;
	if (ch == NULL) { // <Factor>
		return 0;
	}

	if (ch->GetVictim())
	{
		return PASSES_PER_SEC(300);
	}

	sys_log(1, "DESTROY_WHEN_IDLE: %s", ch->GetName());

	ch->m_pkDestroyWhenIdleEvent = NULL;
	M2_DESTROY_CHARACTER(ch);
	return 0;
}

void CHARACTER::StartDestroyWhenIdleEvent()
{
	if (m_pkDestroyWhenIdleEvent)
		return;

	char_event_info* info = AllocEventInfo<char_event_info>();

	info->ch = this;

	m_pkDestroyWhenIdleEvent = event_create(destroy_when_idle_event, info, PASSES_PER_SEC(300));
}

BYTE CHARACTER::GetComboIndex() const
{
	return m_bComboIndex;
}

void CHARACTER::SkipComboAttackByTime(int interval)
{
	m_dwSkipComboAttackByTime = get_dword_time() + interval;
}

DWORD CHARACTER::GetSkipComboAttackByTime() const
{
	return m_dwSkipComboAttackByTime;
}

void CHARACTER::ResetChatCounter()
{
	m_bChatCounter = 0;
}

BYTE CHARACTER::IncreaseChatCounter()
{
	return ++m_bChatCounter;
}

BYTE CHARACTER::GetChatCounter() const
{
	return m_bChatCounter;
}

bool CHARACTER::IsRiding() const
{
	return IsHorseRiding() || GetMountVnum();
}

bool CHARACTER::CanWarp() const
{
	const int iPulse = thecore_pulse();
	const int limit_time = PASSES_PER_SEC(g_nPortalLimitTime);

	if ((iPulse - GetSafeboxLoadTime()) < limit_time)
		return false;

	if ((iPulse - GetExchangeTime()) < limit_time)
		return false;

	if ((iPulse - GetMyShopTime()) < limit_time)
		return false;

	if ((iPulse - GetRefineTime()) < limit_time)
		return false;

	if (GetExchange() || GetMyShop() || GetShopOwner() || IsOpenSafebox() || IsCubeOpen() || IsUnderRefine())
		return false;

#ifdef __INGAME_MALL__
	if ((iPulse - GetVarTimeConst(VAR_TIME_INGAME_MALL)) < limit_time)
		return false;
#endif

#ifdef __ACCE_SYSTEM__
	if (isAcceOpened(true) || isAcceOpened(false))
		return false;
#endif

#ifdef __ITEM_CHANGELOOK__
	if (isChangeLookOpened())
		return false;
#endif

#ifdef __OFFLINE_SHOP__
	if (GetOfflineShopGuest() || GetShopSafebox())
		return false;
	if (iPulse - GetOfflineShopTime() < limit_time)
		return false;
#endif

#ifdef __INGAME_MALL__
	if (GetVarValueConst(VAR_VAL_INGAME_MALL) == true)
		return false;
#endif

	return true;
}

DWORD CHARACTER::GetNextExp() const
{
	if (PLAYER_MAX_LEVEL_CONST < GetLevel())
		return 2500000000u;
	else
		return exp_table[GetLevel()];
}

#ifdef __GROWTH_PET_SYSTEM__
DWORD CHARACTER::PetGetNextExp() const
{
	if (IsNewPet()) {
		if (PLAYER_MAX_LEVEL_CONST < GetLevel())
			return 2500000000u;
		else
			return exppet_table[GetLevel()];
	}

	return 0;
}
#endif

#ifdef __GROWTH_MOUNT_SYSTEM__
DWORD CHARACTER::GetMountNextExp() const
{
	if (IsNewMount()) {
		if (PLAYER_MAX_LEVEL_CONST < GetLevel())
			return 2500000000u;
		else
			return expmount_table[GetLevel()];
	}

	return 0;
}
#endif

int	CHARACTER::GetSkillPowerByLevel(int level, bool bMob) const
{
	return CTableBySkill::instance().GetSkillPowerByLevelFromType(GetJob(), GetSkillGroup(), MINMAX(0, level, SKILL_MAX_LEVEL), bMob);
}

#ifdef __OFFLINE_SHOP__
void CHARACTER::SetShopSafebox(offlineshop::CShopSafebox* pk)
{
	if (m_pkShopSafebox && pk == NULL)
		m_pkShopSafebox->SetOwner(NULL);

	else if (m_pkShopSafebox == NULL && pk)
		pk->SetOwner(this);

	m_pkShopSafebox = pk;
}
#endif

#ifdef __ACCE_SYSTEM__
std::vector<LPITEM> CHARACTER::GetAcceMaterials()
{
	if (!m_PlayerSlots)
		return std::vector<LPITEM>{nullptr, nullptr};
	return std::vector<LPITEM>{ITEM_MANAGER::instance().Find(m_PlayerSlots->pAcceMaterials[0].id), ITEM_MANAGER::instance().Find(m_PlayerSlots->pAcceMaterials[1].id)};
}

const TItemPosEx* CHARACTER::GetAcceMaterialsInfo()
{
	if (!m_PlayerSlots)
		return nullptr;
	return m_PlayerSlots->pAcceMaterials;
}

void CHARACTER::SetAcceMaterial(int pos, LPITEM ptr)
{
	if (!m_PlayerSlots)
		return;
	if (pos < 0 || pos >= ACCE_WINDOW_MAX_MATERIALS)
		return;
	if (!ptr)
		m_PlayerSlots->pAcceMaterials[pos] = {};
	else
	{
		m_PlayerSlots->pAcceMaterials[pos].id = ptr->GetID();
		m_PlayerSlots->pAcceMaterials[pos].pos.cell = ptr->GetCell();
		m_PlayerSlots->pAcceMaterials[pos].pos.window_type = ptr->GetWindow();
	}
}

void CHARACTER::OpenAcce(bool bCombination)
{
	if (isAcceOpened(bCombination))
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("The acce window it's already opened."));
		return;
	}

	if (bCombination)
	{
		if (m_bAcceAbsorption)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Before you may close the acce absorption window."));
			return;
		}

		m_bAcceCombination = true;
	}
	else
	{
		if (m_bAcceCombination)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Before you may close the acce combine window."));
			return;
		}

		m_bAcceAbsorption = true;
	}

	TItemPos tPos;
	tPos.window_type = INVENTORY;
	tPos.cell = 0;

	TPacketAcce sPacket;
	sPacket.header = HEADER_GC_ACCE;
	sPacket.subheader = ACCE_SUBHEADER_GC_OPEN;
	sPacket.bWindow = bCombination;
#ifdef __GOLD_LIMIT_REWORK__
	sPacket.llPrice = 0;
#else
	sPacket.dwPrice = 0;
#endif
	sPacket.bPos = 0;
	sPacket.tPos = tPos;
	sPacket.dwItemVnum = 0;
	sPacket.dwMinAbs = 0;
	sPacket.dwMaxAbs = 0;
	GetDesc()->Packet(&sPacket, sizeof(TPacketAcce));

	ClearAcceMaterials();
}

void CHARACTER::CloseAcce()
{
	if ((!m_bAcceCombination) && (!m_bAcceAbsorption))
		return;

	bool bWindow = (m_bAcceCombination == true ? true : false);

	TItemPos tPos;
	tPos.window_type = INVENTORY;
	tPos.cell = 0;

	TPacketAcce sPacket;
	sPacket.header = HEADER_GC_ACCE;
	sPacket.subheader = ACCE_SUBHEADER_GC_CLOSE;
	sPacket.bWindow = bWindow;
#ifdef __GOLD_LIMIT_REWORK__
	sPacket.llPrice = 0;
#else
	sPacket.dwPrice = 0;
#endif
	sPacket.bPos = 0;
	sPacket.tPos = tPos;
	sPacket.dwItemVnum = 0;
	sPacket.dwMinAbs = 0;
	sPacket.dwMaxAbs = 0;
	GetDesc()->Packet(&sPacket, sizeof(TPacketAcce));

	if (bWindow)
		m_bAcceCombination = false;
	else
		m_bAcceAbsorption = false;

	ClearAcceMaterials();
}

void CHARACTER::ClearAcceMaterials()
{
	auto pkItemMaterial = GetAcceMaterials();
	for (int i = 0; i < ACCE_WINDOW_MAX_MATERIALS; ++i)
	{
		if (!pkItemMaterial[i])
			continue;

		pkItemMaterial[i]->Lock(false);
		pkItemMaterial[i] = NULL;
		SetAcceMaterial(i, nullptr);
	}
}

bool CHARACTER::AcceIsSameGrade(long lGrade)
{
	auto pkItemMaterial = GetAcceMaterials();
	if (!pkItemMaterial[0])
		return false;

	bool bReturn = (pkItemMaterial[0]->GetValue(ACCE_GRADE_VALUE_FIELD) == lGrade ? true : false);
	return bReturn;
}

#ifdef __GOLD_LIMIT_REWORK__
long long CHARACTER::GetAcceCombinePrice(long lGrade)
{
	long long llPrice = 0;
	switch (lGrade)
	{
	case 2:
	{
		llPrice = ACCE_GRADE_2_PRICE;
	}
	break;
	case 3:
	{
		llPrice = ACCE_GRADE_3_PRICE;
	}
	break;
	case 4:
	{
		llPrice = ACCE_GRADE_4_PRICE;
	}
	break;
	default:
	{
		llPrice = ACCE_GRADE_1_PRICE;
	}
	break;
	}

	return llPrice;
}
#else
DWORD CHARACTER::GetAcceCombinePrice(long lGrade)
{
	DWORD dwPrice = 0;
	switch (lGrade)
	{
	case 2:
	{
		dwPrice = ACCE_GRADE_2_PRICE;
	}
	break;
	case 3:
	{
		dwPrice = ACCE_GRADE_3_PRICE;
	}
	break;
	case 4:
	{
		dwPrice = ACCE_GRADE_4_PRICE;
	}
	break;
	default:
	{
		dwPrice = ACCE_GRADE_1_PRICE;
	}
	break;
	}

	return dwPrice;
}
#endif

BYTE CHARACTER::CheckEmptyMaterialSlot()
{
	auto pkItemMaterial = GetAcceMaterials();
	for (int i = 0; i < ACCE_WINDOW_MAX_MATERIALS; ++i)
	{
		if (!pkItemMaterial[i])
			return i;
	}

	return 255;
}

#ifdef __ACCE_25ABS_ITEMS__
bool AcceAbsRenewal(DWORD vnum)
{
	//switch (vnum)
	//{
	//case 85004:
	//case 85008:
	//case 85014:
	//case 85018:
	//case 85024:
	//	return true;
	//}
	return false;
}
#endif

#define ACCE_GRADE_4_ABS_RANGE_EX 5
#define ACCE_GRADE_NEW_ABS_25_EX 1000
#define ACCE_GRADE_4_ABS_MAX_EX 1000

void CHARACTER::GetAcceCombineResult(DWORD& dwItemVnum, DWORD& dwMinAbs, DWORD& dwMaxAbs)
{
	auto pkItemMaterial = GetAcceMaterials();

	if (m_bAcceCombination)
	{
		if ((pkItemMaterial[0]) && (pkItemMaterial[1]))
		{
			long lVal = pkItemMaterial[0]->GetValue(ACCE_GRADE_VALUE_FIELD);
			if (lVal == 4)
			{
				dwItemVnum = pkItemMaterial[0]->GetOriginalVnum();
				dwMinAbs = pkItemMaterial[0]->GetSocket(ACCE_ABSORPTION_SOCKET);
#ifdef __ACCE_25ABS_ITEMS__
				DWORD dwMaxAbsCalc = 0;
				if (AcceAbsRenewal(dwItemVnum) == true)
					dwMaxAbsCalc = (dwMinAbs + ACCE_GRADE_4_ABS_RANGE_EX > ACCE_GRADE_NEW_ABS_25_EX ? ACCE_GRADE_NEW_ABS_25_EX : (dwMinAbs + ACCE_GRADE_4_ABS_RANGE_EX));
				else
					dwMaxAbsCalc = (dwMinAbs + ACCE_GRADE_4_ABS_RANGE_EX > ACCE_GRADE_4_ABS_MAX_EX ? ACCE_GRADE_4_ABS_MAX_EX : (dwMinAbs + ACCE_GRADE_4_ABS_RANGE_EX));
#else
				DWORD dwMaxAbsCalc = (dwMinAbs + ACCE_GRADE_4_ABS_RANGE_EX > ACCE_GRADE_4_ABS_MAX_EX ? ACCE_GRADE_4_ABS_MAX_EX : (dwMinAbs + ACCE_GRADE_4_ABS_RANGE_EX));
#endif
				dwMaxAbs = dwMaxAbsCalc;


				if (pkItemMaterial[0]->GetSocket(2) == 0 && dwMaxAbs >= 250)
					dwMaxAbs = 250;
				else if (pkItemMaterial[0]->GetSocket(2) == 1 && dwMaxAbs >= 300)
					dwMaxAbs = 300;
				else if (pkItemMaterial[0]->GetSocket(2) == 2 && dwMaxAbs >= 350)
					dwMaxAbs = 350;
				else if (pkItemMaterial[0]->GetSocket(2) == 3 && dwMaxAbs >= 400)
					dwMaxAbs = 400;
				else if (pkItemMaterial[0]->GetSocket(2) == 4 && dwMaxAbs >= 450)
					dwMaxAbs = 450;
				else if (pkItemMaterial[0]->GetSocket(2) == 5 && dwMaxAbs >= 500)
					dwMaxAbs = 500;
				else if (pkItemMaterial[0]->GetSocket(2) == 6 && dwMaxAbs >= 600)
					dwMaxAbs = 600;
				else if (pkItemMaterial[0]->GetSocket(2) == 7 && dwMaxAbs >= 700)
					dwMaxAbs = 700;
				else if (pkItemMaterial[0]->GetSocket(2) == 8 && dwMaxAbs >= 800)
					dwMaxAbs = 800;
				else if (pkItemMaterial[0]->GetSocket(2) == 9 && dwMaxAbs >= 900)
					dwMaxAbs = 900;
				else if (pkItemMaterial[0]->GetSocket(2) == 10 && dwMaxAbs >= 1000)
					dwMaxAbs = 1000;
			}
			else
			{
				DWORD dwMaskVnum = pkItemMaterial[0]->GetOriginalVnum();
				TItemTable* pTable = ITEM_MANAGER::instance().GetTable(dwMaskVnum + 1);
				if (pTable)
					dwMaskVnum += 1;

				dwItemVnum = dwMaskVnum;
				switch (lVal)
				{
				case 2:
				{
					dwMinAbs = ACCE_GRADE_3_ABS;
					dwMaxAbs = ACCE_GRADE_3_ABS;
				}
				break;
				case 3:
				{
#ifdef __ACCE_25ABS_ITEMS__
					if (AcceAbsRenewal(dwItemVnum) == true)
						dwMinAbs = ACCE_GRADE_4_ABS_MIN_ABS;
					else
						dwMinAbs = ACCE_GRADE_4_ABS_MIN;
#else
					dwMinAbs = ACCE_GRADE_4_ABS_MIN;
#endif
					dwMaxAbs = ACCE_GRADE_4_ABS_MAX_COMB;
				}
				break;
				default:
				{
					dwMinAbs = ACCE_GRADE_2_ABS;
					dwMaxAbs = ACCE_GRADE_2_ABS;
				}
				break;
				}
			}
		}
		else
		{
			dwItemVnum = 0;
			dwMinAbs = 0;
			dwMaxAbs = 0;
		}
	}
	else
	{
		if ((pkItemMaterial[0]) && (pkItemMaterial[1]))
		{
			dwItemVnum = pkItemMaterial[0]->GetOriginalVnum();
			dwMinAbs = pkItemMaterial[0]->GetSocket(ACCE_ABSORPTION_SOCKET);
			dwMaxAbs = dwMinAbs;
		}
		else
		{
			dwItemVnum = 0;
			dwMinAbs = 0;
			dwMaxAbs = 0;
		}
	}
}

void CHARACTER::AddAcceMaterial(TItemPos tPos, BYTE bPos)
{
	if (bPos >= ACCE_WINDOW_MAX_MATERIALS)
	{
		if (bPos == 255)
		{
			bPos = CheckEmptyMaterialSlot();
			if (bPos >= ACCE_WINDOW_MAX_MATERIALS)
				return;
		}
		else
			return;
	}

#ifdef __OFFLINE_SHOP__
	if (GetOfflineShopGuest() || GetShopSafebox())// Offlineshop Update
		return;
#endif

	if (GetExchange() || GetMyShop() || GetShopOwner() || IsOpenSafebox() || IsCubeOpen() || IsUnderRefine())
		return;

#ifdef __ITEM_CHANGELOOK__
	if (isChangeLookOpened())
		return;
#endif

	LPITEM pkItem = GetItem(tPos);
	if (!pkItem)
		return;
	else if ((pkItem->GetCell() >= INVENTORY_MAX_NUM) || (pkItem->IsEquipped()) || (tPos.IsBeltInventoryPosition()) || (pkItem->IsDragonSoul()))
		return;
	else if ((pkItem->GetType() != ITEM_COSTUME) && (m_bAcceCombination))
		return;
	else if ((pkItem->GetType() != ITEM_COSTUME) && (bPos == 0) && (m_bAcceAbsorption))
		return;
#ifdef __SLOT_MARKING__
	else if (IS_SET(pkItem->GetAntiFlag(), ITEM_ANTIFLAG_ACCE))
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("WARNING_ANTIFLAG_ACCE"));
		return;
	}
#endif
	else if (pkItem->isLocked())
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You can't add locked items."));
		return;
	}
#ifdef __BEGINNER_ITEM__
	else if ((pkItem->IsBasicItem()))
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You can't add binded items."));
		return;
	}
#endif
#ifdef __ITEM_EVOLUTION__
	else if (pkItem->GetEvolution() != 0)
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("KUSAK_EVRIM_FX"));
		return;
	}
#endif
	else if (m_bAcceCombination && (pkItem->GetSocket(2) < 1 && (pkItem->GetSocket(ACCE_ABSORPTION_SOCKET) >= 250)))
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("KUSAK_ENGEL_6"));
		return;
	}
	else if (m_bAcceCombination && (pkItem->GetSocket(2) < 2 && (pkItem->GetSocket(ACCE_ABSORPTION_SOCKET) >= 300)))
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("KUSAK_ENGEL_7"));
		return;
	}
	else if (m_bAcceCombination && (pkItem->GetSocket(2) < 3 && (pkItem->GetSocket(ACCE_ABSORPTION_SOCKET) >= 350)))
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("KUSAK_ENGEL_8"));
		return;
	}
	else if (m_bAcceCombination && (pkItem->GetSocket(2) < 4 && (pkItem->GetSocket(ACCE_ABSORPTION_SOCKET) >= 400)))
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("KUSAK_ENGEL_9"));
		return;
	}
	else if (m_bAcceCombination && (pkItem->GetSocket(2) < 5 && (pkItem->GetSocket(ACCE_ABSORPTION_SOCKET) >= 450)))
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("KUSAK_ENGEL_10"));
		return;
	}
	else if (m_bAcceCombination && (pkItem->GetSocket(2) < 6 && (pkItem->GetSocket(ACCE_ABSORPTION_SOCKET) >= 500)))
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("KUSAK_ENGEL_11"));
		return;
	}
	else if (m_bAcceCombination && (pkItem->GetSocket(2) < 7 && (pkItem->GetSocket(ACCE_ABSORPTION_SOCKET) >= 600)))
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("KUSAK_ENGEL_12"));
		return;
	}
	else if (m_bAcceCombination && (pkItem->GetSocket(2) < 8 && (pkItem->GetSocket(ACCE_ABSORPTION_SOCKET) >= 700)))
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("KUSAK_ENGEL_13"));
		return;
	}
	else if (m_bAcceCombination && (pkItem->GetSocket(2) < 9 && (pkItem->GetSocket(ACCE_ABSORPTION_SOCKET) >= 800)))
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("KUSAK_ENGEL_14"));
		return;
	}
	else if (m_bAcceCombination && (pkItem->GetSocket(2) < 10 && (pkItem->GetSocket(ACCE_ABSORPTION_SOCKET) >= 900)))
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("KUSAK_ENGEL_15"));
		return;
	}
	else if ((m_bAcceCombination) && (bPos == 1) && (!AcceIsSameGrade(pkItem->GetValue(ACCE_GRADE_VALUE_FIELD))))
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You can combine just accees of same grade."));
		return;
	}
	else if ((m_bAcceCombination) && (pkItem->GetSocket(ACCE_ABSORPTION_SOCKET) >= ACCE_GRADE_4_ABS_MAX))
	{
#ifdef __ACCE_25ABS_ITEMS__
		if (AcceAbsRenewal(pkItem->GetVnum()) == true)
		{
			if (pkItem->GetSocket(ACCE_ABSORPTION_SOCKET) >= ACCE_GRADE_NEW_ABS_25)
			{
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("This acce got already maximum absorption chance."));
				return;
			}
		}
		else
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("This acce got already maximum absorption chance."));
			return;
		}
#else
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("This acce got already maximum absorption chance."));
		return;
#endif
	}
	else if ((bPos == 1) && (m_bAcceAbsorption))
	{
		if ((pkItem->GetType() != ITEM_WEAPON))
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You can absorb just the bonuses from armors and weapons."));
			return;
		}
	}
	else if ((pkItem->GetSubType() != COSTUME_ACCE) && (m_bAcceCombination))
		return;
	else if ((pkItem->GetSubType() != COSTUME_ACCE) && (bPos == 0) && (m_bAcceAbsorption))
		return;
	else if ((pkItem->GetSocket(ACCE_ABSORBED_SOCKET) > 0) && (bPos == 0) && (m_bAcceAbsorption))
		return;

	auto pkItemMaterial = GetAcceMaterials();
	if ((bPos == 1) && (!pkItemMaterial[0]))
		return;

	if (pkItemMaterial[bPos])
		return;
	SetAcceMaterial(bPos, pkItem);
	pkItemMaterial[bPos] = pkItem;
	pkItemMaterial[bPos]->Lock(true);

	DWORD dwItemVnum, dwMinAbs, dwMaxAbs;
	GetAcceCombineResult(dwItemVnum, dwMinAbs, dwMaxAbs);

	TPacketAcce sPacket;
	sPacket.header = HEADER_GC_ACCE;
	sPacket.subheader = ACCE_SUBHEADER_GC_ADDED;
	sPacket.bWindow = m_bAcceCombination == true ? true : false;
#ifdef __GOLD_LIMIT_REWORK__
	sPacket.llPrice = GetAcceCombinePrice(pkItem->GetValue(ACCE_GRADE_VALUE_FIELD));
#else
	sPacket.dwPrice = GetAcceCombinePrice(pkItem->GetValue(ACCE_GRADE_VALUE_FIELD));
#endif
	sPacket.bPos = bPos;
	sPacket.tPos = tPos;
	sPacket.dwItemVnum = dwItemVnum;
	sPacket.dwMinAbs = dwMinAbs;
	sPacket.dwMaxAbs = dwMaxAbs;
	GetDesc()->Packet(&sPacket, sizeof(TPacketAcce));
}

void CHARACTER::RemoveAcceMaterial(BYTE bPos)
{
	if (bPos >= ACCE_WINDOW_MAX_MATERIALS)
		return;

	auto pkItemMaterial = GetAcceMaterials();

#ifdef __GOLD_LIMIT_REWORK__
	long long llPrice = 0;
#else
	DWORD dwPrice = 0;
#endif

	if (bPos == 1)
	{
		if (pkItemMaterial[bPos])
		{
			pkItemMaterial[bPos]->Lock(false);
			pkItemMaterial[bPos] = NULL;
			SetAcceMaterial(bPos, nullptr);
		}

		if (pkItemMaterial[0])
#ifdef __GOLD_LIMIT_REWORK__
			llPrice = GetAcceCombinePrice(pkItemMaterial[0]->GetValue(ACCE_GRADE_VALUE_FIELD));
#else
			dwPrice = GetAcceCombinePrice(pkItemMaterial[0]->GetValue(ACCE_GRADE_VALUE_FIELD));
#endif
	}
	else
		ClearAcceMaterials();

	TItemPos tPos;
	tPos.window_type = INVENTORY;
	tPos.cell = 0;

	TPacketAcce sPacket;
	sPacket.header = HEADER_GC_ACCE;
	sPacket.subheader = ACCE_SUBHEADER_GC_REMOVED;
	sPacket.bWindow = m_bAcceCombination == true ? true : false;
#ifdef __GOLD_LIMIT_REWORK__
	sPacket.llPrice = llPrice;
#else
	sPacket.dwPrice = dwPrice;
#endif
	sPacket.bPos = bPos;
	sPacket.tPos = tPos;
	sPacket.dwItemVnum = 0;
	sPacket.dwMinAbs = 0;
	sPacket.dwMaxAbs = 0;
	GetDesc()->Packet(&sPacket, sizeof(TPacketAcce));
}

BYTE CHARACTER::CanRefineAcceMaterials()
{
	BYTE bReturn = 0;
	if (!GetDesc())
		return bReturn;

	if (GetExchange() || GetMyShop() || GetShopOwner() || IsOpenSafebox() || IsCubeOpen() || IsUnderRefine())
		return bReturn;

#ifdef __ITEM_CHANGELOOK__
	if (isChangeLookOpened())
		return bReturn;
#endif

#ifdef __OFFLINE_SHOP__
	if (GetOfflineShopGuest() || GetShopSafebox())// Offlineshop Update
		return 0;
#endif

	auto materialInfo = GetAcceMaterialsInfo();
	auto pkItemMaterial = GetAcceMaterials();
	if (!pkItemMaterial[0] || !pkItemMaterial[1])
		return bReturn;
	else if (pkItemMaterial[0]->GetOwner() != this || pkItemMaterial[1]->GetOwner() != this)
		return bReturn;
	else if (pkItemMaterial[0]->IsEquipped() || pkItemMaterial[1]->IsEquipped())
		return bReturn;
	else if (pkItemMaterial[0]->GetWindow() != INVENTORY || pkItemMaterial[1]->GetWindow() != INVENTORY)
		return bReturn;
	else if (!materialInfo[0].id || !materialInfo[1].id)
		return bReturn;
	else if (materialInfo[0].pos.cell != pkItemMaterial[0]->GetCell() || materialInfo[1].pos.cell != pkItemMaterial[1]->GetCell())
		return bReturn;
	else if (materialInfo[0].pos.window_type != pkItemMaterial[0]->GetWindow() || materialInfo[1].pos.window_type != pkItemMaterial[1]->GetWindow())
		return bReturn;

	if (m_bAcceCombination)
	{
		for (int i = 0; i < ACCE_WINDOW_MAX_MATERIALS; ++i)
		{
			if (pkItemMaterial[i])
			{
				// @kusak icin 30 emis - 25 emis fixi
				if ((m_bAcceCombination) && AcceAbsRenewal(pkItemMaterial[0]->GetVnum()))
				{
					if (!AcceAbsRenewal(pkItemMaterial[1]->GetVnum()))
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("AYNI_MAX_EMISI_KULLAN."));
						bReturn = 0;
						break;
					}
				}
				if ((m_bAcceCombination) && AcceAbsRenewal(pkItemMaterial[1]->GetVnum()))
				{
					if (!AcceAbsRenewal(pkItemMaterial[0]->GetVnum()))
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("AYNI_MAX_EMISI_KULLAN."));
						bReturn = 0;
						break;
					}
				}
				if ((pkItemMaterial[i]->GetType() == ITEM_COSTUME) && (pkItemMaterial[i]->GetSubType() == COSTUME_ACCE))
					bReturn = 1;
				else
				{
					bReturn = 0;
					break;
				}
			}
			else
			{
				bReturn = 0;
				break;
			}
		}
	}
	else if (m_bAcceAbsorption)
	{
		if ((pkItemMaterial[0]) && (pkItemMaterial[1]))
		{
			if ((pkItemMaterial[0]->GetType() == ITEM_COSTUME) && (pkItemMaterial[0]->GetSubType() == COSTUME_ACCE))
				bReturn = 2;
			else
				bReturn = 0;

			if ((pkItemMaterial[1]->GetType() == ITEM_WEAPON))
				bReturn = 2;
			else
				bReturn = 0;

			if (pkItemMaterial[0]->GetSocket(ACCE_ABSORBED_SOCKET) > 0)
				bReturn = 0;
		}
		else
			bReturn = 0;
	}

	return bReturn;
}

void CHARACTER::RefineAcceMaterials()
{
	BYTE bCan = CanRefineAcceMaterials();
	if (bCan == 0)
		return;

	auto pkItemMaterial = GetAcceMaterials();

	DWORD dwItemVnum, dwMinAbs, dwMaxAbs;
	GetAcceCombineResult(dwItemVnum, dwMinAbs, dwMaxAbs);
#ifdef __GOLD_LIMIT_REWORK__
	long long llPrice = GetAcceCombinePrice(pkItemMaterial[0]->GetValue(ACCE_GRADE_VALUE_FIELD));
#else
	DWORD dwPrice = GetAcceCombinePrice(pkItemMaterial[0]->GetValue(ACCE_GRADE_VALUE_FIELD));
#endif

	if (bCan == 1)
	{
		int iSuccessChance = 0;
		long lVal = pkItemMaterial[0]->GetValue(ACCE_GRADE_VALUE_FIELD);
		switch (lVal)
		{
		case 2:
		{
			iSuccessChance = ACCE_COMBINE_GRADE_2;
		}
		break;
		case 3:
		{
			iSuccessChance = ACCE_COMBINE_GRADE_3;
		}
		break;
		case 4:
		{
			iSuccessChance = ACCE_COMBINE_GRADE_4;
		}
		break;
		default:
		{
			iSuccessChance = ACCE_COMBINE_GRADE_1;
		}
		break;
		}

#ifdef __GOLD_LIMIT_REWORK__
		if (GetGold() < llPrice)
#else
		if (GetGold() < dwPrice)
#endif
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You don't have enough Yang."));
			return;
		}

		int iChance = number(1, 100);
		bool bSucces = (iChance <= iSuccessChance ? true : false);
		if (bSucces)
		{
			LPITEM pkItem = ITEM_MANAGER::instance().CreateItem(dwItemVnum, 1, 0, false);
			if (!pkItem)
			{
				sys_err("%d can't be created.", dwItemVnum);
				return;
			}

			ITEM_MANAGER::CopyAllAttrTo(pkItemMaterial[0], pkItem);
			DWORD dwAbs = (dwMinAbs == dwMaxAbs ? dwMinAbs : number(dwMinAbs + 1, dwMaxAbs));
			pkItem->SetSocket(ACCE_ABSORPTION_SOCKET, dwAbs);
			pkItem->SetSocket(ACCE_ABSORBED_SOCKET, pkItemMaterial[0]->GetSocket(ACCE_ABSORBED_SOCKET));
			pkItem->SetSocket(2, pkItemMaterial[0]->GetSocket(2));

#ifdef __GOLD_LIMIT_REWORK__
			PointChange(POINT_GOLD, -llPrice);
#else
			PointChange(POINT_GOLD, -dwPrice);
#endif
			SetAcceTime();
			WORD wCell = pkItemMaterial[0]->GetCell();
			ITEM_MANAGER::instance().RemoveItem(pkItemMaterial[0], "ACCE COMBINE (REFINE SUCCESS)");
			ITEM_MANAGER::instance().RemoveItem(pkItemMaterial[1], "ACCE COMBINE (REFINE SUCCESS)");

			pkItem->__ADD_TO_CHARACTER(this, TItemPos(INVENTORY, wCell));
			ITEM_MANAGER::instance().FlushDelayedSave(pkItem);

			if (lVal == 4)
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("New absorption rate: %d%"), dwAbs);
			else
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Success."));

			EffectPacket(SE_EFFECT_ACCE_SUCCEDED);

			ClearAcceMaterials();
		}
		else
		{
#ifdef __GOLD_LIMIT_REWORK__
			PointChange(POINT_GOLD, -llPrice);
#else
			PointChange(POINT_GOLD, -dwPrice);
#endif
			SetAcceTime();
			ITEM_MANAGER::instance().RemoveItem(pkItemMaterial[1], "ACCE COMBINE (REFINE FAIL)");

			if (lVal == 4)
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("New absorption rate: %d%"), pkItemMaterial[0]->GetSocket(ACCE_ABSORPTION_SOCKET));
			else
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Failed."));

			pkItemMaterial[1] = NULL;
		}

		TItemPos tPos;
		tPos.window_type = INVENTORY;
		tPos.cell = 0;

		TPacketAcce sPacket;
		sPacket.header = HEADER_GC_ACCE;
		sPacket.subheader = ACCE_SUBHEADER_CG_REFINED;
		sPacket.bWindow = m_bAcceCombination == true ? true : false;
#ifdef __GOLD_LIMIT_REWORK__
		sPacket.llPrice = llPrice;
#else
		sPacket.dwPrice = dwPrice;
#endif
		sPacket.bPos = 0;
		sPacket.tPos = tPos;
		sPacket.dwItemVnum = 0;
		sPacket.dwMinAbs = 0;
		if (bSucces)
			sPacket.dwMaxAbs = 100;
		else
			sPacket.dwMaxAbs = 0;

		GetDesc()->Packet(&sPacket, sizeof(TPacketAcce));
	}
	else
	{
		pkItemMaterial[1]->CopyAttributeTo(pkItemMaterial[0]);
		pkItemMaterial[0]->SetSocket(ACCE_ABSORBED_SOCKET, pkItemMaterial[1]->GetOriginalVnum());
		for (int i = 0; i < ITEM_ATTRIBUTE_MAX_NUM; ++i)
		{
			if (pkItemMaterial[0]->GetAttributeValue(i) < 0)
				pkItemMaterial[0]->SetForceAttribute(i, pkItemMaterial[0]->GetAttributeType(i), 0);
		}

		ITEM_MANAGER::instance().RemoveItem(pkItemMaterial[1], "ABSORBED (REFINE SUCCESS)");

		ITEM_MANAGER::instance().FlushDelayedSave(pkItemMaterial[0]);
		SetAcceTime();

		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Success."));

		ClearAcceMaterials();

		TItemPos tPos;
		tPos.window_type = INVENTORY;
		tPos.cell = 0;

		TPacketAcce sPacket;
		sPacket.header = HEADER_GC_ACCE;
		sPacket.subheader = ACCE_SUBHEADER_CG_REFINED;
		sPacket.bWindow = m_bAcceCombination == true ? true : false;
#ifdef __GOLD_LIMIT_REWORK__
		sPacket.llPrice = llPrice;
#else
		sPacket.dwPrice = dwPrice;
#endif
		sPacket.bPos = 255;
		sPacket.tPos = tPos;
		sPacket.dwItemVnum = 0;
		sPacket.dwMinAbs = 0;
		sPacket.dwMaxAbs = 1;
		GetDesc()->Packet(&sPacket, sizeof(TPacketAcce));
	}
}

bool CHARACTER::CleanAcceAttr(LPITEM pkItem, LPITEM pkTarget)
{
	if (!CanHandleItem())
		return false;
	else if ((!pkItem) || (!pkTarget))
		return false;
	else if ((pkTarget->GetType() != ITEM_COSTUME) && (pkTarget->GetSubType() != COSTUME_ACCE))
		return false;

	if (pkTarget->GetSocket(ACCE_ABSORBED_SOCKET) <= 0)
		return false;

	pkTarget->SetSocket(ACCE_ABSORBED_SOCKET, 0);
	for (int i = 0; i < ITEM_ATTRIBUTE_MAX_NUM; ++i)
		pkTarget->SetForceAttribute(i, 0, 0);

	pkItem->SetCount(pkItem->GetCount() - 1);
	SetAcceTime();
	return true;
}
#endif

#ifdef __CHANNEL_CHANGER__
bool CHARACTER::SwitchChannel(long newAddr, WORD newPort)
{
	if (!IsPC() || !GetDesc() || !CanWarp())
		return false;

	long x = GetX();
	long y = GetY();
	long lAddr = newAddr;
	long lMapIndex = GetMapIndex();
	WORD wPort = newPort;
	if (lMapIndex >= 10000)
	{
		sys_err("Invalid change channel request from dungeon %d!", lMapIndex);
		return false;
	}
	if (g_bChannel == 99)
	{
		sys_err("%s attempted to change channel from CH99, ignoring req.", GetName());
		return false;
	}
	Stop();
	Save();
	if (GetSectree())
	{
		GetSectree()->RemoveEntity(this);
		ViewCleanup();
		EncodeRemovePacket(this);
	}
	m_lWarpMapIndex = lMapIndex;
	m_posWarp.x = x;
	m_posWarp.y = y;
	sys_log(0, "ChangeChannel %s, %ld %ld map %ld to port %d", GetName(), x, y, GetMapIndex(), wPort);
	TPacketGCWarp p;
	p.bHeader = HEADER_GC_WARP;
	p.lX = x;
	p.lY = y;
	p.lAddr = lAddr;
	if (!g_stProxyIP.empty())
		p.lAddr = inet_addr(g_stProxyIP.c_str());
	p.wPort = wPort;

#ifdef __SWITCHBOT__
	CSwitchbotManager::Instance().SetIsWarping(GetPlayerID(), true);

	if (p.wPort != mother_port)
		CSwitchbotManager::Instance().P2PSendSwitchbot(GetPlayerID(), p.wPort);
#endif

	GetDesc()->Packet(&p, sizeof(TPacketGCWarp));
	return true;
}
EVENTINFO(switch_channel_info)
{
	DynamicCharacterPtr ch;
	int secs;
	long newAddr;
	WORD newPort;
	switch_channel_info()
		: ch(),
		secs(0),
		newAddr(0),
		newPort(0)
	{
	}
};
EVENTFUNC(switch_channel)
{
	switch_channel_info* info = dynamic_cast<switch_channel_info*>(event->info);
	if (!info)
	{
		sys_err("No switch channel event info!");
		return 0;
	}
	LPCHARACTER	ch = info->ch;
	if (!ch)
	{
		sys_err("No char to work on for the switch.");
		return 0;
	}
	if (!ch->GetDesc())
		return 0;

	if (!ch->CanWarp())
		return 0;

	if (quest::CQuestManager::instance().GetPCForce(ch->GetPlayerID())->IsRunning() == true)
		return 0;

	if (!ch->CanAct(true, true, VAR_TIME_CHANNEL_CHANGE))
		return 0;

	if (info->secs > 0)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Channel switch in %d seconds."), info->secs);
		--info->secs;
		return PASSES_PER_SEC(1);
	}
	ch->SwitchChannel(info->newAddr, info->newPort);
	ch->m_pkTimedEvent = NULL;
	return 0;
}
bool CHARACTER::StartChannelSwitch(long newAddr, WORD newPort)
{
	if (quest::CQuestManager::instance().GetPCForce(GetPlayerID())->IsRunning() == true)
		return false;

	if (!CanAct(true, true, VAR_TIME_CHANNEL_CHANGE))
		return false;

	AddAffect(AFFECT_KANAL_FIX, APPLY_NONE, 0, AFF_NONE, 10, 0, false);

	switch_channel_info* info = AllocEventInfo<switch_channel_info>();
	info->ch = this;
	info->secs = CanWarp() && !IsPosition(POS_FIGHTING) ? 3 : 10; // !IsPosition(POS_FIGHTING) ? 3 : 10;
	info->newAddr = newAddr;
	info->newPort = newPort;
	m_pkTimedEvent = event_create(switch_channel, info, 1);
	ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Channel switch starting."));
	return true;
}
#endif

#ifdef __SKILLBOOK_SYSTEM__
int CHARACTER::BKBul(long skillindex) const
{
	int	cell = 999;
	LPITEM item;

#ifdef __ADDITIONAL_INVENTORY__
	for (int i = 0; i < SPECIAL_INVENTORY_MAX_NUM; ++i)
	{
		item = GetBookInventoryItem(i);
		if (NULL != item && item->GetSocket(0) == skillindex && item->GetVnum() == 50300)
		{
			if (m_pkMyShop && m_pkMyShop->IsSellingItem(item->GetID()))
				continue;
			else
				cell = item->GetCell();
		}
	}
#else
	for (int i = 0; i < INVENTORY_MAX_NUM; ++i)
	{
		item = GetInventoryItem(i);
		if (NULL != item && item->GetSocket(0) == skillindex && item->GetVnum() == 50300)
		{
			if (m_pkMyShop && m_pkMyShop->IsSellingItem(item->GetID()))
				continue;
			else
				cell = item->GetCell();
		}
	}
#endif

	return cell;
}
#endif

#ifdef __INGAME_MALL__
DWORD CHARACTER::GetDragonCoin()
{
	std::unique_ptr<SQLMsg> pMsg(DBManager::instance().DirectQuery("SELECT cash FROM account.account WHERE id = '%d';", GetDesc()->GetAccountTable().id));
	if (pMsg->Get()->uiNumRows == 0)
		return 0;

	MYSQL_ROW row = mysql_fetch_row(pMsg->Get()->pSQLResult);
	DWORD dc = 0;
	str_to_number(dc, row[0]);
	return dc;
}

DWORD CHARACTER::GetDragonMark()
{
	std::unique_ptr<SQLMsg> pMsg(DBManager::instance().DirectQuery("SELECT coins FROM account.account WHERE id = '%d';", GetDesc()->GetAccountTable().id));
	if (pMsg->Get()->uiNumRows == 0)
		return 0;

	MYSQL_ROW row = mysql_fetch_row(pMsg->Get()->pSQLResult);
	DWORD mark = 0;
	str_to_number(mark, row[0]);
	return mark;
}

void CHARACTER::SetDragonCoin(DWORD amount)
{
	std::unique_ptr<SQLMsg> pMsg(DBManager::instance().DirectQuery("UPDATE account.account SET cash = '%d' WHERE id = '%d';", amount, GetDesc()->GetAccountTable().id));
	ChatPacket(CHAT_TYPE_COMMAND, "RefreshDragonCoin %d", GetDragonCoin());
	ChatPacket(CHAT_TYPE_COMMAND, "RefreshDragonMark %d", GetDragonMark());
}

void CHARACTER::SetDragonMark(DWORD amount)
{
	std::unique_ptr<SQLMsg> pMsg(DBManager::instance().DirectQuery("UPDATE account.account SET coins = '%d' WHERE id = '%d';", amount, GetDesc()->GetAccountTable().id));
	ChatPacket(CHAT_TYPE_COMMAND, "RefreshDragonCoin %d", GetDragonCoin());
	ChatPacket(CHAT_TYPE_COMMAND, "RefreshDragonMark %d", GetDragonMark());
}

bool CHARACTER::IsRefreshDragonCoinTimeOut()
{
	if (m_dwRefreshDragonCoinTime == 0)
		return true;

	return m_dwRefreshDragonCoinTime >= (get_global_time() + 60);
}

void CHARACTER::RefreshDragonCoin()
{
	if (!IsRefreshDragonCoinTimeOut())
		return;

	ChatPacket(CHAT_TYPE_COMMAND, "RefreshDragonCoin %d", GetDragonCoin());
	ChatPacket(CHAT_TYPE_COMMAND, "RefreshDragonMark %d", GetDragonMark());

	SetRefreshDragonCoinTime(get_global_time());
}

void CHARACTER::UpdateShop()
{
	ChatPacket(CHAT_TYPE_COMMAND, "BINARY_ItemShop_Close");
	CItemShopManager::instance().SendClientPacket(this);
}
#endif

#if defined(__ITEM_CHANGELOOK__) && defined(__MOUNT_CHANGELOOK__)
void CHARACTER::ChangeLookWindow(bool bOpen, bool bRequest, bool bMount)
#elif defined(__ITEM_CHANGELOOK__) && !defined(__MOUNT_CHANGELOOK__)
void CHARACTER::ChangeLookWindow(bool bOpen, bool bRequest)
#endif
#if defined(__ITEM_CHANGELOOK__)
{
	if ((bOpen) && (isChangeLookOpened()))
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[Transmutation] The window is already opened."));
		return;
	}

	if ((!bOpen) && (!isChangeLookOpened()))
	{
		if (!bRequest)
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[Transmutation] The window is not opened."));

		return;
	}

#ifdef __OFFLINE_SHOP__
	if (GetOfflineShopGuest() || GetShopSafebox())// Offlineshop Update
		return;
#endif

	if (GetExchange() || GetMyShop() || GetShopOwner() || IsOpenSafebox() || IsCubeOpen() || IsUnderRefine())
		return;

#ifdef __ACCE_SYSTEM__
	if (isAcceOpened(true) || isAcceOpened(false))
		return;
#endif


	TItemPos tPos;
	tPos.window_type = INVENTORY;
	tPos.cell = 0;

	TPacketChangeLook sPacket;
	sPacket.header = HEADER_GC_CL;
	sPacket.subheader = CL_SUBHEADER_OPEN;
	sPacket.subheader = bOpen == true ? CL_SUBHEADER_OPEN : CL_SUBHEADER_CLOSE;
#ifdef __GOLD_LIMIT_REWORK__
	sPacket.llCost = bOpen == true ? CL_TRANSMUTATION_PRICE : 0;
#else
	sPacket.dwCost = bOpen == true ? CL_TRANSMUTATION_PRICE : 0;
#endif
	sPacket.bPos = 0;
	sPacket.tPos = tPos;
#ifdef __MOUNT_CHANGELOOK__
	sPacket.bMount = bMount;
#endif
	GetDesc()->Packet(&sPacket, sizeof(TPacketChangeLook));

	m_bChangeLook = bOpen;
#ifdef __MOUNT_CHANGELOOK__
	m_bMountChangeLook = bMount;
#endif
	ClearClWindowMaterials();
}

void CHARACTER::ClearClWindowMaterials()
{
	LPITEM* pkItemMaterial;
	pkItemMaterial = GetClWindowMaterials();
	for (int i = 0; i < CL_WINDOW_MAX_MATERIALS; ++i)
	{
		if (!pkItemMaterial[i])
			continue;

		pkItemMaterial[i]->Lock(false);
		pkItemMaterial[i] = NULL;
	}
}

BYTE CHARACTER::CheckClEmptyMaterialSlot()
{
	LPITEM* pkItemMaterial;
	pkItemMaterial = GetClWindowMaterials();
	for (int i = 0; i < CL_WINDOW_MAX_MATERIALS; ++i)
	{
		if (!pkItemMaterial[i])
			return i;
	}

	return 255;
}

void CHARACTER::AddClMaterial(TItemPos tPos, BYTE bPos)
{
	if (!isChangeLookOpened())
		return;
	else if (bPos >= CL_WINDOW_MAX_MATERIALS)
	{
		if (bPos != 255)
			return;

		LPITEM pkScroll = GetItem(tPos);
		if (!pkScroll)
			return;
		if (pkScroll->GetVnum() == CL_SCROLL_VNUM)
			bPos = 2;
		else
			bPos = CheckClEmptyMaterialSlot();
		if (bPos >= CL_WINDOW_MAX_MATERIALS)
			return;
	}

#ifdef __OFFLINE_SHOP__
	if (GetOfflineShopGuest() || GetShopSafebox())// Offlineshop Update
		return;
#endif

	if (GetExchange() || GetMyShop() || GetShopOwner() || IsOpenSafebox() || IsCubeOpen() || IsUnderRefine())
		return;

#ifdef __ACCE_SYSTEM__
	if (isAcceOpened(true) || isAcceOpened(false))
		return;
#endif

	LPITEM pkItem = GetItem(tPos);
	if (!pkItem)
		return;
	if ((pkItem->GetCell() >= INVENTORY_MAX_NUM) || (tPos.IsBeltInventoryPosition()))
		return;
#ifdef __SLOT_MARKING__
	if (IS_SET(pkItem->GetAntiFlag(), ITEM_ANTIFLAG_CHANGELOOK))
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("WARNING_ANTIFLAG_CHANGELOOK"));
		return;
	}
#endif
	if (pkItem->IsEquipped())
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[Transmutation] You cannot transmute an item while it is equipped."));
		return;
	}
	if ((bPos == 2 && pkItem->GetVnum() != CL_SCROLL_VNUM) || (bPos != 2 && pkItem->GetVnum() == CL_SCROLL_VNUM))
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[Transmutation] This item cannot be transmuted."));
		return;
	}
	if (pkItem->GetVnum() != CL_SCROLL_VNUM)
	{
#ifdef __MOUNT_CHANGELOOK__
		if (m_bMountChangeLook == true)
		{
			if (!pkItem->IsMountItem())
			{
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[Transmutation] This item cannot be transmuted."));
				return;
			}
#ifdef __BEGINNER_ITEM__
			else if (pkItem->IsBasicItem())
			{
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ITEM_IS_BASIC_CANNOT_DO"));
				return;
			}
#endif
			else if (pkItem->GetTransmutation() != 0)
			{
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[Transmutation] You can't add items which are transmuted yet."));
				return;
			}
		}
		else
#endif
		{
			if ((pkItem->GetType() != ITEM_WEAPON) && (pkItem->GetType() != ITEM_ARMOR) && (pkItem->GetType() != ITEM_COSTUME))
			{
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[Transmutation] This item cannot be transmuted."));
				return;
			}
#ifdef __QUIVER_SYSTEM__
			else if ((pkItem->GetType() == ITEM_WEAPON) && ((pkItem->GetSubType() == WEAPON_ARROW) || (pkItem->GetSubType() == WEAPON_MOUNT_SPEAR) || (pkItem->GetSubType() == WEAPON_QUIVER)))
#else
			else if ((pkItem->GetType() == ITEM_WEAPON) && ((pkItem->GetSubType() == WEAPON_ARROW) || (pkItem->GetSubType() == WEAPON_MOUNT_SPEAR)))
#endif
			{
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[Transmutation] This item cannot be transmuted."));
				return;
			}
			else if ((pkItem->GetType() == ITEM_ARMOR) && (pkItem->GetSubType() != ARMOR_BODY))
			{
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[Transmutation] This item cannot be transmuted."));
				return;
			}
#ifdef __WEAPON_COSTUME_SYSTEM__
			else if ((pkItem->GetType() == ITEM_COSTUME) && (pkItem->GetSubType() != COSTUME_BODY) && (pkItem->GetSubType() != COSTUME_HAIR) && (pkItem->GetSubType() != COSTUME_WEAPON))
#else
			else if ((pkItem->GetType() == ITEM_COSTUME) && (pkItem->GetSubType() != COSTUME_BODY) && (pkItem->GetSubType() != COSTUME_HAIR))
#endif
			{
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[Transmutation] This item cannot be transmuted."));
				return;
			}
#ifdef __BEGINNER_ITEM__
			else if (pkItem->IsBasicItem())
			{
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ITEM_IS_BASIC_CANNOT_DO"));
				return;
			}
#endif
			else if (pkItem->GetTransmutation() != 0)
			{
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[Transmutation] You can't add items which are transmuted yet."));
				return;
			}
		}
	}

	LPITEM* pkItemMaterial;
	pkItemMaterial = GetClWindowMaterials();
	if ((bPos == 1) && (!pkItemMaterial[0]))
		return;

	if (pkItemMaterial[bPos])
		return;

#ifdef __MOUNT_CHANGELOOK__
	if (m_bMountChangeLook)
	{
		if (bPos == 1)
		{
			bool bStop = true;
			if (pkItem->IsMountItem() && pkItemMaterial[0]->IsMountItem())
				bStop = false;

			if (pkItemMaterial[0]->GetOriginalVnum() == pkItem->GetOriginalVnum())
				bStop = true;

			if (bStop)
			{
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[Transmutation] You cannot submit this item."));
				return;
			}
		}
	}
	else
#endif
	{
		if (bPos == 1)
		{
			bool bStop = false;
			if ((pkItem->GetType() != pkItemMaterial[0]->GetType()))
			{
				if ((pkItem->GetType() == ITEM_COSTUME && pkItem->GetSubType() == COSTUME_BODY) && (pkItemMaterial[0]->GetType() == ITEM_ARMOR && pkItemMaterial[0]->GetSubType() == ARMOR_BODY))
					bStop = false;
#ifdef __WEAPON_COSTUME_SYSTEM__
				else if ((pkItem->GetType() == ITEM_COSTUME && pkItem->GetSubType() == COSTUME_WEAPON) && (pkItemMaterial[0]->GetType() == ITEM_WEAPON))
					bStop = false;
#endif
				else if ((pkItem->GetType() == ITEM_COSTUME && pkItem->GetSubType() == COSTUME_BODY) && (pkItemMaterial[0]->GetType() == ITEM_COSTUME && pkItemMaterial[0]->GetSubType() == COSTUME_BODY))
					bStop = false;
				else if ((pkItem->GetType() == ITEM_COSTUME && pkItem->GetSubType() == COSTUME_HAIR) && (pkItemMaterial[0]->GetType() == ITEM_COSTUME && pkItemMaterial[0]->GetSubType() == COSTUME_HAIR))
					bStop = false;
#ifdef __WEAPON_COSTUME_SYSTEM__
				else if ((pkItem->GetType() == ITEM_COSTUME && pkItem->GetSubType() == COSTUME_WEAPON) && (pkItemMaterial[0]->GetType() == ITEM_COSTUME && pkItemMaterial[0]->GetSubType() == COSTUME_WEAPON))
					bStop = false;
#endif
				else if ((pkItem->GetType() == ITEM_ARMOR && pkItem->GetSubType() == ARMOR_BODY) && (pkItemMaterial[0]->GetType() == ITEM_COSTUME && pkItemMaterial[0]->GetSubType() == COSTUME_BODY))
					bStop = false;
#ifdef __WEAPON_COSTUME_SYSTEM__
				else if ((pkItem->GetType() == ITEM_WEAPON) && (pkItemMaterial[0]->GetType() == ITEM_COSTUME && pkItemMaterial[0]->GetSubType() == COSTUME_WEAPON))
					bStop = false;
#endif
#ifdef __PET_SYSTEM_PROTO__
				else if ((pkItem->IsPetItem()) && pkItemMaterial[0]->IsPetItem())
					bStop = false;
#endif
				else
					bStop = true;
			}
			else
			{
				if (pkItemMaterial[0]->GetType() != pkItem->GetType())
					bStop = true;
				else if (pkItemMaterial[0]->GetSubType() != pkItem->GetSubType())
					bStop = true;
			}

			if (pkItemMaterial[0]->GetOriginalVnum() == pkItem->GetOriginalVnum())
				bStop = true;
			else if (((IS_SET(pkItemMaterial[0]->GetAntiFlag(), ITEM_ANTIFLAG_FEMALE)) && (!IS_SET(pkItem->GetAntiFlag(), ITEM_ANTIFLAG_FEMALE))) || ((IS_SET(pkItemMaterial[0]->GetAntiFlag(), ITEM_ANTIFLAG_MALE)) && (!IS_SET(pkItem->GetAntiFlag(), ITEM_ANTIFLAG_MALE))))
				bStop = true;
			else if ((pkItem->GetAntiFlag() & ITEM_ANTIFLAG_WARRIOR) && (!IS_SET(pkItemMaterial[0]->GetAntiFlag(), ITEM_ANTIFLAG_WARRIOR)))
				bStop = true;
			else if ((pkItem->GetAntiFlag() & ITEM_ANTIFLAG_ASSASSIN) && (!IS_SET(pkItemMaterial[0]->GetAntiFlag(), ITEM_ANTIFLAG_ASSASSIN)))
				bStop = true;
			else if ((pkItem->GetAntiFlag() & ITEM_ANTIFLAG_SHAMAN) && (!IS_SET(pkItemMaterial[0]->GetAntiFlag(), ITEM_ANTIFLAG_SHAMAN)))
				bStop = true;
			else if ((pkItem->GetAntiFlag() & ITEM_ANTIFLAG_SURA) && (!IS_SET(pkItemMaterial[0]->GetAntiFlag(), ITEM_ANTIFLAG_SURA)))
				bStop = true;
#ifdef __WOLFMAN_CHARACTER__
			else if ((pkItem->GetAntiFlag() & ITEM_ANTIFLAG_WOLFMAN) && (!IS_SET(pkItemMaterial[0]->GetAntiFlag(), ITEM_ANTIFLAG_WOLFMAN)))
				bStop = true;
#endif
#ifdef __WEAPON_COSTUME_SYSTEM__
			else if ((pkItem->GetType() == ITEM_COSTUME && pkItemMaterial[0]->GetType() == ITEM_COSTUME) && ((pkItem->GetType() == ITEM_COSTUME && pkItem->GetSubType() == COSTUME_WEAPON) && (pkItemMaterial[0]->GetType() == ITEM_COSTUME && pkItemMaterial[0]->GetSubType() == COSTUME_WEAPON)))
			{
				if (pkItem->GetValue(3) != pkItemMaterial[0]->GetValue(3))
					bStop = true;
			}
			else if ((pkItem->GetType() == ITEM_WEAPON && pkItemMaterial[0]->GetType() == ITEM_COSTUME) && ((pkItem->GetType() == ITEM_WEAPON && ((pkItemMaterial[0]->GetType() == ITEM_COSTUME && pkItemMaterial[0]->GetSubType() == COSTUME_WEAPON)))))
			{
				if (pkItem->GetType() != pkItemMaterial[0]->GetValue(3))
					bStop = true;
			}
			else if ((pkItem->GetType() == ITEM_COSTUME && pkItemMaterial[0]->GetType() == ITEM_WEAPON) && ((pkItem->GetType() == ITEM_COSTUME && pkItem->GetSubType() == COSTUME_WEAPON) && ((pkItemMaterial[0]->GetType() == ITEM_WEAPON))))
			{
				if (pkItem->GetValue(3) != pkItemMaterial[0]->GetType())
					bStop = true;
			}
#endif

			if (bStop)
			{
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[Transmutation] You cannot submit this item."));
				return;
			}
		}
	}

	pkItemMaterial[bPos] = pkItem;
	pkItemMaterial[bPos]->Lock(true);

	TPacketChangeLook sPacket;
	sPacket.header = HEADER_GC_CL;
	sPacket.subheader = CL_SUBHEADER_ADD;
#ifdef __GOLD_LIMIT_REWORK__
	sPacket.llCost = 0;
#else
	sPacket.dwCost = 0;
#endif
	sPacket.bPos = bPos;
	sPacket.tPos = tPos;
	GetDesc()->Packet(&sPacket, sizeof(TPacketChangeLook));
}

void CHARACTER::RemoveClMaterial(BYTE bPos)
{
	if (bPos >= CL_WINDOW_MAX_MATERIALS)
		return;

	LPITEM* pkItemMaterial;
	pkItemMaterial = GetClWindowMaterials();

	if (!pkItemMaterial[bPos])
		return;

	if (bPos == 1 || bPos == 2)
	{
		pkItemMaterial[bPos]->Lock(false);
		pkItemMaterial[bPos] = NULL;
	}
	else
		ClearClWindowMaterials();

	TItemPos tPos;
	tPos.window_type = INVENTORY;
	tPos.cell = 0;

	TPacketChangeLook sPacket;
	sPacket.header = HEADER_GC_CL;
	sPacket.subheader = CL_SUBHEADER_REMOVE;
#ifdef __GOLD_LIMIT_REWORK__
	sPacket.llCost = 0;
#else
	sPacket.dwCost = 0;
#endif
	sPacket.bPos = bPos;
	sPacket.tPos = tPos;
	GetDesc()->Packet(&sPacket, sizeof(TPacketChangeLook));
}

void CHARACTER::RefineClMaterials()
{
	if (GetExchange() || GetMyShop() || GetShopOwner() || IsOpenSafebox() || IsCubeOpen() || IsUnderRefine())
		return;
#ifdef __OFFLINE_SHOP__
	if (GetOfflineShopGuest() || GetShopSafebox())// Offlineshop Update
		return;
#endif
#ifdef __ACCE_SYSTEM__
	if (isAcceOpened(true) || isAcceOpened(false))
		return;
#endif
	LPITEM* pkItemMaterial;
	pkItemMaterial = GetClWindowMaterials();
	if (!pkItemMaterial[0])
		return;
	else if (!pkItemMaterial[1])
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[Transmutation] Please submit the item you want to transmute."));
		return;
	}

#ifdef __GOLD_LIMIT_REWORK__
	long long llPrice = CL_TRANSMUTATION_PRICE;
	bool isNeedGold = true;
	if (pkItemMaterial[2] && pkItemMaterial[2]->GetVnum() == CL_SCROLL_VNUM)
		isNeedGold = false;
	if (isNeedGold == true)
	{
		if (GetGold() < llPrice)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[Transmutation] You don't have enough Yang."));
			return;
		}
	}
#else
	DWORD dwPrice = CL_TRANSMUTATION_PRICE;
	bool isNeedGold = true;
	if (pkItemMaterial[2] && pkItemMaterial[2]->GetVnum() == CL_SCROLL_VNUM)
		isNeedGold = false;
	if (isNeedGold == true)
	{
		if (GetGold() < dwPrice)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[Transmutation] You don't have enough Yang."));
			return;
		}
	}
#endif

	DWORD dwVnum = pkItemMaterial[1]->GetVnum();
	if (isNeedGold == false)
		ITEM_MANAGER::instance().RemoveItem(pkItemMaterial[2], "SPECIFY ITEM TRANSMUTED");
	else
#ifdef __GOLD_LIMIT_REWORK__
		PointChange(POINT_GOLD, -llPrice);
#else
		PointChange(POINT_GOLD, -dwPrice);
#endif
	SetChangeLookTime();

	ITEM_MANAGER::instance().RemoveItem(pkItemMaterial[1], "TRANSMUTED (SUCCESSFULLY)");

	pkItemMaterial[0]->SetTransmutation(dwVnum, true);
	ClearClWindowMaterials();

	TItemPos tPos;
	tPos.window_type = INVENTORY;
	tPos.cell = 0;

	TPacketChangeLook sPacket;
	sPacket.header = HEADER_GC_CL;
	sPacket.subheader = CL_SUBHEADER_REFINE;
#ifdef __GOLD_LIMIT_REWORK__
	sPacket.llCost = 0;
#else
	sPacket.dwCost = 0;
#endif
	sPacket.bPos = 0;
	sPacket.tPos = tPos;
	GetDesc()->Packet(&sPacket, sizeof(TPacketChangeLook));
}

bool CHARACTER::CleanTransmutation(LPITEM pkItem, LPITEM pkTarget)
{
	if (!CanHandleItem())
		return false;
	else if ((!pkItem) || (!pkTarget))
		return false;
	else if ((pkTarget->GetType() != ITEM_WEAPON) && (pkTarget->GetType() != ITEM_ARMOR) && (pkTarget->GetType() != ITEM_COSTUME))
		return false;
	else if (pkTarget->isLocked())
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You can't remove the transmute because item is locked."));
		return false;
	}
#ifdef __BEGINNER_ITEM__
	else if (pkTarget->IsBasicItem())
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ITEM_IS_BASIC_CANNOT_DO"));
		return false;
	}
#endif

	if (pkTarget->GetTransmutation() == 0)
		return false;

	pkTarget->SetTransmutation(0);
	pkItem->SetCount(pkItem->GetCount() - 1);
	SetChangeLookTime();
	return true;
}
#endif


bool CHARACTER::IsHizli()
{
	if (IsPC())
		return false;

	switch(GetRaceNum())
	{
		case 2065:
		case 2415:
		case 2066:
		case 2414:
			return true;
		break;
	}

	return false;
}

bool CHARACTER::IsBoss()
{
	if (IsPC())
		return false;

	return IsRaceFlag(RACE_FLAG_BOSS);
}

bool CHARACTER::IsBoss() const
{
	if (IsPC())
		return false;

	return IsRaceFlag(RACE_FLAG_BOSS);
}

#ifdef __PET_SYSTEM_PROTO__
void CHARACTER::CheckPet()
{
	if (CWarMapManager::instance().IsWarMap(GetMapIndex()))
		return;

	LPITEM item = GetWear(WEAR_PET);
	if (item)
	{
		if (!item->IsPetItem())
			return;
		else
		{
			CPetSystem* petSystem = GetPetSystem();
			if (petSystem)
				petSystem->Summon(item->GetValue(0), item, 0, false);
		}
	}
	else
		return;
}
#endif

#ifdef __SKILL_COLOR__
void CHARACTER::SetSkillColor(DWORD* dwSkillColor)
{
	memcpy(m_dwSkillColor, dwSkillColor, sizeof(m_dwSkillColor));
	UpdatePacket();
}
#endif

#ifdef __OFFLINE_SHOP__
bool CHARACTER::CanTakeInventoryItem(LPITEM item, TItemPos* cell)
{
	int iEmpty = -1;
	if (item->IsDragonSoul())
	{
		cell->window_type = DRAGON_SOUL_INVENTORY;
		cell->cell = iEmpty = GetEmptyDragonSoulInventory(item);
	}
#ifdef __ADDITIONAL_INVENTORY__
	else if (item->IsUpgradeItem())
	{
		cell->window_type = UPGRADE_INVENTORY;
		cell->cell = iEmpty = GetEmptyUpgradeInventory(item);
	}
	else if (item->IsBook())
	{
		cell->window_type = BOOK_INVENTORY;
		cell->cell = iEmpty = GetEmptyBookInventory(item);
	}
	else if (item->IsStone())
	{
		cell->window_type = STONE_INVENTORY;
		cell->cell = iEmpty = GetEmptyStoneInventory(item);
	}
	else if (item->IsFlower())
	{
		cell->window_type = FLOWER_INVENTORY;
		cell->cell = iEmpty = GetEmptyFlowerInventory(item);
	}
	else if (item->IsAttrItem())
	{
		cell->window_type = ATTR_INVENTORY;
		cell->cell = iEmpty = GetEmptyAttrInventory(item);
	}
	else if (item->IsChest())
	{
		cell->window_type = CHEST_INVENTORY;
		cell->cell = iEmpty = GetEmptyChestInventory(item);
	}
#endif
	else
	{
		cell->window_type = INVENTORY;
		cell->cell = iEmpty = GetEmptyInventory(item->GetSize());
	}

	return iEmpty != -1;
}
#endif

// @duzenleme karakter safezonedemi diye bakiyor bunun aslinda ifdefsiz bolume tasinmasi gerekli gibi.
bool CHARACTER::IsInSafezone() const
{
	LPSECTREE sectree = GetSectree();
	return (sectree && sectree->IsAttr(GetX(), GetY(), ATTR_BANPK));
}

#ifdef __ALIGNMENT_REWORK__
int CHARACTER::GetAlignGrade()
{
	int iAlignIndex;

	if (GetRealAlignment() >=      200000000)
		iAlignIndex = 0;
	else if (GetRealAlignment() >= 120000000)
		iAlignIndex = 1;
	else if (GetRealAlignment() >= 110000000)
		iAlignIndex = 2;
	else if (GetRealAlignment() >= 100000000)
		iAlignIndex = 3;
	else if (GetRealAlignment() >= 90000000)
		iAlignIndex = 4;
	else if (GetRealAlignment() >= 80000000)
		iAlignIndex = 5;
	else if (GetRealAlignment() >= 70000000)
		iAlignIndex = 6;
	else if (GetRealAlignment() >= 60000000)
		iAlignIndex = 7;
	else if (GetRealAlignment() >= 50000000)
		iAlignIndex = 8;
	else if (GetRealAlignment() >= 40000000)
		iAlignIndex = 9;
	else if (GetRealAlignment() >= 30000000)
		iAlignIndex = 10;
	else if (GetRealAlignment() >= 20000000)
		iAlignIndex = 11;
	else if (GetRealAlignment() >= 10000000)
		iAlignIndex = 12;
	else if (GetRealAlignment() >= 2000000)
		iAlignIndex = 13;
	else if (GetRealAlignment() >= 1800000)
		iAlignIndex = 14;
	else if (GetRealAlignment() >= 1600000)
		iAlignIndex = 15;
	else if (GetRealAlignment() >= 1400000)
		iAlignIndex = 16;
	else if (GetRealAlignment() >= 1200000)
		iAlignIndex = 17;
	else if (GetRealAlignment() >= 1000000)
		iAlignIndex = 18;
	else if (GetRealAlignment() >= 900000)
		iAlignIndex = 19;
	else if (GetRealAlignment() >= 800000)
		iAlignIndex = 20;
	else if (GetRealAlignment() >= 700000)
		iAlignIndex = 21;
	else if (GetRealAlignment() >= 600000)
		iAlignIndex = 22;
	else if (GetRealAlignment() >= 500000)
		iAlignIndex = 23;
	else if (GetRealAlignment() >= 400000)
		iAlignIndex = 24;
	else if (GetRealAlignment() >= 300000)
		iAlignIndex = 25;
	else if (GetRealAlignment() >= 200000)
		iAlignIndex = 26;
	else if (GetRealAlignment() >= 120000)
		iAlignIndex = 27;
	else if (GetRealAlignment() >= 80000)
		iAlignIndex = 28;
	else if (GetRealAlignment() >= 40000)
		iAlignIndex = 29;
	else if (GetRealAlignment() >= 10000)
		iAlignIndex = 30;
	else if (GetRealAlignment() >= 0)
		iAlignIndex = 31;
	else if (GetRealAlignment() > -40000)
		iAlignIndex = 32;
	else if (GetRealAlignment() > -80000)
		iAlignIndex = 33;
	// else if (GetRealAlignment() > -120000)
		// iAlignIndex = 34;
	else
		iAlignIndex = 34;

	return iAlignIndex;
}
void CHARACTER::RefreshAlignBonus()
{
	// Remove all bonuses
	for (size_t i = 0; i < ALIGN_BONUS_COUNT; i++)
		RemoveAffect(AFFECT_ALIGN_BONUS_TYPE_1 + i);

	// Give bonus based on align grade
	int grade = GetAlignGrade();
	if (grade >= (19+12) || grade < 0) // 19 = 0
		return;

	for (size_t i = 0; i < ALIGN_BONUS_COUNT; i++)
	{
		if (cAlignBonusTable[grade + 1][i] == 0)
			break;
		ADD_AFFECT_INFINITY(AFFECT_ALIGN_BONUS_TYPE_1 + i, cAlignBonusTable[0][i], cAlignBonusTable[grade + 1][i]);
	}
}
#endif

#ifdef __RANK_SYSTEM__
void CHARACTER::RefreshRankBonus()
{
	// Remove all bonuses
	for (size_t i = 0; i < RANK_BONUS_COUNT; i++)
		RemoveAffect(AFFECT_RANK_BONUS_TYPE_1 + i);

	// Give bonus based on rank
	int rank = GetRank();
	if (rank == 0)
		return;

	for (size_t i = 0; i < RANK_BONUS_COUNT; i++)
	{
		if (cRankBonusTable[rank][i] == 0)
			break;
		ADD_AFFECT_INFINITY(AFFECT_RANK_BONUS_TYPE_1 + i, cRankBonusTable[0][i], cRankBonusTable[rank][i]);
	}
}
#endif

#ifdef __TEAM_SYSTEM__
void CHARACTER::RefreshTeamBonus()
{
	// Remove all bonuses
	for (size_t i = 0; i < 5; i++)
		RemoveAffect(AFFECT_TEAM_BONUS_TYPE_1 + i);

	if (GetTeam() == 0)
		return;

	ADD_AFFECT_INFINITY(AFFECT_TEAM_BONUS_TYPE_1, POINT_MALL_ATTBONUS, 20);
}
#endif

#ifdef __LANDRANK_SYSTEM__
void CHARACTER::RefreshLandRankBonus()
{
	// Remove all bonuses
	for (size_t i = 0; i < LANDRANK_BONUS_COUNT; i++)
		RemoveAffect(AFFECT_LANDRANK_BONUS_TYPE_1 + i);

	// Give bonus based on landrank
	int rank = GetLandRank();
	if (rank == 0)
		return;

	/* Converting landrank to usable: 267 -> 2
	(int) 267/100
	*/
	rank = (int)rank / 100;

	for (size_t i = 0; i < LANDRANK_BONUS_COUNT; i++)
	{
		if (cLandRankBonusTable[rank + 1][i] == 0)
			break;
		ADD_AFFECT_INFINITY(AFFECT_LANDRANK_BONUS_TYPE_1 + i, cLandRankBonusTable[0][i], cLandRankBonusTable[rank + 1][i]);
	}
}
#endif

#ifdef __REBORN_SYSTEM__
void CHARACTER::RefreshRebornBonus()
{
	// Remove all bonuses
	for (size_t i = 0; i < REBORN_BONUS_COUNT; i++)
		RemoveAffect(AFFECT_REBORN_BONUS_TYPE_1 + i);

	// Give bonus based on reborn
	int reborn = GetReborn();
	if (reborn == 0)
		return;

	for (size_t i = 0; i < REBORN_BONUS_COUNT; i++)
	{
		if (cRebornBonusTable[reborn][i] == 0)
			break;
		ADD_AFFECT_INFINITY(AFFECT_REBORN_BONUS_TYPE_1 + i, cRebornBonusTable[0][i], cRebornBonusTable[reborn][i]);
	}
}
#endif

#ifdef __MONIKER_SYSTEM__
void CHARACTER::RefreshMonikerBonus()
{
	// Remove all bonuses
	for (size_t i = 0; i < 5; i++)
		RemoveAffect(AFFECT_MONIKER_BONUS_TYPE_1 + i);

	if (strlen(GetMoniker()) == 0)
		return;

	ADD_AFFECT_INFINITY(AFFECT_MONIKER_BONUS_TYPE_1, POINT_MALL_ATTBONUS, 20);
}
#endif

#ifdef ENABLE_LOVE_SYSTEM
void CHARACTER::RefreshLoveBonus()
{
	// Remove all bonuses
	RemoveAffect(AFFECT_LOVE);

	if (strlen(GetLove1()) == 0)
		return;

	ADD_AFFECT_INFINITY(AFFECT_LOVE, POINT_NORMAL_HIT_DAMAGE_BONUS, 20);
}
#endif

#ifdef ENABLE_WORD_SYSTEM
void CHARACTER::RefreshWordBonus()
{
	// Remove all bonuses
	RemoveAffect(AFFECT_WORD);

	if (strlen(GetWord()) == 0)
		return;

	ADD_AFFECT_INFINITY(AFFECT_WORD, POINT_NORMAL_HIT_DAMAGE_BONUS, 20);
}
#endif

#ifdef __BIOLOG_SYSTEM__
void CHARACTER::OpenBiologWindow()
{
	if (GetLevel() < cBiologSettings[1][0]) // if small than min level requirement
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("bioenaz%dlevel"), cBiologSettings[1][0]);
		return;
	}

	int q_level = GetQuestFlag("bio.level"); // better game-db packets

	if (q_level == 0) // starting for biologs
	{
		SetQuestFlag("bio.level", 1);
		q_level = 1;
	}
	// else if (q_level > 15) // if bigger than max level
	// {
	// 	ChatPacket(CHAT_TYPE_INFO, LC_TEXT("biobitmis"));
	// 	return;
	// }

	int q_gived = GetQuestFlag("bio.gived"); // better game-db packets
	int q_sub_level = GetQuestFlag("bio.sublevel");

	CAffect* pAff = FindAffect(AFFECT_BIO_ITEM_DURATION);
	ChatPacket(CHAT_TYPE_COMMAND, "OpenBiologWindow %d#%d#%d#%d",
		q_level, q_sub_level, q_gived, pAff ? pAff->lDuration : 0);
	// OpenBiologWindow level sublevel my_count duration
}
void CHARACTER::SetBiologItem(bool all, bool useItem1, bool useItem2, bool useItem3)
{
	if (IsDead() || IsStun())
		return;

	if (quest::CQuestManager::instance().GetPCForce(GetPlayerID())->IsRunning() == true)
		return;

	if (!CanAct(true, true, VAR_TIME_BIOLOG, nullptr, 0))
		return;

	if (FindAffect(AFFECT_BIO_PACKET_DURATION))
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Bio wait 2 sec. for click again"));
		return;
	}
	else
		AddAffect(AFFECT_BIO_PACKET_DURATION, APPLY_NONE, 0, AFF_NONE, 2, 0, false);

	int q_level = GetQuestFlag("bio.level");

	if (q_level == 0) // starting for biologs
	{
		SetQuestFlag("bio.level", 1);
		q_level = 1;
	}
	else if (q_level >= 21) // if bigger than max level
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("biobitmis"));
		return;
	}
	else if (GetLevel() < cBiologSettings[q_level][0])
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("biolevel%dolmalý"), cBiologSettings[q_level][0]);
		return;
	}

	WORD q_sub_level = GetQuestFlag("bio.sublevel");
	WORD q_gived = GetQuestFlag("bio.gived");
	WORD giving = q_sub_level == 0 ? (all ? (cBiologSettings[q_level][2] - q_gived) : 1) : (all ? (cBiologSettings[q_level][6] - q_gived) : 1);
	WORD remain = q_sub_level == 0 ? (cBiologSettings[q_level][2] - q_gived) : (cBiologSettings[q_level][6] - q_gived);

	DWORD needItem = q_sub_level == 0 ? cBiologSettings[q_level][1] : cBiologSettings[q_level][5];

	bool canContinue = true;
#ifdef __ADDITIONAL_INVENTORY__
	for (int i = 0; i < SPECIAL_INVENTORY_MAX_NUM; i++) // for all of special inventory
	{
		LPITEM item = GetUpgradeInventoryItem(i);
#else
	for (int i = 0; i < INVENTORY_MAX_NUM; i++) // for all of special inventory
	{
		LPITEM item = GetInventoryItem(i);
#endif
		if (!item)
			continue;
		if (item->GetVnum() != needItem)
			continue;
		for (int j = 0; j < giving;)
		{
			if (!item || item->GetCount() < 1)
				break;

			bool doubleAffItem = false;

			if (q_sub_level == 0)
			{
				auto waitForItemAff = FindAffect(AFFECT_BIO_ITEM_DURATION);
				if (waitForItemAff)
				{
					LPITEM use = nullptr;
					if (useItem3)
					{
#ifdef __ADDITIONAL_INVENTORY__
						use = FindSpecifyItemSpecial(BIOLOG_DOUBLE_ITEM);
#else
						use = FindSpecifyItem(BIOLOG_DOUBLE_ITEM);
#endif
					}

					if (use)
					{
						doubleAffItem = true;
					}
					else if (useItem2)
					{
#ifdef __ADDITIONAL_INVENTORY__
						use = FindSpecifyItemSpecial(BIOLOG_SURE_SIFIRLA_ITEM);
#else
						use = FindSpecifyItem(BIOLOG_SURE_SIFIRLA_ITEM);
#endif
					}

					if (use)
					{
						use->SetCount(use->GetCount() - 1);
					}
					else
					{
						canContinue = false;
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Bio wait %d min. for give another item."), waitForItemAff->lDuration / 60);
						break;
					}
				}
				if (cBiologSettings[q_level][4] > 0)
				{
					SetQuestFlag("bio.reminder", 1);
					AddAffect(AFFECT_BIO_ITEM_DURATION, APPLY_NONE, 0, AFF_NONE, cBiologSettings[q_level][4], 0, true);
				}
			}

			item->SetCount(item->GetCount() - 1);

			BYTE perc = cBiologSettings[q_level][3];

			if (q_sub_level == 0 && doubleAffItem == false)
			{
				if (useItem1)
				{
#ifdef __ADDITIONAL_INVENTORY__
					LPITEM use = FindSpecifyItemSpecial(BIOLOG_100_PERC_ITEM);
#else
					LPITEM use = FindSpecifyItem(BIOLOG_100_PERC_ITEM);
#endif
					if (use)
					{
						// sys_log(0, "used item2 name %s", GetName());
						use->SetCount(use->GetCount() - 1);
						doubleAffItem = true;
						// perc = 100;
					}
				}
				else if (useItem3)
				{
#ifdef __ADDITIONAL_INVENTORY__
					LPITEM use = FindSpecifyItemSpecial(BIOLOG_DOUBLE_ITEM);
#else
					LPITEM use = FindSpecifyItem(BIOLOG_DOUBLE_ITEM);
#endif
					if (use)
					{
						// sys_log(0, "used item3 as perc name %s", GetName());
						use->SetCount(use->GetCount() - 1);
						doubleAffItem = true;
						// perc = 100;
					}
				}
			}

			if (q_sub_level == 1 || doubleAffItem || number(1, 100) <= perc)
			{
				giving--; remain--;
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("BioBasarili"));
				sys_log(0, "%s: BioBasarili level: %d-%d (%d %d %d)", GetName(), q_level, q_sub_level, useItem1, useItem2, useItem3);
			}
			else
			{
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("BioBasarisiz"));
				sys_log(0, "%s: BioBasarisiz level: %d-%d (%d %d %d)", GetName(), q_level, q_sub_level, useItem1, useItem2, useItem3);
			}

			if (!all)
			{
				canContinue = false;
				break;
			}
		}
		if (giving == 0 || canContinue == false)
		{
			break;
		}
	}

	if (remain == 0)
	{
		SetQuestFlag("bio.gived", 0);
		RemoveAffect(AFFECT_BIO_ITEM_DURATION);
		RemoveAffect(AFFECT_BIO_PACKET_DURATION);
		if (q_sub_level == 0)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("bio tamamlandi, ruh tasina gectiniz."));
			SetQuestFlag("bio.sublevel", 1);
		}
		else
		{
			SetQuestFlag("bio.sublevel", 0);
			SetQuestFlag("bio.level", q_level + 1);

			for (size_t ii = 8; ii <= 20; ii+=2)
			{
				if (cBiologSettings[q_level][ii] == 0)
					break;
				AddAffect(AFFECT_BIO_BONUS1+(ii-8)/2, cBiologSettings[q_level][ii], cBiologSettings[q_level][ii+1], 0, INFINITE_AFFECT_DURATION, 0, false, true);
			}

			// AutoGiveItem(cBiologSettings[q_level][7]);
		}
	}
	else if (q_sub_level == 0)
	{
		TItemTable* pTable = ITEM_MANAGER::instance().GetTable(needItem);
		if (pTable)
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Bio remain %s item %d"), pTable->szLocaleName, remain);
		SetQuestFlag("bio.gived", cBiologSettings[q_level][2] - remain);
	}
	else
	{
		TItemTable* pTable = ITEM_MANAGER::instance().GetTable(needItem);
		if (pTable)
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Bio remain %s item %d"), pTable->szLocaleName, remain);
		SetQuestFlag("bio.gived", cBiologSettings[q_level][6] - remain);
	}

	OpenBiologWindow();
}

void CHARACTER::SetGrandBiolog(BYTE level)
{
	SetQuestFlag("bio.sublevel", 0);
	SetQuestFlag("bio.gived", 0);
	SetQuestFlag("bio.level", level);

	// Remove all bonuses
	for (size_t i = 0; i < 5; i++)
		RemoveAffect(AFFECT_BIO_BONUS1 + i);

	for (size_t i = 0+1; i < level; i++)
	{
		for (size_t ii = 8; ii <= 18; ii+=2)
		{
			if (cBiologSettings[i][ii] == 0)
				break;
			AddAffect(AFFECT_BIO_BONUS1+(ii-8)/2, cBiologSettings[i][ii], cBiologSettings[i][ii+1], 0, INFINITE_AFFECT_DURATION, 0, false, true);
		}
	}

}
#endif

#ifdef __GAME_MASTER_UTILITY__
bool CHARACTER::MasterCanAction(BYTE bAction)
{
	if (GetGMLevel() == GM_PLAYER)
		return true;

	switch (bAction)
	{
	case ACTION_EXCHANGE:
	case ACTION_SHOP:
	case ACTION_SAFEBOX:
	case ACTION_RISKLY_SET:
#ifdef __OFFLINE_SHOP__
	case ACTION_OFFLINE_SHOP:
#endif
	{
		if (GetGMLevel() >= GM_IMPLEMENTOR)
			return true;

		return false;
	}
	break;

	default:
		return false;
	}

	return false;
}
#endif

bool CHARACTER::CanAct(bool bSendMsg, bool bCheckTime, BYTE passVarCheckValue, LPCHARACTER chMessager, DWORD varFlags)
{
	if (bCheckTime)
	{
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
#ifdef __GROWTH_MOUNT_SYSTEM__
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
			0,
#endif
#ifdef __BIOLOG_SYSTEM__
			0,
#endif
#ifdef __INVENTORY_SORT__
			3,
#endif
		};

		const int iPulse = thecore_pulse();
		const int limit_time = PASSES_PER_SEC(5);

		if (IS_SET(varFlags, VAR_SAFEBOX))
		{
			if ((iPulse - GetSafeboxLoadTime()) < limit_time)
			{
				if (bSendMsg && chMessager == NULL)
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_OPERATION_IS_FAILED_SAFEBOX_TIME"));
				else if (chMessager != NULL && bSendMsg)
					chMessager->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_MESSAGER_OPERATION_IS_FAILED_SAFEBOX_TIME"));
				return false;
			}
		}

		if (IS_SET(varFlags, VAR_EXCHANGE))
		{
			if ((iPulse - GetExchangeTime()) < limit_time)
			{
				if (bSendMsg && chMessager == NULL)
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_OPERATION_IS_FAILED_EXCHANGE_TIME"));
				else if (chMessager != NULL && bSendMsg)
					chMessager->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_MESSAGER_OPERATION_IS_FAILED_EXCHANGE_TIME"));
				return false;
			}
		}

		if (IS_SET(varFlags, VAR_MYSHOP))
		{
			if ((iPulse - GetMyShopTime()) < limit_time)
			{
				if (bSendMsg && chMessager == NULL)
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_OPERATION_IS_FAILED_MYSHOP_TIME"));
				else if (chMessager != NULL && bSendMsg)
					chMessager->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_MESSAGER_OPERATION_IS_FAILED_MYSHOP_TIME"));
				return false;
			}
		}

		if (IS_SET(varFlags, VAR_REFINE))
		{
			if ((iPulse - GetRefineTime()) < limit_time)
			{
				if (bSendMsg && chMessager == NULL)
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_OPERATION_IS_FAILED_REFINE_TIME"));
				else if (chMessager != NULL && bSendMsg)
					chMessager->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_MESSAGER_OPERATION_IS_FAILED_REFINE_TIME"));
				return false;
			}
		}

#ifdef __ACCE_SYSTEM__
		if (IS_SET(varFlags, VAR_ACCE))
		{
			if ((iPulse - GetAcceTime()) < limit_time)
			{
				if (bSendMsg && chMessager == NULL)
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_OPERATION_IS_FAILED_ACCE_TIME"));
				else if (chMessager != NULL && bSendMsg)
					chMessager->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_MESSAGER_OPERATION_IS_FAILED_ACCE_TIME"));
				return false;
			}
		}
#endif

#ifdef __ITEM_CHANGELOOK__
		if (IS_SET(varFlags, VAR_CHANGELOOK))
		{
			if ((iPulse - GetChangeLookTime()) < limit_time)
			{
				if (bSendMsg && chMessager == NULL)
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_OPERATION_IS_FAILED_CHANGELOOK_TIME"));
				else if (chMessager != NULL && bSendMsg)
					chMessager->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_MESSAGER_OPERATION_IS_FAILED_CHANGELOOK_TIME"));
				return false;
			}
		}
#endif

#ifdef __OFFLINE_SHOP__
		if (IS_SET(varFlags, VAR_OFFLINESHOP))
		{
			if ((iPulse - GetOfflineShopTime()) < limit_time)
			{
				if (bSendMsg && chMessager == NULL)
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_OPERATION_IS_FAILED_OFFLINESHOP_TIME"));
				else if (chMessager != NULL && bSendMsg)
					chMessager->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_MESSAGER_OPERATION_IS_FAILED_OFFLINESHOP_TIME"));
				return false;
			}
		}
#endif

		for (int x = 0; x < VAR_TIME_MAX_NUM; ++x)
		{
			if (x == passVarCheckValue)
				continue;
			if ((iPulse - GetVarTime(x)) < var_limit_time[x])
			{
				if (bSendMsg && chMessager == NULL)
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_OPERATION_IS_FAILED_ANY_VAR_WINDOW_TIME"));
				else if (chMessager != NULL && bSendMsg)
					chMessager->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_MESSAGER_OPERATION_IS_FAILED_ANY_VAR_WINDOW_TIME"));
				return false;
			}
		}
	}

	if (GetExchange())
	{
		if (bSendMsg && chMessager == NULL)
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_OPERATION_IS_FAILED_EXCHANGE_DLG_OPENED"));
		else if (chMessager != NULL && bSendMsg)
			chMessager->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_MESSAGER_OPERATION_IS_FAILED_EXCHANGE_DLG_OPENED"));
		return false;
	}

	if (NULL != DragonSoul_RefineWindow_GetOpener())
	{
		if (bSendMsg && chMessager == NULL)
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_OPERATION_IS_FAILED_EXCHANGE_DLG_OPENED"));
		else if (chMessager != NULL && bSendMsg)
			chMessager->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_MESSAGER_OPERATION_IS_FAILED_EXCHANGE_DLG_OPENED"));
		return false;
	}

	if (GetMyShop())
	{
		if (bSendMsg && chMessager == NULL)
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_OPERATION_IS_FAILED_SHOPVIEW_DLG_OPENED"));
		else if (chMessager != NULL && bSendMsg)
			chMessager->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_MESSAGER_OPERATION_IS_FAILED_SHOPVIEW_DLG_OPENED"));
		return false;
	}

	if (GetShopOwner())
	{
		if (bSendMsg && chMessager == NULL)
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_OPERATION_IS_FAILED_SHOPOWNER_DLG_OPENED"));
		else if (chMessager != NULL && bSendMsg)
			chMessager->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_MESSAGER_OPERATION_IS_FAILED_SHOPOWNER_DLG_OPENED"));
		return false;
	}

	if (IsOpenSafebox())
	{
		if (bSendMsg && chMessager == NULL)
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_OPERATION_IS_FAILED_SAFEBOX_DLG_OPENED"));
		else if (chMessager != NULL && bSendMsg)
			chMessager->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_MESSAGER_OPERATION_IS_FAILED_SAFEBOX_DLG_OPENED"));
		return false;
	}

	if (IsCubeOpen())
	{
		if (bSendMsg && chMessager == NULL)
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_OPERATION_IS_FAILED_CUBE_DLG_OPENED"));
		else if (chMessager != NULL && bSendMsg)
			chMessager->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_MESSAGER_OPERATION_IS_FAILED_CUBE_DLG_OPENED"));
		return false;
	}

	if (IS_SET(varFlags, VAR_REFINE))
	{
		if (IsUnderRefine())
		{
			if (bSendMsg && chMessager == NULL)
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_OPERATION_IS_FAILED_REFINE_DLG_OPENED"));
			else if (chMessager != NULL && bSendMsg)
				chMessager->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_MESSAGER_OPERATION_IS_FAILED_REFINE_DLG_OPENED"));
			return false;
		}
	}

#ifdef __ACCE_SYSTEM__
	if (isAcceOpened(true))
	{
		if (bSendMsg && chMessager == NULL)
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_OPERATION_IS_FAILED_ACCE_COMB_DLG_OPENED"));
		else if (chMessager != NULL && bSendMsg)
			chMessager->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_MESSAGER_OPERATION_IS_FAILED_ACCE_COMB_DLG_OPENED"));
		return false;
	}

	if (isAcceOpened(false))
	{
		if (bSendMsg && chMessager == NULL)
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_OPERATION_IS_FAILED_ACCE_ABSORB_DLG_OPENED"));
		else if (chMessager != NULL && bSendMsg)
			chMessager->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_MESSAGER_OPERATION_IS_FAILED_ACCE_ABSORB_DLG_OPENED"));
		return false;
	}
#endif

#ifdef __ITEM_CHANGELOOK__
	if (isChangeLookOpened())
	{
		if (bSendMsg && chMessager == NULL)
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_OPERATION_IS_FAILED_CHANGELOOK_DLG_OPENED"));
		else if (chMessager != NULL && bSendMsg)
			chMessager->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_MESSAGER_OPERATION_IS_FAILED_CHANGELOOK_DLG_OPENED"));
		return false;
	}
#endif

#ifdef __OFFLINE_SHOP__
	if (GetOfflineShopGuest() != NULL || GetShopSafebox())
	{
		if (bSendMsg && chMessager == NULL)
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_OPERATION_IS_FAILED_OFFSHOP_DLG_OPENED"));
		else if (chMessager != NULL && bSendMsg)
			chMessager->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_MESSAGER_OPERATION_IS_FAILED_OFFSHOP_DLG_OPENED"));
		return false;
	}
#endif

	for (int x = 0; x < VAR_VAL_MAX_NUM; ++x)
	{
		if (x == passVarCheckValue)
			continue;
		if (GetVarValue(x) == true)
		{
			if (bSendMsg && chMessager == NULL)
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_OPERATION_IS_FAILED_ANY_VAR_WINDOW_OPENED"));
			else if (chMessager != NULL && bSendMsg)
				chMessager->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CH_CAN_ACT_MESSAGER_OPERATION_IS_FAILED_ANY_VAR_WINDOW_DLG_OPENED"));
			return false;
		}
	}

	return true;
}

#ifdef __MOUNT_COSTUME_SYSTEM__
void CHARACTER::MountSummon(LPITEM mountItem)
{
	if (CWarMapManager::instance().IsWarMap(GetMapIndex()))
		return;

	CMountSystem* mountSystem = GetMountSystem();
	DWORD mobVnum = 0;

	if (!mountSystem || !mountItem)
		return;

#ifdef __MOUNT_CHANGELOOK__
	TItemTable* pkItemTransmutation = ITEM_MANAGER::instance().GetTable(mountItem->GetTransmutation());
	if (pkItemTransmutation)
	{
		for (int i = 0; i < ITEM_APPLY_MAX_NUM; ++i)
		{
			BYTE bType = pkItemTransmutation->aApplies[i].bType;
			long value = pkItemTransmutation->aApplies[i].lValue;
			if (bType == APPLY_MOUNT)
				mobVnum = value;
		}
	}
	else
	{
		if (mountItem->FindApplyValue(APPLY_MOUNT) != 0)
			mobVnum = mountItem->FindApplyValue(APPLY_MOUNT);
	}
#else
	if (mountItem->FindApplyValue(APPLY_MOUNT) != 0)
		mobVnum = mountItem->FindApplyValue(APPLY_MOUNT);
#endif

	if (IsHorseRiding())
		StopRiding();

	if (GetHorse())
		HorseSummon(false);

	mountSystem->Summon(mobVnum, mountItem, false);
}

void CHARACTER::MountUnsummon(LPITEM mountItem)
{
	CMountSystem* mountSystem = GetMountSystem();
	DWORD mobVnum = 0;

	if (!mountSystem || !mountItem)
		return;
#ifdef __MOUNT_CHANGELOOK__
	TItemTable* pkItemTransmutation = ITEM_MANAGER::instance().GetTable(mountItem->GetTransmutation());
	if (pkItemTransmutation)
	{
		for (int i = 0; i < ITEM_APPLY_MAX_NUM; ++i)
		{
			BYTE bType = pkItemTransmutation->aApplies[i].bType;
			long value = pkItemTransmutation->aApplies[i].lValue;
			if (bType == APPLY_MOUNT)
				mobVnum = value;
		}
	}
	else
	{
		if (mountItem->FindApplyValue(APPLY_MOUNT) != 0)
			mobVnum = mountItem->FindApplyValue(APPLY_MOUNT);
	}
#else
	if (mountItem->FindApplyValue(APPLY_MOUNT) != 0)
		mobVnum = mountItem->FindApplyValue(APPLY_MOUNT);
#endif
	if (GetMountVnum() == mobVnum)
		mountSystem->Unmount(mobVnum);

	mountSystem->Unsummon(mobVnum);
}

void CHARACTER::CheckMount()
{
	if (CWarMapManager::instance().IsWarMap(GetMapIndex()))
		return;

	CMountSystem* mountSystem = GetMountSystem();
	LPITEM mountItem = GetWear(WEAR_COSTUME_MOUNT);
	DWORD mobVnum = 0;

	if (!mountSystem || !mountItem)
		return;
#ifdef __MOUNT_CHANGELOOK__
	TItemTable* pkItemTransmutation = ITEM_MANAGER::instance().GetTable(mountItem->GetTransmutation());
	if (pkItemTransmutation)
	{
		for (int i = 0; i < ITEM_APPLY_MAX_NUM; ++i)
		{
			BYTE bType = pkItemTransmutation->aApplies[i].bType;
			long value = pkItemTransmutation->aApplies[i].lValue;
			if (bType == APPLY_MOUNT)
				mobVnum = value;
		}
	}
	else
	{
		if (mountItem->FindApplyValue(APPLY_MOUNT) != 0)
			mobVnum = mountItem->FindApplyValue(APPLY_MOUNT);
	}
#else
	if (mountItem->FindApplyValue(APPLY_MOUNT) != 0)
		mobVnum = mountItem->FindApplyValue(APPLY_MOUNT);
#endif
	if (mountSystem->CountSummoned() == 0 && !GetMountVnum())
	{
		mountSystem->Summon(mobVnum, mountItem, false);
		mountSystem->Mount(mobVnum, mountItem);
	}
	else if (mountSystem->CountSummoned() > 0 && !GetMountVnum())
	{
		mountSystem->Mount(mobVnum, mountItem);
	}
	else
	{
		if (GetMountVnum() == mobVnum)
			mountSystem->Unmount(mobVnum);

		mountSystem->Unsummon(mobVnum);
		mountSystem->Summon(mobVnum, mountItem, false);
		mountSystem->Mount(mobVnum, mountItem);
	}

	UpdatePacket();
}

bool CHARACTER::IsRidingMount()
{
	return (GetWear(WEAR_COSTUME_MOUNT) || FindAffect(AFFECT_MOUNT));
}
#endif

#ifdef __OFFLINE_SHOP__
void CHARACTER::SetLastShopSearchPulse(void)
{
	m_iLastShopSearchPulse = thecore_pulse() + 25;
}
#endif

#ifdef __REGEN_REWORK__
LPREGEN CHARACTER::GetRegen()
{
	return m_pkRegen;
}
#endif

DWORD CHARACTER::GetPartMainWithoutCostume() const
{
	DWORD armorPart = 0;
	LPITEM pkArmor = GetWear(WEAR_BODY);
	if (!pkArmor)
		return armorPart;

#ifdef __ITEM_CHANGELOOK__
	armorPart = pkArmor->GetTransmutation() != 0 ? pkArmor->GetTransmutation() : pkArmor->GetVnum();
#else
	armorPart = pkArmor->GetVnum();
#endif // __ITEM_CHANGELOOK__
	return armorPart;
}

#ifdef __WEAPON_COSTUME_SYSTEM__
DWORD CHARACTER::GetPartWeaponWithoutCostume() const
{
	DWORD weaponPart = 0;
	LPITEM pkWeapon = GetWear(WEAR_WEAPON);
	if (!pkWeapon)
		return weaponPart;

#ifdef __ITEM_CHANGELOOK__
	weaponPart = pkWeapon->GetTransmutation() != 0 ? pkWeapon->GetTransmutation() : pkWeapon->GetVnum();
#else
	weaponPart = pkWeapon->GetVnum();
#endif // __ITEM_CHANGELOOK__
	return weaponPart;
}
#endif // __WEAPON_COSTUME_SYSTEM__

#ifdef __FAST_CHEQUE_TRANSFER__
void CHARACTER::WonExchange(BYTE bOption, DWORD wValue)
{
	if (quest::CQuestManager::instance().GetPCForce(GetPlayerID())->IsRunning() == true)
		return;

	if (!CanAct(true, true))
		return;

	if (wValue < 0)
	{
		LogManager::instance().HackLogEx(this, "Trying copy cheque CHARACTER::WonExchange");
		return;
	}

	float fMul = (bOption == WON_EXCHANGE_CG_SUBHEADER_BUY) ? 1.0f + (static_cast<float>(TAX_MUL) / 100.0f) : 1.0f;
	DWORD wVal = wValue;
	if (bOption == WON_EXCHANGE_CG_SUBHEADER_SELL)
	{
		if (static_cast<DWORD>(GetCheque()) < wVal)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("±³È¯À» ÁøÇàÇÒ ¼öÀÖ´Â ÃæºÐÇÑ ±Ý¾×ÀÌ ¾ø½À´Ï´Ù."));
			return;
		}
		if (GetGold() + static_cast<long long>(wVal) * static_cast<long long>(CHEQUE_NAME_VALUE * fMul) >= GOLD_MAX)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´ç½ÅÀÇ ±ÝÀÌ ÇÑµµ¸¦ ³Ñ¾î ¼³ °ÍÀÌ±â ¶§¹®¿¡ ±³È¯À» ÁøÇàÇÒ ¼ö ¾ø½À´Ï´Ù."));
			return;
		}

		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("±³È¯Àº ¼º°øÇß´Ù."));
		PointChange(POINT_GOLD, static_cast<long long>(wVal) * static_cast<long long>(CHEQUE_NAME_VALUE * fMul), true);
		PointChange(POINT_CHEQUE, -static_cast<long long>(wVal), true);
	}
	else if (bOption == WON_EXCHANGE_CG_SUBHEADER_BUY)
	{
		if (GetGold() < static_cast<long long>(wVal) * static_cast<long long>(CHEQUE_NAME_VALUE * fMul))
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("±³È¯À» ÁøÇàÇÒ ¾çÀÌ ÃæºÐÇÏÁö ¾Ê½À´Ï´Ù."));
			return;
		}
		if (GetCheque() + wVal > CHEQUE_MAX)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¿øÀÌ ÇÑµµ¸¦ ÃÊ°úÇÏ¹Ç·Î ±³È¯À» ÁøÇàÇÒ ¼ö ¾ø½À´Ï´Ù."));
			return;
		}

		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("±³È¯Àº ¼º°øÇß´Ù."));
		PointChange(POINT_GOLD, -(static_cast<long long>(wVal) * static_cast<long long>(CHEQUE_NAME_VALUE * fMul)), true);
		PointChange(POINT_CHEQUE, static_cast<long long>(wVal), true);
	}
	else
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÁË¼ÛÇÕ´Ï´Ù. ¹®Á¦°¡ ¹ß»ýÇß½À´Ï´Ù. ¾Ë ¼ö¾ø´Â ±³È¯ ÀÛ¾÷."));
	}
}
#endif

#ifdef ENABLE_PLAYER_STATISTICS
void CHARACTER::SendPlayerStatisticsPacket()
{
	if (!GetDesc())
		return;

	TPacketGCPlayerStatistics playerstatistics;

	playerstatistics.bHeader = HEADER_GC_PLAYER_STATISTICS;
	playerstatistics.iKilledShinsoo = GetKilledShinsoo();
	playerstatistics.iKilledChunjo = GetKilledChunjo();
	playerstatistics.iKilledJinno = GetKilledJinno();
	playerstatistics.iTotalKill = GetTotalKill();
	playerstatistics.iDuelWon = GetDuelWon();
	playerstatistics.iDuelLost = GetDuelLost();
	playerstatistics.iKilledMonster = GetKilledMonster();
	playerstatistics.iKilledStone = GetKilledStone();
	playerstatistics.iKilledBoss = GetKilledBoss();
	playerstatistics.iCompletedDungeon = GetCompletedDungeon();
	playerstatistics.iTakedFish = GetTakedFish();
	playerstatistics.iBestStoneDamage = GetBestStoneDamage();
	playerstatistics.iBestBossDamage = GetBestBossDamage();

	GetDesc()->Packet(&playerstatistics, sizeof(TPacketGCPlayerStatistics));
}
#endif

#ifdef __BATTLE_PASS_SYSTEM__
EVENTINFO(load_battle_pass_login_event_info)
{
	DWORD pid;
	DWORD count;
	char* data;

	load_battle_pass_login_event_info()
		: pid(0)
		, count(0)
		, data(0)
	{
	}
};

EVENTFUNC(load_battle_pass_login_event)
{
	load_battle_pass_login_event_info* info = dynamic_cast<load_battle_pass_login_event_info*>(event->info);

	if (info == NULL)
	{
		sys_err("load_battle_pass_login_event_info> <Factor> Null pointer");
		return 0;
	}

	DWORD dwPID = info->pid;
	LPCHARACTER ch = CHARACTER_MANAGER::instance().FindByPID(dwPID);

	if (!ch)
	{
		M2_DELETE_ARRAY(info->data);
		return 0;
	}

	LPDESC d = ch->GetDesc();

	if (!d)
	{
		M2_DELETE_ARRAY(info->data);
		return 0;
	}

	if (d->IsPhase(PHASE_HANDSHAKE) ||
		d->IsPhase(PHASE_LOGIN) ||
		d->IsPhase(PHASE_SELECT) ||
		d->IsPhase(PHASE_DEAD) ||
		d->IsPhase(PHASE_LOADING))
	{
		return PASSES_PER_SEC(1);
	}
	else if (d->IsPhase(PHASE_CLOSE))
	{
		M2_DELETE_ARRAY(info->data);
		return 0;
	}
	else if (d->IsPhase(PHASE_GAME))
	{
		//sys_log(0, "Affect Load by Event PID %d", ch->GetPlayerID());
		ch->LoadBattlePass(info->count, (TPlayerBattlePassMission*)info->data);
		M2_DELETE_ARRAY(info->data);
		return 0;
	}
	else
	{
		sys_err("input_db.cpp:quest_login_event INVALID PHASE pid %d", ch->GetPlayerID());
		M2_DELETE_ARRAY(info->data);
		return 0;
	}
}

void CHARACTER::LoadBattlePass(DWORD dwCount, TPlayerBattlePassMission* data)
{
	m_bIsLoadedBattlePass = false;

	if (!GetDesc()->IsPhase(PHASE_GAME))
	{
		++m_iBattlePassLoadFailCounter;

		if (test_server || (m_iBattlePassLoadFailCounter > 4 && m_iBattlePassLoadFailCounter < 10))
			sys_err("LoadBattlePass: Phase not PHASE_GAME, creating Event PID COUNT %d %d (fail count: %d)", GetPlayerID(), dwCount, m_iBattlePassLoadFailCounter);

		load_battle_pass_login_event_info* info = AllocEventInfo<load_battle_pass_login_event_info>();

		info->pid = GetPlayerID();
		info->count = dwCount;
		info->data = M2_NEW char[sizeof(TPlayerBattlePassMission) * dwCount];
		thecore_memcpy(info->data, data, sizeof(TPlayerBattlePassMission) * dwCount);

		event_create(load_battle_pass_login_event, info, PASSES_PER_SEC(1));

		return;
	}

	for (DWORD i = 0; i < dwCount; ++i, ++data)
	{
		TPlayerBattlePassMission* newMission = new TPlayerBattlePassMission;
		newMission->dwPlayerId = data->dwPlayerId;
		newMission->dwMissionId = data->dwMissionId;
		newMission->dwBattlePassId = data->dwBattlePassId;
		newMission->dwExtraInfo = data->dwExtraInfo;
		newMission->bCompleted = data->bCompleted;
		newMission->bIsUpdated = data->bIsUpdated;

		m_listBattlePass.push_back(newMission);
	}

	ChatPacket(CHAT_TYPE_COMMAND, "EnableBattlePassUI");
	m_bIsLoadedBattlePass = true;
}

DWORD CHARACTER::GetMissionProgress(DWORD dwMissionID, DWORD dwBattlePassID)
{
	ListBattlePassMap::iterator it = m_listBattlePass.begin();
	while (it != m_listBattlePass.end())
	{
		TPlayerBattlePassMission* pkMission = *it++;
		if (pkMission->dwMissionId == dwMissionID && pkMission->dwBattlePassId == dwBattlePassID)
			return pkMission->dwExtraInfo;
	}
	return 0;
}

bool CHARACTER::IsCompletedMission(BYTE bMissionType, DWORD dwBattlePassID)
{
	ListBattlePassMap::iterator it = m_listBattlePass.begin();
	while (it != m_listBattlePass.end())
	{
		TPlayerBattlePassMission* pkMission = *it++;
		if (pkMission->dwMissionId == bMissionType && pkMission->dwBattlePassId == dwBattlePassID)
			return (pkMission->bCompleted ? true : false);
	}
	return false;
}

DWORD CHARACTER::GetNextMissionByType(BYTE bMissionType, BYTE bEndMissionType, DWORD dwBattlePassID)
{
	bool bFound = false;
	ListBattlePassMap::iterator it = m_listBattlePass.begin();
	while (it != m_listBattlePass.end())
	{
		TPlayerBattlePassMission* pkMission = *it++;
		if (pkMission->dwBattlePassId == dwBattlePassID)
		{
			if (pkMission->dwMissionId >= bMissionType && pkMission->dwMissionId <= bEndMissionType)
			{
				if (pkMission->bCompleted == false)
					return pkMission->dwMissionId;
			}

			bFound = true;
		}
	}

	if (!bFound)
	{
		TPlayerBattlePassMission* newMission = new TPlayerBattlePassMission;
		newMission->dwPlayerId = GetPlayerID();
		newMission->dwMissionId = bMissionType;
		newMission->dwBattlePassId = dwBattlePassID;
		newMission->bCompleted = 0;
		newMission->dwExtraInfo = 0;
		newMission->bIsUpdated = 0;
		m_listBattlePass.push_back(newMission);
		return bMissionType;
	}
	else
	{
		for (int i = bMissionType; i <= bEndMissionType; ++i)
		{
			if (!IsCompletedMission(i, dwBattlePassID))
			{
				TPlayerBattlePassMission* newMission = new TPlayerBattlePassMission;
				newMission->dwPlayerId = GetPlayerID();
				newMission->dwMissionId = i;
				newMission->dwBattlePassId = dwBattlePassID;
				newMission->bCompleted = 0;
				newMission->dwExtraInfo = 0;
				newMission->bIsUpdated = 0;
				m_listBattlePass.push_back(newMission);
				return i;
			}
		}
	}

	return 0;
}

void CHARACTER::UpdateMissionProgress(DWORD dwMissionID, DWORD dwBattlePassID, DWORD dwUpdateValue, DWORD dwTotalValue, bool isOverride)
{
	if (!m_bIsLoadedBattlePass)
		return;

	if (dwBattlePassID == 0)
		return;

	// orosbu evladi fix
	if (IsCompletedMission(dwMissionID, dwBattlePassID))
		return;

	bool foundMission = false;
	DWORD dwSaveProgress = 0;

	ListBattlePassMap::iterator it = m_listBattlePass.begin();
	while (it != m_listBattlePass.end())
	{
		TPlayerBattlePassMission* pkMission = *it++;

		if (pkMission->dwMissionId == dwMissionID && pkMission->dwBattlePassId == dwBattlePassID)
		{
			pkMission->bIsUpdated = 1;

			if (isOverride)
				pkMission->dwExtraInfo = dwUpdateValue;
			else
				pkMission->dwExtraInfo += dwUpdateValue;

			if (pkMission->dwExtraInfo >= dwTotalValue)
			{
				pkMission->dwExtraInfo = dwTotalValue;
				pkMission->bCompleted = 1;

				std::string stMissionName = CBattlePass::instance().GetMissionNameByType(pkMission->dwMissionId);
				std::string stBattlePassName = CBattlePass::instance().GetBattlePassNameByID(pkMission->dwBattlePassId);
				if (dwMissionID != PLAYTIME)
					ChatPacket(CHAT_TYPE_NOTICE, LC_TEXT("BATTLEPASS_MISSION_PROGRESS_SUCCESS"));

				CBattlePass::instance().BattlePassRewardMission(this, dwMissionID, dwBattlePassID);
			}

			dwSaveProgress = pkMission->dwExtraInfo;
			foundMission = true;
			break;
		}
	}

	if (!foundMission)
	{
		TPlayerBattlePassMission* newMission = new TPlayerBattlePassMission;
		newMission->dwPlayerId = GetPlayerID();
		newMission->dwMissionId = dwMissionID;
		newMission->dwBattlePassId = dwBattlePassID;

		if (dwUpdateValue >= dwTotalValue)
		{
			newMission->dwExtraInfo = dwTotalValue;
			newMission->bCompleted = 1;

			CBattlePass::instance().BattlePassRewardMission(this, dwMissionID, dwBattlePassID);

			dwSaveProgress = dwTotalValue;
		}
		else
		{
			newMission->dwExtraInfo = dwUpdateValue;
			newMission->bCompleted = 0;

			dwSaveProgress = dwUpdateValue;
		}

		newMission->bIsUpdated = 1;

		m_listBattlePass.push_back(newMission);
	}

	if (!GetDesc())
		return;

	DWORD dwMissionInfo = 0;
	DWORD dwUnUsedInfo = 0;
	CBattlePass::instance().BattlePassMissionGetInfo(dwBattlePassID, dwMissionID, &dwMissionInfo, &dwUnUsedInfo);


	TPacketGCBattlePassUpdate packet;
	packet.bHeader = HEADER_GC_BATTLE_PASS_UPDATE;
	packet.bMissionType = dwMissionID;
	packet.dwNewProgress = dwSaveProgress;
	GetDesc()->Packet(&packet, sizeof(TPacketGCBattlePassUpdate));
}

void CHARACTER::SkipBattlePassMission(DWORD dwMissionID, DWORD dwBattlePassID)
{
	if (!m_bIsLoadedBattlePass)
		return;

	bool foundMission = false;

	DWORD dwMissionInfo = 0;
	DWORD dwTotalValue = 0;
	CBattlePass::instance().BattlePassMissionGetInfo(dwBattlePassID, dwMissionID, &dwMissionInfo, &dwTotalValue);

	ListBattlePassMap::iterator it = m_listBattlePass.begin();
	while (it != m_listBattlePass.end())
	{
		TPlayerBattlePassMission* pkMission = *it++;

		if (pkMission->dwMissionId == dwMissionID && pkMission->dwBattlePassId == dwBattlePassID)
		{
			pkMission->bIsUpdated = 1;
			pkMission->bCompleted = 1;
			pkMission->dwExtraInfo = dwTotalValue;
			ChatPacket(CHAT_TYPE_NOTICE, LC_TEXT("BATTLE_PASS_MISSION_SKIPPED_SUCCESS"));
			CBattlePass::instance().BattlePassRewardMission(this, dwMissionID, dwBattlePassID);
			foundMission = true;
			break;
		}
	}

	if (!foundMission)
	{
		TPlayerBattlePassMission* newMission = new TPlayerBattlePassMission;
		newMission->dwPlayerId = GetPlayerID();
		newMission->dwMissionId = dwMissionID;
		newMission->dwBattlePassId = dwBattlePassID;
		newMission->bIsUpdated = 1;
		newMission->bCompleted = 1;
		newMission->dwExtraInfo = dwTotalValue;
		CBattlePass::instance().BattlePassRewardMission(this, dwMissionID, dwBattlePassID);
		m_listBattlePass.push_back(newMission);
	}

	if (!GetDesc())
		return;

	TPacketGCBattlePassUpdate packet;
	packet.bHeader = HEADER_GC_BATTLE_PASS_UPDATE;
	packet.bMissionType = dwMissionID;
	packet.dwNewProgress = dwTotalValue;
	GetDesc()->Packet(&packet, sizeof(TPacketGCBattlePassUpdate));
}

BYTE CHARACTER::GetBattlePassId()
{
	CAffect* pAffect = FindAffect(AFFECT_BATTLE_PASS_PREMIUM, POINT_BATTLE_PASS_ID);

	if (!pAffect)
	{
		pAffect = FindAffect(AFFECT_BATTLE_PASS, POINT_BATTLE_PASS_ID);
		if (!pAffect)
			return 0;
	}

	return pAffect->lApplyValue;
}

int CHARACTER::GetSecondsTillNextWeek()
{
	auto getNumberOfDays = [&](int month, int year) {
		//leap year condition, if month is 2
		if( month == 2)
		{
			if((year%400==0) || (year%4==0 && year%100!=0))
				return 29;
			else
				return 28;
		}
		//months which has 31 days
		else if(month == 1 || month == 3 || month == 5 || month == 7 || month == 8 ||month == 10 || month==12)
			return 31;

		return 30;
	};
	time_t iTime;
	time(&iTime);
	struct tm endTime = *localtime(&iTime);

	int iCurrentYear = endTime.tm_year;

	int iCurrentMonth = endTime.tm_mon;

	int iStartOfWeek = endTime.tm_mday - endTime.tm_wday;

	int iEndMDay = iStartOfWeek+7;

	int iTotalDaysCount = getNumberOfDays(iCurrentMonth, iCurrentYear);

	endTime.tm_hour = 0;
	endTime.tm_min = 0;
	endTime.tm_sec = 0;
	endTime.tm_mday = iEndMDay; // next week
	endTime.tm_wday = 0;
	endTime.tm_mon = endTime.tm_mon;
	endTime.tm_year = endTime.tm_year;

	if (iEndMDay > iTotalDaysCount)
	{
		if (iCurrentMonth == 12) {
			endTime.tm_mon = 0;
			endTime.tm_year = endTime.tm_year + 1;
		} else {
			endTime.tm_mon = endTime.tm_mon+1;
		}
		endTime.tm_mday = (iEndMDay - iTotalDaysCount)-1;
	}

	int seconds = difftime(mktime(&endTime), iTime);

	return seconds;
}

bool CHARACTER::IsBetaMapBoss()
{
	switch (GetRaceNum())
	{
		case 3890:
		case 3891:
		case 3390:
		case 3391:
		case 3595:
		case 3596:
		case 3790:
		case 3791:
		case 3190:
		case 3191:
			return true;
	}
	return false;
}

bool CHARACTER::IsDungeonBoss()
{
	switch (GetRaceNum())
	{
		case 4383:
		case 4385:
		case 2701:
		case 9515:
		case 4335:
		case 4352:
		case 4311:
		case 4398:
		case 4360:
		case 950:
		case 719:
		case 4011:
		case 4103:
		case 4095:
		case 64158:
		case 997:
		case 283:
		case 4031:
		case 1206:
		case 816:
		case 4081:
		case 64110:
		case 472:
		case 4307:
		case 4089:
		case 4061:
		case 4070:
		case 292:
		case 4043:
		case 676:
		case 4060:
		case 768:
		case 8600:
		case 880:
		case 765:
		case 879:
			return true;
	}
	return false;
}

void CHARACTER::SaveBattlePass()
{
	if (!m_bIsLoadedBattlePass)
		return;

	std::vector<TPlayerBattlePassMission> pVec(m_listBattlePass.size());
	DWORD count = 0;
	TPlayerBattlePassMission* p;

	itertype(m_listBattlePass) it = m_listBattlePass.begin();

	while (it != m_listBattlePass.end())
	{
		TPlayerBattlePassMission* pkMission = *it++;

		p = &pVec.at(count++);

		p->dwPlayerId = GetPlayerID();
		p->dwBattlePassId = pkMission->dwBattlePassId;
		p->dwMissionId = pkMission->dwMissionId;
		p->dwExtraInfo = pkMission->dwExtraInfo;
		p->bCompleted = pkMission->bCompleted;
		p->bIsUpdated = pkMission->bIsUpdated;
	}

	if (count)
	{
		db_clientdesc->DBPacket(HEADER_GD_SAVE_BATTLE_PASS, 0, &pVec[0], sizeof(TPlayerBattlePassMission) * count);
	}
}
#endif

#ifdef ENABLE_CHAT_COLOR_SYSTEM
BYTE CHARACTER::GetChatColor() const
{
	auto pkAffect = FindAffect(AFFECT_PB2_GLOBAL_CHAT);
	if (!pkAffect)
		return 0;

	return m_points.color;
}
#endif // ENABLE_CHAT_COLOR_SYSTEM

#ifdef ENABLE_MULTI_FARM_BLOCK
void CHARACTER::SetProtectTime(const std::string& flagname, int value)
{
	itertype(m_protection_Time) it = m_protection_Time.find(flagname);
	if (it != m_protection_Time.end())
		it->second = value;
	else
		m_protection_Time.insert(make_pair(flagname, value));
}

int CHARACTER::GetProtectTime(const std::string& flagname) const
{
	itertype(m_protection_Time) it = m_protection_Time.find(flagname);
	if (it != m_protection_Time.end())
		return it->second;

	return 0;
}
#endif
