#include "stdafx.h"
#include "utils.h"
#include "config.h"
#include "desc.h"
#include "desc_manager.h"
#include "char_manager.h"
#include "item.h"
#include "item_manager.h"
#include "mob_manager.h"
#include "battle.h"
#include "pvp.h"
#include "skill.h"
#include "start_position.h"
#include "cmd.h"
#ifdef __SCP1453_EXTENSIONS__
#include "dungeon_new.h"
#else
#include "dungeon.h"
#endif
#include "log.h"
#include "unique_item.h"
#include "priv_manager.h"
#include "db.h"
#include "vector.h"
#include "regen.h"
#include "exchange.h"
#include "shop_manager.h"
#include "packet.h"
#include "party.h"
#include "affect.h"
#include "guild.h"
#include "guild_manager.h"
#include "questmanager.h"
#include "questlua.h"
#include "war_map.h"

#ifdef __GROWTH_PET_SYSTEM__
#include "New_PetSystem.h"
#endif

#ifdef __GROWTH_MOUNT_SYSTEM__
#include "New_MountSystem.h"
#endif
#ifdef __BATTLE_PASS_SYSTEM__
#include "battle_pass.h"
#endif

static DWORD __GetPartyExpNP(const DWORD level)
{
	if (!level || level > PLAYER_EXP_TABLE_MAX)
		return 14000;
	return party_exp_distribute_table[level];
}

static int __GetExpLossPerc(const DWORD level)
{
	if (!level || level > PLAYER_EXP_TABLE_MAX)
		return 1;
	return aiExpLossPercents[level];
}

DWORD AdjustExpByLevel(const LPCHARACTER ch, const DWORD exp)
{
	if (g_bStoneExpConst)
	{
		if (ch->IsStone())
			return exp;
	}

	if (PLAYER_MAX_LEVEL_CONST < ch->GetLevel())
	{
		double ret = 0.95;
		double factor = 0.1;

		for (ssize_t i = 0; i < ch->GetLevel() - 100; ++i)
		{
			if ((i % 10) == 0)
				factor /= 2.0;

			ret *= 1.0 - factor;
		}

		ret = ret * static_cast<double>(exp);

		if (ret < 1.0)
			return 1;

		return static_cast<DWORD>(ret);
	}

	return exp;
}

bool CHARACTER::CanBeginFight() const
{
	if (!CanMove())
		return false;

	return m_pointsInstant.position == POS_STANDING && !IsDead() && !IsStun();
}

void CHARACTER::BeginFight(LPCHARACTER pkVictim)
{
	SetVictim(pkVictim);
	SetPosition(POS_FIGHTING);
	SetNextStatePulse(1);
}

bool CHARACTER::CanFight() const
{
	return m_pointsInstant.position >= POS_FIGHTING ? true : false;
}

void CHARACTER::CreateFly(BYTE bType, LPCHARACTER pkVictim)
{
	TPacketGCCreateFly packFly;

	packFly.bHeader = HEADER_GC_CREATE_FLY;
	packFly.bType = bType;
	packFly.dwStartVID = GetVID();
	packFly.dwEndVID = pkVictim->GetVID();

	PacketAround(&packFly, sizeof(TPacketGCCreateFly));
}

void CHARACTER::DistributeSP(LPCHARACTER pkKiller, int iMethod)
{
	if (pkKiller->GetSP() >= pkKiller->GetMaxSP())
		return;

	bool bAttacking = (get_dword_time() - GetLastAttackTime()) < 3000;
	bool bMoving = (get_dword_time() - GetLastMoveTime()) < 3000;

	if (iMethod == 1)
	{
		int num = number(0, 3);

		if (!num)
		{
			int iLvDelta = GetLevel() - pkKiller->GetLevel();
			int iAmount = 0;

			if (iLvDelta >= 5)
				iAmount = 10;
			else if (iLvDelta >= 0)
				iAmount = 6;
			else if (iLvDelta >= -3)
				iAmount = 2;

			if (iAmount != 0)
			{
				iAmount += (iAmount * pkKiller->GetPoint(POINT_SP_REGEN)) / 100;

				if (iAmount >= 11)
					CreateFly(FLY_SP_BIG, pkKiller);
				else if (iAmount >= 7)
					CreateFly(FLY_SP_MEDIUM, pkKiller);
				else
					CreateFly(FLY_SP_SMALL, pkKiller);

				pkKiller->PointChange(POINT_SP, iAmount);
			}
		}
	}
	else
	{
		if (pkKiller->GetJob() == JOB_SHAMAN || (pkKiller->GetJob() == JOB_SURA && pkKiller->GetSkillGroup() == 2))
		{
			int iAmount;

			if (bAttacking)
				iAmount = 2 + GetMaxSP() / 100;
			else if (bMoving)
				iAmount = 3 + GetMaxSP() * 2 / 100;
			else
				iAmount = 10 + GetMaxSP() * 3 / 100;

			iAmount += (iAmount * pkKiller->GetPoint(POINT_SP_REGEN)) / 100;
			pkKiller->PointChange(POINT_SP, iAmount);
		}
		else
		{
			int iAmount;

			if (bAttacking)
				iAmount = 2 + pkKiller->GetMaxSP() / 200;
			else if (bMoving)
				iAmount = 2 + pkKiller->GetMaxSP() / 100;
			else
			{
				if (pkKiller->GetHP() < pkKiller->GetMaxHP())
					iAmount = 2 + (pkKiller->GetMaxSP() / 100);
				else
					iAmount = 9 + (pkKiller->GetMaxSP() / 100);
			}

			iAmount += (iAmount * pkKiller->GetPoint(POINT_SP_REGEN)) / 100;
			pkKiller->PointChange(POINT_SP, iAmount);
		}
	}
}

bool CHARACTER::Attack(LPCHARACTER pkVictim, BYTE bType)
{
	if (!pkVictim)
	{
		sys_err("!--> I can't find pkVictim(0x0)...");
		return false;
	}

	if (IsPC() && (!CanMove() || IsObserverMode()))
		return false;

	// @duzenleme
	// marty attack fonksiyonuna eger atak yapilamiyor ise diye koymus fakat bu tarz kontrollerin call fonksiyonlarda degilde
	// input kisimlarinda olmasi gerekiyor.
	if (!battle_is_attackable(this, pkVictim))
		return false;

	DWORD dwCurrentTime = get_dword_time();

	if (IsPC())
	{
		if (bType == 0 && dwCurrentTime < GetSkipComboAttackByTime())
			return false;

		// Validating max range
		if (!pkVictim->IsPC() && DISTANCE_APPROX(pkVictim->GetX() - GetX(), pkVictim->GetY() - GetY()) > 300)
			return false;

	}
	else
	{
		MonsterChat(MONSTER_CHAT_ATTACK);
	}

	pkVictim->SetSyncOwner(this);

	if (pkVictim->CanBeginFight())
		pkVictim->BeginFight(this);

	int iRet;

	if (bType == 0)
	{
		//
		switch (GetMobBattleType())
		{
		case BATTLE_TYPE_MELEE:
		case BATTLE_TYPE_POWER:
		case BATTLE_TYPE_TANKER:
		case BATTLE_TYPE_SUPER_POWER:
		case BATTLE_TYPE_SUPER_TANKER:
			iRet = battle_melee_attack(this, pkVictim);
			break;

		case BATTLE_TYPE_RANGE:
			FlyTarget(pkVictim->GetVID(), pkVictim->GetX(), pkVictim->GetY(), HEADER_CG_FLY_TARGETING);
			iRet = Shoot(0) ? BATTLE_DAMAGE : BATTLE_NONE;
			break;

		case BATTLE_TYPE_MAGIC:
			FlyTarget(pkVictim->GetVID(), pkVictim->GetX(), pkVictim->GetY(), HEADER_CG_FLY_TARGETING);
			iRet = Shoot(1) ? BATTLE_DAMAGE : BATTLE_NONE;
			break;

		default:
			sys_err("Unhandled battle type %d", GetMobBattleType());
			iRet = BATTLE_NONE;
			break;
		}
	}
	else
	{
		if (IsPC() == true)
		{
			if (dwCurrentTime - m_dwLastSkillTime > 1500)
			{
				sys_log(1, "HACK: Too long skill using term. Name(%s) PID(%u) delta(%u)",
					GetName(), GetPlayerID(), (dwCurrentTime - m_dwLastSkillTime));
				return false;
			}
		}

		sys_log(1, "Attack call ComputeSkill %d %s", bType, pkVictim ? pkVictim->GetName() : "");
		iRet = ComputeSkill(bType, pkVictim);
	}

	//if (test_server && IsPC())
	//	sys_log(0, "%s Attack %s type %u ret %d", GetName(), pkVictim->GetName(), bType, iRet);
	if (iRet == BATTLE_DAMAGE || iRet == BATTLE_DEAD)
	{
		OnMove(true);
		pkVictim->OnMove();

		// only pc sets victim null. For npc, state machine will reset this.
		if (BATTLE_DEAD == iRet && IsPC())
			SetVictim(NULL);

		return true;
	}

	return false;
}

void CHARACTER::DeathPenalty(BYTE bTown)
{
	sys_log(1, "DEATH_PERNALY_CHECK(%s) town(%d)", GetName(), bTown);

	Cube_close(this);

#ifdef __ACCE_SYSTEM__
	if (IsPC())
		CloseAcce();
#endif

#ifdef __ITEM_CHANGELOOK__
	if (IsPC())
		ChangeLookWindow(false, true);
#endif

	if (GetLevel() < 10)
	{
		sys_log(0, "NO_DEATH_PENALTY_LESS_LV10(%s)", GetName());
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¿ë½ÅÀÇ °¡È£·Î °æÇèÄ¡°¡ ¶³¾îÁöÁö ¾Ê¾Ò½À´Ï´Ù."));
		return;
	}

	if (number(0, 2))
	{
		sys_log(0, "NO_DEATH_PENALTY_LUCK(%s)", GetName());
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¿ë½ÅÀÇ °¡È£·Î °æÇèÄ¡°¡ ¶³¾îÁöÁö ¾Ê¾Ò½À´Ï´Ù."));
		return;
	}

	if (IS_SET(m_pointsInstant.instant_flag, INSTANT_FLAG_DEATH_PENALTY))
	{
		REMOVE_BIT(m_pointsInstant.instant_flag, INSTANT_FLAG_DEATH_PENALTY);

		// NO_DEATH_PENALTY_BUG_FIX
		if (!bTown)
		{
			if (FindAffect(AFFECT_NO_DEATH_PENALTY))
			{
				sys_log(0, "NO_DEATH_PENALTY_AFFECT(%s)", GetName());
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¿ë½ÅÀÇ °¡È£·Î °æÇèÄ¡°¡ ¶³¾îÁöÁö ¾Ê¾Ò½À´Ï´Ù."));
				RemoveAffect(AFFECT_NO_DEATH_PENALTY);
				return;
			}
		}
		// END_OF_NO_DEATH_PENALTY_BUG_FIX

		int iLoss = ((GetNextExp() * __GetExpLossPerc(GetLevel())) / 100);

		iLoss = MIN(800000, iLoss);

		if (bTown)
			iLoss = 0;

		if (IsEquipUniqueItem(UNIQUE_ITEM_TEARDROP_OF_GODNESS))
			iLoss /= 2;

		sys_log(0, "DEATH_PENALTY(%s) EXP_LOSS: %d percent %d%%", GetName(), iLoss, __GetExpLossPerc(GetLevel()));

		PointChange(POINT_EXP, -iLoss, true);
	}
}

bool CHARACTER::IsStun() const
{
	if (IS_SET(m_pointsInstant.instant_flag, INSTANT_FLAG_STUN))
		return true;

	return false;
}

EVENTFUNC(StunEvent)
{
	char_event_info* info = dynamic_cast<char_event_info*>(event->info);

	if (info == NULL)
	{
		sys_err("StunEvent> <Factor> Null pointer");
		return 0;
	}

	LPCHARACTER ch = info->ch;

	if (ch == NULL) { // <Factor>
		return 0;
	}
	ch->m_pkStunEvent = NULL;
	ch->Dead();
	return 0;
}

void CHARACTER::Stun()
{
	if (IsStun())
		return;

	if (IsDead())
		return;

	if (!IsPC() && m_pkParty)
	{
		m_pkParty->SendMessage(this, PM_ATTACKED_BY, 0, 0);
	}

	sys_log(1, "%s: Stun %p", GetName(), this);

	PointChange(POINT_HP_RECOVERY, -GetPoint(POINT_HP_RECOVERY));
	PointChange(POINT_SP_RECOVERY, -GetPoint(POINT_SP_RECOVERY));

	CloseMyShop();

	event_cancel(&m_pkRecoveryEvent);

	TPacketGCStun pack;
	pack.header = HEADER_GC_STUN;
	pack.vid = m_vid;
	PacketAround(&pack, sizeof(pack));

	SET_BIT(m_pointsInstant.instant_flag, INSTANT_FLAG_STUN);

	if (m_pkStunEvent)
		return;

	char_event_info* info = AllocEventInfo<char_event_info>();

	info->ch = this;

	m_pkStunEvent = event_create(StunEvent, info, PASSES_PER_SEC(0));
}

EVENTINFO(SCharDeadEventInfo)
{
	bool isPC;
	uint32_t dwID;

	SCharDeadEventInfo()
	: isPC(0)
	, dwID(0)
	{
	}
};

EVENTFUNC(dead_event)
{
	const SCharDeadEventInfo* info = dynamic_cast<SCharDeadEventInfo*>(event->info);

	if (info == NULL)
	{
		sys_err("dead_event> <Factor> Null pointer");
		return 0;
	}

	LPCHARACTER ch = NULL;

	if (true == info->isPC)
		ch = CHARACTER_MANAGER::instance().FindByPID(info->dwID);
	else
		ch = CHARACTER_MANAGER::instance().Find(info->dwID);

	if (NULL == ch)
	{
		sys_err("DEAD_EVENT: cannot find char pointer with %s id(%d)", info->isPC ? "PC" : "MOB", info->dwID);
		return 0;
	}

	ch->m_pkDeadEvent = NULL;

	if (ch->GetDesc())
	{
		ch->GetDesc()->SetPhase(PHASE_GAME);

		ch->SetPosition(POS_STANDING);

		PIXEL_POSITION pos;

		if (SECTREE_MANAGER::instance().GetRecallPositionByEmpire(ch->GetMapIndex(), ch->GetEmpire(), pos))
			ch->WarpSet(pos.x, pos.y);
		else
		{
			sys_err("cannot find spawn position (name %s)", ch->GetName());
			ch->WarpSet(EMPIRE_START_X(ch->GetEmpire()), EMPIRE_START_Y(ch->GetEmpire()));
		}

		ch->PointChange(POINT_HP, (ch->GetMaxHP() / 2) - ch->GetHP(), true);

		ch->DeathPenalty(0);

		ch->StartRecoveryEvent();

		ch->ChatPacket(CHAT_TYPE_COMMAND, "CloseRestartWindow");
	}
	else
	{
		if (ch->IsMonster() == true)
		{
			if (ch->IsRevive() == false && ch->HasReviverInParty() == true)
			{
				ch->SetPosition(POS_STANDING);
				ch->SetHP(ch->GetMaxHP());

				ch->ViewReencode();

				ch->SetAggressive();
				ch->SetRevive(true);

				return 0;
			}
		}

		M2_DESTROY_CHARACTER(ch);
	}

	return 0;
}

bool CHARACTER::IsDead() const
{
	if (m_pointsInstant.position == POS_DEAD)
		return true;

	return false;
}

#ifdef __GOLD_REWARD_REWORK__
void CHARACTER::RewardGold(LPCHARACTER pkAttacker)
{
	if (m_pkMobData == NULL)
	{
		sys_err("RewardGold:: CHARACTER hasnt pTable");
		return;
	}
#ifdef __GOLD_LIMIT_REWORK__
	long long llTotalGold = 0;
#else
	int iTotalGold = 0;
#endif
	int iGoldPercent = MobRankStats[GetMobRank()].iGoldPercent;

	//if (pkAttacker->IsPC())
	//	iGoldPercent = iGoldPercent * (100 + CPrivManager::instance().GetPriv(pkAttacker, PRIV_GOLD_DROP)) / 100;

	//if (pkAttacker->GetPoint(POINT_MALL_GOLDBONUS))
	//	iGoldPercent += (iGoldPercent * pkAttacker->GetPoint(POINT_MALL_GOLDBONUS) / 100);

	iGoldPercent = iGoldPercent * CHARACTER_MANAGER::instance().GetMobGoldDropRate(pkAttacker) / 100;

	if (pkAttacker->GetPremiumRemainSeconds(PREMIUM_GOLD) > 0 || pkAttacker->IsEquipUniqueGroup(UNIQUE_GROUP_LUCKY_GOLD))
		iGoldPercent += iGoldPercent;

	if (iGoldPercent > 100)
		iGoldPercent = 100;

	int iPercent;

	if (GetMobRank() >= MOB_RANK_BOSS)
		iPercent = ((iGoldPercent * PERCENT_LVDELTA_BOSS(pkAttacker->GetLevel(), GetLevel())) / 100);
	else
		iPercent = ((iGoldPercent * PERCENT_LVDELTA(pkAttacker->GetLevel(), GetLevel())) / 100);

	if (number(1, 100) > iPercent)
		return;

	int iGoldMultipler = GOLD_MULTIPLER;

	if (1 == number(1, 50000))
		iGoldMultipler *= 10;
	else if (1 == number(1, 10000))
		iGoldMultipler *= 5;

	//if (pkAttacker->GetPoint(POINT_GOLD_DOUBLE_BONUS))
	//	if (number(1, 100) <= pkAttacker->GetPoint(POINT_GOLD_DOUBLE_BONUS))
	//		iGoldMultipler *= 2;

	if (test_server)
		pkAttacker->ChatPacket(CHAT_TYPE_PARTY, "gold_mul %d rate %d", iGoldMultipler, CHARACTER_MANAGER::instance().GetMobGoldAmountRate(pkAttacker));
#ifdef __GOLD_LIMIT_REWORK__
	long long llGold = number(GetMobTable().llGoldMin, GetMobTable().llGoldMax);
	llGold = llGold * CHARACTER_MANAGER::instance().GetMobGoldAmountRate(pkAttacker) / 100;
	llGold *= iGoldMultipler;
#else
	int iGold = number(GetMobTable().dwGoldMin, GetMobTable().dwGoldMax);
	iGold = iGold * CHARACTER_MANAGER::instance().GetMobGoldAmountRate(pkAttacker) / 100;
	iGold *= iGoldMultipler;
#endif
	int iSplitCount;

#ifdef __GOLD_LIMIT_REWORK__
	if (llGold >= 3)
#else
	if (iGold >= 3)
#endif
		iSplitCount = number(1, 3);
	else if (GetMobRank() >= MOB_RANK_BOSS)
	{
		iSplitCount = number(3, 10);

#ifdef __GOLD_LIMIT_REWORK__
		if ((llGold / iSplitCount) == 0)
#else
		if ((iGold / iSplitCount) == 0)
#endif
			iSplitCount = 1;
	}
	else
		iSplitCount = 1;

#ifdef __GOLD_LIMIT_REWORK__
	if (llGold != 0)
	{
		llTotalGold += llGold;

		for (int i = 0; i < iSplitCount; ++i)
		{
			pkAttacker->GiveGold(llTotalGold / iSplitCount);
		}
	}

#else
	if (iGold != 0)
	{
		iTotalGold += iGold;

		for (int i = 0; i < iSplitCount; ++i)
		{
			pkAttacker->GiveGold(iGold / iSplitCount);
		}
	}

#endif
}
#else
void CHARACTER::RewardGold(LPCHARACTER pkAttacker)
{
	// ADD_PREMIUM
	bool isAutoLoot =
		(pkAttacker->GetPremiumRemainSeconds(PREMIUM_AUTOLOOT) > 0 ||
			pkAttacker->IsEquipUniqueGroup(UNIQUE_GROUP_AUTOLOOT))
		? true : false;
	// END_OF_ADD_PREMIUM

	PIXEL_POSITION pos;

	if (!isAutoLoot)
		if (!SECTREE_MANAGER::instance().GetMovablePosition(GetMapIndex(), GetX(), GetY(), pos))
			return;

	int iTotalGold = 0;
	//

	//
	int iGoldPercent = MobRankStats[GetMobRank()].iGoldPercent;

	//if (pkAttacker->IsPC())
	//	iGoldPercent = iGoldPercent * (100 + CPrivManager::instance().GetPriv(pkAttacker, PRIV_GOLD_DROP)) / 100;

	//if (pkAttacker->GetPoint(POINT_MALL_GOLDBONUS))
	//	iGoldPercent += (iGoldPercent * pkAttacker->GetPoint(POINT_MALL_GOLDBONUS) / 100);

	iGoldPercent = iGoldPercent * CHARACTER_MANAGER::instance().GetMobGoldDropRate(pkAttacker) / 100;

	// ADD_PREMIUM
	if (pkAttacker->GetPremiumRemainSeconds(PREMIUM_GOLD) > 0 ||
		pkAttacker->IsEquipUniqueGroup(UNIQUE_GROUP_LUCKY_GOLD))
		iGoldPercent += iGoldPercent;
	// END_OF_ADD_PREMIUM

	if (iGoldPercent > 100)
		iGoldPercent = 100;

	int iPercent;

	if (GetMobRank() >= MOB_RANK_BOSS)
		iPercent = ((iGoldPercent * PERCENT_LVDELTA_BOSS(pkAttacker->GetLevel(), GetLevel())) / 100);
	else
		iPercent = ((iGoldPercent * PERCENT_LVDELTA(pkAttacker->GetLevel(), GetLevel())) / 100);
	//int iPercent = CALCULATE_VALUE_LVDELTA(pkAttacker->GetLevel(), GetLevel(), iGoldPercent);

	if (number(1, 100) > iPercent)
		return;

	int iGoldMultipler = GOLD_MULTIPLER;

	if (1 == number(1, 50000))
		iGoldMultipler *= 10;
	else if (1 == number(1, 10000))
		iGoldMultipler *= 5;

	//if (pkAttacker->GetPoint(POINT_GOLD_DOUBLE_BONUS))
	//	if (number(1, 100) <= pkAttacker->GetPoint(POINT_GOLD_DOUBLE_BONUS))
	//		iGoldMultipler *= 2;

	if (test_server)
		pkAttacker->ChatPacket(CHAT_TYPE_PARTY, "gold_mul %d rate %d", iGoldMultipler, CHARACTER_MANAGER::instance().GetMobGoldAmountRate(pkAttacker));

	LPITEM item;

	int iGold10DropPct = 100;
	iGold10DropPct = (iGold10DropPct * 100) / (100 + CPrivManager::instance().GetPriv(pkAttacker, PRIV_GOLD10_DROP));

	if (GetMobRank() >= MOB_RANK_BOSS && !IsStone() && GetMobTable().dwGoldMax != 0)
	{
		if (1 == number(1, iGold10DropPct))
			iGoldMultipler *= 10;

		int iSplitCount = number(25, 35);

		for (int i = 0; i < iSplitCount; ++i)
		{
			int iGold = number(GetMobTable().dwGoldMin, GetMobTable().dwGoldMax) / iSplitCount;
			if (test_server)
				sys_log(0, "iGold %d", iGold);
			iGold = iGold * CHARACTER_MANAGER::instance().GetMobGoldAmountRate(pkAttacker) / 100;
			iGold *= iGoldMultipler;

			if (iGold == 0)
			{
				continue;
			}

			if (test_server)
			{
				sys_log(0, "Drop Moeny MobGoldAmountRate %d %d", CHARACTER_MANAGER::instance().GetMobGoldAmountRate(pkAttacker), iGoldMultipler);
				sys_log(0, "Drop Money gold %d GoldMin %d GoldMax %d", iGold, GetMobTable().dwGoldMax, GetMobTable().dwGoldMax);
			}

			if ((item = ITEM_MANAGER::instance().CreateItem(1, iGold)))
			{
				pos.x = GetX() + ((number(-14, 14) + number(-14, 14)) * 23);
				pos.y = GetY() + ((number(-14, 14) + number(-14, 14)) * 23);

				item->AddToGround(GetMapIndex(), pos);
				item->StartDestroyEvent();

				iTotalGold += iGold; // Total gold
			}
		}
	}

	else if (1 == number(1, iGold10DropPct))
	{
		//

		//
		for (int i = 0; i < 10; ++i)
		{
			int iGold = number(GetMobTable().dwGoldMin, GetMobTable().dwGoldMax);
			iGold = iGold * CHARACTER_MANAGER::instance().GetMobGoldAmountRate(pkAttacker) / 100;
			iGold *= iGoldMultipler;

			if (iGold == 0)
			{
				continue;
			}

			if ((item = ITEM_MANAGER::instance().CreateItem(1, iGold)))
			{
				pos.x = GetX() + (number(-7, 7) * 20);
				pos.y = GetY() + (number(-7, 7) * 20);

				item->AddToGround(GetMapIndex(), pos);
				item->StartDestroyEvent();

				iTotalGold += iGold; // Total gold
			}
		}
	}
	else
	{
		//

		//
		int iGold = number(GetMobTable().dwGoldMin, GetMobTable().dwGoldMax);
		iGold = iGold * CHARACTER_MANAGER::instance().GetMobGoldAmountRate(pkAttacker) / 100;
		iGold *= iGoldMultipler;

		int iSplitCount;

		if (iGold >= 3)
			iSplitCount = number(1, 3);
		else if (GetMobRank() >= MOB_RANK_BOSS)
		{
			iSplitCount = number(3, 10);

			if ((iGold / iSplitCount) == 0)
				iSplitCount = 1;
		}
		else
			iSplitCount = 1;

		if (iGold != 0)
		{
			iTotalGold += iGold; // Total gold

			for (int i = 0; i < iSplitCount; ++i)
			{
				if (isAutoLoot)
				{
					pkAttacker->GiveGold(iGold / iSplitCount);
				}
				else if ((item = ITEM_MANAGER::instance().CreateItem(1, iGold / iSplitCount)))
				{
					pos.x = GetX() + (number(-7, 7) * 20);
					pos.y = GetY() + (number(-7, 7) * 20);

					item->AddToGround(GetMapIndex(), pos);
					item->StartDestroyEvent();
				}
			}
		}
	}
}
#endif

#ifdef __ENABLE_KILL_EVENT_FIX__
LPCHARACTER CHARACTER::GetMostAttacked() {
#ifdef __DAMAGE_LIMIT_REWORK__
	long long iMostDam=-1;
#else
	int iMostDam=-1;
#endif
	LPCHARACTER pkChrMostAttacked = NULL;
	auto it = m_map_kDamage.begin();

	while (it != m_map_kDamage.end()){
		//* getting information from the iterator
		const VID & c_VID = it->first;
#ifdef __DAMAGE_LIMIT_REWORK__
		const long long iDam    = it->second.iTotalDamage;
#else
		const int iDam    = it->second.iTotalDamage;
#endif
		//* increasing the iterator
		++it;

		//* finding the character from his vid
		LPCHARACTER pAttacker = CHARACTER_MANAGER::instance().Find(c_VID);

		//* if the attacked is now offline
		if (!pAttacker)
			continue;

		//* if the attacker is not a player
		if(pAttacker->IsNPC())
			continue;

		//* if the player is too far
		// if(DISTANCE_APPROX(GetX()-pAttacker->GetX(), GetY()-pAttacker->GetY())>5000)
		// 	continue;

		if (pAttacker->GetMapIndex() != this->GetMapIndex())
			continue;

		if (iDam > iMostDam){
			pkChrMostAttacked = pAttacker;
			iMostDam = iDam;
		}
	}

	return pkChrMostAttacked;
}
#endif

void CHARACTER::Reward(bool bItemDrop)
{

	//PROF_UNIT puReward("Reward");
	LPCHARACTER pkAttacker = DistributeExp();

#ifdef __ENABLE_KILL_EVENT_FIX__
	if (!pkAttacker && !(pkAttacker = GetMostAttacked()))
	{
		if (GetDungeon())
		{
			sys_err("Dungeon anaymous mob dead:: name: %s randommember_name: %s", GetName(), GetDungeon()->GetRandomMember() ? GetDungeon()->GetRandomMember()->GetName() : "");
		}
		return;
	}
#else
	if (!pkAttacker)
		return;
#endif

	//PROF_UNIT pu1("r1");
	if (pkAttacker->IsPC())
	{
		if ((GetLevel() - pkAttacker->GetLevel()) >= -10)
		{
			if (pkAttacker->GetRealAlignment() < 0)
			{
				if (pkAttacker->IsEquipUniqueItem(UNIQUE_ITEM_FASTER_ALIGNMENT_UP_BY_KILL))
					pkAttacker->UpdateAlignment(14);
				else
					pkAttacker->UpdateAlignment(7);
			}
			else
				pkAttacker->UpdateAlignment(2);
		}

		pkAttacker->SetQuestNPCID(GetVID());
		quest::CQuestManager::instance().Kill(pkAttacker->GetPlayerID(), GetRaceNum());

#ifdef __BATTLE_PASS_SYSTEM__
		if (IsStone())
		{
			BYTE bBattlePassId = pkAttacker->GetBattlePassId();
			if (bBattlePassId)
			{
				DWORD dwMonsterVnum, dwToKillCount;
				if (CBattlePass::instance().BattlePassMissionGetInfo(bBattlePassId, STONE_KILL, &dwMonsterVnum, &dwToKillCount))
				{
					if (dwMonsterVnum == 0 && pkAttacker->GetMissionProgress(STONE_KILL, bBattlePassId) < dwToKillCount)
						pkAttacker->UpdateMissionProgress(STONE_KILL, bBattlePassId, 1, dwToKillCount);
					else if (dwMonsterVnum == GetRaceNum() && pkAttacker->GetMissionProgress(STONE_KILL, bBattlePassId) < dwToKillCount)
						pkAttacker->UpdateMissionProgress(STONE_KILL, bBattlePassId, 1, dwToKillCount);
				}

#ifdef ENABLE_BATTLE_PASS_EX
				BYTE nextMissionId = pkAttacker->GetNextMissionByType(METIN_KILL1, METIN_KILL50, bBattlePassId);
				if (nextMissionId != 0)
				{
					DWORD dwMonsterVnum, dwToKillCount;
					if (CBattlePass::instance().BattlePassMissionGetInfo(bBattlePassId, nextMissionId, &dwMonsterVnum, &dwToKillCount))
					{
						if (dwMonsterVnum == 0 && pkAttacker->GetMissionProgress(nextMissionId, bBattlePassId) < dwToKillCount)
							pkAttacker->UpdateMissionProgress(nextMissionId, bBattlePassId, 1, dwToKillCount);
						else if (dwMonsterVnum == GetRaceNum() && pkAttacker->GetMissionProgress(nextMissionId, bBattlePassId) < dwToKillCount)
							pkAttacker->UpdateMissionProgress(nextMissionId, bBattlePassId, 1, dwToKillCount);
					}
				}
#endif // ENABLE_BATTLE_PASS_EX
			}
		}
		else if (IsBoss())
		{
			if (IsBetaMapBoss())
			{
				BYTE bBattlePassId = pkAttacker->GetBattlePassId();
				if (bBattlePassId)
				{
					DWORD dwMonsterVnum, dwToKillCount;
					if (CBattlePass::instance().BattlePassMissionGetInfo(bBattlePassId, BETA_BOSS_KILL, &dwMonsterVnum, &dwToKillCount))
					{
						if (dwMonsterVnum == 0 && pkAttacker->GetMissionProgress(BETA_BOSS_KILL, bBattlePassId) < dwToKillCount)
							pkAttacker->UpdateMissionProgress(BETA_BOSS_KILL, bBattlePassId, 1, dwToKillCount);
						else if (dwMonsterVnum == GetRaceNum() && pkAttacker->GetMissionProgress(BETA_BOSS_KILL, bBattlePassId) < dwToKillCount)
							pkAttacker->UpdateMissionProgress(BETA_BOSS_KILL, bBattlePassId, 1, dwToKillCount);
					}
				}
			}
			else
			{
				BYTE bBattlePassId = pkAttacker->GetBattlePassId();
				if (bBattlePassId)
				{
					DWORD dwMonsterVnum, dwToKillCount;
					if (CBattlePass::instance().BattlePassMissionGetInfo(bBattlePassId, BOSS_KILL, &dwMonsterVnum, &dwToKillCount))
					{
						if (dwMonsterVnum == 0 && pkAttacker->GetMissionProgress(BOSS_KILL, bBattlePassId) < dwToKillCount)
							pkAttacker->UpdateMissionProgress(BOSS_KILL, bBattlePassId, 1, dwToKillCount);
						else if (dwMonsterVnum == GetRaceNum() && pkAttacker->GetMissionProgress(BOSS_KILL, bBattlePassId) < dwToKillCount)
							pkAttacker->UpdateMissionProgress(BOSS_KILL, bBattlePassId, 1, dwToKillCount);
					}
				}
			}
#ifdef ENABLE_BATTLE_PASS_EX
			if (IsDungeonBoss())
			{
				BYTE bBattlePassId = pkAttacker->GetBattlePassId();
				BYTE nextMissionId = pkAttacker->GetNextMissionByType(DUNGEON_DONE1, DUNGEON_DONE30, bBattlePassId);
				if (nextMissionId != 0)
				{
					DWORD dwDungeonID, dwCount;
					if (CBattlePass::instance().BattlePassMissionGetInfo(bBattlePassId, nextMissionId, &dwDungeonID, &dwCount))
					{
						if (pkAttacker->GetMissionProgress(nextMissionId, bBattlePassId) < dwCount)
							pkAttacker->UpdateMissionProgress(nextMissionId, bBattlePassId, 1, dwCount);
					}
				}
			}
#endif // ENABLE_BATTLE_PASS_EX
		}
		else
		{
			BYTE bBattlePassId = pkAttacker->GetBattlePassId();
			if (bBattlePassId)
			{
				DWORD dwMonsterVnum, dwToKillCount;
				if (CBattlePass::instance().BattlePassMissionGetInfo(bBattlePassId, MONSTER_KILL, &dwMonsterVnum, &dwToKillCount))
				{
					if (dwMonsterVnum == 0 && pkAttacker->GetMissionProgress(MONSTER_KILL, bBattlePassId) < dwToKillCount)
						pkAttacker->UpdateMissionProgress(MONSTER_KILL, bBattlePassId, 1, dwToKillCount);
					else if (dwMonsterVnum == GetRaceNum() && pkAttacker->GetMissionProgress(MONSTER_KILL, bBattlePassId) < dwToKillCount)
						pkAttacker->UpdateMissionProgress(MONSTER_KILL, bBattlePassId, 1, dwToKillCount);
				}

#ifdef ENABLE_BATTLE_PASS_EX
				BYTE nextMissionId = pkAttacker->GetNextMissionByType(MONSTER_KILL1, MONSTER_KILL30, bBattlePassId);
				if (nextMissionId != 0)
				{
					DWORD dwMonsterVnum, dwToKillCount;
					if (CBattlePass::instance().BattlePassMissionGetInfo(bBattlePassId, nextMissionId, &dwMonsterVnum, &dwToKillCount))
					{
						if (dwMonsterVnum == 0 && pkAttacker->GetMissionProgress(nextMissionId, bBattlePassId) < dwToKillCount)
							pkAttacker->UpdateMissionProgress(nextMissionId, bBattlePassId, 1, dwToKillCount);
						else if (dwMonsterVnum == GetRaceNum() && pkAttacker->GetMissionProgress(nextMissionId, bBattlePassId) < dwToKillCount)
							pkAttacker->UpdateMissionProgress(nextMissionId, bBattlePassId, 1, dwToKillCount);
					}
				}
#endif // ENABLE_BATTLE_PASS_EX
			}
		}

#endif


		if (!number(0, 9))
		{
			if (pkAttacker->GetPoint(POINT_KILL_HP_RECOVERY))
			{
				HPTYPE iHP = pkAttacker->GetMaxHP() * pkAttacker->GetPoint(POINT_KILL_HP_RECOVERY) / 100;
				pkAttacker->PointChange(POINT_HP, iHP);
				CreateFly(FLY_HP_SMALL, pkAttacker);
			}

			if (pkAttacker->GetPoint(POINT_KILL_SP_RECOVER))
			{
				int iSP = pkAttacker->GetMaxSP() * pkAttacker->GetPoint(POINT_KILL_SP_RECOVER) / 100;
				pkAttacker->PointChange(POINT_SP, iSP);
				CreateFly(FLY_SP_SMALL, pkAttacker);
			}
		}
	}

	if (!bItemDrop)
		return;

	PIXEL_POSITION pos = GetXYZ();

	if (!SECTREE_MANAGER::instance().GetMovablePosition(GetMapIndex(), pos.x, pos.y, pos))
		return;

#ifdef ENABLE_MULTI_FARM_BLOCK
	if (!pkAttacker->GetMultiStatus())
		return;
#endif

	if (test_server)
		sys_log(0, "Drop money : Attacker %s", pkAttacker->GetName());
	RewardGold(pkAttacker);

	LPITEM item;

	static std::vector<LPITEM> s_vec_item;
	s_vec_item.clear();

#ifdef __DROP_CH_RENEWAL__
	bool isAuto = IsStone();
#endif // __DROP_CH_RENEWAL__

	if (ITEM_MANAGER::instance().CreateDropItem(this, pkAttacker, s_vec_item))
	{
		if (s_vec_item.size() == 0);
		else if (s_vec_item.size() == 1)
		{
			item = s_vec_item[0];
			item->AddToGround(GetMapIndex(), pos);

#ifdef __DICE_SYSTEM__
			if (pkAttacker->GetParty())
			{
				FPartyDropDiceRoll f(item, pkAttacker);
				f.Process(this);
			}
			else
				item->SetOwnership(pkAttacker);
#else
			item->SetOwnership(pkAttacker);
#endif

			item->StartDestroyEvent();
#ifdef __DROP_CH_RENEWAL__
			if (isAuto)
				pkAttacker->PickupItem(item->GetVID(), true);
#endif // __DROP_CH_RENEWAL__

			pos.x = number(-7, 7) * 20;
			pos.y = number(-7, 7) * 20;
			pos.x += GetX();
			pos.y += GetY();

			//sys_log(0, "DROP_ITEM: %s %d %d from %s", item->GetName(), pos.x, pos.y, GetName());
		}
		else
		{
			int iItemIdx = s_vec_item.size() - 1;

			std::priority_queue<std::pair<long long, LPCHARACTER> > pq;
#ifdef __DAMAGE_LIMIT_REWORK__
			long long total_dam = 0;
#else
			int total_dam = 0;
#endif
			for (TDamageMap::iterator it = m_map_kDamage.begin(); it != m_map_kDamage.end(); ++it)
			{
#ifdef __DAMAGE_LIMIT_REWORK__
				long long iDamage = it->second.iTotalDamage;
#else
				int iDamage = it->second.iTotalDamage;
#endif
				if (iDamage > 0)
				{
					LPCHARACTER ch = CHARACTER_MANAGER::instance().Find(it->first);

					if (ch)
					{
						pq.push(std::make_pair(iDamage, ch));
						total_dam += iDamage;
					}
				}
			}

			std::vector<LPCHARACTER> v;


			while (!pq.empty() && pq.top().first >= GetMaxHP()/2)
			{
				v.push_back(pq.top().second);
				pq.pop();
			}


			if (v.empty())
			{
				while (iItemIdx >= 0)
				{
					item = s_vec_item[iItemIdx--];

					if (!item)
					{
						sys_err("item null in vector idx %d", iItemIdx + 1);
						continue;
					}

					item->AddToGround(GetMapIndex(), pos);

					item->SetOwnership(pkAttacker);
					item->StartDestroyEvent();

#ifdef __DROP_CH_RENEWAL__
					if (isAuto)
						pkAttacker->PickupItem(item->GetVID(), true);
#endif // __DROP_CH_RENEWAL__

					pos.x = number(-7, 7) * 20;
					pos.y = number(-7, 7) * 20;
					pos.x += GetX();
					pos.y += GetY();

					//sys_log(0, "DROP_ITEM: %s %d %d by %s", item->GetName(), pos.x, pos.y, GetName());
				}
			}
			else
			{
				std::vector<LPCHARACTER>::iterator it = v.begin();

				while (iItemIdx >= 0)
				{
					item = s_vec_item[iItemIdx--];

					if (!item)
					{
						sys_err("item null in vector idx %d", iItemIdx + 1);
						continue;
					}

					item->AddToGround(GetMapIndex(), pos);

					LPCHARACTER ch = *it;

					if (ch->GetParty())
						ch = ch->GetParty()->GetNextOwnership(ch, GetX(), GetY());

					++it;

					if (it == v.end())
						it = v.begin();

#ifdef __DICE_SYSTEM__
					if (ch->GetParty())
					{
						FPartyDropDiceRoll f(item, ch);
						f.Process(this);
					}
					else
						item->SetOwnership(ch);
#else
					item->SetOwnership(ch);
#endif

					item->StartDestroyEvent();

#ifdef __DROP_CH_RENEWAL__
					if (isAuto)
						ch->PickupItem(item->GetVID(), true);
#endif // __DROP_CH_RENEWAL__

					pos.x = number(-7, 7) * 20;
					pos.y = number(-7, 7) * 20;
					pos.x += GetX();
					pos.y += GetY();

					//sys_log(0, "DROP_ITEM: %s %d %d by %s", item->GetName(), pos.x, pos.y, GetName());
				}
			}
		}
	}

	m_map_kDamage.clear();
}

class FPartyAlignmentCompute
{
public:
	FPartyAlignmentCompute(int iAmount, int x, int y)
	{
		m_iAmount = iAmount;
		m_iCount = 0;
		m_iStep = 0;
		m_iKillerX = x;
		m_iKillerY = y;
	}

	void operator () (LPCHARACTER pkChr)
	{
		if (DISTANCE_APPROX(pkChr->GetX() - m_iKillerX, pkChr->GetY() - m_iKillerY) < PARTY_DEFAULT_RANGE)
		{
			if (m_iStep == 0)
			{
				++m_iCount;
			}
			else
			{
				pkChr->UpdateAlignment(m_iAmount / m_iCount);
			}
		}
	}

	int m_iAmount;
	int m_iCount;
	int m_iStep;

	int m_iKillerX;
	int m_iKillerY;
};

void CHARACTER::Dead(LPCHARACTER pkKiller, bool bImmediateDead)
{
	if (IsDead())
		return;

	if (!g_bDisableStopWhenRidingDie)
	{
		if (IsHorseRiding())
		{
			StopRiding();
		}
		else if (GetMountVnum())
		{
#ifdef __MOUNT_COSTUME_SYSTEM__
			LPITEM wearCostumeMount = GetWear(WEAR_COSTUME_MOUNT);
			if (wearCostumeMount)
				MountUnsummon(wearCostumeMount);
			RemoveAffect(AFFECT_MOUNT);
			RemoveAffect(AFFECT_MOUNT_BONUS);
#endif
			m_dwMountVnum = 0;
			UnEquipSpecialRideUniqueItem();

			UpdatePacket();
		}
	}

	if (!pkKiller && m_dwKillerPID)
		pkKiller = CHARACTER_MANAGER::instance().FindByPID(m_dwKillerPID);

	m_dwKillerPID = 0;

	// @duzenleme neden boyle birsey yaptik bilmiyorum orjinalde boyle birsey yok.
	if (IsPC())
		RemoveBadAffect();

	bool isAgreedPVP = false;
	bool isUnderGuildWar = false;

	if (pkKiller && pkKiller->IsPC())
	{
		if (pkKiller->m_pkChrTarget == this)
			pkKiller->SetTarget(NULL);

		if (!IsPC() && pkKiller->GetDungeon())
			pkKiller->GetDungeon()->IncKillCount(pkKiller, this);

		isAgreedPVP = CPVPManager::instance().Dead(this, pkKiller->GetPlayerID());
		if (IsPC())
		{
			CGuild* g1 = GetGuild();
			CGuild* g2 = pkKiller->GetGuild();

			if (g1 && g2)
				if (g1->UnderWar(g2->GetID()))
					isUnderGuildWar = true;
			pkKiller->SetQuestNPCID(GetVID());
			quest::CQuestManager::instance().Kill(pkKiller->GetPlayerID(), quest::QUEST_NO_NPC);
			CGuildManager::instance().Kill(pkKiller, this);
		}
	}

	SetPosition(POS_DEAD);

#ifdef __NOT_BUFF_CLEAR__
	ClearAffect(true, true);
#else
	ClearAffect(true);
#endif

	if (pkKiller && IsPC())
	{
		if (!pkKiller->IsPC())
		{
			sys_log(1, "DEAD: %s %p WITH PENALTY", GetName(), this);
			SET_BIT(m_pointsInstant.instant_flag, INSTANT_FLAG_DEATH_PENALTY);
		}
		else
		{
			sys_log(1, "DEAD_BY_PC: %s %p KILLER %s %p", GetName(), this, pkKiller->GetName(), get_pointer(pkKiller));
			REMOVE_BIT(m_pointsInstant.instant_flag, INSTANT_FLAG_DEATH_PENALTY);

			if (GetEmpire() != pkKiller->GetEmpire())
			{
				// pass...
			}
			else
			{
				if (pkKiller != this) // derece dusme fix
				{
					if (!isAgreedPVP && !isUnderGuildWar)
					{
						if (!IsGM() && GetRealAlignment() < 0 && GetLevel() >= 50 && !GetMyShop())
						{
							ItemDropPenalty(pkKiller);
						}
						if (!IsKillerMode() && GetAlignment() >= 0)
						{
							int iNoPenaltyProb = 0;

							if (pkKiller->GetAlignment() >= 0)	// 1/3 percent down
								iNoPenaltyProb = 33;
							else				// 4/5 percent down
								iNoPenaltyProb = 20;

							if (number(1, 100) < iNoPenaltyProb)
								pkKiller->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¿ë½ÅÀÇ º¸È£·Î ¾ÆÀÌÅÛÀÌ ¶³¾îÁöÁö ¾Ê¾Ò½À´Ï´Ù."));
							else
							{
								if (pkKiller->GetParty())
								{
									FPartyAlignmentCompute f(-20000, pkKiller->GetX(), pkKiller->GetY());
									pkKiller->GetParty()->ForEachOnlineMember(f);

									if (f.m_iCount == 0)
										pkKiller->UpdateAlignment(-20000);
									else
									{
										sys_log(0, "ALIGNMENT PARTY count %d amount %d", f.m_iCount, f.m_iAmount);

										f.m_iStep = 1;
										pkKiller->GetParty()->ForEachOnlineMember(f);
									}
								}
								else
									pkKiller->UpdateAlignment(-20000);
							}
						}

					}
				}
			}

#ifdef __BATTLE_PASS_SYSTEM__
			BYTE bBattlePassId = pkKiller->GetBattlePassId();
			if (bBattlePassId)
			{
				DWORD dwMinLevel, dwToKillCount;
				if (CBattlePass::instance().BattlePassMissionGetInfo(bBattlePassId, PLAYER_KILL, &dwMinLevel, &dwToKillCount))
				{
					if (GetLevel() >= dwMinLevel && pkKiller->GetMissionProgress(PLAYER_KILL, bBattlePassId) < dwToKillCount)
						pkKiller->UpdateMissionProgress(PLAYER_KILL, bBattlePassId, 1, dwToKillCount);
				}
			}
#endif
		}
	}
	else
	{
		sys_log(1, "DEAD: %s %p", GetName(), this);
		REMOVE_BIT(m_pointsInstant.instant_flag, INSTANT_FLAG_DEATH_PENALTY);
	}

	ClearSync();

	//sys_log(1, "stun cancel %s[%d]", GetName(), (DWORD)GetVID());
	event_cancel(&m_pkStunEvent);

	if (IsPC())
	{
		m_dwLastDeadTime = get_dword_time();
		SetKillerMode(false);
		GetDesc()->SetPhase(PHASE_DEAD);
	}
	else
	{
		if (!IS_SET(m_pointsInstant.instant_flag, INSTANT_FLAG_NO_REWARD))
		{
			if (!(pkKiller && pkKiller->IsPC() && pkKiller->GetGuild() && pkKiller->GetGuild()->UnderAnyWar(GUILD_WAR_TYPE_FIELD)))
			{
				if (m_pkMobData != NULL && GetMobTable().dwResurrectionVnum) // @duzenleme buraya bunu koymak yerine sebebine bakilmasi lazim mobdata olmamasi pek mumkun degil cunku.
				{
					// DUNGEON_MONSTER_REBIRTH_BUG_FIX
					LPCHARACTER chResurrect = CHARACTER_MANAGER::instance().SpawnMob(GetMobTable().dwResurrectionVnum, GetMapIndex(), GetX(), GetY(), GetZ(), true, (int)GetRotation());
					if (GetDungeon() && chResurrect)
					{
						chResurrect->SetDungeon(GetDungeon());
					}
					// END_OF_DUNGEON_MONSTER_REBIRTH_BUG_FIX

					Reward(false);
				}
				else if (IsRevive() == true)
					Reward(false);
				else
#ifdef __DUNGEON_TEST_MODE__
					Reward(!(GetDungeon() && GetDungeon()->IsTest())); // Drops gold, item, etc..
#else // __DUNGEON_TEST_MODE__
					Reward(true); // Drops gold, item, etc..
#endif // __DUNGEON_TEST_MODE__
			}
			else
			{
				if (pkKiller->m_dwUnderGuildWarInfoMessageTime < get_dword_time())
				{
					pkKiller->m_dwUnderGuildWarInfoMessageTime = get_dword_time() + 60000;
					pkKiller->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<±æµå> ±æµåÀüÁß¿¡´Â »ç³É¿¡ µû¸¥ ÀÌÀÍÀÌ ¾ø½À´Ï´Ù."));
				}
			}
		}
	}

	TPacketGCDead pack;
	pack.header = HEADER_GC_DEAD;
	pack.vid = m_vid;
	PacketAround(&pack, sizeof(pack));

	REMOVE_BIT(m_pointsInstant.instant_flag, INSTANT_FLAG_STUN);

	if (GetDesc() != NULL) {
		itertype(m_list_pkAffect) it = m_list_pkAffect.begin();

		while (it != m_list_pkAffect.end())
			SendAffectAddPacket(GetDesc(), *it++);
	}

#ifdef ENABLE_PLAYER_STATISTICS
	if (pkKiller && pkKiller->IsPC())
	{
		if (IsPC())
		{
			if (GetEmpire() == 1)
				pkKiller->AddToKilledShinsoo();
			else if (GetEmpire() == 2)
				pkKiller->AddToKilledChunjo();
			else if (GetEmpire() == 3)
				pkKiller->AddToKilledJinno();

			pkKiller->AddToTotalKill();
		}
		else if (IsStone())
		{
			pkKiller->AddToKilledStone();
		}
		else if (IsBoss())
		{
			pkKiller->AddToKilledBoss();
		}
		else if (IsMonster())
		{
			pkKiller->AddToKilledMonster();
		}

		if (IsPC())
			SendPlayerStatisticsPacket();
		pkKiller->SendPlayerStatisticsPacket();
	}
#endif

	if (m_pkDeadEvent)
	{
		sys_log(1, "DEAD_EVENT_CANCEL: %s %p %p", GetName(), this, get_pointer(m_pkDeadEvent));
		event_cancel(&m_pkDeadEvent);
	}

	if (IsStone())
		ClearStone();

	if (GetDungeon())
		GetDungeon()->DeadCharacter(this);

	SCharDeadEventInfo* pEventInfo = AllocEventInfo<SCharDeadEventInfo>();

	if (IsPC())
	{
		pEventInfo->isPC = true;
		pEventInfo->dwID = this->GetPlayerID();

		m_pkDeadEvent = event_create(dead_event, pEventInfo, PASSES_PER_SEC(180));
	}
	else
	{
		pEventInfo->isPC = false;
		pEventInfo->dwID = this->GetVID();

		if (IsRevive() == false && HasReviverInParty() == true)
			m_pkDeadEvent = event_create(dead_event, pEventInfo, bImmediateDead ? 1 : PASSES_PER_SEC(0));
		else
			m_pkDeadEvent = event_create(dead_event, pEventInfo, bImmediateDead ? 1 : PASSES_PER_SEC(0));
	}

	sys_log(1, "DEAD_EVENT_CREATE: %s %p %p", GetName(), this, get_pointer(m_pkDeadEvent));

	if (m_pkExchange != NULL)
		m_pkExchange->Cancel();

	if (IsCubeOpen() == true)
		Cube_close(this);

#ifdef __ACCE_SYSTEM__
	if (IsPC())
		CloseAcce();
#endif

#ifdef __ITEM_CHANGELOOK__
	if (IsPC())
		ChangeLookWindow(false, true);
#endif

	CShopManager::instance().StopShopping(this);
	CloseMyShop();
	CloseSafebox();
#ifdef __REGEN_REWORK__
	if (!IsPC() && !GetDungeon())
	{
		if (GetRegen() != NULL)
			regen_event_create(GetRegen());
	}
#endif
}

struct TItemDropPenalty
{
	int iInventoryPct;		// Range: 1 ~ 100
	int iInventoryQty;		// Range: -- count
	int iEquipmentPct;		// Range: 1 ~ 100
	int iEquipmentQty;		// Range: -- count
};
std::array<TItemDropPenalty, 3> aItemDropPenalty_kor =
{{
	{	10,	4,	20,	2 }, // 14
	{	14,	8,	30,	2 }, // 15
	{	20,	16,	40,	2 }, // 16
}};

void CHARACTER::ItemDropPenalty(LPCHARACTER pkKiller)
{
	return;

	int iAlignGrade = GetAlignGrade();

	if (iAlignGrade < 14)
		return;

	std::vector<std::pair<LPITEM, int> > vec_item;
	LPITEM pkItem;
	int	i;
	bool isDropAllEquipments = false;

	TItemDropPenalty & r = aItemDropPenalty_kor.at(iAlignGrade - 14);
	sys_log(0, "%s align %d inven_pct %d equip_pct %d", GetName(), iAlignGrade, r.iInventoryPct, r.iEquipmentPct);

	bool bDropInventory = r.iInventoryPct >= number(1, 100);
	bool bDropEquipment = r.iEquipmentPct >= number(1, 100);
	bool bDropAntiDropUniqueItem = false;

	if ((bDropInventory || bDropEquipment) && IsEquipUniqueItem(UNIQUE_ITEM_SKIP_ITEM_DROP_PENALTY))
	{
		bDropInventory = false;
		bDropEquipment = false;
		bDropAntiDropUniqueItem = true;
	}

	if (bDropInventory) // Drop Inventory
	{
		std::vector<BYTE> vec_bSlots;

		for (i = 0; i < INVENTORY_MAX_NUM; ++i)
		{
			if (!(pkItem = GetInventoryItem(i)))
				continue;

			if (IS_SET(pkItem->GetAntiFlag(), ITEM_ANTIFLAG_GIVE | ITEM_ANTIFLAG_PKDROP))
				continue;

			if (pkItem->IsBasicItem())
				continue;

			if (pkItem->IsNewPetItem())
				continue;

			if (pkItem->IsExchanging())
				continue;

			vec_bSlots.push_back(i);
		}

		if (!vec_bSlots.empty())
		{
			random_shuffle(vec_bSlots.begin(), vec_bSlots.end());

			int iQty = MIN(vec_bSlots.size(), r.iInventoryQty);

			if (iQty)
				iQty = number(1, iQty);

			for (i = 0; i < iQty; ++i)
			{
				pkItem = GetInventoryItem(vec_bSlots[i]);

				SyncQuickslot(QUICKSLOT_TYPE_ITEM, vec_bSlots[i], 255);
				vec_item.push_back(std::make_pair(pkItem->RemoveFromCharacter(), INVENTORY));
			}
		}
	}

	if (bDropEquipment) // Drop Equipment
	{
		std::vector<BYTE> vec_bSlots;

		for (i = 0; i < WEAR_MAX_NUM; ++i)
		{
			if (!(pkItem = GetWear(i)))
				continue;

			if (IS_SET(pkItem->GetAntiFlag(), ITEM_ANTIFLAG_GIVE | ITEM_ANTIFLAG_PKDROP))
				continue;

			if (pkItem->IsBasicItem())
				continue;

			if (pkItem->IsNewPetItem())
				continue;

			if (pkItem->IsExchanging())
				continue;

			vec_bSlots.push_back(i);
		}

		if (!vec_bSlots.empty())
		{
			random_shuffle(vec_bSlots.begin(), vec_bSlots.end());
			int iQty;

			if (isDropAllEquipments)
				iQty = vec_bSlots.size();
			else
				iQty = MIN(vec_bSlots.size(), number(1, r.iEquipmentQty));

			if (iQty)
				iQty = number(1, iQty);

			for (i = 0; i < iQty; ++i)
			{
				if (!(pkItem = GetWear(vec_bSlots[i])))
					continue;

				SyncQuickslot(QUICKSLOT_TYPE_ITEM, vec_bSlots[i], 255);
				vec_item.push_back(std::make_pair(pkItem->RemoveFromCharacter(), EQUIPMENT));
			}
		}
	}

	if (bDropAntiDropUniqueItem)
	{
		LPITEM pkItem;

		pkItem = GetWear(WEAR_UNIQUE1);

		if (pkItem && pkItem->GetVnum() == UNIQUE_ITEM_SKIP_ITEM_DROP_PENALTY)
		{
			SyncQuickslot(QUICKSLOT_TYPE_ITEM, WEAR_UNIQUE1, 255);
			vec_item.push_back(std::make_pair(pkItem->RemoveFromCharacter(), EQUIPMENT));
		}

		pkItem = GetWear(WEAR_UNIQUE2);

		if (pkItem && pkItem->GetVnum() == UNIQUE_ITEM_SKIP_ITEM_DROP_PENALTY)
		{
			SyncQuickslot(QUICKSLOT_TYPE_ITEM, WEAR_UNIQUE2, 255);
			vec_item.push_back(std::make_pair(pkItem->RemoveFromCharacter(), EQUIPMENT));
		}

		pkItem = GetWear(WEAR_COSTUME_MOUNT);

		if (pkItem && pkItem->GetVnum() == UNIQUE_ITEM_SKIP_ITEM_DROP_PENALTY)
		{
			SyncQuickslot(QUICKSLOT_TYPE_ITEM, WEAR_COSTUME_MOUNT, 255);
			vec_item.push_back(std::make_pair(pkItem->RemoveFromCharacter(), EQUIPMENT));
		}
	}

	{
		PIXEL_POSITION pos;
		pos.x = GetX();
		pos.y = GetY();

		size_t i;

		for (i = 0; i < vec_item.size(); ++i)
		{
			LPITEM item = vec_item[i].first;
			//int window = vec_item[i].second;

			item->AddToGround(GetMapIndex(), pos);
			item->SetOwnership(pkKiller);
			item->StartDestroyEvent();

			//sys_log(0, "DROP_ITEM_PK: %s %d %d from %s", item->GetName(), pos.x, pos.y, GetName());
			// LogManager::instance().ItemLog(this, item, "DEAD_DROP", (window == INVENTORY) ? "INVENTORY" : ((window == EQUIPMENT) ? "EQUIPMENT" : ""));

			pos.x = GetX() + number(-7, 7) * 20;
			pos.y = GetY() + number(-7, 7) * 20;
		}
	}
}

struct FuncSetLastAttacked
{
	FuncSetLastAttacked(DWORD dwTime) : m_dwTime(dwTime)
	{
	}

	void operator () (LPCHARACTER ch)
	{
		ch->SetLastAttacked(m_dwTime);
	}

	DWORD m_dwTime;
};

void CHARACTER::SetLastAttacked(DWORD dwTime)
{
	assert(m_pkMobInst != NULL);

	m_pkMobInst->m_dwLastAttackedTime = dwTime;
	m_pkMobInst->m_posLastAttacked = GetXYZ();
}

#ifdef ENABLE_AUTO_HUNT_SYSTEM
DWORD CHARACTER::GetLastAttacked()
{
	assert(m_pkMobInst != NULL);

	return m_pkMobInst->m_dwLastAttackedTime;
}
#endif // ENABLE_AUTO_HUNT_SYSTEM

#ifdef __DAMAGE_LIMIT_REWORK__
void CHARACTER::SendDamagePacket(LPCHARACTER pAttacker, long long Damage, BYTE DamageFlag)
#else
void CHARACTER::SendDamagePacket(LPCHARACTER pAttacker, int Damage, BYTE DamageFlag)
#endif
{
	if (IsPC() == true || (pAttacker->IsPC() == true && pAttacker->GetTarget() == this))
	{
		TPacketGCDamageInfo damageInfo;
		memset(&damageInfo, 0, sizeof(TPacketGCDamageInfo));

		damageInfo.header = HEADER_GC_DAMAGE_INFO;
		damageInfo.dwVID = (DWORD)GetVID();
		damageInfo.flag = DamageFlag;
		// std::min ile ugrasmak istemedim cahil degilim biliyom daha iyisi yapilabilir.
		if (Damage < 0)
			Damage = 0;
		damageInfo.damage = Damage;

		if (GetDesc() != NULL)
		{
			GetDesc()->Packet(&damageInfo, sizeof(TPacketGCDamageInfo));
		}

		if (pAttacker->GetDesc() != NULL)
		{
			pAttacker->GetDesc()->Packet(&damageInfo, sizeof(TPacketGCDamageInfo));
		}
	}
}

#ifdef __DAMAGE_LIMIT_REWORK__
bool CHARACTER::Damage(LPCHARACTER pAttacker, long long dam, EDamageType type) // returns true if dead
#else
bool CHARACTER::Damage(LPCHARACTER pAttacker, int dam, EDamageType type) // returns true if dead
#endif
{
	if (!pAttacker) // @duzenleme pkAttacker yoksa demisiz fakat olmayadabilir yani o yuzden bu arkadaslari inputlara tasimamiz lazim.
		return false;

#if defined(__GROWTH_PET_SYSTEM__) || defined(__GROWTH_MOUNT_SYSTEM__)
	if (IsImmortal())
		return false;
#endif

#ifdef __DIS_DAMAGE_TYPE_NORMAL__
	if (pAttacker->IsPC() && pAttacker->GetPart(PART_WEAPON) == 0)
		return false;

	if (pAttacker->IsPC() && !pAttacker->GetWear(WEAR_WEAPON))
		return false;
#endif

	if (!pAttacker->IsPC() && IsPC())// slotlar hasar atamasýn
		return false;

	if (!GetSectree() || IsInSafezone()) // @duzenleme buralar hosuma gitmiyor dedigim gibi.
		return false;

	if (IsPC() && (IsObserverMode() || IsDead())) // @duzenleme Observer kisinin damage gonderememesini saglar ama bence pvp vs. class uzerinde bunun kontrolu mevcut gereksiz yani...
		return false;

	if (!IsPC() && DISTANCE_APPROX(GetX() - pAttacker->GetX(), GetY() - pAttacker->GetY()) > 300)
		return false;

	if (DAMAGE_TYPE_MAGIC == type)
		dam = (long long)((float)dam * (100 + (pAttacker->GetPoint(POINT_MAGIC_ATT_BONUS_PER) + pAttacker->GetPoint(POINT_MELEE_MAGIC_ATT_BONUS_PER))) / 100.f + 0.5f);

	if (type != DAMAGE_TYPE_NORMAL && type != DAMAGE_TYPE_NORMAL_RANGE)
	{
		if (IsAffectFlag(AFF_TERROR))
		{
			int pct = GetSkillPower(SKILL_TERROR) / 400;

			if (number(1, 100) <= pct)
				return false;
		}
	}

	HPTYPE iCurHP = GetHP();
	int iCurSP = GetSP();

	bool IsCritical = false;
	bool IsPenetrate = false;

	if (type == DAMAGE_TYPE_MELEE || type == DAMAGE_TYPE_RANGE || type == DAMAGE_TYPE_MAGIC)
	{
		if (pAttacker)
		{
			int iCriticalPct = pAttacker->GetPoint(POINT_CRITICAL_PCT);

			if (iCriticalPct)
			{
				if (iCriticalPct >= 10)
					iCriticalPct = 5 + (iCriticalPct - 10) / 4;
				else
					iCriticalPct /= 2;

				iCriticalPct -= GetPoint(POINT_RESIST_CRITICAL) / 3;

				if (iCriticalPct > 50)
					iCriticalPct = 50;

				if (number(1, 100) <= iCriticalPct)
				{
					IsCritical = true;
					dam *= 2;
					EffectPacket(SE_CRITICAL);

					if (IsAffectFlag(AFF_MANASHIELD))
						RemoveAffect(AFF_MANASHIELD);
				}
			}

			int iPenetratePct = pAttacker->GetPoint(POINT_PENETRATE_PCT);

			if (iPenetratePct)
			{
				if (iPenetratePct >= 10)
				{
					iPenetratePct = 5 + (iPenetratePct - 10) / 4;
				}
				else
				{
					iPenetratePct /= 2;
				}

				iPenetratePct -= GetPoint(POINT_RESIST_PENETRATE);

				if (number(1, 100) <= iPenetratePct)
				{
					IsPenetrate = true;

					if (test_server)
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("°üÅë Ãß°¡ µ¥¹ÌÁö %d"), GetPoint(POINT_DEF_GRADE) * (100 + GetPoint(POINT_DEF_BONUS)) / 100);

					dam += GetPoint(POINT_DEF_GRADE) * (100 + GetPoint(POINT_DEF_BONUS)) / 100;

					if (IsAffectFlag(AFF_MANASHIELD))
						RemoveAffect(AFF_MANASHIELD);

				}
			}
		}
	}
	else if (type == DAMAGE_TYPE_NORMAL || type == DAMAGE_TYPE_NORMAL_RANGE)
	{
		if (type == DAMAGE_TYPE_NORMAL)
		{
			if (GetPoint(POINT_BLOCK) && number(1, 100) <= MINMAX(0, GetPoint(POINT_BLOCK), 50))
			{
				if (test_server)
				{
					pAttacker->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%s ºí·°! (%d%%)"), GetName(), GetPoint(POINT_BLOCK));
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%s ºí·°! (%d%%)"), GetName(), GetPoint(POINT_BLOCK));
				}

				SendDamagePacket(pAttacker, 0, DAMAGE_BLOCK);
				return false;
			}
		}
		else if (type == DAMAGE_TYPE_NORMAL_RANGE)
		{
			if (GetPoint(POINT_DODGE) && number(1, 100) <= GetPoint(POINT_DODGE))
			{
				if (test_server)
				{
					pAttacker->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%s È¸ÇÇ! (%d%%)"), GetName(), GetPoint(POINT_DODGE));
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%s È¸ÇÇ! (%d%%)"), GetName(), GetPoint(POINT_DODGE));
				}

				SendDamagePacket(pAttacker, 0, DAMAGE_DODGE);
				return false;
			}
		}

		if (IsAffectFlag(AFF_JEONGWIHON))
			dam = (long long)(dam * (100 + GetSkillPower(SKILL_JEONGWI) * 25 / 100) / 100);

		if (IsAffectFlag(AFF_TERROR))
			dam = (long long)(dam * (95 - GetSkillPower(SKILL_TERROR) / 5) / 100);

		if (IsAffectFlag(AFF_HOSIN))
			dam = dam * (100 - GetPoint(POINT_RESIST_NORMAL_DAMAGE)) / 100;

		if (pAttacker)
		{
			if (type == DAMAGE_TYPE_NORMAL)
			{
				if (GetPoint(POINT_REFLECT_MELEE))
				{
#ifdef __DAMAGE_LIMIT_REWORK__
					long long reflectDamage = dam * GetPoint(POINT_REFLECT_MELEE) / 100;
#else
					int reflectDamage = dam * GetPoint(POINT_REFLECT_MELEE) / 100;
#endif
					if (pAttacker->IsImmune(IMMUNE_REFLECT))
						reflectDamage = (long long)(reflectDamage / 3.0f + 0.5f);

					pAttacker->Damage(this, reflectDamage, DAMAGE_TYPE_SPECIAL);
				}
			}

			int iCriticalPct = pAttacker->GetPoint(POINT_CRITICAL_PCT);

			if (iCriticalPct)
			{
				//iCriticalPct -= GetPoint(POINT_RESIST_CRITICAL) / 3;

				if (number(1, 100) <= iCriticalPct)
				{
					IsCritical = true;
					dam *= 2;
					EffectPacket(SE_CRITICAL);
				}
			}

			int iPenetratePct = pAttacker->GetPoint(POINT_PENETRATE_PCT);

			if (iPenetratePct)
			{
				iPenetratePct -= GetPoint(POINT_RESIST_PENETRATE);

				if (number(1, 100) <= iPenetratePct)
				{
					IsPenetrate = true;

					if (test_server)
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("°üÅë Ãß°¡ µ¥¹ÌÁö %d"), GetPoint(POINT_DEF_GRADE) * (100 + GetPoint(POINT_DEF_BONUS)) / 100);
					dam += GetPoint(POINT_DEF_GRADE) * (100 + GetPoint(POINT_DEF_BONUS)) / 100;
				}
			}

			if (pAttacker->GetPoint(POINT_STEAL_HP))
			{
				int pct = 1;

				if (number(1, 10) <= pct)
				{
					HPTYPE iHP = MIN(dam, MAX(0, iCurHP)) * pAttacker->GetPoint(POINT_STEAL_HP) / 100;

					if (iHP > 0 && GetHP() >= iHP)
					{
						CreateFly(FLY_HP_SMALL, pAttacker);
						pAttacker->PointChange(POINT_HP, iHP);
						PointChange(POINT_HP, -iHP);
					}
				}
			}

			if (pAttacker->GetPoint(POINT_STEAL_SP))
			{
				int pct = 1;

				if (number(1, 10) <= pct)
				{
					int iCur;

					if (IsPC())
						iCur = iCurSP;
					else
						iCur = iCurHP;

					int iSP = MIN(dam, MAX(0, iCur)) * pAttacker->GetPoint(POINT_STEAL_SP) / 100;

					if (iSP > 0 && iCur >= iSP)
					{
						CreateFly(FLY_SP_SMALL, pAttacker);
						pAttacker->PointChange(POINT_SP, iSP);

						if (IsPC())
							PointChange(POINT_SP, -iSP);
					}
				}
			}

			if (pAttacker->GetPoint(POINT_STEAL_GOLD))
			{
				if (number(1, 100) <= pAttacker->GetPoint(POINT_STEAL_GOLD))
				{
#ifdef __GOLD_LIMIT_REWORK__
					long long iAmount = number(1, GetLevel());
#else
					int iAmount = number(1, GetLevel());
#endif
					if (pAttacker->GetGold() < GOLD_MAX)
						pAttacker->PointChange(POINT_GOLD, iAmount);
				}
			}

			if (pAttacker->GetPoint(POINT_HIT_HP_RECOVERY) && number(0, 4) > 0)
			{
				HPTYPE i = ((iCurHP >= 0) ? MIN(dam, iCurHP) : dam) * pAttacker->GetPoint(POINT_HIT_HP_RECOVERY) / 100; //@fixme107

				if (i > 0)
				{
					CreateFly(FLY_HP_SMALL, pAttacker);
					pAttacker->PointChange(POINT_HP, i);
				}
			}

			if (pAttacker->GetPoint(POINT_HIT_SP_RECOVERY) && number(0, 4) > 0)
			{
				int i = ((iCurHP >= 0) ? MIN(dam, iCurHP) : dam) * pAttacker->GetPoint(POINT_HIT_SP_RECOVERY) / 100; //@fixme107

				if (i > 0)
				{
					CreateFly(FLY_SP_SMALL, pAttacker);
					pAttacker->PointChange(POINT_SP, i);
				}
			}

			if (pAttacker->GetPoint(POINT_MANA_BURN_PCT))
			{
				if (number(1, 100) <= pAttacker->GetPoint(POINT_MANA_BURN_PCT))
					PointChange(POINT_SP, -50);
			}
		}
	}

	switch (type)
	{
		case DAMAGE_TYPE_NORMAL:
		case DAMAGE_TYPE_NORMAL_RANGE:
			if (pAttacker)
				if (pAttacker->GetPoint(POINT_NORMAL_HIT_DAMAGE_BONUS) && !IsPC())
					dam = dam * (100 + pAttacker->GetPoint(POINT_NORMAL_HIT_DAMAGE_BONUS)) / 100;

			if (!IsPC())
				dam = dam * (100 - MIN(99, GetPoint(POINT_NORMAL_HIT_DEFEND_BONUS))) / 100;
			break;

	case DAMAGE_TYPE_MELEE:
	case DAMAGE_TYPE_RANGE:
	case DAMAGE_TYPE_FIRE:
	case DAMAGE_TYPE_ICE:
	case DAMAGE_TYPE_ELEC:
	case DAMAGE_TYPE_MAGIC:
		if (pAttacker)
		{
			int iSkillDamageBonus = pAttacker->GetPoint(POINT_SKILL_DAMAGE_BONUS);
			CSkillProto* pkSk = CSkillManager::instance().Get(SKILL_ADD_HP);

			if (NULL != pkSk)
			{
				pkSk->SetPointVar("k", 1.0f * GetSkillPower(SKILL_ADD_HP) / 100.0f);
				iSkillDamageBonus += static_cast<int>(pkSk->kPointPoly.Eval());
			}
			if (iSkillDamageBonus)
				dam = dam * (100 + pAttacker->GetPoint(POINT_SKILL_DAMAGE_BONUS)) / 100;
		}
		dam = dam * (100 - MIN(93, GetPoint(POINT_SKILL_DEFEND_BONUS))) / 100;// Beceriye karþý koyma %100 ü geçince hasar yememe fix.
		break;

	default:
		break;
	}

	if (IsAffectFlag(AFF_MANASHIELD))
	{
#ifdef __DAMAGE_LIMIT_REWORK__
		long long iDamageSPPart = dam / 10;
#else
		int iDamageSPPart = dam / 3;
#endif
		int iDamageToSP = iDamageSPPart * GetPoint(POINT_MANASHIELD) / 100;
		int iSP = GetSP();

		if (iDamageToSP <= iSP)
		{
			PointChange(POINT_SP, -iDamageToSP);
			dam -= iDamageSPPart;
		}
		else
		{
			PointChange(POINT_SP, -GetSP());
			dam -= iSP * 100 / MAX(GetPoint(POINT_MANASHIELD), 1);
		}
	}

	if (GetPoint(POINT_MALL_DEFBONUS) > 0)
	{
#ifdef __DAMAGE_LIMIT_REWORK__
		long long dec_dam = MIN(200, dam * GetPoint(POINT_MALL_DEFBONUS) / 100);
#else
		int dec_dam = MIN(200, dam * GetPoint(POINT_MALL_DEFBONUS) / 100);
#endif
		dam -= dec_dam;
	}

	if (pAttacker)
	{
		if (pAttacker->IsPC())
		{
			int iEmpire = pAttacker->GetEmpire();
			long lMapIndex = pAttacker->GetMapIndex();
			int iMapEmpire = SECTREE_MANAGER::instance().GetEmpireFromMapIndex(lMapIndex);

			if (iEmpire && iMapEmpire && iEmpire != iMapEmpire)
				dam = dam * 9 / 10;

			if (!IsPC() && GetMonsterDrainSPPoint())
			{
				int iDrain = GetMonsterDrainSPPoint();

				if (iDrain <= pAttacker->GetSP())
					pAttacker->PointChange(POINT_SP, -iDrain);
				else
				{
					int iSP = pAttacker->GetSP();
					pAttacker->PointChange(POINT_SP, -iSP);
				}
			}
		}
		else if (pAttacker->IsGuardNPC())
		{
			SET_BIT(m_pointsInstant.instant_flag, INSTANT_FLAG_NO_REWARD);
			Stun();
			return true;
		}
	}

	if (!IsPC())
	{
		if (m_pkParty && m_pkParty->GetLeader())
			m_pkParty->GetLeader()->SetLastAttacked(get_dword_time());
		else
			SetLastAttacked(get_dword_time());

		MonsterChat(MONSTER_CHAT_ATTACKED);
	}

	if (IsStun())
	{
		Dead(pAttacker);
		return true;
	}

	if (IsDead())
		return true;

	if (type == DAMAGE_TYPE_POISON)
	{
		if (GetHP() - dam <= 0)
		{
			dam = GetHP() - 1;
		}
	}
#ifdef __WOLFMAN_CHARACTER__
	else if (type == DAMAGE_TYPE_BLEEDING)
	{
		if (GetHP() - dam <= 0)
		{
			dam = GetHP();
		}
	}
#endif

	if (pAttacker && pAttacker->IsPC())
	{
#ifdef __DAMAGE_LIMIT_REWORK__
		long long iDmgPct = CHARACTER_MANAGER::instance().GetUserDamageRate(pAttacker);
#else
		int iDmgPct = CHARACTER_MANAGER::instance().GetUserDamageRate(pAttacker);
#endif
		dam = dam * iDmgPct / 100;
	}

	if (IsMonster() && IsStoneSkinner())
	{
		if (GetHPPct() < GetMobTable().bStoneSkinPoint)
			dam /= 2;
	}

	if (pAttacker)
	{
#ifdef __MONSTER_DEATHBLOW_REWORK__
		if (pAttacker->IsMonster() && pAttacker->IsDeathBlower())
		{
			if (pAttacker->IsDeathBlow())
			{
				int rate = 2 * MINMAX(1, 51 - pAttacker->GetHPPct(), 50);
				if (GetPoint(POINT_BLOCK) > 0 && number(1, 100) <= MINMAX(0, GetPoint(POINT_BLOCK), 50)) {
					rate -= MINMAX(1, GetPoint(POINT_BLOCK), 50);
				}

				dam *= static_cast<long long>(1.5 + 1.5 * rate / 100.0);
			}
		}
#else
		if (pAttacker->IsMonster() && pAttacker->IsDeathBlower())
		{
			if (pAttacker->IsDeathBlow())
			{
				if (number(1, 4) == GetJob())
				{
					dam = dam * 4;
				}
			}
		}
#endif

		BYTE damageFlag = 0;

		if (type == DAMAGE_TYPE_POISON)
			damageFlag = DAMAGE_POISON;
#if defined(__WOLFMAN_CHARACTER__) && !defined(USE_MOB_BLEEDING_AS_POISON)
		else if (type == DAMAGE_TYPE_BLEEDING)
			damageFlag = DAMAGE_BLEEDING;
#elif defined(__WOLFMAN_CHARACTER__) && defined(USE_MOB_BLEEDING_AS_POISON)
		else if (type == DAMAGE_TYPE_BLEEDING)
			damageFlag = DAMAGE_POISON;
#endif
		else
			damageFlag = DAMAGE_NORMAL;

		if (IsCritical == true)
			damageFlag |= DAMAGE_CRITICAL;

		if (IsPenetrate == true)
			damageFlag |= DAMAGE_PENETRATE;

		switch (GetRaceNum())
		{
		case 64136:
		case 64137:
		case 64138:
		case 471:
		case 469:
		case 988:
		case 987:
		case 6421:
			dam = 5231;
			break;

		default:
			dam = dam * GetDamMul();
			break;
		}

		if (pAttacker)
			SendDamagePacket(pAttacker, dam, damageFlag);

#ifdef ENABLE_PLAYER_STATISTICS
		if (pAttacker)
		{
			if (IsStone())
			{
				if (dam > pAttacker->GetBestStoneDamage())
				{
					//pAttacker->ChatPacket(CHAT_TYPE_INFO, "Yeni metin hasarý rekorunu kýrdýn -> Hedef: %s, Hasar: %s, Ek Bilgiler: %s%s",
					//	GetName(),
					//	NumberToDots(dam).c_str(),
					//	IsCritical ? "Kritik " : "",
					//	IsPenetrate ? "Delici " : "");
					pAttacker->SetBestStoneDamage(dam);
				}
			}
			else if (IsBoss())
			{
				if (dam > pAttacker->GetBestBossDamage())
				{
					//pAttacker->ChatPacket(CHAT_TYPE_INFO, "Yeni patron hasarý rekorunu kýrdýn -> Hedef: %s, Hasar: %s, Ek Bilgiler: %s%s",
					//	GetName(),
					//	NumberToDots(dam).c_str(),
					//	IsCritical ? "Kritik " : "",
					//	IsPenetrate ? "Delici " : "");
					pAttacker->SetBestBossDamage(dam);
				}
			}
		}
#endif

#ifdef __BATTLE_PASS_SYSTEM__
		if (dam > 0)
		{
			BYTE bBattlePassId = pAttacker->GetBattlePassId();
			if (bBattlePassId)
			{
				//if ((damageFlag == DAMAGE_NORMAL || damageFlag & DAMAGE_CRITICAL || damageFlag & DAMAGE_PENETRATE) &&
				//	type != DAMAGE_TYPE_SPECIAL && type != DAMAGE_TYPE_MELEE && type != DAMAGE_TYPE_RANGE && type != DAMAGE_TYPE_MAGIC)
				{
					if (IsPC())
					{
						DWORD dwMinLevel, dwDamage;
						if (CBattlePass::instance().BattlePassMissionGetInfo(bBattlePassId, PLAYER_DAMAGE, &dwMinLevel, &dwDamage))
						{
							DWORD dwCurrentDamage = pAttacker->GetMissionProgress(PLAYER_DAMAGE, bBattlePassId);
							if (GetLevel() >= dwMinLevel && dwCurrentDamage < dwDamage && dwCurrentDamage < dam)
								pAttacker->UpdateMissionProgress(PLAYER_DAMAGE, bBattlePassId, dam, dwDamage, true);
						}
					}
					else
					{
						DWORD dwMonsterVnum, dwDamage;
						if (CBattlePass::instance().BattlePassMissionGetInfo(bBattlePassId, MONSTER_DAMAGE, &dwMonsterVnum, &dwDamage))
						{
							DWORD dwCurrentDamage = pAttacker->GetMissionProgress(MONSTER_DAMAGE, bBattlePassId);
							if (dwMonsterVnum == GetRaceNum() && dwCurrentDamage < dwDamage && dwCurrentDamage < dam)
								pAttacker->UpdateMissionProgress(MONSTER_DAMAGE, bBattlePassId, dam, dwDamage, true);
						}
					}
				}
			}
		}
#endif

		if (test_server)
		{
			BYTE iTmpPercent = GetHPPct();

			if (pAttacker)
			{
				pAttacker->ChatPacket(CHAT_TYPE_INFO, "-> %s, DAM %lld HP %lld(%d%%) %s%s",
					GetName(),
					dam,
					GetHP(),
					iTmpPercent,
					IsCritical ? "crit " : "",
					IsPenetrate ? "pene " : "");
			}
		}
	}

	if (!cannot_dead)
	{
		if (GetHP() - dam <= 0) // @duzenleme Karakterin hp si sifirdan dusukse karakter ekraninda GetHP() olarak gozukecek ? Bence olmasi gereken 0 olarak gozukmesi fakat tam olarak calismiyor bence...
			dam = GetHP();
		PointChange(POINT_HP, -dam, false);
#ifdef ENABLE_AUTO_HUNT_SYSTEM
		if (HasAttacker() == false)
		{
			SetHasAttacker(true);
			if (GetNormalPlayer() == NULL)
				SetNormalPlayer(pAttacker);
			UpdatePacket();
		}
#endif // ENABLE_AUTO_HUNT_SYSTEM
	}

	if (pAttacker && dam > 0 && IsNPC())
	{
		TDamageMap::iterator it = m_map_kDamage.find(pAttacker->GetVID());

		if (it == m_map_kDamage.end())
		{
			m_map_kDamage.insert(TDamageMap::value_type(pAttacker->GetVID(), TBattleInfo(dam, 0)));
			it = m_map_kDamage.find(pAttacker->GetVID());
		}
		else
		{
			it->second.iTotalDamage += dam;
		}

		StartRecoveryEvent();
		UpdateAggrPointEx(pAttacker, type, dam, it->second);
	}

	if (GetHP() <= 0)
	{
		Stun();

		if (pAttacker && !pAttacker->IsNPC())
			m_dwKillerPID = pAttacker->GetPlayerID();
		else
			m_dwKillerPID = 0;
	}

	return false;
}

#ifdef __MARTY_NEWEXP_CALCULATION__
#define NEW_GET_LVDELTA(me, victim) aiPercentByDeltaLev[MINMAX(0, (victim + 15) - me, MAX_EXP_DELTA_OF_LEV - 1)]
typedef long double rate_t;
static void GiveExp(LPCHARACTER from, LPCHARACTER to, int iExp)
{
	if (test_server && iExp < 0)
	{
		to->ChatPacket(CHAT_TYPE_INFO, "exp(%d) overflow", iExp);
		return;
	}
	// decrease/increase exp based on player<>mob level
	rate_t lvFactor = static_cast<rate_t>(NEW_GET_LVDELTA(to->GetLevel(), from->GetLevel())) / 100.0L;
	iExp *= lvFactor;
	// start calculating rate exp bonus
	int iBaseExp = iExp;
	rate_t rateFactor = 100;

	rateFactor += CPrivManager::instance().GetPriv(to, PRIV_EXP_PCT);
	if (to->IsEquipUniqueItem(UNIQUE_ITEM_LARBOR_MEDAL))
		rateFactor += 20;
	if (to->GetMapIndex() >= 660000 && to->GetMapIndex() < 670000)
		rateFactor += 20;
	if (to->GetPoint(POINT_EXP_DOUBLE_BONUS))
		if (number(1, 100) <= to->GetPoint(POINT_EXP_DOUBLE_BONUS))
			rateFactor += 30;
	if (to->IsEquipUniqueItem(UNIQUE_ITEM_DOUBLE_EXP))
		rateFactor += 50;

#ifdef __EXP_RING_ITEMS__
	if (to->FindAffect(AFFECT_NEW_EXP1))
		rateFactor += 75;
	if (to->FindAffect(AFFECT_NEW_EXP2))
		rateFactor += 100;
	if (to->FindAffect(AFFECT_NEW_EXP3))
		rateFactor += 125;
	if (to->FindAffect(AFFECT_NEW_EXP4))
		rateFactor += 150;
	if (to->FindAffect(AFFECT_NEW_EXP5))
		rateFactor += 175;
	if (to->FindAffect(AFFECT_NEW_EXP6))
		rateFactor += 200;
#endif

	switch (to->GetMountVnum())
	{
	case 20110:
	case 20111:
	case 20112:
	case 20113:
		if (to->IsEquipUniqueItem(71115) || to->IsEquipUniqueItem(71117) || to->IsEquipUniqueItem(71119) ||
			to->IsEquipUniqueItem(71121))
		{
			rateFactor += 10;
		}
		break;

	case 20114:
	case 20120:
	case 20121:
	case 20122:
	case 20123:
	case 20124:
	case 20125:
		rateFactor += 30;
		break;
	}

	if (to->GetPremiumRemainSeconds(PREMIUM_EXP) > 0)
		rateFactor += 50;
	if (to->IsEquipUniqueGroup(UNIQUE_GROUP_RING_OF_EXP))
		rateFactor += 50;

	rateFactor += to->GetPoint(POINT_RAMADAN_CANDY_BONUS_EXP);
	rateFactor += to->GetPoint(POINT_MALL_EXPBONUS);
	rateFactor += to->GetPoint(POINT_EXP);
	// useless (never used except for china intoxication) = always 100
	rateFactor = rateFactor * static_cast<rate_t>(CHARACTER_MANAGER::instance().GetMobExpRate(to)) / 100.0L;
	// fix underflow formula
	iExp = std::max<int>(0, iExp);
	rateFactor = std::max<rate_t>(100.0L, rateFactor);
	// apply calculated rate bonus
	iExp *= (rateFactor / 100.0L);
	if (test_server)
		to->ChatPacket(CHAT_TYPE_INFO, "base_exp(%d) * rate(%Lf) = exp(%d)", iBaseExp, rateFactor / 100.0L, iExp);
	// you can get at maximum only 10% of the total required exp at once (so, you need to kill at least 10 mobs to level up) (useless)
	iExp = MIN(to->GetNextExp() / 10, iExp);
	// it recalculate the given exp if the player level is greater than the exp_table size (useless)
	iExp = AdjustExpByLevel(to, iExp);
	if (test_server)
		to->ChatPacket(CHAT_TYPE_INFO, "exp+minGNE+adjust(%d)", iExp);

/*#ifdef __GROWTH_PET_SYSTEM__
	if (to->GetNewPetSystem())
	{
		if (to->GetNewPetSystem()->IsActivePet() && to->GetNewPetSystem()->GetLevelStep() < 4)
		{
			int tmpexp = iExp * 9 / 20;
			if (to->GetQuestFlag("PetSystems.petfullexp") == 1)
				tmpexp = 0;
			iExp = iExp - tmpexp;
			to->GetNewPetSystem()->SetExp(tmpexp, 0);
		}
	}
#endif

#ifdef __GROWTH_MOUNT_SYSTEM__
	if (to->GetNewMountSystem())
	{
		if (to->GetNewMountSystem()->IsActiveMount() && to->GetNewMountSystem()->GetLevelStep() < 4)
		{
			int tmpexp = iExp * 9 / 20;
			if (to->GetQuestFlag("MountSystems.mountfullexp") == 1)
				tmpexp = 0;
			iExp = iExp - tmpexp;
			to->GetNewMountSystem()->SetExp(tmpexp, 0);
		}
	}
#endif*/

#ifdef EXP_MULTIPLER
	iExp *= EXP_MULTIPLER;
#endif

	iExp *= 2;


	switch (from->GetRaceNum())
	{
		case 8817:
		case 8800:
		case 8815:
		case 8801:
		case 8819:
		case 8802:
		case 8816:
		case 8804:
			if (to->GetLevel() >= 75)
				iExp = 0;
			break;
		case 8818:
		case 8805:
		case 2066:
		case 2065:
			if (to->GetLevel() >= 90)
				iExp = 0;
			break;
		case 8442:
		case 8806:
			if (to->GetLevel() >= 105)
				iExp = 0;
			break;
		case 8061:
		case 8807:
		case 2414:
		case 2415:
			if (to->GetLevel() >= 120)
				iExp = 0;
			break;

		case 8483:
		case 8809:
			if (to->GetLevel() >= 130)
				iExp = 0;
			break;

		case 8835:
		case 8810:
			if (to->GetLevel() >= 140)
				iExp = 0;
			break;

		case 8808:
			if (to->GetLevel() >= 150)
				iExp = 0;
			break;

		case 8836:
		case 8811:
			if (to->GetLevel() >= 160)
				iExp = 0;
			break;

		case 8837:
		case 8812:
			if (to->GetLevel() >= 170)
				iExp = 0;
			break;

		case 8407:
			if (to->GetLevel() >= 180)
				iExp = 0;
			break;

		case 8456:
			if (to->GetLevel() >= 190)
				iExp = 0;
			break;

		case 8414:
			if (to->GetLevel() >= 200)
				iExp = 0;
			break;

	}

#ifdef ENABLE_MULTI_FARM_BLOCK
	if (!to->GetMultiStatus())
		return;
#endif

	if (!to->IsBlockMode(BLOCK_POINT_EXP))
	{
		to->PointChange(POINT_EXP, iExp, true);
		from->CreateFly(FLY_EXP, to);
	}
}
#else
static void GiveExp(LPCHARACTER from, LPCHARACTER to, int iExp)
{
	iExp = CALCULATE_VALUE_LVDELTA(to->GetLevel(), from->GetLevel(), iExp);

	int iBaseExp = iExp;

	iExp = iExp * (100 + CPrivManager::instance().GetPriv(to, PRIV_EXP_PCT)) / 100;

	{
		if (to->IsEquipUniqueItem(UNIQUE_ITEM_LARBOR_MEDAL))
			iExp += iExp * 20 / 100;

		if (to->GetMapIndex() >= 660000 && to->GetMapIndex() < 670000)
			iExp += iExp * 20 / 100;

		if (to->GetPoint(POINT_EXP_DOUBLE_BONUS))
			if (number(1, 100) <= to->GetPoint(POINT_EXP_DOUBLE_BONUS))
				iExp += iExp * 30 / 100;

		if (to->IsEquipUniqueItem(UNIQUE_ITEM_DOUBLE_EXP))
			iExp += iExp * 50 / 100;

		switch (to->GetMountVnum())
		{
		case 20110:
		case 20111:
		case 20112:
		case 20113:
			if (to->IsEquipUniqueItem(71115) || to->IsEquipUniqueItem(71117) || to->IsEquipUniqueItem(71119) ||
				to->IsEquipUniqueItem(71121))
			{
				iExp += iExp * 10 / 100;
			}
			break;

		case 20114:
		case 20120:
		case 20121:
		case 20122:
		case 20123:
		case 20124:
		case 20125:

			iExp += iExp * 30 / 100;
			break;
		}
	}

	{
		if (to->GetPremiumRemainSeconds(PREMIUM_EXP) > 0)
			iExp += (iExp * 50 / 100);

		if (to->IsEquipUniqueGroup(UNIQUE_GROUP_RING_OF_EXP) == true)
			iExp += (iExp * 50 / 100);
	}

	iExp += (iExp * to->GetPoint(POINT_RAMADAN_CANDY_BONUS_EXP) / 100);
	iExp += (iExp * to->GetPoint(POINT_MALL_EXPBONUS) / 100);
	iExp += (iExp * to->GetPoint(POINT_EXP) / 100);

	if (test_server)
	{
		sys_log(0, "Bonus Exp : Ramadan Candy: %d MallExp: %d PointExp: %d",
			to->GetPoint(POINT_RAMADAN_CANDY_BONUS_EXP),
			to->GetPoint(POINT_MALL_EXPBONUS),
			to->GetPoint(POINT_EXP)
		);
	}

	iExp = iExp * CHARACTER_MANAGER::instance().GetMobExpRate(to) / 100;
	iExp = MIN(to->GetNextExp() / 10, iExp);

	if (test_server)
	{
		if (quest::CQuestManager::instance().GetEventFlag("exp_bonus_log") && iBaseExp > 0)
			to->ChatPacket(CHAT_TYPE_INFO, "exp bonus %d%%", (iExp - iBaseExp) * 100 / iBaseExp);
		to->ChatPacket(CHAT_TYPE_INFO, "exp(%d) base_exp(%d)", iExp, iBaseExp);
	}

	iExp = AdjustExpByLevel(to, iExp);

	to->PointChange(POINT_EXP, iExp, true);
	from->CreateFly(FLY_EXP, to);
}
#endif

namespace NPartyExpDistribute
{
	struct FPartyTotaler
	{
		int		total;
		int		member_count;
		int		x, y;

		FPartyTotaler(LPCHARACTER center)
			: total(0), member_count(0), x(center->GetX()), y(center->GetY())
		{};

		void operator () (LPCHARACTER ch)
		{
			if (DISTANCE_APPROX(ch->GetX() - x, ch->GetY() - y) <= PARTY_DEFAULT_RANGE)
			{
				total += __GetPartyExpNP(ch->GetLevel());

				++member_count;
			}
		}
	};

	struct FPartyDistributor
	{
		int		total;
		LPCHARACTER	c;
		int		x, y;
		DWORD		_iExp;
		int		m_iMode;
		int		m_iMemberCount;

		FPartyDistributor(LPCHARACTER center, int member_count, int total, DWORD iExp, int iMode)
			: total(total), c(center), x(center->GetX()), y(center->GetY()), _iExp(iExp), m_iMode(iMode), m_iMemberCount(member_count)
		{
			if (m_iMemberCount == 0)
				m_iMemberCount = 1;
		};

		void operator () (LPCHARACTER ch)
		{
			if (DISTANCE_APPROX(ch->GetX() - x, ch->GetY() - y) <= PARTY_DEFAULT_RANGE)
			{
				DWORD iExp2 = 0;

				switch (m_iMode)
				{
				case PARTY_EXP_DISTRIBUTION_NON_PARITY:
					iExp2 = (DWORD)(_iExp * (float)__GetPartyExpNP(ch->GetLevel()) / total);
					break;

				case PARTY_EXP_DISTRIBUTION_PARITY:
					iExp2 = _iExp / m_iMemberCount;
					break;

				default:
					sys_err("Unknown party exp distribution mode %d", m_iMode);
					return;
				}

				GiveExp(c, ch, iExp2);
			}
		}
	};
}

typedef struct SDamageInfo
{
#ifdef __DAMAGE_LIMIT_REWORK__
	long long iDam;
#else
	int iDam;
#endif
	LPCHARACTER pAttacker;
	LPPARTY pParty;

	void Clear()
	{
		pAttacker = NULL;
		pParty = NULL;
	}

	inline void Distribute(LPCHARACTER ch, int iExp)
	{
		if (pAttacker)
			GiveExp(ch, pAttacker, iExp);
		else if (pParty)
		{
			NPartyExpDistribute::FPartyTotaler f(ch);
			pParty->ForEachOnlineMember(f);

			if (pParty->IsPositionNearLeader(ch))
				iExp = iExp * (100 + pParty->GetExpBonusPercent()) / 100;

			if (test_server)
			{
				if (quest::CQuestManager::instance().GetEventFlag("exp_bonus_log") && pParty->GetExpBonusPercent())
					pParty->ChatPacketToAllMember(CHAT_TYPE_INFO, "exp party bonus %d%%", pParty->GetExpBonusPercent());
			}

			if (pParty->GetExpCentralizeCharacter())
			{
				LPCHARACTER tch = pParty->GetExpCentralizeCharacter();

				if (DISTANCE_APPROX(ch->GetX() - tch->GetX(), ch->GetY() - tch->GetY()) <= PARTY_DEFAULT_RANGE)
				{
					int iExpCenteralize = (int)(iExp * 0.05f);
					iExp -= iExpCenteralize;

					GiveExp(ch, pParty->GetExpCentralizeCharacter(), iExpCenteralize);
				}
			}

			NPartyExpDistribute::FPartyDistributor fDist(ch, f.member_count, f.total, iExp, pParty->GetExpDistributionMode());
			pParty->ForEachOnlineMember(fDist);
		}
	}
} TDamageInfo;

LPCHARACTER CHARACTER::DistributeExp()
{
	int iExpToDistribute = GetExp();

	if (iExpToDistribute <= 0)
		return NULL;
#ifdef __DAMAGE_LIMIT_REWORK__
	long long	iTotalDam = 0;
	LPCHARACTER pkChrMostAttacked = NULL;
	long long iMostDam = 0;
#else
	int	iTotalDam = 0;
	LPCHARACTER pkChrMostAttacked = NULL;
	int iMostDam = 0;
#endif
	typedef std::vector<TDamageInfo> TDamageInfoTable;
	TDamageInfoTable damage_info_table;
	std::map<LPPARTY, TDamageInfo> map_party_damage;

	damage_info_table.reserve(m_map_kDamage.size());

	TDamageMap::iterator it = m_map_kDamage.begin();

	while (it != m_map_kDamage.end())
	{
		const VID& c_VID = it->first;
#ifdef __DAMAGE_LIMIT_REWORK__
		long long iDam = it->second.iTotalDamage;
#else
		int iDam = it->second.iTotalDamage;
#endif
		++it;

		LPCHARACTER pAttacker = CHARACTER_MANAGER::instance().Find(c_VID);

		if (!pAttacker || !pAttacker->IsPC() || pAttacker->IsNPC())
			continue;

		// Block exp award based on distance
		// Groups have DOUBLE the exp distance to prevent the "exploit" where the party leader gets full exp bonus and some party members don't get exp.
		int dist = DISTANCE_APPROX(GetX() - pAttacker->GetX(), GetY() - pAttacker->GetY());
		if (dist > 10000 || (dist > 5000 && !pAttacker->GetParty()))
			continue;

		iTotalDam += iDam;
		if (!pkChrMostAttacked || iDam > iMostDam)
		{
			pkChrMostAttacked = pAttacker;
			iMostDam = iDam;
		}

		if (pAttacker->GetParty())
		{
			std::map<LPPARTY, TDamageInfo>::iterator it = map_party_damage.find(pAttacker->GetParty());
			if (it == map_party_damage.end())
			{
				TDamageInfo di;
				di.iDam = iDam;
				di.pAttacker = NULL;
				di.pParty = pAttacker->GetParty();
				map_party_damage.insert(std::make_pair(di.pParty, di));
			}
			else
				it->second.iDam += iDam;
		}
		else
		{
			TDamageInfo di;

			di.iDam = iDam;
			di.pAttacker = pAttacker;
			di.pParty = NULL;
			damage_info_table.push_back(di);
		}
	}

	for (std::map<LPPARTY, TDamageInfo>::iterator it = map_party_damage.begin(); it != map_party_damage.end(); ++it)
		damage_info_table.push_back(it->second);

	SetExp(0);

	if (iTotalDam == 0)
		return NULL;

	if (m_pkChrStone)
	{
		int iExp = iExpToDistribute >> 1;
		m_pkChrStone->SetExp(m_pkChrStone->GetExp() + iExp);
		iExpToDistribute -= iExp;
	}

	sys_log(1, "%s total exp: %d, damage_info_table.size() == %d, TotalDam %d", GetName(), iExpToDistribute, damage_info_table.size(), iTotalDam);

	if (damage_info_table.empty())
		return NULL;

	{
		TDamageInfoTable::iterator di = damage_info_table.begin();
		{
			TDamageInfoTable::iterator it;

			for (it = damage_info_table.begin(); it != damage_info_table.end(); ++it)
			{
				if (it->iDam > di->iDam)
					di = it;
			}
		}

		int	iExp = iExpToDistribute / 5;
		iExpToDistribute -= iExp;

		float fPercent = (float)di->iDam / iTotalDam;

		if (fPercent > 1.0f)
		{
			sys_err("DistributeExp percent over 1.0 (fPercent %f name %s)", fPercent, di->pAttacker->GetName());
			fPercent = 1.0f;
		}

		iExp += (int)(iExpToDistribute * fPercent);

		di->Distribute(this, iExp);

		if (fPercent == 1.0f)
			return pkChrMostAttacked;

		di->Clear();
	}

	{
		TDamageInfoTable::iterator it;

		for (it = damage_info_table.begin(); it != damage_info_table.end(); ++it)
		{
			TDamageInfo& di = *it;

			float fPercent = (float)di.iDam / iTotalDam;

			if (fPercent > 1.0f)
			{
				sys_err("DistributeExp percent over 1.0 (fPercent %f name %s)", fPercent, di.pAttacker->GetName());
				fPercent = 1.0f;
			}

			di.Distribute(this, (int)(iExpToDistribute * fPercent));
		}
	}

	return pkChrMostAttacked;
}

int CHARACTER::GetArrowAndBow(LPITEM* ppkBow, LPITEM* ppkArrow, int iArrowCount/* = 1 */)
{
	LPITEM pkBow;

	if (!(pkBow = GetWear(WEAR_WEAPON)) || pkBow->GetProto()->bSubType != WEAPON_BOW)
		return 0;

	LPITEM pkArrow;

#ifdef __QUIVER_SYSTEM__
	if (!(pkArrow = GetWear(WEAR_ARROW)) || pkArrow->GetType() != ITEM_WEAPON || (pkArrow->GetSubType() != WEAPON_ARROW && pkArrow->GetSubType() != WEAPON_QUIVER))
#else
	if (!(pkArrow = GetWear(WEAR_ARROW)) || pkArrow->GetType() != ITEM_WEAPON || pkArrow->GetSubType() != WEAPON_ARROW)
#endif
		return 0;

#ifdef __QUIVER_SYSTEM__
	if (pkArrow->GetSubType() == WEAPON_QUIVER)
		iArrowCount = MIN(iArrowCount, pkArrow->GetSocket(0) - time(0));
	else
		iArrowCount = MIN(iArrowCount, pkArrow->GetCount());
#else
	iArrowCount = MIN(iArrowCount, pkArrow->GetCount());
#endif

	* ppkBow = pkBow;
	*ppkArrow = pkArrow;

	return iArrowCount;
}

void CHARACTER::UseArrow(LPITEM pkArrow, DWORD dwArrowCount)
{
#ifdef __QUIVER_SYSTEM__
	if (pkArrow->GetSubType() == WEAPON_QUIVER)
		return;
#endif

	int iCount = pkArrow->GetCount();
	DWORD dwVnum = pkArrow->GetVnum();
	iCount = iCount - MIN(iCount, dwArrowCount);
	pkArrow->SetCount(iCount);

	if (iCount == 0)
	{
		LPITEM pkNewArrow = FindSpecifyItem(dwVnum);
		sys_log(0, "UseArrow : FindSpecifyItem %u %p", dwVnum, get_pointer(pkNewArrow));

		if (pkNewArrow)
			EquipItem(pkNewArrow);
	}
}

class CFuncShoot
{
public:
	LPCHARACTER	m_me;
	BYTE		m_bType;
	bool		m_bSucceed;

	CFuncShoot(LPCHARACTER ch, BYTE bType) : m_me(ch), m_bType(bType), m_bSucceed(FALSE)
	{
	}

	void operator () (DWORD dwTargetVID)
	{
		if (m_bType > 1)
		{
			if (g_bSkillDisable)
				return;

			m_me->m_SkillUseInfo[m_bType].SetMainTargetVID(dwTargetVID);
		}

		LPCHARACTER pkVictim = CHARACTER_MANAGER::instance().Find(dwTargetVID);

		if (!pkVictim)
			return;

		if (!battle_is_attackable(m_me, pkVictim))
			return;

		if (m_me->IsNPC())
		{
			if (DISTANCE_APPROX(m_me->GetX() - pkVictim->GetX(), m_me->GetY() - pkVictim->GetY()) > 5000)
				return;
		}

		if (m_me->IsPC() && m_bType > 0 && m_me->SkillIsOnCooldown(m_bType))
			return;

		LPITEM pkBow, pkArrow;

		switch (m_bType)
		{
		case 0:
		{
#ifdef __DAMAGE_LIMIT_REWORK__
			long long iDam = 0;
#else
			int iDam = 0;
#endif
			if (m_me->IsPC())
			{
				if (m_me->GetJob() != JOB_ASSASSIN)
					return;

				if (0 == m_me->GetArrowAndBow(&pkBow, &pkArrow))
					return;

				if (m_me->GetSkillGroup() != 0)
					if (!m_me->IsNPC() && m_me->GetSkillGroup() != 2)
					{
						if (m_me->GetSP() < 5)
							return;

						m_me->PointChange(POINT_SP, -5);
					}

				iDam = CalcArrowDamage(m_me, pkVictim, pkBow, pkArrow);
				m_me->UseArrow(pkArrow, 1);
			}
			else
				iDam = CalcMeleeDamage(m_me, pkVictim);

			NormalAttackAffect(m_me, pkVictim);

#ifdef __PENDANT_SYSTEM__
			iDam = iDam * (100 - (pkVictim->GetPoint(POINT_RESIST_BOW) - pkVictim->GetPoint(POINT_ATTBONUS_BOW))) / 100;
#else
			iDam = iDam * (100 - pkVictim->GetPoint(POINT_RESIST_BOW)) / 100;
#endif

			m_me->OnMove(true);
			pkVictim->OnMove();

			if (pkVictim->CanBeginFight())
				pkVictim->BeginFight(m_me);

			pkVictim->Damage(m_me, iDam, DAMAGE_TYPE_NORMAL_RANGE);
		}
		break;

		case 1:
		{
#ifdef __DAMAGE_LIMIT_REWORK__
			long long iDam;
#else
			int iDam;
#endif
			if (m_me->IsPC())
				return;

			iDam = CalcMagicDamage(m_me, pkVictim);

			NormalAttackAffect(m_me, pkVictim);

//#ifdef __MAGIC_REDUCTION_SYSTEM__
//			const int resist_magic = MINMAX(0, pkVictim->GetPoint(POINT_RESIST_MAGIC), 100);
//			const int resist_magic_reduction = MINMAX(0, (m_me->GetJob() == JOB_SURA) ? m_me->GetPoint(POINT_RESIST_MAGIC_REDUCTION) / 2 : m_me->GetPoint(POINT_RESIST_MAGIC_REDUCTION), 50);
//			const int total_res_magic = MINMAX(0, resist_magic - resist_magic_reduction, 100);
//			iDam = iDam * (100 - total_res_magic) / 100;
//#else
			//iDam = iDam * (100 - pkVictim->GetPoint(POINT_RESIST_MAGIC)) / 100;
//#endif

			m_me->OnMove(true);
			pkVictim->OnMove();

			if (pkVictim->CanBeginFight())
				pkVictim->BeginFight(m_me);

			pkVictim->Damage(m_me, iDam, DAMAGE_TYPE_MAGIC);
		}
		break;

		case SKILL_YEONSA:
		{
			int iUseArrow = 1;

			{
				if (iUseArrow == m_me->GetArrowAndBow(&pkBow, &pkArrow, iUseArrow))
				{
					m_me->OnMove(true);
					pkVictim->OnMove();

					if (pkVictim->CanBeginFight())
						pkVictim->BeginFight(m_me);

					m_me->ComputeSkill(m_bType, pkVictim);
					m_me->UseArrow(pkArrow, iUseArrow);

					if (pkVictim->IsDead())
						break;
				}
				else
					break;
			}
		}
		break;

		case SKILL_KWANKYEOK:
		{
			int iUseArrow = 1;

			if (iUseArrow == m_me->GetArrowAndBow(&pkBow, &pkArrow, iUseArrow))
			{
				m_me->OnMove(true);
				pkVictim->OnMove();

				if (pkVictim->CanBeginFight())
					pkVictim->BeginFight(m_me);

				sys_log(0, "%s kwankeyok %s", m_me->GetName(), pkVictim->GetName());
				m_me->ComputeSkill(m_bType, pkVictim);
				m_me->UseArrow(pkArrow, iUseArrow);
			}
		}
		break;

		case SKILL_GIGUNG:
		{
			int iUseArrow = 1;
			if (iUseArrow == m_me->GetArrowAndBow(&pkBow, &pkArrow, iUseArrow))
			{
				m_me->OnMove(true);
				pkVictim->OnMove();

				if (pkVictim->CanBeginFight())
					pkVictim->BeginFight(m_me);

				sys_log(0, "%s gigung %s", m_me->GetName(), pkVictim->GetName());
				m_me->ComputeSkill(m_bType, pkVictim);
				m_me->UseArrow(pkArrow, iUseArrow);
			}
		}

		break;
		case SKILL_HWAJO:
		{
			int iUseArrow = 1;
			if (iUseArrow == m_me->GetArrowAndBow(&pkBow, &pkArrow, iUseArrow))
			{
				m_me->OnMove(true);
				pkVictim->OnMove();

				if (pkVictim->CanBeginFight())
					pkVictim->BeginFight(m_me);

				sys_log(0, "%s hwajo %s", m_me->GetName(), pkVictim->GetName());
				m_me->ComputeSkill(m_bType, pkVictim);
				m_me->UseArrow(pkArrow, iUseArrow);
			}
		}

		break;

		case SKILL_HORSE_WILDATTACK_RANGE:
		{
			int iUseArrow = 1;
			if (iUseArrow == m_me->GetArrowAndBow(&pkBow, &pkArrow, iUseArrow))
			{
				m_me->OnMove(true);
				pkVictim->OnMove();

				if (pkVictim->CanBeginFight())
					pkVictim->BeginFight(m_me);

				sys_log(0, "%s horse_wildattack %s", m_me->GetName(), pkVictim->GetName());
				m_me->ComputeSkill(m_bType, pkVictim);
				m_me->UseArrow(pkArrow, iUseArrow);
			}
		}

		break;

		case SKILL_MARYUNG:
		case SKILL_TUSOK:
		case SKILL_BIPABU:
		case SKILL_NOEJEON:
		case SKILL_GEOMPUNG:
		case SKILL_SANGONG:
		case SKILL_MAHWAN:
		case SKILL_PABEOB:
		{
			m_me->OnMove(true);
			pkVictim->OnMove();

			if (pkVictim->CanBeginFight())
				pkVictim->BeginFight(m_me);

			sys_log(0, "%s - Skill %d -> %s", m_me->GetName(), m_bType, pkVictim->GetName());
			m_me->ComputeSkill(m_bType, pkVictim);
		}
		break;

		case SKILL_CHAIN:
		{
			m_me->OnMove(true);
			pkVictim->OnMove();

			if (pkVictim->CanBeginFight())
				pkVictim->BeginFight(m_me);

			sys_log(0, "%s - Skill %d -> %s", m_me->GetName(), m_bType, pkVictim->GetName());
			m_me->ComputeSkill(m_bType, pkVictim);
		}
		break;

		case SKILL_YONGBI:
			{
				m_me->OnMove(true);
			}
			break;

		default:
			sys_err("CFuncShoot: I don't know this type [%d] of range attack.", (int)m_bType);
			break;
		}

		m_bSucceed = TRUE;
	}
};

bool CHARACTER::Shoot(BYTE bType)
{
	sys_log(1, "Shoot %s type %u flyTargets.size %zu", GetName(), bType, m_vec_dwFlyTargets.size());

	if (!CanMove())
		return false;

	CFuncShoot f(this, bType);

	if (m_dwFlyTargetID != 0)
	{
		f(m_dwFlyTargetID);
		m_dwFlyTargetID = 0;
	}

	f = std::for_each(m_vec_dwFlyTargets.begin(), m_vec_dwFlyTargets.end(), f);
	m_vec_dwFlyTargets.clear();

	return f.m_bSucceed;
}

void CHARACTER::FlyTarget(DWORD dwTargetVID, long x, long y, BYTE bHeader)
{
	LPCHARACTER pkVictim = CHARACTER_MANAGER::instance().Find(dwTargetVID);
	TPacketGCFlyTargeting pack;

	pack.bHeader = (bHeader == HEADER_CG_FLY_TARGETING) ? HEADER_GC_FLY_TARGETING : HEADER_GC_ADD_FLY_TARGETING;
	pack.dwShooterVID = GetVID();

	if (pkVictim)
	{
		pack.dwTargetVID = pkVictim->GetVID();
		pack.x = pkVictim->GetX();
		pack.y = pkVictim->GetY();

		if (bHeader == HEADER_CG_FLY_TARGETING)
			m_dwFlyTargetID = dwTargetVID;
		else
			m_vec_dwFlyTargets.push_back(dwTargetVID);
	}
	else
	{
		pack.dwTargetVID = 0;
		pack.x = x;
		pack.y = y;
	}

	sys_log(1, "FlyTarget %s vid %d x %d y %d", GetName(), pack.dwTargetVID, pack.x, pack.y);
	PacketAround(&pack, sizeof(pack), this);
}

LPCHARACTER CHARACTER::GetNearestVictim(LPCHARACTER pkChr)
{
	if (NULL == pkChr)
		pkChr = this;

	float fMinDist = 99999.0f;
	LPCHARACTER pkVictim = NULL;

	TDamageMap::iterator it = m_map_kDamage.begin();

	while (it != m_map_kDamage.end())
	{
		const VID& c_VID = it->first;
		++it;

		LPCHARACTER pAttacker = CHARACTER_MANAGER::instance().Find(c_VID);

		if (!pAttacker)
			continue;

		if (pAttacker->IsAffectFlag(AFF_EUNHYUNG) ||
			pAttacker->IsAffectFlag(AFF_INVISIBILITY) ||
			pAttacker->IsAffectFlag(AFF_REVIVE_INVISIBLE))
			continue;

		float fDist = DISTANCE_APPROX(pAttacker->GetX() - pkChr->GetX(), pAttacker->GetY() - pkChr->GetY());

		if (fDist < fMinDist)
		{
			pkVictim = pAttacker;
			fMinDist = fDist;
		}
	}

	return pkVictim;
}

void CHARACTER::SetVictim(LPCHARACTER pkVictim)
{
	if (!pkVictim)
	{
		m_kVIDVictim.Reset();
		battle_end(this);
	}
	else
	{
		m_kVIDVictim = pkVictim->GetVID();
		m_dwLastVictimSetTime = get_dword_time();
	}
}

LPCHARACTER CHARACTER::GetVictim() const
{
	return CHARACTER_MANAGER::instance().Find(m_kVIDVictim);
}

LPCHARACTER CHARACTER::GetProtege() const
{
	if (m_pkChrStone)
		return m_pkChrStone;

	if (m_pkParty)
		return m_pkParty->GetLeader();

	return NULL;
}

int CHARACTER::GetAlignment() const
{
	return m_iAlignment;
}

int CHARACTER::GetRealAlignment() const
{
	return m_iRealAlignment;
}

void CHARACTER::ShowAlignment(bool bShow)
{
	if (bShow)
	{
		if (m_iAlignment != m_iRealAlignment)
		{
			m_iAlignment = m_iRealAlignment;
			UpdatePacket();
		}
	}
	else
	{
		if (m_iAlignment != 0)
		{
			m_iAlignment = 0;
			UpdatePacket();
		}
	}
}

#ifdef __ALIGNMENT_REWORK__
void CHARACTER::UpdateAlignment(int iAmount, bool hard)
#else
void CHARACTER::UpdateAlignment(int iAmount)
#endif
{
#ifdef __ALIGNMENT_REWORK__
	if (hard) { /*Better testing, hard used only gm alignment command*/
		m_iRealAlignment += iAmount;
		m_iAlignment = m_iRealAlignment;
		RefreshAlignBonus();
		UpdatePacket();
		return;
	}
#endif
	bool bShow = false;

	if (m_iAlignment == m_iRealAlignment)
		bShow = true;

	int i = m_iAlignment / 10;

#ifdef __ALIGNMENT_REWORK__
	if (m_iRealAlignment >= 0 && iAmount > 0
		&& GetLevel() < ENABLE_RENEWAL_ALIGN_UPDATE_2)
		return;

	/**
	 * Get previous align grade and compare with new.
	 * If different: Refresh align bonus
	 * else: do nothing
	 *
	 * So align bonus will not refresh for minor changes.
	*/
	int previousAlignGrade = GetAlignGrade();

	/*
	if (previousAlignGrade == 0 // 0 is max!
		&& iAmount < 0)
		return;
	*/

	m_iRealAlignment = MINMAX(-200000, m_iRealAlignment + iAmount, 200000000);

	int newAlignGrade = GetAlignGrade();
	if (previousAlignGrade != newAlignGrade)
	{
		sys_log(0, "Alignment grade changing name: %s previous_grade: %d new_grade: %d", GetName(), previousAlignGrade, newAlignGrade);
		RefreshAlignBonus();
	}
#else
	m_iRealAlignment = MINMAX(-200000, m_iRealAlignment + iAmount, 200000);
#endif
	if (bShow)
	{
		m_iAlignment = m_iRealAlignment;

		if (i != m_iAlignment / 10)
			UpdatePacket();
	}
}

#ifdef __RANK_SYSTEM__
int CHARACTER::GetRank() const
{
	return m_iRank;
}

void CHARACTER::ChangeRank(int iAmount)
{
	if (iAmount == m_iRank)
		return;

	sys_log(0, "Rank changing name: %s previous: %d iAmount: %d", GetName(), m_iRank, iAmount);
	m_iRank = MINMAX(0, iAmount, RANK_GRADE_COUNT - 1); // -1 for include 0
	RefreshRankBonus();
	UpdatePacket();
}
#endif

#ifdef __TEAM_SYSTEM__
int CHARACTER::GetTeam() const
{
	return m_iTeam;
}

void CHARACTER::ChangeTeam(int iAmount)
{
	if (iAmount == m_iTeam)
		return;

	sys_log(0, "Team changing name: %s previous: %d iAmount: %d", GetName(), m_iTeam, iAmount);

	m_iTeam = iAmount;

	RefreshTeamBonus();
	UpdatePacket();
}
#endif

#ifdef __LANDRANK_SYSTEM__
int CHARACTER::GetLandRank() const
{
	return m_iLandRank;
}

void CHARACTER::ChangeLandRank(int iAmount)
{
	if (iAmount == m_iLandRank)
		return;

	sys_log(0, "LandRank changing name: %s previous: %d iAmount: %d", GetName(), m_iLandRank, iAmount);
	m_iLandRank = MINMAX(0, iAmount, 300);
	RefreshLandRankBonus();
	UpdatePacket();
}
#endif

#ifdef __REBORN_SYSTEM__
int CHARACTER::GetReborn() const
{
	return m_iReborn;
}

void CHARACTER::ChangeReborn(int iAmount)
{
	if (iAmount == m_iReborn)
		return;

	sys_log(0, "Reborn changing name: %s previous: %d iAmount: %d", GetName(), m_iReborn, iAmount);
	m_iReborn = MINMAX(0, iAmount, REBORN_GRADE_COUNT); // -1 for include 0
	RefreshRebornBonus();
	UpdatePacket();
}
#endif

#ifdef __MONIKER_SYSTEM__
const char* CHARACTER::GetMoniker() const
{
	return m_moniker;
}

void CHARACTER::ChangeMoniker(const char* moniker)
{
	strcpy(m_moniker, moniker);
	sys_log(0, "Moniker updating name: %s %s", GetName(), m_moniker);

	RefreshMonikerBonus();
	UpdatePacket();
}
#endif

void CHARACTER::SetKillerMode(bool isOn)
{
	if ((isOn ? ADD_CHARACTER_STATE_KILLER : 0) == IS_SET(m_bAddChrState, ADD_CHARACTER_STATE_KILLER))
		return;

	if (isOn)
		SET_BIT(m_bAddChrState, ADD_CHARACTER_STATE_KILLER);
	else
		REMOVE_BIT(m_bAddChrState, ADD_CHARACTER_STATE_KILLER);

	m_iKillerModePulse = thecore_pulse();
	UpdatePacket();
	sys_log(0, "SetKillerMode Update %s[%d]", GetName(), GetPlayerID());
}

bool CHARACTER::IsKillerMode() const
{
	return IS_SET(m_bAddChrState, ADD_CHARACTER_STATE_KILLER);
}

void CHARACTER::UpdateKillerMode()
{
	if (!IsKillerMode())
		return;

	if (thecore_pulse() - m_iKillerModePulse >= PASSES_PER_SEC(30))
		SetKillerMode(false);
}

void CHARACTER::SetPKMode(BYTE bPKMode)
{
	if (bPKMode >= PK_MODE_MAX_NUM)
		return;

	if (m_bPKMode == bPKMode)
		return;

	if (bPKMode == PK_MODE_GUILD && !GetGuild())
		bPKMode = PK_MODE_FREE;

	m_bPKMode = bPKMode;
	UpdatePacket();

	sys_log(0, "PK_MODE: %s %d", GetName(), m_bPKMode);
}

BYTE CHARACTER::GetPKMode() const
{
	return m_bPKMode;
}

struct FuncForgetMyAttacker
{
	LPCHARACTER m_ch;
	FuncForgetMyAttacker(LPCHARACTER ch)
	{
		m_ch = ch;
	}
	void operator()(LPENTITY ent)
	{
		if (ent->IsType(ENTITY_CHARACTER))
		{
			LPCHARACTER ch = (LPCHARACTER)ent;
			if (ch->IsPC())
				return;
			if (ch->m_kVIDVictim == m_ch->GetVID())
				ch->SetVictim(NULL);
		}
	}
};

struct FuncAggregateMonster
{
	LPCHARACTER m_ch;
	FuncAggregateMonster(LPCHARACTER ch)
	{
		m_ch = ch;
	}
	void operator()(LPENTITY ent)
	{
		if (ent->IsType(ENTITY_CHARACTER))
		{
			LPCHARACTER ch = (LPCHARACTER)ent;

			if (ch->IsPC())
				return;

			if (!ch->IsMonster())
				return;

			if (ch->GetVictim())
				return;

			if (DISTANCE_APPROX(ch->GetX() - m_ch->GetX(), ch->GetY() - m_ch->GetY()) < 5000)// accelerated.
				if (ch->CanBeginFight())
					ch->BeginFight(m_ch);
		}
	}
};

struct FuncAttractRanger
{
	LPCHARACTER m_ch;
	FuncAttractRanger(LPCHARACTER ch)
	{
		m_ch = ch;
	}

	void operator()(LPENTITY ent)
	{
		if (ent->IsType(ENTITY_CHARACTER))
		{
			LPCHARACTER ch = (LPCHARACTER)ent;
			if (ch->IsPC())
				return;

			if (!ch->IsMonster())
				return;

			if (ch->GetVictim() && ch->GetVictim() != m_ch)
				return;

			if (ch->GetMobAttackRange() > 150)
			{
				int iNewRange = 150;//(int)(ch->GetMobAttackRange() * 0.2);
				if (iNewRange < 150)
					iNewRange = 150;

				ch->AddAffect(AFFECT_BOW_DISTANCE, POINT_BOW_DISTANCE, iNewRange - ch->GetMobAttackRange(), AFF_NONE, 3 * 60, 0, false);
			}
		}
	}
};

struct FuncPullMonster
{
	LPCHARACTER m_ch;
	int m_iLength;
	FuncPullMonster(LPCHARACTER ch, int iLength = 300)
	{
		m_ch = ch;
		m_iLength = iLength;
	}

	void operator()(LPENTITY ent)
	{
		if (ent->IsType(ENTITY_CHARACTER))
		{
			LPCHARACTER ch = (LPCHARACTER)ent;
			if (ch->IsPC())
				return;

			if (!ch->IsMonster())
				return;

			float fDist = DISTANCE_APPROX(m_ch->GetX() - ch->GetX(), m_ch->GetY() - ch->GetY());
			if (fDist > 3000 || fDist < 100)
				return;

			float fNewDist = fDist - m_iLength;
			if (fNewDist < 100)
				fNewDist = 100;

			float degree = GetDegreeFromPositionXY(ch->GetX(), ch->GetY(), m_ch->GetX(), m_ch->GetY());
			float fx;
			float fy;

			GetDeltaByDegree(degree, fDist - fNewDist, &fx, &fy);
			long tx = (long)(ch->GetX() + fx);
			long ty = (long)(ch->GetY() + fy);

			ch->Sync(tx, ty);
			ch->Goto(tx, ty);
			ch->CalculateMoveDuration();

			ch->SyncPacket();
		}
	}
};

void CHARACTER::ForgetMyAttacker()
{
	LPSECTREE pSec = GetSectree();
	if (pSec)
	{
		FuncForgetMyAttacker f(this);
		pSec->ForEachAround(f);
	}
	ReviveInvisible(5);
}

void CHARACTER::AggregateMonster()
{
	LPSECTREE pSec = GetSectree();
	if (pSec)
	{
		FuncAggregateMonster f(this);
		pSec->ForEachAround(f);

#ifdef __AGGR_MONSTER_EFFECT__
		if ((GetBraveryCapeUseTime() + 3) > get_global_time())
			return;
		SpecificEffectPacket("d:/ymir work/effect/etc/buff/buff_item9.mse");
		SetBraveryCapeUseTime(get_global_time());
#endif
	}
}

void CHARACTER::AttractRanger()
{
	LPSECTREE pSec = GetSectree();
	if (pSec)
	{
		FuncAttractRanger f(this);
		pSec->ForEachAround(f);
	}
}

void CHARACTER::PullMonster()
{
	LPSECTREE pSec = GetSectree();
	if (pSec)
	{
		FuncPullMonster f(this);
		pSec->ForEachAround(f);
	}
}

#ifdef __DAMAGE_LIMIT_REWORK__
void CHARACTER::UpdateAggrPointEx(LPCHARACTER pAttacker, EDamageType type, long long dam, CHARACTER::TBattleInfo& info)
#else
void CHARACTER::UpdateAggrPointEx(LPCHARACTER pAttacker, EDamageType type, int dam, CHARACTER::TBattleInfo& info)
#endif
{
	switch (type)
	{
#ifdef __DAMAGE_LIMIT_REWORK__
	case DAMAGE_TYPE_NORMAL_RANGE:
		dam = (long long)(dam * 1.2f);
		break;

	case DAMAGE_TYPE_RANGE:
		dam = (long long)(dam * 1.5f);
		break;

	case DAMAGE_TYPE_MAGIC:
		dam = (long long)(dam * 1.2f);
		break;
	default:
		break;
	}

	if (pAttacker == GetVictim())
		dam = (long long)(dam * 1.2f);
#else
	case DAMAGE_TYPE_NORMAL_RANGE:
		dam = (int)(dam * 1.2f);
		break;

	case DAMAGE_TYPE_RANGE:
		dam = (int)(dam * 1.5f);
		break;

	case DAMAGE_TYPE_MAGIC:
		dam = (int)(dam * 1.2f);
		break;
	default:
		break;
	}

	if (pAttacker == GetVictim())
		dam = (int)(dam * 1.2f);
#endif

	info.iAggro += dam;

	if (info.iAggro < 0)
		info.iAggro = 0;

	if (GetParty() && dam > 0 && type != DAMAGE_TYPE_SPECIAL)
	{
		LPPARTY pParty = GetParty();

		int iPartyAggroDist = dam;

		if (pParty->GetLeaderPID() == GetVID())
			iPartyAggroDist /= 2;
		else
			iPartyAggroDist /= 3;

		pParty->SendMessage(this, PM_AGGRO_INCREASE, iPartyAggroDist, pAttacker->GetVID());
	}

	if (g_bPoisonUnaggr)
	{
		if (type != DAMAGE_TYPE_POISON)
			ChangeVictimByAggro(info.iAggro, pAttacker);
	}
	else
		ChangeVictimByAggro(info.iAggro, pAttacker);
}

#ifdef __DAMAGE_LIMIT_REWORK__
void CHARACTER::UpdateAggrPoint(LPCHARACTER pAttacker, EDamageType type, long long dam)
#else
void CHARACTER::UpdateAggrPoint(LPCHARACTER pAttacker, EDamageType type, int dam)
#endif
{
	if (IsDead() || IsStun())
		return;

	TDamageMap::iterator it = m_map_kDamage.find(pAttacker->GetVID());

	if (it == m_map_kDamage.end())
	{
		m_map_kDamage.insert(TDamageMap::value_type(pAttacker->GetVID(), TBattleInfo(0, dam)));
		it = m_map_kDamage.find(pAttacker->GetVID());
	}

	UpdateAggrPointEx(pAttacker, type, dam, it->second);
}

void CHARACTER::ChangeVictimByAggro(int iNewAggro, LPCHARACTER pNewVictim)
{
	if (get_dword_time() - m_dwLastVictimSetTime < 3000)
		return;

	if (pNewVictim == GetVictim())
	{
		if (m_iMaxAggro < iNewAggro)
		{
			m_iMaxAggro = iNewAggro;
			return;
		}

		TDamageMap::iterator it;
		TDamageMap::iterator itFind = m_map_kDamage.end();

		for (it = m_map_kDamage.begin(); it != m_map_kDamage.end(); ++it)
		{
			if (it->second.iAggro > iNewAggro)
			{
				LPCHARACTER ch = CHARACTER_MANAGER::instance().Find(it->first);

				if (ch && !ch->IsDead() && DISTANCE_APPROX(ch->GetX() - GetX(), ch->GetY() - GetY()) < 5000)
				{
					itFind = it;
					iNewAggro = it->second.iAggro;
				}
			}
		}

		if (itFind != m_map_kDamage.end())
		{
			m_iMaxAggro = iNewAggro;
			SetVictim(CHARACTER_MANAGER::instance().Find(itFind->first));
			m_dwStateDuration = 1;
		}
	}
	else
	{
		if (m_iMaxAggro < iNewAggro)
		{
			m_iMaxAggro = iNewAggro;
			SetVictim(pNewVictim);
			m_dwStateDuration = 1;
		}
	}
}

#ifdef ENABLE_LOVE_SYSTEM
const char* CHARACTER::GetLove1() const
{
	return m_love1;
}

const char* CHARACTER::GetLove2() const
{
	return m_love2;
}

void CHARACTER::ChangeLove1(const char* love)
{
	strcpy(m_love1, love);
	sys_log(0, "Love1 updating name: %s %s", GetName(), m_love1);

	RefreshLoveBonus();
	UpdatePacket();
}

void CHARACTER::ChangeLove2(const char* love)
{
	strcpy(m_love2, love);
	sys_log(0, "Love2 updating name: %s %s", GetName(), m_love2);

	UpdatePacket();
}
#endif // ENABLE_LOVE_SYSTEM

#ifdef ENABLE_WORD_SYSTEM
const char* CHARACTER::GetWord() const
{
	return m_word;
}

void CHARACTER::ChangeWord(const char* word)
{
	strcpy(m_word, word);
	sys_log(0, "Word updating name: %s %s", GetName(), m_word);

	RefreshWordBonus();
	UpdatePacket();
}
#endif // ENABLE_WORD_SYSTEM