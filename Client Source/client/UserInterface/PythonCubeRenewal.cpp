#include "StdAfx.h"
#include "PythonCubeRenewal.h"
#include "PythonNetworkStream.h"
#include "Packet.h"

CPythonCubeRenewal::CPythonCubeRenewal()
	: m_CubeRenewalHandler(NULL)
{
	ClearList();
}

CPythonCubeRenewal::~CPythonCubeRenewal()
{
	ClearList();
}

void CPythonCubeRenewal::LoadingList()
{
	if (m_CubeRenewalHandler)
		PyCallClassMemberFunc(m_CubeRenewalHandler, "BINARY_CUBE_RENEWAL_LOADING", Py_BuildValue("()"));
}

void CPythonCubeRenewal::ClearList()
{
	cube_renewal_list.clear();
}

void CPythonCubeRenewal::ReceiveList(const TInfoDateCubeRenewal& TInfoCR)
{
	cube_renewal_list.push_back(TInfoCR);
}

int CPythonCubeRenewal::GetCountList()
{
	return cube_renewal_list.size();
}

const CPythonCubeRenewal::TInfoStrucCubeRenewal& CPythonCubeRenewal::GetList()
{
	return cube_renewal_list;
}

PyObject* GetCountCubeRenewalList(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", CPythonCubeRenewal::Instance().GetCountList());
}

PyObject* GetCubeRenewalDates(PyObject* poSelf, PyObject* poArgs)
{
	int index;
	if (!PyTuple_GetInteger(poArgs, 0, &index))
		return Py_BuildException();

	const CPythonCubeRenewal::TInfoStrucCubeRenewal& CRItemVector = CPythonCubeRenewal::Instance().GetList();
	if (DWORD(index) >= CRItemVector.size())
#ifdef ENABLE_GOLD_LIMIT_REWORK
#ifdef ENABLE_GEM_SYSTEM
		return Py_BuildValue("iibiiiiiiiiiiiLiis", 0, 0, false, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "None");
#else
		return Py_BuildValue("iibiiiiiiiiiiiLis", 0, 0, false, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "None");
#endif
#else
#ifdef ENABLE_GEM_SYSTEM
		return Py_BuildValue("iibiiiiiiiiiiiiiis", 0, 0, false, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "None");
#else
		return Py_BuildValue("iibiiiiiiiiiiiiis", 0, 0, false, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "None");
#endif
#endif

	const TInfoDateCubeRenewal& CRenewalItemVector = CRItemVector[index];
#ifdef ENABLE_GOLD_LIMIT_REWORK
#ifdef ENABLE_GEM_SYSTEM
	return Py_BuildValue("iibiiiiiiiiiiiiLiis",
#else
	return Py_BuildValue("iibiiiiiiiiiiiiLis",
#endif
#else
#ifdef ENABLE_GEM_SYSTEM
	return Py_BuildValue("iibiiiiiiiiiiiiiiis",
#else
	return Py_BuildValue("iibiiiiiiiiiiiiiis",
#endif
#endif
		CRenewalItemVector.vnum_reward,
		CRenewalItemVector.count_reward,
		CRenewalItemVector.item_reward_stackable,
		CRenewalItemVector.vnum_material_1,
		CRenewalItemVector.count_material_1,
		CRenewalItemVector.vnum_material_2,
		CRenewalItemVector.count_material_2,
		CRenewalItemVector.vnum_material_3,
		CRenewalItemVector.count_material_3,
		CRenewalItemVector.vnum_material_4,
		CRenewalItemVector.count_material_4,
		CRenewalItemVector.vnum_material_5,
		CRenewalItemVector.count_material_5,
		CRenewalItemVector.vnum_material_6,
		CRenewalItemVector.count_material_6,
		CRenewalItemVector.gold,
#ifdef ENABLE_GEM_SYSTEM
		CRenewalItemVector.gem,
#endif
		CRenewalItemVector.percent,
		CRenewalItemVector.category
	);
}

PyObject* SetCubeRenewalHandler(PyObject* poSelf, PyObject* poArgs)
{
	PyObject* poEventHandler;
	if (!PyTuple_GetObject(poArgs, 0, &poEventHandler))
		return Py_BuildException();

	CPythonCubeRenewal::Instance().SetCubeRenewalHandler(poEventHandler);
	return Py_BuildNone();
}

PyObject* CubeRenewalMakeItem(PyObject* poSelf, PyObject* poArgs)
{
	int index_craft;
	if (!PyTuple_GetInteger(poArgs, 0, &index_craft))
		return Py_BuildException();

	int count_item;
	if (!PyTuple_GetInteger(poArgs, 1, &count_item))
		return Py_BuildException();

	int index_item_improve;
	if (!PyTuple_GetInteger(poArgs, 2, &index_item_improve))
		return Py_BuildException();

	CPythonNetworkStream& rkNetStream = CPythonNetworkStream::Instance();
	rkNetStream.CubeRenewalMakeItem(index_craft, count_item, index_item_improve);
	return Py_BuildNone();
}

PyObject* CubeRenewalClose(PyObject* poSelf, PyObject* poArgs)
{
	CPythonNetworkStream& rkNetStream = CPythonNetworkStream::Instance();
	rkNetStream.CubeRenewalClose();
	return Py_BuildNone();
}

void intcuberenewal()
{
	static PyMethodDef s_methods[] =
	{
		{ "CountDates",						GetCountCubeRenewalList,					METH_VARARGS },
		{ "GetDates", 						GetCubeRenewalDates, 						METH_VARARGS },
		{ "SetCubeRenewalHandler",			SetCubeRenewalHandler,						METH_VARARGS },
		{ "CubeRenewalMakeItem",			CubeRenewalMakeItem,						METH_VARARGS },
		{ "CubeRenewalClose",				CubeRenewalClose,							METH_VARARGS},

		{ NULL,								NULL,								NULL },
	};

	PyObject* poModule = Py_InitModule("cube_renewal", s_methods);
}