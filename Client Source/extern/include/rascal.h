#pragma once
#ifndef SDK_H
#define SDK_H
#pragma comment(linker, "/ALIGN:0x10000")
#pragma check_stack(off)
#include <Windows.h>
#include <vector>
#include <string>
#include <vector>
#include "skCrypter.h"
#include "../../client/EterLib/MSApplication.h"
#include <intrin.h>
//#include "../../source/UserInterface/Splash.h"
//static CSplashWnd splash;
//static ULONG_PTR gdiplusToken;
//static IStream* stream;
//
//static VOID __stdcall StartSplash()
//{
//	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
//	Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
//	int imageSize = sizeof(imageData);
//	CreateStreamOnHGlobal(NULL, TRUE, &stream);
//	stream->Write(imageData, imageSize, NULL);
//	stream->Seek({ 0 }, STREAM_SEEK_SET, NULL);
//	Gdiplus::Image* image = new Gdiplus::Image(stream);
//	splash.SetImage(image);
//	splash.Show();
//}
//
//static void WINAPI FinishSplash()
//{
//	splash.Hide();
//	Gdiplus::GdiplusShutdown(gdiplusToken);
//}

#define ImageNtHeaders(smodule) \
    (reinterpret_cast<PIMAGE_NT_HEADERS>(\
        reinterpret_cast<SIZE_T>(smodule) + \
        static_cast<SIZE_T>((reinterpret_cast<PIMAGE_DOS_HEADER>(smodule))->e_lfanew)\
    ))

static uintptr_t __fastcall getcalleraddr()
{
	void* caller_addr = NULL;
	ULONG frame_index = 1;
	HMODULE module = GetModuleHandle(NULL);
	PIMAGE_NT_HEADERS nt_header = ImageNtHeaders(module);
	PIMAGE_OPTIONAL_HEADER optional_header = &nt_header->OptionalHeader;
	PVOID exception_handler = (PVOID)GetProcAddress(module, skCrypt("ExceptionHandler"));
	const int max_frames = 62;
	void* frames[max_frames];

	int frame_count = CaptureStackBackTrace(1, max_frames, frames, NULL);
	if (frame_count > 1)
	{
		return reinterpret_cast<uintptr_t>(frames[1]);
	}

	return 0;
}

typedef void(*FunctionProtect)(void*);
typedef void(*Initialize)(std::string, int,std::string);
typedef void(*getunknown)(int);
enum Protect
{
	ATTACK,
	ATTACKSPEED,
	SETATTACKKEY,
	MOVE,
	ATTACKINIT,
	ATTACKKEYINIT,
	NOTHING
};
typedef void(*tokucbabo)(Protect atype, void* addr, int previousvalue);

static int GetRascalID(DWORD threadId)
{
	CMSApplication::DThreadId = threadId;
	return threadId;
}

static VOID WINAPI initRascal(LPARAM lParam)
{
	HMODULE hRascal = LoadLibraryA(skCrypt("rascal.dll"));
	if (hRascal != NULL)
	{
		Initialize Init = (Initialize)(GetProcAddress(hRascal, skCrypt("Initialize")));
		getunknown getunk = (getunknown)(GetProcAddress(hRascal, skCrypt("getunknown")));
		if(getunk == NULL){
			printf("%d", *(DWORD*)*(DWORD*)*(DWORD*)*(DWORD*)0x400U);
			__fastfail(0);
			std::terminate();
			*((unsigned int*)0) = 0xDEAD;
			atexit(0);
			abort();
		}
		getunk(89);
		Init(std::string(skCrypt("194.15.52.109")), 3,std::string(skCrypt("user=tatianayigottensiktik")));
		void* getidfunc = GetProcAddress(hRascal, skCrypt("GetRascalThreadID"));
		if (getidfunc != NULL)
		{
			reinterpret_cast<void(*)(void*)>(getidfunc)(reinterpret_cast<void*>(GetRascalID));
		}
		else
		{
			printf("%d", *(DWORD*)*(DWORD*)*(DWORD*)*(DWORD*)0x400U);
			__fastfail(0);
			std::terminate();
			*((unsigned int*)0) = 0xDEAD;
			atexit(0);
			abort();
		}
	}
	else
	{
		printf("%d", *(DWORD*)*(DWORD*)*(DWORD*)*(DWORD*)0x400U);
		__fastfail(0);
		std::terminate();
		*((unsigned int*)0) = 0xDEAD;
		atexit(0);
		abort();
	}
}

#endif