#include "StdAfx.h"
#if defined(ENABLE_GROWTH_MOUNT_SYSTEM)
#include "PythonSkillMount.h"

#include "../EterBase/Poly/Poly.h"
#include "../EterPack/EterPackManager.h"
#include "InstanceBase.h"
#include "PythonPlayer.h"

bool CPythonSkillMount::RegisterSkillMount(const char* c_szFileName)
{
	const VOID* pvData;
	CMappedFile kFile;
	if (!CEterPackManager::Instance().Get(kFile, c_szFileName, &pvData))
		return false;

	CMemoryTextFileLoader textFileLoader;
	textFileLoader.Bind(kFile.Size(), pvData);

	CTokenVector TokenVector;
	for (DWORD i = 0; i < textFileLoader.GetLineCount() - 1; ++i)
	{
		if (!textFileLoader.SplitLine(i, &TokenVector, "\t"))
			return false;
		if (DESCMOUNT_TOKEN_TYPE_MAX_NUM > TokenVector.size())
			return false;
		DWORD iSkillIndex = atoi(TokenVector[DESCMOUNT_TOKEN_TYPE_VNUM].c_str());
		if (iSkillIndex == 0)
			continue;

		m_SkillDataMountMap.insert(make_pair(iSkillIndex, TSkillDataMount()));

		TSkillDataMount& rSkillData = m_SkillDataMountMap[iSkillIndex];

		rSkillData.dwSkillIndex = iSkillIndex;

		rSkillData.strName = TokenVector[DESCMOUNT_TOKEN_TYPE_NAME];

		char szIconFileNameHeader[64 + 1];
		_snprintf(szIconFileNameHeader, sizeof(szIconFileNameHeader), "%sskill/pet/%s.sub", g_strImagePath.c_str(), TokenVector[DESCMOUNT_TOKEN_TYPE_ICON_NAME].c_str());
		//TraceError("%s", szIconFileNameHeader);
		rSkillData.strIconFileName = szIconFileNameHeader;

		std::map<std::string, DWORD>::iterator itor = m_SkillMountTypeIndexMap.begin();
		for (; itor != m_SkillMountTypeIndexMap.end(); ++itor) {
			if (TokenVector[DESCMOUNT_TOKEN_TYPE].compare(itor->first) == 0)
				rSkillData.byType = itor->second;
		}
		rSkillData.strDescription = TokenVector[DESCMOUNT_TOKEN_TYPE_DESCRIPTION];
		rSkillData.dwskilldelay = atoi(TokenVector[DESCMOUNT_TOKEN_TYPE_DELAY].c_str());

		rSkillData.pImage = (CGraphicImage*)CResourceManager::Instance().GetResourcePointer(szIconFileNameHeader);
	}

	return true;
}

void CPythonSkillMount::__RegisterNormalIconImage(TSkillDataMount& rData, const char* c_szHeader, const char* c_szImageName)
{
	std::string strFileName = "";
	strFileName += c_szHeader;
	strFileName += c_szImageName;
	strFileName += ".sub";
	rData.pImage = (CGraphicImage*)CResourceManager::Instance().GetResourcePointer(strFileName.c_str());
}
void CPythonSkillMount::Destroy()
{
	m_SkillDataMountMap.clear();
}

CPythonSkillMount::SSkillDataMount::SSkillDataMount()
{
	dwSkillIndex = 0;
	strName = "";
	strIconFileName = "";
	byType = 0;
	strDescription = "";
	dwskilldelay = 0;
	pImage = NULL;
}

CPythonSkillMount::CPythonSkillMount()
{
	mountslot[0] = 0;
	mountslot[1] = 0;
	mountslot[2] = 0;

	m_SkillMountTypeIndexMap.insert(std::map<std::string, DWORD>::value_type("EMPTY", SKILLMOUNT_TYPE_NONE));
	m_SkillMountTypeIndexMap.insert(std::map<std::string, DWORD>::value_type("PASSIVE", SKILLMOUNT_TYPE_PASSIVE));
	m_SkillMountTypeIndexMap.insert(std::map<std::string, DWORD>::value_type("AUTO", SKILLMOUNT_TYPE_AUTO));
}
CPythonSkillMount::~CPythonSkillMount()
{
}

void CPythonSkillMount::SetSkillbySlot(int slot, int skillIndex)
{
	//TraceError("Setto lo slot %d --> skill %d", slot, skillIndex);
	mountslot[slot] = skillIndex;
}

BOOL CPythonSkillMount::GetSkillData(DWORD dwSkillIndex, TSkillDataMount** ppSkillData)
{
	TSkillDataMountMap::iterator it = m_SkillDataMountMap.find(dwSkillIndex);

	if (m_SkillDataMountMap.end() == it)
		return FALSE;

	*ppSkillData = &(it->second);
	return TRUE;
}

BOOL CPythonSkillMount::GetSkillIndex(int slot, int* skillIndex)
{
	*skillIndex = mountslot[slot];
	return TRUE;
}

PyObject* mountskillSetSkillSlot(PyObject* poSelf, PyObject* poArgs)
{
	int slot;
	int skillIndex;

	if (!PyTuple_GetInteger(poArgs, 0, &slot))
		return Py_BadArgument();
	if (!PyTuple_GetInteger(poArgs, 1, &skillIndex))
		return Py_BadArgument();
	CPythonSkillMount::Instance().SetSkillbySlot(slot, skillIndex);
	return Py_BuildValue("i", 0);
}

PyObject* mountskillGetIconImage(PyObject* poSelf, PyObject* poArgs)
{
	int iSkillIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iSkillIndex))
		return Py_BadArgument();

	CPythonSkillMount::SSkillDataMount* c_pSkillData;
	if (!CPythonSkillMount::Instance().GetSkillData(iSkillIndex, &c_pSkillData))
		return Py_BuildValue("i", 0);

	return Py_BuildValue("i", c_pSkillData->pImage);
}

PyObject* mountskillGetSkillbySlot(PyObject* poSelf, PyObject* poArgs)
{
	int slot;
	int skillIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &slot))
		return Py_BadArgument();

	if (slot > 2)
		return Py_BadArgument();

	if (!CPythonSkillMount::Instance().GetSkillIndex(slot, &skillIndex))
		return Py_BuildValue("i", 0);

	return Py_BuildValue("i", skillIndex);
}

PyObject* mountskillGetEmptySkill(PyObject* poSelf, PyObject* poArgs)
{
	CGraphicImage* noskillbtn;
	char szIconFileNameHeader[64 + 1];
	_snprintf(szIconFileNameHeader, sizeof(szIconFileNameHeader), "d:/ymir work/ui/pet/skill_button/skill_enable_button.sub");
	noskillbtn = (CGraphicImage*)CResourceManager::Instance().GetResourcePointer(szIconFileNameHeader);
	return Py_BuildValue("i", noskillbtn);
}

void initskillmount()
{
	static PyMethodDef s_methods[] =
	{
		///Variabili setting //
		{ "SetSkillSlot",					mountskillSetSkillSlot,						METH_VARARGS },

		//Variabili Get //

		{ "GetIconImage",					mountskillGetIconImage,						METH_VARARGS },
		{ "GetSkillbySlot",					mountskillGetSkillbySlot ,					METH_VARARGS },
		{ "GetEmptySkill",					mountskillGetEmptySkill,						METH_VARARGS },

		{ NULL,										NULL,										NULL },
	};

	PyObject* poModule = Py_InitModule("mountskill", s_methods);
}
#endif