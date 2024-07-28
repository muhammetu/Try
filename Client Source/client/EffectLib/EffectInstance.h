#pragma once

#include "../eterlib/GrpObjectInstance.h"
#include "../eterlib/Pool.h"
#include "../mileslib/Type.h"

#include "EffectElementBaseInstance.h"
#include "EffectData.h"
#include "EffectMeshInstance.h"
#include "ParticleSystemInstance.h"
#include "SimpleLightInstance.h"
#include "../UserInterface/Locale_Inc.h"

class CEffectInstance : public CGraphicObjectInstance
{
public:
	typedef std::vector<CEffectElementBaseInstance*> TEffectElementInstanceVector;

	enum EEffectObjectID
	{
		ID = EFFECT_OBJECT
	};
	int GetType() const
	{
		return CEffectInstance::ID;
	}

	bool GetBoundingSphere(D3DXVECTOR3& v3Center, float& fRadius);

	static void DestroySystem();

	static CEffectInstance* New();
	static void Delete(CEffectInstance* pkEftInst);

	static void ResetRenderingEffectCount();
	static int GetRenderingEffectCount();

public:
	CEffectInstance();
	virtual ~CEffectInstance();

	bool LessRenderOrder(CEffectInstance* pkEftInst);

#ifdef ENABLE_SKILL_COLOR_SYSTEM
	void SetEffectDataPointer(CEffectData* pEffectData, DWORD* dwSkillColor = NULL, DWORD EffectID = 0);
#else
	void SetEffectDataPointer(CEffectData* pEffectData);
#endif

	void Clear();
	BOOL isAlive();
	void SetActive();
	void SetDeactive();
	void SetGlobalMatrix(const D3DXMATRIX& c_rmatGlobal);
	D3DXMATRIX GetGlobalMatrix();
	void UpdateSound();
	void OnUpdate();
	void OnRender();
	void OnBlendRender() {} // Not used
	void OnRenderToShadowMap() {} // Not used
	void OnRenderShadow() {} // Not used
	void OnRenderPCBlocker() {} // Not used

protected:
	void					__Initialize();

	void					__SetParticleData(CParticleSystemData* pData);
	void					__SetMeshData(CEffectMeshScript* pMesh);
	void					__SetLightData(CLightData* pData);

	virtual void			OnUpdateCollisionData(const CStaticCollisionDataVector* pscdVector) {} // Not used
	virtual void			OnUpdateHeighInstance(CAttributeInstance* pAttributeInstance) {}
	virtual bool			OnGetObjectHeight(float fX, float fY, float* pfHeight) { return false; }

protected:
	BOOL					m_isAlive;
	DWORD					m_dwFrame;
	D3DXMATRIX				m_matGlobal;

	CEffectData* m_pkEftData;

	std::vector<CParticleSystemInstance*>	m_ParticleInstanceVector;
	std::vector<CEffectMeshInstance*>		m_MeshInstanceVector;
	std::vector<CLightInstance*>			m_LightInstanceVector;

	NSound::TSoundInstanceVector* m_pSoundInstanceVector;

	float m_fBoundingSphereRadius;
	D3DXVECTOR3 m_v3BoundingSpherePosition;

	float m_fLastTime;

public:
	static CDynamicPool<CEffectInstance>	ms_kPool;
	static int ms_iRenderingEffectCount;

#ifdef USE_MODEL_RENDER_TARGET_EFFECT
protected:
	bool m_ignoreFrustum;

	struct FEffectFrustum
	{
		bool ignoreFrustum;
		FEffectFrustum(bool igno)
			: ignoreFrustum(igno)
		{
		}
		void operator () (CEffectElementBaseInstance * pInstance)
		{
			pInstance->SetIgnoreFrustum(ignoreFrustum);
		}
	};

public:
	void SetIgnoreFrustum(bool frustum)
	{
		m_ignoreFrustum = frustum;
		FEffectFrustum f2(m_ignoreFrustum);
		std::for_each(m_ParticleInstanceVector.begin(), m_ParticleInstanceVector.end(), f2);
		std::for_each(m_MeshInstanceVector.begin(), m_MeshInstanceVector.end(), f2);
		std::for_each(m_LightInstanceVector.begin(), m_LightInstanceVector.end(), f2);
	}
#endif
protected:
	float m_fParticleScale;
	D3DXVECTOR3 m_v3MeshScale;

public:
	void SetParticleScale(float fParticleScale) { m_fParticleScale = fParticleScale; }
	float GetParticleScale() { return m_fParticleScale; }
	void SetMeshScale(D3DXVECTOR3 rv3MeshScale) { m_v3MeshScale = rv3MeshScale; }
	D3DXVECTOR3 GetMeshScale() { return m_v3MeshScale; }

};
