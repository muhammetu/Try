#pragma once

#include "GrpImage.h"
#include "GrpIndexBuffer.h"
#include "Pool.h"

class CGraphicImageInstance
{
public:
	static DWORD Type();
	BOOL IsType(DWORD dwType);

public:
	CGraphicImageInstance();
	virtual ~CGraphicImageInstance();

	void Destroy();

	void Render();
	void SetDiffuseColor(float fr, float fg, float fb, float fa);
	void SetPosition(float fx, float fy);
	void UpdatePosition(float fx, float fy);
#ifdef ENABLE_MODEL_RENDER_TARGET
	void SetScale(float fx, float fy);
#endif
	void SetScale(D3DXVECTOR2 v2Scale);
	const D3DXVECTOR2 & GetScale() const;
	void SetScalePercent(BYTE byPercent);
	void SetScalePivotCenter(bool bScalePivotCenter);
	void SetImagePointer(CGraphicImage* pImage);
	void ReloadImagePointer(CGraphicImage* pImage);
	bool IsEmpty() const;
	D3DXVECTOR2 GetPosition() const { return m_v2Position; };
	void SetClipRect(float fLeft, float fTop, float fRight, float fBottom);

	int GetWidth();
	int GetHeight();

	CGraphicTexture* GetTexturePointer();
	const CGraphicTexture& GetTextureReference() const;
	CGraphicImage* GetGraphicImagePointer();

	bool operator == (const CGraphicImageInstance& rhs) const;

protected:
	void Initialize();

	virtual void OnRender();
	virtual void OnSetImagePointer();

	virtual BOOL OnIsType(DWORD dwType);

protected:
	D3DXCOLOR m_DiffuseColor;
	D3DXVECTOR2 m_v2Position;
	bool m_bScalePivotCenter;
	CGraphicImage::TRef m_roImage;
	RECT m_clipRect;
	bool m_bClipEnable;
#ifdef ENABLE_MODEL_RENDER_TARGET
	D3DXVECTOR2 m_v2Scale;
#endif

public:
	static void CreateSystem(UINT uCapacity);
	static void DestroySystem();

	static CGraphicImageInstance* New();
	static void Delete(CGraphicImageInstance* pkImgInst);

	static CDynamicPool<CGraphicImageInstance>		ms_kPool;
};
