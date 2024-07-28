#pragma once

class CPythonSystem : public CSingleton<CPythonSystem>
{
	public:
#ifdef ENABLE_GAME_OPTION_DLG_RENEWAL
		enum EPages
		{
			PAGE_GAME,
			PAGE_GRAPHIC,
			PAGE_SOUND,
		};

		enum ECategory
		{
			OPTION_CAT_GAME_CHARACTER,
			OPTION_CAT_GAME_USER_INTERFACE,
			OPTION_CAT_GAME_PRIVACY,
			OPTION_CAT_GAME_CHAT_FILTER,
			OPTION_CAT_GRAPHIC_DISPLAY,
			OPTION_CAT_GRAPHIC_ADVANCED,
			OPTION_CAT_GRAPHIC_EFFECT,
			OPTION_CAT_GRAPHIC_EQUIPMENT,
			OPTION_CAT_SOUND_GENERAL,
		};

		enum EHideTypes
		{
			HIDE_ACCE,
			HIDE_AURA,
			HIDE_COSTUME_BODY,
			HIDE_COSTUME_HAIR,
			HIDE_COSTUME_WEAPON,
			HIDE_COSTUME_ACCE,
			HIDE_MAX_NUM,
		};
#endif

		enum
		{
			FREQUENCY_MAX_NUM  = 30,
			RESOLUTION_MAX_NUM = 100
		};

		typedef struct SResolution
		{
			DWORD	width;
			DWORD	height;
			DWORD	bpp;		// bits per pixel (high-color = 16bpp, true-color = 32bpp)

			DWORD	frequency[20];
			BYTE	frequency_count;
		} TResolution;

		typedef struct SConfig
		{
			DWORD			width;
			DWORD			height;
			DWORD			bpp;
			DWORD			frequency;

			bool			is_software_cursor;
			bool			is_object_culling;
			int				iDistance;
			int				iShadowLevel;

			FLOAT			music_volume;
			BYTE			voice_volume;

			int				gamma;

			int				isSaveID;
			char			SaveID[20];

			bool			bWindowed;
			bool			bDecompressDDS;
			bool			bNoSoundCard;
			bool			bUseDefaultIME;
			BYTE			bSoftwareTiling;
			bool			bViewChat;
			bool			bAlwaysShowName;
			bool			bShowDamage;
			bool			bShowSalesText;
			bool			bFogMode;
#if defined(ENABLE_MOB_AGGR_LVL_INFO)
			bool			bShowMobLevel;
			bool			bShowMobAIFlag;
#endif
#ifdef ENABLE_ENVIRONMENT_EFFECT_OPTION
			bool			bShowNightMode;
			bool			bShowSnowMode;
			bool			bShowSnowTextureMode;
			bool			bShowDesertTextureMode;
#endif
#ifdef ENABLE_GRAPHIC_OPTIMIZATION
			int				iEffectLevel;
			int				iPrivateShopLevel;
			int				iDropItemLevel;

			bool			bPetStatus;
			bool			bMountStatus;
			bool			bPrivateShopStatus;
			bool			bCharacterStatus;
			bool			bNpcNameStatus;
#endif // ENABLE_GRAPHIC_OPTIMIZATION
#ifdef ENABLE_HIGHLIGHT_ITEM
			bool			bShowItemHighlight;
#endif
#ifdef ENABLE_PICKUP_OPTION
			bool			bPickUpMode;
			bool			bGoldInfo;
			bool			bExpInfo;
#endif // ENABLE_PICKUP_OPTION
#ifdef ENABLE_CHAT_FILTER_OPTION
			bool			bChatFilter[CHAT_FILTER_MAX_NUM];
#endif // ENABLE_CHAT_FILTER_OPTION
#ifdef ENABLE_GAME_OPTION_DLG_RENEWAL
			BYTE			bCameraMode;
#endif // ENABLE_GAME_OPTION_DLG_RENEWAL
#ifdef ENABLE_FOV_OPTION
			FLOAT			iFOVLevel;
#endif // ENABLE_FOV_OPTION
#ifdef ENABLE_RANK_SYSTEM
			bool			bShowRank;
#endif // ENABLE_RANK_SYSTEM
#ifdef ENABLE_LANDRANK_SYSTEM
			bool			bShowLandRank;
#endif // ENABLE_LANDRANK_SYSTEM
#ifdef ENABLE_TEAM_SYSTEM
			bool			bShowTeam;
#endif // ENABLE_TEAM_SYSTEM
#ifdef ENABLE_REBORN_SYSTEM
			bool			bShowReborn;
#endif // ENABLE_REBORN_SYSTEM
#ifdef ENABLE_MONIKER_SYSTEM
			bool			bShowMoniker;
#endif // ENABLE_MONIKER_SYSTEM
#ifdef ENABLE_MODEL_RENDER_TARGET
			bool			bShowRenderTarget;
#endif // ENABLE_MODEL_RENDER_TARGET
#ifdef ENABLE_DAMAGE_DOT
			bool			bShowDamageDot;
			bool			bShowDamageLetter;
#endif // ENABLE_DAMAGE_DOT
#ifdef ENABLE_HIDE_BODY_PARTS
			bool			bHideCostumeBody;
			bool			bHideCostumeHair;
			bool			bHideCostumeWeapon;
			bool			bHideCostumeAcce;
			//
			bool			bHideAcce;
			bool			bHideAura;
#endif // ENABLE_HIDE_BODY_PARTS
#ifdef ENABLE_DOG_MODE
			bool			bDogMode;
#endif
			int				iStoneSize;
			int				iMobSize;
			int				iPetSize;
			int				iDamageSize;
			BYTE			bLowDamageMode;
		} TConfig;

	public:
		CPythonSystem();
		virtual ~CPythonSystem();

		void Clear();
#if defined(ENABLE_GAME_OPTION_DLG_RENEWAL) && !defined(_LIB)
		void SetPyHandler(PyObject * poHandler);
		void OpenDlg();
#endif

		// Config
		void							SetDefaultConfig();
		bool							LoadConfig();
		bool							SaveConfig();
		void							ApplyConfig();
		void							SetConfig(TConfig * set_config);
		TConfig *						GetConfig();
		void							ChangeSystem();

		DWORD							GetWidth();
		DWORD							GetHeight();
		DWORD							GetBPP();
		DWORD							GetFrequency();
		bool							IsSoftwareCursor();
		bool							IsWindowed();
		bool							IsViewChat();
		bool							IsAlwaysShowName();
		bool							IsShowDamage();
		bool							IsShowSalesText();
		bool							IsUseDefaultIME();
		bool							IsNoSoundCard();
		bool							IsAutoTiling();
		bool							IsSoftwareTiling();
		void							SetSoftwareTiling(bool isEnable);
		void							SetViewChatFlag(int iFlag);
		void							SetAlwaysShowNameFlag(int iFlag);
		void							SetShowDamageFlag(int iFlag);
		void							SetShowSalesTextFlag(int iFlag);
		void							SetFogMode(int iFlag);
		bool							IsFogMode();
#if defined(ENABLE_MOB_AGGR_LVL_INFO)
		void							SetShowMobAIFlag(int iFlag);
		bool							IsShowMobAIFlag();
		void							SetShowMobLevel(int iFlag);
		bool							IsShowMobLevel();
#endif
#ifdef ENABLE_ENVIRONMENT_EFFECT_OPTION
		void							SetNightModeOption(int iFlag);
		bool							GetNightModeOption();
		void							SetSnowModeOption(int iFlag);
		bool							GetSnowModeOption();
		void							SetSnowTextureModeOption(int iFlag);
		bool							GetSnowTextureModeOption();
		void							SetDesertTextureModeOption(int iFlag);
		bool							GetDesertTextureModeOption();
#endif
#ifdef ENABLE_GRAPHIC_OPTIMIZATION
		int								GetEffectLevel() const;
		void							SetEffectLevel(unsigned int level);
		int								GetPrivateShopLevel() const;
		void							SetPrivateShopLevel(unsigned int level);
		int								GetDropItemLevel() const;
		void							SetDropItemLevel(unsigned int level);
		bool							IsPetStatus();
		void							SetPetStatusFlag(int iFlag);
		bool							IsMountStatus();
		void							SetMountStatusFlag(int iFlag);
		bool							IsPrivateShopStatus();
		void							SetPrivateShopStatus(int iFlag);
		bool							IsCharacterStatus();
		void							SetCharacterStatus(int iFlag);
		bool							IsNpcNameStatus();
		void							SetNpcNameStatusFlag(int iFlag);
#endif
#ifdef ENABLE_HIGHLIGHT_ITEM
		bool							IsShowItemHighlight();
		void							SetShowItemHighlight(int iFlag);
#endif // ENABLE_HIGHLIGHT_ITEM
#ifdef ENABLE_PICKUP_OPTION
		bool							IsPickUpMode();
		void							SetPickUpMode(int iFlag);

		bool							IsGoldInfo() { return m_Config.bGoldInfo; }
		void							SetGoldInfo(bool f) { m_Config.bGoldInfo = f; }

		bool							IsExpInfo() { return m_Config.bExpInfo; }
		void							SetExpInfo(bool f) { m_Config.bExpInfo = f; }
#endif // ENABLE_PICKUP_OPTION
#ifdef ENABLE_CHAT_FILTER_OPTION
		bool							GetChatFilterMode(BYTE chatFilterIdx);
		bool							IsChatFilter(BYTE chatType);
		void							SetChatFilterMode(BYTE chatFilterIdx, bool mode);
#endif // ENABLE_CHAT_FILTER_OPTION
#ifdef ENABLE_GAME_OPTION_DLG_RENEWAL
		BYTE							GetCameraMode();
		void							SetCameraMode(BYTE bFlag);
#endif // ENABLE_GAME_OPTION_DLG_RENEWAL

#ifdef ENABLE_FOV_OPTION
		float							GetFOVLevel();
		void							SetFOVLevel(float fFOV);
#endif

#ifdef ENABLE_RANK_SYSTEM
		bool							IsShowRank() { return m_Config.bShowRank; }
		void							SetShowRank(bool f) { m_Config.bShowRank = f; }
#endif // ENABLE_RANK_SYSTEM

#ifdef ENABLE_LANDRANK_SYSTEM
		bool							IsShowLandRank() { return m_Config.bShowLandRank; }
		void							SetShowLandRank(bool f) { m_Config.bShowLandRank = f; }
#endif // ENABLE_LANDRANK_SYSTEM

#ifdef ENABLE_TEAM_SYSTEM
		bool							IsShowTeam() { return m_Config.bShowTeam; }
		void							SetShowTeam(bool f) { m_Config.bShowTeam = f; }
#endif // ENABLE_TEAM_SYSTEM

#ifdef ENABLE_REBORN_SYSTEM
		bool							IsShowReborn() { return m_Config.bShowReborn; }
		void							SetShowReborn(bool f) { m_Config.bShowReborn = f; }
#endif // ENABLE_REBORN_SYSTEM

#ifdef ENABLE_MONIKER_SYSTEM
		bool							IsShowMoniker() { return m_Config.bShowMoniker; }
		void							SetShowMoniker(bool f) { m_Config.bShowMoniker = f; }
#endif // ENABLE_MONIKER_SYSTEM

#ifdef ENABLE_MODEL_RENDER_TARGET
		bool							IsShowRenderTarget() { return m_Config.bShowRenderTarget; }
		void							SetShowRenderTarget(bool f) { m_Config.bShowRenderTarget = f; }
#endif // ENABLE_MODEL_RENDER_TARGET

#ifdef ENABLE_DAMAGE_DOT
		bool							IsShowDamageDot() { return m_Config.bShowDamageDot; }
		void							SetShowDamageDot(bool f) { m_Config.bShowDamageDot = f; }
		bool							IsShowDamageLetter() { return m_Config.bShowDamageLetter; }
		void							SetShowDamageLetter(bool f) { m_Config.bShowDamageLetter = f; }
#endif // ENABLE_DAMAGE_DOT


#ifdef ENABLE_HIDE_BODY_PARTS
		bool							IsHideCostumeBody() { return m_Config.bHideCostumeBody; }
		void							SetHideCostumeBody(bool f) { m_Config.bHideCostumeBody = f; }

		bool							IsHideCostumeHair() { return m_Config.bHideCostumeHair; }
		void							SetHideCostumeHair(bool f) { m_Config.bHideCostumeHair = f; }

		bool							IsHideCostumeWeapon() { return m_Config.bHideCostumeWeapon; }
		void							SetHideCostumeWeapon(bool f) { m_Config.bHideCostumeWeapon = f; }

		bool							IsHideCostumeAcce() { return m_Config.bHideCostumeAcce; }
		void							SetHideCostumeAcce(bool f) { m_Config.bHideCostumeAcce = f; }

		bool							IsHideAcce() { return m_Config.bHideAcce; }
		void							SetHideAcce(bool f) { m_Config.bHideAcce = f; }

		bool							IsHideAura() { return m_Config.bHideAura; }
		void							SetHideAura(bool f) { m_Config.bHideAura = f; }
#endif // ENABLE_HIDE_BODY_PARTS

		int								GetStoneSize() { return m_Config.iStoneSize; }
		void							SetStoneSize(int level) { m_Config.iStoneSize = level; }

		int								GetMobSize() { return m_Config.iMobSize; }
		void							SetMobSize(int level) { m_Config.iMobSize = level; }

		int								GetPetSize() { return m_Config.iPetSize; }
		void							SetPetSize(int level) { m_Config.iPetSize = level; }

		void							SetDamageSize(int level) { m_Config.iDamageSize = level; }
		int								GetDamageSize() { return m_Config.iDamageSize; }

		void							SetLowDamageMode(BYTE bFlag);
		BYTE							GetLowDamageMode();

#ifdef ENABLE_DOG_MODE
		void							SetDogMode(int iFlag);
		bool							GetDogMode();
#endif

		// SaveID
		int								IsSaveID();
		const char *					GetSaveID();
		void							SetSaveID(int iValue, const char * c_szSaveID);

		/// Display
		void							GetDisplaySettings();

		int								GetResolutionCount();
		int								GetFrequencyCount(int index);
		bool							GetResolution(int index, OUT DWORD *width, OUT DWORD *height, OUT DWORD *bpp);
		bool							GetFrequency(int index, int freq_index, OUT DWORD *frequncy);
		int								GetResolutionIndex(DWORD width, DWORD height, DWORD bpp);
		int								GetFrequencyIndex(int res_index, DWORD frequency);
		bool							isViewCulling();

		// Sound
		float							GetMusicVolume();
		int								GetSoundVolume();
		void							SetMusicVolume(float fVolume);
		void							SetSoundVolumef(float fVolume);

		int								GetDistance();
		int								GetShadowLevel();
		void							SetShadowLevel(unsigned int level);

	protected:
		TResolution						m_ResolutionList[RESOLUTION_MAX_NUM];
		int								m_ResolutionCount;

		TConfig							m_Config;
		TConfig							m_OldConfig;

#if defined(ENABLE_GAME_OPTION_DLG_RENEWAL) && !defined(_LIB)
		PyObject *						m_poHandler;
#endif

#ifdef ENABLE_HIDE_BODY_PARTS
	#ifndef _LIB
		DWORD							m_instancePartBackups[8];
		DWORD							m_instancePartBackupSpecular;
	public:
		void							SetUpdateInstance(BYTE setting, int iOpt);
		void							SetPartBackup(BYTE part, DWORD val) { m_instancePartBackups[part] = val; }
		void							SetPartSpecularBackup(DWORD val) { m_instancePartBackupSpecular = val; }

		DWORD							GetPartBackup(BYTE part) { return m_instancePartBackups[part]; }
		DWORD							GetPartSpecularBackup() { return m_instancePartBackupSpecular; }
	#endif // _LIB	
#endif // ENABLE_HIDE_BODY_PARTS

	private:
		std::string GetCpuID();
		std::string GUIDtoString(GUID guid);
		std::string GetGpuID();
		bool getDMI(std::vector<uint8_t>& buffer);
	public:
		std::string	GenerateHash(std::string aString);
		const char* GetHWID();
};