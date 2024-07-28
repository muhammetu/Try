#include "StdAfx.h"
#include "GrpImageInstance.h"
#include "StateManager.h"

#include "../eterBase/CRC32.h"
//STATEMANAGER.SaveRenderState(D3DRS_SRCBLEND, D3DBLEND_INVDESTCOLOR);
//STATEMANAGER.SaveRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
//STATEMANAGER.RestoreRenderState(D3DRS_SRCBLEND);
//STATEMANAGER.RestoreRenderState(D3DRS_DESTBLEND);

CDynamicPool<CGraphicImageInstance>		CGraphicImageInstance::ms_kPool;

void CGraphicImageInstance::CreateSystem(UINT uCapacity)
{
	ms_kPool.Create(uCapacity);
}

void CGraphicImageInstance::DestroySystem()
{
	ms_kPool.Destroy();
}

CGraphicImageInstance* CGraphicImageInstance::New()
{
	return ms_kPool.Alloc();
}

void CGraphicImageInstance::Delete(CGraphicImageInstance* pkImgInst)
{
	pkImgInst->Destroy();
	ms_kPool.Free(pkImgInst);
}

void CGraphicImageInstance::Render()
{
	if (IsEmpty())
		return;

	assert(!IsEmpty());

	OnRender();
}

void CGraphicImageInstance::SetClipRect(float fLeft, float fTop, float fRight, float fBottom)
{
	if (IsEmpty())
		return;

	m_clipRect.left = fLeft;
	m_clipRect.top = fTop;
	m_clipRect.right = fRight;
	m_clipRect.bottom = fBottom;

	m_bClipEnable = true;
}

void CGraphicImageInstance::OnRender()
{
	CGraphicImage* pImage = m_roImage.GetPointer();
	CGraphicTexture* pTexture = pImage->GetTexturePointer();
#ifdef ENABLE_MODEL_RENDER_TARGET
	float fimgWidth = m_roImage->GetWidth() * m_v2Scale.x;
	float fimgHeight = m_roImage->GetHeight() * m_v2Scale.y;
#else
	float fimgWidth = pImage->GetWidth();
	float fimgHeight = pImage->GetHeight();
#endif
	const RECT& c_rRect = pImage->GetRectReference();
	float texReverseWidth = 1.0f / float(pTexture->GetWidth());
	float texReverseHeight = 1.0f / float(pTexture->GetHeight());
	float textureLeft = c_rRect.left * texReverseWidth;
	float textureTop = c_rRect.top * texReverseHeight;
	float textureRight = c_rRect.right * texReverseWidth;
	float textureBottom = c_rRect.bottom * texReverseHeight;

	RECT myRect = {
		m_v2Position.x,
		m_v2Position.y,
		m_v2Position.x + fimgWidth,
		m_v2Position.y + fimgHeight
	};

	if (m_bClipEnable)
	{
		if (myRect.left > m_clipRect.right)
			return;
		if (myRect.right < m_clipRect.left)
			return;

		if (myRect.top > m_clipRect.bottom)
			return;
		if (myRect.bottom < m_clipRect.top)
			return;

		if (m_clipRect.left > myRect.left)
		{
			int idx = m_clipRect.left - myRect.left;
			myRect.left = m_clipRect.left;
			textureLeft += float(idx) / float(fimgWidth);
		}
		if (myRect.right > m_clipRect.right)
		{
			int idx = myRect.right - m_clipRect.right;
			myRect.right =  m_clipRect.right;
			textureRight -= float(idx) / float(fimgWidth);
		}

		if (m_clipRect.top > myRect.top)
		{
			int idy = m_clipRect.top - myRect.top;
			myRect.top = m_clipRect.top;
			textureTop += float(idy) / float(fimgHeight);
		}
		if (myRect.bottom > m_clipRect.bottom)
		{
			int idy = myRect.bottom - m_clipRect.bottom;
			myRect.bottom = m_clipRect.bottom;
			textureBottom -= float(idy) / float(fimgHeight);
		}
	}

	TPDTVertex vertices[4];
	vertices[0].position.x = myRect.left - 0.5f;
	vertices[0].position.y = myRect.top - 0.5f;
	vertices[0].position.z = 0.0f;
	vertices[0].texCoord = TTextureCoordinate(textureLeft, textureTop);
	vertices[0].diffuse = m_DiffuseColor;

	vertices[1].position.x = myRect.right - 0.5f;
	vertices[1].position.y = myRect.top - 0.5f;
	vertices[1].position.z = 0.0f;
	vertices[1].texCoord = TTextureCoordinate(textureRight, textureTop);
	vertices[1].diffuse = m_DiffuseColor;

	vertices[2].position.x = myRect.left - 0.5f;
	vertices[2].position.y = myRect.bottom - 0.5f;
	vertices[2].position.z = 0.0f;
	vertices[2].texCoord = TTextureCoordinate(textureLeft, textureBottom);
	vertices[2].diffuse = m_DiffuseColor;

	vertices[3].position.x = myRect.right - 0.5f;
	vertices[3].position.y = myRect.bottom - 0.5f;
	vertices[3].position.z = 0.0f;
	vertices[3].texCoord = TTextureCoordinate(textureRight, textureBottom);
	vertices[3].diffuse = m_DiffuseColor;

	if (m_bScalePivotCenter)
	{
		vertices[0].texCoord = TTextureCoordinate(textureRight, textureTop);
		vertices[1].texCoord = TTextureCoordinate(textureLeft, textureTop);
		vertices[2].texCoord = TTextureCoordinate(textureRight, textureBottom);
		vertices[3].texCoord = TTextureCoordinate(textureLeft, textureBottom);
	}

	if (CGraphicBase::SetPDTStream(vertices, 4))
	{
		CGraphicBase::SetDefaultIndexBuffer(CGraphicBase::DEFAULT_IB_FILL_RECT);

		STATEMANAGER.SetTexture(0, pTexture->GetD3DTexture());
		STATEMANAGER.SetTexture(1, NULL);
		STATEMANAGER.SetVertexShader(D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1);
		STATEMANAGER.DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 4, 0, 2);
	}
	//OLD: STATEMANAGER.DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST, 0, 4, 2, c_FillRectIndices, D3DFMT_INDEX16, vertices, sizeof(TPDTVertex));
	////////////////////////////////////////////////////////////
}

const CGraphicTexture& CGraphicImageInstance::GetTextureReference() const
{
	return m_roImage->GetTextureReference();
}

CGraphicTexture* CGraphicImageInstance::GetTexturePointer()
{
	CGraphicImage* pkImage = m_roImage.GetPointer();
	return pkImage ? pkImage->GetTexturePointer() : NULL;
}

CGraphicImage* CGraphicImageInstance::GetGraphicImagePointer()
{
	return m_roImage.GetPointer();
}

int CGraphicImageInstance::GetWidth()
{
	if (IsEmpty())
		return 0;

	return m_roImage->GetWidth();
}

int CGraphicImageInstance::GetHeight()
{
	if (IsEmpty())
		return 0;

	return m_roImage->GetHeight();
}

void CGraphicImageInstance::SetDiffuseColor(float fr, float fg, float fb, float fa)
{
	m_DiffuseColor.r = fr;
	m_DiffuseColor.g = fg;
	m_DiffuseColor.b = fb;
	m_DiffuseColor.a = fa;
}

void CGraphicImageInstance::SetScale(D3DXVECTOR2 v2Scale)
{
	m_v2Scale = v2Scale;
}

void CGraphicImageInstance::SetScalePercent(BYTE byPercent)
{
	m_v2Scale.x *= byPercent;
	m_v2Scale.y *= byPercent;
}

const D3DXVECTOR2 & CGraphicImageInstance::GetScale() const
{
	return m_v2Scale;
}

void CGraphicImageInstance::SetScalePivotCenter(bool bScalePivotCenter)
{
	m_bScalePivotCenter = bScalePivotCenter;
}

void CGraphicImageInstance::SetPosition(float fx, float fy)
{
	m_v2Position.x = fx;
	m_v2Position.y = fy;
}

void CGraphicImageInstance::UpdatePosition(float fx, float fy)
{
	m_v2Position.x += fx;
	m_v2Position.y += fy;
}

#ifdef ENABLE_MODEL_RENDER_TARGET
void CGraphicImageInstance::SetScale(float fx, float fy)
{
	m_v2Scale.x = fx;
	m_v2Scale.y = fy;
}
#endif

void CGraphicImageInstance::SetImagePointer(CGraphicImage* pImage)
{
	m_roImage.SetPointer(pImage);

	OnSetImagePointer();
}

void CGraphicImageInstance::ReloadImagePointer(CGraphicImage* pImage)
{
	if (m_roImage.IsNull())
	{
		SetImagePointer(pImage);
		return;
	}

	CGraphicImage* pkImage = m_roImage.GetPointer();

	if (pkImage)
		pkImage->Reload();
}

bool CGraphicImageInstance::IsEmpty() const
{
	if (!m_roImage.IsNull() && !m_roImage->IsEmpty())
		return false;

	return true;
}

bool CGraphicImageInstance::operator == (const CGraphicImageInstance& rhs) const
{
	return (m_roImage.GetPointer() == rhs.m_roImage.GetPointer());
}

DWORD CGraphicImageInstance::Type()
{
	static DWORD s_dwType = GetCRC32("CGraphicImageInstance", strlen("CGraphicImageInstance"));
	return (s_dwType);
}

BOOL CGraphicImageInstance::IsType(DWORD dwType)
{
	return OnIsType(dwType);
}

BOOL CGraphicImageInstance::OnIsType(DWORD dwType)
{
	if (CGraphicImageInstance::Type() == dwType)
		return TRUE;

	return FALSE;
}

void CGraphicImageInstance::OnSetImagePointer()
{
}

void CGraphicImageInstance::Initialize()
{
	m_DiffuseColor.r = m_DiffuseColor.g = m_DiffuseColor.b = m_DiffuseColor.a = 1.0f;
	m_v2Position.x = m_v2Position.y = 0.0f;
	m_bScalePivotCenter = false;
	memset(&m_clipRect, 0, sizeof(RECT));
	m_bClipEnable = false;
#ifdef ENABLE_MODEL_RENDER_TARGET
	m_v2Scale.x = m_v2Scale.y = 1.0f;
#endif
}

void CGraphicImageInstance::Destroy()
{
	m_roImage.SetPointer(NULL);

	Initialize();
}

CGraphicImageInstance::CGraphicImageInstance()
{
	Initialize();
}

CGraphicImageInstance::~CGraphicImageInstance()
{
	Destroy();
}