#pragma once

#include "GrpBase.h"

class CGraphicTexture : public CGraphicBase
{
public:
	virtual bool IsEmpty() const;

	int GetWidth() const;
	int GetHeight() const;

	void SetTextureStage(int stage) const;
	LPDIRECT3DTEXTURE8 GetD3DTexture() const;

	void DestroyDeviceObjects();

protected:
	CGraphicTexture();
	virtual	~CGraphicTexture();

	virtual void Destroy();
	virtual void Initialize();

protected:
	bool m_bEmpty;

	int m_width;
	int m_height;

	LPDIRECT3DTEXTURE8 m_lpd3dTexture;
};
