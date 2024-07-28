#include "stdafx.h"
#include "utils.h"
#include "config.h"
#include "desc.h"
#include "char.h"
#include "char_manager.h"
#include "battle.h"
#include "item.h"
#include "item_manager.h"
#include "mob_manager.h"
#include "vector.h"
#include "packet.h"
#include "pvp.h"
#include "guild.h"
#include "affect.h"
#include "unique_item.h"
#include "lua_incl.h"
#include "sectree.h"
#include "locale_service.h"

#ifdef __EVENT_SYSTEM__
#include "game_events.h"
#endif // __EVENT_SYSTEM__

#ifdef __DAMAGE_LIMIT_REWORK__
long long battle_hit(LPCHARACTER ch, LPCHARACTER victim, long long& iRetDam);
#else
int battle_hit(LPCHARACTER ch, LPCHARACTER victim, int& iRetDam);
#endif

bool battle_distance_valid_by_xy(long x, long y, long tx, long ty)
{
	long distance = DISTANCE_APPROX(x - tx, y - ty);

	if (distance > 170)
		return false;

	return true;
}

bool battle_distance_valid(LPCHARACTER ch, LPCHARACTER victim)
{
	return battle_distance_valid_by_xy(ch->GetX(), ch->GetY(), victim->GetX(), victim->GetY());
}

bool timed_event_cancel(LPCHARACTER ch)
{
	if (ch->m_pkTimedEvent)
	{
		event_cancel(&ch->m_pkTimedEvent);
		return true;
	}

	return false;
}

bool battle_is_attackable(LPCHARACTER ch, LPCHARACTER victim)
{
	if (victim->IsDead())
		return false;

	if (ch->IsPC() && victim->IsPC() && (victim->GetMapIndex() != 325))
		return false;

	if (ch->IsInSafezone())
		return false;

	if (victim->IsInSafezone())
		return false;

	if (ch->IsStun() || ch->IsDead())
		return false;

	if (!victim->IsPC() && DISTANCE_APPROX(victim->GetX() - ch->GetX(), victim->GetY() - ch->GetY()) > 300)
		return false;

	if (ch->IsPC() && victim->IsPC())
	{
		CGuild* g1 = ch->GetGuild();
		CGuild* g2 = victim->GetGuild();

		if (g1 && g2)
		{
			if (g1->UnderWar(g2->GetID()))
				return true;
		}
	}

	return CPVPManager::instance().CanAttack(ch, victim);
}

#ifdef __DAMAGE_LIMIT_REWORK__
long long battle_melee_attack(LPCHARACTER ch, LPCHARACTER victim)
#else
int battle_melee_attack(LPCHARACTER ch, LPCHARACTER victim)
#endif
{
	if (test_server && ch->IsPC())
		sys_log(0, "battle_melee_attack : [%s] attack to [%s]", ch->GetName(), victim->GetName());

	if (!victim || ch == victim)
		return BATTLE_NONE;

	if (test_server && ch->IsPC())
		sys_log(0, "battle_melee_attack : [%s] attack to [%s]", ch->GetName(), victim->GetName());

	if (!battle_is_attackable(ch, victim))
		return BATTLE_NONE;

	if (test_server && ch->IsPC())
		sys_log(0, "battle_melee_attack : [%s] attack to [%s]", ch->GetName(), victim->GetName());

	int distance = DISTANCE_APPROX(ch->GetX() - victim->GetX(), ch->GetY() - victim->GetY());

	int max = 300;

	if (false == ch->IsPC())
	{
		max = (int)(ch->GetMobAttackRange() * 1.15f);
	}
	else
	{
		if (false == victim->IsPC() && BATTLE_TYPE_MELEE == victim->GetMobBattleType())
			max = MAX(450, (int)(victim->GetMobAttackRange() * 1.15f));
	}

	if (distance > max)
	{
		if (test_server)
			sys_log(0, "VICTIM_FAR: %s distance: %d max: %d", ch->GetName(), distance, max);

		return BATTLE_NONE;
	}

	if (timed_event_cancel(ch))
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("전투가 시작 되어 취소 되었습니다."));

	if (timed_event_cancel(victim))
		victim->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("전투가 시작 되어 취소 되었습니다."));

	ch->SetPosition(POS_FIGHTING);
	ch->SetVictim(victim);

	const PIXEL_POSITION& vpos = victim->GetXYZ();
	ch->SetRotationToXY(vpos.x, vpos.y);

#ifdef __DAMAGE_LIMIT_REWORK__
	long long dam;
	long long ret = battle_hit(ch, victim, dam);
#else
	int dam;
	int ret = battle_hit(ch, victim, dam);
#endif
	return (ret);
}

void battle_end_ex(LPCHARACTER ch)
{
	if (ch->IsPosition(POS_FIGHTING))
		ch->SetPosition(POS_STANDING);
}

void battle_end(LPCHARACTER ch)
{
	battle_end_ex(ch);
}

// AG = Attack Grade
// AL = Attack Limit
#ifdef __DAMAGE_LIMIT_REWORK__
long long CalcBattleDamage(long long iDam, int iAttackerLev, int iVictimLev)
#else
int CalcBattleDamage(int iDam, int iAttackerLev, int iVictimLev)
#endif
{
	if (iDam < 3)
		iDam = number(1, 5);

	//return CALCULATE_DAMAGE_LVDELTA(iAttackerLev, iVictimLev, iDam);
	return iDam;
}
#ifdef __DAMAGE_LIMIT_REWORK__
long long CalcMagicDamageWithValue(long long iDam, LPCHARACTER pkAttacker, LPCHARACTER pkVictim)
#else
int CalcMagicDamageWithValue(int iDam, LPCHARACTER pkAttacker, LPCHARACTER pkVictim)
#endif
{
	return CalcBattleDamage(iDam, pkAttacker->GetLevel(), pkVictim->GetLevel());
}

#ifdef __DAMAGE_LIMIT_REWORK__
long long CalcMagicDamage(LPCHARACTER pkAttacker, LPCHARACTER pkVictim)
{
	long long iDam = 0;
#else
int CalcMagicDamage(LPCHARACTER pkAttacker, LPCHARACTER pkVictim)
{
	int iDam = 0;
#endif
	if (pkAttacker->IsNPC())
	{
		iDam = CalcMeleeDamage(pkAttacker, pkVictim, false, false);
	}

	iDam += pkAttacker->GetPoint(POINT_PARTY_ATTACKER_BONUS);

	return CalcMagicDamageWithValue(iDam, pkAttacker, pkVictim);
}

float CalcAttackRating(LPCHARACTER pkAttacker, LPCHARACTER pkVictim, bool bIgnoreTargetRating)
{
	int iARSrc;
	int iERSrc;

	{
		int attacker_dx = pkAttacker->GetPoint(POINT_DX);
		int attacker_lv = pkAttacker->GetLevel();

		int victim_dx = pkVictim->GetPoint(POINT_DX);
		int victim_lv = pkAttacker->GetLevel();

		iARSrc = MIN(90, (attacker_dx * 4 + attacker_lv * 2) / 6);
		iERSrc = MIN(90, (victim_dx * 4 + victim_lv * 2) / 6);
	}

	float fAR = ((float)iARSrc + 210.0f) / 300.0f; // fAR = 0.7 ~ 1.0

	if (bIgnoreTargetRating)
		return fAR;

	// ((Edx * 2 + 20) / (Edx + 110)) * 0.3
	float fER = ((float)(iERSrc * 2 + 5) / (iERSrc + 95)) * 3.0f / 10.0f;

	return fAR - fER;
}

// @edit017 BEGIN
#ifdef __DAMAGE_LIMIT_REWORK__
long long CalcAttBonus(LPCHARACTER pkAttacker, LPCHARACTER pkVictim, long long iAtk)
{
	long long iBasicAtk = iAtk; // temel atak degeri.
#else
int CalcAttBonus(LPCHARACTER pkAttacker, LPCHARACTER pkVictim, int iAtk)
{
	int iBasicAtk = iAtk; // temel atak degeri.
#endif
	// Ilk once saldiri degerini yukseltecegiz.
	// sonrasinda savunmalari ekleyip dusurecegiz
	// en sona duzgun hasar degeri kalacak.

	// MobDamageRate ekleme
	if (pkAttacker->IsNPC() && pkVictim->IsPC())
	{
		iBasicAtk = (iAtk * CHARACTER_MANAGER::instance().GetMobDamageRate(pkAttacker)) / 100;
	}

	// Pendant system
#ifdef __PENDANT_SYSTEM__
	if (pkVictim->IsPC())
	{
		LPITEM pkWeapon = pkVictim->GetWear(WEAR_WEAPON);
		if (pkWeapon)
		{
			switch (pkWeapon->GetSubType())
			{
			case WEAPON_SWORD:
				iBasicAtk += (iAtk * pkAttacker->GetPoint(POINT_ATTBONUS_SWORD)) / 100;
				break;

			case WEAPON_TWO_HANDED:
				iBasicAtk += (iAtk * pkAttacker->GetPoint(POINT_ATTBONUS_TWOHAND)) / 100;
				break;

			case WEAPON_DAGGER:
				iBasicAtk += (iAtk * pkAttacker->GetPoint(POINT_ATTBONUS_DAGGER)) / 100;
				break;

			case WEAPON_BELL:
				iBasicAtk += (iAtk * pkAttacker->GetPoint(POINT_ATTBONUS_BELL)) / 100;
				break;

			case WEAPON_FAN:
				iBasicAtk += (iAtk * pkAttacker->GetPoint(POINT_ATTBONUS_FAN)) / 100;
				break;

			case WEAPON_BOW:
				iBasicAtk += (iAtk * pkAttacker->GetPoint(POINT_ATTBONUS_BOW)) / 100;
				break;

#ifdef __WOLFMAN_CHARACTER__
			case WEAPON_CLAW:
				iBasicAtk += (iAtk * pkAttacker->GetPoint(POINT_ATTBONUS_DAGGER)) / 100;
				break;

#endif
			}
		}
	}
#endif

	// mob hasar degeri hesaplama
	if (pkVictim->IsNPC())
	{
		if (pkVictim->IsRaceFlag(RACE_FLAG_ANIMAL))
			iBasicAtk += (iAtk * pkAttacker->GetPoint(POINT_ATTBONUS_ANIMAL)) / 100;
		if (pkVictim->IsRaceFlag(RACE_FLAG_UNDEAD))
			iBasicAtk += (iAtk * pkAttacker->GetPoint(POINT_ATTBONUS_UNDEAD)) / 100;
		if (pkVictim->IsRaceFlag(RACE_FLAG_DEVIL))
			iBasicAtk += (iAtk * pkAttacker->GetPoint(POINT_ATTBONUS_DEVIL)) / 100;
		if (pkVictim->IsRaceFlag(RACE_FLAG_HUMAN))
			iBasicAtk += (iAtk * pkAttacker->GetPoint(POINT_ATTBONUS_HUMAN)) / 100;
		if (pkVictim->IsRaceFlag(RACE_FLAG_ORC))
			iBasicAtk += (iAtk * pkAttacker->GetPoint(POINT_ATTBONUS_ORC)) / 100;
		if (pkVictim->IsRaceFlag(RACE_FLAG_MILGYO))
			iBasicAtk += (iAtk * pkAttacker->GetPoint(POINT_ATTBONUS_MILGYO)) / 100;
		if (pkVictim->IsRaceFlag(RACE_FLAG_INSECT))
			iBasicAtk += (iAtk * pkAttacker->GetPoint(POINT_ATTBONUS_INSECT)) / 100;
		if (pkVictim->IsRaceFlag(RACE_FLAG_FIRE))
			iBasicAtk += (iAtk * pkAttacker->GetPoint(POINT_ATTBONUS_FIRE)) / 100;
		if (pkVictim->IsRaceFlag(RACE_FLAG_ICE))
			iBasicAtk += (iAtk * pkAttacker->GetPoint(POINT_ATTBONUS_ICE)) / 100;
		if (pkVictim->IsRaceFlag(RACE_FLAG_DESERT))
			iBasicAtk += (iAtk * pkAttacker->GetPoint(POINT_ATTBONUS_DESERT)) / 100;
		if (pkVictim->IsRaceFlag(RACE_FLAG_TREE))
			iBasicAtk += (iAtk * pkAttacker->GetPoint(POINT_ATTBONUS_TREE)) / 100;
#ifdef __PENDANT_SYSTEM__
		if (pkVictim->IsRaceFlag(RACE_FLAG_ATT_ELEC))
			iBasicAtk += (iAtk * pkAttacker->GetPoint(POINT_ENCHANT_ELECT)) / 100;
		if (pkVictim->IsRaceFlag(RACE_FLAG_ATT_WIND))
			iBasicAtk += (iAtk * pkAttacker->GetPoint(POINT_ENCHANT_WIND)) / 100;
		if (pkVictim->IsRaceFlag(RACE_FLAG_ATT_EARTH))
			iBasicAtk += (iAtk * pkAttacker->GetPoint(POINT_ENCHANT_EARTH)) / 100;
		if (pkVictim->IsRaceFlag(RACE_FLAG_ATT_DARK))
			iBasicAtk += (iAtk * pkAttacker->GetPoint(POINT_ENCHANT_DARK)) / 100;
		if (pkVictim->IsRaceFlag(RACE_FLAG_ATT_FIRE))
			iBasicAtk += (iAtk * pkAttacker->GetPoint(POINT_ENCHANT_FIRE)) / 100;
		if (pkVictim->IsRaceFlag(RACE_FLAG_ATT_ICE))
			iBasicAtk += (iAtk * pkAttacker->GetPoint(POINT_ENCHANT_ICE)) / 100;
#endif

#ifdef __ATTRIBUTES_TYPES__
		if (pkVictim->IsRaceFlag(RACE_FLAG_ATT_ELEC) || pkVictim->IsRaceFlag(RACE_FLAG_ATT_WIND) || pkVictim->IsRaceFlag(RACE_FLAG_ATT_EARTH) || pkVictim->IsRaceFlag(RACE_FLAG_ATT_DARK) || pkVictim->IsRaceFlag(RACE_FLAG_ATT_FIRE) || pkVictim->IsRaceFlag(RACE_FLAG_ATT_ICE) || pkVictim->IsBoss())
			iBasicAtk += (iAtk * pkAttacker->GetPoint(POINT_ATTBONUS_ELEMENTS)) / 100;
#endif

		long long iStoneBonus = pkAttacker->GetPoint(POINT_ATTBONUS_STONE);
		long long iBossBonus = pkAttacker->GetPoint(POINT_ATTBONUS_BOSS);
		long long iMonsterBonus = pkAttacker->GetPoint(POINT_ATTBONUS_MONSTER);

#ifdef __PARTY_ROLE_REWORK__
		iMonsterBonus += pkAttacker->GetPoint(POINT_PARTY_ATTACKER_MONSTER_BONUS);
		iStoneBonus += pkAttacker->GetPoint(POINT_PARTY_ATTACKER_STONE_BONUS);
		iBossBonus += pkAttacker->GetPoint(POINT_PARTY_ATTACKER_BOSS_BONUS);
#endif

#ifdef __SUB_SKILL_REWORK__
		CSkillProto* pkSk = CSkillManager::instance().Get(SKILL_SUB_MONSTER);

		if (NULL != pkSk)
		{
			pkSk->SetPointVar("k", 1.0f * pkAttacker->GetSkillPower(SKILL_SUB_MONSTER) / 100.0f);
			iMonsterBonus += static_cast<int>(pkSk->kPointPoly.Eval());
		}

		if (pkVictim->IsStone())
		{
			CSkillProto* pkSk = CSkillManager::instance().Get(SKILL_SUB_STONE);

			if (NULL != pkSk)
			{
				pkSk->SetPointVar("k", 1.0f * pkAttacker->GetSkillPower(SKILL_SUB_STONE) / 100.0f);
				iStoneBonus += static_cast<int>(pkSk->kPointPoly.Eval());
			}
		}

		if (pkVictim->IsBoss())
		{
			CSkillProto* pkSk = CSkillManager::instance().Get(SKILL_SUB_BOSS);

			if (NULL != pkSk)
			{
				pkSk->SetPointVar("k", 1.0f * pkAttacker->GetSkillPower(SKILL_SUB_BOSS) / 100.0f);
				iBossBonus += static_cast<int>(pkSk->kPointPoly.Eval());
			}
		}
#endif

#ifdef __ATTRIBUTES_TYPES__
		if (pkVictim->IsStone())
			iBasicAtk += (iBasicAtk * iStoneBonus) / 100;
		if (pkVictim->IsBoss())
			iBasicAtk += (iBasicAtk * iBossBonus) / 100;
#endif

		iBasicAtk += (iBasicAtk * iMonsterBonus) / 100;
	}
	else if (pkVictim->IsPC())
	{
		CSkillProto* pkSk = CSkillManager::instance().Get(SKILL_SUB_HUMAN);

		if (NULL != pkSk)
		{
			pkSk->SetPointVar("k", 1.0f * pkAttacker->GetSkillPower(SKILL_SUB_HUMAN) / 100.0f);
			iBasicAtk += (iAtk * static_cast<int>(pkSk->kPointPoly.Eval())) / 100;
		}

		iBasicAtk += (iAtk * pkAttacker->GetPoint(POINT_ATTBONUS_HUMAN)) / 100;
#ifdef __ATTRIBUTES_TYPES__
		iBasicAtk += (iAtk * pkAttacker->GetPoint(POINT_ATTBONUS_CHARACTERS)) / 100;
#endif
		switch (pkVictim->GetJob())
		{
		case JOB_WARRIOR:
			iBasicAtk += (iAtk * pkAttacker->GetPoint(POINT_ATTBONUS_WARRIOR)) / 100;
			break;

		case JOB_ASSASSIN:
			iBasicAtk += (iAtk * pkAttacker->GetPoint(POINT_ATTBONUS_ASSASSIN)) / 100;
			break;

		case JOB_SURA:
			iBasicAtk += (iAtk * pkAttacker->GetPoint(POINT_ATTBONUS_SURA)) / 100;
			break;

		case JOB_SHAMAN:
			iBasicAtk += (iAtk * pkAttacker->GetPoint(POINT_ATTBONUS_SHAMAN)) / 100;
			break;
#ifdef __WOLFMAN_CHARACTER__
		case JOB_WOLFMAN:
			iBasicAtk += (iAtk * pkAttacker->GetPoint(POINT_ATTBONUS_WOLFMAN)) / 100;
			break;
#endif
		}
	}

	// saldirilar bitti savunmalara geciliyor.
	if (pkAttacker->IsPC() == true)
	{
#ifdef __PENDANT_SYSTEM__
		iBasicAtk -= (iAtk * pkVictim->GetPoint(POINT_RESIST_HUMAN)) / 100;
#endif
#ifdef __ATTRIBUTES_TYPES__
		iBasicAtk -= (iAtk * pkVictim->GetPoint(POINT_ENCHANT_CHARACTERS)) / 100;
#endif
		switch (pkAttacker->GetJob())
		{
		case JOB_WARRIOR:
			iBasicAtk -= (iAtk * pkVictim->GetPoint(POINT_RESIST_WARRIOR)) / 100;
			break;

		case JOB_ASSASSIN:
			iBasicAtk -= (iAtk * pkVictim->GetPoint(POINT_RESIST_ASSASSIN)) / 100;
			break;

		case JOB_SURA:
			iBasicAtk -= (iAtk * pkVictim->GetPoint(POINT_RESIST_SURA)) / 100;
			break;

		case JOB_SHAMAN:
			iBasicAtk -= (iAtk * pkVictim->GetPoint(POINT_RESIST_SHAMAN)) / 100;
			break;
#ifdef __WOLFMAN_CHARACTER__
		case JOB_WOLFMAN:
			iBasicAtk -= (iAtk * pkVictim->GetPoint(POINT_RESIST_WOLFMAN)) / 100;
			break;
#endif
		}
	}

#ifdef __DAMAGE_MULTIPLER__
	if (pkAttacker->IsPC() && !pkVictim->IsPC())
		iBasicAtk *= DAMAGE_MULTIPLER_1;
#endif

#ifdef __EVENT_SYSTEM__
	if (CGameEventsManager::instance().IsActivateEvent(EVENT_MAXDAMAGE) == true)
		iBasicAtk += iBasicAtk/10;
#endif // __EVENT_SYSTEM__

	return iBasicAtk;
}
// @edit017 END

#ifdef __DAMAGE_LIMIT_REWORK__
void Item_GetDamage(LPITEM pkItem, long long* pdamMin, long long* pdamMax)
#else
void Item_GetDamage(LPITEM pkItem, int* pdamMin, int* pdamMax)
#endif
{
	*pdamMin = 0;
	*pdamMax = 1;

	if (!pkItem)
		return;

	switch (pkItem->GetType())
	{
	case ITEM_ROD:
	case ITEM_PICK:
		return;
	}

	if (pkItem->GetType() != ITEM_WEAPON)
		sys_err("Item_GetDamage - !ITEM_WEAPON vnum=%d, type=%d", pkItem->GetOriginalVnum(), pkItem->GetType());

	*pdamMin = pkItem->GetValue(3);
	*pdamMax = pkItem->GetValue(4);
}
#ifdef __DAMAGE_LIMIT_REWORK__
long long CalcMeleeDamage(LPCHARACTER pkAttacker, LPCHARACTER pkVictim, bool bIgnoreDefense, bool bIgnoreTargetRating)
#else
int CalcMeleeDamage(LPCHARACTER pkAttacker, LPCHARACTER pkVictim, bool bIgnoreDefense, bool bIgnoreTargetRating)
#endif
{
	LPITEM pWeapon = pkAttacker->GetWear(WEAR_WEAPON);

	if (pWeapon)
	{
		if (pWeapon->GetType() != ITEM_WEAPON)
			return 0;

		switch (pWeapon->GetSubType())
		{
		case WEAPON_SWORD:
		case WEAPON_DAGGER:
		case WEAPON_TWO_HANDED:
		case WEAPON_BELL:
		case WEAPON_FAN:
		case WEAPON_MOUNT_SPEAR:
#ifdef __WOLFMAN_CHARACTER__
		case WEAPON_CLAW:
#endif
			break;

		case WEAPON_BOW:
			sys_err("CalcMeleeDamage should not handle bows (name: %s)", pkAttacker->GetName());
			return 0;

		default:
			return 0;
		}
	}

#ifdef __DAMAGE_LIMIT_REWORK__
	long long iDam = 0;
	float fAR = CalcAttackRating(pkAttacker, pkVictim, bIgnoreTargetRating);
	long long iDamMin = 0, iDamMax = 0;
#else
	int iDam = 0;
	float fAR = CalcAttackRating(pkAttacker, pkVictim, bIgnoreTargetRating);
	int iDamMin = 0, iDamMax = 0;
#endif

	if (pWeapon)
	{
		// MONKEY_ROD_ATTACK_BUG_FIX
		Item_GetDamage(pWeapon, &iDamMin, &iDamMax);
		// END_OF_MONKEY_ROD_ATTACK_BUG_FIX
	}
	else if (pkAttacker->IsNPC())
	{
		iDamMin = pkAttacker->GetMobDamageMin();
		iDamMax = pkAttacker->GetMobDamageMax();
	}

	iDam = number(iDamMin, iDamMax) * 2;

#ifdef __DAMAGE_LIMIT_REWORK__
	long long iAtk = 0;
#else
	int iAtk = 0;
#endif

	// level must be ignored when multiply by fAR, so subtract it before calculation.
	if (pkVictim->IsPC())
		iAtk = pkAttacker->GetPoint(POINT_ATT_GRADE) - pkAttacker->GetPoint(POINT_ATT_GRADE_BONUS) + iDam - (pkAttacker->GetLevel() * 2);
	else
		iAtk = pkAttacker->GetPoint(POINT_ATT_GRADE) + iDam - (pkAttacker->GetLevel() * 2);
	iAtk = (long long)(iAtk * fAR);
	iAtk += pkAttacker->GetLevel() * 2; // and add again

	if (pWeapon)
	{
		iAtk += pWeapon->GetValue(5) * 2;
	}

	iAtk += pkAttacker->GetPoint(POINT_PARTY_ATTACKER_BONUS); // party attacker role bonus
	if (!pkVictim->IsPC())
		iAtk = (long long)(iAtk * (100 + (pkAttacker->GetPoint(POINT_ATT_BONUS) + pkAttacker->GetPoint(POINT_MELEE_MAGIC_ATT_BONUS_PER))) / 100);

	iAtk = CalcAttBonus(pkAttacker, pkVictim, iAtk);

	// @duzenleme mall attbonus hesaplamasi degistirildi.
	if (!pkVictim->IsPC())
	{
		if (pkAttacker->GetPoint(POINT_MALL_ATTBONUS) > 0)
		{
	#ifdef __DAMAGE_LIMIT_REWORK__
			long long add_dam = (iAtk * pkAttacker->GetLimitPoint(POINT_MALL_ATTBONUS)) / 100;
	#else
			int add_dam = (iAtk * pkAttacker->GetLimitPoint(POINT_MALL_ATTBONUS)) / 100;
	#endif
			iAtk += add_dam;
		}

#ifdef __SUB_SKILL_REWORK__
		{
			CSkillProto* pkSk = CSkillManager::instance().Get(SKILL_SUB_BERSERKER);

			if (NULL != pkSk)
			{
				pkSk->SetPointVar("k", 1.0f * pkAttacker->GetSkillPower(SKILL_SUB_BERSERKER) / 100.0f);
				iAtk += (iAtk * static_cast<int>(pkSk->kPointPoly.Eval())) / 100;
			}
		}
#endif
	}

	int iDef = 0;

	if (!bIgnoreDefense)
		iDef = (pkVictim->GetPoint(POINT_DEF_GRADE) * (100 + pkVictim->GetPoint(POINT_DEF_BONUS)) / 100);

	if (pkAttacker->IsNPC())
		iAtk = (int)(iAtk * pkAttacker->GetMobDamageMultiply());

	iDam = MAX(0, iAtk - iDef);

	return CalcBattleDamage(iDam, pkAttacker->GetLevel(), pkVictim->GetLevel());
}

#ifdef __DAMAGE_LIMIT_REWORK__
long long CalcArrowDamage(LPCHARACTER pkAttacker, LPCHARACTER pkVictim, LPITEM pkBow, LPITEM pkArrow, bool bIgnoreDefense)
#else
int CalcArrowDamage(LPCHARACTER pkAttacker, LPCHARACTER pkVictim, LPITEM pkBow, LPITEM pkArrow, bool bIgnoreDefense)
#endif
{
	if (!pkBow || pkBow->GetType() != ITEM_WEAPON || pkBow->GetSubType() != WEAPON_BOW)
		return 0;

	if (!pkArrow)
		return 0;

	int iDist = (int)(DISTANCE_SQRT(pkAttacker->GetX() - pkVictim->GetX(), pkAttacker->GetY() - pkVictim->GetY()));
	//int iGap = (iDist / 100) - 5 - pkBow->GetValue(5) - pkAttacker->GetPoint(POINT_BOW_DISTANCE);
	int iGap = (iDist / 100) - 5 - pkAttacker->GetPoint(POINT_BOW_DISTANCE);
	int iPercent = 100 - (iGap * 5);

#ifdef __QUIVER_SYSTEM__
	if (pkArrow->GetSubType() == WEAPON_QUIVER)
		iPercent = 100;
#endif

	if (iPercent <= 0)
		return 0;
	else if (iPercent > 100)
		iPercent = 100;
#ifdef __DAMAGE_LIMIT_REWORK__
	long long iDam = 0;
#else
	int iDam = 0;
#endif
	float fAR = CalcAttackRating(pkAttacker, pkVictim, false);
	iDam = number(pkBow->GetValue(3), pkBow->GetValue(4)) * 2 + pkArrow->GetValue(3);
#ifdef __DAMAGE_LIMIT_REWORK__
	long long iAtk;
#else
	int iAtk;
#endif
	// level must be ignored when multiply by fAR, so subtract it before calculation.
	if (pkVictim->IsPC())
		iAtk = pkAttacker->GetPoint(POINT_ATT_GRADE) - pkAttacker->GetPoint(POINT_ATT_GRADE_BONUS) + iDam - (pkAttacker->GetLevel() * 2);
	else
		iAtk = pkAttacker->GetPoint(POINT_ATT_GRADE) + iDam - (pkAttacker->GetLevel() * 2);

	iAtk = (long long)(iAtk * fAR);
	iAtk += pkAttacker->GetLevel() * 2; // and add again

	// Refine Grade
	iAtk += pkBow->GetValue(5) * 2;

	iAtk += pkAttacker->GetPoint(POINT_PARTY_ATTACKER_BONUS);
	if (!pkVictim->IsPC())
		iAtk = (long long)(iAtk * (100 + (pkAttacker->GetPoint(POINT_ATT_BONUS) + pkAttacker->GetPoint(POINT_MELEE_MAGIC_ATT_BONUS_PER))) / 100);

	iAtk = CalcAttBonus(pkAttacker, pkVictim, iAtk);

	// @duzenleme mall attbonus hesaplamasi degistirildi.
	if (!pkVictim->IsPC())
	{
		if (pkAttacker->GetPoint(POINT_MALL_ATTBONUS) > 0)
		{
#ifdef __DAMAGE_LIMIT_REWORK__
			long long add_dam = (iAtk * pkAttacker->GetLimitPoint(POINT_MALL_ATTBONUS)) / 100;
#else
			int add_dam = (iAtk * pkAttacker->GetLimitPoint(POINT_MALL_ATTBONUS)) / 100;
#endif
			iAtk += add_dam;
		}
	}

	int iDef = 0;

	if (!bIgnoreDefense)
		iDef = (pkVictim->GetPoint(POINT_DEF_GRADE) * (100 + pkAttacker->GetPoint(POINT_DEF_BONUS)) / 100);

	if (pkAttacker->IsNPC())
		iAtk = (long long)(iAtk * pkAttacker->GetMobDamageMultiply());

	iDam = MAX(0, iAtk - iDef);
#ifdef __DAMAGE_LIMIT_REWORK__
	long long iPureDam = iDam;
#else
	int iPureDam = iDam;
#endif
	iPureDam = (iPureDam * iPercent) / 100;

	if (test_server)
	{
		pkAttacker->ChatPacket(CHAT_TYPE_INFO, "ARROW %s -> %s, DAM %d DIST %d GAP %d %% %d",
			pkAttacker->GetName(),
			pkVictim->GetName(),
			iPureDam,
			iDist, iGap, iPercent);
	}

	return iPureDam;
	//return iDam;
}

void NormalAttackAffect(LPCHARACTER pkAttacker, LPCHARACTER pkVictim)
{
#ifdef __SCP1453_EXTENSIONS__
	if ((pkVictim->GetRaceNum() != 6118) && pkAttacker->GetPoint(POINT_POISON_PCT) && !pkVictim->IsAffectFlag(AFF_POISON))
#else
	if (pkAttacker->GetPoint(POINT_POISON_PCT) && !pkVictim->IsAffectFlag(AFF_POISON))
#endif
	{
		if (number(1, 100) <= pkAttacker->GetPoint(POINT_POISON_PCT))
			pkVictim->AttackedByPoison(pkAttacker);
	}
#ifdef __WOLFMAN_CHARACTER__
#ifdef __SCP1453_EXTENSIONS__
	if ((pkVictim->IsPC()) && pkAttacker->GetPoint(POINT_BLEEDING_PCT) && !pkVictim->IsAffectFlag(AFF_BLEEDING))
#else
	if (pkAttacker->GetPoint(POINT_BLEEDING_PCT) && !pkVictim->IsAffectFlag(AFF_BLEEDING))
#endif
	{
		if (number(1, 100) <= pkAttacker->GetPoint(POINT_BLEEDING_PCT))
			pkVictim->AttackedByBleeding(pkAttacker);
	}
#endif
	int iStunDuration = 2;
	if (pkAttacker->IsPC() && !pkVictim->IsPC())
		iStunDuration = 4;

	AttackAffect(pkAttacker, pkVictim, POINT_STUN_PCT, IMMUNE_STUN, AFFECT_STUN, POINT_NONE, 0, AFF_STUN, iStunDuration, "STUN");
	AttackAffect(pkAttacker, pkVictim, POINT_SLOW_PCT, IMMUNE_SLOW, AFFECT_SLOW, POINT_MOV_SPEED, -30, AFF_SLOW, 20, "SLOW");
}

#ifdef __DAMAGE_LIMIT_REWORK__
long long battle_hit(LPCHARACTER pkAttacker, LPCHARACTER pkVictim, long long& iRetDam)
{
	//PROF_UNIT puHit("Hit");
	if (test_server)
		sys_log(0, "battle_hit : [%s] attack to [%s] : dam :%lld", pkAttacker->GetName(), pkVictim->GetName(), iRetDam);

	long long iDam = CalcMeleeDamage(pkAttacker, pkVictim);
#else
int battle_hit(LPCHARACTER pkAttacker, LPCHARACTER pkVictim, int& iRetDam)
{
	//PROF_UNIT puHit("Hit");
	if (test_server)
		sys_log(0, "battle_hit : [%s] attack to [%s] : dam :%d", pkAttacker->GetName(), pkVictim->GetName(), iRetDam);

	int iDam = CalcMeleeDamage(pkAttacker, pkVictim);
#endif
	if (iDam <= 0)
		return (BATTLE_DAMAGE);

	NormalAttackAffect(pkAttacker, pkVictim);

	//iDam = iDam * (100 - pkVictim->GetPoint(POINT_RESIST)) / 100;
	LPITEM pkWeapon = pkAttacker->GetWear(WEAR_WEAPON);

	if (pkWeapon)
		switch (pkWeapon->GetSubType())
		{
		case WEAPON_SWORD:
			iDam = iDam * (100 - pkVictim->GetPoint(POINT_RESIST_SWORD)) / 100;
			break;

		case WEAPON_TWO_HANDED:
			iDam = iDam * (100 - pkVictim->GetPoint(POINT_RESIST_TWOHAND)) / 100;
			break;

		case WEAPON_DAGGER:
			iDam = iDam * (100 - pkVictim->GetPoint(POINT_RESIST_DAGGER)) / 100;
			break;

		case WEAPON_BELL:
			iDam = iDam * (100 - pkVictim->GetPoint(POINT_RESIST_BELL)) / 100;
			break;

		case WEAPON_FAN:
			iDam = iDam * (100 - pkVictim->GetPoint(POINT_RESIST_FAN)) / 100;
			break;

		case WEAPON_BOW:
			iDam = iDam * (100 - MIN(91, pkVictim->GetPoint(POINT_RESIST_BOW))) / 100;
			break;
#ifdef __WOLFMAN_CHARACTER__
		case WEAPON_CLAW:
#if defined(__WOLFMAN_CHARACTER__) && defined(USE_ITEM_CLAW_AS_DAGGER)
			iDam = iDam * (100 - pkVictim->GetPoint(POINT_RESIST_DAGGER)) / 100;
#else
			iDam = iDam * (100 - pkVictim->GetPoint(POINT_RESIST_CLAW)) / 100;
#endif
			break;
#endif
		}

	float attMul = pkAttacker->GetAttMul();
	float tempIDam = iDam;
	iDam = attMul * tempIDam + 0.5f;

	iRetDam = iDam;

	if (pkVictim->Damage(pkAttacker, iDam, DAMAGE_TYPE_NORMAL))
		return (BATTLE_DEAD);

	return (BATTLE_DAMAGE);
}