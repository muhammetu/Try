#include "stdafx.h"
#include "constants.h"
#include "config.h"
#include "char.h"
#include "char_manager.h"
#include "affect.h"
#include "locale_service.h"

const int poison_damage_rate[MOB_RANK_MAX_NUM] =
{
	80, 50, 40, 30, 25, 1
};

int GetPoisonDamageRate(LPCHARACTER ch)
{
	int iRate;

	if (ch->IsPC())
		iRate = 50;
	else
		iRate = poison_damage_rate[ch->GetMobRank()];

	iRate = MAX(0, iRate - ch->GetPoint(POINT_POISON_REDUCE));
	return iRate;
}

EVENTINFO(TPoisonEventInfo)
{
	DynamicCharacterPtr ch;
	int		count;
	DWORD	attacker_pid;

	TPoisonEventInfo()
		: ch()
		, count(0)
		, attacker_pid(0)
	{
	}
};

EVENTFUNC(poison_event)
{
	TPoisonEventInfo* info = dynamic_cast<TPoisonEventInfo*>(event->info);

	if (info == NULL)
	{
		sys_err("poison_event> <Factor> Null pointer");
		return 0;
	}

	LPCHARACTER ch = info->ch;

	if (ch == NULL) { // <Factor>
		return 0;
	}

	if (ch->IsBoss() || ch->IsStone())
		return 0;

	LPCHARACTER pkAttacker = CHARACTER_MANAGER::instance().FindByPID(info->attacker_pid);
#ifdef __DAMAGE_LIMIT_REWORK__
	long long dam = ch->GetMaxHP() * GetPoisonDamageRate(ch) / 1000;
#else
	int dam = ch->GetMaxHP() * GetPoisonDamageRate(ch) / 1000;
#endif
	if (test_server) ch->ChatPacket(CHAT_TYPE_NOTICE, "Poison Damage %d", dam);

	// @duzenleme eger saldiran karakter yok ise damage atmayi onler.
	if (!pkAttacker)
	{
		ch->m_pkPoisonEvent = NULL;
		return 0;
	}

	if (ch->Damage(pkAttacker, dam, DAMAGE_TYPE_POISON))
	{
		ch->m_pkPoisonEvent = NULL;
		return 0;
	}

	--info->count;

	if (info->count)
		return PASSES_PER_SEC(3);
	else
	{
		ch->m_pkPoisonEvent = NULL;
		return 0;
	}
}

#ifdef __WOLFMAN_CHARACTER__
const int bleeding_damage_rate[MOB_RANK_MAX_NUM] =
{
	80, 50, 40, 30, 25, 1
};

int GetBleedingDamageRate(LPCHARACTER ch)
{
	int iRate;

	if (ch->IsPC())
		iRate = 50;
	else
		iRate = bleeding_damage_rate[ch->GetMobRank()];

	iRate = MAX(0, iRate - ch->GetPoint(POINT_BLEEDING_REDUCE));
#if defined(__WOLFMAN_CHARACTER__) && defined(USE_ITEM_BLEEDING_AS_POISON)
	iRate = MAX(0, iRate - ch->GetPoint(POINT_POISON_REDUCE));
#endif
	return iRate;
}

EVENTINFO(TBleedingEventInfo)
{
	DynamicCharacterPtr ch;
	int		count;
	DWORD	attacker_pid;

	TBleedingEventInfo()
		: ch()
		, count(0)
		, attacker_pid(0)
	{
	}
};

EVENTFUNC(bleeding_event)
{
	TBleedingEventInfo* info = dynamic_cast<TBleedingEventInfo*>(event->info);

	if (info == NULL)
	{
		sys_err("bleeding_event> <Factor> Null pointer");
		return 0;
	}

	LPCHARACTER ch = info->ch;

	if (ch == NULL) { // <Factor>
		return 0;
	}
	LPCHARACTER pkAttacker = CHARACTER_MANAGER::instance().FindByPID(info->attacker_pid);
#ifdef __DAMAGE_LIMIT_REWORK__
	long long dam = ch->GetMaxHP() * GetBleedingDamageRate(ch) / 1000;
#else
	int dam = ch->GetMaxHP() * GetBleedingDamageRate(ch) / 1000;
#endif
	if (test_server) ch->ChatPacket(CHAT_TYPE_NOTICE, "Bleeding Damage %d", dam);

	// @duzenleme eger saldiran karakter yok ise damage atmayi onler.
	if (!pkAttacker)
	{
		ch->m_pkBleedingEvent = NULL;
		return 0;
	}

	if (ch->Damage(pkAttacker, dam, DAMAGE_TYPE_BLEEDING))
	{
		ch->m_pkBleedingEvent = NULL;
		return 0;
	}

	--info->count;

	if (info->count)
		return PASSES_PER_SEC(3);
	else
	{
		ch->m_pkBleedingEvent = NULL;
		return 0;
	}
}
#endif

EVENTINFO(TFireEventInfo)
{
	DynamicCharacterPtr ch;
	int		count;
	int		amount;
	DWORD	attacker_pid;

	TFireEventInfo()
		: ch()
		, count(0)
		, amount(0)
		, attacker_pid(0)
	{
	}
};

EVENTFUNC(fire_event)
{
	TFireEventInfo* info = dynamic_cast<TFireEventInfo*>(event->info);

	if (info == NULL)
	{
		sys_err("fire_event> <Factor> Null pointer");
		return 0;
	}

	LPCHARACTER ch = info->ch;
	if (ch == NULL) { // <Factor>
		return 0;
	}
	LPCHARACTER pkAttacker = CHARACTER_MANAGER::instance().FindByPID(info->attacker_pid);

#ifdef __DAMAGE_LIMIT_REWORK__
	long long dam = info->amount;
#else
	int dam = info->amount;
#endif
	if (test_server) ch->ChatPacket(CHAT_TYPE_NOTICE, "Fire Damage %d", dam);

	if (ch->Damage(pkAttacker, dam, DAMAGE_TYPE_FIRE))
	{
		ch->m_pkFireEvent = NULL;
		return 0;
	}

	--info->count;

	if (info->count)
		return PASSES_PER_SEC(3);
	else
	{
		ch->m_pkFireEvent = NULL;
		return 0;
	}
}

static int poison_level_adjust[9] =
{
	100, 90, 80, 70, 50, 30, 10, 5, 0
};

#ifdef __WOLFMAN_CHARACTER__
static int bleeding_level_adjust[9] =
{
	100, 90, 80, 70, 50, 30, 10, 5, 0
};
#endif

void CHARACTER::AttackedByFire(LPCHARACTER pkAttacker, int amount, int count)
{
	if (m_pkFireEvent)
		return;

	AddAffect(AFFECT_FIRE, POINT_NONE, 0, AFF_FIRE, count * 3 + 1, 0, true);

	TFireEventInfo* info = AllocEventInfo<TFireEventInfo>();

	info->ch = this;
	info->count = count;
	info->amount = amount;
	info->attacker_pid = pkAttacker->GetPlayerID();

	m_pkFireEvent = event_create(fire_event, info, 1);
}

void CHARACTER::AttackedByPoison(LPCHARACTER pkAttacker)
{
	if (m_pkPoisonEvent)
		return;

	if (IsBoss())
		return;

	if (m_bHasPoisoned && !IsPC())
	{
		return;
	}

	if (pkAttacker && pkAttacker->GetLevel() < GetLevel())
	{
		int delta = GetLevel() - pkAttacker->GetLevel();

		if (delta > 8)
			delta = 8;

		if (number(1, 100) > poison_level_adjust[delta])
			return;
	}

	/*if (IsImmune(IMMUNE_POISON))
	  return;*/

	m_bHasPoisoned = true;

	AddAffect(AFFECT_POISON, POINT_NONE, 0, AFF_POISON, POISON_LENGTH + 1, 0, IsPC()); // pc ise override yap, degilse ustune ekle, aramakla ugrasma

	TPoisonEventInfo* info = AllocEventInfo<TPoisonEventInfo>();

	info->ch = this;
	info->count = 10;
	info->attacker_pid = pkAttacker ? pkAttacker->GetPlayerID() : 0;

	m_pkPoisonEvent = event_create(poison_event, info, 1);

	if (test_server && pkAttacker)
	{
		char buf[256];
		snprintf(buf, sizeof(buf), "POISON %s -> %s", pkAttacker->GetName(), GetName());
		pkAttacker->ChatPacket(CHAT_TYPE_INFO, "%s", buf);
	}
}

#ifdef __WOLFMAN_CHARACTER__
void CHARACTER::AttackedByBleeding(LPCHARACTER pkAttacker)
{
	if (m_pkBleedingEvent)
		return;

	if (m_bHasBled && !IsPC())
		return;

	if (pkAttacker && pkAttacker->GetLevel() < GetLevel())
	{
		int delta = GetLevel() - pkAttacker->GetLevel();

		if (delta > 8)
			delta = 8;

		if (number(1, 100) > bleeding_level_adjust[delta])
			return;
	}

	/*if (IsImmune(IMMUNE_BLEEDING))
	  return;*/

	m_bHasBled = true;

	AddAffect(AFFECT_BLEEDING, POINT_NONE, 0, AFF_BLEEDING, BLEEDING_LENGTH + 1, 0, true);

	TBleedingEventInfo* info = AllocEventInfo<TBleedingEventInfo>();

	info->ch = this;
	info->count = 10;
	info->attacker_pid = pkAttacker ? pkAttacker->GetPlayerID() : 0;

	m_pkBleedingEvent = event_create(bleeding_event, info, 1);

	if (test_server && pkAttacker)
	{
		char buf[256];
		snprintf(buf, sizeof(buf), "BLEEDING %s -> %s", pkAttacker->GetName(), GetName());
		pkAttacker->ChatPacket(CHAT_TYPE_INFO, "%s", buf);
	}
}
#endif

void CHARACTER::RemoveFire()
{
	RemoveAffect(AFFECT_FIRE);
	event_cancel(&m_pkFireEvent);
}

void CHARACTER::RemovePoison()
{
	RemoveAffect(AFFECT_POISON);
	event_cancel(&m_pkPoisonEvent);
}

#ifdef __WOLFMAN_CHARACTER__
void CHARACTER::RemoveBleeding()
{
	RemoveAffect(AFFECT_BLEEDING);
	event_cancel(&m_pkBleedingEvent);
}
#endif

void CHARACTER::ApplyMobAttribute(const TMobTable* table)
{
	for (int i = 0; i < MOB_ENCHANTS_MAX_NUM; ++i)
	{
		if (table->cEnchants[i] != 0)
			ApplyPoint(aiMobEnchantApplyIdx[i], table->cEnchants[i]);
	}
#if defined(__WOLFMAN_CHARACTER__) && defined(USE_MOB_BLEEDING_AS_POISON)
	if (table->cEnchants[MOB_ENCHANT_POISON] != 0)
		ApplyPoint(APPLY_BLEEDING_PCT, table->cEnchants[MOB_ENCHANT_POISON] / 50); // @warme009
#endif

	for (int i = 0; i < MOB_RESISTS_MAX_NUM; ++i)
	{
		if (table->cResists[i] != 0)
			ApplyPoint(aiMobResistsApplyIdx[i], table->cResists[i]);
	}
#if defined(__WOLFMAN_CHARACTER__) && defined(USE_MOB_CLAW_AS_DAGGER)
	/*if (table->cResists[MOB_RESIST_DAGGER] != 0)
		ApplyPoint(APPLY_RESIST_CLAW, table->cResists[MOB_RESIST_DAGGER]);*/
#endif
#if defined(__WOLFMAN_CHARACTER__) && defined(USE_MOB_BLEEDING_AS_POISON)
	if (table->cResists[MOB_RESIST_POISON] != 0)
		ApplyPoint(APPLY_BLEEDING_REDUCE, table->cResists[MOB_RESIST_POISON]);
#endif

	m_bIsLoadedAffect = true;
}

bool CHARACTER::IsImmune(DWORD dwImmuneFlag)
{
	// 1 stun, 2 slow, 4 fall = 7 all == X
	// ChatPacket(CHAT_TYPE_PARTY, "<IMMUNE_IS> (%u == %u)", m_pointsInstant.dwImmuneFlag, dwImmuneFlag);
	if (IS_SET(m_pointsInstant.dwImmuneFlag, dwImmuneFlag))
		return true;

	if (test_server && IsPC())
		ChatPacket(CHAT_TYPE_PARTY, "<IMMUNE_FAIL> (%s) NO_IMMUNE_FLAG", GetName());

	return false;
}