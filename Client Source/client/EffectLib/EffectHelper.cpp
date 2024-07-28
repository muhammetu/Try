#include "StdAfx.h"

#ifdef ENABLE_GRAPHIC_OPTIMIZATION
#include "EffectHelper.h"
#include "../ScriptLib/Stdafx.h"
#include "../UserInterface/PythonSystem.h"

extern class CActorInstance;

bool EffectHelper::GetEffectRenderStatus(bool isMain, BYTE actorType)
{
	BYTE effectLevel = CPythonSystem::Instance().GetEffectLevel();
	bool petLevel = CPythonSystem::Instance().IsPetStatus();
	if (petLevel == false && actorType == TYPE_PET)
		return false;
	bool mountLevel = CPythonSystem::Instance().IsMountStatus();
	if (mountLevel == false && actorType == TYPE_MOUNT)
		return false;
	bool shopLevel = CPythonSystem::Instance().IsPrivateShopStatus();
	if (shopLevel == false && actorType == TYPE_SHOP)
		return false;
	bool characterStatus = CPythonSystem::Instance().IsCharacterStatus();
	if (characterStatus == false && actorType == TYPE_PC && !isMain)
		return false;

	// 0 = ALL (force show)
	if (effectLevel == 0)
		return true;
	// 1 = ME AND MONSTERS
	else if (effectLevel == 1)
	{
		if (isMain || (actorType != TYPE_PC))
			return true;
	}
	// 2 = ME AND PLAYERS
	else if (effectLevel == 2)
	{
		if (isMain || (actorType == TYPE_PC))
			return true;
	}
	// 3 = ONLY ME
	else if (effectLevel == 3)
	{
		if (isMain)
			return true;
	}
	// 4 = NEVER
	else if (effectLevel == 4)
		return false;

	return false;
}

EffectHelper::EffectHelper()
{
}

EffectHelper::~EffectHelper()
{
}
#endif