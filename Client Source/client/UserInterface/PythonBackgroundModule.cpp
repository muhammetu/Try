#include "StdAfx.h"
#include "PythonBackground.h"
#ifdef ENABLE_ENVIRONMENT_EFFECT_OPTION
#include "PythonSystem.h"
#endif
#include "../eterlib/StateManager.h"
#include "../gamelib/MapOutDoor.h"

PyObject* backgroundDestroy(PyObject* poSelf, PyObject* poArgs)
{
	CPythonBackground& rkBG = CPythonBackground::Instance();
	rkBG.SetShadowLevel(CPythonBackground::SHADOW_NONE);
	rkBG.Destroy();
	return Py_BuildNone();
}

PyObject* backgroundRegisterEnvironmentData(PyObject* poSelf, PyObject* poArgs)
{
	int iIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iIndex))
		return Py_BadArgument();

	char* pszEnvironmentFileName;
	if (!PyTuple_GetString(poArgs, 1, &pszEnvironmentFileName))
		return Py_BadArgument();

	CPythonBackground& rkBG = CPythonBackground::Instance();
	if (!rkBG.RegisterEnvironmentData(iIndex, pszEnvironmentFileName))
	{
		TraceError("background.RegisterEnvironmentData(iIndex=%d, szEnvironmentFileName=%s)", iIndex, pszEnvironmentFileName);

		// TODO:
	}

	return Py_BuildNone();
}

PyObject* backgroundSetEnvironmentData(PyObject* poSelf, PyObject* poArgs)
{
	int iIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iIndex))
		return Py_BadArgument();

	const TEnvironmentData* c_pEnvironmenData;

	CPythonBackground& rkBG = CPythonBackground::Instance();
	if (rkBG.GetEnvironmentData(iIndex, &c_pEnvironmenData))
		rkBG.ResetEnvironmentDataPtr(c_pEnvironmenData);

	return Py_BuildNone();
}

PyObject* backgroundGetCurrentMapName(PyObject* poSelf, PyObject* poArgs)
{
	CPythonBackground& rkBG = CPythonBackground::Instance();
	return Py_BuildValue("s", rkBG.GetWarpMapName());
}

PyObject* backgroundGlobalPositionToLocalPosition(PyObject* poSelf, PyObject* poArgs)
{
	int iX;
	if (!PyTuple_GetInteger(poArgs, 0, &iX))
		return Py_BadArgument();

	int iY;
	if (!PyTuple_GetInteger(poArgs, 1, &iY))
		return Py_BadArgument();

	LONG lX = iX;
	LONG lY = iY;
	CPythonBackground& rkBG = CPythonBackground::Instance();
	rkBG.GlobalPositionToLocalPosition(lX, lY);

	return Py_BuildValue("ii", lX, lY);
}

PyObject* backgroundGlobalPositionToMapInfo(PyObject* poSelf, PyObject* poArgs)
{
	int iX;
	if (!PyTuple_GetInteger(poArgs, 0, &iX))
		return Py_BadArgument();

	int iY;
	if (!PyTuple_GetInteger(poArgs, 1, &iY))
		return Py_BadArgument();

	CPythonBackground& rkBG = CPythonBackground::Instance();
	CPythonBackground::TMapInfo* pkMapInfo = rkBG.GlobalPositionToMapInfo(iX, iY);

	if (pkMapInfo)
		return Py_BuildValue("sii", pkMapInfo->m_strName.c_str(), pkMapInfo->m_dwBaseX, pkMapInfo->m_dwBaseY);
	else
		return Py_BuildValue("sii", "", 0, 0);
}

PyObject* backgroundRegisterDungeonMapName(PyObject* poSelf, PyObject* poArgs)
{
	char* szName;
	if (!PyTuple_GetString(poArgs, 0, &szName))
		return Py_BadArgument();

	CPythonBackground::Instance().RegisterDungeonMapName(szName);
	return Py_BuildNone();
}

#ifdef ENABLE_FOG_FIX
PyObject* backgroundSetEnvironmentFog(PyObject* poSelf, PyObject* poArgs)
{
	bool bFlag;
	if (!PyTuple_GetBoolean(poArgs, 0, &bFlag))
		return Py_BadArgument();

	CPythonBackground& rkBG = CPythonBackground::Instance();
	rkBG.SetEnvironmentFog(bFlag);

	return Py_BuildNone();
}
#endif

#ifdef ENABLE_TEXTURE_MODE
PyObject* backgroundTextureChange(PyObject* poSelf, PyObject* poArgs)
{
	char* szFileName;
	if (!PyTuple_GetString(poArgs, 0, &szFileName))
		return Py_BuildException();

	CPythonBackground& rkBG = CPythonBackground::Instance();
	CMapOutdoor& rkMap = rkBG.GetMapOutdoorRef();
	rkMap.TextureChange(szFileName);
	return Py_BuildNone();
}
#endif

#ifdef ENABLE_MAP_ALGORITHM_RENEWAL
PyObject* backgroundGetMapIndex(PyObject* poSelf, PyObject* poArgs)
{
	char* stMapName;
	if (!PyTuple_GetString(poArgs, 0, &stMapName))
		return Py_BuildException();

	CPythonBackground& rkBG = CPythonBackground::Instance();
	return Py_BuildValue("i", rkBG.GetMapIndex(std::string(stMapName)));
}

PyObject* backgroundGetMapName(PyObject * poSelf, PyObject * poArgs)
{
	int mapIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &mapIndex))
		return Py_BuildException();

	CPythonBackground& rkBG = CPythonBackground::Instance();
	return Py_BuildValue("s", rkBG.GetMapName(mapIndex).c_str());
}

PyObject* backgroundGetRealMapName(PyObject* poSelf, PyObject* poArgs)
{
	char* stMapName;
	if (!PyTuple_GetString(poArgs, 0, &stMapName))
		return Py_BuildException();

	CPythonBackground& rkBG = CPythonBackground::Instance();
	return Py_BuildValue("s", rkBG.GetRealMapName(std::string(stMapName)).c_str());
}
#endif

PyObject * backgroundSelectViewDistanceNum(PyObject * poSelf, PyObject * poArgs)
{
	int iNum;

	if (!PyTuple_GetInteger(poArgs, 0, &iNum))
		return Py_BadArgument();

	CPythonBackground& rkBG=CPythonBackground::Instance();
	rkBG.SelectViewDistanceNum(iNum);

	return Py_BuildNone();
}

PyObject * backgroundSetViewDistanceSet(PyObject * poSelf, PyObject * poArgs)
{
	int iNum;
	if (!PyTuple_GetInteger(poArgs, 0, &iNum))
		return Py_BadArgument();

	float fFarClip;
	if (!PyTuple_GetFloat(poArgs, 1, &fFarClip))
		return Py_BadArgument();

	CPythonBackground& rkBG=CPythonBackground::Instance();
	rkBG.SetViewDistanceSet(iNum, fFarClip);
	return Py_BuildNone();
}

void initBackground()
{
	static PyMethodDef s_methods[] =
	{
		{ "GlobalPositionToLocalPosition",		backgroundGlobalPositionToLocalPosition,	METH_VARARGS },
		{ "GlobalPositionToMapInfo",			backgroundGlobalPositionToMapInfo,			METH_VARARGS },
		{ "Destroy",							backgroundDestroy,							METH_VARARGS },
		{ "RegisterEnvironmentData",			backgroundRegisterEnvironmentData,			METH_VARARGS },
		{ "SetEnvironmentData",					backgroundSetEnvironmentData,				METH_VARARGS },
		{ "GetCurrentMapName",					backgroundGetCurrentMapName,				METH_VARARGS },

		{ "RegisterDungeonMapName",				backgroundRegisterDungeonMapName,			METH_VARARGS },
#ifdef ENABLE_FOG_FIX
		{ "SetEnvironmentFog",					backgroundSetEnvironmentFog,				METH_VARARGS },
#endif
#ifdef ENABLE_TEXTURE_MODE
		{ "TextureChange",						backgroundTextureChange,					METH_VARARGS },
#endif
#ifdef ENABLE_MAP_ALGORITHM_RENEWAL
		{ "GetMapIndex",						backgroundGetMapIndex,						METH_VARARGS },
		{ "GetMapName",							backgroundGetMapName,						METH_VARARGS },
		{ "GetRealMapName",						backgroundGetRealMapName,					METH_VARARGS },
#endif
		{ "SelectViewDistanceNum",				backgroundSelectViewDistanceNum,			METH_VARARGS },
		{ "SetViewDistanceSet",					backgroundSetViewDistanceSet,				METH_VARARGS },
		{ NULL, NULL, NULL },
	};

	PyObject* poModule = Py_InitModule("background", s_methods);

	PyModule_AddIntConstant(poModule, "PART_SKY", CMapOutdoor::PART_SKY);
	PyModule_AddIntConstant(poModule, "PART_TREE", CMapOutdoor::PART_TREE);
	PyModule_AddIntConstant(poModule, "PART_CLOUD", CMapOutdoor::PART_CLOUD);
	PyModule_AddIntConstant(poModule, "PART_WATER", CMapOutdoor::PART_WATER);
	PyModule_AddIntConstant(poModule, "PART_OBJECT", CMapOutdoor::PART_OBJECT);
	PyModule_AddIntConstant(poModule, "PART_TERRAIN", CMapOutdoor::PART_TERRAIN);

	PyModule_AddIntConstant(poModule, "SKY_RENDER_MODE_DEFAULT", CSkyObject::SKY_RENDER_MODE_DEFAULT);
	PyModule_AddIntConstant(poModule, "SKY_RENDER_MODE_DIFFUSE", CSkyObject::SKY_RENDER_MODE_DIFFUSE);
	PyModule_AddIntConstant(poModule, "SKY_RENDER_MODE_TEXTURE", CSkyObject::SKY_RENDER_MODE_TEXTURE);
	PyModule_AddIntConstant(poModule, "SKY_RENDER_MODE_MODULATE", CSkyObject::SKY_RENDER_MODE_MODULATE);
	PyModule_AddIntConstant(poModule, "SKY_RENDER_MODE_MODULATE2X", CSkyObject::SKY_RENDER_MODE_MODULATE2X);
	PyModule_AddIntConstant(poModule, "SKY_RENDER_MODE_MODULATE4X", CSkyObject::SKY_RENDER_MODE_MODULATE4X);

	PyModule_AddIntConstant(poModule, "SHADOW_NONE", CPythonBackground::SHADOW_NONE);
	PyModule_AddIntConstant(poModule, "SHADOW_GROUND", CPythonBackground::SHADOW_GROUND);
	PyModule_AddIntConstant(poModule, "SHADOW_GROUND_AND_SOLO", CPythonBackground::SHADOW_GROUND_AND_SOLO);
	PyModule_AddIntConstant(poModule, "SHADOW_ALL", CPythonBackground::SHADOW_ALL);
	PyModule_AddIntConstant(poModule, "SHADOW_ALL_HIGH", CPythonBackground::SHADOW_ALL_HIGH);
	PyModule_AddIntConstant(poModule, "SHADOW_ALL_MAX", CPythonBackground::SHADOW_ALL_MAX);

	PyModule_AddIntConstant(poModule, "DISTANCE0", CPythonBackground::DISTANCE0);
	PyModule_AddIntConstant(poModule, "DISTANCE1", CPythonBackground::DISTANCE1);
	PyModule_AddIntConstant(poModule, "DISTANCE2", CPythonBackground::DISTANCE2);
	PyModule_AddIntConstant(poModule, "DISTANCE3", CPythonBackground::DISTANCE3);
	PyModule_AddIntConstant(poModule, "DISTANCE4", CPythonBackground::DISTANCE4);

	PyModule_AddIntConstant(poModule, "DISTANCE_SORT", CMapOutdoor::DISTANCE_SORT);
	PyModule_AddIntConstant(poModule, "TEXTURE_SORT", CMapOutdoor::TEXTURE_SORT);
}
