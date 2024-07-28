#include "stdafx.h"

#include "config.h"
#include "char.h"
#include "char_manager.h"
#include "affect.h"
#include "packet.h"
#include "buffer_manager.h"
#include "desc_client.h"
#include "battle.h"
#include "guild.h"
#include "utils.h"
#include "locale_service.h"
#include "lua_incl.h"
#include "horsename_manager.h"
#include "item.h"
#include "DragonSoul.h"
#include "war_map.h"
#include "log.h"
#include "db.h"
#ifdef __BATTLE_PASS_SYSTEM__
#include "battle_pass.h"
#endif

//DRAGON_BONE
#define IS_NO_SAVE_AFFECT(type) \
	((type) == AFFECT_WAR_FLAG || \
	(type) == AFFECT_REVIVE_INVISIBLE || \
	((type) >= AFFECT_PREMIUM_START && (type) <= AFFECT_PREMIUM_END) || \
	(type) == AFFECT_DBONE_1 || \
	(type) == AFFECT_DBONE_2 || \
	(type) == AFFECT_NEW_PET_DURATION || \
	(type) == AFFECT_NEW_MOUNT_DURATION || \
	(type) == AFFECT_OTOAV_EFFECT || \
	(type) == AFFECT_MOUNT || \
	(type) == AFFECT_MOUNT_BONUS)

#define IS_NO_CLEAR_ON_DEATH_AFFECT(type) \
	((type) == AFFECT_BLOCK_CHAT || \
	((type) >= AFFECT_PREMIUM_START && (type) <= AFFECT_RAMADAN_ABILITY) || \
	(type) == AFFECT_MOV_SPEED || \
	(type) == AFFECT_ATT_SPEED || \
	((type) >= AFFECT_RAMADAN_RING && (type) <= AFFECT_SKILL_BOOK_BONUS2) || \
	((type) >= AFFECT_BIO_BONUS1 && (type) <= AFFECT_BIO_BONUS8) || \
	((type) >= AFFECT_FISH_EXP_BONUS && (type) <= AFFECT_FISH_APPLY_DEF_GRADE) || \
	((type) >= AFFECT_SKILL_BOOK_BONUS_EXPERT && (type) <= AFFECT_RAMADAN_NEW8) || \
	((type) >= AFFECT_SEKILL_SET_BONUS_1 && (type) <= AFFECT_SEKILL_SET_BONUS_6) || \
	((type) >= AFFECT_ADAMSINIZ && (type) <= AFFECT_BASLANGIC_ESYA))

void SendAffectRemovePacket(LPDESC d, DWORD pid, DWORD type, BYTE point)
{
	TPacketGCAffectRemove ptoc;
	ptoc.bHeader = HEADER_GC_AFFECT_REMOVE;
	ptoc.dwType = type;
	ptoc.bApplyOn = point;
	d->Packet(&ptoc, sizeof(TPacketGCAffectRemove));

	TPacketGDRemoveAffect ptod;
	ptod.dwPID = pid;
	ptod.dwType = type;
	ptod.bApplyOn = point;
	db_clientdesc->DBPacket(HEADER_GD_REMOVE_AFFECT, 0, &ptod, sizeof(ptod));

}

void SendAffectAddPacket(LPDESC d, CAffect* pkAff, bool byLoad)
{
	TPacketGCAffectAdd ptoc;
	ptoc.bHeader = HEADER_GC_AFFECT_ADD;
	ptoc.bByLoad = byLoad;
	ptoc.elem.dwType = pkAff->dwType;
	ptoc.elem.bApplyOn = pkAff->bApplyOn;
	ptoc.elem.lApplyValue = pkAff->lApplyValue;
	ptoc.elem.dwFlag = pkAff->dwFlag;
	ptoc.elem.lDuration = pkAff->lDuration;
	ptoc.elem.lSPCost = pkAff->lSPCost;
	d->Packet(&ptoc, sizeof(TPacketGCAffectAdd));
}
////////////////////////////////////////////////////////////////////
// Affect
CAffect* CHARACTER::FindAffect(DWORD dwType, BYTE bApply) const
{
	itertype(m_list_pkAffect) it = m_list_pkAffect.begin();

	while (it != m_list_pkAffect.end())
	{
		CAffect* pkAffect = *it++;
		if (!pkAffect)// @duzenleme affect varmi diye bakiyoruz ama buradan yapilmamasi lazim.
			continue;

		if (pkAffect->dwType == dwType && (bApply == APPLY_NONE || bApply == pkAffect->bApplyOn))
			return pkAffect;
	}

	return NULL;
}

#ifdef __REMOVE_SKILL_AFFECT__
CAffect* CHARACTER::FindAffectByFlag(DWORD dwFlag) const
{
	itertype(m_list_pkAffect) it = m_list_pkAffect.begin();

	while (it != m_list_pkAffect.end())
	{
		CAffect* pkAffect = *it++;

		if (!pkAffect)// @duzenleme affect varmi diye bakiyoruz ama buradan yapilmamasi lazim.
			continue;

		if (pkAffect->dwFlag == dwFlag)
			return pkAffect;
	}

	return NULL;
}
#endif

EVENTFUNC(affect_event)
{
	char_event_info* info = dynamic_cast<char_event_info*>(event->info);

	if (info == NULL)
	{
		sys_err("affect_event> <Factor> Null pointer");
		return 0;
	}

	LPCHARACTER ch = info->ch;

	if (ch == NULL) { // <Factor>
		return 0;
	}

	if (!ch->UpdateAffect())
		return 0;
	else
		return passes_per_sec;
}

bool CHARACTER::UpdateAffect()
{
	if (GetPoint(POINT_HP_RECOVERY) > 0)
	{
		if (GetMaxHP() <= GetHP())
			PointChange(POINT_HP_RECOVERY, -GetPoint(POINT_HP_RECOVERY));
		else
		{
			HPTYPE iVal = MIN(GetPoint(POINT_HP_RECOVERY), GetMaxHP() * 7 / 100);

			PointChange(POINT_HP, iVal);
			PointChange(POINT_HP_RECOVERY, -iVal);
		}
	}

	if (GetPoint(POINT_SP_RECOVERY) > 0)
	{
		if (GetMaxSP() <= GetSP())
			PointChange(POINT_SP_RECOVERY, -GetPoint(POINT_SP_RECOVERY));
		else
		{
			int iVal = MIN(GetPoint(POINT_SP_RECOVERY), GetMaxSP() * 7 / 100);

			PointChange(POINT_SP, iVal);
			PointChange(POINT_SP_RECOVERY, -iVal);
		}
	}

	if (GetPoint(POINT_HP_RECOVER_CONTINUE) > 0)
		PointChange(POINT_HP, GetPoint(POINT_HP_RECOVER_CONTINUE));

	if (GetPoint(POINT_SP_RECOVER_CONTINUE) > 0)
		PointChange(POINT_SP, GetPoint(POINT_SP_RECOVER_CONTINUE));

	AutoRecoveryItemProcess(AFFECT_AUTO_HP_RECOVERY);
	AutoRecoveryItemProcess(AFFECT_AUTO_SP_RECOVERY);

	if (GetMaxStamina() > GetStamina())
	{
		int iSec = (get_dword_time() - GetStopTime()) / 3000;
		if (iSec)
			PointChange(POINT_STAMINA, GetMaxStamina() / 1);
	}

	if (ProcessAffect())
		if (GetPoint(POINT_HP_RECOVERY) == 0 && GetPoint(POINT_SP_RECOVERY) == 0 && GetStamina() == GetMaxStamina())
		{
			m_pkAffectEvent = NULL;
			return false;
		}

	return true;
}

void CHARACTER::StartAffectEvent()
{
	if (m_pkAffectEvent)
		return;

	char_event_info* info = AllocEventInfo<char_event_info>();
	info->ch = this;
	m_pkAffectEvent = event_create(affect_event, info, passes_per_sec);
}

#ifdef __NOT_BUFF_CLEAR__
void CHARACTER::ClearAffect(bool bSave, bool bSomeAffect)
#else
void CHARACTER::ClearAffect(bool bSave)
#endif
{
	TAffectFlag afOld = m_afAffectFlag;
	WORD	wMovSpd = GetPoint(POINT_MOV_SPEED);
	WORD	wAttSpd = GetPoint(POINT_ATT_SPEED);

	itertype(m_list_pkAffect) it = m_list_pkAffect.begin();

	while (it != m_list_pkAffect.end())
	{
		CAffect* pkAff = *it;

		if (!pkAff) // PACHI because if 0x0 continue loop
		{
			++it;
			continue;
		}

		if (bSave)
		{
			if (IS_NO_CLEAR_ON_DEATH_AFFECT(pkAff->dwType) || IS_NO_SAVE_AFFECT(pkAff->dwType))
			{
				++it;
				continue;
			}

			if (AFFECT_QUEST_START_IDX <= pkAff->dwType && pkAff->dwType <= AFFECT_QUEST_END_IDX)
			{
				++it;
				continue;
			}

#ifdef __POTION_AS_REWORK__
			DWORD pkAff_blend[] = { RED_BLEND_POTION, ORANGE_BLEND_POTION, YELLOW_BLEND_POTION, GREEN_BLEND_POTION, BLUE_BLEND_POTION, WHITE_BLEND_POTION, JADE_BLEND_POTION, DARK_BLEND_POTION, SAFE_BLEND_POTION, DEVIL_BLEND_POTION, HEAL_BLEND_POTION, NORMAL_HIT_BLEND_POTION, UNDEAD_BLEND_POTION, DEVIL_ATT_BLEND_POTION, ANIMAL_BLEND_POTION, MILGYO_BLEND_POTION };
			if ((pkAff->dwType == pkAff_blend[0]) || (pkAff->dwType == pkAff_blend[1]) || (pkAff->dwType == pkAff_blend[2]) || (pkAff->dwType == pkAff_blend[3]) || (pkAff->dwType == pkAff_blend[4]) || (pkAff->dwType == pkAff_blend[5]) ||
				(pkAff->dwType == pkAff_blend[6]) || (pkAff->dwType == pkAff_blend[7]) || (pkAff->dwType == pkAff_blend[8]) || (pkAff->dwType == pkAff_blend[9]) || (pkAff->dwType == pkAff_blend[10]) || (pkAff->dwType == pkAff_blend[11]) || (pkAff->dwType == pkAff_blend[12]) || (pkAff->dwType == pkAff_blend[13]) || (pkAff->dwType == pkAff_blend[14]) || (pkAff->dwType == pkAff_blend[15]))
			{
				++it;
				continue;
			}
#endif
#ifdef __BLEND_R_ITEMS__
			DWORD new_aff[] = { AFFECT_18385, AFFECT_18386, AFFECT_18387, AFFECT_18388, AFFECT_18389, AFFECT_18390, AFFECT_ATT_SPEED, AFFECT_MOV_SPEED };
			if ((pkAff->dwType == new_aff[0]) || (pkAff->dwType == new_aff[1]) || (pkAff->dwType == new_aff[2]) ||
				(pkAff->dwType == new_aff[3]) || (pkAff->dwType == new_aff[4]) || (pkAff->dwType == new_aff[5]) ||
				(pkAff->dwType == new_aff[6]) ||
				(pkAff->dwType == new_aff[7]))
			{
				++it;
				continue;
			}
#endif

#ifdef __NOT_BUFF_CLEAR__
			if (bSomeAffect)
			{
				switch (pkAff->dwType)
				{
					case (SKILL_JEONGWI):
					case (SKILL_GEOMKYUNG):
					case (SKILL_CHUNKEON):
					case (SKILL_GWIGEOM):
					case (SKILL_TERROR):
					case (SKILL_JUMAGAP):
					case (SKILL_HOSIN):
					case (SKILL_REFLECT): //
					case (SKILL_GICHEON):
					case (SKILL_KWAESOK): // IYILESTIRME
					case (SKILL_JEUNGRYEOK): // IYILESTIRME
					case (SKILL_GYEONGGONG): // HAFIF ADIM
#ifdef __WOLFMAN_CHARACTER__
					case (SKILL_JEOKRANG):
					case (SKILL_CHEONGRANG):
#endif
					{
						++it;
						continue;
					}
				}
			}
#endif

#ifdef ENABLE_MULTI_FARM_BLOCK
			if (pkAff->dwType == AFFECT_MULTI_FARM_PREMIUM)
			{
				++it;
				continue;
			}
#endif


			if (IsPC())
			{
				SendAffectRemovePacket(GetDesc(), GetPlayerID(), pkAff->dwType, pkAff->bApplyOn);
			}
		}

		ComputeAffect(pkAff, false);

		it = m_list_pkAffect.erase(it);
		CAffect::Release(pkAff);
	}
	if (afOld != m_afAffectFlag || wMovSpd != GetPoint(POINT_MOV_SPEED) || wAttSpd != GetPoint(POINT_ATT_SPEED))
		UpdatePacket();

	CheckMaximumPoints();

	if (m_list_pkAffect.empty())
		event_cancel(&m_pkAffectEvent);
}

const int gercekZamanliAff[] = {
	AFFECT_OTOMATIKAV,
};

int CHARACTER::ProcessAffect()
{
	bool	bDiff = false;
	CAffect* pkAff = NULL;

	for (int i = 0; i <= PREMIUM_MAX_NUM; ++i)
	{
		int aff_idx = i + AFFECT_PREMIUM_START;

		pkAff = FindAffect(aff_idx);

		if (!pkAff)
			continue;

		int remain = GetPremiumRemainSeconds(i);

		if (remain < 0)
		{
			RemoveAffect(aff_idx);
			bDiff = true;
		}
		else
			pkAff->lDuration = remain + 1;
	}

	////////// HAIR_AFFECT
	pkAff = FindAffect(AFFECT_HAIR);
	if (pkAff)
	{
		// IF HAIR_LIMIT_TIME() < CURRENT_TIME()
		if (this->GetQuestFlag("hair.limit_time") < get_global_time())
		{
			// SET HAIR NORMAL
			this->SetPart(PART_HAIR, 0);
			// REMOVE HAIR AFFECT
			RemoveAffect(AFFECT_HAIR);
		}
		else
		{
			// INCREASE AFFECT DURATION
			++(pkAff->lDuration);
		}
	}
	////////// HAIR_AFFECT
	//

#ifdef __BATTLE_PASS_SYSTEM__
	pkAff = FindAffect(AFFECT_BATTLE_PASS_PREMIUM);

	if (!pkAff)
		pkAff = FindAffect(AFFECT_BATTLE_PASS);

	if (pkAff)
	{
		int remain = GetBattlePassEndTime();
		
		if (remain < 0)
		{
			RemoveAffect(pkAff);
			m_dwBattlePassEndTime = 0;
			bDiff = true;
		}
		else
			pkAff->lDuration = remain + 1;
	}
#endif

	CHorseNameManager::instance().Validate(this);

	TAffectFlag afOld = m_afAffectFlag;
	long lMovSpd = GetPoint(POINT_MOV_SPEED);
	long lAttSpd = GetPoint(POINT_ATT_SPEED);

	itertype(m_list_pkAffect) it;

	it = m_list_pkAffect.begin();

	while (it != m_list_pkAffect.end())
	{
		pkAff = *it;

		bool bEnd = false;

		if (!pkAff) // @duzenleme affect varmi diye bakiyoruz ama buradan yapilmamasi lazim.
		{
			sys_err("Null affect! (name %s list.len: %d)", GetName(), m_list_pkAffect.size());
			it = m_list_pkAffect.erase(it);
			continue;
		}

		if (pkAff->dwType >= GUILD_SKILL_START && pkAff->dwType <= GUILD_SKILL_END)
		{
			if (!GetWarMap())//@duzenleme eger adam lonca savas haritasinda degilse lonca skillerini kapatiyoruz.
				bEnd = true;

			if (!GetGuild() || !GetGuild()->UnderAnyWar())
				bEnd = true;
		}

		if (pkAff->lSPCost > 0)
		{
			if (GetSP() < pkAff->lSPCost)
				bEnd = true;
			else
				PointChange(POINT_SP, -pkAff->lSPCost);
		}

		// AFFECT_DURATION_BUG_FIX
		const long suanTime = get_global_time();
		bool gercekZamanli = false;
		for (BYTE i = 0; i < (sizeof(gercekZamanliAff) / sizeof(gercekZamanliAff[0])); i++) {
			if (gercekZamanliAff[i] == pkAff->dwType) {
				if (pkAff->lDuration <= (long)suanTime) {
					RemoveAffect(pkAff->dwType);
					bDiff = true;
				}
				gercekZamanli = true;
				break;
			}
		}
		if (!gercekZamanli) {
			if (--pkAff->lDuration <= 0) { bEnd = true; }
		}
		// END_AFFECT_DURATION_BUG_FIX

		if (bEnd)
		{
			it = m_list_pkAffect.erase(it);
			ComputeAffect(pkAff, false);
			bDiff = true;
			if (IsPC())
				SendAffectRemovePacket(GetDesc(), GetPlayerID(), pkAff->dwType, pkAff->bApplyOn);

			CAffect::Release(pkAff);

			continue;
		}

		++it;
	}

	if (bDiff)
	{
		if (afOld != m_afAffectFlag || lMovSpd != GetPoint(POINT_MOV_SPEED) || lAttSpd != GetPoint(POINT_ATT_SPEED))
			UpdatePacket();

		CheckMaximumPoints();
	}

	if (m_list_pkAffect.empty())
		return true;

	return false;
}

void CHARACTER::SaveAffect()
{
	if (!IsLoadedAffect())
	{
		//sys_err("SaveAffect : not loaded (%s)", GetName());
		return;
	}

	std::vector<TPacketGDAddAffect> pVec(m_list_pkAffect.size());
	DWORD count = 0;
	TPacketGDAddAffect* p;

	itertype(m_list_pkAffect) it = m_list_pkAffect.begin();

	while (it != m_list_pkAffect.end())
	{
		CAffect* pkAff = *it++;

		if (IS_NO_SAVE_AFFECT(pkAff->dwType))
			continue;

		// if (IS_UNLIMITED_AFFECT_DURATION(pkAff->lDuration))
		// 	continue;
		// save paketini alinca hepsinni temizleyip tekrar kurdugumuz icin
		// butun affectlerin gitmesi gerekiyor

		//sys_log(0, "AFFECT_SAVE: %u %u %d %d", pkAff->dwType, pkAff->bApplyOn, pkAff->lApplyValue, pkAff->lDuration);

		p = &pVec.at(count++);

		p->dwPID = GetPlayerID();
		p->elem.dwType = pkAff->dwType;
		p->elem.bApplyOn = pkAff->bApplyOn;
		p->elem.lApplyValue = pkAff->lApplyValue;
		p->elem.dwFlag = pkAff->dwFlag;
		p->elem.lDuration = pkAff->lDuration;
		p->elem.lSPCost = pkAff->lSPCost;

	}

	if (count)
	{
		//sys_log(0, "AFFECT_SAVE: %u %u", GetPlayerID(), count);
		// TPacketGDAddAffect* x = &pVec[0];
		// sys_log(0, "AFFECT_SAVE TEST: %u", x->dwPID);
		db_clientdesc->DBPacket(HEADER_GD_SAVE_AFFECT, 0, &pVec[0], sizeof(TPacketGDAddAffect) * count);
	}
}

EVENTINFO(load_affect_login_event_info)
{
	DWORD pid;
	DWORD count;
	char* data;

	load_affect_login_event_info()
		: pid(0)
		, count(0)
		, data(0)
	{
	}
};

EVENTFUNC(load_affect_login_event)
{
	load_affect_login_event_info* info = dynamic_cast<load_affect_login_event_info*>(event->info);

	if (info == NULL)
	{
		sys_err("load_affect_login_event_info> <Factor> Null pointer");
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
		ch->LoadAffect(info->count, (TPacketAffectElement*)info->data);
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

void CHARACTER::LoadAffect(DWORD dwCount, TPacketAffectElement* pElements)
{
	m_bIsLoadedAffect = false;

	if (!GetDesc()->IsPhase(PHASE_GAME))
	{
		if (test_server)
			sys_log(0, "LOAD_AFFECT: Creating Event PID COUNT %d %d", GetPlayerID(), dwCount);

		load_affect_login_event_info* info = AllocEventInfo<load_affect_login_event_info>();

		info->pid = GetPlayerID();
		info->count = dwCount;
		info->data = M2_NEW char[sizeof(TPacketAffectElement) * dwCount];
		thecore_memcpy(info->data, pElements, sizeof(TPacketAffectElement) * dwCount);

		event_create(load_affect_login_event, info, PASSES_PER_SEC(1));

		return;
	}

	ClearAffect(true);

	if (test_server)
		sys_log(0, "LOAD_AFFECT: PID count %d %d", GetPlayerID(), dwCount);

	TAffectFlag afOld = m_afAffectFlag;

	long lMovSpd = GetPoint(POINT_MOV_SPEED);
	long lAttSpd = GetPoint(POINT_ATT_SPEED);

	for (DWORD i = 0; i < dwCount; ++i, ++pElements)
	{
		if (pElements->dwType == SKILL_MUYEONG)
			continue;

		if (AFFECT_AUTO_HP_RECOVERY == pElements->dwType || AFFECT_AUTO_SP_RECOVERY == pElements->dwType)
		{
			LPITEM item = FindItemByID(pElements->dwFlag);

			if (NULL == item)
				continue;

			item->Lock(true);
		}

		if (pElements->bApplyOn >= POINT_MAX_NUM)
		{
			sys_err("invalid affect data %s ApplyOn %u ApplyValue %d",
				GetName(), pElements->bApplyOn, pElements->lApplyValue);
			continue;
		}

		if (test_server)
			sys_log(0, "Load Affect : Affect %s %d %d", GetName(), pElements->dwType, pElements->bApplyOn);

		CAffect* pkAff = CAffect::Acquire();
		m_list_pkAffect.push_back(pkAff);

		pkAff->dwType = pElements->dwType;
		pkAff->bApplyOn = pElements->bApplyOn;
		pkAff->lApplyValue = pElements->lApplyValue;
		pkAff->dwFlag = pElements->dwFlag;
		pkAff->lDuration = pElements->lDuration;
		pkAff->lSPCost = pElements->lSPCost;

		SendAffectAddPacket(GetDesc(), pkAff, true);

		ComputeAffect(pkAff, true);
	}
	if (afOld != m_afAffectFlag || lMovSpd != GetPoint(POINT_MOV_SPEED) || lAttSpd != GetPoint(POINT_ATT_SPEED))
		UpdatePacket();

	StartAffectEvent();

	m_bIsLoadedAffect = true;

	//ComputePoints(); // @duzenleme binek bonusu 2 sefer islemesin diye yaptik ama kontrol lazim. cunku binek sistemi artik farkli..
	DragonSoul_Initialize();

	// @duzenleme
	// marty orjinalinde mevcut.
	if (!IsDead())
	{
		PointChange(POINT_HP, GetMaxHP() - GetHP());
		PointChange(POINT_SP, GetMaxSP() - GetSP());
	}

#ifdef __CHECK_ITEMS_ON_TELEPORT__
	CheckTeleportItems();
#endif
}

bool CHARACTER::AddAffect(DWORD dwType, BYTE bApplyOn, long lApplyValue, DWORD dwFlag, long lDuration, long lSPCost, bool bOverride, bool IsCube)
{
	// CHAT_BLOCK
	if (dwType == AFFECT_BLOCK_CHAT && lDuration > 1)
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("운영자 제제로 채팅이 금지 되었습니다."));
	}
	// END_OF_CHAT_BLOCK

	if (lDuration == 0)
	{
		sys_err("Character::AddAffect lDuration == 0 type %d", lDuration, dwType);
		lDuration = 1;
	}

	if (dwFlag == AFF_STUN)
	{
		if (m_posDest.x != GetX() || m_posDest.y != GetY())
		{
			m_posDest.x = m_posStart.x = GetX();
			m_posDest.y = m_posStart.y = GetY();
			battle_end(this);

			SyncPacket();
		}
	}

	CAffect* pkOldAff = NULL;

	if (IsCube)
		pkOldAff = FindAffect(dwType, bApplyOn);
	else
		pkOldAff = FindAffect(dwType);

	if (dwFlag == AFF_STUN)
	{
		if (m_posDest.x != GetX() || m_posDest.y != GetY())
		{
			m_posDest.x = m_posStart.x = GetX();
			m_posDest.y = m_posStart.y = GetY();
			battle_end(this);

			SyncPacket();
		}
	}

	if (pkOldAff && bOverride)
	{
		m_list_pkAffect.remove(pkOldAff);

		ComputeAffect(pkOldAff, false);

		if (GetDesc())
			SendAffectRemovePacket(GetDesc(), GetPlayerID(), pkOldAff->dwType, pkOldAff->bApplyOn);
	}

	CAffect* pkNewAff = CAffect::Acquire();
	m_list_pkAffect.push_back(pkNewAff);

	//sys_log(1, "AddAffect %s type %d apply %d %d flag %u duration %d", GetName(), dwType, bApplyOn, lApplyValue, dwFlag, lDuration);
	//sys_log(0, "AddAffect %s type %d apply %d %d flag %u duration %d", GetName(), dwType, bApplyOn, lApplyValue, dwFlag, lDuration);

	pkNewAff->dwType = dwType;
	pkNewAff->bApplyOn = bApplyOn;
	pkNewAff->lApplyValue = lApplyValue;
	pkNewAff->dwFlag = dwFlag;
	pkNewAff->lDuration = lDuration;
	pkNewAff->lSPCost = lSPCost;

	WORD wMovSpd = GetPoint(POINT_MOV_SPEED);
	WORD wAttSpd = GetPoint(POINT_ATT_SPEED);

	ComputeAffect(pkNewAff, true);

	if ((pkOldAff && pkOldAff->dwFlag) || (pkNewAff && pkNewAff->dwFlag) || wMovSpd != GetPoint(POINT_MOV_SPEED) || wAttSpd != GetPoint(POINT_ATT_SPEED))
		UpdatePacket();

	StartAffectEvent();

	if (IsPC())
	{
		SendAffectAddPacket(GetDesc(), pkNewAff);

		if (IS_NO_SAVE_AFFECT(pkNewAff->dwType))
			return true;

		TPacketGDAddAffect p;
		p.dwPID = GetPlayerID();
		p.elem.dwType = pkNewAff->dwType;
		p.elem.bApplyOn = pkNewAff->bApplyOn;
		p.elem.lApplyValue = pkNewAff->lApplyValue;
		p.elem.dwFlag = pkNewAff->dwFlag;
		p.elem.lDuration = pkNewAff->lDuration;
		p.elem.lSPCost = pkNewAff->lSPCost;
		db_clientdesc->DBPacket(HEADER_GD_ADD_AFFECT, 0, &p, sizeof(p));
	}

	return true;
}

void CHARACTER::RefreshAffect()
{
	itertype(m_list_pkAffect) it = m_list_pkAffect.begin();

	while (it != m_list_pkAffect.end())
	{
		CAffect* pkAff = *it++;
		ComputeAffect(pkAff, true);
	}
}

void CHARACTER::ComputeAffect(CAffect* pkAff, bool bAdd)
{
	if (bAdd && pkAff->dwType >= GUILD_SKILL_START && pkAff->dwType <= GUILD_SKILL_END)
	{
		if (!GetGuild())
			return;

		if (!GetGuild()->UnderAnyWar())
			return;

		if (!GetWarMap()) // @duzenleme eger adam lonca haritasi disindaysa lonca becerisi kullanamiyor.
			return;
	}

	if (pkAff->dwFlag)
	{
		if (!bAdd)
			m_afAffectFlag.Reset(pkAff->dwFlag);
		else
			m_afAffectFlag.Set(pkAff->dwFlag);
	}

	if (bAdd)
		PointChange(pkAff->bApplyOn, pkAff->lApplyValue);
	else
		PointChange(pkAff->bApplyOn, -pkAff->lApplyValue);

	if (pkAff->dwType == SKILL_MUYEONG)
	{
		if (bAdd)
			StartMuyeongEvent();
		else
			StopMuyeongEvent();
	}
}

bool CHARACTER::RemoveAffect(CAffect* pkAff)
{
	if (!pkAff)
		return false;

	// AFFECT_BUF_FIX
	m_list_pkAffect.remove(pkAff);
	// END_OF_AFFECT_BUF_FIX

	ComputeAffect(pkAff, false);

	if (AFFECT_REVIVE_INVISIBLE != pkAff->dwType && AFFECT_MOUNT != pkAff->dwType)
		ComputePoints();
	else // @duzenleme gorunmezlik buga giriyordu bununla alakali.
		UpdatePacket();

	CheckMaximumPoints();

	if (test_server)
		sys_log(0, "AFFECT_REMOVE: %s (flag %u apply: %u)", GetName(), pkAff->dwFlag, pkAff->bApplyOn);

	if (IsPC())
		SendAffectRemovePacket(GetDesc(), GetPlayerID(), pkAff->dwType, pkAff->bApplyOn);

	CAffect::Release(pkAff);
	return true;
}

bool CHARACTER::RemoveAffect(DWORD dwType)
{
	// CHAT_BLOCK
	if (dwType == AFFECT_BLOCK_CHAT)
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("채팅 금지가 풀렸습니다."));
	}
	// END_OF_CHAT_BLOCK

	bool flag = false;

	CAffect* pkAff;

	while ((pkAff = FindAffect(dwType)))
	{
		RemoveAffect(pkAff);
		flag = true;
	}

	return flag;
}

int CHARACTER::CheckAffectValue(DWORD dwType, BYTE bApply)
{
	int flag = 0;

	CAffect* pkAff = FindAffect(dwType, bApply);

	if (pkAff)
		flag = pkAff->lApplyValue;

	return flag;
}

bool CHARACTER::IsAffectFlag(DWORD dwAff) const
{
	return m_afAffectFlag.IsSet(dwAff);
}

void CHARACTER::RemoveGoodAffect()
{
	RemoveAffect(AFFECT_MOV_SPEED);
	RemoveAffect(AFFECT_ATT_SPEED);
	RemoveAffect(AFFECT_STR);
	RemoveAffect(AFFECT_DEX);
	RemoveAffect(AFFECT_INT);
	RemoveAffect(AFFECT_CON);
	RemoveAffect(AFFECT_CHINA_FIREWORK);

	RemoveAffect(SKILL_JEONGWI);
	RemoveAffect(SKILL_GEOMKYUNG);
	RemoveAffect(SKILL_CHUNKEON);
	RemoveAffect(SKILL_EUNHYUNG);
	RemoveAffect(SKILL_GYEONGGONG);
	RemoveAffect(SKILL_GWIGEOM);
	RemoveAffect(SKILL_TERROR);
	RemoveAffect(SKILL_JUMAGAP);
	RemoveAffect(SKILL_MANASHILED);
	RemoveAffect(SKILL_MUYEONG); // Sura skill added.
	RemoveAffect(SKILL_HOSIN);
	RemoveAffect(SKILL_REFLECT);
	RemoveAffect(SKILL_KWAESOK);
	RemoveAffect(SKILL_JEUNGRYEOK);
	RemoveAffect(SKILL_GICHEON);
#ifdef __WOLFMAN_CHARACTER__
	RemoveAffect(SKILL_JEOKRANG);
	RemoveAffect(SKILL_CHEONGRANG);
#endif
}

void CHARACTER::RemoveOnlySkillAffect()
{
	RemoveAffect(SKILL_JEONGWI);
	RemoveAffect(SKILL_GEOMKYUNG);
	RemoveAffect(SKILL_CHUNKEON);
	RemoveAffect(SKILL_EUNHYUNG);
	RemoveAffect(SKILL_GYEONGGONG);
	RemoveAffect(SKILL_GWIGEOM);
	RemoveAffect(SKILL_TERROR);
	RemoveAffect(SKILL_JUMAGAP);
	RemoveAffect(SKILL_MANASHILED);
	RemoveAffect(SKILL_MUYEONG); // Sura skill added.
	RemoveAffect(SKILL_HOSIN);
	RemoveAffect(SKILL_REFLECT);
	RemoveAffect(SKILL_KWAESOK);
	RemoveAffect(SKILL_JEUNGRYEOK);
	RemoveAffect(SKILL_GICHEON);
#ifdef __WOLFMAN_CHARACTER__
	RemoveAffect(SKILL_JEOKRANG);
	RemoveAffect(SKILL_CHEONGRANG);
#endif
}

bool CHARACTER::IsGoodAffect(BYTE bAffectType) const
{
	switch (bAffectType)
	{
	case (AFFECT_MOV_SPEED):
	case (AFFECT_ATT_SPEED):
	case (AFFECT_STR):
	case (AFFECT_DEX):
	case (AFFECT_INT):
	case (AFFECT_CON):
	case (AFFECT_CHINA_FIREWORK):

	case (SKILL_JEONGWI):
	case (SKILL_GEOMKYUNG):
	case (SKILL_CHUNKEON):
	case (SKILL_EUNHYUNG):
	case (SKILL_GYEONGGONG):
	case (SKILL_GWIGEOM):
	case (SKILL_TERROR):
	case (SKILL_JUMAGAP):
	case (SKILL_MANASHILED):
	case (SKILL_HOSIN):
	case (SKILL_REFLECT):
	case (SKILL_KWAESOK):
	case (SKILL_JEUNGRYEOK):
	case (SKILL_GICHEON):
#ifdef __WOLFMAN_CHARACTER__
	case (SKILL_JEOKRANG):
	case (SKILL_CHEONGRANG):
#endif
	case (SKILL_MUYEONG):
		return true;
	}
	return false;
}

void CHARACTER::RemoveBadAffect()
{
	//sys_log(0, "RemoveBadAffect %s", GetName());
	RemovePoison();
#ifdef __WOLFMAN_CHARACTER__
	RemoveBleeding();
#endif
	RemoveFire();
	RemoveAffect(AFFECT_STUN);
	RemoveAffect(AFFECT_SLOW);
	RemoveAffect(SKILL_TUSOK);
}
