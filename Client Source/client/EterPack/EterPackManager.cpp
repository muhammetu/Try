#include "StdAfx.h"

#include <io.h>
#include <assert.h>

#include "EterPackManager.h"

#include "../eterBase/Debug.h"
#include "../eterBase/CRC32.h"

CEterPack* CEterPackManager::FindPack(const char* c_szPathName)
{
	std::string strFileName;

	if (0 == ConvertFileName(c_szPathName, strFileName))
	{
		return &m_RootPack;
	}
	else
	{
		for (TEterPackMap::iterator itor = m_DirPackMap.begin(); itor != m_DirPackMap.end(); ++itor)
		{
			const std::string& c_rstrName = itor->first;
			CEterPack* pEterPack = itor->second;

			if (CompareName(c_rstrName.c_str(), c_rstrName.length(), strFileName.c_str()))
			{
				return pEterPack;
			}
		}
	}

	return NULL;
}

void CEterPackManager::SetCacheMode()
{
	m_isCacheMode = true;
}

void CEterPackManager::SetRelativePathMode()
{
	m_bTryRelativePath = true;
}

int CEterPackManager::ConvertFileName(const char* c_szFileName, std::string& rstrFileName)
{
	rstrFileName = c_szFileName;
	stl_lowers(rstrFileName);

	int iCount = 0;

	for (DWORD i = 0; i < rstrFileName.length(); ++i)
	{
		if (rstrFileName[i] == '/')
			++iCount;
		else if (rstrFileName[i] == '\\')
		{
			rstrFileName[i] = '/';
			++iCount;
		}
	}

	return iCount;
}

bool CEterPackManager::CompareName(const char* c_szDirectoryName, DWORD /*dwLength*/, const char* c_szFileName)
{
	const char* c_pszSrc = c_szDirectoryName;
	const char* c_pszCmp = c_szFileName;

	while (*c_pszSrc)
	{
		if (*(c_pszSrc++) != *(c_pszCmp++))
			return false;

		if (!*c_pszCmp)
			return false;
	}

	return true;
}

void CEterPackManager::LoadStaticCache(const char* c_szFileName)
{
	if (!m_isCacheMode)
		return;

	std::string strFileName;
	if (0 == ConvertFileName(c_szFileName, strFileName))
	{
		return;
	}

	DWORD dwFileNameHash = GetCRC32(strFileName.c_str(), strFileName.length());

	boost::unordered_map<DWORD, SCache>::iterator f = m_kMap_dwNameKey_kCache.find(dwFileNameHash);
	if (m_kMap_dwNameKey_kCache.end() != f)
		return;

	CMappedFile kMapFile;
	const void* c_pvData;
	if (!Get(kMapFile, c_szFileName, &c_pvData))
		return;

	SCache kNewCache;
	kNewCache.m_dwBufSize = kMapFile.Size();
	kNewCache.m_abBufData = new BYTE[kNewCache.m_dwBufSize];
	memcpy(kNewCache.m_abBufData, c_pvData, kNewCache.m_dwBufSize);
	m_kMap_dwNameKey_kCache.insert(boost::unordered_map<DWORD, SCache>::value_type(dwFileNameHash, kNewCache));
}

CEterPackManager::SCache* CEterPackManager::__FindCache(DWORD dwFileNameHash)
{
	boost::unordered_map<DWORD, SCache>::iterator f = m_kMap_dwNameKey_kCache.find(dwFileNameHash);
	if (m_kMap_dwNameKey_kCache.end() == f)
		return NULL;

	return &f->second;
}

void	CEterPackManager::__ClearCacheMap()
{
	boost::unordered_map<DWORD, SCache>::iterator i;

	for (i = m_kMap_dwNameKey_kCache.begin(); i != m_kMap_dwNameKey_kCache.end(); ++i)
		delete[] i->second.m_abBufData;

	m_kMap_dwNameKey_kCache.clear();
}

struct TimeChecker
{
	TimeChecker(const char* name) : name(name)
	{
		baseTime = timeGetTime();
	}
	~TimeChecker()
	{
		printf("load %s (%d)\n", name, timeGetTime() - baseTime);
	}

	const char* name;
	DWORD baseTime;
};

#ifdef USE_EPK_NEW_SIGNATURE
bool CEterPackManager::_Get(const char* c_szFileName, int, int, int, int, const int pass, LPCVOID* pData, CMappedFile& rMappedFile)
{
	if (USE_EPK_NEW_SIGNATURE != pass)
		return true;
#else
bool CEterPackManager::Get(CMappedFile & rMappedFile, const char* c_szFileName, LPCVOID * pData)
{
#endif

	if (m_iSearchMode == SEARCH_PACK_FIRST)
	{
		if (GetFromPack(rMappedFile, c_szFileName, pData))
			return true;
		
		if (c_szFileName[1] != ':' && GetFromFile(rMappedFile, c_szFileName, pData))
		{
			TraceError("%s", c_szFileName); // only for log. it's not an error.
			return true;
		}
	}
	
	if (m_iSearchMode == SEARCH_FILE_FIRST)
	{
		if (GetFromFile(rMappedFile, c_szFileName, pData))
			return true;
		
		return GetFromPack(rMappedFile, c_szFileName, pData);
	}
	
	return false;
}

struct FinderLock
{
	FinderLock(CRITICAL_SECTION& cs) : p_cs(&cs)
	{
		EnterCriticalSection(p_cs);
	}

	~FinderLock()
	{
		LeaveCriticalSection(p_cs);
	}

	CRITICAL_SECTION* p_cs;
};
#ifdef USE_EPK_NEW_SIGNATURE
bool CEterPackManager::_GetFromPack(LPCVOID * pData, int, int, int, int, const int pass, const char* c_szFileName, CMappedFile & rMappedFile)
{
	if (USE_EPK_NEW_SIGNATURE != pass)
		return true;
#else
bool CEterPackManager::GetFromPack(CMappedFile & rMappedFile, const char* c_szFileName, LPCVOID * pData)
{
#endif
	FinderLock lock(m_csFinder);

	static std::string strFileName;

	if (0 == ConvertFileName(c_szFileName, strFileName))
	{
		return m_RootPack.Get(rMappedFile, strFileName.c_str(), pData);
	}
	else
	{
		DWORD dwFileNameHash = GetCRC32(strFileName.c_str(), strFileName.length());
		SCache* pkCache = __FindCache(dwFileNameHash);

		if (pkCache)
		{
			rMappedFile.Link(pkCache->m_dwBufSize, pkCache->m_abBufData);
			return true;
		}

		CEterFileDict::Item* pkFileItem = m_FileDict.GetItem(dwFileNameHash, strFileName.c_str());

		if (pkFileItem)
			if (pkFileItem->pkPack)
			{
				bool r = pkFileItem->pkPack->Get2(rMappedFile, strFileName.c_str(), pkFileItem->pkInfo, pData);
				//pkFileItem->pkPack->ClearDataMemoryMap();
				return r;
			}
	}
#ifdef _DEBUG
	TraceError("CANNOT_FIND_PACK_FILE [%s]", strFileName.c_str());
#endif

	return false;
}

const time_t g_tCachingInterval = 10;
void CEterPackManager::ArrangeMemoryMappedPack()
{
	//time_t curTime = time(NULL);
	//CEterFileDict::TDict dict = m_FileDict.GetDict();
	//for (CEterFileDict::TDict::iterator it = dict.begin(); it != dict.end(); ++it)
	//{
	//	CEterFileDict::Item &rFileItem = it->second;
	//	CEterPack* pkPack = rFileItem.pkPack;
	//	if (pkPack)
	//	{
	//		if (curTime - pkPack->GetLastAccessTime() > g_tCachingInterval)
	//		{
	//			pkPack->ClearDataMemoryMap();
	//		}
	//	}
	//}
}

bool CEterPackManager::GetFromFile(CMappedFile & rMappedFile, const char* c_szFileName, LPCVOID * pData)
{
#ifdef ENABLE_PREVENT_FILE_READ_FROM_DRIVE
	if (strlen(c_szFileName) > 2 && c_szFileName[1] == ':')
		return false;
#endif
	return rMappedFile.Create(c_szFileName, pData, 0, 0) ? true : false;
}

bool CEterPackManager::isExistInPack(const char* c_szFileName)
{
	std::string strFileName;

	if (0 == ConvertFileName(c_szFileName, strFileName))
	{
		return m_RootPack.IsExist(strFileName.c_str());
	}
	else
	{
		DWORD dwFileNameHash = GetCRC32(strFileName.c_str(), strFileName.length());
		CEterFileDict::Item* pkFileItem = m_FileDict.GetItem(dwFileNameHash, strFileName.c_str());

		if (pkFileItem)
			if (pkFileItem->pkPack)
				return pkFileItem->pkPack->IsExist(strFileName.c_str());
	}

	return false;
}

bool CEterPackManager::isExist(const char* c_szFileName)
{
	return isExistInPack(c_szFileName);
}

void CEterPackManager::RegisterRootPack(const char* c_szName)
{
	if (!m_RootPack.Create(m_FileDict, c_szName, ""))
	{
		TraceError("%s: Pack file does not exist", c_szName);
	}
}

const char* CEterPackManager::GetRootPackFileName()
{
	return m_RootPack.GetDBName();
}

bool CEterPackManager::RegisterPackWhenPackMaking(const char* c_szName, const char* c_szDirectory, CEterPack * pPack)
{
	m_PackMap.insert(TEterPackMap::value_type(c_szName, pPack));
	m_PackList.push_front(pPack);

	m_DirPackMap.insert(TEterPackMap::value_type(c_szDirectory, pPack));
	return true;
}

bool CEterPackManager::RegisterPack(const char* c_szName, const char* c_szDirectory)
{
	CEterPack* pEterPack = NULL;
	{
		TEterPackMap::iterator itor = m_PackMap.find(c_szName);

		if (m_PackMap.end() == itor)
		{
			bool bReadOnly = true;

			pEterPack = new CEterPack;
			if (pEterPack->Create(m_FileDict, c_szName, c_szDirectory, bReadOnly))
			{
				m_PackMap.insert(TEterPackMap::value_type(c_szName, pEterPack));
			}
			else
			{
#ifdef _DEBUG
				Tracef("The eterpack doesn't exist [%s]\n", c_szName);
#endif
				delete pEterPack;
				pEterPack = NULL;
				return false;
			}
		}
		else
		{
			pEterPack = itor->second;
		}
	}

	if (c_szDirectory && c_szDirectory[0] != '*')
	{
		TEterPackMap::iterator itor = m_DirPackMap.find(c_szDirectory);
		if (m_DirPackMap.end() == itor)
		{
			m_PackList.push_front(pEterPack);
			m_DirPackMap.insert(TEterPackMap::value_type(c_szDirectory, pEterPack));
		}
	}

	return true;
}

void CEterPackManager::SetSearchMode(bool bPackFirst)
{
	m_iSearchMode = bPackFirst ? SEARCH_PACK_FIRST : SEARCH_FILE_FIRST;
}

int CEterPackManager::GetSearchMode()
{
	return m_iSearchMode;
}

CEterPackManager::CEterPackManager() : m_bTryRelativePath(false), m_iSearchMode(SEARCH_FILE_FIRST), m_isCacheMode(false)
{
	InitializeCriticalSection(&m_csFinder);
}

CEterPackManager::~CEterPackManager()
{
	__ClearCacheMap();

	TEterPackMap::iterator i = m_PackMap.begin();
	TEterPackMap::iterator e = m_PackMap.end();
	while (i != e)
	{
		delete i->second;
		i++;
	}
	DeleteCriticalSection(&m_csFinder);
}

