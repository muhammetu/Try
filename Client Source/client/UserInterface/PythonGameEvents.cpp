#include "StdAfx.h"

#include "PythonSkill.h"
#include "PythonNonPlayer.h"
#include "PythonPlayer.h"
#include "PythonCharacterManager.h"
#include "PythonGameEvents.h"
#include "PythonBackground.h"

void CPythonGameEvents::SetActivateEvent(bool isActivate, BYTE bEventID)
{
	if (bEventID > EVENT_MAX_NUM)
		return;

	m_pkActivateEvents[bEventID] = isActivate;
}

void CPythonGameEvents::SetEventTime(BYTE bEventID, DWORD event_time)
{
	if (bEventID > EVENT_MAX_NUM)
		return;

	m_dwEventEndTime[bEventID] = event_time;
}

bool CPythonGameEvents::IsActivateEvent(BYTE bEventID)
{
	return m_pkActivateEvents[bEventID];
}

DWORD CPythonGameEvents::GetEventTime(BYTE bEventID)
{
	return m_dwEventEndTime[bEventID];
}

CPythonGameEvents::CPythonGameEvents()
{
	for (int i = 0; i < EVENT_MAX_NUM; ++i)
	{
		m_pkActivateEvents[i] = false;
		m_dwEventEndTime[i] = 0;
	}
}

CPythonGameEvents::~CPythonGameEvents()
{
}

PyObject* gameEventsIsActivateEvent(PyObject* poSelf, PyObject* poArgs)
{
	int iEventIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iEventIndex))
		return Py_BuildNone();

	bool isActivate = CPythonGameEvents::instance().IsActivateEvent(iEventIndex);

	return Py_BuildValue("i", isActivate == true ? 1 : 0);
}

PyObject* gameEventsGetEventTime(PyObject* poSelf, PyObject* poArgs)
{
	int iEventIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iEventIndex))
		return Py_BuildNone();

	DWORD event_time = CPythonGameEvents::instance().GetEventTime(iEventIndex);

	return Py_BuildValue("i", event_time);
}

void initGameEvents()
{
	static PyMethodDef s_methods[] =
	{
		{"IsActivateEvent",gameEventsIsActivateEvent,METH_VARARGS},
		{"GetEventTime",gameEventsGetEventTime,METH_VARARGS},
		{NULL, NULL, NULL},
	};

	PyObject* poModule = Py_InitModule("gameEvents", s_methods);

	/*******************************************************************************************/
	/*** Events ***/
	/*******************************************************************************************/
	PyModule_AddIntConstant(poModule, "EVENT_DOUBLEMETIN", EVENT_DOUBLEMETIN);
	PyModule_AddIntConstant(poModule, "EVENT_DOUBLEBOSS", EVENT_DOUBLEBOSS);
	PyModule_AddIntConstant(poModule, "EVENT_MAXDAMAGE", EVENT_MAXDAMAGE);
	PyModule_AddIntConstant(poModule, "EVENT_X2DUYURU", EVENT_X2DUYURU);
	PyModule_AddIntConstant(poModule, "EVENT_X2SURE", EVENT_X2SURE);
	PyModule_AddIntConstant(poModule, "EVENT_UPGRADE", EVENT_UPGRADE);
	PyModule_AddIntConstant(poModule, "EVENT_MAX_NUM", EVENT_MAX_NUM);
	/*******************************************************************************************/
	/*** END ***/
	/*******************************************************************************************/
}
