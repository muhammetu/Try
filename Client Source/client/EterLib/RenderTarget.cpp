#include "StdAfx.h"
#ifdef ENABLE_MODEL_RENDER_TARGET
#include "RenderTargetManager.h"
#include "ResourceManager.h"
#include "RenderTarget.h"

#include "../EterLib/Camera.h"
#include "../EterPythonLib/PythonGraphic.h"
#include "../EterBase/CRC32.h"
#include "../GameLib/GameType.h"
#include "../GameLib/MapType.h"
#include "../GameLib/ItemData.h"
#include "../GameLib/ActorInstance.h"
#include "../GameLib/RaceMotionData.h"
#include "../GameLib/ItemManager.h"
#include "../UserInterface/InstanceBase.h"
#include "../UserInterface/PythonSystem.h"

#include <algorithm>

CRenderTarget::CRenderTarget(const DWORD width, const DWORD height) : 
	m_pModel(nullptr),
	m_background(nullptr),
	m_modelRotation(0),
	m_visible(false),
	m_isMenu(false),
	m_isWiki(false)
{
	auto pTex = new CGraphicRenderTargetTexture;
	if (!pTex->Create(width, height, D3DFMT_X8R8G8B8, D3DFMT_D16)) 
	{
		delete pTex;
		TraceError("CRenderTarget::CRenderTarget: Could not create CGraphicRenderTargetTexture %dx%d", width, height);
		throw std::runtime_error("CRenderTarget::CRenderTarget: Could not create CGraphicRenderTargetTexture");
	}

	m_renderTargetTexture = std::unique_ptr<CGraphicRenderTargetTexture>(pTex); // c++2a mi kullaniyoz yarragim burasi std::unique_ptr
}

CRenderTarget::~CRenderTarget() // Destruction
{
}

/***
 * @@ UI Functions
 * 
 * 
 * 
 * 
 * **/

void CRenderTarget::SetVisibility(bool isShow) 
{
	m_visible = isShow;
}

#undef min
void CRenderTarget::ScaleToFitInViewport(int width, int height) const
{
	if (!m_pModel || !m_visible)
		return;

	D3DXVECTOR3 centerPos = {};
	float radius = 0.0f;

	auto& CameraMgr = CCameraManager::instance();

	float distance = CameraMgr.GetRenderTargetCamera()->GetDistance();

	m_pModel->GetGraphicThingInstanceRef().GetBoundingSphere(centerPos, radius);

	float modelheight = m_pModel->GetGraphicThingInstanceRef().GetHeight();
	float yScaleFactor = std::min(1.0f, static_cast<float>(height) / modelheight);
	float xScaleFactor = std::min(yScaleFactor, static_cast<float>(width) / radius);
	float zScaleFactor = xScaleFactor;

	m_vScale.x = xScaleFactor;
	m_vScale.y = yScaleFactor;
	m_vScale.z = zScaleFactor;

#ifdef ENABLE_ACCE_SYSTEM
	if (m_pModel->IsPC())
	{
		BYTE bRace = (BYTE)m_pModel->GetRace();
		BYTE bJob = (BYTE)RaceToJob(bRace);
		BYTE bSex = (BYTE)RaceToSex(bRace);

		CItemData* pItemData = NULL;
		CItemManager::Instance().GetItemDataPointer(m_pModel->GetPart(CRaceData::PART_ACCE), &pItemData);

		if (pItemData)
		{
			m_pModel->GetGraphicThingInstancePtr()->SetAcceScale(pItemData->GetItemScaleVector(bJob, bSex), bRace);
			return;
		}
	}
#endif

	m_pModel->GetGraphicThingInstancePtr()->SetScale(xScaleFactor, yScaleFactor, zScaleFactor);
}

void CRenderTarget::SetMenu(bool f)
{
	m_isMenu = f;
}

void CRenderTarget::SetWiki(bool f)
{
	m_isWiki = f;
}

/***
 * @@ Model Functions
 * 
 * 
 * 
 * 
 * **/

void CRenderTarget::ChangeGuild(DWORD guildID)
{
	if (!m_visible || !m_pModel)
		return;

	// @@
	// sadece oyuncularda isleyecek.
	if (!m_pModel->IsPC())
		return;

	m_pModel->ChangeGuild(guildID);
}

void CRenderTarget::SetArmor(DWORD armor)
{
	if (!m_visible || !m_pModel)
		return;

	// @@
	// sadece oyuncularda isleyecek.
	if (!m_pModel->IsPC())
		return;

#if defined(ENABLE_ITEM_EVOLUTION_SYSTEM) && defined(ENABLE_ARMOR_EVOLUTION_SYSTEM)
	m_pModel->ChangeArmor(armor, 0);
#else
	m_pModel->ChangeArmor(armor);
#endif
}

void CRenderTarget::SetWeapon(DWORD weapon)
{
	if (!m_visible || !m_pModel)
		return;

	// @@
	// sadece oyuncularda isleyecek.
	if (!m_pModel->IsPC())
		return;

	m_pModel->ChangeWeapon(weapon);
}

void CRenderTarget::SetHair(DWORD hair)
{
	if (!m_visible || !m_pModel)
		return;

	// @@
	// sadece oyuncularda isleyecek.
	if (!m_pModel->IsPC())
		return;

	m_pModel->SetHair(hair);
	m_pModel->RefreshState(CRaceMotionData::NAME_WAIT, true);
}

#ifdef ENABLE_ACCE_SYSTEM
void CRenderTarget::SetAcce(DWORD acce)
{
	if (!m_visible || !m_pModel)
		return;

	// @@
	// sadece oyuncularda isleyecek.
	if (!m_pModel->IsPC())
		return;

	m_pModel->ChangeAcce(acce, 95.0f);
}

void CRenderTarget::SetShining(DWORD Index, DWORD shining)
{
	if (!m_visible || !m_pModel)
		return;

	// @@
	// sadece oyuncularda isleyecek.
	if (!m_pModel->IsPC())
		return;

	m_pModel->SetShining(Index, shining);
}
#endif

#ifdef ENABLE_AURA_COSTUME_SYSTEM
void CRenderTarget::SetAura(DWORD aura)
{
	if (!m_visible || !m_pModel)
		return;

	// @@
	// sadece oyuncularda isleyecek.
	if (!m_pModel->IsPC())
		return;

	m_pModel->ChangeAura(aura);
}
#endif

void CRenderTarget::SetLevel(DWORD level)
{
	if (!m_visible || !m_pModel)
		return;

	m_pModel->SetLevel(level);
}

void CRenderTarget::SetAlignment(DWORD alignment)
{
	if (!m_visible || !m_pModel)
		return;

	// @@
	// sadece oyuncularda isleyecek.
	if (!m_pModel->IsPC())
		return;

	m_pModel->SetAlignment(alignment);
}

void CRenderTarget::SetPKMode(BYTE pkMode)
{
	if (!m_visible || !m_pModel)
		return;

	// @@
	// sadece oyuncularda isleyecek.
	if (!m_pModel->IsPC())
		return;

	m_pModel->SetPKMode(pkMode);
}

void CRenderTarget::SelectModel(DWORD index)
{
	CInstanceBase::SCreateData kCreateData{};

	// @@ eger wolfman mevcut ise 8 -> 9 yapilacak.
#ifdef ENABLE_WOLFMAN_CHARACTER
	kCreateData.m_bType = index < 9 ? CActorInstance::TYPE_PC : CActorInstance::TYPE_NPC;
#else
	kCreateData.m_bType = index < 8 ? CActorInstance::TYPE_PC : CActorInstance::TYPE_NPC;
#endif
	kCreateData.m_dwRace = index;
	// this is not our character...
	kCreateData.m_isMain = false;

	auto model = std::make_unique<CInstanceBase>();
	if (!model->Create(kCreateData))
	{
		if (m_pModel)
			m_pModel.reset();
		return;
	}

	m_pModel = std::move(model);

	// set pixel position to 0x0x0
	m_pModel->NEW_SetPixelPosition(TPixelPosition(0, 0, 0));

	// clear effects
	m_pModel->GetGraphicThingInstancePtr()->ClearAttachingEffect();

	// model rotation set 0.0f
	SetModelRotation(0.0f);

	// refresh model anim
	m_pModel->Refresh(CRaceMotionData::NAME_WAIT, true);

	// set model anim loop
	m_pModel->SetLoopMotion(CRaceMotionData::NAME_WAIT);

	// set model always render
	m_pModel->SetAlwaysRender(true);

	// set model rotation
	m_pModel->SetRotation(0.0f);

	m_pModel->GetGraphicThingInstancePtr()->SetRenderTarget(true);

	// Camera Part
	auto& CameraMgr = CCameraManager::instance();
	CameraMgr.SetCurrentCamera(CCameraManager::DEFAULT_MODEL_RENDER_CAMERA);
	CameraMgr.GetCurrentCamera()->SetTargetHeight(110.0);
	CameraMgr.ResetToPreviousCamera();
}

void CRenderTarget::SetScale(float x, float y, float z)
{
	if (!m_pModel)
		return;

	m_pModel->GetGraphicThingInstancePtr()->SetScaleWorld(x, y, z);
}

const D3DXVECTOR3& CRenderTarget::GetScale()
{
	return m_pModel->GetGraphicThingInstancePtr()->GetScale();
}

/***
 * @@ Rendering Arguments
 * 
 * 
 * 
 * **/

bool CRenderTarget::CreateBackground(const char* imgPath, const DWORD width, const DWORD height)
{
	// background is empty ?
	if (m_background)
		return false;

	m_background = std::make_unique<CGraphicImageInstance>();
	
	const auto graphic_image = dynamic_cast<CGraphicImage*>(CResourceManager::instance().GetResourcePointer(imgPath));
	if (!graphic_image)
	{
		m_background.reset();
		return false;
	}

	m_background->SetImagePointer(graphic_image);
	m_background->SetScale(static_cast<float>(width) / graphic_image->GetWidth(), static_cast<float>(height) / graphic_image->GetHeight());
	return true;
}

void CRenderTarget::CreateTextures() const
{
	m_renderTargetTexture->CreateTextures();
}

void CRenderTarget::ReleaseTextures() const
{
	m_renderTargetTexture->ReleaseTextures();
}

void CRenderTarget::SetRenderingRect(RECT* rect) const
{
	m_renderTargetTexture->SetRenderingRect(rect);
}

/***
 * @@ Rendering Functions
 * 
 * 
 * 
 * 
 * **/
void CRenderTarget::RenderTexture() const
{
	m_renderTargetTexture->Render();
}

void CRenderTarget::RenderModel() const
{
	if (!m_visible || !m_pModel)
		return;

	auto& pyGraphic = CPythonGraphic::Instance();
	auto& CameraMgr = CCameraManager::instance();
	auto& StateMgr = CStateManager::Instance();
	auto& RectRender = *m_renderTargetTexture->GetRenderingRect();

	m_renderTargetTexture->SetRenderTarget();

	if (!m_background)
		m_renderTargetTexture->Clear();

	pyGraphic.ClearDepthBuffer();

	const float tmp_fov = pyGraphic.GetFOV();
	const float tmp_aspect = pyGraphic.GetAspect();
	const float tmp_near_y = pyGraphic.GetNear();
	const float tmp_far_y = pyGraphic.GetFar();

	const auto width = static_cast<float>(RectRender.right - RectRender.left);
	const auto height = static_cast<float>(RectRender.bottom - RectRender.top);

	// we don't need fog
	StateMgr.SetRenderState(D3DRS_FOGENABLE, FALSE);

	pyGraphic.SetViewport(0.0f, 0.0f, width, height);
	pyGraphic.PushState();

	CameraMgr.SetCurrentCamera(CCameraManager::DEFAULT_MODEL_RENDER_CAMERA);

	if (m_isMenu)
	{
		CameraMgr.GetCurrentCamera()->SetViewParams(D3DXVECTOR3{ 0.0f, -1500.0f, 600.0f }, D3DXVECTOR3{ 0.0f, 0.0f, 160.0f }, D3DXVECTOR3{0.0f, 0.0f, 1.0f});
		pyGraphic.UpdateViewMatrix();
		pyGraphic.SetPerspective(10.0f, width - height, 100.0f, 3000.0f);
		CameraMgr.GetCurrentCamera()->SetTargetHeight(110.0f);
		pyGraphic.SetOrtho3D(width, height, 0, 5000);
	}
	else if (m_isWiki)
	{
		CameraMgr.GetCurrentCamera()->SetViewParams(D3DXVECTOR3{ 0.0f, -1500.0f, 600.0f }, D3DXVECTOR3{ 0.0f, 0.0f, 95.0f }, D3DXVECTOR3{0.0f, 0.0f, 1.0f});
		pyGraphic.UpdateViewMatrix();
		pyGraphic.SetPerspective(10.0f, width / height, 100.0f, 3000.0f);
		CameraMgr.GetCurrentCamera()->SetTargetHeight(110.0f);
		pyGraphic.SetOrtho3D(width, height, 0, 5000);
	}
	else
	{
		CameraMgr.GetCurrentCamera()->SetViewParams(D3DXVECTOR3{ 0.0f, -1500.0f, 600.0f }, D3DXVECTOR3{ 0.0f, 0.0f, 95.0f }, D3DXVECTOR3{0.0f, 0.0f, 1.0f});
		pyGraphic.UpdateViewMatrix();
		pyGraphic.SetPerspective(10.0f, width - height, 100.0f, 3000.0f);
		CameraMgr.GetCurrentCamera()->SetTargetHeight(110.0f);
		pyGraphic.SetOrtho3D(width, height, 0, 5000);
	}

	if (!m_isWiki)
		ScaleToFitInViewport(width, height);

	m_pModel->Render();
#ifdef USE_MODEL_RENDER_TARGET_EFFECT
	m_pModel->GetGraphicThingInstanceRef().RenderAllAttachingEffect();
#endif

	CameraMgr.ResetToPreviousCamera();

	pyGraphic.RestoreViewport();
	pyGraphic.PopState();
	pyGraphic.SetPerspective(tmp_fov, tmp_aspect, tmp_near_y, tmp_far_y);

	m_renderTargetTexture->ResetRenderTarget();

	// set fog to original value
	StateMgr.SetRenderState(D3DRS_FOGENABLE, CPythonSystem::Instance().IsFogMode());
}

void CRenderTarget::RenderBackground() const
{
	if (!m_visible || !m_background)
		return;

	auto& rectRender = *m_renderTargetTexture->GetRenderingRect();

	m_renderTargetTexture->SetRenderTarget();

	CGraphicRenderTargetTexture::Clear();

	CPythonGraphic::Instance().SetInterfaceRenderState();

	m_background->Render();

	m_renderTargetTexture->ResetRenderTarget();
}

void CRenderTarget::UpdateModel()
{
	if (!m_visible || !m_pModel)
		return;

	if (m_modelRotation < 360.0f)
		m_modelRotation += 1.0f;
	else
		m_modelRotation = 0.0f;

	m_pModel->SetRotation(m_modelRotation);
	m_pModel->Transform();
	m_pModel->GetGraphicThingInstanceRef().RotationProcess();
}

void CRenderTarget::DeformModel() const
{
	if (!m_visible || !m_pModel)
		return;

	m_pModel->Deform();
}
#endif