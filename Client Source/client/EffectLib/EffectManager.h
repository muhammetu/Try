#pragma once

#include "EffectInstance.h"

class CEffectManager : public CScreen, public CSingleton<CEffectManager>
{
public:
	enum EEffectType
	{
		EFFECT_TYPE_NONE = 0,
		EFFECT_TYPE_PARTICLE = 1,
		EFFECT_TYPE_ANIMATION_TEXTURE = 2,
		EFFECT_TYPE_MESH = 3,
		EFFECT_TYPE_SIMPLE_LIGHT = 4,

		EFFECT_TYPE_MAX_NUM = 4,
	};

	typedef std::map<DWORD, CEffectData*> TEffectDataMap;
	typedef std::map<DWORD, CEffectInstance*> TEffectInstanceMap;

public:
	CEffectManager();
	virtual ~CEffectManager();

	void Destroy();

	void UpdateSound();
	void Update();
	void Render();
#ifdef USE_MODEL_RENDER_TARGET_EFFECT
	void RenderOne(DWORD id);
#endif

	void GetInfo(std::string* pstInfo);

	bool IsAliveEffect(DWORD dwInstanceIndex);

	// Register
#ifdef ENABLE_SKILL_COLOR_SYSTEM
	BOOL RegisterEffect(const char* c_szFileName, bool isExistDelete = false, bool isNeedCache = false, const char* name = NULL);
	BOOL RegisterEffect2(const char* c_szFileName, DWORD* pdwRetCRC, bool isNeedCache = false, const char* name = NULL);
#else
	BOOL RegisterEffect(const char* c_szFileName, bool isExistDelete = false, bool isNeedCache = false);
	BOOL RegisterEffect2(const char* c_szFileName, DWORD* pdwRetCRC, bool isNeedCache = false);
#endif

	void DeleteAllInstances();

	// Usage
#ifdef ENABLE_SKILL_COLOR_SYSTEM
	#ifdef ENABLE_GRAPHIC_OPTIMIZATION
		int CreateEffect(DWORD dwID, const D3DXVECTOR3& c_rv3Position, const D3DXVECTOR3& c_rv3Rotation, DWORD* dwSkillColor = NULL, bool ignoreFrustum = false);
	#else
		int CreateEffect(DWORD dwID, const D3DXVECTOR3& c_rv3Position, const D3DXVECTOR3& c_rv3Rotation, DWORD* dwSkillColor = NULL);
	#endif
#else
	int CreateEffect(DWORD dwID, const D3DXVECTOR3& c_rv3Position, const D3DXVECTOR3& c_rv3Rotation);
#endif
	int CreateEffect(const char* c_szFileName, const D3DXVECTOR3& c_rv3Position, const D3DXVECTOR3& c_rv3Rotation);

#ifdef ENABLE_SKILL_COLOR_SYSTEM
	void CreateEffectInstance(DWORD dwInstanceIndex, DWORD dwID, DWORD* dwSkillColor = NULL, bool ignoreFrustum = false);
#else
	void CreateEffectInstance(DWORD dwInstanceIndex, DWORD dwID);
#endif
	BOOL SelectEffectInstance(DWORD dwInstanceIndex);
	bool DestroyEffectInstance(DWORD dwInstanceIndex);
	void DeactiveEffectInstance(DWORD dwInstanceIndex);

#ifdef ENABLE_GRAPHIC_OPTIMIZATION
	int CreateEffectWithScale(DWORD dwID, const D3DXVECTOR3& c_rv3Position, const D3DXVECTOR3& c_rv3Rotation, DWORD* dwSkillColor = NULL, bool ignoreFrustum = false, float fParticleScale = 1.0f);
	void CreateEffectInstanceWithScale(DWORD dwInstanceIndex, DWORD dwID, DWORD* dwSkillColor = NULL, bool ignoreFrustum = false, float fParticleScale = 1.0f, const D3DXVECTOR3* c_pv3MeshScale = NULL);
#else
	int CreateEffectWithScale(DWORD dwID, const D3DXVECTOR3& c_rv3Position, const D3DXVECTOR3& c_rv3Rotation, DWORD* dwSkillColor = NULL, float fParticleScale = 1.0f);
	void CreateEffectInstanceWithScale(DWORD dwInstanceIndex, DWORD dwID, DWORD* dwSkillColor = NULL, float fParticleScale = 1.0f, const D3DXVECTOR3* c_pv3MeshScale = NULL);
#endif
	void SetEffectTextures(DWORD dwID, vector<string> textures);
	void SetEffectInstancePosition(const D3DXVECTOR3& c_rv3Position);
	void SetEffectInstanceRotation(const D3DXVECTOR3& c_rv3Rotation);
	void SetEffectInstanceGlobalMatrix(const D3DXMATRIX& c_rmatGlobal);
#ifdef ENABLE_GRAPHIC_OPTIMIZATION
	void SetEffectInstanceIgnoreFrustum(bool f);
#endif

	void ShowEffect();
	void HideEffect();

	// Temporary function
	DWORD GetRandomEffect();
	int GetEmptyIndex();
	bool GetEffectData(DWORD dwID, CEffectData** ppEffect);
	bool GetEffectData(DWORD dwID, const CEffectData** c_ppEffect);

	void CreateUnsafeEffectInstance(DWORD dwEffectDataID, CEffectInstance** ppEffectInstance);
	bool DestroyUnsafeEffectInstance(CEffectInstance* pEffectInstance);

protected:
	void __Initialize();

	void __DestroyEffectInstanceMap();
	void __DestroyEffectCacheMap();
	void __DestroyEffectDataMap();

protected:
	bool m_isDisableSortRendering;
	TEffectDataMap					m_kEftDataMap;
	TEffectInstanceMap				m_kEftInstMap;
	TEffectInstanceMap				m_kEftCacheMap;
	CEffectInstance* m_pSelectedEffectInstance;
};
