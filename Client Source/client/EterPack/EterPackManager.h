#pragma once

#include <windows.h>
#include <boost/unordered_map.hpp>
#include "../eterBase/Singleton.h"
#include "../eterBase/Stl.h"

#include "EterPack.h"

class CEterPackManager : public CSingleton<CEterPackManager>
{
public:
	struct SCache
	{
		BYTE* m_abBufData;
		DWORD m_dwBufSize;
	};
public:
	enum ESearchModes
	{
		SEARCH_FILE_FIRST,
		SEARCH_PACK_FIRST
	};

	typedef std::list<CEterPack*> TEterPackList;
	typedef boost::unordered_map<std::string, CEterPack*, stringhash> TEterPackMap;

public:
	CEterPackManager();
	virtual ~CEterPackManager();

	void SetCacheMode();
	void SetRelativePathMode();

	void LoadStaticCache(const char* c_szFileName);

	void SetSearchMode(bool bPackFirst);
	int GetSearchMode();

	// replace the function signatures and the int32 1234 password (highly suggested)
#ifdef USE_EPK_NEW_SIGNATURE
	__forceinline bool Get(CMappedFile& rMappedFile, const char* c_szFileName, LPCVOID* pData)
	{
		return _Get(c_szFileName, 0, 0, 0, 0, USE_EPK_NEW_SIGNATURE, pData, rMappedFile);
	}
	bool _Get(const char* c_szFileName, int, int, int, int, const int pass, LPCVOID* pData, CMappedFile& rMappedFile);

	__forceinline bool GetFromPack(CMappedFile& rMappedFile, const char* c_szFileName, LPCVOID* pData)
	{
		return _GetFromPack(pData, 0, 0, 0, 0, USE_EPK_NEW_SIGNATURE, c_szFileName, rMappedFile);
	}
	bool _GetFromPack(LPCVOID* pData, int, int, int, int, const int pass, const char* c_szFileName, CMappedFile& rMappedFile);
#else
	bool Get(CMappedFile& rMappedFile, const char* c_szFileName, LPCVOID* pData);
	bool GetFromPack(CMappedFile& rMappedFile, const char* c_szFileName, LPCVOID* pData);
#endif

	//THEMIDA
	bool GetFromFile(CMappedFile& rMappedFile, const char* c_szFileName, LPCVOID* pData);
	bool isExist(const char* c_szFileName);
	bool isExistInPack(const char* c_szFileName);

	bool RegisterPack(const char* c_szName, const char* c_szDirectory);
	void RegisterRootPack(const char* c_szName);
	bool RegisterPackWhenPackMaking(const char* c_szName, const char* c_szDirectory, CEterPack* pPack);

	const char* GetRootPackFileName();

public:
	void ArrangeMemoryMappedPack();

protected:
	int ConvertFileName(const char* c_szFileName, std::string& rstrFileName);
	bool CompareName(const char* c_szDirectoryName, DWORD iLength, const char* c_szFileName);

	CEterPack* FindPack(const char* c_szPathName);

	SCache* __FindCache(DWORD dwFileNameHash);
	void	__ClearCacheMap();

protected:
	bool					m_bTryRelativePath;
	bool					m_isCacheMode;
	int						m_iSearchMode;

	CEterFileDict			m_FileDict;
	CEterPack				m_RootPack;
	TEterPackList			m_PackList;
	TEterPackMap			m_PackMap;
	TEterPackMap			m_DirPackMap;
	boost::unordered_map<DWORD, SCache> m_kMap_dwNameKey_kCache;

	CRITICAL_SECTION		m_csFinder;
};
