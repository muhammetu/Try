#include "StdAfx.h"
#include "BlockTexture.h"
#include "GrpBase.h"
#include "GrpDib.h"
#include "../eterbase/Stl.h"
#include "../eterlib/StateManager.h"

void CBlockTexture::SetClipRect(const RECT& c_rRect)
{
	m_bClipEnable = TRUE;
	m_clipRect = c_rRect;
}

void CBlockTexture::Render(int ix, int iy, DWORD dwColor)
{
	RECT myRect = {ix + m_rect.left, iy + m_rect.top, ix + m_rect.left + m_dwWidth, iy + m_rect.top + m_dwHeight};

	float textureLeft = 0.0f;
	float textureTop = 0.0f;
	float textureRight = 1.0f;
	float textureBottom = 1.0f;

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
			textureLeft += float(idx) / float(m_dwWidth);
		}
		if (myRect.right > m_clipRect.right)
		{
			int idx = myRect.right - m_clipRect.right;
			myRect.right =  m_clipRect.right;
			textureRight -= float(idx) / float(m_dwWidth);
		}

		if (m_clipRect.top > myRect.top)
		{
			int idy = m_clipRect.top - myRect.top;
			myRect.top = m_clipRect.top;
			textureTop += float(idy) / float(m_dwHeight);
		}
		if (myRect.bottom > m_clipRect.bottom)
		{
			int idy = myRect.bottom - m_clipRect.bottom;
			myRect.bottom = m_clipRect.bottom;
			textureBottom -= float(idy) / float(m_dwHeight);
		}
	}

	TPDTVertex vertices[4];
	vertices[0].position.x = myRect.left - 0.5f;
	vertices[0].position.y = myRect.top - 0.5f;
	vertices[0].position.z = 0.0f;
	vertices[0].texCoord = TTextureCoordinate(textureLeft, textureTop);
	vertices[0].diffuse = dwColor;

	vertices[1].position.x = myRect.right - 0.5f;
	vertices[1].position.y = myRect.top - 0.5f;
	vertices[1].position.z = 0.0f;
	vertices[1].texCoord = TTextureCoordinate(textureRight, textureTop);
	vertices[1].diffuse = dwColor;

	vertices[2].position.x = myRect.left - 0.5f;
	vertices[2].position.y = myRect.bottom - 0.5f;
	vertices[2].position.z = 0.0f;
	vertices[2].texCoord = TTextureCoordinate(textureLeft, textureBottom);
	vertices[2].diffuse = dwColor;

	vertices[3].position.x = myRect.right - 0.5f;
	vertices[3].position.y = myRect.bottom - 0.5f;
	vertices[3].position.z = 0.0f;
	vertices[3].texCoord = TTextureCoordinate(textureRight, textureBottom);
	vertices[3].diffuse = dwColor;

	if (CGraphicBase::SetPDTStream(vertices, 4))
	{
		CGraphicBase::SetDefaultIndexBuffer(CGraphicBase::DEFAULT_IB_FILL_RECT);

		STATEMANAGER.SetTexture(0, m_lpd3dTexture);
		STATEMANAGER.SetTexture(1, NULL);
		STATEMANAGER.SetVertexShader(D3DFVF_XYZ | D3DFVF_TEX1 | D3DFVF_DIFFUSE);
		STATEMANAGER.DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 4, 0, 2);
	}
}

void CBlockTexture::InvalidateRect(const RECT& c_rsrcRect)
{
	RECT dstRect = m_rect;
	if (c_rsrcRect.right < dstRect.left ||
		c_rsrcRect.left > dstRect.right ||
		c_rsrcRect.bottom < dstRect.top ||
		c_rsrcRect.top > dstRect.bottom)
	{
		Tracef("InvalidateRect() - Strange rect");
		return;
	}

	// DIBBAR_LONGSIZE_BUGFIX
	const RECT clipRect = {
		max(c_rsrcRect.left - dstRect.left, 0),
		max(c_rsrcRect.top - dstRect.top, 0),
		min(c_rsrcRect.right - dstRect.left, dstRect.right - dstRect.left),
		min(c_rsrcRect.bottom - dstRect.top, dstRect.bottom - dstRect.top),
	};
	// END_OF_DIBBAR_LONGSIZE_BUGFIX

	DWORD* pdwSrc;
	pdwSrc = (DWORD*)m_pDIB->GetPointer();
	pdwSrc += dstRect.left + dstRect.top * m_pDIB->GetWidth();

	D3DLOCKED_RECT lockedRect;
	if (FAILED(m_lpd3dTexture->LockRect(0, &lockedRect, &clipRect, 0)))
	{
		Tracef("InvalidateRect() - Failed to LockRect");
		return;
	}

	int iclipWidth = clipRect.right - clipRect.left;
	int iclipHeight = clipRect.bottom - clipRect.top;
	DWORD* pdwDst = (DWORD*)lockedRect.pBits;
	DWORD dwDstWidth = lockedRect.Pitch >> 2;
	DWORD dwSrcWidth = m_pDIB->GetWidth();
	for (int i = 0; i < iclipHeight; ++i)
	{
		for (int i = 0; i < iclipWidth; ++i)
		{
			if (pdwSrc[i])
				pdwDst[i] = pdwSrc[i] | 0xff000000;
			else
				pdwDst[i] = 0;
		}
		pdwDst += dwDstWidth;
		pdwSrc += dwSrcWidth;
	}

	m_lpd3dTexture->UnlockRect(0);
}

bool CBlockTexture::Create(CGraphicDib* pDIB, const RECT& c_rRect, DWORD dwWidth, DWORD dwHeight)
{
	if (FAILED(ms_lpd3dDevice->CreateTexture(dwWidth, dwHeight, 0, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &m_lpd3dTexture)))
	{
		Tracef("Failed to create block texture %u, %u\n", dwWidth, dwHeight);
		return false;
	}

	m_pDIB = pDIB;
	m_rect = c_rRect;
	m_dwWidth = dwWidth;
	m_dwHeight = dwHeight;
	m_bClipEnable = FALSE;

	return true;
}

CBlockTexture::CBlockTexture() : m_bClipEnable(false), m_dwWidth(0), m_dwHeight(0)
{
	m_pDIB = NULL;
	m_lpd3dTexture = NULL;
}

CBlockTexture::~CBlockTexture()
{
	safe_release(m_lpd3dTexture);
	m_lpd3dTexture = NULL;
}
