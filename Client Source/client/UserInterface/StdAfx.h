#pragma once

#pragma warning(disable:4702)
#pragma warning(disable:4100)
#pragma warning(disable:4201)
#pragma warning(disable:4511)
#pragma warning(disable:4663)
#pragma warning(disable:4018)
#pragma warning(disable:4245)

#if _MSC_VER >= 1400
//if don't use below, time_t is 64bit
#define _USE_32BIT_TIME_T
#endif
#include <iterator>
#include "../eterLib/StdAfx.h"
#include "../eterPythonLib/StdAfx.h"
#include "../gameLib/StdAfx.h"
#include "../scriptLib/StdAfx.h"
#include "../milesLib/StdAfx.h"
#include "../EffectLib/StdAfx.h"
#include "../PRTerrainLib/StdAfx.h"
#include "../SpeedTreeLib/StdAfx.h"

#ifndef __D3DRM_H__
#define __D3DRM_H__
#endif

#include <dshow.h>
#include <qedit.h>

#include <stdint.h>
#include <cinttypes>

#include "Locale.h"

#include "GameType.h"
#include <XORstr.h>

extern DWORD __DEFAULT_CODE_PAGE__;

#define APP_NAME	"Metin 2"

enum EOther
{
	POINT_MAX_NUM = 255,
	CHARACTER_NAME_MAX_LEN = 64,
	PLAYER_NAME_MAX_LEN = 12,
#ifdef ENABLE_OFFLINE_SHOP
	OFFLINE_SHOP_NAME_MAX_LEN = 64 + 1,
	OFFLINE_SHOP_ITEM_MAX_LEN = 64,
#endif
};

void initapp();
void initime();
void initchr();
void initchrmgr();
void initsystemSetting();
void initChat();
void initTextTail();
void initime();
void initItem();
void initNonPlayer();
void initnet();
void initPlayer();
void initSectionDisplayer();
void initServerStateChecker();
void initTrade();
void initMiniMap();
void initEvent();
void initeffect();
void initsnd();
void initeventmgr();
void initBackground();
void initwndMgr();
void initshop();
void initpack();
void initskill();
void initfly();
void initquest();
void initsafebox();
void initguild();
void initMessenger();
#ifdef USE_LOADING_DLG_OPTIMIZATION
void initplayersettingsmodule();
#endif // USE_LOADING_DLG_OPTIMIZATION
#ifdef ENABLE_OFFLINE_SHOP
void initofflineshop();
#endif
#ifdef ENABLE_ACCE_SYSTEM
void initAcce();
#endif
#ifdef ENABLE_GROWTH_PET_SYSTEM
void initskillpet();
#endif
#ifdef ENABLE_GROWTH_MOUNT_SYSTEM
void initskillmount();
#endif
#ifdef ENABLE_EVENT_SYSTEM
void initGameEvents();
#endif
#ifdef ENABLE_CHANGELOOK_SYSTEM
void initChangeLook();
#endif
#ifdef ENABLE_SWITCHBOT_SYSTEM
void initSwitchbot();
#endif
#ifdef ENABLE_CUBE_RENEWAL
void intcuberenewal();
#endif
#ifdef ENABLE_LOGIN_DLG_RENEWAL
void initAccountManager();
#endif
#ifdef ENABLE_MODEL_RENDER_TARGET
void initRenderTarget();
#endif

extern const std::string& ApplicationStringTable_GetString(DWORD dwID);
extern const std::string& ApplicationStringTable_GetString(DWORD dwID, LPCSTR szKey);

extern const char* ApplicationStringTable_GetStringz(DWORD dwID);
extern const char* ApplicationStringTable_GetStringz(DWORD dwID, LPCSTR szKey);

extern void ApplicationSetErrorString(const char* szErrorString);

