#include "StdAfx.h"
#include "../eterBase/Error.h"
#include "../eterlib/Camera.h"
#include "../eterlib/AttributeInstance.h"
#include "../gamelib/AreaTerrain.h"
#include "../EterGrnLib/Material.h"
#include "../CWebBrowser/CWebBrowser.h"

#include "resource.h"
#include "PythonApplication.h"
#include "PythonCharacterManager.h"

#ifdef ENABLE_SWITCHBOT_SYSTEM
#include "PythonSwitchbot.h"
#endif

extern void GrannyCreateSharedDeformBuffer();
extern void GrannyDestroySharedDeformBuffer();

float MIN_FOG = 2400.0f;
double g_specularSpd=0.007f;

CPythonApplication* CPythonApplication::ms_pInstance;

float c_fDefaultCameraRotateSpeed = 1.5f;
float c_fDefaultCameraPitchSpeed = 1.5f;
float c_fDefaultCameraZoomSpeed = 0.05f;

CPythonApplication::CPythonApplication() :
m_bCursorVisible(TRUE),
m_bLiarCursorOn(false),
m_iCursorMode(CURSOR_MODE_HARDWARE),
m_isWindowed(false),
m_isFrameSkipDisable(false),
m_poMouseHandler(NULL),
m_dwUpdateFPS(0),
m_dwRenderFPS(0),
m_fAveRenderTime(0.0f),
m_dwFaceCount(0),
m_fGlobalTime(0.0f),
m_fGlobalElapsedTime(0.0f),
m_dwLButtonDownTime(0),
m_dwLastIdleTime(0)
{
#if !defined(_DEBUG) && !defined(LIVE_SERVER)
	SetEterExceptionHandler();
#endif

	CTimer::Instance().UseCustomTime();
	m_dwWidth = 800;
	m_dwHeight = 600;

	ms_pInstance = this;
	m_isWindowFullScreenEnable = FALSE;

	m_v3CenterPosition = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	m_dwStartLocalTime = ELTimer_GetMSec();
	m_tServerTime = 0;
	m_tLocalStartTime = 0;

	m_iPort = 0;
	m_iFPS = 60;

	m_isActivateWnd = false;
	m_isMinimizedWnd = true;

	m_fRotationSpeed = 0.0f;
	m_fPitchSpeed = 0.0f;
	m_fZoomSpeed = 0.0f;

	m_fFaceSpd = 0.0f;

	m_dwFaceAccCount = 0;
	m_dwFaceAccTime = 0;

	m_dwFaceSpdSum = 0;
	m_dwFaceSpdCount = 0;

	m_FlyingManager.SetMapManagerPtr(&m_pyBackground);

	m_iCursorNum = CURSOR_SHAPE_NORMAL;
	m_iContinuousCursorNum = CURSOR_SHAPE_NORMAL;

	m_isSpecialCameraMode = FALSE;
	m_fCameraRotateSpeed = c_fDefaultCameraRotateSpeed;
	m_fCameraPitchSpeed = c_fDefaultCameraPitchSpeed;
	m_fCameraZoomSpeed = c_fDefaultCameraZoomSpeed;

	m_iCameraMode = CAMERA_MODE_NORMAL;
	m_fBlendCameraStartTime = 0.0f;
	m_fBlendCameraBlendTime = 0.0f;

	m_iForceSightRange = -1;

	CCameraManager::Instance().AddCamera(EVENT_CAMERA_NUMBER);
}

CPythonApplication::~CPythonApplication()
{
}

void CPythonApplication::GetMousePosition(POINT* ppt)
{
	CMSApplication::GetMousePosition(ppt);
}

void CPythonApplication::SetMinFog(float fMinFog)
{
	MIN_FOG = fMinFog;
}

void CPythonApplication::SetFrameSkip(bool isEnable)
{
	if (isEnable)
		m_isFrameSkipDisable = false;
	else
		m_isFrameSkipDisable = true;
}

void CPythonApplication::NotifyHack(const char* c_szFormat, ...)
{
	char szBuf[1024];

	va_list args;
	va_start(args, c_szFormat);
	_vsnprintf(szBuf, sizeof(szBuf), c_szFormat, args);
	va_end(args);
	m_pyNetworkStream.NotifyHack(szBuf);
}

void CPythonApplication::GetInfo(UINT eInfo, std::string* pstInfo)
{
	switch (eInfo)
	{
	case INFO_ACTOR:
		m_kChrMgr.GetInfo(pstInfo);
		break;
	case INFO_EFFECT:
		m_kEftMgr.GetInfo(pstInfo);
		break;
	case INFO_ITEM:
		m_pyItem.GetInfo(pstInfo);
		break;
	case INFO_TEXTTAIL:
		m_pyTextTail.GetInfo(pstInfo);
		break;
	}
}

void CPythonApplication::Abort()
{
	TraceError("============================================================================================================");
	TraceError("Abort!!!!\n\n");

	PostQuitMessage(0);
}

void CPythonApplication::Exit()
{
	PostQuitMessage(0);
}

void CPythonApplication::RenderGame()
{
#ifdef ENABLE_MODEL_RENDER_TARGET
	m_kRenderTargetMgr.RenderBackgrounds();
#endif
	float fAspect = m_kWndMgr.GetAspect();
	float fFarClip = m_pyBackground.GetFarClip();

#ifdef ENABLE_FOV_OPTION
	float fFOV = m_pySystem.GetFOVLevel();
	m_pyGraphic.SetPerspective(fFOV, fAspect, 100.0, fFarClip);
#else
	m_pyGraphic.SetPerspective(30.0f, fAspect, 100.0, fFarClip);
#endif

	CCullingManager::Instance().Process();

	m_kChrMgr.Deform();
#ifdef ENABLE_MODEL_RENDER_TARGET
	m_kRenderTargetMgr.DeformModels();
#endif
	m_pyBackground.RenderCharacterShadowToTexture();

	m_pyGraphic.SetGameRenderState();
	m_pyGraphic.PushState();

	{
		long lx, ly;
		m_kWndMgr.GetMousePosition(lx, ly);
		m_pyGraphic.SetCursorPosition(lx, ly);
	}

	m_pyBackground.RenderSky();

	m_pyBackground.RenderBeforeLensFlare();

	m_pyBackground.RenderCloud();

	m_pyBackground.BeginEnvironment();
	m_pyBackground.Render();

	m_pyBackground.SetCharacterDirLight();
	m_kChrMgr.Render();
#ifdef ENABLE_MODEL_RENDER_TARGET
	m_kRenderTargetMgr.RenderModels();
#endif

	m_pyBackground.SetBackgroundDirLight();
	m_pyBackground.RenderWater();
	m_pyBackground.RenderSnow();
	m_pyBackground.RenderEffect();

	m_pyBackground.EndEnvironment();

	m_kEftMgr.Render();

	m_pyItem.Render();
#if defined(ENABLE_OFFLINE_SHOP_CITIES)
	m_pyOfflineshop.RenderEntities();
#endif
	m_FlyingManager.Render();

	m_pyBackground.BeginEnvironment();
	m_pyBackground.RenderPCBlocker();
	m_pyBackground.EndEnvironment();

	m_pyBackground.RenderAfterLensFlare();
}

void CPythonApplication::UpdateGame()
{
	POINT ptMouse;
	GetMousePosition(&ptMouse);

	CGraphicTextInstance::Hyperlink_UpdateMousePos(ptMouse.x, ptMouse.y);

	//if (m_isActivateWnd)
	{
		CScreen s;
		float fAspect = UI::CWindowManager::Instance().GetAspect();
		float fFarClip = CPythonBackground::Instance().GetFarClip();


#ifdef ENABLE_FOV_OPTION
		float fFOV = CPythonSystem::Instance().GetFOVLevel();
		s.SetPerspective(fFOV, fAspect, 100.0f, fFarClip);
#else
		s.SetPerspective(30.0f,fAspect, 100.0f, fFarClip);
#endif

#ifdef ENABLE_GRAPHIC_OPTIMIZATION
		s.BuildViewFrustumAll();
#else
		s.BuildViewFrustum();
#endif
	}

#ifdef ENABLE_MODEL_RENDER_TARGET
	m_kRenderTargetMgr.UpdateModels();
#endif

	TPixelPosition kPPosMainActor;
	m_pyPlayer.NEW_GetMainActorPosition(&kPPosMainActor);

	m_pyBackground.Update(kPPosMainActor.x, kPPosMainActor.y, kPPosMainActor.z);

	m_GameEventManager.SetCenterPosition(kPPosMainActor.x, kPPosMainActor.y, kPPosMainActor.z);
	m_GameEventManager.Update();

	m_kChrMgr.Update();
	m_kEftMgr.Update();
	m_kEftMgr.UpdateSound();
	m_FlyingManager.Update();
	m_pyItem.Update(ptMouse);
	m_pyPlayer.Update();

	m_pyPlayer.NEW_GetMainActorPosition(&kPPosMainActor);
	SetCenterPosition(kPPosMainActor.x, kPPosMainActor.y, kPPosMainActor.z);

#ifdef ENABLE_DOG_MODE
	if (CPythonSystem::Instance().GetDogMode())
		m_kChrMgr.SetDogMode(CPythonSystem::Instance().GetDogMode());
#endif

}

void CPythonApplication::SkipRenderBuffering(DWORD dwSleepMSec)
{
	m_dwBufSleepSkipTime = ELTimer_GetMSec() + dwSleepMSec;
}

bool CPythonApplication::Process()
{
	ELTimer_SetFrameMSec();

	DWORD dwStart = ELTimer_GetMSec();

	///////////////////////////////////////////////////////////////////////////////////////////////////
	static DWORD	s_dwUpdateFrameCount = 0;
	static DWORD	s_dwRenderFrameCount = 0;
	static DWORD	s_dwFaceCount = 0;
	static UINT		s_uiLoad = 0;
	static DWORD	s_dwCheckTime = ELTimer_GetMSec();

	if (ELTimer_GetMSec() - s_dwCheckTime > 1000)
	{
		m_dwUpdateFPS = s_dwUpdateFrameCount;
		m_dwRenderFPS = s_dwRenderFrameCount;
		m_dwLoad = s_uiLoad;

		m_dwFaceCount		= s_dwFaceCount / max(1, s_dwRenderFrameCount);

		s_dwCheckTime = ELTimer_GetMSec();

		s_uiLoad = s_dwFaceCount = s_dwUpdateFrameCount = s_dwRenderFrameCount = 0;
	}

	// Update Time
	static BOOL s_bFrameSkip = false;
	static UINT s_uiNextFrameTime = ELTimer_GetMSec();

	CTimer& rkTimer = CTimer::Instance();
	rkTimer.Advance();

	m_fGlobalTime = rkTimer.GetCurrentSecond();
	m_fGlobalElapsedTime = rkTimer.GetElapsedSecond();

	UINT uiFrameTime = rkTimer.GetElapsedMilliecond();
	s_uiNextFrameTime += uiFrameTime;

	DWORD updatestart = ELTimer_GetMSec();
	// Network I/O
	m_pyNetworkStream.Process();

	m_kGuildMarkUploader.Process();

	m_kGuildMarkDownloader.Process();
	m_kAccountConnector.Process();

	//////////////////////
	// Input Process
	// Keyboard
	UpdateKeyboard();
	// Mouse
	POINT Point;
	if (GetCursorPos(&Point))
	{
		ScreenToClient(m_hWnd, &Point);
		OnMouseMove(Point.x, Point.y);
	}
	//////////////////////
	//if (m_isActivateWnd)
	__UpdateCamera();
	// Update Game Playing
	CResourceManager::Instance().Update();
	OnCameraUpdate();
	OnMouseUpdate();
	OnUIUpdate();
#ifdef ENABLE_DISCORD_UTILITY
	m_kDiscordManager.OnUpdate();
#endif

	m_dwCurUpdateTime = ELTimer_GetMSec() - updatestart;

	DWORD dwCurrentTime = ELTimer_GetMSec();
	BOOL  bCurrentLateUpdate = FALSE;

	s_bFrameSkip = false;

	if (dwCurrentTime > s_uiNextFrameTime)
	{
		int dt = dwCurrentTime - s_uiNextFrameTime;
		int nAdjustTime = ((float)dt / (float)uiFrameTime) * uiFrameTime;

		if (dt >= 500)
		{
			s_uiNextFrameTime += nAdjustTime;
			printf("FrameSkip Adjusting... %d\n", nAdjustTime);
			CTimer::Instance().Adjust(nAdjustTime);
		}

		s_bFrameSkip = true;
		bCurrentLateUpdate = TRUE;
	}

	if (m_isFrameSkipDisable)
		s_bFrameSkip = false;

	if (!s_bFrameSkip)
	{
		//		static double pos=0.0f;
		//		CGrannyMaterial::TranslateSpecularMatrix(fabs(sin(pos)*0.005), fabs(cos(pos)*0.005), 0.0f);
		//		pos+=0.01f;

		CGrannyMaterial::TranslateSpecularMatrix(g_specularSpd, g_specularSpd, 0.0f);

		DWORD dwRenderStartTime = ELTimer_GetMSec();

		bool canRender = true;

		if (m_isMinimizedWnd)
		{
			canRender = false;
		}
		else
		{
#ifdef ENABLE_FIX_MOBS_LAG
			if (DEVICE_STATE_OK != CheckDeviceState())
#else
			if (m_pyGraphic.IsLostDevice())
#endif
			{
				CPythonBackground& rkBG = CPythonBackground::Instance();
#ifdef ENABLE_FIX_MOBS_LAG
				canRender = false;
#else
				rkBG.ReleaseCharacterShadowTexture();
#endif
				
#ifdef ENABLE_MODEL_RENDER_TARGET
				CRenderTargetManager::Instance().ReleaseRenderTargetTextures();
#endif
				if (m_pyGraphic.RestoreDevice())
#ifdef ENABLE_MODEL_RENDER_TARGET
				{
					CRenderTargetManager::Instance().CreateRenderTargetTextures();
					rkBG.CreateCharacterShadowTexture();
				}
#else
					rkBG.CreateCharacterShadowTexture();
#endif
				else
					canRender = false;
			}
		}

		if (!IsActive())
		{
			SkipRenderBuffering(3000);
		}

		if (!canRender)
		{
			SkipRenderBuffering(3000);
		}
		else
		{
			// RestoreLostDevice
			CCullingManager::Instance().Update();
			if (m_pyGraphic.Begin())
			{
				m_pyGraphic.ClearDepthBuffer();

#ifdef _DEBUG
				m_pyGraphic.SetClearColor(0.3f, 0.3f, 0.3f);
				m_pyGraphic.Clear();
#endif

				/////////////////////
				// Interface
				m_pyGraphic.SetInterfaceRenderState();

				OnUIRender();
				OnMouseRender();
				/////////////////////

				m_pyGraphic.End();

				m_pyGraphic.Show();

				DWORD dwRenderEndTime = ELTimer_GetMSec();

				static DWORD s_dwRenderCheckTime = dwRenderEndTime;
				static DWORD s_dwRenderRangeTime = 0;
				static DWORD s_dwRenderRangeFrame = 0;

				m_dwCurRenderTime = dwRenderEndTime - dwRenderStartTime;
				s_dwRenderRangeTime += m_dwCurRenderTime;
				++s_dwRenderRangeFrame;

				if (dwRenderEndTime - s_dwRenderCheckTime > 1000)
				{
					m_fAveRenderTime = float(double(s_dwRenderRangeTime) / double(s_dwRenderRangeFrame));

					s_dwRenderCheckTime = ELTimer_GetMSec();
					s_dwRenderRangeTime = 0;
					s_dwRenderRangeFrame = 0;
				}

				DWORD dwCurFaceCount = m_pyGraphic.GetFaceCount();
				m_pyGraphic.ResetFaceCount();
				s_dwFaceCount += dwCurFaceCount;

				if (dwCurFaceCount > 5000)
				{
					if (dwRenderEndTime > m_dwBufSleepSkipTime)
					{
						static float s_fBufRenderTime = 0.0f;

						float fCurRenderTime = m_dwCurRenderTime;

						if (fCurRenderTime > s_fBufRenderTime)
						{
							float fRatio = fMAX(0.5f, (fCurRenderTime - s_fBufRenderTime) / 30.0f);
							s_fBufRenderTime = (s_fBufRenderTime * (100.0f - fRatio) + (fCurRenderTime + 5) * fRatio) / 100.0f;
						}
						else
						{
							float fRatio = 0.5f;
							s_fBufRenderTime = (s_fBufRenderTime * (100.0f - fRatio) + fCurRenderTime * fRatio) / 100.0f;
						}

						if (s_fBufRenderTime > 100.0f)
							s_fBufRenderTime = 100.0f;

						DWORD dwBufRenderTime = s_fBufRenderTime;

						if (m_isWindowed)
						{
							if (dwBufRenderTime>58)
								dwBufRenderTime=64;
							else if (dwBufRenderTime>42)
								dwBufRenderTime=48;
							else if (dwBufRenderTime>26)
								dwBufRenderTime=32;
							else if (dwBufRenderTime>10)
								dwBufRenderTime=16;
							else
								dwBufRenderTime=8;
						}


						m_fAveRenderTime = s_fBufRenderTime;
					}

					m_dwFaceAccCount += dwCurFaceCount;
					m_dwFaceAccTime += m_dwCurRenderTime;

					m_fFaceSpd = (m_dwFaceAccCount / m_dwFaceAccTime);

					if (-1 == m_iForceSightRange)
					{
						static float s_fAveRenderTime = 16.0f;
						float fRatio = 0.3f;
						s_fAveRenderTime=(s_fAveRenderTime*(100.0f-fRatio)+max(16.0f, m_dwCurRenderTime)*fRatio)/100.0f;

						float fFar = 25600.0f;
						float fNear = MIN_FOG;
						double dbAvePow = double(1000.0f / s_fAveRenderTime);
						double dbMaxPow = 60.0;
						float fDistance=max(fNear+(fFar-fNear)*(dbAvePow)/dbMaxPow, fNear);
						m_pyBackground.SetViewDistanceSet (0, fDistance);
					}
					else
					{
						m_pyBackground.SetViewDistanceSet(0, float(m_iForceSightRange));
					}
				}
				else
				{
					m_pyBackground.SetViewDistanceSet(0, 25600.0f);
				}

				++s_dwRenderFrameCount;
			}
		}
	}

	int rest = s_uiNextFrameTime - ELTimer_GetMSec();

	if (rest > 0 && !bCurrentLateUpdate)
	{
		s_uiLoad -= rest;
		Sleep(rest);
	}

	++s_dwUpdateFrameCount;

	s_uiLoad += ELTimer_GetMSec() - dwStart;
	return true;
}

void CPythonApplication::UpdateClientRect()
{
	RECT rcApp;
	GetClientRect(&rcApp);
	OnSizeChange(rcApp.right - rcApp.left, rcApp.bottom - rcApp.top);
}

void CPythonApplication::SetMouseHandler(PyObject* poMouseHandler)
{
	m_poMouseHandler = poMouseHandler;
}

int CPythonApplication::CheckDeviceState()
{
	CGraphicDevice::EDeviceState e_deviceState = m_grpDevice.GetDeviceState();

	switch (e_deviceState)
	{
	case CGraphicDevice::DEVICESTATE_NULL:
		return DEVICE_STATE_FALSE;

	case CGraphicDevice::DEVICESTATE_BROKEN:
		return DEVICE_STATE_SKIP;

#ifdef ENABLE_FIX_MOBS_LAG
	case CGraphicDevice::DEVICESTATE_NEEDS_RESET:
	{
		m_pyBackground.ReleaseCharacterShadowTexture();
		CRenderTargetManager::Instance().ReleaseRenderTargetTextures();

		Trace("DEVICESTATE_NEEDS_RESET - attempting");
		if (!m_grpDevice.Reset())
		{
			return DEVICE_STATE_SKIP;
		}
		CRenderTargetManager::Instance().CreateRenderTargetTextures();
		m_pyBackground.CreateCharacterShadowTexture();
	}
	break;

	case CGraphicDevice::DEVICESTATE_OK:
		break;
	default:
		break;
	}
#else
	case CGraphicDevice::DEVICESTATE_NEEDS_RESET:
		if (!m_grpDevice.Reset())
			return DEVICE_STATE_SKIP;

		break;
	}
#endif

	return DEVICE_STATE_OK;
}

bool CPythonApplication::CreateDevice(int width, int height, int Windowed, int bit /* = 32*/, int frequency /* = 0*/)
{
	int iRet;

	m_grpDevice.InitBackBufferCount(2);
	m_grpDevice.RegisterWarningString(CGraphicDevice::CREATE_BAD_DRIVER, ApplicationStringTable_GetStringz(IDS_WARN_BAD_DRIVER, "WARN_BAD_DRIVER"));
	m_grpDevice.RegisterWarningString(CGraphicDevice::CREATE_NO_TNL, ApplicationStringTable_GetStringz(IDS_WARN_NO_TNL, "WARN_NO_TNL"));

	iRet = m_grpDevice.Create(GetWindowHandle(), width, height, Windowed ? true : false, bit, frequency);

	switch (iRet)
	{
	case CGraphicDevice::CREATE_OK:
		return true;

	case CGraphicDevice::CREATE_REFRESHRATE:
		return true;

	case CGraphicDevice::CREATE_ENUM:
	case CGraphicDevice::CREATE_DETECT:
		SET_EXCEPTION(CREATE_NO_APPROPRIATE_DEVICE);
		TraceError("CreateDevice: Enum & Detect failed");
		return false;

	case CGraphicDevice::CREATE_NO_DIRECTX:
		//PyErr_SetString(PyExc_RuntimeError, "DirectX 8.1 or greater required to run game");
		SET_EXCEPTION(CREATE_NO_DIRECTX);
		TraceError("CreateDevice: DirectX 8.1 or greater required to run game");
		return false;

	case CGraphicDevice::CREATE_DEVICE:
		//PyErr_SetString(PyExc_RuntimeError, "GraphicDevice create failed");
		SET_EXCEPTION(CREATE_DEVICE);
		TraceError("CreateDevice: GraphicDevice create failed");
		return false;

	case CGraphicDevice::CREATE_FORMAT:
		SET_EXCEPTION(CREATE_FORMAT);
		TraceError("CreateDevice: Change the screen format");
		return false;

		/*case CGraphicDevice::CREATE_GET_ADAPTER_DISPLAY_MODE:
		//PyErr_SetString(PyExc_RuntimeError, "GetAdapterDisplayMode failed");
		SET_EXCEPTION(CREATE_GET_ADAPTER_DISPLAY_MODE);
		TraceError("CreateDevice: GetAdapterDisplayMode failed");
		return false;*/

	case CGraphicDevice::CREATE_GET_DEVICE_CAPS:
		PyErr_SetString(PyExc_RuntimeError, "GetDevCaps failed");
		TraceError("CreateDevice: GetDevCaps failed");
		return false;

	case CGraphicDevice::CREATE_GET_DEVICE_CAPS2:
		PyErr_SetString(PyExc_RuntimeError, "GetDevCaps2 failed");
		TraceError("CreateDevice: GetDevCaps2 failed");
		return false;

	default:
		if (iRet & CGraphicDevice::CREATE_OK)
		{
			//if (iRet & CGraphicDevice::CREATE_BAD_DRIVER)
			//{
			//	LogBox(ApplicationStringTable_GetStringz(IDS_WARN_BAD_DRIVER), NULL, GetWindowHandle());
			//}
			if (iRet & CGraphicDevice::CREATE_NO_TNL)
			{
				CGrannyLODController::SetMinLODMode(true);
				//LogBox(ApplicationStringTable_GetStringz(IDS_WARN_NO_TNL), NULL, GetWindowHandle());
			}
			return true;
		}

		//PyErr_SetString(PyExc_RuntimeError, "Unknown Error!");
		SET_EXCEPTION(UNKNOWN_ERROR);
		TraceError("CreateDevice: Unknown Error!");
		return false;
	}
}

void CPythonApplication::Loop()
{
	while (1)
	{
		if (IsMessage())
		{
			if (!MessageProcess())
				break;
		}
		else
		{
			if (!Process())
				break;

			m_dwLastIdleTime = ELTimer_GetMSec();
		}
	}
}

// SUPPORT_NEW_KOREA_SERVER

bool LoadLocaleData(const char* localePath)
{
	CPythonNonPlayer& rkNPCMgr = CPythonNonPlayer::Instance();
	CItemManager& rkItemMgr = CItemManager::Instance();
	CPythonSkill& rkSkillMgr = CPythonSkill::Instance();
#ifdef ENABLE_GROWTH_PET_SYSTEM
	CPythonSkillPet& rkSkillPetMgr = CPythonSkillPet::Instance();
#endif
#ifdef ENABLE_GROWTH_MOUNT_SYSTEM
	CPythonSkillMount& rkSkillMountMgr = CPythonSkillMount::Instance();
#endif
#ifdef ENABLE_CHAT_DISCREDIT
	CPythonNetworkStream& rkNetStream = CPythonNetworkStream::Instance();
#endif

	char szItemList[256];
	char szItemProto[256];
	char szItemDesc[256];
#ifdef ENABLE_GROWTH_PET_SYSTEM
	char szSkillPetFileName[256];
#endif
#ifdef ENABLE_GROWTH_MOUNT_SYSTEM
	char szSkillMountFileName[256];
#endif
	char szMobProto[256];
	char szSkillDescFileName[256];
	char szSkillTableFileName[256];
#ifdef ENABLE_SHINING_EFFECT_UTILITY
	char szShiningTable[256];
#endif
#ifdef ENABLE_BLEND_REWORK
	char szBlendFileName[256];
#endif
#ifdef ENABLE_DS_SET
	char szDragonSoulTable[256];
#endif
#ifdef ENABLE_CHAT_DISCREDIT
	char szDiscreditList[256];
#endif

	snprintf(szItemProto, sizeof(szItemProto), "%s/item_proto", localePath);
	snprintf(szItemDesc, sizeof(szItemDesc), "%s/itemdesc.txt", localePath);
#ifdef ENABLE_GROWTH_PET_SYSTEM
	snprintf(szSkillPetFileName, sizeof(szSkillPetFileName), "%s/pet_skill.txt", localePath);
#endif
#ifdef ENABLE_GROWTH_MOUNT_SYSTEM
	snprintf(szSkillMountFileName, sizeof(szSkillMountFileName), "%s/mount_skill.txt", localePath);
#endif
	snprintf(szMobProto, sizeof(szMobProto), "%s/mob_proto", localePath);
	snprintf(szSkillDescFileName, sizeof(szSkillDescFileName), "%s/SkillDesc.txt", localePath);
	snprintf(szItemList, sizeof(szItemList), "%s/item_list.txt", localePath);
	snprintf(szSkillTableFileName, sizeof(szSkillTableFileName), "%s/skilltable.txt", localePath);
#ifdef ENABLE_SHINING_EFFECT_UTILITY
	snprintf(szShiningTable, sizeof(szShiningTable), "%s/shining_table.txt", localePath);
#endif
#ifdef ENABLE_BLEND_REWORK
	snprintf(szBlendFileName, sizeof(szBlendFileName), "%s/blend.txt", localePath);
#endif
#ifdef ENABLE_DS_SET
	snprintf(szDragonSoulTable, sizeof(szDragonSoulTable), "%s/dragon_soul_table.txt", localePath);
#endif
#ifdef ENABLE_CHAT_DISCREDIT
	snprintf(szDiscreditList, sizeof(szDiscreditList), "%s/discredit.txt", localePath);
#endif

	rkNPCMgr.Destroy();
	rkItemMgr.Destroy();
	rkSkillMgr.Destroy();
#ifdef ENABLE_GROWTH_PET_SYSTEM
	rkSkillPetMgr.Destroy();
#endif
#ifdef ENABLE_GROWTH_MOUNT_SYSTEM
	rkSkillMountMgr.Destroy();
#endif

	if (!rkItemMgr.LoadItemList(szItemList))
	{
		TraceError("LoadLocaleData - LoadItemList(%s) Error", szItemList);
	}

	if (!rkItemMgr.LoadItemTable(szItemProto))
	{
		TraceError("LoadLocaleData - LoadItemProto(%s) Error", szItemProto);
		return false;
	}

	if (!rkItemMgr.LoadItemDesc(szItemDesc))
	{
		Tracenf("LoadLocaleData - LoadItemDesc(%s) Error", szItemDesc);
	}

#ifdef ENABLE_SHINING_EFFECT_UTILITY
	if (!rkItemMgr.LoadShiningTable(szShiningTable))
	{
		Tracenf("LoadLocaleData - LoadShiningTable(%s) Error", szShiningTable);
	}
#endif

	if (!rkNPCMgr.LoadNonPlayerData(szMobProto))
	{
		TraceError("LoadLocaleData - LoadMobProto(%s) Error", szMobProto);
		return false;
	}

	if (!rkSkillMgr.RegisterSkillDesc(szSkillDescFileName))
	{
		TraceError("LoadLocaleData - RegisterSkillDesc(%s) Error", szSkillDescFileName);
		return false;
	}

	if (!rkSkillMgr.RegisterSkillTable(szSkillTableFileName))
	{
		TraceError("LoadLocaleData - RegisterSkillTable(%s) Error", szSkillTableFileName);
		return false;
	}
#ifdef ENABLE_GROWTH_PET_SYSTEM
	if (!rkSkillPetMgr.RegisterSkillPet(szSkillPetFileName))
	{
		TraceError("LoadLocaleData - RegisterSkillPet(%s) Error", szSkillPetFileName);
		return false;
	}
#endif
#ifdef ENABLE_GROWTH_MOUNT_SYSTEM
	if (!rkSkillMountMgr.RegisterSkillMount(szSkillMountFileName))
	{
		TraceError("LoadLocaleData - RegisterSkillMount(%s) Error", szSkillMountFileName);
		return false;
	}
#endif

#ifdef ENABLE_DS_SET
	if (!rkItemMgr.LoadDragonSoulTable(szDragonSoulTable))
	{
		Tracenf("LoadLocaleData - LoadDragonSoulTable(%s) Error", szDragonSoulTable);
	}
#endif

#ifdef ENABLE_CHAT_DISCREDIT
	if (!rkNetStream.LoadDiscreditFile(szDiscreditList))
	{
		Tracenf("CPythonApplication - CPythonNetworkStream::LoadDiscreditFile(%s)", szDiscreditList);
	}
#endif

#ifdef ENABLE_ACCE_SYSTEM
	char szItemScale[256];
	snprintf(szItemScale, sizeof(szItemScale), "%s/item_scale.txt", localePath);

	if (!rkItemMgr.LoadItemScale(szItemScale))
	{
		Tracenf("LoadLocaleData: error while loading %s.", szItemScale);
		return false;
	}
#endif

#ifdef ENABLE_AURA_COSTUME_SYSTEM
	char szAuraScale[256];
	snprintf(szAuraScale, sizeof(szAuraScale), "%s/aura_scale.txt", localePath);
	if (!rkItemMgr.LoadAuraScale(szAuraScale))
	{
		TraceError("LoadLocaleData - LoadAuraScale(%s) Error", szAuraScale);
	}
#endif

#ifdef ENABLE_BLEND_REWORK
	if (!rkItemMgr.LoadItemBlend(szBlendFileName))
	{
		TraceError("LoadLocaleData - LoadItemBlend(%s) Error", szBlendFileName);
		return false;
	}
#endif

	return true;
}
// END_OF_SUPPORT_NEW_KOREA_SERVER

unsigned __GetWindowMode(bool windowed)
{
	if (windowed)
		return WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;

	return WS_POPUP;
}

bool CPythonApplication::Create(PyObject* poSelf, const char* c_szName, int width, int height, int Windowed)
{
	Windowed = CPythonSystem::Instance().IsWindowed() ? 1 : 0;

	m_dwWidth = width;
	m_dwHeight = height;

	// Window
	UINT WindowMode = __GetWindowMode(Windowed ? true : false);

	if (!CMSWindow::Create(c_szName, 4, 0, WindowMode, ::LoadIcon(GetInstance(), MAKEINTRESOURCE(IDI_METIN2)), IDC_CURSOR_NORMAL))
	{
		//PyErr_SetString(PyExc_RuntimeError, "CMSWindow::Create failed");
		TraceError("CMSWindow::Create failed");
		SET_EXCEPTION(CREATE_WINDOW);
		return false;
	}

	if (m_pySystem.IsUseDefaultIME())
	{
		CPythonIME::Instance().UseDefaultIME();
	}

	if (!m_pySystem.IsWindowed())
	{
		m_isWindowed = false;
		m_isWindowFullScreenEnable = TRUE;
		__SetFullScreenWindow(GetWindowHandle(), width, height, m_pySystem.GetBPP());

		Windowed = true;
	}
	else
	{
		AdjustSize(m_pySystem.GetWidth(), m_pySystem.GetHeight());

		if (Windowed)
		{
			m_isWindowed = true;

			RECT workArea;
			SystemParametersInfo(SPI_GETWORKAREA, 0, &workArea, 0);

			const UINT workAreaWidth = (workArea.right - workArea.left);
			const UINT workAreaHeight = (workArea.bottom - workArea.top);

			const UINT windowWidth = m_pySystem.GetWidth() + GetSystemMetrics(SM_CXBORDER) * 2 + GetSystemMetrics(SM_CXDLGFRAME) * 2 + GetSystemMetrics(SM_CXFRAME) * 2;
			const UINT windowHeight = m_pySystem.GetHeight() + GetSystemMetrics(SM_CYBORDER) * 2 + GetSystemMetrics(SM_CYDLGFRAME) * 2 + GetSystemMetrics(SM_CYFRAME) * 2 + GetSystemMetrics(SM_CYCAPTION);

			const UINT x = workAreaWidth / 2 - windowWidth / 2;
			const UINT y = workAreaHeight / 2 - windowHeight / 2;

			SetPosition(x, y);
		}
		else
		{
			m_isWindowed = false;
			SetPosition(0, 0);
		}
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Cursor
	if (!CreateCursors())
	{
		//PyErr_SetString(PyExc_RuntimeError, "CMSWindow::Cursors Create Error");
		TraceError("CMSWindow::Cursors Create Error");
		SET_EXCEPTION("CREATE_CURSOR");
		return false;
	}

	if (!m_pySystem.IsNoSoundCard())
		m_SoundManager.Create();

	extern bool GRAPHICS_CAPS_SOFTWARE_TILING;

	// Device
	if (!CreateDevice(m_pySystem.GetWidth(), m_pySystem.GetHeight(), Windowed, m_pySystem.GetBPP(), m_pySystem.GetFrequency()))
		return false;

	GrannyCreateSharedDeformBuffer();
	SetVisibleMode(true);

	if (m_isWindowFullScreenEnable)
		SetWindowPos(GetWindowHandle(), HWND_TOP, 0, 0, width, height, SWP_SHOWWINDOW);

	if (!InitializeKeyboard(GetWindowHandle()))
		return false;

	m_pySystem.GetDisplaySettings();

	// Mouse
	if (m_pySystem.IsSoftwareCursor())
		SetCursorMode(CURSOR_MODE_SOFTWARE);
	else
		SetCursorMode(CURSOR_MODE_HARDWARE);

	// Network
	if (!m_netDevice.Create())
	{
		//PyErr_SetString(PyExc_RuntimeError, "NetDevice::Create failed");
		TraceError("NetDevice::Create failed");
		SET_EXCEPTION("CREATE_NETWORK");
		return false;
	}

	if (!m_grpDevice.IsFastTNL())
		CGrannyLODController::SetMinLODMode(true);

	m_pyItem.Create();

	// Other Modules
	DefaultFont_Startup();

	CPythonIME::Instance().Create(GetWindowHandle());
	CPythonIME::Instance().SetText("", 0);
	CPythonTextTail::Instance().Initialize();

	// Light Manager
	m_LightManager.Initialize();

	CGraphicImageInstance::CreateSystem(32);

	STICKYKEYS sStickKeys;
	memset(&sStickKeys, 0, sizeof(sStickKeys));
	sStickKeys.cbSize = sizeof(sStickKeys);
	SystemParametersInfo(SPI_GETSTICKYKEYS, sizeof(sStickKeys), &sStickKeys, 0);
	m_dwStickyKeysFlag = sStickKeys.dwFlags;

	sStickKeys.dwFlags &= ~(SKF_AVAILABLE | SKF_HOTKEYACTIVE);
	SystemParametersInfo(SPI_SETSTICKYKEYS, sizeof(sStickKeys), &sStickKeys, 0);

	// SphereMap
	CGrannyMaterial::CreateSphereMap(0, "d:/ymir work/special/spheremap.jpg");
	CGrannyMaterial::CreateSphereMap(1, "d:/ymir work/special/spheremap01.jpg");
#ifdef ENABLE_DISCORD_UTILITY
	m_kDiscordManager.InitSDK();
#endif
	return true;
}

void CPythonApplication::SetGlobalCenterPosition(LONG x, LONG y)
{
	CPythonBackground& rkBG = CPythonBackground::Instance();
	rkBG.GlobalPositionToLocalPosition(x, y);

	float z = CPythonBackground::Instance().GetHeight(x, y);

	CPythonApplication::Instance().SetCenterPosition(x, y, z);
}

void CPythonApplication::SetCenterPosition(float fx, float fy, float fz)
{
	m_v3CenterPosition.x = +fx;
	m_v3CenterPosition.y = -fy;
	m_v3CenterPosition.z = +fz;
}

void CPythonApplication::GetCenterPosition(TPixelPosition* pPixelPosition)
{
	pPixelPosition->x = +m_v3CenterPosition.x;
	pPixelPosition->y = -m_v3CenterPosition.y;
	pPixelPosition->z = +m_v3CenterPosition.z;
}

void CPythonApplication::SetServerTime(time_t tTime)
{
	m_dwStartLocalTime = ELTimer_GetMSec();
	m_tServerTime = tTime;
	m_tLocalStartTime = time(0);
}

time_t CPythonApplication::GetServerTime()
{
	return (ELTimer_GetMSec() - m_dwStartLocalTime) + m_tServerTime;
}

time_t CPythonApplication::GetServerTimeStamp()
{
	return (time(0) - m_tLocalStartTime) + m_tServerTime;
}

float CPythonApplication::GetGlobalTime()
{
	return m_fGlobalTime;
}

float CPythonApplication::GetGlobalElapsedTime()
{
	return m_fGlobalElapsedTime;
}

void CPythonApplication::SetFPS(int iFPS)
{
	m_iFPS = iFPS;
}

int CPythonApplication::GetWidth()
{
	return m_dwWidth;
}

int CPythonApplication::GetHeight()
{
	return m_dwHeight;
}

void CPythonApplication::SetConnectData(const char* c_szIP, int iPort)
{
	m_strIP = c_szIP;
	m_iPort = iPort;
}

void CPythonApplication::GetConnectData(std::string& rstIP, int& riPort)
{
	rstIP = m_strIP;
	riPort = m_iPort;
}

void CPythonApplication::EnableSpecialCameraMode()
{
	m_isSpecialCameraMode = TRUE;
}

void CPythonApplication::SetCameraSpeed(int iPercentage)
{
	m_fCameraRotateSpeed = c_fDefaultCameraRotateSpeed * float(iPercentage) / 100.0f;
	m_fCameraPitchSpeed = c_fDefaultCameraPitchSpeed * float(iPercentage) / 100.0f;
	m_fCameraZoomSpeed = c_fDefaultCameraZoomSpeed * float(iPercentage) / 100.0f;
}

void CPythonApplication::SetForceSightRange(int iRange)
{
	m_iForceSightRange = iRange;
}

void CPythonApplication::Clear()
{
	m_pySystem.Clear();
}

void CPythonApplication::Destroy()
{
	WebBrowser_Destroy();

	// SphereMap
	CGrannyMaterial::DestroySphereMap();

	m_kWndMgr.Destroy();

	CPythonSystem::Instance().SaveConfig();

#ifdef ENABLE_MODEL_RENDER_TARGET
	m_kRenderTargetMgr.Destroy();
#endif

	DestroyCollisionInstanceSystem();

	m_pyEventManager.Destroy();
	m_FlyingManager.Destroy();

	m_pyMiniMap.Destroy();

	m_pyTextTail.Destroy();
	m_pyChat.Destroy();
	m_kChrMgr.Destroy();
	m_RaceManager.Destroy();

	m_pyItem.Destroy();
	m_kItemMgr.Destroy();

	m_pyBackground.Destroy();

	m_kEftMgr.Destroy();
	m_LightManager.Destroy();

#ifdef USE_DISCORD_PRESENCE_NON_SDK
	m_kDiscordManager.ResetPresence();
#endif // USE_DISCORD_PRESENCE_NON_SDK

	// DEFAULT_FONT
	DefaultFont_Cleanup();
	// END_OF_DEFAULT_FONT

	GrannyDestroySharedDeformBuffer();

	m_pyGraphic.Destroy();

	m_pyRes.Destroy();

	m_kGuildMarkDownloader.Disconnect();

	CGrannyModelInstance::DestroySystem();
	CGraphicImageInstance::DestroySystem();

	m_SoundManager.Destroy();
	m_grpDevice.Destroy();

	//CSpeedTreeForestDirectX8::Instance().Clear();

	CAttributeInstance::DestroySystem();
	CTextFileLoader::DestroySystem();
	DestroyCursors();

	CMSApplication::Destroy();

	STICKYKEYS sStickKeys;
	memset(&sStickKeys, 0, sizeof(sStickKeys));
	sStickKeys.cbSize = sizeof(sStickKeys);
	sStickKeys.dwFlags = m_dwStickyKeysFlag;
	SystemParametersInfo(SPI_SETSTICKYKEYS, sizeof(sStickKeys), &sStickKeys, 0);
}


#ifdef ENABLE_MULTIFARM_BLOCK
void CPythonApplication::MultiFarmBlockIcon(uint8_t bStatus)
{
	HICON exeIcon = bStatus ? LoadIcon(ms_hInstance, MAKEINTRESOURCE(IDI_METIN2)) : LoadIcon(ms_hInstance, MAKEINTRESOURCE(IDI_METIN2));
	SendMessage(GetWindowHandle(), WM_SETICON, ICON_BIG, (LPARAM)exeIcon);
}
#endif