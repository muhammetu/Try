#pragma once

#ifdef USE_LOADING_DLG_OPTIMIZATION
#include <thread>
#endif // USE_LOADING_DLG_OPTIMIZATION

class CPlayerSettingsModule : public CSingleton<CPlayerSettingsModule>
{
public:
	CPlayerSettingsModule() = default;
	virtual ~CPlayerSettingsModule() = default;

	enum EConfig
	{
		DUST_GAP = 250,
		HORSE_DUST_GAP = 500,
		RACE_WARRIOR_M = 0,
		RACE_ASSASSIN_W = 1,
		RACE_SURA_M = 2,
		RACE_SHAMAN_W = 3,
		RACE_WARRIOR_W = 4,
		RACE_ASSASSIN_M = 5,
		RACE_SURA_W = 6,
		RACE_SHAMAN_M = 7,
#ifdef ENABLE_WOLFMAN_CHARACTER
		RACE_WOLFMAN_M = 8,
#endif
		COMBO_TYPE_1 = 0,
		COMBO_TYPE_2 = 1,
		COMBO_TYPE_3 = 2,
		COMBO_INDEX_1 = 0,
		COMBO_INDEX_2 = 1,
		COMBO_INDEX_3 = 2,
		COMBO_INDEX_4 = 3,
		COMBO_INDEX_5 = 4,
		COMBO_INDEX_6 = 5,
		FILE_MAX_NUM = 150,
		BONE_MAX_NUM = 24,
	};
private:

	typedef struct SEffect
	{
		UINT uiType;
		char stBone[BONE_MAX_NUM];
		char stEffect[FILE_MAX_NUM];
	} TEffect;

	typedef struct SRaceData
	{
		DWORD dwRace;
		std::string stMSM;
		char stIntroMotion[FILE_MAX_NUM];
	} TRaceData;

	typedef struct SMotion
	{
		int wMotionIndex;
		char stName[FILE_MAX_NUM];
		BYTE byPercentage;
	} TMotion;

	typedef struct SFly
	{
		DWORD dwIndex;
		BYTE byType;
		char stName[FILE_MAX_NUM];
	} TFly;

	typedef struct SSound
	{
		DWORD dwType;
		char stName[FILE_MAX_NUM];
	} TSound;

public:
#ifdef USE_LOADING_DLG_OPTIMIZATION
		void Load();

		bool LoadInitData();
		bool LoadGameEffect();
		bool LoadGameSound();

		bool LoadGameWarrior(DWORD dwRace, char stFolder[FILE_MAX_NUM]);
		bool LoadGameAssassin(DWORD dwRace, char stFolder[FILE_MAX_NUM]);
		bool LoadGameSura(DWORD dwRace, char stFolder[FILE_MAX_NUM]);
		bool LoadGameShaman(DWORD dwRace, char stFolder[FILE_MAX_NUM]);
#ifdef ENABLE_WOLFMAN_CHARACTER
		bool LoadGameWolfman(DWORD dwRace, char stFolder[FILE_MAX_NUM]);
#endif // ENABLE_WOLFMAN_CHARACTER
		bool LoadGameNPC(char stFolder[FILE_MAX_NUM]);

		// sub
		bool RegisterEmotionAnis(char stFolder[FILE_MAX_NUM]);
#ifdef ENABLE_WOLFMAN_CHARACTER
		bool LoadNewGeneralMotion(char stFolder[FILE_MAX_NUM]);
#endif // ENABLE_WOLFMAN_CHARACTER
		bool LoadGeneralMotion(char stFolder[FILE_MAX_NUM]);
	protected:
		// 
	public:
		std::thread m_loadThread;
		bool m_loadedThread = false;
		bool m_alreadyjoined = false;

		void startLoadThread()
		{
			if (!m_loadedThread)
			{
				m_loadThread = std::thread(&CPlayerSettingsModule::Load, this);
				m_loadedThread = true;
			}
		}

		void joinLoadThread()
		{
			if (m_loadThread.joinable() && !m_alreadyjoined)
			{
				m_loadThread.join();
				m_alreadyjoined = true;
			}
		}
#else // USE_LOADING_DLG_OPTIMIZATION
	const static bool LoadInitData();
	const static bool LoadGameEffect();
	const static bool RegisterEmotionAnis(char stFolder[FILE_MAX_NUM]);
	const static bool LoadGeneralMotion(char stFolder[FILE_MAX_NUM]);
#ifdef ENABLE_WOLFMAN_CHARACTER
	const static bool LoadNewGeneralMotion(char stFolder[FILE_MAX_NUM]);
#endif
	const static bool LoadGameWarrior(DWORD dwRace, char stFolder[FILE_MAX_NUM]);
	const static bool LoadGameAssassin(DWORD dwRace, char stFolder[FILE_MAX_NUM]);
	const static bool LoadGameSura(DWORD dwRace, char stFolder[FILE_MAX_NUM]);
	const static bool LoadGameShaman(DWORD dwRace, char stFolder[FILE_MAX_NUM]);
#ifdef ENABLE_WOLFMAN_CHARACTER
	const static bool LoadGameWolfman(DWORD dwRace, char stFolder[FILE_MAX_NUM]);
#endif
	const static bool LoadGameSound();
	const static bool LoadGameSkill();
	const static bool LoadGameNPC(char stFolder[FILE_MAX_NUM]);
#endif // USE_LOADING_DLG_OPTIMIZATION
};
