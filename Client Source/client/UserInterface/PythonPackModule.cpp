#include "StdAfx.h"
#include "../eterPack/EterPackManager.h"
#include "../eterBase/tea.h"

// CHINA_CRYPT_KEY
DWORD g_adwEncryptKey[4];
DWORD g_adwDecryptKey[4];

#include "AccountConnector.h"

inline const BYTE* GetKey_20050304Myevan()
{
	volatile static DWORD s_adwKey[1938];

	volatile DWORD seed = 1491971513;
	for (UINT i = 0; i < BYTE(seed); i++)
	{
		seed ^= 2148941891;
		seed += 3592385981;
		s_adwKey[i] = seed;
	}

	return (const BYTE*)s_adwKey;
}

//#include <eterCrypt.h>

void CAccountConnector::__BuildClientKey_20050304Myevan()
{
	const BYTE* c_pszKey = GetKey_20050304Myevan();
	memcpy(g_adwEncryptKey, c_pszKey + 157, 16);

	for (DWORD i = 0; i < 4; ++i)
		g_adwEncryptKey[i] = random();

	tea_encrypt((DWORD*)g_adwDecryptKey, (const DWORD*)g_adwEncryptKey, (const DWORD*)(c_pszKey + 37), 16);
	//	TEA_Encrypt((DWORD *) g_adwDecryptKey, (const DWORD *) g_adwEncryptKey, (const DWORD *) (c_pszKey+37), 16);
}
// END_OF_CHINA_CRYPT_KEY

PyObject* packExist(PyObject* poSelf, PyObject* poArgs)
{
	char* strFileName;

	if (!PyTuple_GetString(poArgs, 0, &strFileName))
		return Py_BuildException();

	return Py_BuildValue("i", CEterPackManager::Instance().isExist(strFileName) ? 1 : 0);
}

#ifdef ENABLE_PACK_IMPORT_MODULE
#	include <msl/utils.h>

static PyObject* packImportModule(PyObject* poSelf, PyObject* poArgs)
{
	char* str;
	char* filename;
	int mode = 0; // 0:"exec"
	int dont_inherit = 0;
	int supplied_flags = 0;
	PyCompilerFlags cf;
	PyObject* result = NULL, * tmp = NULL;
	Py_ssize_t length;
	int start[] = { Py_file_input, Py_eval_input, Py_single_input };

	if (!PyTuple_GetString(poArgs, 0, &filename))
		return Py_BuildException();

	cf.cf_flags = supplied_flags;
	PyEval_MergeCompilerFlags(&cf);

	CMappedFile file;
	const void* pData = nullptr;
	if (CEterPackManager::Instance().Get(file, filename, &pData))
	{
		/* Copy to NUL-terminated buffer. */
		tmp = PyString_FromStringAndSize((const char*)pData, file.Size());
		if (tmp == NULL)
			return NULL;
		str = PyString_AS_STRING(tmp);
		length = PyString_GET_SIZE(tmp);
	}
	else
	{
		std::string msg = "pack.importModule() filename not found: ";
		msg += filename;
		PyErr_SetString(PyExc_TypeError, msg.c_str());
		goto cleanup;
	}

	if ((size_t)length != strlen(str))
	{
		PyErr_SetString(PyExc_TypeError, "pack.importModule() expected string without null bytes");
		goto cleanup;
	}

	result = Py_CompileStringFlags(str, filename, start[mode], &cf);
cleanup:
	Py_XDECREF(tmp);
	return result;
}

#	include "../GameLib/RaceManager.h"
static PyObject* packLoadNpcList(PyObject* poSelf, PyObject* poArgs)
{
	char* filename;
	if (!PyTuple_GetString(poArgs, 0, &filename))
		return Py_BuildException();

	const VOID* pvData;
	CMappedFile kFile;
	if (!CEterPackManager::Instance().Get(kFile, filename, &pvData))
	{
		TraceError("packLoadNpcList(c_szFileName=%s) - Load Error", filename);
		return Py_BuildException();
	}

	std::string fileData((const char*)pvData, kFile.Size());
	const auto& lines = msl::string_split_any(fileData, "\r\n");
	for (const auto& line : lines)
	{
		auto& kTokenVector = msl::string_split(line, "\t");
		for (auto& token : kTokenVector)
			msl::trim_in_place(token);

		if (kTokenVector.size() < 2)
			continue;

		uint32_t vnum = atoi(kTokenVector[0].c_str());
		if (vnum)
			CRaceManager::Instance().RegisterRaceName(vnum, kTokenVector[1].c_str());
		else if (kTokenVector.size() >= 3)
			CRaceManager::Instance().RegisterRaceSrcName(kTokenVector[1].c_str(), kTokenVector[2].c_str());
		else
			TraceError("packLoadNpcList(c_szFileName=%s) - Line Error %s %s", filename, kTokenVector[0].c_str(), kTokenVector[1].c_str());
	}
	return Py_BuildNone();
}

static PyObject* packLoadLocaleInterface(PyObject* poSelf, PyObject* poArgs)
{
	char* filename;
	PyObject* localeDict;
	if (!PyTuple_GetString(poArgs, 0, &filename))
		return Py_BuildException();
	if (!PyTuple_GetObject(poArgs, 1, &localeDict))
		return Py_BuildException();

	const VOID* pvData;
	CMappedFile kFile;
	if (!CEterPackManager::Instance().Get(kFile, filename, &pvData))
	{
		TraceError("packLoadLocaleInterface(c_szFileName=%s) - Load Error", filename);
		return Py_BuildException();
	}

	std::string fileData((const char*)pvData, kFile.Size());
	const auto& lines = msl::string_split_any(fileData, "\r\n");
	for (const auto& line : lines)
	{
		auto& kTokenVector = msl::string_split(line, "\t");
		for (auto& token : kTokenVector)
			msl::trim_in_place(token);

		if (kTokenVector.size() < 2)
			continue;

		PyDict_SetItemString(localeDict, kTokenVector[0].c_str(), PyString_FromString(kTokenVector[1].c_str()));
	}
	return Py_BuildNone();
}

static PyObject* packLoadLocaleGame(PyObject* poSelf, PyObject* poArgs)
{
	char* filename;
	PyObject* localeDict;
	PyObject* funcDict;
	if (!PyTuple_GetString(poArgs, 0, &filename))
		return Py_BuildException();
	if (!PyTuple_GetObject(poArgs, 1, &localeDict))
		return Py_BuildException();
	if (!PyTuple_GetObject(poArgs, 2, &funcDict))
		return Py_BuildException();

	const VOID* pvData;
	CMappedFile kFile;
	if (!CEterPackManager::Instance().Get(kFile, filename, &pvData))
	{
		TraceError("packLoadLocaleGame(c_szFileName=%s) - Load Error", filename);
		return Py_BuildException();
	}

	std::string fileData((const char*)pvData, kFile.Size());
	const auto& lines = msl::string_split_any(fileData, "\r\n");
	for (const auto& line : lines)
	{
		auto& kTokenVector = msl::string_split(line, "\t");
		for (auto& token : kTokenVector)
			msl::trim_in_place(token);

		if (kTokenVector.size() < 2) // skip empty or incomplete
			continue;

		PyObject* itemValue = nullptr;

		// if 3 tabs, key0=func[type2](value1)
		if (kTokenVector.size() > 2 && !kTokenVector[2].empty())
		{
			PyObject* funcType = PyDict_GetItemString(funcDict, kTokenVector[2].c_str());
			if (funcType)
				itemValue = PyObject_CallFunction(funcType, "s#", kTokenVector[1].c_str(), kTokenVector[1].size());
			else
			{
				TraceError("packLoadLocaleGame(c_szFileName=%s, funcDict=%s) - Tag Error %d", filename, kTokenVector[2].c_str(),
					kTokenVector.size());
				return Py_BuildException();
			}
		}
		else
			itemValue = PyString_FromString(kTokenVector[1].c_str());

		// if 2 tabs, key0=value1
		PyDict_SetItemString(localeDict, kTokenVector[0].c_str(), itemValue);
	}
	return Py_BuildNone();
}

#ifdef ENABLE_TOOLTIP_DROP_INFORMATION
static PyObject* packLoadDropInformation(PyObject* poSelf, PyObject* poArgs)
{
	char* filename;
	PyObject* localeDict;
	if (!PyTuple_GetString(poArgs, 0, &filename))
		return Py_BuildException();
	if (!PyTuple_GetObject(poArgs, 1, &localeDict))
		return Py_BuildException();

	const VOID* pvData;
	CMappedFile kFile;
	if (!CEterPackManager::Instance().Get(kFile, filename, &pvData))
	{
		TraceError("packLoadDropInformation(c_szFileName=%s) - Load Error", filename);
		return Py_BuildException();
	}

	std::string fileData((const char*)pvData, kFile.Size());
	const auto& lines = msl::string_split_any(fileData, "\r\n");
	for (const auto& line : lines)
	{
		auto& kTokenVector = msl::string_split(line, "\t");
		for (auto& token : kTokenVector)
			msl::trim_in_place(token);

		if (kTokenVector.size() < 2)
			continue;

		PyDict_SetItemString(localeDict, line.c_str(), PyString_FromString(kTokenVector[1].c_str()));
	}
	return Py_BuildNone();
}
#endif

#ifdef ENABLE_GROWTH_PET_SYSTEM
static std::vector<std::string> petSkillTable;
static std::vector<std::string> petSkillBonusTable;

static PyObject* packLoadPetSkillTable(PyObject* poSelf, PyObject* poArgs)
{
	char* filename;
	if (!PyTuple_GetString(poArgs, 0, &filename))
		return Py_BuildException();

	const VOID* pvData;
	CMappedFile kFile;
	if (!CEterPackManager::Instance().Get(kFile, filename, &pvData))
	{
		TraceError("packLoadPetSkillTable(c_szFileName=%s) - Load Error", filename);
		return Py_BuildException();
	}

	std::string fileData((const char*)pvData, kFile.Size());
	const auto& lines = msl::string_split_any(fileData, "\r\n");
	for (const auto& line : lines)
	{
		auto& kTokenVector = msl::string_split(line, "\t");
		for (auto& token : kTokenVector)
			msl::trim_in_place(token);

		if (kTokenVector.size() < 6)
			continue;

		petSkillTable.push_back(line);
	}
	return Py_BuildNone();
}

static PyObject* packGetPetSkillTableSize(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", petSkillTable.size());
}

static PyObject* packGetPetSkillTableItem(PyObject* poSelf, PyObject* poArgs)
{
	int index = 0;
	if (!PyTuple_GetInteger(poArgs, 0, &index))
		return Py_BuildException();
	
	return Py_BuildValue("s", petSkillTable.at(index).c_str());
}

static PyObject* packClearPetSkillTable(PyObject* poSelf, PyObject* poArgs)
{
	petSkillTable.clear();
	return Py_BuildNone();
}

static PyObject* packLoadPetSkillBonusTable(PyObject* poSelf, PyObject* poArgs)
{
	char* filename;
	if (!PyTuple_GetString(poArgs, 0, &filename))
		return Py_BuildException();

	const VOID* pvData;
	CMappedFile kFile;
	if (!CEterPackManager::Instance().Get(kFile, filename, &pvData))
	{
		TraceError("packLoadPetSkillBonusTable(c_szFileName=%s) - Load Error", filename);
		return Py_BuildException();
	}

	std::string fileData((const char*)pvData, kFile.Size());
	const auto& lines = msl::string_split_any(fileData, "\r\n");
	for (const auto& line : lines)
	{
		auto& kTokenVector = msl::string_split(line, "\t");
		for (auto& token : kTokenVector)
			msl::trim_in_place(token);

		if (kTokenVector.size() < 22)
			continue;

		petSkillBonusTable.push_back(line);
	}
	return Py_BuildNone();
}

static PyObject* packGetPetSkillBonusTableSize(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", petSkillBonusTable.size());
}

static PyObject* packGetPetSkillBonusTableItem(PyObject* poSelf, PyObject* poArgs)
{
	int index = 0;
	if (!PyTuple_GetInteger(poArgs, 0, &index))
		return Py_BuildException();
	
	return Py_BuildValue("s", petSkillBonusTable.at(index).c_str());
}

static PyObject* packClearPetSkillBonusTable(PyObject* poSelf, PyObject* poArgs)
{
	petSkillBonusTable.clear();
	return Py_BuildNone();
}
#endif

#ifdef ENABLE_GROWTH_MOUNT_SYSTEM
static std::vector<std::string> mountSkillTable;
static std::vector<std::string> mountSkillBonusTable;

static PyObject* packLoadMountSkillTable(PyObject* poSelf, PyObject* poArgs)
{
	char* filename;
	if (!PyTuple_GetString(poArgs, 0, &filename))
		return Py_BuildException();

	const VOID* pvData;
	CMappedFile kFile;
	if (!CEterPackManager::Instance().Get(kFile, filename, &pvData))
	{
		TraceError("packLoadMountSkillTable(c_szFileName=%s) - Load Error", filename);
		return Py_BuildException();
	}

	std::string fileData((const char*)pvData, kFile.Size());
	const auto& lines = msl::string_split_any(fileData, "\r\n");
	for (const auto& line : lines)
	{
		auto& kTokenVector = msl::string_split(line, "\t");
		for (auto& token : kTokenVector)
			msl::trim_in_place(token);

		if (kTokenVector.size() < 6)
			continue;

		mountSkillTable.push_back(line);
	}
	return Py_BuildNone();
}

static PyObject* packGetMountSkillTableSize(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", mountSkillTable.size());
}

static PyObject* packGetMountSkillTableItem(PyObject* poSelf, PyObject* poArgs)
{
	int index = 0;
	if (!PyTuple_GetInteger(poArgs, 0, &index))
		return Py_BuildException();
	
	return Py_BuildValue("s", mountSkillTable.at(index).c_str());
}

static PyObject* packClearMountSkillTable(PyObject* poSelf, PyObject* poArgs)
{
	mountSkillTable.clear();
	return Py_BuildNone();
}

static PyObject* packLoadMountSkillBonusTable(PyObject* poSelf, PyObject* poArgs)
{
	char* filename;
	if (!PyTuple_GetString(poArgs, 0, &filename))
		return Py_BuildException();

	const VOID* pvData;
	CMappedFile kFile;
	if (!CEterPackManager::Instance().Get(kFile, filename, &pvData))
	{
		TraceError("packLoadMountSkillBonusTable(c_szFileName=%s) - Load Error", filename);
		return Py_BuildException();
	}

	std::string fileData((const char*)pvData, kFile.Size());
	const auto& lines = msl::string_split_any(fileData, "\r\n");
	for (const auto& line : lines)
	{
		auto& kTokenVector = msl::string_split(line, "\t");
		for (auto& token : kTokenVector)
			msl::trim_in_place(token);

		if (kTokenVector.size() < 22)
			continue;

		mountSkillBonusTable.push_back(line);
	}
	return Py_BuildNone();
}

static PyObject* packGetMountSkillBonusTableSize(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", mountSkillBonusTable.size());
}

static PyObject* packGetMountSkillBonusTableItem(PyObject* poSelf, PyObject* poArgs)
{
	int index = 0;
	if (!PyTuple_GetInteger(poArgs, 0, &index))
		return Py_BuildException();
	
	return Py_BuildValue("s", mountSkillBonusTable.at(index).c_str());
}

static PyObject* packClearMountSkillBonusTable(PyObject* poSelf, PyObject* poArgs)
{
	mountSkillBonusTable.clear();
	return Py_BuildNone();
}
#endif

#ifdef ENABLE_INGAME_MALL_SYSTEM
static std::vector<std::string> itemShopLoad;
static PyObject* packLoadItemShopTable(PyObject* poSelf, PyObject* poArgs)
{
	char* filename;
	if (!PyTuple_GetString(poArgs, 0, &filename))
		return Py_BuildException();

	const VOID* pvData;
	CMappedFile kFile;
	if (!CEterPackManager::Instance().Get(kFile, filename, &pvData))
	{
		TraceError("packLoadItemShopTable(c_szFileName=%s) - Load Error", filename);
		return Py_BuildException();
	}

	std::string fileData((const char*)pvData, kFile.Size());
	const auto& lines = msl::string_split(fileData, "\r\n");
	for (const auto& line : lines)
	{
		auto& kTokenVector = msl::string_split(line, "\t");
		for (auto& token : kTokenVector)
			msl::trim_in_place(token);

		if (kTokenVector.size() < 7)
			continue;

		itemShopLoad.push_back(line);
	}
	return Py_BuildNone();
}

static PyObject* packGetItemShopTableSize(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", itemShopLoad.size());
}

static PyObject* packGetItemShopTableItem(PyObject* poSelf, PyObject* poArgs)
{
	int index = 0;
	if (!PyTuple_GetInteger(poArgs, 0, &index))
		return Py_BuildException();

	return Py_BuildValue("s", itemShopLoad.at(index).c_str());
}

static PyObject* packClearItemShopTable(PyObject* poSelf, PyObject* poArgs)
{
	itemShopLoad.clear();
	return Py_BuildNone();
}
#endif

#else

PyObject* packGet(PyObject* poSelf, PyObject* poArgs)
{
	char* strFileName;

	if (!PyTuple_GetString(poArgs, 0, &strFileName))
		return Py_BuildException();

	// used for python and txt data
	const char* pcExt = strrchr(strFileName, '.');
	if (pcExt) // simple extension check
	{
		// @duzenleme cekilen dosya py pyc veya txt mi diye kontrol eklenmis.
		if ((stricmp(pcExt, ".py") == 0) ||
			(stricmp(pcExt, ".pyc") == 0) ||
			(stricmp(pcExt, ".txt") == 0))
		{
			CMappedFile file;
			const void* pData = NULL;

			if (CEterPackManager::Instance().Get(file, strFileName, &pData))
				return Py_BuildValue("s#", pData, file.Size());
		}
	}

	return Py_BuildException();
}

#endif

void initpack()
{
	static PyMethodDef s_methods[] = { {"Exist", packExist, METH_VARARGS},
#ifdef ENABLE_PACK_IMPORT_MODULE
									  {"importModule", packImportModule, METH_VARARGS},
									  {"loadLocaleGame", packLoadLocaleGame, METH_VARARGS},
									  {"loadLocaleInterface", packLoadLocaleInterface, METH_VARARGS},
									  {"loadNpcList", packLoadNpcList, METH_VARARGS},
#ifdef ENABLE_TOOLTIP_DROP_INFORMATION
									  {"loadDropInformation", packLoadDropInformation, METH_VARARGS},
#endif
#ifdef ENABLE_GROWTH_PET_SYSTEM
									  {"loadPetSkillTable", packLoadPetSkillTable, METH_VARARGS},
									  {"GetPetSkillTableSize", packGetPetSkillTableSize, METH_VARARGS},
									  {"GetPetSkillTableItem", packGetPetSkillTableItem,			 METH_VARARGS},
									  {"ClearPetSkillTable", packClearPetSkillTable, 			METH_VARARGS},
									  {"loadPetSkillBonusTable", packLoadPetSkillBonusTable,			METH_VARARGS},
									  {"GetPetSkillBonusTableSize", packGetPetSkillBonusTableSize, 			METH_VARARGS},
									  {"GetPetSkillBonusTableItem", packGetPetSkillBonusTableItem,			METH_VARARGS},
									  {"ClearPetSkillBonusTable", packClearPetSkillBonusTable, METH_VARARGS},
#endif
#ifdef ENABLE_GROWTH_MOUNT_SYSTEM
									  {"loadMountSkillTable", packLoadMountSkillTable, METH_VARARGS},
									  {"GetMountSkillTableSize", packGetMountSkillTableSize, METH_VARARGS},
									  {"GetMountSkillTableItem", packGetMountSkillTableItem,			 METH_VARARGS},
									  {"ClearMountSkillTable", packClearMountSkillTable, 			METH_VARARGS},
									  {"loadMountSkillBonusTable", packLoadMountSkillBonusTable,			METH_VARARGS},
									  {"GetMountSkillBonusTableSize", packGetMountSkillBonusTableSize, 			METH_VARARGS},
									  {"GetMountSkillBonusTableItem", packGetMountSkillBonusTableItem,			METH_VARARGS},
									  {"ClearMountSkillBonusTable", packClearMountSkillBonusTable, METH_VARARGS},
#endif
#ifdef ENABLE_INGAME_MALL_SYSTEM
									  {"loadItemShopTable", packLoadItemShopTable,	METH_VARARGS},
									  {"GetItemShopTableSize", packGetItemShopTableSize, 			METH_VARARGS},
									  {"GetItemShopTableItem", packGetItemShopTableItem, 			METH_VARARGS},
									  {"ClearItemShopTable", packClearItemShopTable, 			METH_VARARGS},
#endif
#else
									  {"Get", packGet, METH_VARARGS},
#endif
									  {nullptr, nullptr} };

	PyObject* poModule = Py_InitModule("pack", s_methods);
#ifdef ENABLE_PACK_IMPORT_MODULE
	PyModule_AddIntConstant(poModule, "ENABLE_PACK_IMPORT_MODULE", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_PACK_IMPORT_MODULE", false);
#endif
}