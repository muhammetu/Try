#include "StdAfx.h"
#include "GameType.h"
#include "../effectLib/EffectManager.h"

extern float g_fGameFPS = 60.0f;

CDynamicPool<NRaceData::TCollisionData>			NRaceData::g_CollisionDataPool;
CDynamicPool<NRaceData::TAttachingEffectData>	NRaceData::g_EffectDataPool;
CDynamicPool<NRaceData::TAttachingObjectData>	NRaceData::g_ObjectDataPool;

void NRaceData::DestroySystem()
{
	g_CollisionDataPool.Destroy();
	g_EffectDataPool.Destroy();
	g_ObjectDataPool.Destroy();
}

/////////////////////////////////////////////////////////////////////////////////
// Character Attaching Collision Data
BOOL NRaceData::LoadAttackData(CTextFileLoader& rTextFileLoader, TAttackData* pData)
{
	if (!rTextFileLoader.GetTokenInteger("attacktype", &pData->iAttackType))
	{
		pData->iAttackType = ATTACK_TYPE_SPLASH;
	}
	if (!rTextFileLoader.GetTokenInteger("hittingtype", &pData->iHittingType))
		return false;
	if (!rTextFileLoader.GetTokenFloat("stiffentime", &pData->fStiffenTime))
		return false;
	if (!rTextFileLoader.GetTokenFloat("invisibletime", &pData->fInvisibleTime))
		return false;
	if (!rTextFileLoader.GetTokenFloat("externalforce", &pData->fExternalForce))
		return false;
	if (!rTextFileLoader.GetTokenInteger("hitlimitcount", &pData->iHitLimitCount))
	{
		pData->iHitLimitCount = 0;
	}

	return true;
}

bool NRaceData::THitData::Load(CTextFileLoader& rTextFileLoader)
{
	if (!rTextFileLoader.GetTokenFloat("attackingstarttime", &fAttackStartTime))
		return false;
	if (!rTextFileLoader.GetTokenFloat("attackingendtime", &fAttackEndTime))
		return false;

	if (!rTextFileLoader.GetTokenString("attackingbone", &strBoneName))
		strBoneName = "";
	if (!rTextFileLoader.GetTokenFloat("weaponlength", &fWeaponLength))
		fWeaponLength = 0.0f;

	mapHitPosition.clear();

	CTokenVector* tv;
	if (rTextFileLoader.GetTokenVector("hitposition", &tv))
	{
		CTokenVector::iterator it = tv->begin();

		while (it != tv->end())
		{
			float time;
			NRaceData::THitTimePosition hp;
			time = atof(it++->c_str());
			hp.v3LastPosition.x = atof(it++->c_str());
			hp.v3LastPosition.y = atof(it++->c_str());
			hp.v3LastPosition.z = atof(it++->c_str());
			hp.v3Position.x = atof(it++->c_str());
			hp.v3Position.y = atof(it++->c_str());
			hp.v3Position.z = atof(it++->c_str());

			mapHitPosition[time] = hp;
		}
	}

	return true;
}

BOOL NRaceData::LoadMotionAttackData(CTextFileLoader& rTextFileLoader, TMotionAttackData* pData)
{
	if (!LoadAttackData(rTextFileLoader, pData))
		return FALSE;

	if (!rTextFileLoader.GetTokenInteger("motiontype", &pData->iMotionType))
	{
		if (!rTextFileLoader.GetTokenInteger("attackingtype", &pData->iMotionType))
			return FALSE;
	}

	pData->HitDataContainer.clear();

	DWORD dwHitDataCount;
	if (!rTextFileLoader.GetTokenDoubleWord("hitdatacount", &dwHitDataCount))
	{
		pData->HitDataContainer.push_back(SHitData());
		THitData& rHitData = *(pData->HitDataContainer.rbegin());
		if (!rHitData.Load(rTextFileLoader))
			return FALSE;
	}
	else
	{
		for (DWORD i = 0; i < dwHitDataCount; ++i)
		{
			if (!rTextFileLoader.SetChildNode(i))
				return FALSE;

			pData->HitDataContainer.push_back(SHitData());
			THitData& rHitData = *(pData->HitDataContainer.rbegin());
			if (!rHitData.Load(rTextFileLoader))
				return FALSE;

			rTextFileLoader.SetParentNode();
		}
	}

	return TRUE;
}

BOOL NRaceData::LoadCollisionData(CTextFileLoader& rTextFileLoader, TCollisionData* pCollisionData)
{
	if (!rTextFileLoader.GetTokenInteger("collisiontype", &pCollisionData->iCollisionType))
		return false;

	DWORD dwSphereDataCount;
	if (!rTextFileLoader.GetTokenDoubleWord("spheredatacount", &dwSphereDataCount))
		return false;

	pCollisionData->SphereDataVector.clear();
	pCollisionData->SphereDataVector.resize(dwSphereDataCount);
	for (DWORD i = 0; i < dwSphereDataCount; ++i)
	{
		TSphereData& r = pCollisionData->SphereDataVector[i].GetAttribute();

		if (!rTextFileLoader.SetChildNode("spheredata", i))
			return false;

		if (!rTextFileLoader.GetTokenFloat("radius", &r.fRadius))
			return false;
		if (!rTextFileLoader.GetTokenPosition("position", &r.v3Position))
			return false;

		rTextFileLoader.SetParentNode();
	}

	return true;
}

BOOL NRaceData::LoadEffectData(CTextFileLoader& rTextFileLoader, TAttachingEffectData* pEffectData)
{
	if (!rTextFileLoader.GetTokenString("effectscriptname", &pEffectData->strFileName))
		return false;

	if (!rTextFileLoader.GetTokenPosition("effectposition", &pEffectData->v3Position))
	{
		pEffectData->v3Position = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	}

	if (!rTextFileLoader.GetTokenPosition("effectrotation", &pEffectData->v3Rotation))
	{
		//pEffectData->qRotation = D3DXQUATERNION(0.0f,0.0f,0.0f,1.0f);
		pEffectData->v3Rotation = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	}

	/*if (!*/
	// TODO DELETEME FIXME
	CEffectManager::Instance().RegisterEffect(pEffectData->strFileName.c_str());
	/*) return false;*/

	return true;
}

BOOL NRaceData::LoadObjectData(CTextFileLoader& rTextFileLoader, TAttachingObjectData* pObjectData)
{
	if (!rTextFileLoader.GetTokenString("objectscriptname", &pObjectData->strFileName))
		return false;

	return true;
}

BOOL NRaceData::LoadAttachingData(CTextFileLoader& rTextFileLoader, TAttachingDataVector* pAttachingDataVector)
{
	DWORD dwDataCount;
	if (!rTextFileLoader.GetTokenDoubleWord("attachingdatacount", &dwDataCount))
		return false;

	pAttachingDataVector->clear();
	pAttachingDataVector->resize(dwDataCount);

	for (DWORD i = 0; i < dwDataCount; ++i)
	{
		NRaceData::TAttachingData& rAttachingData = pAttachingDataVector->at(i);
		if (!rTextFileLoader.SetChildNode("attachingdata", i))
			return false;

		if (!rTextFileLoader.GetTokenDoubleWord("attachingdatatype", &rAttachingData.dwType))
			return false;

		if (!rTextFileLoader.GetTokenInteger("isattaching", &rAttachingData.isAttaching))
			return false;
		if (!rTextFileLoader.GetTokenDoubleWord("attachingmodelindex", &rAttachingData.dwAttachingModelIndex))
			return false;
		if (!rTextFileLoader.GetTokenString("attachingbonename", &rAttachingData.strAttachingBoneName))
			return false;

		switch (rAttachingData.dwType)
		{
		case NRaceData::ATTACHING_DATA_TYPE_COLLISION_DATA:
			rAttachingData.pCollisionData = g_CollisionDataPool.Alloc();
			if (!NRaceData::LoadCollisionData(rTextFileLoader, rAttachingData.pCollisionData))
				return false;
			break;
		case NRaceData::ATTACHING_DATA_TYPE_EFFECT:
			rAttachingData.pEffectData = g_EffectDataPool.Alloc();
			if (!NRaceData::LoadEffectData(rTextFileLoader, rAttachingData.pEffectData))
				return false;
			break;
		case NRaceData::ATTACHING_DATA_TYPE_OBJECT:
			rAttachingData.pObjectData = g_ObjectDataPool.Alloc();
			if (!NRaceData::LoadObjectData(rTextFileLoader, rAttachingData.pObjectData))
				return false;
			break;
		}

		rTextFileLoader.SetParentNode();
	}
	return TRUE;
}