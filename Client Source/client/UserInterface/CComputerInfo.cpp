#include "CComputerInfo.h"
#include "windows.h"
#include <Xorstr.h>

CComputerInfo::CComputerInfo()
{
}

std::string CComputerInfo::getComputerName()
{
	const int buffer_size = MAX_COMPUTERNAME_LENGTH + 1;
	char buffer[buffer_size];
	DWORD lpnSize = buffer_size;
	if (GetComputerNameA(buffer, &lpnSize) == FALSE)
		return std::string(XOR("UNKNOWN_PCNAME"));
	return std::string{ buffer };
}

std::string CComputerInfo::getUserName()
{
	TCHAR username[256 + 1];
	DWORD size = 256 + 1;
	if (GetUserName((TCHAR*)username, &size) == FALSE)
		return std::string(XOR("UNKNOWN_USERNAME"));
	return username;
}

std::string CComputerInfo::getOSVersion()
{
	/*if (IsWindows10OrGreater() == true)
		return "Windows10";
	else if (IsWindows8Point1OrGreater() == true)
		return "Windows8";
	else if (IsWindows8OrGreater() == true)
		return "Windows8";
	else if (IsWindows7SP1OrGreater() == true)
		return "Windows7";
	else if (IsWindows7OrGreater() == true)
		return "Windows7";
	else if (IsWindowsVistaSP2OrGreater() == true)
		return "WindowsVista";
	else if (IsWindowsVistaSP1OrGreater() == true)
		return "WindowsVista";
	else if (IsWindowsVistaOrGreater() == true)
		return "WindowsVista";
	else if (IsWindowsXPSP3OrGreater() == true)
		return "WindowsXP";
	else if (IsWindowsXPSP2OrGreater() == true)
		return "WindowsXP";
	else if (IsWindowsXPSP1OrGreater() == true)
		return "WindowsXP";
	else if (IsWindowsXPOrGreater() == true)
		return "WindowsXP";
	else if (IsWindowsServer() == true)
		return "WindowsServer";*/
	
	return "Bilinmiyor.";
}

CComputerInfo::~CComputerInfo()
{
}
