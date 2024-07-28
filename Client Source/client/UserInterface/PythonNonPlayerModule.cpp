#include "StdAfx.h"
#include "PythonNonPlayer.h"

#include "InstanceBase.h"
#include "PythonCharacterManager.h"
#include "../GameLib/RaceManager.h"

#ifdef ENABLE_WIKI_SYSTEM
PyObject* nonplayerFindMobSearchBox(PyObject* poSelf, PyObject* poArgs)
{
	char* sItemName;
	if (!PyTuple_GetString(poArgs, 0, &sItemName))
		return Py_BadArgument();
	int iIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &iIndex))
		return Py_BadArgument();
	bool reverse;
	if (!PyTuple_GetBoolean(poArgs, 2, &reverse))
		return Py_BadArgument();

	return Py_BuildValue("i", CPythonNonPlayer::Instance().FindMobSearchBox(sItemName, iIndex, reverse));
}
#endif
PyObject* nonplayerGetEventType(PyObject* poSelf, PyObject* poArgs)
{
	int iVirtualNumber;
	if (!PyTuple_GetInteger(poArgs, 0, &iVirtualNumber))
		return Py_BuildException();

	BYTE iType = CPythonNonPlayer::Instance().GetEventType(iVirtualNumber);

	return Py_BuildValue("i", iType);
}

PyObject* nonplayerGetEventTypeByVID(PyObject* poSelf, PyObject* poArgs)
{
	int iVirtualID;
	if (!PyTuple_GetInteger(poArgs, 0, &iVirtualID))
		return Py_BuildException();

	BYTE iType = CPythonNonPlayer::Instance().GetEventTypeByVID(iVirtualID);

	return Py_BuildValue("i", iType);
}

PyObject* nonplayerGetLevelByVID(PyObject* poSelf, PyObject* poArgs)
{
	int iVirtualID;
	if (!PyTuple_GetInteger(poArgs, 0, &iVirtualID))
		return Py_BuildException();

	CInstanceBase* pInstance = CPythonCharacterManager::Instance().GetInstancePtr(iVirtualID);

	if (!pInstance)
		return Py_BuildValue("i", -1);

	const CPythonNonPlayer::TMobTable* pMobTable = CPythonNonPlayer::Instance().GetTable(pInstance->GetVirtualNumber());

	if (!pMobTable)
		return Py_BuildValue("i", -1);

	float fAverageLevel = pMobTable->bLevel;//(float(pMobTable->abLevelRange[0]) + float(pMobTable->abLevelRange[1])) / 2.0f;
	fAverageLevel = floor(fAverageLevel + 0.5f);
	return Py_BuildValue("i", int(fAverageLevel));
}

PyObject* nonplayerGetGradeByVID(PyObject* poSelf, PyObject* poArgs)
{
	int iVirtualID;
	if (!PyTuple_GetInteger(poArgs, 0, &iVirtualID))
		return Py_BuildException();

	CInstanceBase* pInstance = CPythonCharacterManager::Instance().GetInstancePtr(iVirtualID);

	if (!pInstance)
		return Py_BuildValue("i", -1);

	const CPythonNonPlayer::TMobTable* pMobTable = CPythonNonPlayer::Instance().GetTable(pInstance->GetVirtualNumber());

	if (!pMobTable)
		return Py_BuildValue("i", -1);

	return Py_BuildValue("i", pMobTable->bRank);
}

PyObject* nonplayerGetMonsterName(PyObject* poSelf, PyObject* poArgs)
{
	int iVNum;
	if (!PyTuple_GetInteger(poArgs, 0, &iVNum))
		return Py_BuildException();

	CPythonNonPlayer& rkNonPlayer = CPythonNonPlayer::Instance();
	return Py_BuildValue("s", rkNonPlayer.GetMonsterName(iVNum));
}

PyObject* nonplayerLoadNonPlayerData(PyObject* poSelf, PyObject* poArgs)
{
	char* szFileName;
	if (!PyTuple_GetString(poArgs, 0, &szFileName))
		return Py_BuildException();

	CPythonNonPlayer::Instance().LoadNonPlayerData(szFileName);
	return Py_BuildNone();
}

#ifdef ENABLE_PENDANT_SYSTEM
PyObject* nonplayerGetMonsterRaceFlagAtt(PyObject* poSelf, PyObject* poArgs)
{
	int iVirtualID;
	if (!PyTuple_GetInteger(poArgs, 0, &iVirtualID))
		return Py_BuildException();

	CInstanceBase* pInstance = CPythonCharacterManager::Instance().GetInstancePtr(iVirtualID);

	if (!pInstance)
		return Py_BuildValue("i", -1);

	return Py_BuildValue("i", CPythonNonPlayer::Instance().GetMonsterRaceFlagAtt(pInstance->GetVirtualNumber()));
}
#endif

#ifdef ENABLE_TARGET_BOARD_RENEWAL
PyObject* nonplayerGetRaceNumByVID(PyObject* poSelf, PyObject* poArgs)
{
	int iVirtualID;
	if (!PyTuple_GetInteger(poArgs, 0, &iVirtualID))
		return Py_BuildException();

	CInstanceBase* pInstance = CPythonCharacterManager::Instance().GetInstancePtr(iVirtualID);

	if (!pInstance)
		return Py_BuildValue("i", -1);

	const CPythonNonPlayer::TMobTable* pMobTable = CPythonNonPlayer::Instance().GetTable(pInstance->GetVirtualNumber());

	if (!pMobTable)
		return Py_BuildValue("i", -1);

	return Py_BuildValue("i", pMobTable->dwVnum);
}

PyObject* nonplayerGetTableValueBYTE(PyObject* poSelf, PyObject* poArgs)
{
	int iMobVnum;
	if (!PyTuple_GetInteger(poArgs, 0, &iMobVnum))
		return Py_BuildException();

	char* szItorName;
	if (!PyTuple_GetString(poArgs, 1, &szItorName))
		return Py_BuildException();

	return Py_BuildValue("i", CPythonNonPlayer::Instance().GetTableValueBYTE(iMobVnum, std::string(szItorName)));
}

PyObject* nonplayerGetTableValueDWORD(PyObject* poSelf, PyObject* poArgs)
{
	int iMobVnum;
	if (!PyTuple_GetInteger(poArgs, 0, &iMobVnum))
		return Py_BuildException();

	char* szItorName;
	if (!PyTuple_GetString(poArgs, 1, &szItorName))
		return Py_BuildException();

	return Py_BuildValue("L", CPythonNonPlayer::Instance().GetTableValueDWORD(iMobVnum, std::string(szItorName)));
}

PyObject* nonplayerGetTableValueLongLong(PyObject* poSelf, PyObject* poArgs)
{
	int iMobVnum;
	if (!PyTuple_GetInteger(poArgs, 0, &iMobVnum))
		return Py_BuildException();

	char* szItorName;
	if (!PyTuple_GetString(poArgs, 1, &szItorName))
		return Py_BuildException();

	return Py_BuildValue("i", CPythonNonPlayer::Instance().GetTableValueLongLong(iMobVnum, std::string(szItorName)));
}

PyObject* nonplayerGetTableValueFloat(PyObject* poSelf, PyObject* poArgs)
{
	int iMobVnum;
	if (!PyTuple_GetInteger(poArgs, 0, &iMobVnum))
		return Py_BuildException();

	char* szItorName;
	if (!PyTuple_GetString(poArgs, 1, &szItorName))
		return Py_BuildException();

	return Py_BuildValue("i", CPythonNonPlayer::Instance().GetTableValueFloat(iMobVnum, std::string(szItorName)));
}

PyObject* nonplayerHasMonsterRaceFlag(PyObject* poSelf, PyObject* poArgs)
{
	int iFlag;
	if (!PyTuple_GetInteger(poArgs, 0, &iFlag))
		return Py_BuildException();

	int iMobVnum;
	if (!PyTuple_GetInteger(poArgs, 1, &iMobVnum))
		return Py_BuildException();

	return Py_BuildValue("i", CPythonNonPlayer::Instance().HasMonsterRaceFlag(iFlag, iMobVnum));
}
#endif
PyObject* nonplayerIsRaceAvaible(PyObject* poSelf, PyObject* poArgs)
{
	int race = 0;
	if (!PyTuple_GetInteger(poArgs, 0, &race))
		return Py_BadArgument();

	return Py_BuildValue("b", CRaceManager::Instance().RaceIsAvaible(race));
}

void initNonPlayer()
{
	static PyMethodDef s_methods[] =
	{
#ifdef ENABLE_WIKI_SYSTEM
		{ "FindMobSearchBox",			nonplayerFindMobSearchBox,			METH_VARARGS },
#endif
		{ "GetEventType",				nonplayerGetEventType,				METH_VARARGS },
		{ "GetEventTypeByVID",			nonplayerGetEventTypeByVID,			METH_VARARGS },
		{ "GetLevelByVID",				nonplayerGetLevelByVID,				METH_VARARGS },
		{ "GetGradeByVID",				nonplayerGetGradeByVID,				METH_VARARGS },
		{ "GetMonsterName",				nonplayerGetMonsterName,			METH_VARARGS },
#ifdef ENABLE_PENDANT_SYSTEM
		{ "GetMonsterRaceFlagAtt",		nonplayerGetMonsterRaceFlagAtt,		METH_VARARGS },
#endif
#ifdef ENABLE_TARGET_BOARD_RENEWAL
		{ "GetRaceNumByVID",			nonplayerGetRaceNumByVID,			METH_VARARGS },
		{ "GetTableValueBYTE",			nonplayerGetTableValueBYTE,			METH_VARARGS },
		{ "GetTableValueDWORD",			nonplayerGetTableValueDWORD,		METH_VARARGS },
		{ "GetTableValueLongLong",		nonplayerGetTableValueLongLong,		METH_VARARGS },
		{ "GetTableValueFloat",			nonplayerGetTableValueFloat,		METH_VARARGS },
		{ "HasMonsterRaceFlag",			nonplayerHasMonsterRaceFlag,		METH_VARARGS },
#endif
		{ "LoadNonPlayerData",			nonplayerLoadNonPlayerData,			METH_VARARGS },
		{ "IsRaceAvaible",				nonplayerIsRaceAvaible,			METH_VARARGS },

		{ NULL,							NULL,								NULL		 },
	};

	PyObject* poModule = Py_InitModule("nonplayer", s_methods);

	PyModule_AddIntConstant(poModule, "ON_CLICK_EVENT_NONE", CPythonNonPlayer::ON_CLICK_EVENT_NONE);
	PyModule_AddIntConstant(poModule, "ON_CLICK_EVENT_BATTLE", CPythonNonPlayer::ON_CLICK_EVENT_BATTLE);
	PyModule_AddIntConstant(poModule, "ON_CLICK_EVENT_SHOP", CPythonNonPlayer::ON_CLICK_EVENT_SHOP);
	PyModule_AddIntConstant(poModule, "ON_CLICK_EVENT_TALK", CPythonNonPlayer::ON_CLICK_EVENT_TALK);
	PyModule_AddIntConstant(poModule, "ON_CLICK_EVENT_VEHICLE", CPythonNonPlayer::ON_CLICK_EVENT_VEHICLE);

#ifndef ENABLE_TARGET_BOARD_RENEWAL
	PyModule_AddIntConstant(poModule, "PAWN", 0);
	PyModule_AddIntConstant(poModule, "S_PAWN", 1);
	PyModule_AddIntConstant(poModule, "KNIGHT", CPythonNonPlayer::MOB_RANK_KNIGHT);
	PyModule_AddIntConstant(poModule, "S_KNIGHT", CPythonNonPlayer::MOB_RANK_S_KNIGHT);
	PyModule_AddIntConstant(poModule, "BOSS", CPythonNonPlayer::MOB_RANK_BOSS);
	PyModule_AddIntConstant(poModule, "KING", CPythonNonPlayer::MOB_RANK_KING);
#else
	PyModule_AddIntConstant(poModule, "MOB_RESIST_SWORD", CPythonNonPlayer::MOB_RESIST_SWORD);
	PyModule_AddIntConstant(poModule, "MOB_RESIST_TWOHAND", CPythonNonPlayer::MOB_RESIST_TWOHAND);
	PyModule_AddIntConstant(poModule, "MOB_RESIST_DAGGER", CPythonNonPlayer::MOB_RESIST_DAGGER);
	PyModule_AddIntConstant(poModule, "MOB_RESIST_BELL", CPythonNonPlayer::MOB_RESIST_BELL);
	PyModule_AddIntConstant(poModule, "MOB_RESIST_FAN", CPythonNonPlayer::MOB_RESIST_FAN);
	PyModule_AddIntConstant(poModule, "MOB_RESIST_BOW", CPythonNonPlayer::MOB_RESIST_BOW);
	PyModule_AddIntConstant(poModule, "MOB_RESIST_FIRE", CPythonNonPlayer::MOB_RESIST_FIRE);
	PyModule_AddIntConstant(poModule, "MOB_RESIST_ELECT", CPythonNonPlayer::MOB_RESIST_ELECT);
	PyModule_AddIntConstant(poModule, "MOB_RESIST_MAGIC", CPythonNonPlayer::MOB_RESIST_MAGIC);
	PyModule_AddIntConstant(poModule, "MOB_RESIST_WIND", CPythonNonPlayer::MOB_RESIST_WIND);
	PyModule_AddIntConstant(poModule, "MOB_RESIST_POISON", CPythonNonPlayer::MOB_RESIST_POISON);
	PyModule_AddIntConstant(poModule, "MOB_RESISTS_MAX_NUM", CPythonNonPlayer::MOB_RESISTS_MAX_NUM);

	PyModule_AddIntConstant(poModule, "MOB_ENCHANT_CURSE", CPythonNonPlayer::MOB_ENCHANT_CURSE);
	PyModule_AddIntConstant(poModule, "MOB_ENCHANT_SLOW", CPythonNonPlayer::MOB_ENCHANT_SLOW);
	PyModule_AddIntConstant(poModule, "MOB_ENCHANT_POISON", CPythonNonPlayer::MOB_ENCHANT_POISON);
	PyModule_AddIntConstant(poModule, "MOB_ENCHANT_STUN", CPythonNonPlayer::MOB_ENCHANT_STUN);
	PyModule_AddIntConstant(poModule, "MOB_ENCHANT_CRITICAL", CPythonNonPlayer::MOB_ENCHANT_CRITICAL);
	PyModule_AddIntConstant(poModule, "MOB_ENCHANT_PENETRATE", CPythonNonPlayer::MOB_ENCHANT_PENETRATE);
	PyModule_AddIntConstant(poModule, "MOB_ENCHANTS_MAX_NUM", CPythonNonPlayer::MOB_ENCHANTS_MAX_NUM);

	PyModule_AddIntConstant(poModule, "RACE_FLAG_ANIMAL", CPythonNonPlayer::RACE_FLAG_ANIMAL);
	PyModule_AddIntConstant(poModule, "RACE_FLAG_UNDEAD", CPythonNonPlayer::RACE_FLAG_UNDEAD);
	PyModule_AddIntConstant(poModule, "RACE_FLAG_DEVIL", CPythonNonPlayer::RACE_FLAG_DEVIL);
	PyModule_AddIntConstant(poModule, "RACE_FLAG_HUMAN", CPythonNonPlayer::RACE_FLAG_HUMAN);
	PyModule_AddIntConstant(poModule, "RACE_FLAG_ORC", CPythonNonPlayer::RACE_FLAG_ORC);
	PyModule_AddIntConstant(poModule, "RACE_FLAG_MILGYO", CPythonNonPlayer::RACE_FLAG_MILGYO);
	PyModule_AddIntConstant(poModule, "RACE_FLAG_INSECT", CPythonNonPlayer::RACE_FLAG_INSECT);
	PyModule_AddIntConstant(poModule, "RACE_FLAG_FIRE", CPythonNonPlayer::RACE_FLAG_FIRE);
	PyModule_AddIntConstant(poModule, "RACE_FLAG_ICE", CPythonNonPlayer::RACE_FLAG_ICE);
	PyModule_AddIntConstant(poModule, "RACE_FLAG_DESERT", CPythonNonPlayer::RACE_FLAG_DESERT);
	PyModule_AddIntConstant(poModule, "RACE_FLAG_TREE", CPythonNonPlayer::RACE_FLAG_TREE);
#endif

#ifdef ENABLE_PENDANT_SYSTEM
	PyModule_AddIntConstant(poModule, "RACE_FLAG_ATT_ELEC", CPythonNonPlayer::RACE_FLAG_ATT_ELEC);
	PyModule_AddIntConstant(poModule, "RACE_FLAG_ATT_FIRE", CPythonNonPlayer::RACE_FLAG_ATT_FIRE);
	PyModule_AddIntConstant(poModule, "RACE_FLAG_ATT_ICE", CPythonNonPlayer::RACE_FLAG_ATT_ICE);
	PyModule_AddIntConstant(poModule, "RACE_FLAG_ATT_WIND", CPythonNonPlayer::RACE_FLAG_ATT_WIND);
	PyModule_AddIntConstant(poModule, "RACE_FLAG_ATT_EARTH", CPythonNonPlayer::RACE_FLAG_ATT_EARTH);
	PyModule_AddIntConstant(poModule, "RACE_FLAG_ATT_DARK", CPythonNonPlayer::RACE_FLAG_ATT_DARK);
#endif
}