#include "StdAfx.h"
#include "PythonPlayerSettingsModule.h"
#include "InstanceBase.h"
#include "../EffectLib/EffectManager.h"
#include "PythonSkill.h"
#include "../gamelib/RaceManager.h"
#include "PythonItem.h"
#include "PythonPlayer.h"
#include "../EterPack/EterPackManager.h"
#include <msl/utils.h>
#include "PythonNetworkStream.h"
#include "PythonBackground.h"
#include "PythonSystem.h"
#include <fmt/fmt.h>
#include <fmt/core.h>

CRaceMotionData& pkMotionManager = CRaceMotionData();
static const DWORD GUILD_SKILL_DRAGONBLOOD = pkMotionManager.NAME_SKILL + 101;
static const DWORD GUILD_SKILL_DRAGONBLESS = pkMotionManager.NAME_SKILL + 102;
static const DWORD GUILD_SKILL_BLESSARMOR = pkMotionManager.NAME_SKILL + 103;
static const DWORD GUILD_SKILL_SPPEDUP = pkMotionManager.NAME_SKILL + 104;
static const DWORD GUILD_SKILL_DRAGONWRATH = pkMotionManager.NAME_SKILL + 105;
static const DWORD GUILD_SKILL_MAGICUP = pkMotionManager.NAME_SKILL + 106;

static const DWORD HORSE_SKILL_WILDATTACK = pkMotionManager.NAME_SKILL + 121;
static const DWORD HORSE_SKILL_CHARGE = pkMotionManager.NAME_SKILL + 122;
static const DWORD HORSE_SKILL_SPLASH = pkMotionManager.NAME_SKILL + 123;

#ifdef USE_LOADING_DLG_OPTIMIZATION
void CPlayerSettingsModule::Load()
{
	LoadInitData();
	LoadGameEffect();
	LoadGameSound();
	std::string npcDir = LocaleService_GetLocalePath() + std::string("/npclist.txt");
	char npcPath[150];
	strcpy(npcPath, npcDir.c_str());
	LoadGameNPC(npcPath);

	LoadGameWarrior(CPlayerSettingsModule::RACE_WARRIOR_M, "d:/ymir work/pc/warrior/");
	LoadGameWarrior(CPlayerSettingsModule::RACE_WARRIOR_W, "d:/ymir work/pc2/warrior/");
	LoadGameAssassin(CPlayerSettingsModule::RACE_ASSASSIN_W, "d:/ymir work/pc/assassin/");
	LoadGameAssassin(CPlayerSettingsModule::RACE_ASSASSIN_M, "d:/ymir work/pc2/assassin/");
	LoadGameSura(CPlayerSettingsModule::RACE_SURA_M, "d:/ymir work/pc/sura/");
	LoadGameSura(CPlayerSettingsModule::RACE_SURA_W, "d:/ymir work/pc2/sura/");
	LoadGameShaman(CPlayerSettingsModule::RACE_SHAMAN_W, "d:/ymir work/pc/shaman/");
	LoadGameShaman(CPlayerSettingsModule::RACE_SHAMAN_M, "d:/ymir work/pc2/shaman/");
#ifdef ENABLE_WOLFMAN_CHARACTER
	LoadGameWolfman(CPlayerSettingsModule::RACE_WOLFMAN_M, "d:/ymir work/pc3/wolfman/");
#endif // ENABLE_WOLFMAN_CHARACTER
}
#endif // USE_LOADING_DLG_OPTIMIZATION

#ifdef USE_LOADING_DLG_OPTIMIZATION
bool CPlayerSettingsModule::LoadInitData()
#else
const bool CPlayerSettingsModule::LoadInitData()
#endif // USE_LOADING_DLG_OPTIMIZATION
{
	CInstanceBase& pkBase = CInstanceBase();
	static const std::vector<TEffect> m_vecEffectData =
	{
		{ pkBase.EFFECT_DUST, "", "d:/ymir work/effect/etc/dust/dust.mse" } ,
		{ pkBase.EFFECT_HORSE_DUST, "", "d:/ymir work/effect/etc/dust/running_dust.mse" },
		{ pkBase.EFFECT_HIT, "", "d:/ymir work/effect/hit/blow_1/blow_1_low.mse" },

		{ pkBase.EFFECT_HPUP_RED, "", "d:/ymir work/effect/etc/recuperation/drugup_red.mse" },
		{ pkBase.EFFECT_SPUP_BLUE, "", "d:/ymir work/effect/etc/recuperation/drugup_blue.mse" },
		{ pkBase.EFFECT_SPEEDUP_GREEN, "", "d:/ymir work/effect/etc/recuperation/drugup_green.mse" },
		{ pkBase.EFFECT_DXUP_PURPLE, "", "d:/ymir work/effect/etc/recuperation/drugup_purple.mse" },

		{ pkBase.EFFECT_AUTO_HPUP, "", "d:/ymir work/effect/etc/recuperation/autodrugup_red.mse" },
		{ pkBase.EFFECT_AUTO_SPUP, "", "d:/ymir work/effect/etc/recuperation/autodrugup_blue.mse" },

		{ pkBase.EFFECT_RAMADAN_RING_EQUIP, "", "d:/ymir work/effect/etc/buff/buff_item1.mse" },
		{ pkBase.EFFECT_HALLOWEEN_CANDY_EQUIP, "", "d:/ymir work/effect/etc/buff/buff_item2.mse" },
		{ pkBase.EFFECT_HAPPINESS_RING_EQUIP, "", "d:/ymir work/effect/etc/buff/buff_item3.mse" },
		{ pkBase.EFFECT_LOVE_PENDANT_EQUIP, "", "d:/ymir work/effect/etc/buff/buff_item4.mse" },

		{ pkBase.EFFECT_PENETRATE, "Bip01", "d:/ymir work/effect/hit/gwantong.mse" },
		{ pkBase.EFFECT_FIRECRACKER, "", "d:/ymir work/effect/etc/firecracker/newyear_firecracker.mse" },
		{ pkBase.EFFECT_SPIN_TOP, "", "d:/ymir work/effect/etc/firecracker/paing_i.mse" },
		{ pkBase.EFFECT_SELECT, "", "d:/ymir work/effect/etc/click/click_select.mse" },
		{ pkBase.EFFECT_TARGET, "", "d:/ymir work/effect/etc/click/click_glow_select.mse" },
		{ pkBase.EFFECT_STUN, "Bip01 Head", "d:/ymir work/effect/etc/stun/stun.mse" },
		{ pkBase.EFFECT_CRITICAL, "Bip01 R Hand", "d:/ymir work/effect/hit/critical.mse" },

		{ pkBase.EFFECT_DAMAGE_TARGET, "", "d:/ymir work/effect/affect/damagevalue/target.mse" },
		{ pkBase.EFFECT_DAMAGE_NOT_TARGET, "", "d:/ymir work/effect/affect/damagevalue/nontarget.mse" },
		{ pkBase.EFFECT_DAMAGE_SELFDAMAGE, "", "d:/ymir work/effect/affect/damagevalue/damage.mse" },
		{ pkBase.EFFECT_DAMAGE_SELFDAMAGE2, "", "d:/ymir work/effect/affect/damagevalue/damage_1.mse" },
		{ pkBase.EFFECT_DAMAGE_POISON, "", "d:/ymir work/effect/affect/damagevalue/poison.mse" },
		{ pkBase.EFFECT_DAMAGE_MISS, "", "d:/ymir work/effect/affect/damagevalue/miss.mse" },
		{ pkBase.EFFECT_DAMAGE_TARGETMISS, "", "d:/ymir work/effect/affect/damagevalue/target_miss.mse" },

		{ pkBase.EFFECT_PERCENT_DAMAGE1, "", "d:/ymir work/effect/hit/percent_damage1.mse" },
		{ pkBase.EFFECT_PERCENT_DAMAGE2, "", "d:/ymir work/effect/hit/percent_damage2.mse" },
		{ pkBase.EFFECT_PERCENT_DAMAGE3, "", "d:/ymir work/effect/hit/percent_damage3.mse" },

#ifdef ENABLE_PVP_EFFECT
		{ pkBase.EFFECT_PVP_WIN, "", "d:/ymir work/effect/etc/pvp/pvp_victory.mse" },
		{ pkBase.EFFECT_PVP_OPEN1, "", "d:/ymir work/effect/etc/pvp/pvp_open1.mse" },
		{ pkBase.EFFECT_PVP_OPEN2, "", "d:/ymir work/effect/etc/pvp/pvp_open2.mse" },
		{ pkBase.EFFECT_PVP_BEGIN1, "", "d:/ymir work/effect/etc/pvp/pvp_begin1.mse" },
		{ pkBase.EFFECT_PVP_BEGIN2, "", "d:/ymir work/effect/etc/pvp/pvp_begin2.mse" },
#endif

#ifdef ENABLE_SMITH_EFFECT
		{ pkBase.EFFECT_FR_SUCCESS, "", "d:/ymir work/effect/etc/smith/up_ok.mse" },
		{ pkBase.EFFECT_FR_FAIL, "", "d:/ymir work/effect/etc/smith/up_failed.mse" },
#endif

#ifdef ENABLE_AGGR_MONSTER_EFFECT
		{ pkBase.EFFECT_AGGREGATE_MONSTER, "", "d:/ymir work/effect/etc/buff/buff_item9.mse" },
#endif

#ifdef ENABLE_ACCE_SYSTEM
		{ pkBase.EFFECT_ACCE_SUCCEDED, "", "d:/ymir work/effect/etc/buff/buff_item6.mse" },
		{ pkBase.EFFECT_ACCE_EQUIP, "", "d:/ymir work/effect/etc/buff/buff_item7.mse" },
#endif
	};

	CPythonPlayer& rkPlayer = CPythonPlayer::Instance();
	rkPlayer.RegisterEffect(CPythonPlayer::EFFECT_PICK, "d:/ymir work/effect/etc/click/click.mse", true);

	std::string wPCMSM = fmt::format("{}/msm/warrior_m.msm", LocaleService_GetLocalePath());
	std::string wPC2MSM = fmt::format("{}/msm/warrior_w.msm", LocaleService_GetLocalePath());
	std::string aPCMSM = fmt::format("{}/msm/assassin_w.msm", LocaleService_GetLocalePath());
	std::string aPC2MSM = fmt::format("{}/msm/assassin_m.msm", LocaleService_GetLocalePath());
	std::string sPCMSM = fmt::format("{}/msm/sura_m.msm", LocaleService_GetLocalePath());
	std::string sPC2MSM = fmt::format("{}/msm/sura_w.msm", LocaleService_GetLocalePath());
	std::string bPCMSM = fmt::format("{}/msm/shaman_w.msm", LocaleService_GetLocalePath());
	std::string bPC2MSM = fmt::format("{}/msm/shaman_m.msm", LocaleService_GetLocalePath());
#ifdef ENABLE_WOLFMAN_CHARACTER
	std::string wolfmanMSM = fmt::format("{}/msm/wolfman_m.msm", LocaleService_GetLocalePath());
#endif // ENABLE_WOLFMAN_CHARACTER


	static std::vector<TRaceData> m_vecRaceData =
	{
		{ RACE_WARRIOR_M, wPCMSM, "d:/ymir work/pc/warrior/intro/" },
		{ RACE_WARRIOR_W, wPC2MSM, "d:/ymir work/pc2/warrior/intro/" },

		{ RACE_ASSASSIN_W, aPCMSM, "d:/ymir work/pc/assassin/intro/" },
		{ RACE_ASSASSIN_M, aPC2MSM, "d:/ymir work/pc2/assassin/intro/" },

		{ RACE_SURA_M, sPCMSM, "d:/ymir work/pc/sura/intro/" },
		{ RACE_SURA_W, sPC2MSM, "d:/ymir work/pc2/sura/intro/" },

		{ RACE_SHAMAN_W, bPCMSM, "d:/ymir work/pc/shaman/intro/" },
		{ RACE_SHAMAN_M, bPC2MSM, "d:/ymir work/pc2/shaman/intro/" },

#ifdef ENABLE_WOLFMAN_CHARACTER
		{ RACE_WOLFMAN_M, wolfmanMSM, "d:/ymir work/pc3/wolfman/intro/" },
#endif
	};


	std::vector<SMotion> m_vecMotion =
	{
		{ pkMotionManager.NAME_INTRO_WAIT, "wait.msa", 0 },
		{ pkMotionManager.NAME_INTRO_SELECTED, "selected.msa", 0 },
		{ pkMotionManager.NAME_INTRO_NOT_SELECTED, "not_selected.msa", 0 },
	};

	pkBase.SetDustGap(DUST_GAP);
	pkBase.SetHorseDustGap(HORSE_DUST_GAP);

	for (const auto& it : m_vecEffectData)
	{
		pkBase.RegisterEffect(it.uiType, it.stBone, it.stEffect, true);
	}

	CRaceManager& pkManager = CRaceManager::Instance();
	char szFileName[FILE_MAX_NUM];
	for (auto& it : m_vecRaceData)
	{
		pkManager.CreateRace(it.dwRace);
		pkManager.SelectRace(it.dwRace);

		CRaceData* pRaceData = pkManager.GetSelectedRaceDataPointer();
		if (!pRaceData)
			return false;

		if (!pRaceData->LoadRaceData(it.stMSM.c_str()))
		{
			return false;
		}

#ifdef ENABLE_CHARACTER_WINDOW_RENEWAL
		pRaceData->RegisterMotionMode(pkMotionManager.MODE_INTRO_GENERAL);
#else
		pRaceData->RegisterMotionMode(pkMotionManager.MODE_GENERAL);
#endif
		for (const auto& it2 : m_vecMotion)
		{
			snprintf(szFileName, sizeof(szFileName), "%s%s", it.stIntroMotion, it2.stName);
#ifdef ENABLE_CHARACTER_WINDOW_RENEWAL
			const CGraphicThing* pkMotionThing = pRaceData->RegisterMotionData(pkMotionManager.MODE_INTRO_GENERAL, it2.wMotionIndex, szFileName, it2.byPercentage);

			if (pkMotionThing)
				CResourceManager::Instance().LoadStaticCache(pkMotionThing->GetFileName());

#else
			pRaceData->RegisterMotionData(pkMotionManager.MODE_GENERAL, it2.wMotionIndex, szFileName, it2.byPercentage);
#endif
		}
	}

#ifdef ENABLE_MAP_ALGORITHM_RENEWAL
	CPythonBackground::Instance().LoadMapIndex();
#endif
	CPythonBackground::Instance().LoadAtlasInfo(true);
	return true;
}

#ifdef USE_LOADING_DLG_OPTIMIZATION
bool CPlayerSettingsModule::LoadGameEffect()
#else
const bool CPlayerSettingsModule::LoadGameEffect()
#endif // USE_LOADING_DLG_OPTIMIZATION
{
	CInstanceBase& pkBase = CInstanceBase();
	CFlyingManager& pkFly = CFlyingManager::Instance();

	char GM_MARK[FILE_MAX_NUM];
	snprintf(GM_MARK, sizeof(GM_MARK), "%s/effect/gm.mse", LocaleService_GetLocalePath());

	std::vector<TEffect> m_vecEffectData =
	{
		{ pkBase.EFFECT_SPAWN_APPEAR, "Bip01", "d:/ymir work/effect/etc/appear_die/monster_appear.mse" },
		{ pkBase.EFFECT_SPAWN_DISAPPEAR, "Bip01", "d:/ymir work/effect/etc/appear_die/monster_die.mse" },
		{ pkBase.EFFECT_FLAME_ATTACK, "equip_right_hand", "d:/ymir work/effect/hit/blow_flame/flame_3_weapon.mse" },
		{ pkBase.EFFECT_FLAME_HIT, "", "d:/ymir work/effect/hit/blow_flame/flame_3_blow.mse" },
		{ pkBase.EFFECT_FLAME_ATTACH, "", "d:/ymir work/effect/hit/blow_flame/flame_3_body.mse" },
		{ pkBase.EFFECT_ELECTRIC_ATTACK, "equip_right", "d:/ymir work/effect/hit/blow_electric/light_1_weapon.mse" },
		{ pkBase.EFFECT_ELECTRIC_HIT, "", "d:/ymir work/effect/hit/blow_electric/light_1_blow.mse" },
		{ pkBase.EFFECT_ELECTRIC_ATTACH, "", "d:/ymir work/effect/hit/blow_electric/light_1_body.mse" },

		{ pkBase.EFFECT_LEVELUP, "", "d:/ymir work/effect/etc/levelup_1/level_up.mse" },
		{ pkBase.EFFECT_SKILLUP, "", "d:/ymir work/effect/etc/skillup/skillup_1.mse" },

		{ pkBase.EFFECT_EMPIRE + pkBase.EMPIRE_A, "Bip01", "d:/ymir work/effect/etc/empire/empire_A.mse" },
		{ pkBase.EFFECT_EMPIRE + pkBase.EMPIRE_B, "Bip01", "d:/ymir work/effect/etc/empire/empire_B.mse" },
		{ pkBase.EFFECT_EMPIRE + pkBase.EMPIRE_C, "Bip01", "d:/ymir work/effect/etc/empire/empire_C.mse" },

		{ pkBase.EFFECT_AFFECT + pkBase.AFFECT_POISON, "Bip01", "d:/ymir work/effect/hit/blow_poison/poison_loop.mse" },
		{ pkBase.EFFECT_AFFECT + pkBase.AFFECT_SLOW, "", "d:/ymir work/effect/affect/slow.mse" },
		{ pkBase.EFFECT_AFFECT + pkBase.AFFECT_STUN, "Bip01 Head", "d:/ymir work/effect/etc/stun/stun_loop.mse" },
		{ pkBase.EFFECT_AFFECT + pkBase.AFFECT_DUNGEON_READY, "", "d:/ymir work/effect/etc/ready/ready.mse" },

		{ pkBase.EFFECT_AFFECT + pkBase.AFFECT_WAR_FLAG1, "", "d:/ymir work/effect/etc/guild_war_flag/flag_red.mse" },
		{ pkBase.EFFECT_AFFECT + pkBase.AFFECT_WAR_FLAG2, "", "d:/ymir work/effect/etc/guild_war_flag/flag_blue.mse" },
		{ pkBase.EFFECT_AFFECT + pkBase.AFFECT_WAR_FLAG3, "", "d:/ymir work/effect/etc/guild_war_flag/flag_yellow.mse" },

#ifdef ENABLE_WOLFMAN_CHARACTER
		{ pkBase.EFFECT_AFFECT + pkBase.AFFECT_BLEEDING, "Bip01", "d:/ymir work/effect/hit/blow_poison/bleeding_loop.mse" },
#endif

#ifdef ENABLE_DRAGON_BONE_EFFECT
		{ pkBase.EFFECT_AFFECT + pkBase.AFFECT_DBONE1, "Bip01 R Hand", "d:/ymir work/effect/etc/buff/dragonbone.mse" },
		{ pkBase.EFFECT_AFFECT + pkBase.AFFECT_DBONE2, "Bip01 L Hand", "d:/ymir work/effect/etc/buff/dragonbone2.mse" },
#endif

#ifdef ENABLE_DS_SET
		{ pkBase.EFFECT_AFFECT + pkBase.AFFECT_DS, "", "d:/ymir work/pc/assassin/effect/kwaegeom_loop.mse" },
#endif

#ifdef ENABLE_PB2_NEW_TAG_SYSTEM
		{ pkBase.EFFECT_AFFECT + pkBase.AFFECT_PB2_TAG_VIP, "Bip01", "d:/ymir work/effect/vip.mse" },
		{ pkBase.EFFECT_AFFECT + pkBase.AFFECT_PB2_TAG_KING, "Bip01", "d:/ymir work/effect/kral.mse" },
		{ pkBase.EFFECT_AFFECT + pkBase.AFFECT_PB2_TAG_QUEEN, "Bip01", "d:/ymir work/effect/kralice.mse" },
		{ pkBase.EFFECT_AFFECT + pkBase.AFFECT_PB2_TAG_MAFIA, "Bip01", "d:/ymir work/effect/mafya.mse" },
		{ pkBase.EFFECT_AFFECT + pkBase.AFFECT_PB2_TAG_BARON, "Bip01", "d:/ymir work/effect/baron.mse" },
		{ pkBase.EFFECT_AFFECT + pkBase.AFFECT_PB2_TAG_STONE_KING, "Bip01", "d:/ymir work/effect/metinkrali.mse" },
		{ pkBase.EFFECT_AFFECT + pkBase.AFFECT_PB2_TAG_BOSS_KING, "Bip01", "d:/ymir work/effect/patronkrali.mse" },
#endif // ENABLE_PB2_NEW_TAG_SYSTEM

		{ pkBase.EFFECT_AFFECT + pkBase.AFF_OTOAV, "Bip01", "d:/ymir work/effect/otoav.mse" },

		{ pkBase.EFFECT_REFINED + pkBase.EFFECT_SWORD_REFINED7, "PART_WEAPON", "D:/ymir work/pc/common/effect/sword/sword_7.mse" },
		{ pkBase.EFFECT_REFINED + pkBase.EFFECT_SWORD_REFINED8, "PART_WEAPON", "D:/ymir work/pc/common/effect/sword/sword_8.mse" },
		{ pkBase.EFFECT_REFINED + pkBase.EFFECT_SWORD_REFINED9, "PART_WEAPON", "D:/ymir work/pc/common/effect/sword/sword_9.mse" },

		{ pkBase.EFFECT_REFINED + pkBase.EFFECT_BOW_REFINED7, "PART_WEAPON_LEFT", "D:/ymir work/pc/common/effect/sword/sword_7_b.mse" },
		{ pkBase.EFFECT_REFINED + pkBase.EFFECT_BOW_REFINED8, "PART_WEAPON_LEFT", "D:/ymir work/pc/common/effect/sword/sword_8_b.mse" },
		{ pkBase.EFFECT_REFINED + pkBase.EFFECT_BOW_REFINED9, "PART_WEAPON_LEFT", "D:/ymir work/pc/common/effect/sword/sword_9_b.mse" },

		{ pkBase.EFFECT_REFINED + pkBase.EFFECT_FANBELL_REFINED7, "PART_WEAPON", "D:/ymir work/pc/common/effect/sword/sword_7_f.mse" },
		{ pkBase.EFFECT_REFINED + pkBase.EFFECT_FANBELL_REFINED8, "PART_WEAPON", "D:/ymir work/pc/common/effect/sword/sword_8_f.mse" },
		{ pkBase.EFFECT_REFINED + pkBase.EFFECT_FANBELL_REFINED9, "PART_WEAPON", "D:/ymir work/pc/common/effect/sword/sword_9_f.mse" },

		{ pkBase.EFFECT_REFINED + pkBase.EFFECT_SMALLSWORD_REFINED7, "PART_WEAPON", "D:/ymir work/pc/common/effect/sword/sword_7_s.mse" },
		{ pkBase.EFFECT_REFINED + pkBase.EFFECT_SMALLSWORD_REFINED8, "PART_WEAPON", "D:/ymir work/pc/common/effect/sword/sword_8_s.mse" },
		{ pkBase.EFFECT_REFINED + pkBase.EFFECT_SMALLSWORD_REFINED9, "PART_WEAPON", "D:/ymir work/pc/common/effect/sword/sword_9_s.mse" },

		{ pkBase.EFFECT_REFINED + pkBase.EFFECT_SMALLSWORD_REFINED7_LEFT, "PART_WEAPON_LEFT", "D:/ymir work/pc/common/effect/sword/sword_7_s.mse" },
		{ pkBase.EFFECT_REFINED + pkBase.EFFECT_SMALLSWORD_REFINED8_LEFT, "PART_WEAPON_LEFT", "D:/ymir work/pc/common/effect/sword/sword_8_s.mse" },
		{ pkBase.EFFECT_REFINED + pkBase.EFFECT_SMALLSWORD_REFINED9_LEFT, "PART_WEAPON_LEFT", "D:/ymir work/pc/common/effect/sword/sword_9_s.mse" },

		{ pkBase.EFFECT_REFINED + pkBase.EFFECT_BODYARMOR_REFINED7, "Bip01", "D:/ymir work/pc/common/effect/armor/armor_7.mse" },
		{ pkBase.EFFECT_REFINED + pkBase.EFFECT_BODYARMOR_REFINED8, "Bip01", "D:/ymir work/pc/common/effect/armor/armor_8.mse" },
		{ pkBase.EFFECT_REFINED + pkBase.EFFECT_BODYARMOR_REFINED9, "Bip01", "D:/ymir work/pc/common/effect/armor/armor_9.mse" },

		{ pkBase.EFFECT_REFINED + pkBase.EFFECT_BODYARMOR_SPECIAL, "Bip01", "D:/ymir work/pc/common/effect/armor/armor-4-2-1.mse" },
		{ pkBase.EFFECT_REFINED + pkBase.EFFECT_BODYARMOR_SPECIAL2, "Bip01", "D:/ymir work/pc/common/effect/armor/armor-4-2-2.mse" },

		{ pkBase.EFFECT_REFINED + pkBase.EFFECT_BODYARMOR_SPECIAL3, "Bip01", "D:/ymir work/pc/common/effect/armor/armor-5-1.mse" },

#ifdef ENABLE_WOLFMAN_CHARACTER
		{ pkBase.EFFECT_REFINED + pkBase.EFFECT_CLAW_REFINED7, "PART_WEAPON", "D:/ymir work/pc/common/effect/sword/sword_7_w.mse" },
		{ pkBase.EFFECT_REFINED + pkBase.EFFECT_CLAW_REFINED8, "PART_WEAPON", "D:/ymir work/pc/common/effect/sword/sword_8_w.mse" },
		{ pkBase.EFFECT_REFINED + pkBase.EFFECT_CLAW_REFINED9, "PART_WEAPON", "D:/ymir work/pc/common/effect/sword/sword_9_w.mse" },

		{ pkBase.EFFECT_REFINED + pkBase.EFFECT_CLAW_REFINED7_LEFT, "PART_WEAPON_LEFT", "D:/ymir work/pc/common/effect/sword/sword_7_w.mse" },
		{ pkBase.EFFECT_REFINED + pkBase.EFFECT_CLAW_REFINED8_LEFT, "PART_WEAPON_LEFT", "D:/ymir work/pc/common/effect/sword/sword_8_w.mse" },
		{ pkBase.EFFECT_REFINED + pkBase.EFFECT_CLAW_REFINED9_LEFT, "PART_WEAPON_LEFT", "D:/ymir work/pc/common/effect/sword/sword_9_w.mse" },
#endif

#ifdef ENABLE_ITEM_EVOLUTION_SYSTEM
		{ pkBase.EFFECT_REFINED + pkBase.EFFECT_SWORD_RARITY_FIRE, "PART_WEAPON", "d:/ymir work/pc/common/effect/sword/element02_fire.mse" },
		{ pkBase.EFFECT_REFINED + pkBase.EFFECT_SWORD_RARITY_ICE, "PART_WEAPON", "D:/ymir work/pc/common/effect/sword/element03_ice.mse" },
		{ pkBase.EFFECT_REFINED + pkBase.EFFECT_SWORD_RARITY_EARTH, "PART_WEAPON", "D:/ymir work/pc/common/effect/sword/element05_daeji.mse" },
		{ pkBase.EFFECT_REFINED + pkBase.EFFECT_SWORD_RARITY_WIND, "PART_WEAPON", "D:/ymir work/pc/common/effect/sword/element04_wind.mse" },
		{ pkBase.EFFECT_REFINED + pkBase.EFFECT_SWORD_RARITY_DARK, "PART_WEAPON", "D:/ymir work/pc/common/effect/sword/element06_dark.mse" },
		{ pkBase.EFFECT_REFINED + pkBase.EFFECT_SWORD_RARITY_ELEC, "PART_WEAPON", "D:/ymir work/pc/common/effect/sword/element01_electric.mse" },

		{ pkBase.EFFECT_REFINED + pkBase.EFFECT_BOW_RARITY_FIRE, "PART_WEAPON_LEFT", "D:/ymir work/pc/common/effect/sword/element02_fire_b.mse" },
		{ pkBase.EFFECT_REFINED + pkBase.EFFECT_BOW_RARITY_ICE, "PART_WEAPON_LEFT", "D:/ymir work/pc/common/effect/sword/element03_ice_b.mse" },
		{ pkBase.EFFECT_REFINED + pkBase.EFFECT_BOW_RARITY_EARTH, "PART_WEAPON_LEFT", "D:/ymir work/pc/common/effect/sword/element05_daeji_b.mse" },
		{ pkBase.EFFECT_REFINED + pkBase.EFFECT_BOW_RARITY_WIND, "PART_WEAPON_LEFT", "D:/ymir work/pc/common/effect/sword/element04_wind_b.mse" },
		{ pkBase.EFFECT_REFINED + pkBase.EFFECT_BOW_RARITY_DARK, "PART_WEAPON_LEFT", "D:/ymir work/pc/common/effect/sword/element06_dark_b.mse" },
		{ pkBase.EFFECT_REFINED + pkBase.EFFECT_BOW_RARITY_ELEC, "PART_WEAPON_LEFT", "D:/ymir work/pc/common/effect/sword/element01_electric_b.mse" },

		{ pkBase.EFFECT_REFINED + pkBase.EFFECT_FANBELL_RARITY_FIRE, "PART_WEAPON", "D:/ymir work/pc/common/effect/sword/element02_fire_f.mse" },
		{ pkBase.EFFECT_REFINED + pkBase.EFFECT_FANBELL_RARITY_ICE, "PART_WEAPON", "D:/ymir work/pc/common/effect/sword/element03_ice_f.mse" },
		{ pkBase.EFFECT_REFINED + pkBase.EFFECT_FANBELL_RARITY_EARTH, "PART_WEAPON", "D:/ymir work/pc/common/effect/sword/element05_daeji_f.mse" },
		{ pkBase.EFFECT_REFINED + pkBase.EFFECT_FANBELL_RARITY_WIND, "PART_WEAPON", "D:/ymir work/pc/common/effect/sword/element04_wind_f.mse" },
		{ pkBase.EFFECT_REFINED + pkBase.EFFECT_FANBELL_RARITY_DARK, "PART_WEAPON", "D:/ymir work/pc/common/effect/sword/element06_dark_f.mse" },
		{ pkBase.EFFECT_REFINED + pkBase.EFFECT_FANBELL_RARITY_ELEC, "PART_WEAPON", "D:/ymir work/pc/common/effect/sword/element01_electric_f.mse" },

		{ pkBase.EFFECT_REFINED + pkBase.EFFECT_SMALLSWORD_RARITY_FIRE, "PART_WEAPON", "D:/ymir work/pc/common/effect/sword/element02_fire_s.mse" },
		{ pkBase.EFFECT_REFINED + pkBase.EFFECT_SMALLSWORD_RARITY_ICE, "PART_WEAPON", "D:/ymir work/pc/common/effect/sword/element03_ice_s.mse" },
		{ pkBase.EFFECT_REFINED + pkBase.EFFECT_SMALLSWORD_RARITY_EARTH, "PART_WEAPON", "D:/ymir work/pc/common/effect/sword/element05_daeji_s.mse" },
		{ pkBase.EFFECT_REFINED + pkBase.EFFECT_SMALLSWORD_RARITY_WIND, "PART_WEAPON", "D:/ymir work/pc/common/effect/sword/element04_wind_s.mse" },
		{ pkBase.EFFECT_REFINED + pkBase.EFFECT_SMALLSWORD_RARITY_DARK, "PART_WEAPON", "D:/ymir work/pc/common/effect/sword/element06_dark_s.mse" },
		{ pkBase.EFFECT_REFINED + pkBase.EFFECT_SMALLSWORD_RARITY_ELEC, "PART_WEAPON", "D:/ymir work/pc/common/effect/sword/element01_electric_s.mse" },

		{ pkBase.EFFECT_REFINED + pkBase.EFFECT_SMALLSWORD_RARITY_LEFT_FIRE, "PART_WEAPON_LEFT", "D:/ymir work/pc/common/effect/sword/element02_fire_s.mse" },
		{ pkBase.EFFECT_REFINED + pkBase.EFFECT_SMALLSWORD_RARITY_LEFT_ICE, "PART_WEAPON_LEFT", "D:/ymir work/pc/common/effect/sword/element03_ice_s.mse" },
		{ pkBase.EFFECT_REFINED + pkBase.EFFECT_SMALLSWORD_RARITY_LEFT_EARTH, "PART_WEAPON_LEFT", "D:/ymir work/pc/common/effect/sword/element05_daeji_s.mse" },
		{ pkBase.EFFECT_REFINED + pkBase.EFFECT_SMALLSWORD_RARITY_LEFT_WIND, "PART_WEAPON_LEFT", "D:/ymir work/pc/common/effect/sword/element04_wind_s.mse" },
		{ pkBase.EFFECT_REFINED + pkBase.EFFECT_SMALLSWORD_RARITY_LEFT_DARK, "PART_WEAPON_LEFT", "D:/ymir work/pc/common/effect/sword/element06_dark_s.mse" },
		{ pkBase.EFFECT_REFINED + pkBase.EFFECT_SMALLSWORD_RARITY_LEFT_ELEC, "PART_WEAPON_LEFT", "D:/ymir work/pc/common/effect/sword/element01_electric_s.mse" },

#ifdef ENABLE_WOLFMAN_CHARACTER
		{ pkBase.EFFECT_REFINED + pkBase.EFFECT_CLAW_RARITY_FIRE, "PART_WEAPON", "D:/ymir work/pc/common/effect/sword/element02_fire_w.mse" },
		{ pkBase.EFFECT_REFINED + pkBase.EFFECT_CLAW_RARITY_ICE, "PART_WEAPON", "D:/ymir work/pc/common/effect/sword/element03_ice_w.mse" },
		{ pkBase.EFFECT_REFINED + pkBase.EFFECT_CLAW_RARITY_EARTH, "PART_WEAPON", "D:/ymir work/pc/common/effect/sword/element05_daeji_w.mse" },
		{ pkBase.EFFECT_REFINED + pkBase.EFFECT_CLAW_RARITY_WIND, "PART_WEAPON", "D:/ymir work/pc/common/effect/sword/element04_wind_w.mse" },
		{ pkBase.EFFECT_REFINED + pkBase.EFFECT_CLAW_RARITY_DARK, "PART_WEAPON", "D:/ymir work/pc/common/effect/sword/element06_dark_w.mse" },
		{ pkBase.EFFECT_REFINED + pkBase.EFFECT_CLAW_RARITY_ELEC, "PART_WEAPON", "D:/ymir work/pc/common/effect/sword/element01_electric_w.mse" },

		{ pkBase.EFFECT_REFINED + pkBase.EFFECT_CLAW_RARITY_LEFT_FIRE, "PART_WEAPON_LEFT", "D:/ymir work/pc/common/effect/sword/element02_fire_w.mse" },
		{ pkBase.EFFECT_REFINED + pkBase.EFFECT_CLAW_RARITY_LEFT_ICE, "PART_WEAPON_LEFT", "D:/ymir work/pc/common/effect/sword/element03_ice_w.mse" },
		{ pkBase.EFFECT_REFINED + pkBase.EFFECT_CLAW_RARITY_LEFT_EARTH, "PART_WEAPON_LEFT", "D:/ymir work/pc/common/effect/sword/element05_daeji_w.mse" },
		{ pkBase.EFFECT_REFINED + pkBase.EFFECT_CLAW_RARITY_LEFT_WIND, "PART_WEAPON_LEFT", "D:/ymir work/pc/common/effect/sword/element04_wind_w.mse" },
		{ pkBase.EFFECT_REFINED + pkBase.EFFECT_CLAW_RARITY_LEFT_DARK, "PART_WEAPON_LEFT", "D:/ymir work/pc/common/effect/sword/element06_dark_w.mse" },
		{ pkBase.EFFECT_REFINED + pkBase.EFFECT_CLAW_RARITY_LEFT_ELEC, "PART_WEAPON_LEFT", "D:/ymir work/pc/common/effect/sword/element01_electric_w.mse" },
#endif
#ifdef ENABLE_ARMOR_EVOLUTION_SYSTEM
		{ pkBase.EFFECT_REFINED + pkBase.EFFECT_ARMOR_RARITY_FIRE, "Bip01", "D:/ymir work/effect/element_armor/ridack_armorelm2.mse" },
		{ pkBase.EFFECT_REFINED + pkBase.EFFECT_ARMOR_RARITY_ICE, "Bip01", "D:/ymir work/effect/element_armor/ridack_armorelm3.mse" },
		{ pkBase.EFFECT_REFINED + pkBase.EFFECT_ARMOR_RARITY_EARTH, "Bip01", "D:/ymir work/effect/element_armor/ridack_armorelm4.mse" },
		{ pkBase.EFFECT_REFINED + pkBase.EFFECT_ARMOR_RARITY_WIND, "Bip01", "D:/ymir work/effect/element_armor/ridack_armorelm5.mse" },
		{ pkBase.EFFECT_REFINED + pkBase.EFFECT_ARMOR_RARITY_DARK, "Bip01", "D:/ymir work/effect/element_armor/ridack_armorelm6.mse" },
		{ pkBase.EFFECT_REFINED + pkBase.EFFECT_ARMOR_RARITY_ELEC, "Bip01", "D:/ymir work/effect/element_armor/ridack_armorelm1.mse" },
#endif
		{ pkBase.EFFECT_REFINED + pkBase.EFFECT_SWORD_RARITY_ALL, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_galaxy/ridack_sword.mse"},
		{ pkBase.EFFECT_REFINED + pkBase.EFFECT_BOW_RARITY_ALL, "PART_WEAPON_LEFT", "D:/ymir work/effect/shining_system/ridack_galaxy/ridack_bow.mse"},
		{ pkBase.EFFECT_REFINED + pkBase.EFFECT_FAN_RARITY_ALL, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_galaxy/ridack_fan.mse"},
		{ pkBase.EFFECT_REFINED + pkBase.EFFECT_OTHER_RARITY_ALL, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_galaxy/ridack_bellknife.mse"},
		{ pkBase.EFFECT_REFINED + pkBase.EFFECT_OTHER_RARITY_ALL_LEFT, "PART_WEAPON_LEFT", "D:/ymir work/effect/shining_system/ridack_galaxy/ridack_bellknife.mse"},
		{ pkBase.EFFECT_REFINED + pkBase.EFFECT_ARMOR_RARITY_ALL, "Bip01", "D:/ymir work/effect/shining_system/ridack_galaxy/ridack_costumeffect.mse"},
#endif

#ifdef ENABLE_ACCE_SYSTEM
		{ pkBase.EFFECT_REFINED + pkBase.EFFECT_ACCE, "Bip01", "d:/ymir work/pc/common/effect/armor/acc_01.mse" },
#endif
#ifdef ENABLE_SHINING_ITEM_SYSTEM
		//Kilic 0-49
		{ pkBase.EFFECT_SHINING_WEAPON + 0, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_demonblue/ridack_bluesword2.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 1, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_demongreen/ridack_greensword2.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 2, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_demonred/ridack_redsword2.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 3, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_effect_v7/ridack_weapon.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 4, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_smith/sword/sword_9.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 5, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_set_v1/ridack_effect_blue/ridack_bleusword.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 6, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_set_v1/ridack_effect_brown/ridack_brownsword.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 7, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_set_v1/ridack_effect_green/ridack_greensword.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 8, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_set_v1/ridack_effect_grey/ridack_greysword.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 9, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_set_v1/ridack_effect_orange/ridack_orangesword.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 10, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_set_v1/ridack_effect_pink/ridack_pinksword.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 11, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_set_v1/ridack_effect_purple/ridack_purplesword.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 12, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_set_v1/ridack_effect_red/ridack_redsword.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 13, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_set_v1/ridack_effect_turquoise/ridack_turquoisesword.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 14, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_set_v1/ridack_effect_yellow/ridack_yellowsword.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 15, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_set_v3/ridackeffectblackwhite/ridack_firebw_1h.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 16, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_set_v3/ridackeffectblue/ridack_fireblu_1h.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 17, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_set_v3/ridackeffectbrown/ridack_firebrwn_1h.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 18, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_set_v3/ridackeffectgreen/ridack_firegrn_1h.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 19, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_set_v3/ridackeffectorange/ridack_fireor_1h.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 20, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_set_v3/ridackeffectpink/ridack_firepnk_1h.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 21, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_set_v3/ridackeffectpurple/ridack_firepurp_1h.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 22, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_set_v3/ridackeffectred/ridack_firered_1h.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 23, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_set_v3/ridackeffectturquoise/ridack_firetur_1h.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 24, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_set_v3/ridackeffectyellow/ridack_fireyel_1h.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 25, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_set_v4/ridack_blue_fire/ridack_weapon_blue.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 26, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_set_v4/ridack_bluewhite_fire/ridack_weapon_bw.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 27, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_set_v4/ridack_brown_fire/ridack_weapon_brwn.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 28, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_set_v4/ridack_green_fire/ridack_weapon_green.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 29, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_set_v4/ridack_orange_fire/ridack_weapon_fire.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 30, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_set_v4/ridack_pink_fire/ridack_weapon_pink.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 31, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_set_v4/ridack_purple_fire/ridack_weapon_purp.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 32, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_set_v4/ridack_red_fire/ridack_weapon_red.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 33, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_set_v4/ridack_turquoise_fire/ridack_weapon_turq.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 34, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_set_v4/ridack_yellow_fire/ridack_weapon_yellw.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 35, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_fire/ridack_weapon.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 36, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_galaxy/ridack_sword.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 37, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_galaxy_old/ridack_sword.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 38, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_ghost/ridack_weapon.mse" },


		//Yay 50-99
		{ pkBase.EFFECT_SHINING_WEAPON + 50, "PART_WEAPON_LEFT", "D:/ymir work/effect/shining_system/ridack_demonblue/ridack_bluesword2_b.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 51, "PART_WEAPON_LEFT", "D:/ymir work/effect/shining_system/ridack_demongreen/ridack_greensword2_b.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 52, "PART_WEAPON_LEFT", "D:/ymir work/effect/shining_system/ridack_demonred/ridack_redsword2_b.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 53, "PART_WEAPON_LEFT", "D:/ymir work/effect/shining_system/ridack_effect_v7/ridack_weapon_b.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 54, "PART_WEAPON_LEFT", "D:/ymir work/effect/shining_system/ridack_smith/sword/sword_9_b.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 55, "PART_WEAPON_LEFT", "D:/ymir work/effect/shining_system/ridack_set_v1/ridack_effect_blue/ridack_bleusword_b.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 56, "PART_WEAPON_LEFT", "D:/ymir work/effect/shining_system/ridack_set_v1/ridack_effect_brown/ridack_brownsword_b.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 57, "PART_WEAPON_LEFT", "D:/ymir work/effect/shining_system/ridack_set_v1/ridack_effect_green/ridack_greensword_b.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 58, "PART_WEAPON_LEFT", "D:/ymir work/effect/shining_system/ridack_set_v1/ridack_effect_grey/ridack_greysword_b.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 59, "PART_WEAPON_LEFT", "D:/ymir work/effect/shining_system/ridack_set_v1/ridack_effect_orange/ridack_orangesword_b.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 60, "PART_WEAPON_LEFT", "D:/ymir work/effect/shining_system/ridack_set_v1/ridack_effect_pink/ridack_pinksword_b.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 61, "PART_WEAPON_LEFT", "D:/ymir work/effect/shining_system/ridack_set_v1/ridack_effect_purple/ridack_purplesword_b.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 62, "PART_WEAPON_LEFT", "D:/ymir work/effect/shining_system/ridack_set_v1/ridack_effect_red/ridack_redsword_b.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 63, "PART_WEAPON_LEFT", "D:/ymir work/effect/shining_system/ridack_set_v1/ridack_effect_turquoise/ridack_turquoisesword_b.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 64, "PART_WEAPON_LEFT", "D:/ymir work/effect/shining_system/ridack_set_v1/ridack_effect_yellow/ridack_yellowsword_b.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 65, "PART_WEAPON_LEFT", "D:/ymir work/effect/shining_system/ridack_set_v3/ridackeffectblackwhite/ridack_firebw_b.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 66, "PART_WEAPON_LEFT", "D:/ymir work/effect/shining_system/ridack_set_v3/ridackeffectblue/ridack_fireblu_b.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 67, "PART_WEAPON_LEFT", "D:/ymir work/effect/shining_system/ridack_set_v3/ridackeffectbrown/ridack_firebrwn_b.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 68, "PART_WEAPON_LEFT", "D:/ymir work/effect/shining_system/ridack_set_v3/ridackeffectgreen/ridack_firegrn_b.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 69, "PART_WEAPON_LEFT", "D:/ymir work/effect/shining_system/ridack_set_v3/ridackeffectorange/ridack_fireor_b.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 70, "PART_WEAPON_LEFT", "D:/ymir work/effect/shining_system/ridack_set_v3/ridackeffectpink/ridack_firepnk_b.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 71, "PART_WEAPON_LEFT", "D:/ymir work/effect/shining_system/ridack_set_v3/ridackeffectpurple/ridack_firepurp_b.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 72, "PART_WEAPON_LEFT", "D:/ymir work/effect/shining_system/ridack_set_v3/ridackeffectred/ridack_firered_b.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 73, "PART_WEAPON_LEFT", "D:/ymir work/effect/shining_system/ridack_set_v3/ridackeffectturquoise/ridack_firetur_b.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 74, "PART_WEAPON_LEFT", "D:/ymir work/effect/shining_system/ridack_set_v3/ridackeffectyellow/ridack_fireyel_b.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 75, "PART_WEAPON_LEFT", "D:/ymir work/effect/shining_system/ridack_set_v4/ridack_blue_fire/ridack_weapon_blue_b.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 76, "PART_WEAPON_LEFT", "D:/ymir work/effect/shining_system/ridack_set_v4/ridack_bluewhite_fire/ridack_weapon_bw_b.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 77, "PART_WEAPON_LEFT", "D:/ymir work/effect/shining_system/ridack_set_v4/ridack_brown_fire/ridack_weapon_brwn_b.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 78, "PART_WEAPON_LEFT", "D:/ymir work/effect/shining_system/ridack_set_v4/ridack_green_fire/ridack_weapon_green_b.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 79, "PART_WEAPON_LEFT", "D:/ymir work/effect/shining_system/ridack_set_v4/ridack_orange_fire/ridack_weapon_fire_b.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 80, "PART_WEAPON_LEFT", "D:/ymir work/effect/shining_system/ridack_set_v4/ridack_pink_fire/ridack_weapon_pink_b.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 81, "PART_WEAPON_LEFT", "D:/ymir work/effect/shining_system/ridack_set_v4/ridack_purple_fire/ridack_weapon_purp_b.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 82, "PART_WEAPON_LEFT", "D:/ymir work/effect/shining_system/ridack_set_v4/ridack_red_fire/ridack_weapon_red_b.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 83, "PART_WEAPON_LEFT", "D:/ymir work/effect/shining_system/ridack_set_v4/ridack_turquoise_fire/ridack_weapon_turq_b.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 84, "PART_WEAPON_LEFT", "D:/ymir work/effect/shining_system/ridack_set_v4/ridack_yellow_fire/ridack_weapon_yellw_b.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 85, "PART_WEAPON_LEFT", "D:/ymir work/effect/shining_system/ridack_fire/ridack_weapon_b.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 86, "PART_WEAPON_LEFT", "D:/ymir work/effect/shining_system/ridack_galaxy/ridack_bow.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 87, "PART_WEAPON_LEFT", "D:/ymir work/effect/shining_system/ridack_galaxy_old/ridack_bow.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 88, "PART_WEAPON_LEFT", "D:/ymir work/effect/shining_system/ridack_ghost/ridack_weapon_b.mse" },


		//Yelpaze/Can 100-149
		{ pkBase.EFFECT_SHINING_WEAPON + 100, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_demonblue/ridack_bluesword2_f.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 101, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_demongreen/ridack_greensword2_f.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 102, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_demonred/ridack_redsword2_f.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 103, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_effect_v7/ridack_weapon_f.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 104, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_smith/sword/sword_9_f.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 105, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_set_v1/ridack_effect_blue/ridack_bleusword_f.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 106, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_set_v1/ridack_effect_brown/ridack_brownsword_f.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 107, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_set_v1/ridack_effect_green/ridack_greensword_f.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 108, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_set_v1/ridack_effect_grey/ridack_greysword_f.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 109, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_set_v1/ridack_effect_orange/ridack_orangesword_f.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 110, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_set_v1/ridack_effect_pink/ridack_pinksword_f.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 111, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_set_v1/ridack_effect_purple/ridack_purplesword_f.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 112, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_set_v1/ridack_effect_red/ridack_redsword_f.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 113, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_set_v1/ridack_effect_turquoise/ridack_turquoisesword_f.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 114, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_set_v1/ridack_effect_yellow/ridack_yellowsword_f.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 115, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_set_v3/ridackeffectblackwhite/ridack_firebw_f.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 116, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_set_v3/ridackeffectblue/ridack_fireblu_f.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 117, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_set_v3/ridackeffectbrown/ridack_firebrwn_f.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 118, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_set_v3/ridackeffectgreen/ridack_firegrn_f.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 119, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_set_v3/ridackeffectorange/ridack_fireor_f.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 120, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_set_v3/ridackeffectpink/ridack_firepnk_f.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 121, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_set_v3/ridackeffectpurple/ridack_firepurp_f.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 122, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_set_v3/ridackeffectred/ridack_firered_f.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 123, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_set_v3/ridackeffectturquoise/ridack_firetur_f.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 124, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_set_v3/ridackeffectyellow/ridack_fireyel_f.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 125, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_set_v4/ridack_blue_fire/ridack_weapon_blue_f.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 126, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_set_v4/ridack_bluewhite_fire/ridack_weapon_bw_f.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 127, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_set_v4/ridack_brown_fire/ridack_weapon_brwn_f.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 128, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_set_v4/ridack_green_fire/ridack_weapon_green_f.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 129, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_set_v4/ridack_orange_fire/ridack_weapon_fire_f.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 130, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_set_v4/ridack_pink_fire/ridack_weapon_pink_f.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 131, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_set_v4/ridack_purple_fire/ridack_weapon_purp_f.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 132, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_set_v4/ridack_red_fire/ridack_weapon_red_f.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 133, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_set_v4/ridack_turquoise_fire/ridack_weapon_turq_f.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 134, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_set_v4/ridack_yellow_fire/ridack_weapon_yellw_f.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 135, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_fire/ridack_weapon_f.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 136, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_galaxy/ridack_fan.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 137, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_galaxy_old/ridack_fan.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 138, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_ghost/ridack_weapon_f.mse" },

		//Hancer (sag 150-199)
		{ pkBase.EFFECT_SHINING_WEAPON + 150, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_demonblue/ridack_bluesword2_s.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 151, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_demongreen/ridack_greensword2_s.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 152, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_demonred/ridack_redsword2_s.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 153, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_effect_v7/ridack_weapon_s.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 154, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_smith/sword/sword_9_s.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 155, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_set_v1/ridack_effect_blue/ridack_bleusword_s.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 156, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_set_v1/ridack_effect_brown/ridack_brownsword_s.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 157, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_set_v1/ridack_effect_green/ridack_greensword_s.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 158, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_set_v1/ridack_effect_grey/ridack_greysword_s.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 159, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_set_v1/ridack_effect_orange/ridack_orangesword_s.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 160, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_set_v1/ridack_effect_pink/ridack_pinksword_s.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 161, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_set_v1/ridack_effect_purple/ridack_purplesword_s.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 162, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_set_v1/ridack_effect_red/ridack_redsword_s.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 163, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_set_v1/ridack_effect_turquoise/ridack_turquoisesword_s.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 164, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_set_v1/ridack_effect_yellow/ridack_yellowsword_s.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 165, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_set_v3/ridackeffectblackwhite/ridack_firebw_s.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 166, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_set_v3/ridackeffectblue/ridack_fireblu_s.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 167, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_set_v3/ridackeffectbrown/ridack_firebrwn_s.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 168, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_set_v3/ridackeffectgreen/ridack_firegrn_s.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 169, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_set_v3/ridackeffectorange/ridack_fireor_s.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 170, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_set_v3/ridackeffectpink/ridack_firepnk_s.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 171, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_set_v3/ridackeffectpurple/ridack_firepurp_s.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 172, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_set_v3/ridackeffectred/ridack_firered_s.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 173, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_set_v3/ridackeffectturquoise/ridack_firetur_s.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 174, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_set_v3/ridackeffectyellow/ridack_fireyel_s.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 175, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_set_v4/ridack_blue_fire/ridack_weapon_blue_s.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 176, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_set_v4/ridack_bluewhite_fire/ridack_weapon_bw_s.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 177, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_set_v4/ridack_brown_fire/ridack_weapon_brwn_s.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 178, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_set_v4/ridack_green_fire/ridack_weapon_green_s.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 179, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_set_v4/ridack_orange_fire/ridack_weapon_fire_s.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 180, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_set_v4/ridack_pink_fire/ridack_weapon_pink_s.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 181, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_set_v4/ridack_purple_fire/ridack_weapon_purp_s.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 182, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_set_v4/ridack_red_fire/ridack_weapon_red_s.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 183, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_set_v4/ridack_turquoise_fire/ridack_weapon_turq_s.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 184, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_set_v4/ridack_yellow_fire/ridack_weapon_yellw_s.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 185, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_fire/ridack_weapon_s.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 186, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_galaxy/ridack_bellknife.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 187, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_galaxy_old/ridack_knifebell.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 188, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_ghost/ridack_weapon_s.mse" },

		//Hancer(sol 200-249)
		{ pkBase.EFFECT_SHINING_WEAPON + 200, "PART_WEAPON_LEFT", "D:/ymir work/effect/shining_system/ridack_demonblue/ridack_bluesword2_s.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 201, "PART_WEAPON_LEFT", "D:/ymir work/effect/shining_system/ridack_demongreen/ridack_greensword2_s.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 202, "PART_WEAPON_LEFT", "D:/ymir work/effect/shining_system/ridack_demonred/ridack_redsword2_s.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 203, "PART_WEAPON_LEFT", "D:/ymir work/effect/shining_system/ridack_effect_v7/ridack_weapon_s.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 204, "PART_WEAPON_LEFT", "D:/ymir work/effect/shining_system/ridack_smith/sword/sword_9_s.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 205, "PART_WEAPON_LEFT", "D:/ymir work/effect/shining_system/ridack_set_v1/ridack_effect_blue/ridack_bleusword_s.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 206, "PART_WEAPON_LEFT", "D:/ymir work/effect/shining_system/ridack_set_v1/ridack_effect_brown/ridack_brownsword_s.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 207, "PART_WEAPON_LEFT", "D:/ymir work/effect/shining_system/ridack_set_v1/ridack_effect_green/ridack_greensword_s.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 208, "PART_WEAPON_LEFT", "D:/ymir work/effect/shining_system/ridack_set_v1/ridack_effect_grey/ridack_greysword_s.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 209, "PART_WEAPON_LEFT", "D:/ymir work/effect/shining_system/ridack_set_v1/ridack_effect_orange/ridack_orangesword_s.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 210, "PART_WEAPON_LEFT", "D:/ymir work/effect/shining_system/ridack_set_v1/ridack_effect_pink/ridack_pinksword_s.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 211, "PART_WEAPON_LEFT", "D:/ymir work/effect/shining_system/ridack_set_v1/ridack_effect_purple/ridack_purplesword_s.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 212, "PART_WEAPON_LEFT", "D:/ymir work/effect/shining_system/ridack_set_v1/ridack_effect_red/ridack_redsword_s.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 213, "PART_WEAPON_LEFT", "D:/ymir work/effect/shining_system/ridack_set_v1/ridack_effect_turquoise/ridack_turquoisesword_s.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 214, "PART_WEAPON_LEFT", "D:/ymir work/effect/shining_system/ridack_set_v1/ridack_effect_yellow/ridack_yellowsword_s.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 215, "PART_WEAPON_LEFT", "D:/ymir work/effect/shining_system/ridack_set_v3/ridackeffectblackwhite/ridack_firebw_s.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 216, "PART_WEAPON_LEFT", "D:/ymir work/effect/shining_system/ridack_set_v3/ridackeffectblue/ridack_fireblu_s.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 217, "PART_WEAPON_LEFT", "D:/ymir work/effect/shining_system/ridack_set_v3/ridackeffectbrown/ridack_firebrwn_s.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 218, "PART_WEAPON_LEFT", "D:/ymir work/effect/shining_system/ridack_set_v3/ridackeffectgreen/ridack_firegrn_s.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 219, "PART_WEAPON_LEFT", "D:/ymir work/effect/shining_system/ridack_set_v3/ridackeffectorange/ridack_fireor_s.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 220, "PART_WEAPON_LEFT", "D:/ymir work/effect/shining_system/ridack_set_v3/ridackeffectpink/ridack_firepnk_s.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 221, "PART_WEAPON_LEFT", "D:/ymir work/effect/shining_system/ridack_set_v3/ridackeffectpurple/ridack_firepurp_s.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 222, "PART_WEAPON_LEFT", "D:/ymir work/effect/shining_system/ridack_set_v3/ridackeffectred/ridack_firered_s.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 223, "PART_WEAPON_LEFT", "D:/ymir work/effect/shining_system/ridack_set_v3/ridackeffectturquoise/ridack_firetur_s.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 224, "PART_WEAPON_LEFT", "D:/ymir work/effect/shining_system/ridack_set_v3/ridackeffectyellow/ridack_fireyel_s.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 225, "PART_WEAPON_LEFT", "D:/ymir work/effect/shining_system/ridack_set_v4/ridack_blue_fire/ridack_weapon_blue_s.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 226, "PART_WEAPON_LEFT", "D:/ymir work/effect/shining_system/ridack_set_v4/ridack_bluewhite_fire/ridack_weapon_bw_s.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 227, "PART_WEAPON_LEFT", "D:/ymir work/effect/shining_system/ridack_set_v4/ridack_brown_fire/ridack_weapon_brwn_s.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 228, "PART_WEAPON_LEFT", "D:/ymir work/effect/shining_system/ridack_set_v4/ridack_green_fire/ridack_weapon_green_s.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 229, "PART_WEAPON_LEFT", "D:/ymir work/effect/shining_system/ridack_set_v4/ridack_orange_fire/ridack_weapon_fire_s.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 230, "PART_WEAPON_LEFT", "D:/ymir work/effect/shining_system/ridack_set_v4/ridack_pink_fire/ridack_weapon_pink_s.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 231, "PART_WEAPON_LEFT", "D:/ymir work/effect/shining_system/ridack_set_v4/ridack_purple_fire/ridack_weapon_purp_s.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 232, "PART_WEAPON_LEFT", "D:/ymir work/effect/shining_system/ridack_set_v4/ridack_red_fire/ridack_weapon_red_s.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 233, "PART_WEAPON_LEFT", "D:/ymir work/effect/shining_system/ridack_set_v4/ridack_turquoise_fire/ridack_weapon_turq_s.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 234, "PART_WEAPON_LEFT", "D:/ymir work/effect/shining_system/ridack_set_v4/ridack_yellow_fire/ridack_weapon_yellw_s.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 235, "PART_WEAPON_LEFT", "D:/ymir work/effect/shining_system/ridack_fire/ridack_weapon_s.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 236, "PART_WEAPON_LEFT", "D:/ymir work/effect/shining_system/ridack_galaxy/ridack_bellknife.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 237, "PART_WEAPON_LEFT", "D:/ymir work/effect/shining_system/ridack_galaxy_old/ridack_knifebell.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 238, "PART_WEAPON_LEFT", "D:/ymir work/effect/shining_system/ridack_ghost/ridack_weapon_s.mse" },

		//Pence (sag 250-299)
		{ pkBase.EFFECT_SHINING_WEAPON + 250, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_demonblue/ridack_bluesword2_s.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 251, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_demongreen/ridack_greensword2_s.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 252, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_demonred/ridack_redsword2_s.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 253, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_effect_v7/ridack_weapon_s.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 254, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_smith/sword/sword_9_s.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 255, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_set_v1/ridack_effect_blue/ridack_bleusword_c.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 256, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_set_v1/ridack_effect_brown/ridack_brownsword_c.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 257, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_set_v1/ridack_effect_green/ridack_greensword_c.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 258, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_set_v1/ridack_effect_grey/ridack_greysword_c.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 259, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_set_v1/ridack_effect_orange/ridack_orangesword_c.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 260, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_set_v1/ridack_effect_pink/ridack_pinksword_c.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 261, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_set_v1/ridack_effect_purple/ridack_purplesword_c.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 262, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_set_v1/ridack_effect_red/ridack_redsword_c.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 263, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_set_v1/ridack_effect_turquoise/ridack_turquoisesword_c.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 264, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_set_v1/ridack_effect_yellow/ridack_yellowsword_c.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 265, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_set_v3/ridackeffectblackwhite/ridack_firebw_s.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 266, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_set_v3/ridackeffectblue/ridack_fireblu_s.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 267, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_set_v3/ridackeffectbrown/ridack_firebrwn_s.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 268, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_set_v3/ridackeffectgreen/ridack_firegrn_s.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 269, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_set_v3/ridackeffectorange/ridack_fireor_s.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 270, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_set_v3/ridackeffectpink/ridack_firepnk_s.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 271, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_set_v3/ridackeffectpurple/ridack_firepurp_s.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 272, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_set_v3/ridackeffectred/ridack_firered_s.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 273, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_set_v3/ridackeffectturquoise/ridack_firetur_s.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 274, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_set_v3/ridackeffectyellow/ridack_fireyel_s.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 275, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_set_v4/ridack_blue_fire/ridack_weapon_blue_s.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 276, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_set_v4/ridack_bluewhite_fire/ridack_weapon_bw_s.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 277, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_set_v4/ridack_brown_fire/ridack_weapon_brwn_s.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 278, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_set_v4/ridack_green_fire/ridack_weapon_green_s.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 279, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_set_v4/ridack_orange_fire/ridack_weapon_fire_s.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 280, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_set_v4/ridack_pink_fire/ridack_weapon_pink_s.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 281, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_set_v4/ridack_purple_fire/ridack_weapon_purp_s.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 282, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_set_v4/ridack_red_fire/ridack_weapon_red_s.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 283, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_set_v4/ridack_turquoise_fire/ridack_weapon_turq_s.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 284, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_set_v4/ridack_yellow_fire/ridack_weapon_yellw_s.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 285, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_fire/ridack_weapon_s.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 286, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_galaxy/ridack_bellknife.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 287, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_galaxy_old/ridack_knifebell.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 288, "PART_WEAPON", "D:/ymir work/effect/shining_system/ridack_ghost/ridack_weapon_s.mse" },

		//Pence (sol 300-349)
		{ pkBase.EFFECT_SHINING_WEAPON + 300, "PART_WEAPON_LEFT", "D:/ymir work/effect/shining_system/ridack_demonblue/ridack_bluesword2_s.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 301, "PART_WEAPON_LEFT", "D:/ymir work/effect/shining_system/ridack_demongreen/ridack_greensword2_s.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 302, "PART_WEAPON_LEFT", "D:/ymir work/effect/shining_system/ridack_demonred/ridack_redsword2_s.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 303, "PART_WEAPON_LEFT", "D:/ymir work/effect/shining_system/ridack_effect_v7/ridack_weapon_s.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 304, "PART_WEAPON_LEFT", "D:/ymir work/effect/shining_system/ridack_smith/sword/sword_9_s.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 305, "PART_WEAPON_LEFT", "D:/ymir work/effect/shining_system/ridack_set_v1/ridack_effect_blue/ridack_bleusword_c.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 306, "PART_WEAPON_LEFT", "D:/ymir work/effect/shining_system/ridack_set_v1/ridack_effect_brown/ridack_brownsword_c.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 307, "PART_WEAPON_LEFT", "D:/ymir work/effect/shining_system/ridack_set_v1/ridack_effect_green/ridack_greensword_c.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 308, "PART_WEAPON_LEFT", "D:/ymir work/effect/shining_system/ridack_set_v1/ridack_effect_grey/ridack_greysword_c.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 309, "PART_WEAPON_LEFT", "D:/ymir work/effect/shining_system/ridack_set_v1/ridack_effect_orange/ridack_orangesword_c.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 310, "PART_WEAPON_LEFT", "D:/ymir work/effect/shining_system/ridack_set_v1/ridack_effect_pink/ridack_pinksword_c.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 311, "PART_WEAPON_LEFT", "D:/ymir work/effect/shining_system/ridack_set_v1/ridack_effect_purple/ridack_purplesword_c.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 312, "PART_WEAPON_LEFT", "D:/ymir work/effect/shining_system/ridack_set_v1/ridack_effect_red/ridack_redsword_c.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 313, "PART_WEAPON_LEFT", "D:/ymir work/effect/shining_system/ridack_set_v1/ridack_effect_turquoise/ridack_turquoisesword_c.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 314, "PART_WEAPON_LEFT", "D:/ymir work/effect/shining_system/ridack_set_v1/ridack_effect_yellow/ridack_yellowsword_c.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 315, "PART_WEAPON_LEFT", "D:/ymir work/effect/shining_system/ridack_set_v3/ridackeffectblackwhite/ridack_firebw_s.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 316, "PART_WEAPON_LEFT", "D:/ymir work/effect/shining_system/ridack_set_v3/ridackeffectblue/ridack_fireblu_s.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 317, "PART_WEAPON_LEFT", "D:/ymir work/effect/shining_system/ridack_set_v3/ridackeffectbrown/ridack_firebrwn_s.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 318, "PART_WEAPON_LEFT", "D:/ymir work/effect/shining_system/ridack_set_v3/ridackeffectgreen/ridack_firegrn_s.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 319, "PART_WEAPON_LEFT", "D:/ymir work/effect/shining_system/ridack_set_v3/ridackeffectorange/ridack_fireor_s.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 320, "PART_WEAPON_LEFT", "D:/ymir work/effect/shining_system/ridack_set_v3/ridackeffectpink/ridack_firepnk_s.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 321, "PART_WEAPON_LEFT", "D:/ymir work/effect/shining_system/ridack_set_v3/ridackeffectpurple/ridack_firepurp_s.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 322, "PART_WEAPON_LEFT", "D:/ymir work/effect/shining_system/ridack_set_v3/ridackeffectred/ridack_firered_s.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 323, "PART_WEAPON_LEFT", "D:/ymir work/effect/shining_system/ridack_set_v3/ridackeffectturquoise/ridack_firetur_s.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 324, "PART_WEAPON_LEFT", "D:/ymir work/effect/shining_system/ridack_set_v3/ridackeffectyellow/ridack_fireyel_s.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 325, "PART_WEAPON_LEFT", "D:/ymir work/effect/shining_system/ridack_set_v4/ridack_blue_fire/ridack_weapon_blue_s.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 326, "PART_WEAPON_LEFT", "D:/ymir work/effect/shining_system/ridack_set_v4/ridack_bluewhite_fire/ridack_weapon_bw_s.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 327, "PART_WEAPON_LEFT", "D:/ymir work/effect/shining_system/ridack_set_v4/ridack_brown_fire/ridack_weapon_brwn_s.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 328, "PART_WEAPON_LEFT", "D:/ymir work/effect/shining_system/ridack_set_v4/ridack_green_fire/ridack_weapon_green_s.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 329, "PART_WEAPON_LEFT", "D:/ymir work/effect/shining_system/ridack_set_v4/ridack_orange_fire/ridack_weapon_fire_s.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 330, "PART_WEAPON_LEFT", "D:/ymir work/effect/shining_system/ridack_set_v4/ridack_pink_fire/ridack_weapon_pink_s.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 331, "PART_WEAPON_LEFT", "D:/ymir work/effect/shining_system/ridack_set_v4/ridack_purple_fire/ridack_weapon_purp_s.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 332, "PART_WEAPON_LEFT", "D:/ymir work/effect/shining_system/ridack_set_v4/ridack_red_fire/ridack_weapon_red_s.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 333, "PART_WEAPON_LEFT", "D:/ymir work/effect/shining_system/ridack_set_v4/ridack_turquoise_fire/ridack_weapon_turq_s.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 334, "PART_WEAPON_LEFT", "D:/ymir work/effect/shining_system/ridack_set_v4/ridack_yellow_fire/ridack_weapon_yellw_s.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 335, "PART_WEAPON_LEFT", "D:/ymir work/effect/shining_system/ridack_fire/ridack_weapon_s.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 336, "PART_WEAPON_LEFT", "D:/ymir work/effect/shining_system/ridack_galaxy/ridack_bellknife.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 337, "PART_WEAPON_LEFT", "D:/ymir work/effect/shining_system/ridack_galaxy_old/ridack_knifebell.mse" },
		{ pkBase.EFFECT_SHINING_WEAPON + 338, "PART_WEAPON_LEFT", "D:/ymir work/effect/shining_system/ridack_ghost/ridack_weapon_s.mse" },

		//Zirh 0-50
		{ pkBase.EFFECT_SHINING_ARMOR + 0, "Bip01", "D:/ymir work/effect/shining_system/ridack_demonblue/ridack_bluearmor2.mse" },
		{ pkBase.EFFECT_SHINING_ARMOR + 1, "Bip01", "D:/ymir work/effect/shining_system/ridack_demongreen/ridack_greenarmor2.mse" },
		{ pkBase.EFFECT_SHINING_ARMOR + 2, "Bip01", "D:/ymir work/effect/shining_system/ridack_demonred/ridack_redarmor2.mse" },
		{ pkBase.EFFECT_SHINING_ARMOR + 3, "Bip01", "D:/ymir work/effect/shining_system/ridack_effect_v7/ridack_armor.mse" },
		{ pkBase.EFFECT_SHINING_ARMOR + 4, "Bip01", "D:/ymir work/effect/shining_system/ridack_smith/armor/armor_9.mse" },
		{ pkBase.EFFECT_SHINING_ARMOR + 5, "Bip01", "D:/ymir work/effect/shining_system/ridack_set_v1/ridack_effect_blue/ridack_bleuarmor.mse" },
		{ pkBase.EFFECT_SHINING_ARMOR + 6, "Bip01", "D:/ymir work/effect/shining_system/ridack_set_v1/ridack_effect_brown/ridack_brownarmor.mse" },
		{ pkBase.EFFECT_SHINING_ARMOR + 7, "Bip01", "D:/ymir work/effect/shining_system/ridack_set_v1/ridack_effect_green/ridack_greenarmor.mse" },
		{ pkBase.EFFECT_SHINING_ARMOR + 8, "Bip01", "D:/ymir work/effect/shining_system/ridack_set_v1/ridack_effect_grey/ridack_greyarmor.mse" },
		{ pkBase.EFFECT_SHINING_ARMOR + 9, "Bip01", "D:/ymir work/effect/shining_system/ridack_set_v1/ridack_effect_orange/ridack_orangearmor.mse" },
		{ pkBase.EFFECT_SHINING_ARMOR + 10, "Bip01", "D:/ymir work/effect/shining_system/ridack_set_v1/ridack_effect_pink/ridack_pinkarmor.mse" },
		{ pkBase.EFFECT_SHINING_ARMOR + 11, "Bip01", "D:/ymir work/effect/shining_system/ridack_set_v1/ridack_effect_purple/ridack_purplearmor.mse" },
		{ pkBase.EFFECT_SHINING_ARMOR + 12, "Bip01", "D:/ymir work/effect/shining_system/ridack_set_v1/ridack_effect_red/ridack_redarmor.mse" },
		{ pkBase.EFFECT_SHINING_ARMOR + 13, "Bip01", "D:/ymir work/effect/shining_system/ridack_set_v1/ridack_effect_turquoise/ridack_turquoisearmor.mse" },
		{ pkBase.EFFECT_SHINING_ARMOR + 14, "Bip01", "D:/ymir work/effect/shining_system/ridack_set_v1/ridack_effect_yellow/ridack_yellowarmor.mse" },
		{ pkBase.EFFECT_SHINING_ARMOR + 15, "Bip01", "D:/ymir work/effect/shining_system/ridack_set_v3/ridackeffectblackwhite/ridack_armorfirebw.mse" },
		{ pkBase.EFFECT_SHINING_ARMOR + 16, "Bip01", "D:/ymir work/effect/shining_system/ridack_set_v3/ridackeffectblue/ridack_armorfireblu.mse" },
		{ pkBase.EFFECT_SHINING_ARMOR + 17, "Bip01", "D:/ymir work/effect/shining_system/ridack_set_v3/ridackeffectbrown/ridack_armorfirebrwn.mse" },
		{ pkBase.EFFECT_SHINING_ARMOR + 18, "Bip01", "D:/ymir work/effect/shining_system/ridack_set_v3/ridackeffectgreen/ridack_armorfiregrn.mse" },
		{ pkBase.EFFECT_SHINING_ARMOR + 19, "Bip01", "D:/ymir work/effect/shining_system/ridack_set_v3/ridackeffectorange/ridack_armorfireor.mse" },
		{ pkBase.EFFECT_SHINING_ARMOR + 20, "Bip01", "D:/ymir work/effect/shining_system/ridack_set_v3/ridackeffectpink/ridack_armorfirepnk.mse" },
		{ pkBase.EFFECT_SHINING_ARMOR + 21, "Bip01", "D:/ymir work/effect/shining_system/ridack_set_v3/ridackeffectpurple/ridack_armorfirepurp.mse" },
		{ pkBase.EFFECT_SHINING_ARMOR + 22, "Bip01", "D:/ymir work/effect/shining_system/ridack_set_v3/ridackeffectred/ridack_armorfirered.mse" },
		{ pkBase.EFFECT_SHINING_ARMOR + 23, "Bip01", "D:/ymir work/effect/shining_system/ridack_set_v3/ridackeffectturquoise/ridack_armorfiretur.mse" },
		{ pkBase.EFFECT_SHINING_ARMOR + 24, "Bip01", "D:/ymir work/effect/shining_system/ridack_set_v3/ridackeffectyellow/ridack_armorfireyel.mse" },
		{ pkBase.EFFECT_SHINING_ARMOR + 25, "Bip01", "D:/ymir work/effect/shining_system/ridack_set_v4/ridack_blue_fire/ridack_armor_blue.mse" },
		{ pkBase.EFFECT_SHINING_ARMOR + 26, "Bip01", "D:/ymir work/effect/shining_system/ridack_set_v4/ridack_bluewhite_fire/ridack_armor_bw.mse" },
		{ pkBase.EFFECT_SHINING_ARMOR + 27, "Bip01", "D:/ymir work/effect/shining_system/ridack_set_v4/ridack_brown_fire/ridack_armor_brwn.mse" },
		{ pkBase.EFFECT_SHINING_ARMOR + 28, "Bip01", "D:/ymir work/effect/shining_system/ridack_set_v4/ridack_green_fire/ridack_armor_green.mse" },
		{ pkBase.EFFECT_SHINING_ARMOR + 29, "Bip01", "D:/ymir work/effect/shining_system/ridack_set_v4/ridack_orange_fire/ridack_armor_fire.mse" },
		{ pkBase.EFFECT_SHINING_ARMOR + 30, "Bip01", "D:/ymir work/effect/shining_system/ridack_set_v4/ridack_pink_fire/ridack_armor_pink.mse" },
		{ pkBase.EFFECT_SHINING_ARMOR + 31, "Bip01", "D:/ymir work/effect/shining_system/ridack_set_v4/ridack_purple_fire/ridack_armor_purp.mse" },
		{ pkBase.EFFECT_SHINING_ARMOR + 32, "Bip01", "D:/ymir work/effect/shining_system/ridack_set_v4/ridack_red_fire/ridack_armor_red.mse" },
		{ pkBase.EFFECT_SHINING_ARMOR + 33, "Bip01", "D:/ymir work/effect/shining_system/ridack_set_v4/ridack_turquoise_fire/ridack_armor_turq.mse" },
		{ pkBase.EFFECT_SHINING_ARMOR + 34, "Bip01", "D:/ymir work/effect/shining_system/ridack_set_v4/ridack_yellow_fire/ridack_armor_yellw.mse" },
		{ pkBase.EFFECT_SHINING_ARMOR + 35, "Bip01", "D:/ymir work/effect/shining_system/ridack_fire/ridack_armor.mse" },
		{ pkBase.EFFECT_SHINING_ARMOR + 36, "Bip01", "D:/ymir work/effect/shining_system/ridack_galaxy/ridack_costumeffect.mse" },
		{ pkBase.EFFECT_SHINING_ARMOR + 37, "Bip01", "D:/ymir work/effect/shining_system/ridack_galaxy_old/ridack_spear.mse" },
		{ pkBase.EFFECT_SHINING_ARMOR + 38, "Bip01", "D:/ymir work/effect/shining_system/ridack_ghost/ridack_armor.mse" },
		{ pkBase.EFFECT_SHINING_ARMOR + 39, "Bip01", "D:/ymir work/effect/shining_system/ridack_valentin/ridack_valcostum.mse" },

		//Ozel 0-10
		//{ pkBase.EFFECT_SHINING_SPECIAL + 0, "Bip01", "d:/ymir work/effect/etc/buff/dragonbone.mse" },
		{ pkBase.EFFECT_SHINING_SPECIAL3 + 0, "", "d:/ymir work/effect/etc/soul/soul_red_001.mse" },
		{ pkBase.EFFECT_SHINING_SPECIAL3 + 1, "", "d:/ymir work/effect/etc/soul/soul_blue_001.mse" },
		{ pkBase.EFFECT_SHINING_SPECIAL3 + 2, "", "d:/ymir work/effect/etc/soul/ridack_soul3.mse" },
		{ pkBase.EFFECT_SHINING_SPECIAL3 + 3, "", "d:/ymir work/effect/etc/soul/ridack_soul4.mse" },
		{ pkBase.EFFECT_SHINING_SPECIAL3 + 4, "", "d:/ymir work/effect/etc/soul/ridack_soul5.mse" },
		{ pkBase.EFFECT_SHINING_SPECIAL3 + 5, "", "d:/ymir work/effect/etc/soul/soul_mix_001.mse" },
		//Ozel2 0-10
		//{ pkBase.EFFECT_SHINING_SPECIAL2 + 0, "Bip01", "d:/ymir work/effect/etc/buff/dragonbone2.mse" },
		//Kanat 0-50
		{ pkBase.EFFECT_SHINING_WING + 0, "Bip01", "D:/ymir work/aoh/wings/1.mse" },
		{ pkBase.EFFECT_SHINING_WING + 1, "Bip01", "D:/ymir work/aoh/wings/2.mse" },
		{ pkBase.EFFECT_SHINING_WING + 2, "Bip01", "D:/ymir work/aoh/wings/3.mse" },
		{ pkBase.EFFECT_SHINING_WING + 3, "Bip01", "D:/ymir work/aoh/wings/4.mse" },
		{ pkBase.EFFECT_SHINING_WING + 4, "Bip01", "D:/ymir work/aoh/wings/5.mse" },
		{ pkBase.EFFECT_SHINING_WING + 5, "Bip01", "D:/ymir work/aoh/wings/fairy_wings.mse" },
		{ pkBase.EFFECT_SHINING_WING + 6, "Bip01", "D:/ymir work/aoh/wings/mecha_wings.mse" },
		{ pkBase.EFFECT_SHINING_WING + 7, "Bip01", "D:/ymir work/aoh/wings/pheonix_wings.mse" },
		{ pkBase.EFFECT_SHINING_WING + 8, "Bip01", "D:/ymir work/aoh/wings/raven_wings.mse" },
		{ pkBase.EFFECT_SHINING_WING + 9, "Bip01", "D:/ymir work/kami_effects/wings/kami_deamon_wings_lv1.mse" },
		{ pkBase.EFFECT_SHINING_WING + 10, "Bip01", "D:/ymir work/kami_effects/wings/kami_deamon_wings_lv2.mse" },
		{ pkBase.EFFECT_SHINING_WING + 11, "Bip01", "D:/ymir work/kami_effects/wings/kami_deamon_wings_lv3.mse" },
		{ pkBase.EFFECT_SHINING_WING + 12, "Bip01", "D:/ymir work/kami_effects/wings/kami_illusion_wing_lv1.mse" },
		{ pkBase.EFFECT_SHINING_WING + 13, "Bip01", "D:/ymir work/kami_effects/wings/kami_illusion_wing_lv2.mse" },
		{ pkBase.EFFECT_SHINING_WING + 14, "Bip01", "D:/ymir work/kami_effects/wings/kami_illusion_wing_lv3.mse" },
		{ pkBase.EFFECT_SHINING_WING + 15, "Bip01", "D:/ymir work/kami_effects/wings/kami_pheonix_wings_lv1.mse" },
		{ pkBase.EFFECT_SHINING_WING + 16, "Bip01", "D:/ymir work/kami_effects/wings/kami_pheonix_wings_lv2.mse" },
		{ pkBase.EFFECT_SHINING_WING + 17, "Bip01", "D:/ymir work/kami_effects/wings/kami_pheonix_wings_lv3.mse" },
		{ pkBase.EFFECT_SHINING_WING + 18, "Bip01", "D:/ymir work/kami_effects/wings/kami_sky_wings_lv1.mse" },
		{ pkBase.EFFECT_SHINING_WING + 19, "Bip01", "D:/ymir work/kami_effects/wings/kami_sky_wings_lv2.mse" },
		{ pkBase.EFFECT_SHINING_WING + 20, "Bip01", "D:/ymir work/kami_effects/wings/kami_sky_wings_lv3.mse" },
		{ pkBase.EFFECT_SHINING_WING + 21, "Bip01", "D:/ymir work/effect/ridack_work/ridack_wing.mse" },
		{ pkBase.EFFECT_SHINING_WING + 22, "Bip01", "D:/ymir work/aoh/wings/dark_wings.mse" },
		{ pkBase.EFFECT_SHINING_WING + 23, "Bip01", "D:/ymir work/ridack_wing/five_color/ridack_blackwing.mse" },
		{ pkBase.EFFECT_SHINING_WING + 24, "Bip01", "D:/ymir work/ridack_wing/five_color/ridack_bluewing.mse" },
		{ pkBase.EFFECT_SHINING_WING + 25, "Bip01", "D:/ymir work/ridack_wing/five_color/ridack_redwing.mse" },
		{ pkBase.EFFECT_SHINING_WING + 26, "Bip01", "D:/ymir work/ridack_wing/five_color/ridack_whitewing.mse" },
		{ pkBase.EFFECT_SHINING_WING + 27, "Bip01", "D:/ymir work/ridack_wing/five_color/ridack_yellowwing.mse" },
		{ pkBase.EFFECT_SHINING_WING + 28, "Bip01", "D:/ymir work/ridack_wing/angel_wing/ridack_angelwingice.mse" },
		{ pkBase.EFFECT_SHINING_WING + 29, "Bip01", "D:/ymir work/ridack_wing/black_wing/ridack_wingblack.mse" },
		{ pkBase.EFFECT_SHINING_WING + 30, "Bip01", "D:/ymir work/ridack_wing/death_wing/ridack_wing4.mse" },
		{ pkBase.EFFECT_SHINING_WING + 31, "Bip01", "D:/ymir work/ridack_wing/fire_wing/ridack_firewing.mse" },
		{ pkBase.EFFECT_SHINING_WING + 32, "Bip01", "D:/ymir work/ridack_wing/magic_fire/ridack_wingfire.mse" },
		{ pkBase.EFFECT_SHINING_WING + 33, "Bip01", "D:/ymir work/ridack_wing/mystic_wing/ridack_wing9.mse" },
		{ pkBase.EFFECT_SHINING_WING + 34, "Bip01", "D:/ymir work/ridack_wing/phoenix_wing/ridack_pheonixwingfire.mse" },
		{ pkBase.EFFECT_SHINING_WING + 35, "Bip01", "D:/ymir work/ridack_wing/pink_wing/ridack_wingstval.mse" },
		{ pkBase.EFFECT_SHINING_WING + 36, "Bip01", "D:/ymir work/ridack_wing/five_angel/ridack_wingblack.mse" },
		{ pkBase.EFFECT_SHINING_WING + 37, "Bip01", "D:/ymir work/ridack_wing/five_angel/ridack_wingblue.mse" },
		{ pkBase.EFFECT_SHINING_WING + 38, "Bip01", "D:/ymir work/ridack_wing/five_angel/ridack_wingred.mse" },
		{ pkBase.EFFECT_SHINING_WING + 39, "Bip01", "D:/ymir work/ridack_wing/five_angel/ridack_wingwhite.mse" },
		{ pkBase.EFFECT_SHINING_WING + 40, "Bip01", "D:/ymir work/ridack_wing/five_angel/ridack_wingyellow.mse" },
		{ pkBase.EFFECT_SHINING_WING + 41, "Bip01", "D:/ymir work/ridack_wing/electric_wing/ridack_electricwing.mse" },
#endif
	};

	std::vector<TEffect> m_vecEmoticonData =
	{
		{ pkBase.EFFECT_EMOTICON + 0, "", "d:/ymir work/effect/etc/emoticon/sweat.mse" },
		{ pkBase.EFFECT_EMOTICON + 1, "", "d:/ymir work/effect/etc/emoticon/money.mse" },
		{ pkBase.EFFECT_EMOTICON + 2, "", "d:/ymir work/effect/etc/emoticon/happy.mse" },
		{ pkBase.EFFECT_EMOTICON + 3, "", "d:/ymir work/effect/etc/emoticon/love_s.mse" },
		{ pkBase.EFFECT_EMOTICON + 4, "", "d:/ymir work/effect/etc/emoticon/love_l.mse" },
		{ pkBase.EFFECT_EMOTICON + 5, "", "d:/ymir work/effect/etc/emoticon/angry.mse" },
		{ pkBase.EFFECT_EMOTICON + 6, "", "d:/ymir work/effect/etc/emoticon/aha.mse" },
		{ pkBase.EFFECT_EMOTICON + 7, "", "d:/ymir work/effect/etc/emoticon/gloom.mse" },
		{ pkBase.EFFECT_EMOTICON + 8, "", "d:/ymir work/effect/etc/emoticon/sorry.mse" },
		{ pkBase.EFFECT_EMOTICON + 9, "", "d:/ymir work/effect/etc/emoticon/!_mix_back.mse" },
		{ pkBase.EFFECT_EMOTICON + 10, "", "d:/ymir work/effect/etc/emoticon/question.mse" },
		{ pkBase.EFFECT_EMOTICON + 11, "", "d:/ymir work/effect/etc/emoticon/fish.mse" },
	};

	enum
	{
		FLY_NONE,
		FLY_EXP,
		FLY_HP_MEDIUM,
		FLY_HP_BIG,
		FLY_SP_SMALL,
		FLY_SP_MEDIUM,
		FLY_SP_BIG,
		FLY_FIREWORK1,
		FLY_FIREWORK2,
		FLY_FIREWORK3,
		FLY_FIREWORK4,
		FLY_FIREWORK5,
		FLY_FIREWORK6,
		FLY_FIREWORK_XMAS,
		FLY_CHAIN_LIGHTNING,
		FLY_HP_SMALL,
		FLY_SKILL_MUYEONG,
#ifdef ENABLE_QUIVER_SYSTEM
		FLY_QUIVER_ATTACK_NORMAL,
#endif
	};

	std::vector<SFly> m_vecFlyData = {
		{ FLY_EXP, pkFly.INDEX_FLY_TYPE_NORMAL, "d:/ymir work/effect/etc/gathering/ga_piece_yellow_small2.msf" },
		{ FLY_HP_MEDIUM, pkFly.INDEX_FLY_TYPE_NORMAL, "d:/ymir work/effect/etc/gathering/ga_piece_red_small.msf" },
		{ FLY_HP_BIG, pkFly.INDEX_FLY_TYPE_NORMAL, "d:/ymir work/effect/etc/gathering/ga_piece_red_big.msf" },
		{ FLY_SP_SMALL, pkFly.INDEX_FLY_TYPE_NORMAL, "d:/ymir work/effect/etc/gathering/ga_piece_blue_warrior_small.msf" },
		{ FLY_SP_MEDIUM, pkFly.INDEX_FLY_TYPE_NORMAL, "d:/ymir work/effect/etc/gathering/ga_piece_blue_small.msf" },
		{ FLY_SP_BIG, pkFly.INDEX_FLY_TYPE_NORMAL, "d:/ymir work/effect/etc/gathering/ga_piece_blue_big.msf" },
		{ FLY_FIREWORK1, pkFly.INDEX_FLY_TYPE_FIRE_CRACKER, "d:/ymir work/effect/etc/firecracker/firecracker_1.msf" },
		{ FLY_FIREWORK2, pkFly.INDEX_FLY_TYPE_FIRE_CRACKER, "d:/ymir work/effect/etc/firecracker/firecracker_2.msf" },
		{ FLY_FIREWORK3, pkFly.INDEX_FLY_TYPE_FIRE_CRACKER, "d:/ymir work/effect/etc/firecracker/firecracker_3.msf" },
		{ FLY_FIREWORK4, pkFly.INDEX_FLY_TYPE_FIRE_CRACKER, "d:/ymir work/effect/etc/firecracker/firecracker_4.msf" },
		{ FLY_FIREWORK5, pkFly.INDEX_FLY_TYPE_FIRE_CRACKER, "d:/ymir work/effect/etc/firecracker/firecracker_5.msf" },
		{ FLY_FIREWORK6, pkFly.INDEX_FLY_TYPE_FIRE_CRACKER, "d:/ymir work/effect/etc/firecracker/firecracker_6.msf" },
		{ FLY_FIREWORK_XMAS, pkFly.INDEX_FLY_TYPE_FIRE_CRACKER, "d:/ymir work/effect/etc/firecracker/firecracker_xmas.msf" },
		{ FLY_CHAIN_LIGHTNING, pkFly.INDEX_FLY_TYPE_NORMAL, "d:/ymir work/pc/shaman/effect/pokroe.msf" },
		{ FLY_HP_SMALL, pkFly.INDEX_FLY_TYPE_NORMAL, "d:/ymir work/effect/etc/gathering/ga_piece_red_smallest.msf" },
		{ FLY_SKILL_MUYEONG, pkFly.INDEX_FLY_TYPE_AUTO_FIRE, "d:/ymir work/pc/sura/effect/muyeong_fly.msf" },
#ifdef ENABLE_QUIVER_SYSTEM
		{ FLY_QUIVER_ATTACK_NORMAL, pkFly.INDEX_FLY_TYPE_NORMAL, "d:/ymir work/pc/assassin/effect/arrow_02.msf" },
#endif
	};

	// GM_EFFECT
	TEffect effect;
	effect.uiType = pkBase.EFFECT_AFFECT + 0;
	memcpy(effect.stBone, "Bip01", sizeof(effect.stBone));
	memcpy(effect.stEffect, GM_MARK, sizeof(effect.stEffect));

	m_vecEffectData.push_back(effect);
	// GM_EFFECT

	for (const auto& it : m_vecEffectData)
	{
		pkBase.RegisterEffect(it.uiType, it.stBone, it.stEffect, false);
	}

	CPythonNetworkStream& rkNetStream = CPythonNetworkStream::Instance();

	rkNetStream.RegisterEmoticonString("(sweat)");
	rkNetStream.RegisterEmoticonString("(money)");
	rkNetStream.RegisterEmoticonString("(happy)");
	rkNetStream.RegisterEmoticonString("(love)");
	rkNetStream.RegisterEmoticonString("(love2)");
	rkNetStream.RegisterEmoticonString("(angry)");
	rkNetStream.RegisterEmoticonString("(aha)");
	rkNetStream.RegisterEmoticonString("(gloom)");
	rkNetStream.RegisterEmoticonString("(sorry)");
	rkNetStream.RegisterEmoticonString("(!)");
	rkNetStream.RegisterEmoticonString("(?)");
	rkNetStream.RegisterEmoticonString("(fish)");

	for (const auto& it : m_vecEmoticonData)
	{
		pkBase.RegisterEffect(it.uiType, it.stBone, it.stEffect, false);
	}

	for (const auto& it : m_vecFlyData)
	{
		pkFly.RegisterIndexedFlyData(it.dwIndex, it.byType, it.stName);
	}

	return true;
}

#ifdef USE_LOADING_DLG_OPTIMIZATION
bool CPlayerSettingsModule::LoadGameNPC(char stFolder[FILE_MAX_NUM])
#else
const bool CPlayerSettingsModule::LoadGameNPC(char stFolder[FILE_MAX_NUM])
#endif // USE_LOADING_DLG_OPTIMIZATION
{
	const VOID* pvData;
	CMappedFile kFile;
	if (!CEterPackManager::Instance().Get(kFile, stFolder, &pvData))
	{
		TraceError("LoadGameNPC(c_szFileName=%s) - Load Error", stFolder);
		return false;
	}

	std::string fileData((const char*)pvData, kFile.Size());
	const auto& lines = msl::string_split_any(fileData, "\r\n");
	for (const auto& line : lines)
	{
		auto& kTokenVector = msl::string_split(line, "\t");
		for (auto& token : kTokenVector)
			msl::trim_in_place(token);

		if (kTokenVector.size() < 2)
			continue;

		uint32_t vnum = atoi(kTokenVector[0].c_str());
		if (vnum)
			CRaceManager::Instance().RegisterRaceName(vnum, kTokenVector[1].c_str());
		else if (kTokenVector.size() >= 3)
			CRaceManager::Instance().RegisterRaceSrcName(kTokenVector[1].c_str(), kTokenVector[2].c_str());
		else
			TraceError("LoadGameNPC(c_szFileName=%s) - Line Error %s %s", stFolder, kTokenVector[0].c_str(), kTokenVector[1].c_str());
	}

	return true;
}

#ifdef USE_LOADING_DLG_OPTIMIZATION
bool CPlayerSettingsModule::RegisterEmotionAnis(char stFolder[FILE_MAX_NUM])
#else
const bool CPlayerSettingsModule::RegisterEmotionAnis(char stFolder[FILE_MAX_NUM])
#endif // USE_LOADING_DLG_OPTIMIZATION
{
	CRaceManager& pkManager = CRaceManager::Instance();

	CRaceData* pRaceData = pkManager.GetSelectedRaceDataPointer();
	if (!pRaceData)
		return false;

	std::vector<SMotion> m_vecEmotion =
	{
		{ pkMotionManager.NAME_CLAP, "clap.msa", 0 },
		{ pkMotionManager.NAME_CHEERS_1, "cheers_1.msa", 0 },
		{ pkMotionManager.NAME_CHEERS_2, "cheers_2.msa", 0 },
		{ pkMotionManager.NAME_DANCE_1, "dance_1.msa", 0 },
		{ pkMotionManager.NAME_DANCE_2, "dance_2.msa", 0 },
		{ pkMotionManager.NAME_DANCE_3, "dance_3.msa", 0 },
		{ pkMotionManager.NAME_DANCE_4, "dance_4.msa", 0 },
		{ pkMotionManager.NAME_DANCE_5, "dance_5.msa", 0 },
		{ pkMotionManager.NAME_DANCE_6, "dance_6.msa", 0 },
		{ pkMotionManager.NAME_CONGRATULATION, "congratulation.msa", 0 },
		{ pkMotionManager.NAME_FORGIVE, "forgive.msa", 0 },
		{ pkMotionManager.NAME_ANGRY, "angry.msa", 0 },
		{ pkMotionManager.NAME_ATTRACTIVE, "attractive.msa", 0 },
		{ pkMotionManager.NAME_SAD, "sad.msa", 0 },
		{ pkMotionManager.NAME_SHY, "shy.msa", 0 },
		{ pkMotionManager.NAME_CHEERUP, "cheerup.msa", 0 },
		{ pkMotionManager.NAME_BANTER, "banter.msa", 0 },
		{ pkMotionManager.NAME_JOY, "joy.msa", 0 },
		{ pkMotionManager.NAME_FRENCH_KISS_WITH_WARRIOR, "french_kiss_with_warrior.msa", 0 },
		{ pkMotionManager.NAME_FRENCH_KISS_WITH_ASSASSIN, "french_kiss_with_assassin.msa", 0 },
		{ pkMotionManager.NAME_FRENCH_KISS_WITH_SURA, "french_kiss_with_sura.msa", 0 },
		{ pkMotionManager.NAME_FRENCH_KISS_WITH_SHAMAN, "french_kiss_with_shaman.msa", 0 },
#ifdef ENABLE_WOLFMAN_CHARACTER
		{ pkMotionManager.NAME_FRENCH_KISS_WITH_WOLFMAN, "french_kiss_with_wolfman.msa", 0 },
#endif
		{ pkMotionManager.NAME_KISS_WITH_WARRIOR, "kiss_with_warrior.msa", 0 },
		{ pkMotionManager.NAME_KISS_WITH_ASSASSIN, "kiss_with_assassin.msa", 0 },
		{ pkMotionManager.NAME_KISS_WITH_SURA, "kiss_with_sura.msa", 0 },
		{ pkMotionManager.NAME_KISS_WITH_SHAMAN, "kiss_with_shaman.msa", 0 },
#ifdef ENABLE_WOLFMAN_CHARACTER
		{ pkMotionManager.NAME_KISS_WITH_WOLFMAN, "kiss_with_wolfman.msa", 0 },
#endif
		{ pkMotionManager.NAME_SLAP_HIT_WITH_WARRIOR, "slap_hit.msa", 0 },
		{ pkMotionManager.NAME_SLAP_HIT_WITH_ASSASSIN, "slap_hit.msa", 0 },
		{ pkMotionManager.NAME_SLAP_HIT_WITH_SURA, "slap_hit.msa", 0 },
		{ pkMotionManager.NAME_SLAP_HIT_WITH_SHAMAN, "slap_hit.msa", 0 },
#ifdef ENABLE_WOLFMAN_CHARACTER
		{ pkMotionManager.NAME_SLAP_HIT_WITH_WOLFMAN, "slap_hit.msa", 0 },
#endif
		{ pkMotionManager.NAME_SLAP_HURT_WITH_WARRIOR, "slap_hurt.msa", 0 },
		{ pkMotionManager.NAME_SLAP_HURT_WITH_ASSASSIN, "slap_hurt.msa", 0 },
		{ pkMotionManager.NAME_SLAP_HURT_WITH_SURA, "slap_hurt.msa", 0 },
		{ pkMotionManager.NAME_SLAP_HURT_WITH_SHAMAN, "slap_hurt.msa", 0 },
#ifdef ENABLE_WOLFMAN_CHARACTER
		{ pkMotionManager.NAME_SLAP_HURT_WITH_WOLFMAN, "slap_hurt.msa", 0 },
#endif
	};

	char szFileName[FILE_MAX_NUM];
	pRaceData->RegisterMotionMode(pkMotionManager.MODE_GENERAL);
	for (const auto& it : m_vecEmotion)
	{
		snprintf(szFileName, sizeof(szFileName), "%s%s%s", stFolder, "action/", it.stName);
		const CGraphicThing *pkMotionThing = pRaceData->RegisterMotionData(pkMotionManager.MODE_GENERAL, it.wMotionIndex, szFileName, it.byPercentage);
		if (pkMotionThing)
			CResourceManager::Instance().LoadStaticCache(pkMotionThing->GetFileName());
	}

	return true;
}

#ifdef USE_LOADING_DLG_OPTIMIZATION
bool CPlayerSettingsModule::LoadGeneralMotion(char stFolder[FILE_MAX_NUM])
#else
const bool CPlayerSettingsModule::LoadGeneralMotion(char stFolder[FILE_MAX_NUM])
#endif // USE_LOADING_DLG_OPTIMIZATION
{
	CRaceManager& pkManager = CRaceManager::Instance();

	CRaceData* pRaceData = pkManager.GetSelectedRaceDataPointer();
	if (!pRaceData)
		return false;

	std::vector<SMotion> m_vecBasicMotion =
	{
		{ pkMotionManager.NAME_WAIT, "wait.msa", 0 },
		{ pkMotionManager.NAME_WALK, "walk.msa", 0 },
		{ pkMotionManager.NAME_RUN, "run.msa", 0 },
		{ pkMotionManager.NAME_DAMAGE, "damage.msa", 50 },
		{ pkMotionManager.NAME_DAMAGE, "damage_1.msa", 50 },
		{ pkMotionManager.NAME_DAMAGE_BACK, "damage_2.msa", 50 },
		{ pkMotionManager.NAME_DAMAGE_BACK, "damage_3.msa", 50 },
		{ pkMotionManager.NAME_DAMAGE_FLYING, "damage_flying.msa", 0 },
		{ pkMotionManager.NAME_STAND_UP, "falling_stand.msa", 0 },
		{ pkMotionManager.NAME_DAMAGE_FLYING_BACK, "back_damage_flying.msa", 0 },
		{ pkMotionManager.NAME_STAND_UP_BACK, "back_falling_stand.msa", 0 },
		{ pkMotionManager.NAME_DEAD, "dead.msa", 0 },
		{ pkMotionManager.NAME_DIG, "dig.msa", 0 },
	};

	char szFileName[FILE_MAX_NUM];
	pRaceData->RegisterMotionMode(pkMotionManager.MODE_GENERAL);
	for (const auto& it : m_vecBasicMotion)
	{
		snprintf(szFileName, sizeof(szFileName), "%s%s%s", stFolder, "general/", it.stName);
		const CGraphicThing *pkMotionThing = pRaceData->RegisterMotionData(pkMotionManager.MODE_GENERAL, it.wMotionIndex, szFileName, it.byPercentage);

		if (pkMotionThing)
			CResourceManager::Instance().LoadStaticCache(pkMotionThing->GetFileName());
	}

	return true;
}

#ifdef ENABLE_WOLFMAN_CHARACTER
#ifdef USE_LOADING_DLG_OPTIMIZATION
bool CPlayerSettingsModule::LoadNewGeneralMotion(char stFolder[FILE_MAX_NUM])
#else
const bool CPlayerSettingsModule::LoadNewGeneralMotion(char stFolder[FILE_MAX_NUM])
#endif // USE_LOADING_DLG_OPTIMIZATION
{
	CRaceManager& pkManager = CRaceManager::Instance();

	CRaceData* pRaceData = pkManager.GetSelectedRaceDataPointer();
	if (!pRaceData)
		return false;

	std::vector<SMotion> m_vecBasicMotion =
	{
		{ pkMotionManager.NAME_WAIT, "wait.msa", 0 },
		{ pkMotionManager.NAME_WALK, "walk.msa", 0 },
		{ pkMotionManager.NAME_RUN, "run.msa", 0 },
		{ pkMotionManager.NAME_DAMAGE, "front_damage.msa", 50 },
		{ pkMotionManager.NAME_DAMAGE, "front_damage1.msa", 50 },
		{ pkMotionManager.NAME_DAMAGE_BACK, "back_damage.msa", 50 },
		{ pkMotionManager.NAME_DAMAGE_BACK, "back_damage1.msa", 50 },
		{ pkMotionManager.NAME_DAMAGE_FLYING, "front_damage_flying.msa", 0 },
		{ pkMotionManager.NAME_STAND_UP, "front_falling_standup.msa", 0 },
		{ pkMotionManager.NAME_DAMAGE_FLYING_BACK, "back_damage_flying.msa", 0 },
		{ pkMotionManager.NAME_STAND_UP_BACK, "back_falling_standup.msa", 0 },
		{ pkMotionManager.NAME_DEAD, "dead.msa", 0 },
		{ pkMotionManager.NAME_DIG, "dig.msa", 0 },
	};

	char szFileName[FILE_MAX_NUM];
	pRaceData->RegisterMotionMode(pkMotionManager.MODE_GENERAL);
	for (const auto& it : m_vecBasicMotion)
	{
		snprintf(szFileName, sizeof(szFileName), "%s%s%s", stFolder, "general/", it.stName);
		const CGraphicThing *pkMotionThing = pRaceData->RegisterMotionData(pkMotionManager.MODE_GENERAL, it.wMotionIndex, szFileName, it.byPercentage);

		if (pkMotionThing)
			CResourceManager::Instance().LoadStaticCache(pkMotionThing->GetFileName());
	}

	return true;
}
#endif

#ifdef USE_LOADING_DLG_OPTIMIZATION
bool CPlayerSettingsModule::LoadGameWarrior(DWORD dwRace, char stFolder[FILE_MAX_NUM])
#else
const bool CPlayerSettingsModule::LoadGameWarrior(DWORD dwRace, char stFolder[FILE_MAX_NUM])
#endif // USE_LOADING_DLG_OPTIMIZATION
{
	CRaceManager& pkManager = CRaceManager::Instance();
	pkManager.SelectRace(dwRace);

	if (!LoadGeneralMotion(stFolder))
		return false;

	CRaceData* pRaceData = pkManager.GetSelectedRaceDataPointer();
	if (!pRaceData)
		return false;

	if (!pRaceData->SetMotionRandomWeight(pkMotionManager.MODE_GENERAL, pkMotionManager.NAME_WAIT, 0, 70))
		return false;

	std::vector<SMotion> m_vecMotionGeneral =
	{
		{ pkMotionManager.NAME_WAIT, "wait_1.msa", 30 },
		{ pkMotionManager.NAME_COMBO_ATTACK_1, "attack.msa", 50 },
		{ pkMotionManager.NAME_COMBO_ATTACK_1, "attack_1.msa", 50 },
	};

	char szFileName[FILE_MAX_NUM];
	pRaceData->RegisterMotionMode(pkMotionManager.MODE_GENERAL);
	for (const auto& it : m_vecMotionGeneral)
	{
		snprintf(szFileName, sizeof(szFileName), "%s%s%s", stFolder, "general/", it.stName);
		const CGraphicThing *pkMotionThing = pRaceData->RegisterMotionData(pkMotionManager.MODE_GENERAL, it.wMotionIndex, szFileName, it.byPercentage);

		if (pkMotionThing)
			CResourceManager::Instance().LoadStaticCache(pkMotionThing->GetFileName());
	}

	std::vector<SMotion> m_vecMotionSkill =
	{
		{ GUILD_SKILL_DRAGONBLOOD, "guild_yongsinuipi.msa", 0 },
		{ GUILD_SKILL_DRAGONBLESS, "guild_yongsinuichukbok.msa", 0 },
		{ GUILD_SKILL_BLESSARMOR, "guild_seonghwigap.msa", 0 },
		{ GUILD_SKILL_SPPEDUP, "guild_gasokhwa.msa", 0 },
		{ GUILD_SKILL_DRAGONWRATH, "guild_yongsinuibunno.msa", 0 },
		{ GUILD_SKILL_MAGICUP, "guild_jumunsul.msa", 0 },
	};

	std::vector<std::pair<BYTE, const char*>> m_vecSkillName = {
		{ 1, "samyeon" },
		{ 2, "palbang" },
		{ 3, "jeongwi" },
		{ 4, "geomgyeong" },
		{ 5, "tanhwan" },
		{ 6, "gihyeol" },
		{ 16, "gigongcham" },
		{ 17, "gyeoksan" },
		{ 18, "daejin" },
		{ 19, "cheongeun" },
		{ 20, "geompung" },
		{ 21, "noegeom" },
	};

	char szSkillAdd[4] = "";
	for (WORD i = 0; i < CPythonSkill::SKILL_EFFECT_COUNT; i++)
	{
		for (auto& it : m_vecSkillName)
		{
			SMotion motion;

			motion.wMotionIndex = pkMotionManager.NAME_SKILL + (i * CPythonSkill::SKILL_GRADEGAP) + it.first;
			motion.byPercentage = 0;

			if (i == 4 || i == 5 || i == 6) // ENABLE_EXPERT_SKILL
				snprintf(szSkillAdd, sizeof(szSkillAdd), "_4");
			else if (i > 0)
				snprintf(szSkillAdd, sizeof(szSkillAdd), "_%d", i + 1);
			else
				snprintf(szSkillAdd, sizeof(szSkillAdd), "");

			snprintf(motion.stName, sizeof(motion.stName), "%s%s.msa", it.second, szSkillAdd);
			m_vecMotionSkill.push_back(motion);
		}
	}

	pRaceData->RegisterMotionMode(pkMotionManager.MODE_GENERAL);
	for (const auto& it : m_vecMotionSkill)
	{
		snprintf(szFileName, sizeof(szFileName), "%s%s%s", stFolder, "skill/", it.stName);
		const CGraphicThing *pkMotionThing = pRaceData->RegisterMotionData(pkMotionManager.MODE_GENERAL, it.wMotionIndex, szFileName, it.byPercentage);
	
		if (pkMotionThing)
			CResourceManager::Instance().LoadStaticCache(pkMotionThing->GetFileName());
	}

	pRaceData->ReserveComboAttack(pkMotionManager.MODE_GENERAL, COMBO_TYPE_1, 1);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_GENERAL, COMBO_TYPE_1, COMBO_INDEX_1, pkMotionManager.NAME_COMBO_ATTACK_1);

	RegisterEmotionAnis(stFolder);

	std::vector<SMotion> m_vecMotionSword =
	{
		{ pkMotionManager.NAME_WAIT, "wait.msa", 50 },
		{ pkMotionManager.NAME_WAIT, "wait_1.msa", 50 },
		{ pkMotionManager.NAME_WALK, "walk.msa", 0 },
		{ pkMotionManager.NAME_RUN, "run.msa", 0 },
		{ pkMotionManager.NAME_DAMAGE, "damage.msa", 50 },
		{ pkMotionManager.NAME_DAMAGE, "damage_1.msa", 50 },
		{ pkMotionManager.NAME_DAMAGE_BACK, "damage_2.msa", 50 },
		{ pkMotionManager.NAME_DAMAGE_BACK, "damage_3.msa", 50 },
		{ pkMotionManager.NAME_COMBO_ATTACK_1, "combo_01.msa", 0 },
		{ pkMotionManager.NAME_COMBO_ATTACK_2, "combo_02.msa", 0 },
		{ pkMotionManager.NAME_COMBO_ATTACK_3, "combo_03.msa", 0 },
		{ pkMotionManager.NAME_COMBO_ATTACK_4, "combo_04.msa", 0 },
		{ pkMotionManager.NAME_COMBO_ATTACK_5, "combo_05.msa" , 0},
		{ pkMotionManager.NAME_COMBO_ATTACK_6, "combo_06.msa", 0 },
		{ pkMotionManager.NAME_COMBO_ATTACK_7, "combo_07.msa", 0 },
	};

	pRaceData->RegisterMotionMode(pkMotionManager.MODE_ONEHAND_SWORD);
	for (const auto& it : m_vecMotionSword)
	{
		snprintf(szFileName, sizeof(szFileName), "%s%s%s", stFolder, "onehand_sword/", it.stName);
		const CGraphicThing *pkMotionThing = pRaceData->RegisterMotionData(pkMotionManager.MODE_ONEHAND_SWORD, it.wMotionIndex, szFileName, it.byPercentage);

		if (pkMotionThing)
			CResourceManager::Instance().LoadStaticCache(pkMotionThing->GetFileName());
	}

	pRaceData->ReserveComboAttack(pkMotionManager.MODE_ONEHAND_SWORD, COMBO_TYPE_1, 4);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_ONEHAND_SWORD, COMBO_TYPE_1, COMBO_INDEX_1, pkMotionManager.NAME_COMBO_ATTACK_1);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_ONEHAND_SWORD, COMBO_TYPE_1, COMBO_INDEX_2, pkMotionManager.NAME_COMBO_ATTACK_2);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_ONEHAND_SWORD, COMBO_TYPE_1, COMBO_INDEX_3, pkMotionManager.NAME_COMBO_ATTACK_3);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_ONEHAND_SWORD, COMBO_TYPE_1, COMBO_INDEX_4, pkMotionManager.NAME_COMBO_ATTACK_4);

	pRaceData->ReserveComboAttack(pkMotionManager.MODE_ONEHAND_SWORD, COMBO_TYPE_2, 5);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_ONEHAND_SWORD, COMBO_TYPE_2, COMBO_INDEX_1, pkMotionManager.NAME_COMBO_ATTACK_1);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_ONEHAND_SWORD, COMBO_TYPE_2, COMBO_INDEX_2, pkMotionManager.NAME_COMBO_ATTACK_2);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_ONEHAND_SWORD, COMBO_TYPE_2, COMBO_INDEX_3, pkMotionManager.NAME_COMBO_ATTACK_3);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_ONEHAND_SWORD, COMBO_TYPE_2, COMBO_INDEX_4, pkMotionManager.NAME_COMBO_ATTACK_5);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_ONEHAND_SWORD, COMBO_TYPE_2, COMBO_INDEX_5, pkMotionManager.NAME_COMBO_ATTACK_7);

	pRaceData->ReserveComboAttack(pkMotionManager.MODE_ONEHAND_SWORD, COMBO_TYPE_3, 6);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_ONEHAND_SWORD, COMBO_TYPE_3, COMBO_INDEX_1, pkMotionManager.NAME_COMBO_ATTACK_1);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_ONEHAND_SWORD, COMBO_TYPE_3, COMBO_INDEX_2, pkMotionManager.NAME_COMBO_ATTACK_2);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_ONEHAND_SWORD, COMBO_TYPE_3, COMBO_INDEX_3, pkMotionManager.NAME_COMBO_ATTACK_3);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_ONEHAND_SWORD, COMBO_TYPE_3, COMBO_INDEX_4, pkMotionManager.NAME_COMBO_ATTACK_5);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_ONEHAND_SWORD, COMBO_TYPE_3, COMBO_INDEX_5, pkMotionManager.NAME_COMBO_ATTACK_6);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_ONEHAND_SWORD, COMBO_TYPE_3, COMBO_INDEX_6, pkMotionManager.NAME_COMBO_ATTACK_4);

	std::vector<SMotion> m_vecMotionTwoHandSword =
	{
		{ pkMotionManager.NAME_WAIT, "wait.msa", 70 },
		{ pkMotionManager.NAME_WAIT, "wait_1.msa", 30 },
		{ pkMotionManager.NAME_WALK, "walk.msa", 0 },
		{ pkMotionManager.NAME_RUN, "run.msa", 0 },
		{ pkMotionManager.NAME_DAMAGE, "damage.msa", 50 },
		{ pkMotionManager.NAME_DAMAGE, "damage_1.msa", 50 },
		{ pkMotionManager.NAME_DAMAGE_BACK, "damage_2.msa", 50 },
		{ pkMotionManager.NAME_DAMAGE_BACK, "damage_3.msa", 50 },
		{ pkMotionManager.NAME_COMBO_ATTACK_1, "combo_01.msa", 0 },
		{ pkMotionManager.NAME_COMBO_ATTACK_2, "combo_02.msa", 0 },
		{ pkMotionManager.NAME_COMBO_ATTACK_3, "combo_03.msa", 0 },
		{ pkMotionManager.NAME_COMBO_ATTACK_4, "combo_04.msa", 0 },
		{ pkMotionManager.NAME_COMBO_ATTACK_5, "combo_05.msa", 0 },
		{ pkMotionManager.NAME_COMBO_ATTACK_6, "combo_06.msa", 0 },
		{ pkMotionManager.NAME_COMBO_ATTACK_7, "combo_07.msa", 0 },
	};

	pRaceData->RegisterMotionMode(pkMotionManager.MODE_TWOHAND_SWORD);
	for (const auto& it : m_vecMotionSword)
	{
		snprintf(szFileName, sizeof(szFileName), "%s%s%s", stFolder, "twohand_sword/", it.stName);
		const CGraphicThing *pkMotionThing = pRaceData->RegisterMotionData(pkMotionManager.MODE_TWOHAND_SWORD, it.wMotionIndex, szFileName, it.byPercentage);

		if (pkMotionThing)
			CResourceManager::Instance().LoadStaticCache(pkMotionThing->GetFileName());
	}

	pRaceData->ReserveComboAttack(pkMotionManager.MODE_TWOHAND_SWORD, COMBO_TYPE_1, 4);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_TWOHAND_SWORD, COMBO_TYPE_1, COMBO_INDEX_1, pkMotionManager.NAME_COMBO_ATTACK_1);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_TWOHAND_SWORD, COMBO_TYPE_1, COMBO_INDEX_2, pkMotionManager.NAME_COMBO_ATTACK_2);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_TWOHAND_SWORD, COMBO_TYPE_1, COMBO_INDEX_3, pkMotionManager.NAME_COMBO_ATTACK_3);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_TWOHAND_SWORD, COMBO_TYPE_1, COMBO_INDEX_4, pkMotionManager.NAME_COMBO_ATTACK_4);

	pRaceData->ReserveComboAttack(pkMotionManager.MODE_TWOHAND_SWORD, COMBO_TYPE_2, 5);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_TWOHAND_SWORD, COMBO_TYPE_2, COMBO_INDEX_1, pkMotionManager.NAME_COMBO_ATTACK_1);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_TWOHAND_SWORD, COMBO_TYPE_2, COMBO_INDEX_2, pkMotionManager.NAME_COMBO_ATTACK_2);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_TWOHAND_SWORD, COMBO_TYPE_2, COMBO_INDEX_3, pkMotionManager.NAME_COMBO_ATTACK_3);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_TWOHAND_SWORD, COMBO_TYPE_2, COMBO_INDEX_4, pkMotionManager.NAME_COMBO_ATTACK_5);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_TWOHAND_SWORD, COMBO_TYPE_2, COMBO_INDEX_5, pkMotionManager.NAME_COMBO_ATTACK_7);

	pRaceData->ReserveComboAttack(pkMotionManager.MODE_TWOHAND_SWORD, COMBO_TYPE_3, 6);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_TWOHAND_SWORD, COMBO_TYPE_3, COMBO_INDEX_1, pkMotionManager.NAME_COMBO_ATTACK_1);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_TWOHAND_SWORD, COMBO_TYPE_3, COMBO_INDEX_2, pkMotionManager.NAME_COMBO_ATTACK_2);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_TWOHAND_SWORD, COMBO_TYPE_3, COMBO_INDEX_3, pkMotionManager.NAME_COMBO_ATTACK_3);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_TWOHAND_SWORD, COMBO_TYPE_3, COMBO_INDEX_4, pkMotionManager.NAME_COMBO_ATTACK_5);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_TWOHAND_SWORD, COMBO_TYPE_3, COMBO_INDEX_5, pkMotionManager.NAME_COMBO_ATTACK_6);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_TWOHAND_SWORD, COMBO_TYPE_3, COMBO_INDEX_6, pkMotionManager.NAME_COMBO_ATTACK_4);

	std::vector<SMotion> m_vecMotionFishing =
	{
		{ pkMotionManager.NAME_WAIT, "wait.msa", 0 },
		{ pkMotionManager.NAME_WALK, "walk.msa", 0 },
		{ pkMotionManager.NAME_RUN, "run.msa", 0 },
		{ pkMotionManager.NAME_FISHING_THROW, "throw.msa", 0 },
		{ pkMotionManager.NAME_FISHING_WAIT, "fishing_wait.msa", 0 },
		{ pkMotionManager.NAME_FISHING_STOP, "fishing_cancel.msa", 0 },
		{ pkMotionManager.NAME_FISHING_REACT, "fishing_react.msa", 0 },
		{ pkMotionManager.NAME_FISHING_CATCH, "fishing_catch.msa", 0 },
		{ pkMotionManager.NAME_FISHING_FAIL, "fishing_fail.msa", 0 },
	};

	pRaceData->RegisterMotionMode(pkMotionManager.MODE_FISHING);
	for (const auto& it : m_vecMotionFishing)
	{
		snprintf(szFileName, sizeof(szFileName), "%s%s%s", stFolder, "fishing/", it.stName);
		const CGraphicThing *pkMotionThing = pRaceData->RegisterMotionData(pkMotionManager.MODE_FISHING, it.wMotionIndex, szFileName, it.byPercentage);

		if (pkMotionThing)
			CResourceManager::Instance().LoadStaticCache(pkMotionThing->GetFileName());
	}

	std::vector<SMotion> m_vecMotionHorse =
	{
		{ pkMotionManager.NAME_WAIT, "wait.msa", 90 },
		{ pkMotionManager.NAME_WAIT, "wait_1.msa", 9 },
		{ pkMotionManager.NAME_WAIT, "wait_2.msa", 1 },
		{ pkMotionManager.NAME_WALK, "walk.msa", 0 },
		{ pkMotionManager.NAME_RUN, "run.msa", 0 },
		{ pkMotionManager.NAME_DAMAGE, "damage.msa", 0 },
		{ pkMotionManager.NAME_DAMAGE_BACK, "damage.msa", 0 },
		{ pkMotionManager.NAME_DEAD, "dead.msa", 0 },
		{ HORSE_SKILL_CHARGE, "skill_charge.msa", 0 },
	};

	pRaceData->RegisterMotionMode(pkMotionManager.MODE_HORSE);
	for (const auto& it : m_vecMotionHorse)
	{
		snprintf(szFileName, sizeof(szFileName), "%s%s%s", stFolder, "horse/", it.stName);
		const CGraphicThing *pkMotionThing = pRaceData->RegisterMotionData(pkMotionManager.MODE_HORSE, it.wMotionIndex, szFileName, it.byPercentage);

		if (pkMotionThing)
			CResourceManager::Instance().LoadStaticCache(pkMotionThing->GetFileName());
	}

	std::vector<SMotion> m_vecMotionHorseSword =
	{
		{ pkMotionManager.NAME_COMBO_ATTACK_1, "combo_01.msa", 0 },
		{ pkMotionManager.NAME_COMBO_ATTACK_2, "combo_02.msa", 0 },
		{ pkMotionManager.NAME_COMBO_ATTACK_3, "combo_03.msa", 0 },
		{ HORSE_SKILL_WILDATTACK, "skill_wildattack.msa", 0 },
		{ HORSE_SKILL_SPLASH, "skill_splash.msa", 0 },
	};

	pRaceData->RegisterMotionMode(pkMotionManager.MODE_HORSE_ONEHAND_SWORD);
	for (const auto& it : m_vecMotionHorseSword)
	{
		snprintf(szFileName, sizeof(szFileName), "%s%s%s", stFolder, "horse_onehand_sword/", it.stName);
		const CGraphicThing *pkMotionThing = pRaceData->RegisterMotionData(pkMotionManager.MODE_HORSE_ONEHAND_SWORD, it.wMotionIndex, szFileName, it.byPercentage);

		if (pkMotionThing)
			CResourceManager::Instance().LoadStaticCache(pkMotionThing->GetFileName());
	}

	pRaceData->ReserveComboAttack(pkMotionManager.MODE_HORSE_ONEHAND_SWORD, COMBO_TYPE_1, 3);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_HORSE_ONEHAND_SWORD, COMBO_TYPE_1, COMBO_INDEX_1, pkMotionManager.NAME_COMBO_ATTACK_1);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_HORSE_ONEHAND_SWORD, COMBO_TYPE_1, COMBO_INDEX_2, pkMotionManager.NAME_COMBO_ATTACK_2);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_HORSE_ONEHAND_SWORD, COMBO_TYPE_1, COMBO_INDEX_3, pkMotionManager.NAME_COMBO_ATTACK_3);

	std::vector<SMotion> m_vecMotionHorseTwoHandSword =
	{
		{ pkMotionManager.NAME_COMBO_ATTACK_1, "combo_01.msa", 0 },
		{ pkMotionManager.NAME_COMBO_ATTACK_2, "combo_02.msa", 0 },
		{ pkMotionManager.NAME_COMBO_ATTACK_3, "combo_03.msa", 0 },
		{ HORSE_SKILL_WILDATTACK, "skill_wildattack.msa", 0 },
		{ HORSE_SKILL_SPLASH, "skill_splash.msa", 0 },
	};

	pRaceData->RegisterMotionMode(pkMotionManager.MODE_HORSE_TWOHAND_SWORD);
	for (const auto& it : m_vecMotionHorseTwoHandSword)
	{
		snprintf(szFileName, sizeof(szFileName), "%s%s%s", stFolder, "horse_twohand_sword/", it.stName);
		const CGraphicThing *pkMotionThing = pRaceData->RegisterMotionData(pkMotionManager.MODE_HORSE_TWOHAND_SWORD, it.wMotionIndex, szFileName, it.byPercentage);

		if (pkMotionThing)
			CResourceManager::Instance().LoadStaticCache(pkMotionThing->GetFileName());
	}

	pRaceData->ReserveComboAttack(pkMotionManager.MODE_HORSE_TWOHAND_SWORD, COMBO_TYPE_1, 3);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_HORSE_TWOHAND_SWORD, COMBO_TYPE_1, COMBO_INDEX_1, pkMotionManager.NAME_COMBO_ATTACK_1);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_HORSE_TWOHAND_SWORD, COMBO_TYPE_1, COMBO_INDEX_2, pkMotionManager.NAME_COMBO_ATTACK_2);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_HORSE_TWOHAND_SWORD, COMBO_TYPE_1, COMBO_INDEX_3, pkMotionManager.NAME_COMBO_ATTACK_3);

	pRaceData->RegisterAttachingBoneName(CRaceData::PART_WEAPON, "equip_right_hand");

#ifdef ENABLE_ACCE_SYSTEM
	pRaceData->RegisterAttachingBoneName(CRaceData::PART_ACCE, "Bip01 Spine2");
#endif

	return true;
}

#ifdef USE_LOADING_DLG_OPTIMIZATION
bool CPlayerSettingsModule::LoadGameAssassin(DWORD dwRace, char stFolder[FILE_MAX_NUM])
#else
const bool CPlayerSettingsModule::LoadGameAssassin(DWORD dwRace, char stFolder[FILE_MAX_NUM])
#endif // USE_LOADING_DLG_OPTIMIZATION
{
	CRaceManager& pkManager = CRaceManager::Instance();
	pkManager.SelectRace(dwRace);

	if (!LoadGeneralMotion(stFolder))
		return false;

	CRaceData* pRaceData = pkManager.GetSelectedRaceDataPointer();
	if (!pRaceData)
		return false;

	if (!pRaceData->SetMotionRandomWeight(pkMotionManager.MODE_GENERAL, pkMotionManager.NAME_WAIT, 0, 70))
		return false;

	std::vector<SMotion> m_vecMotionGeneral =
	{
		{ pkMotionManager.NAME_WAIT, "wait_1.msa", 30 },
		{ pkMotionManager.NAME_COMBO_ATTACK_1, "attack.msa", 50 },
		{ pkMotionManager.NAME_COMBO_ATTACK_1, "attack_1.msa", 50 },
	};

	char szFileName[FILE_MAX_NUM];
	pRaceData->RegisterMotionMode(pkMotionManager.MODE_GENERAL);
	for (const auto& it : m_vecMotionGeneral)
	{
		snprintf(szFileName, sizeof(szFileName), "%s%s%s", stFolder, "general/", it.stName);
		const CGraphicThing *pkMotionThing = pRaceData->RegisterMotionData(pkMotionManager.MODE_GENERAL, it.wMotionIndex, szFileName, it.byPercentage);

		if (pkMotionThing)
			CResourceManager::Instance().LoadStaticCache(pkMotionThing->GetFileName());
	}

	std::vector<SMotion> m_vecMotionSkill =
	{
		{ GUILD_SKILL_DRAGONBLOOD, "guild_yongsinuipi.msa", 0 },
		{ GUILD_SKILL_DRAGONBLESS, "guild_yongsinuichukbok.msa", 0 },
		{ GUILD_SKILL_BLESSARMOR, "guild_seonghwigap.msa", 0 },
		{ GUILD_SKILL_SPPEDUP, "guild_gasokhwa.msa", 0 },
		{ GUILD_SKILL_DRAGONWRATH, "guild_yongsinuibunno.msa", 0 },
		{ GUILD_SKILL_MAGICUP, "guild_jumunsul.msa", 0 },
	};

	std::vector<std::pair<BYTE, const char*>> m_vecSkillName = {
		{ 1, "amseup" },
		{ 2, "gungsin" },
		{ 3, "charyun" },
		{ 4, "eunhyeong" },
		{ 5, "sangong" },
		{ 6, "seomjeon" },
		{ 16, "yeonsa" },
		{ 17, "gwangyeok" },
		{ 18, "hwajo" },
		{ 19, "gyeonggong" },
		{ 20, "dokgigung" },
		{ 21, "seomgwang" },
	};

	char szSkillAdd[4] = "";
	for (WORD i = 0; i < CPythonSkill::SKILL_EFFECT_COUNT; i++)
	{
		for (auto& it : m_vecSkillName)
		{
			SMotion motion;

			motion.wMotionIndex = pkMotionManager.NAME_SKILL + (i * CPythonSkill::SKILL_GRADEGAP) + it.first;
			motion.byPercentage = 0;

			if (i == 4 || i == 5 || i == 6) // ENABLE_EXPERT_SKILL
				snprintf(szSkillAdd, sizeof(szSkillAdd), "_4");
			else if (i > 0)
				snprintf(szSkillAdd, sizeof(szSkillAdd), "_%d", i + 1);
			else
				snprintf(szSkillAdd, sizeof(szSkillAdd), "");

			snprintf(motion.stName, sizeof(motion.stName), "%s%s.msa", it.second, szSkillAdd);
			m_vecMotionSkill.push_back(motion);
		}
	}

	pRaceData->RegisterMotionMode(pkMotionManager.MODE_GENERAL);
	for (const auto& it : m_vecMotionSkill)
	{
		snprintf(szFileName, sizeof(szFileName), "%s%s%s", stFolder, "skill/", it.stName);
		const CGraphicThing *pkMotionThing = pRaceData->RegisterMotionData(pkMotionManager.MODE_GENERAL, it.wMotionIndex, szFileName, it.byPercentage);

		if (pkMotionThing)
			CResourceManager::Instance().LoadStaticCache(pkMotionThing->GetFileName());
	}

	pRaceData->ReserveComboAttack(pkMotionManager.MODE_GENERAL, COMBO_TYPE_1, 1);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_GENERAL, COMBO_TYPE_1, COMBO_INDEX_1, pkMotionManager.NAME_COMBO_ATTACK_1);

	RegisterEmotionAnis(stFolder);

	std::vector<SMotion> m_vecMotionSword =
	{
		{ pkMotionManager.NAME_WAIT, "wait.msa", 70 },
		{ pkMotionManager.NAME_WAIT, "wait_1.msa", 30 },
		{ pkMotionManager.NAME_WALK, "walk.msa" , 0 },
		{ pkMotionManager.NAME_RUN, "run.msa" , 0 },
		{ pkMotionManager.NAME_DAMAGE, "damage.msa", 50 },
		{ pkMotionManager.NAME_DAMAGE, "damage_1.msa", 50 },
		{ pkMotionManager.NAME_DAMAGE_BACK, "damage_2.msa", 50 },
		{ pkMotionManager.NAME_DAMAGE_BACK, "damage_3.msa", 50 },
		{ pkMotionManager.NAME_COMBO_ATTACK_1, "combo_01.msa" , 0 },
		{ pkMotionManager.NAME_COMBO_ATTACK_2, "combo_02.msa" , 0 },
		{ pkMotionManager.NAME_COMBO_ATTACK_3, "combo_03.msa" , 0 },
		{ pkMotionManager.NAME_COMBO_ATTACK_4, "combo_04.msa" , 0 },
		{ pkMotionManager.NAME_COMBO_ATTACK_5, "combo_05.msa" , 0 },
		{ pkMotionManager.NAME_COMBO_ATTACK_6, "combo_06.msa" , 0 },
		{ pkMotionManager.NAME_COMBO_ATTACK_7, "combo_07.msa" , 0 },
	};

	pRaceData->RegisterMotionMode(pkMotionManager.MODE_ONEHAND_SWORD);
	for (const auto& it : m_vecMotionSword)
	{
		snprintf(szFileName, sizeof(szFileName), "%s%s%s", stFolder, "onehand_sword/", it.stName);
		const CGraphicThing *pkMotionThing = pRaceData->RegisterMotionData(pkMotionManager.MODE_ONEHAND_SWORD, it.wMotionIndex, szFileName, it.byPercentage);

		if (pkMotionThing)
			CResourceManager::Instance().LoadStaticCache(pkMotionThing->GetFileName());
	}

	pRaceData->ReserveComboAttack(pkMotionManager.MODE_ONEHAND_SWORD, COMBO_TYPE_1, 4);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_ONEHAND_SWORD, COMBO_TYPE_1, COMBO_INDEX_1, pkMotionManager.NAME_COMBO_ATTACK_1);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_ONEHAND_SWORD, COMBO_TYPE_1, COMBO_INDEX_2, pkMotionManager.NAME_COMBO_ATTACK_2);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_ONEHAND_SWORD, COMBO_TYPE_1, COMBO_INDEX_3, pkMotionManager.NAME_COMBO_ATTACK_3);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_ONEHAND_SWORD, COMBO_TYPE_1, COMBO_INDEX_4, pkMotionManager.NAME_COMBO_ATTACK_4);

	pRaceData->ReserveComboAttack(pkMotionManager.MODE_ONEHAND_SWORD, COMBO_TYPE_2, 5);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_ONEHAND_SWORD, COMBO_TYPE_2, COMBO_INDEX_1, pkMotionManager.NAME_COMBO_ATTACK_1);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_ONEHAND_SWORD, COMBO_TYPE_2, COMBO_INDEX_2, pkMotionManager.NAME_COMBO_ATTACK_2);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_ONEHAND_SWORD, COMBO_TYPE_2, COMBO_INDEX_3, pkMotionManager.NAME_COMBO_ATTACK_3);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_ONEHAND_SWORD, COMBO_TYPE_2, COMBO_INDEX_4, pkMotionManager.NAME_COMBO_ATTACK_5);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_ONEHAND_SWORD, COMBO_TYPE_2, COMBO_INDEX_5, pkMotionManager.NAME_COMBO_ATTACK_7);

	pRaceData->ReserveComboAttack(pkMotionManager.MODE_ONEHAND_SWORD, COMBO_TYPE_3, 6);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_ONEHAND_SWORD, COMBO_TYPE_3, COMBO_INDEX_1, pkMotionManager.NAME_COMBO_ATTACK_1);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_ONEHAND_SWORD, COMBO_TYPE_3, COMBO_INDEX_2, pkMotionManager.NAME_COMBO_ATTACK_2);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_ONEHAND_SWORD, COMBO_TYPE_3, COMBO_INDEX_3, pkMotionManager.NAME_COMBO_ATTACK_3);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_ONEHAND_SWORD, COMBO_TYPE_3, COMBO_INDEX_4, pkMotionManager.NAME_COMBO_ATTACK_5);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_ONEHAND_SWORD, COMBO_TYPE_3, COMBO_INDEX_5, pkMotionManager.NAME_COMBO_ATTACK_6);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_ONEHAND_SWORD, COMBO_TYPE_3, COMBO_INDEX_6, pkMotionManager.NAME_COMBO_ATTACK_4);

	std::vector<SMotion> m_vecMotionDagger =
	{
		{ pkMotionManager.NAME_WAIT, "wait.msa", 70 },
		{ pkMotionManager.NAME_WAIT, "wait_1.msa", 30 },
		{ pkMotionManager.NAME_WALK, "walk.msa", 0 },
		{ pkMotionManager.NAME_RUN, "run.msa", 0 },
		{ pkMotionManager.NAME_DAMAGE, "damage.msa", 50 },
		{ pkMotionManager.NAME_DAMAGE, "damage_1.msa", 50 },
		{ pkMotionManager.NAME_DAMAGE_BACK, "damage_2.msa", 50 },
		{ pkMotionManager.NAME_DAMAGE_BACK, "damage_3.msa", 50 },
		{ pkMotionManager.NAME_COMBO_ATTACK_1, "combo_01.msa", 0 },
		{ pkMotionManager.NAME_COMBO_ATTACK_2, "combo_02.msa", 0 },
		{ pkMotionManager.NAME_COMBO_ATTACK_3, "combo_03.msa", 0 },
		{ pkMotionManager.NAME_COMBO_ATTACK_4, "combo_04.msa", 0 },
		{ pkMotionManager.NAME_COMBO_ATTACK_5, "combo_05.msa", 0 },
		{ pkMotionManager.NAME_COMBO_ATTACK_6, "combo_06.msa", 0 },
		{ pkMotionManager.NAME_COMBO_ATTACK_7, "combo_07.msa", 0 },
		{ pkMotionManager.NAME_COMBO_ATTACK_8, "combo_08.msa", 0 },
	};

	pRaceData->RegisterMotionMode(pkMotionManager.MODE_DUALHAND_SWORD);
	for (const auto& it : m_vecMotionDagger)
	{
		snprintf(szFileName, sizeof(szFileName), "%s%s%s", stFolder, "dualhand_sword/", it.stName);
		const CGraphicThing *pkMotionThing = pRaceData->RegisterMotionData(pkMotionManager.MODE_DUALHAND_SWORD, it.wMotionIndex, szFileName, it.byPercentage);

		if (pkMotionThing)
			CResourceManager::Instance().LoadStaticCache(pkMotionThing->GetFileName());
	}

	pRaceData->ReserveComboAttack(pkMotionManager.MODE_DUALHAND_SWORD, COMBO_TYPE_1, 4);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_DUALHAND_SWORD, COMBO_TYPE_1, COMBO_INDEX_1, pkMotionManager.NAME_COMBO_ATTACK_1);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_DUALHAND_SWORD, COMBO_TYPE_1, COMBO_INDEX_2, pkMotionManager.NAME_COMBO_ATTACK_2);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_DUALHAND_SWORD, COMBO_TYPE_1, COMBO_INDEX_3, pkMotionManager.NAME_COMBO_ATTACK_3);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_DUALHAND_SWORD, COMBO_TYPE_1, COMBO_INDEX_4, pkMotionManager.NAME_COMBO_ATTACK_4);

	pRaceData->ReserveComboAttack(pkMotionManager.MODE_DUALHAND_SWORD, COMBO_TYPE_2, 5);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_DUALHAND_SWORD, COMBO_TYPE_2, COMBO_INDEX_1, pkMotionManager.NAME_COMBO_ATTACK_1);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_DUALHAND_SWORD, COMBO_TYPE_2, COMBO_INDEX_2, pkMotionManager.NAME_COMBO_ATTACK_2);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_DUALHAND_SWORD, COMBO_TYPE_2, COMBO_INDEX_3, pkMotionManager.NAME_COMBO_ATTACK_3);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_DUALHAND_SWORD, COMBO_TYPE_2, COMBO_INDEX_4, pkMotionManager.NAME_COMBO_ATTACK_5);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_DUALHAND_SWORD, COMBO_TYPE_2, COMBO_INDEX_5, pkMotionManager.NAME_COMBO_ATTACK_7);

	pRaceData->ReserveComboAttack(pkMotionManager.MODE_DUALHAND_SWORD, COMBO_TYPE_3, 6);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_DUALHAND_SWORD, COMBO_TYPE_3, COMBO_INDEX_1, pkMotionManager.NAME_COMBO_ATTACK_1);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_DUALHAND_SWORD, COMBO_TYPE_3, COMBO_INDEX_2, pkMotionManager.NAME_COMBO_ATTACK_2);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_DUALHAND_SWORD, COMBO_TYPE_3, COMBO_INDEX_3, pkMotionManager.NAME_COMBO_ATTACK_3);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_DUALHAND_SWORD, COMBO_TYPE_3, COMBO_INDEX_4, pkMotionManager.NAME_COMBO_ATTACK_5);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_DUALHAND_SWORD, COMBO_TYPE_3, COMBO_INDEX_5, pkMotionManager.NAME_COMBO_ATTACK_6);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_DUALHAND_SWORD, COMBO_TYPE_3, COMBO_INDEX_6, pkMotionManager.NAME_COMBO_ATTACK_8);

	std::vector<SMotion> m_vecMotionBow =
	{
		{ pkMotionManager.NAME_WAIT, "wait.msa", 70 },
		{ pkMotionManager.NAME_WAIT, "wait_1.msa", 30},
		{ pkMotionManager.NAME_WALK, "walk.msa", 0 },
		{ pkMotionManager.NAME_RUN, "run.msa", 0 },
		{ pkMotionManager.NAME_DAMAGE, "damage.msa", 50},
		{ pkMotionManager.NAME_DAMAGE, "damage_1.msa", 50},
		{ pkMotionManager.NAME_DAMAGE_BACK, "damage_2.msa", 50},
		{ pkMotionManager.NAME_DAMAGE_BACK, "damage_3.msa", 50},
		{ pkMotionManager.NAME_COMBO_ATTACK_1, "attack.msa", 0 },
	};

	pRaceData->RegisterMotionMode(pkMotionManager.MODE_BOW);
	for (const auto& it : m_vecMotionBow)
	{
		snprintf(szFileName, sizeof(szFileName), "%s%s%s", stFolder, "bow/", it.stName);
		const CGraphicThing *pkMotionThing = pRaceData->RegisterMotionData(pkMotionManager.MODE_BOW, it.wMotionIndex, szFileName, it.byPercentage);

		if (pkMotionThing)
			CResourceManager::Instance().LoadStaticCache(pkMotionThing->GetFileName());
	}

	pRaceData->ReserveComboAttack(pkMotionManager.MODE_BOW, COMBO_TYPE_1, 1);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_BOW, COMBO_TYPE_1, COMBO_INDEX_1, pkMotionManager.NAME_COMBO_ATTACK_1);

	std::vector<SMotion> m_vecMotionFishing =
	{
		{ pkMotionManager.NAME_WAIT, "wait.msa", 0 },
		{ pkMotionManager.NAME_WALK, "walk.msa", 0 },
		{ pkMotionManager.NAME_RUN, "run.msa", 0 },
		{ pkMotionManager.NAME_FISHING_THROW, "throw.msa", 0 },
		{ pkMotionManager.NAME_FISHING_WAIT, "fishing_wait.msa", 0 },
		{ pkMotionManager.NAME_FISHING_STOP, "fishing_cancel.msa", 0 },
		{ pkMotionManager.NAME_FISHING_REACT, "fishing_react.msa", 0 },
		{ pkMotionManager.NAME_FISHING_CATCH, "fishing_catch.msa", 0 },
		{ pkMotionManager.NAME_FISHING_FAIL, "fishing_fail.msa", 0 },
	};

	pRaceData->RegisterMotionMode(pkMotionManager.MODE_FISHING);
	for (const auto& it : m_vecMotionFishing)
	{
		snprintf(szFileName, sizeof(szFileName), "%s%s%s", stFolder, "fishing/", it.stName);
		const CGraphicThing *pkMotionThing = pRaceData->RegisterMotionData(pkMotionManager.MODE_FISHING, it.wMotionIndex, szFileName, it.byPercentage);

		if (pkMotionThing)
			CResourceManager::Instance().LoadStaticCache(pkMotionThing->GetFileName());
	}

	std::vector<SMotion> m_vecMotionHorse =
	{
		{ pkMotionManager.NAME_WAIT, "wait.msa", 90 },
		{ pkMotionManager.NAME_WAIT, "wait_1.msa", 9 },
		{ pkMotionManager.NAME_WAIT, "wait_2.msa", 1 },
		{ pkMotionManager.NAME_WALK, "walk.msa", 0 },
		{ pkMotionManager.NAME_RUN, "run.msa", 0 },
		{ pkMotionManager.NAME_DAMAGE, "damage.msa", 0 },
		{ pkMotionManager.NAME_DAMAGE_BACK, "damage.msa", 0 },
		{ pkMotionManager.NAME_DEAD, "dead.msa", 0 },
		{ HORSE_SKILL_CHARGE, "skill_charge.msa", 0 },
	};

	pRaceData->RegisterMotionMode(pkMotionManager.MODE_HORSE);
	for (const auto& it : m_vecMotionHorse)
	{
		snprintf(szFileName, sizeof(szFileName), "%s%s%s", stFolder, "horse/", it.stName);
		const CGraphicThing *pkMotionThing = pRaceData->RegisterMotionData(pkMotionManager.MODE_HORSE, it.wMotionIndex, szFileName, it.byPercentage);

		if (pkMotionThing)
			CResourceManager::Instance().LoadStaticCache(pkMotionThing->GetFileName());
	}

	std::vector<SMotion> m_vecMotionHorseSword =
	{
		{ pkMotionManager.NAME_COMBO_ATTACK_1, "combo_01.msa", 0 },
		{ pkMotionManager.NAME_COMBO_ATTACK_2, "combo_02.msa", 0 },
		{ pkMotionManager.NAME_COMBO_ATTACK_3, "combo_03.msa", 0 },
		{ HORSE_SKILL_WILDATTACK, "skill_wildattack.msa", 0 },
		{ HORSE_SKILL_SPLASH, "skill_splash.msa", 0 },
	};

	pRaceData->RegisterMotionMode(pkMotionManager.MODE_HORSE_ONEHAND_SWORD);
	for (const auto& it : m_vecMotionHorseSword)
	{
		snprintf(szFileName, sizeof(szFileName), "%s%s%s", stFolder, "horse_onehand_sword/", it.stName);
		const CGraphicThing *pkMotionThing = pRaceData->RegisterMotionData(pkMotionManager.MODE_HORSE_ONEHAND_SWORD, it.wMotionIndex, szFileName, it.byPercentage);

		if (pkMotionThing)
			CResourceManager::Instance().LoadStaticCache(pkMotionThing->GetFileName());
	}

	pRaceData->ReserveComboAttack(pkMotionManager.MODE_HORSE_ONEHAND_SWORD, COMBO_TYPE_1, 3);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_HORSE_ONEHAND_SWORD, COMBO_TYPE_1, COMBO_INDEX_1, pkMotionManager.NAME_COMBO_ATTACK_1);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_HORSE_ONEHAND_SWORD, COMBO_TYPE_1, COMBO_INDEX_2, pkMotionManager.NAME_COMBO_ATTACK_2);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_HORSE_ONEHAND_SWORD, COMBO_TYPE_1, COMBO_INDEX_3, pkMotionManager.NAME_COMBO_ATTACK_3);

	std::vector<SMotion> m_vecMotionHorseDagger =
	{
		{ pkMotionManager.NAME_COMBO_ATTACK_1, "combo_01.msa", 0 },
		{ pkMotionManager.NAME_COMBO_ATTACK_2, "combo_02.msa", 0 },
		{ pkMotionManager.NAME_COMBO_ATTACK_3, "combo_03.msa", 0 },
		{ HORSE_SKILL_WILDATTACK, "skill_wildattack.msa", 0 },
		{ HORSE_SKILL_SPLASH, "skill_splash.msa", 0 },
	};

	pRaceData->RegisterMotionMode(pkMotionManager.MODE_HORSE_DUALHAND_SWORD);
	for (const auto& it : m_vecMotionHorseDagger)
	{
		snprintf(szFileName, sizeof(szFileName), "%s%s%s", stFolder, "horse_dualhand_sword/", it.stName);
		const CGraphicThing *pkMotionThing = pRaceData->RegisterMotionData(pkMotionManager.MODE_HORSE_DUALHAND_SWORD, it.wMotionIndex, szFileName, it.byPercentage);

		if (pkMotionThing)
			CResourceManager::Instance().LoadStaticCache(pkMotionThing->GetFileName());
	}

	pRaceData->ReserveComboAttack(pkMotionManager.MODE_HORSE_DUALHAND_SWORD, COMBO_TYPE_1, 3);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_HORSE_DUALHAND_SWORD, COMBO_TYPE_1, COMBO_INDEX_1, pkMotionManager.NAME_COMBO_ATTACK_1);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_HORSE_DUALHAND_SWORD, COMBO_TYPE_1, COMBO_INDEX_2, pkMotionManager.NAME_COMBO_ATTACK_2);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_HORSE_DUALHAND_SWORD, COMBO_TYPE_1, COMBO_INDEX_3, pkMotionManager.NAME_COMBO_ATTACK_3);

	std::vector<SMotion> m_vecMotionHorseBow =
	{
		{ pkMotionManager.NAME_WAIT, "wait.msa", 90 },
		{ pkMotionManager.NAME_WAIT, "wait_1.msa", 9 },
		{ pkMotionManager.NAME_WAIT, "wait_2.msa", 1 },
		{ pkMotionManager.NAME_RUN, "run.msa", 0 },
		{ pkMotionManager.NAME_DAMAGE, "damage.msa", 0 },
		{ pkMotionManager.NAME_DEAD, "dead.msa", 0 },
		{ pkMotionManager.NAME_COMBO_ATTACK_1, "attack.msa", 0 },
		{ HORSE_SKILL_WILDATTACK, "skill_wildattack.msa", 0 },
		{ HORSE_SKILL_SPLASH, "skill_splash.msa", 0 },
	};

	pRaceData->RegisterMotionMode(pkMotionManager.MODE_HORSE_BOW);
	for (const auto& it : m_vecMotionHorseBow)
	{
		snprintf(szFileName, sizeof(szFileName), "%s%s%s", stFolder, "horse_bow/", it.stName);
		const CGraphicThing *pkMotionThing = pRaceData->RegisterMotionData(pkMotionManager.MODE_HORSE_BOW, it.wMotionIndex, szFileName, it.byPercentage);

		if (pkMotionThing)
			CResourceManager::Instance().LoadStaticCache(pkMotionThing->GetFileName());
	}

	pRaceData->ReserveComboAttack(pkMotionManager.MODE_HORSE_BOW, COMBO_TYPE_1, 1);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_HORSE_BOW, COMBO_TYPE_1, COMBO_INDEX_1, pkMotionManager.NAME_COMBO_ATTACK_1);

	pRaceData->RegisterAttachingBoneName(CRaceData::PART_WEAPON, "equip_right");
	pRaceData->RegisterAttachingBoneName(CRaceData::PART_WEAPON_LEFT, "equip_left");

#ifdef ENABLE_ACCE_SYSTEM
	pRaceData->RegisterAttachingBoneName(CRaceData::PART_ACCE, "Bip01 Spine2");
#endif
	return true;
}

#ifdef USE_LOADING_DLG_OPTIMIZATION
bool CPlayerSettingsModule::LoadGameSura(DWORD dwRace, char stFolder[FILE_MAX_NUM])
#else
const bool CPlayerSettingsModule::LoadGameSura(DWORD dwRace, char stFolder[FILE_MAX_NUM])
#endif // USE_LOADING_DLG_OPTIMIZATION
{
	CRaceManager& pkManager = CRaceManager::Instance();
	pkManager.SelectRace(dwRace);

	if (!LoadGeneralMotion(stFolder))
		return false;

	CRaceData* pRaceData = pkManager.GetSelectedRaceDataPointer();
	if (!pRaceData)
		return false;

	std::vector<SMotion> m_vecMotionGeneral =
	{
		{ pkMotionManager.NAME_COMBO_ATTACK_1, "attack.msa", 50 },
		{ pkMotionManager.NAME_COMBO_ATTACK_1, "attack_1.msa", 50 },
	};

	char szFileName[FILE_MAX_NUM];
	pRaceData->RegisterMotionMode(pkMotionManager.MODE_GENERAL);
	for (const auto& it : m_vecMotionGeneral)
	{
		snprintf(szFileName, sizeof(szFileName), "%s%s%s", stFolder, "general/", it.stName);
		const CGraphicThing *pkMotionThing = pRaceData->RegisterMotionData(pkMotionManager.MODE_GENERAL, it.wMotionIndex, szFileName, it.byPercentage);

		if (pkMotionThing)
			CResourceManager::Instance().LoadStaticCache(pkMotionThing->GetFileName());
	}

	std::vector<SMotion> m_vecMotionSkill =
	{
		{ GUILD_SKILL_DRAGONBLOOD, "guild_yongsinuipi.msa", 0 },
		{ GUILD_SKILL_DRAGONBLESS, "guild_yongsinuichukbok.msa", 0 },
		{ GUILD_SKILL_BLESSARMOR, "guild_seonghwigap.msa", 0 },
		{ GUILD_SKILL_SPPEDUP, "guild_gasokhwa.msa", 0 },
		{ GUILD_SKILL_DRAGONWRATH, "guild_yongsinuibunno.msa", 0 },
		{ GUILD_SKILL_MAGICUP, "guild_jumunsul.msa", 0 },
	};

	std::vector<std::pair<BYTE, const char*>> m_vecSkillName = {
		{ 1, "swaeryeong" },
		{ 2, "yonggwon" },
		{ 3, "gwigeom" },
		{ 4, "gongpo" },
		{ 5, "jumagap" },
		{ 6, "pabeop" },
		{ 16, "maryeong" },
		{ 17, "hwayeom" },
		{ 18, "muyeong" },
		{ 19, "heuksin" },
		{ 20, "tusok" },
		{ 21, "mahwan" },
	};

	char szSkillAdd[4] = "";
	for (WORD i = 0; i < CPythonSkill::SKILL_EFFECT_COUNT; i++)
	{
		for (auto& it : m_vecSkillName)
		{
			SMotion motion;

			motion.wMotionIndex = pkMotionManager.NAME_SKILL + (i * CPythonSkill::SKILL_GRADEGAP) + it.first;
			motion.byPercentage = 0;

			if (i == 4 || i == 5 || i == 6) // ENABLE_EXPERT_SKILL
				snprintf(szSkillAdd, sizeof(szSkillAdd), "_4");
			else if (i > 0)
				snprintf(szSkillAdd, sizeof(szSkillAdd), "_%d", i + 1);
			else
				snprintf(szSkillAdd, sizeof(szSkillAdd), "");

			snprintf(motion.stName, sizeof(motion.stName), "%s%s.msa", it.second, szSkillAdd);
			m_vecMotionSkill.push_back(motion);
		}
	}

	pRaceData->RegisterMotionMode(pkMotionManager.MODE_GENERAL);
	for (const auto& it : m_vecMotionSkill)
	{
		snprintf(szFileName, sizeof(szFileName), "%s%s%s", stFolder, "skill/", it.stName);
		const CGraphicThing *pkMotionThing = pRaceData->RegisterMotionData(pkMotionManager.MODE_GENERAL, it.wMotionIndex, szFileName, it.byPercentage);

		if (pkMotionThing)
			CResourceManager::Instance().LoadStaticCache(pkMotionThing->GetFileName());
	}

	pRaceData->ReserveComboAttack(pkMotionManager.MODE_GENERAL, COMBO_TYPE_1, 1);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_GENERAL, COMBO_TYPE_1, COMBO_INDEX_1, pkMotionManager.NAME_COMBO_ATTACK_1);

	RegisterEmotionAnis(stFolder);

	std::vector<SMotion> m_vecMotionSword =
	{
		{ pkMotionManager.NAME_WAIT, "wait.msa", 0 },
		{ pkMotionManager.NAME_WALK, "walk.msa" , 0 },
		{ pkMotionManager.NAME_RUN, "run.msa" , 0 },
		{ pkMotionManager.NAME_DAMAGE, "damage.msa", 50 },
		{ pkMotionManager.NAME_DAMAGE, "damage_1.msa", 50 },
		{ pkMotionManager.NAME_DAMAGE_BACK, "damage_2.msa", 50 },
		{ pkMotionManager.NAME_DAMAGE_BACK, "damage_3.msa", 50 },
		{ pkMotionManager.NAME_COMBO_ATTACK_1, "combo_01.msa" , 0 },
		{ pkMotionManager.NAME_COMBO_ATTACK_2, "combo_02.msa" , 0 },
		{ pkMotionManager.NAME_COMBO_ATTACK_3, "combo_03.msa" , 0 },
		{ pkMotionManager.NAME_COMBO_ATTACK_4, "combo_04.msa" , 0 },
		{ pkMotionManager.NAME_COMBO_ATTACK_5, "combo_05.msa" , 0 },
		{ pkMotionManager.NAME_COMBO_ATTACK_6, "combo_06.msa" , 0 },
		{ pkMotionManager.NAME_COMBO_ATTACK_7, "combo_07.msa" , 0 },
	};

	pRaceData->RegisterMotionMode(pkMotionManager.MODE_ONEHAND_SWORD);
	for (const auto& it : m_vecMotionSword)
	{
		snprintf(szFileName, sizeof(szFileName), "%s%s%s", stFolder, "onehand_sword/", it.stName);
		const CGraphicThing *pkMotionThing = pRaceData->RegisterMotionData(pkMotionManager.MODE_ONEHAND_SWORD, it.wMotionIndex, szFileName, it.byPercentage);

		if (pkMotionThing)
			CResourceManager::Instance().LoadStaticCache(pkMotionThing->GetFileName());
	}

	pRaceData->ReserveComboAttack(pkMotionManager.MODE_ONEHAND_SWORD, COMBO_TYPE_1, 4);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_ONEHAND_SWORD, COMBO_TYPE_1, COMBO_INDEX_1, pkMotionManager.NAME_COMBO_ATTACK_1);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_ONEHAND_SWORD, COMBO_TYPE_1, COMBO_INDEX_2, pkMotionManager.NAME_COMBO_ATTACK_2);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_ONEHAND_SWORD, COMBO_TYPE_1, COMBO_INDEX_3, pkMotionManager.NAME_COMBO_ATTACK_3);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_ONEHAND_SWORD, COMBO_TYPE_1, COMBO_INDEX_4, pkMotionManager.NAME_COMBO_ATTACK_4);

	pRaceData->ReserveComboAttack(pkMotionManager.MODE_ONEHAND_SWORD, COMBO_TYPE_2, 5);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_ONEHAND_SWORD, COMBO_TYPE_2, COMBO_INDEX_1, pkMotionManager.NAME_COMBO_ATTACK_1);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_ONEHAND_SWORD, COMBO_TYPE_2, COMBO_INDEX_2, pkMotionManager.NAME_COMBO_ATTACK_2);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_ONEHAND_SWORD, COMBO_TYPE_2, COMBO_INDEX_3, pkMotionManager.NAME_COMBO_ATTACK_3);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_ONEHAND_SWORD, COMBO_TYPE_2, COMBO_INDEX_4, pkMotionManager.NAME_COMBO_ATTACK_5);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_ONEHAND_SWORD, COMBO_TYPE_2, COMBO_INDEX_5, pkMotionManager.NAME_COMBO_ATTACK_7);

	pRaceData->ReserveComboAttack(pkMotionManager.MODE_ONEHAND_SWORD, COMBO_TYPE_3, 6);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_ONEHAND_SWORD, COMBO_TYPE_3, COMBO_INDEX_1, pkMotionManager.NAME_COMBO_ATTACK_1);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_ONEHAND_SWORD, COMBO_TYPE_3, COMBO_INDEX_2, pkMotionManager.NAME_COMBO_ATTACK_2);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_ONEHAND_SWORD, COMBO_TYPE_3, COMBO_INDEX_3, pkMotionManager.NAME_COMBO_ATTACK_3);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_ONEHAND_SWORD, COMBO_TYPE_3, COMBO_INDEX_4, pkMotionManager.NAME_COMBO_ATTACK_5);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_ONEHAND_SWORD, COMBO_TYPE_3, COMBO_INDEX_5, pkMotionManager.NAME_COMBO_ATTACK_6);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_ONEHAND_SWORD, COMBO_TYPE_3, COMBO_INDEX_6, pkMotionManager.NAME_COMBO_ATTACK_4);

	std::vector<SMotion> m_vecMotionFishing =
	{
		{ pkMotionManager.NAME_WAIT, "wait.msa", 0 },
		{ pkMotionManager.NAME_WALK, "walk.msa", 0 },
		{ pkMotionManager.NAME_RUN, "run.msa", 0 },
		{ pkMotionManager.NAME_FISHING_THROW, "throw.msa", 0 },
		{ pkMotionManager.NAME_FISHING_WAIT, "fishing_wait.msa", 0 },
		{ pkMotionManager.NAME_FISHING_STOP, "fishing_cancel.msa", 0 },
		{ pkMotionManager.NAME_FISHING_REACT, "fishing_react.msa", 0 },
		{ pkMotionManager.NAME_FISHING_CATCH, "fishing_catch.msa", 0 },
		{ pkMotionManager.NAME_FISHING_FAIL, "fishing_fail.msa", 0 },
	};

	pRaceData->RegisterMotionMode(pkMotionManager.MODE_FISHING);
	for (const auto& it : m_vecMotionFishing)
	{
		snprintf(szFileName, sizeof(szFileName), "%s%s%s", stFolder, "fishing/", it.stName);
		const CGraphicThing *pkMotionThing = pRaceData->RegisterMotionData(pkMotionManager.MODE_FISHING, it.wMotionIndex, szFileName, it.byPercentage);

		if (pkMotionThing)
			CResourceManager::Instance().LoadStaticCache(pkMotionThing->GetFileName());
	}

	std::vector<SMotion> m_vecMotionHorse =
	{
		{ pkMotionManager.NAME_WAIT, "wait.msa", 90 },
		{ pkMotionManager.NAME_WAIT, "wait_1.msa", 9 },
		{ pkMotionManager.NAME_WAIT, "wait_2.msa", 1 },
		{ pkMotionManager.NAME_WALK, "walk.msa", 0 },
		{ pkMotionManager.NAME_RUN, "run.msa", 0 },
		{ pkMotionManager.NAME_DAMAGE, "damage.msa", 0 },
		{ pkMotionManager.NAME_DAMAGE_BACK, "damage.msa", 0 },
		{ pkMotionManager.NAME_DEAD, "dead.msa", 0 },
		{ HORSE_SKILL_CHARGE, "skill_charge.msa", 0 },
		{ HORSE_SKILL_CHARGE, "skill_splash.msa", 0 },
	};

	pRaceData->RegisterMotionMode(pkMotionManager.MODE_HORSE);
	for (const auto& it : m_vecMotionHorse)
	{
		snprintf(szFileName, sizeof(szFileName), "%s%s%s", stFolder, "horse/", it.stName);
		const CGraphicThing *pkMotionThing = pRaceData->RegisterMotionData(pkMotionManager.MODE_HORSE, it.wMotionIndex, szFileName, it.byPercentage);

		if (pkMotionThing)
			CResourceManager::Instance().LoadStaticCache(pkMotionThing->GetFileName());
	}

	std::vector<SMotion> m_vecMotionHorseSword =
	{
		{ pkMotionManager.NAME_COMBO_ATTACK_1, "combo_01.msa", 0 },
		{ pkMotionManager.NAME_COMBO_ATTACK_2, "combo_02.msa", 0 },
		{ pkMotionManager.NAME_COMBO_ATTACK_3, "combo_03.msa", 0 },
		{ HORSE_SKILL_WILDATTACK, "skill_wildattack.msa", 0 },
	};

	pRaceData->RegisterMotionMode(pkMotionManager.MODE_HORSE_ONEHAND_SWORD);
	for (const auto& it : m_vecMotionHorseSword)
	{
		snprintf(szFileName, sizeof(szFileName), "%s%s%s", stFolder, "horse_onehand_sword/", it.stName);
		const CGraphicThing *pkMotionThing = pRaceData->RegisterMotionData(pkMotionManager.MODE_HORSE_ONEHAND_SWORD, it.wMotionIndex, szFileName, it.byPercentage);

		if (pkMotionThing)
			CResourceManager::Instance().LoadStaticCache(pkMotionThing->GetFileName());
	}

	pRaceData->ReserveComboAttack(pkMotionManager.MODE_HORSE_ONEHAND_SWORD, COMBO_TYPE_1, 3);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_HORSE_ONEHAND_SWORD, COMBO_TYPE_1, COMBO_INDEX_1, pkMotionManager.NAME_COMBO_ATTACK_1);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_HORSE_ONEHAND_SWORD, COMBO_TYPE_1, COMBO_INDEX_2, pkMotionManager.NAME_COMBO_ATTACK_2);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_HORSE_ONEHAND_SWORD, COMBO_TYPE_1, COMBO_INDEX_3, pkMotionManager.NAME_COMBO_ATTACK_3);

	pRaceData->RegisterAttachingBoneName(CRaceData::PART_WEAPON, "equip_right");

#ifdef ENABLE_ACCE_SYSTEM
	pRaceData->RegisterAttachingBoneName(CRaceData::PART_ACCE, "Bip01 Spine2");
#endif

	return true;
}

#ifdef USE_LOADING_DLG_OPTIMIZATION
bool CPlayerSettingsModule::LoadGameShaman(DWORD dwRace, char stFolder[FILE_MAX_NUM])
#else
const bool CPlayerSettingsModule::LoadGameShaman(DWORD dwRace, char stFolder[FILE_MAX_NUM])
#endif // USE_LOADING_DLG_OPTIMIZATION
{
	CRaceManager& pkManager = CRaceManager::Instance();
	pkManager.SelectRace(dwRace);

	if (!LoadGeneralMotion(stFolder))
		return false;

	CRaceData* pRaceData = pkManager.GetSelectedRaceDataPointer();
	if (!pRaceData)
		return false;

	std::vector<SMotion> m_vecMotionGeneral =
	{
		{ pkMotionManager.NAME_COMBO_ATTACK_1, "attack.msa", 50 },
		{ pkMotionManager.NAME_COMBO_ATTACK_1, "attack_1.msa", 50 },
	};

	char szFileName[FILE_MAX_NUM];
	pRaceData->RegisterMotionMode(pkMotionManager.MODE_GENERAL);
	for (const auto& it : m_vecMotionGeneral)
	{
		snprintf(szFileName, sizeof(szFileName), "%s%s%s", stFolder, "general/", it.stName);
		const CGraphicThing *pkMotionThing = pRaceData->RegisterMotionData(pkMotionManager.MODE_GENERAL, it.wMotionIndex, szFileName, it.byPercentage);

		if (pkMotionThing)
			CResourceManager::Instance().LoadStaticCache(pkMotionThing->GetFileName());
	}

	std::vector<SMotion> m_vecMotionSkill =
	{
		{ pkMotionManager.NAME_SKILL + 1, "bipabu.msa" },
		{ pkMotionManager.NAME_SKILL + 2, "yongpa.msa" },
		{ pkMotionManager.NAME_SKILL + 3, "paeryong.msa" },
		{ pkMotionManager.NAME_SKILL + 4, "hosin_target.msa" },
		{ pkMotionManager.NAME_SKILL + 5, "boho_target.msa" },
		{ pkMotionManager.NAME_SKILL + 6, "gicheon_target.msa" },
		{ pkMotionManager.NAME_SKILL + 16, "noejeon.msa" },
		{ pkMotionManager.NAME_SKILL + 17, "byeorak.msa" },
		{ pkMotionManager.NAME_SKILL + 18, "pokroe.msa" },
		{ pkMotionManager.NAME_SKILL + 19, "jeongeop_target.msa" },
		{ pkMotionManager.NAME_SKILL + 20, "kwaesok_target.msa" },
		{ pkMotionManager.NAME_SKILL + 21, "jeungryeok_target.msa" },
		{ GUILD_SKILL_DRAGONBLOOD, "guild_yongsinuipi.msa", 0 },
		{ GUILD_SKILL_DRAGONBLESS, "guild_yongsinuichukbok.msa", 0 },
		{ GUILD_SKILL_BLESSARMOR, "guild_seonghwigap.msa", 0 },
		{ GUILD_SKILL_SPPEDUP, "guild_gasokhwa.msa", 0 },
		{ GUILD_SKILL_DRAGONWRATH, "guild_yongsinuibunno.msa", 0 },
		{ GUILD_SKILL_MAGICUP, "guild_jumunsul.msa", 0 },
	};

	std::vector<std::pair<BYTE, const char*>> m_vecSkillName = {
		{ 1, "bipabu" },
		{ 2, "yongpa" },
		{ 3, "paeryong" },
		{ 4, "hosin" },
		{ 5, "boho" },
		{ 6, "gicheon" },
		{ 16, "noejeon" },
		{ 17, "byeorak" },
		{ 18, "pokroe" },
		{ 19, "jeongeop" },
		{ 20, "kwaesok" },
		{ 21, "jeungryeok" },
	};

	char szSkillAdd[4] = "";
	for (WORD i = 1; i < CPythonSkill::SKILL_EFFECT_COUNT; i++)
	{
		for (auto& it : m_vecSkillName)
		{
			SMotion motion;

			motion.wMotionIndex = pkMotionManager.NAME_SKILL + (i * CPythonSkill::SKILL_GRADEGAP) + it.first;
			motion.byPercentage = 0;

			if (i == 4 || i == 5 || i == 6) // ENABLE_EXPERT_SKILL
				snprintf(szSkillAdd, sizeof(szSkillAdd), "_4");
			else
				snprintf(szSkillAdd, sizeof(szSkillAdd), "_%d", i + 1);

			snprintf(motion.stName, sizeof(motion.stName), "%s%s.msa", it.second, szSkillAdd);
			m_vecMotionSkill.push_back(motion);
		}
	}

	pRaceData->RegisterMotionMode(pkMotionManager.MODE_GENERAL);
	for (const auto& it : m_vecMotionSkill)
	{
		snprintf(szFileName, sizeof(szFileName), "%s%s%s", stFolder, "skill/", it.stName);
		const CGraphicThing *pkMotionThing = pRaceData->RegisterMotionData(pkMotionManager.MODE_GENERAL, it.wMotionIndex, szFileName, it.byPercentage);

		if (pkMotionThing)
			CResourceManager::Instance().LoadStaticCache(pkMotionThing->GetFileName());
	}

	pRaceData->ReserveComboAttack(pkMotionManager.MODE_GENERAL, COMBO_TYPE_1, 1);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_GENERAL, COMBO_TYPE_1, COMBO_INDEX_1, pkMotionManager.NAME_COMBO_ATTACK_1);

	RegisterEmotionAnis(stFolder);

	std::vector<SMotion> m_vecMotionFan =
	{
		{ pkMotionManager.NAME_WAIT, "wait.msa", 0 },
		{ pkMotionManager.NAME_WALK, "walk.msa", 0 },
		{ pkMotionManager.NAME_RUN, "run.msa", 0 },
		{ pkMotionManager.NAME_DAMAGE, "damage.msa", 50,},
		{ pkMotionManager.NAME_DAMAGE, "damage_1.msa", 50, },
		{ pkMotionManager.NAME_DAMAGE_BACK, "damage_2.msa", 50,},
		{ pkMotionManager.NAME_DAMAGE_BACK, "damage_3.msa", 50, },
		{ pkMotionManager.NAME_COMBO_ATTACK_1, "combo_01.msa", 0 },
		{ pkMotionManager.NAME_COMBO_ATTACK_2, "combo_02.msa", 0 },
		{ pkMotionManager.NAME_COMBO_ATTACK_3, "combo_03.msa", 0 },
		{ pkMotionManager.NAME_COMBO_ATTACK_4, "combo_04.msa", 0 },
		{ pkMotionManager.NAME_COMBO_ATTACK_5, "combo_05.msa", 0 },
		{ pkMotionManager.NAME_COMBO_ATTACK_6, "combo_06.msa", 0 },
		{ pkMotionManager.NAME_COMBO_ATTACK_7, "combo_07.msa", 0 },
	};

	pRaceData->RegisterMotionMode(pkMotionManager.MODE_FAN);
	for (const auto& it : m_vecMotionFan)
	{
		snprintf(szFileName, sizeof(szFileName), "%s%s%s", stFolder, "fan/", it.stName);
		const CGraphicThing *pkMotionThing = pRaceData->RegisterMotionData(pkMotionManager.MODE_FAN, it.wMotionIndex, szFileName, it.byPercentage);

		if (pkMotionThing)
			CResourceManager::Instance().LoadStaticCache(pkMotionThing->GetFileName());
	}

	pRaceData->ReserveComboAttack(pkMotionManager.MODE_FAN, COMBO_TYPE_1, 4);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_FAN, COMBO_TYPE_1, COMBO_INDEX_1, pkMotionManager.NAME_COMBO_ATTACK_1);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_FAN, COMBO_TYPE_1, COMBO_INDEX_2, pkMotionManager.NAME_COMBO_ATTACK_2);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_FAN, COMBO_TYPE_1, COMBO_INDEX_3, pkMotionManager.NAME_COMBO_ATTACK_3);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_FAN, COMBO_TYPE_1, COMBO_INDEX_4, pkMotionManager.NAME_COMBO_ATTACK_4);

	pRaceData->ReserveComboAttack(pkMotionManager.MODE_FAN, COMBO_TYPE_2, 5);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_FAN, COMBO_TYPE_2, COMBO_INDEX_1, pkMotionManager.NAME_COMBO_ATTACK_1);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_FAN, COMBO_TYPE_2, COMBO_INDEX_2, pkMotionManager.NAME_COMBO_ATTACK_2);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_FAN, COMBO_TYPE_2, COMBO_INDEX_3, pkMotionManager.NAME_COMBO_ATTACK_3);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_FAN, COMBO_TYPE_2, COMBO_INDEX_4, pkMotionManager.NAME_COMBO_ATTACK_5);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_FAN, COMBO_TYPE_2, COMBO_INDEX_5, pkMotionManager.NAME_COMBO_ATTACK_7);

	pRaceData->ReserveComboAttack(pkMotionManager.MODE_FAN, COMBO_TYPE_3, 6);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_FAN, COMBO_TYPE_3, COMBO_INDEX_1, pkMotionManager.NAME_COMBO_ATTACK_1);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_FAN, COMBO_TYPE_3, COMBO_INDEX_2, pkMotionManager.NAME_COMBO_ATTACK_2);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_FAN, COMBO_TYPE_3, COMBO_INDEX_3, pkMotionManager.NAME_COMBO_ATTACK_3);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_FAN, COMBO_TYPE_3, COMBO_INDEX_4, pkMotionManager.NAME_COMBO_ATTACK_5);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_FAN, COMBO_TYPE_3, COMBO_INDEX_5, pkMotionManager.NAME_COMBO_ATTACK_6);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_FAN, COMBO_TYPE_3, COMBO_INDEX_6, pkMotionManager.NAME_COMBO_ATTACK_4);

	std::vector<SMotion> m_vecMotionBell =
	{
		{ pkMotionManager.NAME_WAIT, "wait.msa", 0 },
		{ pkMotionManager.NAME_WALK, "walk.msa", 0 },
		{ pkMotionManager.NAME_RUN, "run.msa", 0 },
		{ pkMotionManager.NAME_DAMAGE, "damage.msa", 50,},
		{ pkMotionManager.NAME_DAMAGE, "damage_1.msa", 50, },
		{ pkMotionManager.NAME_DAMAGE_BACK, "damage_2.msa", 50,},
		{ pkMotionManager.NAME_DAMAGE_BACK, "damage_3.msa", 50, },
		{ pkMotionManager.NAME_COMBO_ATTACK_1, "combo_01.msa", 0 },
		{ pkMotionManager.NAME_COMBO_ATTACK_2, "combo_02.msa", 0 },
		{ pkMotionManager.NAME_COMBO_ATTACK_3, "combo_03.msa", 0 },
		{ pkMotionManager.NAME_COMBO_ATTACK_4, "combo_04.msa", 0 },
		{ pkMotionManager.NAME_COMBO_ATTACK_5, "combo_05.msa", 0 },
		{ pkMotionManager.NAME_COMBO_ATTACK_6, "combo_06.msa", 0 },
		{ pkMotionManager.NAME_COMBO_ATTACK_7, "combo_07.msa", 0 },
	};

	pRaceData->RegisterMotionMode(pkMotionManager.MODE_BELL);
	for (const auto& it : m_vecMotionBell)
	{
		snprintf(szFileName, sizeof(szFileName), "%s%s%s", stFolder, "bell/", it.stName);
		const CGraphicThing *pkMotionThing = pRaceData->RegisterMotionData(pkMotionManager.MODE_BELL, it.wMotionIndex, szFileName, it.byPercentage);

		if (pkMotionThing)
			CResourceManager::Instance().LoadStaticCache(pkMotionThing->GetFileName());
	}

	pRaceData->ReserveComboAttack(pkMotionManager.MODE_BELL, COMBO_TYPE_1, 4);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_BELL, COMBO_TYPE_1, COMBO_INDEX_1, pkMotionManager.NAME_COMBO_ATTACK_1);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_BELL, COMBO_TYPE_1, COMBO_INDEX_2, pkMotionManager.NAME_COMBO_ATTACK_2);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_BELL, COMBO_TYPE_1, COMBO_INDEX_3, pkMotionManager.NAME_COMBO_ATTACK_3);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_BELL, COMBO_TYPE_1, COMBO_INDEX_4, pkMotionManager.NAME_COMBO_ATTACK_4);

	pRaceData->ReserveComboAttack(pkMotionManager.MODE_BELL, COMBO_TYPE_2, 5);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_BELL, COMBO_TYPE_2, COMBO_INDEX_1, pkMotionManager.NAME_COMBO_ATTACK_1);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_BELL, COMBO_TYPE_2, COMBO_INDEX_2, pkMotionManager.NAME_COMBO_ATTACK_2);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_BELL, COMBO_TYPE_2, COMBO_INDEX_3, pkMotionManager.NAME_COMBO_ATTACK_3);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_BELL, COMBO_TYPE_2, COMBO_INDEX_4, pkMotionManager.NAME_COMBO_ATTACK_5);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_BELL, COMBO_TYPE_2, COMBO_INDEX_5, pkMotionManager.NAME_COMBO_ATTACK_7);

	pRaceData->ReserveComboAttack(pkMotionManager.MODE_BELL, COMBO_TYPE_3, 6);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_BELL, COMBO_TYPE_3, COMBO_INDEX_1, pkMotionManager.NAME_COMBO_ATTACK_1);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_BELL, COMBO_TYPE_3, COMBO_INDEX_2, pkMotionManager.NAME_COMBO_ATTACK_2);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_BELL, COMBO_TYPE_3, COMBO_INDEX_3, pkMotionManager.NAME_COMBO_ATTACK_3);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_BELL, COMBO_TYPE_3, COMBO_INDEX_4, pkMotionManager.NAME_COMBO_ATTACK_5);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_BELL, COMBO_TYPE_3, COMBO_INDEX_5, pkMotionManager.NAME_COMBO_ATTACK_6);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_BELL, COMBO_TYPE_3, COMBO_INDEX_6, pkMotionManager.NAME_COMBO_ATTACK_4);

	std::vector<SMotion> m_vecMotionFishing =
	{
		{ pkMotionManager.NAME_WAIT, "wait.msa", 0 },
		{ pkMotionManager.NAME_WALK, "walk.msa", 0 },
		{ pkMotionManager.NAME_RUN, "run.msa", 0 },
		{ pkMotionManager.NAME_FISHING_THROW, "throw.msa", 0 },
		{ pkMotionManager.NAME_FISHING_WAIT, "fishing_wait.msa", 0 },
		{ pkMotionManager.NAME_FISHING_STOP, "fishing_cancel.msa", 0 },
		{ pkMotionManager.NAME_FISHING_REACT, "fishing_react.msa", 0 },
		{ pkMotionManager.NAME_FISHING_CATCH, "fishing_catch.msa", 0 },
		{ pkMotionManager.NAME_FISHING_FAIL, "fishing_fail.msa", 0 },
	};

	pRaceData->RegisterMotionMode(pkMotionManager.MODE_FISHING);
	for (const auto& it : m_vecMotionFishing)
	{
		snprintf(szFileName, sizeof(szFileName), "%s%s%s", stFolder, "fishing/", it.stName);
		const CGraphicThing *pkMotionThing = pRaceData->RegisterMotionData(pkMotionManager.MODE_FISHING, it.wMotionIndex, szFileName, it.byPercentage);

		if (pkMotionThing)
			CResourceManager::Instance().LoadStaticCache(pkMotionThing->GetFileName());
	}

	std::vector<SMotion> m_vecMotionHorse =
	{
		{ pkMotionManager.NAME_WAIT, "wait.msa", 90 },
		{ pkMotionManager.NAME_WAIT, "wait_1.msa", 9 },
		{ pkMotionManager.NAME_WAIT, "wait_2.msa", 1 },
		{ pkMotionManager.NAME_WALK, "walk.msa", 0 },
		{ pkMotionManager.NAME_RUN, "run.msa", 0 },
		{ pkMotionManager.NAME_DAMAGE, "damage.msa", 0 },
		{ pkMotionManager.NAME_DAMAGE_BACK, "damage.msa", 0 },
		{ pkMotionManager.NAME_DEAD, "dead.msa", 0 },
		{ HORSE_SKILL_CHARGE, "skill_charge.msa", 0 },
		{ HORSE_SKILL_CHARGE, "skill_splash.msa", 0 },
	};

	pRaceData->RegisterMotionMode(pkMotionManager.MODE_HORSE);
	for (const auto& it : m_vecMotionHorse)
	{
		snprintf(szFileName, sizeof(szFileName), "%s%s%s", stFolder, "horse/", it.stName);
		const CGraphicThing *pkMotionThing = pRaceData->RegisterMotionData(pkMotionManager.MODE_HORSE, it.wMotionIndex, szFileName, it.byPercentage);

		if (pkMotionThing)
			CResourceManager::Instance().LoadStaticCache(pkMotionThing->GetFileName());
	}

	std::vector<SMotion> m_vecMotionHorseFan =
	{
		{ pkMotionManager.NAME_COMBO_ATTACK_1, "combo_01.msa", 0 },
		{ pkMotionManager.NAME_COMBO_ATTACK_2, "combo_02.msa", 0 },
		{ pkMotionManager.NAME_COMBO_ATTACK_3, "combo_03.msa", 0 },
		{ HORSE_SKILL_WILDATTACK, "skill_wildattack.msa", 0 },
	};

	pRaceData->RegisterMotionMode(pkMotionManager.MODE_HORSE_FAN);
	for (const auto& it : m_vecMotionHorseFan)
	{
		snprintf(szFileName, sizeof(szFileName), "%s%s%s", stFolder, "horse_fan/", it.stName);
		const CGraphicThing *pkMotionThing = pRaceData->RegisterMotionData(pkMotionManager.MODE_HORSE_FAN, it.wMotionIndex, szFileName, it.byPercentage);

		if (pkMotionThing)
			CResourceManager::Instance().LoadStaticCache(pkMotionThing->GetFileName());
	}

	pRaceData->ReserveComboAttack(pkMotionManager.MODE_HORSE_FAN, COMBO_TYPE_1, 3);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_HORSE_FAN, COMBO_TYPE_1, COMBO_INDEX_1, pkMotionManager.NAME_COMBO_ATTACK_1);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_HORSE_FAN, COMBO_TYPE_1, COMBO_INDEX_2, pkMotionManager.NAME_COMBO_ATTACK_2);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_HORSE_FAN, COMBO_TYPE_1, COMBO_INDEX_3, pkMotionManager.NAME_COMBO_ATTACK_3);

	std::vector<SMotion> m_vecMotionHorseBell =
	{
		{ pkMotionManager.NAME_COMBO_ATTACK_1, "combo_01.msa", 0 },
		{ pkMotionManager.NAME_COMBO_ATTACK_2, "combo_02.msa", 0 },
		{ pkMotionManager.NAME_COMBO_ATTACK_3, "combo_03.msa", 0 },
		{ HORSE_SKILL_WILDATTACK, "skill_wildattack.msa", 0 },
	};

	pRaceData->RegisterMotionMode(pkMotionManager.MODE_HORSE_BELL);
	for (const auto& it : m_vecMotionHorseBell)
	{
		snprintf(szFileName, sizeof(szFileName), "%s%s%s", stFolder, "horse_bell/", it.stName);
		const CGraphicThing *pkMotionThing = pRaceData->RegisterMotionData(pkMotionManager.MODE_HORSE_BELL, it.wMotionIndex, szFileName, it.byPercentage);

		if (pkMotionThing)
			CResourceManager::Instance().LoadStaticCache(pkMotionThing->GetFileName());
	}

	pRaceData->ReserveComboAttack(pkMotionManager.MODE_HORSE_BELL, COMBO_TYPE_1, 3);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_HORSE_BELL, COMBO_TYPE_1, COMBO_INDEX_1, pkMotionManager.NAME_COMBO_ATTACK_1);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_HORSE_BELL, COMBO_TYPE_1, COMBO_INDEX_2, pkMotionManager.NAME_COMBO_ATTACK_2);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_HORSE_BELL, COMBO_TYPE_1, COMBO_INDEX_3, pkMotionManager.NAME_COMBO_ATTACK_3);

	pRaceData->RegisterAttachingBoneName(CRaceData::PART_WEAPON, "equip_right");
	pRaceData->RegisterAttachingBoneName(CRaceData::PART_WEAPON_LEFT, "equip_left");

#ifdef ENABLE_ACCE_SYSTEM
	pRaceData->RegisterAttachingBoneName(CRaceData::PART_ACCE, "Bip01 Spine2");
#endif

	return true;
}

#ifdef ENABLE_WOLFMAN_CHARACTER
#ifdef USE_LOADING_DLG_OPTIMIZATION
bool CPlayerSettingsModule::LoadGameWolfman(DWORD dwRace, char stFolder[FILE_MAX_NUM])
#else
const bool CPlayerSettingsModule::LoadGameWolfman(DWORD dwRace, char stFolder[FILE_MAX_NUM])
#endif // USE_LOADING_DLG_OPTIMIZATION
{
	CRaceManager& pkManager = CRaceManager::Instance();
	pkManager.SelectRace(dwRace);

	if (!LoadNewGeneralMotion(stFolder))
		return false;

	CRaceData* pRaceData = pkManager.GetSelectedRaceDataPointer();
	if (!pRaceData)
		return false;
	if (!pRaceData->SetMotionRandomWeight(pkMotionManager.MODE_GENERAL, pkMotionManager.NAME_WAIT, 0, 50))
		return false;
	std::vector<SMotion> m_vecMotionGeneral =
	{
		{ pkMotionManager.NAME_WAIT, "wait1.msa", 30 },
		{ pkMotionManager.NAME_WAIT, "wait2.msa", 20 },
		{ pkMotionManager.NAME_COMBO_ATTACK_1, "attack1.msa", 50 },
		{ pkMotionManager.NAME_COMBO_ATTACK_1, "attack2.msa", 50 },
	};

	char szFileName[FILE_MAX_NUM];
	pRaceData->RegisterMotionMode(pkMotionManager.MODE_GENERAL);
	for (const auto& it : m_vecMotionGeneral)
	{
		snprintf(szFileName, sizeof(szFileName), "%s%s%s", stFolder, "general/", it.stName);
		const CGraphicThing *pkMotionThing = pRaceData->RegisterMotionData(pkMotionManager.MODE_GENERAL, it.wMotionIndex, szFileName, it.byPercentage);

		if (pkMotionThing)
			CResourceManager::Instance().LoadStaticCache(pkMotionThing->GetFileName());
	}

	std::vector<SMotion> m_vecMotionSkill =
	{
		{ GUILD_SKILL_DRAGONBLOOD, "guild_yongsinuipi.msa", 0 },
		{ GUILD_SKILL_DRAGONBLESS, "guild_yongsinuichukbok.msa", 0 },
		{ GUILD_SKILL_BLESSARMOR, "guild_seonghwigap.msa", 0 },
		{ GUILD_SKILL_SPPEDUP, "guild_gasokhwa.msa", 0 },
		{ GUILD_SKILL_DRAGONWRATH, "guild_yongsinuibunno.msa", 0 },
		{ GUILD_SKILL_MAGICUP, "guild_jumunsul.msa", 0 },
	};

	std::vector<std::pair<BYTE, const char*>> m_vecSkillName = {
		{ 1, "split_slash" },
		{ 2, "wind_death" },
		{ 3, "reef_attack" },
		{ 4, "wreckage" },
		{ 5, "red_possession" },
		{ 6, "blue_possession" },
	};

	char szSkillAdd[4] = "";
	for (WORD i = 0; i < CPythonSkill::SKILL_EFFECT_COUNT; i++)
	{
		for (auto& it : m_vecSkillName)
		{
			SMotion motion;

			motion.wMotionIndex = pkMotionManager.NAME_SKILL + (i * CPythonSkill::SKILL_GRADEGAP) + it.first;
			motion.byPercentage = 0;

			if (i == 4 || i == 5 || i == 6) // ENABLE_EXPERT_SKILL
				snprintf(szSkillAdd, sizeof(szSkillAdd), "_4");
			else if (i > 0)
				snprintf(szSkillAdd, sizeof(szSkillAdd), "_%d", i + 1);
			else
				snprintf(szSkillAdd, sizeof(szSkillAdd), "");

			snprintf(motion.stName, sizeof(motion.stName), "%s%s.msa", it.second, szSkillAdd);
			m_vecMotionSkill.push_back(motion);
		}
	}

	pRaceData->RegisterMotionMode(pkMotionManager.MODE_GENERAL);
	for (const auto& it : m_vecMotionSkill)
	{
		snprintf(szFileName, sizeof(szFileName), "%s%s%s", stFolder, "skill/", it.stName);
		const CGraphicThing *pkMotionThing = pRaceData->RegisterMotionData(pkMotionManager.MODE_GENERAL, it.wMotionIndex, szFileName, it.byPercentage);

		if (pkMotionThing)
			CResourceManager::Instance().LoadStaticCache(pkMotionThing->GetFileName());
	}

	pRaceData->ReserveComboAttack(pkMotionManager.MODE_GENERAL, COMBO_TYPE_1, 1);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_GENERAL, COMBO_TYPE_1, COMBO_INDEX_1, pkMotionManager.NAME_COMBO_ATTACK_1);

	RegisterEmotionAnis(stFolder);

	std::vector<SMotion> m_vecMotionClaw =
	{
		{ pkMotionManager.NAME_WAIT, "wait.msa", 50 },
		{ pkMotionManager.NAME_WAIT, "wait.msa", 30 },
		{ pkMotionManager.NAME_WAIT, "wait.msa", 20 },
		{ pkMotionManager.NAME_WALK, "walk.msa", 0 },
		{ pkMotionManager.NAME_RUN, "run.msa", 0 },
		{ pkMotionManager.NAME_DAMAGE, "front_damage.msa", 50,},
		{ pkMotionManager.NAME_DAMAGE, "front_damage1.msa", 50, },
		{ pkMotionManager.NAME_DAMAGE_BACK, "back_damage.msa", 50,},
		{ pkMotionManager.NAME_DAMAGE_BACK, "back_damage1.msa", 50, },
		{ pkMotionManager.NAME_COMBO_ATTACK_1, "combo_01.msa", 0 },
		{ pkMotionManager.NAME_COMBO_ATTACK_2, "combo_02.msa", 0 },
		{ pkMotionManager.NAME_COMBO_ATTACK_3, "combo_03.msa", 0 },
		{ pkMotionManager.NAME_COMBO_ATTACK_4, "combo_04.msa", 0 },
		{ pkMotionManager.NAME_COMBO_ATTACK_5, "combo_05.msa", 0 },
		{ pkMotionManager.NAME_COMBO_ATTACK_6, "combo_06.msa", 0 },
		{ pkMotionManager.NAME_COMBO_ATTACK_7, "combo_07.msa", 0 },
	};

	pRaceData->RegisterMotionMode(pkMotionManager.MODE_CLAW);
	for (const auto& it : m_vecMotionClaw)
	{
		snprintf(szFileName, sizeof(szFileName), "%s%s%s", stFolder, "claw/", it.stName);
		const CGraphicThing *pkMotionThing = pRaceData->RegisterMotionData(pkMotionManager.MODE_CLAW, it.wMotionIndex, szFileName, it.byPercentage);

		if (pkMotionThing)
			CResourceManager::Instance().LoadStaticCache(pkMotionThing->GetFileName());
	}

	pRaceData->ReserveComboAttack(pkMotionManager.MODE_CLAW, COMBO_TYPE_1, 4);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_CLAW, COMBO_TYPE_1, COMBO_INDEX_1, pkMotionManager.NAME_COMBO_ATTACK_1);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_CLAW, COMBO_TYPE_1, COMBO_INDEX_2, pkMotionManager.NAME_COMBO_ATTACK_2);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_CLAW, COMBO_TYPE_1, COMBO_INDEX_3, pkMotionManager.NAME_COMBO_ATTACK_3);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_CLAW, COMBO_TYPE_1, COMBO_INDEX_4, pkMotionManager.NAME_COMBO_ATTACK_4);

	pRaceData->ReserveComboAttack(pkMotionManager.MODE_CLAW, COMBO_TYPE_2, 5);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_CLAW, COMBO_TYPE_2, COMBO_INDEX_1, pkMotionManager.NAME_COMBO_ATTACK_1);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_CLAW, COMBO_TYPE_2, COMBO_INDEX_2, pkMotionManager.NAME_COMBO_ATTACK_2);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_CLAW, COMBO_TYPE_2, COMBO_INDEX_3, pkMotionManager.NAME_COMBO_ATTACK_3);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_CLAW, COMBO_TYPE_2, COMBO_INDEX_4, pkMotionManager.NAME_COMBO_ATTACK_5);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_CLAW, COMBO_TYPE_2, COMBO_INDEX_5, pkMotionManager.NAME_COMBO_ATTACK_7);

	pRaceData->ReserveComboAttack(pkMotionManager.MODE_CLAW, COMBO_TYPE_3, 6);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_CLAW, COMBO_TYPE_3, COMBO_INDEX_1, pkMotionManager.NAME_COMBO_ATTACK_1);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_CLAW, COMBO_TYPE_3, COMBO_INDEX_2, pkMotionManager.NAME_COMBO_ATTACK_2);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_CLAW, COMBO_TYPE_3, COMBO_INDEX_3, pkMotionManager.NAME_COMBO_ATTACK_3);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_CLAW, COMBO_TYPE_3, COMBO_INDEX_4, pkMotionManager.NAME_COMBO_ATTACK_5);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_CLAW, COMBO_TYPE_3, COMBO_INDEX_5, pkMotionManager.NAME_COMBO_ATTACK_6);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_CLAW, COMBO_TYPE_3, COMBO_INDEX_6, pkMotionManager.NAME_COMBO_ATTACK_4);

	std::vector<SMotion> m_vecMotionFishing =
	{
		{ pkMotionManager.NAME_WAIT, "wait.msa", 0 },
		{ pkMotionManager.NAME_WALK, "walk.msa", 0 },
		{ pkMotionManager.NAME_RUN, "run.msa", 0 },
		{ pkMotionManager.NAME_FISHING_THROW, "throw.msa", 0 },
		{ pkMotionManager.NAME_FISHING_WAIT, "fishing_wait.msa", 0 },
		{ pkMotionManager.NAME_FISHING_STOP, "fishing_cancel.msa", 0 },
		{ pkMotionManager.NAME_FISHING_REACT, "fishing_react.msa", 0 },
		{ pkMotionManager.NAME_FISHING_CATCH, "fishing_catch.msa", 0 },
		{ pkMotionManager.NAME_FISHING_FAIL, "fishing_fail.msa", 0 },
	};

	pRaceData->RegisterMotionMode(pkMotionManager.MODE_FISHING);
	for (const auto& it : m_vecMotionFishing)
	{
		snprintf(szFileName, sizeof(szFileName), "%s%s%s", stFolder, "fishing/", it.stName);
		const CGraphicThing *pkMotionThing = pRaceData->RegisterMotionData(pkMotionManager.MODE_FISHING, it.wMotionIndex, szFileName, it.byPercentage);

		if (pkMotionThing)
			CResourceManager::Instance().LoadStaticCache(pkMotionThing->GetFileName());
	}

	std::vector<SMotion> m_vecMotionHorse =
	{
		{ pkMotionManager.NAME_WAIT, "wait.msa", 90 },
		{ pkMotionManager.NAME_WAIT, "wait1.msa", 9 },
		{ pkMotionManager.NAME_WAIT, "wait2.msa", 1 },
		{ pkMotionManager.NAME_WALK, "walk.msa", 0 },
		{ pkMotionManager.NAME_RUN, "run.msa", 0 },
		{ pkMotionManager.NAME_DAMAGE, "front_damage.msa", 0 },
		{ pkMotionManager.NAME_DAMAGE_BACK, "front_damage.msa", 0 },
		{ pkMotionManager.NAME_DEAD, "dead.msa", 0 },
		{ HORSE_SKILL_CHARGE, "skill_charge.msa", 0 },
		{ HORSE_SKILL_CHARGE, "skill_splash.msa", 0 },
	};

	pRaceData->RegisterMotionMode(pkMotionManager.MODE_HORSE);
	for (const auto& it : m_vecMotionHorse)
	{
		snprintf(szFileName, sizeof(szFileName), "%s%s%s", stFolder, "horse/", it.stName);
		const CGraphicThing *pkMotionThing = pRaceData->RegisterMotionData(pkMotionManager.MODE_HORSE, it.wMotionIndex, szFileName, it.byPercentage);

		if (pkMotionThing)
			CResourceManager::Instance().LoadStaticCache(pkMotionThing->GetFileName());
	}

	std::vector<SMotion> m_vecMotionHorseClaw =
	{
		{ pkMotionManager.NAME_COMBO_ATTACK_1, "combo_01.msa", 0 },
		{ pkMotionManager.NAME_COMBO_ATTACK_2, "combo_02.msa", 0 },
		{ pkMotionManager.NAME_COMBO_ATTACK_3, "combo_03.msa", 0 },
		{ HORSE_SKILL_WILDATTACK, "skill_wildattack.msa", 0 },
	};

	pRaceData->RegisterMotionMode(pkMotionManager.MODE_HORSE_CLAW);
	for (const auto& it : m_vecMotionHorseClaw)
	{
		snprintf(szFileName, sizeof(szFileName), "%s%s%s", stFolder, "horse_claw/", it.stName);
		const CGraphicThing *pkMotionThing = pRaceData->RegisterMotionData(pkMotionManager.MODE_HORSE_CLAW, it.wMotionIndex, szFileName, it.byPercentage);
		if (pkMotionThing)
			CResourceManager::Instance().LoadStaticCache(pkMotionThing->GetFileName());
	}

	pRaceData->ReserveComboAttack(pkMotionManager.MODE_HORSE_CLAW, COMBO_TYPE_1, 3);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_HORSE_CLAW, COMBO_TYPE_1, COMBO_INDEX_1, pkMotionManager.NAME_COMBO_ATTACK_1);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_HORSE_CLAW, COMBO_TYPE_1, COMBO_INDEX_2, pkMotionManager.NAME_COMBO_ATTACK_2);
	pRaceData->RegisterComboAttack(pkMotionManager.MODE_HORSE_CLAW, COMBO_TYPE_1, COMBO_INDEX_3, pkMotionManager.NAME_COMBO_ATTACK_3);

	pRaceData->RegisterAttachingBoneName(CRaceData::PART_WEAPON, "equip_right_weapon");
	pRaceData->RegisterAttachingBoneName(CRaceData::PART_WEAPON_LEFT, "equip_left_weapon");
#ifdef ENABLE_ACCE_SYSTEM
	pRaceData->RegisterAttachingBoneName(CRaceData::PART_ACCE, "Bip01 Spine2");
#endif

	return true;
}
#endif

#ifdef USE_LOADING_DLG_OPTIMIZATION
bool CPlayerSettingsModule::LoadGameSound()
#else
const bool CPlayerSettingsModule::LoadGameSound()
#endif // USE_LOADING_DLG_OPTIMIZATION
{
	std::vector<TSound> m_vecSound = {
		{ CPythonItem::USESOUND_DEFAULT, "sound/ui/drop.wav" },
		{ CPythonItem::USESOUND_ACCESSORY, "sound/ui/equip_ring_amulet.wav" },
		{ CPythonItem::USESOUND_ARMOR, "sound/ui/equip_metal_armor.wav" },
		{ CPythonItem::USESOUND_BOW, "sound/ui/equip_bow.wav" },
		{ CPythonItem::USESOUND_WEAPON, "sound/ui/equip_metal_weapon.wav" },
		{ CPythonItem::USESOUND_POTION, "sound/ui/eat_potion.wav" },
		{ CPythonItem::USESOUND_PORTAL, "sound/ui/potal_scroll.wav" },
	};

	CPythonItem& rkItem = CPythonItem::Instance();

	for (const auto& it : m_vecSound)
		rkItem.SetUseSoundFileName(it.dwType, it.stName);

	std::vector<TSound> m_vecDropSound = {
		{ CPythonItem::DROPSOUND_DEFAULT, "sound/ui/drop.wav" },
		{ CPythonItem::DROPSOUND_ACCESSORY, "sound/ui/equip_ring_amulet.wav" },
		{ CPythonItem::DROPSOUND_ARMOR, "sound/ui/equip_metal_armor.wav" },
		{ CPythonItem::DROPSOUND_BOW, "sound/ui/equip_bow.wav" },
		{ CPythonItem::DROPSOUND_WEAPON, "sound/ui/equip_metal_weapon.wav" },
	};

	for (const auto& it : m_vecDropSound)
		rkItem.SetDropSoundFileName(it.dwType, it.stName);

	return true;
}

#ifndef USE_LOADING_DLG_OPTIMIZATION
const bool CPlayerSettingsModule::LoadGameSkill()
{
	return true;
}
#endif // USE_LOADING_DLG_OPTIMIZATION

#ifndef USE_LOADING_DLG_OPTIMIZATION
PyObject* cpsmInitData(PyObject* poSelf, PyObject* poArgs)
{
	CPlayerSettingsModule::LoadInitData();
	return Py_BuildNone();
}

PyObject* cpsmLoadGameSound(PyObject* poSelf, PyObject* poArgs)
{
	CPlayerSettingsModule::LoadGameSound();
	return Py_BuildNone();
}

PyObject* cpsmLoadGameEffect(PyObject* poSelf, PyObject* poArgs)
{
	CPlayerSettingsModule::LoadGameEffect();
	return Py_BuildNone();
}

PyObject* cpsmLoadGameWarrior(PyObject* poSelf, PyObject* poArgs)
{
	CPlayerSettingsModule::LoadGameWarrior(CPlayerSettingsModule::RACE_WARRIOR_M, "d:/ymir work/pc/warrior/");
	CPlayerSettingsModule::LoadGameWarrior(CPlayerSettingsModule::RACE_WARRIOR_W, "d:/ymir work/pc2/warrior/");
	return Py_BuildNone();
}

PyObject* cpsmLoadGameAssassin(PyObject* poSelf, PyObject* poArgs)
{
	CPlayerSettingsModule::LoadGameAssassin(CPlayerSettingsModule::RACE_ASSASSIN_W, "d:/ymir work/pc/assassin/");
	CPlayerSettingsModule::LoadGameAssassin(CPlayerSettingsModule::RACE_ASSASSIN_M, "d:/ymir work/pc2/assassin/");
	return Py_BuildNone();
}

PyObject* cpsmLoadGameSura(PyObject* poSelf, PyObject* poArgs)
{
	CPlayerSettingsModule::LoadGameSura(CPlayerSettingsModule::RACE_SURA_M, "d:/ymir work/pc/sura/");
	CPlayerSettingsModule::LoadGameSura(CPlayerSettingsModule::RACE_SURA_W, "d:/ymir work/pc2/sura/");
	return Py_BuildNone();
}

PyObject* cpsmLoadGameShaman(PyObject* poSelf, PyObject* poArgs)
{
	CPlayerSettingsModule::LoadGameShaman(CPlayerSettingsModule::RACE_SHAMAN_W, "d:/ymir work/pc/shaman/");
	CPlayerSettingsModule::LoadGameShaman(CPlayerSettingsModule::RACE_SHAMAN_M, "d:/ymir work/pc2/shaman/");
	return Py_BuildNone();
}

#ifdef ENABLE_WOLFMAN_CHARACTER
PyObject* cpsmLoadGameWolfman(PyObject* poSelf, PyObject* poArgs)
{
	CPlayerSettingsModule::LoadGameWolfman(CPlayerSettingsModule::RACE_WOLFMAN_M, "d:/ymir work/pc3/wolfman/");
	return Py_BuildNone();
}
#endif

PyObject* cpsmLoadGameSkill(PyObject* poSelf, PyObject* poArgs)
{
	CPlayerSettingsModule::LoadGameSkill();
	return Py_BuildNone();
}

PyObject* cpsmLoadGameNPC(PyObject* poSelf, PyObject* poArgs)
{
	std::string path = LocaleService_GetLocalePath() + fmt::format("/npclist.txt");
	char paths[150];
	strcpy(paths, path.c_str());
	CPlayerSettingsModule::LoadGameNPC(paths);
	return Py_BuildNone();
}

void initplayersettingsmodule()
{
	static PyMethodDef s_methods[] =
	{
		{ "InitData",					cpsmInitData,						METH_VARARGS },
		{ "LoadGameSound",				cpsmLoadGameSound,					METH_VARARGS },
		{ "LoadGameEffect",				cpsmLoadGameEffect,					METH_VARARGS },
		{ "LoadGameWarrior",			cpsmLoadGameWarrior,				METH_VARARGS },
		{ "LoadGameAssassin",			cpsmLoadGameAssassin,				METH_VARARGS },
		{ "LoadGameSura",				cpsmLoadGameSura,					METH_VARARGS },
		{ "LoadGameShaman",				cpsmLoadGameShaman,					METH_VARARGS },
#ifdef ENABLE_WOLFMAN_CHARACTER
		{ "LoadGameWolfman",			cpsmLoadGameWolfman,				METH_VARARGS },
#endif
		{ "LoadGameSkill",				cpsmLoadGameSkill,					METH_VARARGS },
		{ "LoadGameNPC",				cpsmLoadGameNPC,					METH_VARARGS },

		{ NULL,								NULL,								NULL		 },
	};

	PyObject* poModule = Py_InitModule("cpsm", s_methods);
}
#endif // USE_LOADING_DLG_OPTIMIZATION
