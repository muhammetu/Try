#include "StdAfx.h"
#include "PythonSystem.h"

PyObject * systemGetWidth(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", CPythonSystem::Instance().GetWidth());
}

PyObject * systemGetHeight(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", CPythonSystem::Instance().GetHeight());
}
#ifdef ENABLE_GAME_OPTION_DLG_RENEWAL
PyObject* systemSetPyHandler(PyObject * poSelf, PyObject * poArgs)
{
	PyObject* poHandler;
	if (!PyTuple_GetObject(poArgs, 0, &poHandler))
		return Py_BuildException();
	
	CPythonSystem::Instance().SetPyHandler(poHandler);
	return Py_BuildNone();
}

PyObject* systemOpenDlg(PyObject * poSelf, PyObject * poArgs)
{
	CPythonSystem::Instance().OpenDlg();
	return Py_BuildNone();
}
#endif

PyObject * systemReserveResource(PyObject* poSelf, PyObject* poArgs)
{
	std::set<std::string> ResourceSet;
	CResourceManager::Instance().PushBackgroundLoadingSet(ResourceSet);
	return Py_BuildNone();
}

PyObject * systemGetConfig(PyObject * poSelf, PyObject * poArgs)
{
	CPythonSystem::TConfig *tmp = CPythonSystem::Instance().GetConfig();

	int iRes = CPythonSystem::Instance().GetResolutionIndex(tmp->width, tmp->height, tmp->bpp);
	int iFrequency = CPythonSystem::Instance().GetFrequencyIndex(iRes, tmp->frequency);

	return Py_BuildValue("iiiiiiii",  iRes,
									  iFrequency,
									  tmp->is_software_cursor,
									  tmp->is_object_culling,
									  tmp->music_volume,
									  tmp->voice_volume,
									  tmp->gamma,
									  tmp->iDistance);
}

PyObject * systemSetSaveID(PyObject * poSelf, PyObject * poArgs)
{
	int iValue;
	if (!PyTuple_GetInteger(poArgs, 0, &iValue))
		return Py_BuildException();

	char * szSaveID;
	if (!PyTuple_GetString(poArgs, 1, &szSaveID))
		return Py_BuildException();

	CPythonSystem::Instance().SetSaveID(iValue, szSaveID);
	return Py_BuildNone();
}

PyObject * systemisSaveID(PyObject * poSelf, PyObject * poArgs)
{
	int value = CPythonSystem::Instance().IsSaveID();
	return Py_BuildValue("i", value);
}

PyObject * systemGetSaveID(PyObject * poSelf, PyObject * poArgs)
{
	const char * c_szSaveID = CPythonSystem::Instance().GetSaveID();
	return Py_BuildValue("s", c_szSaveID);
}

PyObject * systemGetMusicVolume(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildValue("f", CPythonSystem::Instance().GetMusicVolume());
}

PyObject * systemGetSoundVolume(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildValue("i", CPythonSystem::Instance().GetSoundVolume());
}

PyObject * systemSetMusicVolume(PyObject * poSelf, PyObject * poArgs)
{
	float fVolume;
	if (!PyTuple_GetFloat(poArgs, 0, &fVolume))
		return Py_BuildException();

	CPythonSystem::Instance().SetMusicVolume(fVolume);
	CPythonSystem::Instance().SaveConfig();
	return Py_BuildNone();
}

PyObject * systemSetSoundVolumef(PyObject * poSelf, PyObject * poArgs)
{
	float fVolume;
	if (!PyTuple_GetFloat(poArgs, 0, &fVolume))
		return Py_BuildException();

	CPythonSystem::Instance().SetSoundVolumef(fVolume);
	CPythonSystem::Instance().SaveConfig();
	return Py_BuildNone();
}

PyObject * systemIsSoftwareCursor(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildValue("i", CPythonSystem::Instance().IsSoftwareCursor());
}

PyObject * systemSetViewChatFlag(PyObject * poSelf, PyObject * poArgs)
{
	int iFlag;
	if (!PyTuple_GetInteger(poArgs, 0, &iFlag))
		return Py_BuildException();

	CPythonSystem::Instance().SetViewChatFlag(iFlag);
	CPythonSystem::Instance().SaveConfig();
	return Py_BuildNone();
}

PyObject * systemIsViewChat(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildValue("i", CPythonSystem::Instance().IsViewChat());
}

PyObject * systemSetAlwaysShowNameFlag(PyObject * poSelf, PyObject * poArgs)
{
	int iFlag;
	if (!PyTuple_GetInteger(poArgs, 0, &iFlag))
		return Py_BuildException();

	CPythonSystem::Instance().SetAlwaysShowNameFlag(iFlag);
	CPythonSystem::Instance().SaveConfig();
	return Py_BuildNone();
}

PyObject * systemSetShowDamageFlag(PyObject * poSelf, PyObject * poArgs)
{
	int iFlag;
	if (!PyTuple_GetInteger(poArgs, 0, &iFlag))
		return Py_BuildException();

	CPythonSystem::Instance().SetShowDamageFlag(iFlag);
	CPythonSystem::Instance().SaveConfig();
	return Py_BuildNone();
}

PyObject * systemSetShowSalesTextFlag(PyObject * poSelf, PyObject * poArgs)
{
	int iFlag;
	if (!PyTuple_GetInteger(poArgs, 0, &iFlag))
		return Py_BuildException();

	CPythonSystem::Instance().SetShowSalesTextFlag(iFlag);
	CPythonSystem::Instance().SaveConfig();
	return Py_BuildNone();
}

PyObject * systemIsAlwaysShowName(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildValue("i", CPythonSystem::Instance().IsAlwaysShowName());
}

PyObject * systemIsShowDamage(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildValue("i", CPythonSystem::Instance().IsShowDamage());
}

PyObject * systemIsShowSalesText(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildValue("i", CPythonSystem::Instance().IsShowSalesText());
}

PyObject * systemSetConfig(PyObject * poSelf, PyObject * poArgs)
{
	int res_index;
	int width;
	int height;
	int bpp;
	int frequency_index;
	int frequency;
	int software_cursor;
	int shadow;
	int object_culling;
	int music_volume;
	int voice_volume;
	int gamma;
	int distance;

	if (!PyTuple_GetInteger(poArgs, 0, &res_index))
		return Py_BuildException();

	if (!PyTuple_GetInteger(poArgs, 1, &frequency_index))
		return Py_BuildException();

	if (!PyTuple_GetInteger(poArgs, 2, &software_cursor))
		return Py_BuildException();

	if (!PyTuple_GetInteger(poArgs, 3, &shadow))
		return Py_BuildException();

	if (!PyTuple_GetInteger(poArgs, 4, &object_culling))
		return Py_BuildException();

	if (!PyTuple_GetInteger(poArgs, 5, &music_volume))
		return Py_BuildException();

	if (!PyTuple_GetInteger(poArgs, 6, &voice_volume))
		return Py_BuildException();

	if (!PyTuple_GetInteger(poArgs, 7, &gamma))
		return Py_BuildException();

	if (!PyTuple_GetInteger(poArgs, 8, &distance))
		return Py_BuildException();

	if (!CPythonSystem::Instance().GetResolution(res_index, (DWORD *) &width, (DWORD *) &height, (DWORD *) &bpp))
		return Py_BuildNone();

	if (!CPythonSystem::Instance().GetFrequency(res_index,frequency_index, (DWORD *) &frequency))
		return Py_BuildNone();

	CPythonSystem::TConfig tmp;

	memcpy(&tmp, CPythonSystem::Instance().GetConfig(), sizeof(tmp));

	tmp.width				= width;
	tmp.height				= height;
	tmp.bpp					= bpp;
	tmp.frequency			= frequency;
	tmp.is_software_cursor	= software_cursor ? true : false;
	tmp.is_object_culling	= object_culling ? true : false;
	tmp.music_volume		= (char) music_volume;
	tmp.voice_volume		= (char) voice_volume;
	tmp.gamma				= gamma;
	tmp.iDistance			= distance;

	CPythonSystem::Instance().SetConfig(&tmp);
	return Py_BuildNone();
}

PyObject * systemApplyConfig(PyObject * poSelf, PyObject * poArgs)
{
	CPythonSystem::Instance().ApplyConfig();
	return Py_BuildNone();
}

PyObject * systemSaveConfig(PyObject * poSelf, PyObject * poArgs)
{
	int ret = CPythonSystem::Instance().SaveConfig();
	return Py_BuildValue("i", ret);
}

PyObject * systemGetResolutionCount(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildValue("i", CPythonSystem::Instance().GetResolutionCount());
}

PyObject * systemGetFrequencyCount(PyObject * poSelf, PyObject * poArgs)
{
	int	index;

	if (!PyTuple_GetInteger(poArgs, 0, &index))
		return Py_BuildException();

	return Py_BuildValue("i", CPythonSystem::Instance().GetFrequencyCount(index));
}

PyObject * systemGetResolution(PyObject * poSelf, PyObject * poArgs)
{
	int	index;
	DWORD width = 0, height = 0, bpp = 0;

	if (!PyTuple_GetInteger(poArgs, 0, &index))
		return Py_BuildException();

	CPythonSystem::Instance().GetResolution(index, &width, &height, &bpp);
	return Py_BuildValue("iii", width, height, bpp);
}

PyObject * systemGetCurrentResolution(PyObject * poSelf, PyObject *poArgs)
{
	CPythonSystem::TConfig *tmp = CPythonSystem::Instance().GetConfig();
	return Py_BuildValue("iii", tmp->width, tmp->height, tmp->bpp);
}

PyObject * systemGetFrequency(PyObject * poSelf, PyObject * poArgs)
{
	int	index, frequency_index;
	DWORD frequency = 0;

	if (!PyTuple_GetInteger(poArgs, 0, &index))
		return Py_BuildException();

	if (!PyTuple_GetInteger(poArgs, 1, &frequency_index))
		return Py_BuildException();

	CPythonSystem::Instance().GetFrequency(index, frequency_index, &frequency);
	return Py_BuildValue("i", frequency);
}

PyObject * systemGetShadowLevel(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildValue("i", CPythonSystem::Instance().GetShadowLevel());
}

PyObject * systemSetShadowLevel(PyObject * poSelf, PyObject * poArgs)
{
	int level;

	if (!PyTuple_GetInteger(poArgs, 0, &level))
		return Py_BuildException();

	if (level > 0)
		CPythonSystem::Instance().SetShadowLevel(level);
	CPythonSystem::Instance().SaveConfig();
	return Py_BuildNone();
}

PyObject * systemSetFogMode(PyObject * poSelf, PyObject * poArgs)
{
	int iFlag;
	if (!PyTuple_GetInteger(poArgs, 0, &iFlag))
		return Py_BuildException();

	CPythonSystem::Instance().SetFogMode(iFlag);
	CPythonSystem::Instance().SaveConfig();
	return Py_BuildNone();
}

PyObject * systemIsFogMode(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildValue("i", CPythonSystem::Instance().IsFogMode());
}

#ifdef ENABLE_ENVIRONMENT_EFFECT_OPTION
PyObject * systemSetNightModeOption(PyObject * poSelf, PyObject * poArgs)
{
	int iOpt;
	if (!PyTuple_GetInteger(poArgs, 0, &iOpt))
		return Py_BuildException();

	CPythonSystem::Instance().SetNightModeOption(iOpt);
	CPythonSystem::Instance().SaveConfig();
	return Py_BuildNone();
}

PyObject * systemGetNightModeOption(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildValue("i", CPythonSystem::Instance().GetNightModeOption());
}

PyObject * systemSetSnowModeOption(PyObject * poSelf, PyObject * poArgs)
{
	int iOpt;
	if (!PyTuple_GetInteger(poArgs, 0, &iOpt))
		return Py_BuildException();

	CPythonSystem::Instance().SetSnowModeOption(iOpt);
	CPythonSystem::Instance().SaveConfig();
	return Py_BuildNone();
}

PyObject * systemGetSnowModeOption(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildValue("i", CPythonSystem::Instance().GetSnowModeOption());
}

PyObject * systemSetSnowTextureModeOption(PyObject * poSelf, PyObject * poArgs)
{
	int iOpt;
	if (!PyTuple_GetInteger(poArgs, 0, &iOpt))
		return Py_BuildException();

	CPythonSystem::Instance().SetSnowTextureModeOption(iOpt);
	CPythonSystem::Instance().SaveConfig();
	return Py_BuildNone();
}

PyObject * systemGetSnowTextureModeOption(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildValue("i", CPythonSystem::Instance().GetSnowTextureModeOption());
}

PyObject * systemSetDesertTextureModeOption(PyObject * poSelf, PyObject * poArgs)
{
	int iOpt;
	if (!PyTuple_GetInteger(poArgs, 0, &iOpt))
		return Py_BuildException();

	CPythonSystem::Instance().SetDesertTextureModeOption(iOpt);
	CPythonSystem::Instance().SaveConfig();
	return Py_BuildNone();
}

PyObject * systemGetDesertTextureModeOption(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildValue("i", CPythonSystem::Instance().GetDesertTextureModeOption());
}
#endif

#ifdef ENABLE_GRAPHIC_OPTIMIZATION
PyObject * systemGetEffectLevel(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildValue("i", CPythonSystem::Instance().GetEffectLevel());
}
PyObject * systemSetEffectLevel(PyObject * poSelf, PyObject * poArgs)
{
	int level;

	if (!PyTuple_GetInteger(poArgs, 0, &level))
		return Py_BuildException();

	if (level >= 0)
		CPythonSystem::Instance().SetEffectLevel(level);
	CPythonSystem::Instance().SaveConfig();
	return Py_BuildNone();
}

PyObject * systemGetPrivateShopLevel(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildValue("i", CPythonSystem::Instance().GetPrivateShopLevel());
}
PyObject * systemSetPrivateShopLevel(PyObject * poSelf, PyObject * poArgs)
{
	int level;

	if (!PyTuple_GetInteger(poArgs, 0, &level))
		return Py_BuildException();

	if (level >= 0)
		CPythonSystem::Instance().SetPrivateShopLevel(level);
	CPythonSystem::Instance().SaveConfig();
	return Py_BuildNone();
}

PyObject * systemGetDropItemLevel(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildValue("i", CPythonSystem::Instance().GetDropItemLevel());
}
PyObject * systemSetDropItemLevel(PyObject * poSelf, PyObject * poArgs)
{
	int level;
	if (!PyTuple_GetInteger(poArgs, 0, &level))
		return Py_BuildException();

	if (level >= 0)
		CPythonSystem::Instance().SetDropItemLevel(level);
	CPythonSystem::Instance().SaveConfig();
	return Py_BuildNone();
}

PyObject * systemSetPetStatusFlag(PyObject * poSelf, PyObject * poArgs)
{
	int iFlag;
	if (!PyTuple_GetInteger(poArgs, 0, &iFlag))
		return Py_BuildException();

	CPythonSystem::Instance().SetPetStatusFlag(iFlag);
	CPythonSystem::Instance().SaveConfig();
	return Py_BuildNone();
}
PyObject * systemIsPetStatus(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildValue("i", CPythonSystem::Instance().IsPetStatus());
}

PyObject * systemSetMountStatusFlag(PyObject * poSelf, PyObject * poArgs)
{
	int iFlag;
	if (!PyTuple_GetInteger(poArgs, 0, &iFlag))
		return Py_BuildException();

	CPythonSystem::Instance().SetMountStatusFlag(iFlag);
	CPythonSystem::Instance().SaveConfig();
	return Py_BuildNone();
}
PyObject * systemIsMountStatus(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildValue("i", CPythonSystem::Instance().IsMountStatus());
}

PyObject* systemSetPrivateShopStatusFlag(PyObject * poSelf, PyObject * poArgs)
{
	int iFlag;
	if (!PyTuple_GetInteger(poArgs, 0, &iFlag))
		return Py_BuildException();
	
	CPythonSystem::Instance().SetPrivateShopStatus(iFlag);
	CPythonSystem::Instance().SaveConfig();
	return Py_BuildNone();
}
PyObject* systemIsPrivateShopStatus(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildValue("i", CPythonSystem::Instance().IsPrivateShopStatus());
}

PyObject* systemSetCharacterStatusFlag(PyObject * poSelf, PyObject * poArgs)
{
	int iFlag;
	if (!PyTuple_GetInteger(poArgs, 0, &iFlag))
		return Py_BuildException();
	
	CPythonSystem::Instance().SetCharacterStatus(iFlag);
	CPythonSystem::Instance().SaveConfig();
	return Py_BuildNone();
}
PyObject* systemIsCharacterStatus(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildValue("i", CPythonSystem::Instance().IsCharacterStatus());
}

PyObject * systemSetNpcNameStatusFlag(PyObject * poSelf, PyObject * poArgs)
{
	int iFlag;
	if (!PyTuple_GetInteger(poArgs, 0, &iFlag))
		return Py_BuildException();

	CPythonSystem::Instance().SetNpcNameStatusFlag(iFlag);
	CPythonSystem::Instance().SaveConfig();
	return Py_BuildNone();
}

PyObject * systemIsNpcNameStatus(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildValue("i", CPythonSystem::Instance().IsNpcNameStatus());
}
#endif

#if defined(ENABLE_MOB_AGGR_LVL_INFO)
PyObject * systemSetShowMobLevel(PyObject * poSelf, PyObject * poArgs)
{
	int iOpt;
	if (!PyTuple_GetInteger(poArgs, 0, &iOpt))
		return Py_BuildException();

	CPythonSystem::Instance().SetShowMobLevel(iOpt);
	CPythonSystem::Instance().SaveConfig();
	return Py_BuildNone();
}

PyObject * systemIsShowMobLevel(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildValue("i", CPythonSystem::Instance().IsShowMobLevel());
}

PyObject * systemSetShowMobAIFlag(PyObject * poSelf, PyObject * poArgs)
{
	int iOpt;
	if (!PyTuple_GetInteger(poArgs, 0, &iOpt))
		return Py_BuildException();

	CPythonSystem::Instance().SetShowMobAIFlag(iOpt);
	CPythonSystem::Instance().SaveConfig();
	return Py_BuildNone();
}

PyObject * systemIsShowMobAIFlag(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildValue("i", CPythonSystem::Instance().IsShowMobAIFlag());
}
#endif

#ifdef ENABLE_HIGHLIGHT_ITEM
PyObject* systemIsShowItemHighlight(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", CPythonSystem::Instance().IsShowItemHighlight());
}

PyObject* systemSetShowItemHighlight(PyObject* poSelf, PyObject* poArgs)
{
	int iFlag;
	if (!PyTuple_GetInteger(poArgs, 0, &iFlag))
		return Py_BuildException();

	CPythonSystem::Instance().SetShowItemHighlight(iFlag);
	CPythonSystem::Instance().SaveConfig();
	return Py_BuildNone();
}
#endif

#ifdef ENABLE_PICKUP_OPTION
PyObject* systemSetPickUpMode(PyObject * poSelf, PyObject * poArgs)
{
	int iOpt;
	if (!PyTuple_GetInteger(poArgs, 0, &iOpt))
		return Py_BuildException();
	
	CPythonSystem::Instance().SetPickUpMode(iOpt);
	CPythonSystem::Instance().SaveConfig();
	return Py_BuildNone();
}

PyObject* systemIsPickUpMode(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildValue("i", CPythonSystem::Instance().IsPickUpMode());
}

PyObject* systemSetGoldInfo(PyObject * poSelf, PyObject * poArgs)
{
	int iOpt;
	if (!PyTuple_GetInteger(poArgs, 0, &iOpt))
		return Py_BuildException();
	
	CPythonSystem::Instance().SetGoldInfo(iOpt);
	CPythonSystem::Instance().SaveConfig();
	return Py_BuildNone();
}

PyObject* systemIsGoldInfo(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildValue("i", CPythonSystem::Instance().IsGoldInfo());
}

PyObject* systemSetExpInfo(PyObject * poSelf, PyObject * poArgs)
{
	int iOpt;
	if (!PyTuple_GetInteger(poArgs, 0, &iOpt))
		return Py_BuildException();
	
	CPythonSystem::Instance().SetExpInfo(iOpt);
	CPythonSystem::Instance().SaveConfig();
	return Py_BuildNone();
}

PyObject* systemIsExpInfo(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildValue("i", CPythonSystem::Instance().IsExpInfo());
}
#endif

#ifdef ENABLE_CHAT_FILTER_OPTION
PyObject* systemSetChatFilterMode(PyObject * poSelf, PyObject* poArgs)
{
	int iOpt;
	if (!PyTuple_GetInteger(poArgs, 0, &iOpt))
		return Py_BuildException();

	int iMode;
	if (!PyTuple_GetInteger(poArgs, 1, &iMode))
		return Py_BuildException();

	CPythonSystem::Instance().SetChatFilterMode(iOpt, iMode);
	CPythonSystem::Instance().SaveConfig();
	return Py_BuildNone();
}

PyObject* systemGetChatFilterMode(PyObject * poSelf, PyObject* poArgs)
{
	int iOpt;
	if (!PyTuple_GetInteger(poArgs, 0, &iOpt))
		return Py_BuildException();

	return Py_BuildValue("i", CPythonSystem::Instance().GetChatFilterMode(iOpt));
}
#endif

#ifdef ENABLE_GAME_OPTION_DLG_RENEWAL
PyObject* systemSetCameraMode(PyObject* poSelf, PyObject* poArgs)
{
	int iOpt;
	if (!PyTuple_GetInteger(poArgs, 0, &iOpt))
		return Py_BuildException();
	
	CPythonSystem::Instance().SetCameraMode(iOpt);
	CPythonSystem::Instance().SaveConfig();
	return Py_BuildNone();
}

PyObject* systemGetCameraMode(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", CPythonSystem::Instance().GetCameraMode());
}
#endif

#ifdef ENABLE_FOV_OPTION
PyObject * systemSetFOVLevel(PyObject * poSelf, PyObject * poArgs)
{
	float fFOV;
	if (!PyTuple_GetFloat(poArgs, 0, &fFOV))
		return Py_BuildException();
	CPythonSystem::Instance().SetFOVLevel(fFOV);
	CPythonSystem::Instance().SaveConfig();
	return Py_BuildNone();
}

PyObject * systemGetFOVLevel(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildValue("f", CPythonSystem::Instance().GetFOVLevel());
}
#endif

#ifdef ENABLE_RANK_SYSTEM
PyObject* systemSetShowRank(PyObject * poSelf, PyObject* poArgs)
{
	int iOpt;
	if (!PyTuple_GetInteger(poArgs, 0, &iOpt))
		return Py_BuildException();
	
	CPythonSystem::Instance().SetShowRank(iOpt);
	CPythonSystem::Instance().SaveConfig();
	return Py_BuildNone();
}

PyObject* systemIsShowRank(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildValue("i", CPythonSystem::Instance().IsShowRank());
}
#endif // ENABLE_RANK_SYSTEM

#ifdef ENABLE_LANDRANK_SYSTEM
PyObject* systemSetShowLandRank(PyObject * poSelf, PyObject* poArgs)
{
	int iOpt;
	if (!PyTuple_GetInteger(poArgs, 0, &iOpt))
		return Py_BuildException();
	
	CPythonSystem::Instance().SetShowLandRank(iOpt);
	CPythonSystem::Instance().SaveConfig();
	return Py_BuildNone();
}

PyObject* systemIsShowLandRank(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildValue("i", CPythonSystem::Instance().IsShowLandRank());
}
#endif // ENABLE_LANDRANK_SYSTEM

#ifdef ENABLE_TEAM_SYSTEM
PyObject* systemSetShowTeam(PyObject * poSelf, PyObject* poArgs)
{
	int iOpt;
	if (!PyTuple_GetInteger(poArgs, 0, &iOpt))
		return Py_BuildException();
	
	CPythonSystem::Instance().SetShowTeam(iOpt);
	CPythonSystem::Instance().SaveConfig();
	return Py_BuildNone();
}

PyObject* systemIsShowTeam(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildValue("i", CPythonSystem::Instance().IsShowTeam());
}
#endif // ENABLE_TEAM_SYSTEM

#ifdef ENABLE_REBORN_SYSTEM
PyObject* systemSetShowReborn(PyObject * poSelf, PyObject* poArgs)
{
	int iOpt;
	if (!PyTuple_GetInteger(poArgs, 0, &iOpt))
		return Py_BuildException();
	
	CPythonSystem::Instance().SetShowReborn(iOpt);
	CPythonSystem::Instance().SaveConfig();
	return Py_BuildNone();
}

PyObject* systemIsShowReborn(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildValue("i", CPythonSystem::Instance().IsShowReborn());
}
#endif // ENABLE_REBORN_SYSTEM

#ifdef ENABLE_MONIKER_SYSTEM
PyObject* systemSetShowMoniker(PyObject * poSelf, PyObject* poArgs)
{
	int iOpt;
	if (!PyTuple_GetInteger(poArgs, 0, &iOpt))
		return Py_BuildException();
	
	CPythonSystem::Instance().SetShowMoniker(iOpt);
	CPythonSystem::Instance().SaveConfig();
	return Py_BuildNone();
}

PyObject* systemIsShowMoniker(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildValue("i", CPythonSystem::Instance().IsShowMoniker());
}
#endif // ENABLE_MONIKER_SYSTEM

#ifdef ENABLE_MODEL_RENDER_TARGET
PyObject* systemSetShowRenderTarget(PyObject * poSelf, PyObject* poArgs)
{
	int iOpt;
	if (!PyTuple_GetInteger(poArgs, 0, &iOpt))
		return Py_BuildException();
	
	CPythonSystem::Instance().SetShowRenderTarget(iOpt);
	CPythonSystem::Instance().SaveConfig();
	return Py_BuildNone();
}

PyObject* systemIsShowRenderTarget(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildValue("i", CPythonSystem::Instance().IsShowRenderTarget());
}
#endif // ENABLE_MODEL_RENDER_TARGET

#ifdef ENABLE_DAMAGE_DOT
PyObject* systemSetShowDamageDot(PyObject * poSelf, PyObject* poArgs)
{
	int iOpt;
	if (!PyTuple_GetInteger(poArgs, 0, &iOpt))
		return Py_BuildException();
	
	CPythonSystem::Instance().SetShowDamageDot(iOpt);
	CPythonSystem::Instance().SaveConfig();
	return Py_BuildNone();
}

PyObject* systemIsShowDamageDot(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildValue("i", CPythonSystem::Instance().IsShowDamageDot());
}

PyObject* systemSetShowDamageLetter(PyObject* poSelf, PyObject* poArgs)
{
	int iOpt;
	if (!PyTuple_GetInteger(poArgs, 0, &iOpt))
		return Py_BuildException();

	CPythonSystem::Instance().SetShowDamageLetter(iOpt);
	CPythonSystem::Instance().SaveConfig();
	return Py_BuildNone();
}

PyObject* systemIsShowDamageLetter(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", CPythonSystem::Instance().IsShowDamageLetter());
}
#endif // ENABLE_DAMAGE_DOT


#ifdef ENABLE_HIDE_BODY_PARTS
PyObject* systemSetHideCostumeBody(PyObject * poSelf, PyObject* poArgs)
{
	int iOpt;
	if (!PyTuple_GetInteger(poArgs, 0, &iOpt))
		return Py_BuildException();
	
	CPythonSystem::Instance().SetHideCostumeBody(iOpt);
	CPythonSystem::Instance().SaveConfig();
	CPythonSystem::Instance().SetUpdateInstance(CPythonSystem::HIDE_COSTUME_BODY, iOpt);
	return Py_BuildNone();
}

PyObject* systemIsHideCostumeBody(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildValue("i", CPythonSystem::Instance().IsHideCostumeBody());
}

PyObject* systemSetHideCostumeHair(PyObject * poSelf, PyObject* poArgs)
{
	int iOpt;
	if (!PyTuple_GetInteger(poArgs, 0, &iOpt))
		return Py_BuildException();
	
	CPythonSystem::Instance().SetHideCostumeHair(iOpt);
	CPythonSystem::Instance().SaveConfig();
	return Py_BuildNone();
}

PyObject* systemIsHideCostumeHair(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildValue("i", CPythonSystem::Instance().IsHideCostumeHair());
}

PyObject* systemSetHideCostumeWeapon(PyObject * poSelf, PyObject* poArgs)
{
	int iOpt;
	if (!PyTuple_GetInteger(poArgs, 0, &iOpt))
		return Py_BuildException();
	
	CPythonSystem::Instance().SetHideCostumeWeapon(iOpt);
	CPythonSystem::Instance().SaveConfig();
	return Py_BuildNone();
}

PyObject* systemIsHideCostumeWeapon(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildValue("i", CPythonSystem::Instance().IsHideCostumeWeapon());
}

PyObject* systemSetHideCostumeAcce(PyObject * poSelf, PyObject* poArgs)
{
	int iOpt;
	if (!PyTuple_GetInteger(poArgs, 0, &iOpt))
		return Py_BuildException();
	
	CPythonSystem::Instance().SetHideCostumeAcce(iOpt);
	CPythonSystem::Instance().SaveConfig();
	return Py_BuildNone();
}

PyObject* systemIsHideCostumeAcce(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildValue("i", CPythonSystem::Instance().IsHideCostumeAcce());
}

PyObject* systemSetHideAcce(PyObject * poSelf, PyObject* poArgs)
{
	int iOpt;
	if (!PyTuple_GetInteger(poArgs, 0, &iOpt))
		return Py_BuildException();
	
	CPythonSystem::Instance().SetHideAcce(iOpt);
	CPythonSystem::Instance().SaveConfig();
	CPythonSystem::Instance().SetUpdateInstance(CPythonSystem::HIDE_ACCE, iOpt);
	return Py_BuildNone();
}

PyObject* systemIsHideAcce(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildValue("i", CPythonSystem::Instance().IsHideAcce());
}

PyObject* systemSetHideAura(PyObject * poSelf, PyObject* poArgs)
{
	int iOpt;
	if (!PyTuple_GetInteger(poArgs, 0, &iOpt))
		return Py_BuildException();
	
	CPythonSystem::Instance().SetHideAura(iOpt);
	CPythonSystem::Instance().SaveConfig();
	CPythonSystem::Instance().SetUpdateInstance(CPythonSystem::HIDE_AURA, iOpt);
	return Py_BuildNone();
}

PyObject* systemIsHideAura(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildValue("i", CPythonSystem::Instance().IsHideAura());
}
#endif // ENABLE_HIDE_BODY_PARTS

#include "PythonCharacterManager.h"

PyObject* systemSetStoneSize(PyObject* poSelf, PyObject* poArgs)
{
	int level;

	if (!PyTuple_GetInteger(poArgs, 0, &level))
		return Py_BuildException();

	
	// islev
	CPythonSystem::Instance().SetStoneSize(level);
	CPythonCharacterManager::Instance().RefreshAllStone();

	CPythonSystem::Instance().SaveConfig();
	return Py_BuildNone();
}

PyObject* systemGetStoneSize(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", CPythonSystem::Instance().GetStoneSize() / 5);
}

PyObject* systemSetMobSize(PyObject* poSelf, PyObject* poArgs)
{
	int level;

	if (!PyTuple_GetInteger(poArgs, 0, &level))
		return Py_BuildException();

	
	// islev
	CPythonSystem::Instance().SetMobSize(level);
	CPythonCharacterManager::Instance().RefreshAllMobs();

	CPythonSystem::Instance().SaveConfig();
	return Py_BuildNone();
}

PyObject* systemGetMobSize(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", CPythonSystem::Instance().GetMobSize());
}

PyObject* systemSetPetSize(PyObject* poSelf, PyObject* poArgs)
{
	int level;

	if (!PyTuple_GetInteger(poArgs, 0, &level))
		return Py_BuildException();

	
	// islev
	CPythonSystem::Instance().SetPetSize(level);
	CPythonCharacterManager::Instance().RefreshAllNewPets();

	CPythonSystem::Instance().SaveConfig();
	return Py_BuildNone();
}

PyObject* systemGetPetSize(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", CPythonSystem::Instance().GetPetSize() / 5);
}

PyObject* systemSetDamageSize(PyObject* poSelf, PyObject* poArgs)
{
	int iFlag;
	if (!PyTuple_GetInteger(poArgs, 0, &iFlag))
		return Py_BuildException();

	CPythonSystem::Instance().SetDamageSize(iFlag);

	return Py_BuildNone();
}

PyObject* systemGetDamageSize(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", CPythonSystem::Instance().GetDamageSize());
}

#ifdef ENABLE_DOG_MODE
PyObject* systemSetDogMode(PyObject* poSelf, PyObject* poArgs)
{
	int iFlag;
	if (!PyTuple_GetInteger(poArgs, 0, &iFlag))
		return Py_BuildException();

	CPythonSystem::Instance().SetDogMode(iFlag);

	return Py_BuildNone();
}

PyObject* systemGetDogMode(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", CPythonSystem::Instance().GetDogMode());
}
#endif

PyObject* systemSetLowDamageMode(PyObject* poSelf, PyObject* poArgs)
{
	int iFlag;
	if (!PyTuple_GetInteger(poArgs, 0, &iFlag))
		return Py_BuildException();

	CPythonSystem::Instance().SetLowDamageMode(iFlag);

	return Py_BuildNone();
}

PyObject* systemGetLowDamageMode(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", CPythonSystem::Instance().GetLowDamageMode());
}

void initsystemSetting()
{
	static PyMethodDef s_methods[] =
	{
		{ "GetWidth",					systemGetWidth,					METH_VARARGS },
		{ "GetHeight",					systemGetHeight,				METH_VARARGS },

#ifdef ENABLE_GAME_OPTION_DLG_RENEWAL
		{ "SetPyHandler",				systemSetPyHandler,				METH_VARARGS },
		{ "OpenDlg",					systemOpenDlg,					METH_VARARGS },
#endif
		{ "ReserveResource",			systemReserveResource,			METH_VARARGS },

		{ "GetResolutionCount",			systemGetResolutionCount,		METH_VARARGS },
		{ "GetFrequencyCount",			systemGetFrequencyCount,		METH_VARARGS },

		{ "GetCurrentResolution",		systemGetCurrentResolution,		METH_VARARGS },

		{ "GetResolution",				systemGetResolution,			METH_VARARGS },
		{ "GetFrequency",				systemGetFrequency,				METH_VARARGS },

		{ "ApplyConfig",				systemApplyConfig,				METH_VARARGS },
		{ "SetConfig",					systemSetConfig,				METH_VARARGS },
		{ "SaveConfig",					systemSaveConfig,				METH_VARARGS },
		{ "GetConfig",					systemGetConfig,				METH_VARARGS },

		{ "SetSaveID",					systemSetSaveID,				METH_VARARGS },
		{ "isSaveID",					systemisSaveID,					METH_VARARGS },
		{ "GetSaveID",					systemGetSaveID,				METH_VARARGS },

		{ "GetMusicVolume",				systemGetMusicVolume,			METH_VARARGS },
		{ "GetSoundVolume",				systemGetSoundVolume,			METH_VARARGS },

		{ "SetMusicVolume",				systemSetMusicVolume,			METH_VARARGS },
		{ "SetSoundVolumef",			systemSetSoundVolumef,			METH_VARARGS },
		{ "IsSoftwareCursor",			systemIsSoftwareCursor,			METH_VARARGS },

		{ "SetViewChatFlag",			systemSetViewChatFlag,			METH_VARARGS },
		{ "IsViewChat",					systemIsViewChat,				METH_VARARGS },

		{ "SetAlwaysShowNameFlag",		systemSetAlwaysShowNameFlag,	METH_VARARGS },
		{ "IsAlwaysShowName",			systemIsAlwaysShowName,			METH_VARARGS },

		{ "SetShowDamageFlag",			systemSetShowDamageFlag,		METH_VARARGS },
		{ "IsShowDamage",				systemIsShowDamage,				METH_VARARGS },

		{ "SetShowSalesTextFlag",		systemSetShowSalesTextFlag,		METH_VARARGS },
		{ "IsShowSalesText",			systemIsShowSalesText,			METH_VARARGS },

		{ "GetShadowLevel",				systemGetShadowLevel,			METH_VARARGS },
		{ "SetShadowLevel",				systemSetShadowLevel,			METH_VARARGS },

		{ "SetFogMode",	systemSetFogMode,	METH_VARARGS },
		{ "IsFogMode",	systemIsFogMode,	METH_VARARGS },

#if defined(ENABLE_MOB_AGGR_LVL_INFO)
		{ "SetShowMobAIFlag",			systemSetShowMobAIFlag,			METH_VARARGS },
		{ "IsShowMobAIFlag",			systemIsShowMobAIFlag,			METH_VARARGS },
		{ "SetShowMobLevel",			systemSetShowMobLevel,			METH_VARARGS },
		{ "IsShowMobLevel",				systemIsShowMobLevel,			METH_VARARGS },
#endif

#ifdef ENABLE_ENVIRONMENT_EFFECT_OPTION
		{ "SetNightModeOption",			systemSetNightModeOption,		METH_VARARGS },
		{ "GetNightModeOption",			systemGetNightModeOption,		METH_VARARGS },

		{ "SetSnowModeOption",			systemSetSnowModeOption,		METH_VARARGS },
		{ "GetSnowModeOption",			systemGetSnowModeOption,		METH_VARARGS },

		{ "SetSnowTextureModeOption",	systemSetSnowTextureModeOption,	METH_VARARGS },
		{ "GetSnowTextureModeOption",	systemGetSnowTextureModeOption,	METH_VARARGS },

		{ "SetDesertTextureModeOption",	systemSetDesertTextureModeOption,	METH_VARARGS },
		{ "GetDesertTextureModeOption",	systemGetDesertTextureModeOption,	METH_VARARGS },
#endif

#ifdef ENABLE_GRAPHIC_OPTIMIZATION
		{"GetEffectLevel", systemGetEffectLevel, METH_VARARGS},
		{"SetEffectLevel", systemSetEffectLevel, METH_VARARGS},

		{"GetPrivateShopLevel", systemGetPrivateShopLevel, METH_VARARGS},
		{"SetPrivateShopLevel", systemSetPrivateShopLevel, METH_VARARGS},

		{"GetDropItemLevel", systemGetDropItemLevel, METH_VARARGS},
		{"SetDropItemLevel", systemSetDropItemLevel, METH_VARARGS},


		{"SetPetStatusFlag", systemSetPetStatusFlag, METH_VARARGS},
		{"IsPetStatus", systemIsPetStatus, METH_VARARGS},

		{"SetMountStatusFlag", systemSetMountStatusFlag, METH_VARARGS},
		{"IsMountStatus", systemIsMountStatus, METH_VARARGS},

		{"SetPrivateShopStatusFlag", systemSetPrivateShopStatusFlag, METH_VARARGS},
		{"IsPrivateShopStatus",	systemIsPrivateShopStatus, METH_VARARGS},

		{"SetCharacterStatusFlag",	systemSetCharacterStatusFlag, METH_VARARGS},
		{"IsCharacterStatusFlag",	systemIsCharacterStatus,	METH_VARARGS},

		{"SetNpcNameStatusFlag", systemSetNpcNameStatusFlag, METH_VARARGS},
		{"IsNpcNameStatus", systemIsNpcNameStatus, METH_VARARGS},
#endif

#ifdef ENABLE_HIGHLIGHT_ITEM
		{ "IsShowItemHighlight",		systemIsShowItemHighlight,		METH_VARARGS },
		{ "SetShowItemHighlight",		systemSetShowItemHighlight,		METH_VARARGS },
#endif // ENABLE_HIGHLIGHT_ITEM

#ifdef ENABLE_PICKUP_OPTION
		{"IsPickUpMode",			systemIsPickUpMode,			METH_VARARGS},
		{"SetPickUpMode",			systemSetPickUpMode,			METH_VARARGS},

		{"IsGoldInfo",				systemIsGoldInfo,			METH_VARARGS},
		{"SetGoldInfo",				systemSetGoldInfo,			METH_VARARGS},

		{"IsExpInfo",				systemIsExpInfo,			METH_VARARGS},
		{"SetExpInfo",				systemSetExpInfo,			METH_VARARGS},
#endif // ENABLE_PICKUP_OPTION

#ifdef ENABLE_CHAT_FILTER_OPTION
		{ "GetChatFilterMode",		systemGetChatFilterMode,	METH_VARARGS },
		{ "SetChatFilterMode",		systemSetChatFilterMode,	METH_VARARGS },
#endif // ENABLE_CHAT_FILTER_OPTION

#ifdef ENABLE_GAME_OPTION_DLG_RENEWAL
		{ "GetCameraMode",			systemGetCameraMode,		METH_VARARGS },
		{ "SetCameraMode",			systemSetCameraMode,		METH_VARARGS },
#endif // ENABLE_GAME_OPTION_DLG_RENEWAL

#ifdef ENABLE_FOV_OPTION
		{ "GetFOVLevel",			systemGetFOVLevel,			METH_VARARGS },
		{ "SetFOVLevel",			systemSetFOVLevel,			METH_VARARGS },
#endif // ENABLE_FOV_OPTION

#ifdef ENABLE_RANK_SYSTEM
		{ "IsShowRank",				systemIsShowRank,			METH_VARARGS },
		{ "SetShowRank",			systemSetShowRank,			METH_VARARGS },
#endif // ENABLE_RANK_SYSTEM

#ifdef ENABLE_LANDRANK_SYSTEM
		{ "IsShowLandRank",				systemIsShowLandRank,			METH_VARARGS },
		{ "SetShowLandRank",			systemSetShowLandRank,			METH_VARARGS },
#endif // ENABLE_LANDRANK_SYSTEM

#ifdef ENABLE_TEAM_SYSTEM
		{ "IsShowTeam",				systemIsShowTeam,			METH_VARARGS },
		{ "SetShowTeam",			systemSetShowTeam,			METH_VARARGS },
#endif // ENABLE_TEAM_SYSTEM

#ifdef ENABLE_REBORN_SYSTEM
		{ "IsShowReborn",			systemIsShowReborn,				METH_VARARGS },
		{ "SetShowReborn",			systemSetShowReborn,			METH_VARARGS },
#endif // ENABLE_REBORN_SYSTEM

#ifdef ENABLE_MONIKER_SYSTEM
		{ "IsShowMoniker",			systemIsShowMoniker,			METH_VARARGS },
		{ "SetShowMoniker",			systemSetShowMoniker,			METH_VARARGS },
#endif // ENABLE_MONIKER_SYSTEM

#ifdef ENABLE_MODEL_RENDER_TARGET
		{ "IsShowRenderTarget",		systemIsShowRenderTarget,		METH_VARARGS },
		{ "SetShowRenderTarget",	systemSetShowRenderTarget,		METH_VARARGS },
#endif // ENABLE_MODEL_RENDER_TARGET

#ifdef ENABLE_DAMAGE_DOT
		{ "IsShowDamageDot",		systemIsShowDamageDot,		METH_VARARGS },
		{ "SetShowDamageDot",		systemSetShowDamageDot,		METH_VARARGS },

		{ "IsShowDamageLetter",		systemIsShowDamageLetter,	METH_VARARGS },
		{ "SetShowDamageLetter",	systemSetShowDamageLetter,	METH_VARARGS },

#endif // ENABLE_DAMAGE_DOT


#ifdef ENABLE_HIDE_BODY_PARTS
		{ "IsHideCostumeBody",		systemIsHideCostumeBody,	METH_VARARGS },
		{ "SetHideCostumeBody",		systemSetHideCostumeBody,	METH_VARARGS },

		{ "IsHideCostumeHair",		systemIsHideCostumeHair,	METH_VARARGS },
		{ "SetHideCostumeHair",		systemSetHideCostumeHair,	METH_VARARGS },

		{ "IsHideCostumeWeapon",	systemIsHideCostumeWeapon,	METH_VARARGS },
		{ "SetHideCostumeWeapon",	systemSetHideCostumeWeapon,	METH_VARARGS },

		{ "IsHideCostumeAcce",		systemIsHideCostumeAcce,	METH_VARARGS },
		{ "SetHideCostumeAcce",		systemSetHideCostumeAcce,	METH_VARARGS },

		{ "IsHideAcce",				systemIsHideAcce,			METH_VARARGS },
		{ "SetHideAcce",			systemSetHideAcce,			METH_VARARGS },

		{ "IsHideAura",				systemIsHideAura,			METH_VARARGS },
		{ "SetHideAura",			systemSetHideAura,			METH_VARARGS },
#endif // ENABLE_HIDE_BODY_PARTS

		{ "SetStoneSize",			systemSetStoneSize,			METH_VARARGS },
		{ "GetStoneSize",			systemGetStoneSize,			METH_VARARGS },

		{ "SetMobSize",				systemSetMobSize,			METH_VARARGS },
		{ "GetMobSize",				systemGetMobSize,			METH_VARARGS },

#ifdef ENABLE_DOG_MODE
		{ "SetDogMode",				systemSetDogMode,				METH_VARARGS },
		{ "GetDogMode",				systemGetDogMode,				METH_VARARGS },
#endif

		{ "SetPetSize",				systemSetPetSize,			METH_VARARGS },
		{ "GetPetSize",				systemGetPetSize,			METH_VARARGS },

		{ "SetDamageSize",			systemSetDamageSize,			METH_VARARGS },
		{ "GetDamageSize",			systemGetDamageSize,			METH_VARARGS },

		{ "SetLowDamageMode",			systemSetLowDamageMode,			METH_VARARGS },
		{ "GetLowDamageMode",			systemGetLowDamageMode,			METH_VARARGS },

		{ NULL,							NULL,							NULL }
	};

	PyObject * poModule = Py_InitModule("systemSetting", s_methods);

#ifdef ENABLE_GAME_OPTION_DLG_RENEWAL
	PyModule_AddIntConstant(poModule, "PAGE_GAME",			CPythonSystem::PAGE_GAME);
	PyModule_AddIntConstant(poModule, "PAGE_GRAPHIC",		CPythonSystem::PAGE_GRAPHIC);
	PyModule_AddIntConstant(poModule, "PAGE_SOUND",			CPythonSystem::PAGE_SOUND);

	PyModule_AddIntConstant(poModule, "OPTION_CAT_GAME_CHARACTER",		CPythonSystem::OPTION_CAT_GAME_CHARACTER);
	PyModule_AddIntConstant(poModule, "OPTION_CAT_GAME_USER_INTERFACE",		CPythonSystem::OPTION_CAT_GAME_USER_INTERFACE);
	PyModule_AddIntConstant(poModule, "OPTION_CAT_GAME_PRIVACY",		CPythonSystem::OPTION_CAT_GAME_PRIVACY);
	PyModule_AddIntConstant(poModule, "OPTION_CAT_GAME_CHAT_FILTER",	CPythonSystem::OPTION_CAT_GAME_CHAT_FILTER);
	PyModule_AddIntConstant(poModule, "OPTION_CAT_GRAPHIC_DISPLAY",		CPythonSystem::OPTION_CAT_GRAPHIC_DISPLAY);
	PyModule_AddIntConstant(poModule, "OPTION_CAT_GRAPHIC_ADVANCED",		CPythonSystem::OPTION_CAT_GRAPHIC_ADVANCED);
	PyModule_AddIntConstant(poModule, "OPTION_CAT_GRAPHIC_EFFECT",		CPythonSystem::OPTION_CAT_GRAPHIC_EFFECT);
	PyModule_AddIntConstant(poModule, "OPTION_CAT_GRAPHIC_EQUIPMENT",	CPythonSystem::OPTION_CAT_GRAPHIC_EQUIPMENT);
	PyModule_AddIntConstant(poModule, "OPTION_CAT_SOUND_GENERAL",		CPythonSystem::OPTION_CAT_SOUND_GENERAL);
#endif
}
