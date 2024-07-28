#include "CMachineGuid.h"

CMachineGuid::~CMachineGuid()
{
}

std::string WidestringToString(std::wstring wstr)
{
	if (wstr.empty())
	{
		return std::string();
	}
#if defined WIN32
	int size = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, &wstr[0], wstr.size(), NULL, 0, NULL, NULL);
	std::string ret = std::string(size, 0);
	WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, &wstr[0], wstr.size(), &ret[0], size, NULL, NULL);
#else
	size_t size = 0;
	_locale_t lc = _create_locale(LC_ALL, "en_US.UTF-8");
	errno_t err = _wcstombs_s_l(&size, NULL, 0, &wstr[0], _TRUNCATE, lc);
	std::string ret = std::string(size, 0);
	err = _wcstombs_s_l(&size, &ret[0], size, &wstr[0], _TRUNCATE, lc);
	_free_locale(lc);
	ret.resize(size - 1);
#endif
	return ret;
}

CMachineGuid::CMachineGuid()
{
	HKEY hKey;
	LSTATUS res = RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Cryptography", 0, KEY_READ | KEY_WOW64_64KEY, &hKey);

	if (res == ERROR_SUCCESS)
	{
		std::wstring temp;
		res = GetStringRegKey(hKey, L"MachineGuid", temp, L"");
		std::string tempStr = WidestringToString(temp);
		if (res == ERROR_SUCCESS)
		{
			m_MachineGUID.assign(tempStr.begin(), tempStr.end());
		}
	}
	RegCloseKey(hKey);
}

LONG CMachineGuid::GetStringRegKey(HKEY hKey, const std::wstring& strValueName, std::wstring& strValue, const std::wstring& strDefaultValue)
{
	strValue = strDefaultValue;
	WCHAR szBuffer[512];
	DWORD dwBufferSize = sizeof(szBuffer);
	ULONG nError;
	nError = RegQueryValueExW(hKey, strValueName.c_str(), 0, nullptr, (LPBYTE)szBuffer, &dwBufferSize);
	if (ERROR_SUCCESS == nError)
		strValue = szBuffer;

	return nError;
}