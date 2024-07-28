#include "StdAfx.h"
#include "PythonApplication.h"
#include "PythonPlayerSettingsModule.h"
#include "resource.h"
#include "Version.h"
#ifdef _DEBUG
#include <crtdbg.h>
#endif

#include "../eterPack/EterPackManager.h"
#include "../eterLib/Util.h"
#include "../CWebBrowser/CWebBrowser.h"

#include "../EterBase/error.h"

#include "../eterBase/CPostIt.h"

extern "C" {
	extern int _fltused;
	volatile int _AVOID_FLOATING_POINT_LIBRARY_BUG = _fltused;
};

extern "C"
{
	__declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
	__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

#pragma comment(linker, "/NODEFAULTLIB:libci.lib")
#pragma comment( lib, "version.lib" )
#pragma comment( lib, "python2.lib" )
#pragma comment( lib, "imagehlp.lib" )
#pragma comment( lib, "devil.lib" )
#if GrannyProductMinorVersion==4
#pragma comment( lib, "granny2.4.0.10.lib" )
#elif GrannyProductMinorVersion==7
#pragma comment( lib, "granny2.7.0.30.lib" )
#elif GrannyProductMinorVersion==8
#pragma comment( lib, "granny2.8.49.0.lib" )
#ifdef BUILDING_GRANNY_STATIC
#pragma comment( lib, "granny2.9.12.0_static.lib" )
#else
#pragma comment( lib, "granny2.9.12.0.lib" )
#endif // BUILDING_GRANNY_STATIC
#elif GrannyProductMinorVersion==11
#ifdef BUILDING_GRANNY_STATIC
#pragma comment( lib, "granny2.11.8.0_static.lib")
#else
#pragma comment( lib, "granny2.11.8.0.lib" )
#endif // BUILDING_GRANNY_STATIC
#else
#error "unknown granny version"
#endif
#pragma comment( lib, "mss32.lib" )
#pragma comment( lib, "winmm.lib" )
#pragma comment( lib, "imm32.lib" )
#pragma comment( lib, "oldnames.lib" )
#pragma comment( lib, "SpeedTreeRT.lib" )
#pragma comment( lib, "dinput8.lib" )
#pragma comment( lib, "dxguid.lib" )
#pragma comment( lib, "ws2_32.lib" )
#pragma comment( lib, "strmiids.lib" )
#pragma comment( lib, "ddraw.lib" )
#pragma comment( lib, "dmoguids.lib" )
//#pragma comment( lib, "wsock32.lib" )
#include <stdlib.h>
#include <cryptopp/cryptoppLibLink.h>

#ifdef ENABLE_PACK_TYPE_DIO
#	ifdef _WIN32
#		ifdef _DEBUG
#			pragma comment(lib, "liblz4_static-Debug.lib")
#			pragma comment(lib, "libPackTypeDio-Debug.lib")
#		else
#			pragma comment(lib, "liblz4_static-Release.lib")
#			pragma comment(lib, "libPackTypeDio-Release.lib")
#		endif
#	endif
#endif

#ifdef ENABLE_DISCORD_UTILITY
#pragma comment( lib, "discord_game_sdk.dll.lib" )
#pragma comment( lib, "discord-rpc.lib")
#ifdef _DEBUG
#pragma comment( lib, "discord_sdk_debug.lib" )
#else
#pragma comment( lib, "discord_sdk.lib" )
#endif // _DEBUG
#endif

#ifdef ENABLE_HWID_SECURITY_UTILITY
#include "HWIDManager.h"
#endif
#ifdef ENABLE_SPLASH
#include "Splash.h"
#endif
#ifdef ENABLE_ANTICHEAT
#include <rascal.h>
#endif
#ifdef ENABLE_CYTHON_MAIN
// don't include these two files .h .cpp if you're implementing cython via .pyd
//#include "eterCythonLib/PythonrootlibManager.h"
// it would be better including such file in the project, but this is easier at this moment:
//#include "eterCythonLib/PythonrootlibManager.cpp"
#ifdef ENABLE_CYTHON_EXTRA
// don't include these two files .h .cpp if you're implementing cython via .pyd
//#include "eterCythonLib/PythonuiscriptlibManager.h"
// it would be better including such file in the project, but this is easier at this moment:
//#include "eterCythonLib/PythonuiscriptlibManager.cpp"
#endif
#endif

extern bool SetDefaultCodePage(DWORD codePage);

char gs_szErrorString[512] = "";

void ApplicationSetErrorString(const char* szErrorString)
{
	strcpy(gs_szErrorString, szErrorString);
}

struct ApplicationStringTable
{
	HINSTANCE m_hInstance;
	std::map<DWORD, std::string> m_kMap_dwID_stLocale;
} gs_kAppStrTable;

void ApplicationStringTable_Initialize(HINSTANCE hInstance)
{
	gs_kAppStrTable.m_hInstance = hInstance;
}

const std::string& ApplicationStringTable_GetString(DWORD dwID, LPCSTR szKey)
{
	char szBuffer[512];
	char szIniFileName[256];
	char szLocale[256];

	::GetCurrentDirectory(sizeof(szIniFileName), szIniFileName);
	if (szIniFileName[lstrlen(szIniFileName) - 1] != '\\')
		strcat(szIniFileName, "\\");
	strcat(szIniFileName, "metin2client.dat");

	strcpy(szLocale, LocaleService_GetLocalePath());
	if (strnicmp(szLocale, "locale/", strlen("locale/")) == 0)
		strcpy(szLocale, LocaleService_GetLocalePath() + strlen("locale/"));
	::GetPrivateProfileString(szLocale, szKey, NULL, szBuffer, sizeof(szBuffer) - 1, szIniFileName);
	if (szBuffer[0] == '\0')
		LoadString(gs_kAppStrTable.m_hInstance, dwID, szBuffer, sizeof(szBuffer) - 1);
	if (szBuffer[0] == '\0')
		::GetPrivateProfileString("en", szKey, NULL, szBuffer, sizeof(szBuffer) - 1, szIniFileName);
	if (szBuffer[0] == '\0')
		strcpy(szBuffer, szKey);

	std::string& rstLocale = gs_kAppStrTable.m_kMap_dwID_stLocale[dwID];
	rstLocale = szBuffer;

	return rstLocale;
}

const std::string& ApplicationStringTable_GetString(DWORD dwID)
{
	char szBuffer[512];

	LoadString(gs_kAppStrTable.m_hInstance, dwID, szBuffer, sizeof(szBuffer) - 1);
	std::string& rstLocale = gs_kAppStrTable.m_kMap_dwID_stLocale[dwID];
	rstLocale = szBuffer;

	return rstLocale;
}

const char* ApplicationStringTable_GetStringz(DWORD dwID, LPCSTR szKey)
{
	return ApplicationStringTable_GetString(dwID, szKey).c_str();
}

const char* ApplicationStringTable_GetStringz(DWORD dwID)
{
	return ApplicationStringTable_GetString(dwID).c_str();
}

////////////////////////////////////////////

int Setup(LPSTR lpCmdLine); // Internal function forward

bool PackInitialize(const char* c_pszFolder)
{
	if (_access(c_pszFolder, 0) != 0)
		return true;
	bool bPackFirst = TRUE;
#ifdef NDEBUG
	Tracef("Note: PackFirst mode enabled. [pack]\n");
#else
	bPackFirst = FALSE;
	Tracef("Note: PackFirst mode not disabled. [file]\n");
#endif
	std::string stFolder = c_pszFolder + std::string("/");
	CTextFileLoader::SetCacheMode();
	CEterPackManager::Instance().SetCacheMode();
	CEterPackManager::Instance().SetSearchMode(bPackFirst);
	CSoundData::SetPackMode();
	std::vector<std::string> packlist
	{
		"_assets_season1",
		"effect",
		"environment",
		"guild",
		"icon",
		"item",
		"locale",
		"mapbase",
		"mapindoor",
		"mapoutdoor",
		"monster",
		"monster2",
		"npc",
		"npc_mount",
		"npc_pet",
		"npc2",
		"patch1",
		"pc",
		"pc2",
		"pc3",
		"Property",
		"sound",
		"special",
		"terrain",
		"textureset",
		"tree",
		"ui",
		"zone",
		"zonedg",
		"zoneindoor",
	};
	for (const auto& pack : packlist)
		CEterPackManager::Instance().RegisterPack((stFolder + pack).c_str(), "*");
	CEterPackManager::Instance().RegisterRootPack((stFolder + std::string("root")).c_str());
	return true;
}

bool RunMainScript(CPythonLauncher& pyLauncher, const char* lpCmdLine)
{
	initpack();
	initdbg();
	initime();
	initgrp();
	initgrpImage();
	initgrpText();
	initsystemSetting();
	initwndMgr();
#ifdef ENABLE_MODEL_RENDER_TARGET
	initRenderTarget();
#endif
	initapp();
	initchr();
	initchrmgr();
	initPlayer();
	initItem();
	initNonPlayer();
	initTrade();
	initChat();
	initTextTail();
	initnet();
	initMiniMap();
	initEvent();
	initeffect();
	initfly();
	initsnd();
	initeventmgr();
	initshop();
	initskill();
	initquest();
	initBackground();
	initMessenger();
	initsafebox();
	initguild();
	initServerStateChecker();
#ifndef USE_LOADING_DLG_OPTIMIZATION
	initplayersettingsmodule();
#endif // USE_LOADING_DLG_OPTIMIZATION
#ifdef ENABLE_OFFLINE_SHOP
	initofflineshop();
#endif
#ifdef ENABLE_ACCE_SYSTEM
	initAcce();
#endif
#ifdef ENABLE_GROWTH_PET_SYSTEM
	initskillpet();
#endif
#ifdef ENABLE_GROWTH_MOUNT_SYSTEM
	initskillmount();
#endif
#ifdef ENABLE_EVENT_SYSTEM
	initGameEvents();
#endif
#ifdef ENABLE_CHANGELOOK_SYSTEM
	initChangeLook();
#endif
#ifdef ENABLE_SWITCHBOT_SYSTEM
	initSwitchbot();
#endif
#ifdef ENABLE_CUBE_RENEWAL
	intcuberenewal();
#endif
#ifdef ENABLE_LOGIN_DLG_RENEWAL
	initAccountManager();
#endif

	PyObject* builtins = PyImport_ImportModule("__builtin__");
#ifdef NDEBUG // @warme601 _DISTRIBUTE -> NDEBUG
	PyModule_AddIntConstant(builtins, "__DEBUG__", 1);
#else
	PyModule_AddIntConstant(builtins, "__DEBUG__", 0);
#endif
#ifdef ENABLE_CYTHON_MAIN
	PyModule_AddIntConstant(builtins, "ENABLE_CYTHON_MAIN", 1);
#else
	PyModule_AddIntConstant(builtins, "ENABLE_CYTHON_MAIN", 0);
#endif
#ifdef ENABLE_CYTHON_EXTRA
	PyModule_AddIntConstant(builtins, "ENABLE_CYTHON_EXTRA", 1);
#else
	PyModule_AddIntConstant(builtins, "ENABLE_CYTHON_EXTRA", 0);
#endif

	// RegisterCommandLine
	{
		const char* loginMark = "-cs";
		const char* loginMark_NonEncode = "-ncs";
		const char* seperator = " ";

		std::string stCmdLine;
		const int CmdSize = 3;
		vector<std::string> stVec;
		SplitLine(lpCmdLine, seperator, &stVec);
		if (CmdSize == stVec.size() && stVec[0] == loginMark)
		{
			char buf[MAX_PATH];
			base64_decode(stVec[2].c_str(), buf);
			stVec[2] = buf;
			string_join(seperator, stVec, &stCmdLine);
		}
		else if (CmdSize <= stVec.size() && stVec[0] == loginMark_NonEncode)
		{
			stVec[0] = loginMark;
			string_join(" ", stVec, &stCmdLine);
		}
		else
			stCmdLine = lpCmdLine;

		PyModule_AddStringConstant(builtins, "__COMMAND_LINE__", stCmdLine.c_str());
	}
	{
		vector<std::string> stVec;
		SplitLine(lpCmdLine, " ", &stVec);

		if (stVec.size() != 0 && "--pause-before-create-window" == stVec[0])
		{
			system("pause");
		}

#ifdef ENABLE_SPLASH
	CSplash splash1("hack.bmp", RGB(128, 128, 128));
	splash1.ShowSplash();
	Sleep(2000);
	splash1.CloseSplash();
#endif

#ifdef ENABLE_CYTHON_MAIN
		if (!pyLauncher.RunLine("import rootlib\nrootlib.moduleImport('system')"))
#else
		if (!pyLauncher.RunFile("system.py"))
#endif
		{
			TraceError("RunMain Error");
			return false;
		}
	}

	return true;
}

bool Main(HINSTANCE hInstance, LPSTR lpCmdLine)
{
	DWORD dwRandSeed = time(NULL) + DWORD(GetCurrentProcess());
	srandom(dwRandSeed);
	srand(random());

	SetLogLevel(1);
	SetEterExceptionHandler();

	ilInit();
	if (!Setup(lpCmdLine))
		return false;

#ifdef _DEBUG
	OpenConsoleWindow();
#endif

#ifdef LIVE_SERVER
	OpenLogFile(false);
#else
	OpenLogFile(true);
#endif

	static CLZO				lzo;
	static CEterPackManager	EterPackManager;
#ifdef ENABLE_HWID_SECURITY_UTILITY
	static HWIDMANAGER		hwidManager;
#endif
#ifdef USE_LOADING_DLG_OPTIMIZATION
	static CPlayerSettingsModule	playerSettings;
#endif // USE_LOADING_DLG_OPTIMIZATION

	if (!PackInitialize("pack"))
	{
		LogBox("Pack Initialization failed. Check log.txt file..");
		return false;
	}

	if (LocaleService_LoadGlobal(hInstance))
		SetDefaultCodePage(LocaleService_GetCodePage());

	CPythonApplication* app = new CPythonApplication;

	app->Initialize(hInstance);

	bool ret = false;
	{
		CPythonLauncher pyLauncher;

		if (pyLauncher.Create())
		{
			ret = RunMainScript(pyLauncher, lpCmdLine);
		}

		app->Clear();

		timeEndPeriod(1);
		pyLauncher.Clear();
	}

	app->Destroy();
	delete app;

	return ret;
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
#ifdef ENABLE_ANTICHEAT
	initRascal(NULL);
#endif
#ifdef _DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_CHECK_CRT_DF | _CRTDBG_LEAK_CHECK_DF);
	//_CrtSetBreakAlloc( 110247 );
#endif

	ApplicationStringTable_Initialize(hInstance);

	LocaleService_LoadConfig("locale.cfg");
	SetDefaultCodePage(LocaleService_GetCodePage());
	WebBrowser_Startup(hInstance);

	Main(hInstance, lpCmdLine);

	WebBrowser_Cleanup();

	::CoUninitialize();

	if (gs_szErrorString[0])
		MessageBox(NULL, gs_szErrorString, ApplicationStringTable_GetStringz(IDS_APP_NAME, "APP_NAME"), MB_ICONSTOP);

	return 0;
}

#if GrannyProductMinorVersion==4 || GrannyProductMinorVersion==7
static void GrannyError(granny_log_message_type Type,
	granny_log_message_origin Origin,
	char const* Error,
	void* UserData)
{
	//TraceError("GRANNY: %s", Error);
}
#elif GrannyProductMinorVersion==11 || GrannyProductMinorVersion==9 || GrannyProductMinorVersion==8
static void GrannyError(granny_log_message_type Type,
	granny_log_message_origin Origin,
	char const* File,
	granny_int32x Line,
	char const* Error,
	void* UserData)
{
	//Origin==GrannyFileReadingLogMessage for granny run-time tag& revision warning (Type==GrannyWarningLogMessage)
	//Origin==GrannyControlLogMessage for miss track_group on static models as weapons warning (Type==GrannyWarningLogMessage)
	//Origin==GrannyMeshBindingLogMessage for miss bone ToSkeleton on new ymir models error (Type==GrannyErrorLogMessage)
	// if (Type == GrannyWarningLogMessage)
	if (Origin == GrannyFileReadingLogMessage || Origin == GrannyControlLogMessage || Origin == GrannyMeshBindingLogMessage)
		return;
	//TraceError("GRANNY: %s(%d): ERROR: %s --- [%d] %s --- [%d] %s", File, Line, Error, Type, GrannyGetLogMessageTypeString(Type), Origin, GrannyGetLogMessageOriginString(Origin));
}
#else
#error "unknown granny version"
#endif

int Setup(LPSTR lpCmdLine)
{
	TIMECAPS tc;
	UINT wTimerRes;

	if (timeGetDevCaps(&tc, sizeof(TIMECAPS)) != TIMERR_NOERROR)
		return 0;

	wTimerRes = MINMAX(tc.wPeriodMin, 1, tc.wPeriodMax);
	timeBeginPeriod(wTimerRes);

	granny_log_callback Callback;
	Callback.Function = GrannyError;
	Callback.UserData = 0;
	GrannySetLogCallback(&Callback);
	return 1;
}


