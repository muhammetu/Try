#include "StdAfx.h"
#include "../eterLib/StateManager.h"
#include "../eterLib/GrpSubImage.h"
#include "../eterlib/Camera.h"
#include "../EterPack/EterPackManager.h"

#include "PythonMiniMap.h"
#include "PythonBackground.h"
#include "PythonCharacterManager.h"
#include "PythonGuild.h"
#include "PythonNonPlayer.h"

#include "AbstractPlayer.h"

#include "../eterPythonLib/PythonWindowManager.h"

void CPythonMiniMap::AddObserver(DWORD dwVID, float fSrcX, float fSrcY)
{
	std::map<DWORD, SObserver>::iterator f=m_kMap_dwVID_kObserver.find(dwVID);
	if (m_kMap_dwVID_kObserver.end()==f)
	{
		SObserver kObserver;
		kObserver.dwSrcTime=ELTimer_GetMSec();
		kObserver.dwDstTime=kObserver.dwSrcTime+1000;
		kObserver.fSrcX=fSrcX;
		kObserver.fSrcY=fSrcY;
		kObserver.fDstX=fSrcX;
		kObserver.fDstY=fSrcY;
		kObserver.fCurX=fSrcX;
		kObserver.fCurY=fSrcY;
		m_kMap_dwVID_kObserver.insert(std::map<DWORD, SObserver>::value_type(dwVID, kObserver));
	}
	else
	{
		SObserver& rkObserver=f->second;
		rkObserver.dwSrcTime=ELTimer_GetMSec();
		rkObserver.dwDstTime=rkObserver.dwSrcTime+1000;
		rkObserver.fSrcX=fSrcX;
		rkObserver.fSrcY=fSrcY;
		rkObserver.fDstX=fSrcX;
		rkObserver.fDstY=fSrcY;
		rkObserver.fCurX=fSrcX;
		rkObserver.fCurY=fSrcY;
	}
}

void CPythonMiniMap::MoveObserver(DWORD dwVID, float fDstX, float fDstY)
{
	std::map<DWORD, SObserver>::iterator f=m_kMap_dwVID_kObserver.find(dwVID);
	if (m_kMap_dwVID_kObserver.end()==f)
		return;

	SObserver& rkObserver=f->second;
	rkObserver.dwSrcTime=ELTimer_GetMSec();
	rkObserver.dwDstTime=rkObserver.dwSrcTime+1000;
	rkObserver.fSrcX=rkObserver.fCurX;
	rkObserver.fSrcY=rkObserver.fCurY;
	rkObserver.fDstX=fDstX;
	rkObserver.fDstY=fDstY;
}

void CPythonMiniMap::RemoveObserver(DWORD dwVID)
{
	m_kMap_dwVID_kObserver.erase(dwVID);
}

void CPythonMiniMap::SetCenterPosition(float fCenterX, float fCenterY)
{
	m_fCenterX = fCenterX;
	m_fCenterY = fCenterY;

	CMapOutdoor& rkMap = CPythonBackground::Instance().GetMapOutdoorRef();
	for (BYTE byTerrainNum = 0; byTerrainNum < AROUND_AREA_NUM; ++byTerrainNum)
	{
		m_lpMiniMapTexture[byTerrainNum] = NULL;
		CTerrain * pTerrain;
		if (rkMap.GetTerrainPointer(byTerrainNum, &pTerrain))
			m_lpMiniMapTexture[byTerrainNum] = pTerrain->GetMiniMapTexture();
	}

	const TOutdoorMapCoordinate & rOutdoorMapCoord = rkMap.GetCurCoordinate();

	m_fCenterCellX = (m_fCenterX - (float)(rOutdoorMapCoord.m_sTerrainCoordX * CTerrainImpl::TERRAIN_XSIZE)) / (float)(CTerrainImpl::CELLSCALE);
	m_fCenterCellY = (m_fCenterY - (float)(rOutdoorMapCoord.m_sTerrainCoordY * CTerrainImpl::TERRAIN_YSIZE)) / (float)(CTerrainImpl::CELLSCALE);

	__SetPosition();
}

void CPythonMiniMap::Update(float fCenterX, float fCenterY)
{
	CPythonBackground& rkBG=CPythonBackground::Instance();
	if (!rkBG.IsMapOutdoor())
		return;

	if (m_fCenterX != fCenterX || m_fCenterY != fCenterY )
		SetCenterPosition(fCenterX, fCenterY);

	m_MinimapPosVector.clear();
	float fooCellScale = 1.0f / ((float) CTerrainImpl::CELLSCALE);

	CPythonCharacterManager& rkChrMgr=CPythonCharacterManager::Instance();

	CInstanceBase* pkInstMain=rkChrMgr.GetMainInstancePtr();
	if (!pkInstMain)
		return;

	CPythonCharacterManager::CharacterIterator i;
	for(i = rkChrMgr.CharacterInstanceBegin(); i!=rkChrMgr.CharacterInstanceEnd(); ++i)
	{
		CInstanceBase* pkInstEach=*i;

		TPixelPosition kInstancePosition;
		pkInstEach->NEW_GetPixelPosition(&kInstancePosition);
		float fDistanceFromCenterX = (kInstancePosition.x - m_fCenterX) * fooCellScale * m_fScale;
		float fDistanceFromCenterY = (kInstancePosition.y - m_fCenterY) * fooCellScale * m_fScale;
		if (fabs(fDistanceFromCenterX) >= m_fMiniMapRadius || fabs(fDistanceFromCenterY) >= m_fMiniMapRadius)
			continue;

		float fDistanceFromCenter = sqrtf(fDistanceFromCenterX * fDistanceFromCenterX + fDistanceFromCenterY * fDistanceFromCenterY );
		if ( fDistanceFromCenter >= m_fMiniMapRadius )
			continue;

		TMarkPosition aMarkPosition;

		if (pkInstEach->IsPC() && !pkInstEach->IsInvisibility())
		{
			if (pkInstEach == CPythonCharacterManager::Instance().GetMainInstancePtr())
				continue;

			aMarkPosition.m_bType = CActorInstance::TYPE_PC;
			aMarkPosition.m_fX = ( m_fWidth - (float)m_WhiteMark.GetWidth() ) / 2.0f + fDistanceFromCenterX + m_fScreenX;
			aMarkPosition.m_fY = ( m_fHeight - (float)m_WhiteMark.GetHeight() ) / 2.0f + fDistanceFromCenterY + m_fScreenY;
			aMarkPosition.m_eNameColor=pkInstEach->GetNameColorIndex();
			m_MinimapPosVector.push_back (aMarkPosition);
		}
		else if (pkInstEach->IsNPC())
		{
			aMarkPosition.m_bType = CActorInstance::TYPE_NPC;
			aMarkPosition.m_fX = ( m_fWidth - (float)m_WhiteMark.GetWidth() ) / 2.0f + fDistanceFromCenterX + m_fScreenX;
			aMarkPosition.m_fY = ( m_fHeight - (float)m_WhiteMark.GetHeight() ) / 2.0f + fDistanceFromCenterY + m_fScreenY;

			m_MinimapPosVector.push_back (aMarkPosition);
		}
		else if (pkInstEach->IsEnemy())
		{
			aMarkPosition.m_bType = CActorInstance::TYPE_ENEMY;
			aMarkPosition.m_fX = ( m_fWidth - (float)m_WhiteMark.GetWidth() ) / 2.0f + fDistanceFromCenterX + m_fScreenX;
			aMarkPosition.m_fY = ( m_fHeight - (float)m_WhiteMark.GetHeight() ) / 2.0f + fDistanceFromCenterY + m_fScreenY;

			m_MinimapPosVector.push_back (aMarkPosition);
		}
		else if (pkInstEach->IsWarp())
		{
			aMarkPosition.m_bType = CActorInstance::TYPE_WARP;
			aMarkPosition.m_fX = ( m_fWidth - (float)m_WhiteMark.GetWidth() ) / 2.0f + fDistanceFromCenterX + m_fScreenX;
			aMarkPosition.m_fY = ( m_fHeight - (float)m_WhiteMark.GetHeight() ) / 2.0f + fDistanceFromCenterY + m_fScreenY;

			m_MinimapPosVector.push_back (aMarkPosition);
		}
#ifdef ENABLE_METINSTONE_ON_MINIMAP
		else if (pkInstEach->IsStone())
		{
			aMarkPosition.m_bType = CActorInstance::TYPE_STONE;
			aMarkPosition.m_fX = ( m_fWidth - (float)m_WhiteMark.GetWidth() ) / 2.0f + fDistanceFromCenterX + m_fScreenX;
			aMarkPosition.m_fY = ( m_fHeight - (float)m_WhiteMark.GetHeight() ) / 2.0f + fDistanceFromCenterY + m_fScreenY;

			m_MinimapPosVector.push_back (aMarkPosition);
		}
#endif
#ifdef ENABLE_BOSS_ON_MINIMAP
		else if (pkInstEach->IsBoss())
		{
			aMarkPosition.m_bType = TYPE_BOSS;
			aMarkPosition.m_fX = ( m_fWidth - (float)m_WhiteMark.GetWidth() ) / 2.0f + fDistanceFromCenterX + m_fScreenX;
			aMarkPosition.m_fY = ( m_fHeight - (float)m_WhiteMark.GetHeight() ) / 2.0f + fDistanceFromCenterY + m_fScreenY;

			m_MinimapPosVector.push_back (aMarkPosition);
		}
#endif
	}

	{
		DWORD dwCurTime=ELTimer_GetMSec();

		std::map<DWORD, SObserver>::iterator i;
		for (i=m_kMap_dwVID_kObserver.begin(); i!=m_kMap_dwVID_kObserver.end(); ++i)
		{
			SObserver& rkObserver=i->second;

			float fPos=float(dwCurTime-rkObserver.dwSrcTime)/float(rkObserver.dwDstTime-rkObserver.dwSrcTime);
			if (fPos < 0.0f)
				fPos = 0.0f;
			else if (fPos > 1.0f)
				fPos = 1.0f;

			rkObserver.fCurX=(rkObserver.fDstX-rkObserver.fSrcX)*fPos+rkObserver.fSrcX;
			rkObserver.fCurY=(rkObserver.fDstY-rkObserver.fSrcY)*fPos+rkObserver.fSrcY;

			TPixelPosition kInstancePosition;
			kInstancePosition.x=rkObserver.fCurX;
			kInstancePosition.y=rkObserver.fCurY;
			kInstancePosition.z=0.0f;

			float fDistanceFromCenterX = (kInstancePosition.x - m_fCenterX) * fooCellScale * m_fScale;
			float fDistanceFromCenterY = (kInstancePosition.y - m_fCenterY) * fooCellScale * m_fScale;
			if (fabs(fDistanceFromCenterX) >= m_fMiniMapRadius || fabs(fDistanceFromCenterY) >= m_fMiniMapRadius)
				continue;

			float fDistanceFromCenter = sqrtf(fDistanceFromCenterX * fDistanceFromCenterX + fDistanceFromCenterY * fDistanceFromCenterY );
			if ( fDistanceFromCenter >= m_fMiniMapRadius )
				continue;

			TMarkPosition aMarkPosition;
			aMarkPosition.m_bType = CActorInstance::TYPE_PC;
			aMarkPosition.m_fX = ( m_fWidth - (float)m_WhiteMark.GetWidth() ) / 2.0f + fDistanceFromCenterX + m_fScreenX;
			aMarkPosition.m_fY = ( m_fHeight - (float)m_WhiteMark.GetHeight() ) / 2.0f + fDistanceFromCenterY + m_fScreenY;
			aMarkPosition.m_eNameColor=CInstanceBase::NAMECOLOR_PARTY;
			m_MinimapPosVector.push_back (aMarkPosition);
		}
	}

	std::sort (m_MinimapPosVector.begin(), m_MinimapPosVector.end(), [] (TMarkPosition a, TMarkPosition b)
	{
		return a.m_bType > b.m_bType;
	}); //Sort minimap by TYPE => LESS GPU calls

	{
		TAtlasMarkInfoVector::iterator itor = m_AtlasWayPointInfoVector.begin();
		for (; itor != m_AtlasWayPointInfoVector.end(); ++itor)
		{
			TAtlasMarkInfo & rAtlasMarkInfo = *itor;

			if (TYPE_TARGET != rAtlasMarkInfo.m_byType)
				continue;

			if (0 != rAtlasMarkInfo.m_dwChrVID)
			{
				CInstanceBase * pInstance = CPythonCharacterManager::Instance().GetInstancePtr(rAtlasMarkInfo.m_dwChrVID);
				if (pInstance)
				{
					TPixelPosition kPixelPosition;
					pInstance->NEW_GetPixelPosition(&kPixelPosition);
					__UpdateWayPoint(&rAtlasMarkInfo, kPixelPosition.x, kPixelPosition.y);
				}
			}

			const float c_fMiniMapWindowRadius = 55.0f;

			float fDistanceFromCenterX = (rAtlasMarkInfo.m_fX - m_fCenterX) * fooCellScale * m_fScale;
			float fDistanceFromCenterY = (rAtlasMarkInfo.m_fY - m_fCenterY) * fooCellScale * m_fScale;
			float fDistanceFromCenter = sqrtf(fDistanceFromCenterX * fDistanceFromCenterX + fDistanceFromCenterY * fDistanceFromCenterY );

			if (fDistanceFromCenter >= c_fMiniMapWindowRadius)
			{
				float fRadianX = acosf(fDistanceFromCenterX / fDistanceFromCenter);
				float fRadianY = asinf(fDistanceFromCenterY / fDistanceFromCenter);
				fDistanceFromCenterX = 55.0f * cosf(fRadianX);
				fDistanceFromCenterY = 55.0f * sinf(fRadianY);
				rAtlasMarkInfo.m_fMiniMapX = ( m_fWidth - (float)m_WhiteMark.GetWidth() ) / 2.0f + fDistanceFromCenterX + m_fScreenX + 2.0f;
				rAtlasMarkInfo.m_fMiniMapY = ( m_fHeight - (float)m_WhiteMark.GetHeight() ) / 2.0f + fDistanceFromCenterY + m_fScreenY + 2.0f;
			}
			else
			{
				rAtlasMarkInfo.m_fMiniMapX = ( m_fWidth - (float)m_WhiteMark.GetWidth() ) / 2.0f + fDistanceFromCenterX + m_fScreenX;
				rAtlasMarkInfo.m_fMiniMapY = ( m_fHeight - (float)m_WhiteMark.GetHeight() ) / 2.0f + fDistanceFromCenterY + m_fScreenY;
			}
		}
	}
}

void CPythonMiniMap::Render(float fScreenX, float fScreenY)
{
	CPythonBackground& rkBG=CPythonBackground::Instance();
	if (!rkBG.IsMapOutdoor())
		return;

	if (!m_bShow)
		return;

	if (!rkBG.IsMapReady())
		return;

	if (m_fScreenX != fScreenX || m_fScreenY != fScreenY)
	{
		m_fScreenX = fScreenX;
		m_fScreenY = fScreenY;
		__SetPosition();
	}

	STATEMANAGER.SaveTextureStageState(0, D3DTSS_MIPFILTER, D3DTEXF_POINT);
	STATEMANAGER.SaveTextureStageState(0, D3DTSS_MINFILTER, D3DTEXF_POINT);
	STATEMANAGER.SaveTextureStageState(0, D3DTSS_MAGFILTER, D3DTEXF_POINT);

	STATEMANAGER.SaveTextureStageState(0, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
	STATEMANAGER.SaveTextureStageState(0, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);

	STATEMANAGER.SaveTextureStageState(1, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEPOSITION);
	STATEMANAGER.SaveTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
	STATEMANAGER.SaveTextureStageState(1, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
	STATEMANAGER.SaveTextureStageState(1, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);

	STATEMANAGER.SaveTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	STATEMANAGER.SaveTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	STATEMANAGER.SaveTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	STATEMANAGER.SaveTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	STATEMANAGER.SaveTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
	STATEMANAGER.SaveTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);

	STATEMANAGER.SaveTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	STATEMANAGER.SaveTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CURRENT);
	STATEMANAGER.SaveTextureStageState(1, D3DTSS_COLOROP, D3DTOP_MODULATE);
	STATEMANAGER.SaveTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	STATEMANAGER.SaveTextureStageState(1, D3DTSS_ALPHAARG2, D3DTA_CURRENT);
	STATEMANAGER.SaveTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);

	STATEMANAGER.SaveRenderState(D3DRS_TEXTUREFACTOR, 0xFF000000);

	STATEMANAGER.SetTexture(1, m_MiniMapFilterGraphicImageInstance.GetTexturePointer()->GetD3DTexture());
	STATEMANAGER.SetTransform(D3DTS_TEXTURE1, &m_matMiniMapCover);

	STATEMANAGER.SetVertexShader(D3DFVF_XYZ | D3DFVF_TEX1);
	STATEMANAGER.SetStreamSource(0, m_VertexBuffer.GetD3DVertexBuffer(), 20);
	STATEMANAGER.SetIndices(m_IndexBuffer.GetD3DIndexBuffer(), 0);
	STATEMANAGER.SetTransform(D3DTS_WORLD, &m_matWorld);

	for (BYTE byTerrainNum = 0; byTerrainNum < AROUND_AREA_NUM; ++byTerrainNum)
	{
		LPDIRECT3DTEXTURE8 pMiniMapTexture = m_lpMiniMapTexture[byTerrainNum];
		STATEMANAGER.SetTexture(0, pMiniMapTexture);
		if (pMiniMapTexture)
		{
			CStateManager& rkSttMgr=CStateManager::Instance();
			rkSttMgr.DrawIndexedPrimitive(D3DPT_TRIANGLELIST, byTerrainNum * 4, 4, byTerrainNum * 6, 2);
		}
		else
		{
			STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TFACTOR);
			STATEMANAGER.DrawIndexedPrimitive(D3DPT_TRIANGLELIST, byTerrainNum * 4, 4, byTerrainNum * 6, 2);
			STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		}
	}

	STATEMANAGER.RestoreRenderState(D3DRS_TEXTUREFACTOR);

	STATEMANAGER.RestoreTextureStageState(1, D3DTSS_ALPHAARG2);
	STATEMANAGER.RestoreTextureStageState(1, D3DTSS_ALPHAARG1);
	STATEMANAGER.RestoreTextureStageState(1, D3DTSS_ALPHAOP);
	STATEMANAGER.RestoreTextureStageState(1, D3DTSS_COLORARG1);
	STATEMANAGER.RestoreTextureStageState(1, D3DTSS_COLORARG2);
	STATEMANAGER.RestoreTextureStageState(1, D3DTSS_COLOROP);

	STATEMANAGER.RestoreTextureStageState(0, D3DTSS_ALPHAARG2);
	STATEMANAGER.RestoreTextureStageState(0, D3DTSS_ALPHAARG1);
	STATEMANAGER.RestoreTextureStageState(0, D3DTSS_ALPHAOP);
	STATEMANAGER.RestoreTextureStageState(0, D3DTSS_COLORARG1);
	STATEMANAGER.RestoreTextureStageState(0, D3DTSS_COLORARG2);
	STATEMANAGER.RestoreTextureStageState(0, D3DTSS_COLOROP);

	STATEMANAGER.RestoreTextureStageState(0, D3DTSS_ADDRESSU);
	STATEMANAGER.RestoreTextureStageState(0, D3DTSS_ADDRESSV);
	STATEMANAGER.RestoreTextureStageState(1, D3DTSS_TEXCOORDINDEX);
	STATEMANAGER.RestoreTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS);
	STATEMANAGER.RestoreTextureStageState(1, D3DTSS_ADDRESSU);
	STATEMANAGER.RestoreTextureStageState(1, D3DTSS_ADDRESSV);

	SetDiffuseOperation();
	STATEMANAGER.SetTransform(D3DTS_WORLD, &m_matIdentity);

	STATEMANAGER.SaveRenderState(D3DRS_TEXTUREFACTOR, 0xFFFFFFFF);
	STATEMANAGER.SaveTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TFACTOR);
	STATEMANAGER.SaveTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TEXTURE);
	STATEMANAGER.SaveTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	STATEMANAGER.SaveTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TFACTOR);
	STATEMANAGER.SaveTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_TEXTURE);
	STATEMANAGER.SaveTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG2);

	TInstancePositionVectorIterator aIterator;
	char stage = 0;
	bool scaleIsOK = m_fScale >= 2.0f;

	for (auto& i : m_MinimapPosVector)
	{
		if (!scaleIsOK && (i.m_bType == CActorInstance::TYPE_PC || i.m_bType == CActorInstance::TYPE_ENEMY)) // Stop iterating if we cant draw
			continue;

		if (i.m_bType == CActorInstance::TYPE_PC) // Can have multiple colors no stage used
		{
			if (i.m_eNameColor == CInstanceBase::NAMECOLOR_PARTY)
			{
				float v = (1 + sinf (CTimer::Instance().GetCurrentSecond() * 6)) / 5 + 0.6;
				D3DXCOLOR c (CInstanceBase::GetIndexedNameColor (CInstanceBase::NAMECOLOR_PARTY)); //(m_MarkTypeToColorMap[TYPE_PARTY]);
				D3DXCOLOR d (v, v, v, 1);
				D3DXColorModulate (&c, &c, &d);
				STATEMANAGER.SetRenderState (D3DRS_TEXTUREFACTOR, (DWORD)c);
			}
			else
				STATEMANAGER.SetRenderState (D3DRS_TEXTUREFACTOR, CInstanceBase::GetIndexedNameColor (i.m_eNameColor));
		}
		else if (stage != 2 && i.m_bType == CActorInstance::TYPE_ENEMY)
		{
			STATEMANAGER.SetRenderState (D3DRS_TEXTUREFACTOR, CInstanceBase::GetIndexedNameColor (CInstanceBase::NAMECOLOR_MOB)); //m_MarkTypeToColorMap[TYPE_MONSTER]);
			stage = 2;
		}
		else if (stage != 3 && i.m_bType == CActorInstance::TYPE_NPC)
		{
			STATEMANAGER.SetRenderState (D3DRS_TEXTUREFACTOR, CInstanceBase::GetIndexedNameColor (CInstanceBase::NAMECOLOR_NPC));
			stage = 3;
		}
		else if (stage != 4 && i.m_bType == CActorInstance::TYPE_WARP)
		{
			STATEMANAGER.SetRenderState (D3DRS_TEXTUREFACTOR, CInstanceBase::GetIndexedNameColor (CInstanceBase::NAMECOLOR_WARP));
			stage = 4;
		}
#ifdef ENABLE_METINSTONE_ON_MINIMAP
		else if (stage != 5 && i.m_bType == CActorInstance::TYPE_STONE)
		{
			STATEMANAGER.SetRenderState (D3DRS_TEXTUREFACTOR, CInstanceBase::GetIndexedNameColor (CInstanceBase::NAMECOLOR_STONE));
			stage = 5;
		}
#endif
#ifdef ENABLE_BOSS_ON_MINIMAP
		else if (stage != 6 && i.m_bType == TYPE_BOSS)
		{
			STATEMANAGER.SetRenderState (D3DRS_TEXTUREFACTOR, CInstanceBase::GetIndexedNameColor (CInstanceBase::NAMECOLOR_MOB));
			stage = 6;
		}
#endif

		m_WhiteMark.SetPosition (i.m_fX, i.m_fY);
		m_WhiteMark.Render();
	}

	STATEMANAGER.RestoreRenderState(D3DRS_TEXTUREFACTOR);

	STATEMANAGER.RestoreTextureStageState(0, D3DTSS_ALPHAARG2);
	STATEMANAGER.RestoreTextureStageState(0, D3DTSS_ALPHAARG1);
	STATEMANAGER.RestoreTextureStageState(0, D3DTSS_ALPHAOP);
	STATEMANAGER.RestoreTextureStageState(0, D3DTSS_COLORARG1);
	STATEMANAGER.RestoreTextureStageState(0, D3DTSS_COLORARG2);
	STATEMANAGER.RestoreTextureStageState(0, D3DTSS_COLOROP);

	STATEMANAGER.RestoreTextureStageState(0, D3DTSS_MIPFILTER);
	STATEMANAGER.RestoreTextureStageState(0, D3DTSS_MINFILTER);
	STATEMANAGER.RestoreTextureStageState(0, D3DTSS_MAGFILTER);

	STATEMANAGER.SaveTextureStageState(0, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
	STATEMANAGER.SaveTextureStageState(0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);

	CInstanceBase * pkInst = CPythonCharacterManager::Instance().GetMainInstancePtr();

	if (pkInst)
	{
		float fRotation;
		fRotation = (540.0f - pkInst->GetRotation());
		while(fRotation > 360.0f)
			fRotation -= 360.0f;
		while(fRotation < 0.0f)
			fRotation += 360.0f;

		m_PlayerMark.SetRotation(fRotation);
		m_PlayerMark.Render();
	}

	// Target
	{
		TAtlasMarkInfoVector::iterator itor = m_AtlasWayPointInfoVector.begin();
		for (; itor != m_AtlasWayPointInfoVector.end(); ++itor)
		{
			TAtlasMarkInfo & rAtlasMarkInfo = *itor;

			if (TYPE_TARGET != rAtlasMarkInfo.m_byType)
				continue;
			if (rAtlasMarkInfo.m_fMiniMapX <= 0.0f)
				continue;
			if (rAtlasMarkInfo.m_fMiniMapY <= 0.0f)
				continue;

			__RenderTargetMark(rAtlasMarkInfo.m_fMiniMapX, rAtlasMarkInfo.m_fMiniMapY);
		}
	}

	CCamera* pkCmrCur=CCameraManager::Instance().GetCurrentCamera();

	if (pkCmrCur)
	{
		m_MiniMapCameraraphicImageInstance.SetRotation(pkCmrCur->GetRoll());
		m_MiniMapCameraraphicImageInstance.Render();
	}
	STATEMANAGER.RestoreTextureStageState(0, D3DTSS_MINFILTER);
	STATEMANAGER.RestoreTextureStageState(0, D3DTSS_MAGFILTER);
}

void CPythonMiniMap::SetScale(float fScale)
{
	if (fScale >= 4.0f)
		fScale = 4.0f;
	if (fScale <= 0.5f)
		fScale = 0.5f;
	m_fScale = fScale;

	__SetPosition();
}

void CPythonMiniMap::ScaleUp()
{
	m_fScale *= 2.0f;
	if (m_fScale >= 4.0f)
		m_fScale = 4.0f;
	__SetPosition();
}

void CPythonMiniMap::ScaleDown()
{
	m_fScale *= 0.5f;
	if (m_fScale <= 0.5f)
		m_fScale = 0.5f;
	__SetPosition();
}

void CPythonMiniMap::SetMiniMapSize(float fWidth, float fHeight)
{
	m_fWidth = fWidth;
	m_fHeight = fHeight;
}

#pragma pack(push)
#pragma pack(1)
typedef struct _MINIMAPVERTEX
{
    float x, y, z;          // position
    float u, v;       // normal
} MINIMAPVERTEX, *LPMINIMAPVERTEX;
#pragma pack(pop)

bool CPythonMiniMap::Create()
{
	const std::string strImageRoot = "D:/ymir work/ui/";
	const std::string strImageFilter = strImageRoot + "minimap_image_filter.dds";
	const std::string strImageCamera = strImageRoot + "minimap_camera.dds";
	const std::string strPlayerMark = strImageRoot + "minimap/playermark.sub";
	const std::string strWhiteMark = strImageRoot + "minimap/whitemark_new.tga";

	CGraphicImage * pImage = (CGraphicImage *) CResourceManager::Instance().GetResourcePointer(strImageFilter.c_str());
	m_MiniMapFilterGraphicImageInstance.SetImagePointer(pImage);
	pImage = (CGraphicImage *) CResourceManager::Instance().GetResourcePointer(strImageCamera.c_str());
	m_MiniMapCameraraphicImageInstance.SetImagePointer(pImage);

	m_matMiniMapCover._11 = 1.0f / ((float)m_MiniMapFilterGraphicImageInstance.GetWidth());
	m_matMiniMapCover._22 = 1.0f / ((float)m_MiniMapFilterGraphicImageInstance.GetHeight());
	m_matMiniMapCover._33 = 0.0f;

	CGraphicSubImage * pSubImage = (CGraphicSubImage *) CResourceManager::Instance().GetResourcePointer(strPlayerMark.c_str());
	m_PlayerMark.SetImagePointer(pSubImage);

	CGraphicImage * pWhiteMark = (CGraphicImage *) CResourceManager::Instance().GetResourcePointer(strWhiteMark.c_str());
	m_WhiteMark.SetImagePointer(pWhiteMark);

	char buf[256];
	for (int i = 0; i < MINI_WAYPOINT_IMAGE_COUNT; ++i)
	{
		sprintf(buf, "%sminimap/mini_waypoint%02d.sub", strImageRoot.c_str(), i+1);
		m_MiniWayPointGraphicImageInstances[i].SetImagePointer((CGraphicSubImage *) CResourceManager::Instance().GetResourcePointer(buf));
		m_MiniWayPointGraphicImageInstances[i].SetRenderingMode(CGraphicExpandedImageInstance::RENDERING_MODE_SCREEN);
	}
	for (int j = 0; j < WAYPOINT_IMAGE_COUNT; ++j)
	{
		sprintf(buf, "%sminimap/waypoint%02d.sub", strImageRoot.c_str(), j+1);
		m_WayPointGraphicImageInstances[j].SetImagePointer((CGraphicSubImage *) CResourceManager::Instance().GetResourcePointer(buf));
		m_WayPointGraphicImageInstances[j].SetRenderingMode(CGraphicExpandedImageInstance::RENDERING_MODE_SCREEN);
	}
	for (int k = 0; k < TARGET_MARK_IMAGE_COUNT; ++k)
	{
		sprintf(buf, "%sminimap/targetmark%02d.sub", strImageRoot.c_str(), k+1);
		m_TargetMarkGraphicImageInstances[k].SetImagePointer((CGraphicSubImage *) CResourceManager::Instance().GetResourcePointer(buf));
		m_TargetMarkGraphicImageInstances[k].SetRenderingMode(CGraphicExpandedImageInstance::RENDERING_MODE_SCREEN);
	}

#pragma pack(push)
#pragma pack(1)
	LPMINIMAPVERTEX		lpMiniMapVertex;
	LPMINIMAPVERTEX		lpOrigMiniMapVertex;
#pragma pack(pop)

	if (!m_VertexBuffer.Create(36, D3DFVF_XYZ | D3DFVF_TEX1, D3DUSAGE_WRITEONLY, D3DPOOL_MANAGED) )
	{
		return false;
	}

	if (m_VertexBuffer.Lock((void **) &lpOrigMiniMapVertex))
	{
		char * pchMiniMapVertex = (char *)lpOrigMiniMapVertex;
		memset(pchMiniMapVertex, 0, sizeof(char) * 720);
		lpMiniMapVertex = (LPMINIMAPVERTEX) pchMiniMapVertex;

		for (int iY = -3; iY <= 1; ++iY)
		{
			if (0 == iY%2)
				continue;
			float fY = 0.5f * ((float)iY);
			for (int iX = -3; iX <= 1; ++iX)
			{
				if (0 == iX%2)
					continue;
				float fX = 0.5f * ((float)iX);
				lpMiniMapVertex = (LPMINIMAPVERTEX) pchMiniMapVertex;
				lpMiniMapVertex->x = fX;
				lpMiniMapVertex->y = fY;
				lpMiniMapVertex->z = 0.0f;
				lpMiniMapVertex->u = 0.0f;
				lpMiniMapVertex->v = 0.0f;
				pchMiniMapVertex += 20;
				lpMiniMapVertex = (LPMINIMAPVERTEX) pchMiniMapVertex;
				lpMiniMapVertex->x = fX;
				lpMiniMapVertex->y = fY + 1.0f;
				lpMiniMapVertex->z = 0.0f;
				lpMiniMapVertex->u = 0.0f;
				lpMiniMapVertex->v = 1.0f;
				pchMiniMapVertex += 20;
				lpMiniMapVertex = (LPMINIMAPVERTEX) pchMiniMapVertex;
				lpMiniMapVertex->x = fX + 1.0f;
				lpMiniMapVertex->y = fY;
				lpMiniMapVertex->z = 0.0f;
				lpMiniMapVertex->u = 1.0f;
				lpMiniMapVertex->v = 0.0f;
				pchMiniMapVertex += 20;
				lpMiniMapVertex = (LPMINIMAPVERTEX) pchMiniMapVertex;
				lpMiniMapVertex->x = fX + 1.0f;
				lpMiniMapVertex->y = fY + 1.0f;
				lpMiniMapVertex->z = 0.0f;
				lpMiniMapVertex->u = 1.0f;
				lpMiniMapVertex->v = 1.0f;
				pchMiniMapVertex += 20;
			}
		}

		m_VertexBuffer.Unlock();
	}

	if (!m_IndexBuffer.Create(54, D3DFMT_INDEX16))
	{
		return false;
	}

	WORD pwIndices[54] =
	{
		0, 1, 2, 2, 1, 3,
		4, 5, 6, 6, 5, 7,
		8, 9, 10, 10, 9, 11,

		12, 13, 14, 14, 13, 15,
		16, 17, 18, 18, 17, 19,
		20, 21, 22, 22, 21, 23,

		24, 25, 26, 26, 25, 27,
		28, 29, 30, 30, 29, 31,
		32, 33, 34, 34, 33, 35
	};

	void * pIndices;

	if (m_IndexBuffer.Lock(&pIndices))
	{
		memcpy(pIndices, pwIndices, 54 * sizeof(WORD));
		m_IndexBuffer.Unlock();
	}

	return true;
}

void CPythonMiniMap::__SetPosition()
{
	m_fMiniMapRadius = fMIN(6400.0f / ((float) CTerrainImpl::CELLSCALE) * m_fScale, 64.0f);

	m_matWorld._11 = m_fWidth * m_fScale;
	m_matWorld._22 = m_fHeight * m_fScale;
	m_matWorld._41 = (1.0f + m_fScale) * m_fWidth * 0.5f - m_fCenterCellX * m_fScale + m_fScreenX;
	m_matWorld._42 = (1.0f + m_fScale) * m_fHeight * 0.5f - m_fCenterCellY * m_fScale + m_fScreenY;

	if (!m_MiniMapFilterGraphicImageInstance.IsEmpty())
	{
		m_matMiniMapCover._41 = -(m_fScreenX) / ((float)m_MiniMapFilterGraphicImageInstance.GetWidth());
		m_matMiniMapCover._42 = -(m_fScreenY) / ((float)m_MiniMapFilterGraphicImageInstance.GetHeight());
	}

	if (!m_PlayerMark.IsEmpty())
		m_PlayerMark.SetPosition( ( m_fWidth - (float)m_PlayerMark.GetWidth() ) / 2.0f + m_fScreenX,
		( m_fHeight - (float)m_PlayerMark.GetHeight() ) / 2.0f  + m_fScreenY );

	if (!m_MiniMapCameraraphicImageInstance.IsEmpty())
		m_MiniMapCameraraphicImageInstance.SetPosition( ( m_fWidth - (float)m_MiniMapCameraraphicImageInstance.GetWidth() ) / 2.0f + m_fScreenX,
		( m_fHeight - (float)m_MiniMapCameraraphicImageInstance.GetHeight() ) / 2.0f  + m_fScreenY );
}

//////////////////////////////////////////////////////////////////////////
// Atlas

void CPythonMiniMap::ClearAtlasMarkInfo()
{
	m_AtlasNPCInfoVector.clear();
	m_AtlasWarpInfoVector.clear();
	m_AtlasStoneInfoVector.clear();
	m_AtlasBossInfoVector.clear();
}

void CPythonMiniMap::RegisterAtlasMark(BYTE byType, const char * c_szName, long lx, long ly)
{
	TAtlasMarkInfo aAtlasMarkInfo;
	memset(&aAtlasMarkInfo, 0, sizeof(aAtlasMarkInfo));

	aAtlasMarkInfo.m_fX = float(lx);
	aAtlasMarkInfo.m_fY = float(ly);
	aAtlasMarkInfo.m_strText = c_szName;

	aAtlasMarkInfo.m_fScreenX = aAtlasMarkInfo.m_fX / m_fAtlasMaxX * m_fAtlasImageSizeX - (float)m_WhiteMark.GetWidth() / 2.0f;
	aAtlasMarkInfo.m_fScreenY = aAtlasMarkInfo.m_fY / m_fAtlasMaxY * m_fAtlasImageSizeY - (float)m_WhiteMark.GetHeight() / 2.0f;

	switch(byType)
	{
		case CActorInstance::TYPE_NPC:
			aAtlasMarkInfo.m_byType = TYPE_NPC;
			m_AtlasNPCInfoVector.push_back(aAtlasMarkInfo);
			break;
		case CActorInstance::TYPE_WARP:
			aAtlasMarkInfo.m_byType = TYPE_WARP;
			{
				int iPos = aAtlasMarkInfo.m_strText.find(" ");
				if (iPos >= 0)
					aAtlasMarkInfo.m_strText[iPos]=0;

			}
			m_AtlasWarpInfoVector.push_back(aAtlasMarkInfo);
			break;
		case CActorInstance::TYPE_STONE:
			aAtlasMarkInfo.m_byType = TYPE_STONE;
			m_AtlasStoneInfoVector.push_back(aAtlasMarkInfo);
			break;
		case TYPE_BOSS:
			aAtlasMarkInfo.m_byType = TYPE_BOSS;
			m_AtlasBossInfoVector.push_back(aAtlasMarkInfo);
			break;
	}
}


void CPythonMiniMap::CreateTarget(int iID, const char * c_szName)
{
	AddWayPoint(TYPE_TARGET, iID, 0.0f, 0.0f, c_szName);
}

void CPythonMiniMap::UpdateTarget(int iID, int ix, int iy)
{
	TAtlasMarkInfo * pkInfo;
	if (!__GetWayPoint(iID, &pkInfo))
		return;

	if (0 != pkInfo->m_dwChrVID)
	{
		if (CPythonCharacterManager::Instance().GetInstancePtr(pkInfo->m_dwChrVID))
			return;
	}

	if (ix < m_dwAtlasBaseX)
		return;
	if (iy < m_dwAtlasBaseY)
		return;
	if (ix > m_dwAtlasBaseX+DWORD(m_fAtlasMaxX))
		return;
	if (iy > m_dwAtlasBaseY+DWORD(m_fAtlasMaxY))
		return;

	__UpdateWayPoint(pkInfo, ix-int(m_dwAtlasBaseX), iy-int(m_dwAtlasBaseY));
}

void CPythonMiniMap::CreateTarget(int iID, const char * c_szName, DWORD dwVID)
{
	AddWayPoint(TYPE_TARGET, iID, 0.0f, 0.0f, c_szName, dwVID);
}

void CPythonMiniMap::DeleteTarget(int iID)
{
	RemoveWayPoint(iID);
}

void CPythonMiniMap::__LoadAtlasMarkInfo(const char* fileName)
{
	CPythonBackground & rkBG=CPythonBackground::Instance();
	if (!rkBG.IsMapOutdoor())
		return;

	CMapOutdoor & rkMap=rkBG.GetMapOutdoorRef();

	// LOCALE
	char szAtlasMarkInfoFileName[64 + 1];
	_snprintf(szAtlasMarkInfoFileName, sizeof(szAtlasMarkInfoFileName), "%s/map/%s/%s.txt", LocaleService_GetLocalePath(), rkMap.GetName().c_str(), fileName);
	// END_OF_LOCALE

	const VOID* pvData;
	CMappedFile kFile;
	if (!CEterPackManager::Instance().Get(kFile, szAtlasMarkInfoFileName, &pvData))
	{
		Tracenf("CPythonMiniMap::__LoadAtlasMarkInfo(szAtlasMarkInfoFileName=%s) - Load Error", szAtlasMarkInfoFileName);
		return;
	}

	CMemoryTextFileLoader kTextFileLoader;
	kTextFileLoader.Bind(kFile.Size(), pvData);

	std::string stTemp;
	CTokenVector kTokenVector;
	for (DWORD i = 0; i < kTextFileLoader.GetLineCount(); ++i)
	{
		if (!kTextFileLoader.SplitLine(i, &kTokenVector, "\t"))
			continue;

		if (kTokenVector.size() < 10)
			continue;

		TAtlasMarkInfo aAtlasMarkInfo;

		const std::string& c_rsRegenType  = kTokenVector[0].c_str();
		const std::string& c_rstrPositionX = kTokenVector[1].c_str();
		const std::string& c_rstrPositionY = kTokenVector[2].c_str();
		const std::string& c_rstrRebornTime = kTokenVector[7].substr(0, kTokenVector[7].size()-1);
		const std::string& c_rstrVnum = kTokenVector[10].c_str();
		const DWORD rebornTime = atoi(c_rstrRebornTime.c_str());
		const DWORD c_dwVnum = atoi(c_rstrVnum.c_str());
		const int c_position_x = atoi(c_rstrPositionX.c_str());
		const int c_position_y = atoi(c_rstrPositionY.c_str());

		if (c_rsRegenType.compare("m") != 0)
			continue;

		const CPythonNonPlayer::TMobTable* c_pMobTable = CPythonNonPlayer::Instance().GetTable(c_dwVnum);
		if (c_pMobTable)
		{
			TAtlasMarkInfo aAtlasMarkInfo;
			aAtlasMarkInfo.m_fX = (float)(c_position_x * 100);
			aAtlasMarkInfo.m_fY = (float)(c_position_y * 100);

			switch (c_pMobTable->bType)
			{
				case CActorInstance::TYPE_NPC:
					{
						aAtlasMarkInfo.m_byType = TYPE_NPC;
						aAtlasMarkInfo.m_strText = c_pMobTable->szLocaleName;
					}
					break;

				case CActorInstance::TYPE_WARP:
					{
						aAtlasMarkInfo.m_byType = TYPE_WARP;
						aAtlasMarkInfo.m_strText = c_pMobTable->szLocaleName;
						int iPos = aAtlasMarkInfo.m_strText.find_first_of("|", 0);
						if (iPos >= 0)
							aAtlasMarkInfo.m_strText[iPos] = 0;
					}
					break;

				case CActorInstance::TYPE_STONE:
					{
						aAtlasMarkInfo.m_byType = TYPE_STONE;
						aAtlasMarkInfo.m_strText = c_pMobTable->szLocaleName + std::string(" - ") + std::string("Do�ma S�resi: ") + convertTime(rebornTime);
					}
					break;

				case CActorInstance::TYPE_ENEMY:
					{
						if (!IS_SET(c_pMobTable->dwRaceFlag, (1 << 17)))
							continue;

						aAtlasMarkInfo.m_byType = TYPE_BOSS;
						aAtlasMarkInfo.m_strText = c_pMobTable->szLocaleName + std::string(" - ") + std::string("Do�ma S�resi: ") + convertTime(rebornTime);
					}
					break;
				
				default:
					aAtlasMarkInfo.m_byType = 99;
					aAtlasMarkInfo.m_strText = "Gizemli Yarat�k";
					break;
			}

			aAtlasMarkInfo.m_fScreenX = aAtlasMarkInfo.m_fX / m_fAtlasMaxX * m_fAtlasImageSizeX - (float)m_WhiteMark.GetWidth() / 2.0f;
			aAtlasMarkInfo.m_fScreenY = aAtlasMarkInfo.m_fY / m_fAtlasMaxY * m_fAtlasImageSizeY - (float)m_WhiteMark.GetHeight() / 2.0f;

			switch (aAtlasMarkInfo.m_byType)
			{
				case TYPE_NPC:
					m_AtlasNPCInfoVector.push_back(aAtlasMarkInfo);
					break;
				case TYPE_WARP:
					m_AtlasWarpInfoVector.push_back(aAtlasMarkInfo);
					break;
				case TYPE_STONE:
					m_AtlasStoneInfoVector.push_back(aAtlasMarkInfo);
					break;
				case TYPE_BOSS:
					m_AtlasBossInfoVector.push_back(aAtlasMarkInfo);
					break;
			}
		}
	}
}

bool CPythonMiniMap::LoadAtlas()
{
	CPythonBackground& rkBG=CPythonBackground::Instance();
	if (!rkBG.IsMapOutdoor())
		return false;

	CMapOutdoor& rkMap=rkBG.GetMapOutdoorRef();

	const char* playerMarkFileName = "d:/ymir work/ui/minimap/playermark.sub";

	char atlasFileName[1024+1];
	snprintf(atlasFileName, sizeof(atlasFileName), "%s/atlas.sub", rkMap.GetName().c_str());
	if (!CEterPackManager::Instance().isExist(atlasFileName))
	{
		snprintf(atlasFileName, sizeof(atlasFileName), "d:/ymir work/ui/atlas/%s/atlas.sub", rkMap.GetName().c_str());
	}

	m_AtlasImageInstance.Destroy();
	m_AtlasPlayerMark.Destroy();
	CGraphicImage* pkGrpImgAtlas = (CGraphicImage *) CResourceManager::Instance().GetResourcePointer(atlasFileName);
	if (pkGrpImgAtlas)
	{
		m_AtlasImageInstance.SetImagePointer(pkGrpImgAtlas);

		if (pkGrpImgAtlas->IsEmpty())
			m_bAtlas=false;
		else
			m_bAtlas=true;
	}

	m_AtlasPlayerMark.SetImagePointer((CGraphicSubImage *) CResourceManager::Instance().GetResourcePointer(playerMarkFileName));

	short sTerrainCountX, sTerrainCountY;
	rkMap.GetBaseXY(&m_dwAtlasBaseX, &m_dwAtlasBaseY);
	rkMap.GetTerrainCount(&sTerrainCountX, &sTerrainCountY);
	m_fAtlasMaxX = (float) sTerrainCountX * CTerrainImpl::TERRAIN_XSIZE;
	m_fAtlasMaxY = (float) sTerrainCountY * CTerrainImpl::TERRAIN_YSIZE;

	m_fAtlasImageSizeX = (float) m_AtlasImageInstance.GetWidth();
	m_fAtlasImageSizeY = (float) m_AtlasImageInstance.GetHeight();

	ClearAtlasMarkInfo();
	const char* atlas_files[] = { "npc", "boss", "stone" };
	for (int i = 0; i < _countof(atlas_files); i++)
		__LoadAtlasMarkInfo(atlas_files[i]);

	if (m_bShowAtlas)
		OpenAtlasWindow();

	return true;
}

void CPythonMiniMap::__GlobalPositionToAtlasPosition(long lx, long ly, float * pfx, float * pfy)
{
	*pfx = lx / m_fAtlasMaxX * m_fAtlasImageSizeX;
	*pfy = ly / m_fAtlasMaxY * m_fAtlasImageSizeY;
}

void CPythonMiniMap::UpdateAtlas()
{
	CInstanceBase * pkInst = CPythonCharacterManager::Instance().GetMainInstancePtr();

	if (pkInst)
	{
		TPixelPosition kInstPos;
		pkInst->NEW_GetPixelPosition(&kInstPos);

		float fRotation;
		fRotation = (540.0f - pkInst->GetRotation());
		while(fRotation > 360.0f)
			fRotation -= 360.0f;
		while(fRotation < 0.0f)
			fRotation += 360.0f;

		m_AtlasPlayerMark.SetPosition(kInstPos.x / m_fAtlasMaxX * m_fAtlasImageSizeX - (float)m_AtlasPlayerMark.GetWidth() / 2.0f,
			kInstPos.y / m_fAtlasMaxY * m_fAtlasImageSizeY - (float)m_AtlasPlayerMark.GetHeight() / 2.0f);
		m_AtlasPlayerMark.SetRotation(fRotation);
	}
}

void CPythonMiniMap::RenderAtlas(float fScreenX, float fScreenY)
{
	if (!m_bShowAtlas)
		return;

	if (m_fAtlasScreenX != fScreenX || m_fAtlasScreenY != fScreenY)
	{
		m_matWorldAtlas._41 = fScreenX;
		m_matWorldAtlas._42 = fScreenY;
		m_fAtlasScreenX = fScreenX;
		m_fAtlasScreenY = fScreenY;
	}

	STATEMANAGER.SetTransform(D3DTS_WORLD, &m_matWorldAtlas);
	STATEMANAGER.SaveTextureStageState(0, D3DTSS_MINFILTER, D3DTEXF_POINT);
	STATEMANAGER.SaveTextureStageState(0, D3DTSS_MAGFILTER, D3DTEXF_POINT);
	m_AtlasImageInstance.Render();

	STATEMANAGER.SaveRenderState(D3DRS_TEXTUREFACTOR, 0xFFFFFFFF);
	STATEMANAGER.SaveTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TFACTOR);
	STATEMANAGER.SaveTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TEXTURE);
	STATEMANAGER.SaveTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);

	if (m_bAtlasRenderNpc)
	{
		STATEMANAGER.SetRenderState (D3DRS_TEXTUREFACTOR, CInstanceBase::GetIndexedNameColor (CInstanceBase::NAMECOLOR_NPC));
		for (auto& rAtlasMarkInfo : m_AtlasNPCInfoVector)
		{
			m_WhiteMark.SetPosition (rAtlasMarkInfo.m_fScreenX, rAtlasMarkInfo.m_fScreenY);
			m_WhiteMark.Render();
		}
	}

	if (m_bAtlasRenderWarp)
	{
		STATEMANAGER.SetRenderState (D3DRS_TEXTUREFACTOR, CInstanceBase::GetIndexedNameColor (CInstanceBase::NAMECOLOR_WARP));
		for (auto& rAtlasMarkInfo : m_AtlasWarpInfoVector)
		{
			m_WhiteMark.SetPosition (rAtlasMarkInfo.m_fScreenX, rAtlasMarkInfo.m_fScreenY);
			m_WhiteMark.Render();
		}
	}

	if (m_bAtlasRenderStone)
	{
		STATEMANAGER.SetRenderState (D3DRS_TEXTUREFACTOR, CInstanceBase::GetIndexedNameColor (CInstanceBase::NAMECOLOR_STONE));
		for (auto& rAtlasMarkInfo : m_AtlasStoneInfoVector)
		{
			m_WhiteMark.SetPosition (rAtlasMarkInfo.m_fScreenX, rAtlasMarkInfo.m_fScreenY);
			m_WhiteMark.Render();
		}
	}

	if (m_bAtlasRenderBoss)
	{
		STATEMANAGER.SetRenderState (D3DRS_TEXTUREFACTOR, CInstanceBase::GetIndexedNameColor (CInstanceBase::NAMECOLOR_BOSS));
		for (auto& rAtlasMarkInfo : m_AtlasBossInfoVector)
		{
			m_WhiteMark.SetPosition (rAtlasMarkInfo.m_fScreenX, rAtlasMarkInfo.m_fScreenY);
			m_WhiteMark.Render();
		}
	}

	if (m_bAtlasRenderWaypoint)
	{
		STATEMANAGER.SetTextureStageState (0, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
		STATEMANAGER.SetTextureStageState (0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
		STATEMANAGER.SetRenderState (D3DRS_TEXTUREFACTOR, CInstanceBase::GetIndexedNameColor (CInstanceBase::NAMECOLOR_WAYPOINT));

		for (auto& rAtlasMarkInfo : m_AtlasWayPointInfoVector)
		{
			if (rAtlasMarkInfo.m_fScreenX <= 0.0f)
				continue;

			if (rAtlasMarkInfo.m_fScreenY <= 0.0f)
				continue;

			if (TYPE_TARGET == rAtlasMarkInfo.m_byType)
				__RenderMiniWayPointMark (rAtlasMarkInfo.m_fScreenX, rAtlasMarkInfo.m_fScreenY);
			else
				__RenderWayPointMark (rAtlasMarkInfo.m_fScreenX, rAtlasMarkInfo.m_fScreenY);
		}
	}

	STATEMANAGER.RestoreRenderState(D3DRS_TEXTUREFACTOR);

	STATEMANAGER.RestoreTextureStageState(0, D3DTSS_COLORARG1);
	STATEMANAGER.RestoreTextureStageState(0, D3DTSS_COLORARG2);
	STATEMANAGER.RestoreTextureStageState(0, D3DTSS_COLOROP);

	if ((ELTimer_GetMSec() / 500) % 2)
		m_AtlasPlayerMark.Render();

	STATEMANAGER.RestoreTextureStageState(0, D3DTSS_MINFILTER);
	STATEMANAGER.RestoreTextureStageState(0, D3DTSS_MAGFILTER);
	STATEMANAGER.SetTransform(D3DTS_WORLD, &m_matIdentity);
}

bool CPythonMiniMap::GetPickedInstanceInfo(float fScreenX, float fScreenY, std::string & rReturnName, float * pReturnPosX, float * pReturnPosY, DWORD * pdwTextColor)
{
	float fDistanceFromMiniMapCenterX = fScreenX - m_fScreenX - m_fWidth * 0.5f;
	float fDistanceFromMiniMapCenterY = fScreenY - m_fScreenY - m_fHeight * 0.5f;

	if (sqrtf(fDistanceFromMiniMapCenterX * fDistanceFromMiniMapCenterX + fDistanceFromMiniMapCenterY * fDistanceFromMiniMapCenterY) > m_fMiniMapRadius )
		return false;

	float fRealX = m_fCenterX + fDistanceFromMiniMapCenterX / m_fScale * ((float) CTerrainImpl::CELLSCALE);
	float fRealY = m_fCenterY + fDistanceFromMiniMapCenterY / m_fScale * ((float) CTerrainImpl::CELLSCALE);

	CInstanceBase * pkInst = CPythonCharacterManager::Instance().GetMainInstancePtr();

	if (pkInst)
	{
		TPixelPosition kInstPos;
		pkInst->NEW_GetPixelPosition(&kInstPos);

		if (fabs(kInstPos.x - fRealX) < ((float) CTerrainImpl::CELLSCALE) * 6.0f / m_fScale &&
			fabs(kInstPos.y - fRealY) < ((float) CTerrainImpl::CELLSCALE) * 6.0f / m_fScale)
		{
			rReturnName = pkInst->GetNameString();
			*pReturnPosX = kInstPos.x;
			*pReturnPosY = kInstPos.y;
			*pdwTextColor = pkInst->GetNameColor();
			return true;
		}
	}

	if (m_fScale < 1.0f)
		return false;

	CPythonCharacterManager& rkChrMgr=CPythonCharacterManager::Instance();
	CPythonCharacterManager::CharacterIterator i;
	for(i = rkChrMgr.CharacterInstanceBegin(); i!=rkChrMgr.CharacterInstanceEnd(); ++i)
	{
		CInstanceBase* pkInstEach=*i;
		if (pkInstEach->IsInvisibility())
			continue;
		if (m_fScale < 2.0f && (pkInstEach->IsEnemy() || pkInstEach->IsPC() || pkInstEach->IsBoss()))
			continue;
		TPixelPosition kInstancePosition;
		pkInstEach->NEW_GetPixelPosition(&kInstancePosition);

		if (fabs(kInstancePosition.x - fRealX) < ((float) CTerrainImpl::CELLSCALE) * 3.0f / m_fScale &&
			fabs(kInstancePosition.y - fRealY) < ((float) CTerrainImpl::CELLSCALE) * 3.0f / m_fScale)
		{
			rReturnName = pkInstEach->GetNameString();
			*pReturnPosX = kInstancePosition.x;
			*pReturnPosY = kInstancePosition.y;
			*pdwTextColor = pkInstEach->GetNameColor();
			return true;
		}
	}
	return false;
}

bool CPythonMiniMap::GetAtlasInfo(float fScreenX, float fScreenY, std::string & rReturnString, float * pReturnPosX, float * pReturnPosY, DWORD * pdwTextColor, DWORD * pdwGuildID)
{
	float fRealX = (fScreenX - m_fAtlasScreenX) * (m_fAtlasMaxX / m_fAtlasImageSizeX);
	float fRealY = (fScreenY - m_fAtlasScreenY) * (m_fAtlasMaxY / m_fAtlasImageSizeY);

	*pReturnPosX = fRealX;
	*pReturnPosY = fRealY;

	//((float) CTerrainImpl::CELLSCALE) * 10.0f
	float fCheckWidth = (m_fAtlasMaxX / m_fAtlasImageSizeX) * 5.0f;
	float fCheckHeight = (m_fAtlasMaxY / m_fAtlasImageSizeY) * 5.0f;

	CInstanceBase * pkInst = CPythonCharacterManager::Instance().GetMainInstancePtr();

	if (pkInst)
	{
		TPixelPosition kInstPos;
		pkInst->NEW_GetPixelPosition(&kInstPos);

		if (kInstPos.x-fCheckWidth<fRealX && kInstPos.x+fCheckWidth>fRealX &&
			kInstPos.y-fCheckHeight<fRealY && kInstPos.y+fCheckHeight>fRealY)
		{
			rReturnString = pkInst->GetNameString();
			*pReturnPosX = kInstPos.x;
			*pReturnPosY = kInstPos.y;
			*pdwTextColor = pkInst->GetNameColor();
			return true;
		}
	}

	if (m_bAtlasRenderNpc)
	{
		for (auto& rAtlasMarkInfo : m_AtlasNPCInfoVector)
		{
			if (rAtlasMarkInfo.m_fX - fCheckWidth / 2 < fRealX && rAtlasMarkInfo.m_fX + fCheckWidth > fRealX &&
					rAtlasMarkInfo.m_fY - fCheckWidth / 2 < fRealY && rAtlasMarkInfo.m_fY + fCheckHeight > fRealY)
			{
				rReturnString = rAtlasMarkInfo.m_strText;
				*pReturnPosX = rAtlasMarkInfo.m_fX;
				*pReturnPosY = rAtlasMarkInfo.m_fY;
				*pdwTextColor = CInstanceBase::GetIndexedNameColor (CInstanceBase::NAMECOLOR_NPC); //m_MarkTypeToColorMap[rAtlasMarkInfo.m_byType];
				return true;
			}
		}
	}

	if (m_bAtlasRenderWarp)
	{
		for (auto& rAtlasMarkInfo : m_AtlasWarpInfoVector)
		{
			if (rAtlasMarkInfo.m_fX - fCheckWidth / 2 < fRealX && rAtlasMarkInfo.m_fX + fCheckWidth > fRealX &&
					rAtlasMarkInfo.m_fY - fCheckWidth / 2 < fRealY && rAtlasMarkInfo.m_fY + fCheckHeight > fRealY)
			{
				rReturnString = rAtlasMarkInfo.m_strText;
				*pReturnPosX = rAtlasMarkInfo.m_fX;
				*pReturnPosY = rAtlasMarkInfo.m_fY;
				*pdwTextColor = CInstanceBase::GetIndexedNameColor (CInstanceBase::NAMECOLOR_WARP); //m_MarkTypeToColorMap[rAtlasMarkInfo.m_byType];
				return true;
			}
		}
	}

	if (m_bAtlasRenderStone)
	{
		for (auto& rAtlasMarkInfo : m_AtlasStoneInfoVector)
		{
			if (rAtlasMarkInfo.m_fX - fCheckWidth / 2 < fRealX && rAtlasMarkInfo.m_fX + fCheckWidth > fRealX &&
					rAtlasMarkInfo.m_fY - fCheckWidth / 2 < fRealY && rAtlasMarkInfo.m_fY + fCheckHeight > fRealY)
			{
				rReturnString = rAtlasMarkInfo.m_strText;
				*pReturnPosX = rAtlasMarkInfo.m_fX;
				*pReturnPosY = rAtlasMarkInfo.m_fY;
				*pdwTextColor = CInstanceBase::GetIndexedNameColor (CInstanceBase::NAMECOLOR_STONE); //m_MarkTypeToColorMap[rAtlasMarkInfo.m_byType];
				return true;
			}
		}
	}

	if (m_bAtlasRenderBoss)
	{
		for (auto& rAtlasMarkInfo : m_AtlasBossInfoVector)
		{
			if (rAtlasMarkInfo.m_fX - fCheckWidth / 2 < fRealX && rAtlasMarkInfo.m_fX + fCheckWidth > fRealX &&
					rAtlasMarkInfo.m_fY - fCheckWidth / 2 < fRealY && rAtlasMarkInfo.m_fY + fCheckHeight > fRealY)
			{
				rReturnString = rAtlasMarkInfo.m_strText;
				*pReturnPosX = rAtlasMarkInfo.m_fX;
				*pReturnPosY = rAtlasMarkInfo.m_fY;
				*pdwTextColor = CInstanceBase::GetIndexedNameColor (CInstanceBase::NAMECOLOR_BOSS); //m_MarkTypeToColorMap[rAtlasMarkInfo.m_byType];
				return true;
			}
		}
	}

	if (m_bAtlasRenderWaypoint)
	{
		for (auto& rAtlasMarkInfo : m_AtlasWayPointInfoVector)
		{
			if (rAtlasMarkInfo.m_fScreenX > 0.0f)
				if (rAtlasMarkInfo.m_fScreenY > 0.0f)
					if (rAtlasMarkInfo.m_fX - fCheckWidth / 2 < fRealX && rAtlasMarkInfo.m_fX + fCheckWidth > fRealX &&
							rAtlasMarkInfo.m_fY - fCheckWidth / 2 < fRealY && rAtlasMarkInfo.m_fY + fCheckHeight > fRealY)
					{
						rReturnString = rAtlasMarkInfo.m_strText;
						*pReturnPosX = rAtlasMarkInfo.m_fX;
						*pReturnPosY = rAtlasMarkInfo.m_fY;
						*pdwTextColor = CInstanceBase::GetIndexedNameColor (CInstanceBase::NAMECOLOR_WAYPOINT); //m_MarkTypeToColorMap[rAtlasMarkInfo.m_byType];
						return true;
					}
		}
	}

	return false;
}

bool CPythonMiniMap::GetAtlasSize(float * pfSizeX, float * pfSizeY)
{
	CPythonBackground& rkBG=CPythonBackground::Instance();
	if (!rkBG.IsMapOutdoor())
		return false;

	*pfSizeX = m_fAtlasImageSizeX;
	*pfSizeY = m_fAtlasImageSizeY;

	return true;
}

// Atlas
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// WayPoint
void CPythonMiniMap::AddWayPoint(BYTE byType, DWORD dwID, float fX, float fY, std::string strText, DWORD dwChrVID)
{
	m_AtlasMarkInfoListIterator = m_AtlasWayPointInfoVector.begin();
	while (m_AtlasMarkInfoListIterator != m_AtlasWayPointInfoVector.end())
	{
		TAtlasMarkInfo& rAtlasMarkInfo = *m_AtlasMarkInfoListIterator;
		if (rAtlasMarkInfo.m_dwID == dwID)
			return;

		++m_AtlasMarkInfoListIterator;
	}

	TAtlasMarkInfo aAtlasMarkInfo;
	memset(&aAtlasMarkInfo, 0, sizeof(aAtlasMarkInfo));
	aAtlasMarkInfo.m_byType = byType;
	aAtlasMarkInfo.m_dwID = dwID;
	aAtlasMarkInfo.m_fX = fX;
	aAtlasMarkInfo.m_fY = fY;
	aAtlasMarkInfo.m_fScreenX = 0.0f;
	aAtlasMarkInfo.m_fScreenY = 0.0f;
	aAtlasMarkInfo.m_fMiniMapX = 0.0f;
	aAtlasMarkInfo.m_fMiniMapY = 0.0f;
	aAtlasMarkInfo.m_strText = strText;
	aAtlasMarkInfo.m_dwChrVID = dwChrVID;
	__UpdateWayPoint(&aAtlasMarkInfo, fX, fY);
	m_AtlasWayPointInfoVector.push_back(aAtlasMarkInfo);

}

void CPythonMiniMap::RemoveWayPoint(DWORD dwID)
{
	m_AtlasMarkInfoListIterator = m_AtlasWayPointInfoVector.begin();
	while (m_AtlasMarkInfoListIterator != m_AtlasWayPointInfoVector.end())
	{
		TAtlasMarkInfo& rAtlasMarkInfo = *m_AtlasMarkInfoListIterator;
		if (rAtlasMarkInfo.m_dwID == dwID)
		{
			m_AtlasMarkInfoListIterator = m_AtlasWayPointInfoVector.erase (m_AtlasMarkInfoListIterator);
			return;
		}
		++m_AtlasMarkInfoListIterator;
	}
}

bool CPythonMiniMap::__GetWayPoint(DWORD dwID, TAtlasMarkInfo ** ppkInfo)
{
	TAtlasMarkInfoVectorIterator itor = m_AtlasWayPointInfoVector.begin();
	for (; itor != m_AtlasWayPointInfoVector.end(); ++itor)
	{
		TAtlasMarkInfo & rInfo = *itor;
		if (dwID == rInfo.m_dwID)
		{
			*ppkInfo = &rInfo;
			return true;
		}
	}

	return false;
}

void CPythonMiniMap::__UpdateWayPoint(TAtlasMarkInfo * pkInfo, int ix, int iy)
{
	pkInfo->m_fX = float(ix);
	pkInfo->m_fY = float(iy);
	pkInfo->m_fScreenX = pkInfo->m_fX / m_fAtlasMaxX * m_fAtlasImageSizeX;
	pkInfo->m_fScreenY = pkInfo->m_fY / m_fAtlasMaxY * m_fAtlasImageSizeY;
}

// WayPoint
//////////////////////////////////////////////////////////////////////////

void CPythonMiniMap::__RenderWayPointMark(int ixCenter, int iyCenter)
{
	int iNum = (ELTimer_GetMSec() / 67) % WAYPOINT_IMAGE_COUNT;

	CGraphicImageInstance & rInstance = m_WayPointGraphicImageInstances[iNum];
	rInstance.SetPosition(ixCenter - rInstance.GetWidth()/2, iyCenter - rInstance.GetHeight()/2);
	rInstance.Render();
}

void CPythonMiniMap::__RenderMiniWayPointMark(int ixCenter, int iyCenter)
{
	int iNum = (ELTimer_GetMSec() / 67) % MINI_WAYPOINT_IMAGE_COUNT;

	CGraphicImageInstance & rInstance = m_MiniWayPointGraphicImageInstances[iNum];
	rInstance.SetPosition(ixCenter - rInstance.GetWidth()/2, iyCenter - rInstance.GetHeight()/2);
	rInstance.Render();
}

void CPythonMiniMap::__RenderTargetMark(int ixCenter, int iyCenter)
{
	int iNum = (ELTimer_GetMSec() / 80) % TARGET_MARK_IMAGE_COUNT;

	CGraphicImageInstance & rInstance = m_TargetMarkGraphicImageInstances[iNum];
	rInstance.SetPosition(ixCenter - rInstance.GetWidth()/2, iyCenter - rInstance.GetHeight()/2);
	rInstance.Render();
}

void CPythonMiniMap::AddSignalPoint(float fX, float fY)
{
	static unsigned int g_id = 255;

	TSignalPoint sp;
	sp.id = g_id;
	sp.v2Pos.x = fX;
	sp.v2Pos.y = fY;

	m_SignalPointVector.push_back(sp);

	AddWayPoint(TYPE_WAYPOINT, g_id, fX, fY, "");

	g_id++;
}

void CPythonMiniMap::ClearAllSignalPoint()
{
	vector<TSignalPoint>::iterator it;
	for(it = m_SignalPointVector.begin();it!=m_SignalPointVector.end();++it)
		RemoveWayPoint(it->id);

	m_SignalPointVector.clear();
}

void CPythonMiniMap::RegisterAtlasWindow(PyObject* poHandler)
{
	m_poHandler = poHandler;
}

void CPythonMiniMap::UnregisterAtlasWindow()
{
	m_poHandler = 0;
}

void CPythonMiniMap::OpenAtlasWindow()
{
	if (m_poHandler)
		PyCallClassMemberFunc(m_poHandler,"Show", Py_BuildValue("()"));
}

void CPythonMiniMap::SetAtlasCenterPosition(int x, int y)
{
	if (m_poHandler)
	{
		PyCallClassMemberFunc(m_poHandler,"SetCenterPositionAdjust", Py_BuildValue("(ii)",x,y));
	}
}

bool CPythonMiniMap::IsAtlas()
{
	return m_bAtlas;
}

void CPythonMiniMap::ShowAtlas()
{
	m_bShowAtlas=true;
}

void CPythonMiniMap::HideAtlas()
{
	m_bShowAtlas=false;
}

bool CPythonMiniMap::CanShowAtlas()
{
	return m_bShowAtlas;
}

bool CPythonMiniMap::CanShow()
{
	return m_bShow;
}

void CPythonMiniMap::Show()
{
	m_bShow=true;
}

void CPythonMiniMap::Hide()
{
	m_bShow=false;
}

#ifdef ENABLE_ATLAS_RENDER_OPTION
bool CPythonMiniMap::ToggleAtlasMarker(int type)
{
	switch (type)
	{
		case CPythonMiniMap::MARKER_NPC:
			return m_bAtlasRenderNpc = !m_bAtlasRenderNpc;
		case CPythonMiniMap::MARKER_WARP:
			return m_bAtlasRenderWarp = !m_bAtlasRenderWarp;
		case CPythonMiniMap::MARKER_WAYPOINT:
			return m_bAtlasRenderWaypoint = !m_bAtlasRenderWaypoint;
		case CPythonMiniMap::MARKER_STONE:
			return m_bAtlasRenderStone = !m_bAtlasRenderStone;
		case CPythonMiniMap::MARKER_BOSS:
			return m_bAtlasRenderBoss = !m_bAtlasRenderBoss;
	}

	return false;
}

bool CPythonMiniMap::IsCheckedAtlasMarker(int type)
{
	switch (type)
	{
		case CPythonMiniMap::MARKER_NPC:
			return m_bAtlasRenderNpc;
		case CPythonMiniMap::MARKER_WARP:
			return m_bAtlasRenderWaypoint;
		case CPythonMiniMap::MARKER_WAYPOINT:
			return m_bAtlasRenderWarp;
		case CPythonMiniMap::MARKER_STONE:
			return m_bAtlasRenderStone;
		case CPythonMiniMap::MARKER_BOSS:
			return m_bAtlasRenderBoss;
	}

	return false;
}
#endif

void CPythonMiniMap::__Initialize()
{
	m_poHandler = 0;

	SetMiniMapSize(128.0f, 128.0f);

	m_fScale = 2.0f;

	m_fCenterX = m_fWidth * 0.5f;
	m_fCenterY = m_fHeight * 0.5f;

	m_fScreenX = 0.0f;
	m_fScreenY = 0.0f;

	m_fAtlasScreenX = 0.0f;
	m_fAtlasScreenY = 0.0f;

	m_dwAtlasBaseX = 0;
	m_dwAtlasBaseY = 0;

	m_fAtlasMaxX = 0.0f;
	m_fAtlasMaxY = 0.0f;

	m_fAtlasImageSizeX = 0.0f;
	m_fAtlasImageSizeY = 0.0f;

	m_bAtlas = false;

	m_bShow = false;
	m_bShowAtlas = false;

	m_bAtlasRenderNpc = true;
	m_bAtlasRenderWarp = true;
	m_bAtlasRenderWaypoint = true;
	m_bAtlasRenderStone = true;
	m_bAtlasRenderBoss = true;

	D3DXMatrixIdentity(&m_matIdentity);
	D3DXMatrixIdentity(&m_matWorld);
	D3DXMatrixIdentity(&m_matMiniMapCover);
	D3DXMatrixIdentity(&m_matWorldAtlas);
}

void CPythonMiniMap::Destroy()
{
	ClearAllSignalPoint();
	m_poHandler = 0;

	m_VertexBuffer.Destroy();
	m_IndexBuffer.Destroy();

	m_PlayerMark.Destroy();

	m_MiniMapFilterGraphicImageInstance.Destroy();
	m_MiniMapCameraraphicImageInstance.Destroy();

	m_AtlasWayPointInfoVector.clear();
	m_AtlasImageInstance.Destroy();
	m_AtlasPlayerMark.Destroy();
	m_WhiteMark.Destroy();

	for (int i = 0; i < MINI_WAYPOINT_IMAGE_COUNT; ++i)
		m_MiniWayPointGraphicImageInstances[i].Destroy();
	for (int j = 0; j < WAYPOINT_IMAGE_COUNT; ++j)
		m_WayPointGraphicImageInstances[j].Destroy();
	for (int k = 0; k < TARGET_MARK_IMAGE_COUNT; ++k)
		m_TargetMarkGraphicImageInstances[k].Destroy();

	__Initialize();
}

CPythonMiniMap::CPythonMiniMap()
{
	__Initialize();
}

CPythonMiniMap::~CPythonMiniMap()
{
	Destroy();
}
