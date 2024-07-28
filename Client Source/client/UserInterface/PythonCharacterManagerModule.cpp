#include "StdAfx.h"
#include "PythonCharacterManager.h"
#include "PythonBackground.h"
#include "InstanceBase.h"
#include "../gamelib/RaceManager.h"

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

PyObject* chrmgrSetEmpireNameMode(PyObject* poSelf, PyObject* poArgs)
{
	int	iEnable;
	if (!PyTuple_GetInteger(poArgs, 0, &iEnable))
		return Py_BadArgument();

	CInstanceBase::SetEmpireNameMode(iEnable ? true : false);
	CPythonCharacterManager::Instance().RefreshAllPCTextTail();

	return Py_BuildNone();
}

PyObject* chrmgrRegisterTitleName(PyObject* poSelf, PyObject* poArgs)
{
	int	iIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iIndex))
		return Py_BadArgument();
	char* szTitleName;
	if (!PyTuple_GetString(poArgs, 1, &szTitleName))
		return Py_BadArgument();

	CInstanceBase::RegisterTitleName(iIndex, szTitleName);
	return Py_BuildNone();
}

PyObject* chrmgrRegisterNameColor(PyObject* poSelf, PyObject* poArgs)
{
	int index;
	if (!PyTuple_GetInteger(poArgs, 0, &index))
		return Py_BadArgument();

	int ir;
	if (!PyTuple_GetInteger(poArgs, 1, &ir))
		return Py_BadArgument();
	int ig;
	if (!PyTuple_GetInteger(poArgs, 2, &ig))
		return Py_BadArgument();
	int ib;
	if (!PyTuple_GetInteger(poArgs, 3, &ib))
		return Py_BadArgument();

	CInstanceBase::RegisterNameColor(index, ir, ig, ib);
	return Py_BuildNone();
}

PyObject* chrmgrRegisterTitleColor(PyObject* poSelf, PyObject* poArgs)
{
	int	iIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iIndex))
		return Py_BadArgument();
	int ir;
	if (!PyTuple_GetInteger(poArgs, 1, &ir))
		return Py_BadArgument();
	int ig;
	if (!PyTuple_GetInteger(poArgs, 2, &ig))
		return Py_BadArgument();
	int ib;
	if (!PyTuple_GetInteger(poArgs, 3, &ib))
		return Py_BadArgument();

	CInstanceBase::RegisterTitleColor(iIndex, ir, ig, ib);
	return Py_BuildNone();
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

PyObject* chrmgrSetMovingSpeed(PyObject* poSelf, PyObject* poArgs)
{
#ifndef NDEBUG // @warme601 _DISTRIBUTE -> NDEBUG
	int	nMovSpd;
	if (!PyTuple_GetInteger(poArgs, 0, &nMovSpd))
		return Py_BadArgument();

	if (nMovSpd < 0)
		return Py_BuildException("MovingSpeed < 0");

	CInstanceBase* pkInst = CPythonCharacterManager::Instance().GetSelectedInstancePtr();

	if (!pkInst)
		return Py_BuildException("MainCharacter has not selected!");

	pkInst->SetMoveSpeed(nMovSpd);
#endif
	return Py_BuildNone();
}

PyObject* chrmgrSetDustGap(PyObject* poSelf, PyObject* poArgs)
{
	int nGap;
	if (!PyTuple_GetInteger(poArgs, 0, &nGap))
		return Py_BadArgument();

	CInstanceBase::SetDustGap(nGap);
	return Py_BuildNone();
}

PyObject* chrmgrSetHorseDustGap(PyObject* poSelf, PyObject* poArgs)
{
	int nGap;
	if (!PyTuple_GetInteger(poArgs, 0, &nGap))
		return Py_BadArgument();

	CInstanceBase::SetHorseDustGap(nGap);
	return Py_BuildNone();
}

void CPythonCharacterManager::SCRIPT_SetAffect(DWORD dwVID, DWORD eState, BOOL isVisible)
{
	CInstanceBase* pkInstSel = (dwVID == 0xffffffff) ? GetSelectedInstancePtr() : GetInstancePtr(dwVID);
	if (!pkInstSel)
		return;

	pkInstSel->SCRIPT_SetAffect(eState, isVisible ? true : false);
}

void CPythonCharacterManager::SetEmoticon(DWORD dwVID, DWORD eState)
{
	CInstanceBase* pkInstSel = (dwVID == 0xffffffff) ? GetSelectedInstancePtr() : GetInstancePtr(dwVID);
	if (!pkInstSel)
		return;

	pkInstSel->SetEmoticon(eState);
}

bool CPythonCharacterManager::IsPossibleEmoticon(DWORD dwVID)
{
	CInstanceBase* pkInstSel = (dwVID == 0xffffffff) ? GetSelectedInstancePtr() : GetInstancePtr(dwVID);
	if (!pkInstSel)
		return false;

	return pkInstSel->IsPossibleEmoticon();
}

PyObject* chrmgrSetEmoticon(PyObject* poSelf, PyObject* poArgs)
{
	int nVID;
	if (!PyTuple_GetInteger(poArgs, 0, &nVID))
		return Py_BadArgument();

	int nEft;
	if (!PyTuple_GetInteger(poArgs, 1, &nEft))
		return Py_BadArgument();

	CPythonCharacterManager& rkChrMgr = CPythonCharacterManager::Instance();
	rkChrMgr.SetEmoticon(nVID >= 0 ? nVID : 0xffffffff, nEft);
	return Py_BuildNone();
}

PyObject* chrmgrIsPossibleEmoticon(PyObject* poSelf, PyObject* poArgs)
{
	int nVID;
	if (!PyTuple_GetInteger(poArgs, 0, &nVID))
		return Py_BadArgument();

	CPythonCharacterManager& rkChrMgr = CPythonCharacterManager::Instance();
	int result = rkChrMgr.IsPossibleEmoticon(nVID >= 0 ? nVID : 0xffffffff);

	return Py_BuildValue("i", result);
}

PyObject* chrmgrIsPc(PyObject* poSelf, PyObject* poArgs)
{
	int nVID;
	if (!PyTuple_GetInteger(poArgs, 0, &nVID))
		return Py_BadArgument();

	bool result = false;
	CPythonCharacterManager& rkChrMgr = CPythonCharacterManager::Instance();
	CInstanceBase* pkInstBase = rkChrMgr.GetInstancePtr(nVID);
	if (pkInstBase)
	{
		if (pkInstBase->IsPC())
			result = true;
		else
			result = false;
	}

	return Py_BuildValue("b", result);
}

PyObject* chrmgrRefreshAllNewPets(PyObject* poSelf, PyObject* poArgs)
{
	CPythonCharacterManager::Instance().RefreshAllNewPets();
	return Py_BuildNone();
}

PyObject* chrmgrRefreshAllMobs(PyObject* poSelf, PyObject* poArgs)
{
	CPythonCharacterManager::Instance().RefreshAllMobs();
	return Py_BuildNone();
}

PyObject* chrmgrRefreshAllStone(PyObject* poSelf, PyObject* poArgs)
{
	CPythonCharacterManager::Instance().RefreshAllStone();
	return Py_BuildNone();
}

void initchrmgr()
{
	static PyMethodDef s_methods[] =
	{
		{ "SetEmpireNameMode",			chrmgrSetEmpireNameMode,				METH_VARARGS },
		// ETC
		{ "SetEmoticon",				chrmgrSetEmoticon,						METH_VARARGS },
		{ "IsPossibleEmoticon",			chrmgrIsPossibleEmoticon,				METH_VARARGS },
		{ "SetMovingSpeed",				chrmgrSetMovingSpeed,					METH_VARARGS },
		{ "SetDustGap",					chrmgrSetDustGap,						METH_VARARGS },
		{ "SetHorseDustGap",			chrmgrSetHorseDustGap,					METH_VARARGS },

		{ "RegisterTitleName",			chrmgrRegisterTitleName,				METH_VARARGS },
		{ "RegisterNameColor",			chrmgrRegisterNameColor,				METH_VARARGS },
		{ "RegisterTitleColor",			chrmgrRegisterTitleColor,				METH_VARARGS },
		{ "IsPC",						chrmgrIsPc,								METH_VARARGS },
		{ "RefreshAllNewPets",			chrmgrRefreshAllNewPets,				METH_VARARGS },
		{ "RefreshAllMobs",				chrmgrRefreshAllMobs,					METH_VARARGS },
		{ "RefreshAllStone",			chrmgrRefreshAllStone,					METH_VARARGS },
		{ NULL,							NULL,									NULL },
	};

	PyObject* poModule = Py_InitModule("chrmgr", s_methods);

	PyModule_AddIntConstant(poModule, "NAMECOLOR_MOB", CInstanceBase::NAMECOLOR_NORMAL_MOB);
	PyModule_AddIntConstant(poModule, "NAMECOLOR_NPC", CInstanceBase::NAMECOLOR_NORMAL_NPC);
	PyModule_AddIntConstant(poModule, "NAMECOLOR_PC", CInstanceBase::NAMECOLOR_NORMAL_PC);
	PyModule_AddIntConstant(poModule, "NAMECOLOR_EMPIRE_MOB", CInstanceBase::NAMECOLOR_EMPIRE_MOB);
	PyModule_AddIntConstant(poModule, "NAMECOLOR_EMPIRE_NPC", CInstanceBase::NAMECOLOR_EMPIRE_NPC);
	PyModule_AddIntConstant(poModule, "NAMECOLOR_EMPIRE_PC", CInstanceBase::NAMECOLOR_EMPIRE_PC);
	PyModule_AddIntConstant(poModule, "NAMECOLOR_FUNC", CInstanceBase::NAMECOLOR_FUNC);
	PyModule_AddIntConstant(poModule, "NAMECOLOR_PK", CInstanceBase::NAMECOLOR_PK);
	PyModule_AddIntConstant(poModule, "NAMECOLOR_PVP", CInstanceBase::NAMECOLOR_PVP);
	PyModule_AddIntConstant(poModule, "NAMECOLOR_PARTY", CInstanceBase::NAMECOLOR_PARTY);
	PyModule_AddIntConstant(poModule, "NAMECOLOR_WARP", CInstanceBase::NAMECOLOR_WARP);
	PyModule_AddIntConstant(poModule, "NAMECOLOR_WAYPOINT", CInstanceBase::NAMECOLOR_WAYPOINT);
	PyModule_AddIntConstant(poModule, "NAMECOLOR_EXTRA", CInstanceBase::NAMECOLOR_EXTRA);

#ifdef ENABLE_METINSTONE_ON_MINIMAP
	PyModule_AddIntConstant(poModule, "NAMECOLOR_STONE", CInstanceBase::NAMECOLOR_STONE);
#endif
#ifdef ENABLE_BOSS_ON_MINIMAP
	PyModule_AddIntConstant(poModule, "NAMECOLOR_BOSS", CInstanceBase::NAMECOLOR_BOSS);
#endif
}