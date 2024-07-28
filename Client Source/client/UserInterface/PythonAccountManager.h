#pragma once

#include "StdAfx.h"

#ifdef ENABLE_LOGIN_DLG_RENEWAL
#include "../EterPack/md5.h"
#include <IPHlpApi.h>

class CPythonAccountManager : public CSingleton<CPythonAccountManager>
{
public:
	typedef struct SAccountInfo {
		BYTE	bIndex;
		char	szLoginName[64 + 1];
		char	szPassword[64 + 1];
#ifdef ENABLE_PIN_SYSTEM
		char	szPIN[64 + 1];
#endif
	} TAccountInfo;

public:
	CPythonAccountManager();
	~CPythonAccountManager();

	void	Initialize(const char* szFileName);
	void	Destroy();

public:
	void				SetLastAccountName(const std::string& c_rstLoginName) { m_stLastAccountName = c_rstLoginName; }
	const std::string& GetLastAccountName() const { return m_stLastAccountName; }

#ifdef ENABLE_PIN_SYSTEM
	void				SetAccountInfo(BYTE bIndex, const char* szLoginName, const char* szPassword, const char* szPIN);
#else
	void				SetAccountInfo(BYTE bIndex, const char* szLoginName, const char* szPassword);
#endif
	void				RemoveAccountInfo(BYTE bIndex);
	const TAccountInfo* GetAccountInfo(BYTE bIndex);

private:
	std::string	m_stFileName;

	std::string	m_stLastAccountName;
	std::vector<TAccountInfo> m_vec_AccountInfo;

protected:
	void ReadFromRegistry(char* szPIDName, char* szPath, char* szBuf, bool bLocalMachine)
	{
		HKEY Registry;
		HKEY key = HKEY_CURRENT_USER;
		if (bLocalMachine)
			key = HKEY_LOCAL_MACHINE;

		DWORD regType = 0, regSize = 0;
		long ReturnStatus = RegOpenKeyEx(key, szPath, 0, KEY_QUERY_VALUE | KEY_WOW64_64KEY, &Registry);
		if (ReturnStatus == ERROR_SUCCESS)
		{
			RegQueryValueEx(Registry, szPIDName, 0, &regType, 0, &regSize);
			RegQueryValueEx(Registry, szPIDName, 0, &regType, (LPBYTE)szBuf, &regSize);
			RegCloseKey(Registry);
			if (szBuf[regSize] < 32)
				szBuf[regSize] = '\0';

			if (regSize > 1)
				return;
			else
				strcpy(szBuf, "2181");
		}
		else
		{
			RegCloseKey(Registry);
			strcpy(szBuf, "2181");
		}
	};

	const char* GetMD5(const char* szString)
	{
		static char s_szResult[16 * 2 + 1];
		static unsigned char* s_pszBuf;
		static int s_iLen;

		s_iLen = strlen(szString);
		s_pszBuf = new unsigned char[s_iLen + 1];
		memcpy(s_pszBuf, szString, s_iLen);
		s_pszBuf[s_iLen] = '\0';

		MD5_CTX md5;
		MD5Init(&md5);
		MD5Update(&md5, s_pszBuf, s_iLen);
		MD5Final(&md5);

		for (int i = 0; i < 16; ++i)
			sprintf(s_szResult + (i * 2), "%02x", md5.digest[i]);

		return s_szResult;
	};

	const char* GetHWID() {
		static char s_szHWIDBuffer[2048];
		static bool s_bSet = false;

		if (!s_bSet)
		{
			try
			{
				// get system data
				SYSTEM_INFO kSystemInfo;
				GetSystemInfo(&kSystemInfo);

				char szProductID[512 + 1];
				ReadFromRegistry("ProductId", "SOFTWARE\\MICROSOFT\\Windows NT\\CurrentVersion", szProductID, true);

				unsigned long ulOutBufLen = sizeof(IP_ADAPTER_INFO);
				IP_ADAPTER_INFO* pAdapterInfo = (IP_ADAPTER_INFO*) new char[ulOutBufLen];
				DWORD dwRetVal = GetAdaptersInfo(pAdapterInfo, &ulOutBufLen);
				if (dwRetVal != ERROR_SUCCESS)
				{
					if (dwRetVal == ERROR_BUFFER_OVERFLOW)
					{
						pAdapterInfo = (IP_ADAPTER_INFO*) new char[ulOutBufLen];
						dwRetVal = GetAdaptersInfo(pAdapterInfo, &ulOutBufLen);
					}
				}

				// write into string
				std::string stTemp;
				char szNum[20 + 1];
#define AddNumber(num) _itoa_s(num, szNum, 10), stTemp += szNum

				stTemp += "*3a!#\"y";
				AddNumber(kSystemInfo.wProcessorArchitecture);
				AddNumber(kSystemInfo.dwNumberOfProcessors);
				AddNumber(kSystemInfo.dwProcessorType);
				AddNumber(kSystemInfo.wProcessorLevel);
				AddNumber(kSystemInfo.wProcessorRevision);

				stTemp += szProductID;

				AddNumber(IsProcessorFeaturePresent(PF_3DNOW_INSTRUCTIONS_AVAILABLE));
				AddNumber(IsProcessorFeaturePresent(PF_CHANNELS_ENABLED));
				AddNumber(IsProcessorFeaturePresent(PF_COMPARE_EXCHANGE_DOUBLE));
				AddNumber(IsProcessorFeaturePresent(PF_FLOATING_POINT_EMULATED));
				AddNumber(IsProcessorFeaturePresent(PF_MMX_INSTRUCTIONS_AVAILABLE));
				AddNumber(IsProcessorFeaturePresent(PF_PAE_ENABLED));
				AddNumber(IsProcessorFeaturePresent(PF_RDTSC_INSTRUCTION_AVAILABLE));
				AddNumber(IsProcessorFeaturePresent(PF_XMMI_INSTRUCTIONS_AVAILABLE));
				AddNumber(IsProcessorFeaturePresent(PF_XMMI64_INSTRUCTIONS_AVAILABLE));
#undef AddNumber

				strcpy(s_szHWIDBuffer, stTemp.c_str());
				strcat(s_szHWIDBuffer, "pebia22##1+7de=  ");

				if (dwRetVal == ERROR_SUCCESS)
				{
					for (int i = 0; i < pAdapterInfo->AddressLength; ++i)
					{
						char szTemp[20];
						if (unsigned(i) == pAdapterInfo->AddressLength - 1)
							snprintf(szTemp, sizeof(szTemp), "%.2X", (int)pAdapterInfo->Address[i]);
						else
							snprintf(szTemp, sizeof(szTemp), "%.2X-", (int)pAdapterInfo->Address[i]);
						strcat(s_szHWIDBuffer, szTemp);
					}
					strcat(s_szHWIDBuffer, pAdapterInfo->AdapterName);
				}

				delete[]((char*)pAdapterInfo);

				// get serial number of C: drive
				for (int i = 0; i < 2; ++i)
				{
					char szDrives[256];
					char szVolumeName[256];
					char szFileSystem[256];
					DWORD dwSerialNumber;

					GetLogicalDriveStrings(256, szDrives);
					char* p = szDrives;

					while (*p != '\0')
					{
						DWORD dwDriveType = GetDriveType(p);
						if (i == 1 || strstr(p, "C:"))
						{
							if (dwDriveType != DRIVE_REMOVABLE && dwDriveType != DRIVE_REMOTE && dwDriveType != DRIVE_CDROM && dwDriveType != DRIVE_RAMDISK)
							{
								GetVolumeInformation(p, szVolumeName, 256, &dwSerialNumber, 0, 0, szFileSystem, 256);

								_itoa_s(dwSerialNumber, szNum, 10);
								strcat(s_szHWIDBuffer, szNum);

								s_bSet = true;

								return s_szHWIDBuffer;
							}
						}

						p = strchr(p, '\0');
						p += 1;
					}
				}
			}
			catch (int e)
			{
				snprintf(s_szHWIDBuffer, sizeof(s_szHWIDBuffer), "ERROR: %d", e);
			}

			s_bSet = true;
		}

		return s_szHWIDBuffer;
	};

	const char* GetHWIDHash() { return GetMD5(GetHWID()); };
	const char* GetSecurityHWIDHash() 
	{
		std::string stHWID = GetHWID();
		std::string stKey = XOR("AnaniSikiyorumHerGunSenseDebuggerleBuYaziyiOkuyorsun:)");
		std::string comp = stHWID + stKey;
		return GetMD5(comp.c_str());
	};
};

#endif