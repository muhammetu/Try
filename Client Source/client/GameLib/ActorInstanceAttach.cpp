#include "StdAfx.h"
#include "../EffectLib/EffectManager.h"
#ifdef ENABLE_GRAPHIC_OPTIMIZATION
#include "../EffectLib/EffectHelper.h"
#endif

#include "ActorInstance.h"
#include "ItemData.h"
#include "ItemManager.h"
#include "RaceData.h"
#include "WeaponTrace.h"
#include "GameLibDefines.h"

DWORD CActorInstance::AttachSmokeEffect(DWORD eSmoke)
{
	if (!m_pkCurRaceData)
		return 0;

	DWORD dwSmokeEffectID = m_pkCurRaceData->GetSmokeEffectID(eSmoke);

	return AttachEffectByID(0, m_pkCurRaceData->GetSmokeBone().c_str(), dwSmokeEffectID);
}

bool CActorInstance::__IsLeftHandWeapon(DWORD type)
{
	if (CItemData::WEAPON_DAGGER == type || (CItemData::WEAPON_FAN == type && __IsMountingHorse()))
		return true;
	else if (CItemData::WEAPON_BOW == type)
		return true;
#ifdef ENABLE_WOLFMAN_CHARACTER
	else if (CItemData::WEAPON_CLAW == type)
		return true;
#endif
	else
		return false;
}

bool CActorInstance::__IsRightHandWeapon(DWORD type)
{
	if (CItemData::WEAPON_DAGGER == type || (CItemData::WEAPON_FAN == type && __IsMountingHorse()))
		return true;
	else if (CItemData::WEAPON_BOW == type)
		return false;
#ifdef ENABLE_WOLFMAN_CHARACTER
	else if (CItemData::WEAPON_CLAW == type)
		return true;
#endif
	else
		return true;
}

bool CActorInstance::__IsWeaponTrace(DWORD weaponType)
{
	switch (weaponType)
	{
	case CItemData::WEAPON_BELL:
	case CItemData::WEAPON_FAN:
	case CItemData::WEAPON_BOW:
		return false;
	default:
		return true;
	}
}

void CActorInstance::AttachWeapon(DWORD dwItemIndex, DWORD dwParentPartIndex, DWORD dwPartIndex)
{
	if (dwPartIndex >= CRaceData::PART_MAX_NUM)
		return;

	m_adwPartItemID[dwPartIndex] = dwItemIndex;

	CItemData* pItemData;
	if (!CItemManager::Instance().GetItemDataPointer(dwItemIndex, &pItemData))
	{
		RegisterModelThing(dwPartIndex, NULL);
		SetModelInstance(dwPartIndex, dwPartIndex, 0);

		RegisterModelThing(CRaceData::PART_WEAPON_LEFT, NULL);
		SetModelInstance(CRaceData::PART_WEAPON_LEFT, CRaceData::PART_WEAPON_LEFT, 0);

		RefreshActorInstance();
		return;
	}

	__DestroyWeaponTrace();
	if (__IsRightHandWeapon(pItemData->GetWeaponType()))
		AttachWeapon(dwParentPartIndex, CRaceData::PART_WEAPON, pItemData);
	if (__IsLeftHandWeapon(pItemData->GetWeaponType()))
		AttachWeapon(dwParentPartIndex, CRaceData::PART_WEAPON_LEFT, pItemData);
}

BOOL CActorInstance::GetAttachingBoneName(DWORD dwPartIndex, const char** c_pszBoneName)
{
	return m_pkCurRaceData->GetAttachingBoneName(dwPartIndex, c_pszBoneName);
}

void CActorInstance::AttachWeapon(DWORD dwParentPartIndex, DWORD dwPartIndex, CItemData* pItemData)
{
	//	assert(m_pkCurRaceData);
	if (!pItemData)
		return;

#if defined(ENABLE_WOLFMAN_CHARACTER) && defined(AUTODETECT_LYCAN_RODNPICK_BONE)
	const char* szBoneName;
	if ((GetRace() == 8) && (pItemData->GetType() == CItemData::ITEM_TYPE_ROD || pItemData->GetType() == CItemData::ITEM_TYPE_PICK))
	{
		szBoneName = "equip_right";
	}
	else if (!GetAttachingBoneName(dwPartIndex, &szBoneName))
		return;
#else
	const char* szBoneName;
	if (!GetAttachingBoneName(dwPartIndex, &szBoneName))
		return;
#endif

	if (CRaceData::PART_WEAPON_LEFT == dwPartIndex)
	{
		RegisterModelThing(dwPartIndex, pItemData->GetSubModelThing());
	}
	else
	{
		RegisterModelThing(dwPartIndex, pItemData->GetModelThing());
	}

	for (DWORD i = 0; i < pItemData->GetLODModelThingCount(); ++i)
	{
		CGraphicThing* pThing;

		if (!pItemData->GetLODModelThingPointer(i, &pThing))
			continue;

		RegisterLODThing(dwPartIndex, pThing);
	}

	SetModelInstance(dwPartIndex, dwPartIndex, 0);
	AttachModelInstance(dwParentPartIndex, szBoneName, dwPartIndex);

	SMaterialData kMaterialData;
	kMaterialData.pImage = NULL;
	kMaterialData.isSpecularEnable = TRUE;
	kMaterialData.fSpecularPower = pItemData->GetSpecularPowerf();
	kMaterialData.bSphereMapIndex = 1;
	SetMaterialData(dwPartIndex, NULL, kMaterialData);

	// Weapon Trace
	if (__IsWeaponTrace(pItemData->GetWeaponType()))
	{
		CWeaponTrace* pWeaponTrace = CWeaponTrace::New();
		pWeaponTrace->SetWeaponInstance(this, dwPartIndex, szBoneName);
		m_WeaponTraceVector.push_back(pWeaponTrace);
	}

	if (CGrannyLODController* pLODController = m_LODControllerVector[dwPartIndex])
	{
		if (CGrannyModelInstance* pWeaponModelInstance = pLODController->GetModelInstance())
		{
			CGraphicThing* pItemGraphicThing = pItemData->GetModelThing();
			if (std::shared_ptr<CGrannyMotion> pItemMotion = pItemGraphicThing->GetMotionPointer(0))
			{
				pWeaponModelInstance->SetMotionPointer(pItemMotion);
			}
		}
	}
}

void  CActorInstance::DettachEffect(DWORD dwEID)
{
	std::list<TAttachingEffect>::iterator i = m_AttachingEffectList.begin();

	while (i != m_AttachingEffectList.end())
	{
		TAttachingEffect& rkAttEft = (*i);

		if (rkAttEft.dwEffectIndex == dwEID)
		{
			i = m_AttachingEffectList.erase(i);
			CEffectManager::Instance().DestroyEffectInstance(dwEID);
		}
		else
		{
			++i;
		}
	}
}

#ifdef ENABLE_GRAPHIC_OPTIMIZATION
DWORD CActorInstance::AttachEffectByName(DWORD dwParentPartIndex, const char * c_pszBoneName, const char * c_pszEffectName, bool ignoreFrustum)
#else
DWORD CActorInstance::AttachEffectByName(DWORD dwParentPartIndex, const char * c_pszBoneName, const char * c_pszEffectName)
#endif
{
	std::string str;
	DWORD dwCRC;
	StringPath(c_pszEffectName, str);
	dwCRC = GetCaseCRC32(str.c_str(), str.length());

#ifdef ENABLE_GRAPHIC_OPTIMIZATION
	return AttachEffectByID(dwParentPartIndex, c_pszBoneName, dwCRC, NULL, NULL, ignoreFrustum);
#else
	return AttachEffectByID(dwParentPartIndex, c_pszBoneName, dwCRC);
#endif
}

// ENABLE_GRAPHIC_OPTIMIZATION
// Uyarlanmadi dikkat !
#ifdef ENABLE_SKILL_COLOR_SYSTEM
DWORD CActorInstance::AttachEffectByID(DWORD dwParentPartIndex, const char* c_pszBoneName, DWORD dwEffectID, const D3DXVECTOR3* c_pv3Position, DWORD* dwSkillColor, bool ignoreFrustum, float fParticleScale, const D3DXVECTOR3* c_pv3MeshScale)
#else
DWORD CActorInstance::AttachEffectByID(DWORD dwParentPartIndex, const char* c_pszBoneName, DWORD dwEffectID, const D3DXVECTOR3* c_pv3Position, bool ignoreFrustum)
#endif
{
	TAttachingEffect ae;
	ae.iLifeType = EFFECT_LIFE_INFINITE;
	ae.dwEndTime = 0;
	ae.dwModelIndex = dwParentPartIndex;
	ae.dwEffectIndex = CEffectManager::Instance().GetEmptyIndex();
	ae.isAttaching = TRUE;
	if (c_pv3Position)
	{
		D3DXMatrixTranslation(&ae.matTranslation, c_pv3Position->x, c_pv3Position->y, c_pv3Position->z);
	}
	else
	{
		D3DXMatrixIdentity(&ae.matTranslation);
	}
	CEffectManager& rkEftMgr = CEffectManager::Instance();
	if (c_pv3MeshScale)
		rkEftMgr.CreateEffectInstanceWithScale(ae.dwEffectIndex, dwEffectID, dwSkillColor, false, fParticleScale, c_pv3MeshScale);
	else
		rkEftMgr.CreateEffectInstanceWithScale(ae.dwEffectIndex, dwEffectID, dwSkillColor, false, fParticleScale);

#ifdef ENABLE_GRAPHIC_OPTIMIZATION
	rkEftMgr.SelectEffectInstance(ae.dwEffectIndex);
	rkEftMgr.SetEffectInstanceIgnoreFrustum(ignoreFrustum);
#endif

	if (c_pszBoneName)
	{
		int iBoneIndex;

		if (!FindBoneIndex(dwParentPartIndex, c_pszBoneName, &iBoneIndex))
		{
			ae.iBoneIndex = -1;
			//Tracef("Cannot get Bone Index\n");
			//assert(false && "Cannot get Bone Index");
		}
		else
		{
			ae.iBoneIndex = iBoneIndex;
		}
	}
	else
	{
		ae.iBoneIndex = -1;
	}

	m_AttachingEffectList.push_back(ae);

	return ae.dwEffectIndex;
}

void CActorInstance::RefreshActorInstance()
{
	if (!m_pkCurRaceData)
	{
		TraceError("void CActorInstance::RefreshActorInstance() - m_pkCurRaceData=NULL");
		return;
	}

	// This is Temporary place before making the weapon detection system
	// Setup Collison Detection Data
	m_BodyPointInstanceList.clear();
	//m_AttackingPointInstanceList.clear();
	m_DefendingPointInstanceList.clear();

	// Base
	for (DWORD i = 0; i < m_pkCurRaceData->GetAttachingDataCount(); ++i)
	{
		const NRaceData::TAttachingData* c_pAttachingData;

		if (!m_pkCurRaceData->GetAttachingDataPointer(i, &c_pAttachingData))
			continue;

		switch (c_pAttachingData->dwType)
		{
		case NRaceData::ATTACHING_DATA_TYPE_COLLISION_DATA:
		{
			const NRaceData::TCollisionData* c_pCollisionData = c_pAttachingData->pCollisionData;

			TCollisionPointInstance PointInstance;
			if (NRaceData::COLLISION_TYPE_ATTACKING == c_pCollisionData->iCollisionType)
				continue;

			if (!CreateCollisionInstancePiece(CRaceData::PART_MAIN, c_pAttachingData, &PointInstance))
				continue;

			switch (c_pCollisionData->iCollisionType)
			{
			case NRaceData::COLLISION_TYPE_ATTACKING:
				//m_AttackingPointInstanceList.push_back(PointInstance);
				break;
			case NRaceData::COLLISION_TYPE_DEFENDING:
				m_DefendingPointInstanceList.push_back(PointInstance);
				break;
			case NRaceData::COLLISION_TYPE_BODY:
				m_BodyPointInstanceList.push_back(PointInstance);
				break;
			}
		}
		break;

		case NRaceData::ATTACHING_DATA_TYPE_EFFECT:
			//				if (!m_bEffectInitialized)
			//				{
			//					DWORD dwCRC;
			//					StringPath(c_pAttachingData->pEffectData->strFileName);
			//					dwCRC = GetCaseCRC32(c_pAttachingData->pEffectData->strFileName.c_str(),c_pAttachingData->pEffectData->strFileName.length());
			//
			//					TAttachingEffect ae;
			//					ae.iLifeType = EFFECT_LIFE_INFINITE;
			//					ae.dwEndTime = 0;
			//					ae.dwModelIndex = 0;
			//					ae.dwEffectIndex = CEffectManager::Instance().GetEmptyIndex();
			//					ae.isAttaching = TRUE;
			//					CEffectManager::Instance().CreateEffectInstance(ae.dwEffectIndex, dwCRC);
			//
			//					if (c_pAttachingData->isAttaching)
			//					{
			//						int iBoneIndex;
			//						if (!FindBoneIndex(0,c_pAttachingData->strAttachingBoneName.c_str(), &iBoneIndex))
			//						{
			//							Tracef("Cannot get Bone Index\n");
			//							assert(false/*Cannot get Bone Index*/);
			//						}
			//
			//						ae.iBoneIndex = iBoneIndex;
			//					}
			//					else
			//					{
			//						ae.iBoneIndex = -1;
			//					}
			//
			//					m_AttachingEffectList.push_back(ae);
			//				}

			if (c_pAttachingData->isAttaching)
			{
				AttachEffectByName(0, c_pAttachingData->strAttachingBoneName.c_str(), c_pAttachingData->pEffectData->strFileName.c_str());
			}
			else
			{
				AttachEffectByName(0, 0, c_pAttachingData->pEffectData->strFileName.c_str());
			}
			break;

		case NRaceData::ATTACHING_DATA_TYPE_OBJECT:
			break;

		default:
			assert(false/*NOT_IMPLEMENTED*/);
			break;
		}
	}

	for (DWORD j = 0; j < CRaceData::PART_MAX_NUM; ++j)
	{
		if (0 == m_adwPartItemID[j])
			continue;

		CItemData* pItemData;
		if (!CItemManager::Instance().GetItemDataPointer(m_adwPartItemID[j], &pItemData))
			return;

		for (DWORD k = 0; k < pItemData->GetAttachingDataCount(); ++k)
		{
			const NRaceData::TAttachingData* c_pAttachingData;

			if (!pItemData->GetAttachingDataPointer(k, &c_pAttachingData))
				continue;

			switch (c_pAttachingData->dwType)
			{
			case NRaceData::ATTACHING_DATA_TYPE_COLLISION_DATA:
			{
				const NRaceData::TCollisionData* c_pCollisionData = c_pAttachingData->pCollisionData;

				TCollisionPointInstance PointInstance;
				if (NRaceData::COLLISION_TYPE_ATTACKING == c_pCollisionData->iCollisionType)
					continue;
				if (!CreateCollisionInstancePiece(j, c_pAttachingData, &PointInstance))
					continue;

				switch (c_pCollisionData->iCollisionType)
				{
				case NRaceData::COLLISION_TYPE_ATTACKING:
					//m_AttackingPointInstanceList.push_back(PointInstance);
					break;
				case NRaceData::COLLISION_TYPE_DEFENDING:
					m_DefendingPointInstanceList.push_back(PointInstance);
					break;
				case NRaceData::COLLISION_TYPE_BODY:
					m_BodyPointInstanceList.push_back(PointInstance);
					break;
				}
			}
			break;

			case NRaceData::ATTACHING_DATA_TYPE_EFFECT:
				if (!m_bEffectInitialized)
				{
					DWORD dwCRC;
					StringPath(c_pAttachingData->pEffectData->strFileName);
					dwCRC = GetCaseCRC32(c_pAttachingData->pEffectData->strFileName.c_str(), c_pAttachingData->pEffectData->strFileName.length());

					TAttachingEffect ae;
					ae.iLifeType = EFFECT_LIFE_INFINITE;
					ae.dwEndTime = 0;
					ae.dwModelIndex = j;
					ae.dwEffectIndex = CEffectManager::Instance().GetEmptyIndex();
					ae.isAttaching = TRUE;
					CEffectManager::Instance().CreateEffectInstance(ae.dwEffectIndex, dwCRC);

					int iBoneIndex;
					if (!FindBoneIndex(j, c_pAttachingData->strAttachingBoneName.c_str(), &iBoneIndex))
					{
						Tracef("Cannot get Bone Index\n");
						assert(false/*Cannot get Bone Index*/);
					}
					Tracef("Creating %p %d %d\n", this, j, k);

					ae.iBoneIndex = iBoneIndex;

					m_AttachingEffectList.push_back(ae);
				}
				break;

			case NRaceData::ATTACHING_DATA_TYPE_OBJECT:
				break;

			default:
				assert(false/*NOT_IMPLEMENTED*/);
				break;
			}
		}
	}

	m_bEffectInitialized = true;
}

void CActorInstance::SetWeaponTraceTexture(const char* szTextureName)
{
	std::vector<CWeaponTrace*>::iterator it;
	for (it = m_WeaponTraceVector.begin(); it != m_WeaponTraceVector.end(); ++it)
	{
		(*it)->SetTexture(szTextureName);
	}
}

void CActorInstance::UseTextureWeaponTrace()
{
	for_each(
		m_WeaponTraceVector.begin(),
		m_WeaponTraceVector.end(),
		std::void_mem_fun(&CWeaponTrace::UseTexture)
	);
}

void CActorInstance::UseAlphaWeaponTrace()
{
	for_each(
		m_WeaponTraceVector.begin(),
		m_WeaponTraceVector.end(),
		std::void_mem_fun(&CWeaponTrace::UseAlpha)
	);
}

void CActorInstance::UpdateAttachingInstances()
{
	CEffectManager& rkEftMgr = CEffectManager::Instance();
#ifdef ENABLE_GRAPHIC_OPTIMIZATION
	EffectHelper& rkEftHelper=EffectHelper::Instance();
#endif // ENABLE_GRAPHIC_OPTIMIZATION

	std::list<TAttachingEffect>::iterator it;
	DWORD dwCurrentTime = CTimer::Instance().GetCurrentMillisecond();
	for (it = m_AttachingEffectList.begin(); it != m_AttachingEffectList.end();)
	{
		if (EFFECT_LIFE_WITH_MOTION == it->iLifeType)
		{
			++it;
			continue;
		}

		if ((EFFECT_LIFE_NORMAL == it->iLifeType && it->dwEndTime < dwCurrentTime) ||
			!rkEftMgr.IsAliveEffect(it->dwEffectIndex))
		{
			rkEftMgr.DestroyEffectInstance(it->dwEffectIndex);
			it = m_AttachingEffectList.erase(it);
		}
		else
		{
			if (it->isAttaching)
			{
				rkEftMgr.SelectEffectInstance(it->dwEffectIndex);
#ifdef ENABLE_GRAPHIC_OPTIMIZATION
				if (!IsRenderTarget())
				{
					if (rkEftHelper.GetEffectRenderStatus(m_isMain, m_eActorType) == false)
						rkEftMgr.HideEffect();
					else
						rkEftMgr.ShowEffect();
				}
				else
				{
					rkEftMgr.ShowEffect();
					rkEftMgr.SetEffectInstanceIgnoreFrustum(true);
				}
#endif // ENABLE_GRAPHIC_OPTIMIZATION
				if (it->iBoneIndex == -1)
				{
					D3DXMATRIX matTransform;
					matTransform = it->matTranslation;
					matTransform *= m_worldMatrix;
					rkEftMgr.SetEffectInstanceGlobalMatrix(matTransform);
				}
				else
				{
					D3DXMATRIX* pBoneMat;
					if (GetBoneMatrix(it->dwModelIndex, it->iBoneIndex, &pBoneMat))
					{
						D3DXMATRIX matTransform;
						matTransform = *pBoneMat;
						matTransform *= it->matTranslation;
						matTransform *= m_worldMatrix;
						rkEftMgr.SetEffectInstanceGlobalMatrix(matTransform);
					}
				}
			}

			++it;
		}
	}
}

void CActorInstance::ShowAllAttachingEffect()
{
	std::list<TAttachingEffect>::iterator it;
	for (it = m_AttachingEffectList.begin(); it != m_AttachingEffectList.end(); ++it)
	{
		CEffectManager::Instance().SelectEffectInstance(it->dwEffectIndex);
		CEffectManager::Instance().ShowEffect();
	}
}

void CActorInstance::HideAllAttachingEffect()
{
	std::list<TAttachingEffect>::iterator it;
	for (it = m_AttachingEffectList.begin(); it != m_AttachingEffectList.end(); ++it)
	{
		CEffectManager::Instance().SelectEffectInstance(it->dwEffectIndex);
		CEffectManager::Instance().HideEffect();
	}
}

void CActorInstance::__ClearAttachingEffect()
{
	m_bEffectInitialized = false;

	std::list<TAttachingEffect>::iterator it;
	for (it = m_AttachingEffectList.begin(); it != m_AttachingEffectList.end(); ++it)
	{
		CEffectManager::Instance().DestroyEffectInstance(it->dwEffectIndex);
	}
	m_AttachingEffectList.clear();
}

#ifdef ENABLE_ACCE_SYSTEM
void CActorInstance::AttachAcce(CItemData* pItemData, float fSpecular)
{
	if (!pItemData)
	{
		RegisterModelThing(CRaceData::PART_ACCE, NULL);
		SetModelInstance(CRaceData::PART_ACCE, CRaceData::PART_ACCE, 0);
		RefreshActorInstance();
		return;
	}

	RegisterModelThing(CRaceData::PART_ACCE, pItemData->GetModelThing());
	SetModelInstance(CRaceData::PART_ACCE, CRaceData::PART_ACCE, 0);
	AttachModelInstance(CRaceData::PART_MAIN, "Bip01 Spine2", CRaceData::PART_ACCE);

	if (fSpecular > 0.0f)
	{
		SMaterialData kMaterialData;
		kMaterialData.pImage = NULL;
		kMaterialData.isSpecularEnable = TRUE;
		kMaterialData.fSpecularPower = fSpecular;
		kMaterialData.bSphereMapIndex = 1;
		SetMaterialData(CRaceData::PART_ACCE, NULL, kMaterialData);
	}

	if (CGrannyLODController* pLODController = m_LODControllerVector[CRaceData::PART_ACCE])
	{
		if (CGrannyModelInstance* pAcceModelInstance = pLODController->GetModelInstance())
		{
			CGraphicThing* pItemGraphicThing = pItemData->GetModelThing();
			if (std::shared_ptr<CGrannyMotion> pItemMotion = pItemGraphicThing->GetMotionPointer(0))
			{
				pAcceModelInstance->SetMotionPointer(pItemMotion);
			}
		}
	}
}
#endif
#ifdef ENABLE_QUIVER_SYSTEM
void CActorInstance::SetQuiverEquipped(bool bEquipped)
{
	m_bIsQuiverEquipped = bEquipped;
}
void CActorInstance::SetQuiverEffectID(DWORD dwEffectID)
{
	m_dwQuiverEffectID = dwEffectID;
}
#endif

#ifdef ENABLE_AURA_COSTUME_SYSTEM
void CActorInstance::AttachAura(CItemData* pItemData)
{
	if (!pItemData)
	{
		RegisterModelThing(CRaceData::PART_AURA, NULL);
		SetModelInstance(CRaceData::PART_AURA, CRaceData::PART_AURA, 0);
		RefreshActorInstance();
		return;
	}

	RegisterModelThing(CRaceData::PART_AURA, pItemData->GetModelThing());
	SetModelInstance(CRaceData::PART_AURA, CRaceData::PART_AURA, 0);
	AttachModelInstance(CRaceData::PART_MAIN, "Bip01 Spine2", CRaceData::PART_AURA);
}
#endif
