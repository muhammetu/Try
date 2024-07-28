#include "StdAfx.h"
#include "PythonSystem.h"
#include "PythonApplication.h"
#ifndef _LIB
#include "InstanceBase.h"
#endif // _LIB
#ifdef ENABLE_GAME_OPTION_DLG_RENEWAL
#include "../EterLib/Camera.h"
#endif
#ifdef ENABLE_CHAT_FILTER_OPTION
#include "PythonChat.h"
#endif
#ifdef ENABLE_HIDE_BODY_PARTS
#include "PythonCharacterManager.h"
#endif // ENABLE_HIDE_BODY_PARTS

#include <iomanip>
#include "picosha2.h"
#include "smbios.cpp"

#define DEFAULT_VALUE_ALWAYS_SHOW_NAME	true

#ifdef ENABLE_GAME_OPTION_DLG_RENEWAL
void CPythonSystem::SetPyHandler(PyObject * poHandler)
{
	m_poHandler = poHandler;
}

void CPythonSystem::OpenDlg()
{
	PyCallClassMemberFunc(m_poHandler, "Open", Py_BuildValue("()"));
}
#endif

void CPythonSystem::GetDisplaySettings()
{
	memset(m_ResolutionList, 0, sizeof(TResolution) * RESOLUTION_MAX_NUM);
	m_ResolutionCount = 0;

	LPDIRECT3D8 lpD3D = CPythonGraphic::Instance().GetD3D();

	D3DADAPTER_IDENTIFIER8 d3dAdapterIdentifier;
	D3DDISPLAYMODE d3ddmDesktop;

	lpD3D->GetAdapterIdentifier(0, D3DENUM_NO_WHQL_LEVEL, &d3dAdapterIdentifier);
	lpD3D->GetAdapterDisplayMode(0, &d3ddmDesktop);

	DWORD dwNumAdapterModes = lpD3D->GetAdapterModeCount(0);

	for (UINT iMode = 0; iMode < dwNumAdapterModes; iMode++)
	{
		D3DDISPLAYMODE DisplayMode;
		lpD3D->EnumAdapterModes(0, iMode, &DisplayMode);
		DWORD bpp = 0;

		if (DisplayMode.Width < 800 || DisplayMode.Height < 600)
			continue;

		if (DisplayMode.Format == D3DFMT_R5G6B5)
			bpp = 16;
		else if (DisplayMode.Format == D3DFMT_X8R8G8B8)
			bpp = 32;
		else
			continue;

		int check_res = false;

		for (int i = 0; !check_res && i < m_ResolutionCount; ++i)
		{
			if (m_ResolutionList[i].bpp != bpp ||
				m_ResolutionList[i].width != DisplayMode.Width ||
				m_ResolutionList[i].height != DisplayMode.Height)
				continue;

			int check_fre = false;

			for (int j = 0; j < m_ResolutionList[i].frequency_count; ++j)
			{
				if (m_ResolutionList[i].frequency[j] == DisplayMode.RefreshRate)
				{
					check_fre = true;
					break;
				}
			}

			if (!check_fre)
				if (m_ResolutionList[i].frequency_count < FREQUENCY_MAX_NUM)
					m_ResolutionList[i].frequency[m_ResolutionList[i].frequency_count++] = DisplayMode.RefreshRate;

			check_res = true;
		}

		if (!check_res)
		{
			if (m_ResolutionCount < RESOLUTION_MAX_NUM)
			{
				m_ResolutionList[m_ResolutionCount].width			= DisplayMode.Width;
				m_ResolutionList[m_ResolutionCount].height			= DisplayMode.Height;
				m_ResolutionList[m_ResolutionCount].bpp				= bpp;
				m_ResolutionList[m_ResolutionCount].frequency[0]	= DisplayMode.RefreshRate;
				m_ResolutionList[m_ResolutionCount].frequency_count	= 1;

				++m_ResolutionCount;
			}
		}
	}
}

int	CPythonSystem::GetResolutionCount()
{
	return m_ResolutionCount;
}

int CPythonSystem::GetFrequencyCount(int index)
{
	if (index >= m_ResolutionCount)
		return 0;

    return m_ResolutionList[index].frequency_count;
}

bool CPythonSystem::GetResolution(int index, OUT DWORD *width, OUT DWORD *height, OUT DWORD *bpp)
{
	if (index >= m_ResolutionCount)
		return false;

	*width = m_ResolutionList[index].width;
	*height = m_ResolutionList[index].height;
	*bpp = m_ResolutionList[index].bpp;
	return true;
}

bool CPythonSystem::GetFrequency(int index, int freq_index, OUT DWORD *frequncy)
{
	if (index >= m_ResolutionCount)
		return false;

	if (freq_index >= m_ResolutionList[index].frequency_count)
		return false;

	*frequncy = m_ResolutionList[index].frequency[freq_index];
	return true;
}

int	CPythonSystem::GetResolutionIndex(DWORD width, DWORD height, DWORD bit)
{
	DWORD re_width, re_height, re_bit;
	int i = 0;

	while (GetResolution(i, &re_width, &re_height, &re_bit))
	{
		if (re_width == width)
			if (re_height == height)
				if (re_bit == bit)
					return i;
		i++;
	}

	return 0;
}

int	CPythonSystem::GetFrequencyIndex(int res_index, DWORD frequency)
{
	DWORD re_frequency;
	int i = 0;

	while (GetFrequency(res_index, i, &re_frequency))
	{
		if (re_frequency == frequency)
			return i;

		i++;
	}

	return 0;
}

DWORD CPythonSystem::GetWidth()
{
	return m_Config.width;
}

DWORD CPythonSystem::GetHeight()
{
	return m_Config.height;
}
DWORD CPythonSystem::GetBPP()
{
	return m_Config.bpp;
}
DWORD CPythonSystem::GetFrequency()
{
	return m_Config.frequency;
}

bool CPythonSystem::IsNoSoundCard()
{
	return m_Config.bNoSoundCard;
}

bool CPythonSystem::IsSoftwareCursor()
{
	return m_Config.is_software_cursor;
}

float CPythonSystem::GetMusicVolume()
{
	return m_Config.music_volume;
}

int CPythonSystem::GetSoundVolume()
{
	return m_Config.voice_volume;
}

void CPythonSystem::SetMusicVolume(float fVolume)
{
	m_Config.music_volume = fVolume;
}

void CPythonSystem::SetSoundVolumef(float fVolume)
{
	m_Config.voice_volume = int(5 * fVolume);
}

#ifdef ENABLE_FOV_OPTION
void CPythonSystem::SetFOVLevel(float fFOV)
{
	m_Config.iFOVLevel = fMINMAX(30.0f, fFOV, 120.0f);
}

float CPythonSystem::GetFOVLevel()
{
	return m_Config.iFOVLevel;
}
#endif

int CPythonSystem::GetDistance()
{
	return m_Config.iDistance;
}

int CPythonSystem::GetShadowLevel()
{
	return m_Config.iShadowLevel;
}

void CPythonSystem::SetShadowLevel(unsigned int level)
{
	m_Config.iShadowLevel = MIN(level, 5);
	CPythonBackground::instance().RefreshShadowLevel();
}

int CPythonSystem::IsSaveID()
{
	return m_Config.isSaveID;
}

const char * CPythonSystem::GetSaveID()
{
	return m_Config.SaveID;
}

bool CPythonSystem::isViewCulling()
{
	return m_Config.is_object_culling;
}

void CPythonSystem::SetSaveID(int iValue, const char * c_szSaveID)
{
	if (iValue != 1)
		return;

	m_Config.isSaveID = iValue;
	strncpy(m_Config.SaveID, c_szSaveID, sizeof(m_Config.SaveID) - 1);
}

CPythonSystem::TConfig * CPythonSystem::GetConfig()
{
	return &m_Config;
}

void CPythonSystem::SetConfig(TConfig * pNewConfig)
{
	m_Config = *pNewConfig;
}

void CPythonSystem::SetDefaultConfig()
{
	ZeroMemory(&m_Config, sizeof(m_Config));

	m_Config.width				= 1024;
	m_Config.height				= 768;
	m_Config.bpp				= 32;

	m_Config.bWindowed			= true;

	m_Config.is_software_cursor	= false;
	m_Config.is_object_culling	= true;
	m_Config.iDistance			= 3;

	m_Config.gamma				= 3;
	m_Config.music_volume		= 1.0f;
	m_Config.voice_volume		= 5;

	m_Config.bDecompressDDS		= 0;
	m_Config.bSoftwareTiling	= 0;
	m_Config.iShadowLevel		= 3;
	m_Config.bViewChat			= true;
	m_Config.bAlwaysShowName	= DEFAULT_VALUE_ALWAYS_SHOW_NAME;
	m_Config.bShowDamage		= true;
	m_Config.bShowSalesText		= true;
	m_Config.bFogMode			= true;
#if defined(ENABLE_MOB_AGGR_LVL_INFO)
	m_Config.bShowMobLevel		= true;
	m_Config.bShowMobAIFlag		= true;
#endif
#ifdef ENABLE_ENVIRONMENT_EFFECT_OPTION
	m_Config.bShowNightMode			= false;
	m_Config.bShowSnowMode			= false;
	m_Config.bShowSnowTextureMode	= false;
	m_Config.bShowDesertTextureMode	= false;
#endif
#ifdef ENABLE_GRAPHIC_OPTIMIZATION
	m_Config.iEffectLevel = 3;
	m_Config.iPrivateShopLevel	= 0;
	m_Config.iDropItemLevel		= 0;
	m_Config.bPetStatus			= true;
	m_Config.bMountStatus		= true;
	m_Config.bPrivateShopStatus	= true;
	m_Config.bCharacterStatus	= true;
	m_Config.bNpcNameStatus		= true;
#endif
#ifdef ENABLE_HIGHLIGHT_ITEM
	m_Config.bShowItemHighlight	= true;
#endif
#ifdef ENABLE_PICKUP_OPTION
	m_Config.bPickUpMode = true;
	m_Config.bGoldInfo			= true;
	m_Config.bExpInfo			= true;
#endif // ENABLE_PICKUP_OPTION
#ifdef ENABLE_CHAT_FILTER_OPTION
	std::fill(m_Config.bChatFilter, m_Config.bChatFilter+CHAT_TYPE_MAX_NUM, false);
#endif // ENABLE_CHAT_FILTER_OPTION
#ifdef ENABLE_GAME_OPTION_DLG_RENEWAL
	m_Config.bCameraMode		= 0;
#endif // ENABLE_GAME_OPTION_DLG_RENEWAL
#ifdef ENABLE_FOV_OPTION
	m_Config.iFOVLevel = 30.0f;
#endif // ENABLE_FOV_OPTION
#ifdef ENABLE_RANK_SYSTEM
	m_Config.bShowRank			= true;
#endif // ENABLE_RANK_SYSTEM
#ifdef ENABLE_LANDRANK_SYSTEM
	m_Config.bShowLandRank		= true;
#endif // ENABLE_LANDRANK_SYSTEM
#ifdef ENABLE_TEAM_SYSTEM
	m_Config.bShowTeam			= true;
#endif // ENABLE_TEAM_SYSTEM
#ifdef ENABLE_REBORN_SYSTEM
	m_Config.bShowReborn		= true;
#endif // ENABLE_REBORN_SYSTEM
#ifdef ENABLE_MONIKER_SYSTEM
	m_Config.bShowMoniker		= true;
#endif // ENABLE_MONIKER_SYSTEM
#ifdef ENABLE_MODEL_RENDER_TARGET
	m_Config.bShowRenderTarget	= true;
#endif // ENABLE_MODEL_RENDER_TARGET
#ifdef ENABLE_DAMAGE_DOT
	m_Config.bShowDamageDot		= false;
	m_Config.bShowDamageLetter = true;
#endif // ENABLE_DAMAGE_DOT
#ifdef ENABLE_HIDE_BODY_PARTS
	m_Config.bHideCostumeBody	= false;
	m_Config.bHideCostumeHair	= false;
	m_Config.bHideCostumeWeapon	= false;
	m_Config.bHideCostumeAcce	= false;
	m_Config.bHideAcce			= false;
	m_Config.bHideAura			= false;
#endif // ENABLE_HIDE_BODY_PARTS
#ifdef ENABLE_DOG_MODE
	m_Config.bDogMode = false;
#endif
	m_Config.iStoneSize			= 1;
	m_Config.iMobSize			= 0;
	m_Config.iPetSize			= 1;
	m_Config.iDamageSize = 0;
	m_Config.bLowDamageMode = 0;
}

bool CPythonSystem::IsWindowed()
{
	return m_Config.bWindowed;
}

bool CPythonSystem::IsViewChat()
{
	return m_Config.bViewChat;
}

void CPythonSystem::SetViewChatFlag(int iFlag)
{
	m_Config.bViewChat = iFlag == 1 ? true : false;
}

bool CPythonSystem::IsAlwaysShowName()
{
	return m_Config.bAlwaysShowName;
}

void CPythonSystem::SetAlwaysShowNameFlag(int iFlag)
{
	m_Config.bAlwaysShowName = iFlag == 1 ? true : false;
}

bool CPythonSystem::IsShowDamage()
{
	return m_Config.bShowDamage;
}

void CPythonSystem::SetShowDamageFlag(int iFlag)
{
	m_Config.bShowDamage = iFlag == 1 ? true : false;
}

bool CPythonSystem::IsShowSalesText()
{
	return m_Config.bShowSalesText;
}

void CPythonSystem::SetShowSalesTextFlag(int iFlag)
{
	m_Config.bShowSalesText = iFlag == 1 ? true : false;
}

bool CPythonSystem::IsAutoTiling()
{
	if (m_Config.bSoftwareTiling == 0)
		return true;

	return false;
}

void CPythonSystem::SetSoftwareTiling(bool isEnable)
{
	if (isEnable)
		m_Config.bSoftwareTiling=1;
	else
		m_Config.bSoftwareTiling=2;
}

bool CPythonSystem::IsSoftwareTiling()
{
	if (m_Config.bSoftwareTiling==1)
		return true;

	return false;
}

bool CPythonSystem::IsUseDefaultIME()
{
	return m_Config.bUseDefaultIME;
}

void CPythonSystem::SetFogMode(int iFlag)
{
	m_Config.bFogMode = iFlag == 1 ? true : false;
}

bool CPythonSystem::IsFogMode()
{
	return m_Config.bFogMode;
}

#if defined(ENABLE_MOB_AGGR_LVL_INFO)
void CPythonSystem::SetShowMobLevel(int iOpt)
{
	m_Config.bShowMobLevel = iOpt == 1 ? true : false;
}

bool CPythonSystem::IsShowMobLevel()
{
	return m_Config.bShowMobLevel;
}

void CPythonSystem::SetShowMobAIFlag(int iOpt)
{
	m_Config.bShowMobAIFlag = iOpt == 1 ? true : false;
}

bool CPythonSystem::IsShowMobAIFlag()
{
	return m_Config.bShowMobAIFlag;
}
#endif

#ifdef ENABLE_ENVIRONMENT_EFFECT_OPTION
void CPythonSystem::SetNightModeOption(int iOpt)
{
	m_Config.bShowNightMode = iOpt == 1 ? true : false;
}

bool CPythonSystem::GetNightModeOption()
{
	return m_Config.bShowNightMode;
}

void CPythonSystem::SetSnowModeOption(int iOpt)
{
	m_Config.bShowSnowMode = iOpt == 1 ? true : false;
}

bool CPythonSystem::GetSnowModeOption()
{
	return m_Config.bShowSnowMode;
}

void CPythonSystem::SetSnowTextureModeOption(int iOpt)
{
	m_Config.bShowSnowTextureMode = iOpt == 1 ? true : false;
}

bool CPythonSystem::GetSnowTextureModeOption()
{
	return m_Config.bShowSnowTextureMode;
}

void CPythonSystem::SetDesertTextureModeOption(int iOpt)
{
	m_Config.bShowDesertTextureMode = iOpt == 1 ? true : false;
}

bool CPythonSystem::GetDesertTextureModeOption()
{
	return m_Config.bShowDesertTextureMode;
}
#endif

#ifdef ENABLE_GRAPHIC_OPTIMIZATION
int CPythonSystem::GetEffectLevel() const
{
	return m_Config.iEffectLevel;
}
void CPythonSystem::SetEffectLevel(unsigned int level)
{
	m_Config.iEffectLevel = MIN(level, 5);
}

int CPythonSystem::GetPrivateShopLevel() const
{
	return m_Config.iPrivateShopLevel;
}
void CPythonSystem::SetPrivateShopLevel(unsigned int level)
{
	m_Config.iPrivateShopLevel = MIN(level, 5);
}

int CPythonSystem::GetDropItemLevel() const
{
	return m_Config.iDropItemLevel;
}
void CPythonSystem::SetDropItemLevel(unsigned int level)
{
	m_Config.iDropItemLevel = MIN(level, 5);
}


bool CPythonSystem::IsPetStatus()
{
	return m_Config.bPetStatus;
}
void CPythonSystem::SetPetStatusFlag(int iFlag)
{
	m_Config.bPetStatus = iFlag == 1 ? true : false;
}

bool CPythonSystem::IsMountStatus()
{
	return m_Config.bMountStatus;
}
void CPythonSystem::SetMountStatusFlag(int iFlag)
{
	m_Config.bMountStatus = iFlag == 1 ? true : false;
}

bool CPythonSystem::IsPrivateShopStatus()
{
	return m_Config.bPrivateShopStatus;
}

void CPythonSystem::SetPrivateShopStatus(int iFlag)
{
	m_Config.bPrivateShopStatus = iFlag == 1 ? true : false;
}

bool CPythonSystem::IsCharacterStatus()
{
	return m_Config.bCharacterStatus;
}

void CPythonSystem::SetCharacterStatus(int iFlag)
{
	m_Config.bCharacterStatus = iFlag == 1 ? true : false;
}

bool CPythonSystem::IsNpcNameStatus()
{
	return m_Config.bNpcNameStatus;
}
void CPythonSystem::SetNpcNameStatusFlag(int iFlag)
{
	m_Config.bNpcNameStatus = iFlag == 1 ? true : false;
}
#endif

#ifdef ENABLE_HIGHLIGHT_ITEM
bool CPythonSystem::IsShowItemHighlight()
{
	return m_Config.bShowItemHighlight;
}

void CPythonSystem::SetShowItemHighlight(int iFlag)
{
	m_Config.bShowItemHighlight = iFlag == 1 ? true : false;
}
#endif

#ifdef ENABLE_PICKUP_OPTION
bool CPythonSystem::IsPickUpMode()
{
	return m_Config.bPickUpMode;
}

void CPythonSystem::SetPickUpMode(int iFlag)
{
	m_Config.bPickUpMode = iFlag == 1 ? true : false;
}
#endif

#ifdef ENABLE_CHAT_FILTER_OPTION
bool CPythonSystem::GetChatFilterMode(BYTE chatFilterIdx)
{
	return m_Config.bChatFilter[chatFilterIdx];
}

bool CPythonSystem::IsChatFilter(BYTE chatFilter)
{
	switch (chatFilter)
	{
		case CHAT_TYPE_PARTY:
			return GetChatFilterMode(CHAT_FILTER_PARTY);
			break;

		case CHAT_TYPE_GUILD:
			return GetChatFilterMode(CHAT_FILTER_GUILD);
			break;

		case CHAT_TYPE_SHOUT:
			return GetChatFilterMode(CHAT_FILTER_SHOUT);
			break;

		default:
			return false;
	}

	return false;
}

void CPythonSystem::SetChatFilterMode(BYTE chatFilterIdx, bool mode)
{
	m_Config.bChatFilter[chatFilterIdx] = mode;
}
#endif

#ifdef ENABLE_GAME_OPTION_DLG_RENEWAL
void CPythonSystem::SetCameraMode(BYTE bFlag)
{
	m_Config.bCameraMode = bFlag;
}

BYTE CPythonSystem::GetCameraMode()
{
	return m_Config.bCameraMode;
}
#endif

void CPythonSystem::SetLowDamageMode(BYTE bFlag)
{
	m_Config.bLowDamageMode = bFlag;
}

BYTE CPythonSystem::GetLowDamageMode()
{
	return m_Config.bLowDamageMode;
}

bool CPythonSystem::LoadConfig()
{
	FILE * fp = nullptr;

	if (nullptr == (fp = fopen("phebia.cfg", "rt")))
		return false;

	char buf[256];
	char command[256];
	char value[256];

	while (fgets(buf, 256, fp))
	{
		if (sscanf(buf, " %s %s\n", command, value) == EOF)
			break;

		if (!stricmp(command, "WIDTH"))
			m_Config.width		= atoi(value);
		else if (!stricmp(command, "HEIGHT"))
			m_Config.height	= atoi(value);
		else if (!stricmp(command, "BPP"))
			m_Config.bpp		= atoi(value);
		else if (!stricmp(command, "FREQUENCY"))
			m_Config.frequency = atoi(value);
		else if (!stricmp(command, "SOFTWARE_CURSOR"))
			m_Config.is_software_cursor = atoi(value) ? true : false;
		else if (!stricmp(command, "OBJECT_CULLING"))
			m_Config.is_object_culling = atoi(value) ? true : false;
		else if (!stricmp(command, "VISIBILITY"))
			m_Config.iDistance = atoi(value);
		else if (!stricmp(command, "MUSIC_VOLUME")) {
			if(strchr(value, '.') == 0) { // Old compatiability
				m_Config.music_volume = pow(10.0f, (-1.0f + (((float) atoi(value)) / 5.0f)));
				if(atoi(value) == 0)
					m_Config.music_volume = 0.0f;
			} else
				m_Config.music_volume = atof(value);
		} else if (!stricmp(command, "VOICE_VOLUME"))
			m_Config.voice_volume = (char) atoi(value);
		else if (!stricmp(command, "GAMMA"))
			m_Config.gamma = atoi(value);
		else if (!stricmp(command, "IS_SAVE_ID"))
			m_Config.isSaveID = atoi(value);
		else if (!stricmp(command, "SAVE_ID"))
			strncpy(m_Config.SaveID, value, 20);
		else if (!stricmp(command, "PRE_LOADING_DELAY_TIME"))
			g_iLoadingDelayTime = atoi(value);
		else if (!stricmp(command, "WINDOWED"))
		{
			m_Config.bWindowed = atoi(value) == 1 ? true : false;
		}
		else if (!stricmp(command, "USE_DEFAULT_IME"))
			m_Config.bUseDefaultIME = atoi(value) == 1 ? true : false;
		else if (!stricmp(command, "SOFTWARE_TILING"))
			m_Config.bSoftwareTiling = atoi(value);
		else if (!stricmp(command, "SHADOW_LEVEL"))
			m_Config.iShadowLevel = atoi(value);
		else if (!stricmp(command, "DECOMPRESSED_TEXTURE"))
			m_Config.bDecompressDDS = atoi(value) == 1 ? true : false;
		else if (!stricmp(command, "NO_SOUND_CARD"))
			m_Config.bNoSoundCard = atoi(value) == 1 ? true : false;
		else if (!stricmp(command, "VIEW_CHAT"))
			m_Config.bViewChat = atoi(value) == 1 ? true : false;
		else if (!stricmp(command, "ALWAYS_VIEW_NAME"))
			m_Config.bAlwaysShowName = atoi(value) == 1 ? true : false;
		else if (!stricmp(command, "SHOW_DAMAGE"))
			m_Config.bShowDamage = atoi(value) == 1 ? true : false;
		else if (!stricmp(command, "SHOW_SALESTEXT"))
			m_Config.bShowSalesText = atoi(value) == 1 ? true : false;
		else if (!stricmp(command, "FOG_MODE_ON"))
			m_Config.bFogMode = atoi(value) == 1 ? true : false;
#if defined(ENABLE_MOB_AGGR_LVL_INFO)
		else if (!stricmp(command, "SHOW_MOBLEVEL"))
			m_Config.bShowMobLevel = atoi(value) == 1 ? true : false;
		else if (!stricmp(command, "SHOW_MOBAIFLAG"))
			m_Config.bShowMobAIFlag = atoi(value) == 1 ? true : false;
#endif
#ifdef ENABLE_ENVIRONMENT_EFFECT_OPTION
		else if (!stricmp(command, "NIGHT_MODE_ON"))
			m_Config.bShowNightMode = atoi(value) == 1 ? true : false;
		else if (!stricmp(command, "SNOW_MODE_ON"))
			m_Config.bShowSnowMode = atoi(value) == 1 ? true : false;
		else if (!stricmp(command, "SNOW_TEXTURE_MODE"))
			m_Config.bShowSnowTextureMode = atoi(value) == 1 ? true : false;
		else if (!stricmp(command, "DESERT_TEXTURE_MODE"))
			m_Config.bShowDesertTextureMode = atoi(value) == 1 ? true : false;
#endif
#ifdef ENABLE_GRAPHIC_OPTIMIZATION
		else if (!stricmp(command, "EFFECT_LEVEL"))
			m_Config.iEffectLevel = atoi(value);
		else if (!stricmp(command, "PRIVATE_SHOP_LEVEL"))
			m_Config.iPrivateShopLevel = atoi(value);
		else if (!stricmp(command, "DROP_ITEM_LEVEL"))
			m_Config.iDropItemLevel = atoi(value);
		else if (!stricmp(command, "PET_STATUS"))
			m_Config.bPetStatus = atoi(value) == 1 ? true : false;
		else if (!stricmp(command, "MOUNT_STATUS"))
			m_Config.bMountStatus = atoi(value) == 1 ? true : false;
		else if (!stricmp(command, "PRIVATE_SHOP_STATUS"))
			m_Config.bPrivateShopStatus = atoi(value) == 1 ? true : false;
		else if (!stricmp(command, "CHARACTER_STATUS"))
			m_Config.bCharacterStatus = atoi(value) == 1 ? true : false;
		else if (!stricmp(command, "NPC_NAME_STATUS"))
			m_Config.bNpcNameStatus = atoi(value) == 1 ? true : false;
#endif
#ifdef ENABLE_HIGHLIGHT_ITEM
		else if (!stricmp(command, "SHOW_ITEM_HIGHLIGHT"))
			m_Config.bShowItemHighlight = atoi(value) == 1 ? true : false;
#endif // ENABLE_HIGHLIGHT_ITEM
#ifdef ENABLE_PICKUP_OPTION
		else if (!stricmp(command, "PICKUP_MODE"))
			m_Config.bPickUpMode = atoi(value) == 1 ? true : false;
		else if (!stricmp(command, "GOLD_INFO"))
			m_Config.bGoldInfo = atoi(value) == 1 ? true : false;
		else if (!stricmp(command, "EXP_INFO"))
			m_Config.bExpInfo = atoi(value) == 1 ? true : false;
#endif // ENABLE_PICKUP_OPTION
#ifdef ENABLE_GAME_OPTION_DLG_RENEWAL
		else if (!stricmp(command, "CAMERA_MODE"))
			m_Config.bCameraMode = atoi(value) == 1 ? true : false;
#endif // ENABLE_GAME_OPTION_DLG_RENEWAL
#ifdef ENABLE_FOV_OPTION
		else if (!stricmp(command, "FIELD_OF_VIEW"))
			m_Config.iFOVLevel = atoi(value);
#endif // ENABLE_FOV_OPTION
#ifdef ENABLE_RANK_SYSTEM
		else if (!stricmp(command, "SHOW_RANK"))
			m_Config.bShowRank = atoi(value) == 1 ? true : false;
#endif // ENABLE_RANK_SYSTEM
#ifdef ENABLE_LANDRANK_SYSTEM
		else if (!stricmp(command, "SHOW_LANDRANK"))
			m_Config.bShowLandRank = atoi(value) == 1 ? true : false;
#endif // ENABLE_LANDRANK_SYSTEM
#ifdef ENABLE_TEAM_SYSTEM
		else if (!stricmp(command, "SHOW_TEAM"))
			m_Config.bShowTeam = atoi(value) == 1 ? true : false;
#endif // ENABLE_TEAM_SYSTEM
#ifdef ENABLE_REBORN_SYSTEM
		else if (!stricmp(command, "SHOW_REBORN"))
			m_Config.bShowReborn = atoi(value) == 1 ? true : false;
#endif // ENABLE_REBORN_SYSTEM
#ifdef ENABLE_MONIKER_SYSTEM
		else if (!stricmp(command, "SHOW_MONIKER"))
			m_Config.bShowMoniker = atoi(value) == 1 ? true : false;
#endif // ENABLE_MONIKER_SYSTEM
#ifdef ENABLE_MODEL_RENDER_TARGET
		else if (!stricmp(command, "SHOW_RENDER_TARGET"))
			m_Config.bShowRenderTarget = atoi(value) == 1 ? true : false;
#endif // ENABLE_MODEL_RENDER_TARGET
#ifdef ENABLE_DAMAGE_DOT
		else if (!stricmp(command, "SHOW_DAMAGE_DOT"))
			m_Config.bShowDamageDot = atoi(value) == 1 ? true : false;
		else if (!stricmp(command, "SHOW_DAMAGE_LETTER"))
			m_Config.bShowDamageLetter = atoi(value) == 1 ? true : false;
#endif // ENABLE_DAMAGE_DOT
#ifdef ENABLE_HIDE_BODY_PARTS
		else if (!stricmp(command, "HIDE_COSTUME_BODY"))
			m_Config.bHideCostumeBody = atoi(value) == 1 ? true : false;
		else if (!stricmp(command, "HIDE_COSTUME_HAIR"))
			m_Config.bHideCostumeHair = atoi(value) == 1 ? true : false;
		else if (!stricmp(command, "HIDE_COSTUME_WEAPON"))
			m_Config.bHideCostumeWeapon = atoi(value) == 1 ? true : false;
		else if (!stricmp(command, "HIDE_COSTUME_ACCE"))
			m_Config.bHideCostumeAcce = atoi(value) == 1 ? true : false;
		else if (!stricmp(command, "HIDE_ACCE"))
			m_Config.bHideAcce = atoi(value) == 1 ? true : false;
		else if (!stricmp(command, "HIDE_AURA"))
			m_Config.bHideAura = atoi(value) == 1 ? true : false;
#endif // ENABLE_HIDE_BODY_PARTS
		else if (!stricmp(command, "STONE_LEVEL"))
			m_Config.iStoneSize = atoi(value);
		else if (!stricmp(command, "MOB_SIZE"))
			m_Config.iMobSize = atoi(value);
		else if (!stricmp(command, "PET_SIZE"))
			m_Config.iPetSize = atoi(value);
		else if (!stricmp(command, "DAMAGE_SCALE"))
			m_Config.iDamageSize = atoi(value);
		else if (!stricmp(command, "LOW_DAMAGE_MODE"))
			m_Config.bLowDamageMode = atoi(value);
#ifdef ENABLE_DOG_MODE
		else if (!stricmp(command, "DOG_MODE_ON"))
			m_Config.bDogMode = atoi(value) == 1 ? true : false;
#endif
#ifdef ENABLE_CHAT_FILTER_OPTION
		for (BYTE x = 0; x < CHAT_TYPE_MAX_NUM; ++x)
		{
			char filterName[64];
			snprintf(filterName, sizeof(filterName), "CHAT_FILTER_%02d", x);
			if (!stricmp(command, filterName))
				m_Config.bChatFilter[x] = atoi(value) == 1 ? true : false;
		}
#endif

	}

	if (m_Config.bWindowed)
	{
		unsigned screen_width = GetSystemMetrics(SM_CXFULLSCREEN);
		unsigned screen_height = GetSystemMetrics(SM_CYFULLSCREEN);

		if (m_Config.width >= screen_width)
		{
			m_Config.width = screen_width;
		}
		if (m_Config.height >= screen_height)
		{
			m_Config.height = screen_height;
		}
	}

	if (m_Config.width < 800)
		m_Config.width = 800;

	if (m_Config.height < 600)
		m_Config.height = 600;


	m_OldConfig = m_Config;

	fclose(fp);

	return true;
}

bool CPythonSystem::SaveConfig()
{
	FILE *fp;

	if (nullptr == (fp = fopen("phebia.cfg", "wt")))
		return false;

	fprintf(fp, "WIDTH						%d\n"
				"HEIGHT						%d\n"
				"BPP						%d\n"
				"FREQUENCY					%d\n"
				"SOFTWARE_CURSOR			%d\n"
				"OBJECT_CULLING				%d\n"
				"VISIBILITY					%d\n"
				"MUSIC_VOLUME				%.3f\n"
				"VOICE_VOLUME				%d\n"
				"GAMMA						%d\n"
				"IS_SAVE_ID					%d\n"
				"SAVE_ID					%s\n"
				"PRE_LOADING_DELAY_TIME		%d\n"
				"DECOMPRESSED_TEXTURE		%d\n",
				m_Config.width,
				m_Config.height,
				m_Config.bpp,
				m_Config.frequency,
				m_Config.is_software_cursor,
				m_Config.is_object_culling,
				m_Config.iDistance,
				m_Config.music_volume,
				m_Config.voice_volume,
				m_Config.gamma,
				m_Config.isSaveID,
				m_Config.SaveID,
				g_iLoadingDelayTime,
				m_Config.bDecompressDDS);

	if (m_Config.bWindowed == 1)
		fprintf(fp, "WINDOWED				%d\n", m_Config.bWindowed);
	if (m_Config.bViewChat == 0)
		fprintf(fp, "VIEW_CHAT				%d\n", m_Config.bViewChat);
	if (m_Config.bAlwaysShowName != DEFAULT_VALUE_ALWAYS_SHOW_NAME)
		fprintf(fp, "ALWAYS_VIEW_NAME		%d\n", m_Config.bAlwaysShowName);
	if (m_Config.bShowDamage == 0)
		fprintf(fp, "SHOW_DAMAGE		%d\n", m_Config.bShowDamage);
	if (m_Config.bShowSalesText == 0)
		fprintf(fp, "SHOW_SALESTEXT		%d\n", m_Config.bShowSalesText);
	if (m_Config.bFogMode == 0)
		fprintf(fp, "FOG_MODE_ON	%d\n", m_Config.bFogMode);

#if defined(ENABLE_MOB_AGGR_LVL_INFO)
	fprintf(fp, "SHOW_MOBLEVEL		%d\n", m_Config.bShowMobLevel);
	fprintf(fp, "SHOW_MOBAIFLAG		%d\n", m_Config.bShowMobAIFlag);
#endif // ENABLE_MOB_AGGR_LVL_INFO
#ifdef ENABLE_ENVIRONMENT_EFFECT_OPTION
	fprintf(fp, "NIGHT_MODE_ON			%d\n", m_Config.bShowNightMode);
	fprintf(fp, "SNOW_MODE_ON			%d\n", m_Config.bShowSnowMode);
	fprintf(fp, "SNOW_TEXTURE_MODE		%d\n", m_Config.bShowSnowTextureMode);
	fprintf(fp, "DESERT_TEXTURE_MODE	%d\n", m_Config.bShowDesertTextureMode);
#endif // ENABLE_ENVIRONMENT_EFFECT_OPTION
#ifdef ENABLE_GRAPHIC_OPTIMIZATION
	fprintf(fp, "EFFECT_LEVEL	%d\n", m_Config.iEffectLevel);
	fprintf(fp, "PRIVATE_SHOP_LEVEL	%d\n", m_Config.iPrivateShopLevel);
	fprintf(fp, "DROP_ITEM_LEVEL	%d\n", m_Config.iDropItemLevel);
	if (m_Config.bPetStatus	== 0)
		fprintf(fp, "PET_STATUS	%d\n", m_Config.bPetStatus);
	if (m_Config.bMountStatus == 0)
		fprintf(fp, "MOUNT_STATUS	%d\n", m_Config.bMountStatus);
	if (m_Config.bPrivateShopStatus == 0)
		fprintf(fp, "PRIVATE_SHOP_STATUS	%d\n", m_Config.bPrivateShopStatus);
	if (m_Config.bCharacterStatus == 0)
		fprintf(fp, "CHARACTER_STATUS		%d\n", m_Config.bCharacterStatus);
	if (m_Config.bNpcNameStatus	== 0)
		fprintf(fp, "NPC_NAME_STATUS	%d\n", m_Config.bNpcNameStatus);
#endif // ENABLE_GRAPHIC_OPTIMIZATION
#ifdef ENABLE_HIGHLIGHT_ITEM
	fprintf(fp, "SHOW_ITEM_HIGHLIGHT			%d\n", m_Config.bShowItemHighlight);
#endif // ENABLE_HIGHLIGHT_ITEM
#ifdef ENABLE_PICKUP_OPTION
	fprintf(fp, "PICKUP_MODE			%d\n", m_Config.bPickUpMode);
	fprintf(fp, "GOLD_INFO				%d\n", m_Config.bGoldInfo);
	fprintf(fp, "EXP_INFO				%d\n", m_Config.bExpInfo);
#endif // ENABLE_PICKUP_OPTION
#ifdef ENABLE_GAME_OPTION_DLG_RENEWAL
	fprintf(fp, "CAMERA_MODE			%d\n", m_Config.bCameraMode);
#endif // ENABLE_GAME_OPTION_DLG_RENEWAL
#ifdef ENABLE_FOV_OPTION
	fprintf(fp, "FIELD_OF_VIEW			%.1f\n", m_Config.iFOVLevel);
#endif // ENABLE_FOV_OPTION
#ifdef ENABLE_RANK_SYSTEM
	fprintf(fp, "SHOW_RANK				%d\n", m_Config.bShowRank);
#endif // ENABLE_RANK_SYSTEM
#ifdef ENABLE_LANDRANK_SYSTEM
	fprintf(fp, "SHOW_LANDRANK			%d\n", m_Config.bShowLandRank);
#endif // ENABLE_LANDRANK_SYSTEM
#ifdef ENABLE_TEAM_SYSTEM
	fprintf(fp, "SHOW_TEAM				%d\n", m_Config.bShowTeam);
#endif // ENABLE_TEAM_SYSTEM
#ifdef ENABLE_REBORN_SYSTEM
	fprintf(fp, "SHOW_REBORN			%d\n", m_Config.bShowReborn);
#endif // ENABLE_REBORN_SYSTEM
#ifdef ENABLE_MONIKER_SYSTEM
	fprintf(fp, "SHOW_MONIKER			%d\n", m_Config.bShowMoniker);
#endif // ENABLE_MONIKER_SYSTEM
#ifdef ENABLE_MODEL_RENDER_TARGET
	fprintf(fp, "SHOW_RENDER_TARGET		%d\n", m_Config.bShowRenderTarget);
#endif // ENABLE_MODEL_RENDER_TARGET
#ifdef ENABLE_DAMAGE_DOT
	fprintf(fp, "SHOW_DAMAGE_DOT		%d\n", m_Config.bShowDamageDot);
	fprintf(fp, "SHOW_DAMAGE_LETTER		%d\n", m_Config.bShowDamageLetter);
#endif // ENABLE_DAMAGE_DOT
#ifdef ENABLE_HIDE_BODY_PARTS
	fprintf(fp, "HIDE_COSTUME_BODY		%d\n", m_Config.bHideCostumeBody == true ? 1 : 0);
	fprintf(fp, "HIDE_COSTUME_HAIR		%d\n", m_Config.bHideCostumeHair == true ? 1 : 0);
	fprintf(fp, "HIDE_COSTUME_WEAPON	%d\n", m_Config.bHideCostumeWeapon == true ? 1 : 0);
	fprintf(fp, "HIDE_COSTUME_ACCE		%d\n", m_Config.bHideCostumeAcce == true ? 1 : 0);
	fprintf(fp, "HIDE_ACCE				%d\n", m_Config.bHideAcce == true ? 1 : 0);
	fprintf(fp, "HIDE_AURA				%d\n", m_Config.bHideAura == true ? 1 : 0);
#endif // ENABLE_HIDE_BODY_PARTS
	fprintf(fp, "STONE_LEVEL			%d\n", m_Config.iStoneSize);
	fprintf(fp, "MOB_LEVEL				%d\n", m_Config.iMobSize);
	fprintf(fp, "PET_SIZE				%d\n", m_Config.iPetSize);
	fprintf(fp, "DAMAGE_SCALE			%d\n", m_Config.iDamageSize);
	fprintf(fp, "LOW_DAMAGE_MODE		%d\n", m_Config.bLowDamageMode);
#ifdef ENABLE_DOG_MODE
	fprintf(fp, "DOG_MODE_ON			%d\n", m_Config.bDogMode);
#endif
#ifdef ENABLE_CHAT_FILTER_OPTION
	for (BYTE x = 0; x < CHAT_FILTER_MAX_NUM; ++x)
		fprintf(fp, "CHAT_FILTER_%02d			%d\n", x, m_Config.bChatFilter[x] == true ? 1 : 0);
#endif

	fprintf(fp, "USE_DEFAULT_IME		%d\n", m_Config.bUseDefaultIME);
	fprintf(fp, "SOFTWARE_TILING		%d\n", m_Config.bSoftwareTiling);
	fprintf(fp, "SHADOW_LEVEL			%d\n", m_Config.iShadowLevel);
	fprintf(fp, "\n");

	fclose(fp);
	return true;
}

#ifdef ENABLE_HIDE_BODY_PARTS
void CPythonSystem::SetUpdateInstance(BYTE setting, int iOpt)
{
	CInstanceBase* pInstance = CPythonCharacterManager::Instance().GetMainInstancePtr();
	if (pInstance != nullptr)
	{
		pInstance->UpdatePartsBySetting(setting, iOpt);
	}
}
#endif // ENABLE_HIDE_BODY_PARTS

#ifdef ENABLE_GAME_OPTION_DLG_RENEWAL
constexpr BYTE CAMERA_DISTANCE_MAX = 3;
constexpr float cameraDistanceList[3] = { 2500.0f, 3500.0f, 5000.0f };
#endif

void CPythonSystem::ApplyConfig()
{
	if (m_OldConfig.gamma != m_Config.gamma)
	{
		float val = 1.0f;

		switch (m_Config.gamma)
		{
			case 0:
				val = 0.4f;
				break;
			case 1:
				val = 0.7f;
				break;
			case 2:
				val = 1.0f;
				break;
			case 3:
				val = 1.2f;
				break;
			case 4:
				val = 1.4f;
				break;
		}

		CPythonGraphic::Instance().SetGamma(val);
	}

	if (m_OldConfig.is_software_cursor != m_Config.is_software_cursor)
	{
		if (m_Config.is_software_cursor)
			CPythonApplication::Instance().SetCursorMode(CPythonApplication::CURSOR_MODE_SOFTWARE);
		else
			CPythonApplication::Instance().SetCursorMode(CPythonApplication::CURSOR_MODE_HARDWARE);
	}


#ifdef ENABLE_GAME_OPTION_DLG_RENEWAL
	if (m_OldConfig.bCameraMode != m_Config.bCameraMode)
		CCamera::SetCameraMaxDistance(cameraDistanceList[m_Config.bCameraMode]);
#endif // ENABLE_GAME_OPTION_DLG_RENEWAL

	m_OldConfig = m_Config;

	ChangeSystem();
}

#ifdef ENABLE_DOG_MODE
void CPythonSystem::SetDogMode(int iFlag)
{
	m_Config.bDogMode = iFlag == 1 ? true : false;

	CPythonCharacterManager& rkChrMgr = CPythonCharacterManager::Instance();
	rkChrMgr.SetDogMode(m_Config.bDogMode);
}

bool CPythonSystem::GetDogMode()
{
	return m_Config.bDogMode;
}
#endif

std::string CPythonSystem::GetCpuID()
{
	SYSTEM_INFO kSystemInfo;
	GetSystemInfo(&kSystemInfo);

	std::string stTemp;
	char szNum[15 + 1];
#define AddNumber(num) _itoa_s(num, szNum, 10), stTemp += szNum
	AddNumber(kSystemInfo.wProcessorArchitecture);
	AddNumber(kSystemInfo.dwNumberOfProcessors);
	AddNumber(kSystemInfo.dwProcessorType);
	AddNumber(kSystemInfo.wProcessorLevel);
	AddNumber(kSystemInfo.wProcessorRevision);
	AddNumber(IsProcessorFeaturePresent(PF_3DNOW_INSTRUCTIONS_AVAILABLE));
	AddNumber(IsProcessorFeaturePresent(PF_CHANNELS_ENABLED));
	AddNumber(IsProcessorFeaturePresent(PF_COMPARE_EXCHANGE_DOUBLE));
	AddNumber(IsProcessorFeaturePresent(PF_FLOATING_POINT_EMULATED));
	AddNumber(IsProcessorFeaturePresent(PF_MMX_INSTRUCTIONS_AVAILABLE));
	AddNumber(IsProcessorFeaturePresent(PF_PAE_ENABLED));
	AddNumber(IsProcessorFeaturePresent(PF_RDTSC_INSTRUCTION_AVAILABLE));
	AddNumber(IsProcessorFeaturePresent(PF_XMMI_INSTRUCTIONS_AVAILABLE));
	AddNumber(IsProcessorFeaturePresent(PF_XMMI64_INSTRUCTIONS_AVAILABLE));
#undef AddNumber

	return stTemp;
}

std::string CPythonSystem::GUIDtoString(GUID guid)
{
	char buff[256];

	snprintf(buff, sizeof(buff), "%08lX-%04hX-%04hX-%02hhX%02hhX-%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX",
		guid.Data1, guid.Data2, guid.Data3,
		guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3],
		guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);

	return buff;
}

std::string CPythonSystem::GetGpuID()
{
	IDirect3D8* d9object = Direct3DCreate8(D3D_SDK_VERSION);
	unsigned int adaptercount = d9object->GetAdapterCount();
	D3DADAPTER_IDENTIFIER8* adapters = new D3DADAPTER_IDENTIFIER8[sizeof(adaptercount)];

	for (unsigned int i = 0; i < adaptercount; i++) {
		d9object->GetAdapterIdentifier(i, 0, &(adapters[i]));
	}

	return GUIDtoString(adapters->DeviceIdentifier);
}

bool CPythonSystem::getDMI(std::vector<uint8_t>& buffer)
{
	const BYTE byteSignature[] = { 'B', 'M', 'S', 'R' };
	const DWORD signature = *((DWORD*)byteSignature);

	DWORD size = GetSystemFirmwareTable(signature, 0, NULL, 0);
	if (size == 0) return false;
	buffer.resize(size, 0);

	if (size != GetSystemFirmwareTable(signature, 0, buffer.data(), size))
	{
		buffer.clear();
		return false;
	}

	return true;
}

typedef struct _hwidData
{
	std::string biosVendor;
	std::string biosVersion;
	std::string biosRelease;
	std::string biosSize;
	std::string biosSerial;
	std::string biosUuid;
} hwidData;

hwidData printSMBIOS(smbios::Parser& parser)
{
	hwidData data;
	int version = parser.version();
	const smbios::Entry* entry = NULL;

	while (true)
	{
		entry = parser.next();
		if (entry == NULL) break;

		if (entry->type == DMI_TYPE_BIOS)
		{
			if (version >= smbios::SMBIOS_2_0)
			{
				data.biosVendor = entry->data.bios.Vendor;
				data.biosVersion = entry->data.bios.BIOSVersion;
				data.biosRelease = entry->data.bios.BIOSReleaseDate;
				data.biosSize = std::to_string((((int)entry->data.bios.BIOSROMSize + 1) * 64));
			}
		}
		else if (entry->type == DMI_TYPE_BASEBOARD)
		{
			if (version >= smbios::SMBIOS_2_0)
			{
				data.biosSerial = entry->data.baseboard.SerialNumber;
			}
		}
		else if (entry->type == DMI_TYPE_SYSINFO)
		{
			if (version >= smbios::SMBIOS_2_1)
			{
				std::stringstream stream;
				for (size_t i = 0; i < 16; ++i)
					stream << std::hex << std::setw(2) << std::setfill('0') << (int)entry->data.sysinfo.UUID[i] << ' ';

				data.biosUuid = stream.str();
			}
		}
	}

	return data;
}

std::string CPythonSystem::GenerateHash(std::string aString)
{
	std::vector<unsigned char> hash(picosha2::k_digest_size);
	picosha2::hash256(aString.begin(), aString.end(), hash.begin(), hash.end());

	std::string hex_str = picosha2::bytes_to_hex_string(hash.begin(), hash.end());

	std::transform(hex_str.begin(), hex_str.end(), hex_str.begin(), ::toupper);

	return hex_str;
}

const char* CPythonSystem::GetHWID()
{
	hwidData data;

	std::vector<uint8_t> buffer;
	bool found = false;
	found = getDMI(buffer);

	if (!found)
	{
		ExitProcess(EXIT_FAILURE);
	}

	smbios::Parser parser(buffer.data(), buffer.size());
	if (parser.valid())
		data = printSMBIOS(parser);
	else
	{
		ExitProcess(EXIT_FAILURE);
	}

	std::string result = GetCpuID() + GetGpuID() + data.biosVendor + data.biosVersion + data.biosRelease + data.biosSize + data.biosSerial + data.biosUuid;
	//std::string result = GetCpuID() + GetGpuID() + data.biosSize + data.biosSerial;

	return &GenerateHash(result)[0];
}

void CPythonSystem::ChangeSystem()
{
	CSoundManager& rkSndMgr = CSoundManager::Instance();
	rkSndMgr.SetMusicVolume(m_Config.music_volume);
	rkSndMgr.SetSoundVolumeGrade(m_Config.voice_volume);
}

void CPythonSystem::Clear()
{
#ifdef ENABLE_GAME_OPTION_DLG_RENEWAL
	m_poHandler = nullptr;
#endif
}

CPythonSystem::CPythonSystem()
{
	ZeroMemory(&m_Config, sizeof(TConfig));

#ifdef ENABLE_GAME_OPTION_DLG_RENEWAL
	m_poHandler = nullptr;
#endif

	SetDefaultConfig();
	LoadConfig();
	ChangeSystem();
#ifdef ENABLE_HIDE_BODY_PARTS
	std::fill_n(m_instancePartBackups, 0, CRaceData::PART_MAX_NUM);
	m_instancePartBackupSpecular = 0;
#endif // ENABLE_HIDE_BODY_PARTS
}

CPythonSystem::~CPythonSystem()
{
}

